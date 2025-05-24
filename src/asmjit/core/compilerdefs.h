// This file is part of AsmJit project <https://asmjit.com>
//
// See asmjit.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_CORE_COMPILERDEFS_H_INCLUDED
#define ASMJIT_CORE_COMPILERDEFS_H_INCLUDED

#include "../core/api-config.h"
#include "../core/operand.h"
#include "../core/type.h"
#include "../core/zonestring.h"

ASMJIT_BEGIN_NAMESPACE

class RAWorkReg;

//! \addtogroup asmjit_compiler
//! \{

//! Public virtual register interface, managed by \ref BaseCompiler.
//!
//! When a virtual register is created by \ref BaseCompiler a `VirtReg` is linked with the register operand id it
//! returns. This `VirtReg` can be accessed via \ref BaseCompiler::virtRegByReg() function, which returns a pointer
//! to `VirtReg`.
//!
//! In general, `VirtReg` should be only introspected as it contains important variables that are needed and managed
//! by AsmJit, however, the `VirtReg` API can also be used to influence register allocation. For example there is
//! a \ref VirtReg::setWeight() function, which could be used to increase a weight of a virtual register (thus make
//! it hard to spill, for example). In addition, there is a \ref VirtReg::setHomeIdHint() function, which can be used
//! to do an initial assignment of a physical register of a virtual register. However, AsmJit could still override
//! the physical register assigned in some special cases.
class VirtReg {
public:
  ASMJIT_NONCOPYABLE(VirtReg)

  //! \name Members
  //! \{

  //! Virtual register signature.
  OperandSignature _signature {};
  //! Virtual register id.
  uint32_t _id = 0;
  //! Virtual register size (can be smaller than `_signature._size`).
  uint32_t _virtSize = 0;
  //! Virtual register alignment (for spilling).
  uint8_t _alignment = 0;
  //! Type-id.
  TypeId _typeId = TypeId::kVoid;
  //! Virtual register weight for alloc/spill decisions.
  uint8_t _weight = 1;
  //! True if this is a fixed register, never reallocated.
  uint8_t _isFixed : 1;
  //! True if the virtual register is only used as a stack (never accessed as register).
  uint8_t _isStack : 1;
  //! True if this virtual register has assigned stack offset (can be only valid after register allocation pass).
  uint8_t _hasStackSlot : 1;
  uint8_t _reservedBits : 5;
  //! Home register hint for the register allocator (initially unassigned).
  uint8_t _homeIdHint = BaseReg::kIdBad;

  //! Stack offset assigned by the register allocator relative to stack pointer (can be negative as well).
  int32_t _stackOffset = 0;

  //! Reserved for future use (padding).
  uint32_t _reservedU32 = 0;

  //! Virtual register name (user provided or automatically generated).
  ZoneString<16> _name {};

  // The following members are used exclusively by RAPass. They are initialized when the VirtReg is created to
  // null pointers and then changed during RAPass execution. RAPass sets them back to NULL before it returns.

  //! Reference to `RAWorkReg`, used during register allocation.
  RAWorkReg* _workReg = nullptr;

  //! \}

  //! \name Construction & Destruction
  //! \{

  ASMJIT_INLINE_NODEBUG VirtReg(OperandSignature signature, uint32_t id, uint32_t virtSize, uint32_t alignment, TypeId typeId) noexcept
    : _signature(signature),
      _id(id),
      _virtSize(virtSize),
      _alignment(uint8_t(alignment)),
      _typeId(typeId),
      _isFixed(0),
      _isStack(0),
      _hasStackSlot(0),
      _reservedBits(0) {}

  //! \}

  //! \name Accessors
  //! \{

  //! Returns the virtual register id.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t id() const noexcept { return _id; }

  //! Returns the virtual register name.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const char* name() const noexcept { return _name.data(); }

  //! Returns the size of the virtual register name.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t nameSize() const noexcept { return _name.size(); }

  //! Returns a register signature of this virtual register.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG OperandSignature signature() const noexcept { return _signature; }

  //! Returns a virtual register type (maps to the physical register type as well).
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RegType type() const noexcept { return _signature.regType(); }

  //! Returns a virtual register group (maps to the physical register group as well).
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RegGroup group() const noexcept { return _signature.regGroup(); }

  //! Returns a real size of the register this virtual register maps to.
  //!
  //! For example if this is a 128-bit SIMD register used for a scalar single precision floating point value then
  //! its virtSize would be 4, however, the `regSize` would still say 16 (128-bits), because it's the smallest size
  //! of that register type.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t regSize() const noexcept { return _signature.size(); }

  //! Returns the virtual register size.
  //!
  //! The virtual register size describes how many bytes the virtual register needs to store its content. It can be
  //! smaller than the physical register size, see `regSize()`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t virtSize() const noexcept { return _virtSize; }

  //! Returns the virtual register alignment.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t alignment() const noexcept { return _alignment; }

  //! Returns the virtual register type id.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG TypeId typeId() const noexcept { return _typeId; }

  //! Returns the virtual register weight - the register allocator can use it as explicit hint for alloc/spill
  //! decisions.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t weight() const noexcept { return _weight; }

  //! Sets the virtual register weight (0 to 255) - the register allocator can use it as explicit hint for
  //! alloc/spill decisions and initial bin-packing.
  ASMJIT_INLINE_NODEBUG void setWeight(uint32_t weight) noexcept { _weight = uint8_t(weight); }

  //! Returns whether the virtual register is always allocated to a fixed physical register (and never reallocated).
  //!
  //! \note This is only used for special purposes and it's mostly internal.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool isFixed() const noexcept { return bool(_isFixed); }

  //! Tests whether the virtual register is in fact a stack that only uses the virtual register id.
  //!
  //! \note It's an error if a stack is accessed as a register.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool isStack() const noexcept { return bool(_isStack); }

  //! Tests whether this virtual register (or stack) has assigned a stack offset.
  //!
  //! If this is a virtual register that was never allocated on stack, it would return false, otherwise if
  //! it's a virtual register that was spilled or explicitly allocated stack, the return value would be true.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool hasStackSlot() const noexcept { return bool(_hasStackSlot); }

  //! Assigns a stack offset of this virtual register to `stackOffset` and sets `_hasStackSlot` to true.
  ASMJIT_INLINE_NODEBUG void assignStackSlot(int32_t stackOffset) noexcept {
    _hasStackSlot = 1;
    _stackOffset = stackOffset;
  }

  //! Tests whether this virtual register has assigned a physical register as a hint to the register allocator.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool hasHomeIdHint() const noexcept { return _homeIdHint != BaseReg::kIdBad; }

  //! Returns a physical register hint, which will be used by the register allocator.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t homeIdHint() const noexcept { return _homeIdHint; }

  //! Assigns a physical register hint, which will be used by the register allocator.
  ASMJIT_INLINE_NODEBUG void setHomeIdHint(uint32_t homeId) noexcept { _homeIdHint = uint8_t(homeId); }
  //! Resets a physical register hint.
  ASMJIT_INLINE_NODEBUG void resetHomeIdHint() noexcept { _homeIdHint = BaseReg::kIdBad; }

  //! Returns a stack offset associated with a virtual register or explicit stack allocation.
  //!
  //! \note Always verify that the stack offset has been assigned by calling \ref hasStackSlot(). The return
  //! value will be zero when the stack offset was not assigned.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG int32_t stackOffset() const noexcept { return _stackOffset; }

  //! Tests whether the virtual register has an associated `RAWorkReg` at the moment.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool hasWorkReg() const noexcept { return _workReg != nullptr; }

  //! Returns an associated RAWorkReg with this virtual register (only valid during register allocation).
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RAWorkReg* workReg() const noexcept { return _workReg; }

  //! Associates a RAWorkReg with this virtual register (used by register allocator).
  ASMJIT_INLINE_NODEBUG void setWorkReg(RAWorkReg* workReg) noexcept { _workReg = workReg; }

  //! Reset the RAWorkReg association (used by register allocator).
  ASMJIT_INLINE_NODEBUG void resetWorkReg() noexcept { _workReg = nullptr; }

  //! \}
};

//! \}

ASMJIT_END_NAMESPACE

#endif // ASMJIT_CORE_COMPILERDEFS_H_INCLUDED

