#include <iostream>
#include <memory>
#include <csignal>
#include <atomic>

#include "include/net/connection/connectionManager.h"
#include "include/net/connection/chat_server.h"
#include "include/handler/Messages/implementations/broadcast_handler.h"
#include "include/handler/Messages/chain/chained_handler.h"

std::atomic<bool> g_running(true);

void signal_handler(int signal)
{
  std::cout << "\nReceived signal " << signal << ", shutting down... \n";
  g_running = false;
}

int main()
{
  std::signal(SIGINT, signal_handler);
  std::signal(SIGTERM, signal_handler);

  try
  {
    auto chain = std::make_unique<ChainedHandler>();

    auto manager = std::make_unique<connectionManager>(AF_INET, SOCK_STREAM, 0, std::move(chain));

    if (auto *chain_ptr = manager->get_handler_as<ChainedHandler>())
    {
      chain_ptr->add(std::make_unique<BroadcastHandler>(
          manager->get_clients(),
          manager->get_clients_mutex()));
    }

    ChatServer server(std::move(manager));

    server.start("0.0.0.0", 8080);

    std::cout << "Server started on 0.0.0.0:8080. Press Ctrl+C to stop...\n";

    while (g_running)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    std::cout << "Server stopped greacefully\n";
    return 0;
  }
  catch (std::exception &e)
  {
    std::cerr << e.what();
  }
  catch (...)
  {
    std::cerr << "Unknown error";
  }
}