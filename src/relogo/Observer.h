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
 *  Observer.h
 *
 *  Created on: Jul 15, 2010
 *      Author: nick
 */

#ifndef OBSERVER_H_
#define OBSERVER_H_

#include <typeinfo>

#include "repast_hpc/SharedContext.h"
#include "repast_hpc/Properties.h"
#include "repast_hpc/Random.h"
#include "repast_hpc/DataSet.h"
#include "repast_hpc/RepastProcess.h"

#include "AgentSet.h"
#include "RelogoAgent.h"
#include "RelogoDiscreteSpaceAdder.h"
#include "RelogoContinuousSpaceAdder.h"
#include "RelogoLink.h"
#include "WorldDefinition.h"
#include "creators.h"
#include "relogo.h"
#include "agent_set_functions.h"

/**
 * \mainpage Repast HPC- ReLogo Logo-Like Semantics for Repast HPC
 *
 * By Argonne National Laboratory, 2009-2013
 *
 * \section intro_sec What is ReLogo
 *
 * ReLogo is a collection of classes and methods that allow Repast HPC
 * simulations (agent-based simulations for high-performance computing
 * environments) to be built using simple and easily apprehensible
 * semantics.
 */

namespace repast {

namespace relogo {

class Patch;
class WorldCreator;
class DataSet;

/**
 * Compare two elements of type std::type_info using 'before'
 */
struct TypeInfoCmp {
	bool operator()(const std::type_info* one, const std::type_info* two) const {
		return one->before(*two) != 0;
	}
};

class Turtle;

/**
 * Unary function used in the transform_iterator that allows context iterators
 * to return the agent maps values.
 */
template<typename TargetType>
struct Caster: public std::unary_function<boost::shared_ptr<RelogoAgent>, TargetType*> {

	TargetType* operator()(boost::shared_ptr<RelogoAgent> ptr) const {
		return static_cast<TargetType*> (ptr.get());
	}
};


/**
 * Implementation of a logo Observer.
 */
class Observer {

public:
	virtual ~Observer();

	/**
	 * Adds a dataset to this Observer. This observer will schedule the dataset
	 * for recording and writing, and properly destroy the dataset.
	 *
	 * @param dataSet the data set to add
	 */
	void addDataSet(repast::DataSet* dataSet);

	/**
	 * Non API method for closing all the datasets at the end of a sim runs.
	 */
	void dataSetClose();

	/**
	 * Called every tick of the simulation. Implementations of this method will
	 * implement that actual simulation behavior.
	 */
	virtual void go() = 0;

	/**
	 * Classes that extend this should include model initialization here.
	 *
	 * @param props Properties collection that can be used to drive initialization
	 */
  virtual void setup(Properties& props){ }

  /**
   * Performs internal Relogo initialization
   *
   * @param props Properties collection that can be used to drive initialization
   */
  void _setup(Properties& props);


	/**
	 * Gets the minimum x coordinate of the patches managed by this Observer.
	 *
	 * @return the minimum x coordinate of the patches managed by this Observer.
	 */
	int minPxcor() const;

	/**
	 * Gets the minimum y coordinate of the patches managed by this Observer.
	 *
	 * @return the minimum y coordinate of the patches managed by this Observer.
	 */
	int minPycor() const;

	/**
	 * Gets the maximum x coordinate of the patches managed by this Observer.
	 *
	 * @return the maximum x coordinate of the patches managed by this Observer.
	 */
	int maxPxcor() const;

	/**
	 * Gets the maximum y coordinate of the patches managed by this Observer.
	 *
	 * @return the maximum y coordinate of the patches managed by this Observer.
	 */
	int maxPycor() const;

	/**
	 * Gets a random x coodinate of the patches managed by this Observer.
	 *
	 * @return a random x coordinate of the patches managed by this Observer.
	 */
	int randomPxcor();

	/**
	 * Gets a random y coodinate of the patches managed by this Observer.
	 *
	 * @return a random y coordinate of the patches managed by this Observer.
	 */
	int randomPycor();

	/**
	 * Gets a random x coodinate of the turtles managed by this Observer.
	 *
	 * @return a random x coordinate of the turtles managed by this Observer.
	 */
	double randomXcor();

	/**
	 * Gets a random y coodinate of the turtles managed by this Observer.
	 *
	 * @return a random y coordinate of the turtles managed by this Observer.
	 */
	double randomYcor();

	/*
	 * Calculates the grid point from the space point. Under some circumstances
	 * the space point has to be changed in order to stay in sync w/r to process location
	 * with the grid point. If that has to be done, then return true.
	 *
	 * NON API method
	 */
	bool spacePtToGridPt(std::vector<double>& spacePt, std::vector<int>& gridPt);

	/**
	 * Hatchs an agent of the specified type. The new agent
	 * will have the location and heading of the specified "parent".
	 *
	 * @param the "parent" of the hatched agent
	 * @tparam AgentType the type of turtle to hatch
	 */
	template<typename AgentType>
	AgentType* hatch(RelogoAgent* parent);

	/**
	 * Hatchs an agent of the specified type. The new agent
	 * will have the location and heading of the specified "parent" and
	 * will be created using the FactoryFunctor.
	 *
	 * @param the "parent" of the hatched agent
	 * @param agentCreator a FactoryFunctor used to create the agent
	 *
	 * @tparam FactoryFunctor a functor or function with the following
	 * signature AgentType* (AgentId id, Observer* obs).
	 *
	 * @tparam AgentType the type of turtle to hatch. This must either be
	 * Turtle or extend Turtle.
	 */
	template<typename AgentType, typename FactoryFunctor>
	AgentType* hatch(RelogoAgent* parent, FactoryFunctor agentCreator);

	/**
	 * Gets the agent with the specified id.
	 *
	 * @param id the id of the agent to get
	 * @tparam AgentType the type of the agent to get
	 *
	 * @return the agent with the specified id, or 0 if
	 * the agent is not found.
	 */
	template<typename AgentType>
	AgentType* who(const AgentId& id);

	/**
	 * Create count number of agents of the specified type.
	 *
	 * @param count the number of agents to create
	 * @tparam AgentType the type of agents to create. This
	 * must be a Turtle or a class that extends Turtle. It must
	 * have a constructor that takes an AgentId and a pointer
	 * to this Observer.
	 *
	 * @return the integer type id for agents of this type.
	 */
	template<typename AgentType>
	int create(size_t count);

	/**
	 * Create count number of agents of the specified type,
	 * using the specified FactoryFunctor.
	 *
	 * @param count the number of agents to create
	 * @param agentCreator a FactoryFunctor used to create the agents
	 *
	 * @tparam AgentType the type of agent to create. This must either be a Turtle
	 * or extend Turtle.
	 * @tparam FactoryFunctor a functor or function with the following
	 * signature AgentType* (AgentId id, Observer* obs).
	 *
	 * @return an the integer id for agents of this type
	 */
	template<typename AgentType, typename FactoryFunctor>
	int create(size_t count, FactoryFunctor agentCreator);

	/**
	 * Removes the specified turtle from the world.
	 */
	void removeAgent(const AgentId& id);

	/**
	 * Gets all of the agents of the templated type and returns
	 * them in an AgentSet.
	 *
	 * @tparam AgentType the type of turtle agents to get
	 * @return an agent set containing the agents
	 */
	template<typename AgentType>
	AgentSet<AgentType> get();

	/**
	 * Gets all the turtles in this world and return them in the
	 * AgentSet.
	 */
	AgentSet<Turtle> turtles();

	/**
	 * Gets all the turtles in this world and put them
	 * into the specified AgentSet.
	 *
	 * @param turtles the AgentSet to put the turtles in
	 */
	void get(AgentSet<Turtle>& turtles);

	/**
	 * Gets all of the agents of the templated type and puts
	 * them into the agentSet.
	 *
	 * @param agentSet the AgentSet to put the found agents in
	 *
	 * @tparam AgentType the type of turtle agents to get
	 * @return an agent set containing the agents
	 */
	template<typename AgentType>
	void get(AgentSet<AgentType>& agentSet);

	/**
	 * Gets the turtle with the specified id or 0 if no such
	 * turtle is found.
	 *
	 * @tparam AgentType the type of the turtle to find
	 * @return the turtle with the specified id, or 0.
	 */
	template<typename AgentType>
	AgentType* get(const AgentId& id);

	/**
	 * Gets all of the agents of the templated type at
	 * the specified patch location
	 *
	 * @tparam AgentType the type of the agents
	 * @return an agent list containing the agents at
	 * the specified location.
	 */
	template<typename AgentType>
	AgentSet<AgentType> turtlesAt(int x, int y);

	/**
	 * Gets all of the agents of the templated type at
	 * the specified patch location and puts them in the
	 * specified set.
	 *
	 * @param x the x coordinate of the patch
	 * @param y the y coordinate of the patch
	 * @param set the AgentSet to add the found agents to
	 * @tparam AgentType the agent type
	 */
	template<typename AgentType>
	void turtlesAt(int x, int y, AgentSet<AgentType>& set);

	/**
	 * Gets the process rank of this Observer.
	 *
	 * @return the process rank of this Observer.
	 */
	int rank() const {
		return _rank;
	}

	/**
	 * Gets the grid managed by this Observer.
	 *
	 * @return the grid managed by this Observer.
	 */
	const RelogoGridType* grid();

	/**
	 * Gets the space managed by this Observer.
	 *
	 * @return the space managed by this Observer.
	 */
	const RelogoSpaceType* space();

	/**
	 * Creates a link between the source and target agents in the named network.
	 *
	 * @param source the source agent
	 * @param target the target agent
	 * @param networkName the name of the network to create the link in
	 */
	void createLink(RelogoAgent* source, RelogoAgent* target, const std::string& networkName);

	/**
	 * Creates a link between the source and target agents in the named network,
	 * using the specified LinkCreator.
	 *
	 * @param source the source agent
	 * @param target the target agent
	 * @param networkName the name of the network to create the link in
	 *
	 * @tparam LinkCreator an function or functor with the following signature
	 * RelogoLink* (Turtle* source, Turtle* target)
	 */
	template<typename LinkCreator>
	void createLink(RelogoAgent* source, RelogoAgent* target, const std::string& networkName, LinkCreator& creator);

	/**
	 * Gets the link, if any, between the source and target agents in the named network.
	 *
	 * @param source the source of the link
	 * @param target the target of the link
	 * @param networkName the name of the network to find link in
	 */
	boost::shared_ptr<RelogoLink> link(RelogoAgent* source, RelogoAgent* target, const std::string& networkName);

	/**
	 * Gets the network predecessors of the specified agent in the specified network and puts
	 * the result into out.
	 *
	 * @param agent the agent to get the predecessors of
	 * @param networkName the name of the network
	 * @param out an AgentSet when the predecessors will be put
	 *
	 * @tparam AgentType the type of the predecessors
	 */
	template<typename AgentType>
	void predecessors(RelogoAgent* agent, const std::string& networkName, AgentSet<AgentType>& out);

	/**
	 * Gets the network successors of the specified agent in the specified network and puts
	 * the result into out.
	 *
	 * @param agent the agent to get the successors of
	 * @param networkName the name of the network
	 * @param out an AgentSet when the successors will be put
	 *
	 * @tparam AgentType the type of the successors
	 */
	template<typename AgentType>
	void successors(RelogoAgent* agent, const std::string& networkName, AgentSet<AgentType>& out);

	/**
	 * Gets the patch at the specified coordinates
	 *
	 * @param x the x coordinate
	 * @param y the y coordinate
	 *
	 * @tparam the patch type
	 *
	 * @return a pointer to the patch at x,y
	 *
	 */
	template<typename PatchType>
	PatchType* patchAt(int x, int y);

	/**
	 * Gets the patch at the specified coordinates
	 *
	 * @param x the x coordinate
	 * @param y the y coordinate
	 *
	 * @return a pointer to the patch at x,y
	 *
	 */
	Patch* patchAt(int x, int y);

	/**
	 * Gets the patch at the delta from the specified location or
	 * 0 if the resulting location is outside the world.
	 *
	 * @param location
	 * @param dx the delta along the x dimension
	 * @param dy the delta along the y dimension
	 *
	 * @return the patch at the delta from the specified location or
	 * 0 if the resulting location is outside the world.
	 */
	Patch* patchAt(Point<double> location, double dx, double dy);

  /**
   * Gets the patch at the heading/distance offset from the specified location or
   * 0 if the resulting location is outside the world.
   *
   * @param location
   * @param heading the heading away from location
   * @param distance distance along heading
   *
   * @return the patch at the delta from the specified location or
   * 0 if the resulting location is outside the world.
   */
	Patch* patchAtOffset(Point<double> location, double heading, double distance);

	/**
	 * Gets an agent set of the all the patches.
	 *
	 * @tparam PatchType the patch type
	 *
	 * @return all the patches.
	 */
	template<typename PatchType>
	AgentSet<PatchType> patches();

	/**
	 * Gets all the patches and places them in the
	 * specified set.
	 *
	 * @param set the AgentSet to put the patches in
	 * @tparam PatchType the patch type
	 */
	template<typename PatchType>
	void patches(AgentSet<PatchType>& set);

	/**
	 * Gets all the turtles that are on any patches contained
	 * in the agentSet or on the patches where any turtles in
	 * the agentSet are standing. The result is placed in out.
	 *
	 * @param agentSet a set of turtles or patches
	 * @param out the AgentSet where the found turtles will put
	 * @tparam TurtleType the type of the turtles to return
	 */
	template<typename TurtleType>
	void turtlesOn(AgentSet<RelogoAgent>& agentSet, AgentSet<TurtleType>& out);

	/**
	 * Gets all the turtles that are on the patch if
	 * the agent is a patch, otherwise get all the agents
	 * on the patch where the agent is standing. The result is
	 * placed in out.
	 *
	 * @param agent the turtle or patch used to determine which turtles to get
	 * @param out the agent set where the found turtles will be put
	 *
	 * @tparam TurtleType the type of the turtles to return
	 */
	template<typename TurtleType>
	void turtlesOn(const RelogoAgent* agent, AgentSet<TurtleType>& out);

	/**
	 * Puts all the agents in the inSet that are of the specified type and within
	 * the specified radius from the specified center into the outSet.
	 *
	 * @param center the center of the circle within whose radius we filter on
	 * @param inSet the set of agents to filter
	 * @param outSet the set that will contain the results of the radius filter
	 *
	 * @tparam the type of agent to get
	 */
	template<typename AgentType>
	void
	inRadius(const Point<double>& center, AgentSet<RelogoAgent>& inSet, double radius, AgentSet<AgentType>& outSet);

	/**
	 * Synchronizes the status (moved or died) of all turtles across processes.
	 * If any turtle may have moved into the grid portion managed by another process
	 * or if any turtle has died then this must be called prior to those turtles doing
	 * anything.
	 *
	 * @param provider the class that provides agents given an AgentRequest
	 * @param creator creates Turtles given TurtleContent
	 *
	 * @tparam TurtleContent the serializable struct or class that describes
	 * a turtles state.
	 * @tparam Provider a class that provides TurtleContent from given an AgentRequest,
	 * implementing void provideContent(const repast::AgentRequest&, std::vector<TurtleContent>& out)
	 * @tparam AgentCreator a class that can create agents from TurtleContent, implementing
	 * RelogoAgent* createAgent(TurtleContent&).
	 *
	 */
  template<typename TurtleContent, typename Provider, typename Updater, typename AgentCreator>
  void synchronizeTurtleStatus(Provider& provider, Updater& updater, AgentCreator& creator, RepastProcess::EXCHANGE_PATTERN exchangePattern = RepastProcess::POLL);

	/**
	 * Synchronizes the state of any Turtles that are shared across processes.
	 * If no turtles are shared across processes, then this does not need to
	 * be called.
	 *
	 * @param provider provides TurtleContent given an AgentRequest
	 * @param updater updates an existing agent given TurtleContent
	 *
	 * @tparam TurtleContent the serializable struct or class that describes the
	 * state of turtles and patches
	 * @tparam Provider given an AgentRequest, a Provider provides the TurtleContent
	 * for the requested Turtles, implementing void provideContent(const AgentRequest&,
	 * std::vector<TurtleContent>&)
	 * @tparam Updater given TurtleContent, an Updater updates an existing agent with
	 * the TurtleContent, implementing void updateAgent(const TurtleContent&).
	 */
	template<typename TurtleContent, typename Provider, typename Updater>
	void synchronizeTurtleStates(Provider& provider, Updater& updater);

  template<typename TurtleContent, typename Provider, typename Updater, typename AgentCreator>
  void synchronize(Provider& provider, Updater& updater, AgentCreator& creator, RepastProcess::EXCHANGE_PATTERN exchangePattern = RepastProcess::POLL
#ifdef SHARE_AGENTS_BY_SET
    , bool declareNoAgentsKeptOnAnyProcess = false
#endif
    );


protected:
	typedef SharedNetwork<RelogoAgent, RelogoLink, RelogoLinkContent, RelogoLinkContentManager> NetworkType;

	Observer();
	Properties _props;
	int _rank;
	GridDimensions localBounds;
	repast::SharedContext<RelogoAgent> context;
	std::vector<repast::DataSet*> dataSets;

	NetworkType* findNetwork(const std::string& name);
private:
	friend class WorldCreator;
	const static int NO_TYPE_ID;

	// first is the typeid, second is the next good id of that type.
	typedef std::map<const std::type_info*, std::pair<int, int>, TypeInfoCmp> TypeMap;
	typedef boost::filter_iterator<IsNotType<RelogoAgent> , Context<RelogoAgent>::const_iterator>
			const_not_type_iterator;
	typedef TypeMap::iterator TypeMapIterator;



	TypeMap typeMap;
	static int nextTypeId;

	template<typename Agent>
	int getTypeId();

	void initBounds(GridDimensions& bounds);

	IntUniformGenerator* rndXP, *rndYP;
	DoubleUniformGenerator* rndX, *rndY;

};

template<typename AgentType>
AgentType* Observer::hatch(RelogoAgent* parent) {
	return hatch<AgentType> (parent, DefaultAgentCreator<AgentType> ());
}

template<typename AgentType, typename FactoryFunctor>
AgentType* Observer::hatch(RelogoAgent* parent, FactoryFunctor agentCreator) {
	const std::type_info* info = &(typeid(AgentType));
	int agentTypeId = 0;
	int id = 0;
	TypeMapIterator iter = typeMap.find(info);
	if (iter == typeMap.end()) {
		agentTypeId = nextTypeId;
		nextTypeId++;
	} else {
		agentTypeId = iter->second.first;
		id = iter->second.second;
	}

	AgentType* agent = agentCreator(repast::AgentId(id, _rank, agentTypeId), this);
	agent->hatchCopy(parent);
	context.addAgent(agent);
	agent->setxy(parent->xCor(), parent->yCor());
	typeMap[info] = std::make_pair(agentTypeId, id + 1);
	return agent;
}

template<typename AgentType>
int Observer::create(size_t count) {
	return create<AgentType> (count, DefaultAgentCreator<AgentType> ());
}

template<typename AgentType>
int Observer::getTypeId() {
	const std::type_info* info = &(typeid(AgentType));
	TypeMapIterator iter = typeMap.find(info);
	if (iter == typeMap.end())
		return NO_TYPE_ID;
	else
		return iter->second.first;
}

template<typename AgentType, typename FactoryFunctor>
int Observer::create(size_t count, FactoryFunctor agentCreator) {
	const std::type_info* info = &(typeid(AgentType));
	int agentTypeId = 0;
	int id = 0;
	TypeMapIterator iter = typeMap.find(info);
	if (iter == typeMap.end()) {
		agentTypeId = nextTypeId;
		nextTypeId++;
	} else {
		agentTypeId = iter->second.first;
		id = iter->second.second;
	}

	DoubleUniformGenerator gen = Random::instance()->createUniDoubleGenerator(0, 360);
	for (size_t i = 0; i < count; i++) {
//		AgentType* agent = agentCreator(repast::AgentId(id + i, _rank, agentTypeId), this);
	  repast::AgentId agentid(id+i, _rank, agentTypeId);
	  agentid.currentRank(_rank);
	  AgentType* agent = agentCreator(agentid, this);
		agent->heading((float) gen.next());
		context.addAgent(agent);
	}
	typeMap[info] = std::make_pair(agentTypeId, id + count);
	return agentTypeId;
}

template<typename AgentType>
AgentType* Observer::get(const AgentId& id) {
	return static_cast<AgentType*> (context.getAgent(id));
}

template<typename Agent>
AgentSet<Agent> Observer::get() {
	AgentSet<Agent> agentSet;
	get(agentSet);
	return agentSet;
}

template<typename AgentType>
void Observer::get(AgentSet<AgentType>& agentSet) {
  int typeId = getTypeId<AgentType> ();
	if (typeId != NO_TYPE_ID) {
		for (SharedContext<RelogoAgent>::const_local_iterator iter = context.localBegin(); iter != context.localEnd(); ++iter) {
			AgentId id = iter->get()->getId();
			if (id.agentType() == typeId) {
				agentSet.add(static_cast<AgentType*> (iter->get()));
			}
		}
	}
}

template<typename AgentType>
AgentSet<AgentType> Observer::turtlesAt(int x, int y) {
	AgentSet<AgentType> set;
	turtlesAt(x, y, set);
	return set;
}

template<typename AgentType>
void Observer::turtlesAt(int x, int y, AgentSet<AgentType>& set) {
	int typeId = getTypeId<AgentType> ();
	if (typeId != NO_TYPE_ID) {
		std::vector<RelogoAgent*> out;
		grid()->getObjectsAt(Point<int> (x, y), out);
		filterVecToSet(out, set, typeId);
	}
}

template<typename LinkCreator>
void Observer::createLink(RelogoAgent* source, RelogoAgent* target, const std::string& name, LinkCreator& creator) {
	NetworkType* net = findNetwork(name);
	boost::shared_ptr<RelogoLink> link(creator(source, target));
	net->addEdge(link);
}


template<typename AgentType>
void Observer::predecessors(RelogoAgent* agent, const std::string& networkName, AgentSet<AgentType>& out) {
	int typeId = getTypeId<AgentType> ();
	if (typeId != NO_TYPE_ID) {
		NetworkType* net = findNetwork(networkName);
		std::vector<RelogoAgent*> agents;
		net->predecessors(agent, agents);
		filterVecToSet(agents, out, typeId);
	}

}

template<typename AgentType>
void Observer::successors(RelogoAgent* agent, const std::string& networkName, AgentSet<AgentType>& out) {
	int typeId = getTypeId<AgentType> ();
	if (typeId != NO_TYPE_ID) {
		NetworkType* net = findNetwork(networkName);
		std::vector<RelogoAgent*> agents;
		net->successors(agent, agents);
		filterVecToSet(agents, out, typeId);
	}
}

template<typename PatchType>
AgentSet<PatchType> Observer::patches() {
	AgentSet<PatchType> set;
	patches(set);
	return set;
}

template<typename PatchType>
void Observer::patches(AgentSet<PatchType>& set) {
	boost::transform_iterator<Caster<PatchType> , Context<RelogoAgent>::const_bytype_iterator> begin(
			context.byTypeBegin(PATCH_TYPE_ID));
	boost::transform_iterator<Caster<PatchType> , Context<RelogoAgent>::const_bytype_iterator> end(context.byTypeEnd(
			PATCH_TYPE_ID));
	set.addAll(begin, end);
}

template<typename PatchType>
PatchType* Observer::patchAt(int x, int y) {
	return static_cast<PatchType*> (patchAt(x, y));
}

template<typename TurtleType>
void Observer::turtlesOn(const RelogoAgent* agent, AgentSet<TurtleType>& out) {
  int typeId = getTypeId<TurtleType> ();
	if (typeId != NO_TYPE_ID) {
		std::vector<RelogoAgent*> in;
		grid()->getObjectsAt(Point<int> (agent->pxCor(), agent->pyCor()), in);
		filterVecToSet(in, out, typeId);
	}
}

template<typename TurtleType>
void Observer::turtlesOn(AgentSet<RelogoAgent>& agentSet, AgentSet<TurtleType>& out) {
  int typeId = getTypeId<TurtleType> ();
	if (typeId != NO_TYPE_ID) {
		std::vector<RelogoAgent*> in;
		for (AgentSet<RelogoAgent>::const_as_iterator iter = agentSet.begin(); iter != agentSet.end(); ++iter) {
			RelogoAgent* agent = *iter;
			grid()->getObjectsAt(Point<int> (agent->pxCor(), agent->pyCor()), in);
		}
		filterVecToSetNoDuplicates(in, out, typeId);
	}
}

template<typename AgentType>
void Observer::inRadius(const Point<double>& center, AgentSet<RelogoAgent>& inSet, double radius,
		AgentSet<AgentType>& outSet) {
	int typeId = getTypeId<AgentType> ();
	if (typeId != NO_TYPE_ID) {
		double radiusSq = radius * radius;
		std::vector<RelogoAgent*> vec;
		const RelogoSpaceType* spc = space();
		for (size_t i = 0; i < inSet.size(); i++) {
			RelogoAgent* agent = inSet[i];
			if (spc->getDistanceSq(center, agent->location()) <= radiusSq && agent->getId().agentType() == typeId) {
				outSet.add(static_cast<AgentType*> (const_cast<RelogoAgent*> (agent)));
			}
		}
	}
}

template<typename TurtleContent, typename Provider, typename Updater>
void Observer::synchronizeTurtleStates(Provider& provider, Updater& updater) {
	repast::RepastProcess::instance()->synchronizeAgentStates<TurtleContent>(provider, updater);
}

template<typename TurtleContent, typename Provider, typename Updater, typename AgentCreator>
void Observer::synchronizeTurtleStatus(Provider& provider, Updater& updater, AgentCreator& creator, RepastProcess::EXCHANGE_PATTERN exchangePattern) {
  repast::RepastProcess::instance()->synchronizeAgentStatus<RelogoAgent, TurtleContent, Provider, Updater, AgentCreator>(context, provider, updater, creator, exchangePattern);
}

template<typename TurtleContent, typename Provider, typename Updater, typename AgentCreator>
void Observer::synchronize(Provider& provider, Updater& updater, AgentCreator& creator, RepastProcess::EXCHANGE_PATTERN exchangePattern
#ifdef SHARE_AGENTS_BY_SET
    , bool declareNoAgentsKeptOnAnyProcess
#endif
    ){

  context.getProjection(SPACE_NAME)->balance();

  synchronizeTurtleStatus<TurtleContent>(provider, updater, creator, exchangePattern);

#ifdef SHARE_AGENTS_BY_SET
  repast::RepastProcess::instance()->synchronizeProjectionInfo<RelogoAgent, TurtleContent, Provider, AgentCreator, Updater>(context, provider, updater, creator, exchangePattern, declareNoAgentsKeptOnAnyProcess);
#else
  repast::RepastProcess::instance()->synchronizeProjectionInfo<RelogoAgent, TurtleContent, Provider, AgentCreator, Updater>(context, provider, updater, creator, exchangePattern);
#endif


//  synchronizeTurtleStates<TurtleContent>(provider, updater);
}


template<typename AgentType>
AgentType* Observer::who(const AgentId& id) {
	RelogoAgent* agent = context.getAgent(id);
	if (agent != 0) {
		return static_cast<AgentType*> (agent);
	}
	return 0;
}

}
}
#endif /* OBSERVER_H_ */
