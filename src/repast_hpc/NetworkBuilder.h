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
 *  NetworkBuilder.h
 *
 *  Created on: Oct 30, 2009
 *      Author: nick
 */

#ifndef NETWORKBUILDER_H_
#define NETWORKBUILDER_H_

#include "Graph.h"
#include "Properties.h"
#include "Utilities.h"
#include "Random.h"

#include <string>

namespace repast {

/**
 * Helper class for calculating outcomes based on
 * a set of probabilities that sum to 1.
 */
class ProbItem {
private:
	int _index;
	double lowerBound, upperBound;

public:
	ProbItem(int i, double lb, double ub);
	bool contains(double val);

	int index() const {
		return _index;
	}
};

/**
 * Buils KE type networks. See Klemm and Eguiluz,
 * "Growing scale-free network with small world behavior" in
 * Phys. Rev. E 65.
 */
template<typename V, typename E, typename Ec, typename EcM>
class KEBuilder {

private:
	static const std::string M;

public:
	/**
	 * Builds the network. The graph should contains the vertices the build the
	 * network with and props should contain the M values.
	 *
	 * @param props a Properties containing a property "ke.model.m" that specifies
	 * the M value.
	 * @param graph the graph to build the network
	 */
	void build(repast::Properties& props, repast::Graph<V, E, Ec, EcM>* graph);
};

template<typename V, typename E, typename Ec, typename EcM>
const std::string KEBuilder<V, E, Ec, EcM>::M = "ke.model.m";

template<typename V, typename E, typename Ec, typename EcM>
void KEBuilder<V, E, Ec, EcM>::build(repast::Properties& props, repast::Graph<V, E, Ec, EcM>* graph) {
	int m = strToInt(props.getProperty(M));
	typename repast::Graph<V, E, Ec, EcM>::vertex_iterator iter;
	int k = 0;
	// advance iter m - 1 number of elements.
	for (iter = graph->verticesBegin(); k < m; ++k, ++iter);
	std::vector<V*> activeNodes(graph->verticesBegin(), iter);
	// fully connect all the active nodes
	for (int i = 0; i < m; i++) {
		V* source = activeNodes[i];
		for (int j = 0; j < m; j++) {
			if (i != j) {
				graph->addEdge(source, (activeNodes[j]));
			}
		}
	}
	// add the remaining nodes -- iter through verticesEnd()
	while (iter != graph->verticesEnd()) {
		V* source = *iter;
		double sum = 0;
		// make an edge between iter and all the active nodes
		for (int i = 0, n = activeNodes.size(); i < n; i++) {
			V* target = activeNodes[i];
			graph->addEdge(source, target);
			sum += 1.0 / graph->inDegree(target);
		}

		std::vector<ProbItem> probItems;
		double lowerBound = 0;
		for (int i = 0, n = activeNodes.size(); i < n; i++) {
			V* node = activeNodes[i];
			double upperBound = lowerBound + (1.0 / graph->inDegree(node) / sum);
			probItems.push_back(ProbItem(i, lowerBound, upperBound));
			lowerBound = upperBound;
		}

		double p = repast::Random::instance()->nextDouble();
		for (int i = 0, n = probItems.size(); i < n; i++) {
			ProbItem& item = probItems[i];
			if (item.contains(p)) {
				activeNodes.erase(activeNodes.begin() + item.index());
				break;
			}
		}
		activeNodes.push_back(source);
		++iter;
	}
}

}

#endif /* NETWORKBUILDER_H_ */
