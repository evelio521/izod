// Copyright (c) 2006-2008 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/at_exit.h"
#include "base/log.h"

_START_BASE_NAMESPACE_

// Keep a stack of registered AtExitManagers.  We always operate on the most
// recent, and we should never have more than one outside of testing, when we
// use the shadow version of the constructor.  We don't protect this for
// thread-safe access, since it will only be modified in testing.
static AtExitManager* g_top_manager = NULL;

AtExitManager::AtExitManager() : next_manager_(NULL) {
  DCHECK(!g_top_manager);
  g_top_manager = this;
}

AtExitManager::AtExitManager(bool shadow) : next_manager_(g_top_manager) {
  DCHECK(shadow || !g_top_manager);
  g_top_manager = this;
}

AtExitManager::~AtExitManager() {
  if (!g_top_manager) {
    LOG(FATAL) << "Tried to ~AtExitManager without an AtExitManager";
    return;
  }
  DCHECK(g_top_manager == this);

  ProcessCallbacksNow();
  g_top_manager = next_manager_;
}

// static
void AtExitManager::RegisterCallback(AtExitCallbackType func, void* param) {
  if (!g_top_manager) {
    LOG(FATAL) << "Tried to RegisterCallback without an AtExitManager";
    return;
  }

  DCHECK(func);

  base::MutexLock lock(&g_top_manager->lock_);
  g_top_manager->stack_.push(CallbackAndParam(func, param));
}

// static
void AtExitManager::ProcessCallbacksNow() {
  if (!g_top_manager) {
    LOG(FATAL) << "Tried to ProcessCallbacksNow without an AtExitManager";
    return;
  }

  base::MutexLock lock(&g_top_manager->lock_);

  while (!g_top_manager->stack_.empty()) {
    CallbackAndParam callback_and_param = g_top_manager->stack_.top();
    g_top_manager->stack_.pop();

    callback_and_param.func_(callback_and_param.param_);
  }
}

_END_BASE_NAMESPACE_
