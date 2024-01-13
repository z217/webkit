#pragma once

#include <string>

#include "webkit/protocol_adapter.h"

namespace webkit {
class SimpleAdapter : public ProtocolAdapter {
 private:
#pragma pack(push, 1)
  struct Header {
    uint32_t version;
    uint32_t length;
  };
#pragma pack(pop)

 public:
  SimpleAdapter(IoBase &src, IoBase &dst, size_t src_size);

  virtual ~SimpleAdapter() = default;

  virtual Status AdaptTo() override;

  virtual Status AdaptFrom() override;

 private:
  Status WriteHeader();

  Status WriteBody();

  Status ReadHeader();

  Status ReadBody();

  Header header_;
  size_t header_size_;
};

class SimpleAdapterFactory : public ProtocolAdapterFactory {
 public:
  SimpleAdapterFactory() = default;

  ~SimpleAdapterFactory() = default;

  std::shared_ptr<ProtocolAdapter> Build(IoBase &src, IoBase &dst,
                                         size_t src_size) override;
};
}  // namespace webkit