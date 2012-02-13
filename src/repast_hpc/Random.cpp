/*
 *   Repast for High Performance Computing (Repast HPC)
 *
 *   Copyright (c) 2010 Argonne National Laboratory
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with
 *   or without modification, are permitted provided that the following
 *   conditions are met:
 *
 *     Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *
 *     Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *
 *     Neither the name of the Argonne National Laboratory nor the names of its
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
 *
 *  Random.cpp
 *
 *  Created on: Jun 9, 2009
 *      Author: nick
 */

#include "Random.h"

#include <ctime>

namespace repast {

using namespace std;
using namespace boost;

Random* Random::instance_ = 0;

Random::Random(uint32_t seed) : _seed(seed), rng(seed), uni10(0, 1), uniGen(_RealUniformGenerator(rng, boost::uniform_real<>(0, 1))) {

}

Random::~Random() {
	for (map<string, NumberGenerator*>::iterator iter = generators.begin(); iter != generators.end(); iter++) {
		delete iter->second;
	}
}

void Random::initialize(uint32_t seed) {
	instance_ = new Random(seed);
}

Random* Random::instance() {
	if (instance_ == 0) {
		Random::initialize(std::time(0));
	}
	return instance_;
}

/**
 * inclusive of from, exclusive of to.
 */
DoubleUniformGenerator Random::createUniDoubleGenerator(double from, double to) {
	_RealUniformGenerator gen(rng, boost::uniform_real<>(from, to));
	return DefaultNumberGenerator<_RealUniformGenerator> (gen);
	//return DoubleUniformGenerator(from, to);
}

double Random::nextDouble() {
	return uniGen();
}

IntUniformGenerator Random::createUniIntGenerator(int from, int to) {
	_IntUniformGenerator gen(rng, boost::uniform_int<>(from, to));
	return DefaultNumberGenerator<_IntUniformGenerator> (gen);
	//return IntUniformGenerator(from, to);
}

TriangleGenerator Random::createTriangleGenerator(double lowerBound, double mostLikely, double upperBound) {
	boost::triangle_distribution<> dist(lowerBound, mostLikely, upperBound);
	_TriangleGenerator gen(rng, dist);
	return DefaultNumberGenerator<_TriangleGenerator> (gen);
}

CauchyGenerator Random::createCauchyGenerator(double median, double sigma) {
	boost::cauchy_distribution<> dist(median, sigma);
	_CauchyGenerator gen(rng, dist);
	return DefaultNumberGenerator<_CauchyGenerator> (gen);
}

ExponentialGenerator Random::createExponentialGenerator(double lambda) {
	boost::exponential_distribution<> dist(lambda);
	_ExponentialGenerator gen(rng, dist);
	return DefaultNumberGenerator<_ExponentialGenerator> (gen);
}

/*
 GeometricGenerator Random::createGeometricGenerator(double p) {
 boost::geometric_distribution<boost::uniform_real<> > dist(p);
 _GeometricGenerator gen(rng, dist);
 return GeometricGenerator(gen);
 }
 */

NormalGenerator Random::createNormalGenerator(double mean, double sigma) {
	boost::normal_distribution<> dist(mean, sigma);
	_NormalGenerator gen(rng, dist);
	return DefaultNumberGenerator<_NormalGenerator> (gen);
}

LogNormalGenerator Random::createLogNormalGenerator(double mean, double sigma) {
	boost::lognormal_distribution<> dist(mean, sigma);
	_LogNormalGenerator gen(rng, dist);
	return DefaultNumberGenerator<_LogNormalGenerator> (gen);
}

/*
 GeometricGenerator::GeometricGenerator(_GeometricGenerator generator) : gen(generator) {}

 double GeometricGenerator::next() {
 return gen()(gen.engine());
 }
 */

void Random::putGenerator(const string& id, NumberGenerator* generator) {
	generators.insert( make_pair(id, generator));
}

NumberGenerator* Random::getGenerator(const string& id) {
	map<string, NumberGenerator*>::iterator iter = generators.find(id);
	if (iter == generators.end()) return 0;
	return iter->second;
}

ptrdiff_t uni_random(ptrdiff_t i) {
	IntUniformGenerator gen = Random::instance()->createUniIntGenerator(0, i - 1);
	return (ptrdiff_t)gen.next();
}

}
