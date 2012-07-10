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
 *  Created on: Jun 8, 2009
 *      Author: nick
 */

#include <sstream>

#include "NCDataSet.h"
#include "Utilities.h"
#include "io.h"

#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>

using namespace std;

namespace fs = boost::filesystem;

namespace repast {

NCDataSet::NCDataSet(std::string file, const Schedule& schedule) :
	schedule_(&schedule), start(0), open(true) {
  std::string filename = file;
	rank = RepastProcess::instance()->rank();
	if (rank == 0) {
		fs::path filepath(file);
		if (!fs::exists(filepath.parent_path())) 	fs::create_directories(filepath.parent_path());
    int i = 1;
    std::string stem = filepath.stem().string();
    while(fs::exists(filepath)){    // This will increment i until it hits a unique name
      i++;
      std::stringstream ss;
      ss << stem << "_" << i << filepath.extension().string();
      fs::path newName(filepath.parent_path() / ss.str());
      filepath = newName;
    }
    filename = filepath.string();
  }
	file_ = filename;
}

NCDataSet::~NCDataSet() {
	close();
}

void NCDataSet::close() {
	if (open) {
		for (size_t i = 0, n = dataSources.size(); i < n; i++) {
			NCDataSource* ds = dataSources[i];
			delete ds;
		}
		if (rank == 0) {
			ncfile->close();
			delete ncfile;
		}
		open = false;
	}
}

// DataSet implementation
void NCDataSet::record() {
	//Timer timer;
	//timer.start();
	if (rank == 0) {
		ticks.push_back(schedule_->getCurrentTick());
	}
	for (size_t i = 0; i < dataSources.size(); i++) {
		dataSources[i]->record();
	}
	//Log4CL::instance()->get_logger("root").log(INFO, "dataset record, time: " + boost::lexical_cast<std::string>(timer.stop()));
}

void NCDataSet::write() {
	//Timer timer;
	//timer.start();
	if (rank == 0) {
		NcVar* tickVar = ncfile->get_var("tick");
		tickVar->set_cur(start);
		tickVar->put(&ticks[0], ticks.size());
		start += ticks.size();

		ticks.clear();
	}

	for (size_t i = 0; i < dataSources.size(); i++) {
		NCDataSource * ds = dataSources[i];
		NcVar* var = 0;
		if (rank == 0)
			var = ncfile->get_var(ds->name().c_str());
		ds->write(var);
	}

	//Log4CL::instance()->get_logger("root").log(INFO, "dataset write, time: " + boost::lexical_cast<std::string>(timer.stop()));
}

}

