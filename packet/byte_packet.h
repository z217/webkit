#pragma once

#include <cstddef>

#include "webkit/packet.h"

namespace webkit {
class BytePacket : public Packet {
 public:
  BytePacket();

  virtual ~BytePacket();

  virtual Status Write(const void *src, size_t src_size,
                       size_t &write_size) override;

  virtual Status Read(void *dst, size_t dst_size, size_t &read_size) override;

  virtual void Clear() override;

  size_t GetRemainDataSize() const override;

 private:
  void ExpandIfNoSpace(size_t append_size);

  void Fold(std::byte *dst, size_t &dst_size);

  std::byte *buffer_;
  size_t capacity_;
  size_t write_pos_;
  size_t read_pos_;
};

class BytePacketFactory : public PacketFactory {
 public:
  BytePacketFactory() = default;

  ~BytePacketFactory() = default;

  std::shared_ptr<Packet> Build() override;
};
}  // namespace webkit