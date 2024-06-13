// This file is part of AsmJit project <https://asmjit.com>
//
// See asmjit.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_LA_LAUTILS_H_INCLUDED
#define ASMJIT_LA_LAUTILS_H_INCLUDED

#include "../core/support.h"
#include "../loongarch/laglobals.h"

ASMJIT_BEGIN_SUB_NAMESPACE(la)

//! \addtogroup asmjit_la
//! \{

//! Public utilities and helpers for targeting Loongarch architectures.
namespace Utils {

//! Decomposed fields of a logical immediate value.
struct LogicalImm {
  uint32_t n;
  uint32_t s;
  uint32_t r;
};

//! A generic implementation that checjs whether a floating point value can be converted to LA Imm8.
template<typename T, uint32_t kNumBBits, uint32_t kNumCDEFGHBits, uint32_t kNumZeroBits>
static ASMJIT_FORCE_INLINE bool isFPImm8Generic(T val) noexcept {
  constexpr uint32_t kAllBsMask = Support::lsbMask<uint32_t>(kNumBBits);
  constexpr uint32_t kB0Pattern = Support::bitMask(kNumBBits - 1);
  constexpr uint32_t kB1Pattern = kAllBsMask ^ kB0Pattern;

  T immZ = val & Support::lsbMask<T>(kNumZeroBits);
  uint32_t immB = uint32_t(val >> (kNumZeroBits + kNumCDEFGHBits)) & kAllBsMask;

  // ImmZ must be all zeros and ImmB must either be B0 or B1 pattern.
  return immZ == 0 && (immB == kB0Pattern || immB == kB1Pattern);
}
//! \endcond

//! Returns true if the given half precision floating point `val` can be encoded as LA IMM8 value, which represents
//! a limited set of floating point immediate values, which can be used with FMOV instruction.
//!
//! The floating point must have bits distributed in the following way:
//!
//! ```
//! [aBbbcdef|gh000000]
//! ```
static ASMJIT_INLINE_NODEBUG bool isFP16Imm8(uint32_t val) noexcept { return isFPImm8Generic<uint32_t, 3, 6, 6>(val); }

//! Returns true if the given single precision floating point `val` can be encoded as LA IMM8 value, which represents
//! a limited set of floating point immediate values, which can be used with FMOV instruction.
//!
//! The floating point must have bits distributed in the following way:
//!
//! ```
//! [aBbbbbbc|defgh000|00000000|00000000]
//! ```
static ASMJIT_INLINE_NODEBUG bool isFP32Imm8(uint32_t val) noexcept { return isFPImm8Generic<uint32_t, 6, 6, 19>(val); }
//! \overload
static ASMJIT_INLINE_NODEBUG bool isFP32Imm8(float val) noexcept { return isFP32Imm8(Support::bitCast<uint32_t>(val)); }

//! Returns true if the given double precision floating point `val` can be encoded as LA IMM8 value, which represents
//! a limited set of floating point immediate values, which can be used with FMOV instruction.
//!
//! The floating point must have bits distributed in the following way:
//!
//! ```
//! [aBbbbbbb|bbcdefgh|00000000|00000000|00000000|00000000|00000000|00000000]
//! ```
static ASMJIT_INLINE_NODEBUG bool isFP64Imm8(uint64_t val) noexcept { return isFPImm8Generic<uint64_t, 9, 6, 48>(val); }
//! \overload
static ASMJIT_INLINE_NODEBUG bool isFP64Imm8(double val) noexcept { return isFP64Imm8(Support::bitCast<uint64_t>(val)); }

//! \cond
template<typename T, uint32_t kNumBBits, uint32_t kNumCDEFGHBits, uint32_t kNumZeroBits>
static ASMJIT_INLINE_NODEBUG uint32_t encodeFPToImm8Generic(T val) noexcept {
  uint32_t bits = uint32_t(val >> kNumZeroBits);
  return ((bits >> (kNumBBits + kNumCDEFGHBits - 7)) & 0x80u) | (bits & 0x7F);
}
//! \endcond

//! Encodes a double precision floating point value into IMM8 format.
//!
//! \note This function expects that `isFP64Imm8(val) == true` so it doesn't perform any checks of the value and just
//! rearranges some bits into Imm8 order.
static ASMJIT_INLINE_NODEBUG uint32_t encodeFP64ToImm8(uint64_t val) noexcept { return encodeFPToImm8Generic<uint64_t, 9, 6, 48>(val); }
//! \overload
static ASMJIT_INLINE_NODEBUG uint32_t encodeFP64ToImm8(double val) noexcept { return encodeFP64ToImm8(Support::bitCast<uint64_t>(val)); }

} // {Utils}

//! \}

ASMJIT_END_SUB_NAMESPACE

#endif // ASMJIT_LA_LAUTILS_H_INCLUDED

