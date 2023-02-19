#include "json_server_dispatcher.h"

using webkit::Dispatcher;
using webkit::Status;
using webkit::StatusCode::WebkitCode;

webkit::Status JsonServerDispatcher::Forward(const std::string &method_name,
                                             const std::string &req,
                                             std::string &rsp) {
  if (method_name == "echo") return server_impl_.Echo(req, rsp);
  return Status::ErrorF(WebkitCode::eParamError, "unknown method name %s",
                        method_name);
}

std::shared_ptr<Dispatcher> JsonServerDispatcherFactory::Build() {
  return std::make_shared<JsonServerDispatcher>();
}