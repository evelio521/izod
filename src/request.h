/***************************************************************************
 * 
 * Copyright (c) 2016 evelio, Inc. All Rights Reserved
 *
 * *************************************************************************/

/**
 * @file:   request.h
 * @author: sunqiang(evelio@126.com)
 * @date:   2016-04-22 14:40:33
 * @brief
 *
 **/
#ifndef SERVER_SRC_REQUEST_H_
#define SERVER_SRC_REQUEST_H_

#include <string>

#include "base/marcos.h"
#include "base/basictypes.h"
#include "base/compat.h"
#include "thirdlibs/event/include/event2/http.h"

struct evhttp_request;

_START_SERVER_NAMESPACE_

// please insert more functions if you need;
// reference on http_struct.h
class Request {
 public:
  explicit Request(struct evhttp_request* request);
  ~Request();

  struct evhttp_request* request() {
    return request_;
  }

  string GetHeader(const string& key) const;
  string GetRequestData() const;
  /** Returns the request command */
  enum evhttp_cmd_type GetRequestMethod() const;

  const string GetRequestUri() const;
  string GetPath() const;
  bool GetQueryParams(map<string, string>* params) const;
  bool GetPostParams(map<string, string>* params) const;

  struct evhttp_uri* GetUrielems() const;
  const string GetRemoteHost() const;
  ev_uint16_t GetRemotePort() const;
  //  debug info for request
  void Dump() const;

 private:
  struct evhttp_request* request_;
  //map<string, string>* params;
  DISALLOW_COPY_AND_ASSIGN(Request);
};


//  Parse url params, for example, "http://www.baidu.com/?k1=v1&k2=v2"
//  return a map containing k1 and k2.
//  Note that value will be decoded.
map<string, string> ParseUrlParam(const Request& request);
void ParseKvlist(const string& line,
                   const string& key_value_delimiter,
                   char key_value_pair_delimiter, map<string, string> *kv_pairs,
                   vector<pair<string, string> >* vec);
void ParseUrlParams(const string& query, map<string, string> *params);

_END_SERVER_NAMESPACE_

#endif  // SERVER_SRC_REQUEST_H_

