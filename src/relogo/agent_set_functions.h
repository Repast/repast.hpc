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
 *  agent_set_functions.h
 *
 *  Created on: Aug 12, 2010
 *      Author: nick
 */

#ifndef AGENT_SET_FUNCTIONS_H_
#define AGENT_SET_FUNCTIONS_H_

#include <boost/iterator/filter_iterator.hpp>
#include <vector>

#include "RelogoAgent.h"
#include "AgentSet.h"

namespace repast {
namespace relogo {

/**
 *  Unary function used in the transform_iterator that allows
 */
template<typename TargetType>
struct Caster2: public std::unary_function<RelogoAgent, TargetType*> {

	TargetType* operator()(const RelogoAgent* agent) const {
		RelogoAgent* agentRef = const_cast<RelogoAgent*> (agent);
		return static_cast<TargetType*> (agentRef);
	}
};

/**
 * Used to filter by agent type but ensure that only the
 * first encountered instance of agent is considered.
 */
template<typename T>
struct IsAgentTypeNoDup {
	IsAgentType<T> isAgentType;
	boost::unordered_set<AgentId, HashId > set;

	IsAgentTypeNoDup(int typeId) :
		isAgentType(typeId) {
	}

	bool operator()(const T* agent) {
		int count = set.count(agent->getId());
		if (count == 0) {
			set.insert(agent->getId());
			return isAgentType(agent);
		}
		return false;
	}
};



typedef boost::filter_iterator<IsAgentType<RelogoAgent> , std::vector<RelogoAgent*>::const_iterator>
		const_bytype_iterator;
typedef boost::filter_iterator<IsAgentTypeNoDup<RelogoAgent> , std::vector<RelogoAgent*>::const_iterator>
		const_bytype_nodup_iterator;
typedef boost::filter_iterator<IsAgentType<RelogoAgent> , std::vector<RelogoAgent*>::iterator> bytype_iterator;

/**
 * Filters a vector of RelogoAgents based on their type into out.
 *
 * @param in the vector to filter
 * @param out the AgentSet to put the filtered output into
 * @param typeId the id of the type to filter on
 * @tparam AgentType the type of the agents in the AgentSet
 */
template<typename AgentType>
void filterVecToSet(std::vector<RelogoAgent*>& in, AgentSet<AgentType>& out, int typeId) {
	//const_bytype_iterator typeFilterBegin(IsAgentType<RelogoAgent> (typeId), in.begin(), in.end());
	//const_bytype_iterator typeFilterEnd(IsAgentType<RelogoAgent> (typeId), in.end(), in.end());
	//boost::transform_iterator<Caster2<AgentType> , const_bytype_iterator> begin(typeFilterBegin);
	//boost::transform_iterator<Caster2<AgentType> , const_bytype_iterator> end(typeFilterEnd);
	for (int i = 0, n = in.size(); i < n; ++i) {
		RelogoAgent* agent = in[i];
		if (agent->getId().agentType() == typeId) {
			out.add(static_cast<AgentType*> (const_cast<RelogoAgent*>(agent)));
		}
	}
	//out.addAll(begin, end);
}


/**
 * Filters a vector of RelogoAgents based on their type into out, making sure that
 * there is only one instance of an agent in out.
 *
 * @param in the vector to filter
 * @param out the AgentSet to put the filtered output into
 * @param typeId the id of the type to filter on
 * @tparam AgentType the type of the agents in the AgentSet
 */
template<typename AgentType>
void filterVecToSetNoDuplicates(std::vector<RelogoAgent*>& in, AgentSet<AgentType>& out, int typeId) {
	const_bytype_nodup_iterator typeFilterBegin(IsAgentTypeNoDup<RelogoAgent> (typeId), in.begin(), in.end());
	const_bytype_nodup_iterator typeFilterEnd(IsAgentTypeNoDup<RelogoAgent> (typeId), in.end(), in.end());
	boost::transform_iterator<Caster2<AgentType> , const_bytype_nodup_iterator> begin(typeFilterBegin);
	boost::transform_iterator<Caster2<AgentType> , const_bytype_nodup_iterator> end(typeFilterEnd);
	out.addAll(begin, end);
}

}
}

#endif /* AGENT_SET_FUNCTIONS_H_ */
