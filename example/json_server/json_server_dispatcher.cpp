#include "json_server_dispatcher.h"

#include "constant.h"
#include "webkit/logger.h"

using webkit::Dispatcher;
using webkit::Status;
using webkit::StatusCode::WebkitCode;

Status JsonServerDispatcher::Forward(uint32_t method_id, const std::string &req,
                                     std::string &rsp) {
  nlohmann::json req_json = nlohmann::json::parse(req, nullptr, false);
  if (req_json.is_discarded()) {
    WEBKIT_LOGERROR("parse req error");
    return Status::Error(WebkitCode::eJsonParseError, "parse request error");
  }

  Status s;
  nlohmann::json rsp_data;
  switch (method_id) {
    case eMethodIdEcho:
      s = server_impl_.Echo(req_json, rsp_data);
      break;

    default:
      WEBKIT_LOGERROR("unknown method id %u", method_id);
      return Status::Error(WebkitCode::eParamError, "unknown method id");
  }
  if (!s.Ok()) {
    WEBKIT_LOGERROR("method id %u call error status code %d message %s",
                    method_id, s.Code(), s.Message());
  }
  nlohmann::json rsp_json;
  rsp_json["code"] = s.Code();
  rsp_json["message"] = s.Message();
  rsp_json["data"] = rsp_data;
  rsp = rsp_json.dump(-1, ' ', false, nlohmann::json::error_handler_t::ignore);
  return Status::OK();
}

std::shared_ptr<Dispatcher> JsonServerDispatcherFactory::Build() {
  return std::make_shared<JsonServerDispatcher>();
}