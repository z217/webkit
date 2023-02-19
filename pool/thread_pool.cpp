#include "thread_pool.h"

namespace webkit {
ThreadPool::ThreadPool(uint32_t thread_num, size_t capacity)
    : thread_num_(thread_num), queue_(capacity), is_running_(false) {}

ThreadPool::~ThreadPool() { Stop(); }

void ThreadPool::Run() {
  is_running_ = true;
  for (uint32_t idx = 0; idx < thread_num_; idx++) {
    thread_vec_.emplace_back([&] {
      while (is_running_) {
        if (queue_.IsEmpty()) {
          std::unique_lock<std::mutex> ul(run_mutex_);
          run_cv_.wait(ul, [&] { return !queue_.IsEmpty() || !is_running_; });
          ul.unlock();
        }
        if (!is_running_) break;
        FuncType func;
        Status s = queue_.Pop(func);
        if (!s.Ok()) continue;
        submit_cv_.notify_one();
        func();
      }
    });
  }
}

void ThreadPool::Stop() {
  is_running_ = false;
  submit_cv_.notify_all();
  run_cv_.notify_all();
  for (std::thread &t : thread_vec_) t.join();
  thread_vec_.clear();
}

Status ThreadPool::Submit(FuncType func) {
  if (queue_.IsFull()) {
    std::unique_lock<std::mutex> ul(submit_mutex_);
    submit_cv_.wait(ul, [&] { return !queue_.IsFull() || !is_running_; });
    ul.unlock();
  }
  if (!is_running_) {
    return Status::Error(StatusCode::ePoolStopped, "thread pool stopped");
  }
  Status s = queue_.Push(func);
  if (!s.Ok()) return s;
  run_cv_.notify_one();
  return Status::OK();
}

ThreadPoolFactory::ThreadPoolFactory(uint32_t thread_num, size_t capacity)
    : thread_num_(thread_num), capacity_(capacity) {}

std::shared_ptr<Pool> ThreadPoolFactory::Build() {
  return std::make_shared<ThreadPool>(thread_num_, capacity_);
}
}  // namespace webkit