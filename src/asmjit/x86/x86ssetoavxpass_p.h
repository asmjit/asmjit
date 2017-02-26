// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_X86_X86SSETOAVXPASS_P_H
#define _ASMJIT_X86_X86SSETOAVXPASS_P_H

#include "../core/build.h"
#ifndef ASMJIT_DISABLE_BUILDER

// [Dependencies]
#include "../core/intutils.h"
#include "../x86/x86builder.h"

ASMJIT_BEGIN_NAMESPACE

//! \addtogroup asmjit_x86
//! \{

// ============================================================================
// [asmjit::X86SseToAvxPass]
// ============================================================================

class X86SseToAvxPass : public CBPass {
  ASMJIT_NONCOPYABLE(X86SseToAvxPass)
public:
  typedef CBPass Base;

  X86SseToAvxPass() noexcept;
  Error run(Zone* zone, Logger* logger) noexcept override;

  enum ProbeMask : uint32_t {
    kProbeMmx  = 1U << X86Reg::kRegMm,    //!< Instruction uses MMX registers.
    kProbeXmm  = 1U << X86Reg::kRegXmm    //!< Instruction uses XMM registers.
  };

  static inline uint32_t probeRegs(const Operand* operands, uint32_t count) noexcept {
    uint32_t mask = 0;
    for (uint32_t i = 0; i < count; i++) {
      const Operand& op = operands[i];
      if (!op.isReg()) continue;
      mask |= IntUtils::mask(op.as<Reg>().getType());
    }
    return mask;
  }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  bool _translated;
};

//! \}

ASMJIT_END_NAMESPACE

// [Guard]
#endif // !ASMJIT_DISABLE_BUILDER
#endif // _ASMJIT_X86_X86SSETOAVXPASS_P_H
