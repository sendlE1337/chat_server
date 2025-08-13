/**
 * @file chained_handler.h
 * @brief Реализация паттерна "Цепочка обязанностей" для обработчиков сообщений
 * @ingroup Handlers
 */

#pragma once
#include "imessage_handler.h"
#include <memory>
#include <vector>

/**
 * @class ChainedHandler
 * @brief Контейнер для последовательного выполнения обработчиков сообщений
 *
 * @details Реализует паттерн Chain of Responsibility, позволяя последовательно
 * применять различные обработчики к входящим сообщениям до тех пор, пока один
 * из обработчиков не обработает сообщение.
 *
 * @note Потокобезопасность зависит от используемых обработчиков
 */
class ChainedHandler : public IMessageHandler
{
private:
  std::vector<std::unique_ptr<IMessageHandler>> handlers_;

public:
  /**
   * @brief Добавить обработчик в цепочку
   * @param handler Уникальный указатель на обработчик
   * @post Обработчик становится частью цепочки вызовов
   * @note Владелец обработчика передается классу ChainedHandler
   */
  void add(std::unique_ptr<IMessageHandler> handler);

  /**
   * @brief Обработать сообщение
   * @param sender Сокет-отправитель сообщения
   * @param msg Текст сообщения
   * @return true если сообщение было обработано хотя бы одним обработчиком
   * @return false если ни один обработчик не принял сообщение
   *
   * @details Метод последовательно вызывает handle() у всех обработчиков
   * до первого успешного выполнения. Порядок вызова соответствует порядку добавления.
   */
  bool handle(std::shared_ptr<Socket> sender, const std::string &msg) override;
};