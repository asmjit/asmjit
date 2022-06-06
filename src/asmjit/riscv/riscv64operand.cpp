// This file is part of AsmJit project <https://asmjit.com>
//
// See asmjit.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include "../core/api-build_p.h"
#if !defined(ASMJIT_NO_RISCV64)

#include "../core/misc_p.h"
#include "../riscv/riscv64operand.h"

ASMJIT_BEGIN_SUB_NAMESPACE(riscv64)

// riscv64::Operand - Tests
// ====================

#if defined(ASMJIT_TEST)
UNIT(x86_operand) {
    // TODO: [RISC-V] unit test
}
#endif

ASMJIT_END_SUB_NAMESPACE

#endif // !ASMJIT_NO_RISCV64
