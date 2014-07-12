// [Broken]
// Lightweight Unit Testing for C++.
//
// [License]
// Public Domain (Unlicense)

// [Guard]
#ifndef BROKEN_INTERNAL_H
#define BROKEN_INTERNAL_H

// [Dependencies - C]
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// If using Doxygen to document a source-code hide everything. Ideally this
// can be also done by a macro, but there is no global and widely used one.

//! \internal
//! \{

// ============================================================================
// [Broken - API]
// ============================================================================

struct BrokenAPI {
  //! Test entry point.
  typedef void (*Entry)(void);

  //! Test unit.
  struct Unit {
    const char* name;
    Entry entry;
    size_t finished;
    Unit* next;
  };

  //! Automatic unit registration by using static initialization.
  struct AutoUnit : Unit {
    inline AutoUnit(const char* _name, Entry _entry) {
      name = _name;
      entry = _entry;
      finished = false;
      next = NULL;

      BrokenAPI::add(this);
    }
  };

  //! Register a new test (called automatically by `AutoUnit` and `UNIT`).
  static void add(Unit* unit);

  //! Set output file to `file`.
  static void setOutputFile(FILE* file);

  //! Set the current context.
  static void setContext(const char* file, int line);

  //! Initialize `Broken` framework.
  //!
  //! Returns `true` if `run()` should be called.
  static int run(int argc, const char* argv[],
    Entry onBeforeRun = (Entry)NULL,
    Entry onAfterRun = (Entry)NULL);

  //! 
  template<typename T>
  static void expect(const T& exp, const char* fmt = NULL, ...) {
    if (exp)
      return;

    va_list ap;
    va_start(ap, fmt);
    fail(fmt, ap);
    va_end(ap);
  }

  //! Log message, adds automatically new line if not present.
  static void info(const char* fmt, ...);
  //! Called on `EXPECT()` failure.
  static void fail(const char* fmt, va_list ap);
};

// ============================================================================
// [Broken - Macros]
// ============================================================================

//! Define a unit.
//!
//! `_Name_` can only contain ASCII characters, numbers and underscore. It has
//! the same rules as identifiers in C and C++.
#define UNIT(_Name_) \
  static void unit_##_Name_##_entry(void); \
  \
  static ::BrokenAPI::AutoUnit unit_##_Name_##_autoinit( \
    #_Name_, unit_##_Name_##_entry); \
  \
  static void unit_##_Name_##_entry(void)

//! Informative message printed to stdout.
#define INFO(...) \
  ::BrokenAPI::info(__VA_ARGS__)

//! Expect `_Exp_` to be truthy, fail otherwise.
#define EXPECT(...) \
  do { \
    ::BrokenAPI::setContext(__FILE__, __LINE__); \
    ::BrokenAPI::expect(__VA_ARGS__); \
  } while(0)

//! \}

// [Guard]
#endif // BROKEN_INTERNAL_H
