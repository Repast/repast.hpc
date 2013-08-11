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
 *  NCDataSource.h
 *
 *  Created on: Oct 14, 2010
 *      Author: nick
 */

#ifndef NCDATASOURCE_H_
#define NCDATASOURCE_H_

namespace repast {

/**
 * Data source used internally by NCDataSets.
 */
class NCDataSource {

protected:
	std::string _name;

public:
	NCDataSource(std::string name) : _name(name) {}
	virtual ~NCDataSource() {};
	virtual void record() = 0;
	virtual void write(NcVar* var) = 0;

	virtual NcType ncType() = 0;

	const std::string name() const {
		return _name;
	}
};

/**
 * Base class for specialized int and double NcType classes
 */
template<typename T>
struct NcTypeTrait;

/**
 * Used for converting to NetCDF Data, double type
 */
template<>
struct NcTypeTrait<double> {
	const static NcType type = ncDouble;
};

/**
 * Used for converting to NetCDF Data, int type
 */
template<>
struct NcTypeTrait<int> {
	const static NcType type = ncInt;
};



}


#endif /* NCDATASOURCE_H_ */
