// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_X86_X86LOGGING_H
#define _ASMJIT_X86_X86LOGGING_H

#include "../asmjit_build.h"
#if !defined(ASMJIT_DISABLE_LOGGING)

// [Dependencies]
#include "../base/logging.h"

// [Api-Begin]
#include "../asmjit_apibegin.h"

namespace asmjit {

//! \addtogroup asmjit_base
//! \{

// ============================================================================
// [asmjit::X86Formatter]
// ============================================================================

class ASMJIT_VIRTAPI X86Formatter : public Formatter {
public:
  ASMJIT_API X86Formatter() noexcept;
  ASMJIT_API virtual ~X86Formatter() noexcept;

  ASMJIT_API virtual Error formatRegister(StringBuilder& out, uint32_t logOptions, uint32_t regType, uint32_t regId) const noexcept override;
  ASMJIT_API virtual Error formatOperand(StringBuilder& out, uint32_t logOptions, const Operand_& op) const noexcept override;

  ASMJIT_API virtual Error formatInstruction(
    StringBuilder& out,
    uint32_t logOptions,
    uint32_t instId,
    uint32_t options,
    const Operand_& opExtra,
    const Operand_* opArray, uint32_t opCount) const noexcept override;
};

//! \}

} // asmjit namespace

// [Api-End]
#include "../asmjit_apiend.h"

// [Guard]
#endif // !ASMJIT_DISABLE_LOGGING
#endif // _ASMJIT_X86_X86LOGGING_H
