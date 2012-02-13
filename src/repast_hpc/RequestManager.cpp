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
 *  RequestManager.cpp
 *
 *  Created on: Aug 19, 2010
 *      Author: nick
 */

#include "RequestManager.h"
#include "mpi_constants.h"

namespace repast {

using namespace std;
namespace mpi = boost::mpi;

RequestManager::RequestManager(int processCount) :
	processCount_(processCount) {
	// fill the vector with 0s
	requestCounts_.resize(processCount_, 0);
}

RequestManager::~RequestManager() {
	map<int, vector<AgentRequest>*>::iterator iter;
	for (iter = requestMap.begin(); iter != requestMap.end(); iter++) {
		delete iter->second;
	}
}

void RequestManager::fillRequests(mpi::communicator& world, vector<vector<AgentRequest> >& reqs) {
	for (int i = 1; i < processCount_; i++) {
		map<int, vector<AgentRequest>*>::iterator iter = requestMap.find(i);
		if (iter != requestMap.end()) {
			reqs[i] = *(iter->second);
		}
	}
}

/*
 void RequestManager::sendRequests(mpi::communicator& world) {

 for (int i = 1; i < processCount_; i++) {
 map<int, vector<AgentRequest>*>::iterator iter = requestMap.find(i);
 if (iter == requestMap.end()) {
 // nothing requested from that process so send 0 count.
 Timer t;
 t.start();

 world.send(i, AGENT_TO_SEND_COUNT, 0);

 long double time = t.stop();
 Log4CL::instance()->get_logger("root").log(INFO, "requestAgents, sendRequests 0, time: "
 + boost::lexical_cast<std::string>(time));

 } else {
 vector<AgentRequest>* list = iter->second;
 Timer t;
 t.start();

 world.send(i, AGENT_TO_SEND_COUNT, list->size());
 long double time = t.stop();
 Log4CL::instance()->get_logger("root").log(INFO, "requestAgents, sendRequests size, time: "
 + boost::lexical_cast<std::string>(time));

 t.start();

 world.send(i, AGENT_TO_SEND_REQUEST, list);

 time = t.stop();
 Log4CL::instance()->get_logger("root").log(INFO, "requestAgents, sendRequests list, time: "
 + boost::lexical_cast<std::string>(time));
 }
 }
 }
 */

vector<AgentRequest>* RequestManager::requestsFor(int rank) {
	map<int, vector<AgentRequest>*>::iterator iter = requestMap.find(rank);
	if (iter == requestMap.end())
		return 0;
	return iter->second;
}

void RequestManager::mapRequest(AgentRequest& request) {
	int source = request.sourceProcess();
	vector<AgentId>::const_iterator agentId;
		
	const vector<AgentId>& requestedAgents = request.requestedAgents();
	for(agentId = requestedAgents.begin(); agentId != requestedAgents.end(); agentId++)
            getRequestFromRequestMap(source, agentId->currentRank()).addRequest(*agentId);
		
	const vector<AgentId>& cancellations    = request.cancellations();
	for(agentId = cancellations.begin(); agentId != cancellations.end(); agentId++)
            getRequestFromRequestMap(source, agentId->currentRank()).addCancellation(*agentId);
		
}
	
AgentRequest& RequestManager::getRequestFromRequestMap(int source, int target){
	map<int, vector<AgentRequest>*>::iterator mapIter = requestMap.find(target);
	if (mapIter == requestMap.end()) {
		vector<AgentRequest>* list = new vector<AgentRequest> ();
		requestMap[target] = list;
		mapIter = requestMap.find(target);
	}
	vector<AgentRequest>* list = mapIter->second; // We know this exists; we may just have created it
	for (int i = 0, n = list->size(); i < n; i++) {
		AgentRequest& req = (*list)[i];
		if (req.sourceProcess() == source && req.targetProcess() == target) return req; // Done; note: check of 'target' may not be needed
	}
	AgentRequest aRequest(source, target);
	list->push_back(aRequest);
	return list->back();
		
}
	

void RequestManager::receiveRequests() {
	mpi::communicator world;
	// skip requests from 0 as this process should be 0
	for (int i = 1; i < processCount_; i++) {
		int amtToRecv = requestCounts_[i];
		if (amtToRecv > 0) {

			/*
			 // debug
			 Logger logger = Log4CL::instance()->get_logger("repast.system");
			 ostringstream os;
			 os << "expecting to receive " << amtToRecv << " requests from " << i << endl;
			 logger.log(DEBUG, os.str());
			 // end debug
			 */

			//Timer t;
			//t.start();

			AgentRequest request(i);
			world.recv(i, AGENT_REQUEST_TAG, request);

			//long double time = t.stop();
			//Log4CL::instance()->get_logger("root").log(INFO, "requestAgents, receiveRequest, time: "
			//		+ boost::lexical_cast<std::string>(time));
			/*
			 // debug
			 os.str("");
			 os << "Received from " << i << " " << request << endl;
			 logger.log(DEBUG, os.str());
			 // end debug
			 */

			mapRequest(request);
		}
	}
}

}
