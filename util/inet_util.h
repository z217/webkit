#pragma once

#include <arpa/inet.h>
#include <endian.h>
#include <sys/socket.h>

#include <string>
#include <type_traits>

#include "webkit/status.h"

namespace webkit {
class InetUtil {
 public:
  template <typename T>
  static T Ntoh(T v);

  template <typename T>
  static T Hton(T v);

  template <typename T>
  static Status InetNtop(T addr, std::string &ip);

  template <typename T>
  static Status InetPton(const std::string &ip, T *p_addr);

  template <typename T>
  static Status MakeSockAddr(const std::string &ip, uint16_t port, T *p_sin);
};

template <>
Status InetUtil::InetNtop<struct in_addr>(struct in_addr addr, std::string &ip);

template <>
Status InetUtil::InetNtop<in_addr_t>(in_addr_t addr, std::string &ip);

template <>
Status InetUtil::InetPton<struct in_addr>(const std::string &ip,
                                          struct in_addr *p_addr);

template <>
Status InetUtil::InetPton<in_addr_t>(const std::string &ip, in_addr_t *p_addr);

template <>
Status InetUtil::MakeSockAddr<struct sockaddr_in>(const std::string &ip,
                                                  uint16_t port,
                                                  struct sockaddr_in *p_sin);

template <typename T>
T InetUtil::Ntoh(T v) {
  static_assert(std::is_convertible_v<T, uint64_t> &&
                sizeof(T) <= sizeof(uint64_t));
  if constexpr (sizeof(T) <= sizeof(uint8_t)) {
    return v;
  }
  if constexpr (sizeof(T) <= sizeof(uint16_t)) {
    return static_cast<T>(be16toh(static_cast<uint16_t>(v)));
  }
  if constexpr (sizeof(T) <= sizeof(uint32_t)) {
    return static_cast<T>(be32toh(static_cast<uint32_t>(v)));
  }
  return static_cast<T>(be64toh(static_cast<uint64_t>(v)));
}

template <typename T>
T InetUtil::Hton(T v) {
  static_assert(std::is_convertible_v<T, uint64_t> &&
                sizeof(T) <= sizeof(uint64_t));
  if constexpr (sizeof(T) <= sizeof(uint8_t)) {
    return v;
  }
  if constexpr (sizeof(T) <= sizeof(uint16_t)) {
    return static_cast<T>(htobe16(static_cast<uint16_t>(v)));
  }
  if constexpr (sizeof(T) <= sizeof(uint32_t)) {
    return static_cast<T>(htobe32(static_cast<uint32_t>(v)));
  }
  return static_cast<T>(htobe64(static_cast<uint64_t>(v)));
}
}  // namespace webkit