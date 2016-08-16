/* Demo_04_Agent.cpp */

#include "Demo_04_Agent.h"

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

void RepastHPCDemoAgent::play(repast::SharedContext<RepastHPCDemoAgent>* context){
    std::set<RepastHPCDemoAgent*> agentsToPlay;
	
//    agentsToPlay.insert(this); // Prohibit playing against self
//    context->selectAgents(3, agentsToPlay, true);
	
    double cPayoff     = 0;
    double totalPayoff = 0;
    std::set<RepastHPCDemoAgent*>::iterator agentToPlay = agentsToPlay.begin();
    while(agentToPlay != agentsToPlay.end()){
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

void RepastHPCDemoAgent::move(repast::SharedDiscreteSpace<RepastHPCDemoAgent, repast::WrapAroundBorders, repast::SimpleAdder<RepastHPCDemoAgent> >* space){

    std::vector<int> agentLoc;
    space->getLocation(id_, agentLoc);
    std::vector<int> agentNewLoc;
    agentNewLoc.push_back(agentLoc[0] + (id_.id() < 7 ? -1 : 1));
    agentNewLoc.push_back(agentLoc[1] + (id_.id() > 3 ? -1 : 1));
    agentNewLoc.push_back(agentLoc[2] + (id_.id() > 5 ? -1 : 1));
    space->moveTo(id_,agentNewLoc);
    
}

void RepastHPCDemoAgent::processValues(repast::ValueLayerNDSU<double>* valueLayer, repast::SharedDiscreteSpace<RepastHPCDemoAgent, repast::WrapAroundBorders, repast::SimpleAdder<RepastHPCDemoAgent> >* space){
    std::vector<int> agentLoc;
    space->getLocation(id_, agentLoc);
      std::cout << "     " << id_ << " " << agentLoc[0] << "," << agentLoc[1] << "," << agentLoc[2] << " SPACE: " << space->dimensions() << std::endl;
    std::vector<int> agentNewLoc;
    agentNewLoc.push_back(agentLoc[0] + (id_.id() < 7 ? -1 : 1));
    agentNewLoc.push_back(agentLoc[1] + (id_.id() > 3 ? -1 : 1));
    agentNewLoc.push_back(agentLoc[2] + (id_.id() > 5 ? -1 : 1));

    bool errorFlag1 = false;
    bool errorFlag2 = false;
    double originalValue = valueLayer->getValueAt(agentLoc, errorFlag1);
    double nextValue = valueLayer->getValueAt(agentNewLoc, errorFlag2);
    if(errorFlag1 || errorFlag2){
      std::cout << "An error occurred for Agent " << id_ << " in RepastHPCDemoAgent::processValues()" << std::endl;
      std::cout << "     " << errorFlag1 << " " << errorFlag2 << std::endl;
      std::cout << "     " << id_ << " " << agentLoc[0] << "," << agentLoc[1] << "," << agentLoc[2] << std::endl;
      std::cout << "     " << valueLayer->getLocalBoundaries() << std::endl;
      return;
    }
    if(originalValue < nextValue && total > 0){                // If the likely next value is better
      valueLayer->addSecondaryValueAt(total-c, agentLoc, errorFlag1);   // Drop part of what you're carrying
        std::cout << "Agent " << id_ << " dropping " << (total - c) << " at " << agentLoc[0] << "," << agentLoc[1] << "," << agentLoc[2] << " for new value " << valueLayer->getSecondaryValueAt(agentLoc, errorFlag1) << std::endl;
      total = c;
      
    }
    else{                                                // Otherwise
      total += originalValue;                            // Pick up whatever was in the value layer
      valueLayer->setSecondaryValueAt(0, agentLoc, errorFlag2);  // Value Layer is now at zero
        std::cout << "Agent " << id_ << " picking up " << originalValue << " at " << agentLoc[0] << "," << agentLoc[1] << "," << agentLoc[2] << " for new value " << valueLayer->getSecondaryValueAt(agentLoc, errorFlag2)<< std::endl;
    }
}

/* Serializable Agent Package Data */

RepastHPCDemoAgentPackage::RepastHPCDemoAgentPackage(){ }

RepastHPCDemoAgentPackage::RepastHPCDemoAgentPackage(int _id, int _rank, int _type, int _currentRank, double _c, double _total):
id(_id), rank(_rank), type(_type), currentRank(_currentRank), c(_c), total(_total){ }
