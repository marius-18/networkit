// networkit-format
#include <cassert>

#include <networkit/graph/GraphBuilder.hpp>
#include <networkit/randomization/BipartiteGlobalCurveball.hpp>

namespace NetworKit {

void BipartiteGlobalCurveball::run(count numberOfGlobalTrades) {
    if (!hasRun) {
        // we may allow multiple calls to run(). In this case, we continue where we left,
        // which is useful if we want to take snapshots of the randomisation process;
        buildAdjList();
    }

    // vector<node> perm = 0 .... |bipartitClass| - 1
    for (count round = 0; round < numberOfGlobalTrades; ++round) {
#if 0
        // std::random_shuffle / std::shuffle (perm)

#pragma omp parallel
        {
            // allok

#pragma omp for
            for (size_t i = 0; i < perm.size(); i += 2) {
                u = perm[i];
                v = perm[i + 1];


            }
        }

        // TODO: implement
#endif
    }

    hasRun = true;
}

void BipartiteGlobalCurveball::buildAdjList() {
    assert(bipartitionClass.size() < inputGraph.numberOfNodes());
    adjList.reserve(bipartitionClass.size());

    for (node u : bipartitionClass) {
        assert(u < inputGraph.upperNodeIdBound());
        adjList.emplace_back();
        auto &current = adjList.back();

        current.resize(inputGraph.degree(u));
        auto range = inputGraph.neighborRange(u);
        std::copy(range.begin(), range.end(), current.begin());
    }
}

Graph BipartiteGlobalCurveball::getGraph() {
    Graph graph(inputGraph.numberOfNodes(), false, inputGraph.isDirected());

    if (inputGraph.isDirected()) {
        inputGraph.forNodes([&](node u) {
            graph.preallocateDirected(u, inputGraph.degreeOut(u), inputGraph.degreeIn(u));
        });
    } else {
        inputGraph.forNodes([&](node u) { graph.preallocateUndirected(u, inputGraph.degree(u)); });
    }

    node i = 0;
    for (auto &vs : adjList) {
        const auto u = bipartitionClass[i++];
        for (auto v : vs) {
            graph.addEdge(u, v);
        }
    }

    return graph;
}

} // namespace NetworKit
