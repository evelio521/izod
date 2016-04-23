/***************************************************************************
 * 
 * Copyright (c) 2016 evelio, Inc. All Rights Reserved
 *
 * *************************************************************************/

/**
 * @file:   handlers.h
 * @author: sunqiang(evelio@126.com)
 * @date:   2016-04-23 11:01:46
 * @brief
 *
 **/
#ifndef SERVER_SRC_HANDLERS_H_
#define SERVER_SRC_HANDLERS_H_

#include <string>

#include "base/marcos.h"
#include "base/basictypes.h"
#include "base/compat.h"
//#include "base/callback.h"

_START_SERVER_NAMESPACE_

class Request;
class Response;

class Handler {
 public:
  Handler(string regpath);
  virtual ~Handler();

  virtual int HttpHandler(Request* request, Response* response) = 0;

 protected:
  string path;

 private:
  DISALLOW_COPY_AND_ASSIGN(Handler);
};

class JsonHandler : public Handler {
 public:
  JsonHandler(string regpath);
  virtual ~JsonHandler();

  //  Call callback function, then send back response.
  virtual int HttpHandler(Request* request, Response* response);

 private:
  DISALLOW_COPY_AND_ASSIGN(JsonHandler);
};

_END_SERVER_NAMESPACE_

#endif  // SERVER_SRC_HANDLERS_H_
