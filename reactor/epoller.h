#pragma once

#include <sys/epoll.h>

#include <memory>
#include <vector>

#include "webkit/reactor.h"
#include "webkit/server_config.h"
#include "webkit/status.h"

namespace webkit {
class EpollEvent;

class Epoller : public Reactor, public std::enable_shared_from_this<Epoller> {
 public:
  Epoller() = default;
  Epoller(int max_event, int timeout_ms = 0);

  ~Epoller() = default;

  std::shared_ptr<Event> CreateEvent(
      std::shared_ptr<Socket> socket_sp,
      std::shared_ptr<Packet> packet_sp) override;

  Status Init(int flags = 0);

  Status Add(EpollEvent *event);

  Status Delete(EpollEvent *event);

  Status Modify(EpollEvent *event);

  Status Wait(std::vector<Event *> &event_vec) override;

  void SetMaxEvent(int max_event);

  void SetTimeoutMs(int timeout_ms);

 private:
  int epoll_fd_;
  int max_event_;
  int timeout_ms_;
};

class EpollerFactory : public ReactorFactory {
 public:
  EpollerFactory(ServerConfig *config);

  ~EpollerFactory() = default;

  std::shared_ptr<Reactor> Build() override;

 private:
  ServerConfig *config_;
};
}  // namespace webkit