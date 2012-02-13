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
 *  Utilities.h
 *
 *  Created on: Oct 27, 2009
 *      Author: nick
 */

#ifndef UTILITIES_H_
#define UTILITIES_H_

#include <string>
#include <vector>
#include <time.h>

#include <boost/cstdint.hpp>

namespace repast {

/**
 * Simple timing class. Calling start() starts the timer,
 * calling stop returns the milliseconds since calling start.
 */
class Timer {

private:
	clock_t startTime;

public:
	Timer();

	/**
	 * Starts the timer.
	 */
	void start();

	/**
	 * Stops the timer and returns the number of milliseconds elapsed since
	 * calling start().
	 *
	 * @return the number of milliseconds elapsed since
	 * calling start().
	 */
	long double stop();
};

/**
 * Tokenizes str and puts the results into tokens. default delimiters is
 * " ".
 *
 * @param str the string to tokenize
 * @param [out] tokens returns the tokens
 * @param delimiter the string that delimits the tokens
 */
void tokenize(const std::string& str, std::vector<std::string>& tokens, const std::string& delimiters = " ");

/**
 * Trims white space from sides of str.
 *
 * @param str the string to trim
 *
 * @return the trimmed string.
 */
std::string trim(const std::string& str);

/**
 * Converts the string to a unsigned int.
 *
 * @param val the string representation of the int
 *
 * @return the unsigned int
 */
boost::uint32_t strToUInt(const std::string& val);


/**
 * Converts the string to a  int.
 *
 * @param val the string representation of the int
 *
 * @return the  int
 */
int strToInt(const std::string& val);

/**
 * Converts the string to a double.
 *
 * @param val the string representation of the double.
 *
 * @return the double.
 */
double strToDouble(const std::string& val);

}

#endif /* UTILITIES_H_ */
