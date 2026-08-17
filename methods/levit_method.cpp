#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "methods.hpp"
#include "../include/levit.hpp"
#include "../include/weighted_graph.hpp"

namespace graph {

int LevitMethod(const nlohmann::json& input, nlohmann::json* output) {
  if (!input.contains("graph") || !input["graph"].is_array() ||
      !input.contains("start_vertex")) {
    (*output)["error"] =
        "Invalid input format: missing graph array or start_vertex";
    return -1;
  }

  WeightedGraph<double> graph_data;
  auto json_graph = input["graph"];

  for (const auto& edge : json_graph) {
    if (edge.contains("u") && edge.contains("v") && edge.contains("weight")) {
      size_t u = edge["u"].get<size_t>();
      size_t v = edge["v"].get<size_t>();
      double weight = edge["weight"].get<double>();

      graph_data.AddVertex(u);
      graph_data.AddVertex(v);
      graph_data.AddEdge(u, v, weight);
    }
  }

  size_t start_vertex = input["start_vertex"].get<size_t>();
  if (!graph_data.HasVertex(start_vertex)) {
    graph_data.AddVertex(start_vertex);
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
