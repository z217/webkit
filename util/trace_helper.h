#pragma once

#include <mutex>

#include "webkit/status.h"

namespace webkit {
class TraceHelper {
 public:
  TraceHelper() = default;

  ~TraceHelper() = default;

  static TraceHelper *GetInstance() {
    static thread_local TraceHelper instance;
    return &instance;
  }

  void SetTraceId(const std::string &trace_id) { trace_id_ = trace_id; }

  std::string GetTraceId() { return trace_id_; }

  void ClearTraceId() { trace_id_.clear(); }

 private:
  std::string trace_id_;
};
}  // namespace webkit