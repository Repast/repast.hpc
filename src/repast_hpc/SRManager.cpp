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
 * SRManager.cpp
 *
 *  Created on: 2 Dec 2011
 *      Author: JTM
 */

#include "SRManager.h"

#include <boost/mpi.hpp>

using namespace std;

SRManager::SRManager(boost::mpi::communicator* comm): _comm(comm){
  int s = _comm->size();
  mySend = new int[s];
  myRecv = new int[s];
  send = mySend;
  recv = myRecv;
  clear();
}

SRManager::SRManager(boost::mpi::communicator* comm, int* toSend, int* toRecv): _comm(comm), send(toSend), recv(toRecv){
  mySend = NULL;
  myRecv = NULL;
  if(recv == NULL){
    myRecv = new int[_comm->size()];
    recv = myRecv;
    int *r = recv;
    for(unsigned int i = _comm->size(); i != 0; i--, r++){ *r = 0; }
  }
}

SRManager::~SRManager(){
  delete [] mySend;
  delete [] myRecv;
}


void SRManager::mark(int pos){
  mySend[pos] = 1;
}

void SRManager::setVal(int pos, int val){
  mySend[pos] = val;
}

void SRManager::clear(){
  int *s = send;
  int *r = recv;
  for(unsigned int i = _comm->size(); i != 0; i--, s++, r++){ *s = 0; *r = 0; }
}

void SRManager::retrieveSources(){
  MPI_Alltoall(send, 1, MPI_INT, recv, 1, MPI_INT, (*_comm));
}

void SRManager::retrieveSources(std::vector<int>& sources){
  retrieveSources();
  const int  e = _comm->size();
  int *r = recv;
  for(int i = 0; i != e; i++, r++) if(*r != 0) sources.push_back(i);
}

void SRManager::retrieveSources(const std::vector<int>& targets, std::vector<int>& sources, int tag){
  std::vector<int>::const_iterator iEnd = targets.end();
  for(std::vector<int>::const_iterator iter = targets.begin(); iter != iEnd; iter++) send[*iter] = 1;
  retrieveSources(sources);
}
