// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_X86_X86OPERAND_H
#define _ASMJIT_X86_X86OPERAND_H

// [Dependencies]
#include "../core/arch.h"
#include "../core/operand.h"
#include "../core/type.h"
#include "../x86/x86globals.h"

ASMJIT_BEGIN_NAMESPACE

// ============================================================================
// [Forward Declarations]
// ============================================================================

class X86Reg;
class X86Mem;

class X86Gp;
class X86Gpb;
class X86GpbLo;
class X86GpbHi;
class X86Gpw;
class X86Gpd;
class X86Gpq;
class X86Vec;
class X86Xmm;
class X86Ymm;
class X86Zmm;
class X86Mm;
class X86KReg;
class X86Seg;
class X86Rip;
class X86CReg;
class X86DReg;
class X86Fp;
class X86Bnd;

//! \addtogroup asmjit_x86
//! \{

// ============================================================================
// [asmjit::X86RegTraits]
// ============================================================================

//! Register traits (X86/X64).
//!
//! Register traits contains information about a particular register type. It's
//! used by asmjit to setup register information on-the-fly and to populate
//! tables that contain register information (this way it's possible to change
//! register types and groups without having to reorder these tables).
template<uint32_t REG_TYPE>
struct X86RegTraits {
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
ASMJIT_DEFINE_REG_TRAITS(X86RegTraits, X86GpbLo, Reg::kRegGp8Lo     , Reg::kGroupGp      , 1 , 16, Type::kIdI8    );
ASMJIT_DEFINE_REG_TRAITS(X86RegTraits, X86GpbHi, Reg::kRegGp8Hi     , Reg::kGroupGp      , 1 , 4 , Type::kIdI8    );
ASMJIT_DEFINE_REG_TRAITS(X86RegTraits, X86Gpw  , Reg::kRegGp16      , Reg::kGroupGp      , 2 , 16, Type::kIdI16   );
ASMJIT_DEFINE_REG_TRAITS(X86RegTraits, X86Gpd  , Reg::kRegGp32      , Reg::kGroupGp      , 4 , 16, Type::kIdI32   );
ASMJIT_DEFINE_REG_TRAITS(X86RegTraits, X86Gpq  , Reg::kRegGp64      , Reg::kGroupGp      , 8 , 16, Type::kIdI64   );
ASMJIT_DEFINE_REG_TRAITS(X86RegTraits, X86Xmm  , Reg::kRegVec128    , Reg::kGroupVec     , 16, 32, Type::kIdI32x4 );
ASMJIT_DEFINE_REG_TRAITS(X86RegTraits, X86Ymm  , Reg::kRegVec256    , Reg::kGroupVec     , 32, 32, Type::kIdI32x8 );
ASMJIT_DEFINE_REG_TRAITS(X86RegTraits, X86Zmm  , Reg::kRegVec512    , Reg::kGroupVec     , 64, 32, Type::kIdI32x16);
ASMJIT_DEFINE_REG_TRAITS(X86RegTraits, X86Mm   , Reg::kRegOther0    , Reg::kGroupOther0  , 8 , 8 , Type::kIdMmx64 );
ASMJIT_DEFINE_REG_TRAITS(X86RegTraits, X86KReg , Reg::kRegOther1    , Reg::kGroupOther1  , 0 , 8 , Type::kIdVoid  );
ASMJIT_DEFINE_REG_TRAITS(X86RegTraits, X86Rip  , Reg::kRegIP        , Reg::kGroupVirt + 0, 0 , 1 , Type::kIdVoid  );
ASMJIT_DEFINE_REG_TRAITS(X86RegTraits, X86Seg  , Reg::kRegCustom + 0, Reg::kGroupVirt + 1, 2 , 7 , Type::kIdVoid  );
ASMJIT_DEFINE_REG_TRAITS(X86RegTraits, X86CReg , Reg::kRegCustom + 1, Reg::kGroupVirt + 2, 0 , 16, Type::kIdVoid  );
ASMJIT_DEFINE_REG_TRAITS(X86RegTraits, X86DReg , Reg::kRegCustom + 2, Reg::kGroupVirt + 3, 0 , 16, Type::kIdVoid  );
ASMJIT_DEFINE_REG_TRAITS(X86RegTraits, X86Fp   , Reg::kRegCustom + 3, Reg::kGroupVirt + 4, 10, 8 , Type::kIdF80   );
ASMJIT_DEFINE_REG_TRAITS(X86RegTraits, X86Bnd  , Reg::kRegCustom + 4, Reg::kGroupVirt + 5, 16, 4 , Type::kIdVoid  );

// ============================================================================
// [asmjit::X86Reg]
// ============================================================================

//! Register (X86/X64).
class X86Reg : public Reg {
public:
  ASMJIT_DEFINE_ABSTRACT_REG(X86Reg, Reg)

  //! Register type.
  enum RegType : uint32_t {
    kRegNone      = Reg::kRegNone,       //!< No register type or invalid register.
    kRegGpbLo     = Reg::kRegGp8Lo,      //!< Low GPB register (AL, BL, CL, DL, ...).
    kRegGpbHi     = Reg::kRegGp8Hi,      //!< High GPB register (AH, BH, CH, DH only).
    kRegGpw       = Reg::kRegGp16,       //!< GPW register.
    kRegGpd       = Reg::kRegGp32,       //!< GPD register.
    kRegGpq       = Reg::kRegGp64,       //!< GPQ register (X64).
    kRegXmm       = Reg::kRegVec128,     //!< XMM register (SSE+).
    kRegYmm       = Reg::kRegVec256,     //!< YMM register (AVX+).
    kRegZmm       = Reg::kRegVec512,     //!< ZMM register (AVX512+).
    kRegMm        = Reg::kRegOther0,     //!< MMX register.
    kRegK         = Reg::kRegOther1,     //!< K register (AVX512+).
    kRegRip       = Reg::kRegIP,         //!< Instruction pointer (EIP, RIP).
    kRegSeg       = Reg::kRegCustom + 0, //!< Segment register (None, ES, CS, SS, DS, FS, GS).
    kRegCr        = Reg::kRegCustom + 1, //!< Control register (CR).
    kRegDr        = Reg::kRegCustom + 2, //!< Debug register (DR).
    kRegFp        = Reg::kRegCustom + 3, //!< FPU (x87) register.
    kRegBnd       = Reg::kRegCustom + 4, //!< Bound register (BND).
    kRegCount                            //!< Count of register types.
  };

  //! Register group.
  enum Group : uint32_t {
    kGroupGp      = Reg::kGroupGp,       //!< GP register group or none (universal).
    kGroupVec     = Reg::kGroupVec,      //!< XMM|YMM|ZMM register group (universal).
    kGroupMm      = Reg::kGroupOther0,   //!< MMX register group (legacy).
    kGroupK       = Reg::kGroupOther1,   //!< K register group.

    // These are not managed by CodeCompiler nor used by Func-API:
    kGroupRip     = Reg::kGroupVirt + 0, //!< Instrucion pointer (IP).
    kGroupSeg     = Reg::kGroupVirt + 1, //!< Segment register group.
    kGroupCr      = Reg::kGroupVirt + 2, //!< Control register group.
    kGroupDr      = Reg::kGroupVirt + 3, //!< Debug register group.
    kGroupFp      = Reg::kGroupVirt + 4, //!< FPU/X87 register group.
    kGroupBnd     = Reg::kGroupVirt + 5, //!< Bound register group.
    kGroupCount                          //!< Count of all register groups.
  };

  //! Get whether the register is a GPB register (8-bit).
  constexpr bool isGpb() const noexcept { return getSize() == 1; }
  //! Get whether the register is a low GPB register (8-bit).
  constexpr bool isGpbLo() const noexcept { return hasSignature(X86RegTraits<kRegGpbLo>::kSignature); }
  //! Get whether the register is a high GPB register (8-bit).
  constexpr bool isGpbHi() const noexcept { return hasSignature(X86RegTraits<kRegGpbHi>::kSignature); }
  //! Get whether the register is a GPW register (16-bit).
  constexpr bool isGpw() const noexcept { return hasSignature(X86RegTraits<kRegGpw>::kSignature); }
  //! Get whether the register is a GPD register (32-bit).
  constexpr bool isGpd() const noexcept { return hasSignature(X86RegTraits<kRegGpd>::kSignature); }
  //! Get whether the register is a GPQ register (64-bit).
  constexpr bool isGpq() const noexcept { return hasSignature(X86RegTraits<kRegGpq>::kSignature); }
  //! Get whether the register is an XMM register (128-bit).
  constexpr bool isXmm() const noexcept { return hasSignature(X86RegTraits<kRegXmm>::kSignature); }
  //! Get whether the register is a YMM register (256-bit).
  constexpr bool isYmm() const noexcept { return hasSignature(X86RegTraits<kRegYmm>::kSignature); }
  //! Get whether the register is a ZMM register (512-bit).
  constexpr bool isZmm() const noexcept { return hasSignature(X86RegTraits<kRegZmm>::kSignature); }
  //! Get whether the register is an MMX register (64-bit).
  constexpr bool isMm() const noexcept { return hasSignature(X86RegTraits<kRegMm>::kSignature); }
  //! Get whether the register is a K register (64-bit).
  constexpr bool isK() const noexcept { return hasSignature(X86RegTraits<kRegK>::kSignature); }
  //! Get whether the register is RIP.
  constexpr bool isRip() const noexcept { return hasSignature(X86RegTraits<kRegRip>::kSignature); }
  //! Get whether the register is a segment register.
  constexpr bool isSeg() const noexcept { return hasSignature(X86RegTraits<kRegSeg>::kSignature); }
  //! Get whether the register is a control register.
  constexpr bool isCr() const noexcept { return hasSignature(X86RegTraits<kRegCr>::kSignature); }
  //! Get whether the register is a debug register.
  constexpr bool isDr() const noexcept { return hasSignature(X86RegTraits<kRegDr>::kSignature); }
  //! Get whether the register is an FPU register (80-bit).
  constexpr bool isFp() const noexcept { return hasSignature(X86RegTraits<kRegFp>::kSignature); }
  //! Get whether the register is a bound register.
  constexpr bool isBnd() const noexcept { return hasSignature(X86RegTraits<kRegBnd>::kSignature); }

  template<uint32_t REG_TYPE>
  inline void setX86RegT(uint32_t rId) noexcept {
    setSignature(X86RegTraits<REG_TYPE>::kSignature);
    setId(rId);
  }

  inline void setTypeAndId(uint32_t rType, uint32_t rId) noexcept {
    ASMJIT_ASSERT(rType < kRegCount);
    setSignature(signatureOf(rType));
    setId(rId);
  }

  static inline uint32_t groupOf(uint32_t rType) noexcept;
  template<uint32_t REG_TYPE>
  static inline uint32_t groupOfT() noexcept { return X86RegTraits<REG_TYPE>::kGroup; }

  static inline uint32_t typeIdOf(uint32_t rType) noexcept;
  template<uint32_t REG_TYPE>
  static inline uint32_t typeIdOfT() noexcept { return X86RegTraits<REG_TYPE>::kTypeId; }

  static inline uint32_t signatureOf(uint32_t rType) noexcept;
  template<uint32_t REG_TYPE>
  static inline uint32_t signatureOfT() noexcept { return X86RegTraits<REG_TYPE>::kSignature; }

  static inline uint32_t signatureOfVecByType(uint32_t typeId) noexcept {
    return typeId <= Type::_kIdVec128End ? signatureOfT<kRegXmm>() :
           typeId <= Type::_kIdVec256End ? signatureOfT<kRegYmm>() : signatureOfT<kRegZmm>() ;
  }

  static inline uint32_t signatureOfVecBySize(uint32_t size) noexcept {
    return size <= 16 ? signatureOfT<kRegXmm>() :
           size <= 32 ? signatureOfT<kRegYmm>() : signatureOfT<kRegZmm>() ;
  }

  //! Get whether the `op` operand is either a low or high 8-bit GPB register.
  static inline bool isGpb(const Operand_& op) noexcept {
    // Check operand type, register group, and size. Not interested in register type.
    const uint32_t kSgn = (Operand::kOpReg << kSignatureOpShift  ) |
                          (1               << kSignatureSizeShift) ;
    return (op.getSignature() & (kSignatureOpMask | kSignatureSizeMask)) == kSgn;
  }

  static inline bool isRip(const Operand_& op) noexcept { return op.as<X86Reg>().isRip(); }
  static inline bool isSeg(const Operand_& op) noexcept { return op.as<X86Reg>().isSeg(); }
  static inline bool isGpbLo(const Operand_& op) noexcept { return op.as<X86Reg>().isGpbLo(); }
  static inline bool isGpbHi(const Operand_& op) noexcept { return op.as<X86Reg>().isGpbHi(); }
  static inline bool isGpw(const Operand_& op) noexcept { return op.as<X86Reg>().isGpw(); }
  static inline bool isGpd(const Operand_& op) noexcept { return op.as<X86Reg>().isGpd(); }
  static inline bool isGpq(const Operand_& op) noexcept { return op.as<X86Reg>().isGpq(); }
  static inline bool isFp(const Operand_& op) noexcept { return op.as<X86Reg>().isFp(); }
  static inline bool isMm(const Operand_& op) noexcept { return op.as<X86Reg>().isMm(); }
  static inline bool isK(const Operand_& op) noexcept { return op.as<X86Reg>().isK(); }
  static inline bool isXmm(const Operand_& op) noexcept { return op.as<X86Reg>().isXmm(); }
  static inline bool isYmm(const Operand_& op) noexcept { return op.as<X86Reg>().isYmm(); }
  static inline bool isZmm(const Operand_& op) noexcept { return op.as<X86Reg>().isZmm(); }
  static inline bool isBnd(const Operand_& op) noexcept { return op.as<X86Reg>().isBnd(); }
  static inline bool isCr(const Operand_& op) noexcept { return op.as<X86Reg>().isCr(); }
  static inline bool isDr(const Operand_& op) noexcept { return op.as<X86Reg>().isDr(); }

  static inline bool isRip(const Operand_& op, uint32_t rId) noexcept { return isRip(op) & (op.getId() == rId); }
  static inline bool isSeg(const Operand_& op, uint32_t rId) noexcept { return isSeg(op) & (op.getId() == rId); }
  static inline bool isGpb(const Operand_& op, uint32_t rId) noexcept { return isGpb(op) & (op.getId() == rId); }
  static inline bool isGpbLo(const Operand_& op, uint32_t rId) noexcept { return isGpbLo(op) & (op.getId() == rId); }
  static inline bool isGpbHi(const Operand_& op, uint32_t rId) noexcept { return isGpbHi(op) & (op.getId() == rId); }
  static inline bool isGpw(const Operand_& op, uint32_t rId) noexcept { return isGpw(op) & (op.getId() == rId); }
  static inline bool isGpd(const Operand_& op, uint32_t rId) noexcept { return isGpd(op) & (op.getId() == rId); }
  static inline bool isGpq(const Operand_& op, uint32_t rId) noexcept { return isGpq(op) & (op.getId() == rId); }
  static inline bool isFp(const Operand_& op, uint32_t rId) noexcept { return isFp(op) & (op.getId() == rId); }
  static inline bool isMm(const Operand_& op, uint32_t rId) noexcept { return isMm(op) & (op.getId() == rId); }
  static inline bool isK(const Operand_& op, uint32_t rId) noexcept { return isK(op) & (op.getId() == rId); }
  static inline bool isXmm(const Operand_& op, uint32_t rId) noexcept { return isXmm(op) & (op.getId() == rId); }
  static inline bool isYmm(const Operand_& op, uint32_t rId) noexcept { return isYmm(op) & (op.getId() == rId); }
  static inline bool isZmm(const Operand_& op, uint32_t rId) noexcept { return isZmm(op) & (op.getId() == rId); }
  static inline bool isBnd(const Operand_& op, uint32_t rId) noexcept { return isBnd(op) & (op.getId() == rId); }
  static inline bool isCr(const Operand_& op, uint32_t rId) noexcept { return isCr(op) & (op.getId() == rId); }
  static inline bool isDr(const Operand_& op, uint32_t rId) noexcept { return isDr(op) & (op.getId() == rId); }
};

//! General purpose register (X86/X64).
class X86Gp : public X86Reg {
public:
  ASMJIT_DEFINE_ABSTRACT_REG(X86Gp, X86Reg)

  //! X86/X64 physical id.
  //!
  //! NOTE: Register indexes have been reduced to only support general purpose
  //! registers. There is no need to have enumerations with number suffix that
  //! expands to the exactly same value as the suffix value itself.
  enum Id : uint32_t {
    kIdAx  = 0,  //!< Physical id of AL|AH|AX|EAX|RAX registers.
    kIdCx  = 1,  //!< Physical id of CL|CH|CX|ECX|RCX registers.
    kIdDx  = 2,  //!< Physical id of DL|DH|DX|EDX|RDX registers.
    kIdBx  = 3,  //!< Physical id of BL|BH|BX|EBX|RBX registers.
    kIdSp  = 4,  //!< Physical id of SPL|SP|ESP|RSP registers.
    kIdBp  = 5,  //!< Physical id of BPL|BP|EBP|RBP registers.
    kIdSi  = 6,  //!< Physical id of SIL|SI|ESI|RSI registers.
    kIdDi  = 7,  //!< Physical id of DIL|DI|EDI|RDI registers.
    kIdR8  = 8,  //!< Physical id of R8B|R8W|R8D|R8 registers (64-bit only).
    kIdR9  = 9,  //!< Physical id of R9B|R9W|R9D|R9 registers (64-bit only).
    kIdR10 = 10, //!< Physical id of R10B|R10W|R10D|R10 registers (64-bit only).
    kIdR11 = 11, //!< Physical id of R11B|R11W|R11D|R11 registers (64-bit only).
    kIdR12 = 12, //!< Physical id of R12B|R12W|R12D|R12 registers (64-bit only).
    kIdR13 = 13, //!< Physical id of R13B|R13W|R13D|R13 registers (64-bit only).
    kIdR14 = 14, //!< Physical id of R14B|R14W|R14D|R14 registers (64-bit only).
    kIdR15 = 15  //!< Physical id of R15B|R15W|R15D|R15 registers (64-bit only).
  };

  //! Cast this register to 8-bit (LO) part.
  constexpr X86GpbLo r8() const noexcept;
  //! Cast this register to 8-bit (LO) part.
  constexpr X86GpbLo r8Lo() const noexcept;
  //! Cast this register to 8-bit (HI) part.
  constexpr X86GpbHi r8Hi() const noexcept;
  //! Cast this register to 16-bit.
  constexpr X86Gpw r16() const noexcept;
  //! Cast this register to 32-bit.
  constexpr X86Gpd r32() const noexcept;
  //! Cast this register to 64-bit.
  constexpr X86Gpq r64() const noexcept;
};

//! XMM|YMM|ZMM register (X86/X64).
class X86Vec : public X86Reg {
  ASMJIT_DEFINE_ABSTRACT_REG(X86Vec, X86Reg)

  //! Cast this register to XMM (clone).
  constexpr X86Xmm xmm() const noexcept;
  //! Cast this register to YMM.
  constexpr X86Ymm ymm() const noexcept;
  //! Cast this register to ZMM.
  constexpr X86Zmm zmm() const noexcept;
};

//! Segment register (X86/X64).
class X86Seg : public X86Reg {
  ASMJIT_DEFINE_FINAL_REG(X86Seg, X86Reg, X86RegTraits<kRegSeg>)

  //! X86/X64 segment id.
  enum Id : uint32_t {
    kIdNone = 0, //!< No segment (default).
    kIdEs   = 1, //!< ES segment.
    kIdCs   = 2, //!< CS segment.
    kIdSs   = 3, //!< SS segment.
    kIdDs   = 4, //!< DS segment.
    kIdFs   = 5, //!< FS segment.
    kIdGs   = 6, //!< GS segment.

    //! Count of X86 segment registers supported by AsmJit.
    //!
    //! NOTE: X86 architecture has 6 segment registers - ES, CS, SS, DS, FS, GS.
    //! X64 architecture lowers them down to just FS and GS. AsmJit supports 7
    //! segment registers - all addressable in both X86 and X64 modes and one
    //! extra called `X86Seg::kIdNone`, which is AsmJit specific and means that
    //! there is no segment register specified.
    kIdCount = 7
  };
};

//! GPB (low or high) register (X86/X64).
class X86Gpb : public X86Gp { ASMJIT_DEFINE_ABSTRACT_REG(X86Gpb, X86Gp) };
//! GPB low register (X86/X64).
class X86GpbLo : public X86Gpb { ASMJIT_DEFINE_FINAL_REG(X86GpbLo, X86Gpb, X86RegTraits<kRegGpbLo>) };
//! GPB high register (X86/X64).
class X86GpbHi : public X86Gpb { ASMJIT_DEFINE_FINAL_REG(X86GpbHi, X86Gpb, X86RegTraits<kRegGpbHi>) };
//! GPW register (X86/X64).
class X86Gpw : public X86Gp { ASMJIT_DEFINE_FINAL_REG(X86Gpw, X86Gp, X86RegTraits<kRegGpw>) };
//! GPD register (X86/X64).
class X86Gpd : public X86Gp { ASMJIT_DEFINE_FINAL_REG(X86Gpd, X86Gp, X86RegTraits<kRegGpd>) };
//! GPQ register (X64).
class X86Gpq : public X86Gp { ASMJIT_DEFINE_FINAL_REG(X86Gpq, X86Gp, X86RegTraits<kRegGpq>) };
//! RIP register (X86/X64).
class X86Rip : public X86Reg { ASMJIT_DEFINE_FINAL_REG(X86Rip, X86Reg, X86RegTraits<kRegRip>) };
//! 80-bit FPU register (X86/X64).
class X86Fp : public X86Reg { ASMJIT_DEFINE_FINAL_REG(X86Fp, X86Reg, X86RegTraits<kRegFp>) };
//! 64-bit MMX register (MMX+).
class X86Mm : public X86Reg { ASMJIT_DEFINE_FINAL_REG(X86Mm, X86Reg, X86RegTraits<kRegMm>) };
//! 64-bit K register (AVX512+).
class X86KReg : public X86Reg { ASMJIT_DEFINE_FINAL_REG(X86KReg, X86Reg, X86RegTraits<kRegK>) };
//! 128-bit XMM register (SSE+).
class X86Xmm : public X86Vec { ASMJIT_DEFINE_FINAL_REG(X86Xmm, X86Vec, X86RegTraits<kRegXmm>) };
//! 256-bit YMM register (AVX+).
class X86Ymm : public X86Vec { ASMJIT_DEFINE_FINAL_REG(X86Ymm, X86Vec, X86RegTraits<kRegYmm>) };
//! 512-bit ZMM register (AVX512+).
class X86Zmm : public X86Vec { ASMJIT_DEFINE_FINAL_REG(X86Zmm, X86Vec, X86RegTraits<kRegZmm>) };
//! 128-bit BND register (BND+).
class X86Bnd : public X86Reg { ASMJIT_DEFINE_FINAL_REG(X86Bnd, X86Reg, X86RegTraits<kRegBnd>) };
//! 32-bit or 64-bit control register (X86/X64).
class X86CReg : public X86Reg { ASMJIT_DEFINE_FINAL_REG(X86CReg, X86Reg, X86RegTraits<kRegCr>) };
//! 32-bit or 64-bit debug register (X86/X64).
class X86DReg : public X86Reg { ASMJIT_DEFINE_FINAL_REG(X86DReg, X86Reg, X86RegTraits<kRegDr>) };

constexpr X86GpbLo X86Gp::r8() const noexcept { return X86GpbLo(getId()); }
constexpr X86GpbLo X86Gp::r8Lo() const noexcept { return X86GpbLo(getId()); }
constexpr X86GpbHi X86Gp::r8Hi() const noexcept { return X86GpbHi(getId()); }
constexpr X86Gpw X86Gp::r16() const noexcept { return X86Gpw(getId()); }
constexpr X86Gpd X86Gp::r32() const noexcept { return X86Gpd(getId()); }
constexpr X86Gpq X86Gp::r64() const noexcept { return X86Gpq(getId()); }
constexpr X86Xmm X86Vec::xmm() const noexcept { return X86Xmm(*this, getId()); }
constexpr X86Ymm X86Vec::ymm() const noexcept { return X86Ymm(*this, getId()); }
constexpr X86Zmm X86Vec::zmm() const noexcept { return X86Zmm(*this, getId()); }

ASMJIT_DEFINE_TYPE_ID(X86Gpb, kIdI8);
ASMJIT_DEFINE_TYPE_ID(X86Gpw, kIdI16);
ASMJIT_DEFINE_TYPE_ID(X86Gpd, kIdI32);
ASMJIT_DEFINE_TYPE_ID(X86Gpq, kIdI64);
ASMJIT_DEFINE_TYPE_ID(X86Mm , kIdMmx64);
ASMJIT_DEFINE_TYPE_ID(X86Xmm, kIdI32x4);
ASMJIT_DEFINE_TYPE_ID(X86Ymm, kIdI32x8);
ASMJIT_DEFINE_TYPE_ID(X86Zmm, kIdI32x16);

// ============================================================================
// [asmjit::X86Mem]
// ============================================================================

//! Memory operand (X86).
class X86Mem : public Mem {
public:
  //! Additional bits of operand's signature used by `X86Mem`.
  enum AdditionalBits : uint32_t {
    kSignatureMemShiftShift   = 19,
    kSignatureMemShiftBits    = 0x03U,
    kSignatureMemShiftMask    = kSignatureMemShiftBits << kSignatureMemShiftShift,

    kSignatureMemSegmentShift = 21,
    kSignatureMemSegmentBits  = 0x07U,
    kSignatureMemSegmentMask  = kSignatureMemSegmentBits << kSignatureMemSegmentShift
  };

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Construct a default `X86Mem` operand that points to [0].
  constexpr X86Mem() noexcept
    : Mem() {}

  constexpr X86Mem(const X86Mem& other) noexcept
    : Mem(other) {}

  constexpr X86Mem(const Label& base, int32_t off, uint32_t size = 0, uint32_t flags = 0) noexcept
    : Mem(Globals::Init, Label::kLabelTag, base.getId(), 0, 0, off, size, flags) {}

  constexpr X86Mem(const Label& base, const Reg& index, uint32_t shift, int32_t off, uint32_t size = 0, uint32_t flags = 0) noexcept
    : Mem(Globals::Init, Label::kLabelTag, base.getId(), index.getType(), index.getId(), off, size, flags | (shift << kSignatureMemShiftShift)) {}

  constexpr X86Mem(const Reg& base, int32_t off, uint32_t size = 0, uint32_t flags = 0) noexcept
    : Mem(Globals::Init, base.getType(), base.getId(), 0, 0, off, size, flags) {}

  constexpr X86Mem(const Reg& base, const Reg& index, uint32_t shift, int32_t off, uint32_t size = 0, uint32_t flags = 0) noexcept
    : Mem(Globals::Init, base.getType(), base.getId(), index.getType(), index.getId(), off, size, flags | (shift << kSignatureMemShiftShift)) {}

  constexpr X86Mem(uint64_t base, uint32_t size = 0, uint32_t flags = 0) noexcept
    : Mem(Globals::Init, 0, uint32_t(base >> 32), 0, 0, int32_t(uint32_t(base & 0xFFFFFFFFU)), size, flags) {}

  constexpr X86Mem(uint64_t base, const Reg& index, uint32_t shift = 0, uint32_t size = 0, uint32_t flags = 0) noexcept
    : Mem(Globals::Init, 0, uint32_t(base >> 32), index.getType(), index.getId(), int32_t(uint32_t(base & 0xFFFFFFFFU)), size, flags | (shift << kSignatureMemShiftShift)) {}

  constexpr X86Mem(Globals::Init_, uint32_t baseType, uint32_t baseId, uint32_t indexType, uint32_t indexId, int32_t off, uint32_t size, uint32_t flags) noexcept
    : Mem(Globals::Init, baseType, baseId, indexType, indexId, off, size, flags) {}

  explicit inline X86Mem(Globals::NoInit_) noexcept
    : Mem(Globals::NoInit) {}

  // --------------------------------------------------------------------------
  // [X86Mem]
  // --------------------------------------------------------------------------

  //! Clone the memory operand.
  constexpr X86Mem clone() const noexcept { return X86Mem(*this); }
  //! Get new memory operand adjusted by `off`.
  inline X86Mem cloneAdjusted(int64_t off) const noexcept {
    X86Mem result(*this);
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

  //! Get whether the memory operand has a segment override.
  constexpr bool hasSegment() const noexcept { return _hasSignatureData(kSignatureMemSegmentMask); }
  //! Get associated segment override as `X86Seg` operand.
  constexpr X86Seg getSegment() const noexcept { return X86Seg(getSegmentId()); }
  //! Get segment override as id, see \ref X86Seg::Id.
  constexpr uint32_t getSegmentId() const noexcept { return _getSignatureData(kSignatureMemSegmentBits, kSignatureMemSegmentShift); }

  //! Set the segment override to `seg`.
  inline void setSegment(const X86Seg& seg) noexcept { setSegment(seg.getId()); }
  //! Set the segment override to `id`.
  inline void setSegment(uint32_t rId) noexcept { _setSignatureData(rId, kSignatureMemSegmentBits, kSignatureMemSegmentShift); }
  //! Reset the segment override.
  inline void resetSegment() noexcept { _any.signature &= ~kSignatureMemSegmentMask; }

  // --------------------------------------------------------------------------
  // [Operator Overload]
  // --------------------------------------------------------------------------

  inline X86Mem& operator=(const X86Mem& other) noexcept = default;
};

// ============================================================================
// [asmjit::X86OpData]
// ============================================================================

struct X86OpData {
  //! Information about all architecture registers.
  ArchRegs archRegs;
};
ASMJIT_VARAPI const X86OpData x86OpData;

inline uint32_t X86Reg::groupOf(uint32_t rType) noexcept {
  ASMJIT_ASSERT(rType <= Reg::kRegMax);
  return x86OpData.archRegs.regInfo[rType].getGroup();
}

inline uint32_t X86Reg::typeIdOf(uint32_t rType) noexcept {
  ASMJIT_ASSERT(rType <= Reg::kRegMax);
  return x86OpData.archRegs.regTypeToTypeId[rType];
}

// ... X86Reg methods that require `x86OpData`.
inline uint32_t X86Reg::signatureOf(uint32_t rType) noexcept {
  ASMJIT_ASSERT(rType <= Reg::kRegMax);
  return x86OpData.archRegs.regInfo[rType].getSignature();
}

// ============================================================================
// [asmjit::x86::regs]
// ============================================================================

namespace x86 {
namespace regs {

namespace {

//! Create an 8-bit low GPB register operand.
static constexpr X86GpbLo gpb(uint32_t rId) noexcept { return X86GpbLo(rId); }
//! Create an 8-bit low GPB register operand.
static constexpr X86GpbLo gpb_lo(uint32_t rId) noexcept { return X86GpbLo(rId); }
//! Create an 8-bit high GPB register operand.
static constexpr X86GpbHi gpb_hi(uint32_t rId) noexcept { return X86GpbHi(rId); }
//! Create a 16-bit GPW register operand.
static constexpr X86Gpw gpw(uint32_t rId) noexcept { return X86Gpw(rId); }
//! Create a 32-bit GPD register operand.
static constexpr X86Gpd gpd(uint32_t rId) noexcept { return X86Gpd(rId); }
//! Create a 64-bit GPQ register operand (X64).
static constexpr X86Gpq gpq(uint32_t rId) noexcept { return X86Gpq(rId); }
//! Create an 80-bit Fp register operand.
static constexpr X86Fp fp(uint32_t rId) noexcept { return X86Fp(rId); }
//! Create a 64-bit Mm register operand.
static constexpr X86Mm mm(uint32_t rId) noexcept { return X86Mm(rId); }
//! Create a 64-bit K register operand.
static constexpr X86KReg k(uint32_t rId) noexcept { return X86KReg(rId); }
//! Create a 128-bit XMM register operand.
static constexpr X86Xmm xmm(uint32_t rId) noexcept { return X86Xmm(rId); }
//! Create a 256-bit YMM register operand.
static constexpr X86Ymm ymm(uint32_t rId) noexcept { return X86Ymm(rId); }
//! Create a 512-bit ZMM register operand.
static constexpr X86Zmm zmm(uint32_t rId) noexcept { return X86Zmm(rId); }
//! Create a 128-bit bound register operand.
static constexpr X86Bnd bnd(uint32_t rId) noexcept { return X86Bnd(rId); }
//! Create a 32-bit or 64-bit control register operand.
static constexpr X86CReg cr(uint32_t rId) noexcept { return X86CReg(rId); }
//! Create a 32-bit or 64-bit debug register operand.
static constexpr X86DReg dr(uint32_t rId) noexcept { return X86DReg(rId); }

static constexpr X86Gp al(X86GpbLo::kSignature, X86Gp::kIdAx);
static constexpr X86Gp bl(X86GpbLo::kSignature, X86Gp::kIdBx);
static constexpr X86Gp cl(X86GpbLo::kSignature, X86Gp::kIdCx);
static constexpr X86Gp dl(X86GpbLo::kSignature, X86Gp::kIdDx);
static constexpr X86Gp spl(X86GpbLo::kSignature, X86Gp::kIdSp);
static constexpr X86Gp bpl(X86GpbLo::kSignature, X86Gp::kIdBp);
static constexpr X86Gp sil(X86GpbLo::kSignature, X86Gp::kIdSi);
static constexpr X86Gp dil(X86GpbLo::kSignature, X86Gp::kIdDi);
static constexpr X86Gp r8b(X86GpbLo::kSignature, X86Gp::kIdR8);
static constexpr X86Gp r9b(X86GpbLo::kSignature, X86Gp::kIdR9);
static constexpr X86Gp r10b(X86GpbLo::kSignature, X86Gp::kIdR10);
static constexpr X86Gp r11b(X86GpbLo::kSignature, X86Gp::kIdR11);
static constexpr X86Gp r12b(X86GpbLo::kSignature, X86Gp::kIdR12);
static constexpr X86Gp r13b(X86GpbLo::kSignature, X86Gp::kIdR13);
static constexpr X86Gp r14b(X86GpbLo::kSignature, X86Gp::kIdR14);
static constexpr X86Gp r15b(X86GpbLo::kSignature, X86Gp::kIdR15);

static constexpr X86Gp ah(X86GpbHi::kSignature, X86Gp::kIdAx);
static constexpr X86Gp bh(X86GpbHi::kSignature, X86Gp::kIdBx);
static constexpr X86Gp ch(X86GpbHi::kSignature, X86Gp::kIdCx);
static constexpr X86Gp dh(X86GpbHi::kSignature, X86Gp::kIdDx);

static constexpr X86Gp ax(X86Gpw::kSignature, X86Gp::kIdAx);
static constexpr X86Gp bx(X86Gpw::kSignature, X86Gp::kIdBx);
static constexpr X86Gp cx(X86Gpw::kSignature, X86Gp::kIdCx);
static constexpr X86Gp dx(X86Gpw::kSignature, X86Gp::kIdDx);
static constexpr X86Gp sp(X86Gpw::kSignature, X86Gp::kIdSp);
static constexpr X86Gp bp(X86Gpw::kSignature, X86Gp::kIdBp);
static constexpr X86Gp si(X86Gpw::kSignature, X86Gp::kIdSi);
static constexpr X86Gp di(X86Gpw::kSignature, X86Gp::kIdDi);
static constexpr X86Gp r8w(X86Gpw::kSignature, X86Gp::kIdR8);
static constexpr X86Gp r9w(X86Gpw::kSignature, X86Gp::kIdR9);
static constexpr X86Gp r10w(X86Gpw::kSignature, X86Gp::kIdR10);
static constexpr X86Gp r11w(X86Gpw::kSignature, X86Gp::kIdR11);
static constexpr X86Gp r12w(X86Gpw::kSignature, X86Gp::kIdR12);
static constexpr X86Gp r13w(X86Gpw::kSignature, X86Gp::kIdR13);
static constexpr X86Gp r14w(X86Gpw::kSignature, X86Gp::kIdR14);
static constexpr X86Gp r15w(X86Gpw::kSignature, X86Gp::kIdR15);

static constexpr X86Gp eax(X86Gpd::kSignature, X86Gp::kIdAx);
static constexpr X86Gp ebx(X86Gpd::kSignature, X86Gp::kIdBx);
static constexpr X86Gp ecx(X86Gpd::kSignature, X86Gp::kIdCx);
static constexpr X86Gp edx(X86Gpd::kSignature, X86Gp::kIdDx);
static constexpr X86Gp esp(X86Gpd::kSignature, X86Gp::kIdSp);
static constexpr X86Gp ebp(X86Gpd::kSignature, X86Gp::kIdBp);
static constexpr X86Gp esi(X86Gpd::kSignature, X86Gp::kIdSi);
static constexpr X86Gp edi(X86Gpd::kSignature, X86Gp::kIdDi);
static constexpr X86Gp r8d(X86Gpd::kSignature, X86Gp::kIdR8);
static constexpr X86Gp r9d(X86Gpd::kSignature, X86Gp::kIdR9);
static constexpr X86Gp r10d(X86Gpd::kSignature, X86Gp::kIdR10);
static constexpr X86Gp r11d(X86Gpd::kSignature, X86Gp::kIdR11);
static constexpr X86Gp r12d(X86Gpd::kSignature, X86Gp::kIdR12);
static constexpr X86Gp r13d(X86Gpd::kSignature, X86Gp::kIdR13);
static constexpr X86Gp r14d(X86Gpd::kSignature, X86Gp::kIdR14);
static constexpr X86Gp r15d(X86Gpd::kSignature, X86Gp::kIdR15);

static constexpr X86Gp rax(X86Gpq::kSignature, X86Gp::kIdAx);
static constexpr X86Gp rbx(X86Gpq::kSignature, X86Gp::kIdBx);
static constexpr X86Gp rcx(X86Gpq::kSignature, X86Gp::kIdCx);
static constexpr X86Gp rdx(X86Gpq::kSignature, X86Gp::kIdDx);
static constexpr X86Gp rsp(X86Gpq::kSignature, X86Gp::kIdSp);
static constexpr X86Gp rbp(X86Gpq::kSignature, X86Gp::kIdBp);
static constexpr X86Gp rsi(X86Gpq::kSignature, X86Gp::kIdSi);
static constexpr X86Gp rdi(X86Gpq::kSignature, X86Gp::kIdDi);
static constexpr X86Gp r8(X86Gpq::kSignature, X86Gp::kIdR8);
static constexpr X86Gp r9(X86Gpq::kSignature, X86Gp::kIdR9);
static constexpr X86Gp r10(X86Gpq::kSignature, X86Gp::kIdR10);
static constexpr X86Gp r11(X86Gpq::kSignature, X86Gp::kIdR11);
static constexpr X86Gp r12(X86Gpq::kSignature, X86Gp::kIdR12);
static constexpr X86Gp r13(X86Gpq::kSignature, X86Gp::kIdR13);
static constexpr X86Gp r14(X86Gpq::kSignature, X86Gp::kIdR14);
static constexpr X86Gp r15(X86Gpq::kSignature, X86Gp::kIdR15);

static constexpr X86Xmm xmm0(X86Xmm::kSignature, 0);
static constexpr X86Xmm xmm1(X86Xmm::kSignature, 1);
static constexpr X86Xmm xmm2(X86Xmm::kSignature, 2);
static constexpr X86Xmm xmm3(X86Xmm::kSignature, 3);
static constexpr X86Xmm xmm4(X86Xmm::kSignature, 4);
static constexpr X86Xmm xmm5(X86Xmm::kSignature, 5);
static constexpr X86Xmm xmm6(X86Xmm::kSignature, 6);
static constexpr X86Xmm xmm7(X86Xmm::kSignature, 7);
static constexpr X86Xmm xmm8(X86Xmm::kSignature, 8);
static constexpr X86Xmm xmm9(X86Xmm::kSignature, 9);
static constexpr X86Xmm xmm10(X86Xmm::kSignature, 10);
static constexpr X86Xmm xmm11(X86Xmm::kSignature, 11);
static constexpr X86Xmm xmm12(X86Xmm::kSignature, 12);
static constexpr X86Xmm xmm13(X86Xmm::kSignature, 13);
static constexpr X86Xmm xmm14(X86Xmm::kSignature, 14);
static constexpr X86Xmm xmm15(X86Xmm::kSignature, 15);
static constexpr X86Xmm xmm16(X86Xmm::kSignature, 16);
static constexpr X86Xmm xmm17(X86Xmm::kSignature, 17);
static constexpr X86Xmm xmm18(X86Xmm::kSignature, 18);
static constexpr X86Xmm xmm19(X86Xmm::kSignature, 19);
static constexpr X86Xmm xmm20(X86Xmm::kSignature, 20);
static constexpr X86Xmm xmm21(X86Xmm::kSignature, 21);
static constexpr X86Xmm xmm22(X86Xmm::kSignature, 22);
static constexpr X86Xmm xmm23(X86Xmm::kSignature, 23);
static constexpr X86Xmm xmm24(X86Xmm::kSignature, 24);
static constexpr X86Xmm xmm25(X86Xmm::kSignature, 25);
static constexpr X86Xmm xmm26(X86Xmm::kSignature, 26);
static constexpr X86Xmm xmm27(X86Xmm::kSignature, 27);
static constexpr X86Xmm xmm28(X86Xmm::kSignature, 28);
static constexpr X86Xmm xmm29(X86Xmm::kSignature, 29);
static constexpr X86Xmm xmm30(X86Xmm::kSignature, 30);
static constexpr X86Xmm xmm31(X86Xmm::kSignature, 31);

static constexpr X86Ymm ymm0(X86Ymm::kSignature, 0);
static constexpr X86Ymm ymm1(X86Ymm::kSignature, 1);
static constexpr X86Ymm ymm2(X86Ymm::kSignature, 2);
static constexpr X86Ymm ymm3(X86Ymm::kSignature, 3);
static constexpr X86Ymm ymm4(X86Ymm::kSignature, 4);
static constexpr X86Ymm ymm5(X86Ymm::kSignature, 5);
static constexpr X86Ymm ymm6(X86Ymm::kSignature, 6);
static constexpr X86Ymm ymm7(X86Ymm::kSignature, 7);
static constexpr X86Ymm ymm8(X86Ymm::kSignature, 8);
static constexpr X86Ymm ymm9(X86Ymm::kSignature, 9);
static constexpr X86Ymm ymm10(X86Ymm::kSignature, 10);
static constexpr X86Ymm ymm11(X86Ymm::kSignature, 11);
static constexpr X86Ymm ymm12(X86Ymm::kSignature, 12);
static constexpr X86Ymm ymm13(X86Ymm::kSignature, 13);
static constexpr X86Ymm ymm14(X86Ymm::kSignature, 14);
static constexpr X86Ymm ymm15(X86Ymm::kSignature, 15);
static constexpr X86Ymm ymm16(X86Ymm::kSignature, 16);
static constexpr X86Ymm ymm17(X86Ymm::kSignature, 17);
static constexpr X86Ymm ymm18(X86Ymm::kSignature, 18);
static constexpr X86Ymm ymm19(X86Ymm::kSignature, 19);
static constexpr X86Ymm ymm20(X86Ymm::kSignature, 20);
static constexpr X86Ymm ymm21(X86Ymm::kSignature, 21);
static constexpr X86Ymm ymm22(X86Ymm::kSignature, 22);
static constexpr X86Ymm ymm23(X86Ymm::kSignature, 23);
static constexpr X86Ymm ymm24(X86Ymm::kSignature, 24);
static constexpr X86Ymm ymm25(X86Ymm::kSignature, 25);
static constexpr X86Ymm ymm26(X86Ymm::kSignature, 26);
static constexpr X86Ymm ymm27(X86Ymm::kSignature, 27);
static constexpr X86Ymm ymm28(X86Ymm::kSignature, 28);
static constexpr X86Ymm ymm29(X86Ymm::kSignature, 29);
static constexpr X86Ymm ymm30(X86Ymm::kSignature, 30);
static constexpr X86Ymm ymm31(X86Ymm::kSignature, 31);

static constexpr X86Zmm zmm0(X86Zmm::kSignature, 0);
static constexpr X86Zmm zmm1(X86Zmm::kSignature, 1);
static constexpr X86Zmm zmm2(X86Zmm::kSignature, 2);
static constexpr X86Zmm zmm3(X86Zmm::kSignature, 3);
static constexpr X86Zmm zmm4(X86Zmm::kSignature, 4);
static constexpr X86Zmm zmm5(X86Zmm::kSignature, 5);
static constexpr X86Zmm zmm6(X86Zmm::kSignature, 6);
static constexpr X86Zmm zmm7(X86Zmm::kSignature, 7);
static constexpr X86Zmm zmm8(X86Zmm::kSignature, 8);
static constexpr X86Zmm zmm9(X86Zmm::kSignature, 9);
static constexpr X86Zmm zmm10(X86Zmm::kSignature, 10);
static constexpr X86Zmm zmm11(X86Zmm::kSignature, 11);
static constexpr X86Zmm zmm12(X86Zmm::kSignature, 12);
static constexpr X86Zmm zmm13(X86Zmm::kSignature, 13);
static constexpr X86Zmm zmm14(X86Zmm::kSignature, 14);
static constexpr X86Zmm zmm15(X86Zmm::kSignature, 15);
static constexpr X86Zmm zmm16(X86Zmm::kSignature, 16);
static constexpr X86Zmm zmm17(X86Zmm::kSignature, 17);
static constexpr X86Zmm zmm18(X86Zmm::kSignature, 18);
static constexpr X86Zmm zmm19(X86Zmm::kSignature, 19);
static constexpr X86Zmm zmm20(X86Zmm::kSignature, 20);
static constexpr X86Zmm zmm21(X86Zmm::kSignature, 21);
static constexpr X86Zmm zmm22(X86Zmm::kSignature, 22);
static constexpr X86Zmm zmm23(X86Zmm::kSignature, 23);
static constexpr X86Zmm zmm24(X86Zmm::kSignature, 24);
static constexpr X86Zmm zmm25(X86Zmm::kSignature, 25);
static constexpr X86Zmm zmm26(X86Zmm::kSignature, 26);
static constexpr X86Zmm zmm27(X86Zmm::kSignature, 27);
static constexpr X86Zmm zmm28(X86Zmm::kSignature, 28);
static constexpr X86Zmm zmm29(X86Zmm::kSignature, 29);
static constexpr X86Zmm zmm30(X86Zmm::kSignature, 30);
static constexpr X86Zmm zmm31(X86Zmm::kSignature, 31);

static constexpr X86Mm mm0(0);
static constexpr X86Mm mm1(1);
static constexpr X86Mm mm2(2);
static constexpr X86Mm mm3(3);
static constexpr X86Mm mm4(4);
static constexpr X86Mm mm5(5);
static constexpr X86Mm mm6(6);
static constexpr X86Mm mm7(7);

static constexpr X86KReg k0(0);
static constexpr X86KReg k1(1);
static constexpr X86KReg k2(2);
static constexpr X86KReg k3(3);
static constexpr X86KReg k4(4);
static constexpr X86KReg k5(5);
static constexpr X86KReg k6(6);
static constexpr X86KReg k7(7);

static constexpr X86Rip rip(0);

static constexpr X86Seg no_seg(X86Seg::kIdNone);
static constexpr X86Seg es(X86Seg::kIdEs);
static constexpr X86Seg cs(X86Seg::kIdCs);
static constexpr X86Seg ss(X86Seg::kIdSs);
static constexpr X86Seg ds(X86Seg::kIdDs);
static constexpr X86Seg fs(X86Seg::kIdFs);
static constexpr X86Seg gs(X86Seg::kIdGs);

static constexpr X86CReg cr0(0);
static constexpr X86CReg cr1(1);
static constexpr X86CReg cr2(2);
static constexpr X86CReg cr3(3);
static constexpr X86CReg cr4(4);
static constexpr X86CReg cr5(5);
static constexpr X86CReg cr6(6);
static constexpr X86CReg cr7(7);
static constexpr X86CReg cr8(8);
static constexpr X86CReg cr9(9);
static constexpr X86CReg cr10(10);
static constexpr X86CReg cr11(11);
static constexpr X86CReg cr12(12);
static constexpr X86CReg cr13(13);
static constexpr X86CReg cr14(14);
static constexpr X86CReg cr15(15);

static constexpr X86DReg dr0(0);
static constexpr X86DReg dr1(1);
static constexpr X86DReg dr2(2);
static constexpr X86DReg dr3(3);
static constexpr X86DReg dr4(4);
static constexpr X86DReg dr5(5);
static constexpr X86DReg dr6(6);
static constexpr X86DReg dr7(7);
static constexpr X86DReg dr8(8);
static constexpr X86DReg dr9(9);
static constexpr X86DReg dr10(10);
static constexpr X86DReg dr11(11);
static constexpr X86DReg dr12(12);
static constexpr X86DReg dr13(13);
static constexpr X86DReg dr14(14);
static constexpr X86DReg dr15(15);

static constexpr X86Fp fp0(0);
static constexpr X86Fp fp1(1);
static constexpr X86Fp fp2(2);
static constexpr X86Fp fp3(3);
static constexpr X86Fp fp4(4);
static constexpr X86Fp fp5(5);
static constexpr X86Fp fp6(6);
static constexpr X86Fp fp7(7);

static constexpr X86Bnd bnd0(0);
static constexpr X86Bnd bnd1(1);
static constexpr X86Bnd bnd2(2);
static constexpr X86Bnd bnd3(3);

} // anonymous namespace
} // regs namespace

// Make `x86::regs` accessible through `x86` namespace as well.
using namespace regs;

} // x86 namespace

// ============================================================================
// [asmjit::x86::ptr]
// ============================================================================

namespace x86 {
namespace {

//! Create a `[base.reg + offset]` memory operand.
static constexpr X86Mem ptr(const X86Gp& base, int32_t offset = 0, uint32_t size = 0) noexcept {
  return X86Mem(base, offset, size);
}
//! Create a `[base.reg + (index << shift) + offset]` memory operand (scalar index).
static constexpr X86Mem ptr(const X86Gp& base, const X86Gp& index, uint32_t shift = 0, int32_t offset = 0, uint32_t size = 0) noexcept {
  return X86Mem(base, index, shift, offset, size);
}
//! Create a `[base.reg + (index << shift) + offset]` memory operand (vector index).
static constexpr X86Mem ptr(const X86Gp& base, const X86Vec& index, uint32_t shift = 0, int32_t offset = 0, uint32_t size = 0) noexcept {
  return X86Mem(base, index, shift, offset, size);
}

//! Create a `[base + offset]` memory operand.
static constexpr X86Mem ptr(const Label& base, int32_t offset = 0, uint32_t size = 0) noexcept {
  return X86Mem(base, offset, size);
}
//! Create a `[base + (index << shift) + offset]` memory operand.
static constexpr X86Mem ptr(const Label& base, const X86Gp& index, uint32_t shift, int32_t offset = 0, uint32_t size = 0) noexcept {
  return X86Mem(base, index, shift, offset, size);
}
//! Create a `[base + (index << shift) + offset]` memory operand.
static constexpr X86Mem ptr(const Label& base, const X86Vec& index, uint32_t shift, int32_t offset = 0, uint32_t size = 0) noexcept {
  return X86Mem(base, index, shift, offset, size);
}

//! Create `[rip + offset]` memory operand.
static constexpr X86Mem ptr(const X86Rip& rip_, int32_t offset = 0, uint32_t size = 0) noexcept {
  return X86Mem(rip_, offset, size);
}

//! Create an `[base]` absolute memory operand.
static constexpr X86Mem ptr(uint64_t base, uint32_t size = 0) noexcept {
  return X86Mem(base, size);
}
//! Create an `[abs + (index.reg << shift)]` absolute memory operand.
static constexpr X86Mem ptr(uint64_t base, const X86Reg& index, uint32_t shift = 0, uint32_t size = 0) noexcept {
  return X86Mem(base, index, shift, size);
}
//! Create an `[abs + (index.reg << shift)]` absolute memory operand.
static constexpr X86Mem ptr(uint64_t base, const X86Vec& index, uint32_t shift = 0, uint32_t size = 0) noexcept {
  return X86Mem(base, index, shift, size);
}

//! \internal
#define ASMJIT_X86_PTR_FN(FUNC, SIZE)                                                 \
  /*! Create a `[base + offset]` memory operand. */                                   \
  static constexpr X86Mem FUNC(const X86Gp& base, int32_t offset = 0) noexcept {      \
    return X86Mem(base, offset, SIZE);                                                \
  }                                                                                   \
  /*! Create a `[base + (index << shift) + offset]` memory operand. */                \
  static constexpr X86Mem FUNC(const X86Gp& base, const X86Gp& index, uint32_t shift = 0, int32_t offset = 0) noexcept { \
    return X86Mem(base, index, shift, offset, SIZE);                                  \
  }                                                                                   \
  /*! Create a `[base + (vec_index << shift) + offset]` memory operand. */            \
  static constexpr X86Mem FUNC(const X86Gp& base, const X86Vec& index, uint32_t shift = 0, int32_t offset = 0) noexcept { \
    return X86Mem(base, index, shift, offset, SIZE);                                  \
  }                                                                                   \
  /*! Create a `[base + offset]` memory operand. */                                   \
  static constexpr X86Mem FUNC(const Label& base, int32_t offset = 0) noexcept {      \
    return X86Mem(base, offset, SIZE);                                                \
  }                                                                                   \
  /*! Create a `[base + (index << shift) + offset]` memory operand. */                \
  static constexpr X86Mem FUNC(const Label& base, const X86Gp& index, uint32_t shift, int32_t offset = 0) noexcept { \
    return X86Mem(base, index, shift, offset, SIZE);                                  \
  }                                                                                   \
  /*! Create a `[rip + offset]` memory operand. */                                    \
  static constexpr X86Mem FUNC(const X86Rip& rip_, int32_t offset = 0) noexcept {     \
    return X86Mem(rip_, offset, SIZE);                                                \
  }                                                                                   \
  /*! Create a `[base + offset]` memory operand. */                                   \
  static constexpr X86Mem FUNC(uint64_t base) noexcept {                              \
    return X86Mem(base, SIZE);                                                        \
  }                                                                                   \
  /*! Create a `[base + (index << shift) + offset]` memory operand. */                \
  static constexpr X86Mem FUNC(uint64_t base, const X86Gp& index, uint32_t shift = 0) noexcept { \
    return X86Mem(base, index, shift, SIZE);                                          \
  }                                                                                   \
  /*! Create a `[base + (vec_index << shift) + offset]` memory operand. */            \
  static constexpr X86Mem FUNC(uint64_t base, const X86Vec& index, uint32_t shift = 0) noexcept { \
    return X86Mem(base, index, shift, SIZE, Mem::kSignatureMemAbs);                   \
  }                                                                                   \
  /*! Create a `[base + offset]` memory operand. */                                   \
  static constexpr X86Mem FUNC##_abs(uint64_t base) noexcept {                        \
    return X86Mem(base, SIZE);                                                        \
  }                                                                                   \
  /*! Create a `[base + (index << shift) + offset]` memory operand. */                \
  static constexpr X86Mem FUNC##_abs(uint64_t base, const X86Gp& index, uint32_t shift = 0) noexcept { \
    return X86Mem(base, index, shift, SIZE, Mem::kSignatureMemAbs);                   \
  }                                                                                   \
  /*! Create a `[base + (vec_index << shift) + offset]` memory operand. */            \
  static constexpr X86Mem FUNC##_abs(uint64_t base, const X86Vec& index, uint32_t shift = 0) noexcept { \
    return X86Mem(base, index, shift, SIZE, Mem::kSignatureMemAbs);                   \
  }

// Define memory operand constructors that use platform independent naming.
ASMJIT_X86_PTR_FN(ptr_8, 1)
ASMJIT_X86_PTR_FN(ptr_16, 2)
ASMJIT_X86_PTR_FN(ptr_32, 4)
ASMJIT_X86_PTR_FN(ptr_48, 6)
ASMJIT_X86_PTR_FN(ptr_64, 8)
ASMJIT_X86_PTR_FN(ptr_80, 10)
ASMJIT_X86_PTR_FN(ptr_128, 16)
ASMJIT_X86_PTR_FN(ptr_256, 32)
ASMJIT_X86_PTR_FN(ptr_512, 64)

// Define memory operand constructors that use X86/X64 specific naming.
ASMJIT_X86_PTR_FN(byte_ptr, 1)
ASMJIT_X86_PTR_FN(word_ptr, 2)
ASMJIT_X86_PTR_FN(dword_ptr, 4)
ASMJIT_X86_PTR_FN(qword_ptr, 8)
ASMJIT_X86_PTR_FN(tword_ptr, 10)
ASMJIT_X86_PTR_FN(oword_ptr, 16)
ASMJIT_X86_PTR_FN(dqword_ptr, 16)
ASMJIT_X86_PTR_FN(xmmword_ptr, 16)
ASMJIT_X86_PTR_FN(ymmword_ptr, 32)
ASMJIT_X86_PTR_FN(zmmword_ptr, 64)

#undef ASMJIT_X86_PTR_FN

} // anonymous namespace
} // x86 namespace

//! \}

ASMJIT_END_NAMESPACE

// [Guard]
#endif // _ASMJIT_X86_X86OPERAND_H
