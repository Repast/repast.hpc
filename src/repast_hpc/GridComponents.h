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
 *  GridComponents.h
 *
 *  Created on: Jun 23, 2009
 *      Author: nick
 */

#ifndef GRIDCOMPONENTS_H_
#define GRIDCOMPONENTS_H_

#include <boost/unordered_map.hpp>
#include <boost/shared_ptr.hpp>

#include "GridDimensions.h"
#include "Grid.h"
#include "Random.h"

namespace repast {

/**
 * Base class for representations of border semantics
 * (e.g. Strict, Sticky, etc.)
 */
class Borders {

protected:
	const GridDimensions _dimensions;

	void boundsCheck(const std::vector<int>& pt) const;
	void boundsCheck(const std::vector<double>& pt) const;

public:
	Borders(GridDimensions d);

	void transform(const std::vector<int>& in, std::vector<int>& out) const;
	void transform(const std::vector<double>& in, std::vector<double>& out) const;

	bool isPeriodic() const {
		return false;
	}
};

/**
 * Implements strict grid border semantics: anything
 * outside the dimensions is out of bounds.
 */
class StrictBorders : public Borders {

public:
  StrictBorders(GridDimensions d);

	void translate(const std::vector<double>& oldPos, std::vector<double>& newPos, const std::vector<double>& displacement) const;
	void translate(const std::vector<int>& oldPos, std::vector<int>& newPos, const std::vector<int>& displacement) const;

};

/**
 * Implements sticky border semantics: translates out
 * side of the border are clamped to the border coordinates.
 * Tranforms outside the border throw an exception.
 */
class StickyBorders : public Borders {

private:
	std::vector<int> mins, maxs;

	template<typename T>
	T calcCoord(T coord, int dimension) const;

public:

	StickyBorders(GridDimensions d);
	void translate(const std::vector<double>& oldPos, std::vector<double>& newPos, const std::vector<double>& displacement) const;
	void translate(const std::vector<int>& oldPos, std::vector<int>& newPos, const std::vector<int>& displacement) const;
};

template<typename T>
T StickyBorders::calcCoord(T coord, int dimension) const {
	if (coord < mins[dimension]) return mins[dimension];
	else if (coord > maxs[dimension]) return maxs[dimension];
	else return coord;
}

/**
 * Implements periodic wrap around style border semantics.
 * Points that are outside the borders are wrapped until
 * the point is inside the borders.
 *
 */
class WrapAroundBorders {

private:
	GridDimensions _dimensions;
	std::vector<int> mins, maxs;

public:

	WrapAroundBorders(GridDimensions dimensions);

	void transform(const std::vector<int>& in, std::vector<int>& out) const;
	void transform(const std::vector<double>& in, std::vector<double>& out) const;
	void translate(const std::vector<double>& oldPos, std::vector<double>& newPos, const std::vector<double>& displacement) const;
	void translate(const std::vector<int>& oldPos, std::vector<int>& newPos, const std::vector<int>& displacement) const;

	void init(const GridDimensions& dimensions);

	bool isPeriodic() const {
		return true;
	}

};

/**
 * Basic class for adding elements to grids. NOTE: This does NOT
 * actually add the element to the grid; this simply returns 'true'
 * and leaves the actual addition to the grid up to the user.
 * Other classes may do other things (e.g. add to a random location)
 * but this one does NOT.
 */
template<typename T>
class SimpleAdder {
public:

	template <typename GridType>
	void init(GridDimensions dimensions, GridType* grid) {}
	bool add(boost::shared_ptr<T> agent) {
		return true;
	}
};

/*
template<typename T>
class RandomAdder {
private:
	std::vector<IntUniformGenerator> gens;

public:

	void init(GridDimensions dimensions);
	bool add(boost::shared_ptr<T> agent);
};
*/




}

#endif /* GRIDCOMPONENTS_H_ */
