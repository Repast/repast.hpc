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

#include "repast_hpc/Random.h"
#include "repast_hpc/Properties.h"
#include "repast_hpc/initialize_random.h"

#include <typeinfo>

using namespace repast;

TEST(Random, Uniform)
{

	repast::Random* random = repast::Random::instance();
	double val = random->nextDouble();
	ASSERT_TRUE(val >= 0 && val < 1);

	repast::DoubleUniformGenerator gen = random->createUniDoubleGenerator(10, 30);
	for (int i = 0; i < 1000; i++) {
		val = gen.next();
		ASSERT_TRUE(val >= 10 && val < 30);
	}

	repast::IntUniformGenerator iGen = random->createUniIntGenerator(20, 40);
	for (int i = 0; i < 1000; i++) {
		val = iGen.next();
		ASSERT_TRUE(val >= 20 && val <= 40);
	}
}

// not testing correctness of the random distributions per-se
// but if our adapter generators are working.
TEST(Random, Others)
{
	repast::Random* random = repast::Random::instance();
	repast::TriangleGenerator tGen = random->createTriangleGenerator(1, 3, 10);
	for (int i = 0; i < 1000; i++) {
		double val = tGen.next();
		ASSERT_TRUE(val >= 1 && val <= 10);
	}

	repast::ExponentialGenerator eGen = random->createExponentialGenerator(2);
	for (int i = 0; i < 1000; i++) {
		double val = eGen.next();
		ASSERT_TRUE(val > 0);
	}

	double val = random->createCauchyGenerator(1, 2).next();
	val = random->createNormalGenerator(0, 1).next();
	val = random->createLogNormalGenerator(1, 1).next();
}

TEST(Random, StoredDist)
{

	Random* random = repast::Random::instance();
	_IntUniformGenerator gen(random->engine(), boost::uniform_int<>(10, 20));
	IntUniformGenerator* iGen = new repast::IntUniformGenerator(gen);
	random->putGenerator("int gen", iGen);

	NumberGenerator* nGen = random->getGenerator("int gen");
	for (int i = 0; i < 1000; i++) {
		double val = nGen->next();
		ASSERT_TRUE(val >= 10 && val <= 20);
	}
}

TEST(Random, DistFromProps)
{
	Properties props = Properties();
	props.putProperty("random.seed", "123");
	props.putProperty("distribution.double_uni", "double_uniform, 10.3, 20.6");
	props.putProperty("distribution.int_uni", "int_uniform, 3, 10");
	props.putProperty("distribution.aTriangle", "triangle, 3, 4, 10");
	props.putProperty("distribution.aCauchy", "cauchy, 3, 10");
	props.putProperty("distribution.aExponential", "exponential, 2");
	props.putProperty("distribution.aNormal", "normal, .5, 1");
	props.putProperty("distribution.aLogNormal", "lognormal, 1, 1");

	initializeRandom(props);
	ASSERT_EQ(123, Random::instance()->seed());

	Random* random = Random::instance();
	NumberGenerator* nGen = random->getGenerator("double_uni");
	ASSERT_TRUE(typeid(*nGen) == typeid(DoubleUniformGenerator));
	for (int i = 0; i < 1000; i++) {
		double val = nGen->next();
		ASSERT_TRUE(val >= 10.3 && val < 20.6);
	}

	nGen = random->getGenerator("int_uni");
	ASSERT_TRUE(typeid(*nGen) == typeid(IntUniformGenerator));
	for (int i = 0; i < 1000; i++) {
		double val = nGen->next();
		ASSERT_TRUE(val >= 3 && val <= 10);
	}

	nGen = random->getGenerator("aTriangle");
	ASSERT_TRUE(typeid(*nGen) == typeid(TriangleGenerator));
	double val = nGen->next();

	nGen = random->getGenerator("aCauchy");
	ASSERT_TRUE(typeid(*nGen) == typeid(CauchyGenerator));
	val = nGen->next();

	nGen = random->getGenerator("aExponential");
	ASSERT_TRUE(typeid(*nGen) == typeid(ExponentialGenerator));
	val = nGen->next();

	nGen = random->getGenerator("aNormal");
	ASSERT_TRUE(typeid(*nGen) == typeid(NormalGenerator));
	val = nGen->next();

	nGen = random->getGenerator("aLogNormal");
	ASSERT_TRUE(typeid(*nGen) == typeid(LogNormalGenerator));
	val = nGen->next();

}
