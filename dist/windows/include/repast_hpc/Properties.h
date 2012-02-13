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
 * Properties.h
 *
 *  Created on: Sep 25, 2009
 *      Author: nick
 */

#ifndef PROPERTIES_H_
#define PROPERTIES_H_

#include <map>
#include <string>
#include <boost/iterator/transform_iterator.hpp>

namespace repast {

// Unary function used in a transform_iterator that allows the map
// iterator to return the keys
struct KeyGetter: public std::unary_function<std::map<std::string, std::string>::value_type, std::string> {
	std::string operator()(const std::map<std::string, std::string>::value_type& value) const;
};

/**
 * Map type object that contains key, value string properties. A Properties
 * instance can be constructed from a file. Each line is a property with the
 * key and value separated by =. For example,
 *
 * some.property = 3<br>
 * another.property = hello
 */
class Properties {

private:
	std::map<std::string, std::string> map;

public:

	typedef boost::transform_iterator<KeyGetter, std::map<std::string, std::string>::const_iterator> key_iterator;

	/**
	 * Creates an empty Properties.
	 */
	Properties();

	/**
	 * Creates a new Properties using the properties defined in the specified file.
	 * Each line is a property with the key and value separated by =. For example,
	 *
	 * some.property = 3<br>
	 * another.property = hello
	 *
	 * @param file the properties file path
	 */
	Properties(const std::string& file);
	virtual ~Properties() {
	}

	/**
	 * Puts a property into this Properties with
	 * the specified key and value.
	 *
	 * @param key the property key
	 * @param value the property value
	 */
	void putProperty(const std::string& key, std::string value);

	/**
	 * Gets the property with the specified key.
	 *
	 * @param key the property key
	 *
	 * @return the value for that key, or an empty string
	 * if the property is not found.
	 */
	std::string getProperty(const std::string& key) const;

	/**
	 * Gets whether or not this Properties contains the specified key.
	 *
	 * @param the property key
	 */
	bool contains(const std::string& key) const;

	/**
	 * Gets the start of an iterator over this Properties' keys.
	 *
	 * @return the start of an iterator over this Properties' keys.
	 */
	key_iterator keys_begin() const {
		return key_iterator(map.begin());
	}

	/**
	 * Gets the end of an iterator over this Properties' keys.
	 *
	 * @return the end of an iterator over this Properties' keys.
	 */
	key_iterator keys_end() const {
		return key_iterator(map.end());
	}

	/**
	 * Gets the number of properties in this Properties.
	 *
	 * @return the number of properties in this Properties.
	 */
	int size() const {
		return map.size();
	}

};

}

#endif /* PROPERTIES_H_ */
