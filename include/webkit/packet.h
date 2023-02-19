#pragma once

#include <memory>

#include "webkit/class_factory.h"
#include "webkit/status.h"

namespace webkit {
class Packet {
 public:
  Packet() = default;

  virtual ~Packet() = default;

  virtual Status Write(const void *data, size_t data_size,
                       size_t &write_size) = 0;

  virtual Status Read(void *buffer, size_t buffer_size, size_t &read_size) = 0;

  virtual void Clear() = 0;

  virtual size_t GetRemainDataSize() const = 0;
};

using PacketFactory = ClassFactory<Packet>;
}  // namespace webkit