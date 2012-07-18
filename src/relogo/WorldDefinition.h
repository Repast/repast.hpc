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
 *  WorldDefinition.h
 *
 *  Created on: Aug 5, 2010
 *      Author: nick
 */

#ifndef WORLDDEFINITION_H_
#define WORLDDEFINITION_H_

#include <vector>
#include "RelogoAgent.h"
#include "RelogoLink.h"

#include "repast_hpc/SharedNetwork.h"

namespace repast {

namespace relogo {


class RelogoGridAdder;
class RelogoSpaceAdder;

/**
 * Defines a Relogo world.
 */
class WorldDefinition {

private:

  GridDimensions                        _dims;
  bool                                  _wrapped;
  int                                   _buffer;
  std::vector<Projection<RelogoAgent>*> networks;


public:

	/**
	 * An iterator over pointers to Projection<RelogoAgent>.
	 */
	typedef std::vector<Projection<RelogoAgent>*>::const_iterator proj_iter;

	/**
	 * Creates a world definition with the specified parameters. These parameter will
	 * be applied when the world is created using a WorldCreator.
	 *
	 * @param minX the minimum x coordinate of the world
	 * @param minY the minimum y coordinate of the world
	 * @param maxX the maximum x coordinate of the world
	 * @param maxY the maximum y coordinate of the world
	 * @param wrapped whether or not the space is periodic, wrapped as a torus
	 * @param buffer the size of the grid and space buffer between process grid and space
	 * representations
	 */
	WorldDefinition(int minX, int minY, int maxX, int maxY, bool wrapped, int buffer);
	virtual ~WorldDefinition();

	/**
	 * Defines a network with the specified name and whether or
	 * not the network is directed. The network will use the
	 * default RelogoEdge.
	 *
	 * @param name the name of the network
	 * @param directed if true, the network will be directed, otherwise it will be undirected
	 */
	void defineNetwork(std::string name, bool directed, RelogoLinkContentManager* rlcm);

	/**
	 * Defines the default network and whether or
	 * not the network is directed. Any network related calls
	 * that don't specify a name will use this network. The network
	 * will use RelogoEdge-s by default
	 *
	 * @param directed if true, the network will be directed, otherwise it will be undirected
	 */
	void defineNetwork(bool directed, RelogoLinkContentManager* rlcm);

	/**
	 * Gets the start of an iterator over the network Projections
	 * defined in this WorldDefinition. The iterator returns a pointer to
	 * a Projection<RelogoAgent>*.
	 */
	proj_iter networks_begin() const {
		return networks.begin();
	}

	/**
		 * Gets the end of an iterator over the network Projections
		 * defined in this WorldDefinition. The iterator returns a pointer to
		 * a Projection<RelogoAgent>*.
		 */
	proj_iter networks_end() const {
		return networks.end();
	}

	/**
	 * Gets the minimum x coordinate of the world.
	 *
	 * @return the minimum x coordinate of the world.
	 */
	int minX() const {
		return _dims.origin(0);
	}

	/**
		 * Gets the minimum y coordinate of the world.
		 *
		 * @return the minimum y coordinate of the world.
		 */
	int minY() const {
		return _dims.origin(1);
	}

	/**
		 * Gets the maximum x coordinate of the world.
		 *
		 * @return the maximum x coordinate of the world.
		 */
	int maxX() const {
		return _dims.origin(0) + _dims.extents(0) - 1;
	}

	/**
			 * Gets the maximum y coordinate of the world.
			 *
			 * @return the maximum y coordinate of the world.
			 */
	int maxY() const {
		return _dims.origin(1) + _dims.extents(1) - 1;
	}

	/**
	 * Gets the dimensions of the world expressed as a GridDimensions.
	 *
	 * @return the dimensions of the world expressed as a GridDimensions.
	 */
	const GridDimensions dimensions() const {
		return _dims;
	}


	/**
	 * Gets whether or not the world wraps.
	 *
	 * @return true if the world wraps, otherwise false.
	 */
	bool isWrapped() const {
		return _wrapped;
	}

	/**
	 * Gets the size of the grid / space buffer.
	 *
	 * @return the size of the grid / space buffer.
	 */
	int buffer() const {
		return _buffer;
	}


};


}

}

#endif /* WORLDDEFINITION_H_ */
