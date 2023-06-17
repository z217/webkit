#pragma once

#include <sys/epoll.h>

#include <memory>

#include "webkit/event.h"
#include "webkit/packet.h"
#include "webkit/socket.h"

namespace webkit {
class Epoller;
class EpollEvent : public Event {
 public:
  EpollEvent(std::shared_ptr<Epoller> epoller_sp,
             std::shared_ptr<Socket> socket_sp,
             std::shared_ptr<Packet> packet_sp, bool is_et_mode = true);

  struct epoll_event *GetEpollEvent();

  void SetEtMode(bool is_et_mode);

  bool IsEtMode() const;

  void SetReadyToSend() override;

  bool IsReadyToSend() const override;

  void SetReadyToRecv() override;

  bool IsReadyToRecv() const override;

  void ClearEvent() override;

  Status Send() override;

  Status Recv() override;

  Status AddToReactor() override;

  Status DelFromReactor() override;

  Status ModInReactor() override;

  void SetSocket(std::shared_ptr<Socket> socket_sp);

  std::shared_ptr<Socket> GetSocket();

  void SetPacket(std::shared_ptr<Packet> packet_sp);

  std::shared_ptr<Packet> GetPacket();

 private:
  std::shared_ptr<Epoller> epoller_sp_;
  std::shared_ptr<Socket> socket_sp_;
  std::shared_ptr<Packet> packet_sp_;
  struct epoll_event epoll_event_;
  bool is_et_mode_;
};
}  // namespace webkit