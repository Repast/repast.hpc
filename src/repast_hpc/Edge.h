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
 *  Edge.h
 *
 *  Created on: Jun 4, 2009
 *      Author: nick
 */

#ifndef EDGE_H_
#define EDGE_H_

#include "AgentId.h"
#include "Context.h"
#include "RepastProcess.h"

#include <boost/serialization/access.hpp>

namespace repast {

/**
 * Default graph / network edge implementation.
 *
 * @tparam V agent type that is the source and target of the edge
 */
template<typename V>
class RepastEdge {

public:
  enum MASTER_NODE{ DEFAULT, SOURCE, TARGET };

private:
  double _weight;
  V* _source, *_target;
  bool _useTargetAsMaster;
  bool _conflicted;

  bool defaultTarget(int sourceRank, int targetRank, MASTER_NODE useTargetAsMaster){
    int rank = repast::RepastProcess::instance()->rank();
    return (useTargetAsMaster == SOURCE ? false :
            useTargetAsMaster == TARGET ? true  :
            ((_source->getId().currentRank() != rank) &&
             (_target->getId().currentRank() == rank)));
  }

public:

  // no arg constructor for serialization
  RepastEdge() : _weight(1), _source(0), _target(0), _useTargetAsMaster(false), _conflicted(false){ }
  ~RepastEdge(){ }

  /**
   * Creates a RepastEdge with the specified source and target and a default
   * weight of 1.
   *
   * @param source the edge source
   * @param target the edge target
   */
  RepastEdge(V* source, V* target, MASTER_NODE useTargetAsMaster = DEFAULT);

  /**
   * Creates a RepastEdge with the specified source, target,
   * and weight
   *
   * @param source the edge source
   * @param target the edge target
   * @param weight the edge weight
   */
  RepastEdge(V* source, V* target, double weight, MASTER_NODE useTargetAsMaster = DEFAULT);

  /**
   * Creates a RepastEdge with the specified source and target and a default
   * weight of 1.
   *
   * @param source the edge source
   * @param target the edge target
   */
  RepastEdge(boost::shared_ptr<V> source, boost::shared_ptr<V> target, MASTER_NODE useTargetAsMaster = DEFAULT);

  /**
   * Creates a RepastEdge with the specified source, target,
   * and weight
   *
   * @param source the edge source
   * @param target the edge target
   * @param weight the edge weight
   */
  RepastEdge(boost::shared_ptr<V> source, boost::shared_ptr<V> target, double weight, MASTER_NODE useTargetAsMaster = DEFAULT);

  /**
   * Copy constructor that creates a RepastEdge from another RepastEdge.
   */
  RepastEdge(const RepastEdge& edge);

  /**
   * Gets the source of this RepastEdge.
   *
   * @return the source of this RepastEdge.
   */
  V* source() const {
    return _source;
  }

  /**
   * Gets the target of this RepastEdge.
   *
   * @return the target of this RepastEdge.
   */
  V* target() const {
    return _target;
  }

  // sets the target. NON USER API
  void target(V* target) {
    _target = target;
  }

  // sets the source. NON USER API
  void source(V* source) {
    _source = source;
  }

  /**
   * Gets the weight of this RepastEdge.
   *
   * @return the weight of this RepastEdge.
   */
  double weight() const {
    return _weight;
  }

  void weight(double wt){
    _weight = wt;
  }

  bool usesTargetAsMaster(){ return _useTargetAsMaster; }

  void markConflicted(){  _conflicted = true; }
  void clearConflicted(){ _conflicted = false; }
  bool isConflicted(){ return _conflicted; }

};

template<typename V>
RepastEdge<V>::RepastEdge(boost::shared_ptr<V> source, boost::shared_ptr<V> target, MASTER_NODE useTargetAsMaster) :
  _source(source.get()), _target(target.get()), _weight(1), _conflicted(false) {
  _useTargetAsMaster = defaultTarget(_source->getId().currentRank(), _target->getId().currentRank(), useTargetAsMaster);
}

template<typename V>
RepastEdge<V>::RepastEdge(V* source, V* target, MASTER_NODE useTargetAsMaster) :
  _source(source), _target(target), _weight(1), _conflicted(false){
  _useTargetAsMaster = defaultTarget(_source->getId().currentRank(), _target->getId().currentRank(), useTargetAsMaster);
}

template<typename V>
RepastEdge<V>::RepastEdge(V* source, V* target, double weight, MASTER_NODE useTargetAsMaster) :
  _source(source), _target(target), _weight(weight), _conflicted(false){
  _useTargetAsMaster = defaultTarget(_source->getId().currentRank(), _target->getId().currentRank(), useTargetAsMaster);
}

template<typename V>
RepastEdge<V>::RepastEdge(boost::shared_ptr<V> source, boost::shared_ptr<V> target, double weight, MASTER_NODE useTargetAsMaster) :
  _source(source.get()), _target(target.get()), _weight(weight), _conflicted(false){
  _useTargetAsMaster = defaultTarget(_source->getId().currentRank(), _target->getId().currentRank(), useTargetAsMaster);
}

template<typename V>
RepastEdge<V>::RepastEdge(const RepastEdge& edge) :
  _source(edge._source), _target(edge._target), _weight(edge._weight),
  _useTargetAsMaster(edge._useTargetAsMaster), _conflicted(edge._conflicted) { }

template<typename V>
std::ostream& operator<<(std::ostream& os, const RepastEdge<V>& edge) {
  os << (*edge.source()) << (edge._useTargetAsMaster ? "" : "(M)") << " -- " << (*edge.target() << (edge._useTargetAsMaster ? "(M)" : ""));
  return os;
}



/**
 * Serializable; also, does not include agent content,
 * only agent IDs
 *
 * @tparam V type for vertices; must provide AgentID
 */
template<typename V>
struct RepastEdgeContent {

  friend class boost::serialization::access;

  template<class Archive>
  void serialize(Archive& ar, const unsigned int version) {
    ar & usesTargetAsMaster;
    ar & weight;
    ar & source;
    ar & target;
  }

  AgentId source;
  AgentId target;
  double weight;
  bool usesTargetAsMaster;

  RepastEdgeContent(){} // For serialization

  RepastEdgeContent(RepastEdge<V>* edge):
      source(edge->source()->getId()),
      target(edge->target()->getId()),
      weight(edge->weight()),
      usesTargetAsMaster(edge->usesTargetAsMaster()){}
};


/**
 * Class for creating RepastEdges from RepastEdgeContent,
 * and vice versa
 *
 * @tparam V type for vertices; must provide AgentID
 */
template<typename V>
class RepastEdgeContentManager {

public:
  RepastEdgeContentManager(){}
  virtual ~RepastEdgeContentManager(){}

  RepastEdge<V>* createEdge(RepastEdgeContent<V>& content, Context<V>* context){
    return new RepastEdge<V>(context->getAgent(content.source), context->getAgent(content.target), content.weight,
        (content.usesTargetAsMaster ? repast::RepastEdge<V>::TARGET : repast::RepastEdge<V>::SOURCE) );
  }

  RepastEdgeContent<V>* provideEdgeContent(RepastEdge<V>* edge){
    return new RepastEdgeContent<V>(edge);
  }

};

}

#endif /* EDGE_H_ */
