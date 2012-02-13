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
 * agent_set_tests.cpp
 *
 *  Created on: Aug 16, 2010
 *      Author: nick
 */

#include <gtest/gtest.h>
#include "relogo/AgentSet.h"
#include <vector>

using namespace repast::relogo;
using namespace std;

class TestObj {
private:
	int _val;

public:
	TestObj(int val) :
		_val(val) {
	}

	virtual ~TestObj() {
	}

	int val() const {
		return _val;
	}

	void val(int newVal) {
		_val = newVal;
	}
};

struct ValGetter {

	double operator()(const TestObj* obj) const {
		return obj->val();
	}
};

TEST(AgentSet, ShuffleTest)
{
	std::vector<TestObj*> objs;
	AgentSet<TestObj> set;
	for (int i = 0; i < 30; i++) {
		TestObj* obj = new TestObj(i);
		set.add(obj);
		objs.push_back(obj);
	}

	set.shuffle();
	// hopefully shuffle will move the first item
	ASSERT_NE(set[0], objs[0]);
}

TEST(AgentSet, MinMaxTest)
{
	AgentSet<TestObj> set;
	for (int i = 0; i < 30; i++) {
		TestObj* obj = new TestObj(i);
		set.add(obj);
	}
	// min should be 0
	ValGetter getter;
	TestObj* obj = set.minOneOf(getter);
	ASSERT_EQ(0, obj->val());

	obj = set.maxOneOf(getter);
	ASSERT_EQ(29, obj->val());

	AgentSet<TestObj> out;
	set[3]->val(40);
	set[4]->val(40);
	set.withMax(getter, out);
	ASSERT_EQ(2, out.size());
	ASSERT_EQ(40, out[0]->val());
	ASSERT_EQ(40, out[1]->val());

	out.clear();
	set[0]->val(-15);
	set[1]->val(-15);
	set.withMin(getter, out);
	ASSERT_EQ(2, out.size());
	ASSERT_EQ(-15, out[0]->val());
	ASSERT_EQ(-15, out[1]->val());

	out.clear();
	vector<int> expected;
	expected.push_back(-15);
	expected.push_back(-15);
	expected.push_back(2);
	expected.push_back(5);
	expected.push_back(6);
	set.minNOf(5, getter, out);
	//for (int i = 0; i < out.size(); i++) {
	//	std::cout << out[i]->val() << std::endl;
	//}
	ASSERT_EQ(5, out.size());
	for (int i = 0; i < 5; i++) {
		//std::cout << out[i]->val() << std::endl;
		vector<int>::iterator iter = find(expected.begin(), expected.end(), out[i]->val());
		ASSERT_TRUE(iter != expected.end());
		expected.erase(iter);
	}

	out.clear();
	expected.clear();
	expected.push_back(40);
	expected.push_back(40);
	expected.push_back(29);
	expected.push_back(28);
	expected.push_back(27);
	set.maxNOf(5, getter, out);
	//for (int i = 0; i < out.size(); i++) {
	//	std::cout << out[i]->val() << std::endl;
	//}
	ASSERT_EQ(5, out.size());
	for (int i = 0; i < 5; i++) {
		//std::cout << out[i]->val() << std::endl;
		vector<int>::iterator iter = find(expected.begin(), expected.end(), out[i]->val());
		ASSERT_TRUE(iter != expected.end());
		expected.erase(iter);
	}

	for (size_t i = 0; i < set.size(); i++) {
		delete set[i];
	}
	set.clear();

}
