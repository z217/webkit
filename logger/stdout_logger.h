#pragma once

#include <unistd.h>

#include "logger/callback_logger.h"

namespace webkit {
class StdoutLogger : public CallbackLogger {
 public:
  StdoutLogger(CallbackType prefix_cb = DefaultPrefixCallback,
               CallbackType suffix_cb = DefaultSuffixCallback)
      : CallbackLogger(prefix_cb, suffix_cb) {}

  virtual ~StdoutLogger() = default;

  void Log(Level level, const std::string &message) override {
    if (!IsLogEnable(level)) return;
    write(fileno(stdout), message.data(), message.length());
  }
};
}  // namespace webkit