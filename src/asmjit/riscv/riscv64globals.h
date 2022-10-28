// This file is part of AsmJit project <https://asmjit.com>
//
// See asmjit.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_RISCV_RISCV64GLOBALS_H_INCLUDED
#define ASMJIT_RISCV_RISCV64GLOBALS_H_INCLUDED

#include "../riscv/riscvglobals.h"

//! \namespace asmjit::riscv64
//! \ingroup asmjit_riscv64
//!
//! RISC-V64 backend.

ASMJIT_BEGIN_SUB_NAMESPACE(riscv64)

// riscv64 uses everything from arm namespace and adds into it.
using namespace riscv;

//! \addtogroup asmjit_riscv64
//! \{

//! RISC-V 64-bit instruction.
//!
//! \note Only used to hold RISC-V-specific enumerations and static functions.
struct Inst {
  //! Instruction id.
  enum Id : uint32_t {
    // ${riscv64::InstId:Begin}
    // ------------------- Automatically generated, do not edit -------------------
    kIdAdd,
    kIdAddi,
    kIdAnd,
    kIdAndi,
    kIdAuipc,
    kIdBeq,
    kIdBge,
    kIdBgeu,
    kIdBlt,
    kIdBltu,
    kIdBne,
    kIdEbreak,
    kIdEcall,
    kIdFence,
    kIdJal,
    kIdJalr,
    kIdLb,
    kIdLbu,
    kIdLh,
    kIdLhu,
    kIdLui,
    kIdLw,
    kIdOr,
    kIdOri,
    kIdSb,
    kIdSh,
    kIdSll,
    kIdSlt,
    kIdSlti,
    kIdSltiu,
    kIdSltu,
    kIdSra,
    kIdSrl,
    kIdSub,
    kIdSw,
    kIdXor,
    kIdXori,
    _kIdCount
    // ----------------------------------------------------------------------------
    // ${riscv64::InstId:End}
  };

  //! Tests whether the `instId` is defined (counts also Inst::kIdNone, which must be zero).
  //!
  //! \note This function required identifier to be without modifiers. If the given instruction id contains modifiers
  //! it would return false as modifiers overflow `_kIdCount`.
  static constexpr inline bool isDefinedId(InstId instId) noexcept {
    return instId < _kIdCount;
  }
};

//! \}

ASMJIT_END_SUB_NAMESPACE

#endif // ASMJIT_RISCV_RISCV64GLOBALS_H_INCLUDED
