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
 *  Context.h
 *
 *  Created on: May 22, 2009
 *      Author: nick
 */

#ifndef CONTEXT_H_
#define CONTEXT_H_

#include <vector>
#include <set>

#include <boost/unordered_map.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/iterator/filter_iterator.hpp>
#include <boost/function.hpp>

#include "AgentId.h"
#include "AgentRequest.h"
#include "Random.h"
#include "ValueLayer.h"
#include "Projection.h"
#include "RepastErrors.h"

namespace repast {

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
	  interval((int)(sqrt((double)size))), maxLandmark(0)  {
        begin = beginning;
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
	

// Unary function used in the transform_iterator that allows context iterators
// to return the agent maps values.
template<typename T>
struct SecondElement: public std::unary_function<
		typename boost::unordered_map<AgentId, boost::shared_ptr<T> >::value_type, boost::shared_ptr<T> > {
	boost::shared_ptr<T> operator()(
			const typename boost::unordered_map<AgentId, boost::shared_ptr<T> >::value_type& value) const {
		const boost::shared_ptr<T>& ptr = value.second;
		//std::cout << ptr->getId() << std::endl;
		return ptr;
	}
};


/**
 * Collection of agents of type T with set semantics. Object identity and equality
 * is determined by their AgentId.
 *
 * @tparam the type objects contained by the Context. The T must extends repast::Agent.
 */
template<typename T>
class Context {

private:

	typedef typename std::vector<Projection<T>*>::iterator ProjPtrIter;
	typedef typename boost::unordered_map<AgentId, boost::shared_ptr<T>, HashId> AgentMap;

	typedef typename AgentMap::iterator AgentMapIterator;
	typedef typename AgentMap::const_iterator AgentMapConstIterator;

	AgentMap agents;
	std::vector<Projection<T> *> projections;
	std::map<std::string, BaseValueLayer*> valueLayers;

public:

	typedef typename boost::transform_iterator<SecondElement<T> , typename AgentMap::const_iterator> const_iterator;
	typedef typename boost::filter_iterator<IsAgentType<T> , typename Context<T>::const_iterator> const_bytype_iterator;

	Context();

	/**
	 * Destroys this context and the projections it contains.
	 */
	virtual ~Context();

	/**
	 * Adds the agent to the context. Returns true if the
	 * agent is added, or false if an agent with the same
	 * id is already in the context.
	 *
	 * @param agent the agent to add
	 *
	 * @return true if the agent is successfully dded, or false if an agent with the same
	 * id is already in the context.
	 */
	bool addAgent(T* agent);

	/**
	 * Adds the specified projection to this context. All the agents in this
	 * context will be added to the Projection. Any agents subsequently added
	 * to this context will also be added to the Projection.
	 *
	 * @param projection the projection to add
	 */
	void addProjection(Projection<T>* projection);

	/**
	 * Get the named Projection.
	 *
	 * @param the name of the projection to get
	 *
	 * @return the named Projection or 0 if no such Projection is found.
	 */
	Projection<T>* getProjection(const std::string& name);

	/**
	 * Removes the specified agent from this context.
	 *
	 * @param id the id of the agent to remove
	 */
	void removeAgent(const AgentId id);

	/**
	 * Removes the specified agent from this context.
	 */
	void removeAgent(T* agent);

	/**
	 * Gets the specified agent.
	 *
	 * @param the id of the agent to get.
	 */
	T* getAgent(const AgentId& id);

	/**
	 * Gets at random the specified count of agents and returns them
	 * in the agents vector.
	 *
	 * @param count the number of agents to get
	 * @param [out] agents a vector where the agents will be returned
	 */
	void getRandomAgents(const int count, std::vector<T*>& agents);

	/**
	 * Gets the start of iterator over the agents in this context.
	 * The iterator derefrences into shared_ptr<T>. The actual
	 * agent can be accessed by derefrenceing the iter: (*iter)->getId() for example.
	 *
	 * @return the start of iterator over the agents in this context.
	 */
	const_iterator begin() const {
		return const_iterator(agents.begin());
	}

	/**
	 * Gets the end of an iterator over the agents in this context. The iterator derefrences
	 * into shared_ptr<T>. The actual agent can be accessed by derefrenceing the iter:
	 * (*iter)->getId() for example.
	 *
	 * @return  the end of an iterator over the agents in this context
	 */
	const_iterator end() const {
		return const_iterator(agents.end());
	}

	/**
	 * Gets the start of an iterator over agents in this context of the specified type. The type
	 * corresponds to the type component of an agent's AgentId.
	 *
	 * @param typeId the type of the agent. Only Agents whose agentId.agentType() is equal to
	 * this typeId will be included in the iterator
	 *
	 * @return the start of an iterator over agents in this context of the specified type.
	 */
	const_bytype_iterator byTypeBegin(int typeId) const {
		return const_bytype_iterator(IsAgentType<T> (typeId), Context<T>::begin(), Context<T>::end());
	}

	/**
	 * Gets the end of an iterator over agents in this context of the specified type. The type
	 * corresponds to the type component of an agent's AgentId.
	 *
	 * @param typeId the type of the agent. Only Agents whose agentId.agentType() is equal to
	 * this typeId will be included in the iterator
	 *
	 * @return the end of an iterator over agents in this context of the specified type.
	 */
	const_bytype_iterator byTypeEnd(int typeId) const {
		return const_bytype_iterator(IsAgentType<T> (typeId), Context<T>::end(), Context<T>::end());
	}

	/**
	 * Returns true if the specified agent is in this context, otherwise false.
	 */
	bool contains(const AgentId& id);

	/**
	 * Gets the size (number of agents) in this context.
	 */
	int size() const {
		return agents.size();
	}

	/**
	 * Adds a value layer to this context.
	 *
	 * @param valueLayer the value layer to add
	 */
	void addValueLayer(BaseValueLayer* valueLayer);

	/**
	 * Gets the named discrete value layer from this Context. The value layer must have been
	 * previously added.
	 *
	 * @param valueLayerName the name of the value layer to get
	 *
	 * @tparam ValueType the numeric type contained by the value layer
	 * @tparam Borders the Border type of the value layer
	 *
	 * @return the named discrete value layer from this Context.
	 */
	template<typename ValueType, typename Borders>
	DiscreteValueLayer<ValueType, Borders>* getDiscreteValueLayer(const std::string& valueLayerName);

	/**
	 * Gets the named continuous value layer from this Context. The value layer must have been
	 * previously added.
	 *
	 * @param valueLayerName the name of the value layer to get
	 *
	 * @tparam ValueType the numeric type contained by the value layer
	 * @tparam Borders the Border type of the value layer
	 *
	 * @return the named continuous value layer from this Context.
	 */
	template<typename ValueType, typename Borders>
	ContinuousValueLayer<ValueType, Borders>* getContinuousValueLayer(const std::string& valueLayerName);

	/**
	 * Creates a filtered iterator over the set of agents
	 * in this context and returns it with a value equal
	 * to the beginning of the list.
	 *
	 * The struct can be any user-defined structure that implements
	 * a unary operator (see IsAgentType) that can be passed and
	 * which will become a filter to sort across the agent
     * list, e.g.:
     *
     * struct filter {
     *    bool operator()(const boost::shared_ptr<T>& ptr){ return (ptr->getAgentValue() == targetValue;) }
     * }
     *
     * This should allow filtering of agents by any attribute.
     *
     * @param fStruct an instance of the struct to be used as the filter
	 *
	 * @tparam filterStruct the type of the filter to be applied to the agents
	 *
	 * @return an iterator positioned at the beginning of the list of
	 * agents meeting the filter's criteria
	 */
	template<typename filterStruct>
	boost::filter_iterator<filterStruct, typename Context<T>::const_iterator> filteredBegin(const filterStruct& fStruct);
	
	/**
	 * Creates a filtered iterator over the set of agents
	 * in this context and returns it with a value equal
	 * to one step past end of the list.
	 *
	 * The struct can be any user-defined structure that implements
	 * a unary operator (see IsAgentType) that can be passed and
	 * which will become a filter to sort across the agent
	 * list, e.g.:
	 *
	 * struct filter {
	 *    bool operator()(const boost::shared_ptr<T>& ptr){ return (ptr->getAgentValue() == targetValue;) }
	 * }
	 *
	 * This should allow filtering of agents by any attribute.
	 *
	 * @param fStruct an instance of the struct to be used as the filter
	 *
	 * @tparam filterStruct the type of the filter to be applied to the agents
	 *
	 * @return an iterator positioned at one past the end of the list of
	 * agents meeting the filter's criteria
	 */
    template<typename filterStruct>
    boost::filter_iterator<filterStruct, typename Context<T>::const_iterator> filteredEnd(const filterStruct& fStruct);
	
	
	/**
	 * Creates a filtered iterator over the set of agents
	 * in this context of the specified type (per their AgentId
	 * values), and returns it with a value equal
	 * to the beginning of the list.
	 *
	 * The struct can be any user-defined structure that implements
	 * a unary operator (see IsAgentType) that can be passed and
	 * which will become a filter to sort across the agent
	 * list, e.g.:
	 *
	 * struct filter {
	 *    bool operator()(const boost::shared_ptr<T>& ptr){ return (ptr->getAgentValue() == targetValue;) }
	 * }
	 *
	 * This should allow filtering of agents by type and on any attribute.
	 *
	 * @param fStruct an instance of the struct to be used as the filter
	 * @param type the numeric type of agents to be included in the list
	 *
	 * @tparam filterStruct the type of the filter to be applied to the agents
	 *
	 * @return an iterator positioned at the beginning of the list of
	 * agents meeting the filter's criteria
	 */
    template<typename filterStruct>
    boost::filter_iterator<filterStruct, typename Context<T>::const_bytype_iterator> byTypeFilteredBegin(const int type, const filterStruct& fStruct);
	
	/**
	 * Creates a filtered iterator over the set of agents
	 * in this context of the specified type (per their AgentId
	 * values), and returns it with a value equal
	 * to one past the end of the list.
	 *
	 * The struct can be any user-defined structure that implements
	 * a unary operator (see IsAgentType) that can be passed and
	 * which will become a filter to sort across the agent
	 * list, e.g.:
	 *
	 * struct filter {
	 *    bool operator()(const boost::shared_ptr<T>& ptr){ return (ptr->getAgentValue() == targetValue;) }
	 * }
	 *
	 * This should allow filtering of agents by type and on any attribute.
	 *
	 * @param fStruct an instance of the struct to be used as the filter
	 * @param type the numeric type of agents to be included in the list
	 *
	 * @tparam filterStruct the type of the filter to be applied to the agents
	 *
	 * @return an iterator positioned at one past the end of the list of
	 * agents meeting the filter's criteria
	 */
    template<typename filterStruct>
    boost::filter_iterator<filterStruct, typename Context<T>::const_bytype_iterator> byTypeFilteredEnd(const int type, const filterStruct& fStruct);
	
	/**
	 * Gets a set of pointers to all agents in this context.
	 *
	 * If the 'remove' parameter is set to true, any elements in the
	 * original set will be removed before the method returns.
	 *
	 * @param [out] selectedAgents a set into which the pointers to the agents will be placed
	 * @param remove if true, remove any elements originally in the set before the
	 * set is returned (default is false)
	 */
	void selectAgents(std::set<T*>& selectedAgents, bool remove = false);

	/**
	 * Gets a randomly ordered vector of pointers to all agents in this context.
	 *
	 * If the 'remove' parameter is set to true, any elements in the
	 * original vector will be removed before the method returns.
	 *
	 * @param [out] selectedAgents a vector into which the pointers to the agents will be placed
	 * @param remove if true, remove any elements originally in the set before the
	 * set is returned (default is false)
	 */
	void selectAgents(std::vector<T*>& selectedAgents, bool remove = false);

	/**
	 * Gets a set of pointers to a specified number of randomly selected agents.
	 *
	 * If the set passed contains any elements when this method is called,
	 * the agents pointed to by those elements will be omitted from the selection.
	 *
	 * If the 'remove' parameter is set to true, any elements in the
	 * original set will be removed before the method returns.
	 *
	 * @param count the number of agents to be selected. If this exceeds the number
	 * that can possibly be selected, all possible agents will be selected
	 * @param [out] selectedAgents a set into which the pointers to the agents will be placed
	 * @param remove if true, remove any elements originally in the set before the
	 * set is returned (default is false)
	 */
	void selectAgents(int count, std::set<T*>& selectedAgents, bool remove = false);

	/**
	 * Gets a randomly ordered vector of pointers to a specified number of randomly selected agents.
	 *
	 * If the vector passed contains any elements when this method is called,
	 * the agents pointed to by those elements will be omitted from the selection.
	 *
	 * If the 'remove' parameter is set to true, any elements in the
	 * original vector will be removed before the method returns.
	 *
	 * @param count the number of agents to be selected. If this exceeds the number
	 * that can possibly be selected, all possible agents will be selected
	 * @param [out] selectedAgents a vector into which the pointers to the agents will be placed
	 * @param remove if true, remove any elements originally in the set before the
	 * set is returned (default is false)
	 */
	void selectAgents(int count, std::vector<T*>& selectedAgents, bool remove = false);
	
	/**
	 * Gets a set of pointers to all agents in this context
	 * of a specified type (per their AgentId values).
	 *
	 * If the 'remove' parameter is set to true, any elements in the
	 * original set will be removed before the method returns.
	 *
	 * The popSize parameter is used when the method is repeatedly called
	 * on a population whose size is known. Calls to this method typically begin
	 * by determining the size of the (valid) population to be sampled; if this is known,
	 * it can be provided here, improving performance.
	 *
	 * @param [out] selectedAgents a set into which the pointers to the agents will be placed
	 * @param type numeric type of agent to be selected
	 * @param remove if true, remove any elements originally in the set before the
	 * set is returned (default is false)
	 * @param popSize size of the population from which the sample will be drawn
	 */
    void selectAgents(std::set<T*>& selectedAgents, int type, bool remove = false, int popSize = -1);

	/**
	 * Gets a randomly ordered vector of pointers to all agents in this context
	 * of a specified type (per their AgentId values).
	 *
	 * If the 'remove' parameter is set to true, any elements in the
	 * original vector will be removed before the method returns.
	 *
	 * The popSize parameter is used when the method is repeatedly called
	 * on a population whose size is known. Calls to this method typically begin
	 * by determining the size of the (valid) population to be sampled; if this is known,
	 * it can be provided here, improving performance.
	 *
	 * @param [out] selectedAgents a vector into which the pointers to the agents will be placed
	 * @param type numeric type of agent to be selected
	 * @param remove if true, remove any elements originally in the set before the
	 * set is returned (default is false)
	 * @param popSize size of the population from which the sample will be drawn
	 */
	void selectAgents(std::vector<T*>& selectedAgents, int type, bool remove = false, int popSize = -1);

	/**
	 * Gets a set of pointers to a specified number of randomly selected agents
	 * of a specified type (per their AgentId values).
	 *
	 * If the set passed contains any elements when this method is called,
	 * the agents pointed to by those elements will be omitted from the selection.
	 *
	 * If the 'remove' parameter is set to true, any elements in the
	 * original set will be removed before the method returns.
	 *
	 * The popSize parameter is used when the method is repeatedly called
	 * on a population whose size is known. Calls to this method typically begin
	 * by determining the size of the (valid) population to be sampled; if this is known,
	 * it can be provided here, improving performance.
	 *
	 * @param count the number of agents to be selected. If this exceeds the number
	 * that can possibly be selected, all possible agents will be selected
	 * @param [out] selectedAgents a set into which the pointers to the agents will be placed
	 * @param type numeric type of agent to be selected
	 * @param remove if true, remove any elements originally in the set before the
	 * set is returned (default is false)
	 * @param popSize size of the population from which the sample will be drawn
	 */
	void selectAgents(int count, std::set<T*>& selectedAgents, int type, bool remove = false, int popSize = -1);

	/**
	 * Gets a randomly ordered vector of pointers to a specified number of randomly selected agents
	 * of a specified type (per their AgentId values).
	 *
	 * If the vector passed contains any elements when this method is called,
	 * the agents pointed to by those elements will be omitted from the selection.
	 *
	 * If the 'remove' parameter is set to true, any elements in the
	 * original vector will be removed before the method returns.
	 *
	 * The popSize parameter is used when the method is repeatedly called
	 * on a population whose size is known. Calls to this method typically begin
	 * by determining the size of the (valid) population to be sampled; if this is known,
	 * it can be provided here, improving performance.
	 *
	 * @param count the number of agents to be selected. If this exceeds the number
	 * that can possibly be selected, all possible agents will be selected
	 * @param [out] selectedAgents a vector into which the pointers to the agents will be placed
	 * @param type numeric type of agent to be selected
	 * @param remove if true, remove any elements originally in the set before the
	 * set is returned (default is false)
	 * @param popSize size of the population from which the sample will be drawn
	 */
	void selectAgents(int count, std::vector<T*>& selectedAgents, int type, bool remove = false, int popSize = -1);
	
	/**
	 * Gets a set of pointers to all agents in this context matching a user-defined filter.
	 *
	 * If the 'remove' parameter is set to true, any elements in the
	 * original set will be removed before the method returns.
	 *
	 * The popSize parameter is used when the method is repeatedly called
	 * on a population whose size is known. Calls to this method typically begin
	 * by determining the size of the (valid) population to be sampled; if this is known,
	 * it can be provided here, improving performance.
	 *
	 * that can possibly be selected, all possible agents will be selected
	 * @param [out] selectedAgents a set into which the pointers to the agents will be placed
	 * @param filter user-defined filter specifying any criteria agents to be selected
	 * must meet
	 * @param remove if true, remove any elements originally in the set before the
	 * set is returned (default is false)
	 * @param popSize size of the population from which the sample will be drawn
	 *
	 * @tparam filterStruct the type of the filter to be applied to the agents
	 */
    template<typename filterStruct>
    void selectAgents(std::set<T*>& selectedAgents, filterStruct& filter, bool remove = false, int popSize = -1);

	/**
	 * Gets a randomly ordered vector of pointers to all agents in this context
	 * matching a user-defined filter.
	 *
	 * If the 'remove' parameter is set to true, any elements in the
	 * original vector will be removed before the method returns.
	 *
	 * The popSize parameter is used when the method is repeatedly called
	 * on a population whose size is known. Calls to this method typically begin
	 * by determining the size of the (valid) population to be sampled; if this is known,
	 * it can be provided here, improving performance.
	 *
	 * @param [out] selectedAgents a vector into which the pointers to the agents will be placed
	 * @param filter user-defined filter specifying any criteria agents to be selected
	 * must meet
	 * @param remove if true, remove any elements originally in the set before the
	 * set is returned (default is false)
	 * @param popSize size of the population from which the sample will be drawn
	 *
	 * @tparam filterStruct the type of the filter to be applied to the agents
	 */
	template<typename filterStruct>
    void selectAgents(std::vector<T*>& selectedAgents, filterStruct& filter, bool remove = false, int popSize = -1);

	/**
	 * Gets a set of pointers to a specified number of randomly selected agents
	 * matching a user-defined filter.
	 *
	 * If the set passed contains any elements when this method is called,
	 * the agents pointed to by those elements will be omitted from the selection.
	 *
	 * If the 'remove' parameter is set to true, any elements in the
	 * original set will be removed before the method returns.
	 *
	 * The popSize parameter is used when the method is repeatedly called
	 * on a population whose size is known. Calls to this method typically begin
	 * by determining the size of the (valid) population to be sampled; if this is known,
	 * it can be provided here, improving performance.
	 *
	 * @param count the number of agents to be selected. If this exceeds the number
	 * that can possibly be selected, all possible agents will be selected
	 * @param [out] selectedAgents a set into which the pointers to the agents will be placed
	 * @param filter user-defined filter specifying any criteria agents to be selected
	 * must meet
	 * @param remove if true, remove any elements originally in the set before the
	 * set is returned (default is false)
	 * @param popSize size of the population from which the sample will be drawn
	 *
	 * @tparam filterStruct the type of the filter to be applied to the agents
	 */
	template<typename filterStruct>
    void selectAgents(int count, std::set<T*>& selectedAgents, filterStruct& filter, bool remove = false, int popSize = -1);

	/**
	 * Gets a randomly ordered vector of pointers to a specified number of randomly selected agents
	 * matching a user-defined filter.
	 *
	 * If the vector passed contains any elements when this method is called,
	 * the agents pointed to by those elements will be omitted from the selection.
	 *
	 * If the 'remove' parameter is set to true, any elements in the
	 * original vector will be removed before the method returns.
	 *
	 * The popSize parameter is used when the method is repeatedly called
	 * on a population whose size is known. Calls to this method typically begin
	 * by determining the size of the (valid) population to be sampled; if this is known,
	 * it can be provided here, improving performance.
	 *
	 * @param count the number of agents to be selected. If this exceeds the number
	 * that can possibly be selected, all possible agents will be selected
	 * @param [out] selectedAgents a vector into which the pointers to the agents will be placed
	 * @param filter user-defined filter specifying any criteria agents to be selected
	 * must meet
	 * @param remove if true, remove any elements originally in the set before the
	 * set is returned (default is false)
	 * @param popSize size of the population from which the sample will be drawn
	 *
	 * @tparam filterStruct the type of the filter to be applied to the agents
	 */
	template<typename filterStruct>
    void selectAgents(int count, std::vector<T*>& selectedAgents, filterStruct& filter, bool remove = false, int popSize = -1);
	
	/**
	 * Gets a set of pointers to all agents in this context
	 * of a specified type (per their AgentId values) and matching a user-defined filter.
	 *
	 * If the 'remove' parameter is set to true, any elements in the
	 * original set will be removed before the method returns.
	 *
	 * The popSize parameter is used when the method is repeatedly called
	 * on a population whose size is known. Calls to this method typically begin
	 * by determining the size of the (valid) population to be sampled; if this is known,
	 * it can be provided here, improving performance.
	 *
	 * @param [out] selectedAgents a set into which the pointers to the agents will be placed
	 * @param type numeric type of agent to be selected
	 * @param filter user-defined filter specifying any criteria agents to be selected
	 * must meet
	 * @param remove if true, remove any elements originally in the set before the
	 * set is returned (default is false)
	 * @param popSize size of the population from which the sample will be drawn
	 *
	 * @tparam filterStruct the type of the filter to be applied to the agents
	 */
    template<typename filterStruct>
    void selectAgents(std::set<T*>& selectedAgents, int type, filterStruct& filter, bool remove = false, int popSize = -1);

	/**
	 * Gets a randomly ordered vector of pointers to all agents in this context
	 * of a specified type (per their AgentId values) and matching a user-defined filter.
	 *
	 * If the 'remove' parameter is set to true, any elements in the
	 * original vector will be removed before the method returns.
	 *
	 * The popSize parameter is used when the method is repeatedly called
	 * on a population whose size is known. Calls to this method typically begin
	 * by determining the size of the (valid) population to be sampled; if this is known,
	 * it can be provided here, improving performance.
	 *
	 * @param [out] selectedAgents a vector into which the pointers to the agents will be placed
	 * @param type numeric type of agent to be selected
	 * @param filter user-defined filter specifying any criteria agents to be selected
	 * must meet
	 * @param remove if true, remove any elements originally in the set before the
	 * set is returned (default is false)
	 * @param popSize size of the population from which the sample will be drawn
	 *
	 * @tparam filterStruct the type of the filter to be applied to the agents
	 */
	template<typename filterStruct>
    void selectAgents(std::vector<T*>& selectedAgents, int type, filterStruct& filter, bool remove = false, int popSize = -1);

	/**
	 * Gets a set of pointers to a specified number of randomly selected agents
	 * of a specified type (per their AgentId values) and matching a user-defined filter.
	 *
	 * If the set passed contains any elements when this method is called,
	 * the agents pointed to by those elements will be omitted from the selection.
	 *
	 * If the 'remove' parameter is set to true, any elements in the
	 * original set will be removed before the method returns.
	 *
	 * The popSize parameter is used when the method is repeatedly called
	 * on a population whose size is known. Calls to this method typically begin
	 * by determining the size of the (valid) population to be sampled; if this is known,
	 * it can be provided here, improving performance.
	 *
	 * @param count the number of agents to be selected. If this exceeds the number
	 * that can possibly be selected, all possible agents will be selected
	 * @param [out] selectedAgents a set into which the pointers to the agents will be placed
	 * @param type numeric type of agent to be selected
	 * @param filter user-defined filter specifying any criteria agents to be selected
	 * must meet
	 * @param remove if true, remove any elements originally in the set before the
	 * set is returned (default is false)
	 * @param popSize size of the population from which the sample will be drawn
	 *
	 * @tparam filterStruct the type of the filter to be applied to the agents
	 */
	template<typename filterStruct>
    void selectAgents(int count, std::set<T*>& selectedAgents, int type, filterStruct& filter, bool remove = false, int popSize = -1);

	/**
	 * Gets a randomly ordered vector of pointers to a specified number of randomly selected agents
	 * of a specified type (per their AgentId values) and matching a user-defined filter.
	 *
	 * If the vector passed contains any elements when this method is called,
	 * the agents pointed to by those elements will be omitted from the selection.
	 *
	 * If the 'remove' parameter is set to true, any elements in the
	 * original vector will be removed before the method returns.
	 *
	 * The popSize parameter is used when the method is repeatedly called
	 * on a population whose size is known. Calls to this method typically begin
	 * by determining the size of the (valid) population to be sampled; if this is known,
	 * it can be provided here, improving performance.
	 *
	 * @param count the number of agents to be selected. If this exceeds the number
	 * that can possibly be selected, all possible agents will be selected
	 * @param [out] selectedAgents a vector into which the pointers to the agents will be placed
	 * @param type numeric type of agent to be selected
	 * @param filter user-defined filter specifying any criteria agents to be selected
	 * must meet
	 * @param remove if true, remove any elements originally in the set before the
	 * set is returned (default is false)
	 * @param popSize size of the population from which the sample will be drawn
	 *
	 * @tparam filterStruct the type of the filter to be applied to the agents
	 */
	template<typename filterStruct>
    void selectAgents(int count, std::vector<T*>& selectedAgents, int type, filterStruct& filter, bool remove = false, int popSize = -1);
	

};





template<typename T>
Context<T>::Context() {
}

template<typename T>
Context<T>::~Context() {
	agents.erase(agents.begin(), agents.end());
	for (ProjPtrIter iter = projections.begin(); iter != projections.end(); ++iter) {
		Projection<T>* proj = *iter;
		delete proj;
	}
	projections.clear();

	for (std::map<std::string, BaseValueLayer*>::iterator iter = valueLayers.begin(); iter != valueLayers.end(); ++iter) {
		BaseValueLayer* layer = iter->second;
		delete layer;
	}
	valueLayers.clear();
}

template<typename T>
void Context<T>::addProjection(Projection<T>* projection) {
	if (find(projections.begin(), projections.end(), projection) != projections.end())
      throw Repast_Error_9(projection->name()); // Projection with specified name already in context

	for (const_iterator iter = begin(); iter != end(); ++iter) {
		projection->addAgent(*iter);
	}
	projections.push_back(projection);
}

template<typename T>
Projection<T>* Context<T>::getProjection(const std::string& name) {
	for (ProjPtrIter iter = projections.begin(); iter != projections.end(); ++iter) {
		Projection<T>* proj = *iter;
		if (proj->name() == name)
			return proj;
	}
	return NULL;
}

template<typename T>
T* Context<T>::getAgent(const AgentId& id) {
	AgentMapIterator iter = agents.find(id);
	if (iter == agents.end())
		return 0;
	return iter->second.get();
}

template<typename T>
void Context<T>::getRandomAgents(const int count, std::vector<T*>& agents) {
	for (int i = 0; i < count; i++) {
		IntUniformGenerator rnd = Random::instance()->createUniIntGenerator(0, size() - 1);
		bool found = false;
		while (!found) {
			const_iterator iter = begin();
			for (int j = 0, n = rnd.next(); j < n; iter++, j++)
				;
			T* agent = iter->get();
			if (find(agents.begin(), agents.end(), agent) == agents.end()) {
				agents.push_back(agent);
				found = true;
			}
		}
	}
}

template<typename T>
bool Context<T>::addAgent(T* agent) {
	const AgentId& id = agent->getId();
	if (agents.find(id) != agents.end())
		return false;

	boost::shared_ptr<T> ptr(agent);
	agents[id] = ptr;

	for (ProjPtrIter iter = projections.begin(); iter != projections.end(); ++iter) {
		Projection<T>* proj = *iter;
		proj->addAgent(ptr);
	}

	return true;
}

template<typename T>
void Context<T>::removeAgent(T* agent) {
	removeAgent(agent->getId());
}

template<typename T>
void Context<T>::removeAgent(const AgentId id) {
	const AgentMapIterator iter = agents.find(id);
	if (iter != agents.end()) {
		boost::shared_ptr<T>& ptr = iter->second;
		for (ProjPtrIter pIter = projections.begin(); pIter != projections.end(); ++pIter) {
			Projection<T>* proj = *pIter;
			proj->removeAgent(ptr.get());
		}
		agents.erase(iter);
	}
}

template<typename T>
bool Context<T>::contains(const AgentId& id) {
	return agents.find(id) != agents.end();
}

template<typename T>
void Context<T>::addValueLayer(BaseValueLayer* layer) {
	valueLayers[layer->name()] = layer;
}

template<typename T>
template<typename ValueType, typename Borders>
DiscreteValueLayer<ValueType, Borders>* Context<T>::getDiscreteValueLayer(const std::string& valueLayerName) {

	std::map<std::string, BaseValueLayer*>::iterator iter = valueLayers.find(valueLayerName);
	if (iter == valueLayers.end())
		return 0;
	return static_cast<DiscreteValueLayer<ValueType, Borders>*> (iter->second);
}

template<typename T>
template<typename ValueType, typename Borders>
ContinuousValueLayer<ValueType, Borders>* Context<T>::getContinuousValueLayer(const std::string& valueLayerName) {

	std::map<std::string, BaseValueLayer*>::iterator iter = valueLayers.find(valueLayerName);
	if (iter == valueLayers.end())
		return 0;
	return static_cast<ContinuousValueLayer<ValueType, Borders>*> (iter->second);
}

	

	
template<typename T>
template<typename filterStruct>
boost::filter_iterator<filterStruct, typename Context<T>::const_iterator> Context<T>::filteredBegin(const filterStruct& fStruct){
	return boost::filter_iterator<filterStruct , typename Context<T>::const_iterator> (fStruct, Context<T>::begin(), Context<T>::end());
}
	
template<typename T>
template<typename filterStruct>
boost::filter_iterator<filterStruct, typename Context<T>::const_iterator> Context<T>::filteredEnd(const filterStruct& fStruct){
	return boost::filter_iterator<filterStruct , typename Context<T>::const_iterator> (fStruct, Context<T>::end(), Context<T>::end());
}
	
	
template<typename T>
template<typename filterStruct>
boost::filter_iterator<filterStruct, typename Context<T>::const_bytype_iterator> Context<T>::byTypeFilteredBegin(const int type, const filterStruct& fStruct){
	return boost::filter_iterator<filterStruct , typename Context<T>::const_bytype_iterator> (fStruct, Context<T>::byTypeBegin(type), Context<T>::byTypeEnd(type));
}
	
template<typename T>
template<typename filterStruct>
boost::filter_iterator<filterStruct, typename Context<T>::const_bytype_iterator> Context<T>::byTypeFilteredEnd(const int type, const filterStruct& fStruct){
	return boost::filter_iterator<filterStruct , typename Context<T>::const_bytype_iterator> (fStruct, Context<T>::byTypeEnd(type), Context<T>::byTypeEnd(type));
}
	
	

	
	
	
	/* ****************************** */
	
// Selection
	
template<typename T>
void Context<T>::selectAgents(std::set<T*>& selectedAgents, bool remove){
	selectNElementsAtRandom(begin(), size(), size(), selectedAgents, remove);
}
	
template<typename T>
void Context<T>::selectAgents(std::vector<T*>& selectedAgents, bool remove){
	selectNElementsInRandomOrder(begin(), size(), size(), selectedAgents, remove);
}
	
template<typename T>
void Context<T>::selectAgents(int count, std::set<T*>& selectedAgents, bool remove){
	selectNElementsAtRandom(begin(), size(), count, selectedAgents, remove);
}
	
template<typename T>
void Context<T>::selectAgents(int count, std::vector<T*>& selectedAgents, bool remove){
	selectNElementsInRandomOrder(begin(), end(), count, selectedAgents, remove);
}
	
template<typename T>
void Context<T>::selectAgents(std::set<T*>& selectedAgents, int type, bool remove, int popSize){
	if(popSize <= -1) selectNElementsAtRandom(byTypeBegin(type), byTypeEnd(type), size(), selectedAgents, remove);
	else              selectNElementsAtRandom(byTypeBegin(type), popSize, size(), selectedAgents, remove);
}
	
template<typename T>
void Context<T>::selectAgents(std::vector<T*>& selectedAgents, int type, bool remove, int popSize){
	if(popSize <= -1) selectNElementsInRandomOrder(byTypeBegin(type), byTypeEnd(type), size(), selectedAgents, remove);
	else              selectNElementsInRandomOrder(byTypeBegin(type), popSize, size(), selectedAgents, remove);
}
	
template<typename T>
void Context<T>::selectAgents(int count, std::set<T*>& selectedAgents, int type, bool remove, int popSize){
	if(popSize <= -1) selectNElementsAtRandom(byTypeBegin(type), byTypeEnd(type), count, selectedAgents, remove);
	else              selectNElementsAtRandom(byTypeBegin(type), popSize, count, selectedAgents, remove);
}
	
template<typename T>
void Context<T>::selectAgents(int count, std::vector<T*>& selectedAgents, int type, bool remove, int popSize){
	if(popSize <= -1) selectNElementsInRandomOrder(byTypeBegin(type), byTypeEnd(type), count, selectedAgents, remove);
	else              selectNElementsInRandomOrder(byTypeBegin(type), popSize, count, selectedAgents, remove);
}
	
template<typename T>
template<typename filterStruct>
void Context<T>::selectAgents(std::set<T*>& selectedAgents, filterStruct& filter, bool remove, int popSize){
	if(popSize <= -1) selectNElementsAtRandom(filteredBegin<filterStruct>(filter), filteredEnd<filterStruct>(filter), size(), selectedAgents, remove);
	else              selectNElementsAtRandom(filteredBegin<filterStruct>(filter), popSize, size(), selectedAgents, remove);
}
	
template<typename T>
template<typename filterStruct>
void Context<T>::selectAgents(std::vector<T*>& selectedAgents, filterStruct& filter, bool remove, int popSize){
	if(popSize <= -1) selectNElementsInRandomOrder(filteredBegin(filter), filteredEnd(filter), size(), selectedAgents, remove);
	else              selectNElementsInRandomOrder(filteredBegin(filter), popSize, size(), selectedAgents, remove);
}
	
template<typename T>
template<typename filterStruct>
void Context<T>::selectAgents(int count, std::set<T*>& selectedAgents, filterStruct& filter, bool remove, int popSize){
	if(popSize <= -1) selectNElementsAtRandom(filteredBegin<filterStruct>(filter), filteredEnd<filterStruct>(filter), count, selectedAgents, remove);
	else              selectNElementsAtRandom(filteredBegin<filterStruct>(filter), popSize, count, selectedAgents, remove);
}
	
template<typename T>
template<typename filterStruct>
void Context<T>::selectAgents(int count, std::vector<T*>& selectedAgents, filterStruct& filter, bool remove, int popSize){
	if(popSize <= -1) selectNElementsInRandomOrder(filteredBegin(filter), filteredEnd(filter), count, selectedAgents, remove);
	else              selectNElementsInRandomOrder(filteredBegin(filter), popSize, count, selectedAgents, remove);
}
	
template<typename T>
template<typename filterStruct>
void Context<T>::selectAgents(std::set<T*>& selectedAgents, int type, filterStruct& filter, bool remove, int popSize){
	if(popSize <= -1) selectNElementsAtRandom(byTypeFilteredBegin(type, filter), byTypeFilteredEnd(type, filter), size(), selectedAgents, remove);
	else              selectNElementsAtRandom(byTypeFilteredBegin(type, filter), popSize, size(), selectedAgents, remove);
}
	
template<typename T>
template<typename filterStruct>
void Context<T>::selectAgents(std::vector<T*>& selectedAgents, int type, filterStruct& filter, bool remove, int popSize){
	if(popSize <= -1) selectNElementsInRandomOrder(byTypeFilteredBegin(type, filter), byTypeFilteredEnd(type, filter), size(), selectedAgents, remove);
	else              selectNElementsInRandomOrder(byTypeFilteredBegin(type, filter), popSize, size(), selectedAgents, remove);
}
	
template<typename T>
template<typename filterStruct>
void Context<T>::selectAgents(int count, std::set<T*>& selectedAgents, int type, filterStruct& filter, bool remove, int popSize){
	if(popSize <= -1) selectNElementsAtRandom(byTypeFilteredBegin(type, filter), byTypeFilteredEnd(type, filter), count, selectedAgents, remove);
	else              selectNElementsAtRandom(byTypeFilteredBegin(type, filter), popSize, count, selectedAgents, remove);
}
	
template<typename T>
template<typename filterStruct>
void Context<T>::selectAgents(int count, std::vector<T*>& selectedAgents, int type, filterStruct& filter, bool remove, int popSize){
	if(popSize <= -1) selectNElementsInRandomOrder(byTypeFilteredBegin(type, filter), byTypeFilteredEnd(type, filter), count, selectedAgents, remove);
	else              selectNElementsInRandomOrder(byTypeFilteredBegin(type, filter), popSize, count, selectedAgents, remove);
}
	
	
	

}
#endif /* CONTEXT_H_ */
