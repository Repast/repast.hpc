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
 *  spatial_math.cpp
 *
 *  Created on: Aug 10, 2010
 *      Author: nick
 */

#include "spatial_math.h"

#include <cmath>
#include <math.h>

#ifdef _MSC_VER
// Round a double
// Note that behavior may not be as expected at max and min double values
double rint(double x){
	return ( x < 0 ?                         
		       ceil(x - 0.5)    :   // base value is negative
			   floor(x + 0.5));     // base value is positive
			        
}
#endif

namespace repast {

void _rotate(double* plane, double angle) {
	double x = plane[0];
	double y = plane[1];
	plane[0] = x * cos(angle) - y * sin(angle);
	plane[1] = y * cos(angle) + x * sin(angle);
}

template<>
std::vector<int> calculateDisplacement<int>(int dimCount, int unitDimension, double scale, const std::vector<double>& anglesInRadians) {
	Point<double> res = calculateDisplacement<double>( dimCount, unitDimension, scale, anglesInRadians);
	std::vector<int> displacement(dimCount, 0);
	std::vector<double> coords = res.coords();
	for (size_t i = 0; i < coords.size(); ++i) {
		displacement[i] = rint(coords[i]);
	}
	return displacement;
}

double toRadians(double angdeg) {
	return angdeg / 180 * PI;
}

double toDegrees(double angrad) {
	return angrad * 180.0 / PI;
}


}
