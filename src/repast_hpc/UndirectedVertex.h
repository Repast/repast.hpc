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
 *  UndirectedVertex.h
 *
 *  Created on: Oct 13, 2010
 *      Author: nick
 */

#ifndef UNDIRECTEDVERTEX_H_
#define UNDIRECTEDVERTEX_H_

#include "Vertex.h"

namespace repast {

/**
 * A vertex in an undirected network.
 *
 * @tparam V the vertex type
 * @tparam E the edge type. The edge type must be or extend RepastEdge.
 */
template<typename V, typename E>
class UndirectedVertex : public Vertex<V, E> {

private:
	typedef typename Vertex<V,E>::AdjListMap AdjListMap;
	typedef typename Vertex<V,E>::AdjListMap::iterator AdjListMapIterator;
	typedef typename Vertex<V,E>::EdgeType EdgeType;

	AdjListMap* adjMap;

public:
	UndirectedVertex(boost::shared_ptr<V> item);
	virtual ~UndirectedVertex();

	// doc inherited from Vertex.h
	virtual boost::shared_ptr<E> removeEdge(Vertex<V,E>* other, EdgeType type);

	// doc inherited from Vertex.h
	virtual boost::shared_ptr<E> findEdge(Vertex<V,E>* other, EdgeType type);

	// doc inherited from Vertex.h
	virtual void addEdge(Vertex<V,E>* other, boost::shared_ptr<E> edge, EdgeType type);

	// doc inherited from Vertex.h
	virtual void successors(std::vector<V*>& out);

	// doc inherited from Vertex.h
	virtual void predecessors(std::vector<V*>& out);

	// doc inherited from Vertex.h
	virtual void adjacent(std::vector<V*>& out);

	// doc inherited from Vertex.h
	virtual void edges(EdgeType type , std::vector<boost::shared_ptr<E> >& out);

	// doc inherited from Vertex.h
	int inDegree();

	// doc inherited from Vertex.h
	int outDegree();
};

template<typename V, typename E>
UndirectedVertex<V,E>::UndirectedVertex(boost::shared_ptr<V> item) : Vertex<V,E>(item) {
	adjMap = new AdjListMap();
}

template<typename V, typename E>
UndirectedVertex<V,E>::~UndirectedVertex() {
	delete adjMap;
}

template<typename V, typename E>
boost::shared_ptr<E> UndirectedVertex<V,E>::removeEdge(Vertex<V,E>* other, EdgeType type) {
	return Vertex<V,E>::removeEdge(other, adjMap);
}

template<typename V, typename E>
boost::shared_ptr<E> UndirectedVertex<V,E>::findEdge(Vertex<V,E>* other, EdgeType type) {
	boost::shared_ptr<E> ret;
	AdjListMapIterator iter = adjMap->find(other);
	return (iter != adjMap->end() ? iter->second : ret);
}

template<typename V, typename E>
void UndirectedVertex<V,E>::addEdge(Vertex<V,E>* other, boost::shared_ptr<E> edge, EdgeType type) {
	(*adjMap)[other] = edge;
}

template<typename V, typename E>
void UndirectedVertex<V,E>::successors(std::vector<V*>& out) {
	this->getItems(adjMap, out);
}

template<typename V, typename E>
void UndirectedVertex<V,E>::predecessors(std::vector<V*>& out) {
	this->getItems(adjMap, out);
}

template<typename V, typename E>
void UndirectedVertex<V,E>::adjacent(std::vector<V*>& out) {
	this->getItems(adjMap, out);
}

template<typename V, typename E>
int UndirectedVertex<V,E>::inDegree() {
	return adjMap->size();
}

template<typename V, typename E>
int UndirectedVertex<V,E>::outDegree() {
	return adjMap->size();
}

template<typename V, typename E>
void UndirectedVertex<V,E>::edges(EdgeType type , std::vector<boost::shared_ptr<E> >& out) {
  Vertex<V, E>::edges(adjMap, out);
}



}


#endif /* UNDIRECTEDVERTEX_H_ */
