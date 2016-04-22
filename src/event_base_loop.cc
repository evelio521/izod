/*******************************************************************************
 *
 * Copyright (c) 2016 evelio, Inc. All Rights Reserved
 *
 *******************************************************************************/



/**
 * @file:   event_base_loop.cc
 * @author: sunqiang(evelio@126.com)
 * @date:   2016-04-22 14:06:04
 * @brief
 *
 **/

#include "src/event_base_loop.h"

#include "base/log.h"
#include "thirdlibs/event/include/event2/event.h"

_START_SERVER_NAMESPACE_

EventBaseLoop::EventBaseLoop() {
  base_ = event_base_new();
  CHECK(base_) << "creating event_base failed, Exit!!!!";
}

EventBaseLoop::~EventBaseLoop() {
}

void EventBaseLoop::Dispatch() {
  event_base_dispatch(base_);
}

_END_SERVER_NAMESPACE_

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */ 

