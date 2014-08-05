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
 * @tparam E the edge type of the graph. This type should extend
 * repast::RepastEdge.
 * @tparam Ec class of serializable Edge Content
 * @tparam EcM Class that is capable of transforming an Edge into Edge Content and
 * vice versa
 */
template<typename V, typename E, typename Ec, typename EcM>
class Graph: public Projection<V> {

protected:
  typedef boost::unordered_map<AgentId, Vertex<V, E>*, HashId> VertexMap;
  typedef typename VertexMap::iterator VertexMapIterator;

  typedef typename Projection<V>::RADIUS RADIUS;

  int edgeCount_;
  bool isDirected;
  VertexMap vertices;

  EcM* edgeContentManager;

  void cleanUp();
  void init(const Graph& graph);

  virtual bool addAgent(boost::shared_ptr<V> agent);
  virtual void removeAgent(V* agent);

  virtual void doAddEdge(boost::shared_ptr<E> edge, bool allowOverwrite = true);

public:


  /**
   * An iterator over the agents that are the vertices in this Graph.
   */
  typedef typename boost::transform_iterator<NodeGetter<V, E> , typename VertexMap::const_iterator> vertex_iterator;

  std::set<int> ranksToSendProjInfoTo;              // Set these if the ranks for exchanging projection info are known
  std::set<int> ranksToReceiveProjInfoFrom;

  std::set<int> ranksToSendAgentStatusInfoTo;       // Set these if the ranks for exchanging agent status info are known
  std::set<int> ranksToReceiveAgentStatusInfoFrom;

  bool keepsAgents;          // Set this to false if you are CERTAIN that this projection will never 'keep' an agent during a proj info sync
  bool sendsSecondaryAgents; // Set this to false if you are CERTAIN that this projection will never send secondary agents during an agent status sync (VERY RARE)

  /**
   * Creates a Graph with the specified name.
   *
   * @param name the name of the graph
   * @param directed whether or not the created Graph is directed
   */
  Graph(std::string name, bool directed, EcM* edgeContentMgr) :
    Projection<V> (name), edgeCount_(0), isDirected(directed), edgeContentManager(edgeContentMgr), keepsAgents(true), sendsSecondaryAgents(true) {
  }

  /**
   * Copy constructor for the graph.
   */
  Graph(const Graph<V, E, Ec, EcM>& graph);
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

  void showEdges();


  // Beta
  virtual bool isMaster(E* e) = 0;

  virtual bool keepsAgentsOnSyncProj(){ return keepsAgents; }

  virtual bool sendsSecondaryAgentsOnStatusExchange(){ return sendsSecondaryAgents; }

  virtual void getInfoExchangePartners(std::set<int>& psToSendTo, std::set<int>& psToReceiveFrom);

  virtual void getAgentStatusExchangePartners(std::set<int>& psToSendTo, std::set<int>& psToReceiveFrom);

  virtual void getProjectionInfo(std::vector<AgentId>& agents, std::vector<ProjectionInfoPacket*>& packets,
        bool secondaryInfo = false, std::set<AgentId>* secondaryIds = 0, int destProc = -1);

  virtual ProjectionInfoPacket* getProjectionInfo(AgentId id, bool secondaryInfo = false, std::set<AgentId>* secondaryIds = 0, int destProc = -1 );

  virtual void updateProjectionInfo(ProjectionInfoPacket* pip, Context<V>* context);

  virtual void getRequiredAgents(std::set<AgentId>& agentsToTest, std::set<AgentId>& agentsRequired, RADIUS radius =Projection<V>::PRIMARY);

  virtual void getAgentsToPush(std::set<AgentId>& agentsToTest, std::map<int, std::set<AgentId> >& agentsToPush);

  virtual void cleanProjectionInfo(std::set<AgentId>& agentsToKeep);

  void clearConflictedEdges();

  void getConflictedEdges(std::set<boost::shared_ptr<E> >& conflictedEdges);

};

template<typename V, typename E, typename Ec, typename EcM>
Graph<V, E, Ec, EcM>::~Graph() {
  cleanUp();
}

template<typename V, typename E, typename Ec, typename EcM>
void Graph<V, E, Ec, EcM>::cleanUp() {
  for (VertexMapIterator iter = vertices.begin(); iter != vertices.end(); ++iter) {
    delete iter->second;
  }
  vertices.clear();
}

template<typename V, typename E, typename Ec, typename EcM>
Graph<V, E, Ec, EcM>::Graph(const Graph<V, E, Ec, EcM>& graph) {
  init(graph);
}

template<typename V, typename E, typename Ec, typename EcM>
void Graph<V, E, Ec, EcM>::init(const Graph<V, E, Ec, EcM>& graph) {
  edgeCount_         = graph.edgeCount_;
  isDirected         = graph.isDirected;
  edgeContentManager = graph.edgeContentManager;

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

template<typename V, typename E, typename Ec, typename EcM>
Graph<V, E, Ec, EcM>& Graph<V, E, Ec, EcM>::operator=(const Graph<V, E, Ec, EcM>& graph) {
  if (this != &graph) {
    cleanUp();
    init(graph);
  }

  return *this;
}

template<typename V, typename E, typename Ec, typename EcM>
boost::shared_ptr<E> Graph<V, E, Ec, EcM>::addEdge(V* source, V* target) {
  boost::shared_ptr<E> ret;

  const VertexMapIterator notFound = Graph<V, E, Ec, EcM>::vertices.end();

  VertexMapIterator srcIter    = vertices.find(source->getId());
  if (srcIter == notFound)    return ret;

  VertexMapIterator targetIter = vertices.find(target->getId());
  if (targetIter == notFound) return ret;

  boost::shared_ptr<E> edge(new E(srcIter->second->item(), targetIter->second->item()));
  doAddEdge(edge);
  return edge;
}

template<typename V, typename E, typename Ec, typename EcM>
boost::shared_ptr<E> Graph<V, E, Ec, EcM>::addEdge(V* source, V* target, double weight) {
  boost::shared_ptr<E> ret;

  const VertexMapIterator notFound = Graph<V, E, Ec, EcM>::vertices.end();

  VertexMapIterator srcIter    = vertices.find(source->getId());
  if (srcIter == notFound)    return ret;

  VertexMapIterator targetIter = vertices.find(target->getId());
  if (targetIter == notFound) return ret;

  boost::shared_ptr<E> edge(new E(srcIter->second->item(), targetIter->second->item(), weight));
  doAddEdge(edge);
  return edge;
}

template<typename V, typename E, typename Ec, typename EcM>
void Graph<V, E, Ec, EcM>::successors(V* vertex, std::vector<V*>& out) {
  VertexMapIterator iter = Graph<V, E, Ec, EcM>::vertices.find(vertex->getId());
  if (iter != Graph<V, E, Ec, EcM>::vertices.end()) iter->second->successors(out);
}

template<typename V, typename E, typename Ec, typename EcM>
void Graph<V, E, Ec, EcM>::predecessors(V* vertex, std::vector<V*>& out) {
  VertexMapIterator iter = Graph<V, E, Ec, EcM>::vertices.find(vertex->getId());
  if (iter != vertices.end()) iter->second->predecessors(out);
}

template<typename V, typename E, typename Ec, typename EcM>
void Graph<V, E, Ec, EcM>::adjacent(V* vertex, std::vector<V*>& out) {
  VertexMapIterator iter = Graph<V, E, Ec, EcM>::vertices.find(vertex->getId());
  if (iter != vertices.end()) iter->second->adjacent(out);
}

template<typename V, typename E, typename Ec, typename EcM>
int Graph<V, E, Ec, EcM>::inDegree(V* vertex) {
  VertexMapIterator iter = Graph<V, E, Ec, EcM>::vertices.find(vertex->getId());
  return (iter != vertices.end() ? iter->second->inDegree() : 0);
}

template<typename V, typename E, typename Ec, typename EcM>
int Graph<V, E, Ec, EcM>::outDegree(V* vertex) {
  VertexMapIterator iter = Graph<V, E, Ec, EcM>::vertices.find(vertex->getId());
  return (iter != vertices.end() ? iter->second->outDegree() : 0);
}

template<typename V, typename E, typename Ec, typename EcM>
void Graph<V, E, Ec, EcM>::removeEdge(const AgentId& sourceId, const AgentId& targetId) {
  const VertexMapIterator vertexNotFound = Graph<V, E, Ec, EcM>::vertices.end();

  VertexMapIterator iter = Graph<V, E, Ec, EcM>::vertices.find(sourceId);
  if (iter == vertexNotFound) return;
  Vertex<V, E>* sVert = iter->second;

  iter = Graph<V, E, Ec, EcM>::vertices.find(targetId);
  if (iter == vertexNotFound) return;
  Vertex<V, E>* tVert = iter->second;

  boost::shared_ptr<E> edgeNotFound;
  if(sVert->removeEdge(tVert, Vertex<V, E>::OUTGOING) != edgeNotFound) edgeCount_--;
  tVert->removeEdge(sVert, Vertex<V, E>::INCOMING);

}

template<typename V, typename E, typename Ec, typename EcM>
void Graph<V, E, Ec, EcM>::removeEdge(V* source, V* target) {
  removeEdge(source->getId(), target->getId());
}

template<typename V, typename E, typename Ec, typename EcM>
void Graph<V, E, Ec, EcM>::removeAgent(V* vertex) {
  VertexMapIterator iter = Graph<V, E, Ec, EcM>::vertices.find(vertex->getId());
  if (iter != vertices.end()) {
    Vertex<V, E>* iVert = iter->second;
    std::vector<V*> adjacentVertices;
    iVert->adjacent(adjacentVertices);
    for(typename std::vector<V*>::iterator adjIter = adjacentVertices.begin(), adjIterEnd = adjacentVertices.end(); adjIter != adjIterEnd; ++adjIter){
      removeEdge(vertex->getId(), (*adjIter)->getId());
      removeEdge((*adjIter)->getId(), vertex->getId());
    }

    delete iVert;
    vertices.erase(iter);
  }
}

template<typename V, typename E, typename Ec, typename EcM>
bool Graph<V, E, Ec, EcM>::addAgent(boost::shared_ptr<V> agent) {
  if(!Projection<V>::agentCanBeAdded(agent)) return false;
  if (vertices.find(agent->getId()) != vertices.end()) return false;

  if(isDirected) vertices[agent->getId()] = new DirectedVertex<V, E> (agent);
  else           vertices[agent->getId()] = new UndirectedVertex<V, E> (agent);

  return true;
}

template<typename V, typename E, typename Ec, typename EcM>
boost::shared_ptr<E> Graph<V, E, Ec, EcM>::findEdge(V* source, V* target) {
  boost::shared_ptr<E> ret;

  const VertexMapIterator notFound = Graph<V, E, Ec, EcM>::vertices.end();

  VertexMapIterator sIter = vertices.find(source->getId());
  if (sIter == notFound) return ret;

  VertexMapIterator tIter = vertices.find(target->getId());
  if (tIter == notFound) return ret;

  return sIter->second->findEdge(tIter->second, Vertex<V, E>::OUTGOING);
}

template<typename V, typename E, typename Ec, typename EcM>
void Graph<V, E, Ec, EcM>::doAddEdge(boost::shared_ptr<E> edge, bool allowOverwrite) {
  V* source = edge->source();
  V* target = edge->target();

  Vertex<V, E>* vSource = vertices[source->getId()];
  Vertex<V, E>* vTarget = vertices[target->getId()];

  boost::shared_ptr<E> notFound;
  boost::shared_ptr<E> extant = vSource->findEdge(vTarget, Vertex<V, E>::OUTGOING);
  if(extant == notFound){
    vSource->addEdge(vTarget, edge, Vertex<V, E>::OUTGOING);
    vTarget->addEdge(vSource, edge, Vertex<V, E>::INCOMING);
    edgeCount_++;
  }
  else{
    if(allowOverwrite){
      vSource->removeEdge(vTarget, Vertex<V, E>::OUTGOING);
      vTarget->removeEdge(vSource, Vertex<V, E>::INCOMING);

      vSource->addEdge(vTarget, edge, Vertex<V, E>::OUTGOING);
      vTarget->addEdge(vSource, edge, Vertex<V, E>::INCOMING);
    }
    else extant->markConflicted();
  }
}

template<typename V, typename E, typename Ec, typename EcM>
void Graph<V, E, Ec, EcM>::showEdges(){
  std::set<boost::shared_ptr<E> > edgeSet;
  for(typename VertexMap::iterator iter = vertices.begin(), iterEnd = vertices.end(); iter != iterEnd; ++iter){
    std::vector<boost::shared_ptr<E> > edges;
    (*iter).second->edges(repast::Vertex<V, E>::INCOMING, edges);
    (*iter).second->edges(repast::Vertex<V, E>::OUTGOING, edges);
    for(typename std::vector<boost::shared_ptr<E> >::iterator EI = edges.begin(), EIEnd = edges.end(); EI != EIEnd; ++EI) edgeSet.insert(*EI);
  }
  for(typename std::set<boost::shared_ptr<E> >::iterator ei = edgeSet.begin(), eiEnd = edgeSet.end(); ei != eiEnd; ++ei){
    std::cout << "SOURCE: " << (*ei)->source()->getId() << " TARGET: " << (*ei)->target()->getId() << " " << (isMaster(&**ei) ? "MASTER" : "NONLOCAL") << " Weight = " << (*ei)->weight() << std::endl;
  }
}



// Beta

template<typename V, typename E, typename Ec, typename EcM>
void Graph<V, E, Ec, EcM>::getInfoExchangePartners(std::set<int>& psToSendTo, std::set<int>& psToReceiveFrom){
  psToSendTo.insert(ranksToSendProjInfoTo.begin(), ranksToSendProjInfoTo.end());
  psToReceiveFrom.insert(ranksToReceiveProjInfoFrom.begin(), ranksToReceiveProjInfoFrom.end());
}

template<typename V, typename E, typename Ec, typename EcM>
void Graph<V, E, Ec, EcM>::getAgentStatusExchangePartners(std::set<int>& psToSendTo, std::set<int>& psToReceiveFrom){
  psToSendTo.insert(ranksToSendAgentStatusInfoTo.begin(), ranksToSendAgentStatusInfoTo.end());
  psToReceiveFrom.insert(ranksToReceiveAgentStatusInfoFrom.begin(), ranksToReceiveAgentStatusInfoFrom.end());
}


template<typename V, typename E, typename Ec, typename EcM>
void Graph<V, E, Ec, EcM>::getProjectionInfo(std::vector<AgentId>& agents, std::vector<ProjectionInfoPacket*>& packets,
      bool secondaryInfo, std::set<AgentId>* secondaryIds, int destProc){
  if(secondaryInfo == false) return; // Can be skipped entirely for graphs
  // If not, call the superclass's implementation
  Projection<V>::getProjectionInfo(agents, packets, secondaryInfo, secondaryIds, destProc);
}


template<typename V, typename E, typename Ec, typename EcM>
ProjectionInfoPacket* Graph<V, E, Ec, EcM>::getProjectionInfo(AgentId id, bool secondaryInfo, std::set<AgentId>* secondaryIds, int destProc ){
  if(secondaryInfo == false) return 0; // All graph projection info is secondary; if not returning it, done.

  VertexMapIterator agent = vertices.find(id);
  if(agent == vertices.end()) return 0;  // The requested agent is not in this graph

  std::vector<Ec> edgeContent;

  std::vector<boost::shared_ptr<E> > edges;
  agent->second->edges(Vertex<V, E>::INCOMING, edges);
  agent->second->edges(Vertex<V, E>::OUTGOING, edges);
  // Sometimes the incoming and outgoing edges are the same; purge duplicates
  std::set<boost::shared_ptr<E> > edgeSet;
  edgeSet.insert(edges.begin(), edges.end());
  // Make all four instances of the loop to optimize for each case
  AgentId sourceId;
  AgentId targetId;
  AgentId otherId;

  if(secondaryIds == 0){
    if(destProc > -1){
      for(typename std::set<boost::shared_ptr<E> >::iterator iter = edgeSet.begin(), iterEnd = edgeSet.end(); iter != iterEnd; ++iter){
        sourceId = (*iter)->source()->getId();
        targetId = (*iter)->target()->getId();
        otherId = (sourceId != id ? sourceId : targetId);
        if(otherId.currentRank() == destProc)  edgeContent.push_back(*(edgeContentManager->provideEdgeContent(iter->get())));
      }
    }
    else{
      for(typename std::set<boost::shared_ptr<E> >::iterator iter = edgeSet.begin(), iterEnd = edgeSet.end(); iter != iterEnd; ++iter){
        sourceId = (*iter)->source()->getId();
        targetId = (*iter)->target()->getId();
        otherId = (sourceId != id ? sourceId : targetId);
        edgeContent.push_back(*(edgeContentManager->provideEdgeContent(iter->get())));
      }
    }
  }
  else{
    if(destProc > -1){
      for(typename std::set<boost::shared_ptr<E> >::iterator iter = edgeSet.begin(), iterEnd = edgeSet.end(); iter != iterEnd; ++iter){
        sourceId = (*iter)->source()->getId();
        targetId = (*iter)->target()->getId();
        otherId = (sourceId != id ? sourceId : targetId);
        if(otherId.currentRank() == destProc){
          secondaryIds->insert(otherId);
          edgeContent.push_back(*(edgeContentManager->provideEdgeContent(iter->get())));
        }
      }
    }
    else{
      for(typename std::set<boost::shared_ptr<E> >::iterator iter = edgeSet.begin(), iterEnd = edgeSet.end(); iter != iterEnd; ++iter){
        sourceId = (*iter)->source()->getId();
        targetId = (*iter)->target()->getId();
        otherId = (sourceId != id ? sourceId : targetId);
        secondaryIds->insert(otherId);
        edgeContent.push_back(*(edgeContentManager->provideEdgeContent(iter->get())));
      }
    }
  }

  return new SpecializedProjectionInfoPacket<Ec>(id, edgeContent);
}

template<typename V, typename E, typename Ec, typename EcM>
void Graph<V, E, Ec, EcM>::updateProjectionInfo(ProjectionInfoPacket* pip, Context<V>* context){
  SpecializedProjectionInfoPacket<Ec>* spip = static_cast<SpecializedProjectionInfoPacket<Ec>*>(pip);
  std::vector<Ec> &edges = spip->data;
  for(int i = 0; i < edges.size(); i++){
    boost::shared_ptr<E> newEdge(edgeContentManager->createEdge(edges[i], context));
    doAddEdge(newEdge, false);
  }
}



template<typename V, typename E, typename Ec, typename EcM>
void Graph<V, E, Ec, EcM>::getRequiredAgents(std::set<AgentId>& agentsToTest, std::set<AgentId>& agentsRequired, RADIUS radius){
  switch(radius){
    case Projection<V>::PRIMARY: {// Keep only the nonlocal ends of MASTER edges
    std::set<AgentId>::iterator iter = agentsToTest.begin();
      while(iter != agentsToTest.end()){
        VertexMapIterator vertex = Graph<V, E, Ec, EcM>::vertices.find(*iter);
        if(vertex != vertices.end()){
          std::vector<boost::shared_ptr<E> > edges;
          vertex->second->edges(Vertex<V, E>::INCOMING, edges);
          vertex->second->edges(Vertex<V, E>::OUTGOING, edges);
          std::set<boost::shared_ptr<E> > edgeSet;
          edgeSet.insert(edges.begin(), edges.end());
          edges.clear();
          edges.assign(edgeSet.begin(), edgeSet.end());
          bool keep = false;
          for(typename std::vector<boost::shared_ptr<E> >::iterator edgeIter = edges.begin(), edgeIterEnd = edges.end(); edgeIter != edgeIterEnd; edgeIter++){
            if(isMaster(&**edgeIter)){
              agentsRequired.insert(*iter);
              keep = true;
              break;
            }
          }
          if(keep){
            std::set<AgentId>::iterator iterTEMP = iter;
            iter++;
            agentsToTest.erase(*iterTEMP);
          }
          else iter++;
        }
        else iter++;
      }
      break;
    }
    case Projection<V>::SECONDARY: {// Keep any nonlocal agent that is in any edge
      std::set<AgentId>::iterator iter = agentsToTest.begin();
      while(iter != agentsToTest.end()){
        VertexMapIterator vertex = Graph<V, E, Ec, EcM>::vertices.find(*iter);
        if(vertex != vertices.end()){
          std::vector<boost::shared_ptr<E> > edges;
          vertex->second->edges(Vertex<V, E>::INCOMING, edges);
          vertex->second->edges(Vertex<V, E>::OUTGOING, edges);
          if(edges.size() > 0){
            std::set<AgentId>::iterator iterTEMP = iter;
            iter++;
            agentsToTest.erase(*iterTEMP);
          }
          else iter++;
        }
        else iter++;
      }
      break;
    }
  }
}

template<typename V, typename E, typename Ec, typename EcM>
void Graph<V, E, Ec, EcM>::getAgentsToPush(std::set<AgentId>& agentsToTest, std::map<int, std::set<AgentId> >& agentsToPush){
  if(agentsToTest.size() == 0) return;
  // The local agent ends of master edges must be pushed to the process of the non-local end
  std::set<AgentId>::iterator iter = agentsToTest.begin();
  while(iter != agentsToTest.end()){
    VertexMapIterator vertexMapEntry = Graph<V, E, Ec, EcM>::vertices.find(*iter);
    if(vertexMapEntry != vertices.end()){
      int localRank = vertexMapEntry->second->item()->getId().currentRank();
      std::vector<boost::shared_ptr<E> > edges;
      vertexMapEntry->second->edges(Vertex<V, E>::INCOMING, edges);
      vertexMapEntry->second->edges(Vertex<V, E>::OUTGOING, edges);
      for(typename std::vector<boost::shared_ptr<E> >::iterator edgeIter = edges.begin(), edgeIterEnd = edges.end(); edgeIter != edgeIterEnd; ++edgeIter){
        boost::shared_ptr<E> e = *edgeIter;
        if(isMaster(&**edgeIter)){
          AgentId sourceId = (*edgeIter)->source()->getId();
          AgentId targetId = (*edgeIter)->target()->getId();
          AgentId otherAgentId = (sourceId != *iter ? sourceId : targetId);
          int destRank = otherAgentId.currentRank();
          if(destRank != localRank){
            agentsToPush[destRank].insert(*iter);
            break; // Out of 'for' loop
          }
        }
      }
    }
    iter++;
  }
}

template<typename V, typename E, typename Ec, typename EcM>
void Graph<V, E, Ec, EcM>::cleanProjectionInfo(std::set<AgentId>& agentsToKeep){
  for(std::set<AgentId>::iterator iter = agentsToKeep.begin(), iterEnd = agentsToKeep.end(); iter != iterEnd; ++iter){
    VertexMapIterator vertexMapEntry = Graph<V, E, Ec, EcM>::vertices.find(*iter);
    if(vertexMapEntry != vertices.end()){
      std::vector<boost::shared_ptr<E> > edges;
      vertexMapEntry->second->edges(Vertex<V, E>::INCOMING, edges);
      for(typename std::vector<boost::shared_ptr<E> >::iterator edgeIter = edges.begin(), edgeIterEnd = edges.end(); edgeIter != edgeIterEnd; ++edgeIter){
        if(!isMaster(&**edgeIter)) removeEdge((*edgeIter)->source(), (*edgeIter)->target());
      }
      edges.clear();
      vertexMapEntry->second->edges(Vertex<V, E>::OUTGOING, edges);
      for(typename std::vector<boost::shared_ptr<E> >::iterator edgeIter = edges.begin(), edgeIterEnd = edges.end(); edgeIter != edgeIterEnd; ++edgeIter){
        if(!isMaster(&**edgeIter)) removeEdge((*edgeIter)->source(), (*edgeIter)->target());
      }
    }
  }
}

template<typename V, typename E, typename Ec, typename EcM>
void Graph<V, E, Ec, EcM>::clearConflictedEdges(){
  for(typename VertexMap::iterator iter = vertices.begin(), iterEnd = vertices.end(); iter != iterEnd; ++iter){
    std::vector<boost::shared_ptr<E> > edges;
    (*iter).second->edges(repast::Vertex<V, E>::INCOMING, edges);
    (*iter).second->edges(repast::Vertex<V, E>::OUTGOING, edges);
    for(typename std::vector<boost::shared_ptr<E> >::iterator EI = edges.begin(), EIEnd = edges.end(); EI != EIEnd; ++EI){
      (*EI)->clearConflicted();
    }
  }
}

template<typename V, typename E, typename Ec, typename EcM>
void Graph<V, E, Ec, EcM>::getConflictedEdges(std::set<boost::shared_ptr<E> >& conflictedEdges){
  for(typename VertexMap::iterator iter = vertices.begin(), iterEnd = vertices.end(); iter != iterEnd; ++iter){
    std::vector<boost::shared_ptr<E> > edges;
    (*iter).second->edges(repast::Vertex<V, E>::INCOMING, edges);
    (*iter).second->edges(repast::Vertex<V, E>::OUTGOING, edges);
    for(typename std::vector<boost::shared_ptr<E> >::iterator EI = edges.begin(), EIEnd = edges.end(); EI != EIEnd; ++EI){
      if((*EI)->isConflicted()) conflictedEdges.insert(*EI);
    }
  }
}


}

#endif /* GRAPH_H_ */
