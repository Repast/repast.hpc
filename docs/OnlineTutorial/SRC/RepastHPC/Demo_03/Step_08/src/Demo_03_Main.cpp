/* Demo_03_Main.cpp */

#include <boost/mpi.hpp>
#include "repast_hpc/RepastProcess.h"

#include "Demo_03_Model.h"


int main(int argc, char** argv){
	
	std::string configFile = argv[1]; // The name of the configuration file is Arg 1
	std::string propsFile  = argv[2]; // The name of the properties file is Arg 2
	
	boost::mpi::environment env(argc, argv);
	boost::mpi::communicator world;

	repast::RepastProcess::init(configFile);
	
	RepastHPCDemoModel* model = new RepastHPCDemoModel(propsFile, argc, argv, &world);
	repast::ScheduleRunner& runner = repast::RepastProcess::instance()->getScheduleRunner();
	
	model->init();
	model->initSchedule(runner);
	
	runner.run();
	
	delete model;
	
	repast::RepastProcess::instance()->done();
	
}