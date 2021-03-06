// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// This file defines utility functions for working with strings.

#ifndef BASE_STRING_UTIL_H_
#define BASE_STRING_UTIL_H_

#include <stdarg.h>   // va_list

#include <string>
#include <vector>

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/compat.h"
#include "base/string16.h"
#include "base/string_piece.h"  // For implicit conversions.
#include "base/marcos.h"

// Safe standard library wrappers for all platforms.

_START_BASE_NAMESPACE_
// C standard-library functions like "strncasecmp" and "snprintf" that aren't
// cross-platform are provided as "strncasecmp", and their prototypes
// are listed below.  These functions are then implemented as inline calls
// to the platform-specific equivalents in the platform-specific headers.

// Compares the two strings s1 and s2 without regard to case using
// the current locale; returns 0 if they are equal, 1 if s1 > s2, and -1 if
// s2 > s1 according to a lexicographic comparison.
//int strcasecmp(const char* s1, const char* s2);

inline int StrCaseCmp(const char* string1, const char* string2) {
  return strcasecmp(string1, string2);
}

// Compares up to count characters of s1 and s2 without regard to case using
// the current locale; returns 0 if they are equal, 1 if s1 > s2, and -1 if
// s2 > s1 according to a lexicographic comparison.
//int strncasecmp(const char* s1, const char* s2, size_t count);

// Same as strncmp but for char16 strings.
int strncmp16(const char16* s1, const char16* s2, size_t count);

// Wrapper for vsnprintf that always null-terminates and always returns the
// number of characters that would be in an untruncated formatted
// string, even when truncation occurs.
//int vsnprintf(char* buffer, size_t size, const char* format, va_list arguments)
//    PRINTF_FORMAT(3, 0);

// vswprintf always null-terminates, but when truncation occurs, it will either
// return -1 or the number of characters that would be in an untruncated
// formatted string.  The actual return value depends on the underlying
// C library's vswprintf implementation.
int vswprintf(wchar_t* buffer, size_t size,
              const wchar_t* format, va_list arguments) WPRINTF_FORMAT(3, 0);

// Some of these implementations need to be inlined.

// We separate the declaration from the implementation of this inline
// function just so the PRINTF_FORMAT works.
//inline int snprintf(char* buffer, size_t size, const char* format, ...)
//    PRINTF_FORMAT(3, 4);
//inline int snprintf(char* buffer, size_t size, const char* format, ...) {
//  va_list arguments;
//  va_start(arguments, format);
//  int result = vsnprintf(buffer, size, format, arguments);
//  va_end(arguments);
//  return result;
//}

// We separate the declaration from the implementation of this inline
// function just so the WPRINTF_FORMAT works.
//inline int swprintf(wchar_t* buffer, size_t size, const wchar_t* format, ...)
//    WPRINTF_FORMAT(3, 4);
//inline int swprintf(wchar_t* buffer, size_t size, const wchar_t* format, ...) {
//  va_list arguments;
//  va_start(arguments, format);
//  int result = vswprintf(buffer, size, format, arguments);
//  va_end(arguments);
//  return result;
//}

// BSD-style safe and consistent string copy functions.
// Copies |src| to |dst|, where |dst_size| is the total allocated size of |dst|.
// Copies at most |dst_size|-1 characters, and always NULL terminates |dst|, as
// long as |dst_size| is not 0.  Returns the length of |src| in characters.
// If the return value is >= dst_size, then the output was truncated.
// NOTE: All sizes are in number of characters, NOT in bytes.
size_t strlcpy(char* dst, const char* src, size_t dst_size);
size_t wcslcpy(wchar_t* dst, const wchar_t* src, size_t dst_size);
// TODO(yesp) : improve with stringencoders
void StringLowerCopy(char* dst, const char* src, size_t size);

// Scan a wprintf format string to determine whether it's portable across a
// variety of systems.  This function only checks that the conversion
// specifiers used by the format string are supported and have the same meaning
// on a variety of systems.  It doesn't check for other errors that might occur
// within a format string.
//
// Nonportable conversion specifiers for wprintf are:
//  - 's' and 'c' without an 'l' length modifier.  %s and %c operate on char
//     data on all systems except Windows, which treat them as wchar_t data.
//     Use %ls and %lc for wchar_t data instead.
//  - 'S' and 'C', which operate on wchar_t data on all systems except Windows,
//     which treat them as char data.  Use %ls and %lc for wchar_t data
//     instead.
//  - 'F', which is not identified by Windows wprintf documentation.
//  - 'D', 'O', and 'U', which are deprecated and not available on all systems.
//     Use %ld, %lo, and %lu instead.
//
// Note that there is no portable conversion specifier for char data when
// working with wprintf.
//
// This function is intended to be called from vswprintf.
bool IsWprintfFormatPortable(const wchar_t* format);

}  // namespace base

#if defined(OS_WIN)
#include "base/string_util_win.h"
#elif defined(OS_POSIX)
#include "base/string_util_posix.h"
#else
#error Define string operations appropriately for your platform
#endif

// These threadsafe functions return references to globally unique empty
// strings.
//
// DO NOT USE THESE AS A GENERAL-PURPOSE SUBSTITUTE FOR DEFAULT CONSTRUCTORS.
// There is only one case where you should use these: functions which need to
// return a string by reference (e.g. as a class member accessor), and don't
// have an empty string to use (e.g. in an error case).  These should not be
// used as initializers, function arguments, or return values for functions
// which return by value or outparam.
const string& EmptyString();
const std::wstring& EmptyWString();
const string16& EmptyString16();

extern const wchar_t kWhitespaceWide[];
extern const char16 kWhitespaceUTF16[];
extern const char kWhitespaceASCII[];

extern const char kUtf8ByteOrderMark[];

// Removes characters in remove_chars from anywhere in input.  Returns true if
// any characters were removed.
// NOTE: Safe to use the same variable for both input and output.
bool RemoveChars(const std::wstring& input,
                 const wchar_t remove_chars[],
                 std::wstring* output);
bool RemoveChars(const string16& input,
                 const char16 remove_chars[],
                 string16* output);
bool RemoveChars(const string& input,
                 const char remove_chars[],
                 string* output);

// Removes characters in trim_chars from the beginning and end of input.
// NOTE: Safe to use the same variable for both input and output.
bool TrimString(const std::wstring& input,
                const wchar_t trim_chars[],
                std::wstring* output);
bool TrimString(const string16& input,
                const char16 trim_chars[],
                string16* output);
bool TrimString(const string& input,
                const char trim_chars[],
                string* output);

// Truncates a string to the nearest UTF-8 character that will leave
// the string less than or equal to the specified byte size.
void TruncateUTF8ToByteSize(const string& input,
                            const size_t byte_size,
                            string* output);

// Trims any whitespace from either end of the input string.  Returns where
// whitespace was found.
// The non-wide version has two functions:
// * TrimWhitespaceASCII()
//   This function is for ASCII strings and only looks for ASCII whitespace;
// Please choose the best one according to your usage.
// NOTE: Safe to use the same variable for both input and output.
enum TrimPositions {
  TRIM_NONE     = 0,
  TRIM_LEADING  = 1 << 0,
  TRIM_TRAILING = 1 << 1,
  TRIM_ALL      = TRIM_LEADING | TRIM_TRAILING,
};
TrimPositions TrimWhitespace(const std::wstring& input,
                             TrimPositions positions,
                             std::wstring* output);
TrimPositions TrimWhitespace(const string16& input,
                             TrimPositions positions,
                             string16* output);
TrimPositions TrimWhitespaceASCII(const string& input,
                                  TrimPositions positions,
                                  string* output);

// Deprecated. This function is only for backward compatibility and calls
// TrimWhitespaceASCII().
TrimPositions TrimWhitespace(const string& input,
                             TrimPositions positions,
                             string* output);

// Searches  for CR or LF characters.  Removes all contiguous whitespace
// strings that contain them.  This is useful when trying to deal with text
// copied from terminals.
// Returns |text|, with the following three transformations:
// (1) Leading and trailing whitespace is trimmed.
// (2) If |trim_sequences_with_line_breaks| is true, any other whitespace
//     sequences containing a CR or LF are trimmed.
// (3) All other whitespace sequences are converted to single spaces.
std::wstring CollapseWhitespace(const std::wstring& text,
                                bool trim_sequences_with_line_breaks);
string16 CollapseWhitespace(const string16& text,
                            bool trim_sequences_with_line_breaks);
string CollapseWhitespaceASCII(const string& text,
                                    bool trim_sequences_with_line_breaks);

// Returns true if the passed string is empty or contains only white-space
// characters.
bool ContainsOnlyWhitespaceASCII(const string& str);
bool ContainsOnlyWhitespace(const string16& str);

// Returns true if |input| is empty or contains only characters found in
// |characters|.
bool ContainsOnlyChars(const std::wstring& input,
                       const std::wstring& characters);
bool ContainsOnlyChars(const string16& input, const string16& characters);
bool ContainsOnlyChars(const string& input, const string& characters);

// These convert between ASCII (7-bit) and Wide/UTF16 strings.
string WideToASCII(const std::wstring& wide);
std::wstring ASCIIToWide(const base::StringPiece& ascii);
string UTF16ToASCII(const string16& utf16);
string16 ASCIIToUTF16(const base::StringPiece& ascii);

// Converts the given wide string to the corresponding Latin1. This will fail
// (return false) if any characters are more than 255.
bool WideToLatin1(const std::wstring& wide, string* latin1);

// Returns true if the specified string matches the criteria. How can a wide
// string be 8-bit or UTF8? It contains only characters that are < 256 (in the
// first case) or characters that use only 8-bits and whose 8-bit
// representation looks like a UTF-8 string (the second case).
//
// Note that IsStringUTF8 checks not only if the input is structrually
// valid but also if it doesn't contain any non-character codepoint
// (e.g. U+FFFE). It's done on purpose because all the existing callers want
// to have the maximum 'discriminating' power from other encodings. If
// there's a use case for just checking the structural validity, we have to
// add a new function for that.
bool IsString8Bit(const std::wstring& str);
bool IsStringUTF8(const string& str);
bool IsStringASCII(const std::wstring& str);
bool IsStringASCII(const base::StringPiece& str);
bool IsStringASCII(const string16& str);

// ASCII-specific tolower.  The standard library's tolower is locale sensitive,
// so we don't want to use it here.
template <class Char> inline Char ToLowerASCII(Char c) {
  return (c >= 'A' && c <= 'Z') ? (c + ('a' - 'A')) : c;
}

// Converts the elements of the given string.  This version uses a pointer to
// clearly differentiate it from the non-pointer variant.
template <class str> inline void StringToLowerASCII(str* s) {
  for (typename str::iterator i = s->begin(); i != s->end(); ++i)
    *i = ToLowerASCII(*i);
}

template <class str> inline str StringToLowerASCII(const str& s) {
  // for string and std::wstring
  str output(s);
  StringToLowerASCII(&output);
  return output;
}

//  If string is ascii , call StringToLowerASCII
bool StringToLowerUtf8(const string& str, string* out);

// ASCII-specific toupper.  The standard library's toupper is locale sensitive,
// so we don't want to use it here.
template <class Char> inline Char ToUpperASCII(Char c) {
  return (c >= 'a' && c <= 'z') ? (c + ('A' - 'a')) : c;
}

// Converts the elements of the given string.  This version uses a pointer to
// clearly differentiate it from the non-pointer variant.
template <class str> inline void StringToUpperASCII(str* s) {
  for (typename str::iterator i = s->begin(); i != s->end(); ++i)
    *i = ToUpperASCII(*i);
}

template <class str> inline str StringToUpperASCII(const str& s) {
  // for string and std::wstring
  str output(s);
  StringToUpperASCII(&output);
  return output;
}

// Compare the lower-case form of the given string against the given ASCII
// string.  This is useful for doing checking if an input string matches some
// token, and it is optimized to avoid intermediate string copies.  This API is
// borrowed from the equivalent APIs in Mozilla.
bool LowerCaseEqualsASCII(const string& a, const char* b);
bool LowerCaseEqualsASCII(const std::wstring& a, const char* b);
bool LowerCaseEqualsASCII(const string16& a, const char* b);

// Same thing, but with string iterators instead.
bool LowerCaseEqualsASCII(string::const_iterator a_begin,
                          string::const_iterator a_end,
                          const char* b);
bool LowerCaseEqualsASCII(std::wstring::const_iterator a_begin,
                          std::wstring::const_iterator a_end,
                          const char* b);
bool LowerCaseEqualsASCII(string16::const_iterator a_begin,
                          string16::const_iterator a_end,
                          const char* b);
bool LowerCaseEqualsASCII(const char* a_begin,
                          const char* a_end,
                          const char* b);
bool LowerCaseEqualsASCII(const wchar_t* a_begin,
                          const wchar_t* a_end,
                          const char* b);
bool LowerCaseEqualsASCII(const char16* a_begin,
                          const char16* a_end,
                          const char* b);

// Performs a case-sensitive string compare. The behavior is undefined if both
// strings are not ASCII.
bool EqualsASCII(const string16& a, const base::StringPiece& b);

// Returns true if str starts with search, or false otherwise.
bool StartsWithASCII(const string& str,
                     const string& search,
                     bool case_sensitive);
bool StartsWith(const std::wstring& str,
                const std::wstring& search,
                bool case_sensitive);
bool StartsWith(const string16& str,
                const string16& search,
                bool case_sensitive);

// Returns true if str ends with search, or false otherwise.
bool EndsWith(const string& str,
              const string& search,
              bool case_sensitive);
bool EndsWith(const std::wstring& str,
              const std::wstring& search,
              bool case_sensitive);
bool EndsWith(const string16& str,
              const string16& search,
              bool case_sensitive);


// Determines the type of ASCII character, independent of locale (the C
// library versions will change based on locale).
template <typename Char>
inline bool IsAsciiWhitespace(Char c) {
  return c == ' ' || c == '\r' || c == '\n' || c == '\t';
}
template <typename Char>
inline bool IsAsciiAlpha(Char c) {
  return ((c >= 'A') && (c <= 'Z')) || ((c >= 'a') && (c <= 'z'));
}
template <typename Char>
inline bool IsAsciiDigit(Char c) {
  return c >= '0' && c <= '9';
}

// Returns true if it's a whitespace character.
inline bool IsWhitespace(wchar_t c) {
  return wcschr(kWhitespaceWide, c) != NULL;
}

enum DataUnits {
  DATA_UNITS_BYTE = 0,
  DATA_UNITS_KIBIBYTE,
  DATA_UNITS_MEBIBYTE,
  DATA_UNITS_GIBIBYTE,
};

// Return the unit type that is appropriate for displaying the amount of bytes
// passed in.
DataUnits GetByteDisplayUnits(int64 bytes);

// Return a byte string in human-readable format, displayed in units appropriate
// specified by 'units', with an optional unit suffix.
// Ex: FormatBytes(512, DATA_UNITS_KIBIBYTE, true) => "0.5 KB"
// Ex: FormatBytes(10*1024, DATA_UNITS_MEBIBYTE, false) => "0.1"
std::wstring FormatBytes(int64 bytes, DataUnits units, bool show_units);

// As above, but with "/s" units.
// Ex: FormatSpeed(512, DATA_UNITS_KIBIBYTE, true) => "0.5 KB/s"
// Ex: FormatSpeed(10*1024, DATA_UNITS_MEBIBYTE, false) => "0.1"
std::wstring FormatSpeed(int64 bytes, DataUnits units, bool show_units);

// Return a number formated with separators in the user's locale way.
// Ex: FormatNumber(1234567) => 1,234,567
std::wstring FormatNumber(int64 number);

// Starting at |start_offset| (usually 0), replace the first instance of
// |find_this| with |replace_with|.
void ReplaceFirstSubstringAfterOffset(string16* str,
                                      string16::size_type start_offset,
                                      const string16& find_this,
                                      const string16& replace_with);
void ReplaceFirstSubstringAfterOffset(string* str,
                                      string::size_type start_offset,
                                      const string& find_this,
                                      const string& replace_with);

// Starting at |start_offset| (usually 0), look through |str| and replace all
// instances of |find_this| with |replace_with|.
//
// This does entire substrings; use std::replace in <algorithm> for single
// characters, for example:
//   std::replace(str.begin(), str.end(), 'a', 'b');
void ReplaceSubstringsAfterOffset(string16* str,
                                  string16::size_type start_offset,
                                  const string16& find_this,
                                  const string16& replace_with);
void ReplaceSubstringsAfterOffset(string* str,
                                  string::size_type start_offset,
                                  const string& find_this,
                                  const string& replace_with);

// Specialized string-conversion functions.
string IntToString(int value);
std::wstring IntToWString(int value);
string16 IntToString16(int value);
string UintToString(unsigned int value);
std::wstring UintToWString(unsigned int value);
string16 UintToString16(unsigned int value);
string Int64ToString(int64 value);
std::wstring Int64ToWString(int64 value);
string Uint64ToString(uint64 value);
std::wstring Uint64ToWString(uint64 value);
// The DoubleToString methods convert the double to a string format that
// ignores the locale.  If you want to use locale specific formatting, use ICU.
string DoubleToString(double value);
std::wstring DoubleToWString(double value);

// Perform a best-effort conversion of the input string to a numeric type,
// setting |*output| to the result of the conversion.  Returns true for
// "perfect" conversions; returns false in the following cases:
//  - Overflow/underflow.  |*output| will be set to the maximum value supported
//    by the data type.
//  - Trailing characters in the string after parsing the number.  |*output|
//    will be set to the value of the number that was parsed.
//  - No characters parseable as a number at the beginning of the string.
//    |*output| will be set to 0.
//  - Empty string.  |*output| will be set to 0.
bool StringToInt(const string& input, int* output);
bool StringToInt(const string16& input, int* output);
bool StringToInt64(const string& input, int64* output);
bool StringToInt64(const string16& input, int64* output);
bool HexStringToInt(const string& input, int* output);
bool HexStringToInt(const string16& input, int* output);

// Similar to the previous functions, except that output is a vector of bytes.
// |*output| will contain as many bytes as were successfully parsed prior to the
// error.  There is no overflow, but input.size() must be evenly divisible by 2.
// Leading 0x or +/- are not allowed.
bool HexStringToBytes(const string& input, std::vector<uint8>* output);
bool HexStringToBytes(const string16& input, std::vector<uint8>* output);

// For floating-point conversions, only conversions of input strings in decimal
// form are defined to work.  Behavior with strings representing floating-point
// numbers in hexadecimal, and strings representing non-fininte values (such as
// NaN and inf) is undefined.  Otherwise, these behave the same as the integral
// variants.  This expects the input string to NOT be specific to the locale.
// If your input is locale specific, use ICU to read the number.
bool StringToDouble(const string& input, double* output);
bool StringToDouble(const string16& input, double* output);

// Convenience forms of the above, when the caller is uninterested in the
// boolean return value.  These return only the |*output| value from the
// above conversions: a best-effort conversion when possible, otherwise, 0.
int StringToInt(const string& value);
int StringToInt(const string16& value);
uint32 StringToUint(const string& value);
int64 StringToInt64(const string& value);
int64 StringToInt64(const string16& value);
uint64 StringToUint64(const string& value);
bool StringToUint64(const string& value, uint64* out);
int HexStringToInt(const string& value);
int HexStringToInt(const string16& value);
double StringToDouble(const string& value);
double StringToDouble(const string16& value);

// Return a C++ string given printf-like input.
string StringPrintf(const char* format, ...) PRINTF_FORMAT(1, 2);
std::wstring StringPrintf(const wchar_t* format, ...) WPRINTF_FORMAT(1, 2);

// Return a C++ string given vprintf-like input.
string StringPrintV(const char* format, va_list ap) PRINTF_FORMAT(1, 0);

// Store result into a supplied string and return it
const string& SStringPrintf(string* dst, const char* format, ...)
    PRINTF_FORMAT(2, 3);
const std::wstring& SStringPrintf(std::wstring* dst,
                                  const wchar_t* format, ...)
    WPRINTF_FORMAT(2, 3);

// Append result to a supplied string
void StringAppendF(string* dst, const char* format, ...)
    PRINTF_FORMAT(2, 3);
void StringAppendF(std::wstring* dst, const wchar_t* format, ...)
    WPRINTF_FORMAT(2, 3);

// Lower-level routine that takes a va_list and appends to a specified
// string.  All other routines are just convenience wrappers around it.
void StringAppendV(string* dst, const char* format, va_list ap)
    PRINTF_FORMAT(2, 0);
void StringAppendV(std::wstring* dst, const wchar_t* format, va_list ap)
    WPRINTF_FORMAT(2, 0);

// This is mpcomplete's pattern for saving a string copy when dealing with
// a function that writes results into a wchar_t[] and wanting the result to
// end up in a std::wstring.  It ensures that the std::wstring's internal
// buffer has enough room to store the characters to be written into it, and
// sets its .length() attribute to the right value.
//
// The reserve() call allocates the memory required to hold the string
// plus a terminating null.  This is done because resize() isn't
// guaranteed to reserve space for the null.  The resize() call is
// simply the only way to change the string's 'length' member.
//
// XXX-performance: the call to wide.resize() takes linear time, since it fills
// the string's buffer with nulls.  I call it to change the length of the
// string (needed because writing directly to the buffer doesn't do this).
// Perhaps there's a constant-time way to change the string's length.
template <class string_type>
inline typename string_type::value_type* WriteInto(string_type* str,
                                                   size_t length_with_null) {
  str->reserve(length_with_null);
  str->resize(length_with_null - 1);
  return &((*str)[0]);
}

//-----------------------------------------------------------------------------

// Function objects to aid in comparing/searching strings.

template<typename Char> struct CaseInsensitiveCompare {
 public:
  bool operator()(Char x, Char y) const {
    // TODO(darin): Do we really want to do locale sensitive comparisons here?
    // See http://crbug.com/24917
    return tolower(x) == tolower(y);
  }
};

template<typename Char> struct CaseInsensitiveCompareASCII {
 public:
  bool operator()(Char x, Char y) const {
    return ToLowerASCII(x) == ToLowerASCII(y);
  }
};

// TODO(timsteele): Move these split string functions into their own API on
// string_split.cc/.h files.
//-----------------------------------------------------------------------------

// Splits |str| into a vector of strings delimited by |s|. Append the results
// into |r| as they appear. If several instances of |s| are contiguous, or if
// |str| begins with or ends with |s|, then an empty string is inserted.
//
// Every substring is trimmed of any leading or trailing white space.
void SplitString(const std::wstring& str,
                 wchar_t s,
                 std::vector<std::wstring>* r);
void SplitString(const string16& str,
                 char16 s,
                 std::vector<string16>* r);
void SplitString(const string& str,
                 char s,
                 std::vector<string>* r);

// The same as SplitString, but don't trim white space.
void SplitStringDontTrim(const std::wstring& str,
                         wchar_t s,
                         std::vector<std::wstring>* r);
void SplitStringDontTrim(const string16& str,
                         char16 s,
                         std::vector<string16>* r);
void SplitStringDontTrim(const string& str,
                         char s,
                         std::vector<string>* r);

// The same as SplitString, but use a substring delimiter instead of a char.
void SplitStringUsingSubstr(const string16& str,
                            const string16& s,
                            std::vector<string16>* r);
void SplitStringUsingSubstr(const string& str,
                            const string& s,
                            std::vector<string>* r);

// Splits a string into its fields delimited by any of the characters in
// |delimiters|.  Each field is added to the |tokens| vector.  Returns the
// number of tokens found.
size_t Tokenize(const std::wstring& str,
                const std::wstring& delimiters,
                std::vector<std::wstring>* tokens);
size_t Tokenize(const string16& str,
                const string16& delimiters,
                std::vector<string16>* tokens);
size_t Tokenize(const string& str,
                const string& delimiters,
                std::vector<string>* tokens);
size_t Tokenize(const base::StringPiece& str,
                const base::StringPiece& delimiters,
                std::vector<base::StringPiece>* tokens);

// Does the opposite of SplitString().
std::wstring JoinString(const std::vector<std::wstring>& parts, wchar_t s);
string16 JoinString(const std::vector<string16>& parts, char16 s);
string JoinString(const std::vector<string>& parts, char s);
string JoinString(std::vector<string>::const_iterator begin,
                  std::vector<string>::const_iterator end,
                  char s);

// WARNING: this uses whitespace as defined by the HTML5 spec. If you need
// a function similar to this but want to trim all types of whitespace, then
// factor this out into a function that takes a string containing the characters
// that are treated as whitespace.
//
// Splits the string along whitespace (where whitespace is the five space
// characters defined by HTML 5). Each contiguous block of non-whitespace
// characters is added to result.
void SplitStringAlongWhitespace(const std::wstring& str,
                                std::vector<std::wstring>* result);
void SplitStringAlongWhitespace(const string16& str,
                                std::vector<string16>* result);
void SplitStringAlongWhitespace(const string& str,
                                std::vector<string>* result);

// Replace $1-$2-$3..$9 in the format string with |a|-|b|-|c|..|i| respectively.
// Additionally, $$ is replaced by $. The offsets parameter here can
// be NULL. This only allows you to use up to nine replacements.
string16 ReplaceStringPlaceholders(const string16& format_string,
                                   const std::vector<string16>& subst,
                                   std::vector<size_t>* offsets);

string ReplaceStringPlaceholders(const base::StringPiece& format_string,
                                      const std::vector<string>& subst,
                                      std::vector<size_t>* offsets);

string ReplaceStringPlaceholders(
    const base::StringPiece& format_string, ...);

// Single-string shortcut for ReplaceStringHolders.
string16 ReplaceStringPlaceholders(const string16& format_string,
                                   const string16& a,
                                   size_t* offset);

// If the size of |input| is more than |max_len|, this function returns true and
// |input| is shortened into |output| by removing chars in the middle (they are
// replaced with up to 3 dots, as size permits).
// Ex: ElideString(L"Hello", 10, &str) puts Hello in str and returns false.
// ElideString(L"Hello my name is Tom", 10, &str) puts "Hell...Tom" in str and
// returns true.
bool ElideString(const std::wstring& input, int max_len, std::wstring* output);

// Returns true if the string passed in matches the pattern. The pattern
// string can contain wildcards like * and ?
// The backslash character (\) is an escape character for * and ?
// We limit the patterns to having a max of 16 * or ? characters.
bool MatchPatternWide(const std::wstring& string, const std::wstring& pattern);
bool MatchPatternASCII(const std::string& string, const std::string& pattern);

// Returns a hex string representation of a binary buffer.
// The returned hex string will be in upper case.
// This function does not check if |size| is within reasonable limits since
// it's written with trusted data in mind.
// If you suspect that the data you want to format might be large,
// the absolute max size for |size| should be is
//   std::numeric_limits<size_t>::max() / 2
string HexEncode(const void* bytes, size_t size);

string JoinVector(const std::vector<string>& values, char ch);
string JoinVector(const std::vector<string>& values, const string& seperator);
void JoinVector(const std::vector<string>& values, const string& seperator, string* ret);

string JoinSet(const std::set<string>& values, const string& seperator);

// Hack to convert any char-like type to its unsigned counterpart.
// For example, it will convert char, signed char and unsigned char to unsigned
// char.
template<typename T>
struct ToUnsigned {
  typedef T Unsigned;
};

template<>
struct ToUnsigned<char> {
  typedef unsigned char Unsigned;
};
template<>
struct ToUnsigned<signed char> {
  typedef unsigned char Unsigned;
};
template<>
struct ToUnsigned<wchar_t> {
#if defined(WCHAR_T_IS_UTF16)
  typedef unsigned short Unsigned;
#elif defined(WCHAR_T_IS_UTF32)
  typedef uint32 Unsigned;
#endif
};
template<>
struct ToUnsigned<short> {
  typedef unsigned short Unsigned;
};

namespace base {

// Splits string using any of the single character delimiters.
// If omit_empty_strings == true, the output will contain any
// nonempty strings after splitting on any of the
// characters in the delimiter.  If omit_empty_strings == false,
// the output will contain n+1 strings if there are n characters
// in the set "delim" within the input string.  In this case
// the empty string is split to a single empty string.
void SplitStringToVector(const string& full, const char* delimiters,
                         bool omit_empty_strings,
                         std::vector<string>* out);

// Split utf8 string into characters.
bool SplitUTF8String(const string& str,
                     std::vector<string>* characters);

string UTF8CodeToUTF8String(int32_t code);

_END_BASE_NAMESPACE_

#endif  // BASE_STRING_UTIL_H_
