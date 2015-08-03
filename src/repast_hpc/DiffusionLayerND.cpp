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
 *  DiffusionLayerND.cpp
 *
 *  Created on: July 25, 2008
 *      Author: jtm
 */

#include "DiffusionLayerND.h"
#include "RepastProcess.h"
#include <boost/mpi.hpp>

using namespace std;

namespace repast {

DiffusionLayerND::DiffusionLayerND(vector<int> processesPerDim, GridDimensions globalBoundaries, int bufferSize, bool periodic, double initialValue): globalSpaceIsPeriodic(periodic){
  CartesianTopology* cartTopology = RepastProcess::instance()->getCartesianTopology(processesPerDim, periodic);
  int rank = RepastProcess::instance()->rank();
  GridDimensions localBoundaries = cartTopology->getDimensions(rank, globalBoundaries);

  // Calculate the size to be used for the buffers
  numDims = processesPerDim.size();
  RelativeLocation relLoc(numDims);
  RelativeLocation relLocTrimmed = cartTopology->trim(rank, relLoc); // Initialized to minima

  vector<int> minima = relLocTrimmed.getCurrentValue();
  while(relLocTrimmed.increment());
  vector<int> maxima = relLocTrimmed.getCurrentValue();

  vector<int> outerLowerBounds;
  vector<int> localLowerBounds;
  vector<int> innerLowerBounds;
  vector<int> innerUpperBounds;
  vector<int> localUpperBounds;
  vector<int> outerUpperBounds;
  for(int i = 0; i < numDims; i++){
    outerLowerBounds.push_back(localBoundaries.origin(i) + minima[i] * bufferSize); // Note: minima and maxima will be -1 or 0
    localLowerBounds.push_back(localBoundaries.origin(i));
    innerLowerBounds.push_back(localBoundaries.origin(i) + bufferSize);
    innerUpperBounds.push_back(localBoundaries.origin(i) + localBoundaries.extents(i) - bufferSize);
    localUpperBounds.push_back(localBoundaries.origin(i) + localBoundaries.extents(i));
    outerUpperBounds.push_back(localBoundaries.origin(i) + localBoundaries.extents(i) + maxima * bufferSize);
  }

  vector<int> receiveWidths; // The whole width of the data array
  vector<int> sendWidths;




  initialize(initialValue);

}

DiffusionLayerND::~DiffusionLayerND(){
  delete[] currentDataSpace;
  delete[] otherDataSpace;
}


void DiffusionLayerND::setPlaces(){
  places.clear();
  int val = 1;
  for(int i = 0; i < numDims; i++){
    places.push_back(val);
    val *= (int)(floor(overallGridDimensions.extents(i) + .1));
  }
}

void DiffusionLayerND::initialize(double initialValue){
  for(int i = 0; i < length; i++){ // TODO Optimize
    dataSpace1[i] = initialValue;
    dataSpace2[i] = initialValue;
  }
}

void DiffusionLayerND::diffuse(){



  // Switch the data banks
  double* tempDataSpace    = currentDataSpace;
  currentDataSpace = otherDataSpace;
  otherDataSpace   = tempDataSpace;
}


vector<int> DiffusionLayerND::transform(vector<int> location){


}

int DiffusionLayerND::getIndex(vector<int> location){
  int val = 0;
  for(int i = numDims - 1; i >= 0; i--){
    val += location[i] * places[i];
  }
}

}
