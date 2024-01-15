#pragma once

#include <string>

#include "webkit/dispatcher.h"

namespace webkit {
class StringDispatcher : public Dispatcher {
 public:
  StringDispatcher() = default;

  virtual ~StringDispatcher() = default;

  Status Dispatch(Packet *p_packet) override;

  virtual Status Forward(uint32_t method_id, const std::string &req,
                         std::string &rsp) = 0;
};
}  // namespace webkit