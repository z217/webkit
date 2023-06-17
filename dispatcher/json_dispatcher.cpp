#include "json_dispatcher.h"

#include "packet/string_serialization.h"
#include "third_party/json/include/nlohmann/json.hpp"
#include "util/trace_helper.h"
#include "webkit/logger.h"

#define JSON_MUST_GET(JSON, FIELD, VALUE)                                      \
  do {                                                                         \
    try {                                                                      \
      (VALUE) = (JSON)[(FIELD)];                                               \
    } catch (nlohmann::json::type_error &) {                                   \
      WEBKIT_LOGERROR("req json get %s type error", FIELD);                    \
      return Status::Error(StatusCode::eJsonTypeError, "req json type error"); \
    } catch (nlohmann::json::parse_error &) {                                  \
      WEBKIT_LOGERROR("req json get %s parse error", FIELD);                   \
      return Status::Error(StatusCode::eJsonParseError,                        \
                           "req json parse error");                            \
    }                                                                          \
  } while (false)

namespace webkit {
Status JsonDispatcher::Dispatch(Packet *packet) {
  TraceHelper::GetInstance()->ClearTraceId();

  Status s;
  std::string req;
  StringParser req_parser(req);
  s = req_parser.ParseFrom(*packet);
  if (s.Code() == StatusCode::eRetry) return s;
  if (!s.Ok()) {
    WEBKIT_LOGERROR(
        "string parser parse from packet error status code %d message %s",
        s.Code(), s.Message());
    return Status::Error(StatusCode::eDisptachError, "parser parse from error");
  }

  nlohmann::json req_json = nlohmann::json::parse(req, nullptr, false);
  if (req_json.is_discarded()) {
    WEBKIT_LOGERROR("parse request json error");
    return Status::Error(StatusCode::eDisptachError, "packet parse json error");
  }
  WEBKIT_LOGDEBUG("recv req %s", req_json.dump());
  std::string method_name;
  nlohmann::json data_json;
  JSON_MUST_GET(req_json, "method", method_name);
  JSON_MUST_GET(req_json, "data", data_json);
  std::string req_data = data_json.dump();

  std::string rsp_data;
  s = Forward(method_name, req_data, rsp_data);
  if (!s.Ok()) {
    WEBKIT_LOGERROR("method %s forward error status code %d %s", method_name,
                    s.Code(), s.Message());
  }

  std::string rsp_json_str = BuildRsp(s, rsp_data);
  WEBKIT_LOGDEBUG("send rsp %s", rsp_json_str);

  StringSerializer rsp_serializer(rsp_json_str);
  s = rsp_serializer.SerializeTo(*packet);
  if (!s.Ok()) {
    WEBKIT_LOGERROR(
        "string serializer serialize to error status code %d message %s",
        s.Code(), s.Message());
    return Status::Error(StatusCode::eDisptachError,
                         "serializer serialize to error");
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