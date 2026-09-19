#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "methods.hpp"
#include "../include/levit.hpp"
#include "../include/weighted_graph.hpp"

namespace graph {

int LevitMethod(const nlohmann::json& input, nlohmann::json* output) {
  if (!input.contains("vertices") || !input["vertices"].is_array() ||
      !input.contains("edges") || !input["edges"].is_array() ||
      !input.contains("start_vertex")) {
    (*output)["error"] =
        "Invalid input format: missing vertices, edges or start_vertex";
    return -1;
  }

  WeightedGraph<double> graph_data;

  for (const auto& v : input["vertices"]) {
    graph_data.AddVertex(v.get<size_t>());
  }

  auto json_edges = input["edges"];
  for (const auto& edge : json_edges) {
    if (edge.contains("from") && edge.contains("to") &&
        edge.contains("weight")) {
      size_t u = edge["from"].get<size_t>();
      size_t v = edge["to"].get<size_t>();
      double weight = edge["weight"].get<double>();

      graph_data.AddEdge(u, v, weight);
    }
  }

  size_t start_vertex = input["start_vertex"].get<size_t>();

  /* ИСПРАВЛЕНИЕ: Если стартовой вершины нет в графе (например, граф пустой),
     то и путей от неё быть не может. Сразу возвращаем пустые distances. */
  if (!graph_data.HasVertex(start_vertex)) {
    (*output)["distances"] = nlohmann::json::object();
    return 0;
  }

  LevitAlgorithm<WeightedGraph<double>> algorithm;
  auto result = algorithm.FindShortestPaths(graph_data, start_vertex);

  nlohmann::json distances_json = nlohmann::json::object();
  for (const auto& pair : result.distances) {
    distances_json[std::to_string(pair.first)] = pair.second;
  }

  (*output)["distances"] = distances_json;

  return 0;
}

}  // namespace graph
