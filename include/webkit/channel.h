#pragma once

#include "webkit/serialization.h"
#include "webkit/status.h"

namespace webkit {
class Channel {
 public:
  Channel() = default;

  virtual ~Channel() = default;

  virtual Status Write(Serializer &serializer) = 0;

  virtual Status Read(Parser &parser) = 0;
};
}  // namespace webkit