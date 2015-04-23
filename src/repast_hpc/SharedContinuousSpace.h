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
 *  SharedContinuousSpace.h
 *
 *  Created on: Jul 20, 2010
 *      Author: nick
 */

#ifndef SHAREDCONTINUOUSSPACE_H_
#define SHAREDCONTINUOUSSPACE_H_

#include <boost/mpi/communicator.hpp>

#include "SharedBaseGrid.h"

namespace repast {

/**
 * Continuous space SharedBaseGrid implementation. This
 * primarily adds the buffer synchronization appropriate for this
 * type. Default templated typical SharedContinuousSpaces are defined in SharedGrids.
 *
 * @see SharedBaseGrid for more details.
 *
 * @tparam T the type of objects contained by this BaseGrid
 * @tparam GPTransformer transforms cell points according to the topology (e.g. periodic)
 * of the BaseGrid.
 * @tparam Adder determines how objects are added to the grid from its associated context.
 */
template<typename T, typename GPTransformer, typename Adder>
class SharedContinuousSpace: public SharedBaseGrid<T, GPTransformer, Adder, double> {

protected:
	virtual void synchMoveTo(const AgentId& id, const Point<double>& pt);

private:

	typedef SharedBaseGrid<T, GPTransformer, Adder, double> SharedBaseGridType;

public:
	virtual ~SharedContinuousSpace();
	SharedContinuousSpace(std::string name, GridDimensions gridDims, std::vector<int> processDims, int buffer, boost::mpi::communicator* communicator);

};

template<typename T, typename GPTransformer, typename Adder>
SharedContinuousSpace<T, GPTransformer, Adder>::SharedContinuousSpace(std::string name, GridDimensions gridDims,
		std::vector<int> processDims, int buffer, boost::mpi::communicator* communicator) :
	SharedBaseGrid<T, GPTransformer, Adder, double> (name, gridDims, processDims, buffer, communicator) {
}

template<typename T, typename GPTransformer, typename Adder>
void SharedContinuousSpace<T, GPTransformer, Adder>::synchMoveTo(const AgentId& id, const Point<double>& pt) {
	//unlikely chance that agent could have
	// moved and then "died" and so removed from sending context, in which case
	// it would never get sent to this grid.
	if (SharedBaseGridType::GridBaseType::contains(id)) {
		SharedBaseGridType::GridBaseType::moveTo(id, pt.coords());
	}
}


template<typename T, typename GPTransformer, typename Adder>
SharedContinuousSpace<T, GPTransformer, Adder>::~SharedContinuousSpace() {
}

}

#endif /* SHAREDCONTINUOUSSPACE_H_ */
