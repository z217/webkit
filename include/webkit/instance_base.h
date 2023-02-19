#pragma once

namespace webkit {
template <typename T>
class InstanceBase {
 private:
  class InstanceHelper {
   public:
    InstanceHelper() : ptr_(nullptr) {}

    static InstanceHelper *GetInstance() {
      static InstanceHelper helper;
      return &helper;
    }

    void Set(T *ptr) { ptr_ = ptr; }

    T *Get() { return ptr_; }

   private:
    T *ptr_;
  };

 public:
  static void SetDefaultInstance(T *ptr) {
    InstanceHelper::GetInstance()->Set(ptr);
  }

  static T *GetDefaultInstance() {
    return InstanceHelper::GetInstance()->Get();
  }
};
}  // namespace webkit