// [AsmJit]
// Machine Code Generation for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

#ifndef _ASMJIT_X86_X86LOGGING_P_H
#define _ASMJIT_X86_X86LOGGING_P_H

#include "../core/build.h"
#ifndef ASMJIT_NO_LOGGING

#include "../core/logging.h"
#include "../core/string.h"
#include "../x86/x86globals.h"

ASMJIT_BEGIN_SUB_NAMESPACE(x86)

//! \addtogroup asmjit_x86
//! \{

// ============================================================================
// [asmjit::x86::LoggingInternal]
// ============================================================================

namespace LoggingInternal {
  Error formatRegister(
    String& sb,
    uint32_t flags,
    const BaseEmitter* emitter,
    uint32_t archId,
    uint32_t regType,
    uint32_t regId) noexcept;

  Error formatOperand(
    String& sb,
    uint32_t flags,
    const BaseEmitter* emitter,
    uint32_t archId,
    const Operand_& op) noexcept;

  Error formatInstruction(
    String& sb,
    uint32_t flags,
    const BaseEmitter* emitter,
    uint32_t archId,
    const BaseInst& inst, const Operand_* operands, uint32_t opCount) noexcept;
};

//! \}

ASMJIT_END_SUB_NAMESPACE

#endif // !ASMJIT_NO_LOGGING
#endif // _ASMJIT_X86_X86LOGGING_P_H
