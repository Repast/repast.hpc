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
 *  Patch.h
 *
 *  Created on: Jul 15, 2010
 *      Author: nick
 */

#ifndef PATCH_H_
#define PATCH_H_

#include "AbstractRelogoAgent.h"
#include "RelogoDiscreteSpaceAdder.h"
#include "AgentSet.h"
#include "repast_hpc/Moore2DGridQuery.h"
#include "repast_hpc/VN2DGridQuery.h"
#include "Observer.h"
#include "relogo.h"

namespace repast {
namespace relogo {

/**
 * A logo patch.
 */
class Patch: public AbstractRelogoAgent {

public:

	/**
	 * Creates a Patch that will have the specified id and be managed by the specified Observer.
	 *
	 * @param id
	 * @param observer
	 */
	Patch(repast::AgentId id, Observer* observer);
	virtual ~Patch();

	/**
	 * Gets the patch x coordinate of this patch's location.
	 */
	virtual int pxCor() const {
		return (int) _location[0];
	}

	/**
	 * Gets the patch y coordinate of this patch's location.
	 */
	virtual int pyCor() const {
		return (int) _location[1];
	}

	/**
	 * Gets the 8 (Moore neighborhood) neighboring Patches of this
	 * Patch.
	 *
	 * @tparam the patch type
	 * @return the 8 (Moore neighborhood) neighboring Patches of this
	 * Patch.
	 */
	template<typename PatchType>
	AgentSet<PatchType> neighbors();

	/**
	 * Gets the 4 (Von Neumann neighborhood) neighboring Patches of this
	 * Patch.
	 *
	 * @tparam the patch type
	 * @return the 4 (Von Neumann neighborhood) neighboring Patches of this
	 * Patch.
	 */
	template<typename PatchType>
	AgentSet<PatchType> neighbors4();

	/**
	 * Gets the 8 (Moore neighborhood) neighboring Patches of this
	 * Patch and puts them out.
	 *
	 * @param out the AgentSet to the neighbors in
	 * @tparam the patch type
	 */
	template<typename PatchType>
	void neighbors(AgentSet<PatchType>& out);

	/**
	 * Gets the 4 (Von Neumann neighborhood) neighboring Patches of this
	 * Patch.
	 *
	 * @param out the AgentSet to put the neighbors in
	 * @tparam the patch type
	 *
	 */
	template<typename PatchType>
	void neighbors4(AgentSet<PatchType>& out);

};

template<typename PatchType>
AgentSet<PatchType> Patch::neighbors() {
	AgentSet<PatchType> set;
	neighbors(set);
	return set;
}

template<typename PatchType>
AgentSet<PatchType> Patch::neighbors4() {
	AgentSet<PatchType> set;
	neighbors4(set);
	return set;
}

template<typename PatchType>
void Patch::neighbors(AgentSet<PatchType>& out) {
  std::vector<RelogoAgent*> vec;
	const RelogoGridType* grid = _observer->grid();
	Moore2DGridQuery<RelogoAgent> query(grid);
	query.query(Point<int> (pxCor(), pyCor()), 1, false, vec);
	AgentSet<PatchType> set;
	filterVecToSet(vec, out, PATCH_TYPE_ID);
}

template<typename PatchType>
void Patch::neighbors4(AgentSet<PatchType>& out) {
	std::vector<RelogoAgent*> vec;
	const RelogoGridType* grid = _observer->grid();
	VN2DGridQuery<RelogoAgent> query(grid);
	query.query(Point<int> (pxCor(), pyCor()), 1, false, vec);
	filterVecToSet(vec, out, PATCH_TYPE_ID);
}

}

}

#endif /* PATCH_H_ */
