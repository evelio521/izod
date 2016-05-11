/***************************************************************************
 * 
 * Copyright (c) 2016 evelio, Inc. All Rights Reserved
 *
 * *************************************************************************/

/**
 * @file:   client.h
 * @author: sunqiang(evelio@126.com)
 * @date:   2016-05-09 19:37:10
 * @brief
 *
 **/
#ifndef SERVER_SRC_CLIENT_H_
#define SERVER_SRC_CLIENT_H_


#include <string>

#include "base/basictypes.h"
#include "base/compat.h"
#include "base/hash_tables.h"
#include "src/event_base_loop.h"
#include "thirdlibs/event/include/event2/event.h"
#include "thirdlibs/event/include/event2/buffer.h"
#include "thirdlibs/event/include/event2/http.h"
#include "thirdlibs/event/include/event2/http_struct.h"
#include "thirdlibs/event/include/event2/keyvalq_struct.h"
_START_CLIENT_NAMESPACE_

class Buffer;
struct http_request_get;
struct http_request_post;

//enum HttpMethod {
//  EVHTTP_REQ_GET     = 1 << 0,
//  EVHTTP_REQ_POST    = 1 << 1,
//  EVHTTP_REQ_HEAD    = 1 << 2,
//  EVHTTP_REQ_PUT     = 1 << 3,
//  EVHTTP_REQ_DELETE  = 1 << 4,
//  EVHTTP_REQ_OPTIONS = 1 << 5,
//  EVHTTP_REQ_TRACE   = 1 << 6,
//  EVHTTP_REQ_CONNECT = 1 << 7,
//  EVHTTP_REQ_PATCH   = 1 << 8
//};

class Client {
 public:
  Client();
  ~Client();

  //  Call these functions after Reset().
//  void SetHttpMethod(HttpMethod type);
  // Join map to format like "k1=v1&k2=v2"
  void SetPostParams(const map<string, string>& m);
  void SetPostData(const string& data);

  void Reset();
  bool FetchGetUrl(const string& url);
  bool FetchPostUrl(const string& url);

  int response_code() const {
    return response_code_;
  }

  const string& ResponseHeader() const;

  const string& ResponseBody() const;

  bool IsHeaderTooLarge() const;
  bool IsBodyTooLarge() const;

  int GetResponseCode() const;

  // Sets header.
  void AddHeader(const string& key, const string& value);

  void SetConnectTimeout(int time_ms);
  void SetFetchTimeout(int time_ms);
  void SetAuth(const string& user, const string& password);
  void SetProxy(const string& proxy_host, int proxy_port);

 private:
//  void print_request_head_info(struct evkeyvalq *header);
//  void print_uri_parts_info(const struct evhttp_uri * http_uri);
//  void http_requset_post_cb(struct evhttp_request *req, void *arg);
//  void http_requset_get_cb(struct evhttp_request *req, void *arg);
//  int start_url_request(struct http_request_get *http_req, int req_get_flag);
  void *http_request_new(struct event_base* base,
                         const char *url,
                         int req_get_flag,
                         const char *content_type,
                         const char* data);
  void http_request_free(struct http_request_get *http_req_get,
                         int req_get_flag);
  bool start_http_requset(struct event_base* base,
                           const char *url,
                           int req_get_flag,
                           const char *content_type,
                           const char* data);

  int response_code_;  // NOLINT
  string  head_write_buffer_;
  string  body_write_buffer_;
//  HttpMethod method_;
  string post_data_;
  //server::EventBaseLoop event_loop;
  DISALLOW_COPY_AND_ASSIGN(Client);
};
_END_CLIENT_NAMESPACE_  // namespace client

#endif  // SERVER_SRC_CLIENT_H_

