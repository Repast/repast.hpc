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
 * model.h
 *
 *  Created on: Jul 13, 2010
 *      Author: nick
 */

#ifndef MODEL_H_
#define MODEL_H_

#include "repast_hpc/AgentId.h"
#include "repast_hpc/SharedContext.h"
#include "repast_hpc/RepastProcess.h"
#include "repast_hpc/SharedNetwork.h"
#include "repast_hpc/SharedSpace.h"
#include <vector>

#include <boost/serialization/access.hpp>

struct ModelAgentContent {

	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version) {
		ar & id;
		ar & state;
	}

	repast::AgentId id;
	int state;

	repast::AgentId getId() const {
		return id;
	}
};

//BOOST_IS_MPI_DATATYPE(ModelAgentContent)

struct EdgeContent {

	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version) {
		ar & sourceContent;
		ar & targetContent;
		ar & state;

	}

	// source and target agents
	ModelAgentContent sourceContent, targetContent;
	int state;
};

//BOOST_IS_MPI_DATATYPE(EdgeContent)

class ModelAgent: public repast::Agent {

private:
	repast::AgentId _id;
	int _state;

public:

	ModelAgent(repast::AgentId id, int state);
	virtual ~ModelAgent();

	int state() const {
		return _state;
	}

	void state(int val) {
		_state = val;
	}

	virtual repast::AgentId& getId() {
		return _id;
	}

	virtual const repast::AgentId& getId() const {
		return _id;
	}
};

class ModelEdge: public repast::RepastEdge<ModelAgent> {

private:
	int _state;

public:
	ModelEdge(ModelAgent* source, ModelAgent* target);
	ModelEdge(boost::shared_ptr<ModelAgent>, boost::shared_ptr<ModelAgent>);
	ModelEdge(boost::shared_ptr<ModelAgent>, boost::shared_ptr<ModelAgent>, double weight);
	virtual ~ModelEdge() {
	}

	int state() const {
		return _state;
	}

	void state(int newState) {
		_state = newState;
	}
};

class Model {

private:

	int rank;

public:
	repast::SharedContext<ModelAgent> agents;
	repast::SharedNetwork<ModelAgent, ModelEdge>* net;
	repast::SharedGrids<ModelAgent>::SharedWrappedGrid* grid;
	repast::SharedGrids<ModelAgent>::SharedWrappedSpace* space;

	Model();
	virtual ~Model();
	void provideContent(const repast::AgentRequest& rec, std::vector<ModelAgentContent>& contents);
	void provideEdgeContent(const ModelEdge* edge, std::vector<EdgeContent>& edgeContent);
	// updates synched edges
	void receiveEdgeContent(const EdgeContent& edge);
	ModelEdge* createEdge(repast::Context<ModelAgent>& context, EdgeContent& edge);

	void createAgents(const std::vector<ModelAgentContent>&, std::vector<ModelAgent*>& agents) ;
	void provideContent(ModelAgent* agent, std::vector<ModelAgentContent>& out) ;
};

class AgentCreatorUpdater {
private:
	Model* _model;

public:
	AgentCreatorUpdater(Model* model) :
		_model(model) {
	}
	virtual ~AgentCreatorUpdater() {
	}
	ModelAgent* createAgent(const ModelAgentContent& content);
	void updateAgent(const ModelAgentContent& content);
};

#endif /* MODEL_H_ */
