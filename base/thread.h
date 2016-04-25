// Copyright 2014  All Rights Reserved.
// Author: evelio 
//

#ifndef BASE_THREAD_H__
#define BASE_THREAD_H__

#include <pthread.h>
#include "base/basictypes.h"
#include "base/marcos.h"

_START_BASE_NAMESPACE_

class Thread {
 public:
  explicit Thread(bool joinable = true);
  virtual ~Thread();

  virtual void Run() = 0;

  void SetJoinable(bool joinable) {
    if (!isrunning_) {
      isjoinable_ = joinable;
    }
  }

  // begin running.
  void Start();

  // Waits until the thread completes.
  void Join();

  pthread_t GetThreadId() { return tid_; }

  bool running() const {
    return isrunning_;
  }

 private:
  static void* RunThread(void* v);

  pthread_t tid_;
  bool isrunning_;
  bool isjoinable_;
  DISALLOW_COPY_AND_ASSIGN(Thread);
};

_END_BASE_NAMESPACE_

#endif  // BASE_THREAD_H__
