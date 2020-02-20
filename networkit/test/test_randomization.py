#!/usr/bin/env python3
import unittest
import networkit as nk
import random

def check_graphs(G1, G2):
    assert(G1.numberOfNodes() == G2.numberOfNodes())
    assert(G1.numberOfEdges() == G2.numberOfEdges())

    failed = False
    for i in range(G1.numberOfNodes()):
        if (G1.degree(i) != G2.degree(i)):
            print("Degree mismatch of node %d (%d != %d)" % (i, G1.degree(i), G2.degree(i)))
            failed = True

        if (G1.degreeIn(i) != G2.degreeIn(i)):
            print("In-Degree mismatch of node %d (%d != %d)" % (i, G1.degree(i), G2.degree(i)))
            failed = True

    if failed:
        nk.overview(G1)
        nk.overview(G2)
        raise RuntimeError("Degree mismatch")

class TestRandomization(unittest.TestCase):
    def setUp(self):
        self.graphs = []
        self.graphs.append(nk.generators.HyperbolicGenerator(1001,  50).generate())
        self.graphs.append(nk.generators.HyperbolicGenerator(1002,   5).generate())
        self.graphs.append(nk.generators.HyperbolicGenerator(10003, 10).generate())
        self.graphs.append(nk.generators.ErdosRenyiGenerator(1004, 0.005).generate())
        self.graphs.append(nk.generators.ErdosRenyiGenerator(1005, 0.05).generate())
        self.graphs.append(nk.generators.ErdosRenyiGenerator(1004, 0.005, True).generate())
        self.graphs.append(nk.generators.ErdosRenyiGenerator(1005, 0.05, True).generate())

    def test_global_curveball(self):
        for G in self.graphs:
            algo = nk.randomization.GlobalCurveball(G, 5)
            algo.run()
            G2 = algo.getGraph()
            check_graphs(G, G2)

    def test_global_curveball_with_selfloops(self):
        for G in self.graphs:
            if not G.isDirected(): continue

            algo = nk.randomization.GlobalCurveball(G, 5, True, True)
            algo.run()
            G2 = algo.getGraph()
            check_graphs(G, G2)

    def test_global_curveball_with_preprocessing(self):
        for G in self.graphs:
            algo = nk.randomization.GlobalCurveball(G, 5, False, True)
            algo.run()
            G2 = algo.getGraph()
            check_graphs(G, G2)

    def test_curveball_with_global(self):
        for G in self.graphs:
            if G.isDirected(): continue

            n = G.numberOfNodes()
            ts = nk.randomization.CurveballGlobalTradeGenerator(5, n).generate()
            algo = nk.randomization.Curveball(G)
            algo.run(ts)
            algo.run(ts)
            G2 = algo.getGraph()
            check_graphs(G, G2)

    def test_curveball_with_uniform(self):
        for G in self.graphs:
            if G.isDirected(): continue

            n = G.numberOfNodes()
            ts = nk.randomization.CurveballUniformTradeGenerator(5 * n, n).generate()
            algo = nk.randomization.Curveball(G)
            algo.run(ts)
            algo.run(ts)
            G2 = algo.getGraph()
            check_graphs(G, G2)

    def test_degree_preserving_shuffle(self):
        for G in self.graphs:
            dps = nk.randomization.DegreePreservingShuffle(G)
            dps.run()
            G2 = dps.getGraph()
            check_graphs(G, G2)
            perm = dps.getPermutation()
            for u in range(G.numberOfNodes()):
                self.assertEqual(G.degree(u), G.degree(perm[u]))
                self.assertEqual(G.degreeIn(u), G.degreeIn(perm[u]))

    def test_bipartite_global_curveball(self):
        G = nk.Graph(100)
        nodes = list(range(0, G.numberOfNodes()))
        clsA = random.sample(nodes, len(nodes) // 3)
        clsB = list(set(nodes).difference(clsA))

        while G.numberOfEdges() < G.numberOfNodes() * 10:
            u = random.choice(clsA)
            v = random.choice(clsB)
            if not G.hasEdge(u, v):
                G.addEdge(u, v)

        for cls in [clsA, clsB]:
            alg = nk.randomization.BipartiteGlobalCurveball(G, cls, 20)
            alg.run()
            G1 = alg.getGraph()

            for u in nodes:
                self.assertEqual(G.degree(u), G1.degree(u))

    def test_degree_preserving_shuffle_directed_triangle(self):
        """Test whether a directed triangle is reoriented in 50% of cases"""
        G = nk.Graph(3, False, True)
        G.addEdge(0, 1)
        G.addEdge(1, 2)
        G.addEdge(2, 0)

        num_clockwise = 0
        num_iterations = 1000

        for i in range(num_iterations):
            dps = nk.randomization.DegreePreservingShuffle(G)
            dps.run()
            G2 = dps.getGraph()
            check_graphs(G, G2)

            # check orientation
            clockwise = G2.hasEdge(0, 1)
            anticlkw  = G2.hasEdge(1, 0)
            self.assertNotEqual(clockwise, anticlkw)

            num_clockwise += clockwise
            G = G2

        # confidence interval with an error rate of ~ 1e-6
        self.assertGreater(num_clockwise, 400)
        self.assertLess   (num_clockwise, 600)

if __name__ == "__main__":
    unittest.main()
