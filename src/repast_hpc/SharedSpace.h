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
 *  SharedSpace.h
 *
 *  Created on: Jul 20, 2010
 *      Author: nick
 */

#ifndef SHAREDSPACE_H_
#define SHAREDSPACE_H_

#include <boost/mpi/communicator.hpp>

#include "SharedGrid.h"

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
class SharedGrid: public SharedBaseGrid<T, GPTransformer, Adder, int> {

protected:
	virtual void synchMoveTo(const AgentId& id, const Point<int>& pt);

private:
	template<typename AgentContent, typename ContentProvider>
	void sendToNeighbor(GridBufferSyncher<AgentContent, int>& syncher, Neighbors::Location location,
			ContentProvider& provider, int val);

	typedef SharedBaseGrid<T, GPTransformer, Adder, int> SharedBaseGridType;

public:
	virtual ~SharedGrid();
	SharedGrid(std::string name, GridDimensions gridDims, std::vector<int> processDims, int buffer, boost::mpi::communicator* world);

	/**
	 * Synchronize the buffer area of this SharedGrid with its neighbors. This
	 * will copy the buffer area from the neighbors into this SharedGrid.
	 * This should be called immediately after initSynchBuffer.
	 *
	 * @param context the SharedContext that contains the agents in this SharedGrid.
	 * @param provider a class that provides AgentContent for the agents being buffered
	 * in neighboring grids.
	 * @param creator a class that creates a agents of type T when given
	 * AgentContent.
	 *
	 * @tparam T the type of agent in this SharedGrid
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
};

template<typename T, typename GPTransformer, typename Adder>
SharedGrid<T, GPTransformer, Adder>::SharedGrid(std::string name, GridDimensions gridDims,
		std::vector<int> processDims, int buffer, boost::mpi::communicator* world) :
	SharedBaseGrid<T, GPTransformer, Adder, int> (name, gridDims, processDims, buffer, world) {
}

template<typename T, typename GPTransformer, typename Adder>
void SharedGrid<T, GPTransformer, Adder>::synchMoveTo(const AgentId& id, const Point<int>& pt) {
	//unlikely chance that agent could have
	// moved and then "died" and so removed from sending context, in which case
	// it would never get sent to this grid.
	if (SharedBaseGridType::GridBaseType::contains(id)) {
		SharedBaseGridType::GridBaseType::moveTo(id, pt.coords());
	}
}

template<typename T, typename GPTransformer, typename Adder>
template<typename AgentContent, typename ContentProvider>
void SharedGrid<T, GPTransformer, Adder>::sendToNeighbor(GridBufferSyncher<AgentContent, int>& syncher,
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
void SharedGrid<T, GPTransformer, Adder>::synchBuffer(SharedContext<T>& context, ContentProvider& provider,
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
				if (!context.addAgent(obj)) {
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
SharedGrid<T, GPTransformer, Adder>::~SharedGrid() {
}

/**
 * Continuous space SharedBaseGrid implementation. This
 * primarily adds the buffer synchronization appropriate for this
 * type. Default templated typical SharedSpaces are defined in SharedGrids.
 *
 * @see SharedBaseGrid for more details.
 *
 * @tparam T the type of objects contained by this BaseGrid
 * @tparam GPTransformer transforms cell points according to the topology (e.g. periodic)
 * of the BaseGrid.
 * @tparam Adder determines how objects are added to the grid from its associated context.
 */
template<typename T, typename GPTransformer, typename Adder>
class SharedSpace: public SharedBaseGrid<T, GPTransformer, Adder, double> {

protected:
	virtual void synchMoveTo(const AgentId& id, const Point<double>& pt);

private:
	template<typename AgentContent, typename ContentProvider>
	void sendToNeighbor(GridBufferSyncher<AgentContent, double>& syncher, Neighbors::Location location,
			ContentProvider& provider, int val);

	typedef SharedBaseGrid<T, GPTransformer, Adder, double> SharedBaseGridType;

public:
	virtual ~SharedSpace();
	SharedSpace(std::string name, GridDimensions gridDims, std::vector<int> processDims, int buffer, boost::mpi::communicator* world);

	/**
	 * Synchronize the buffer area of this SharedGrid with its neighbors. This
	 * will copy the buffer area from the neighbors into this SharedGrid.
	 * This should be called immediately after initSynchBuffer.
	 *
	 * @param context the SharedContext that contains the agents in this SharedGrid.
	 * @param provider a class that provides AgentContent for the agents being buffered
	 * in neighboring grids.
	 * @param creator a class that creates a agents of type T when given
	 * AgentContent.
	 *
	 * @tparam T the type of agent in this SharedGrid
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

	void printLocations();

};

template<typename T, typename GPTransformer, typename Adder>
SharedSpace<T, GPTransformer, Adder>::SharedSpace(std::string name, GridDimensions gridDims,
		std::vector<int> processDims, int buffer, boost::mpi::communicator* world) :
	SharedBaseGrid<T, GPTransformer, Adder, double> (name, gridDims, processDims, buffer, world) {
}

template<typename T, typename GPTransformer, typename Adder>
void SharedSpace<T, GPTransformer, Adder>::synchMoveTo(const AgentId& id, const Point<double>& pt) {
	//unlikely chance that agent could have
	// moved and then "died" and so removed from sending context, in which case
	// it would never get sent to this grid.
	if (SharedBaseGridType::GridBaseType::contains(id)) {
		SharedBaseGridType::GridBaseType::moveTo(id, pt.coords());
	}
}

template<typename T, typename GPTransformer, typename Adder>
template<typename AgentContent, typename ContentProvider>
void SharedSpace<T, GPTransformer, Adder>::sendToNeighbor(GridBufferSyncher<AgentContent, double>& syncher,
		Neighbors::Location location, ContentProvider& provider, int val) {

	Neighbor* ngh = SharedBaseGridType::nghs.neighbor(location);
	if (ngh != 0) {
		std::vector<CellContents<AgentContent, double> > toSend;
		GridDimensions bounds = SharedBaseGridType::createSendBufferBounds(location);
		// check each agent if its within the bounds and if so get its content etc.
		// TODO this is crazy inefficient -- need a KDTree or something similar
		// container, so can get range out of that together with their points
		for (typename SharedBaseGridType::GridBaseType::LocationMapConstIter iter =
				SharedBaseGridType::GridBaseType::locationsBegin(); iter
				!= SharedBaseGridType::GridBaseType::locationsEnd(); ++iter) {
			GridPointHolder<T, double> *gp = iter->second;
			if (gp->inGrid && bounds.contains(gp->point)) {
				CellContents<AgentContent, double> contents(gp->point);
				provider.provideContent(gp->ptr.get(), contents._objs);
				toSend.push_back(contents);
			}
		}

		syncher.send(ngh->rank(), toSend, val);
	}
}

template<typename T, typename GPTransformer, typename Adder>
template<typename AgentContent, typename ContentProvider, typename ContentReceiver>
void SharedSpace<T, GPTransformer, Adder>::synchBuffer(SharedContext<T>& context, ContentProvider& provider,
		ContentReceiver& receiver) {

	SharedBaseGridType::buffered.clear();

	GridBufferSyncher<AgentContent, double> syncher(SharedBaseGridType::comm);

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
		std::vector<CellContents<AgentContent, double> >* contentsList = syncher.received(i);
		int rank = syncher.nghRank(i);
		for (size_t j = 0; j < contentsList->size(); ++j) {
			CellContents<AgentContent, double> contents = (*contentsList)[j];
			std::vector<T*> out;
			receiver.createAgents(contents._objs, out);
			for (typename std::vector<T*>::iterator iter = out.begin(); iter != out.end(); ++iter) {
				T* obj = *iter;
				obj->getId().currentRank(rank);
				AgentId id = obj->getId();
				/*
				 if (id == AgentId(420, 3, 1)) {
				 boost::mpi::communicator world;
				 double tick = RepastProcess::instance()->GetScheduleRunner().currentTick();
				 std::cout << "T" << tick << " : " << world.rank() << " : space buffering 420 " << id
				 << " in context: " << (context.getAgent(id) != 0) << std::endl;
				 }
				 */
				SharedBaseGridType::buffered.push_back(id);
				if (!context.addAgent(obj)) {
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
void SharedSpace<T, GPTransformer, Adder>::printLocations() {
	for (typename SharedBaseGridType::GridBaseType::LocationMapConstIter iter =
			SharedBaseGridType::GridBaseType::locationsBegin(); iter
			!= SharedBaseGridType::GridBaseType::locationsEnd(); ++iter) {
		GridPointHolder<T, double> *gp = iter->second;
	}
}

template<typename T, typename GPTransformer, typename Adder>
SharedSpace<T, GPTransformer, Adder>::~SharedSpace() {
}

template<typename T>
struct SharedGrids {

	/**
	 * Discrete grid space with periodic (toroidal) borders. Any added
	 * agents are not given a location, but are in "grid limbo" until
	 * moved via a grid move call.
	 */
	typedef SharedGrid<T, WrapAroundBorders, SimpleAdder<T> > SharedWrappedGrid;

	/**
	 * Discrete grid space with strict borders. Any added
	 * agents are not given a location, but are in "grid limbo" until
	 * moved via a grid move call.
	 */
	typedef SharedGrid<T, StrictBorders, SimpleAdder<T> > SharedStrictGrid;

	/**
	 * Continuous space with periodic (toroidal) borders. Any added
	 * agents are not given a location, but are in "grid limbo" until
	 * moved via a grid move call.
	 */
	typedef SharedSpace<T, WrapAroundBorders, SimpleAdder<T> > SharedWrappedSpace;

	/**
	 * Continuous space with strict borders. Any added
	 * agents are not given a location, but are in "grid limbo" until
	 * moved via a grid move call.
	 */
	typedef SharedSpace<T, StrictBorders, SimpleAdder<T> > SharedStrictSpace;
};

}

#endif /* SHAREDSPACE_H_ */
