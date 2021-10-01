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
template<typename T>
class Diffusor{

public:

  Diffusor();
  virtual ~Diffusor();

  /**
   * Implementing classes must return the value that is the
   * number of concentric layers that are used in diffusion
   * calculations. Typically this will be 1, meaning that
   * only immediately adjacent grid cells are considered.
   *
   * @return the radius of the (rectilinear) volume that encompasses
   * all cells that will diffuse into the central cell
   */
  virtual int getRadius();

  /**
   * Given a list of values that represent the values in
   * adjacent cells, return the value that should be placed
   * in the central cell. The list of values should be in
   * the order defined by a RelativeLocation object of the
   * specified radius with the central cell being at
   * (0, 0, 0, ...)
   *
   * @param values An array of values found in the adjacent
   * cells from which the diffusion into this cell can be calculated
   *
   * @return the value that should be placed in the central cell
   * based on diffusion from adjacent cells
   */
  virtual T getNewValue(T* values) = 0;
};


/**
 * Empty constructor
 */
template<typename T>
Diffusor<T>::Diffusor(){}

/**
 * No-Op Destructor
 */
template<typename T>
Diffusor<T>::~Diffusor(){}

/**
 * Tefault radius is one
 */
template<typename T>
int Diffusor<T>::getRadius(){
  return 1;
}

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
template<typename T>
class DiffusionLayerND: public ValueLayerNDSU<T>{

private:

public:

  DiffusionLayerND(vector<int> processesPerDim, GridDimensions globalBoundaries, int bufferSize, bool periodic, T initialValue = 0, T initialBufferZoneValue = 0);
  virtual ~DiffusionLayerND();

  /**
   * Performs the diffusion operation on the entire
   * grid (only within local boundaries)
   * If omit synchronize is set to 'true' will not perform
   * a synchronization after diffusion- this is mainly
   * useful for performance testing, as a synchronization
   * is required to complete diffusion
   *
   * @param diffusor A pointer to an instance of a diffusor class
   * that will contain the simulation-specific diffusion code
   * @param omitSynchronize If true, diffusion will be done but
   * not synchronized across processes; this is mainly useful
   * for debugging. By default synchronization is performed
   * after diffusion.
   */
  void diffuse(Diffusor<T>* diffusor, bool omitSynchronize = false);

private:

  /**
   * Diffuse across one of the dimensions. Note that this is called
   * recursively.
   */
  void diffuseDimension(T* currentDataSpacePointer, T* otherDataSpacePointer, T* vals, Diffusor<T>* diffusor, int dimIndex);

  /**
   * Gets the data found in the relevant dimension
   */
  void grabDimensionData(T*& destinationPointer, T* startPointer, int radius, int dimIndex);
};


template<typename T>
DiffusionLayerND<T>::DiffusionLayerND(vector<int> processesPerDim, GridDimensions globalBoundaries, int bufferSize, bool periodic,
    T initialValue, T initialBufferZoneValue): ValueLayerNDSU<T>(processesPerDim, globalBoundaries, bufferSize, periodic,
        initialValue, initialBufferZoneValue){

}

template<typename T>
DiffusionLayerND<T>::~DiffusionLayerND(){
}

template<typename T>
void DiffusionLayerND<T>::diffuse(Diffusor<T>* diffusor, bool omitSynchronize){
  int countOfVals = (int)(pow(diffusor->getRadius() * 2 + 1, AbstractValueLayerND<T>::numDims));
  T* vals = new T[countOfVals];

  diffuseDimension(ValueLayerNDSU<T>::currentDataSpace, ValueLayerNDSU<T>::otherDataSpace, vals, diffusor, AbstractValueLayerND<T>::numDims - 1);

  this->switchValueLayer();

  if(!omitSynchronize) this->synchronize();

  delete[] vals;
}

template<typename T>
void DiffusionLayerND<T>::diffuseDimension(T* currentDataSpacePointer, T* otherDataSpacePointer, T* vals, Diffusor<T>* diffusor, int dimIndex){
  int bufferEdge = AbstractValueLayerND<T>::dimensionData[dimIndex].leftBufferSize;
  int localEdge  = bufferEdge + AbstractValueLayerND<T>::dimensionData[dimIndex].localWidth;

  int pointerIncrement = AbstractValueLayerND<T>::places[dimIndex];

  int i = 0;
  for(; i < bufferEdge; i++){
    // Increment the pointers
    currentDataSpacePointer += pointerIncrement;
    otherDataSpacePointer   += pointerIncrement;
  }
  for(; i < localEdge; i++){
    if(dimIndex == 0){
      // Populate the vals array
      T* destLocation = vals; // Note: This gets passed as a handle and changed
      grabDimensionData(destLocation, currentDataSpacePointer, diffusor->getRadius(), AbstractValueLayerND<T>::numDims - 1);
      *otherDataSpacePointer = diffusor->getNewValue(vals);
    }
    else{
      diffuseDimension(currentDataSpacePointer, otherDataSpacePointer, vals, diffusor, dimIndex - 1);
    }
    // Increment the pointers
    currentDataSpacePointer += pointerIncrement;
    otherDataSpacePointer   += pointerIncrement;
  }
}

template<typename T>
void DiffusionLayerND<T>::grabDimensionData(T*& destinationPointer, T* startPointer, int radius, int dimIndex){
  int pointerIncrement = AbstractValueLayerND<T>::places[dimIndex];
  startPointer -= pointerIncrement * radius; // Go back
  int size = 2 * radius + 1;
  for(int i = 0; i < size; i++){
    if(dimIndex == 0){
      *destinationPointer = 1;
      double myVal = *startPointer;
      *destinationPointer = myVal;
      destinationPointer++;                 // Handle; all recursive instances share
    }
    else{
      grabDimensionData(destinationPointer, startPointer, radius, dimIndex - 1);
    }
    startPointer += pointerIncrement;
  }

}



}

#endif /* DIFFUSIONLAYERND_H_ */
