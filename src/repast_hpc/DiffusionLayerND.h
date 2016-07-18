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

#include <fstream>
#include <vector>
#include <map>

#include "mpi.h"

#include "RelativeLocation.h"
#include "CartesianTopology.h"
#include "RepastProcess.h"
#include "ValueLayerND.h"

using namespace std;

namespace repast {

/**
 * A Diffusor is a custom class that performs diffusion.
 *
 */
class Diffusor{

public:

  Diffusor();
  virtual ~Diffusor();

  /**
   * Implementing classes must return the value that is the
   * number of concentric layers that are used in diffusion
   * calculations. Typically this will be 1, meaning that
   * only immediately adjacent grid cells are considered.
   */
  virtual int getRadius();

  /**
   * Given a list of values that represent the values in
   * adjacent cells, return the value that should be placed
   * in the central cell. The list of values should be in
   * the order defined by a RelativeLocation object of the
   * specified radius with the central cell being at
   * (0, 0, 0, ...)
   */
  virtual double getNewValue(double* values) = 0;
};


/**
 * The DiffusionLayerND class is an N-dimensional layer of
 * double values that can be used to diffuse through an N-D
 * space. Diffusion is a synchronous updating of cells
 * based on adjacent cells' values.
 *
 * The synchronous update is achieved through bank switching:
 * two separate grids are maintained, and at any time one of
 * them is 'active' and the other is obsolete and ready to
 * accept the next round of values.
 *
 * The diffusion routine uses an MPI_Datatype to collect
 * the cells within the defined radius and assemble them
 * into a single buffer. For convenience and performance
 * it uses an MPI send to self- that is, the process
 * performs the send to itself. MPI is used to collect
 * the information from its nested, looped memory and
 * put it into a contiguous and linear buffer. It is
 * up to the diffusor to parse this buffer. Note that
 * if the space is at the edge of a strict boundary,
 * the values in the buffer will be NaN values.
 *
 * The radius of diffusion must be less than or equal to
 * the size of the buffer zone.
 *
 */
class DiffusionLayerND: public AbstractValueLayerND{

private:

  double*                dataSpace1;             // Permanent pointer to bank 1 of the data space
  double*                dataSpace2;             // Permanent pointer to bank 2 of the data space
  double*                currentDataSpace;       // Temporary pointer to the active data space
  double*                otherDataSpace;         // Temporary pointer to the inactive data space

public:
  static int syncCount;

  DiffusionLayerND(vector<int> processesPerDim, GridDimensions globalBoundaries, int bufferSize, bool periodic, double initialValue = 0, double initialBufferZoneValue = 0);
  virtual ~DiffusionLayerND();

  /**
   * Inherited from AbstractValueLayerND
   */
  virtual void initialize(double initialValue, bool fillBufferZone = false, bool fillLocal = true);

  /**
   * Inherited from AbstractValueLayerND
   */
  virtual void initialize(double initialLocalValue, double initialBufferZoneValue);

  /**
   * Inherited from AbstractValueLayerND
   */
  virtual double addValueAt(double val, Point<int> location);

  /**
   * Inherited from AbstractValueLayerND
   */
  virtual double addValueAt(double val, vector<int> location);

  /**
   * Inherited from AbstractValueLayerND
   */
  virtual double setValueAt(double val, Point<int> location);

  /**
   * Inherited from AbstractValueLayerND
   */
  virtual double setValueAt(double val, vector<int> location);

  /**
   * Inherited from AbstractValueLayerND
   */
  virtual double getValueAt(Point<int> location);

  /**
   * Inherited from AbstractValueLayerND
   */
  virtual double getValueAt(vector<int> location);

  /**
   * Inherited from AbstractValueLayerND
   */
  virtual void synchronize();

  /**
   * Performs the diffusion operation on the entire
   * grid (only within local boundaries)
   * If omit synchronize is set to 'true' will not perform
   * a synchronization after diffusion- this is mainly
   * useful for performance testing, as a synchronization
   * is required to complete diffusion
   */
  void diffuse(Diffusor* diffusor, bool omitSynchronize = false);

  /**
   * Write this rank's data to a CSV file
   */
  void write(string fileLocation, string filetag, bool writeSharedBoundaryAreas = false);

private:

  /**
   * Fills a dimension of space with the given value. Used for initialization
   * and clearing only.
   */
  void fillDimension(double localValue, double bufferZoneValue, bool doBufferZone, bool doLocal, double* dataSpace1Pointer, double* dataSpace2Pointer, int dimIndex);

  void diffuseDimension(double* currentDataSpacePointer, double* otherDataSpacePointer, double* vals, Diffusor* diffusor, int dimIndex);

  void grabDimensionData(double*& destinationPointer, double* startPointer, int radius, int dimIndex);

  /*
   * Writes one dimension's information to the specified csv file.
   */
  void writeDimension(std::ofstream& outfile, double* dataSpace1Pointer, int* currentPosition, int dimIndex, bool writeSharedBoundaryAreas = false);

};

}

#endif /* DIFFUSIONLAYERND_H_ */
