/* Demo_01_Agent.h */

#ifndef DEMO_01_AGENT
#define DEMO_01_AGENT

#include "repast_hpc/AgentId.h"
#include "repast_hpc/SharedContext.h"


/* Agents */
class RepastHPCDemoAgent{
	
private:
    repast::AgentId   id_;
    double              c;
    double          total;
	
public:
    RepastHPCDemoAgent(repast::AgentId id);
	
    RepastHPCDemoAgent(repast::AgentId id, double newC, double newTotal);
	
    ~RepastHPCDemoAgent();
	
    /* Required Getters */
    virtual repast::AgentId& getId(){                   return id_;    }
    virtual const repast::AgentId& getId() const {      return id_;    }
	
    /* Getters specific to this kind of Agent */
    double getC(){                                      return c;      }
    double getTotal(){                                  return total;  }
	
    /* Setter */
    void set(int currentRank, double newC, double newTotal);
	
    /* Actions */
    bool cooperate();                                                 // Will indicate whether the agent cooperates or not; probability determined by = c / total
    void play(repast::SharedContext<RepastHPCDemoAgent>* context);    // Choose three other agents from the given context and see if they cooperate or not
	
};

#endif