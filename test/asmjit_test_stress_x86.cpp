// This file is part of AsmJit project <https://asmjit.com>
//
// See asmjit.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

// ----------------------------------------------------------------------------
// This is a working example that generates a very large function with
// non-trivial control flow.
// It is adapted from the ZScript compiler of the ZQuest Classic game engine.
//
// Source:
// https://github.com/ZQuestClassic/ZQuestClassic/blob/72c677bd35a94c3a52ea674c2a4d8682b9fd3027/src/zc/jit_x64.cpp
//
// Explainer for ZScript JIT compiler:
// https://github.com/ZQuestClassic/ZQuestClassic/blob/72c677bd35a94c3a52ea674c2a4d8682b9fd3027/docs/jit.md
// ----------------------------------------------------------------------------

#include <asmjit/core.h>
#if ASMJIT_ARCH_X86 && !defined(ASMJIT_NO_X86) && !defined(ASMJIT_NO_JIT)

#include <asmjit/x86.h>
#include <stdio.h>
#include "asmjit_test_stress_x86_impl.h"

using namespace asmjit;

int main() {
  printf("AsmJit X86 Stress Test\n\n");

  Environment env = Environment::host();
  JitAllocator allocator;

#ifndef ASMJIT_NO_LOGGING
  FileLogger logger(stdout);
  logger.setIndentation(FormatIndentationGroup::kCode, 2);
#endif

  CodeHolder code;
  code.init(env);

#ifndef ASMJIT_NO_LOGGING
  code.setLogger(&logger);
#endif

  bool success = stress_compiler();

  printf("\n");
  if (!success) {
    printf("** FAILURE: The function did not compile **\n");
    return 1;
  }

  printf("** SUCCESS **\n");
  return 0;
}

#else
int main() {
  printf("AsmJit X86 Stress Test is disabled on non-x86 host or when compiled with ASMJIT_NO_JIT\n\n");
  return 0;
}
#endif // ASMJIT_ARCH_X86 && !ASMJIT_NO_X86 && !ASMJIT_NO_JIT
