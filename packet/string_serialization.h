#pragma once

#include <string>

#include "webkit/serialization.h"

namespace webkit {
#pragma pack(push, 1)
struct MetaInfo {
  size_t message_length;
  char trace_id[32];
};
#pragma pack(pop)

class StringSerializer : public Serializer {
 public:
  StringSerializer(const std::string &str);

  Status SerializeTo(Packet &packet) override;

 private:
  const std::string &str_;
  MetaInfo meta_info_;
};

class StringParser : public Parser {
 public:
  StringParser(std::string &str);

  Status ParseFrom(Packet &packet) override;

 private:
  std::string &str_;
  MetaInfo meta_info_;
};
}  // namespace webkit