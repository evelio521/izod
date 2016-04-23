// Copyright (c) 2006-2008 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_COMPILER_SPECIFIC_H_
#define BASE_COMPILER_SPECIFIC_H_

#include "base/build_config.h"



#define MSVC_SUPPRESS_WARNING(n)
#define MSVC_PUSH_DISABLE_WARNING(n)
#define MSVC_PUSH_WARNING_LEVEL(n)
#define MSVC_POP_WARNING()
#define MSVC_DISABLE_OPTIMIZE()
#define MSVC_ENABLE_OPTIMIZE()
#define ALLOW_THIS_IN_INITIALIZER_LIST(code) code

#endif  // COMPILER_MSVC


#if defined(COMPILER_GCC)

#define ALLOW_UNUSED __attribute__((unused))
#define WARN_UNUSED_RESULT __attribute__((warn_unused_result))

// Tell the compiler a function is using a printf-style format string.
// |format_param| is the one-based index of the format string parameter;
// |dots_param| is the one-based index of the "..." parameter.
// For v*printf functions (which take a va_list), pass 0 for dots_param.
// (This is undocumented but matches what the system C headers do.)
#define PRINTF_FORMAT(format_param, dots_param) \
    __attribute__((format(printf, format_param, dots_param)))

// WPRINTF_FORMAT is the same, but for wide format strings.
// This doesn't appear to yet be implemented.
// See http://gcc.gnu.org/bugzilla/show_bug.cgi?id=38308 .
#define WPRINTF_FORMAT(format_param, dots_param)
// If available, it would look like:
//   __attribute__((format(wprintf, format_param, dots_param)))

#endif  // BASE_COMPILER_SPECIFIC_H_
