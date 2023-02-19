#pragma once

#include <string>

#include "webkit/dispatcher.h"

namespace webkit {
class JsonDispatcher : public Dispatcher {
 public:
  JsonDispatcher() = default;

  virtual ~JsonDispatcher() = default;

  Status Dispatch(Packet *packet) override;

  virtual Status Forward(const std::string &method_name, const std::string &req,
                         std::string &rsp) = 0;

 private:
  std::string BuildRsp(const Status &status, const std::string &data);
};
}  // namespace webkit