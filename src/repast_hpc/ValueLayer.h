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
 *  ValueLayer.h
 *
 *  Created on: Apr 1, 2010
 *      Author: nick
 */

#ifndef VALUELAYER_H_
#define VALUELAYER_H_

#include <boost/noncopyable.hpp>

#include "Point.h"
#include "GridDimensions.h"
#include "matrix.h"

#include <functional>

namespace repast {

/**
 * Base implementation of a ValueLayer. A ValueLayer
 * stores values by location.
 */
class BaseValueLayer: public boost::noncopyable {

protected:
	std::string _name;

public:
	/**
	 * Creates a BaseValueLayer with the specified name.
	 */
	BaseValueLayer(const std::string& name);
	virtual ~BaseValueLayer() {
	}

	/**
	 * Gets the value layer's name.
	 *
	 * @return the name of the value layer.
	 */
	std::string name() const {
		return _name;
	}
};

/**
 * A collection that stores values at point locations.
 *
 * @tparam ValueType the  type stored by the value layer.
 * @tparam the coordinate type (int or double) of the point locations.
 */
template<typename ValueType, typename PointType>
class ValueLayer: public BaseValueLayer {

protected:
	GridDimensions _dimensions;

	/**
	 * Translates pt by dimensions origin.
	 */
	void translate(std::vector<PointType>& pt);

public:
	ValueLayer(const std::string& name, const GridDimensions& dimensions);
	virtual ~ValueLayer() {
	}

	/**
	 * Gets the value at the specified point. If no value
	 * has been set at the specified point then this returns
	 * some default value. Subclasses will determine the default value.
	 *
	 * param pt the location to get the value of
	 *
	 * @return the value at the specified point, or if no value has been
	 * set, then some default value.
	 */
	virtual ValueType& get(const Point<PointType>& pt) = 0;

	/**
	 * Sets the value at the specified point.
	 *
	 * @param value the value
	 * @param pt the point where the value should be stored
	 *
	 */
	virtual void set(const ValueType& value, const Point<PointType>& pt) = 0;

	/**
	 * Gets the value at the specified point. If no value
	 * has been set at the specified point then this returns
	 * some default value. Subclasses will determine the default value.
	 *
	 * param pt the location to get the value of
	 *
	 * @return the value at the specified point, or if no value has been
	 * set, then some default value.
	 */
	ValueType& operator[](const Point<PointType>& pt);

	/**
	 * Gets the value at the specified point. If no value
	 * has been set at the specified point then this returns
	 * some default value. Subclasses will determine the default value.
	 *
	 * param pt the location to get the value of
	 *
	 * @return the value at the specified point, or if no value has been
	 * set, then some default value.
	 */
	const ValueType& operator[](const Point<PointType>& pt) const;

	/**
	 * Gets the dimensions of this ValueLayer.
	 *
	 * @return the dimensions of this ValueLayer.
	 */
	const GridDimensions dimensions() const {
		return _dimensions;
	}

	/**
	 * Gets the extents of this ValueLayer.
	 *
	 * @return the extents of this ValueLayer.
	 */
	const Point<double> shape() const {
		return _dimensions.extents();
	}
};

template<typename ValueType, typename PointType>
ValueLayer<ValueType, PointType>::ValueLayer(const std::string& name, const GridDimensions& dimensions) :
	BaseValueLayer(name), _dimensions(dimensions) {
}

template<typename ValueType, typename PointType>
void ValueLayer<ValueType, PointType>::translate(std::vector<PointType>& pt) {
	for (size_t i = 0; i < _dimensions.dimensionCount(); i++) {
		pt[i] -= _dimensions.origin(i);
	}
}

template<typename ValueType, typename PointType>
ValueType& ValueLayer<ValueType, PointType>::operator[](const Point<PointType>& pt) {
	return get(pt);
}

template<typename ValueType, typename PointType>
const ValueType& ValueLayer<ValueType, PointType>::operator[](const Point<PointType>& pt) const {
	return get(pt);
}

/**
 * Creates ValueLayer whose location coordinates are ints.
 *
 * @tparam ValueType the type of what the value layer stores.
 * @tparam Borders the type of borders (wrapped / periodic, strict). Border types
 * can be found in GridComponents.h
 */
template<typename ValueType, typename Borders>
class DiscreteValueLayer: public ValueLayer<ValueType, int> {

private:
	Matrix<ValueType>* matrix;
	bool _dense;
	Borders borders;

	void create(bool dense, Matrix<ValueType>* other);

public:
	DiscreteValueLayer(const DiscreteValueLayer<ValueType, Borders>& other);
	DiscreteValueLayer& operator=(const DiscreteValueLayer<ValueType, Borders>& rhs);

	/**
	 * Creates a DiscreteValueLayer whose cells contain a default value of ValueType()
	 * with the specified dimensions.
	 *
	 * @param name the name of the DiscreteValueLayer
	 * @param dimension the dimensions of the DiscreteValueLayer
	 * @param dense whether or not the ValueLayer will be densely populated or not
	 * @param defaultValue the default value to return if no value has been
	 * set of a location. The default is the result of ValueType().
	 */
	DiscreteValueLayer(const std::string& name, const GridDimensions& dimensions, bool dense,
			const ValueType& defaultValue = ValueType());
	~DiscreteValueLayer();

	/**
	 * Gets the value at the specified point. If no value
	 * has been set at the specified point then this returns
	 * the default value.
	 *
	 * param pt the location to get the value of
	 *
	 * @return the value at the specified point, or if no value has been
	 * set, then the default value.
	 */
	ValueType& get(const Point<int>& pt);

	/**
	 * Sets the value at the specified point.
	 *
	 * @param value the value
	 * @param pt the point where the value should be stored
	 *
	 */
	void set(const ValueType& value, const Point<int>& pt);
};

template<typename ValueType, typename Borders>
void DiscreteValueLayer<ValueType, Borders>::create(bool dense, Matrix<ValueType>* other) {
	if (dense) {
		matrix = new DenseMatrix<ValueType> (*(dynamic_cast<DenseMatrix<ValueType>*> (other)));
	} else {
		matrix = new SparseMatrix<ValueType> (*(dynamic_cast<SparseMatrix<ValueType>*> (other)));
	}

}

template<typename ValueType, typename Borders>
DiscreteValueLayer<ValueType, Borders>::DiscreteValueLayer(const DiscreteValueLayer<ValueType, Borders>& other) :
	ValueLayer<ValueType, int> (other.name(), other.dimensions()), _dense(other._dense), borders(other.dimensions()) {
	create(_dense, other.matrix);
}

template<typename ValueType, typename Borders>
DiscreteValueLayer<ValueType, Borders>& DiscreteValueLayer<ValueType, Borders>::operator=(const DiscreteValueLayer<
		ValueType, Borders>& rhs) {
	if (&rhs != this) {
		delete matrix;
		ValueLayer<ValueType, int>::_name = rhs.name();
		ValueLayer<ValueType, int>::_dimensions = rhs.dimensions();
		_dense = rhs._dense;
		create(_dense, rhs.matrix);
		borders = Borders(ValueLayer<ValueType, int>::_dimensions);
	}
	return *this;
}

template<typename ValueType, typename Borders>
DiscreteValueLayer<ValueType, Borders>::~DiscreteValueLayer() {
	delete matrix;
}

template<typename ValueType, typename Borders>
DiscreteValueLayer<ValueType, Borders>::DiscreteValueLayer(const std::string& name, const GridDimensions& dimensions,
		bool dense, const ValueType& defaultValue) :
	ValueLayer<ValueType, int> (name, dimensions), _dense(dense) , borders(Borders(ValueLayer<ValueType, int>::_dimensions)) {

	// this is dangerous but at some point dimensions has been converted to take
	// double extents and we don't have time to convert everything
	const std::vector<double>& coords = dimensions.extents().coords();
	std::vector<int> converted_coords;
	for (double d : coords) {
		converted_coords.push_back(static_cast<int>(d));
	}
	if (dense) {
		matrix = new DenseMatrix<ValueType> (Point<int>(converted_coords), defaultValue);
	} else {
		matrix = new SparseMatrix<ValueType> (Point<int>(converted_coords), defaultValue);
	}
}

template<typename ValueType, typename Borders>
ValueType& DiscreteValueLayer<ValueType, Borders>::get(const Point<int>& pt) {
	std::vector<int> out(pt.dimensionCount());
	borders.transform(pt.coords(), out);
	if (_dense)
		ValueLayer<ValueType, int>::translate(out);
	return matrix->get(Point<int> (out));
}

template<typename ValueType, typename Borders>
void DiscreteValueLayer<ValueType, Borders>::set(const ValueType& value, const Point<int>& pt) {
	std::vector<int> out(pt.dimensionCount());
	borders.transform(pt.coords(), out);
	if (_dense)
		ValueLayer<ValueType, int>::translate(out);
	return matrix->set(value, Point<int> (out));
}

/**
 * Continous value layer whose location coordinates are double.
 *
 * @tparam ValueType the type of what the value layer stores.
 * @tparam Borders the type of borders (wrapped / periodic, strict). Border types
 * can be found in GridComponents.h
 */

template<typename ValueType, typename Borders>
class ContinuousValueLayer: public ValueLayer<ValueType, double> {

private:

	Borders borders;
	std::map<std::vector<double>, ValueType> values;
	ValueType _defaultValue;

	typedef typename std::map<std::vector<double>, ValueType>::iterator values_iter;

public:
	ContinuousValueLayer(const ContinuousValueLayer<ValueType, Borders>& other);
	ContinuousValueLayer& operator=(const ContinuousValueLayer<ValueType, Borders>& rhs);

	/**
	 * Creates a ContinuousValueLayer whose cells contain a default value of ValueType()
	 * with the specified dimensions.
	 *
	 * @param name the name of the ContinuousValueLayer
	 * @param dimension the dimensions of the ContinuousValueLayer
	 * @param dense whether or not the ValueLayer will be densely populated or not
	 * @param defaultValue the default value to return if no value has been
	 * set of a location. The default is the result of ValueType().
	 */
	ContinuousValueLayer(const std::string& name, const GridDimensions& dimensions, const ValueType& defaultValue =
			ValueType());
	~ContinuousValueLayer() {
	}

	/**
	 * Gets the value at the specified point. If no value
	 * has been set at the specified point then this returns
	 * the default value.
	 *
	 * param pt the location to get the value of
	 *
	 * @return the value at the specified point, or if no value has been
	 * set, then the default value.
	 */
	ValueType& get(const Point<double>& pt);

	/**
	 * Sets the value at the specified point.
	 *
	 * @param value the value
	 * @param pt the point where the value should be stored
	 *
	 */
	void set(const ValueType& value, const Point<double>& pt);
};


template<typename ValueType, typename Borders>
ContinuousValueLayer<ValueType, Borders>::ContinuousValueLayer(const ContinuousValueLayer<ValueType, Borders>& other) :
	ValueLayer<ValueType, double> (other._name, other._dimensions), values(other.values), _defaultValue(
			other._defaultValue), borders(other._dimensions) {
}

template<typename ValueType, typename Borders>
ContinuousValueLayer<ValueType, Borders>& ContinuousValueLayer<ValueType, Borders>::operator=(
		const ContinuousValueLayer<ValueType, Borders>& rhs) {

	if (&rhs != this) {
		values.clear();
		values.insert(rhs.values.begin(), rhs.values.end());
		ValueLayer<ValueType, double>::_name = rhs.name();
		ValueLayer<ValueType, double>::_dimensions = rhs.dimensions();
		_defaultValue = rhs._defaultValue;
		borders = Borders(ValueLayer<ValueType, int>::_dimensions);
	}
	return *this;
}

template<typename ValueType, typename Borders>
ContinuousValueLayer<ValueType, Borders>::ContinuousValueLayer(const std::string& name,
		const GridDimensions& dimensions, const ValueType& defaultValue) :
	ValueLayer<ValueType, double> (name, dimensions), _defaultValue(defaultValue), borders(ValueLayer<ValueType, double>::_dimensions) {
}

template<typename ValueType, typename Borders>
ValueType& ContinuousValueLayer<ValueType, Borders>::get(const Point<double>& pt) {
	std::vector<double> out(pt.dimensionCount());
	borders.transform(pt.coords(), out);
	// need to insert do an insert and return reference to
	// result so that assignment via [] uses a reference in
	// the map. insert inserts the entry if it doesn't exist
	// and returns the entry or the existing entry
	std::pair<values_iter, bool> res = values.insert(std::make_pair(out, _defaultValue));
	return res.first->second;
}

template<typename ValueType, typename Borders>
void ContinuousValueLayer<ValueType, Borders>::set(const ValueType& value, const Point<double>& pt) {
	std::vector<double> out(pt.dimensionCount());
	borders.transform(pt.coords(), out);
	values[out] = value;
}

}

#endif /* VALUELAYER_H_ */
