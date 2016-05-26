/*******************************************************************************
 *
 * Copyright (c) 2016 evelio, Inc. All Rights Reserved
 *
 *******************************************************************************/



/**
 * @file:   status_test.cc
 * @author: sunqiang(evelio@126.com)
 * @date:   2016-05-26 16:39:24
 * @brief
 *
 **/

#include "util/status/status.h"
#include "thirdlibs/gtest/gtest.h"

_START_UTIL_NAMESPACE_


TEST(StatusOK, demo) {
  EXPECT_TRUE(util::error::OK ==  StatusOk().error_code());

//  Status StatusAbortedTest(const string& msg) {
//    return STATUS_FROM_ENUM(ABORTED, msg);
//  }
//
//
//  Status StatusCanceledTest(const string& msg) {
//    return STATUS_FROM_ENUM(CANCELLED, msg);
//  }
//
//  Status StatusDataLossTest(const string& msg) {
//    return STATUS_FROM_ENUM(DATA_LOSS, msg);
//  }
//
//  Status StatusDeadlineExceededTest(const string& msg) {
//    return STATUS_FROM_ENUM(DEADLINE_EXCEEDED, msg);
//  }
//
//  Status StatusInternalErrorTest(const string& msg) {
//    return STATUS_FROM_ENUM(INTERNAL, msg);
//  }
//
//  Status StatusInvalidArgumentTest(const string& msg) {
//    return STATUS_FROM_ENUM(INVALID_ARGUMENT, msg);
//  }
//
//  Status StatusOutOfRangeTest(const string& msg) {
//    return STATUS_FROM_ENUM(OUT_OF_RANGE, msg);
//  }
//
//  Status StatusPermissionDeniedTest(const string& msg) {
//    return STATUS_FROM_ENUM(PERMISSION_DENIED, msg);
//  }
//
//  Status StatusUnimplementedTest(const string& msg) {
//    return STATUS_FROM_ENUM(UNIMPLEMENTED, msg);
//  }
//
//  Status StatusUnknownTest(const string& msg) {
//    return STATUS_FROM_ENUM(UNKNOWN, msg);
//  }
//
//  Status StatusResourceExhaustedTest(const string& msg) {
//    return STATUS_FROM_ENUM(RESOURCE_EXHAUSTED, msg);
//  }
//
//  Status StatusFailedPreconditionTest(const string& msg) {
//    return STATUS_FROM_ENUM(FAILED_PRECONDITION, msg);
//  }
}

_END_UTIL_NAMESPACE_




























/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */ 

