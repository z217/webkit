#pragma once

#include <memory>

#include "webkit/packet.h"
#include "webkit/socket.h"
#include "webkit/status.h"

namespace webkit {
class Event : public std::enable_shared_from_this<Event> {
 public:
  Event() = default;

  virtual ~Event() = default;

  virtual void SetReadyToSend() = 0;

  virtual bool IsReadyToSend() const = 0;

  virtual void SetReadyToRecv() = 0;

  virtual bool IsReadyToRecv() const = 0;

  virtual void ClearEvent() = 0;

  virtual Status Send() = 0;

  virtual Status Recv() = 0;

  virtual void SetSocket(std::shared_ptr<Socket> socket_sp) = 0;

  virtual std::shared_ptr<Socket> GetSocket() = 0;

  virtual void SetPacket(std::shared_ptr<Packet> packet_sp) = 0;

  virtual std::shared_ptr<Packet> GetPacket() = 0;

  virtual Status AddToReactor() = 0;

  virtual Status DelFromReactor() = 0;

  virtual Status ModInReactor() = 0;

  virtual void SetBusy(bool is_busy) = 0;

  virtual bool IsBusy() const = 0;
};
}  // namespace webkit