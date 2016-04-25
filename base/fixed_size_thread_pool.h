// Copyright 2016 All Rights Reserved.
// Author: 

#ifndef BASE_FIXED_SIZE_THREAD_POOL_H
#define BASE_FIXED_SIZE_THREAD_POOL_H

#include <queue>
#include <vector>

#include "base/concurrent_queue.h"
#include "base/thread.h"
#include "base/thread_pool_interface.h"
#include "base/compat.h"
#include "base/basictypes.h"
#include "base/marcos.h"

_START_BASE_NAMESPACE_
class JobThread;

class FixedSizeThreadPool : public ThreadPoolInterface {
 public:
  explicit FixedSizeThreadPool(int num_threads);
  virtual ~FixedSizeThreadPool();

  virtual void Add(const std::function<void()>& callback);

 private:
  base::ConcurrentQueue<std::function<void()>> callbacks_;
  std::vector<JobThread*> threads_;
};

_END_BASE_NAMESPACE_

#endif  // BASE_FIXED_SIZE_THREAD_POOL_H
