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
 *  RelogoSharedContinuousSpace.h
 *
 *  Created on: Sep 7, 2010
 *      Author: nick
 */

#ifndef RELOGOSHAREDCONTINUOUSSPACE_H_
#define RELOGOSHAREDCONTINUOUSSPACE_H_

#include <boost/mpi/communicator.hpp>

#include "repast_hpc/SharedContinuousSpace.h"

namespace repast {
namespace relogo {

/**
 * Repast SharedContinuousSpace specialized for Relogo. This overrides synchMoveTo.
 */
template<typename GPTransformer, typename Adder>
class RelogoSharedContinuousSpace: public repast::SharedContinuousSpace<RelogoAgent, GPTransformer, Adder> {

protected:
	void synchMoveTo(const repast::AgentId& id, const repast::Point<double>& pt);

public:
	virtual ~RelogoSharedContinuousSpace() {
	}
	RelogoSharedContinuousSpace(std::string name, repast::GridDimensions gridDims, std::vector<int> processDims, int buffer, boost::mpi::communicator* comm);
};

template<typename GPTransformer, typename Adder>
RelogoSharedContinuousSpace<GPTransformer, Adder>::RelogoSharedContinuousSpace(std::string name, repast::GridDimensions gridDims,
		std::vector<int> processDims, int buffer, boost::mpi::communicator* comm) :
	repast::SharedContinuousSpace<RelogoAgent, GPTransformer, Adder>(name, gridDims, processDims, buffer, comm) {
}

template<typename GPTransformer, typename Adder>
void RelogoSharedContinuousSpace<GPTransformer, Adder>::synchMoveTo(const repast::AgentId& id, const repast::Point<double>& pt) {
	RelogoAgent* agent = repast::SharedBaseGrid<RelogoAgent, GPTransformer, Adder, double>::GridBaseType::get(id);
	if (agent != 0) {
		agent->_location = pt;
		repast::SharedBaseGrid<RelogoAgent, GPTransformer, Adder, double>::GridBaseType::moveTo(id, pt.coords());
	}
}

}
}

#endif /* RELOGOSHAREDCONTINUOUSSPACE_H_ */
