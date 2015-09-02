#include <stdio.h>
#include <vector>
#include <boost/mpi.hpp>
#include "repast_hpc/RepastProcess.h"
#include "repast_hpc/Utilities.h"
#include "repast_hpc/Properties.h"

class RepastHPCDemoModel{
	int stopAt;
	repast::Properties* props;
public:
	RepastHPCDemoModel(std::string propsFile, int argc, char** argv, boost::mpi::communicator* comm){
		props = new repast::Properties(propsFile, argc, argv, comm);
		stopAt = repast::strToInt(props->getProperty("stop.at"));
		if(repast::RepastProcess::instance()->rank() == 0) props->writeToSVFile("record.csv");
	}
	~RepastHPCDemoModel(){
		delete props;
	}
	void init(){}
	void doSomething(){
		std::cout << "Rank " << repast::RepastProcess::instance()->rank() << " is doing something." << std::endl;
	}
	void initSchedule(repast::ScheduleRunner& runner){
		runner.scheduleEvent(1, 1, repast::Schedule::FunctorPtr(new repast::MethodFunctor<RepastHPCDemoModel> (this, &RepastHPCDemoModel::doSomething)));
		runner.scheduleEndEvent(repast::Schedule::FunctorPtr(new repast::MethodFunctor<RepastHPCDemoModel> (this, &RepastHPCDemoModel::recordResults)));
		runner.scheduleStop(stopAt);
	}
	void recordResults(){
		if(repast::RepastProcess::instance()->rank() == 0){
			props->putProperty("Result","Passed");
			std::vector<std::string> keyOrder;
			keyOrder.push_back("RunNumber");
			keyOrder.push_back("stop.at");
			keyOrder.push_back("Result");
			props->writeToSVFile("results.csv", keyOrder);
	    }
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