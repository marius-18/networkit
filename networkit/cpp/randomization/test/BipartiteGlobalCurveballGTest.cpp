// networkit-format

#include <numeric>
#include <gtest/gtest.h>
#include <networkit/randomization/BipartiteGlobalCurveball.hpp>

namespace NetworKit {

class CurveballUniformTradeGeneratorGTest : public ::testing::Test {};

static Graph create_random_bipartite_Graph(count n, count part_size, std::vector<node> &partition,
                                           int type) {
    Graph graph(n, false, false);

    assert(part_size < n);

    std::vector<node> second_part;
    second_part.resize(n - part_size); // TODO: DAS MUSS EIN RESIZE SEIN... warum?
    std::iota(second_part.begin(), second_part.end(), part_size);

    if (type == 1) {
        // computes a bipartite Graph with random edges between the partitions

        std::mt19937_64 prng(1);
        std::uniform_int_distribution<node> dis(0, second_part.size() - 1);

        for (node u = 0; u < part_size; ++u) {
            partition.push_back(u);
            std::shuffle(second_part.begin(), second_part.end(), prng);

            node count = dis(prng);

            for (node i = 0; i < count; ++i) {
                graph.addEdge(u, second_part[i]);
            }
        }
    }

    if (type == 2) {
        // creates a bipartite Graph where the nodes are mapped bijectively
        // no trades are possible
        for (node u = 0; u < part_size; ++u) {
            partition.push_back(u);
            graph.addEdge(u, second_part[u]);
        }
    }

    if (type == 3) {
        // nodes 0 ... part_size-1 belong to one class, part_size .. n-1 to the other
        // node 0 <= u < part_size is connected to u many others
        // no trades are possible
        for (node u = 0; u < part_size; ++u) {
            partition.push_back(u);
            for (node v = 0; v < u; ++v)
                graph.addEdge(u, part_size + v);
        }
    }

    return graph;
}

TEST_F(CurveballUniformTradeGeneratorGTest, testBipartition) {

    std::vector<node> partition;
    Graph graphIn = create_random_bipartite_Graph(100, 50, partition, 1);

    BipartiteGlobalCurveball algo(graphIn, partition);

    algo.run(20);

    const auto graphOut = algo.getGraph();

    // check if the number of Nodes changes
    ASSERT_EQ(graphIn.numberOfNodes(), graphOut.numberOfNodes());
    ASSERT_EQ(graphIn.numberOfEdges(), graphOut.numberOfEdges());

    // check if graphIn != graphOut
    bool changes = false;
    graphIn.forEdges([&](node u, node v) {
        if (!graphOut.hasEdge(u, v)) {
            changes = true;
        }
    });
    ASSERT_TRUE(changes);

    // check if the node degrees are the same
    for (node u : partition) {
        ASSERT_EQ(graphIn.degree(u), graphOut.degree(u));
    }
}

TEST_F(CurveballUniformTradeGeneratorGTest, testNoop) {
    // tests the correctness of the conversion to the adjacency-list
    std::vector<node> partition;
    Graph graphIn = create_random_bipartite_Graph(100, 50, partition, 3);

    BipartiteGlobalCurveball algo(graphIn, partition);
    algo.run(0); // do not carry out any trades

    const auto graphOut = algo.getGraph();

    ASSERT_EQ(graphIn.numberOfNodes(), graphOut.numberOfNodes());
    ASSERT_EQ(graphIn.numberOfEdges(), graphOut.numberOfEdges());

    for (node u : partition) {
        ASSERT_EQ(graphIn.degree(u), graphOut.degree(u));
    }

    graphIn.forEdges([&](node u, node v) { ASSERT_TRUE(graphOut.hasEdge(u, v)); });
}

} // namespace NetworKit
