/*******************************************************************************
 *
 * Copyright (c) 2016 evelio, Inc. All Rights Reserved
 *
 *******************************************************************************/



/**
 * @file:   testhandlers.cc
 * @author: sunqiang(evelio@126.com)
 * @date:   2016-04-27 15:14:25
 * @brief
 *
 **/
#include "sample/testhandlers.h"

#include "sample/defs.h"
#include "base/marcos.h"

_START_SERVER_NAMESPACE_

/*
 * Extract paras from args
 */
static void ExtractString(const In& args, const string& name, std::string &value) {
  map<string, string>::const_iterator it = args.find(name);
  if (it != args.end()){
       value = it->second;
  }
}

bool TestDefaultHandler::Excute(Request* request, Response* response,
                                const In &in) {
  string query;
  ExtractString(in, "query", query);
  response->AppendBuffer(query);
  return true;
}

bool TestBinaryHandler::Excute(Request* request, Response* response,
                               const In &in) {
  string query;
  ExtractString(in, "query", query);
  response->AppendBuffer(query);
  return true;
}

bool TestJsonHandler::Excute(Request* request, Response* response,
                             const In &in) {
  string query;
  ExtractString(in, "query", query);
  query = "{ \
  \"firstName\": \"John\",\
  \"lastName\": \"Smith\",\
  \"sex\": \"male\",\
  \"age\": 25\
  }";
  response->AppendBuffer(query);
  return true;
}

_END_SERVER_NAMESPACE_

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */ 

