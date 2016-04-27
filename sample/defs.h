/* Copyright (c) 2016 evelio, Inc. All Rights Reserved
 *
 * *************************************************************************/

/**
 * @file:   defs.h
 * @author: sunqiang(evelio@126.com)
 * @date:   2016-04-22 11:24:46
 * @brief
 *
 **/
#ifndef SERVER_SRC_DEFS_H_
#define SERVER_SRC_DEFS_H_

#include "base/marcos.h"

_START_SERVER_NAMESPACE_

_START_CSTR_NAMESPACE_

inline const char *testDefaultRequest() {
  return "/default";
}

inline const char *testBinaryRequest() {
  return "/binary";
}
  
inline const char *testJsonRequest() {
  return "/json";
}
_END_CSTR_NAMESPACE_

_END_SERVER_NAMESPACE_

#endif  // SERVER_SRC_DEFS_H_

