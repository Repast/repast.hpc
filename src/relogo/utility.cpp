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
 *  utility.cpp
 *
 *  Created on: Aug 4, 2010
 *      Author: nick
 */

#include <cmath>

#include "utility.h"
#include "repast_hpc/spatial_math.h"

namespace repast {
namespace relogo {


int doubleCoordToInt(double val) {
  return (int)floor(val + 0.5);
}

float subtractHeadings(float to, float from) {
	float tmp = fmodf((to - from), 360);
	tmp = tmp < 0 ? tmp + 360 : tmp;
	return tmp > 180 ? tmp - 360 : tmp;
}

std::vector<double> calcDisplacementFromHeadingDistance(float heading, double distance) {
	std::vector<double> angles(2, 0);
	// convert heading degrees to radians
	angles[0] = repast::PI / 2 - repast::toRadians(heading);
	return repast::calculateDisplacement<double>(2, 0, distance, angles);
}

float angleFromDisplacement(double dx, double dy) {
	if (dy == 0) {
		return dx >= 0 ? 90 : 270.0;
	}

	double angRad = atan(dx / dy);
	if (dy > 0) {
		return angRad < 0 ? repast::toDegrees(2 * repast::PI + angRad) : repast::toDegrees(angRad);
	}
	return repast::toDegrees(repast::PI + angRad);
}

std::vector<double> adjustOrigin(const std::vector<double> origin) {
  std::vector<double> adjusted(origin.size(), 0);
  for(size_t i = 0, n = origin.size(); i < n; i++) {
    adjusted[i] = origin[i] - 0.5;
  }
  return adjusted;
}

}
}
