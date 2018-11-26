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
 *  WeightedRandomSelector.h
 *
 *  Created on: 12 June 2015
 *      Author: murphy
 */

#ifndef WEIGHTEDRANDOMSELECTOR_H_
#define WEIGHTEDRANDOMSELECTOR_H_

#include <map>

#include "Random.h"

namespace repast {

template<typename T>
class WeightedRandomSelector{

private:

  static bool dblComp (double lhs, double rhs) { return lhs > rhs; } // Note: we want these sorted in reverse order

  std::multimap<double, T*, bool(*)(double, double)>* scoresAndObjects;
  std::set<T*>*                                       contentSet;
  double                                              total;

public:
  WeightedRandomSelector();

  virtual ~WeightedRandomSelector();

  void set(T* instance, double score);

  double remove(T* instance);

  bool contains(T* instance);

  T* getRandomInstance();

  void clear();

  size_t size();

  void report();

};

template<typename T>
WeightedRandomSelector<T>::WeightedRandomSelector(){
  scoresAndObjects = new std::multimap<double, T*, bool(*)(double, double)>(dblComp);
  contentSet       = new std::set<T*>();
  total            = 0;
};

template<typename T>
WeightedRandomSelector<T>::~WeightedRandomSelector(){
  delete scoresAndObjects;
  delete contentSet;
}

template<typename T>
void WeightedRandomSelector<T>::set(T* instance, double score){
  remove(instance);
  if(score <= 0) return; // Setting to zero removes from set
  scoresAndObjects->emplace(score, instance);
  contentSet->emplace(instance);
  total += score;
}

template<typename T>
double WeightedRandomSelector<T>::remove(T* instance){
  if(contentSet->find(instance) != contentSet->end()){
    contentSet->erase(instance);
    for(typename std::multimap<double, T*>::iterator iter = scoresAndObjects->begin(); iter != scoresAndObjects->end(); iter++){
      if(iter->second == instance){
        double score = iter->first;
        total -= score;
        scoresAndObjects->erase(iter);
        return score;
      }
    }
  }
  return 0;
}

template<typename T>
bool WeightedRandomSelector<T>::contains(T* instance){
  return contentSet->find() != contentSet->end();
}

template<typename T>
T* WeightedRandomSelector<T>::getRandomInstance(){
  if(scoresAndObjects->size() == 0) return 0;
  double val = Random::instance()->nextDouble() * total;
  double sum = 0;
  for(typename std::multimap<double, T*>::iterator iter = scoresAndObjects->begin(); iter != scoresAndObjects->end(); iter++){
    sum += iter->first;
    if(sum > val) return iter->second;
  }
  // Can't happen, but if it did, return the last entry
  typename std::multimap<double, T*>::iterator iter = scoresAndObjects->end();
  iter--;
  return iter->second;
}

template<typename T>
void WeightedRandomSelector<T>::clear(){
  scoresAndObjects->clear();
  contentSet->clear();
  total = 0;
}

template<typename T>
size_t WeightedRandomSelector<T>::size(){
  return contentSet->size();
}

template<typename T>
void WeightedRandomSelector<T>::report(){
  for(typename std::multimap<double, T*>::iterator iter = scoresAndObjects->begin(); iter != scoresAndObjects->end(); iter++){
    std::cout << "   " << *(iter->second) << " == " << iter->first << std::endl;
  }
}


}

#endif /* AGENTID_H_ */
