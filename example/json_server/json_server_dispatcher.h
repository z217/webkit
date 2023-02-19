#pragma once

#include "dispatcher/json_dispatcher.h"
#include "server_impl.h"

class JsonServerDispatcher : public webkit::JsonDispatcher {
 public:
  JsonServerDispatcher() = default;

  ~JsonServerDispatcher() = default;

  webkit::Status Forward(const std::string &method_name, const std::string &req,
                         std::string &rsp) override;

 private:
  ServerImpl server_impl_;
};

class JsonServerDispatcherFactory : public webkit::DispatcherFactory {
 public:
  JsonServerDispatcherFactory() = default;

  ~JsonServerDispatcherFactory() = default;

  std::shared_ptr<webkit::Dispatcher> Build() override;
};