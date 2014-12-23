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
 *  Schedule.h
 *
 *  Created on: Jan 5, 2009
 *      Author: nick
 */

#ifndef SCHEDULE_H_
#define SCHEDULE_H_

#include <vector>
#include <queue>
#include <boost/mpi/communicator.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>

namespace repast {

/**
 * Functor interface.
 */
class Functor {
public:
	virtual ~Functor() = 0;
	virtual void operator()() = 0;
};

/**
 * Adapts a no-arg method call on an object instance
 * to a Functor interface. This is used by the Schedule
 * code to schedule method calls on objects.
 *
 * @tparam T the object type on which the call will be made.
 */
template<typename T>
class MethodFunctor: public Functor {
private:
	void (T::*fptr)();
	T *obj;
public:
	MethodFunctor(T *_obj, void(T::*_fptr)()) :
		fptr(_fptr), obj(_obj) {
	}
	;
	~MethodFunctor() {
	}
	;
	void operator()() {
		(obj->*fptr)();
	}
};

/**
 * General class linking a function pointer to a specific tick.
 */
class RepastEvent {

public:
	double tick;
	boost::shared_ptr<Functor> func_ptr;

	virtual ~RepastEvent();

};

class EventCompare;

/**
 * The object that is placed (scheduled) in the priority queue for execution.
 */
class ScheduledEvent {

protected:
	RepastEvent *event;
	double start;

public:
	friend class EventCompare;
	ScheduledEvent(double, RepastEvent *);
	virtual ~ScheduledEvent();
	/**
	 * Returns true if this event is rescheduled on the specified queue, otherwise false.
	 */
	virtual bool reschedule(std::priority_queue<ScheduledEvent *, std::vector<ScheduledEvent*>, EventCompare>&) = 0;

	/**
	 * Gets the RepastEvent that this ScheduleEvent wraps.
	 */
	RepastEvent* get_event() {
		return event;
	}
	;
};

/**
 * ScheduledEvent that will only execute only once.
 */
class OneTimeEvent: public ScheduledEvent {

public:
	OneTimeEvent(double, RepastEvent*);
	~OneTimeEvent();
	/**
	 * Always returns false, as it does not reschedule itself.
	 */
	virtual bool reschedule(std::priority_queue<ScheduledEvent *, std::vector<ScheduledEvent*>, EventCompare>&);
};

/**
 * ScheduledEvent that executes repeatedly. This will
 * reschedule itself repeatedly at the appropriate interval.
 */
class RepeatingEvent: public ScheduledEvent {

private:
	double interval;

public:
	RepeatingEvent(double start, double _interval, RepastEvent*);
	~RepeatingEvent();
	virtual bool reschedule(std::priority_queue<ScheduledEvent *, std::vector<ScheduledEvent*>, EventCompare>&);
};

/**
 * Compares ScheduledEvents based on their tick times.
 */
class EventCompare {
public:
	int operator()(const ScheduledEvent* one, const ScheduledEvent* two) {
		double tick1 = one->event->tick;
		double tick2 = two->event->tick;
		return tick1 > tick2 ? 1 : 0;
	}
};

/**
 * The simulation schedule queue. This wraps a priority queue
 * to schedule repast ScheduledEvents.
 */
class Schedule {
private:
	typedef std::priority_queue<ScheduledEvent *, std::vector<ScheduledEvent*>, EventCompare> QueueType;
	QueueType queue;
	double currentTick;

public:
	/**
	 * Typedef of for the functors that get scheduled.
	 */
	typedef boost::shared_ptr<Functor> FunctorPtr;
	virtual ~Schedule();

	/**
	 * Schedule the specified functor to execute once at the specified tick.
	 *
	 * @param at the tick to execute at
	 * @param functor the functor to schedule
	 *
	 * @return the event that has been scheduled
	 */
	ScheduledEvent* schedule_event(double at, FunctorPtr functor);

	/**
	 * Schedules the specified functor to execute start at start, and at the specified interval
	 * thereafter.
	 *
	 * @param start
	 * @param interval
	 * @param func
	 *
	 * @return the event that has been scheduled
	 */
	ScheduledEvent* schedule_event(double start, double interval, FunctorPtr func);
	void execute();

	/**
	 * Gets the current simulation tick.
	 *
	 * @return the current simulation tick.
	 */
	double getCurrentTick() const {
		return currentTick;
	}
	;

	/**
	 * Gets the next tick at which the next events will be executed.
	 *
	 * @return the next tick at which the next events will be executed.
	 */
	double getNextTick() const {
		if (queue.empty())
			return -1;
		return queue.top()->get_event()->tick;
	}
	;
};

/**
 * Runs the Schedule by popping events off of the Schedule and executing them;
 * also provides methods for scheduling events. Simulation events should be
 * scheduled for execution using this class which is accessible via
 * RepastProcess::instance()->getScheduleRunner()
 */
class ScheduleRunner: public boost::noncopyable {

private:

	bool go;
	Schedule schedule_;
	double globalNextTick, localNextTick;
	void nextTick();
	boost::mpi::communicator* comm;
	std::vector<boost::shared_ptr<Functor> > endEvents;

public:
	ScheduleRunner(boost::mpi::communicator* communicator);
	~ScheduleRunner();

	/**
	 * Schedules the Functor to execute at the specified tick.
	 *
	 * @param at the time to execute at
	 * @param func the functor to execute
	 *
	 * @return the event that was scheduled for the func
	 */
	ScheduledEvent* scheduleEvent(double at, Schedule::FunctorPtr func);

	/**
	 * Schedules the Functor to execute at the specified start tick
	 * and every interval thereafter.
	 *
	 * @param start the time to start at
	 * @param interval the interval to execute at
	 * @param func the functor to execute
	 *
	 * @return the event that was scheduled for the func
	 */
	ScheduledEvent* scheduleEvent(double start, double interval, Schedule::FunctorPtr func);

	/**
	 * Schedules the specified functor to execute when the simulation ends.
	 *
	 * @param func the functor to execute when the simulatione ends
	 */
	void scheduleEndEvent(Schedule::FunctorPtr func);

	/**
	 * Schedules the simulation to stop at the specified tick.
	 *
	 * @param at the tick at which the simulation should stop
	 */
	void scheduleStop(double at);

	/**
	 * Starts and runs the simulation schedule.
	 */
	void run();

	/**
	 * Gets the current tick.
	 *
	 * @return the current tick
	 */
	double currentTick() {
		return schedule_.getCurrentTick();
	}

	/**
	 * Stops the simulation.
	 */
	void stop();

	/**
	 * Gets the schedule executed by this simulation runner.
	 *
	 * @return the schedule used by this simulation runner.
	 */
	const Schedule& schedule() {
		return schedule_;
	}
};

}

#endif /* SCHEDULE_H_ */
