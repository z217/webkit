#pragma once

#include "webkit/status.h"

namespace webkit {
class Socket {
 public:
  Socket() = default;

  virtual ~Socket() = default;

  virtual Status Write(const void *data, size_t data_size,
                       size_t &write_size) = 0;

  virtual Status Read(void *buffer, size_t data_size, size_t &read_size) = 0;

  virtual Status Close() = 0;

  virtual int GetFd() = 0;
};
}  // namespace webkit