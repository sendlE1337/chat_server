/**
 * @file connectionManager.h
 * @brief Реализация менеджера подключений (thread-per-connection)
 * @ingroup ServerCore
 */

#pragma once
#include <vector>
#include <thread>
#include <mutex>
#include <memory>
#include "../include/net/socket.h"
#include "IConnectionManager.h"
#include "../include/handler/imessage_handler.h"

/**
 * @class connectionManager
 * @brief Реализация менеджера подключений с пулом потоков
 *
 * @details Особенности:
 * - Принимает новые подключения в отдельном потоке
 * - Для каждого клиента создает отдельный поток обработки
 * - Использует Chain of Responsibility для обработки сообщений
 *
 * @warning Деструктор останавливает все рабочие потоки
 * @threadsafe Все публичные методы потокобезопасны
 */
class connectionManager : public IConnectionManager
{
public:
  /// Тип контейнера для хранения клиентов
  using ClientsContainer = std::vector<std::shared_ptr<Socket>>;

  /**
   * @brief Конструктор
   * @param domain Домен (AF_INET/AF_INET6)
   * @param type Тип сокета (SOCK_STREAM/SOCK_DGRAM)
   * @param protocol Протокол (0 для авто)
   * @param handler Обработчик сообщений (передача владения)
   */
  connectionManager(int domain, int type, int protocol, std::unique_ptr<IMessageHandler> handler);

  /**
   * @brief Деструктор
   * @note Останавливает все потоки и закрывает соединения
   */
  ~connectionManager();
  void start(const std::string &ip, const int port);

  /**
   * @brief Получить список активных клиентов
   * @return Ссылка на контейнер клиентов
   * @warning Не потокобезопасно! Используйте get_clients_mutex()
   */
  ClientsContainer &get_clients() { return active_clients_; }

  /**
   * @brief Получить мьютекс для работы с клиентами
   * @return Ссылка на мьютекс
   */
  std::mutex &get_clients_mutex() { return clientsMutex_; }

  /**
   * @brief Получить обработчик приведенный к типу T
   * @tparam T Целевой тип обработчика
   * @return Указатель на обработчик или nullptr
   */
  template <typename T>
  T *get_handler_as()
  {
    return dynamic_cast<T *>(handler_.get());
  }

  // Константная версия get_handler_as
  template <typename T>
  const T *get_handler_as() const
  {
    return dynamic_cast<T *>(handler_.get());
  }

private:
  Socket serverSocket_;                                ///< Основной серверный сокет
  std::thread thread_accept_;                          ///< Поток для приема подключений
  std::unique_ptr<IMessageHandler> handler_;           ///< Обработчик сообщений
  std::vector<std::thread> thread_connection_clients_; ///< Потоки клиентов
  std::mutex clientsMutex_;                            ///< Мьютекс для доступа к клиентам
  ClientsContainer active_clients_;                    ///< Активные подключения

  /**
   * @brief Цикл принятия новых подключений
   * @note Работает в отдельном потоке (thread_accept_)
   */
  void acceptClients();

  /**
   * @brief Обработка клиентского подключения
   * @param client Умный указатель на клиентский сокет
   * @note Работает в отдельном потоке для каждого клиента
   */
  void handleClient(std::shared_ptr<Socket> client);

  /**
   * @brief Удаление отключенного клиента
   * @param client Сокет отключенного клиента
   * @threadsafe Использует clientsMutex_
   */
  void cleanupDisconnectedClients(std::shared_ptr<Socket> client);
};