/*
 * Repast for High Performance Computing (Repast HPC)
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
 * AgentImporterExporter.cpp
 *
 *  Created on: Jun 14, 2011
 *      Author: John T. Murphy, nick
 */

#include "AgentImporterExporter.h"

#include <algorithm>

#include "boost/serialization/set.hpp"

#include "boost/mpi.hpp" // debug only

using namespace repast;

/* AgentExporterData */

#ifndef SHARE_AGENTS_BY_SET
AgentExporterData::AgentExporterData(){}
#else
AgentExporterData::AgentExporterData(): selectedSet(DEFAULT_AGENT_REQUEST_SET){}
#endif


AgentExporterData::~AgentExporterData(){
  clear();
}

void AgentExporterData::addData(const AgentId& id, const int destProc, const int sourceProc, const int numberOfCopies){
  AgentRequest* req;
  std::map<int, AgentExporterInfo* >::iterator infoIter = data.find(destProc);

#ifndef SHARE_AGENTS_BY_SET
  if(infoIter == data.end()){
    req = new AgentRequest(sourceProc, destProc);
    std::map<int, AgentRequest*>* sourceMap = new std::map<int, AgentRequest*>;
    sourceMap->insert(std::pair<int, AgentRequest*>(sourceProc, req));
    data.insert(std::pair<int, std::map<int, AgentRequest*>*>(destProc, sourceMap));
  }
  else{
    std::map<int, AgentRequest*>* sourceMap = infoIter->second;
    std::map<int, AgentRequest*>::iterator reqIter = sourceMap->find(sourceProc);
    if(reqIter == sourceMap->end()){
      req = new AgentRequest(sourceProc, destProc);
      sourceMap->insert(std::pair<int, AgentRequest*>(sourceProc, req));
    }
    else{
      req = reqIter->second;
    }
  }
#else
  if(infoIter == data.end()){
    req = new AgentRequest(sourceProc, destProc);
    std::map<int, AgentRequest*>* sourceMap = new std::map<int, AgentRequest*>;
    sourceMap->insert(std::pair<int, AgentRequest*>(sourceProc, req));
    std::map<std::string, std::map<int, AgentRequest*>* >* stringMap = new std::map<std::string, std::map<int, AgentRequest*>* >;
    stringMap->insert(std::pair<std::string, std::map<int, AgentRequest*>* >(selectedSet, sourceMap));
    data.insert(std::pair<int, std::map<std::string, std::map<int, AgentRequest*>*>*>(destProc, stringMap));
  }
  else{
    std::map<std::string, std::map<int, AgentRequest*>*>* stringMap = infoIter->second;
    std::map<std::string, std::map<int, AgentRequest*>*>::iterator stringIter = stringMap->find(selectedSet);
    if(stringIter == stringMap->end()){
      req = new AgentRequest(sourceProc, destProc);
      std::map<int, AgentRequest*>* sourceMap = new std::map<int, AgentRequest*>;
      sourceMap->insert(std::pair<int, AgentRequest*>(sourceProc, req));
      stringMap->insert(std::pair<std::string, std::map<int, AgentRequest*>* >(selectedSet, sourceMap));
    }
    else{
      std::map<int, AgentRequest*>* sourceMap = stringIter->second;
      std::map<int, AgentRequest*>::iterator sourceIter = sourceMap->find(sourceProc);
      if(sourceIter == sourceMap->end()){
        req = new AgentRequest(sourceProc, destProc);
        sourceMap->insert(std::pair<int, AgentRequest*>(sourceProc, req));
      }
      else{
        req = sourceIter->second;
      }
    }
  }
#endif
  for(int i = 0; i < numberOfCopies; i++) req->addRequest(id);
}

AgentExporterInfo* AgentExporterData::dataForProc(int destProc){
  std::map<int, AgentExporterInfo* >::iterator iter = data.find(destProc);
  return (iter == data.end() ?  &empty : iter->second);
}


void AgentExporterData::clear(){
  std::map<int, AgentExporterInfo* >::iterator iter = data.begin();
  while(iter != data.end()){
#ifndef SHARE_AGENTS_BY_SET
    std::map<int, AgentRequest*>& sourceMap = *(iter->second);
    std::map<int, AgentRequest*>::iterator sourceIter = sourceMap.begin();
    while(sourceIter != sourceMap.end()){
      delete sourceIter->second;
      sourceIter++;
    }
#else
    std::map<std::string, std::map<int, AgentRequest*>*> stringMap = *(iter->second);
    std::map<std::string, std::map<int, AgentRequest*>*>::iterator stringIter = stringMap.begin();
    while(stringIter != stringMap.end()){
      std::map<int, AgentRequest*>& sourceMap = *(stringIter->second);
      std::map<int, AgentRequest*>::iterator sourceIter = sourceMap.begin();
      while(sourceIter != sourceMap.end()){
        delete sourceIter->second;
        sourceIter++;
      }
      delete stringIter->second;
      stringIter++;
    }
#endif
    delete iter->second;
    iter++;
  }
  data.clear();
}

void AgentExporterData::removeAllDataForAgent(AgentId& id){
  std::map<int, AgentExporterInfo* >::iterator iter = data.begin();
  while(iter != data.end()){
#ifndef SHARE_AGENTS_BY_SET
    std::map<int, AgentRequest*>& sourceMap = *(iter->second);
    std::map<int, AgentRequest*>::iterator sourceIter = sourceMap.begin();
    while(sourceIter != sourceMap.end()){
      sourceIter->second->removeRequest(id, true);
      sourceIter++;
    }
#else
    std::map<std::string, std::map<int, AgentRequest*>*> stringMap = *(iter->second);
    std::map<std::string, std::map<int, AgentRequest*>*>::iterator stringIter = stringMap.begin();
    while(stringIter != stringMap.end()){
      std::map<int, AgentRequest*>& sourceMap = *(stringIter->second);
      std::map<int, AgentRequest*>::iterator sourceIter = sourceMap.begin();
      while(sourceIter != sourceMap.end()){
        sourceIter->second->removeRequest(id, true);
        sourceIter++;
      }
      stringIter++;
    }
#endif
    iter++;
  }
}

#ifdef SHARE_AGENTS_BY_SET
void AgentExporterData::selectSet(std::string setName){
  selectedSet = setName;
}
#endif



/* AbstractImporter */
AbstractImporter::AbstractImporter(){}

AbstractImporter::~AbstractImporter(){}

/* Importer_COUNT */

#if !defined OMIT_IMPORTER_EXPORTER_COUNT_LIST || \
    !defined OMIT_IMPORTER_EXPORTER_COUNT_SET

Importer_COUNT::Importer_COUNT(){}

Importer_COUNT::~Importer_COUNT(){}

void Importer_COUNT::incrementCount(int sourceProcess){
  std::map<int, int>::iterator iter = sources.find(sourceProcess);
  if (iter == sources.end()){
    sources[sourceProcess] = 1;
    exportingProcesses.insert(sourceProcess);
  }
  else{
    sources[sourceProcess] = iter->second + 1;
  }
}

void Importer_COUNT::decrementCount(int sourceProcess){
  std::map<int, int>::iterator iter = sources.find(sourceProcess);
  if (iter == sources.end()) throw std::domain_error("Cannot decrement the agent count for a non-exporting rank");
  else {
    int newVal = iter->second - 1;
    if (newVal == 0){
      sources.erase(iter);
      exportingProcesses.erase(sourceProcess);
    }
    else{
      sources[sourceProcess] = newVal;
    }
  }
}

void Importer_COUNT::registerOutgoingRequests(AgentRequest &req){
  std::vector<AgentId>::const_iterator iter;
  for (iter = req.requestedAgents().begin(); iter != req.requestedAgents().end(); ++iter)  incrementCount(iter->currentRank());
  for (iter = req.cancellations().begin(); iter != req.cancellations().end(); ++iter)      decrementCount(iter->currentRank());
}

void Importer_COUNT::importedAgentIsRemoved(const AgentId& id){
  decrementCount(id.currentRank());
}

void Importer_COUNT::importedAgentIsMoved(const AgentId& id, int newProcess){
  decrementCount(id.currentRank());
  incrementCount(newProcess);
}

std::string Importer_COUNT::getReport(){
  std::stringstream ss;
  ss << "Importer_COUNT: Map Size = " << sources.size() << ":  \n";
  if(sources.size() > 0){
    ss << "   ";
    std::map<int, int>::iterator it          = sources.begin();
    const std::map<int, int>::iterator itEnd = sources.end();
    while(it != itEnd){
      ss << "[ Source Proc: " << it->first << " | " << it->second << "] ";
      it++;
    }
    ss << std::endl;
  }
  return ss.str();
}

void Importer_COUNT::getSetOfAgentsBeingImported(std::set<AgentId>& set){
  // Not implemented for 'count'
}

#endif


/* Importer_LIST */

#ifndef OMIT_IMPORTER_EXPORTER_LIST

Importer_LIST::Importer_LIST(){}

Importer_LIST::~Importer_LIST(){
  std::map<int, std::list<AgentId>* >::iterator itr = sources.begin();
  while(itr != sources.end()){
    delete itr->second;
    itr++;
  }
}

std::list<AgentId>* Importer_LIST::getRecord(int rank){
  std::map<int, std::list<AgentId>* >::iterator iter = sources.find(rank);
  if(iter != sources.end()) return iter->second;
  else{
    exportingProcesses.insert(rank);
    return (sources[rank] = new std::list<AgentId>); // Create, assign, return
  }
}

int Importer_LIST::removeAll(const AgentId& id, std::list<AgentId>* record, int rank){
  int initialSize = record->size();
  record->remove(id);
  int finalSize = record->size();
  checkRecord(record, rank);
  return initialSize - finalSize;
}

void Importer_LIST::removeID(AgentId& id, std::list<AgentId>* record, int rank){
  std::list<AgentId>::iterator idPosition = find(record->begin(), record->end(), id);
  if(idPosition != record->end()) removeID(idPosition, record, rank);
}

void Importer_LIST::removeID(std::list<AgentId>::iterator idPosition, std::list<AgentId>* record, int rank){
  record->erase(idPosition);
  checkRecord(record, rank);
}

void Importer_LIST::checkRecord(std::list<AgentId>* record, int rank){
  if(record->size() == 0){
    sources.erase(rank);
    exportingProcesses.erase(rank);
    delete record;
  }
}


void Importer_LIST::registerOutgoingRequests(AgentRequest &req){
  // First, loop through the requested Agent IDs
  std::vector<AgentId>::iterator reqAgentIdIterator = req.requestedAgents_.begin();
  while(reqAgentIdIterator != req.requestedAgents_.end()){
    const AgentId &id = *reqAgentIdIterator;

    std::list<AgentId>* record = getRecord(id.currentRank());

    // If the record already contains the ID, delete the ID from the request
    if(find(record->begin(), record->end(), id) != record->end())    reqAgentIdIterator = req.requestedAgents_.erase(reqAgentIdIterator);
    else                                                             reqAgentIdIterator++;

    // And add the ID to the record
    record->push_back(id);
  }

  // Then loop through the cancellations
  std::vector<AgentId>::iterator cancelledIdIterator = req.cancellations_.begin();

  while(cancelledIdIterator != req.cancellations_.end()){
    AgentId &id = *cancelledIdIterator;

#ifdef ALLOW_FULL_AGENT_REQUEST_CANCELLATION
    bool isFullCancel = checkForFullCancellation(id);
#endif
    int idRank = id.currentRank();

    // Retrieve the current list for this source rank
    std::map<int, std::list<AgentId>* >::iterator sourceListIter = sources.find(idRank);

    // If there is not even a list for this rank, just delete the cancellation
    if(sourceListIter == sources.end()) cancelledIdIterator = req.cancellations_.erase(cancelledIdIterator);
    else{
      std::list<AgentId>* record = sourceListIter->second;
      std::list<AgentId>::iterator firstInstance = find(record->begin(), record->end(), id);

      // If there is no entry for this ID, just delete the cancellation
      if(firstInstance == record->end()) cancelledIdIterator = req.cancellations_.erase(cancelledIdIterator);
      else{

#ifdef ALLOW_FULL_AGENT_REQUEST_CANCELLATION
        if(!isFullCancel){
#endif

        // See if there is a second entry in the record
        std::list<AgentId>::iterator secondFind = firstInstance;
        secondFind++;
        secondFind = find(secondFind, record->end(), id);

        // If so, need to delete one but should not pass on the cancellation
        if(secondFind != record->end()){
          record->erase(secondFind);    // We know there are at least two, so no possibility that size->0
          cancelledIdIterator = req.cancellations_.erase(cancelledIdIterator);
        }
        else{
          // If there is only one entry in the record, delete it AND pass along
          // the cancellation
          removeID(firstInstance, record, idRank);
          cancelledIdIterator++;
        }

#ifdef ALLOW_FULL_AGENT_REQUEST_CANCELLATION
        }
        else{
          removeAll(id, record, idRank);         // Remove all instances of the ID
          cancelledIdIterator++;
        }
#endif
      }
    }
  }
}

void Importer_LIST::importedAgentIsRemoved(const AgentId& id){
  std::map<int, std::list<AgentId>*>::iterator iter = sources.find(id.currentRank());
  if(iter != sources.end()) removeAll(id, iter->second, id.currentRank());
}

void Importer_LIST::importedAgentIsMoved(const AgentId& id, int newProcess){
  std::map<int, std::list<AgentId>*>::iterator iter = sources.find(id.currentRank());
  if(iter != sources.end()){
    int c = removeAll(id, iter->second, id.currentRank());
    AgentId newId(id);
    newId.currentRank(newProcess);
    std::list<AgentId>* record = getRecord(newProcess);
    for(int i = 0; i < c; i++) record->push_back(newId);
  }
}


std::string Importer_LIST::getReport(){
  std::stringstream ss;
  ss << "Importer_LIST: Map Size = " << sources.size() << ":  \n";
  if(sources.size() > 0){
    ss << "   ";
    std::map<int, std::list<AgentId>* >::iterator it          = sources.begin();
    const std::map<int, std::list<AgentId>* >::iterator itEnd = sources.end();
    while(it != itEnd){
      ss << "[ Source Proc: " << it->first << " | ";
      std::list<AgentId>::iterator listIt = it->second->begin();
      const std::list<AgentId>::iterator listItEnd = it->second->end();
      while(listIt != listItEnd){
        ss << *listIt << " ";
        listIt++;
      }
      ss << "]" << std::endl;
      it++;
    }
  }
  return ss.str();
}

void Importer_LIST::getSetOfAgentsBeingImported(std::set<AgentId>& set){
  if(sources.size() > 0){
    std::map<int, std::list<AgentId>* >::iterator it          = sources.begin();
    const std::map<int, std::list<AgentId>* >::iterator itEnd = sources.end();
    while(it != itEnd){
      set.insert(it->second->begin(), it->second->end());
      it++;
    }
  }
}

#endif


/* Importer_SET */

#ifndef OMIT_IMPORTER_EXPORTER_SET

Importer_SET::Importer_SET(){
}

Importer_SET::~Importer_SET(){
  std::map<int, std::set<AgentId>* >::iterator iter = sources.begin();
  while(iter != sources.end()){
    delete iter->second;
    iter++;
  }
}

std::set<AgentId>* Importer_SET::getRecord(int rank){
  std::map<int, std::set<AgentId>* >::iterator iter = sources.find(rank);
  if(iter != sources.end()) return iter->second;
  else{
    exportingProcesses.insert(rank);
    return (sources[rank] = new std::set<AgentId>); // Create, assign, return
  }
}

int Importer_SET::removeID(const AgentId& id, std::set<AgentId>* record, int rank){
  int initialSize = record->size();
  record->erase(id);
  int finalSize = record->size();
  checkRecord(record, rank);
  return initialSize - finalSize;
}

void Importer_SET::checkRecord(std::set<AgentId>* record, int rank){
  if(record->size() == 0){
    sources.erase(rank);
    exportingProcesses.erase(rank);
    delete record;
  }
}



void Importer_SET::registerOutgoingRequests(AgentRequest& req){

  // First, loop through the requested Agent IDs
  std::vector<AgentId>::iterator reqAgentIdIterator = req.requestedAgents_.begin();
  while(reqAgentIdIterator != req.requestedAgents_.end()){
    // Try to insert the id; if it is inserted, it wasn't in the set before, so pass along the request
    if(getRecord(reqAgentIdIterator->currentRank())->insert(*reqAgentIdIterator).second)
        reqAgentIdIterator++;
    else // Otherwise, already in the set, so delete from the request
        reqAgentIdIterator = req.requestedAgents_.erase(reqAgentIdIterator);
  }

  // Then loop through the cancellations
  std::vector<AgentId>::iterator cancelledIdIterator = req.cancellations_.begin();
  while(cancelledIdIterator != req.cancellations_.end()){
    std::map<int, std::set<AgentId>* >::iterator iter = sources.find(cancelledIdIterator->currentRank());
    if(iter == sources.end()) req.cancellations_.erase(cancelledIdIterator); // No other action required
    else{
      if(removeID(*cancelledIdIterator, iter->second, cancelledIdIterator->currentRank()) > 0 ) cancelledIdIterator++;
      else                                                                                     req.cancellations_.erase(cancelledIdIterator);
    }
  }
}

void Importer_SET::importedAgentIsRemoved(const AgentId& id){
  std::map<int, std::set<AgentId>*>::iterator iter = sources.find(id.currentRank());
  if(iter != sources.end()) removeID(id, iter->second, id.currentRank());
}

void Importer_SET::importedAgentIsMoved(const AgentId& id, int newProcess){
  std::map<int, std::set<AgentId>*>::iterator iter = sources.find(id.currentRank());
  if(iter != sources.end()){
    int c = removeID(id, iter->second, id.currentRank());
    if(c > 0){
      AgentId newId(id);
      newId.currentRank(newProcess);
      std::set<AgentId>* record = getRecord(newProcess);
      record->insert(newId);
    }
  }
}

std::string Importer_SET::getReport(){
  std::stringstream ss;
  ss << "Importer_SET: Map Size = " << sources.size() << ":  \n";
  if(sources.size() > 0){
    ss << "   ";
    std::map<int, std::set<AgentId>* >::iterator it          = sources.begin();
    const std::map<int, std::set<AgentId>* >::iterator itEnd = sources.end();
    while(it != itEnd){
      ss << "[ Source Proc: " << it->first << " | ";
      std::set<AgentId>::iterator setIt = it->second->begin();
      const std::set<AgentId>::iterator setItEnd = it->second->end();
      while(setIt != setItEnd){
        ss << *setIt << " ";
        setIt++;
      }
      ss << "]" << std::endl;
      it++;
    }
  }
  return ss.str();
}

void Importer_SET::getSetOfAgentsBeingImported(std::set<AgentId>& set){
  if(sources.size() > 0){
    std::map<int, std::set<AgentId>* >::iterator it          = sources.begin();
    const std::map<int, std::set<AgentId>* >::iterator itEnd = sources.end();
    while(it != itEnd){
      set.insert(it->second->begin(), it->second->end());
      it++;
    }
  }
}

#endif


/* Importer_MAP_int */

#ifndef OMIT_IMPORTER_EXPORTER_MAP_int

Importer_MAP_int::Importer_MAP_int(){}

Importer_MAP_int::~Importer_MAP_int(){
  std::map<int, std::map<AgentId, int>* >::iterator iter = sources.begin();
  while(iter != sources.end()){
    delete iter->second;
    iter++;
  }
}

std::map<AgentId, int>* Importer_MAP_int::getRecord(int rank){
  std::map<int, std::map<AgentId, int>* >::iterator iter = sources.find(rank);
  if(iter != sources.end()) return iter->second;
  else{
    exportingProcesses.insert(rank);
    return (sources[rank] = new std::map<AgentId, int>); // Create, assign, return
  }
}

int Importer_MAP_int::removeAll(const AgentId& id, std::map<AgentId, int>* record, int rank){
  int ret = 0;
  std::map<AgentId, int>::iterator idPos= record->find(id);
  if(idPos == record->end()) return ret;
  else{
    ret = idPos->second;
    record->erase(idPos);
  }
  checkRecord(record, rank);
  return ret;
}


int Importer_MAP_int::removeID(const AgentId& id, std::map<AgentId, int>* record, int rank){
  std::map<AgentId, int>::iterator idPos= record->find(id);
  if(idPos == record->end()) return 0;
  else{
    idPos->second = idPos->second - 1;
    if(idPos->second == 0) record->erase(idPos);
  }
  checkRecord(record, rank);
  return 1;
}

void Importer_MAP_int::removeID(std::map<AgentId, int>::iterator idPosition, std::map<AgentId, int>* record, int rank){
  record->erase(idPosition);
  checkRecord(record, rank);
}

void Importer_MAP_int::checkRecord(std::map<AgentId, int>* record, int rank){
  if(record->size() == 0){
    sources.erase(rank);
    exportingProcesses.erase(rank);
    delete record;
  }
}


void Importer_MAP_int::registerOutgoingRequests(AgentRequest &req){
  // First, loop through the requested Agent IDs
  std::vector<AgentId>::iterator reqAgentIdIterator = req.requestedAgents_.begin();
  while(reqAgentIdIterator != req.requestedAgents_.end()){
    const AgentId &id = *reqAgentIdIterator;

    // Retrieve the current list for this source rank
    std::map<AgentId, int>* record = getRecord(id.currentRank());

    std::map<AgentId, int>::iterator mapEntry = record->find(id);
    if(mapEntry != record->end()){
      mapEntry->second = mapEntry->second + 1;
      reqAgentIdIterator = req.requestedAgents_.erase(reqAgentIdIterator);
    }
    else{
      (*record)[id] = 1;
      reqAgentIdIterator++;
    }
  }

  // Then loop through the cancellations
  std::vector<AgentId>::iterator cancelledIdIterator = req.cancellations_.begin();
  while(cancelledIdIterator != req.cancellations_.end()){
    AgentId &id = *cancelledIdIterator;

#ifdef ALLOW_FULL_AGENT_REQUEST_CANCELLATION
    bool isFullCancel = checkForFullCancellation(id);
#endif

    int rank = id.currentRank();

    // Retrieve the current list for this source rank
    std::map<int, std::map<AgentId, int>* >::iterator sourceMapIter = sources.find(rank);

    if(sourceMapIter == sources.end()) req.cancellations_.erase(cancelledIdIterator);
    else{
      std::map<AgentId, int>* record = sourceMapIter->second;
      std::map<AgentId, int>::iterator instance = record->find(id);

      if(instance == record->end())
          cancelledIdIterator = req.cancellations_.erase(cancelledIdIterator);

      else{
#ifdef ALLOW_FULL_AGENT_REQUEST_CANCELLATION
        if(!isFullCancel){
#endif

        if(instance->second == 1){
          removeID(instance, record, rank);
          cancelledIdIterator++;
        }
        else{
          // n.b.: Removing an element here, too... but we know there are at least two
          instance->second = instance->second - 1;
          cancelledIdIterator = req.cancellations_.erase(cancelledIdIterator);
        }
#ifdef ALLOW_FULL_AGENT_REQUEST_CANCELLATION
        }
        else{
          removeAll(id, record, rank);
          cancelledIdIterator++;
        }
#endif
      }
    }
  }
}

void Importer_MAP_int::importedAgentIsRemoved(const AgentId& id){
  std::map<int, std::map<AgentId, int>* >::iterator sourceMapIter = sources.find(id.currentRank());
  if(sourceMapIter != sources.end()) removeAll(id, sourceMapIter->second, id.currentRank());
}

void Importer_MAP_int::importedAgentIsMoved(const AgentId& id, int newProcess){
  std::map<int, std::map<AgentId, int>* >::iterator sourceMapIter = sources.find(id.currentRank());
  if(sourceMapIter != sources.end()){
    int c = removeAll(id, sourceMapIter->second, id.currentRank());
    AgentId newId(id);
    newId.currentRank(newProcess);
    std::map<AgentId, int>* record = getRecord(newProcess);
    record->insert(std::pair<AgentId, int>(newId, c));
  }
}

std::string Importer_MAP_int::getReport(){
  std::stringstream ss;
  ss << "Importer_MAP_int: Map Size = " << sources.size() << ":  \n";
  if(sources.size() > 0){
    ss << "   ";
    std::map<int, std::map<AgentId, int>* >::iterator it          = sources.begin();
    const std::map<int, std::map<AgentId, int>* >::iterator itEnd = sources.end();
    while(it != itEnd){
      ss << "[ Source Proc: " << it->first << " | ";
      std::map<AgentId, int>::iterator mapIt = it->second->begin();
      const std::map<AgentId, int>::iterator mapItEnd = it->second->end();
      while(mapIt != mapItEnd){
        ss << mapIt->first << " = " << mapIt->second << ", ";
        mapIt++;
      }
      ss << " ]" << std::endl;
      it++;
    }
  }
  return ss.str();
}

void Importer_MAP_int::getSetOfAgentsBeingImported(std::set<AgentId>& set){
  if(sources.size() > 0){
    std::map<int, std::map<AgentId, int>* >::iterator it          = sources.begin();
    const std::map<int, std::map<AgentId, int>* >::iterator itEnd = sources.end();
    while(it != itEnd){
      std::map<AgentId, int>::iterator mapIt = it->second->begin();
      const std::map<AgentId, int>::iterator mapItEnd = it->second->end();
      while(mapIt != mapItEnd){
        set.insert(mapIt->first);
        mapIt++;
      }
      it++;
    }
  }
}

#endif









/* AbstractExporter */
AbstractExporter::AbstractExporter(){
  outgoingStatusChangesDeletePtr             = new StatusMap;
  outgoingStatusChanges                      = outgoingStatusChangesDeletePtr;
  outgoingAgentExporterInformationDeletePtr  = new AgentExporterData;
  outgoingAgentExporterInformation           = outgoingAgentExporterInformationDeletePtr;
}

#ifdef SHARE_AGENTS_BY_SET
AbstractExporter::AbstractExporter(StatusMap* outgoingStatusMap, AgentExporterData* outgoingAgentExporterInfo){
  outgoingStatusChangesDeletePtr             = 0;
  outgoingStatusChanges                      = outgoingStatusMap;
  outgoingAgentExporterInformationDeletePtr  = 0;
  outgoingAgentExporterInformation           = outgoingAgentExporterInfo;
}
#endif

AbstractExporter::~AbstractExporter(){
  delete outgoingStatusChangesDeletePtr;
  delete outgoingAgentExporterInformationDeletePtr;
}


const std::set<int>& AbstractExporter::getProcessesExportedTo(){
  return processesExportedTo;
}


void AbstractExporter::agentRemoved(const AgentId& id){
  AgentStatus status(id); // Indicates removal

  // Loop through the exported map
  std::map<int, AgentRequest>::iterator requestIter = exportedMap.begin();
  while(requestIter != exportedMap.end()){
    AgentRequest* req = &requestIter->second;
    int initialSize = req->requestCountRequested();
    req->remove(id, true);
    int numberRemoved = initialSize - req->requestCountRequested();
    if(numberRemoved > 0){
      (*outgoingStatusChanges)[requestIter->first].insert(status);
      if(req->requestCountRequested() == 0){
        processesExportedTo.erase(requestIter->first);
        std::map<int, AgentRequest>::iterator temp = requestIter;
        requestIter++;
        exportedMap.erase(temp);
      }
      else requestIter++;
    }
    else requestIter++;
  }
}

void AbstractExporter::agentMoved(const AgentId& id, int process){
  AgentId newId(id);
  newId.currentRank(process);
  AgentStatus status(id, newId); // Indicates move

  // Loop through the exported map
  std::map<int, AgentRequest>::iterator requestIter = exportedMap.begin();
  while(requestIter != exportedMap.end()){
    AgentRequest* req = &requestIter->second;
    int initialSize = req->requestCountRequested();
    req->remove(id, true);
    int numberRemoved = initialSize - req->requestCountRequested();
    if(numberRemoved > 0){
      outgoingAgentExporterInformation->addData(id, process, requestIter->first, numberRemoved);
      (*outgoingStatusChanges)[requestIter->first].insert(status);
      if(req->requestCountRequested() == 0){
        processesExportedTo.erase(requestIter->first);
        std::map<int, AgentRequest>::iterator temp = requestIter;
        requestIter++;
        exportedMap.erase(temp);
      }
      else requestIter++;
    }
    else requestIter++;
  }
}

void AbstractExporter::incorporateAgentExporterInfo(std::map<int, AgentRequest*> info){
  std::map<int, AgentRequest*>::iterator infoIter = info.begin();
  while(infoIter != info.end()){
    processesExportedTo.insert(infoIter->first);
    exportedMap[infoIter->first].addAll(*(infoIter->second));
    infoIter++;
  }
}


AgentExporterInfo* AbstractExporter::getAgentExportInfo(int destProc){
  return outgoingAgentExporterInformation->dataForProc(destProc);
}

const AbstractExporter::StatusMap* AbstractExporter::getOutgoingStatusChanges(){
  return outgoingStatusChanges;
}

void AbstractExporter::clearAgentExportInfo(){
  outgoingAgentExporterInformation->clear();
}

void AbstractExporter::clearStatusMap(){
  outgoingStatusChanges->clear();
}

const std::map<int, AgentRequest>& AbstractExporter::getAgentsToExport(){
  return exportedMap;
}



/* Exporter_LIST */

#if !defined OMIT_IMPORTER_EXPORTER_COUNT_LIST || \
    !defined OMIT_IMPORTER_EXPORTER_LIST       || \
    !defined OMIT_IMPORTER_EXPORTER_SET        || \
    !defined OMIT_IMPORTER_EXPORTER_MAP_int
Exporter_LIST::Exporter_LIST(){}

#ifdef SHARE_AGENTS_BY_SET
  Exporter_LIST::Exporter_LIST(StatusMap* outgoingStatusMap, AgentExporterData* outgoingAgentExporterInfo):
      AbstractExporter(outgoingStatusMap, outgoingAgentExporterInfo){}
#endif

Exporter_LIST::~Exporter_LIST(){}

void Exporter_LIST::registerIncomingRequests(std::vector<AgentRequest>& requests){
  std::vector<AgentRequest>::iterator reqIter = requests.begin();
  while(reqIter != requests.end()){
    AgentRequest& request = *reqIter;
    int requestingProc = request.sourceProcess();
    std::map<int, AgentRequest>::iterator iter = exportedMap.find(requestingProc);

    if (iter == exportedMap.end()) {
      AgentRequest request(requestingProc, -1);
      exportedMap[requestingProc] = request;
      processesExportedTo.insert(requestingProc);
      iter = exportedMap.find(requestingProc);
    }

    iter->second.addAllRequests(request); // Adds all

    // Now deal with cancellations
    std::vector<AgentId> cancellations = request.cancellations();
    for(std::vector<AgentId>::iterator agentIdIter = cancellations.begin(); agentIdIter != cancellations.end(); agentIdIter++){
      iter->second.removeRequest(*agentIdIter, false); // Remove only the first instance, if any
    }

    // The mapped request may now be empty
    if(iter->second.requestCountRequested() == 0){
      exportedMap.erase(requestingProc);
      processesExportedTo.erase(requestingProc);
    }

    reqIter++;
  }
}

std::string Exporter_LIST::getReport(){
  std::stringstream ss;
  ss << "Exporter_LIST: Sending to " << exportedMap.size() << " (" << processesExportedTo.size() << ") procs:" << std::endl;
  std::map<int, AgentRequest>::iterator it = exportedMap.begin();
  const std::map<int, AgentRequest>::iterator itEnd = exportedMap.end();
  while(it != itEnd){
    ss << "   " << it->first << " -> " << it->second << std::endl;
    it++;
  }
  return ss.str();
}

#endif


/* Exporter_SET */

#ifndef OMIT_IMPORTER_EXPORTER_COUNT_SET

Exporter_SET::Exporter_SET(): AbstractExporter(){}

#ifdef SHARE_AGENTS_BY_SET
  Exporter_SET::Exporter_SET(StatusMap* outgoingStatusMap, AgentExporterData* outgoingAgentExporterInfo):
      AbstractExporter(outgoingStatusMap, outgoingAgentExporterInfo){}
#endif

Exporter_SET::~Exporter_SET(){ }

void Exporter_SET::registerIncomingRequests(std::vector<AgentRequest>& requests){
  std::vector<AgentRequest>::iterator reqIter = requests.begin();
  while(reqIter != requests.end()){
    AgentRequest& request = *reqIter;
    int requestingProc = request.sourceProcess();
    std::map<int, AgentRequest>::iterator iter = exportedMap.find(requestingProc);

    if (iter == exportedMap.end()) {
      AgentRequest request(requestingProc, -1);
      exportedMap[requestingProc] = request;
      processesExportedTo.insert(requestingProc);
      iter = exportedMap.find(requestingProc);
    }

    std::vector<AgentId> requestedIDs = request.requestedAgents();
    std::vector<AgentId>::iterator requestedID = requestedIDs.begin();
    while(requestedID != requestedIDs.end()){
      if(!iter->second.containsInRequests(*requestedID)) iter->second.addRequest(*requestedID);
      requestedID++;
    }

    // Now deal with cancellations
    std::vector<AgentId> cancellations = request.cancellations();
    for(std::vector<AgentId>::iterator agentIdIter = cancellations.begin(); agentIdIter != cancellations.end(); agentIdIter++){
      iter->second.removeRequest(*agentIdIter, true); // Remove ALL instances, though there should never be more than one
    }

    // The mapped request may now be empty
    if(iter->second.requestCountRequested() == 0){
      exportedMap.erase(requestingProc);
      processesExportedTo.erase(requestingProc);
    }

    reqIter++;
  }
}

std::string Exporter_SET::getReport(){
  std::stringstream ss;
  ss << "Exporter_SET: Sending to " << exportedMap.size() << " procs:" << std::endl;
  std::map<int, AgentRequest>::iterator it = exportedMap.begin();
  const std::map<int, AgentRequest>::iterator itEnd = exportedMap.end();
  while(it != itEnd){
    ss << "   " << it->first << " -> " << it->second << std::endl;
    it++;
  }
  return ss.str();
}

#endif


/* AbstractImporterExporter */

AbstractImporterExporter::AbstractImporterExporter(AbstractImporter* i, AbstractExporter* e):importer(i), exporter(e){}

AbstractImporterExporter::~AbstractImporterExporter(){
  delete importer;
  delete exporter;
}

const AbstractExporter::StatusMap* AbstractImporterExporter::getOutgoingStatusChanges(){
  return exporter->getOutgoingStatusChanges();
}

void AbstractImporterExporter::exchangeAgentStatusUpdates(boost::mpi::communicator comm, std::vector<std::vector<AgentStatus>* >& statusUpdates){
  std::vector<boost::mpi::request> requests;

  // Create the appropriate receives...
  const std::set<int>& toReceiveFrom = getExportingProcesses();

  for (std::set<int>::const_iterator iter = toReceiveFrom.begin(); iter != toReceiveFrom.end(); ++iter) {
    int exportingProcess = *iter;
    std::vector<AgentStatus>* vec = new std::vector<AgentStatus> ();
    statusUpdates.push_back(vec);
    requests.push_back(comm.irecv(exportingProcess, AGENT_SYNC_STATUS, *vec));
  }

  // ... and sends
  const std::set<int>& exportedSet                         = getProcessesExportedTo();
  const AbstractExporter::StatusMap* outgoingStatusChanges = getOutgoingStatusChanges();

  // Using two iterators, both sorted on proc ID (!)
  std::set<int>::iterator                           exportedSetIter  = exportedSet.begin();
  AbstractExporter::StatusMap::const_iterator       statusMapIter    = outgoingStatusChanges->begin();

  while((exportedSetIter != exportedSet.end()) || (statusMapIter != outgoingStatusChanges->end())){
    int exportedSetVal = (exportedSetIter != exportedSet.end()            ? *exportedSetIter       : INT_MAX);
    int statusMapVal   = (statusMapIter   != outgoingStatusChanges->end() ? statusMapIter->first   : INT_MAX);
    if(statusMapVal == exportedSetVal){
      requests.push_back(comm.isend(statusMapVal, AGENT_SYNC_STATUS, statusMapIter->second));
      exportedSetIter++;
      statusMapIter++;
    }
    else if(statusMapVal < exportedSetVal){
      requests.push_back(comm.isend(statusMapVal, AGENT_SYNC_STATUS, statusMapIter->second));
      statusMapIter++;
    }
    else if(statusMapVal > exportedSetVal){
      requests.push_back(comm.isend(exportedSetVal, AGENT_SYNC_STATUS, emptyStatus));
      exportedSetIter++;
    }
  }

  // Exchange data via mpi
  boost::mpi::wait_all(requests.begin(), requests.end());

  // Clear the data from the status map once the sends are complete
  clearStatusMap();

}



#ifndef OMIT_IMPORTER_EXPORTER_COUNT_LIST
/* Importer_Exporter_COUNT_LIST */
ImporterExporter_COUNT_LIST::ImporterExporter_COUNT_LIST():AbstractImporterExporter(new Importer_COUNT(), new Exporter_LIST()){}

#ifdef SHARE_AGENTS_BY_SET
ImporterExporter_COUNT_LIST::ImporterExporter_COUNT_LIST(AbstractExporter::StatusMap* outgoingStatusMap, AgentExporterData* outgoingAgentExporterInfo):
    AbstractImporterExporter(new Importer_COUNT(), new Exporter_LIST(outgoingStatusMap, outgoingAgentExporterInfo)){}
#endif

ImporterExporter_COUNT_LIST::~ImporterExporter_COUNT_LIST(){}

std::string ImporterExporter_COUNT_LIST::version(){ return "COUNT_LIST"; }
#endif

#ifndef OMIT_IMPORTER_EXPORTER_COUNT_SET
/* Importer_Exporter_COUNT_SET */
ImporterExporter_COUNT_SET::ImporterExporter_COUNT_SET():AbstractImporterExporter(new Importer_COUNT(), new Exporter_SET()){}

#ifdef SHARE_AGENTS_BY_SET
ImporterExporter_COUNT_SET::ImporterExporter_COUNT_SET(AbstractExporter::StatusMap* outgoingStatusMap, AgentExporterData* outgoingAgentExporterInfo):
    AbstractImporterExporter(new Importer_COUNT(), new Exporter_SET(outgoingStatusMap, outgoingAgentExporterInfo)){}
#endif

ImporterExporter_COUNT_SET::~ImporterExporter_COUNT_SET(){}

std::string ImporterExporter_COUNT_SET::version(){ return "COUNT_SET"; }
#endif

#ifndef OMIT_IMPORTER_EXPORTER_LIST
/* ImporterExporter_LIST */
ImporterExporter_LIST::ImporterExporter_LIST():AbstractImporterExporter(new Importer_LIST(), new Exporter_LIST()){}

#ifdef SHARE_AGENTS_BY_SET
ImporterExporter_LIST::ImporterExporter_LIST(AbstractExporter::StatusMap* outgoingStatusMap, AgentExporterData* outgoingAgentExporterInfo):
    AbstractImporterExporter(new Importer_LIST(), new Exporter_LIST(outgoingStatusMap, outgoingAgentExporterInfo)){}
#endif

ImporterExporter_LIST::~ImporterExporter_LIST(){}

std::string ImporterExporter_LIST::version(){ return "LIST"; }
#endif

#ifndef OMIT_IMPORTER_EXPORTER_SET
/* ImporterExporter_SET */
ImporterExporter_SET::ImporterExporter_SET():AbstractImporterExporter(new Importer_SET(), new Exporter_LIST()){}

#ifdef SHARE_AGENTS_BY_SET
ImporterExporter_SET::ImporterExporter_SET(AbstractExporter::StatusMap* outgoingStatusMap, AgentExporterData* outgoingAgentExporterInfo):
    AbstractImporterExporter(new Importer_SET(), new Exporter_LIST(outgoingStatusMap, outgoingAgentExporterInfo)){}
#endif

ImporterExporter_SET::~ImporterExporter_SET(){}

std::string ImporterExporter_SET::version(){ return "SET"; }
#endif

#ifndef OMIT_IMPORTER_EXPORTER_MAP_int
/* ImporterExporter_MAP_int */
ImporterExporter_MAP_int::ImporterExporter_MAP_int():AbstractImporterExporter(new Importer_MAP_int(), new Exporter_LIST()){}

#ifdef SHARE_AGENTS_BY_SET
ImporterExporter_MAP_int::ImporterExporter_MAP_int(AbstractExporter::StatusMap* outgoingStatusMap, AgentExporterData* outgoingAgentExporterInfo):
    AbstractImporterExporter(new Importer_MAP_int(), new Exporter_LIST(outgoingStatusMap, outgoingAgentExporterInfo)){}
#endif

ImporterExporter_MAP_int::~ImporterExporter_MAP_int(){}

std::string ImporterExporter_MAP_int::version(){ return "MAP(int)"; }
#endif







/* ImporterExporter_BY_SET */

#ifdef SHARE_AGENTS_BY_SET

ImporterExporter_BY_SET::ImporterExporter_BY_SET():AbstractImporterExporter(NULL,NULL),
    exportingProcessesIsDirty(true), processesExportedToIsDirty(true), exportedMapIsDirty(true){
  outgoingStatusChanges             = new AbstractExporter::StatusMap;
  outgoingAgentExporterInformation  = new AgentExporterData;
}

ImporterExporter_BY_SET::~ImporterExporter_BY_SET(){
  std::map<std::string, AbstractImporterExporter*>::iterator iter = importersExportersMap.begin();
  while(iter != importersExportersMap.end()){
    delete iter->second;
    iter++;
  }
  outgoingStatusChanges->clear();
  delete outgoingStatusChanges;
  outgoingAgentExporterInformation->clear();
  delete outgoingAgentExporterInformation;
}

const AbstractExporter::StatusMap* ImporterExporter_BY_SET::getOutgoingStatusChanges(){
  return outgoingStatusChanges;
}


AbstractImporterExporter* ImporterExporter_BY_SET::getSet(std::string setName,
    AGENT_IMPORTER_EXPORTER_TYPE setType){
  std::map<std::string, AbstractImporterExporter*>::iterator match = importersExportersMap.find(setName);
  if(match != importersExportersMap.end()) return match->second;

  // Otherwise, create a new one, put the ref to it in the map, and return
  switch(setType){
#ifndef OMIT_IMPORTER_EXPORTER_COUNT_LIST
    case COUNT_LIST:
        return ( importersExportersMap[setName] = new ImporterExporter_COUNT_LIST(outgoingStatusChanges, outgoingAgentExporterInformation) );
#endif
#ifndef OMIT_IMPORTER_EXPORTER_COUNT_SET
    case COUNT_SET:
        return ( importersExportersMap[setName] = new ImporterExporter_COUNT_SET(outgoingStatusChanges, outgoingAgentExporterInformation) );
#endif
#ifndef OMIT_IMPORTER_EXPORTER_LIST
    case LIST:
        return ( importersExportersMap[setName] = new ImporterExporter_LIST(outgoingStatusChanges, outgoingAgentExporterInformation) );
#endif
#ifndef OMIT_IMPORTER_EXPORTER_SET
    case SET:
        return ( importersExportersMap[setName] = new ImporterExporter_SET(outgoingStatusChanges, outgoingAgentExporterInformation) );
#endif
#ifndef OMIT_IMPORTER_EXPORTER_MAP_int
    case MAP_int:
        return ( importersExportersMap[setName] = new ImporterExporter_MAP_int(outgoingStatusChanges, outgoingAgentExporterInformation) );
#endif
  }
  return NULL;
}


void ImporterExporter_BY_SET::rebuildExportingProcesses(bool forceRebuild){
  if(exportingProcessesIsDirty || forceRebuild){
    exportingProcesses.clear();
    std::map<std::string, AbstractImporterExporter*>::iterator ieIter = importersExportersMap.begin();
    while(ieIter != importersExportersMap.end()){
      AbstractImporterExporter* ie = ieIter->second;
      const std::set<int>& toAdd = ie->getExportingProcesses();
      exportingProcesses.insert(toAdd.begin(), toAdd.end());
      ieIter++;
    }
    exportingProcessesIsDirty = false;
  }
}

void ImporterExporter_BY_SET::rebuildProcessesExportedTo(bool forceRebuild){
  if(processesExportedToIsDirty || forceRebuild){
    processesExportedTo.clear();
    std::map<std::string, AbstractImporterExporter*>::iterator ieIter = importersExportersMap.begin();
    while(ieIter != importersExportersMap.end()){
      AbstractImporterExporter* ie = ieIter->second;
      const std::set<int>& toAdd = ie->getProcessesExportedTo();
      processesExportedTo.insert(toAdd.begin(), toAdd.end());
      ieIter++;
    }
    processesExportedToIsDirty = false;
  }
}

void ImporterExporter_BY_SET::rebuildExportedMap(bool forceRebuild){
  if(exportedMapIsDirty || forceRebuild){
    exportedMap.clear();
    std::map<int, std::set<AgentId> > idsByProc;
    std::map<std::string, AbstractImporterExporter*>::iterator ie = importersExportersMap.begin();
    // Collect the agents to be exported in a set first; this will eliminate duplicates
    while(ie != importersExportersMap.end()){
      std::map<int, AgentRequest> reqs = ie->second->getAgentsToExport();
      std::map<int, AgentRequest>::iterator procIter = reqs.begin();
      while(procIter != reqs.end()){
        std::set<AgentId>& set = idsByProc[procIter->first];
        AgentRequest& r = procIter->second;
        set.insert(r.requestedAgents().begin(), r.requestedAgents().end());
        procIter++;
      }
      ie++;
    }
    std::map<int, std::set<AgentId> >::iterator setIter = idsByProc.begin();
    while(setIter != idsByProc.end()){
      AgentRequest& req = exportedMap[setIter->first];
      std::set<AgentId>::iterator idListIter = setIter->second.begin();
      while(idListIter != setIter->second.end()){
        req.addRequest(*idListIter);
        idListIter++;
      }
      setIter++;
    }
    exportedMapIsDirty = false;
  }
}


const std::set<int>& ImporterExporter_BY_SET::getExportingProcesses(){
  return getExportingProcesses(REQUEST_AGENTS_ALL);
}

const std::set<int>& ImporterExporter_BY_SET::getExportingProcesses(std::string setName){
  if(setName != REQUEST_AGENTS_ALL) return getSet(setName)->getExportingProcesses();
  else{
    rebuildExportingProcesses();
    return exportingProcesses;
  }
}


void ImporterExporter_BY_SET::registerOutgoingRequests(AgentRequest& request){
  registerOutgoingRequests(request, DEFAULT_AGENT_REQUEST_SET, DEFAULT_ENUM_SYMBOL);
}

void ImporterExporter_BY_SET::registerOutgoingRequests(AgentRequest& request,
    std::string setName, AGENT_IMPORTER_EXPORTER_TYPE setType){
  getSet(setName, setType)->registerOutgoingRequests(request);
  processesExportedToIsDirty = true; // May not need all of these
  exportingProcessesIsDirty  = true;
  exportedMapIsDirty         = true;
}


void ImporterExporter_BY_SET::importedAgentIsRemoved(const AgentId& id){
  std::map<std::string, AbstractImporterExporter*>::iterator mapIter = importersExportersMap.begin();
  while(mapIter != importersExportersMap.end()){
    mapIter->second->importedAgentIsRemoved(id);
    mapIter++;
  }
  processesExportedToIsDirty = true; // May not need all of these
  exportingProcessesIsDirty  = true;
  exportedMapIsDirty         = true;
}

void ImporterExporter_BY_SET::importedAgentIsMoved(const AgentId& id, int newProcess){
  std::map<std::string, AbstractImporterExporter*>::iterator mapIter = importersExportersMap.begin();
  while(mapIter != importersExportersMap.end()){
    mapIter->second->importedAgentIsMoved(id, newProcess);
    mapIter++;
  }
  processesExportedToIsDirty = true; // May not need all of these
  exportingProcessesIsDirty  = true;
  exportedMapIsDirty         = true;
}

void ImporterExporter_BY_SET::importedAgentIsNowLocal(const AgentId& id){
  importedAgentIsRemoved(id);
}



const std::set<int>& ImporterExporter_BY_SET::getProcessesExportedTo(){
  return getProcessesExportedTo(REQUEST_AGENTS_ALL);
}

const std::set<int>& ImporterExporter_BY_SET::getProcessesExportedTo(std::string setName){
  if(setName != REQUEST_AGENTS_ALL) return getSet(setName)->getProcessesExportedTo();
  else{
    rebuildProcessesExportedTo();
    return processesExportedTo;
  }
}


void ImporterExporter_BY_SET::registerIncomingRequests(std::vector<AgentRequest>& requests){
  registerIncomingRequests(requests, DEFAULT_AGENT_REQUEST_SET);
}

void ImporterExporter_BY_SET::registerIncomingRequests(std::vector<AgentRequest>& requests, std::string setName){
  getSet(setName)->registerIncomingRequests(requests);
  processesExportedToIsDirty = true; // May not need all of these
  exportingProcessesIsDirty  = true;
  exportedMapIsDirty         = true;
}

void ImporterExporter_BY_SET::agentRemoved(const AgentId& id){
  std::map<std::string, AbstractImporterExporter*>::iterator mapIter = importersExportersMap.begin();
  while(mapIter != importersExportersMap.end()){
    outgoingAgentExporterInformation->selectSet(mapIter->first);
    mapIter->second->agentRemoved(id);
    mapIter++;
  }
  processesExportedToIsDirty = true; // May not need all of these
  exportingProcessesIsDirty  = true;
  exportedMapIsDirty         = true;
}

void ImporterExporter_BY_SET::agentMoved(const AgentId& id, int process){
  std::map<std::string, AbstractImporterExporter*>::iterator mapIter = importersExportersMap.begin();
  while(mapIter != importersExportersMap.end()){
    outgoingAgentExporterInformation->selectSet(mapIter->first);
    mapIter->second->agentMoved(id, process);
    mapIter++;
  }
  processesExportedToIsDirty = true; // May not need all of these
  exportingProcessesIsDirty  = true;
  exportedMapIsDirty         = true;
}

void ImporterExporter_BY_SET::incorporateAgentExporterInfo(std::map<int, AgentRequest* > info){
  getSet(DEFAULT_AGENT_REQUEST_SET)->incorporateAgentExporterInfo(info);
}

void ImporterExporter_BY_SET::incorporateAgentExporterInfo(std::map<std::string, std::map<int, AgentRequest*>* >  info){
  std::map<std::string, std::map<int, AgentRequest*>* >::iterator setIter = info.begin();
  while(setIter != info.end()){
    getSet(setIter->first)->incorporateAgentExporterInfo(*(setIter->second)); // In theory this could create a new set, but really only extant sets should be passed
    setIter++;
  }
  processesExportedToIsDirty = true; // May not need all of these
  exportingProcessesIsDirty  = true;
  exportedMapIsDirty         = true;
}

void ImporterExporter_BY_SET::clearStatusMap(){
  outgoingStatusChanges->clear();
}

AgentExporterInfo* ImporterExporter_BY_SET::getAgentExportInfo(int destProc){
  return outgoingAgentExporterInformation->dataForProc(destProc);
}

void ImporterExporter_BY_SET::clearAgentExportInfo(){
  outgoingAgentExporterInformation->clear();
}

const std::map<int, AgentRequest>& ImporterExporter_BY_SET::getAgentsToExport(){
  return getAgentsToExport(REQUEST_AGENTS_ALL);
}

const std::map<int, AgentRequest>& ImporterExporter_BY_SET::getAgentsToExport(std::string setName){
  if(setName != REQUEST_AGENTS_ALL) return getSet(setName)->getAgentsToExport();
  else{
    rebuildExportedMap();
    return exportedMap;
  }
}


void ImporterExporter_BY_SET::getSetOfAgentsBeingImported(std::set<AgentId>& set){
  std::map<std::string, AbstractImporterExporter*>::iterator mapIter = importersExportersMap.begin();
  while(mapIter != importersExportersMap.end()){
    mapIter->second->getSetOfAgentsBeingImported(set);
    mapIter++;
  }
}

void ImporterExporter_BY_SET::getSetOfAgentsBeingImported(std::set<AgentId>& set, std::string excludeSet){
  std::map<std::string, AbstractImporterExporter*>::iterator mapIter = importersExportersMap.begin();
  while(mapIter != importersExportersMap.end()){
    if(mapIter->first.compare(excludeSet) != 0)    mapIter->second->getSetOfAgentsBeingImported(set);
    mapIter++;
  }
}

std::string ImporterExporter_BY_SET::version(){ return "BY SET: "
#if DEFAULT_IMPORTER_EXPORTER == 1
    "COUNT_LIST"
#elif DEFAULT_IMPORTER_EXPORTER == 2
    "COUNT_SET"
#elif DEFAULT_IMPORTER_EXPORTER == 3
    "LIST"
#elif DEFAULT_IMPORTER_EXPORTER == 4
    "SET"
#elif DEFAULT_IMPORTER_EXPORTER == 5
    "MAP(int)"
#endif
    ; }

#endif
