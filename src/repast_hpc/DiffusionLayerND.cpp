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
 *  DiffusionLayerND.cpp
 *
 *  Created on: July 25, 2008
 *      Author: jtm
 */
#include "DiffusionLayerND.h"


#include <boost/mpi.hpp>

using namespace std;

namespace repast {

/**
 * Empty constructor
 */
Diffusor::Diffusor(){}

/**
 * No-Op Destructor
 */
Diffusor::~Diffusor(){}

/**
 * Default radius is one
 */
int Diffusor::getRadius(){
  return 1;
}

DiffusionLayerND::DiffusionLayerND(vector<int> processesPerDim, GridDimensions globalBoundaries, int bufferSize, bool periodic,
    double initialValue, double initialBufferZoneValue): ValueLayerNDSU(processesPerDim, globalBoundaries, bufferSize, periodic,
        initialValue, initialBufferZoneValue){

}

DiffusionLayerND::~DiffusionLayerND(){
}


void DiffusionLayerND::diffuse(Diffusor* diffusor, bool omitSynchronize){
  int countOfVals = (int)(pow(diffusor->getRadius() * 2 + 1, numDims));
  double* vals = new double[countOfVals];

  diffuseDimension(currentDataSpace, otherDataSpace, vals, diffusor, numDims - 1);

  switchValueLayer();

  if(!omitSynchronize) synchronize();

  delete[] vals;
}


void DiffusionLayerND::diffuseDimension(double* currentDataSpacePointer, double* otherDataSpacePointer, double* vals, Diffusor* diffusor, int dimIndex){
  int bufferEdge = dimensionData[dimIndex].leftBufferSize;
  int localEdge  = bufferEdge + dimensionData[dimIndex].localWidth;

  int pointerIncrement = places[dimIndex];

  int i = 0;
  for(; i < bufferEdge; i++){
    // Increment the pointers
    currentDataSpacePointer += pointerIncrement;
    otherDataSpacePointer   += pointerIncrement;
  }
  for(; i < localEdge; i++){
    if(dimIndex == 0){
      // Populate the vals array
      double* destLocation = vals; // Note: This gets passed as a handle and changed
      grabDimensionData(destLocation, currentDataSpacePointer, diffusor->getRadius(), numDims - 1);
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


void DiffusionLayerND::grabDimensionData(double*& destinationPointer, double* startPointer, int radius, int dimIndex){
  int pointerIncrement = places[dimIndex];
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



/** UNIT TESTS **/
/*
class TestDiffusorWrapAround: public Diffusor{

  RelativeLocation relLoc;

public:
  TestDiffusorWrapAround(): relLoc(3){}


  virtual double getNewValue(double* values){
    return -1;
  }
};

class TestDiffusorStrict: public Diffusor{

  RelativeLocation relLoc;

public:
  TestDiffusorStrict(): relLoc(3){}

  virtual double getNewValue(double* values){
    return -1;
  }
};



class TestDiffusionLayerND: public ::testing::Test{

  DiffusionLayerND* diffusionLayerWrapAround;
  DiffusionLayerND* diffusionLayerStrict;

public:
  TestDiffusionLayerND(){}
  ~TestDiffusionLayerND(){}

};

TEST_F(TestDiffusionLayerND, Creation){
  repast::Point<double> origin(0, 0, 0);
  repast::Point<double> extent(100, 100, 100);
  repast::GridDimensions gd(origin, extent);
  std::vector<int> processDims;
  processDims.push_back(2);
  processDims.push_back(2);
  processDims.push_back(2);

  int rank = repast::RepastProcess::instance()->rank();

  DiffusionLayerND* diffusionLayerWrapAround = new repast::DiffusionLayerND(processDims, gd, 1, true, 0, nan("") );
  DiffusionLayerND* diffusionLayerStrict     = new repast::DiffusionLayerND(processDims, gd, 1, false, 0, nan("") );
}

TEST_F(TestDiffusionLayerND, Synchronize){

}

TEST_F(TestDiffusionLayerND, Diffusion){
  // Create a diffusor

  // Set a value at a specific location

  // Diffuse it

}

*/ // End Unit Tests

}

