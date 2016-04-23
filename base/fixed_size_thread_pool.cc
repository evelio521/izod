// Copyright 2016  All Rights Reserved.
// Author: 

#include <functional>

#include "base/log.h"
#include "base/thread.h"
#include "base/fixed_size_thread_pool.h"

namespace base {

class JobThread : public Thread {
 public:
  JobThread(base::ConcurrentQueue<std::function<void()>>* callbacks)
    : shutdown_(false), callbacks_(callbacks) {}
  virtual ~JobThread() {}

  virtual void Run() {
    for (;;) {
      if (shutdown_) {
        LOG(INFO) << "Shutdown thread.";
        break;
      }
      // Drain callbacks before considering shutdown to ensure all work
      // gets completed.
      std::function<void()> cb = nullptr;
      callbacks_->Pop(cb);
      if (cb == nullptr) {
        break;
      }
      cb();
    }
    LOG(INFO) << "Exit Thread:" << GetThreadId();
  }
  void ShutDown() {
    shutdown_ = true;
  }
 private:
  bool shutdown_;
  base::ConcurrentQueue<std::function<void()>>* callbacks_;
};

FixedSizeThreadPool::FixedSizeThreadPool(int num_threads) {
  for (int i = 0; i < num_threads; i++) {
    base::JobThread* thread = new JobThread(&callbacks_);
    thread->Start();
    threads_.push_back(thread);
  }
}

FixedSizeThreadPool::~FixedSizeThreadPool() {
  for (auto& thread : threads_) {
    thread->ShutDown();
    thread->Join();
    delete thread;
  }
}

void FixedSizeThreadPool::Add(const std::function<void()>& callback) {
  callbacks_.Push(callback);
}

}  // namespace grpc
