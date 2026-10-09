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

// See `zero_test.h` for more details.

#include "zero_test.h"
#include <stdarg.h>

namespace ZeroTest {

using MT = MessageType;

// Test Runner Status
// ------------------

// Describes what is currently written into the output.
static constexpr uint32_t kStatusNone = 0u;
static constexpr uint32_t kStatusTesting = 0x01u;
static constexpr uint32_t kStatusHasVariations = 0x02u;
static constexpr uint32_t kStatusHasMessages = 0x04u;

// Escape Codes
// ------------

static const ColorEscape color_escape_table[] = {
  {""           , 0, ""       , 0}, // [0] kRegularText
  {"\x1B[93m"   , 5, "\x1B[0m", 4}, // [1] kStatus
  {"\x1B[96m"   , 5, "\x1B[0m", 4}, // [2] kBright
  {""           , 0, ""       , 0}, // [3] kRunnerMessage
  {"\x1B[96m"   , 5, "\x1B[0m", 4}, // [4] kTestCase
  {"\x1B[95m"   , 5, "\x1B[0m", 4}, // [5] kTestVariant
  {""           , 0, ""       , 0}, // [6] kTestLog
  {"\x1B[93m"   , 5, "\x1B[0m", 4}, // [7] kImportant
  {"\x1B[92m"   , 5, "\x1B[0m", 4}, // [8] kSuccess
  {"\x1B[91m"   , 5, "\x1B[0m", 4}  // [9] kFailure
};

static const char str_open_variation[] = "<";
static const char str_close_variation[] = ">";

// Globals
// -------

// Zero initialized globals.
struct Globals {
  TestRunner* test_runner;
  TestCase* test_list;
  TestCase* last_added_test;
};
static Globals globals {};

// String Utilities
// ----------------

static size_t str_find(const char* s, size_t len, char c) noexcept {
  for (size_t i = 0; i < len; i++) {
    if (s[i] == c) {
      return i;
    }
  }

  return len;
}

//! Compares names and priority of two test cases.
static int test_cmp(const TestCase* a, const TestCase* b) noexcept {
  int test_file_comparison = strcmp(a->test_file, b->test_file);

  // Tests within the same file are sorted according to their positions in source file.
  if (test_file_comparison == 0) {
    return a->test_line < b->test_line ? -1 : int(a->test_line > b->test_line);
  }
  else {
    return test_file_comparison;
  }
}

static bool test_name_eq(const char* a, const char* b, size_t len) {
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
static bool test_match(const char* a, const char* b) noexcept {
  size_t a_len = strlen(a);
  size_t b_len = strlen(b);

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
      if (test_name_eq(a + i, b, b_len)) {
        return true;
      }
    }
    return false;
  }

  // Suffix match - *str.
  if (prefix_star) {
    return a_len >= b_len && test_name_eq(a + a_len - b_len, b, b_len);
  }

  // Prefix match - str*.
  if (suffix_star) {
    return a_len >= b_len && test_name_eq(a, b, b_len);
  }

  // Exact match
  return a_len == b_len && test_name_eq(a, b, b_len);
}

// Test Registration
// -----------------

void register_test(TestCase* test) noexcept {
  TestCase** prev_ptr = &globals.test_list;
  TestCase* current = *prev_ptr;

  // C++ static initialization doesn't guarantee anything. We sort all units according to
  // `test_cmd()` so the execution will always happen in a deterministic order.
  if (globals.last_added_test && strcmp(globals.last_added_test->test_file, test->test_file) == 0) {
    test->next_test = globals.last_added_test->next_test;
    globals.last_added_test->next_test = test;
  }
  else {
    while (current != nullptr) {
      if (test_cmp(current, test) >= 0) {
        break;
      }

      prev_ptr = &current->next_test;
      current = *prev_ptr;
    }

    *prev_ptr = test;
    test->next_test = current;
  }

  globals.last_added_test = test;
}

// String Buffer
// -------------

void StringBuffer::clear() noexcept {
  _size = 0u;
  _failed = false;
}

void StringBuffer::reset() noexcept {
  _size = 0u;
  _failed = false;

  if (_data && !_temporary) {
    free(_data);
    _data = nullptr;
    _capacity = 0u;
  }
}

bool StringBuffer::reserve_for_append(size_t append_size) noexcept {
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

void StringBuffer::append(const char c) noexcept {
  if (reserve_for_append(1u)) {
    _data[_size++] = c;
  }
}

void StringBuffer::append_chars(const char c, size_t count) noexcept {
  if (reserve_for_append(count)) {
    memset(_data + _size, int(uint8_t(c)), count);
    _size += count;
  }
}

void StringBuffer::append(const char* s) noexcept {
  return append_data(s, strlen(s));
}

void StringBuffer::append_data(const char* s, size_t size) noexcept {
  if (reserve_for_append(size)) {
    memcpy(_data + _size, s, size);
    _size += size;
  }
}

void StringBuffer::append_data_indented(const char* s, size_t size, size_t indentation_size, bool indent_first) noexcept {
  bool indent = indent_first;

  do {
    size_t line_size = str_find(s, size, '\n');

    if (indent) {
      append_chars(' ', indentation_size);
    }

    append_data(s, line_size);
    append('\n');

    if (line_size < size) {
      line_size++;
    }

    s += line_size;
    size -= line_size;
    indent = true;
  } while (size);
}

void StringBuffer::append_format(const char* fmt, ...) noexcept {
  va_list ap;
  va_start(ap, fmt);
  append_format_v(fmt, ap);
  va_end(ap);
}

void StringBuffer::append_format_v(const char* fmt, va_list ap) noexcept {
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

void StringBuffer::append_start_escape(const ColorEscape& escape) noexcept {
  append_data(escape.start, escape.start_len);
}

void StringBuffer::append_end_escape(const ColorEscape& escape) noexcept {
  append_data(escape.end, escape.end_len);
}

bool StringBuffer::eat_nl() noexcept {
  if (_size && _data[_size - 1u] == '\n') {
    _size--;
    return true;
  }
  else {
    return false;
  }
}

void StringBuffer::append_colorized_s(const ColorEscape& esc, const char* s, size_t len) noexcept {
  append_start_escape(esc);
  append_data(s, len);
  bool nl = eat_nl();
  append_end_escape(esc);
  if (nl) {
    append('\n');
  }
}

void StringBuffer::append_colorized_v(const ColorEscape& esc, const char* fmt, va_list ap) noexcept {
  append_start_escape(esc);
  append_format_v(fmt, ap);

  bool nl = eat_nl();
  append_end_escape(esc);
  if (nl) {
    append('\n');
  }
}

void StringBuffer::append_colorized_f(const ColorEscape& esc, const char* fmt, ...) noexcept {
  append_start_escape(esc);

  va_list ap;
  va_start(ap, fmt);
  append_format_v(fmt, ap);
  va_end(ap);

  bool nl = eat_nl();
  append_end_escape(esc);
  if (nl) {
    append('\n');
  }
}

// Test Runner Logging
// -------------------

static void TestRunner_variation_log(TestRunner& test_runner, const char* variation_name, size_t len) {
  StringBufferTmp<1024> buf;
  auto& status = test_runner._status;

  if (!(status & kStatusHasVariations)) {
    // Opens a variation <X, Y, Z>.
    status |= kStatusHasVariations;
    buf.append(str_open_variation);
  }
  else if (!(status & kStatusHasMessages)) {
    // Variation mode without additional log messages within - continue `<X, Y, Z, ...>` pattern.
    buf.append(", ");
  }
  else {
    // Variation mode with logging - repeat the test case name with variation `test_case<variation_name`
    status &= ~kStatusHasMessages;
    buf.append_colorized(test_runner.color_escape_of(MT::kTestCase), test_runner._test_running->test_name);
    buf.append(str_open_variation);
  }

  buf.append_colorized_s(test_runner.color_escape_of(MT::kTestVariant), variation_name, len);
  test_runner.message(MT::kTestCase, buf);
}

static void TestRunner_variation_done(TestRunner& test_runner, bool message_reason) {
  auto& status = test_runner._status;

  if (!(status & kStatusHasVariations)) {
    return;
  }

  if (!(status & kStatusHasMessages)) {
    StringBufferTmp<1024> buf;
    buf.append_format(message_reason ? "*%s\n" : "%s", str_close_variation);
    test_runner.message(MT::kRegularText, buf);

    if (!message_reason) {
      status &= ~(kStatusHasMessages | kStatusHasVariations);
    }
  }
}

static void TestRunner_log_test_case(TestRunner& test_runner) {
  const auto& esc = test_runner._color_escapes[size_t(MT::kTestCase)];

  StringBufferTmp<256> buf;
  buf.append_format("- %s%s%s", esc.start, test_runner._test_running->test_name, esc.end);

  test_runner._log_pending.clear();
  test_runner._message_out.func(test_runner, MT::kTestCase, buf.data(), buf.size());
}

static void TestRunner_test_case_done(TestRunner& test_runner, bool success) {
  TestRunner_variation_done(test_runner, false);

  StringBufferTmp<1024> buf;
  auto& status = test_runner._status;

  MessageType msg_type =  success ? MT::kSuccess : MT::kFailure;
  const char* symbol = success ? test_runner._symbol_success : test_runner._symbol_failure;

  buf.append_colorized(test_runner.color_escape_of(msg_type),
    !(status & kStatusHasMessages) ? " %s\n": "  %s\n", symbol);

  test_runner.message(msg_type, buf);
}

static void TestRunner_message_indented(TestRunner& test_runner, MessageType msg_type, const char* msg, size_t msg_size) {
  size_t indentation_size = 2;

  if (test_runner._message_out.func && msg_size > 0u) {
    auto& status = test_runner._status;

    if (status & kStatusHasVariations) {
      TestRunner_variation_done(test_runner, true);
    }
    else if (!(status & kStatusHasMessages)) {
      test_runner.message(MT::kRegularText, "\n");
    }

    status |= kStatusHasMessages;

    StringBufferTmp<1024> buf;
    buf.append_data_indented(msg, msg_size, indentation_size, true);
    test_runner._message_out.func(test_runner, msg_type, buf.data(), buf.size());
  }
}

// Test Runner Failed
// ------------------

void test_failure(CheckResult* result, const char* fmt, ...) noexcept {
  result->_handled = true;
  TestRunner* test_runner = globals.test_runner;

  if (test_runner) {
    TestRunner_test_case_done(*test_runner, false);
    const auto& esc = test_runner->color_escape_of(MT::kFailure);
    const auto& imp = test_runner->color_escape_of(MT::kImportant);

    if (test_runner->_log_pending.size()) {
      TestRunner_message_indented(*test_runner, MT::kTestLog, test_runner->_log_pending.data(), test_runner->_log_pending.size());
      test_runner->_log_pending.clear();
    }

    StringBufferTmp<1024> buf;
    buf.append('\n');
    buf.append_colorized(esc, "FAILED CHECK: ");
    buf.append_colorized(imp, "%s", result->_expression);
    buf.eat_nl();
    buf.append('\n');

    if (result->_buf.size()) {
      buf.append_colorized(esc, "        WITH: ");
      buf.append_colorized(imp, result->_buf.data(), result->_buf.size());
      buf.eat_nl();
      buf.append('\n');
    }

    if (fmt) {
      va_list ap;
      va_start(ap, fmt);
      buf.append_colorized(esc, "      REASON: ");

      StringBufferTmp<512> reason_str;
      reason_str.append_format_v(fmt, ap);

      buf.append_start_escape(imp);
      buf.append_data_indented(reason_str.data(), reason_str.size(), 14, false);
      buf.eat_nl();
      buf.append_end_escape(imp);
      buf.append('\n');
      va_end(ap);
    }

    buf.append_colorized(esc, "SRC LOCATION: ");
    buf.append_colorized(imp, "%s (Line: %d)\n", result->_file, result->_line);

    test_runner->message(MT::kFailure, buf);
  }

  abort();
}

// Test Runner Public
// ------------------

TestRunner::TestRunner(int argc, const char* const* argv) : _argc(argc), _argv(argv) {
  globals.test_runner = this;

  _verbose = has_arg("--verbose");
  _run_filter = get_arg("--run", nullptr);

  bool colors_enabled = !has_arg("--no-color");

  // Doesn't work on Windows.
#if defined(_WIN32)
  colors_enabled = false;
#endif

  set_color_output(colors_enabled);
}

TestRunner::~TestRunner() noexcept {
  if (globals.test_runner == this) {
    globals.test_runner = nullptr;
  }
}

bool TestRunner::has_arg(const char* key) const noexcept {
  for (int i = 1; i < _argc; i++) {
    if (strcmp(_argv[i], key) == 0) {
      return true;
    }
  }
  return false;
}

const char* TestRunner::get_arg(const char* key, const char* default_value) const noexcept {
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

void TestRunner::apply_precedence(const TestPrecedence* entries, size_t count) {
  _test_precedence_max = unsigned(count);

  for (size_t i = 0; i < count; i++) {
    const TestPrecedence& entry = entries[i];
    unsigned filter_size = unsigned(strlen(entry.filter));

    for (TestCase* test = globals.test_list; test != nullptr; test = test->next_test) {
      if (i == 0) {
        // Tests not matching any precedence rules are executed last.
        test->test_precedence = _test_precedence_max;
      }

      bool matched = test_match(test->test_name, entry.filter);
      if (matched) {
        if (filter_size > test->matched_filter_size) {
          test->test_precedence = unsigned(i);
          test->matched_filter_size = filter_size;
        }
      }
    }
  }
}

void TestRunner::message_f(MessageType msg_type, const char* fmt, ...) {
  if (_message_out.func) {
    StringBufferTmp<1024> buf;
    va_list ap;
    va_start(ap, fmt);
    buf.append_format_v(fmt, ap);
    va_end(ap);
    _message_out.func(*this, msg_type, buf.data(), buf.size());
  }
}

void TestRunner::message_s(MessageType msg_type, const char* msg) {
  if (_message_out.func) {
    _message_out.func(*this, msg_type, msg, strlen(msg));
  }
}

void TestRunner::message_s(MessageType msg_type, const char* msg, size_t msg_size) {
  if (_message_out.func) {
    _message_out.func(*this, msg_type, msg, msg_size);
  }
}

void TestRunner::print_help() {
  StringBufferTmp<1024> buf;

  buf.append_colorized(color_escape_of(MT::kBright), "ZeroTest Options:\n");
  buf.append_format("  --help        - print this usage\n");
  buf.append_format("  --verbose     - turn on verbose test messages via TEST_LOG()\n");
  buf.append_format("  --no-color    - disable color output\n");
  buf.append_format("  --list        - list all tests (possibly filtered via --run)\n");
  buf.append_format("  --run ...     - run specific test(s) (prefix, suffix, substr)\n");

  if (_options_list.func) {
    buf.append('\n');
    buf.append_colorized(color_escape_of(MT::kBright), "Custom Options:\n");
    message(MT::kStatus, buf);
    _options_list.func(*this);
  }
  else {
    message(MT::kStatus, buf);
  }
}

void TestRunner::set_color_output(bool enabled) {
  _color_output = enabled;
  if (_color_output) {
    memcpy(_symbol_success, "\xE2\x9C\x94", 4);
    memcpy(_symbol_failure, "\xE2\x9C\x97", 4);
    memcpy(_color_escapes, color_escape_table, sizeof(color_escape_table));
  }
  else {
    memcpy(_symbol_success, "[OK]", 5);
    memcpy(_symbol_failure, "[!!]", 5);
    memset(_color_escapes, 0, sizeof(_color_escapes));
  }
}

void TestRunner::list_tests() {
  size_t test_count = 0;

  StringBufferTmp<2048> buf;

  for (unsigned precedence = 0; precedence <= _test_precedence_max; precedence++) {
    for (TestCase* test = globals.test_list; test != nullptr; test = test->next_test) {
      if (test->test_precedence == precedence && can_run(test)) {
        if (test_count == 0) {
          if (_run_filter) {
            buf.append_format("Tests matching --run \"%s\":\n", _run_filter);
          }
          else {
            buf.append_format("Tests:\n");
          }
        }
        buf.append("- ");
        buf.append_colorized_f(color_escape_of(MT::kTestCase), "%s\n", test->test_name);
        test_count++;
      }
    }
  }

  if (!test_count) {
    if (_run_filter) {
      buf.append_colorized_f(color_escape_of(MT::kFailure), "No tests matched --run \"%s\" \n", _run_filter);
      list_filter_options();
    }
    else {
      buf.append_colorized_f(color_escape_of(MT::kFailure), "No tests provided!\n");
    }
  }

  message(MT::kStatus, buf);
}

void TestRunner::list_filter_options() {
  StringBufferTmp<1024> buf;
  buf.append("  - use --run xxx to run a test matching xxx exactly\n");
  buf.append("  - use --run prefix* to run tests having a common prefix\n");
  buf.append("  - use --run *suffix to run tests having a common suffix\n");
  buf.append("  - use --run *substr* to run tests matching a substring\n");
  message(MT::kRunnerMessage, buf);
}

int TestRunner::run_tests() {
  if (has_arg("--help")) {
    print_help();
    return 0;
  }

  if (has_arg("--list")) {
    list_tests();
    return 0;
  }

  if (_run_before.func) {
    _run_before.func(*this);
  }

  size_t test_executed_count = 0u;
  size_t test_skipped_count = 0u;

  for (unsigned precedence = 0; precedence <= _test_precedence_max; precedence++) {
    for (TestCase* test = globals.test_list; test != nullptr; test = test->next_test) {
      if (test->test_precedence == precedence) {
        if (can_run(test)) {
          run_test(test);
          test_executed_count++;
        }
        else {
          test_skipped_count++;
        }
      }
    }
  }

  StringBufferTmp<256> buf;
  MessageType message_type = test_executed_count ? MT::kSuccess : MT::kFailure;

  if (test_executed_count) {
    buf.append('\n');
    buf.append_colorized_f(color_escape_of(message_type), "PASSED");
    buf.append(" [");
    buf.append_colorized_f(color_escape_of(message_type), "%zu %s", test_executed_count, test_executed_count > 1u ? "tests" : "test");

    if (test_skipped_count != 0u) {
      buf.append(", ");
      buf.append_colorized_f(color_escape_of(MT::kImportant), "%zu skipped", test_skipped_count);
    }

    buf.append("]\n");
  }
  else {
    if (_run_filter) {
      buf.append_colorized_f(color_escape_of(message_type), "NO TESTS MATCHED [--run %s] filter!\n", _run_filter);
      list_filter_options();
    }
    else {
      buf.append_colorized_f(color_escape_of(message_type), "NO TESTS AVAILABLE!\n");
    }
  }

  message(message_type, buf);

  if (_run_after.func) {
    _run_after.func(*this);
  }

  return 0;
}

bool TestRunner::can_run(const TestCase* test) const {
  if (!_run_filter || _run_filter[0] == '\0') {
    return true;
  }

  return test_match(test->test_name, _run_filter);
}

void TestRunner::run_test(TestCase* test) {
  _test_running = test;
  _status = kStatusTesting;
  _log_pending.clear();

  TestRunner_log_test_case(*this);
  test->test_func(*this);
  test->test_flags = TestCase::Flags(unsigned(test->test_flags) | unsigned(TestCase::Flags::kFinished));
  TestRunner_test_case_done(*this, true);

  _status = kStatusNone;
  _test_running = nullptr;
}

bool TestRunner::test_variation(const char* variation_name, size_t len) noexcept {
  if (len == SIZE_MAX) {
    len = strlen(variation_name);
  }

  TestRunner_variation_log(*this, variation_name, len);
  return true;
}

TestRunner* test_runner() noexcept {
  return globals.test_runner;
}

void log_info(const char* fmt, ...) noexcept {
  TestRunner* test_runner = globals.test_runner;
  if (test_runner) {
    va_list ap;
    va_start(ap, fmt);

    if (test_runner->_verbose) {
      StringBufferTmp<1024> buf;
      buf.append_format_v(fmt, ap);
      TestRunner_message_indented(*test_runner, MessageType::kTestLog, buf.data(), buf.size());
    }
    else {
      auto& lp = test_runner->_log_pending;
      lp.append_format_v(fmt, ap);
      lp.eat_nl();
      lp.append('\n');
    }

    va_end(ap);
  }
}

} // {ZeroTest}
