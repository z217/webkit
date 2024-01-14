#pragma once

#include "webkit/io_base.h"
#include "webkit/status.h"

namespace webkit {
class Socket : public IoBase {
 public:
  Socket() = default;

  virtual ~Socket() = default;

  using IoBase::Write;

  using IoBase::Read;

  virtual Status Close() = 0;

  virtual int GetFd() const = 0;
};
}  // namespace webkit