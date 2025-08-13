/**
 * @file connectionManager.cpp
 * @brief Реализация методов connectionManager
 */

#include <string>
#include <iostream>
#include <algorithm>
#include "../include/net/connection/connectionManager.h"

connectionManager::connectionManager(int domain, int type, int protocol, std::unique_ptr<IMessageHandler> handler) : serverSocket_(domain, type, protocol), handler_(std::move(handler)) {}
connectionManager::~connectionManager()
{
  // Остановка потока accept
  if (thread_accept_.joinable())
    thread_accept_.join();

  // Остановка клиентских потоков
  for (auto &t : thread_connection_clients_)
  {
    if (t.joinable())
      t.join();
  }
}

void connectionManager::start(const std::string &ip, const int port)
{
  serverSocket_.universal_struct_parameters(ip, port);
  serverSocket_.bind_socket();
  serverSocket_.listen_socket(5);

  // Запуск потока для приема подключений
  thread_accept_ = std::thread(&connectionManager::acceptClients, this);
}
void connectionManager::acceptClients()
{
  while (true)
  {
    Socket client = serverSocket_.accept_socket(NULL, NULL);

    auto client_ptr = std::make_shared<Socket>(std::move(client));
    {
      std::lock_guard<std::mutex> lock(clientsMutex_);
      active_clients_.push_back(client_ptr);
    }
    auto worker_ = std::thread(&connectionManager::handleClient, this, client_ptr);
    thread_connection_clients_.emplace_back(std::move(worker_));
  }
}

void connectionManager::handleClient(std::shared_ptr<Socket> client)
{
  client->send("Welcome to chat! Type 'exit' to quit.\n");

  while (true)
  {
    std::string msg;
    ssize_t len = client->recv(msg);

    if (len <= 0)
      break;

    std::cout << "Received: " << msg << std::endl;

    if (!handler_->handle(client, msg))
    {
      std::cout << "Отсутствует метод для обработки сообщения\n";
    }
  }

  cleanupDisconnectedClients(client);
  client->close_socket();
}

void connectionManager::cleanupDisconnectedClients(std::shared_ptr<Socket> client)
{
  std::lock_guard<std::mutex> lock(clientsMutex_);
  active_clients_.erase(
      std::remove(active_clients_.begin(), active_clients_.end(), client),
      active_clients_.end());
}