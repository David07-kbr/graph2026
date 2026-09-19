/**
 * @file levit_test.cpp
 *
 * Тесты для алгоритма Левита.
 */

#include <httplib.h>
#include <unordered_map>
#include <string>
#include <vector>
#include <algorithm>
#include <random>
#include <nlohmann/json.hpp>
#include "test_core.hpp"

static void EmptyGraphTest(httplib::Client* cli);
static void SimpleTest(httplib::Client* cli);
static void RandomTest(httplib::Client* cli);
static void RandomTestHelper(httplib::Client* cli,
    const std::string& graphType);

void TestLevit(httplib::Client* cli) {
  TestSuite suite("TestLevit");

  RUN_TEST_REMOTE(suite, cli, EmptyGraphTest);
  RUN_TEST_REMOTE(suite, cli, SimpleTest);
  RUN_TEST_REMOTE(suite, cli, RandomTest);
}

/**
 * @brief Тест для пустого графа.
 *
 * @param cli Указатель на HTTP клиент.
 */
static void EmptyGraphTest(httplib::Client* cli) {
  nlohmann::json input = R"(
{
  "graph_type": "OrientedWeightedGraph",
  "start_vertex": 1,
  "vertices": [ ],
  "edges": [ ]
}
)"_json;

  /* Делаем POST запрос по адресу нашего метода на сервере. */
  auto res = cli->Post("/Levit", input.dump(), "application/json");

  if (!res) {
    REQUIRE(false);
  }

  /* Используем метод parse() для преобразования строки ответа сервера. */
  nlohmann::json output = nlohmann::json::parse(res->body);

  REQUIRE(output.at("distances").empty());
}

/**
 * @brief Простейший статический тест.
 *
 * @param cli Указатель на HTTP клиент.
 */
static void SimpleTest(httplib::Client* cli) {
  nlohmann::json input;

  input["graph_type"] = "OrientedWeightedGraph";
  input["start_vertex"] = 1;
  input["vertices"] = std::vector<int>{1, 2, 3, 4};

  input["edges"][0]["from"] = 1;
  input["edges"][0]["to"] = 2;
  input["edges"][0]["weight"] = 2.0;

  input["edges"][1]["from"] = 1;
  input["edges"][1]["to"] = 3;
  input["edges"][1]["weight"] = 5.0;

  input["edges"][2]["from"] = 2;
  input["edges"][2]["to"] = 3;
  input["edges"][2]["weight"] = 1.0;

  input["edges"][3]["from"] = 3;
  input["edges"][3]["to"] = 4;
  input["edges"][3]["weight"] = 2.0;

  auto res = cli->Post("/Levit", input.dump(), "application/json");

  if (!res) {
    REQUIRE(false);
  }

  nlohmann::json output = nlohmann::json::parse(res->body);
  std::unordered_map<int, double> distances;

  /* В JSON ключи всегда строки, поэтому переводим их обратно в int. */
  for (auto& el : output.at("distances").items()) {
    distances[std::stoi(el.key())] = el.value().get<double>();
  }

  /* Ожидаемые кратчайшие расстояния от вершины 1 */
  std::unordered_map<int, double> expected = {
    {1, 0.0},
    {2, 2.0},
    {3, 3.0}, /* Путь 1->2->3 (2.0 + 1.0) короче, чем 1->3 (5.0) */
    {4, 5.0}  /* Путь 1->2->3->4 (3.0 + 2.0) */
  };

  REQUIRE_EQUAL(expected, distances);
}

/**
 * @brief Случайный тест.
 *
 * @param cli Указатель на HTTP клиент.
 */
static void RandomTest(httplib::Client* cli) {
  RandomTestHelper(cli, "OrientedWeightedGraph");
}

/**
 * @brief Вспомогательный метод для генерации случайного графа.
 *
 * @param cli Указатель на HTTP клиент.
 * @param graphType Тип графа.
 */
static void RandomTestHelper(httplib::Client* cli,
    const std::string& graphType) {
  const int numTries = 100;
  std::random_device rd;
  std::mt19937 gen(rd());

  std::uniform_int_distribution<size_t> vertexId(100, 150);
  std::uniform_int_distribution<size_t> edgesSize(10, 300);
  std::uniform_real_distribution<double> weightDist(1.0, 100.0);

  for (int it = 0; it < numTries; it++) {
    size_t numEdges = edgesSize(gen);
    std::vector<size_t> vertices;
    nlohmann::json input;

    input["graph_type"] = graphType;

    size_t startVertex = vertexId(gen);
    input["start_vertex"] = startVertex;
    vertices.push_back(startVertex);

    size_t numRealEdges = 0;
    for (size_t i = 0; i < numEdges; i++) {
      size_t id1 = vertexId(gen);
      size_t id2 = vertexId(gen);
      double weight = weightDist(gen);

      if (id1 == id2)
        continue;

      vertices.push_back(id1);
      vertices.push_back(id2);

      input["edges"][numRealEdges]["from"] = id1;
      input["edges"][numRealEdges]["to"] = id2;
      input["edges"][numRealEdges]["weight"] = weight;
      numRealEdges++;
    }

    std::sort(vertices.begin(), vertices.end());
    vertices.erase(std::unique(vertices.begin(), vertices.end()),
        vertices.end());

    input["vertices"] = vertices;

    auto res = cli->Post("/Levit", input.dump(), "application/json");

    if (!res) {
      REQUIRE(false);
    }

    nlohmann::json output = nlohmann::json::parse(res->body);

    /* Проверяем, что сервер ответил 200 OK и вернул хоть какой-то JSON */
    REQUIRE_EQUAL(res->status, 200);
    REQUIRE(output.contains("distances"));
  }
}
