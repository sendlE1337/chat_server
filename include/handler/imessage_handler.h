#pragma once
#include "./net/socket.h"
#include <memory>
// Chain of Responsibility
class IMessageHandler
{
public:
  virtual bool handle(std::shared_ptr<Socket> sender, const std::string &msg) = 0;
  virtual ~IMessageHandler() = default;
};
