/*
 * Repast for High Performance Computing (Repast HPC)
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
 * AgentRequest.h
 *
 *  Created on: Aug 19, 2010
 *      Author: nick
 */

#ifndef AGENTREQUEST_H_
#define AGENTREQUEST_H_

#include <iostream>
#include <boost/serialization/access.hpp>
#include <boost/serialization/vector.hpp>
#include <set>

#include "AgentId.h"


namespace repast {

/**
 * Encapsulates a request made by one process for agents in another.
 *
 * Includes a list of requests and a list that represents cancellations
 * of previous requests.
 */
class AgentRequest {

	friend std::ostream& operator<<(std::ostream& os, const AgentRequest& request);
	friend class boost::serialization::access;

  // To allow AgentImporters to edit agent requests before sending them out
  friend class Importer_LIST;
  friend class Importer_SET;
  friend class Importer_MAP_int;

private:
	int source, target;
	std::vector<AgentId> requestedAgents_;
  std::vector<AgentId> cancellations_;

	template<class Archive>
	void serialize(Archive& ar, const unsigned int version) {
		ar & source;
		ar & target;
		ar & requestedAgents_;
		ar & cancellations_;
	}

public:
	// No-arg constructor for serialization
	AgentRequest() :
	source(-1), target(-1) {
	}

	/**
	 * Creates an AgentRequest that comes from the specified process.
	 *
	 * @param sourceProcess the rank of the process making the request
	 */
	AgentRequest(int sourceProcess);

	/**
	 * Creates an AgentRequest made from the source process to the target process.
	 * This can be used when all the requested agents reside on the same process
	 * (i.e. the target process).
	 *
	 * @param sourceProcess the rank of the source process
	 * @param targetProcess the rank of the target process
	 */
	AgentRequest(int sourceProcess, int targetProcess);

	/**
	 * Adds all the agent ids (both requests and cancellations)
	 * in req to this AgentRequest.
	 *
	 * @param req the AgentRequest to add all the agent ids from
	 */
	void addAll(const AgentRequest& req);
	
	/**
	 * Adds all the agent ids in req to this request,
	 * including only the ids that are requests and
	 * not those that are cancellations.
	 *
	 * @param req the AgentRequest to add all the agent ids from
	 */
	void addAllRequests(const AgentRequest& req);

	/**
	 * Adds all the agent ids in req to this request,
	 * including only the ids that are cancellations and
	 * not those that are requests.
	 *
	 * @param req the AgentRequest to add all the agent ids from
	 */
	void addAllCancellations(const AgentRequest& req);
		
	/**
	 * Gets a reference to the vector of requested agents.
	 *
	 * @return a reference to the vector of requested agents.
	 */
	const std::vector<AgentId>& requestedAgents() const {
		return requestedAgents_;
	}

	/**
	 * Gets a reference to the vector of cancellations.
	 *
	 * @return a reference to the vector of AgentIds representing
	 * cancellations.
	 */
	const std::vector<AgentId>& cancellations() const {
		return cancellations_;
	}
	
	/**
	 * Removes the specified id from the lists of requested agents,
	 * including both requests and cancellations.
	 *
	 * @param id the AgentId to be removed
	 * @param removeAllInstances if true (the default), all instances of the
	 * AgentId are removed; if false, only the first instance found
	 * is removed
	 *
	 * @return true if the id was found (in either list) and removed,
	 * otherwise false.
	 */
	bool remove(const AgentId& id, bool removeAllInstances = true);

	/**
	 * Removes the specified id from the list of agent requests;
	 * does not affect the list of cancellations.
	 *
	 * @param id the AgentId to be removed
   * @param removeAllInstances if true (the default), all instances of the
   * AgentId are removed; if false, only the first instance found
   * is removed
	 *
	 * @return true if the id was found in the list of requests
	 * and removed, otherwise false
	 */
	bool removeRequest(const AgentId& id, bool removeAllInstances = true);

	/**
	 * Removes the specified id from the list of agent request
	 * cancellations; does not affect the list of requests.
	 *
	 * @param id the AgentId to be removed
   * @param removeAllInstances if true (the default), all instances of the
   * AgentId are removed; if false, only the first instance found
   * is removed
	 *
	 * @return true if the id was found in the list of cancellations
	 * and removed, otherwise false
	 */
	bool removeCancellation(const AgentId& id, bool removeAllInstances = true);
	
	/**
	 * Puts the targets of all the requests into the set.
	 * Includes both the requests and the cancellations.
	 *
	 * @param targets set into which targets will be placed
	 */
	void targets(std::set<int>& targets);

	/**
	 * Puts the targets of all the requests into the set,
	 * including only the requests and not the cancellations.
	 *
	 * @param targets the set into which the targets will be placed
	 */
	void targetsOfRequests(std::set<int>& targets);
	
	/**
	 * Puts the targets of all the requests into the set,
	 * including only the requests and not the cancellations.
	 *
	 * @param targets the set into which the targets will be placed
	 */
	void targetsOfCancellations(std::set<int>& targets);	
	
	/**
	 * Adds the specified agent to the collection agents being
	 * requested.
	 *
	 * @param id the requested agent
	 */
	void addRequest(const AgentId& id);

	/**
	 * Adds the specified agent to the collection of agents
	 * for which a previous request is being cancelled.
	 *
	 * @param id the AgentId of the agent for which the
	 * request is being cancelled
	 */
	void addCancellation(const AgentId& id);
	
	/**
	 * Gets the number agents requested.
	 *
	 * Includes both requests and cancellations;
	 * exactly equivalent to 
	 *
	 * requestCountRequested() + requestCountCancellations()
	 *
	 * @return the number agents requested.
	 */
	int requestCount() const {
		return requestedAgents_.size() + cancellations_.size();
	}
	
	/**
	 * Gets the number of agents requested, counting
	 * only the requests and not the cancellations.
	 *
	 * @return the number of agents requested (requests only)
	 */
	int requestCountRequested() const {
	    return requestedAgents_.size();
	}
	
	/**
	 * Gets the number of agents requested, counting
	 * only the cancellations and not the requests.
	 *
	 * @return the number of agents requested (cancellations only)
	 */
	int requestCountCancellations() const {
	    return cancellations_.size();
	}
	
	/**
	 * Returns true if this AgentRequest contains a request for
	 * the specified id (either a request or a cancellation),
	 * otherwise false.
	 *
	 * @param id the id sought in the lists of requests and cancellations
	 *
	 * @return true if either the list of requests or the
	 * list of cancellations contains the specified id
	 */
	bool contains(const AgentId& id);

	/**
	 * Returns true if the list of requests contains the specified
	 * id (the list of cancellations is ignored)
	 *
	 * @param id the AgentId sought
	 *
	 * @return true if the specified AgentId is in the list of requests
	 */
	bool containsInRequests(const AgentId& id);
	
	/**
	 * Returns true if the list of cancellations contains the specified
	 * id (the list of requests is ignored)
	 *
	 * @param id the AgentId sought
	 *
	 * @return true if the specified AgentId is in the list of cancellations
	 */
	bool containsInCancellations(const AgentId& id);

	/**
	 * Gets the source process of these requests, that is,
	 * the process making the request.
	 *
	 * @return the process making the request
	 */
	int sourceProcess() const {
		return source;
	}

	/**
	 * If the requested agent ids are all on the same process
	 * then target process will identify that process. Otherwise
	 * this will return -1.
	 */
	int targetProcess() const {
		return target;
	}

};

/**
 * Prints the specified AgentRequest to the specified ostream.
 */
std::ostream& operator<<(std::ostream& os, const AgentRequest& request);


}

#endif /* AGENTREQUEST_H_ */
