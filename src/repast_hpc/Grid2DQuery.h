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
 *  Grid2DQuery.h
 *
 *  Created on: Aug 12, 2010
 *      Author: nick
 */

#ifndef GRID2DQUERY_H_
#define GRID2DQUERY_H_

#include "RepastErrors.h"

namespace repast {

/**
 * Base class for neighborhood queries on discrete Grids.
 *
 * @tparam T the type of object in the Grid.
 */
template<typename T>
class Grid2DQuery {

protected:
	const Grid<T, int>* _grid;
	int minMax[2][2];

public:

	/**
	 * Creates Grid2DQuery that will query the specified Grid.
	 */
	Grid2DQuery(const Grid<T, int>* grid);
	virtual ~Grid2DQuery() {}

	/**
	 * Queries the Grid for the neighbors surrounding the center point within a specified range. What
	 * constitutes the neighborhood is determines by subclass implementors.
	 *
	 * @param center the center of the neighborhood
	 * @param range the range of the neighborhood out from the center
	 * @param includeCenter whether or not to include any agents at the center
	 * @param [out] the neighboring agents will be returned in this vector
	 */
	virtual void query(const Point<int>& center, int range, bool includeCenter, std::vector<T*>& out) const = 0;
};

template<typename T>
Grid2DQuery<T>::Grid2DQuery(const Grid<T, int>* grid) :
	_grid(grid) {
	if (grid->bounds().dimensionCount() != 2)
      throw Repast_Error_10(grid->bounds().dimensionCount()); // Grid2DQuery only accepts 2D grids

	GridDimensions bounds = grid->bounds();

	for (size_t i = 0; i < 2; i++) {
		int origin = bounds.origin(i);
		minMax[i][0] = origin;
		// max is EXCLUSIVE
		minMax[i][1] = (bounds.extents(i) + bounds.origin(i));
	}
}

}

#endif /* GRID2DQUERY_H_ */
