#include "json_server_client.h"

#include "channel/hash_router.h"
#include "channel/tcp_channel.h"
#include "constant.h"
#include "dispatcher/string_serialization.h"
#include "util/generator.h"
#include "util/trace_helper.h"
#include "webkit/logger.h"

using webkit::Status;
using webkit::StatusCode::WebkitCode;

JsonServerClient::JsonServerClient(webkit::ClientConfig *config)
    : config_(config) {}

Status JsonServerClient::Echo(const std::string &req, std::string &rsp) {
  std::string trace_id = webkit::UidGenerator::GetInstance()->Generate();
  webkit::TraceHelper::GetInstance()->SetTraceId(trace_id);

  webkit::TcpChannel channel(config_);
  webkit::HashRouter<std::string> router(config_, trace_id);
  Status s = channel.Open(router);
  if (!s.Ok()) {
    WEBKIT_LOGERROR("channel open error");
    return Status::Error(-1);
  }

  static constexpr int kRetryCount = 3;
  webkit::StringSerializer req_serializer(eMethodIdEcho, req);
  for (int i = 0; i < kRetryCount; i++) {
    s = channel.Write(req_serializer);
    if (s.Code() != webkit::StatusCode::eRetry) break;
  }
  if (!s.Ok()) {
    WEBKIT_LOGERROR("channel write error");
    return Status::Error(-1);
  }

  webkit::StringParser rsp_parser(rsp);
  for (int i = 0; i < kRetryCount; i++) {
    s = channel.Read(rsp_parser);
    if (s.Code() != webkit::StatusCode::eRetry) break;
  }
  if (!s.Ok()) {
    WEBKIT_LOGERROR("channel read error");
    return Status::Error(-1);
  }
  if (rsp_parser.GetMetaInfo().method_id != eMethodIdEcho) {
    WEBKIT_LOGERROR("method id error");
    return Status::Error(-1);
  }

  return Status::OK();
}