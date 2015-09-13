/*
*Repast for High Performance Computing (Repast HPC)
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
*/

#include <gtest/gtest.h>
#include <boost/mpi.hpp>

#include "repast_hpc/RepastProcess.h"
#include "repast_hpc/SVDataSetBuilder.h"
#include "repast_hpc/TDataSource.h"
#include "model.h"

#include <set>
#include <vector>

namespace mpi = boost::mpi;
using namespace std;
using namespace repast;

class RSMPITests: public ::testing::Test  {

protected:
	Model* model;
	int rank;

public:
	RSMPITests() {
		repast::RepastProcess::init("./config.props");
		model = new Model();
		rank = RepastProcess::instance()->rank();
	}

	virtual ~RSMPITests() {
		delete model;
		delete RepastProcess::instance();
	}
};

TEST_F(RSMPITests, Agents)
{
	for (int i = 0; i < 4; i++) {
		AgentId id(i, rank, 0);
		Agent* agent = model->agents.getAgent(id);
		ASSERT_NE((void*)0, agent);
	}
}

void requestAgents(int rank, Model* model) {
	AgentRequest request(rank);
	if (rank == 1) {
		// P1 request (3, 2, 0) from P2
		AgentId id(3, 2, 0);
		request.addRequest(id);
		id = AgentId(3, 3, 0);
		request.addRequest(id);
	} else if (rank == 3) {
		AgentId id(1, 1, 0);
		request.addRequest(id);
	}

	AgentCreatorUpdater creator(model);
	repast::requestAgents<ModelAgent, ModelAgentContent>(model->agents, request, *model, creator);
}

TEST_F(RSMPITests, RequestAgents)
{
	::requestAgents(rank, model);
	if (rank == 1) {
		// P1 and P3 request (3, 2, 0) from P2
		AgentId id(3, 2, 0);
		ASSERT_TRUE(model->agents.contains(id));
		ASSERT_TRUE(model->agents.contains(AgentId(3, 3, 0)));
	} else if (rank == 3) {
		AgentId id(1, 1, 0);
		ASSERT_TRUE(model->agents.contains(id));
	}

	if (rank == 2) {
		ModelAgent* agent = model->agents.getAgent(AgentId(3, 2, 0));
		agent->state(200);
	}

	AgentCreatorUpdater creator(model);
	syncAgents<ModelAgentContent> (*model, creator);

	if (rank == 1) {
		ModelAgent* agent = model->agents.getAgent(AgentId(3, 2, 0));
		ASSERT_EQ(200, agent->state());

	}
}


void fillGrid(int rank, Model* model) {
	// total grid 40 x 60, each P (out of 4) is responsible
	// for a 20 x 30 chunk
	GridDimensions bounds = model->grid->bounds();
	int x = bounds.origin(0) + bounds.extents(0) / 2;
	int y = bounds.origin(1) + bounds.extents(1) / 2;
	//std::cout << rank << ": " << x << ", " << y << std::endl;
	std::vector<int> pt;
	pt.push_back(x);
	pt.push_back(y);
	Grid<ModelAgent, int>* grid = static_cast<Grid<ModelAgent, int>*> (model->grid);
	for (int i = 0; i < 4; i++) {
		grid->moveTo(AgentId(i, rank, 0), pt);
	}
}

TEST_F(RSMPITests, Grid)
{
	fillGrid(rank, model);
	GridDimensions bounds = model->grid->bounds();
	int x = bounds.origin(0) + bounds.extents(0) / 2;
	int y = bounds.origin(1) + bounds.extents(1) / 2;
	std::vector<ModelAgent*> out;
	model->grid->getObjectsAt(Point<int> (x, y), out);
	ASSERT_EQ(4, model->grid->size());
	ASSERT_EQ(4, out.size());
}

TEST_F(RSMPITests, GridBufferSync)
{
	fillGrid(rank, model);
	if (rank == 0) {
		model->grid->moveTo(AgentId(0, 0, 0), Point<int> (-1, -30).coords());
	}
	model->grid->synchBuffer<ModelAgentContent> (model->agents, *model, *model);

	if (rank == 2 || rank == 1 || rank == 3) {
		Point<int> pt(-1, -30);
		std::vector<ModelAgent*> out;
		model->grid->getObjectsAt(pt, out);
		ASSERT_EQ(1, out.size());
		ModelAgent* agent = model->grid->getObjectAt(pt);
		ASSERT_NE((void*)0, agent);
	}

	if (rank == 1) {
		model->grid->moveTo(AgentId(1, 1, 0), Point<int> (1, -15));
	}
	model->grid->synchBuffer<ModelAgentContent> (model->agents, *model, *model);

	if (rank == 0) {
		Point<int> pt(1, -15);
		std::vector<ModelAgent*> out;
		model->grid->getObjectsAt(pt, out);
		ASSERT_EQ(1, out.size());
		ModelAgent* agent = model->grid->getObjectAt(pt);
		ASSERT_NE((void*)0, agent);
		ASSERT_EQ(agent->getId(), AgentId(1, 1, 0));
	}

	if (rank == 2 || rank == 3) {
		Point<int> pt(1, -15);
		std::vector<ModelAgent*> out;
		model->grid->getObjectsAt(pt, out);
		ASSERT_EQ(0, out.size());
	}
}

void addEdges(int rank, Model* model) {
	AgentId source(1, 1, 0);
	AgentId target1(3, 2, 0);
	AgentId target2(3, 3, 0);
	if (rank == 1) {
		model->net->addEdge(model->agents.getAgent(source), model->agents.getAgent(target1));
	} else if (rank == 3) {
		// create edge between 3, 3, 0 and 1, 1, 0
		model->net->addEdge(model->agents.getAgent(source), model->agents.getAgent(target2));
	}
}

TEST_F(RSMPITests, CompEdges)
{
	AgentId source(1, 1, 0);
	AgentId target1(3, 2, 0);
	AgentId target2(3, 3, 0);

	::requestAgents(rank, model);
	addEdges(rank, model);

	AgentCreatorUpdater adder(model);
	createComplementaryEdges<ModelAgent, ModelEdge, ModelAgentContent, EdgeContent, Model, AgentCreatorUpdater> (
			model->net, model->agents, *model, adder);

	int expected[] = { 0, 2, 1, 1 };
	ASSERT_EQ(expected[rank], model->net->edgeCount());

	if (rank == 1) {
		ASSERT_NE((void*)0, model->net->findEdge(model->agents.getAgent(source), model->agents.getAgent(target1)));
		ASSERT_NE((void*)0, model->net->findEdge(model->agents.getAgent(source), model->agents.getAgent(target2)));

	} else if (rank == 2) {
		ASSERT_NE((void*)0, model->net->findEdge(model->agents.getAgent(source), model->agents.getAgent(target1)));
	} else if (rank == 3) {
		ASSERT_NE((void*)0, model->net->findEdge(model->agents.getAgent(source), model->agents.getAgent(target2)));
	}

	if (rank == 3) {
		//
		model->net->removeEdge(model->agents.getAgent(source), model->agents.getAgent(target2));
	}

	model->net->synchRemovedEdges();

	// make sure the shared edge is no longer exists
	if (rank == 1) {
		ASSERT_EQ((void*)0, model->net->findEdge(model->agents.getAgent(source), model->agents.getAgent(target2)));
	} else if (rank == 3) {
		ASSERT_EQ((void*)0, model->net->findEdge(model->agents.getAgent(source), model->agents.getAgent(target2)));
	}

	// synch edges just to make sure that the synch remove doesn't cause problems
	// with this
	synchEdges<ModelAgent, ModelEdge, EdgeContent, Model> (model->net, *model);

}

TEST_F(RSMPITests, SyncEdges)
{
	::requestAgents(rank, model);
	addEdges(rank, model);
	AgentCreatorUpdater adder(model);
	createComplementaryEdges<ModelAgent, ModelEdge, ModelAgentContent, EdgeContent, Model, AgentCreatorUpdater> (
			model->net, model->agents, *model, adder);

	AgentId source(1, 1, 0);
	AgentId target(3, 2, 0);
	AgentId target2(3, 3, 0);

	if (rank == 1) {
		ModelEdge* edge = model->net->findEdge(model->agents.getAgent(source), model->agents.getAgent(target));
		ASSERT_NE((void*)0, edge);
		edge->state(12);
	}
	synchEdges<ModelAgent, ModelEdge, EdgeContent, Model> (model->net, *model);

	if (rank == 2) {
		ModelEdge* edge = model->net->findEdge(model->agents.getAgent(source), model->agents.getAgent(target));
		ASSERT_NE((void*)0, edge);
		ASSERT_EQ(12, edge->state());
	} else if (rank == 3) {
		// state for  this edge would be updated but wouln't have changed
		ModelEdge* edge = model->net->findEdge(model->agents.getAgent(source), model->agents.getAgent(target2));
		ASSERT_NE((void*)0, edge);
		ASSERT_EQ(1, edge->state());
	}

}

TEST_F(RSMPITests, DeleteAgentSyncEdge)
{
	::requestAgents(rank, model);
	addEdges(rank, model);
	AgentCreatorUpdater adder(model);
	createComplementaryEdges<ModelAgent, ModelEdge, ModelAgentContent, EdgeContent, Model, AgentCreatorUpdater> (
			model->net, model->agents, *model, adder);

	AgentId source(1, 1, 0);
	AgentId target(3, 2, 0);
	AgentId target2(3, 3, 0);

	if (rank == 1) {
		ModelEdge* edge = model->net->findEdge(model->agents.getAgent(source), model->agents.getAgent(target));
		ASSERT_NE((void*)0, edge);
		edge->state(12);
	}
	synchEdges<ModelAgent, ModelEdge, EdgeContent, Model> (model->net, *model);

	if (rank == 2) {
		ModelEdge* edge = model->net->findEdge(model->agents.getAgent(source), model->agents.getAgent(target));
		ASSERT_NE((void*)0, edge);
		ASSERT_EQ(12, edge->state());
	} else if (rank == 3) {
		// state for  this edge would be updated but wouln't have changed
		ModelEdge* edge = model->net->findEdge(model->agents.getAgent(source), model->agents.getAgent(target2));
		ASSERT_NE((void*)0, edge);
		ASSERT_EQ(1, edge->state());
	}

	if (rank == 1) {
		model->agents.removeAgent(model->agents.getAgent(source));
	}
	AgentCreatorUpdater creator(model);
	RepastProcess::instance()->syncAgentStatus<ModelAgent, ModelAgentContent>(model->agents,
			*model, creator);

	ASSERT_EQ((void*)0, model->agents.getAgent(source));

	std::vector<ModelAgent*> out;
	// agent 1,1,0 should be eliminated from 1 and 3, and no edges
	if (rank == 1 || rank == 3) {
		// target2 is still there but no longer has an edge, the edge was exported from 3 to 1
		model->net->adjacent(model->agents.getAgent(target2), out);
		if (out.size() > 0) {
			std::cout << out[0]->getId() << std::endl;
		}
		ASSERT_EQ(0, out.size());
	}

	model->net->synchRemovedEdges();
	synchEdges<ModelAgent, ModelEdge, EdgeContent, Model> (model->net, *model);

}

void fillSpace(int rank, Model* model) {

	// total space 40 x 60, each P (out of 4) is responsible
	// for a 20 x 30 chunk
	GridDimensions bounds = model->space->bounds();
	double x = bounds.origin(0) + bounds.extents(0) / 2 + .4;
	double y = bounds.origin(1) + bounds.extents(1) / 2 + .4;
	std::vector<double> pt;
	pt.push_back(x);
	pt.push_back(y);
	for (int i = 0; i < 4; i++) {
		ASSERT_TRUE(model->space->moveTo(AgentId(i, rank, 0), pt));
	}

}

TEST_F(RSMPITests, Space)
{
	fillSpace(rank, model);
	GridDimensions bounds = model->space->bounds();
	double x = bounds.origin(0) + bounds.extents(0) / 2 + .4;
	double y = bounds.origin(1) + bounds.extents(1) / 2 + .4;
	std::vector<ModelAgent*> out;
	model->space->getObjectsAt(Point<double> (x, y), out);
	ASSERT_EQ(4, model->space->size());
	ASSERT_EQ(4, out.size());
}

TEST_F(RSMPITests, SpaceBuffer)
{
	fillSpace(rank, model);
	if (rank == 0) {
		model->space->moveTo(AgentId(0, 0, 0), Point<double> (18.0, 0).coords());
	}
	model->space->synchBuffer<ModelAgentContent> (model->agents, *model, *model);

	if (rank == 2 || rank == 1 || rank == 3) {
		Point<double> pt(18, 0);
		std::vector<ModelAgent*> out;
		model->space->getObjectsAt(pt, out);
		ASSERT_EQ(1, out.size());
		ModelAgent* agent = model->space->getObjectAt(pt);
		ASSERT_NE((void*)0, agent);
	}

	if (rank == 1) {
		model->space->moveTo(AgentId(1, 1, 0), Point<double> (21, 15));
	}
	model->space->synchBuffer<ModelAgentContent> (model->agents, *model, *model);

	if (rank == 0) {
		Point<double> pt(21, 15);
		std::vector<ModelAgent*> out;
		model->space->getObjectsAt(pt, out);
		ASSERT_EQ(1, out.size());
		ModelAgent* agent = model->space->getObjectAt(pt);
		ASSERT_NE((void*)0, agent);
		ASSERT_EQ(agent->getId(), AgentId(1, 1, 0));
	}

	if (rank == 2 || rank == 3) {
		Point<double> pt(21, 15);
		std::vector<ModelAgent*> out;
		model->space->getObjectsAt(pt, out);
		ASSERT_EQ(0, out.size());
	}
}

TEST_F(RSMPITests, SyncGridAndSpaceBuffers)
{
	fillGrid(rank, model);
	fillSpace(rank, model);

	if (rank == 0) {
		model->space->moveTo(AgentId(0, 0, 0), Point<double> (18.0, 0).coords());
		model->grid->moveTo(AgentId(0, 0, 0), Point<int> (-1, -30).coords());
	}

	model->grid->synchBuffer<ModelAgentContent> (model->agents, *model, *model);
	model->space->synchBuffer<ModelAgentContent> (model->agents, *model, *model);

	if (rank == 2 || rank == 1 || rank == 3) {
		Point<double> pt(18, 0);
		std::vector<ModelAgent*> out;
		model->space->getObjectsAt(pt, out);
		ASSERT_EQ(1, out.size());
		ModelAgent* agent = model->space->getObjectAt(pt);
		ASSERT_NE((void*)0, agent);

		Point<int> ipt(-1, -30);
		out.clear();
		model->grid->getObjectsAt(ipt, out);
		ASSERT_EQ(1, out.size());
		agent = model->grid->getObjectAt(ipt);
		ASSERT_NE((void*)0, agent);
	}

	if (rank == 1) {
		model->space->moveTo(AgentId(1, 1, 0), Point<double> (21, 15));
		model->grid->moveTo(AgentId(1, 1, 0), Point<int> (1, -15));
	}

	model->space->synchBuffer<ModelAgentContent> (model->agents, *model, *model);
	model->grid->synchBuffer<ModelAgentContent> (model->agents, *model, *model);

	if (rank == 0) {
		Point<double> pt(21, 15);
		std::vector<ModelAgent*> out;
		model->space->getObjectsAt(pt, out);
		ASSERT_EQ(1, out.size());
		ModelAgent* agent = model->space->getObjectAt(pt);
		ASSERT_NE((void*)0, agent);
		ASSERT_EQ(agent->getId(), AgentId(1, 1, 0));

		Point<int> ipt(1, -15);
		out.clear();
		model->grid->getObjectsAt(ipt, out);
		ASSERT_EQ(1, out.size());
		agent = model->grid->getObjectAt(ipt);
		ASSERT_NE((void*)0, agent);
		ASSERT_EQ(agent->getId(), AgentId(1, 1, 0));
	}

	if (rank == 2 || rank == 3) {
		Point<double> pt(21, 15);
		std::vector<ModelAgent*> out;
		model->space->getObjectsAt(pt, out);
		ASSERT_EQ(0, out.size());

		Point<int> ipt(1, -15);
		out.clear();
		model->grid->getObjectsAt(ipt, out);
		ASSERT_EQ(0, out.size());
	}
}

TEST_F(RSMPITests, AgentRemoved)
{
	fillGrid(rank, model);
	AgentRequest request(rank);
	if (rank == 1 || rank == 2) {
		// P1 and 2 request (0, 0, 0) from P0
		AgentId id(0, 0, 0);
		request.addRequest(id);
		id = AgentId(1, 0, 0);
		request.addRequest(id);
	}

	AgentCreatorUpdater creator(model);
	repast::requestAgents<ModelAgent, ModelAgentContent>(model->agents, request, *model, creator);
	GridDimensions bounds = model->grid->bounds();
	int x = bounds.origin(0) + bounds.extents(0) / 2;
	int y = bounds.origin(1) + bounds.extents(1) / 2;

	if (rank == 1 || rank == 2) {
		ModelAgent* agent = model->agents.getAgent(AgentId(0, 0, 0));
		ASSERT_NE((void*)0, agent);
		model->grid->moveTo(agent->getId(), Point<int>(x, y));

		agent = model->agents.getAgent(AgentId(1, 0, 0));
		ASSERT_NE((void*)0, agent);
		model->net->addEdge(model->agents.getAgent(AgentId(0, 0, 0)), model->agents.getAgent(AgentId(0, rank, 0)));
	}

	AgentCreatorUpdater adder(model);
	createComplementaryEdges<ModelAgent, ModelEdge, ModelAgentContent, EdgeContent, Model, AgentCreatorUpdater> (
			model->net, model->agents, *model, adder);

	if (rank == 0) {
		ASSERT_NE((void*)0, model->net->findEdge(model->agents.getAgent(AgentId(0, 0, 0)), model->agents.getAgent(AgentId(0, 1, 0))));
		ASSERT_NE((void*)0, model->net->findEdge(model->agents.getAgent(AgentId(0, 0, 0)), model->agents.getAgent(AgentId(0, 2, 0))));

		model->agents.removeAgent(AgentId(0, 0, 0));
		ASSERT_NE((void*)0, model->agents.getAgent(AgentId(0, 1, 0)));
		ASSERT_NE((void*)0, model->agents.getAgent(AgentId(0, 2, 0)));
		//ASSERT_EQ((void*)0, model->net->predecessors *(model->agents.getAgent(AgentId(0, 1, 0)))));
		//ASSERT_EQ((void*)0, model->net->findEdge(*(model->agents.getAgent(AgentId(0, 0, 0))), *(model->agents.getAgent(AgentId(0, 2, 0)))));
	}

	RepastProcess::instance()->syncAgentStatus<ModelAgent, ModelAgentContent>(model->agents, *model, creator);
	ASSERT_EQ((void*)0, model->agents.getAgent(AgentId(0, 0, 0)));

	if (rank == 1 || rank == 2) {
		std::vector<ModelAgent*> out;
		ModelAgent* agent = model->agents.getAgent(AgentId(0, rank, 0));
		model->net->predecessors(agent, out);
		ASSERT_EQ(0, out.size());
	}

	if (rank == 0) {
		ModelAgent* agent = model->agents.getAgent(AgentId(1, 0, 0));
		agent->state(101);
	}
	syncAgents<ModelAgentContent> (*model, creator);
	if (rank == 1 || rank == 2) {
		ModelAgent* agent = model->agents.getAgent(AgentId(1, 0, 0));
		ASSERT_EQ(101, agent->state());
	}
}

TEST_F(RSMPITests, MoveOffGrid)
{
	fillGrid(rank, model);
	AgentRequest request(rank);
	if (rank == 1 || rank == 2) {
		// P1 and 2 request (0, 0, 0) from P0
		AgentId id(0, 0, 0);
		request.addRequest(id);
		id = AgentId(1, 0, 0);
		request.addRequest(id);
	}

	AgentCreatorUpdater creator(model);
	repast::requestAgents<ModelAgent, ModelAgentContent>(model->agents, request, *model, creator);

	if (rank == 1 || rank == 2) {
		ModelAgent* agent = model->agents.getAgent(AgentId(0, 0, 0));
		ASSERT_NE((void*)0, agent);
		agent = model->agents.getAgent(AgentId(1, 0, 0));
		ASSERT_NE((void*)0, agent);
	}

	GridDimensions bounds = model->grid->bounds();
	//int x = bounds.origin(0) + bounds.extents(0) / 2;
	int y = bounds.origin(1) + bounds.extents(1) / 2;

	if (rank == 0) {
		ModelAgent* agent = model->agents.getAgent(AgentId(0, 0, 0));
		ASSERT_NE((void*)0, agent);
		// move into 1
		model->grid->moveTo(AgentId(0, 0, 0), Point<int>(2, y));
	}

	RepastProcess::instance()->syncAgentStatus<ModelAgent, ModelAgentContent>(model->agents, *model, creator);
	model->grid->synchMove();

	if (rank == 0) {
		// gone from 0 now
		ModelAgent* agent = model->agents.getAgent(AgentId(0, 0, 0));
		ASSERT_EQ((void*)0, agent);
	} else if (rank == 1 || rank == 2) {
		// should be on 1 now with current rank of 1
		ModelAgent* agent = model->agents.getAgent(AgentId(0, 0, 0));
		ASSERT_NE((void*)0, agent);
		ASSERT_EQ(1, agent->getId().currentRank());
		if (rank == 1) {
			std::vector<int> out(2, 0);
			model->grid->getLocation(agent, out);
			ASSERT_EQ(2, out[0]);
			ASSERT_EQ(-15, out[1]);
			agent->state(101);
		}
	}

	// synchronize the agents, 2 should now receive the update from 1
	syncAgents<ModelAgentContent> (*model, creator);
	if (rank == 2) {
		ModelAgent* agent = model->agents.getAgent(AgentId(0, 0, 0));
		ASSERT_NE((void*)0, agent);
		ASSERT_EQ(1, agent->getId().currentRank());
		ASSERT_EQ(101, agent->state());
	}
}

struct DData: public TDataSource<double> {
	double getData() {
		return 2.5;

	}
};

struct IntData: public TDataSource<int> {
	int getData() {
		return 2 * RepastProcess::instance()->rank();

	}
};

TEST(DataSet, WriteRecord)
{
	repast::RepastProcess::init("./config.props");
	SVDataSetBuilder builder("./data.csv", ",", RepastProcess::instance()->getScheduleRunner().schedule());
	DData ddata;
	IntData idata;
	builder.addDataSource(createSVDataSource("test_d_data", &ddata, std::plus<double>()));
	builder.addDataSource(createSVDataSource("test_i_data", &idata, boost::mpi::minimum<int>()));
	SVDataSet* dataSet = builder.createDataSet();

	ScheduleRunner& runner = RepastProcess::instance()->getScheduleRunner();
	runner.scheduleStop(20);
	Schedule::FunctorPtr record = Schedule::FunctorPtr(new MethodFunctor<SVDataSet> (dataSet, &SVDataSet::record));
	runner.scheduleEvent(1, 1, record);
	Schedule::FunctorPtr write = Schedule::FunctorPtr(new MethodFunctor<SVDataSet> (dataSet, &SVDataSet::write));
	runner.scheduleEvent(15.1, write);
	runner.scheduleEndEvent(write);

	runner.run();
	delete RepastProcess::instance();
}

void testAgentRequestState(AgentRequest request, int expectedRequestsCount, int expectedCancellationsCount,
						   AgentId* requestsContains, AgentId* requestsDoesNotContain, AgentId* cancellationsContains,
						   AgentId* cancellationsDoesNotContain, AgentId* neitherContains, int expectedTargetSize, int expectedRequestsTargetSize,
						   int expectedCancellationsTargetSize){
	
	ASSERT_EQ(expectedRequestsCount + expectedCancellationsCount, request.requestCount());
	ASSERT_EQ(expectedRequestsCount, request.requestCountRequested());
	ASSERT_EQ(expectedCancellationsCount, request.requestCountCancellations());
	
	if(requestsContains != 0){
		ASSERT_TRUE(request.contains(*requestsContains));
		ASSERT_TRUE(request.containsInRequests(*requestsContains));
		ASSERT_FALSE(request.containsInCancellations(*requestsContains));	
	}
	if(cancellationsContains != 0){
		ASSERT_TRUE(request.contains(*cancellationsContains));
		ASSERT_FALSE(request.containsInRequests(*cancellationsContains));	
		ASSERT_TRUE(request.containsInCancellations(*cancellationsContains));
	}
	if(requestsDoesNotContain != 0){
        ASSERT_FALSE(request.containsInRequests(*requestsDoesNotContain));
	}
	if(cancellationsDoesNotContain != 0){
		ASSERT_FALSE(request.containsInCancellations(*cancellationsDoesNotContain));
	}
	if(neitherContains != 0){
		ASSERT_FALSE(request.contains(*neitherContains));
	}
	
	std::set<int> targets;
	
	targets.clear();
	request.targets(targets);
	ASSERT_EQ(expectedTargetSize, targets.size());
	
	targets.clear();
	request.targetsOfRequests(targets);
	ASSERT_EQ(expectedRequestsTargetSize, targets.size());
	
	targets.clear();
	request.targetsOfCancellations(targets);
	ASSERT_EQ(expectedCancellationsTargetSize, targets.size());
	
}

TEST_F(RSMPITests, AgentRequestII)
{
	// Tests of all the AgentRequest methods
	if(rank==0){
		AgentId A1(0,0,0); A1.currentRank(0);
		AgentId A2(1,0,0); A2.currentRank(0);
		AgentId A3(2,0,0); A3.currentRank(1);
		AgentId A4(3,0,0); A4.currentRank(2);
		AgentId A5(4,0,0); A5.currentRank(0);
		AgentId B1(0,1,0); B1.currentRank(0);
		AgentId B2(1,1,0); B2.currentRank(0);
		AgentId B3(2,1,0); B3.currentRank(1);
		AgentId B4(3,1,0); B4.currentRank(2);
		AgentId B5(4,1,0); B5.currentRank(0);
		
		AgentId D0(0,2,0);
		
		AgentRequest request;
		testAgentRequestState(request, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
		
		request.addRequest(A1);
		testAgentRequestState(request, 1, 0, &A1, &A5, 0, 0, &D0, 1, 1, 0);
		
		request.addRequest(A2);
		testAgentRequestState(request, 2, 0, &A2, &A5, 0, 0, &D0, 1, 1, 0);
		
		request.addRequest(A3);
		testAgentRequestState(request, 3, 0, &A3, &A5, 0, 0, &D0, 2, 2, 0);
		
		request.addRequest(A4);
		testAgentRequestState(request, 4, 0, &A4, &A5, 0, 0, &D0, 3, 3, 0);
		
		request.remove(A1);
		testAgentRequestState(request, 3, 0, &A4, &A1, 0, 0, &D0, 3, 3, 0);
		
		request.remove(A3);
		testAgentRequestState(request, 2, 0, &A4, &A3, 0, 0, &D0, 2, 2, 0);
		// Request object now has just A2 and A4, with targets 0 and 2
		
		request.addCancellation(B1);
		testAgentRequestState(request, 2, 1, &A4, &A3, &B1, &B5, &D0, 2, 2, 1);
		
		request.addCancellation(B2);
		testAgentRequestState(request, 2, 2, &A4, &A3, &B2, &B5, &D0, 2, 2, 1);
		
		request.addCancellation(B3);
		testAgentRequestState(request, 2, 3, &A4, &A3, &B3, &B5, &D0, 3, 2, 2);
		
		request.addCancellation(B4);
		testAgentRequestState(request, 2, 4, &A4, &A3, &B3, &B5, &D0, 3, 2, 3);
		
		request.removeCancellation(B1);
		testAgentRequestState(request, 2, 3, &A4, &A3, &B3, &B5, &D0, 3, 2, 3);
		
		AgentRequest requestii;
		requestii.addCancellation(B1);
		testAgentRequestState(requestii, 0, 1, 0, &A3, &B1, &B5, &D0, 1, 0, 1);
		
		requestii.addCancellation(B2);
		testAgentRequestState(requestii, 0, 2, 0, &A3, &B2, &B5, &D0, 1, 0, 1);
		
		requestii.addCancellation(B3);
		testAgentRequestState(requestii, 0, 3, 0, &A3, &B3, &B5, &D0, 2, 0, 2);
		
		requestii.addCancellation(B4);
		testAgentRequestState(requestii, 0, 4, 0, &A3, &B3, &B5, &D0, 3, 0, 3);
		
		requestii.removeCancellation(B1);
		testAgentRequestState(requestii, 0, 3, 0, &A3, &B3, &B5, &D0, 3, 0, 3);
		
		// And test addAll
		
		// Using addAll with just requests
		AgentRequest requestiii;
		requestiii.addRequest(A1);
		requestiii.addRequest(A2);
		requestiii.addRequest(A3);
		
		AgentRequest requestiv;
		requestiv.addRequest(A4);
		requestiv.addRequest(A5);
		
		requestiii.addAll(requestiv);
		testAgentRequestState(requestiii, 5, 0, &A5, &B1, 0, 0, &B1, 3, 3, 0);
		
		// Using addAll with just cancellations
		AgentRequest requestv;
		requestv.addCancellation(A1);
		requestv.addCancellation(A2);
		requestv.addCancellation(A3);
		
		AgentRequest requestvi;
		requestvi.addCancellation(A4);
		requestvi.addCancellation(A5);
		
		requestv.addAll(requestvi);
		testAgentRequestState(requestv, 0, 5, 0, 0, &A5, &B1, &B1, 3, 0, 3);
		
		// Just adding requests
		AgentRequest requestA;
		requestA.addRequest(A1);
		requestA.addRequest(A2);
		requestA.addCancellation(B1);
		requestA.addCancellation(B2);
		testAgentRequestState(requestA, 2, 2, &A1, &A3, &B1, &B3, &B5, 1, 1, 1);
		
		AgentRequest requestB;
		requestB.addRequest(A3);
		requestB.addCancellation(B3);
		
		requestA.addAllRequests(requestB);
		testAgentRequestState(requestA, 3, 2, &A3, &A4, &B1, &B3, &B5, 2, 2, 1);
		
		
		requestA.addAllCancellations(requestB);
		testAgentRequestState(requestA, 3, 3, &A3, &A4, &B3, &B4, &B5, 2, 2, 2);
		
		
		
	}
	
	
	
	
	AgentCreatorUpdater creator(model);
	AgentId id(0, 0, 0);
	AgentId id_A(4,0,0);
	
	if(rank == 0){
		ASSERT_TRUE(model->agents.contains(id));
		ModelAgent* agent = model->agents.getAgent(id);
		agent->state(101);
    }
	
	if(rank == 1){
		ASSERT_FALSE(model->agents.contains(id));
    }
	
	AgentRequest request1(rank);
	
	if (rank == 1) {
		request1.addRequest(id);
	}
	
	// Request the agent
	repast::requestAgents<ModelAgent, ModelAgentContent>(model->agents, request1, *model, creator);
    RepastProcess::instance()->syncAgentStatus<ModelAgent, ModelAgentContent>(model->agents, *model, creator);
	syncAgents<ModelAgentContent> (*model, creator);
	
	// Confirm that the agent is shared
	if(rank == 1){
		ASSERT_TRUE(model->agents.contains(id));
		ModelAgent* agent = model->agents.getAgent(id);
		ASSERT_EQ(101, agent->state());
	}
	
	// Imagine some time passing...
	if(rank == 0){
		ModelAgent* agent = model->agents.getAgent(id);
		agent->state(102);
    }

	RepastProcess::instance()->syncAgentStatus<ModelAgent, ModelAgentContent>(model->agents, *model, creator);
	syncAgents<ModelAgentContent> (*model, creator);
	
	if(rank == 1){
		ASSERT_TRUE(model->agents.contains(id));
		ModelAgent* agent = model->agents.getAgent(id);
		ASSERT_EQ(102, agent->state()); // Should get the updated state
	}
	
	// Now cancel the agent request
	AgentRequest request2(rank);
	
	if (rank == 1) {
		request2.addCancellation(id);
		model->agents.importedAgentRemoved(id);
	}
	repast::requestAgents<ModelAgent, ModelAgentContent>(model->agents, request2, *model, creator);
	
	// Confirm that the agent is no longer shared
	if(rank == 1){
		ASSERT_FALSE(model->agents.contains(id));
    }
	
    RepastProcess::instance()->syncAgentStatus<ModelAgent, ModelAgentContent>(model->agents, *model, creator);
	syncAgents<ModelAgentContent> (*model, creator);

	if(rank == 1){
		ASSERT_FALSE(model->agents.contains(id));
    }

	// Imagine some more time passing...
	if(rank == 0){
		ModelAgent* agent = model->agents.getAgent(id);
		agent->state(103);
    }
	
	// A slightly more elaborate test; two processes are going to request an agent,
	// then one of those processes is going to cancel the request
	if(rank == 1 || rank == 2){
		ASSERT_FALSE(model->agents.contains(id));
    }
	
	AgentRequest request3(rank);
	
	if (rank == 1 || rank == 2) {
		request3.addRequest(id);
	}
	
	// Request the agent
	repast::requestAgents<ModelAgent, ModelAgentContent>(model->agents, request3, *model, creator);
    RepastProcess::instance()->syncAgentStatus<ModelAgent, ModelAgentContent>(model->agents, *model, creator);
	syncAgents<ModelAgentContent> (*model, creator);
	
	// Confirm that the agent is shared
	if(rank == 1 || rank == 2){
		ASSERT_TRUE(model->agents.contains(id));
		ModelAgent* agent = model->agents.getAgent(id);
		ASSERT_EQ(103, agent->state()); // Should get the updated state
	}

	
	// Imagine some more time passing...
	if(rank == 0){
		ModelAgent* agent = model->agents.getAgent(id);
		agent->state(104);
    }
	
	// Now cancel the agent request from proc 1 but not from 2
	AgentRequest request4(rank);
	
	if (rank == 1) {
		request4.addCancellation(id);
		model->agents.importedAgentRemoved(id);
	}
	
	repast::requestAgents<ModelAgent, ModelAgentContent>(model->agents, request4, *model, creator);
	RepastProcess::instance()->syncAgentStatus<ModelAgent, ModelAgentContent>(model->agents, *model, creator);
	syncAgents<ModelAgentContent> (*model, creator);
	
	// Confirm that the agent is no longer shared on 1
	if(rank == 1){
		ASSERT_FALSE(model->agents.contains(id));
    }

	// ... but is still on 2
	if(rank == 2){
		ASSERT_TRUE(model->agents.contains(id));
		ModelAgent* agent = model->agents.getAgent(id);
		ASSERT_EQ(104, agent->state()); // Should get the updated state
	}
	
	// Imagine some more time passing...
	if(rank == 0){
		ModelAgent* agent = model->agents.getAgent(id);
		agent->state(105);
    }
	
	
    RepastProcess::instance()->syncAgentStatus<ModelAgent, ModelAgentContent>(model->agents, *model, creator);
	syncAgents<ModelAgentContent> (*model, creator);
	
	if(rank == 2){
		ASSERT_TRUE(model->agents.contains(id));
		ModelAgent* agent = model->agents.getAgent(id);
		ASSERT_EQ(105, agent->state()); // Should get the updated state
	}
	
	/* Tests of improper agent requests */
	
	// An agent is requested and then requested again
	//    Behavior: Should fail silently; the redundant request should be ignored
	AgentId aid1(1,0,0);
	AgentRequest req1(rank);
	if(rank == 1){
		req1.addRequest(aid1);
    }
	
	repast::requestAgents<ModelAgent, ModelAgentContent>(model->agents, req1, *model, creator);
	RepastProcess::instance()->syncAgentStatus<ModelAgent, ModelAgentContent>(model->agents, *model, creator);
	syncAgents<ModelAgentContent> (*model, creator);
	
	repast::requestAgents<ModelAgent, ModelAgentContent>(model->agents, req1, *model, creator);
	RepastProcess::instance()->syncAgentStatus<ModelAgent, ModelAgentContent>(model->agents, *model, creator);
	syncAgents<ModelAgentContent> (*model, creator);
	
	// An agent request contains two requests for the same agent
	//    Behavior: Should fail silently; the redundant request should be ignored
	AgentId aid2(2,0,0);
	AgentRequest req2(rank);
	if(rank == 1){
		req2.addRequest(aid2);
		req2.addRequest(aid2);
		ASSERT_EQ(2, req2.requestCount());

    }
	
	if(rank == 0){
		ModelAgent* agent = model->agents.getAgent(aid2);
		agent->state(500);
    }
	
	
	repast::requestAgents<ModelAgent, ModelAgentContent>(model->agents, req2, *model, creator);
	RepastProcess::instance()->syncAgentStatus<ModelAgent, ModelAgentContent>(model->agents, *model, creator);
	syncAgents<ModelAgentContent> (*model, creator);
	
	
	if(rank == 1){
		ASSERT_TRUE(model->agents.contains(aid2));
		ModelAgent* agent = model->agents.getAgent(aid2);
		ASSERT_EQ(500, agent->state()); // Should get the updated state
	}
	
	// An agent request contains both a request and a cancellation for the same agent
	//    Behavior: The cancellation should be processed second; the effect is as if the
	//       request had not contained the agent at all
	AgentId aid3(3,0,0);
	AgentRequest req3(rank);
	if(rank == 1){
		req3.addRequest(aid3);
		req3.addCancellation(aid3);
		ASSERT_EQ(2, req2.requestCount());
		
    }
	
	if(rank == 0){
		ModelAgent* agent = model->agents.getAgent(aid3);
		agent->state(600);
    }
	
	
	repast::requestAgents<ModelAgent, ModelAgentContent>(model->agents, req3, *model, creator);
	RepastProcess::instance()->syncAgentStatus<ModelAgent, ModelAgentContent>(model->agents, *model, creator);
	syncAgents<ModelAgentContent> (*model, creator);
	
	
	if(rank == 1){
		ASSERT_FALSE(model->agents.contains(aid3));
	}
	
	
	
	// An agent request contains two cancellations for the same agent
	//    Behavior: Should fail silently, the second request being ignored
	
	AgentRequest req4(rank);
	if(rank == 1){
		req4.addCancellation(aid2);
		req4.addCancellation(aid2);
    }
	
	if(rank == 0){
		ModelAgent* agent = model->agents.getAgent(aid2);
		agent->state(300);
    }
	
	repast::requestAgents<ModelAgent, ModelAgentContent>(model->agents, req4, *model, creator);
	RepastProcess::instance()->syncAgentStatus<ModelAgent, ModelAgentContent>(model->agents, *model, creator);
	syncAgents<ModelAgentContent> (*model, creator);
	
	// Rank 1 still 'contains' the agent- it hasn't been removed- but the state should NOT be updated:
	if(rank == 1){
		ASSERT_TRUE(model->agents.contains(aid2));
		ModelAgent* agent = model->agents.getAgent(aid2);
		ASSERT_EQ(500, agent->state());
		// Remove it
		model->agents.importedAgentRemoved(aid2);
		ASSERT_FALSE(model->agents.contains(aid2));
	}
	
	
	
	// An agent request contains a cancellation for an agent that was never requested
	//    Behavior: Should fail silently
	AgentId aid4(4,0,0);
	AgentRequest req5(rank);
	if(rank == 1){
		req5.addCancellation(aid4);
	}
	
	repast::requestAgents<ModelAgent, ModelAgentContent>(model->agents, req5, *model, creator);
	RepastProcess::instance()->syncAgentStatus<ModelAgent, ModelAgentContent>(model->agents, *model, creator);
	syncAgents<ModelAgentContent> (*model, creator);
	
	
	// An agent request contains a cancellation for an agent that was previously requested and then cancelled
	//    Behavior: Should fail silently
	
	AgentRequest req6(rank);
	if(rank == 1){
		req6.addCancellation(aid2);
    }

	repast::requestAgents<ModelAgent, ModelAgentContent>(model->agents, req6, *model, creator);
	RepastProcess::instance()->syncAgentStatus<ModelAgent, ModelAgentContent>(model->agents, *model, creator);
	syncAgents<ModelAgentContent> (*model, creator);
	
	// These two 'tests' are actually for conditions that will cause the program to halt:
	
//	// An agent request is issued for an agent that is not in the population
//	//    Program should fail with log to error
//	
//	AgentId aidDUMMY(10,0,0);
//	AgentRequest req7(rank);
//	if(rank == 1){
//		req7.addRequest(aidDUMMY);
//	}
//
//	repast::requestAgents<ModelAgent, ModelAgentContent>(model->agents, req7, *model, creator);
//	RepastProcess::instance()->syncAgentStatus<ModelAgent, ModelAgentContent>(model->agents, *model, creator);
//	syncAgents<ModelAgentContent> (*model, creator);
//	
//	// An agent cancellation is issued for an agent that is not in the population
//	//    This will fail silently
//	AgentRequest req8(rank);
//	if(rank == 1){
//		req8.addCancellation(aidDUMMY);
//	}
//	
//	repast::requestAgents<ModelAgent, ModelAgentContent>(model->agents, req8, *model, creator);
//	RepastProcess::instance()->syncAgentStatus<ModelAgent, ModelAgentContent>(model->agents, *model, creator);
//	syncAgents<ModelAgentContent> (*model, creator);
	
}




// Useful tests
void testSetContains(std::set<ModelAgent*>& agentSet, std::vector<ModelAgent*>& agentVec, std::vector<ModelAgent*>& excluding){
	std::vector<ModelAgent*>::iterator excIterator = excluding.begin();
	
	while(excIterator != excluding.end()){
		ASSERT_TRUE(find(agentSet.begin(), agentSet.end(), *excIterator) != agentSet.end());
		ASSERT_TRUE(find(agentVec.begin(), agentVec.end(), *excIterator) != agentVec.end());
		excIterator++;
	}
}


void testSetDoesNotContain(std::set<ModelAgent*>& agentSet, std::vector<ModelAgent*>& agentVec, std::vector<ModelAgent*>& excluding){
	std::vector<ModelAgent*>::iterator excIterator = excluding.begin();
	
	while(excIterator != excluding.end()){
		ASSERT_TRUE(find(agentSet.begin(), agentSet.end(), *excIterator) == agentSet.end());
		ASSERT_TRUE(find(agentVec.begin(), agentVec.end(), *excIterator) == agentVec.end());
		excIterator++;
	}
	
}

void testAgentsAreOfType(std::set<ModelAgent*>& agentSet, std::vector<ModelAgent*>& agentVec, int type){
	set<ModelAgent*>::iterator setIterator = agentSet.begin();
	vector<ModelAgent*>::iterator vecIterator = agentVec.begin();
	
	for(unsigned int i = 0; i < agentVec.size(); i++){
		ASSERT_EQ(type, (*setIterator)->getId().agentType());
		ASSERT_EQ(type, (*vecIterator)->getId().agentType());
		setIterator++;
		vecIterator++;
	}
}


void testAgentIDsBelow(std::set<ModelAgent*>& agentSet, std::vector<ModelAgent*>& agentVec, int upperIDBound){
	set<ModelAgent*>::iterator setIterator = agentSet.begin();
	vector<ModelAgent*>::iterator vecIterator = agentVec.begin();
	
	for(unsigned int i = 0; i < agentVec.size(); i++){
		ASSERT_TRUE((*setIterator)->getId().id() <= upperIDBound);
		ASSERT_TRUE((*vecIterator)->getId().id() <= upperIDBound);
		setIterator++;
		vecIterator++;
	}
}


void testLocalNonLocal(std::set<ModelAgent*>& agentSet, std::vector<ModelAgent*>& agentVec, repast::SharedContext<ModelAgent>::filterLocalFlag FLAG){
	std::set<ModelAgent*>::iterator setIterator = agentSet.begin();
	std::vector<ModelAgent*>::iterator vecIterator = agentVec.begin();
	
	for(unsigned int i = 0; i < agentVec.size(); i++){
		if(FLAG == repast::SharedContext<ModelAgent>::LOCAL){
			ASSERT_TRUE((*setIterator)->getId().startingRank() == 1);
			ASSERT_TRUE((*vecIterator)->getId().startingRank() == 1);
		}
		else{
			ASSERT_TRUE((*setIterator)->getId().startingRank() != 1);
			ASSERT_TRUE((*vecIterator)->getId().startingRank() != 1);
		}
		setIterator++;
		vecIterator++;
	}
}


// Demonstration filter
struct IsLowNumberedAgent {
    IsLowNumberedAgent(){}
	
    bool operator()(const boost::shared_ptr<ModelAgent>& ptr){
        return ptr->getId().id() <= 1200;
    }
	
};





TEST_F(RSMPITests, SelectAgents){
	for(int i = 0; i < 1000; i++){
		AgentId id(1000 + i, rank, i % 3);
		ModelAgent* agent = new ModelAgent(id, 0); // Note: some different types
		model->agents.addAgent(agent);
	}
	
	// Exchange agents
	
	AgentRequest request1(rank);
	AgentCreatorUpdater creator(model);
	
	if(rank == 1){
        for(int i = 0; i < 500; i++){
			AgentId id(1000 + i, 0, i % 3);
			request1.addRequest(id);
		}
	}
	
	// Request the agents
	repast::requestAgents<ModelAgent, ModelAgentContent>(model->agents, request1, *model, creator);
    RepastProcess::instance()->syncAgentStatus<ModelAgent, ModelAgentContent>(model->agents, *model, creator);
	syncAgents<ModelAgentContent> (*model, creator);
	
	
	set<ModelAgent*> resultSet;
	vector<ModelAgent*> resultVec;
	
	int popSize = countOf(model->agents.begin(), model->agents.end());
	
	
	if(rank == 1){
		
		// Chance plays a role; do multiple runs
		for(int t = 0; t < 20; t++){

			
			// SELECTION I: All Agents
			
			// SETUP
			resultSet.clear();
			resultVec.clear();
			
			// SELECTION
			model->agents.selectAgents(resultSet);
			model->agents.selectAgents(resultVec);
			
			// TESTS
			ASSERT_EQ(popSize, resultSet.size());
			ASSERT_EQ(popSize, resultVec.size());
			
			
			
			// SELECTION II: Specific Number of Agents
			
			// SETUP
			resultSet.clear();
			resultVec.clear();
			
			// SELECTION
			model->agents.selectAgents(100, resultSet);
			model->agents.selectAgents(100, resultVec);
			
			// TESTS
			ASSERT_EQ(100, resultSet.size());
			ASSERT_EQ(100, resultVec.size());
			
			
			// SELECTION III: Excluding certain agents, and leaving them in the result set
			
			// SETUP
			vector<ModelAgent*> temp(resultVec.begin(), resultVec.end());
			
			resultSet.clear(); resultSet.insert(temp.begin(), temp.end());
			resultVec.clear(); resultVec.insert(resultVec.begin(), temp.begin(), temp.end());
			
			
			// SELECTION
			model->agents.selectAgents(100, resultSet);
			model->agents.selectAgents(100, resultVec);
			
			
			// TESTS
			ASSERT_EQ(200, resultSet.size());
			ASSERT_EQ(200, resultVec.size());
			testSetContains(resultSet, resultVec, temp);
			
			
			
			// SELECTION IV: Excluding certain agents, but removing them from the result set
			
			// SETUP
			resultSet.clear(); resultSet.insert(temp.begin(), temp.end());
			resultVec.clear(); resultVec.insert(resultVec.begin(), temp.begin(), temp.end());
			
			// SELECTION
			model->agents.selectAgents(100, resultSet, true);
			model->agents.selectAgents(100, resultVec, true);
			
			// TESTS
			ASSERT_EQ(100, resultSet.size());
			ASSERT_EQ(100, resultVec.size());
			testSetDoesNotContain(resultSet, resultVec, temp);
						
			
			
			// SELECTION V: Test of the optional 'popSize' parameter
			// Not used for basic selection	
			
		}
	}
}



TEST_F(RSMPITests, SelectAgentsByType){
	for(int i = 0; i < 1000; i++){
		AgentId id(1000 + i, rank, i % 3);
		ModelAgent* agent = new ModelAgent(id, 0); // Note: some different types
		model->agents.addAgent(agent);
	}
	
	// Exchange agents
	
	AgentRequest request1(rank);
	AgentCreatorUpdater creator(model);
	
	if(rank == 1){
        for(int i = 0; i < 500; i++){
			AgentId id(1000 + i, 0, i % 3);
			request1.addRequest(id);
		}
	}
	
	// Request the agents
	repast::requestAgents<ModelAgent, ModelAgentContent>(model->agents, request1, *model, creator);
    RepastProcess::instance()->syncAgentStatus<ModelAgent, ModelAgentContent>(model->agents, *model, creator);
	syncAgents<ModelAgentContent> (*model, creator);

	
	set<ModelAgent*> resultSet;
	vector<ModelAgent*> resultVec;

	int _type = 1;
	
	int popSize = countOf(model->agents.byTypeBegin(_type), model->agents.byTypeEnd(_type));
	
	if(rank == 1){
		
		// Chance plays a role; do multiple runs
		for(int t = 0; t < 20; t++){
			
			
			// SELECTION I: All Agents
			
			// SETUP
			resultSet.clear();
			resultVec.clear();
			
			// SELECTION
			model->agents.selectAgents(resultSet, _type);
			model->agents.selectAgents(resultVec, _type);
			
			// TESTS
			ASSERT_EQ(popSize, resultSet.size());
			ASSERT_EQ(popSize, resultVec.size());
			testAgentsAreOfType(resultSet, resultVec, _type);
			
			
			// SELECTION II: Specific Number of Agents
			
			// SETUP
			resultSet.clear();
			resultVec.clear();
			
			
			// SELECTION Select agents
			model->agents.selectAgents(100, resultSet, _type);
			model->agents.selectAgents(100, resultVec, _type);
			
			// TESTS
			ASSERT_EQ(100, resultSet.size());
			ASSERT_EQ(100, resultVec.size());
			testAgentsAreOfType(resultSet, resultVec, _type);
			
			
			// SELECTION III: Excluding certain agents, and leaving them in the result set
			
			// SETUP
			vector<ModelAgent*> temp(resultVec.begin(), resultVec.end());
			
			// SETUP
			resultSet.clear();		resultSet.insert(temp.begin(), temp.end());
			resultVec.clear();		resultVec.insert(resultVec.begin(), temp.begin(), temp.end());
			
			model->agents.selectAgents(100, resultSet, _type);
			model->agents.selectAgents(100, resultVec, _type);
			
			// TESTS
			ASSERT_EQ(200, resultSet.size());
			ASSERT_EQ(200, resultVec.size());
			testAgentsAreOfType(resultSet, resultVec, _type);
			testSetContains(resultSet, resultVec, temp);
			
			
			// SELECTION IV: Excluding certain agents, but removing them from the result set
			
			// SETUP
			resultSet.clear();		resultSet.insert(temp.begin(), temp.end());
			resultVec.clear();		resultVec.insert(resultVec.begin(), temp.begin(), temp.end());
			
			
			// SELECTION
			model->agents.selectAgents(100, resultSet, _type, true);
			model->agents.selectAgents(100, resultVec, _type, true);
			
			// TESTS
			ASSERT_EQ(100, resultSet.size());
			ASSERT_EQ(100, resultVec.size());
			testAgentsAreOfType(resultSet, resultVec, _type);
			testSetDoesNotContain(resultSet, resultVec, temp);		
			
			
			
			// SELECTION V: Test of the optional 'popSize' parameter
			
			// SETUP
			int expectedSize = (popSize < 100 ? popSize	: 100);
			resultSet.clear();
			resultVec.clear();
			
			// SELECTION
			model->agents.selectAgents(100, resultSet, _type, false, popSize);
			model->agents.selectAgents(100, resultVec, _type, false, popSize);
			
			// TESTS
			ASSERT_EQ(expectedSize, resultSet.size());
			ASSERT_EQ(expectedSize, resultVec.size());
			testAgentsAreOfType(resultSet, resultVec, _type);
			
			
		}
	}
}


TEST_F(RSMPITests, SelectFilteredAgents){
	for(int i = 0; i < 1000; i++){
		AgentId id(1000 + i, rank, i % 3);
		ModelAgent* agent = new ModelAgent(id, 0); // Note: some different types
		model->agents.addAgent(agent);
	}
	
	// Exchange agents
	
	AgentRequest request1(rank);
	AgentCreatorUpdater creator(model);
	
	if(rank == 1){
        for(int i = 0; i < 500; i++){
			AgentId id(1000 + i, 0, i % 3);
			request1.addRequest(id);
		}
	}
	
	// Request the agents
	repast::requestAgents<ModelAgent, ModelAgentContent>(model->agents, request1, *model, creator);
    RepastProcess::instance()->syncAgentStatus<ModelAgent, ModelAgentContent>(model->agents, *model, creator);
	syncAgents<ModelAgentContent> (*model, creator);

	
	set<ModelAgent*> resultSet;
	vector<ModelAgent*> resultVec;
	
	IsLowNumberedAgent filter;
	
	int popSize = countOf(model->agents.filteredBegin(filter), model->agents.filteredEnd(filter));
	
	if(rank == 1){
		
		// Chance plays a role; do multiple runs
		for(int t = 0; t < 20; t++){
			
			// SELECTION I: All Agents
			
			// SETUP
			resultSet.clear();
			resultVec.clear();
			
			// SELECTION
			model->agents.selectAgents(resultSet, filter);
			model->agents.selectAgents(resultVec, filter);
			
			// Should have received all of the agents
			ASSERT_EQ(popSize, resultSet.size());
			ASSERT_EQ(popSize, resultVec.size());
			testAgentIDsBelow(resultSet, resultVec, 1200);
			
			
			
			// SELECTION II: Specific Number of Agents
			
			// SETUP
			resultSet.clear();
			resultVec.clear();
			
			// SELECTION
			model->agents.selectAgents(100, resultSet, filter);
			model->agents.selectAgents(100, resultVec, filter);
			
			// TESTS
			ASSERT_EQ(100, resultSet.size());
			ASSERT_EQ(100, resultVec.size());
			testAgentIDsBelow(resultSet, resultVec, 1200);		
			
			
			
			// SELECTION III: Excluding certain agents, and leaving them in the result set
			
			// SETUP
			vector<ModelAgent*> temp(resultVec.begin(), resultVec.end());
			resultSet.clear();		resultSet.insert(temp.begin(), temp.end());
			resultVec.clear();		resultVec.insert(resultVec.begin(), temp.begin(), temp.end());
			
			// SELECTION
			model->agents.selectAgents(100, resultSet, filter);
			model->agents.selectAgents(100, resultVec, filter);
			
			// TESTS
			// Now should have 200 (note: there are 201 (0-200, inclusive) that we could have selected
			ASSERT_EQ(200, resultSet.size());
			ASSERT_EQ(200, resultVec.size());
			testAgentIDsBelow(resultSet, resultVec, 1200);
			testSetContains(resultSet, resultVec, temp);
			
			
			// SELECTION IV: Excluding certain agents, but removing them from the result set
			// SETUP
			resultSet.clear();		resultSet.insert(temp.begin(), temp.end());
			resultVec.clear();		resultVec.insert(resultVec.begin(), temp.begin(), temp.end());
			
			// SELECTION
			model->agents.selectAgents(100, resultSet, filter, true);
			model->agents.selectAgents(100, resultVec, filter, true);
			
			// TESTS
			ASSERT_EQ(100, resultSet.size());
			ASSERT_EQ(100, resultVec.size());
			testAgentIDsBelow(resultSet, resultVec, 1200);
			testSetDoesNotContain(resultSet, resultVec, temp);
			
			
			
			// SELECTION V: Test of the optional 'popSize' parameter
			
			// SETUP
			int expectedSize = (popSize < 100 ? popSize : 100);
			resultSet.clear();
			resultVec.clear();
			
			// SELECTION
			model->agents.selectAgents(100, resultSet, filter, false, popSize);
			model->agents.selectAgents(100, resultVec, filter, false, popSize);
			
			// Now should have the original 100+ up to 100 more
			ASSERT_EQ(expectedSize, resultSet.size());
			ASSERT_EQ(expectedSize, resultVec.size());
			testAgentIDsBelow(resultSet, resultVec, 1200);
			
		}
	}
}






TEST_F(RSMPITests, SelectFilteredAgentsByType){
	for(int i = 0; i < 1000; i++){
		AgentId id(1000 + i, rank, i % 3);
		ModelAgent* agent = new ModelAgent(id, 0); // Note: some different types
		model->agents.addAgent(agent);
	}
	
	// Exchange agents
	
	AgentRequest request1(rank);
	AgentCreatorUpdater creator(model);
	
	if(rank == 1){
        for(int i = 0; i < 500; i++){
			AgentId id(1000 + i, 0, i % 3);
			request1.addRequest(id);
		}
	}
	
	// Request the agents
	repast::requestAgents<ModelAgent, ModelAgentContent>(model->agents, request1, *model, creator);
    RepastProcess::instance()->syncAgentStatus<ModelAgent, ModelAgentContent>(model->agents, *model, creator);
	syncAgents<ModelAgentContent> (*model, creator);
	
	set<ModelAgent*> resultSet;
	vector<ModelAgent*> resultVec;
	
	IsLowNumberedAgent filter;
	
	int _type = 1;
	
	int popSize = countOf(model->agents.byTypeFilteredBegin(_type, filter), model->agents.byTypeFilteredEnd(_type, filter));
	
	
	if(rank == 1){
		
		// Chance plays a role; do multiple runs
		for(int t = 0; t < 20; t++){
	
				
			// SELECTION I: All Agents
			
			// SETUP
			resultSet.clear();
			resultVec.clear();
			
			// SELECTION
			model->agents.selectAgents(resultSet, _type, filter);
			model->agents.selectAgents(resultVec, _type, filter);
			
			// Should have received all of the agents
			ASSERT_EQ(popSize, resultSet.size());
			ASSERT_EQ(popSize, resultVec.size());
			testAgentsAreOfType(resultSet, resultVec, _type);
			testAgentIDsBelow(resultSet, resultVec, 1200);
			
			
			// SELECTION II: Specific Number of Agents
			
			// SETUP
			resultSet.clear();
			resultVec.clear();
			
			// SELECTION
			model->agents.selectAgents(10, resultSet, _type, filter);
			model->agents.selectAgents(10, resultVec, _type, filter);
			
			// TESTS
			ASSERT_EQ(10, resultSet.size());
			ASSERT_EQ(10, resultVec.size());
			testAgentsAreOfType(resultSet, resultVec, _type);
			testAgentIDsBelow(resultSet, resultVec, 1200);
			
			
			// SELECTION III: Excluding certain agents, and leaving them in the result set
			
			// SETUP
			vector<ModelAgent*> temp(resultVec.begin(), resultVec.end());
			resultSet.clear();		resultSet.insert(temp.begin(), temp.end());
			resultVec.clear();		resultVec.insert(resultVec.begin(), temp.begin(), temp.end());
			
			// SELECTION
			model->agents.selectAgents(10, resultSet, _type, filter);
			model->agents.selectAgents(10, resultVec, _type, filter);
			
			// TESTS
			// Now should have 20 
			ASSERT_EQ(20, resultSet.size());
			ASSERT_EQ(20, resultVec.size());
			testAgentsAreOfType(resultSet, resultVec, _type);
			testAgentIDsBelow(resultSet, resultVec, 1200);
			testSetContains(resultSet, resultVec, temp);
			
			
			// SELECTION IV: Excluding certain agents, but removing them from the result set
			
			// SETUP
			resultSet.clear();		resultSet.insert(temp.begin(), temp.end());
			resultVec.clear();		resultVec.insert(resultVec.begin(), temp.begin(), temp.end());
			
			// SELECTION
			model->agents.selectAgents(10, resultSet, _type, filter, true);
			model->agents.selectAgents(10, resultVec, _type, filter, true);
			
			// Now should have just 10
			ASSERT_EQ(10, resultSet.size());
			ASSERT_EQ(10, resultVec.size());
			testAgentsAreOfType(resultSet, resultVec, _type);
			testAgentIDsBelow(resultSet, resultVec, 1200);
			testSetDoesNotContain(resultSet, resultVec, temp);
			
			
			// SELECTION V: Test of the optional 'popSize' parameter
			
			// SETUP
			int expectedSize = (popSize < 100 ? popSize : 100);
			resultSet.clear();
			resultVec.clear();
			
			// SELECTION
			model->agents.selectAgents(100, resultSet, _type, filter, false, popSize);
			model->agents.selectAgents(100, resultVec, _type, filter, false, popSize);
			
			// TESTS
			ASSERT_EQ(expectedSize, resultSet.size());
			ASSERT_EQ(expectedSize, resultVec.size());
			testAgentsAreOfType(resultSet, resultVec, _type);
			testAgentIDsBelow(resultSet, resultVec, 1200);
			
		}
	}
}






/* Local v. non-local agents */

	


TEST_F(RSMPITests, SelectLocalOrNonLocalAgents){
	for(int i = 0; i < 1000; i++){
		AgentId id(1000 + i, rank, i % 3);
		ModelAgent* agent = new ModelAgent(id, 0); // Note: some different types
		model->agents.addAgent(agent);
	}
	
	// Exchange agents
	
	AgentRequest request1(rank);
	AgentCreatorUpdater creator(model);
	
	if(rank == 1){
        for(int i = 0; i < 500; i++){
			AgentId id(1000 + i, 0, i % 3);
			request1.addRequest(id);
		}
	}
	
	// Request the agents
	repast::requestAgents<ModelAgent, ModelAgentContent>(model->agents, request1, *model, creator);
    RepastProcess::instance()->syncAgentStatus<ModelAgent, ModelAgentContent>(model->agents, *model, creator);
	syncAgents<ModelAgentContent> (*model, creator);
	
	
	set<ModelAgent*> resultSet;
	vector<ModelAgent*> resultVec;
	
	
	if(rank == 1){
		
		for (int l = 0; l <= 1; l++){
			repast::SharedContext<ModelAgent>::filterLocalFlag FLAG = repast::SharedContext<ModelAgent>::LOCAL;
			if(l == 1)                                         FLAG = repast::SharedContext<ModelAgent>::NON_LOCAL;

			int popSize = countOf(model->agents.begin(FLAG), model->agents.end(FLAG));

			// SELECTION I: All Agents
			
			// SETUP
			resultSet.clear();
			resultVec.clear();
			
			// SELECTION
			model->agents.selectAgents(FLAG, resultSet);
			model->agents.selectAgents(FLAG, resultVec);
			
			// TESTS
			ASSERT_EQ(popSize, resultSet.size());
			ASSERT_EQ(popSize, resultVec.size());
			testLocalNonLocal(resultSet, resultVec, FLAG);
			
			
			
			// SELECTION II: Specific Number of Agents
			
			// SETUP
			resultSet.clear();
			resultVec.clear();
			
			// SELECTION
			model->agents.selectAgents(FLAG, 100, resultSet);
			model->agents.selectAgents(FLAG, 100, resultVec);
			
			// TESTS
			ASSERT_EQ(100, resultSet.size());
			ASSERT_EQ(100, resultVec.size());
			testLocalNonLocal(resultSet, resultVec, FLAG);			
			
			// SELECTION III: Excluding certain agents, and leaving them in the result set
			
			// SETUP
			vector<ModelAgent*> temp(resultVec.begin(), resultVec.end());
			
			resultSet.clear(); resultSet.insert(temp.begin(), temp.end());
			resultVec.clear(); resultVec.insert(resultVec.begin(), temp.begin(), temp.end());
			
			
			// SELECTION
			model->agents.selectAgents(FLAG, 100, resultSet);
			model->agents.selectAgents(FLAG, 100, resultVec);
			
			
			// TESTS
			ASSERT_EQ(200, resultSet.size());
			ASSERT_EQ(200, resultVec.size());
			testSetContains(resultSet, resultVec, temp);
			testLocalNonLocal(resultSet, resultVec, FLAG);
			
			
			// SELECTION IV: Excluding certain agents, but removing them from the result set
			
			// SETUP
			resultSet.clear(); resultSet.insert(temp.begin(), temp.end());
			resultVec.clear(); resultVec.insert(resultVec.begin(), temp.begin(), temp.end());
			
			// SELECTION
			model->agents.selectAgents(FLAG, 100, resultSet, true);
			model->agents.selectAgents(FLAG, 100, resultVec, true);
			
			// TESTS
			ASSERT_EQ(100, resultSet.size());
			ASSERT_EQ(100, resultVec.size());
			testSetDoesNotContain(resultSet, resultVec, temp);
			testLocalNonLocal(resultSet, resultVec, FLAG);			
			
			
			// SELECTION V: Test of the optional 'popSize' parameter
			// Not used for basic selection	
			
			
		}
	}
}



TEST_F(RSMPITests, SelectLocalOrNonLocalAgentsByType){
	for(int i = 0; i < 1000; i++){
		AgentId id(1000 + i, rank, i % 3);
		ModelAgent* agent = new ModelAgent(id, 0); // Note: some different types
		model->agents.addAgent(agent);
	}
	
	// Exchange agents
	
	AgentRequest request1(rank);
	AgentCreatorUpdater creator(model);
	
	if(rank == 1){
        for(int i = 0; i < 500; i++){
			AgentId id(1000 + i, 0, i % 3);
			request1.addRequest(id);
		}
	}
	
	// Request the agents
	repast::requestAgents<ModelAgent, ModelAgentContent>(model->agents, request1, *model, creator);
    RepastProcess::instance()->syncAgentStatus<ModelAgent, ModelAgentContent>(model->agents, *model, creator);
	syncAgents<ModelAgentContent> (*model, creator);
	
	set<ModelAgent*> resultSet;
	vector<ModelAgent*> resultVec;

	int _type = 1;
	
	if(rank == 1){
		for(int l = 0; l <= 1; l++){
			repast::SharedContext<ModelAgent>::filterLocalFlag FLAG = repast::SharedContext<ModelAgent>::LOCAL;
			if(l == 1)                                         FLAG = repast::SharedContext<ModelAgent>::NON_LOCAL;

			int popSize = countOf(model->agents.byTypeBegin(FLAG, _type), model->agents.byTypeEnd(FLAG, _type));
			
			// Chance plays a role; do multiple runs
			for(int t = 0; t < 20; t++){
			
				// SELECTION I: All Agents
				
				// SETUP
				resultSet.clear();
				resultVec.clear();
				
				// SELECTION
				model->agents.selectAgents(FLAG, resultSet, _type);
				model->agents.selectAgents(FLAG, resultVec, _type);
				
				// TESTS
				ASSERT_EQ(popSize, resultSet.size());
				ASSERT_EQ(popSize, resultVec.size());
				testAgentsAreOfType(resultSet, resultVec, _type);
				testLocalNonLocal(resultSet, resultVec, FLAG);
				
				
				// SELECTION II: Specific Number of Agents
				
				// SETUP
				resultSet.clear();
				resultVec.clear();
				
				
				// SELECTION Select agents
				model->agents.selectAgents(FLAG, 100, resultSet, _type);
				model->agents.selectAgents(FLAG, 100, resultVec, _type);
				
				// TESTS
				ASSERT_EQ((popSize < 100 ? popSize : 100), resultSet.size());
				ASSERT_EQ((popSize < 100 ? popSize : 100), resultVec.size());
				testAgentsAreOfType(resultSet, resultVec, _type);
				testLocalNonLocal(resultSet, resultVec, FLAG);
				
				
				// SELECTION III: Excluding certain agents, and leaving them in the result set
				
				// SETUP
				vector<ModelAgent*> temp(resultVec.begin(), resultVec.end());
				
				// SETUP
				resultSet.clear();		resultSet.insert(temp.begin(), temp.end());
				resultVec.clear();		resultVec.insert(resultVec.begin(), temp.begin(), temp.end());
				
				ASSERT_EQ(100, resultSet.size());
				
				model->agents.selectAgents(FLAG, 100, resultSet, _type);
				model->agents.selectAgents(FLAG, 100, resultVec, _type);
				
				// TESTS
				ASSERT_EQ((popSize < 200 ? popSize : 200), resultSet.size());
				ASSERT_EQ((popSize < 200 ? popSize : 200), resultVec.size());
				testAgentsAreOfType(resultSet, resultVec, _type);
				testSetContains(resultSet, resultVec, temp);
				testLocalNonLocal(resultSet, resultVec, FLAG);
				
				
				// SELECTION IV: Excluding certain agents, but removing them from the result set
				
				// SETUP
				resultSet.clear();		resultSet.insert(temp.begin(), temp.end());
				resultVec.clear();		resultVec.insert(resultVec.begin(), temp.begin(), temp.end());
				
				
				// SELECTION
				model->agents.selectAgents(FLAG, 100, resultSet, _type, true);
				model->agents.selectAgents(FLAG, 100, resultVec, _type, true);
				
				// TESTS
				ASSERT_EQ(((popSize - 100) < 100 ? popSize - 100 : 100), resultSet.size());
				ASSERT_EQ(((popSize - 100) < 100 ? popSize - 100 : 100), resultVec.size());
				testAgentsAreOfType(resultSet, resultVec, _type);
				testSetDoesNotContain(resultSet, resultVec, temp);		
				testLocalNonLocal(resultSet, resultVec, FLAG);
				
				
				
				// SELECTION V: Test of the optional 'popSize' parameter
				
				// SETUP
				int expectedSize = (popSize < 100 ? popSize	: 100);
				resultSet.clear();
				resultVec.clear();
				
				// SELECTION
				model->agents.selectAgents(FLAG, 100, resultSet, _type, false, popSize);
				model->agents.selectAgents(FLAG, 100, resultVec, _type, false, popSize);
				
				// TESTS
				ASSERT_EQ(expectedSize, resultSet.size());
				ASSERT_EQ(expectedSize, resultVec.size());
				testAgentsAreOfType(resultSet, resultVec, _type);
//				testLocalNonLocal(resultSet, resultVec, FLAG);
				
			}
		}
	}
}


TEST_F(RSMPITests, SelectLocalOrNonLocalFilteredAgents){
	for(int i = 0; i < 1000; i++){
		AgentId id(1000 + i, rank, i % 3);
		ModelAgent* agent = new ModelAgent(id, 0); // Note: some different types
		model->agents.addAgent(agent);
	}
	
	// Exchange agents
	
	AgentRequest request1(rank);
	AgentCreatorUpdater creator(model);
	
	if(rank == 1){
        for(int i = 0; i < 500; i++){
			AgentId id(1000 + i, 0, i % 3);
			request1.addRequest(id);
		}
	}
	
	// Request the agents
	repast::requestAgents<ModelAgent, ModelAgentContent>(model->agents, request1, *model, creator);
    RepastProcess::instance()->syncAgentStatus<ModelAgent, ModelAgentContent>(model->agents, *model, creator);
	syncAgents<ModelAgentContent> (*model, creator);
	
	
	set<ModelAgent*> resultSet;
	vector<ModelAgent*> resultVec;

	IsLowNumberedAgent filter;
	
	if(rank == 1){
		for (int l = 0; l <= 1; l++){
			repast::SharedContext<ModelAgent>::filterLocalFlag FLAG = repast::SharedContext<ModelAgent>::LOCAL;
			if(l == 1)                                         FLAG = repast::SharedContext<ModelAgent>::NON_LOCAL;
			

			int popSize = countOf(model->agents.filteredBegin(FLAG, filter), model->agents.filteredEnd(FLAG, filter));

			// Chance plays a role; do multiple runs
			for(int t = 0; t < 20; t++){
			
			
				// SELECTION I: All Agents
				
				// SETUP
				resultSet.clear();
				resultVec.clear();
				
				// SELECTION
				model->agents.selectAgents(FLAG, resultSet, filter);
				model->agents.selectAgents(FLAG, resultVec, filter);
				
				// Should have received all of the agents
				ASSERT_EQ(popSize, resultSet.size());
				ASSERT_EQ(popSize, resultVec.size());
				testAgentIDsBelow(resultSet, resultVec, 1200);
				testLocalNonLocal(resultSet, resultVec, FLAG);				
				
				
				// SELECTION II: Specific Number of Agents
				
				// SETUP
				resultSet.clear();
				resultVec.clear();
				
				// SELECTION
				model->agents.selectAgents(FLAG, 100, resultSet, filter);
				model->agents.selectAgents(FLAG, 100, resultVec, filter);
				
				// TESTS
				ASSERT_EQ(100, resultSet.size());
				ASSERT_EQ(100, resultVec.size());
				testAgentIDsBelow(resultSet, resultVec, 1200);		
				testLocalNonLocal(resultSet, resultVec, FLAG);
				
				
				// SELECTION III: Excluding certain agents, and leaving them in the result set
				
				// SETUP
				vector<ModelAgent*> temp(resultVec.begin(), resultVec.end());
				resultSet.clear();		resultSet.insert(temp.begin(), temp.end());
				resultVec.clear();		resultVec.insert(resultVec.begin(), temp.begin(), temp.end());
				
				// SELECTION
				model->agents.selectAgents(FLAG, 100, resultSet, filter);
				model->agents.selectAgents(FLAG, 100, resultVec, filter);
				
				// TESTS
				// Now should have 200 (note: there are 201 (0-200, inclusive) that we could have selected
				ASSERT_EQ(200, resultSet.size());
				ASSERT_EQ(200, resultVec.size());
				testAgentIDsBelow(resultSet, resultVec, 1200);
				testSetContains(resultSet, resultVec, temp);
				testLocalNonLocal(resultSet, resultVec, FLAG);
				
				
				// SELECTION IV: Excluding certain agents, but removing them from the result set
				// SETUP
				resultSet.clear();		resultSet.insert(temp.begin(), temp.end());
				resultVec.clear();		resultVec.insert(resultVec.begin(), temp.begin(), temp.end());
				
				// SELECTION
				model->agents.selectAgents(FLAG, 100, resultSet, filter, true);
				model->agents.selectAgents(FLAG, 100, resultVec, filter, true);
				
				// TESTS
				ASSERT_EQ(100, resultSet.size());
				ASSERT_EQ(100, resultVec.size());
				testAgentIDsBelow(resultSet, resultVec, 1200);
				testSetDoesNotContain(resultSet, resultVec, temp);
				testLocalNonLocal(resultSet, resultVec, FLAG);
				
				
				// SELECTION V: Test of the optional 'popSize' parameter
				
				// SETUP
				int expectedSize = (popSize < 100 ? popSize : 100);
				resultSet.clear();
				resultVec.clear();
				
				// SELECTION
				model->agents.selectAgents(FLAG, 100, resultSet, filter, false, popSize);
				model->agents.selectAgents(FLAG, 100, resultVec, filter, false, popSize);
				
				// Now should have the original 100+ up to 100 more
				ASSERT_EQ(expectedSize, resultSet.size());
				ASSERT_EQ(expectedSize, resultVec.size());
				testAgentIDsBelow(resultSet, resultVec, 1200);
				testLocalNonLocal(resultSet, resultVec, FLAG);
			}
		}
	}
}






TEST_F(RSMPITests, SelectLocalOrNonLocalFilteredAgentsByType){
	for(int i = 0; i < 1000; i++){
		AgentId id(1000 + i, rank, i % 3);
		ModelAgent* agent = new ModelAgent(id, 0); // Note: some different types
		model->agents.addAgent(agent);
	}
	
	// Exchange agents
	
	AgentRequest request1(rank);
	AgentCreatorUpdater creator(model);
	
	if(rank == 1){
        for(int i = 0; i < 500; i++){
			AgentId id(1000 + i, 0, i % 3);
			request1.addRequest(id);
		}
	}
	
	// Request the agents
	repast::requestAgents<ModelAgent, ModelAgentContent>(model->agents, request1, *model, creator);
    RepastProcess::instance()->syncAgentStatus<ModelAgent, ModelAgentContent>(model->agents, *model, creator);
	syncAgents<ModelAgentContent> (*model, creator);
	
	set<ModelAgent*> resultSet;
	vector<ModelAgent*> resultVec;

	IsLowNumberedAgent filter;
	int _type = 1;
	
	
	if(rank == 1){
		for (int l = 0; l <= 1; l++){
			repast::SharedContext<ModelAgent>::filterLocalFlag FLAG = repast::SharedContext<ModelAgent>::LOCAL;
			if(l == 1)                                         FLAG = repast::SharedContext<ModelAgent>::NON_LOCAL;

			int popSize = countOf(model->agents.byTypeFilteredBegin(FLAG, _type, filter), model->agents.byTypeFilteredEnd(FLAG, _type, filter));

			// Chance plays a role; do multiple runs
			for(int t = 0; t < 20; t++){
			
				
				// SELECTION I: All Agents
				
				// SETUP
				resultSet.clear();
				resultVec.clear();
				
				// SELECTION
				model->agents.selectAgents(FLAG, resultSet, _type, filter);
				model->agents.selectAgents(FLAG, resultVec, _type, filter);
				
				// Should have received all of the agents
				ASSERT_EQ(popSize, resultSet.size());
				ASSERT_EQ(popSize, resultVec.size());
				testAgentsAreOfType(resultSet, resultVec, _type);
				testAgentIDsBelow(resultSet, resultVec, 1200);
				testLocalNonLocal(resultSet, resultVec, FLAG);
				
				
				// SELECTION II: Specific Number of Agents
				
				// SETUP
				resultSet.clear();
				resultVec.clear();
				
				// SELECTION
				model->agents.selectAgents(FLAG, 10, resultSet, _type, filter);
				model->agents.selectAgents(FLAG, 10, resultVec, _type, filter);
				
				// TESTS
				ASSERT_EQ(10, resultSet.size());
				ASSERT_EQ(10, resultVec.size());
				testAgentsAreOfType(resultSet, resultVec, _type);
				testAgentIDsBelow(resultSet, resultVec, 1200);
				testLocalNonLocal(resultSet, resultVec, FLAG);
				
				
				// SELECTION III: Excluding certain agents, and leaving them in the result set
				
				// SETUP
				vector<ModelAgent*> temp(resultVec.begin(), resultVec.end());
				resultSet.clear();		resultSet.insert(temp.begin(), temp.end());
				resultVec.clear();		resultVec.insert(resultVec.begin(), temp.begin(), temp.end());
				
				// SELECTION
				model->agents.selectAgents(10, resultSet, _type, filter);
				model->agents.selectAgents(10, resultVec, _type, filter);
				
				// TESTS
				// Now should have 20 
				ASSERT_EQ(20, resultSet.size());
				ASSERT_EQ(20, resultVec.size());
				testAgentsAreOfType(resultSet, resultVec, _type);
				testAgentIDsBelow(resultSet, resultVec, 1200);
				testSetContains(resultSet, resultVec, temp);
				
				
				// SELECTION IV: Excluding certain agents, but removing them from the result set
				
				// SETUP
				resultSet.clear();		resultSet.insert(temp.begin(), temp.end());
				resultVec.clear();		resultVec.insert(resultVec.begin(), temp.begin(), temp.end());
				
				// SELECTION
				model->agents.selectAgents(FLAG, 10, resultSet, _type, filter, true);
				model->agents.selectAgents(FLAG, 10, resultVec, _type, filter, true);
				
				// Now should have just 10
				ASSERT_EQ(10, resultSet.size());
				ASSERT_EQ(10, resultVec.size());
				testAgentsAreOfType(resultSet, resultVec, _type);
				testAgentIDsBelow(resultSet, resultVec, 1200);
				testSetDoesNotContain(resultSet, resultVec, temp);
				testLocalNonLocal(resultSet, resultVec, FLAG);
				
				// SELECTION V: Test of the optional 'popSize' parameter
				
				// SETUP
				int expectedSize = (popSize < 100 ? popSize : 100);
				resultSet.clear();
				resultVec.clear();
				
				// SELECTION
				model->agents.selectAgents(FLAG, 100, resultSet, _type, filter, false, popSize);
				model->agents.selectAgents(FLAG, 100, resultVec, _type, filter, false, popSize);
				
				// TESTS
				ASSERT_EQ(expectedSize, resultSet.size());
				ASSERT_EQ(expectedSize, resultVec.size());
				testAgentsAreOfType(resultSet, resultVec, _type);
				testAgentIDsBelow(resultSet, resultVec, 1200);
				testLocalNonLocal(resultSet, resultVec, FLAG);
				
			}
		}
	}
}


