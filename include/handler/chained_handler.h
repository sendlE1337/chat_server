#pragma once
#include "imessage_handler.h"
#include <memory>
#include <vector>

class ChainedHandler : public IMessageHandler
{
private:
  std::vector<std::unique_ptr<IMessageHandler>> handlers_;

public:
  void add(std::unique_ptr<IMessageHandler> handler);
  bool handle(std::shared_ptr<Socket> sender, const std::string &msg) override;
};