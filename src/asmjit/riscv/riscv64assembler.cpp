// This file is part of AsmJit project <https://asmjit.com>
//
// See asmjit.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include "../core/api-build_p.h"
#if !defined(ASMJIT_NO_RISCV64)

#include "../core/codewriter_p.h"
#include "../core/cpuinfo.h"
#include "../core/emitterutils_p.h"
#include "../core/formatter.h"
#include "../core/logger.h"
#include "../core/misc_p.h"
#include "../core/support.h"
#include "../riscv/riscv64assembler.h"

ASMJIT_BEGIN_SUB_NAMESPACE(riscv64)

// TODO: [RISC-V]

// riscv64::Assembler - Construction & Destruction
// ===========================================

Assembler::Assembler(CodeHolder* code) noexcept : BaseAssembler() {
  // TODO: [RISC-V]
};

Assembler::~Assembler() noexcept {}

// riscv64::Assembler - Emit
// =====================

Error Assembler::_emit(InstId instId, const Operand_& o0, const Operand_& o1, const Operand_& o2, const Operand_* opExt) {
  // TODO: [RISC-V]
}

// riscv64::Assembler - Align
// ======================

Error Assembler::align(AlignMode alignMode, uint32_t alignment) {
  // TODO: [RISC-V]
}

// riscv64::Assembler - Events
// =======================

Error Assembler::onAttach(CodeHolder* code) noexcept {
  return Base::onAttach(code);
}

Error Assembler::onDetach(CodeHolder* code) noexcept {
  return Base::onDetach(code);
}

ASMJIT_END_SUB_NAMESPACE

#endif // !ASMJIT_NO_RISCV64
