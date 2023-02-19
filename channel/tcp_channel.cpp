#include "tcp_channel.h"

#include "webkit/logger.h"

namespace webkit {
TcpChannel::TcpChannel(const ClientConfig *config) : config_(config) {}

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

Status TcpChannel::Write(const void *data, size_t data_size,
                         size_t &write_size) {
  Status s;
  write_size = 0;
  while (write_size < data_size) {
    size_t size = 0;
    s = tcp_socket_.Write(
        reinterpret_cast<const std::byte *>(data) + write_size,
        data_size - write_size, size);
    if (!s.Ok() && s.Code() != StatusCode::eSocketPeerClosed) {
      WEBKIT_LOGERROR("tcp socket write error %d %s", s.Code(), s.Message());
      return Status::Error(StatusCode::eChannelWriteError,
                           "channel write error");
    }
    write_size += size;
    if (write_size > data_size) {
      WEBKIT_LOGFATAL(
          "tcp socket write may have bug, data size is less than write size");
      return Status::Error(StatusCode::eChannelWriteError,
                           "channel write may have bug");
    }
    if (s.Code() == StatusCode::eSocketPeerClosed) break;
  }
  return Status::OK();
}

Status TcpChannel::Read(void *buffer, size_t data_size, size_t &read_size) {
  Status s;
  read_size = 0;
  while (read_size < data_size) {
    size_t size;
    s = tcp_socket_.Read(reinterpret_cast<std::byte *>(buffer) + read_size,
                         data_size - read_size, size);
    if (s.Code() == StatusCode::eNoData) break;
    if (!s.Ok() && s.Code() != StatusCode::eSocketPeerClosed) {
      WEBKIT_LOGERROR("tcp socket read error %d %s", s.Code(), s.Message());
      return Status::Error(StatusCode::eChannelReadError, "channel read error");
    }
    read_size += size;
    if (read_size > data_size) {
      WEBKIT_LOGFATAL(
          "tcp socket read may have bug, data size is less than read size");
      return Status::Error(StatusCode::eChannelReadError,
                           "channel read may have bug");
    }
    if (s.Code() == StatusCode::eSocketPeerClosed) break;
  }
  return Status::OK();
}
}  // namespace webkit