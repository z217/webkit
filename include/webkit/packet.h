#pragma once

#include <iostream>
#include <memory>

#include "webkit/class_factory.h"
#include "webkit/io_base.h"
#include "webkit/status.h"

namespace webkit {
class Packet : public IoBase, public std::iostream {
 public:
  Packet(std::streambuf *p_buf) : std::iostream(p_buf) {}

  virtual ~Packet() = default;

  using IoBase::Write;

  using IoBase::Read;

  virtual void Clear() = 0;

  virtual size_t GetDataSize() const = 0;
};

using PacketFactory = ClassFactory<Packet>;
}  // namespace webkit