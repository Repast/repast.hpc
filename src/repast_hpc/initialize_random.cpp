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
 *  DistributionFactory.cpp
 *
 *  Created on: Oct 27, 2009
 *      Author: nick
 */

#include "boost/random/mersenne_twister.hpp"
#include <boost/numeric/conversion/bounds.hpp>
#include <boost/limits.hpp>
#include <boost/mpi.hpp>

#include "initialize_random.h"
#include "Random.h"
#include "Utilities.h"

#include "RepastProcess.h" // TESTING ONLY

#include <vector>

using namespace std;

namespace repast {

const size_t DIST_TAG_LENGTH = 13;

const string DBL_UNI_DIST = "double_uniform";
const string INT_UNI_DIST = "int_uniform";
const string TRIANGLE_DIST = "triangle";
const string CAUCHY_DIST = "cauchy";
const string EXPONENTIAL_DIST = "exponential";
const string NORMAL_DIST = "normal";
const string LOG_NORMAL_DIST = "lognormal";

void initializeSeed(const Properties& props, boost::mpi::communicator* comm);

void createDblUni(string& name, vector<string>& params) {
	if (params.size() != 3)
		throw Repast_Error_19(name, params); // Wrong number of parameters
	double from = strToDouble(trim(params[1]));
	double to = strToDouble(trim(params[2]));
	Random* random = Random::instance();
	_RealUniformGenerator gen(random->engine(), boost::uniform_real<>(from, to));
	NumberGenerator* ng = new DefaultNumberGenerator<_RealUniformGenerator> (gen);
	random->putGenerator(name, ng);
}

void createIntUni(string& name, vector<string>& params) {
	if (params.size() != 3)
	  throw Repast_Error_20(name, params); // Wrong number of parameters
	int from = strToInt(trim(params[1]));
	int to = strToInt(trim(params[2]));

	Random* random = Random::instance();
	_IntUniformGenerator gen(random->engine(), boost::uniform_int<>(from, to));
	NumberGenerator* ng = new DefaultNumberGenerator<_IntUniformGenerator> (gen);
	random->putGenerator(name, ng);
}

void createTriangle(string& name, vector<string>& params) {
	if (params.size() != 4)
	  throw Repast_Error_21(name, params); // Wrong number of parameters
	double lower = strToDouble(trim(params[1]));
	double mostLikely = strToDouble(trim(params[2]));
	double upper = strToDouble(trim(params[3]));

	Random* random = Random::instance();
	boost::triangle_distribution<> dist(lower, mostLikely, upper);
	_TriangleGenerator gen(random->engine(), dist);
	NumberGenerator* ng = new DefaultNumberGenerator<_TriangleGenerator> (gen);
	random->putGenerator(name, ng);
}

void createCauchy(string& name, vector<string>& params) {
	if (params.size() != 3)
	  throw Repast_Error_22(name, params); // Wrong number of parameters
	double median = strToDouble(trim(params[1]));
	double sigma = strToDouble(trim(params[2]));

	Random* random = Random::instance();
	boost::cauchy_distribution<> dist(median, sigma);
	_CauchyGenerator gen(random->engine(), dist);
	NumberGenerator* ng = new DefaultNumberGenerator<_CauchyGenerator> (gen);
	random->putGenerator(name, ng);
}

void createExponential(string& name, vector<string>& params) {
	if (params.size() != 2)
	  throw Repast_Error_23(name, params); // Wrong number of parameters
	double lambda = strToDouble(trim(params[1]));

	Random* random = Random::instance();
	boost::exponential_distribution<> dist(lambda);
	_ExponentialGenerator gen(random->engine(), dist);
	NumberGenerator* ng = new DefaultNumberGenerator<_ExponentialGenerator> (gen);
	random->putGenerator(name, ng);
}

void createNormal(string& name, vector<string>& params) {
	if (params.size() != 3)
	  throw Repast_Error_24(name, params); // Wrong number of parameters
	double mean = strToDouble(trim(params[1]));
	double sigma = strToDouble(trim(params[2]));

	Random* random = Random::instance();
	boost::normal_distribution<> dist(mean, sigma);
	_NormalGenerator gen(random->engine(), dist);
	NumberGenerator* ng = new DefaultNumberGenerator<_NormalGenerator> (gen);
	random->putGenerator(name, ng);
}

void createLogNormal(string& name, vector<string>& params) {
	if (params.size() != 3)
	  throw Repast_Error_25(name, params); // Wrong number of parameters
	double mean = strToDouble(trim(params[1]));
	double sigma = strToDouble(trim(params[2]));

	Random* random = Random::instance();
	boost::lognormal_distribution<> dist(mean, sigma);
	_LogNormalGenerator gen(random->engine(), dist);
	NumberGenerator* ng = new DefaultNumberGenerator<_LogNormalGenerator> (gen);
	random->putGenerator(name, ng);
}

void initializeRandom(Properties& props, boost::mpi::communicator* comm) {
	initializeSeed(props, comm);
	for (Properties::key_iterator iter = props.keys_begin(); iter != props.keys_end(); iter++) {
		string key = *iter;
		// starts with distribution tag
		if (key.find("distribution.", 0) == 0 && key.size() > DIST_TAG_LENGTH) {
			string name = key.substr(13);
			vector<string> params;
			tokenize(props.getProperty(key), params, ",");
			if (params.size() < 1)
			  throw Repast_Error_26(name, params); // Wrong number of parameters
			const string type = params[0];
			if (type == DBL_UNI_DIST)
				createDblUni(name, params);
			else if (type == INT_UNI_DIST)
				createIntUni(name, params);
			else if (type == TRIANGLE_DIST)
				createTriangle(name, params);
			else if (type == CAUCHY_DIST)
				createCauchy(name, params);
			else if (type == EXPONENTIAL_DIST)
				createExponential(name, params);
			else if (type == NORMAL_DIST)
				createNormal(name, params);
			else if (type == LOG_NORMAL_DIST)
				createLogNormal(name, params);
			else
			  throw Repast_Error_27(name, params); // Unrecognized type name
		}
	}
}

void initializeSeed(Properties& props, boost::mpi::communicator* comm) {
  // Default value for seed is local proc's system time
  boost::uint32_t seed = (boost::uint32_t)time(0);

  if(props.contains(GLOBAL_RANDOM_SEED_PROPERTY)){
    std::string propVal = props.getProperty(GLOBAL_RANDOM_SEED_PROPERTY);
    if(propVal.compare("AUTO") == 0){
      if(comm == 0)  throw Repast_Error_57(); // Needs communicator to share global seed
      boost::mpi::broadcast(*comm, seed, 0);
      props.putProperty(GLOBAL_RANDOM_SEED_PROPERTY, seed);
    }
    else{
      seed = strToUInt(propVal);
    }
   }
   else{
    // If no global seed, then each proc will use its own seed
    if(props.contains(RANDOM_SEED_PROPERTY)){
      std::string propVal = props.getProperty(RANDOM_SEED_PROPERTY);
      if(propVal.compare("AUTO") == 0){
        if(comm != 0) boost::mpi::broadcast(*comm, seed, 0);
        props.putProperty(RANDOM_SEED_PROPERTY, seed);
      }
      else{
        seed = (boost::uint32_t)strToUInt(propVal);
      }
    }
    if(comm != 0){
      boost::mt19937 gen;
      boost::uniform_real<> dist(0, boost::numeric::bounds<boost::uint32_t>::highest());
      gen.seed(seed);
      boost::variate_generator<boost::mt19937&, boost::uniform_real<> > localRNG(gen, dist);
      for(int i = 0; i < comm->rank(); i++)  seed = localRNG(); // The assignment only matters on the last time through, but calling the generator is requisite.
    }
    stringstream ss;
    ss << std::fixed << seed;
    props.putProperty(RANDOM_SEED_PROPERTY, ss.str());
  }
  Random::initialize(seed);
}

}
