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
 *  SharedNetwork.h
 *
 *  Created on: May 27, 2009
 *      Author: nick
 */

#ifndef SHAREDNETWORK_H_
#define SHAREDNETWORK_H_

#include <map>
#include <utility>
#include <set>

#include <boost/unordered_set.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/mpi.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

#include "Graph.h"
#include "RepastProcess.h"
#include "logger.h"
#include "Utilities.h"
#include "SRManager.h"
#include "RepastErrors.h"


namespace repast {

const int NET_SR_PAIR = 2000;
const int NET_RECV_PROC = 2001;
const int NET_EDGE_UPDATE = 2002;
const int NET_EXPORTERS_A = 2003;
const int NET_EXPORTERS_B = 2004;
const int NET_EXPORT_REQUESTS = 2005;
const int NET_EDGE_SYNC = 2006;
const int NET_EDGE_REMOVE_SYNC = 2007;


/**
 * Network implementation that can be shared across processes.
 * Networks are shared across processes by creating edges between
 * local and non-local agents on a process. The createComplementaryEdges
 * function will create complementary edges across processes in
 * those cases. For example, if an edge is created between A1 and B2 on process 1
 * where B2 is copy of B1 on process 2, then creating complementary edges
 * will create a copy of that edge on process 2, importing A1 into process 2
 * if necessary.
 *
 * @tparam V the agent (vertex) type
 * @tparam E the edge type. The edge type must be contain a constructor
 * that takes a source and target of type V and extends RepastEdge. RepastEdge
 * can also be used.
 */
template<typename V, typename E, typename Ec, typename EcM>
class SharedNetwork: public Graph<V, E, Ec, EcM> {
private:

  template<typename Vertex, typename Edge, typename AgentContent, typename EdgeContent, typename EdgeManager, typename AgentCreator>
  friend void createComplementaryEdges(SharedNetwork<Vertex, Edge, EdgeContent, EdgeManager>* net, SharedContext<Vertex>& context,
      EdgeManager& edgeManager, AgentCreator& creator);

  template<typename Vertex, typename Edge, typename EdgeContent, typename EdgeManager>
  friend void synchEdges(SharedNetwork<Vertex, Edge, EdgeContent, EdgeManager>*, EdgeManager&);

	boost::unordered_set<AgentId, HashId> fAgents;
	std::vector<boost::shared_ptr<E> > sharedEdges;
	int rank, worldSize;
	std::map<int, int> senders;
	// maps removed edges to the process to inform that the edges
	// have been deleted
	std::map<int, std::vector<std::pair<AgentId, AgentId> > > removedEdges;

protected:

	virtual bool addAgent(boost::shared_ptr<V> agent);
	virtual void removeAgent(V* agent);

	virtual void doAddEdge(boost::shared_ptr<E> edge);

public:

	using Graph<V, E, Ec, EcM>::addEdge;
	using Graph<V, E, Ec, EcM>::removeEdge;

	/**
	 * Creates a SharedNetwork with the specified name and
	 * whether or not the network is directed.
	 *
	 * @param the network name
	 * @param directed if true the network will be directed, otherwise not.
	 */
	SharedNetwork(std::string name, bool directed, EcM* edgeContentMgr);
	virtual ~SharedNetwork() {
	}

	/**
	 * NON USER API.
	 *
	 * Increments the count of edges that are sent from rank to this
	 * network.
	 */
	void addSender(int rank);

	/**
	 * NON USER API
	 * Decrements the count of edges that are sent from rank to this
	 * network.
	 */
	void removeSender(int rank);

	// doc inherited from Graphs
	void removeEdge(V* source, V* target);

	/**
	 * Add an edge to this SharedNetwork.
	 *
	 * @param edge the edge to add
	 */
	void addEdge(boost::shared_ptr<E> edge);

	/**
	 * Synchronizes any removed edges that are have been copied
	 * across processes.
	 */
	void synchRemovedEdges();

	/**
	 * Returns true if this is a master link; will be a master link if
	 * its master node is local. The master node is usually the edge 'source',
	 * but if the usesTargetAsMaster flag is set to true then the 'target'
	 * is the master node.
	 */
  virtual bool isMaster(E* e){
    return (e->usesTargetAsMaster() ? e->target()->getId().currentRank() : e->source()->getId().currentRank()) == rank;
  }
};

/* Shared Network Definition */

template<typename V, typename E, typename Ec, typename EcM>
SharedNetwork<V, E, Ec, EcM>::SharedNetwork(std::string name, bool directed, EcM* edgeContentMgr) :
  Graph<V, E, Ec, EcM> (name, directed, edgeContentMgr) {
	rank = RepastProcess::instance()->rank();
	worldSize = RepastProcess::instance()->worldSize();
}

template<typename V, typename E, typename Ec, typename EcM>
void SharedNetwork<V, E, Ec, EcM>::addSender(int rank) {
	std::map<int, int>::iterator iter = senders.find(rank);
	if (iter == senders.end()) {
		senders[rank] = 1;
	} else {
		senders[rank] = senders[rank] + 1;
	}
}

template<typename V, typename E, typename Ec, typename EcM>
void SharedNetwork<V, E, Ec, EcM>::removeSender(int rank) {
	std::map<int, int>::iterator iter = senders.find(rank);
	if (iter == senders.end()) throw Repast_Error_30(rank); // Cannot remove non-existent sender

	int val = iter->second;
	if (val == 1)
		senders.erase(rank);
	else
		senders[rank] = val - 1;
}

template<typename V, typename E, typename Ec, typename EcM>
bool SharedNetwork<V, E, Ec, EcM>::addAgent(boost::shared_ptr<V> agent) {
	AgentId id = agent->getId();
	if (id.currentRank() != rank) {
		// add to foreign ids
		fAgents.insert(id);
	}
	return Graph<V, E, Ec, EcM>::addAgent(agent);
}

template<typename V, typename E, typename Ec, typename EcM>
void SharedNetwork<V, E, Ec, EcM>::removeEdge(V* source, V* target) {
  boost::shared_ptr<E> edge = Graph<V, E, Ec, EcM>::findEdge(source, target);
  Graph<V, E, Ec, EcM>::removeEdge(source, target);

}

template<typename V, typename E, typename Ec, typename EcM>
void SharedNetwork<V, E, Ec, EcM>::removeAgent(V* agent) {
	AgentId id = agent->getId();
	if (id.currentRank() != rank) {
		fAgents.erase(id);
	}
	Graph<V, E, Ec, EcM>::removeAgent(agent);
}
template<typename V, typename E, typename Ec, typename EcM>
void SharedNetwork<V, E, Ec, EcM>::addEdge(boost::shared_ptr<E> edge) {
  SharedNetwork<V, E, Ec, EcM>::doAddEdge(edge);
}

template<typename V, typename E, typename Ec, typename EcM>
void SharedNetwork<V, E, Ec, EcM>::doAddEdge(boost::shared_ptr<E> edge) {
  Graph<V, E, Ec, EcM>::doAddEdge(edge);
}

}

#endif /* SHAREDNETWORK_H_ */
