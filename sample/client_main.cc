/*******************************************************************************
 *
 * Copyright (c) 2016 evelio, Inc. All Rights Reserved
 *
 *******************************************************************************/



/**
 * @file:   client_main.cc
 * @author: sunqiang(evelio@126.com)
 * @date:   2016-05-11 15:32:07
 * @brief
 *
 **/

#include "src/client.h"
#include "thirdlibs/gflags/gflags.h"


int main(int argc, char **argv) {
  google::ParseCommandLineFlags(&argc, &argv, false);
  client::Client cli;
  cli.FetchGetUrl("http://127.0.0.1:8888/json?query=999999&qq=3&yy=4");
  cout << "++++++++++++++++++++++++++++++++" << endl;
  cout <<   cli.ResponseBody() <<endl;
  cout << "++++++++++++++++++++++++++++++++" << endl;

  return 0;
}




























/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */ 

