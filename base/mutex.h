// A pthread mutex wrapper.

#ifndef BASE_MUTEX_H_
#define BASE_MUTEX_H_

#include <pthread.h>
#include "base/marcos.h"

_START_BASE_NAMESPACE_

class Mutex {
 public:
  Mutex();
  ~Mutex();

  void Lock();
  void Unlock();

 private:
  friend class CondVar;
  pthread_mutex_t mutex_;
};

class MutexLock {
 public:
  explicit MutexLock(Mutex* mutex);
  ~MutexLock();

  // If want to release mutex explicit, call Unlock function.
  // If NOT, ~MutexLock() will unlock auto.
  void Unlock();
 private:
  Mutex* mutex_;
};

class SharedMutex {
 public:
  SharedMutex();
  ~SharedMutex();

  //  Get a Shared Read Lock
  void ReadLock();
  //  Get an Exclusive Write Lock
  void WriteLock();

  //  Get a Shared Read Lock with No Wait
  bool TryReadLock();
  //  Get an Exclusive Write Lock with No Wait
  bool TryWriteLock();

  //  Unlock an Exclusive Write or Shared Read Lock
  void Unlock();

 private:
  pthread_rwlock_t rwlock_;
};

class ReadLock {
 public:
  explicit ReadLock(SharedMutex* mutex);
  ~ReadLock();

 private:
  SharedMutex* mutex_;
};

class WriteLock {
 public:
  explicit WriteLock(SharedMutex* mutex);
  ~WriteLock();

 private:
  SharedMutex* mutex_;
};

/// Wrapper for pthread_cond_t.
class CondVar {
 public:
  CondVar();
  ~CondVar();

  void Wait(Mutex *mu);
  bool WaitWithTimeout(Mutex *mu, int millis);

  void Signal();
  void SignalAll();
 private:
  pthread_cond_t cv_;
};
_END_BASE_NAMESPACE_
#endif  // BASE_MUTEX_H_
