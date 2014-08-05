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
 *  SharedContext.h
 *
 *  Created on: Jun 18, 2009
 *      Author: nick
 */

#ifndef SHAREDCONEXT_HPP_
#define SHAREDCONEXT_HPP_

#include "Context.h"
#include "RepastErrors.h"

#include <boost/mpi.hpp>
#include <exception>

namespace repast {

/**
 * Used in a filter iterator to filter on local agents only
 */
template<typename T>
struct IsLocalAgent {
	int rank;
	IsLocalAgent(int rankInCommunicator): rank(rankInCommunicator){ 	}

	bool operator()(const boost::shared_ptr<T>& ptr) {
		return ptr->getId().currentRank() == rank;
	}

};

/**
 * Used in a filter iterator to filter on local or non-local agents only
 */
template<typename T>
struct AgentStateFilter{
	int rank;
	bool local;
	AgentStateFilter(int rankInCommunicator): rank(rankInCommunicator){
		local = true;
	}
	AgentStateFilter(bool localFlag, int rankInCommunicator):  rank(rankInCommunicator), local(localFlag) {	}
		
	bool operator()(const boost::shared_ptr<T>& ptr) {
		return (local ? ptr->getId().currentRank() == rank : ptr->getId().currentRank() != rank);
	}
};
//
///*
// * An instance of the AgentStateFilter that filters for local agents
// */
//template<typename T>
//struct LocalFilter: public AgentStateFilter<T>{
//	LocalFilter(int rank): AgentStateFilter<T>(true, rank){}
//};
//
///*
// * An instance of the AgentStateFilter that filters for nonlocal agents
// */
//template<typename T>
//struct NonLocalFilter: public AgentStateFilter<T>{
//	NonLocalFilter(int rank): AgentStateFilter<T>(false, rank){}
//};
//
	
/**
 * Used to remove agents.
 */
void rpRemoveAgent(const AgentId& id);


/**
 * Context implementation specialized for the parallel distributed
 * simulation. A SharedContext contains both local, that is, agents
 * whose behavior is run on the SharedContext's process and foreign agents,
 * that is, copies of agents whose behavior is run on some other process.
 *
 * @param T the type of agents in the context.
 */
template<typename T>
class SharedContext: public Context<T> {

private:

	typedef typename boost::unordered_map<AgentId, int, HashId> RefMap;

	// holds reference count to foreign agents that are
	// referenced by projections. If a projection removes an
	// agent from a context, this should be checked to make sure
	// no other projections hold a reference before actually deleting
	// the agent.
	RefMap projRefMap;
	int _rank;

public:

	// Create single instances for these and reuse them
  IsLocalAgent<T>      localPredicate;
  AgentStateFilter<T>  LOCAL_FILTER;
  AgentStateFilter<T>  NON_LOCAL_FILTER;

  // For more efficient 'push' during projection sync
  std::vector<std::string> getAgentsToPushProjOrder;

	typedef typename boost::filter_iterator<IsLocalAgent<T> , typename Context<T>::const_iterator> const_local_iterator;

  typedef typename boost::filter_iterator<AgentStateFilter<T> , typename Context<T>::const_iterator>        const_state_aware_iterator;
  typedef typename boost::filter_iterator<AgentStateFilter<T> , typename Context<T>::const_bytype_iterator> const_state_aware_bytype_iterator;

  typedef typename Projection<T>::RADIUS RADIUS;

	SharedContext(boost::mpi::communicator* comm);
	virtual ~SharedContext();

	/**
	 * Gets the start of iterator over the local agents in this context.
	 * The iterator derefrences into shared_ptr<T>. The actual
	 * agent can be accessed by dereferencing the iter: (*iter)->getId() for example.
	 *
	 * @return the start of iterator over the local agents in this context.
	 */
	const_local_iterator localBegin() const;

	/**
	 * Gets the end of an iterator over the local agents in this context.
	 * The iterator derefrences into shared_ptr<T>. The actual
	 * agent can be accessed by derefrenceing the iter: (*iter)->getId() for example.
	 */
	const_local_iterator localEnd() const;

	/**
	 * Removes the specified agent from this context. If the
	 * agent is non-local, this checks to make sure that it
	 * is not referenced by any projection before its removed.
	 *
	 * @param id the id of the agent to remove
	 */
	void removeAgent(const AgentId id);

	/**
	 * Removes the specified agent from this context. If the
	 * agent is non-local, this checks to make sure that it
	 * is not referenced by any projection before its removed.
	 *
	 * @param agent the agent to remove
	 */
	void removeAgent(T* agent);

	/**
	 * Notifies this context that the specified non-local agent
	 * has been removed and this context should then delete that
	 * agent from itself.
	 *
	 * @param id the id of the agent that was removed
	 */
	void importedAgentRemoved(const AgentId& id);

	/**
	 * Increments the projection reference count for the specified
	 * agent.
	 *
	 * @param id the id of the agent
	 */
	void incrementProjRefCount(const AgentId& id);

	/**
	 * Decrements the projection reference count for the specified agent.
	 *
	 * @param id the id of the agent
	 */
	void decrementProjRefCount(const AgentId& id);

	/*
	 * Used as an argument to the 'selectAgents' routines;
	 * cannot use an 'int' because doing so would mask
	 * the different versions of these routines
	 */
    enum filterLocalFlag{
        LOCAL     = 1,
        NON_LOCAL = 0
    };
	
	
    // Unhide these from the parent class so operation is transparent
    using Context<T>::begin;
    using Context<T>::end;
    using Context<T>::byTypeBegin;
    using Context<T>::byTypeEnd;
    using Context<T>::filteredBegin;
    using Context<T>::filteredEnd;
    using Context<T>::byTypeFilteredBegin;
    using Context<T>::byTypeFilteredEnd;
    using Context<T>::size;
	
	/**
	 * Gets the start of an iterator that will iterate over only local or non-local agents
	 *
	 * @param local flag indicating whether local or non-local agents are to be included
	 */
    const_state_aware_iterator begin(filterLocalFlag local);
    
	/**
	 * Gets the end of an iterator that will iterate over only local or non-local agents
	 *
	 * @param local flag indicating whether local or non-local agents are to be included	 
	 */
    const_state_aware_iterator end(filterLocalFlag local);
	
	/**
	 * Gets the start of an iterator that will iterate over only local or non-local agents of a certain
	 * type (per their AgentId value)
	 *
 	 * @param local flag indicating whether local or non-local agents are to be included
	 * @param type type to included
	 */
	const_state_aware_bytype_iterator byTypeBegin(filterLocalFlag local, int type);

	/**
	 * Gets the end of an iterator that will iterate over only local or non-local agents of a certain
	 * type (per their AgentId value)
	 *
	 * @param local flag indicating whether local or non-local agents are to be included	 
	 * @param type type to included
	 */
	const_state_aware_bytype_iterator byTypeEnd(filterLocalFlag local, int type);
	
	
  /**
	 * Gets the start of an iterator that will iterate over only local or non-local agents meeting
	 * the criteria of the user-defined struct (see IsLocalAgent for an example)
	 *
	 * @param local flag indicating whether local or non-local agents are to be included	 
	 * @param filter struct with unary operator (boost::shared_ptr<T>) that returns
	 * true or false; used to selectively include agents.
	 *
	 * @tparam filterStruct the class of the filter to be used
	 */
	template<typename filterStruct>
    boost::filter_iterator<filterStruct, typename SharedContext<T>::const_state_aware_iterator> filteredBegin(filterLocalFlag local, filterStruct& fStruct);

  /**
	 * Gets the end of an iterator that will iterate over only local or non-local agents meeting
	 * the criteria of the user-defined struct (see IsLocalAgent for an example)
	 *
	 * @param local flag indicating whether local or non-local agents are to be included	 
	 * @param filter struct with unary operator (boost::shared_ptr<T>) that returns
	 * true or false; used to selectively include agents.
	 *
	 * @tparam filterStruct the class of the filter to be used
	 */
	template<typename filterStruct>
    boost::filter_iterator<filterStruct, typename SharedContext<T>::const_state_aware_iterator> filteredEnd(filterLocalFlag local, filterStruct& fStruct);

	
  /**
	 * Gets the start of an iterator that will iterate over only local or non-local agents
	 * of the specified type and meeting the criteria of the user-defined struct (see IsLocalAgent for an example)
	 *
	 * @param local flag indicating whether local or non-local agents are to be included	 
	 * @param type type to be included
	 * @param filter struct with unary operator (boost::shared_ptr<T>) that returns
	 * true or false; used to selectively include agents.
	 *
	 * @tparam filterStruct the class of the filter to be used
	 */
    template<typename filterStruct>
    boost::filter_iterator<filterStruct, typename SharedContext<T>::const_state_aware_bytype_iterator> byTypeFilteredBegin(filterLocalFlag local, int type, filterStruct& fStruct);
    
	/**
	 * Gets the end of an iterator that will iterate over only local or non-local agents
	 * of the specified type and meeting the criteria of the user-defined struct (see IsLocalAgent for an example)
	 *
	 * @param local flag indicating whether local or non-local agents are to be included
	 * @param type type to be included
	 * @param filter struct with unary operator (boost::shared_ptr<T>) that returns
	 * true or false; used to selectively include agents.
	 *
	 * @tparam filterStruct the class of the filter to be used
	 */
	template<typename filterStruct>
    boost::filter_iterator<filterStruct, typename SharedContext<T>::const_state_aware_bytype_iterator> byTypeFilteredEnd(filterLocalFlag local, int type, filterStruct& fStruct);
	
	
    // Select Methods
	
    // Unhide the parent class's versions
    using Context<T>::selectAgents;
	
	
	/**
	 * Gets a set of pointers to all local or non-local agents in this context.
	 *
	 * If the 'remove' parameter is set to true, any elements in the
	 * original set will be removed before the method returns.
	 *
	 * The popSize parameter is used when the method is repeatedly called
	 * on a population whose size is known. Calls to this method typically begin
	 * by determining the size of the (valid) population to be sampled; if this is known,
	 * it can be provided here, improving performance.
	 *
	 * @param localOrNonLocalOnly flag that indicates that the agents selected
	 * will be drawn only from agents either local or non-local to this process
	 * @param [out] selectedAgents a set into which the pointers to the agents will be placed
	 * @param remove if true, remove any elements originally in the set before the
	 * set is returned (default is false)
	 * @param popSize size of the population from which the sample will be drawn
	 */
  void selectAgents(filterLocalFlag localOrNonLocalOnly, std::set<T*>& selectedAgents, bool remove = false, int popSize = -1);

  /**
	 * Gets a randomly ordered vector of pointers to all local or non-local agents in this context.
	 *
	 * If the 'remove' parameter is set to true, any elements in the
	 * original vector will be removed before the method returns.
	 *
	 * The popSize parameter is used when the method is repeatedly called
	 * on a population whose size is known. Calls to this method typically begin
	 * by determining the size of the (valid) population to be sampled; if this is known,
	 * it can be provided here, improving performance.
	 *
	 * @param localOrNonLocalOnly flag that indicates that the agents selected
	 * will be drawn only from agents either local or non-local to this process
	 * @param [out] selectedAgents a vector into which the pointers to the agents will be placed
	 * @param remove if true, remove any elements originally in the set before the
	 * set is returned (default is false)
	 * @param popSize size of the population from which the sample will be drawn
	 */
	void selectAgents(filterLocalFlag localOrNonLocalOnly, std::vector<T*>& selectedAgents, bool remove = false, int popSize = -1);

	/**
	 * Gets a set of pointers to a specified number of randomly selected
	 * local or non-local agents.
	 *
	 * If the set passed contains any elements when this method is called,
	 * the agents pointed to by those elements will be omitted from the selection.
	 *
	 * If the 'remove' parameter is set to true, any elements in the
	 * original set will be removed before the method returns.
	 *
	 * The popSize parameter is used when the method is repeatedly called
	 * on a population whose size is known. Calls to this method typically begin
	 * by determining the size of the (valid) population to be sampled; if this is known,
	 * it can be provided here, improving performance.
	 *
	 * @param localOrNonLocalOnly flag that indicates that the agents selected
	 * will be drawn only from agents either local or non-local to this process
	 * @param count the number of agents to be selected. If this exceeds the number
	 * that can possibly be selected, all possible agents will be selected
	 * @param [out] selectedAgents a set into which the pointers to the agents will be placed
	 * @param remove if true, remove any elements originally in the set before the
	 * set is returned (default is false)
	 * @param popSize size of the population from which the sample will be drawn
	 */
  void selectAgents(filterLocalFlag localOrNonLocalOnly, int count, std::set<T*>& selectedAgents, bool remove = false, int popSize = -1);

	/**
	 * Gets a randomly ordered vector of pointers to a specified number of randomly selected
	 * local or non-local agents.
	 *
	 * If the vector passed contains any elements when this method is called,
	 * the agents pointed to by those elements will be omitted from the selection.
	 *
	 * If the 'remove' parameter is set to true, any elements in the
	 * original vector will be removed before the method returns.
	 *
	 * The popSize parameter is used when the method is repeatedly called
	 * on a population whose size is known. Calls to this method typically begin
	 * by determining the size of the (valid) population to be sampled; if this is known,
	 * it can be provided here, improving performance.
	 *
	 * @param localOrNonLocalOnly flag that indicates that the agents selected
	 * will be drawn only from agents either local or non-local to this process
	 * @param count the number of agents to be selected. If this exceeds the number
	 * that can possibly be selected, all possible agents will be selected
	 * @param [out] selectedAgents a vector into which the pointers to the agents will be placed
	 * @param remove if true, remove any elements originally in the set before the
	 * set is returned (default is false)
	 * @param popSize size of the population from which the sample will be drawn
	 */
	void selectAgents(filterLocalFlag localOrNonLocalOnly, int count, std::vector<T*>& selectedAgents, bool remove = false, int popSize = -1);
	
	/**
	 * Gets a set of pointers to all local or non-local agents in this context
	 * of a specified type (per their AgentId values).
	 *
	 * If the 'remove' parameter is set to true, any elements in the
	 * original set will be removed before the method returns.
	 *
	 * The popSize parameter is used when the method is repeatedly called
	 * on a population whose size is known. Calls to this method typically begin
	 * by determining the size of the (valid) population to be sampled; if this is known,
	 * it can be provided here, improving performance.
	 *
	 * @param localOrNonLocalOnly flag that indicates that the agents selected
	 * will be drawn only from agents either local or non-local to this process
	 * @param [out] selectedAgents a set into which the pointers to the agents will be placed
	 * @param type numeric type of agent to be selected
	 * @param remove if true, remove any elements originally in the set before the
	 * set is returned (default is false)
	 * @param popSize size of the population from which the sample will be drawn
	 */
	void selectAgents(filterLocalFlag localOrNonLocalOnly, std::set<T*>& selectedAgents, int type, bool remove = false, int popSize = -1);

	/**
	 * Gets a randomly ordered vector of pointers to all local or non-local agents in this context
	 * of a specified type (per their AgentId values).
	 *
	 * If the 'remove' parameter is set to true, any elements in the
	 * original vector will be removed before the method returns.
	 *
	 * The popSize parameter is used when the method is repeatedly called
	 * on a population whose size is known. Calls to this method typically begin
	 * by determining the size of the (valid) population to be sampled; if this is known,
	 * it can be provided here, improving performance.
	 *
	 * @param localOrNonLocalOnly flag that indicates that the agents selected
	 * will be drawn only from agents either local or non-local to this process
	 * @param [out] selectedAgents a vector into which the pointers to the agents will be placed
	 * @param type numeric type of agent to be selected
	 * @param remove if true, remove any elements originally in the set before the
	 * set is returned (default is false)
	 * @param popSize size of the population from which the sample will be drawn
	 */
	void selectAgents(filterLocalFlag localOrNonLocalOnly, std::vector<T*>& selectedAgents, int type, bool remove = false, int popSize = -1);

	/**
	 * Gets a set of pointers to a specified number of randomly selected local or non-local agents
	 * of a specified type (per their AgentId values).
	 *
	 * If the set passed contains any elements when this method is called,
	 * the agents pointed to by those elements will be omitted from the selection.
	 *
	 * If the 'remove' parameter is set to true, any elements in the
	 * original set will be removed before the method returns.
	 *
	 * The popSize parameter is used when the method is repeatedly called
	 * on a population whose size is known. Calls to this method typically begin
	 * by determining the size of the (valid) population to be sampled; if this is known,
	 * it can be provided here, improving performance.
	 *
	 * @param localOrNonLocalOnly flag that indicates that the agents selected
	 * will be drawn only from agents either local or non-local to this process
	 * @param count the number of agents to be selected. If this exceeds the number
	 * that can possibly be selected, all possible agents will be selected
	 * @param [out] selectedAgents a set into which the pointers to the agents will be placed
	 * @param type numeric type of agent to be selected
	 * @param remove if true, remove any elements originally in the set before the
	 * set is returned (default is false)
	 * @param popSize size of the population from which the sample will be drawn
	 */
	void selectAgents(filterLocalFlag localOrNonLocalOnly, int count, std::set<T*>& selectedAgents, int type, bool remove = false, int popSize = -1);

	/**
	 * Gets a randomly ordered vector of pointers to a specified number of randomly selected
	 * local or non-local agents of a specified type (per their AgentId values).
	 *
	 * If the vector passed contains any elements when this method is called,
	 * the agents pointed to by those elements will be omitted from the selection.
	 *
	 * If the 'remove' parameter is set to true, any elements in the
	 * original vector will be removed before the method returns.
	 *
	 * The popSize parameter is used when the method is repeatedly called
	 * on a population whose size is known. Calls to this method typically begin
	 * by determining the size of the (valid) population to be sampled; if this is known,
	 * it can be provided here, improving performance.
	 *
	 * @param localOrNonLocalOnly flag that indicates that the agents selected
	 * will be drawn only from agents either local or non-local to this process
	 * @param count the number of agents to be selected. If this exceeds the number
	 * that can possibly be selected, all possible agents will be selected
	 * @param [out] selectedAgents a vector into which the pointers to the agents will be placed
	 * @param type numeric type of agent to be selected
	 * @param remove if true, remove any elements originally in the set before the
	 * set is returned (default is false)
	 * @param popSize size of the population from which the sample will be drawn
	 */
  void selectAgents(filterLocalFlag localOrNonLocalOnly, int count, std::vector<T*>& selectedAgents, int type, bool remove = false, int popSize = -1);
	
	/**
	 * Gets a set of pointers to all local or non-local agents in this
	 * context matching a user-defined filter.
	 *
	 * If the 'remove' parameter is set to true, any elements in the
	 * original set will be removed before the method returns.
	 *
	 * The popSize parameter is used when the method is repeatedly called
	 * on a population whose size is known. Calls to this method typically begin
	 * by determining the size of the (valid) population to be sampled; if this is known,
	 * it can be provided here, improving performance.
	 *
	 * @param localOrNonLocalOnly flag that indicates that the agents selected
	 * will be drawn only from agents either local or non-local to this process
	 * @param [out] selectedAgents a set into which the pointers to the agents will be placed
	 * @param filter user-defined filter specifying any criteria agents to be selected
	 * must meet
	 * @param remove if true, remove any elements originally in the set before the
	 * set is returned (default is false)
	 * @param popSize size of the population from which the sample will be drawn
	 *
	 * @tparam filterStruct the type of the filter to be applied to the agents
	 */
	template<typename filterStruct>
  void selectAgents(filterLocalFlag localOrNonLocalOnly, std::set<T*>& selectedAgents, filterStruct& filter, bool remove = false, int popSize = -1);

	/**
	 * Gets a randomly ordered vector of pointers to all local or non-local agents in this context
	 * matching a user-defined filter.
	 *
	 * If the 'remove' parameter is set to true, any elements in the
	 * original vector will be removed before the method returns.
	 *
	 * The popSize parameter is used when the method is repeatedly called
	 * on a population whose size is known. Calls to this method typically begin
	 * by determining the size of the (valid) population to be sampled; if this is known,
	 * it can be provided here, improving performance.
	 *
	 * @param localOrNonLocalOnly flag that indicates that the agents selected
	 * will be drawn only from agents either local or non-local to this process
	 * @param [out] selectedAgents a vector into which the pointers to the agents will be placed
	 * @param filter user-defined filter specifying any criteria agents to be selected
	 * must meet
	 * @param remove if true, remove any elements originally in the set before the
	 * set is returned (default is false)
	 * @param popSize size of the population from which the sample will be drawn
	 *
	 * @tparam filterStruct the type of the filter to be applied to the agents
	 */
	template<typename filterStruct>
  void selectAgents(filterLocalFlag localOrNonLocalOnly, std::vector<T*>& selectedAgents, filterStruct& filter, bool remove = false, int popSize = -1);

	/**
	 * Gets a set of pointers to a specified number of randomly selected local or non-local agents
	 * matching a user-defined filter.
	 *
	 * If the set passed contains any elements when this method is called,
	 * the agents pointed to by those elements will be omitted from the selection.
	 *
	 * If the 'remove' parameter is set to true, any elements in the
	 * original set will be removed before the method returns.
	 *
	 * The popSize parameter is used when the method is repeatedly called
	 * on a population whose size is known. Calls to this method typically begin
	 * by determining the size of the (valid) population to be sampled; if this is known,
	 * it can be provided here, improving performance.
	 *
	 * @param localOrNonLocalOnly flag that indicates that the agents selected
	 * will be drawn only from agents either local or non-local to this process
	 * @param count the number of agents to be selected. If this exceeds the number
	 * that can possibly be selected, all possible agents will be selected
	 * @param [out] selectedAgents a set into which the pointers to the agents will be placed
	 * @param filter user-defined filter specifying any criteria agents to be selected
	 * must meet
	 * @param remove if true, remove any elements originally in the set before the
	 * set is returned (default is false)
	 * @param popSize size of the population from which the sample will be drawn
	 *
	 * @tparam filterStruct the type of the filter to be applied to the agents
	 */
	template<typename filterStruct>
  void selectAgents(filterLocalFlag localOrNonLocalOnly, int count, std::set<T*>& selectedAgents, filterStruct& filter, bool remove = false, int popSize = -1);

	/**
	 * Gets a randomly ordered vector of pointers to a specified number of randomly selected local or non-local agents
	 * matching a user-defined filter.
	 *
	 * If the vector passed contains any elements when this method is called,
	 * the agents pointed to by those elements will be omitted from the selection.
	 *
	 * If the 'remove' parameter is set to true, any elements in the
	 * original vector will be removed before the method returns.
	 *
	 * The popSize parameter is used when the method is repeatedly called
	 * on a population whose size is known. Calls to this method typically begin
	 * by determining the size of the (valid) population to be sampled; if this is known,
	 * it can be provided here, improving performance.
	 *
	 * @param localOrNonLocalOnly flag that indicates that the agents selected
	 * will be drawn only from agents either local or non-local to this process
	 * @param count the number of agents to be selected. If this exceeds the number
	 * that can possibly be selected, all possible agents will be selected
	 * @param [out] selectedAgents a vector into which the pointers to the agents will be placed
	 * @param filter user-defined filter specifying any criteria agents to be selected
	 * must meet
	 * @param remove if true, remove any elements originally in the set before the
	 * set is returned (default is false)
	 * @param popSize size of the population from which the sample will be drawn
	 *
	 * @tparam filterStruct the type of the filter to be applied to the agents
	 */
	template<typename filterStruct>
  void selectAgents(filterLocalFlag localOrNonLocalOnly, int count, std::vector<T*>& selectedAgents, filterStruct& filter, bool remove = false, int popSize = -1);
	
	/**
	 * Gets a set of pointers to all local or non-local agents in this context
	 * of a specified type (per their AgentId values) and matching a user-defined filter.
	 *
	 * If the 'remove' parameter is set to true, any elements in the
	 * original set will be removed before the method returns.
	 *
	 * The popSize parameter is used when the method is repeatedly called
	 * on a population whose size is known. Calls to this method typically begin
	 * by determining the size of the (valid) population to be sampled; if this is known,
	 * it can be provided here, improving performance.
	 *
	 * @param localOrNonLocalOnly flag that indicates that the agents selected
	 * will be drawn only from agents either local or non-local to this process
	 * @param [out] selectedAgents a set into which the pointers to the agents will be placed
	 * @param type numeric type of agent to be selected
	 * @param filter user-defined filter specifying any criteria agents to be selected
	 * must meet
	 * @param remove if true, remove any elements originally in the set before the
	 * set is returned (default is false)
	 * @param popSize size of the population from which the sample will be drawn
	 *
	 * @tparam filterStruct the type of the filter to be applied to the agents
	 */
  template<typename filterStruct>
  void selectAgents(filterLocalFlag localOrNonLocalOnly, std::set<T*>& selectedAgents, int type, filterStruct& filter, bool remove = false, int popSize = -1);

	/**
	 * Gets a randomly ordered vector of pointers to all local or non-local agents in this context
	 * of a specified type (per their AgentId values) and matching a user-defined filter.
	 *
	 * If the 'remove' parameter is set to true, any elements in the
	 * original vector will be removed before the method returns.
	 *
	 * The popSize parameter is used when the method is repeatedly called
	 * on a population whose size is known. Calls to this method typically begin
	 * by determining the size of the (valid) population to be sampled; if this is known,
	 * it can be provided here, improving performance.
	 *
	 * @param localOrNonLocalOnly flag that indicates that the agents selected
	 * will be drawn only from agents either local or non-local to this process
	 * @param [out] selectedAgents a vector into which the pointers to the agents will be placed
	 * @param type numeric type of agent to be selected
	 * @param filter user-defined filter specifying any criteria agents to be selected
	 * must meet
	 * @param remove if true, remove any elements originally in the set before the
	 * set is returned (default is false)
	 * @param popSize size of the population from which the sample will be drawn
	 *
	 * @tparam filterStruct the type of the filter to be applied to the agents
	 */
	template<typename filterStruct>
  void selectAgents(filterLocalFlag localOrNonLocalOnly, std::vector<T*>& selectedAgents, int type, filterStruct& filter, bool remove = false, int popSize = -1);

	/**
	 * Gets a set of pointers to a specified number of randomly selected local or non-local agents
	 * of a specified type (per their AgentId values) and matching a user-defined filter.
	 *
	 * If the set passed contains any elements when this method is called,
	 * the agents pointed to by those elements will be omitted from the selection.
	 *
	 * If the 'remove' parameter is set to true, any elements in the
	 * original set will be removed before the method returns.
	 *
	 * The popSize parameter is used when the method is repeatedly called
	 * on a population whose size is known. Calls to this method typically begin
	 * by determining the size of the (valid) population to be sampled; if this is known,
	 * it can be provided here, improving performance.
	 *
	 * @param localOrNonLocalOnly flag that indicates that the agents selected
	 * will be drawn only from agents either local or non-local to this process
	 * @param count the number of agents to be selected. If this exceeds the number
	 * that can possibly be selected, all possible agents will be selected
	 * @param [out] selectedAgents a set into which the pointers to the agents will be placed
	 * @param type numeric type of agent to be selected
	 * @param filter user-defined filter specifying any criteria agents to be selected
	 * must meet
	 * @param remove if true, remove any elements originally in the set before the
	 * set is returned (default is false)
	 * @param popSize size of the population from which the sample will be drawn
	 *
	 * @tparam filterStruct the type of the filter to be applied to the agents
	 */
	template<typename filterStruct>
  void selectAgents(filterLocalFlag localOrNonLocalOnly, int count, std::set<T*>& selectedAgents, int type, filterStruct& filter, bool remove = false, int popSize = -1);
	
	/**
	 * Gets a randomly ordered vector of pointers to a specified number of randomly selected
	 * local or non-local agents of a specified type (per their AgentId values) and
	 * matching a user-defined filter.
	 *
	 * If the vector passed contains any elements when this method is called,
	 * the agents pointed to by those elements will be omitted from the selection.
	 *
	 * If the 'remove' parameter is set to true, any elements in the
	 * original vector will be removed before the method returns.
	 *
	 * The popSize parameter is used when the method is repeatedly called
	 * on a population whose size is known. Calls to this method typically begin
	 * by determining the size of the (valid) population to be sampled; if this is known,
	 * it can be provided here, improving performance.
	 *
	 * @param localOrNonLocalOnly flag that indicates that the agents selected
	 * will be drawn only from agents either local or non-local to this process
	 * @param count the number of agents to be selected. If this exceeds the number
	 * that can possibly be selected, all possible agents will be selected
	 * @param [out] selectedAgents a vector into which the pointers to the agents will be placed
	 * @param type numeric type of agent to be selected
	 * @param filter user-defined filter specifying any criteria agents to be selected
	 * must meet
	 * @param remove if true, remove any elements originally in the set before the
	 * set is returned (default is false)
	 * @param popSize size of the population from which the sample will be drawn
	 *
	 * @tparam filterStruct the type of the filter to be applied to the agents
	 */
  template<typename filterStruct>
  void selectAgents(filterLocalFlag localOrNonLocalOnly, int count, std::vector<T*>& selectedAgents, int type, filterStruct& filter, bool remove = false, int popSize = -1);


  // Beta

  /**
   * Returns true if any of the projections in this context will try to 'keep' non-local agents
   * during a synchronize projection operation. (Generally graphs keep local agents that are
   * part of master copies of links, but spaces do not keep any local agents.)
   */
  bool keepsAgentsOnSyncProj();

  bool sendsSecondaryDataOnStatusExchange();

  void getProjInfoExchangePartners(std::set<int>& sends, std::set<int>& recvs);

  void getAgentStatusInfoExchangePartners(std::set<int>& sends, std::set<int>& recvs);

  /**
   * Given a set of agents to test, returns the set of those agents that must be kept in order
   * to keep required projection information.
   */
  void getRequiredAgents(std::set<AgentId>& agentsToTest, std::set<AgentId>& agentsToKeep, RADIUS radius = Projection<T>::PRIMARY);

  /**
   * Given an initial set of agents that must be kept a priori, add any agents that must be kept due to
   * projection requirements, and return the set of all non-local agents that can be dropped.
   */
  void getNonlocalAgentsToDrop(std::set<AgentId>& agentsToKeep, std::set<AgentId>& agentsToDrop, RADIUS radius = Projection<T>::PRIMARY);

  void getAgentsToPushToOtherProcesses(std::map<int, std::set<AgentId> >& agentsToPush);

  virtual void addProjection(Projection<T>* projection);

};

template<typename T>
SharedContext<T>::SharedContext(boost::mpi::communicator* comm) : Context<T> (), _rank(comm->rank()), localPredicate(comm->rank()),
  LOCAL_FILTER(true, comm->rank()),
  NON_LOCAL_FILTER(false, comm->rank()){
}

template<typename T>
SharedContext<T>::~SharedContext() { }

template<typename T>
void SharedContext<T>::removeAgent(T* agent) {
	removeAgent(agent->getId());
}

template<typename T>
void SharedContext<T>::removeAgent(const AgentId id) {
	if (id.currentRank() != _rank) {
		if (projRefMap.find(id) == projRefMap.end()) {
			Context<T>::removeAgent(id);
		}
	} else {
		Context<T>::removeAgent(id);
		rpRemoveAgent(id);
	}
}

template <typename T>
void SharedContext<T>::importedAgentRemoved(const AgentId& id) {
	projRefMap.erase(id);
	Context<T>::removeAgent(id);
}

template<typename T>
void SharedContext<T>::incrementProjRefCount(const AgentId& id) {
	if (id.currentRank() != _rank) {
		RefMap::iterator iter = projRefMap.find(id);
		if (iter == projRefMap.end()) {
			projRefMap[id] = 1;
		} else {
			projRefMap[id] = ++(iter->second);
		}
	}
}

template<typename T>
void SharedContext<T>::decrementProjRefCount(const AgentId& id) {
	if (id.currentRank() == _rank) return;

	RefMap::iterator iter = projRefMap.find(id);
	if (iter == projRefMap.end())	throw Repast_Error_31<AgentId>(id); // Id is not in the projection reference map

	int count = --(iter->second);
	if   (count == 0)	projRefMap.erase(iter);
	else              projRefMap[id] = count;

}

// Local Agents Only

template<typename T>
boost::filter_iterator<IsLocalAgent<T> , typename Context<T>::const_iterator> SharedContext<T>::localBegin() const {
	return const_local_iterator(localPredicate, Context<T>::begin(), Context<T>::end());
}

template<typename T>
boost::filter_iterator<IsLocalAgent<T> , typename Context<T>::const_iterator> SharedContext<T>::localEnd() const {
	return const_local_iterator(localPredicate, Context<T>::end(), Context<T>::end());
}


// Iterator creation

template<typename T>
boost::filter_iterator<AgentStateFilter<T> , typename Context<T>::const_iterator> SharedContext<T>::begin(filterLocalFlag local){
	if(local) return const_state_aware_iterator(LOCAL_FILTER, Context<T>::begin(), Context<T>::end());
	else      return const_state_aware_iterator(NON_LOCAL_FILTER, Context<T>::begin(), Context<T>::end());
}

template<typename T>
boost::filter_iterator<AgentStateFilter<T> , typename Context<T>::const_iterator> SharedContext<T>::end(filterLocalFlag local){
	if(local) return const_state_aware_iterator(LOCAL_FILTER, Context<T>::end(), Context<T>::end());
	else      return const_state_aware_iterator(NON_LOCAL_FILTER, Context<T>::end(), Context<T>::end());
}

template<typename T>
boost::filter_iterator<AgentStateFilter<T> , typename Context<T>::const_bytype_iterator> SharedContext<T>::byTypeBegin(filterLocalFlag local, int type){
	if(local) return const_state_aware_bytype_iterator(LOCAL_FILTER, Context<T>::byTypeBegin(type), Context<T>::byTypeEnd(type));
	else      return const_state_aware_bytype_iterator(NON_LOCAL_FILTER, Context<T>::byTypeBegin(type), Context<T>::byTypeEnd(type));
}

template<typename T>
boost::filter_iterator<AgentStateFilter<T> , typename Context<T>::const_bytype_iterator> SharedContext<T>::byTypeEnd(filterLocalFlag local, int type){
	if(local) return const_state_aware_bytype_iterator(LOCAL_FILTER, Context<T>::byTypeEnd(type), Context<T>::byTypeEnd(type));
	else      return const_state_aware_bytype_iterator(NON_LOCAL_FILTER, Context<T>::byTypeEnd(type), Context<T>::byTypeEnd(type));
}

template<typename T>
template<typename filterStruct>
boost::filter_iterator<filterStruct, typename SharedContext<T>::const_state_aware_iterator> SharedContext<T>::filteredBegin(filterLocalFlag local, filterStruct& fStruct){
	return boost::filter_iterator<filterStruct , typename SharedContext<T>::const_state_aware_iterator> (fStruct, SharedContext<T>::begin(local), SharedContext<T>::end(local));
}

template<typename T>
template<typename filterStruct>
boost::filter_iterator<filterStruct, typename SharedContext<T>::const_state_aware_iterator> SharedContext<T>::filteredEnd(filterLocalFlag local, filterStruct& fStruct){
	return boost::filter_iterator<filterStruct , typename SharedContext<T>::const_state_aware_iterator> (fStruct, SharedContext<T>::end(local), SharedContext<T>::end(local));
}


template<typename T>
template<typename filterStruct>
boost::filter_iterator<filterStruct, typename SharedContext<T>::const_state_aware_bytype_iterator> SharedContext<T>::byTypeFilteredBegin(filterLocalFlag local, int type, filterStruct& fStruct){
	return boost::filter_iterator<filterStruct , typename SharedContext<T>::const_state_aware_bytype_iterator> (fStruct, SharedContext<T>::byTypeBegin(local, type), SharedContext<T>::byTypeEnd(local, type));
}

template<typename T>
template<typename filterStruct>
boost::filter_iterator<filterStruct, typename SharedContext<T>::const_state_aware_bytype_iterator> SharedContext<T>::byTypeFilteredEnd(filterLocalFlag local, int type, filterStruct& fStruct){
	return boost::filter_iterator<filterStruct , typename SharedContext<T>::const_state_aware_bytype_iterator> (fStruct, SharedContext<T>::byTypeEnd(local, type), SharedContext<T>::byTypeEnd(local, type));
}


// Agent Selection

template<typename T>
void SharedContext<T>::selectAgents(filterLocalFlag localOrNonLocalOnly, std::set<T*>& selectedAgents, bool remove, int popSize){
	if(popSize <= -1) selectNElementsAtRandom(begin(localOrNonLocalOnly), end(localOrNonLocalOnly), size(), selectedAgents, remove);
	else              selectNElementsAtRandom(begin(localOrNonLocalOnly), popSize, size(), selectedAgents, remove);
}

template<typename T>
void SharedContext<T>::selectAgents(filterLocalFlag localOrNonLocalOnly, std::vector<T*>& selectedAgents, bool remove, int popSize){
	if(popSize <= -1) selectNElementsInRandomOrder(begin(localOrNonLocalOnly), end(localOrNonLocalOnly), size(), selectedAgents, remove);
	else              selectNElementsInRandomOrder(begin(localOrNonLocalOnly), popSize, size(), selectedAgents, remove);
}

template<typename T>
void SharedContext<T>::selectAgents(filterLocalFlag localOrNonLocalOnly, int count, std::set<T*>& selectedAgents, bool remove, int popSize){
	if(popSize <= -1) selectNElementsAtRandom(begin(localOrNonLocalOnly), end(localOrNonLocalOnly), count, selectedAgents, remove);
	else              selectNElementsAtRandom(begin(localOrNonLocalOnly), popSize, count, selectedAgents, remove);
}

template<typename T>
void SharedContext<T>::selectAgents(filterLocalFlag localOrNonLocalOnly, int count, std::vector<T*>& selectedAgents, bool remove, int popSize){
	if(popSize <= -1) selectNElementsInRandomOrder(begin(localOrNonLocalOnly), end(localOrNonLocalOnly), count, selectedAgents, remove);
	else              selectNElementsInRandomOrder(begin(localOrNonLocalOnly), popSize, count, selectedAgents, remove);
}

template<typename T>
void SharedContext<T>::selectAgents(filterLocalFlag localOrNonLocalOnly, std::set<T*>& selectedAgents, int type, bool remove, int popSize){
	if(popSize <= -1) selectNElementsAtRandom(byTypeBegin(localOrNonLocalOnly, type), byTypeEnd(localOrNonLocalOnly, type), size(), selectedAgents, remove);
	else              selectNElementsAtRandom(byTypeBegin(localOrNonLocalOnly, type), popSize, size(), selectedAgents, remove);
}

template<typename T>
void SharedContext<T>::selectAgents(filterLocalFlag localOrNonLocalOnly, std::vector<T*>& selectedAgents, int type, bool remove, int popSize){
	if(popSize <= -1) selectNElementsInRandomOrder(byTypeBegin(localOrNonLocalOnly, type), byTypeEnd(localOrNonLocalOnly, type), size(), selectedAgents, remove);
	else              selectNElementsInRandomOrder(byTypeBegin(localOrNonLocalOnly, type), popSize, size(), selectedAgents, remove);
}

template<typename T>
void SharedContext<T>::selectAgents(filterLocalFlag localOrNonLocalOnly, int count, std::set<T*>& selectedAgents, int type, bool remove, int popSize){
	if(popSize <= -1) selectNElementsAtRandom(byTypeBegin(localOrNonLocalOnly, type), byTypeEnd(localOrNonLocalOnly, type), count, selectedAgents, remove);
	else              selectNElementsAtRandom(byTypeBegin(localOrNonLocalOnly, type), popSize, count, selectedAgents, remove);
}

template<typename T>
void SharedContext<T>::selectAgents(filterLocalFlag localOrNonLocalOnly, int count, std::vector<T*>& selectedAgents, int type, bool remove, int popSize){
	if(popSize <= -1) selectNElementsInRandomOrder(byTypeBegin(localOrNonLocalOnly, type), byTypeEnd(localOrNonLocalOnly, type), count, selectedAgents, remove);
	else              selectNElementsInRandomOrder(byTypeBegin(localOrNonLocalOnly, type), popSize, count, selectedAgents, remove);
}

template<typename T>
template<typename filterStruct>
void SharedContext<T>::selectAgents(filterLocalFlag localOrNonLocalOnly, std::set<T*>& selectedAgents, filterStruct& filter, bool remove, int popSize){
	if(popSize <= -1) selectNElementsAtRandom(filteredBegin(localOrNonLocalOnly, filter), filteredEnd(localOrNonLocalOnly, filter), size(), selectedAgents, remove);
	else              selectNElementsAtRandom(filteredBegin(localOrNonLocalOnly, filter), popSize, size(), selectedAgents, remove);
}

template<typename T>
template<typename filterStruct>
void SharedContext<T>::selectAgents(filterLocalFlag localOrNonLocalOnly, std::vector<T*>& selectedAgents, filterStruct& filter, bool remove, int popSize){
	if(popSize <= -1) selectNElementsInRandomOrder(filteredBegin(localOrNonLocalOnly, filter), filteredEnd(localOrNonLocalOnly, filter), size(), selectedAgents, remove);
	else              selectNElementsInRandomOrder(filteredBegin(localOrNonLocalOnly, filter), popSize, size(), selectedAgents, remove);
}

template<typename T>
template<typename filterStruct>
void SharedContext<T>::selectAgents(filterLocalFlag localOrNonLocalOnly, int count, std::set<T*>& selectedAgents, filterStruct& filter, bool remove, int popSize){
	if(popSize <= -1) selectNElementsAtRandom(filteredBegin(localOrNonLocalOnly, filter), filteredEnd(localOrNonLocalOnly, filter), count, selectedAgents, remove);
	else              selectNElementsAtRandom(filteredBegin(localOrNonLocalOnly, filter), popSize, count, selectedAgents, remove);
}

template<typename T>
template<typename filterStruct>
void SharedContext<T>::selectAgents(filterLocalFlag localOrNonLocalOnly, int count, std::vector<T*>& selectedAgents, filterStruct& filter, bool remove, int popSize){
	if(popSize <= -1) selectNElementsInRandomOrder(filteredBegin(localOrNonLocalOnly, filter), filteredEnd(localOrNonLocalOnly, filter), count, selectedAgents, remove);
	else              selectNElementsInRandomOrder(filteredBegin(localOrNonLocalOnly, filter), popSize, count, selectedAgents, remove);
}

template<typename T>
template<typename filterStruct>
void SharedContext<T>::selectAgents(filterLocalFlag localOrNonLocalOnly, std::set<T*>& selectedAgents, int type, filterStruct& filter, bool remove, int popSize){
	if(popSize <= -1) selectNElementsAtRandom(byTypeFilteredBegin(localOrNonLocalOnly, type, filter), byTypeFilteredEnd(localOrNonLocalOnly, type, filter), size(), selectedAgents, remove);
	else              selectNElementsAtRandom(byTypeFilteredBegin(localOrNonLocalOnly, type, filter), popSize, size(), selectedAgents, remove);
}

template<typename T>
template<typename filterStruct>
void SharedContext<T>::selectAgents(filterLocalFlag localOrNonLocalOnly, std::vector<T*>& selectedAgents, int type, filterStruct& filter, bool remove, int popSize){
	if(popSize <= -1) selectNElementsInRandomOrder(byTypeFilteredBegin(localOrNonLocalOnly, type, filter), byTypeFilteredEnd(localOrNonLocalOnly, type, filter), size(), selectedAgents, remove);
	else              selectNElementsInRandomOrder(byTypeFilteredBegin(localOrNonLocalOnly, type, filter), popSize, size(), selectedAgents, remove);
}

template<typename T>
template<typename filterStruct>
void SharedContext<T>::selectAgents(filterLocalFlag localOrNonLocalOnly, int count, std::set<T*>& selectedAgents, int type, filterStruct& filter, bool remove, int popSize){
	if(popSize <= -1) selectNElementsAtRandom(byTypeFilteredBegin(localOrNonLocalOnly, type, filter), byTypeFilteredEnd(localOrNonLocalOnly, type, filter), count, selectedAgents, remove);
	else              selectNElementsAtRandom(byTypeFilteredBegin(localOrNonLocalOnly, type, filter), popSize, count, selectedAgents, remove);
}

template<typename T>
template<typename filterStruct>
void SharedContext<T>::selectAgents(filterLocalFlag localOrNonLocalOnly, int count, std::vector<T*>& selectedAgents, int type, filterStruct& filter, bool remove, int popSize){
	if(popSize <= -1) selectNElementsInRandomOrder(byTypeFilteredBegin(localOrNonLocalOnly, type, filter), byTypeFilteredEnd(localOrNonLocalOnly, type, filter), count, selectedAgents, remove);
	else              selectNElementsInRandomOrder(byTypeFilteredBegin(localOrNonLocalOnly, type, filter), popSize, count, selectedAgents, remove);
}




// Beta

template<typename T>
bool SharedContext<T>::keepsAgentsOnSyncProj(){
  typename std::vector<Projection<T> *>::iterator iter    = Context<T>::projections.begin();
  typename std::vector<Projection<T> *>::iterator iterEnd = Context<T>::projections.end();
  while((iter != iterEnd)){
    if((*iter)->keepsAgentsOnSyncProj()) return true;
    iter++;
  }
  return false;
}

template<typename T>
bool SharedContext<T>::sendsSecondaryDataOnStatusExchange(){
  for(typename std::vector<Projection<T> *>::iterator iter = Context<T>::projections.begin(), iterEnd = Context<T>::projections.end(); iter != iterEnd; iter++){
    if((*iter)->sendsSecondaryAgentsOnStatusExchange()) return true;
  }
  return false;
}

template<typename T>
void SharedContext<T>::getProjInfoExchangePartners(std::set<int>& sends, std::set<int>& recvs){
  for(typename std::vector<Projection<T> *>::iterator iter = Context<T>::projections.begin(), iterEnd = Context<T>::projections.end(); iter != iterEnd; iter++){
    (*iter)->getInfoExchangePartners(sends, recvs);
  }
}


template<typename T>
void SharedContext<T>::getAgentStatusInfoExchangePartners(std::set<int>& sends, std::set<int>& recvs){
  for(typename std::vector<Projection<T> *>::iterator iter = Context<T>::projections.begin(), iterEnd = Context<T>::projections.end(); iter != iterEnd; iter++){
    (*iter)->getAgentStatusExchangePartners(sends, recvs);
  }
}


template<typename T>
void SharedContext<T>::getRequiredAgents(std::set<AgentId>& agentsToTest, std::set<AgentId>& agentsToKeep, RADIUS radius){
  typename std::vector<Projection<T> *>::iterator iter    = Context<T>::projections.begin();
  typename std::vector<Projection<T> *>::iterator iterEnd = Context<T>::projections.end();
  while((iter != iterEnd) && (agentsToTest.size() > 0)){
    (*iter)->getRequiredAgents(agentsToTest, agentsToKeep, radius);
    iter++;
  }
}

template<typename T>
void SharedContext<T>::getNonlocalAgentsToDrop(std::set<AgentId>& agentsToKeep, std::set<AgentId>& agentsToDrop, RADIUS radius){
  if(agentsToKeep.size() > 0){
    const_state_aware_iterator iter = begin(NON_LOCAL), iterEnd = end(NON_LOCAL);
    std::set<AgentId>::iterator notFound = agentsToKeep.end();
    while(iter != iterEnd){
      AgentId id = (*iter)->getId();
      if(agentsToKeep.find(id) == notFound) agentsToDrop.insert((*iter)->getId());
      iter++;
    }
  }
  else{
    const_state_aware_iterator iter = begin(NON_LOCAL), iterEnd = end(NON_LOCAL);
    while(iter != iterEnd){
      agentsToDrop.insert((*iter)->getId());
      iter++;
    }
  }
  getRequiredAgents(agentsToDrop, agentsToKeep, radius);
}

template<typename T>
void SharedContext<T>::getAgentsToPushToOtherProcesses(std::map<int, std::set<AgentId> >& agentsToPush){
  std::vector<AgentId> tmp;
  std::set<AgentId> agentsToTest;
  for(const_state_aware_iterator iter = begin(LOCAL), iterEnd = end(LOCAL); iter != iterEnd; ++iter){
    tmp.push_back((*iter)->getId());
  }
  agentsToTest.insert(tmp.begin(), tmp.end());
  for(typename std::vector<std::string>::iterator iter = getAgentsToPushProjOrder.begin(), iterEnd = getAgentsToPushProjOrder.end(); iter != iterEnd; iter++){
     Context<T>::getProjection(*iter)->getAgentsToPush(agentsToTest, agentsToPush);
  }
}

template<typename T>
void SharedContext<T>::addProjection(Projection<T>* projection){
  int sizeBefore = Context<T>::projections.size();
  Context<T>::addProjection(projection);
  if(sizeBefore != Context<T>::projections.size()) getAgentsToPushProjOrder.push_back(projection->name());
}




}
#endif /* SHAREDCONEXT_HPP_ */
