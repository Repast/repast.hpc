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
 *  GridMovePackets.h
 *
 *  Created on: Sep 3, 2010
 *      Author: nick
 */

#ifndef GRIDMOVEPACKETS_H_
#define GRIDMOVEPACKETS_H_

#include <map>
#include <vector>

namespace repast {


const int GRID_MOVE_SYNC_SENDERS = 3010;
const int GRID_MOVE_SYNC_PACKETS = 3011;


/**
 * Encapsulates a info about an agent moving off the grid:
 * the rank it moved to, its grid location, and the
 * agent id.
 */
template<typename PtType>
struct GridMovePacket {

	GridMovePacket() : _pt(Point<PtType>(0)) {}
	GridMovePacket(const Point<PtType>& pt, const AgentId& id, int rank) : _pt(pt), _id(id), _rank(rank) {}

	template<class Archive>
	void serialize(Archive& ar, const unsigned int version) {
		ar & _pt;
		ar & _id;
		ar & _rank;
	}

	Point<PtType> _pt;
	AgentId _id;
	int _rank;
};

template <typename PtType>
class GridMovePackets {

private:
	typedef typename std::vector<GridMovePacket<PtType> > PacketVecType;
	typedef typename std::map<int, PacketVecType*> MapType;
	MapType packets;
	std::set<int> toSendTo;

public:
	GridMovePackets() {
	}
	virtual ~GridMovePackets();
	void addPacket(const GridMovePacket<PtType>& packet);
	void clear();
	/**
	 * Removes any GridMovePacket-s associated with the specified agent id.
	 */
	void removePacketFor(const AgentId& id);

	void send(std::vector<boost::mpi::request>& requests);

	void receivers(std::vector<int>& receivers) {
		receivers.insert(receivers.begin(), toSendTo.begin(), toSendTo.end());
	}
};

template<typename PtType>
void GridMovePackets<PtType>::removePacketFor(const AgentId& id) {
	bool found = false;
	for (typename MapType::iterator iter = packets.begin(); iter != packets.end(); ++iter) {
		std::vector<GridMovePacket<PtType> >* vec = iter->second;
		for (typename PacketVecType::iterator vecIter = vec->begin(); vecIter != vec->end(); ++vecIter) {
			if ((*vecIter)._id == id) {
				//std::cout << "removing packet for " << id << std::endl;
				//throw std::domain_error("foo");
				vec->erase(vecIter);
				found = true;
				break;
			}
		}
		if (found) break;
	}
}

template<typename PtType>
void GridMovePackets<PtType>::addPacket(const GridMovePacket<PtType>& packet) {
	int rank = packet._rank;
	toSendTo.insert(rank);
	typename MapType::iterator iter = packets.find(rank);
	if (iter == packets.end()) {
		std::vector<GridMovePacket<PtType> >* vec = new std::vector<GridMovePacket<PtType> > ();
		vec->push_back(packet);
		packets[rank] = vec;
		//std::cout << "adding packet for " << packet._id << std::endl;
	} else {
		iter->second->push_back(packet);
	}
}

template<typename PtType>
void GridMovePackets<PtType>::clear() {
	for (typename MapType::iterator iter = packets.begin(); iter != packets.end(); ++iter) {
		delete iter->second;
	}
	packets.clear();
	toSendTo.clear();
}

template<typename PtType>
void GridMovePackets<PtType>::send(std::vector<boost::mpi::request>& requests) {
	boost::mpi::communicator world;
	for (typename MapType::iterator iter = packets.begin(); iter != packets.end(); ++iter) {
		requests.push_back(world.isend(iter->first, GRID_MOVE_SYNC_PACKETS, *(iter->second)));
	}
}

template<typename PtType>
GridMovePackets<PtType>::~GridMovePackets() {
	for (typename MapType::iterator iter = packets.begin(); iter != packets.end(); ++iter) {
		delete iter->second;
	}
}


}

#endif /* GRIDMOVEPACKETS_H_ */
