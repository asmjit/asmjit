// [AsmJit]
// Machine Code Generation for C++.
//
// [License]
// ZLIB - See LICENSE.md file in the package.

#ifndef _ASMJIT_ARM_ARMOPERAND_H
#define _ASMJIT_ARM_ARMOPERAND_H

#include "../core/arch.h"
#include "../core/operand.h"
#include "../core/type.h"
#include "../arm/armglobals.h"

ASMJIT_BEGIN_SUB_NAMESPACE(arm)

//! \addtogroup asmjit_arm_api
//! \{

// ============================================================================
// [Forward Declarations]
// ============================================================================

class Reg;
class Mem;

class Gp;
class Gpw;
class Gpx;

class Vec;
class VecS;
class VecD;
class VecV;

// ============================================================================
// [asmjit::arm::RegTraits]
// ============================================================================

//! Register traits (ARM/AArch64).
//!
//! Register traits contains information about a particular register type. It's
//! used by asmjit to setup register information on-the-fly and to populate
//! tables that contain register information (this way it's possible to change
//! register types and groups without having to reorder these tables).
template<uint32_t REG_TYPE>
struct RegTraits : public BaseRegTraits {};

// <--------------------+-----+----------------------+---------------------+---+---+----------------+
//                      | Reg |       Reg-Type       |      Reg-Group      |Sz |Cnt|     TypeId     |
// <--------------------+-----+----------------------+---------------------+---+---+----------------+
ASMJIT_DEFINE_REG_TRAITS(Gpw  , BaseReg::kTypeGp32   , BaseReg::kGroupGp   , 4 , 32, Type::kIdI32   );
ASMJIT_DEFINE_REG_TRAITS(Gpx  , BaseReg::kTypeGp64   , BaseReg::kGroupGp   , 8 , 32, Type::kIdI64   );
ASMJIT_DEFINE_REG_TRAITS(VecS , BaseReg::kTypeVec32  , BaseReg::kGroupVec  , 4 , 32, Type::kIdI32x1 );
ASMJIT_DEFINE_REG_TRAITS(VecD , BaseReg::kTypeVec64  , BaseReg::kGroupVec  , 8 , 32, Type::kIdI32x2 );
ASMJIT_DEFINE_REG_TRAITS(VecV , BaseReg::kTypeVec128 , BaseReg::kGroupVec  , 16, 32, Type::kIdI32x4 );

// ============================================================================
// [asmjit::arm::Reg]
// ============================================================================

//! Register (ARM).
class Reg : public BaseReg {
public:
  ASMJIT_DEFINE_ABSTRACT_REG(Reg, BaseReg)

  //! Register type.
  enum RegType : uint32_t {
    kTypeNone    = BaseReg::kTypeNone,   //!< No register type or invalid register.
    kTypeGpw     = BaseReg::kTypeGp32,   //!< 32-bit general purpose register (R or W).
    kTypeGpx     = BaseReg::kTypeGp64,   //!< 64-bit general purpose register (X).
    kTypeVecS    = BaseReg::kTypeVec32,  //!< 32-bit view of VFP/ASIMD register (S).
    kTypeVecD    = BaseReg::kTypeVec64,  //!< 64-bit view of VFP/ASIMD register (D).
    kTypeVecV    = BaseReg::kTypeVec128, //!< 128-bit view of VFP/ASIMD register (Q|V).
    kTypeIP      = BaseReg::kTypeIP,     //!< Instruction pointer (A64).
    kTypeCount                           //!< Count of register types.
  };

  //! Register group.
  enum RegGroup : uint32_t {
    kGroupGp     = BaseReg::kGroupGp,    //!< General purpose register group.
    kGroupVec    = BaseReg::kGroupVec,   //!< Vector (VFP/ASIMD) register group.
    kGroupCount                          //!< Count of all ARM register groups.
  };

  //! Gets whether the register is a GP register of any size.
  constexpr bool isGp() const noexcept { return _reg.group == kGroupGp; }
  //! Gets whether the register is a VFP/ASIMD register of any size.
  constexpr bool isVec() const noexcept { return _reg.group == kGroupVec; }

  //! Gets whether the register is a `R|W` register (32-bit).
  constexpr bool isGpw() const noexcept { return hasSignature(RegTraits<kTypeGpw>::kSignature); }
  //! Gets whether the register is an `X` register (64-bit).
  constexpr bool isGpx() const noexcept { return hasSignature(RegTraits<kTypeGpx>::kSignature); }

  //! Gets whether the register is a VEC-S register (32-bit).
  constexpr bool isVecS() const noexcept { return hasSignature(RegTraits<kTypeVecS>::kSignature); }
  //! Gets whether the register is a VEC-D register (64-bit).
  constexpr bool isVecD() const noexcept { return hasSignature(RegTraits<kTypeVecD>::kSignature); }
  //! Gets whether the register is a VEC-V register (128-bit).
  constexpr bool isVecV() const noexcept { return hasSignature(RegTraits<kTypeVecV>::kSignature); }

  template<uint32_t REG_TYPE>
  inline void setArmRegT(uint32_t id) noexcept {
    setSignature(RegTraits<REG_TYPE>::kSignature);
    setId(id);
  }

  inline void setTypeAndId(uint32_t regType, uint32_t id) noexcept {
    ASMJIT_ASSERT(regType < kTypeCount);
    setSignature(signatureOf(regType));
    setId(id);
  }

  static inline uint32_t groupOf(uint32_t regType) noexcept;
  template<uint32_t REG_TYPE>
  static inline uint32_t groupOfT() noexcept { return RegTraits<REG_TYPE>::kGroup; }

  static inline uint32_t signatureOf(uint32_t regType) noexcept;
  template<uint32_t REG_TYPE>
  static inline uint32_t signatureOfT() noexcept { return RegTraits<REG_TYPE>::kSignature; }

  static inline bool isGpw(const Operand_& op) noexcept { return op.as<Reg>().isGpW(); }
  static inline bool isGpx(const Operand_& op) noexcept { return op.as<Reg>().isGpX(); }
  static inline bool isGpw(const Operand_& op, uint32_t id) noexcept { return isGpW(op) & (op.id() == id); }
  static inline bool isGpx(const Operand_& op, uint32_t id) noexcept { return isGpX(op) & (op.id() == id); }

  static inline bool isVecS(const Operand_& op) noexcept { return op.as<Reg>().isVecS(); }
  static inline bool isVecD(const Operand_& op) noexcept { return op.as<Reg>().isVecD(); }
  static inline bool isVecV(const Operand_& op) noexcept { return op.as<Reg>().isVecV(); }
  static inline bool isVecS(const Operand_& op, uint32_t id) noexcept { return isVecS(op) & (op.id() == id); }
  static inline bool isVecD(const Operand_& op, uint32_t id) noexcept { return isVecD(op) & (op.id() == id); }
  static inline bool isVecV(const Operand_& op, uint32_t id) noexcept { return isVecV(op) & (op.id() == id); }
};

//! General purpose register (ARM).
class Gp : public Reg {
public:
  ASMJIT_DEFINE_ABSTRACT_REG(Gp, Reg)

  //! Cast this register to a 32-bit R|W.
  constexpr Gpw w() const noexcept;
  //! Cast this register to a 64-bit X.
  constexpr Gpx x() const noexcept;
};

//! Vector register (ARM).
class Vec : public Reg {
public:
  ASMJIT_DEFINE_ABSTRACT_REG(Vec, Reg)

  //! Cast this register to a 32-bit S register.
  constexpr VecS s() const noexcept;
  //! Cast this register to a 64-bit D register.
  constexpr VecD d() const noexcept;
  //! Cast this register to a 128-bit V register.
  constexpr VecV v() const noexcept;
};

//! 32-bit GPW (AArch64) and/or GPR (ARM/AArch32) register.
class Gpw : public Gp { ASMJIT_DEFINE_FINAL_REG(Gpw, Gp, RegTraits<kTypeGpw>) };
//! 64-bit GPX (AArch64) register.
class Gpx : public Gp { ASMJIT_DEFINE_FINAL_REG(Gpx, Gp, RegTraits<kTypeGpx>) };
//! 32-bit view (S) of VFP/SIMD register.
class VecS : public Vec { ASMJIT_DEFINE_FINAL_REG(VecS, Vec, RegTraits<kTypeVecS>) };
//! 64-bit view (D) of VFP/SIMD register.
class VecD : public Vec { ASMJIT_DEFINE_FINAL_REG(VecD, Vec, RegTraits<kTypeVecD>) };
//! 128-bit vector register (Q or V).
class VecV : public Vec { ASMJIT_DEFINE_FINAL_REG(VecV, Vec, RegTraits<kTypeVecV>) };

constexpr Gpw Gp::w() const noexcept { return Gpw(id()); }
constexpr Gpx Gp::x() const noexcept { return Gpx(id()); }
constexpr VecS Vec::s() const noexcept { return VecS(id()); }
constexpr VecD Vec::d() const noexcept { return VecD(id()); }
constexpr VecV Vec::v() const noexcept { return VecV(id()); }

ASMJIT_DEFINE_TYPE_ID(Gpw, kIdI32);
ASMJIT_DEFINE_TYPE_ID(Gpx, kIdI64);
ASMJIT_DEFINE_TYPE_ID(VecS, kIdF32x1);
ASMJIT_DEFINE_TYPE_ID(VecD, kIdF64x1);
ASMJIT_DEFINE_TYPE_ID(VecV, kIdI32x4);

// ============================================================================
// [asmjit::arm::Mem]
// ============================================================================

//! Memory operand (ARM).
class Mem : public BaseMem {
public:
  //! Additional bits of operand's signature used by `Mem`.
  enum AdditionalBits : uint32_t {
    kSignatureMemShiftShift   = 16,
    kSignatureMemShiftMask    = 0x1Fu << kSignatureMemShiftShift,

    kSignatureMemModeShift    = 21,
    kSignatureMemModeMask     = 0x03u << kSignatureMemModeShift,
  };

  //! Memory offset mode.
  enum OffsetMode : uint32_t {
    kOffsetFixed              = 0,       //!< Address + offset "[BASE, #Offset]".
    kOffsetPreInc             = 1,       //!< Pre-increment    "[BASE, #Offset]!".
    kOffsetPostInc            = 2        //!< Post-increment   "[BASE], #Offset".
  };

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Construct a default `Mem` operand, that points to [0].
  constexpr Mem() noexcept : BaseMem() {}
  constexpr Mem(const Mem& other) noexcept : BaseMem(other) {}
  inline explicit Mem(Globals::NoInit_) noexcept : BaseMem(Globals::NoInit) {}

  // --------------------------------------------------------------------------
  // [Arm Specific]
  // --------------------------------------------------------------------------

  //! Clones the memory operand.
  constexpr Mem clone() const noexcept { return Mem(*this); }
  //! Gets new memory operand adjusted by `off`.
  inline Mem cloneAdjusted(int64_t off) const noexcept {
    Mem result(*this);
    result.addOffset(off);
    return result;
  }

  using BaseMem::setIndex;

  inline void setIndex(const BaseReg& index, uint32_t shift) noexcept {
    setIndex(index);
    setShift(shift);
  }

  //! Gets whether the memory operand has shift (aka scale) constant.
  constexpr bool hasShift() const noexcept { return _hasSignaturePart<kSignatureMemShiftMask>(); }
  //! Gets the memory operand's shift (aka scale) constant.
  constexpr uint32_t shift() const noexcept { return _getSignaturePart<kSignatureMemShiftMask>(); }
  //! Sets the memory operand's shift (aka scale) constant.
  inline void setShift(uint32_t shift) noexcept { _setSignaturePart<kSignatureMemShiftMask>(shift); }
  //! Resets the memory operand's shift (aka scale) constant to zero.
  inline void resetShift() noexcept { _setSignaturePart<kSignatureMemShiftMask>(0); }

  //! Gets the addressing mode, see `Mem::OffsetMode`.
  constexpr uint32_t offsetMode() const noexcept { return _getSignaturePart<kSignatureMemModeMask>(); }
  //! Sets the addressing mode, see `Mem::OffsetMode`.
  inline void setOffsetMode(uint32_t mode) noexcept { _setSignaturePart<kSignatureMemModeMask>(mode); }
  //! Resets the addressing mode to `Mem::OffsetMode`.
  inline void resetOffsetMode() noexcept { _setSignaturePart<kSignatureMemModeMask>(kOffsetFixed); }

  constexpr bool isFixedOffset() const noexcept { return offsetMode() == kOffsetFixed; }
  constexpr bool isPreIncOffset() const noexcept { return offsetMode() == kOffsetPreInc; }
  constexpr bool isPostIncOffset() const noexcept { return offsetMode() == kOffsetPostInc; }

  inline Mem pre() const noexcept {
    Mem result(*this);
    result.setOffsetMode(kOffsetPreInc);
    return result;
  }

  inline Mem pre(int64_t off) const noexcept {
    Mem result(*this);
    result.setOffsetMode(kOffsetPreInc);
    result.addOffset(off);
    return result;
  }

  inline Mem post() const noexcept {
    Mem result(*this);
    result.setOffsetMode(kOffsetPreInc);
    return result;
  }

  inline Mem post(int64_t off) const noexcept {
    Mem result(*this);
    result.setOffsetMode(kOffsetPostInc);
    result.addOffset(off);
    return result;
  }

  // --------------------------------------------------------------------------
  // [Operator Overload]
  // --------------------------------------------------------------------------

  inline Mem& operator=(const Mem& other) noexcept = default;
};

// ============================================================================
// [asmjit::ArmOpData]
// ============================================================================

struct ArmOpData {
  //! Information about all architecture registers.
  ArchRegs archRegs;
};
ASMJIT_VARAPI const ArmOpData armOpData;

// ... Reg methods that require `armOpData`.
inline uint32_t Reg::signatureOf(uint32_t regType) noexcept {
  ASMJIT_ASSERT(regType <= BaseReg::kTypeMax);
  return armOpData.archRegs.regInfo[regType].signature;
}

inline uint32_t Reg::groupOf(uint32_t regType) noexcept {
  ASMJIT_ASSERT(regType <= BaseReg::kTypeMax);
  return armOpData.archRegs.regInfo[regType].group;
}

// ============================================================================
// [asmjit::arm::regs]
// ============================================================================

namespace regs {
namespace {

//! Creates a 32-bit W register operand (ARM/AArch64).
static constexpr Gpw w(uint32_t id) noexcept { return Gpw(id); }
//! Creates a 64-bit X register operand (AArch64).
static constexpr Gpx x(uint32_t id) noexcept { return Gpx(id); }
//! Creates a 32-bit S register operand (ARM/AArch64).
static constexpr VecS s(uint32_t id) noexcept { return VecS(id); }
//! Creates a 64-bit D register operand (ARM/AArch64).
static constexpr VecD d(uint32_t id) noexcept { return VecD(id); }
//! Creates a 1282-bit V register operand (ARM/AArch64).
static constexpr VecV v(uint32_t id) noexcept { return VecV(id); }

static constexpr Gpw w0(0);
static constexpr Gpw w1(1);
static constexpr Gpw w2(2);
static constexpr Gpw w3(3);
static constexpr Gpw w4(4);
static constexpr Gpw w5(5);
static constexpr Gpw w6(6);
static constexpr Gpw w7(7);
static constexpr Gpw w8(8);
static constexpr Gpw w9(9);
static constexpr Gpw w10(10);
static constexpr Gpw w11(11);
static constexpr Gpw w12(12);
static constexpr Gpw w13(13);
static constexpr Gpw w14(14);
static constexpr Gpw w15(15);
static constexpr Gpw w16(16);
static constexpr Gpw w17(17);
static constexpr Gpw w18(18);
static constexpr Gpw w19(19);
static constexpr Gpw w20(20);
static constexpr Gpw w21(21);
static constexpr Gpw w22(22);
static constexpr Gpw w23(23);
static constexpr Gpw w24(24);
static constexpr Gpw w25(25);
static constexpr Gpw w26(26);
static constexpr Gpw w27(27);
static constexpr Gpw w28(28);
static constexpr Gpw w29(29);
static constexpr Gpw w30(30);
static constexpr Gpw w31(31);

static constexpr Gpx x0(0);
static constexpr Gpx x1(1);
static constexpr Gpx x2(2);
static constexpr Gpx x3(3);
static constexpr Gpx x4(4);
static constexpr Gpx x5(5);
static constexpr Gpx x6(6);
static constexpr Gpx x7(7);
static constexpr Gpx x8(8);
static constexpr Gpx x9(9);
static constexpr Gpx x10(10);
static constexpr Gpx x11(11);
static constexpr Gpx x12(12);
static constexpr Gpx x13(13);
static constexpr Gpx x14(14);
static constexpr Gpx x15(15);
static constexpr Gpx x16(16);
static constexpr Gpx x17(17);
static constexpr Gpx x18(18);
static constexpr Gpx x19(19);
static constexpr Gpx x20(20);
static constexpr Gpx x21(21);
static constexpr Gpx x22(22);
static constexpr Gpx x23(23);
static constexpr Gpx x24(24);
static constexpr Gpx x25(25);
static constexpr Gpx x26(26);
static constexpr Gpx x27(27);
static constexpr Gpx x28(28);
static constexpr Gpx x29(29);
static constexpr Gpx x30(30);
static constexpr Gpx x31(31);

} // {anonymous}
} // {regs}

// ============================================================================
// [asmjit::arm - Ptr]
// ============================================================================

//! \}

ASMJIT_END_SUB_NAMESPACE

#endif // _ASMJIT_ARM_ARMOPERAND_H
