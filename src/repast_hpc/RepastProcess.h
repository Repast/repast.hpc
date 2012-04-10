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
 *  RepastProcess.h
 *
 *  Created on: Jan 5, 2009
 *      Author: nick
 */

#ifndef REPASTPROCESS_H_
#define REPASTPROCESS_H_

#include <vector>
#include <map>
#include <set>
#include <list>
#include <iostream>

#include <boost/mpi/communicator.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/unordered_set.hpp>
#include <boost/noncopyable.hpp>
#include <boost/ptr_container/ptr_list.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/map.hpp>

#include "Schedule.h"
#include "AgentId.h"
#include "SharedContext.h"
#include "AgentRequest.h"
#include "AgentStatus.h"
#include "mpi_constants.h"
#include "SRManager.h"
#include "RepastErrors.h"
#include "AgentImporterExporter.h"

// these are for the timings logging
#include "Utilities.h"
#include "logger.h"

namespace repast {

static std::string DEFAULT_SHARED_AGENT_SET_NAME = "DefaultSharedAgentSetName";


// TODO move these to where they are used
typedef std::vector<int>::iterator IntIter;
typedef std::vector<int>::const_iterator ConstIntIter;
typedef std::vector<repast::Agent*>::iterator AgentPIterator;
typedef std::map<int, repast::AgentRequest>::iterator IntARIter;


/**
 * Encapsulates the process in which repast is running and
 * manages interprocess communication etc. This is singleton to
 * insure that there is one per actual process.
 */
class RepastProcess: public boost::noncopyable {

private:

	typedef boost::unordered_set<AgentId, HashId> MovedAgentSetType;

	template<typename Content, typename Provider>
	friend void _synchAgents(Provider& provider, int tag, std::vector<std::vector<Content>*>& contents
#ifdef SHARE_AGENTS_BY_SET
      , std::string setName = DEFAULT_AGENT_REQUEST_SET
#endif
);


	template<typename T, typename Content, typename Provider, typename Receiver>
	friend void requestAgents(SharedContext<T>&, AgentRequest& req, Provider& provider, Receiver& receiver
#ifdef SHARE_AGENTS_BY_SET
	    , std::string setName = DEFAULT_AGENT_REQUEST_SET, AGENT_IMPORTER_EXPORTER_TYPE setType = DEFAULT_ENUM_SYMBOL
#endif
);

	static RepastProcess *_instance;
	ScheduleRunner* runner;
	boost::mpi::communicator* world;
	boost::mpi::communicator myWorld; // In case one is not provided
	int rank_, worldSize_;

#ifndef SHARE_AGENTS_BY_SET
	AbstractImporterExporter* importer_exporter;
#else
	ImporterExporter_BY_SET* importer_exporter;
#endif

	// key is process and value are requests from importers
	// that the key-process should now export to. Used when
	// an agent moves from this process and we need
	// to tell the process-it-moves-to where it should
	// export agents to
	std::map<int, std::vector<AgentRequest>*> importers;
	MovedAgentSetType movedAgents;

	// called by request agents function to initiate the request
#ifndef SHARE_AGENTS_BY_SET
	void initiateAgentRequest(AgentRequest& requests);
#else
	void initiateAgentRequest(AgentRequest& requests, std::string setName, AGENT_IMPORTER_EXPORTER_TYPE setType);
#endif

protected:
	RepastProcess(boost::mpi::communicator* comm = 0);

public:

	/**
	 * Initialize this RepastProcess. This must be called before the
	 * RepastProcess is used. If a configuration properties file is specified
	 * this properties file will be used to configure logging.
	 *
	 * @param propsfile a configuration properties file. This can be an
	 * empty string.
	 */
	static RepastProcess* init(std::string propsfile, boost::mpi::communicator* comm = 0, int maxConfigFileSize = MAX_CONFIG_FILE_SIZE);

	/**
	 * Gets this RepastProcess.
	 *
	 * @return this RepastProcess instance.
	 */
	static RepastProcess* instance();

  static boost::mpi::communicator* communicator();

	virtual ~RepastProcess();

	/**
	 * Synchronizes the status (moved or died) of all agents across processes.
	 * If any agent may have moved into the grid portion managed by another process
	 * or if any agent has died then this must be called prior to those agents doing
	 * anything.
	 *
	 * @param context the SharedContext that contains the agents on this proceses
	 * @param provider the class that provides agents given an AgentRequest
	 * @param creator creates agents of type T given Content.
	 *
	 * @tparam T the type of agents in the context
	 * @tparam Content the serializable struct or class that describes
	 * an agents state.
	 * @tparam Provider a class that provides Content, when ggiven an AgentRequest,
	 * implementing void provideContent(const repast::AgentRequest&, std::vector<Content>& out)
	 * @tparam AgentCreator a class that can create agents from Content, implementing
	 * T* createAgent(Content&).
	 *
	 */
	template<typename T, typename Content, typename Provider, typename AgentCreator>
	void syncAgentStatus(SharedContext<T>& context, Provider& provider, AgentCreator& creator);

	/**
	 * NON USER API.
	 *
	 * Notifies this RepastProcess that the specified agent
	 * has been removed (e.g. the agent "died").
	 */
	void agentRemoved(const AgentId& id);

	/**
	 * NON USER API.
	 *
	 * Notifies this RepastProcess that the specified agent
	 * should be moved from this process to the specified process.
	 *
	 * @param id the id of the agent to be moved
	 * @param process the process to move the agent to
	 */
	void moveAgent(const AgentId& id, int process);


	/**
	 * NON USER API.
	 *
	 * Notifies this RepastProcess that it is exporting the specified
	 * agent to the specified process. This sort of notification is
	 * done automatically when requesting agents, but agents may get
	 * added in other ways.
	 */
	void addExportedAgent(int importingProcess, AgentId id);

	/**
	 * NON USER API.
	 *
	 * Notifies this RepastProcess that it is importing
	 * the specified agent. This sort of notification
	 * is normally done automatically when requesting agents, but imports can
	 * occur in other ways.
	 */
	void addImportedAgent(AgentId id);

	/**
	 * Gets the rank of this process.
	 *
	 * @return the rank of this process.
	 */
	int rank() const {
		return rank_;
	}

	/**
	 * Gets the number of processes in the world.
	 */
	int worldSize() const {
		return worldSize_;
	}

	/**
	 * Notifes this RepastProcess that simulation has completed. This should be called
	 * when the simulation has completed.
	 */
	void done();

	/**
	 * Gets the ScheduleRunner used by this
	 * RepastProcess.
	 *
	 * @return the ScheduleRunner used by this RepastProcess.
	 */
	ScheduleRunner& getScheduleRunner() {
		return *runner;
	}

  boost::mpi::communicator* getCommunicator(){
    return world;
  }

#ifdef SHARE_AGENTS_BY_SET
  void dropImporterExporterSet(std::string setName){
    importer_exporter->dropSet(setName);
  }
#endif

  std::string ImporterExporterVersion(){ return "" + importer_exporter->version(); }

  std::string ImporterExporterReport(){ return importer_exporter->getReport(); }
};


template<typename T, typename Content, typename Provider, typename AgentCreator>
void RepastProcess::syncAgentStatus(SharedContext<T>& context, Provider& provider, AgentCreator& creator) {
  std::vector<std::vector<AgentStatus>* > statusUpdates;
  importer_exporter->exchangeAgentStatusUpdates(*world, statusUpdates);

  for (size_t i = 0, n = statusUpdates.size(); i < n; i++) {
		std::vector<AgentStatus>* vec = statusUpdates[i];
		for (size_t j = 0, k = vec->size(); j < k; ++j) {
			AgentStatus& status = (*vec)[j];
			if (status.getStatus() == AgentStatus::REMOVED) {
        importer_exporter->importedAgentIsRemoved(status.getOldId()); // Notify importer/exporter that this agent will not be imported anymore
        context.importedAgentRemoved(status.getOldId());              // Remove from context; agent cannot exist on this process after removal from home process
      }
			else if (status.getStatus() == AgentStatus::MOVED) {
				if (rank_ != status.getNewId().currentRank()) {
					// Notify importer that this agent will not be imported from the original
				  // process, but will instead be imported from its new home
					importer_exporter->importedAgentIsMoved(status.getOldId(), status.getNewId().currentRank());
					// find it and update its id
					T* agent = context.getAgent(status.getOldId());
					if (agent == (void*) 0)	throw Repast_Error_32<AgentId>(status.getOldId()); // Agent not found

					agent->getId().currentRank(status.getNewId().currentRank());
				}
				else{
				  // Notify importer that the agent will not be imported anymore because this is its home process now
				  importer_exporter->importedAgentIsNowLocal(status.getOldId());
				}
			}
		}
		delete vec;
	}

	// send the moved agents to the process they moved to
	// who does this have to send to
	std::set<int> psMovedTo;
	std::map<int, AgentRequest> agentRequests;
	for (MovedAgentSetType::iterator iter = movedAgents.begin(); iter != movedAgents.end(); ++iter) {
		AgentId id = *iter;
		int currentProc = id.currentRank();
		psMovedTo.insert(currentProc);
		IntARIter mapIter = agentRequests.find(currentProc);
		if (mapIter == agentRequests.end()) {
			AgentRequest req = AgentRequest(currentProc, rank_);
			req.addRequest(id);
			agentRequests[currentProc] = req;
		} else {
			AgentRequest& req = mapIter->second;
			if (!req.contains(id))
				req.addRequest(id);
		}
	}

	// Clear the moved agents data
	movedAgents.clear();

	std::vector<int> psToSendTo(psMovedTo.begin(), psMovedTo.end());

	// coordinate the send, receive pairs
	std::vector<int> psToReceiveFrom;
	SRManager manager(world);
	manager.retrieveSources(psToSendTo, psToReceiveFrom, AGENT_MOVED_SENDERS);

	std::vector<boost::mpi::request> requests;

	std::vector<std::pair<std::vector<Content>*, AgentExporterInfo*>* > contentsAndExporterInfo;

	for (std::vector<int>::const_iterator iter = psToReceiveFrom.begin(); iter != psToReceiveFrom.end(); ++iter) {
		int source = *iter;
    std::pair<std::vector<Content>*, AgentExporterInfo*>* toRecv =
        new std::pair<std::vector<Content>*, AgentExporterInfo*>;
		requests.push_back(world->irecv(source, AGENT_MOVED_AGENT, *toRecv));
		contentsAndExporterInfo.push_back(toRecv);
	}

	// Note: boost ptr_list takes care of memory cleanup:
  boost::ptr_list<std::vector<Content> >                                 contentPtrList;       // To contain the vectors of content
  boost::ptr_list<std::pair<std::vector<Content>*, AgentExporterInfo*> > pairPtrList;          // To contain the pairs of ptrs to vectors and agent export info

  std::vector<Content>*                                                  content;
  std::pair<std::vector<Content>*, AgentExporterInfo*>*                  toSend;

	for (IntARIter iter = agentRequests.begin(); iter != agentRequests.end(); ++iter) {
    contentPtrList.push_back( content = new std::vector<Content> );
		provider.provideContent(iter->second, *content);
    AgentExporterInfo* agentImporterInfoPtr = importer_exporter->getAgentExportInfo(iter->first);
    pairPtrList.push_back( toSend = new std::pair<std::vector<Content>*, AgentExporterInfo*>(content, agentImporterInfoPtr) );
		requests.push_back(world->isend(iter->first, AGENT_MOVED_AGENT, *toSend));
	}

	boost::mpi::wait_all(requests.begin(), requests.end());

	// Clear all sent data
	importer_exporter->clearAgentExportInfo();

  // Remove the agents that are moving to other processes
	for (IntARIter iter = agentRequests.begin(); iter != agentRequests.end(); ++iter) {
		const AgentRequest& req = iter->second;
		const std::vector<AgentId>& ids = req.requestedAgents();
		for (size_t i = 0, n = ids.size(); i < n; ++i) {
			const AgentId& id = ids[i];
			context.removeAgent(id);
		}
	}

	// Insert the newly received agents that moved to this process and update exporters
	typename std::vector<std::pair<std::vector<Content>*, AgentExporterInfo*> * >::iterator pairIter = contentsAndExporterInfo.begin();
	while(pairIter != contentsAndExporterInfo.end()){
	  // First do all the contents
	  typename std::vector<Content>::iterator contentIter = (*((*pairIter)->first)).begin();
	  while(contentIter != (*((*pairIter)->first)).end()){
      T* out = creator.createAgent(*contentIter);
      out->getId().currentRank(rank_);
      if (!context.addAgent(out)) {
        // Already contained the agent as an imported agent so update the existing agent's id
        T* agent = context.getAgent(out->getId());
        agent->getId().currentRank(rank_);
        delete out;
      } else {
        context.incrementProjRefCount(out->getId());
      }
	    contentIter++;
	  }
	  // Next do the exporter update
	  importer_exporter->incorporateAgentExporterInfo(*((*pairIter)->second));
    // And go to the next proc's data
	  pairIter++;
	}
}

/**
 * Synchronizes the state of any agents that are shared across processes.
 * If no agents are shared across processes, then this does not need to
 * be called.
 *
 * @param provider provides Content for a given an AgentRequest
 * @param updater updates an existing agent given Content
 *
 * @tparam Content the serializable struct or class that describes the
 * state of agents
 * @tparam Provider given an AgentRequest, a Provider provides the Content
 * for the requested agents, implementing void provideContent(const AgentRequest&,
 * std::vector<Content>&)
 * @tparam Updater given Content, an Updater updates an existing agent with
 * the Content, implementing void updateAgent(const Content&).
 */
template<typename Content, typename Provider, typename Updater>
void syncAgents(Provider& provider, Updater& updater
#ifdef SHARE_AGENTS_BY_SET
   , std::string setName = REQUEST_AGENTS_ALL
#endif
) {
  std::vector<std::vector<Content>*> contents;

#ifndef SHARE_AGENTS_BY_SET
	_synchAgents<Content> (provider, AGENT_SYNC_STATE_TAG, contents);
#else
	_synchAgents<Content> (provider, AGENT_SYNC_STATE_TAG, contents, setName);
#endif
	for (int i = 0, n = contents.size(); i < n; i++) {
	  std::vector<Content>* content = contents[i];
	  for (typename std::vector<Content>::const_iterator iter = content->begin(); iter != content->end(); ++iter) {
      updater.updateAgent(*iter);
    }
		delete contents[i];
	}
}


/**
 * NON USER API.
 *
 * @param tag identifies what is being synched -- agent
 * state, status
 *
 */
template<typename Content, typename Provider>
void _synchAgents(Provider& provider, int tag, std::vector<std::vector<Content>*>& contents
#ifdef SHARE_AGENTS_BY_SET
   , std::string setName = REQUEST_AGENTS_ALL
#endif
){
  RepastProcess* rp = RepastProcess::instance();
	boost::mpi::communicator* world = rp->getCommunicator();
	std::vector<boost::mpi::request> requests;

  // set of processes exporting to this
#ifndef SHARE_AGENTS_BY_SET
	const std::set<int>& exporters = rp->importer_exporter->getExportingProcesses();
#else
	const std::set<int>& exporters = rp->importer_exporter->getExportingProcesses(setName);
#endif

	for (std::set<int>::const_iterator iter = exporters.begin(); iter != exporters.end(); ++iter) {
		int source = *iter;
		std::vector<Content>* content = new std::vector<Content>();
		requests.push_back(world->irecv(source, tag, *content));
		contents.push_back(content);
	}

  // map processes that this exports to
#ifndef SHARE_AGENTS_BY_SET
  const std::map<int, AgentRequest>& agentsToExport = rp->importer_exporter->getAgentsToExport();
#else
  const std::map<int, AgentRequest>& agentsToExport = rp->importer_exporter->getAgentsToExport(setName);
#endif

  boost::ptr_list<std::vector<Content> > ptrList;

  std::vector<Content>* content;
	for (std::map<int, AgentRequest>::const_iterator iter = agentsToExport.begin(); iter != agentsToExport.end(); ++iter) {
		ptrList.push_back( content = new std::vector<Content> );
		provider.provideContent(iter->second, *content);
		requests.push_back(world->isend(iter->first, tag, *content));
	}

	boost::mpi::wait_all(requests.begin(), requests.end());

}

/**
 * Requests agents from one process to others. Copies of the requested agents'
 * Content are retrieved from their respective processes, created using
 * the AgentCreator and added to the specified context.
 *
 * @param context the context to which the requested agents will be added
 * @param request the AgentRequest containing the ids of the requested agents
 * @param provider provides Content for a given an AgentRequest
 * @param creator creates agents of type T given Content.
 *
 *
 * @tparam T the type of the agents in the context
 * @tparam Content the serializable struct or class that describes the
 * state of agents
 * @tparam Provider given an AgentRequest, a Provider provides the Content
 * for the requested agents, implementing void provideContent(const AgentRequest&,
 * std::vector<Content>&)
 * @tparam AgentCreator a class that can create agents from Content, implementing
 * T* createAgent(Content&).
 *
 */
template<typename T, typename Content, typename Provider, typename AgentCreator>
void requestAgents(SharedContext<T>& context, AgentRequest& request, Provider& provider, AgentCreator& creator
#ifdef SHARE_AGENTS_BY_SET
    , std::string setName = DEFAULT_AGENT_REQUEST_SET, AGENT_IMPORTER_EXPORTER_TYPE setType = DEFAULT_ENUM_SYMBOL
#endif
){

  std::vector<std::vector<Content>*> contents;

#ifndef SHARE_AGENTS_BY_SET
  RepastProcess::instance()->initiateAgentRequest(request);
  _synchAgents<Content> (provider, AGENT_TAG, contents);
#else
  RepastProcess::instance()->initiateAgentRequest(request, setName, setType);
	_synchAgents<Content> (provider, AGENT_TAG, contents, setName);
#endif

	// Add the agents to the context (Note: information for previously shared agents
	// may also be sent; if so the 'addAgent' will fail silently; the agent's information
	// will NOT be updated)
	for (int i = 0, n = contents.size(); i < n; i++) {
		std::vector<Content>* content = contents[i];
		for (typename std::vector<Content>::const_iterator iter = content->begin(); iter != content->end(); ++iter) {
			T* out = creator.createAgent(*iter);
			if (context.addAgent(out)) {
				context.incrementProjRefCount(out->getId());
			}
		}
		delete content;
	}
}

}

#endif /* REPASTPROCESS_H_ */
