#pragma once

#include <cstddef>

#include "webkit/packet.h"

namespace webkit {
class BytePacket : public Packet {
 public:
  BytePacket();

  ~BytePacket();

  Status Write(const void *src, size_t src_size, size_t &write_size) override;

  Status Write(IoBase &other) override;

  Status Read(void *dst, size_t dst_size, size_t &read_size) override;

  Status Read(IoBase &other) override;

  void Clear() override;

  size_t GetRemainDataSize() const override;

  Status Expand(size_t expand_size) override;

 private:
  size_t GetRemainEmptySize() const;

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