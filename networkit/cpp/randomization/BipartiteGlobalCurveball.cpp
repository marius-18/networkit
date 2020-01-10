// networkit-format
#include <cassert>

#include <networkit/graph/GraphBuilder.hpp>
#include <networkit/randomization/BipartiteGlobalCurveball.hpp>

// darf ich das?
#include <tlx/algorithm/random_bipartition_shuffle.hpp>

#include <iostream>
#include <networkit/graph/GraphTools.hpp>

namespace NetworKit {

static void common_disjoint_sortsort(std::vector<node> &neighbourhood_of_u,
                                     std::vector<node> &neighbourhood_of_v,
                                     std::vector<node> &common_neighbours,
                                     std::vector<node> &disjoint_neighbours) {

    assert(common_neighbours.empty());
    assert(disjoint_neighbours.empty());

    // sort neighbourhoods to easily identify common neighbours
    // was machen wir mit dem vorsortieren?!?!
    // if (true){
    std::sort(neighbourhood_of_u.begin(), neighbourhood_of_u.end());
    std::sort(neighbourhood_of_v.begin(), neighbourhood_of_v.end());
    //}

    auto u_nit = neighbourhood_of_u.cbegin();
    auto v_nit = neighbourhood_of_v.cbegin();

    while ((u_nit != neighbourhood_of_u.cend()) && (v_nit != neighbourhood_of_v.cend())) {

        if (*u_nit > *v_nit) {
            disjoint_neighbours.push_back(*v_nit);
            v_nit++;
        } else if (*u_nit < *v_nit) {
            disjoint_neighbours.push_back(*u_nit);
            u_nit++;
        } else /* (*u_nit == *v_nit) */ {
            common_neighbours.push_back(*v_nit);
            u_nit++;
            v_nit++;
        }
    }

    // Append yet unprocessed neighbours (which have to be
    // disjoint since the trading partner has no more neighbours)
    if (u_nit != neighbourhood_of_u.cend()) {
        disjoint_neighbours.insert(disjoint_neighbours.end(), u_nit, neighbourhood_of_u.cend());
    } else {
        disjoint_neighbours.insert(disjoint_neighbours.end(), v_nit, neighbourhood_of_v.cend());
    }
}

static void trade(std::vector<node> &common, std::vector<node> &disjoint,
                  std::vector<node> &neighbourhood_of_u, std::vector<node> &neighbourhood_of_v,
                  std::mt19937_64 &prng) {

    // ANNAHME: u ist der größere Vektor!
    // evtl Umbauen, mit Fallunterscheidung?

    // tausche die Elemente in disjoint, aber nur so dass die erste partition zufällig ist, rest
    // egal
    const size_t partition_size = neighbourhood_of_v.size() - common.size();
    tlx::random_bipartition_shuffle(disjoint.begin(), disjoint.end(), partition_size, prng);

    // kopiere die gleichen elemente in u und v
    std::copy(common.begin(), common.end(), neighbourhood_of_u.begin());
    std::copy(common.begin(), common.end(), neighbourhood_of_v.begin());

    // u := large, v := small
    // Beginn nach common.size()
    // in v die ersten Elemente, der Rest in u
    std::copy(disjoint.begin(), disjoint.begin() + partition_size,
              neighbourhood_of_v.begin() + common.size());
    std::copy(disjoint.begin() + partition_size, disjoint.end(),
              neighbourhood_of_u.begin() + common.size());
}

void BipartiteGlobalCurveball::run(count numberOfGlobalTrades) {
    if (!hasRun) {
        // we may allow multiple calls to run(). In this case, we continue where we left,
        // which is useful if we want to take snapshots of the randomisation process;
        buildAdjList();
    }

    // wohin hiermit? gibts da probleme mit der parallelität?
    std::random_device rd;
    std::mt19937_64 prng(rd());

    for (count round = 0; round < numberOfGlobalTrades; ++round) {
        //#if 0

        // Permutationsvektor erstellen:
        std::vector<node> perm;
        for (node i = 0; i < adjList.size(); ++i) {
            perm.push_back(i);
        }

        std::shuffle(perm.begin(), perm.end(), prng);

        //#pragma omp parallel
        {
            // allok
            std::vector<node> common, disjoint;
            // maxDegree() ist deprecated
            int maxGrad = NetworKit::GraphTools::maxDegree(inputGraph);

            disjoint.reserve(2 * maxGrad);
            common.reserve(2 * maxGrad);

            std::vector<node> *u;
            std::vector<node> *v;

            //#pragma omp for
            for (size_t i = 0; i < adjList.size() - 1; i += 2) {
                common.clear();
                disjoint.clear();

                // ich bin mir nicht sicher ob man das so macht mit den adressen und pointern für
                // u,v...
                u = &adjList[perm[i]];
                v = &adjList[perm[i + 1]];

                NetworKit::common_disjoint_sortsort(*u, *v, common, disjoint);

                // u.clear();
                // v.clear();

                if ((*u).size() >= (*v).size()) {
                    NetworKit::trade(common, disjoint, *u, *v, prng);
                } else {
                    NetworKit::trade(common, disjoint, *v, *u, prng);
                }
            }
        }
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
