/*******************************************************************************
 *
 * Copyright (c) 2016 evelio, Inc. All Rights Reserved
 *
 *******************************************************************************/



/**
 * @file:   server_main.cpp
 * @author: sunqiang(evelio@126.com)
 * @date:   2016-04-23 16:30:22
 * @brief
 *
 **/

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

//#include "base/callback.h"
#include "base/log.h"
#include "src/request.h"
#include "src/response.h"
#include "src/modules.h"
#include "src/handlers.h"
#include "src/event_base_loop.h"
#include "thirdlibs/event/include/event2/http.h"
#include "thirdlibs/event/include/event2/buffer.h"
#include "thirdlibs/gflags/gflags.h"

int main(int argc, char **argv) {
  google::ParseCommandLineFlags(&argc, &argv, false);
  server::Modules md;

  server::DefaultHandler dh("test");
  std::cout << "++++++" <<md.RegisterHttpHandler("/test",&dh);
  md.Server();
  return 0;

}





























/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */ 

