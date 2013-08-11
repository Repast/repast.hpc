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
 *     Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *
 *     Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *
 *     Neither the name of the Argonne National Laboratory nor the names of its
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
 *  AgentSet.h
 *
 *  Created on: Jul 16, 2010
 *      Author: nick
 */

#ifndef AGENTSET_H_
#define AGENTSET_H_


#define USE_RANDOM_RELOGO_SET_OPS

#include <vector>
#include <algorithm>
#include <iostream>
#include <limits>

#include "repast_hpc/Random.h"

namespace repast {

namespace relogo {

/**
 * Compares two items using the specified getter.
 */
template<typename T, typename ValueGetter>
struct SetCmp {

	const ValueGetter* _getter;

	SetCmp(const ValueGetter* getter) :
		_getter(getter) {
	}

	bool operator()(T* one, T* two) {
		return (*_getter)(one) < (*_getter)(two);
	}
};

/**
 * Specialized indexable collection class for agents. This includes methods
 * designed to call arbitrary code on the agents it contains.
 *
 * @tparam T the type of agent the AgentSet contains
 */
template<typename T>
class AgentSet {

public:
	typedef typename std::vector<T*>::iterator as_iterator;
	typedef typename std::vector<T*>::const_iterator const_as_iterator;

	/**
	 * Creates an empty agent set.
	 */
	AgentSet();

	/**
	 * Creates an agent set and fills with elements from start through end.
	 */
	template<typename input_iterator>
	AgentSet(input_iterator start, input_iterator end);

	/**
	 * Copy constructor.
	 */
	AgentSet(const AgentSet& set);

	virtual ~AgentSet();

	/**
	 * Adds all the agents from the start iterator through the end to this
	 * AgentSet.
	 */
	template<typename input_iterator>
	void addAll(input_iterator begin, input_iterator end);

	/**
	 * Adds an agent to this AgentSet.
	 */
	void add(T* agent);

	/**
	 * Calls the Functor on each agent in this AgentSet.
	 *
	 * @param func a pointer to the method to call each member of the set
	 *
	 * @tparam Functor pointer to no-arg method belonging to the type of agent contained
	 * by this AgentSet.
	 */
	template<typename Functor>
	void ask(Functor func);

	/**
	 * Calls the Functor on each agent in this AgentSet, passing the specified argument
	 *
	 * @param func a pointer to the method to call each member of the set
	 * @param p1 a reference to a P1 type that is passed to the called method
	 *
	 * @tparam Functor pointer to method belonging to the type of agent contained
	 * by this AgentSet.
	 * @tparam P1 the type of the method parameter
	 */
	template<typename Functor, typename P1>
	void ask(Functor func, const P1& p1);

	/**
	 * Calls the Functor on each agent in this AgentSet, passing the specified argument
	 *
	 * @param func a pointer to the method to call each member of the set
	 * @param p1 a reference to a P1 type that is passed to the called method
	 * @tparam Functor pointer to method belonging to the type of agent contained
	 * by this AgentSet.
	 * @tparam P1 the type of the method parameter
	 */
	template<typename Functor, typename P1>
	void ask(Functor func, P1& p1);

	/**
	 * Applies the functor to each each agent in the agent set.
	 *
	 * @tparam Functor an object that implements operator()(T* agent);
	 */
	template<typename Functor>
	void apply(Functor& func);

	/**
	 * Applies the functor to each each agent in the agent set.
	 *
	 * @tparam Functor an object that implements operator()(T* agent);
	 */
	template<typename Functor>
	void apply(const Functor& func);

	/**
	 * Gets the item at the specified index.
	 *
	 * @param index the index of the agent to get
	 * @return the agent at the specified index
	 */
	T* at(int index);

	/**
	 * Gets the size of this AgentSet.
	 *
	 * @return the size of this AgentSet.
	 */
	size_t count() const {
		return agents.size();
	}

	/**
	 * Gets the size of this AgentSet.
	 *
	 * @return the size of this AgentSet.
	 */
	size_t size() const {
		return agents.size();
	}

	/**
	 * Gets the item at the specified index without doing any
	 * range checking.
	 *
	 * @param index the index of the agent to get
	 * @return the agent at the specified index
	 */
	T* operator[](size_t index);

	/**
	 * Gets an iterator to the begining of this AgentSet.
	 *
	 * @return an iterator to the beginning of this AgentSet.
	 */
	as_iterator begin() {
		return agents.begin();
	}

	/**
	 * Gets a const iterator to the begining of this AgentSet.
	 *
	 * @return a const iterator to the beginning of this AgentSet.
	 */
	const_as_iterator begin() const {
		return agents.begin();
	}

	/**
	 * Gets an iterator to the end of this AgentSet.
	 *
	 * @return an iterator to the end of this AgentSet.
	 */
	as_iterator end() {
		return agents.end();
	}

	/**
	 * Gets a const iterator to the end of this AgentSet.
	 *
	 * @return a const iterator to the end of this AgentSet.
	 */
	const_as_iterator end() const {
		return agents.end();
	}

	/**
	 * Clears this AgentSet of any agents that it contains.
	 */
	void clear() {
		agents.clear();
	}

	/**
	 * Gets the set member that has the minimum value of the number
	 * returned by ValueGetter. If more than one agent has the minimum
	 * value, then return one of those at random.
	 *
	 * @param getter the ValueGetter to use in retreiving the
	 * value used in the min comparison
	 *
	 * @tparam ValueGetter a function or functor that takes a
	 * member of this agentset and returns a double value. This
	 * double value is used in the min comparison.
	 */
	template<typename ValueGetter>
	T* minOneOf(const ValueGetter& getter);

	/**
	 * Gets the set member that has the maximum value of the number
	 * returned by ValueGetter. If more than one agent has the minimum
	 * value, then return one of those at random.
	 *
	 * @param getter the ValueGetter to use in retreiving the
	 * value used in the max comparison
	 *
	 * @tparam ValueGetter a function or functor that takes a
	 * member of this agentset and returns a double value. This
	 * double value is used in the max comparison.
	 */
	template<typename ValueGetter>
	T* maxOneOf(const ValueGetter& getter);

	/**
	 * Gets the set members that have the minimum value of the number
	 * returned by ValueGetter, and puts them in the specified set.
	 *
	 * @param getter the ValueGetter to use in retreiving the
	 * value used in the min comparison
	 *
	 * @tparam ValueGetter a function or functor that takes a
	 * member of this agentset and returns a double value. This
	 * double value is used in the min comparison.
	 */
	template<typename ValueGetter>
	void withMin(const ValueGetter& getter, AgentSet<T>& set);

	/**
	 * Gets the set members that have the maximum value of the number
	 * returned by ValueGetter and puts them in the specified set.
	 *
	 * @param getter the ValueGetter to use in retreiving the
	 * value used in the max comparison
	 *
	 * @tparam ValueGetter a function or functor that takes a
	 * member of this agentset and returns a double value. This
	 * double value is used in the max comparison.
	 */
	template<typename ValueGetter>
	void withMax(const ValueGetter& getter, AgentSet<T>& set);

	/**
	 * Gets count number of set members that have the minimum value of
	 * the number returned by ValueGetter. If there are not enough
	 * to satisfy the count then members with the second lowest
	 * value are returned and so on.
	 *
	 * @param getter the ValueGetter to use in retreiving the
	 * value used in the min comparison
	 *
	 * @tparam ValueGetter a function or functor that takes a
	 * member of this agentset and returns a double value. This
	 * double value is used in the min comparison.
	 * @param initialSetIsSorted Optional performance parameter; if
	 * false (the default), a call to this function must sort
	 * an entire copy of the original set; if true, the function
	 * assumes the original set is already sorted. Useful if the
	 * same set is to be used repeatedly.
	 */
	template<typename ValueGetter>
	void minNOf(size_t count, const ValueGetter& getter, AgentSet<T>& set, bool initialSetIsSorted = false);

	/**
	 * Gets count number of set members that have the maximum value of
	 * the number returned by ValueGetter. If there are not enough
	 * to satisfy the count then members with the second lowest
	 * value are returned and so on.
	 *
	 * @param getter the ValueGetter to use in retreiving the
	 * value used in the max comparison
	 *
	 * @tparam ValueGetter a function or functor that takes a
	 * member of this agentset and returns a double value. This
	 * double value is used in the max comparison.
   * @param initialSetIsSorted Optional performance parameter; if
   * false (the default), a call to this function must sort
   * an entire copy of the original set; if true, the function
   * assumes the original set is already sorted. Useful if the
   * same set is to be used repeatedly.
	 */
	template<typename ValueGetter>
	void maxNOf(size_t count, const ValueGetter& getter, AgentSet<T>& set, bool initialSetIsSorted = false);

	/**
	 * Gets one of the members of this AgentSet at random. If the set is
	 * empty, this returns 0.
	 */
	T* oneOf();

	/**
	 * Removes all instances of the specified agent from this AgentSet.
	 */
	void remove(T* agent);

	/**
	 * Randomly shuffles the elements of this AgentSet.
	 */
	void shuffle();

//private:
	std::vector<T*> agents;

};

template<typename T>
AgentSet<T>::AgentSet() {
}

template<typename T>
template<typename input_iterator>
AgentSet<T>::AgentSet(input_iterator start, input_iterator end) :
	agents(start, end) {
	std::random_shuffle(agents.begin(), agents.end(), uni_random);
}

template<typename T>
AgentSet<T>::AgentSet(const AgentSet& set) :
	agents(set.agents.begin(), set.agents.end()) {
}

template<typename T>
AgentSet<T>::~AgentSet() {
}

template<typename T>
void AgentSet<T>::shuffle() {
	std::random_shuffle(agents.begin(), agents.end(), uni_random);
}

template<typename T>
template<typename input_iterator>
void AgentSet<T>::addAll(input_iterator begin, input_iterator end) {
	agents.insert(agents.end(), begin, end);
}

template<typename T>
T* AgentSet<T>::operator[](size_t index) {
	return agents[index];
}

template<typename T>
void AgentSet<T>::add(T* agent) {
	agents.push_back(agent);
}

template<typename T>
T* AgentSet<T>::at(int index) {
	return agents.at(index);
}

template<typename T>
template<typename Functor>
void AgentSet<T>::apply(Functor& func) {
	for (as_iterator iter = agents.begin(); iter != agents.end(); ++iter) {
		func(*iter);
	}
}

template<typename T>
template<typename Functor>
void AgentSet<T>::apply(const Functor& func) {
	for (as_iterator iter = agents.begin(); iter != agents.end(); ++iter) {
		func(*iter);
	}
}

template<typename T>
template<typename Functor>
void AgentSet<T>::ask(Functor func) {
	for (as_iterator iter = agents.begin(); iter != agents.end(); ++iter) {
		T* target = *iter;
		(target->*func)();
	}
}

template<typename T>
template<typename Functor, typename P1>
void AgentSet<T>::ask(Functor func, const P1& p1) {
	for (as_iterator iter = agents.begin(); iter != agents.end(); ++iter) {
		T* target = *iter;
		(target->*func)(p1);
	}
}

template<typename T>
template<typename Functor, typename P1>
void AgentSet<T>::ask(Functor func, P1& p1) {
	for (as_iterator iter = agents.begin(); iter != agents.end(); ++iter) {
		T* target = *iter;
		(target->*func)(p1);
	}
}

template<typename T>
template<typename ValueGetter>
T* AgentSet<T>::minOneOf(const ValueGetter& getter) {
	if (agents.size() == 0)
		return 0;

	std::vector<T*> mins;
	double min = std::numeric_limits<double>::max();
	for (as_iterator iter = agents.begin(); iter != agents.end(); ++iter) {
		T* agent = *iter;
		double val = getter(agent);
		if (val < min) {
			mins.clear();
			mins.push_back(agent);
			min = val;
		} else if (val == min) {
			mins.push_back(agent);
		}
	}

	if (mins.size() > 1) {
		int index = (int) Random::instance()->createUniIntGenerator(0, mins.size() - 1).next();
		return mins[index];
	} else
	  return mins[0];
}

template<typename T>
template<typename ValueGetter>
T* AgentSet<T>::maxOneOf(const ValueGetter& getter) {
	if (agents.size() == 0)
		return 0;

	std::vector<T*> maxs;
	double max = -(std::numeric_limits<double>::max());
	for (as_iterator iter = agents.begin(); iter != agents.end(); ++iter) {
		T* agent = *iter;
		double val = getter(agent);
		if (val > max) {
			maxs.clear();
			maxs.push_back(agent);
			max = val;
		} else if (val == max) {
			maxs.push_back(agent);
		}
	}

	if (maxs.size() > 1) {
		int index = (int) Random::instance()->createUniIntGenerator(0, maxs.size() - 1).next();
		return maxs[index];
	} else
		return maxs[0];
}

template<typename T>
template<typename ValueGetter>
void AgentSet<T>::withMin(const ValueGetter& getter, AgentSet<T>& set) {
	set.clear();
	if (agents.size() == 0)
		return;

	double min = std::numeric_limits<double>::max();
	for (as_iterator iter = agents.begin(); iter != agents.end(); ++iter) {
		T* agent = *iter;
		double val = getter(agent);
		if (val < min) {
			set.clear();
			set.add(agent);
			min = val;
		} else if (val == min) {
			set.add(agent);
		}
	}
}

template<typename T>
template<typename ValueGetter>
void AgentSet<T>::withMax(const ValueGetter& getter, AgentSet<T>& set) {
  set.clear();
  if (agents.size() == 0)  return;
  //  double max = std::numeric_limits<double>::min();      // This gives smallest positive number
  double max = std::numeric_limits<double>::max() * (-1);   // CORRECTED
  for (as_iterator iter = agents.begin(); iter != agents.end(); ++iter) {
    T* agent = *iter;
    double val = getter(agent);
    if (val > max) {
      set.clear();
      set.add(agent);
      max = val;
    } else if (val == max) {
      set.add(agent);
    }
  }
}
template<typename T>
template<typename ValueGetter>
void AgentSet<T>::minNOf(size_t count, const ValueGetter& getter, AgentSet<T>& set, bool initialSetIsSorted) {
	set.clear();
	if (size() <= count) {
		set.addAll(begin(), end());
	} else {
		AgentSet<T> tmp(begin(), end());
		if(!initialSetIsSorted){
      SetCmp<T, ValueGetter> cmp(&getter);
      sort(tmp.begin(), tmp.end(), cmp);
		}
#ifdef USE_RANDOM_RELOGO_SET_OPS
		int numberToAdd = count;
		while(numberToAdd > 0){
		  double v1 = getter(tmp[0]);
		  int indx = 1;
		  while(getter(tmp[indx]) == v1) indx++;
		  if(indx <= numberToAdd){
		    for(size_t i = 0; i < indx; i++){
		      set.add(tmp[0]);
		      numberToAdd--;
		      tmp.remove(tmp[0]);
		    }
		  }
		  else{ // Must choose randomly from the remaining qualified entries
		    while(numberToAdd > 0){
          int i = (int) Random::instance()->createUniIntGenerator(0, indx - 1).next();
          set.add(tmp[i]);
          numberToAdd--;
          tmp.remove(tmp[i]);
          indx--;
		    }
		  }
		}
#else
		for (size_t i = 0; i < count; i++) {
			set.add(tmp[i]);
		}
#endif
	}
}

template<typename T>
template<typename ValueGetter>
void AgentSet<T>::maxNOf(size_t count, const ValueGetter& getter, AgentSet<T>& set, bool initialSetIsSorted) {
	set.clear();
	if (size() <= count) {
		set.addAll(begin(), end());
	} else {
		AgentSet<T> tmp(begin(), end());
		if(!initialSetIsSorted){
      SetCmp<T, ValueGetter> cmp(&getter);
      sort(tmp.begin(), tmp.end(), cmp);
		}
#ifdef USE_RANDOM_RELOGO_SET_OPS
		int numberToAdd = count;
		int endIndx = tmp.size() -1;
    while(numberToAdd > 0){
      double v1 = getter(tmp[endIndx]);
      int indx = endIndx - 1;
      while(getter(tmp[indx]) == v1) indx--;
      if((endIndx - indx) <= numberToAdd){
        while( endIndx > indx){
          set.add(tmp[endIndx]);
          numberToAdd--;
          tmp.remove(tmp[endIndx]);
          endIndx--;
        }
      }
      else{ // Must choose randomly from the remaining qualified entries
        while(numberToAdd > 0){
          int i = (int) Random::instance()->createUniIntGenerator(indx + 1, endIndx).next();
          set.add(tmp[i]);
          numberToAdd--;
          tmp.remove(tmp[i]);
          endIndx--;
        }
      }
    }
#else
		for (size_t i = tmp.size() - count; i < tmp.size(); i++) {
			set.add(tmp[i]);
		}
#endif
	}
}

template<typename T>
T* AgentSet<T>::oneOf() {
	size_t size = agents.size();
	if (size == 0)
		return 0;
	int index = (int) Random::instance()->createUniIntGenerator(0, size - 1).next();
	return agents[index];
}

template<typename T>
void AgentSet<T>::remove(T* agent) {
	typename std::vector<T*>::iterator iter = agents.begin();
	while (iter != agents.end()) {
		iter = std::find(iter, agents.end(), agent);
		if (iter != agents.end())
			iter = agents.erase(iter);
	}
}

}

}

#endif /* AGENTSET_H_ */
