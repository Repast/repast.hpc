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
 * grid_comp_test.cpp
 *
 *  Created on: Jun 23, 2009
 *      Author: nick
 */

#include "repast_hpc/GridComponents.h"
#include "repast_hpc/MultipleOccupancy.h"
#include "repast_hpc/SingleOccupancy.h"
#include "test.h"

#include <gtest/gtest.h>
#include <boost/unordered_set.hpp>

using namespace repast;
using namespace std;

/*
 * T* get(Point& location);

 bool put(boost::shared_ptr<T>& agent, Point& location);

 void remove(boost::shared_ptr<T>& agent, Point& location);
 */

TEST(WrapAroundBorders, Transform)
{

	GridDimensions dimensions(Point<int> (5, 8));
	WrapAroundBorders borders;
	borders.init(dimensions);

	vector<int> in;
	vector<int> out(2, 0);

	in.push_back(-4);
	in.push_back(17);
	borders.transform(in, out);

	ASSERT_EQ(1, out[0]);
	ASSERT_EQ(1, out[1]);

	WrapAroundBorders borders2 = WrapAroundBorders();
	borders2.init(GridDimensions(Point<int>(-4), Point<int>(10)));
	in.resize(1, 0);

	int input[] = {16, 8, 10, 14, 2, -3, 5, -15, -11, -13, -14};
	int expected[] = {-4, -2, 0, 4, 2, -3, 5, 5, -1, -3, -4};
	for (int i = 0; i < 11; i++) {
		in[0] = input[i];
		//std::cout << in[0] << std::endl;
		borders2.transform(in, out);
		ASSERT_EQ(expected[i], out[0]);
	}

	WrapAroundBorders borders3 = WrapAroundBorders();
	borders3.init(GridDimensions(Point<int>(1, 1), Point<int>(51, 101)));
	std::vector<double> in2;
	in2.push_back(9.06);
	in2.push_back(0.93934);
	std::vector<double> out2;
	borders3.transform(in2, out2);
	ASSERT_EQ(9.06, out2[0]);
	// needs wrap completely in order to get to 1
	ASSERT_EQ(101, out2[1]);


}

TEST(StickyBorders, Translate)
{
	GridDimensions dimensions(Point<int> (5, 8));
	StickyBorders borders;
	borders.init(dimensions);

	vector<int> old;
	old.push_back(3);
	old.push_back(6);
	vector<int> pos(2, 0);
	vector<int> displacement(2, 0);
	displacement[0] = 2;
	borders.translate(old, pos, displacement);
	ASSERT_EQ(4, pos[0]);
	ASSERT_EQ(6, pos[1]);

	displacement[0] = -10;
	displacement[1] = -10;
	borders.translate(old, pos, displacement);
	ASSERT_EQ(0, pos[0]);
	ASSERT_EQ(0, pos[1]);

	vector<double> oldd;
	oldd.push_back(3.5);
	oldd.push_back(6.0);
	vector<double> posd(2, 0);
	vector<double> displacementd(2, 0);
	displacementd[0] = 2;
	borders.translate(oldd, posd, displacementd);
	ASSERT_EQ(4.0, posd[0]);
	ASSERT_EQ(6.0, posd[1]);

	displacementd[0] = -10;
	displacementd[1] = -10;
	borders.translate(oldd, posd, displacementd);
	ASSERT_EQ(0, posd[0]);
	ASSERT_EQ(0, posd[1]);
}

TEST(StickyBorders, Transform)
{
	GridDimensions dimensions(Point<int> (5, 8));
	StickyBorders borders;
	borders.init(dimensions);

	vector<int> in;
	vector<int> out(2, 0);

	in.push_back(4);
	in.push_back(2);
	borders.transform(in, out);

	ASSERT_EQ(4, out[0]);
	ASSERT_EQ(2, out[1]);

	in.clear();

	try {
		in.push_back(10);
		in.push_back(2);
		borders.transform(in, out);
		ASSERT_TRUE(false);
	} catch (std::out_of_range) {
		ASSERT_TRUE(true);
	}

	in.clear();
	try {
		in.push_back(4);
		in.push_back(-1);
		borders.transform(in, out);
		ASSERT_TRUE(false);
	} catch (std::out_of_range) {
		ASSERT_TRUE(true);
	}

}

TEST(StrictBorders, Transform)
{
	GridDimensions dimensions(Point<int> (5, 8));
	StrictBorders borders;
	borders.init(dimensions);

	vector<int> in;
	vector<int> out(2, 0);

	in.push_back(4);
	in.push_back(2);
	borders.transform(in, out);

	ASSERT_EQ(4, out[0]);
	ASSERT_EQ(2, out[1]);

	in.clear();

	try {
		in.push_back(10);
		in.push_back(2);
		borders.transform(in, out);
		ASSERT_TRUE(false);
	} catch (std::out_of_range) {
		ASSERT_TRUE(true);
	}

	in.clear();
	try {
		in.push_back(4);
		in.push_back(-1);
		borders.transform(in, out);
		ASSERT_TRUE(false);
	} catch (std::out_of_range) {
		ASSERT_TRUE(true);
	}
}

TEST(SingleOccupancy, GetAll)
{
	SingleOccupancy<TestAgent, int> so;
	Point<int> pt(0, 0);
	ASSERT_EQ(NULL, so.get(pt));

	boost::shared_ptr<TestAgent> ptr(new TestAgent(1, 0, 0));
	so.put(ptr, pt);
	ASSERT_EQ(ptr.get(), so.get(pt));

	vector<TestAgent*> vec;
	so.getAll(pt, vec);
	ASSERT_EQ(1, vec.size());
	ASSERT_EQ(ptr.get(), vec[0]);

	pt = Point<int>(3, 3);
	vec.clear();
	so.getAll(pt, vec);
	ASSERT_EQ(0, vec.size());
}

TEST(MultipleOccupancy, All)
{
	MultipleOccupancy<TestAgent, int> mo;
	Point<int> pt(0, 0);
	ASSERT_EQ(NULL, mo.get(pt));

	vector<boost::shared_ptr<TestAgent> > agents;
	for (int i = 0; i < 10; i++) {
		agents.push_back(boost::shared_ptr<TestAgent>(new TestAgent(i, 0, 0)));
	}

	ASSERT_TRUE(mo.put(agents[0], pt));
	ASSERT_TRUE(mo.put(agents[1], pt));
	ASSERT_EQ(agents[0].get(), mo.get(pt));

	vector<TestAgent*> vec;
	mo.getAll(pt, vec);
	boost::unordered_set<AgentId, HashId> expected;
	expected.insert(AgentId(0, 0, 0));
	expected.insert(AgentId(1, 0, 0));

	ASSERT_EQ(2, vec.size());
	for (size_t i = 0; i < vec.size(); i++) {
		//std::cout << vec[i]->getId() << std::endl;
		expected.erase(vec[i]->getId());
	}
	ASSERT_EQ(0, expected.size());

	mo.remove(agents[0], pt);
	ASSERT_EQ(agents[1].get(), mo.get(pt));

	pt = Point<int>(4, 6);
	mo.put(agents[2], pt);
	ASSERT_EQ(agents[2].get(), mo.get(pt));
}
