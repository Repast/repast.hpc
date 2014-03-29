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
 * ZombieObserver.cpp
 *
 *  Created on: Sep 1, 2010
 *      Author: nick
 */
#include <sstream>

#include "relogo/RandomMove.h"
#include "relogo/grid_types.h"
#include "relogo/Patch.h"
#include "repast_hpc/AgentRequest.h"
#include "repast_hpc/SVDataSet.h"
#include "repast_hpc/SVDataSetBuilder.h"

#ifndef _WIN32
#include "repast_hpc/NCDataSetBuilder.h"
#endif

#include "ZombieObserver.h"
#include "Human.h"
#include "Zombie.h"
#include "InfectionSum.h"

using namespace std;
using namespace repast;
using namespace relogo;

const string HUMAN_COUNT_PROP = "human.count";
const string ZOMBIE_COUNT_PROP = "zombie.count";

void ZombieObserver::go() {
  if (_rank == 0) {
    Log4CL::instance()->get_logger("root").log(INFO, "TICK BEGINS: " + boost::lexical_cast<string>(RepastProcess::instance()->getScheduleRunner().currentTick()));
  }
  synchronize<AgentPackage>(*this, *this, *this, RepastProcess::USE_LAST_OR_USE_CURRENT);

  AgentSet<Zombie> zombies;
  get(zombies);
  zombies.ask(&Zombie::step);

  AgentId id(0,0,2);
  Zombie* z = who<Zombie>(id);

  AgentSet<Human> humans;
  get(humans);
  humans.ask(&Human::step);

  if (_rank == 0) {
    Log4CL::instance()->get_logger("root").log(INFO, "TICK ENDS: " + boost::lexical_cast<string>(RepastProcess::instance()->getScheduleRunner().currentTick()));
  }
}

void ZombieObserver::setup(Properties& props) {
  //	Observer::setup(props); // No longer need to call this (SimRunner calls _setup, which calls this after all initialization is done)
  repast::Timer initTimer;
  initTimer.start();

  int humanCount = strToInt(props.getProperty(HUMAN_COUNT_PROP));
  humanType = create<Human> (humanCount);

  int zombieCount = strToInt(props.getProperty(ZOMBIE_COUNT_PROP));
  zombieType = create<Zombie> (zombieCount);

  AgentSet<Human> humans;
  get(humans);
  humans.apply(RandomMove(this));

  AgentSet<Zombie> zombies;
  get(zombies);
  zombies.apply(RandomMove(this));

	SVDataSetBuilder svbuilder("./output/data.csv", ",", repast::RepastProcess::instance()->getScheduleRunner().schedule());
	InfectionSum* iSum = new InfectionSum(this);
	svbuilder.addDataSource(repast::createSVDataSource("number_infected", iSum, std::plus<int>()));
	addDataSet(svbuilder.createDataSet());


#ifndef _WIN32
	// no netcdf under windows
	NCDataSetBuilder builder("./output/data.ncf", RepastProcess::instance()->getScheduleRunner().schedule());
	InfectionSum* infectionSum = new InfectionSum(this);
	builder.addDataSource(repast::createNCDataSource("number_infected", infectionSum, std::plus<int>()));
	addDataSet(builder.createDataSet());
#endif

	long double t = initTimer.stop();
	std::stringstream ss;
	ss << t;
	props.putProperty("init.time", ss.str());
}

RelogoAgent* ZombieObserver::createAgent(const AgentPackage& content) {
	if (content.type == zombieType) {
		return new Zombie(content.getId(), this);
	} else {
		return new Human(content.getId(), this, content);
	}
}

void ZombieObserver::provideContent(const repast::AgentRequest& request, std::vector<AgentPackage>& out) {
	const vector<AgentId>& ids = request.requestedAgents();
	for (int i = 0, n = ids.size(); i < n; i++) {
		AgentId id = ids[i];
		AgentPackage content = { id.id(), id.startingRank(), id.agentType(), id.currentRank(), 0, false };
		if (id.agentType() == humanType) {
			Human* human = who<Human> (id);
			content.infected = human->infected();
			content.infectionTime = human->infectionTime();
		}
		out.push_back(content);
	}
}

void ZombieObserver::provideContent(RelogoAgent* agent, std::vector<AgentPackage>& out) {
	AgentId id = agent->getId();
	AgentPackage content = { id.id(), id.startingRank(), id.agentType(), id.currentRank(), 0, false };
	if (id.agentType() == humanType) {
		Human* human = static_cast<Human*> (agent);
		content.infected = human->infected();
		content.infectionTime = human->infectionTime();
	}
	out.push_back(content);
}

void ZombieObserver::createAgents(std::vector<AgentPackage>& contents, std::vector<RelogoAgent*>& out) {
	for (size_t i = 0, n = contents.size(); i < n; ++i) {
		AgentPackage content = contents[i];
		if (content.type == zombieType) {
			out.push_back(new Zombie(content.getId(), this));
		} else if (content.type == humanType) {
			out.push_back(new Human(content.getId(), this, content));
		} else {
			// its a patch.
			out.push_back(new Patch(content.getId(), this));
		}
	}
}

void ZombieObserver::updateAgent(AgentPackage package){
  repast::AgentId id(package.id, package.proc, package.type);
  if (id.agentType() == humanType) {
    Human * human = who<Human> (id);
    human->set(package.infected, package.infectionTime);
  }
}
