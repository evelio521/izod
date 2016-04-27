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

#include "sample/testmodules.h"
#include "thirdlibs/gflags/gflags.h"

int main(int argc, char **argv) {
  google::ParseCommandLineFlags(&argc, &argv, false);
  (new server::TestModules())->Server();
  return 0;

}



























/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */ 

