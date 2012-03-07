/*
 *Repast for High Performance Computing (Repast HPC)
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
 * AgentStatus.h
 *
 *  Created on: Aug 19, 2010
 *      Author: nick
 */

#ifndef AGENTSTATUS_H_
#define AGENTSTATUS_H_

#include <boost/serialization/access.hpp>
#include "AgentId.h"

namespace repast {

/**
 * Encapsulates the status (moved or removed) of agent in order to synchronize that
 * status across processes.
 */
class AgentStatus {

  friend bool operator<(const AgentStatus &one, const AgentStatus &two);

public:

	/**
	 * Enum indicating the status of th agent.
	 */
	enum Status {
		REMOVED, MOVED
	};

	/**
	 * No-arg constructor for serialization.
	 */
	AgentStatus() {
	}

	/**
	 * Creates an AgentStatus indicating the status for the specified agent.
	 *
	 * @param id the id of the agent whose status this represents
	 */
	AgentStatus(AgentId id);

	/**
	 * Creates an AgentStatus indicating the status for the specified agent
	 * and the new id of that agent as result from the change in status. When
	 * an agent moves between processes its current rank may change and thus
	 * the current rank part of its id will change.
	 *
	 * @param old the id of the agent whose status this represents
	 * @param newId the new id of the agent that results from its status change
	 */
	AgentStatus(AgentId old, AgentId newId);

	/**
	 * Gets the status.
	 *
	 * @return the status
	 */
	Status getStatus() const {
		return _status;
	}

	/**
	 * Gets the id of the agent that this is the status for.
	 *
	 * @return  the id of the agent that this is the status for.
	 */
	const AgentId& getId() const {
		return _oldId;
	}

	/**
	 * Gets the old id of the agent that this is the status for, if
	 * this contains an old and updated AgentId.
	 *
	 * @return Gets the old id of the agent that this is the status for, if
	 * this contains an old and updated AgentId.
	 */
	const AgentId& getOldId() const {
		return _oldId;
	}

	/**
	 * Gets the new updated id of the agent that this is the status for, if
	 * this contains an old and updated AgentId.
	 *
	 * @return Gets the new id of the agent that this is the status for, if
	 * this contains an old and updated AgentId.
	 */
	const AgentId& getNewId() const {
		return _newId;
	}

private:
	friend class boost::serialization::access;

	AgentId _oldId, _newId;
	Status _status;

	template<class Archive>
	void serialize(Archive& ar, const unsigned int version) {
		ar & _oldId;
		ar & _newId;
		ar & _status;
	}
};

/**
 * Comparison operator that can be used in sorts, etc.
 */
bool operator<(const AgentStatus &one, const AgentStatus &two);
	
	
}

#endif /* AGENTSTATUS_H_ */
