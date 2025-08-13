#pragma once
#include <vector>
#include <thread>
#include <mutex>
#include <memory>
#include "../include/net/socket.h"
#include "IConnectionManager.h"
#include "../include/handler/imessage_handler.h"

class connectionManager : public IConnectionManager
{
public:
  using ClientsContainer = std::vector<std::shared_ptr<Socket>>;

  connectionManager(int domain, int type, int protocol, std::unique_ptr<IMessageHandler> handler);
  ~connectionManager();
  void start(const std::string &ip, const int port);
  ClientsContainer &get_clients() { return active_clients_; }
  std::mutex &get_clients_mutex() { return clientsMutex_; }
  template <typename T>
  T *get_handler_as()
  {
    return dynamic_cast<T *>(handler_.get());
  }
  template <typename T>
  const T *get_handler_as() const
  {
    return dynamic_cast<T *>(handler_.get());
  }

private:
  Socket serverSocket_;
  std::thread thread_accept_;
  std::unique_ptr<IMessageHandler> handler_;
  std::vector<std::thread> thread_connection_clients_;
  std::mutex clientsMutex_;
  ClientsContainer active_clients_;

  void acceptClients();
  void handleClient(std::shared_ptr<Socket> client);
  void cleanupDisconnectedClients(std::shared_ptr<Socket> client);
};