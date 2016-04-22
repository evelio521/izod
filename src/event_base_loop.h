/***************************************************************************
 * 
 * Copyright (c) 2016 evelio, Inc. All Rights Reserved
 *
 * *************************************************************************/

/**
 * @file:   event_base_loop.h
 * @author: sunqiang(evelio@126.com)
 * @date:   2016-04-22 14:05:23
 * @brief
 *
 **/
#ifndef SERVER_SRC_EVENT_BASE_LOOP_H_
#define SERVER_SRC_EVENT_BASE_LOOP_H_

#include "base/marcos.h"
#include "base/basictypes.h"

struct event_base;

_START_SERVER_NAMESPACE_

class EventBaseLoop {
 public:
  EventBaseLoop();
  ~EventBaseLoop();

  struct event_base* base() {
    return base_;
  }
  void Dispatch();
 private:
  struct event_base* base_;
  DISALLOW_COPY_AND_ASSIGN(EventBaseLoop);
};

_END_SERVER_NAMESPACE_

#endif  // SERVER_SRC_EVENT_BASE_LOOP_H_

