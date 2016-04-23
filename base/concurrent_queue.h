// Copyright 2014. All Rights Reserved.
// Author: sunqiang 

#ifndef BASE_CONCURRENT_QUEUE_H_
#define BASE_CONCURRENT_QUEUE_H_

#include <queue>

#include "base/basictypes.h"
#include "base/mutex.h"

namespace base {

template<typename Data>
class ConcurrentQueue {
 public:
  ConcurrentQueue() {}
  virtual ~ConcurrentQueue() {}
  virtual void Push(Data const& data) {
    MutexLock mutex(&mutex_);
    queue_.push(data);
    //  lock.unlock();
    cond_var_.Signal();
  }

  virtual bool Empty() const {
    MutexLock mutex(&mutex_);
    return queue_.empty();
  }

  virtual bool TryPop(Data& popped_value) {  // NOLINT
    MutexLock mutex(&mutex_);
    if (queue_.empty()) {
      return false;
    }
    popped_value = queue_.front();
    queue_.pop();
    return true;
  }

  virtual void Pop(Data& popped_value) {  // NOLINT
    MutexLock mutex(&mutex_);
    while (queue_.empty()) {
      cond_var_.Wait(&mutex_);
    }
    popped_value = queue_.front();
    queue_.pop();
  }

  virtual void Swap(std::queue<Data>* popped_queue) {  // NOLINT
      MutexLock mutex(&mutex_);
      std::swap(queue_, *popped_queue);
    }

  virtual size_t Size() const {
    MutexLock mutex(&mutex_);
    return queue_.size();
  }

 private:
  std::queue<Data> queue_;
  mutable Mutex mutex_;
  CondVar cond_var_;
  DISALLOW_COPY_AND_ASSIGN(ConcurrentQueue);
};

template<typename Data>
class FixedSizeConQueue : public ConcurrentQueue<Data> {
 public:
  explicit FixedSizeConQueue(size_t max_size)
    : max_size_(max_size) {
  }
  virtual ~FixedSizeConQueue() {}
  virtual void Push(Data const& data) {
    MutexLock mutex(&mutex_);
    while (queue_.size() == max_size_) {
      cond_var_.Wait(&mutex_);
    }
    queue_.push(data);
    cond_var_.Signal();
  }

  bool TryPush(const Data& data) {  // NOLINT
    MutexLock mutex(&mutex_);
    if (queue_.size() >= max_size_) {
      return false;
    }
    queue_.push(data);
    cond_var_.Signal();
    return true;
  }

  virtual void Pop(Data& popped_value) {  // NOLINT
    MutexLock mutex(&mutex_);
    while (queue_.empty()) {
      cond_var_.Wait(&mutex_);
    }
    popped_value = queue_.front();
    queue_.pop();
    cond_var_.Signal();
  }

 private:
  std::queue<Data> queue_;
  size_t max_size_;
  mutable Mutex mutex_;
  CondVar cond_var_;
  DISALLOW_COPY_AND_ASSIGN(FixedSizeConQueue);
};
}  //  namespace base
#endif  // BASE_CONCURRENT_QUEUE_H_
