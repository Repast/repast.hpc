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

int DiffusionLayerND::syncCount = 0;

DiffusionLayerND::DiffusionLayerND(vector<int> processesPerDim, GridDimensions globalBoundaries, int bufferSize, bool periodic,
    double initialValue, double initialBufferZoneValue): AbstractValueLayerND(processesPerDim, globalBoundaries, bufferSize, periodic){

  // Create the actual arrays for the data
  dataSpace1 = new double[length];
  dataSpace2 = new double[length];
  currentDataSpace = dataSpace1;
  otherDataSpace   = dataSpace2;

  // Finally, fill the data with the initial values
  initialize(initialValue, initialBufferZoneValue);

  // And synchronize
  synchronize();

}

DiffusionLayerND::~DiffusionLayerND(){
  delete[] currentDataSpace;
  delete[] otherDataSpace;
}


void DiffusionLayerND::initialize(double initialValue, bool fillBufferZone, bool fillLocal){
  fillDimension(initialValue, initialValue, fillBufferZone, fillLocal, dataSpace1, dataSpace2, numDims - 1);
}

void DiffusionLayerND::initialize(double initialLocalValue, double initialBufferZoneValue){
  fillDimension(initialLocalValue, initialBufferZoneValue, true, true, dataSpace1, dataSpace2, numDims - 1);
}


double DiffusionLayerND::addValueAt(double val, Point<int> location){
  int indx = getIndex(location);
  if(indx == -1) return nan("");
  double* pt = &currentDataSpace[indx];
  return (*pt = *pt + val);
}

double DiffusionLayerND::addValueAt(double val, vector<int> location){
  int indx = getIndex(location);
  if(indx == -1) return nan("");
  double* pt = &currentDataSpace[indx];
  return (*pt = *pt + val);
}

double DiffusionLayerND::setValueAt(double val, Point<int> location){
  int indx = getIndex(location);
  if(indx == -1) return nan("");
  double* pt = &currentDataSpace[indx];
  return (*pt = val);
}

double DiffusionLayerND::setValueAt(double val, vector<int> location){
  int indx = getIndex(location);
  if(indx == -1) return nan("");
  double* pt = &currentDataSpace[indx];
  return (*pt = val);
}

double DiffusionLayerND::getValueAt(vector<int> location){
  int indx = getIndex(location);
  if(indx == -1) return nan("");
  return currentDataSpace[indx];
}



void DiffusionLayerND::synchronize(){
  syncCount++;
  if(syncCount > 9) syncCount = 0;
  // Note: the syncCount and send/recv directions are used to create a unique tag value for the
  // mpi sends and receives. The tag value must be unique in two ways: first, successive calls to this
  // function must be different enough that they can't be confused. The 'syncCount' value is used to
  // achieve this, and it will loop from 0-9 and then repeat. The second, the tag must sometimes
  // differentiate between sends and receives that are going to the same rank. If a dimension
  // has only 2 processes but wrap-around borders, then one process may be sending to the other
  // process twice (once left and once right). The 'sendDir' and 'recvDir' values trap this

  // For each entry in neighbors:
  MPI_Status statuses[neighborCount * 2];
  for(int i = 0; i < neighborCount; i++){
    MPI_Isend(&currentDataSpace[neighborData[i].sendPtrOffset], 1, neighborData[i].datatype,
        neighborData[i].rank, 10 * (neighborData[i].sendDir + 1) + syncCount, cartTopology->topologyComm, &requests[i]);
    MPI_Irecv(&currentDataSpace[neighborData[i].receivePtrOffset], 1, neighborData[i].datatype,
        neighborData[i].rank, 10 * (neighborData[i].recvDir + 1) + syncCount, cartTopology->topologyComm, &requests[neighborCount + i]);
  }
  int ret = MPI_Waitall(neighborCount, requests, statuses);
}

void DiffusionLayerND::diffuse(Diffusor* diffusor, bool omitSynchronize){
  int countOfVals = (int)(pow(diffusor->getRadius() * 2 + 1, numDims));
  double* vals = new double[countOfVals];

  diffuseDimension(currentDataSpace, otherDataSpace, vals, diffusor, numDims - 1);

  // Switch the data banks
  double* tempDataSpace = currentDataSpace;
  currentDataSpace      = otherDataSpace;
  otherDataSpace        = tempDataSpace;

  if(!omitSynchronize) synchronize();

  delete[] vals;
}


void DiffusionLayerND::write(string fileLocation, string fileTag, bool writeSharedBoundaryAreas){
  std::ofstream outfile;
  std::ostringstream stream;
  int rank = repast::RepastProcess::instance()->rank();
  stream << fileLocation << "DiffusionLayer_" << fileTag << "_" << rank << ".csv";
  std::string filename = stream.str();

  const char * c = filename.c_str();
  outfile.open(c, std::ios_base::trunc | std::ios_base::out); // it will not delete the content of file, will add a new line

  // Write headers
  for(int i = 0; i < numDims; i++) outfile << "DIM_" << i << ",";
  outfile << "VALUE" << endl;

  int* positions = new int[numDims];
  for(int i = 0; i < numDims; i++) positions[i] = 0;

  writeDimension(outfile, currentDataSpace, positions, numDims - 1, writeSharedBoundaryAreas);

  outfile.close();
}



void DiffusionLayerND::fillDimension(double localValue, double bufferValue, bool doBufferZone, bool doLocal, double* dataSpace1Pointer, double* dataSpace2Pointer, int dimIndex){
  if(!doBufferZone && !doLocal) return;
  int bufferEdge = dimensionData[dimIndex].leftBufferSize;
  int localEdge  = bufferEdge + dimensionData[dimIndex].localWidth;
  int upperBound = localEdge + dimensionData[dimIndex].rightBufferSize;

  int pointerIncrement = places[dimIndex];


  int i = 0;
  for(; i < bufferEdge; i++){
    if(doBufferZone){
      if(dimIndex == 0){
        *dataSpace1Pointer = bufferValue;
        *dataSpace2Pointer = bufferValue;
      }
      else{
        fillDimension(bufferValue, bufferValue, doBufferZone, doLocal, dataSpace1Pointer, dataSpace2Pointer, dimIndex - 1);
      }
    }
    // Increment the pointers
    dataSpace1Pointer += pointerIncrement;
    dataSpace2Pointer += pointerIncrement;
  }
  for(; i < localEdge; i++){
    if(doLocal){
      if(dimIndex == 0){
        *dataSpace1Pointer = localValue;
        *dataSpace2Pointer = localValue;
      }
      else{
        fillDimension(localValue, bufferValue, doBufferZone, doLocal, dataSpace1Pointer, dataSpace2Pointer, dimIndex - 1);
      }
    }
    // Increment the pointers
    dataSpace1Pointer += pointerIncrement;
    dataSpace2Pointer += pointerIncrement;
  }
  if(doBufferZone){ // Note: we don't need to finish this at all if not doing buffer zone
    for(; i < upperBound; i++){
      if(dimIndex == 0){
        *dataSpace1Pointer = bufferValue;
        *dataSpace2Pointer = bufferValue;
      }
      else{
        fillDimension(bufferValue, bufferValue, doBufferZone, doLocal, dataSpace1Pointer, dataSpace2Pointer, dimIndex - 1);
      }
    }
    dataSpace1Pointer += pointerIncrement;
    dataSpace2Pointer += pointerIncrement;
  }

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


void DiffusionLayerND::writeDimension(std::ofstream& outfile, double* dataSpacePointer, int* currentPosition, int dimIndex, bool writeSharedBoundaryAreas){
  int bufferEdge = dimensionData[dimIndex].leftBufferSize;
  int localEdge  = bufferEdge + dimensionData[dimIndex].localWidth;
  int upperBound = localEdge + dimensionData[dimIndex].rightBufferSize;

  int pointerIncrement = places[dimIndex];
  int i = 0;
  for(; i < bufferEdge; i++){
    currentPosition[dimIndex] = i;
    if(writeSharedBoundaryAreas){
      if(dimIndex == 0){
        double val = *dataSpacePointer;
        if(val != 0){
          for(int j = 0; j < numDims; j++) outfile << (currentPosition[j] - dimensionData[j].leftBufferSize) << ",";
          outfile << val << endl;
        }
      }
      else{
        writeDimension(outfile, dataSpacePointer, currentPosition, dimIndex - 1, writeSharedBoundaryAreas);
      }
    }
    // Increment the pointers
    dataSpacePointer += pointerIncrement;
  }
  for(; i < localEdge; i++){
    currentPosition[dimIndex] = i;
    if(dimIndex == 0){
        double val = *dataSpacePointer;
        if(val != 0){
          for(int j = 0; j < numDims; j++) outfile << (currentPosition[j] - dimensionData[j].leftBufferSize) << ",";
          outfile << val << endl;
        }
    }
    else{
      writeDimension(outfile, dataSpacePointer, currentPosition, dimIndex - 1, writeSharedBoundaryAreas);
    }
    // Increment the pointers
    dataSpacePointer += pointerIncrement;
  }
  if(writeSharedBoundaryAreas){ // Note: we don't need to finish this at all if not doing buffer zone
    for(; i < upperBound; i++){
      currentPosition[dimIndex] = i;
      if(dimIndex == 0){
        double val = *dataSpacePointer;
        if(val != 0){
          for(int j = 0; j < numDims; j++) outfile << (currentPosition[j] - dimensionData[j].leftBufferSize) << ",";
          outfile << *dataSpacePointer << endl;
        }
      }
      else{
        writeDimension(outfile, dataSpacePointer, currentPosition, dimIndex - 1, writeSharedBoundaryAreas);
      }
    }
    dataSpacePointer += pointerIncrement;
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

