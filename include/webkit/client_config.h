#pragma once

#include <string>

namespace webkit {
class ClientConfig {
 public:
  ClientConfig() : ip_(""), port_(0), sock_timeout_sec_(2) {}

  virtual ~ClientConfig() = default;

  void SetIp(const std::string &ip) { ip_ = ip; }
  const std::string &GetIp() const { return ip_; }

  void SetPort(uint16_t port) { port_ = port; }
  uint16_t GetPort() const { return port_; }

  void SetSockTimeoutSec(int sock_timeout_sec) {
    sock_timeout_sec_ = sock_timeout_sec;
  }
  int GetSockTimeoutSec() const { return sock_timeout_sec_; }

 protected:
  std::string ip_;
  uint16_t port_;
  int sock_timeout_sec_;
};
}  // namespace webkit