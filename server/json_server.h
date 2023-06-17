#pragma once

#include <thread>

#include "util/circular_queue.h"
#include "webkit/event.h"
#include "webkit/pool.h"
#include "webkit/reactor.h"
#include "webkit/server_config.h"

namespace webkit {
class JsonServer {
 public:
  JsonServer(const ServerConfig *config);

  ~JsonServer();

  Status Init();

  Status Run();

  void Stop();

 private:
  void RunIo(uint32_t thread_id, std::shared_ptr<Reactor> reactor_sp);

  void RunAccept(std::vector<std::shared_ptr<Reactor>> reactor_sp_vec);

  Status FreeEvent(std::shared_ptr<Event> event_sp,
                   std::shared_ptr<Reactor> reactor_sp);

  const ServerConfig *config_;
  std::shared_ptr<Pool> worker_pool_;
  CircularQueue<std::shared_ptr<Event>> *event_queue_;
  CircularQueue<std::shared_ptr<Event>> *event_free_queue_;
  std::thread accept_thread_;
  std::vector<std::thread> io_thread_vec_;
  bool is_running_;
};
}  // namespace webkit