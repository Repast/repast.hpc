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
 *  GridDimensions.h
 *
 *  Created on: 20 June 2012
 *      Author: nick
 */

#ifndef GRIDDIMENSIONS_H_
#define GRIDDIMENSIONS_H_

#include <vector>

#include "RepastErrors.h"
#include "Point.h"

namespace repast {


class GridDimensions {

private:
	friend bool operator==(const GridDimensions &one, const GridDimensions &two);
	friend std::ostream& operator<<(std::ostream& os, const GridDimensions& dimensions);
	Point<int> _extents, _origin;

public:
	GridDimensions();
	explicit GridDimensions(Point<double> extent);

	/**
	 * Creates a GridDimensions with the specified origin and extent.
	 */
	GridDimensions(Point<int> origin, Point<int> extent);

	bool contains(const Point<int>& pt) const;
	bool contains(const std::vector<int>& pt) const;

	bool contains(const Point<double>& pt) const;
	bool contains(const std::vector<double>& pt) const;

	/**
	 * Gets the origin.
	 */
	const Point<int>& origin() const {
		return _origin;
	}

	/**
	 * Gets the extents along each dimension.
	 */
	const Point<int>& extents() const {
		return _extents;
	}

	const int& origin(int index) const {
		return _origin[index];
	}

	const int& extents(int index) const {
		return _extents[index];
	}

	size_t dimensionCount() const {
		return _extents.dimensionCount();
	}

};

bool operator==(const GridDimensions &one, const GridDimensions &two);
bool operator!=(const GridDimensions &one, const GridDimensions &two);
std::ostream& operator<<(std::ostream& os, const GridDimensions& dimensions);

}

#endif /* GRIDDIMENSIONS_H_ */

