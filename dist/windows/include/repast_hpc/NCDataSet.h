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
 *  DataSet.h
 *
 *  Created on: Jun 8, 2009
 *      Author: nick
 */

#ifndef NCDATASET_H
#define NCDATASET_H

#include <vector>
#include <boost/mpi.hpp>
#include <netcdfcpp.h>

#include "Schedule.h"
#include "RepastProcess.h"
#include "TDataSource.h"
#include "NCReducibleDataSource.h"
#include "DataSet.h"

namespace repast {

class NCDataSetBuilder;

/**
 * Provides data recording and writing into a single file in NetCDF
 * format. A NCDataSet uses rank 0 to
 * write to a single file from multiple pan-process data sources. A NCDataSet
 * should be built using a NCDataSetBuilder.
 */
class NCDataSet: public DataSet {

	friend class NCDataSetBuilder;

private:
	std::vector<NCDataSource*> dataSources;
	std::vector<double> ticks;
	std::string file_;
	const Schedule* schedule_;
	int rank, start;
	bool open;

	NcFile* ncfile;

	// private so can only be created using an NCDataSetBuilder
	NCDataSet(std::string file, const Schedule& schedule);

public:

	virtual ~NCDataSet();

	// doc inherited from DataSet
	void record();

	// doc inherited from DataSet
	void write();

	// doc inherited from DataSet
	void close();
};

/**
 * Creates an NCDataSource with the specified name that will retreive int data from the
 * specified TDataSource, and perform the specified reduction Op on it. This function
 * is used to add data sources to an NCDataSetBuilder prior to creating an NCDataSet
 * from it.
 *
 * @param name the name of the data source. This will be the name of the variable for which
 * data is collected from the TDataSource.
 * @param intDataSource the actual source of the data that will be recorded.
 * @param op the reduction operation to perform on the recorded data when combining
 * the data across processes.
 *
 * @tparam Op an associative binary function or function object that work with ints. For example,
 * std::plus<int>, or mpi::minimum<int> and so on.
 */
template<typename Op>
NCDataSource* createNCDataSource(std::string name, TDataSource<int>* intDataSource, Op op) {
	return new NCReducibleDataSource<Op, int> (name, intDataSource, op);
}

/**
 * Creates an NCDataSource with the specified name that will retreive double data from the
 * specified TDataSource, and perform the specified reduction Op on it. This function
 * is used to add data sources to an NCDataSetBuilder prior to creating an NCDataSet
 * from it.
 *
 * @param name the name of the data source. This will be the name of the variable for which
 * data is collected from the TDataSource.
 * @param doubleDataSource the actual source of the data that will be recorded.
 * @param op the reduction operation to perform on the recorded data when combining
 * the data across processes.
 *
 * @tparam Op an associative binary function or function object that work with doubles. For example,
 * std::plus<double>, or mpi::minimum<double> and so on.
 */
template<typename Op>
NCDataSource* createNCDataSource(std::string name, TDataSource<double>* doubleDataSource, Op op) {
	return new NCReducibleDataSource<Op, double> (name, doubleDataSource, op);
}

}

#endif /* DATASET_H_ */
