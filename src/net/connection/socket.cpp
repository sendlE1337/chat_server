#include "../include/net/socket.h"
#include <utility>

/**
 * @throws runtime_error В случае ошибок конфигурации или неудачи при создании сокета
 */
Socket::Socket(int domain, int type, int protocol) : config_(domain, type, protocol)
{
  if (!config_.isValid())
  {
    throw std::runtime_error("Invalid socket configuration");
  }
  fd_ = socket(domain, type, protocol);
  int yes = 1;
  if (setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)))
  {
    close(fd_);
    throw std::runtime_error("setsockopt(SO_REUSEADDR) failed");
  }
}

/**
 * @throws runtime_error Если дескриптор неверен
 */
Socket::Socket(int fd) : fd_(fd)
{
  if (fd_ == -1)
    throw std::runtime_error("Invalid socket descriptor");
}
Socket::~Socket()
{
  if (fd_ != -1)
    close(fd_);
}

Socket::Socket(Socket &&other) noexcept
    : fd_(std::exchange(other.fd_, -1)), servaddr(other.servaddr), servaddr6(other.servaddr6), config_(std::move(other.config_))
{
  memset(&other.servaddr, 0, sizeof(other.servaddr));   // Обнуляем старое содержимое
  memset(&other.servaddr6, 0, sizeof(other.servaddr6)); // Обнуляем старое содержимое
}
Socket &Socket::operator=(Socket &&other) noexcept
{
  if (this != &other)
  {
    if (fd_ != -1)
    {
      close(fd_); // Закрываем старый сокет
    }

    fd_ = std::exchange(other.fd_, -1); // Меняем дескрипторы
    servaddr = other.servaddr;          // Копируем адрес IPv4
    servaddr6 = other.servaddr6;        // Копируем адрес IPv6
    config_ = std::move(other.config_); // Перемещаем конфигурацию

    memset(&other.servaddr, 0, sizeof(other.servaddr));   // Обнуляем старое содержимое
    memset(&other.servaddr6, 0, sizeof(other.servaddr6)); // Обнуляем старое содержимое
  }
  return *this;
}

/**
 * @throws runtime_error Если сокет не действителен или операция bind завершилась ошибкой
 */
void Socket::bind_socket()
{
  if (!is_valid())
  {
    throw std::runtime_error("Socket is not valid");
  }
  if (config_.domain_ == AF_INET)
  {
    if (bind(fd_, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
      close(fd_);
      throw std::runtime_error(std::string("bind failed") + strerror(errno));
    }
  }

  if (config_.domain_ == AF_INET6)
  {

    if (bind(fd_, (struct sockaddr *)&servaddr6, sizeof(servaddr6)) < 0)
    {
      close(fd_);
      throw std::runtime_error(std::string("bind failed") + strerror(errno));
    }
  }
}

/**
 * @throws runtime_error Если сокет не действителен или произошла ошибка слушания
 */
void Socket::listen_socket(int backlog)
{
  if (!is_valid())
  {
    throw std::runtime_error("Socket is not valid");
  }

  if (config_.type_ != SOCK_STREAM)
  {
    throw std::runtime_error("Listen only for SOCK_STREAM");
  }

  if (listen(fd_, backlog) < 0)
  {
    close(fd_);
    throw std::runtime_error(std::string("listen failed: ") + strerror(errno));
  }
}

/**
 * @throws runtime_error Если сокет не действителен или произошло отклонение соединения
 */
Socket Socket::accept_socket(struct sockaddr *addr, socklen_t *addrlen)
{
  if (!is_valid())
  {
    throw std::runtime_error("Socket is not valid");
  }

  if (config_.type_ != SOCK_STREAM)
  {
    throw std::runtime_error("Accept only for SOCK_STREAM");
  }

  int connfd = accept(fd_, addr, addrlen);
  if (connfd < 0)
  {
    throw std::runtime_error(std::string("accept failed: ") + strerror(errno));
  }

  return Socket(connfd);
}

/**
 * @throws runtime_error Если сокет не действителен или попытка подключения завершилась ошибкой
 */
void Socket::connect_socket()
{
  if (!is_valid())
  {
    throw std::runtime_error("Socket is not valid");
  }

  if (config_.domain_ == AF_INET)
  {
    if (connect(fd_, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1)
    {
      close(fd_);
      throw std::runtime_error(std::string("IPv4 connect failed: ") + strerror(errno));
    }
  }
  else if (config_.domain_ == AF_INET6)
  {
    if (connect(fd_, (struct sockaddr *)&servaddr6, sizeof(servaddr6)) == -1)
    {
      close(fd_);
      throw std::runtime_error(std::string("IPv6 connect failed: ") + strerror(errno));
    }
  }
  else
  {
    throw std::runtime_error("Unsupported address family");
  }
}

/**
 * @throws runtime_error Если указанный адрес некорректен или поддерживается неподдерживаемое семейство адресов
 */
void Socket::universal_struct_parameters(const std::string address, int port)
{
  if (!is_valid())
  {
    throw std::runtime_error("Socket is not valid");
  }

  switch (config_.domain_)
  {
  case AF_INET:
  {
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);

    if (inet_pton(AF_INET, address.c_str(), &servaddr.sin_addr) != 1)
    {
      throw std::runtime_error("Invalid IPv4 address: " + address);
    }
    break;
  }
  case AF_INET6:
  {
    memset(&servaddr6, 0, sizeof(servaddr6));
    servaddr6.sin6_family = AF_INET6;
    servaddr6.sin6_port = htons(port);

    if (inet_pton(AF_INET6, address.c_str(), &servaddr6.sin6_addr) != 1)
    {
      throw std::runtime_error("Invalid IPv6 address: " + address);
    }
    break;
  }
  default:
    throw std::runtime_error("Unsupported address family in config");
  }
}

/**
 * @return Количество полученных байт или -1 в случае ошибки
 */
ssize_t Socket::recv(std::string &buffer)
{
  if (fd_ == -1)
  {
    errno = EBADF;
    return -1;
  }

  char buf[1024];                                   // Временный буфер для приёма данных
  ssize_t bytes = ::recv(fd_, buf, sizeof(buf), 0); // Получаем данные

  if (bytes > 0)
  {
    buffer.clear();            // Очистка буфера перед записью новых данных
    buffer.assign(buf, bytes); // Добавляем полученные данные в буфер
  }
  // bytes == 0 — соединение закрыто, errno не трогаем
  // bytes < 0 — ошибка, errno уже установлен системой
  return bytes;
}

/**
 * @return Количество отправленных байт или -1 в случае ошибки
 */
ssize_t Socket::send(const std::string &message)
{
  if (fd_ == -1)
  {
    errno = EBADF;
    return -1;
  }

  ssize_t sent = ::send(fd_, message.data(), message.size(), MSG_NOSIGNAL); // Отправляем данные

  if (sent == -1)
  {
    if (errno == EAGAIN || errno == EWOULDBLOCK)
    {
      errno = EAGAIN; // Явно устанавливаем ошибку блокировки
    }
  }
  return sent;
}

void Socket::shutdown()
{

  if (fd_ != -1)
  {
    ::shutdown(fd_, SHUT_RDWR); // Прекращаем ввод-вывод
    close(fd_);                 // Закрываем сокет
    fd_ = -1;                   // Отмечаем сокет как закрытый
  }
}