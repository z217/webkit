#include "byte_packet.h"

#include <algorithm>

#include "webkit/logger.h"

namespace webkit {
BytePacket::BytePacket()
    : buffer_(nullptr), capacity_(128), write_pos_(0), read_pos_(0) {
  ExpandIfNoSpace(capacity_);
}

BytePacket::~BytePacket() { delete[] buffer_; }

Status BytePacket::Write(const void *src, size_t src_size, size_t &write_size) {
  ExpandIfNoSpace(src_size);
  write_size = 0;
  if (write_pos_ + src_size > capacity_) {
    size_t tail_size = capacity_ - write_pos_;
    memcpy(buffer_ + write_pos_, src, tail_size);
    write_pos_ = 0;
    src_size -= tail_size;
    write_size = tail_size;
  }
  memcpy(buffer_ + write_pos_,
         reinterpret_cast<const std::byte *>(src) + write_size, src_size);
  write_size += src_size;
  write_pos_ = (write_pos_ + src_size) % capacity_;
  return Status::OK();
}

Status BytePacket::Write(IoBase &other) {
  Status s;
  size_t write_size = 0;
  size_t remain_size = GetRemainDataSize();

  if (read_pos_ + remain_size > capacity_) {
    size_t tail_size = capacity_ - read_pos_;
    s = other.Write(buffer_ + read_pos_, tail_size, write_size);
    read_pos_ = (read_pos_ + write_size) % capacity_;
    if (!s.Ok()) return s;
    if (write_size != tail_size) {
      return Status::Debug(StatusCode::eRetry, "retry later");
    }
    remain_size -= tail_size;
  }

  write_size = 0;
  s = other.Write(buffer_ + read_pos_, remain_size, write_size);
  read_pos_ += write_size;
  if (!s.Ok()) return s;
  if (write_size != remain_size) {
    return Status::Debug(StatusCode::eRetry, "retry later");
  }

  return Status::OK();
}

Status BytePacket::Read(void *dst, size_t dst_size, size_t &read_size) {
  size_t target_size = std::min(GetRemainDataSize(), dst_size);
  read_size = 0;
  if (target_size == 0) return Status::OK();
  if (read_pos_ + target_size > capacity_) {
    size_t tail_size = capacity_ - read_pos_;
    memcpy(dst, buffer_ + read_pos_, tail_size);
    read_pos_ = 0;
    target_size -= tail_size;
    read_size = tail_size;
  }
  memcpy(reinterpret_cast<std::byte *>(dst) + read_size, buffer_ + read_pos_,
         target_size);
  read_size += target_size;
  read_pos_ = (read_pos_ + target_size) % capacity_;
  return Status::OK();
}

Status BytePacket::Read(IoBase &other) {
  Status s;
  size_t read_size = 0;
  size_t empty_size = GetRemainEmptySize();
  if (empty_size == 0) {
    ExpandIfNoSpace(capacity_);
    empty_size = GetRemainEmptySize();
  }

  if (write_pos_ >= read_pos_) {
    size_t tail_size = capacity_ - write_pos_;
    s = other.Read(buffer_ + write_pos_, tail_size, read_size);
    write_pos_ = (write_pos_ + read_size) % capacity_;
    if (s.Code() == StatusCode::eNoData) return Status::OK();
    if (s.Code() == StatusCode::eRetry) return s;
    if (!s.Ok()) return s;
    return Status::OK();
  }

  s = other.Read(buffer_, empty_size, read_size);
  if (s.Code() == StatusCode::eNoData) return Status::OK();
  if (s.Code() == StatusCode::eRetry) return s;
  if (!s.Ok()) return s;
  return Status::OK();
}

void BytePacket::Clear() {
  write_pos_ = 0;
  read_pos_ = 0;
}

size_t BytePacket::GetRemainDataSize() const {
  if (capacity_ == 0) return 0;
  if (read_pos_ <= write_pos_) return write_pos_ - read_pos_;
  return write_pos_ + capacity_ - read_pos_;
}

size_t BytePacket::GetRemainEmptySize() const {
  if (capacity_ == 0) return 0;
  if (read_pos_ >= write_pos_) return read_pos_ - write_pos_;
  return read_pos_ + capacity_ - write_pos_;
}

Status BytePacket::Expand(size_t expand_size) {
  ExpandIfNoSpace(expand_size);
  return Status::OK();
}

void BytePacket::ExpandIfNoSpace(size_t append_size) {
  size_t use_size = GetRemainDataSize();
  if (use_size + append_size < capacity_) return;
  size_t new_capacity = (capacity_ + append_size) / 2 * 3;
  std::byte *new_buffer = new std::byte[new_capacity];
  size_t new_use_size;
  Fold(new_buffer, new_use_size);
  delete[] buffer_;
  buffer_ = new_buffer;
  capacity_ = new_capacity;
  write_pos_ = new_use_size;
  read_pos_ = 0;
}

void BytePacket::Fold(std::byte *dst, size_t &dst_size) {
  dst_size = 0;
  if (read_pos_ > write_pos_) {
    memcpy(dst, buffer_ + read_pos_, capacity_ - read_pos_);
    read_pos_ = 0;
    dst_size = capacity_ - read_pos_;
  }
  memcpy(dst + dst_size, buffer_ + read_pos_, write_pos_ - read_pos_);
  dst_size += write_pos_ - read_pos_;
}

std::shared_ptr<Packet> BytePacketFactory::Build() {
  return std::make_shared<BytePacket>();
}
}  // namespace webkit