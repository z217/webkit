#include "byte_packet.h"

#include <algorithm>

#include "webkit/logger.h"

namespace webkit {
BytePacketBuf::BytePacketBuf(size_t capacity)
    : buffer_(nullptr), size_(0), capacity_(capacity) {
  setg(nullptr, nullptr, nullptr);
  setp(nullptr, nullptr);
}

BytePacketBuf::~BytePacketBuf() { delete[] buffer_; }

Status BytePacketBuf::Write(const void *src, size_t src_size,
                            size_t &write_size) {
  Fit();
  Status s = EnsureSize(src_size);
  if (!s.Ok()) return s;
  write_size = 0;
  if (pptr() + src_size >= epptr()) {
    size_t tail_size = static_cast<size_t>(epptr() - pptr());
    memcpy(pptr(), src, tail_size);
    pbump(tail_size);
    Fit();
    src_size -= tail_size;
    write_size += tail_size;
  }
  memcpy(pptr(), reinterpret_cast<const std::byte *>(src) + write_size,
         src_size);
  pbump(src_size);
  write_size += src_size;
  return Status::OK();
}

Status BytePacketBuf::Write(IoBase &src, size_t src_size, size_t &write_size) {
  Fit();
  Status s = EnsureSize(src_size);
  if (!s.Ok()) return s;
  write_size = 0;
  if (pptr() + src_size >= epptr()) {
    size_t tail_size = static_cast<size_t>(epptr() - pptr());
    size_t read_size = 0;
    s = src.Read(pptr(), tail_size, read_size);
    pbump(read_size);
    Fit();
    src_size -= read_size;
    write_size += read_size;
    if (!s.Ok() || tail_size != read_size) {
      WEBKIT_LOGERROR("read src expect %zu get %zu status code %d message %s",
                      tail_size, read_size, s.Code(), s.Message());
      return s;
    }
  }
  size_t read_size = 0;
  s = src.Read(pptr(), src_size, read_size);
  pbump(read_size);
  write_size += read_size;
  if (!s.Ok() || src_size != read_size) {
    WEBKIT_LOGERROR("read src expect %zu get %zu status code %d message %s",
                    src_size, read_size, s.Code(), s.Message());
    return s;
  }

  return Status::OK();
}

Status BytePacketBuf::Read(void *dst, size_t dst_size, size_t &read_size) {
  Fit();
  size_t data_size = std::min(GetDataSize(), dst_size);
  read_size = 0;
  if (gptr() + data_size >= egptr()) {
    size_t tail_size = static_cast<size_t>(egptr() - gptr());
    memcpy(dst, gptr(), tail_size);
    gbump(tail_size);
    Fit();
    data_size -= tail_size;
    read_size += tail_size;
  }
  memcpy(reinterpret_cast<std::byte *>(dst) + read_size, gptr(), data_size);
  gbump(data_size);
  read_size += data_size;
  return Status::OK();
}

Status BytePacketBuf::Read(IoBase &dst, size_t dst_size, size_t &read_size) {
  Fit();
  size_t data_size = std::min(GetDataSize(), dst_size);
  read_size = 0;
  if (gptr() + data_size >= egptr()) {
    size_t tail_size = static_cast<size_t>(egptr() - gptr());
    size_t write_size = 0;
    Status s = dst.Write(gptr(), tail_size, write_size);
    gbump(write_size);
    Fit();
    data_size -= write_size;
    read_size += write_size;
    if (!s.Ok() || write_size != tail_size) {
      WEBKIT_LOGERROR("write dst expect %zu get %zu status code %d message %s",
                      data_size, write_size, s.Code(), s.Message());
      return s;
    }
  }
  size_t write_size = 0;
  Status s = dst.Write(gptr(), data_size, write_size);
  gbump(write_size);
  read_size += write_size;
  if (!s.Ok() || write_size != data_size) {
    WEBKIT_LOGERROR("write dst expect %zu get %zu status code %d message %s",
                    data_size, write_size, s.Code(), s.Message());
    return s;
  }

  return Status::OK();
}

BytePacketBuf::int_type BytePacketBuf::overflow(int_type c) {
  if (pptr() < gptr()) {
    setp(pptr(), gptr());
  } else if (gptr() > eback()) {
    setp(reinterpret_cast<char *>(buffer_), gptr());
  } else if (size_ < capacity_) {
    size_t new_size = std::min(size_ / 2 * 3, capacity_);
    Status s = Expand(new_size - size_);
    if (!s.Ok()) return traits_type::eof();
  } else {
    return traits_type::eof();
  }
  if (!traits_type::eq_int_type(c, traits_type::eof())) {
    sputc(c);
  }
  return traits_type::not_eof(c);
}

BytePacketBuf::int_type BytePacketBuf::underflow() {
  if (gptr() < pptr()) {
    setg(gptr(), gptr(), pptr());
  } else if (pptr() > pbase()) {
    setg(reinterpret_cast<char *>(buffer_), reinterpret_cast<char *>(buffer_),
         pptr());
  } else {
    return traits_type::eof();
  }
  return traits_type::to_int_type(*gptr());
}

void BytePacketBuf::Clear() {
  char *ptr = reinterpret_cast<char *>(buffer_);
  setg(ptr, ptr, ptr);
  setp(ptr, ptr);
}

size_t BytePacketBuf::GetDataSize() const {
  if (egptr() <= epptr()) return static_cast<size_t>(pptr() - gptr());
  return pptr() + size_ - gptr();
}

Status BytePacketBuf::Expand(size_t alloc_size) {
  size_t new_size = size_ + alloc_size;
  if (new_size > capacity_) {
    WEBKIT_LOGERROR("expand packet expect %zu capacity %zu", new_size,
                    capacity_);
    return Status::Error(StatusCode::ePacketFullError, "packet is full");
  }
  new_size = std::min(new_size / 2 * 3, capacity_);
  std::byte *new_buffer = new std::byte[new_size];
  size_t data_size;
  Fold(new_buffer, data_size);
  std::swap(buffer_, new_buffer);
  size_ = new_size;
  setp(reinterpret_cast<char *>(buffer_) + data_size,
       reinterpret_cast<char *>(buffer_) + size_);
  setg(reinterpret_cast<char *>(buffer_), reinterpret_cast<char *>(buffer_),
       reinterpret_cast<char *>(buffer_) + data_size);
  delete[] new_buffer;
  return Status::OK();
}

Status BytePacketBuf::EnsureSize(size_t size) {
  size_t data_size = GetDataSize();
  if (size_ >= size + data_size) return Status::OK();
  return Expand(size + data_size - size_);
}

void BytePacketBuf::Fit() {
  if (egptr() < pptr()) {
    setg(gptr(), gptr(), pptr());
    if (pptr() >= epptr()) {
      setp(reinterpret_cast<char *>(buffer_), gptr());
    }
  } else if (epptr() < gptr()) {
    setp(pptr(), gptr());
    if (gptr() >= egptr()) {
      setg(reinterpret_cast<char *>(buffer_), reinterpret_cast<char *>(buffer_),
           pptr());
    }
  }
}

void BytePacketBuf::Fold(std::byte *dst, size_t &dst_size) {
  dst_size = 0;
  if (egptr() > epptr()) {
    size_t tail_size = static_cast<size_t>(egptr() - gptr());
    memmove(dst, gptr(), tail_size);
    setg(reinterpret_cast<char *>(buffer_), reinterpret_cast<char *>(buffer_),
         pptr());
    dst_size += tail_size;
  }
  size_t data_size = static_cast<size_t>(pptr() - gptr());
  memmove(dst + dst_size, gptr(), data_size);
  dst_size += data_size;
  gbump(data_size);
}

BytePacket::BytePacket(size_t capacity) : buf_(capacity), Packet(&buf_) {}

Status BytePacket::Write(const void *src, size_t src_size, size_t &write_size) {
  return buf_.Write(src, src_size, write_size);
}

Status BytePacket::Write(IoBase &src, size_t src_size, size_t &write_size) {
  return buf_.Write(src, src_size, write_size);
}

Status BytePacket::Read(void *dst, size_t dst_size, size_t &read_size) {
  return buf_.Read(dst, dst_size, read_size);
}

Status BytePacket::Read(IoBase &dst, size_t dst_size, size_t &read_size) {
  return buf_.Read(dst, dst_size, read_size);
}

void BytePacket::Clear() { buf_.Clear(); }

size_t BytePacket::GetDataSize() const { return buf_.GetDataSize(); }

BytePacketFactory::BytePacketFactory(ServerConfig *p_config)
    : p_config_(p_config) {}

std::shared_ptr<Packet> BytePacketFactory::Build() {
  return std::make_shared<BytePacket>(p_config_->GetPacketMaxSize());
}
}  // namespace webkit