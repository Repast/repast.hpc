/* Demo_02_Model.h */

#ifndef DEMO_02_MODEL
#define DEMO_02_MODEL

#include <boost/mpi.hpp>
#include "repast_hpc/Schedule.h"
#include "repast_hpc/Properties.h"
#include "repast_hpc/SharedContext.h"
#include "repast_hpc/AgentRequest.h"
#include "repast_hpc/TDataSource.h"
#include "repast_hpc/SVDataSet.h"
#include "repast_hpc/SharedNetwork.h"

#include "Demo_02_Agent.h"

/* Agent Package Provider */
class RepastHPCDemoAgentPackageProvider {
	
private:
    repast::SharedContext<RepastHPCDemoAgent>* agents;
	
public:
	
    RepastHPCDemoAgentPackageProvider(repast::SharedContext<RepastHPCDemoAgent>* agentPtr);
	
    void providePackage(RepastHPCDemoAgent * agent, std::vector<RepastHPCDemoAgentPackage>& out);
	
    void provideContent(repast::AgentRequest req, std::vector<RepastHPCDemoAgentPackage>& out);
	
};

/* Agent Package Receiver */
class RepastHPCDemoAgentPackageReceiver {
	
private:
    repast::SharedContext<RepastHPCDemoAgent>* agents;
	
public:
	
    RepastHPCDemoAgentPackageReceiver(repast::SharedContext<RepastHPCDemoAgent>* agentPtr);
	
    RepastHPCDemoAgent * createAgent(RepastHPCDemoAgentPackage package);
	
    void updateAgent(RepastHPCDemoAgentPackage package);
	
};


/* Data Collection */
class DataSource_AgentTotals : public repast::TDataSource<int>{
private:
	repast::SharedContext<RepastHPCDemoAgent>* context;

public:
	DataSource_AgentTotals(repast::SharedContext<RepastHPCDemoAgent>* c);
	int getData();
};
	

class DataSource_AgentCTotals : public repast::TDataSource<int>{
private:
	repast::SharedContext<RepastHPCDemoAgent>* context;
	
public:
	DataSource_AgentCTotals(repast::SharedContext<RepastHPCDemoAgent>* c);
	int getData();
};

class RepastHPCDemoModel{
	int stopAt;
	int countOfAgents;
	repast::Properties* props;
	repast::SharedContext<RepastHPCDemoAgent> context;
	
	RepastHPCDemoAgentPackageProvider* provider;
	RepastHPCDemoAgentPackageReceiver* receiver;
    
    repast::RepastEdgeContentManager<RepastHPCDemoAgent> edgeContentManager;

	repast::SVDataSet* agentValues;
	repast::SharedNetwork<RepastHPCDemoAgent, repast::RepastEdge<RepastHPCDemoAgent>, repast::RepastEdgeContent<RepastHPCDemoAgent>, repast::RepastEdgeContentManager<RepastHPCDemoAgent> >* agentNetwork;
	
public:
	RepastHPCDemoModel(std::string propsFile, int argc, char** argv, boost::mpi::communicator* comm);
	~RepastHPCDemoModel();
	void init();
	void requestAgents();
    void connectAgentNetwork();
	void cancelAgentRequests();
	void removeLocalAgents();
	void moveAgents();
	void doSomething();
	void initSchedule(repast::ScheduleRunner& runner);
	void recordResults();
};

#endif
