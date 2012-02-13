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
 * Objects.h
 *
 *  Created on: Jul 30, 2010
 *      Author: nick
 */

#ifndef OBJECTS_H_
#define OBJECTS_H_

#include "relogo/Turtle.h"
#include "relogo/Patch.h"

using namespace repast;
using namespace repast::relogo;

class MyPatch: public Patch {

private:
	int _val;

public:
	MyPatch(repast::AgentId id, Observer* obs) :
		Patch(id, obs), _val(0) {
	}
	virtual ~MyPatch() {
	}

	int val() const {
		return _val;
	}

	void val(int newVal) {
		_val = newVal;
	}
};

struct PatchCreator {

	MyPatch* operator()(AgentId id, Observer* obs) {
		return new MyPatch(id, obs);
	}
};

class MyObserver: public Observer {

public:
	MyObserver() {
	}
	virtual ~MyObserver() {
	}

	virtual void go() {
	}
	virtual void setup() {
	}
};

class MyTurtle: public Turtle {

public:
	MyTurtle(repast::AgentId id, Observer* obs) :
		Turtle(id, obs) {
	}
	~MyTurtle() {
	}
};

class Worker: public Turtle {

private:
	int _status;

public:
	Worker(repast::AgentId id, Observer* obs, int status) :
		Turtle(id, obs), _status(status) {
	}
	~Worker() {
	}

	int status() const {
		return _status;
	}
};

class WorkerCreator {

private:
	repast::IntUniformGenerator _gen;

public:
	WorkerCreator() :
		_gen(Random::instance()->createUniIntGenerator(4, 21)) {
	}

	Worker* operator()(AgentId id, Observer* obs) {
		return new Worker(id, obs, (int) _gen.next());

	}
};

class LinkCreator {
public:
	RelogoLink* operator()(RelogoAgent* source, RelogoAgent* target) {
		return new RelogoLink(source, target, 12);
	}
};

#endif /* OBJECTS_H_ */
