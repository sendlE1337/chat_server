/**
 * @file IConnectionManager.h
 * @brief Базовый интерфейс для менеджера подключений
 * @ingroup ServerCode
 */
#pragma once
#include <string>

/**
 * @class IConnectionManager
 * @brief Абстрактный класс для управления подключениями.
 *
 * @details Обязанности:
 *  - Управление жизненным циклом подключений
 *  - Запуск сервера на указанных данных
 * @note Все реализации должны гарантировать потокобезопасность
 * @see IConnectionManager - пример реализации
 */
class IConnectionManager
{
public:
  /**
   * @brief Запуск сервера
   *
   * @param ip IP-адрес сервера
   * @param port Порт сервера
   */
  virtual void start(const std::string &ip, const int port) = 0;

  /**
   * @brief Виртуальный деструктор
   * @note Гарантирует корректное удаление производных классов
   */

  virtual ~IConnectionManager() = default;
};