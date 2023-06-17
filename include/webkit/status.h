#pragma once

#include <string>

#include "third_party/fmt/include/fmt/printf.h"

namespace webkit {
namespace StatusCode {
enum WebkitCode {
  eOk = 0,
  eRetry = 1,
  eNoData = 2,

  eParamError = -1,

  eSocketConnected = -100,
  eSocketDisonnected = -101,
  eSocketCreateError = -102,
  eSocketConnectError = -103,
  eSocketBindError = -104,
  eSocketListenError = -105,
  eSocketAcceptError = -106,
  eSocketWriteError = -107,
  eSocketReadError = -108,
  eSocketCloseError = -109,
  eSocketPeerClosed = -110,
  eSocketFcntlError = -111,
  eSocketOptError = -112,

  eChannelRouteError = -200,
  eChannelOpenError = -201,
  eChannelWriteError = -202,
  eChannelReadError = 203,

  eEpollInitError = -300,
  eEpollAddError = -301,
  eEpollRemoveError = -302,
  eEpollModifyError = -303,
  eEpollWaitError = -304,

  eDisptachError = -401,

  eCircularQueueEmpty = -501,
  eCircularQueueFull = -502,
  eCircularConcurFail = -503,

  eEventSendError = -601,
  eEventRecvError = -602,

  ePoolStopped = -701,

  eSerializeError = -801,
  eParseError = -802,

  eJsonParseError = -903,
  eJsonNotFoundError = -904,
  eJsonTypeError = -905,

  eSyscallDeamonError = -1001,
  eThreadKeyError = -1002,
};
}

class Status {
 public:
  enum Type {
    eNone = 0,
    eDebug = 1,
    eWarn = 2,
    eError = 3,
    eFatal = 4,
  };

  Status() : type_(eNone), code_(StatusCode::eOk), message_("ok") {}

  Status(Type type, int code, const std::string &message)
      : type_(type), code_(code), message_(message) {}

  ~Status() = default;

  bool Ok() const { return code_ == StatusCode::eOk; }

  int Code() const { return code_; }

  const std::string &Message() const { return message_; }

  std::string ToString() const {
    if (Ok()) return "status ok";
    return fmt::sprintf("[%s] status code %d message %s", TypeToString(type_),
                        code_, message_);
  }

  static Status OK() { return Status(); }

  static Status Debug(int code, const std::string &message = "") {
    return Status(eDebug, code, message);
  }

  template <typename... Args>
  static Status DebugF(int code, const char *format, const Args &...args) {
    return Status(eDebug, code, fmt::sprintf(format, args...));
  }

  static Status Warn(int code, const std::string &message = "") {
    return Status(eWarn, code, message);
  }

  template <typename... Args>
  static Status WarnF(int code, const char *format, const Args &...args) {
    return Status(eWarn, code, fmt::sprintf(format, args...));
  }

  static Status Error(int code, const std::string &message = "") {
    return Status(eError, code, message);
  }

  template <typename... Args>
  static Status ErrorF(int code, const char *format, const Args &...args) {
    return Status(eError, code, fmt::sprintf(format, args...));
  }

  static Status Fatal(int code, const std::string &message = "") {
    return Status(eFatal, code, message);
  }

  template <typename... Args>
  static Status FatalF(int code, const char *format, const Args &...args) {
    return Status(eFatal, code, fmt::sprintf(format, args...));
  }

 private:
  static std::string TypeToString(Type type) {
    static constexpr const char *type_strings[]{"None", "Debug", "Warning",
                                                "Error", "Fatal"};
    return type_strings[type];
  }

  Type type_;
  int code_;
  std::string message_;
};
}  // namespace webkit