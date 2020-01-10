// networkit-format

#include <gtest/gtest.h>
#include <networkit/randomization/BipartiteGlobalCurveball.hpp>

namespace NetworKit {

class CurveballUniformTradeGeneratorGTest : public ::testing::Test {};

enum class CreateType { zufall, bijektiv, Default };

template <CreateType CType>
static Graph create_random_bipartite_Graph(node n, node part_size, std::vector<node> &partition) {
    Graph graph(n, false, false);

    assert(part_size < n);
    std::vector<node> second_part;

    for (node i = part_size; i < n; ++i) {
        second_part.push_back(i);
    }

    std::random_device rd;
    std::mt19937_64 prng(rd());

    std::uniform_int_distribution<node> dis;

    if (CType == CreateType::zufall) {
        for (node u = 0; u < part_size; ++u) {
            partition.push_back(u);
            std::shuffle(second_part.begin(), second_part.end(), prng);
            node count = (dis(prng) % (second_part.size() - 1)) + 1;

            for (node i = 0; i < count; ++i) {
                graph.addEdge(u, second_part[i]);
            }
        }
    }

    if (CType == CreateType::bijektiv) {
        for (node u = 0; u < part_size; ++u) {
            partition.push_back(u);
            graph.addEdge(u, second_part[u]);
        }
    }

    if (CType == CreateType::Default) {
        // kein so gutes Beispiel, weils da nix zu traden gibt....
        // nodes 0 ... 9 belong to one class, 10 .. 19 to the other
        // node 0 <= u < 10 is connected to u many others
        for (node u = 0; u < part_size; ++u) {
            partition.push_back(u);
            for (node v = 0; v < u; ++v)
                graph.addEdge(u, part_size + v);
        }
    }

    return graph;
}

TEST_F(CurveballUniformTradeGeneratorGTest, testNoop) {

    std::vector<node> partition;
    Graph graphIn = create_random_bipartite_Graph<CreateType ::zufall>(1000, 500, partition);

    BipartiteGlobalCurveball algo(graphIn, partition);
    algo.run(10); // do not carry out any trades

    const auto graphOut = algo.getGraph();

    /*
    for (auto x: graphIn.edges()){
        std::cout << "(" << x.first << "," << x.second << ")" << std::endl;
    }
    std::cout << "neu:" << std::endl;
    for (auto x: graphOut.edges()){
        std::cout << "(" << x.first << "," << x.second << ")" << std::endl;
    }
    */

    ASSERT_EQ(graphIn.numberOfNodes(), graphOut.numberOfNodes());
    ASSERT_EQ(graphIn.numberOfEdges(), graphOut.numberOfEdges());

    bool changes = false;
    graphIn.forEdges([&](node u, node v) {
        if (!graphOut.hasEdge(u, v)) {
            changes = true;
        }
    });
    graphOut.forEdges([&](node u, node v) {
        if (!graphIn.hasEdge(u, v)) {
            changes = true;
        }
    });
    ASSERT_TRUE(changes);

    for (node u : partition) {
        // std::cout << "deg_in(" << u << "): " << graphIn.degree(u) ;
        // std::cout << " -- deg_out(" << u << "): " << graphOut.degree(u) << std::endl;
        ASSERT_EQ(graphIn.degree(u), graphOut.degree(u));
    }

    // das Prüft ob der Graph gleichgeblieben ist oder ?
    // graphIn.forEdges([&](node u, node v) { ASSERT_TRUE(graphOut.hasEdge(u, v)); });
}

} // namespace NetworKit
