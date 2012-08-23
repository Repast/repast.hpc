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
 *  SharedDiscreteSpace.h
 *
 *  Created on: Jul 20, 2010
 *      Author: nick
 */

#ifndef SHAREDDISCRETESPACE_H_
#define SHAREDDISCRETESPACE_H_

#include <boost/mpi/communicator.hpp>

#include "SharedBaseGrid.h"

namespace repast {

/**
 * Discrete matrix-like SharedBaseGrid implementation. This
 * primarily adds the buffer synchronization appropriate for this
 * type. Default templated typical SharedGrid types are defined in SharedGrids.
 *
 * @see SharedBaseGrid for more details.
 *
 * @tparam T the type of objects contained by this BaseGrid
 * @tparam GPTransformer transforms cell points according to the topology (e.g. periodic)
 * of the BaseGrid.
 * @tparam Adder determines how objects are added to the grid from its associated context.
 */
template<typename T, typename GPTransformer, typename Adder>
class SharedDiscreteSpace: public SharedBaseGrid<T, GPTransformer, Adder, int> {

protected:
	virtual void synchMoveTo(const AgentId& id, const Point<int>& pt);

private:
	template<typename AgentContent, typename ContentProvider>
	void sendToNeighbor(GridBufferSyncher<AgentContent, int>& syncher, Neighbors::Location location,
			ContentProvider& provider, int val);

	typedef SharedBaseGrid<T, GPTransformer, Adder, int> SharedBaseGridType;

public:
	virtual ~SharedDiscreteSpace();
	SharedDiscreteSpace(std::string name, GridDimensions gridDims, std::vector<int> processDims, int buffer, boost::mpi::communicator* world);

	/**
	 * Synchronize the buffer area of this SharedDiscreteSpace with its neighbors. This
	 * will copy the buffer area from the neighbors into this SharedDiscreteSpace.
	 * This should be called immediately after initSynchBuffer.
	 *
	 * @param context the SharedContext that contains the agents in this SharedDiscreteSpace.
	 * @param provider a class that provides AgentContent for the agents being buffered
	 * in neighboring grids.
	 * @param creator a class that creates a agents of type T when given
	 * AgentContent.
	 *
	 * @tparam T the type of agent in this SharedDiscreteSpace
	 * @tparam AgentContent the serializable struct or class that describes the
	 * state of agents.
	 * @tparam Provider a class that provides AgentContent for aagents,
	 * implementing void provideContent(T* agent, std::vector<AgentContent>& out)
	 * @tparam AgentsCreator a class that creates agents given AgentContent, implementing
	 * void createAgents(std::vector<AgentContent>& contents, std::vector<T*>& out). Creating
	 * agents from the vector of content and placing them in out.
	 */
	template<typename AgentContent, typename Provider, typename AgentsCreator>
	void synchBuffer(SharedContext<T>& context, Provider& provider, AgentsCreator& creator);


  virtual void getAgentsToPush(std::set<AgentId>& agentsToTest, std::map<int, std::set<AgentId> >& agentsToPush);

};

template<typename T, typename GPTransformer, typename Adder>
SharedDiscreteSpace<T, GPTransformer, Adder>::SharedDiscreteSpace(std::string name, GridDimensions gridDims,
		std::vector<int> processDims, int buffer, boost::mpi::communicator* world) :
	SharedBaseGrid<T, GPTransformer, Adder, int> (name, gridDims, processDims, buffer, world) {
}

template<typename T, typename GPTransformer, typename Adder>
void SharedDiscreteSpace<T, GPTransformer, Adder>::synchMoveTo(const AgentId& id, const Point<int>& pt) {
	//unlikely chance that agent could have
	// moved and then "died" and so removed from sending context, in which case
	// it would never get sent to this grid.
	if (SharedBaseGridType::GridBaseType::contains(id)) {
		SharedBaseGridType::GridBaseType::moveTo(id, pt.coords());
	}
}

template<typename T, typename GPTransformer, typename Adder>
template<typename AgentContent, typename ContentProvider>
void SharedDiscreteSpace<T, GPTransformer, Adder>::sendToNeighbor(GridBufferSyncher<AgentContent, int>& syncher,
		Neighbors::Location location, ContentProvider& provider, int val) {

	Neighbor* ngh = SharedBaseGridType::nghs.neighbor(location);
	if (ngh != 0) {
		std::vector<CellContents<AgentContent, int> > toSend;
		GridDimensions bounds = SharedBaseGridType::createSendBufferBounds(location);

		int xEnd = bounds.origin().getX() + bounds.extents().getX();
		for (int x = bounds.origin().getX(); x < xEnd; x++) {
			int yEnd = bounds.origin().getY() + bounds.extents().getY();
			for (int y = bounds.origin().getY(); y < yEnd; y++) {
				Point<int> pt(x, y);
				CellContents<AgentContent, int> contents(pt);
				std::vector<T*> out;
				SharedBaseGridType::GridBaseType::getObjectsAt(pt, out);
				for (size_t i = 0, n = out.size(); i < n; ++i) {
					provider.provideContent(out[i], contents._objs);
					toSend.push_back(contents);
				}
			}
		}
		syncher.send(ngh->rank(), toSend, val);
	}
}

template<typename T, typename GPTransformer, typename Adder>
template<typename AgentContent, typename ContentProvider, typename ContentReceiver>
void SharedDiscreteSpace<T, GPTransformer, Adder>::synchBuffer(SharedContext<T>& context, ContentProvider& provider,
		ContentReceiver& receiver) {

	GridBufferSyncher<AgentContent, int> syncher(SharedBaseGridType::comm);

	syncher.receive(SharedBaseGridType::nghs.neighbor(Neighbors::E), GRID_BUFFER_SYNC0);
	sendToNeighbor<AgentContent> (syncher, Neighbors::W, provider, GRID_BUFFER_SYNC0);

	syncher.receive(SharedBaseGridType::nghs.neighbor(Neighbors::W), GRID_BUFFER_SYNC1);
	sendToNeighbor<AgentContent> (syncher, Neighbors::E, provider, GRID_BUFFER_SYNC1);

	syncher.receive(SharedBaseGridType::nghs.neighbor(Neighbors::N), GRID_BUFFER_SYNC2);
	sendToNeighbor<AgentContent> (syncher, Neighbors::S, provider, GRID_BUFFER_SYNC2);

	syncher.receive(SharedBaseGridType::nghs.neighbor(Neighbors::S), GRID_BUFFER_SYNC3);
	sendToNeighbor<AgentContent> (syncher, Neighbors::N, provider, GRID_BUFFER_SYNC3);

	syncher.receive(SharedBaseGridType::nghs.neighbor(Neighbors::NE), GRID_BUFFER_SYNC4);
	sendToNeighbor<AgentContent> (syncher, Neighbors::SW, provider, GRID_BUFFER_SYNC4);

	syncher.receive(SharedBaseGridType::nghs.neighbor(Neighbors::NW), GRID_BUFFER_SYNC5);
	sendToNeighbor<AgentContent> (syncher, Neighbors::SE, provider, GRID_BUFFER_SYNC5);

	syncher.receive(SharedBaseGridType::nghs.neighbor(Neighbors::SE), GRID_BUFFER_SYNC6);
	sendToNeighbor<AgentContent> (syncher, Neighbors::NW, provider, GRID_BUFFER_SYNC6);

	syncher.receive(SharedBaseGridType::nghs.neighbor(Neighbors::SW), GRID_BUFFER_SYNC7);
	sendToNeighbor<AgentContent> (syncher, Neighbors::NE, provider, GRID_BUFFER_SYNC7);

	syncher.wait();

	for (size_t i = 0; i < syncher.vecsSize(); ++i) {
		std::vector<CellContents<AgentContent, int> >* contentsList = syncher.received(i);
		int rank = syncher.nghRank(i);
		for (size_t j = 0; j < contentsList->size(); ++j) {
			CellContents<AgentContent, int> contents = (*contentsList)[j];
			std::vector<T*> out;
			receiver.createAgents(contents._objs, out);
			for (typename std::vector<T*>::iterator iter = out.begin(); iter != out.end(); ++iter) {
				T* obj = *iter;
				obj->getId().currentRank(rank);
				AgentId id = obj->getId();
				SharedBaseGridType::buffered.push_back(id);
				if (context.addAgent(obj) != obj) {
					// already exists in the context so delete this one.
					// this will occur if an obj is in the buffer of > 1
					// spaces in the same context
					delete obj;
				}
				context.incrementProjRefCount(id);
				synchMoveTo(id, contents._pt);
			}
		}
	}
}

template<typename T, typename GPTransformer, typename Adder>
SharedDiscreteSpace<T, GPTransformer, Adder>::~SharedDiscreteSpace() {
}




template<typename T, typename GPTransformer, typename Adder>
void SharedDiscreteSpace<T, GPTransformer, Adder>::getAgentsToPush(std::set<AgentId>& agentsToTest, std::map<int, std::set<AgentId> >& agentsToPush){

  // The most efficient algorithm will vary, and
  // can be impacted by the number of agents vs. the
  // number of grid cells and the distribution of agents
  // within the space.


  // This implementation will assume that agents are distributed
  // evenly throughout the space and that the space is large
  // relative to the number of agents in it. and the buffer
  // zones comparatively small.

  // Consequently, the strategy will be to look through the
  // cells in the buffer zones, collecting all the
  // agents therein.

  // An increase in efficiency can be gained
  // by considering the overlapping areas of the buffer zones only
  // once (instead of three times)


  int r = SharedBaseGrid<T, GPTransformer, Adder, int>::comm->rank();
  if(r == 0) std::cout << " RUNNING CHILD: AgentsToTest initial size: " << agentsToTest.size() << std::endl;

  // In a general case, we might not want to do this, but
  // for the current configuration, we can make this (perhaps much) more efficient
  // this way:
  Point<double> localOrigin = SharedBaseGrid<T, GPTransformer, Adder, int>::localBounds.origin();
  Point<double> localExtent = SharedBaseGrid<T, GPTransformer, Adder, int>::localBounds.extents();

  int buffer = SharedBaseGrid<T, GPTransformer, Adder, int>::_buffer;

  int X1 = localOrigin.getX();
  int X2 = localOrigin.getX() + buffer;
  int X3 = localOrigin.getX() + localExtent.getX() - buffer;
  int X4 = localOrigin.getX() + localExtent.getX();

  int Y1 = localOrigin.getY();
  int Y2 = localOrigin.getY() + buffer;
  int Y3 = localOrigin.getY() + localExtent.getY() - buffer;
  int Y4 = localOrigin.getY() + localExtent.getY();

  int NW_rank = SharedBaseGrid<T, GPTransformer, Adder, int>::nghs.neighbor(Neighbors::NW)->rank(); std::set<AgentId> NW_set;
  int N_rank  = SharedBaseGrid<T, GPTransformer, Adder, int>::nghs.neighbor(Neighbors::N)->rank();  std::set<AgentId> N_set;
  int NE_rank = SharedBaseGrid<T, GPTransformer, Adder, int>::nghs.neighbor(Neighbors::NE)->rank(); std::set<AgentId> NE_set;
  int E_rank  = SharedBaseGrid<T, GPTransformer, Adder, int>::nghs.neighbor(Neighbors::E)->rank();  std::set<AgentId> E_set;
  int SE_rank = SharedBaseGrid<T, GPTransformer, Adder, int>::nghs.neighbor(Neighbors::SE)->rank(); std::set<AgentId> SE_set;
  int S_rank  = SharedBaseGrid<T, GPTransformer, Adder, int>::nghs.neighbor(Neighbors::S)->rank();  std::set<AgentId> S_set;
  int SW_rank = SharedBaseGrid<T, GPTransformer, Adder, int>::nghs.neighbor(Neighbors::SW)->rank(); std::set<AgentId> SW_set;
  int W_rank  = SharedBaseGrid<T, GPTransformer, Adder, int>::nghs.neighbor(Neighbors::W)->rank();  std::set<AgentId> W_set;


  // NW
  for(int x = X1; x < X2; x++){
    for(int y = Y1; y < Y2; y++){
      Point<int> pt(x, y);
      std::vector<T*> out;
      SharedBaseGridType::GridBaseType::getObjectsAt(pt, out);
      for (size_t i = 0, n = out.size(); i < n; ++i) {
        AgentId id = out[i]->getId();
        agentsToTest.erase(id);
        NW_set.insert(id);
      }
    }
  }
  // N
  for(int x = X2; x < X3; x++){
    for(int y = Y1; y < Y2; y++){
      Point<int> pt(x, y);
      std::vector<T*> out;
      SharedBaseGridType::GridBaseType::getObjectsAt(pt, out);
      for (size_t i = 0, n = out.size(); i < n; ++i) {
        AgentId id = out[i]->getId();
        agentsToTest.erase(id);
        N_set.insert(id);
      }
    }
  }
  // NE
  for(int x = X3; x < X4; x++){
    for(int y = Y1; y < Y2; y++){
      Point<int> pt(x, y);
      std::vector<T*> out;
      SharedBaseGridType::GridBaseType::getObjectsAt(pt, out);
      for (size_t i = 0, n = out.size(); i < n; ++i) {
        AgentId id = out[i]->getId();
        agentsToTest.erase(id);
        NE_set.insert(id);
      }
    }
  }
  // E
  for(int x = X3; x < X4; x++){
    for(int y = Y2; y < Y3; y++){
      Point<int> pt(x, y);
      std::vector<T*> out;
      SharedBaseGridType::GridBaseType::getObjectsAt(pt, out);
      for (size_t i = 0, n = out.size(); i < n; ++i) {
        AgentId id = out[i]->getId();
        agentsToTest.erase(id);
        E_set.insert(id);
      }
    }
  }
  // SE
  for(int x = X3; x < X4; x++){
    for(int y = Y3; y < Y4; y++){
      Point<int> pt(x, y);
      std::vector<T*> out;
      SharedBaseGridType::GridBaseType::getObjectsAt(pt, out);
      for (size_t i = 0, n = out.size(); i < n; ++i) {
        AgentId id = out[i]->getId();
        agentsToTest.erase(id);
        SE_set.insert(id);
      }
    }
  }
  // S
  for(int x = X2; x < X3; x++){
    for(int y = Y3; y < Y4; y++){
      Point<int> pt(x, y);
      std::vector<T*> out;
      SharedBaseGridType::GridBaseType::getObjectsAt(pt, out);
      for (size_t i = 0, n = out.size(); i < n; ++i) {
        AgentId id = out[i]->getId();
       agentsToTest.erase(id);
        S_set.insert(id);
      }
    }
  }
  // SW
  for(int x = X1; x < X2; x++){
    for(int y = Y3; y < Y4; y++){
      Point<int> pt(x, y);
      std::vector<T*> out;
      SharedBaseGridType::GridBaseType::getObjectsAt(pt, out);
      for (size_t i = 0, n = out.size(); i < n; ++i) {
        AgentId id = out[i]->getId();
        agentsToTest.erase(id);
        SW_set.insert(id);
      }
    }
  }
  // W
  for(int x = X1; x < X2; x++){
    for(int y = Y2; y < Y3; y++){
      Point<int> pt(x, y);
      std::vector<T*> out;
      SharedBaseGridType::GridBaseType::getObjectsAt(pt, out);
      for (size_t i = 0, n = out.size(); i < n; ++i) {
        AgentId id = out[i]->getId();
        agentsToTest.erase(id);
        W_set.insert(id);
      }
    }
  }

  if(r == 0) std::cout << " RUNNING CHILD: AgentsToTest   final size: " << agentsToTest.size() << std::endl;

  if(NW_set.size() > 0){
    agentsToPush[ W_rank].insert(NW_set.begin(), NW_set.end());
    agentsToPush[NW_rank].insert(NW_set.begin(), NW_set.end());
    agentsToPush[ N_rank].insert(NW_set.begin(), NW_set.end());
  }
  if( N_set.size() > 0){
    agentsToPush[ N_rank].insert( N_set.begin(),  N_set.end());
  }
  if(NE_set.size() > 0){
    agentsToPush[ N_rank].insert(NE_set.begin(), NE_set.end());
    agentsToPush[NE_rank].insert(NE_set.begin(), NE_set.end());
    agentsToPush[ E_rank].insert(NE_set.begin(), NE_set.end());
  }
  if( E_set.size() > 0){
    agentsToPush[ E_rank].insert( E_set.begin(),  E_set.end());
  }
  if(SE_set.size() > 0){
    agentsToPush[ E_rank].insert(SE_set.begin(), SE_set.end());
    agentsToPush[SE_rank].insert(SE_set.begin(), SE_set.end());
    agentsToPush[ S_rank].insert(SE_set.begin(), SE_set.end());
  }
  if( S_set.size() > 0) agentsToPush[ S_rank].insert( S_set.begin(),  S_set.end());
  if(SW_set.size() > 0){
    agentsToPush[ S_rank].insert(SW_set.begin(), SW_set.end());
    agentsToPush[SW_rank].insert(SW_set.begin(), SW_set.end());
    agentsToPush[ W_rank].insert(SW_set.begin(), SW_set.end());
  }
  if( W_set.size() > 0){
    agentsToPush[ W_rank].insert( W_set.begin(),  W_set.end());
  }

}




}

#endif /* SHAREDDISCRETESPACE_H_ */
