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
 *  Random.h
 *
 *  Created on: Jun 9, 2009
 *      Author: nick
 */

#ifndef RANDOM_H_
#define RANDOM_H_

#include <vector>
#include <set>
#include <map>
#include <string>


#include <boost/random/variate_generator.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/uniform_real.hpp>
#include <boost/random/triangle_distribution.hpp>
#include <boost/random/cauchy_distribution.hpp>
#include <boost/random/exponential_distribution.hpp>
#include <boost/random/geometric_distribution.hpp>
#include <boost/random/normal_distribution.hpp>
#include <boost/random/lognormal_distribution.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/cstdint.hpp>


namespace repast {

typedef boost::variate_generator<boost::mt19937&, boost::uniform_real<> > _RealUniformGenerator;
typedef boost::variate_generator<boost::mt19937&, boost::uniform_int<> > _IntUniformGenerator;

typedef boost::variate_generator<boost::mt19937&, boost::triangle_distribution<> > _TriangleGenerator;
typedef boost::variate_generator<boost::mt19937&, boost::cauchy_distribution<> > _CauchyGenerator;
typedef boost::variate_generator<boost::mt19937&, boost::exponential_distribution<> > _ExponentialGenerator;
typedef boost::variate_generator<boost::mt19937&, boost::geometric_distribution<boost::uniform_real<> > >
		_GeometricGenerator;
typedef boost::variate_generator<boost::mt19937&, boost::normal_distribution<> > _NormalGenerator;
typedef boost::variate_generator<boost::mt19937&, boost::lognormal_distribution<> > _LogNormalGenerator;

/**
 * Number generator interface.
 */
class NumberGenerator {

public:
	virtual ~NumberGenerator() {
	}

	/**
	 * Gets the "next" number from this
	 * Number Generator.
	 */
	virtual double next() = 0;
};

/**
 * Adapts the templated boost::variate_generator to the
 * NumberGenerator interface.
 */
template<typename T>
class DefaultNumberGenerator: public NumberGenerator {

private:
	T gen;

public:
	DefaultNumberGenerator(T generator);
	double next();
};

template<typename T>
DefaultNumberGenerator<T>::DefaultNumberGenerator(T generator) :
	gen(generator) {
}

template<typename T>
double DefaultNumberGenerator<T>::next() {
	return gen();
}

typedef DefaultNumberGenerator<_IntUniformGenerator> IntUniformGenerator;
typedef DefaultNumberGenerator<_RealUniformGenerator> DoubleUniformGenerator;
typedef DefaultNumberGenerator<_TriangleGenerator> TriangleGenerator;
typedef DefaultNumberGenerator<_CauchyGenerator> CauchyGenerator;
typedef DefaultNumberGenerator<_ExponentialGenerator> ExponentialGenerator;
typedef DefaultNumberGenerator<_NormalGenerator> NormalGenerator;
typedef DefaultNumberGenerator<_LogNormalGenerator> LogNormalGenerator;

/**
 * Methods for working with random distributions, draws etc.
 */
class Random {

private:
	static Random* instance_;

	boost::uint32_t _seed;
	boost::mt19937 rng;
	boost::uniform_real<> uni10;
	_RealUniformGenerator uniGen;

	std::map<std::string, NumberGenerator*> generators;

protected:
	Random(boost::uint32_t seed);

public:

	/**
	 * Initialize the Random singleton with the specified seed.
	 *
	 * @param the seed to initialize the random number generator with.
	 */
	static void initialize(boost::uint32_t seed);

	/**
	 * Gets the singleton instance of this Random.
	 */
	static Random* instance();
	virtual ~Random();

	/**
	 * Puts the named generator into this Random. Added
	 * generators will be deleted by Random when it is destroyed.
	 *
	 * @param the id of the generator
	 * @param generator the generator to add
	 */
	void putGenerator(const std::string& id, NumberGenerator* generator);

	/**
	 * Gets the named generator or 0 if the name
	 * is not found.
	 *
	 * @param id the name of the generator to get
	 */
	NumberGenerator* getGenerator(const std::string& id);

	/**
	 * Gets the random number engine from which the distributions are created.
	 *
	 * @return he random number engine from which the distributions are created.
	 */
	boost::mt19937& engine() {
		return rng;
	}

	/**
	 * Gets the current seed.
	 *
	 * @return the current seed.
	 */
	boost::uint32_t seed() {
		return _seed;
	}

	/**
	 * Gets the next double in the range [0, 1).
	 *
	 * @return the next double in the range [0, 1).
	 */
	double nextDouble();

	/**
	 * Creates a generator that produces doubles in the range [from, to).
	 *
	 * @param from the range start (inclusive)
	 * @param to the range end (exclusive)
	 *
	 * @return a generator that produces doubles in the range [from, to).
	 */
	DoubleUniformGenerator createUniDoubleGenerator(double from, double to);

	/**
	 * Creates a generator that produces ints in the range [from, to].
	 *
	 *  @param from the range start (inclusive)
	 * @param to the range end (inclusive)
	 *
	 * @return a generator that produces ints in the range [from, to].
	 */
	IntUniformGenerator createUniIntGenerator(int from, int to);

	/**
	 * Creates a triangle generator with the specified properties. A TriangleGenerator produces a
	 * floating point value x where lowerbound <= x <= upperBound and mostLikely is the most
	 * probable value for x.
	 *
	 * @param lowerBound the lower bound of the values produced by the generator
	 * @param mostLikely the most likely value produced by the generator
	 * @param upperBound the upper bound of the values produced by the generator
	 *
	 * @return a triangle generator.
	 */
	TriangleGenerator createTriangleGenerator(double lowerBound, double mostLikely, double upperBound);

	/**
	 * pdf: p(x) = sigma/(pi*(sigma**2 + (x-median)**2))
	 *
	 * @param median
	 * @param sigma
	 *
	 * @return a Cauchy generator.
	 */
	CauchyGenerator createCauchyGenerator(double median, double sigma);

	/**
	 * pdf: p(x) = lambda * exp(-lambda * x)
	 *
	 * @param lambda must be > 0
	 *
	 * @return an exponential generator.
	 */
	ExponentialGenerator createExponentialGenerator(double lambda);

	/**
	 * Creates a normal generator.
	 *
	 * pdf: p(x) = 1/sqrt(2*pi*sigma) * exp(- (x-mean)2 / (2*sigma2) )
	 *
	 *
	 */
	NormalGenerator createNormalGenerator(double mean, double sigma);

	/**
	 * Produces random numbers with p(x) = 1/(x * normal_sigma * sqrt(2*pi)) * exp( -(log(x)-normal_mean)2 / (2*normal_sigma2) )
	 * for x > 0, where normal_mean = log(mean2/sqrt(sigma2 + mean2)) and normal_sigma = sqrt(log(1 + sigma2/mean2))
	 */
	LogNormalGenerator createLogNormalGenerator(double mean, double sigma);
};

/**
 * Random generator function that can be used in STL algorithms that
 * need a random generator.
 *
 * @return a value from 0 up to, but not including i.
 */

ptrdiff_t uni_random(ptrdiff_t i);

/**
 * Given an iterator's start and end, returns the count of the number
 * of elements.
 *
 * @param iteratorStart
 * @param iteratorEnd
 *
 * @return number of steps from start to end; count of elements
 * in iterator
 */
template<typename I>
int countOf(I iteratorStart, I iteratorEnd){
  I iterator = iteratorStart;
  int c = 0;
  while(iterator != iteratorEnd){ iterator++; c++; }
  return c;
}

/**
 * Shuffles the order of a vector of elements.
 * Note that there can be alternative algorithms
 * for this that trade memory and performance
 * in different ways. This proceeds through the
 * entire list, and at each position randomly
 * chooses one of the other elements and swaps them.
 *
 * The result should be mathematically random-
 * that is, every possible combination should have an equal
 * chance of being the final result.
 *
 *
 * @param elementList the list to be shuffled
 */
template<typename T>
void shuffleList(std::vector<T*>& elementList){
  if(elementList.size() <= 1) return;
  IntUniformGenerator rnd = Random::instance()->createUniIntGenerator(0, elementList.size() - 1);
  T* swap;
  for(size_t i = 0, sz = elementList.size(); i < sz; i++){
    int other = rnd.next();
    swap = elementList[i];
    elementList[i] = elementList[other];
    elementList[other] = swap;
  }

  // OR
    // see: http://en.wikipedia.org/wiki/Fisher-Yates_shuffle#The_modern_algorithm

  //    DoubleUniformGenerator rnd = Random::instance()->createUniDoubleGenerator(0, 1);
  //    T* swap;
  //    for(int pos = elementList.size() - 1; pos > 0; pos--){
  //        int range = pos + 1;
  //        int other = (int)(rnd.next() * (range));
  //        swap = elementList[pos];
  //        elementList[pos] = elementList[other];
  //        elementList[other] = swap;
  //    }
}

/**
 * Given a set, returns the elements in that set in shuffled
 * order. Acts by assigning all elements in the given set
 * to the vector, then calling shuffleList on that vector.
 *
 * @param elementSet collection of elements to be shuffled
 * @param [out] elementList the list, shuffled
 */
template<typename T>
void shuffleSet(std::set<T*>& elementSet, std::vector<T*>& elementList){
  elementList.assign(elementSet.begin(), elementSet.end());
  shuffleList(elementList);

  //    // Or- probably faster... ??
  //
  //    elementList.reserve(elementSet.size());
  //    typename std::set<T*>::iterator setIterator = elementSet.begin();
  //    DoubleUniformGenerator rnd = Random::instance()->createUniDoubleGenerator(0,1);
  //    for(int i = 0; i < elementSet.size(); i++){
  //        int j = (int)(rnd.next() * (i + 1));
  //        elementList.push_back(elementList[j]);
  //        elementList[j] = *setIterator;
  //        setIterator++;
  //    }
}

/**
 * Given an iterator and a number of elements,
 * creates a data structure that allows efficient access
 * to those elements. Is only valid as long as the iterator
 * is valid.
 *
 * The basic implementation creates a vector of ordered
 * pairs linking an integer and an iterator pointing
 * to an element in the original iteration set. To find
 * the nth element, the algorithm searches backwards through the
 * list of 'landmarks', finds the highest landmark lower than
 * n, chooses the iterator associated with that landmark,
 * and steps forward until n is reached, adding new landmarks
 * if appropriate. So given landmarks:
 *
 * 0    - pointer to element 0
 * 100  - pointer to element 100
 * 200  - pointer to element 200
 *
 * if the request for element 438 is given, the algorithm will
 * search backward and find landmark 200; it will
 * then step forward, adding landmarks for 300 and 400, until element 438
 * is reached and returned.
 *
 * Assuming that requests are evenly distributed, optimum interval for
 * landmarks is the square root of the size of the list, and performance
 * for the algorithm will be in log(size) time.
 *
 * Note that other implementations are possible- for example, checking
 * if enough memory would allow a completely indexed list. A long-term
 * possibility is allowing the user to specify (for example, specify that
 * the algorithm with lowest memory cost be used even though memory
 * is initially available, perhaps because other routines will be filling
 * that memory while this object is in use).
 */
template<typename I>
class RandomAccess{
private:
  I it;
  I begin;

  int interval;
  int maxLandmark;
  std::vector<std::pair<int, I > > landmarks;

public:

  /**
   * Constructs a RandomAccess instance for this iterator
   *
   * @param beginning
   * @param size
   */
  RandomAccess(I beginning, int size) :
    interval((int)(sqrt((double)size))), maxLandmark(0), it(beginning), begin(beginning)  {
        landmarks.push_back(std::pair<int, I >(0, beginning));
  }

  /**
   * Gets the element at the specified index
   *
   * @param index
   */
  I get(int index){
    bool place = (index > (maxLandmark + interval));
    typename std::vector<std::pair<int, I > >::iterator lm = landmarks.end();
    while((--lm)->first > index);
    int c = lm->first;
    it    = lm->second;
    if(place){
      while(c != index){
        c++;
        it++;
        if(c % interval == 0){
          landmarks.push_back(std::pair<int, I >(c, it));
          maxLandmark = c;
        }
      }
    }
    else{
      while(c != index){
        c++;
        it++;
      }
    }
    return it;
  }

};


/**
 * Gets N elements selected randomly from the elements that are indexed by the iterator passed.
 * Selection is 'without replacement'; no element will appear in the result set twice.
 * The elements are placed in a set; order is not randomized.
 *
 * If the 'selectedElements' set is not empty initially, the elements chosen will be added
 * to it with duplication prevented- in other words, the elements initially in the
 * set will not be selected from the population and added to the set by this routine.
 *
 * If it is not possible to select all of the agents requested
 * because the number requested exceeds the number of unique agents that can be added
 * to the result set, then all of the agents from the iterator will be added to the set.
 *
 * If the optional 'remove' parameter is set to true, the elements in the
 * original 'selectedElements' set will be removed before the set is
 * returned.
 *
 * @param iteratorStart the start of the iterated list of elements from which
 * selection will be made
 * @param size the size of the source population- i.e., the number of elements
 * in the list pointed to by iteratorStart
 * @param count number of elements to be selected
 * @param [out] selectedElements collection into which selected elements
 * are placed. If not empty, elements in initial set are excluded from
 * selection.
 * @param remove if true, the elements in the original set will be removed from
 * the set returned.
 *
 * @tparam T the type of the elements to be selected (i.e. the agent type)
 * @tparam I the type of the iterator to be used to pass through the population
 * to be sampled
 */
template<typename T, typename I>
void selectNElementsAtRandom(I iterator, int size, unsigned int count, std::set<T*>&selectedElements, bool remove = false){
  // When 'selectedElements' is large in comparison to size, or 'count' is
  // large in comparison to size, or both, a potential problem arises; this is both
  // a performance issue and a potential error (infinite loop). The number
  // of valid selections remaining in the original population will be
  // the size of the population minus the number of members of the original
  // population already in the set of 'selectedElements'. There is a performance
  // problem that can arise if the number of valid selections approaches
  // zero, and an infinite loop arises if that number reaches zero. However, a
  // complication is that we do not assume that all members in
  // 'selectedElements' are drawn from the same population that 'iterator' describes.

  // At a general level, the solution to this is that when the number of valid
  // selections is expected to become small, the algorithm will switch to
  // selecting the elements that will _not_ be added to the final set, rather
  // than those that will.

  // However, the more immediate issue is determining whether the problem
  // will arise at all, and a key question is how many members of 'selectedElements'
  // are drawn from the population. This, however, is costly to check, so
  // it should only be checked if it seems possible to lead to a problem.

  // A ceiling; the actual value may be lower
  int maxAlreadySelected = selectedElements.size();

  // We are not certain _any_ elements in the selectedElements set are from this population
  int knownSelectedElementsFromPop = 0;

  // This is a floor; there are at least this many available, but possibly more
  int minAvailable = size - maxAlreadySelected;

  // If you are requesting more than might be available, you must determine
  // the actual number available.
  if(count > minAvailable){
    if(maxAlreadySelected > 0){
      I tempIt = iterator;
      for(int i = 0; i < size; i++){
        T* ptr = (&**tempIt);
        typename std::set<T*>::iterator found = selectedElements.find(ptr);
        if(found != selectedElements.end()) knownSelectedElementsFromPop++;
        tempIt++;
      }
      maxAlreadySelected = knownSelectedElementsFromPop;
      minAvailable = size - maxAlreadySelected;
    }
  }
  // If removing the original elements, will need a copy of them
  typename std::vector<T*> tempToRemove;
  if(remove){
    tempToRemove.assign(selectedElements.begin(), selectedElements.end());
  }

  // There is no way to satisfy the request; copy all and return
  if(count > minAvailable){
    I it = iterator;
    for(int i = 0; i < size; i++){
      selectedElements.insert(&**it);
      it++;
    }
  }
  else{
    RandomAccess<I> ra(iterator, size);
    IntUniformGenerator rnd = Random::instance()->createUniIntGenerator(0, size - 1);
    T* ptr;

    // If the count of elements is very high in proportion to the population, faster to
    // choose agents that will _not_ be in the final set and then switch...
    // First the normal case, in which the number of agents is low
    if(count <= ((double)minAvailable)*0.6){ // Tests suggest that 2/3 is probably too high, so adjusted to .6; TODO optimize or analytically solve
      for (unsigned int i = 0; i < count; i++)
                do {  ptr = &**ra.get(rnd.next()); } while(!(selectedElements.insert(ptr).second));
    }
    else{ // Now the other case
      std::set<T*> elementsThatWillNotBeAdded;

      if(selectedElements.size() > 0){
        if(selectedElements.size() == knownSelectedElementsFromPop){ // Maybe we already checked and they ALL belong
          elementsThatWillNotBeAdded.insert(selectedElements.begin(), selectedElements.end());
        }
        else{
          I tempIt = iterator;
          for(int i = 0; i < size; i++){
            ptr = (&**tempIt);
            if(selectedElements.find(ptr) != selectedElements.end()) elementsThatWillNotBeAdded.insert(ptr);
            tempIt++;
          }
        }
      }
      // Note: duplicate element will not be inserted; failure will leave size of elementsThatWillNotBeAdded unchanged.
      while((size - elementsThatWillNotBeAdded.size()) > count) elementsThatWillNotBeAdded.insert(&**ra.get(rnd.next()));

      // Once the set of elements that will not be added is complete, cycle through the iterator and add
      // all of those elements that are not in elementsThatWillNotBeAdded
      I toAdd = iterator;
      typename std::set<T*>::iterator notFound = elementsThatWillNotBeAdded.end();
      for(int i = 0; i < size; i++){
        ptr = &**toAdd;
        if(elementsThatWillNotBeAdded.find(ptr) == notFound) selectedElements.insert(ptr);
        toAdd++;
      }
    }
  }
  // Before returning, remove the elements from the set, if the user requested
  if(remove){
    typename std::vector<T*>::iterator toRemove = tempToRemove.begin();
    while(toRemove != tempToRemove.end()){
      selectedElements.erase(*toRemove);
      toRemove++;
    }
  }
}


/**
 * Gets N elements selected randomly from the elements that are indexed by the iterator passed.
 *
 * Acts by calculating the size of the population covered by the iterator given using
 * countOf(iteratorStart, iteratorEnd) and invoking
 * selectNElementsAtRandom(I iterator, int size, int count, std::set<T*>&selectedElements, bool remove).
 *
 * @param iteratorStart the start of the iterated list of elements from which
 * selection will be made
 * @param iteratorEnd the end of the iterated list of elements from which
 * selection will be made
 * @param count number of elements to be selected
 * @param [out] selectedElements collection into which selected elements
 * are placed. If not empty, elements in initial set are excluded from
 * selection.
 * @param remove if true, the elements in the original set will be removed from
 * the set returned.
 *
 * @tparam T the type of the elements to be selected (i.e. the agent type)
 * @tparam I the type of the iterator to be used to pass through the population
 * to be sampled
 */
template<typename T, typename I>
void selectNElementsAtRandom(I iteratorStart, I iteratorEnd, int count, std::set<T*>&selectedElements, bool remove = false){
  selectNElementsAtRandom(iteratorStart, countOf(iteratorStart, iteratorEnd), count, selectedElements, remove);
}

/**
 * Randomly selects elements from the iterator (equivalent
 * to a call to selectNElementsAtRandom), but before returning them
 * places them into the specified vector and
 * randomly shuffles them.
 *
 * @param iteratorStart the start of the list of elements from which
 * selection will be made
 * @param size the count of the source population from which selection
 * will be made (i.e. the end of the iterated list)
 * @param count the number of elements to be selected. If this
 * exceeds the number of valid, unique selections available in the
 * source population then all valid, unique elements in the source population
 * will be returned
 * @param [out] selectedElements collection into which selected elements
 * are placed. If not empty, elements in initial set are excluded from
 * selection.
 * @param remove Optional; if included, specifies whether the elements
 * given in the 'selectedElements' set should appear in the final, sorted set.
 * If true, the elements in this set are not selected
 * from the population and are removed from the results before the final
 * set is returned; if false, the elements in the set are
 * not selected from the population but are included in the result set before
 * it is shuffled and returned.
 *
 * @tparam T the type of the elements to be selected (i.e. the agent type)
 * @tparam I the type of the iterator to be used to pass through the population
 * to be sampled
 */
template<typename T, typename I>
void selectNElementsInRandomOrder(I iterator, int size, int count, std::vector<T*>& selectedElements, bool remove = false){
  // Transfer all elements from the vector to a set
  std::set<T*> selectedElementSet;
  selectedElementSet.insert(selectedElements.begin(), selectedElements.end());
  selectedElements.clear();
  selectNElementsAtRandom(iterator, size, count, selectedElementSet, remove);
  shuffleSet(selectedElementSet, selectedElements);
}

/**
 * Randomly selects elements from the iterator (equivalent
 * to a call to selectNElementsAtRandom), but before returning them
 * places them into the specified vector and
 * randomly shuffles them.
 *
 * @param iteratorStart the start of the list of elements from which
 * selection will be made
 * @param iteratorEnd the end of the list of elements from which selection
 * will be made
 * @param count the number of elements to be selected. If this
 * exceeds the number of valid, unique selections available in the
 * source population then all valid, unique elements in the source population
 * will be returned
 * @param [out] selectedElements collection into which selected elements
 * are placed. If not empty, elements in initial set are excluded from
 * selection.
 * @param remove Optional; if included, specifies whether the elements
 * given in the 'selectedElements' set should appear in the final, sorted set.
 * If true, the elements in this set are not selected
 * from the population and are removed from the results before the final
 * set is returned; if false, the elements in the set are
 * not selected from the population but are included in the result set before
 * it is shuffled and returned.
 *
 * @tparam T the type of the elements to be selected (i.e. the agent type)
 * @tparam I the type of the iterator to be used to pass through the population
 * to be sampled
 */
template<typename T, typename I>
void selectNElementsInRandomOrder(I iteratorStart, I iteratorEnd, int count, std::vector<T*>& selectedElements, bool remove = false){
  selectNElementsInRandomOrder(iteratorStart, countOf(iteratorStart, iteratorEnd), count, selectedElements, remove);
}


}

#endif /* RANDOM_H_ */
