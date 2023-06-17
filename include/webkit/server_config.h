#pragma once

#include <string>

namespace webkit {
class ServerConfig {
 public:
  ServerConfig()
      : ip_(""),
        port_(0),
        log_path_(""),
        sock_timeout_sec_(2),
        epoller_max_event_(2000),
        epoller_timeout_ms_(5),
        is_epoll_event_et_(true),
        io_thread_num_(4),
        worker_thread_num_(8),
        worker_queue_size_(2000),
        max_connection_(2000),
        is_deamon_(false) {}

  virtual ~ServerConfig() = default;

  void SetIp(const std::string &ip) { ip_ = ip; }
  const std::string &GetIp() const { return ip_; }

  void SetPort(uint16_t port) { port_ = port; }
  uint16_t GetPort() const { return port_; }

  void SetLogPath(const std::string &log_path) { log_path_ = log_path; }
  const std::string &GetLogPath() const { return log_path_; }

  void SetSockTimeoutSec(int sock_timeout_sec) {
    sock_timeout_sec_ = sock_timeout_sec;
  }
  int GetSockTimeoutSec() const { return sock_timeout_sec_; }

  void SetEpollerMaxEvent(int epoller_max_event) {
    epoller_max_event_ = epoller_max_event;
  }
  int GetEpollerMaxEvent() const { return epoller_max_event_; }

  void SetEpollEventEt(bool is_epoll_event_et) {
    is_epoll_event_et_ = is_epoll_event_et;
  }
  bool IsEpollEventEt() const { return is_epoll_event_et_; }

  void SetEpollerTimeoutMs(int epoller_timeout_ms) {
    epoller_timeout_ms_ = epoller_timeout_ms;
  }
  int GetEpollerTimeoutMs() const { return epoller_timeout_ms_; }

  void SetIoThreadNum(uint32_t io_thread_num) {
    io_thread_num_ = io_thread_num;
  }
  uint32_t GetIoThreadNum() const { return io_thread_num_; }

  void SetWorkerThreadNum(uint32_t worker_thread_num) {
    worker_thread_num_ = worker_thread_num;
  }
  uint32_t GetWorkerThreadNum() const { return worker_thread_num_; }

  void SetWorkerQueueSize(uint32_t worker_queue_size) {
    worker_queue_size_ = worker_queue_size;
  }
  uint32_t GetWorkerQueueSize() const { return worker_queue_size_; }

  void SetMaxConnection(uint32_t max_connection) {
    max_connection_ = max_connection;
  }
  uint32_t GetMaxConnection() const { return max_connection_; }

  void SetDeamon(bool is_deamon) { is_deamon_ = is_deamon; }
  bool IsDeamon() const { return is_deamon_; }

 protected:
  std::string ip_;
  uint16_t port_;
  std::string log_path_;
  int sock_timeout_sec_;
  int epoller_max_event_;
  int epoller_timeout_ms_;
  bool is_epoll_event_et_;
  uint32_t io_thread_num_;
  uint32_t worker_thread_num_;
  uint32_t worker_queue_size_;
  uint32_t max_connection_;
  bool is_deamon_;
};
}  // namespace webkit