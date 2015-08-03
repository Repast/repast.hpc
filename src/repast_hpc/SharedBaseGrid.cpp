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



Neighbor::Neighbor(int rank, GridDimensions bounds) :
	_rank(rank), _bounds(bounds) {
}

Neighbors::Neighbors(int numDimensions){
  int dimCount = 1;
  for(int i = 0; i < numDimensions; i++) dimCount *= 3;
  for (int i = 0; i < dimCount; i++) {
		nghs.push_back(0);
	}
}

void Neighbors::addNeighbor(Neighbor* ngh, RelativeLocation relLoc) {
	nghs[relLoc.getIndex()] = ngh;
}

Neighbor* Neighbors::neighbor(RelativeLocation relLoc) const {
	return nghs[relLoc.getIndex()];
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
