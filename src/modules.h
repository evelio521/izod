/***************************************************************************
 * 
 * Copyright (c) 2016 evelio, Inc. All Rights Reserved
 *
 * *************************************************************************/

/**
 * @file:   modules.h
 * @author: sunqiang(evelio@126.com)
 * @date:   2016-04-23 13:27:56
 * @brief
 *
 **/
#ifndef SERVER_SRC_MODULES_H_
#define SERVER_SRC_MODULES_H_

#include <string>

#include "base/marcos.h"
#include "base/basictypes.h"
#include "base/compat.h"
#include "base/hash_tables.h"

#include "src/request.h"
#include "src/response.h"
#include "src/handlers.h"
#include "src/event_base_loop.h"
#include "base/thread_pool_interface.h"
#include "thirdlibs/event/include/event2/http.h"
#include "thirdlibs/event/include/event2/buffer.h"

struct evhttp_request;
struct evhttp;
struct evhttp_bound_socket;

_START_SERVER_NAMESPACE_

class Request;
class Response;
class EventBaseLoop;
class Handler;

struct HandlerWrapper {
  ~HandlerWrapper() {
    delete handler;
  }
  Handler* handler;
  // Don't delete it.
  base::ThreadPoolInterface* pool;
};

class Modules {
 public:
  Modules();
  virtual ~Modules();

  bool RegisterHttpHandler(Handler* handler);
  void Server();
  virtual void Init() = 0;

  const string& uri_root() const {
    return uri_root_;
  }
 private:
  int BindSocket();
  void ShowBindInfo(int listen_fd);
  void Run(int listen_fd);

  int listen_port_;
  string uri_root_;
  base::hash_map<string, HandlerWrapper*> handlers_;
  size_t threads_num_;
  unique_ptr<base::ThreadPoolInterface> thread_pool_;
  DISALLOW_COPY_AND_ASSIGN(Modules);
};

_END_SERVER_NAMESPACE_

#endif  // SERVER_SRC_MODULES_H_

