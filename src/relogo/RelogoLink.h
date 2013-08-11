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
 *  RelogoLink.h
 *
 *  Created on: Aug 5, 2010
 *      Author: nick
 */

#ifndef RELOGOEDGE_H_
#define RELOGOEDGE_H_



#include "repast_hpc/Edge.h"
#include "repast_hpc/AgentId.h"

#include "RelogoAgent.h"


namespace repast {
namespace relogo {

/**
 * Network link for Relogo.
 */
class RelogoLink: public repast::RepastEdge<RelogoAgent> {

public:
	/**
	 * Creates an empty RelogoLink with no source or target.
	 */
	RelogoLink() :
		RepastEdge<RelogoAgent> () {
	}

	/**
	 * Creates a RelogoLink with the specified source and target and a default
	 * weight of 1.
	 *
	 * @param source the link source
	 * @param target the link target
	 */
	RelogoLink(RelogoAgent* source, RelogoAgent* target);

	/**
	 * Creates a RelogoLink with the specified source, target,
	 * and weight
	 *
	 * @param source the link source
	 * @param target the link target
	 * @param weight the link weight
	 */
	RelogoLink(RelogoAgent* source, RelogoAgent* target, double weight);

	/**
	 * Creates a RelogoLink with the specified source and target and a default
	 * weight of 1.
	 *
	 * @param source the link source
	 * @param target the link target
	 */
	RelogoLink(boost::shared_ptr<RelogoAgent> source, boost::shared_ptr<RelogoAgent> target);

	/**
	 * Creates a RelogoLink with the specified source, target,
	 * and weight
	 *
	 * @param source the link source
	 * @param target the link target
	 * @param weight the link weight
	 */
	RelogoLink(boost::shared_ptr<RelogoAgent> source, boost::shared_ptr<RelogoAgent> target, double weight);

	/**
	 * Copy constructor that creates a RelogoLink from another RelogoLink.
	 */
	RelogoLink(const RelogoLink& edge);

	virtual ~RelogoLink();
};


/**
 * Subclass of RepastEdgeContent, used in synchronization.
 */
class RelogoLinkContent: public RepastEdgeContent<RelogoAgent>{
public:
  RelogoLinkContent(){}
  RelogoLinkContent(RelogoLink* link): RepastEdgeContent<RelogoAgent>(link){}

};


/**
 * Subclass of RepastEdgeContentManager, used to package and rebuild
 * edges during synchronization.
 */
class RelogoLinkContentManager {

public:
  RelogoLinkContentManager(){}
  virtual ~RelogoLinkContentManager(){}

  RelogoLink* createEdge(RelogoLinkContent& content, repast::Context<RelogoAgent>* context){
    return new RelogoLink(context->getAgent(content.source), context->getAgent(content.target), content.weight);
  }

  RelogoLinkContent* provideEdgeContent(RelogoLink* edge){
    return new RelogoLinkContent(edge);
  }

};

}

}

#endif /* RELOGOEDGE_H_ */
