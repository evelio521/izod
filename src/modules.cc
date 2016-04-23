/*******************************************************************************
 *
 * Copyright (c) 2016 evelio, Inc. All Rights Reserved
 *
 *******************************************************************************/

/**
 * @file:   modules.cc
 * @author: sunqiang(evelio@126.com)
 * @date:   2016-04-23 13:28:30
 * @brief
 *
 **/

#include "src/modules.h"

#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/queue.h>

#include <signal.h>

#include <vector>

#include "base/compat.h"
#include "base/fixed_size_thread_pool.h"
#include "base/log.h"
#include "base/string_util.h"
#include "base/stl_util-inl.h"
#include "base/thread.h"

#include "src/request.h"
#include "src/response.h"
#include "src/handlers.h"
#include "src/util.h"
#include "src/event_base_loop.h"

#include "thirdlibs/event/include/event2/http.h"
#include "thirdlibs/event/include/event2/buffer.h"
#include "thirdlibs/event/include/event2/thread.h"
#include "thirdlibs/event/http-internal.h"
#include "thirdlibs/event/mm-internal.h"
#include "thirdlibs/gflags/gflags.h"
#include "thirdlibs/glog/logging.h"

DEFINE_string(doc_root, "", "");
DEFINE_int32(http_browser_expire_time, 24 * 60 * 60, "Expire time");
DEFINE_int32(listen_port, 8888, "default listen port");
DEFINE_int32(threads_num, 12, "default threads numbers");

_START_SERVER_NAMESPACE_

static string GetExpireString() {
  char expire_time[64];
  time_t now_time = time(NULL);
  now_time += FLAGS_http_browser_expire_time;
  strftime(expire_time, sizeof(expire_time), "%a, %d %b %Y %X GMT",
           gmtime(&now_time));
  return string(expire_time);
}

// This callback gets invoked when we get any http request that doesn't match
// any other callback.  Like any evhttp server callback, it has a simple job:
// it must eventually call evhttp_send_error() or evhttp_send_reply().
void send_document_cb(struct evhttp_request *req, void *arg) {
  struct evbuffer *evb = NULL;
  const char *uri = evhttp_request_get_uri(req);
  struct evhttp_uri *decoded = NULL;
  const char *path;
  char *decoded_path;
  char *whole_path = NULL;
  size_t len;
  int fd = -1;
  struct stat st;

  if (evhttp_request_get_command(req) != EVHTTP_REQ_GET) {
    evhttp_send_reply(req, 200, "OK", NULL);
    return;
  }

  LOG(INFO)<< "Got a GET request for " << uri;

  /* Decode the URI */
  decoded = evhttp_uri_parse(uri);
  if (!decoded) {
    printf("It's not a good URI. Sending BADREQUEST\n");
    evhttp_send_error(req, HTTP_BADREQUEST, 0);
    return;
  }

  /* Let's see what path the user asked for. */
  path = evhttp_uri_get_path(decoded);
  if (!path)
    path = "/";

  /* We need to decode it, to see what path the user really wanted. */
  decoded_path = evhttp_uridecode(path, 0, NULL);
  if (decoded_path == NULL)
    goto err;
  /* Don't allow any ".."s in the path, to avoid exposing stuff outside
   * of the docroot.  This test is both overzealous and underzealous:
   * it forbids aceptable paths like "/this/one..here", but it doesn't
   * do anything to prevent symlink following." */
  if (strstr(decoded_path, ".."))
    goto err;

  len = strlen(decoded_path) + FLAGS_doc_root.length() + 2;
  if (!(whole_path = reinterpret_cast<char*>(malloc(len)))) {
    perror("malloc");
    goto err;
  }
  evutil_snprintf(whole_path, len, "%s/%s", FLAGS_doc_root.c_str(),
                  decoded_path);

  if (stat(whole_path, &st) < 0) {
    goto err;
  }

  /* This holds the content we're sending. */
  evb = evbuffer_new();

  if (S_ISDIR(st.st_mode)) {
    evbuffer_add_printf(evb, "<html><body>Page Not Exist</body></html>\n");
    evhttp_add_header(evhttp_request_get_output_headers(req), "Content-Type",
                      "text/html");
  } else {
    /* Otherwise it's a file; add it to the buffer to get
     * sent via sendfile */
    string type = GetContentType(decoded_path);
    if ((fd = open(whole_path, O_RDONLY)) < 0) {
      perror("open");
      goto err;
    }

    if (fstat(fd, &st) < 0) {
      /* Make sure the length still matches, now that we
       * opened the file :/ */
      perror("fstat");
      goto err;
    }

    string etag = UintToString(st.st_mtime);
    evhttp_add_header(evhttp_request_get_output_headers(req), "Content-Type",
                      type.c_str());
    evhttp_add_header(evhttp_request_get_output_headers(req), "Expires",
                      GetExpireString().c_str());
    evhttp_add_header(evhttp_request_get_output_headers(req), "etag",
                      etag.c_str());
    evkeyvalq* header = evhttp_request_get_input_headers(req);
    const char* old_etag = evhttp_find_header(header, "if-none-match");

    // If etag is same, we can reply a 304
    if (old_etag && strcmp(old_etag, etag.c_str()) == 0) {
      LOG(INFO)<< "Etag match " << old_etag;
      evhttp_send_reply(req, 304, "Not Modified", evb);
      goto done;
    }

    evbuffer_add_file(evb, fd, 0, st.st_size);
  }

  evhttp_send_reply(req, 200, "OK", evb);
  goto done;
  err: evhttp_send_error(req, 404, "Document was not found");
  if (fd >= 0)
    close(fd);
  done: if (decoded)
    evhttp_uri_free(decoded);
  if (decoded_path)
    free(decoded_path);
  if (whole_path)
    free(whole_path);
  if (evb)
    evbuffer_free(evb);
}

Modules::Modules()
    : listen_port_(FLAGS_listen_port),
      threads_num_(FLAGS_threads_num) {
  thread_pool_.reset(new base::FixedSizeThreadPool(threads_num_));
}

Modules::~Modules() {
  base::STLDeleteValues(&handlers_);
}

static void ThreadTask(struct evhttp_request* request, Handler* handler) {
  Request http_request(request);
  Response response(&http_request);
  handler->HttpHandler(&http_request, &response);
}

static void InnerHandlerCallback(struct evhttp_request* request, void* cb) {
  HandlerWrapper* wrapper = reinterpret_cast<HandlerWrapper*>(cb);
  base::ThreadPoolInterface* pool = wrapper->pool;
  LOG(INFO)<< "waiting thread:" << pool->WaitingThreads();
  pool->Add(std::bind(ThreadTask, request, wrapper->handler));
}

bool Modules::RegisterHttpHandler(const string& path, Handler* handler) {
  auto it = handlers_.find(path);
  if (it != handlers_.end()) {
    LOG(ERROR)<< "Handler for path :" << path
    << " exist, fail to register new handler for it";
    return false;
  }
  HandlerWrapper* wrapper = new HandlerWrapper;
  wrapper->handler = handler;
  wrapper->pool = thread_pool_.get();
  handlers_.insert(make_pair(path, wrapper));
  return true;
}

int Modules::BindSocket() {
  int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
  CHECK_GT(listen_fd, -1)<< "Fail to create socket in http server.";

  int one = 1;
  int r = setsockopt(listen_fd,
  SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char *>(&one), sizeof(int));

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(listen_port_);

  r = bind(listen_fd, (struct sockaddr*) &addr, sizeof(addr));
  CHECK_GT(r, -1)<< "Fail to bind port, " << listen_port_;
  r = listen(listen_fd, 10240);
  CHECK_GT(r, -1)<< "Fail to set listen socket in http server.";

  int flags;
  if ((flags = fcntl(listen_fd, F_GETFL, 0)) < 0
      || fcntl(listen_fd, F_SETFL, flags | O_NONBLOCK) < 0) {
    CHECK(false) << "Fail to set nonblock for listen socket in http server.";
  }

  return listen_fd;
}

static void IgnoreSignal(int signum) {
  LOG(WARNING)<< "Caught signal :" << signum << ", ignore it";
}

void Modules::Run(int listen_fd) {
  evthread_use_pthreads();
  EventBaseLoop event_loop;
  evhttp* httpd = evhttp_new(event_loop.base());
  CHECK(httpd) << "Fail to create evhttp in http server.";
  int r = evhttp_accept_socket(httpd, listen_fd);
  CHECK_GT(r, -1)<< "Fail to accept socket in http server.";

  for (auto iter = handlers_.begin(); iter != handlers_.end(); ++iter) {
    evhttp_set_cb(httpd, (iter->first).c_str(), InnerHandlerCallback,
                  static_cast<void*>(iter->second));
  }
  event_loop.Dispatch();
}

void Modules::Server() {
  // Equal to signal(SIGPIPE, SIG_IGN);
  // When client is closed unexpectedly, write bufferevent_writecb in libevent
  // will arise SIGPIPE signal.
  signal(SIGPIPE, IgnoreSignal);

  int listen_fd = BindSocket();
  ShowBindInfo(listen_fd);
  Run(listen_fd);

  LOG(WARNING)<< "http server exit.";
}

void Modules::ShowBindInfo(int listen_fd) {
  // Extract and display the address we're listening on.
  struct sockaddr_storage ss;
  ev_socklen_t socklen = sizeof(ss);
  memset(&ss, 0, sizeof(ss));
  if (getsockname(listen_fd, (struct sockaddr *) &ss, &socklen)) {
    perror("getsockname() failed");
    return;
  }

  void *inaddr;
  int got_port = -1;
  if (ss.ss_family == AF_INET) {
    got_port = ntohs(((struct sockaddr_in*) &ss)->sin_port);
    inaddr = &((struct sockaddr_in*) &ss)->sin_addr;
  } else if (ss.ss_family == AF_INET6) {
    got_port = ntohs(((struct sockaddr_in6*) &ss)->sin6_port);
    inaddr = &((struct sockaddr_in6*) &ss)->sin6_addr;
  } else {
    fprintf(stderr, "Weird address family %d\n", ss.ss_family);
    return;
  }

  char addrbuf[128];
  const char* addr = evutil_inet_ntop(ss.ss_family, inaddr, addrbuf,
                                      sizeof(addrbuf));
  if (addr) {
    LOG(INFO)<<"Listening on " << addr << ":" << got_port;
    uri_root_ = StringPrintf("http://%s:%d", addr, got_port);
  } else {
    LOG(ERROR) << "evutil_inet_ntop failed";
    return;
  }
}

_END_SERVER_NAMESPACE_
