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
 *  	 Redistributions of source code must retain the above copyright notice,
 *  	 this list of conditions and the following disclaimer.
 *
 *  	 Redistributions in binary form must reproduce the above copyright notice,
 *  	 this list of conditions and the following disclaimer in the documentation
 *  	 and/or other materials provided with the distribution.
 *
 *  	 Neither the name of the Argonne National Laboratory nor the names of its
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
 *  AgentId.cpp
 *
 *  Created on: Dec 23, 2008
 *      Author: nick
 */

#include "AgentId.h"

#include <boost/functional/hash.hpp>

using namespace std;

namespace repast {

AgentId::AgentId(int id, int startProc, int agentType, int currentProc) : id_(id), startProc_(startProc),
	agentType_(agentType), currentProc_( (currentProc == -1 ? startProc : currentProc) ) {
	hash = 17;
	hash = 31 * hash + boost::hash_value(id_);
	hash = 31 * hash + boost::hash_value(startProc_);
	hash = 31 * hash + boost::hash_value(agentType_);
}

bool operator==(const AgentId &one, const AgentId &two) {
	return one.id_ == two.id_ && one.startProc_ == two.startProc_ && one.agentType_ == two.agentType_;
}

bool operator!=(const AgentId &one, const AgentId &two) {
	return !(one == two);
}

bool operator<(const AgentId &one, const AgentId &two) {
	return ((one.agentType_ < two.agentType_) ||
			((one.agentType_ == two.agentType_) && (one.startProc_ < two.startProc_)) ||
			((one.agentType_ == two.agentType_) && (one.startProc_ == two.startProc_) && (one.id_ < two.id_)));
}
	
	
AgentId::~AgentId() {}

ostream& operator<<(ostream& os, const AgentId& id) {
	os << "AgentId(" << id.id_ << ", " << id.startProc_ << ", " << id.agentType_
	<< ", " << id.currentProc_ << ")";
	return os;
}

}
