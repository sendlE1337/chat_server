/**
 * @file chat_server.cpp
 * @brief Реализация методов ChatServer
 */

#include "../include/net/connection/chat_server.h"

ChatServer::ChatServer(std::unique_ptr<IConnectionManager> manager) : manager_(std::move(manager)) {}

void ChatServer::start(const std::string &ip, const int port)
{
  manager_->start(ip, port);
}