/**
 * @struct SocketConfig
 * @brief Конфигурация сокета(домен, тип, протокол)
 * @invariant Должен проходить проверку isValid()
 */

#pragma once
#include <sys/socket.h>

struct SocketConfig
{
  int domain_;
  int type_;
  int protocol_;

  // Правильный порядок инициализации
  SocketConfig(int domain = AF_INET, int type = SOCK_STREAM, int protocol = 0)
      : domain_(domain), type_(type), protocol_(protocol) {}

  /**
   * @brief Проверка валидности конфигурации
   * @return true если domain AF_INET/AF_INET6 и type SOCK_STREAM/SOCK_DGRAM
   */
  bool isValid() const
  {
    // Проверяем все возможные комбинации
    bool validDomain = (domain_ == AF_INET) || (domain_ == AF_INET6);
    bool validType = (type_ == SOCK_STREAM) || (type_ == SOCK_DGRAM);

    return validDomain && validType;
  }
};