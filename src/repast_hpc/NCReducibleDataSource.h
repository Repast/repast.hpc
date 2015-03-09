/*
 *   Repast for High Performance Computing (Repast HPC)
 *
 *   Copyright (c) 2010 Argonne National Laboratory
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with
 *   or without modification, are permitted provided that the following
 *   conditions are met:
 *
 *     Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *
 *     Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *
 *     Neither the name of the Argonne National Laboratory nor the names of its
 *     contributors may be used to endorse or promote products derived from
 *     this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 *   PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE TRUSTEES OR
 *   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 *   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 *   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 *   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 *   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 *   EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *
 *  NCReducibleDataSource.h
 *
 *  Created on: Oct 14, 2010
 *      Author: nick
 */

#ifndef NCREDUCIBLEDATASOURCE_H_
#define NCREDUCIBLEDATASOURCE_H_

#include "NCDataSource.h"
#include "RepastProcess.h"
#include "TDataSource.h"

#include <boost/mpi.hpp>
#include <vector>
#include <netcdfcpp.h>

namespace repast {

/**
 * Source of data and a reduction operation. Used internally by a NCDataSet to
 * store the data sources. their associated ops etc.
 */
template <typename Op, typename T>
class NCReducibleDataSource : public NCDataSource {

protected:
	Op op_;
	std::vector<T> data;
	TDataSource<T>* dataSource_;
	int rank, start;

public:
	NCReducibleDataSource(std::string name, TDataSource<T>* dataSource, Op op);
	~NCReducibleDataSource();

	virtual NcType ncType();

	virtual void record();
	virtual void write(NcVar* var);

};

template<typename Op, typename T>
NCReducibleDataSource<Op, T>::NCReducibleDataSource(std::string name, TDataSource<T>* dataSource, Op op) : NCDataSource(name), op_(op),
dataSource_(dataSource), start(0) {
	rank = RepastProcess::instance()->rank();
};

template<typename Op, typename T>
NCReducibleDataSource<Op, T>::~NCReducibleDataSource() {
	delete dataSource_;
}
template<typename Op, typename T>
NcType NCReducibleDataSource<Op, T>::ncType() {
	return NcTypeTrait<T>::type;
}

template<typename Op, typename T>
void NCReducibleDataSource<Op, T>::record() {
	data.push_back(dataSource_->getData());
}

template<typename Op, typename T>
void NCReducibleDataSource<Op, T>::write(NcVar* var) {
	boost::mpi::communicator* comm = RepastProcess::instance()->getCommunicator();
	if (rank == 0) {
		size_t size = data.size();
		T* results = new T[size];
		reduce(*comm, &data[0], size, results, op_, 0);

		var->set_cur(start, 0);
		// writing results along the tick dimension
		// and run dimension -- each result is indexed by the
		// the tick values of the tick dimension and the single run dimension
		var->put(results, size, 1);
		start += size;

		delete[] results;
	} else {
		reduce(*comm, &data[0], data.size(), op_, 0);
	}
	data.clear();
}



}


#endif /* NCREDUCIBLEDATASOURCE_H_ */
