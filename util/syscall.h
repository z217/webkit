#pragma once

#include <fcntl.h>
#include <signal.h>
#include <syslog.h>

#include "webkit/status.h"

namespace webkit {
using SigFuncType = void(int);

SigFuncType *Signal(int signo, SigFuncType *func) {
  struct sigaction act, oact;
  act.sa_handler = func;
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;
  if (signo == SIGALRM) {
#ifdef SA_INERRUPT
    act.sa_flags |= SA_INTERRUPT;
#endif
  } else {
#ifndef SA_RESTART
    act.sa_flag |= SA_RESTART;
#endif
  }
  if (sigaction(signo, &act, &oact) < 0) {
    return SIG_ERR;
  }
  return oact.sa_handler;
}

Status Deamon() {
  pid_t pid = fork();
  if (pid < 0) {
    return Status::FatalF(StatusCode::eSyscallDeamonError, "fork error %d %s",
                          errno, strerror(errno));
  }
  if (pid > 0) _exit(0);

  if (setsid() < 0) {
    return Status::FatalF(StatusCode::eSyscallDeamonError, "setsid error %d %s",
                          errno, strerror(errno));
  }

  Signal(SIGHUP, SIG_IGN);

  pid = fork();
  if (pid < 0) {
    return Status::FatalF(StatusCode::eSyscallDeamonError,
                          "fork twice error %d %s", errno, strerror(errno));
  }
  if (pid > 0) _exit(0);

  chdir("/");

  // Close the first 64 descriptors,
  // ensuring that there are no open descriptors
  for (int fd = 0; fd < 64; fd++) {
    close(fd);
  }
  // redirect stdin, stdout and stderr to /dev/null
  open("/dev/null", O_RDONLY);
  open("/dev/null", O_RDWR);
  open("/dev/null", O_RDWR);

  return Status::OK();
}
}  // namespace webkit