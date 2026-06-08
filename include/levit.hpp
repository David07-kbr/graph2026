/**
 * @file include/levit.hpp
 * @author David 
 *
 * Алгоритм Левита для нахождения кратчайших путей.
 */

#ifndef INCLUDE_LEVIT_HPP_
#define INCLUDE_LEVIT_HPP_

#include <deque>
#include <unordered_map>
#include <limits>
#include <cstddef>

namespace graph {

/**
 * @brief Результат работы алгоритма Левита.
 *
 * @tparam Weight Тип веса рёбер графа.
 */
template<typename Weight>
struct LevitResult {
  //! Кратчайшие расстояния от стартовой вершины.
  std::unordered_map<size_t, Weight> distances;

  //! Предшественники на кратчайшем пути.
  std::unordered_map<size_t, size_t> predecessors;
};

/**
 * @brief Алгоритм Левита для нахождения кратчайших путей от заданной
 * вершины до всех остальных.
 *
 * @tparam GraphType Тип графа. Должен иметь методы Vertices(),
 *         Edges(id), EdgeWeight(id1, id2) и тип WeightType.
 * @param graph Взвешенный граф.
 * @param source Стартовая вершина.
 * @return Структура LevitResult с расстояниями и предшественниками.
 *
 * Алгоритм использует три множества вершин (M0, M1, M2) и
 * двунаправленную очередь (deque).
 *
 * Время работы
 * \f$
 * O(N \cdot M)
 * \f$.
 *
 * На практике алгоритм работает за O(M log N).
 *
 * Алгоритм основан на описании с
 * http://e-maxx.ru/algo/levit_algorithm
 */
template<typename GraphType>
LevitResult<typename GraphType::WeightType> Levit(
    const GraphType& graph, size_t source) {
  using Weight = typename GraphType::WeightType;
  const Weight inf = std::numeric_limits<Weight>::max();

  LevitResult<Weight> result;

  /* Инициализация расстояний. */
  for (size_t v : graph.Vertices()) {
    result.distances[v] = inf;
  }
  result.distances[source] = Weight();

  /*
   * id[v]: принадлежность вершины к множеству.
   * 0 — M2 (расстояние ещё не вычислено),
   * 1 — M1 (в очереди, расстояние вычисляется),
   * 2 — M0 (расстояние вычислено окончательно).
   */
  std::unordered_map<size_t, int> id;
  for (size_t v : graph.Vertices()) {
    id[v] = 0;
  }

  std::deque<size_t> q;
  q.push_back(source);
  id[source] = 1;

  while (!q.empty()) {
    size_t v = q.front();
    q.pop_front();
    id[v] = 2;

    for (size_t to : graph.Edges(v)) {
      Weight len = graph.EdgeWeight(v, to);
      Weight newDist = result.distances[v] + len;

      if (newDist < result.distances[to]) {
        result.distances[to] = newDist;
        result.predecessors[to] = v;

        if (id[to] == 0) {
          /* Вершина из M2: добавляем в конец очереди. */
          q.push_back(to);
          id[to] = 1;
        } else if (id[to] == 2) {
          /* Вершина из M0: возвращаем в начало очереди. */
          q.push_front(to);
          id[to] = 1;
        }
        /*
         * Если id[to] == 1 (вершина уже в M1), то расстояние
         * обновлено, но позиция в очереди не меняется.
         */
      }
    }
  }

  return result;
}

}  // namespace graph

#endif  // INCLUDE_LEVIT_HPP_
