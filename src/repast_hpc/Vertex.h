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
 *  Vertex.h
 *
 *  Created on: Oct 13, 2010
 *      Author: nick
 */

#ifndef VERTEX_H_
#define VERTEX_H_

#include "AgentId.h"

#include <boost/unordered_map.hpp>
#include <boost/smart_ptr.hpp>

namespace repast {

template<typename V, typename E>
class Vertex;

/**
 * Hashes a Vertex using the hashcode of the AgentId that
 * the vertex contains.
 */
template<typename V, typename E>
struct HashVertex {
	std::size_t operator()(Vertex<V, E>* vertex) const {
		return vertex->item()->getId().hashcode();
	}
};

/**
 * Unary function used in the transform_iterator that allows an
 * iterator over the vertex map to return the node.
 */
template<typename V, typename E>
struct NodeGetter: public std::unary_function<
		typename boost::unordered_map<AgentId, Vertex<V, E>*, HashId>::value_type, V*> {
	V* operator()(const typename boost::unordered_map<AgentId, Vertex<V, E>*, HashId>::value_type& value) const {
		return value.second->ptr.get();
	}
};

/**

 * Used internally by repast graphs / networks to encapsulate Vertices.
 *
 * @tparam V the type of object stored by in a Vertex.
 * @tparam E the edge type of the network.
 */
template<typename V, typename E>
class Vertex {

public:
	/**
	 * Typedef for the adjacency list map that contains the other Vertices that
	 * this Vertex links to.
	 */
	typedef boost::unordered_map<Vertex<V, E>*, boost::shared_ptr<E>, HashVertex<V, E> > AdjListMap;
	typedef typename AdjListMap::iterator AdjListMapIterator;

	/**
	 * Enum the identifies whether an edge is incoming or outgoing.
	 */
	enum EdgeType {
		INCOMING, OUTGOING
	};

	/**
	 * Creates a Vertex that contains the specified item.
	 *
	 * @param item the item the Vertex should contain
	 */
	Vertex(boost::shared_ptr<V> item);
	virtual ~Vertex() {
	}

	/**
	 * Removes the edge of the specified type between this Vertex and the
	 * specified Vertex.
	 *
	 * @param other the other end of the edge
	 * @param type the type of edge to remove
	 *
	 * @return the removed edge if such an edge was found, otherwise 0.
	 */
	virtual boost::shared_ptr<E> removeEdge(Vertex* other, EdgeType type) = 0;

	/**
	 * Finds the edge of the specified type between this Vertex and the
	 * specified vertex.
	 *
	 * @param other the other end of the edge
	 * @param type the type of edge to remove
	 *
	 * @return the found edge, or 0.
	 */
	virtual boost::shared_ptr<E> findEdge(Vertex* other, EdgeType type) = 0;

	/**
	 * Adds an edge of the specified type between this Vertex and the
	 * specified vertex.
	 *
	 * @param edge the edge to add
	 * @param other the other end of the edge
	 * @param type the type of edge to add
	 */
	virtual void addEdge(Vertex<V, E>* other, boost::shared_ptr<E> edge, EdgeType type) = 0;

	/**
	 * Gets the successors of this Vertex.
	 *
	 * @param [out] the vector where any successors will be put
	 */
	virtual void successors(std::vector<V*>& out) = 0;

	/**
	 * Gets the predecessors of this Vertex.
	 *
	 * @param [out] the vector where any predecessors will be put
	 */
	virtual void predecessors(std::vector<V*>& out) = 0;

	/**
	 * Gets the Vertices adjacent to this Vertex.
	 *
	 * @param [out] the vector where the adjacent vectors will be put
	 */
	virtual void adjacent(std::vector<V*>& out) = 0;

	/**
	 * Gets all the edges of the specified type in which this Vertex
	 * participates and return them in out.
	 *
	 * @param type the type of edges to get
	 * @param [out] where the edges will be put.
	 */
	virtual void edges(EdgeType type, std::vector<boost::shared_ptr<E> >& out) = 0;

	/**
	 * Gets the in degree of this Vertex.
	 *
	 * @return the in degree of this Vertex.
	 */
	virtual int inDegree() = 0;

	/**
	 * Gets the out degree of this Vertex.
	 *
	 * @return the out degree of this Vertex.
	 */
	virtual int outDegree() = 0;

	/**
	 * Gets the item that this Vertex contains.
	 *
	 * @return the item.
	 */
	boost::shared_ptr<V> item() const {
		return ptr;
	}

protected:
	friend struct NodeGetter<V, E> ;
	boost::shared_ptr<V> ptr;
	boost::shared_ptr<E> removeEdge(Vertex<V, E>* other, AdjListMap* adjMap);
	void getItems(AdjListMap *adjMap, std::vector<V*>& out);
	void edges(AdjListMap *adjMap, std::vector<boost::shared_ptr<E> >& out);
};

template<typename V, typename E>
Vertex<V, E>::Vertex(boost::shared_ptr<V> item) :
	ptr(item) {
}

template<typename V, typename E>
boost::shared_ptr<E> Vertex<V, E>::removeEdge(Vertex<V, E>* other, AdjListMap* adjMap) {
  boost::shared_ptr<E> ret;
  AdjListMapIterator iter = adjMap->find(other);
  if (iter != adjMap->end()) {
    ret = iter->second;
    adjMap->erase(iter);
  }
  return ret;
}

template<typename V, typename E>
void Vertex<V, E>::getItems(AdjListMap *adjMap, std::vector<V*>& out) {
  const AdjListMapIterator iterEnd = adjMap->end();
  for (AdjListMapIterator iter = adjMap->begin(); iter != iterEnd; ++iter) {
    out.push_back(iter->first->item().get());
  }
}

template<typename V, typename E>
void Vertex<V, E>::edges(AdjListMap *adjMap, std::vector<boost::shared_ptr<E> >& out){
  const AdjListMapIterator mapEnd = adjMap->end();
  for (AdjListMapIterator iter = adjMap->begin(); iter != mapEnd; ++iter) {
    out.push_back(iter->second);
  }
}

}

#endif /* VERTEX_H_ */
