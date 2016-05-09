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
#include "thirdlibs/event/include/event2/event.h"
#include "thirdlibs/event/include/event2/buffer.h"
#include "thirdlibs/event/include/event2/http.h"
#include "thirdlibs/event/include/event2/http_struct.h"
#include "thirdlibs/event/include/event2/keyvalq_struct.h"

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

#define REQUEST_POST_FLAG               2
#define REQUEST_GET_FLAG                3

struct http_request_get {
    struct evhttp_uri *uri;
    struct event_base *base;
    struct evhttp_connection *cn;
    struct evhttp_request *req;
};

struct http_request_post {
    struct evhttp_uri *uri;
    struct event_base *base;
    struct evhttp_connection *cn;
    struct evhttp_request *req;
    char *content_type;
    char *post_data;
};

/************************** Ahead Declare ******************************/
void http_requset_post_cb(struct evhttp_request *req, void *arg);
void http_requset_get_cb(struct evhttp_request *req, void *arg);
int start_url_request(struct http_request_get *http_req, int req_get_flag);

/************************** Tools Function ******************************/
static inline void print_request_head_info(struct evkeyvalq *header)
{
    struct evkeyval *first_node = header->tqh_first;
    while (first_node) {
        MITLog_DetPrintf(MITLOG_LEVEL_COMMON,"key:%s  value:%s", first_node->key, first_node->value);
        first_node = first_node->next.tqe_next;
    }
}

static inline void print_uri_parts_info(const struct evhttp_uri * http_uri)
{
    MITLog_DetPrintf(MITLOG_LEVEL_COMMON,"scheme:%s", evhttp_uri_get_scheme(http_uri));
    MITLog_DetPrintf(MITLOG_LEVEL_COMMON,"host:%s", evhttp_uri_get_host(http_uri));
    MITLog_DetPrintf(MITLOG_LEVEL_COMMON,"path:%s", evhttp_uri_get_path(http_uri));
    MITLog_DetPrintf(MITLOG_LEVEL_COMMON,"port:%d", evhttp_uri_get_port(http_uri));
    MITLog_DetPrintf(MITLOG_LEVEL_COMMON,"query:%s", evhttp_uri_get_query(http_uri));
    MITLog_DetPrintf(MITLOG_LEVEL_COMMON,"userinfo:%s", evhttp_uri_get_userinfo(http_uri));
    MITLog_DetPrintf(MITLOG_LEVEL_COMMON,"fragment:%s", evhttp_uri_get_fragment(http_uri));
}

/************************** Request Function ******************************/
void http_requset_post_cb(struct evhttp_request *req, void *arg)
{
    struct http_request_post *http_req_post = (struct http_request_post *)arg;
    switch(req->response_code)
    {
        case HTTP_OK:
        {
            struct evbuffer* buf = evhttp_request_get_input_buffer(req);
            size_t len = evbuffer_get_length(buf);
            MITLog_DetPuts(MITLOG_LEVEL_COMMON, "print the head info:");
            print_request_head_info(req->output_headers);

            MITLog_DetPrintf(MITLOG_LEVEL_COMMON,"len:%zu  body size:%zu", len, req->body_size);
            char *tmp = malloc(len+1);
            memcpy(tmp, evbuffer_pullup(buf, -1), len);
            tmp[len] = '\0';
            MITLog_DetPuts(MITLOG_LEVEL_COMMON, "print the body:");
            MITLog_DetPrintf(MITLOG_LEVEL_COMMON,"HTML BODY:%s", tmp);
            free(tmp);

            event_base_loopexit(http_req_post->base, 0);
            break;
        }
        case HTTP_MOVEPERM:
            MITLog_DetPrintf(MITLOG_LEVEL_ERROR, "%s", "the uri moved permanently");
            break;
        case HTTP_MOVETEMP:
        {
            const char *new_location = evhttp_find_header(req->input_headers, "Location");
            struct evhttp_uri *new_uri = evhttp_uri_parse(new_location);
            evhttp_uri_free(http_req_post->uri);
            http_req_post->uri = new_uri;
            start_url_request((struct http_request_get *)http_req_post, REQUEST_POST_FLAG);
            return;
        }

        default:
            event_base_loopexit(http_req_post->base, 0);
            return;
    }
}
void http_requset_get_cb(struct evhttp_request *req, void *arg)
{
    struct http_request_get *http_req_get = (struct http_request_get *)arg;
    switch(req->response_code)
    {
        case HTTP_OK:
        {
            struct evbuffer* buf = evhttp_request_get_input_buffer(req);
            size_t len = evbuffer_get_length(buf);
            MITLog_DetPuts(MITLOG_LEVEL_COMMON, "print the head info:");
            print_request_head_info(req->output_headers);

            MITLog_DetPrintf(MITLOG_LEVEL_COMMON,"len:%zu  body size:%zu", len, req->body_size);
            char *tmp = malloc(len+1);
            memcpy(tmp, evbuffer_pullup(buf, -1), len);
            tmp[len] = '\0';
            MITLog_DetPuts(MITLOG_LEVEL_COMMON, "print the body:");
            MITLog_DetPrintf(MITLOG_LEVEL_COMMON,"HTML BODY:%s", tmp);
            free(tmp);

            event_base_loopexit(http_req_get->base, 0);
            break;
        }
        case HTTP_MOVEPERM:
            MITLog_DetPrintf(MITLOG_LEVEL_ERROR, "%s", "the uri moved permanently");
            break;
        case HTTP_MOVETEMP:
        {
            const char *new_location = evhttp_find_header(req->input_headers, "Location");
            struct evhttp_uri *new_uri = evhttp_uri_parse(new_location);
            evhttp_uri_free(http_req_get->uri);
            http_req_get->uri = new_uri;
            start_url_request(http_req_get, REQUEST_GET_FLAG);
            return;
        }

        default:
            event_base_loopexit(http_req_get->base, 0);
            return;
    }
}

int start_url_request(struct http_request_get *http_req, int req_get_flag)
{
    if (http_req->cn)
        evhttp_connection_free(http_req->cn);

    int port = evhttp_uri_get_port(http_req->uri);
    http_req->cn = evhttp_connection_base_new(http_req->base,
                                                   NULL,
                                                   evhttp_uri_get_host(http_req->uri),
                                                   (port == -1 ? 80 : port));

    /**
     * Request will be released by evhttp connection
     * See info of evhttp_make_request()
     */
    if (req_get_flag == REQUEST_POST_FLAG) {
        http_req->req = evhttp_request_new(http_requset_post_cb, http_req);
    } else if (req_get_flag ==  REQUEST_GET_FLAG) {
        http_req->req = evhttp_request_new(http_requset_get_cb, http_req);
    }

    if (req_get_flag == REQUEST_POST_FLAG) {
        const char *path = evhttp_uri_get_path(http_req->uri);
        evhttp_make_request(http_req->cn, http_req->req, EVHTTP_REQ_POST,
                            path ? path : "/");
        /** Set the post data */
        struct http_request_post *http_req_post = (struct http_request_post *)http_req;
        evbuffer_add(http_req_post->req->output_buffer, http_req_post->post_data, strlen(http_req_post->post_data));
        evhttp_add_header(http_req_post->req->output_headers, "Content-Type", http_req_post->content_type);
    } else if (req_get_flag == REQUEST_GET_FLAG) {
        const char *query = evhttp_uri_get_query(http_req->uri);
        const char *path = evhttp_uri_get_path(http_req->uri);
        size_t len = (query ? strlen(query) : 0) + (path ? strlen(path) : 0) + 1;
        char *path_query = NULL;
        if (len > 1) {
            path_query = calloc(len, sizeof(char));
            sprintf(path_query, "%s?%s", path, query);
        }
        evhttp_make_request(http_req->cn, http_req->req, EVHTTP_REQ_GET,
                             path_query ? path_query: "/");
    }
    /** Set the header properties */
    evhttp_add_header(http_req->req->output_headers, "Host", evhttp_uri_get_host(http_req->uri));

    return 0;
}

/************************** New/Free Function ******************************/
/**
 * @param get_flag: refer REQUEST_GET_*
 *
 */
void *http_request_new(struct event_base* base, const char *url, int req_get_flag, \
                       const char *content_type, const char* data)
{
    int len = 0;
    if (req_get_flag == REQUEST_GET_FLAG) {
        len = sizeof(struct http_request_get);
    } else if(req_get_flag == REQUEST_POST_FLAG) {
        len = sizeof(struct http_request_post);
    }

    struct http_request_get *http_req_get = calloc(1, len);
    http_req_get->uri = evhttp_uri_parse(url);
    print_uri_parts_info(http_req_get->uri);

    http_req_get->base = base;

    if (req_get_flag == REQUEST_POST_FLAG) {
        struct http_request_post *http_req_post = (struct http_request_post *)http_req_get;
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

void http_request_free(struct http_request_get *http_req_get, int req_get_flag)
{
    evhttp_connection_free(http_req_get->cn);
    evhttp_uri_free(http_req_get->uri);
    if (req_get_flag == REQUEST_GET_FLAG) {
        free(http_req_get);
    } else if(req_get_flag == REQUEST_POST_FLAG) {
        struct http_request_post *http_req_post = (struct http_request_post*)http_req_get;
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
void *start_http_requset(struct event_base* base, const char *url, int req_get_flag, \
                                                const char *content_type, const char* data)
{
    struct http_request_get *http_req_get = http_request_new(base, url, req_get_flag, content_type, data);
    start_url_request(http_req_get, req_get_flag);

    return http_req_get;
}




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

static size_t WriteMemoryCallback(void *contents,
                                  size_t size,
                                  size_t nmemb,
                                  void *userp) {
  Buffer *mem = reinterpret_cast<Buffer*>(userp);
  if (mem->is_full) {
    return 0;
  }

  size_t realsize = size * nmemb;
  mem->data.append(reinterpret_cast<const char*>(contents), realsize);
  if (mem->data.size() > mem->max_size) {
    mem->data.resize(mem->max_size);
    mem->is_full = true;
  }
  return realsize;
}

Client::Client()
    : response_code_(0) {
  head_write_buffer_.reset(new Buffer(FLAGS_max_head_buffer_size));
  body_write_buffer_.reset(new Buffer(FLAGS_max_body_buffer_size));

  Reset();
}

void Client::SetHttpMethod(HttpMethod type) {
  method_ = type;
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

Client::~Client() {}

const string& Client::ResponseHeader() const {
  return head_write_buffer_->data;
}

const string& Client::ResponseBody() const {
  return body_write_buffer_->data;
}

// Set header
void Client::AddHeader(const string& key, const string& value) {
  string line = StringPrintf("%s:%s", key.c_str(), value.c_str());
  header_list_ = curl_slist_append(header_list_, line.c_str());
}

// Set options.
void Client::SetUserAgent(const string& user_agent) {
  curl_easy_setopt(curl_handle_, CURLOPT_USERAGENT, user_agent.c_str());
}

void Client::SetConnectTimeout(int time_ms) {
  curl_easy_setopt(curl_handle_, CURLOPT_CONNECTTIMEOUT_MS, time_ms);
}

void Client::SetFetchTimeout(int time_ms) {
  curl_easy_setopt(curl_handle_, CURLOPT_TIMEOUT_MS, time_ms);
}

void Client::SetAuth(const string& user, const string& password) {
  curl_easy_setopt(curl_handle_, CURLOPT_USERPWD, (user + ":" + password).c_str());
}


bool Client::IsHeaderTooLarge() const {
  return head_write_buffer_->is_full;
}

bool Client::IsBodyTooLarge() const {
  return body_write_buffer_->is_full;
}

void Client::Reset() {
  response_code_ = 0;
  head_write_buffer_->Reset();
  body_write_buffer_->Reset();

  method_ = HttpMethod::kGet;
  post_data_.clear();
}

bool Client::FetchUrl(const string& url) {
  VLOG(1) << "fetch url:" << url;
  CURLcode code = curl_easy_setopt(curl_handle_, CURLOPT_URL, url.c_str());
  if (code != CURLE_OK) {
    LOG(ERROR) << "bad code:" << code;
    return false;
  }

  if (FLAGS_set_accept_encoding) {
    curl_easy_setopt(curl_handle_, CURLOPT_ACCEPT_ENCODING,
                     FLAGS_accept_encoding.c_str());
  }

  if (method_ == HttpMethod::kGet) {
    curl_easy_setopt(curl_handle_, CURLOPT_HTTPGET, 1);
  } else if (method_ == HttpMethod::kPost) {
    curl_easy_setopt(curl_handle_, CURLOPT_POST, 1);
    curl_easy_setopt(curl_handle_, CURLOPT_POSTFIELDS, post_data_.data());
    curl_easy_setopt(curl_handle_, CURLOPT_POSTFIELDSIZE, post_data_.size());
  } else if (method_ == HttpMethod::kHead) {
    curl_easy_setopt(curl_handle_, CURLOPT_HTTPGET, 0);
    curl_easy_setopt(curl_handle_, CURLOPT_NOBODY, 1);
  }

  if (FLAGS_fetcher_print_debug_info) {
    long verbose = 1;
    curl_easy_setopt(curl_handle_, CURLOPT_VERBOSE, verbose);
  } else {
    long verbose = 0;
    curl_easy_setopt(curl_handle_, CURLOPT_VERBOSE, verbose);
  }

  if (header_list_) {
    VLOG(2) << "Set header list";
    code = curl_easy_setopt(curl_handle_, CURLOPT_HTTPHEADER, header_list_);
    CHECK(code == CURLE_OK);
  }

  curl_code_ = curl_easy_perform(curl_handle_);
  code = curl_easy_getinfo(curl_handle_,
      CURLINFO_RESPONSE_CODE, &response_code_);
  if (code != CURLE_OK) {
    LOG(ERROR) << "Fail to get response code:" << response_code_;
  }

  return curl_code_ == CURLE_OK;
}
_END_CLIENT_NAMESPACE_  // namespace CLIENT

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */ 

