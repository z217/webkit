#pragma once

#include <string>

#include "webkit/serialization.h"

namespace webkit {
#pragma pack(push, 1)
struct StringSerializationMetaInfo {
  uint32_t method_id;
  uint32_t message_length;
  char trace_id[32];
};
#pragma pack(pop)

class StringSerializer : public Serializer {
 public:
  using MetaInfo = StringSerializationMetaInfo;

  StringSerializer(uint32_t method_id, const std::string &str);

  virtual Status SerializeTo(Packet &packet) override;

 private:
  const std::string &str_;
  MetaInfo meta_info_;
};

class StringParser : public Parser {
 public:
  using MetaInfo = StringSerializationMetaInfo;

  StringParser(std::string &str);

  virtual Status ParseFrom(Packet &packet) override;

  const MetaInfo &GetMetaInfo() const;

 private:
  std::string &str_;
  MetaInfo meta_info_;
};
}  // namespace webkit