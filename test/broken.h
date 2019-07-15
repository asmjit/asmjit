// [Broken]
// Lightweight Unit Testing for C++.
//
// [License]
// Public Domain (Unlicense)

#ifndef BROKEN_INTERNAL_H
#define BROKEN_INTERNAL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <utility>

// Hide everything when using Doxygen. Ideally this can be protected by a macro,
// but there is not globally and widely used one across multiple projects.

//! \internal
//! \{

// ============================================================================
// [Broken - API]
// ============================================================================

struct BrokenAPI {
  //! Entry point of a unit test defined by `UNIT` macro.
  typedef void (*Entry)(void);

  //! Test defined by `UNIT` macro.
  struct Unit {
    const char* name;
    Entry entry;
    size_t finished;
    Unit* next;
  };

  //! Automatic unit registration by using static initialization.
  struct AutoUnit : Unit {
    inline AutoUnit(const char* _name, Entry _entry) noexcept {
      name = _name;
      entry = _entry;
      finished = false;
      next = NULL;

      BrokenAPI::add(this);
    }
  };

  static bool hasArg(const char* name) noexcept;

  //! Register a new unit test (called automatically by `AutoUnit` and `UNIT`).
  static void add(Unit* unit) noexcept;

  //! Set output file to a `file`.
  static void setOutputFile(FILE* file) noexcept;

  //! Initialize `Broken` framework.
  //!
  //! Returns `true` if `run()` should be called.
  static int run(int argc, const char* argv[], Entry onBeforeRun = nullptr, Entry onAfterRun = nullptr) noexcept;

  //! Log message, adds automatically new line if not present.
  static void info(const char* fmt, ...) noexcept;

  //! Called on `EXPECT()` failure.
  static void fail(const char* file, int line, const char* fmt, ...) noexcept;

  //! Used internally by `EXPECT` macro.
  template<typename T>
  static inline void expect(const char* file, int line, const T& exp) noexcept {
    if (!exp)
      fail(file, line, nullptr);
  }

  //! Used internally by `EXPECT` macro.
  template<typename T, typename... Args>
  static inline void expect(const char* file, int line, const T& exp, const char* fmt, Args&&... args) noexcept {
    if (!exp)
      fail(file, line, fmt, std::forward<Args>(args)...);
  }
};

// ============================================================================
// [Broken - Macros]
// ============================================================================

//! Define a unit test.
//!
//! `NAME` can only contain ASCII characters, numbers and underscore. It has
//! the same rules as identifiers in C and C++.
#define UNIT(NAME) \
  static void unit_##NAME##_entry(void) noexcept; \
  \
  static ::BrokenAPI::AutoUnit unit_##NAME##_autoinit( \
    #NAME, unit_##NAME##_entry); \
  \
  static void unit_##NAME##_entry(void) noexcept

//! #define INFO(FORMAT [, ...])
//!
//! Informative message printed to `stdout`.
#define INFO(...) ::BrokenAPI::info(__VA_ARGS__)

//! #define INFO(EXP [, FORMAT [, ...]])
//!
//! Expect `EXP` to be true or evaluates to true, fail otherwise.
#define EXPECT(...) ::BrokenAPI::expect(__FILE__, __LINE__, __VA_ARGS__)

//! \}

#endif // BROKEN_INTERNAL_H
