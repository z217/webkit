#pragma once

#include "webkit/status.h"

namespace webkit {

class Syscall {
 public:
  using SignalFuncType = void (*)(int);

  static SignalFuncType Signal(int signo, SignalFuncType func);

  static Status Deamon();

  static Status SetNonBlock(int fd);
};
}  // namespace webkit