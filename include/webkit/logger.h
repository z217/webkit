#pragma once

#include <string>

#include "third_party/fmt/include/fmt/printf.h"
#include "webkit/instance_base.h"

#define WEBKIT_LOG(LEVEL, FORMAT, ...)                                        \
  do {                                                                        \
    if (!webkit::Logger::IsLogEnable((LEVEL))) break;                         \
    const char *file_ptr = strrchr(__FILE__, '/');                            \
    if (file_ptr == nullptr) {                                                \
      file_ptr = __FILE__;                                                    \
    } else {                                                                  \
      file_ptr++;                                                             \
    }                                                                         \
    webkit::Logger::GetDefaultInstance()->LogF(                               \
        (LEVEL), fmt::sprintf("%s %s\n", "[%s] %s:%d(%s)", (FORMAT)),         \
        webkit::Logger::LevelToString((LEVEL)), file_ptr, __LINE__, __func__, \
        ##__VA_ARGS__);                                                       \
  } while (false)

#define WEBKIT_LOGDEBUG(FORMAT, ...) \
  WEBKIT_LOG(webkit::Logger::eDebug, (FORMAT), ##__VA_ARGS__)

#define WEBKIT_LOGINFO(FORMAT, ...) \
  WEBKIT_LOG(webkit::Logger::eInfo, (FORMAT), ##__VA_ARGS__)

#define WEBKIT_LOGWARN(FORMAT, ...) \
  WEBKIT_LOG(webkit::Logger::eWarn, (FORMAT), ##__VA_ARGS__)

#define WEBKIT_LOGERROR(FORMAT, ...) \
  WEBKIT_LOG(webkit::Logger::eError, (FORMAT), ##__VA_ARGS__)

#define WEBKIT_LOGFATAL(FORMAT, ...) \
  WEBKIT_LOG(webkit::Logger::eFatal, (FORMAT), ##__VA_ARGS__)

namespace webkit {
class Logger : public InstanceBase<Logger> {
 public:
  enum Level {
    eDebug = 1,
    eInfo = 2,
    eWarn = 3,
    eError = 4,
    eFatal = 5,
  };

 public:
  Logger() : level_(eDebug) {}

  virtual ~Logger() = default;

  virtual void Log(Level level, const std::string &message) = 0;

  template <typename... Args>
  void LogF(Level level, const std::string &format, Args &&...args) {
    using namespace std::string_literals;
    if (!IsLogEnable(level)) return;
    Log(level, fmt::sprintf("%s"s + format + "%s"s, GetPrefix(),
                            std::forward<Args>(args)..., GetSuffix()));
  }

  virtual std::string GetPrefix() { return ""; }

  virtual std::string GetSuffix() { return ""; }

  void SetLevel(Level level) { level_ = level; }

  Level GetLevel() const { return level_; }

  static bool IsLogEnable(Level level) {
    return GetDefaultInstance()->GetLevel() <= level;
  }

  static void SetLogLevel(Level level) {
    GetDefaultInstance()->SetLevel(level);
  }

  static void Debug(const std::string &message) {
    GetDefaultInstance()->Log(eDebug, message);
  }

  template <typename... Args>
  static void DebugF(const std::string &format, Args &&...args) {
    GetDefaultInstance()->LogF(eDebug, format, std::forward<Args>(args)...);
  }

  static void Info(const std::string &message) {
    GetDefaultInstance()->Log(eInfo, message);
  }

  template <typename... Args>
  static void InfoF(const std::string &format, Args &&...args) {
    GetDefaultInstance()->LogF(eInfo, format, std::forward<Args>(args)...);
  }

  static void Warn(const std::string &message) {
    GetDefaultInstance()->Log(eWarn, message);
  }

  template <typename... Args>
  static void WarnF(const std::string &format, Args &&...args) {
    GetDefaultInstance()->Log(eWarn, format, std::forward<Args>(args)...);
  }

  static void Error(const std::string &message) {
    GetDefaultInstance()->Log(eError, message);
  }

  template <typename... Args>
  static void ErrorF(const std::string &format, Args &&...args) {
    GetDefaultInstance()->LogF(eError, format, std::forward<Args>(args)...);
  }

  static void Fatal(const std::string &message) {
    GetDefaultInstance()->Log(eFatal, message);
  }

  template <typename... Args>
  static void FatalF(const std::string &format, Args &&...args) {
    GetDefaultInstance()->LogF(eFatal, format, std::forward<Args>(args)...);
  }

  static std::string LevelToString(Level level) {
    static constexpr const char *level_strings[]{"",
                                                 "\x1B[32mDebug\x1B[0m",
                                                 "\x1B[32mInfo\x1B[0m",
                                                 "\x1B[33mWarn\x1B[0m",
                                                 "\x1B[31mError\x1B[0m",
                                                 "\x1B[31mFatal\x1B[0m"};
    return level_strings[level];
  }

 protected:
  Level level_;
};
}  // namespace webkit