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
 *  Turtle.h
 *
 *  Created on: Jul 15, 2010
 *      Author: nick
 */

#ifndef TURTLE_H_
#define TURTLE_H_

#include "AbstractRelogoAgent.h"
#include "relogo.h"
#include "Observer.h"
#include "utility.h"

namespace repast {

namespace relogo {

class RelogoLink;
class Patch;

/**
 * Relogo Turtle implementation.
 */
class Turtle: public AbstractRelogoAgent {

public:

	using RelogoAgent::xCor;
	using RelogoAgent::yCor;

	/**
	 * Creates a Turtle that will have the specified id, and be managed by the
	 * specified Observer.
	 */
	Turtle(repast::AgentId id, Observer* observer);
	virtual ~Turtle() {
	}

	// Doc from RelogoAgent.h
	virtual void hatchCopy(RelogoAgent* parent){
	  Turtle* parentTurtle = (Turtle*)parent;
	  _heading = parentTurtle->heading();
	}

	/**
	 * Sets the x coordinate of the Turtle's location. If the
	 * location is outside of the world bounds this will throw an exception.
	 *
	 * @param x the new coordinate
	 */
	void xCor(double x);

	/**
	 * Sets the y coordinate of the Turtle's location. If the
	 * location is outside of the world bounds this will throw an exception.
	 *
	 * @param y the new y coordinate
	 */
	void yCor(double y);

	/**
	 * Sets the x and y coordinate of the Turtle's location. If the
	 * location is outside of the world bounds this will throw an exception.
	 *
	 * @param x the x coordinate
	 * @param y the y coordinate
	 */
	void setxy(double x, double y);

	/**
	 * Gets the Patch x coordinate of this Turtle.
	 *
	 * @return the patch x coordinate of this Turtle.
	 */
	virtual int pxCor() const;

	/**
	 * Gets the Patch x coordinate of this Turtle.
	 *
	 * @return the patch x coordinate of this Turtle.
	 */
	virtual int pyCor() const;

	/**
	 * Removes this turtle from the world. Do not call this
	 * if there is a chance the Turtle will be referred to
	 * after the call to die()- for example, if it has moved
	 * and will be part of a move synchronization.
	 */
	void die();

	/**
	 * Creates a link between this turtle and the specified turtle in the specified
	 * undirected network. The network defaults to the default undirected network.
	 *
	 * @param turtle the turtle to create the link with
	 * @param network the network to create the link in
	 */
	void createLinkWith(Turtle* turtle, const std::string& network = DEFAULT_UNDIR_NET);

	/**
	 * Creates a link between this turtle and the specified turtle in the specified
	 * undirected network, using the specified linke creator.  The network defaults to the
	 * default undirected network.
	 *
	 * @param turtle the turtle to create the link with
	 * @param creator the functor to create the link with
	 * @param network the network to create the link in
	 *
	 * @tparam LinkCreator an function or functor with the following signature
	 * RelogoLink* (Turtle* source, Turtle* target)
	 */
	template<typename LinkCreator>
	void createLinkWithLC(Turtle* turtle, LinkCreator& creator, const std::string& network = DEFAULT_UNDIR_NET);

	/**
	 * Creates links between this turtle and all the agents in the AgentSet in the named network.
	 * The network defaults to the default undirected network.
	 *
	 * @param agents the agentset of agents to create links with
	 * @param network the name of the network to create the links in. This defaults to
	 * the default undirected network
	 *
	 * @tparam Agent the type of object contained by the agentset.
	 */
	template<typename AgentType>
	void createLinksWith(AgentSet<AgentType>& agents, const std::string& network = DEFAULT_UNDIR_NET);

	/**
	 * Creates links between this turtle and all the agents in the agentset using
	 * the link creator and in the named network. The network defaults to the default
	 * undirected network.
	 *
	 * @param agents the agentset of agents to create links with
	 * @param network the name of the network to create the links in. This defaults to
	 * the default undirected network
	 * @param creator the functor to create the links with
	 *
	 * @tparam Agent the type of object contained by the agentset
	 * @tparam LinkCreator the object used to create the links
	 *
	 * @tparam LinkCreator an function or functor with the following signature
	 * RelogoLink* (Turtle* source, Turtle* target)
	 */
	template<typename AgentType, typename LinkCreator>
	void createLinksWithLC(AgentSet<AgentType>& agents, LinkCreator& creator, const std::string& network =
			DEFAULT_UNDIR_NET);

	/**
	 * Creates a link to this Turtle from the specified turtle in the named network
	 * which defaults to the default directed network.
	 *
	 * @param turtle the turtle that will be the source turtle of the lin
	 * @param network the name of the network
	 */
	void createLinkFrom(Turtle* turtle, const std::string& network = DEFAULT_DIR_NET);

	/**
	 * Creates a link to this Turtle from the specified turtle in the named network which
	 * defaults to the default directed network.
	 *
	 * @param turtle the turtle that will be the source turtle of the link
	 * @param network the name of the network
	 * @param linkCreator an object used to create the link
	 *
	 * @tparam LinkCreator an function or functor with the following signature
	 * RelogoLink* (Turtle* source, Turtle* target)
	 */
	template<typename LinkCreator>
	void createLinkFromLC(Turtle* turtle, LinkCreator& linkCreator, const std::string& network = DEFAULT_DIR_NET);

	/**
	 * Creates links to this turtle from all the agents in the agentset in the named network.
	 * The network defaults to the default directed network.
	 *
	 * @param agents the agentset of agents to create links from
	 * @param network the name of the network to create the links in. This defaults to
	 * the default directed network
	 *
	 * @tparam AgentType the type of object contained by the agentset.
	 *
	 */
	template<typename AgentType>
	void createLinksFrom(AgentSet<AgentType>& agents, const std::string& network = DEFAULT_DIR_NET);

	/**
	 * Creates links to this turtle from all the agents in the agentset using
	 * the link creator and in the named network. The network defaults to the default
	 * directed network.
	 *
	 * @param agents the agentset of agents to create links from
	 * @param network the name of the network to create the links in. This defaults to
	 * the default directed network
	 *
	 * @tparam AgentType the type of object contained by the agentset
	 * @tparam LinkCreator an function or functor with the following signature
	 * RelogoLink* (Turtle* source, Turtle* target)
	 */
	template<typename AgentType, typename LinkCreator>
	void createLinksFromLC(AgentSet<AgentType>& agents, LinkCreator& creator, const std::string& network =
			DEFAULT_DIR_NET);

	/**
	 * Creates links from this turtle to all the agents in the agentset in the named network.
	 * The network defaults to the default directed network.
	 *
	 * @param agents the agentset of agents to create links to
	 * @param network the name of the network to create the links in. This defaults to
	 * the default directed network
	 *
	 * @tparam AgentType the type of object contained by the agentset.
	 */
	template<typename AgentType>
	void createLinksTo(AgentSet<AgentType>& agents, const std::string& network = DEFAULT_DIR_NET);

	/**
	 * Creates links from this turtle to all the agents in the agentset using
	 * the link creator and in the named network. The network defaults to the default
	 * directed network.
	 *
	 * @param agents the agentset of agents to create links to
	 * @param network the name of the network to create the links in. This defaults to
	 * the default directed network
	 *
	 * @tparam AgentType the type of object contained by the agentset
	 * @tparam LinkCreator an function or functor with the following signature
	 * boost::shared_ptr<RelogoLink> (Turtle* source, Turtle* target)
	 */
	template<typename AgentType, typename LinkCreator>
	void
	createLinksToLC(AgentSet<AgentType>& agents, LinkCreator& creator, const std::string& network = DEFAULT_DIR_NET);

	/**
	 * Creates a link from this Turtle to the specified turtle in the named network which
	 * defaults to the default directed network.
	 *
	 * @param turtle the turtle that will be the target turtle of the link
	 * @param network the name of the network
	 */
	void createLinkTo(Turtle* turtle, const std::string& network = DEFAULT_DIR_NET);

	/**
	 * Creates a link from this Turtle to the specified turtle in the named network
	 * which defaults to the default directed network.
	 *
	 * @param turtle the turtle that will be the target turtle of the link
	 * @param network the name of the network
	 * @param linkCreator an object used to create the link
	 * @tparam LinkCreator an function or functor with the following signature
	 * boost::shared_ptr<RelogoLink> (Turtle* source, Turtle* target)
	 */
	template<typename LinkCreator>
	void createLinkToLC(Turtle* turtle, LinkCreator& linkCreator, const std::string& network = DEFAULT_DIR_NET);

	/**
	 * Gets the link from the specified turtle to this one in the
	 * specified network which defaults to the default directed network.
	 *
	 * @param turtle the turtle to get the link from
	 * @param name the name of the network containing the link
	 *
	 * @return the link from the specified turtle to this one in the
	 * specified network which defaults to the default directed network.
	 */
	boost::shared_ptr<RelogoLink> inLinkFrom(Turtle* turtle, const std::string& name = DEFAULT_DIR_NET);

	/**
	 * Gets the link from the this turtle to the specified turtle in the
	 * specified network which defaults to the default directed network.
	 *
	 * @param turtle the turtle to get the link to
	 * @param name the name of the network containing the link
	 *
	 * @return the link from the this turtle to the specified turtle in the
	 * specified network which defaults to the default directed network.
	 */
	boost::shared_ptr<RelogoLink> outLinkTo(Turtle* turtle, const std::string& name = DEFAULT_DIR_NET);

	/**
	 * Gets the link between this turtle and the specified on in the named
	 * undirected network. The network defaults to the default undirected
	 * network.
	 *
	 * @return the link between this turtle and the specified on in the named
	 * undirected network.
	 */
	boost::shared_ptr<RelogoLink> linkWith(Turtle* turtle, const std::string& name = DEFAULT_UNDIR_NET);

	/**
	 * Gets whether or not this turtle is linked to the specified turtle, in the
	 * specified network. The network defaults to the default undirected network.
	 *
	 * @param turtle the turtle to check that this links to
	 * @param name the name of the network to check, defaults to the default undirected
	 * network.
	 *
	 * @return true if this this turtle is linked to the specified turtle in the
	 * named undirected network, otherwise false.
	 */
	bool linkNeighborQ(Turtle* turtle, const std::string& name = DEFAULT_UNDIR_NET);

	/**
	 * Gets all the network neighbors of this turtle in the named network and
	 * puts them in the specified AgentSet.
	 *
	 * @param out the AgentSet to the return the neighbors in
	 * @param name the name of the network to get the network neighbors from
	 * @tparam AgentType the type of agents to find in the network
	 */
	template<typename AgentType>
	void linkNeighbors(AgentSet<AgentType>& out, const std::string& name = DEFAULT_UNDIR_NET);

	/**
	 * Gets whether or not there is an edge into this turtle from the specified turtle, in the
	 * specified network. The network defaults to the default directed network.
	 *
	 * @param turtle the turtle to check as the source of the edge
	 * @param name the name of the network to check, defaults to the default directed
	 * network.
	 *
	 * @return true if there is an edge into this turtle from the specified turtle in the
	 * named directed network, otherwise false.
	 */
	bool inLinkNeighborQ(Turtle* turtle, const std::string& name = DEFAULT_DIR_NET);

	/**
	 * Gets all the network predecessors of this turtle in the named network and
	 * puts them in the specified array list.
	 *
	 * @param out the AgentSet to the return the neighbors in
	 * @param name the name of the network to get the network neighbors from
	 * @tparam AgentType the type of agents to find in the network
	 */
	template<typename AgentType>
	void inLinkNeighbors(AgentSet<AgentType>& out, const std::string& name = DEFAULT_DIR_NET);

	/**
	 * Gets whether or not there is an edge from this turtle to the specified turtle, in the
	 * specified network. The network defaults to the default directed network.
	 *
	 * @param turtle the turtle to check as the target of the edge
	 * @param name the name of the network to check, defaults to the default directed
	 * network.
	 *
	 * @return true if there is an edge from this turtle into the specified turtle in the
	 * named directed network, otherwise false.
	 */
	bool outLinkNeighborQ(Turtle* turtle, const std::string& name = DEFAULT_DIR_NET);

	/**
	 * Gets all the network successors of this turtle in the named network and
	 * puts them in the specified array list.
	 *
	 * @param out the AgentSet to the return the neighbors in
	 * @param name the name of the network to get the network neighbors from
	 * @tparam AgentType the type of agents to find in the network
	 */
	template<typename AgentType>
	void outLinkNeighbors(AgentSet<AgentType>& out, const std::string& name = DEFAULT_DIR_NET);

	/**
	 * Moves this turtle to the location of the specified turtle.
	 *
	 * @param turtle the turtle whose location this turtle will be moved to
	 *
	 */
	void moveTo(Turtle* turtle);

	/**
	 * Moves this turtle to the location of the specified patch.
	 *
	 * @param patch the patch whose location this turtle will be moved to
	 */
	void moveTo(Patch* patch);

	/**
	 * Moves this turtle the specified distance along the current heading.
	 *
	 * @param distance the distance to move
	 */
	void move(double distance);

	/**
	 * Moves this turtle the specified distance along the current heading.
	 *
	 * @param distance the distance to move
	 */
	void mv(double distance) {
		move(distance);
	}

	/**
	 * Moves this turtle forward the specified distance, if and only if
	 * that would not take this turtle outside the current topology.
	 *
	 * @param distance the amount to move
	 */
	void jump(double distance) {
		if (canMoveQ(distance))
			forward(distance);
	}

	/**
	 * Moves this turtle forward the specified distance
	 *
	 * @param distance the distance to move
	 */
	void forward(double distance) {
		move(distance);
	}

	/**
	 * Moves this turtle backward the specified distance
	 *
	 * @param distance the distance to move
	 */
	void backward(double distance) {
		move(-distance);
	}

	/**
	 * Moves this turtle forward the specified distance
	 *
	 * @param distance the distance to move
	 */
	void fd(double distance) {
		forward(distance);
	}

	/**
	 * Moves this turtle backward the specified distance
	 *
	 * @param distance the distance to move
	 */
	void bk(double distance) {
		backward(-distance);
	}

	/**
	 * Moves this turtle to a neighboring patch with lowest value as retrieved via the ValueGetter.
	 * The 8 neighboring patches and the current patch the turtle is on are considered.
	 * If no surrounding patch has a lower value than the patch the turtle is on, this
	 * turtle stays on the current patch. If there is more than one patch with the
	 * minimum value, then one will be chosen at random.
	 * .
	 * Note that this turtle will end up in the
	 * center of one of the surrounding patches or in the center of its current patch.
	 *
	 * @param getter the function or functor used to retrieve the value from the patch
	 *
	 * @tparam PatchType the patch's type
	 * @tparam a functor or function with the following signature
	 * double (PatchType* patch) const
	 *
	 *
	 */
	template<typename PatchType, typename ValueGetter>
	void downhill(ValueGetter& getter);

	/**
	 * Moves this turtle to the patch with lowest value as retrieved via the ValueGetter.
	 * The 4 neighboring patches and the current patch the turtle is on are considered.
	 * If no surrounding patch has a lower value than the patch the turtle is on, this
	 * turtle stays on the current patch. If there is more than on patch with the
	 * minimum value, then one will be chosen at random. This considers only the current
	 * patch and the 4 surrounding patches (N, S, E, W).
	 *
	 * Note that this turtle will end up in the
	 * center of one of the surrounding patches or in the center of its current patch.
	 *
	 * @param getter the function or functor used to retrieve the value from the patch
	 * @tparam PatchType the patch's type
	 * @tparam a functor or function with the following signature
	 * double (PatchType* patch) const
	 *
	 */
	template<typename PatchType, typename ValueGetter>
	void downhill4(ValueGetter& getter);

	/**
	 * Moves this turtle to the patch with highest value as retrieved via the ValueGetter.
	 * The 8 neighboring patches and the current patch the turtle is on are considered.
	 * If no surrounding patch has a higher value than the patch the turtle is on, this
	 * turtle stays on the current patch. If there is more than on patch with the
	 * minimum value, then one will be chosen at random.
	 *
	 * Note that this turtle will end up in the
	 * center of one of the surrounding patches or in the center of its current patch.
	 *
	 * @param getter the function or functor used to retrieve the value from the patch
	 * @tparam PatchType the patch's type
	 * @tparam a functor or function with the following signature
	 * double (PatchType* patch) const
	 *
	 */
	template<typename PatchType, typename ValueGetter>
	void uphill(ValueGetter& getter);

	/**
	 * Moves this turtle to the patch with highest value as retrieved via the ValueGetter.
	 * The 4 neighboring patches and the current patch the turtle is on are considered.
	 * If no surrounding patch has a higher value than the patch the turtle is on, this
	 * turtle stays on the current patch. If there is more than on patch with the
	 * minimum value, then one will be chosen at random. This considers only the current
	 * patch and the 4 surrounding patches (N, S, E, W).
	 *
	 * Note that this turtle will end up in the
	 * center of one of the surrounding patches or in the center of its current patch.
	 *
	 * @param getter the function or functor used to retrieve the value from the patch
	 * @tparam PatchType the patch's type
	 * @tparam a functor or function with the following signature
	 * double (PatchType* patch) const
	 *
	 */
	template<typename PatchType, typename ValueGetter>
	void uphill4(ValueGetter& getter);

	/**
	 * Gets the distance traveled along the x dimension if the turtle were to take one step forward along
	 * its current heading.
	 *
	 * @return the distance traveled along the x dimension if the turtle were to take one step forward along
	 * its current heading.
	 */
	double dx() const;

	/**
	 * Gets the distance traveled along the y dimension if the turtle were to take one step forward along
	 * its current heading.
	 *
	 * @return the distance traveled along the y dimension if the turtle were to take one step forward along
	 * its current heading.
	 */
	double dy() const;

	/**
	 * Gets whether or not this turtle can move the specified
	 * distance along its current heading given the current topology.
	 *
	 * @return true if this turtle can move the specified
	 * distance along its current heading given the current topology, otherwise false
	 */
	bool canMoveQ(double distance) const;

	/**
	 * Gets the heading from this turtle to the specified RelogoAgent (turtle or patch).
	 *
	 * @param agent the Turtle or Patch this will get the heading to
	 *
	 * @return the heading from this turtle to the specified RelogoAgent (turtle or patch).
	 */
	float towards(RelogoAgent* agent) const;

	/**
	 * Gets the heading from this turtle to the specified location.
	 *
	 * @param x the x coordinate of the location
	 * @param y the y coordinate of the location
	 *
	 * @return the heading from this turtle to the specified location.
	 */
	float towardsxy(double x, double y) const;

	/**
	 * Gets the heading from this turtle to the specified location.
	 *
	 * @param location the location to get the heading to
	 *
	 * @return the heading from this turtle to the specified location.
	 */
	float towards(const Point<double>& location) const;

	/**
	 * Gets the distance from this turtle to the specified turtle.
	 *
	 * @param turtle the turtle to get the distance to
	 *
	 * @return the distance from this turtle to the specified turtle.
	 */
	double distance(Turtle* turtle) const;

	/**
	 * Gets this Turtle's current heading.
	 *
	 * @return this Turtle's current heading.
	 */
	float heading() const {
		return _heading;
	}

	/**
	 * Sets this turtle's heading to the specified heading.
	 *
	 * @param heading the new heading
	 */
	void heading(float heading);

	/**
	 * Gets the patch under this turtle.
	 *
	 * @tparam the type of the Patch
	 */
	template<typename PatchType>
	PatchType* patchHere() const;

	/**
	 * Sets the turtles heading to face towards the specified turtle.
	 *
	 * @param turtle the turtle to face
	 */
	void face(Turtle* turtle);

	/**
	 * Sets the turtles heading to face towards the specified pach.
	 *
	 * @param patch the patch to face
	 */
	void face(Patch* patch);

	/**
	 * Sets the turtles heading to face the specified coordinates.
	 *
	 * @param nx the x coordinate of the location to face
	 * @param ny the y coordinate of the location to face
	 */
	void facexy(double nx, double ny);

	/**
	 * Turns the turtle left by the specified number of degrees.
	 * To turn right, use a negative number.
	 *
	 * @param degrees the amount to turn
	 */
	void left(float degrees);

	/**
	 * Turns the turtle left by the specified number of degrees.
	 * To turn right, use a negative number.
	 *
	 * @param degrees the amount to turn
	 */
	void lt(float degrees);

	/**
	 * Gets the patch that is the specified distance from this turtle, at
	 * the specified angle (turning left) from this turtle's heading. Returns 0
	 * if the patch would be outside of the world.
	 *
	 * @param angleInDegrees the angle
	 * @param distance the distance
	 *
	 * @tparam PatchType the type of the Patch
	 * @return the patch that is the specified distance from this turtle, at
	 * the specified angle (turning left) from this turtle's heading or 0
	 * if the patch would be outside of the world.
	 */
	template<typename PatchType>
	PatchType* patchLeftAndAhead(float angleInDegrees, double distance);

	/**
	 * Gets the patch that is the specified distance from this turtle, in
	 * the specified degrees (turning right) from this turtle's heading. Returns 0
	 * if the patch would be outside of the world.
	 *
	 * @param angleInDegrees the angle
	 * @param distance the distance
	 *
	 * @tparam PatchType the type of the Patch
	 * @return the patch that is the specified distance from this turtle, at
	 * the specified angle (turning right) from this turtle's heading or 0
	 * if the patch would be outside of the world.
	 */
	template<typename PatchType>
	PatchType* patchRightAndAhead(float angleInDegrees, double distance);

private:
	float _heading;
	bool moved;

	// contains the "tied" turtles
	typedef boost::unordered_set<Turtle*, AgentHashId<Turtle> > TiedSetType;
	TiedSetType fixedLeaves, freeLeaves;

	/*
	 * Moves tied turtles vector diff between oldLocation and this
	 * turtles current location
	 */
	void moveTiedTurtles(const Point<double>& oldLocation);
	/*
	 * Moves a tied turtle as the result of this turtle changing its heading.
	 */
	void moveTiedTurtle(Turtle* t, float angleTurned);

};

template<typename LinkCreator>
void Turtle::createLinkFromLC(Turtle* turtle, LinkCreator& linkCreator, const std::string& network) {
	_observer->createLink(turtle, this, network, linkCreator);
}

template<typename LinkCreator>
void Turtle::createLinkToLC(Turtle* turtle, LinkCreator& linkCreator, const std::string& network) {
	_observer->createLink(this, turtle, network, linkCreator);
}

template<typename LinkCreator>
void Turtle::createLinkWithLC(Turtle* turtle, LinkCreator& linkCreator, const std::string& network) {
	_observer->createLink(this, turtle, network, linkCreator);
}

template<typename AgentType>
void Turtle::createLinksWith(AgentSet<AgentType>& agents, const std::string& network) {
	for (size_t i = 0, n = agents.size(); i < n; i++) {
		_observer->createLink(this, agents[i], network);
	}
}

template<typename AgentType, typename LinkCreator>
void Turtle::createLinksWithLC(AgentSet<AgentType>& agents, LinkCreator& creator, const std::string& network) {
	for (size_t i = 0, n = agents.size(); i < n; i++) {
		_observer->createLink(this, agents[i], network, creator);
	}
}

template<typename AgentType>
void Turtle::createLinksFrom(AgentSet<AgentType>& agents, const std::string& network) {
	for (size_t i = 0, n = agents.size(); i < n; i++) {
		_observer->createLink(agents[i], this, network);
	}
}

template<typename AgentType, typename LinkCreator>
void Turtle::createLinksFromLC(AgentSet<AgentType>& agents, LinkCreator& creator, const std::string& network) {
	for (size_t i = 0, n = agents.size(); i < n; i++) {
		_observer->createLink(agents[i], this, network, creator);
	}
}

template<typename AgentType>
void Turtle::createLinksTo(AgentSet<AgentType>& agents, const std::string& network) {
	for (size_t i = 0, n = agents.size(); i < n; i++) {
		_observer->createLink(this, agents[i], network);
	}
}

template<typename AgentType, typename LinkCreator>
void Turtle::createLinksToLC(AgentSet<AgentType>& agents, LinkCreator& creator, const std::string& network) {
	for (size_t i = 0, n = agents.size(); i < n; i++) {
		_observer->createLink(this, agents[i], network, creator);
	}
}

template<typename AgentType>
void Turtle::linkNeighbors(AgentSet<AgentType>& out, const std::string& name) {
	_observer->predecessors(this, name, out);
}

template<typename AgentType>
void Turtle::inLinkNeighbors(AgentSet<AgentType>& out, const std::string& name) {
	_observer->predecessors(this, name, out);
}

template<typename AgentType>
void Turtle::outLinkNeighbors(AgentSet<AgentType>& out, const std::string& name) {
	_observer->successors(this, name, out);
}

template<typename PatchType>
PatchType* Turtle::patchHere() const {
	return _observer->patchAt<PatchType> (doubleCoordToInt(_location[0]), doubleCoordToInt(_location[1]));
}

template<typename PatchType, typename ValueGetter>
void Turtle::uphill(ValueGetter& getter) {
	PatchType* myPatch = patchHere<PatchType> ();
	moveTo(myPatch);
	// ".template" is needed to tell the compiler that "<" is
	// a template and not a less than.
	AgentSet<PatchType> patches = (*myPatch).template neighbors<PatchType> ();
	PatchType* max = patches.maxOneOf(getter);
	if (getter(max) > getter(myPatch)) {
		face(max);
		moveTo(max);
	}
}

template<typename PatchType, typename ValueGetter>
void Turtle::uphill4(ValueGetter& getter) {
	PatchType* myPatch = patchHere<PatchType> ();
	moveTo(myPatch);
	// ".template" is needed to tell the compiler that "<" is
	// a template and not a less than.
	AgentSet<PatchType> patches = (*myPatch).template neighbors4<PatchType> ();
	PatchType* max = patches.maxOneOf(getter);
	if (getter(max) > getter(myPatch)) {
		face(max);
		moveTo(max);
	}
}

template<typename PatchType, typename ValueGetter>
void Turtle::downhill(ValueGetter& getter) {
	PatchType* myPatch = patchHere<PatchType> ();
	moveTo(myPatch);
	// ".template" is needed to tell the compiler that "<" is
	// a template and not a less than.
	AgentSet<PatchType> patches = (*myPatch).template neighbors<PatchType> ();
	PatchType* min = patches.minOneOf(getter);
	if (getter(min) < getter(myPatch)) {
		face(min);
		moveTo(min);
	}
}

template<typename PatchType, typename ValueGetter>
void Turtle::downhill4(ValueGetter& getter) {
	PatchType* myPatch = patchHere<PatchType> ();
	moveTo(myPatch);
	// ".template" is needed to tell the compiler that "<" is
	// a template and not a less than.
	AgentSet<PatchType> patches = (*myPatch).template neighbors4<PatchType> ();
	PatchType* min = patches.minOneOf(getter);
	if (getter(min) < getter(myPatch)) {
		face(min);
		moveTo(min);
	}
}

template<typename PatchType>
PatchType* Turtle::patchLeftAndAhead(float angleInDegrees, double distance) {
	float heading = fmodf(_heading - angleInDegrees, 360);
	if (heading < 0)
		heading += 360;
	std::vector<double> disp = calcDisplacementFromHeadingDistance(heading, distance);
	return static_cast<PatchType*> (_observer->patchAt(_location, disp[0], disp[1]));
}

template<typename PatchType>
PatchType* Turtle::patchRightAndAhead(float angleInDegrees, double distance) {
	float heading = fmodf(angleInDegrees + _heading, 360);
	if (heading < 0)
		heading += 360;
	std::vector<double> disp = calcDisplacementFromHeadingDistance(heading, distance);
	return static_cast<PatchType*> (_observer->patchAt(_location, disp[0], disp[1]));
}

}
}

#endif /* TURTLE_H_ */
