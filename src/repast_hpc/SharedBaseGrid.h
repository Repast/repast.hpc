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
 *  SharedBaseGrid.h
 *
 *  Created on: Sep 10, 2009
 *      Author: nick
 */

#ifndef SHAREDBASEGRID_H_
#define SHAREDBASEGRID_H_

#include <cmath>
#include <mpi.h>
#include <iostream>
#include <algorithm>
#include <sstream>

#include "BaseGrid.h"
#include "GridComponents.h"
#include "MultipleOccupancy.h"
#include "RepastProcess.h"
#include "logger.h"
#include "SRManager.h"
#include "GridMovePackets.h"
#include "RepastErrors.h"

namespace repast {

// tags for the pair-wise grid
// buffer syncs
const int GRID_BUFFER_SYNC0 = 3000;
const int GRID_BUFFER_SYNC1 = 3001;
const int GRID_BUFFER_SYNC2 = 3002;
const int GRID_BUFFER_SYNC3 = 3003;
const int GRID_BUFFER_SYNC4 = 3004;
const int GRID_BUFFER_SYNC5 = 3005;
const int GRID_BUFFER_SYNC6 = 3006;
const int GRID_BUFFER_SYNC7 = 3007;

/**
 * NON USER API.
 *
 * Encapsulates the contents of a grid / space location
 * so that it can be sent between processes.
 */
template<typename AgentContent, typename GPType>
class CellContents {
public:
	friend class boost::serialization::access;

	Point<GPType> _pt;
	std::vector<AgentContent> _objs;

	template<class Archive>
	void serialize(Archive& ar, const unsigned int version) {
		ar & _pt;
		ar & _objs;
	}

	// no-arg for serialization
	CellContents();
	CellContents(Point<GPType> pt);
};

template<typename AgentContent, typename GPType>
CellContents<AgentContent, GPType>::CellContents() :
	_pt(0) {
}

template<typename AgentContent, typename GPType>
CellContents<AgentContent, GPType>::CellContents(Point<GPType> pt) :
	_pt(pt) {
}

/**
 * NON USER API.
 *
 * A grid topology process neighbor.
 */
class Neighbor {

private:
	int            _rank;
	GridDimensions _bounds;

public:
	Neighbor(int rank, GridDimensions bounds);

  int            rank() const {     return _rank;   }
  GridDimensions bounds() const {   return _bounds; }

};


/**
 * NON USER API.
 *
 * Provides look up grid topology process neighbors given
 * a point in the pan process grid.
 */
class Neighbors {

  friend std::ostream& operator<<(std::ostream& os, const Neighbors& nghs);

private:

	std::vector<Neighbor*> nghs;

public:

	/**
	 * Describes the relative location of grid topology
	 * process neighbors.
	 */
	enum Location {
		E, W, N, S, NE, NW, SE, SW
	};
	static const int LOCATION_SIZE = 8;

	Neighbors();
	virtual ~Neighbors();

	/**
	 * Adds a neighbor at the specified location.
	 *
	 */
	void addNeighbor(Neighbor* ngh, Neighbors::Location location);

	/**
	 * Gets the neighbor at the specified location.
	 *
	 * @param location the location of the neighbor.
	 */
	Neighbor* neighbor(Neighbors::Location location) const;

	/**
	 * Finds the neighbor that contains the specified point.
	 *
	 * @return the found neighbor
	 */
	Neighbor* findNeighbor(const std::vector<int>& pt);

	/**
	 * Finds the neighbor that contains the specified point.
	 *
	 * @return the found neighbor
	 */
	Neighbor* findNeighbor(const std::vector<double>& pt);

  void getNeighborRanks(std::set<int>& ranks){
    for(std::vector<Neighbor*>::iterator iter = nghs.begin(), iterEnd=nghs.end(); iter != iterEnd; ++iter) ranks.insert((*iter)->rank());
  }

};

std::ostream& operator<<(std::ostream& os, const Neighbors& nghs);


/**
 * NON USER API.
 *
 * Helper class that provides support for synchronizing a
 * grid / space buffer.
 */
template<typename T, typename GPType>
class GridBufferSyncher {

private:
	std::vector<boost::mpi::request> requests;
	std::vector<std::vector<CellContents<T, GPType> >*> vecs;
	std::vector<int> nghRanks;

	boost::mpi::communicator* comm;

public:
	GridBufferSyncher(boost::mpi::communicator* world): comm(world) {
	}
	virtual ~GridBufferSyncher();

	std::vector<CellContents<T, GPType> >* received(size_t index) {
		return vecs[index];
	}

	int nghRank(size_t index) {
		return nghRanks[index];
	}

	size_t vecsSize() {
		return vecs.size();
	}

	/**
	 * Sends the contents to the rank.
	 */
	void send(int rank, std::vector<CellContents<T, GPType> >& contents, int tag);
	void receive(Neighbor* ngh, int tag);
	void wait();
};

template<typename T, typename GPType>
void GridBufferSyncher<T, GPType>::send(int rank, std::vector<CellContents<T, GPType> >& contents, int tag) {
	requests.push_back(comm->isend(rank, tag, contents));
}

template<typename T, typename GPType>
void GridBufferSyncher<T, GPType>::receive(Neighbor* ngh, int tag) {
	if (ngh != 0) {
		std::vector<CellContents<T, GPType> >* vec = new std::vector<CellContents<T, GPType> >();
		vecs.push_back(vec);
		nghRanks.push_back(ngh->rank());
		requests.push_back(comm->irecv(ngh->rank(), tag, *vec));
	}
}

template<typename T, typename GPType>
void GridBufferSyncher<T, GPType>::wait() {
	boost::mpi::wait_all(requests.begin(), requests.end());
	requests.clear();
}

template<typename T, typename GPType>
GridBufferSyncher<T, GPType>::~GridBufferSyncher() {
	for (int i = 0, n = vecs.size(); i < n; ++i) {
		delete vecs[i];
	}
}



class CartTopology {

private:
  MPI_Comm           topologyComm;
  GridDimensions     globalBounds;
  bool               periodic;
  std::vector<int>   procsPerDim;

	template <typename T>
	void swapXY(std::vector<T>& vec);

	int  getRank(std::vector<int>& loc, int rowAdj, int colAdj);
	void createNeighbor(Neighbors& nghs, int rank, Neighbors::Location location);

public:
	// x major
	CartTopology(std::vector<int> processesPerDim, std::vector<double> origin, std::vector<double> extents, bool spaceIsPeriodic, boost::mpi::communicator* world);

  /**
   * Gets the coordinates in the MPI Cartesian Communicator
   * for the specified rank
   */
  void getCoordinates(int rank, std::vector<int>& coords);

  /**
   * Gets the GridDimensions boundaries for the specified
   * rank
   */
  GridDimensions getDimensions(int rank);

  /**
   * Gets the GridDimensions boundaries for the specified
   * MPI coordinates
   */
  GridDimensions getDimensions(std::vector<int>& pCoordinates);

	void createNeighbors(Neighbors& nghs);

};

template <typename T>
void CartTopology::swapXY(std::vector<T>& vec) {
  if (vec.size() > 1) {
    T tmp = vec[0];
    vec[0] = vec[1];
    vec[1] = tmp;
  }
}





/**
 * Grid / Space implementation specialized for the distributed context.
 * Each SharedBaseGrid of the same name running on different processes
 * is part of a pan process grid. This class manages this local part of
 * the grid and its communication with its process neighbors. Users
 * can specify a buffer size that determines how much of the neighboring grids
 * are visible in this grid. For example, if this grid originates at 0x0 and ends
 * at 3x3, a buffer of 1 would make the locations (4,0), (4,1) (4,2) ... (4,4)
 * and (0,4), (1,4)... (4,4) visible in this grid. The SharedBaseGrid takes many
 * template parameters. Default variations of these that define typical grids and
 * spaces are given in SharedGrids in SharedSpace.h
 *
 * @tparam T the type of objects contained by this BaseGrid
 * @tparam GPTransformer transforms cell points according to the topology (e.g. periodic)
 * of the BaseGrid.
 * @tparam Adder determines how objects are added to the grid from its associated context.
 * @tparam GPType the coordinate type of the grid point locations. This must
 * be an int or a double.
 */
template<typename T, typename GPTransformer, typename Adder, typename GPType>
class SharedBaseGrid: public BaseGrid<T, MultipleOccupancy<T, GPType> , GPTransformer, Adder, GPType> {

private:
	GridDimensions localBounds;
	int _buffer;
	GridMovePackets<GPType> movePackets;

protected:
	Neighbors nghs;
	// vector of ids of agents in this spaces buffer
	std::vector<AgentId> buffered;
	GridDimensions createSendBufferBounds(Neighbors::Location location);

	virtual void synchMoveTo(const AgentId& id, const Point<GPType>& pt) = 0;

	int rank;
	typedef typename repast::BaseGrid<T, MultipleOccupancy<T, GPType> , GPTransformer, Adder, GPType> GridBaseType;
	boost::mpi::communicator* comm;


  void getMovingAgentInfo(std::map<int, std::vector<AgentId> > agentsToMove, GridMovePackets<GPType> outgoing);

  bool locationIsInBuffer(Point<GPType> pt){ return false; }
  bool agentIsInBuffer(AgentId id){ return false; }

public:
  void balance();

	// overriding moveTo that takes newLocation hides the moveTo in the
	// Grid base class that takes a Point. This using directive
	// makes the Point arg moveTo available.
	using GridBaseType::moveTo;

	/**
	 * Creates a SharedGrid with the specified name.
	 *
	 * @param name the name of this SharedBaseGrid
	 * @param gridDims the dimensions of the entire pan-process grid
	 * @param processDims the number of processes in each dimension. This must
	 * divide evenly into gridDims.
	 * @param buffer the size of the buffer between this part of the pan-process grid
	 * and its neighbors.
	 */
	SharedBaseGrid(std::string name, GridDimensions gridDims, std::vector<int> processDims, int buffer, boost::mpi::communicator* world);
	virtual ~SharedBaseGrid();

	/**
	 * Gets the local bounds of this SharedGrid. The local bounds
	 * are the dimensions of the section of the pan-process grid represented
	 * by this SharedGrid.
	 *
	 * @return  the local bounds of this SharedGrid.
	 */
	GridDimensions bounds() const {
		return localBounds;
	}

	/**
	 * Gets the local bounds of this SharedGrid. The local bounds
	 * are the dimensions of the section of the pan-process grid represented
	 * by this SharedGrid.
	 *
	 * @return  the local bounds of this SharedGrid.
	 */
	virtual const GridDimensions dimensions() const {
		return localBounds;
	}

	/**
	 * Synchronizes the movement of agents off on one grid and onto another.
	 * If there is any chance that an agent has moved off the local
	 * dimensions of this SharedGrid and into those managed by another
	 * then this must be called.
	 */
	void synchMove();

	/**
	 * Initializes the synch buffer operation. This should be called
	 * before synchronizing the buffers themselves.
	 *
	 * @param the SharedContext that contains this SharedGrid projection.
	 */
	void initSynchBuffer(SharedContext<T>& context);

	// doc inherited from BaseGrid.h
	virtual bool moveTo(const AgentId& id, const std::vector<GPType>& newLocation);

	// doc inherited from BaseGrid.h
	virtual bool moveTo(const AgentId& id, const Point<GPType>& pt);

	virtual void removeAgent(T* agent);

	// doc inherited from Projection.h

  virtual void getRequiredAgents(std::set<AgentId>& agentsToTest, std::set<AgentId>& agentsRequired){ } // Grids don't keep agents

  virtual void getAgentsToPush(std::set<AgentId>& agentsToTest, std::map<int, std::set<AgentId> >& agentsToPush);


  virtual void getInfoExchangePartners(std::set<int>& psToSendTo, std::set<int>& psToReceiveFrom){
    nghs.getNeighborRanks(psToSendTo);
    nghs.getNeighborRanks(psToReceiveFrom);
  }

  virtual void getAgentStatusExchangePartners(std::set<int>& psToSendTo, std::set<int>& psToReceiveFrom){
    nghs.getNeighborRanks(psToSendTo);
    nghs.getNeighborRanks(psToReceiveFrom);
  }

  virtual void updateProjectionInfo(ProjectionInfoPacket* pip, Context<T>* context);

};

template<typename T, typename GPTransformer, typename Adder, typename GPType>
SharedBaseGrid<T, GPTransformer, Adder, GPType>::SharedBaseGrid(std::string name, GridDimensions gridDims, std::vector<
		int> processDims, int buffer, boost::mpi::communicator* world) :
	GridBaseType(name, gridDims), _buffer(buffer), comm(world) {

  rank = comm->rank();

  size_t dimCount = gridDims.dimensionCount();

  if (dimCount > 2)
      throw Repast_Error_49<GridDimensions>(dimCount, gridDims); // Number of grid dimensions must be 1 or 2
	if (processDims.size() != gridDims.dimensionCount())
      throw Repast_Error_50<GridDimensions>(dimCount, gridDims, processDims.size()); // Number of grid dimensions must be equal to number of process dimensions

	bool periodic = GridBaseType::gpTransformer.isPeriodic();
	CartTopology topology(processDims, gridDims.origin().coords(), gridDims.extents().coords(), periodic, comm);

	std::vector<int> coords;
	topology.getCoordinates(rank, coords);
	localBounds = topology.getDimensions(coords);
	GridBaseType::adder.init(localBounds, this);

	topology.createNeighbors(nghs);
}

template<typename T, typename GPTransformer, typename Adder, typename GPType>
SharedBaseGrid<T, GPTransformer, Adder, GPType>::~SharedBaseGrid() { }


template<typename T, typename GPTransformer, typename Adder, typename GPType>
GridDimensions SharedBaseGrid<T, GPTransformer, Adder, GPType>::createSendBufferBounds(Neighbors::Location location) {
	Point<double> localOrigin = localBounds.origin();
	Point<double> localExtent = localBounds.extents();

	switch (location) {
	double xStart, yStart;
case Neighbors::E:
	xStart = localOrigin.getX() + localExtent.getX() - _buffer;
	return GridDimensions(Point<double> (xStart, localOrigin.getY()), Point<double> (_buffer, localExtent.getY()));

case Neighbors::W:
	return GridDimensions(localOrigin, Point<double> (_buffer, localExtent.getY()));

case Neighbors::N:
	return GridDimensions(localOrigin, Point<double> (localExtent.getX(), _buffer));

case Neighbors::S:
	yStart = localOrigin.getY() + localExtent.getY() - _buffer;
	return GridDimensions(Point<double> (localOrigin.getX(), yStart), Point<double> (localExtent.getX(), _buffer));

case Neighbors::NE:
	xStart = localOrigin.getX() + localExtent.getX() - _buffer;
	return GridDimensions(Point<double> (xStart, localOrigin.getY()), Point<double> (_buffer, _buffer));

case Neighbors::NW:
	return GridDimensions(Point<double> (localOrigin.getX(), localOrigin.getY()), Point<double> (_buffer, _buffer));

case Neighbors::SE:
	xStart = localOrigin.getX() + localExtent.getX() - _buffer;
	yStart = localOrigin.getY() + localExtent.getY() - _buffer;
	return GridDimensions(Point<double> (xStart, yStart), Point<double> (_buffer, _buffer));

case Neighbors::SW:
	yStart = localOrigin.getY() + localExtent.getY() - _buffer;
	return GridDimensions(Point<double> (localOrigin.getX(), yStart), Point<double> (_buffer, _buffer));
	}

	return GridDimensions();
}

template<typename T, typename GPTransformer, typename Adder, typename GPType>
void SharedBaseGrid<T, GPTransformer, Adder, GPType>::initSynchBuffer(SharedContext<T>& context) {

	// removing from the context should be enough to delete the
	// pointer correctly because the context uses shared pointers
	for (std::vector<AgentId>::iterator iter = buffered.begin(); iter != buffered.end(); ++iter) {
		AgentId id = *iter;

		context.decrementProjRefCount(*iter);
		context.removeAgent(*iter);
	}
	buffered.clear();

}

template<typename T, typename GPTransformer, typename Adder, typename GPType>
void SharedBaseGrid<T, GPTransformer, Adder, GPType>::synchMove() {

	SRManager manager(comm);
	std::vector<int> senders;

	std::vector<int> receivers;
	movePackets.receivers(receivers);
	manager.retrieveSources(receivers, senders, GRID_MOVE_SYNC_SENDERS);

	std::vector<boost::mpi::request> requests;
	movePackets.send(requests, *comm);

	std::vector<std::vector<GridMovePacket<GPType> >*> packetList;
	for (int i = 0, n = senders.size(); i < n; ++i) {
		std::vector<GridMovePacket<GPType> >* packets = new std::vector<GridMovePacket<GPType> >();
		requests.push_back(comm->irecv(senders[i], GRID_MOVE_SYNC_PACKETS, *packets));
		packetList.push_back(packets);
	}
	boost::mpi::wait_all(requests.begin(), requests.end());
	movePackets.clear();

	for (int i = 0, n = packetList.size(); i < n; ++i) {
		std::vector<GridMovePacket<GPType> >* packets = packetList[i];
		for (int k = 0, j = packets->size(); k < j; ++k) {
			GridMovePacket<GPType>& packet = (*packets)[k];
			synchMoveTo(packet._id, packet._pt);
		}
		delete packets;
	}
}


template<typename T, typename GPTransformer, typename Adder, typename GPType>
void SharedBaseGrid<T, GPTransformer, Adder, GPType>::balance() {
  int r = comm->rank();
  typename GridBaseType::LocationMapConstIter iterEnd = GridBaseType::locationsEnd();
  for (typename GridBaseType::LocationMapConstIter iter = GridBaseType::locationsBegin(); iter != iterEnd; ++iter) {
    AgentId id = iter->second->ptr->getId();

    if(id.currentRank() == r){                                   // Local agents only
      Point<GPType> loc = iter->second->point;
      if(!localBounds.contains(loc)){                            // If inside bounds, ignore
        Neighbor* ngh = nghs.findNeighbor(loc.coords());
        RepastProcess::instance()->moveAgent(id, ngh->rank());
      }
    }
  }
}

template<typename T, typename GPTransformer, typename Adder, typename GPType>
void SharedBaseGrid<T, GPTransformer, Adder, GPType>::getMovingAgentInfo(std::map<int, std::vector<AgentId> > agentsToMove, GridMovePackets<GPType> outgoing){
  std::map<int, std::vector<AgentId> >::const_iterator mapIterEnd = agentsToMove.end();
  for(std::map<int, std::vector<AgentId> >::const_iterator mapIter = agentsToMove.begin(); mapIter != mapIterEnd; ++mapIter){
    int dest                      = mapIter->first;
    std::vector<AgentId> agentIDs = mapIter->second;

    std::vector<AgentId>::const_iterator agentIdIterEnd = agentIDs.end();
    for(std::vector<AgentId>::const_iterator agentIdIter = agentIDs.begin(); agentIdIter != agentIdIterEnd; ++agentIdIter){
      AgentId id = *agentIdIter;
      Point<GPType> loc = GridBaseType::agentToLocation.find(id)->second.point;
      GridMovePacket<GPType> packet(loc, id, mapIter->first);
      outgoing.addPacket(packet);
    }
  }
}

template<typename T, typename GPTransformer, typename Adder, typename GPType>
bool SharedBaseGrid<T, GPTransformer, Adder, GPType>::moveTo(const AgentId& id, const Point<GPType>& newLocation) {
	return SharedBaseGrid<T, GPTransformer, Adder, GPType>::moveTo(id, newLocation.coords());
}

template<typename T, typename GPTransformer, typename Adder, typename GPType>
bool SharedBaseGrid<T, GPTransformer, Adder, GPType>::moveTo(const AgentId& id, const std::vector<GPType>& newLocation) {
	return GridBaseType::moveTo(id, newLocation);
}

template<typename T, typename GPTransformer, typename Adder, typename GPType>
void SharedBaseGrid<T, GPTransformer, Adder, GPType>::removeAgent(T* agent) {
	GridBaseType::removeAgent(agent);
}


// Beta

template<typename T, typename GPTransformer, typename Adder, typename GPType>
void SharedBaseGrid<T, GPTransformer, Adder, GPType>::getAgentsToPush(std::set<AgentId>& agentsToTest, std::map<int, std::set<AgentId> >& agentsToPush){

  // In a general case, we might not want to do this, but
  // for the current configuration, we can make this (perhaps much) more efficient
  // this way:
  Point<double> localOrigin = localBounds.origin();
  Point<double> localExtent = localBounds.extents();

  double xStart = localOrigin.getX() + _buffer;
  double xEnd   = localOrigin.getX() + localExtent.getX() - _buffer * 2;
  double yStart = localOrigin.getY() + _buffer;
  double yEnd   = localOrigin.getY() + localExtent.getY() - _buffer * 2;
  GridDimensions unbuffered(Point<double> (xStart, yStart), Point<double> (xEnd, yEnd));

  GridDimensions NW_bounds = createSendBufferBounds(Neighbors::NW);  int NW_rank = nghs.neighbor(Neighbors::NW)->rank(); std::set<AgentId> NW_set;
  GridDimensions N_bounds  = createSendBufferBounds(Neighbors::N);   int N_rank  = nghs.neighbor(Neighbors::N)->rank();  std::set<AgentId> N_set;
  GridDimensions NE_bounds = createSendBufferBounds(Neighbors::NE);  int NE_rank = nghs.neighbor(Neighbors::NE)->rank(); std::set<AgentId> NE_set;
  GridDimensions E_bounds  = createSendBufferBounds(Neighbors::E);   int E_rank  = nghs.neighbor(Neighbors::E)->rank();  std::set<AgentId> E_set;
  GridDimensions SE_bounds = createSendBufferBounds(Neighbors::SE);  int SE_rank = nghs.neighbor(Neighbors::SE)->rank(); std::set<AgentId> SE_set;
  GridDimensions S_bounds  = createSendBufferBounds(Neighbors::S);   int S_rank  = nghs.neighbor(Neighbors::S)->rank();  std::set<AgentId> S_set;
  GridDimensions SW_bounds = createSendBufferBounds(Neighbors::SW);  int SW_rank = nghs.neighbor(Neighbors::SW)->rank(); std::set<AgentId> SW_set;
  GridDimensions W_bounds  = createSendBufferBounds(Neighbors::W);   int W_rank  = nghs.neighbor(Neighbors::W)->rank();  std::set<AgentId> W_set;


  // Local agents that are in other processes' 'buffer zones' must be exported to those other processes.
  int r = comm->rank();
  for(typename std::set<AgentId>::iterator idIter = agentsToTest.begin(), idIterEnd = agentsToTest.end(); idIter != idIterEnd; ++idIter){
    AgentId id = *idIter;
    if(id.currentRank() == r){ // Local agents only
      std::vector<GPType> locationVector;
      GridBaseType::getLocation(id, locationVector);
      Point<GPType> loc(locationVector);
      if(!unbuffered.contains(loc)){
        if(W_bounds.contains(loc)){
          W_set.insert(id);
          if(N_bounds.contains(loc)){
            N_set.insert(id);
            NW_set.insert(id);
          }
          else{
            if(S_bounds.contains(loc)){
              S_set.insert(id);
              SW_set.insert(id);
            }
          }
        }
        else{
          if(E_bounds.contains(loc)){
            E_set.insert(id);
            if(N_bounds.contains(loc)){
              N_set.insert(id);
              NE_set.insert(id);
            }
            else{
              if(S_bounds.contains(loc)){
                S_set.insert(id);
                SE_set.insert(id);
              }
            }
          }
          else{
            if(N_bounds.contains(loc)){
              N_set.insert(id);
            }
            else{
              if(S_bounds.contains(loc)){
                S_set.insert(id);
              }
            }
          }
        }
      }
    }
  }

  if(NW_set.size() > 0) agentsToPush[NW_rank].insert(NW_set.begin(), NW_set.end());
  if( N_set.size() > 0) agentsToPush[ N_rank].insert( N_set.begin(),  N_set.end());
  if(NE_set.size() > 0) agentsToPush[NE_rank].insert(NE_set.begin(), NE_set.end());
  if( E_set.size() > 0) agentsToPush[ E_rank].insert( E_set.begin(),  E_set.end());
  if(SE_set.size() > 0) agentsToPush[SE_rank].insert(SE_set.begin(), SE_set.end());
  if( S_set.size() > 0) agentsToPush[ S_rank].insert( S_set.begin(),  S_set.end());
  if(SW_set.size() > 0) agentsToPush[SW_rank].insert(SW_set.begin(), SW_set.end());
  if( W_set.size() > 0) agentsToPush[ W_rank].insert( W_set.begin(),  W_set.end());


}

template<typename T, typename GPTransformer, typename Adder, typename GPType>
void SharedBaseGrid<T, GPTransformer, Adder, GPType>::updateProjectionInfo(ProjectionInfoPacket* pip, Context<T>* context){
  SpecializedProjectionInfoPacket<GPType>* spip = static_cast<SpecializedProjectionInfoPacket<GPType>*>(pip);
  synchMoveTo(spip->id, spip->data);
}

}

#endif /* SHAREDBASEGRID_H_ */
