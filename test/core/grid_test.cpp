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
 * grid.cpp
 *
 *  Created on: Jul 19, 2010
 *      Author: nick
 */

#include "repast_hpc/Spaces.h"
#include "repast_hpc/Context.h"
#include "repast_hpc/Moore2DGridQuery.h"
#include "repast_hpc/VN2DGridQuery.h"
#include "test.h"

#include <gtest/gtest.h>

using namespace repast;
using namespace std;

TEST(GridTest, GridFill)
{
	Context<TestAgent> context;

	GridDimensions dims(Point<double> (0, 0), Point<double> (5, 10));
	Spaces<TestAgent>::MultipleStrictDiscreteSpace* grid = new Spaces<TestAgent>::MultipleStrictDiscreteSpace("grid", dims);
	context.addProjection(grid);

	int id = 0;
	for (int x = 0; x < 5; ++x) {
		for (int y = 0; y < 10; ++y) {
			TestAgent* agent = new TestAgent(id, 0, 0);
			id++;
			context.addAgent(agent);
			grid->moveTo(agent->getId(), Point<int> (x, y));
		}
	}

	std::vector<TestAgent*> out;
	for (int x = 0; x < 5; ++x) {
		for (int y = 0; y < 10; ++y) {
			grid->getObjectsAt(Point<int> (x, y), out);
			ASSERT_EQ(1, out.size());
			out.clear();
		}
	}

	TestAgent* agent = grid->getObjectAt(Point<int>(1, 1));
	ASSERT_TRUE(agent != 0);
	grid->moveTo(agent->getId(), Point<int>(1, 1));
	agent = grid->getObjectAt(Point<int>(1, 1));
	ASSERT_TRUE(agent != 0);

	out.clear();
	grid->getObjectsAt(Point<int>(1, 1), out);
	ASSERT_EQ(1, out.size());
}

TEST(GridTest, GridSingleOcc)
{
	Context<TestAgent> context;

	GridDimensions dims(Point<double> (20, 30));
	//BaseGrid<TestAgent, SingleOccupancy<TestAgent, int> , StrictBorders, SimpleAdder<TestAgent>,int >* grid
	//= new BaseGrid<TestAgent, SingleOccupancy<TestAgent, int> , StrictBorders, SimpleAdder<TestAgent>,int > ("grid", dims);
	Spaces<TestAgent>::SingleStrictDiscreteSpace* grid = new Spaces<TestAgent>::SingleStrictDiscreteSpace("grid", dims);

	context.addProjection(grid);

	for (int i = 0; i < 10; i++) {
		TestAgent* agent = new TestAgent(i, 0, 0);
		context.addAgent(agent);
	}

	ASSERT_EQ(0, grid->size());

	AgentId id(0, 0, 0);
	TestAgent* zero = context.getAgent(id);
	std::vector<int> out;

	ASSERT_TRUE(!grid->getLocation(zero, out));

	bool res = grid->moveTo(id, Point<int>(0, 0));
	ASSERT_TRUE(res);
	ASSERT_TRUE(grid->getLocation(zero, out));
	ASSERT_EQ(0, out[0]);
	ASSERT_EQ(0, out[0]);

	std::vector<int> in;
	in.push_back(10);
	in.push_back(15);
	res = grid->moveTo(zero, in);

	ASSERT_TRUE(res);
	ASSERT_TRUE(grid->getLocation(zero, out));

	ASSERT_EQ(10, out[0]);
	ASSERT_EQ(15, out[1]);

	Point<int> pt(out);
	TestAgent* found = grid->getObjectAt(pt);
	ASSERT_EQ(zero, found);

	std::vector<TestAgent*> agentOut;
	grid->getObjectsAt(pt, agentOut);
	ASSERT_EQ(1, agentOut.size());
	ASSERT_EQ(zero, agentOut[0]);

	pt = Point<int>(1, 2);
	ASSERT_EQ(NULL, grid->getObjectAt(pt));

	// moveTo after first move
	in[0] = 1;
	in[1] = 2;
	res = grid->moveTo(zero, in);
	ASSERT_TRUE(res);
	ASSERT_TRUE(grid->getLocation(zero, out));
	ASSERT_EQ(1, out[0]);
	ASSERT_EQ(2, out[1]);

	pt = Point<int>(out);
	found = grid->getObjectAt(pt);
	ASSERT_EQ(zero, found);

	// make sure nothing at old location
	pt = Point<int>(10, 15);
	ASSERT_EQ(NULL, grid->getObjectAt(pt));

	// try to put outside bounds
	try {
		in[0] = -1;
		in[1] = -1;
		grid->moveTo(zero, in);
		ASSERT_TRUE(false);
	} catch (out_of_range ex) {
		ASSERT_TRUE(true);
	}

	// try to put outside bounds
	try {
		in[0] = 20;
		in[1] = 30;
		grid->moveTo(zero, in);
		ASSERT_TRUE(false);
	} catch (out_of_range ex) {
		ASSERT_TRUE(true);
	}

	// try to put outside bounds
	try {
		in[0] = 19;
		in[1] = 30;
		grid->moveTo(zero, in);
		ASSERT_TRUE(false);
	} catch (out_of_range ex) {
		ASSERT_TRUE(true);
	}

	// try to put outside bounds
	try {
		in[0] = 20;
		in[1] = 29;
		grid->moveTo(zero, in);
		ASSERT_TRUE(false);
	} catch (out_of_range ex) {
		ASSERT_TRUE(true);
	}

	// put two where zero is, should be false
	id = AgentId(2, 0, 0);
	TestAgent* two = context.getAgent(id);
	pt = Point<int>(1, 2);
	ASSERT_TRUE(!grid->moveTo(two, pt));
	ASSERT_EQ(1, grid->size());

	pt = Point<int>(2, 2);
	grid->moveTo(two, pt);
	ASSERT_EQ(2, grid->size());
}

TEST(GridTest, SpaceMultOcc)
{
	Context<TestAgent> context;

	GridDimensions dims(Point<double> (20, 30));
	Spaces<TestAgent>::MultipleStrictContinuousSpace* space = new Spaces<TestAgent>::MultipleStrictContinuousSpace("space", dims);

	context.addProjection(space);

	for (int i = 0; i < 10; i++) {
		TestAgent* agent = new TestAgent(i, 0, 0);
		context.addAgent(agent);
	}

	ASSERT_EQ(0, space->size());

	AgentId id(0, 0, 0);
	TestAgent* zero = context.getAgent(id);
	std::vector<double> out;

	ASSERT_TRUE(!space->getLocation(zero, out));

	for (int i = 0; i < 10; i++) {
		//space->moveTo()
	}

	// moveTo for first insertion
	std::vector<double> in;
	in.push_back(10.5);
	in.push_back(15.7);
	bool res = space->moveTo(zero, in);
	ASSERT_TRUE(res);

	res = space->moveTo(context.getAgent(AgentId(1, 0, 0)), in);
	ASSERT_TRUE(res);
	ASSERT_TRUE(space->getLocation(zero, out));
	ASSERT_TRUE(space->getLocation(context.getAgent(AgentId(1, 0, 0)), out));

	std::vector<TestAgent*> agents;
	space->getObjectsAt(Point<double>(out), agents);
	ASSERT_EQ(2, agents.size());

}

TEST(GridTest, SpaceSingleOcc)
{
	Context<TestAgent> context;

	GridDimensions dims(Point<double> (20, 30));
	Spaces<TestAgent>::SingleStrictContinuousSpace* space = new Spaces<TestAgent>::SingleStrictContinuousSpace("space", dims);

	context.addProjection(space);

	for (int i = 0; i < 10; i++) {
		TestAgent* agent = new TestAgent(i, 0, 0);
		context.addAgent(agent);
	}

	ASSERT_EQ(0, space->size());

	AgentId id(0, 0, 0);
	TestAgent* zero = context.getAgent(id);
	std::vector<double> out;

	ASSERT_TRUE(!space->getLocation(zero, out));

	// moveTo for first insertion
	std::vector<double> in;
	in.push_back(10.5);
	in.push_back(15.7);
	bool res = space->moveTo(zero, in);

	ASSERT_TRUE(res);
	ASSERT_TRUE(space->getLocation(zero, out));

	ASSERT_EQ(10.5, out[0]);
	ASSERT_EQ(15.7, out[1]);

	Point<double> pt(out);
	TestAgent* found = space->getObjectAt(pt);
	ASSERT_EQ(zero, found);

	std::vector<TestAgent*> agentOut;
	space->getObjectsAt(pt, agentOut);
	ASSERT_EQ(1, agentOut.size());
	ASSERT_EQ(zero, agentOut[0]);

	pt = Point<double>(1, 2);
	ASSERT_EQ(NULL, space->getObjectAt(pt));

	// moveTo after first move
	in[0] = 1;
	in[1] = 2;
	res = space->moveTo(zero, in);
	ASSERT_TRUE(res);
	ASSERT_TRUE(space->getLocation(zero, out));
	ASSERT_EQ(1, out[0]);
	ASSERT_EQ(2, out[1]);

	pt = Point<double>(out);
	found = space->getObjectAt(pt);
	ASSERT_EQ(zero, found);

	// make sure nothing at old location
	pt = Point<double>(10.5, 15.7);
	ASSERT_EQ(NULL, space->getObjectAt(pt));

	// try to put outside bounds
	try {
		in[0] = -1;
		in[1] = -1;
		space->moveTo(zero, in);
		ASSERT_TRUE(false);
	} catch (out_of_range ex) {
		ASSERT_TRUE(true);
	}

	// try to put outside bounds
	try {
		in[0] = 20;
		in[1] = 30;
		space->moveTo(zero, in);
		ASSERT_TRUE(false);
	} catch (out_of_range ex) {
		ASSERT_TRUE(true);
	}

	// try to put outside bounds
	try {
		in[0] = 19;
		in[1] = 30;
		space->moveTo(zero, in);
		ASSERT_TRUE(false);
	} catch (out_of_range ex) {
		ASSERT_TRUE(true);
	}

	// try to put outside bounds
	try {
		in[0] = 20;
		in[1] = 29;
		space->moveTo(zero, in);
		ASSERT_TRUE(false);
	} catch (out_of_range ex) {
		ASSERT_TRUE(true);
	}

	// put two where zero is, should be false
	id = AgentId(2, 0, 0);
	TestAgent* two = context.getAgent(id);
	pt = Point<double>(1, 2);
	ASSERT_TRUE(!space->moveTo(two, pt));
	ASSERT_EQ(1, space->size());

	pt = Point<double>(2, 2);
	space->moveTo(two, pt);
	ASSERT_EQ(2, space->size());
}

TEST(GridTest, GridSingleOccNonZeroOrig)
{
	Context<TestAgent> context;
	// runs from -5 to 5, and -10 to 5
	GridDimensions dims(Point<double> (-5, -10), Point<double> (10, 15));
	Spaces<TestAgent>::SingleStrictDiscreteSpace* grid = new Spaces<TestAgent>::SingleStrictDiscreteSpace("grid", dims);

	context.addProjection(grid);

	for (int i = 0; i < 10; i++) {
		TestAgent* agent = new TestAgent(i, 0, 0);
		context.addAgent(agent);
	}

	ASSERT_EQ(0, grid->size());

	AgentId id(0, 0, 0);
	TestAgent* zero = context.getAgent(id);
	std::vector<int> out;

	ASSERT_TRUE(!grid->getLocation(zero, out));

	// moveTo for first insertion
	std::vector<int> in;
	in.push_back(-3);
	in.push_back(-5);
	bool res = grid->moveTo(zero, in);

	ASSERT_TRUE(res);
	ASSERT_TRUE(grid->getLocation(zero, out));

	ASSERT_EQ(-3, out[0]);
	ASSERT_EQ(-5, out[1]);
}

TEST(GridTest, GridVectorMove)
{
	Context<TestAgent> context;

	GridDimensions dims(Point<double> (15, 8, 14));
	Spaces<TestAgent>::SingleStrictDiscreteSpace* grid = new Spaces<TestAgent>::SingleStrictDiscreteSpace("grid", dims);
	context.addProjection(grid);

	for (int i = 0; i < 10; i++) {
		TestAgent* agent = new TestAgent(i, 0, 0);
		context.addAgent(agent);
	}

	TestAgent* zero = context.getAgent(AgentId(0, 0, 0));
	ASSERT_TRUE(grid->moveTo(zero, Point<int>(3, 4, 5)));
	std::vector<int> out;
	grid->getLocation(zero, out);
	ASSERT_EQ(3, out[0]);
	ASSERT_EQ(4, out[1]);
	ASSERT_EQ(5, out[2]);

	Point<double> north(toRadians(90), 0, 0);
	pair<bool, Point<int> > result = grid->moveByVector(zero, 1, north.coords());
	ASSERT_TRUE(result.first);
	ASSERT_EQ(Point<int>(3, 5, 5), result.second);

	Point<double> east(0, 0, 0);
	result = grid->moveByVector(zero, 1, east.coords());
	ASSERT_TRUE(result.first);
	ASSERT_EQ(Point<int>(4, 5, 5), result.second);

	Point<double> west(toRadians(180), 0, 0);
	result = grid->moveByVector(zero, 1, west.coords());
	ASSERT_TRUE(result.first);
	ASSERT_EQ(Point<int>(3, 5, 5), result.second);

	Point<double> south(toRadians(270), 0, 0);
	result = grid->moveByVector(zero, 1, south.coords());
	ASSERT_TRUE(result.first);
	ASSERT_EQ(Point<int>(3, 4, 5), result.second);

	TestAgent* agent = grid->getObjectAt(Point<int>(3, 4, 5));
	ASSERT_EQ(zero, agent);

	result = grid->moveByDisplacement(zero, Point<int>(-2, 3, -1).coords());
	ASSERT_EQ(Point<int>(1, 7, 4), result.second);
	ASSERT_TRUE(result.first);
	agent = grid->getObjectAt(Point<int>(1, 7, 4));
	ASSERT_EQ(zero, agent);
}

TEST(GridTest, SpaceVectorMove)
{
	Context<TestAgent> context;

	GridDimensions dims(Point<double> (15, 10, 14));
	Spaces<TestAgent>::SingleStrictContinuousSpace* grid = new Spaces<TestAgent>::SingleStrictContinuousSpace("grid", dims);
	context.addProjection(grid);

	for (int i = 0; i < 10; i++) {
		TestAgent* agent = new TestAgent(i, 0, 0);
		context.addAgent(agent);
	}

	TestAgent* zero = context.getAgent(AgentId(0, 0, 0));
	ASSERT_TRUE(grid->moveTo(zero, Point<double>(3.5, 4.5, 5.5)));
	std::vector<double> out;
	grid->getLocation(zero, out);
	ASSERT_EQ(3.5, out[0]);
	ASSERT_EQ(4.5, out[1]);
	ASSERT_EQ(5.5, out[2]);

	Point<double> north(toRadians(90), 0, 0);
	pair<bool, Point<double> > result = grid->moveByVector(zero, 1, north.coords());
	ASSERT_TRUE(result.first);
	ASSERT_NEAR(3.5, result.second[0], .0001);
	ASSERT_NEAR(5.5, result.second[1], .0001);
	ASSERT_NEAR(5.5, result.second[2], .0001);

	Point<double> east(0, 0, 0);
	result = grid->moveByVector(zero, 1, east.coords());
	ASSERT_TRUE(result.first);
	ASSERT_NEAR(4.5, result.second[0], .0001);
	ASSERT_NEAR(5.5, result.second[1], .0001);
	ASSERT_NEAR(5.5, result.second[2], .0001);

	Point<double> west(toRadians(180), 0, 0);
	result = grid->moveByVector(zero, 1, west.coords());
	ASSERT_TRUE(result.first);
	ASSERT_NEAR(3.5, result.second[0], .0001);
	ASSERT_NEAR(5.5, result.second[1], .0001);
	ASSERT_NEAR(5.5, result.second[2], .0001);

	Point<double> south(toRadians(270), 0, 0);
	result = grid->moveByVector(zero, 1, south.coords());
	ASSERT_TRUE(result.first);
	ASSERT_NEAR(3.5, result.second[0], .0001);
	ASSERT_NEAR(4.5, result.second[1], .0001);
	ASSERT_NEAR(5.5, result.second[2], .0001);

	TestAgent* agent = grid->getObjectAt(result.second);
	ASSERT_EQ(zero, agent);

	result = grid->moveByDisplacement(zero, Point<double>(-2.5, 3.5, -1.5).coords());
	ASSERT_TRUE(result.first);
	ASSERT_NEAR(1.0, result.second[0], .0001);
	ASSERT_NEAR(8, result.second[1], .0001);
	ASSERT_NEAR(4.0, result.second[2], .0001);
	ASSERT_TRUE(result.first);

	agent = grid->getObjectAt(result.second);
	ASSERT_EQ(zero, agent);
}

TEST(GridTest, MooreQueries)
{
	Context<TestAgent> context;

	GridDimensions dims(Point<double> (10, 10));
	Spaces<TestAgent>::MultipleStrictDiscreteSpace* grid = new Spaces<TestAgent>::MultipleStrictDiscreteSpace("grid", dims);
	context.addProjection(grid);

	std::set<int> ids;
	for (int i = 0; i < 25; i++) {
		TestAgent* agent = new TestAgent(i, 0, 0);
		context.addAgent(agent);
		ids.insert(i);
	}

	int id = 0;
	for (int x = 0; x < 5; x++) {
		for (int y = 0; y < 5; y++) {
			grid->moveTo(AgentId(id++, 0, 0), Point<int> (x, y));
		}
	}

	// should get everyone in the grid
	Moore2DGridQuery<TestAgent> mooreQuery(grid);
	std::vector<TestAgent*> out;
	mooreQuery.query(Point<int> (2, 2), 2, true, out);
	set<int> expected(ids);
	ASSERT_EQ(25, out.size());
	for (size_t i = 0; i < out.size(); i++) {
		TestAgent* agent = out[i];
		ASSERT_EQ(1, expected.erase(agent->getId().id()));
	}

	expected.insert(ids.begin(), ids.end());
	expected.erase(12);
	out.clear();
	mooreQuery.query(Point<int>(2, 2), 2, false, out);
	ASSERT_EQ(24, out.size());
	for (size_t i = 0; i < out.size(); i++) {
		TestAgent* agent = out[i];
		ASSERT_EQ(1, expected.erase(agent->getId().id()));
	}
	ASSERT_EQ(0, expected.size());

	out.clear();
	mooreQuery.query(Point<int>(0, 0), 2, true, out);
	for (int i = 0; i < 13; i += 3) {
		expected.insert(i);
		expected.insert(++i);
		expected.insert(++i);
	}
	ASSERT_EQ(9, out.size());
	for (size_t i = 0; i < out.size(); i++) {
		TestAgent* agent = out[i];
		//std::cout << agent->getId().id() << std::endl;
		ASSERT_EQ(1, expected.erase(agent->getId().id()));
	}
	ASSERT_EQ(0, expected.size());

	id = 0;
	for (int x = 5; x < 10; x++) {
		for (int y = 5; y < 10; y++) {
			grid->moveTo(AgentId(id++, 0, 0), Point<int>(x, y));
		}
	}

	out.clear();
	mooreQuery.query(Point<int>(9, 9), 2, true, out);
	expected.insert(12);
	expected.insert(13);
	expected.insert(14);
	expected.insert(17);
	expected.insert(18);
	expected.insert(19);
	expected.insert(22);
	expected.insert(23);
	expected.insert(24);

	ASSERT_EQ(9, out.size());
	for (size_t i = 0; i < out.size(); i++) {
		TestAgent* agent = out[i];
		//std::cout << agent->getId().id() << std::endl;
		ASSERT_EQ(1, expected.erase(agent->getId().id()));
	}
	ASSERT_EQ(0, expected.size());

}

TEST(GridTest, VNQueries)
{
	Context<TestAgent> context;

	GridDimensions dims(Point<double> (10, 10));
	Spaces<TestAgent>::MultipleStrictDiscreteSpace* grid = new Spaces<TestAgent>::MultipleStrictDiscreteSpace("grid", dims);
	context.addProjection(grid);

	for (int i = 0; i < 25; i++) {
		TestAgent* agent = new TestAgent(i, 0, 0);
		context.addAgent(agent);
	}

	int id = 0;
	for (int x = 0; x < 5; x++) {
		for (int y = 0; y < 5; y++) {
			grid->moveTo(AgentId(id++, 0, 0), Point<int> (x, y));
		}
	}

	VN2DGridQuery<TestAgent> query(grid);
	vector<TestAgent*> out;
	query.query(Point<int> (2, 2), 2, true, out);
	ASSERT_EQ(9, out.size());

	set<int> expected;
	for (int i = 10; i < 15; i++) {
		expected.insert(i);
	}
	for (int i = 2; i < 23; i += 5) {
		expected.insert(i);
	}

	for (size_t i = 0; i < out.size(); i++) {
		TestAgent* agent = out[i];
		//std::cout << agent->getId().id() << std::endl;
		ASSERT_EQ(1, expected.erase(agent->getId().id()));
	}
	ASSERT_EQ(0, expected.size());

	out.clear();
	query.query(Point<int> (0, 0), 2, false, out);
	expected.insert(1);
	expected.insert(2);
	expected.insert(5);
	expected.insert(10);
	ASSERT_EQ(4, out.size());
	for (size_t i = 0; i < out.size(); i++) {
		TestAgent* agent = out[i];
		//std::cout << agent->getId().id() << std::endl;
		ASSERT_EQ(1, expected.erase(agent->getId().id()));
	}
	ASSERT_EQ(0, expected.size());

	out.clear();
	id = 0;
	for (int x = 5; x < 10; x++) {
		for (int y = 5; y < 10; y++) {
			grid->moveTo(AgentId(id++, 0, 0), Point<int>(x, y));
		}
	}
	query.query(Point<int> (9, 9), 2, true, out);
	expected.insert(24);
	expected.insert(14);
	expected.insert(19);
	expected.insert(23);
	expected.insert(22);
	ASSERT_EQ(5, out.size());
	for (size_t i = 0; i < out.size(); i++) {
		TestAgent* agent = out[i];
		//std::cout << agent->getId().id() << std::endl;
		ASSERT_EQ(1, expected.erase(agent->getId().id()));
	}
	ASSERT_EQ(0, expected.size());

	out.clear();
	query.query(Point<int> (1, 1), 2, true, out);
	ASSERT_EQ(0, out.size());

}

