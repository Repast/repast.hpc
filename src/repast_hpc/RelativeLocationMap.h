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
 *  RelativeLocationMap.h
 *
 *  Created on: July 28, 2015
 *      Author: jtm
 */

#ifndef RELATIVELOCATIONMAP_H_
#define RELATIVELOCATION_H_

#include "RelativeLocation.h"
using namespace std;

namespace repast {

template<typename T>
class RelativeLocationMap{
private:
  vector<T>* data;
  int size;

public:
  RelativeLocationMap(RelativeLocation location);
  virtual ~RelativeLocationMap();

  T&   get(int key);
  T&   get(RelativeLocation location);
  T&   operator[](RelativeLocation location);
  int getSize();
};

template<typename T>
RelativeLocationMap<T>::RelativeLocationMap(RelativeLocation location){
  size = location.getTotalValues();
  data = new vector<T>(size);
}

template<typename T>
RelativeLocationMap<T>::~RelativeLocationMap(){
  delete data;
}

template<typename T>
T& RelativeLocationMap<T>::get(int key){
  if(key < 0 || key > size) return 0;
  return data[key];
}

template<typename T>
T& RelativeLocationMap<T>::RelativeLocationMap<T>::get(RelativeLocation location){
  return get(location.getIndex());
}

template<typename T>
T& RelativeLocationMap<T>::operator[](RelativeLocation location){
  return data[location.getIndex()];
}

template<typename T>
int RelativeLocationMap<T>::getSize(){
  return size;
}

}

#endif /* DIFFUSIONLAYERND_H_ */
