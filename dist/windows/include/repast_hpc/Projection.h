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
 *  Projection.h
 *
 *  Created on: Aug 5, 2010
 *      Author: nick
 */

#ifndef PROJECTION_H_
#define PROJECTION_H_

#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>

namespace repast {

template<typename T>
class Context;

/**
 * Abstract base class for Projections.
 */
template<typename T>
class Projection : public boost::noncopyable {

	friend class Context<T> ;

protected:

	std::string name_;
	virtual bool addAgent(boost::shared_ptr<T> agent) = 0;
	virtual void removeAgent(T* agent) = 0;

public:

	/**
	 * Creates a projection with specified name.
	 *
	 * @param name the name of the projection. This must be unique
	 * across projections
	 */
	Projection(std::string name) :
		name_(name) {
	}

	virtual ~Projection() {
	}

	/**
	 * Gets the name of this projection.
	 */
	const std::string name() const {
		return name_;
	}

};

}

#endif /* PROJECTION_H_ */
