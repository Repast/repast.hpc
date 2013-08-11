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
 *  Variable.h
 *
 *  Created on: Aug 23, 2010
 *      Author: nick
 */

#ifndef VARIABLE_H_
#define VARIABLE_H_

#include <fstream>
#include <vector>

namespace repast {

/**
 * Used in SVDataSet to manage and store the data.
 */
class Variable {

public:
	virtual ~Variable() {}

	/**
	 * Writes the data at the specified index to the specified ofstream.
	 *
	 * @param index the index of the data to write
	 * @param out the ofstream to write the data to
	 */
	virtual void write(size_t index, std::ofstream& out) = 0;


	/**
	 * Inserts all the doubles in the double array into the collection
	 * of data stored in this Variable.
	 *
	 * @param array the array to insert
	 * @param size the size of the array
	 */
	virtual void insert(double* array, size_t size) = 0;


	/**
	 * Inserts all the ints in the int array into the collection
	 * of data stored in this Variable.
	 *
	 * @param array the array to insert
	 * @param size the size of the array
	 */
	virtual void insert(int* array, size_t size) = 0;

	/**
	 * Clears this Variable of all the data stored in it.
	 */
	virtual void clear() = 0;

};

/**
 * Used in SVDataSet to manage integer data.
 */
class IntVariable: public Variable {

private:
	std::vector<int> data;

public:

	virtual void write(size_t index, std::ofstream& out);
	virtual void insert(double* array, size_t size);
	virtual void insert(int* array, size_t size);
	virtual void clear() {
		data.clear();
	}
};

/**
 * Used in SVDataSet to manage double data.
 */
class DoubleVariable: public Variable {

private:
	std::vector<double> data;

public:

	virtual void write(size_t index, std::ofstream& out);
	virtual void insert(double* array, size_t size);
	virtual void insert(int* array, size_t size);
	virtual void clear() {
		data.clear();
	}
};

}

#endif /* VARIABLE_H_ */
