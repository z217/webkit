#include "json_server_client.h"

#include "channel/tcp_channel.h"
#include "webkit/logger.h"

using webkit::Status;
using webkit::StatusCode::WebkitCode;

JsonServerClient::JsonServerClient(webkit::ClientConfig *config)
    : config_(config) {}

Status JsonServerClient::Echo(const std::string &req, std::string &rsp) {
  webkit::TcpChannel channel;
  Status s = channel.Open([&](std::string &ip, uint16_t &port) {
    ip = config_->GetIp();
    port = config_->GetPort();
    return Status::OK();
  });
  if (!s.Ok()) {
    WEBKIT_LOGERROR("channel open error");
    return Status::Error(-1);
  }

  size_t write_size;
  s = channel.Write(req.data(), req.length(), write_size);
  if (!s.Ok()) {
    WEBKIT_LOGERROR("channel write error");
    return Status::Error(-1);
  }

  char buffer[128];
  size_t read_size;
  s = channel.Read(buffer, sizeof(buffer), read_size);
  if (read_size == 0 && !s.Ok()) {
    WEBKIT_LOGERROR("channel read error");
    return Status::Error(-1);
  }
  rsp.assign(buffer, read_size);

  return Status::OK();
}