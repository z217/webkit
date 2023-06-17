#include "tcp_socket.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <unistd.h>

#include <memory>

#include "webkit/logger.h"

namespace webkit {
TcpSocket::TcpSocket() : fd_(-1), ip_(""), port_(0), is_connected_(false) {}

TcpSocket::TcpSocket(int fd, const std::string &ip, uint16_t port)
    : fd_(fd), ip_(ip), port_(port), is_connected_(true) {}

TcpSocket::~TcpSocket() {
  if (is_connected_) Close();
}

Status TcpSocket::Connect(const std::string &ip, uint16_t port) {
  if (is_connected_) {
    WEBKIT_LOGERROR("tcp socket already connected %s:%u", ip, port);
    return Status::Error(StatusCode::eSocketConnected,
                         "socket already connected");
  }

  fd_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (fd_ < 0) {
    WEBKIT_LOGERROR("tcp socket create error %d %s", errno, strerror(errno));
    return Status::Error(StatusCode::eSocketCreateError, "socket create error");
  }

  int ret = 0;
  struct sockaddr_in sin;
  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_port = htons(port);
  ret = inet_pton(AF_INET, ip.c_str(), &sin.sin_addr);
  if (ret < 1) {
    WEBKIT_LOGERROR("tcp socket ip %s pattern error", ip);
    return Status::Error(StatusCode::eParamError, "socket ip pattern error");
  }

  ret = connect(fd_, reinterpret_cast<struct sockaddr *>(&sin), sizeof(sin));
  if (ret < 0) {
    WEBKIT_LOGERROR("tcp socket connect error %d %s", errno, strerror(errno));
    return Status::Error(StatusCode::eSocketConnectError,
                         "socket connect error");
  }

  is_connected_ = true;
  return SetTimeout();
}

Status TcpSocket::Listen(const std::string &ip, uint16_t port) {
  if (is_connected_) {
    WEBKIT_LOGERROR("tcp socket already connected %s:%u", ip, port);
    return Status::Error(StatusCode::eSocketConnected,
                         "socket already connected");
  }

  fd_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (fd_ < 0) {
    WEBKIT_LOGERROR("tcp socket create error %d %s", errno, strerror(errno));
    return Status::Error(StatusCode::eSocketCreateError,
                         "socket connect error");
  }

  int ret = 0;
  sockaddr_in sin;
  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_port = htons(port);
  ret = inet_pton(AF_INET, ip.c_str(), &sin.sin_addr);
  if (ret < 1) {
    WEBKIT_LOGERROR("tcp socket ip %s pattern error", ip);
    return Status::Error(StatusCode::eParamError, "socket ip pattern error");
  }

  ret = bind(fd_, reinterpret_cast<sockaddr *>(&sin), sizeof(sin));
  if (ret < 0) {
    WEBKIT_LOGERROR("tcp socket bind error %d %s", errno, strerror(errno));
    return Status::Error(StatusCode::eSocketBindError, "socket bind error");
  }

  ret = listen(fd_, 1024);
  if (ret < 0) {
    WEBKIT_LOGERROR("tcp socket listen error %d %s", errno, strerror(errno));
    return Status::Error(StatusCode::eSocketListenError, "socket listen error");
  }

  is_connected_ = true;
  return SetTimeout();
}

Status TcpSocket::Accept(TcpSocket *socket) {
  if (!is_connected_) {
    WEBKIT_LOGERROR("tcp socket disconnected");
    return Status::Error(StatusCode::eSocketDisonnected, "socket disconnected");
  }

  sockaddr_in sin;
  memset(&sin, 0, sizeof(sin));
  socklen_t sin_len = sizeof(sin);
  socket->fd_ = accept(fd_, reinterpret_cast<sockaddr *>(&sin), &sin_len);
  if (socket->fd_ < 0) {
    if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK) {
      return Status::Warn(StatusCode::eRetry);
    }
    WEBKIT_LOGERROR("tcp socket accept error %d %s", errno, strerror(errno));
    return Status::Error(StatusCode::eSocketAcceptError, "socket accept error");
  }

  {
    static thread_local char buffer[INET_ADDRSTRLEN];
    if (inet_ntop(AF_INET, &sin.sin_addr, buffer, INET_ADDRSTRLEN) == nullptr) {
      WEBKIT_LOGERROR("trans ip addr error %d %s", errno, strerror(errno));
      return Status::Error(StatusCode::eSocketAcceptError,
                           "socket accept error");
    }
    socket->ip_.assign(buffer, INET_ADDRSTRLEN);
  }
  socket->port_ = ntohs(sin.sin_port);
  socket->is_connected_ = true;
  return Status::OK();
}

Status TcpSocket::Close() {
  if (!is_connected_) {
    WEBKIT_LOGFATAL("tcp socket disconnected");
    return Status::Error(StatusCode::eSocketDisonnected, "socket disconnected");
  }
  int ret = close(fd_);
  if (ret != 0) {
    WEBKIT_LOGERROR("tcp socket close error %d %s", errno, strerror(errno));
    return Status::Error(StatusCode::eSocketCloseError, "socket close error");
  }
  fd_ = -1;
  ip_ = "";
  port_ = 0;
  is_connected_ = false;
  return Status::OK();
}

Status TcpSocket::Write(const void *data, size_t data_size,
                        size_t &write_size) {
  if (!is_connected_) {
    WEBKIT_LOGERROR("tcp socket disconnected");
    return Status::Error(StatusCode::eSocketDisonnected, "socket disconnected");
  }
  while (true) {
    ssize_t nwrite = write(fd_, data, data_size);
    if (nwrite < 0) {
      if (errno == EINTR) continue;
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        return Status::Warn(StatusCode::eNoData);
      }
      WEBKIT_LOGERROR("tcp socket write error %d %s", errno, strerror(errno));
      return Status::Error(StatusCode::eSocketWriteError, "socket write error");
    }
    write_size = nwrite;
    if (nwrite == 0) {  // no log error if peer closed
      return Status::Error(StatusCode::eSocketPeerClosed, "socket peer closed");
    }
    break;
  }
  return Status::OK();
}

Status TcpSocket::Read(void *buffer, size_t buffer_size, size_t &read_size) {
  if (!is_connected_) {
    WEBKIT_LOGERROR("tcp socket disconnected");
    return Status::Error(StatusCode::eSocketDisonnected, "socket disconnected");
  }
  while (true) {
    ssize_t nread = read(fd_, buffer, buffer_size);
    if (nread < 0) {
      if (errno == EINTR) continue;
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        return Status::Warn(StatusCode::eNoData);
      }
      WEBKIT_LOGERROR("tcp socket read error %d %s", errno, strerror(errno));
      return Status::Error(StatusCode::eSocketReadError, "socket read error");
    }
    read_size = nread;
    if (nread == 0) {  // no log error if peer closed
      return Status::Error(StatusCode::eSocketPeerClosed, "socket peer closed");
    }
    break;
  }
  return Status::OK();
}

Status TcpSocket::SetNonBlock() {
  if (!is_connected_) {
    WEBKIT_LOGERROR("tcp socket disconnected");
    return Status::Error(StatusCode::eSocketDisonnected, "socket disconnected");
  }
  int flags = fcntl(fd_, F_GETFL, 0);
  if (flags < 0) {
    WEBKIT_LOGERROR("tcp socket set non block get file flags error %d %s",
                    errno, strerror(errno));
    return Status::Error(StatusCode::eSocketFcntlError,
                         "socket set non block error");
  }
  int ret = fcntl(fd_, F_SETFL, flags | O_NONBLOCK);
  if (ret < 0) {
    WEBKIT_LOGERROR("tcp socket set non block set file flags error %d %s",
                    errno, strerror(errno));
    return Status::Error(StatusCode::eSocketFcntlError,
                         "socket set non block error");
  }
  return Status::OK();
}

Status TcpSocket::SetTimeout(suseconds_t timeout_sec, suseconds_t timeout_us) {
  if (!is_connected_) {
    WEBKIT_LOGERROR("tcp socket disconnected");
    return Status::Error(StatusCode::eSocketDisonnected, "socket disconnected");
  }

  int ret = 0;
  struct timeval timeout;
  timeout.tv_sec = timeout_sec;
  timeout.tv_usec = timeout_us;

  ret = setsockopt(fd_, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
  if (ret != 0) {
    WEBKIT_LOGERROR("tcp socket set recv timeout error %d %s", errno,
                    strerror(errno));
    return Status::Error(StatusCode::eSocketOptError,
                         "socket set sock opt error");
  }

  ret = setsockopt(fd_, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
  if (ret != 0) {
    WEBKIT_LOGERROR("tcp socket set send timeout error %d %s", errno,
                    strerror(errno));
    return Status::Error(StatusCode::eSocketOptError,
                         "socket set sock ope error");
  }

  return SetLinger(true, timeout_sec);
}

int TcpSocket::GetFd() { return fd_; }

bool TcpSocket::IsConnected() const { return is_connected_; }

const std::string &TcpSocket::GetIp() const { return ip_; }

uint16_t TcpSocket::GetPort() const { return port_; }

Status TcpSocket::SetLinger(bool is_on, int linger_sec) {
  if (!is_connected_) {
    WEBKIT_LOGERROR("tcp socket disconnected");
    return Status::Error(StatusCode::eSocketDisonnected, "socket disconnected");
  }
  struct linger linger_opt;
  linger_opt.l_onoff = is_on;
  linger_opt.l_linger = linger_sec;
  int ret =
      setsockopt(fd_, SOL_SOCKET, SO_LINGER, &linger_opt, sizeof(linger_opt));
  if (ret != 0) {
    WEBKIT_LOGERROR("tcp socket set sock opt error %d %s", errno,
                    strerror(errno));
    return Status::Error(StatusCode::eSocketOptError,
                         "socket set sock opt error");
  }
  return Status::OK();
}
}  // namespace webkit
