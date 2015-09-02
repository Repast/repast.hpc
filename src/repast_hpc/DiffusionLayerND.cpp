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
#include <fstream>

#include "DiffusionLayerND.h"
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

  // Set these provisionally; adjust below if needed
  simplifiedBoundariesMin  = localBoundariesMin;
  if(spaceContinuesLeft)  simplifiedBoundariesMin -= leftBufferSize;

  simplifiedBoundariesMax  = localBoundariesMax;
  if(spaceContinuesRight) simplifiedBoundariesMax += rightBufferSize;

  matchingCoordinateMin    = localBoundariesMin;
  if(spaceContinuesLeft  && !atLeftBound ) matchingCoordinateMin -= leftBufferSize;

  matchingCoordinateMax    = localBoundariesMax;
  if(spaceContinuesRight && !atRightBound) matchingCoordinateMax += rightBufferSize;

  globalWidth = globalCoordinateMax - globalCoordinateMin;
  localWidth = localBoundariesMax - localBoundariesMin;
  width = simplifiedBoundariesMax - simplifiedBoundariesMin;
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
  //std::cout << " GETTING INDEXED COORD FOR ORIGINAL COORD " << originalCoord << " IS SIMPLE? " << isSimplified << " WITH SIMPLE BOUNDS " << simplifiedBoundariesMin << " RET = " << (isSimplified ? originalCoord : getTransformedCoord(originalCoord)) - simplifiedBoundariesMin << endl;
  return (isSimplified ? originalCoord : getTransformedCoord(originalCoord)) - simplifiedBoundariesMin;
}

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

bool Diffusor::skip(vector<int> location){
  return false;
}


DiffusionLayerND::DiffusionLayerND(vector<int> processesPerDim, GridDimensions globalBoundaries, int bufferSize, bool periodic, double initialValue): globalSpaceIsPeriodic(periodic){
  cartTopology = RepastProcess::instance()->getCartesianTopology(processesPerDim, periodic);
  int rank = RepastProcess::instance()->rank();
  GridDimensions localBoundaries = cartTopology->getDimensions(rank, globalBoundaries);

  // Calculate the size to be used for the buffers
  numDims = processesPerDim.size();

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
    datum.report(i);
  }

  // Now create the rank-based data per neighbor
  RelativeLocation relLoc(numDims);
  RelativeLocation relLocTrimmed = cartTopology->trim(rank, relLoc); // Initialized to minima

  neighborData = new RankDatum[relLoc.getMaxIndex() - 1];
  neighborCount = 0;
  int i = 0;
  do{
    if(relLoc.validNonCenter()){ // Skip 0,0,0,0,0
      RankDatum* datum;
      datum = &neighborData[i];
      // Collect the information about this rank here
      getMPIDataType(relLoc, numDims - 1, datum->datatype);
      datum->sendPtrOffset    = getSendPointerOffset(relLoc);
      datum->receivePtrOffset = getReceivePointerOffset(relLoc);
      neighborCount++;
    }
  }while(relLoc.increment());

  // Create the actual arrays for the data
  dataSpace1 = new double[length];
  dataSpace2 = new double[length];
  currentDataSpace = dataSpace1;
  otherDataSpace   = dataSpace2;

  // Create arrays for MPI requests and results (statuses)
  requests = new MPI_Request[neighborCount];

  // Finally, fill the data with the initial values
  initialize(initialValue);

}

DiffusionLayerND::~DiffusionLayerND(){
  delete[] currentDataSpace;
  delete[] otherDataSpace;
  delete[] neighborData; // Should Free MPI Datatypes first...
  delete[] requests;
}


void DiffusionLayerND::initialize(double initialValue){
  for(int i = 0; i < length; i++){ // TODO Optimize
    dataSpace1[i] = initialValue;
    dataSpace2[i] = initialValue;
  }
}

void DiffusionLayerND::diffuse(Diffusor* diffusor){
  vector<int> globalMinima;
  vector<int> globalMaxima;
  vector<int> localMinima;
  vector<int> localMaxima;
  for(int i = 0; i < numDims; i++){
    DimensionDatum* datum = &dimensionData[i];
    globalMinima.push_back(datum->simplifiedBoundariesMin);
    globalMaxima.push_back(datum->simplifiedBoundariesMax);
    localMinima.push_back(datum->localBoundariesMin);
    localMaxima.push_back(datum->localBoundariesMax);
  }
  RelativeLocation trimmer(globalMinima, globalMaxima);
  RelativeLocation looper(localMinima, localMaxima);

  int preferredRadius = diffusor->getRadius();
  vector<int> localRadiusMinima;
  vector<int> localRadiusMaxima;
  for(int i = 0; i < numDims; i++){
    localRadiusMinima.push_back(-1 * preferredRadius);
    localRadiusMaxima.push_back(     preferredRadius);
  }

  // BaseLocal is a basic RelativeLocation that has the needed radius,
  // usually 1.
  RelativeLocation baseLocal(localRadiusMinima, localRadiusMaxima);

  int c = 0;

  std::cout << " ABOUT TO ENTER DIFFUSOR LOOP ON RANK " << repast::RepastProcess::instance()->rank() << std::endl;
  do{
    if(!diffusor->skip(looper.getCurrentValue())){
      c++;
      RelativeLocation currentLocal(baseLocal);
//      currentLocal.translate(looper.getCurrentValue());
//      RelativeLocation trimmedLocal = trimmer.trim(currentLocal);


//      double* vals = new double[trimmedLocal.getMaxIndex()];
//
//      int indx = 0;
//      do{
//        vals[indx] = currentDataSpace[getIndex(trimmedLocal.getCurrentValue(), true)];
//        indx++;
//      }while(trimmedLocal.increment());
//      otherDataSpace[getIndex(looper.getCurrentValue(), true)] = diffusor->getNewValue(trimmedLocal, vals);
//      delete vals;
    }
  }while(looper.increment());
  std::cout << " COUNT ON RANK " << repast::RepastProcess::instance()->rank() << " IS " << c << std::endl;

  // Switch the data banks
  double* tempDataSpace = currentDataSpace;
  currentDataSpace      = otherDataSpace;
  otherDataSpace        = tempDataSpace;

  synchronize();
}


vector<int> DiffusionLayerND::getIndexes(vector<int> location, bool isSimplified){
  vector<int> ret;
  ret.assign(numDims, 0); // Make the right amount of space
  for(int i = 0; i < numDims; i++) ret.push_back(dimensionData[i].getIndexedCoord(location[i], isSimplified));
  return ret;
}

int DiffusionLayerND::getIndex(vector<int> location, bool isSimplified){
  vector<int> indexed = getIndexes(location, isSimplified);
  int val = 0;
  for(int i = numDims - 1; i >= 0; i--)  val += indexed[i] * places[i];

  return val;
}

int DiffusionLayerND::getIndex(Point<int> location){
  return getIndex(location.coords());
}

void DiffusionLayerND::getMPIDataType(RelativeLocation relLoc, int dimensionIndex, MPI_Datatype& datatype){

  if(dimensionIndex == 0){
    MPI_Type_contiguous(dimensionData[dimensionIndex].getSendReceiveSize(relLoc[dimensionIndex]), MPI_DOUBLE, &datatype);
  }
  else{
    MPI_Datatype innerType;
    getMPIDataType(relLoc, dimensionIndex - 1, innerType);
    MPI_Type_hvector(dimensionData[dimensionIndex].getSendReceiveSize(relLoc[dimensionIndex]), // Count
                     1,                                                                        // BlockLength: just one of the inner data type
                     strides[dimensionIndex],                                                  // Stride, in bytes
                     innerType,                                                                // Inner Datatype
                     &datatype);
  }
  // Commit?
  MPI_Type_commit(&datatype);
}

void DiffusionLayerND::synchronize(){
  // For each entry in neighbors:
  MPI_Status* statuses = new MPI_Status[neighborCount];
  for(int i = 0; i < neighborCount; i++){
    MPI_Isend(&currentDataSpace[neighborData[i].sendPtrOffset], 1, neighborData[i].datatype,
        neighborData[i].rank, 10101, cartTopology->topologyComm, &requests[i]);
    MPI_Irecv(&currentDataSpace[neighborData[i].receivePtrOffset], 1, neighborData[i].datatype,
        neighborData[i].rank, 10101, cartTopology->topologyComm, &requests[i + 1]);
  }
  // Wait
  MPI_Waitall(neighborCount, requests, statuses);
  delete[] statuses;

  // Done! Data will be in the new buffer in the current data space
}

int DiffusionLayerND::getSendPointerOffset(RelativeLocation relLoc){
  int rank = repast::RepastProcess::instance()->rank();
  int ret = 0;
  for(int i = 0; i < numDims; i++){
    DimensionDatum* datum = &dimensionData[i];
    ret += (relLoc[i] <= 0 ? datum->leftBufferSize : datum->width - datum->rightBufferSize) * places[i];
  }
  return ret;
}

int DiffusionLayerND::getReceivePointerOffset(RelativeLocation relLoc){
  int rank = repast::RepastProcess::instance()->rank();
  int ret = 0;
  for(int i = 0; i < numDims; i++){
    DimensionDatum* datum = &dimensionData[i];
    ret += (relLoc[i] < 0 ? 0 : (relLoc[i] == 0 ? datum->leftBufferSize : datum->width - datum->rightBufferSize)) * places[i];
  }
  return ret;
}


double DiffusionLayerND::addValueAt(double val, Point<int> location){
  double* pt = &currentDataSpace[getIndex(location)];
  return (*pt = *pt + val);
}

double DiffusionLayerND::addValueAt(double val, vector<int> location){
  double* pt = &currentDataSpace[getIndex(location)];
  return (*pt = *pt + val);
  }

double DiffusionLayerND::setValueAt(double val, Point<int> location){
  double* pt = &currentDataSpace[getIndex(location)];
  return (*pt = val);

}

double DiffusionLayerND::setValueAt(double val, vector<int> location){
  double* pt = &currentDataSpace[getIndex(location)];
  return (*pt = val);
}

double DiffusionLayerND::getValueAt(vector<int> location){
  return currentDataSpace[getIndex(location)];
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

  // Create a RelativeLocation object will all the values
  vector<int> min;
  vector<int> max;
  for(int i = 0; i < numDims; i++){
    DimensionDatum* datum = &dimensionData[i];
    min.push_back(writeSharedBoundaryAreas ? datum->simplifiedBoundariesMin : datum->localBoundariesMin);
    max.push_back(writeSharedBoundaryAreas ? datum->simplifiedBoundariesMax : datum->localBoundariesMax);
  }
  RelativeLocation relLoc(min, max);
  do{
    vector<int> currentLocation = relLoc.getCurrentValue();
    repast::Point<int> locPoint(currentLocation);
//    std::cout << " Attempting to deal with : " << locPoint << " at " << getIndex(currentLocation, false) << endl;
    for(int i = 0; i < numDims; i++) outfile << currentLocation[i] << ",";
    outfile << currentDataSpace[getIndex(currentLocation, true)] << endl;
  }while(relLoc.increment());


  outfile.close();


}

}
