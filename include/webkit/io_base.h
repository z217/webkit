#pragma once

#include "webkit/status.h"

namespace webkit {
class IoBase {
 public:
  virtual Status Write(const void *src, size_t src_size,
                       size_t &write_size) = 0;

  virtual Status Read(void *dst, size_t dst_size, size_t &read_size) = 0;
};
}  // namespace webkit