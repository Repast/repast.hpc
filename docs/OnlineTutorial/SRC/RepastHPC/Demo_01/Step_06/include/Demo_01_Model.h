/* Demo_01_Model.h */

#ifndef DEMO_01_MODEL
#define DEMO_01_MODEL

#include <boost/mpi.hpp>
#include "Schedule.h"
#include "Properties.h"
#include "SharedContext.h"

#include "Demo_01_Agent.h"

class RepastHPCDemoModel{
	int stopAt;
	int countOfAgents;
	repast::Properties* props;
	repast::SharedContext<RepastHPCDemoAgent> context;
public:
	RepastHPCDemoModel(std::string propsFile, int argc, char** argv, boost::mpi::communicator* comm);
	~RepastHPCDemoModel();
	void init();
	void doSomething();
	void initSchedule(repast::ScheduleRunner& runner);
	void recordResults();
};

#endif
