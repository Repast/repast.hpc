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
 *  SharedGrid.cpp
 *
 *  Created on: Sep 11, 2009
 *      Author: nick
 */

#include "SharedBaseGrid.h"

#include <vector>
#include <boost/mpi.hpp>

using namespace std;

namespace repast {


CartTopology::CartTopology(vector<int> processesPerDim, vector<double> origin, vector<double> extents, bool spaceIsPeriodic, boost::mpi::communicator* comm) :
  periodic(spaceIsPeriodic), procsPerDim(processesPerDim) {
  int numDims = procsPerDim.size();
  int* periods = new int[numDims];
  int periodicFlag = periodic ? 1 : 0;
  for (int i = 0; i < numDims; i++) periods[i] = periodicFlag;

	// swap xy so row major

	MPI_Cart_create(*comm, numDims, &processesPerDim[0], periods, 0, &topologyComm);
	delete[] periods;
}

void CartTopology::getCoordinates(int rank, std::vector<int>& coords, GridDimensions globalBoundaries) {
	coords.assign(globalBoundaries.dimensionCount(), 0);
	MPI_Cart_coords(topologyComm, rank, globalBoundaries.dimensionCount(), &coords[0]);
}

GridDimensions CartTopology::getDimensions(int rank, GridDimensions globalBoundaries) {
	vector<int> coords;
	getCoordinates(rank, coords, globalBoundaries);
	return getDimensions(coords, globalBoundaries);
}

GridDimensions CartTopology::getDimensions(vector<int>& pCoordinates, GridDimensions globalBoundaries) {
  vector<double> origins, extents;
  for (size_t i = 0; i < pCoordinates.size(); i++) {
    double lower = globalBoundaries.origin(i) + ((double)pCoordinates[i] / (double)procsPerDim[i]) * globalBoundaries.extents(i);
    double upper = globalBoundaries.origin(i) + (((double)pCoordinates[i] + 1 )/ (double)procsPerDim[i]) * globalBoundaries.extents(i);
    origins.push_back(lower);
    extents.push_back(upper - lower);
  }

	return GridDimensions(Point<double> (origins), Point<double> (extents));
}

int CartTopology::getRank(vector<int>& loc, std::vector<int>& relLoc) {
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

void CartTopology::createNeighbors(Neighbors* nghs, GridDimensions globalBoundaries) {
  int numDims = procsPerDim.size();
  std::vector<int> relativeLocation;
  relativeLocation.assign(numDims, -1);

  int myRank = RepastProcess::instance()->rank();
  vector<int> myMPICoordinates;
  getCoordinates(myRank, myMPICoordinates, globalBoundaries);
  do{
    int rankOfNeighbor = getRank(myMPICoordinates, relativeLocation);
    if(rankOfNeighbor != myRank && rankOfNeighbor != MPI_PROC_NULL) createNeighbor(nghs, rankOfNeighbor, relativeLocation, globalBoundaries);
  }while(nghs->increment(relativeLocation));

}

void CartTopology::createNeighbor(Neighbors* nghs, int rank, std::vector<int> relativeLocation, GridDimensions globalBoundaries) {
	if (rank != MPI_PROC_NULL) {
		Neighbor* ngh = new Neighbor(rank, getDimensions(rank, globalBoundaries));
		nghs->addNeighbor(ngh, relativeLocation);
	}
}

Neighbor::Neighbor(int rank, GridDimensions bounds) :
	_rank(rank), _bounds(bounds) {
}

int Neighbors::getIndex(std::vector<int> relativeLocation) const{
  int index = 0;
  int base = 1;
  for(int i = 0; i < relativeLocation.size(); i++){
    index += (relativeLocation[i] + 1) * base;
    base *= 3;
  }
  return index;
}

bool Neighbors::increment(std::vector<int>& relativeLocation){
  int i = 0;
  bool addNext = true;
  while((addNext) && (i < relativeLocation.size())){
    relativeLocation[i] = relativeLocation[i] + 1;
    if(relativeLocation[i] < 2) addNext = false;
    else(relativeLocation[i] = -1);
    i++;
  }
  return ((i < relativeLocation.size()) || (addNext == false));

}

Neighbors::Neighbors(int numDimensions){
  int dimCount = 1;
  for(int i = 0; i < numDimensions; i++) dimCount *= 3;
  for (int i = 0; i < dimCount; i++) {
		nghs.push_back(0);
	}
}

void Neighbors::addNeighbor(Neighbor* ngh, std::vector<int> relativeLocation) {
	nghs[getIndex(relativeLocation)] = ngh;
}

Neighbor* Neighbors::neighbor(std::vector<int> relativeLocation) const {
	return nghs[getIndex(relativeLocation)];
}

Neighbor* Neighbors::findNeighbor(const std::vector<int>& pt) {
	for (std::vector<Neighbor*>::iterator iter = nghs.begin(); iter != nghs.end(); ++iter) {
		Neighbor* ngh = *iter;
		if ((ngh != 0) && (ngh->bounds().contains(pt)))
			return ngh;
	}
	return 0;
}

Neighbor* Neighbors::findNeighbor(const std::vector<double>& pt) {
  for (std::vector<Neighbor*>::iterator iter = nghs.begin(); iter != nghs.end(); ++iter) {
		Neighbor* ngh = *iter;
		if ((ngh != 0) && (ngh->bounds().contains(pt)))
			return ngh;
	}
	return 0;
}

Neighbors::~Neighbors() {
	for (vector<Neighbor*>::iterator iter = nghs.begin(); iter != nghs.end(); iter++) {
		Neighbor* ngh = *iter;
		delete ngh;
	}
}

ostream& operator<<(ostream& os, const Neighbors& nghs) {
//	Neighbor* ngh = nghs.neighbor(Neighbors::NW);
//	if (ngh != 0) {
//		os << "\tNW ngh: " << ngh->rank() << ": " << ngh->bounds() << "\n";
//	}
//
//	ngh = nghs.neighbor(Neighbors::NE);
//	if (ngh != 0) {
//		os << "\tNE ngh: " << ngh->rank() << ": " << ngh->bounds() << "\n";
//	}
//
//	ngh = nghs.neighbor(Neighbors::SW);
//	if (ngh != 0) {
//		os << "\tSW ngh: " << ngh->rank() << ": " << ngh->bounds() << "\n";
//	}
//
//	ngh = nghs.neighbor(Neighbors::SE);
//	if (ngh != 0) {
//		os << "\tSE ngh: " << ngh->rank() << ": " << ngh->bounds() << "\n";
//	}
//
//	ngh = nghs.neighbor(Neighbors::N);
//	if (ngh != 0) {
//		os << "\tN ngh: " << ngh->rank() << ": " << ngh->bounds() << "\n";
//	}
//
//	ngh = nghs.neighbor(Neighbors::S);
//	if (ngh != 0) {
//		os << "\tS ngh: " << ngh->rank() << ": " << ngh->bounds() << "\n";
//	}
//
//	ngh = nghs.neighbor(Neighbors::E);
//	if (ngh != 0) {
//		os << "\tE ngh: " << ngh->rank() << ": " << ngh->bounds() << "\n";
//	}
//
//	ngh = nghs.neighbor(Neighbors::W);
//	if (ngh != 0) {
//		os << "\tW ngh: " << ngh->rank() << ": " << ngh->bounds() << "\n";
//	}

	return os;

}

}
