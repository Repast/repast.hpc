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
 *  NCDataSetBuilder.h
 *
 *  Created on: Oct 14, 2010
 *      Author: nick
 */

#ifndef NCDATASETBUILDER_H_
#define NCDATASETBUILDER_H_

#include "NCDataSet.h"
#include "Schedule.h"

namespace repast {

/**
 * Used to build NCDataSets to record data in NetCDF format. Steps for use
 * are:
 * <ol>
 * <li>Create a NCDataSetBuilder.
 * <li> Add NCDataSources to the builder using the createNCDataSource functions. Each
 * DataSource defines a variable and where the data for that variable will be retrieved.
 * Recording data on the NCDataSet produced by the builder will record this data for
 * each variable.
 * <li>Call createDataSet to create the NCDataSet.
 * <li>Schedule calls to record and write on the NCDataSet.
 * </ol>
 */
class NCDataSetBuilder {

private:
	NCDataSet* dataSet;
	bool returned;

public:
	/**
	 * Creates an NCDataSetBuilder that will write to the specified file and
	 * get its tick counts from the specified schedule.
	 *
	 * @param file the name of the file to write to. Only rank 0 will
	 * actually write to this file.
	 * @param schedule the schedule to get tick counts from
	 */
	NCDataSetBuilder(std::string file, const Schedule& schedule);
	~NCDataSetBuilder();

	/**
	 * Adds a NCDataSource to this NCDataSetBuilder. The added NCDataSource defines
	 * a variable and where the data for that variable will be retrieved.
	 * Recording data on the NCDataSet produced by this builder will record this data for
	 * each variable.
	 */
	NCDataSetBuilder& addDataSource(NCDataSource* source);

	/**
	 * Creates the NCDataSet defined by this NCDataSetBuilder.
	 * The caller is responsible for properly deleting the
	 * returned pointer.
	 *
	 * @return the created NCDataSet.
	 */
	NCDataSet* createDataSet();

};

}

#endif /* NCDATASETBUILDER_H_ */
