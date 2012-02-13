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
 *  RequestManager.h
 *
 *  Created on: Aug 19, 2010
 *      Author: nick
 */

#ifndef REQUESTMANAGER_H_
#define REQUESTMANAGER_H_

#include <vector>
#include <boost/mpi/communicator.hpp>
#include <map>

#include "AgentRequest.h"

namespace repast {

/**
 * NON USER API.
 *
 * Used by root process to manage requests to / from other processes.
 */
class RequestManager {

private:
	std::map<int, std::vector<AgentRequest>*> requestMap;
	std::vector<int> requestCounts_;
	int processCount_;
    AgentRequest& getRequestFromRequestMap(int source, int target);

public:

	RequestManager(int processCount);
	~RequestManager();

	void receiveRequests();

	void mapRequest(AgentRequest& request);

	void fillRequests(boost::mpi::communicator& world, std::vector<std::vector<AgentRequest> >& reqs);

	/*void sendRequests(boost::mpi::communicator& world);*/

	std::vector<AgentRequest>* requestsFor(int rank);

	std::vector<int>& requestCounts() {
		return requestCounts_;
	}
};


}

#endif /* REQUESTMANAGER_H_ */
