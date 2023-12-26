// This file is part of AsmJit project <https://asmjit.com>
//
// See asmjit.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_ARM_ARMOPERAND_H_INCLUDED
#define ASMJIT_ARM_ARMOPERAND_H_INCLUDED

#include "../core/archtraits.h"
#include "../core/operand.h"
#include "../core/type.h"
#include "../arm/armglobals.h"

ASMJIT_BEGIN_SUB_NAMESPACE(arm)

//! \addtogroup asmjit_arm
//! \{

class Reg;
class Mem;

class Vec;
class VecB;
class VecH;
class VecS;
class VecD;
class VecV;

//! Register traits (AArch32/AArch64).
//!
//! Register traits contains information about a particular register type. It's used by asmjit to setup register
//! information on-the-fly and to populate tables that contain register information (this way it's possible to
//! change register types and groups without having to reorder these tables).
template<RegType kRegType>
struct RegTraits : public BaseRegTraits {};

//! \cond
// <--------------------+------------------------+------------------------+---+------------------+
//                      |       Reg-Type         |        Reg-Group       |Sz |      TypeId      |
// <--------------------+------------------------+------------------------+---+------------------+
ASMJIT_DEFINE_REG_TRAITS(RegType::kARM_GpW       , RegGroup::kGp          , 4 , TypeId::kInt32   ); // AArch32 & AArch64
ASMJIT_DEFINE_REG_TRAITS(RegType::kARM_GpX       , RegGroup::kGp          , 8 , TypeId::kInt64   ); //           AArch64
ASMJIT_DEFINE_REG_TRAITS(RegType::kARM_VecB      , RegGroup::kVec         , 1 , TypeId::kVoid    ); //           AArch64
ASMJIT_DEFINE_REG_TRAITS(RegType::kARM_VecH      , RegGroup::kVec         , 2 , TypeId::kVoid    ); //           AArch64
ASMJIT_DEFINE_REG_TRAITS(RegType::kARM_VecS      , RegGroup::kVec         , 4 , TypeId::kInt32x1 ); // AArch32 & AArch64
ASMJIT_DEFINE_REG_TRAITS(RegType::kARM_VecD      , RegGroup::kVec         , 8 , TypeId::kInt32x2 ); // AArch32 & AArch64
ASMJIT_DEFINE_REG_TRAITS(RegType::kARM_VecV      , RegGroup::kVec         , 16, TypeId::kInt32x4 ); // AArch32 & AArch64
ASMJIT_DEFINE_REG_TRAITS(RegType::kARM_PC        , RegGroup::kPC          , 8 , TypeId::kInt64   ); //           AArch64
//! \endcond

//! Register operand that can represent AArch32 and AArch64 registers.
class Reg : public BaseReg {
public:
  ASMJIT_DEFINE_ABSTRACT_REG(Reg, BaseReg)

  //! Gets whether the register is either `R` or `W` register (32-bit).
  ASMJIT_INLINE_NODEBUG constexpr bool isGpR() const noexcept { return baseSignature() == RegTraits<RegType::kARM_GpW>::kSignature; }
  //! Gets whether the register is either `R` or `W` register (32-bit).
  ASMJIT_INLINE_NODEBUG constexpr bool isGpW() const noexcept { return baseSignature() == RegTraits<RegType::kARM_GpW>::kSignature; }
  //! Gets whether the register is an `X` register (64-bit).
  ASMJIT_INLINE_NODEBUG constexpr bool isGpX() const noexcept { return baseSignature() == RegTraits<RegType::kARM_GpX>::kSignature; }

  //! Gets whether the register is a VEC-B register (8-bit).
  ASMJIT_INLINE_NODEBUG constexpr bool isVecB() const noexcept { return baseSignature() == RegTraits<RegType::kARM_VecB>::kSignature; }
  //! Gets whether the register is a VEC-H register (16-bit).
  ASMJIT_INLINE_NODEBUG constexpr bool isVecH() const noexcept { return baseSignature() == RegTraits<RegType::kARM_VecH>::kSignature; }
  //! Gets whether the register is a VEC-S register (32-bit).
  ASMJIT_INLINE_NODEBUG constexpr bool isVecS() const noexcept { return baseSignature() == RegTraits<RegType::kARM_VecS>::kSignature; }
  //! Gets whether the register is a VEC-D register (64-bit).
  ASMJIT_INLINE_NODEBUG constexpr bool isVecD() const noexcept { return baseSignature() == RegTraits<RegType::kARM_VecD>::kSignature; }
  //! Gets whether the register is a VEC-Q register (128-bit).
  ASMJIT_INLINE_NODEBUG constexpr bool isVecQ() const noexcept { return baseSignature() == RegTraits<RegType::kARM_VecV>::kSignature; }
  //! Gets whether the register is either VEC-D (64-bit) or VEC-Q (128-bit).
  ASMJIT_INLINE_NODEBUG constexpr bool isVecDOrQ() const noexcept { return uint32_t(type()) - uint32_t(RegType::kARM_VecD) <= 1u; }
  //! Gets whether the register is a VEC-V register (128-bit).
  ASMJIT_INLINE_NODEBUG constexpr bool isVecV() const noexcept { return baseSignature() == RegTraits<RegType::kARM_VecV>::kSignature; }

  template<RegType kRegType>
  ASMJIT_INLINE_NODEBUG void setRegT(uint32_t id) noexcept {
    setSignature(RegTraits<kRegType>::kSignature);
    setId(id);
  }

  ASMJIT_INLINE_NODEBUG void setTypeAndId(RegType type, uint32_t id) noexcept {
    setSignature(signatureOf(type));
    setId(id);
  }

  static ASMJIT_INLINE_NODEBUG RegGroup groupOf(RegType type) noexcept { return ArchTraits::byArch(Arch::kAArch64).regTypeToGroup(type); }
  static ASMJIT_INLINE_NODEBUG TypeId typeIdOf(RegType type) noexcept { return ArchTraits::byArch(Arch::kAArch64).regTypeToTypeId(type); }
  static ASMJIT_INLINE_NODEBUG OperandSignature signatureOf(RegType type) noexcept { return ArchTraits::byArch(Arch::kAArch64).regTypeToSignature(type); }

  template<RegType kRegType>
  static ASMJIT_INLINE_NODEBUG RegGroup groupOfT() noexcept { return RegTraits<kRegType>::kGroup; }

  template<RegType kRegType>
  static ASMJIT_INLINE_NODEBUG TypeId typeIdOfT() noexcept { return RegTraits<kRegType>::kTypeId; }

  template<RegType kRegType>
  static ASMJIT_INLINE_NODEBUG OperandSignature signatureOfT() noexcept { return OperandSignature{RegTraits<kRegType>::kSignature}; }

  static ASMJIT_INLINE_NODEBUG bool isGpW(const Operand_& op) noexcept { return op.as<Reg>().isGpW(); }
  static ASMJIT_INLINE_NODEBUG bool isGpX(const Operand_& op) noexcept { return op.as<Reg>().isGpX(); }
  static ASMJIT_INLINE_NODEBUG bool isVecB(const Operand_& op) noexcept { return op.as<Reg>().isVecB(); }
  static ASMJIT_INLINE_NODEBUG bool isVecH(const Operand_& op) noexcept { return op.as<Reg>().isVecH(); }
  static ASMJIT_INLINE_NODEBUG bool isVecS(const Operand_& op) noexcept { return op.as<Reg>().isVecS(); }
  static ASMJIT_INLINE_NODEBUG bool isVecD(const Operand_& op) noexcept { return op.as<Reg>().isVecD(); }
  static ASMJIT_INLINE_NODEBUG bool isVecQ(const Operand_& op) noexcept { return op.as<Reg>().isVecQ(); }
  static ASMJIT_INLINE_NODEBUG bool isVecV(const Operand_& op) noexcept { return op.as<Reg>().isVecV(); }

  static ASMJIT_INLINE_NODEBUG bool isGpW(const Operand_& op, uint32_t id) noexcept { return bool(unsigned(isGpW(op)) & unsigned(op.id() == id)); }
  static ASMJIT_INLINE_NODEBUG bool isGpX(const Operand_& op, uint32_t id) noexcept { return bool(unsigned(isGpX(op)) & unsigned(op.id() == id)); }
  static ASMJIT_INLINE_NODEBUG bool isVecB(const Operand_& op, uint32_t id) noexcept { return bool(unsigned(isVecB(op)) & unsigned(op.id() == id)); }
  static ASMJIT_INLINE_NODEBUG bool isVecH(const Operand_& op, uint32_t id) noexcept { return bool(unsigned(isVecH(op)) & unsigned(op.id() == id)); }
  static ASMJIT_INLINE_NODEBUG bool isVecS(const Operand_& op, uint32_t id) noexcept { return bool(unsigned(isVecS(op)) & unsigned(op.id() == id)); }
  static ASMJIT_INLINE_NODEBUG bool isVecD(const Operand_& op, uint32_t id) noexcept { return bool(unsigned(isVecD(op)) & unsigned(op.id() == id)); }
  static ASMJIT_INLINE_NODEBUG bool isVecQ(const Operand_& op, uint32_t id) noexcept { return bool(unsigned(isVecQ(op)) & unsigned(op.id() == id)); }
  static ASMJIT_INLINE_NODEBUG bool isVecV(const Operand_& op, uint32_t id) noexcept { return bool(unsigned(isVecV(op)) & unsigned(op.id() == id)); }
};

//! Vector register base - a common base for both AArch32 & AArch64 vector register.
class BaseVec : public Reg {
public:
  ASMJIT_DEFINE_ABSTRACT_REG(BaseVec, Reg)
};

//! Vector register (ARM).
class Vec : public BaseVec {
public:
  ASMJIT_DEFINE_ABSTRACT_REG(Vec, BaseVec)

  //! Additional signature bits used by arm::Vec.
  enum AdditionalBits : uint32_t {
    // Register element type (3 bits).
    // |........|........|.XXX....|........|
    kSignatureRegElementTypeShift = 12,
    kSignatureRegElementTypeMask = 0x07 << kSignatureRegElementTypeShift,

    // Register has element index (1 bit).
    // |........|........|X.......|........|
    kSignatureRegElementFlagShift = 15,
    kSignatureRegElementFlagMask = 0x01 << kSignatureRegElementFlagShift,

    // Register element index (4 bits).
    // |........|....XXXX|........|........|
    kSignatureRegElementIndexShift = 16,
    kSignatureRegElementIndexMask = 0x0F << kSignatureRegElementIndexShift
  };

  //! Element type (AArch64 only).
  enum ElementType : uint32_t {
    //! No element type specified.
    kElementTypeNone = 0,
    //! Byte elements (B8 or B16).
    kElementTypeB,
    //! Halfword elements (H4 or H8).
    kElementTypeH,
    //! Singleword elements (S2 or S4).
    kElementTypeS,
    //! Doubleword elements (D2).
    kElementTypeD,
    //! Byte elements grouped by 4 bytes (B4).
    //!
    //! \note This element-type is only used by few instructions.
    kElementTypeB4,
    //! Halfword elements grouped by 2 halfwords (H2).
    //!
    //! \note This element-type is only used by few instructions.
    kElementTypeH2,

    //! Count of element types.
    kElementTypeCount
  };

  //! \cond
  //! Shortcuts.
  enum SignatureReg : uint32_t {
    kSignatureElementB  = kElementTypeB  << kSignatureRegElementTypeShift,
    kSignatureElementH  = kElementTypeH  << kSignatureRegElementTypeShift,
    kSignatureElementS  = kElementTypeS  << kSignatureRegElementTypeShift,
    kSignatureElementD  = kElementTypeD  << kSignatureRegElementTypeShift,
    kSignatureElementB4 = kElementTypeB4 << kSignatureRegElementTypeShift,
    kSignatureElementH2 = kElementTypeH2 << kSignatureRegElementTypeShift
  };
  //! \endcond

  //! Returns whether the register has associated an element type.
  ASMJIT_INLINE_NODEBUG constexpr bool hasElementType() const noexcept { return _signature.hasField<kSignatureRegElementTypeMask>(); }
  //! Returns whether the register has element index (it's an element index access).
  ASMJIT_INLINE_NODEBUG constexpr bool hasElementIndex() const noexcept { return _signature.hasField<kSignatureRegElementFlagMask>(); }
  //! Returns whether the register has element type or element index (or both).
  ASMJIT_INLINE_NODEBUG constexpr bool hasElementTypeOrIndex() const noexcept { return _signature.hasField<kSignatureRegElementTypeMask | kSignatureRegElementFlagMask>(); }

  //! Returns element type of the register.
  ASMJIT_INLINE_NODEBUG constexpr uint32_t elementType() const noexcept { return _signature.getField<kSignatureRegElementTypeMask>(); }
  //! Sets element type of the register to `elementType`.
  ASMJIT_INLINE_NODEBUG void setElementType(uint32_t elementType) noexcept { _signature.setField<kSignatureRegElementTypeMask>(elementType); }
  //! Resets element type to none.
  ASMJIT_INLINE_NODEBUG void resetElementType() noexcept { _signature.setField<kSignatureRegElementTypeMask>(0); }

  //! Returns element index of the register.
  ASMJIT_INLINE_NODEBUG constexpr uint32_t elementIndex() const noexcept { return _signature.getField<kSignatureRegElementIndexMask>(); }
  //! Sets element index of the register to `elementType`.
  ASMJIT_INLINE_NODEBUG void setElementIndex(uint32_t elementIndex) noexcept {
    _signature |= kSignatureRegElementFlagMask;
    _signature.setField<kSignatureRegElementIndexMask>(elementIndex);
  }
  //! Resets element index of the register.
  ASMJIT_INLINE_NODEBUG void resetElementIndex() noexcept {
    _signature &= ~(kSignatureRegElementFlagMask | kSignatureRegElementIndexMask);
  }

  ASMJIT_INLINE_NODEBUG constexpr bool isVecB8() const noexcept { return _signature.subset(kBaseSignatureMask | kSignatureRegElementTypeMask) == (RegTraits<RegType::kARM_VecD>::kSignature | kSignatureElementB); }
  ASMJIT_INLINE_NODEBUG constexpr bool isVecH4() const noexcept { return _signature.subset(kBaseSignatureMask | kSignatureRegElementTypeMask) == (RegTraits<RegType::kARM_VecD>::kSignature | kSignatureElementH); }
  ASMJIT_INLINE_NODEBUG constexpr bool isVecS2() const noexcept { return _signature.subset(kBaseSignatureMask | kSignatureRegElementTypeMask) == (RegTraits<RegType::kARM_VecD>::kSignature | kSignatureElementS); }
  ASMJIT_INLINE_NODEBUG constexpr bool isVecD1() const noexcept { return _signature.subset(kBaseSignatureMask | kSignatureRegElementTypeMask) == (RegTraits<RegType::kARM_VecD>::kSignature); }

  ASMJIT_INLINE_NODEBUG constexpr bool isVecB16() const noexcept { return _signature.subset(kBaseSignatureMask | kSignatureRegElementTypeMask) == (RegTraits<RegType::kARM_VecV>::kSignature | kSignatureElementB); }
  ASMJIT_INLINE_NODEBUG constexpr bool isVecH8() const noexcept { return _signature.subset(kBaseSignatureMask | kSignatureRegElementTypeMask) == (RegTraits<RegType::kARM_VecV>::kSignature | kSignatureElementH); }
  ASMJIT_INLINE_NODEBUG constexpr bool isVecS4() const noexcept { return _signature.subset(kBaseSignatureMask | kSignatureRegElementTypeMask) == (RegTraits<RegType::kARM_VecV>::kSignature | kSignatureElementS); }
  ASMJIT_INLINE_NODEBUG constexpr bool isVecD2() const noexcept { return _signature.subset(kBaseSignatureMask | kSignatureRegElementTypeMask) == (RegTraits<RegType::kARM_VecV>::kSignature | kSignatureElementD); }
  ASMJIT_INLINE_NODEBUG constexpr bool isVecB4x4() const noexcept { return _signature.subset(kBaseSignatureMask | kSignatureRegElementTypeMask) == (RegTraits<RegType::kARM_VecV>::kSignature | kSignatureElementB4); }
  ASMJIT_INLINE_NODEBUG constexpr bool isVecH2x4() const noexcept { return _signature.subset(kBaseSignatureMask | kSignatureRegElementTypeMask) == (RegTraits<RegType::kARM_VecV>::kSignature | kSignatureElementH2); }

  //! Creates a cloned register with element access.
  ASMJIT_INLINE_NODEBUG Vec at(uint32_t elementIndex) const noexcept {
    return Vec((signature() & ~kSignatureRegElementIndexMask) | (elementIndex << kSignatureRegElementIndexShift) | kSignatureRegElementFlagMask, id());
  }

  //! Cast this register to an 8-bit B register (AArch64 only).
  ASMJIT_INLINE_NODEBUG VecB b() const noexcept;
  //! Cast this register to a 16-bit H register (AArch64 only).
  ASMJIT_INLINE_NODEBUG VecH h() const noexcept;
  //! Cast this register to a 32-bit S register.
  ASMJIT_INLINE_NODEBUG VecS s() const noexcept;
  //! Cast this register to a 64-bit D register.
  ASMJIT_INLINE_NODEBUG VecD d() const noexcept;
  //! Cast this register to a 128-bit Q register.
  ASMJIT_INLINE_NODEBUG VecV q() const noexcept;
  //! Cast this register to a 128-bit V register.
  ASMJIT_INLINE_NODEBUG VecV v() const noexcept;

  //! Cast this register to a 128-bit V.B[elementIndex] register.
  ASMJIT_INLINE_NODEBUG VecV b(uint32_t elementIndex) const noexcept;
  //! Cast this register to a 128-bit V.H[elementIndex] register.
  ASMJIT_INLINE_NODEBUG VecV h(uint32_t elementIndex) const noexcept;
  //! Cast this register to a 128-bit V.S[elementIndex] register.
  ASMJIT_INLINE_NODEBUG VecV s(uint32_t elementIndex) const noexcept;
  //! Cast this register to a 128-bit V.D[elementIndex] register.
  ASMJIT_INLINE_NODEBUG VecV d(uint32_t elementIndex) const noexcept;
  //! Cast this register to a 128-bit V.H2[elementIndex] register.
  ASMJIT_INLINE_NODEBUG VecV h2(uint32_t elementIndex) const noexcept;
  //! Cast this register to a 128-bit V.B4[elementIndex] register.
  ASMJIT_INLINE_NODEBUG VecV b4(uint32_t elementIndex) const noexcept;

  //! Cast this register to V.8B.
  ASMJIT_INLINE_NODEBUG VecD b8() const noexcept;
  //! Cast this register to V.16B.
  ASMJIT_INLINE_NODEBUG VecV b16() const noexcept;
  //! Cast this register to V.2H.
  ASMJIT_INLINE_NODEBUG VecS h2() const noexcept;
  //! Cast this register to V.4H.
  ASMJIT_INLINE_NODEBUG VecD h4() const noexcept;
  //! Cast this register to V.8H.
  ASMJIT_INLINE_NODEBUG VecV h8() const noexcept;
  //! Cast this register to V.2S.
  ASMJIT_INLINE_NODEBUG VecD s2() const noexcept;
  //! Cast this register to V.4S.
  ASMJIT_INLINE_NODEBUG VecV s4() const noexcept;
  //! Cast this register to V.2D.
  ASMJIT_INLINE_NODEBUG VecV d2() const noexcept;

  static ASMJIT_INLINE_NODEBUG constexpr OperandSignature _makeElementAccessSignature(uint32_t elementType, uint32_t elementIndex) noexcept {
    return OperandSignature{
      uint32_t(RegTraits<RegType::kARM_VecV>::kSignature)      |
      uint32_t(kSignatureRegElementFlagMask)                   |
      uint32_t(elementType << kSignatureRegElementTypeShift)   |
      uint32_t(elementIndex << kSignatureRegElementIndexShift)};
  }
};

//! 8-bit view (S) of VFP/SIMD register.
class VecB : public Vec { ASMJIT_DEFINE_FINAL_REG(VecB, Vec, RegTraits<RegType::kARM_VecB>) };
//! 16-bit view (S) of VFP/SIMD register.
class VecH : public Vec { ASMJIT_DEFINE_FINAL_REG(VecH, Vec, RegTraits<RegType::kARM_VecH>) };
//! 32-bit view (S) of VFP/SIMD register.
class VecS : public Vec { ASMJIT_DEFINE_FINAL_REG(VecS, Vec, RegTraits<RegType::kARM_VecS>) };
//! 64-bit view (D) of VFP/SIMD register.
class VecD : public Vec { ASMJIT_DEFINE_FINAL_REG(VecD, Vec, RegTraits<RegType::kARM_VecD>) };
//! 128-bit vector register (Q or V).
class VecV : public Vec { ASMJIT_DEFINE_FINAL_REG(VecV, Vec, RegTraits<RegType::kARM_VecV>) };

ASMJIT_INLINE_NODEBUG VecB Vec::b() const noexcept { return VecB(id()); }
ASMJIT_INLINE_NODEBUG VecH Vec::h() const noexcept { return VecH(id()); }
ASMJIT_INLINE_NODEBUG VecS Vec::s() const noexcept { return VecS(id()); }
ASMJIT_INLINE_NODEBUG VecD Vec::d() const noexcept { return VecD(id()); }
ASMJIT_INLINE_NODEBUG VecV Vec::q() const noexcept { return VecV(id()); }
ASMJIT_INLINE_NODEBUG VecV Vec::v() const noexcept { return VecV(id()); }

ASMJIT_INLINE_NODEBUG VecV Vec::b(uint32_t elementIndex) const noexcept { return VecV(_makeElementAccessSignature(kElementTypeB, elementIndex), id()); }
ASMJIT_INLINE_NODEBUG VecV Vec::h(uint32_t elementIndex) const noexcept { return VecV(_makeElementAccessSignature(kElementTypeH, elementIndex), id()); }
ASMJIT_INLINE_NODEBUG VecV Vec::s(uint32_t elementIndex) const noexcept { return VecV(_makeElementAccessSignature(kElementTypeS, elementIndex), id()); }
ASMJIT_INLINE_NODEBUG VecV Vec::d(uint32_t elementIndex) const noexcept { return VecV(_makeElementAccessSignature(kElementTypeD, elementIndex), id()); }
ASMJIT_INLINE_NODEBUG VecV Vec::h2(uint32_t elementIndex) const noexcept { return VecV(_makeElementAccessSignature(kElementTypeH2, elementIndex), id()); }
ASMJIT_INLINE_NODEBUG VecV Vec::b4(uint32_t elementIndex) const noexcept { return VecV(_makeElementAccessSignature(kElementTypeB4, elementIndex), id()); }

ASMJIT_INLINE_NODEBUG VecD Vec::b8() const noexcept { return VecD(OperandSignature{VecD::kSignature | kSignatureElementB}, id()); }
ASMJIT_INLINE_NODEBUG VecS Vec::h2() const noexcept { return VecS(OperandSignature{VecS::kSignature | kSignatureElementH}, id()); }
ASMJIT_INLINE_NODEBUG VecD Vec::h4() const noexcept { return VecD(OperandSignature{VecD::kSignature | kSignatureElementH}, id()); }
ASMJIT_INLINE_NODEBUG VecD Vec::s2() const noexcept { return VecD(OperandSignature{VecD::kSignature | kSignatureElementS}, id()); }
ASMJIT_INLINE_NODEBUG VecV Vec::b16() const noexcept { return VecV(OperandSignature{VecV::kSignature | kSignatureElementB}, id()); }
ASMJIT_INLINE_NODEBUG VecV Vec::h8() const noexcept { return VecV(OperandSignature{VecV::kSignature | kSignatureElementH}, id()); }
ASMJIT_INLINE_NODEBUG VecV Vec::s4() const noexcept { return VecV(OperandSignature{VecV::kSignature | kSignatureElementS}, id()); }
ASMJIT_INLINE_NODEBUG VecV Vec::d2() const noexcept { return VecV(OperandSignature{VecV::kSignature | kSignatureElementD}, id()); }

#ifndef _DOXYGEN
namespace regs {
#endif

//! Creates a 32-bit S register operand (ARM/AArch64).
static ASMJIT_INLINE_NODEBUG constexpr VecS s(uint32_t id) noexcept { return VecS(id); }
//! Creates a 64-bit D register operand (ARM/AArch64).
static ASMJIT_INLINE_NODEBUG constexpr VecD d(uint32_t id) noexcept { return VecD(id); }
//! Creates a 1282-bit V register operand (ARM/AArch64).
static ASMJIT_INLINE_NODEBUG constexpr VecV v(uint32_t id) noexcept { return VecV(id); }

#ifndef _DOXYGEN
} // {regs}

// Make `arm::regs` accessible through `arm` namespace as well.
using namespace regs;
#endif

//! Memory operand (ARM).
class Mem : public BaseMem {
public:
  //! \cond INTERNAL
  //! Additional bits of operand's signature used by `arm::Mem`.
  enum AdditionalBits : uint32_t {
    // Index shift value (5 bits).
    // |........|.....XXX|XX......|........|
    kSignatureMemShiftValueShift = 14,
    kSignatureMemShiftValueMask = 0x1Fu << kSignatureMemShiftValueShift,

    // Index shift operation (4 bits).
    // |........|XXXX....|........|........|
    kSignatureMemShiftOpShift = 20,
    kSignatureMemShiftOpMask = 0x0Fu << kSignatureMemShiftOpShift,

    // Offset mode type (2 bits).
    // |......XX|........|........|........|
    kSignatureMemOffsetModeShift = 24,
    kSignatureMemOffsetModeMask = 0x03u << kSignatureMemOffsetModeShift
  };
  //! \endcond

  //! \name Construction & Destruction
  //! \{

  //! Construct a default `Mem` operand, that points to [0].
  ASMJIT_INLINE_NODEBUG constexpr Mem() noexcept
    : BaseMem() {}

  ASMJIT_INLINE_NODEBUG constexpr Mem(const Mem& other) noexcept
    : BaseMem(other) {}

  ASMJIT_INLINE_NODEBUG explicit Mem(Globals::NoInit_) noexcept
    : BaseMem(Globals::NoInit) {}

  ASMJIT_INLINE_NODEBUG constexpr Mem(const Signature& signature, uint32_t baseId, uint32_t indexId, int32_t offset) noexcept
    : BaseMem(signature, baseId, indexId, offset) {}

  ASMJIT_INLINE_NODEBUG constexpr explicit Mem(const Label& base, int32_t off = 0, Signature signature = Signature{0}) noexcept
    : BaseMem(Signature::fromOpType(OperandType::kMem) |
              Signature::fromMemBaseType(RegType::kLabelTag) |
              signature, base.id(), 0, off) {}

  ASMJIT_INLINE_NODEBUG constexpr explicit Mem(const BaseReg& base, int32_t off = 0, Signature signature = Signature{0}) noexcept
    : BaseMem(Signature::fromOpType(OperandType::kMem) |
              Signature::fromMemBaseType(base.type()) |
              signature, base.id(), 0, off) {}

  ASMJIT_INLINE_NODEBUG constexpr Mem(const BaseReg& base, const BaseReg& index, Signature signature = Signature{0}) noexcept
    : BaseMem(Signature::fromOpType(OperandType::kMem) |
              Signature::fromMemBaseType(base.type()) |
              Signature::fromMemIndexType(index.type()) |
              signature, base.id(), index.id(), 0) {}

  ASMJIT_INLINE_NODEBUG constexpr Mem(const BaseReg& base, const BaseReg& index, const Shift& shift, Signature signature = Signature{0}) noexcept
    : BaseMem(Signature::fromOpType(OperandType::kMem) |
              Signature::fromMemBaseType(base.type()) |
              Signature::fromMemIndexType(index.type()) |
              Signature::fromValue<kSignatureMemShiftOpMask>(uint32_t(shift.op())) |
              Signature::fromValue<kSignatureMemShiftValueMask>(shift.value()) |
              signature, base.id(), index.id(), 0) {}

  ASMJIT_INLINE_NODEBUG constexpr Mem(uint64_t base, Signature signature = Signature{0}) noexcept
    : BaseMem(Signature::fromOpType(OperandType::kMem) |
              signature, uint32_t(base >> 32), 0, int32_t(uint32_t(base & 0xFFFFFFFFu))) {}

  //! \}

  //! \name Overloaded Operators
  //! \{

  ASMJIT_INLINE_NODEBUG Mem& operator=(const Mem& other) noexcept = default;

  //! \}

  //! \name Clone
  //! \{

  //! Clones the memory operand.
  ASMJIT_INLINE_NODEBUG constexpr Mem clone() const noexcept { return Mem(*this); }

  //! Gets new memory operand adjusted by `off`.
  ASMJIT_INLINE_NODEBUG Mem cloneAdjusted(int64_t off) const noexcept {
    Mem result(*this);
    result.addOffset(off);
    return result;
  }

  //! Clones the memory operand and makes it pre-index.
  ASMJIT_INLINE_NODEBUG Mem pre() const noexcept {
    Mem result(*this);
    result.setOffsetMode(OffsetMode::kPreIndex);
    return result;
  }

  //! Clones the memory operand, applies a given offset `off` and makes it pre-index.
  ASMJIT_INLINE_NODEBUG Mem pre(int64_t off) const noexcept {
    Mem result(*this);
    result.setOffsetMode(OffsetMode::kPreIndex);
    result.addOffset(off);
    return result;
  }

  //! Clones the memory operand and makes it post-index.
  ASMJIT_INLINE_NODEBUG Mem post() const noexcept {
    Mem result(*this);
    result.setOffsetMode(OffsetMode::kPostIndex);
    return result;
  }

  //! Clones the memory operand, applies a given offset `off` and makes it post-index.
  ASMJIT_INLINE_NODEBUG Mem post(int64_t off) const noexcept {
    Mem result(*this);
    result.setOffsetMode(OffsetMode::kPostIndex);
    result.addOffset(off);
    return result;
  }

  //! \}

  //! \name Base & Index
  //! \{

  //! Converts memory `baseType` and `baseId` to `arm::Reg` instance.
  //!
  //! The memory must have a valid base register otherwise the result will be wrong.
  ASMJIT_INLINE_NODEBUG Reg baseReg() const noexcept { return Reg::fromTypeAndId(baseType(), baseId()); }

  //! Converts memory `indexType` and `indexId` to `arm::Reg` instance.
  //!
  //! The memory must have a valid index register otherwise the result will be wrong.
  ASMJIT_INLINE_NODEBUG Reg indexReg() const noexcept { return Reg::fromTypeAndId(indexType(), indexId()); }

  using BaseMem::setIndex;

  ASMJIT_INLINE_NODEBUG void setIndex(const BaseReg& index, uint32_t shift) noexcept {
    setIndex(index);
    setShift(shift);
  }

  ASMJIT_INLINE_NODEBUG void setIndex(const BaseReg& index, Shift shift) noexcept {
    setIndex(index);
    setShift(shift);
  }

  //! \}

  //! \name ARM Specific Features
  //! \{

  //! Gets offset mode.
  ASMJIT_INLINE_NODEBUG constexpr OffsetMode offsetMode() const noexcept { return OffsetMode(_signature.getField<kSignatureMemOffsetModeMask>()); }
  //! Sets offset mode to `mode`.
  ASMJIT_INLINE_NODEBUG void setOffsetMode(OffsetMode mode) noexcept { _signature.setField<kSignatureMemOffsetModeMask>(uint32_t(mode)); }
  //! Resets offset mode to default (fixed offset, without write-back).
  ASMJIT_INLINE_NODEBUG void resetOffsetMode() noexcept { _signature.setField<kSignatureMemOffsetModeMask>(uint32_t(OffsetMode::kFixed)); }

  //! Tests whether the current memory offset mode is fixed (see \ref OffsetMode::kFixed).
  ASMJIT_INLINE_NODEBUG constexpr bool isFixedOffset() const noexcept { return offsetMode() == OffsetMode::kFixed; }
  //! Tests whether the current memory offset mode is either pre-index or post-index (write-back is used).
  ASMJIT_INLINE_NODEBUG constexpr bool isPreOrPost() const noexcept { return offsetMode() != OffsetMode::kFixed; }
  //! Tests whether the current memory offset mode is pre-index (write-back is used).
  ASMJIT_INLINE_NODEBUG constexpr bool isPreIndex() const noexcept { return offsetMode() == OffsetMode::kPreIndex; }
  //! Tests whether the current memory offset mode is post-index (write-back is used).
  ASMJIT_INLINE_NODEBUG constexpr bool isPostIndex() const noexcept { return offsetMode() == OffsetMode::kPostIndex; }

  //! Sets offset mode of this memory operand to pre-index (write-back is used).
  ASMJIT_INLINE_NODEBUG void makePreIndex() noexcept { setOffsetMode(OffsetMode::kPreIndex); }
  //! Sets offset mode of this memory operand to post-index (write-back is used).
  ASMJIT_INLINE_NODEBUG void makePostIndex() noexcept { setOffsetMode(OffsetMode::kPostIndex); }

  //! Gets shift operation that is used by index register.
  ASMJIT_INLINE_NODEBUG constexpr ShiftOp shiftOp() const noexcept { return ShiftOp(_signature.getField<kSignatureMemShiftOpMask>()); }
  //! Sets shift operation that is used by index register.
  ASMJIT_INLINE_NODEBUG void setShiftOp(ShiftOp sop) noexcept { _signature.setField<kSignatureMemShiftOpMask>(uint32_t(sop)); }
  //! Resets shift operation that is used by index register to LSL (default value).
  ASMJIT_INLINE_NODEBUG void resetShiftOp() noexcept { _signature.setField<kSignatureMemShiftOpMask>(uint32_t(ShiftOp::kLSL)); }

  //! Gets whether the memory operand has shift (aka scale) constant.
  ASMJIT_INLINE_NODEBUG constexpr bool hasShift() const noexcept { return _signature.hasField<kSignatureMemShiftValueMask>(); }
  //! Gets the memory operand's shift (aka scale) constant.
  ASMJIT_INLINE_NODEBUG constexpr uint32_t shift() const noexcept { return _signature.getField<kSignatureMemShiftValueMask>(); }
  //! Sets the memory operand's shift (aka scale) constant.
  ASMJIT_INLINE_NODEBUG void setShift(uint32_t shift) noexcept { _signature.setField<kSignatureMemShiftValueMask>(shift); }

  //! Sets the memory operand's shift and shift operation.
  ASMJIT_INLINE_NODEBUG void setShift(Shift shift) noexcept {
    _signature.setField<kSignatureMemShiftOpMask>(uint32_t(shift.op()));
    _signature.setField<kSignatureMemShiftValueMask>(shift.value());
  }

  //! Resets the memory operand's shift (aka scale) constant to zero.
  ASMJIT_INLINE_NODEBUG void resetShift() noexcept { _signature.setField<kSignatureMemShiftValueMask>(0); }

  //! \}
};

//! \name Shift Operation Construction
//! \{

//! Constructs a `LSL #value` shift (logical shift left).
static ASMJIT_INLINE_NODEBUG constexpr Shift lsl(uint32_t value) noexcept { return Shift(ShiftOp::kLSL, value); }
//! Constructs a `LSR #value` shift (logical shift right).
static ASMJIT_INLINE_NODEBUG constexpr Shift lsr(uint32_t value) noexcept { return Shift(ShiftOp::kLSR, value); }
//! Constructs a `ASR #value` shift (arithmetic shift right).
static ASMJIT_INLINE_NODEBUG constexpr Shift asr(uint32_t value) noexcept { return Shift(ShiftOp::kASR, value); }
//! Constructs a `ROR #value` shift (rotate right).
static ASMJIT_INLINE_NODEBUG constexpr Shift ror(uint32_t value) noexcept { return Shift(ShiftOp::kROR, value); }
//! Constructs a `RRX` shift (rotate with carry by 1).
static ASMJIT_INLINE_NODEBUG constexpr Shift rrx() noexcept { return Shift(ShiftOp::kRRX, 0); }
//! Constructs a `MSL #value` shift (logical shift left filling ones).
static ASMJIT_INLINE_NODEBUG constexpr Shift msl(uint32_t value) noexcept { return Shift(ShiftOp::kMSL, value); }

//! \}

//! \name Memory Operand Construction
//! \{

//! Creates `[base]` absolute memory operand (AArch32 or AArch64).
//!
//! \note The concept of absolute memory operands doesn't exist on ARM, the ISA only provides PC relative addressing.
//! Absolute memory operands can only be used if it's known that the PC relative offset is encodable and that it
//! would be within the limits. Absolute address is also often output from disassemblers, so AsmJit supports it to
//! make it possible to assemble such output back.
static ASMJIT_INLINE_NODEBUG constexpr Mem ptr(uint64_t base) noexcept { return Mem(base); }

//! \}

//! \}

ASMJIT_END_SUB_NAMESPACE

//! \cond INTERNAL
ASMJIT_BEGIN_NAMESPACE
ASMJIT_DEFINE_TYPE_ID(arm::VecS, TypeId::kFloat32x1);
ASMJIT_DEFINE_TYPE_ID(arm::VecD, TypeId::kFloat64x1);
ASMJIT_DEFINE_TYPE_ID(arm::VecV, TypeId::kInt32x4);
ASMJIT_END_NAMESPACE
//! \endcond

#endif // ASMJIT_ARM_ARMOPERAND_H_INCLUDED
