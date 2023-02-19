#pragma once

#include <functional>

#include "webkit/class_factory.h"
#include "webkit/status.h"

namespace webkit {
class Pool {
 public:
  using FuncType = std::function<void()>;

  Pool() = default;

  virtual ~Pool() = default;

  virtual void Run() = 0;

  virtual void Stop() = 0;

  virtual Status Submit(FuncType func) = 0;
};

using PoolFactory = ClassFactory<Pool>;
}  // namespace webkit