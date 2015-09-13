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
 *  RelativeLocation.h
 *
 *  Created on: July 28, 2015
 *      Author: jtm
 */

#ifndef RELATIVELOCATION_H_
#define RELATIVELOCATION_H_

#include <vector>

using namespace std;

namespace repast {

/**
 * A RelativeLocation is a vector of integers that
 * express a relative location in a coordinate system; this
 * is generally used in an MPI Cartesian Topology. A vector
 * might look like:
 *
 *   [ -1, -1, 1 ]
 *
 * which would indicate one unit to the 'left' in the x dimension,
 * one to the 'left' in the y dimension, and one to the 'right'
 * in the z dimension.
 *
 * This class can be 'incremented' so that it advances
 * through a series of relative locations in a fixed and
 * predictable order. The boundaries can be customized
 * so that the range of possible values in any dimension
 * is settable; by default it is [-1, 1] (inclusive) in
 * all dimensions, meaning, roughly, 'left, center, right'.
 * However, different values can be used, e.g.:
 *
 * minima: [-2, -1, -2]
 * maxima: [1, 2, 3]
 *
 * This would describe a volume of space ranging from -2 to 1
 * in the x dimension, -1 to 2 in the y dimension, and -2 to 3
 * in the z dimension. A typical use when incrementing would
 * be to start at the lowest value [-2, -1, -2] and loop
 * through all possible variants:
 *
 *  [ -2, -1, -2 ]
 *  [ -1, -1, -2 ]
 *  [  0, -1, -2 ]
 *  [  1, -1, -2 ]
 *  [ -2,  0, -2 ]
 *  [ -1,  0, -2 ]
 *  [  0,  0, -2 ]
 *  [  1,  0, -2 ]
 *  [ -2,  1, -2 ]
 *  [ -1,  1, -2 ]
 *  [  0,  1, -2 ]
 *  [  1,  1, -2 ]
 *  [ -2,  2, -2 ]
 *  [ -1,  2, -2 ]
 *  [  0,  2, -2 ]
 *  [  1,  2, -2 ]
 *  [ -2, -1, -1 ]
 *  [ -1, -1, -1 ]
 *  [  0, -1, -1 ]
 *  [  1, -1, -1 ]
 *  ...
 *  [  0,  2,  3 ]
 *  [  1,  2,  3 ]
 */
class RelativeLocation{

public:
  /**
   * Assumes that the vector given can be reduced to a 'unit' vector
   * (in which all values are either -1, 0, or 1); returns the index
   * value of this vector assuming a RelativeLocation with
   * minima of -1, -,1 -1, ... and maxima of 1, 1, 1.
   */
  static int getDirectionIndex(vector<int> dirVec);

  /**
   * Assumes that the vector given can be reduced to a 'unit' vector
   * (in which all values are either -1, 0, or 1); returns the index
   * value of the negative of this vector (all values multiplied
   * by -1), assuming a RelativeLocation with
   * minima of -1, -,1 -1, ... and maxima of 1, 1, 1.
   */
  static int getReverseDirectionIndex(vector<int> dirVec);

private:
  vector<int> currentValue;
  vector<int> minima;
  vector<int> maxima;
  int         countOfDimensions;
  vector<int> places;
  int         maxIndex;           // Memoize
  int         indexOfCenter;      // Memoize

  void setPlaces();

public:

  /**
   * Default constructor; min values are all -1,
   * max values are all 1, for the specified number
   * of dimensions
   */
  RelativeLocation(int dimensions);

  /**
   * Constructor that takes specific minima and maxima.
   * Current Value is set to the minima
   * Note that if any maximum value is less than the
   * corresponding minimum value, it will be reset
   * to match the minimum value. If the minima and maxima
   * vectors are of different sizes, the shorter vector
   * size is used and the extra values in the longer
   * one are ignored.
   */
  RelativeLocation(vector<int> minima, vector<int> maxima);

  /**
   * Copy Constructor
   */
  RelativeLocation(const RelativeLocation& original);

  void translate(vector<int> displacement);

  virtual ~RelativeLocation();

  /*
   * Increments this RelativeLocation instance.
   * Returns false if the instance is already at its
   * maximum (but will also roll over to minimum values!)
   */
  bool increment();

  bool increment(bool skipZero);


  /**
   * Set the current value. Will return false
   * if the number of dimensions does not match the existing
   * value, or if any of the new values are out of the
   * ranges defined by the minima and maxima vectors.
   */
  bool set(vector<int> newValues);

  /**
   * Returns true if the current value for the other
   * instance is equal to this one; does NOT
   * compare minima or maxima. Note: if the two
   * instances are different sizes, only the
   * first N values are compared, where N is
   * the size of the shorter instance.
   */
  bool equals(RelativeLocation other);

  /**
   * Gets the array of current values
   */
  vector<int> getCurrentValue();

  /**
   * Gets the current value at the specified index
   */
  int operator[](int index);

  int getCountOfDimensions();

  int getMaxIndex();

  int getTotalValues();

  int getIndex(vector<int> value);

  int getIndex();

  int getIndexOfCenter();


  /**
   * Returns false when the values
   * are all zeroes, true otherwise
   */
  bool validNonCenter();

  int getMinimumAt(int index);
  int getMaximumAt(int index);

  /**
   * Returns a new RelativeLocation object
   * based on the one passed, but trimmed
   * so that it fits within the boundaries
   * of this one.
   *
   * If the result would be invalid (because the
   * one to be trimmed falls outside of this
   * one) the return value will be a RelativeLocation
   * with the original number of dimensions, but
   * all zeroes in the values.
   *
   * Trimming is done only on the first N dimensions,
   * where N is the smaller of the number of dimensions
   * in this or the passed RelativeLocation; extra
   * dimensions are ignored
   *
   */
  RelativeLocation trim(RelativeLocation toBeTrimmed);

  std::string report();
};

}

#endif /* DIFFUSIONLAYERND_H_ */
