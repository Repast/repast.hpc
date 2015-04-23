#include <stdio.h>
#include <boost/mpi.hpp>
#include "RepastProcess.h"
#include "Utilities.h"
#include "Properties.h"

class RepastHPCDemoModel{
	int stopAt;
public:
	RepastHPCDemoModel(std::string propsFile, int argc, char** argv, boost::mpi::communicator* comm){
		repast::Properties props(propsFile, argc, argv, comm);
		stopAt = repast::strToInt(props.getProperty("stop.at"));
	}
	~RepastHPCDemoModel(){}
	void init(){}
	void doSomething(){
		std::cout << "Rank " << repast::RepastProcess::instance()->rank() << " is doing something." << std::endl;
	}
	void initSchedule(repast::ScheduleRunner& runner){
		runner.scheduleEvent(1, 1, repast::Schedule::FunctorPtr(new repast::MethodFunctor<RepastHPCDemoModel> (this, &RepastHPCDemoModel::doSomething)));
		runner.scheduleStop(stopAt);
	}
};

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