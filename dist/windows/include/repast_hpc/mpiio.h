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
*/

/*
 * mpiio.h
 *
 *  Created on: Sep 17, 2009
 *      Author: nick
 */

#ifndef MPIIO_H_
#define MPIIO_H_

#include <vector>
#include <boost/mpi/communicator.hpp>
#include <boost/serialization/vector.hpp>

/**
 * Coordinates send and receive between processes
 * by notifying processes to expect a send from
 * X other processes.
 *
 * Idea is that processes that need to send to other processes send a list of those processes
 * to P0. P0 then sorts the list of the processes to send to and then sends out
 * a list of processes that each process can expect to receive from.
 */
class SRManager {

private:
	boost::mpi::communicator _comm;
	int rank, worldSize;

public:

	/**
	 * Creates an SRManager that uses the specified communicator.
	 *
	 * @param comm the communicator to use
	 */
	SRManager(boost::mpi::communicator comm);

	/**
	 * Given a list of targets that this process wants to send to, retreive
	 * a list of sources that this target will receive from.
	 *
	 * @param tag the mpi send / recv tag.
	 */
	void retrieveSources(const std::vector<int>& targets, std::vector<int>& sources, int tag);

};


#endif /* MPIIO_H_ */
