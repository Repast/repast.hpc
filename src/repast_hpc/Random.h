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
 *  Random.h
 *
 *  Created on: Jun 9, 2009
 *      Author: nick
 */

#ifndef RANDOM_H_
#define RANDOM_H_

#include <boost/random/variate_generator.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/uniform_real.hpp>
#include <boost/random/triangle_distribution.hpp>
#include <boost/random/cauchy_distribution.hpp>
#include <boost/random/exponential_distribution.hpp>
#include <boost/random/geometric_distribution.hpp>
#include <boost/random/normal_distribution.hpp>
#include <boost/random/lognormal_distribution.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/cstdint.hpp>

#include <map>
#include <string>

namespace repast {

typedef boost::variate_generator<boost::mt19937&, boost::uniform_real<> > _RealUniformGenerator;
typedef boost::variate_generator<boost::mt19937&, boost::uniform_int<> > _IntUniformGenerator;

typedef boost::variate_generator<boost::mt19937&, boost::triangle_distribution<> > _TriangleGenerator;
typedef boost::variate_generator<boost::mt19937&, boost::cauchy_distribution<> > _CauchyGenerator;
typedef boost::variate_generator<boost::mt19937&, boost::exponential_distribution<> > _ExponentialGenerator;
typedef boost::variate_generator<boost::mt19937&, boost::geometric_distribution<boost::uniform_real<> > >
		_GeometricGenerator;
typedef boost::variate_generator<boost::mt19937&, boost::normal_distribution<> > _NormalGenerator;
typedef boost::variate_generator<boost::mt19937&, boost::lognormal_distribution<> > _LogNormalGenerator;

/**
 * Number generator interface.
 */
class NumberGenerator {

public:
	virtual ~NumberGenerator() {
	}

	/**
	 * Gets the "next" number from this
	 * Number Generator.
	 */
	virtual double next() = 0;
};

/**
 * Adapts the templated boost::variate_generator to the
 * NumberGenerator interface.
 */
template<typename T>
class DefaultNumberGenerator: public NumberGenerator {

private:
	T gen;

public:
	DefaultNumberGenerator(T generator);
	double next();
};

template<typename T>
DefaultNumberGenerator<T>::DefaultNumberGenerator(T generator) :
	gen(generator) {
}

template<typename T>
double DefaultNumberGenerator<T>::next() {
	return gen();
}

typedef DefaultNumberGenerator<_IntUniformGenerator> IntUniformGenerator;
typedef DefaultNumberGenerator<_RealUniformGenerator> DoubleUniformGenerator;
typedef DefaultNumberGenerator<_TriangleGenerator> TriangleGenerator;
typedef DefaultNumberGenerator<_CauchyGenerator> CauchyGenerator;
typedef DefaultNumberGenerator<_ExponentialGenerator> ExponentialGenerator;
typedef DefaultNumberGenerator<_NormalGenerator> NormalGenerator;
typedef DefaultNumberGenerator<_LogNormalGenerator> LogNormalGenerator;

/**
 * Methods for working with random distributions, draws etc.
 */
class Random {

private:
	static Random* instance_;

	boost::uint32_t _seed;
	boost::mt19937 rng;
	boost::uniform_real<> uni10;
	_RealUniformGenerator uniGen;

	std::map<std::string, NumberGenerator*> generators;

protected:
	Random(boost::uint32_t seed);

public:

	/**
	 * Initialize the Random singleton with the specified seed.
	 *
	 * @param the seed to initialize the random number generator with.
	 */
	static void initialize(boost::uint32_t seed);

	/**
	 * Gets the singleton instance of this Random.
	 */
	static Random* instance();
	virtual ~Random();

	/**
	 * Puts the named generator into this Random. Added
	 * generators will be deleted by Random when it is destroyed.
	 *
	 * @param the id of the generator
	 * @param generator the generator to add
	 */
	void putGenerator(const std::string& id, NumberGenerator* generator);

	/**
	 * Gets the named generator or 0 if the name
	 * is not found.
	 *
	 * @param id the name of the generator to get
	 */
	NumberGenerator* getGenerator(const std::string& id);

	/**
	 * Gets the random number engine from which the distributions are created.
	 *
	 * @return he random number engine from which the distributions are created.
	 */
	boost::mt19937& engine() {
		return rng;
	}

	/**
	 * Gets the current seed.
	 *
	 * @return the current seed.
	 */
	boost::uint32_t seed() {
		return _seed;
	}

	/**
	 * Gets the next double in the range [0, 1).
	 *
	 * @return the next double in the range [0, 1).
	 */
	double nextDouble();

	/**
	 * Creates a generator that produces doubles in the range [from, to).
	 *
	 * @param from the range start (inclusive)
	 * @param to the range end (exclusive)
	 *
	 * @return a generator that produces doubles in the range [from, to).
	 */
	DoubleUniformGenerator createUniDoubleGenerator(double from, double to);

	/**
	 * Creates a generator that produces ints in the range [from, to].
	 *
	 *  @param from the range start (inclusive)
	 * @param to the range end (inclusive)
	 *
	 * @return a generator that produces ints in the range [from, to].
	 */
	IntUniformGenerator createUniIntGenerator(int from, int to);

	/**
	 * Creates a triangle generator with the specified properties. A TriangleGenerator produces a
	 * floating point value x where lowerbound <= x <= upperBound and mostLikely is the most
	 * probable value for x.
	 *
	 * @param lowerBound the lower bound of the values produced by the generator
	 * @param mostLikely the most likely value produced by the generator
	 * @param upperBound the upper bound of the values produced by the generator
	 *
	 * @return a triangle generator.
	 */
	TriangleGenerator createTriangleGenerator(double lowerBound, double mostLikely, double upperBound);

	/**
	 * pdf: p(x) = sigma/(pi*(sigma**2 + (x-median)**2))
	 *
	 * @param median
	 * @param sigma
	 *
	 * @return a Cauchy generator.
	 */
	CauchyGenerator createCauchyGenerator(double median, double sigma);

	/**
	 * pdf: p(x) = lambda * exp(-lambda * x)
	 *
	 * @param lambda must be > 0
	 *
	 * @return an exponential generator.
	 */
	ExponentialGenerator createExponentialGenerator(double lambda);

	/**
	 * Creates a normal generator.
	 *
	 * pdf: p(x) = 1/sqrt(2*pi*sigma) * exp(- (x-mean)2 / (2*sigma2) )
	 *
	 *
	 */
	NormalGenerator createNormalGenerator(double mean, double sigma);

	/**
	 * Produces random numbers with p(x) = 1/(x * normal_sigma * sqrt(2*pi)) * exp( -(log(x)-normal_mean)2 / (2*normal_sigma2) )
	 * for x > 0, where normal_mean = log(mean2/sqrt(sigma2 + mean2)) and normal_sigma = sqrt(log(1 + sigma2/mean2))
	 */
	LogNormalGenerator createLogNormalGenerator(double mean, double sigma);
};

/**
 * Random generator function that can be used in STL algorithms that
 * need a random generator.
 *
 * @return a value from 0 up to, but not including i.
 */

ptrdiff_t uni_random(ptrdiff_t i);

}

#endif /* RANDOM_H_ */
