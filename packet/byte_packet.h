#pragma once

#include <cstddef>

#include "webkit/packet.h"
#include "webkit/server_config.h"

namespace webkit {
class BytePacketBuf : public IoBase, public std::streambuf {
 public:
  BytePacketBuf(size_t capactiy);

  virtual ~BytePacketBuf();

  virtual Status Write(const void *src, size_t src_size,
                       size_t &write_size) override;

  virtual Status Write(IoBase &src, size_t src_size,
                       size_t &write_size) override;

  virtual Status Read(void *dst, size_t dst_size, size_t &read_size) override;

  virtual Status Read(IoBase &dst, size_t dst_size, size_t &read_size) override;

  virtual int_type overflow(int_type c) override;

  virtual int_type underflow() override;

  void Clear();

  size_t GetDataSize() const;

 private:
  Status Expand(size_t new_size);

  Status EnsureSize(size_t size);

  void Fit();

  void Fold(std::byte *dst, size_t &dst_size);

  std::byte *buffer_;
  size_t size_;
  size_t capacity_;
};

class BytePacket : public Packet {
 public:
  BytePacket(size_t capacity);

  virtual ~BytePacket() = default;

  virtual Status Write(const void *src, size_t src_size,
                       size_t &write_size) override;

  virtual Status Write(IoBase &src, size_t src_size,
                       size_t &write_size) override;

  virtual Status Read(void *dst, size_t dst_size, size_t &read_size) override;

  virtual Status Read(IoBase &dst, size_t dst_size, size_t &read_size) override;

  virtual void Clear();

  virtual size_t GetDataSize() const;

 private:
  BytePacketBuf buf_;
};

class BytePacketFactory : public PacketFactory {
 public:
  BytePacketFactory(ServerConfig *p_config);

  virtual ~BytePacketFactory() = default;

  virtual std::shared_ptr<Packet> Build() override;

 private:
  ServerConfig *p_config_;
};
}  // namespace webkit