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
 */

#include <boost/mpi.hpp>

#include "repast_hpc/io.h"
#include "repast_hpc/RepastProcess.h"
#include "relogo/SimulationRunner.h"
#include "relogo/Patch.h"
//#include "repast_hpc/initialize_random.h"

#include "ZombieObserver.h"

using namespace repast;
using namespace relogo;


void usage() {
	std::cerr << "usage: X  string string" << std::endl;
	std::cerr << "  first string: string is the path to the Repast HPC \n\tconfiguration properties file"
			<< std::endl;
	std::cerr << "  second string: string is the path to the model properties file" << std::endl;
}

void runZombies(std::string propsFile, int argc, char ** argv) {
  boost::mpi::communicator world;
	Properties props(propsFile, argc, argv, &world);

  std::string time;
  repast::timestamp(time);
  props.putProperty("date_time.run", time);

  props.putProperty("process.count", world.size());

	SimulationRunner runner(&world);

	if(world.rank() == 0) std::cout << " Starting... " << std::endl;
	repast::Timer timer;
	timer.start();
	runner.run<ZombieObserver, Patch>(props);

  props.putProperty("run.time", timer.stop());


  if(world.rank() == 0){
    std::vector<std::string> keysToWrite;
    keysToWrite.push_back("run.number");
    keysToWrite.push_back("date_time.run");
    keysToWrite.push_back("process.count");
    keysToWrite.push_back("random.seed");
    keysToWrite.push_back("human.count");
    keysToWrite.push_back("zombie.count");
    keysToWrite.push_back("min.x");
    keysToWrite.push_back("min.y");
    keysToWrite.push_back("max.x");
    keysToWrite.push_back("max.y");
    keysToWrite.push_back("proc.per.x");
    keysToWrite.push_back("proc.per.y");
    keysToWrite.push_back("grid.buffer");
    keysToWrite.push_back("init.time");
    keysToWrite.push_back("stop.at");
    keysToWrite.push_back("run.time");
    props.log("root");
    props.writeToSVFile("Zombies.csv", keysToWrite);
  }



}

int main(int argc, char **argv) {
	boost::mpi::environment env(argc, argv);
	std::string config, props;
	boost::mpi::communicator world;

	if (argc >= 3) {
		config = argv[1];
		props = argv[2];

	} else {
		if (world.rank() == 0) usage();
		return 0;
	}

	if (config.size() > 0 && props.size() > 0) {
		RepastProcess::init(config, &world);
		runZombies(props, argc, argv);
	} else {
		if (world.rank() == 0) usage();
		return 0;
	}

	RepastProcess::instance()->done();
	return 0;
}

