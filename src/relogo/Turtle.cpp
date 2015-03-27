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
 *  Turtle.cpp
 *
 *  Created on: Jul 15, 2010
 *      Author: nick
 */

#include <boost/unordered_set.hpp>
#include <cmath>

#include "Turtle.h"
#include "utility.h"
#include "relogo.h"
#include "Patch.h"

using namespace std;

namespace repast {

namespace relogo {

Turtle::Turtle(AgentId id, Observer* observer) :
	AbstractRelogoAgent(id, observer), _heading(0), moved(false) {
}

void Turtle::xCor(double x) {
	setxy(x, _location[1]);
}

void Turtle::yCor(double y) {
	setxy(_location[0], y);
}

void Turtle::die() {
	_observer->removeAgent(getId());
}

void Turtle::setxy(double x, double y) {
 	if (!moved) {
		moved = true;
		// cast away the constness so that we can move
		// needs to const so that users cannot move using the space
		RelogoSpaceType* space = const_cast<RelogoSpaceType*> (_observer->space());
		std::vector<double> pos(2, 0);
		pos[0] = x;
		pos[1] = y;
		std::vector<double> transformedCoords(2, 0);
		std::vector<double> oldLocation(_location.coords());
		space->transform(pos, transformedCoords);
		if (transformedCoords[0] != _location.getX() || transformedCoords[1] != _location.getY()) {
			_location = Point<double> (transformedCoords);
			std::vector<int> gridPt(2, 0);
			std::vector<double> spacePt(_location.coords());
			if (_observer->spacePtToGridPt(spacePt, gridPt)) {
				_location = Point<double> (spacePt);
			}

			space->moveTo(getId(), _location);

			// cast away the constness so that we can move
			// needs to const so that users can move using the grid
			RelogoGridType* grid = const_cast<RelogoGridType*> (_observer->grid());
			grid->moveTo(getId(), Point<int> (gridPt));
			moveTiedTurtles(oldLocation);

		}

		moved = false;
	}
}

void Turtle::moveTiedTurtles(const Point<double>& oldLocation) {
	if (!fixedLeaves.empty() || !freeLeaves.empty()) {
		RelogoSpaceType* space = const_cast<RelogoSpaceType*> (_observer->space());
		vector<double> displacement;
		space->getDisplacement(oldLocation, _location, displacement);

		for (TiedSetType::iterator iter = fixedLeaves.begin(); iter != fixedLeaves.end(); ++iter) {
			Turtle* t = *iter;
			t->setxy(t->xCor() + displacement[0], t->yCor() + displacement[1]);
		}

		for (TiedSetType::iterator iter = freeLeaves.begin(); iter != freeLeaves.end(); ++iter) {
			Turtle* t = *iter;
			t->setxy(t->xCor() + displacement[0], t->yCor() + displacement[1]);
		}
	}
}

bool Turtle::canMoveQ(double distance) const {
	double xd = _location[0] + distance * dx();
	double yd = _location[1] + distance * dy();
	const RelogoSpaceType* space = _observer->space();
	try {
		vector<double> out(2, 0);
		space->translate(_location, Point<double> (xd, yd), out);
	} catch (out_of_range ex) {
		return false;
	}
	return true;
}

void Turtle::moveTo(Turtle* turtle) {
	setxy(turtle->_location[0], turtle->_location[1]);
}

void Turtle::moveTo(Patch* patch) {
	setxy(patch->xCor(), patch->yCor());
}

void Turtle::move(double distance) {
	if (!moved) {
		moved = true;
		// convert to relogo angle where 0 is north, rather then repast where 0 is east
		vector<double> angles(2, 0);
		angles[0] = repast::PI / 2 - repast::toRadians(_heading);
		Point<double> oldLocation(_location.coords());

		// cast away the constness so that we can move, normally const so that
		// users cannot move using the space directly
		RelogoSpaceType* space = const_cast<RelogoSpaceType*> (_observer->space());

		std::vector<double> displacement = calculateDisplacement<double> (2, 0, distance, angles);
		std::vector<double> newPos(2, 0);

		space->translate(_location, Point<double> (displacement), newPos);
		std::vector<double> transformedCoords(newPos.size(), 0);
		space->transform(newPos, transformedCoords);
		if (transformedCoords[0] != _location.getX() || transformedCoords[1] != _location.getY()) {
			_location = Point<double> (transformedCoords);
			std::vector<int> gridPt(2, 0);
			std::vector<double> spacePt(_location.coords());
			if (_observer->spacePtToGridPt(spacePt, gridPt)) {
				_location = Point<double> (spacePt);
			}

			space->moveTo(getId(), _location);

			// cast away the constness so that we can move
			// needs to const so that users can move using the grid
			RelogoGridType* grid = const_cast<RelogoGridType*> (_observer->grid());
			grid->moveTo(getId(), Point<int> (gridPt));
			moveTiedTurtles(oldLocation);
		}
		moved = false;
	}
}

void Turtle::moveTiedTurtle(Turtle* t, float angleTurned) {
	float angle = towards(t) + angleTurned;
	// convert so that 0 is north rather than east
	double tmp = fmod(angle, 360);
	angle = tmp < 0 ? tmp + 360 : tmp;
	double distanceToLeaf = distance(t);
	vector<double> displacement = calcDisplacementFromHeadingDistance(angle, distanceToLeaf);
	double x = displacement[0] + xCor();
	double y = displacement[1] + yCor();
	t->setxy(x, y);
}

float Turtle::towards(const Point<double>& pt) const {
	const RelogoSpaceType* space = _observer->space();
	vector<double> displacement(2, 0);
	space->getDisplacement(_location, pt, displacement);
	return (float) angleFromDisplacement(displacement[0], displacement[1]);
}

float Turtle::towards(RelogoAgent* agent) const {
	return towards(Point<double> (agent->xCor(), agent->yCor()));
}

float Turtle::towardsxy(double x, double y) const {
	return towards(Point<double> (x, y));
}

double Turtle::distance(Turtle* turtle) const {
	const RelogoSpaceType* space = _observer->space();
	return space->getDistance(_location, turtle->_location);
}

double Turtle::dx() const {
	return sin(_heading * PI / 180);
}

double Turtle::dy() const {
	return cos(_heading * PI / 180);
}

void Turtle::face(Turtle* turtle) {
	if (turtle->_location != _location) {
		heading(towards(turtle));
	}
}

void Turtle::face(Patch* patch) {
	if (!(xCor() == patch->pxCor() && yCor() == patch->pyCor())) {
		heading(towards(patch));
	}
}

void Turtle::facexy(double nx, double ny) {
	if (!(xCor() == nx && yCor() == ny)) {
		heading(towardsxy(nx, ny));
	}
}

void Turtle::left(float degrees) {
	heading(_heading - degrees);
}

void Turtle::lt(float degrees) {
	left(degrees);
}

void Turtle::heading(float heading) {
	if (!moved) {
		moved = true;
		float oldHeading = _heading;
		float tmp = fmodf(heading, 360);
		_heading = (tmp < 0) ? tmp + 360 : tmp;

		if (!fixedLeaves.empty() || !freeLeaves.empty()) {
			float angleTurned = subtractHeadings(_heading, oldHeading);

			for (TiedSetType::iterator iter = freeLeaves.begin(); iter != freeLeaves.end(); ++iter) {
				Turtle* t = *iter;
				moveTiedTurtle(t, angleTurned);
			}

			for (TiedSetType::iterator iter = fixedLeaves.begin(); iter != fixedLeaves.end(); ++iter) {
				Turtle* t = *iter;
				moveTiedTurtle(t, angleTurned);
				t->heading(t->heading() + angleTurned);
			}
		}
		moved = false;
	}

}

int Turtle::pxCor() const {
	return doubleCoordToInt(_location[0]);
}

int Turtle::pyCor() const {
	return doubleCoordToInt(_location[1]);
}

void Turtle::createLinkFrom(Turtle* turtle, const std::string& network) {
	_observer->createLink(turtle, this, network);
}

void Turtle::createLinkWith(Turtle* turtle, const std::string& network) {
	_observer->createLink(this, turtle, network);
}

void Turtle::createLinkTo(Turtle* turtle, const std::string& network) {
	_observer->createLink(this, turtle, network);
}

boost::shared_ptr<RelogoLink> Turtle::inLinkFrom(Turtle* turtle, const std::string& name) {
	return _observer->link(turtle, this, name);
}

boost::shared_ptr<RelogoLink> Turtle::outLinkTo(Turtle* turtle, const std::string& name) {
	return _observer->link(this, turtle, name);
}

boost::shared_ptr<RelogoLink> Turtle::linkWith(Turtle* turtle, const std::string& name) {
	return _observer->link(this, turtle, name);
}

bool Turtle::linkNeighborQ(Turtle* turtle, const std::string& name) {
	boost::shared_ptr<RelogoLink> link(_observer->link(this, turtle, name));
	if (link == 0)
		return _observer->link(turtle, this, name) != 0;
	return true;
}

bool Turtle::inLinkNeighborQ(Turtle* turtle, const std::string& name) {
	return _observer->link(turtle, this, name) != 0;
}

bool Turtle::outLinkNeighborQ(Turtle* turtle, const std::string& name) {
	return _observer->link(this, turtle, name) != 0;
}

}
}

