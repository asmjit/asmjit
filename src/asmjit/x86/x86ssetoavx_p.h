// [AsmJit]
// Machine Code Generation for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

#ifndef _ASMJIT_X86_X86SSETOAVX_P_H
#define _ASMJIT_X86_X86SSETOAVX_P_H

#include "../core/build.h"
#ifndef ASMJIT_DISABLE_BUILDER

#include "../core/support.h"
#include "../x86/x86builder.h"

ASMJIT_BEGIN_SUB_NAMESPACE(x86)

//! \cond INTERNAL
//! \addtogroup asmjit_x86
//! \{

// ============================================================================
// [asmjit::x86::SseToAvxData]
// ============================================================================

//! Contains data that can be used to convert SSE to AVX or vice versa.
struct SseToAvxData {
  //! Conversion mode.
  enum Mode : uint32_t {
    //! No conversion possible.
    kModeNone = 0,
    //! No change (no operands changed).
    kModeMove = 1,
    //! No change if the second operand is mem, extend otherwise.
    kModeMoveIfMem = 2,
    //! The first SSE operand becomes first and second AVX operand.
    kModeExtend = 3,
    //! Special case for 'vblendvpd', 'vblendvps', and 'vpblendvb'.
    kModeBlend = 4
  };

  inline uint32_t mode() const noexcept { return _mode; }
  inline int32_t delta() const noexcept { return _delta; }

  //! SSE to AVX conversion mode, see `Mode`.
  uint16_t _mode :  3;
  //! Delta to get the counterpart SSE/AVX instruction.
  int16_t _delta : 13;
};

// ============================================================================
// [asmjit::x86::SseToAvxPass]
// ============================================================================

class SseToAvxPass final : public Pass {
public:
  ASMJIT_NONCOPYABLE(SseToAvxPass)
  typedef Pass Base;

  bool _translated;

  SseToAvxPass() noexcept;
  Error run(Zone* zone, Logger* logger) noexcept override;

  enum ProbeMask : uint32_t {
    //! Instruction uses MMX registers.
    kProbeMmx  = 1u << Reg::kTypeMm,
    //! Instruction uses XMM registers.
    kProbeXmm  = 1u << Reg::kTypeXmm
  };

  static inline uint32_t probeRegs(const Operand* operands, uint32_t opCount) noexcept {
    uint32_t mask = 0;
    for (uint32_t i = 0; i < opCount; i++) {
      const Operand& op = operands[i];
      if (!op.isReg()) continue;
      mask |= Support::bitMask(op.as<BaseReg>().type());
    }
    return mask;
  }
};

//! \}
//! \endcond

ASMJIT_END_SUB_NAMESPACE

#endif // !ASMJIT_DISABLE_BUILDER
#endif // _ASMJIT_X86_X86SSETOAVX_P_H
