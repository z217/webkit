#pragma once

#include <atomic>
#include <cassert>

#include "webkit/status.h"

namespace webkit {
template <typename T>
class CircularQueue {
 protected:
  enum class State {
    eDefault = 0,
    eEmpty = 1,
    eWriting = 2,
    eReading = 3,
    eOk = 4,
  };

 public:
  CircularQueue(size_t capacity);

  virtual ~CircularQueue();

  virtual Status Push(const T &value);

  virtual Status Pop(T &data);

  size_t Capacity() const;

  size_t Size() const;

  bool IsFull() const;

  bool IsEmpty() const;

 private:
  State *state_buffer_;
  T *data_buffer_;
  size_t capacity_;
  std::atomic<size_t> head_;
  std::atomic<size_t> tail_;
};

template <typename T>
CircularQueue<T>::CircularQueue(size_t capacity)
    : state_buffer_(new State[capacity]),
      data_buffer_(new T[capacity]),
      capacity_(capacity),
      head_(0),
      tail_(0) {}

template <typename T>
CircularQueue<T>::~CircularQueue() {
  delete[] state_buffer_;
  delete[] data_buffer_;
}

template <typename T>
Status CircularQueue<T>::Push(const T &value) {
  size_t idx;
  while (true) {
    idx = tail_.load(std::memory_order_acquire);
    if (IsFull()) {
      return Status::Error(StatusCode::eCircularQueueFull,
                           "circular queue is full");
    }
    if (tail_.compare_exchange_strong(idx, (idx + 1) % capacity_,
                                      std::memory_order_release,
                                      std::memory_order_acquire)) {
      idx = (idx + 1) % capacity_;
      break;
    }
  }
  state_buffer_[idx] = State::eWriting;
  assert(state_buffer_[idx] == State::eWriting);
  data_buffer_[idx] = value;
  assert(state_buffer_[idx] == State::eWriting);
  state_buffer_[idx] = State::eOk;
  return Status::OK();
}

template <typename T>
Status CircularQueue<T>::Pop(T &data) {
  size_t idx;
  while (true) {
    idx = head_.load(std::memory_order_acquire);
    if (IsEmpty()) {
      return Status::Error(StatusCode::eCircularQueueEmpty,
                           "circular queue is empty");
    }
    if (head_.compare_exchange_strong(idx, (idx + 1) % capacity_,
                                      std::memory_order_release,
                                      std::memory_order_acquire)) {
      idx = (idx + 1) % capacity_;
      break;
    }
  }
  state_buffer_[idx] = State::eReading;
  assert(state_buffer_[idx] == State::eReading);
  data = data_buffer_[idx];
  assert(state_buffer_[idx] == State::eReading);
  state_buffer_[idx] = State::eEmpty;
  return Status::OK();
}

template <typename T>
size_t CircularQueue<T>::Capacity() const {
  return capacity_;
}

template <typename T>
size_t CircularQueue<T>::Size() const {
  size_t head = head_.load(std::memory_order_acquire);
  size_t tail = tail_.load(std::memory_order_acquire);
  if (head < tail) return tail - head;
  if (tail > head) return head + capacity_ - tail;
  if (state_buffer_[tail] == State::eOk) {
    return capacity_;
  }
  return 0;
}

template <typename T>
bool CircularQueue<T>::IsFull() const {
  return Size() == Capacity();
}

template <typename T>
bool CircularQueue<T>::IsEmpty() const {
  return Size() == 0;
}
}  // namespace webkit