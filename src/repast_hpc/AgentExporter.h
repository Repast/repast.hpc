/*
 *Repast for High Performance Computing (Repast HPC)
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
 * AgentExporter.h
 *
 *  Created on: Aug 19, 2010
 *      Author: nick
 */

#ifndef AGENTEXPORTER_H_
#define AGENTEXPORTER_H_

#include <boost/mpi/communicator.hpp>
#include <vector>

#include "AgentStatus.h"
#include "AgentRequest.h"

namespace repast {

/**
 * Data structure and associated functions that maintain the record of agents that
 * have been 'requested' by other processes. When agents are synchronized across
 * processes, this data structure is referenced to determine which agents' information
 * must be sent, and to which other processes it is to be sent. Functions of this
 * data structure manage updates in cases where an agent on Process A that is being
 * exported to Processes X, Y, and Z is moved to Process B. The ego-centric view
 * is that this data structure can export information about agents that it is no
 * longer responsible for (by sending information to B that it needs to export
 * the agent to X, Y, and Z, and to X, Y, and Z that the agent will now be exported
 * from B and not A). This works in conjunction with AgentImporter.
 *
 * NON public API.
 *
 */
class AgentExporter {
private:
	template<typename Content, typename Provider>
	friend void _synchAgents(Provider& provider, int tag, std::vector<std::vector<Content>*>& contents);

	// map of agent requests by requesting process (source)
	std::map<int, AgentRequest> exportedMap;

	// key: request process rank, value: list of agent state updates to send to that process.
	std::map<int, std::vector<AgentStatus> > stateMap;


	// Dummy collection; empty
	std::vector<AgentStatus> emptyStatus;

	/**
	 * Given an agent request from a requesting process, updates the record of which agents
	 * are being exported (and to where) by adding newly requested agents and removing
	 * cancellations
	 *
	 * @param req AgentRequest to be processed
	 */
	void updateExportedAgents(AgentRequest& req);

	/**
	 * Given an AgentID, a source and a target, adds the ID to the collection
	 * of updates
	 *
	 * @param exportUpdates AgentRequest collection of updated information to which agent is to
	 * be added
	 * @param source source of the AgentRequest to which this agent ID is to be added
	 * @param target target of the AgentRequest to which this agent ID is to be added
	 * @param id the AgentId to be added
	 */
	void addExportUpdate(std::vector<AgentRequest>& exportUpdates, int source, int target, const AgentId& id);


public:

	/**
	 * Notifies this AgentExporter that it is exporting the specified
	 * agent to the specified process.
	 */
	void addExportedAgent(int importingProcess, AgentId id);

	/**
	 * Given a collection of incoming AgentRequest, updates the record of which agents
	 * are being exported (and to where) by adding newly requested agents
	 * and removing canceled ones.
	 *
	 * @param reqs the collection of AgentRequest objects to use to update
	 * the export map
	 */
	void updateExportedAgents(std::vector<AgentRequest>* reqs);

	/**
	 * Creates and initiates a collection of isends to transmit current agent status
	 * information (for agents that are being exported, to show if they have moved
	 * or been removed) to all of the agents' respective recipient processes
	 *
	 * @param comm communicator to use for send
	 * @param requests vector to contain requests that are created
	 */
	void sendStatus(boost::mpi::communicator& comm, std::vector<boost::mpi::request>& requests);

	/**
	 * Removes the specified agent from the set of agents that are exported
	 *
	 * @param id the id to be removed from the export
	 */
	void removeExport(const AgentId& id);

	/**
	 * Removes the specified agent from the set of agents that are exported to
	 * the specified process
	 *
	 * @param importerProcess the process to which information about the specified
	 * agent will no longer be sent
	 * @param id the id to be removed from the export
	 */
	void removeExport(int importerProcess, AgentId& id);
	
	/**
	 * Removes the agent from the list of agents to be exported and
	 * configures the appropriate state update.
	 *
	 * @param id ID of agent to be removed
	 */
	void agentRemoved(const AgentId& id);

	/**
	 * Removes the agent from the list of agents to be exported
	 * and configures a moved state upate.
	 *
	 * @param ID of agent to be moved
	 * @param int process to which the agent is moving
	 * @param exportUpdates collection of information that will be sent
	 * to processes that receive exports of this agent, informing them
	 * that the agent is moving to a new process
	 */
	void agentMoved(const AgentId& id, int process, std::vector<AgentRequest>& exportUpdates);

	/**
	 * Clears the stateMap data structure for reuse
	 */
	void clearStateMap();
};


}

#endif /* AGENTEXPORTER_H_ */
