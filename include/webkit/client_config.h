#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace webkit {
class ClientConfig {
 public:
  struct Host {
    std::string ip;
    uint16_t port;
  };

  ClientConfig() : sock_timeout_sec_(2), sock_timeout_usec_(0) {}

  virtual ~ClientConfig() = default;

  virtual void AddHost(Host host) { host_vec_.push_back(host); }
  virtual const std::vector<Host> &GetHostVec() { return host_vec_; }

  virtual void SetSockTimeoutSec(int sock_timeout_sec) {
    sock_timeout_sec_ = sock_timeout_sec;
  }
  virtual int GetSockTimeoutSec() const { return sock_timeout_sec_; }

  virtual void SetSockTimeoutUsec(int sock_timeout_usec) {
    sock_timeout_usec_ = sock_timeout_usec;
  }
  virtual int GetSockTimeoutUsec() const { return sock_timeout_usec_; }

 protected:
  std::vector<Host> host_vec_;
  int sock_timeout_sec_;
  int sock_timeout_usec_;
};
}  // namespace webkit