#include "epoller.h"

#include <fcntl.h>

#include <thread>

#include "socket/tcp_socket.h"
#include "webkit/logger.h"

namespace webkit {
Epoller::Epoller(int max_event, int timeout_ms)
    : max_event_(max_event), timeout_ms_(timeout_ms) {}

Status Epoller::Init(int flags) {
  epoll_fd_ = epoll_create1(flags);
  if (epoll_fd_ < 0) {
    WEBKIT_LOGFATAL("epoller init error %d %s", errno, strerror(errno));
    return Status::Fatal(StatusCode::eEpollInitError, "epoller init error");
  }
  return Status::OK();
}

std::shared_ptr<Event> Epoller::CreateEvent(std::shared_ptr<Socket> socket_sp,
                                            std::shared_ptr<Packet> packet_sp) {
  auto event_sp = std::make_shared<EpollEvent>(socket_sp, packet_sp);
  return event_sp;
}

Status Epoller::Add(Event *event) {
  return Add(static_cast<EpollEvent *>(event));
}

Status Epoller::Add(EpollEvent *event) {
  int ret = epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, event->GetSocket()->GetFd(),
                      event->GetEpollEvent());
  if (ret != 0) {
    WEBKIT_LOGERROR("epoller add event error %d %s", errno, strerror(errno));
    return Status::Error(StatusCode::eEpollAddError, "epoller add error");
  }
  return Status::OK();
}

Status Epoller::Delete(Event *event) {
  return Delete(static_cast<EpollEvent *>(event));
}

Status Epoller::Delete(EpollEvent *event) {
  int ret = epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, event->GetSocket()->GetFd(),
                      event->GetEpollEvent());
  if (ret != 0) {
    WEBKIT_LOGERROR("epoller remove event error %d %s", errno, strerror(errno));
    return Status::Error(StatusCode::eEpollRemoveError, "epoller remove error");
  }
  return Status::OK();
}

Status Epoller::Modify(Event *event) {
  return Modify(static_cast<EpollEvent *>(event));
}

Status Epoller::Modify(EpollEvent *event) {
  int ret = epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, event->GetSocket()->GetFd(),
                      event->GetEpollEvent());
  if (ret != 0) {
    WEBKIT_LOGERROR("epoller modify event error %d %s", errno, strerror(errno));
    return Status::Error(StatusCode::eEpollModifyError, "epoller modify error");
  }
  return Status::OK();
}

Status Epoller::Wait(std::vector<Event *> &event_vec) {
  static thread_local std::vector<struct epoll_event> event_buffer;
  if (event_buffer.size() < max_event_) {
    event_buffer.resize(max_event_);
  }
  int nevent = epoll_wait(epoll_fd_, &event_buffer[0], max_event_, timeout_ms_);
  if (nevent < 0) {
    WEBKIT_LOGERROR("epoller wait event error %d %s", errno, strerror(errno));
    return Status::Error(StatusCode::eEpollWaitError, "epoller wait error");
  }
  if (nevent == 0) return Status::Warn(StatusCode::eRetry);
  for (int i = 0; i < nevent; i++) {
    event_vec.push_back(reinterpret_cast<Event *>(event_buffer[i].data.ptr));
  }
  return Status::OK();
}

void Epoller::SetMaxEvent(int max_event) { max_event_ = max_event; }

void Epoller::SetTimeoutMs(int timeout_ms) { timeout_ms_ = timeout_ms; }

EpollerFactory::EpollerFactory(ServerConfig *config) : config_(config) {}

std::shared_ptr<Reactor> EpollerFactory::Build() {
  auto epoller_sp = std::make_shared<Epoller>(config_->GetEpollerMaxEvent(),
                                              config_->GetEpollerTimeoutMs());
  Status s = epoller_sp->Init();
  if (!s.Ok()) {
    WEBKIT_LOGFATAL("epoller init error status code %d message %s", s.Code(),
                    s.Message());
    return nullptr;
  }
  return epoller_sp;
}
}  // namespace webkit