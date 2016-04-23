//

#include "base/mutex.h"

#include "base/log.h"
#include "base/time.h"

namespace base {

Mutex::Mutex() {
  pthread_mutex_init(&mutex_, 0);
}

Mutex::~Mutex() {
  pthread_mutex_destroy(&mutex_);
}

void Mutex::Lock() {
  pthread_mutex_lock(&mutex_);
}

void Mutex::Unlock() {
  pthread_mutex_unlock(&mutex_);
}

MutexLock::MutexLock(Mutex* mutex)
    : mutex_(mutex) {
  mutex_->Lock();
}

MutexLock::~MutexLock() {
  if (mutex_) {
    mutex_->Unlock();
  }
}

void MutexLock::Unlock() {
  mutex_->Unlock();
  mutex_ = NULL;
}

SharedMutex::SharedMutex() {
  CHECK_EQ(pthread_rwlock_init(&rwlock_, NULL), 0) << "Fail to init rwlock";
}

SharedMutex::~SharedMutex() {
  CHECK_EQ(pthread_rwlock_destroy(&rwlock_), 0) << "Fail to destroy rwlock";
}

void SharedMutex::ReadLock() {
  pthread_rwlock_rdlock(&rwlock_);
}

void SharedMutex::WriteLock() {
  pthread_rwlock_wrlock(&rwlock_);
}

bool SharedMutex::TryReadLock() {
  return pthread_rwlock_tryrdlock(&rwlock_) == 0;
}

bool SharedMutex::TryWriteLock() {
  return pthread_rwlock_trywrlock(&rwlock_) == 0;
}

void SharedMutex::Unlock() {
  pthread_rwlock_unlock(&rwlock_);
}

ReadLock::ReadLock(SharedMutex* mutex) : mutex_(mutex) {
  mutex_->ReadLock();
}

ReadLock::~ReadLock() {
  mutex_->Unlock();
}

WriteLock::WriteLock(SharedMutex* mutex) : mutex_(mutex) {
  mutex_->WriteLock();
}

WriteLock::~WriteLock() {
  mutex_->Unlock();
}

CondVar::CondVar() {
  CHECK(0 == pthread_cond_init(&cv_, NULL));
}

CondVar::~CondVar() {
  CHECK(0 == pthread_cond_destroy(&cv_));
}

void CondVar::Wait(Mutex *mu) {
  CHECK(0 == pthread_cond_wait(&cv_, &mu->mutex_));
}

bool CondVar::WaitWithTimeout(Mutex *mu, int millis) {
  struct timeval now;
  struct timespec timeout;
  gettimeofday(&now, NULL);
  timeval2timespec(&now, &timeout, millis);
  return 0 != pthread_cond_timedwait(&cv_, &mu->mutex_, &timeout);
}

void CondVar::Signal() {
  CHECK(0 == pthread_cond_signal(&cv_));
}

void CondVar::SignalAll() {
  CHECK(0 == pthread_cond_broadcast(&cv_));
}

}  // namespace base
