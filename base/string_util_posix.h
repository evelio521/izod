// Copyright (c) 2006-2008 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_STRING_UTIL_POSIX_H_
#define BASE_STRING_UTIL_POSIX_H_

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>

#include "base/basictypes.h"
#include "base/log.h"
#include "base/marcos.h"

_START_BASE_NAMESPACE_

inline int strncmp16(const char16* s1, const char16* s2, size_t count) {
#if defined(WCHAR_T_IS_UTF16)
  return ::wcsncmp(s1, s2, count);
#elif defined(WCHAR_T_IS_UTF32)
  return c16memcmp(s1, s2, count);
#endif
}

inline int vswprintf(wchar_t* buffer, size_t size,
                     const wchar_t* format, va_list arguments) {
  DCHECK(IsWprintfFormatPortable(format));
  return ::vswprintf(buffer, size, format, arguments);
}

_END_BASE_NAMESPACE_

#endif  // BASE_STRING_UTIL_POSIX_H_
