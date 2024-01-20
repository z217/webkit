#include "tcp_channel.h"

#include "webkit/logger.h"
#include "webkit/protocol_adapter.h"

namespace webkit {
TcpChannel::TcpChannel(const ClientConfig *config)
    : config_(config), packet_sp_(nullptr) {
  packet_sp_ = PacketFactory::GetDefaultInstance()->Build();
}

Status TcpChannel::Open(Router &router) {
  std::string ip;
  uint16_t port;
  Status s = router.Route(ip, port);
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
  std::shared_ptr<ProtocolAdapter> adapter_sp =
      ProtocolAdapterFactory::GetDefaultInstance()->Build(
          *packet_sp_, tcp_socket_, packet_sp_->GetDataSize());
  s = adapter_sp->AdaptTo();
  if (s.Code() == StatusCode::eRetry) return s;
  if (!s.Ok()) {
    WEBKIT_LOGERROR("packet write tcp socket error code %d message %s",
                    s.Code(), s.Message());
    return Status::Error(StatusCode::eChannelWriteError, "channel write error");
  }
  return s;
}

Status TcpChannel::Read(Parser &parser) {
  std::shared_ptr<ProtocolAdapter> adapter_sp =
      ProtocolAdapterFactory::GetDefaultInstance()->Build(*packet_sp_,
                                                          tcp_socket_, 0);
  Status s = adapter_sp->AdaptFrom();
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