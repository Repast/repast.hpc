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
 *  AgentImporter.cpp
 *
 *  Created on: Aug 19, 2010
 *      Author: nick
 */

#include "AgentImporter.h"
#include "mpi_constants.h"

using namespace std;

namespace mpi = boost::mpi;

namespace repast {

void AgentImporter::recvStatus(mpi::communicator& comm, vector<mpi::request>& requests,
		vector<vector<AgentStatus>*>& out) {
	for (map<int, int>::const_iterator iter = exportingSources.begin(); iter != exportingSources.end(); ++iter) {
		int exportingProcess = iter->first;
		vector<AgentStatus>* vec = new vector<AgentStatus> ();
		out.push_back(vec);
		requests.push_back(comm.irecv(exportingProcess, AGENT_SYNC_STATUS, *vec));
	}
}

void AgentImporter::incrementImportedAgentCount(int exportingRank) {
	map<int, int>::iterator iter = exportingSources.find(exportingRank);
	if (iter == exportingSources.end()) {
		exportingSources[exportingRank] = 1;
	} else {
		exportingSources[exportingRank] = iter->second + 1;
	}
}

void AgentImporter::decrementImportedAgentCount(int exportingRank) {
	map<int, int>::iterator iter = exportingSources.find(exportingRank);
	if (iter == exportingSources.end()) {
		throw domain_error("Cannot decrement the agent count for a non-exporting rank");
	} else {
		int val = iter->second - 1;
		if (val == 0) {
			exportingSources.erase(iter);
		} else {
			exportingSources[exportingRank] = val;
		}
	}
}


}
