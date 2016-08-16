/* Demo_03_Agent.cpp */

#include "Demo_03_Agent.h"
#include "repast_hpc/Moore2DGridQuery.h"
#include "repast_hpc/Point.h"

RepastHPCDemoAgent::RepastHPCDemoAgent(repast::AgentId id): id_(id), c(100), total(200){ }

RepastHPCDemoAgent::RepastHPCDemoAgent(repast::AgentId id, double newC, double newTotal): id_(id), c(newC), total(newTotal){ }

RepastHPCDemoAgent::~RepastHPCDemoAgent(){ }


void RepastHPCDemoAgent::set(int currentRank, double newC, double newTotal){
    id_.currentRank(currentRank);
    c     = newC;
    total = newTotal;
}

bool RepastHPCDemoAgent::cooperate(){
	return repast::Random::instance()->nextDouble() < c/total;
}

void RepastHPCDemoAgent::play(repast::SharedContext<RepastHPCDemoAgent>* context,
                              repast::SharedDiscreteSpace<RepastHPCDemoAgent, repast::StrictBorders, repast::SimpleAdder<RepastHPCDemoAgent> >* space){
    std::vector<RepastHPCDemoAgent*> agentsToPlay;
    
    std::vector<int> agentLoc;
    space->getLocation(id_, agentLoc);
    repast::Point<int> center(agentLoc);
    repast::Moore2DGridQuery<RepastHPCDemoAgent> moore2DQuery(space);
    moore2DQuery.query(center, 1, false, agentsToPlay);
    
    
    double cPayoff     = 0;
    double totalPayoff = 0;
    std::vector<RepastHPCDemoAgent*>::iterator agentToPlay = agentsToPlay.begin();
    while(agentToPlay != agentsToPlay.end()){
        std::vector<int> otherLoc;
        space->getLocation((*agentToPlay)->getId(), otherLoc);
        repast::Point<int> otherPoint(otherLoc);
        std::cout << " AGENT " << id_ << " AT " << center << " PLAYING " << ((*agentToPlay)->getId().currentRank() == id_.currentRank() ? "LOCAL" : "NON-LOCAL") << " AGENT " << (*agentToPlay)->getId() << " AT " << otherPoint << std::endl;
        bool iCooperated = cooperate();                          // Do I cooperate?
        double payoff = (iCooperated ?
						 ((*agentToPlay)->cooperate() ?  7 : 1) :     // If I cooperated, did my opponent?
						 ((*agentToPlay)->cooperate() ? 10 : 3));     // If I didn't cooperate, did my opponent?
        if(iCooperated) cPayoff += payoff;
        totalPayoff             += payoff;
		
        agentToPlay++;
    }
    c      += cPayoff;
    total  += totalPayoff;
	
}

void RepastHPCDemoAgent::move(repast::SharedDiscreteSpace<RepastHPCDemoAgent, repast::StrictBorders, repast::SimpleAdder<RepastHPCDemoAgent> >* space){

    std::vector<int> agentLoc;
    space->getLocation(id_, agentLoc);
    
    std::vector<int> agentNewLoc;
    do{
        agentNewLoc.clear();
        double xRand = repast::Random::instance()->nextDouble();
        double yRand = repast::Random::instance()->nextDouble();
        agentNewLoc.push_back(agentLoc[0] + (xRand < .33 ? -1 : (xRand < .66 ? 0 : 1)));
        agentNewLoc.push_back(agentLoc[1] + (yRand < .33 ? -1 : (yRand < .66 ? 0 : 1)));
        // Note: checking to see if agent would move outside GLOBAL bounds; exceeding local bounds is OK
        if(!space->bounds().contains(agentNewLoc)) std::cout << " INVALID: " << agentNewLoc[0] << "," << agentNewLoc[1] << std::endl;
        
    }while(!space->bounds().contains(agentNewLoc));
    
    space->moveTo(id_,agentNewLoc);
    
}


/* Serializable Agent Package Data */

RepastHPCDemoAgentPackage::RepastHPCDemoAgentPackage(){ }

RepastHPCDemoAgentPackage::RepastHPCDemoAgentPackage(int _id, int _rank, int _type, int _currentRank, double _c, double _total):
id(_id), rank(_rank), type(_type), currentRank(_currentRank), c(_c), total(_total){ }
