#pragma once

#include <functional>

#include "util/time.h"
#include "util/trace_helper.h"
#include "webkit/logger.h"

namespace webkit {
class CallbackLogger : public Logger {
 public:
  using CallbackType = std::function<std::string()>;

  CallbackLogger(CallbackType prefix_cb = DefaultPrefixCallback,
                 CallbackType suffix_cb = DefaultSuffixCallback)
      : prefix_cb_(prefix_cb), suffix_cb_(suffix_cb) {}

  ~CallbackLogger() = default;

  virtual std::string GetPrefix() override { return prefix_cb_(); }

  virtual std::string GetSuffix() override { return suffix_cb_(); }

  static std::string DefaultPrefixCallback() {
    return Time::GetNowTimeStr() + " " +
           TraceHelper::GetInstance()->GetTraceId() + " ";
  }

  static std::string DefaultSuffixCallback() { return ""; }

 private:
  CallbackType prefix_cb_;
  CallbackType suffix_cb_;
};
}  // namespace webkit
