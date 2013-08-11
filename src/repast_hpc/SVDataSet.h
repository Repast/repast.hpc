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
 *  Created on: Aug 23, 2010
 *      Author: nick
 */

#ifndef SVDATASET_H_
#define SVDATASET_H_

#include <fstream>
#include <vector>

#include "Schedule.h"
#include "Variable.h"
#include "SVDataSource.h"
#include "DataSet.h"

namespace repast {

class SVDataSetBuilder;

/**
 * Encapsulates data recording to a single plain text file, separating the recorded
 * values using a specified separator value. An SVDataSet uses rank 0 to
 * write to a single file from multiple pan-process data sources. A SVDataSet
 * should be built using a SVDataSetBuilder.
 */
class SVDataSet: public DataSet {

private:
	friend class SVDataSetBuilder;

	std::string _separator;
	std::vector<SVDataSource*> dataSources;
	std::vector<double> ticks;
	std::vector<Variable*> vars;
	const Schedule* _schedule;

	std::ofstream out;
	bool open;
	int rank;

	void init();

	/**
	 * Creates a DataSet that will write to the specified file and use the specified
	 * string as a data value separator. Tick info will be gathered from the specified schedule.
	 */
	// private so only SVDataSetBuilder can create it
	SVDataSet(const std::string& file, const std::string& separator, const Schedule* schedule);

public:

	~SVDataSet();

	/**
	 * Records data from any added data sources.
	 */
	void record();

	/**
	 * Writes any recorded data to a file.
	 */
	void write();

	/**
	 * Closes the data set.
	 */
	void close();
};

}

#endif /* DATASET_H_ */
