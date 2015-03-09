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
 *  ReducibleDataSource.h
 *
 *  Created on: Aug 23, 2010
 *      Author: nick
 */

#ifndef REDUCEABLEDATASOURCE_H_
#define REDUCEABLEDATASOURCE_H_

#include <vector>
#include <boost/mpi.hpp>

#include "TDataSource.h"
#include "SVDataSource.h"
#include "Variable.h"
#include "RepastProcess.h"

namespace repast {

/**
 * Source of data and a reduction operation. Used internally by a SVDataSet to
 * store the data sources. their associated ops etc.
 */
template <typename Op, typename T>
class ReducibleDataSource : public SVDataSource {

private:
	bool dirty;

protected:
	Op _op;
	std::vector<T> data;
	TDataSource<T>* _dataSource;
	int rank;

public:
	ReducibleDataSource(std::string name, TDataSource<T>* dataSource, Op op);
	~ReducibleDataSource();

	virtual void record();
	virtual void write(Variable* var);
	virtual SVDataSource::DataType type() const {
		return data_type_traits<T>::data_type();
	}
};

template<typename Op, typename T>
ReducibleDataSource<Op, T>::ReducibleDataSource(std::string name, TDataSource<T>* dataSource, Op op) : SVDataSource(name), dirty(false), _op(op),
_dataSource(dataSource) {
	rank = RepastProcess::instance()->rank();
};

template<typename Op, typename T>
ReducibleDataSource<Op, T>::~ReducibleDataSource() {
	delete _dataSource;
}

template<typename Op, typename T>
void ReducibleDataSource<Op, T>::record() {
	data.push_back(_dataSource->getData());
}

template<typename Op, typename T>
void ReducibleDataSource<Op, T>::write(Variable* var) {
	boost::mpi::communicator* comm = RepastProcess::instance()->getCommunicator();
	if (rank == 0) {
		size_t size = data.size();
		T* results = new T[size];
		reduce(*comm, &data[0], size, results, _op, 0);
		var->insert(results, size);
		delete[] results;
	} else {
		reduce(*comm, &data[0], data.size(), _op, 0);
	}
	data.clear();
}

}

#endif /* REDUCEABLEDATASOURCE_H_ */
