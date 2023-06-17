#pragma once

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "logger/callback_logger.h"

namespace webkit {
class CommLogger : public CallbackLogger {
 private:
  CommLogger(CallbackType prefix_cb, CallbackType suffix_cb)
      : CallbackLogger(prefix_cb, suffix_cb) {}

 public:
  ~CommLogger() {
    if (fd_ >= 0) close(fd_);
  }

  static std::unique_ptr<CommLogger> Open(
      const std::string &path, CallbackType prefix_cb = DefaultPrefixCallback,
      CallbackType suffix_cb = DefaultSuffixCallback) {
    std::unique_ptr<CommLogger> logger_up(new CommLogger(prefix_cb, suffix_cb));

    constexpr mode_t mode =
        S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;
    {
      const char *end = strrchr(path.c_str(), '/');
      if (end != nullptr) {
        std::string dir(path.c_str(), end);
        if (access(dir.c_str(), F_OK) != 0) {
          mkdir(dir.c_str(), mode);
        }
      }
    }

    logger_up->fd_ = open(path.c_str(), O_WRONLY | O_APPEND | O_CREAT, mode);
    if (logger_up->fd_ < 0) return nullptr;
    return logger_up;
  }

  void Log(Level level, const std::string &message) override {
    if (!IsLogEnable(level)) return;
    write(fd_, message.data(), message.length());
  }

 private:
  int fd_;
};
}  // namespace webkit