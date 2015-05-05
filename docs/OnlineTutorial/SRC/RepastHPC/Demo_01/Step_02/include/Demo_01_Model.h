/* Demo_01_Model.h */

#ifndef DEMO_01_MODEL
#define DEMO_01_MODEL

#include <boost/mpi.hpp>
#include "repast_hpc/Schedule.h"
#include "repast_hpc/Properties.h"
#include "repast_hpc/SharedContext.h"

#include "Demo_01_Agent.h"

class RepastHPCDemoModel{
	int stopAt;
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
