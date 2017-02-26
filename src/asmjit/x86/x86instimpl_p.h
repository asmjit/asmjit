// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_X86_X86INSTIMPL_P_H
#define _ASMJIT_X86_X86INSTIMPL_P_H

// [Dependencies]
#include "../x86/x86inst.h"

ASMJIT_BEGIN_NAMESPACE

//! \addtogroup asmjit_x86
//! \{

// ============================================================================
// [asmjit::X86InstImpl]
// ============================================================================

//! \internal
//!
//! Contains X86/X64 specific implementation of APIs provided by `asmjit::Inst`.
//!
//! The purpose of `X86InstImpl` is to move most of the logic out of `X86Inst`.
struct X86InstImpl {
#ifndef ASMJIT_DISABLE_INST_API
  static Error validate(uint32_t archType, const Inst::Detail& detail, const Operand_* operands, uint32_t count) noexcept;
  static Error queryRWInfo(uint32_t archType, const Inst::Detail& detail, const Operand_* operands, uint32_t count, Inst::IRWInfo& out) noexcept;
  static Error queryCpuFeatures(uint32_t archType, const Inst::Detail& detail, const Operand_* operands, uint32_t count, CpuFeatures& out) noexcept;
#endif
};

//! \}

ASMJIT_END_NAMESPACE

// [Guard]
#endif // _ASMJIT_X86_X86INSTIMPL_P_H
