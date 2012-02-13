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
 *  DataSetBuilder.h
 *
 *  Created on: Aug 23, 2010
 *      Author: nick
 */

#ifndef SVDATASETBUILDER_H_
#define SVDATASETBUILDER_H_

#include "SVDataSet.h"
#include "ReducibleDataSource.h"
#include "Schedule.h"

namespace repast {

/**
 * Creates a SVDataSource with the specified name that will retreive int data from the
 * specified TDataSource, and perform the specified reduction Op on it. This function
 * is used to add data sources to an SVDataSetBuilder prior to creating an SVDataSet
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
SVDataSource* createSVDataSource(std::string name, TDataSource<int>* intDataSource, Op op) {
	return new ReducibleDataSource<Op, int> (name, intDataSource, op);
}

/**
 * Creates a SVDataSource with the specified name that will retreive double data from the
 * specified TDataSource, and perform the specified reduction Op on it. This function
 * is used to add data sources to an SVDataSetBuilder prior to creating an SVDataSet
 * from it.
 *
 * @param name the name of the data source. This will be the name of the variable for which
 * data is collected from the TDataSource.
 * @param doubleDataSource the actual source of the data that will be recorded.
 * @param op the reduction operation to perform on the recorded data when combining
 * the data across processes.
 *
 * @tparam Op an associative binary function or function object that work with ints. For example,
 * std::plus<int>, or mpi::minimum<int> and so on.
 */
template<typename Op>
SVDataSource* createSVDataSource(std::string name, TDataSource<double>* doubleDataSource, Op op) {
	return new ReducibleDataSource<Op, double> (name, doubleDataSource, op);
}

/**
 * Used to build SVDataSets to record data in plain text tabular format. Steps for use
 * are:
 * <ol>
 * <li>Create a SVDataSetBuilder.
 * <li> Add SVDataSources to the builder using the createSVDataSource functions. Each
 * data source defines a column in the output and where the data for that column will be retrieved.
 * Recording data on the SVDataSet produced by the builder will record this data for
 * each column.
 * <li>Call createDataSet to create the SVDataSet.
 * <li>Schedule calls to record and write on the SVDataSet.
 * </ol>
 */
class SVDataSetBuilder {

private:
	SVDataSet* dataSet;
	bool returned;

public:
	/**
	 * Creates a SVDataSetBuilder that will create a SVDataSet that will write to the specified file and use the specified
	 * string as a data value separator. Tick info will be gathered from the specified schedule.
	 *
	 * @param file the file path where the data will be recorded to
	 * @param separator a string used to separate the data values (e.g. a ",").
	 *
	 */
	SVDataSetBuilder(const std::string& file, const std::string& separator, const Schedule& schedule);
	~SVDataSetBuilder() {
	}

	/**
	 * Adds a DataSource to the DataSet produced by this builder. The createDataSource functions can
	 * be used to create Data Sources. Each
	 * data source defines a column in the output and where the data for that column will be retrieved.
	 * Recording data on the SVDataSet produced by the builder will record this data for
	 * each column.
	 *
	 * @param source the data source to add
	 */
	SVDataSetBuilder& addDataSource(SVDataSource* source);

	/**
	 * Creates the DataSource defined by this builder. This can only be called once.
	 * The caller is responsible for properly deleting the returned pointer.
	 */
	SVDataSet* createDataSet();
};

}

#endif /* DATASETBUILDER_H_ */
