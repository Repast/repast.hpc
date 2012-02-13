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
 *  context_test.cpp
 *
 *  Created on: Jan 6, 2009
 *      Author: nick
 */

#include "repast_hpc/Context.h"
#include "repast_hpc/Graph.h"
#include "repast_hpc/ValueLayer.h"
#include "repast_hpc/GridComponents.h"

#include "test.h"

#include <gtest/gtest.h>
#include <boost/smart_ptr.hpp>
#include <boost/unordered_set.hpp>
#include <vector>

using namespace repast;
using namespace boost;
using namespace std;

class ContextTest: public testing::Test {

protected:

	repast::Context<TestAgent> context;

public:

};

TEST_F(ContextTest, AddRemove)
{
	ASSERT_EQ(0, context.size());

	TestAgent* agent = new TestAgent(0, 1, 1);
	context.addAgent(agent);
	ASSERT_EQ(1, context.size());
	ASSERT_TRUE(context.contains(agent->getId()));

	context.removeAgent(agent->getId());
	ASSERT_EQ(0, context.size());
	ASSERT_TRUE(!context.contains(agent->getId()));

	for (int i = 0; i < 20; i++) {
		agent = new TestAgent(i, 0, 0);
		context.addAgent(agent);
	}

	//for (Context<TestAgent>::const_iterator iter = context.begin(); iter != context.end(); ++iter) {
	//	std::cout << (*iter)->getId() << std::endl;
	//}
}

TEST_F(ContextTest, ValueLayer)
{
	DiscreteValueLayer<int, StrictBorders>* discrete = new DiscreteValueLayer<int, StrictBorders> ("D", GridDimensions(
			Point<int> (2, 2)), true);
	discrete->set(12, Point<int> (0, 0));
	context.addValueLayer(discrete);

	ContinuousValueLayer<double, StrictBorders>* continuous = new ContinuousValueLayer<double, StrictBorders> ("C",
			GridDimensions(Point<int> (2, 2)));
	continuous->set(12.3, Point<double> (1.2, 0));
	context.addValueLayer(continuous);

	DiscreteValueLayer<int, StrictBorders>* valueLayer = context.getDiscreteValueLayer<int, StrictBorders> ("D");
	ASSERT_EQ(12, valueLayer->get(Point<int>(0, 0)));

	ContinuousValueLayer<double, StrictBorders>* cLayer = context.getContinuousValueLayer<double, StrictBorders>("C");
	ASSERT_EQ(12.3, cLayer->get(Point<double>(1.2, 0)));
}

TEST_F(ContextTest, DirectedGraph)
{
	Graph<TestAgent, RepastEdge<TestAgent> >* graph = new Graph<TestAgent, RepastEdge<TestAgent> > ("graph", true);
	context.addProjection(graph);

	for (int i = 0; i < 10; i++) {
		TestAgent* agent = new TestAgent(i, 0, 0);
		context.addAgent(agent);
	}

	ASSERT_EQ(10, graph->vertexCount());

	AgentId id(0, 0, 0);
	TestAgent* source = context.getAgent(id);

	for (int i = 3; i < 6; i++) {
		id = AgentId(i, 0, 0);
		TestAgent* target = context.getAgent(id);
		graph->addEdge(source, target);
	}

	ASSERT_EQ(3, graph->edgeCount());

	vector<AgentId> expected;
	expected.push_back(AgentId(3, 0, 0));
	expected.push_back(AgentId(4, 0, 0));
	expected.push_back(AgentId(5, 0, 0));

	vector<TestAgent*> successors;
	graph->successors(source, successors);
	ASSERT_EQ(3, successors.size());
	vector<TestAgent*>::iterator iter;
	for (iter = successors.begin(); iter != successors.end(); ++iter) {
		AgentId id = (*iter)->getId();
		vector<AgentId>::iterator find;
		for (find = expected.begin(); find != expected.end(); ++find) {
			if (*find == id) {
				expected.erase(find);
				break;
			}
		}
	}
	ASSERT_EQ(0, expected.size());

	id = AgentId(3, 0, 0);
	TestAgent* three = context.getAgent(id);
	id = AgentId(4, 0, 0);
	TestAgent* four = context.getAgent(id);

	RepastEdge<TestAgent>* edge = graph->addEdge(four, three, 2.5);
	ASSERT_EQ(2.5, edge->weight());
	ASSERT_EQ(four, edge->source());
	ASSERT_EQ(three, edge->target());

	edge = graph->findEdge(four, three);
	ASSERT_EQ(2.5, edge->weight());
	ASSERT_EQ(four, edge->source());
	ASSERT_EQ(three, edge->target());

	// edge is directed from 4 to 3, no edge from 3 to 4.
	ASSERT_EQ(NULL, graph->findEdge(three, four));

	vector<TestAgent*> preds;
	graph->predecessors(three, preds);
	ASSERT_EQ(2, preds.size());

	expected.push_back(AgentId(4, 0, 0));
	expected.push_back(AgentId(0, 0, 0));
	for (iter = preds.begin(); iter != preds.end(); ++iter) {
		AgentId id = (*iter)->getId();
		vector<AgentId>::iterator find;
		for (find = expected.begin(); find != expected.end(); ++find) {
			if (*find == id) {
				expected.erase(find);
				break;
			}
		}
	}
	ASSERT_EQ(0, expected.size());

	id = AgentId(1, 0, 0);
	TestAgent* one = context.getAgent(id);
	graph->addEdge(three, one);

	vector<TestAgent*> adj;
	graph->adjacent(three, adj);
	ASSERT_EQ(3, adj.size());

	expected.push_back(AgentId(4, 0, 0));
	expected.push_back(AgentId(0, 0, 0));
	expected.push_back(AgentId(1, 0, 0));
	for (iter = adj.begin(); iter != adj.end(); ++iter) {
		AgentId id = (*iter)->getId();
		vector<AgentId>::iterator find;
		for (find = expected.begin(); find != expected.end(); ++find) {
			if (*find == id) {
				expected.erase(find);
				break;
			}
		}
	}
	ASSERT_EQ(0, expected.size());
}

TEST_F(ContextTest, UndirectedGraph)
{

	Graph<TestAgent, RepastEdge<TestAgent> >* graph = new Graph<TestAgent, RepastEdge<TestAgent> > ("ungraph", false);
	context.addProjection(graph);

	for (int i = 0; i < 10; i++) {
		TestAgent* agent = new TestAgent(i, 0, 0);
		context.addAgent(agent);
	}

	ASSERT_EQ(10, graph->vertexCount());

	AgentId id(0, 0, 0);
	TestAgent* source = context.getAgent(id);

	for (int i = 3; i < 6; i++) {
		id = AgentId(i, 0, 0);
		TestAgent* target = context.getAgent(id);
		graph->addEdge(source, target);
	}

	ASSERT_EQ(3, graph->edgeCount());

	vector<AgentId> expected;
	expected.push_back(AgentId(3, 0, 0));
	expected.push_back(AgentId(4, 0, 0));
	expected.push_back(AgentId(5, 0, 0));

	vector<TestAgent*> actual;
	graph->successors(source, actual);
	ASSERT_EQ(3, actual.size());
	vector<TestAgent*>::iterator iter;
	for (iter = actual.begin(); iter != actual.end(); ++iter) {
		AgentId id = (*iter)->getId();
		vector<AgentId>::iterator find;
		for (find = expected.begin(); find != expected.end(); ++find) {
			if (*find == id) {
				expected.erase(find);
				break;
			}
		}
	}
	ASSERT_EQ(0, expected.size());

	expected.push_back(AgentId(3, 0, 0));
	expected.push_back(AgentId(4, 0, 0));
	expected.push_back(AgentId(5, 0, 0));

	actual.clear();
	graph->predecessors(source, actual);
	ASSERT_EQ(3, actual.size());
	for (iter = actual.begin(); iter != actual.end(); ++iter) {
		AgentId id = (*iter)->getId();
		vector<AgentId>::iterator find;
		for (find = expected.begin(); find != expected.end(); ++find) {
			if (*find == id) {
				expected.erase(find);
				break;
			}
		}
	}
	ASSERT_EQ(0, expected.size());

	expected.push_back(AgentId(3, 0, 0));
	expected.push_back(AgentId(4, 0, 0));
	expected.push_back(AgentId(5, 0, 0));

	actual.clear();
	graph->adjacent(source, actual);
	ASSERT_EQ(3, actual.size());
	for (iter = actual.begin(); iter != actual.end(); ++iter) {
		AgentId id = (*iter)->getId();
		vector<AgentId>::iterator find;
		for (find = expected.begin(); find != expected.end(); ++find) {
			if (*find == id) {
				expected.erase(find);
				break;
			}
		}
	}
	ASSERT_EQ(0, expected.size());

	id = AgentId(3, 0, 0);
	TestAgent* three = context.getAgent(id);
	id = AgentId(4, 0, 0);
	TestAgent* four = context.getAgent(id);

	RepastEdge<TestAgent>* edge = graph->addEdge(four, three, 2.5);
	ASSERT_EQ(2.5, edge->weight());
	ASSERT_EQ(four, edge->source());
	ASSERT_EQ(three, edge->target());

	edge = graph->findEdge(three, four);
	ASSERT_EQ(2.5, edge->weight());
	ASSERT_EQ(four, edge->source());
	ASSERT_EQ(three, edge->target());

	RepastEdge<TestAgent>* revE = graph->findEdge(four, three);
	ASSERT_EQ(2.5, revE->weight());
	ASSERT_EQ(four, revE->source());
	ASSERT_EQ(three, revE->target());
	ASSERT_EQ(edge, revE);

	actual.clear();
	graph->adjacent(three, actual);
	ASSERT_EQ(2, actual.size());

	expected.push_back(AgentId(4, 0, 0));
	expected.push_back(AgentId(0, 0, 0));

	for (iter = actual.begin(); iter != actual.end(); ++iter) {
		AgentId id = (*iter)->getId();
		vector<AgentId>::iterator find;
		for (find = expected.begin(); find != expected.end(); ++find) {
			if (*find == id) {
				expected.erase(find);
				break;
			}
		}
	}
	ASSERT_EQ(0, expected.size());
}

TEST_F(ContextTest, DirectedGraphRemove)
{
	Graph<TestAgent, RepastEdge<TestAgent> >* graph = new Graph<TestAgent, RepastEdge<TestAgent> > ("graph", true);
	context.addProjection(graph);

	for (int i = 0; i < 10; i++) {
		TestAgent* agent = new TestAgent(i, 0, 0);
		context.addAgent(agent);
	}

	ASSERT_EQ(10, graph->vertexCount());

	AgentId id(0, 0, 0);
	context.removeAgent(id);
	ASSERT_EQ(9, context.size());
	ASSERT_TRUE(!context.contains(id));
	ASSERT_EQ(9, graph->vertexCount());

	id = AgentId(3, 0, 0);
	TestAgent* three = context.getAgent(id);
	id = AgentId(4, 0, 0);
	TestAgent* four = context.getAgent(id);
	id = AgentId(1, 0, 0);
	TestAgent* one = context.getAgent(id);

	graph->addEdge(three, four);
	graph->addEdge(one, four);
	graph->addEdge(three, one);

	vector<TestAgent*> actual;
	graph->successors(three, actual);
	ASSERT_EQ(2, actual.size());

	// remove 1,0,0
	context.removeAgent(id);
	actual.clear();
	graph->successors(three, actual);
	ASSERT_EQ(1, actual.size());
	ASSERT_EQ(four, actual[0]);

	actual.clear();
	graph->predecessors(four, actual);
	ASSERT_EQ(1, actual.size());
	ASSERT_EQ(three, actual[0]);

	ASSERT_EQ(0, graph->findEdge(three, one));
}

TEST_F(ContextTest, UndirectedGraphRemove)
{
	Graph<TestAgent, RepastEdge<TestAgent> >* graph = new Graph<TestAgent, RepastEdge<TestAgent> > ("graph", false);
	context.addProjection(graph);

	for (int i = 0; i < 10; i++) {
		TestAgent* agent = new TestAgent(i, 0, 0);
		context.addAgent(agent);
	}

	ASSERT_EQ(10, graph->vertexCount());

	AgentId id(0, 0, 0);
	context.removeAgent(id);
	ASSERT_EQ(9, context.size());
	ASSERT_TRUE(!context.contains(id));
	ASSERT_EQ(9, graph->vertexCount());

	id = AgentId(3, 0, 0);
	TestAgent* three = context.getAgent(id);
	id = AgentId(4, 0, 0);
	TestAgent* four = context.getAgent(id);
	id = AgentId(1, 0, 0);
	TestAgent* one = context.getAgent(id);

	graph->addEdge(three, four);
	graph->addEdge(one, four);
	graph->addEdge(three, one);

	vector<TestAgent*> actual;
	graph->adjacent(three, actual);
	ASSERT_EQ(2, actual.size());

	// remove 1,0,0
	context.removeAgent(id);
	actual.clear();
	graph->adjacent(three, actual);
	ASSERT_EQ(1, actual.size());
	ASSERT_EQ(four, actual[0]);

	actual.clear();
	graph->adjacent(four, actual);
	ASSERT_EQ(1, actual.size());
	ASSERT_EQ(three, actual[0]);

	ASSERT_EQ(0, graph->findEdge(three, one));
}

TEST_F(ContextTest, AgentByType)
{
	ASSERT_EQ(0, context.size());

	TestAgent* agent = new TestAgent(0, 1, 1);
	context.addAgent(agent);
	ASSERT_EQ(1, context.size());
	ASSERT_TRUE(context.contains(agent->getId()));

	context.removeAgent(agent->getId());
	ASSERT_EQ(0, context.size());
	ASSERT_TRUE(!context.contains(agent->getId()));

	for (int i = 0; i < 10; i++) {
		agent = new TestAgent(i, 0, 0);
		context.addAgent(agent);
	}

	for (int i = 0; i < 10; i++) {
		agent = new TestAgent(i, 0, 1);
		context.addAgent(agent);
	}

	boost::unordered_set<AgentId, HashId> expected;
	for (int i = 0; i < 10; i++) {
		expected.insert(AgentId(i, 0, 0));
	}

	for (Context<TestAgent>::const_bytype_iterator iter = context.byTypeBegin(0); iter != context.byTypeEnd(0); ++iter) {
		ASSERT_EQ(1, expected.erase((*iter)->getId()));
	}
	ASSERT_EQ(0, expected.size());

	for (int i = 0; i < 10; i++) {
		expected.insert(AgentId(i, 0, 1));
	}

	for (Context<TestAgent>::const_bytype_iterator iter = context.byTypeBegin(1); iter != context.byTypeEnd(0); ++iter) {
		ASSERT_EQ(1, expected.erase((*iter)->getId()));
	}
	ASSERT_EQ(0, expected.size());
}





// JTM Additions


TEST_F(ContextTest, CountOf)
{

	// CountOf
	vector<int*> vec;
	int testArray[10];
	
	for(int i = 0; i < 10; i++) {
		testArray[i] = i;
		vec.push_back(&testArray[i]);
	}
	ASSERT_EQ(10, countOf(vec.begin(), vec.end()));
	
	vec.erase(vec.begin());
	vec.erase(vec.begin());
	vec.erase(vec.begin());
	
	ASSERT_EQ(7, countOf(vec.begin(), vec.end()));
}

TEST_F(ContextTest, Shuffle)
{
	vector<int*> vec;
	int testArray[10];
	
	for(int i = 0; i < 10; i++) {
		testArray[i] = i;
		vec.push_back(&testArray[i]);
	}
	
	// shuffleList
	repast::shuffleList(vec);
	
    	// No way to test if this is mathematically randomized!

	// shuffleSet
	set<int*> set;
	set.insert(vec.begin(), vec.end());
	vector<int*> shuffledResult;
	shuffleSet(set, shuffledResult);
	
	ASSERT_EQ(10, shuffledResult.size());
}

TEST_F(ContextTest, RandomAccess)
{
	// Random Access
	set<int*> sourceSet;
	int testArray[10000];
	for(int i = 0; i < 10000; i++){
		testArray[i] = i;
		sourceSet.insert(&testArray[i]);
    }
	
	RandomAccess<set<int*>::iterator> ra(sourceSet.begin(), sourceSet.size());
	

	set<int*>::iterator testIterator;
	int index = 100;
	testIterator = sourceSet.begin();
	for(int i = 0; i < index; i++) testIterator++;
	ASSERT_EQ(**testIterator, **ra.get(index));

	
	index = 9900;
	testIterator = sourceSet.begin();
	for(int i = 0; i < index; i++) testIterator++;
	ASSERT_EQ(**testIterator, **ra.get(index));
}


TEST_F(ContextTest, ElementSelection__I)
{
	set<int*> sourceSet;
	int testArray[10000];
	for(int i = 0; i < 10000; i++){
		testArray[i] = i;
		sourceSet.insert(&testArray[i]);
    }
	
	// Random selection of elements
	// Note: two versions tested A & B,
	// Unordered and ordered
	set<int*> setA, setB;
	vector<int*> vecA, vecB;
	
	// Basic case: selecting 100 elements
	selectNElementsAtRandom(sourceSet.begin(), sourceSet.size(), 100, setA);
	selectNElementsAtRandom(sourceSet.begin(), sourceSet.end(), 100, setB);
	selectNElementsInRandomOrder(sourceSet.begin(), sourceSet.size(), 100, vecA);
	selectNElementsInRandomOrder(sourceSet.begin(), sourceSet.end(), 100, vecB);
	
	
	ASSERT_EQ(100, setA.size());
	ASSERT_EQ(100, setB.size());
	ASSERT_EQ(100, vecA.size());
	ASSERT_EQ(100, vecB.size());
	
	
	setA.clear();
	setB.clear();
	vecA.clear();
	vecB.clear();
	
	// Case in which many elements are requested
	selectNElementsAtRandom(sourceSet.begin(), sourceSet.size(), 9999, setA);
	selectNElementsAtRandom(sourceSet.begin(), sourceSet.end(), 9999, setB);
	selectNElementsInRandomOrder(sourceSet.begin(), sourceSet.size(), 9999, vecA);
	selectNElementsInRandomOrder(sourceSet.begin(), sourceSet.end(), 9999, vecB);
	
	ASSERT_EQ(9999, setA.size());
	ASSERT_EQ(9999, setB.size());
	ASSERT_EQ(9999, vecA.size());
	ASSERT_EQ(9999, vecB.size());
	
	setA.clear();
	setB.clear();
	vecA.clear();
	vecB.clear();
	
	
	// Case in which request exceeds number of available elements
	selectNElementsAtRandom(sourceSet.begin(), sourceSet.size(), 20000, setA);
	selectNElementsAtRandom(sourceSet.begin(), sourceSet.end(), 20000, setB);
	selectNElementsInRandomOrder(sourceSet.begin(), sourceSet.size(), 20000, vecA);
	selectNElementsInRandomOrder(sourceSet.begin(), sourceSet.end(), 20000, vecB);
	
	ASSERT_EQ(10000, setA.size());
	ASSERT_EQ(10000, setB.size());
	ASSERT_EQ(10000, vecA.size());
	ASSERT_EQ(10000, vecB.size());
	
}


TEST_F(ContextTest, ElementSelection_Unordered_II)
{
	// A smaller demo for seeding the result set with elements that
	// should not be selected
	set<int*> seededSetDemoSource;
	int testArray[100];
	
	for(int i = 0; i < 100; i++) testArray[i] = i;
    for(int i = 0; i < 10; i++)  seededSetDemoSource.insert(&testArray[i]);
	
	set<int*> seededSetDemoOutputSet;
	vector<int*> seededSetDemoOutputVec;
	
	seededSetDemoOutputSet.insert(&testArray[0]); // Put '0' in
	seededSetDemoOutputSet.insert(&testArray[1]); // Put '1' in
	seededSetDemoOutputVec.push_back(&testArray[0]); // Put '0' in
	seededSetDemoOutputVec.push_back(&testArray[1]); // Put '1' in
	
	
	
	// Now request 5
	selectNElementsAtRandom(seededSetDemoSource.begin(), seededSetDemoSource.end(), 5, seededSetDemoOutputSet);
	selectNElementsInRandomOrder(seededSetDemoSource.begin(), seededSetDemoSource.end(), 5, seededSetDemoOutputVec);

	
	// The result will be seven elements in the set- the 5 requested plus the 2 originally present
	ASSERT_EQ(7, seededSetDemoOutputSet.size());
	ASSERT_TRUE(seededSetDemoOutputSet.find(&testArray[0]) != seededSetDemoOutputSet.end());
	ASSERT_TRUE(seededSetDemoOutputSet.find(&testArray[1]) != seededSetDemoOutputSet.end());	
	
	ASSERT_EQ(7, seededSetDemoOutputVec.size());
	ASSERT_TRUE(find(seededSetDemoOutputVec.begin(), seededSetDemoOutputVec.end(), &testArray[0]) != seededSetDemoOutputVec.end());	
	ASSERT_TRUE(find(seededSetDemoOutputVec.begin(), seededSetDemoOutputVec.end(), &testArray[1]) != seededSetDemoOutputVec.end());	
	
	
	// Now request 5 more to the same result set; this isn't possible (7 are already 'excluded',
	// leaving only three unchosen. The result will be all three added to the set for a total of (all) ten
	selectNElementsAtRandom(seededSetDemoSource.begin(), seededSetDemoSource.end(), 5, seededSetDemoOutputSet);
	selectNElementsInRandomOrder(seededSetDemoSource.begin(), seededSetDemoSource.end(), 5, seededSetDemoOutputVec);
	ASSERT_EQ(10, seededSetDemoOutputSet.size());
	ASSERT_EQ(10, seededSetDemoOutputVec.size());
	
	// A mildly unusual case: the output set is seeded with 20 elements, only seven of which
	// are in the source set. The request is for 5 from the source set; this is impossible
	// but the three that are available are added, so the total is 23 elements.
	seededSetDemoOutputSet.clear();
	seededSetDemoOutputVec.clear();
	
	for(int i = 0; i < 7; i++){
		seededSetDemoOutputSet.insert(&testArray[i]);
		seededSetDemoOutputVec.push_back(&testArray[i]);
    }
	for(int i = 10; i < 23; i++){
		seededSetDemoOutputSet.insert(&testArray[i]);
		seededSetDemoOutputVec.push_back(&testArray[i]);
    }
	
	ASSERT_EQ(20, seededSetDemoOutputSet.size());
	ASSERT_EQ(20, seededSetDemoOutputVec.size());
	
	selectNElementsAtRandom(seededSetDemoSource.begin(), seededSetDemoSource.end(), 5, seededSetDemoOutputSet);
	selectNElementsInRandomOrder(seededSetDemoSource.begin(), seededSetDemoSource.end(), 5, seededSetDemoOutputVec);
	
	ASSERT_EQ(23, seededSetDemoOutputSet.size());
	ASSERT_EQ(23, seededSetDemoOutputVec.size());
	
	// And, yes, they're the right ones:
	for(int i = 0; i < 23; i++){
		ASSERT_TRUE(find(seededSetDemoOutputSet.begin(), seededSetDemoOutputSet.end(), &testArray[i]) != seededSetDemoOutputSet.end());
		ASSERT_TRUE(find(seededSetDemoOutputVec.begin(), seededSetDemoOutputVec.end(), &testArray[i]) != seededSetDemoOutputVec.end());
	}
	
}

TEST_F(ContextTest, ElementSelection_Unordered_III)
{
	// A demo for seeding the result set with elements that
	// should not be selected but are deleted the selection
	set<int*> seededSetDemoSource;
	int testArray[100];
	
	for(int i = 0; i < 100; i++) testArray[i] = i;
    for(int i = 0; i < 10; i++)  seededSetDemoSource.insert(&testArray[i]);
	
	set<int*> seededSetDemoOutputSet;
	vector<int*> seededSetDemoOutputVec;
	
	seededSetDemoOutputSet.insert(&testArray[0]); // Put '0' in
	seededSetDemoOutputSet.insert(&testArray[1]); // Put '1' in
	seededSetDemoOutputVec.push_back(&testArray[0]); // Put '0' in
	seededSetDemoOutputVec.push_back(&testArray[1]); // Put '1' in
	
	
	
	// Now request 5
	selectNElementsAtRandom(seededSetDemoSource.begin(), seededSetDemoSource.end(), 5, seededSetDemoOutputSet, true);
	selectNElementsInRandomOrder(seededSetDemoSource.begin(), seededSetDemoSource.end(), 5, seededSetDemoOutputVec, true);
	
	
	// The result will be five elements in the set
	ASSERT_EQ(5, seededSetDemoOutputSet.size());
	ASSERT_EQ(5, seededSetDemoOutputVec.size());
	
	// None of these five will be the two that were originally there
	ASSERT_TRUE(seededSetDemoOutputSet.find(&testArray[0]) == seededSetDemoOutputSet.end());
	ASSERT_TRUE(seededSetDemoOutputSet.find(&testArray[1]) == seededSetDemoOutputSet.end());	
	ASSERT_TRUE(find(seededSetDemoOutputVec.begin(), seededSetDemoOutputVec.end(), &testArray[0]) == seededSetDemoOutputVec.end());
	ASSERT_TRUE(find(seededSetDemoOutputVec.begin(), seededSetDemoOutputVec.end(), &testArray[1]) == seededSetDemoOutputVec.end());
	
	
	// Now request 7 more to the same result set; this isn't possible (5 are already 'excluded',
	// leaving only 5 unchosen). The result will be (the other) five elements
	selectNElementsAtRandom(seededSetDemoSource.begin(), seededSetDemoSource.end(), 5, seededSetDemoOutputSet, true);
	selectNElementsInRandomOrder(seededSetDemoSource.begin(), seededSetDemoSource.end(), 5, seededSetDemoOutputVec, true);
	ASSERT_EQ(5, seededSetDemoOutputSet.size());
	ASSERT_EQ(5, seededSetDemoOutputVec.size());
	
	// A mildly unusual case: the output set is seeded with 20 elements, only seven of which
	// are in the source set. The request is for 5 from the source set; this is impossible
	// but the three that are available are added. Then the original 20 are removed.
	seededSetDemoOutputSet.clear();
	seededSetDemoOutputVec.clear();
	
	for(int i = 0; i < 7; i++){
		seededSetDemoOutputSet.insert(&testArray[i]);
		seededSetDemoOutputVec.push_back(&testArray[i]);
    }
	for(int i = 10; i < 23; i++){
		seededSetDemoOutputSet.insert(&testArray[i]);
		seededSetDemoOutputVec.push_back(&testArray[i]);
    }
	
	ASSERT_EQ(20, seededSetDemoOutputSet.size());
	ASSERT_EQ(20, seededSetDemoOutputVec.size());
	
	selectNElementsAtRandom(seededSetDemoSource.begin(), seededSetDemoSource.end(), 5, seededSetDemoOutputSet, true);
	selectNElementsInRandomOrder(seededSetDemoSource.begin(), seededSetDemoSource.end(), 5, seededSetDemoOutputVec, true);
	
	ASSERT_EQ(3, seededSetDemoOutputSet.size());
	ASSERT_EQ(3, seededSetDemoOutputVec.size());
	
	// And, yes, they're the right ones:
	for(int i = 0; i < 0; i++){
		ASSERT_TRUE(find(seededSetDemoOutputSet.begin(), seededSetDemoOutputSet.end(), &testArray[i]) == seededSetDemoOutputSet.end());
		ASSERT_TRUE(find(seededSetDemoOutputVec.begin(), seededSetDemoOutputVec.end(), &testArray[i]) == seededSetDemoOutputVec.end());
    }
	for(int i = 7; i < 10; i++){
		ASSERT_TRUE(find(seededSetDemoOutputSet.begin(), seededSetDemoOutputSet.end(), &testArray[i]) != seededSetDemoOutputSet.end());
		ASSERT_TRUE(find(seededSetDemoOutputVec.begin(), seededSetDemoOutputVec.end(), &testArray[i]) != seededSetDemoOutputVec.end());
    }
	for(int i = 10; i < 100; i++){
		ASSERT_TRUE(find(seededSetDemoOutputSet.begin(), seededSetDemoOutputSet.end(), &testArray[i]) == seededSetDemoOutputSet.end());
		ASSERT_TRUE(find(seededSetDemoOutputVec.begin(), seededSetDemoOutputVec.end(), &testArray[i]) == seededSetDemoOutputVec.end());
    }
}

// Test to ensure that the randomness is all determined from the
// same random number seed
TEST_F(ContextTest, ShuffleAndSelect){
	// First test of shuffle
	vector<int*> vec1;
	vector<int*> vec2;
	vector<int*> vec3;
	vector<int*> source;
	int testArray[10000];
	
	for(int i = 0; i < 10000; i++) {
		testArray[i] = i;
		vec1.push_back(&testArray[i]);
		vec2.push_back(&testArray[i]);
		vec3.push_back(&testArray[i]);
		source.push_back(&testArray[i]);
	}
	
	// shuffleList
	
	repast::Random::initialize(1);
	repast::shuffleList(vec1);
	
	repast::Random::initialize(1);
	repast::shuffleList(vec2); // Should be the same as 1
	
	repast::Random::initialize(1000);
	repast::shuffleList(vec3);	// Should (almost certainly) be different from 1
	
	vector<int*>::iterator it1 = vec1.begin();
	vector<int*>::iterator it2 = vec2.begin();
	vector<int*>::iterator it3 = vec3.begin();
	
	bool vec3Matches = true;
	while(it1 != vec1.end()){
		ASSERT_EQ(**it1, **it2);
		vec3Matches = vec3Matches && (**it1 == **it3);
		it1++;
		it2++;
		it3++;
	}
	ASSERT_FALSE(vec3Matches);
	
	
	// Now test of selection
	set<int*> set1;
	set<int*> set2;
	set<int*> set3;
	
	
	repast::Random::initialize(1);
	repast::selectNElementsAtRandom(source.begin(), source.end(), 1000, set1);
	
	repast::Random::initialize(1);
	repast::selectNElementsAtRandom(source.begin(), source.end(), 1000, set2); // Should be the same as 1
	
	repast::Random::initialize(1000);
	repast::selectNElementsAtRandom(source.begin(), source.end(), 1000, set3);	// Should (almost certainly) be different from 1

	set<int*>::iterator setIt1 = set1.begin();
	set<int*>::iterator setIt2 = set2.begin();
	set<int*>::iterator setIt3 = set3.begin();
	
	bool set3Matches = true;
	while(setIt1 != set1.end()){
		ASSERT_EQ(**setIt1, **setIt2);
		set3Matches = set3Matches && (**setIt1 == **setIt3);
		setIt1++;
		setIt2++;
		setIt3++;
	}
	ASSERT_FALSE(set3Matches);
	
}




// Agents


// Useful tests
void testSetContains(std::set<TestAgent*>& agentSet, std::vector<TestAgent*>& agentVec, std::vector<TestAgent*>& excluding){
	std::vector<TestAgent*>::iterator excIterator = excluding.begin();
	
	while(excIterator != excluding.end()){
		ASSERT_TRUE(find(agentSet.begin(), agentSet.end(), *excIterator) != agentSet.end());
		ASSERT_TRUE(find(agentVec.begin(), agentVec.end(), *excIterator) != agentVec.end());
		excIterator++;
	}
}


void testSetDoesNotContain(std::set<TestAgent*>& agentSet, std::vector<TestAgent*>& agentVec, std::vector<TestAgent*>& excluding){
	std::vector<TestAgent*>::iterator excIterator = excluding.begin();
	
	while(excIterator != excluding.end()){
		ASSERT_TRUE(find(agentSet.begin(), agentSet.end(), *excIterator) == agentSet.end());
		ASSERT_TRUE(find(agentVec.begin(), agentVec.end(), *excIterator) == agentVec.end());
		excIterator++;
	}
	
}

void testAgentsAreOfType(std::set<TestAgent*>& agentSet, std::vector<TestAgent*>& agentVec, int type){
	set<TestAgent*>::iterator setIterator = agentSet.begin();
	vector<TestAgent*>::iterator vecIterator = agentVec.begin();
	
	for(unsigned int i = 0; i < agentVec.size(); i++){
		ASSERT_EQ(type, (*setIterator)->getId().agentType());
		ASSERT_EQ(type, (*vecIterator)->getId().agentType());
		setIterator++;
		vecIterator++;
	}
}


void testAgentIDsBelow(std::set<TestAgent*>& agentSet, std::vector<TestAgent*>& agentVec, int upperIDBound){
	set<TestAgent*>::iterator setIterator = agentSet.begin();
	vector<TestAgent*>::iterator vecIterator = agentVec.begin();
	
	for(unsigned int i = 0; i < agentVec.size(); i++){
		ASSERT_TRUE((*setIterator)->getId().id() <= upperIDBound);
		ASSERT_TRUE((*vecIterator)->getId().id() <= upperIDBound);
		setIterator++;
		vecIterator++;
	}
}


// Demonstration filter
struct IsLowNumberedAgent {
    IsLowNumberedAgent(){}
	
    bool operator()(const boost::shared_ptr<TestAgent>& ptr){
        return ptr->getId().id() <= 200;
    }
	
};



TEST_F(ContextTest, AgentSelection_Basic)
{
	for(int i = 0; i < 1000; i++){
		TestAgent* agent = new TestAgent(i, 0, i % 3); // Note: some different types
		context.addAgent(agent);
    }
	
	set<TestAgent*> resultSet;
	vector<TestAgent*> resultVec;

	int popSize = countOf(context.begin(), context.end());
	
	
	
	// SELECTION I: All Agents

	// SETUP
	resultSet.clear();
	resultVec.clear();
	
	// SELECTION
	context.selectAgents(resultSet);
	context.selectAgents(resultVec);
	
	// TESTS
	ASSERT_EQ(popSize, resultSet.size());
	ASSERT_EQ(popSize, resultVec.size());
	
	
	
	// SELECTION II: Specific Number of Agents
	
	// SETUP
	resultSet.clear();
	resultVec.clear();
	
	// SELECTION
	context.selectAgents(100, resultSet);
	context.selectAgents(100, resultVec);
	
	// TESTS
	ASSERT_EQ(100, resultSet.size());
	ASSERT_EQ(100, resultVec.size());

	
	// SELECTION III: Excluding certain agents, and leaving them in the result set
	
	// SETUP
	vector<TestAgent*> temp(resultVec.begin(), resultVec.end());
	
	resultSet.clear(); resultSet.insert(temp.begin(), temp.end());
	resultVec.clear(); resultVec.insert(resultVec.begin(), temp.begin(), temp.end());
	
	
	// SELECTION
	context.selectAgents(100, resultSet);
	context.selectAgents(100, resultVec);
	
	
	// TESTS
	ASSERT_EQ(200, resultSet.size());
	ASSERT_EQ(200, resultVec.size());
	testSetContains(resultSet, resultVec, temp);


	
	// SELECTION IV: Excluding certain agents, but removing them from the result set
	
	// SETUP
	resultSet.clear(); resultSet.insert(temp.begin(), temp.end());
	resultVec.clear(); resultVec.insert(resultVec.begin(), temp.begin(), temp.end());

	// SELECTION
	context.selectAgents(100, resultSet, true);
	context.selectAgents(100, resultVec, true);
	
	// TESTS
	ASSERT_EQ(100, resultSet.size());
	ASSERT_EQ(100, resultVec.size());
	testSetDoesNotContain(resultSet, resultVec, temp);

	
	
	
	// SELECTION V: Test of the optional 'popSize' parameter
    // Not used for basic selection	
		
}



TEST_F(ContextTest, AgentSelection_ByType){

	for(int i = 0; i < 1000; i++){
		TestAgent* agent = new TestAgent(i, 0, i % 3); // Note: some different types
		context.addAgent(agent);
    }

	set<TestAgent*> resultSet;
	vector<TestAgent*> resultVec;

	int _type = 1;

	int popSize = countOf(context.byTypeBegin(_type), context.byTypeEnd(_type));

	
	// There is chance involved; do repeated trials
	for(int t = 0; t < 20; t++){
		
		// SELECTION I: All Agents

		// SETUP
		resultSet.clear();
		resultVec.clear();
		
		// SELECTION
		context.selectAgents(resultSet, _type);
		context.selectAgents(resultVec, _type);
		
		// TESTS
		ASSERT_EQ(popSize, resultSet.size());
		ASSERT_EQ(popSize, resultVec.size());
		testAgentsAreOfType(resultSet, resultVec, _type);

		
		// SELECTION II: Specific Number of Agents
		
		// SETUP
		resultSet.clear();
		resultVec.clear();
	
		
		// SELECTION Select agents
		context.selectAgents(100, resultSet, _type);
		context.selectAgents(100, resultVec, _type);
	
		// TESTS
		ASSERT_EQ(100, resultSet.size());
		ASSERT_EQ(100, resultVec.size());
		testAgentsAreOfType(resultSet, resultVec, _type);

		
		// SELECTION III: Excluding certain agents, and leaving them in the result set
		
		// SETUP
		vector<TestAgent*> temp(resultVec.begin(), resultVec.end());
		
		// SETUP
		resultSet.clear();		resultSet.insert(temp.begin(), temp.end());
		resultVec.clear();		resultVec.insert(resultVec.begin(), temp.begin(), temp.end());
	
		context.selectAgents(100, resultSet, _type);
		context.selectAgents(100, resultVec, _type);
	
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
		context.selectAgents(100, resultSet, _type, true);
		context.selectAgents(100, resultVec, _type, true);
	
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
		context.selectAgents(100, resultSet, _type, false, popSize);
		context.selectAgents(100, resultVec, _type, false, popSize);
		
		// TESTS
		ASSERT_EQ(expectedSize, resultSet.size());
		ASSERT_EQ(expectedSize, resultVec.size());
	    testAgentsAreOfType(resultSet, resultVec, _type);
				
	}
}



// Filtered
TEST_F(ContextTest, AgentSelection_Filtered){
	
	for(int i = 0; i < 1000; i++){
		TestAgent* agent = new TestAgent(i, 0, i % 3); // Note: some different types
		context.addAgent(agent);
    }
	
	set<TestAgent*> resultSet;
	vector<TestAgent*> resultVec;

	IsLowNumberedAgent filter;

	int popSize = countOf(context.filteredBegin(filter), context.filteredEnd(filter));
	
	
	// There is chance involved; do repeated trials
	for(int t = 0; t < 20; t++){
	
		
		// SELECTION I: All Agents
		
		// SETUP
		resultSet.clear();
		resultVec.clear();
		
		// SELECTION
		context.selectAgents(resultSet, filter);
		context.selectAgents(resultVec, filter);
		
		// TESTS
		ASSERT_EQ(popSize, resultSet.size());
		ASSERT_EQ(popSize, resultVec.size());
		testAgentIDsBelow(resultSet, resultVec, 200);
	
		
		
		// SELECTION II: Specific Number of Agents
		
		// SETUP
		resultSet.clear();
		resultVec.clear();
		
		// SELECTION
		context.selectAgents(100, resultSet, filter);
		context.selectAgents(100, resultVec, filter);
		
		// TESTS
		ASSERT_EQ(100, resultSet.size());
		ASSERT_EQ(100, resultVec.size());
		testAgentIDsBelow(resultSet, resultVec, 200);		
	
		
		
		// SELECTION III: Excluding certain agents, and leaving them in the result set

		// SETUP
		vector<TestAgent*> temp(resultVec.begin(), resultVec.end());
		resultSet.clear();		resultSet.insert(temp.begin(), temp.end());
		resultVec.clear();		resultVec.insert(resultVec.begin(), temp.begin(), temp.end());

		// SELECTION
		context.selectAgents(100, resultSet, filter);
		context.selectAgents(100, resultVec, filter);
		
		// TESTS
		// Now should have 200 (note: there are 201 (0-200, inclusive) that we could have selected
		ASSERT_EQ(200, resultSet.size());
		ASSERT_EQ(200, resultVec.size());
		testAgentIDsBelow(resultSet, resultVec, 200);
		testSetContains(resultSet, resultVec, temp);

		
		// SELECTION IV: Excluding certain agents, but removing them from the result set
		// SETUP
		resultSet.clear();		resultSet.insert(temp.begin(), temp.end());
		resultVec.clear();		resultVec.insert(resultVec.begin(), temp.begin(), temp.end());
		
		// SELECTION
		context.selectAgents(100, resultSet, filter, true);
		context.selectAgents(100, resultVec, filter, true);
		
		// TESTS
		ASSERT_EQ(100, resultSet.size());
		ASSERT_EQ(100, resultVec.size());
		testAgentIDsBelow(resultSet, resultVec, 200);
		testSetDoesNotContain(resultSet, resultVec, temp);
		

		
		// SELECTION V: Test of the optional 'popSize' parameter
		
		// SETUP
		int expectedSize = (popSize < 100 ? popSize : 100);
		resultSet.clear();
		resultVec.clear();

		// SELECTION
		context.selectAgents(100, resultSet, filter, false, popSize);
		context.selectAgents(100, resultVec, filter, false, popSize);
		
		// Now should have the original 100+ up to 100 more
		ASSERT_EQ(expectedSize, resultSet.size());
		ASSERT_EQ(expectedSize, resultVec.size());
		testAgentIDsBelow(resultSet, resultVec, 200);
		
	}
}



TEST_F(ContextTest, AgentSelection_Filtered_ByType){
	
	for(int i = 0; i < 1000; i++){
		TestAgent* agent = new TestAgent(i, 0, i % 3); // Note: some different types
		context.addAgent(agent);
    }
	
	
	set<TestAgent*> resultSet;
	vector<TestAgent*> resultVec;
	
	IsLowNumberedAgent filter;
	
	int _type = 1;

	int popSize = countOf(context.byTypeFilteredBegin(_type, filter), context.byTypeFilteredEnd(_type, filter));
	
	
	// There is chance involved; do repeated trials
	for(int t = 0; t < 20; t++){
		
		// SELECTION I: All Agents
		
		// SETUP
		resultSet.clear();
		resultVec.clear();
		
		// SELECTION
		context.selectAgents(resultSet, _type, filter);
		context.selectAgents(resultVec, _type, filter);
		
		// TESTS
		ASSERT_EQ(popSize, resultSet.size());
		ASSERT_EQ(popSize, resultVec.size());
		testAgentsAreOfType(resultSet, resultVec, _type);
		testAgentIDsBelow(resultSet, resultVec, 200);

		
		// SELECTION II: Specific Number of Agents
		
		// SETUP
		resultSet.clear();
		resultVec.clear();
		
		// SELECTION
		context.selectAgents(10, resultSet, _type, filter);
		context.selectAgents(10, resultVec, _type, filter);
		
		// TESTS
		ASSERT_EQ(10, resultSet.size());
		ASSERT_EQ(10, resultVec.size());
		testAgentsAreOfType(resultSet, resultVec, _type);
		testAgentIDsBelow(resultSet, resultVec, 200);
		
		
		// SELECTION III: Excluding certain agents, and leaving them in the result set
		
		// SETUP
		vector<TestAgent*> temp(resultVec.begin(), resultVec.end());
		resultSet.clear();		resultSet.insert(temp.begin(), temp.end());
		resultVec.clear();		resultVec.insert(resultVec.begin(), temp.begin(), temp.end());
		
		// SELECTION
		context.selectAgents(10, resultSet, _type, filter);
		context.selectAgents(10, resultVec, _type, filter);
		
		// TESTS
		// Now should have 20 
		ASSERT_EQ(20, resultSet.size());
		ASSERT_EQ(20, resultVec.size());
		testAgentsAreOfType(resultSet, resultVec, _type);
		testAgentIDsBelow(resultSet, resultVec, 200);
		testSetContains(resultSet, resultVec, temp);

		
		// SELECTION IV: Excluding certain agents, but removing them from the result set
		
		// SETUP
		resultSet.clear();		resultSet.insert(temp.begin(), temp.end());
		resultVec.clear();		resultVec.insert(resultVec.begin(), temp.begin(), temp.end());
		
		// SELECTION
		context.selectAgents(10, resultSet, _type, filter, true);
		context.selectAgents(10, resultVec, _type, filter, true);
		
		// Now should have just 10
		ASSERT_EQ(10, resultSet.size());
		ASSERT_EQ(10, resultVec.size());
		testAgentsAreOfType(resultSet, resultVec, _type);
		testAgentIDsBelow(resultSet, resultVec, 200);
		testSetDoesNotContain(resultSet, resultVec, temp);
		
		
		// SELECTION V: Test of the optional 'popSize' parameter
		
		// SETUP
		int expectedSize = (popSize < 100 ? popSize : 100);
		resultSet.clear();
		resultVec.clear();
		
		// SELECTION
		context.selectAgents(100, resultSet, _type, filter, false, popSize);
		context.selectAgents(100, resultVec, _type, filter, false, popSize);
		
		// TESTS
		ASSERT_EQ(expectedSize, resultSet.size());
		ASSERT_EQ(expectedSize, resultVec.size());
		testAgentsAreOfType(resultSet, resultVec, _type);
		testAgentIDsBelow(resultSet, resultVec, 200);
		
	}
}
