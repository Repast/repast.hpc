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
 *  	 Redistributions of source code must retain the above copyright notice,
 *  	 this list of conditions and the following disclaimer.
 *
 *  	 Redistributions in binary form must reproduce the above copyright notice,
 *  	 this list of conditions and the following disclaimer in the documentation
 *  	 and/or other materials provided with the distribution.
 *
 *  	 Neither the name of the Argonne National Laboratory nor the names of its
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
 *  AgentId.h
 *
 *  Created on: Dec 23, 2008
 *      Author: nick
 */

#ifndef AGENTID_H_
#define AGENTID_H_

#include <iostream>
#include <boost/serialization/access.hpp>
#include <boost/mpi.hpp>

namespace repast {

/**
 * Agent identity information. An Agent ID consists of four values:
 * 1) a numerical identifier; 2) the process on which the agent
 * was created; 3) a numerical identifier that indicates the
 * agent's type (in simulation semantic terms, not a software object
 * type); and 4) the process on which the agent is a local agent.
 * Each agent should be uniquely identified by an AgentId using the
 * first three of the four values, which should be immutable. The
 * fourth value can change throughout the simulation.
 */
class AgentId {

	friend std::ostream& operator<<(std::ostream& os, const AgentId& id);
	friend bool operator==(const AgentId &one, const AgentId &two);
	friend bool operator<(const AgentId &one, const AgentId &two);
	friend class boost::serialization::access;

private:
	int id_, startProc_, agentType_, currentProc_;
	std::size_t hash;

	template<class Archive>
	void serialize(Archive& ar, const unsigned int version) {
		ar & id_;
		ar & startProc_;
		ar & agentType_;
		ar & currentProc_;
		ar & hash;
	}

public:

	/**
	 * No-arg constructor necessary for serialization.
	 */
	AgentId() {
	}

	/**
	 * Creates an AgentId. The combination of the first
	 * three parameters should uniquely identify the agent.
	 *
	 * @param id the agent's id
	 * @param startProc the rank of the agent's starting process
	 * @param agentType the agent's type (user defined)
	 * @param currentProc the rank where the agent is a local agent
	 */
	AgentId(int id, int startProc, int agentType, int currentProc = -1);

	virtual ~AgentId();

	/**
	 * Gets the id component of this AgentId.
	 *
	 * @return the id component of this AgentId.
	 */
	int id() const {
		return id_;
	}

	/**
	 * Gets the starting rank component of this AgentId.
	 *
	 * @return the starting rank component of this AgentId.
	 */
	int startingRank() const {
		return startProc_;
	}

	/**
	 * Gets the agent type component of this AgentId.
	 *
	 * @return the agent type component of this AgentId.
	 */
	int agentType() const {
		return agentType_;
	}

	/**
	 * Gets the current process rank of this AgentId. The current rank
	 * identifies which process the agent with this AgentId is
	 * currently on.
	 *
	 * @return the current process rank of this AgentId.
	 */
	int currentRank() const {
		return currentProc_;
	}

	/**
	 * Sets the current process rank of this AgentId. The current rank
	 * identifies which process the agent with this AgentId is
	 * currently on.
	 *
	 * @param val the current process rank
	 */
	void currentRank(int val) {
		currentProc_ = val;
	}

	/**
	 * Gets the hashcode for this AgentId.
	 *
	 * @return the hashcode for this AgentId.
	 */
	std::size_t hashcode() const {
		return hash;
	}
};

/**
 * Writes the agent id to the ostream.
 */
std::ostream& operator<<(std::ostream& os, const AgentId& id);

/**
 * Equality operator
 */
bool operator==(const AgentId &one, const AgentId &two);

/**
 * Inequality operator
 */
bool operator!=(const AgentId &one, const AgentId &two);

	
/**
 *  A comparison operator for use with std::set
 */
bool operator<(const AgentId &one, const AgentId &two);

/**
 * operator() implementation that returns the hashcode of
 * an AgentId.
 */
struct HashId {
	std::size_t operator()(const AgentId& id) const {
		return id.hashcode();
	}
};

/**
 * operator() implementation that returns the hashcode of
 * an agent via its AgentId.
 */
template<typename AgentType>
struct AgentHashId {
	std::size_t operator()(const AgentType* agent) const {
		return agent->getId().hashcode();
	}
};

/**
 * Interface for agent classes.
 *
 */
class Agent {

public:
	virtual ~Agent() {
	}

	/**
	 * Gets the AgentId for this Agent.
	 *
	 * @return the AgentId for this Agent.
	 */
	virtual AgentId& getId() = 0;

	/**
	 * Gets the AgentId for this Agent.
	 *
	 * @return the AgentId for this Agent.
	 */
	virtual const AgentId& getId() const = 0;
};


/**
 * Struct that allows filtering by Agent Type
 */
template<typename T>
struct IsAgentType {
	int _typeId;
	
	IsAgentType(){
		_typeId = 0; 
	}
	
	IsAgentType(int typeId) :
		_typeId(typeId) {
	}

	bool operator()(const boost::shared_ptr<T>& ptr) {
		return ptr->getId().agentType() == _typeId;
	}

	bool operator()(const T* agent) {
		return agent->getId().agentType() == _typeId;
	}
};


/**
 * Struct that allows filtering by !(Agent Type)
 */
template<typename T>
struct IsNotType {
	int _typeId;
	
	IsNotType(){
		_typeId = 0; 
	}
	
	IsNotType(int typeId) :
		_typeId(typeId) {
	}

	bool operator()(const boost::shared_ptr<T>& ptr) {
		return ptr->getId().agentType() != _typeId;
	}

	bool operator()(const T* agent) {
		return agent->getId().agentType() != _typeId;
	}

};

}

#endif /* AGENTID_H_ */
