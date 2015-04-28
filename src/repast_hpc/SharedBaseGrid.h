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
#include "RepastErrors.h"

namespace repast {


/**
 * Contains the rank and boundaries of a semantically adjacent
 * process (that is, a process that manages the space that is
 * adjacent to the simulation space managed by this process).
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
 * Provides lookup of grid topology process neighbors given
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
 * Allows retrieval of the position of this process within the
 * MPI Cartesian Topology into which it is placed.
 */
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

protected:
	int _buffer;
	GridDimensions localBounds;
	GridDimensions globalBounds;
	Neighbors nghs;
	// vector of ids of agents in this spaces buffer
	std::vector<AgentId> buffered;
	GridDimensions createSendBufferBounds(Neighbors::Location location);

	virtual void synchMoveTo(const AgentId& id, const Point<GPType>& pt) = 0;

	int rank;
	typedef typename repast::BaseGrid<T, MultipleOccupancy<T, GPType> , GPTransformer, Adder, GPType> GridBaseType;
	boost::mpi::communicator* comm;

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
	SharedBaseGrid(std::string name, GridDimensions gridDims, std::vector<int> processDims, int buffer, boost::mpi::communicator* communicator);
	virtual ~SharedBaseGrid();

	/**
	 * Gets the global bounds for this grid
	 *
	 * @return  the global bounds of this SharedGrid.
	 */
	virtual GridDimensions const bounds() const {
		return globalBounds;
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
		int> processDims, int buffer, boost::mpi::communicator* communicator) :
	GridBaseType(name, gridDims), _buffer(buffer), comm(communicator), globalBounds(gridDims) {

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

  if(_buffer == 0) return; // A buffer zone of zero means that no agents will be pushed.

  // In a general case, we might not want to do this, but
  // for the current configuration, we can make this (perhaps much) more efficient
  // this way:
  Point<double> localOrigin = localBounds.origin();
  Point<double> localExtent = localBounds.extents();

  double xStart = localOrigin.getX() + _buffer;
  double xEnd   = localExtent.getX() - _buffer * 2;
  double yStart = localOrigin.getY() + _buffer;
  double yEnd   = localExtent.getY() - _buffer * 2;

  GridDimensions unbuffered(Point<double> (xStart, yStart), Point<double> (xEnd, yEnd));

  Neighbor* neighbor;

  neighbor = nghs.neighbor(Neighbors::NW);
  int NW_rank = (neighbor == 0 ? -1 : neighbor->rank());
  neighbor = nghs.neighbor(Neighbors::N );
  int N_rank  = (neighbor == 0 ? -1 : neighbor->rank());
  neighbor = nghs.neighbor(Neighbors::NE);
  int NE_rank = (neighbor == 0 ? -1 : neighbor->rank());
  neighbor = nghs.neighbor(Neighbors::E );
  int E_rank  = (neighbor == 0 ? -1 : neighbor->rank());
  neighbor = nghs.neighbor(Neighbors::SE);
  int SE_rank = (neighbor == 0 ? -1 : neighbor->rank());
  neighbor = nghs.neighbor(Neighbors::S );
  int S_rank  = (neighbor == 0 ? -1 : neighbor->rank());
  neighbor = nghs.neighbor(Neighbors::SW);
  int SW_rank = (neighbor == 0 ? -1 : neighbor->rank());
  neighbor = nghs.neighbor(Neighbors::W );
  int W_rank  = (neighbor == 0 ? -1 : neighbor->rank());

  GridDimensions empty(Point<double>(0,0), Point<double>(0,0));

  GridDimensions N_bounds  = (N_rank  > - 1 ? createSendBufferBounds(Neighbors::N)  : empty);
  GridDimensions E_bounds  = (E_rank  > - 1 ? createSendBufferBounds(Neighbors::E)  : empty);
  GridDimensions S_bounds  = (S_rank  > - 1 ? createSendBufferBounds(Neighbors::S)  : empty);
  GridDimensions W_bounds  = (W_rank  > - 1 ? createSendBufferBounds(Neighbors::W)  : empty);

  std::set<AgentId> NW_set;
  std::set<AgentId> N_set;
  std::set<AgentId> NE_set;
  std::set<AgentId> E_set;
  std::set<AgentId> SE_set;
  std::set<AgentId> S_set;
  std::set<AgentId> SW_set;
  std::set<AgentId> W_set;


  // Local agents that are in other processes' 'buffer zones' must be exported to those other processes.
  int r = comm->rank();
  std::set<AgentId>::iterator idIter = agentsToTest.begin();
  while(idIter != agentsToTest.end()){
    AgentId id = *idIter;
    bool found = false;
    if(id.currentRank() == r){ // Local agents only
      std::vector<GPType> locationVector;
      GridBaseType::getLocation(id, locationVector);
      Point<GPType> loc(locationVector);
      if(!unbuffered.contains(loc)){
        if(W_bounds.contains(loc)){
          found = true;
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
            found = true;
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
              found = true;
              N_set.insert(id);
            }
            else{
              if(S_bounds.contains(loc)){
                found = true;
                S_set.insert(id);
              }
            }
          }
        }
      }
    }
    if(found){
      std::set<AgentId>::iterator tmp = idIter;
      idIter++;
      agentsToTest.erase(tmp);
    }
    else{
      idIter++;
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
