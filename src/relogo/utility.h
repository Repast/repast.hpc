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
 *  utility.h
 *
 *  Created on: Aug 4, 2010
 *      Author: nick
 */

#ifndef UTILITY_H_
#define UTILITY_H_

#include <vector>

namespace repast {
namespace relogo {

/**
 * Gets the coresponding grid coordinate for the given space coordinate.
 *
 * @param val the space coordinate value
 * @return the coresponding grid coordinate for the given space coordinate.
 */
int doubleCoordToInt(double val);

/**
 * Computes the difference between the given angles, that is, the number
 * of degrees in the smallest angle by which angle2 could be rotated to
 * produce angle1. A positive answer means a clockwise rotation, a
 * negative answer counterclockwise. The result is always in the range -180
 * to 180, but is never exactly -180. Note that simply subtracting the two
 * angles using the - (minus) operator won't work. Just subtracting
 * corresponds to always rotating clockwise from angle2 to angle1; but
 * sometimes the counterclockwise rotation is shorter. For example, the
 * difference between 5 degrees and 355 degrees is 10 degrees, not -350
 * degrees.
 *
 * @param angle1
 * @param angle2
 */
float subtractHeadings(float angle1, float angle2);

/**
 * Gets the displacement corresponding to moving in the
 * "heading" direction for distance "distance".
 *
 * @param heading
 * @param distance
 * @return the displacement corresponding to moving in the
 * "heading" direction for distance "distance".
 */
std::vector<double> calcDisplacementFromHeadingDistance(float heading, double distance);

/**
 * Gets the angle from the displacement. This returns the angle
 * such that 0 is north.
 *
 * @param dx
 * @param dy
 */
float angleFromDisplacement(double dx, double dy);

/**
 * Adjust the specified origin so that per process continuous spaces can
 * be combined properly.
 */
std::vector<double> adjustOrigin(const std::vector<double> origin);

}
}

#endif /* UTILITY_H_ */
