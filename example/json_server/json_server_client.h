#pragma once

#include "webkit/client_config.h"
#include "webkit/status.h"

class JsonServerClient {
 public:
  JsonServerClient(webkit::ClientConfig *config);

  ~JsonServerClient() = default;

  webkit::Status Echo(const std::string &req, std::string &rsp);

 private:
  webkit::ClientConfig *config_;
};