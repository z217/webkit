#include "syscall.h"

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <syslog.h>

#include "webkit/logger.h"

namespace webkit {
Syscall::SignalFuncType Syscall::Signal(int signo, SignalFuncType func) {
  struct sigaction act;
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
  struct sigaction oact;
  if (sigaction(signo, &act, &oact) < 0) {
    return SIG_ERR;
  }
  return oact.sa_handler;
}

Status Syscall::Deamon() {
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
  open("/dev/null", O_WRONLY);
  open("/dev/null", O_RDWR);

  return Status::OK();
}

Status Syscall::SetNonBlock(int fd) {
  int flags = fcntl(fd, F_GETFL, 0);
  if (flags < 0) {
    WEBKIT_LOGERROR("fcntl get fd flags error %d %s", errno, strerror(errno));
    return Status::Error(StatusCode::eFcntlError, "fcntl getfl error");
  }
  int ret = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
  if (ret < 0) {
    WEBKIT_LOGERROR("fcntl set fd flags error %d %s", errno, strerror(errno));
    return Status::Error(StatusCode::eFcntlError, "fcntl setfl error");
  }
  return Status::OK();
}
}  // namespace webkit