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
 *  DiffusionLayerND.h
 *
 *  Created on: July 25, 2015
 *      Author: jtm
 */

#ifndef DIFFUSIONLAYERND_H_
#define DIFFUSIONLAYERND_H_

#include "GridDimensions.h"
#include "RelativeLocation.h"
#include <vector>
#include <map>
#include "mpi.h"

using namespace std;

namespace repast {

struct TypeStore {
public:
  double*      sendAddress;
  double*      receiveAddress;
  MPI_Datatype mpiDatatype;
};

/**
 * The DiffusionLayerND class is an N-dimensional layer of
 * double values that can be used to diffuse through an N-D
 * space. Diffusion is a synchronous updating of cells
 * based on adjacent cells' values.
 *
 * The asynchronous update is achieved through bank switching:
 * two separate grids are maintained, and at any time one of
 * them is 'active' and the other is obsolete and ready to
 * accept the next round of values.
 *
 * The most complex part of the DiffusionLayerND class is the
 * interaction with MPI. Cross-process synchronization requires
 * that blocks of cells (technically volumes in N-space) be
 * sent across processes. MPI Derived Datatypes are used
 * to achieve this.
 *
 * The memory for the N-Dimensional array is organized
 * as a nested loop. Assume that the dimensions for
 * the array are d1, d2, d3 ... dN. The extent of the grid
 * in each dimension is e1, e2, e3 ... eN. Note that this
 * includes both the space within the local boundaries
 * and the adjacent buffer zones. For convenience
 * we pre-calculate a vector M1, M2, M3 ... MN of multipliers; each
 * entry is equal to the product of all the extents of
 * lower-numbered dimensions, with M1 = 1. The address of a cell
 * a locations l1, l2, l3 ... lN will be:
 *
 *    l1 * M1 + l2 * M2 + l3 * M3 ... lN * MN
 *
 * A volume in this space will not occupy a contiguous
 * block of memory. However, the MPI specification indicates
 * that derived data types can be used to define complex
 * regions of memory; the MPI implementation can optimize
 * the sending and receiving of these.
 *
 * In this class, an MPI Datatype is defined to represent
 * the volume of space being sent to and received from
 * each of the 3N - 1 adjacent processes.
 *
 * One important note is that the send and receive data types
 * for a given exchange partner will be identical; only the
 * starting pointer need be changed.
 *
 * (It should be noted that MPI allows these data types to
 * 'match' if they are structurally compatible. They need
 * not actually be identical. So, consider a send/receive
 * pair where one of the pair is against the global
 * simulation boundaries
 *
 * The data type can be defined recursively using MPI's
 * HVector function for all types except the innermost,
 * which is a contiguous block of double values.
 *
 *
 */
class DiffusionLayerND{

private:
  double*        dataSpace1;             // Permanent pointer to bank 1 of the data space
  double*        dataSpace2;             // Permanent pointer to bank 2 of the data space
  double*        currentDataSpace;       // Temporary pointer to the active data space
  double*        otherDataSpace;         // Temporary pointer to the inactive data space
  int            length;                 // Total length of the entire array (one data space)
  GridDimensions overallGridDimensions;  // Origin in global sim coordinates + extents
  int            numDims;                // Number of dimensions
  bool           globalSpaceIsPeriodic;  // True if the global space is periodic

  vector<int>    places;                 // Multipliers to calculate index, for each dimension


  // Note that although the send and receive data types must 'match', they needn't be identical
  // They can bring in different swatches of memory. The MPI implementation will handle this

  map<int, TypeStore> typeStores; // Map of neighbors, by rank, with MPI datatypes for receives


public:
  DiffusionLayerND(vector<int> processesPerDim, GridDimensions globalBoundaries, int bufferSize, bool periodic, double initialValue = 0);
  virtual ~DiffusionLayerND();

  void initialize(double initialValue);
  void diffuse();

  double getValueAt(Point<int> location);
  double getValueAt(vector<int> location);

  double* getSendStart(RelativeLocation relLoc);
  double* getReceiveStart(RelativeLocation relLoc);
  MPI_Datatype getMPIDataType();

private:

  void setPlaces();

  /**
   * Takes a location in global simulation coordinates,
   * which may cross periodic boundaries, and transforms
   * it into local coordinates, which must be absolute.
   */
  vector<int> transform(vector<int> location);

  /**
   * Given a location in global simulation coordinates,
   * get the offset from the global base pointer to the
   * position in the global array representing that location
   */
  int getIndex(vector<int> location);

};

}

#endif /* DIFFUSIONLAYERND_H_ */
