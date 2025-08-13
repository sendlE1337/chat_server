#pragma once
#include "IConnectionManager.h"
#include <memory>

class ChatServer
{
public:
  ChatServer(std::unique_ptr<IConnectionManager> manager);
  void start(const std::string &ip, const int port);

private:
  std::unique_ptr<IConnectionManager> manager_;
};