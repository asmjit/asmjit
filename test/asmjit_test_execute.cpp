// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if ASMJIT_ARCH_X86 != 0
  #include <asmjit/x86.h>
#endif

#if ASMJIT_ARCH_ARM == 64
  #include <asmjit/a64.h>
#endif

#include "asmjitutils.h"

using namespace asmjit;

static void printAppInfo() noexcept {
  printf("AsmJit Execute Tests v%u.%u.%u [Arch=%s] [Mode=%s]\n\n",
    unsigned((ASMJIT_LIBRARY_VERSION >> 16)       ),
    unsigned((ASMJIT_LIBRARY_VERSION >>  8) & 0xFF),
    unsigned((ASMJIT_LIBRARY_VERSION      ) & 0xFF),
    asmjitArchAsString(Arch::kHost),
    asmjitBuildType()
  );
}

#if !defined(ASMJIT_NO_JIT) && ((ASMJIT_ARCH_X86 != 0  && !defined(ASMJIT_NO_X86    )) || \
                                (ASMJIT_ARCH_ARM == 64 && !defined(ASMJIT_NO_AARCH64)) )

// Signature of the generated function.
using EmptyFunc = void (*)(void);

// Generate Empty Function
// -----------------------

#if ASMJIT_ARCH_X86 != 0
static void generateEmptyFunc(CodeHolder& code) noexcept {
  x86::Assembler a(&code);
  a.ret();
}
#endif

#if ASMJIT_ARCH_ARM == 64
static void generateEmptyFunc(CodeHolder& code) noexcept {
  a64::Assembler a(&code);
  a.ret(a64::x30);
}
#endif

// Testing
// -------

static void executeEmptyFunc(JitRuntime& rt) noexcept {
  CodeHolder code;
  code.init(rt.environment(), rt.cpuFeatures());

  EmptyFunc fn;

  generateEmptyFunc(code);
  Error err = rt.add(&fn, &code);

  if (err) {
    printf("** FAILURE: JitRuntime::add() failed: %s **\n", DebugUtils::errorAsString(err));
    exit(1);
  }

  fn();

  rt.release(&fn);
}

int main() {
  printAppInfo();

  {
    printf("Trying to execute empty function with JitRuntime (default settings)\n");
    JitRuntime rt;
    executeEmptyFunc(rt);
  }

  if (VirtMem::hardenedRuntimeInfo().hasFlag(VirtMem::HardenedRuntimeFlags::kDualMapping)) {
    printf("Trying to execute empty function with JitRuntime (dual-mapped)\n");
    JitAllocator::CreateParams params {};
    params.options |= JitAllocatorOptions::kUseDualMapping;
    JitRuntime rt;
    executeEmptyFunc(rt);
  }

  // If we are here we were successful, otherwise the process would crash.
  printf("** SUCCESS **\n");
  return 0;
}
#else
int main() {
  printAppInfo();
  printf("!! Execution test suite is disabled: <ASMJIT_NO_JIT> or unsuitable target architecture !!\n");
  return 0;
}
#endif // ASMJIT_ARCH_X86 && !ASMJIT_NO_X86 && !ASMJIT_NO_JIT
