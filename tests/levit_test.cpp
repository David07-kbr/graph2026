/**
 * @file tests/levit_test.cpp
 * @author Anton
 *
 * Тесты для алгоритма graph::Levit.
 */

#include <httplib.h>
#include <unordered_set>
#include <unordered_map>
#include <map>
#include <string>
#include <vector>
#include <algorithm>
#include <random>
#include <limits>
#include <nlohmann/json.hpp>
#include "test_core.hpp"

static void EmptyGraphTest(httplib::Client* cli);
static void SimpleTest(httplib::Client* cli);
static void DiamondTest(httplib::Client* cli);
static void UnreachableTest(httplib::Client* cli);
static void RandomTest(httplib::Client* cli);
static void RandomTestHelper(httplib::Client* cli,
    const std::string& graphType);

void TestLevit(httplib::Client* cli) {
  TestSuite suite("TestLevit");

  RUN_TEST_REMOTE(suite, cli, EmptyGraphTest);
  RUN_TEST_REMOTE(suite, cli, SimpleTest);
  RUN_TEST_REMOTE(suite, cli, DiamondTest);
  RUN_TEST_REMOTE(suite, cli, UnreachableTest);
  RUN_TEST_REMOTE(suite, cli, RandomTest);
}

/**
 * @brief Простейший статический тест для графа с одной вершиной.
 *
 * @param cli Указатель на HTTP клиент.
 */
static void EmptyGraphTest(httplib::Client* cli) {
  nlohmann::json input;
  input["graph_type"] = "WeightedOrientedGraph";
  input["weight_type"] = "int";
  input["vertices"] = std::vector<int>{0};
  input["edges"] = nlohmann::json::array();
  input["source"] = 0;

  auto res = cli->Post("/Levit", input.dump(),
      "application/json");

  if (!res) {
    REQUIRE(false);
  }

  nlohmann::json output = nlohmann::json::parse(res->body);

  int d0 = output.at("distances").at("0").get<int>();

  REQUIRE_EQUAL(d0, 0);
}

/**
 * @brief Статический тест на линейном графе 0 -> 1 -> 2.
 *
 * @param cli Указатель на HTTP клиент.
 */
static void SimpleTest(httplib::Client* cli) {
  nlohmann::json input;
  input["graph_type"] = "WeightedOrientedGraph";
  input["weight_type"] = "int";
  input["vertices"] = std::vector<int>{0, 1, 2};

  input["edges"][0]["from"] = 0;
  input["edges"][0]["to"] = 1;
  input["edges"][0]["weight"] = 5;

  input["edges"][1]["from"] = 1;
  input["edges"][1]["to"] = 2;
  input["edges"][1]["weight"] = 3;

  input["source"] = 0;

  auto res = cli->Post("/Levit", input.dump(),
      "application/json");

  if (!res) {
    REQUIRE(false);
  }

  nlohmann::json output = nlohmann::json::parse(res->body);

  int d0 = output.at("distances").at("0").get<int>();
  int d1 = output.at("distances").at("1").get<int>();
  int d2 = output.at("distances").at("2").get<int>();

  REQUIRE_EQUAL(d0, 0);
  REQUIRE_EQUAL(d1, 5);
  REQUIRE_EQUAL(d2, 8);
}

/**
 * @brief Статический тест на графе-ромбе.
 *
 * Граф:
 *   0 --5--> 1 --2--> 3
 *   0 --1--> 2 --1--> 3
 * Кратчайший путь 0->3: через вершину 2, длина 2.
 *
 * @param cli Указатель на HTTP клиент.
 */
static void DiamondTest(httplib::Client* cli) {
  nlohmann::json input;
  input["graph_type"] = "WeightedOrientedGraph";
  input["weight_type"] = "int";
  input["vertices"] = std::vector<int>{0, 1, 2, 3};

  input["edges"][0]["from"] = 0;
  input["edges"][0]["to"] = 1;
  input["edges"][0]["weight"] = 5;

  input["edges"][1]["from"] = 1;
  input["edges"][1]["to"] = 3;
  input["edges"][1]["weight"] = 2;

  input["edges"][2]["from"] = 0;
  input["edges"][2]["to"] = 2;
  input["edges"][2]["weight"] = 1;

  input["edges"][3]["from"] = 2;
  input["edges"][3]["to"] = 3;
  input["edges"][3]["weight"] = 1;

  input["source"] = 0;

  auto res = cli->Post("/Levit", input.dump(),
      "application/json");

  if (!res) {
    REQUIRE(false);
  }

  nlohmann::json output = nlohmann::json::parse(res->body);

  int d0 = output.at("distances").at("0").get<int>();
  int d1 = output.at("distances").at("1").get<int>();
  int d2 = output.at("distances").at("2").get<int>();
  int d3 = output.at("distances").at("3").get<int>();

  REQUIRE_EQUAL(d0, 0);
  REQUIRE_EQUAL(d1, 5);
  REQUIRE_EQUAL(d2, 1);
  REQUIRE_EQUAL(d3, 2);

  /* Проверяем предшественника вершины 3. */
  size_t pred3 = output.at("predecessors").at("3").get<size_t>();
  REQUIRE_EQUAL(pred3, static_cast<size_t>(2));
}

/**
 * @brief Статический тест с недостижимой вершиной.
 *
 * Граф: 0 -> 1, вершина 2 изолирована.
 *
 * @param cli Указатель на HTTP клиент.
 */
static void UnreachableTest(httplib::Client* cli) {
  nlohmann::json input;
  input["graph_type"] = "WeightedOrientedGraph";
  input["weight_type"] = "int";
  input["vertices"] = std::vector<int>{0, 1, 2};

  input["edges"][0]["from"] = 0;
  input["edges"][0]["to"] = 1;
  input["edges"][0]["weight"] = 3;

  input["source"] = 0;

  auto res = cli->Post("/Levit", input.dump(),
      "application/json");

  if (!res) {
    REQUIRE(false);
  }

  nlohmann::json output = nlohmann::json::parse(res->body);

  int d0 = output.at("distances").at("0").get<int>();
  int d1 = output.at("distances").at("1").get<int>();
  int d2 = output.at("distances").at("2").get<int>();

  REQUIRE_EQUAL(d0, 0);
  REQUIRE_EQUAL(d1, 3);

  /* Вершина 2 недостижима, расстояние должно быть INT_MAX. */
  REQUIRE_EQUAL(d2, std::numeric_limits<int>::max());
}

/**
 * @brief Случайный тест.
 *
 * @param cli Указатель на HTTP клиент.
 */
static void RandomTest(httplib::Client* cli) {
  RandomTestHelper(cli, "WeightedOrientedGraph");
  RandomTestHelper(cli, "WeightedGraph");
}

/**
 * @brief Случайный тест для указанного типа графа.
 *
 * @param cli Указатель на HTTP клиент.
 * @param graphType Тип графа.
 */
static void RandomTestHelper(httplib::Client* cli,
    const std::string& graphType) {
  /* Число попыток. */
  const int numTries = 50;

  /* Генератор случайных чисел. */
  std::mt19937 gen(42);
  /* Распределение для вершин графа. */
  std::uniform_int_distribution<size_t> vertexId(0, 30);
  /* Распределение для количества вершин графа. */
  std::uniform_int_distribution<size_t> verticesSize(2, 15);
  /* Распределение для количества рёбер графа. */
  std::uniform_int_distribution<size_t> edgesSize(1, 50);
  /* Распределение для весов рёбер. */
  std::uniform_int_distribution<int> weightDist(1, 100);

  for (int it = 0; it < numTries; it++) {
    size_t numVertices = verticesSize(gen);
    size_t numEdges = edgesSize(gen);

    std::vector<size_t> vertices;
    nlohmann::json input;

    input["graph_type"] = graphType;
    input["weight_type"] = "int";

    size_t numRealEdges = 0;
    /*
     * Словарь финальных весов рёбер.
     * Если одно и то же ребро добавляется несколько раз, то
     * в графе останется последний вес.
     */
    std::map<std::pair<size_t, size_t>, int> edgeWeights;

    for (size_t i = 0; i < numEdges; i++) {
      size_t id1 = vertexId(gen);
      size_t id2 = vertexId(gen);
      int w = weightDist(gen);

      if (id1 == id2)
        continue;

      vertices.push_back(id1);
      vertices.push_back(id2);

      input["edges"][numRealEdges]["from"] = id1;
      input["edges"][numRealEdges]["to"] = id2;
      input["edges"][numRealEdges]["weight"] = w;
      numRealEdges++;

      edgeWeights[{id1, id2}] = w;
      if (graphType == "WeightedGraph") {
        edgeWeights[{id2, id1}] = w;
      }
    }

    /* Добавляем несколько изолированных вершин. */
    for (size_t i = 0; i < numVertices; i++) {
      vertices.push_back(vertexId(gen));
    }

    /* Удаляем дубликаты. */
    std::sort(vertices.begin(), vertices.end());
    vertices.erase(std::unique(vertices.begin(), vertices.end()),
        vertices.end());

    input["vertices"] = vertices;
    input["source"] = vertices[0];

    auto res = cli->Post("/Levit", input.dump(),
        "application/json");

    if (!res) {
      REQUIRE(false);
    }

    nlohmann::json output = nlohmann::json::parse(res->body);

    /*
     * Проверяем неравенство треугольника: для каждого ребра (u, v)
     * с весом w должно выполняться d[v] <= d[u] + w.
     * Используем финальные веса из словаря edgeWeights.
     */
    auto distances = output.at("distances");
    const int inf = std::numeric_limits<int>::max();

    for (const auto& edge : edgeWeights) {
      size_t from = edge.first.first;
      size_t to = edge.first.second;
      int w = edge.second;

      int dFrom = distances.at(
          std::to_string(from)).get<int>();
      int dTo = distances.at(
          std::to_string(to)).get<int>();

      /* Пропускаем проверку, если вершина from недостижима. */
      if (dFrom != inf) {
        REQUIRE(dTo <= dFrom + w);
      }
    }

    /* Расстояние от стартовой вершины до себя равно 0. */
    int dSource = distances.at(
        std::to_string(vertices[0])).get<int>();
    REQUIRE_EQUAL(dSource, 0);
  }
}
