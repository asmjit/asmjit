// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_ARM_ARMOPERAND_H
#define _ASMJIT_ARM_ARMOPERAND_H

// [Dependencies]
#include "../core/arch.h"
#include "../core/operand.h"
#include "../core/type.h"
#include "../arm/armglobals.h"

ASMJIT_BEGIN_NAMESPACE

// ============================================================================
// [Forward Declarations]
// ============================================================================

class ArmReg;
class ArmMem;

class ArmGp;
class ArmGpw;
class ArmGpx;

class ArmVec;
class ArmVecS;
class ArmVecD;
class ArmVecV;

//! \addtogroup asmjit_arm
//! \{

// ============================================================================
// [asmjit::ArmRegTraits]
// ============================================================================

//! Register traits (ARM/AArch64).
//!
//! Register traits contains information about a particular register type. It's
//! used by asmjit to setup register information on-the-fly and to populate
//! tables that contain register information (this way it's possible to change
//! register types and groups without having to reorder these tables).
template<uint32_t REG_TYPE>
struct ArmRegTraits {
  enum : uint32_t {
    kValid     = 0,                      //!< RegType is not valid by default.
    kCount     = 0,                      //!< Count of registers (0 if none).
    kTypeId    = Type::kIdVoid,          //!< Everything is void by default.

    kType      = 0,                      //!< Zero type by default.
    kGroup     = 0,                      //!< Zero group by default.
    kSize      = 0,                      //!< No size by default.
    kSignature = 0                       //!< No signature by default.
  };
};

// <--------------------+------------+---------+--------------------+--------------------+---+---+----------------+
//                      |  Traits-T  |  Reg-T  |      Reg-Type      |      Reg-Group     |Sz |Cnt|     TypeId     |
// <--------------------+------------+---------+--------------------+--------------------+---+---+----------------+
ASMJIT_DEFINE_REG_TRAITS(ArmRegTraits, ArmGpw  , Reg::kRegGp32      , Reg::kGroupGp      , 4 , 32, Type::kIdI32   );
ASMJIT_DEFINE_REG_TRAITS(ArmRegTraits, ArmGpx  , Reg::kRegGp64      , Reg::kGroupGp      , 8 , 32, Type::kIdI64   );
ASMJIT_DEFINE_REG_TRAITS(ArmRegTraits, ArmVecS , Reg::kRegVec32     , Reg::kGroupVec     , 4 , 32, Type::kIdI32x1 );
ASMJIT_DEFINE_REG_TRAITS(ArmRegTraits, ArmVecD , Reg::kRegVec64     , Reg::kGroupVec     , 8 , 32, Type::kIdI32x2 );
ASMJIT_DEFINE_REG_TRAITS(ArmRegTraits, ArmVecV , Reg::kRegVec128    , Reg::kGroupVec     , 16, 32, Type::kIdI32x4 );

// ============================================================================
// [asmjit::ArmReg]
// ============================================================================

//! Register (ARM/AArch64).
class ArmReg : public Reg {
public:
  ASMJIT_DEFINE_ABSTRACT_REG(ArmReg, Reg)

  //! Register type.
  enum RegType : uint32_t {
    kRegNone      = Reg::kRegNone,       //!< No register type or invalid register.
    kRegGpw       = Reg::kRegGp32,       //!< 32-bit general purpose register (R or W).
    kRegGpx       = Reg::kRegGp64,       //!< 64-bit general purpose register (X).
    kRegVecS      = Reg::kRegVec32,      //!< 32-bit view of VFP/ASIMD register (S).
    kRegVecD      = Reg::kRegVec64,      //!< 64-bit view of VFP/ASIMD register (D).
    kRegVecV      = Reg::kRegVec128,     //!< 128-bit view of VFP/ASIMD register (Q|V).
    kRegIP        = Reg::kRegIP,         //!< Instruction pointer (A64).
    kRegCount                            //!< Count of register types.
  };

  //! Register group.
  enum Group : uint32_t {
    kGroupGp       = Reg::kGroupGp,      //!< General purpose register group.
    kGroupVec      = Reg::kGroupVec,     //!< Vector (VFP/ASIMD) register group.
    kGroupCount                          //!< Count of all ARM register groups.
  };

  //! Get whether the register is a GP register of any size.
  constexpr bool isGp() const noexcept { return _reg.group == kGroupGp; }
  //! Get whether the register is a VFP/ASIMD register of any size.
  constexpr bool isVec() const noexcept { return _reg.group == kGroupVec; }

  //! Get whether the register is a `R|W` register (32-bit).
  constexpr bool isGpw() const noexcept { return hasSignature(X86RegTraits<kRegGpw>::kSignature); }
  //! Get whether the register is an `X` register (64-bit).
  constexpr bool isGpx() const noexcept { return hasSignature(X86RegTraits<kRegGpx>::kSignature); }

  //! Get whether the register is a VEC-S register (32-bit).
  constexpr bool isVecS() const noexcept { return hasSignature(X86RegTraits<kRegVecS>::kSignature); }
  //! Get whether the register is a VEC-D register (64-bit).
  constexpr bool isVecD() const noexcept { return hasSignature(X86RegTraits<kRegVecD>::kSignature); }
  //! Get whether the register is a VEC-V register (128-bit).
  constexpr bool isVecV() const noexcept { return hasSignature(X86RegTraits<kRegVecV>::kSignature); }

  template<uint32_t REG_TYPE>
  inline void setArmRegT(uint32_t id) noexcept {
    setSignature(ArmRegTraits<REG_TYPE>::kSignature);
    setId(id);
  }

  inline void setTypeAndId(uint32_t regType, uint32_t id) noexcept {
    ASMJIT_ASSERT(regType < kRegCount);
    setSignature(signatureOf(regType));
    setId(id);
  }

  static inline uint32_t groupOf(uint32_t regType) noexcept;
  template<uint32_t REG_TYPE>
  static inline uint32_t groupOfT() noexcept { return ArmRegTraits<REG_TYPE>::kGroup; }

  static inline uint32_t signatureOf(uint32_t regType) noexcept;
  template<uint32_t REG_TYPE>
  static inline uint32_t signatureOfT() noexcept { return ArmRegTraits<REG_TYPE>::kSignature; }

  static inline bool isGpw(const Operand_& op) noexcept { return op.as<ArmReg>().isGpW(); }
  static inline bool isGpx(const Operand_& op) noexcept { return op.as<ArmReg>().isGpX(); }
  static inline bool isGpw(const Operand_& op, uint32_t id) noexcept { return isGpW(op) & (op.getId() == id); }
  static inline bool isGpx(const Operand_& op, uint32_t id) noexcept { return isGpX(op) & (op.getId() == id); }

  static inline bool isVecS(const Operand_& op) noexcept { return op.as<ArmReg>().isVecS(); }
  static inline bool isVecD(const Operand_& op) noexcept { return op.as<ArmReg>().isVecD(); }
  static inline bool isVecV(const Operand_& op) noexcept { return op.as<ArmReg>().isVecV(); }
  static inline bool isVecS(const Operand_& op, uint32_t id) noexcept { return isVecS(op) & (op.getId() == id); }
  static inline bool isVecD(const Operand_& op, uint32_t id) noexcept { return isVecD(op) & (op.getId() == id); }
  static inline bool isVecV(const Operand_& op, uint32_t id) noexcept { return isVecV(op) & (op.getId() == id); }
};

//! General purpose register (ARM/AArch64).
class ArmGp : public ArmReg {
public:
  ASMJIT_DEFINE_ABSTRACT_REG(ArmGp, ArmReg)

  //! Cast this register to a 32-bit R|W.
  constexpr ArmGpw w() const noexcept;
  //! Cast this register to a 64-bit X.
  constexpr ArmGpx x() const noexcept;
};

//! Vector register (ARM/AArch64).
class ArmVec : public ArmReg {
public:
  ASMJIT_DEFINE_ABSTRACT_REG(ArmVec, ArmReg)

  //! Cast this register to a 32-bit S register.
  constexpr ArmVecS s() const noexcept;
  //! Cast this register to a 64-bit D register.
  constexpr ArmVecD d() const noexcept;
  //! Cast this register to a 128-bit V register.
  constexpr ArmVecV v() const noexcept;
};

//! 32-bit GPW (AArch64) and/or GPR (ARM/AArch32) register.
class ArmGpw : public ArmGp { ASMJIT_DEFINE_FINAL_REG(ArmGpw, ArmGp, ArmRegTraits<kRegGpw>) };
//! 64-bit GPX (AArch64) register.
class ArmGpx : public ArmGp { ASMJIT_DEFINE_FINAL_REG(ArmGpx, ArmGp, ArmRegTraits<kRegGpx>) };
//! 32-bit view (S) of VFP/SIMD register.
class ArmVecS : public ArmVec { ASMJIT_DEFINE_FINAL_REG(ArmVecS, ArmVec, ArmRegTraits<kRegVecS>) };
//! 64-bit view (D) of VFP/SIMD register.
class ArmVecD : public ArmVec { ASMJIT_DEFINE_FINAL_REG(ArmVecD, ArmVec, ArmRegTraits<kRegVecD>) };
//! 128-bit vector register (Q or V).
class ArmVecV : public ArmVec { ASMJIT_DEFINE_FINAL_REG(ArmVecV, ArmVec, ArmRegTraits<kRegVecV>) };

constexpr ArmGpw ArmGp::w() const noexcept { return ArmGpw(getId()); }
constexpr ArmGpx ArmGp::x() const noexcept { return ArmGpx(getId()); }
constexpr ArmVecS ArmVec::s() const noexcept { return ArmVecS(getId()); }
constexpr ArmVecD ArmVec::d() const noexcept { return ArmVecD(getId()); }
constexpr ArmVecV ArmVec::v() const noexcept { return ArmVecV(getId()); }

ASMJIT_DEFINE_TYPE_ID(ArmGpw, kIdI32);
ASMJIT_DEFINE_TYPE_ID(ArmGpx, kIdI64);
ASMJIT_DEFINE_TYPE_ID(ArmVecS, kIdF32x1);
ASMJIT_DEFINE_TYPE_ID(ArmVecD, kIdF64x1);
ASMJIT_DEFINE_TYPE_ID(ArmVecV, kIdI32x4);

// ============================================================================
// [asmjit::ArmMem]
// ============================================================================

//! Memory operand (ARM/AArch64).
class ArmMem : public Mem {
public:
  //! Additional bits of operand's signature used by `ArmMem`.
  enum AdditionalBits : uint32_t {
    kSignatureMemShiftShift   = 16,
    kSignatureMemShiftBits    = 0x1FU,
    kSignatureMemShiftMask    = kSignatureMemShiftBits << kSignatureMemShiftShift,

    kSignatureMemModeShift    = 21,
    kSignatureMemModeBits     = 0x03U,
    kSignatureMemModeMask     = kSignatureMemSegmentBits << kSignatureMemSegmentShift,
  };

  //! Memory addressing mode.
  enum Mode : uint32_t {
    kModeOffset               = 0,       //!< Address + offset "[BASE, #Offset]".
    kModePreInc               = 1,       //!< Pre-increment    "[BASE, #Offset]!".
    kModePostInc              = 2        //!< Post-increment   "[BASE], #Offset".
  };

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Construct a default `ArmMem` operand, that points to [0].
  constexpr ArmMem() noexcept : Mem() {}
  constexpr ArmMem(const ArmMem& other) noexcept : Mem(other) {}
  explicit inline ArmMem(Globals::NoInit_) noexcept : Mem(Globals::NoInit) {}

  // --------------------------------------------------------------------------
  // [ArmMem]
  // --------------------------------------------------------------------------

  //! Clone the memory operand.
  constexpr ArmMem clone() const noexcept { return ArmMem(*this); }
  //! Get new memory operand adjusted by `off`.
  inline ArmMem cloneAdjusted(int64_t off) const noexcept {
    ArmMem result(*this);
    result.addOffset(off);
    return result;
  }

  using Mem::setIndex;

  inline void setIndex(const Reg& index, uint32_t shift) noexcept {
    setIndex(index);
    setShift(shift);
  }

  //! Get whether the memory operand has shift (aka scale) constant.
  constexpr bool hasShift() const noexcept { return _hasSignatureData(kSignatureMemShiftMask); }
  //! Get the memory operand's shift (aka scale) constant.
  constexpr uint32_t getShift() const noexcept { return _getSignatureData(kSignatureMemShiftBits, kSignatureMemShiftShift); }
  //! Set the memory operand's shift (aka scale) constant.
  inline void setShift(uint32_t shift) noexcept { _setSignatureData(shift, kSignatureMemShiftBits, kSignatureMemShiftShift); }
  //! Reset the memory operand's shift (aka scale) constant to zero.
  inline void resetShift() noexcept { _any.signature &= ~kSignatureMemShiftMask; }

  //! Get the addressing mode, see \ref ArmMem::Mode.
  constexpr uint32_t getMode() const noexcept { return _getSignatureData(kSignatureMemModeBits, kSignatureMemModeShift); }
  //! Set the addressing mode, see \ref ArmMem::Mode.
  inline void setMode(uint32_t mode) noexcept { _setSignatureData(mode, kSignatureMemModeBits, kSignatureMemModeShift); }
  //! Reset the addressing mode to \ref ArmMem::kModeOffset.
  inline void resetMode() noexcept { _any.signature &= ~kSignatureMemModeMask; }

  constexpr bool isOffsetMode() const noexcept { return getMode() == kModeOffset; }
  constexpr bool isPreIncMode() const noexcept { return getMode() == kModePreInc; }
  constexpr bool isPostIncMode() const noexcept { return getMode() == kModePostInc; }

  inline ArmMem pre() const noexcept {
    ArmMem result(*this);
    result.setMode(kModePreInc);
    return result;
  }

  inline ArmMem pre(int64_t off) const noexcept {
    ArmMem result(*this);
    result.setMode(kModePreInc);
    result.addOffset(off);
    return result;
  }

  inline ArmMem post() const noexcept {
    ArmMem result(*this);
    result.setMode(kModePreInc);
    return result;
  }

  inline ArmMem post(int64_t off) const noexcept {
    ArmMem result(*this);
    result.setMode(kModePostInc);
    result.addOffset(off);
    return result;
  }

  // --------------------------------------------------------------------------
  // [Operator Overload]
  // --------------------------------------------------------------------------

  inline ArmMem& operator=(const ArmMem& other) noexcept = default;
};

// ============================================================================
// [asmjit::ArmOpData]
// ============================================================================

struct ArmOpData {
  //! Information about all architecture registers.
  ArchRegs archRegs;
};
ASMJIT_VARAPI const ArmOpData armOpData;

// ... ArmReg methods that require `armOpData`.
inline uint32_t ArmReg::signatureOf(uint32_t regType) noexcept {
  ASMJIT_ASSERT(regType <= Reg::kRegMax);
  return armOpData.archRegs.regInfo[regType].signature;
}

inline uint32_t ArmReg::groupOf(uint32_t regType) noexcept {
  ASMJIT_ASSERT(regType <= Reg::kRegMax);
  return armOpData.archRegs.regInfo[regType].group;
}

// ============================================================================
// [asmjit::arm::regs]
// ============================================================================

namespace arm {
namespace regs {
namespace {

//! Create a 32-bit W register operand (ARM/AArch64).
static constexpr ArmGpw w(uint32_t id) noexcept { return ArmGpw(id); }
//! Create a 64-bit X register operand (AArch64).
static constexpr ArmGpx x(uint32_t id) noexcept { return ArmGpx(id); }
//! Create a 32-bit S register operand (ARM/AArch64).
static constexpr ArmVecS s(uint32_t id) noexcept { return ArmVecS(id); }
//! Create a 64-bit D register operand (ARM/AArch64).
static constexpr ArmVecD d(uint32_t id) noexcept { return ArmVecD(id); }
//! Create a 1282-bit V register operand (ARM/AArch64).
static constexpr ArmVecV v(uint32_t id) noexcept { return ArmVecV(id); }

constexpr ArmGpw w0(0);
constexpr ArmGpw w1(1);
constexpr ArmGpw w2(2);
constexpr ArmGpw w3(3);
constexpr ArmGpw w4(4);
constexpr ArmGpw w5(5);
constexpr ArmGpw w6(6);
constexpr ArmGpw w7(7);
constexpr ArmGpw w8(8);
constexpr ArmGpw w9(9);
constexpr ArmGpw w10(10);
constexpr ArmGpw w11(11);
constexpr ArmGpw w12(12);
constexpr ArmGpw w13(13);
constexpr ArmGpw w14(14);
constexpr ArmGpw w15(15);
constexpr ArmGpw w16(16);
constexpr ArmGpw w17(17);
constexpr ArmGpw w18(18);
constexpr ArmGpw w19(19);
constexpr ArmGpw w20(20);
constexpr ArmGpw w21(21);
constexpr ArmGpw w22(22);
constexpr ArmGpw w23(23);
constexpr ArmGpw w24(24);
constexpr ArmGpw w25(25);
constexpr ArmGpw w26(26);
constexpr ArmGpw w27(27);
constexpr ArmGpw w28(28);
constexpr ArmGpw w29(29);
constexpr ArmGpw w30(30);
constexpr ArmGpw w31(31);

constexpr ArmGpx x0(0);
constexpr ArmGpx x1(1);
constexpr ArmGpx x2(2);
constexpr ArmGpx x3(3);
constexpr ArmGpx x4(4);
constexpr ArmGpx x5(5);
constexpr ArmGpx x6(6);
constexpr ArmGpx x7(7);
constexpr ArmGpx x8(8);
constexpr ArmGpx x9(9);
constexpr ArmGpx x10(10);
constexpr ArmGpx x11(11);
constexpr ArmGpx x12(12);
constexpr ArmGpx x13(13);
constexpr ArmGpx x14(14);
constexpr ArmGpx x15(15);
constexpr ArmGpx x16(16);
constexpr ArmGpx x17(17);
constexpr ArmGpx x18(18);
constexpr ArmGpx x19(19);
constexpr ArmGpx x20(20);
constexpr ArmGpx x21(21);
constexpr ArmGpx x22(22);
constexpr ArmGpx x23(23);
constexpr ArmGpx x24(24);
constexpr ArmGpx x25(25);
constexpr ArmGpx x26(26);
constexpr ArmGpx x27(27);
constexpr ArmGpx x28(28);
constexpr ArmGpx x29(29);
constexpr ArmGpx x30(30);
constexpr ArmGpx x31(31);

} // anonymous namespace
} // regs namespace
} // arm namespace

// ============================================================================
// [asmjit::arm - Ptr]
// ============================================================================

//! \}

ASMJIT_END_NAMESPACE

// [Guard]
#endif // _ASMJIT_ARM_ARMOPERAND_H
