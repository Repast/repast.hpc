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
 * AgentExporter.cpp
 *
 *  Created on: Aug 19, 2010
 *      Author: nick
 */

#include <map>
#include "RepastProcess.h"
#include "AgentExporter.h"

using namespace std;

namespace repast {

// AgentExporter Implemementation
void AgentExporter::addExportedAgent(int importingProcess, AgentId id) {
	map<int, AgentRequest>::iterator iter = exportedMap.find(importingProcess);
	if (iter == exportedMap.end()) {
		AgentRequest req(importingProcess, RepastProcess::instance()->rank());
		req.addRequest(id);
		exportedMap[importingProcess] = req;
	} else {
		AgentRequest& request = iter->second;
		if (!request.containsInRequests(id))
			request.addRequest(id);
	}
}

void AgentExporter::updateExportedAgents(vector<AgentRequest>* reqs) {
	for (int i = 0, n = reqs->size(); i < n; i++) {
		updateExportedAgents((*reqs)[i]);
	}
}

void AgentExporter::updateExportedAgents(AgentRequest& req) {
	int requestingProc = req.sourceProcess();
	map<int, AgentRequest>::iterator iter = exportedMap.find(requestingProc);
		
	if (iter == exportedMap.end()) {
		AgentRequest request(requestingProc, RepastProcess::instance()->rank());
		exportedMap[requestingProc] = request;
	    iter = exportedMap.find(requestingProc);
	}
		
	iter->second.addAllRequests(req);

	vector<AgentId> cancellations = req.cancellations();
    for(vector<AgentId>::iterator agentIdIter = cancellations.begin(); agentIdIter != cancellations.end(); agentIdIter++){
        iter->second.removeRequest(*agentIdIter);
    }
	
  // The mapped request may now be empty
  if(iter->second.requestCountRequested() == 0) exportedMap.erase(requestingProc);
	
}
	
void AgentExporter::removeExport(const AgentId& id) {
	map<int, AgentRequest>::iterator iter;
	for (iter = exportedMap.begin(); iter != exportedMap.end(); ) {
		AgentRequest& req = iter->second;
		if (req.removeRequest(id) && req.requestCountRequested() == 0) {
			exportedMap.erase(iter++);
		} else {
			++iter;
		}
	}
}

void AgentExporter::removeExport(int importerProcess, AgentId& id){
	map<int, AgentRequest>::iterator iter = exportedMap.find(importerProcess);
	if(iter != exportedMap.end()){
		iter->second.removeRequest(id); // Fails silently
		if(iter->second.requestCount() == 0) exportedMap.erase(importerProcess);
	}
}
	
void AgentExporter::sendStatus(boost::mpi::communicator& comm, vector<boost::mpi::request>& requests) {

	map<int, AgentRequest>::iterator iter;
	for (iter = exportedMap.begin(); iter != exportedMap.end();) {
		int importingProcess = iter->first;
		map<int, vector<AgentStatus> >::iterator statusIter = stateMap.find(importingProcess);
		if (statusIter == stateMap.end()) {
			requests.push_back(comm.isend(importingProcess, AGENT_SYNC_STATUS, emptyStatus));
		} else {
			requests.push_back(comm.isend(importingProcess, AGENT_SYNC_STATUS, statusIter->second));
		}

		// if there is nothing to send to the requesting process then
		// remove that entry from the map. We do that here because a
		// move can decrement the request count but we still want that process id
		// in the map so we can iterate over it to let it now about status changes
		if (iter->second.requestCount() == 0) {
			exportedMap.erase(iter++);
		} else {
			++iter;
		}
	}
}

void AgentExporter::agentRemoved(const AgentId& id) {
	map<int, AgentRequest>::iterator iter;
	for (iter = exportedMap.begin(); iter != exportedMap.end(); ++iter) {
		AgentRequest& req = iter->second;
		bool removed = req.removeRequest(id);
		if (removed) {
			// need to inform process (key) that the agent has been removed
			AgentStatus status(id);
			map<int, vector<AgentStatus> >::iterator stIter = stateMap.find(req.sourceProcess());
			if (stIter == stateMap.end()) {
				vector<AgentStatus> vec;
				vec.push_back(status);
				stateMap[req.sourceProcess()] = vec;
			} else {
				stIter->second.push_back(status);
			}
		}
	}
}

// id moved to process
void AgentExporter::agentMoved(const AgentId& id, int process, vector<AgentRequest>& exportUpdates) {
	map<int, AgentRequest>::iterator iter;
	for (iter = exportedMap.begin(); iter != exportedMap.end(); ++iter) {
		AgentRequest& req = iter->second;
		bool removed = req.removeRequest(id);
		if (removed) {
			addExportUpdate(exportUpdates, req.sourceProcess(), req.targetProcess(), id);
			// need to inform process (key) that the agent has been moved
			AgentId newId(id);
			newId.currentRank(process);
			AgentStatus status(id, newId);
			map<int, vector<AgentStatus> >::iterator stIter = stateMap.find(req.sourceProcess());
			if (stIter == stateMap.end()) {
				vector<AgentStatus> vec;
				vec.push_back(status);
				stateMap[req.sourceProcess()] = vec;
			} else {
				stIter->second.push_back(status);
			}
		}
	}
}

void AgentExporter::addExportUpdate(vector<AgentRequest>& exportUpdates, int source, int target, const AgentId& id) {
	bool found = false;
	for (size_t i = 0; i < exportUpdates.size(); i++) {
		AgentRequest& req = exportUpdates[i];
		if (req.sourceProcess() == source && req.targetProcess() == target) {
			req.addRequest(id);
			found = true;
			break;
		}
	}

	if (!found) {
		AgentRequest newReq(source, target);
		newReq.addRequest(id);
		exportUpdates.push_back(newReq);
	}
}

void AgentExporter::clearStateMap(){
  stateMap.clear();
}

}
