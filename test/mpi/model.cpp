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
 * model.cpp
 *
 *  Created on: Jul 13, 2010
 *      Author: nick
 */

#include <vector>

#include "model.h"
#include "repast_hpc/RepastProcess.h"

using namespace repast;

ModelAgent::ModelAgent(AgentId id, int state) :
	_id(id), _state(state) {
}

ModelAgent::~ModelAgent() {
}

ModelEdge::ModelEdge(ModelAgent* source, ModelAgent* target) :
	RepastEdge<ModelAgent> (source, target), _state(1) {

}

ModelEdge::ModelEdge(boost::shared_ptr<ModelAgent> source, boost::shared_ptr<ModelAgent> target) :
	RepastEdge<ModelAgent> (source, target), _state(1) {

}

ModelEdge::ModelEdge(boost::shared_ptr<ModelAgent> source, boost::shared_ptr<ModelAgent> target, double weight) :
	RepastEdge<ModelAgent> (source, target, weight), _state(1) {

}

Model::Model() {
	rank = RepastProcess::instance()->rank();
	for (int i = 0; i < 4; i++) {
		AgentId id(i, rank, 0);
		agents.addAgent(new ModelAgent(id, rank + 10));
	}
	net = new SharedNetwork<ModelAgent, ModelEdge> ("network", true);
	agents.addProjection(net);

	// entire grid is 41 x 61 starting at -20, -30, 2 process per column and 2 per row, with a buffer of 2
	grid = new SharedGrids<ModelAgent>::SharedWrappedGrid("grid", GridDimensions(Point<int>(-20, -30), Point<int> (42, 62)),
			std::vector<int>(2, 2), 2);
	agents.addProjection(grid);

	// entire space is 40 x 60, 2 process per column and 2 per row, with a buffer of 2
	space = new SharedGrids<ModelAgent>::SharedWrappedSpace("grid", GridDimensions(Point<int> (40, 60)),
			std::vector<int>(2, 2), 2);
	agents.addProjection(space);
}

Model::~Model() {

}

void Model::provideContent(const AgentRequest& req, std::vector<ModelAgentContent>& out) {
	const std::vector<AgentId>& ids = req.requestedAgents();

	for (int i = 0, n = ids.size(); i < n; i++) {
		AgentId id = ids[i];
		ModelAgent* node = agents.getAgent(id);
		//std::cout << rank << ": " << id << " " << node << std::endl;
		ModelAgentContent content = { id, node->state() };
		out.push_back(content);
	}
}

void Model::receiveEdgeContent(const EdgeContent& content) {
	// find the matching edge and update it
	AgentId sourceId = content.sourceContent.getId();
	AgentId targetId = content.targetContent.getId();
	ModelEdge* edge = net->findEdge(agents.getAgent(sourceId), agents.getAgent(targetId));
	edge->state(content.state);
}

void Model::provideEdgeContent(const ModelEdge* edge, std::vector<EdgeContent>& edgeContent) {
	ModelAgent* source = edge->source();
	ModelAgent* target = edge->target();
	ModelAgentContent sContent = { source->getId(), source->state() };
	ModelAgentContent tContent = { target->getId(), target->state() };
	EdgeContent content = { sContent, tContent, edge->state() };
	edgeContent.push_back(content);
}

ModelEdge* Model::createEdge(repast::Context<ModelAgent>& context, EdgeContent& edge) {
	AgentId sourceId = edge.sourceContent.getId();
	AgentId targetId = edge.targetContent.getId();

	return new ModelEdge(context.getAgent(sourceId), context.getAgent(targetId));
}

void Model::provideContent(ModelAgent* agent, std::vector<ModelAgentContent>& out) {
	ModelAgentContent content = { agent->getId(), agent->state() };
	out.push_back(content);
}

void Model::createAgents(const std::vector<ModelAgentContent>& contents, std::vector<ModelAgent*>& agents)  {
	for (std::vector<ModelAgentContent>::const_iterator iter = contents.begin(); iter != contents.end(); ++iter) {
		ModelAgentContent content = *iter;
		agents.push_back(new ModelAgent(content.getId(), content.state));
	}
}

ModelAgent* AgentCreatorUpdater::createAgent(const ModelAgentContent& content) {
	AgentId id = content.getId();
	int state = content.state;
	return new ModelAgent(id, state);
}
void AgentCreatorUpdater::updateAgent(const ModelAgentContent& content) {
	ModelAgent* agent = _model->agents.getAgent(content.getId());
	agent->state(content.state);
}
