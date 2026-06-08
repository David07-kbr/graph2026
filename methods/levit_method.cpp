/**
 * @file methods/levit_method.cpp
 * @author Anton
 *
 * Серверная часть алгоритма Левита.
 */

#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include <weighted_graph.hpp>
#include <weighted_oriented_graph.hpp>
#include <levit.hpp>

using graph::WeightedGraph;
using graph::WeightedOrientedGraph;

namespace graph {

/**
 * @brief Метод алгоритма Левита.
 *
 * @param input Входные данные в формате JSON.
 * @param output Выходные данные в формате JSON.
 * @return Функция возвращает 0 в случае успеха и отрицательное число
 * если входные данные заданы некорректно.
 *
 * Функция запускает алгоритм Левита, используя входные данные
 * в JSON формате. Результат также выдаётся в JSON формате.
 */
template<typename GraphType>
int LevitMethodHelper(const nlohmann::json& input,
    nlohmann::json* output) {
  using Weight = typename GraphType::WeightType;

  GraphType graph;

  for (auto& vertex : input.at("vertices")) {
    graph.AddVertex(vertex);
  }

  for (auto& edge : input.at("edges")) {
    graph.AddEdge(edge.at("from"), edge.at("to"),
                  edge.at("weight").get<Weight>());
  }

  size_t source = input.at("source").get<size_t>();

  LevitResult<Weight> result = Levit(graph, source);

  /* Записываем расстояния в выходной JSON. */
  nlohmann::json distances;
  for (const auto& pair : result.distances) {
    distances[std::to_string(pair.first)] = pair.second;
  }
  (*output)["distances"] = distances;

  /* Записываем предшественников в выходной JSON. */
  nlohmann::json predecessors;
  for (const auto& pair : result.predecessors) {
    predecessors[std::to_string(pair.first)] = pair.second;
  }
  (*output)["predecessors"] = predecessors;

  return 0;
}

int LevitMethod(const nlohmann::json& input,
    nlohmann::json* output) {
  std::string graphType = input.at("graph_type");

  if (graphType == "WeightedGraph") {
    std::string weightType = input.at("weight_type");
    if (weightType == "int") {
      return LevitMethodHelper<WeightedGraph<int>>(input, output);
    } else if (weightType == "double") {
      return LevitMethodHelper<WeightedGraph<double>>(
          input, output);
    } else {
      return -1;
    }
  } else if (graphType == "WeightedOrientedGraph") {
    std::string weightType = input.at("weight_type");
    if (weightType == "int") {
      return LevitMethodHelper<WeightedOrientedGraph<int>>(
          input, output);
    } else if (weightType == "double") {
      return LevitMethodHelper<WeightedOrientedGraph<double>>(
          input, output);
    } else {
      return -1;
    }
  }

  return -1;
}

}  // namespace graph
