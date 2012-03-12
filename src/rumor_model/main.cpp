/*
 * Repast for High Performance Computing (Repast HPC)
 *
 *   Copyright (c) 2010 Argonne National Laboratory
 *   All rights reserved.
 *  
 *   Redistribution and use in source and binary forms, with 
 *   or without modification, are permitted provided that the following 
 *   conditions are met:
 *  
 *  	 Redistributions of source code must retain the above copyright notice,
 *  	 this list of conditions and the following disclaimer.
 *  
 *  	 Redistributions in binary form must reproduce the above copyright notice,
 *  	 this list of conditions and the following disclaimer in the documentation
 *  	 and/or other materials provided with the distribution.
 *  
 *  	 Neither the name of the Argonne National Laboratory nor the names of its
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
 */

#include "repast_hpc/RepastProcess.h"
#include "repast_hpc/logger.h"
#include "repast_hpc/Edge.h"
#include "repast_hpc/Utilities.h"

#include "RumorModel.h"

#include <boost/mpi.hpp>
#include <boost/serialization/export.hpp>
#include <boost/lexical_cast.hpp>
#include <vector>
#include <exception>

#ifndef _WIN32
#include <getopt.h>
#endif

#include <time.h>

namespace mpi = boost::mpi;
using namespace repast;

//BOOST_CLASS_EXPORT(Node)
//BOOST_CLASS_EXPORT(RepastEdge<Node>)

#ifndef _WIN32
void usage() {
	std::cerr << "usage: X -config string -properties string" << std::endl;
	std::cerr << "  config string: string is the path to the repast exascale \n\tconfiguration properties file"
			<< std::endl;
	std::cerr << "  properties string: string is the path to the model properties file" << std::endl;
}
#else
void usage() {
	std::cerr << "usage: X config properties" << std::endl;
	std::cerr << "  config = the path to the repast configuration file"
	<< std::endl;
	std::cerr << "  properties = the path to the model properties file" << std::endl;
}
#endif

void runRumorModel(std::string propsFile, int argc , char** argv) {
	mpi::communicator world;

  if (world.rank() == 0)
    Log4CL::instance()->get_logger("root").log(INFO, "Starting...");

  Timer timer_init;
  timer_init.start();

	RumorModel* rumorModel = new RumorModel(propsFile, argc, argv, &world);
	rumorModel->init();
	ScheduleRunner& runner = RepastProcess::instance()->getScheduleRunner();
	rumorModel->initSchedule(runner);

	world.barrier();
	std::string init_time = boost::lexical_cast<std::string>(timer_init.stop());

	if (world.rank() == 0)
		Log4CL::instance()->get_logger("root").log(INFO, "init, time: "	+ init_time);

	Timer timer_run;
	timer_run.start();
	runner.run();
	world.barrier();
	std::string run_time = boost::lexical_cast<std::string>(timer_run.stop());

	if (world.rank() == 0)
		Log4CL::instance()->get_logger("root").log(INFO, "run, time: " + run_time);

	if (world.rank() == 0)
	  rumorModel->writeProps("RumorModelRuns.csv", init_time, run_time);
	
	delete rumorModel;
}

int main(int argc, char **argv) {
	mpi::environment env(argc, argv);
	std::string config, props;

	mpi::communicator world;

  // First check: if there aren't enough arguments, warn the user and exit
   if (argc < 3) {
      usage();
      return -1;
   }

   // Second: Assume the first arg is config file name and the second is prop file name
   config = argv[1];
   props  = argv[2];

#ifndef _WIN32
   // Can use alternative --config ConfigFile --properties PropFile format (and other --switches) in Unix only:
   // This will re-set config and props if --config and/or --props is found
   static struct option long_options[] = { { "config", 1, 0, 0 }, { "properties", 1, 0, 0 }, { 0, 0, 0, 0 } };
   int optionIndex = 0;
   int c = 0;
   while ((c = getopt_long_only(argc, argv, "", long_options, &optionIndex)) != -1) {
     switch (c) {
       case 0:
         if (optionIndex == 0)
            config = optarg;
         else if (optionIndex == 1)
            props = optarg;
         break;
       default:
         usage();
     }
   }
#endif

	if (config.size() > 0 && props.size() > 0) {
		try {
			RepastProcess::init(config, &world);
			clock_t start = clock();
			runRumorModel(props, argc, argv);
			// need to make sure everyone has ended to get the total execution time
			world.barrier();
			clock_t end = clock();
			if (world.rank() == 0) {
				long double diff = end - start;
				Log4CL::instance()->get_logger("root").log(INFO, "total execution, time: " + boost::lexical_cast<
						std::string>(diff / CLOCKS_PER_SEC));
			}
		} catch (std::exception& ex) {
			std::cerr << "Error while running the rumor model: " << ex.what() << std::endl;
			throw ex;
		}
	} else {
		if (world.rank() == 0)
			usage();
	}
	RepastProcess::instance()->done();
	return 0;
}

