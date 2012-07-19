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
 *  Projection.h
 *
 *  Created on: Aug 5, 2010
 *      Author: nick
 */

#ifndef PROJECTION_H_
#define PROJECTION_H_

#include <sstream>
#include <string>
#include <set>
#include <map>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>

#include "AgentId.h"

namespace repast {

template<typename T>
class Context;

/**
 * Abstract base class for Projections.
 */
template<typename T>
class Projection : public boost::noncopyable {

	friend class Context<T> ;

protected:

	std::string name_;
	virtual bool addAgent(boost::shared_ptr<T> agent) = 0;
	virtual void removeAgent(T* agent) = 0;


	// Beta (Protected)
	std::set<int> filter;


public:


	/**
	 * Creates a projection with specified name.
	 *
	 * @param name the name of the projection. This must be unique
	 * across projections
	 */
	Projection(std::string name) :
		name_(name) {
	}

	virtual ~Projection() {
	}

	/**
	 * Gets the name of this projection.
	 */
	const std::string name() const {
		return name_;
	}

	// Beta (public)

	/**
	 * Adds an entry to the list of agent types that can be added to this projection.
	 *
	 * Note: no indication if type is already listed
	 *
	 * @param type type to be added
	 */
	void addFilterVal(int type){
	  filter.insert(type);
	}

	/**
	 * Removes an entry from the list of agent types that can be added to this projection.
	 *
	 * Note: no indication if type is not listed
	 *
	 * @param type entry to be removed
	 */
	void removeFilterVal(int type){
	  filter.erase(type);
	}

	/**
	 * Clears the list of agent types that can be added to this projection; the result
	 * is that the filter is empty, and any agent can be added.
	 */
	void clearFilter(){
	  filter.clear();
	}

	/**
	 * Returns true if the agent can be added to the projection, which will
	 * be the case if the filter list is empty or if the agent's type is in the
	 * filter list.
	 *
	 * @param agent pointer to the agent to be tested
	 */
	bool agentCanBeAdded(boost::shared_ptr<T> agent){
	  return ( (filter.size() == 0) ||
	           (filter.find(agent->getId().agentType()) != filter.end()));
	}
};

}

#endif /* PROJECTION_H_ */
