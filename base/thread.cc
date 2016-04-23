// Copyright 2014 All Rights Reserved.
// Author: evelio

#include "base/thread.h"

#include <cstring>

#include "thirdlibs/glog/logging.h"
#include "base/marcos.h"

_START_BASE_NAMESPACE_
Thread::Thread(bool joinable)
    : tid_(0), isrunning_(false), isjoinable_(joinable) {}

Thread::~Thread() {}

void* Thread::RunThread(void* v) {
  Thread* t = static_cast<Thread*>(v);
  t->Run();
  return 0;
}

void Thread::Start() {
  CHECK(!isrunning_);
  pthread_attr_t attr;
  CHECK_EQ(pthread_attr_init(&attr), 0);
  CHECK_EQ(
      pthread_attr_setdetachstate(
          &attr,
          isjoinable_ ? PTHREAD_CREATE_JOINABLE : PTHREAD_CREATE_DETACHED),
          0);

  int err = pthread_create(&tid_, &attr, Thread::RunThread, this);
  if (err != 0) {
    LOG(ERROR) << "Thread create failed: " << strerror(err);
  }
  CHECK_EQ(pthread_attr_destroy(&attr), 0);
  isrunning_ = true;
}

void Thread::Join() {
  CHECK(isjoinable_) << "Thread is not joinable";
  CHECK(isrunning_);
  const int err = pthread_join(tid_, NULL);
  if (err != 0) {
    LOG(ERROR) << "Thread join failed: " << strerror(err);
  }
  isrunning_ = false;
}

_END_BASE_NAMESPACE_
