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

  virtual ~EpollEvent() = default;

  struct epoll_event *GetEpollEvent();

  void SetEtMode(bool is_et_mode);

  bool IsEtMode() const;

  virtual void SetReadyToSend() override;

  virtual bool IsReadyToSend() const override;

  virtual void SetReadyToRecv() override;

  virtual bool IsReadyToRecv() const override;

  virtual void ClearEvent() override;

  virtual Status Send() override;

  virtual Status Recv() override;

  virtual Status AddToReactor() override;

  virtual Status DelFromReactor() override;

  virtual Status ModInReactor() override;

  virtual void SetBusy(bool is_busy) override;

  virtual bool IsBusy() const override;

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
  bool is_busy_;
};
}  // namespace webkit