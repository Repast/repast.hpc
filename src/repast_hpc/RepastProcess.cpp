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
#define INITIATE_AGENT_REQ_ALLTOALL

#include <map>
#include <algorithm>
#include <utility>
#include <exception>
#include <mpi.h>
#include <boost/mpi.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/unordered_set.hpp>

#include "RepastProcess.h"
#include "logger.h"
#include "SRManager.h"
#include "Utilities.h"
#include "RequestManager.h"

using namespace std;
namespace mpi = boost::mpi;

namespace repast {

RepastProcess* RepastProcess::_instance = 0;

RepastProcess::RepastProcess(boost::mpi::communicator* comm){
  world = (comm  != 0 ? comm : &myWorld);
  runner = new ScheduleRunner(world);
  rank_ = world->rank();
  worldSize_ = world->size();
}

RepastProcess* RepastProcess::init(string propsfile, boost::mpi::communicator* comm, int maxConfigFileSize) {
  if (_instance == 0) {
    boost::mpi::communicator globalWorld;
    boost::mpi::communicator* world;
    world = (comm != 0 ? comm : &globalWorld);

    if (propsfile.length() > 0)
			Log4CL::configure(world->rank(), propsfile, comm, maxConfigFileSize);
		else
			Log4CL::configure(world->rank());
		_instance = new RepastProcess(comm);
	}
	return _instance;
}

RepastProcess* RepastProcess::instance() {
	if (_instance == 0)
		throw domain_error("RepastProcess must be initialized before calling instance");
	return _instance;
}

boost::mpi::communicator* RepastProcess::communicator(){
  return instance()->getCommunicator();
}

void RepastProcess::done() {
	Log4CL::instance()->close();
}

void RepastProcess::updateExporters() {
	// update importing ranks to import from the process
	// where an agent as moved to.
	vector<int> exportersToUpdate;
	for (map<int, vector<AgentRequest>*>::iterator iter = importers.begin(); iter != importers.end(); ++iter) {
		exportersToUpdate.push_back(iter->first);
	}

	vector<int> senders;
	SRManager manager(world);
	manager.retrieveSources(exportersToUpdate, senders, EXPORTER_UPDATE_SENDERS);

	vector<mpi::request> requests;
	// isend
	for (map<int, vector<AgentRequest>*>::iterator iter = importers.begin(); iter != importers.end(); ++iter) {
		requests.push_back(world->isend(iter->first, EXPORTER_UPDATE_IMPORTERS, *(iter->second)));
	}

	// should now have list of senders to that we should receive from
	vector<vector<AgentRequest>*> importersToAdd;
	for (vector<int>::iterator iter = senders.begin(); iter != senders.end(); ++iter) {
		int sender = *iter;
		vector<AgentRequest>* vec = new vector<AgentRequest> ();
		importersToAdd.push_back(vec);
		requests.push_back(world->irecv(sender, EXPORTER_UPDATE_IMPORTERS, *vec));
	}

	boost::mpi::wait_all(requests.begin(), requests.end());

	// notifies the P at this rank that it should now export -- fulfilling the specified requests
	for (vector<vector<AgentRequest>*>::iterator iter = importersToAdd.begin(); iter != importersToAdd.end(); ++iter) {
		vector<AgentRequest>* vec = *iter;
		for (int i = 0, n = vec->size(); i < n; ++i) {
			AgentRequest req = vec->at(i);
			//std::cout << rank_ << std::endl;
			for (vector<AgentId>::const_iterator vIter = req.requestedAgents().begin(); vIter
					!= req.requestedAgents().end(); ++vIter) {
				// don't add agents that are exported by me to be imported by me
				if (req.sourceProcess() != rank_)
					addExportedAgent(req.sourceProcess(), *vIter);
			}
		}

		delete vec;
	}

	// Should clear the map of importing processes
	importers.clear();
}

void RepastProcess::addExportedAgent(int importingProcess, AgentId id) {
	exporter.addExportedAgent(importingProcess, id);
}

void RepastProcess::addImportedAgent(AgentId id) {
	importer.incrementImportedAgentCount(id.currentRank());
}

void RepastProcess::agentRemoved(const AgentId& id) {
	movedAgents.erase(id);
	exporter.agentRemoved(id);
}

void RepastProcess::moveAgent(const AgentId& id, int process) {
	MovedAgentSetType::const_iterator iter = movedAgents.find(id);
	if (iter == movedAgents.end()) {
		AgentId newId(id);
		newId.currentRank(process);
		movedAgents.insert(newId);
		map<int, vector<AgentRequest>*>::iterator iter = importers.find(process);
		if (iter == importers.end()) {
			vector<AgentRequest>* tmp = new vector<AgentRequest> ();
			exporter.agentMoved(id, process, *tmp);
			if (tmp->size() == 0) {
				delete tmp;
			} else {
				importers[process] = tmp;
			}
		} else {
			exporter.agentMoved(id, process, *(iter->second));
		}
	} else {
		AgentId other = *iter;
		if (other.currentRank() != process) {
			std::cout << rank_ << " : " << other << ", " << id << " trying to move to " << process << std::endl;
			throw std::domain_error("Cannot move agent to two different processes during the same iteration");
		}
	}
}

void RepastProcess::initiateAgentRequest(AgentRequest &request) {

  boost::unordered_set<AgentId, HashId> requestedAgentSet;
  const std::vector<AgentId>& requestedIds = request.requestedAgents();
  std::vector<AgentId>::const_iterator requestedIter;
  for (requestedIter = requestedIds.begin(); requestedIter != requestedIds.end(); ++requestedIter) {
      if (requestedAgentSet.find(*requestedIter) == requestedAgentSet.end()) {
          importer.incrementImportedAgentCount(requestedIter->currentRank());
          requestedAgentSet.insert(*requestedIter);
      }
  }

  boost::unordered_set<AgentId, HashId> cancelledAgentSet;
  const std::vector<AgentId>& cancelledIds = request.cancellations();
  std::vector<AgentId>::const_iterator cancellationIter;
  for (cancellationIter = cancelledIds.begin(); cancellationIter != cancelledIds.end(); ++cancellationIter) {
      if (cancelledAgentSet.find(*cancellationIter) == cancelledAgentSet.end()) {
          importer.decrementImportedAgentCount(cancellationIter->currentRank());
          cancelledAgentSet.insert(*cancellationIter);
      }
  }


#ifdef INITIATE_AGENT_REQ_ALLTOALL

  int* countsOfRequests = new int[worldSize_];
  int maxNumRequests = 0;

  const vector<AgentId>& requestedAgents = request.requestedAgents();
  const vector<AgentId>& cancellations   = request.cancellations();

  vector<AgentId>::const_iterator agentId;
  vector<AgentId>::const_iterator agentIdEnd;

  agentIdEnd = requestedAgents.end();
  for(agentId = requestedAgents.begin(); agentId != agentIdEnd; agentId++){
    int target = agentId->currentRank();
    countsOfRequests[target]++;
    if(countsOfRequests[target] > maxNumRequests) maxNumRequests++;
  }

  agentIdEnd = cancellations.end();
  for(agentId = cancellations.begin(); agentId != agentIdEnd; agentId++){
    int target = agentId->currentRank();
    countsOfRequests[target]++;
    if(countsOfRequests[target] > maxNumRequests) maxNumRequests++;
  }

  int maxGlobalNumRequests = 0;

  MPI_Allreduce(&maxNumRequests, &maxGlobalNumRequests, 1, MPI_INT, MPI_MAX, *world);


  // Reset and reuse the values from countsOfRequests
  for(int i = 0; i < worldSize_; i++) countsOfRequests[i] = 0;

  // Create an array for the actual send; note 'padding' for extra values
  // Size of an individual element is 3 ints; don't need to send 'current rank'
  int dataElementSize = (maxGlobalNumRequests + 2) * 3;
  int dataSize = dataElementSize * worldSize_;
  int* data = new int[dataSize];
  int* rec  = new int[dataSize];

  // Initialize
  for(int i = 0; i < dataSize; i++) data[i] = -1; // Can't be zero

  // Now move the data into the send buffer:
  agentIdEnd = requestedAgents.end();
  for(agentId = requestedAgents.begin(); agentId != agentIdEnd; agentId++){
    int target = agentId->currentRank();
    int pos = target * dataElementSize + countsOfRequests[target] * 3;
    data[pos]   = agentId->id();
    data[pos+1] = agentId->startingRank();
    data[pos+2] = agentId->agentType();
    countsOfRequests[target]++;
  }

  for(int i = 0; i < worldSize_; i++) countsOfRequests[i]++; // Skip one entry to mark boundary between requests and cancellations

  agentIdEnd = cancellations.end();
  for(agentId = cancellations.begin(); agentId != agentIdEnd; agentId++){
    int target = agentId->currentRank();
    int pos = target * dataElementSize + countsOfRequests[target] * 3;
    data[pos]   = agentId->id();
    data[pos+1] = agentId->startingRank();
    data[pos+2] = agentId->agentType();
    countsOfRequests[target]++;
  }

  int RTEST = 3;

  // Exchange data
  MPI_Alltoall(data, dataElementSize, MPI_INT, rec, dataElementSize, MPI_INT, *world);
  delete data; // Done with this...

  // Now re-package the received data as the vector<AgentRequest> that is needed

  vector<AgentRequest> reqsRecd;
  for(int i = 0; i < worldSize_; i++){
    if(i != rank_){  // This isn't necessary as long as a process doesn't request agents from itself (!); this acts as an error trap if this happens
      int index = i * dataElementSize;
      // If there were no requests OR cancellations sent, skip
      if(!((rec[index] == -1) && rec[index + 3] == -1)){
        // Create the agent request
        AgentRequest req(i, rank_);
        // Add all the requests
        while(rec[index] != -1){
          AgentId id(rec[index], rec[index + 1], rec[index + 2]);
          id.currentRank(rank_);
          req.addRequest(id);
          index += 3;
        }
        index += 3;
        while(rec[index] != -1){
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

  if (reqsRecd.size() > 0) {// != 0) {
    exporter.updateExportedAgents(&reqsRecd);
  }

#else
  RequestManager *receiver = 0;

	// gather the request counts
	int numRequests = request.requestCount();
	if (rank_ == 0) {
		receiver = new RequestManager(worldSize_);
		vector<int>& requestCounts = receiver->requestCounts();

		//Timer t;
		//t.start();
		gather(*world, numRequests, requestCounts, 0);
		//Log4CL::instance()->get_logger("root").log(INFO, "ra_gather, time: " + boost::lexical_cast<
		//		std::string>(t.stop()));
	} else {
		//Timer t;
		//t.start();
		gather(*world, numRequests, 0);
		//Log4CL::instance()->get_logger("root").log(INFO, "ra_gather, time: " + boost::lexical_cast<
		//		std::string>(t.stop()));
	}

	if (rank_ == 0) {
		// proc 0 receives the requests
		receiver->receiveRequests();
		// process any requests that proc 0 makes
		if (numRequests > 0) {
			receiver->mapRequest(request);
		}
	} else {
		// other procs send their requests
		if (numRequests > 0) {
			//Timer t;
			//t.start();

			world->send(0, AGENT_REQUEST_TAG, request);
			//Log4CL::instance()->get_logger("root").log(INFO, "ra_send, time: "
			//		+ boost::lexical_cast<std::string>(t.stop()));
		}
	}

	vector<AgentRequest> requestsToFulfill;// = 0;
	vector<vector<AgentRequest> > allReqs;
	if (rank_ == 0) {
		allReqs.assign(worldSize_, vector<AgentRequest> ());
		receiver->fillRequests(*world, allReqs);
	}

	//Timer t;
	//t.start();
	scatter(*world, allReqs, requestsToFulfill, 0);
	//Log4CL::instance()->get_logger("root").log(INFO, "ra_scatter_reqs, time: " + boost::lexical_cast<
	//		std::string>(t.stop()));
	if (rank_ == 0) {
		vector<AgentRequest>* req = receiver->requestsFor(0);
		if (req != 0)
			requestsToFulfill = *(req);
	}

	if (requestsToFulfill.size() > 0) {// != 0) {
		exporter.updateExportedAgents(&requestsToFulfill);
	}
	delete receiver;
#endif
}

RepastProcess::~RepastProcess() {
  delete runner;
	_instance = 0;
}

}

