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

// Zero test is a minimalist unit testing framework with the following goals:
//
//   - Zero dependencies, easy to embed in most projects.
//   - Zero memory allocations by the framework itself (unless user prints
//     gigantic messages, which would need buffering).
//   - Minimalist interface and features.

#ifndef ZERO_TEST_H_INCLUDED
#define ZERO_TEST_H_INCLUDED

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <type_traits>

// Hide everything when using Doxygen. Ideally this can be protected by a macro,
// but there is not globally and widely used one across multiple projects.

//! \cond ZERO_TEST_DOC


#if defined(__clang__) && !defined(_DOXYGEN)
  #define ZERO_TEST_INLINE_NODEBUG inline __attribute__((__always_inline__, __nodebug__))
#elif defined(__GNUC__) && !defined(_DOXYGEN)
  #define ZERO_TEST_INLINE_NODEBUG inline __attribute__((__always_inline__, __artificial__))
#else
  #define ZERO_TEST_INLINE_NODEBUG inline
#endif

namespace ZeroTest {

struct CheckResult;
struct TestCase;
class TestRunner;

//! Register a new unit test (called automatically by `TestCaseStatic` and `TEST_CASE`).
void register_test(TestCase* test) noexcept;

//! Returns an active test runner.
TestRunner* test_runner() noexcept;

//! Log message, adds automatically new line if not present.
void log_info(const char* fmt, ...) noexcept;

//! Called when `EXPECT()` fails.
[[noreturn]] void test_failure(CheckResult* result, const char* fmt, ...) noexcept;

template<typename T> [[nodiscard]]
static constexpr ZERO_TEST_INLINE_NODEBUG T&& forward(std::remove_reference_t<T>& v) noexcept { return static_cast<T&&>(v); }

template<typename T> [[nodiscard]]
static constexpr ZERO_TEST_INLINE_NODEBUG T&& forward(std::remove_reference_t<T>&& v) noexcept { return static_cast<T&&>(v); }

enum class MessageType : unsigned {
  kRegularText = 0,
  kStatus = 1,
  kBright = 2,
  kRunnerMessage = 3,
  kTestCase = 4,
  kTestVariant = 5,
  kTestLog = 6,
  kImportant = 7,
  kSuccess = 8,
  kFailure = 9,

  kMaxValue = 9
};

struct ColorEscape {
  char start[9];
  unsigned char start_len;
  char end[5];
  unsigned char end_len;
};

// String buffer is a non-null terminated string designed for concatenation.
class StringBuffer {
public:
  char* _data {};
  size_t _size {};
  size_t _capacity {};
  bool _failed = false;
  bool _temporary = false;

  ZERO_TEST_INLINE_NODEBUG StringBuffer() noexcept {}
  ZERO_TEST_INLINE_NODEBUG ~StringBuffer() noexcept { reset(); }

  ZERO_TEST_INLINE_NODEBUG char* data() noexcept { return _data; }
  ZERO_TEST_INLINE_NODEBUG const char* data() const noexcept { return _data; }

  ZERO_TEST_INLINE_NODEBUG size_t size() const noexcept { return _size; }
  ZERO_TEST_INLINE_NODEBUG size_t capacity() const noexcept { return _capacity; }

  ZERO_TEST_INLINE_NODEBUG bool failed() const noexcept { return _failed; }

  void clear() noexcept;
  void reset() noexcept;
  bool reserve_for_append(size_t append_size) noexcept;

  void append(const char c) noexcept;
  void append_chars(const char c, size_t count) noexcept;
  void append(const char* s) noexcept;

  void append_data(const char* s, size_t size) noexcept;
  void append_data_indented(const char* s, size_t size, size_t indentation_size, bool indent_first) noexcept;

  void append_format(const char* fmt, ...) noexcept;
  void append_format_v(const char* fmt, va_list ap) noexcept;

  void append_start_escape(const ColorEscape& esc) noexcept;
  void append_end_escape(const ColorEscape& esc) noexcept;

  void append_colorized_s(const ColorEscape& esc, const char* s, size_t len) noexcept;
  void append_colorized_v(const ColorEscape& esc, const char* fmt, va_list ap) noexcept;
  void append_colorized_f(const ColorEscape& esc, const char* fmt, ...) noexcept;

  template<typename... Args>
  inline void append_colorized(const ColorEscape& esc, const char* msg, Args&&... args) noexcept {
    if constexpr (sizeof...(args) == 0) {
      append_colorized_s(esc, msg, strlen(msg));
    }
    else {
      append_colorized_f(esc, msg, forward<Args>(args)...);
    }
  }

  ZERO_TEST_INLINE_NODEBUG void append_value(const bool& value) noexcept { append(value ? "true" : "false"); }
  ZERO_TEST_INLINE_NODEBUG void append_value(const char& value) noexcept { append_format("%u", uint8_t(value)) ; }
  ZERO_TEST_INLINE_NODEBUG void append_value(const signed char& value) noexcept { append_format("%d", int(value)) ; }
  ZERO_TEST_INLINE_NODEBUG void append_value(const unsigned char& value) noexcept { append_format("%u", unsigned(value)); }
  ZERO_TEST_INLINE_NODEBUG void append_value(const short& value) noexcept { append_format("%d", int(value)); }
  ZERO_TEST_INLINE_NODEBUG void append_value(const unsigned short& value) noexcept { append_format("%u", unsigned(value)); }
  ZERO_TEST_INLINE_NODEBUG void append_value(const int& value) noexcept { append_format("%d", value); }
  ZERO_TEST_INLINE_NODEBUG void append_value(const unsigned int& value) noexcept { append_format("%u", value); }
  ZERO_TEST_INLINE_NODEBUG void append_value(const long& value) noexcept { append_format("%ld", value); }
  ZERO_TEST_INLINE_NODEBUG void append_value(const unsigned long& value) noexcept { append_format("%lu", value); }
  ZERO_TEST_INLINE_NODEBUG void append_value(const long long& value) noexcept { append_format("%lld", value); }
  ZERO_TEST_INLINE_NODEBUG void append_value(const unsigned long long& value) noexcept { append_format("%llu", value); }
  ZERO_TEST_INLINE_NODEBUG void append_value(const float& value) noexcept { append_format("%g", double(value)); }
  ZERO_TEST_INLINE_NODEBUG void append_value(const double& value) noexcept { append_format("%g", double(value)); }

  bool eat_nl() noexcept;
};

template<size_t N>
class StringBufferTmp : public StringBuffer {
public:
  char _embedded_data[N];

  ZERO_TEST_INLINE_NODEBUG StringBufferTmp() noexcept {
    _data = _embedded_data;
    _capacity = N;
    _temporary = true;
  }

  ZERO_TEST_INLINE_NODEBUG void reset() noexcept {
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

template<typename T>
struct Formatter {
  static constexpr bool kExists = std::is_integral_v<T> ||
                                  std::is_floating_point_v<T> ||
                                  std::is_enum_v<T> ||
                                  std::is_pointer_v<T>;

  static inline void stringify(StringBuffer& buf, const T& value) requires (std::is_integral_v<T> || std::is_floating_point_v<T> || std::is_enum_v<T> || std::is_pointer_v<T>) {
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

template<typename LHS, typename RHS>
static bool format_binary_expression(StringBuffer& buf, const char* op, const LHS& lhs, const RHS& rhs) {
  if constexpr (Formatter<LHS>::kExists && Formatter<RHS>::kExists) {
    Formatter<LHS>::stringify(buf, lhs);
    buf.append(op);
    Formatter<RHS>::stringify(buf, rhs);
    return true;
  }
  else {
    return false;
  }
}

//! TestCase defined by `TEST_CASE()` macro.
struct TestCase {
  //! Entry point of a unit test defined by `TEST_CASE()` macro.
  using EntryFunc = void (*)(TestRunner& test_runner);

  //! Test case flags.
  enum class Flags : unsigned {
    kNone     = 0x00u,
    kFinished = 0x01u
  };

  EntryFunc test_func;
  const char* test_name;
  const char* test_file;
  Flags test_flags;
  unsigned test_precedence;
  unsigned test_line;
  unsigned matched_filter_size;
  TestCase* next_test;
};

//! Carries a single type into a generic lambda (so a type pack can be expanded into calls).
template<typename T>
struct TypeTag { using type = T; };

//! Non-owning string view (avoids a `<string_view>` dependency), used to pass type-names around.
struct TestTypeName {
  const char* data;
  size_t size;
};

//! Automatic test registration by using static initialization.
class TestCaseStatic : public TestCase {
public:
  inline TestCaseStatic(EntryFunc func, const char* name, const char* file, unsigned line) noexcept
    : TestCase{
      .test_func = func,
      .test_name = name,
      .test_file = file,
      .test_flags = Flags::kNone,
      .test_precedence = 0,
      .test_line = line,
      .matched_filter_size = 0,
      .next_test = nullptr
    }
  {
    register_test(this);
  }
};

//! TestPrecedence record in a test_precedence_table, which can be used by `TestRunner` to run tests
//! according to a user precedence rules (specified via filters).
struct TestPrecedence {
  const char* filter;
};

//! Test runner is responsible for running all tests (or tests filtered by --run).
class TestRunner {
public:
  using HandlerFn = void(*)(TestRunner& test_runner);
  using MessageOutFn = void(*)(TestRunner& test_runner, MessageType msg_type, const char* msg, size_t size);

  template<typename F>
  struct UserFunc { F func; void* data; };

  // Command line arguments.
  int _argc {};
  const char* const* _argv {};

  bool _verbose = false;
  bool _color_output = false;
  unsigned _status = 0;
  unsigned _test_precedence_max = 0;
  const char* _run_filter {};

  UserFunc<MessageOutFn> _message_out {};
  UserFunc<HandlerFn> _run_before {};
  UserFunc<HandlerFn> _run_after {};
  UserFunc<HandlerFn> _options_list {};
  StringBufferTmp<512> _log_pending;

  TestCase* _test_running {};

  char _symbol_success[8] {};
  char _symbol_failure[8] {};
  ColorEscape _color_escapes[size_t(MessageType::kMaxValue) + 1u] {};

  TestRunner(int argc, const char* const* argv);
  ~TestRunner() noexcept;

  void apply_precedence(const TestPrecedence* entries, size_t count);

  template<size_t N>
  inline void apply_precedence(const TestPrecedence(&entries)[N]) { apply_precedence(entries, N); }

  bool has_arg(const char* key) const noexcept;
  const char* get_arg(const char* key, const char* default_value = nullptr) const noexcept;

  inline const ColorEscape& color_escape_of(MessageType message_type) const noexcept {
    return _color_escapes[size_t(message_type)];
  }

  template<typename LambdaFn>
  inline void on_message(const LambdaFn& lambda_fn) {
    _message_out.data = (void*)(&lambda_fn);
    _message_out.func = [](TestRunner& test_runner, MessageType msg_type, const char* msg, size_t size) {
      LambdaFn& lambda_fn = *static_cast<LambdaFn*>(test_runner._message_out.data);
      lambda_fn(msg_type, msg, size);
    };
  }

  template<typename LambdaFn>
  inline void on_before_run(const LambdaFn& lambda_fn) {
    _run_before.data = (void*)(&lambda_fn);
    _run_before.func = [](TestRunner& test_runner) {
      const LambdaFn& lambda_fn = *static_cast<const LambdaFn*>(test_runner._run_before.data);
      lambda_fn();
    };
  }

  template<typename LambdaFn>
  inline void on_after_run(const LambdaFn& lambda_fn) {
    _run_after.data = (void*)(&lambda_fn);
    _run_after.func = [](TestRunner& test_runner) {
      const LambdaFn& lambda_fn = *static_cast<const LambdaFn*>(test_runner._run_after.data);
      lambda_fn();
    };
  }

  template<typename LambdaFn>
  inline void on_options_list(const LambdaFn& lambda_fn) {
    _options_list.data = (void*)(&lambda_fn);
    _options_list.func = [](TestRunner& test_runner) {
      const LambdaFn& lambda_fn = *static_cast<const LambdaFn*>(test_runner._options_list.data);
      lambda_fn();
    };
  }

  void message_f(MessageType msg_type, const char* fmt, ...);
  void message_s(MessageType msg_type, const char* msg);
  void message_s(MessageType msg_type, const char* msg, size_t msg_size);

  inline void message(MessageType msg_type, const StringBuffer& sb) { message_s(msg_type, sb.data(), sb.size()); }

  template<typename... Args>
  inline void message(MessageType msg_type, const char* fmt, Args&&... args) noexcept {
    if constexpr (sizeof...(args) == 0) {
      message_s(msg_type, fmt);
    }
    else {
      message_f(msg_type, fmt, forward<Args>(args)...);
    }
  }

  void print_help();
  void set_color_output(bool enabled);

  void list_tests();
  void list_filter_options();

  int run_tests();
  bool can_run(const TestCase* test) const;
  void run_test(TestCase* test);

  //! Lets the test runner know that the current test runs a variation test-case named `variation_name`.
  //!
  //! Any test case can have multiple variations.
  //!
  //! Returns when the test-case should run. Please note that honoring the return value is important as the
  //! runner may only want to run a single test-case variation of a whole TEST_CASE() if user specified it
  //! via command line.
  bool test_variation(const char* variation_name, size_t len = SIZE_MAX) noexcept;
};

//! Returns the `index`-th comma-separated token of a stringified argument list (e.g. the string
//! produced by `#__VA_ARGS__`), trimmed of surrounding whitespace.
//!
//! Commas nested inside `<>`, `()`, `[]` or `{}` are ignored, so a template type such as
//! `std::pair<int, float>` is returned as a single type-name.
static constexpr TestTypeName type_name_at(const char* names, size_t index) noexcept {
  size_t i = 0;
  size_t depth = 0;
  size_t current = 0;
  size_t token_start = 0;

  for (;;) {
    char c = names[i];
    if (c == '<' || c == '(' || c == '[' || c == '{') {
      depth++;
    }
    else if (depth && (c == '>' || c == ')' || c == ']' || c == '}')) {
      depth--;
    }

    if (c == '\0' || (c == ',' && depth == 0)) {
      if (current == index) {
        size_t s = token_start;
        size_t e = i;
        while (s < e && (names[s] == ' ' || names[s] == '\t')) {
          s++;
        }
        while (e > s && (names[e - 1] == ' ' || names[e - 1] == '\t')) {
          e--;
        }
        return {names + s, e - s};
      }

      current++;
      token_start = i + 1;
    }

    i++;
    if (c == '\0') {
      break;
    }
  }

  return {nullptr, 0};
}

//! Compile-time list of types that also carries the source spelling of each type.
//!
//! `names` is the stringified argument list (`#__VA_ARGS__`) as produced by `TEST_DEFINE_TYPES()`,
//! e.g. `"int8_t, int16_t, int32_t"`. The name at position `i` corresponds to the `i`-th type.
template<typename... Types>
struct TestTypes {
  const char* names;
  static constexpr size_t size() noexcept { return sizeof...(Types); }
};

//! Invokes `fn(TypeTag<T>{}, name)` for each `T` in the list (left-to-right, sequenced), taking
//! `name` from the source spelling captured by `TEST_DEFINE_TYPES()`.
template<typename Fn, typename... Types>
static inline void for_each_type(const TestTypes<Types...>& list, Fn&& fn) {
  size_t index = 0;
  (fn(TypeTag<Types>{}, type_name_at(list.names, index++)), ...);
}

static inline uint32_t float_as_int(float x) noexcept { uint32_t out; memcpy(&out, &x, 4); return out; }
static inline uint64_t float_as_int(double x) noexcept { uint64_t out; memcpy(&out, &x, 8); return out; }

template<typename T>
static inline auto ulp_diff_t(T lhs, T rhs) noexcept {
  uint32_t kBitSize = sizeof(T) * 8;
  uint32_t kSignPos = kBitSize - 1;

  auto lhs_u = float_as_int(lhs);
  auto rhs_u = float_as_int(rhs);

  auto lhs_abs = lhs_u ^ ((lhs_u >> kSignPos) << kSignPos);
  auto rhs_abs = rhs_u ^ ((rhs_u >> kSignPos) << kSignPos);

  auto max_abs = lhs_abs > rhs_abs ? lhs_abs : rhs_abs;
  auto min_abs = lhs_abs < rhs_abs ? lhs_abs : rhs_abs;

  bool sign_diff = ((lhs_u ^ rhs_u) >> kSignPos) != 0u;
  return (sign_diff) ? lhs_abs + rhs_abs : max_abs - min_abs;
}

static inline uint32_t ulp_diff(float lhs, float rhs) noexcept { return ulp_diff_t<float>(lhs, rhs); }
static inline uint64_t ulp_diff(double lhs, double rhs) noexcept { return ulp_diff_t<double>(lhs, rhs); }

//! Expected result from EXPECT_XXX checks.
struct CheckResult {
  const char* _file {};
  const char* _expression {};
  int _line = 0;
  bool _ok = true;
  bool _done = false;
  bool _handled = false;
  StringBufferTmp<64> _buf;

  explicit ZERO_TEST_INLINE_NODEBUG CheckResult(const char* file, int line, const char* expression, bool ok) noexcept
    : _file(file),
      _expression(expression),
      _line(line),
      _ok(ok) {};

  ZERO_TEST_INLINE_NODEBUG ~CheckResult() noexcept {
    if (!_ok && !_handled) {
      test_failure(this, nullptr);
    }
  }

  ZERO_TEST_INLINE_NODEBUG CheckResult& self() noexcept { return *this; }

  ZERO_TEST_INLINE_NODEBUG bool proceed() const noexcept { return !_ok && !_done; }
  ZERO_TEST_INLINE_NODEBUG void make_done() noexcept { _done = true; }

  template<typename... Args>
  inline void message(const char* fmt, Args&&... args) noexcept {
    test_failure(this, fmt, forward<Args>(args)...);
  }
};

template<typename T>
static CheckResult check(const char* file, int line, const char* exp, T&& result) noexcept {
  bool ok = !!result;
  CheckResult out(file, line, exp, ok);
  if (!ok) { out._buf.append_value((!!result)); }
  return out;
}

template<typename T>
static CheckResult expect_true(const char* file, int line, const char* exp, T&& result) noexcept {
  bool ok = result == true;
  CheckResult out(file, line, exp, ok);
  if (!ok) { out._buf.append_value(static_cast<bool>(result)); }
  return out;
}

template<typename T>
static CheckResult expect_false(const char* file, int line, const char* exp, T&& result) noexcept {
  bool ok = result == false;
  CheckResult out(file, line, exp, ok);
  if (!ok) { out._buf.append_value(static_cast<bool>(result)); }
  return out;
}

template<typename T>
static CheckResult expect_null(const char* file, int line, const char* exp, T* result) noexcept requires (std::is_pointer_v<T*>) {
  bool ok = result == nullptr;
  CheckResult out(file, line, exp, ok);
  if (!ok) { Formatter<T*>::stringify(out._buf, result); }
  return out;
}

template<typename T>
static CheckResult expect_not_null(const char* file, int line, const char* exp, T* result) noexcept requires (std::is_pointer_v<T*>) {
  bool ok = result != nullptr;
  CheckResult out(file, line, exp, ok);
  if (!ok) { Formatter<T*>::stringify(out._buf, result); }
  return out;
}

template<typename LHS, typename RHS>
static CheckResult expect_eq(const char* file, int line, const char* exp, LHS&& lhs, RHS&& rhs) noexcept {
  bool ok = lhs == rhs;
  CheckResult out(file, line, exp, ok);
  if (!ok) { format_binary_expression<std::decay_t<LHS>, std::decay_t<RHS>>(out._buf, " == ", lhs, rhs); }
  return out;
}

template<typename LHS, typename RHS>
static CheckResult expect_ne(const char* file, int line, const char* exp, LHS&& lhs, RHS&& rhs) noexcept {
  bool ok = lhs != rhs;
  CheckResult out(file, line, exp, ok);
  if (!ok) { format_binary_expression<std::decay_t<LHS>, std::decay_t<RHS>>(out._buf, " != ", lhs, rhs); }
  return out;
}

template<typename LHS, typename RHS>
static CheckResult expect_gt(const char* file, int line, const char* exp, LHS&& lhs, RHS&& rhs) noexcept {
  bool ok = lhs > rhs;
  CheckResult out(file, line, exp, ok);
  if (!ok) { format_binary_expression<std::decay_t<LHS>, std::decay_t<RHS>>(out._buf, " > ", lhs, rhs); }
  return out;
}

template<typename LHS, typename RHS>
static CheckResult expect_ge(const char* file, int line, const char* exp, LHS&& lhs, RHS&& rhs) noexcept {
  bool ok = lhs >= rhs;
  CheckResult out(file, line, exp, ok);
  if (!ok) { format_binary_expression<std::decay_t<LHS>, std::decay_t<RHS>>(out._buf, " >= ", lhs, rhs); }
  return out;
}

template<typename LHS, typename RHS>
static CheckResult expect_lt(const char* file, int line, const char* exp, LHS&& lhs, RHS&& rhs) noexcept {
  bool ok = lhs < rhs;
  CheckResult out(file, line, exp, ok);
  if (!ok) { format_binary_expression<std::decay_t<LHS>, std::decay_t<RHS>>(out._buf, " < ", lhs, rhs); }
  return out;
}

template<typename LHS, typename RHS>
static CheckResult expect_le(const char* file, int line, const char* exp, LHS&& lhs, RHS&& rhs) noexcept {
  bool ok = lhs <= rhs;
  CheckResult out(file, line, exp, ok);
  if (!ok) { format_binary_expression<std::decay_t<LHS>, std::decay_t<RHS>>(out._buf, " <= ", lhs, rhs); }
  return out;
}

template<typename LHS, typename RHS>
static CheckResult expect_near_ulp(const char* file, int line, const char* exp, LHS&& lhs, RHS&& rhs, uint64_t max_ulp_diff) noexcept {
  auto actual_ulp_diff = ulp_diff(lhs, rhs);
  bool ok = actual_ulp_diff <= max_ulp_diff;
  CheckResult out(file, line, exp, ok);

  if (!ok) {
    format_binary_expression<std::decay_t<LHS>, std::decay_t<RHS>>(out._buf, " ~= ", lhs, rhs);
    out._buf.append_format(" (ulp_diff=%llu, max_ulp_diff=%llu)",
      (unsigned long long)actual_ulp_diff,
      (unsigned long long)max_ulp_diff);
  }

  return out;
}

template<typename LHS, typename RHS, typename E>
static CheckResult expect_near_eps(const char* file, int line, const char* exp, LHS&& lhs, RHS&& rhs, const E& epsilon) noexcept {
  double diff = double(lhs) - double(rhs);
  if (diff < 0.0) {
    diff = -diff;
  }

  bool ok = diff <= double(epsilon);
  CheckResult out(file, line, exp, ok);

  if (!ok) {
    format_binary_expression<std::decay_t<LHS>, std::decay_t<RHS>>(out._buf, " ~= ", lhs, rhs);
    out._buf.append_format(" (diff=%g, epsilon=%g)", diff, double(epsilon));
  }

  return out;
}

} // {ZeroTest}

//! \def TEST_DEFINE_TYPES(NAME, ...)
//!
//! Define a reusable `ZeroTest::TestTypes` named `NAME` holding the given types.
//!
//! The names of the types are captured from the source spelling (`#__VA_ARGS__`), so typedef names
//! such as `int32_t` are preserved instead of being resolved to their underlying type. The resulting
//! variable can be passed to `TEST_CASE_T()`, and reused by multiple tests:
//!
//! ```
//! TEST_DEFINE_TYPES(int_types, int8_t, int16_t, int32_t, int64_t);
//! TEST_CASE_T(my_test, int_types) { /* `T` is int8_t, int16_t, ... */ }
//! ```
#define TEST_DEFINE_TYPES(NAME, ...) \
  static constexpr ::ZeroTest::TestTypes<__VA_ARGS__> NAME { #__VA_ARGS__ }

//! Internal macro used by `TEST_CASE()`.
#define TEST_CASE_INTERNAL(NAME, FILE, LINE)                                                        \
  static void NAME##_func(::ZeroTest::TestRunner& test_runner);                                     \
  static ::ZeroTest::TestCaseStatic NAME##_test_case(NAME##_func, #NAME, FILE, unsigned(LINE));     \
  static void NAME##_func([[maybe_unused]] ::ZeroTest::TestRunner& test_runner)

//! Internal macro used by `TEST_CASE_T()`.
//!
//! The type list is passed via `__VA_ARGS__` (as the trailing argument) so that the commas that
//! separate the types inside `ZeroTest::types<...>` are not mistaken for macro argument separators.
#define TEST_CASE_INTERNAL_T(NAME, FILE, LINE, ...)                                                 \
  template<typename T>                                                                              \
  static void NAME##_func(::ZeroTest::TestRunner& test_runner);                                     \
                                                                                                    \
  static void NAME##_dispatch(::ZeroTest::TestRunner& test_runner) {                                \
    ::ZeroTest::for_each_type(__VA_ARGS__, [&](auto type_tag, ::ZeroTest::TestTypeName type_name) { \
      if (test_runner.test_variation(type_name.data, type_name.size)) {                             \
        NAME##_func<typename decltype(type_tag)::type>(test_runner);                                \
      }                                                                                             \
    });                                                                                             \
  }                                                                                                 \
  static ::ZeroTest::TestCaseStatic NAME##_test_case(NAME##_dispatch, #NAME, FILE, unsigned(LINE)); \
                                                                                                    \
  template<typename T>                                                                              \
  static void NAME##_func([[maybe_unused]] ::ZeroTest::TestRunner& test_runner)

//! \def TEST_CASE(NAME)
//!
//! Define a unit test with an optional priority.
//!
//! `NAME` can only contain ASCII characters, numbers and underscore. It has the same rules as identifiers in C and C++.
#define TEST_CASE(NAME) TEST_CASE_INTERNAL(NAME, __FILE__, __LINE__)

//! \def TEST_CASE_T(NAME, TYPES)
//!
//! Define a unit test whose body is instantiated and run once per type in `TYPES`.
//!
//! `NAME` follows the same rules as `TEST_CASE()`.
//!
//! `TYPES` is a `ZeroTest::TestTypes` value (typically defined via `TEST_DEFINE_TYPES()`, which preserves
//! the source spelling of each type). Inside the body, `T` refers to the type of the current instantiation.
//! Each type is logged before it runs, so the last `[T = ...]` line printed identifies which type a failure
//! belongs to.
#define TEST_CASE_T(NAME, ...) TEST_CASE_INTERNAL_T(NAME, __FILE__, __LINE__, __VA_ARGS__)

//! #define TEST_LOG(FORMAT [, ...])
//!
//! Informative message printed to `stdout`.
#define TEST_LOG(...) ::ZeroTest::log_info(__VA_ARGS__)

#define EXPECT_CUSTOM(...) \
  for (::ZeroTest::CheckResult _zero_test_result(__VA_ARGS__); _zero_test_result.proceed(); _zero_test_result.make_done()) \
    _zero_test_result.self()

#define EXPECT(...)          EXPECT_CUSTOM(::ZeroTest::expect_true(__FILE__, __LINE__, "EXPECT(" #__VA_ARGS__ ")", !!(__VA_ARGS__)))
#define EXPECT_EQ(...)       EXPECT_CUSTOM(::ZeroTest::expect_eq(__FILE__, __LINE__, "EXPECT_EQ(" #__VA_ARGS__ ")", __VA_ARGS__))
#define EXPECT_NE(...)       EXPECT_CUSTOM(::ZeroTest::expect_ne(__FILE__, __LINE__, "EXPECT_NE(" #__VA_ARGS__ ")", __VA_ARGS__))
#define EXPECT_GT(...)       EXPECT_CUSTOM(::ZeroTest::expect_gt(__FILE__, __LINE__, "EXPECT_GT(" #__VA_ARGS__ ")", __VA_ARGS__))
#define EXPECT_GE(...)       EXPECT_CUSTOM(::ZeroTest::expect_ge(__FILE__, __LINE__, "EXPECT_GE(" #__VA_ARGS__ ")", __VA_ARGS__))
#define EXPECT_LT(...)       EXPECT_CUSTOM(::ZeroTest::expect_lt(__FILE__, __LINE__, "EXPECT_LT(" #__VA_ARGS__ ")", __VA_ARGS__))
#define EXPECT_LE(...)       EXPECT_CUSTOM(::ZeroTest::expect_le(__FILE__, __LINE__, "EXPECT_LE(" #__VA_ARGS__ ")", __VA_ARGS__))
#define EXPECT_TRUE(...)     EXPECT_CUSTOM(::ZeroTest::expect_true(__FILE__, __LINE__, "EXPECT_TRUE(" #__VA_ARGS__ ")", __VA_ARGS__))
#define EXPECT_FALSE(...)    EXPECT_CUSTOM(::ZeroTest::expect_false(__FILE__, __LINE__, "EXPECT_FALSE(" #__VA_ARGS__ ")", __VA_ARGS__))
#define EXPECT_NULL(...)     EXPECT_CUSTOM(::ZeroTest::expect_null(__FILE__, __LINE__, "EXPECT_NULL(" #__VA_ARGS__ ")", __VA_ARGS__))
#define EXPECT_NOT_NULL(...) EXPECT_CUSTOM(::ZeroTest::expect_not_null(__FILE__, __LINE__, "EXPECT_NOT_NULL(" #__VA_ARGS__ ")", __VA_ARGS__))
#define EXPECT_NEAR_EPS(...) EXPECT_CUSTOM(::ZeroTest::expect_near_eps(__FILE__, __LINE__, "EXPECT_NEAR_EPS(" #__VA_ARGS__ ")", __VA_ARGS__))
#define EXPECT_NEAR_ULP(...) EXPECT_CUSTOM(::ZeroTest::expect_near_ulp(__FILE__, __LINE__, "EXPECT_NEAR_ULP(" #__VA_ARGS__ ")", __VA_ARGS__))

#undef ZERO_TEST_INLINE_NODEBUG

//! \endcond

#endif // ZERO_TEST_H_INCLUDED
