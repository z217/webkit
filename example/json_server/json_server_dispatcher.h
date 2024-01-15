#pragma once

#include "dispatcher/string_dispatcher.h"
#include "json_server_impl.h"

class JsonServerDispatcher : public webkit::StringDispatcher {
 public:
  JsonServerDispatcher() = default;

  ~JsonServerDispatcher() = default;

  webkit::Status Forward(uint32_t method_id, const std::string &req,
                         std::string &rsp) override;

 private:
  JsonServerImpl server_impl_;
};

class JsonServerDispatcherFactory : public webkit::DispatcherFactory {
 public:
  JsonServerDispatcherFactory() = default;

  ~JsonServerDispatcherFactory() = default;

  std::shared_ptr<webkit::Dispatcher> Build() override;
};