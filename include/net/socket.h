/**
 * @file socket.h
 * @brief RAII-обертка для сокетов с поддержкой IPv4/IPv6
 */

#pragma once

#include <string.h>
#include <system_error>
#include <cstdint>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include "socketConfig.h"

/**
 * @class Socket
 * @brief Потокобезопасная обертка для системны вызовов
 *
 * @warning Не поддерживает копирование (только move-семантика)
 * @note Все методы безопасны для вызова из разных потоков
 */
class Socket
{
private:
  int fd_ = -1;                ///< Дескриптор сокета (-1 если невалиден)
  struct sockaddr_in servaddr; ///< Структура для хранения адреса сервера (IPv4)

  struct sockaddr_in6 servaddr6; ///< Структура для хранения адреса сервера (IPv6)
  SocketConfig config_;          ///< < Конфигурационные настройки сокета

public:
  /**
   * @brief Конструктор  для инициализации нового сокета
   *
   * @param domain Семейство протоколов(AF_INET для IPv4 и AF_INET6 для IPv6)
   * @param type Тип сокета (SOCK_STREAM для TCP, SOCK_DGRAM для UDP)
   * @param protocol Протокол передачи данных(Обычно 0)
   */

  Socket(int domain, int type, int protocol);

  /**
   * @brief Альтернативный конструктор принимающий существующий файловый дескриптор
   * @param fd Уже созданный файловый дескриптор
   */
  explicit Socket(int fd);

  /// @brief Деструктор закрывающий сокет(освобождает ресурсы)
  ~Socket();

  /// Запрещаем копирование объектов класса
  Socket(const Socket &) = delete;
  Socket &operator=(const Socket &) = delete;

  /**
   * @brief Перемещающий конструктор, позволяющий перемещать объект.
   *
   * @warning После перемещения объект становится недействительным.
   *
   * @param other Другой объект сокета
   */
  Socket(Socket &&other) noexcept;

  /**
   * @brief Move-assignment
   *
   * Позволяет присвоить один объект другому, сохраняя прежний ресурс.
   *
   * @param other Передаваемый объект сокета
   * @return Ссылка на текущий объект
   */
  Socket &operator=(Socket &&other) noexcept;

  /**
   *@brief Привязывает сокет к определенному адресу и порту
   * Метод вызывает системный вызов bind(), привязывая сокет к указанному адресу и порту.
   */
  void bind_socket();

  /**
   * @brief Переводит сокет в режим прослушивания входящих соединений.
   *
   * @param backlog Максимальное количество ожидающий подключений, которое сервер может принять (по умолчанию 5)
   * @warning backlog не должен превышать 1024
   */
  void listen_socket(int backlog = 5);

  /**
   * @brief Принимает входящее соединение и возвращает новый объект сокета для общения с клиентом.
   *
   * Данный метод создает новый сокет для конкретного соединения и сохраняет связь с удаленным хостом.
   *
   * @param addr Адрес клиента (по умолчанию NULL)
   * @param addrlen Длина структуры адреса клиента
   * @return Новый объект сокета, представляющий принятое подключение
   */
  Socket accept_socket(struct sockaddr *addr = NULL, socklen_t *addrlen = NULL);

  /**
   * @brief Осуществляет подключенние к удаленному серверу по заданному адресу и порту.
   */
  void connect_socket();

  /**
   * @brief Отпраляет сообщение по установленному соединению.
   *
   * @param message Сообшение для отправки
   * @return Кол-во отправленных байтов
   */
  ssize_t send(const std::string &message);

  /**
   * @brief Получает данные из установленного соединения.
   *
   * Читает поступающие данные из сокета и помещает их в буфер
   *
   * @param buffer Буфер для записи полученных данных
   * @return Кол-во принятых байт
   */
  ssize_t recv(std::string &buffer);

  /**
   * @brief  Получить текущий файловый дескриптор.
   *
   * @return Файловый дескриптор.
   */
  int fd() const noexcept { return fd_; }

  /**
   * @brief Явное закрытие сокета и дальнейшее освобождение связанного ресурса.
   */
  void close_socket() noexcept
  {
    if (fd_ != -1)
    {
      close(fd_);
      fd_ = -1;
    }
  };

  /**
   * @brief Проверка валидности дескриптора
   *
   * @return true/false в зависимости от состояния.
   */
  bool is_valid() const noexcept { return fd_ != -1; }

  /**
   * @brief Завершение работы с сокетом на стороне ввода/вывода.
   *
   * Этот метод позволяет завершить чтение, запись или обе операции.
   */
  void shutdown();

  /**
   * @brief Универсальная настройка параметров структуру sockaddr
   *
   * Настраивает внутреннюю структуру sockaddr, используемую сокетом.
   *
   * @param address Строка с IP-адресом(например: "127.0.0.1")
   * @param port Номер порта.
   */
  void universal_struct_parameters(const std::string address, int port);
};
