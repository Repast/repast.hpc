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
 *  GridDimensions.cpp
 *
 *  Created on: 20 June 2012
 *      Author: nick
 */

#include "Point.h"
#include "GridDimensions.h"
#include "RepastErrors.h"

#include <stdexcept>
#include <iostream>
#include <algorithm>

using namespace std;

namespace repast {

GridDimensions::GridDimensions() :
	_extents(0), _origin(0) {
}

GridDimensions::GridDimensions(Point<double> extent) :
	_extents(extent), _origin(Point<double>(vector<double>(extent.dimensionCount(), 0))) {
}

GridDimensions::GridDimensions(Point<double> origin, Point<double> dimensions) :
	_extents(dimensions), _origin(origin) {
	if (_extents.dimensionCount() != _origin.dimensionCount()) {
		throw Repast_Error_36<Point<double> >(origin, dimensions, _origin.dimensionCount(), _extents.dimensionCount()); // Origin dimension count != dimensions dimension count
	}
}

bool GridDimensions::contains(const std::vector<int>& pt) const {
	if (pt.size() != _origin.dimensionCount())
      throw Repast_Error_37(pt, _origin.dimensionCount()); // Point dimension count != dimensions' dimension count


	for (size_t i = 0; i < pt.size(); i++) {
		double start = _origin.getCoordinate(i);
		double end = start + _extents.getCoordinate(i);
		int pVal = pt[i];
		if (pVal < start || pVal >= end)
			return false;
	}
	return true;
}

bool GridDimensions::contains(const Point<int>& pt) const {
	return contains(pt.coords());
}

bool GridDimensions::contains(const Point<double>& pt) const {
	return contains(pt.coords());
}

bool GridDimensions::contains(const std::vector<double>& pt) const {
	if (pt.size() != _origin.dimensionCount()) {
	  throw Repast_Error_38(pt, _origin.dimensionCount()); // Point dimension count != dimensions' dimension count
	}

	for (size_t i = 0; i < pt.size(); i++) {
		double start = _origin.getCoordinate(i);
		double end = start + _extents.getCoordinate(i);
		double pVal = pt[i];
		if (pVal < start || pVal >= end)
			return false;
	}

	return true;
}


bool operator==(const GridDimensions &one, const GridDimensions &two) {
	return one._extents == two._extents && one._origin == two._origin;
}

bool operator!=(const GridDimensions &one, const GridDimensions &two) {
	return !(one == two);
}

ostream& operator<<(ostream& os, const GridDimensions& dimensions) {
	os << "GridDimensions(Origin:[" << dimensions._origin << "], Extent: [" << dimensions._extents << "])";
	return os;
}

}
