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

	typedef SharedBaseGrid<T, GPTransformer, Adder, int> SharedBaseGridType;

public:
	virtual ~SharedDiscreteSpace();
	SharedDiscreteSpace(std::string name, GridDimensions gridDims, std::vector<int> processDims, int buffer, boost::mpi::communicator* communicator);

  virtual void getAgentsToPush(std::set<AgentId>& agentsToTest, std::map<int, std::set<AgentId> >& agentsToPush);

};

template<typename T, typename GPTransformer, typename Adder>
SharedDiscreteSpace<T, GPTransformer, Adder>::SharedDiscreteSpace(std::string name, GridDimensions gridDims,
		std::vector<int> processDims, int buffer, boost::mpi::communicator* communicator) :
	SharedBaseGrid<T, GPTransformer, Adder, int> (name, gridDims, processDims, buffer, communicator) {
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
SharedDiscreteSpace<T, GPTransformer, Adder>::~SharedDiscreteSpace() {
}


template<typename T, typename GPTransformer, typename Adder>
void SharedDiscreteSpace<T, GPTransformer, Adder>::getAgentsToPush(std::set<AgentId>& agentsToTest, std::map<int, std::set<AgentId> >& agentsToPush){

  int buffer = SharedBaseGrid<T, GPTransformer, Adder, int>::_buffer;
  if(buffer == 0) return; // A buffer zone of zero means that no agents will be pushed.

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


  // In a general case, we might not want to do this, but
  // for the current configuration, we can make this (perhaps much) more efficient
  // this way:
  Point<double> localOrigin = SharedBaseGrid<T, GPTransformer, Adder, int>::localBounds.origin();
  Point<double> localExtent = SharedBaseGrid<T, GPTransformer, Adder, int>::localBounds.extents();

  int X1 = localOrigin.getX();
  int X2 = localOrigin.getX() + buffer;
  int X3 = localOrigin.getX() + localExtent.getX() - buffer;
  int X4 = localOrigin.getX() + localExtent.getX();

  int Y1 = localOrigin.getY();
  int Y2 = localOrigin.getY() + buffer;
  int Y3 = localOrigin.getY() + localExtent.getY() - buffer;
  int Y4 = localOrigin.getY() + localExtent.getY();

  Neighbor* neighbor;

  neighbor = SharedBaseGrid<T, GPTransformer, Adder, int>::nghs.neighbor(Neighbors::NW);
  int NW_rank = (neighbor == 0 ? -1 : neighbor->rank());
  neighbor = SharedBaseGrid<T, GPTransformer, Adder, int>::nghs.neighbor(Neighbors::N );
  int N_rank  = (neighbor == 0 ? -1 : neighbor->rank());
  neighbor = SharedBaseGrid<T, GPTransformer, Adder, int>::nghs.neighbor(Neighbors::NE);
  int NE_rank = (neighbor == 0 ? -1 : neighbor->rank());
  neighbor = SharedBaseGrid<T, GPTransformer, Adder, int>::nghs.neighbor(Neighbors::E );
  int E_rank  = (neighbor == 0 ? -1 : neighbor->rank());
  neighbor = SharedBaseGrid<T, GPTransformer, Adder, int>::nghs.neighbor(Neighbors::SE);
  int SE_rank = (neighbor == 0 ? -1 : neighbor->rank());
  neighbor = SharedBaseGrid<T, GPTransformer, Adder, int>::nghs.neighbor(Neighbors::S );
  int S_rank  = (neighbor == 0 ? -1 : neighbor->rank());
  neighbor = SharedBaseGrid<T, GPTransformer, Adder, int>::nghs.neighbor(Neighbors::SW);
  int SW_rank = (neighbor == 0 ? -1 : neighbor->rank());
  neighbor = SharedBaseGrid<T, GPTransformer, Adder, int>::nghs.neighbor(Neighbors::W );
  int W_rank  = (neighbor == 0 ? -1 : neighbor->rank());


  std::set<AgentId> NW_set;
  std::set<AgentId> N_set;
  std::set<AgentId> NE_set;
  std::set<AgentId> E_set;
  std::set<AgentId> SE_set;
  std::set<AgentId> S_set;
  std::set<AgentId> SW_set;
  std::set<AgentId> W_set;


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

  if(NW_set.size() > 0){
    if(W_rank  > -1) agentsToPush[ W_rank].insert(NW_set.begin(), NW_set.end());
    if(NW_rank > -1) agentsToPush[NW_rank].insert(NW_set.begin(), NW_set.end());
    if(N_rank  > -1) agentsToPush[ N_rank].insert(NW_set.begin(), NW_set.end());
  }
  if( N_set.size() > 0){
    if(N_rank  > -1) agentsToPush[ N_rank].insert( N_set.begin(),  N_set.end());
  }
  if(NE_set.size() > 0){
    if(N_rank  > -1) agentsToPush[ N_rank].insert(NE_set.begin(), NE_set.end());
    if(NE_rank > -1) agentsToPush[NE_rank].insert(NE_set.begin(), NE_set.end());
    if(E_rank  > -1) agentsToPush[ E_rank].insert(NE_set.begin(), NE_set.end());
  }
  if( E_set.size() > 0){
    if(E_rank  > -1) agentsToPush[ E_rank].insert( E_set.begin(),  E_set.end());
  }
  if(SE_set.size() > 0){
    if(E_rank  > -1) agentsToPush[ E_rank].insert(SE_set.begin(), SE_set.end());
    if(SE_rank > -1) agentsToPush[SE_rank].insert(SE_set.begin(), SE_set.end());
    if(S_rank  > -1) agentsToPush[ S_rank].insert(SE_set.begin(), SE_set.end());
  }
  if( S_set.size() > 0){
    if(S_rank > -1) agentsToPush[ S_rank].insert( S_set.begin(),  S_set.end());
  }
  if(SW_set.size() > 0){
    if(S_rank  > -1) agentsToPush[ S_rank].insert(SW_set.begin(), SW_set.end());
    if(SW_rank > -1) agentsToPush[SW_rank].insert(SW_set.begin(), SW_set.end());
    if(W_rank  > -1) agentsToPush[ W_rank].insert(SW_set.begin(), SW_set.end());
  }
  if( W_set.size() > 0){
    if(W_rank > -1)  agentsToPush[ W_rank].insert( W_set.begin(),  W_set.end());
  }

}




}

#endif /* SHAREDDISCRETESPACE_H_ */
