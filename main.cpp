#include "include/net/connection/connectionManager.h"
#include "include/net/connection/chat_server.h"
#include "include/handler/broadcast_handler.h"
#include "include/handler/chained_handler.h"
#include <iostream>

int main()
{
  auto chain = std::make_unique<ChainedHandler>();

  auto manager = std::make_unique<connectionManager>(AF_INET, SOCK_STREAM, 0, std::move(chain));

  if (auto *chain_handler = manager->get_handler_as<ChainedHandler>())
  {
    chain_handler->add(std::make_unique<BroadcastHandler>(manager->get_clients(), manager->get_clients_mutex()));
  }

  ChatServer server(std::move(manager));
  server.start("0.0.0.0", 8080);
  return 0;
}