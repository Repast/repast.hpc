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
 *  Observer.cpp
 *
 *  Created on: Jul 15, 2010
 *      Author: nick
 */

#include <exception>
#include <cmath>

#include "Observer.h"
#include "Patch.h"
#include "repast_hpc/RepastProcess.h"
#include "repast_hpc/initialize_random.h"
#include "repast_hpc/Point.h"
#include "utility.h"
#include "creators.h"
#include "RelogoErrors.h"

using namespace std;

namespace repast {
namespace relogo {

const string STOP_AT = "stop.at";

int Observer::nextTypeId = 1;
const int Observer::NO_TYPE_ID = -1;

static DefaultLinkCreator _defaultLinkCreator;

Observer::Observer() :
		context(RepastProcess::instance()->getCommunicator()), rndXP(0), rndYP(0), rndX(0), rndY(0) {
	_rank = RepastProcess::instance()->rank();
}

Observer::~Observer() {
	delete rndXP;
	delete rndYP;
	delete rndX;
	delete rndY;

	for (size_t i = 0; i < dataSets.size(); ++i) {
		delete dataSets[i];
	}
}

void Observer::dataSetClose() {
	for (size_t i = 0; i < dataSets.size(); ++i) {
		(dataSets[i])->write();
		(dataSets[i])->close();
	}
}

void Observer::addDataSet(repast::DataSet* dataSet) {
	dataSets.push_back(dataSet);
	ScheduleRunner& runner = RepastProcess::instance()->getScheduleRunner();
	runner.scheduleEvent(1.1, 1, Schedule::FunctorPtr(new MethodFunctor<repast::DataSet> (dataSet,
			&repast::DataSet::record)));
	Schedule::FunctorPtr dsWrite = Schedule::FunctorPtr(new MethodFunctor<repast::DataSet> (dataSet,
			&repast::DataSet::write));
	runner.scheduleEvent(100.2, 100, dsWrite);
}

int Observer::minPxcor() const {
	return localBounds.origin(0);
}
int Observer::minPycor() const {
	return localBounds.origin(1);
}

int Observer::maxPxcor() const {
	return localBounds.origin(0) + localBounds.extents(0) - 1;
}
int Observer::maxPycor() const {
	return localBounds.origin(1) + localBounds.extents(1) - 1;
}

int Observer::randomPxcor() {
	return (int) rndXP->next();
}

int Observer::randomPycor() {
	return (int) rndYP->next();
}

double Observer::randomXcor() {
	return rndX->next();
}
double Observer::randomYcor() {
	return rndY->next();
}

void Observer::_setup(Properties& props) {
	_props = props;
	repast::initializeRandom(props, RepastProcess::instance()->getCommunicator());
	ScheduleRunner& runner = RepastProcess::instance()->getScheduleRunner();
	runner.scheduleEvent(1, 1, Schedule::FunctorPtr(new MethodFunctor<Observer> (this, &Observer::go)));
	if (props.contains(STOP_AT)) {
		double stopAt = strToDouble(props.getProperty(STOP_AT));
		runner.scheduleStop(stopAt);
	}
	runner.scheduleEndEvent(Schedule::FunctorPtr(new MethodFunctor<Observer> (this, &Observer::dataSetClose)));

	Random* random = Random::instance();
	_IntUniformGenerator gen(random->engine(), boost::uniform_int<>(minPxcor(), maxPxcor()));
	rndXP = new DefaultNumberGenerator<_IntUniformGenerator> (gen);

	_IntUniformGenerator gen2(random->engine(), boost::uniform_int<>(minPycor(), maxPycor()));
	rndYP = new DefaultNumberGenerator<_IntUniformGenerator> (gen2);

	_RealUniformGenerator gen3(random->engine(), boost::uniform_real<>(minPxcor(), maxPxcor()));
	rndX = new DefaultNumberGenerator<_RealUniformGenerator> (gen3);

	_RealUniformGenerator gen4(random->engine(), boost::uniform_real<>(minPycor(), maxPycor()));
	rndY = new DefaultNumberGenerator<_RealUniformGenerator> (gen4);

	setup(props);
}

const RelogoGridType* Observer::grid() {
	return static_cast<RelogoGridType*> (context.getProjection(GRID_NAME));
}

const RelogoSpaceType* Observer::space() {
	return static_cast<RelogoSpaceType*> (context.getProjection(SPACE_NAME));
}

bool Observer::spacePtToGridPt(std::vector<double>& spacePt, std::vector<int>& gridPt) {
  gridPt[0] = doubleCoordToInt(spacePt[0]);
  gridPt[1] = doubleCoordToInt(spacePt[1]);
  return false;
//	bool switchSpace = false;
//	int px = doubleCoordToInt(spacePt[0]);
//	int py = doubleCoordToInt(spacePt[1]);
//	gridPt[0] = px;
//	gridPt[1] = py;
//	bool gridContains = grid()->dimensions().contains(gridPt);
//	bool spaceContains = space()->dimensions().contains(spacePt);
//	if (spaceContains) {
//		if (!gridContains) {
//			switchSpace = true;
//			spacePt[0] = px;
//			spacePt[1] = py;
//		}
//	} else {
//		if (gridContains) {
//			switchSpace = true;
//			spacePt[0] = px;
//			spacePt[1] = py;
//			/*
//			 // move out of the grid as well then.
//			 double x = spacePt[0];
//			 double y = spacePt[1];
//			 if (x < minPxcor()) {
//			 gridPt[0] = (int) floor(x);
//			 } else if (x > maxPxcor()) {
//			 gridPt[0] = (int) ceil(x);
//			 }
//
//			 if (y < minPycor()) {
//			 gridPt[1] = (int) floor(y);
//			 } else if (y > maxPycor()) {
//			 gridPt[1] = (int) ceil(y);
//			 }
//			 */
//
//		} else {
//			// outside of both need to make sure
//			// that the points refer to the same neighbor
//			double x = spacePt[0];
//			double y = spacePt[1];
//			if ((x > maxPxcor() && x < maxPxcor() + 1) || (x < minPxcor() && x > minPxcor() - 1)) {
//				// x is in no-man's land but px must be
//				// past that
//				switchSpace = true;
//				spacePt[0] = px;
//			}
//
//			if ((y > maxPycor() && y < maxPycor() + 1) || (y < minPycor() && y > minPycor() - 1)) {
//				// y is in no-man's land but py must be
//				// past that
//				switchSpace = true;
//				spacePt[1] = py;
//			}
//		}
//	}
//
//	return switchSpace;
}

Observer::NetworkType* Observer::findNetwork(const std::string& name) {
	NetworkType* net = static_cast<NetworkType*> (context.getProjection(name));
	if (net == 0) {
		if (name == DEFAULT_DIR_NET)
			throw ReLogo_Error_1(name); // The default directed network must be created before using it
		else if (name == DEFAULT_UNDIR_NET)
			throw ReLogo_Error_2(name); // The default undirected network must be create before using it
		else
			throw ReLogo_Error_3(name); // Network must be created before it can be used
	}
	return net;
}

AgentSet<Turtle> Observer::turtles() {
	AgentSet<Turtle> turtles;
	Observer::get(turtles);
	return turtles;
}

/**
 * Casts a pointer to a RelogoAgent to a pointer to a Turtle
 */
struct TurtleCaster: public std::unary_function<boost::shared_ptr<RelogoAgent>, Turtle*> {
	Turtle* operator()(boost::shared_ptr<RelogoAgent> ptr) const {
		return (Turtle*) (ptr.get());
	}
};

void Observer::get(AgentSet<Turtle>& turtles) {
	const_not_type_iterator istart(repast::IsNotType<RelogoAgent>(PATCH_TYPE_ID), context.begin(), context.end());
	const_not_type_iterator iend(repast::IsNotType<RelogoAgent>(PATCH_TYPE_ID), context.end(), context.end());
	boost::transform_iterator<TurtleCaster, const_not_type_iterator> begin(istart);
	boost::transform_iterator<TurtleCaster, const_not_type_iterator> end(iend);
	turtles.addAll(begin, end);
}

void Observer::createLink(RelogoAgent* source, RelogoAgent* target, const std::string& name) {
	createLink(source, target, name, _defaultLinkCreator);
}

void Observer::removeAgent(const AgentId& id) {
	// by removing it that should be enough to
	// delete the agent via the shared_ptr, hopefully ...
	context.removeAgent(id);
}

boost::shared_ptr<RelogoLink> Observer::link(RelogoAgent* source, RelogoAgent* target, const std::string& networkName) {
	NetworkType* net = findNetwork(networkName);
	return net->findEdge(source, target);
}

Patch* Observer::patchAt(int x, int y) {
	vector<RelogoAgent*> out;
	grid()->getObjectsAt(Point<int> (x, y), out);
	for (int i = 0, n = out.size(); i < n; i++) {
		if ((out[i])->getId().agentType() == PATCH_TYPE_ID) {
			return static_cast<Patch*> (out[i]);
		}
	}
	return 0;
}

Patch* Observer::patchAt(Point<double> location, double dx, double dy) {
	const RelogoSpaceType* aSpace = space();
	Point<double> displacement(dx, dy);
	vector<double> out(2, 0);

	try {
		aSpace->translate(location, displacement, out);
	} catch (out_of_range ex) {
		return 0;
	}
	return patchAt(doubleCoordToInt(out[0]), doubleCoordToInt(out[1]));
}

Patch* Observer::patchAtOffset(Point<double> location, double heading, double distance){
  std::vector<double> offset = relogo::calcDisplacementFromHeadingDistance(heading, distance);
  return patchAt(location, offset[0], offset[1]);
}

}
}
