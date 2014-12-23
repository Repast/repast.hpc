
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
 *  WorldCreator.h
 *
 *  Created on: Aug 5, 2010
 *      Author: nick
 */

#ifndef WORLDCREATOR_H_
#define WORLDCREATOR_H_

#include <vector>

#include <boost/mpi/communicator.hpp>

#include "repast_hpc/SharedSpaces.h"

#include "WorldDefinition.h"
#include "RelogoDiscreteSpaceAdder.h"
#include "RelogoContinuousSpaceAdder.h"
#include "creators.h"
#include "relogo.h"
#include "grid_types.h"

namespace repast {

namespace relogo {

/**
 * Creates a the relogo world given some parameters.
 */
class WorldCreator {

  boost::mpi::communicator* comm;

public:
	WorldCreator(boost::mpi::communicator* communicator):comm(communicator) {}
	virtual ~WorldCreator() {}

	/**
	 * Creates the Relogo world using the specified parameters and returns an Observer of
	 * ObsType.
	 *
	 * @param worldDef the world definition
	 * @param pConfig a two element vector describing the number of processes along the x and y dimensions
	 * @param patchCreator used to create the Patches.
	 *
	 * @tparam ObsType the type of Observer to create. This must extend Observer.
	 * @tparam PatchType the type of Patches to create. This must either be or extend Patch.
	 * @tparam PatchCreator a function or functor with the following signature
	 * PatchType* (AgentId id, Observer* obs).
	 *
	 */
	template<typename ObsType, typename PatchType, typename PatchCreator>
	ObsType* createWorld(const WorldDefinition& worldDef, const std::vector<int>& pConfig, PatchCreator& patchCreator);

	/**
	 * Creates an observer of the specified type. The observer will
	 * contain a world defined by worldDef and pConfig.
	 *
	 * @param worldDef the world definition
	 * @param pConfig a 2D vector containing the number of processes per grid dimension
	 *
	 * @tparam ObsType the type of Observer to create. This must extend Observer.
	 * @tparam PatchType the type of Patches to create. This must either be or extend Patch.
	 */
	template<typename ObsType, typename PatchType>
	ObsType* createWorld(const WorldDefinition& worldDef, const std::vector<int>& pConfig);

private:

	repast::Projection<RelogoAgent>*  createDiscreteSpace(const WorldDefinition&, const std::vector<int>&) const;
	repast::Projection<RelogoAgent>*  createContinuousSpace(const WorldDefinition&, const std::vector<int>&) const;
};

template<typename ObsType, typename PatchType>
ObsType* WorldCreator::createWorld(const WorldDefinition& worldDef, const std::vector<int>& pConfig) {
	DefaultAgentCreator<PatchType> pCreator;
	return createWorld<ObsType, PatchType> (worldDef, pConfig, pCreator);
}

template<typename ObsType, typename PatchType, typename PatchCreator>
ObsType* WorldCreator::createWorld(const WorldDefinition& worldDef, const std::vector<int>& pConfig,
		PatchCreator& pCreator) {
	ObsType* observer = new ObsType();

	observer->context.addProjection(createDiscreteSpace(worldDef, pConfig));

	Projection<RelogoAgent>* spaceProj = createContinuousSpace(worldDef, pConfig);
	observer->context.addProjection(spaceProj);
	std::string projName = spaceProj->name();
	std::vector<std::string>::iterator iter = observer->context.getAgentsToPushProjOrder.begin();
	while(iter != observer->context.getAgentsToPushProjOrder.end()){
	  if(*iter == projName) iter = observer->context.getAgentsToPushProjOrder.erase(iter);
	  else iter++;
	}


	RelogoGridType* grid = static_cast<RelogoGridType*> (observer->context.getProjection(GRID_NAME));
	RelogoSpaceType* space = static_cast<RelogoSpaceType*> (observer->context.getProjection(SPACE_NAME));
	observer->localBounds = grid->dimensions();

	int id = 0;

	GridDimensions dims = grid->dimensions();
	for (int x = dims.origin(0), n = dims.origin(0) + dims.extents(0); x < n; ++x) {
		for (int y = dims.origin(1), k = dims.origin(1) + dims.extents(1); y < k; ++y) {
			repast::AgentId agentId(id, observer->rank(), PATCH_TYPE_ID);
			agentId.currentRank(observer->_rank);
			id++;
			PatchType* patch = pCreator(agentId, observer);
			observer->context.addAgent(patch);
			patch->_location = Point<double>(x, y);
			grid->moveTo(patch->getId(), Point<int> (x, y));
			space->moveTo(patch->getId(), Point<double> (x, y));
		}
	}

	for (WorldDefinition::proj_iter iter = worldDef.networks_begin(); iter != worldDef.networks_end(); ++iter) {
		observer->context.addProjection(*iter);
	}

	return observer;
}

}

}

#endif /* WORLDCREATOR_H_ */
