// ZeroTest - Lightweight and embeddable unit testing framework for C++
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

#if defined(_WIN32) && !defined(_CRT_SECURE_NO_WARNINGS)
  // Otherwise we would get a warning regarding `getenv()` use.
  #define _CRT_SECURE_NO_WARNINGS
#endif

#include "zero_lib.h"

#include <stdlib.h>

#if defined(_WIN32)
  #include <windows.h>
#else
  #include <unistd.h>
#endif

namespace ZeroLib {

// Color Support
// =============

//! A color escape sequence that ZeroString supports.
struct ColorEscape {
  char start[9];
  unsigned char start_len;
  char end[5];
  unsigned char end_len;
};

// Table of color escape sequences.
static const ColorEscape color_table[] = {
  {""           , 0, ""       , 0}, // [00] kNone
  {"\x1B[30m"   , 5, "\x1B[0m", 4}, // [01] kBlack
  {"\x1B[31m"   , 5, "\x1B[0m", 4}, // [02] kReg
  {"\x1B[32m"   , 5, "\x1B[0m", 4}, // [03] kGreen
  {"\x1B[33m"   , 5, "\x1B[0m", 4}, // [04] kYellow
  {"\x1B[34m"   , 5, "\x1B[0m", 4}, // [05] kBlue
  {"\x1B[35m"   , 5, "\x1B[0m", 4}, // [06] kMagenta
  {"\x1B[36m"   , 5, "\x1B[0m", 4}, // [07] kCyan
  {"\x1B[37m"   , 5, "\x1B[0m", 4}, // [08] kWhite
  {"\x1B[90m"   , 5, "\x1B[0m", 4}, // [09] kGray
  {"\x1B[91m"   , 5, "\x1B[0m", 4}, // [10] kBrightRed
  {"\x1B[92m"   , 5, "\x1B[0m", 4}, // [11] kBrightGreen
  {"\x1B[93m"   , 5, "\x1B[0m", 4}, // [12] kBrightYellow
  {"\x1B[94m"   , 5, "\x1B[0m", 4}, // [13] kBrightBlue
  {"\x1B[95m"   , 5, "\x1B[0m", 4}, // [14] kBrightMagenta
  {"\x1B[96m"   , 5, "\x1B[0m", 4}, // [15] kBrightCyan
  {"\x1B[97m"   , 5, "\x1B[0m", 4}, // [16] kBrightWhite
};

static bool _color_output_enabled = false;

bool is_color_output_supported() noexcept {
  const char* no_color_env = getenv("NO_COLOR");
  if (no_color_env && no_color_env[0]) {
    return false;
  }

#ifdef _WIN32
  DWORD mode;
  HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);

  if (h == nullptr || h == INVALID_HANDLE_VALUE) {
    return false;
  }

  return GetConsoleMode(h, &mode) != 0;
#else
  return isatty(fileno(stdout)) != 0;
#endif
}

void set_color_output(bool value) noexcept {
#ifdef _WIN32
  if (value) {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    if (h == INVALID_HANDLE_VALUE || h == nullptr) {
      value = false;
    }
    else {
      DWORD mode;
      if (!GetConsoleMode(h, &mode)) {
        value = false;
      }
      else {
        value = SetConsoleMode(h, mode | ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
      }
    }
  }

#endif

  _color_output_enabled = value;
}

// String Utilities
// ================

static size_t str_find(const char* s, size_t len, char c) noexcept {
  for (size_t i = 0; i < len; i++) {
    if (s[i] == c) {
      return i;
    }
  }

  return len;
}

// String Container
// ================

void String::clear() noexcept {
  if (_size != 0) {
    _size = 0u;
    _failed = false;
  }
}

void String::reset() noexcept {
  _size = 0u;
  _failed = false;

  if (_data && !_temporary) {
    free(_data);
    _data = nullptr;
    _capacity = 0u;
  }
}

void String::assign(const char* s) noexcept {
  assign(s, strlen(s));
}

void String::assign(const char* s, size_t size) noexcept {
  clear();
  append(s, size);
}

bool String::reserve_for_append(size_t append_size) noexcept {
  if (_capacity - _size >= append_size) {
    return true;
  }

  size_t result_size = _size + append_size;
  size_t new_capacity = (_size | 128) * 2u;

  if (new_capacity < result_size) {
    new_capacity = result_size + 128;
  }

  new_capacity = (new_capacity + 63u) & ~size_t(63);
  char* new_data = static_cast<char*>(malloc(new_capacity));

  if (new_data == nullptr) {
    _failed = true;
    return false;
  }

  memcpy(new_data, _data, _size);
  if (_data && !_temporary) {
    free(_data);
  }

  _data = new_data;
  _capacity = new_capacity;
  _temporary = false;

  return true;
}

void String::append(const char c) noexcept {
  if (reserve_for_append(1u)) {
    _data[_size++] = c;
  }
}

void String::append_chars(const char c, size_t count) noexcept {
  if (reserve_for_append(count)) {
    memset(_data + _size, int(uint8_t(c)), count);
    _size += count;
  }
}

void String::append(const char* s) noexcept {
  return append(s, strlen(s));
}

void String::append(const char* s, size_t size) noexcept {
  if (reserve_for_append(size)) {
    memcpy(_data + _size, s, size);
    _size += size;
  }
}

void String::append_padded(const char* s, size_t size, size_t left_padding, size_t right_padding) noexcept {
  if (left_padding) {
    append_chars(' ', left_padding);
  }

  append(s, size);

  if (right_padding) {
    append_chars(' ', right_padding);
  }

}

void String::append_indented(const char* s, size_t size, size_t indentation_size, bool indent_first) noexcept {
  bool indent = indent_first;

  do {
    size_t line_size = str_find(s, size, '\n');

    if (indent) {
      append_chars(' ', indentation_size);
    }

    append(s, line_size);
    append('\n');

    if (line_size < size) {
      line_size++;
    }

    s += line_size;
    size -= line_size;
    indent = true;
  } while (size);
}

void String::append_format(const char* fmt, ...) noexcept {
  va_list ap;
  va_start(ap, fmt);
  append_format_v(fmt, ap);
  va_end(ap);
}

void String::append_format_v(const char* fmt, va_list ap) noexcept {
  va_list ap_copy;
  va_copy(ap_copy, ap);

  size_t remaining_capacity = _capacity - _size;
  int result = vsnprintf(_data + _size, remaining_capacity, fmt, ap);

  if (result > 0) {
    size_t formatted_size = size_t(result);

    if (formatted_size < remaining_capacity) {
      _size += formatted_size;
      return;
    }

    if (reserve_for_append(formatted_size + 1u)) {
      result = vsnprintf(_data + _size, formatted_size + 1u, fmt, ap_copy);
      if (size_t(result) <= formatted_size) {
        _size += formatted_size;
        return;
      }
    }
  }

  _failed = true;
}

void String::append_start_escape(Color color) noexcept {
  if (_color_output_enabled) {
    const ColorEscape& escape = color_table[static_cast<size_t>(color)];
    append(escape.start, escape.start_len);
  }
}

void String::append_end_escape(Color color) noexcept {
  if (_color_output_enabled) {
    const ColorEscape& escape = color_table[static_cast<size_t>(color)];
    append(escape.end, escape.end_len);
  }
}

bool String::eat_nl() noexcept {
  if (_size && _data[_size - 1u] == '\n') {
    _size--;
    return true;
  }
  else {
    return false;
  }
}

void String::c_append(Color color, const char* s) noexcept {
  c_append(color, s, strlen(s));
}

void String::c_append(Color color, const char* s, size_t len) noexcept {
  append_start_escape(color);
  append(s, len);
  bool nl = eat_nl();
  append_end_escape(color);
  if (nl) {
    append('\n');
  }
}

void String::c_append_padded(Color color, const char* s, size_t size, size_t left_padding, size_t right_padding) noexcept {
  if (left_padding) {
    append_chars(' ', left_padding);
  }

  c_append(color, s, size);

  if (right_padding) {
    append_chars(' ', right_padding);
  }
}

void String::c_append_format_v(Color color, const char* fmt, va_list ap) noexcept {
  append_start_escape(color);
  append_format_v(fmt, ap);

  bool nl = eat_nl();
  append_end_escape(color);
  if (nl) {
    append('\n');
  }
}

void String::c_append_format_s(Color color, const char* fmt, ...) noexcept {
  append_start_escape(color);

  va_list ap;
  va_start(ap, fmt);
  append_format_v(fmt, ap);
  va_end(ap);

  bool nl = eat_nl();
  append_end_escape(color);
  if (nl) {
    append('\n');
  }
}

// StringUtils
// ===========

namespace StringUtils {

//! Fuzzy equality - case-insensitive and dashes/underscores are considered equal.
[[nodiscard]]
static bool fuzzy_eq(const char* a, const char* b, size_t len) {
  using U = unsigned int;

  for (size_t i = 0; i != len; i++) {
    U ca = (unsigned char)a[i];
    U cb = (unsigned char)b[i];

    if (ca == U('-')) ca = U('_');
    if (cb == U('-')) cb = U('_');

    if (ca >= U('A') && ca <= U('Z')) ca += U('a' - 'A');
    if (cb >= U('A') && cb <= U('Z')) cb += U('a' - 'A');

    if (ca != cb) {
      return false;
    }
  }

  return true;
}

// Get whether the strings `a` and `b` are equal, ignoring case and treating `-` as `_`.
bool match_case(const char* a, size_t a_len, const char* b, size_t b_len) noexcept {
  bool prefix_star = b_len > 0u && b[0] == '*';
  bool suffix_star = b_len > 0u && b[b_len - 1] == '*';

  b += size_t(prefix_star);
  b_len = b_len - size_t(prefix_star) - size_t(suffix_star);

  if (a_len < b_len) {
    return false;
  }

  // Substring match - *str*.
  if (prefix_star && suffix_star) {
    size_t n = a_len - b_len;
    for (size_t i = 0; i < n; i++) {
      if (fuzzy_eq(a + i, b, b_len)) {
        return true;
      }
    }
    return false;
  }

  // Suffix match - *str.
  if (prefix_star) {
    return a_len >= b_len && fuzzy_eq(a + a_len - b_len, b, b_len);
  }

  // Prefix match - str*.
  if (suffix_star) {
    return a_len >= b_len && fuzzy_eq(a, b, b_len);
  }

  // Exact match
  return a_len == b_len && fuzzy_eq(a, b, b_len);
}

} // {StringUtils}

} // {ZeroLib}
