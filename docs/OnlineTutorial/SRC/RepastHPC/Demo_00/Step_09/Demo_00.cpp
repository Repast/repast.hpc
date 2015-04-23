#include <stdio.h>
#include <boost/mpi.hpp>
#include "RepastProcess.h"

class RepastHPCDemoModel{
public:
	RepastHPCDemoModel(){}
	~RepastHPCDemoModel(){}
	void init(){}
	void doSomething(){
		std::cout << "Rank " << repast::RepastProcess::instance()->rank() << " is doing something." << std::endl;
	}
	void initSchedule(repast::ScheduleRunner& runner){
		runner.scheduleEvent(1, 1, repast::Schedule::FunctorPtr(new repast::MethodFunctor<RepastHPCDemoModel> (this, &RepastHPCDemoModel::doSomething)));
		runner.scheduleStop(2);
	}
};

int main(int argc, char** argv){
	
	std::string configFile = argv[1]; // The name of the configuration file is Arg 1
	
	boost::mpi::environment env(argc, argv);
	boost::mpi::communicator world;

	repast::RepastProcess::init(configFile);
	
	RepastHPCDemoModel* model = new RepastHPCDemoModel();
	repast::ScheduleRunner& runner = repast::RepastProcess::instance()->getScheduleRunner();
	
	model->init();
	model->initSchedule(runner);
	
	runner.run();
	
	delete model;
	
	repast::RepastProcess::instance()->done();
	
}