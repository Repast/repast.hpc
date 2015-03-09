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

Neighbors::Location sendDirections[8] = {
    Neighbors::NW, Neighbors::N, Neighbors::NE,
    Neighbors::W,                Neighbors::E,
    Neighbors::SW, Neighbors::S, Neighbors::SE
};

Neighbors::Location recvDirections[8] = {
    Neighbors::SE, Neighbors::S, Neighbors::SW,
    Neighbors::E,                Neighbors::W,
    Neighbors::NE, Neighbors::N, Neighbors::NW
};


CartTopology::CartTopology(vector<int> processesPerDim, vector<double> origin, vector<double> extents, bool spaceIsPeriodic, boost::mpi::communicator* comm) :
  periodic(spaceIsPeriodic), procsPerDim(processesPerDim) {
  int numDims = procsPerDim.size();
  int* periods = new int[numDims];
  int periodicFlag = periodic ? 1 : 0;
  for (int i = 0; i < numDims; i++) periods[i] = periodicFlag;

	// swap xy so row major
	swapXY(processesPerDim);
	swapXY(procsPerDim);
	swapXY(origin);
	swapXY(extents);

	MPI_Cart_create(*comm, numDims, &processesPerDim[0], periods, 0, &topologyComm);
	delete[] periods;
	globalBounds = GridDimensions(origin, extents);
}

void CartTopology::getCoordinates(int rank, std::vector<int>& coords) {
	coords.assign(globalBounds.dimensionCount(), 0);
	MPI_Cart_coords(topologyComm, rank, globalBounds.dimensionCount(), &coords[0]);
}

GridDimensions CartTopology::getDimensions(int rank) {
	vector<int> coords;
	getCoordinates(rank, coords);
	return getDimensions(coords);
}

GridDimensions CartTopology::getDimensions(vector<int>& pCoordinates) {
  vector<double> origins, extents;
  for (size_t i = 0; i < pCoordinates.size(); i++) {
    double lower = globalBounds.origin(i) + ((double)pCoordinates[i] / (double)procsPerDim[i]) * globalBounds.extents(i);
    double upper = globalBounds.origin(i) + (((double)pCoordinates[i] + 1 )/ (double)procsPerDim[i]) * globalBounds.extents(i);
    origins.push_back(lower);
    extents.push_back(upper - lower);
  }

	swapXY(origins);
	swapXY(extents);
	return GridDimensions(Point<double> (origins), Point<double> (extents));
}

int CartTopology::getRank(vector<int>& loc, int rowAdj, int colAdj) {
	int* coord = new int[2];
	coord[0] = loc[0] + rowAdj;
	coord[1] = loc[1] + colAdj;
	if (!periodic) {
		if (coord[0] < 0 || coord[0] > procsPerDim[0] - 1 || coord[1] < 0 || coord[1] > procsPerDim[1] - 1){
		  delete[] coord;
			return MPI_PROC_NULL;
		}
	}
	int rank;
	MPI_Cart_rank(topologyComm, coord, &rank);
	delete[] coord;
	return rank;
}

void CartTopology::createNeighbors(Neighbors& nghs) {
  int eRank, wRank, nRank, sRank;
  MPI_Cart_shift(topologyComm, 1,  1, &wRank, &eRank);
	MPI_Cart_shift(topologyComm, 0, -1, &sRank, &nRank);

	createNeighbor(nghs, eRank, Neighbors::E);
	createNeighbor(nghs, wRank, Neighbors::W);
	createNeighbor(nghs, nRank, Neighbors::N);
	createNeighbor(nghs, sRank, Neighbors::S);

	int rank = RepastProcess::instance()->rank();
	vector<int> pCoordinates;
	getCoordinates(rank, pCoordinates);

	createNeighbor(nghs, getRank(pCoordinates, -1, -1), Neighbors::NW);
	createNeighbor(nghs, getRank(pCoordinates, -1, 1), Neighbors::NE);
	createNeighbor(nghs, getRank(pCoordinates, 1, -1), Neighbors::SW);
	createNeighbor(nghs, getRank(pCoordinates, 1, 1), Neighbors::SE);
}

void CartTopology::createNeighbor(Neighbors& nghs, int rank, Neighbors::Location location) {
	if (rank != MPI_PROC_NULL) {
		Neighbor* ngh = new Neighbor(rank, getDimensions(rank));
		nghs.addNeighbor(ngh, location);
	}
}

Neighbor::Neighbor(int rank, GridDimensions bounds) :
	_rank(rank), _bounds(bounds) {
}


Neighbors::Neighbors() {
	for (int i = 0; i < Neighbors::LOCATION_SIZE; i++) {
		nghs.push_back(0);
	}
}

void Neighbors::addNeighbor(Neighbor* ngh, Neighbors::Location location) {
	nghs[location] = ngh;
}

Neighbor* Neighbors::neighbor(Neighbors::Location location) const {
	return nghs[location];
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
	Neighbor* ngh = nghs.neighbor(Neighbors::NW);
	if (ngh != 0) {
		os << "\tNW ngh: " << ngh->rank() << ": " << ngh->bounds() << "\n";
	}

	ngh = nghs.neighbor(Neighbors::NE);
	if (ngh != 0) {
		os << "\tNE ngh: " << ngh->rank() << ": " << ngh->bounds() << "\n";
	}

	ngh = nghs.neighbor(Neighbors::SW);
	if (ngh != 0) {
		os << "\tSW ngh: " << ngh->rank() << ": " << ngh->bounds() << "\n";
	}

	ngh = nghs.neighbor(Neighbors::SE);
	if (ngh != 0) {
		os << "\tSE ngh: " << ngh->rank() << ": " << ngh->bounds() << "\n";
	}

	ngh = nghs.neighbor(Neighbors::N);
	if (ngh != 0) {
		os << "\tN ngh: " << ngh->rank() << ": " << ngh->bounds() << "\n";
	}

	ngh = nghs.neighbor(Neighbors::S);
	if (ngh != 0) {
		os << "\tS ngh: " << ngh->rank() << ": " << ngh->bounds() << "\n";
	}

	ngh = nghs.neighbor(Neighbors::E);
	if (ngh != 0) {
		os << "\tE ngh: " << ngh->rank() << ": " << ngh->bounds() << "\n";
	}

	ngh = nghs.neighbor(Neighbors::W);
	if (ngh != 0) {
		os << "\tW ngh: " << ngh->rank() << ": " << ngh->bounds() << "\n";
	}

	return os;

}

}
