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

/*
 * ObserverTests.cpp
 *
 *  Created on: Jul 30, 2010
 *      Author: nick
 */
#include <gtest/gtest.h>
#include <algorithm>

#include "relogo/Observer.h"
#include "relogo/Patch.h"
#include "Objects.h"
#include "relogo/utility.h"
#include "relogo/WorldCreator.h"

class ObserverTests: public ::testing::Test {

protected:
	repast::relogo::Observer* obs;

public:
	ObserverTests() {
		WorldDefinition def(-50, -100, 51, 101, true, 2);
		def.defineNetwork(true);
		def.defineNetwork(false);
		def.defineNetwork("my_network", true);
		def.defineNetwork("my_undir_network", false);
		WorldCreator creator;
		PatchCreator patchCreator;
		obs = creator.createWorld<MyObserver, MyPatch> (def, std::vector<int>(2, 2), patchCreator);
	}

	~ObserverTests() {
		delete obs;
	}
};

struct MyPatchGetter {

	double operator()(MyPatch* patch) const {
		return patch->val();
	}
};

TEST_F(ObserverTests, TurtleTest)
{

	obs->create<MyTurtle> (10);
	AgentSet<MyTurtle> turtles = obs->get<MyTurtle> ();
	ASSERT_EQ(10, turtles.size());

	const RelogoGridType* grid = obs->grid();
	const RelogoSpaceType* space = obs->space();

	Point<double> spaceCenter(space->dimensions().origin(0) + space->dimensions().extents(0) / 2.0,
			space->dimensions().origin(1) + space->dimensions().extents(1) / 2.0);

	Point<int> gridCenter(doubleCoordToInt(spaceCenter.getX()), doubleCoordToInt(spaceCenter.getY()));

	std::vector<int> gridLocation;
	std::vector<double> spaceLocation;
	for (AgentSet<MyTurtle>::const_as_iterator iter = turtles.begin(); iter != turtles.end(); ++iter) {
		Turtle* turtle = *iter;
		ASSERT_EQ(1, turtle->getId().agentType());

		ASSERT_TRUE(grid->getLocation(turtle, gridLocation));
		ASSERT_EQ(gridCenter, Point<int>(gridLocation));
		ASSERT_EQ(gridCenter.getX(), turtle->pxCor());
		ASSERT_EQ(gridCenter.getY(), turtle->pyCor());

		ASSERT_TRUE(space->getLocation(turtle, spaceLocation));
		ASSERT_EQ(spaceCenter, Point<double>(spaceLocation));
		ASSERT_EQ(spaceCenter.getX(), turtle->xCor());
		ASSERT_EQ(spaceCenter.getY(), turtle->yCor());
	}

}

TEST_F(ObserverTests, WorkerTest)
{

	WorkerCreator creator;
	obs->create<Worker> (2000, creator);
	AgentSet<Worker> workers = obs->get<Worker> ();
	ASSERT_EQ(2000, workers.size());

	const RelogoGridType* grid = obs->grid();
	const RelogoSpaceType* space = obs->space();

	Point<double> spaceCenter(space->dimensions().origin(0) + space->dimensions().extents(0) / 2.0,
			space->dimensions().origin(1) + space->dimensions().extents(1) / 2.0);

	Point<int> gridCenter(doubleCoordToInt(spaceCenter.getX()), doubleCoordToInt(spaceCenter.getY()));

	std::vector<int> gridLocation;
	std::vector<double> spaceLocation;
	for (AgentSet<Worker>::const_as_iterator iter = workers.begin(); iter != workers.end(); ++iter) {
		Worker* worker = *iter;
		//std::cout << "worker: " << worker << std::endl;
		ASSERT_EQ(2, worker->getId().agentType());

		ASSERT_TRUE(grid->getLocation(worker, gridLocation));
		ASSERT_EQ(gridCenter, Point<int>(gridLocation));
		ASSERT_GE(worker->status(), 4);
		ASSERT_LE(worker->status(), 21);

		ASSERT_TRUE(space->getLocation(worker, spaceLocation));
		ASSERT_EQ(spaceCenter, Point<double>(spaceLocation));
	}
}

TEST_F(ObserverTests, TurtleMove)
{

	obs->create<MyTurtle> (10);
	AgentSet<MyTurtle> turtles = obs->get<MyTurtle> ();
	ASSERT_EQ(10, turtles.size());

	Turtle* turtle = turtles[0];
	int ox = turtle->pxCor();
	int oy = turtle->pyCor();

	turtle->xCor(ox + 3);
	AgentSet<MyTurtle> found = obs->turtlesAt<MyTurtle>(ox + 3, oy);
	ASSERT_EQ(1, found.size());
	ASSERT_EQ(turtle, found[0]);

	turtle->yCor(oy + 3);
	found = obs->turtlesAt<MyTurtle>(ox + 3, oy + 3);
	ASSERT_EQ(1, found.size());
	ASSERT_EQ(turtle, found[0]);

	AgentSet<MyTurtle> others = obs->turtlesAt<MyTurtle>(ox, oy);
	ASSERT_EQ(9, others.size());

	turtle->setxy(ox, oy);
	found = obs->turtlesAt<MyTurtle>(ox, oy);
	ASSERT_EQ(10, found.size());

}

TEST_F(ObserverTests, TurtleDirectedLinkTest)
{
	obs->create<MyTurtle> (10);
	AgentSet<MyTurtle> turtles = obs->get<MyTurtle> ();
	ASSERT_EQ(10, turtles.size());

	RelogoLink* link = turtles[0]->inLinkFrom(turtles[1]);
	ASSERT_EQ((void*)0, link);
	link = turtles[0]->inLinkFrom(turtles[1], "my_network");
	ASSERT_EQ((void*)0, link);

	turtles[0]->createLinkFrom(turtles[1]);
	turtles[0]->createLinkFrom(turtles[1], "my_network");
	LinkCreator creator;
	turtles[2]->createLinkFromLC(turtles[0],creator, "my_network");
	turtles[2]->createLinkFromLC(turtles[0], creator);

	link = turtles[0]->inLinkFrom(turtles[1]);
	ASSERT_NE((void*)0, link);
	ASSERT_EQ(turtles[1], link->source());
	ASSERT_EQ(turtles[0], link->target());

	link = turtles[0]->inLinkFrom(turtles[1], "my_network");
	ASSERT_NE((void*)0, link);
	ASSERT_EQ(turtles[1], link->source());
	ASSERT_EQ(turtles[0], link->target());

	link = turtles[2]->inLinkFrom(turtles[0], "my_network");
	ASSERT_NE((void*)0, link);
	ASSERT_EQ(turtles[0], link->source());
	ASSERT_EQ(turtles[2], link->target());
	// creator sets weight to 12
	ASSERT_EQ(12.0, link->weight());

	link = turtles[2]->inLinkFrom(turtles[0]);
	ASSERT_NE((void*)0, link);
	ASSERT_EQ(turtles[0], link->source());
	ASSERT_EQ(turtles[2], link->target());
	// creator sets weight to 12
	ASSERT_EQ(12.0, link->weight());

	turtles[3]->createLinkTo(turtles[7]);
	turtles[3]->createLinkTo(turtles[7], "my_network");
	turtles[7]->createLinkToLC(turtles[3], creator, "my_network");
	turtles[7]->createLinkToLC(turtles[3], creator);

	link = turtles[3]->outLinkTo(turtles[0]);
	ASSERT_EQ((void*)0, link);

	link = turtles[3]->outLinkTo(turtles[7]);
	ASSERT_NE((void*)0, link);
	ASSERT_EQ(turtles[3], link->source());
	ASSERT_EQ(turtles[7], link->target());

	link = turtles[3]->outLinkTo(turtles[7], "my_network");
	ASSERT_NE((void*)0, link);
	ASSERT_EQ(turtles[3], link->source());
	ASSERT_EQ(turtles[7], link->target());

	link = turtles[7]->outLinkTo(turtles[3]);
	ASSERT_NE((void*)0, link);
	ASSERT_EQ(turtles[7], link->source());
	ASSERT_EQ(turtles[3], link->target());
	// creator sets weight to 12
	ASSERT_EQ(12.0, link->weight());

	link = turtles[7]->outLinkTo(turtles[3], "my_network");
	ASSERT_NE((void*)0, link);
	ASSERT_EQ(turtles[7], link->source());
	ASSERT_EQ(turtles[3], link->target());
	// creator sets weight to 12
	ASSERT_EQ(12.0, link->weight());
}

TEST_F(ObserverTests, TurtleNeighbors)
{
	obs->create<MyTurtle> (10);
	AgentSet<MyTurtle> turtles = obs->get<MyTurtle> ();
	ASSERT_EQ(10, turtles.size());

	turtles[0]->createLinkWith(turtles[2]);
	ASSERT_TRUE(turtles[0]->linkNeighborQ(turtles[2]));
	ASSERT_TRUE(turtles[2]->linkNeighborQ(turtles[0]));
	turtles[0]->createLinkWith(turtles[2], "my_undir_network");
	ASSERT_TRUE(turtles[0]->linkNeighborQ(turtles[2], "my_undir_network"));
	ASSERT_TRUE(turtles[2]->linkNeighborQ(turtles[0], "my_undir_network"));

	turtles[1]->createLinkWith(turtles[0]);
	ASSERT_TRUE(turtles[1]->linkNeighborQ(turtles[0]));
	ASSERT_TRUE(turtles[0]->linkNeighborQ(turtles[1]));

	turtles[1]->createLinkWith(turtles[0], "my_undir_network");
	turtles[4]->createLinkWith(turtles[0], "my_undir_network");
	ASSERT_TRUE(turtles[1]->linkNeighborQ(turtles[0], "my_undir_network"));
	ASSERT_TRUE(turtles[0]->linkNeighborQ(turtles[1], "my_undir_network"));

	AgentSet<MyTurtle> set;
	turtles[0]->linkNeighbors(set);
	ASSERT_EQ(2, set.size());
	ASSERT_TRUE(set.end() != find(set.begin(), set.end(), turtles[1]));
	ASSERT_TRUE(set.end() != find(set.begin(), set.end(), turtles[2]));
	set.clear();

	turtles[0]->linkNeighbors(set, "my_undir_network");
	ASSERT_EQ(3, set.size());
	ASSERT_TRUE(set.end() != find(set.begin(), set.end(), turtles[1]));
	ASSERT_TRUE(set.end() != find(set.begin(), set.end(), turtles[2]));
	ASSERT_TRUE(set.end() != find(set.begin(), set.end(), turtles[4]));
	set.clear();

	turtles[1]->linkNeighbors(set);
	ASSERT_EQ(1, set.size());
	ASSERT_EQ(set[0], turtles[0]);
	set.clear();

	turtles[7]->linkNeighbors(set);
	ASSERT_EQ(0, set.size());
}

TEST_F(ObserverTests, TurtleNeighbors2)
{
	obs->create<MyTurtle> (10);
	AgentSet<MyTurtle> turtles = obs->get<MyTurtle> ();
	ASSERT_EQ(10, turtles.size());

	//bool linkNeighborQ(Turtle* turtle, const std::string& name = DEFAULT_UNDIR_NET);
	//void linkNeighbors(AgentSet<AgentType>& out, const std::string& name = DEFAULT_UNDIR_NET);
	//bool inLinkNeighborQ(Turtle* turtle, const std::string& name = DEFAULT_DIR_NET);
	//void inLinkNeighbors(AgentSet<AgentType>& out, const std::string& name = DEFAULT_DIR_NET);
	//bool outLinkNeighborQ(Turtle* turtle, const std::string& name = DEFAULT_DIR_NET);
	//void outLinkNeighbors(AgentSet<AgentType>& out, const std::string& name = DEFAULT_DIR_NET);

	turtles[0]->createLinkFrom(turtles[2]);
	turtles[0]->createLinkFrom(turtles[8]);
	ASSERT_TRUE(turtles[0]->inLinkNeighborQ(turtles[2]));
	ASSERT_FALSE(turtles[2]->inLinkNeighborQ(turtles[0]));
	ASSERT_TRUE(turtles[2]->outLinkNeighborQ(turtles[0]));

	turtles[0]->createLinkFrom(turtles[2], "my_network");
	turtles[0]->createLinkFrom(turtles[8], "my_network");
	turtles[0]->createLinkFrom(turtles[9], "my_network");
	ASSERT_TRUE(turtles[0]->inLinkNeighborQ(turtles[2], "my_network"));
	ASSERT_TRUE(turtles[2]->outLinkNeighborQ(turtles[0], "my_network"));
	ASSERT_TRUE(turtles[0]->inLinkNeighborQ(turtles[8], "my_network"));
	ASSERT_TRUE(turtles[8]->outLinkNeighborQ(turtles[0], "my_network"));
	ASSERT_TRUE(turtles[0]->inLinkNeighborQ(turtles[9], "my_network"));
	ASSERT_TRUE(turtles[9]->outLinkNeighborQ(turtles[0], "my_network"));

	AgentSet<MyTurtle> set;
	turtles[0]->inLinkNeighbors(set);
	ASSERT_EQ(2, set.size());
	ASSERT_TRUE(set.end() != find(set.begin(), set.end(), turtles[2]));
	ASSERT_TRUE(set.end() != find(set.begin(), set.end(), turtles[8]));
	set.clear();

	turtles[0]->inLinkNeighbors(set, "my_network");
	ASSERT_EQ(3, set.size());
	ASSERT_TRUE(set.end() != find(set.begin(), set.end(), turtles[2]));
	ASSERT_TRUE(set.end() != find(set.begin(), set.end(), turtles[8]));
	ASSERT_TRUE(set.end() != find(set.begin(), set.end(), turtles[9]));
	set.clear();

	turtles[9]->createLinkTo(turtles[2]);
	turtles[9]->createLinkTo(turtles[8]);
	ASSERT_TRUE(turtles[2]->inLinkNeighborQ(turtles[9]));
	ASSERT_FALSE(turtles[9]->inLinkNeighborQ(turtles[2]));
	ASSERT_TRUE(turtles[9]->outLinkNeighborQ(turtles[8]));
	ASSERT_TRUE(turtles[9]->outLinkNeighborQ(turtles[2]));

	turtles[9]->outLinkNeighbors(set);
	ASSERT_EQ(2, set.size());
	ASSERT_TRUE(set.end() != find(set.begin(), set.end(), turtles[2]));
	ASSERT_TRUE(set.end() != find(set.begin(), set.end(), turtles[8]));
}

TEST_F(ObserverTests, TurtleUndirectedLinkTest)
{
	obs->create<MyTurtle> (10);
	AgentSet<MyTurtle> turtles = obs->get<MyTurtle> ();
	ASSERT_EQ(10, turtles.size());

	RelogoLink* link = turtles[0]->linkWith(turtles[1]);
	ASSERT_EQ((void*)0, link);
	link = turtles[0]->linkWith(turtles[1], "my_undir_network");
	ASSERT_EQ((void*)0, link);

	turtles[3]->createLinkWith(turtles[7]);
	link = turtles[3]->linkWith(turtles[7]);
	ASSERT_NE((void*)0, link);
	ASSERT_EQ(turtles[3], link->source());
	ASSERT_EQ(turtles[7], link->target());
	ASSERT_EQ(link, turtles[7]->linkWith(turtles[3]));

	turtles[3]->createLinkWith(turtles[7], "my_undir_network");
	link = turtles[3]->linkWith(turtles[7], "my_undir_network");
	ASSERT_NE((void*)0, link);
	ASSERT_EQ(turtles[3], link->source());
	ASSERT_EQ(turtles[7], link->target());
	ASSERT_EQ(link, turtles[7]->linkWith(turtles[3], "my_undir_network"));

	LinkCreator creator;
	turtles[0]->createLinkWithLC(turtles[3], creator, "my_undir_network");
	link = turtles[0]->linkWith(turtles[3], "my_undir_network");
	ASSERT_NE((void*)0, link);
	ASSERT_EQ(turtles[0], link->source());
	ASSERT_EQ(turtles[3], link->target());
	ASSERT_EQ(12.0, link->weight());
	ASSERT_EQ(link, turtles[3]->linkWith(turtles[0], "my_undir_network"));

	turtles[0]->createLinkWithLC(turtles[3], creator);
	link = turtles[0]->linkWith(turtles[3]);
	ASSERT_NE((void*)0, link);
	ASSERT_EQ(turtles[0], link->source());
	ASSERT_EQ(turtles[3], link->target());
	ASSERT_EQ(12.0, link->weight());
	ASSERT_EQ(link, turtles[3]->linkWith(turtles[0]));

	AgentSet<MyTurtle> others;
	others.add(turtles[7]);
	others.add(turtles[8]);
	others.add(turtles[9]);

	turtles[2]->createLinksWith(others);
	for (int i = 7; i < 10; i++) {
		link = turtles[2]->linkWith(turtles[i]);
		ASSERT_NE((void*)0, link);
		ASSERT_EQ(turtles[2], link->source());
		ASSERT_EQ(turtles[i], link->target());
		//ASSERT_EQ(12.0, link->weight());
		ASSERT_EQ(link, turtles[i]->linkWith(turtles[2]));
	}

	turtles[2]->createLinksWith(others, "my_undir_network");
	for (int i = 7; i < 10; i++) {
		link = turtles[2]->linkWith(turtles[i], "my_undir_network");
		ASSERT_NE((void*)0, link);
		ASSERT_EQ(turtles[2], link->source());
		ASSERT_EQ(turtles[i], link->target());
		//ASSERT_EQ(12.0, link->weight());
		ASSERT_EQ(link, turtles[i]->linkWith(turtles[2], "my_undir_network"));
	}

	// make sure that these compile post template instantation
	turtles[4]->createLinksWithLC(others, creator);
	turtles[4]->createLinksWithLC(others, creator, "my_undir_network");

	for (int i = 7; i < 10; i++) {
		link = turtles[4]->linkWith(turtles[i]);
		ASSERT_NE((void*)0, link);
		ASSERT_EQ(turtles[4], link->source());
		ASSERT_EQ(turtles[i], link->target());
		ASSERT_EQ(12.0, link->weight());
		ASSERT_EQ(link, turtles[i]->linkWith(turtles[4]));
	}

	for (int i = 7; i < 10; i++) {
		link = turtles[4]->linkWith(turtles[i], "my_undir_network");
		ASSERT_NE((void*)0, link);
		ASSERT_EQ(turtles[4], link->source());
		ASSERT_EQ(turtles[i], link->target());
		ASSERT_EQ(12.0, link->weight());
		ASSERT_EQ(link, turtles[i]->linkWith(turtles[4], "my_undir_network"));
	}
}

TEST_F(ObserverTests, TurtleSetLinkTest)
{
	obs->create<MyTurtle> (10);
	AgentSet<MyTurtle> turtles = obs->get<MyTurtle> ();
	ASSERT_EQ(10, turtles.size());

	AgentSet<MyTurtle> others;
	others.add(turtles[7]);
	others.add(turtles[8]);
	others.add(turtles[9]);

	turtles[2]->createLinksFrom(others);
	for (int i = 7; i < 10; i++) {
		RelogoLink* link = turtles[2]->inLinkFrom(turtles[i]);
		ASSERT_NE((void*)0, link);
		ASSERT_EQ(turtles[i], link->source());
		ASSERT_EQ(turtles[2], link->target());
	}

	turtles[2]->createLinksFrom(others, "my_network");
	for (int i = 7; i < 10; i++) {
		RelogoLink* link = turtles[2]->inLinkFrom(turtles[i], "my_network");
		ASSERT_NE((void*)0, link);
		ASSERT_EQ(turtles[i], link->source());
		ASSERT_EQ(turtles[2], link->target());
	}

	LinkCreator creator;
	turtles[4]->createLinksFromLC(others, creator);
	for (int i = 7; i < 10; i++) {
		RelogoLink* link = turtles[4]->inLinkFrom(turtles[i]);
		ASSERT_NE((void*)0, link);
		ASSERT_EQ(turtles[i], link->source());
		ASSERT_EQ(turtles[4], link->target());
		ASSERT_EQ(12.0, link->weight());
		link = turtles[i]->outLinkTo(turtles[4]);
		ASSERT_NE((void*)0, link);
		ASSERT_EQ(turtles[i], link->source());
		ASSERT_EQ(turtles[4], link->target());
	}

	turtles[4]->createLinksFromLC(others, creator, "my_network");
	for (int i = 7; i < 10; i++) {
		RelogoLink* link = turtles[4]->inLinkFrom(turtles[i], "my_network");
		ASSERT_NE((void*)0, link);
		ASSERT_EQ(turtles[i], link->source());
		ASSERT_EQ(turtles[4], link->target());
		ASSERT_EQ(12.0, link->weight());
		link = turtles[i]->outLinkTo(turtles[4], "my_network");
		ASSERT_NE((void*)0, link);
		ASSERT_EQ(turtles[i], link->source());
		ASSERT_EQ(turtles[4], link->target());
	}

	turtles[2]->createLinksTo(others);
	for (int i = 7; i < 10; i++) {
		RelogoLink* link = turtles[i]->inLinkFrom(turtles[2]);
		ASSERT_NE((void*)0, link);
		ASSERT_EQ(turtles[2], link->source());
		ASSERT_EQ(turtles[i], link->target());
	}

	turtles[2]->createLinksTo(others, "my_network");
	for (int i = 7; i < 10; i++) {
		RelogoLink* link = turtles[i]->inLinkFrom(turtles[2], "my_network");
		ASSERT_NE((void*)0, link);
		ASSERT_EQ(turtles[2], link->source());
		ASSERT_EQ(turtles[i], link->target());
	}

	turtles[4]->createLinksToLC(others, creator);
	for (int i = 7; i < 10; i++) {
		RelogoLink* link = turtles[i]->inLinkFrom(turtles[4]);
		ASSERT_NE((void*)0, link);
		ASSERT_EQ(turtles[4], link->source());
		ASSERT_EQ(turtles[i], link->target());
		ASSERT_EQ(12.0, link->weight());
		link = turtles[4]->outLinkTo(turtles[i]);
		ASSERT_NE((void*)0, link);
		ASSERT_EQ(turtles[4], link->source());
		ASSERT_EQ(turtles[i], link->target());
	}

	turtles[4]->createLinksToLC(others, creator, "my_network");
	for (int i = 7; i < 10; i++) {
		RelogoLink* link = turtles[i]->inLinkFrom(turtles[4], "my_network");
		ASSERT_NE((void*)0, link);
		ASSERT_EQ(turtles[4], link->source());
		ASSERT_EQ(turtles[i], link->target());
		ASSERT_EQ(12.0, link->weight());
		link = turtles[4]->outLinkTo(turtles[i], "my_network");
		ASSERT_NE((void*)0, link);
		ASSERT_EQ(turtles[4], link->source());
		ASSERT_EQ(turtles[i], link->target());
	}
}

void testPatches(const RelogoGridType* grid, const RelogoSpaceType* space) {
	GridDimensions dims = grid->dimensions();
	std::vector<RelogoAgent*> patches;
	for (int x = dims.origin(0); x < dims.origin(0) + dims.extents(0); ++x) {
		for (int y = dims.origin(1); y < dims.origin(1) + dims.extents(1); ++y) {
			grid->getObjectsAt(Point<int> (x, y), patches);
			ASSERT_EQ(1, patches.size());
			MyPatch* patch = static_cast<MyPatch*>(patches[0]);
			ASSERT_EQ(0, patch->getId().agentType());
			ASSERT_EQ(x, patch->pxCor());
			ASSERT_EQ(y, patch->pyCor());

			patches.clear();

			space->getObjectsAt(Point<double>(x, y), patches);
			ASSERT_EQ(1, patches.size());
			MyPatch* patch2 = static_cast<MyPatch*>(patches[0]);
			ASSERT_EQ(patch, patch2);
			patches.clear();
		}
	}
}

TEST_F(ObserverTests, SetupTest)
{
	const RelogoGridType* grid = obs->grid();
	const RelogoSpaceType* space = obs->space();

	int rank = obs->rank();
	if (rank == 0) {
		GridDimensions expected(Point<int> (-50, -100), Point<int> (51, 101));
		ASSERT_EQ(expected, grid->dimensions());
		ASSERT_EQ(expected, space->dimensions());

	} else if (rank == 1) {
		GridDimensions expected(Point<int>(1, -100),Point<int>(51, 101));
		ASSERT_EQ(expected, grid->dimensions());
		ASSERT_EQ(expected, space->dimensions());
	} else if (rank == 2) {
		GridDimensions expected(Point<int>(-50, 1),Point<int>(51, 101));
		ASSERT_EQ(expected, grid->dimensions());
		ASSERT_EQ(expected, space->dimensions());
	} else if (rank == 3) {
		GridDimensions expected(Point<int>(1, 1),Point<int>(51, 101));
		ASSERT_EQ(expected, grid->dimensions());
		ASSERT_EQ(expected, space->dimensions());
	}

	testPatches(grid, space);

}

TEST(WorldDef, WorldDefTests)
{
	WorldDefinition def(-50, -100, 50, 100, true, 2);
	ASSERT_EQ(-50, def.minX());
	ASSERT_EQ(-100, def.minY());
	ASSERT_EQ(50, def.maxX());
	ASSERT_EQ(100, def.maxY());

}

TEST_F(ObserverTests, TurtleMoveByHeadingTest)
{
	obs->create<MyTurtle> (10);
	AgentSet<MyTurtle> turtles = obs->get<MyTurtle> ();
	ASSERT_EQ(10, turtles.size());

	ASSERT_TRUE(turtles[0]->canMoveQ(200));

	MyTurtle* turtle = turtles[0];
	// north
	turtle->heading(0);
	double x = turtle->xCor();
	double y = turtle->yCor();

	turtle->forward(2);
	ASSERT_NEAR(x, turtle->xCor(), .00001);
	ASSERT_NEAR(y + 2, turtle->yCor(), 00001);
	AgentSet<MyTurtle> found = obs->turtlesAt<MyTurtle>(turtle->pxCor(), turtle->pyCor());
	ASSERT_EQ(1, found.size());
	ASSERT_EQ(turtle, found[0]);

	// east
	turtle->heading(90);
	turtle->forward(2);
	ASSERT_NEAR(x + 2, turtle->xCor(), .00001);
	ASSERT_NEAR(y + 2, turtle->yCor(), 00001);
	found = obs->turtlesAt<MyTurtle>(turtle->pxCor(), turtle->pyCor());
	ASSERT_EQ(1, found.size());
	ASSERT_EQ(turtle, found[0]);

	turtle->backward(2);
	ASSERT_NEAR(x, turtle->xCor(), .00001);
	ASSERT_NEAR(y + 2, turtle->yCor(), 00001);
	found = obs->turtlesAt<MyTurtle>(turtle->pxCor(), turtle->pyCor());
	ASSERT_EQ(1, found.size());
	ASSERT_EQ(turtle, found[0]);
}

TEST_F(ObserverTests, HillTests)
{
	obs->create<MyTurtle> (10);
	AgentSet<MyTurtle> turtles = obs->get<MyTurtle> ();
	ASSERT_EQ(10, turtles.size());

	MyTurtle* turtle = turtles[0];
	turtle->heading(0);
	turtle->forward(2);

	MyPatch* patch = turtle->patchHere<MyPatch>();
	patch->val(20);
	MyPatchGetter getter;
	turtle->uphill<MyPatch>(getter);
	ASSERT_EQ(patch, turtle->patchHere<MyPatch>());

	MyPatch* west = obs->patchAt<MyPatch>(patch->pxCor() - 1, patch->pyCor());
	west->val(200);
	turtle->uphill<MyPatch>(getter);
	ASSERT_EQ(west, turtle->patchHere<MyPatch>());

	MyPatch* ne = obs->patchAt<MyPatch>(west->pxCor() + 1, patch->pyCor() + 1);
	ne->val(1000);
	// don't move because the highest is ne, not in 4 cardinal dirs
	turtle->uphill4<MyPatch>(getter);
	ASSERT_EQ(west, turtle->patchHere<MyPatch>());

	MyPatch* north = obs->patchAt<MyPatch>(west->pxCor(), west->pyCor() + 1);
	north->val(1000);
	turtle->uphill4<MyPatch>(getter);
	ASSERT_EQ(north, turtle->patchHere<MyPatch>());

	ne->val(-100);
	turtle->downhill<MyPatch>(getter);
	ASSERT_EQ(ne, turtle->patchHere<MyPatch>());

	north->val(-200);
	turtle->downhill4<MyPatch>(getter);
	ASSERT_EQ(north, turtle->patchHere<MyPatch>());

	MyPatch* se = obs->patchAt<MyPatch>(north->pxCor() + 1, north->pyCor() - 1);
	se->val(-10000);
	// shouldn't move
	turtle->downhill4<MyPatch>(getter);
	ASSERT_EQ(north, turtle->patchHere<MyPatch>());
}

TEST_F(ObserverTests, PatchLocationTest)
{
	obs->create<MyTurtle> (10);
	AgentSet<MyTurtle> turtles = obs->get<MyTurtle> ();
	ASSERT_EQ(10, turtles.size());

	const RelogoSpaceType* space = obs->space();
	Point<double> spaceCenter(space->dimensions().origin(0) + space->dimensions().extents(0) / 2.0,
			space->dimensions().origin(1) + space->dimensions().extents(1) / 2.0);
	Point<int> gridCenter(doubleCoordToInt(spaceCenter.getX()), doubleCoordToInt(spaceCenter.getY()));

	int x = gridCenter[0] + 1;
	int y = gridCenter[1] - 1;
	Patch* patch = obs->patchAt(x, y);
	ASSERT_EQ(x, patch->pxCor());
	ASSERT_EQ(y, patch->pyCor());

	MyPatch* myPatch = turtles[0]->patchHere<MyPatch>();
	ASSERT_EQ(turtles[0]->pxCor(), myPatch->pxCor());
	ASSERT_EQ(turtles[0]->pyCor(), myPatch->pyCor());

	turtles[0]->setxy(x, y);
	myPatch = turtles[0]->patchAt<MyPatch>(4, -5);
	x = x + 4;
	y = y - 5;
	ASSERT_EQ(x, myPatch->pxCor());
	ASSERT_EQ(y, myPatch->pyCor());

	turtles[2]->moveTo(myPatch);
	ASSERT_EQ(myPatch->xCor(), turtles[2]->xCor());
	ASSERT_EQ(myPatch->yCor(), turtles[2]->yCor());

	turtles[2]->moveTo(turtles[0]);
	ASSERT_EQ(turtles[0]->xCor(), turtles[2]->xCor());
	ASSERT_EQ(turtles[0]->yCor(), turtles[2]->yCor());

	AgentSet<MyPatch> nghs = myPatch->neighbors<MyPatch>();
	ASSERT_EQ(8, nghs.size());

	nghs = myPatch->neighbors4<MyPatch>();
	ASSERT_EQ(4, nghs.size());
}

TEST_F(ObserverTests, GetXTests)
{
	obs->create<MyTurtle> (10);
	AgentSet<MyTurtle> turtles = obs->get<MyTurtle> ();
	ASSERT_EQ(10, turtles.size());

	turtles.clear();
	obs->get<MyTurtle>(turtles);
	ASSERT_EQ(10, turtles.size());

	AgentSet<MyPatch> patches = obs->patches<MyPatch>();
	ASSERT_EQ(5151, patches.size());

	patches.clear();
	obs->patches<MyPatch>(patches);
	ASSERT_EQ(5151, patches.size());

	std::set<int> expected;
	for (int i = 0; i < 4; i++) {
		int x = turtles[i]->pxCor();
		int y = turtles[i]->pyCor();
		expected.insert(turtles[i]->getId().id());
		turtles[i]->setxy(x + 4, y + 4);
	}

	AgentSet<MyTurtle> set;
	turtles[0]->turtlesHere<MyTurtle>(set);
	ASSERT_EQ(4, set.size());
	for (int i = 0; i < 4; i++) {
		MyTurtle* turtle = set[i];
		ASSERT_EQ(1, expected.erase(turtle->getId().id()));
	}
	ASSERT_EQ(0, expected.size());
}

TEST_F(ObserverTests, InRadius)
{
	obs->create<MyTurtle> (10);
	AgentSet<MyTurtle> turtles = obs->get<MyTurtle> ();
	ASSERT_EQ(10, turtles.size());
	std::set<int> expected;

	int x = turtles[0]->pxCor();
	int y = turtles[0]->pyCor();
	turtles[0]->setxy(x + 5, y + 5);
	turtles[1]->setxy(x + 3, y + 3);

	AgentSet<MyTurtle> outSet;
	expected.insert(turtles[0]->getId().id());
	expected.insert(turtles[1]->getId().id());

	AgentSet<RelogoAgent> agents;
	agents.addAll(turtles.begin(), turtles.end());

	turtles[0]->inRadius<MyTurtle>(agents, 4, outSet);
	ASSERT_EQ(2, outSet.size());
	for (int i = 0; i < 2; i++) {
		ASSERT_EQ(1, expected.erase(outSet[i]->getId().id()));
	}
	ASSERT_EQ(0, expected.size());
}

TEST_F(ObserverTests, PatchAtHeadingTests)
{
	obs->create<MyTurtle> (10);
	AgentSet<MyTurtle> turtles = obs->get<MyTurtle> ();
	ASSERT_EQ(10, turtles.size());

	MyTurtle* t = turtles[0];
	double x = t->xCor();
	double y = t->yCor();
	// 0 is north
	MyPatch* patch = t->patchAtHeadingAndDistance<MyPatch>(0, 2);
	if (x < 0) {
		// + 1 is necesasry because heading of 0 results is small amount
		// of displacement which causes -24.5 to be less than -24.5
		// so it rounds down to -24. NOTE: relogo does this as well
		// so expected behavior.
		ASSERT_EQ(doubleCoordToInt(x + 1), patch->pxCor());
	} else {
		ASSERT_EQ(doubleCoordToInt(x), patch->pxCor());
	}

	ASSERT_EQ(doubleCoordToInt(y + 2), patch->pyCor());

	// west
	patch = t->patchAtHeadingAndDistance<MyPatch>(270, 2);
	ASSERT_EQ(doubleCoordToInt(x - 2), patch->pxCor());
	// TODO this fails because of a small amount of displacement on the
	// y axis resulting in a round down rather than up.
	ASSERT_EQ(doubleCoordToInt(y), patch->pyCor());

	// north
	t->heading(0);
	// so should be west of t
	patch = t->patchLeftAndAhead<MyPatch>(90, 2);
	ASSERT_EQ(doubleCoordToInt(x - 2), patch->pxCor());
	// TODO this fails because of a small amount of displacement on the
	// y axis resulting in a round down rather than up.
	ASSERT_EQ(doubleCoordToInt(y), patch->pyCor());

	// should be south of t
	patch = t->patchRightAndAhead<MyPatch>(180, 2);
	// TODO can fail because of small displacement on x axis
	// so rounds down rather than up
	ASSERT_EQ(doubleCoordToInt(x), patch->pxCor());
	ASSERT_EQ(doubleCoordToInt(y - 2), patch->pyCor());

}

TEST_F(ObserverTests, XOnTests)
{
	obs->create<MyTurtle> (10);
	AgentSet<MyTurtle> turtles = obs->get<MyTurtle> ();
	ASSERT_EQ(10, turtles.size());
	std::set<int> expected;

	for (int i = 0; i < 2; i++) {
		int x = turtles[i]->pxCor();
		int y = turtles[i]->pyCor();
		expected.insert(turtles[i]->getId().id());
		turtles[i]->setxy(x + 4, y + 4);
	}

	for (int i = 2; i < 5; i++) {
		int x = turtles[i]->pxCor();
		int y = turtles[i]->pyCor();
		expected.insert(turtles[i]->getId().id());
		turtles[i]->setxy(x + 6, y + 6);
	}

	AgentSet<RelogoAgent> agentSet;
	// gets the agents at in the patches where 0 is and the patch at px + 6, py + 6
	agentSet.add(turtles[0]);
	// add this in here to make sure no duplicates
	agentSet.add(turtles[1]);
	agentSet.add(obs->patchAt(turtles[2]->pxCor(), turtles[2]->pyCor()));

	AgentSet<MyTurtle> outSet;
	obs->turtlesOn(agentSet, outSet);
	ASSERT_EQ(5, outSet.size());
	for (int i = 0; i < 5; i++) {
		MyTurtle* turtle = outSet[i];
		ASSERT_EQ(1, expected.erase(turtle->getId().id()));
	}
	ASSERT_EQ(0, expected.size());

	outSet.clear();
	for (int i = 0; i < 2; i++) {
		expected.insert(turtles[i]->getId().id());
	}

	obs->turtlesOn(turtles[0], outSet);
	ASSERT_EQ(2, outSet.size());
	for (int i = 0; i < 2; i++) {
		MyTurtle* turtle = outSet[i];
		ASSERT_EQ(1, expected.erase(turtle->getId().id()));
	}
	ASSERT_EQ(0, expected.size());

	outSet.clear();
	for (int i = 2; i < 5; i++) {
		expected.insert(turtles[i]->getId().id());
	}
	obs->turtlesOn(obs->patchAt(turtles[2]->pxCor(), turtles[2]->pyCor()), outSet);
	ASSERT_EQ(3, outSet.size());
	for (int i = 0; i < 3; i++) {
		MyTurtle* turtle = outSet[i];
		ASSERT_EQ(1, expected.erase(turtle->getId().id()));
	}
	ASSERT_EQ(0, expected.size());

}

