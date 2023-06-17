#pragma once

#include <vector>

#include "webkit/logger.h"

namespace webkit {
class ChainLogger : public Logger {
 public:
  template <typename... Args>
  ChainLogger(Args... args) {
    Push(args...);
  }

  ~ChainLogger() = default;

  template <typename... Args>
  void Push(Logger *logger, Args... args) {
    logger_vec_.push_back(logger);
    Push(args...);
  }

  void Log(Level level, const std::string &message) override {
    if (!IsLogEnable(level)) return;
    for (Logger *logger : logger_vec_) logger->LogF(level, message);
  }

 private:
  void Push() {}

  std::vector<Logger *> logger_vec_;
};
}  // namespace webkit