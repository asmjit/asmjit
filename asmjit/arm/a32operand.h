// This file is part of AsmJit project <https://asmjit.com>
//
// See asmjit.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_ARM_A32OPERAND_H_INCLUDED
#define ASMJIT_ARM_A32OPERAND_H_INCLUDED

#include <asmjit/core/operand.h>
#include <asmjit/arm/a32globals.h>

ASMJIT_BEGIN_SUB_NAMESPACE(a32)

//! \addtogroup asmjit_a32
//! \{

//! General purpose register (AArch32).
class Gp : public UniGp {
public:
  ASMJIT_DEFINE_ABSTRACT_REG(Gp, UniGp)

  //! \name Constants
  //! \{

  //! Special register id.
  enum Id : uint32_t {
    //! Frame pointer register id.
    kIdFP = 11u,
    //! Stack register id.
    kIdSP = 13u,
    //! Link register id.
    kIdLR = 14u,
    //! Program counter register id.
    kIdPC = 15u
  };

  //! \}

  //! \name Static Constructors
  //! \{

  //! Creates a new 32-bit low general purpose register (R) having the given register id `reg_id`.
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR Gp make_r32(uint32_t reg_id) noexcept { return Gp(signature_of_t<RegType::kGp32>(), reg_id); }

  //! Creates a new 32-bit low general purpose register (R) having the given register id `reg_id`.
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR Gp make_r(uint32_t reg_id) noexcept { return make_r32(reg_id); }

  //! \}

  //! \name Gp Register Accessors
  //! \{

  //! Test whether this register is FP register.
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_fp() const noexcept { return id() == kIdFP; }

  //! Test whether this register is SP register.
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_sp() const noexcept { return id() == kIdSP; }

  //! Test whether this register is LR register.
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_lr() const noexcept { return id() == kIdLR; }

  //! Test whether this register is PC register.
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_pc() const noexcept { return id() == kIdPC; }

  //! Returns whether the register contains a shift operation predicate
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool hast_shift_op() const noexcept { return predicate() != 0u; }

  //! Returns shift operation predicate.
  ASMJIT_INLINE_CONSTEXPR ShiftOp shift_op() const noexcept { return ShiftOp(predicate()); }

  //! Sets a shift operation predicate to `op`.
  ASMJIT_INLINE_CONSTEXPR void set_shift_op(ShiftOp op) noexcept { set_predicate(uint32_t(op)); }

  //! Resets a shift operation predicate.
  ASMJIT_INLINE_CONSTEXPR void reset_shift_op() noexcept { return reset_predicate(); }

  //! \}

  //! \name Clone
  //! \{

  //! Clones the register with assigned shift operation predicate `op`.
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Gp shifted(ShiftOp op) const noexcept {
    Gp r(*this);
    r.set_shift_op(op);
    return r;
  }

  //! \}
};

//! Vector register (AArch32).
class Vec : public UniVec {
public:
  ASMJIT_DEFINE_ABSTRACT_REG(Vec, UniVec)

  //! \cond

  // Register has element index (1 bit).
  // |........|........|X.......|........|
  static inline constexpr uint32_t kSignatureRegElementFlagShift = 15;
  static inline constexpr uint32_t kSignatureRegElementFlagMask = 0x01 << kSignatureRegElementFlagShift;

  // Register element index (4 bits).
  // |........|....XXXX|........|........|
  static inline constexpr uint32_t kSignatureRegElementIndexShift = 16;
  static inline constexpr uint32_t kSignatureRegElementIndexMask = 0x0F << kSignatureRegElementIndexShift;

  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR OperandSignature _make_element_access_signature(uint32_t element_index) noexcept {
    return OperandSignature{
      uint32_t(RegTraits<RegType::kVec128>::kSignature)          |
      uint32_t(kSignatureRegElementFlagMask)                     |
      (uint32_t(element_index << kSignatureRegElementIndexShift))
    };
  }

  //! \endcond

  //! \name Static Constructors
  //! \{

  //! Creates a new 32-bit vector register (S) having the given register id `reg_id`.
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR Vec make_v32(uint32_t reg_id) noexcept { return Vec(signature_of_t<RegType::kVec32>(), reg_id); }

  //! Creates a new 64-bit vector register (D) having the given register id `reg_id`.
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR Vec make_v64(uint32_t reg_id) noexcept { return Vec(signature_of_t<RegType::kVec64>(), reg_id); }

  //! Creates a new 128-bit vector register (Q) having the given register id `reg_id`.
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR Vec make_v128(uint32_t reg_id) noexcept { return Vec(signature_of_t<RegType::kVec128>(), reg_id); }

  //! Creates a new 32-bit vector register (S) having the given register id `reg_id`.
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR Vec make_s(uint32_t reg_id) noexcept { return make_v32(reg_id); }

  //! Creates a new 64-bit vector register (D) having the given register id `reg_id`.
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR Vec make_d(uint32_t reg_id) noexcept { return make_v64(reg_id); }

  //! Creates a new 128-bit vector register (Q) having the given register id `reg_id`.
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR Vec make_q(uint32_t reg_id) noexcept { return make_v128(reg_id); }

  //! Creates a new 128-bit vector of type specified by `element_type` and `element_index`.
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR Vec make_v128_with_element_index(uint32_t element_index, uint32_t reg_id) noexcept {
    return Vec(_make_element_access_signature(element_index), reg_id);
  }

  //! \}

  //! \name Vector Register Accessors
  //! \{

  //! Clones and casts the register to a 32-bit S register (element type & index is not cloned).
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Vec v32() const noexcept { return make_v32(id()); }

  //! Clones and casts the register to a 64-bit D register (element type & index is not cloned).
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Vec v64() const noexcept { return make_v64(id()); }

  //! Clones and casts the register to a 128-bit Q register (element type & index is not cloned).
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Vec v128() const noexcept { return make_v128(id()); }

  //! Clones and casts the register to a 32-bit S register (element type & index is not cloned).
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Vec s() const noexcept { return make_v32(id()); }

  //! Clones and casts the register to a 64-bit D register (element type & index is not cloned).
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Vec d() const noexcept { return make_v64(id()); }

  //! Clones and casts the register to a 128-bit Q register (element type & index is not cloned).
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Vec q() const noexcept { return make_v128(id()); }

  //! \}

  //! \name Element Index Accessors
  //! \{

  //! Returns whether the register has element index (it's an element index access).
  ASMJIT_INLINE_CONSTEXPR bool has_element_index() const noexcept {
    return _signature.has_field<kSignatureRegElementFlagMask>();
  }

  //! Returns element index of the register.
  ASMJIT_INLINE_CONSTEXPR uint32_t element_index() const noexcept {
    return _signature.get_field<kSignatureRegElementIndexMask>();
  }

  //! Sets element index of the register to `element_type`.
  ASMJIT_INLINE_CONSTEXPR void set_element_index(uint32_t element_index) noexcept {
    _signature |= kSignatureRegElementFlagMask;
    _signature.set_field<kSignatureRegElementIndexMask>(element_index);
  }

  //! Resets element index of the register.
  ASMJIT_INLINE_CONSTEXPR void reset_element_index() noexcept {
    _signature &= ~(kSignatureRegElementFlagMask | kSignatureRegElementIndexMask);
  }

  //! Clones a vector register with element access enabled at the given `element_index`.
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Vec at(uint32_t element_index) const noexcept {
    return Vec((signature() & ~kSignatureRegElementIndexMask) | (element_index << kSignatureRegElementIndexShift) | kSignatureRegElementFlagMask, id());
  }

  //! \}
};

//! Register-list of 32-bit GP registers.
class GpList : public RegListT<Gp> {
public:
  //! \name Constants
  //! \{

  static inline constexpr uint32_t kSignature =
    Signature::from_op_type(OperandType::kRegList).bits() | (RegTraits<RegType::kGp32>::kSignature & ~Signature::kOpTypeMask);

  //! \}

  //! \name Construction & Destruction
  //! \{

  //! Creates a dummy register-list operand.
  ASMJIT_INLINE_NODEBUG constexpr GpList() noexcept
    : RegListT<Gp>(Signature{kSignature}, RegMask(0)) {}

  //! Creates a register-list operand which is the same as `other` .
  ASMJIT_INLINE_NODEBUG constexpr GpList(const GpList& other) noexcept
    : RegListT<Gp>(other) {}

  //! Creates a register-list operand initialized to the given `regMask`.
  ASMJIT_INLINE_NODEBUG explicit constexpr GpList(RegMask regMask) noexcept
    : RegListT<Gp>(Signature{kSignature}, regMask) {}

  //! Creates a register-list operand initialized to `regs`.
  ASMJIT_INLINE_NODEBUG explicit GpList(std::initializer_list<Gp> regs) noexcept
    : RegListT(Signature{kSignature}, regs) {}

  ASMJIT_INLINE_NODEBUG explicit GpList(Globals::NoInit_) noexcept
    : RegListT<Gp>(Globals::NoInit) {}

  //! \}

  //! \name Overloaded Operators
  //! \{

  ASMJIT_INLINE_NODEBUG GpList& operator=(const GpList& other) noexcept = default;

  ASMJIT_INLINE_NODEBUG GpList& operator|=(const GpList& other) noexcept { add_list(other); return *this; }
  ASMJIT_INLINE_NODEBUG GpList& operator&=(const GpList& other) noexcept { and_list(other); return *this; }
  ASMJIT_INLINE_NODEBUG GpList& operator^=(const GpList& other) noexcept { xor_list(other); return *this; }

  ASMJIT_INLINE_NODEBUG GpList operator|(const GpList& other) const noexcept { return GpList(list() | other.list()); }
  ASMJIT_INLINE_NODEBUG GpList operator&(const GpList& other) const noexcept { return GpList(list() & other.list()); }
  ASMJIT_INLINE_NODEBUG GpList operator^(const GpList& other) const noexcept { return GpList(list() ^ other.list()); }

  //! \}
};

//! Register-list of 32-bit (vector) S registers.
class VecSList : public RegListT<Vec> {
public:
  //! \name Constants
  //! \{

  static inline constexpr uint32_t kSignature =
    Signature::from_op_type(OperandType::kRegList).bits() | (RegTraits<RegType::kVec32>::kSignature & ~Signature::kOpTypeMask);

  //! \}

  //! \name Construction & Destruction
  //! \{

  //! Creates a dummy register-list operand.
  ASMJIT_INLINE_NODEBUG constexpr VecSList() noexcept
    : RegListT<Vec>(Signature{kSignature}, RegMask(0)) {}

  //! Creates a register-list operand which is the same as `other` .
  ASMJIT_INLINE_NODEBUG constexpr VecSList(const VecSList& other) noexcept
    : RegListT<Vec>(other) {}

  //! Creates a register-list operand initialized to the given `regMask`.
  ASMJIT_INLINE_NODEBUG explicit constexpr VecSList(RegMask regMask) noexcept
    : RegListT<Vec>(Signature{kSignature}, regMask) {}

  //! Creates a register-list operand initialized to `regs`.
  ASMJIT_INLINE_NODEBUG explicit VecSList(std::initializer_list<Vec> regs) noexcept
    : RegListT(Signature{kSignature}, regs) {}

  ASMJIT_INLINE_NODEBUG explicit VecSList(Globals::NoInit_) noexcept
    : RegListT<Vec>(Globals::NoInit) {}

  //! \}

  //! \name Overloaded Operators
  //! \{

  ASMJIT_INLINE_NODEBUG VecSList& operator=(const VecSList& other) noexcept = default;

  ASMJIT_INLINE_NODEBUG VecSList& operator|=(const VecSList& other) noexcept { add_list(other); return *this; }
  ASMJIT_INLINE_NODEBUG VecSList& operator&=(const VecSList& other) noexcept { and_list(other); return *this; }
  ASMJIT_INLINE_NODEBUG VecSList& operator^=(const VecSList& other) noexcept { xor_list(other); return *this; }

  ASMJIT_INLINE_NODEBUG VecSList operator|(const VecSList& other) const noexcept { return VecSList(list() | other.list()); }
  ASMJIT_INLINE_NODEBUG VecSList operator&(const VecSList& other) const noexcept { return VecSList(list() & other.list()); }
  ASMJIT_INLINE_NODEBUG VecSList operator^(const VecSList& other) const noexcept { return VecSList(list() ^ other.list()); }

  //! \}
};

//! Register-list of 32-bit (vector) D registers.
class VecDList : public RegListT<Vec> {
public:
  //! \name Constants
  //! \{

  static inline constexpr uint32_t kSignature =
    Signature::from_op_type(OperandType::kRegList).bits() | (RegTraits<RegType::kVec64>::kSignature & ~Signature::kOpTypeMask);

  //! \}

  //! \name Construction & Destruction
  //! \{

  //! Creates a dummy register-list operand.
  ASMJIT_INLINE_NODEBUG constexpr VecDList() noexcept
    : RegListT<Vec>(Signature{kSignature}, RegMask(0)) {}

  //! Creates a register-list operand which is the same as `other` .
  ASMJIT_INLINE_NODEBUG constexpr VecDList(const VecDList& other) noexcept
    : RegListT<Vec>(other) {}

  //! Creates a register-list operand initialized to the given `regMask`.
  ASMJIT_INLINE_NODEBUG explicit constexpr VecDList(RegMask regMask) noexcept
    : RegListT<Vec>(Signature{kSignature}, regMask) {}

  //! Creates a register-list operand initialized to `regs`.
  ASMJIT_INLINE_NODEBUG explicit VecDList(std::initializer_list<Vec> regs) noexcept
    : RegListT(Signature{kSignature}, regs) {}

  ASMJIT_INLINE_NODEBUG explicit VecDList(Globals::NoInit_) noexcept
    : RegListT<Vec>(Globals::NoInit) {}

  //! \}

  //! \name Overloaded Operators
  //! \{

  ASMJIT_INLINE_NODEBUG VecDList& operator=(const VecDList& other) noexcept = default;

  ASMJIT_INLINE_NODEBUG VecDList& operator|=(const VecDList& other) noexcept { add_list(other); return *this; }
  ASMJIT_INLINE_NODEBUG VecDList& operator&=(const VecDList& other) noexcept { and_list(other); return *this; }
  ASMJIT_INLINE_NODEBUG VecDList& operator^=(const VecDList& other) noexcept { xor_list(other); return *this; }

  ASMJIT_INLINE_NODEBUG VecDList operator|(const VecDList& other) const noexcept { return VecDList(list() | other.list()); }
  ASMJIT_INLINE_NODEBUG VecDList operator&(const VecDList& other) const noexcept { return VecDList(list() & other.list()); }
  ASMJIT_INLINE_NODEBUG VecDList operator^(const VecDList& other) const noexcept { return VecDList(list() ^ other.list()); }

  //! \}
};

//! Memory operand (AArch32).
class Mem : public BaseMem {
public:
  //! \cond INTERNAL

  // Index shift value (5 bits).
  // |........|.....XXX|XX......|........|
  static inline constexpr uint32_t kSignatureMemShiftValueShift = 14;
  static inline constexpr uint32_t kSignatureMemShiftValueMask = 0x1Fu << kSignatureMemShiftValueShift;

  // Index shift operation (4 bits).
  // |........|XXXX....|........|........|
  static inline constexpr uint32_t kSignatureMemShiftOpShift = 20;
  static inline constexpr uint32_t kSignatureMemShiftOpMask = 0x0Fu << kSignatureMemShiftOpShift;

  // Offset mode type (2 bits).
  // |......XX|........|........|........|
  static inline constexpr uint32_t kSignatureMemOffsetModeShift = 24;
  static inline constexpr uint32_t kSignatureMemOffsetModeMask = 0x03u << kSignatureMemOffsetModeShift;

  //! \endcond

  //! \name Construction & Destruction
  //! \{

  //! Construct a default `Mem` operand, that points to [0].
  ASMJIT_INLINE_CONSTEXPR Mem() noexcept
    : BaseMem() {}

  ASMJIT_INLINE_CONSTEXPR Mem(const Mem& other) noexcept
    : BaseMem(other) {}

  ASMJIT_INLINE_NODEBUG explicit Mem(Globals::NoInit_) noexcept
    : BaseMem(Globals::NoInit) {}

  ASMJIT_INLINE_CONSTEXPR Mem(const Signature& signature, uint32_t base_id, uint32_t index_id, int32_t offset) noexcept
    : BaseMem(signature, base_id, index_id, offset) {}

  ASMJIT_INLINE_CONSTEXPR explicit Mem(const Label& base, int32_t off = 0, Signature signature = Signature{0}) noexcept
    : BaseMem(Signature::from_op_type(OperandType::kMem) |
              Signature::from_mem_base_type(RegType::kLabelTag) |
              signature, base.id(), 0, off) {}

  ASMJIT_INLINE_CONSTEXPR explicit Mem(const Reg& base, int32_t off = 0, Signature signature = Signature{0}) noexcept
    : BaseMem(Signature::from_op_type(OperandType::kMem) |
              Signature::from_mem_base_type(base.reg_type()) |
              signature, base.id(), 0, off) {}

  ASMJIT_INLINE_CONSTEXPR Mem(const Reg& base, const Reg& index, Signature signature = Signature{0}) noexcept
    : BaseMem(Signature::from_op_type(OperandType::kMem) |
              Signature::from_mem_base_type(base.reg_type()) |
              Signature::from_mem_index_type(index.reg_type()) |
              signature, base.id(), index.id(), 0) {}

  ASMJIT_INLINE_CONSTEXPR Mem(const Reg& base, const Reg& index, const Shift& shift, Signature signature = Signature{0}) noexcept
    : BaseMem(Signature::from_op_type(OperandType::kMem) |
              Signature::from_mem_base_type(base.reg_type()) |
              Signature::from_mem_index_type(index.reg_type()) |
              Signature::from_value<kSignatureMemShiftOpMask>(uint32_t(shift.op())) |
              Signature::from_value<kSignatureMemShiftValueMask>(shift.value()) |
              signature, base.id(), index.id(), 0) {}

  ASMJIT_INLINE_CONSTEXPR explicit Mem(uint64_t base, Signature signature = Signature{0}) noexcept
    : BaseMem(Signature::from_op_type(OperandType::kMem) |
              signature, uint32_t(base >> 32), 0, int32_t(uint32_t(base & 0xFFFFFFFFu))) {}

  //! \}

  //! \name Overloaded Operators
  //! \{

  ASMJIT_INLINE_CONSTEXPR Mem& operator=(const Mem& other) noexcept {
    copy_from(other);
    return *this;
  }

  //! \}

  //! \name Clone
  //! \{

  //! Clones the memory operand.
  ASMJIT_INLINE_CONSTEXPR Mem clone() const noexcept { return Mem(*this); }

  //! Gets new memory operand adjusted by `off`.
  ASMJIT_INLINE_CONSTEXPR Mem clone_adjusted(int64_t off) const noexcept {
    Mem result(*this);
    result.add_offset(off);
    return result;
  }

  //! Clones the memory operand and makes it pre-index.
  ASMJIT_INLINE_CONSTEXPR Mem pre() const noexcept {
    Mem result(*this);
    result.set_offset_mode(OffsetMode::kPreIndex);
    return result;
  }

  //! Clones the memory operand, applies a given offset `off` and makes it pre-index.
  ASMJIT_INLINE_CONSTEXPR Mem pre(int64_t off) const noexcept {
    Mem result(*this);
    result.set_offset_mode(OffsetMode::kPreIndex);
    result.add_offset(off);
    return result;
  }

  //! Clones the memory operand and makes it post-index.
  ASMJIT_INLINE_CONSTEXPR Mem post() const noexcept {
    Mem result(*this);
    result.set_offset_mode(OffsetMode::kPostIndex);
    return result;
  }

  //! Clones the memory operand, applies a given offset `off` and makes it post-index.
  ASMJIT_INLINE_CONSTEXPR Mem post(int64_t off) const noexcept {
    Mem result(*this);
    result.set_offset_mode(OffsetMode::kPostIndex);
    result.add_offset(off);
    return result;
  }

  //! \}

  //! \name Base & Index
  //! \{

  //! Converts memory `base_type` and `base_id` to `arm::Reg` instance.
  //!
  //! The memory must have a valid base register otherwise the result will be wrong.
  ASMJIT_INLINE_NODEBUG Reg base_reg() const noexcept { return Reg::from_type_and_id(base_type(), base_id()); }

  //! Converts memory `index_type` and `index_id` to `arm::Reg` instance.
  //!
  //! The memory must have a valid index register otherwise the result will be wrong.
  ASMJIT_INLINE_NODEBUG Reg index_reg() const noexcept { return Reg::from_type_and_id(index_type(), index_id()); }

  using BaseMem::set_index;

  ASMJIT_INLINE_CONSTEXPR void set_index(const Reg& index, uint32_t shift) noexcept {
    set_index(index);
    set_shift(shift);
  }

  ASMJIT_INLINE_CONSTEXPR void set_index(const Reg& index, Shift shift) noexcept {
    set_index(index);
    set_shift(shift);
  }

  //! \}

  //! \name ARM Specific Features
  //! \{

  //! Gets offset mode.
  ASMJIT_INLINE_CONSTEXPR OffsetMode offset_mode() const noexcept { return OffsetMode(_signature.get_field<kSignatureMemOffsetModeMask>()); }
  //! Sets offset mode to `mode`.
  ASMJIT_INLINE_CONSTEXPR void set_offset_mode(OffsetMode mode) noexcept { _signature.set_field<kSignatureMemOffsetModeMask>(uint32_t(mode)); }
  //! Resets offset mode to default (fixed offset, without write-back).
  ASMJIT_INLINE_CONSTEXPR void reset_offset_mode() noexcept { _signature.set_field<kSignatureMemOffsetModeMask>(uint32_t(OffsetMode::kFixed)); }

  //! Tests whether the current memory offset mode is fixed (see \ref arm::OffsetMode::kFixed).
  ASMJIT_INLINE_CONSTEXPR bool is_fixed_offset() const noexcept { return offset_mode() == OffsetMode::kFixed; }
  //! Tests whether the current memory offset mode is either pre-index or post-index (write-back is used).
  ASMJIT_INLINE_CONSTEXPR bool is_pre_or_post() const noexcept { return offset_mode() != OffsetMode::kFixed; }
  //! Tests whether the current memory offset mode is pre-index (write-back is used).
  ASMJIT_INLINE_CONSTEXPR bool is_pre_index() const noexcept { return offset_mode() == OffsetMode::kPreIndex; }
  //! Tests whether the current memory offset mode is post-index (write-back is used).
  ASMJIT_INLINE_CONSTEXPR bool is_post_index() const noexcept { return offset_mode() == OffsetMode::kPostIndex; }

  //! Sets offset mode of this memory operand to pre-index (write-back is used).
  ASMJIT_INLINE_CONSTEXPR void make_pre_index() noexcept { set_offset_mode(OffsetMode::kPreIndex); }
  //! Sets offset mode of this memory operand to post-index (write-back is used).
  ASMJIT_INLINE_CONSTEXPR void make_post_index() noexcept { set_offset_mode(OffsetMode::kPostIndex); }

  //! Gets shift operation that is used by index register.
  ASMJIT_INLINE_CONSTEXPR ShiftOp shift_op() const noexcept { return ShiftOp(_signature.get_field<kSignatureMemShiftOpMask>()); }
  //! Sets shift operation that is used by index register.
  ASMJIT_INLINE_CONSTEXPR void set_shift_op(ShiftOp sop) noexcept { _signature.set_field<kSignatureMemShiftOpMask>(uint32_t(sop)); }
  //! Resets shift operation that is used by index register to LSL (default value).
  ASMJIT_INLINE_CONSTEXPR void reset_shift_op() noexcept { _signature.set_field<kSignatureMemShiftOpMask>(uint32_t(ShiftOp::kLSL)); }

  //! Gets whether the memory operand has shift (aka scale) constant.
  ASMJIT_INLINE_CONSTEXPR bool has_shift() const noexcept { return _signature.has_field<kSignatureMemShiftValueMask>(); }
  //! Gets the memory operand's shift (aka scale) constant.
  ASMJIT_INLINE_CONSTEXPR uint32_t shift() const noexcept { return _signature.get_field<kSignatureMemShiftValueMask>(); }
  //! Sets the memory operand's shift (aka scale) constant.
  ASMJIT_INLINE_CONSTEXPR void set_shift(uint32_t shift) noexcept { _signature.set_field<kSignatureMemShiftValueMask>(shift); }

  //! Sets the memory operand's shift and shift operation.
  ASMJIT_INLINE_CONSTEXPR void set_shift(Shift shift) noexcept {
    _signature.set_field<kSignatureMemShiftOpMask>(uint32_t(shift.op()));
    _signature.set_field<kSignatureMemShiftValueMask>(shift.value());
  }

  //! Resets the memory operand's shift (aka scale) constant to zero.
  ASMJIT_INLINE_CONSTEXPR void reset_shift() noexcept { _signature.set_field<kSignatureMemShiftValueMask>(0); }

  //! \}
};

#ifndef _DOXYGEN
namespace regs {
#endif

//! Creates a 32-bit R register operand.
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Gp r(uint32_t id) noexcept { return Gp::make_r32(id); }

//! Creates a 32-bit S register operand.
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Vec s(uint32_t id) noexcept { return Vec::make_v32(id); }

//! Creates a 64-bit D register operand.
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Vec d(uint32_t id) noexcept { return Vec::make_v64(id); }

//! Creates a 128-bit V register operand.
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Vec q(uint32_t id) noexcept { return Vec::make_v128(id); }

static constexpr Gp r0 = Gp::make_r32(0);
static constexpr Gp r1 = Gp::make_r32(1);
static constexpr Gp r2 = Gp::make_r32(2);
static constexpr Gp r3 = Gp::make_r32(3);
static constexpr Gp r4 = Gp::make_r32(4);
static constexpr Gp r5 = Gp::make_r32(5);
static constexpr Gp r6 = Gp::make_r32(6);
static constexpr Gp r7 = Gp::make_r32(7);
static constexpr Gp r8 = Gp::make_r32(8);
static constexpr Gp r9 = Gp::make_r32(9);
static constexpr Gp r10 = Gp::make_r32(10);
static constexpr Gp r11 = Gp::make_r32(11);
static constexpr Gp r12 = Gp::make_r32(12);
static constexpr Gp r13 = Gp::make_r32(13);
static constexpr Gp r14 = Gp::make_r32(14);
static constexpr Gp r15 = Gp::make_r32(15);

static constexpr Gp fp = Gp::make_r32(Gp::kIdFP);
static constexpr Gp sp = Gp::make_r32(Gp::kIdSP);
static constexpr Gp lr = Gp::make_r32(Gp::kIdLR);
static constexpr Gp pc = Gp::make_r32(Gp::kIdPC);

static constexpr Vec s0 = Vec::make_v32(0);
static constexpr Vec s1 = Vec::make_v32(1);
static constexpr Vec s2 = Vec::make_v32(2);
static constexpr Vec s3 = Vec::make_v32(3);
static constexpr Vec s4 = Vec::make_v32(4);
static constexpr Vec s5 = Vec::make_v32(5);
static constexpr Vec s6 = Vec::make_v32(6);
static constexpr Vec s7 = Vec::make_v32(7);
static constexpr Vec s8 = Vec::make_v32(8);
static constexpr Vec s9 = Vec::make_v32(9);
static constexpr Vec s10 = Vec::make_v32(10);
static constexpr Vec s11 = Vec::make_v32(11);
static constexpr Vec s12 = Vec::make_v32(12);
static constexpr Vec s13 = Vec::make_v32(13);
static constexpr Vec s14 = Vec::make_v32(14);
static constexpr Vec s15 = Vec::make_v32(15);
static constexpr Vec s16 = Vec::make_v32(16);
static constexpr Vec s17 = Vec::make_v32(17);
static constexpr Vec s18 = Vec::make_v32(18);
static constexpr Vec s19 = Vec::make_v32(19);
static constexpr Vec s20 = Vec::make_v32(20);
static constexpr Vec s21 = Vec::make_v32(21);
static constexpr Vec s22 = Vec::make_v32(22);
static constexpr Vec s23 = Vec::make_v32(23);
static constexpr Vec s24 = Vec::make_v32(24);
static constexpr Vec s25 = Vec::make_v32(25);
static constexpr Vec s26 = Vec::make_v32(26);
static constexpr Vec s27 = Vec::make_v32(27);
static constexpr Vec s28 = Vec::make_v32(28);
static constexpr Vec s29 = Vec::make_v32(29);
static constexpr Vec s30 = Vec::make_v32(30);
static constexpr Vec s31 = Vec::make_v32(31);

static constexpr Vec d0 = Vec::make_v64(0);
static constexpr Vec d1 = Vec::make_v64(1);
static constexpr Vec d2 = Vec::make_v64(2);
static constexpr Vec d3 = Vec::make_v64(3);
static constexpr Vec d4 = Vec::make_v64(4);
static constexpr Vec d5 = Vec::make_v64(5);
static constexpr Vec d6 = Vec::make_v64(6);
static constexpr Vec d7 = Vec::make_v64(7);
static constexpr Vec d8 = Vec::make_v64(8);
static constexpr Vec d9 = Vec::make_v64(9);
static constexpr Vec d10 = Vec::make_v64(10);
static constexpr Vec d11 = Vec::make_v64(11);
static constexpr Vec d12 = Vec::make_v64(12);
static constexpr Vec d13 = Vec::make_v64(13);
static constexpr Vec d14 = Vec::make_v64(14);
static constexpr Vec d15 = Vec::make_v64(15);
static constexpr Vec d16 = Vec::make_v64(16);
static constexpr Vec d17 = Vec::make_v64(17);
static constexpr Vec d18 = Vec::make_v64(18);
static constexpr Vec d19 = Vec::make_v64(19);
static constexpr Vec d20 = Vec::make_v64(20);
static constexpr Vec d21 = Vec::make_v64(21);
static constexpr Vec d22 = Vec::make_v64(22);
static constexpr Vec d23 = Vec::make_v64(23);
static constexpr Vec d24 = Vec::make_v64(24);
static constexpr Vec d25 = Vec::make_v64(25);
static constexpr Vec d26 = Vec::make_v64(26);
static constexpr Vec d27 = Vec::make_v64(27);
static constexpr Vec d28 = Vec::make_v64(28);
static constexpr Vec d29 = Vec::make_v64(29);
static constexpr Vec d30 = Vec::make_v64(30);
static constexpr Vec d31 = Vec::make_v64(31);

static constexpr Vec q0 = Vec::make_v128(0);
static constexpr Vec q1 = Vec::make_v128(1);
static constexpr Vec q2 = Vec::make_v128(2);
static constexpr Vec q3 = Vec::make_v128(3);
static constexpr Vec q4 = Vec::make_v128(4);
static constexpr Vec q5 = Vec::make_v128(5);
static constexpr Vec q6 = Vec::make_v128(6);
static constexpr Vec q7 = Vec::make_v128(7);
static constexpr Vec q8 = Vec::make_v128(8);
static constexpr Vec q9 = Vec::make_v128(9);
static constexpr Vec q10 = Vec::make_v128(10);
static constexpr Vec q11 = Vec::make_v128(11);
static constexpr Vec q12 = Vec::make_v128(12);
static constexpr Vec q13 = Vec::make_v128(13);
static constexpr Vec q14 = Vec::make_v128(14);
static constexpr Vec q15 = Vec::make_v128(15);

#ifndef _DOXYGEN
} // {regs}

// Make `a32::regs` accessible through `a32` namespace as well.
using namespace regs;
#endif

//! \name Shift Operation Construction
//! \{

//! Constructs a `LSL #value` shift (logical shift left).
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Gp lsl(const Gp& gp) noexcept { return gp.shifted(ShiftOp::kLSL); }

//! Constructs a `LSR #value` shift (logical shift right).
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Gp lsr(const Gp& gp) noexcept { return gp.shifted(ShiftOp::kLSR); }

//! Constructs a `ASR #value` shift (arithmetic shift right).
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Gp asr(const Gp& gp) noexcept { return gp.shifted(ShiftOp::kASR); }

//! Constructs a `ROR #value` shift (rotate right).
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Gp ror(const Gp& gp) noexcept { return gp.shifted(ShiftOp::kROR); }

//! Constructs a `LSL #value` shift (logical shift left).
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Shift lsl(uint32_t value) noexcept { return Shift(ShiftOp::kLSL, value); }

//! Constructs a `LSR #value` shift (logical shift right).
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Shift lsr(uint32_t value) noexcept { return Shift(ShiftOp::kLSR, value); }

//! Constructs a `ASR #value` shift (arithmetic shift right).
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Shift asr(uint32_t value) noexcept { return Shift(ShiftOp::kASR, value); }

//! Constructs a `ROR #value` shift (rotate right).
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Shift ror(uint32_t value) noexcept { return Shift(ShiftOp::kROR, value); }

//! \}

//! \name Memory Operand Construction
//! \{

//! Creates `[base, offset]` memory operand (offset mode) (AArch32).
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Mem ptr(const Gp& base, int32_t offset = 0) noexcept {
  return Mem(base, offset);
}

//! Creates `[base, offset]!` memory operand (pre-index mode) (AArch32).
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Mem ptr_pre(const Gp& base, int32_t offset = 0) noexcept {
  return Mem(base, offset, OperandSignature::from_value<Mem::kSignatureMemOffsetModeMask>(OffsetMode::kPreIndex));
}

//! Creates `[base], offset` memory operand (post-index mode) (AArch32).
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Mem ptr_post(const Gp& base, int32_t offset = 0) noexcept {
  return Mem(base, offset, OperandSignature::from_value<Mem::kSignatureMemOffsetModeMask>(OffsetMode::kPostIndex));
}

//! Creates `[base, index]` memory operand (AArch32).
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Mem ptr(const Gp& base, const Gp& index) noexcept {
  return Mem(base, index);
}

//! Creates `[base, index]!` memory operand (pre-index mode) (AArch32).
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Mem ptr_pre(const Gp& base, const Gp& index) noexcept {
  return Mem(base, index, OperandSignature::from_value<Mem::kSignatureMemOffsetModeMask>(OffsetMode::kPreIndex));
}

//! Creates `[base], index` memory operand (post-index mode) (AArch32).
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Mem ptr_post(const Gp& base, const Gp& index) noexcept {
  return Mem(base, index, OperandSignature::from_value<Mem::kSignatureMemOffsetModeMask>(OffsetMode::kPostIndex));
}

//! Creates `[base, index, SHIFT_OP #shift]` memory operand (AArch32).
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Mem ptr(const Gp& base, const Gp& index, const Shift& shift) noexcept {
  return Mem(base, index, shift);
}

//! Creates `[base, offset]` memory operand (AArch32).
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Mem ptr(const Label& base, int32_t offset = 0) noexcept {
  return Mem(base, offset);
}


//! \}

ASMJIT_END_SUB_NAMESPACE

#endif // ASMJIT_ARM_A32OPERAND_H_INCLUDED
