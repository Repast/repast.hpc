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
 *  MultipleOccupancy.h
 *
 *  Created on: Aug 11, 2010
 *      Author: nick
 */

#ifndef MULTIPLEOCCUPANCY_H_
#define MULTIPLEOCCUPANCY_H_

#include <boost/unordered_map.hpp>
#include <boost/shared_ptr.hpp>

#include "Point.h"

namespace repast {


/**
 * Unary function that allows retrieving the occupants of locations.
 */
template<typename T>
struct ExtractPtrs: public std::unary_function<
		typename boost::unordered_map<AgentId, boost::shared_ptr<T> >::value_type, T*> {
	T* operator()(typename boost::unordered_map<AgentId, boost::shared_ptr<T> >::value_type& val) {
		return val.second.get();
	}
};

/**
 * Multiple Occupancy cell accessor for accessing the occupants of locations
 * in a Grid. Each locations can have multiple occupants.
 *
 * @param T the type of object in the Grid
 * @param GPType the coordinate type of the grid point locations. This must
 * be an int or a double.
 */
template<typename T, typename GPType>
class MultipleOccupancy {

private:
	typedef typename boost::unordered_map<AgentId, boost::shared_ptr<T>, HashId> ValueType;
	typedef typename ValueType::iterator ValueTypeIter;
	typedef typename boost::unordered_map<Point<GPType> , ValueType*, HashGridPoint<GPType> > LocationMap;

	typedef typename LocationMap::iterator LocationMapIter;
	typedef typename LocationMap::const_iterator LocationMapConstIter;

	LocationMap locations;

	ValueType* doGet(const Point<GPType>& location) const;

public:

	virtual ~MultipleOccupancy();

	/**
	 * Gets the first object found at the specified location.
	 *
	 * @param location the location to get the object at
	 * @return the first object found at the specified location or 0 if there
	 * are no objects at the specified location.
	 */
	T* get(const Point<GPType>& location) const;

	/**
	 * Gets all the items found at the specified location.
	 *
	 * @param location the location to get the items at
	 * @param [out] the found items will be returned in this vector
	 */
	void getAll(const Point<GPType>& location, std::vector<T*>& out) const;

	/**
	 * Puts the specified item at the specified location.
	 *
	 * @param agent the item to put
	 * @param location the location to put the item at
	 */
	bool put(boost::shared_ptr<T>& agent, const Point<GPType>& location);

	/**
	 * Removes the specified item from the specified location.
	 *
	 * @param agent the item to remove
	 * @param location the location to remove the item from
	 */
	void remove(boost::shared_ptr<T>& agent, const Point<GPType>& location);

};

template<typename T, typename GPType>
MultipleOccupancy<T, GPType>::~MultipleOccupancy() {
	for (LocationMapIter iter = locations.begin(); iter != locations.end(); ++iter) {
		delete iter->second;
	}
}

template<typename T, typename GPType>
typename MultipleOccupancy<T, GPType>::ValueType* MultipleOccupancy<T, GPType>::doGet(const Point<GPType>& location) const {
	LocationMapConstIter iter = locations.find(location);
	if (iter == locations.end())
		return NULL;
	return iter->second;
}

template<typename T, typename GPType>
T* MultipleOccupancy<T, GPType>::get(const Point<GPType>& location) const {
	ValueType* ptrs = doGet(location);
	if (ptrs == NULL)
		return NULL;
	return ptrs->begin()->second.get();
}

template<typename T, typename GPType>
void MultipleOccupancy<T, GPType>::getAll(const Point<GPType>& location, std::vector<T*>& out) const {
  ValueType* ptrs = doGet(location);
	if (ptrs != NULL) {
		int index = out.size();
		out.resize(out.size() + ptrs->size(), 0);
		ExtractPtrs<T> func;
		std::transform(ptrs->begin(), ptrs->end(), out.begin() + index, func);
	}
}

template<typename T, typename GPType>
bool MultipleOccupancy<T, GPType>::put(boost::shared_ptr<T>& agent, const Point<GPType>& location) {
	LocationMapIter iter = locations.find(location);
	ValueType* vec;
	if (iter == locations.end()) {
		vec = new ValueType();
		locations[location] = vec;
	} else {
		vec = iter->second;
	}
	vec->insert(std::make_pair(agent->getId(), agent));

	return true;
}

template<typename T, typename GPType>
void MultipleOccupancy<T, GPType>::remove(boost::shared_ptr<T>& agent, const Point<GPType>& location) {
	LocationMapIter iter = locations.find(location);
	if (iter != locations.end()) {
		ValueType* vec = iter->second;
		ValueTypeIter agentIter = vec->find(agent->getId());

		if (agentIter != vec->end()) {
			vec->erase(agentIter);
			if (vec->size() == 0) {
				delete vec;
				locations.erase(iter);
			}

		}

	}
}

}

#endif /* MULTIPLEOCCUPANCY_H_ */
