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
 *  RelogoDiscreteSpaceAdder.cpp
 *
 *  Created on: Aug 5, 2010
 *      Author: nick
 */

#include "RelogoDiscreteSpaceAdder.h"
#include "utility.h"


namespace repast {
namespace relogo {

RelogoDiscreteSpaceAdder::RelogoDiscreteSpaceAdder() :
	_grid(0), center(0, 0) {
}

void RelogoDiscreteSpaceAdder::init(GridDimensions dimensions, RelogoGridType* grid) {
  // we need to use the continuous space adjusted origins to calculate the center
  // because the turtle returns its discrete coordinates by a transform on its
  // continuous coords. Without this turtle.pxCor and pyCor won't match the
  // ones returned by the grid
  std::vector<double> new_origins = adjustOrigin(dimensions.origin().coords());
  GridDimensions newDims(new_origins, dimensions.extents().coords());
	int x = doubleCoordToInt(newDims.origin(0) + newDims.extents(0) / 2.0);
	int y = doubleCoordToInt(newDims.origin(1) + newDims.extents(1) / 2.0);
	center = Point<int> (x, y);
	_grid = grid;
}

bool RelogoDiscreteSpaceAdder::add(boost::shared_ptr<RelogoAgent> agent) {
  return  _grid->moveTo(agent->getId(), center);
}

}
}
