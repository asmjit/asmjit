// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// ZLIB - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_X86_X86SSETOAVX_P_H
#define _ASMJIT_X86_X86SSETOAVX_P_H

#include "../core/build.h"
#ifndef ASMJIT_DISABLE_BUILDER

// [Dependencies]
#include "../core/support.h"
#include "../x86/x86builder.h"

ASMJIT_BEGIN_SUB_NAMESPACE(x86)

//! \addtogroup asmjit_x86_api
//! \{

// ============================================================================
// [asmjit::x86::SseToAvxData]
// ============================================================================

//! Contains data that can be used to convert SSE to AVX or vice versa.
struct SseToAvxData {
  //! Conversion mode.
  enum Mode : uint32_t {
    kModeNone       = 0,                 //!< No conversion possible.
    kModeMove       = 1,                 //!< No change (no operands changed).
    kModeMoveIfMem  = 2,                 //!< No change if the second operand is mem, extend otherwise.
    kModeExtend     = 3,                 //!< The first SSE operand becomes first and second AVX operand.
    kModeBlend      = 4                  //!< Special case for 'vblendvpd', 'vblendvps', and 'vpblendvb'.
  };

  inline uint32_t mode() const noexcept { return _mode; }
  inline int32_t delta() const noexcept { return _delta; }

  uint16_t _mode :  3;                   //!< SSE to AVX conversion mode, see `Mode`.
  int16_t _delta : 13;                   //!< Delta to get the counterpart SSE/AVX instruction.
};

// ============================================================================
// [asmjit::x86::SseToAvxPass]
// ============================================================================

class SseToAvxPass final : public Pass {
public:
  ASMJIT_NONCOPYABLE(SseToAvxPass)
  typedef Pass Base;

  SseToAvxPass() noexcept;
  Error run(Zone* zone, Logger* logger) noexcept override;

  enum ProbeMask : uint32_t {
    kProbeMmx  = 1u << Reg::kTypeMm,     //!< Instruction uses MMX registers.
    kProbeXmm  = 1u << Reg::kTypeXmm     //!< Instruction uses XMM registers.
  };

  static inline uint32_t probeRegs(const Operand* operands, uint32_t count) noexcept {
    uint32_t mask = 0;
    for (uint32_t i = 0; i < count; i++) {
      const Operand& op = operands[i];
      if (!op.isReg()) continue;
      mask |= Support::mask(op.as<BaseReg>().type());
    }
    return mask;
  }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  bool _translated;
};

//! \}

ASMJIT_END_SUB_NAMESPACE

// [Guard]
#endif // !ASMJIT_DISABLE_BUILDER
#endif // _ASMJIT_X86_X86SSETOAVX_P_H
