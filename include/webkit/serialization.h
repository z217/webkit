#pragma once

#include "webkit/packet.h"
#include "webkit/status.h"

namespace webkit {
class Serializer {
 public:
  virtual Status SerializeTo(Packet &packet) = 0;
};

class Parser {
 public:
  virtual Status ParseFrom(Packet &packet) = 0;
};
}  // namespace webkit