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
 *  DirectedVertex.h
 *
 *  Created on: Oct 13, 2010
 *      Author: nick
 */

#ifndef DIRECTEDVERTEX_H_
#define DIRECTEDVERTEX_H_

#include "Vertex.h"

namespace repast {

/**
 * Used internally by repast graphs / networks to encapsulate the vertices
 * of a directed graph.
 *
 * @tparam V the type of object stored by in a Vertex.
 * @tparam E the EdgeType of the network.
 */
template<typename V, typename E>
class DirectedVertex : public Vertex<V, E> {

private:
	typedef typename Vertex<V,E>::AdjListMap AdjListMap;
	typedef typename Vertex<V,E>::AdjListMap::iterator AdjListMapIterator;
	typedef typename Vertex<V,E>::EdgeType EdgeType;

	AdjListMap* incoming, *outgoing;

public:
	/**
	 * Creates a DirectedVertex that will contain the specified item.
	 */
	DirectedVertex(boost::shared_ptr<V> item);
	virtual ~DirectedVertex();

	// doc inherited from Vertex
	virtual E* removeEdge(Vertex<V,E>* other, EdgeType type);

	// doc inherited from Vertex
	virtual E* findEdge(Vertex<V,E>* other, EdgeType type);

	// doc inherited from Vertex
	virtual void addEdge(Vertex<V,E>* other, E* edge, EdgeType type);

	// doc inherited from Vertex
	virtual void successors(std::vector<V*>& out);

	// doc inherited from Vertex
	virtual void predecessors(std::vector<V*>& out);
	// doc inherited from Vertex
	virtual void adjacent(std::vector<V*>& out);

	// doc inherited from Vertex
	virtual void edges(EdgeType type , std::vector<E*>& out);

	// doc inherited from Vertex
	int inDegree();

	// doc inherited from Vertex
	int outDegree();
};

template<typename V, typename E>
DirectedVertex<V,E>::DirectedVertex(boost::shared_ptr<V> item) : Vertex<V,E>(item) {
	incoming = new AdjListMap();
	outgoing = new AdjListMap();
}

template<typename V, typename E>
DirectedVertex<V,E>::~DirectedVertex() {
	for (AdjListMapIterator iter = incoming->begin(); iter != incoming->end(); ++iter) {
		// delete the edge
		delete iter->second;
		iter->first->removeEdge(this, Vertex<V,E>::OUTGOING);
	}
	delete incoming;

	if (outgoing != 0) {
		for (AdjListMapIterator iter = outgoing->begin(); iter != outgoing->end(); ++iter) {
			// delete the edge
			delete iter->second;
			iter->first->removeEdge(this, Vertex<V,E>::INCOMING);
		}
	}
	delete outgoing;
}

template<typename V, typename E>
E* DirectedVertex<V,E>::removeEdge(Vertex<V,E>* other, EdgeType type) {
	AdjListMap* adjMap = outgoing;
	if (type == Vertex<V,E>::INCOMING) {
		adjMap = incoming;
	}
	return Vertex<V,E>::removeEdge(other, adjMap);
}

template<typename V, typename E>
E* DirectedVertex<V,E>::findEdge(Vertex<V,E>* other, EdgeType type) {
	AdjListMap* adjMap = outgoing;
	if (type == Vertex<V,E>::INCOMING) {
		adjMap = incoming;
	}

	AdjListMapIterator iter = adjMap->find(other);
	if (iter == adjMap->end()) return NULL;
	return iter->second;
}

template<typename V, typename E>
void DirectedVertex<V,E>::addEdge(Vertex<V,E>* other, E* edge, EdgeType type) {
	if (type == Vertex<V,E>::INCOMING) (*incoming)[other] = edge;
	else (*outgoing)[other] = edge;
}

template<typename V, typename E>
void DirectedVertex<V,E>::successors(std::vector<V*>& out) {
	getItems(outgoing, out);
}

template<typename V, typename E>
void DirectedVertex<V,E>::predecessors(std::vector<V*>& out) {
	getItems(incoming, out);
}

template<typename V, typename E>
void DirectedVertex<V,E>::adjacent(std::vector<V*>& out) {
	getItems(incoming, out);
	getItems(outgoing, out);
}

template<typename V, typename E>
int DirectedVertex<V,E>::inDegree() {
	return incoming->size();
}

template<typename V, typename E>
int DirectedVertex<V,E>::outDegree() {
	return outgoing->size();
}

template<typename V, typename E>
void DirectedVertex<V,E>::edges(EdgeType type, std::vector<E*>& out) {
	AdjListMap* map = outgoing;
	if (type == Vertex<V,E>::INCOMING) map = incoming;
	for (AdjListMapIterator iter = map->begin(); iter != map->end(); ++iter) {
		out.push_back(iter->second);
	}
}

}



#endif /* DIRECTEDVERTEX_H_ */
