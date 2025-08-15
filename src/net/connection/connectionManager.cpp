/**
 * @file connectionManager.cpp
 * @brief Реализация методов connectionManager
 */

#include <string>
#include <iostream>
#include <algorithm>
#include "../include/net/connection/connectionManager.h"

connectionManager::connectionManager(int domain, int type, int protocol, std::unique_ptr<IMessageHandler> handler) : serverSocket_(domain, type, protocol), handler_(std::move(handler)), running_(true) {}
connectionManager::~connectionManager()
{
  // Остановка
  stop();
}

void connectionManager::start(const std::string &ip, const int port)
{
  try
  {
    serverSocket_.universal_struct_parameters(ip, port);
    serverSocket_.bind_socket();
    serverSocket_.listen_socket(5);

    // Запуск потока для приема подключений
    thread_accept_ = std::thread(&connectionManager::acceptClients, this);
  }
  catch (...)
  {
    stop();
    throw;
  }
}
void connectionManager::stop()
{
  if (!running_.exchange(false))
    return;
  running_ = false;

  if (thread_accept_.joinable())
  {
    serverSocket_.shutdown();
    thread_accept_.join();
  }

  {
    std::lock_guard<std::mutex> lock(clientsMutex_);
    for (auto &client : active_clients_)
    {
      client->shutdown();
    }
    active_clients_.clear();
  }

  for (auto &thread_client : thread_connection_clients_)
  {
    if (thread_client.joinable())
    {
      thread_client.join();
    }
  }
  thread_connection_clients_.clear();
}
void connectionManager::acceptClients()
{
  while (running_)
  {
    try
    {
      Socket client = serverSocket_.accept_socket(NULL, NULL);

      if (!running_)
      {
        client.close_socket();
        break;
      }

      auto client_ptr = std::make_shared<Socket>(std::move(client));
      {
        std::lock_guard<std::mutex> lock(clientsMutex_);
        active_clients_.push_back(client_ptr);
      }
      auto worker_ = std::thread(&connectionManager::handleClient, this, client_ptr);
      thread_connection_clients_.emplace_back(std::move(worker_));
    }
    catch (std::system_error &e)
    {
      if (running_)
        std::cerr << "Accept failed: " << e.what() << '\n';
      break;
    }
    catch (...)
    {
      if (running_)
        std::cerr << "Unknown accept failed\n";
      break;
    }
  }
}

void connectionManager::handleClient(std::shared_ptr<Socket> client)
{
  try
  {
    const std::string exit_cmd = "/quit";
    client->send("Welcome to chat! Type '" + exit_cmd + "' to disconnect.\n");

    while (running_)
    {
      std::string msg;
      ssize_t len = client->recv(msg);

      msg.erase(std::remove(msg.begin(), msg.end(), '\r'), msg.end());
      while (!msg.empty() && msg.back() == '\n')
      {
        msg.pop_back();
      }

      if (len <= 0 || msg.empty())
      {
        continue;
      }

      std::cout << "Received: " << msg << std::endl;

      if (msg == exit_cmd)
      {
        client->send("Goodbye! Disconnecting...\n");
        break;
      }
      std::string response = msg + "\n";
      if (!handler_->handle(client, response))
      {
        std::cout << "No handler for message\n";
      }
    }
  }
  catch (std::exception &e)
  {
    std::cerr << "Client error: " << e.what() << '\n';
  }
  catch (...)
  {
    std::cerr << "Client handler error!";
  }

  cleanupDisconnectedClients(client);
}

void connectionManager::cleanupDisconnectedClients(std::shared_ptr<Socket> client)
{
  client->shutdown();
  std::lock_guard<std::mutex> lock(clientsMutex_);
  active_clients_.erase(
      std::remove(active_clients_.begin(), active_clients_.end(), client),
      active_clients_.end());
}