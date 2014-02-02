// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Export]
#define ASMJIT_EXPORTS

// [Dependencies - AsmJit]
#include "../base/assert.h"

// [Api-Begin]
#include "../base/apibegin.h"

// helpers
namespace asmjit {

// ============================================================================
// [asmjit::Assert]
// ============================================================================

void assertionFailed(const char* exp, const char* file, int line) {
  ::fprintf(stderr, "Assertion failed: %s\n, file %s, line %d\n", exp, file, line);
  ::abort();
}

} // asmjit namespace

// [Api-End]
#include "../base/apiend.h"
