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
 *  ValueLayerND.cpp
 *
 *  Created on: July 18, 2016
 *      Author: jtm
 */
#include "ValueLayerND.h"
#include "RepastProcess.h"
#include "Point.h"

#include <boost/mpi.hpp>

using namespace std;

namespace repast {


DimensionDatum::DimensionDatum(int indx, GridDimensions globalBoundaries, GridDimensions localBoundaries, int buffer, bool isPeriodic):
    leftBufferSize(buffer), rightBufferSize(buffer), periodic(isPeriodic){
  globalCoordinateMin = globalBoundaries.origin(indx);
  globalCoordinateMax = globalBoundaries.origin(indx) + globalBoundaries.extents(indx);
  localBoundariesMin  = localBoundaries.origin(indx);
  localBoundariesMax  = localBoundaries.origin(indx) + localBoundaries.extents(indx);

  atLeftBound  = localBoundariesMin == globalCoordinateMin;
  atRightBound = localBoundariesMax == globalCoordinateMax;

  spaceContinuesLeft  = !atLeftBound  || periodic;
  spaceContinuesRight = !atRightBound || periodic;

  simplifiedBoundariesMin  = localBoundariesMin - leftBufferSize;
  simplifiedBoundariesMax  = localBoundariesMax + rightBufferSize;

  matchingCoordinateMin    = localBoundariesMin;
  if(spaceContinuesLeft  && !atLeftBound ) matchingCoordinateMin -= leftBufferSize;

  matchingCoordinateMax    = localBoundariesMax;
  if(spaceContinuesRight && !atRightBound) matchingCoordinateMax += rightBufferSize;

  globalWidth = globalCoordinateMax - globalCoordinateMin;
  localWidth = localBoundariesMax - localBoundariesMin;
  width = leftBufferSize + localWidth + rightBufferSize;
  widthInBytes = width * (sizeof(double));
}

int DimensionDatum::getSendReceiveSize(int relativeLocation){
  switch(relativeLocation){
    case -1:  return leftBufferSize;
    case  1:  return rightBufferSize;
    case  0:
    default:
      return localWidth;
  }
}

int DimensionDatum::getTransformedCoord(int originalCoord){
  if(originalCoord < matchingCoordinateMin){        // Assume (!) original is on right (!) side of periodic boundary, starting at some value
    return matchingCoordinateMax + (originalCoord - globalCoordinateMin);
  }
  else if(originalCoord > matchingCoordinateMax){
    return matchingCoordinateMin - (globalCoordinateMax - originalCoord);
  }
  else return originalCoord; // Within matching boundaries; no need to transform

}


int DimensionDatum::getIndexedCoord(int originalCoord, bool isSimplified){
  return (isSimplified ? originalCoord : getTransformedCoord(originalCoord)) - simplifiedBoundariesMin;
}

bool DimensionDatum::isInLocalBounds(int originalCoord){
  return originalCoord >= localBoundariesMin && originalCoord < localBoundariesMax;
}


AbstractValueLayerND::AbstractValueLayerND(vector<int> processesPerDim, GridDimensions globalBoundaries,int bufferSize, bool periodic): globalSpaceIsPeriodic(periodic){
  cartTopology = RepastProcess::instance()->getCartesianTopology(processesPerDim, periodic);
  // Calculate the size to be used for the buffers
  numDims = processesPerDim.size();

  int rank = RepastProcess::instance()->rank();
  GridDimensions localBoundaries = cartTopology->getDimensions(rank, globalBoundaries);

  // First create the basic coordinate data per dimension
  length = 1;
  int val = 1;
  for(int i = 0; i < numDims; i++){
    DimensionDatum datum(i, globalBoundaries, localBoundaries, bufferSize, periodic);
    length *= datum.width;
    dimensionData.push_back(datum);
    places.push_back(val);
    strides.push_back(val * sizeof(double));
    val *= dimensionData[i].width;
  }

  // Now create the rank-based data per neighbor
  RelativeLocation relLoc(numDims);
  RelativeLocation relLocTrimmed = cartTopology->trim(rank, relLoc); // Initialized to minima

  vector<int> myCoordinates;
  cartTopology->getCoordinates(rank, myCoordinates);

  neighborData = new RankDatum[relLoc.getMaxIndex()];
  neighborCount = 0;
  int i = 0;
  do{
    if(relLoc.validNonCenter()){ // Skip 0,0,0,0,0
      RankDatum* datum;
      datum = &neighborData[neighborCount];
      // Collect the information about this rank here
      getMPIDataType(relLoc, datum->datatype);
      datum->sendPtrOffset    = getSendPointerOffset(relLoc);
      datum->receivePtrOffset = getReceivePointerOffset(relLoc);
      vector<int> current = relLoc.getCurrentValue();
      datum->rank = cartTopology->getRank(myCoordinates, current);
      datum->sendDir = RelativeLocation::getDirectionIndex(current);
      datum->recvDir = RelativeLocation::getReverseDirectionIndex(current);

      neighborCount++;
    }
  }while(relLoc.increment());

  // Create arrays for MPI requests and results (statuses)
  requests = new MPI_Request[neighborCount * 2];
}

AbstractValueLayerND::~AbstractValueLayerND(){
  delete[] neighborData; // Should Free MPI Datatypes first...
  delete[] requests;
}


bool AbstractValueLayerND::isInLocalBounds(vector<int> coords){
  for(int i = 0; i < numDims; i++){
    DimensionDatum* datum = &dimensionData[i];
    if(!datum->isInLocalBounds(coords[i])) return false;
  }
  return true;
}

bool AbstractValueLayerND::isInLocalBounds(Point<int> location){
  return isInLocalBounds(location.coords());
}

vector<int> AbstractValueLayerND::getIndexes(vector<int> location, bool isSimplified){
  vector<int> ret;
  ret.assign(numDims, 0); // Make the right amount of space
  for(int i = 0; i < numDims; i++) ret[i] = dimensionData[i].getIndexedCoord(location[i], isSimplified);
  return ret;
}

int AbstractValueLayerND::getIndex(vector<int> location, bool isSimplified){
  vector<int> indexed = getIndexes(location, isSimplified);
  int val = 0;
  for(int i = numDims - 1; i >= 0; i--) val += indexed[i] * places[i];
  if(val < 0 || val > length) val = -1;
  return val;
}

int AbstractValueLayerND::getIndex(Point<int> location){
  return getIndex(location.coords());
}


void AbstractValueLayerND::getMPIDataType(RelativeLocation relLoc, MPI_Datatype &datatype){
  vector<int> sideLengths;
  for(int i = 0; i < numDims; i++) sideLengths.push_back(dimensionData[i].getSendReceiveSize(relLoc[i]));
  getMPIDataType(sideLengths, datatype, numDims - 1);
}

void AbstractValueLayerND::getMPIDataType(int radius, MPI_Datatype &datatype){
  vector<int> sideLengths;
  sideLengths.assign(numDims, 2 * radius + 1);
  getMPIDataType(sideLengths, datatype, numDims - 1);
}

void AbstractValueLayerND::getMPIDataType(vector<int> sideLengths, MPI_Datatype &datatype, int dimensionIndex){
  if(dimensionIndex == 0){
    MPI_Type_contiguous(sideLengths[dimensionIndex], MPI_DOUBLE, &datatype);
  }
  else{
    MPI_Datatype innerType;
    getMPIDataType(sideLengths, innerType, dimensionIndex - 1);
    MPI_Type_hvector(sideLengths[dimensionIndex], // Count
                     1,                                                                        // BlockLength: just one of the inner data type
                     strides[dimensionIndex],                                                  // Stride, in bytes
                     innerType,                                                                // Inner Datatype
                     &datatype);
  }
  // Commit?
  MPI_Type_commit(&datatype);
}

int AbstractValueLayerND::getSendPointerOffset(RelativeLocation relLoc){
  int rank = repast::RepastProcess::instance()->rank();
  int ret = 0;
  for(int i = 0; i < numDims; i++){
    DimensionDatum* datum = &dimensionData[i];
    ret += (relLoc[i] <= 0 ? datum->leftBufferSize : datum->width - (2 * datum->rightBufferSize)) * places[i];
  }
  return ret;
}

int AbstractValueLayerND::getReceivePointerOffset(RelativeLocation relLoc){
  int rank = repast::RepastProcess::instance()->rank();
  int ret = 0;
  for(int i = 0; i < numDims; i++){
    DimensionDatum* datum = &dimensionData[i];
    ret += (relLoc[i] < 0 ? 0 : (relLoc[i] == 0 ? datum->leftBufferSize : datum->width - datum->rightBufferSize)) * places[i];
  }
  return ret;
}





int ValueLayerND::syncCount = 0;

ValueLayerND::ValueLayerND(vector<int> processesPerDim, GridDimensions globalBoundaries, int bufferSize, bool periodic,
    double initialValue, double initialBufferZoneValue): AbstractValueLayerND(processesPerDim, globalBoundaries, bufferSize, periodic){

  // Create the actual arrays for the data
  dataSpace = new double[length];

  // Finally, fill the data with the initial values
  initialize(initialValue, initialBufferZoneValue);

  // And synchronize
  synchronize();

}

ValueLayerND::~ValueLayerND(){
  delete[] dataSpace;
}


void ValueLayerND::initialize(double initialValue, bool fillBufferZone, bool fillLocal){
  fillDimension(initialValue, initialValue, fillBufferZone, fillLocal, dataSpace, numDims - 1);
}

void ValueLayerND::initialize(double initialLocalValue, double initialBufferZoneValue){
  fillDimension(initialLocalValue, initialBufferZoneValue, true, true, dataSpace, numDims - 1);
}

double ValueLayerND::addValueAt(double val, Point<int> location){
  int indx = getIndex(location);
  if(indx == -1) return nan("");
  double* pt = &dataSpace[indx];
  return (*pt = *pt + val);
}

double ValueLayerND::addValueAt(double val, vector<int> location){
  int indx = getIndex(location);
  if(indx == -1) return nan("");
  double* pt = &dataSpace[indx];
  return (*pt = *pt + val);
}

double ValueLayerND::setValueAt(double val, Point<int> location){
  int indx = getIndex(location);
  if(indx == -1) return nan("");
  double* pt = &dataSpace[indx];
  return (*pt = val);
}

double ValueLayerND::setValueAt(double val, vector<int> location){
  int indx = getIndex(location);
  if(indx == -1) return nan("");
  double* pt = &dataSpace[indx];
  return (*pt = val);
}

double ValueLayerND::getValueAt(vector<int> location){
  int indx = getIndex(location);
  if(indx == -1) return nan("");
  return dataSpace[indx];
}

void ValueLayerND::synchronize(){
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
    MPI_Isend(&dataSpace[neighborData[i].sendPtrOffset], 1, neighborData[i].datatype,
        neighborData[i].rank, 10 * (neighborData[i].sendDir + 1) + syncCount, cartTopology->topologyComm, &requests[i]);
    MPI_Irecv(&dataSpace[neighborData[i].receivePtrOffset], 1, neighborData[i].datatype,
        neighborData[i].rank, 10 * (neighborData[i].recvDir + 1) + syncCount, cartTopology->topologyComm, &requests[neighborCount + i]);
  }
  int ret = MPI_Waitall(neighborCount, requests, statuses);
}


void ValueLayerND::write(string fileLocation, string fileTag, bool writeSharedBoundaryAreas){
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

  writeDimension(outfile, dataSpace, positions, numDims - 1, writeSharedBoundaryAreas);

  outfile.close();
}


void ValueLayerND::fillDimension(double localValue, double bufferValue, bool doBufferZone, bool doLocal, double* dataSpacePointer, int dimIndex){
  if(!doBufferZone && !doLocal) return;
  int bufferEdge = dimensionData[dimIndex].leftBufferSize;
  int localEdge  = bufferEdge + dimensionData[dimIndex].localWidth;
  int upperBound = localEdge + dimensionData[dimIndex].rightBufferSize;

  int pointerIncrement = places[dimIndex];


  int i = 0;
  for(; i < bufferEdge; i++){
    if(doBufferZone){
      if(dimIndex == 0){
        *dataSpacePointer = bufferValue;
      }
      else{
        fillDimension(bufferValue, bufferValue, doBufferZone, doLocal, dataSpacePointer, dimIndex - 1);
      }
    }
    // Increment the pointers
    dataSpacePointer += pointerIncrement;
  }
  for(; i < localEdge; i++){
    if(doLocal){
      if(dimIndex == 0){
        *dataSpacePointer = localValue;
      }
      else{
        fillDimension(localValue, bufferValue, doBufferZone, doLocal, dataSpacePointer, dimIndex - 1);
      }
    }
    // Increment the pointers
    dataSpacePointer += pointerIncrement;
  }
  if(doBufferZone){ // Note: we don't need to finish this at all if not doing buffer zone
    for(; i < upperBound; i++){
      if(dimIndex == 0){
        *dataSpacePointer = bufferValue;
      }
      else{
        fillDimension(bufferValue, bufferValue, doBufferZone, doLocal, dataSpacePointer, dimIndex - 1);
      }
    }
    dataSpacePointer += pointerIncrement;
  }

}

void ValueLayerND::writeDimension(std::ofstream& outfile, double* dataSpacePointer, int* currentPosition, int dimIndex, bool writeSharedBoundaryAreas){
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



int ValueLayerNDSU::syncCount = 0;

ValueLayerNDSU::ValueLayerNDSU(vector<int> processesPerDim, GridDimensions globalBoundaries, int bufferSize, bool periodic,
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

ValueLayerNDSU::~ValueLayerNDSU(){
  delete[] currentDataSpace;
  delete[] otherDataSpace;
}


void ValueLayerNDSU::initialize(double initialValue, bool fillBufferZone, bool fillLocal){
  fillDimension(initialValue, initialValue, fillBufferZone, fillLocal, dataSpace1, dataSpace2, numDims - 1);
}

void ValueLayerNDSU::initialize(double initialLocalValue, double initialBufferZoneValue){
  fillDimension(initialLocalValue, initialBufferZoneValue, true, true, dataSpace1, dataSpace2, numDims - 1);
}


double ValueLayerNDSU::addValueAt(double val, Point<int> location){
  int indx = getIndex(location);
  if(indx == -1) return nan("");
  double* pt = &currentDataSpace[indx];
  return (*pt = *pt + val);
}

double ValueLayerNDSU::addValueAt(double val, vector<int> location){
  int indx = getIndex(location);
  if(indx == -1) return nan("");
  double* pt = &currentDataSpace[indx];
  return (*pt = *pt + val);
}

double ValueLayerNDSU::setValueAt(double val, Point<int> location){
  int indx = getIndex(location);
  if(indx == -1) return nan("");
  double* pt = &currentDataSpace[indx];
  return (*pt = val);
}

double ValueLayerNDSU::setValueAt(double val, vector<int> location){
  int indx = getIndex(location);
  if(indx == -1) return nan("");
  double* pt = &currentDataSpace[indx];
  return (*pt = val);
}

double ValueLayerNDSU::getValueAt(Point<int> location){
  int indx = getIndex(location);
  if(indx == -1) return nan("");
  return currentDataSpace[indx];
}

double ValueLayerNDSU::getValueAt(vector<int> location){
  int indx = getIndex(location);
  if(indx == -1) return nan("");
  return currentDataSpace[indx];
}



void ValueLayerNDSU::synchronize(){
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

void ValueLayerNDSU::write(string fileLocation, string fileTag, bool writeSharedBoundaryAreas){
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


void ValueLayerNDSU::switchValueLayer(){
  // Switch the data banks
  double* tempDataSpace = currentDataSpace;
  currentDataSpace      = otherDataSpace;
  otherDataSpace        = tempDataSpace;
}


void ValueLayerNDSU::fillDimension(double localValue, double bufferValue, bool doBufferZone, bool doLocal, double* dataSpace1Pointer, double* dataSpace2Pointer, int dimIndex){
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


void ValueLayerNDSU::writeDimension(std::ofstream& outfile, double* dataSpacePointer, int* currentPosition, int dimIndex, bool writeSharedBoundaryAreas){
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





}

