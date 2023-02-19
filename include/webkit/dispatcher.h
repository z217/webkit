#pragma once

#include "webkit/class_factory.h"
#include "webkit/packet.h"
#include "webkit/status.h"

namespace webkit {
class Dispatcher {
 public:
  Dispatcher() = default;

  virtual ~Dispatcher() = default;

  virtual Status Dispatch(Packet *packet) = 0;
};

using DispatcherFactory = ClassFactory<Dispatcher>;
}  // namespace webkit