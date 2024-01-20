#include "simple_adapter.h"

#include <arpa/inet.h>

#include <cassert>
#include <vector>

#include "util/inet_util.h"
#include "webkit/logger.h"

static constexpr uint32_t kVersion = 1U;

namespace webkit {
SimpleAdapter::SimpleAdapter(IoBase &src, IoBase &dst, size_t src_size)
    : ProtocolAdapter(src, dst, src_size), header_size_(0) {
  header_.length = InetUtil::Hton(static_cast<uint32_t>(src_size));
  header_.version = InetUtil::Hton(kVersion);
}

Status SimpleAdapter::AdaptTo() {
  Status s = WriteHeader();
  if (!s.Ok()) return s;

  s = WriteBody();
  if (!s.Ok()) return s;

  return Status::OK();
}

Status SimpleAdapter::AdaptFrom() {
  Status s = ReadHeader();
  if (!s.Ok()) return s;

  s = ReadBody();
  if (!s.Ok()) return s;

  return Status::OK();
}

Status SimpleAdapter::WriteHeader() {
  if (header_size_ >= sizeof(Header)) return Status::OK();

  void *p_src = reinterpret_cast<uint8_t *>(&header_) + header_size_;
  size_t src_length = sizeof(Header) - header_size_;
  size_t write_size = 0;
  Status s = dst_.Write(p_src, src_length, write_size);
  header_size_ += write_size;
  if (!s.Ok() || header_size_ != sizeof(Header)) {
    WEBKIT_LOGERROR("write dst expect %zu get %zu status code %d message %s",
                    src_length, write_size, s.Code(), s.Message());
    return s;
  }
  return Status::OK();
}

Status SimpleAdapter::WriteBody() {
  if (src_size_ == 0) return Status::OK();

  size_t write_size = 0;
  Status s = dst_.Write(src_, src_size_, write_size);
  src_size_ -= write_size;
  if (!s.Ok() || src_size_ != 0) {
    WEBKIT_LOGERROR("write dst expect %zu get %zu status code %d message %s",
                    src_size_ + write_size, write_size, s.Code(), s.Message());
    return s;
  }

  return Status::OK();
}

Status SimpleAdapter::ReadHeader() {
  if (header_size_ >= sizeof(Header)) return Status::OK();

  void *p_dst = reinterpret_cast<uint8_t *>(&header_) + header_size_;
  size_t dst_length = sizeof(Header) - header_size_;
  size_t read_size = 0;
  Status s = dst_.Read(p_dst, dst_length, read_size);
  header_size_ += read_size;
  if (!s.Ok() || header_size_ != sizeof(Header)) {
    WEBKIT_LOGERROR("read dst expect %zu get %zu status code %d message %s",
                    dst_length, read_size, s.Code(), s.Message());
    return s;
  }

  uint32_t version = InetUtil::Ntoh(header_.version);
  if (version != kVersion) {
    WEBKIT_LOGERROR("read header version diff, expect %u get %u", kVersion,
                    version);
    return Status::Error(StatusCode::eAdapterVersionError, "version diff");
  }
  dst_size_ = InetUtil::Ntoh(header_.length);
  return Status::OK();
}

Status SimpleAdapter::ReadBody() {
  if (dst_size_ == 0) return Status::OK();

  size_t read_size = 0;
  Status s = dst_.Read(src_, dst_size_, read_size);
  dst_size_ -= read_size;
  if (!s.Ok() || dst_size_ != 0) {
    WEBKIT_LOGERROR("read dst expect %zu get %zu status code %d message %s",
                    dst_size_ + read_size, read_size, s.Code(), s.Message());
    return s;
  }

  return Status::OK();
}

std::shared_ptr<ProtocolAdapter> SimpleAdapterFactory::Build(IoBase &src,
                                                             IoBase &dst,
                                                             size_t src_size) {
  return std::make_shared<SimpleAdapter>(src, dst, src_size);
}
}  // namespace webkit
