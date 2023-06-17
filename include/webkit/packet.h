#pragma once

#include <memory>

#include "webkit/class_factory.h"
#include "webkit/io_base.h"
#include "webkit/status.h"

namespace webkit {
class Packet : public IoBase {
 public:
  Packet() = default;

  virtual ~Packet() = default;

  using IoBase::Write;

  virtual Status Write(IoBase &other) = 0;

  using IoBase::Read;

  virtual Status Read(IoBase &other) = 0;

  virtual void Clear() = 0;

  virtual size_t GetRemainDataSize() const = 0;

  virtual Status Expand(size_t expand_size) = 0;
};

using PacketFactory = ClassFactory<Packet>;
}  // namespace webkit