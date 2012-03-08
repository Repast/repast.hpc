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
 *  Graph.h
 *
 *  Created on: Dec 23, 2008
 *      Author: nick
 */

#ifndef GRAPH_H_
#define GRAPH_H_

#include "AgentId.h"
#include "Projection.h"
#include "Edge.h"
#include "DirectedVertex.h"
#include "UndirectedVertex.h"

#include <vector>
#include <utility>
#include <map>
#include <iostream>
#include <boost/unordered_map.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/serialization/access.hpp>
#include <boost/iterator/transform_iterator.hpp>

namespace repast {

/**
 * Graph / Network implementation where agents are vertices in the graph.
 *
 * @tparam V the type agents in the graph. This type should extend repast::Agent
 * @tparam E the edge type of the graph. This type should extent
 * repast::RepastEdge.
 *
 */
template<typename V, typename E>
class Graph: public Projection<V> {

protected:
	typedef boost::unordered_map<AgentId, Vertex<V, E>*, HashId> VertexMap;
	typedef typename VertexMap::iterator VertexMapIterator;

	int edgeCount_;
	bool isDirected;
	VertexMap vertices;

	void cleanUp();
	void init(const Graph& graph);

	virtual bool addAgent(boost::shared_ptr<V> agent);
	virtual void removeAgent(V* agent);

	virtual void doAddEdge(boost::shared_ptr<E> edge);

public:

	/**
	 * An iterator over the agents that are the vertices in this Graph.
	 */
	typedef typename boost::transform_iterator<NodeGetter<V, E> , typename VertexMap::const_iterator> vertex_iterator;

	/**
	 * Creates a Graph with the specified name.
	 *
	 * @param name the name of the graph
	 * @param directed whether or not the created Graph is directed
	 */
	Graph(std::string name, bool directed) :
		Projection<V> (name), edgeCount_(0), isDirected(directed) {
	}

	/**
	 * Copy constructor for the graph.
	 */
	Graph(const Graph<V, E>& graph);
	virtual ~Graph();

	// assignment
	Graph& operator=(const Graph& graph);

	/**
	 * Adds an edge between source and target to this Graph.
	 *
	 * @param source the source of the edge
	 * @param target the target of the edge
	 *
	 * @return the added edge.
	 */
	virtual boost::shared_ptr<E> addEdge(V* source, V* target);

	/**
	 * Adds an edge with the specified weight between source and target to this Graph.
	 *
	 * @param source the source of the edge
	 * @param target the target of the edge
	 * @param weight the weight of the edge
	 *
	 * @return the added edge.
	 */
	virtual boost::shared_ptr<E> addEdge(V* source, V* target, double weight);

	/**
	 * Gets the edge between the source and target or 0
	 * if no such edge is found.
	 *
	 * @param source the source of the edge to find
	 * @param target the target of the edge to find
	 *
	 * @return the found edge or 0.
	 */
	virtual boost::shared_ptr<E> findEdge(V* source, V* target);

	/**
	 * Gets the sucessors of the specified vertex and puts them in
	 * out.
	 *
	 * @param vertex the vertex whose successors we want to get
	 * @param [out] where the successors will be returned
	 */
	virtual void successors(V* vertex, std::vector<V*>& out);

	/**
	 * Gets the predecessors of the specified vertex and puts them in
	 * out.
	 *
	 * @param vertex the vertex whose predecessors we want to get
	 * @param [out] where the predecessors will be returned
	 */
	virtual void predecessors(V* vertex, std::vector<V*>& out);

	/**
	 * Gets all the agent adjacent to the specified vertex.
	 *
	 * @param vertex the vertex whose adjacent agents we want to get
	 * @param [out] the vector where the results will be put
	 */
	virtual void adjacent(V* vertex, std::vector<V*>& out);

	/**
	 * Removes the edge between source and target from this Graph.
	 *
	 * @param source the source of the edge
	 * @param target the target of the edge
	 */
	virtual void removeEdge(V* source, V* target);

	/**
	 * Removes the edge between source and target from this Graph.
	 *
	 * @param source the id of the vertex that is the source of the edge
	 * @param target the id of the vertex that is the target of the edge
	 */
	virtual void removeEdge(const AgentId& source, const AgentId& target);

	/**
	 * Gets the in-degree of the specified vertex.
	 *
	 * @return  the in-degree of the specified vertex.
	 */
	virtual int inDegree(V* vertex);

	/**
	 * Gets the out-degree of the specified vertex.
	 *
	 * @return  the out-degree of the specified vertex.
	 */
	virtual int outDegree(V* vertex);

	/**
	 * Gets the number of edges in this Graph.
	 *
	 * @return the number of edges in this Graph.
	 */
	int edgeCount() const {
		return edgeCount_;
	}

	/**
	 * Gets the number of vertices in this Graph.
	 *
	 * @return the number of vertices in this Graph.
	 */
	int vertexCount() const {
		return vertices.size();
	}

	/**
	 * Gets the start of an iterator over all the vertices in this graph.
	 * The iterator dereferences to a pointer to agents of type V.
	 *
	 * @return the start of an iterator over all the vertices in this graph.
	 */
	vertex_iterator verticesBegin() {
		return vertex_iterator(vertices.begin());
	}

	/**
	 * Gets the end of an iterator over all the vertices in this graph.
	 * The iterator dereferences to a pointer to agents of type V.
	 *
	 * @return the end of an iterator over all the vertices in this graph.
	 */
	vertex_iterator verticesEnd() {
		return vertex_iterator(vertices.end());
	}
};

template<typename V, typename E>
Graph<V, E>::~Graph() {
	cleanUp();
}

template<typename V, typename E>
void Graph<V, E>::cleanUp() {
	for (VertexMapIterator iter = vertices.begin(); iter != vertices.end(); ++iter) {
		delete iter->second;
	}
	vertices.clear();
}

template<typename V, typename E>
Graph<V, E>::Graph(const Graph<V, E>& graph) {
	init(graph);
}

template<typename V, typename E>
void Graph<V, E>::init(const Graph<V, E>& graph) {
	edgeCount_ = graph.edgeCount_;
	isDirected = graph.isDirected;

	// create new vertices from the old ones
	for (VertexMapIterator iter = graph.vertices.begin(); iter != graph.vertices.end(); ++iter) {
		Vertex<V, E>* vertex = iter->second;
		if (isDirected) {
			vertices[iter->first] = new DirectedVertex<V, E> (vertex->item());
		} else {
			vertices[iter->first] = new UndirectedVertex<V, E> (vertex->item());
		}
	}

	// fill adj list maps using the new vertex info.
	// create new vertices from the old ones
	for (VertexMapIterator iter = graph.vertices.begin(); iter != graph.vertices.end(); ++iter) {
		Vertex<V, E>* vertex = iter->second;
		Vertex<V, E>* newVert = vertices[iter->first];
		std::vector<boost::shared_ptr<E> > edges;

		vertex->edges(Vertex<V, E>::OUTGOING, edges);
		for (typename std::vector<boost::shared_ptr<E> >::iterator iter = edges.begin(); iter != edges.end(); ++iter) {
			// create new edge and add it
			boost::shared_ptr<E> newEdge(new E(**iter));
			doAddEdge(newEdge);
		}
	}
}

template<typename V, typename E>
Graph<V, E>& Graph<V, E>::operator=(const Graph<V, E>& graph) {
	if (this != &graph) {
		cleanUp();
		init(graph);
	}

	return *this;
}

template<typename V, typename E>
boost::shared_ptr<E> Graph<V, E>::addEdge(V* source, V* target) {
  boost::shared_ptr<E> ret;

  const VertexMapIterator notFound = Graph<V, E>::vertices.end();

	VertexMapIterator srcIter    = vertices.find(source->getId());
	if (srcIter == notFound)    return ret;

	VertexMapIterator targetIter = vertices.find(target->getId());
	if (targetIter == notFound) return ret;

	boost::shared_ptr<E> edge(new E(srcIter->second->item(), targetIter->second->item()));
	doAddEdge(edge);
	return edge;
}

template<typename V, typename E>
boost::shared_ptr<E> Graph<V, E>::addEdge(V* source, V* target, double weight) {
  boost::shared_ptr<E> ret;

  const VertexMapIterator notFound = Graph<V, E>::vertices.end();

  VertexMapIterator srcIter    = vertices.find(source->getId());
  if (srcIter == notFound)    return ret;

  VertexMapIterator targetIter = vertices.find(target->getId());
  if (targetIter == notFound) return ret;

  boost::shared_ptr<E> edge(new E(srcIter->second->item(), targetIter->second->item(), weight));
  doAddEdge(edge);
  return edge;
}

template<typename V, typename E>
void Graph<V, E>::successors(V* vertex, std::vector<V*>& out) {
	VertexMapIterator iter = Graph<V, E>::vertices.find(vertex->getId());
	if (iter != Graph<V, E>::vertices.end()) iter->second->successors(out);
}

template<typename V, typename E>
void Graph<V, E>::predecessors(V* vertex, std::vector<V*>& out) {
	VertexMapIterator iter = Graph<V, E>::vertices.find(vertex->getId());
	if (iter != vertices.end()) iter->second->predecessors(out);
}

template<typename V, typename E>
void Graph<V, E>::adjacent(V* vertex, std::vector<V*>& out) {
	VertexMapIterator iter = Graph<V, E>::vertices.find(vertex->getId());
	if (iter != vertices.end()) iter->second->adjacent(out);
}

template<typename V, typename E>
int Graph<V, E>::inDegree(V* vertex) {
	VertexMapIterator iter = Graph<V, E>::vertices.find(vertex->getId());
	return (iter != vertices.end() ? iter->second->inDegree() : 0);
}

template<typename V, typename E>
int Graph<V, E>::outDegree(V* vertex) {
	VertexMapIterator iter = Graph<V, E>::vertices.find(vertex->getId());
  return (iter != vertices.end() ? iter->second->outDegree() : 0);
}

template<typename V, typename E>
void Graph<V, E>::removeEdge(const AgentId& sourceId, const AgentId& targetId) {
  const VertexMapIterator vertexNotFound = Graph<V, E>::vertices.end();

  VertexMapIterator iter = Graph<V, E>::vertices.find(sourceId);
  if (iter == vertexNotFound) return;
  Vertex<V, E>* sVert = iter->second;

  iter = Graph<V, E>::vertices.find(targetId);
  if (iter == vertexNotFound) return;
  Vertex<V, E>* tVert = iter->second;

  boost::shared_ptr<E> edgeNotFound;
  if(sVert->removeEdge(tVert, Vertex<V, E>::OUTGOING) != edgeNotFound) edgeCount_--;
  tVert->removeEdge(sVert, Vertex<V, E>::INCOMING);

}

template<typename V, typename E>
void Graph<V, E>::removeEdge(V* source, V* target) {
	removeEdge(source->getId(), target->getId());
}

template<typename V, typename E>
void Graph<V, E>::removeAgent(V* vertex) {
	VertexMapIterator iter = Graph<V, E>::vertices.find(vertex->getId());
	if (iter != vertices.end()) {
		Vertex<V, E>* iVert = iter->second;
	  edgeCount_ -= (isDirected ? (iVert->inDegree() + iVert->outDegree()) : iVert->inDegree());
		delete iVert;
		vertices.erase(iter);
	}
}

template<typename V, typename E>
bool Graph<V, E>::addAgent(boost::shared_ptr<V> agent) {
	if (vertices.find(agent->getId()) != vertices.end())	return false;

	if(isDirected) vertices[agent->getId()] = new DirectedVertex<V, E> (agent);
	else           vertices[agent->getId()] = new UndirectedVertex<V, E> (agent);

	return true;
}

template<typename V, typename E>
boost::shared_ptr<E> Graph<V, E>::findEdge(V* source, V* target) {
  boost::shared_ptr<E> ret;

  const VertexMapIterator notFound = Graph<V, E>::vertices.end();

	VertexMapIterator sIter = vertices.find(source->getId());
	if (sIter == notFound) return ret;

	VertexMapIterator tIter = vertices.find(target->getId());
	if (tIter == notFound) return ret;

	return sIter->second->findEdge(tIter->second, Vertex<V, E>::OUTGOING);
}

template<typename V, typename E>
void Graph<V, E>::doAddEdge(boost::shared_ptr<E> edge) {
	V* source = edge->source();
	V* target = edge->target();

	Vertex<V, E>* vSource = vertices[source->getId()];
	Vertex<V, E>* vTarget = vertices[target->getId()];

	// Must remove any extant edge
  boost::shared_ptr<E> notFound;
  if(vSource->removeEdge(vTarget, Vertex<V, E>::OUTGOING) != notFound) edgeCount_--;
  vTarget->removeEdge(vSource, Vertex<V, E>::INCOMING);

	vSource->addEdge(vTarget, edge, Vertex<V, E>::OUTGOING);
	vTarget->addEdge(vSource, edge, Vertex<V, E>::INCOMING);

	edgeCount_++;
}

}

#endif /* GRAPH_H_ */
