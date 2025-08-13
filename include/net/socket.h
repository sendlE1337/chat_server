#pragma once
#ifndef SOCKET_H
#define SOCKET_H

#include <string.h>
#include <system_error>
#include <cstdint>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include "socketConfig.h"

class Socket
{
private:
  int fd_ = -1;
  struct sockaddr_in servaddr;
  struct sockaddr_in6 servaddr6;
  SocketConfig config_;

public:
  Socket(int domain, int type, int protocol);
  explicit Socket(int fd);
  ~Socket();

  Socket(const Socket &) = delete;
  Socket &operator=(const Socket &) = delete;

  Socket(Socket &&other) noexcept;
  Socket &operator=(Socket &&other) noexcept;
  void bind_socket();
  void listen_socket(int backlog = 5);
  Socket accept_socket(struct sockaddr *addr = NULL, socklen_t *addrlen = NULL);
  void connect_socket();
  ssize_t send(const std::string &message);
  ssize_t recv(std::string &buffer);

  int fd() const noexcept { return fd_; }
  void close_socket() noexcept
  {
    if (fd_ != -1)
    {
      close(fd_);
      fd_ = -1;
    }
  };
  bool is_valid() const noexcept { return fd_ != -1; }
  void shutdown();
  void universal_struct_parameters(const std::string address, int port);
};

#endif