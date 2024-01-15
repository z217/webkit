#pragma once

#include "third_party/json/include/nlohmann/json.hpp"
#include "webkit/status.h"

class JsonServerImpl {
 public:
  JsonServerImpl() = default;

  ~JsonServerImpl() = default;

  webkit::Status Echo(const nlohmann::json &req, nlohmann::json &rsp);
};