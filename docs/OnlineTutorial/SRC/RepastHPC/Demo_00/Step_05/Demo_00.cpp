#include <stdio.h>
#include <boost/mpi.hpp>
#include "RepastProcess.h"

int main(int argc, char** argv){
	
	std::string configFile = argv[1]; // The name of the configuration file is Arg 1
	
	boost::mpi::environment env(argc, argv);
	boost::mpi::communicator world;

	repast::RepastProcess::init(configFile);
	
	repast::RepastProcess::instance()->done();
}