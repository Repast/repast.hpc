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
 *  Point.h
 *
 *  Created on: Apr 1, 2010
 *      Author: nick
 */

#ifndef POINT_H_
#define POINT_H_

#include <vector>
#include <stdexcept>
#include <iostream>
#include <algorithm>
#include <math.h>

#include <boost/serialization/access.hpp>
#include <boost/functional/hash.hpp>

#include "RepastErrors.h"

namespace repast {

/**
 * A N-dimensional Point representation.
 */
template<typename T>
class Point;

/**
 * Class that allows retrieval of hash value for Point objects.
 */
template<typename T>
struct HashGridPoint {
	std::size_t operator()(const Point<T>& pt) const {
		return pt.hash;
	}
};

template<typename T>
bool operator==(const Point<T> &one, const Point<T> &two);

template<typename T>
std::ostream& operator<<(std::ostream& os, const Point<T>& pt);

/**
 * N dimensional point for addressing matrix locations.
 *
 * @param T a numeric type. In repast and relogo these are limited to int
 * and double.
 */
template<typename T>
class Point {

private:
	friend bool operator==<> (const Point<T> &one, const Point<T> &two);
	friend std::ostream& operator<<<> (std::ostream& os, const Point<T>& pt);
	friend struct HashGridPoint<T> ;

	void calcHash();

	std::vector<T> point;
	size_t hash;

	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive& ar, const unsigned int version) {
		ar & point;
		ar & hash;
	}

public:

	typedef typename std::vector<T>::const_iterator const_iterator;

	/**
	 * Creates a one dimensional point with the specified value.
	 *
	 * @param x the x coordinate of the point
	 */
	explicit Point(T x);

	/**
	 * Creates a two dimensional point with the specified values.
	 *
	 * @param x the x coordinate of the point
	 * @param y the y coordinate of the point
	 */
	Point(T x, T y);

	/**
	 * Creates a three dimensional point with the specified values.
	 *
	 * @param x the x coordinate of the point
	 * @param y the y coordinate of the point
	 * @param z the z coordinate of the point
	 */
	Point(T x, T y, T z);

	/**
	 * Creates a multi-dimensional point with the specified values.
	 *
	 * @param coordinates the coordinate values of the point. The first
	 * element will be x, the second y and so on.
	 */
	Point(std::vector<T> coordinates);

	/**
	 * Gets the x coordinate of the point.
	 *
	 * @return the x coordinate of the point.
	 */
	T getX() const;

	/**
	 * Gets the y coordinate of the point.
	 *
	 * @return the y coordinate of the point.
	 *
	 * @throws an out_of_range exception if this GridPoint has less than 2
	 * dimensions.
	 */
	T getY() const;

	/**
	 * Gets the z coordinate of the point.
	 *
	 * @return the z coordinate of the point.
	 *
	 * @throws an out_of_range exception if this GridPoint has less than 3
	 * dimensions.
	 */
	T getZ() const;

	/**
	 * Gets the coodinate of the point in the specified dimension.
	 *
	 * @param coordIndex the dimension of the point to get the coordinate for.
	 * X is the first, y is the second and so on.
	 *
	 * @return the coordinate of the point in the specified dimension.
	 *
	 * @throws an out_of_range exception if this GridPoint has doesn't
	 * have the specified dimension.
	 */
	T getCoordinate(int coordIndex) const;

	/**
	 * Adds the specified GridPoint to this GridPoint. This GridPoint
	 * contains the result.
	 *
	 * @throws invalid_argument exception if the pt doesn't have the same
	 * number of dimensions as this GridPoint.
	 */
	void add(const Point<T> &pt);

	/**
	 * Gets the number of dimensions of this point.
	 *
	 * @return the number of dimensions of this point.
	 */
	size_t dimensionCount() const {
		return point.size();
	}

	/**
	 * Gets the coordinate value at the specified index.
	 *
	 * @param index the dimension of the point to get the coordinate for.
	 * X is the first, y is the second and so on.
	 *
	 * @return the coordinate of the point in the specified dimension.
	 */
	const T& operator[](size_t index) const {
		return point[index];
	}

	/**
	 * Gets the coordinate value at the specified index.
	 *
	 * @param index the dimension of the point to get the coordinate for.
	 * X is the first, y is the second and so on.
	 *
	 * @return the coordinate of the point in the specified dimension.
	 */
	T& operator[](size_t index) {
		return point[index];
	}

	/**
	 * Gets the coordinates of this point as a vector.
	 *
	 * @return a vector containing the coordinates of this point.
	 */
	const std::vector<T>& coords() const {
		return point;
	}

	/**
	 * Gets the start of an iterator over the coordinates of this point.
	 *
	 * @return the start of an iterator over the coordinates of this point.
	 */
	const_iterator begin() const {
		return point.begin();
	}

	/**
	 * Gets the end of an iterator over the coordinates of this point.
	 *
	 * @return the end of an iterator over the coordinates of this point.
	 */
	const_iterator end() const {
		return point.end();
	}

	/**
	 * Copies the point into the specified vector. Assumes the
	 * array is the same length as this GridPoint.
	 *
	 * @param [out] the vector to copy the point coordinates into
	 */
	void copy(std::vector<T>& out) const;
};

template<typename T>
bool operator==(const Point<T> &one, const Point<T> &two);
template<typename T>
bool operator!=(const Point<T> &one, const Point<T> &two);
template<typename T>
std::ostream& operator<<(std::ostream& os, const Point<T>& pt);

template<typename T>
Point<T>::Point(T x) {
	point.push_back(x);
	calcHash();
}

template<typename T>
Point<T>::Point(T x, T y) {
	point.push_back(x);
	point.push_back(y);
	calcHash();
}

template<typename T>
Point<T>::Point(T x, T y, T z) {
	point.push_back(x);
	point.push_back(y);
	point.push_back(z);
	calcHash();
}

template<typename T>
Point<T>::Point(std::vector<T> coordinates) :
	point(coordinates.size(), 0) {
	std::copy(coordinates.begin(), coordinates.end(), point.begin());
	calcHash();
}

template<typename T>
void Point<T>::calcHash() {
	hash = 17;
	boost::hash<T> hasher;
	for (size_t i = 0; i < point.size(); i++) {
		hash = 37 * hash + hasher(point[i]);
	}
}

template<typename T>
T Point<T>::getX() const {
	return point.at(0);
}

template<typename T>
T Point<T>::getY() const {
	return point.at(1);
}

template<typename T>
T Point<T>::getZ() const {
	return point.at(2);
}

template<typename T>
T Point<T>::getCoordinate(int coordIndex) const {
	return point.at(coordIndex);
}

template<typename T>
void Point<T>::copy(std::vector<T>& out) const {
	std::copy(point.begin(), point.end(), out.begin());
}

template<typename T>
bool operator==(const Point<T> &one, const Point<T> &two) {
	return one.point == two.point;
}

template<typename T>
bool operator!=(const Point<T> &one, const Point<T> &two) {
	return !(one == two);
}

template<typename T>
void Point<T>::add(const Point<T> &pt) {
	if (pt.dimensionCount() != dimensionCount()) throw Repast_Error_35<Point<T> >(*this, pt); // Points do not have same number of dimensions

	for (size_t i = 0; i < point.size(); i++) {
		point[i] += pt.getCoordinate(i);
	}
}

template<typename T>
std::ostream& operator<<(std::ostream& os, const Point<T>& pt) {
	os << "Point[";
	for (size_t i = 0; i < pt.point.size(); i++) {
		if (i > 0)
			os << ", ";
		os << pt.point[i];
	}
	os << "]";
	return os;
}

template<typename T>
bool operator<(const Point<T>& one, const Point<T>& two) {
	return std::lexicographical_compare(one.begin(), one.end(), two.begin(), two.end());
}

}

#endif /* POINT_H_ */

