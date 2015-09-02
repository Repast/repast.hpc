#include <stdio.h>
#include <boost/mpi.hpp>
#include "repast_hpc/RepastProcess.h"

class RepastHPCDemoModel{
public:
	RepastHPCDemoModel(){}
	~RepastHPCDemoModel(){}
	void init(){}
};

int main(int argc, char** argv){
	
	std::string configFile = argv[1]; // The name of the configuration file is Arg 1
	
	boost::mpi::environment env(argc, argv);
	boost::mpi::communicator world;

	repast::RepastProcess::init(configFile);
	
	RepastHPCDemoModel* model = new RepastHPCDemoModel();
	
	model->init();
	
	delete model;
	
	repast::RepastProcess::instance()->done();
	
}