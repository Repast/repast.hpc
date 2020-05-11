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
 *  ValueLayerND.h
 *
 *  Created on: July 18, 2016
 *      Author: jtm
 */

#ifndef VALUELAYERND_H_
#define VALUELAYERND_H_

#include <fstream>

#include "mpi.h"

#include "Point.h"
#include "GridDimensions.h"
#include "RepastProcess.h"


using namespace std;

namespace repast {

/**
 * The RankDatum struct stores the data that the ValueLayerND
 * class will need for each of its 3^N - 1 neighboring ranks.
 * N.B.: We could use a map from rank to the rest of the data, but
 * we will rarely need to index it that way, and instead can just
 * loop through it
 *
 */
struct RankDatum{
  int            rank;
  MPI_Datatype   datatype;
  int            sendPtrOffset;
  int            receivePtrOffset;
  int            sendDir;  // Integer representing the direction a send will be sent, in N-space
  int            recvDir;  // Integer representing the direction a receive will have been sent, in N-space
};

/**
 * The DimensionDatum class stores all of the data that the
 * ValueLayerND class will need for each dimension in the
 * N-dimensional space. This mainly includes coordinate boundaries
 * along each dimension, but these boundaries include local,
 * global, and a few other memoized variants.
 */
template<typename T>
class DimensionDatum{
public:

  /**
   * Constructor
   *
   * @param indx
   * @param globalBoundaries global simulation space boundaries
   * @param localBoundaries local boundaries of this process's space
   * @param buffer the size of the buffer zone
   * @param isPeriodic true if the space is periodic, false if not
   */
  DimensionDatum(int indx, GridDimensions globalBoundaries, GridDimensions localBoundaries, int buffer, bool isPeriodic);

  /**
   * Destructor
   */
  virtual ~DimensionDatum(){}

  int  globalCoordinateMin, globalCoordinateMax;            // Coordinates of the global simulation boundaries
  int  localBoundariesMin, localBoundariesMax;              // Coordinates of the local process boundaries in this layer
  int  simplifiedBoundariesMin, simplifiedBoundariesMax;    // Coordinates of the 'simplified' coordinates (assuming local boundary origins and NO wrapping)
  int  leftBufferSize, rightBufferSize;                     // Buffer size
  int  matchingCoordinateMin, matchingCoordinateMax;        // Region within simplified boundaries within which coordinates match Global Simulation Coordinate system
  bool periodic;                                            // True if the global simulation space is periodic on this dimension
  bool atLeftBound, atRightBound;                           // True if the local boundaries abut the global boundaries (whether periodic or not)
  bool spaceContinuesLeft, spaceContinuesRight;             // False if the local boundaries abut the (non-periodic) global boundaries
  int  globalWidth;                                         // Global width of the simulation boundaries in simulation units
  int  localWidth;                                          // Width of the local boundaries
  int  width;                                               // Total width (units = sizeof T) on this dimension (local extents + buffer sizes if not against global nonperiodic bounds)
  int  widthInBytes;

  /**
   * Gets the size of the data to be sent during synchronization
   *
   * @return the size of the data to be sent for this dimension.
   */
  int  getSendReceiveSize(int relativeLocation);

  /**
   * Given a coordinate in global simulation coordinates,
   * returns the value in simplified coordinates.
   *
   * For example, suppose the global space is from 0 to 100,
   * and the local space is from 0 to 10, and the buffer
   * zone value is 3. The simplified coordinates for
   * this region of the local space will be -3 to 13.
   * If the value passed to this function
   * is 99, this function will return -1.
   *
   * @param originalCoord the original coordinate
   *
   * @return the coordinate adjusted for local boundaries system
   */
  int getTransformedCoord(int originalCoord);

  /**
   * Given a coordinate, returns the index of that coordinate.
   * The original coordinate may be in global simulation coordinates
   * or in 'simplified' coordinates. If it is not in simplified
   * coordinates, the first step is to simplify.
   *
   * For example, suppose the global space is from 0 to 100,
   * and the local space is from 0 to 10, and the buffer zone
   * value is 3. Passing 99 in global coordinates is equivalent
   * to passing -1 in simplified coordinates. The index value
   * in both cases is ((-1) - (-3)) or 2.
   *
   * @param originalCoord the original coordinate to be transformed
   * @param isSimplified true if the original coordinate is already in
   * simplified coordinates
   */
  int getIndexedCoord(int originalCoord, bool isSimplified = false);

  /**
   * Returns true if the specified coordinate is within the local boundaries
   * on this dimension.
   *
   * @param originalCoordinate the coordinate to be tested
   *
   * @return true if the coordinate is within the local boundaries
   */
  bool isInLocalBounds(int originalCoord);


  /**
   * Writes a report to the std out file
   *
   * @param dimensionNumber passed to the written report, identifying which
   * dimension this is
   */
  void report(int dimensionNumber){
    std::cout << repast::RepastProcess::instance()->rank() << " " << dimensionNumber << " " <<
                 "global (" << globalCoordinateMin     << ", " << globalCoordinateMax     << ") " <<
                 "local  (" << localBoundariesMin      << ", " << localBoundariesMax      << ") " <<
                 "simple (" << simplifiedBoundariesMin << ", " << simplifiedBoundariesMax << ") " <<
                 "match  (" << matchingCoordinateMin   << ", " << matchingCoordinateMax   << ") " <<
                 "globalWidth = " << globalWidth << " localWidth = " << localWidth << " width = " << width << " bytes = " << widthInBytes << std::endl;
  }
};

template<typename T>
DimensionDatum<T>::DimensionDatum(int indx, GridDimensions globalBoundaries, GridDimensions localBoundaries, int buffer, bool isPeriodic):
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
  widthInBytes = width * (sizeof(T));
}


template<typename T>
int DimensionDatum<T>::getSendReceiveSize(int relativeLocation){
  switch(relativeLocation){
    case -1:  return leftBufferSize;
    case  1:  return rightBufferSize;
    case  0:
    default:
      return localWidth;
  }
}

template<typename T>
int DimensionDatum<T>::getTransformedCoord(int originalCoord){
  if(originalCoord < matchingCoordinateMin){        // Assume (!) original is on right (!) side of periodic boundary, starting at some value
    return matchingCoordinateMax + (originalCoord - globalCoordinateMin);
  }
  else if(originalCoord > matchingCoordinateMax){
    return matchingCoordinateMin - (globalCoordinateMax - originalCoord);
  }
  else return originalCoord; // Within matching boundaries; no need to transform

}

template<typename T>
int DimensionDatum<T>::getIndexedCoord(int originalCoord, bool isSimplified){
  return (isSimplified ? originalCoord : getTransformedCoord(originalCoord)) - simplifiedBoundariesMin;
}

template<typename T>
bool DimensionDatum<T>::isInLocalBounds(int originalCoord){
  return originalCoord >= localBoundariesMin && originalCoord < localBoundariesMax;
}

/*******************************************************************/

/**
 * An AbstractValueLayerND is the abstract parent class for N-dimensional value
 * layers
 */
template<typename T>
class AbstractValueLayerND{

private:
  bool dummy; // Used for cases when error flag is not requested

protected:
  CartesianTopology*         cartTopology;
  GridDimensions             localBoundaries;
  int                        length;                 // Total length of the entire array (one data space)

  int                        numDims;                // Number of dimensions
  bool                       globalSpaceIsPeriodic;  // True if the global space is periodic

  vector<int>                places;                 // Multipliers to calculate index, for each dimension
  vector<int>                strides;                // Sizes of each dimensions, in bytes
  vector<DimensionDatum<T> > dimensionData;          // List of data for each dimension
  RankDatum*                 neighborData;           // List of data for each adjacent rank
  int                        neighborCount;          // Count of adjacent ranks
  MPI_Request*               requests;               // Pointer to MPI requests (for wait operations)

  int                        instanceID;             // Unique ID for managing MPI requests without mix-ups
  int                        syncCount;

  /**
   * Constructor
   *
   * @param processesPerDim number of processes in each dimension
   * @param globalBoundaries global boundaries for the simulation
   * @param bufferSize size of the buffer zone
   * @param periodic true if the space is periodic, false otherwise
   */
  AbstractValueLayerND(vector<int> processesPerDim, GridDimensions globalBoundaries, int bufferSize, bool periodic);
  virtual ~AbstractValueLayerND();


public:

  static int instanceCount;

  /**
   * Returns true only if the coordinates given are within the local boundaries
   *
   * @param coords Coordinates to be tested
   * @return true if the coordinates are within the local boundaries
   */
  virtual bool isInLocalBounds(vector<int> coords);

  /*
   * Returns true only if the coordinates given are within the local boundaries
   *
   * @param coords Coordinates to be tested
   * @return true if the coordinates are within the local boundaries
   */
  virtual bool isInLocalBounds(Point<int> location);

  /**
   * Gets the local boundaries for this process's part of the
   * value layer
   *
   * @return the local boundaries
   */
  const GridDimensions& getLocalBoundaries(){
    return localBoundaries;
  }

protected:
  // Methods implemented in this class but visible only to child classes:

  /**
   * Gets a vector of the indexed locations. If the
   * value passed is already simplified (transformed),
   * does not transform.
   *
   * @param location the location to be transformed
   * @param isSimplified true if the coordinates are already in simplified
   * form
   * @return the transformed coordinates
   */
  vector<int> getIndexes(vector<int> location, bool isSimplified = false);

  /**
   * Given a location in global simulation coordinates,
   * get the offset from the global base pointer to the
   * position in the global array representing that location.
   * The location may be simplified (transformed); if it is
   * not, it is first simplified before the index is calculated.
   *
   * @param location the location to be transformed
   * @param isSimplified true if the coordinates are already in simplified
   * form
   * @return the index from the global base pointer to the position
   * in the array in memory
   */
  int getIndex(vector<int> location, bool isSimplified = false);


  /**
   * Given a location in global simulation coordinates,
   * get the offset from the global base pointer to the
   * position in the global array representing that location.
   * The location may be simplified (transformed); if it is
   * not, it is first simplified before the index is calculated.
   *
   * @param location the location to be transformed
   * @return the index from the global base pointer to the position
   * in the array in memory
   */
  int getIndex(Point<int> location);


  // Virtual methods (implemented by child classes

  /**
   * Initializes the array to the specified value
   *
   * Usage:
   *
   * initialize(val);               // Initializes only the local space
   * initialize(val, true);         // Initializes the entire space
   * initialize(val, false);        // Initializes only the local space (default)
   * initialize(val, true, false);  // Initializes only the buffer zone
   * initialize(val, false, true);  // Initializes only the local space (default)
   * initialize(val, true, true);   // Initializes the entire space
   * initialize(val, false, false); // Does nothing
   *
   * @param initialValue Value to be placed in the array
   * @param fillBufferZone true if the value should be placed in all local cells
   * @param fillLocal true if the value should be placed in all non-local cells
   */
  virtual void initialize(T initialValue, bool fillBufferZone = false, bool fillLocal = true) = 0;

  /**
   * Initializes the array to the specified values
   *
   * initialize(val1, val2); // Initializes the local space to val1 and the buffer zones to val2
   * @param initialLocalValue value to be placed in local cells
   * @param initialBufferZoneValue value to be placed in non-local cells
   */
  virtual void initialize(T initialLocalValue, T initialBufferZoneValue) = 0;

  /**
   * Adds to the value in the grid at a specific location
   * Returns the new value. If the location is not within the local
   * boundaries, sets the error flag to 'true', otherwise it will be
   * set to 'false'
   *
   * @param val Value to be placed
   * @param location location where value is to be placed
   * @errFlag a flag that will be set to 'false' if an error occurs
   * @return the new value in the cell
   */
  virtual T addValueAt(T val, Point<int> location, bool& errFlag) = 0;

  /**
   * Adds to the value in the grid at the specific location
   * Returns the new value. If the location is not within the local
   * boundaries, sets the error flag to 'true', otherwise it will be
   * set to 'false'
   *
   * @param val Value to be added
   * @param location location where value is to be placed
   * @errFlag a flag that will be set to 'false' if an error occurs
   * @return the new value in the cell
   */
  virtual T addValueAt(T val, vector<int> location, bool& errFlag) = 0;

  /**
   * Sets the value in the grid at a specific location
   * Returns the new value. If the location is not within the local
   * boundaries, sets the error flag to 'true', otherwise it will be
   * set to 'false'
   *
   * @param val Value to be added
   * @param location location where value is to be placed
   * @errFlag a flag that will be set to 'false' if an error occurs
   * @return the new value in the cell
   */
  virtual T setValueAt(T val, Point<int> location, bool& errFlag) = 0;

  /**
   * Sets the value in the grid at the specific location
   * Returns the new value. If the location is not within the local
   * boundaries, sets the error flag to 'true', otherwise it will be
   * set to 'false'
   *
   * @param val Value to be added
   * @param location location where value is to be placed
   * @errFlag a flag that will be set to 'false' if an error occurs
   * @return the new value in the cell
   */
  virtual T setValueAt(T val, vector<int> location, bool& errFlag) = 0;

  /**
   * Gets the value in the grid at a specific location.  If the location is not within the
   * boundaries, sets the error flag to 'true', otherwise it will be
   * set to 'false'
   *
   * @param location location where value is to be placed
   * @errFlag a flag that will be set to 'false' if an error occurs
   * @return the value in the cell
   */
  virtual T getValueAt(Point<int> location, bool& errFlag) = 0;

  /**
   * Gets the value in the grid at a specific location.  If the location is not within the
   * boundaries, sets the error flag to 'true', otherwise it will be
   * set to 'false'
   *
   * @param location location where value is to be placed
   * @errFlag a flag that will be set to 'false' if an error occurs
   * @return the value in the cell
   */
  virtual T getValueAt(vector<int> location, bool& errFlag) = 0;

  /**
   * Synchronizes across processes. This copies
   * the values in the interior 'buffer zones' from
   * self and sends to adjacent processes, while
   * receiving data from adjacent processes and
   * placing it in the appropriate exterior buffer
   * zones.
   */
  virtual void synchronize() = 0;

private:

  /**
   * Gets an MPI data type given the RelativeLocation
   * and an index indicating which entry in the RelativeLocation
   * is being requested. Typical use will be recursive: if
   * there are N dimensions, this class will be called with
   * dimensionIndex = N - 1, which will call itself with N-2,
   * repeating until N = 0.
   *
   * @param relLoc a RelativeLocation object that describes the
   * shape of the requested MPI data type.
   * @datatype prototype for the datatype requested
   */
  void getMPIDataType(RelativeLocation relLoc, MPI_Datatype &datatype);

  /**
   * A variant of the getMPIDataType function, this
   * one assumes that you are retrieving a block with side
   * 2 x radius + 1 in all dimensions;
   *
   * @param radius size of the data type to retrieve
   * @prototype for the datatype requested
   */
  void getMPIDataType(int radius, MPI_Datatype &datatype);

  /**
   * Gets an MPI data type given the list of side lengths
   *
   *@param sideLengths lengths of each side of the datatype to be returned
   *@param datatype prototype for the datatype
   *@param dimensionIndex index number of the dimension (for
   *@param recursive calling from high dimensions down to 1
   */
  void getMPIDataType(vector<int> sideLengths, MPI_Datatype &datatype, int dimensionIndex);

  /**
   * Gets the raw MPI datatype from which all others are built
   * @return the raw MPI datatype for 'T' for this class
   */
  MPI_Datatype getRawMPIDataType();


  /**
   * Given a relative location, calculates the index value for the
   * first unit that should be in the 'send' buffer. Generally,
   * if the relative location value for a given dimension is
   * -1 or 0, the offset in that dimension should be equal to the
   * buffer zone width, and if it is 1, the offset should be equal
   * to the local width (technically buffer + local - buffer)
   * Note: Assumes RelativeLocation will only include values
   * of -1, 0, and 1
   *
   * @relLoc the RelativeLocation requested
   *
   * @return index of the cell at the specified relative location
   */
  int getSendPointerOffset(RelativeLocation relLoc);

  /**
   * Given a relative location, calculates the index value for the
   * first unit that should be in the 'receive' buffer. Generally,
   * if the relative location value for a given dimension is
   * -1, the offset should be zero; if it is 0, the offset should
   * be equal to the buffer width; and if it is 1, the offset
   * should be equal to the buffer width + the local width
   * (or, equivalently, the total width - buffer width)
   * Note: Assumes RelativeLocation will only include values
   * of -1, 0, and 1
   *
   * @relLoc the RelativeLocation requested
   *
   * @return index of the cell at the specified relative location
   */
  int getReceivePointerOffset(RelativeLocation relLoc);


};



template<typename T>
int AbstractValueLayerND<T>::instanceCount = 0;

template<typename T>
AbstractValueLayerND<T>::AbstractValueLayerND(vector<int> processesPerDim, GridDimensions globalBoundaries,int bufferSize, bool periodic): globalSpaceIsPeriodic(periodic), syncCount(0){
  instanceID = AbstractValueLayerND<T>::instanceCount;
  AbstractValueLayerND<T>::instanceCount++;
  cartTopology = RepastProcess::instance()->getCartesianTopology(processesPerDim, periodic);
  // Calculate the size to be used for the buffers
  numDims = processesPerDim.size();

  int rank = RepastProcess::instance()->rank();
  localBoundaries = cartTopology->getDimensions(rank, globalBoundaries);

  // First create the basic coordinate data per dimension
  length = 1;
  int val = 1;
  for(int i = 0; i < numDims; i++){
    DimensionDatum<T> datum(i, globalBoundaries, localBoundaries, bufferSize, periodic);
    length *= datum.width;
    dimensionData.push_back(datum);
    places.push_back(val);
    strides.push_back(val * sizeof(T));
    val *= dimensionData[i].width;
  }

  // Now create the rank-based data per neighbor
  RelativeLocation relLoc(numDims);
  RelativeLocation relLocTrimmed = cartTopology->trim(rank, relLoc); // Initialized to minima

  vector<int> myCoordinates;
  cartTopology->getCoordinates(rank, myCoordinates);

  neighborData = new RankDatum[relLoc.getMaxIndex()];
  neighborCount = 0;
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

template<typename T>
AbstractValueLayerND<T>::~AbstractValueLayerND(){
  delete[] neighborData; // Should Free MPI Datatypes first...
  delete[] requests;
}

template<typename T>
bool AbstractValueLayerND<T>::isInLocalBounds(vector<int> coords){
  for(int i = 0; i < numDims; i++){
    DimensionDatum<T>* datum = &dimensionData[i];
    if(!datum->isInLocalBounds(coords[i])) return false;
  }
  return true;
}

template<typename T>
bool AbstractValueLayerND<T>::isInLocalBounds(Point<int> location){
  return isInLocalBounds(location.coords());
}

template<typename T>
vector<int> AbstractValueLayerND<T>::getIndexes(vector<int> location, bool isSimplified){
  vector<int> ret;
  ret.assign(numDims, 0); // Make the right amount of space
  for(int i = 0; i < numDims; i++) ret[i] = dimensionData[i].getIndexedCoord(location[i], isSimplified);
  return ret;
}

template<typename T>
int AbstractValueLayerND<T>::getIndex(vector<int> location, bool isSimplified){
  vector<int> indexed = getIndexes(location, isSimplified);
  int val = 0;
  for(int i = numDims - 1; i >= 0; i--) val += indexed[i] * places[i];
  if(val < 0 || val > length) val = -1;
  return val;
}

template<typename T>
int AbstractValueLayerND<T>::getIndex(Point<int> location){
  return getIndex(location.coords());
}


template<typename T>
void AbstractValueLayerND<T>::getMPIDataType(RelativeLocation relLoc, MPI_Datatype &datatype){
  vector<int> sideLengths;
  for(int i = 0; i < numDims; i++) sideLengths.push_back(dimensionData[i].getSendReceiveSize(relLoc[i]));
  getMPIDataType(sideLengths, datatype, numDims - 1);
}

template<typename T>
void AbstractValueLayerND<T>::getMPIDataType(int radius, MPI_Datatype &datatype){
  vector<int> sideLengths;
  sideLengths.assign(numDims, 2 * radius + 1);
  getMPIDataType(sideLengths, datatype, numDims - 1);
}

template<typename T>
void AbstractValueLayerND<T>::getMPIDataType(vector<int> sideLengths, MPI_Datatype &datatype, int dimensionIndex){
  if(dimensionIndex == 0){
    MPI_Type_contiguous(sideLengths[dimensionIndex], getRawMPIDataType(), &datatype);
  }
  else{
    MPI_Datatype innerType;
    getMPIDataType(sideLengths, innerType, dimensionIndex - 1);
    MPI_Type_create_hvector(sideLengths[dimensionIndex], // Count
                     1,                                                                        // BlockLength: just one of the inner data type
                     strides[dimensionIndex],                                                  // Stride, in bytes
                     innerType,                                                                // Inner Datatype
                     &datatype);
  }
  // Commit?
  MPI_Type_commit(&datatype);
}


template<typename T>
int AbstractValueLayerND<T>::getSendPointerOffset(RelativeLocation relLoc){
  int ret = 0;
  for(int i = 0; i < numDims; i++){
    DimensionDatum<T>* datum = &dimensionData[i];
    ret += (relLoc[i] <= 0 ? datum->leftBufferSize : datum->width - (2 * datum->rightBufferSize)) * places[i];
  }
  return ret;
}

template<typename T>
int AbstractValueLayerND<T>::getReceivePointerOffset(RelativeLocation relLoc){
  int ret = 0;
  for(int i = 0; i < numDims; i++){
    DimensionDatum<T>* datum = &dimensionData[i];
    ret += (relLoc[i] < 0 ? 0 : (relLoc[i] == 0 ? datum->leftBufferSize : datum->width - datum->rightBufferSize)) * places[i];
  }
  return ret;
}


/**
 * The ValueLayerND class is an N-dimensional layer of
 * values.
 *
 * The most complex part of the ValueLayerND class is the
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
 * block of memory. It would be more convenient for the MPI
 * call if it did. However, the MPI specification indicates
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
 * starting pointer need be changed to switch from sending
 * to receiving.
 *
 * (It should be noted that MPI allows these data types to
 * 'match' if they are structurally compatible. They need
 * not actually be identical. So, consider a send/receive
 * pair where one of the pair is against the global
 * simulation boundaries but the other is not. The actual
 * pattern of loops and steps that creates the send will
 * be different from the pattern that creates the receive,
 * but MPI will recognize that these are 'matchable' and
 * will perform the communication.)
 *
 * The data type can be defined recursively using MPI's
 * HVector function for all types except the innermost,
 * which is a contiguous block of double values.
 *
 * The memory space allocated by this object includes
 * buffer zones on all 2N sides and all intercardinal
 * directions, even if the space is adjacent to a strict
 * boundary edge.
 */
template<typename T>
class ValueLayerND: public AbstractValueLayerND<T>{

private:
  T* dataSpace;              // Pointer to the data space

public:

  ValueLayerND(vector<int> processesPerDim, GridDimensions globalBoundaries, int bufferSize,
      bool periodic, T initialValue = 0, T initialBufferZoneValue = 0);
  virtual ~ValueLayerND();

  /**
   * Inherited from AbstractValueLayerND
   */
  virtual void initialize(T initialValue, bool fillBufferZone = false, bool fillLocal = true);

  /**
   * Inherited from AbstractValueLayerND
   */
  virtual void initialize(T initialLocalValue, T initialBufferZoneValue);

  /**
   * Inherited from AbstractValueLayerND
   */
  virtual T addValueAt(T val, Point<int> location, bool& errFlag);

  /**
   * Inherited from AbstractValueLayerND
   */
  virtual T addValueAt(T val, vector<int> location, bool& errFlag);

  /**
   * Inherited from AbstractValueLayerND
   */
  virtual T setValueAt(T val, Point<int> location, bool& errFlag);

  /**
   * Inherited from AbstractValueLayerND
   */
  virtual T setValueAt(T val, vector<int> location, bool& errFlag);

  /**
   * Inherited from AbstractValueLayerND
   */
  virtual T getValueAt(Point<int> location, bool& errFlag);

  /**
   * Inherited from AbstractValueLayerND
   */
  virtual T getValueAt(vector<int> location, bool& errFlag);

  /**
   * Inherited from AbstractValueLayerND
   */
  virtual void synchronize();

  /**
   * Write the values in this ValueLayer to a .csv file.
   *
   * The file format is:
   *
   * Dim_0,Dim_1,Dim_2,VAL
   * 0,0,0,100
   * 0,0,1,200
   *
   * The column header indicates the dimensionl the value in the 'VAL'
   * column is the value in the ValueLayer at the coordinates specified
   * by the values in the first N columns.
   *
   * In many common situations, many cells will contain zero; to
   * keep file sizes small, zeros are not written.
   *
   * @param fileLocation path to the file location for output
   * @param filetag infix for the file name
   * @writeSharedBoundaryAreas if true, the data output will include
   * the adjacent processes' buffer zones as they exist in this
   * array; if false, these will be omitted
   */
  void write(string fileLocation, string filetag, bool writeSharedBoundaryAreas = false);


private:


  /**
   * Fills a dimension of space with the given value. Used for initialization
   * and clearing only.
   *
   * @param localValue the value to be placed in local cells
   * @param bufferZoneValue the value to be placed in non-local cells
   * @param doBufferZone if true, places values in the buffer zone
   * @param doLocal if true, places values in the local cells
   * @param dataSpacePointer pointer to the first cell in the data array
   * @param dimIndex index number of this dimension, for recursive calls
   */
  void fillDimension(T localValue, T bufferZoneValue, bool doBufferZone, bool doLocal, T* dataSpacePointer, int dimIndex);

  /*
   * Writes one dimension's information to the specified csv file.
   *
   * @param outfile output file
   * @param dataSpacePointer pointer to the data space to be written
   * @param currentPosition position currently being written (for recursive calls)
   * @param dimIndex dimension currently being written (for recursive calls)
   * @param writeSharedBoundaryAreas if true, write the areas that are non-local to this process
   */
  void writeDimension(std::ofstream& outfile, T* dataSpacePointer, int* currentPosition, int dimIndex, bool writeSharedBoundaryAreas = false);

};





/**
 * ValueLayerNDSU is a version of the ValueLayerND class that
 * facilitates SynchronousUpdating: that is, a process can
 * use the current values in the value layer and create a set of new
 * values, then 'switch' to using the new values. It does this by using
 * two memory banks.
 */
template<typename T>
class ValueLayerNDSU: public AbstractValueLayerND<T>{

protected:

  T*                dataSpace1;             // Permanent pointer to bank 1 of the data space
  T*                dataSpace2;             // Permanent pointer to bank 2 of the data space
  T*                currentDataSpace;       // Temporary pointer to the active data space
  T*                otherDataSpace;         // Temporary pointer to the inactive data space

public:

  ValueLayerNDSU(vector<int> processesPerDim, GridDimensions globalBoundaries, int bufferSize, bool periodic, T initialValue = 0, T initialBufferZoneValue = 0);
  virtual ~ValueLayerNDSU();

  /**
   * Inherited from AbstractValueLayerND
   */
  virtual void initialize(T initialValue, bool fillBufferZone = false, bool fillLocal = true);

  /**
   * Inherited from AbstractValueLayerND
   */
  virtual void initialize(T initialLocalValue, T initialBufferZoneValue);

  /**
   * Inherited from AbstractValueLayerND
   */
  virtual T addValueAt(T val, Point<int> location, bool& errFlag);

  /**
   * Inherited from AbstractValueLayerND
   */
  virtual T addValueAt(T val, vector<int> location, bool& errFlag);

  /**
   * Inherited from AbstractValueLayerND
   */
  virtual T setValueAt(T val, Point<int> location, bool& errFlag);

  /**
   * Inherited from AbstractValueLayerND
   */
  virtual T setValueAt(T val, vector<int> location, bool& errFlag);

  /**
   * Inherited from AbstractValueLayerND
   */
  virtual T getValueAt(Point<int> location, bool& errFlag);

  /**
   * Inherited from AbstractValueLayerND
   */
  virtual T getValueAt(vector<int> location, bool& errFlag);

  /**
   * Inherited from AbstractValueLayerND
   */
  virtual void synchronize();

  /**
   * Write this rank's data to a CSV file
   */
  virtual void write(string fileLocation, string filetag, bool writeSharedBoundaryAreas = false);

  /**
   * Switch from one value layer to the other.
   */
  void switchValueLayer();

  /**
   * Adds the specified value to the value in the non-current
   * data bank at the given location
   *
   * @param val Value to be added
   * @param location location where value is to be placed
   * @errFlag a flag that will be set to 'false' if an error occurs
   * @return the new value in the cell
   */
  virtual T addSecondaryValueAt(T val, Point<int> location, bool& errFlag);

  /**
   * Adds the specified value to the value in the non-current
   * data bank at the given location
   *
   * @param val Value to be added
   * @param location location where value is to be placed
   * @errFlag a flag that will be set to 'false' if an error occurs
   * @return the new value in the cell
   */
  virtual T addSecondaryValueAt(T val, vector<int> location, bool& errFlag);

  /**
   * Sets the specified value to the value in the non-current
   * data bank at the given location
   *
   * @param val Value to be added
   * @param location location where value is to be placed
   * @errFlag a flag that will be set to 'false' if an error occurs
   * @return the new value in the cell
   */
  virtual T setSecondaryValueAt(T val, Point<int> location, bool& errFlag);

  /**
   * Sets the specified value to the value in the non-current
   * data bank at the given location
   *
   * @param val Value to be added
   * @param location location where value is to be placed
   * @errFlag a flag that will be set to 'false' if an error occurs
   * @return the new value in the cell
   */
  virtual T setSecondaryValueAt(T val, vector<int> location, bool& errFlag);

  /**
   * Gets the specified value to the value in the non-current
   * data bank at the given location
   *
   * @param location location where value is to be placed
   * @errFlag a flag that will be set to 'false' if an error occurs
   * @return the value in the cell
   */
  virtual T getSecondaryValueAt(Point<int> location, bool& errFlag);

  /**
   * Gets the specified value to the value in the non-current
   * data bank at the given location
   *
   * @param location location where value is to be placed
   * @errFlag a flag that will be set to 'false' if an error occurs
   * @return the new value in the cell
   */
  virtual T getSecondaryValueAt(vector<int> location, bool& errFlag);

  /**
   * Copies the data in the current value layer to the secondary layer
   */
  virtual void copyCurrentToSecondary();

  /**
   * Copies the data in the secondary layer to the current value layer
   */
  virtual void copySecondaryToCurrent();

  /**
   * ValueLayerNDSU can do something that no other value layer- or, in
   * fact, any other object in Repast HPC- can do: it can take information
   * from non-local processes and allow it to flow back to the local process.
   *
   * This is strictly forbidden in every other context in Repast HPC. Consider
   * a local process 0 with an Agent 'A' in one corner of its space. Because
   * 'A' is in the buffer zone, copies of 'A' are made on the other processes
   * adjacent to process 0; in a normal 2-D space this could include three
   * other processes, p1, p2, and p3. Now suppose that p1, p2, and p3 are all
   * allowed to modify their copies of Agent 'A', and then, further, that we
   * wish to migrate these changes back to process 0. How do we reconcile
   * the changes? In general there is no answer.
   *
   * ValueLayerNDSU, however, makes one special provision for allowing non-
   * local information to flow back to the local process. Assume that as in
   * the agent example, there is a corner of the value layer on p0 that is
   * adjacent to p1, p2, and p3. Copies of the space are made to the other
   * processes. Now assume that these other processes add values to these
   * regions of space. The simulation is responsible for assuring that these
   * additive operations are independent of the simulation synchronization:
   * that in the specific simulation being undertaken, the additions
   * that are occurring without synchronization are semantically acceptable.
   *
   * The 'flowback' method takes these values and performs what is akin to an
   * MPI 'gather' operation: taking the values from processes p1, p2, and p3 and
   * adding them to the values on p0. The result of the operation is that
   * the values on each process reflect the original values plus the values that
   * were found in the non-local processes, with all values summed.
   *
   * N.B.: All values in the secondary layer are set to zeros. Values on the original
   * process inside and outside the buffer zones are unchanged. To update
   * the values outside the buffer zones to their new values, perform a 'synchronize'
   * operation after the 'flowback' operation is completed.
   */
  void flowback();

private:

  /**
   * Fills a dimension of space with the given value. Used for initialization
   * and clearing only.
   *
   * @param localValue the value to be placed in local cells
   * @param bufferZoneValue the value to be placed in non-local cells
   * @param doBufferZone if true, places values in the buffer zone
   * @param doLocal if true, places values in the local cells
   * @param dataSpace1Pointer pointer to the first cell in the #1 data array
   * @param dataSpace2Pointer pointer to the first cell in the #2 data array
   * @param dimIndex index number of this dimension, for recursive calls
   */
  void fillDimension(T localValue, T bufferZoneValue, bool doBufferZone, bool doLocal, T* dataSpace1Pointer, T* dataSpace2Pointer, int dimIndex);

  /*
   * Writes one dimension's information to the specified csv file.
   *
   * @param outfile output file
   * @param dataSpacePointer pointer to the data space to be written
   * @param currentPosition position currently being written (for recursive calls)
   * @param dimIndex dimension currently being written (for recursive calls)
   * @param writeSharedBoundaryAreas if true, write the areas that are non-local to this process
   */
  void writeDimension(std::ofstream& outfile, T* dataSpacePointer, int* currentPosition, int dimIndex, bool writeSharedBoundaryAreas = false);

  /**
   * This is based on the 'fillDimension', but it takes values from the
   * LOCAL portion of the otherDataSpace and sums them into the LOCAL portion
   * of the current data space.
   */
  void sumInto(T* dataSpace1Pointer, T* dataSpace2Pointer, int dimIndex);

};



template<typename T>
ValueLayerND<T>::ValueLayerND(vector<int> processesPerDim, GridDimensions globalBoundaries, int bufferSize, bool periodic,
    T initialValue, T initialBufferZoneValue): AbstractValueLayerND<T>(processesPerDim, globalBoundaries, bufferSize, periodic){

  // Create the actual arrays for the data
  dataSpace = new T[AbstractValueLayerND<T>::length];

  // Finally, fill the data with the initial values
  initialize(initialValue, initialBufferZoneValue);

  // And synchronize
  synchronize();

}

template<typename T>
ValueLayerND<T>::~ValueLayerND(){
  delete[] dataSpace;
}

template<typename T>
void ValueLayerND<T>::initialize(T initialValue, bool fillBufferZone, bool fillLocal){
  fillDimension(initialValue, initialValue, fillBufferZone, fillLocal, dataSpace, AbstractValueLayerND<T>::numDims - 1);
}

template<typename T>
void ValueLayerND<T>::initialize(T initialLocalValue, T initialBufferZoneValue){
  fillDimension(initialLocalValue, initialBufferZoneValue, true, true, dataSpace, AbstractValueLayerND<T>::numDims - 1);
}

template<typename T>
T ValueLayerND<T>::addValueAt(T val, Point<int> location, bool& errFlag){
  errFlag = false;
  int indx = this->getIndex(location);
  if(indx == -1){
    errFlag = true;
    return val;
  }
  T* pt = &dataSpace[indx];
  return (*pt = *pt + val);
}

template<typename T>
T ValueLayerND<T>::addValueAt(T val, vector<int> location, bool& errFlag){
  errFlag = false;
  int indx = this->getIndex(location);
  if(indx == -1){
    errFlag = true;
    return val;
  }

  T* pt = &dataSpace[indx];
  return (*pt = *pt + val);
}

template<typename T>
T ValueLayerND<T>::setValueAt(T val, Point<int> location, bool& errFlag){
  errFlag = false;
  int indx = this->getIndex(location);
  if(indx == -1){
    errFlag = true;
    return val;
  }
  T* pt = &dataSpace[indx];
  return (*pt = val);
}

template<typename T>
T ValueLayerND<T>::setValueAt(T val, vector<int> location, bool& errFlag){
  errFlag = false;
  int indx = this->getIndex(location);
  if(indx == -1){
    errFlag = true;
    return val;
  }
  T* pt = &dataSpace[indx];
  return (*pt = val);
}

template<typename T>
T ValueLayerND<T>::getValueAt(vector<int> location, bool& errFlag){
  errFlag = false;
  int indx = this->getIndex(location);
  if(indx == -1){
    errFlag = true;
    return 0;
  }
  return dataSpace[indx];
}

template<typename T>
T ValueLayerND<T>::getValueAt(Point<int> location, bool& errFlag){
  errFlag = false;
  int indx = this->getIndex(location);
  if(indx == -1){
    errFlag = true;
    return 0;
  }
  return dataSpace[indx];
}

template<typename T>
void ValueLayerND<T>::synchronize(){
  AbstractValueLayerND<T>::syncCount++;
  if(AbstractValueLayerND<T>::syncCount > 9) AbstractValueLayerND<T>::syncCount = 0;
  int mpiTag = AbstractValueLayerND<T>::instanceID * 10 + AbstractValueLayerND<T>::syncCount;
  // Note: the syncCount and send/recv directions are used to create a unique tag value for the
  // mpi sends and receives. The tag value must be unique in two ways: first, successive calls to this
  // function must be different enough that they can't be confused. The 'syncCount' value is used to
  // achieve this, and it will loop from 0-9 and then repeat. The second, the tag must sometimes
  // differentiate between sends and receives that are going to the same rank. If a dimension
  // has only 2 processes but wrap-around borders, then one process may be sending to the other
  // process twice (once left and once right). The 'sendDir' and 'recvDir' values trap this

  // For each entry in neighbors:
  MPI_Status statuses[AbstractValueLayerND<T>::neighborCount * 2];
  for(int i = 0; i < AbstractValueLayerND<T>::neighborCount; i++){
    MPI_Isend(&dataSpace[AbstractValueLayerND<T>::neighborData[i].sendPtrOffset], 1, AbstractValueLayerND<T>::neighborData[i].datatype,
        AbstractValueLayerND<T>::neighborData[i].rank, 10 * (AbstractValueLayerND<T>::neighborData[i].sendDir + 1) + mpiTag, AbstractValueLayerND<T>::cartTopology->topologyComm, &AbstractValueLayerND<T>::requests[i]);
    MPI_Irecv(&dataSpace[AbstractValueLayerND<T>::neighborData[i].receivePtrOffset], 1, AbstractValueLayerND<T>::neighborData[i].datatype,
        AbstractValueLayerND<T>::neighborData[i].rank, 10 * (AbstractValueLayerND<T>::neighborData[i].recvDir + 1) + mpiTag, AbstractValueLayerND<T>::cartTopology->topologyComm, &AbstractValueLayerND<T>::requests[AbstractValueLayerND<T>::neighborCount + i]);
  }
  MPI_Waitall(AbstractValueLayerND<T>::neighborCount * 2, AbstractValueLayerND<T>::requests, statuses);
}


template<typename T>
void ValueLayerND<T>::write(string fileLocation, string fileTag, bool writeSharedBoundaryAreas){
  std::ofstream outfile;
  std::ostringstream stream;
  int rank = repast::RepastProcess::instance()->rank();
  stream << fileLocation << "ValueLayer_" << fileTag << "_" << rank << ".csv";
  std::string filename = stream.str();

  const char * c = filename.c_str();
  outfile.open(c, std::ios_base::trunc | std::ios_base::out); // it will not delete the content of file, will add a new line

  // Write headers
  for(int i = 0; i < AbstractValueLayerND<T>::numDims; i++) outfile << "DIM_" << i << ",";
  outfile << "VALUE" << endl;

  int* positions = new int[AbstractValueLayerND<T>::numDims];
  for(int i = 0; i < AbstractValueLayerND<T>::numDims; i++) positions[i] = 0;

  writeDimension(outfile, dataSpace, positions, AbstractValueLayerND<T>::numDims - 1, writeSharedBoundaryAreas);

  outfile.close();
}


template<typename T>
void ValueLayerND<T>::fillDimension(T localValue, T bufferValue, bool doBufferZone, bool doLocal, T* dataSpacePointer, int dimIndex){
  if(!doBufferZone && !doLocal) return;
  int bufferEdge = AbstractValueLayerND<T>::dimensionData[dimIndex].leftBufferSize;
  int localEdge  = bufferEdge + AbstractValueLayerND<T>::dimensionData[dimIndex].localWidth;
  int upperBound = localEdge + AbstractValueLayerND<T>::dimensionData[dimIndex].rightBufferSize;

  int pointerIncrement = AbstractValueLayerND<T>::places[dimIndex];


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

template<typename T>
void ValueLayerND<T>::writeDimension(std::ofstream& outfile, T* dataSpacePointer, int* currentPosition, int dimIndex, bool writeSharedBoundaryAreas){
  int bufferEdge = AbstractValueLayerND<T>::dimensionData[dimIndex].leftBufferSize;
  int localEdge  = bufferEdge + AbstractValueLayerND<T>::dimensionData[dimIndex].localWidth;
  int upperBound = localEdge + AbstractValueLayerND<T>::dimensionData[dimIndex].rightBufferSize;

  int pointerIncrement = AbstractValueLayerND<T>::places[dimIndex];
  int i = 0;
  for(; i < bufferEdge; i++){
    currentPosition[dimIndex] = i;
    if(writeSharedBoundaryAreas){
      if(dimIndex == 0){
        T val = *dataSpacePointer;
        if(val != 0){
          for(int j = 0; j < AbstractValueLayerND<T>::numDims; j++) outfile << (currentPosition[j] - AbstractValueLayerND<T>::dimensionData[j].leftBufferSize + AbstractValueLayerND<T>::dimensionData[j].localBoundariesMin) << ",";
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
        T val = *dataSpacePointer;
        if(val != 0){
          for(int j = 0; j < AbstractValueLayerND<T>::numDims; j++) outfile << (currentPosition[j] - AbstractValueLayerND<T>::dimensionData[j].leftBufferSize + AbstractValueLayerND<T>::dimensionData[j].localBoundariesMin) << ",";
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
        T val = *dataSpacePointer;
        if(val != 0){
          for(int j = 0; j < AbstractValueLayerND<T>::numDims; j++) outfile << (currentPosition[j] - AbstractValueLayerND<T>::dimensionData[j].leftBufferSize + AbstractValueLayerND<T>::dimensionData[j].localBoundariesMin) << ",";
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




template<typename T>
ValueLayerNDSU<T>::ValueLayerNDSU(vector<int> processesPerDim, GridDimensions globalBoundaries, int bufferSize, bool periodic,
    T initialValue, T initialBufferZoneValue): AbstractValueLayerND<T>(processesPerDim, globalBoundaries, bufferSize, periodic){

  // Create the actual arrays for the data
  dataSpace1 = new T[AbstractValueLayerND<T>::length];
  dataSpace2 = new T[AbstractValueLayerND<T>::length];
  currentDataSpace = dataSpace1;
  otherDataSpace   = dataSpace2;

  // Finally, fill the data with the initial values
  initialize(initialValue, initialBufferZoneValue);

  // And synchronize
  synchronize();

}

template<typename T>
ValueLayerNDSU<T>::~ValueLayerNDSU(){
  delete[] currentDataSpace;
  delete[] otherDataSpace;
}

template<typename T>
void ValueLayerNDSU<T>::initialize(T initialValue, bool fillBufferZone, bool fillLocal){
  fillDimension(initialValue, initialValue, fillBufferZone, fillLocal, dataSpace1, dataSpace2, AbstractValueLayerND<T>::numDims - 1);
}

template<typename T>
void ValueLayerNDSU<T>::initialize(T initialLocalValue, T initialBufferZoneValue){
  fillDimension(initialLocalValue, initialBufferZoneValue, true, true, dataSpace1, dataSpace2, AbstractValueLayerND<T>::numDims - 1);
}

template<typename T>
T ValueLayerNDSU<T>::addValueAt(T val, Point<int> location, bool& errFlag){
  int indx = this->getIndex(location);
  if(indx == -1) return nan("");
  T* pt = &currentDataSpace[indx];
  return (*pt = *pt + val);
}

template<typename T>
T ValueLayerNDSU<T>::addValueAt(T val, vector<int> location, bool& errFlag){
  int indx = this->getIndex(location);
  if(indx == -1) return nan("");
  T* pt = &currentDataSpace[indx];
  return (*pt = *pt + val);
}

template<typename T>
T ValueLayerNDSU<T>::setValueAt(T val, Point<int> location, bool& errFlag){
  int indx = this->getIndex(location);
  if(indx == -1) return nan("");
  T* pt = &currentDataSpace[indx];
  return (*pt = val);
}

template<typename T>
T ValueLayerNDSU<T>::setValueAt(T val, vector<int> location, bool& errFlag){
  int indx = this->getIndex(location);
  if(indx == -1) return nan("");
  T* pt = &currentDataSpace[indx];
  return (*pt = val);
}

template<typename T>
T ValueLayerNDSU<T>::getValueAt(Point<int> location, bool& errFlag){
  int indx = this->getIndex(location);
  if(indx == -1) return nan("");
  return currentDataSpace[indx];
}

template<typename T>
T ValueLayerNDSU<T>::getValueAt(vector<int> location, bool& errFlag){
  int indx = this->getIndex(location);
  if(indx == -1) return nan("");
  return currentDataSpace[indx];
}


template<typename T>
void ValueLayerNDSU<T>::synchronize(){
  AbstractValueLayerND<T>::syncCount++;
  if(AbstractValueLayerND<T>::syncCount > 9) AbstractValueLayerND<T>::syncCount = 0;
  int mpiTag = AbstractValueLayerND<T>::instanceID * 10 + AbstractValueLayerND<T>::syncCount;
  // Note: the syncCount and send/recv directions are used to create a unique tag value for the
  // mpi sends and receives. The tag value must be unique in two ways: first, successive calls to this
  // function must be different enough that they can't be confused. The 'syncCount' value is used to
  // achieve this, and it will loop from 0-9 and then repeat. The second, the tag must sometimes
  // differentiate between sends and receives that are going to the same rank. If a dimension
  // has only 2 processes but wrap-around borders, then one process may be sending to the other
  // process twice (once left and once right). The 'sendDir' and 'recvDir' values trap this

  // For each entry in neighbors:
  MPI_Status statuses[AbstractValueLayerND<T>::neighborCount * 2];
  for(int i = 0; i < AbstractValueLayerND<T>::neighborCount; i++){
    MPI_Isend(&currentDataSpace[AbstractValueLayerND<T>::neighborData[i].sendPtrOffset], 1, AbstractValueLayerND<T>::neighborData[i].datatype,
        AbstractValueLayerND<T>::neighborData[i].rank, 10 * (AbstractValueLayerND<T>::neighborData[i].sendDir + 1) + mpiTag, AbstractValueLayerND<T>::cartTopology->topologyComm, &AbstractValueLayerND<T>::requests[i]);
    MPI_Irecv(&currentDataSpace[AbstractValueLayerND<T>::neighborData[i].receivePtrOffset], 1, AbstractValueLayerND<T>::neighborData[i].datatype,
        AbstractValueLayerND<T>::neighborData[i].rank, 10 * (AbstractValueLayerND<T>::neighborData[i].recvDir + 1) + mpiTag, AbstractValueLayerND<T>::cartTopology->topologyComm, &AbstractValueLayerND<T>::requests[AbstractValueLayerND<T>::neighborCount + i]);
  }
  MPI_Waitall(AbstractValueLayerND<T>::neighborCount * 2, AbstractValueLayerND<T>::requests, statuses);
}

template<typename T>
void ValueLayerNDSU<T>::write(string fileLocation, string fileTag, bool writeSharedBoundaryAreas){
  std::ofstream outfile;
  std::ostringstream stream;
  int rank = repast::RepastProcess::instance()->rank();
  stream << fileLocation << "ValueLayer_" << fileTag << "_" << rank << ".csv";
  std::string filename = stream.str();

  const char * c = filename.c_str();
  outfile.open(c, std::ios_base::trunc | std::ios_base::out); // it will not delete the content of file, will add a new line

  // Write headers
  for(int i = 0; i < AbstractValueLayerND<T>::numDims; i++) outfile << "DIM_" << i << ",";
  outfile << "VALUE" << endl;

  int* positions = new int[AbstractValueLayerND<T>::numDims];
  for(int i = 0; i < AbstractValueLayerND<T>::numDims; i++) positions[i] = 0;

  writeDimension(outfile, currentDataSpace, positions, AbstractValueLayerND<T>::numDims - 1, writeSharedBoundaryAreas);

  outfile.close();
}

template<typename T>
void ValueLayerNDSU<T>::switchValueLayer(){
  // Switch the data banks
  T* tempDataSpace = currentDataSpace;
  currentDataSpace      = otherDataSpace;
  otherDataSpace        = tempDataSpace;
}

template<typename T>
T ValueLayerNDSU<T>::addSecondaryValueAt(T val, Point<int> location, bool& errFlag){
  errFlag = false;
  int indx = this->getIndex(location);
  if(indx == -1){
    errFlag = true;
    return val;
  }
  T* pt = &otherDataSpace[indx];
  return (*pt = *pt + val);
}

template<typename T>
T ValueLayerNDSU<T>::addSecondaryValueAt(T val, vector<int> location, bool& errFlag){
  errFlag = false;
  int indx = this->getIndex(location);
  if(indx == -1){
    errFlag = true;
    return val;
  }
  T* pt = &otherDataSpace[indx];
  return (*pt = *pt + val);
}

template<typename T>
T ValueLayerNDSU<T>::setSecondaryValueAt(T val, Point<int> location, bool& errFlag){
  errFlag = false;
  int indx = this->getIndex(location);
  if(indx == -1){
    errFlag = true;
    return val;
  }
  T* pt = &otherDataSpace[indx];
  return (*pt = val);
}

template<typename T>
T ValueLayerNDSU<T>::setSecondaryValueAt(T val, vector<int> location, bool& errFlag){
  errFlag = false;
  int indx = this->getIndex(location);
  if(indx == -1){
    errFlag = true;
    return val;
  }
  T* pt = &otherDataSpace[indx];
  return (*pt = val);
}

template<typename T>
T ValueLayerNDSU<T>::getSecondaryValueAt(Point<int> location, bool& errFlag){
  errFlag = false;
  int indx = this->getIndex(location);
  if(indx == -1){
    errFlag = true;
    return 0;
  }
  return otherDataSpace[indx];
}

template<typename T>
T ValueLayerNDSU<T>::getSecondaryValueAt(vector<int> location, bool& errFlag){
  errFlag = false;
  int indx = this->getIndex(location);
  if(indx == -1){
    errFlag = true;
    return 0;
  }
  return otherDataSpace[indx];
}

template<typename T>
void ValueLayerNDSU<T>::copyCurrentToSecondary(){
  T d = 0;
  memcpy(otherDataSpace, currentDataSpace, AbstractValueLayerND<T>::length * sizeof d);
}

template<typename T>
void ValueLayerNDSU<T>::copySecondaryToCurrent(){
  T d = 0;
  memcpy(currentDataSpace, otherDataSpace, AbstractValueLayerND<T>::length * sizeof d);
}

template<typename T>
void ValueLayerNDSU<T>::flowback(){
  std::cout << " valueLayer is executing flowback on rank " << repast::RepastProcess::instance()->rank() << std::endl;
  // Fill the other data space with zeros
  T d = 0;
  memset(otherDataSpace, 0, AbstractValueLayerND<T>::length * sizeof d);

  // Loop through the (3^N - 1)/2 directions

  // Note: TODO: Do all the sends as 'waitall', then process all the receives, with the final
  // sends waitall being the block before returning to simulation operation

  // For each direction:
  RelativeLocation relLoc(AbstractValueLayerND<T>::numDims);
  int listSize = AbstractValueLayerND<T>::neighborCount;
  std::cout << " valueLayer is executing flowback on rank " << repast::RepastProcess::instance()->rank() << " LIST SIZE = " << listSize << std::endl;
  MPI_Request* flowbackRequests = new MPI_Request[4]; // No more than four at a time (may be only 2)
  do{
    int LDir = relLoc.getIndex();
    int RDir = RelativeLocation::getReverseDirectionIndex(relLoc.getCurrentValue());
    // Need to create a send to the L and a send to the R, then matching receives
    // We create these in pairs because we can take a small advantage of non-blocking
    // sends but be assured that the L and R data are not going to interfere
    RankDatum* datum;


    int countOfRequests = 0;
    for(int i = 0; i < listSize; i++){ // To do: Shouldn't loop through this, but instead should index once
      datum = & AbstractValueLayerND<T>::neighborData[i];
      // Note: (!!!) Yes, we are using the 'send' pointer for the _receive_ and the 'receive' pointer for the send...
      if(datum->sendDir == LDir){
        MPI_Isend(&currentDataSpace[datum->receivePtrOffset], 1, datum->datatype,
                  datum->rank, 1001, AbstractValueLayerND<T>::cartTopology->topologyComm, &flowbackRequests[countOfRequests]);
        countOfRequests++;
      }
      else if(datum->sendDir == RDir){
        MPI_Isend(&currentDataSpace[datum->receivePtrOffset], 1, datum->datatype,
                  datum->rank, 2002, AbstractValueLayerND<T>::cartTopology->topologyComm, &flowbackRequests[countOfRequests]);
        countOfRequests++;
      }
      if(datum->recvDir == LDir){
        MPI_Irecv(&otherDataSpace[datum->sendPtrOffset], 1, datum->datatype,
                          datum->rank, 1001, AbstractValueLayerND<T>::cartTopology->topologyComm, &flowbackRequests[countOfRequests]);
        countOfRequests++;
      }
      else if(datum->recvDir == RDir){
        MPI_Irecv(&otherDataSpace[datum->sendPtrOffset], 1, datum->datatype,
                          datum->rank, 2002, AbstractValueLayerND<T>::cartTopology->topologyComm, &flowbackRequests[countOfRequests]);
        countOfRequests++;
      }
    }
    MPI_Status statuses[countOfRequests];
    // Perform the sends and receives
    std::cout << " valueLayer is executing flowback on rank " << repast::RepastProcess::instance()->rank() << " DOING WAITALL WITH LDIRECTION " << LDir << std::endl;
    MPI_Waitall(countOfRequests, flowbackRequests, statuses);
    std::cout << " valueLayer is executing flowback on rank " << repast::RepastProcess::instance()->rank() << " DONE WITH WAITALL... " << std::endl;
    // Copy the received data from the other data space into
    // the current data space, summing the values and clearing
    // the other data space to zeros
    sumInto(currentDataSpace, otherDataSpace, AbstractValueLayerND<T>::numDims - 1);

    relLoc.increment();
  }while(relLoc.validNonCenter()); // Note: STOP at 0,0,0,0...: because each location creates a send and a receive in both L and R, only need to do half

  delete[] flowbackRequests; // Cleanup

}

template<typename T>
void ValueLayerNDSU<T>::fillDimension(T localValue, T bufferValue, bool doBufferZone, bool doLocal, T* dataSpace1Pointer, T* dataSpace2Pointer, int dimIndex){
  if(!doBufferZone && !doLocal) return;
  int bufferEdge = AbstractValueLayerND<T>::dimensionData[dimIndex].leftBufferSize;
  int localEdge  = bufferEdge + AbstractValueLayerND<T>::dimensionData[dimIndex].localWidth;
  int upperBound = localEdge + AbstractValueLayerND<T>::dimensionData[dimIndex].rightBufferSize;

  int pointerIncrement = AbstractValueLayerND<T>::places[dimIndex];


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

template<typename T>
void ValueLayerNDSU<T>::writeDimension(std::ofstream& outfile, T* dataSpacePointer, int* currentPosition, int dimIndex, bool writeSharedBoundaryAreas){
  int bufferEdge = AbstractValueLayerND<T>::dimensionData[dimIndex].leftBufferSize;
  int localEdge  = bufferEdge + AbstractValueLayerND<T>::dimensionData[dimIndex].localWidth;
  int upperBound = localEdge + AbstractValueLayerND<T>::dimensionData[dimIndex].rightBufferSize;

  int pointerIncrement = AbstractValueLayerND<T>::places[dimIndex];
  int i = 0;
  for(; i < bufferEdge; i++){
    currentPosition[dimIndex] = i;
    if(writeSharedBoundaryAreas){
      if(dimIndex == 0){
        T val = *dataSpacePointer;
        if(val != 0){
          for(int j = 0; j < AbstractValueLayerND<T>::numDims; j++) outfile << (currentPosition[j] - AbstractValueLayerND<T>::dimensionData[j].leftBufferSize + AbstractValueLayerND<T>::dimensionData[j].localBoundariesMin) << ",";
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
        T val = *dataSpacePointer;
        if(val != 0){
          for(int j = 0; j < AbstractValueLayerND<T>::numDims; j++) outfile << (currentPosition[j] - AbstractValueLayerND<T>::dimensionData[j].leftBufferSize + AbstractValueLayerND<T>::dimensionData[j].localBoundariesMin) << ",";
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
        T val = *dataSpacePointer;
        if(val != 0){
          for(int j = 0; j < AbstractValueLayerND<T>::numDims; j++) outfile << (currentPosition[j] - AbstractValueLayerND<T>::dimensionData[j].leftBufferSize + AbstractValueLayerND<T>::dimensionData[j].localBoundariesMin) << ",";
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


template<typename T>
void ValueLayerNDSU<T>::sumInto(T* dataSpace1Pointer, T* dataSpace2Pointer, int dimIndex){
  //std::cout << " ON RANK " << repast::RepastProcess::instance()->rank() << " SUMINTO RUNNING WITH DIM INDEX OF " << dimIndex << std::endl;
  int bufferEdge = AbstractValueLayerND<T>::dimensionData[dimIndex].leftBufferSize;
  int localEdge  = bufferEdge + AbstractValueLayerND<T>::dimensionData[dimIndex].localWidth;
  //int upperBound = localEdge + AbstractValueLayerND<T>::dimensionData[dimIndex].rightBufferSize;

  int pointerIncrement = AbstractValueLayerND<T>::places[dimIndex];
  int leftPointerSkip  = pointerIncrement * AbstractValueLayerND<T>::dimensionData[dimIndex].leftBufferSize;
  //int rightPointerSkip = pointerIncrement * AbstractValueLayerND<T>::dimensionData[dimIndex].rightBufferSize;

  // Skip the left buffer zone
  dataSpace1Pointer += leftPointerSkip;
  dataSpace2Pointer += leftPointerSkip;

  // Loop local edge to local edge
  for(int i = bufferEdge; i < localEdge; i++){
    if(dimIndex == 0){
      //if(*dataSpace2Pointer > 0) std::cout << " ON RANK " << repast::RepastProcess::instance()->rank() << " SUMINTO FOUND VALUE OF " << *dataSpace2Pointer << " ADDING TO " << *dataSpace1Pointer << " at " << i << " dimIndex " << dimIndex << std::endl;
      *dataSpace1Pointer += *dataSpace2Pointer; // Add space 2's value into space 1
      *dataSpace2Pointer = 0;                   // Zero out space 2
    }
    else sumInto(dataSpace1Pointer, dataSpace2Pointer, dimIndex - 1); // Recursive call
    dataSpace1Pointer += pointerIncrement;
    dataSpace2Pointer += pointerIncrement;
  }

}







}

#endif /* VALUELAYERND_H_ */
