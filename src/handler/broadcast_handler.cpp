/**
 * @file broadcast_handler.cpp
 * @brief Реализация методов BroadcastHandler
 */
#include "../include/handler/broadcast_handler.h"
#include <iostream>

BroadcastHandler::BroadcastHandler(ClientContainer &clients, std::mutex &mutex)
    : clients_(clients), mutex_(mutex) {}

bool BroadcastHandler::handle(std::shared_ptr<Socket> sender, const std::string &msg)
{
  std::lock_guard<std::mutex> lock(mutex_);

  for (auto &client : clients_)
  {
    if (sender != client)
    {
      if (client->send(msg) < 0)
      {
        std::cerr << "Error sending to client (continuing with others)\n";
      }
    }
  }
  return true;
}