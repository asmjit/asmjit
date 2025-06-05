// This file is part of AsmJit project <https://asmjit.com>
//
// See asmjit.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_LA_A64INSTDB_H_INCLUDED
#define ASMJIT_LA_A64INSTDB_H_INCLUDED

#include "../loongarch/la64globals.h"

ASMJIT_BEGIN_SUB_NAMESPACE(la64)

//! \addtogroup asmjit_a64
//! \{

//! Instruction database.
namespace InstDB {

//! Instruction flags.
enum InstFlags : uint32_t {
  //! The instruction provides conditional execution.
  kInstFlagCond = 0x00000001u,
  //! SIMD instruction that processes elements in pairs.
  kInstFlagPair = 0x00000002u,
  //! SIMD instruction that does widening (Long).
  kInstFlagLong = 0x00000004u,
  //! SIMD instruction that does narrowing (Narrow).
  kInstFlagNarrow = 0x00000008u,
  //! SIMD element access of half-words can only be used with v0..15.
  kInstFlagVH0_15 = 0x00000010u,

  //! Instruction uses consecutive registers if the number of operands is greater than 2.
  kInstFlagConsecutive = 0x00000080u
};

//! Instruction information.
struct InstInfo {
  //! Instruction encoding type.
  uint32_t _encoding : 16;
  //! Index to data specific to each encoding type.
  uint32_t _encodingDataIndex : 16;
  uint32_t _reserved : 16;

  uint16_t _rwInfoIndex;
  uint16_t _flags;

  //! \name Accessors
  //! \{

  ASMJIT_INLINE_NODEBUG uint32_t rwInfoIndex() const noexcept { return _rwInfoIndex; }
  ASMJIT_INLINE_NODEBUG uint32_t flags() const noexcept { return _flags; }

  ASMJIT_INLINE_NODEBUG bool hasFlag(uint32_t flag) const { return (_flags & flag) != 0; }

  //! \}
};

ASMJIT_VARAPI const InstInfo _instInfoTable[];

static inline const InstInfo& infoById(InstId instId) noexcept {
  instId &= uint32_t(InstIdParts::kRealId);
  ASMJIT_ASSERT(Inst::isDefinedId(instId));
  return _instInfoTable[instId];
}

} // {InstDB}

//! \}

ASMJIT_END_SUB_NAMESPACE

#endif // ASMJIT_LA_A64INSTDB_H_INCLUDED
