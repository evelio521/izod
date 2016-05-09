/*******************************************************************************
 *
 * Copyright (c) 2016 evelio, Inc. All Rights Reserved
 *
 *******************************************************************************/



/**
 * @file:   response.cc
 * @author: sunqiang(evelio@126.com)
 * @date:   2016-04-23 10:00:39
 * @brief
 *
 **/

#include "src/response.h"

#include "base/marcos.h"
#include "base/basictypes.h"
#include "base/compat.h"
#include "src/request.h"
#include "src/util.h"
#include "thirdlibs/event/include/event2/buffer.h"
#include "thirdlibs/event/include/event2/http.h"
#include "thirdlibs/event/include/event2/http_struct.h"

_START_SERVER_NAMESPACE_

Response::Response(Request* request)
    : request_(request), code_(HTTP_OK) {
}

Response::~Response() {
}

void Response::AppendHeader(const string& key, const string& value) {
    evhttp_add_header(request_->request()->output_headers,
                      key.c_str(), value.c_str());
}

void Response::SetJsonContentType() {
  AppendHeader("Content-Type", "application/json; charset=UTF-8");
}

/*
 * Please look up base/mime.types
 */
void Response::SetReturnType(string header, string content) {
  AppendHeader(header, content);
}

void Response::SetHtmlContentType() {
  AppendHeader("Content-Type", "text/html; charset=utf-8");
}

void Response::SetBinaryContentType() {
  AppendHeader("Content-Type", "application/octet-stream charset=utf-8");
}

void Response::AppendBuffer(const string& buff) {
    content_.append(buff);
}

/* Response codes */
// #define HTTP_OK                 200 /**< request completed ok */
// #define HTTP_NOCONTENT          204 /**< request does not have content */
// #define HTTP_MOVEPERM           301 /**< the uri moved permanently */
// #define HTTP_MOVETEMP           302 /**< the uri moved temporarily */
// #define HTTP_NOTMODIFIED        304 /**< page was not modified from last */
// #define HTTP_BADREQUEST         400 /**< invalid http request was made */
// #define HTTP_NOTFOUND           404 /**< could not find content for uri */
// #define HTTP_BADMETHOD          405 /**< method not allowed for this uri */
// #define HTTP_ENTITYTOOLARGE     413 /**<  */
// #define HTTP_EXPECTATIONFAILED  417 /**< we can't handle this expectation */
// #define HTTP_INTERNAL           500     /**< internal error */
// #define HTTP_NOTIMPLEMENTED     501     /**< not implemented */
// #define HTTP_SERVUNAVAIL        503 /**< the server is not available */

void Response::SetResponseCode(int code) {
  code_ = code;
}

bool Response::SendToClient() {
    struct evbuffer *evb = evbuffer_new();
    evbuffer_add(evb, content_.data(), content_.size());
    evhttp_send_reply(request_->request(),
                      code_,
                      GetStatusInfo(code_).c_str(),
                      evb);
    evbuffer_free(evb);
    return true;
}

_END_SERVER_NAMESPACE_
