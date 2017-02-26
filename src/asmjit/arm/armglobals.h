// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_ARM_ARMGLOBALS_H
#define _ASMJIT_ARM_ARMGLOBALS_H

// [Dependencies]
#include "../core/globals.h"

ASMJIT_BEGIN_NAMESPACE
namespace arm {

//! \addtogroup asmjit_arm
//! \{

// ============================================================================
// [asmjit::armdefs::Cond]
// ============================================================================

//! Condition code.
enum Cond : uint32_t {
  kCondEQ               = 0x00U,         //!<        Z==1         (any_sign ==)
  kCondNE               = 0x01U,         //!<        Z==0         (any_sign !=)
  kCondCS               = 0x02U,         //!< C==1                (unsigned >=)
  kCondHS               = 0x02U,         //!< C==1                (unsigned >=)
  kCondCC               = 0x03U,         //!< C==0                (unsigned < )
  kCondLO               = 0x03U,         //!< C==0                (unsigned < )
  kCondMI               = 0x04U,         //!<               N==1  (is negative)
  kCondPL               = 0x05U,         //!<               N==0  (is positive or zero)
  kCondVS               = 0x06U,         //!<               V==1  (is overflow)
  kCondVC               = 0x07U,         //!<               V==0  (no overflow)
  kCondHI               = 0x08U,         //!< C==1 & Z==0         (unsigned > )
  kCondLS               = 0x09U,         //!< C==0 | Z==1         (unsigned <=)
  kCondGE               = 0x0AU,         //!<               N==V  (signed   >=)
  kCondLT               = 0x0BU,         //!<               N!=V  (signed   < )
  kCondGT               = 0x0CU,         //!<        Z==0 & N==V  (signed   > )
  kCondLE               = 0x0DU,         //!<        Z==1 | N!=V  (signed   <=)
  kCondAL               = 0x0EU,         //!< (no condition code) (always)
  kCondNA               = 0x0FU,         //!< (not available)     (special)
  kCondCount            = 0x10U,

  // Simplified condition codes.
  kCondSign             = kCondMI,       //!< Sign.
  kCondNotSign          = kCondPL,       //!< Not sign.

  kCondOverflow         = kCondVS,       //!< Signed overflow.
  kCondNotOverflow      = kCondVC,       //!< Not signed overflow.

  kCondEqual            = kCondEQ,       //!< Equal     `a == b`.
  kCondNotEqual         = kCondNE,       //!< Not Equal `a != b`.

  kCondSignedLT         = kCondLT,       //!< Signed    `a <  b`.
  kCondSignedLE         = kCondLE,       //!< Signed    `a <= b`.
  kCondSignedGT         = kCondGT,       //!< Signed    `a >  b`.
  kCondSignedGE         = kCondGE,       //!< Signed    `a >= b`.

  kCondUnsignedLT       = kCondLO,       //!< Unsigned  `a <  b`.
  kCondUnsignedLE       = kCondLS,       //!< Unsigned  `a <= b`.
  kCondUnsignedGT       = kCondHI,       //!< Unsigned  `a >  b`.
  kCondUnsignedGE       = kCondHS,       //!< Unsigned  `a >= b`.

  kCondZero             = kCondEQ,
  kCondNotZero          = kCondNE,

  kCondNegative         = kCondMI,
  kCondPositive         = kCondPL,

  kCondAlways           = kCondAL        //!< No condition code (always).
};

// ============================================================================
// [asmjit::armdefs::ShiftType]
// ============================================================================

//! Shift (and/or rotation) type.
enum ShiftType : uint32_t {
  kShiftLSL             = 0x00U,         //!< Shift left logical (default).
  kShiftLSR             = 0x01U,         //!< Shift right logical.
  kShiftASR             = 0x02U,         //!< Shift right arithmetic.
  kShiftROR             = 0x03U,         //!< Rotate right.
  kShiftRRX             = 0x04U          //!< Rotate right with carry (encoded as \ref kShiftROR with zero).
};

// ============================================================================
// [asmjit::armdefs::BarrierOption]
// ============================================================================

//! Memory barrier options.
enum BarrierOption : uint32_t {
  kBarrierReserved0     = 0x00U,
  kBarrierOSHLD         = 0x01U,         //!< Waits only for loads to complete, and only applies to the outer shareable domain.
  kBarrierOSHST         = 0x02U,         //!< Waits only for stores to complete, and only applies to the outer shareable domain.
  kBarrierOSH           = 0x03U,         //!< Only applies to the outer shareable domain.
  kBarrierReserved4     = 0x04U,
  kBarrierNSHLD         = 0x05U,         //!< Waits only for loads to complete and only applies out to the point of unification.
  kBarrierNSHST         = 0x06U,         //!< Waits only for stores to complete and only applies out to the point of unification.
  kBarrierNSH           = 0x07U,         //!< Only applies out to the point of unification.
  kBarrierReserved8     = 0x08U,
  kBarrierISHLD         = 0x09U,         //!< Waits only for loads to complete, and only applies to the inner shareable domain.
  kBarrierISHST         = 0x0AU,         //!< Waits only for stores to complete, and only applies to the inner shareable domain.
  kBarrierISH           = 0x0BU,         //!< Only applies to the inner shareable domain.
  kBarrierReserved12    = 0x0CU,
  kBarrierLD            = 0x0DU,         //!< Waits only for loads to complete.
  kBarrierST            = 0x0EU,         //!< Waits only for stores to complete.
  kBarrierSY            = 0x0FU          //!< Full system memory barrier operation.
};

//! \}

} // arm namespace
ASMJIT_END_NAMESPACE

// [Guard]
#endif // _ASMJIT_ARM_ARMGLOBALS_H
