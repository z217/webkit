#pragma once

#include <unistd.h>

#include "webkit/logger.h"

namespace webkit {
class StdoutLogger : public Logger {
 public:
  StdoutLogger() = default;

  virtual ~StdoutLogger() = default;

  void Log(Level level, const std::string &message) override {
    if (!IsLogEnable(level)) return;
    write(fileno(stdout), message.data(), message.length());
  }
};
}  // namespace webkit