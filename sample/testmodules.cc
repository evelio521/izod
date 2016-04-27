/*******************************************************************************
 *
 * Copyright (c) 2016 evelio, Inc. All Rights Reserved
 *
 *******************************************************************************/



/**
 * @file:   testmodules.cc
 * @author: sunqiang(evelio@126.com)
 * @date:   2016-04-27 15:15:12
 * @brief
 *
 **/

#include "sample/testmodules.h"
#include "src/modules.h"
#include "base/marcos.h"
#include "sample/testhandlers.h"

_START_SERVER_NAMESPACE_

void TestModules::Init() {
  RegisterHttpHandler(new server::TestBinaryHandler());
  RegisterHttpHandler(new server::TestDefaultHandler());
  RegisterHttpHandler(new server::TestJsonHandler());
}

_END_SERVER_NAMESPACE_



























/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */ 

