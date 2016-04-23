// Copyright 2016 All Rights Reserved.
// Author: 

#ifndef BASE_SPIN_LOCK_H_
#define BASE_SPIN_LOCK_H_

#include <pthread.h>
#include "base/basictypes.h"
#include "base/marcos.h"

_START_BASE_NAMESPACE_
class SpinLock {
 public:
  SpinLock() {
    pthread_spin_init(&lock_, 0);
  }
  ~SpinLock() {
    pthread_spin_destroy(&lock_);
  }
 
  void Lock() {
    pthread_spin_lock(&lock_);
  }

  void Unlock() {
    pthread_spin_unlock(&lock_);
  }

 private:
  pthread_spinlock_t lock_;
  DISALLOW_COPY_AND_ASSIGN(SpinLock);
};
_END_BASE_NAMESPACE_
#endif  // BASE_SPIN_LOCK_H_
