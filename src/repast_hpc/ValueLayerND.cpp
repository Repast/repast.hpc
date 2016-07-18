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
 *  ValueLayerND.cpp
 *
 *  Created on: July 18, 2016
 *      Author: jtm
 */
#include "ValueLayerND.h"
#include "RepastProcess.h"
#include "Point.h"

#include <boost/mpi.hpp>

using namespace std;

namespace repast {


DimensionDatum::DimensionDatum(int indx, GridDimensions globalBoundaries, GridDimensions localBoundaries, int buffer, bool isPeriodic):
    leftBufferSize(buffer), rightBufferSize(buffer), periodic(isPeriodic){
  globalCoordinateMin = globalBoundaries.origin(indx);
  globalCoordinateMax = globalBoundaries.origin(indx) + globalBoundaries.extents(indx);
  localBoundariesMin  = localBoundaries.origin(indx);
  localBoundariesMax  = localBoundaries.origin(indx) + localBoundaries.extents(indx);

  atLeftBound  = localBoundariesMin == globalCoordinateMin;
  atRightBound = localBoundariesMax == globalCoordinateMax;

  spaceContinuesLeft  = !atLeftBound  || periodic;
  spaceContinuesRight = !atRightBound || periodic;

  simplifiedBoundariesMin  = localBoundariesMin - leftBufferSize;
  simplifiedBoundariesMax  = localBoundariesMax + rightBufferSize;

  matchingCoordinateMin    = localBoundariesMin;
  if(spaceContinuesLeft  && !atLeftBound ) matchingCoordinateMin -= leftBufferSize;

  matchingCoordinateMax    = localBoundariesMax;
  if(spaceContinuesRight && !atRightBound) matchingCoordinateMax += rightBufferSize;

  globalWidth = globalCoordinateMax - globalCoordinateMin;
  localWidth = localBoundariesMax - localBoundariesMin;
  width = leftBufferSize + localWidth + rightBufferSize;
  widthInBytes = width * (sizeof(double));
}

int DimensionDatum::getSendReceiveSize(int relativeLocation){
  switch(relativeLocation){
    case -1:  return leftBufferSize;
    case  1:  return rightBufferSize;
    case  0:
    default:
      return localWidth;
  }
}

int DimensionDatum::getTransformedCoord(int originalCoord){
  if(originalCoord < matchingCoordinateMin){        // Assume (!) original is on right (!) side of periodic boundary, starting at some value
    return matchingCoordinateMax + (originalCoord - globalCoordinateMin);
  }
  else if(originalCoord > matchingCoordinateMax){
    return matchingCoordinateMin - (globalCoordinateMax - originalCoord);
  }
  else return originalCoord; // Within matching boundaries; no need to transform

}


int DimensionDatum::getIndexedCoord(int originalCoord, bool isSimplified){
  return (isSimplified ? originalCoord : getTransformedCoord(originalCoord)) - simplifiedBoundariesMin;
}

bool DimensionDatum::isInLocalBounds(int originalCoord){
  return originalCoord >= localBoundariesMin && originalCoord < localBoundariesMax;
}


}

