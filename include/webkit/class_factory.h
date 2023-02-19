#pragma once

#include <memory>

#include "webkit/instance_base.h"

namespace webkit {
template <typename T>
class ClassFactory : public InstanceBase<ClassFactory<T>> {
 public:
  ClassFactory() = default;

  virtual ~ClassFactory() = default;

  virtual std::shared_ptr<T> Build() = 0;
};
}