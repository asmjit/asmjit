// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_X86_X86LOGGING_P_H
#define _ASMJIT_X86_X86LOGGING_P_H

#include "../core/build.h"
#ifndef ASMJIT_DISABLE_LOGGING

// [Dependencies]
#include "../core/logging.h"
#include "../core/stringbuilder.h"
#include "../x86/x86globals.h"

ASMJIT_BEGIN_NAMESPACE

//! \addtogroup asmjit_core
//! \{

// ============================================================================
// [asmjit::X86Logging]
// ============================================================================

struct X86Logging {
  static Error formatRegister(
    StringBuilder& sb,
    uint32_t logOptions,
    const CodeEmitter* emitter,
    uint32_t archType,
    uint32_t regType,
    uint32_t regId) noexcept;

  static Error formatOperand(
    StringBuilder& sb,
    uint32_t logOptions,
    const CodeEmitter* emitter,
    uint32_t archType,
    const Operand_& op) noexcept;

  static Error formatInstruction(
    StringBuilder& sb,
    uint32_t logOptions,
    const CodeEmitter* emitter,
    uint32_t archType,
    const Inst::Detail& detail, const Operand_* operands, uint32_t count) noexcept;
};

//! \}

ASMJIT_END_NAMESPACE

// [Guard]
#endif // !ASMJIT_DISABLE_LOGGING
#endif // _ASMJIT_X86_X86LOGGING_P_H
