#pragma once

#include <functional>

#include "webkit/client_config.h"
#include "webkit/router.h"

namespace webkit {
template <typename KeyT, typename HashT = std::hash<KeyT>>
class HashRouter : public Router {
 public:
  HashRouter(ClientConfig *p_config, KeyT key, HashT hasher);

  HashRouter(ClientConfig *p_config, KeyT key);

  ~HashRouter() = default;

  virtual Status Route(std::string &ip, uint16_t &port);

 private:
  ClientConfig *p_config_;
  KeyT key_;
  HashT hasher_;
};

template <typename KeyT, typename HashT>
HashRouter<KeyT, HashT>::HashRouter(ClientConfig *p_config, KeyT key,
                                    HashT hasher)
    : p_config_(p_config), key_(key), hasher_(hasher) {}

template <typename KeyT, typename HashT>
HashRouter<KeyT, HashT>::HashRouter(ClientConfig *p_config, KeyT key)
    : p_config_(p_config), key_(key), hasher_(HashT{}) {}

template <typename KeyT, typename HashT>
Status HashRouter<KeyT, HashT>::Route(std::string &ip, uint16_t &port) {
  const std::vector<ClientConfig::Host> &host_vec = p_config_->GetHostVec();
  uint64_t hash = hasher_(key_);
  const ClientConfig::Host &host = host_vec[hash % host_vec.size()];
  ip = host.ip;
  port = host.port;
  return Status::OK();
}
}  // namespace webkit
