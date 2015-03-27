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
 *  WorldCreator.cpp
 *
 *  Created on: Aug 5, 2010
 *      Author: nick
 */
#include "WorldCreator.h"
#include "utility.h"

#include <boost/mpi/communicator.hpp>

namespace repast {
namespace relogo {

repast::Projection<RelogoAgent>*  WorldCreator::createDiscreteSpace(const WorldDefinition& def, const std::vector<int>& pConfiguration) const {
	repast::Projection<RelogoAgent>* proj = 0;
	if (def.isWrapped()) {
		proj = new ToroidalDiscreteSpace(GRID_NAME, def.dimensions(), pConfiguration, def.buffer(), comm);
	} else {
		proj = new BoundedDiscreteSpace(GRID_NAME, def.dimensions(), pConfiguration, def.buffer(), comm);
	}
	return proj;

}

repast::Projection<RelogoAgent>*  WorldCreator::createContinuousSpace(const WorldDefinition& def, const std::vector<int>& pConfiguration) const {
	repast::Projection<RelogoAgent>* proj = 0;
  GridDimensions originalDimensions = def.dimensions();
  std::vector<double> new_origins = adjustOrigin(originalDimensions.origin().coords());
  GridDimensions newDimensions(new_origins, def.dimensions().extents().coords());
  if (def.isWrapped()) {
    proj = new ToroidalContinuousSpace(SPACE_NAME, newDimensions, pConfiguration, def.buffer(), comm);
  } else {
    proj = new BoundedContinuousSpace(SPACE_NAME, newDimensions, pConfiguration, def.buffer(), comm);
  }
  return proj;
}

}
}
