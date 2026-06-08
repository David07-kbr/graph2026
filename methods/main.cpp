/**
 * @file methods/main.cpp
 * @author Mikhail Lozhnikov
 *
 * Файл с функией main() для серверной части программы.
 */

#include <httplib.h>
#include <iostream>
#include <cstdio>
#include <nlohmann/json.hpp>
#include "methods.hpp"



int main(int argc, char* argv[]) {
  // Порт по-умолчанию.
  int port = 8080;

  if (argc >= 2) {
    // Меняем порт по умолчанию, если предоставлен соответствующий
    // аргумент командной строки.
    if (std::sscanf(argv[1], "%d", &port) != 1)
      return -1;
  }

  std::cerr << "Listening on port " << port << "..." << std::endl;

  httplib::Server svr;

  // Обработчик для GET запроса по адресу /stop. Этот обработчик
  // останавливает сервер.
  svr.Get("/stop", [&](const httplib::Request&, httplib::Response&) {
    svr.stop();
  });

  /* Сюда нужно вставить обработчик post запроса для алгоритма. */

svr.Post("/levit", [](const httplib::Request& req, httplib::Response& res) {
    // 1. Читаем JSON, который прислал клиент
    auto input_json = nlohmann::json::parse(req.body);

    // 2. Вызываем вашу функцию алгоритма Левита
    auto output_json = graph::levit_algorithm_method(input_json);

    // 3. Отправляем ответ клиенту
    res.set_content(output_json.dump(), "application/json");
});

  /* Конец вставки. */

  // Эта функция запускает сервер на указанном порту. Программа не завершится
  // до тех пор, пока сервер не будет остановлен.
  svr.listen("0.0.0.0", port);

  return 0;
}
