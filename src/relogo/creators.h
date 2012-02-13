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
 *  creators.h
 *
 *  Created on: Aug 5, 2010
 *      Author: nick
 */

#ifndef CREATORS_H_
#define CREATORS_H_

#include "RelogoLink.h"

namespace repast {
class AgentId;
}

namespace repast {
namespace relogo {

class Observer;
class RelogoAgent;

/**
 * operator() that creates an agent of type Agent. The type Agent must
 * have a constructor that takes an AgentId and pointer to an Observer.
 */
template<typename Agent>
struct DefaultAgentCreator {
	Agent* operator()(const repast::AgentId& id, Observer* obs);
};

template<typename Agent>
Agent* DefaultAgentCreator<Agent>::operator()(const repast::AgentId& id, Observer* obs) {
	return new Agent(id, obs);
}

/**
 * operator() that creates a RelogoLink from a source and target RelogoAgents.
 */
struct DefaultLinkCreator {
	RelogoLink* operator()(RelogoAgent* source, RelogoAgent* target);
};

}
}

#endif /* CREATORS_H_ */
