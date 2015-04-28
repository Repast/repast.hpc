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
 *  Grid.h
 *
 *  Created on: Aug 10, 2010
 *      Author: nick
 */

#ifndef GRID_H_
#define GRID_H_

#include <vector>
#include <utility>

#include "Projection.h"
#include "Point.h"
#include "GridDimensions.h"
#include "AgentId.h"

namespace repast {

/**
 * Abstract interface for Grids and ContinuousSpaces.
 *
 * @tparam T the type of objects this Grid contains
 * @tparam GPType the coordinate type of the grid point locations. This must
 * be an int or a double.
 */
template<typename T, typename GPType>
class Grid: public Projection<T> {
  typedef typename Projection<T>::RADIUS RADIUS;

public:

	/**
	 * Creates a Grid with the specified name.
	 *
	 * @param name the name of the Grid. This should be unique
	 * among Projections.
	 */
	Grid(std::string name) :
		Projection<T> (name) {
	}
	virtual ~Grid() {
	}

	/**
	 * Gets whether or not this grid contains the agent with the specified id.
	 *
	 * @param id the id of the agent to check
	 *
	 * @return true if the grid contains the agent, otherwise false.
	 */
	virtual bool contains(const AgentId& id) = 0;

	/**
	 * Moves the specified agent to the specified point.
	 *
	 * @param id the id of the agent to move
	 * @param pt where to move the agent to
	 *
	 * @return true if the move was successful, otherwise false
	 */
	virtual bool moveTo(const AgentId& id, const Point<GPType>& pt) = 0;

	/**
	 * Moves the specifed object the specified distance from its current
	 * position along the specified angle. For example, <code>moveByVector(object, 1, Grid.NORTH)</code>
	 * will move the object 1 unit "north" up the y-axis, assuming a 2D grid. Similarly,
	 * <code>grid.moveByVector(object, 2, 0, Math.toRadians(90), 0)</code> will rotate 90
	 * degrees around the y-axis, thus moving the object 2 units along the z-axis.
	 * <p/>
	 * <b> Note that the radians / degrees are incremented in a anti-clockwise fashion, such that
	 * 0 degrees is "east",  90 degrees is "north", 180 is "west" and 270 is "south."
	 *
	 * @param agent          the object to move
	 * @param distance        the distance to move
	 * @param anglesInRadians the angle to move along in radians.
	 * @return a pair containing a bool that indicates whether the move was a success or not, and the
	 * point where the agent was moved to.
	 */
	virtual std::pair<bool, Point<GPType> > moveByVector(const T* agent, double distance,
			const std::vector<double>& anglesInRadians) = 0;

	/**
	 * Moves the specified object from its current location by the specified
	 * amount. For example <code>moveByDisplacement(object, 3, -2, 1)</code>
	 * will move the object by 3 along the x-axis, -2 along the y and 1 along
	 * the z. The displacement argument can be less than the number of
	 * dimensions in the space in which case the remaining argument will be set
	 * to 0. For example, <code>moveByDisplacement(object, 3)</code> will move
	 * the object 3 along the x-axis and 0 along the y and z axes, assuming a 3D
	 * grid.
	 *
	 * @param agent
	 *            the object to move
	 * @param displacement
	 *            the amount to move the object
	 * @return a pair containing a bool that indicates whether the move was a success or not, and the
	 * point where the agent was moved to.
	 */
	virtual std::pair<bool, Point<GPType> >
	moveByDisplacement(const T* agent, const std::vector<GPType>& displacement) = 0;

	/**
	 * Gets the dimensions of this Grid.
	 *
	 * @return the dimensions of this Grid.
	 */
	virtual const GridDimensions dimensions() const = 0;

  virtual const GridDimensions bounds() const = 0;


	/**
	 * Gets the first object found at the specified point, or NULL if there is no
	 * such object.
	 *
	 * @return the first object found at the specified point, or NULL if there is no
	 * such object.
	 */
	virtual T* getObjectAt(const Point<GPType>& pt) const = 0;

	/**
	 * Gets all the objects found at the specified point. The found objects
	 * will be put into the out parameter.
	 *
	 * @param pt the point to get all the objects at
	 * @param [out] out the vector into which the found objects will be put
	 */
	virtual void getObjectsAt(const Point<GPType>& pt, std::vector<T*>& out) const = 0;

	/**
	 * Gets the location of this agent and puts it in the
	 * specified vector. The x coordinate will be the first value,
	 * the y the second and so on.
	 *
	 * @param agent the agent whose location we want to get
	 * @param [out] the vector where the agents location will be put
	 *
	 * @return true if the location was successfully found, otherwise false.
	 */
	virtual bool getLocation(const T* agent, std::vector<GPType>& out) const = 0;

	/**
	 * Gets the location of this agent and puts it in the
	 * specified vectors. The x coordinate will be the first value,
	 * the y the second and so on.
	 *
	 * @param id the id of the agent whose location we want to get
	 * @param [out] out the agent's location will be put into this vector
	 *
	 * @return true if the location was successfully found, otherwise false.
	 */
	virtual bool getLocation(const AgentId& id, std::vector<GPType>& out) const = 0;

	/**
	 * Gets vector difference between point 1 and point 2, putting the result
	 * in out.
	 *
	 * @param p1 the first point
	 * @param p2 the second point
	 * @param [out] the vector where the difference will be put
	 *
	 */
	virtual void getDisplacement(const Point<GPType>& pt1, const Point<GPType>& pt2, std::vector<GPType>& out) const = 0;

	/**
	 * Gets the distance between the two grid points.
	 *
	 * @param p1 the first point
	 * @param p2 the second point
	 *
	 * @return the distance between pt1 and pt2.
	 */
	virtual double getDistance(const Point<GPType>& pt1, const Point<GPType>& pt2) const = 0;

	/**
	 * Gets the square of the distance between the two grid points.
	 *
	 * @param p1 the first point
	 * @param p2 the second point
	 *
	 * @return the square of the distance between pt1 and pt2.
	 */
	virtual double getDistanceSq(const Point<GPType>& pt1, const Point<GPType>& pt2) const = 0;

	/**
	 * Translates the specified location by the specified displacement put the result in out.
	 *
	 * @param location the initial location
	 * @param displacement the amount to translate the location by
	 * @param [out] out the vector where the result of the translation is put
	 */
	virtual void
	translate(const Point<GPType>& location, const Point<GPType>& displacement, std::vector<GPType>& out) const = 0;

	/**
	 * Transforms the specified location using the properties (e.g. toroidal) of this space.
	 *
	 * @param location the location to transform
	 * @param [out] out the vector where the result of the transform will be put
	 */
	virtual void
	transform(const std::vector<GPType>& location, std::vector<GPType>& out) const = 0;

	/**
	 * Gets whether or not this grid is periodic (i.e. toroidal).
	 *
	 * @return true if this Grid is periodic, otherwise false.
	 */
	virtual bool isPeriodic() const = 0;

  virtual ProjectionInfoPacket* getProjectionInfo(AgentId id, bool secondaryInfo = false, std::set<AgentId>* secondaryIds = 0, int destProc = -1 ) = 0;

  virtual void updateProjectionInfo(ProjectionInfoPacket* pip, Context<T>* context) = 0;

  virtual void getRequiredAgents(std::set<AgentId>& agentsToTest, std::set<AgentId>& agentsRequired, RADIUS radius = Projection<T>::PRIMARY){} // Grids allow all agents to be dropped b/c agent info not dependent on other agents

  virtual void getAgentsToPush(std::set<AgentId>& agentsToTest, std::map<int, std::set<AgentId> >& agentsToPush) = 0;

  virtual bool keepsAgentsOnSyncProj(){ return false; }

  virtual bool sendsSecondaryAgentsOnStatusExchange(){ return false; }

  virtual void getInfoExchangePartners(std::set<int>& psToSendTo, std::set<int>& psToReceiveFrom) = 0;

  virtual void getAgentStatusExchangePartners(std::set<int>& psToSendTo, std::set<int>& psToReceiveFrom) = 0;

  virtual void cleanProjectionInfo(std::set<AgentId>& agentsToKeep){}; // Grids don't do this

};

}

#endif /* GRID_H_ */
