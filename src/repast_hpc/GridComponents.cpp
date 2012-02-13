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
 *  GridComponents.cpp
 *
 *  Created on: Jun 23, 2009
 *      Author: nick
 */

#include "GridComponents.h"

#include <algorithm>
#include <cmath>

using namespace std;

namespace repast {

// strict borders implementation
void Borders::boundsCheck(const vector<int>& pt) const {
	if (!_dimensions.contains(pt))
		throw std::out_of_range("Point is out of dimension range");
}

void Borders::boundsCheck(const vector<double>& pt) const {
	if (!_dimensions.contains(pt))
		throw std::out_of_range("Point is out of dimension range");
}

void Borders::transform(const std::vector<double>& in, std::vector<double>& out) const {
	boundsCheck(in);
	if (out.size() != in.size()) {
		out.insert(out.begin(), in.size(), 0);
	}
	std::copy(in.begin(), in.end(), out.begin());
}

void Borders::transform(const std::vector<int>& in, std::vector<int>& out) const {
	boundsCheck(in);
	if (out.size() != in.size()) {
		out.insert(out.begin(), in.size(), 0);
	}
	std::copy(in.begin(), in.end(), out.begin());
}

void StrictBorders::translate(const std::vector<double>& oldPos, std::vector<double>& newPos, const std::vector<double>& displacement) const {
	if (displacement.size() != oldPos.size() || displacement.size() != newPos.size())
		throw std::invalid_argument("Position and displacement vectors must be of the same size");

	for (int i = 0, n = displacement.size(); i < n; ++i) {
		newPos[i] = displacement[i] + oldPos[i];
	}
	boundsCheck(newPos);
}

void StrictBorders::translate(const std::vector<int>& oldPos, std::vector<int>& newPos,
		const std::vector<int>& displacement) const {
	if (displacement.size() != oldPos.size() || displacement.size() != newPos.size())
		throw std::invalid_argument("Position and displacement vectors must be of the same size");

	for (int i = 0, n = displacement.size(); i < n; ++i) {
		newPos[i] = displacement[i] + oldPos[i];
	}
	boundsCheck(newPos);
}

void WrapAroundBorders::init(const GridDimensions& dimensions) {
	_dimensions = dimensions;
	for (size_t i = 0; i < dimensions.dimensionCount(); i++) {
		int origin = dimensions.origin(i);
		mins.push_back(origin);
		maxs.push_back((dimensions.extents(i) + dimensions.origin(i)) - 1);
	}
}

void WrapAroundBorders::transform(const std::vector<int>& in, std::vector<int>& out) const {
	if (out.size() < in.size()) {
		out.insert(out.begin(), in.size(), 0);
	}

	for (size_t i = 0, n = in.size(); i < n; i++) {
		//std::cout << "mins[i]: " << mins[i] << std::endl;
		int coord = in[i];
		if (coord < mins[i] || coord > maxs[i]) {
			//std::cout << "coord: " << coord << std::endl;
			int shiftedLocation = coord - _dimensions.origin(i);
			//std::cout << "sl: " << shiftedLocation << std::endl;
			int value = (int)(fmod((double)shiftedLocation, (double)_dimensions.extents(i)));
			//std::cout << mod << ", " << (int)mod << std::endl;
			//int value = shiftedLocation % _dimensions.extents(i);
			//std::cout << "value: " << value << std::endl;
			if (value < 0)
				value = _dimensions.extents(i) + value;
			//std::cout << "value: " << value << std::endl;
			out[i] = value + _dimensions.origin(i);
			//std::cout << "out[i]: " << out[i] << std::endl;
		} else {
			out[i] = coord;
		}
	}
}

void WrapAroundBorders::transform(const std::vector<double>& in, std::vector<double>& out) const {
	if (out.size() < in.size()) {
		out.insert(out.begin(), in.size(), 0);
	}

	for (size_t i = 0, n = in.size(); i < n; i++) {
		//std::cout << "maxs[i]: " << maxs[i] << std::endl;
		double coord = in[i];
		if (coord < mins[i] || coord > maxs[i]) {
			//std::cout << "coord: " << coord << std::endl;
			double shiftedLocation = coord - _dimensions.origin(i);
			//std::cout << "sl: " << shiftedLocation << std::endl;
			double value = fmod(shiftedLocation, _dimensions.extents(i));
			//std::cout << "value: " << value << std::endl;
			if (value < 0)
				value = _dimensions.extents(i) + value;
			//std::cout << "value: " << value << std::endl;

			value += _dimensions.origin(i);
			// check if wraps completely, if not then clamp so
			// within bounds
			if (value < mins[i]) value = mins[i];
			else if (value > maxs[i]) value = maxs[i];
			out[i] = value;
			//std::cout << "out[i]: " << out[i] << std::endl;
		} else {
			out[i] = coord;
		}
	}
}

void WrapAroundBorders::translate(const std::vector<double>& oldPos, std::vector<double>& newPos, const std::vector<
		double>& displacement) const {

	if (displacement.size() != oldPos.size() || displacement.size() != newPos.size())
		throw std::invalid_argument("Position and displacement vectors must be of the same size");

	for (int i = 0, n = displacement.size(); i < n; ++i) {
		newPos[i] = displacement[i] + oldPos[i];
	}
	transform(newPos, newPos);

}

void WrapAroundBorders::translate(const std::vector<int>& oldPos, std::vector<int>& newPos,
		const std::vector<int>& displacement) const {

	if (displacement.size() != oldPos.size() || displacement.size() != newPos.size())
		throw std::invalid_argument("Position and displacement vectors must be of the same size");

	for (int i = 0, n = displacement.size(); i < n; ++i) {
		newPos[i] = displacement[i] + oldPos[i];
	}
	transform(newPos, newPos);

}

void StickyBorders::init(const GridDimensions& dimensions) {
	_dimensions = dimensions;
	for (size_t i = 0; i < dimensions.dimensionCount(); i++) {
		int origin = dimensions.origin(i);
		mins.push_back(origin);
		maxs.push_back((dimensions.extents(i) + dimensions.origin(i)) - 1);
	}
}


void StickyBorders::translate(const std::vector<double>& oldPos, std::vector<double>& newPos,
		const std::vector<double>& displacement) const {
	if (displacement.size() != oldPos.size() || displacement.size() != newPos.size())
		throw std::invalid_argument("Position and displacement vectors must be of the same size");

	for (int i = 0, n = displacement.size(); i < n; ++i) {
		newPos[i] = calcCoord<double>(displacement[i] + oldPos[i], i);
	}
}

void StickyBorders::translate(const std::vector<int>& oldPos, std::vector<int>& newPos,
		const std::vector<int>& displacement) const {
	//std::cout << displacement.size() << ", " << oldPos.size() << "," << newPos.size() << std::endl;
	if (displacement.size() != oldPos.size() || displacement.size() != newPos.size())
			throw std::invalid_argument("Position and displacement vectors must be of the same size");

		for (int i = 0, n = displacement.size(); i < n; ++i) {
			newPos[i] = calcCoord<int>(displacement[i] + oldPos[i], i);
		}
}

}

