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
#include "RepastErrors.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <limits.h>
#include <math.h>

using namespace std;

namespace repast {

// Borders (parent class for border implementations with fixed boundaries
Borders::Borders(GridDimensions d): _dimensions(d){ }

void Borders::boundsCheck(const vector<int>& pt) const {
	if (!_dimensions.contains(pt)) throw Repast_Error_12<GridDimensions>(pt, _dimensions); // Point is out of dimension range
}

void Borders::boundsCheck(const vector<double>& pt) const {
	if (!_dimensions.contains(pt)) throw std::out_of_range("Point is out of dimension range"); // !
}

void Borders::transform(const std::vector<double>& in, std::vector<double>& out) const {
	boundsCheck(in);
	if (out.size() != in.size()) out.insert(out.begin(), in.size(), 0);
	std::copy(in.begin(), in.end(), out.begin());
}

void Borders::transform(const std::vector<int>& in, std::vector<int>& out) const {
	boundsCheck(in);
	if (out.size() != in.size()) out.insert(out.begin(), in.size(), 0);
	std::copy(in.begin(), in.end(), out.begin());
}


// Strict Borders (translations outside the borders throw error
StrictBorders::StrictBorders(GridDimensions d): Borders(d) { }

void StrictBorders::translate(const std::vector<double>& oldPos, std::vector<double>& newPos, const std::vector<double>& displacement) const {
	if (displacement.size() != oldPos.size() || displacement.size() != newPos.size())
      throw Repast_Error_13(oldPos, newPos, displacement); // Position and displacement vectors must be of the same size

	for (int i = 0, n = displacement.size(); i < n; ++i) newPos[i] = oldPos[i] + displacement[i];

	boundsCheck(newPos);
}

void StrictBorders::translate(const std::vector<int>& oldPos, std::vector<int>& newPos,
		const std::vector<int>& displacement) const {
	if (displacement.size() != oldPos.size() || displacement.size() != newPos.size())
      throw Repast_Error_14(oldPos, newPos, displacement); // Position and displacement vectors must be of the same size

	for (int i = 0, n = displacement.size(); i < n; ++i) newPos[i] = oldPos[i] + displacement[i];

	boundsCheck(newPos);
}



// Sticky Borders: Translations outside the border are fixed to the border

StickyBorders::StickyBorders(GridDimensions d): Borders(d) {
  for (size_t i = 0, n = _dimensions.dimensionCount(); i < n; ++i) {
    mins.push_back(_dimensions.origin(i));
    maxs.push_back(_dimensions.origin(i) + _dimensions.extents(i)); // Originally - 1
  }
}


void StickyBorders::translate(const std::vector<double>& oldPos, std::vector<double>& newPos,
    const std::vector<double>& displacement) const {
  if (displacement.size() != oldPos.size() || displacement.size() != newPos.size())
      throw Repast_Error_17(oldPos, newPos, displacement); // Position and displacement vectors must be of the same size

  for (size_t i = 0, n = displacement.size(); i < n; ++i) newPos[i] = calcCoord<double>(oldPos[i] + displacement[i], i);

}

void StickyBorders::translate(const std::vector<int>& oldPos, std::vector<int>& newPos,
    const std::vector<int>& displacement) const {
  if (displacement.size() != oldPos.size() || displacement.size() != newPos.size())
      throw Repast_Error_18(oldPos, newPos, displacement); // Position and displacement vectors must be of the same size

  for (size_t i = 0, n = displacement.size(); i < n; ++i) newPos[i] = calcCoord<int>(oldPos[i] + displacement[i], i);

}


// Wrap-around Borders

WrapAroundBorders::WrapAroundBorders(GridDimensions dimensions): _dimensions(dimensions) {
  for (size_t i = 0; i < dimensions.dimensionCount(); ++i) {
    mins.push_back(dimensions.origin(i));
    maxs.push_back(dimensions.origin(i) + dimensions.extents(i)); // Originally - 1
  }
}

void WrapAroundBorders::transform(const std::vector<int>& in, std::vector<int>& out) const {
	if (out.size() < in.size())	out.insert(out.begin(), in.size(), 0);

  for (size_t i = 0, n = in.size(); i < n; ++i){
    int coord = in[i];
    if(coord >= mins[i] && coord < maxs[i])
      out[i] = coord;
    else
      out[i] = fmod((double)(coord-_dimensions.origin(i)), (double)_dimensions.extents(i))  +
               (coord < _dimensions.origin(i) ? _dimensions.extents(i) : 0) +
               _dimensions.origin(i);
  }
}

void WrapAroundBorders::transform(const std::vector<double>& in, std::vector<double>& out) const {
	if (out.size() < in.size()) out.insert(out.begin(), in.size(), 0);

  for (size_t i = 0, n = in.size(); i < n; ++i){
    double coord = in[i];
    if(coord >= mins[i] && coord < maxs[i])
      out[i] = coord;
    else{
      out[i] = fmod((coord-_dimensions.origin(i)), _dimensions.extents(i))  +
               (coord < _dimensions.origin(i) ? _dimensions.extents(i) : 0) +
               _dimensions.origin(i);
      if(out[i] >= maxs[i])       out[i] = nextafter(maxs[i], -DBL_MAX);
      else if(out[i] < mins[i])   out[i] = nextafter(mins[i],  DBL_MAX);
    }
  }

}

void WrapAroundBorders::translate(const std::vector<double>& oldPos, std::vector<double>& newPos, const std::vector<
    double>& displacement) const {

  if (displacement.size() != oldPos.size() || displacement.size() != newPos.size())
      throw Repast_Error_15(oldPos, newPos, displacement); // Position and displacement vectors must be of the same size

  for (int i = 0, n = displacement.size(); i < n; ++i) newPos[i] = oldPos[i] + displacement[i];
  transform(newPos, newPos);
}

void WrapAroundBorders::translate(const std::vector<int>& oldPos, std::vector<int>& newPos,
    const std::vector<int>& displacement) const {

  if (displacement.size() != oldPos.size() || displacement.size() != newPos.size())
      throw Repast_Error_16(oldPos, newPos, displacement); // Position and displacement vectors must be of the same size

  for (int i = 0, n = displacement.size(); i < n; ++i) newPos[i] = oldPos[i] + displacement[i];
  transform(newPos, newPos);
}

}

