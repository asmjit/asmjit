// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// ZLIB - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_X86_X86LOGGING_P_H
#define _ASMJIT_X86_X86LOGGING_P_H

#include "../core/build.h"
#ifndef ASMJIT_DISABLE_LOGGING

// [Dependencies]
#include "../core/logging.h"
#include "../core/stringbuilder.h"
#include "../x86/x86globals.h"

ASMJIT_BEGIN_SUB_NAMESPACE(x86)

//! \addtogroup asmjit_x86_api
//! \{

// ============================================================================
// [asmjit::x86::LoggingInternal]
// ============================================================================

namespace LoggingInternal {
  Error formatRegister(
    StringBuilder& sb,
    uint32_t flags,
    const BaseEmitter* emitter,
    uint32_t archId,
    uint32_t regType,
    uint32_t regId) noexcept;

  Error formatOperand(
    StringBuilder& sb,
    uint32_t flags,
    const BaseEmitter* emitter,
    uint32_t archId,
    const Operand_& op) noexcept;

  Error formatInstruction(
    StringBuilder& sb,
    uint32_t flags,
    const BaseEmitter* emitter,
    uint32_t archId,
    const BaseInst& inst, const Operand_* operands, uint32_t count) noexcept;
};

//! \}

ASMJIT_END_SUB_NAMESPACE

// [Guard]
#endif // !ASMJIT_DISABLE_LOGGING
#endif // _ASMJIT_X86_X86LOGGING_P_H
