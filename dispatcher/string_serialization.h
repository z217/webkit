#pragma once

#include <string>

#include "webkit/serialization.h"

namespace webkit {
struct StringSerialization {
public:
#pragma pack(push, 1)
  struct MetaInfo {
    uint32_t meta_length;
    uint32_t method_id;
    uint32_t message_length;
    char trace_id[32];
  };
#pragma pack(pop)
};

class StringSerializer : public Serializer, public StringSerialization {
public:
  using MetaInfo = StringSerialization::MetaInfo;

  StringSerializer(uint32_t method_id, const std::string &str);

  virtual Status SerializeTo(Packet &packet) override;

private:
  const std::string &str_;
  MetaInfo meta_info_;
};

class StringParser : public Parser, public StringSerialization {
public:
  using MetaInfo = StringSerialization::MetaInfo;

  StringParser(std::string &str);

  virtual Status ParseFrom(Packet &packet) override;

  const MetaInfo &GetMetaInfo() const;

private:
  std::string &str_;
  MetaInfo meta_info_;
};
} // namespace webkit