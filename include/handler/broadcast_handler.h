#pragma once
#include <mutex>
#include "imessage_handler.h"
#include "./net/connection/connectionManager.h"

class BroadcastHandler : public IMessageHandler
{
public:
  using ClientContainer = std::vector<std::shared_ptr<Socket>>;

  explicit BroadcastHandler(ClientContainer &clients, std::mutex &mutex);
  bool handle(std::shared_ptr<Socket> sender, const std::string &msg) override;

private:
  ClientContainer &clients_;
  std::mutex &mutex_;
};