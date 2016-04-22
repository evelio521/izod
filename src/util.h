/***************************************************************************
 * 
 * Copyright (c) 2016 evelio, Inc. All Rights Reserved
 *
 * *************************************************************************/

/**
 * @file:   util.h
 * @author: sunqiang(evelio@126.com)
 * @date:   2016-04-22 11:33:49
 * @brief
 *
 **/
#ifndef SERVER_SRC_UTIL_H_
#define SERVER_SRC_UTIL_H_

#include <string>

#include "base/marcos.h"
#include "base/compat.h"

_START_SERVER_NAMESPACE_

//  Try to guess a good content-type for 'path',
// return "application/misc" as default type.
string GetContentType(const string& path);

// Try to get status infos.
string GetStatusInfo(int code);


_END_SERVER_NAMESPACE_

#endif  // SERVER_INCLUDE_UTIL_H_

