#pragma once

#include <condition_variable>
#include <thread>
#include <vector>

#include "util/circular_queue.h"
#include "webkit/packet.h"
#include "webkit/pool.h"

namespace webkit {
class ThreadPool : public Pool {
 public:
  ThreadPool(uint32_t thread_num, size_t capacity);

  ~ThreadPool();

  void Run() override;

  void Stop() override;

  Status Submit(FuncType func) override;

 private:
  uint32_t thread_num_;
  std::vector<std::thread> thread_vec_;
  CircularQueue<FuncType> queue_;

  std::mutex run_mutex_;
  std::condition_variable run_cv_;

  std::mutex submit_mutex_;
  std::condition_variable submit_cv_;

  bool is_running_;
};

class ThreadPoolFactory : public PoolFactory {
 public:
  ThreadPoolFactory(uint32_t thread_num, size_t capacity);

  ~ThreadPoolFactory() = default;

  std::shared_ptr<Pool> Build() override;

 private:
  uint32_t thread_num_;
  size_t capacity_;
};
}  // namespace webkit