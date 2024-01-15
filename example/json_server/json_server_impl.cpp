#include "json_server_impl.h"

webkit::Status JsonServerImpl::Echo(const nlohmann::json &req,
                                    nlohmann::json &rsp) {
  rsp = req;
  return webkit::Status::OK();
}