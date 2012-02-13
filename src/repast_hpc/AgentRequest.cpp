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
 * AgentRequest.cpp
 *
 *  Created on: Aug 19, 2010
 *      Author: nick
 */

#include <vector>

#include "AgentRequest.h"

using namespace std;

namespace repast {

AgentRequest::AgentRequest(int sourceProcess) :
	source(sourceProcess), target(-1) {
}

AgentRequest::AgentRequest(int sourceProcess, int targetProcess) :
	source(sourceProcess), target(targetProcess) {
}

void AgentRequest::addRequest(const AgentId& id) {
	requestedAgents_.push_back(id);
}

void AgentRequest::addCancellation(const AgentId& id) {
	cancellations_.push_back(id);
}
	
bool AgentRequest::contains(const AgentId& id) {
	return containsInRequests(id) || containsInCancellations(id); // Note: may short-circuit, but this is OK
}
	
bool AgentRequest::containsInRequests(const AgentId& id){
	return find(requestedAgents_.begin(), requestedAgents_.end(), id) != requestedAgents_.end();
}
	
bool AgentRequest::containsInCancellations(const AgentId& id){
	return find(cancellations_.begin(), cancellations_.end(), id) != cancellations_.end();
}

bool AgentRequest::remove(const AgentId& id, bool removeAllInstances) {
	// Note: (removeRequest(id) || removeCancellation(id)) will fail because of short-circuit evaluation
	bool foundInRequests      = removeRequest(id);
	bool foundInCancellations = removeCancellation(id);
	return foundInRequests || foundInCancellations;
}
	
bool AgentRequest::removeRequest(const AgentId& id, bool removeAllInstances) {
	bool retVal = false;
	for (vector<AgentId>::iterator iter = requestedAgents_.begin(); iter != requestedAgents_.end(); ) {
		if ((*iter) == id) {
			iter = requestedAgents_.erase(iter);
			retVal = true;
			if(!removeAllInstances) return retVal;
		} else {
			++iter;
		}
	}
	return retVal;
}
	
	
bool AgentRequest::removeCancellation(const AgentId& id, bool removeAllInstances) {
	bool retVal = false;
	for (vector<AgentId>::iterator iter = cancellations_.begin(); iter != cancellations_.end(); ) {
		if ((*iter) == id) {
			iter = cancellations_.erase(iter);
			retVal = true;
      if(!removeAllInstances) return retVal;
		} else {
			++iter;
		}
	}
	return retVal;
}
	
void AgentRequest::addAll(const AgentRequest& req) {
	addAllRequests(req);
	addAllCancellations(req);
}
	
void AgentRequest::addAllRequests(const AgentRequest& req) {
	requestedAgents_.insert(requestedAgents_.end(), req.requestedAgents_.begin(), req.requestedAgents_.end());
}
	
void AgentRequest::addAllCancellations(const AgentRequest& req) {
	cancellations_.insert(cancellations_.end(), req.cancellations_.begin(), req.cancellations_.end());
}
	
void AgentRequest::targets(set<int>& targets) {
	targetsOfRequests(targets);
	targetsOfCancellations(targets);
}
	
void AgentRequest::targetsOfRequests(std::set<int>& targets) {
	for (int i = 0, n = requestedAgents_.size(); i < n; i++) {
		AgentId& id = requestedAgents_[i];
		int target = id.currentRank();
		targets.insert(target);
	}
}
	
void AgentRequest::targetsOfCancellations(std::set<int>& targets) {
	for (int i = 0, n = cancellations_.size(); i < n; i++) {
		AgentId& id = cancellations_[i];
		int target = id.currentRank();
		targets.insert(target);
	}	
}


std::ostream& operator<<(std::ostream& os, const AgentRequest& request) {
	os << "AgentRequest(" << request.source << ", " << request.target << ", [";
	
	bool comma = false;
	for (vector<AgentId>::const_iterator it = request.requestedAgents_.begin(); it != request.requestedAgents_.end(); it++) {
		if (comma)
			os << ", ";
		os << *it;
		comma = true;
	}
	os << "] CANCELLATIONS:[ ";
	comma = false;
	for (vector<AgentId>::const_iterator it = request.cancellations_.begin(); it != request.cancellations_.end(); it++) {
		if (comma)
			os << ", ";
		os << *it;
		comma = true;
	}
	os << "])";
	return os;
}

}
