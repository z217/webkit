#include "tcp_socket.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <unistd.h>

#include <memory>
#include <vector>

#include "util/inet_util.h"
#include "util/syscall.h"
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
  Status s = InetUtil::MakeSockAddr(ip, port, &sin);
  if (!s.Ok()) {
    WEBKIT_LOGERROR("make sockaddr error status code %d message %s", s.Code(),
                    s.Message());
    return s;
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

  int reuseport = 1;
  int ret = setsockopt(fd_, SOL_SOCKET, SO_REUSEPORT, &reuseport, sizeof(int));
  if (ret < 0) {
    WEBKIT_LOGERROR("tcp socket set SO_REUSEPORT error %d %s", errno,
                    strerror(errno));
    return Status::Error(StatusCode::eSocketOptError,
                         "socket set sock opt error");
  }

  struct sockaddr_in sin;
  Status s = InetUtil::MakeSockAddr(ip, port, &sin);
  if (!s.Ok()) {
    WEBKIT_LOGERROR("make sockaddr error status code %d message %s", s.Code(),
                    s.Message());
    return s;
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

  struct sockaddr_in sin;
  memset(&sin, 0, sizeof(sin));
  socklen_t sin_len = sizeof(sin);
  socket->fd_ =
      accept(fd_, reinterpret_cast<struct sockaddr *>(&sin), &sin_len);
  if (socket->fd_ < 0) {
    if (errno == EINTR || errno == EAGAIN) {
      return Status::Warn(StatusCode::eRetry);
    }
    WEBKIT_LOGERROR("tcp socket accept error %d %s", errno, strerror(errno));
    return Status::Error(StatusCode::eSocketAcceptError, "socket accept error");
  }

  Status s = InetUtil::InetNtop(sin.sin_addr, socket->ip_);
  if (!s.Ok()) {
    WEBKIT_LOGERROR("trans ip addr error %d %s", errno, strerror(errno));
    return s;
  }
  socket->port_ = InetUtil::Ntoh(sin.sin_port);
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
  buffer_.clear();
  return Status::OK();
}

Status TcpSocket::Write(const void *data, size_t data_size,
                        size_t &write_size) {
  if (!is_connected_) {
    WEBKIT_LOGERROR("tcp socket disconnected");
    return Status::Error(StatusCode::eSocketDisonnected, "socket disconnected");
  }
  write_size = 0;
  Status s = WriteFromBuffer(data_size, write_size);
  if (!s.Ok()) return s;
  while (write_size < data_size) {
    ssize_t nwrite = write(fd_, data, data_size);
    if (nwrite < 0) {
      if (errno == EINTR) continue;
      if (errno == EAGAIN) {
        return Status::Warn(StatusCode::eRetry);
      }
      WEBKIT_LOGERROR("tcp socket write error %d %s", errno, strerror(errno));
      return Status::Error(StatusCode::eSocketWriteError, "socket write error");
    }
    if (nwrite == 0) {  // no log error if peer closed
      return Status::Error(StatusCode::eSocketPeerClosed, "socket peer closed");
    }
    write_size += nwrite;
  }
  return Status::OK();
}

Status TcpSocket::Write(IoBase &src, size_t src_size, size_t &write_size) {
  if (!is_connected_) {
    WEBKIT_LOGERROR("tcp socket disconnected");
    return Status::Error(StatusCode::eSocketDisonnected, "socket disconnected");
  }
  write_size = 0;

  if (buffer_.size() < src_size) {
    size_t write_pos = buffer_.size();
    buffer_.resize(src_size);
    size_t read_size = 0;
    Status s = src.Read(&buffer_[write_pos], src_size - write_pos, read_size);
    if (!s.Ok() || write_pos + read_size != src_size) {
      WEBKIT_LOGERROR("read src expect %zu get %zu status code %d message %s",
                      src_size - write_pos, read_size, s.Code(), s.Message());
      return s;
    }
  }

  Status s = WriteFromBuffer(src_size, write_size);
  if (!s.Ok()) return s;

  return Status::OK();
}

Status TcpSocket::Read(void *dst, size_t dst_size, size_t &read_size) {
  if (!is_connected_) {
    WEBKIT_LOGERROR("tcp socket disconnected");
    return Status::Error(StatusCode::eSocketDisonnected, "socket disconnected");
  }
  read_size = 0;
  if (buffer_.size() != 0) {
    size_t data_size = std::min(buffer_.size(), dst_size);
    memcpy(dst, &buffer_[0], data_size);
    BufferPop(data_size);
    read_size = data_size;
  }

  while (read_size < dst_size) {
    ssize_t nread = read(fd_, reinterpret_cast<uint8_t *>(dst) + read_size,
                         dst_size - read_size);
    if (nread < 0) {
      if (errno == EINTR) continue;
      if (errno == EAGAIN) {
        return Status::Warn(StatusCode::eNoData);
      }
      WEBKIT_LOGERROR("tcp socket read error %d %s", errno, strerror(errno));
      return Status::Error(StatusCode::eSocketReadError, "socket read error");
    }
    if (nread == 0) {  // no log error if peer closed
      return Status::Error(StatusCode::eSocketPeerClosed, "socket peer closed");
    }
    read_size += nread;
  }

  return Status::OK();
}

Status TcpSocket::Read(IoBase &dst, size_t dst_size, size_t &read_size) {
  if (!is_connected_) {
    WEBKIT_LOGERROR("tcp socket disconnected");
    return Status::Error(StatusCode::eSocketDisonnected, "socket disconnected");
  }
  read_size = 0;

  if (buffer_.size() < dst_size) {
    size_t data_size = dst_size - buffer_.size();
    size_t write_size = 0;
    Status s = ReadToBuffer(data_size, write_size);
    if (!s.Ok()) return s;
  }

  Status s = dst.Write(&buffer_[0], dst_size, read_size);
  BufferPop(read_size);
  if (!s.Ok() || dst_size != read_size) {
    WEBKIT_LOGERROR("write dst expect %zu get %zu status code %d message %s",
                    dst_size, read_size, s.Code(), s.Message());
    return s;
  }

  return Status::OK();
}

Status TcpSocket::SetNonBlock() {
  if (!is_connected_) {
    WEBKIT_LOGERROR("tcp socket disconnected");
    return Status::Error(StatusCode::eSocketDisonnected, "socket disconnected");
  }
  Status s = Syscall::SetNonBlock(fd_);
  if (!s.Ok()) {
    WEBKIT_LOGERROR("set non block error status code %d message %s", s.Code(),
                    s.Message());
    return s;
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

  return Status::OK();
}

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

int TcpSocket::GetFd() const { return fd_; }

bool TcpSocket::IsConnected() const { return is_connected_; }

const std::string &TcpSocket::GetIp() const { return ip_; }

uint16_t TcpSocket::GetPort() const { return port_; }

Status TcpSocket::ReadToBuffer(size_t data_size, size_t &read_size) {
  size_t write_pos = buffer_.size();
  buffer_.resize(write_pos + data_size);

  read_size = 0;
  while (read_size < data_size) {
    ssize_t nread = read(fd_, &buffer_[write_pos], data_size - read_size);
    if (nread < 0) {
      if (errno == EINTR) continue;
      if (errno == EAGAIN) {
        return Status::Warn(StatusCode::eNoData);
      }
      WEBKIT_LOGERROR("tcp socket read error %d %s", errno, strerror(errno));
      return Status::Error(StatusCode::eSocketReadError, "socket read error");
    }
    if (nread == 0) {
      return Status::Error(StatusCode::eSocketPeerClosed, "socket peer closed");
    }
    read_size += nread;
  }

  return Status::OK();
}

Status TcpSocket::WriteFromBuffer(size_t data_size, size_t &write_size) {
  size_t read_size = std::min(buffer_.size(), data_size);

  write_size = 0;
  Status s = Status::OK();
  while (write_size < read_size) {
    ssize_t nwrite = write(fd_, &buffer_[write_size], read_size - write_size);
    if (nwrite < 0) {
      if (errno == EINTR) continue;
      if (errno == EAGAIN) {
        s = Status::Warn(StatusCode::eRetry);
        break;
      }
      WEBKIT_LOGERROR("tcp socket write error %d %s", errno, strerror(errno));
      s = Status::Error(StatusCode::eSocketWriteError, "socket write error");
      break;
    }
    if (nwrite == 0) {
      s = Status::Error(StatusCode::eSocketPeerClosed, "socket peer closed");
      break;
    }
    write_size += nwrite;
  }

  BufferPop(write_size);
  return s;
}

void TcpSocket::BufferPop(size_t size) {
  size_t new_size = buffer_.size() - size;
  memmove(&buffer_[0], &buffer_[size], new_size);
  buffer_.resize(new_size);
}
}  // namespace webkit
