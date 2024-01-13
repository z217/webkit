#pragma once

#include <memory>

#include "webkit/instance_base.h"
#include "webkit/io_base.h"
#include "webkit/status.h"

namespace webkit {
class ProtocolAdapter {
 public:
  ProtocolAdapter(IoBase &src, IoBase &dst, size_t src_size)
      : src_(src), dst_(dst), src_size_(src_size), dst_size_(0) {}

  virtual ~ProtocolAdapter() = default;

  virtual Status AdaptTo() = 0;

  virtual Status AdaptFrom() = 0;

 protected:
  IoBase &src_;
  IoBase &dst_;
  size_t src_size_;
  size_t dst_size_;
};

class ProtocolAdapterFactory : public InstanceBase<ProtocolAdapterFactory> {
 public:
  ProtocolAdapterFactory() = default;

  virtual ~ProtocolAdapterFactory() = default;

  virtual std::shared_ptr<ProtocolAdapter> Build(IoBase &src, IoBase &dst,
                                                 size_t src_size) = 0;
};
}  // namespace webkit