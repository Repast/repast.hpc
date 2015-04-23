#include <stdio.h>
#include <boost/mpi.hpp>
#include "RepastProcess.h"

class RepastHPCDemoModel{
public:
	RepastHPCDemoModel(){}
	~RepastHPCDemoModel(){}
	void init(){}
	void doSomething(){
		std::cout << "Rank " << repast::RepastProcess::instance()->rank() << " is doing something at " << 
		    repast::RepastProcess::instance()->getScheduleRunner().currentTick() << std::endl;
	}
	void doSomethingElse(){
		std::cout << "Rank " << repast::RepastProcess::instance()->rank() << " is doing something DIFFERENT at " << 
			repast::RepastProcess::instance()->getScheduleRunner().currentTick() << std::endl;
	}
	void initSchedule(repast::ScheduleRunner& runner){
		runner.scheduleEvent(1, 1, repast::Schedule::FunctorPtr(new repast::MethodFunctor<RepastHPCDemoModel> (this, &RepastHPCDemoModel::doSomething)));
		runner.scheduleEvent(2.2, 1, repast::Schedule::FunctorPtr(new repast::MethodFunctor<RepastHPCDemoModel> (this, &RepastHPCDemoModel::doSomething)));
		runner.scheduleEvent(2.3, 4, repast::Schedule::FunctorPtr(new repast::MethodFunctor<RepastHPCDemoModel> (this, &RepastHPCDemoModel::doSomething)));
		if(repast::RepastProcess::instance()->rank() == 0){
			runner.scheduleEvent(3.1, 2, repast::Schedule::FunctorPtr(new repast::MethodFunctor<RepastHPCDemoModel> (this, &RepastHPCDemoModel::doSomethingElse)));
		}
		if(repast::RepastProcess::instance()->rank() == 1){
			runner.scheduleEvent(5.5, repast::Schedule::FunctorPtr(new repast::MethodFunctor<RepastHPCDemoModel> (this, &RepastHPCDemoModel::doSomethingElse)));
		}
		runner.scheduleStop(10.9);
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