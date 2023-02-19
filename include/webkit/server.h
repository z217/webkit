#pragma once

#include "webkit/status.h"

namespace webkit {
class Server {
 public:
  Server() = default;

  virtual ~Server() = default;

  virtual Status Run() = 0;
};
}  // namespace webkit