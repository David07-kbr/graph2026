#ifndef INCLUDE_LEVIT_HPP_
#define INCLUDE_LEVIT_HPP_

#include <deque>
#include <unordered_map>
#include <utility>

namespace graph {

template <typename Graph>
class LevitAlgorithm {
 public:
  using WeightType = typename Graph::WeightType;

  struct Result {
    std::unordered_map<size_t, WeightType> distances;
    std::unordered_map<size_t, size_t> parents;
  };

  Result FindShortestPaths(const Graph& graph, size_t start_vertex) {
    Result result;
    if (!graph.HasVertex(start_vertex)) {
      return result;
    }

    // Состояния: 0 - M2 (не вычислено), 1 - M1 (в очереди), 2 - M0 (вычислено)
    std::unordered_map<size_t, int> state;
    std::deque<size_t> q;

    for (size_t v : graph.Vertices()) {
      state[v] = 0;
    }

    result.distances[start_vertex] = WeightType();
    state[start_vertex] = 1;
    q.push_back(start_vertex);

    while (!q.empty()) {
      size_t v = q.front();
      q.pop_front();
      state[v] = 2;

      for (size_t to : graph.Edges(v)) {
        WeightType len = graph.EdgeWeight(v, to);
        bool has_dist = result.distances.find(to) != result.distances.end();

        if (!has_dist || result.distances[to] > result.distances[v] + len) {
          result.distances[to] = result.distances[v] + len;
          result.parents[to] = v;

          if (state[to] == 0) {
            q.push_back(to);
          } else if (state[to] == 2) {
            q.push_front(to);
          }
          state[to] = 1;
        }
      }
    }

    return result;
  }
};

}  // namespace graph

#endif  // INCLUDE_LEVIT_HPP_
