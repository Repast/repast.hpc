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
 *  Spaces.h
 *
 *  Created on: Jul 19, 2010
 *      Author: nick
 */

#ifndef SPACES_H_
#define SPACES_H_

#include "BaseGrid.h"
#include "GridComponents.h"
#include "MultipleOccupancy.h"
#include "SingleOccupancy.h"

namespace repast {

// Some useful typedefs for standard grid types:

/**
 * Struct within which multiple kinds of space are typedef-ed.
 */
template<typename T>
struct Spaces {

	typedef BaseGrid<T, SingleOccupancy  <T, int>,    StrictBorders,     SimpleAdder<T> , int>    SingleStrictDiscreteSpace;
	typedef BaseGrid<T, SingleOccupancy  <T, int>,    WrapAroundBorders, SimpleAdder<T> , int>    SingleWrappedDiscreteSpace;
	typedef BaseGrid<T, MultipleOccupancy<T, int>,    StrictBorders,     SimpleAdder<T> , int>    MultipleStrictDiscreteSpace;
	typedef BaseGrid<T, MultipleOccupancy<T, int>,    WrapAroundBorders, SimpleAdder<T> , int>    MultipleWrappedDiscreteSpace;

	typedef BaseGrid<T, SingleOccupancy  <T, double>, StrictBorders,     SimpleAdder<T> , double> SingleStrictContinuousSpace;
	typedef BaseGrid<T, SingleOccupancy  <T, double>, WrapAroundBorders, SimpleAdder<T> , double> SingleWrappedContinuousSpace;
	typedef BaseGrid<T, MultipleOccupancy<T, double>, StrictBorders,     SimpleAdder<T> , double> MultipleStrictContinuousSpace;
	typedef BaseGrid<T, MultipleOccupancy<T, double>, WrapAroundBorders, SimpleAdder<T> , double> MultipleWrappedContinuousSpace;

};

}

#endif /* SPACES_H_ */
