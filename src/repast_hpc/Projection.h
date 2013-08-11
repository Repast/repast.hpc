/*
 *   Repast for High Performance Computing (Repast HPC)
 *
 *   Copyright (c) 2010 Argonne National Laboratory
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with
 *   or without modification, are permitted provided that the following
 *   conditions are met:
 *
 *     Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *
 *     Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *
 *     Neither the name of the Argonne National Laboratory nor the names of its
 *     contributors may be used to endorse or promote products derived from
 *     this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 *   PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE TRUSTEES OR
 *   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 *   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 *   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 *   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 *   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 *   EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *
 *  Projection.h
 *
 *  Created on: Aug 5, 2010
 *      Author: nick
 */

#ifndef PROJECTION_H_
#define PROJECTION_H_

#include <sstream>
#include <string>
#include <set>
#include <map>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/set.hpp>

#include "AgentId.h"

namespace repast {


/**
 * Serializable packet that can contain projection
 * information regardless of the type of projection
 * (network or spatial).
 */
class ProjectionInfoPacket{
  friend class boost::serialization::access;

public:
  ProjectionInfoPacket(){} // For serialization
  ProjectionInfoPacket(AgentId agentId): id(agentId){}
  virtual ~ProjectionInfoPacket(){}

  template<class Archive>
  void serialize(Archive& ar, const unsigned int version) {
    ar & id;
  }

  AgentId id;

  virtual bool isEmpty(){ return false; }

};

BOOST_SERIALIZATION_ASSUME_ABSTRACT(ProjectionInfoPacket);

/**
 * Serializable packet that can contain projection information
 * of a specific kind using the template parameter.
 */
template<typename Datum>
class SpecializedProjectionInfoPacket: public ProjectionInfoPacket{
  friend class boost::serialization::access;

public:

  SpecializedProjectionInfoPacket(){} // For serialization
  SpecializedProjectionInfoPacket(AgentId agentId): ProjectionInfoPacket(agentId){}
  SpecializedProjectionInfoPacket(AgentId agentId, std::vector<Datum> projectionData): ProjectionInfoPacket(agentId){
    data.assign(projectionData.begin(), projectionData.end());
  }
  SpecializedProjectionInfoPacket(AgentId agentId, std::set<Datum> projectionData): ProjectionInfoPacket(agentId){
    data.assign(projectionData.begin(), projectionData.end());
  }

  ~SpecializedProjectionInfoPacket(){}

  template<class Archive>
  void serialize(Archive& ar, const unsigned int version) {
    ar & boost::serialization::base_object<ProjectionInfoPacket>(*this);
    ar & data;
  }

  std::vector<Datum> data;

  virtual bool isEmpty(){ return (data.size() == 0); }

};

template<typename T>
class Context;

/**
 * Abstract base class for all Projections.
 */
template<typename T>
class Projection : public boost::noncopyable {

	friend class Context<T> ;

protected:

	std::string name_;
	virtual bool addAgent(boost::shared_ptr<T> agent) = 0;
	virtual void removeAgent(T* agent) = 0;


	// Beta (Protected)
	std::set<int> filter;


  virtual ProjectionInfoPacket* getProjectionInfo(AgentId id, bool secondaryInfo = false, std::set<AgentId>* secondaryIds = 0, int destProc = -1 ) = 0;

  virtual void updateProjectionInfo(ProjectionInfoPacket* pip, Context<T>* context) = 0;

public:

  enum RADIUS{ PRIMARY, SECONDARY };

	/**
	 * Creates a projection with specified name.
	 *
	 * @param name the name of the projection. This must be unique
	 * across projections
	 */
	Projection(std::string name) :
		name_(name) {
	}

	virtual ~Projection() {
	}

	/**
	 * Gets the name of this projection.
	 */
	const std::string name() const {
		return name_;
	}

	// Beta (public)

	/**
	 * Adds an entry to the list of agent types that can be added to this projection.
	 *
	 * Note: no indication if type is already listed
	 *
	 * @param type type to be added
	 */
	void addFilterVal(int type){
	  filter.insert(type);
	}

	/**
	 * Removes an entry from the list of agent types that can be added to this projection.
	 *
	 * Note: no indication if type is not listed
	 *
	 * @param type entry to be removed
	 */
	void removeFilterVal(int type){
	  filter.erase(type);
	}

	/**
	 * Clears the list of agent types that can be added to this projection; the result
	 * is that the filter is empty, and any agent can be added.
	 */
	void clearFilter(){
	  filter.clear();
	}

	/**
	 * Returns true if the agent can be added to the projection, which will
	 * be the case if the filter list is empty or if the agent's type is in the
	 * filter list.
	 *
	 * @param agent pointer to the agent to be tested
	 */
	bool agentCanBeAdded(boost::shared_ptr<T> agent){
	  return ( (filter.size() == 0) ||
	           (filter.find(agent->getId().agentType()) != filter.end()));
	}

	/**
	 * Should return true if the Projection implemented can 'keep' some (non-local)
	 * agents during a projection information synchronization operation. Generally
	 * spaces will allow all non-local agents to be deleted, but graphs keep the
	 * non-local agents that participate in Master edges.
	 *
	 * It is possible to override these. A graph projection can be created that does
	 * not permit non-local agents to be 'kept'. This would be an extremely unusual
	 * use case, but it is possible.
	 *
	 * Note that these are used for optimization. If no projection in a given context
	 * keeps any agents, several steps in the synchronization algorithm can be omitted.
	 * Of course, omitting these steps when a projection actually retains agents can
	 * caused undefined problems.
	 *
	 * @return true if this projection will keep non-local agents during a projection
	 * information synchronziation event, false if it will not.
	 */
  virtual bool keepsAgentsOnSyncProj() = 0;

  /**
   * Should return true if the Projection implemented will send secondary agents during
   * a status exchange. Generally spaces do not and graphs do.
   *
   * If no secondary agents will be sent, portions of the algorithm can be omitted for
   * optimization.
   *
   * @return true if the Projection returns secondary agents, false if not
   */
  virtual bool sendsSecondaryAgentsOnStatusExchange() = 0;

  /**
   * Gets the set of processes with which this Projection exchanges projection info.
   * In the most general case this will be all other processors; this is the case
   * for graphs, where agent connections can be arbitrary. However, spaces usually
   * exchange information only with a small subset of 'neighbor' processes, which
   * is knowable in advance and constant. To accommodate the general case, the
   * algorithm for exchanging information must poll all other processes to see which
   * are sending to this one; if this is known in advance, this additional (expensive)
   * step can be skipped.
   */
  virtual void getInfoExchangePartners(std::set<int>& psToSendTo, std::set<int>& psToReceiveFrom) = 0;

  /**
   * Gets the set of processes with which this Projection exchanges agent status info-
   * that is, the set of processes from which agents can move to this one or to which
   * they can move when moving from this one. In the most general case this will
   * be all other processors. However, simulations where agents move in spaces  will
   * usually exchange agents only with a small subset of 'neighbor' processes, which
   * is knowable in advance and constant. To accommodate the general case, the
   * algorithm for exchanging information must poll all other processes to see which
   * are sending to this one; if this is known in advance, this additional (expensive)
   * step can be skipped.
   */
  virtual void getAgentStatusExchangePartners(std::set<int>& psToSendTo, std::set<int>& psToReceiveFrom) = 0;

  /**
   * Given a set of agents to test, gets the subset that must be kept in order to fulfill the projection's
   * 'contract' to the specified radius. Generally spaces do not require any agents, but graphs
   * do- generally the non-local ends to master copies of edges.
   */
  virtual void getRequiredAgents(std::set<AgentId>& agentsToTest, std::set<AgentId>& agentsRequired, RADIUS radius = PRIMARY) = 0;

  /**
   * Given a set of agents, gets the agents that this projection implementation must 'push' to
   * other processes. Generally spaces must push agents that are in 'buffer zones' and graphs
   * must push local agents that are vertices to master edges where the other vertex is non-
   * local. The results are returned per-process in the agentsToPush map.
   */
  virtual void getAgentsToPush(std::set<AgentId>& agentsToTest, std::map<int, std::set<AgentId> >& agentsToPush) = 0;

  // Note: Virtual because some child classes may be able to short-circuit this (like Graphs)
  /**
   * Convenience wrapper that gets all of the projection information for the agents specified
   * (calls implementation in child class that gets only the information for one agent).
   */
  virtual void getProjectionInfo(std::vector<AgentId>& agents, std::vector<ProjectionInfoPacket*>& packets,
      bool secondaryInfo = false, std::set<AgentId>* secondaryIds = 0, int destProc = -1);

  /**
   * Updates the projection information for the agents in this projection according to
   * the information contained in the vector of information packets passed.
   */
  void updateProjectionInfo(std::vector<ProjectionInfoPacket*>& pips, Context<T>* context);

  virtual void cleanProjectionInfo(std::set<AgentId>& agentsToKeep) = 0;

  virtual void balance(){};

};


template<typename T>
void Projection<T>::getProjectionInfo(std::vector<AgentId>& agents, std::vector<ProjectionInfoPacket*>& packets,
    bool secondaryInfo, std::set<AgentId>* secondaryIds, int destProc){
  for(std::vector<AgentId>::const_iterator iter = agents.begin(), iterEnd = agents.end(); iter != iterEnd; iter++){
    ProjectionInfoPacket* packet = getProjectionInfo((*iter), secondaryInfo, secondaryIds, destProc);
    if((packet != 0) && (!packet->isEmpty())) packets.push_back(packet);
  }
}

template<typename T>
void Projection<T>::updateProjectionInfo(std::vector<ProjectionInfoPacket*>& pips, Context<T>* context){
  for(std::vector<ProjectionInfoPacket*>::const_iterator pipIter = pips.begin(), pipIterEnd = pips.end(); pipIter != pipIterEnd; pipIter++){
      updateProjectionInfo(*pipIter, context);
  }
}

}

#endif /* PROJECTION_H_ */
