// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Dependencies - MiniUnit]
#include "./test.h"

// ============================================================================
// [MiniUnit - Statics]
// ============================================================================

int MiniUnit::argc;
const char** MiniUnit::argv;
FILE* MiniUnit::outFile;

MiniUnit::Unit* MiniUnit::unitList;
MiniUnit::Unit* MiniUnit::unitRunning;

// ============================================================================
// [MiniUnit - Init]
// ============================================================================

bool MiniUnit::init(int argc, const char* argv[]) {
  MiniUnit::argc    = argc;
  MiniUnit::argv    = argv;
  MiniUnit::outFile = stdout;

  return unitList != NULL;
}

// ============================================================================
// [MiniUnit - Add]
// ============================================================================

void MiniUnit::addUnit(Unit* unit) {
  Unit** pPrev = &unitList;
  Unit* current = *pPrev;

  while (current != NULL) {
    if (::strcmp(current->name, unit->name) >= 0)
      break;

    pPrev = &current->next;
    current = *pPrev;
  }

  *pPrev = unit;
  unit->next = current;
}

// ============================================================================
// [MiniUnit - Run]
// ============================================================================

void MiniUnit::run() {
  MiniUnit::Unit* unit = unitList;
  while (unit != NULL) {
    runUnit(unit);
    unit = unit->next; 
  }
}

void MiniUnit::runUnit(Unit* unit) {
  info("[Unit] %s", unit->name);
  unitRunning = unit;
  unit->entry();
  unitRunning = NULL;
}

// ============================================================================
// [MiniUnit - Info]
// ============================================================================

void MiniUnit::info(const char* fmt, ...) {
  const char* prefix = unitRunning ? "  " : "";
  size_t len = ::strlen(fmt);

  if (len != 0) {
    va_list ap;
    va_start(ap, fmt);
    ::fputs(prefix, outFile);
    ::vfprintf(outFile, fmt, ap);
    va_end(ap);
  }

  if (len == 0 || fmt[len - 1] != '\n')
    ::fputs("\n", outFile);

  ::fflush(outFile);
}

void MiniUnit::fail(const char* file, int line, const char* fmt, ...) {
  size_t len = ::strlen(fmt);

  if (len != 0) {
    va_list ap;
    va_start(ap, fmt);
    ::fputs("[Fail] ", outFile);
    ::vfprintf(outFile, fmt, ap);
    va_end(ap);
  }

  if (len > 0 && fmt[len - 1] != '\n')
    ::fputs("\n", outFile);

  ::fprintf(outFile, "[File] %s (Line: %d)\n", file, line);

  ::fflush(outFile);
  ::exit(1);
}
