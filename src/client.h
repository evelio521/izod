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

//enum evhttp_cmd_type {
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

  // Call these functions after Reset().
  void SetHttpMethod(evhttp_cmd_type type);
  // Join map to format like "k1=v1&k2=v2"
  void SetPostParams(const map<string, string>& m);
  void SetPostData(const string& data);

  void Reset();
  // For Http Get Method
  bool FetchGetUrl(const string& url);
  // For Http Post Method
  bool FetchPostUrl(const string& url);
  // Return Header Data
  const string& ResponseHeader() const;
  // Retuen Body Data
  const string& ResponseBody() const;

  // Drop it temporarily
  bool IsHeaderTooLarge() const;
  // Drop it temporarily
  bool IsBodyTooLarge() const;
  // Return response code like 200 etc
  int GetResponseCode() const;

  // Sets header.
  void AddHeader(const string& key, const string& value);
  // Drop it temporarily
  void SetConnectTimeout(int time_ms);
  // Drop it temporarily
  void SetFetchTimeout(int time_ms);
  // Drop it temporarily
  void SetAuth(const string& user, const string& password);
  // Drop it temporarily
  void SetProxy(const string& proxy_host, int proxy_port);

  void SetConnectionKeepAlive(bool isKeepAlive);

  bool GetConnectionKeepAlive();

 private:
  static void print_request_head_info(struct evkeyvalq *header);
  static void print_uri_parts_info(const struct evhttp_uri * http_uri);
  static void http_requset_post_cb(struct evhttp_request *req, void *arg);
  static void http_requset_get_cb(struct evhttp_request *req, void *arg);
  static int start_url_request(struct http_request_get *http_req,
                               int req_get_flag, int conn_time,
                               bool isKeepAlive);
//  定义FUNC类型是一个指向函数的指针，该函数参数为void*，返回值为void*
//  typedef void (*FUNC)(struct evhttp_request *req, void *arg);
//  强制转换func()的类型
//  FUNC http_requset_post_callback = (FUNC)&Client::http_requset_post_cb;
//  强制转换func()的类型
//  FUNC http_requset_get_callback = (FUNC)&Client::http_requset_get_cb;

  void *http_request_new(struct event_base* base, const char *url,
                         int req_get_flag, const char *content_type,
                         const char* data, bool isKeepAlive);
  void http_request_free(struct http_request_get *http_req_get,
                         int req_get_flag);
  bool start_http_requset(struct event_base* base, const char *url,
                          int req_get_flag, const char *content_type,
                          const char* data, bool isKeepAlive);

  static int response_code_;  // NOLINT
  static string head_write_buffer_;
  static string body_write_buffer_;
  string post_data_;
  // Only support Get or Post Method
  evhttp_cmd_type method_;
  int time_;
  int connection_time_;
  struct http_request_get *http_req_get;
  struct http_request_post *http_req_post;
  bool isConnectionKeepAlive_;
  DISALLOW_COPY_AND_ASSIGN(Client);
};
_END_CLIENT_NAMESPACE_  // namespace client

#endif  // SERVER_SRC_CLIENT_H_

