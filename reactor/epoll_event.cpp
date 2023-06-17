#include "epoll_event.h"

#include "reactor/epoller.h"
#include "webkit/logger.h"

namespace webkit {
EpollEvent::EpollEvent(std::shared_ptr<Epoller> epoller_sp,
                       std::shared_ptr<Socket> socket_sp,
                       std::shared_ptr<Packet> packet_sp, bool is_et_mode)
    : epoller_sp_(epoller_sp),
      socket_sp_(socket_sp),
      packet_sp_(packet_sp),
      is_et_mode_(is_et_mode) {
  memset(&epoll_event_, 0, sizeof(epoll_event_));
  epoll_event_.data.ptr = this;
}

struct epoll_event *EpollEvent::GetEpollEvent() { return &epoll_event_; }

void EpollEvent::SetEtMode(bool is_et_mode) { is_et_mode_ = is_et_mode; }

bool EpollEvent::IsEtMode() const { return is_et_mode_; }

void EpollEvent::SetReadyToSend() {
  epoll_event_.events |= EPOLLOUT | EPOLLRDHUP;
  if (is_et_mode_) epoll_event_.events |= EPOLLET;
}

bool EpollEvent::IsReadyToSend() const {
  return epoll_event_.events & EPOLLOUT;
}

void EpollEvent::SetReadyToRecv() {
  epoll_event_.events |= EPOLLIN | EPOLLRDHUP;
  if (is_et_mode_) epoll_event_.events |= EPOLLET;
}

bool EpollEvent::IsReadyToRecv() const { return epoll_event_.events & EPOLLIN; }

Status EpollEvent::Send() {
  Status s = packet_sp_->Write(*socket_sp_);
  if (s.Code() == StatusCode::eRetry) return s;
  if (!s.Ok()) {
    WEBKIT_LOGERROR("packet write socket error code %d message %s", s.Code(),
                    s.Message());
    return Status::Error(StatusCode::eEventSendError,
                         "packet write socket error");
  }
  return Status::OK();
}

Status EpollEvent::Recv() {
  Status s = packet_sp_->Read(*socket_sp_);
  if (s.Code() == StatusCode::eRetry) return s;
  if (!s.Ok()) {
    WEBKIT_LOGERROR("packet read socket error code %d message %s", s.Code(),
                    s.Message());
    return Status::Error(StatusCode::eEventRecvError,
                         "packet read socket error");
  }
  return Status::OK();
}

void EpollEvent::ClearEvent() { epoll_event_.events = 0; }

Status EpollEvent::AddToReactor() { return epoller_sp_->Add(this); }

Status EpollEvent::DelFromReactor() { return epoller_sp_->Delete(this); }

Status EpollEvent::ModInReactor() { return epoller_sp_->Modify(this); }

void EpollEvent::SetSocket(std::shared_ptr<Socket> socket_sp) {
  socket_sp_ = socket_sp;
}

std::shared_ptr<Socket> EpollEvent::GetSocket() { return socket_sp_; }

void EpollEvent::SetPacket(std::shared_ptr<Packet> packet_sp) {
  packet_sp = packet_sp;
}

std::shared_ptr<Packet> EpollEvent::GetPacket() { return packet_sp_; }
}  // namespace webkit