// Copyright 2016  All Rights Reserved.
// Author: 

#ifndef BASE_THREAD_POOL_INTERFACE_H
#define BASE_THREAD_POOL_INTERFACE_H

#include <functional>

#include "base/basictypes.h"

namespace base {
// A thread pool interface for running callbacks.
class ThreadPoolInterface {
 public:
  virtual ~ThreadPoolInterface() {}

  // Schedule the given callback for execution.
  virtual void Add(const std::function<void()>& callback) = 0;
  virtual int WaitingThreads() const {
    // For fixed_size thread pool, all threads are waiting job, return -1 for it.
    return -1;
  }
};
}  // namespace base

#endif  // BASE_THREAD_POOL_INTERFACE_H
