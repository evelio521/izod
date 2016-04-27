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

string Handler::GetClassName() {
  return "Handler";
}

DefaultHandler::DefaultHandler(string regpath)
  : Handler(regpath) {
}

DefaultHandler::~DefaultHandler() {
}

string DefaultHandler::GetPathName() {
  return path;
}

bool DefaultHandler::HttpHandler(Request* request,
                             Response* response) {
  request->Dump();
  //response->AppendBuffer("No Service!");
  In in;
  request->GetQueryParams(&in);
  if (Excute(request, response, in)) {
    return response->SendToClient();
  }
  return false;
}

string DefaultHandler::GetClassName() {
  return "DefaultHandler";
}

JsonHandler::JsonHandler(string regpath)
  : Handler(regpath) {
}

JsonHandler::~JsonHandler() {
}

string JsonHandler::GetPathName() {
  return path;
}

bool JsonHandler::HttpHandler(Request* request,
                             Response* response) {
  request->Dump();
  response->SetJsonContentType();
  In in;
  request->GetQueryParams(&in);
  if (Excute(request, response, in)) {
    return response->SendToClient();
  }
  response->AppendBuffer("{\"error\":\"bad json\"}");

  return false;
}

string JsonHandler::GetClassName() {
  return "JsonHandler";
}

BinaryHandler::BinaryHandler(string regpath)
  : Handler(regpath) {
}

BinaryHandler::~BinaryHandler() {
}

string BinaryHandler::GetPathName() {
  return path;
}

bool BinaryHandler::HttpHandler(Request* request,
                             Response* response) {
  request->Dump();
  response->SetBinaryContentType();
  In in;
  request->GetQueryParams(&in);
  if (Excute(request, response, in)) {
    return response->SendToClient();
  }
  string str = "bad buffer";
  response->AppendBuffer(str);
  return false;
}

string BinaryHandler::GetClassName() {
  return "BinaryHandler";
}

_END_SERVER_NAMESPACE_
