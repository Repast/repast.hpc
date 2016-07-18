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

#include "mpi.h"

#include "Point.h"
#include "GridDimensions.h"
#include "RepastProcess.h"


using namespace std;

namespace repast {

/**
 * The RankDatum struct stores the data that the DiffusionLayerND
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
 * DiffusionLayerND class will need for each dimension in the
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
  // No constructor
  virtual ~AbstractValueLayerND() = 0;

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
   * Returns true only if the coordinates given are within the local boundaries
   */
  virtual bool isInLocalBounds(vector<int> coords) = 0;

  /*
   * Returns true only if the coordinates given are within the local boundaries
   */
  virtual bool isInLocalBounds(Point<int> location) = 0;

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


  /**
   * Write this rank's data to a CSV file
   */
  virtual void write(string fileLocation, string filetag, bool writeSharedBoundaryAreas = false) = 0;

};


}

#endif /* VALUELAYERND_H_ */
