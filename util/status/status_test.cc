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

TEST(StatusOK, Demo) {
  EXPECT_TRUE(util::error::OK == StatusOk().error_code());
  EXPECT_EQ("", StatusOk().error_message());
}
TEST(StatusAborted, Demo) {
  EXPECT_TRUE(util::error::ABORTED ==
      StatusAborted("StatusAborted").error_code());
  EXPECT_EQ("StatusAborted", StatusAborted("StatusAborted").error_message());
}
TEST(StatusDataLoss, Demo) {
  EXPECT_TRUE(util::error::DATA_LOSS ==
      StatusDataLoss("StatusDataLoss").error_code());
  EXPECT_EQ("StatusDataLoss", StatusDataLoss("StatusDataLoss").error_message());
}
TEST(StatusDeadlineExceeded, Demo) {
  EXPECT_TRUE(util::error::DEADLINE_EXCEEDED ==
      StatusDeadlineExceeded("StatusDeadlineExceeded").error_code());
  EXPECT_EQ("StatusDeadlineExceeded",
      StatusDeadlineExceeded("StatusDeadlineExceeded").error_message());
}
TEST(StatusInternalError, Demo) {
  EXPECT_TRUE(util::error::INTERNAL ==
      StatusInternalError("StatusInternalError").error_code());
  EXPECT_EQ("StatusInternalError",
      StatusInternalError("StatusInternalError").error_message());
}
TEST(StatusInvalidArgument, Demo) {
  EXPECT_TRUE(util::error::INVALID_ARGUMENT ==
      StatusInvalidArgument("StatusInvalidArgument").error_code());
  EXPECT_EQ("StatusInvalidArgument",
      StatusInvalidArgument("StatusInvalidArgument").error_message());
}
TEST(StatusOutOfRange, Demo) {
  EXPECT_TRUE(util::error::OUT_OF_RANGE ==
      StatusOutOfRange("StatusOutOfRange").error_code());
  EXPECT_EQ("StatusOutOfRange",
      StatusOutOfRange("StatusOutOfRange").error_message());
}
TEST(StatusPermissionDenied, Demo) {
  EXPECT_TRUE(util::error::PERMISSION_DENIED ==
     StatusPermissionDenied("StatusPermissionDenied").error_code());
  EXPECT_EQ("StatusPermissionDenied",
     StatusPermissionDenied("StatusPermissionDenied").error_message());
}
TEST(StatusUnimplemented, Demo) {
  EXPECT_TRUE(util::error::UNIMPLEMENTED ==
      StatusUnimplemented("StatusUnimplemented").error_code());
  EXPECT_EQ("StatusUnimplemented",
      StatusUnimplemented("StatusUnimplemented").error_message());
}
TEST(StatusUnknown, Demo) {
  EXPECT_TRUE(util::error::UNKNOWN ==
      StatusUnknown("StatusUnknown").error_code());
  EXPECT_EQ("StatusUnknown",
      StatusUnknown("StatusUnknown").error_message());
}
TEST(StatusResourceExhausted, Demo) {
  EXPECT_TRUE(util::error::RESOURCE_EXHAUSTED ==
      StatusResourceExhausted("StatusResourceExhausted").error_code());
  EXPECT_EQ("StatusResourceExhausted",
      StatusResourceExhausted("StatusResourceExhausted").error_message());
}
TEST(StatusFailedPrecondition, Demo) {
  EXPECT_TRUE(util::error::FAILED_PRECONDITION ==
      StatusFailedPrecondition("StatusFailedPrecondition").error_code());
  EXPECT_EQ("StatusFailedPrecondition",
      StatusFailedPrecondition("StatusFailedPrecondition").error_message());
}
_END_UTIL_NAMESPACE_




























/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */ 

