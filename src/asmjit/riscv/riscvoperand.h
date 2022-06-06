// This file is part of AsmJit project <https://asmjit.com>
//
// See asmjit.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_RISCV_RISCVOPERAND_H_INCLUDED
#define ASMJIT_RISCV_RISCVOPERAND_H_INCLUDED

#include "../core/archtraits.h"
#include "../core/operand.h"
#include "../core/type.h"
#include "../riscv/riscvglobals.h"

ASMJIT_BEGIN_SUB_NAMESPACE(riscv)

//! \addtogroup asmjit_riscv
//! \{

class Reg;

class Pc;
class GpX32;
class GpX64;

//! Register traits (RISC-V).
//!
//! Register traits contains information about a particular register type. It's used by asmjit to setup register
//! information on-the-fly and to populate tables that contain register information (this way it's possible to
//! change register types and groups without having to reorder these tables).
template<RegType kRegType>
struct RegTraits : public BaseRegTraits {};

//! \cond
// <--------------------+-----+-------------------------+------------------------+---+---+------------------+
//                      | Reg |        Reg-Type         |        Reg-Group       |Sz |Cnt|      TypeId      |
// <--------------------+-----+-------------------------+------------------------+---+---+------------------+
ASMJIT_DEFINE_REG_TRAITS(Pc   , RegType::kRISCV_PC      , RegGroup::kPC          , 0 , 1 , TypeId::kVoid    );
ASMJIT_DEFINE_REG_TRAITS(GpX32, RegType::kRISCV_GpX32   , RegGroup::kGp          , 4 , 32, TypeId::kInt32   );
ASMJIT_DEFINE_REG_TRAITS(GpX64, RegType::kRISCV_GpX64   , RegGroup::kGp          , 8 , 32, TypeId::kInt64   );

//! Register (RISC-V64).
class Reg : public BaseReg {
public:
  ASMJIT_DEFINE_ABSTRACT_REG(Reg, BaseReg)

  // TODO: [RISC-V] `bool isXXX()` functions

  template<RegType kRegType>
  inline void setRegT(uint32_t id) noexcept {
    setSignature(RegTraits<kRegType>::kSignature);
    setId(id);
  }

  inline void setTypeAndId(RegType type, uint32_t id) noexcept {
    setSignature(signatureOf(type));
    setId(id);
  }

  static inline RegGroup groupOf(RegType type) noexcept { return ArchTraits::byArch(Arch::kRISCV64).regTypeToGroup(type); }
  static inline TypeId typeIdOf(RegType type) noexcept { return ArchTraits::byArch(Arch::kRISCV64).regTypeToTypeId(type); }
  static inline OperandSignature signatureOf(RegType type) noexcept { return ArchTraits::byArch(Arch::kRISCV64).regTypeToSignature(type); }

  template<RegType kRegType>
  static inline RegGroup groupOfT() noexcept { return RegTraits<kRegType>::kGroup; }

  template<RegType kRegType>
  static inline TypeId typeIdOfT() noexcept { return RegTraits<kRegType>::kTypeId; }

  template<RegType kRegType>
  static inline OperandSignature signatureOfT() noexcept { return RegTraits<kRegType>::kSignature; }

  // TODO: [RISC-V] `bool isXXX(const Operand_& op)` functions
};

//! General purpose register (RISC-V64).
class Gp : public Reg {
public:
  ASMJIT_DEFINE_ABSTRACT_REG(Gp, Reg)

  // TODO: [RISC-V]
};

//! Program counter register (RISC-V).
class Pc : public Reg { ASMJIT_DEFINE_FINAL_REG(Pc, Reg, RegTraits<RegType::kRISCV_PC>) };
//! 32-bit GPX (RISC-V32) register.
class GpX32 : public Gp { ASMJIT_DEFINE_FINAL_REG(GpX32, Gp, RegTraits<RegType::kRISCV_GpX32>) };
//! 64-bit GPX (RISC-V64) register.
class GpX64 : public Gp { ASMJIT_DEFINE_FINAL_REG(GpX64, Gp, RegTraits<RegType::kRISCV_GpX64>) };

// TODO: [RISC-V]

#ifndef _DOXYGEN
namespace regs {
#endif

// TODO: [RISC-V]

#ifndef _DOXYGEN
} // {regs}

// Make `risc::regs` accessible through `risc` namespace as well.
using namespace regs;
#endif

// TODO: [RISC-V]

ASMJIT_END_SUB_NAMESPACE

//! \cond INTERNAL
ASMJIT_BEGIN_NAMESPACE
ASMJIT_DEFINE_TYPE_ID(riscv::GpX32, TypeId::kInt32);
ASMJIT_DEFINE_TYPE_ID(riscv::GpX64, TypeId::kInt64);
ASMJIT_END_NAMESPACE
//! \endcond

#endif // ASMJIT_RISCV_RISCVOPERAND_H_INCLUDED