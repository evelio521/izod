/***************************************************************************
 * 
 * Copyright (c) 2016 evelio, Inc. All Rights Reserved
 *
 * *************************************************************************/

/**
 * @file:   testmodules.h
 * @author: sunqiang(evelio@126.com)
 * @date:   2016-04-27 15:14:38
 * @brief
 *
 **/
#ifndef SERVER_SAMPLE_TESTMODULES_H_
#define SERVER_SAMPLE_TESTMODULES_H_

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include "sample/defs.h"
#include "src/modules.h"

_START_SERVER_NAMESPACE_

class TestModules : public Modules {
 public:
  TestModules() {
  }
  void Init();
  static TestModules* Instance();
};
_END_SERVER_NAMESPACE_


































#endif  // SERVER_SAMPLE_TESTMODULES_H_

