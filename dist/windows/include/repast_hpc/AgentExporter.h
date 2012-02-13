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
*/

/*
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
 * Exports agents from one RepastProcess to another. NON public API.
 */
class AgentExporter {
private:
	template<typename Content, typename Provider>
	friend void _synchAgents(Provider& provider, int tag, std::vector<std::vector<Content>*>& contents);

	// map of agent requests by requesting process (source)
	std::map<int, AgentRequest> exportedMap;
	// key: request process rank, value: list of agent state updates to
	// send to that process.
	std::map<int, std::vector<AgentStatus> > stateMap;

	void updateExportedAgents(AgentRequest& req);
	void addExportUpdate(std::vector<AgentRequest>& exportUpdates, int source, int target, const AgentId& id);

public:

	/**
	 * Notifies this AgentExporter that it is exporting the specified
	 * agent to the specified process.
	 */
	void addExportedAgent(int importingProcess, AgentId id);
	void updateExportedAgents(std::vector<AgentRequest>* reqs);

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
	 */
	void agentRemoved(const AgentId& id);

	/**
	 * Removes the agent from the list of agents to be exporeted
	 * and configures a moved state upate.
	 */
	void agentMoved(const AgentId& id, int process, std::vector<AgentRequest>& importers);
};


}

#endif /* AGENTEXPORTER_H_ */
