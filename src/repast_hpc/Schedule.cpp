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
 *  Schedule.cpp
 *
 *  Created on: Jan 5, 2009
 *      Author: nick
 */

#include "Schedule.h"
#include "Utilities.h"
#include "logger.h"

#include <iostream>
#include <boost/mpi.hpp>
#include <boost/lexical_cast.hpp>

namespace repast {

RepastEvent::~RepastEvent() {
//std::cout << func_ptr.use_count() << std::endl;
}

Functor::~Functor() {
}

ScheduledEvent::ScheduledEvent(double at, RepastEvent *evt) :
	event(evt), start(at) {
}

ScheduledEvent::~ScheduledEvent() {
	delete event;
}

OneTimeEvent::OneTimeEvent(double at, RepastEvent *evt) :
	ScheduledEvent(at, evt) {
}

OneTimeEvent::~OneTimeEvent() {
}

// does nothing as its a one time event
bool OneTimeEvent::reschedule(std::priority_queue<ScheduledEvent *, std::vector<ScheduledEvent*>, EventCompare>& queue) {
	return false;
}

RepeatingEvent::RepeatingEvent(double start, double _interval, RepastEvent *evt) :
	ScheduledEvent(start, evt), interval(_interval) {
}

RepeatingEvent::~RepeatingEvent() {
}

bool RepeatingEvent::reschedule(
		std::priority_queue<ScheduledEvent *, std::vector<ScheduledEvent*>, EventCompare>& queue) {
	event->tick += interval;
	queue.push(this);
	return true;
}

Schedule::~Schedule() {
	while (!queue.empty()) {
		ScheduledEvent *evt = queue.top();

		queue.pop();
		delete evt;
	}
}

ScheduledEvent* Schedule::schedule_event(double start, FunctorPtr func) {
	RepastEvent *evt = new RepastEvent();
	evt->func_ptr = func;
	evt->tick = start;
	OneTimeEvent *event = new OneTimeEvent(start, evt);
	queue.push(event);
	return event;
}

ScheduledEvent* Schedule::schedule_event(double start, double interval, FunctorPtr func) {
	RepastEvent *evt = new RepastEvent();
	evt->func_ptr = func;
	evt->tick = start;
	RepeatingEvent *event = new RepeatingEvent(start, interval, evt);
	queue.push(event);
	return event;
}

void Schedule::execute() {
	if (!queue.empty()) {
		ScheduledEvent *evt = queue.top();
		double next = evt->get_event()->tick;
		currentTick = next;
		//std::cout << "execute at: " << currentTick << std::endl;
		bool go = true;
		while (go) {
			queue.pop();
			Functor *func = evt->get_event()->func_ptr.get();
			(*func)();
			bool isLive = evt->reschedule(queue);
			if (!isLive) delete evt;
			if (queue.empty())
				go = false;
			else {
				evt = queue.top();
				go = evt->get_event()->tick == next;
			}
		}
	}
	//std::cout << "execute at: " << getCurrentTick() << std::endl;
}

ScheduleRunner::ScheduleRunner(boost::mpi::communicator* communicator) : go(true), comm(communicator) {
}

ScheduleRunner::~ScheduleRunner() {}

void ScheduleRunner::nextTick() {
	localNextTick = schedule_.getNextTick();
}

void ScheduleRunner::scheduleStop(double at) {
	MethodFunctor<ScheduleRunner> *mf = new MethodFunctor<ScheduleRunner> (this, &ScheduleRunner::stop);
	schedule_.schedule_event(at, Schedule::FunctorPtr(mf));
	nextTick();
}

ScheduledEvent* ScheduleRunner::scheduleEvent(double at, Schedule::FunctorPtr func) {
	ScheduledEvent *evt = schedule_.schedule_event(at, func);
	nextTick();
	return evt;
}

ScheduledEvent* ScheduleRunner::scheduleEvent(double start, double interval, Schedule::FunctorPtr func) {
	ScheduledEvent *evt = schedule_.schedule_event(start, interval, func);
	nextTick();
	return evt;
}

void ScheduleRunner::stop() {
	go = false;
}

void ScheduleRunner::scheduleEndEvent(Schedule::FunctorPtr func) {
	endEvents.push_back(func);
}

void ScheduleRunner::run() {
	//Timer timer;
	while (go) {
		//timer.start();
		all_reduce(*comm, localNextTick, globalNextTick, boost::mpi::minimum<double>());//&localNextTick, &globalNextTick, 1, MPI::DOUBLE, MPI::MIN);
		//Log4CL::instance()->get_logger("root").log(INFO, "schedule idle, time: " + boost::lexical_cast<std::string>(timer.stop()));
		if (localNextTick == globalNextTick)
			schedule_.execute();
		nextTick();
	}
	// execute end events
	for (size_t i = 0; i < endEvents.size(); i++) {
		(*endEvents[i])();
	}
}

}

