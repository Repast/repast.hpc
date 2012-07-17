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
 *  DataSet.cpp
 *
 *  Created on: Aug 23, 2010
 *      Author: nick
 */

#include <exception>

#include <boost/filesystem.hpp>

#include "SVDataSet.h"
#include "RepastProcess.h"
#include "io.h"
#include "RepastErrors.h"

namespace fs = boost::filesystem;

namespace repast {

SVDataSet::SVDataSet(const std::string& file, const std::string& separator, const Schedule* schedule) :
	_separator(separator), _schedule(schedule), out(), open(true) {
	rank = RepastProcess::instance()->rank();
	if (rank == 0) {
	  fs::path filepath(file);
    if (!fs::exists(filepath.parent_path()))  fs::create_directories(filepath.parent_path());
    int i = 1;
    std::string stem = filepath.stem().string();
    while(fs::exists(filepath)){    // This will increment i until it hits a unique name
      i++;
      std::stringstream ss;
      ss << stem << "_" << i << filepath.extension().string();
      fs::path newName(filepath.parent_path() / ss.str());
      filepath = newName;
    }
		out.open(filepath.string().c_str());
	}
}

SVDataSet::~SVDataSet() {
	close();
}

void SVDataSet::close() {
	if (open) {
		if (rank == 0) {
			out.close();
		}
		for (size_t i = 0, n = dataSources.size(); i < n; ++i) {
			delete dataSources[i];
		}
		dataSources.clear();

		for (size_t i = 0, n = vars.size(); i < n; ++i) {
			delete vars[i];
		}
		open = false;
	}
}

void SVDataSet::init() {
	if (rank == 0) {
		out << "\"tick\"";
		for (size_t i = 0; i < dataSources.size(); i++) {
			SVDataSource * ds = dataSources[i];
			SVDataSource::DataType type = ds->type();
			Variable* var;
			if (type == SVDataSource::INT)
				var = new IntVariable();
			else
				var = new DoubleVariable();
			vars.push_back(var);
			out << _separator << "\"" << ds->name() << "\"";
		}
		out << std::endl;
		out.flush();
	}
}

void SVDataSet::record() {
	if (!open) throw Repast_Error_28(); // Data set not open
	if (rank == 0) {
		ticks.push_back(_schedule->getCurrentTick());
	}
	for (size_t i = 0; i < dataSources.size(); i++) {
		dataSources[i]->record();
	}
}

void SVDataSet::write() {
	if (!open) throw Repast_Error_29();
	for (size_t i = 0; i < dataSources.size(); i++) {
		SVDataSource * ds = dataSources[i];
		Variable* var = 0;
		if (rank == 0) {
			var = vars[i];
		}
		ds->write(var);
	}

	if (rank == 0) {
		for (size_t ti = 0, k = ticks.size(); ti < k; ++ti) {
			out << ticks[ti];
			for (size_t i = 0, n = vars.size(); i < n; ++i) {
				Variable* var = vars[i];
				out << _separator;
				var->write(ti, out);
			}
			out << std::endl;
		}

		for (size_t i = 0, n = vars.size(); i < n; ++i) {
			vars[i]->clear();
		}
		out.flush();
	}

	ticks.clear();
}

}
