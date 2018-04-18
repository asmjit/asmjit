// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// ZLIB - See LICENSE.md file in the package.

// [Export]
#define ASMJIT_EXPORTS

// [Dependencies]
#include "../core/globals.h"
#include "../core/stringutils.h"

ASMJIT_BEGIN_NAMESPACE

// ============================================================================
// [asmjit::DebugUtils]
// ============================================================================

ASMJIT_FAVOR_SIZE const char* DebugUtils::errorAsString(Error err) noexcept {
  #ifndef ASMJIT_DISABLE_TEXT

  static const char errorMessages[] =
    "Ok\0"
    "No heap memory\0"
    "No virtual memory\0"
    "Invalid argument\0"
    "Invalid state\0"
    "Invalid architecture\0"
    "Not initialized\0"
    "Already initialized\0"
    "Feature not enabled\0"
    "No code generated\0"
    "Code too large\0"
    "Invalid directive\0"
    "Invalid label\0"
    "Label index overflow\0"
    "Label already bound\0"
    "Label already defined\0"
    "Label name too long\0"
    "Invalid label name\0"
    "Invalid parent label\0"
    "Non-local label can't have parent\0"
    "Relocation index overflow\0"
    "Invalid relocation entry\0"
    "Invalid instruction\0"
    "Invalid register type\0"
    "Invalid register group\0"
    "Invalid register's physical id\0"
    "Invalid register's virtual id\0"
    "Invalid prefix combination\0"
    "Invalid lock prefix\0"
    "Invalid xacquire prefix\0"
    "Invalid xrelease prefix\0"
    "Invalid rep prefix\0"
    "Invalid rex prefix\0"
    "Invalid mask, expected {k}\0"
    "Invalid use of {k}\0"
    "Invalid use of {k}{z}\0"
    "Invalid broadcast {1tox}\0"
    "Invalid {er} or {sae} option\0"
    "Invalid address\0"
    "Invalid address index\0"
    "Invalid address scale\0"
    "Invalid use of 64-bit address\0"
    "Invalid displacement\0"
    "Invalid segment\0"
    "Invalid immediate value\0"
    "Invalid operand size\0"
    "Ambiguous operand size\0"
    "Operand size mismatch\0"
    "Invalid option\0"
    "Option already defined\0"
    "Invalid type-info\0"
    "Invalid use of a low 8-bit GPB register\0"
    "Invalid use of a 64-bit GPQ register in 32-bit mode\0"
    "Invalid use of an 80-bit float\0"
    "Not consecutive registers\0"
    "No more physical registers\0"
    "Overlapped registers\0"
    "Overlapping register and arguments base-address register\0"
    "Unknown error\0";
  return StringUtils::findPackedString(errorMessages, std::min<Error>(err, kErrorCount));

  #else

  ASMJIT_UNUSED(err);
  static const char noMessage[] = "";
  return noMessage;

  #endif
}

ASMJIT_FAVOR_SIZE void DebugUtils::debugOutput(const char* str) noexcept {
  #if ASMJIT_OS_WINDOWS
  ::OutputDebugStringA(str);
  #else
  std::fputs(str, stderr);
  #endif
}

ASMJIT_FAVOR_SIZE void DebugUtils::assertionFailed(const char* file, int line, const char* msg) noexcept {
  char str[1024];

  std::snprintf(str, 1024,
    "[asmjit] Assertion failed at %s (line %d):\n"
    "[asmjit] %s\n", file, line, msg);

  debugOutput(str);
  std::abort();
}

ASMJIT_END_NAMESPACE
