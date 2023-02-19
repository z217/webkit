#include "epoll_event.h"

#include "webkit/logger.h"

namespace webkit {
static constexpr size_t kBufferSize = 8 << 20;

static char *GetBuffer() {
  static thread_local std::unique_ptr<char[]> buffer_up(new char[kBufferSize]);
  return buffer_up.get();
}

EpollEvent::EpollEvent(std::shared_ptr<Socket> socket_sp,
                       std::shared_ptr<Packet> packet_sp, bool is_et_mode)
    : socket_sp_(socket_sp), packet_sp_(packet_sp), is_et_mode_(is_et_mode) {
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
  Status s;
  char *buffer = GetBuffer();
  while (packet_sp_->GetRemainDataSize() != 0) {
    size_t data_size;
    s = packet_sp_->Read(buffer, kBufferSize, data_size);
    if (!s.Ok()) {
      WEBKIT_LOGERROR("read packet error status code %d message %s", s.Code(),
                      s.Message());
      return Status::Error(StatusCode::eEventSendError,
                           "event send packet read error");
    }

    size_t offset = 0;
    while (offset < data_size) {
      size_t write_size;
      s = socket_sp_->Write(buffer + offset, data_size - offset, write_size);
      if (!s.Ok()) {
        WEBKIT_LOGERROR("socket write error status code %d message %s",
                        s.Code(), s.Message());
        return Status::Error(StatusCode::eEventSendError,
                             "event send socket write error");
      }
      offset += write_size;
      if (offset > data_size) {
        WEBKIT_LOGFATAL(
            "socket write may have bug, data size is greater than write size");
        return Status::Error(StatusCode::eEventSendError,
                             "event send socket write may have bug");
      }
    }
  }

  return Status::OK();
}

Status EpollEvent::Recv() {
  Status s;
  char *buffer = GetBuffer();
  bool need_read_again = false;

  do {
    size_t data_size;
    s = socket_sp_->Read(buffer, kBufferSize, data_size);
    if (!s.Ok() && s.Code() != StatusCode::eNoData) {
      WEBKIT_LOGERROR("socket read error status code %d message %s", s.Code(),
                      s.Message());
      return Status::Error(StatusCode::eEventRecvError,
                           "event recv socket read error");
    }
    need_read_again == s.Code() != StatusCode::eNoData;

    size_t offset = 0;
    while (offset < data_size) {
      size_t write_size;
      s = packet_sp_->Write(buffer + offset, data_size - offset, write_size);
      if (!s.Ok()) {
        WEBKIT_LOGERROR("packet write error status code %d message %s",
                        s.Code(), s.Message());
        return Status::Error(StatusCode::eEventRecvError,
                             "event recv packet write error");
      }
      offset += write_size;
      if (offset > data_size) {
        WEBKIT_LOGFATAL(
            "socket read may have bug, data size if less than write size");
        return Status::Error(StatusCode::eEventRecvError,
                             "event recv socket write may have bug");
      }
    }
  } while (need_read_again);

  return Status::OK();
}

void EpollEvent::ClearEvent() { epoll_event_.events = 0; }

void EpollEvent::SetSocket(std::shared_ptr<Socket> socket_sp) {
  socket_sp_ = socket_sp;
}

std::shared_ptr<Socket> EpollEvent::GetSocket() { return socket_sp_; }

void EpollEvent::SetPacket(std::shared_ptr<Packet> packet_sp) {
  packet_sp = packet_sp;
}

std::shared_ptr<Packet> EpollEvent::GetPacket() { return packet_sp_; }
}  // namespace webkit