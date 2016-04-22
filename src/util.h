/***************************************************************************
 * 
 * Copyright (c) 2016 evelio, Inc. All Rights Reserved
 *
 * *************************************************************************/

/**
 * @file:   util.h
 * @author: sunqiang(evelio@126.com)
 * @date:   2016-04-22 11:33:49
 * @brief
 *
 **/
#ifndef SERVER_INCLUDE_UTIL_H_
#define SERVER_INCLUDE_UTIL_H_

#include <string.h>
#include "config.h"

_START_SERVER_NAMESPACE_

static const struct table_entry {
  const char *extension;
  const char *content_type;
} content_type_table[] = {
    { "txt", "text/plain" },
    { "c", "text/plain" },
    { "h", "text/plain" },
    { "html", "text/html" },
    { "htm", "text/htm" },
    { "css", "text/css" },
    { "gif", "image/gif" },
    { "jpg", "image/jpeg" },
    { "jpeg", "image/jpeg" },
    { "png", "image/png" },
    { "pdf", "application/pdf" },
    { "ps", "application/postsript" },
    { NULL, NULL },
};

static const char *informational_phrases[] = {
    /* 100 */ "Continue",
    /* 101 */ "Switching Protocols"
};

static const char *success_phrases[] = {
    /* 200 */ "OK",
    /* 201 */ "Created",
    /* 202 */ "Accepted",
    /* 203 */ "Non-Authoritative Information",
    /* 204 */ "No Content",
    /* 205 */ "Reset Content",
    /* 206 */ "Partial Content"
};

static const char *redirection_phrases[] = {
    /* 300 */ "Multiple Choices",
    /* 301 */ "Moved Permanently",
    /* 302 */ "Found",
    /* 303 */ "See Other",
    /* 304 */ "Not Modified",
    /* 305 */ "Use Proxy",
    /* 307 */ "Temporary Redirect"
};


static const char *client_error_phrases[] = {
    /* 400 */ "Bad Request",
    /* 401 */ "Unauthorized",
    /* 402 */ "Payment Required",
    /* 403 */ "Forbidden",
    /* 404 */ "Not Found",
    /* 405 */ "Method Not Allowed",
    /* 406 */ "Not Acceptable",
    /* 407 */ "Proxy Authentication Required",
    /* 408 */ "Request Time-out",
    /* 409 */ "Conflict",
    /* 410 */ "Gone",
    /* 411 */ "Length Required",
    /* 412 */ "Precondition Failed",
    /* 413 */ "Request Entity Too Large",
    /* 414 */ "Request-URI Too Large",
    /* 415 */ "Unsupported Media Type",
    /* 416 */ "Requested range not satisfiable",
    /* 417 */ "Expectation Failed"
};

static const char *server_error_phrases[] = {
    /* 500 */ "Internal Server Error",
    /* 501 */ "Not Implemented",
    /* 502 */ "Bad Gateway",
    /* 503 */ "Service Unavailable",
    /* 504 */ "Gateway Time-out",
    /* 505 */ "HTTP Version not supported"
};

/* Try to guess a good content-type for 'path' */
static const char *
guess_content_type(const char *path) {
  const char *last_period, *extension;
  const struct table_entry *ent;
  last_period = strrchr(path, '.');
  if (!last_period || strchr(last_period, '/'))
    goto not_found;
  /* no exension */
  extension = last_period + 1;
  for (ent = &content_type_table[0]; ent->extension; ++ent) {
    if (!evutil_ascii_strcasecmp(ent->extension, extension))
      return ent->content_type;
  }
not_found:
  return "application/misc";
}

struct response_class {
    const char *name;
    size_t num_responses;
    const char **responses;
};

static const struct response_class response_classes[] = {
    /* 1xx */ { "Informational", MEMBERSOF(informational_phrases), informational_phrases },
    /* 2xx */ { "Success", MEMBERSOF(success_phrases), success_phrases },
    /* 3xx */ { "Redirection", MEMBERSOF(redirection_phrases), redirection_phrases },
    /* 4xx */ { "Client Error", MEMBERSOF(client_error_phrases), client_error_phrases },
    /* 5xx */ { "Server Error", MEMBERSOF(server_error_phrases), server_error_phrases }
};

static const char *
evhttp_response_phrase_internal(int code)
{
    int klass = code / 100 - 1;
    int subcode = code % 100;

    /* Unknown class - can't do any better here */
    if (klass < 0 || klass >= (int) MEMBERSOF(response_classes))
        return "Unknown Status Class";

    /* Unknown sub-code, return class name at least */
    if (subcode >= (int) response_classes[klass].num_responses)
        return response_classes[klass].name;

    return response_classes[klass].responses[subcode];
}

_END_SERVER_NAMESPACE_


#endif  // SERVER_INCLUDE_UTIL_H_

