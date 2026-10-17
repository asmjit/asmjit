// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core/api_build_test_p.h>
#if defined(ASMJIT_TEST) && !defined(ASMJIT_NO_JIT) && !defined(ASMJIT_NO_X86) && ASMJIT_TARGET_ARCH_X86 == 64

#include <asmjit/axl/mem_ops.h>
#include <asmjit/core/jit_runtime.h>
#include <asmjit/x86/x86_assembler.h>

ASMJIT_BEGIN_NAMESPACE

static int ASMJIT_CDECL jit_runtime_test_callee() noexcept { return 42; }

// Emits a function that calls `jit_runtime_test_callee()` and returns its result, and the offset of the call.
static Error JitRuntimeTest_add_caller(JitRuntime& rt, void** fn, size_t* call_offset) noexcept {
  CodeHolder code;
  ASMJIT_PROPAGATE(code.init(rt.environment()));

  x86::Assembler a(&code);
  a.sub(x86::rsp, 40);
  *call_offset = a.offset();
  a.call(imm((void*)jit_runtime_test_callee));
  a.add(x86::rsp, 40);
  a.ret();

  return rt.add(fn, &code);
}

TEST_CASE(core_jit_runtime_near_calls) {
  using Func = int (ASMJIT_CDECL*)();

  JitAllocator::CreateParams params {};
  params.options = JitAllocatorOptions::kNearModuleAddressRange | JitAllocatorOptions::kStrictAddressRange;

  JitRuntime rt(&params);
  void* fn;
  size_t call_offset;
  Error err = JitRuntimeTest_add_caller(rt, &fn, &call_offset);

  EXPECT_EQ(err, Error::kOk);

  // The call must be relocated to `call rel32` (with the REX prefix that would allow patching it to `call [rip+x]`).
  const uint8_t* call = static_cast<const uint8_t*>(fn) + call_offset;
  EXPECT_EQ(call[0], 0x40u);
  EXPECT_EQ(call[1], 0xE8u);

  int32_t rel32 = axl::loadu_i32_le(call + 2);
  EXPECT_EQ(uintptr_t(call + 6) + uintptr_t(intptr_t(rel32)), uintptr_t(jit_runtime_test_callee));
  EXPECT_EQ(reinterpret_cast<Func>(fn)(), 42);
  EXPECT_EQ(rt.release(fn), Error::kOk);
}

ASMJIT_END_NAMESPACE

#endif // ASMJIT_TEST && !ASMJIT_NO_JIT && !ASMJIT_NO_X86 && ASMJIT_TARGET_ARCH_X86 == 64
