// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Export]
#define ASMJIT_EXPORTS

// [Dependencies - AsmJit]
#include "../base/globals.h"

// [Api-Begin]
#include "../apibegin.h"

namespace asmjit {

// ============================================================================
// [asmjit::DebugUtils]
// ============================================================================

#if !defined(ASMJIT_DISABLE_TEXT)
static const char errorMessages[] = {
  "Ok\0"
  "No heap memory\0"
  "No virtual memory\0"
  "Invalid argument\0"
  "Invalid state\0"
  "Invalid architecture\0"
  "Not initialized\0"
  "No code generated\0"
  "Code too large\0"
  "Label already bound\0"
  "Unknown instruction\0"
  "Illegal instruction\0"
  "Illegal addressing\0"
  "Illegal displacement\0"
  "Overlapped arguments\0"
  "Unknown error\0"
};

static const char* findPackedString(const char* p, uint32_t id, uint32_t maxId) noexcept {
  uint32_t i = 0;

  if (id > maxId)
    id = maxId;

  while (i < id) {
    while (p[0])
      p++;

    p++;
    i++;
  }

  return p;
}
#endif // ASMJIT_DISABLE_TEXT

const char* DebugUtils::errorAsString(Error err) noexcept {
#if !defined(ASMJIT_DISABLE_TEXT)
  return findPackedString(errorMessages, err, kErrorCount);
#else
  static const char noMessage[] = "";
  return noMessage;
#endif
}

void DebugUtils::debugOutput(const char* str) noexcept {
#if ASMJIT_OS_WINDOWS
  ::OutputDebugStringA(str);
#else
  ::fputs(str, stderr);
#endif
}

void DebugUtils::assertionFailed(const char* file, int line, const char* msg) noexcept {
  char str[1024];

  snprintf(str, 1024,
    "[asmjit] Assertion failed at %s (line %d):\n"
    "[asmjit] %s\n", file, line, msg);

  // Support buggy `snprintf` implementations.
  str[1023] = '\0';

  debugOutput(str);
  ::abort();
}

} // asmjit namespace

// [Api-End]
#include "../apiend.h"
