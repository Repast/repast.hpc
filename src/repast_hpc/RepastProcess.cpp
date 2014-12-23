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
 *  RepastProcess.cpp
 *
 *  Created on: Jan 5, 2009
 *      Author: nick
 */
#include <map>
#include <algorithm>
#include <utility>
#include <exception>
#include <mpi.h>
#include <boost/mpi.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/unordered_set.hpp>
#include <boost/serialization/export.hpp>

#include "RepastProcess.h"
#include "logger.h"
#include "SRManager.h"
#include "Utilities.h"

BOOST_CLASS_EXPORT_GUID(repast::SpecializedProjectionInfoPacket<double>,
		"SpecializedProjectionInfoPacket_DOUBLE");

BOOST_CLASS_EXPORT_GUID(repast::SpecializedProjectionInfoPacket<int>,
		"SpecializedProjectionInfoPacket_INT");

using namespace std;
namespace mpi = boost::mpi;

namespace repast {

RepastProcess* RepastProcess::_instance = 0;

RepastProcess::RepastProcess(boost::mpi::communicator* comm) : world(comm), runner(new ScheduleRunner(world)),
		rank_(world->rank()), worldSize_(world->size()),
		procsToSendProjInfoTo(NULL), procsToRecvProjInfoFrom(NULL), procsToSendAgentStatusInfoTo(NULL),
		procsToRecvAgentStatusInfoFrom(NULL) {

	//world = comm;
	//runner = new ScheduleRunner(world);
	//rank_ = world->rank();
	//worldSize_ = world->size();

#ifndef SHARE_AGENTS_BY_SET
	importer_exporter = new DEFAULT_IMPORTER_EXPORTER_CLASS();
#else
	importer_exporter = new ImporterExporter_BY_SET();
#endif

	int topRank = world->size() - 1;
	int recv = (rank_ > 0 ? rank_ - 1 : topRank);
	int send = (rank_ < topRank ? rank_ + 1 : 0);

	// "register" the SpecializedProjectionInfoPacket with
	// boost archive, iff running multiprocess.
	if (send != recv) {
		SpecializedProjectionInfoPacket<double> test1;
		SpecializedProjectionInfoPacket<double> recvT1;
		SpecializedProjectionInfoPacket<int> test2;
		SpecializedProjectionInfoPacket<int> recvT2;

		boost::mpi::request requests[4];
		requests[0] = world->isend(send, 0, test1);
		requests[1] = world->isend(send, 1, test2);
		requests[2] = world->irecv(recv, 0, recvT1);
		requests[3] = world->irecv(recv, 1, recvT2);
		boost::mpi::wait_all(requests, requests + 4);
	}
}

RepastProcess* RepastProcess::init(string propsfile,
		boost::mpi::communicator* comm, int maxConfigFileSize) {
	if (_instance != 0) {
		// reinitializing so delete the old instance
		delete _instance;

	}

	boost::mpi::communicator* tmpWorld = (
			comm != 0 ? comm : new boost::mpi::communicator());

	if (propsfile.length() > 0)
		Log4CL::configure(tmpWorld->rank(), propsfile, tmpWorld,
				maxConfigFileSize);
	else
		Log4CL::configure(tmpWorld->rank());
	_instance = new RepastProcess(tmpWorld);

	return _instance;
}

RepastProcess* RepastProcess::instance() {
	if (_instance == 0)
		throw Repast_Error_39(); // RepastProcess must be initialized before calling instance
	return _instance;
}

boost::mpi::communicator* RepastProcess::communicator() {
	return instance()->getCommunicator();
}

void RepastProcess::done() {
	Log4CL::instance()->close();
}

void RepastProcess::addExportedAgent(int importingProcess, AgentId id) {
//	exporter.addExportedAgent(importingProcess, id);
}

void RepastProcess::addImportedAgent(AgentId id) {
//	importer.incrementImportedAgentCount(id.currentRank());
}

void RepastProcess::agentRemoved(const AgentId& id) {
	movedAgents.erase(id);
	importer_exporter->agentRemoved(id);
}

void RepastProcess::moveAgent(const AgentId& id, int process) {
	AgentId newId(id);
	newId.currentRank(process);
	MovedAgentSetType::const_iterator iter = movedAgents.find(newId);
	if (iter == movedAgents.end()) {
		importer_exporter->agentMoved(id, process);
		movedAgents.insert(newId);
	} else {
		AgentId other = *iter;
		if (other.currentRank() != process) {
			std::cout << rank_ << " : " << other << ", " << id
					<< " trying to move to " << process << std::endl;
			throw Repast_Error_40<AgentId>(id, rank_, other.currentRank(),
					process); // Cannot move agent to two different processes during the same iteration
		}
	}
}

void RepastProcess::initiateAgentRequest(AgentRequest& request
#ifdef SHARE_AGENTS_BY_SET
		, std::string setName, AGENT_IMPORTER_EXPORTER_TYPE setType
#endif
		) {

	// Record and process the outgoing request for agents from other processes
#ifndef SHARE_AGENTS_BY_SET
	importer_exporter->registerOutgoingRequests(request);
#else
	importer_exporter->registerOutgoingRequests(request, setName, setType);
#endif

	int* countsOfRequests = new int[worldSize_];
	for (int i = 0; i < worldSize_; ++i)
		countsOfRequests[i] = 0; // OOPS! This was not included in version 1.0.1 final
	int maxNumRequests = 0;

	const vector<AgentId>& requestedAgents = request.requestedAgents();
	const vector<AgentId>& cancellations = request.cancellations();

	vector<AgentId>::const_iterator agentId;
	vector<AgentId>::const_iterator agentIdEnd;

	agentIdEnd = requestedAgents.end();
	for (agentId = requestedAgents.begin(); agentId != agentIdEnd; agentId++) {
		int target = agentId->currentRank();
		countsOfRequests[target]++;
		if (countsOfRequests[target] > maxNumRequests)
			maxNumRequests++;
	}

	agentIdEnd = cancellations.end();
	for (agentId = cancellations.begin(); agentId != agentIdEnd; agentId++) {
		int target = agentId->currentRank();
		countsOfRequests[target]++;
		if (countsOfRequests[target] > maxNumRequests)
			maxNumRequests++;
	}

	int maxGlobalNumRequests = 0;

	MPI_Allreduce(&maxNumRequests, &maxGlobalNumRequests, 1, MPI_INT, MPI_MAX,
			*world);

	// Reset and reuse the values from countsOfRequests
	for (int i = 0; i < worldSize_; i++)
		countsOfRequests[i] = 0;

	// Create an array for the actual send; note 'padding' for extra values
	// Size of an individual element is 3 ints; don't need to send 'current rank'
	int dataElementSize = (maxGlobalNumRequests + 2) * 3;
	int dataSize = dataElementSize * worldSize_;
	int* data = new int[dataSize];
	int* rec = new int[dataSize];

	// Initialize
	for (int i = 0; i < dataSize; i++)
		data[i] = -1; // Can't be zero

	// Now move the data into the send buffer:
	agentIdEnd = requestedAgents.end();
	for (agentId = requestedAgents.begin(); agentId != agentIdEnd; agentId++) {
		int target = agentId->currentRank();
		int pos = target * dataElementSize + countsOfRequests[target] * 3;
		data[pos] = agentId->id();
		data[pos + 1] = agentId->startingRank();
		data[pos + 2] = agentId->agentType();
		countsOfRequests[target]++;
	}

	for (int i = 0; i < worldSize_; i++)
		countsOfRequests[i]++; // Skip one entry to mark boundary between requests and cancellations

	agentIdEnd = cancellations.end();
	for (agentId = cancellations.begin(); agentId != agentIdEnd; agentId++) {
		int target = agentId->currentRank();
		int pos = target * dataElementSize + countsOfRequests[target] * 3;
		data[pos] = agentId->id();
		data[pos + 1] = agentId->startingRank();
		data[pos + 2] = agentId->agentType();
		countsOfRequests[target]++;
	}

	// Exchange data
	MPI_Alltoall(data, dataElementSize, MPI_INT, rec, dataElementSize, MPI_INT,
			*world);
	delete data; // Done with this...

	// Now re-package the received data as the vector<AgentRequest> that is needed

	vector<AgentRequest> reqsRecd;
	for (int i = 0; i < worldSize_; i++) {
		if (i != rank_) { // This isn't necessary as long as a process doesn't request agents from itself (!); this acts as an error trap if this happens
			int index = i * dataElementSize;
			// If there were no requests OR cancellations sent, skip
			if (!((rec[index] == -1) && rec[index + 3] == -1)) {
				// Create the agent request
				AgentRequest req(i, rank_);
				// Add all the requests
				while (rec[index] != -1) {
					AgentId id(rec[index], rec[index + 1], rec[index + 2]);
					id.currentRank(rank_);
					req.addRequest(id);
					index += 3;
				}
				index += 3;
				while (rec[index] != -1) {
					AgentId id(rec[index], rec[index + 1], rec[index + 2]);
					id.currentRank(rank_);
					req.addCancellation(id);
					index += 3;
				}
				// Add (a copy of) the request to the vector
				reqsRecd.push_back(req);
			}
		}
	}

	delete countsOfRequests;
	delete rec;

	// Set up export of agents requested by other processes
#ifndef SHARE_AGENTS_BY_SET
	importer_exporter->registerIncomingRequests(reqsRecd);
#else
	importer_exporter->registerIncomingRequests(reqsRecd, setName);
#endif

}

RepastProcess::~RepastProcess() {
	delete runner;
	delete importer_exporter;

	delete procsToSendProjInfoTo;
	delete procsToRecvProjInfoFrom;

	delete procsToSendAgentStatusInfoTo;
	delete procsToRecvAgentStatusInfoFrom;

	_instance = 0;
}

}

