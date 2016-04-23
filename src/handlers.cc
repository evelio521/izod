/*******************************************************************************
 *
 * Copyright (c) 2016 evelio, Inc. All Rights Reserved
 *
 *******************************************************************************/



/**
 * @file:   handlers.cc
 * @author: sunqiang(evelio@126.com)
 * @date:   2016-04-23 11:03:07
 * @brief
 *
 **/


#include "src/handlers.h"

#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <string>

#include "base/log.h"
#include "base/string_util.h"
#include "src/util.h"
#include "src/request.h"
#include "src/response.h"
#include "thirdlibs/gflags/gflags.h"

_START_SERVER_NAMESPACE_

Handler::Handler(string regpath)
  : path(regpath) {
}

Handler::~Handler() {
}

JsonHandler::JsonHandler(string regpath)
  : Handler(regpath) {
}

JsonHandler::~JsonHandler() {
}

int JsonHandler::HttpHandler(Request* request,
                             Response* response) {
  return HTTP_BADREQUEST;
}


_END_SERVER_NAMESPACE_
