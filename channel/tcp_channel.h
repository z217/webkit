#pragma once

#include <functional>

#include "socket/tcp_socket.h"
#include "webkit/channel.h"
#include "webkit/client_config.h"
#include "webkit/router.h"

namespace webkit {
class TcpChannel : public Channel {
 public:
  TcpChannel(const ClientConfig *config);

  virtual ~TcpChannel() = default;

  Status Open(Router &router);

  Status Write(Serializer &serializer) override;

  Status Read(Parser &parser) override;

 protected:
  const ClientConfig *config_;
  TcpSocket tcp_socket_;
  std::shared_ptr<Packet> packet_sp_;
};
}  // namespace webkit