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
 * NON USER API
 */
class ExportRequest {

private:
	int exportTo_;
	AgentId agent_;

	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive& ar, const unsigned int version) {
		ar & exportTo_;
		ar & agent_;
	}

public:
	// empty constructor for serialization
	ExportRequest() {
	}
	ExportRequest(int exportTo, AgentId id);

	AgentId agent() const {
		return agent_;
	}

	int exportTo() {
		return exportTo_;
	}

};

/**
 * NON USER API.
 *
 * Receipt for edges
 */
template<typename E>
class ItemReceipt {
public:
	ItemReceipt() :
		source_(-1) {
	}
	ItemReceipt(int source) :
		source_(source) {
	}

	std::vector<E> items;
	int source_;
};

/*
 * Forward declaration for EdgeExporter.
 */
template<typename V, typename E, typename Ec, typename EcM>
class SharedNetwork;

/**
 *
 * NON USER API.
 *
 * Handles exporting edges created locally between
 * one or more non-local agents. This also coordinates
 * notification of which processes should be exporting
 * to which in the case of edges where a node is
 * foreign to the sending and receiving process.
 *
 * All this is done internally in the SharedNetwork.
 */
template<typename E>
class EdgeExporter {
	typedef typename std::map<int, std::vector<ExportRequest> >::iterator ExportRequestMapIter;

private:
  //  template<typename Vertex, typename Edge, typename EdgeContent, typename AgentContent, typename EdgeManager,
  //      typename AgentAdder>
  //  friend void createComplementaryEdges(SharedNetwork<Vertex, Edge, EdgeContent, EdgeManager>* net, Context<Vertex>& context,
  //      EdgeManager& provider, AgentAdder& adder);

	template<typename Vertex, typename Edge, typename AgentContent, typename EdgeContent, typename EdgeManager, typename AgentCreator>
  friend void createComplementaryEdges(SharedNetwork<Vertex, Edge, EdgeContent, EdgeManager>* net, SharedContext<Vertex>& context,
      EdgeManager& edgeManager, AgentCreator& creator);
	// key is the process that we need to export the edges to
	std::map<int, std::vector<boost::shared_ptr<E> >*> edgesToExport;
	// map that holds edges that this P has exported
	// key is the process that we need to export the edges to
	std::map<int, std::vector<boost::shared_ptr<E> >*> exportedEdges;

	std::map<int, std::vector<boost::shared_ptr<E> >*> removedEdges;

	std::map<int, std::vector<ExportRequest> > exportRequests;
	int rank;

public:
	typedef typename std::map<int, std::vector<boost::shared_ptr<E> >*>::iterator EdgeMapIterator;

	EdgeExporter();
	~EdgeExporter();

	void addAgentExportRequest(int exportTo, const AgentId& id);

	/**
	 * Whether or not this is exported the specified edge.
	 */
	void edgeRemoved(boost::shared_ptr<E> edge, std::map<int, std::vector<std::pair<AgentId, AgentId> > >& removeMap);

	/**
	 * Tests if the edge needs to be exported and if so
	 * adds it to the collection of edges to be exported.
	 */
	void addEdge(boost::shared_ptr<E> edge);

	/**
	 * Gathers the receivers into out. A receiver is
	 * a process this EdgeExporter should send an edge to.
	 */
	void gatherReceivers(std::vector<int>& out);

	/**
	 * Gathers the procs that this will send export requests to
	 * into out.
	 */
	void gatherExporters(std::vector<int>& out);

	/**
	 * Send the export requests. This does an isend and the resulting requests are placed in
	 * the specified vector.
	 */
	void sendExportRequests(boost::mpi::communicator& comm, std::vector<boost::mpi::request>& requests);

	/**
	 * Gets the edges to export.
	 */
	std::map<int, std::vector<boost::shared_ptr<E> >*>& getEdgesToExport() {
		return edgesToExport;
	}

	/**
	 * Gets the edges this process is exporting.
	 */
	std::map<int, std::vector<boost::shared_ptr<E> >*>& getExportedEdges() {
		return exportedEdges;
	}

	/**
	 * Cleans up after exported edges have been sent and received.
	 */
	void cleanUp();
};

template<typename E>
EdgeExporter<E>::EdgeExporter() {
	rank = RepastProcess::instance()->rank();
}

template<typename E>
void EdgeExporter<E>::edgeRemoved(boost::shared_ptr<E> edge, std::map<int, std::vector<std::pair<AgentId, AgentId> > >& removeMap) {
	for (EdgeMapIterator emIter = exportedEdges.begin(); emIter != exportedEdges.end(); ++emIter) {
		std::vector<boost::shared_ptr<E> >* edges = emIter->second;
		for (typename std::vector<boost::shared_ptr<E> >::iterator edgeIter = edges->begin(); edgeIter != edges->end();) {
			if (*edgeIter == edge) {
				std::map<int, std::vector<std::pair<AgentId, AgentId> > >::iterator iter =
						removeMap.find(emIter->first);
				if (iter == removeMap.end()) {
					std::vector<std::pair<AgentId, AgentId> > vec;
					vec.push_back(std::make_pair<AgentId, AgentId>(edge->source()->getId(), edge->target()->getId()));
					removeMap[emIter->first] = vec;
				} else {
					iter->second.push_back(std::make_pair<AgentId, AgentId>(edge->source()->getId(),
							edge->target()->getId()));
				}
				edgeIter = edges->erase(edgeIter);
			} else {
				++edgeIter;
			}
		}
	}
}

template<typename E>
void EdgeExporter<E>::cleanUp() {
	// just delete the edge lists themselves. We don't delete the edges as they are
	// part of the network itself.
	for (EdgeMapIterator emIter = edgesToExport.begin(); emIter != edgesToExport.end(); ++emIter) {
		std::vector<boost::shared_ptr<E> >* edges = emIter->second;
		int exportTo = emIter->first;
		if (exportedEdges.find(exportTo) == exportedEdges.end()) {
			std::vector<boost::shared_ptr<E> >* exportedVec = new std::vector<boost::shared_ptr<E> >();
			exportedEdges[exportTo] = exportedVec;
			exportedVec->insert(exportedVec->end(), edges->begin(), edges->end());

		} else {
			std::vector<boost::shared_ptr<E> >* exportedVec = exportedEdges[exportTo];
			exportedVec->insert(exportedVec->end(), edges->begin(), edges->end());

		}
		delete edges;
	}
	edgesToExport.clear();
}

template<typename E>
void EdgeExporter<E>::gatherReceivers(std::vector<int>& out) {
	for (EdgeMapIterator iter = edgesToExport.begin(); iter != edgesToExport.end(); ++iter) {
		out.push_back(iter->first);
	}
}

template<typename E>
void EdgeExporter<E>::gatherExporters(std::vector<int>& out) {
	for (ExportRequestMapIter iter = exportRequests.begin(); iter != exportRequests.end(); ++iter) {
		out.push_back(iter->first);
	}
}

template<typename E>
EdgeExporter<E>::~EdgeExporter() {
	for (EdgeMapIterator iter = edgesToExport.begin(); iter != edgesToExport.end(); ++iter) {
		delete iter->second;
	}

	for (EdgeMapIterator iter = exportedEdges.begin(); iter != exportedEdges.end(); ++iter) {
		delete iter->second;
	}
}

template<typename E>
void EdgeExporter<E>::addEdge(boost::shared_ptr<E> edge) {
	AgentId& sid = edge->source()->getId();
	AgentId& tid = edge->target()->getId();

	if (sid.currentRank() != rank) {
		EdgeMapIterator iter = edgesToExport.find(sid.currentRank());
		if (iter == edgesToExport.end()) {
			std::vector<boost::shared_ptr<E> >* vec = new std::vector<boost::shared_ptr<E> >();
			vec->push_back(edge);
			edgesToExport[sid.currentRank()] = vec;
		} else {
			iter->second->push_back(edge);
		}
	}

	if (tid.currentRank() != rank) {
		EdgeMapIterator iter = edgesToExport.find(tid.currentRank());
		if (iter == edgesToExport.end()) {
			std::vector<boost::shared_ptr<E> >* vec = new std::vector<boost::shared_ptr<E> >();
			vec->push_back(edge);
			edgesToExport[tid.currentRank()] = vec;
		} else {
			iter->second->push_back(edge);
		}
	}
}

template<typename E>
void EdgeExporter<E>::addAgentExportRequest(int exportTo, const AgentId& id) {
	int exportingP = id.currentRank();
	ExportRequestMapIter iter = exportRequests.find(exportingP);
	if (iter == exportRequests.end()) {
		std::vector<ExportRequest> requests;
		requests.push_back(ExportRequest(exportTo, id));
		exportRequests[exportingP] = requests;

	} else {
		std::vector<ExportRequest>& requests = iter->second;
		requests.push_back(ExportRequest(exportTo, id));
	}
}

template<typename E>
void EdgeExporter<E>::sendExportRequests(boost::mpi::communicator& comm, std::vector<boost::mpi::request>& requests) {
	for (ExportRequestMapIter iter = exportRequests.begin(); iter != exportRequests.end(); ++iter) {
		int receiver = iter->first;
		std::vector<ExportRequest>& expRequests = iter->second;
		requests.push_back(comm.isend(receiver, NET_EXPORT_REQUESTS, expRequests));
	}
}

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

//  template<typename Vertex, typename Edge, typename EdgeContent, typename AgentContent, typename EdgeManager,
//      typename AgentAdder>
//  friend void createComplementaryEdges(SharedNetwork<Vertex, Edge, EdgeContent, EdgeManager>* net, SharedContext<Vertex>& context,
//      EdgeManager& edgeManager, AgentAdder&);

  template<typename Vertex, typename Edge, typename AgentContent, typename EdgeContent, typename EdgeManager, typename AgentCreator>
  friend void createComplementaryEdges(SharedNetwork<Vertex, Edge, EdgeContent, EdgeManager>* net, SharedContext<Vertex>& context,
      EdgeManager& edgeManager, AgentCreator& creator);

  template<typename Vertex, typename Edge, typename EdgeContent, typename EdgeManager>
  friend void synchEdges(SharedNetwork<Vertex, Edge, EdgeContent, EdgeManager>*, EdgeManager&);

	boost::unordered_set<AgentId, HashId> fAgents;
	std::vector<boost::shared_ptr<E> > sharedEdges;
	int rank, worldSize;
	EdgeExporter<E> edgeExporter;
	std::map<int, int> senders;
	// maps removed edges to the process to inform that the edges
	// have been deleted
	std::map<int, std::vector<std::pair<AgentId, AgentId> > > removedEdges;

	//
	void notifyExporters();
	// by passes the export check for adding an edge
	void graphAddEdge(boost::shared_ptr<E> edge);

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
};

/**
 * NON USER API
 */
template<typename E, typename EdgeContent, typename ProviderReceiver>
void sendContent(EdgeExporter<E>& exporter, std::vector<boost::mpi::request>& requests, ProviderReceiver& provider, boost::ptr_vector<std::vector<EdgeContent> > &sendBuffer) {
  std::map<int, std::vector<boost::shared_ptr<E> >*>& edgesToExport = exporter.getEdgesToExport();
  boost::mpi::communicator* comm = RepastProcess::instance()->getCommunicator();

  std::vector<EdgeContent>* edgeContent;

  for (typename EdgeExporter<E>::EdgeMapIterator emIter = edgesToExport.begin(); emIter != edgesToExport.end(); ++emIter) {
    // the process to send the edge to
    int receiver = emIter->first;
    std::vector<boost::shared_ptr<E> >* edges = emIter->second;
    sendBuffer.push_back(edgeContent = new std::vector<EdgeContent>);
    // check end points and add any local nodes to the AgentExporter
    for (typename std::vector<boost::shared_ptr<E> >::iterator iter = edges->begin(); iter != edges->end(); ++iter) {

      boost::shared_ptr<E> edge = *iter;
      AgentId srcId = edge->source()->getId();
      AgentId targetId = edge->target()->getId();
      if (srcId.currentRank() == comm->rank()) {
        RepastProcess::instance()->addExportedAgent(receiver, srcId);
      } else if (srcId.currentRank() != receiver) {
        // sending node that's foreign to both this P and the receiver
        // so need to tell the P where the node resides to export it to
        // the receiver.
        exporter.addAgentExportRequest(receiver, srcId);
      }

      if (targetId.currentRank() == comm->rank()) {
        RepastProcess::instance()->addExportedAgent(receiver, targetId);
      } else if (targetId.currentRank() != receiver) {
        // sending node that's foreign to both this P and the receiver
        // so need to tell the P where the node resides to export it to
        // the receiver.
        exporter.addAgentExportRequest(receiver, targetId);
      }

      provider.provideEdgeContent(edge, *edgeContent);
    }
    requests.push_back(comm->isend(receiver, NET_EDGE_UPDATE, *edgeContent)); //*edges));
  }
}

/**
 * Notifies other processes of any edges that have been created between
 * nodes on this process and imported nodes. The other process will then
 * create the complimentary edge. For example, if P1 creates an edge
 * between A and B where B resides on P2, then this method will notify P2
 * to create the incoming edge A->B on its copy of B. Any unknown agents
 * will be added to the context. For example, if P2 didn't have a reference
 * to A, then A will be added to P2's context.
 *
 * @param net the network in which to create the complementary edges or from which to send
 * complementary edges
 * @param context the context that contains the agents in the process
 * @param edgeManager creates edges from EdgeContent and creates EdgeContent from an edge and a context.
 * @param creator creates agents from AgentContent.
 *
 * @tparam Vertex the vertex (agent) type
 * @tparam Edge the edge type
 * @tparam AgentContent the serializable struct or class that describes the agent state. It must contain a getId()
 * method that returns the AgentId of the agent it describes.
 * @tparam EdgeContent the serializable struct or class that describes edge state. At the very least
 * EdgeContent must contain two public fields sourceContent and targetContent of type AgentContent. These represent
 * the source and target of the edge.
 * @tparam EdgeManager create edges from EdgeContent and provides EdgeContent given a context and an edge of type Edge. It must
 * implement void provideEdgeContent(constEdge* edge, std::vector<EdgeContent>& edgeContent) and
 * Edge* createEdge(repast::Context<Vertex>& context, EdgeContent& edge);
 * @tparam AgentCreator creates agents from AgentContent, implementing the following method
 * Vertex* createAgent(constAgentContent& content);
 */
template<typename Vertex, typename Edge, typename AgentContent, typename EdgeContent, typename EdgeManager,
    typename AgentCreator>
void createComplementaryEdges(SharedNetwork<Vertex, Edge>* net, SharedContext<Vertex>& context,
    EdgeManager& edgeManager, AgentCreator& creator) {
  boost::mpi::communicator* world = RepastProcess::instance()->getCommunicator();

  // created edges with foreign node, so push that edge to the foreign proc
  // 1. gather the list of receivers procs that each proc wants to send to
  // 2. Send that list to 0
  // 3. If 0, sort the received lists such that a list of which process to expect
  // edges from can be sent to each proc

  std::vector<int> listToSend;
  std::vector<int> list;

  // gather a list of ints that the edgeExporter
  // should send to.
  net->edgeExporter.gatherReceivers(listToSend);

  // tells each process who to expect edges from
  SRManager exchanger(world);
  exchanger.retrieveSources(listToSend, list);
  listToSend.clear();


  std::vector<boost::mpi::request> requests;


  // sends edge content
  boost::ptr_vector<std::vector<EdgeContent> >* sendBuffer = new boost::ptr_vector<std::vector<EdgeContent> >;
  sendContent<Edge, EdgeContent, EdgeManager> (net->edgeExporter, requests, edgeManager, *sendBuffer);

  std::vector<ItemReceipt<EdgeContent>*> receipts;
  for (size_t i = 0; i < list.size(); i++) {
    int sender = list[i];
    ItemReceipt<EdgeContent>* receipt = new ItemReceipt<EdgeContent> (sender);
    requests.push_back(world->irecv(sender, NET_EDGE_UPDATE, receipt->items));
    receipts.push_back(receipt);
  }
  boost::mpi::wait_all(requests.begin(), requests.end());

  // requests are now fulfilled, so we can clean up the edgeExporter
  net->edgeExporter.cleanUp();
  delete sendBuffer;

  // process the received edges
  for (size_t i = 0; i < receipts.size(); i++) {
    ItemReceipt<EdgeContent>* receipt = receipts[i];
    net->addSender(receipt->source_);

    const size_t countOfItemsReceived = receipt->items.size();
    for (size_t j = 0; j < countOfItemsReceived; j++) {
      EdgeContent edge = receipt->items[j];
      // Process edge:
      // 1. If source or target exist in the network, then replace the
      //       appropriate end points of the edge and delete the existing end point.
      // 2. End point agents that don't exist in context need to be added to the context
      // 3. Add the edge to the network.
      AgentContent source = edge.sourceContent;
      AgentContent target = edge.targetContent;

      AgentId sourceId = source.getId();
      AgentId targetId = target.getId();
      if (!context.contains(sourceId)) {
        // source was added to context, so this P didn't know about it
        // so we need to add it to the agents to import
        Vertex* out = creator.createAgent(source);
        context.addAgent(out);
        context.incrementProjRefCount(out->getId());
        RepastProcess::instance()->addImportedAgent(sourceId);
      }

      if (!context.contains(targetId)) {
        // target was added to context, so this P didn't know about it
        // so we need to add it to the agents to import
        Vertex* out = creator.createAgent(target);
        context.addAgent(out);
        context.incrementProjRefCount(out->getId());
        RepastProcess::instance()->addImportedAgent(targetId);
      }

      boost::shared_ptr<Edge> newEdge(edgeManager.createEdge(context, edge));
      net->graphAddEdge(newEdge);
    }

    delete receipt;
    receipt = 0;
  }
  net->notifyExporters();
}


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
  edgeExporter.edgeRemoved(edge, removedEdges);
  Graph<V, E, Ec, EcM>::removeEdge(source, target);

}

template<typename V, typename E, typename Ec, typename EcM>
void SharedNetwork<V, E, Ec, EcM>::removeAgent(V* agent) {
	AgentId id = agent->getId();
	if (id.currentRank() != rank) {
		fAgents.erase(id);
	}

	typename Graph<V,E>::VertexMapIterator iter = Graph<V,E>::vertices.find(id);
	if (iter != Graph<V,E>::vertices.end()) {
		Vertex<V, E>* vertex = iter->second;
		std::vector<boost::shared_ptr<E> > edges;
		vertex->edges(Vertex<V,E>::OUTGOING, edges);

		for (size_t i = 0, n = edges.size(); i < n; ++i) {
			edgeExporter.edgeRemoved(edges[i], removedEdges);
		}

		if (Graph<V,E, Ec, EcM>::isDirected) {
			edges.clear();
			vertex->edges(Vertex<V, E>::INCOMING, edges);
			for (size_t i = 0, n = edges.size(); i < n; ++i) {
				edgeExporter.edgeRemoved(edges[i], removedEdges);
			}
		}
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
	edgeExporter.addEdge(edge);
}

template<typename V, typename E, typename Ec, typename EcM>
void SharedNetwork<V, E, Ec, EcM>::graphAddEdge(boost::shared_ptr<E> edge) {
  Graph<V, E, Ec, EcM>::doAddEdge(edge);
}

/**
 * Called after an edge update to notify process X that it should
 * now export agents to process Y because agents were shared as a result
 * of sending edges.
 */
template<typename V, typename E, typename Ec, typename EcM>
void SharedNetwork<V, E, Ec, EcM>::notifyExporters() {
  RepastProcess* rp = RepastProcess::instance();
  boost::mpi::communicator* world = rp->getCommunicator();

	std::vector<int> listToSend;
	std::vector<int> list;

  // get list of ints that the edgeExporter needs to send export requests to
  edgeExporter.gatherExporters(listToSend);

	SRManager exchange(world);
	exchange.retrieveSources(listToSend, list);


	std::vector<boost::mpi::request> requests;
	std::vector<ItemReceipt<ExportRequest>*> receipts;
	for (size_t i = 0; i < list.size(); i++) {
		int sender = list[i];
		ItemReceipt<ExportRequest>* receipt = new ItemReceipt<ExportRequest> ();
		requests.push_back(world->irecv(sender, NET_EXPORT_REQUESTS, receipt->items));
		receipts.push_back(receipt);
	}

	edgeExporter.sendExportRequests(*world, requests);
	boost::mpi::wait_all(requests.begin(), requests.end());

	// process the received requests
	for (size_t i = 0; i < receipts.size(); i++) {
		ItemReceipt<ExportRequest>* receipt = receipts[i];
		for (size_t j = 0; j < receipt->items.size(); j++) {
			ExportRequest& req = receipt->items[j];
			rp->addExportedAgent(req.exportTo(), req.agent());
		}

		delete receipt;
	}
}


template<typename V, typename E>
void SharedNetwork<V, E>::synchRemovedEdges() {
	std::vector<boost::mpi::request> requests;
	// receive from senders
	std::map<int, std::vector<std::pair<AgentId, AgentId> >*> contents;

	boost::mpi::communicator* world = RepastProcess::instance()->getCommunicator();
	// receive edge content from those this P is importing from
	for (std::map<int, int>::const_iterator iter = senders.begin(); iter != senders.end(); ++iter) {
		int sender = iter->first;
		std::vector<std::pair<AgentId, AgentId> >* content = new std::vector<std::pair<AgentId, AgentId> >();
		requests.push_back(world->irecv(sender, NET_EDGE_REMOVE_SYNC, *content));
		contents[sender] = content;
	}

	std::vector<std::pair<AgentId, AgentId> > empty;
	std::map<int, std::vector<boost::shared_ptr<E> >*>& exports = edgeExporter.getExportedEdges();
	for (typename std::map<int, std::vector<boost::shared_ptr<E> >*>::iterator iter = exports.begin(); iter != exports.end();) {
		int sendTo = iter->first;
		std::map<int, std::vector<std::pair<AgentId, AgentId> > >::iterator removeIter = removedEdges.find(sendTo);
		if (removeIter == removedEdges.end()) {
			requests.push_back(world->isend(sendTo, NET_EDGE_REMOVE_SYNC, empty));
		} else {
			requests.push_back(world->isend(sendTo, NET_EDGE_REMOVE_SYNC, removeIter->second));
		}

		// delete any empty vectors from exportedEdges
		// and remove them from the map. This eliminates the
		// map key as a rank to send to
		if (iter->second->size() == 0) {
			delete iter->second;
			exports.erase(iter++);
		} else {
			++iter;
		}
	}

	boost::mpi::wait_all(requests.begin(), requests.end());

	for (std::map<int, std::vector<std::pair<AgentId, AgentId> >*>::iterator iter = contents.begin(); iter
			!= contents.end(); ++iter) {
		std::vector<std::pair<AgentId, AgentId> >* vec = iter->second;
		for (size_t j = 0, k = vec->size(); j < k; ++j) {
			std::pair<AgentId, AgentId>& ends = (*vec)[j];
			Graph<V, E>::removeEdge(ends.first, ends.second);
			removeSender(iter->first);
		}
		delete vec;
	}
	removedEdges.clear();
}

/**
 * Synchronizes any edges that have been created as complementary edges. This only
 * necessary if the edges have been deleted or their state has been changed in some way.
 *
 * @param net the network in which to create the complementary edges or from which to send
 * complementary edges
 * @param edgeManager updates edges from EdgeContent and creates EdgeContent from an edge and a context.
 *
 * @tparam Vertex the vertex (agent) type
 * @tparam Edge the edge type
 * @tparam EdgeContent the serializable struct or class that describes edge state.
 * @tparam EdgeManager updates edges from EdgeContent and provides EdgeContent given a context and an edge of type Edge. It must
 * implement void provideEdgeContent(const Edge* edge, std::vector<EdgeContent>& edgeContent) and
 * void receiveEdgeContent(const EdgeContent& content);
 */
template<typename Vertex, typename Edge, typename EdgeContent, typename EdgeManager>
void synchEdges(SharedNetwork<Vertex, Edge>* net, EdgeManager& edgeManager) {

	std::map<int, std::vector<boost::shared_ptr<Edge> >* >& exports = net->edgeExporter.getExportedEdges();
	boost::mpi::communicator* world = RepastProcess::instance()->getCommunicator();

	std::vector<boost::mpi::request> requests;
	std::vector<std::vector<EdgeContent>*> contents;

	// receive edge content from those this P is importing from
	for (std::map<int, int>::const_iterator iter = net->senders.begin(); iter != net->senders.end(); ++iter) {
		int sender = iter->first;
		std::vector<EdgeContent>* content = new std::vector<EdgeContent>();
		requests.push_back(world->irecv(sender, NET_EDGE_SYNC, *content));
		contents.push_back(content);
	}

	boost::ptr_vector<std::vector<EdgeContent> >* contentVector = new boost::ptr_vector<std::vector<EdgeContent> >;
	std::vector<EdgeContent>* edgeContent;

	// send updated edge content to those I'm exporting to
	for (typename EdgeExporter<Edge>::EdgeMapIterator emIter = exports.begin(); emIter != exports.end(); ++emIter) {
		// the process to send the edge to
		int receiver = emIter->first;
		std::vector<boost::shared_ptr<Edge> >* edges = emIter->second;
		contentVector->push_back( edgeContent = new std::vector<EdgeContent>);
		for (typename std::vector<boost::shared_ptr<Edge> >::iterator iter = edges->begin(); iter != edges->end(); ++iter) {
			edgeManager.provideEdgeContent(*iter, *edgeContent);
		}
		requests.push_back(world->isend(receiver, NET_EDGE_SYNC, *edgeContent));
	}

	boost::mpi::wait_all(requests.begin(), requests.end());
	delete contentVector;

	for (int i = 0, n = contents.size(); i < n; i++) {
		std::vector<EdgeContent>* content = contents[i];
		for (typename std::vector<EdgeContent>::const_iterator iter = content->begin(); iter != content->end(); ++iter) {
			edgeManager.receiveEdgeContent(*iter);
		}
		delete content;
	}
}

}

#endif /* SHAREDNETWORK_H_ */
