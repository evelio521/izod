/*******************************************************************************
 *
 * Copyright (c) 2016 evelio, Inc. All Rights Reserved
 *
 *******************************************************************************/



/**
 * @file:   client.cc
 * @author: sunqiang(evelio@126.com)
 * @date:   2016-05-09 19:37:23
 * @brief
 *
 **/

#include "src/client.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "base/log.h"
#include "base/string_util.h"
#include "thirdlibs/gflags/gflags.h"
#include "thirdlibs/glog/logging.h"


DEFINE_int32(max_head_buffer_size, 20 * 1024, "");
DEFINE_int32(max_body_buffer_size, 2 * 1024 * 1024, "");
DEFINE_bool(fetcher_print_debug_info, false, "");
DEFINE_string(accept_encoding, "gzip, deflate, sdch", "");
DEFINE_bool(set_accept_encoding, false, "");

_START_CLIENT_NAMESPACE_

// (default)
#define HTTP_CONTENT_TYPE_URL_ENCODED   "application/x-www-form-urlencoded"
// (use for files: picture, mp3, tar-file etc.)
#define HTTP_CONTENT_TYPE_FORM_DATA     "multipart/form-data"
// (use for plain text)
#define HTTP_CONTENT_TYPE_TEXT_PLAIN    "text/plain"
// second request timeout
#define MOVETEMP_TIMEOUT 2000


struct http_request_get {
    struct evhttp_uri *uri;
    struct event_base *base;
    struct evhttp_connection *cn;
    struct evhttp_request *req;
    bool isKeepAlive;
};

struct http_request_post {
    struct evhttp_uri *uri;
    struct event_base *base;
    struct evhttp_connection *cn;
    struct evhttp_request *req;
    bool isKeepAlive;
    char *content_type;
    char *post_data;
};

struct Buffer {
  explicit Buffer(size_t max_buf)
    : is_full(false), max_size(max_buf) {}
  void Reset() {
    is_full = false;
    data.clear();
  }

  bool is_full;
  size_t max_size;
  string data;
};

/************************** Ahead Declare ******************************/
//static void http_requset_post_cb(struct evhttp_request *req, void *arg);
//static void http_requset_get_cb(struct evhttp_request *req, void *arg);
//static int start_url_request(struct http_request_get *http_req,
//                             int req_get_flag,
//                             int conn_time);

/************************** Tools Function ******************************/
void Client::print_request_head_info(struct evkeyvalq *header) {
  struct evkeyval *first_node = header->tqh_first;
  while (first_node) {
    LOG(INFO) << "key:" << first_node->key <<" value:" << first_node->value;
    first_node = first_node->next.tqe_next;
  }
}

void Client::print_uri_parts_info(const struct evhttp_uri * http_uri) {
  LOG(INFO) << "scheme:" << evhttp_uri_get_scheme(http_uri);
  LOG(INFO) << "host:" << evhttp_uri_get_host(http_uri);
  LOG(INFO) << "path:" << evhttp_uri_get_path(http_uri);
  LOG(INFO) << "port:" << evhttp_uri_get_port(http_uri);
  LOG(INFO) << "query:" << evhttp_uri_get_query(http_uri);
  LOG(INFO) << "userinfo:" << evhttp_uri_get_userinfo(http_uri);
  LOG(INFO) << "fragment:" << evhttp_uri_get_fragment(http_uri);
}

int Client::start_url_request(struct http_request_get *http_req,
                              int req_get_flag,
                              int conn_time,
                              bool isKeepAlive) {
  if (http_req->cn) {
    LOG(INFO) << "evhttp connection free";
    evhttp_connection_free(http_req->cn);
  }
  int port = evhttp_uri_get_port(http_req->uri);
  http_req->cn = evhttp_connection_base_new(http_req->base,
                                            NULL,
                                            evhttp_uri_get_host(http_req->uri),
                                            (port == -1 ? 80 : port));
  /**
   * Request will be released by evhttp connection
   * See info of evhttp_make_request()
   */
  if (req_get_flag == EVHTTP_REQ_POST) {
    LOG(INFO) << "POST Method";
    http_req->req = evhttp_request_new(http_requset_post_cb, http_req);
  } else if (req_get_flag == EVHTTP_REQ_GET) {
    LOG(INFO) << "GET Method";
    http_req->req = evhttp_request_new(http_requset_get_cb, http_req);
  }


  if (req_get_flag == EVHTTP_REQ_POST) {
    const char *path = evhttp_uri_get_path(http_req->uri);
    string path_post = path ? path : "/";
    evhttp_make_request(http_req->cn, http_req->req, EVHTTP_REQ_POST,
                        reinterpret_cast<const char*>(path_post.c_str()));
    /** Set the post data */
    struct http_request_post *http_req_post =
        (struct http_request_post *) http_req;
    evbuffer_add(http_req_post->req->output_buffer, http_req_post->post_data,
                 strlen(http_req_post->post_data));
    evhttp_add_header(http_req_post->req->output_headers, "Content-Type",
                      http_req_post->content_type);
  } else if (req_get_flag == EVHTTP_REQ_GET) {
    const char *query = evhttp_uri_get_query(http_req->uri);
    const char *path = evhttp_uri_get_path(http_req->uri);
    size_t len = (query ? strlen(query) : 0) + (path ? strlen(path) : 0) + 1;
    char *path_query = NULL;
    if (len > 1) {
      path_query = (char*)calloc(len, sizeof(char));
      sprintf(path_query, "%s?%s", path, query);
    }
    evhttp_make_request(http_req->cn, http_req->req, EVHTTP_REQ_GET,
                        path_query ? path_query : "/");
    free(path_query);
  }
  /** Set the header properties */
  if (isKeepAlive) {
    LOG(INFO) << "KEEPALIVE";
    evhttp_add_header(http_req->req->input_headers, "Connection", "keep-alive");
  }

  evhttp_add_header(http_req->req->output_headers, "Host",
                    evhttp_uri_get_host(http_req->uri));
  evhttp_connection_set_timeout(http_req->cn, conn_time);
  return 0;
}

/************************** Request Function ******************************/
void Client::http_requset_post_cb(struct evhttp_request *req, void *arg) {
  struct http_request_post *http_req_post = (struct http_request_post *) arg;
  switch (req->response_code) {
    case HTTP_OK: {
      struct evbuffer* buf = evhttp_request_get_input_buffer(req);
      size_t len = evbuffer_get_length(buf);
      LOG(INFO) << "print the head info:";
      print_request_head_info(req->output_headers);

      LOG(INFO) << "len :" << len << " body size" << req->body_size;
      char* tmp = (char* )malloc(len + 1);
      memcpy(tmp, evbuffer_pullup(buf, -1), len);
      tmp[len] = '\0';
      LOG(INFO) <<"print the body:";
      LOG(INFO) << "HTML BODY:" <<  tmp;
      body_write_buffer_ = tmp;
      response_code_ = HTTP_OK;
      head_write_buffer_ = "";
      free(tmp);

      event_base_loopexit(http_req_post->base, 0);
      break;
    }
    case HTTP_MOVEPERM: {
      LOG(INFO) << "the uri moved permanently";
      body_write_buffer_ = "";
      head_write_buffer_ = "";
      response_code_ = HTTP_MOVEPERM;
      break;
    }
    case HTTP_MOVETEMP: {
      const char *new_location = evhttp_find_header(req->input_headers,
                                                    "Location");
      struct evhttp_uri *new_uri = evhttp_uri_parse(new_location);
      evhttp_uri_free(http_req_post->uri);
      http_req_post->uri = new_uri;
      start_url_request((struct http_request_get *) http_req_post,
                        EVHTTP_REQ_POST,
                        MOVETEMP_TIMEOUT,
                        http_req_post->isKeepAlive);
      break;
    }

    default:
      event_base_loopexit(http_req_post->base, 0);
  }
  return;
}
void Client::http_requset_get_cb(struct evhttp_request *req, void *arg) {
  struct http_request_get *http_req_get = (struct http_request_get *) arg;

  switch (req->response_code) {
    case HTTP_OK: {
      struct evbuffer* buf = evhttp_request_get_input_buffer(req);
      size_t len = evbuffer_get_length(buf);
      LOG(INFO) << "print the head info:";
      print_request_head_info(req->output_headers);

      LOG(INFO) << "len :" << len << " body size" << req->body_size;
      char *tmp = (char *)malloc(len + 1);
      memcpy(tmp, evbuffer_pullup(buf, -1), len);
      tmp[len] = '\0';
      LOG(INFO) <<"print the body:";
      LOG(INFO) << "HTML BODY:" <<  tmp;
      body_write_buffer_ = tmp;
      response_code_ = HTTP_OK;
      head_write_buffer_ = "";
      free(tmp);
      event_base_loopexit(http_req_get->base, 0);
      break;
    }
    case HTTP_MOVEPERM: {
      LOG(INFO) << "the uri moved permanently";
      body_write_buffer_ = "";
      head_write_buffer_ = "";
      response_code_ = HTTP_MOVEPERM;
      event_base_loopexit(http_req_get->base, 0);
      break;
    }
    case HTTP_MOVETEMP: {
      const char *new_location = evhttp_find_header(req->input_headers,
                                                    "Location");
      struct evhttp_uri *new_uri = evhttp_uri_parse(new_location);
      evhttp_uri_free(http_req_get->uri);
      http_req_get->uri = new_uri;
      start_url_request(http_req_get, EVHTTP_REQ_GET, MOVETEMP_TIMEOUT,
                        http_req_get->isKeepAlive);
      break;
    }

    default:
      event_base_loopexit(http_req_get->base, 0);
  }
  return;
}

/************************** New/Free Function ******************************/
/**
 * @param get_flag: refer REQUEST_GET_*
 *
 */
void* Client::http_request_new(struct event_base* base,
                               const char *url,
                               int req_get_flag,
                               const char *content_type,
                               const char* data,
                               bool isKeepAlive) {
  int len = 0;
  if (req_get_flag == EVHTTP_REQ_GET) {
    len = sizeof(struct http_request_get);
  } else if (req_get_flag == EVHTTP_REQ_POST) {
    len = sizeof(struct http_request_post);
  }

  struct http_request_get *http_req_get = (client::http_request_get*)calloc(1, len);
  http_req_get->uri = evhttp_uri_parse(url);
  print_uri_parts_info(http_req_get->uri);

  http_req_get->base = base;
  http_req_get->isKeepAlive = isKeepAlive;

  if (req_get_flag == EVHTTP_REQ_POST) {
    struct http_request_post *http_req_post =
        (struct http_request_post *) http_req_get;

    if (content_type == NULL) {
      content_type = HTTP_CONTENT_TYPE_URL_ENCODED;
    }
    http_req_post->content_type = strdup(content_type);

    if (data == NULL) {
      http_req_post->post_data = NULL;
    } else {
      http_req_post->post_data = strdup(data);
    }
  }

  return http_req_get;
}

void Client::http_request_free(struct http_request_get *http_req_get,
                               int req_get_flag) {
  evhttp_connection_free(http_req_get->cn);
  evhttp_uri_free(http_req_get->uri);
  if (req_get_flag == EVHTTP_REQ_GET) {
    free(http_req_get);
  } else if (req_get_flag == EVHTTP_REQ_POST) {
    struct http_request_post *http_req_post =
        (struct http_request_post*) http_req_get;
    if (http_req_post->content_type) {
      free(http_req_post->content_type);
    }
    if (http_req_post->post_data) {
      free(http_req_post->post_data);
    }
    free(http_req_post);
  }
  http_req_get = NULL;
}

/************************** Start POST/GET Function ******************************/
/**
 * @param content_type: refer HTTP_CONTENT_TYPE_*
 */
bool Client::start_http_requset(struct event_base* base,
                                const char *url,
                                int req_get_flag,
                                const char *content_type,
                                const char* data,
                                bool isKeepAlive) {
  http_req_get = (struct http_request_get*)http_request_new(base, url,
                                                           req_get_flag,
                                                           content_type,
                                                           data,
                                                           isKeepAlive);

  start_url_request(http_req_get, req_get_flag, connection_time_,isKeepAlive);
  return true;
}

string Client::body_write_buffer_ = "";
string Client::head_write_buffer_ = "";
int Client::response_code_ = HTTP_SERVUNAVAIL;

Client::Client()
    : http_req_post(NULL),
      http_req_get(NULL),
      time_(3000),
      connection_time_(3000),
      isConnectionKeepAlive_(false){
  Reset();
}

void Client::SetHttpMethod(evhttp_cmd_type type) {
  method_ = type;
}

int Client::GetResponseCode() const {
  return response_code_;
}

void Client::SetPostParams(const map<string, string>& m) {
  if (m.empty()) {
    return;
  }

  for (auto it = m.begin(); it != m.end(); ++it) {
    post_data_.append(it->first + "=" + it->second);
    post_data_.append("&");
  }
  // Remove last '&'
  post_data_.resize(post_data_.size() - 1);
}

void Client::SetPostData(const string& data) {
  post_data_ = data;
}

Client::~Client() {
  if (http_req_post != NULL) {
    http_request_free((struct http_request_get *)http_req_post, EVHTTP_REQ_POST);
    //free(http_req_post);
  }

  if (http_req_get != NULL) {
    http_request_free(http_req_get, EVHTTP_REQ_GET);
    //free(http_req_get);
  }
}

const string& Client::ResponseHeader() const {
  return head_write_buffer_;
}

const string& Client::ResponseBody() const {
  return body_write_buffer_;
}


// Set options.
//void Client::SetUserAgent(const string& user_agent) {
//  curl_easy_setopt(curl_handle_, CURLOPT_USERAGENT, user_agent.c_str());
//}

void Client::SetConnectTimeout(int time_ms) {
  connection_time_ = time_ms;
}

void Client::SetFetchTimeout(int time_second) {
  time_ = time_second;
}

void Client::SetConnectionKeepAlive(bool isKeepAlive) {
  this->isConnectionKeepAlive_ = isKeepAlive;
}

bool Client::GetConnectionKeepAlive() {
  return isConnectionKeepAlive_;
}


//void Client::SetAuth(const string& user, const string& password) {
//  curl_easy_setopt(curl_handle_, CURLOPT_USERPWD, (user + ":" + password).c_str());
//}


bool Client::IsHeaderTooLarge() const {
  return false;
}

bool Client::IsBodyTooLarge() const {
  return false;
}

void Client::Reset() {
  response_code_ = 0;
  post_data_.clear();
  method_ = EVHTTP_REQ_GET;
  isConnectionKeepAlive_ = false;

  if (http_req_post != NULL) {
    http_request_free((struct http_request_get *)http_req_post, EVHTTP_REQ_POST);
    free(http_req_post);
  }

  if (http_req_get != NULL) {
    http_request_free(http_req_get, EVHTTP_REQ_GET);
    free(http_req_get);
  }
}

bool Client::FetchPostUrl(const string& url) {
  server::EventBaseLoop event_loop;
  SetHttpMethod(EVHTTP_REQ_POST);
  struct timeval tv = {0,time_};
  event_base_loopexit(event_loop.base(),&tv);
  start_http_requset(event_loop.base(),
                     url.c_str(),
                     EVHTTP_REQ_POST,
                     HTTP_CONTENT_TYPE_URL_ENCODED,
                     post_data_.c_str(),
                     isConnectionKeepAlive_);
  event_loop.Dispatch();
  event_base_free(event_loop.base());
  return true;
}

bool Client::FetchGetUrl(const string& url) {
  server::EventBaseLoop event_loop;
  SetHttpMethod(EVHTTP_REQ_GET);
  struct timeval tv = {0,time_};
  event_base_loopexit(event_loop.base(),&tv);
  start_http_requset(event_loop.base(),
                     url.c_str(),
                     EVHTTP_REQ_GET,
                     NULL,
                     NULL,
                     isConnectionKeepAlive_);
  event_loop.Dispatch();
  event_base_free(event_loop.base());
  return true;
}
_END_CLIENT_NAMESPACE_  // namespace CLIENT

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */ 

