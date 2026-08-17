#include <cassert>
#include <cstdlib>
#include <iostream>
#include <vector>

#include "test.hpp"
#include "../include/levit.hpp"
#include "../include/weighted_graph.hpp"

void TestLevit() {
  graph::LevitAlgorithm<graph::WeightedGraph<double>> algorithm;
  graph::WeightedGraph<double> g;

  g.AddVertex(10);
  g.AddVertex(20);
  g.AddVertex(30);
  g.AddVertex(40);

  g.AddEdge(10, 20, 1.5);
  g.AddEdge(10, 30, 2.0);
  g.AddEdge(20, 30, 0.5);
  g.AddEdge(30, 40, 1.0);

  auto result = algorithm.FindShortestPaths(g, 10);

  assert(result.distances[10] == 0.0);
  assert(result.distances[20] == 1.5);
  assert(result.distances[30] == 2.0);
  assert(result.distances[40] == 3.0);

  graph::WeightedGraph<double> g_rand;
  int n = 10 + std::rand() % 90;
  std::vector<size_t> vertices;

  for (int i = 0; i < n; ++i) {
    size_t v_id = static_cast<size_t>(std::rand() % 1000000);
    g_rand.AddVertex(v_id);
    vertices.push_back(v_id);
  }

  for (int i = 1; i < n; ++i) {
    int parent = std::rand() % i;
    g_rand.AddEdge(vertices[i], vertices[parent],
                   static_cast<double>(std::rand() % 100));
  }

  auto result_rand = algorithm.FindShortestPaths(g_rand, vertices[0]);
  assert(result_rand.distances.size() == static_cast<size_t>(n));

  std::cout << "Levit tests passed.\n";
}
