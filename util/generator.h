#pragma once

#include <cstring>
#include <ctime>
#include <random>
#include <string>

#include "third_party/fmt/include/fmt/printf.h"

namespace webkit {
class UidGenerator {
 private:
  UidGenerator() : host_id_(0) {}

 public:
  ~UidGenerator() = default;

  static UidGenerator *GetInstance() {
    static UidGenerator instance;
    return &instance;
  }

  void Init(const std::string &ip) {
    size_t len = ip.length();
    size_t buf_len = (len - 1) / 16 * 16;
    char *buf = new char[buf_len];
    memset(buf, 0, buf_len);
    memcpy(buf, ip.data(), ip.length());
    for (int seg = 0; seg * 16 < buf_len; seg++) {
      const uint32_t *u32_ptr = reinterpret_cast<uint32_t *>(buf) + seg;
      host_id_ ^= *u32_ptr;
    }
  }

  std::string Generate() const {
    static thread_local std::mt19937 rng(
        static_cast<uint32_t>(std::time(nullptr)));
    std::uniform_int_distribution distrib(0U,
                                          std::numeric_limits<uint32_t>::max());
    std::time_t now_time = std::time(nullptr);
    uint32_t random = distrib(rng) & 0xFFFF;
    return fmt::sprintf("%010d%05u%05u", now_time, host_id_, random);
  }

 private:
  uint32_t host_id_;
};
}  // namespace webkit