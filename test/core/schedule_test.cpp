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
 * schedule_test.cpp
 *
 *  Created on: Jan 6, 2009
 *      Author: nick
 */

#include "repast_hpc/Schedule.h"
#include <gtest/gtest.h>

using namespace repast;

class TestAgent {

public:
	TestAgent() : _result(0), x(0), y(0) {}
	int _result;
	double x, y;

	void OneTime() {
		_result = 1;
	}

	void Repeat() {
		_result++;
	}

};

class ScheduleTest : public testing::Test {

protected:

	repast::Schedule s1;
	TestAgent agent;

public:

	void X() {
		agent.x = s1.getCurrentTick();
	};

	void Y() {
		agent.y = s1.getCurrentTick();
	};
};

TEST_F(ScheduleTest, OneTime) {
	TestAgent tf;
	ASSERT_EQ(0, tf._result);
	Schedule::FunctorPtr mf = Schedule::FunctorPtr(new repast::MethodFunctor<TestAgent>(&tf, &TestAgent::OneTime));
	s1.schedule_event(1, mf);
	s1.execute();

	ASSERT_EQ(1, tf._result);
}

TEST_F(ScheduleTest, Repeat) {
	TestAgent tf;
	ASSERT_EQ(0, tf._result);
	Schedule::FunctorPtr mf = Schedule::FunctorPtr(new repast::MethodFunctor<TestAgent>(&tf, &TestAgent::Repeat));
	s1.schedule_event(1, 2, mf);
	// execute once and next tick should be 3
	s1.execute();
	ASSERT_EQ(1, tf._result);
	ASSERT_EQ(3.0, s1.getNextTick());

	int expected = 1;
	for (int i = 1; i < 5; i++) {
		s1.execute();
		ASSERT_EQ(++expected, tf._result);
		ASSERT_EQ(3.0 + (i * 2), s1.getNextTick());
	}
}

TEST_F(ScheduleTest, Mix) {
	Schedule::FunctorPtr mf = Schedule::FunctorPtr(new repast::MethodFunctor<ScheduleTest>(this, &ScheduleTest::X));
	s1.schedule_event(3.1, 1, mf);

	Schedule::FunctorPtr mf2 = Schedule::FunctorPtr(new repast::MethodFunctor<ScheduleTest>(this, &ScheduleTest::Y));
	s1.schedule_event(4, mf2);

	s1.execute();
	ASSERT_EQ(3.1, agent.x);
	ASSERT_EQ(0.0, agent.y);

	s1.execute();
	ASSERT_EQ(3.1, agent.x);
	ASSERT_EQ(4.0, agent.y);

	s1.execute();
	ASSERT_EQ(4.1, agent.x);
	ASSERT_EQ(4.0, agent.y);
}



