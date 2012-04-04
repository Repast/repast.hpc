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
 *  RelogoAgent.h
 *
 *  Created on: Aug 5, 2010
 *      Author: nick
 */

#ifndef RELOGOAGENT_H_
#define RELOGOAGENT_H_

#include "repast_hpc/Point.h"
#include "repast_hpc/AgentId.h"

namespace repast {
namespace relogo {

class Observer;
class RelogoContinuousSpaceAdder;
class WorldCreator;

template<typename GPTransformer, typename Adder>
class RelogoSharedContinuousSpace;

/**
 * Base agent for Relogo.
 */
class RelogoAgent: public repast::Agent {

protected:
	friend class RelogoContinuousSpaceAdder;
	friend class WorldCreator;

	template<typename GPTransformer, typename Adder>
	friend class RelogoSharedContinuousSpace;

	Observer* _observer;
	Point<double> _location;
	repast::AgentId _id;

public:

	/**
	 * Creates a RelogoAgent with the specified id and managed by the specified Observer.
	 *
	 * @param id the id of this RelogoAgent
	 * @param observer the observer that will manage this agent.
	 */
	RelogoAgent(repast::AgentId id, Observer* observer) :
		_observer(observer), _location(0, 0), _id(id) {
	}

	virtual ~RelogoAgent() {
	}

	/**
	 * Gets the id of this RelogoAgent.
	 *
	 * @return the id of this RelogoAgent.
	 */
	virtual repast::AgentId& getId() {
		return _id;
	}

	/**
	 * Gets the const id of this RelogoAgent.
	 *
	 * @return the const id of this RelogoAgent.
	 */
	virtual const repast::AgentId& getId() const {
		return _id;
	}

	/**
	 * Gets the location of this RelogoAgent.
	 *
	 * @return the location of this RelogoAgent.
	 */
	Point<double> location() const {
		return _location;
	}

  /**
   * If this ReLogo agent is 'hatched', makes an appropriate
   * copy, setting instance variables as appropriate.
   */
	virtual void hatchCopy(){}

	/**
	 * Gets the x coordinate of the agent's location.
	 *
	 * @return the x coordinate of the agent's location.
	 */
	double xCor() const;

	/**
	 * Gets the y coordinate of the agent's location.
	 *
	 * @return the y coordinate of the agent's location.
	 */
	double yCor() const;

	/**
	 * Gets the patch x coordinate of the agent's location.
	 *
	 * @return the patch x coordinate of the agent's location.
	 */
	virtual int pxCor() const = 0;

	/**
	 * Gets the patch y coordinate of the agent's location.
	 *
	 * @return the patch y coordinate of the agent's location.
	 */
	virtual int pyCor() const = 0;

	/**
	 * Gets the distance from this RelogoAgent to the specified agent.
	 *
	 * @return the distance from this RelogoAgent to the specified agent.
	 */
	double distance(RelogoAgent* obj) const;

	/**
	 * Gets the distance from this RelogoAgent to the specified point.
	 *
	 * @return the distance from this RelogoAgent to the specified point.
	 */
	double distancexy(double x, double y) const;

};

}

}

#endif /* RELOGOAGENT_H_ */
