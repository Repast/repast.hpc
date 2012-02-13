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
 *  AgentImporter.h
 *
 *  Created on: Aug 19, 2010
 *      Author: nick
 */

#ifndef AGENTIMPORTER_H_
#define AGENTIMPORTER_H_

#include <vector>
#include <set>
#include <boost/mpi/communicator.hpp>
#include <map>

#include "AgentStatus.h"

namespace repast {

// fwd declaration so can be friend of AgentImporter and AgentExporter
class RepastProcess;
struct Receipt;
struct StatusReceipt;

/**
 * Maintains a count of the number of agents that this process is importing from
 * each of the other process in this communicator.
 *
 * NON Public API.
 */
class AgentImporter {

private:
	friend class RepastProcess;
	template<typename Content, typename Provider>
	friend void _synchAgents(Provider& provider, int tag, std::vector<std::vector<Content>*>& contents);

	// key: exporting rank, val: count of how many agents are
	// exported to this rank
	std::map<int, int> exportingSources;

public:

	/**
	 * Destructor
	 */
	virtual ~AgentImporter() {}

	/**
	 * Increments the count of agents that the specified process is exporting to
	 * this one
	 */
	void incrementImportedAgentCount(int exportingRank);

	/**
	 * Decrements the count of agents that the specified process is exporting to
	 * this one
	 */
	void decrementImportedAgentCount(int exportingRank);
	
	/**
	 * Creates an non-blocking receive for each process that is exporting to this one;
	 * the communicator passed is used to create the receives, which are placed in the
	 * vector of mpi::requests. Each receive will get a vector of AgentStatus
	 * information; the destination instances are placed in the 'out' collection.
	 */
	void recvStatus(boost::mpi::communicator& comm, std::vector<boost::mpi::request>&, std::vector<std::vector<AgentStatus>*>& out);

};

}


#endif /* AGENTIMPORTER_H_ */
