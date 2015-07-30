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
 *  CartesianTopology.cpp
 *
 *  Created on: July 25, 2008
 *      Author: jtm
 */

#include "CartesianTopology.h"


using namespace std;

namespace repast {


CartesianTopology::CartesianTopology(vector<int> processesPerDim, bool spaceIsPeriodic, boost::mpi::communicator* comm) :
  periodic(spaceIsPeriodic), procsPerDim(processesPerDim) {
  int numDims = procsPerDim.size();
  int* periods = new int[numDims];
  int periodicFlag = periodic ? 1 : 0;
  for (int i = 0; i < numDims; i++) periods[i] = periodicFlag;

  MPI_Cart_create(*comm, numDims, &processesPerDim[0], periods, 0, &topologyComm);
  delete[] periods;
}

CartesianTopology::~CartesianTopology(){}

int CartesianTopology::getRank(vector<int>& loc, std::vector<int>& relLoc) {
  int numDims = relLoc.size();
  int* coord = new int[numDims];
  for(int i = 0; i < numDims; i++){
    coord[i] = loc[i] + relLoc[i];
    if(!periodic){
      if((coord[i] < 0) || (coord[i] > (procsPerDim[i] - 1))){
        delete[] coord;
        return MPI_PROC_NULL;
      }
    }
  }
  int rank;
  MPI_Cart_rank(topologyComm, coord, &rank);
  delete[] coord;
  return rank;
}


void CartesianTopology::getCoordinates(int rank, std::vector<int>& coords) {
  int numDims = procsPerDim.size();
  MPI_Cart_coords(topologyComm, rank, numDims, &coords[0]);
}

GridDimensions CartesianTopology::getDimensions(int rank, GridDimensions globalBoundaries) {
  vector<int> coords;
  getCoordinates(rank, coords);
  return getDimensions(coords, globalBoundaries);
}

GridDimensions CartesianTopology::getDimensions(vector<int>& pCoordinates, GridDimensions globalBoundaries) {
  vector<double> origins, extents;
  for (size_t i = 0; i < pCoordinates.size(); i++) {
    double lower = globalBoundaries.origin(i) + ( (double)pCoordinates[i]      / (double)procsPerDim[i]) * globalBoundaries.extents(i);
    double upper = globalBoundaries.origin(i) + (((double)pCoordinates[i] + 1 )/ (double)procsPerDim[i]) * globalBoundaries.extents(i);
    origins.push_back(lower);
    extents.push_back(upper - lower);
  }

  return GridDimensions(Point<double> (origins), Point<double> (extents));
}

RelativeLocation CartesianTopology::trim(int rank, RelativeLocation volume){
  if( periodic ||
      volume.getCountOfDimensions() != procsPerDim.size()) return RelativeLocation(volume);
  int numDims = volume.getCountOfDimensions();
  vector<int> loc;
  loc.assign(procsPerDim.size(), 0);
  getCoordinates(rank, loc);
  RelativeLocation test(volume); // Note: sets to minima
  vector<int>* min = 0;
  vector<int>* max = 0;
  do{
    vector<int> relLoc(test.getCurrentValue());
    int rankFound = getRank(loc, relLoc);
    if(rankFound != MPI_PROC_NULL){
      if(min == 0){
        min = new vector<int>();
        for(size_t i = 0; i < procsPerDim.size(); i++) min->push_back(test[i]);
      }
      if(max == 0){
        max = new vector<int>();
        for(size_t i = 0; i < procsPerDim.size(); i++) max->push_back(test[i]);
      }
      for(size_t i = 0; i < procsPerDim.size(); i++){
        if(min->at(i) > test[i]) (*min)[i] = test[i]; // In practice this should never happen
        if(max->at(i) < test[i]) (*max)[i] = test[i]; // But this will
      }
    }
  }while(test.increment());
  RelativeLocation ret(*min, *max);
  delete min;
  delete max;
  return ret;
}

}
