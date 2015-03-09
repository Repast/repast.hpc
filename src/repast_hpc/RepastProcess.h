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

/**
 * \mainpage Repast HPC: A High-Performance Agent-Based Modeling Platform
 *
 * By Argonne National Laboratory, 2009-2013
 *
 * \section intro_sec What is Repast HPC?
 *
 * Repast HPC is an Agent-Based Modeling Platform in the spirit of Repast Simphony
 * but designed for top-500 high-performance computing systems (supercomputers).
 */

namespace repast {

/**
 * Contains information sent as agents are exchanged, either in response to
 * requests or agent movement. Contains both agent raw information
 * (of type 'Content') and projection information.
 *
 * Note: A 'Packet' is responsible for deleting the objects to which it points
 * This is essentially not optional: when boost sends the Packet via MPI the locations
 * at which it places the different elements are not known (no 'new' is called
 * in the user code). Some code must be written to track these down and delete,
 * and it is manifestly easier to provide that code in the Packet itself
 * than to rewrite where needed, inspecting the Packet for the locations
 */
template<typename Content>
class Request_Packet {

	friend class boost::serialization::access;

public:
	std::vector<Content>* agentContentPtr;
	std::map<std::string, std::vector<ProjectionInfoPacket*> >* projectionInfoPtr;

	Request_Packet() :
			agentContentPtr(0), projectionInfoPtr(0) {
	}

	Request_Packet(std::vector<Content>* agentContent,
			std::map<std::string, std::vector<ProjectionInfoPacket*> >* projectionInfo) :
			agentContentPtr(agentContent), projectionInfoPtr(projectionInfo) {
	}

	~Request_Packet() {
		delete agentContentPtr;
		agentContentPtr = 0;

		if (projectionInfoPtr != 0) {
			for (std::map<std::string, std::vector<ProjectionInfoPacket*> >::iterator
					iter = projectionInfoPtr->begin(), iterEnd =
							projectionInfoPtr->end(); iter != iterEnd; ++iter) {
				for (std::vector<ProjectionInfoPacket*>::iterator PIPIter =
						iter->second.begin(), PIPIterEnd = iter->second.end();
						PIPIter != PIPIterEnd; ++PIPIter) {
					delete *PIPIter;
				}
			}
		}
		delete projectionInfoPtr;
		projectionInfoPtr = 0;

	}

	template<class Archive>
	void serialize(Archive& ar, const unsigned int version) {
		ar & agentContentPtr;
		ar & projectionInfoPtr;
	}

};

/**
 * Class that contains information sent in conjunction with synchronizing
 * agent status (agents moving or being removed from the simulation).
 * May contain secondary agent information (that is, agents that must newly be
 * created as non-local agents on the receiving process due to obligations
 * of projection contracts and the new existence of the agents being moved
 * to that process).
 *
 * Note the unusual requirement of the deletion of exporter information.
 */
template<typename Content>
class SyncStatus_Packet {

	friend class boost::serialization::access;

public:
	std::vector<Content>* agentContentPtr;
	std::map<std::string, std::vector<ProjectionInfoPacket*> >* projectionInfoPtr;
	std::set<AgentId>* secondaryIdsPtr;
	AgentExporterInfo* exporterInfoPtr;

	SyncStatus_Packet() :
			agentContentPtr(0), projectionInfoPtr(0), secondaryIdsPtr(0), exporterInfoPtr(
					0) {
	}

	SyncStatus_Packet(std::vector<Content>* agentContent,
			std::map<std::string, std::vector<ProjectionInfoPacket*> >* projectionInfo,
			std::set<AgentId>* secondaryIds, AgentExporterInfo* exporterInfo) :
			agentContentPtr(agentContent), projectionInfoPtr(projectionInfo), secondaryIdsPtr(
					secondaryIds), exporterInfoPtr(exporterInfo) {
	}

	~SyncStatus_Packet() {
		delete agentContentPtr;
		agentContentPtr = 0;

		if (projectionInfoPtr != 0) {
			for (std::map<std::string, std::vector<ProjectionInfoPacket*> >::iterator
					iter = projectionInfoPtr->begin(), iterEnd =
							projectionInfoPtr->end(); iter != iterEnd; ++iter) {
				for (std::vector<ProjectionInfoPacket*>::iterator PIPIter =
						iter->second.begin(), PIPIterEnd = iter->second.end();
						PIPIter != PIPIterEnd; ++PIPIter) {
					delete *PIPIter;
				}
			}
		}
		delete projectionInfoPtr;
		projectionInfoPtr = 0;

		delete secondaryIdsPtr;
		secondaryIdsPtr = 0;

	}

	/**
	 * This method includes a very odd construction that arises because
	 * the Packet _must_ delete the exporter info on the process to which
	 * it has been sent, but it _cannot_ delete the exporter info on the
	 * process from which it was sent. The solution is to call this function
	 * manually on the receiving process, but not call it on the sending proc.
	 *
	 * The pointer returned allows the abbreviation:
	 *
	 *    delete  instance.deleteExporterInfo();
	 */
	SyncStatus_Packet* deleteExporterInfo() {
		delete exporterInfoPtr;
		exporterInfoPtr = 0;
		return this;
	}

	template<class Archive>
	void serialize(Archive& ar, const unsigned int version) {
		ar & agentContentPtr;
		ar & projectionInfoPtr;
		ar & secondaryIdsPtr;
		ar & exporterInfoPtr;
	}

};

/**
 * Encapsulates the process in which repast is running and
 * manages interprocess communication etc. This is singleton to
 * insure that there is one per actual process.
 */
class RepastProcess: public boost::noncopyable {

public:
	enum EXCHANGE_PATTERN {
		POLL, USE_CURRENT, USE_LAST_OR_POLL, USE_LAST_OR_USE_CURRENT
	};

private:

	typedef boost::unordered_set<AgentId, HashId> MovedAgentSetType;

	static RepastProcess* _instance;

	boost::mpi::communicator* world;
	ScheduleRunner* runner;
	int rank_;
	int worldSize_;

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
	void initiateAgentRequest(AgentRequest& requests, std::string setName =
	DEFAULT_AGENT_REQUEST_SET, AGENT_IMPORTER_EXPORTER_TYPE setType =
	DEFAULT_ENUM_SYMBOL);
#endif

	std::vector<int>* procsToSendProjInfoTo;
	std::vector<int>* procsToRecvProjInfoFrom;

	std::vector<int>* procsToSendAgentStatusInfoTo;
	std::vector<int>* procsToRecvAgentStatusInfoFrom;

protected:
	RepastProcess(boost::mpi::communicator* comm = 0);

	void saveProjInfoSRProcs(std::vector<int>& sends, std::vector<int>& recvs) {
		if (procsToSendProjInfoTo == NULL) {
			procsToSendProjInfoTo = new std::vector<int>;
			procsToRecvProjInfoFrom = new std::vector<int>;
		} else {
			procsToSendProjInfoTo->clear();
			procsToRecvProjInfoFrom->clear();
		}
		procsToSendProjInfoTo->assign(sends.begin(), sends.end());
		procsToRecvProjInfoFrom->assign(recvs.begin(), recvs.end());
	}

	void saveAgentStatusInfoSRProcs(std::vector<int>& sends,
			std::vector<int>& recvs) {
		if (procsToSendAgentStatusInfoTo == NULL) {
			procsToSendAgentStatusInfoTo = new std::vector<int>;
			procsToRecvAgentStatusInfoFrom = new std::vector<int>;
		} else {
			procsToSendAgentStatusInfoTo->clear();
			procsToRecvAgentStatusInfoFrom->clear();
		}
		procsToSendAgentStatusInfoTo->assign(sends.begin(), sends.end());
		procsToRecvAgentStatusInfoFrom->assign(recvs.begin(), recvs.end());
	}

public:

	/**
	 * Initialize this RepastProcess. This must be called before the
	 * RepastProcess is used. If a configuration properties file is specified
	 * this properties file will be used to configure logging.
	 *
	 * @param propsfile a configuration properties file. This can be an
	 * empty string.
	 */
	static RepastProcess* init(std::string propsfile,
			boost::mpi::communicator* comm = 0, int maxConfigFileSize =
			MAX_CONFIG_FILE_SIZE);

	/**
	 * Gets this RepastProcess.
	 *
	 * @return this RepastProcess instance.
	 */
	static RepastProcess* instance();

	static boost::mpi::communicator* communicator();

	virtual ~RepastProcess();

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

	boost::mpi::communicator* getCommunicator() {
		return world;
	}

#ifdef SHARE_AGENTS_BY_SET
	void dropImporterExporterSet(std::string setName) {
		importer_exporter->dropSet(setName);
	}
#endif

	std::string ImporterExporterVersion() {
		return "" + importer_exporter->version();
	}

	std::string ImporterExporterReport() {
		return importer_exporter->getReport();
	}

	// Repast Process should handle four specific tasks for controlling parallelization:
	//
	// 1. Requesting agents that are to be shared
	// 2. Synchronizing the state values for shared agents
	// 3. Synchronizing the Projection info for all shared agents
	// 4. Synchronizing agent status values, including removing agents and moving them across processes
	//
	// At the end of all four of these operations, the simulation should be in a valid state
	// (Though not necessarily a 'current' state: synchronizing state values does NOT update Projection
	// information. This is a performance consideration: ideally we would always have the simulation in
	// a 100% current state, but because the simulation is being parallelized, a choice must always be made
	// about what information is updated.)

	/**
	 * Request agents from other processes.
	 */
	template<typename T, typename Content, typename Provider, typename Updater,
			typename AgentCreator>
	void requestAgents(SharedContext<T>& context, AgentRequest& request,
			Provider& provider, Updater& updater, AgentCreator& creator
#ifdef SHARE_AGENTS_BY_SET
			, std::string setName = DEFAULT_AGENT_REQUEST_SET,
			AGENT_IMPORTER_EXPORTER_TYPE setType = DEFAULT_ENUM_SYMBOL
#endif
			);

	/**
	 * Synchronizes the state values of shared agents. Does not change the Projection information
	 * for those agents.
	 */
	template<typename Content, typename Provider, typename Updater>
	void synchronizeAgentStates(Provider& provider, Updater& updater
#ifdef SHARE_AGENTS_BY_SET
			, std::string setName = REQUEST_AGENTS_ALL
#endif
			);

	/**
	 * Synchronizes the Projection information for shared projections.
	 */
	template<typename T, typename Content, typename Provider, typename Updater,
			typename AgentCreator>
	void synchronizeProjectionInfo(SharedContext<T>& context,
			Provider& provider, Updater& updater, AgentCreator& creator,
			EXCHANGE_PATTERN exchangePattern = POLL
#ifdef SHARE_AGENTS_BY_SET
			, bool declareNoAgentsKeptOnAnyProcess = false
#endif
			);

	/**
	 * Synchronizes the status (moved or died) of all agents across processes.
	 *
	 * @param context the SharedContext that contains the agents on this proceses
	 * @param provider the class that provides agents given an AgentRequest
	 * @param creator creates agents of type T given Content.
	 *
	 * @tparam T the type of agents in the context
	 * @tparam Content the serializable struct or class that describes
	 * an agents state.
	 * @tparam Provider a class that provides Content, when given an AgentRequest,
	 * implementing void provideContent(const repast::AgentRequest&, std::vector<Content>& out)
	 * @tparam AgentCreator a class that can create agents from Content, implementing
	 * T* createAgent(Content&).
	 *
	 */
	template<typename T, typename Content, typename Provider,
			typename AgentCreator, typename Updater>
	void synchronizeAgentStatus(SharedContext<T>& context, Provider& provider,
			Updater& updater, AgentCreator& creator,
			EXCHANGE_PATTERN exchangePattern = POLL);

};

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
template<typename T, typename Content, typename Provider, typename Updater,
		typename AgentCreator>
void RepastProcess::requestAgents(SharedContext<T>& context,
		AgentRequest& request, Provider& provider, Updater& updater,
		AgentCreator& creator
#ifdef SHARE_AGENTS_BY_SET
		, std::string setName, AGENT_IMPORTER_EXPORTER_TYPE setType
#endif
		) {

	// Initiate the new requests
#ifdef SHARE_AGENTS_BY_SET
	initiateAgentRequest(request, setName, setType);
#else
	initiateAgentRequest(request);
#endif

	// Establish which processes are sending to/receiving from this one
#ifdef SHARE_AGENTS_BY_SET
	const std::set<int>& exporters = importer_exporter->getExportingProcesses(
			setName);
	const std::map<int, AgentRequest>& agentsToExport =
			importer_exporter->getAgentsToExport(setName);
#else
	const std::set<int>& exporters = importer_exporter->getExportingProcesses();
	const std::map<int, AgentRequest>& agentsToExport = importer_exporter->getAgentsToExport();
#endif

	// Construct MPI requests (Receives and Sends)
	std::vector<boost::mpi::request> requests; // MPI Requests (receives and sends)

	// Construct Receives
	std::vector<Request_Packet<Content>*> toReceive;

	for (std::set<int>::const_iterator iter = exporters.begin();
			iter != exporters.end(); ++iter) {
		Request_Packet<Content>* packet;
		toReceive.push_back(packet = new Request_Packet<Content>());
		requests.push_back(world->irecv(*iter, 23, *packet));
	}

	// Construct Sends
	boost::ptr_list<Request_Packet<Content> >* toSend = new boost::ptr_list<
			Request_Packet<Content> >;

	for (std::map<int, AgentRequest>::const_iterator iter =
			agentsToExport.begin(); iter != agentsToExport.end(); ++iter) {

		// Agent Content
		std::vector<Content>* content = new std::vector<Content>;
		provider.provideContent(iter->second, *content);

		// Projection Info
		std::map<std::string, std::vector<ProjectionInfoPacket*> >* projInfo =
				new std::map<std::string,
						std::vector<repast::ProjectionInfoPacket*> >;
		context.getProjectionInfo(iter->second, *projInfo);

		Request_Packet<Content>* packet;
		toSend->push_back(
				packet = new Request_Packet<Content>(content, projInfo));
		requests.push_back(world->isend(iter->first, 23, *packet));
	}

	// Wait until all sends/receives complete
	boost::mpi::wait_all(requests.begin(), requests.end());

	// Clear sent data
	delete toSend;

	// Process (and delete) received data
	for (typename std::vector<Request_Packet<Content>*>::iterator iter =
			toReceive.begin(), iterEnd = toReceive.end(); iter != iterEnd;
			++iter) {
		std::vector<Content>* content = (*iter)->agentContentPtr;
		for (typename std::vector<Content>::const_iterator contentIter =
				content->begin(), contentIterEnd = content->end();
				contentIter != contentIterEnd; ++contentIter) {
			T* out = creator.createAgent(*contentIter);
			T* inContext = context.addAgent(out);
			if (inContext != out) { // This agent was already on this process
				updater.updateAgent(*contentIter);
				delete out;
			}
		}
		context.setProjectionInfo(*((*iter)->projectionInfoPtr));
		delete *iter;
	}

}

template<typename Content, typename Provider, typename Updater>
void RepastProcess::synchronizeAgentStates(Provider& provider, Updater& updater
#ifdef SHARE_AGENTS_BY_SET
		, std::string setName
#endif
		) {

	// Establish which processes are sending/receiving from this one
#ifdef SHARE_AGENTS_BY_SET
	const std::set<int>& processesToReceiveFrom =
			importer_exporter->getExportingProcesses(setName);
	const std::map<int, AgentRequest>& agentsToExport =
			importer_exporter->getAgentsToExport(setName);
#else
	const std::set<int>& processesToReceiveFrom = importer_exporter->getExportingProcesses();
	const std::map<int, AgentRequest>& agentsToExport = importer_exporter->getAgentsToExport();
#endif

	// Construct MPI Requests (sends and receives)
	std::vector<boost::mpi::request> requests;

	// Construct Receives
	std::vector<std::vector<Content>*> received;
	for (std::set<int>::const_iterator iter = processesToReceiveFrom.begin(),
			iterEnd = processesToReceiveFrom.end(); iter != iterEnd; ++iter) {
		std::vector<Content>* content = new std::vector<Content>();
		requests.push_back(world->irecv(*iter, 47, *content));
		received.push_back(content);
	}

	// Construct Sends
	boost::ptr_list<std::vector<Content> >* toSend = new boost::ptr_list<
			std::vector<Content> >;
	std::vector<Content>* content;

	for (std::map<int, AgentRequest>::const_iterator iter =
			agentsToExport.begin(), iterEnd = agentsToExport.end();
			iter != iterEnd; ++iter) {
		toSend->push_back(content = new std::vector<Content>);
		provider.provideContent(iter->second, *content);
		requests.push_back(world->isend(iter->first, 47, *content));
	}

	// Wait until all sends and receives are complete
	boost::mpi::wait_all(requests.begin(), requests.end());

	// Clear sent data
	delete toSend;

	// Process (and clear) received data
	for (typename std::vector<std::vector<Content>*>::iterator iter =
			received.begin(), iterEnd = received.end(); iter != iterEnd;
			++iter) {
		content = *iter;
		for (typename std::vector<Content>::const_iterator agentIter =
				content->begin(), agentIterEnd = content->end();
				agentIter != agentIterEnd; ++agentIter) {
			updater.updateAgent(*agentIter);
		}
		delete content;
	}

}

template<typename T, typename Content, typename Provider, typename Updater,
		typename AgentCreator>
void RepastProcess::synchronizeProjectionInfo(SharedContext<T>& context,
		Provider& provider, Updater& updater, AgentCreator& creator,
		EXCHANGE_PATTERN exchangePattern
#ifdef SHARE_AGENTS_BY_SET
		, bool declareNoAgentsKeptOnAnyProcess
#endif
		) {

	// Generate sets of agents to delete or not delete
	std::set<AgentId> agentsToKeep;

	bool agentsMayBeKept =
#ifdef SHARE_AGENTS_BY_SET
			context.keepsAgentsOnSyncProj() || !declareNoAgentsKeptOnAnyProcess;
#else
	context.keepsAgentsOnSyncProj();
#endif

	// If 'By Set': Construct 'Keep list' from all non-default I/E requests (agents being imported)
#ifdef SHARE_AGENTS_BY_SET
	if (declareNoAgentsKeptOnAnyProcess) {
		importer_exporter->clear();
	} else {
		importer_exporter->getSetOfAgentsBeingImported(agentsToKeep,
		DEFAULT_AGENT_REQUEST_SET);
		importer_exporter->clear(DEFAULT_AGENT_REQUEST_SET);
	}
#else
	importer_exporter->clear();
#endif

	// Determine all agents that the context doesn't need and are not on 'Keep' list, adding those that it needs to the 'Keep' list
	std::set<AgentId> agentsToDrop;
	context.getNonlocalAgentsToDrop(agentsToKeep, agentsToDrop);

	// Drop all of the agents that can be dropped
	std::set<AgentId>::iterator dropIter = agentsToDrop.begin(), dropIterEnd =
			agentsToDrop.end();
	while (dropIter != dropIterEnd) {
		context.removeAgent(*dropIter);
		agentRemoved(*dropIter);
		dropIter++;
	}

	// And drop all unneeded projection information, even for the agents being kept
	context.cleanProjectionInfo(agentsToKeep);

	// Initiate Agent Request (so that I/E will have agents needed by other processes)
	if (agentsMayBeKept) {
		AgentRequest req;
		for (std::set<AgentId>::iterator iter = agentsToKeep.begin(), iterEnd =
				agentsToKeep.end(); iter != iterEnd; ++iter) {
			req.addRequest(*iter); // TO DO: Better optimized constructor
		}
		initiateAgentRequest(req);  // Note: will use default I/E
	}

	// Determine which agents will be 'pushed' to other processes
	std::map<int, std::set<AgentId> > agentsToPush;
	context.getAgentsToPushToOtherProcesses(agentsToPush);

	// Add these to I/E as exports
	std::vector<AgentRequest> requests;
	for (std::map<int, std::set<AgentId> >::iterator iter =
			agentsToPush.begin(), iterEnd = agentsToPush.end(); iter != iterEnd;
			++iter) {
		AgentRequest req(iter->first);
		for (std::set<AgentId>::iterator i = iter->second.begin(), iEnd =
				iter->second.end(); i != iEnd; i++)
			req.addRequest(*i);
		requests.push_back(req);
	}

#ifdef SHARE_AGENTS_BY_SET
	importer_exporter->registerIncomingRequests(requests,
	DEFAULT_AGENT_REQUEST_SET);
#else
	importer_exporter->registerIncomingRequests(requests);
#endif

	// Exchange agent & projection information
	// Establish which processes are sending to/receiving from this one
#ifdef SHARE_AGENTS_BY_SET
	const std::map<int, AgentRequest>& tmpAgentsToExport =
			importer_exporter->getAgentsToExport(DEFAULT_AGENT_REQUEST_SET);
#else
	const std::map<int, AgentRequest>& tmpAgentsToExport = importer_exporter->getAgentsToExport();
#endif

	std::map<int, AgentRequest> agentsToExport = tmpAgentsToExport; // Copy?

	std::vector<int> psToSendTo;
	std::vector<int> psToReceiveFrom;
	if (exchangePattern == USE_CURRENT
			|| ((exchangePattern == USE_LAST_OR_USE_CURRENT)
					&& (procsToSendProjInfoTo == NULL))) {
		std::set<int> sends, recvs;
		context.getProjInfoExchangePartners(sends, recvs);
		psToSendTo.assign(sends.begin(), sends.end());
		psToReceiveFrom.assign(recvs.begin(), recvs.end());
		// Add dummy requests to create empty sends
		for (std::vector<int>::iterator iter = psToSendTo.begin(), iterEnd =
				psToSendTo.end(); iter != iterEnd; ++iter) {
			int dest = *iter;
			if (agentsToExport.find(dest) == agentsToExport.end()) {
				AgentRequest dummy(rank_, dest);
				agentsToExport[dest] = dummy;
			}
		}
	} else if (exchangePattern == POLL
			|| ((exchangePattern == USE_LAST_OR_POLL)
					&& (procsToSendProjInfoTo == NULL))) {
		for (std::map<int, AgentRequest>::const_iterator iter =
				agentsToExport.begin(), iterEnd = agentsToExport.end();
				iter != iterEnd; ++iter) {
			psToSendTo.push_back(iter->first);
		}
		SRManager manager(world);
		manager.retrieveSources(psToSendTo, psToReceiveFrom,
				AGENT_MOVED_SENDERS);
	} else {
		psToSendTo.assign(procsToSendProjInfoTo->begin(),
				procsToSendProjInfoTo->end());
		psToReceiveFrom.assign(procsToRecvProjInfoFrom->begin(),
				procsToRecvProjInfoFrom->end());

		// Add dummy requests to create empty sends
		for (std::vector<int>::iterator iter = psToSendTo.begin(), iterEnd =
				psToSendTo.end(); iter != iterEnd; ++iter) {
			int dest = *iter;
			if (agentsToExport.find(dest) == agentsToExport.end()) {
				AgentRequest dummy(rank_, dest);
				agentsToExport[dest] = dummy;
			}
		}
	}

	saveProjInfoSRProcs(psToSendTo, psToReceiveFrom);

	// Construct MPI requests (Receives and Sends)
	std::vector<boost::mpi::request> MPIRequests; // MPI Requests (receives and sends)

	// Construct Receives
	std::map<int, Request_Packet<Content>*> toReceive;

	for (std::vector<int>::iterator iter = psToReceiveFrom.begin(), iterEnd =
			psToReceiveFrom.end(); iter != iterEnd; ++iter) {
		Request_Packet<Content>* packet;
		toReceive[*iter] = (packet = new Request_Packet<Content>());
		MPIRequests.push_back(world->irecv(*iter, 23, *packet));
	}

	// Construct Sends
	boost::ptr_list<Request_Packet<Content> >* toSend = new boost::ptr_list<
			Request_Packet<Content> >;

	for (std::map<int, AgentRequest>::const_iterator iter =
			agentsToExport.begin(), iterEnd = agentsToExport.end();
			iter != iterEnd; ++iter) {
		int dest = iter->first;
		const AgentRequest& rq = iter->second;

		// Agent Content
		std::vector<Content>* contentVector = new std::vector<Content>;
		provider.provideContent(rq, *contentVector);

		// Projection Info
		std::map<std::string, std::vector<ProjectionInfoPacket*> >* projInfo =
				new std::map<std::string,
						std::vector<repast::ProjectionInfoPacket*> >;
		context.getProjectionInfo(rq, *projInfo, true, 0, dest); // Will collect the edges but not the secondary IDs

		Request_Packet<Content>* packet;
		toSend->push_back(
				packet = new Request_Packet<Content>(contentVector, projInfo));
		MPIRequests.push_back(world->isend(dest, 23, *packet));
	}

	// Wait until all sends/receives complete
	boost::mpi::wait_all(MPIRequests.begin(), MPIRequests.end());

	// Clear sent data
	delete toSend;

	// Process received data (and clear)
	for (typename std::map<int, Request_Packet<Content>*>::iterator iter =
			toReceive.begin(), iterEnd = toReceive.end(); iter != iterEnd;
			++iter) {
		std::vector<Content>* contentVector = iter->second->agentContentPtr;
		AgentRequest requestToRegister(iter->first);
		for (typename std::vector<Content>::const_iterator contentIter =
				contentVector->begin(), contentIterEnd = contentVector->end();
				contentIter != contentIterEnd; ++contentIter) {
			T* newAgent = creator.createAgent(*contentIter);
			T* agentInContext = context.addAgent(newAgent);
			if (agentInContext != newAgent) { // This agent was already on this process
				updater.updateAgent(*contentIter);
				delete newAgent;
			} else {
				// Add the agent to the agent request that will be processed as if it were an OUTGOING request
				requestToRegister.addRequest(agentInContext->getId());
			}
		}

		context.setProjectionInfo(*(iter->second->projectionInfoPtr));
		delete iter->second;
		// Register these as requests, so that the importer/exporter will know these agents will be sent
		importer_exporter->registerOutgoingRequests(requestToRegister);
	}
}

template<typename T, typename Content, typename Provider, typename AgentCreator,
		typename Updater>
void RepastProcess::synchronizeAgentStatus(SharedContext<T>& context,
		Provider& provider, Updater& updater, AgentCreator& creator,
		EXCHANGE_PATTERN exchangePattern) {

	// Step 1: Exchange information about agents whose status will be updated.
	//
	// Status Updates have been added to the importer_exporter and can now be exchanged
	// across processes. All processes will now know that an agent that has previously
	// been managed by process A will either be eliminated from the simulation or
	// moved to process B.
	std::vector<std::vector<AgentStatus>*> statusUpdates;
	importer_exporter->exchangeAgentStatusUpdates(*world, statusUpdates);

	for (size_t i = 0, n = statusUpdates.size(); i < n; i++) {
		std::vector<AgentStatus>* vec = statusUpdates[i];
		for (size_t j = 0, k = vec->size(); j < k; ++j) {
			AgentStatus& status = (*vec)[j];
			if (status.getStatus() == AgentStatus::REMOVED) {
				importer_exporter->importedAgentIsRemoved(status.getOldId()); // Notify importer/exporter that this agent will not be imported anymore
				context.importedAgentRemoved(status.getOldId()); // Remove from context; agent cannot exist on this process after removal from home process
			} else if (status.getStatus() == AgentStatus::MOVED) {
				if (rank_ != status.getNewId().currentRank()) {
					// Notify importer that this agent will not be imported from the original
					// process, but will instead be imported from its new home
					importer_exporter->importedAgentIsMoved(status.getOldId(),
							status.getNewId().currentRank());
				} else {
					// Notify importer that the agent will not be imported anymore because this is its home process now
					importer_exporter->importedAgentIsNowLocal(
							status.getOldId());
				}
				// Find it and update its id
				T* agent = context.getAgent(status.getOldId());
				if (agent == (void*) 0)
					throw Repast_Error_32<AgentId>(status.getOldId()); // Agent not found
				agent->getId().currentRank(status.getNewId().currentRank());
			}
		}
		delete vec;
	}

	// Step 2: Send moving agents' information to new home processes
	//
	// First, some basic data structures must be created for some bookkeeping we will need later
	std::set<AgentId> agentsToDrop; // A list of agents that will be removed from this process
	std::set<int> psMovedTo; // A list of the processes that will be receiving moving agents
	std::map<int, AgentRequest> agentRequests; // A map of these receiving processes and a list of the IDs of the agents going to them

	// This loop applies to all agents moving off of this process (from 'movedAgents'); it:
	//     Re-sets the 'current Process' ID values of the agents moving away from this process to their new values
	//     Adds the agents to the lists of agents to be removed from this process
	//     Creates the map of agent requests per receiving process and adds the agents to it
	for (MovedAgentSetType::const_iterator iter = movedAgents.begin(), iterEnd =
			movedAgents.end(); iter != iterEnd; ++iter) {
		AgentId id = *iter;
		context.getAgent(id)->getId().currentRank(id.currentRank());
		agentsToDrop.insert(id);
		int currentProc = id.currentRank();
		if (psMovedTo.insert(currentProc).second) {
			AgentRequest req = AgentRequest(currentProc, rank_);
			req.addRequest(id);
			agentRequests[currentProc] = req;
		} else
			agentRequests[currentProc].addRequest(id);
	}
	movedAgents.clear();

	// Next, coordinate the send/receive pairs (which processes send to which)
	//
	// Three different methods can be used:
	//     1) POLL: Assume no knowledge of which processes will be sending to this one; do a full exchange
	//          where all processes inform all other processes of whether they need to send to them
	//     2) USE_CURRENT: Assume that the context (and, by implication, all projections in the context)
	//          know which processes they need to send to and which they will receive from. For example, a grid
	//          projection will know its 8 neighbors; a graph projection _might_ know which processes own
	//          nodes to which it is connected
	//     3) USE_LAST: Assume that the set of processes to which sends go and from which sends are received
	//          will be unchanged from the last time this loop was run; once established, just keep using
	//          the same set
	//
	// Variants 'USE_LAST_OR_POLL' and 'USE_LAST_OR_USE_CURRENT' allow for the case in which one method (either
	// POLL or USE_CURRENT) is used for the initial pass through the loop and thereafter the sets are assumed
	// unchanged.
	std::vector<int> psToSendTo;        // Convert set to vector
	std::vector<int> psToReceiveFrom;

	if (exchangePattern == USE_CURRENT
			|| ((exchangePattern == USE_LAST_OR_USE_CURRENT)
					&& (procsToSendAgentStatusInfoTo == NULL))) {
		std::set<int> sends, recvs;
		context.getAgentStatusInfoExchangePartners(sends, recvs);
		psToSendTo.assign(sends.begin(), sends.end());
		psToReceiveFrom.assign(recvs.begin(), recvs.end());
		// Add dummy requests to create empty sends
		for (std::vector<int>::iterator iter = psToSendTo.begin(), iterEnd =
				psToSendTo.end(); iter != iterEnd; ++iter) {
			int dest = *iter;
			if (agentRequests.find(dest) == agentRequests.end()) {
				AgentRequest dummy(rank_, dest);
				agentRequests[dest] = dummy;
			}
		}
	} else if (exchangePattern == POLL
			|| ((exchangePattern == USE_LAST_OR_POLL)
					&& (procsToSendAgentStatusInfoTo == NULL))) {
		for (std::map<int, AgentRequest>::const_iterator iter =
				agentRequests.begin(), iterEnd = agentRequests.end();
				iter != iterEnd; ++iter) {
			psToSendTo.push_back(iter->first);
		}
		SRManager manager(world);
		manager.retrieveSources(psToSendTo, psToReceiveFrom,
				AGENT_MOVED_SENDERS);
	} else {
		psToSendTo.assign(procsToSendAgentStatusInfoTo->begin(),
				procsToSendAgentStatusInfoTo->end());
		psToReceiveFrom.assign(procsToRecvAgentStatusInfoFrom->begin(),
				procsToRecvAgentStatusInfoFrom->end());

		// Add dummy requests to create empty sends
		for (std::vector<int>::iterator iter = psToSendTo.begin(), iterEnd =
				psToSendTo.end(); iter != iterEnd; ++iter) {
			int dest = *iter;
			if (agentRequests.find(dest) == agentRequests.end()) {
				AgentRequest dummy(rank_, dest);
				agentRequests[dest] = dummy;
			}
		}
	}

	saveAgentStatusInfoSRProcs(psToSendTo, psToReceiveFrom);

	// Determine if any projection in the context will need to send 'secondary' agent data:
	bool sendSecondaryData = context.sendsSecondaryDataOnStatusExchange();

	// Create MPI Sends and Receives
	std::vector<boost::mpi::request> requests;

	// STEP 5: Create the receives
	std::vector<SyncStatus_Packet<Content>*> packetsRecd;

	for (std::vector<int>::const_iterator iter = psToReceiveFrom.begin();
			iter != psToReceiveFrom.end(); ++iter) {
		int source = *iter;
		SyncStatus_Packet<Content>* packetToRecv =
				new SyncStatus_Packet<Content>;
		requests.push_back(
				world->irecv(source, AGENT_MOVED_AGENT, *packetToRecv));
		packetsRecd.push_back(packetToRecv);
	}

	// STEP 6: Assemble data to send
	boost::ptr_list<SyncStatus_Packet<Content> >* packetsToSend =
			new boost::ptr_list<SyncStatus_Packet<Content> >;

	for (std::map<int, repast::AgentRequest>::iterator iter =
			agentRequests.begin(); iter != agentRequests.end(); ++iter) {
		// Agent Content
		std::vector<Content>* content = new std::vector<Content>;
		provider.provideContent(iter->second, *content);

		// Projection Info and Secondary Ids
		std::map<std::string, std::vector<ProjectionInfoPacket*> >* projInfo =
				new std::map<std::string,
						std::vector<repast::ProjectionInfoPacket*> >;
		std::set<AgentId>* secondaryIds = (
				sendSecondaryData ? new std::set<AgentId> : 0);
		context.getProjectionInfo(iter->second, *projInfo, sendSecondaryData,
				secondaryIds);

		// Send the information for the secondary agents, too:
		if (secondaryIds != 0) { // use 'sendSecondaryData' instead? Should be equivalent...
			AgentRequest sidReq;
			for (std::set<AgentId>::iterator sidIter = secondaryIds->begin(),
					sidIterEnd = secondaryIds->end(); sidIter != sidIterEnd;
					++sidIter)
				sidReq.addRequest(*sidIter);
			provider.provideContent(sidReq, *content); // Only their state data is needed, not any projection info
		}

		// Agent Exporter Info
		AgentExporterInfo* agentImporterInfoPtr =
				importer_exporter->getAgentExportInfo(iter->first);

		SyncStatus_Packet<Content>* packetToSend;
		packetsToSend->push_back(
				packetToSend = new SyncStatus_Packet<Content>(content, projInfo,
						secondaryIds, agentImporterInfoPtr));

		requests.push_back(
				world->isend(iter->first, AGENT_MOVED_AGENT, *packetToSend));
	}
	boost::mpi::wait_all(requests.begin(), requests.end());
	delete packetsToSend;

	importer_exporter->clearAgentExportInfo();

	// STEP 9: Remove the agents that are moving to other processes and are not needed here
	std::set<AgentId> agentsToKeep;
	context.getRequiredAgents(agentsToDrop, agentsToKeep,
			Projection<T>::SECONDARY);

	for (std::set<AgentId>::iterator idIter = agentsToDrop.begin(), idIterEnd =
			agentsToDrop.end(); idIter != idIterEnd; ++idIter)
		context.removeAgent(*idIter);

	// STEP 10: Insert the newly received agents that moved to this process and update exporters
	typename std::vector<SyncStatus_Packet<Content>*>::iterator packetIter;
	typename std::vector<SyncStatus_Packet<Content>*>::iterator packetIterEnd =
			packetsRecd.end();
	AgentRequest secondaryAgentsToRequest(rank_);
	for (packetIter = packetsRecd.begin(); packetIter != packetIterEnd;
			++packetIter) {
		std::vector<Content>* content = (*packetIter)->agentContentPtr;
		typename std::vector<Content>::iterator contentIter = content->begin();
		while (contentIter != content->end()) {
			T* out = creator.createAgent(*contentIter);
			T* inContext = context.addAgent(out);
			if (inContext != out) { // Already contain the agent
				// If the agent is local on this rank, do nothing (the agent received must be a secondary agent)
				// If the agent is non-local on this rank
				if (inContext->getId().currentRank() != rank_) {
					// If the arriving agent has a current rank equal to this rank, this
					// is an incoming, newly arrived local agent that already existed on this
					// process as a secondary agent; it should be updated and its currentRank in
					// its ID set to the local rank
					if (out->getId().currentRank() == rank_) {
						updater.updateAgent(*contentIter);
						inContext->getId().currentRank(rank_);
					}
					// Otherwise, it's a secondary agent arriving from another process, when it
					// already exists as a non-local agent on this process; leave the original alone
					// and discard the new version.
				}
				delete out;
			} else { // Agent was not already on this rank and is not a new local agent; must process it as a new request
				if (out->getId().currentRank() != rank_)
					secondaryAgentsToRequest.addRequest(out->getId());
			}
			contentIter++;
		}

		// Update the importer/exporter to reflect the newly arrived local agents
		importer_exporter->incorporateAgentExporterInfo(
				*((*packetIter)->exporterInfoPtr));
		importer_exporter->clearExportToSpecificProc(rank_); // Export Info may include 'exports' to self; remove these

	}

	// STEP 11: Newly received secondary agents must be coordinated with current processes
	if (sendSecondaryData)
		initiateAgentRequest(secondaryAgentsToRequest);

	// STEP 12: Set all the Projection Info, including graph edges, and clear the received data
	for (packetIter = packetsRecd.begin(); packetIter != packetIterEnd;
			++packetIter) {
		context.setProjectionInfo(*((*packetIter)->projectionInfoPtr));
		delete (*packetIter)->deleteExporterInfo(); // Exporter Info is only deleted from the received packets, not the sent ones...
	}

}

}

#endif /* REPASTPROCESS_H_ */
