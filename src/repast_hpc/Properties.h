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
 *  Properties.h
 *
 *  Created on: Sep 25, 2009
 *      Author: nick
 */

#ifndef PROPERTIES_H_
#define PROPERTIES_H_

#include <fstream>


#include <iostream>
#include <map>
#include <string>

#include <boost/iterator/transform_iterator.hpp>
#include <boost/mpi/communicator.hpp>
#include <boost/filesystem.hpp>

#include "logger.h"

#define MAX_PROP_FILE_SIZE 16384

namespace repast {

/**
 * Unary function used in a transform_iterator that allows the map
 * iterator to return the keys
 */
struct KeyGetter: public std::unary_function<std::map<std::string, std::string>::value_type, std::string> {
  std::string operator()(const std::map<std::string, std::string>::value_type& value) const;
};

/**
 * Map type object that contains key, value(string) properties. A Properties
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
   * @param comm pointer to a communicator; if null (the default), all
   * processes read the properties file separately. If a communicator
   * is provided, rank 0 reads the file and broadcasts it to all
   * other ranks.
   * @param maxPropFileSize optional parameter; if the properties
   * file is larger than the default MAX_PROP_FILE_SIZE, the
   * new size can be passed here.
   */
  Properties(const std::string& file, boost::mpi::communicator* comm = 0, int maxPropFileSize = MAX_PROP_FILE_SIZE);

  /**
   * Creates a new Properties using the properties defined in the specified
   * file and any properties specified in Key=Val format in the argument
   * array. Properties in the argument array will supersede any in the properties
   * file.
   *
   * Each line in the properties file is a property with the key and value
   * separated by =. For example,
   *
   * some.property = 3<br>
   * another.property = hello
   *
   *
   * @param file the properties file path
   * @param argc count of the elements in the argv array
   * @param array of char* that may include Key=Value pairs. Elements with
   * no '=' are ignored.
   * @param comm pointer to a communicator; if null (the default), all
   * processes read the properties file separately. If a communicator
   * is provided, rank 0 reads the file and broadcasts it to all
   * other ranks.
   * @param maxPropFileSize optional parameter; if the properties
   * file is larger than the default MAX_PROP_FILE_SIZE, the
   * new size can be passed here.
   */
  Properties(const std::string& file, int argc, char** argv, boost::mpi::communicator* comm = 0, int maxPropFileSize = MAX_PROP_FILE_SIZE);

  /**
   * Creates a new Properties using the properties specified in Key=Val
   * format in the argument
   *
   * @param argc count of the elements in the argv array
   * @param array of char* that may include Key=Value pairs. Elements with
   * no '=' are ignored.
   */
  Properties(int argc, char** argv);

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
   * Puts a property into this Properties with
   * the specified key and value. Note that
   * even though the second argument can be passed
   * as a numeric value, it is stored as a string
   *
   * @param key the property key
   * @param value the property value
   */
  void putProperty(const std::string& key, long double value);


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
   * @param key the property key
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
   * Adds any properties defined in the specified file.
   * Each line is a property with the key and value separated by =. For example,
   *
   * some.property = 3<br>
   * another.property = hello
   *
   * @param file the properties file path
   * @param comm pointer to a communicator; if null (the default), all
   * processes read the properties file separately. If a communicator
   * is provided, rank 0 reads the file and broadcasts it to all
   * other ranks.
   */
  void readFile(const std::string& file, boost::mpi::communicator* comm = 0, int maxPropFileSize = MAX_PROP_FILE_SIZE);

  /**
   * Processes a char** array of the given size;
   * any component that has an equals sign is entered
   * as a property value, overriding any previous
   * entry read from the properies file
   *
   * @param argc the number of entries in the array
   * @param argv the array of char values to be mapped
   */
  void processCommandLineArguments(int argc, char **argv);

  /**
   * Gets the number of properties in this Properties.
   *
   * @return the number of properties in this Properties.
   */
  int size() const {
    return map.size();
  }

  /**
   * Writes the contents of the properties file to the specified
   * repast log (at 'INFO' log level)
   *
   * @param logName name of the log to use
   * @param keysToWrite optional; if included, writes only the
   * keys included in the vector and their values, in the
   * order they appear in the vector. Will write
   * blank values for any key name in the vector that is not
   * in the properties file. If not included, all properties
   * and their values are written, in map order.
   */
  void log(std::string logName, std::vector<std::string> *keysToWrite = 0){
    if(keysToWrite != 0){
      std::vector<std::string>::iterator iter    = keysToWrite->begin();
      std::vector<std::string>::iterator iterEnd = keysToWrite->end();
      while(iter != iterEnd){
        Log4CL::instance()->get_logger(logName).log(INFO, (*iter) + " = " + (getProperty(*iter)));
        iter++;
      }
    }
    else{
      std::map<std::string, std::string>::iterator iter    = map.begin();
      std::map<std::string, std::string>::iterator iterEnd = map.end();
      while(iter != iterEnd){
        Log4CL::instance()->get_logger(logName).log(INFO, iter->first + " = " + iter->second);
        iter++;
      }
    }
  }


  /**
   * Writes the contents of the properties file to the specified separated-value
   * file. If the file does not exist it is created and a header line is written
   * with the key values.
   *
   * @param fileName name
   */
  void writeToSVFile(std::string fileName, std::string separator = ",");

  /**
   * Writes the contents of the properties file to the specified separated-value
   * file. If the file does not exist it is created and a header line is written
   * with the key values.
   *
   * @param fileName name
   */
  void writeToSVFile(std::string fileName, std::vector<std::string> &keysToWrite, std::string separator = ",");


};

}

#endif /* PROPERTIES_H_ */
