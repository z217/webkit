#pragma once

#include <vector>

#include "webkit/class_factory.h"
#include "webkit/event.h"
#include "webkit/socket.h"
#include "webkit/status.h"

namespace webkit {
class Reactor {
 public:
  Reactor() = default;

  virtual ~Reactor() = default;

  virtual std::shared_ptr<Event> CreateEvent(
      std::shared_ptr<Socket> socket_sp, std::shared_ptr<Packet> packet_sp) = 0;

  virtual Status Add(Event *event) = 0;

  virtual Status Delete(Event *event) = 0;

  virtual Status Modify(Event *event) = 0;

  virtual Status Wait(std::vector<Event *> &event_vec) = 0;
};

using ReactorFactory = ClassFactory<Reactor>;
}  // namespace webkit