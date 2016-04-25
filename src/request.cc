/*******************************************************************************
 *
 * Copyright (c) 2016 evelio, Inc. All Rights Reserved
 *
 *******************************************************************************/



/**
 * @file:   request.cc
 * @author: sunqiang(evelio@126.com)
 * @date:   2016-04-22 14:41:16
 * @brief
 *
 **/

#include "src/request.h"

#include "base/basictypes.h"
#include "base/compat.h"
#include "base/string_util.h"
#include "base/log.h"
#include "thirdlibs/event/include/event2/http.h"
#include "thirdlibs/event/include/event2/http_struct.h"
#include "thirdlibs/event/include/event2/buffer.h"
#include "thirdlibs/event/include/event2/keyvalq_struct.h"

_START_SERVER_NAMESPACE_

void ParseKvlist(
    const string& line,
    const string& key_value_delimiter,
    char key_value_pair_delimiter,
    map<string, string> *kv_pairs,
    vector<pair<string, string> >* vec) {
  vector<string> pairs;
  vector<string> kvpair;
  SplitString(line, key_value_pair_delimiter, &pairs);

  for (size_t i = 0; i < pairs.size(); ++i) {
    kvpair.clear();
    string::size_type index = pairs[i].find(key_value_delimiter);
    if (index == string::npos) {
      continue;
    }
    string key = pairs[i].substr(0, index);
    string value = pairs[i].substr(index + key_value_delimiter.length());
    if (kv_pairs) {
      kv_pairs->insert(make_pair(key, value));
    }
    if (vec) {
      vec->push_back(make_pair(key, value));
    }
  }
}

map<string, string> ParseUrlParam(const Request& request) {
  string url = request.GetRequestUri();
  string::size_type index = url.find('?');
  map<string, string> kv_pairs;
  if (index == string::npos) {
    return kv_pairs;
  }

  string query_part = url.substr(index + 1);
  LOG(INFO)<< "query part:" << query_part;
  ParseKvlist(query_part, "=", '&', &kv_pairs, NULL);
  map<string, string> result;
  for (auto it = kv_pairs.begin(); it != kv_pairs.end(); ++it) {
    result.insert(make_pair(it->first, it->second));
  }
  return result;
}

void ParseUrlParams(const string& query,
                                 map<string, string> *params) {
  map<string, string> kv_pairs;
  ParseKvlist(query, "=", '&', &kv_pairs, NULL);
  for (auto it = kv_pairs.begin(); it != kv_pairs.end(); ++it) {
    params->insert(make_pair(it->first, it->second));
  }
}

Request::Request(struct evhttp_request* request) :
    request_(request) {
}

Request::~Request() {
}

const string Request::GetRequestUri() const {
  return request_->uri;
}

string Request::GetPath() const {
  string uri(request_->uri);
  string::size_type index = uri.find('?');
  if (index == string::npos) {
    return uri;
  } else {
    return uri.substr(0, index);
  }
}


bool Request::GetQueryParams(map<string, string>* params) const {
  string uri(request_->uri);
  string::size_type index = uri.find('?');
  if (index != string::npos) {
    string query = uri.substr(index + 1);
    ParseUrlParams(query, params);
    return true;
  } else {
    return false;
  }
}

const string Request::GetRemoteHost() const {
  return request_->remote_host;
}

ev_uint16_t Request::GetRemotePort() const {
  return request_->remote_port;
}
bool Request::GetPostParams(map<string, string>* params) const {
  string postdata = GetRequestData();
  if (postdata.empty()) {
    return false;
  }
  ParseUrlParams(postdata, params);
  return true;
}


struct evhttp_uri* Request::GetUrielems() const {
  return request_->uri_elems;
}
string Request::GetHeader(const string& key) const {
  struct evkeyvalq *headers;
  struct evkeyval *header;
  headers = evhttp_request_get_input_headers(request_);
  for (header = headers->tqh_first; header; header = header->next.tqe_next) {
    if (base::StrCaseCmp(header->key, key.c_str()) == 0) {
      return string(header->value);
    }
  }
  return string();
}

string Request::GetRequestData() const {
  struct evbuffer *buf = evhttp_request_get_input_buffer(request_);
  string data;
  while (evbuffer_get_length(buf)) {
    int n;
    char cbuf[128];
    n = evbuffer_remove(buf, cbuf, sizeof(buf) - 1);
    if (n > 0) {
      data.append(cbuf, n);
    }
  }
  return data;
}

enum evhttp_cmd_type Request::GetRequestMethod() const {
  return evhttp_request_get_command(request_);
}

void Request::Dump() const {
  //  Dump request method and Uri
  const char *cmdtype;
  switch (evhttp_request_get_command(request_)) {
    case EVHTTP_REQ_GET:
      cmdtype = "GET";
      break;
    case EVHTTP_REQ_POST:
      cmdtype = "POST";
      break;
    case EVHTTP_REQ_HEAD:
      cmdtype = "HEAD";
      break;
    case EVHTTP_REQ_PUT:
      cmdtype = "PUT";
      break;
    case EVHTTP_REQ_DELETE:
      cmdtype = "DELETE";
      break;
    case EVHTTP_REQ_OPTIONS:
      cmdtype = "OPTIONS";
      break;
    case EVHTTP_REQ_TRACE:
      cmdtype = "TRACE";
      break;
    case EVHTTP_REQ_CONNECT:
      cmdtype = "CONNECT";
      break;
    case EVHTTP_REQ_PATCH:
      cmdtype = "PATCH";
      break;
    default:
      cmdtype = "unknown";
      break;
  }

  printf("Received a %s request for %s\nHeaders:\n", cmdtype,
         evhttp_request_get_uri(request_));

  //  Dump headers
  struct evkeyvalq *headers;
  struct evkeyval *header;
  headers = evhttp_request_get_input_headers(request_);
  for (header = headers->tqh_first; header; header = header->next.tqe_next) {
    printf("  %s: %s\n", header->key, header->value);
  }

  //  Dump input data
  struct evbuffer *buf;
  buf = evhttp_request_get_input_buffer(request_);
  puts("Input data: <<<");
  while (evbuffer_get_length(buf)) {
    int n;
    char cbuf[128];
    n = evbuffer_remove(buf, cbuf, sizeof(buf) - 1);
    if (n > 0)
      (void) fwrite(cbuf, 1, n, stdout);
  }
  puts(">>>");
}

_END_SERVER_NAMESPACE_


/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */ 

