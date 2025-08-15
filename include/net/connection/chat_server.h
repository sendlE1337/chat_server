/**
 * @file chat_server.h
 * @brief Фасад для запуска сервера
 * @ingroup ServerCode
 */
#pragma once
#include "IConnectionManager.h"
#include <memory>

/**
 * @class ChatServer
 * @brief Основной класс для управления жизненным циклом сервера
 *
 * @details Реализует фасадный класс, предоставляя удобный API для:
 *  - Инициализации сервера
 *  - Запуска на указанном адресе
 *
 * @note Переносит владение connectionManager
 * @see IConnectionManager для расширения функциональности.
 */
class ChatServer
{
public:
  /**
   * @brief Конструктор с передачей владения
   * @param manager smart-pointer на менеджер подключений
   */
  ChatServer(std::unique_ptr<IConnectionManager> manager);

  /**
   * @brief Запуск сервера на указанном интерфейсе
   * @param ip IP-адрес для вашего сервера
   * @param port Порт сервера
   */
  void start(const std::string &ip, const int port);

  /**
   * @brief Остановка сервера
   */
  void stop();

private:
  std::unique_ptr<IConnectionManager> manager_; ///< Владелец менеджера подключений
};