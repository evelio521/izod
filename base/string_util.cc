// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/string_util.h"

#include "base/build_config.h"

#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <wchar.h>
#include <wctype.h>

#include <algorithm>
#include <vector>
#include <sstream>

#include "base/basictypes.h"
#include "base/log.h"
#include "base/singleton.h"
#include "base/dmg_fp.h"
#include "base/utf_string_conversion_utils.h"
#include "base/icu_utf.h"

using std::string;
using std::vector;
using std::stringstream;

namespace {

// Force the singleton used by Empty[W]String[16] to be a unique type. This
// prevents other code that might accidentally use Singleton<string> from
// getting our internal one.
struct EmptyStrings {
  EmptyStrings() {}
  const string s;
  const std::wstring ws;
  const string16 s16;
};

// Used by ReplaceStringPlaceholders to track the position in the string of
// replaced parameters.
struct ReplacementOffset {
  ReplacementOffset(uintptr_t parameter, size_t offset)
      : parameter(parameter),
        offset(offset) {}

  // Index of the parameter.
  uintptr_t parameter;

  // Starting position in the string.
  size_t offset;
};

static bool CompareParameter(const ReplacementOffset& elem1,
                             const ReplacementOffset& elem2) {
  return elem1.parameter < elem2.parameter;
}

// Generalized string-to-number conversion.
//
// StringToNumberTraits should provide:
//  - a typedef for string_type, the STL string type used as input.
//  - a typedef for value_type, the target numeric type.
//  - a static function, convert_func, which dispatches to an appropriate
//    strtol-like function and returns type value_type.
//  - a static function, valid_func, which validates |input| and returns a bool
//    indicating whether it is in proper form.  This is used to check for
//    conditions that convert_func tolerates but should result in
//    StringToNumber returning false.  For strtol-like funtions, valid_func
//    should check for leading whitespace.
template<typename StringToNumberTraits>
bool StringToNumber(const typename StringToNumberTraits::string_type& input,
                    typename StringToNumberTraits::value_type* output) {
  typedef StringToNumberTraits traits;

  errno = 0;  // Thread-safe?  It is on at least Mac, Linux, and Windows.
  typename traits::string_type::value_type* endptr = NULL;
  typename traits::value_type value = traits::convert_func(input.c_str(),
                                                           &endptr);
  *output = value;

  // Cases to return false:
  //  - If errno is ERANGE, there was an overflow or underflow.
  //  - If the input string is empty, there was nothing to parse.
  //  - If endptr does not point to the end of the string, there are either
  //    characters remaining in the string after a parsed number, or the string
  //    does not begin with a parseable number.  endptr is compared to the
  //    expected end given the string's stated length to correctly catch cases
  //    where the string contains embedded NUL characters.
  //  - valid_func determines that the input is not in preferred form.
  return errno == 0 &&
         !input.empty() &&
         input.c_str() + input.length() == endptr &&
         traits::valid_func(input);
}

static int strtoi(const char *nptr, char **endptr, int base) {
  long res = strtol(nptr, endptr, base);
#if __LP64__
  // Long is 64-bits, we have to handle under/overflow ourselves.
  if (res > kint32max) {
    res = kint32max;
    errno = ERANGE;
  } else if (res < kint32min) {
    res = kint32min;
    errno = ERANGE;
  }
#endif
  return static_cast<int>(res);
}

static unsigned int strtoui(const char *nptr, char **endptr, int base) {
  unsigned long res = strtoul(nptr, endptr, base);
#if __LP64__
  // Long is 64-bits, we have to handle under/overflow ourselves.  Test to see
  // if the result can fit into 32-bits (as signed or unsigned).
  if (static_cast<int>(static_cast<long>(res)) != static_cast<long>(res) &&
      static_cast<unsigned int>(res) != res) {
    res = kuint32max;
    errno = ERANGE;
  }
#endif
  return static_cast<unsigned int>(res);
}

class StringToIntTraits {
 public:
  typedef string string_type;
  typedef int value_type;
  static const int kBase = 10;
  static inline value_type convert_func(const string_type::value_type* str,
                                        string_type::value_type** endptr) {
    return strtoi(str, endptr, kBase);
  }
  static inline bool valid_func(const string_type& str) {
    return !str.empty() && !isspace(str[0]);
  }
};

class String16ToIntTraits {
 public:
  typedef string16 string_type;
  typedef int value_type;
  static const int kBase = 10;
  static inline value_type convert_func(const string_type::value_type* str,
                                        string_type::value_type** endptr) {
#if defined(WCHAR_T_IS_UTF16)
    return wcstol(str, endptr, kBase);
#elif defined(WCHAR_T_IS_UTF32)
    string ascii_string = UTF16ToASCII(string16(str));
    char* ascii_end = NULL;
    value_type ret = strtoi(ascii_string.c_str(), &ascii_end, kBase);
    if (ascii_string.c_str() + ascii_string.length() == ascii_end) {
      *endptr =
          const_cast<string_type::value_type*>(str) + ascii_string.length();
    }
    return ret;
#endif
  }
  static inline bool valid_func(const string_type& str) {
    return !str.empty() && !iswspace(str[0]);
  }
};

class StringToInt64Traits {
 public:
  typedef string string_type;
  typedef int64 value_type;
  static const int kBase = 10;
  static inline value_type convert_func(const string_type::value_type* str,
                                        string_type::value_type** endptr) {
#ifdef OS_WIN
    return _strtoi64(str, endptr, kBase);
#else  // assume OS_POSIX
    return strtoll(str, endptr, kBase);
#endif
  }
  static inline bool valid_func(const string_type& str) {
    return !str.empty() && !isspace(str[0]);
  }
};

class String16ToInt64Traits {
 public:
  typedef string16 string_type;
  typedef int64 value_type;
  static const int kBase = 10;
  static inline value_type convert_func(const string_type::value_type* str,
                                        string_type::value_type** endptr) {
#ifdef OS_WIN
    return _wcstoi64(str, endptr, kBase);
#else  // assume OS_POSIX
    string ascii_string = UTF16ToASCII(string16(str));
    char* ascii_end = NULL;
    value_type ret = strtoll(ascii_string.c_str(), &ascii_end, kBase);
    if (ascii_string.c_str() + ascii_string.length() == ascii_end) {
      *endptr =
          const_cast<string_type::value_type*>(str) + ascii_string.length();
    }
    return ret;
#endif
  }
  static inline bool valid_func(const string_type& str) {
    return !str.empty() && !iswspace(str[0]);
  }
};

// For the HexString variants, use the unsigned variants like strtoul for
// convert_func so that input like "0x80000000" doesn't result in an overflow.

class HexStringToIntTraits {
 public:
  typedef string string_type;
  typedef int value_type;
  static const int kBase = 16;
  static inline value_type convert_func(const string_type::value_type* str,
                                        string_type::value_type** endptr) {
    return strtoui(str, endptr, kBase);
  }
  static inline bool valid_func(const string_type& str) {
    return !str.empty() && !isspace(str[0]);
  }
};

class HexString16ToIntTraits {
 public:
  typedef string16 string_type;
  typedef int value_type;
  static const int kBase = 16;
  static inline value_type convert_func(const string_type::value_type* str,
                                        string_type::value_type** endptr) {
#if defined(WCHAR_T_IS_UTF16)
    return wcstoul(str, endptr, kBase);
#elif defined(WCHAR_T_IS_UTF32)
    string ascii_string = UTF16ToASCII(string16(str));
    char* ascii_end = NULL;
    value_type ret = strtoui(ascii_string.c_str(), &ascii_end, kBase);
    if (ascii_string.c_str() + ascii_string.length() == ascii_end) {
      *endptr =
          const_cast<string_type::value_type*>(str) + ascii_string.length();
    }
    return ret;
#endif
  }
  static inline bool valid_func(const string_type& str) {
    return !str.empty() && !iswspace(str[0]);
  }
};

class StringToDoubleTraits {
 public:
  typedef string string_type;
  typedef double value_type;
  static inline value_type convert_func(const string_type::value_type* str,
                                        string_type::value_type** endptr) {
    return dmg_fp::strtod(str, endptr);
  }
  static inline bool valid_func(const string_type& str) {
    return !str.empty() && !isspace(str[0]);
  }
};

class String16ToDoubleTraits {
 public:
  typedef string16 string_type;
  typedef double value_type;
  static inline value_type convert_func(const string_type::value_type* str,
                                        string_type::value_type** endptr) {
    // Because dmg_fp::strtod does not like char16, we convert it to ASCII.
    // In theory, this should be safe, but it's possible that 16-bit chars
    // might get ignored by accident causing something to be parsed when it
    // shouldn't.
    string ascii_string = UTF16ToASCII(string16(str));
    char* ascii_end = NULL;
    value_type ret = dmg_fp::strtod(ascii_string.c_str(), &ascii_end);
    if (ascii_string.c_str() + ascii_string.length() == ascii_end) {
      // Put endptr at end of input string, so it's not recognized as an error.
      *endptr =
          const_cast<string_type::value_type*>(str) + ascii_string.length();
    }

    return ret;
  }
  static inline bool valid_func(const string_type& str) {
    return !str.empty() && !iswspace(str[0]);
  }
};

}  // namespace


_START_BASE_NAMESPACE_

bool IsWprintfFormatPortable(const wchar_t* format) {
  for (const wchar_t* position = format; *position != '\0'; ++position) {
    if (*position == '%') {
      bool in_specification = true;
      bool modifier_l = false;
      while (in_specification) {
        // Eat up characters until reaching a known specifier.
        if (*++position == '\0') {
          // The format string ended in the middle of a specification.  Call
          // it portable because no unportable specifications were found.  The
          // string is equally broken on all platforms.
          return true;
        }

        if (*position == 'l') {
          // 'l' is the only thing that can save the 's' and 'c' specifiers.
          modifier_l = true;
        } else if (((*position == 's' || *position == 'c') && !modifier_l) ||
                   *position == 'S' || *position == 'C' || *position == 'F' ||
                   *position == 'D' || *position == 'O' || *position == 'U') {
          // Not portable.
          return false;
        }

        if (wcschr(L"diouxXeEfgGaAcspn%", *position)) {
          // Portable, keep scanning the rest of the format string.
          in_specification = false;
        }
      }
    }
  }

  return true;
}


_END_BASE_NAMESPACE_


const string& EmptyString() {
  return Singleton<EmptyStrings>::get()->s;
}

const std::wstring& EmptyWString() {
  return Singleton<EmptyStrings>::get()->ws;
}

const string16& EmptyString16() {
  return Singleton<EmptyStrings>::get()->s16;
}

#define WHITESPACE_UNICODE \
  0x0009, /* <control-0009> to <control-000D> */ \
  0x000A,                                        \
  0x000B,                                        \
  0x000C,                                        \
  0x000D,                                        \
  0x0020, /* Space */                            \
  0x0085, /* <control-0085> */                   \
  0x00A0, /* No-Break Space */                   \
  0x1680, /* Ogham Space Mark */                 \
  0x180E, /* Mongolian Vowel Separator */        \
  0x2000, /* En Quad to Hair Space */            \
  0x2001,                                        \
  0x2002,                                        \
  0x2003,                                        \
  0x2004,                                        \
  0x2005,                                        \
  0x2006,                                        \
  0x2007,                                        \
  0x2008,                                        \
  0x2009,                                        \
  0x200A,                                        \
  0x200C, /* Zero Width Non-Joiner */            \
  0x2028, /* Line Separator */                   \
  0x2029, /* Paragraph Separator */              \
  0x202F, /* Narrow No-Break Space */            \
  0x205F, /* Medium Mathematical Space */        \
  0x3000, /* Ideographic Space */                \
  0

const wchar_t kWhitespaceWide[] = {
  WHITESPACE_UNICODE
};
const char16 kWhitespaceUTF16[] = {
  WHITESPACE_UNICODE
};
const char kWhitespaceASCII[] = {
  0x09,    // <control-0009> to <control-000D>
  0x0A,
  0x0B,
  0x0C,
  0x0D,
  0x20,    // Space
  0
};

const char kUtf8ByteOrderMark[] = "\xEF\xBB\xBF";

template<typename STR>
bool RemoveCharsT(const STR& input,
                  const typename STR::value_type remove_chars[],
                  STR* output) {
  bool removed = false;
  size_t found;

  *output = input;

  found = output->find_first_of(remove_chars);
  while (found != STR::npos) {
    removed = true;
    output->replace(found, 1, STR());
    found = output->find_first_of(remove_chars, found);
  }

  return removed;
}

bool RemoveChars(const std::wstring& input,
                 const wchar_t remove_chars[],
                 std::wstring* output) {
  return RemoveCharsT(input, remove_chars, output);
}

#if !defined(WCHAR_T_IS_UTF16)
bool RemoveChars(const string16& input,
                 const char16 remove_chars[],
                 string16* output) {
  return RemoveCharsT(input, remove_chars, output);
}
#endif

bool RemoveChars(const string& input,
                 const char remove_chars[],
                 string* output) {
  return RemoveCharsT(input, remove_chars, output);
}

template<typename STR>
TrimPositions TrimStringT(const STR& input,
                          const typename STR::value_type trim_chars[],
                          TrimPositions positions,
                          STR* output) {
  // Find the edges of leading/trailing whitespace as desired.
  const typename STR::size_type last_char = input.length() - 1;
  const typename STR::size_type first_good_char = (positions & TRIM_LEADING) ?
      input.find_first_not_of(trim_chars) : 0;
  const typename STR::size_type last_good_char = (positions & TRIM_TRAILING) ?
      input.find_last_not_of(trim_chars) : last_char;

  // When the string was all whitespace, report that we stripped off whitespace
  // from whichever position the caller was interested in.  For empty input, we
  // stripped no whitespace, but we still need to clear |output|.
  if (input.empty() ||
      (first_good_char == STR::npos) || (last_good_char == STR::npos)) {
    bool input_was_empty = input.empty();  // in case output == &input
    output->clear();
    return input_was_empty ? TRIM_NONE : positions;
  }

  // Trim the whitespace.
  *output =
      input.substr(first_good_char, last_good_char - first_good_char + 1);

  // Return where we trimmed from.
  return static_cast<TrimPositions>(
      ((first_good_char == 0) ? TRIM_NONE : TRIM_LEADING) |
      ((last_good_char == last_char) ? TRIM_NONE : TRIM_TRAILING));
}

bool TrimString(const std::wstring& input,
                const wchar_t trim_chars[],
                std::wstring* output) {
  return TrimStringT(input, trim_chars, TRIM_ALL, output) != TRIM_NONE;
}

#if !defined(WCHAR_T_IS_UTF16)
bool TrimString(const string16& input,
                const char16 trim_chars[],
                string16* output) {
  return TrimStringT(input, trim_chars, TRIM_ALL, output) != TRIM_NONE;
}
#endif

bool TrimString(const string& input,
                const char trim_chars[],
                string* output) {
  return TrimStringT(input, trim_chars, TRIM_ALL, output) != TRIM_NONE;
}

void TruncateUTF8ToByteSize(const string& input,
                            const size_t byte_size,
                            string* output) {
  DCHECK(output);
  if (byte_size > input.length()) {
    *output = input;
    return;
  }
  DCHECK_LE(byte_size, static_cast<uint32>(kint32max));
  // Note: This cast is necessary because CBU8_NEXT uses int32s.
  int32 truncation_length = static_cast<int32>(byte_size);
  int32 char_index = truncation_length - 1;
  const char* data = input.data();

  // Using CBU8, we will move backwards from the truncation point
  // to the beginning of the string looking for a valid UTF8
  // character.  Once a full UTF8 character is found, we will
  // truncate the string to the end of that character.
  while (char_index >= 0) {
    int32 prev = char_index;
    uint32 code_point = 0;
    CBU8_NEXT(data, char_index, truncation_length, code_point);
    if (!base::IsValidCharacter(code_point) ||
        !base::IsValidCodepoint(code_point)) {
      char_index = prev - 1;
    } else {
      break;
    }
  }

  if (char_index >= 0 )
    *output = input.substr(0, char_index);
  else
    output->clear();
}

TrimPositions TrimWhitespace(const std::wstring& input,
                             TrimPositions positions,
                             std::wstring* output) {
  return TrimStringT(input, kWhitespaceWide, positions, output);
}

#if !defined(WCHAR_T_IS_UTF16)
TrimPositions TrimWhitespace(const string16& input,
                             TrimPositions positions,
                             string16* output) {
  return TrimStringT(input, kWhitespaceUTF16, positions, output);
}
#endif

TrimPositions TrimWhitespaceASCII(const string& input,
                                  TrimPositions positions,
                                  string* output) {
  return TrimStringT(input, kWhitespaceASCII, positions, output);
}

// This function is only for backward-compatibility.
// To be removed when all callers are updated.
TrimPositions TrimWhitespace(const string& input,
                             TrimPositions positions,
                             string* output) {
  return TrimWhitespaceASCII(input, positions, output);
}

template<typename STR>
STR CollapseWhitespaceT(const STR& text,
                        bool trim_sequences_with_line_breaks) {
  STR result;
  result.resize(text.size());

  // Set flags to pretend we're already in a trimmed whitespace sequence, so we
  // will trim any leading whitespace.
  bool in_whitespace = true;
  bool already_trimmed = true;

  int chars_written = 0;
  for (typename STR::const_iterator i(text.begin()); i != text.end(); ++i) {
    if (IsWhitespace(*i)) {
      if (!in_whitespace) {
        // Reduce all whitespace sequences to a single space.
        in_whitespace = true;
        result[chars_written++] = L' ';
      }
      if (trim_sequences_with_line_breaks && !already_trimmed &&
          ((*i == '\n') || (*i == '\r'))) {
        // Whitespace sequences containing CR or LF are eliminated entirely.
        already_trimmed = true;
        --chars_written;
      }
    } else {
      // Non-whitespace chracters are copied straight across.
      in_whitespace = false;
      already_trimmed = false;
      result[chars_written++] = *i;
    }
  }

  if (in_whitespace && !already_trimmed) {
    // Any trailing whitespace is eliminated.
    --chars_written;
  }

  result.resize(chars_written);
  return result;
}

std::wstring CollapseWhitespace(const std::wstring& text,
                                bool trim_sequences_with_line_breaks) {
  return CollapseWhitespaceT(text, trim_sequences_with_line_breaks);
}

#if !defined(WCHAR_T_IS_UTF16)
string16 CollapseWhitespace(const string16& text,
                            bool trim_sequences_with_line_breaks) {
  return CollapseWhitespaceT(text, trim_sequences_with_line_breaks);
}
#endif

string CollapseWhitespaceASCII(const string& text,
                                    bool trim_sequences_with_line_breaks) {
  return CollapseWhitespaceT(text, trim_sequences_with_line_breaks);
}

bool ContainsOnlyWhitespaceASCII(const string& str) {
  for (string::const_iterator i(str.begin()); i != str.end(); ++i) {
    if (!IsAsciiWhitespace(*i))
      return false;
  }
  return true;
}

bool ContainsOnlyWhitespace(const string16& str) {
  for (string16::const_iterator i(str.begin()); i != str.end(); ++i) {
    if (!IsWhitespace(*i))
      return false;
  }
  return true;
}

template<typename STR>
static bool ContainsOnlyCharsT(const STR& input, const STR& characters) {
  for (typename STR::const_iterator iter = input.begin();
       iter != input.end(); ++iter) {
    if (characters.find(*iter) == STR::npos)
      return false;
  }
  return true;
}

bool ContainsOnlyChars(const std::wstring& input,
                       const std::wstring& characters) {
  return ContainsOnlyCharsT(input, characters);
}

#if !defined(WCHAR_T_IS_UTF16)
bool ContainsOnlyChars(const string16& input, const string16& characters) {
  return ContainsOnlyCharsT(input, characters);
}
#endif

bool ContainsOnlyChars(const string& input,
                       const string& characters) {
  return ContainsOnlyCharsT(input, characters);
}

string WideToASCII(const std::wstring& wide) {
  //DCHECK(IsStringASCII(wide)) << wide;
  return string(wide.begin(), wide.end());
}

std::wstring ASCIIToWide(const base::StringPiece& ascii) {
  DCHECK(IsStringASCII(ascii)) << ascii;
  return std::wstring(ascii.begin(), ascii.end());
}

string UTF16ToASCII(const string16& utf16) {
  DCHECK(IsStringASCII(utf16)) << utf16;
  return string(utf16.begin(), utf16.end());
}

string16 ASCIIToUTF16(const base::StringPiece& ascii) {
  DCHECK(IsStringASCII(ascii)) << ascii;
  return string16(ascii.begin(), ascii.end());
}

// Latin1 is just the low range of Unicode, so we can copy directly to convert.
bool WideToLatin1(const std::wstring& wide, string* latin1) {
  string output;
  output.resize(wide.size());
  latin1->clear();
  for (size_t i = 0; i < wide.size(); i++) {
    if (wide[i] > 255)
      return false;
    output[i] = static_cast<char>(wide[i]);
  }
  latin1->swap(output);
  return true;
}

bool IsString8Bit(const std::wstring& str) {
  for (size_t i = 0; i < str.length(); i++) {
    if (str[i] > 255)
      return false;
  }
  return true;
}

template<class STR>
static bool DoIsStringASCII(const STR& str) {
  for (size_t i = 0; i < str.length(); i++) {
    typename ToUnsigned<typename STR::value_type>::Unsigned c = str[i];
    if (c > 0x7F)
      return false;
  }
  return true;
}

bool IsStringASCII(const std::wstring& str) {
  return DoIsStringASCII(str);
}

#if !defined(WCHAR_T_IS_UTF16)
bool IsStringASCII(const string16& str) {
  return DoIsStringASCII(str);
}
#endif

bool IsStringASCII(const base::StringPiece& str) {
  return DoIsStringASCII(str);
}

bool IsStringUTF8(const string& str) {
  const char *src = str.data();
  int32 src_len = static_cast<int32>(str.length());
  int32 char_index = 0;

  while (char_index < src_len) {
    int32 code_point;
    CBU8_NEXT(src, char_index, src_len, code_point);
    if (!base::IsValidCharacter(code_point))
       return false;
  }
  return true;
}

template<typename Iter>
static inline bool DoLowerCaseEqualsASCII(Iter a_begin,
                                          Iter a_end,
                                          const char* b) {
  for (Iter it = a_begin; it != a_end; ++it, ++b) {
    if (!*b || ToLowerASCII(*it) != *b)
      return false;
  }
  return *b == 0;
}

// Front-ends for LowerCaseEqualsASCII.
bool LowerCaseEqualsASCII(const string& a, const char* b) {
  return DoLowerCaseEqualsASCII(a.begin(), a.end(), b);
}

bool LowerCaseEqualsASCII(const std::wstring& a, const char* b) {
  return DoLowerCaseEqualsASCII(a.begin(), a.end(), b);
}

#if !defined(WCHAR_T_IS_UTF16)
bool LowerCaseEqualsASCII(const string16& a, const char* b) {
  return DoLowerCaseEqualsASCII(a.begin(), a.end(), b);
}
#endif

bool LowerCaseEqualsASCII(string::const_iterator a_begin,
                          string::const_iterator a_end,
                          const char* b) {
  return DoLowerCaseEqualsASCII(a_begin, a_end, b);
}

bool LowerCaseEqualsASCII(std::wstring::const_iterator a_begin,
                          std::wstring::const_iterator a_end,
                          const char* b) {
  return DoLowerCaseEqualsASCII(a_begin, a_end, b);
}

#if !defined(WCHAR_T_IS_UTF16)
bool LowerCaseEqualsASCII(string16::const_iterator a_begin,
                          string16::const_iterator a_end,
                          const char* b) {
  return DoLowerCaseEqualsASCII(a_begin, a_end, b);
}
#endif

bool LowerCaseEqualsASCII(const char* a_begin,
                          const char* a_end,
                          const char* b) {
  return DoLowerCaseEqualsASCII(a_begin, a_end, b);
}

bool LowerCaseEqualsASCII(const wchar_t* a_begin,
                          const wchar_t* a_end,
                          const char* b) {
  return DoLowerCaseEqualsASCII(a_begin, a_end, b);
}

#if !defined(WCHAR_T_IS_UTF16)
bool LowerCaseEqualsASCII(const char16* a_begin,
                          const char16* a_end,
                          const char* b) {
  return DoLowerCaseEqualsASCII(a_begin, a_end, b);
}
#endif

bool EqualsASCII(const string16& a, const base::StringPiece& b) {
  if (a.length() != b.length())
    return false;
  return std::equal(b.begin(), b.end(), a.begin());
}

bool StartsWithASCII(const string& str,
                     const string& search,
                     bool case_sensitive) {
  if (case_sensitive)
    return str.compare(0, search.length(), search) == 0;
  else
    return ::strncasecmp(str.c_str(), search.c_str(), search.length()) == 0;
}

template <typename STR>
bool StartsWithT(const STR& str, const STR& search, bool case_sensitive) {
  if (case_sensitive) {
    return str.compare(0, search.length(), search) == 0;
  } else {
    if (search.size() > str.size())
      return false;
    return std::equal(search.begin(), search.end(), str.begin(),
                      CaseInsensitiveCompare<typename STR::value_type>());
  }
}

bool StartsWith(const std::wstring& str, const std::wstring& search,
                bool case_sensitive) {
  return StartsWithT(str, search, case_sensitive);
}

#if !defined(WCHAR_T_IS_UTF16)
bool StartsWith(const string16& str, const string16& search,
                bool case_sensitive) {
  return StartsWithT(str, search, case_sensitive);
}
#endif

template <typename STR>
bool EndsWithT(const STR& str, const STR& search, bool case_sensitive) {
  typename STR::size_type str_length = str.length();
  typename STR::size_type search_length = search.length();
  if (search_length > str_length)
    return false;
  if (case_sensitive) {
    return str.compare(str_length - search_length, search_length, search) == 0;
  } else {
    return std::equal(search.begin(), search.end(),
                      str.begin() + (str_length - search_length),
                      CaseInsensitiveCompare<typename STR::value_type>());
  }
}

bool EndsWith(const string& str, const string& search,
              bool case_sensitive) {
  return EndsWithT(str, search, case_sensitive);
}

bool EndsWith(const std::wstring& str, const std::wstring& search,
              bool case_sensitive) {
  return EndsWithT(str, search, case_sensitive);
}

#if !defined(WCHAR_T_IS_UTF16)
bool EndsWith(const string16& str, const string16& search,
              bool case_sensitive) {
  return EndsWithT(str, search, case_sensitive);
}
#endif

DataUnits GetByteDisplayUnits(int64 bytes) {
  // The byte thresholds at which we display amounts.  A byte count is displayed
  // in unit U when kUnitThresholds[U] <= bytes < kUnitThresholds[U+1].
  // This must match the DataUnits enum.
  static const int64 kUnitThresholds[] = {
    0,              // DATA_UNITS_BYTE,
    3*1024,         // DATA_UNITS_KIBIBYTE,
    2*1024*1024,    // DATA_UNITS_MEBIBYTE,
    1024*1024*1024  // DATA_UNITS_GIBIBYTE,
  };

  if (bytes < 0) {
    LOG(FATAL) << "Negative bytes value";
    return DATA_UNITS_BYTE;
  }

  int unit_index = arraysize(kUnitThresholds);
  while (--unit_index > 0) {
    if (bytes >= kUnitThresholds[unit_index])
      break;
  }

  DCHECK(unit_index >= DATA_UNITS_BYTE && unit_index <= DATA_UNITS_GIBIBYTE);
  return DataUnits(unit_index);
}

// TODO(mpcomplete): deal with locale
// Byte suffixes.  This must match the DataUnits enum.
static const wchar_t* const kByteStrings[] = {
  L"B",
  L"kB",
  L"MB",
  L"GB"
};

static const wchar_t* const kSpeedStrings[] = {
  L"B/s",
  L"kB/s",
  L"MB/s",
  L"GB/s"
};

std::wstring FormatBytesInternal(int64 bytes,
                                 DataUnits units,
                                 bool show_units,
                                 const wchar_t* const* suffix) {
  if (bytes < 0) {
    LOG(FATAL) << "Negative bytes value";
    return std::wstring();
  }

  DCHECK(units >= DATA_UNITS_BYTE && units <= DATA_UNITS_GIBIBYTE);

  // Put the quantity in the right units.
  double unit_amount = static_cast<double>(bytes);
  for (int i = 0; i < units; ++i)
    unit_amount /= 1024.0;

  wchar_t buf[64];
  if (bytes != 0 && units != DATA_UNITS_BYTE && unit_amount < 100)
    swprintf(buf, arraysize(buf), L"%.1lf", unit_amount);
  else
    swprintf(buf, arraysize(buf), L"%.0lf", unit_amount);

  std::wstring ret(buf);
  if (show_units) {
    ret += L" ";
    ret += suffix[units];
  }

  return ret;
}

std::wstring FormatBytes(int64 bytes, DataUnits units, bool show_units) {
  return FormatBytesInternal(bytes, units, show_units, kByteStrings);
}

std::wstring FormatSpeed(int64 bytes, DataUnits units, bool show_units) {
  return FormatBytesInternal(bytes, units, show_units, kSpeedStrings);
}

template<class StringType>
void DoReplaceSubstringsAfterOffset(StringType* str,
                                    typename StringType::size_type start_offset,
                                    const StringType& find_this,
                                    const StringType& replace_with,
                                    bool replace_all) {
  if ((start_offset == StringType::npos) || (start_offset >= str->length()))
    return;

  DCHECK(!find_this.empty());
  for (typename StringType::size_type offs(str->find(find_this, start_offset));
      offs != StringType::npos; offs = str->find(find_this, offs)) {
    str->replace(offs, find_this.length(), replace_with);
    offs += replace_with.length();

    if (!replace_all)
      break;
  }
}

void ReplaceFirstSubstringAfterOffset(string16* str,
                                      string16::size_type start_offset,
                                      const string16& find_this,
                                      const string16& replace_with) {
  DoReplaceSubstringsAfterOffset(str, start_offset, find_this, replace_with,
                                 false);  // replace first instance
}

void ReplaceFirstSubstringAfterOffset(string* str,
                                      string::size_type start_offset,
                                      const string& find_this,
                                      const string& replace_with) {
  DoReplaceSubstringsAfterOffset(str, start_offset, find_this, replace_with,
                                 false);  // replace first instance
}

void ReplaceSubstringsAfterOffset(string16* str,
                                  string16::size_type start_offset,
                                  const string16& find_this,
                                  const string16& replace_with) {
  DoReplaceSubstringsAfterOffset(str, start_offset, find_this, replace_with,
                                 true);  // replace all instances
}

void ReplaceSubstringsAfterOffset(string* str,
                                  string::size_type start_offset,
                                  const string& find_this,
                                  const string& replace_with) {
  DoReplaceSubstringsAfterOffset(str, start_offset, find_this, replace_with,
                                 true);  // replace all instances
}

// Overloaded wrappers around vsnprintf and vswprintf. The buf_size parameter
// is the size of the buffer. These return the number of characters in the
// formatted string excluding the NUL terminator. If the buffer is not
// large enough to accommodate the formatted string without truncation, they
// return the number of characters that would be in the fully-formatted string
// (vsnprintf, and vswprintf on Windows), or -1 (vswprintf on POSIX platforms).
inline int vsnprintfT(char* buffer,
                      size_t buf_size,
                      const char* format,
                      va_list argptr) {
  return vsnprintf(buffer, buf_size, format, argptr);
}

inline int vsnprintfT(wchar_t* buffer,
                      size_t buf_size,
                      const wchar_t* format,
                      va_list argptr) {
  return vswprintf(buffer, buf_size, format, argptr);
}

// Templatized backend for StringPrintF/StringAppendF. This does not finalize
// the va_list, the caller is expected to do that.
template <class StringType>
static void StringAppendVT(StringType* dst,
                           const typename StringType::value_type* format,
                           va_list ap) {
  // First try with a small fixed size buffer.
  // This buffer size should be kept in sync with StringUtilTest.GrowBoundary
  // and StringUtilTest.StringPrintfBounds.
  typename StringType::value_type stack_buf[1024];

  va_list ap_copy;
  GG_VA_COPY(ap_copy, ap);

#if !defined(OS_WIN)
  errno = 0;
#endif
  int result = vsnprintfT(stack_buf, arraysize(stack_buf), format, ap_copy);
  va_end(ap_copy);

  if (result >= 0 && result < static_cast<int>(arraysize(stack_buf))) {
    // It fit.
    dst->append(stack_buf, result);
    return;
  }

  // Repeatedly increase buffer size until it fits.
  int mem_length = arraysize(stack_buf);
  while (true) {
    if (result < 0) {
#if !defined(OS_WIN)
      // On Windows, vsnprintfT always returns the number of characters in a
      // fully-formatted string, so if we reach this point, something else is
      // wrong and no amount of buffer-doubling is going to fix it.
      if (errno != 0 && errno != EOVERFLOW)
#endif
      {
        // If an error other than overflow occurred, it's never going to work.
        DLOG(WARNING) << "Unable to printf the requested string due to error.";
        return;
      }
      // Try doubling the buffer size.
      mem_length *= 2;
    } else {
      // We need exactly "result + 1" characters.
      mem_length = result + 1;
    }

    if (mem_length > 32 * 1024 * 1024) {
      // That should be plenty, don't try anything larger.  This protects
      // against huge allocations when using vsnprintfT implementations that
      // return -1 for reasons other than overflow without setting errno.
      DLOG(WARNING) << "Unable to printf the requested string due to size.";
      return;
    }

    std::vector<typename StringType::value_type> mem_buf(mem_length);

    // NOTE: You can only use a va_list once.  Since we're in a while loop, we
    // need to make a new copy each time so we don't use up the original.
    GG_VA_COPY(ap_copy, ap);
    result = vsnprintfT(&mem_buf[0], mem_length, format, ap_copy);
    va_end(ap_copy);

    if ((result >= 0) && (result < mem_length)) {
      // It fit.
      dst->append(&mem_buf[0], result);
      return;
    }
  }
}

namespace {

template <typename STR, typename INT, typename UINT, bool NEG>
struct IntToStringT {
  // This is to avoid a compiler warning about unary minus on unsigned type.
  // For example, say you had the following code:
  //   template <typename INT>
  //   INT abs(INT value) { return value < 0 ? -value : value; }
  // Even though if INT is unsigned, it's impossible for value < 0, so the
  // unary minus will never be taken, the compiler will still generate a
  // warning.  We do a little specialization dance...
  template <typename INT2, typename UINT2, bool NEG2>
  struct ToUnsignedT { };

  template <typename INT2, typename UINT2>
  struct ToUnsignedT<INT2, UINT2, false> {
    static UINT2 ToUnsigned(INT2 value) {
      return static_cast<UINT2>(value);
    }
  };

  template <typename INT2, typename UINT2>
  struct ToUnsignedT<INT2, UINT2, true> {
    static UINT2 ToUnsigned(INT2 value) {
      return static_cast<UINT2>(value < 0 ? -value : value);
    }
  };

  // This set of templates is very similar to the above templates, but
  // for testing whether an integer is negative.
  template <typename INT2, bool NEG2>
  struct TestNegT { };
  template <typename INT2>
  struct TestNegT<INT2, false> {
    static bool TestNeg(INT2 value) {
      // value is unsigned, and can never be negative.
      return false;
    }
  };
  template <typename INT2>
  struct TestNegT<INT2, true> {
    static bool TestNeg(INT2 value) {
      return value < 0;
    }
  };

  static STR IntToString(INT value) {
    // log10(2) ~= 0.3 bytes needed per bit or per byte log10(2**8) ~= 2.4.
    // So round up to allocate 3 output characters per byte, plus 1 for '-'.
    const int kOutputBufSize = 3 * sizeof(INT) + 1;

    // Allocate the whole string right away, we will right back to front, and
    // then return the substr of what we ended up using.
    STR outbuf(kOutputBufSize, 0);

    bool is_neg = TestNegT<INT, NEG>::TestNeg(value);
    // Even though is_neg will never be true when INT is parameterized as
    // unsigned, even the presence of the unary operation causes a warning.
    UINT res = ToUnsignedT<INT, UINT, NEG>::ToUnsigned(value);

    for (typename STR::iterator it = outbuf.end();;) {
      --it;
      DCHECK(it != outbuf.begin());
      *it = static_cast<typename STR::value_type>((res % 10) + '0');
      res /= 10;

      // We're done..
      if (res == 0) {
        if (is_neg) {
          --it;
          DCHECK(it != outbuf.begin());
          *it = static_cast<typename STR::value_type>('-');
        }
        return STR(it, outbuf.end());
      }
    }
    LOG(FATAL);
    return STR();
  }
};

}

string IntToString(int value) {
  return IntToStringT<string, int, unsigned int, true>::
      IntToString(value);
}
std::wstring IntToWString(int value) {
  return IntToStringT<std::wstring, int, unsigned int, true>::
      IntToString(value);
}
string16 IntToString16(int value) {
  return IntToStringT<string16, int, unsigned int, true>::
      IntToString(value);
}
string UintToString(unsigned int value) {
  return IntToStringT<string, unsigned int, unsigned int, false>::
      IntToString(value);
}
std::wstring UintToWString(unsigned int value) {
  return IntToStringT<std::wstring, unsigned int, unsigned int, false>::
      IntToString(value);
}
string16 UintToString16(unsigned int value) {
  return IntToStringT<string16, unsigned int, unsigned int, false>::
      IntToString(value);
}
string Int64ToString(int64 value) {
  return IntToStringT<string, int64, uint64, true>::
      IntToString(value);
}
std::wstring Int64ToWString(int64 value) {
  return IntToStringT<std::wstring, int64, uint64, true>::
      IntToString(value);
}
string Uint64ToString(uint64 value) {
  return IntToStringT<string, uint64, uint64, false>::
      IntToString(value);
}
std::wstring Uint64ToWString(uint64 value) {
  return IntToStringT<std::wstring, uint64, uint64, false>::
      IntToString(value);
}

string DoubleToString(double value) {
  // According to g_fmt.cc, it is sufficient to declare a buffer of size 32.
  char buffer[32];
  dmg_fp::g_fmt(buffer, value);
  return string(buffer);
}

std::wstring DoubleToWString(double value) {
  return ASCIIToWide(DoubleToString(value));
}

void StringAppendV(string* dst, const char* format, va_list ap) {
  StringAppendVT(dst, format, ap);
}

void StringAppendV(std::wstring* dst, const wchar_t* format, va_list ap) {
  StringAppendVT(dst, format, ap);
}

string StringPrintf(const char* format, ...) {
  va_list ap;
  va_start(ap, format);
  string result;
  StringAppendV(&result, format, ap);
  va_end(ap);
  return result;
}

std::wstring StringPrintf(const wchar_t* format, ...) {
  va_list ap;
  va_start(ap, format);
  std::wstring result;
  StringAppendV(&result, format, ap);
  va_end(ap);
  return result;
}

string StringPrintV(const char* format, va_list ap) {
  string result;
  StringAppendV(&result, format, ap);
  return result;
}

const string& SStringPrintf(string* dst, const char* format, ...) {
  va_list ap;
  va_start(ap, format);
  dst->clear();
  StringAppendV(dst, format, ap);
  va_end(ap);
  return *dst;
}

const std::wstring& SStringPrintf(std::wstring* dst,
                                  const wchar_t* format, ...) {
  va_list ap;
  va_start(ap, format);
  dst->clear();
  StringAppendV(dst, format, ap);
  va_end(ap);
  return *dst;
}

void StringAppendF(string* dst, const char* format, ...) {
  va_list ap;
  va_start(ap, format);
  StringAppendV(dst, format, ap);
  va_end(ap);
}

void StringAppendF(std::wstring* dst, const wchar_t* format, ...) {
  va_list ap;
  va_start(ap, format);
  StringAppendV(dst, format, ap);
  va_end(ap);
}

template<typename STR>
static void SplitStringT(const STR& str,
                         const typename STR::value_type s,
                         bool trim_whitespace,
                         std::vector<STR>* r) {
  size_t last = 0;
  size_t i;
  size_t c = str.size();
  for (i = 0; i <= c; ++i) {
    if (i == c || str[i] == s) {
      size_t len = i - last;
      STR tmp = str.substr(last, len);
      if (trim_whitespace) {
        STR t_tmp;
        TrimWhitespace(tmp, TRIM_ALL, &t_tmp);
        r->push_back(t_tmp);
      } else {
        r->push_back(tmp);
      }
      last = i + 1;
    }
  }
}

void SplitString(const std::wstring& str,
                 wchar_t s,
                 std::vector<std::wstring>* r) {
  SplitStringT(str, s, true, r);
}

#if !defined(WCHAR_T_IS_UTF16)
void SplitString(const string16& str,
                 char16 s,
                 std::vector<string16>* r) {
  SplitStringT(str, s, true, r);
}
#endif

void SplitString(const string& str,
                 char s,
                 std::vector<string>* r) {
  SplitStringT(str, s, true, r);
}

void SplitStringDontTrim(const std::wstring& str,
                         wchar_t s,
                         std::vector<std::wstring>* r) {
  SplitStringT(str, s, false, r);
}

#if !defined(WCHAR_T_IS_UTF16)
void SplitStringDontTrim(const string16& str,
                         char16 s,
                         std::vector<string16>* r) {
  SplitStringT(str, s, false, r);
}
#endif

void SplitStringDontTrim(const string& str,
                         char s,
                         std::vector<string>* r) {
  SplitStringT(str, s, false, r);
}

template <typename STR>
static void SplitStringUsingSubstrT(const STR& str,
                                    const STR& s,
                                    std::vector<STR>* r) {
  typename STR::size_type begin_index = 0;
  while (true) {
    const typename STR::size_type end_index = str.find(s, begin_index);
    if (end_index == STR::npos) {
      const STR term = str.substr(begin_index);
      STR tmp;
      TrimWhitespace(term, TRIM_ALL, &tmp);
      r->push_back(tmp);
      return;
    }
    const STR term = str.substr(begin_index, end_index - begin_index);
    STR tmp;
    TrimWhitespace(term, TRIM_ALL, &tmp);
    r->push_back(tmp);
    begin_index = end_index + s.size();
  }
}

void SplitStringUsingSubstr(const string16& str,
                            const string16& s,
                            std::vector<string16>* r) {
  SplitStringUsingSubstrT(str, s, r);
}

void SplitStringUsingSubstr(const string& str,
                            const string& s,
                            std::vector<string>* r) {
  SplitStringUsingSubstrT(str, s, r);
}

template<typename STR>
static size_t TokenizeT(const STR& str,
                        const STR& delimiters,
                        std::vector<STR>* tokens) {
  tokens->clear();

  typename STR::size_type start = str.find_first_not_of(delimiters);
  while (start != STR::npos) {
    typename STR::size_type end = str.find_first_of(delimiters, start + 1);
    if (end == STR::npos) {
      tokens->push_back(str.substr(start));
      break;
    } else {
      tokens->push_back(str.substr(start, end - start));
      start = str.find_first_not_of(delimiters, end + 1);
    }
  }

  return tokens->size();
}

size_t Tokenize(const std::wstring& str,
                const std::wstring& delimiters,
                std::vector<std::wstring>* tokens) {
  return TokenizeT(str, delimiters, tokens);
}

#if !defined(WCHAR_T_IS_UTF16)
size_t Tokenize(const string16& str,
                const string16& delimiters,
                std::vector<string16>* tokens) {
  return TokenizeT(str, delimiters, tokens);
}
#endif

size_t Tokenize(const string& str,
                const string& delimiters,
                std::vector<string>* tokens) {
  return TokenizeT(str, delimiters, tokens);
}

size_t Tokenize(const base::StringPiece& str,
                const base::StringPiece& delimiters,
                std::vector<base::StringPiece>* tokens) {
  return TokenizeT(str, delimiters, tokens);
}

template<typename STR>
static STR JoinStringT(const std::vector<STR>& parts,
                       typename STR::value_type sep) {
  if (parts.size() == 0) return STR();

  STR result(parts[0]);
  typename std::vector<STR>::const_iterator iter = parts.begin();
  ++iter;

  for (; iter != parts.end(); ++iter) {
    result += sep;
    result += *iter;
  }

  return result;
}

template<typename STR>
static STR JoinStringT(typename std::vector<STR>::const_iterator begin,
                       typename std::vector<STR>::const_iterator end,
                       typename STR::value_type sep) {
  if (begin >= end) return STR();

  STR result(*begin);
  typename std::vector<STR>::const_iterator iter = begin;
  ++iter;

  for (; iter != end; ++iter) {
    result += sep;
    result += *iter;
  }

  return result;
}

string JoinString(const std::vector<string>& parts, char sep) {
  return JoinStringT(parts, sep);
}

#if !defined(WCHAR_T_IS_UTF16)
string16 JoinString(const std::vector<string16>& parts, char16 sep) {
  return JoinStringT(parts, sep);
}
#endif

std::wstring JoinString(const std::vector<std::wstring>& parts, wchar_t sep) {
  return JoinStringT(parts, sep);
}

string JoinString(std::vector<string>::const_iterator begin,
                  std::vector<string>::const_iterator end,
                  char sep) {
  return JoinStringT<string>(begin, end, sep);
}

#if !defined(WCHAR_T_IS_UTF16)
string16 JoinString(std::vector<string16>::const_iterator begin,
                    std::vector<string16>::const_iterator end,
                    char16 sep) {
  return JoinStringT<string16>(begin, end, sep);
}
#endif

std::wstring JoinString(std::vector<std::wstring>::const_iterator begin,
                        std::vector<std::wstring>::const_iterator end,
                        wchar_t sep) {
  return JoinStringT<std::wstring>(begin, end, sep);
}

template<typename STR>
void SplitStringAlongWhitespaceT(const STR& str, std::vector<STR>* result) {
  const size_t length = str.length();
  if (!length)
    return;

  bool last_was_ws = false;
  size_t last_non_ws_start = 0;
  for (size_t i = 0; i < length; ++i) {
    switch (str[i]) {
      // HTML 5 defines whitespace as: space, tab, LF, line tab, FF, or CR.
      case L' ':
      case L'\t':
      case L'\xA':
      case L'\xB':
      case L'\xC':
      case L'\xD':
        if (!last_was_ws) {
          if (i > 0) {
            result->push_back(
                str.substr(last_non_ws_start, i - last_non_ws_start));
          }
          last_was_ws = true;
        }
        break;

      default:  // Not a space character.
        if (last_was_ws) {
          last_was_ws = false;
          last_non_ws_start = i;
        }
        break;
    }
  }
  if (!last_was_ws) {
    result->push_back(
        str.substr(last_non_ws_start, length - last_non_ws_start));
  }
}

void SplitStringAlongWhitespace(const std::wstring& str,
                                std::vector<std::wstring>* result) {
  SplitStringAlongWhitespaceT(str, result);
}

#if !defined(WCHAR_T_IS_UTF16)
void SplitStringAlongWhitespace(const string16& str,
                                std::vector<string16>* result) {
  SplitStringAlongWhitespaceT(str, result);
}
#endif

void SplitStringAlongWhitespace(const string& str,
                                std::vector<string>* result) {
  SplitStringAlongWhitespaceT(str, result);
}

template<class FormatStringType, class OutStringType>
OutStringType DoReplaceStringPlaceholders(const FormatStringType& format_string,
    const std::vector<OutStringType>& subst, std::vector<size_t>* offsets) {
  size_t substitutions = subst.size();
  DCHECK(substitutions < 10);

  size_t sub_length = 0;
  for (typename std::vector<OutStringType>::const_iterator iter = subst.begin();
       iter != subst.end(); ++iter) {
    sub_length += (*iter).length();
  }

  OutStringType formatted;
  formatted.reserve(format_string.length() + sub_length);

  std::vector<ReplacementOffset> r_offsets;
  for (typename FormatStringType::const_iterator i = format_string.begin();
       i != format_string.end(); ++i) {
    if ('$' == *i) {
      if (i + 1 != format_string.end()) {
        ++i;
        DCHECK('$' == *i || '1' <= *i) << "Invalid placeholder: " << *i;
        if ('$' == *i) {
          formatted.push_back('$');
        } else {
          uintptr_t index = *i - '1';
          if (offsets) {
            ReplacementOffset r_offset(index,
                static_cast<int>(formatted.size()));
            r_offsets.insert(std::lower_bound(r_offsets.begin(),
                r_offsets.end(), r_offset,
                &CompareParameter),
                r_offset);
          }
          if (index < substitutions)
            formatted.append(subst.at(index));
        }
      }
    } else {
      formatted.push_back(*i);
    }
  }
  if (offsets) {
    for (std::vector<ReplacementOffset>::const_iterator i = r_offsets.begin();
        i != r_offsets.end(); ++i) {
      offsets->push_back(i->offset);
    }
  }
  return formatted;
}

string16 ReplaceStringPlaceholders(const string16& format_string,
                                   const std::vector<string16>& subst,
                                   std::vector<size_t>* offsets) {
  return DoReplaceStringPlaceholders(format_string, subst, offsets);
}

string ReplaceStringPlaceholders(const base::StringPiece& format_string,
                                      const std::vector<string>& subst,
                                      std::vector<size_t>* offsets) {
  return DoReplaceStringPlaceholders(format_string, subst, offsets);
}

string ReplaceStringPlaceholders(
    const base::StringPiece& format_string, ...) {
  va_list ap;
  va_start(ap, format_string);
  int count = 0;
  for (size_t i = 0; i < format_string.size() - 1; i++) {
    if (format_string[i] == '$') {
      if (format_string[i + 1] != '$') { 
        ++count;
      } else {
        // skip $$
        i += 1;
      }
    }
  }
  vector<string> svec;
  for (int i = 0; i < count; i++) {
    string val = va_arg(ap, const char*);
    svec.push_back(val);
  }
  va_end(ap);
  return ReplaceStringPlaceholders(format_string, svec, NULL);
}

string16 ReplaceStringPlaceholders(const string16& format_string,
                                   const string16& a,
                                   size_t* offset) {
  std::vector<size_t> offsets;
  std::vector<string16> subst;
  subst.push_back(a);
  string16 result = ReplaceStringPlaceholders(format_string, subst, &offsets);

  DCHECK(offsets.size() == 1);
  if (offset) {
    *offset = offsets[0];
  }
  return result;
}

template <class CHAR>
static bool IsWildcard(CHAR character) {
  return character == '*' || character == '?';
}

// Move the strings pointers to the point where they start to differ.
template <class CHAR>
static void EatSameChars(const CHAR** pattern, const CHAR** string) {
  bool escaped = false;
  while (**pattern && **string) {
    if (!escaped && IsWildcard(**pattern)) {
      // We don't want to match wildcard here, except if it's escaped.
      return;
    }

    // Check if the escapement char is found. If so, skip it and move to the
    // next character.
    if (!escaped && **pattern == L'\\') {
      escaped = true;
      (*pattern)++;
      continue;
    }

    // Check if the chars match, if so, increment the ptrs.
    if (**pattern == **string) {
      (*pattern)++;
      (*string)++;
    } else {
      // Uh ho, it did not match, we are done. If the last char was an
      // escapement, that means that it was an error to advance the ptr here,
      // let's put it back where it was. This also mean that the MatchPattern
      // function will return false because if we can't match an escape char
      // here, then no one will.
      if (escaped) {
        (*pattern)--;
      }
      return;
    }

    escaped = false;
  }
}

template <class CHAR>
static void EatWildcard(const CHAR** pattern) {
  while (**pattern) {
    if (!IsWildcard(**pattern))
      return;
    (*pattern)++;
  }
}

template <class CHAR>
static bool MatchPatternT(const CHAR* eval, const CHAR* pattern, int depth) {
  const int kMaxDepth = 16;
  if (depth > kMaxDepth)
    return false;

  // Eat all the matching chars.
  EatSameChars(&pattern, &eval);

  // If the string is empty, then the pattern must be empty too, or contains
  // only wildcards.
  if (*eval == 0) {
    EatWildcard(&pattern);
    if (*pattern)
      return false;
    return true;
  }

  // Pattern is empty but not string, this is not a match.
  if (*pattern == 0)
    return false;

  // If this is a question mark, then we need to compare the rest with
  // the current string or the string with one character eaten.
  if (pattern[0] == '?') {
    if (MatchPatternT(eval, pattern + 1, depth + 1) ||
        MatchPatternT(eval + 1, pattern + 1, depth + 1))
      return true;
  }

  // This is a *, try to match all the possible substrings with the remainder
  // of the pattern.
  if (pattern[0] == '*') {
    while (*eval) {
      if (MatchPatternT(eval, pattern + 1, depth + 1))
        return true;
      eval++;
    }

    // We reached the end of the string, let see if the pattern contains only
    // wildcards.
    if (*eval == 0) {
      EatWildcard(&pattern);
      if (*pattern)
        return false;
      return true;
    }
  }

  return false;
}

bool MatchPatternWide(const std::wstring& eval, const std::wstring& pattern) {
  return MatchPatternT(eval.c_str(), pattern.c_str(), 0);
}

bool MatchPatternASCII(const string& eval, const string& pattern) {
  DCHECK(IsStringASCII(eval) && IsStringASCII(pattern));
  return MatchPatternT(eval.c_str(), pattern.c_str(), 0);
}

bool StringToInt(const string& input, int* output) {
  return StringToNumber<StringToIntTraits>(input, output);
}

bool StringToInt(const string16& input, int* output) {
  return StringToNumber<String16ToIntTraits>(input, output);
}

bool StringToInt64(const string& input, int64* output) {
  return StringToNumber<StringToInt64Traits>(input, output);
}

bool StringToInt64(const string16& input, int64* output) {
  return StringToNumber<String16ToInt64Traits>(input, output);
}

bool HexStringToInt(const string& input, int* output) {
  return StringToNumber<HexStringToIntTraits>(input, output);
}

bool HexStringToInt(const string16& input, int* output) {
  return StringToNumber<HexString16ToIntTraits>(input, output);
}

namespace {

template<class CHAR>
bool HexDigitToIntT(const CHAR digit, uint8* val) {
  if (digit >= '0' && digit <= '9')
    *val = digit - '0';
  else if (digit >= 'a' && digit <= 'f')
    *val = 10 + digit - 'a';
  else if (digit >= 'A' && digit <= 'F')
    *val = 10 + digit - 'A';
  else
    return false;
  return true;
}

template<typename STR>
bool HexStringToBytesT(const STR& input, std::vector<uint8>* output) {
  DCHECK(output->size() == 0);
  size_t count = input.size();
  if (count == 0 || (count % 2) != 0)
    return false;
  for (uintptr_t i = 0; i < count / 2; ++i) {
    uint8 msb = 0;  // most significant 4 bits
    uint8 lsb = 0;  // least significant 4 bits
    if (!HexDigitToIntT(input[i * 2], &msb) ||
        !HexDigitToIntT(input[i * 2 + 1], &lsb))
      return false;
    output->push_back((msb << 4) | lsb);
  }
  return true;
}

}  // namespace

bool HexStringToBytes(const string& input, std::vector<uint8>* output) {
  return HexStringToBytesT(input, output);
}

bool HexStringToBytes(const string16& input, std::vector<uint8>* output) {
  return HexStringToBytesT(input, output);
}

int StringToInt(const string& value) {
  int result;
  StringToInt(value, &result);
  return result;
}

int StringToInt(const string16& value) {
  int result;
  StringToInt(value, &result);
  return result;
}
uint32 StringToUint(const string& value) {
  if (value.empty())
    return 0;
  return strtoui(value.c_str(), NULL, 10);
}

int64 StringToInt64(const string& value) {
  int64 result;
  StringToInt64(value, &result);
  return result;
}

int64 StringToInt64(const string16& value) {
  int64 result;
  StringToInt64(value, &result);
  return result;
}
uint64 StringToUint64(const string& value) {
  if (value.empty())
    return 0;
  return strtoull(value.c_str(), NULL, 10);
}
bool StringToUint64(const string& value, uint64* out) {
  if (value.empty())
    return false;
  *out = strtoull(value.c_str(), NULL, 10);
  return true;
}

int HexStringToInt(const string& value) {
  int result;
  HexStringToInt(value, &result);
  return result;
}

int HexStringToInt(const string16& value) {
  int result;
  HexStringToInt(value, &result);
  return result;
}

bool StringToDouble(const string& input, double* output) {
  return StringToNumber<StringToDoubleTraits>(input, output);
}

bool StringToDouble(const string16& input, double* output) {
  return StringToNumber<String16ToDoubleTraits>(input, output);
}

double StringToDouble(const string& value) {
  double result;
  StringToDouble(value, &result);
  return result;
}

double StringToDouble(const string16& value) {
  double result;
  StringToDouble(value, &result);
  return result;
}

// The following code is compatible with the OpenBSD lcpy interface.  See:
//   http://www.gratisoft.us/todd/papers/strlcpy.html
//   ftp://ftp.openbsd.org/pub/OpenBSD/src/lib/libc/string/{wcs,str}lcpy.c

namespace {

template <typename CHAR>
size_t lcpyT(CHAR* dst, const CHAR* src, size_t dst_size) {
  for (size_t i = 0; i < dst_size; ++i) {
    if ((dst[i] = src[i]) == 0)  // We hit and copied the terminating NULL.
      return i;
  }

  // We were left off at dst_size.  We over copied 1 byte.  Null terminate.
  if (dst_size != 0)
    dst[dst_size - 1] = 0;

  // Count the rest of the |src|, and return it's length in characters.
  while (src[dst_size]) ++dst_size;
  return dst_size;
}

}  // namespace

size_t strlcpy(char* dst, const char* src, size_t dst_size) {
  return lcpyT<char>(dst, src, dst_size);
}
size_t wcslcpy(wchar_t* dst, const wchar_t* src, size_t dst_size) {
  return lcpyT<wchar_t>(dst, src, dst_size);
}
void StringLowerCopy(char* dst, const char* src, size_t size) {
  for (size_t i = 0; i < size; ++i) {
      *dst++ = ToLowerASCII(*src++);
  }
  *dst = '\0';
}

bool ElideString(const std::wstring& input, int max_len, std::wstring* output) {
  DCHECK(max_len >= 0);
  if (static_cast<int>(input.length()) <= max_len) {
    output->assign(input);
    return false;
  }

  switch (max_len) {
    case 0:
      output->clear();
      break;
    case 1:
      output->assign(input.substr(0, 1));
      break;
    case 2:
      output->assign(input.substr(0, 2));
      break;
    case 3:
      output->assign(input.substr(0, 1) + L"." +
                     input.substr(input.length() - 1));
      break;
    case 4:
      output->assign(input.substr(0, 1) + L".." +
                     input.substr(input.length() - 1));
      break;
    default: {
      int rstr_len = (max_len - 3) / 2;
      int lstr_len = rstr_len + ((max_len - 3) % 2);
      output->assign(input.substr(0, lstr_len) + L"..." +
                     input.substr(input.length() - rstr_len));
      break;
    }
  }

  return true;
}

string HexEncode(const void* bytes, size_t size) {
  static const char kHexChars[] = "0123456789ABCDEF";

  // Each input byte creates two output hex characters.
  string ret(size * 2, '\0');

  for (size_t i = 0; i < size; ++i) {
    char b = reinterpret_cast<const char*>(bytes)[i];
    ret[(i * 2)] = kHexChars[(b >> 4) & 0xf];
    ret[(i * 2) + 1] = kHexChars[b & 0xf];
  }
  return ret;
}

string JoinVector(const std::vector<string>& values, char ch) {
  if (values.empty()) return string();
  string str;
  JoinVector(values, string(1, ch), &str);
  return str;
}

string JoinSet(const std::set<string>& values, const string& seperator) {
  if (values.empty()) return string();
  string str;
  for (const auto& item : values) {
    str.append(item);
    str.append(seperator);
  }
  str.resize(str.length() - seperator.size());
  return str;
}

string JoinVector(const std::vector<string>& values, const string& seperator) {
  if (values.empty()) return string();
  string str;
  JoinVector(values, seperator, &str);
  return str;
}

void JoinVector(const std::vector<string>& values,
                const string& seperator,
                string* str) {
  str->clear();
  if (values.empty()) return;
  for (size_t i = 0; i < values.size(); ++i) {
    str->append(values[i]);
    str->append(seperator);
  }
  str->resize(str->length() - seperator.size());
}

bool StringToLowerUtf8(const string& str, string* out) {
  const char* data = str.data();
  const size_t length = str.size();
  for (size_t i = 0; i < length; /* no update */) {
    int c = data[i++] & 0xff;
    if ((c & 0x80) == 0) {
      out->push_back(tolower(c));
    } else {
      if ((c & 0xc0) == 0x80) {
        LOG(ERROR)<< "UTF8StringToLabels: continuation byte as lead byte";
        return false;
      }
      out->push_back(c);
      int count = (c >= 0xc0) + (c >= 0xe0) + (c >= 0xf0) + (c >= 0xf8) +
      (c >= 0xfc);
      while (count != 0) {
        if (i == length) {
          LOG(ERROR) << "UTF8StringToLabels: truncated utf-8 byte sequence";
          return false;
        }
        char cb = data[i++];
        out->push_back(cb);
        if ((cb & 0xc0) != 0x80) {
          LOG(ERROR) << "UTF8StringToLabels: missing/invalid continuation byte";
          return false;
        }
        count--;
      }
    }
  }
  return true;
}

namespace base {

void SplitStringToVector(const string& full, const char* delimiters,
                         bool omit_empty_strings,
                         vector<string>* out) {
  CHECK(out != NULL);
  out->clear();

  size_t start = 0, end = full.size();
  size_t found = 0;
  while (found != string::npos) {
    found = full.find_first_of(delimiters, start);

    // start != end condition is for when the delimiter is at the end.
    if (!omit_empty_strings || (found != start && start != end))
      out->push_back(full.substr(start, found - start));
    start = found + 1;
  }
}

// Split utf8 string into characters.
bool SplitUTF8String(const string& str, vector<string>* characters) {
  const char* data = str.data();
  const size_t length = str.size();
  for (size_t i = 0; i < length; /* no update */) {
    int c = data[i++] & 0xff;
    if ((c & 0x80) == 0) {
      characters->push_back(UTF8CodeToUTF8String(c));
    } else {
      if ((c & 0xc0) == 0x80) {
        LOG(ERROR) << "UTF8StringToLabels: continuation byte as lead byte";
        return false;
      }
      int count = (c >= 0xc0) + (c >= 0xe0) + (c >= 0xf0) + (c >= 0xf8) +
                  (c >= 0xfc);
      int code = c & ((1 << (6 - count)) - 1);
      while (count != 0) {
        if (i == length) {
          LOG(ERROR) << "UTF8StringToLabels: truncated utf-8 byte sequence";
          return false;
        }
        char cb = data[i++];
        if ((cb & 0xc0) != 0x80) {
          LOG(ERROR) << "UTF8StringToLabels: missing/invalid continuation byte";
          return false;
        }
        code = (code << 6) | (cb & 0x3f);
        count--;
      }
      if (code < 0) {
        // This should not be able to happen.
        LOG(ERROR) << "UTF8StringToLabels: Invalid character found: " << c;
        return false;
      }
      characters->push_back(UTF8CodeToUTF8String(code));
    }
  }
  return true;
}

string UTF8CodeToUTF8String(int32 code) {
  std::ostringstream ostr;
  if (code < 0) {
    LOG(ERROR) << "LabelsToUTF8String: Invalid character found: " << code;
    return ostr.str();
  } else if (code < 0x80) {
    ostr << static_cast<char>(code);
  } else if (code < 0x800) {
    ostr << static_cast<char>((code >> 6) | 0xc0);
    ostr << static_cast<char>((code & 0x3f) | 0x80);
  } else if (code < 0x10000) {
    ostr << static_cast<char>((code >> 12) | 0xe0);
    ostr << static_cast<char>(((code >> 6) & 0x3f) | 0x80);
    ostr << static_cast<char>((code & 0x3f) | 0x80);
  } else if (code < 0x200000) {
    ostr << static_cast<char>((code >> 18) | 0xf0);
    ostr << static_cast<char>(((code >> 12) & 0x3f) | 0x80);
    ostr << static_cast<char>(((code >> 6) & 0x3f) | 0x80);
    ostr << static_cast<char>((code & 0x3f) | 0x80);
  } else if (code < 0x4000000) {
    ostr << static_cast<char>((code >> 24) | 0xf8);
    ostr << static_cast<char>(((code >> 18) & 0x3f) | 0x80);
    ostr << static_cast<char>(((code >> 12) & 0x3f) | 0x80);
    ostr << static_cast<char>(((code >> 6) & 0x3f) | 0x80);
    ostr << static_cast<char>((code & 0x3f) | 0x80);
  } else {
    ostr << static_cast<char>((code >> 30) | 0xfc);
    ostr << static_cast<char>(((code >> 24) & 0x3f) | 0x80);
    ostr << static_cast<char>(((code >> 18) & 0x3f) | 0x80);
    ostr << static_cast<char>(((code >> 12) & 0x3f) | 0x80);
    ostr << static_cast<char>(((code >> 6) & 0x3f) | 0x80);
    ostr << static_cast<char>((code & 0x3f) | 0x80);
  }
  return ostr.str();
}

}  // namespace base
