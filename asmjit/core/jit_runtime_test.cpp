// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core/api_build_test_p.h>
#if defined(ASMJIT_TEST) && !defined(ASMJIT_NO_JIT)

#include <asmjit/core/jit_runtime.h>

#if ASMJIT_TARGET_ARCH_X86 != 0 && !defined(ASMJIT_NO_X86)
  #include <asmjit/x86/x86_assembler.h>
#endif

#if ASMJIT_TARGET_ARCH_ARM == 64 && !defined(ASMJIT_NO_AARCH64)
  #include <asmjit/arm/a64_assembler.h>
#endif

ASMJIT_BEGIN_NAMESPACE

// JitRuntime::add() must fail if the code references a label that was never bound.
#if ASMJIT_TARGET_ARCH_X86 != 0 && !defined(ASMJIT_NO_X86)
TEST_CASE(core_jit_runtime_unbound_label) {
  JitRuntime rt;
  CodeHolder code;
  code.init(rt.environment());

  x86::Assembler a(&code);
  Label unbound = a.new_label();
  a.jmp(unbound);

  void* fn {};
  EXPECT_EQ(rt.add(&fn, &code), Error::kUnresolvedFixups);
}
#endif

#if ASMJIT_TARGET_ARCH_ARM == 64 && !defined(ASMJIT_NO_AARCH64)
TEST_CASE(core_jit_runtime_unbound_label) {
  JitRuntime rt;
  CodeHolder code;
  code.init(rt.environment());

  a64::Assembler a(&code);
  Label unbound = a.new_label();
  a.b(unbound);

  void* fn {};
  EXPECT_EQ(rt.add(&fn, &code), Error::kUnresolvedFixups);
}
#endif

ASMJIT_END_NAMESPACE

#endif // ASMJIT_TEST
