// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef MINIUNIT_H
#define MINIUNIT_H

// [Dependencies - C]
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

//! \internal
//! \{

// ============================================================================
// [MiniUnit]
// ============================================================================

//! Define a unit.
//!
//! `_Name_` can only contain ASCII characters, numbers and underscore.
#define UNIT(_Name_) \
  static void unit_##_Name_##_entry(void); \
  static ::MiniUnit::AutoUnit unit_##_Name_##_autoinit(#_Name_, unit_##_Name_##_entry); \
  static void unit_##_Name_##_entry(void)

//! Informative message printed to stdout.
#define INFO(...) \
  ::MiniUnit::info(__VA_ARGS__)

//! Expect `_Exp_` to be truthy, fail otherwise.
#define EXPECT(_Exp_, ...) \
  do { \
    if (!(_Exp_)) ::MiniUnit::fail(__FILE__, __LINE__, __VA_ARGS__); \
  } while(0)

struct MiniUnit {
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

      MiniUnit::addUnit(this);
    }
  };

  //! Test arguments count.
  static int argc;
  //! Test arguments list.
  static const char** argv;
  //! File where to log.
  static FILE* outFile;
  //! Test unit list.
  static Unit* unitList;
  //! Running test reference.
  static Unit* unitRunning;

  //! Initialize MiniUnit framework.
  //!
  //! Returns `true` if `run()` should be called.
  static bool init(int argc, const char* argv[]);

  //! Register a new test.
  static void addUnit(Unit* unit);

  //! Run all units.
  static void run();
  //! Run a single unit.
  static void runUnit(Unit* unit);

  //! Log message, adds automatically new line if not present.
  static void info(const char* fmt, ...);
  //! Called on `EXPECT()` failure.
  static void fail(const char* file, int line, const char* fmt, ...);
};

//! \}

// [Guard]
#endif // MINIUNIT_H
