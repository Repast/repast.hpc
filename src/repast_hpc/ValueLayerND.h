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
  int            recvDir;  // Integer representing the directtion a receive will have been sent, in N-space
};

/**
 * The DimensionDatum class stores all of the data that the
 * ValueLayerND class will need for each dimension in the
 * N-dimensional space. This mainly includes coordinate boundaries
 * along each dimension, but these boundaries include local,
 * global, and a few other memoized variants.
 */
class DimensionDatum{
public:
  DimensionDatum(int indx, GridDimensions globalBoundaries, GridDimensions localBoundaries, int buffer, bool isPeriodic);
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
  int  width;                                               // Total width (units = double) on this dimension (local extents + buffer sizes if not against global nonperiodic bounds)
  int  widthInBytes;

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
   */
  int getIndexedCoord(int originalCoord, bool isSimplified = false);

  /**
   * Returns true if the specified coordinate is within the local boundaries
   * on this dimension.
   */
  bool isInLocalBounds(int originalCoord);


  void report(int dimensionNumber){
    std::cout << repast::RepastProcess::instance()->rank() << " " << dimensionNumber << " " <<
                 "global (" << globalCoordinateMin     << ", " << globalCoordinateMax     << ") " <<
                 "local  (" << localBoundariesMin      << ", " << localBoundariesMax      << ") " <<
                 "simple (" << simplifiedBoundariesMin << ", " << simplifiedBoundariesMax << ") " <<
                 "match  (" << matchingCoordinateMin   << ", " << matchingCoordinateMax   << ") " <<
                 "globalWidth = " << globalWidth << " localWidth = " << localWidth << " width = " << width << " bytes = " << widthInBytes << std::endl;
  }
};



class AbstractValueLayerND{

protected:
  CartesianTopology*     cartTopology;
  int                    length;                 // Total length of the entire array (one data space)

  int                    numDims;                // Number of dimensions
  bool                   globalSpaceIsPeriodic;  // True if the global space is periodic

  vector<int>            places;                 // Multipliers to calculate index, for each dimension
  vector<int>            strides;                // Sizes of each dimensions, in bytes
  vector<DimensionDatum> dimensionData;          // List of data for each dimension
  RankDatum*             neighborData;           // List of data for each adjacent rank
  int                    neighborCount;          // Count of adjacent ranks
  MPI_Request*           requests;               // Pointer to MPI requests (for wait operations)


  AbstractValueLayerND(vector<int> processesPerDim, GridDimensions globalBoundaries,int bufferSize, bool periodic);
  virtual ~AbstractValueLayerND();


public:

  /**
   * Returns true only if the coordinates given are within the local boundaries
   */
  bool isInLocalBounds(vector<int> coords);

  /*
   * Returns true only if the coordinates given are within the local boundaries
   */
  bool isInLocalBounds(Point<int> location);


protected:
  // Methods implemented in this class but visible only to child classes:
  /**
   * Gets a vector of the indexed locations. If the
   * value passed is already simplified (transformed),
   * does not transform.
   */
  vector<int> getIndexes(vector<int> location, bool isSimplified = false);

  /**
   * Given a location in global simulation coordinates,
   * get the offset from the global base pointer to the
   * position in the global array representing that location.
   * The location may be simplified (transformed); if it is
   * not, it is first simplified before the index is calculated.
   */
  int getIndex(vector<int> location, bool isSimplified = false);


  /**
   * Given a location in global simulation coordinates,
   * get the offset from the global base pointer to the
   * position in the global array representing that location
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
   */
  virtual void initialize(double initialValue, bool fillBufferZone = false, bool fillLocal = true) = 0;

  /**
   * Initializes the array to the specified values
   *
   * initialize(val1, val2); // Initializes the local space to val1 and the buffer zones to val2
   */
  virtual void initialize(double initialLocalValue, double initialBufferZoneValue) = 0;

  /**
   * Add to the value in the grid at a specific location
   * Returns the new value.
   */
  virtual double addValueAt(double val, Point<int> location) = 0;

  /**
   * Add to the value in the grid at the specific location
   * Returns the new value.
   */
  virtual double addValueAt(double val, vector<int> location) = 0;

  /**
   * Add to the value in the grid at a specific location
   * Returns the new value.
   */
  virtual double setValueAt(double val, Point<int> location) = 0;

  /**
   * Add to the value in the grid at the specific location
   * Returns the new value.
   */
  virtual double setValueAt(double val, vector<int> location) = 0;

  /**
   * Gets the value in the grid at a specific location
   */
  virtual double getValueAt(Point<int> location) = 0;

  /**
   * Gets the value in the grid at a specific location
   */
  virtual double getValueAt(vector<int> location) = 0;

  /**
   * Synchronize across processes. This copies
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
   */
  void getMPIDataType(RelativeLocation relLoc, MPI_Datatype &datatype);

  /**
   * A variant of the getMPIDataType function, this
   * one assumes that you are retrieving a block with side
   * 2 x radius + 1 in all dimensions;
   */
  void getMPIDataType(int radius, MPI_Datatype &datatype);

  /**
   * Gets an MPI data type given the list of side lengths
   */
  void getMPIDataType(vector<int> sideLengths, MPI_Datatype &datatype, int dimensionIndex);


  /**
   * Given a relative location, calculates the index value for the
   * first unit that should be in the 'send' buffer. Generally,
   * if the relative location value for a given dimension is
   * -1 or 0, the offset in that dimension should be equal to the
   * buffer zone width, and if it is 1, the offset should be equal
   * to the local width (technically buffer + local - buffer)
   * Note: Assumes RelativeLocation will only include values
   * of -1, 0, and 1
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
   */
  int getReceivePointerOffset(RelativeLocation relLoc);


};





/**
 * The ValueLayerND class is an N-dimensional layer of
 * double values.
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
class ValueLayerND: public AbstractValueLayerND{

private:
  double*                dataSpace;              // Pointer to the data space

public:
  static int syncCount;

  ValueLayerND(vector<int> processesPerDim, GridDimensions globalBoundaries, int bufferSize,
      bool periodic, double initialValue = 0, double initialBufferZoneValue = 0);
  virtual ~ValueLayerND();

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
   * Write this rank's data to a CSV file
   */
  void write(string fileLocation, string filetag, bool writeSharedBoundaryAreas = false);


private:


  /**
   * Fills a dimension of space with the given value. Used for initialization
   * and clearing only.
   */
  void fillDimension(double localValue, double bufferZoneValue, bool doBufferZone, bool doLocal, double* dataSpacePointer, int dimIndex);

  /*
   * Writes one dimension's information to the specified csv file.
   */
  void writeDimension(std::ofstream& outfile, double* dataSpacePointer, int* currentPosition, int dimIndex, bool writeSharedBoundaryAreas = false);

};





/**
 * ValueLayerNDSU is a version of the ValueLayerND class that
 * facilitates SynchronousUpdating: that is, a process can
 * use the current values in the value layer and create a set of new
 * values, then 'switch' to using the new values. It does this by using
 * two memory banks. Some of the routines (i.e. initialization
 */
class ValueLayerNDSU: public AbstractValueLayerND{

protected:

  double*                dataSpace1;             // Permanent pointer to bank 1 of the data space
  double*                dataSpace2;             // Permanent pointer to bank 2 of the data space
  double*                currentDataSpace;       // Temporary pointer to the active data space
  double*                otherDataSpace;         // Temporary pointer to the inactive data space

public:
  static int syncCount;

  ValueLayerNDSU(vector<int> processesPerDim, GridDimensions globalBoundaries, int bufferSize, bool periodic, double initialValue = 0, double initialBufferZoneValue = 0);
  virtual ~ValueLayerNDSU();

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
   * Write this rank's data to a CSV file
   */
  void write(string fileLocation, string filetag, bool writeSharedBoundaryAreas = false);

private:

  /**
   * Fills a dimension of space with the given value. Used for initialization
   * and clearing only.
   */
  void fillDimension(double localValue, double bufferZoneValue, bool doBufferZone, bool doLocal, double* dataSpace1Pointer, double* dataSpace2Pointer, int dimIndex);

  /*
   * Writes one dimension's information to the specified csv file.
   */
  void writeDimension(std::ofstream& outfile, double* dataSpace1Pointer, int* currentPosition, int dimIndex, bool writeSharedBoundaryAreas = false);

};







}

#endif /* VALUELAYERND_H_ */
