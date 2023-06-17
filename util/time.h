#pragma once

#include <cstring>
#include <ctime>
#include <string>

namespace webkit {
static const constexpr char kDefaultTimeStrFormat[] = "%Y-%m-%d %H:%M:%S";

class Time {
 private:
  Time() = default;

 public:
  ~Time() = default;

  static std::string GetTimeStr(
      time_t timestamp, const std::string &format = kDefaultTimeStrFormat) {
    static thread_local char buf[128];
    struct tm st_time;
    memset(&st_time, 0, sizeof(st_time));
    if (localtime_r(&timestamp, &st_time) == nullptr) return "";
    if (strftime(buf, sizeof(buf), format.c_str(), &st_time) == 0) return "";
    return buf;
  }

  static std::string GetNowTimeStr(
      const std::string &format = kDefaultTimeStrFormat) {
    return GetTimeStr(time(nullptr), format);
  }
};
}  // namespace webkit