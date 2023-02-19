#pragma once

#include "webkit/socket.h"
#include "webkit/status.h"

namespace webkit {
class TcpSocket : public Socket {
 public:
  TcpSocket();

  TcpSocket(int fd, const std::string &ip, uint16_t port);

  ~TcpSocket();

  Status Connect(const std::string &ip, uint16_t port);

  Status Listen(const std::string &ip, uint16_t port);

  Status Accept(TcpSocket *socket);

  Status Write(const void *data, size_t data_size, size_t &write_size) override;

  Status Read(void *buffer, size_t buffer_size, size_t &read_size) override;

  Status Close() override;

  Status SetNonBlock();

  Status SetTimeout(suseconds_t timeout_sec = 2, suseconds_t timeout_us = 0);

  int GetFd() override;

  bool IsConnected() const;

  const std::string &GetIp() const;

  uint16_t GetPort() const;

 private:
  Status SetLinger(bool is_on, int linger_time);

  int fd_;
  std::string ip_;
  uint16_t port_;
  bool is_connected_;
};
}  // namespace webkit