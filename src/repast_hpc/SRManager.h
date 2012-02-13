/*
 *Repast for High Performance Computing (Repast HPC)
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
 * SRManager.h
 *
 *  Created on: 2 Dec 2011
 *      Author: JTM
 */

#ifndef SRMANAGER_H_
#define SRMANAGER_H_

#include <vector>

#include <boost/mpi/communicator.hpp>

/**
 * Coordinates send and receive between processes
 * by notifying processes to expect a send from
 * X other processes.
 */
class SRManager{

private:
  boost::mpi::communicator* _comm;
  int *send;
  int *recv;
  int *mySend;
  int *myRecv;

public:
  /**
   * Creates an SRManager that uses the specified communicator.
   *
   * @param comm the communicator to use
   */
  SRManager(boost::mpi::communicator* comm);

  /**
   * Creates an SRManager that uses the specified communicator,
   * using the user-specified arrays instead of its internal arrays.
   * The ability to use external arrays is a convenience for
   * conditions in which it is useful to maintain the array
   * of values for other purposes but exchange them using the SRManager.
   *
   * @param comm the communicator to use
   * @param toSend the array to be used as the send array
   * @param toRecv the array to be used as the receive array
   *   If the pointer passed for the receive array is null, an internal
   *   array will be used. This is to provide for situations in which
   *   the user wishes to maintain the send array but not the receive
   *   array.
   */
  SRManager(boost::mpi::communicator* comm, int* toSend, int* toRecv);

  ~SRManager();

  /**
   * Marks the position in the array as 'true' (sets to one).
   *
   * @param pos the position in the array to be set, AKA the processor
   * to which information will be sent.
   */
  void mark(int pos);

  /**
   * Sets the value at the given index in the array. Note: Does not perform error
   * checking; user should ensure that index value is valid.
   *
   * @param pos index value for position in array to be set
   * @param val value to which the array element should be set
   */
  void setVal(int pos, int val);

  /**
   * Clears the send and receive arrays (sets all values to 0).
   */
  inline void clear();

  /**
   * Performs the actual send operation, populating the receive array with
   * values from the other processes' send arrays.
   */
  void retrieveSources();


  /**
   * Performs the send operation and populates the vector passed with
   * values representing all elements in the array that have non-zero
   * values after the receive.
   *
   * @param sources vector that will have a list of all processes that
   * sent non-zero values to this one
   */
  void retrieveSources(std::vector<int>& sources);

  /**
   * Populates the send array based on the values listed in the 'targets'
   * vector (which should be a list of process IDs to which this processer
   * will send information) then performs the send operation, then populates
   * the vector passed with values representing all elements in the receive
   * array that have non-zero values after the receive.
   *
   * @param targets vector of integers representing process to which this one
   * intends to send information
   * @param sources vector that will be populated with list of integers representing
   * processes that will send this process information
   * @tag optional parameter, now obsolete (included for backward compatibility with
   * boost-based SRManager prior to 2.0 release.
   */
  void retrieveSources(const std::vector<int>& targets, std::vector<int>& sources, int tag = 0);

};



#endif /* SRMANAGER_H_ */
