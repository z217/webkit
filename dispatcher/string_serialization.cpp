#include "string_serialization.h"

#include <arpa/inet.h>

#include "util/trace_helper.h"
#include "webkit/logger.h"

#define PACKET_WRITE_RETURN_IF_ERROR(PACKET, DATA, SIZE)                      \
  do {                                                                        \
    size_t write_size = 0;                                                    \
    Status s = (PACKET).Write((DATA), (SIZE), write_size);                    \
    if (!s.Ok()) {                                                            \
      WEBKIT_LOGERROR("packet write error code %d message %s", s.Code(),      \
                      s.Message());                                           \
      return s;                                                               \
    }                                                                         \
    if (write_size != (SIZE)) {                                               \
      WEBKIT_LOGERROR("packet write expect %zu get %zu", (SIZE), write_size); \
      return Status::ErrorF(StatusCode::eSerializeError,                      \
                            "packet write expect %zu get %zu", (SIZE),        \
                            write_size);                                      \
    }                                                                         \
  } while (false)

#define PACKET_READ_RETURN_IF_ERROR(PACKET, BUFFER, SIZE)                   \
  do {                                                                      \
    size_t read_size = 0;                                                   \
    Status s = (PACKET).Read((BUFFER), (SIZE), read_size);                  \
    if (!s.Ok()) {                                                          \
      WEBKIT_LOGERROR("packet read error code %d message %s", s.Code(),     \
                      s.Message());                                         \
      return s;                                                             \
    }                                                                       \
    if (read_size != (SIZE)) {                                              \
      WEBKIT_LOGERROR("packet read expect %zu get %zu", (SIZE), read_size); \
      return Status::ErrorF(StatusCode::eParseError,                        \
                            "packet read expect %zu get %zu", (SIZE),       \
                            read_size);                                     \
    }                                                                       \
  } while (false)

namespace webkit {
StringSerializer::StringSerializer(uint32_t method_id, const std::string &str)
    : str_(str) {
  memset(&meta_info_, 0, sizeof(MetaInfo));
  meta_info_.message_length = htonl(str_.length());
  meta_info_.method_id = htonl(method_id);
}

Status StringSerializer::SerializeTo(Packet &packet) {
  Status s = packet.Expand(sizeof(MetaInfo) + str_.length());
  if (!s.Ok()) {
    WEBKIT_LOGERROR("packet expand size %zu error code %d message %s",
                    sizeof(MetaInfo) + str_.length(), s.Code(), s.Message());
    return Status::Error(StatusCode::eSerializeError, "packet expand error");
  }
  strncpy(meta_info_.trace_id, TraceHelper::GetInstance()->GetTraceId().data(),
          sizeof(meta_info_.trace_id));
  PACKET_WRITE_RETURN_IF_ERROR(packet, &meta_info_, sizeof(MetaInfo));
  PACKET_WRITE_RETURN_IF_ERROR(packet, str_.data(), str_.length());
  return Status::OK();
}

StringParser::StringParser(std::string &str) : str_(str) {
  memset(&meta_info_, 0, sizeof(MetaInfo));
}

Status StringParser::ParseFrom(Packet &packet) {
  if (meta_info_.message_length == 0) {
    if (packet.GetRemainDataSize() < sizeof(MetaInfo)) {
      Status s = packet.Expand(sizeof(MetaInfo));
      if (!s.Ok()) {
        WEBKIT_LOGERROR("packet expand size %zu error code %d message %s",
                        sizeof(MetaInfo), s.Code(), s.Message());
        return Status::Error(StatusCode::eParseError, "packet expand error");
      }
      return Status::Debug(StatusCode::eRetry, "retry later");
    }
    PACKET_READ_RETURN_IF_ERROR(packet, &meta_info_, sizeof(MetaInfo));
    meta_info_.method_id = ntohl(meta_info_.method_id);
    meta_info_.message_length = ntohl(meta_info_.message_length);
    str_.resize(meta_info_.message_length);
    TraceHelper::GetInstance()->SetTraceId(meta_info_.trace_id);
    WEBKIT_LOGDEBUG("receive packet message length %zu",
                    meta_info_.message_length);
  }
  PACKET_READ_RETURN_IF_ERROR(packet, &str_[0], str_.length());
  return Status::OK();
}

const StringSerializationMetaInfo &StringParser::GetMetaInfo() const {
  return meta_info_;
}
}  // namespace webkit