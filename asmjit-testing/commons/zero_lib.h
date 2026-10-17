// ZeroLib - Shared utilities for both ZeroBench and ZeroTest.
//
// This is free and unencumbered software released into the public domain.
//
// Anyone is free to copy, modify, publish, use, compile, sell, or
// distribute this software, either in source code form or as a compiled
// binary, for any purpose, commercial or non-commercial, and by any
// means.
//
// In jurisdictions that recognize copyright laws, the author or authors
// of this software dedicate any and all copyright interest in the
// software to the public domain. We make this dedication for the benefit
// of the public at large and to the detriment of our heirs and
// successors. We intend this dedication to be an overt act of
// relinquishment in perpetuity of all present and future rights to this
// software under copyright law.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
// OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.
//
// For more information, please refer to <http://unlicense.org>

#ifndef ZERO_LIB_H_INCLUDED
#define ZERO_LIB_H_INCLUDED

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <type_traits>

#if defined(__clang__) && !defined(_DOXYGEN)
  #define ZERO_INLINE_NODEBUG inline __attribute__((__always_inline__, __nodebug__))
#elif defined(__GNUC__) && !defined(_DOXYGEN)
  #define ZERO_INLINE_NODEBUG inline __attribute__((__always_inline__, __artificial__))
#else
  #define ZERO_INLINE_NODEBUG inline
#endif

//! \cond ZERO_DOC

// C++ Utilities
// =============

namespace ZeroLib {

template<typename T>
[[maybe_unused]] [[nodiscard]]
static constexpr ZERO_INLINE_NODEBUG T&& forward(std::remove_reference_t<T>& v) noexcept { return static_cast<T&&>(v); }

template<typename T>
[[maybe_unused]] [[nodiscard]]
static constexpr ZERO_INLINE_NODEBUG T&& forward(std::remove_reference_t<T>&& v) noexcept { return static_cast<T&&>(v); }

template<typename T>
[[maybe_unused]]
static inline void consume(const T& value) noexcept { [[maybe_unused]] volatile T dummy = value; }

// Color Support
// =============

//! A color
enum class Color : uint8_t {
  kNone = 0,
  kBlack,
  kReg,
  kGreen,
  kYellow,
  kBlue,
  kMagenta,
  kCyan,
  kWhite,
  kGray,
  kBrightRed,
  kBrightGreen,
  kBrightYellow,
  kBrightBlue,
  kBrightMagenta,
  kBrightCyan,
  kBrightWhite,

  kMaxValue = kBrightWhite
};

//! Detects whether color output is enabled by the terminal and whether the used didn't explicitly requested no colors.
bool is_color_output_supported() noexcept;

//! Used to enable color output - should be only called once at the application start.
void set_color_output(bool value) noexcept;

// String Container
// ================

// A minimal string buffer.
class String {
public:
  char* _data {};
  size_t _size {};
  size_t _capacity {};
  bool _failed = false;
  bool _temporary = false;

  ZERO_INLINE_NODEBUG String() noexcept {}
  ZERO_INLINE_NODEBUG ~String() noexcept { reset(); }

  ZERO_INLINE_NODEBUG char* data() noexcept { return _data; }
  ZERO_INLINE_NODEBUG const char* data() const noexcept { return _data; }

  ZERO_INLINE_NODEBUG bool is_empty() const noexcept { return _size == 0u; }
  ZERO_INLINE_NODEBUG size_t size() const noexcept { return _size; }
  ZERO_INLINE_NODEBUG size_t capacity() const noexcept { return _capacity; }

  ZERO_INLINE_NODEBUG bool failed() const noexcept { return _failed; }

  void clear() noexcept;
  void reset() noexcept;
  bool reserve_for_append(size_t append_size) noexcept;

  void assign(const char* s) noexcept;
  void assign(const char* s, size_t size) noexcept;
  ZERO_INLINE_NODEBUG void assign(const String& str) noexcept { assign(str.data(), str.size()); }

  void append(const char c) noexcept;
  void append_chars(const char c, size_t count) noexcept;

  void append(const char* s) noexcept;
  void append(const char* s, size_t size) noexcept;
  ZERO_INLINE_NODEBUG void append(const String& str) noexcept { return append(str.data(), str.size()); }

  void append_padded(const char* s, size_t size, size_t left_padding, size_t right_padding) noexcept;

  ZERO_INLINE_NODEBUG void append_padded(const String& str, size_t left_padding, size_t right_padding) noexcept {
    return append_padded(str.data(), str.size(), left_padding, right_padding);
  }

  void append_indented(const char* s, size_t size, size_t indentation_size, bool indent_first) noexcept;

  void append_format(const char* fmt, ...) noexcept;
  void append_format_v(const char* fmt, va_list ap) noexcept;

  void append_start_escape(Color color) noexcept;
  void append_end_escape(Color color) noexcept;

  void c_append(Color color, const char* s) noexcept;
  void c_append(Color color, const char* s, size_t len) noexcept;
  ZERO_INLINE_NODEBUG void c_append(Color color, const String& str) noexcept { c_append(color, str.data(), str.size()); }

  void c_append_padded(Color color, const char* s, size_t size, size_t left_padding, size_t right_padding) noexcept;

  ZERO_INLINE_NODEBUG void c_append_padded(Color color, const String& str, size_t left_padding, size_t right_padding) noexcept {
    return c_append_padded(color, str.data(), str.size(), left_padding, right_padding);
  }

  void c_append_format_v(Color color, const char* fmt, va_list ap) noexcept;
  void c_append_format_s(Color color, const char* fmt, ...) noexcept;

  template<typename... Args>
  ZERO_INLINE_NODEBUG void c_append_format(Color color, const char* msg, Args&&... args) noexcept {
    if constexpr (sizeof...(args) == 0) {
      c_append(color, msg, strlen(msg));
    }
    else {
      c_append_format_s(color, msg, forward<Args>(args)...);
    }
  }

  ZERO_INLINE_NODEBUG void append_value(const bool& value) noexcept { append(value ? "true" : "false"); }
  ZERO_INLINE_NODEBUG void append_value(const char& value) noexcept { append_format("%u", uint8_t(value)) ; }
  ZERO_INLINE_NODEBUG void append_value(const signed char& value) noexcept { append_format("%d", int(value)) ; }
  ZERO_INLINE_NODEBUG void append_value(const unsigned char& value) noexcept { append_format("%u", unsigned(value)); }
  ZERO_INLINE_NODEBUG void append_value(const short& value) noexcept { append_format("%d", int(value)); }
  ZERO_INLINE_NODEBUG void append_value(const unsigned short& value) noexcept { append_format("%u", unsigned(value)); }
  ZERO_INLINE_NODEBUG void append_value(const int& value) noexcept { append_format("%d", value); }
  ZERO_INLINE_NODEBUG void append_value(const unsigned int& value) noexcept { append_format("%u", value); }
  ZERO_INLINE_NODEBUG void append_value(const long& value) noexcept { append_format("%ld", value); }
  ZERO_INLINE_NODEBUG void append_value(const unsigned long& value) noexcept { append_format("%lu", value); }
  ZERO_INLINE_NODEBUG void append_value(const long long& value) noexcept { append_format("%lld", value); }
  ZERO_INLINE_NODEBUG void append_value(const unsigned long long& value) noexcept { append_format("%llu", value); }
  ZERO_INLINE_NODEBUG void append_value(const float& value) noexcept { append_format("%g", double(value)); }
  ZERO_INLINE_NODEBUG void append_value(const double& value) noexcept { append_format("%g", double(value)); }

  bool eat_nl() noexcept;
};

template<size_t N>
class StringTmp : public String {
public:
  char _embedded_data[N];

  ZERO_INLINE_NODEBUG StringTmp() noexcept {
    _data = _embedded_data;
    _capacity = N;
    _temporary = true;
  }

  ZERO_INLINE_NODEBUG void reset() noexcept {
    _size = 0u;
    _failed = false;

    if (_data && !_temporary) {
      free(_data);
      _data = _embedded_data;
      _capacity = N;
      _temporary = true;
    }
  }
};

[[maybe_unused]]
static void print(const char* data, size_t size) noexcept { fwrite(data, size, 1, stdout); }

[[maybe_unused]]
static void print(const String& buf) noexcept { print(buf.data(), buf.size()); }

// StringUtils
// ===========

namespace StringUtils {

//! Get whether the strings `a` [0..a_len] and `b` [0..b_len] are equal, ignoring case and treating `-` as `_`.
//!
//! The first `a` parameters are a test/bench case name, and `b` is considered a filter to match. It's supported
//! that `b` is an exact match, suffix match when b="*xxx", prefix match when b="xxx*", and substring match when
//! b="*xxx*".
bool match_case(const char* a, size_t a_len, const char* b, size_t b_len) noexcept;

static inline bool match_case(const String& a, const String& b) noexcept {
  return match_case(a.data(), a.size(), b.data(), b.size());
}

} // {StringUtils}

// ZeroCmdLine
// -----------

//! Provides lightweight command-line parsing.
class CmdLine {
public:
  int _argc;
  const char* const* _argv;

  ZERO_INLINE_NODEBUG CmdLine(int argc, const char* const* argv)
    : _argc(argc),
      _argv(argv) {}

  ZERO_INLINE_NODEBUG int count() const { return _argc; }
  ZERO_INLINE_NODEBUG const char* const* args() const { return _argv; }

  int find_arg(const char* key) const {
    for (int i = 1; i < _argc; i++) {
      if (strcmp(key, _argv[i]) == 0)
        return i;
    }

    return -1;
  }

  bool has_arg(const char* key) const {
    return find_arg(key) > 0;
  }

  const char* get_arg(const char* key, const char* default_value) const {
    size_t key_size = strlen(key);
    for (int i = 1; i < _argc; i++) {
      const char* val = _argv[i];
      size_t val_len = strlen(val);
      if (val_len >= key_size && memcmp(val, key, key_size) == 0) {
        if (val_len > key_size && val[key_size] == '=') {
          return val + key_size + 1u;
        }
        else if (val_len == key_size && i + 1 < _argc) {
          return _argv[i + 1];
        }
      }
    }

    return default_value;
  }

  int value_as_int(const char* key, int default_value) const {
    const char* val = get_arg(key, nullptr);
    if (val == nullptr || val[0] == '\0')
      return default_value;

    return atoi(val);
  }

  unsigned value_as_uint(const char* key, unsigned default_value) const {
    const char* val = get_arg(key, nullptr);
    if (val == nullptr || val[0] == '\0')
      return default_value;

    int v = atoi(val);
    if (v < 0)
      return default_value;
    else
      return unsigned(v);
  }
};

} // {ZeroLib}

template<typename T>
struct ZeroFormatter {
  static constexpr bool kExists = std::is_integral_v<T> ||
                                  std::is_floating_point_v<T> ||
                                  std::is_enum_v<T> ||
                                  std::is_pointer_v<T>;

  static inline void stringify(ZeroLib::String& buf, const T& value) noexcept
    requires (std::is_integral_v<T> || std::is_floating_point_v<T> || std::is_enum_v<T> || std::is_pointer_v<T>)
  {
    if constexpr (std::is_integral_v<T> || std::is_floating_point_v<T>) {
      buf.append_value(value);
    }
    else if constexpr (std::is_enum_v<T>) {
      buf.append_value(std::underlying_type_t<T>(value));
    }
    else if (std::is_pointer_v<T>) {
      buf.append_format(sizeof(void*) == 4 ? "0x%08X" : "0x%016X", reinterpret_cast<const void*>(value));
    }
  }
};

#endif // ZERO_LIB_H_INCLUDED
