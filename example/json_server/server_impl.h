#pragma once

#include "webkit/status.h"

class ServerImpl {
 public:
  ServerImpl() = default;

  ~ServerImpl() = default;

  webkit::Status Echo(const std::string &req, std::string &rsp);
};