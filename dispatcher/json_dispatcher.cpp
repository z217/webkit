#include "json_dispatcher.h"

#include "third_party/json/include/nlohmann/json.hpp"
#include "util/trace_helper.h"
#include "webkit/logger.h"

namespace webkit {
Status JsonDispatcher::Dispatch(Packet *packet) {
  Status s;
  size_t data_size = packet->GetRemainDataSize();
  std::unique_ptr<char[]> buffer_up(new char[data_size]);

  TraceHelper::GetInstance()->ClearTraceId();

  size_t read_size;
  s = packet->Read(buffer_up.get(), data_size, read_size);
  if (!s.Ok()) {
    WEBKIT_LOGERROR("packet read error status code %d %s", s.Code(),
                    s.Message());
    return Status::Error(StatusCode::eDisptachError, "packet read error");
  }

  std::string payload_str(buffer_up.get(), read_size);

  nlohmann::json payload_json =
      nlohmann::json::parse(payload_str, nullptr, false);
  if (payload_json.is_discarded()) {
    WEBKIT_LOGERROR("parse request json error");
    return Status::Error(StatusCode::eDisptachError, "packet parse json error");
  }
  const std::string &method_name = payload_json["method"];
  const std::string &trace_id = payload_json["trace_id"];
  TraceHelper::GetInstance()->SetTraceId(trace_id);
  const std::string &req_data = payload_json["data"].dump();

  WEBKIT_LOGDEBUG("recv req %s", payload_str);
  std::string rsp_data;
  s = Forward(method_name, req_data, rsp_data);
  if (!s.Ok()) {
    WEBKIT_LOGERROR("method %s forward error status code %d %s", method_name,
                    s.Code(), s.Message());
  }

  std::string rsp_json = BuildRsp(s, rsp_data);
  WEBKIT_LOGDEBUG("send rsp %s", rsp_json);

  size_t write_size;
  s = packet->Write(rsp_json.data(), rsp_json.length(), write_size);
  if (!s.Ok()) {
    WEBKIT_LOGERROR("packet write error status code %d %s", s.Code(),
                    s.Message());
    return Status::Error(StatusCode::eDisptachError,
                         "dispatch packet write error");
  }

  return Status::OK();
}

std::string JsonDispatcher::BuildRsp(const Status &status,
                                     const std::string &data) {
  nlohmann::json rsp;
  rsp["code"] = status.Code();
  rsp["message"] = status.Message();
  rsp["data"] = data;
  return rsp.dump();
}
}  // namespace webkit