#include "../include/net/socket.h"
#include <utility>

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
  memset(&other.servaddr, 0, sizeof(other.servaddr));
  memset(&other.servaddr6, 0, sizeof(other.servaddr6));
}
Socket &Socket::operator=(Socket &&other) noexcept
{
  if (this != &other)
  {
    if (fd_ != -1)
    {
      close(fd_);
    }

    fd_ = std::exchange(other.fd_, -1);
    servaddr = other.servaddr;
    servaddr6 = other.servaddr6;
    config_ = std::move(other.config_);

    memset(&other.servaddr, 0, sizeof(other.servaddr));
    memset(&other.servaddr6, 0, sizeof(other.servaddr6));
  }
  return *this;
}

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
// size_t Socket::send(const void *data, size_t size) {}
// size_t Socket::recv(void *buffer, size_t size) {}

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

ssize_t Socket::recv(std::string &buffer)
{
  if (fd_ == -1)
  {
    errno = EBADF;
    return -1;
  }

  char buf[1024];
  ssize_t bytes = ::recv(fd_, buf, sizeof(buf), 0);

  if (bytes > 0)
  {
    buffer.clear(); // Очищаем буфер перед записью
    buffer.assign(buf, bytes);
  }
  // bytes == 0 — соединение закрыто, errno не трогаем
  // bytes < 0 — ошибка, errno уже установлен системой
  return bytes;
}

ssize_t Socket::send(const std::string &message)
{
  if (fd_ == -1)
  {
    errno = EBADF;
    return -1;
  }

  ssize_t sent = ::send(fd_, message.data(), message.size(), MSG_NOSIGNAL);

  if (sent == -1)
  {
    if (errno == EAGAIN || errno == EWOULDBLOCK)
    {
      errno = EAGAIN; // Явно указываем, что отправка заблокирована
    }
  }
  return sent;
}

void Socket::shutdown()
{

  if (fd_ != -1)
  {
    ::shutdown(fd_, SHUT_RDWR);
    close(fd_);
    fd_ = -1;
  }
}