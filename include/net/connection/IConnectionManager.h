#pragma once
#include <string>

class IConnectionManager
{
public:
  virtual void start(const std::string &ip, const int port) = 0;
  virtual ~IConnectionManager() = default;
};