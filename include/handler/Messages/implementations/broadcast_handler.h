/**
 * @file broadcast_handler.h
 * @brief Обработчик широковещательной рассылки сообщений
 * @ingroup Handlers
 */

#pragma once
#include <mutex>
#include "../include/handler/Messages/interface/imessage_handler.h"
#include "../include/net/connection/connectionManager.h"
/**
 * @class BroadcastHandler
 * @brief Реализует рассылку сообщений всем подключенным клиентам, кроме отправителя
 *
 * @details Класс обеспечивает:
 * - Потокобезопасную рассылку сообщений
 * - Фильтрацию отправителя
 * - Интеграцию с паттерном Chain of Responsibility
 *
 * @note Для работы требует внешний thread-safe контейнер клиентов
 * @warning Клиенты должны жить дольше экземпляра BroadcastHandler
 * @see IMessageHandler для базового интерфейса
 */
class BroadcastHandler : public IMessageHandler
{
public:
  /// Тип контейнера для хранения клиентов
  using ClientContainer = std::vector<std::shared_ptr<Socket>>;

  /**
   * @brief Конструктор обработчика
   * @param clients Ссылка на контейнер клиентов (должен быть thread-safe)
   * @param mutex Мьютекс для синхронизации доступа к клиентам
   * @pre Контейнер clients должен быть валидным
   * @pre Мьютекс mutex должен защищать доступ к clients
   */
  explicit BroadcastHandler(ClientContainer &clients, std::mutex &mutex);

  /**
   * @brief Обработка входящего сообщения
   * @param sender Сокет-отправитель сообщения
   * @param msg Текст сообщения для рассылки
   * @return Всегда возвращает true (сообщение считается обработанным)
   *
   * @details Алгоритм работы:
   * 1. Блокирует мьютекс для безопасного доступа к клиентам
   * 2. Рассылает сообщение всем клиентам кроме отправителя
   * 3. Игнорирует ошибки отправки отдельным клиентам
   * 4. Разблокирует мьютекс при выходе
   *
   * @threadsafe Гарантируется потокобезопасность при использовании общего мьютекса
   */
  bool handle(std::shared_ptr<Socket> sender, const std::string &msg) override;

private:
  ClientContainer &clients_; ///< Ссылка на контейнер клиентов
  std::mutex &mutex_;        ///< Ссылка на мьютекс для синхронизации
};