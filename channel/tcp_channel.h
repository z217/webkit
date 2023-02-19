#pragma once

#include <functional>

#include "socket/tcp_socket.h"
#include "webkit/channel.h"
#include "webkit/client_config.h"

namespace webkit {
class TcpChannel : public Channel {
 public:
  TcpChannel() = default;

  TcpChannel(const ClientConfig *config);

  virtual ~TcpChannel() = default;

  Status Open(std::function<Status(std::string &, uint16_t &)> route_func);

  Status Write(const void *data, size_t data_size, size_t &write_size) override;

  Status Read(void *buffer, size_t data_size, size_t &read_size) override;

 protected:
  const ClientConfig *config_;
  TcpSocket tcp_socket_;
};
}  // namespace webkit