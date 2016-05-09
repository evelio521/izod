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

_START_CLIENT_NAMESPACE_

class Buffer;

enum HttpMethod {
  kGet = 1,
  kPost = 2,
  kHead = 3,
};

class Client {
 public:
  Client();
  ~Client();

  //  Call these functions after Reset().
  void SetHttpMethod(HttpMethod type);
  // Join map to format like "k1=v1&k2=v2"
  void SetPostParams(const map<string, string>& m);
  void SetPostData(const string& data);

  void Reset();
  bool FetchUrl(const string& url);

  int response_code() const {
    return response_code_;
  }

  const string& ResponseHeader() const;

  const string& ResponseBody() const;

  bool IsHeaderTooLarge() const;
  bool IsBodyTooLarge() const;

  // Sets header.
  void AddHeader(const string& key, const string& value);

  void SetConnectTimeout(int time_ms);
  void SetFetchTimeout(int time_ms);
  void SetAuth(const string& user, const string& password);
  void SetProxy(const string& proxy_host, int proxy_port);

 private:

  long response_code_;  // NOLINT
  std::unique_ptr<Buffer> head_write_buffer_;
  std::unique_ptr<Buffer> body_write_buffer_;
  HttpMethod method_;
  string post_data_;
  DISALLOW_COPY_AND_ASSIGN(Client);
};
_END_CLIENT_NAMESPACE_  // namespace client

#endif  // SERVER_SRC_CLIENT_H_

