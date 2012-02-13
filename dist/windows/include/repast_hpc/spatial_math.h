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
 *  spatial_math.h
 *
 *  Created on: Aug 10, 2010
 *      Author: nick
 */

#ifndef SPATIAL_MATH_H_
#define SPATIAL_MATH_H_

#include <vector>
#include "Point.h"

namespace repast {

const double PI = 3.141592;

void _rotate(double* plane, double angle);

template<typename GPType>
std::vector<GPType> calculateDisplacement(int dimCount, int unitDimension, double scale,
		const std::vector<double>& anglesInRadians) {

	std::vector<double> displacement(dimCount, 0);
	displacement[unitDimension] = 1;
	double tmp[] = { 0, 0 };
	int c = 0;
	for (int i = 0; i < dimCount; i++) {
		if (i == unitDimension) {
			continue;
		} else if (i > unitDimension) {
			tmp[0] = displacement[unitDimension];
			tmp[1] = displacement[i];
			_rotate(tmp, anglesInRadians[c]);
			displacement[unitDimension] = tmp[0];
			displacement[i] = tmp[1];
		} else if (i < unitDimension) {
			tmp[0] = displacement[i];
			tmp[1] = displacement[unitDimension];
			_rotate(tmp, anglesInRadians[c]);
			displacement[unitDimension] = tmp[1];
			displacement[i] = tmp[0];
		}
		c++;
	}
	for (size_t i = 0; i < displacement.size(); ++i) {
		displacement[i] = displacement[i] * scale;
	}
	return displacement;
}

template<>
std::vector<int> calculateDisplacement<int> (int dimCount, int unitDimension, double scale,const std::vector<double>& anglesInRadians);

/**
 * Converts degrees to radians.
 *
 * @param angdeg the angle in degrees
 *
 * @return the angle in radians.
 */
double toRadians(double angdeg);

/**
 * Converts radians to degrees.
 *
 * @param angrad the angle in radians
 *
 * @return the angle in degrees.
 */
double toDegrees(double angrad);

}

#endif /* SPATIAL_MATH_H_ */
