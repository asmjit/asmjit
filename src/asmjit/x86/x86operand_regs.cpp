// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Export]
#define ASMJIT_EXPORTS
#define ASMJIT_EXPORTS_X86_OPERAND

// [Guard]
#include "../asmjit_build.h"
#if defined(ASMJIT_BUILD_X86)

// [Dependencies]
#include "../x86/x86operand.h"

// [Api-Begin]
#include "../asmjit_apibegin.h"

namespace asmjit {

// ============================================================================
// [asmjit::X86OpData]
// ============================================================================

// Register Signature {
//   uint8_t opType;
//   uint8_t regType;
//   uint8_t regKind;
//   uint8_t regSize;
// }
#define ASMJIT_X86_INV_SIGNATURE(TYPE) {{  \
  uint8_t(Operand::kOpNone),               \
  uint8_t(0),                              \
  uint8_t(0),                              \
  uint8_t(0)                               \
}}

#define ASMJIT_X86_REG_SIGNATURE(TYPE) {{  \
  uint8_t(Operand::kOpReg),                \
  uint8_t(TYPE),                           \
  uint8_t(X86RegTraits<TYPE>::kKind),      \
  uint8_t(X86RegTraits<TYPE>::kSize)       \
}}

// Register Operand {
//   uint8_t opType;
//   uint8_t regType;
//   uint8_t regKind;
//   uint8_t regSize;
//   uint32_t id;
//   uint32_t reserved8_4;
//   uint32_t reserved12_4;
// }
#define ASMJIT_X86_REG_01(TYPE, ID) {{{    \
  uint8_t(Operand::kOpReg),                \
  uint8_t(TYPE),                           \
  uint8_t(X86RegTraits<TYPE>::kKind),      \
  uint8_t(X86RegTraits<TYPE>::kSize),      \
  uint32_t(ID),                            \
  uint32_t(0),                             \
  uint32_t(0)                              \
}}}

#define ASMJIT_X86_REG_04(TYPE, ID) \
  ASMJIT_X86_REG_01(TYPE, ID + 0 ), \
  ASMJIT_X86_REG_01(TYPE, ID + 1 ), \
  ASMJIT_X86_REG_01(TYPE, ID + 2 ), \
  ASMJIT_X86_REG_01(TYPE, ID + 3 )

#define ASMJIT_X86_REG_07(TYPE, ID) \
  ASMJIT_X86_REG_04(TYPE, ID + 0 ), \
  ASMJIT_X86_REG_01(TYPE, ID + 4 ), \
  ASMJIT_X86_REG_01(TYPE, ID + 5 ), \
  ASMJIT_X86_REG_01(TYPE, ID + 6 )

#define ASMJIT_X86_REG_08(TYPE, ID) \
  ASMJIT_X86_REG_04(TYPE, ID + 0 ), \
  ASMJIT_X86_REG_04(TYPE, ID + 4 )

#define ASMJIT_X86_REG_16(TYPE, ID) \
  ASMJIT_X86_REG_08(TYPE, ID + 0 ), \
  ASMJIT_X86_REG_08(TYPE, ID + 8 )

#define ASMJIT_X86_REG_32(TYPE, ID) \
  ASMJIT_X86_REG_16(TYPE, ID + 0 ), \
  ASMJIT_X86_REG_16(TYPE, ID + 16)

const X86OpData x86OpData = {
  // --------------------------------------------------------------------------
  // [ArchRegs]
  // --------------------------------------------------------------------------

  {
    // RegType[].
    {
      ASMJIT_X86_INV_SIGNATURE(0),  // #00 (NONE).
      ASMJIT_X86_INV_SIGNATURE(1),  // #01 (LABEL).
      ASMJIT_X86_REG_SIGNATURE(2),  // #02 (RIP).
      ASMJIT_X86_REG_SIGNATURE(3),  // #03 (SEG).
      ASMJIT_X86_REG_SIGNATURE(4),  // #04 (GPB-LO).
      ASMJIT_X86_REG_SIGNATURE(5),  // #05 (GPB-HI).
      ASMJIT_X86_REG_SIGNATURE(6),  // #06 (GPW).
      ASMJIT_X86_REG_SIGNATURE(7),  // #07 (GPD).
      ASMJIT_X86_REG_SIGNATURE(8),  // #08 (GPQ).
      ASMJIT_X86_REG_SIGNATURE(9),  // #09 (FP).
      ASMJIT_X86_REG_SIGNATURE(10), // #10 (MM).
      ASMJIT_X86_REG_SIGNATURE(11), // #11 (K).
      ASMJIT_X86_REG_SIGNATURE(12), // #12 (XMM).
      ASMJIT_X86_REG_SIGNATURE(13), // #13 (YMM).
      ASMJIT_X86_REG_SIGNATURE(14), // #14 (ZMM).
      ASMJIT_X86_INV_SIGNATURE(15), // #15 (FUTURE).
      ASMJIT_X86_REG_SIGNATURE(16), // #16 (BND).
      ASMJIT_X86_REG_SIGNATURE(17), // #17 (CR).
      ASMJIT_X86_REG_SIGNATURE(18), // #18 (DR).
      ASMJIT_X86_INV_SIGNATURE(19)  // #19 (FUTURE).
    },
    // RegTypeToTypeId[].
    {
      X86RegTraits< 0>::kTypeId,
      X86RegTraits< 1>::kTypeId,
      X86RegTraits< 2>::kTypeId,
      X86RegTraits< 3>::kTypeId,
      X86RegTraits< 4>::kTypeId,
      X86RegTraits< 5>::kTypeId,
      X86RegTraits< 6>::kTypeId,
      X86RegTraits< 7>::kTypeId,
      X86RegTraits< 8>::kTypeId,
      X86RegTraits< 9>::kTypeId,
      X86RegTraits<10>::kTypeId,
      X86RegTraits<11>::kTypeId,
      X86RegTraits<12>::kTypeId,
      X86RegTraits<13>::kTypeId,
      X86RegTraits<14>::kTypeId,
      X86RegTraits<15>::kTypeId,
      X86RegTraits<16>::kTypeId,
      X86RegTraits<17>::kTypeId,
      X86RegTraits<18>::kTypeId,
      X86RegTraits<19>::kTypeId
    }
  },

  // --------------------------------------------------------------------------
  // [Registers]
  // --------------------------------------------------------------------------

  { ASMJIT_X86_REG_01(X86Reg::kRegRip  , 0) },
  { ASMJIT_X86_REG_07(X86Reg::kRegSeg  , 0) },
  { ASMJIT_X86_REG_16(X86Reg::kRegGpbLo, 0) },
  { ASMJIT_X86_REG_04(X86Reg::kRegGpbHi, 0) },
  { ASMJIT_X86_REG_16(X86Reg::kRegGpw  , 0) },
  { ASMJIT_X86_REG_16(X86Reg::kRegGpd  , 0) },
  { ASMJIT_X86_REG_16(X86Reg::kRegGpq  , 0) },
  { ASMJIT_X86_REG_08(X86Reg::kRegFp   , 0) },
  { ASMJIT_X86_REG_08(X86Reg::kRegMm   , 0) },
  { ASMJIT_X86_REG_08(X86Reg::kRegK    , 0) },
  { ASMJIT_X86_REG_32(X86Reg::kRegXmm  , 0) },
  { ASMJIT_X86_REG_32(X86Reg::kRegYmm  , 0) },
  { ASMJIT_X86_REG_32(X86Reg::kRegZmm  , 0) },
  { ASMJIT_X86_REG_04(X86Reg::kRegBnd  , 0) },
  { ASMJIT_X86_REG_16(X86Reg::kRegCr   , 0) },
  { ASMJIT_X86_REG_16(X86Reg::kRegDr   , 0) }
};

#undef ASMJIT_X86_REG_32
#undef ASMJIT_X86_REG_16
#undef ASMJIT_X86_REG_08
#undef ASMJIT_X86_REG_04
#undef ASMJIT_X86_REG_01

#undef ASMJIT_X86_REG_SIGNATURE
#undef ASMJIT_X86_INV_SIGNATURE

} // asmjit namespace

// [Api-End]
#include "../asmjit_apiend.h"

// [Guard]
#endif // ASMJIT_BUILD_X86
