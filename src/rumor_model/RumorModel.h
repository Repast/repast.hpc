/*
 * Repast for High Performance Computing (Repast HPC)
 *
 *   Copyright (c) 2010 Argonne National Laboratory
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with
 *   or without modification, are permitted provided that the following
 *   conditions are met:
 *
 *  	 Redistributions of source code must retain the above copyright notice,
 *  	 this list of conditions and the following disclaimer.
 *
 *  	 Redistributions in binary form must reproduce the above copyright notice,
 *  	 this list of conditions and the following disclaimer in the documentation
 *  	 and/or other materials provided with the distribution.
 *
 *  	 Neither the name of the Argonne National Laboratory nor the names of its
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
 * RumorModel.h
 *
 *  Created on: Jun 9, 2009
 *      Author: nick
 */

#ifndef RUMORMODEL_H_
#define RUMORMODEL_H_


// NOTE: This code includes three different ways to track and iterate through
// 'Rumored' nodes. The first is by keeping the list of pointers to rumored
// nodes in an instance of std::vector<Node*>. To use this method, define "USE_VECTOR:

#define USE_VECTOR

// The second is by keeping the nodes in an instance of std::set<Node*>. This
// is the default if USE_VECTOR is not defined.

// The third uses an instance of std::set<Node*, Node_Less>. This uses an explicit
// comparison operator that references the node's AgentId. The difference between
// std::set<Node*> and std::set<Node*, Node_Less> is that std::set<Node*> will
// order its elements according to the address to which the pointer points; this
// will never vary so long as the pointers do not change, and it will probably
// be very consistent on subsequent iterations of simulation runs, provided they
// are run in the same environment. However, it cannot be 100% reproducible. Using
// the explicit comparison orders the elements by AgentId value, which will be the
// same in every simulation run. Variations in the order of the rumored nodes in
// this collection cascade and affect the way that rumors spread; although in
// a statistical sense each variation is equally valid, the runs are not
// identical, even with the same random number seed. However, the explicit
// comparison probably carries a performance cost (this is not yet measured).
// To use the reproducible version, make sure 'Use Vector' is not defined, and
// define 'RANDOM_SEED_REPRODUCIBLE':

//#define RANDOM_SEED_REPRODUCIBLE

// Note that the std::vector<Node*> approach is fully reproducible; order is determined
// by the order in which nodes are added using push_back and insert, which will
// be consistent for every run.

#include "repast_hpc/TDataSource.h"
#include "repast_hpc/SVDataSet.h"
#include "repast_hpc/SharedContext.h"
#include "repast_hpc/Schedule.h"
#include "repast_hpc/SharedNetwork.h"
#include "repast_hpc/Edge.h"
#include "repast_hpc/Random.h"
#include "repast_hpc/Properties.h"
#include "repast_hpc/Utilities.h"

#include <boost/serialization/access.hpp>

struct NodeContent {

	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version) {
		ar & id;
		ar & proc;
		ar & type;
		ar & rumored;
	}

	int id, proc, type;
	bool rumored;

	repast::AgentId getId() const {
		return repast::AgentId(id, proc, type);
	}
};

struct EdgeContent {

	friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive& ar, const unsigned int version) {
			ar & sourceContent;
			ar & targetContent;
		}

		// source and target agents
		NodeContent sourceContent, targetContent;
};


class Node: public repast::Agent {

	friend class boost::serialization::access;

private:
	repast::AgentId id_;
	bool rumored_;
	double rProb;
	std::string distName_;

	// TODO remove this when edges use content sending
	// rather than the whole agent
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version) {
		ar & boost::serialization::base_object<Agent>(*this);
		ar & id_;
		ar & rumored_;
	}

public:
	// TODO remove no-arg ctor when edge sending uses content
	// for end points
	// no-arg constructor for serialization
	Node() {}
	Node(repast::AgentId id, double probability, std::string distName);
	virtual ~Node();

	/**
	 * Updates this Node's rumored state with that of the
	 * specified node.
	 */
	void update(Node* node);

	void init(double prob, std::string dist);

	bool rumored() const {
		return rumored_;
	}

	void rumored(bool val) {
		rumored_ = val;
	}

	/**
	 * Makes the draw against a distribtion to see
	 * if this node receives the rumor.
	 */
	bool receiveRumor();

	virtual repast::AgentId& getId() {
		return id_;
	}

	virtual const repast::AgentId& getId() const {
			return id_;
		}
};


/**
 * Used to create the set of rumored nodes, and to ensure a consistent
 * order from run to run. If set<Node*> is used, order is determined
 * by address to which pointer points; instead use set<Node*, Node_Less>,
 * which will compare by AgentId.
 */
struct Node_Less: public std::binary_function<Node*, Node*, bool>{
  bool operator()(Node* n1, Node* n2) const {
    return n1->getId() < n2->getId();
  }
};

/**
 * DataSource that sums the number of nodes that have
 * been "rumored."
 */
class RumoredSum: public repast::TDataSource<int> {

private:
	repast::SharedContext<Node>* nodes_;
	int sum;
	bool doCount;

public:
	RumoredSum(repast::SharedContext<Node>* nodes);
	int getData();
	void reset() {
		doCount = true;
	}
};

class ProviderReceiver;

class RumorModel /*: public repast::AgentUpdater*/{

private:
	friend class ProviderReceiver;

	repast::SharedContext<Node> nodes;
	repast::RepastEdgeContentManager<Node> edgeContentManager;
	repast::SharedNetwork<Node, repast::RepastEdge<Node>, repast::RepastEdgeContent<Node>, repast::RepastEdgeContentManager<Node> >* net;
	repast::SVDataSet* dataSet;
	int rank, stopAt;
	double totalNodes;
	int lastRumorSum, noChangeCount;
	repast::Properties props;
	int runNumber;

	// set of Nodes that are currently rumored
#ifndef USE_VECTOR
  #ifdef RANDOM_SEED_REPRODUCIBLE
	  std::set<Node*, Node_Less> rumored; // Note use of comparison struct; this ensures that the set is ordered by AgentId
  #else
    std::set<Node*> rumored;            // Faster (probably- untested) but cannot guarantee random number replicability
  #endif
#else
	std::vector<Node*> rumored_vector;
#endif
	RumoredSum* rumorSum;

	void buildNetwork(repast::Properties& props);
	int initializeNodes(repast::Properties& props);
	void initializeRumoredNodes(int numNodes, repast::Properties& props);

public:
	RumorModel(const std::string& propsFile, int argc, char** argv, boost::mpi::communicator* comm);
	virtual ~RumorModel();

	void initSchedule(repast::ScheduleRunner& runner);
	void spreadRumor();
	void checkForStop();
	void logRumorSum();
	void synchAgents();
	void initNode(Node* node);
	void init();
	void writeProps(std::string fileName, std::string init_time, std::string run_time);

	void provideContent(const repast::AgentRequest& request, std::vector<NodeContent>& out);
	void provideEdgeContent(const boost::shared_ptr<repast::RepastEdge<Node> > edge, std::vector<EdgeContent>& edgeContent);
	repast::RepastEdge<Node>* createEdge(repast::Context<Node>& context, EdgeContent& edge);
	void updateAgent(const NodeContent& content);

};

class NodeAdder {
private:
	RumorModel* network_;

public:
	NodeAdder(RumorModel* network);
	Node* createAgent(const NodeContent& content);
};



#endif /* RUMORMODEL_H_ */
