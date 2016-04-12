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
 *  RelativeLocation.cpp
 *
 *  Created on: July 28, 2008
 *      Author: jtm
 */
#include <iostream>
#include <sstream>

#include "RelativeLocation.h"
#include "RepastProcess.h" // DEBUGGING ONLY

using namespace std;

namespace repast {


int RelativeLocation::getDirectionIndex(vector<int> dirVec){
  RelativeLocation baseline(dirVec.size());
  for(int i = 0; i < dirVec.size(); i++){
    int original = dirVec[i];
    dirVec[i] = original < 0 ? -1 : original == 0 ? 0 : 1;
  }
  return baseline.getIndex(dirVec);
}

int RelativeLocation::getReverseDirectionIndex(vector<int> dirVec){
  RelativeLocation baseline(dirVec.size());
  for(int i = 0; i < dirVec.size(); i++){
    int original = dirVec[i];
    dirVec[i] = original > 0 ? -1 : original == 0 ? 0 : 1;
  }
  return baseline.getIndex(dirVec);
}

void RelativeLocation::setPlaces(){
  int v = 1;
  for(size_t i = 0; i < countOfDimensions; i++){
    places.push_back(v);
    v *= (maxima[i] - minima[i] + 1);
  }
}

RelativeLocation::RelativeLocation(int dimensions): countOfDimensions(dimensions), maxIndex(-1), indexOfCenter(-1){
  for(int i = 0; i < dimensions; i++){
    currentValue.push_back(-1);
    minima.push_back(-1);
    maxima.push_back(1);
  }
  setPlaces();
}

RelativeLocation::RelativeLocation(vector<int> minimaVals, vector<int> maximaVals): maxIndex(-1), indexOfCenter(-1){
  countOfDimensions = minimaVals.size() < maximaVals.size() ? minimaVals.size() : maximaVals.size();
  for(int i = 0; i < countOfDimensions; i++){
    currentValue.push_back(minimaVals[i]);
    minima.push_back(minimaVals[i]);
    maxima.push_back(maximaVals[i] >= minimaVals[i] ? maximaVals[i]: minimaVals[i]);
  }
  setPlaces();
}

RelativeLocation::RelativeLocation(const RelativeLocation& original):
  countOfDimensions(original.countOfDimensions),
  currentValue(original.currentValue),
  minima(original.minima),
  maxima(original.maxima),
  places(original.places),
  maxIndex(original.maxIndex),
  indexOfCenter(original.indexOfCenter){}

void RelativeLocation::translate(vector<int> displacement){
  int otherSize = displacement.size();
  int dims = countOfDimensions < otherSize ? countOfDimensions : otherSize;
  for(int i = 0; i < dims; i++){
    minima[i]       += displacement[i];
    maxima[i]       += displacement[i];
    currentValue[i] += displacement[i];
  }
}


RelativeLocation::~RelativeLocation(){}

bool RelativeLocation::increment(){
  int i = 0;
  bool addNext = true;
  while((addNext) && (i < countOfDimensions)){
    currentValue[i] = currentValue[i] + 1;
    if(currentValue[i] <= maxima[i])   addNext = false;
    else                               currentValue[i] = minima[i];
    i++;
  }
  if(addNext == true){
    currentValue.clear();
    currentValue.insert(currentValue.begin(), minima.begin(), minima.end());
    return false;
  }
  else return true;
}

bool RelativeLocation::increment(bool skipZero){
  if(!skipZero) return increment(); // Silly to call this with 'false'
  int i = 0;
  bool addNext = true;
  while((addNext) && (i < countOfDimensions)){
    currentValue[i] = currentValue[i] + 1;
    if(currentValue[i] <= maxima[i])   addNext = false;
    else                               currentValue[i] = minima[i];
    i++;
  }
  if(addNext == true){ // Rolled over
    currentValue.clear();
    currentValue.insert(currentValue.begin(), minima.begin(), minima.end());
    return false;
  }
  else{
    return (validNonCenter() ? true : increment()); // If it's zero, increment again before returning
  }
}



bool RelativeLocation::set(vector<int> newValues){
  if(newValues.size() != countOfDimensions) return false;
  for(size_t i = 0; i < countOfDimensions; i++){
    if(newValues[i] < minima[i] || newValues[i] > maxima[i]) return false;
  }
  currentValue.clear();
  currentValue.insert(currentValue.begin(), newValues.begin(), newValues.end());
  return true;
}

bool RelativeLocation::equals(RelativeLocation other){
  size_t n = currentValue.size() <= other.currentValue.size() ? currentValue.size() : other.currentValue.size();
  for(size_t i = 0; i < n; i++) if(currentValue[i] != other.currentValue[i]) return false;
  return true;
}

vector<int> RelativeLocation::getCurrentValue(){
  return vector<int>(currentValue);
}

int RelativeLocation::operator[](int index){
  return (index > countOfDimensions || index < 0) ? 0 : currentValue[index];
}

int RelativeLocation::getCountOfDimensions(){
  return countOfDimensions;
}

int RelativeLocation::getMaxIndex(){
  if(maxIndex > -1) return maxIndex;
  int index = 1;
  for(size_t i = 0; i < minima.size(); i++) index *= (maxima[i] - minima[i] + 1);
  maxIndex = index - 1;
  return maxIndex;
}

int RelativeLocation::getTotalValues(){
  return getMaxIndex() + 1;
}

int RelativeLocation::getIndex(vector<int> value){
  if(value.size() != countOfDimensions) return -1;
  int index = 0;
  for(int i = countOfDimensions - 1; i >= 0; i--){
    if(value[i] < minima[i] || value[i] > maxima[i]) return -1; // Error
    index += (value[i] - minima[i]) * places[i];
  }
  return index;
}

int RelativeLocation::getIndex(){
  return getIndex(currentValue);
}

int RelativeLocation::getIndexOfCenter(){
  if(indexOfCenter != -1) return indexOfCenter;
  vector<int> center;
  center.assign(countOfDimensions, 0);
  indexOfCenter = getIndex(center);
  return indexOfCenter;
}

bool RelativeLocation::validNonCenter(){
  for(size_t i = 0; i < countOfDimensions; i++) if(currentValue[i] != 0) return true;
  return false;
}

int RelativeLocation::getMinimumAt(int index){
  return minima[index];
}

int RelativeLocation::getMaximumAt(int index){
  return maxima[index];
}

RelativeLocation RelativeLocation::trim(RelativeLocation toBeTrimmed){
  bool isValid = true;
  int dims = toBeTrimmed.countOfDimensions < countOfDimensions ? toBeTrimmed.countOfDimensions : countOfDimensions;
  vector<int> newMinima;
  vector<int> newMaxima;
  for(int i = 0; i < dims; i++){
    int originalMin = toBeTrimmed.getMinimumAt(i);
    int originalMax = toBeTrimmed.getMaximumAt(i);
    isValid = isValid && originalMax >= minima[i] && originalMin <= maxima[i];
    newMinima.push_back(originalMin >= minima[i] ? originalMin : minima[i]);
    newMaxima.push_back(originalMax <= maxima[i] ? originalMax : maxima[i]);
  }
  if(!isValid){
    vector<int> dummy;
    dummy.assign(toBeTrimmed.countOfDimensions, 0);
    RelativeLocation ret(dummy, dummy);
    return ret;
  }
  RelativeLocation ret(newMinima, newMaxima);
  return ret;
}

std::string RelativeLocation::report(){
  std::stringstream ret;
  for(int i = 0; i < countOfDimensions; i++) ret << currentValue[i] << (i < (countOfDimensions - 1) ? " " : "");
  return ret.str();
}
}
