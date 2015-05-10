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

	Neighbors(int numberOfDimensions);
	virtual ~Neighbors();

	/**
	 * Adds a neighbor at the specified location.
	 *
	 */
	void addNeighbor(Neighbor* ngh, std::vector<int> relativeLocation);

	/**
	 * Gets the neighbor at the specified location.
	 *
	 * @param location the location of the neighbor.
	 */
	Neighbor* neighbor(std::vector<int> relativeLocation) const;

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

  /**
   * Assumes the vector will be of ints that are
   * all -1, 0, or 1; finds the next vector in
   * a series. Returns false if the vector is
   * all 1's.
   */
  bool increment(std::vector<int>& relativeLocation);

  /**
   * Given a relative location of the neighbor,
   * calculates the index of this neighbor
   * in the nghs vector.
   *
   * Relative locations are specified by listing
   * the dimensions and indicating whether the neighbor
   * is before, equal to, or after the coordinate
   * of the reference process on each dimension,
   * using -1, 0, and 1. For example, in a
   * 3-D space, imagining a cube of nine processes
   * with the reference process in the center,
   * the process in the upper left back corner would
   * be [-1, -1, -1].
   *
   * Note: 0, 0, 0 is valid even though it is self,
   * not a neighbor
   *
   */
  int getIndex(std::vector<int> relativeLocation) const;


  /**
   * Gets the max index value.
   *
   * Note that there is a slot for relative position
   * 0,0,0,...; hence the number of neighbors
   * is 3^N - 1, the number of slots is 3^N,
   * and the max index is 3^N - 1 (because first
   * slot is zero).
   */
  int maxIndex(){ return nghs.size() - 1; }

  Neighbor* getNeighborByIndex(int index){
    if(index < 0|| index > maxIndex()) return 0;
    return nghs[index];
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

	int  getRank(std::vector<int>& loc, std::vector<int>& relLoc);
	void createNeighbor(Neighbors* nghs, int rank, std::vector<int> relativeLocation);

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

	void createNeighbors(Neighbors* nghs);

};



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
	Neighbors* nghs;
	// vector of ids of agents in this spaces buffer
	std::vector<AgentId> buffered;
//	GridDimensions createSendBufferBounds(std::vector<int> relativeLocation);

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
    nghs->getNeighborRanks(psToSendTo);
    nghs->getNeighborRanks(psToReceiveFrom);
  }

  virtual void getAgentStatusExchangePartners(std::set<int>& psToSendTo, std::set<int>& psToReceiveFrom){
    nghs->getNeighborRanks(psToSendTo);
    nghs->getNeighborRanks(psToReceiveFrom);
  }

  virtual void updateProjectionInfo(ProjectionInfoPacket* pip, Context<T>* context);

};

template<typename T, typename GPTransformer, typename Adder, typename GPType>
SharedBaseGrid<T, GPTransformer, Adder, GPType>::SharedBaseGrid(std::string name, GridDimensions gridDims, std::vector<
		int> processDims, int buffer, boost::mpi::communicator* communicator) :
	GridBaseType(name, gridDims), _buffer(buffer), comm(communicator), globalBounds(gridDims) {

  rank = comm->rank();

  size_t dimCount = gridDims.dimensionCount();

//  if (dimCount > 2)
//      throw Repast_Error_49<GridDimensions>(dimCount, gridDims); // Number of grid dimensions must be 1 or 2
	if (processDims.size() != gridDims.dimensionCount())
      throw Repast_Error_50<GridDimensions>(dimCount, gridDims, processDims.size()); // Number of grid dimensions must be equal to number of process dimensions

	bool periodic = GridBaseType::gpTransformer.isPeriodic();
	CartTopology topology(processDims, gridDims.origin().coords(), gridDims.extents().coords(), periodic, comm);

	std::vector<int> coords;
	topology.getCoordinates(rank, coords);
	localBounds = topology.getDimensions(coords);
	GridBaseType::adder.init(localBounds, this);

	nghs = new Neighbors(dimCount);
	topology.createNeighbors(nghs);
}

template<typename T, typename GPTransformer, typename Adder, typename GPType>
SharedBaseGrid<T, GPTransformer, Adder, GPType>::~SharedBaseGrid() {
  delete nghs;
}


//template<typename T, typename GPTransformer, typename Adder, typename GPType>
//GridDimensions SharedBaseGrid<T, GPTransformer, Adder, GPType>::createSendBufferBounds(std::vector<int> relativeLocation) {
//	Point<double> localOrigin = localBounds.origin();
//	Point<double> localExtent = localBounds.extents();
//
////	switch (location) {
////	double xStart, yStart;
////case Neighbors::E:
////	xStart = localOrigin.getX() + localExtent.getX() - _buffer;
////	return GridDimensions(Point<double> (xStart, localOrigin.getY()), Point<double> (_buffer, localExtent.getY()));
////
////case Neighbors::W:
////	return GridDimensions(localOrigin, Point<double> (_buffer, localExtent.getY()));
////
////case Neighbors::N:
////	return GridDimensions(localOrigin, Point<double> (localExtent.getX(), _buffer));
////
////case Neighbors::S:
////	yStart = localOrigin.getY() + localExtent.getY() - _buffer;
////	return GridDimensions(Point<double> (localOrigin.getX(), yStart), Point<double> (localExtent.getX(), _buffer));
////
////case Neighbors::NE:
////	xStart = localOrigin.getX() + localExtent.getX() - _buffer;
////	return GridDimensions(Point<double> (xStart, localOrigin.getY()), Point<double> (_buffer, _buffer));
////
////case Neighbors::NW:
////	return GridDimensions(Point<double> (localOrigin.getX(), localOrigin.getY()), Point<double> (_buffer, _buffer));
////
////case Neighbors::SE:
////	xStart = localOrigin.getX() + localExtent.getX() - _buffer;
////	yStart = localOrigin.getY() + localExtent.getY() - _buffer;
////	return GridDimensions(Point<double> (xStart, yStart), Point<double> (_buffer, _buffer));
////
////case Neighbors::SW:
////	yStart = localOrigin.getY() + localExtent.getY() - _buffer;
////	return GridDimensions(Point<double> (localOrigin.getX(), yStart), Point<double> (_buffer, _buffer));
////	}
//
//	return GridDimensions();
//}


template<typename T, typename GPTransformer, typename Adder, typename GPType>
void SharedBaseGrid<T, GPTransformer, Adder, GPType>::balance() {
  int r = comm->rank();
  typename GridBaseType::LocationMapConstIter iterEnd = GridBaseType::locationsEnd();
  for (typename GridBaseType::LocationMapConstIter iter = GridBaseType::locationsBegin(); iter != iterEnd; ++iter) {
    AgentId id = iter->second->ptr->getId();
    if(id.currentRank() == r){                                   // Local agents only
      Point<GPType> loc = iter->second->point;
      if(!localBounds.contains(loc)){                            // If inside bounds, ignore
        Neighbor* ngh = nghs->findNeighbor(loc.coords());
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

  int numDims = localBounds.dimensionCount();

  // First, create a zone around the center of this process, inside all of
  // of the buffer zones
  std::vector<double> unbufferedOrigin;
  std::vector<double> unbufferedExtents;

  for(int i = 0; i < numDims; i++){
    unbufferedOrigin.push_back(localBounds.origin(i) + _buffer);
    unbufferedExtents.push_back(localBounds.extents(i) - (_buffer * 2));
  }

  Point<double> ubO(unbufferedOrigin);
  Point<double> ubE(unbufferedExtents);
  GridDimensions unbuffered(ubO, ubE);

  // And create grid boundaries for all the buffer zones
  int numOutgoing = nghs->maxIndex() + 1;
  GridDimensions** outgoing = new GridDimensions*[numOutgoing];
  int*            outRanks = new int[numOutgoing];

  std::vector<int> relativeLocation;
  relativeLocation.assign(numDims, -1);
  do{
    std::vector<double> bufferOrigin;
    std::vector<double> bufferExtents;

    bool isEgo = true;
    for(int i = 0; i < numDims; i++){
      int rel = relativeLocation[i];
      if(rel == 0){
        bufferOrigin.push_back(localBounds.origin(i));
        bufferExtents.push_back(localBounds.extents(i));
      }
      else{
        if(rel < 0){
          bufferOrigin.push_back(localBounds.origin(i));
        }
        else{
          bufferOrigin.push_back(localBounds.origin(i) + localBounds.extents(i) - _buffer);
        }
        bufferExtents.push_back(_buffer);
        isEgo = false;
      }
    }

    // Should not add self!
    int index = nghs->getIndex(relativeLocation);
    if(!isEgo){
      outgoing[index] = new GridDimensions(Point<double>(bufferOrigin), Point<double> (bufferExtents));
      outRanks[index]  =nghs->getNeighborByIndex(index)->rank();
    }
    else{
      outgoing[index] = 0;
      outRanks[index] = 0;
    }

  }while(nghs->increment(relativeLocation));




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
        for(int i = 0; i < numOutgoing; i++){
          if((outgoing[i] > 0) && (outgoing[i]->contains(loc))){
            agentsToPush[outRanks[i]].insert(id);
            found = true;
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
//  if(NW_set.size() > 0) agentsToPush[NW_rank].insert(NW_set.begin(), NW_set.end());
  delete[] outgoing;
  delete[] outRanks;
}

template<typename T, typename GPTransformer, typename Adder, typename GPType>
void SharedBaseGrid<T, GPTransformer, Adder, GPType>::updateProjectionInfo(ProjectionInfoPacket* pip, Context<T>* context){
  SpecializedProjectionInfoPacket<GPType>* spip = static_cast<SpecializedProjectionInfoPacket<GPType>*>(pip);
  synchMoveTo(spip->id, spip->data);
}

}

#endif /* SHAREDBASEGRID_H_ */
