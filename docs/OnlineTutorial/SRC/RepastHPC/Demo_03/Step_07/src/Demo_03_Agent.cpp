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
                              repast::SharedDiscreteSpace<RepastHPCDemoAgent, repast::WrapAroundBorders, repast::SimpleAdder<RepastHPCDemoAgent> >* discreteSpace,
                              repast::SharedContinuousSpace<RepastHPCDemoAgent, repast::WrapAroundBorders, repast::SimpleAdder<RepastHPCDemoAgent> >* continuousSpace,
                              repast::SharedContinuousSpace<RepastHPCDemoAgent, repast::StrictBorders, repast::SimpleAdder<RepastHPCDemoAgent> >* opinionSpace){
    std::vector<RepastHPCDemoAgent*> agentsToPlay;
    
    std::vector<int> agentLocDiscrete;
    discreteSpace->getLocation(id_, agentLocDiscrete);
    repast::Point<int> center(agentLocDiscrete);
    repast::Moore2DGridQuery<RepastHPCDemoAgent> moore2DQuery(discreteSpace);
    moore2DQuery.query(center, 2, false, agentsToPlay);
    
    std::vector<double> agentLocContinuous;
    continuousSpace->getLocation(id_, agentLocContinuous);
    repast::Point<double> agentPointContinuous(agentLocContinuous[0], agentLocContinuous[1]);

    
    std::vector<double>myOpinion;
    opinionSpace->getLocation(id_, myOpinion);
    repast::Point<double> myOpinionPoint(myOpinion[0], myOpinion[1]);

    
    double cPayoff     = 0;
    double totalPayoff = 0;
    std::vector<RepastHPCDemoAgent*>::iterator agentToPlay = agentsToPlay.begin();
    while(agentToPlay != agentsToPlay.end()){
        
        std::vector<double> otherLocContinuous;
        continuousSpace->getLocation((*agentToPlay)->getId(), otherLocContinuous);
        repast::Point<double> otherPointContinuous(otherLocContinuous[0], otherLocContinuous[1]);
        double distance = continuousSpace->getDistance(agentPointContinuous, otherPointContinuous);
        // Only play if within 1.5
        if(distance < 1.5){
            std::cout << " AGENT " << id_ << " AT " << agentPointContinuous << " PLAYING " << ((*agentToPlay)->getId().currentRank() == id_.currentRank() ? "LOCAL" : "NON-LOCAL") << " AGENT " << (*agentToPlay)->getId() << " at " << otherPointContinuous <<  " (distance = " << distance << " )" << std::endl;

            
            //bool iCooperated = cooperate();                          // Do I cooperate?
            std::vector<double> otherOpinion;
            opinionSpace->getLocation((*agentToPlay)->getId(), otherOpinion);
            repast::Point<double> otherOpinionPoint(otherOpinion[0], otherOpinion[1]);
            
            double opinionDistance = opinionSpace->getDistance(myOpinionPoint, otherOpinionPoint);
            bool iCooperated = (opinionDistance < 1); // Must be within 1 of opinion
            double payoff = (iCooperated ?
		    				 ((*agentToPlay)->cooperate() ?  7 : 1) :     // If I cooperated, did my opponent?
						 ((*agentToPlay)->cooperate() ? 10 : 3));     // If I didn't cooperate, did my opponent?
            if(iCooperated) cPayoff += payoff;
            totalPayoff             += payoff;
            std::cout << "  AGENT " << id_ << (iCooperated ? " COOPERATED " : " DID NOT COOPERATE ") << " with agent " << (*agentToPlay)->getId() << " because our opinion distance was " << opinionDistance << " FROM " << myOpinionPoint << " TO " << otherOpinionPoint << std::endl;
		}
        else{
            std::cout << " AGENT " << id_ << " AT " << agentPointContinuous << " NOT PLAYING " << ((*agentToPlay)->getId().currentRank() == id_.currentRank() ? "LOCAL" : "NON-LOCAL") << " AGENT "<< (*agentToPlay)->getId() << " at " << otherPointContinuous <<  " (distance = " << distance << " )" << std::endl;

        }
        agentToPlay++;
    }
    c      += cPayoff;
    total  += totalPayoff;
	
}

void RepastHPCDemoAgent::move(repast::SharedDiscreteSpace<RepastHPCDemoAgent, repast::WrapAroundBorders, repast::SimpleAdder<RepastHPCDemoAgent> >* discreteSpace,
                              repast::SharedContinuousSpace<RepastHPCDemoAgent, repast::WrapAroundBorders, repast::SimpleAdder<RepastHPCDemoAgent> >* continuousSpace){

    std::vector<double> agentLoc;
    continuousSpace->getLocation(id_, agentLoc);
    std::vector<double> agentNewLocContinuous;
    agentNewLocContinuous.push_back(agentLoc[0] + (repast::Random::instance()->nextDouble() - .5) * 1.5);
    agentNewLocContinuous.push_back(agentLoc[1] + (repast::Random::instance()->nextDouble() - .5) * 1.5);
    continuousSpace->moveTo(id_,agentNewLocContinuous);
    std::vector<int> agentNewLocDiscrete;
    agentNewLocDiscrete.push_back((int)(floor(agentNewLocContinuous[0])));
    agentNewLocDiscrete.push_back((int)(floor(agentNewLocContinuous[1])));
    discreteSpace->moveTo(id_, agentNewLocDiscrete);
}


/* Serializable Agent Package Data */

RepastHPCDemoAgentPackage::RepastHPCDemoAgentPackage(){ }

RepastHPCDemoAgentPackage::RepastHPCDemoAgentPackage(int _id, int _rank, int _type, int _currentRank, double _c, double _total):
id(_id), rank(_rank), type(_type), currentRank(_currentRank), c(_c), total(_total){ }
