/*
 * BipartiteGlobalCurveball.h
 *
 *  Created on: 26.05.2018
 *      Author: Marius Hagemann, Manuel Penschuck <networkit@manuel.jetzt>
 */
#ifndef NETWORKIT_RANDOMIZATION_BIPARTITE_GLOBAL_CURVEBALL_HPP_
#define NETWORKIT_RANDOMIZATION_BIPARTITE_GLOBAL_CURVEBALL_HPP_

#include <string>
#include <vector>

#include <networkit/base/Algorithm.hpp>
#include <networkit/graph/Graph.hpp>

namespace NetworKit {

class BipartiteGlobalCurveball final : public Algorithm {
public:
    /**
     * Instantiate a BipartiteGlobalCurveball object
     *
     * @param G                        Undirected and unweighted graph to be randomized
     * @param bipartition              One bipartition class of G (i.e., a vector of node ids
     *                                 such that there exists no edge between two nodes in
     *                                 the set or its complement).
     * @param number_of_global_trades  Number of global trades to be executed (each edge
     *                                 is considered exactly twice per global traded)
     *
     * @note Self loops can only be realized for directed graphs.
     * @warning If self loops are forbidden, degreePreservingShuffle is necessary for
     * directed graphs, since otherwise some topologies cannot be realized (i.e., only
     * preprocessing allows for uniform samples).
     */
    BipartiteGlobalCurveball(const Graph &G, const std::vector<node> &bipartition,
                             count number_of_global_trades = 20)
        : inputGraph(G), bipartitionClass(bipartition), numGlobalTrades(number_of_global_trades) {}

    virtual ~BipartiteGlobalCurveball() = default;

    /**
     * Execute trades as configured in the constructor; multiple calls are supported.
     * @warning This function has to be called at least once before invoking getGraph()
     */
    void run() override { run(numGlobalTrades); }

    void run(count numberOfGlobalTrades);

    /**
     * Returns a new graph instance with the same degree sequence as the input
     * graph, but with randomized neighbourhoods.
     */
    Graph getGraph();

    std::string toString() const override { return "BipartiteGlobalCurveball"; }

    bool isParallel() const override { return false; }

private:
    const Graph& inputGraph;
    const std::vector<node>& bipartitionClass;
    unsigned numGlobalTrades;

    std::vector<std::vector<node>> adjList;

    void buildAdjList();

    //TODO: static oder nicht static?
    static void compute_common_disjoint(std::vector<node> &neighbourhood_of_u,
                                 std::vector<node> &neighbourhood_of_v,
                                 std::vector<node> &common_neighbours,
                                 std::vector<node> &disjoint_neighbours);

    static void make_trade(std::vector<node> &common,
                    std::vector<node> &disjoint,
                    std::vector<node> &neighbourhood_of_u,
                    std::vector<node> &neighbourhood_of_v,
                    std::mt19937_64 &prng);
};

} // namespace NetworKit

#endif // NETWORKIT_RANDOMIZATION_BIPARTITE_GLOBAL_CURVEBALL_HPP_
