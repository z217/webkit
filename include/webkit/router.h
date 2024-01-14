#pragma once

#include "webkit/status.h"

namespace webkit {
class Router {
 public:
  Router() = default;

  virtual ~Router() = default;

  virtual Status Route(std::string &ip, uint16_t &port) = 0;
};
}