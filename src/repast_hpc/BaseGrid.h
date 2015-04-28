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
 *  Created on: Jun 19, 2009
 *      Author: nick
 */

#ifndef BASE_GRID_H_
#define BASE_GRID_H_

#include <vector>
#include <iostream>
#include <math.h>
#include <exception>

#include <boost/unordered_map.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/iterator.hpp>
#include <boost/iterator/transform_iterator.hpp>

#include "Grid.h"
#include "spatial_math.h"
#include "RepastErrors.h"

namespace repast {

/**
 * Encapsulates a grid point and what is held in it.
 */
template<typename T, typename GPType>
struct GridPointHolder {

	bool inGrid;
	Point<GPType> point;
	boost::shared_ptr<T> ptr;

	GridPointHolder() :
		inGrid(false), point(0) {
	}
};

/**
 *  Unary function used in the transform_iterator that allows context iterators
 *  to return the agent maps values.
 */
template<typename T, typename GPType>
struct AgentFromGridPoint: public std::unary_function<typename boost::unordered_map<AgentId,
		GridPointHolder<T, GPType>*>::value_type, boost::shared_ptr<T> > {
	boost::shared_ptr<T> operator()(
			const typename boost::unordered_map<AgentId, GridPointHolder<T, GPType>*>::value_type& value) const {
		GridPointHolder<T, GPType> *gp = value.second;
		return gp->ptr;
	}

};

/**
 * Base grid implementation, implementing elements common to both Grids and ContinuousSpaces.
 * Standard grid and space types that provide defaults for the various template parameters
 * can be found in Space in Space.h
 *
 * @tparam T the type of objects contained by this BaseGrid (generally the type of agents)
 * @tparam CellAccessor implements the actual storage for the grid.
 * @tparam GPTransformer transforms cell points according to the topology (e.g. periodic)
 * of the BaseGrid.
 * @tparam Adder determines how objects are added to the grid from its associated context.
 * @tparam GPType the coordinate type of the grid point locations; this must
 * be an int or a double.
 */
template<typename T, typename CellAccessor, typename GPTransformer, typename Adder, typename GPType>
class BaseGrid: public Grid<T, GPType> {

private:

	// we use a GridPointHolder so we can swap out the GridPoint for an
	// agent with a single map access, rather than have to put the new
	// GridPoint back in the map.
	typedef typename boost::unordered_map<AgentId, GridPointHolder<T, GPType>*, HashId> AgentLocationMap;

	AgentLocationMap agentToLocation;
	GridDimensions dimensions_;

	CellAccessor cellAccessor;

	bool doMove(std::vector<GPType>& point, GridPointHolder<T, GPType>* gpHolder);

	size_t size_;

protected:

	typedef typename AgentLocationMap::iterator LocationMapIter;
	typedef typename AgentLocationMap::const_iterator LocationMapConstIter;

	GPTransformer gpTransformer;
	Adder adder;

	virtual bool addAgent(boost::shared_ptr<T> agent);
	virtual void removeAgent(T* agent);

	LocationMapConstIter locationsBegin() const {
		return agentToLocation.begin();
	}
	LocationMapConstIter locationsEnd() const {
		return agentToLocation.end();
	}

	T* get(const AgentId& id);

public:

	/**
	 * A const iterator over shared_ptr<T>.
	 */
	typedef typename boost::transform_iterator<AgentFromGridPoint<T, GPType> , LocationMapConstIter> const_iterator;

	/**
	 * Creates a BaseGrid with the specified name and dimensions.
	 *
	 * @param name the name of the BaseGrid
	 * @param dimensions the dimensions of the BaseGrid
	 */
	BaseGrid(std::string name, GridDimensions dimensions);
	virtual ~BaseGrid();

	// doc inherited from Grid
	virtual bool contains(const AgentId& id);

	// doc inherited from Grid
	virtual bool getLocation(const T* agent, std::vector<GPType>& pt) const;

	// doc inherited from Grid
	virtual bool getLocation(const AgentId& id, std::vector<GPType>& out) const;

	// doc inherited from Grid
	virtual T* getObjectAt(const Point<GPType>& pt) const;

	// doc inherited from Grid
	virtual void getObjectsAt(const Point<GPType>& pt, std::vector<T*>& out) const;

	/**
	 * Moves the specified agent to the specified location. Returns
	 * true if the move was successful otherwise false. The agent
	 * must be already added to the context associated with this space, otherwise
	 * this throws an out_of_range exception if the new location out of bounds.
	 *
	 * @param agent the agent to move
	 * @param newLocation the location to move to
	 *
	 * @return true if the move was successful, otherwise false
	 */
	virtual bool moveTo(const T* agent, const std::vector<GPType>& newLocation);

	/**
	 * Moves the specified agent to the specified location. Returns
	 * true if the move was successful otherwise false. The agent
	 * must be already added to the context associated with this space, otherwise
	 * this throws an out_of_range exception if the new location out of bounds.
	 *
	 * @param agent the agent to move
	 * @param newLocation the location to move to
	 *
	 * @return true if the move was successful, otherwise false
	 */
	virtual bool moveTo(const T* agent, const Point<GPType>& newLocation);

	/**
	 * Moves the specified agent to the specified location. Returns
	 * true if the move was successful otherwise false. The agent
	 * must be already added to the context associated with this space, otherwise
	 * this throws an out_of_range exception if the new location out of bounds.
	 *
	 * @param id the id of the agent to move
	 * @param newLocation the location to move to
	 *
	 * @return true if the move was successful, otherwise false
	 */
	virtual bool moveTo(const AgentId& id, const std::vector<GPType>& newLocation);

	// doc inherited from Grid
	virtual bool moveTo(const AgentId& id, const Point<GPType>& pt);

	// doc inherited from Grid
	virtual std::pair<bool, Point<GPType> > moveByDisplacement(const T* agent, const std::vector<GPType>& displacement);

	/// doc inherited from Grid
	virtual std::pair<bool, Point<GPType> > moveByVector(const T* agent, double distance,
			const std::vector<double>& anglesInRadians);

	/**
	 * Gets an iterator over the agents in this BaseGrid starting with the
	 * first agent. The iterator derefrences into shared_ptr<T>. The actual
	 * agent can be accessed by derefrenceing the iter: (*iter)->getId() for example.
	 *
	 * @return an iterator over the agents in this BaseGrid starting with the
	 * first agent.
	 */
	virtual const_iterator begin() const {
		return const_iterator(agentToLocation.begin());
	}

	/**
	 * Gets the end of an iterator over the agents in this BaseGrid.
	 *
	 * @return  the end of an iterator over the agents in this BaseGrid.
	 */
	virtual const_iterator end() const {
		return const_iterator(agentToLocation.end());
	}

	/**
	 * Gets the number of agents in this BaseGrid.
	 *
	 * @return the number of agents in this BaseGrid.
	 */
	virtual size_t size() const {
		return size_;
	}

	// doc inherited from Grid
	virtual double getDistance(const Point<GPType>& pt1, const Point<GPType>& pt2) const;

	// doc inherited from Grid
	virtual double getDistanceSq(const Point<GPType>& pt1, const Point<GPType>& pt2) const;

	// doc inherited from Grid
	virtual void getDisplacement(const Point<GPType>& pt1, const Point<GPType>& pt2, std::vector<GPType>& out) const;

	// doc inherited from Grid
	virtual const GridDimensions dimensions() const {
		return dimensions_;
	}

	// doc inherited from Grid
	virtual void translate(const Point<GPType>& location, const Point<GPType>& displacement, std::vector<GPType>& out) const {
		gpTransformer.translate(location.coords(), out, displacement.coords());
	}

	// doc inherited from Grid
	virtual void transform(const std::vector<GPType>& location, std::vector<GPType>& out) const {
		gpTransformer.transform(location, out);
	}

	// doc inherited from Grid
	virtual bool isPeriodic() const {
		return gpTransformer.isPeriodic();
	}

  virtual ProjectionInfoPacket* getProjectionInfo(AgentId id, bool secondaryInfo = false, std::set<AgentId>* secondaryIds = 0, int destProc = -1 );

  virtual void updateProjectionInfo(ProjectionInfoPacket* pip, Context<T>* context);

  virtual void getAgentsToPush(std::set<AgentId>& agentsToTest, std::map<int, std::set<AgentId> >& agentsToPush){ }
  virtual void getInfoExchangePartners(std::set<int>& psToSendTo, std::set<int>& psToReceiveFrom) {}
  virtual void getAgentStatusExchangePartners(std::set<int>& psToSendTo, std::set<int>& psToReceiveFrom) {}
};

template<typename T, typename CellAccessor, typename GPTransformer, typename Adder, typename GPType>
BaseGrid<T, CellAccessor, GPTransformer, Adder, GPType>::BaseGrid(std::string name, GridDimensions dimensions) :
	Grid<T, GPType> (name), gpTransformer(dimensions), dimensions_(dimensions), size_(0) {
//	gpTransformer.init(dimensions);
	adder.init(dimensions, this);
}

template<typename T, typename CellAccessor, typename GPTransformer, typename Adder, typename GPType>
BaseGrid<T, CellAccessor, GPTransformer, Adder, GPType>::~BaseGrid() {
	for (LocationMapIter iter = agentToLocation.begin(); iter != agentToLocation.end(); ++iter) {
		delete iter->second;
	}
}

template<typename T, typename CellAccessor, typename GPTransformer, typename Adder, typename GPType>
T* BaseGrid<T, CellAccessor, GPTransformer, Adder, GPType>::get(const AgentId& id) {
	LocationMapConstIter iter = agentToLocation.find(id);
	if (iter == agentToLocation.end())
		return 0;

	return iter->second->ptr.get();
}

template<typename T, typename CellAccessor, typename GPTransformer, typename Adder, typename GPType>
bool BaseGrid<T, CellAccessor, GPTransformer, Adder, GPType>::getLocation(const AgentId& id, std::vector<GPType>& out) const {
	LocationMapConstIter iter = agentToLocation.find(id);
	if (iter == agentToLocation.end())
		return false;

	GridPointHolder<T, GPType>* holder = iter->second;
	if (!holder->inGrid)
		return false;

	if (out.size() != dimensions_.dimensionCount())
		out.resize(dimensions_.dimensionCount(), 0);
	holder->point.copy(out);
	return true;

}

template<typename T, typename CellAccessor, typename GPTransformer, typename Adder, typename GPType>
bool BaseGrid<T, CellAccessor, GPTransformer, Adder, GPType>::getLocation(const T* agent, std::vector<GPType>& out) const {
	return getLocation(agent->getId(), out);
}

template<typename T, typename CellAccessor, typename GPTransformer, typename Adder, typename GPType>
bool BaseGrid<T, CellAccessor, GPTransformer, Adder, GPType>::contains(const AgentId& id) {
	return agentToLocation.find(id) != agentToLocation.end();
}

template<typename T, typename CellAccessor, typename GPTransformer, typename Adder, typename GPType>
bool BaseGrid<T, CellAccessor, GPTransformer, Adder, GPType>::moveTo(const T* agent, const Point<GPType>& newLocation) {
	return moveTo(agent->getId(), newLocation.coords());
}

template<typename T, typename CellAccessor, typename GPTransformer, typename Adder, typename GPType>
bool BaseGrid<T, CellAccessor, GPTransformer, Adder, GPType>::moveTo(const T* agent,
		const std::vector<GPType>& newLocation) {
	return moveTo(agent->getId(), newLocation);
}

template<typename T, typename CellAccessor, typename GPTransformer, typename Adder, typename GPType>
bool BaseGrid<T, CellAccessor, GPTransformer, Adder, GPType>::moveTo(const AgentId& id,
		const Point<GPType>& newLocation) {
	return moveTo(id, newLocation.coords());
}

template<typename T, typename CellAccessor, typename GPTransformer, typename Adder, typename GPType>
bool BaseGrid<T, CellAccessor, GPTransformer, Adder, GPType>::moveTo(const AgentId& id,
		const std::vector<GPType>& newLocation) {
  LocationMapIter iter = agentToLocation.find(id);

  if (iter == agentToLocation.end())
		throw Repast_Error_2<AgentId>(id, Projection<T>::name()); // Agent has not yet been introduced to this space/is not present

	if (newLocation.size() < dimensions_.dimensionCount())
		throw Repast_Error_3(newLocation.size(), dimensions_.dimensionCount()); // Destination not fully specified

	std::vector<GPType> transformedCoords(newLocation.size(), 0);
	gpTransformer.transform(newLocation, transformedCoords);

	if (iter->second->point.coords() == transformedCoords)  return true;
	return doMove(transformedCoords, iter->second);
}

template<typename T, typename CellAccessor, typename GPTransformer, typename Adder, typename GPType>
bool BaseGrid<T, CellAccessor, GPTransformer, Adder, GPType>::doMove(std::vector<GPType>& location, GridPointHolder<T,
		GPType>* gpHolder) {
	Point<GPType> pt(location);
	if (cellAccessor.put(gpHolder->ptr, pt)) {
		if (gpHolder->inGrid) {
			cellAccessor.remove(gpHolder->ptr, gpHolder->point);
		} else {
			size_++;
			gpHolder->inGrid = true;
		}
		gpHolder->point = pt;
		return true;
	}
	return false;
}

template<typename T, typename CellAccessor, typename GPTransformer, typename Adder, typename GPType>
std::pair<bool, Point<GPType> > BaseGrid<T, CellAccessor, GPTransformer, Adder, GPType>::moveByVector(const T* agent,
		double distance, const std::vector<double>& anglesInRadians) {
	if (anglesInRadians.size() != dimensions_.dimensionCount())
		throw Repast_Error_4(anglesInRadians.size(), dimensions_.dimensionCount()); // Number of angles must equal dimensions

	std::vector<GPType> pt = calculateDisplacement<GPType> (dimensions_.dimensionCount(), 0, distance, anglesInRadians);
	return moveByDisplacement(agent, pt);
}

template<typename T, typename CellAccessor, typename GPTransformer, typename Adder, typename GPType>
std::pair<bool, Point<GPType> > BaseGrid<T, CellAccessor, GPTransformer, Adder, GPType>::moveByDisplacement(
		const T* agent, const std::vector<GPType>& displacement) {
	if (displacement.size() != dimensions_.dimensionCount())
		  throw Repast_Error_5(displacement.size(), dimensions_.dimensionCount()); // displacement vector must equal number of grid/space dimensions

	LocationMapIter iter = agentToLocation.find(agent->getId());
	if (iter == agentToLocation.end())
      throw Repast_Error_6<AgentId>(agent->getId(), Projection<T>::name()); // Agent has not in this grid / space

	GridPointHolder<T, GPType>* gpHolder = iter->second;
	std::vector<GPType> newPos(displacement.size(), 0);
	gpTransformer.translate(gpHolder->point.coords(), newPos, displacement);
	std::vector<GPType> transformedCoords(newPos.size(), 0);
	gpTransformer.transform(newPos, transformedCoords);
	if (iter->second->point.coords() == transformedCoords)
		return std::make_pair(true, Point<GPType> (transformedCoords));
	return std::make_pair(moveTo(agent->getId(), transformedCoords), Point<GPType> (transformedCoords));
}

template<typename T, typename CellAccessor, typename GPTransformer, typename Adder, typename GPType>
T* BaseGrid<T, CellAccessor, GPTransformer, Adder, GPType>::getObjectAt(const Point<GPType>& pt) const {
	return cellAccessor.get(pt);
}

template<typename T, typename CellAccessor, typename GPTransformer, typename Adder, typename GPType>
void BaseGrid<T, CellAccessor, GPTransformer, Adder, GPType>::getObjectsAt(const Point<GPType>& pt,
		std::vector<T*>& out) const {
	return cellAccessor.getAll(pt, out);
}

template<typename T, typename CellAccessor, typename GPTransformer, typename Adder, typename GPType>
bool BaseGrid<T, CellAccessor, GPTransformer, Adder, GPType>::addAgent(boost::shared_ptr<T> agent) {
  if(!Projection<T>::agentCanBeAdded(agent)) return false;
  GridPointHolder<T, GPType>* gp = new GridPointHolder<T, GPType> ();
  gp->ptr = agent;
  agentToLocation[agent->getId()] = gp;
  return adder.add(agent);
}

template<typename T, typename CellAccessor, typename GPTransformer, typename Adder, typename GPType>
void BaseGrid<T, CellAccessor, GPTransformer, Adder, GPType>::removeAgent(T* agent) {
	LocationMapIter iter = agentToLocation.find(agent->getId());
	if (iter != agentToLocation.end()) {
		GridPointHolder<T, GPType>* gp = iter->second;
		cellAccessor.remove(gp->ptr, gp->point);
		delete gp;
		agentToLocation.erase(iter);
	}
}

template<typename T, typename CellAccessor, typename GPTransformer, typename Adder, typename GPType>
double BaseGrid<T, CellAccessor, GPTransformer, Adder, GPType>::getDistance(const Point<GPType>& pt1, const Point<
		GPType>& pt2) const {
	return sqrt(getDistanceSq(pt1, pt2));
}

template<typename T, typename CellAccessor, typename GPTransformer, typename Adder, typename GPType>
double BaseGrid<T, CellAccessor, GPTransformer, Adder, GPType>::getDistanceSq(const Point<GPType>& pt1, const Point<
		GPType>& pt2) const {
	if (pt1.dimensionCount() != pt2.dimensionCount())
      throw Repast_Error_7(pt1.dimensionCount(), pt2.dimensionCount()); // Points do not have same number of dimensions

	double sum = 0;
	for (int i = 0, n = pt1.dimensionCount(); i < n; i++) {
		double diff = pt1.getCoordinate(i) - pt2.getCoordinate(i);
		if (gpTransformer.isPeriodic()) {
			double dim = dimensions_.extents(i);
			double absDiff = ((diff < 0.00) ? (-1.0 * diff) : diff); //abs(diff);
			if (absDiff > dim / 2.0)	diff = dim - absDiff;
		}
		sum += diff * diff;
	}

	return sum;
}

template<typename T, typename CellAccessor, typename GPTransformer, typename Adder, typename GPType>
void BaseGrid<T, CellAccessor, GPTransformer, Adder, GPType>::getDisplacement(const Point<GPType>& pt1, const Point<
		GPType>& pt2, std::vector<GPType>& out) const {
	if (pt1.dimensionCount() != pt2.dimensionCount())
      throw Repast_Error_8(pt1.dimensionCount(), pt2.dimensionCount()); // Points do not have same number of dimensions

	for (int i = 0, n = pt1.dimensionCount(); i < n; i++) {
		GPType diff = pt2.getCoordinate(i) - pt1.getCoordinate(i);
		if (gpTransformer.isPeriodic()) {
			double dim = dimensions_.extents(i);
			GPType absDiff = ((diff < 0.00) ? (-1.0 * diff) : diff); //abs(diff);
			if (absDiff > dim / ((GPType) 2))	diff = dim - absDiff;
		}
		out[i] = diff;
	}
}

template<typename T, typename CellAccessor, typename GPTransformer, typename Adder, typename GPType>
ProjectionInfoPacket* BaseGrid<T, CellAccessor, GPTransformer, Adder, GPType>::getProjectionInfo(AgentId id, bool secondaryInfo,
    std::set<AgentId>* secondaryIds, int destProc){
  typename AgentLocationMap::const_iterator agentIter = agentToLocation.find(id);
  if(agentIter == agentToLocation.end()){
      return 0;
  }
  else{
      return new SpecializedProjectionInfoPacket<GPType>(id, agentIter->second->point.coords());
  }
}


template<typename T, typename CellAccessor, typename GPTransformer, typename Adder, typename GPType>
void BaseGrid<T, CellAccessor, GPTransformer, Adder, GPType>::updateProjectionInfo(ProjectionInfoPacket* pip, Context<T>* context){
  SpecializedProjectionInfoPacket<GPType>* spip = static_cast<SpecializedProjectionInfoPacket<GPType>*>(pip);
  moveTo(spip->id, spip->data);
//  std::cout << " UPDATING PROJECTION INFO: " << spip->id << " " << spip->data[0] << "," << spip->data[1] << " " << context->getAgent(spip->id)->location() << std::endl;
}

}

#endif /* GRID_H_ */
