#include "byte_packet.h"

#include <algorithm>

#include "webkit/logger.h"

namespace webkit {
BytePacket::BytePacket()
    : buffer_(nullptr), capacity_(0), write_pos_(0), read_pos_(0) {}

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

void BytePacket::Clear() {
  write_pos_ = 0;
  read_pos_ = 0;
}

size_t BytePacket::GetRemainDataSize() const {
  if (capacity_ == 0) return 0;
  if (read_pos_ <= write_pos_) return write_pos_ - read_pos_;
  return write_pos_ + capacity_ - read_pos_;
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