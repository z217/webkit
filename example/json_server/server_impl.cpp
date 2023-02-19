#include "server_impl.h"

webkit::Status ServerImpl::Echo(const std::string &req, std::string &rsp) {
  rsp = req;
  return webkit::Status::OK();
}