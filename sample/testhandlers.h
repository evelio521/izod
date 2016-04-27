/***************************************************************************
 * 
 * Copyright (c) 2016 evelio, Inc. All Rights Reserved
 *
 * *************************************************************************/

/**
 * @file:   testhandlers.h
 * @author: sunqiang(evelio@126.com)
 * @date:   2016-04-27 15:13:38
 * @brief
 *
 **/
#ifndef SERVER_SAMPLE_TESTHANDLERS_H_
#define SERVER_SAMPLE_TESTHANDLERS_H_

#include "src/handlers.h"
#include "sample/defs.h"
#include "base/marcos.h"
#include "src/request.h"
#include "src/response.h"

_START_SERVER_NAMESPACE_

class TestDefaultHandler : public DefaultHandler {
 public:
  TestDefaultHandler():DefaultHandler(cstr::testDefaultRequest()){}
  bool Excute(Request* request, Response* response, const In &in);
};

class TestBinaryHandler : public BinaryHandler {
 public:
  TestBinaryHandler():BinaryHandler(cstr::testBinaryRequest()){}
  bool Excute(Request* request, Response* response, const In &in);
};

class TestJsonHandler : public JsonHandler {
 public:
  TestJsonHandler():JsonHandler(cstr::testJsonRequest()){}
  bool Excute(Request* request, Response* response, const In &in);
};

_END_SERVER_NAMESPACE_

#endif  //SERVER_SAMPLE_TESTHANDLERS_H_

