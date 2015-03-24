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
 *  Properties.cpp
 *
 *  Created on: Sep 25, 2009
 *      Author: nick
 */

#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include "Properties.h"
#include "io.h"
#include "RepastErrors.h"

#include "boost/serialization/map.hpp"
#include "boost/mpi/collectives.hpp"


using namespace std;

namespace repast {

string KeyGetter::operator()(const map<string, string>::value_type& value) const {
  return value.first;
}

Properties::Properties() {
}

Properties::Properties(const string& file, boost::mpi::communicator* comm, int maxPropFileSize) {
  readFile(file, comm, maxPropFileSize);
}

Properties::Properties(const string& file, int argc, char** argv, boost::mpi::communicator* comm, int maxPropFileSize) {
  readFile(file, comm, maxPropFileSize);
  processCommandLineArguments(argc, argv);
}

Properties::Properties(int argc, char** argv) {
  this->processCommandLineArguments(argc, argv);
}

void Properties::putProperty(const string& key, string value) {
  map[key] = value;
}

void Properties::putProperty(const string& key, long double value){
  map[key] = std::to_string(value);
}

bool Properties::contains(const string& key) const {
  return map.find(key) != map.end();
}

string Properties::getProperty(const string& key) const {
  std::map<string, string>::const_iterator iter = map.find(key);
  if (iter == map.end())
    return "";
  else
    return iter->second;
}

void Properties::readFile(const std::string& file, boost::mpi::communicator* comm, int maxPropFileSize){

  char* PROPFILEBUFFER = new char[maxPropFileSize];                            // All procs allocate memory for the properties file

  if(comm == 0 || comm->rank() == 0){                                          // If no communicator is passed, all ranks read props file
    ifstream fileInStream(file.c_str(), ios_base::in);
    if (fileInStream.is_open()){
      fileInStream.read(PROPFILEBUFFER, maxPropFileSize);
      // Check if fail:
      if(fileInStream.gcount() >= (maxPropFileSize - 2)){
        throw Repast_Error_52(maxPropFileSize, fileInStream.gcount(), file); // Properties file exceeds maximum allowed size
      }
      PROPFILEBUFFER[fileInStream.gcount()] = '\0'; // Add a null terminator
      fileInStream.close();
    } else {
      throw Repast_Error_53(file); // Properties file not found
    }
  }
  if(comm != 0){                                                               // If a communicator was passed, proc 0 broadcasts to all other procs
    MPI_Bcast(PROPFILEBUFFER, maxPropFileSize, MPI_CHAR, 0, *comm);
  }

  std::string P(PROPFILEBUFFER);
  delete PROPFILEBUFFER;

  istringstream in(P);
  string line;
  while (!in.eof()) {
    getline(in, line);
    str_trim(line);
    if (line.length() > 0 && line[0] != '#') {
      size_t pos = line.find_first_of("=");
      if (pos == string::npos)
        throw Repast_Error_54(line, file); // Missing '=' in properties file
      string key = line.substr(0, pos);
      repast::str_trim(key);
      if (key.length() == 0)
        throw Repast_Error_55(line, file); // Missing key value in properties file
      string value = "";
      if (line.length() > pos) {
        // this makes sure we only try to get value if it exists
        value = line.substr(pos + 1, line.length());
      }
      repast::str_trim(value);
      if (value.length() == 0)
        throw Repast_Error_56(line, file); // Missing key value in properties file
      map[key] = value;
    }
  }
}

void Properties::processCommandLineArguments(int argc, char **argv){
  for(int i = 0; i < argc; i++){
    std::string pEntry(argv[i]);
    size_t indexOfEqualsSign = pEntry.find('=');
    if(indexOfEqualsSign != std::string::npos)
        putProperty(pEntry.substr(0, indexOfEqualsSign), pEntry.substr(indexOfEqualsSign + 1));
  }
}

void Properties::writeToSVFile(std::string fileName, std::string separator){
  std::vector<std::string> keysToWrite;
  std::map<std::string, std::string>::iterator iter    = map.begin();
  std::map<std::string, std::string>::iterator iterEnd = map.end();
  while(iter != iterEnd){
    keysToWrite.push_back(iter->first);
    iter++;
  }
  writeToSVFile(fileName, keysToWrite, separator);
}

void Properties::writeToSVFile(std::string fileName, std::vector<std::string> &keysToWrite, std::string separator){
  bool writeHeader =  !boost::filesystem::exists(fileName);
  std::ofstream outFile;

  outFile.open(fileName.c_str(), std::ios::app);

  if(writeHeader){
    std::vector<std::string>::iterator keys      = keysToWrite.begin();
    std::vector<std::string>::iterator keysEnd   = keysToWrite.end();
    int i = 1;
    while(keys != keysEnd){
      outFile << *keys << (i != keysToWrite.size() ? separator : "");
      keys++;
      i++;
    }
    outFile << std::endl;
  }
  std::vector<std::string>::iterator keys      = keysToWrite.begin();
  std::vector<std::string>::iterator keysEnd   = keysToWrite.end();
  int i = 1;
  while(keys != keysEnd){
    outFile << std::fixed << getProperty(*keys) << (i != keysToWrite.size() ? separator : "");
    keys++;
    i++;
  }
  outFile << std::endl;

  outFile.close();

}


}
