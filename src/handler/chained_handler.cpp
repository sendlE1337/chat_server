/**
 * @file chained_handler.cpp
 * @brief Реализация методов ChainedHandler
 */

#include "../include/handler/chained_handler.h"

void ChainedHandler::add(std::unique_ptr<IMessageHandler> handler)
{
  handlers_.emplace_back(std::move(handler));
}

bool ChainedHandler::handle(std::shared_ptr<Socket> sender, const std::string &msg)
{
  for (auto &handler : handlers_)
  {
    if (handler->handle(sender, msg))
    {
      return true;
    }
  }
  return false;
}