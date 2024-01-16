#include "inet_util.h"

namespace webkit {
template <>
Status InetUtil::InetNtop<struct in_addr>(struct in_addr st_addr,
                                          std::string &ip) {
  return InetNtop(st_addr.s_addr, ip);
}

template <>
Status InetUtil::InetNtop<in_addr_t>(in_addr_t addr, std::string &ip) {
  static thread_local char buffer[INET_ADDRSTRLEN];
  if (inet_ntop(AF_INET, &addr, buffer, INET_ADDRSTRLEN) == nullptr) {
    return Status::Error(StatusCode::eParamError, "ip pattern error");
  }
  ip.assign(buffer, INET_ADDRSTRLEN);
  return Status::OK();
}

template <>
Status InetUtil::InetPton<struct in_addr>(const std::string &ip,
                                          struct in_addr *p_st_addr) {
  return InetPton(ip, &p_st_addr->s_addr);
}

template <>
Status InetUtil::InetPton<in_addr_t>(const std::string &ip, in_addr_t *p_addr) {
  int ret = inet_pton(AF_INET, ip.c_str(), p_addr);
  if (ret != 1) {
    return Status::Error(StatusCode::eParamError, "ip pattern error");
  }
  return Status::OK();
}

template <>
Status InetUtil::MakeSockAddr<struct sockaddr_in>(const std::string &ip,
                                                  uint16_t port,
                                                  struct sockaddr_in *p_sin) {
  memset(p_sin, 0, sizeof(struct sockaddr_in));
  p_sin->sin_family = AF_INET;
  p_sin->sin_port = Hton(port);
  Status s = InetPton(ip, &p_sin->sin_addr);
  if (!s.Ok()) return s;
  return Status::OK();
}
}  // namespace webkit