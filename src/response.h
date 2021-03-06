/***************************************************************************
 * 
 * Copyright (c) 2016 evelio, Inc. All Rights Reserved
 *
 * *************************************************************************/

/**
 * @file:   response.h
 * @author: sunqiang(evelio@126.com)
 * @date:   2016-04-23 09:59:20
 * @brief
 *
 **/
#ifndef SERVER_SRC_RESPONSE_H_
#define SERVER_SRC_RESPONSE_H_

#include <string>

#include "base/basictypes.h"
#include "base/compat.h"
#include "thirdlibs/gflags/gflags.h"
#include "thirdlibs/glog/logging.h"

_START_SERVER_NAMESPACE_

class Request;

class Response {
 public:
  explicit Response(Request* request);
  ~Response();

  void AppendHeader(const string& key, const string& value);
  // helper function to set "Content-Type" to html/json/binary...
  // you should add fn&callback params if you wanna return jsonp
  void SetJsonContentType();
  void SetHtmlContentType();
  void SetBinaryContentType();
  void SetReturnType(string header, string content);

  void AppendBuffer(const string& buff);
  void SetResponseCode(int code);

  bool SendToClient();
 private:
  string header_;
  string content_;
  Request* request_;
  int code_;
  DISALLOW_COPY_AND_ASSIGN(Response);
};

_END_SERVER_NAMESPACE_

#endif  // SERVER_SRC_RESPONSE_H_

