#include "tcp_channel.h"

#include "webkit/logger.h"

namespace webkit {
TcpChannel::TcpChannel(const ClientConfig *config)
    : config_(config), packet_sp_(nullptr) {
  packet_sp_ = PacketFactory::GetDefaultInstance()->Build();
  packet_sp_->Expand(128);
}

Status TcpChannel::Open(
    std::function<Status(std::string &, uint16_t &)> route_func) {
  Status s;
  std::string ip;
  uint16_t port;
  s = route_func(ip, port);
  if (!s.Ok()) {
    WEBKIT_LOGERROR("tcp channel open failed %d %s", s.Code(), s.Message());
    return Status::Error(StatusCode::eChannelRouteError, "channel route error");
  }

  s = tcp_socket_.Connect(ip, port);
  if (!s.Ok()) {
    WEBKIT_LOGERROR("tcp channel socket connect failed %d %s", s.Code(),
                    s.Message());
    return Status::Error(StatusCode::eChannelOpenError,
                         "channel connect error");
  }

  s = tcp_socket_.SetTimeout(config_->GetSockTimeoutSec());
  if (!s.Ok()) {
    WEBKIT_LOGERROR("tcp channel socket set timeout failed %d %s", s.Code(),
                    s.Message());
    return Status::Error(StatusCode::eChannelOpenError,
                         "channel set timeout error");
  }

  return Status::OK();
}

Status TcpChannel::Write(Serializer &serializer) {
  Status s;
  s = serializer.SerializeTo(*packet_sp_);
  if (!s.Ok()) {
    WEBKIT_LOGERROR("serialize to error code %d message %s", s.Code(),
                    s.Message());
    return Status::Error(StatusCode::eChannelWriteError, "channel write error");
  }
  s = packet_sp_->Write(tcp_socket_);
  if (s.Code() == StatusCode::eRetry) return s;
  if (!s.Ok()) {
    WEBKIT_LOGERROR("packet write tcp socket error code %d message %s",
                    s.Code(), s.Message());
    return Status::Error(StatusCode::eChannelWriteError, "channel write error");
  }
  return s;
}

Status TcpChannel::Read(Parser &parser) {
  Status s;
  s = packet_sp_->Read(tcp_socket_);
  if (s.Code() == StatusCode::eRetry) return s;
  if (!s.Ok()) {
    WEBKIT_LOGERROR("packet read tcp socket error code %d message %s", s.Code(),
                    s.Message());
    return Status::Error(StatusCode::eChannelReadError, "channel read error");
  }
  s = parser.ParseFrom(*packet_sp_);
  if (s.Code() == StatusCode::eRetry) return s;
  if (!s.Ok()) {
    WEBKIT_LOGERROR("packet read tcp socket error code %d message %s", s.Code(),
                    s.Message());
    return Status::Error(StatusCode::eChannelReadError, "channel read error");
  }
  return Status::OK();
}
}  // namespace webkit