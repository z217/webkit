#include "string_dispatcher.h"

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
Status StringDispatcher::Dispatch(Packet *packet) {
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

  const MetaInfo &meta_info = req_parser.GetMetaInfo();
  TraceHelper::GetInstance()->SetTraceId(meta_info.trace_id);
  WEBKIT_LOGDEBUG("dispatch request method id %u", meta_info.method_id);

  std::string rsp;
  s = Forward(meta_info.method_id, req, rsp);
  if (!s.Ok()) {
    WEBKIT_LOGERROR(
        "method id %u request forward error status code %d message %s",
        meta_info.method_id, s.Code(), s.Message());
    return Status::Error(StatusCode::eDisptachError, "forward request error");
  }

  StringSerializer rsp_serializer(meta_info.method_id, rsp);
  s = rsp_serializer.SerializeTo(*packet);
  if (!s.Ok()) {
    WEBKIT_LOGERROR(
        "method id %u string serializer serialze to error status code %d "
        "message %s",
        meta_info.method_id, s.Code(), s.Message());
    return Status::Error(StatusCode::eDisptachError,
                         "serializer serialize to error");
  }

  return Status::OK();
}
}  // namespace webkit