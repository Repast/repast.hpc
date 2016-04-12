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
 *  	 Redistributions of source code must retain the above copyright notice,
 *  	 this list of conditions and the following disclaimer.
 *
 *  	 Redistributions in binary form must reproduce the above copyright notice,
 *  	 this list of conditions and the following disclaimer in the documentation
 *  	 and/or other materials provided with the distribution.
 *
 *  	 Neither the name of the Argonne National Laboratory nor the names of its
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
 *  CartesianTopology.h
 *
 *  Created on: July 28, 2015
 *      Author: jtm
 */

#ifndef CARTESIANTOPOLOGY_H_
#define CARTESIANTOPOLOGY_H_

#include <map>
#include "mpi.h"
#include <boost/mpi.hpp>

#include "GridDimensions.h"
#include "RelativeLocation.h"

using namespace std;

namespace repast {

class CartesianTopology {

private:
  bool               periodic;
  std::vector<int>   procsPerDim;

public:
  MPI_Comm           topologyComm;

  CartesianTopology(std::vector<int> processesPerDim, bool spaceIsPeriodic, boost::mpi::communicator* world);
  virtual ~CartesianTopology();

  /**
   * Gets the rank of the process at the specified offset
   * from the specified location
   */
  int  getRank(std::vector<int>& loc, std::vector<int>& relLoc);

  /**
   * Gets the coordinates in the MPI Cartesian Communicator
   * for the specified rank
   */
  void getCoordinates(int rank, std::vector<int>& coords);

  /**
   * Gets the GridDimensions boundaries for the specified
   * rank
   */
  GridDimensions getDimensions(int rank, GridDimensions globalBoundaries);

  /**
   * Gets the GridDimensions boundaries for the specified
   * MPI coordinates
   */
  GridDimensions getDimensions(std::vector<int>& pCoordinates, GridDimensions globalBoundaries);

  /**
   * Trims the relative location volume to only valid values.
   * If the CartesianTopology is periodic, this will have no effect,
   * but if part of the relative location falls outside the
   * boundaries of a (nonperiodic) Cartesian Topology, the
   * RelativeLocation returned will include only the locations
   * that fall within the boundaries and will be initialized
   * to the minima values
   */
  RelativeLocation trim(int rank, RelativeLocation volume);

  /**
   * Returns true only if the periodicity specified matches
   * the periodicity of this CartesianTopology, the size
   * of the vector of processes per dimension matches, and
   * the value for each dimension matches.
   */
  bool matches(std::vector<int> processesPerDim, bool spaceIsPeriodic);
};
}

#endif /* DIFFUSIONLAYERND_H_ */
