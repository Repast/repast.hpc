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
 *  io.cpp
 *
 *  Created on: Sep 25, 2009
 *      Author: nick
 */

#include "io.h"

#include <ctime>
#include <sstream>
#include <iomanip>

namespace repast {

using namespace std;

void str_trim(string &str) {
	size_t pos = str.find_last_not_of(" \t");
	if (pos != string::npos) {
		str.erase(pos + 1);
		pos = str.find_first_not_of(" \t");
		if (pos != string::npos) {
			str.erase(0, pos);
		}
	} else {
		str = "";
	}
}

void timestamp2(string& str) {
	struct tm *tmp;
	time_t t;

	t = time(NULL);
	tmp = localtime(&t);
	ostringstream os;

	os << setw(4);
	os << (tmp->tm_year + 1900);
	os << setfill('0') << setw(2);
	os << (tmp->tm_mon + 1);
	os << setfill('0') << setw(2);
	os << tmp->tm_mday;

	os << setw(2) << setfill('0');
	os << tmp->tm_hour;
	os << setw(2) << setfill('0') << tmp->tm_min;
	os << setw(2) << setfill('0') << tmp->tm_sec;
	str = os.str();
}

void timestamp(string& str) {
	struct tm *tmp;
	time_t t;

	t = time(NULL);
	tmp = localtime(&t);
	ostringstream os;
	os << setfill('0') << setw(2);
	os << tmp->tm_mday << ".";

	os << setfill('0') << setw(2);
	os << (tmp->tm_mon + 1) << ".";

	os << setw(4);
	os << (tmp->tm_year + 1900) << " ";

	os << setw(2) << setfill('0');
	os << tmp->tm_hour << ":";
	os << setw(2) << setfill('0') << tmp->tm_min << ":";
	os << setw(2) << setfill('0') << tmp->tm_sec;
	str = os.str();
}

}
