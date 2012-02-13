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
 *  Moore2DGridQuery.h
 *
 *  Created on: Aug 12, 2010
 *      Author: nick
 */

#ifndef MOORE2DGRIDQUERY_H_
#define MOORE2DGRIDQUERY_H_

#include "Grid2DQuery.h"
#include "Point.h"
#include "Grid.h"

namespace repast {

/**
 * Neighborhood query that gathers neighbors in a Moore (N, S, E, W, NE, etc.)
 * neighborhood.
 *
 * @tparam T the type of agents in the Grid
 *
 */
template<typename T>
class Moore2DGridQuery: public Grid2DQuery<T> {

public:

	/**
	 * Creates Moore2DGridQuery that will query the specified Grid.
	 */
	Moore2DGridQuery(const Grid<T, int>* grid);
	virtual ~Moore2DGridQuery() {
	}

	/**
	 * Queries the Grid for the Moore neighbors surrounding the center point within a specified range.
	 *
	 * @param center the center of the neighborhood
	 * @param range the range of the neighborhood out from the center
	 * @param includeCenter whether or not to include any agents at the center
	 * @param [out] the neighboring agents will be returned in this vector
	 */
	virtual void query(const Point<int>& center, int range, bool includeCenter, std::vector<T*>& out) const;
};

template<typename T>
Moore2DGridQuery<T>::Moore2DGridQuery(const Grid<T, int>* grid) :
	Grid2DQuery<T> (grid) {
}

template<typename T>
void Moore2DGridQuery<T>::query(const Point<int>& center, int range, bool includeCenter, std::vector<T*>& out) const {

	int xMin = center[0] - range;
	int yMin = center[1] - range;
	int xMax = center[0] + range + 1;
	int yMax = center[1] + range + 1;

	if (!Grid2DQuery<T>::_grid->isPeriodic()) {
		if (xMin < Grid2DQuery<T>::minMax[0][0])
			xMin = Grid2DQuery<T>::minMax[0][0];

		if (yMin < Grid2DQuery<T>::minMax[1][0])
			yMin = Grid2DQuery<T>::minMax[1][0];

		if (xMax >= Grid2DQuery<T>::minMax[0][1])
			xMax = Grid2DQuery<T>::minMax[0][1];

		if (yMax >= Grid2DQuery<T>::minMax[1][1])
			yMax = Grid2DQuery<T>::minMax[1][1];
	}

	if (includeCenter) {
		for (int x = xMin; x < xMax; x++) {
			for (int y = yMin; y < yMax; y++) {
				Grid2DQuery<T>::_grid->getObjectsAt(Point<int> (x, y), out);
			}
		}
	} else {
		for (int x = xMin; x < xMax; x++) {
			for (int y = yMin; y < yMax; y++) {
				if (!(x == center[0] && y == center[1])) {
					Grid2DQuery<T>::_grid->getObjectsAt(Point<int> (x, y), out);
				}
			}
		}
	}

}

}

#endif /* MOORE2DGRIDQUERY_H_ */
