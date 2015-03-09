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
 * RumorModel.cpp
 *
 */

#include "RumorModel.h"
#include "repast_hpc/io.h"
#include "repast_hpc/logger.h"
#include "repast_hpc/initialize_random.h"
#include "repast_hpc/Utilities.h"
#include "repast_hpc/NetworkBuilder.h"
#include "repast_hpc/SVDataSetBuilder.h"

#include <boost/lexical_cast.hpp>
#include <boost/mpi.hpp>
#include <boost/mpi/collectives.hpp>

#include <sstream>
#include <cstdlib>
#include <algorithm>
#include <stdexcept>
#include <math.h>
#include <vector>

#define DEBUG_RANK 6

using namespace std;
using namespace repast;

const string NODE_RUMORED_DIST = "node.rumored.distribution";
const string NODE_RUMORED_PROB = "node.rumored.probability";
const string INIT_NODE_COMP = "initial.nodes";
const string NODE_COUNT = "node.count";
const string STOP_AT = "stop.at";
const string SHARED_NODES_N = "shared.nodes.n";
const string RUN_NUMBER = "run.number";

BOOST_CLASS_EXPORT_GUID(repast::SpecializedProjectionInfoPacket<repast::RepastEdgeContent<Node> >, "SpecializedEdgeContent");

Node::Node(AgentId id, double probability, string distName) :
	id_(id), rumored_(false), rProb(probability), distName_(distName) {
}

Node::~Node() {
}

void Node::init(double prob, string dist) {
	rProb = prob;
	distName_ = dist;
}

bool Node::receiveRumor() {
	if (!rumored_) {
		rumored_ = Random::instance()->getGenerator(distName_)->next() <= rProb;
	}
	return rumored_;
}

void Node::update(Node *node) {
	rumored_ = node->rumored_;
}

RumorModel::RumorModel(const string& propsFile, int argc, char** argv, boost::mpi::communicator* comm) :
	lastRumorSum(0), nodes(comm), noChangeCount(0), props(propsFile, argc, argv, comm) {

	RepastProcess *rp = RepastProcess::instance();
	rank = rp->rank();
	stringstream ss;
	ss << rp->worldSize();
	props.putProperty("process.count", ss.str());
	
	initializeRandom(props, comm);
	stopAt = strToDouble(props.getProperty(STOP_AT));
	runNumber = (props.contains(RUN_NUMBER) ? strToInt(props.getProperty(RUN_NUMBER)) : -1);

	net = new SharedNetwork<Node, RepastEdge<Node>, RepastEdgeContent<Node>, RepastEdgeContentManager<Node> > ("network", true, &edgeContentManager);
	nodes.addProjection(net);

	std::string fileOutputName("./output/rumor_model_data" + (runNumber >= 0 ? "_RUN_" + boost::lexical_cast<string>(runNumber) : "") + ".csv");
  SVDataSetBuilder builder(fileOutputName.c_str(), ",", RepastProcess::instance()->getScheduleRunner().schedule());

  rumorSum = new RumoredSum(&nodes);
	builder.addDataSource(createSVDataSource("number_rumored", rumorSum, std::plus<int>()));
	dataSet = builder.createDataSet();

}

void RumorModel::init() {
	int numNodes = initializeNodes(props);
#ifdef USE_VECTOR
  rumored_vector.reserve(numNodes/2); // This gives the vector some space, avoiding resizes during the run
#endif


	buildNetwork(props);
	NodeAdder adder(this);
//	repast::createComplementaryEdges<Node, RepastEdge<Node> , NodeContent, EdgeContent, RumorModel, NodeAdder>(net,
//			nodes, *this, adder);

	repast::RepastProcess::instance()->synchronizeProjectionInfo<Node, NodeContent, RumorModel, RumorModel, NodeAdder >
	   (nodes, *this, *this, adder);

  // Count non-local nodes (proc 0 only) for output
  double nlc = 0;
  repast::SharedContext<Node>::const_iterator iter    = nodes.begin();
  repast::SharedContext<Node>::const_iterator iterEnd = nodes.end();
  while(iter != iterEnd){
    if((*iter)->getId().currentRank() != rank) nlc++;
    iter++;
  }
  double sum;
  MPI_Allreduce(&nlc, &sum, 1, MPI_DOUBLE, MPI_SUM, *(repast::RepastProcess::instance()->getCommunicator()));


  stringstream ss;
  ss << (sum / (repast::RepastProcess::instance()->worldSize()));
  props.putProperty("avg-non_local.node.count", ss.str());


	initializeRumoredNodes(numNodes, props);
}

void RumorModel::initNode(Node* node) {
	double prob = strToDouble(props.getProperty(NODE_RUMORED_PROB));
	string dist = props.getProperty(NODE_RUMORED_DIST);
	node->init(prob, dist);
}

void RumorModel::initializeRumoredNodes(int numNodes, Properties& props) {
	vector<int> counts;
	if (rank == 0) {
		int initCount;
		try {
			initCount = strToInt(props.getProperty(INIT_NODE_COMP));
		} catch (invalid_argument& ex) {
			// it was a string, so assume its a distribution
			initCount = (int) Random::instance()->getGenerator(props.getProperty(INIT_NODE_COMP))->next();
		}
		int worldSize = RepastProcess::instance()->worldSize();
		counts.assign(worldSize, 0);
		IntUniformGenerator rnd = Random::instance()->createUniIntGenerator(0, worldSize - 1);
		for (int i = 0; i < initCount; i++) {
			int rank = rnd.next();
			counts[rank] = ++counts[rank];
		}
	}

	int count = 0;
	boost::mpi::communicator* comm = repast::RepastProcess::instance()->getCommunicator();
	boost::mpi::scatter(*comm, counts, count, 0);

	if (count > 0) {
		IntUniformGenerator rnd = Random::instance()->createUniIntGenerator(0, numNodes - 1);
		// mark that many random local agents as rumored, making sure
		// that the agents have local neighbors
		for (int i = 0; i < count; i++) {
			int localId = rnd.next();
			AgentId id(localId, rank, 0);
			Node* node = nodes.getAgent(id);
			if (node->rumored()) {
				// do the draw again
				--i;
			} else {
				vector<Node*> out;
				net->successors(node, out);
				bool ok = false;
				for (int j = 0, k = out.size(); j < k; j++) {
					if (out[j]->getId().currentRank() == rank) {
						ok = true;
						break;
					}
				}
				if (ok) {
					node->rumored(true);
#ifndef USE_VECTOR
					rumored.insert(node);
#else
					rumored_vector.push_back(node);
#endif
				} else {
					--i;
				}
			}
		}
	}
}

// sets the number of nodes to initialize on each process
// and initializes those nodes
int RumorModel::initializeNodes(Properties& props) {
	vector<int> nodeCounts;
	Logger& logger = Log4CL::instance()->get_logger("root");
	int worldSize = RepastProcess::instance()->worldSize();

	if (rank == 0) {
		totalNodes = strToDouble(props.getProperty(NODE_COUNT));
		logger.log(DEBUG, "Total number of nodes: " + boost::lexical_cast<string>(totalNodes));
		double nodesPerP = totalNodes / worldSize;
		if (nodesPerP < 1) {
			logger.log(ERROR, "total number of nodes is less than process count.");
			return -1;
		}
		nodeCounts.assign(worldSize, nodesPerP);
		int diff = totalNodes - nodesPerP * worldSize;
		for (int i = 0; i < diff; i++) {
			nodeCounts[i] = nodeCounts[i] + 1;
		}
	}

	int numNodes = 0;
	boost::mpi::communicator* comm = repast::RepastProcess::instance()->getCommunicator();
	boost::mpi::scatter(*comm, nodeCounts, numNodes, 0);
	double prob = strToDouble(props.getProperty(NODE_RUMORED_PROB));
	string dist = props.getProperty(NODE_RUMORED_DIST);

	logger.log(DEBUG, "Creating " + boost::lexical_cast<string>(numNodes) + " network nodes\n\t using " + dist
			+ " with probability of " + props.getProperty(NODE_RUMORED_PROB));
	for (int i = 0; i < numNodes; i++) {
		AgentId id(i, rank, 0);
		nodes.addAgent(new Node(id, prob, dist));
	}
	return numNodes;
}

void RumorModel::buildNetwork(Properties& props) {
  int processCount = RepastProcess::instance()->worldSize();
	vector<int> nodeCounts;
	boost::mpi::communicator* comm = repast::RepastProcess::instance()->getCommunicator();

	all_gather(*comm, nodes.size(), nodeCounts);
	// get agents from 4 "neighboring" processes
	vector<int> others;
	if (processCount < 5) {
		for (int i = 0; i < processCount; i++) {
			if (i != rank)
				others.push_back(i);
		}
	} else {
		for (int i = -2; i < 3; i++) {
			if (i != 0) {
				int other = rank + i;
				if (other < 0)
					other = processCount + i;
				else if (other >= processCount)
					other = other - processCount;
				others.push_back(other);
			}
		}
	}

	int amtToShare = strToInt(props.getProperty(SHARED_NODES_N));
	AgentRequest request(rank);
	for (int i = 0, n = others.size(); i < n; i++) {
		int other = others[i];

		// we use the range [0, nodeCount - 1] to grab ids that will be on other processes.
		IntUniformGenerator gen = Random::instance()->createUniIntGenerator(0, nodeCounts[other] - 1);
		set<int> requestedIds;
		// get random agents from other process
		for (int i = 0; i < amtToShare; i++) {
			int otherId = gen.next();
			while (requestedIds.find(otherId) != requestedIds.end()) {
				otherId = gen.next();
			}
			requestedIds.insert(otherId);
			request.addRequest(AgentId(otherId, other, 0));
		}
	}

	NodeAdder adder(this);
	repast::RepastProcess::instance()->requestAgents<Node, NodeContent, RumorModel, RumorModel, NodeAdder>(nodes, request, *this, *this, adder);
	KEBuilder<Node, RepastEdge<Node>, RepastEdgeContent<Node>, RepastEdgeContentManager<Node> > builder;
	builder.build(props, net);

}

void RumorModel::initSchedule(ScheduleRunner& runner) {
	runner.scheduleStop(stopAt);
	runner.scheduleEvent(1, 1, Schedule::FunctorPtr(new MethodFunctor<RumorModel> (this, &RumorModel::spreadRumor)));
	runner.scheduleEvent(1.05, 1, Schedule::FunctorPtr(new MethodFunctor<RumorModel> (this, &RumorModel::synchAgents)));

	runner.scheduleEvent(1.1, 1, Schedule::FunctorPtr(new MethodFunctor<DataSet> (dataSet, &DataSet::record)));
	Schedule::FunctorPtr dsWrite = Schedule::FunctorPtr(new MethodFunctor<DataSet> (dataSet, &DataSet::write));
	runner.scheduleEvent(25.2, 25, dsWrite);
	runner.scheduleEndEvent(dsWrite);

	//runner.ScheduleEvent(1.15, 1, new MethodFunctor<RumorModel> (this, &RumorModel::checkForStop));

	runner.scheduleEndEvent(Schedule::FunctorPtr(new MethodFunctor<RumorModel> (this, &RumorModel::logRumorSum)));
	runner.scheduleEndEvent(Schedule::FunctorPtr(new MethodFunctor<DataSet> (dataSet, &DataSet::close)));
}

void RumorModel::logRumorSum() {
	int rumorCount = rumorSum->getData();
	boost::mpi::communicator* comm = repast::RepastProcess::instance()->getCommunicator();
	if (rank == 0) {
		int sum;
		reduce(*comm, rumorCount, sum, std::plus<int>(), 0);
		Log4CL::instance()->get_logger("root").log(INFO, "Total rumored nodes: " + boost::lexical_cast<string>(sum));
  	props.log("root");

	} else {
		reduce(*comm, rumorCount, std::plus<int>(), 0);
	}
}

void RumorModel::checkForStop() {
	// check if >= half of total nodes have been compromised
	int rumorCount = rumorSum->getData();
	double stopTick = 0;
	boost::mpi::communicator* comm = repast::RepastProcess::communicator();
	if (rank == 0) {
		int sum;
		reduce(*comm, rumorCount, sum, std::plus<int>(), 0);
		if (sum == lastRumorSum)
			noChangeCount++;
		else {
			lastRumorSum = sum;
			noChangeCount = 0;
		}
		if (sum >= totalNodes / 2.0) {
			stopTick = RepastProcess::instance()->getScheduleRunner().currentTick() + .001;
			Log4CL::instance()->get_logger("root").log(INFO, "Half nodes rumored. Stopping at: " + boost::lexical_cast<
					string>(stopTick));
		} else if (noChangeCount > 10) {
			stopTick = RepastProcess::instance()->getScheduleRunner().currentTick() + .001;
			Log4CL::instance()->get_logger("root").log(INFO, "No Change for last 10 ticks. Stopping at: "
					+ boost::lexical_cast<string>(stopTick));
		}
	} else {
		reduce(*comm, rumorCount, std::plus<int>(), 0);
	}
	broadcast(*comm, stopTick, 0);
	if (stopTick > 0) {
		RepastProcess::instance()->getScheduleRunner().scheduleStop(stopTick);
	}
}

RumorModel::~RumorModel() {
	// network is deleted by the context
	// rumorSum is deleted by the dataset
	delete dataSet;
}

void RumorModel::provideContent(const AgentRequest& request, vector<NodeContent>& out) {
	const vector<AgentId>& ids = request.requestedAgents();
	for (int i = 0, n = ids.size(); i < n; i++) {
		AgentId id = ids[i];
		Node* node = nodes.getAgent(id);
		NodeContent content = { id.id(), id.startingRank(), id.agentType(), node->rumored() };
		out.push_back(content);
	}
}

void RumorModel::provideEdgeContent(const boost::shared_ptr<repast::RepastEdge<Node> > edge, std::vector<EdgeContent>& edgeContent) {
	Node* source = edge->source();
	Node* target = edge->target();
	NodeContent sContent = { source->getId().id(), source->getId().startingRank(), source->getId().agentType(),
			source->rumored() };
	NodeContent tContent = { target->getId().id(), target->getId().startingRank(), target->getId().agentType(),
			target->rumored() };
	EdgeContent content = { sContent, tContent };
	edgeContent.push_back(content);
}

RepastEdge<Node>* RumorModel::createEdge(repast::Context<Node>& context, EdgeContent& edge) {
	AgentId sourceId = edge.sourceContent.getId();
	AgentId targetId = edge.targetContent.getId();
	// if there was more to the content we would add it here.
	return new RepastEdge<Node> (context.getAgent(sourceId), context.getAgent(targetId));
}

void RumorModel::synchAgents() {
	repast::RepastProcess::instance()->synchronizeAgentStates<NodeContent, RumorModel, RumorModel>(*this, *this);
  NodeAdder na(this);
  repast::RepastProcess::instance()->synchronizeAgentStatus<Node, NodeContent, RumorModel, NodeAdder, RumorModel>(nodes, *this, *this, na);
}

void RumorModel::spreadRumor() {
  int r = repast::RepastProcess::instance()->rank();
  double t = repast::RepastProcess::instance()->getScheduleRunner().currentTick();
  if(r == 0) Log4CL::instance()->get_logger("root").log(INFO, "Tick: " + boost::lexical_cast<string>(t));
	rumorSum->reset();
	// iterate through currently compromised and try to compromise neighbors
	std::vector<Node*> tmp;
#ifndef USE_VECTOR
	for (set<Node*>::iterator iter = rumored.begin(); iter != rumored.end(); ++iter) {
#else
	 for(std::vector<Node*>::iterator iter = rumored_vector.begin(), iterEnd = rumored_vector.end(); iter != iterEnd; iter++){
#endif
    vector<Node*> out;
		net->successors(*iter, out);
		for (int j = 0, k = out.size(); j < k; j++) {
			Node* node = out[j];
			if (!node->rumored() && node->getId().currentRank() == rank) {
				if (node->receiveRumor()) {
					tmp.push_back(node);
				}
			}
		}
	}
#ifndef USE_VECTOR
	rumored.insert(tmp.begin(), tmp.end());
#else
	rumored_vector.insert(rumored_vector.end(), tmp.begin(), tmp.end());
#endif
}

/**
 * Updates the state of an existing agent with the content.
 */
void RumorModel::updateAgent(const NodeContent& content) {
  int r = repast::RepastProcess::instance()->rank();
  double t = repast::RepastProcess::instance()->getScheduleRunner().currentTick();
	AgentId id = content.getId();
	Node* copy = nodes.getAgent(id);
	if (!copy->rumored() && content.rumored) {
#ifndef USE_VECTOR
	  rumored.insert(copy);
#else
		rumored_vector.push_back(copy);
#endif
		copy->rumored(content.rumored);
	}
}

// NodeAdder implementation
NodeAdder::NodeAdder(RumorModel* model) :
	network_(model) {
}

Node* NodeAdder::createAgent(const NodeContent& content) {
  int r = repast::RepastProcess::instance()->rank();
	AgentId id(content.id, content.proc, content.type);
	Node* node = new Node(id, 0, "");
	node->rumored(content.rumored);
	network_->initNode(node);
	return node;
}

// HackedSum DataSource
RumoredSum::RumoredSum(SharedContext<Node>* nodes) :
	nodes_(nodes), sum(0), doCount(true) {
}

int RumoredSum::getData() {
	if (doCount) {
		sum = 0;
		for (SharedContext<Node>::const_local_iterator iter = nodes_->localBegin(); iter != nodes_->localEnd(); ++iter) {
			Node *node = iter->get();
			if (node->rumored())
				sum++;
		}
		doCount = false;
	}
	return sum;
}


void RumorModel::writeProps(std::string fileName, std::string init_time, std::string run_time){
  props.putProperty("init.time", init_time);
  props.putProperty("run.time", run_time);
  std::string time;
  repast::timestamp(time);
  props.putProperty("date_time.run", time);

  std::vector<std::string> keysToWrite;
  keysToWrite.push_back("run.number");
  keysToWrite.push_back("date_time.run");
  keysToWrite.push_back("process.count");
  keysToWrite.push_back("random.seed");
  keysToWrite.push_back("node.count");
  keysToWrite.push_back("ke.model.m");
  keysToWrite.push_back("shared.nodes.n");
  keysToWrite.push_back("initial.nodes");
  keysToWrite.push_back("avg.non_local.node.count");
  keysToWrite.push_back("stop.at");
  keysToWrite.push_back("init.time");
  keysToWrite.push_back("run.time");

  props.writeToSVFile(fileName, keysToWrite);

}
