// This file is part of AsmJit project <https://asmjit.com>
//
// See asmjit.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_LA_LAOPERAND_H_INCLUDED
#define ASMJIT_LA_LAOPERAND_H_INCLUDED

#include "../core/archtraits.h"
#include "../core/operand.h"
#include "../core/type.h"
#include "../loongarch/laglobals.h"

ASMJIT_BEGIN_SUB_NAMESPACE(la)

//! \addtogroup asmjit_la
//! \{

class Reg;
class Mem;

//! Register traits.
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
ASMJIT_DEFINE_REG_TRAITS(RegType::kLA_GpW       , RegGroup::kGp          , 4 , TypeId::kInt32   );
ASMJIT_DEFINE_REG_TRAITS(RegType::kLA_GpX       , RegGroup::kGp          , 8 , TypeId::kInt64   );
ASMJIT_DEFINE_REG_TRAITS(RegType::kLA_VecB      , RegGroup::kVec         , 1 , TypeId::kVoid    );
ASMJIT_DEFINE_REG_TRAITS(RegType::kLA_VecH      , RegGroup::kVec         , 2 , TypeId::kVoid    );
ASMJIT_DEFINE_REG_TRAITS(RegType::kLA_VecS      , RegGroup::kVec         , 4 , TypeId::kInt32x1 );
ASMJIT_DEFINE_REG_TRAITS(RegType::kLA_VecD      , RegGroup::kVec         , 8 , TypeId::kInt32x2 );
ASMJIT_DEFINE_REG_TRAITS(RegType::kLA_VecQ      , RegGroup::kVec         , 16, TypeId::kInt32x4 );
ASMJIT_DEFINE_REG_TRAITS(RegType::kLA_PC        , RegGroup::kPC          , 8 , TypeId::kInt64   );
//! \endcond

//! Register operand.
class Reg : public BaseReg {
public:
  ASMJIT_DEFINE_ABSTRACT_REG(Reg, BaseReg)

  //! Gets whether the register is either `R` or `W` register (32-bit).
  ASMJIT_INLINE_NODEBUG constexpr bool isGpR() const noexcept { return baseSignature() == RegTraits<RegType::kLA_GpW>::kSignature; }
  //! Gets whether the register is either `R` or `W` register (32-bit).
  ASMJIT_INLINE_NODEBUG constexpr bool isGpW() const noexcept { return baseSignature() == RegTraits<RegType::kLA_GpW>::kSignature; }
  //! Gets whether the register is an `X` register (64-bit).
  ASMJIT_INLINE_NODEBUG constexpr bool isGpX() const noexcept { return baseSignature() == RegTraits<RegType::kLA_GpX>::kSignature; }

  //! Gets whether the register is a VEC-B register (8-bit).
  ASMJIT_INLINE_NODEBUG constexpr bool isVecB() const noexcept { return baseSignature() == RegTraits<RegType::kLA_VecB>::kSignature; }
  //! Gets whether the register is a VEC-H register (16-bit).
  ASMJIT_INLINE_NODEBUG constexpr bool isVecH() const noexcept { return baseSignature() == RegTraits<RegType::kLA_VecH>::kSignature; }
  //! Gets whether the register is a VEC-S register (32-bit).
  ASMJIT_INLINE_NODEBUG constexpr bool isVecS() const noexcept { return baseSignature() == RegTraits<RegType::kLA_VecS>::kSignature; }
  //! Gets whether the register is a VEC-D register (64-bit).
  ASMJIT_INLINE_NODEBUG constexpr bool isVecD() const noexcept { return baseSignature() == RegTraits<RegType::kLA_VecD>::kSignature; }
  //! Gets whether the register is a VEC-Q register (128-bit).
  ASMJIT_INLINE_NODEBUG constexpr bool isVecQ() const noexcept { return baseSignature() == RegTraits<RegType::kLA_VecV>::kSignature; }
  //! Gets whether the register is either VEC-D (64-bit) or VEC-Q (128-bit).
  ASMJIT_INLINE_NODEBUG constexpr bool isVecDOrQ() const noexcept { return uint32_t(type()) - uint32_t(RegType::kLA_VecD) <= 1u; }
  //! Gets whether the register is a VEC-V register (128-bit).
  ASMJIT_INLINE_NODEBUG constexpr bool isVecV() const noexcept { return baseSignature() == RegTraits<RegType::kLA_VecV>::kSignature; }

  //! Gets whether the register is an 8-bit vector register or view, alias if \ref isVecB().
  ASMJIT_INLINE_NODEBUG constexpr bool isVec8() const noexcept { return baseSignature() == RegTraits<RegType::kLA_VecB>::kSignature; }
  //! Gets whether the register is a 16-bit vector register or view, alias if \ref isVecH().
  ASMJIT_INLINE_NODEBUG constexpr bool isVec16() const noexcept { return baseSignature() == RegTraits<RegType::kLA_VecH>::kSignature; }
  //! Gets whether the register is a 32-bit vector register or view, alias if \ref isVecS().
  ASMJIT_INLINE_NODEBUG constexpr bool isVec32() const noexcept { return baseSignature() == RegTraits<RegType::kLA_VecS>::kSignature; }
  //! Gets whether the register is a 64-bit vector register or view, alias if \ref isVecD().
  ASMJIT_INLINE_NODEBUG constexpr bool isVec64() const noexcept { return baseSignature() == RegTraits<RegType::kLA_VecD>::kSignature; }
  //! Gets whether the register is a 128-bit vector register or view, alias if \ref isVecQ().
  ASMJIT_INLINE_NODEBUG constexpr bool isVec128() const noexcept { return baseSignature() == RegTraits<RegType::kLA_VecV>::kSignature; }

  template<RegType kRegType>
  ASMJIT_INLINE_NODEBUG void setRegT(uint32_t id) noexcept {
    setSignature(RegTraits<kRegType>::kSignature);
    setId(id);
  }

  ASMJIT_INLINE_NODEBUG void setTypeAndId(RegType type, uint32_t id) noexcept {
    setSignature(signatureOf(type));
    setId(id);
  }

  static ASMJIT_INLINE_NODEBUG RegGroup groupOf(RegType type) noexcept { return ArchTraits::byArch(Arch::kLOONGARCH64).regTypeToGroup(type); }
  static ASMJIT_INLINE_NODEBUG TypeId typeIdOf(RegType type) noexcept { return ArchTraits::byArch(Arch::kLOONGARCH64).regTypeToTypeId(type); }
  static ASMJIT_INLINE_NODEBUG OperandSignature signatureOf(RegType type) noexcept { return ArchTraits::byArch(Arch::kLOONGARCH64).regTypeToSignature(type); }

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

//! Vector register base.
class BaseVec : public Reg {
public:
  ASMJIT_DEFINE_ABSTRACT_REG(BaseVec, Reg)

  //! Additional signature bits used by a vector register.
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

  //! Returns whether the register has element index (it's an element index access).
  ASMJIT_INLINE_NODEBUG constexpr bool hasElementIndex() const noexcept { return _signature.hasField<kSignatureRegElementFlagMask>(); }
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
};

//! Memory operand.
class Mem : public BaseMem {
public:
  //! \cond INTERNAL
  //! Additional bits of operand's signature used by `la::Mem`.
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

  ASMJIT_INLINE_NODEBUG constexpr explicit Mem(uint64_t base, Signature signature = Signature{0}) noexcept
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

  //! \}

  //! \name Base & Index
  //! \{

  //! Converts memory `baseType` and `baseId` to `la::Reg` instance.
  //!
  //! The memory must have a valid base register otherwise the result will be wrong.
  ASMJIT_INLINE_NODEBUG Reg baseReg() const noexcept { return Reg::fromTypeAndId(baseType(), baseId()); }

  //! Converts memory `indexType` and `indexId` to `la::Reg` instance.
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

  //! \name LA Specific Features
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

  //! Gets shift operation that is used by index register.
  ASMJIT_INLINE_NODEBUG constexpr ShiftOp shiftOp() const noexcept { return ShiftOp(_signature.getField<kSignatureMemShiftOpMask>()); }
  //! Sets shift operation that is used by index register.
  ASMJIT_INLINE_NODEBUG void setShiftOp(ShiftOp sop) noexcept { _signature.setField<kSignatureMemShiftOpMask>(uint32_t(sop)); }
  //! Resets shift operation that is used by index register to LSL (default value).
  ASMJIT_INLINE_NODEBUG void resetShiftOp() noexcept { _signature.setField<kSignatureMemShiftOpMask>(uint32_t(ShiftOp::kSLL)); }

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
static ASMJIT_INLINE_NODEBUG constexpr Shift sll(uint32_t value) noexcept { return Shift(ShiftOp::kSLL, value); }
//! Constructs a `LSR #value` shift (logical shift right).
static ASMJIT_INLINE_NODEBUG constexpr Shift srl(uint32_t value) noexcept { return Shift(ShiftOp::kSRL, value); }
//! Constructs a `ASR #value` shift (arithmetic shift right).
static ASMJIT_INLINE_NODEBUG constexpr Shift sra(uint32_t value) noexcept { return Shift(ShiftOp::kSRA, value); }
//! Constructs a `ROR #value` shift (rotate right).
static ASMJIT_INLINE_NODEBUG constexpr Shift rort(uint32_t value) noexcept { return Shift(ShiftOp::kRORT, value); }

//! \}

//! \name Memory Operand Construction
//! \{

//! Creates `[base]` absolute memory operand.
//!
//! \note The concept of absolute memory operands doesn't exist on LA, the ISA only provides PC relative addressing.
//! Absolute memory operands can only be used if it's known that the PC relative offset is encodable and that it
//! would be within the limits. Absolute address is also often output from disassemblers, so AsmJit supports it to
//! make it possible to assemble such output back.
static ASMJIT_INLINE_NODEBUG constexpr Mem ptr(uint64_t base) noexcept { return Mem(base); }

//! \}

//! \}

ASMJIT_END_SUB_NAMESPACE

#endif // ASMJIT_LA_LAOPERAND_H_INCLUDED
