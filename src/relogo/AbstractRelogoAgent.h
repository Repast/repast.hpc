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
 *  AbstractRelogoAgent.h
 *
 *  Created on: Aug 17, 2010
 *      Author: nick
 */

#ifndef ABSTRACTRELOGOAGENT_H_
#define ABSTRACTRELOGOAGENT_H_

#include "RelogoAgent.h"
#include "Observer.h"
#include "utility.h"

namespace repast {
namespace relogo {

/**
 * Abstract base class for turtles and patches. This contains some methods that can
 * apply to either.
 */
class AbstractRelogoAgent: public RelogoAgent {

public:
	AbstractRelogoAgent(AgentId id, Observer* observer);
	virtual ~AbstractRelogoAgent();

	/**
	 * Gets the patch x coordinate of the agent's location.
	 *
	 * @return the  patch x coordinate
	 */
	virtual int pxCor() const = 0;

	/**
	 * Gets the patch y coordinate of the agent's location.
	 *
	 * @return the patch y coordinate
	 */
	virtual int pyCor() const = 0;

	/**
	 * Gets all the agents in the inSet within the specified radius for this RelogoAgent and put
	 * them in the outSet.
	 *
	 *@param inSet the set of agents to test if they are withinthe radius
	 * @tparam the type of agents to include in the outSet
	 */
	template<typename AgentType>
	void inRadius(AgentSet<RelogoAgent>& inSet, double radius, AgentSet<AgentType>& outSet) const;

	/**
	 * Gets the patch at direction dx, dy from the this agent.
	 * If the resulting location is outside of the world, this returns 0.
	 *
	 * @param dx the distance from the caller along the x dimension
	 * @param dy the distance from the caller along the y dimension
	 * @tparam the type of the Patch
	 *
	 * @return the patch at that distance from this Turtle, or 0
	 * if the resulting location is outside of the world.
	 */
	template<typename PatchType>
	PatchType* patchAt(double dx, double dy) const;

	/**
	 * Gets all the turtles on this turtle's patch and puts
	 * them into the specifed set.
	 *
	 *@param set the set to put the found turtles in
	 * @tparam AgentType the type of turtles to get
	 */
	template<typename AgentType>
	void turtlesHere(AgentSet<AgentType>& set) const;

	/**
	 * Gets all the turtles on this turtle's patch and
	 * returns them in an AgentSet.
	 *
	 * @tparam AgentType the type of turtles to get
	 *
	 * @return an AgentSet containing all the turtles on this
	 * turtles patch.
	 */
	template<typename AgentType>
	AgentSet<AgentType> turtlesHere() const;

	/**
	 * Gets the patch at the specified heading and distance from this
	 * patch or turtle.
	 *
	 *@param heading the heading
	 *@param distance the distance
	 * @tparam PatchType the patch's type
	 */
	template<typename PatchType>
	PatchType* patchAtHeadingAndDistance(float heading, double distance);

	/**
	 * Gets the turtles that are on this patch or if this is a Turtle
	 * get the turtles that are on the patch this turtle is on.
	 *
	 *@tparam AgentType the type of turtle
	 * @param out the turtles will be put in out
	 */
	template<typename AgentType>
	void turtlesOn(AgentSet<AgentType>& out) const;

	/**
	 * Gets the turtles that are on this patch or if this is a Turtle
	 * get the turtles that are on the patch this turtle is on.
	 *
	 * @tparam AgentType the type of turtle
	 *
	 * @return an AgentSet containing all the turtles that are on the patch
	 * the caller is on.
	 *
	 */
	template<typename AgentType>
	AgentSet<AgentType> turtlesOn() const;

};

template<typename AgentType>
void AbstractRelogoAgent::inRadius(AgentSet<RelogoAgent>& inSet, double radius, AgentSet<AgentType>& outSet) const {
	_observer->inRadius<AgentType> (_location, inSet, radius, outSet);
}

template<typename PatchType>
PatchType* AbstractRelogoAgent::patchAt(double dx, double dy) const {
	return static_cast<PatchType*> (_observer->patchAt(_location, dx, dy));
}

template<typename AgentType>
void AbstractRelogoAgent::turtlesHere(AgentSet<AgentType>& set) const {
	_observer->turtlesAt(pxCor(), pyCor(), set);
}

template<typename AgentType>
AgentSet<AgentType> AbstractRelogoAgent::turtlesHere() const {
	AgentSet<AgentType> set;
	turtlesHere(set);
	return set;
}

template<typename PatchType>
PatchType* AbstractRelogoAgent::patchAtHeadingAndDistance(float heading, double distance) {
	std::vector<double> disp = calcDisplacementFromHeadingDistance(heading, distance);
	return static_cast<PatchType*> (_observer->patchAt(_location, disp[0], disp[1]));
}

template<typename AgentType>
void AbstractRelogoAgent::turtlesOn(AgentSet<AgentType>& out) const {
	_observer->turtlesOn(this, out);
}

template<typename AgentType>
AgentSet<AgentType> AbstractRelogoAgent::turtlesOn() const {
	AgentSet<AgentType> agents;
	turtlesOn<AgentType>(agents);
	return agents;

}

}
}

#endif /* ABSTRACTRELOGOAGENT_H_ */
