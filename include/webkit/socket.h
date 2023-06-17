#pragma once

#include "webkit/io_base.h"
#include "webkit/status.h"

namespace webkit {
class Socket : public IoBase {
 public:
  Socket() = default;

  virtual ~Socket() = default;

  virtual Status Close() = 0;

  virtual int GetFd() = 0;
};
}  // namespace webkit