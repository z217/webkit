#pragma once

#include <atomic>
#include <cassert>

#include "webkit/status.h"

namespace webkit {
template <typename T>
class CircularQueue {
 public:
  CircularQueue(size_t capacity);

  ~CircularQueue();

  virtual Status Push(const T &value);

  virtual Status Pop(T &data);

  size_t Capacity() const;

  size_t Size() const;

  bool IsFull() const;

  bool IsEmpty() const;

 private:
  T *buffer_;
  size_t capacity_;
  size_t size_;
  size_t write_pos_;
  size_t read_pos_;
  std::atomic<size_t> next_write_pos_;
  std::atomic<size_t> next_read_pos_;
};

template <typename T>
CircularQueue<T>::CircularQueue(size_t capacity)
    : buffer_(new T[capacity]),
      capacity_(capacity),
      size_(0),
      write_pos_(0),
      read_pos_(capacity),
      next_write_pos_(0),
      next_read_pos_(capacity) {}

template <typename T>
CircularQueue<T>::~CircularQueue() {
  delete[] buffer_;
}

template <typename T>
Status CircularQueue<T>::Push(const T &value) {
  if (IsFull()) {
    return Status::Error(StatusCode::eCircularQueueFull,
                         "circular queue is full");
  }
  size_t next_write_pos = next_write_pos_.load();
  if (next_write_pos != write_pos_ ||
      !next_write_pos_.compare_exchange_strong(
          next_write_pos, (next_write_pos + 1) % capacity_)) {
    return Status::Error(StatusCode::eCircularConcurFail,
                         "circular queue is being written by other");
  }
  next_write_pos = next_write_pos_.load();
  assert((write_pos_ + 1) % capacity_ == next_write_pos);
  buffer_[next_write_pos] = value;
  write_pos_ = next_write_pos;
  size_++;
  return Status::OK();
}

template <typename T>
Status CircularQueue<T>::Pop(T &data) {
  if (IsEmpty()) {
    return Status::Error(StatusCode::eCircularQueueEmpty,
                         "circular queue is empty");
  }
  size_t next_read_pos = next_read_pos_.load();
  if (next_read_pos != read_pos_ ||
      !next_read_pos_.compare_exchange_strong(
          next_read_pos, (next_read_pos + 1) % capacity_)) {
    return Status::Error(StatusCode::eCircularConcurFail,
                         "circular queue is being written by other");
  }
  next_read_pos = next_read_pos_.load();
  assert((read_pos_ + 1) % capacity_ == next_read_pos);
  data = buffer_[next_read_pos];
  read_pos_ = next_read_pos;
  size_--;
  return Status::OK();
}

template <typename T>
size_t CircularQueue<T>::Capacity() const {
  return capacity_;
}

template <typename T>
size_t CircularQueue<T>::Size() const {
  return size_;
}

template <typename T>
bool CircularQueue<T>::IsFull() const {
  return capacity_ == size_;
}

template <typename T>
bool CircularQueue<T>::IsEmpty() const {
  return size_ == 0;
}
}  // namespace webkit