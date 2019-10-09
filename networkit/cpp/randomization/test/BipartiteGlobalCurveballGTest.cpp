// networkit-format

#include <gtest/gtest.h>

#include <networkit/randomization/BipartiteGlobalCurveball.hpp>

namespace NetworKit {

class CurveballUniformTradeGeneratorGTest : public ::testing::Test {};

TEST_F(CurveballUniformTradeGeneratorGTest, testNoop) {
    Graph graphIn(20);

    // nodes 0 ... 9 belong to one class, 10 .. 19 to the other
    // node 0 <= u < 10 is connected to u many others
    std::vector<node> partition;
    for (node u = 1; u < 10; ++u) {
        partition.push_back(u);
        for (node v = 0; v < u; ++v)
            graphIn.addEdge(u, 10 + v);
    }

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
