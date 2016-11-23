// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_X86_X86OPERAND_H
#define _ASMJIT_X86_X86OPERAND_H

// [Dependencies]
#include "../base/arch.h"
#include "../base/assembler.h"
#include "../base/codecompiler.h"
#include "../base/operand.h"
#include "../base/utils.h"

// [Api-Begin]
#include "../asmjit_apibegin.h"

namespace asmjit {

// ============================================================================
// [Forward Declarations]
// ============================================================================

class X86Mem;
class X86Reg;
class X86Vec;

class X86Gp;
class X86Gpb;
class X86GpbLo;
class X86GpbHi;
class X86Gpw;
class X86Gpd;
class X86Gpq;
class X86Fp;
class X86Mm;
class X86KReg;
class X86Xmm;
class X86Ymm;
class X86Zmm;
class X86Bnd;
class X86Seg;
class X86Rip;
class X86CReg;
class X86DReg;

//! \addtogroup asmjit_x86
//! \{

// ============================================================================
// [asmjit::X86RegTraits<>]
// ============================================================================

//! Register traits (X86/X64).
template<uint32_t RegType>
struct X86RegTraits {
  enum { kTypeId = TypeId::kVoid }; // Everything is void by default.
};

// ============================================================================
// [asmjit::X86Mem]
// ============================================================================

//! Memory operand (X86).
class X86Mem : public Mem {
public:
  //! X86/X64 memory-operand flags layout:
  //!   * index shift       [1:0] index shift (0..3)
  //!   * Segment override  [4:2] see \ref X86Seg::Id
  //!   * CodeCompiler bits [7:6] defined by \ref Mem.
  ASMJIT_ENUM(MemFlags) {
    kMemShiftBits     = 0x3,
    kMemShiftIndex    = 0,
    kMemShiftMask     = kMemShiftBits << kMemShiftIndex,

    kMemSegmentBits   = 0x7,
    kMemSegmentIndex  = 2,
    kMemSegmentMask   = kMemSegmentBits << kMemSegmentIndex
  };

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Construct a default `X86Mem` operand, that points to [0].
  ASMJIT_INLINE X86Mem() noexcept : Mem(NoInit) { reset(); }
  ASMJIT_INLINE X86Mem(const X86Mem& other) noexcept : Mem(other) {}

  ASMJIT_INLINE X86Mem(const Label& base, int32_t off, uint32_t size = 0, uint32_t flags = 0) noexcept : Mem(NoInit) {
    uint32_t baseIndex = encodeBaseIndex(Label::kLabelTag, 0);
    _init_packed_op_b1_b2_sz_id(kOpMem, baseIndex, flags, size, kInvalidValue);
    _mem.base = base.getId();
    _mem.offsetLo32 = static_cast<uint32_t>(off);
  }

  ASMJIT_INLINE X86Mem(const Label& base, const Reg& index, uint32_t shift, int32_t off, uint32_t size = 0, uint32_t flags = 0) noexcept : Mem(NoInit) {
    ASMJIT_ASSERT(shift <= kMemShiftBits);

    uint32_t baseIndex = encodeBaseIndex(Label::kLabelTag, index.getRegType());
    _init_packed_op_b1_b2_sz_id(kOpMem, baseIndex, flags | (shift << kMemShiftIndex), size, index.getId());
    _mem.base = base.getId();
    _mem.offsetLo32 = static_cast<uint32_t>(off);
  }

  ASMJIT_INLINE X86Mem(const Reg& base, int32_t off, uint32_t size = 0, uint32_t flags = 0) noexcept : Mem(NoInit) {
    uint32_t baseIndex = encodeBaseIndex(base.getRegType(), 0);
    _init_packed_op_b1_b2_sz_id(kOpMem, baseIndex, flags, size, kInvalidValue);
    _mem.base = base.getId();
    _mem.offsetLo32 = static_cast<uint32_t>(off);
  }

  ASMJIT_INLINE X86Mem(const Reg& base, const Reg& index, uint32_t shift, int32_t off, uint32_t size = 0, uint32_t flags = 0) noexcept : Mem(NoInit) {
    ASMJIT_ASSERT(shift <= kMemShiftBits);

    uint32_t baseIndex =  encodeBaseIndex(base.getRegType(), index.getRegType());
    _init_packed_op_b1_b2_sz_id(kOpMem, baseIndex, flags | (shift << kMemShiftIndex), size, index.getId());
    _mem.base = base.getId();
    _mem.offsetLo32 = static_cast<uint32_t>(off);
  }

  ASMJIT_INLINE X86Mem(uint64_t base, uint32_t size = 0, uint32_t flags = 0) noexcept : Mem(NoInit) {
    uint32_t baseIndex = encodeBaseIndex(0, 0);
    _init_packed_op_b1_b2_sz_id(kOpMem, baseIndex, flags, size, kInvalidValue);
    _mem.offset64 = base;
  }

  ASMJIT_INLINE X86Mem(uint64_t base, const Reg& index, uint32_t shift = 0, uint32_t size = 0, uint32_t flags = 0) noexcept : Mem(NoInit) {
    ASMJIT_ASSERT(shift <= kMemShiftBits);

    uint32_t baseIndex = encodeBaseIndex(0, index.getRegType());
    _init_packed_op_b1_b2_sz_id(kOpMem, baseIndex, flags | (shift << kMemShiftIndex), size, index.getId());
    _mem.offset64 = base;
  }

  ASMJIT_INLINE X86Mem(const _Init& init,
    uint32_t baseType, uint32_t baseId,
    uint32_t indexType, uint32_t indexId,
    int32_t off, uint32_t size, uint32_t flags) noexcept : Mem(init, baseType, baseId, indexType, indexId, off, size, flags) {}

  explicit ASMJIT_INLINE X86Mem(const _NoInit&) noexcept : Mem(NoInit) {}

  // --------------------------------------------------------------------------
  // [X86Mem]
  // --------------------------------------------------------------------------

  //! Clone the memory operand.
  ASMJIT_INLINE X86Mem clone() const noexcept { return X86Mem(*this); }

  using Mem::setIndex;

  ASMJIT_INLINE void setIndex(const Reg& index, uint32_t shift) noexcept {
    ASMJIT_ASSERT(shift <= kMemShiftBits);
    setIndex(index);
    setShift(shift);
  }

  //! Get if the memory operand has shift (aka scale) constant.
  ASMJIT_INLINE bool hasShift() const noexcept { return getShift() != 0; }
  //! Get the memory operand's shift (aka scale) constant.
  ASMJIT_INLINE uint32_t getShift() const noexcept { return _unpackFromFlags(kMemShiftIndex, kMemShiftBits); }
  //! Set the memory operand's shift (aka scale) constant.
  ASMJIT_INLINE void setShift(uint32_t shift) noexcept { _packToFlags(shift, kMemShiftIndex, kMemShiftBits); }
  //! Reset the memory operand's shift (aka scale) constant to zero.
  ASMJIT_INLINE void resetShift() noexcept { setShift(0); }

  //! Get if the memory operand has a segment override.
  ASMJIT_INLINE bool hasSegment() const noexcept { return (_mem.flags & kMemSegmentMask) != 0; }
  //! Get associated segment override as `X86Seg` operand.
  ASMJIT_INLINE X86Seg getSegment() const noexcept;
  //! Get segment override as id, see \ref X86Seg::Id.
  ASMJIT_INLINE uint32_t getSegmentId() const noexcept { return _unpackFromFlags(kMemSegmentIndex, kMemSegmentBits); }
  //! Set the segment override to `seg`.
  ASMJIT_INLINE void setSegment(const X86Seg& seg) noexcept { return setSegmentId(reinterpret_cast<const Reg&>(seg).getId()); }
  //! Set the segment override to `id`.
  ASMJIT_INLINE void setSegmentId(uint32_t id) noexcept { _packToFlags(id, kMemSegmentIndex, kMemSegmentBits); }
  //! Reset the segment override.
  ASMJIT_INLINE void resetSegment() noexcept { setSegmentId(0); }

  //! Get new memory operand adjusted by `off`.
  ASMJIT_INLINE X86Mem adjusted(int64_t off) const noexcept {
    X86Mem result(*this);
    result.addOffset(off);
    return result;
  }

  // --------------------------------------------------------------------------
  // [Operator Overload]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE X86Mem& operator=(const X86Mem& other) noexcept { copyFrom(other); return *this; }
};

// ============================================================================
// [asmjit::X86Reg]
// ============================================================================

//! Register (X86/X64).
class X86Reg : public Reg {
public:
  //! Register type.
  ASMJIT_ENUM(RegType) {
    // NOTE: Don't change these constants, they are essential to many lookup
    // tables and changing them may break AsmJit.

    kRegNone            = Reg::kRegNone, //!< No register type or invalid register.
    kRegRip             = Reg::kRegRip,  //!< Instruction pointer (EIP, RIP).
    kRegSeg             = 3,             //!< Segment register (None, ES, CS, SS, DS, FS, GS).
    kRegGpbLo           = 4,             //!< Low GPB register (AL, BL, CL, DL, ...).
    kRegGpbHi           = 5,             //!< High GPB register (AH, BH, CH, DH only).
    kRegGpw             = 6,             //!< GPW register.
    kRegGpd             = 7,             //!< GPD register.
    kRegGpq             = 8,             //!< GPQ register (X64).
    kRegFp              = 9,             //!< FPU (x87) register.
    kRegMm              = 10,            //!< MMX register.
    kRegK               = 11,            //!< K register (AVX512+).
    kRegXmm             = 12,            //!< XMM register (SSE+).
    kRegYmm             = 13,            //!< YMM register (AVX+).
    kRegZmm             = 14,            //!< ZMM register (AVX512+).
    kRegFuture          = 15,            //!< Reserved for future 1024-bit SIMD register.
    kRegBnd             = 16,            //!< Bound register (BND).
    kRegCr              = 17,            //!< Control register (CR).
    kRegDr              = 18,            //!< Debug register (DR).
    kRegCount           = 19             //!< Count of register types.
  };

  //! Register kind.
  ASMJIT_ENUM(Kind) {
    kKindGp             = 0,             //!< GP register kind or none (universal).
    kKindMm             = 1,             //!< MMX register kind.
    kKindK              = 2,             //!< K register kind.
    kKindVec            = 3,             //!< XMM|YMM|ZMM register kind.
    _kKindRACount       = 4,             //!< Count of register kinds used by \ref X86Compiler.

    kKindFp             = 4,             //!< FPU (x87) register kind.
    kKindCr             = 5,             //!< Control register kind.
    kKindDr             = 6,             //!< Debug register kind.
    kKindBnd            = 7,             //!< Bound register kind.
    kKindSeg            = 8,             //!< Segment register kind.
    kKindRip            = 9,             //!< IP register kind.
    kKindCount          = 10             //!< Count of all register kinds.
  };

  ASMJIT_DEFINE_ABSTRACT_REG(X86Reg, Reg)

  //! Get if the register is a GP register (any size).
  ASMJIT_INLINE bool isGp() const noexcept { return _reg.regKind == kKindGp; }
  //! Get if the register is a GPB register (8-bit).
  ASMJIT_INLINE bool isGpb() const noexcept { return _reg.size == 1; }
  //! Get if the register is XMM, YMM, or ZMM (SIMD).
  ASMJIT_INLINE bool isVec() const noexcept { return _reg.regKind == kKindVec; }

  // This is not that well designed as X86RegTraits is defined after the
  // X64Reg, which means that we cannot use the traits here, which would be great.

  //! Get if the register is RIP.
  ASMJIT_INLINE bool isRip() const noexcept { return hasSignature(kOpReg, kRegRip, kKindRip, 8); }
  //! Get if the register is a segment register.
  ASMJIT_INLINE bool isSeg() const noexcept { return hasSignature(kOpReg, kRegSeg, kKindSeg, 2); }
  //! Get if the register is a low GPB register (8-bit).
  ASMJIT_INLINE bool isGpbLo() const noexcept { return hasSignature(kOpReg, kRegGpbLo, kKindGp, 1); }
  //! Get if the register is a high GPB register (8-bit).
  ASMJIT_INLINE bool isGpbHi() const noexcept { return hasSignature(kOpReg, kRegGpbHi, kKindGp, 1); }
  //! Get if the register is a GPW register (16-bit).
  ASMJIT_INLINE bool isGpw() const noexcept { return hasSignature(kOpReg, kRegGpw, kKindGp, 2); }
  //! Get if the register is a GPD register (32-bit).
  ASMJIT_INLINE bool isGpd() const noexcept { return hasSignature(kOpReg, kRegGpd, kKindGp, 4); }
  //! Get if the register is a GPQ register (64-bit).
  ASMJIT_INLINE bool isGpq() const noexcept { return hasSignature(kOpReg, kRegGpq, kKindGp, 8); }
  //! Get if the register is an FPU register (80-bit).
  ASMJIT_INLINE bool isFp() const noexcept { return hasSignature(kOpReg, kRegFp, kKindFp, 10); }
  //! Get if the register is an MMX register (64-bit).
  ASMJIT_INLINE bool isMm() const noexcept { return hasSignature(kOpReg, kRegMm, kKindMm, 8); }
  //! Get if the register is a K register (64-bit).
  ASMJIT_INLINE bool isK() const noexcept { return hasSignature(kOpReg, kRegK, kKindK, 8); }
  //! Get if the register is an XMM register (128-bit).
  ASMJIT_INLINE bool isXmm() const noexcept { return hasSignature(kOpReg, kRegXmm, kKindVec, 16); }
  //! Get if the register is a YMM register (256-bit).
  ASMJIT_INLINE bool isYmm() const noexcept { return hasSignature(kOpReg, kRegYmm, kKindVec, 32); }
  //! Get if the register is a ZMM register (512-bit).
  ASMJIT_INLINE bool isZmm() const noexcept { return hasSignature(kOpReg, kRegZmm, kKindVec, 64); }
  //! Get if the register is a bound register.
  ASMJIT_INLINE bool isBnd() const noexcept { return hasSignature(kOpReg, kRegBnd, kKindBnd, 16); }
  //! Get if the register is a control register.
  ASMJIT_INLINE bool isCr() const noexcept { return hasSignature(kOpReg, kRegCr, kKindCr, 8); }
  //! Get if the register is a debug register.
  ASMJIT_INLINE bool isDr() const noexcept { return hasSignature(kOpReg, kRegDr, kKindDr, 8); }

  template<uint32_t Type>
  ASMJIT_INLINE void setX86RegT(uint32_t id) noexcept {
    setSignature(X86RegTraits<Type>::kSignature);
    setId(id);
  }

  ASMJIT_INLINE void setTypeAndId(uint32_t regType, uint32_t id) noexcept {
    ASMJIT_ASSERT(regType < kRegCount);
    setSignature(signatureOf(regType));
    setId(id);
  }

  static ASMJIT_INLINE uint32_t kindOf(uint32_t regType) noexcept;
  template<uint32_t Type>
  static ASMJIT_INLINE uint32_t kindOf() noexcept { return X86RegTraits<Type>::kKind; }

  static ASMJIT_INLINE uint32_t signatureOf(uint32_t regType) noexcept;
  template<uint32_t Type>
  static ASMJIT_INLINE uint32_t signatureOf() noexcept { return X86RegTraits<Type>::kSignature; }

  static ASMJIT_INLINE uint32_t signatureOfVecByType(uint32_t typeId) noexcept {
    return typeId <= TypeId::_kVec128End ? signatureOf<kRegXmm>() :
           typeId <= TypeId::_kVec256End ? signatureOf<kRegYmm>() : signatureOf<kRegZmm>() ;
  }

  static ASMJIT_INLINE uint32_t signatureOfVecBySize(uint32_t size) noexcept {
    return size <= 16 ? signatureOf<kRegXmm>() :
           size <= 32 ? signatureOf<kRegYmm>() : signatureOf<kRegZmm>() ;
  }

  static ASMJIT_INLINE bool isGp(const Operand_& op) noexcept {
    // Check operand type and register kind. Not interested in register type and size.
    const uint32_t kMsk = Utils::pack32_4x8(0xFF           , 0x00, 0xFF           , 0x00);
    const uint32_t kSgn = Utils::pack32_4x8(Operand::kOpReg, 0x00, X86Reg::kKindGp, 0x00);
    return (op.getSignature() & kMsk) == kSgn;
  }

  //! Get if the `op` operand is either a low or high 8-bit GPB register.
  static ASMJIT_INLINE bool isGpb(const Operand_& op) noexcept {
    // Check operand type, register kind, and size. Not interested in register type.
    const uint32_t kMsk = Utils::pack32_4x8(0xFF           , 0x00, 0xFF           , 0xFF);
    const uint32_t kSgn = Utils::pack32_4x8(Operand::kOpReg, 0x00, X86Reg::kKindGp, 1   );
    return (op.getSignature() & kMsk) == kSgn;
  }

  //! Get if the `op` operand is either a low or high 8-bit GPB register.
  static ASMJIT_INLINE bool isVec(const Operand_& op) noexcept {
    // Check operand type and register kind. Not interested in register type and size.
    const uint32_t kMsk = Utils::pack32_4x8(0xFF           , 0x00, 0xFF            , 0x00);
    const uint32_t kSgn = Utils::pack32_4x8(Operand::kOpReg, 0x00, X86Reg::kKindVec, 0   );
    return (op.getSignature() & kMsk) == kSgn;
  }

  static ASMJIT_INLINE bool isRip(const Operand_& op) noexcept { return op.as<X86Reg>().isRip(); }
  static ASMJIT_INLINE bool isSeg(const Operand_& op) noexcept { return op.as<X86Reg>().isSeg(); }
  static ASMJIT_INLINE bool isGpbLo(const Operand_& op) noexcept { return op.as<X86Reg>().isGpbLo(); }
  static ASMJIT_INLINE bool isGpbHi(const Operand_& op) noexcept { return op.as<X86Reg>().isGpbHi(); }
  static ASMJIT_INLINE bool isGpw(const Operand_& op) noexcept { return op.as<X86Reg>().isGpw(); }
  static ASMJIT_INLINE bool isGpd(const Operand_& op) noexcept { return op.as<X86Reg>().isGpd(); }
  static ASMJIT_INLINE bool isGpq(const Operand_& op) noexcept { return op.as<X86Reg>().isGpq(); }
  static ASMJIT_INLINE bool isFp(const Operand_& op) noexcept { return op.as<X86Reg>().isFp(); }
  static ASMJIT_INLINE bool isMm(const Operand_& op) noexcept { return op.as<X86Reg>().isMm(); }
  static ASMJIT_INLINE bool isK(const Operand_& op) noexcept { return op.as<X86Reg>().isK(); }
  static ASMJIT_INLINE bool isXmm(const Operand_& op) noexcept { return op.as<X86Reg>().isXmm(); }
  static ASMJIT_INLINE bool isYmm(const Operand_& op) noexcept { return op.as<X86Reg>().isYmm(); }
  static ASMJIT_INLINE bool isZmm(const Operand_& op) noexcept { return op.as<X86Reg>().isZmm(); }
  static ASMJIT_INLINE bool isBnd(const Operand_& op) noexcept { return op.as<X86Reg>().isBnd(); }
  static ASMJIT_INLINE bool isCr(const Operand_& op) noexcept { return op.as<X86Reg>().isCr(); }
  static ASMJIT_INLINE bool isDr(const Operand_& op) noexcept { return op.as<X86Reg>().isDr(); }

  static ASMJIT_INLINE bool isGp(const Operand_& op, uint32_t id) noexcept { return isGp(op) & (op.getId() == id); }
  static ASMJIT_INLINE bool isGpb(const Operand_& op, uint32_t id) noexcept { return isGpb(op) & (op.getId() == id); }
  static ASMJIT_INLINE bool isVec(const Operand_& op, uint32_t id) noexcept { return isVec(op) & (op.getId() == id); }

  static ASMJIT_INLINE bool isRip(const Operand_& op, uint32_t id) noexcept { return isRip(op) & (op.getId() == id); }
  static ASMJIT_INLINE bool isSeg(const Operand_& op, uint32_t id) noexcept { return isSeg(op) & (op.getId() == id); }
  static ASMJIT_INLINE bool isGpbLo(const Operand_& op, uint32_t id) noexcept { return isGpbLo(op) & (op.getId() == id); }
  static ASMJIT_INLINE bool isGpbHi(const Operand_& op, uint32_t id) noexcept { return isGpbHi(op) & (op.getId() == id); }
  static ASMJIT_INLINE bool isGpw(const Operand_& op, uint32_t id) noexcept { return isGpw(op) & (op.getId() == id); }
  static ASMJIT_INLINE bool isGpd(const Operand_& op, uint32_t id) noexcept { return isGpd(op) & (op.getId() == id); }
  static ASMJIT_INLINE bool isGpq(const Operand_& op, uint32_t id) noexcept { return isGpq(op) & (op.getId() == id); }
  static ASMJIT_INLINE bool isFp(const Operand_& op, uint32_t id) noexcept { return isFp(op) & (op.getId() == id); }
  static ASMJIT_INLINE bool isMm(const Operand_& op, uint32_t id) noexcept { return isMm(op) & (op.getId() == id); }
  static ASMJIT_INLINE bool isK(const Operand_& op, uint32_t id) noexcept { return isK(op) & (op.getId() == id); }
  static ASMJIT_INLINE bool isXmm(const Operand_& op, uint32_t id) noexcept { return isXmm(op) & (op.getId() == id); }
  static ASMJIT_INLINE bool isYmm(const Operand_& op, uint32_t id) noexcept { return isYmm(op) & (op.getId() == id); }
  static ASMJIT_INLINE bool isZmm(const Operand_& op, uint32_t id) noexcept { return isZmm(op) & (op.getId() == id); }
  static ASMJIT_INLINE bool isBnd(const Operand_& op, uint32_t id) noexcept { return isBnd(op) & (op.getId() == id); }
  static ASMJIT_INLINE bool isCr(const Operand_& op, uint32_t id) noexcept { return isCr(op) & (op.getId() == id); }
  static ASMJIT_INLINE bool isDr(const Operand_& op, uint32_t id) noexcept { return isDr(op) & (op.getId() == id); }

  // --------------------------------------------------------------------------
  // [Memory Cast]
  // --------------------------------------------------------------------------

  // TODO: This will get deprecated after a new register allocator is implemented.
#define ASMJIT_DEFINE_REG_MEM(NAME, SIZE) \
  /*! Cast a virtual register to a memory operand. */ \
  ASMJIT_INLINE X86Mem NAME(int32_t offset = 0) const noexcept { \
    return X86Mem(*this, offset, SIZE, Mem::kFlagRegHome); \
  } \
  /*! \overload */ \
  ASMJIT_INLINE X86Mem NAME(const X86Gp& index, uint32_t shift = 0, int32_t offset = 0) const noexcept { \
    return X86Mem(*this, reinterpret_cast<const Reg&>(index), shift, offset, SIZE, Mem::kFlagRegHome); \
  }

  ASMJIT_DEFINE_REG_MEM(m, getSize())
  ASMJIT_DEFINE_REG_MEM(m8, 1)
  ASMJIT_DEFINE_REG_MEM(m16, 2)
  ASMJIT_DEFINE_REG_MEM(m32, 4)
  ASMJIT_DEFINE_REG_MEM(m64, 8)
  ASMJIT_DEFINE_REG_MEM(m80, 10)
  ASMJIT_DEFINE_REG_MEM(m128, 16)
  ASMJIT_DEFINE_REG_MEM(m256, 32)
  ASMJIT_DEFINE_REG_MEM(m512, 64)

#undef ASMJIT_DEFINE_REG_MEM
};

#define ASMJIT_DEFINE_X86_REG_TRAITS(REG, TYPE, KIND, SIZE, TYPE_ID)  \
template<>                                                            \
struct X86RegTraits< TYPE > {                                         \
  typedef REG Reg;                                                    \
                                                                      \
  enum {                                                              \
    kType      = TYPE,                                                \
    kKind      = KIND,                                                \
    kSize      = SIZE,                                                \
                                                                      \
    kTypeId    = TYPE_ID,                                             \
    kSignature = ASMJIT_PACK32_4x8(Operand::kOpReg, TYPE, KIND, SIZE) \
  };                                                                  \
}
ASMJIT_DEFINE_X86_REG_TRAITS(X86Rip , X86Reg::kRegRip         , X86Reg::kKindRip, 8 , TypeId::kVoid  );
ASMJIT_DEFINE_X86_REG_TRAITS(X86Seg , X86Reg::kRegSeg         , X86Reg::kKindSeg, 2 , TypeId::kVoid  );
ASMJIT_DEFINE_X86_REG_TRAITS(X86Gp  , X86Reg::kRegGpbLo       , X86Reg::kKindGp , 1 , TypeId::kI8    );
ASMJIT_DEFINE_X86_REG_TRAITS(X86Gp  , X86Reg::kRegGpbHi       , X86Reg::kKindGp , 1 , TypeId::kI8    );
ASMJIT_DEFINE_X86_REG_TRAITS(X86Gp  , X86Reg::kRegGpw         , X86Reg::kKindGp , 2 , TypeId::kI16   );
ASMJIT_DEFINE_X86_REG_TRAITS(X86Gp  , X86Reg::kRegGpd         , X86Reg::kKindGp , 4 , TypeId::kI32   );
ASMJIT_DEFINE_X86_REG_TRAITS(X86Gp  , X86Reg::kRegGpq         , X86Reg::kKindGp , 8 , TypeId::kI64   );
ASMJIT_DEFINE_X86_REG_TRAITS(X86Fp  , X86Reg::kRegFp          , X86Reg::kKindFp , 10, TypeId::kVoid  );
ASMJIT_DEFINE_X86_REG_TRAITS(X86Mm  , X86Reg::kRegMm          , X86Reg::kKindMm , 8 , TypeId::kMmx64 );
ASMJIT_DEFINE_X86_REG_TRAITS(X86KReg, X86Reg::kRegK           , X86Reg::kKindK  , 8 , TypeId::kVoid  );
ASMJIT_DEFINE_X86_REG_TRAITS(X86Xmm , X86Reg::kRegXmm         , X86Reg::kKindVec, 16, TypeId::kI32x4 );
ASMJIT_DEFINE_X86_REG_TRAITS(X86Ymm , X86Reg::kRegYmm         , X86Reg::kKindVec, 32, TypeId::kI32x8 );
ASMJIT_DEFINE_X86_REG_TRAITS(X86Zmm , X86Reg::kRegZmm         , X86Reg::kKindVec, 64, TypeId::kI32x16);
ASMJIT_DEFINE_X86_REG_TRAITS(X86Bnd , X86Reg::kRegBnd         , X86Reg::kKindBnd, 16, TypeId::kVoid  );
ASMJIT_DEFINE_X86_REG_TRAITS(X86CReg, X86Reg::kRegCr          , X86Reg::kKindCr , 8 , TypeId::kVoid  );
ASMJIT_DEFINE_X86_REG_TRAITS(X86DReg, X86Reg::kRegDr          , X86Reg::kKindDr , 8 , TypeId::kVoid  );
#undef ASMJIT_DEFINE_X86_REG_TRAITS

// ============================================================================
// [asmjit::X86...]
// ============================================================================

//! General purpose register (X86/X64).
class X86Gp : public X86Reg {
public:
  //! X86/X64 physical id.
  //!
  //! NOTE: Register indexes have been reduced to only support general purpose
  //! registers. There is no need to have enumerations with number suffix that
  //! expands to the exactly same value as the suffix value itself.
  ASMJIT_ENUM(Id) {
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

  ASMJIT_DEFINE_ABSTRACT_REG(X86Gp, X86Reg)

  //! Cast this register to 8-bit (LO) part.
  ASMJIT_INLINE X86GpbLo r8() const noexcept;
  //! Cast this register to 8-bit (LO) part.
  ASMJIT_INLINE X86GpbLo r8Lo() const noexcept;
  //! Cast this register to 8-bit (HI) part.
  ASMJIT_INLINE X86GpbHi r8Hi() const noexcept;
  //! Cast this register to 16-bit.
  ASMJIT_INLINE X86Gpw r16() const noexcept;
  //! Cast this register to 32-bit.
  ASMJIT_INLINE X86Gpd r32() const noexcept;
  //! Cast this register to 64-bit.
  ASMJIT_INLINE X86Gpq r64() const noexcept;
};

//! XMM|YMM|ZMM register (X86/X64).
class X86Vec : public X86Reg {
  ASMJIT_DEFINE_ABSTRACT_REG(X86Vec, X86Reg)

  //! Cast this register to XMM (clone).
  ASMJIT_INLINE X86Xmm xmm() const noexcept;
  //! Cast this register to YMM.
  ASMJIT_INLINE X86Ymm ymm() const noexcept;
  //! Cast this register to ZMM.
  ASMJIT_INLINE X86Zmm zmm() const noexcept;
};

//! Segment register (X86/X64).
class X86Seg : public X86Reg {
  ASMJIT_DEFINE_FINAL_REG(X86Seg, X86Reg, X86RegTraits<kRegSeg>)

  //! X86/X64 segment id.
  ASMJIT_ENUM(Id) {
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

ASMJIT_INLINE X86GpbLo X86Gp::r8() const noexcept { return X86GpbLo(getId()); }
ASMJIT_INLINE X86GpbLo X86Gp::r8Lo() const noexcept { return X86GpbLo(getId()); }
ASMJIT_INLINE X86GpbHi X86Gp::r8Hi() const noexcept { return X86GpbHi(getId()); }
ASMJIT_INLINE X86Gpw X86Gp::r16() const noexcept { return X86Gpw(getId()); }
ASMJIT_INLINE X86Gpd X86Gp::r32() const noexcept { return X86Gpd(getId()); }
ASMJIT_INLINE X86Gpq X86Gp::r64() const noexcept { return X86Gpq(getId()); }
ASMJIT_INLINE X86Xmm X86Vec::xmm() const noexcept { return X86Xmm(*this, getId()); }
ASMJIT_INLINE X86Ymm X86Vec::ymm() const noexcept { return X86Ymm(*this, getId()); }
ASMJIT_INLINE X86Zmm X86Vec::zmm() const noexcept { return X86Zmm(*this, getId()); }

ASMJIT_INLINE X86Seg X86Mem::getSegment() const noexcept {
  return X86Seg(Init, X86RegTraits<X86Reg::kRegSeg>::kSignature, getSegmentId());
}

ASMJIT_DEFINE_TYPE_ID(X86Gpb, TypeId::kI8);
ASMJIT_DEFINE_TYPE_ID(X86Gpw, TypeId::kI16);
ASMJIT_DEFINE_TYPE_ID(X86Gpd, TypeId::kI32);
ASMJIT_DEFINE_TYPE_ID(X86Gpq, TypeId::kI64);
ASMJIT_DEFINE_TYPE_ID(X86Mm , TypeId::kMmx64);
ASMJIT_DEFINE_TYPE_ID(X86Xmm, TypeId::kI32x4);
ASMJIT_DEFINE_TYPE_ID(X86Ymm, TypeId::kI32x8);
ASMJIT_DEFINE_TYPE_ID(X86Zmm, TypeId::kI32x16);

// ============================================================================
// [asmjit::X86OpData]
// ============================================================================

struct X86OpData {
  // --------------------------------------------------------------------------
  // [Signatures]
  // --------------------------------------------------------------------------

  //! Information about all architecture registers.
  ArchRegs archRegs;

  // --------------------------------------------------------------------------
  // [Operands]
  // --------------------------------------------------------------------------

  // Prevent calling constructors of these registers when exporting.
#if defined(ASMJIT_EXPORTS_X86_OPERAND)
# define ASMJIT_DEFINE_REG_DATA(REG) Operand_
#else
# define ASMJIT_DEFINE_REG_DATA(REG) REG
#endif
  ASMJIT_DEFINE_REG_DATA(X86Rip ) rip[1];
  ASMJIT_DEFINE_REG_DATA(X86Seg ) seg[7];
  ASMJIT_DEFINE_REG_DATA(X86Gp  ) gpbLo[16];
  ASMJIT_DEFINE_REG_DATA(X86Gp  ) gpbHi[4];
  ASMJIT_DEFINE_REG_DATA(X86Gp  ) gpw[16];
  ASMJIT_DEFINE_REG_DATA(X86Gp  ) gpd[16];
  ASMJIT_DEFINE_REG_DATA(X86Gp  ) gpq[16];
  ASMJIT_DEFINE_REG_DATA(X86Fp  ) fp[8];
  ASMJIT_DEFINE_REG_DATA(X86Mm  ) mm[8];
  ASMJIT_DEFINE_REG_DATA(X86KReg) k[8];
  ASMJIT_DEFINE_REG_DATA(X86Xmm ) xmm[32];
  ASMJIT_DEFINE_REG_DATA(X86Ymm ) ymm[32];
  ASMJIT_DEFINE_REG_DATA(X86Zmm ) zmm[32];
  ASMJIT_DEFINE_REG_DATA(X86Bnd ) bnd[4];
  ASMJIT_DEFINE_REG_DATA(X86CReg) cr[16];
  ASMJIT_DEFINE_REG_DATA(X86DReg) dr[16];
#undef ASMJIT_DEFINE_REG_DATA
};
ASMJIT_VARAPI const X86OpData x86OpData;

// ... X86Reg methods that require `x86OpData`.
ASMJIT_INLINE uint32_t X86Reg::signatureOf(uint32_t regType) noexcept {
  ASMJIT_ASSERT(regType <= Reg::kRegMax);
  return x86OpData.archRegs.regInfo[regType].signature;
}

ASMJIT_INLINE uint32_t X86Reg::kindOf(uint32_t regType) noexcept {
  ASMJIT_ASSERT(regType <= Reg::kRegMax);
  return x86OpData.archRegs.regInfo[regType].regKind;
}

// ============================================================================
// [asmjit::x86]
// ============================================================================

namespace x86 {

// ============================================================================
// [asmjit::x86 - Reg]
// ============================================================================

#if !defined(ASMJIT_EXPORTS_X86_OPERAND)
namespace {
#define ASMJIT_PHYS_REG(TYPE, NAME, PROPERTY) \
  static const TYPE& NAME = x86OpData.PROPERTY

ASMJIT_PHYS_REG(X86Rip , rip  , rip[0]);    //!< RIP register.
ASMJIT_PHYS_REG(X86Seg , es   , seg[1]);    //!< CS segment register.
ASMJIT_PHYS_REG(X86Seg , cs   , seg[2]);    //!< SS segment register.
ASMJIT_PHYS_REG(X86Seg , ss   , seg[3]);    //!< DS segment register.
ASMJIT_PHYS_REG(X86Seg , ds   , seg[4]);    //!< ES segment register.
ASMJIT_PHYS_REG(X86Seg , fs   , seg[5]);    //!< FS segment register.
ASMJIT_PHYS_REG(X86Seg , gs   , seg[6]);    //!< GS segment register.

ASMJIT_PHYS_REG(X86Gp  , al   , gpbLo[0]);  //!< 8-bit low GPB register.
ASMJIT_PHYS_REG(X86Gp  , cl   , gpbLo[1]);  //!< 8-bit low GPB register.
ASMJIT_PHYS_REG(X86Gp  , dl   , gpbLo[2]);  //!< 8-bit low GPB register.
ASMJIT_PHYS_REG(X86Gp  , bl   , gpbLo[3]);  //!< 8-bit low GPB register.
ASMJIT_PHYS_REG(X86Gp  , spl  , gpbLo[4]);  //!< 8-bit low GPB register (X64).
ASMJIT_PHYS_REG(X86Gp  , bpl  , gpbLo[5]);  //!< 8-bit low GPB register (X64).
ASMJIT_PHYS_REG(X86Gp  , sil  , gpbLo[6]);  //!< 8-bit low GPB register (X64).
ASMJIT_PHYS_REG(X86Gp  , dil  , gpbLo[7]);  //!< 8-bit low GPB register (X64).
ASMJIT_PHYS_REG(X86Gp  , r8b  , gpbLo[8]);  //!< 8-bit low GPB register (X64).
ASMJIT_PHYS_REG(X86Gp  , r9b  , gpbLo[9]);  //!< 8-bit low GPB register (X64).
ASMJIT_PHYS_REG(X86Gp  , r10b , gpbLo[10]); //!< 8-bit low GPB register (X64).
ASMJIT_PHYS_REG(X86Gp  , r11b , gpbLo[11]); //!< 8-bit low GPB register (X64).
ASMJIT_PHYS_REG(X86Gp  , r12b , gpbLo[12]); //!< 8-bit low GPB register (X64).
ASMJIT_PHYS_REG(X86Gp  , r13b , gpbLo[13]); //!< 8-bit low GPB register (X64).
ASMJIT_PHYS_REG(X86Gp  , r14b , gpbLo[14]); //!< 8-bit low GPB register (X64).
ASMJIT_PHYS_REG(X86Gp  , r15b , gpbLo[15]); //!< 8-bit low GPB register (X64).

ASMJIT_PHYS_REG(X86Gp  , ah   , gpbHi[0]);  //!< 8-bit high GPB register.
ASMJIT_PHYS_REG(X86Gp  , ch   , gpbHi[1]);  //!< 8-bit high GPB register.
ASMJIT_PHYS_REG(X86Gp  , dh   , gpbHi[2]);  //!< 8-bit high GPB register.
ASMJIT_PHYS_REG(X86Gp  , bh   , gpbHi[3]);  //!< 8-bit high GPB register.

ASMJIT_PHYS_REG(X86Gp  , ax   , gpw[0]);    //!< 16-bit GPW register.
ASMJIT_PHYS_REG(X86Gp  , cx   , gpw[1]);    //!< 16-bit GPW register.
ASMJIT_PHYS_REG(X86Gp  , dx   , gpw[2]);    //!< 16-bit GPW register.
ASMJIT_PHYS_REG(X86Gp  , bx   , gpw[3]);    //!< 16-bit GPW register.
ASMJIT_PHYS_REG(X86Gp  , sp   , gpw[4]);    //!< 16-bit GPW register.
ASMJIT_PHYS_REG(X86Gp  , bp   , gpw[5]);    //!< 16-bit GPW register.
ASMJIT_PHYS_REG(X86Gp  , si   , gpw[6]);    //!< 16-bit GPW register.
ASMJIT_PHYS_REG(X86Gp  , di   , gpw[7]);    //!< 16-bit GPW register.
ASMJIT_PHYS_REG(X86Gp  , r8w  , gpw[8]);    //!< 16-bit GPW register (X64).
ASMJIT_PHYS_REG(X86Gp  , r9w  , gpw[9]);    //!< 16-bit GPW register (X64).
ASMJIT_PHYS_REG(X86Gp  , r10w , gpw[10]);   //!< 16-bit GPW register (X64).
ASMJIT_PHYS_REG(X86Gp  , r11w , gpw[11]);   //!< 16-bit GPW register (X64).
ASMJIT_PHYS_REG(X86Gp  , r12w , gpw[12]);   //!< 16-bit GPW register (X64).
ASMJIT_PHYS_REG(X86Gp  , r13w , gpw[13]);   //!< 16-bit GPW register (X64).
ASMJIT_PHYS_REG(X86Gp  , r14w , gpw[14]);   //!< 16-bit GPW register (X64).
ASMJIT_PHYS_REG(X86Gp  , r15w , gpw[15]);   //!< 16-bit GPW register (X64).

ASMJIT_PHYS_REG(X86Gp  , eax  , gpd[0]);    //!< 32-bit GPD register.
ASMJIT_PHYS_REG(X86Gp  , ecx  , gpd[1]);    //!< 32-bit GPD register.
ASMJIT_PHYS_REG(X86Gp  , edx  , gpd[2]);    //!< 32-bit GPD register.
ASMJIT_PHYS_REG(X86Gp  , ebx  , gpd[3]);    //!< 32-bit GPD register.
ASMJIT_PHYS_REG(X86Gp  , esp  , gpd[4]);    //!< 32-bit GPD register.
ASMJIT_PHYS_REG(X86Gp  , ebp  , gpd[5]);    //!< 32-bit GPD register.
ASMJIT_PHYS_REG(X86Gp  , esi  , gpd[6]);    //!< 32-bit GPD register.
ASMJIT_PHYS_REG(X86Gp  , edi  , gpd[7]);    //!< 32-bit GPD register.
ASMJIT_PHYS_REG(X86Gp  , r8d  , gpd[8]);    //!< 32-bit GPD register (X64).
ASMJIT_PHYS_REG(X86Gp  , r9d  , gpd[9]);    //!< 32-bit GPD register (X64).
ASMJIT_PHYS_REG(X86Gp  , r10d , gpd[10]);   //!< 32-bit GPD register (X64).
ASMJIT_PHYS_REG(X86Gp  , r11d , gpd[11]);   //!< 32-bit GPD register (X64).
ASMJIT_PHYS_REG(X86Gp  , r12d , gpd[12]);   //!< 32-bit GPD register (X64).
ASMJIT_PHYS_REG(X86Gp  , r13d , gpd[13]);   //!< 32-bit GPD register (X64).
ASMJIT_PHYS_REG(X86Gp  , r14d , gpd[14]);   //!< 32-bit GPD register (X64).
ASMJIT_PHYS_REG(X86Gp  , r15d , gpd[15]);   //!< 32-bit GPD register (X64).

ASMJIT_PHYS_REG(X86Gp  , rax  , gpq[0]);    //!< 64-bit GPQ register (X64).
ASMJIT_PHYS_REG(X86Gp  , rcx  , gpq[1]);    //!< 64-bit GPQ register (X64).
ASMJIT_PHYS_REG(X86Gp  , rdx  , gpq[2]);    //!< 64-bit GPQ register (X64).
ASMJIT_PHYS_REG(X86Gp  , rbx  , gpq[3]);    //!< 64-bit GPQ register (X64).
ASMJIT_PHYS_REG(X86Gp  , rsp  , gpq[4]);    //!< 64-bit GPQ register (X64).
ASMJIT_PHYS_REG(X86Gp  , rbp  , gpq[5]);    //!< 64-bit GPQ register (X64).
ASMJIT_PHYS_REG(X86Gp  , rsi  , gpq[6]);    //!< 64-bit GPQ register (X64).
ASMJIT_PHYS_REG(X86Gp  , rdi  , gpq[7]);    //!< 64-bit GPQ register (X64).
ASMJIT_PHYS_REG(X86Gp  , r8   , gpq[8]);    //!< 64-bit GPQ register (X64).
ASMJIT_PHYS_REG(X86Gp  , r9   , gpq[9]);    //!< 64-bit GPQ register (X64).
ASMJIT_PHYS_REG(X86Gp  , r10  , gpq[10]);   //!< 64-bit GPQ register (X64).
ASMJIT_PHYS_REG(X86Gp  , r11  , gpq[11]);   //!< 64-bit GPQ register (X64).
ASMJIT_PHYS_REG(X86Gp  , r12  , gpq[12]);   //!< 64-bit GPQ register (X64).
ASMJIT_PHYS_REG(X86Gp  , r13  , gpq[13]);   //!< 64-bit GPQ register (X64).
ASMJIT_PHYS_REG(X86Gp  , r14  , gpq[14]);   //!< 64-bit GPQ register (X64).
ASMJIT_PHYS_REG(X86Gp  , r15  , gpq[15]);   //!< 64-bit GPQ register (X64).

ASMJIT_PHYS_REG(X86Fp  , fp0  , fp[0]);     //!< 80-bit FPU register.
ASMJIT_PHYS_REG(X86Fp  , fp1  , fp[1]);     //!< 80-bit FPU register.
ASMJIT_PHYS_REG(X86Fp  , fp2  , fp[2]);     //!< 80-bit FPU register.
ASMJIT_PHYS_REG(X86Fp  , fp3  , fp[3]);     //!< 80-bit FPU register.
ASMJIT_PHYS_REG(X86Fp  , fp4  , fp[4]);     //!< 80-bit FPU register.
ASMJIT_PHYS_REG(X86Fp  , fp5  , fp[5]);     //!< 80-bit FPU register.
ASMJIT_PHYS_REG(X86Fp  , fp6  , fp[6]);     //!< 80-bit FPU register.
ASMJIT_PHYS_REG(X86Fp  , fp7  , fp[7]);     //!< 80-bit FPU register.

ASMJIT_PHYS_REG(X86Mm  , mm0  , mm[0]);     //!< 64-bit MMX register.
ASMJIT_PHYS_REG(X86Mm  , mm1  , mm[1]);     //!< 64-bit MMX register.
ASMJIT_PHYS_REG(X86Mm  , mm2  , mm[2]);     //!< 64-bit MMX register.
ASMJIT_PHYS_REG(X86Mm  , mm3  , mm[3]);     //!< 64-bit MMX register.
ASMJIT_PHYS_REG(X86Mm  , mm4  , mm[4]);     //!< 64-bit MMX register.
ASMJIT_PHYS_REG(X86Mm  , mm5  , mm[5]);     //!< 64-bit MMX register.
ASMJIT_PHYS_REG(X86Mm  , mm6  , mm[6]);     //!< 64-bit MMX register.
ASMJIT_PHYS_REG(X86Mm  , mm7  , mm[7]);     //!< 64-bit MMX register.

ASMJIT_PHYS_REG(X86KReg, k0   , k[0]);      //!< 64-bit K register.
ASMJIT_PHYS_REG(X86KReg, k1   , k[1]);      //!< 64-bit K register.
ASMJIT_PHYS_REG(X86KReg, k2   , k[2]);      //!< 64-bit K register.
ASMJIT_PHYS_REG(X86KReg, k3   , k[3]);      //!< 64-bit K register.
ASMJIT_PHYS_REG(X86KReg, k4   , k[4]);      //!< 64-bit K register.
ASMJIT_PHYS_REG(X86KReg, k5   , k[5]);      //!< 64-bit K register.
ASMJIT_PHYS_REG(X86KReg, k6   , k[6]);      //!< 64-bit K register.
ASMJIT_PHYS_REG(X86KReg, k7   , k[7]);      //!< 64-bit K register.

ASMJIT_PHYS_REG(X86Xmm , xmm0 , xmm[0]);    //!< 128-bit XMM register.
ASMJIT_PHYS_REG(X86Xmm , xmm1 , xmm[1]);    //!< 128-bit XMM register.
ASMJIT_PHYS_REG(X86Xmm , xmm2 , xmm[2]);    //!< 128-bit XMM register.
ASMJIT_PHYS_REG(X86Xmm , xmm3 , xmm[3]);    //!< 128-bit XMM register.
ASMJIT_PHYS_REG(X86Xmm , xmm4 , xmm[4]);    //!< 128-bit XMM register.
ASMJIT_PHYS_REG(X86Xmm , xmm5 , xmm[5]);    //!< 128-bit XMM register.
ASMJIT_PHYS_REG(X86Xmm , xmm6 , xmm[6]);    //!< 128-bit XMM register.
ASMJIT_PHYS_REG(X86Xmm , xmm7 , xmm[7]);    //!< 128-bit XMM register.
ASMJIT_PHYS_REG(X86Xmm , xmm8 , xmm[8]);    //!< 128-bit XMM register (X64).
ASMJIT_PHYS_REG(X86Xmm , xmm9 , xmm[9]);    //!< 128-bit XMM register (X64).
ASMJIT_PHYS_REG(X86Xmm , xmm10, xmm[10]);   //!< 128-bit XMM register (X64).
ASMJIT_PHYS_REG(X86Xmm , xmm11, xmm[11]);   //!< 128-bit XMM register (X64).
ASMJIT_PHYS_REG(X86Xmm , xmm12, xmm[12]);   //!< 128-bit XMM register (X64).
ASMJIT_PHYS_REG(X86Xmm , xmm13, xmm[13]);   //!< 128-bit XMM register (X64).
ASMJIT_PHYS_REG(X86Xmm , xmm14, xmm[14]);   //!< 128-bit XMM register (X64).
ASMJIT_PHYS_REG(X86Xmm , xmm15, xmm[15]);   //!< 128-bit XMM register (X64).
ASMJIT_PHYS_REG(X86Xmm , xmm16, xmm[16]);   //!< 128-bit XMM register (X64 & AVX512VL+).
ASMJIT_PHYS_REG(X86Xmm , xmm17, xmm[17]);   //!< 128-bit XMM register (X64 & AVX512VL+).
ASMJIT_PHYS_REG(X86Xmm , xmm18, xmm[18]);   //!< 128-bit XMM register (X64 & AVX512VL+).
ASMJIT_PHYS_REG(X86Xmm , xmm19, xmm[19]);   //!< 128-bit XMM register (X64 & AVX512VL+).
ASMJIT_PHYS_REG(X86Xmm , xmm20, xmm[20]);   //!< 128-bit XMM register (X64 & AVX512VL+).
ASMJIT_PHYS_REG(X86Xmm , xmm21, xmm[21]);   //!< 128-bit XMM register (X64 & AVX512VL+).
ASMJIT_PHYS_REG(X86Xmm , xmm22, xmm[22]);   //!< 128-bit XMM register (X64 & AVX512VL+).
ASMJIT_PHYS_REG(X86Xmm , xmm23, xmm[23]);   //!< 128-bit XMM register (X64 & AVX512VL+).
ASMJIT_PHYS_REG(X86Xmm , xmm24, xmm[24]);   //!< 128-bit XMM register (X64 & AVX512VL+).
ASMJIT_PHYS_REG(X86Xmm , xmm25, xmm[25]);   //!< 128-bit XMM register (X64 & AVX512VL+).
ASMJIT_PHYS_REG(X86Xmm , xmm26, xmm[26]);   //!< 128-bit XMM register (X64 & AVX512VL+).
ASMJIT_PHYS_REG(X86Xmm , xmm27, xmm[27]);   //!< 128-bit XMM register (X64 & AVX512VL+).
ASMJIT_PHYS_REG(X86Xmm , xmm28, xmm[28]);   //!< 128-bit XMM register (X64 & AVX512VL+).
ASMJIT_PHYS_REG(X86Xmm , xmm29, xmm[29]);   //!< 128-bit XMM register (X64 & AVX512VL+).
ASMJIT_PHYS_REG(X86Xmm , xmm30, xmm[30]);   //!< 128-bit XMM register (X64 & AVX512VL+).
ASMJIT_PHYS_REG(X86Xmm , xmm31, xmm[31]);   //!< 128-bit XMM register (X64 & AVX512VL+).

ASMJIT_PHYS_REG(X86Ymm , ymm0 , ymm[0]);    //!< 256-bit YMM register.
ASMJIT_PHYS_REG(X86Ymm , ymm1 , ymm[1]);    //!< 256-bit YMM register.
ASMJIT_PHYS_REG(X86Ymm , ymm2 , ymm[2]);    //!< 256-bit YMM register.
ASMJIT_PHYS_REG(X86Ymm , ymm3 , ymm[3]);    //!< 256-bit YMM register.
ASMJIT_PHYS_REG(X86Ymm , ymm4 , ymm[4]);    //!< 256-bit YMM register.
ASMJIT_PHYS_REG(X86Ymm , ymm5 , ymm[5]);    //!< 256-bit YMM register.
ASMJIT_PHYS_REG(X86Ymm , ymm6 , ymm[6]);    //!< 256-bit YMM register.
ASMJIT_PHYS_REG(X86Ymm , ymm7 , ymm[7]);    //!< 256-bit YMM register.
ASMJIT_PHYS_REG(X86Ymm , ymm8 , ymm[8]);    //!< 256-bit YMM register (X64).
ASMJIT_PHYS_REG(X86Ymm , ymm9 , ymm[9]);    //!< 256-bit YMM register (X64).
ASMJIT_PHYS_REG(X86Ymm , ymm10, ymm[10]);   //!< 256-bit YMM register (X64).
ASMJIT_PHYS_REG(X86Ymm , ymm11, ymm[11]);   //!< 256-bit YMM register (X64).
ASMJIT_PHYS_REG(X86Ymm , ymm12, ymm[12]);   //!< 256-bit YMM register (X64).
ASMJIT_PHYS_REG(X86Ymm , ymm13, ymm[13]);   //!< 256-bit YMM register (X64).
ASMJIT_PHYS_REG(X86Ymm , ymm14, ymm[14]);   //!< 256-bit YMM register (X64).
ASMJIT_PHYS_REG(X86Ymm , ymm15, ymm[15]);   //!< 256-bit YMM register (X64).
ASMJIT_PHYS_REG(X86Ymm , ymm16, ymm[16]);   //!< 256-bit YMM register (X64 & AVX512VL+).
ASMJIT_PHYS_REG(X86Ymm , ymm17, ymm[17]);   //!< 256-bit YMM register (X64 & AVX512VL+).
ASMJIT_PHYS_REG(X86Ymm , ymm18, ymm[18]);   //!< 256-bit YMM register (X64 & AVX512VL+).
ASMJIT_PHYS_REG(X86Ymm , ymm19, ymm[19]);   //!< 256-bit YMM register (X64 & AVX512VL+).
ASMJIT_PHYS_REG(X86Ymm , ymm20, ymm[20]);   //!< 256-bit YMM register (X64 & AVX512VL+).
ASMJIT_PHYS_REG(X86Ymm , ymm21, ymm[21]);   //!< 256-bit YMM register (X64 & AVX512VL+).
ASMJIT_PHYS_REG(X86Ymm , ymm22, ymm[22]);   //!< 256-bit YMM register (X64 & AVX512VL+).
ASMJIT_PHYS_REG(X86Ymm , ymm23, ymm[23]);   //!< 256-bit YMM register (X64 & AVX512VL+).
ASMJIT_PHYS_REG(X86Ymm , ymm24, ymm[24]);   //!< 256-bit YMM register (X64 & AVX512VL+).
ASMJIT_PHYS_REG(X86Ymm , ymm25, ymm[25]);   //!< 256-bit YMM register (X64 & AVX512VL+).
ASMJIT_PHYS_REG(X86Ymm , ymm26, ymm[26]);   //!< 256-bit YMM register (X64 & AVX512VL+).
ASMJIT_PHYS_REG(X86Ymm , ymm27, ymm[27]);   //!< 256-bit YMM register (X64 & AVX512VL+).
ASMJIT_PHYS_REG(X86Ymm , ymm28, ymm[28]);   //!< 256-bit YMM register (X64 & AVX512VL+).
ASMJIT_PHYS_REG(X86Ymm , ymm29, ymm[29]);   //!< 256-bit YMM register (X64 & AVX512VL+).
ASMJIT_PHYS_REG(X86Ymm , ymm30, ymm[30]);   //!< 256-bit YMM register (X64 & AVX512VL+).
ASMJIT_PHYS_REG(X86Ymm , ymm31, ymm[31]);   //!< 256-bit YMM register (X64 & AVX512VL+).

ASMJIT_PHYS_REG(X86Zmm , zmm0 , zmm[0]);    //!< 512-bit ZMM register.
ASMJIT_PHYS_REG(X86Zmm , zmm1 , zmm[1]);    //!< 512-bit ZMM register.
ASMJIT_PHYS_REG(X86Zmm , zmm2 , zmm[2]);    //!< 512-bit ZMM register.
ASMJIT_PHYS_REG(X86Zmm , zmm3 , zmm[3]);    //!< 512-bit ZMM register.
ASMJIT_PHYS_REG(X86Zmm , zmm4 , zmm[4]);    //!< 512-bit ZMM register.
ASMJIT_PHYS_REG(X86Zmm , zmm5 , zmm[5]);    //!< 512-bit ZMM register.
ASMJIT_PHYS_REG(X86Zmm , zmm6 , zmm[6]);    //!< 512-bit ZMM register.
ASMJIT_PHYS_REG(X86Zmm , zmm7 , zmm[7]);    //!< 512-bit ZMM register.
ASMJIT_PHYS_REG(X86Zmm , zmm8 , zmm[8]);    //!< 512-bit ZMM register (X64).
ASMJIT_PHYS_REG(X86Zmm , zmm9 , zmm[9]);    //!< 512-bit ZMM register (X64).
ASMJIT_PHYS_REG(X86Zmm , zmm10, zmm[10]);   //!< 512-bit ZMM register (X64).
ASMJIT_PHYS_REG(X86Zmm , zmm11, zmm[11]);   //!< 512-bit ZMM register (X64).
ASMJIT_PHYS_REG(X86Zmm , zmm12, zmm[12]);   //!< 512-bit ZMM register (X64).
ASMJIT_PHYS_REG(X86Zmm , zmm13, zmm[13]);   //!< 512-bit ZMM register (X64).
ASMJIT_PHYS_REG(X86Zmm , zmm14, zmm[14]);   //!< 512-bit ZMM register (X64).
ASMJIT_PHYS_REG(X86Zmm , zmm15, zmm[15]);   //!< 512-bit ZMM register (X64).
ASMJIT_PHYS_REG(X86Zmm , zmm16, zmm[16]);   //!< 512-bit ZMM register (X64 & AVX512F+).
ASMJIT_PHYS_REG(X86Zmm , zmm17, zmm[17]);   //!< 512-bit ZMM register (X64 & AVX512F+).
ASMJIT_PHYS_REG(X86Zmm , zmm18, zmm[18]);   //!< 512-bit ZMM register (X64 & AVX512F+).
ASMJIT_PHYS_REG(X86Zmm , zmm19, zmm[19]);   //!< 512-bit ZMM register (X64 & AVX512F+).
ASMJIT_PHYS_REG(X86Zmm , zmm20, zmm[20]);   //!< 512-bit ZMM register (X64 & AVX512F+).
ASMJIT_PHYS_REG(X86Zmm , zmm21, zmm[21]);   //!< 512-bit ZMM register (X64 & AVX512F+).
ASMJIT_PHYS_REG(X86Zmm , zmm22, zmm[22]);   //!< 512-bit ZMM register (X64 & AVX512F+).
ASMJIT_PHYS_REG(X86Zmm , zmm23, zmm[23]);   //!< 512-bit ZMM register (X64 & AVX512F+).
ASMJIT_PHYS_REG(X86Zmm , zmm24, zmm[24]);   //!< 512-bit ZMM register (X64 & AVX512F+).
ASMJIT_PHYS_REG(X86Zmm , zmm25, zmm[25]);   //!< 512-bit ZMM register (X64 & AVX512F+).
ASMJIT_PHYS_REG(X86Zmm , zmm26, zmm[26]);   //!< 512-bit ZMM register (X64 & AVX512F+).
ASMJIT_PHYS_REG(X86Zmm , zmm27, zmm[27]);   //!< 512-bit ZMM register (X64 & AVX512F+).
ASMJIT_PHYS_REG(X86Zmm , zmm28, zmm[28]);   //!< 512-bit ZMM register (X64 & AVX512F+).
ASMJIT_PHYS_REG(X86Zmm , zmm29, zmm[29]);   //!< 512-bit ZMM register (X64 & AVX512F+).
ASMJIT_PHYS_REG(X86Zmm , zmm30, zmm[30]);   //!< 512-bit ZMM register (X64 & AVX512F+).
ASMJIT_PHYS_REG(X86Zmm , zmm31, zmm[31]);   //!< 512-bit ZMM register (X64 & AVX512F+).

ASMJIT_PHYS_REG(X86Bnd , bnd0 , bnd[0]);    //!< 128-bit bound register.
ASMJIT_PHYS_REG(X86Bnd , bnd1 , bnd[1]);    //!< 128-bit bound register.
ASMJIT_PHYS_REG(X86Bnd , bnd2 , bnd[2]);    //!< 128-bit bound register.
ASMJIT_PHYS_REG(X86Bnd , bnd3 , bnd[3]);    //!< 128-bit bound register.

ASMJIT_PHYS_REG(X86CReg, cr0  , cr[0]);     //!< 32-bit or 64-bit control register.
ASMJIT_PHYS_REG(X86CReg, cr1  , cr[1]);     //!< 32-bit or 64-bit control register.
ASMJIT_PHYS_REG(X86CReg, cr2  , cr[2]);     //!< 32-bit or 64-bit control register.
ASMJIT_PHYS_REG(X86CReg, cr3  , cr[3]);     //!< 32-bit or 64-bit control register.
ASMJIT_PHYS_REG(X86CReg, cr4  , cr[4]);     //!< 32-bit or 64-bit control register.
ASMJIT_PHYS_REG(X86CReg, cr5  , cr[5]);     //!< 32-bit or 64-bit control register.
ASMJIT_PHYS_REG(X86CReg, cr6  , cr[6]);     //!< 32-bit or 64-bit control register.
ASMJIT_PHYS_REG(X86CReg, cr7  , cr[7]);     //!< 32-bit or 64-bit control register.
ASMJIT_PHYS_REG(X86CReg, cr8  , cr[8]);     //!< 32-bit or 64-bit control register.
ASMJIT_PHYS_REG(X86CReg, cr9  , cr[9]);     //!< 32-bit or 64-bit control register.
ASMJIT_PHYS_REG(X86CReg, cr10 , cr[10]);    //!< 32-bit or 64-bit control register.
ASMJIT_PHYS_REG(X86CReg, cr11 , cr[11]);    //!< 32-bit or 64-bit control register.
ASMJIT_PHYS_REG(X86CReg, cr12 , cr[12]);    //!< 32-bit or 64-bit control register.
ASMJIT_PHYS_REG(X86CReg, cr13 , cr[13]);    //!< 32-bit or 64-bit control register.
ASMJIT_PHYS_REG(X86CReg, cr14 , cr[14]);    //!< 32-bit or 64-bit control register.
ASMJIT_PHYS_REG(X86CReg, cr15 , cr[15]);    //!< 32-bit or 64-bit control register.

ASMJIT_PHYS_REG(X86DReg, dr0  , dr[0]);     //!< 32-bit or 64-bit debug register.
ASMJIT_PHYS_REG(X86DReg, dr1  , dr[1]);     //!< 32-bit or 64-bit debug register.
ASMJIT_PHYS_REG(X86DReg, dr2  , dr[2]);     //!< 32-bit or 64-bit debug register.
ASMJIT_PHYS_REG(X86DReg, dr3  , dr[3]);     //!< 32-bit or 64-bit debug register.
ASMJIT_PHYS_REG(X86DReg, dr4  , dr[4]);     //!< 32-bit or 64-bit debug register.
ASMJIT_PHYS_REG(X86DReg, dr5  , dr[5]);     //!< 32-bit or 64-bit debug register.
ASMJIT_PHYS_REG(X86DReg, dr6  , dr[6]);     //!< 32-bit or 64-bit debug register.
ASMJIT_PHYS_REG(X86DReg, dr7  , dr[7]);     //!< 32-bit or 64-bit debug register.
ASMJIT_PHYS_REG(X86DReg, dr8  , dr[8]);     //!< 32-bit or 64-bit debug register.
ASMJIT_PHYS_REG(X86DReg, dr9  , dr[9]);     //!< 32-bit or 64-bit debug register.
ASMJIT_PHYS_REG(X86DReg, dr10 , dr[10]);    //!< 32-bit or 64-bit debug register.
ASMJIT_PHYS_REG(X86DReg, dr11 , dr[11]);    //!< 32-bit or 64-bit debug register.
ASMJIT_PHYS_REG(X86DReg, dr12 , dr[12]);    //!< 32-bit or 64-bit debug register.
ASMJIT_PHYS_REG(X86DReg, dr13 , dr[13]);    //!< 32-bit or 64-bit debug register.
ASMJIT_PHYS_REG(X86DReg, dr14 , dr[14]);    //!< 32-bit or 64-bit debug register.
ASMJIT_PHYS_REG(X86DReg, dr15 , dr[15]);    //!< 32-bit or 64-bit debug register.

#undef ASMJIT_PHYS_REG
} // anonymous namespace
#endif // !ASMJIT_X86_OPERAND_EXPORTS

//! Create an 8-bit low GPB register operand.
static ASMJIT_INLINE X86GpbLo gpb(uint32_t id) noexcept { return X86GpbLo(id); }
//! Create an 8-bit low GPB register operand.
static ASMJIT_INLINE X86GpbLo gpb_lo(uint32_t id) noexcept { return X86GpbLo(id); }
//! Create an 8-bit high GPB register operand.
static ASMJIT_INLINE X86GpbHi gpb_hi(uint32_t id) noexcept { return X86GpbHi(id); }
//! Create a 16-bit GPW register operand.
static ASMJIT_INLINE X86Gpw gpw(uint32_t id) noexcept { return X86Gpw(id); }
//! Create a 32-bit GPD register operand.
static ASMJIT_INLINE X86Gpd gpd(uint32_t id) noexcept { return X86Gpd(id); }
//! Create a 64-bit GPQ register operand (X64).
static ASMJIT_INLINE X86Gpq gpq(uint32_t id) noexcept { return X86Gpq(id); }
//! Create an 80-bit Fp register operand.
static ASMJIT_INLINE X86Fp fp(uint32_t id) noexcept { return X86Fp(id); }
//! Create a 64-bit Mm register operand.
static ASMJIT_INLINE X86Mm mm(uint32_t id) noexcept { return X86Mm(id); }
//! Create a 64-bit K register operand.
static ASMJIT_INLINE X86KReg k(uint32_t id) noexcept { return X86KReg(id); }
//! Create a 128-bit XMM register operand.
static ASMJIT_INLINE X86Xmm xmm(uint32_t id) noexcept { return X86Xmm(id); }
//! Create a 256-bit YMM register operand.
static ASMJIT_INLINE X86Ymm ymm(uint32_t id) noexcept { return X86Ymm(id); }
//! Create a 512-bit ZMM register operand.
static ASMJIT_INLINE X86Zmm zmm(uint32_t id) noexcept { return X86Zmm(id); }
//! Create a 128-bit bound register operand.
static ASMJIT_INLINE X86Bnd bnd(uint32_t id) noexcept { return X86Bnd(id); }
//! Create a 32-bit or 64-bit control register operand.
static ASMJIT_INLINE X86CReg cr(uint32_t id) noexcept { return X86CReg(id); }
//! Create a 32-bit or 64-bit debug register operand.
static ASMJIT_INLINE X86DReg dr(uint32_t id) noexcept { return X86DReg(id); }

// ============================================================================
// [asmjit::x86 - Ptr (Reg)]
// ============================================================================

//! Create a `[base.reg + offset]` memory operand.
static ASMJIT_INLINE X86Mem ptr(const X86Gp& base, int32_t offset = 0, uint32_t size = 0) noexcept {
  return X86Mem(base, offset, size);
}
//! Create a `[base.reg + (index << shift) + offset]` memory operand (scalar index).
static ASMJIT_INLINE X86Mem ptr(const X86Gp& base, const X86Gp& index, uint32_t shift = 0, int32_t offset = 0, uint32_t size = 0) noexcept {
  return X86Mem(base, index, shift, offset, size);
}
//! Create a `[base.reg + (index << shift) + offset]` memory operand (vector index).
static ASMJIT_INLINE X86Mem ptr(const X86Gp& base, const X86Vec& index, uint32_t shift = 0, int32_t offset = 0, uint32_t size = 0) noexcept {
  return X86Mem(base, index, shift, offset, size);
}

//! Create a `[base + offset]` memory operand.
static ASMJIT_INLINE X86Mem ptr(const Label& base, int32_t offset = 0, uint32_t size = 0) noexcept {
  return X86Mem(base, offset, size);
}
//! Create a `[base + (index << shift) + offset]` memory operand.
static ASMJIT_INLINE X86Mem ptr(const Label& base, const X86Gp& index, uint32_t shift, int32_t offset = 0, uint32_t size = 0) noexcept {
  return X86Mem(base, index, shift, offset, size);
}
//! Create a `[base + (index << shift) + offset]` memory operand.
static ASMJIT_INLINE X86Mem ptr(const Label& base, const X86Vec& index, uint32_t shift, int32_t offset = 0, uint32_t size = 0) noexcept {
  return X86Mem(base, index, shift, offset, size);
}

//! Create `[rip + offset]` memory operand.
static ASMJIT_INLINE X86Mem ptr(const X86Rip& rip_, int32_t offset = 0, uint32_t size = 0) noexcept {
  return X86Mem(rip_, offset, size);
}

//! Create an `[base]` absolute memory operand.
static ASMJIT_INLINE X86Mem ptr(uint64_t base, uint32_t size = 0) noexcept {
  return X86Mem(base, size);
}
//! Create an `[abs + (index.reg << shift)]` absolute memory operand.
static ASMJIT_INLINE X86Mem ptr(uint64_t base, const X86Reg& index, uint32_t shift = 0, uint32_t size = 0) noexcept {
  return X86Mem(base, index, shift, size);
}
//! Create an `[abs + (index.reg << shift)]` absolute memory operand.
static ASMJIT_INLINE X86Mem ptr(uint64_t base, const X86Vec& index, uint32_t shift = 0, uint32_t size = 0) noexcept {
  return X86Mem(base, index, shift, size);
}

//! \internal
#define ASMJIT_DEFINE_PTR_FUNC(FUNC, SIZE)                                            \
  /*! Create a `[base + offset]` memory operand. */                                   \
  static ASMJIT_INLINE X86Mem FUNC(const X86Gp& base, int32_t offset = 0) noexcept {  \
    return X86Mem(base, offset, SIZE);                                                \
  }                                                                                   \
  /*! Create a `[base + (index << shift) + offset]` memory operand. */                \
  static ASMJIT_INLINE X86Mem FUNC(const X86Gp& base, const X86Gp& index, uint32_t shift = 0, int32_t offset = 0) noexcept { \
    return X86Mem(base, index, shift, offset, SIZE);                                  \
  }                                                                                   \
  /*! Create a `[base + (vec_index << shift) + offset]` memory operand. */            \
  static ASMJIT_INLINE X86Mem FUNC(const X86Gp& base, const X86Vec& index, uint32_t shift = 0, int32_t offset = 0) noexcept { \
    return X86Mem(base, index, shift, offset, SIZE);                                  \
  }                                                                                   \
  /*! Create a `[base + offset]` memory operand. */                                   \
  static ASMJIT_INLINE X86Mem FUNC(const Label& base, int32_t offset = 0) noexcept {  \
    return X86Mem(base, offset, SIZE);                                                \
  }                                                                                   \
  /*! Create a `[base + (index << shift) + offset]` memory operand. */                \
  static ASMJIT_INLINE X86Mem FUNC(const Label& base, const X86Gp& index, uint32_t shift, int32_t offset = 0) noexcept { \
    return X86Mem(base, index, shift, offset, SIZE);                                  \
  }                                                                                   \
  /*! Create a `[rip + offset]` memory operand. */                                    \
  static ASMJIT_INLINE X86Mem FUNC(const X86Rip& rip_, int32_t offset = 0) noexcept { \
    return X86Mem(rip_, offset, SIZE);                                                \
  }                                                                                   \
  /*! Create a `[base + offset]` memory operand. */                                   \
  static ASMJIT_INLINE X86Mem FUNC(uint64_t base) noexcept {                          \
    return X86Mem(base, SIZE);                                                        \
  }                                                                                   \
  /*! Create a `[base + (index << shift) + offset]` memory operand. */                \
  static ASMJIT_INLINE X86Mem FUNC(uint64_t base, const X86Gp& index, uint32_t shift = 0) noexcept { \
    return X86Mem(base, index, shift, SIZE);                                          \
  }                                                                                   \
  /*! Create a `[base + (vec_index << shift) + offset]` memory operand. */            \
  static ASMJIT_INLINE X86Mem FUNC(uint64_t base, const X86Vec& index, uint32_t shift = 0) noexcept { \
    return X86Mem(base, index, shift, SIZE, Mem::kFlagAbs);                           \
  }                                                                                   \
  /*! Create a `[base + offset]` memory operand. */                                   \
  static ASMJIT_INLINE X86Mem FUNC##_abs(uint64_t base) noexcept {                    \
    return X86Mem(base, SIZE);                                                        \
  }                                                                                   \
  /*! Create a `[base + (index << shift) + offset]` memory operand. */                \
  static ASMJIT_INLINE X86Mem FUNC##_abs(uint64_t base, const X86Gp& index, uint32_t shift = 0) noexcept { \
    return X86Mem(base, index, shift, SIZE, Mem::kFlagAbs);                           \
  }                                                                                   \
  /*! Create a `[base + (vec_index << shift) + offset]` memory operand. */            \
  static ASMJIT_INLINE X86Mem FUNC##_abs(uint64_t base, const X86Vec& index, uint32_t shift = 0) noexcept { \
    return X86Mem(base, index, shift, SIZE, Mem::kFlagAbs);                           \
  }

// Define memory operand constructors that use platform independent naming.
ASMJIT_DEFINE_PTR_FUNC(ptr8, 1)
ASMJIT_DEFINE_PTR_FUNC(ptr16, 2)
ASMJIT_DEFINE_PTR_FUNC(ptr32, 4)
ASMJIT_DEFINE_PTR_FUNC(ptr64, 8)
ASMJIT_DEFINE_PTR_FUNC(ptr80, 10)
ASMJIT_DEFINE_PTR_FUNC(ptr128, 16)
ASMJIT_DEFINE_PTR_FUNC(ptr256, 32)
ASMJIT_DEFINE_PTR_FUNC(ptr512, 64)

// Define memory operand constructors that use X86/X64 specific naming.
ASMJIT_DEFINE_PTR_FUNC(byte_ptr, 1)
ASMJIT_DEFINE_PTR_FUNC(word_ptr, 2)
ASMJIT_DEFINE_PTR_FUNC(dword_ptr, 4)
ASMJIT_DEFINE_PTR_FUNC(qword_ptr, 8)
ASMJIT_DEFINE_PTR_FUNC(tword_ptr, 10)
ASMJIT_DEFINE_PTR_FUNC(oword_ptr, 16)
ASMJIT_DEFINE_PTR_FUNC(dqword_ptr, 16)
ASMJIT_DEFINE_PTR_FUNC(yword_ptr, 32)
ASMJIT_DEFINE_PTR_FUNC(zword_ptr, 64)

#undef ASMJIT_DEFINE_PTR_FUNC

} // x86 namespace

//! \}

} // asmjit namespace

// [Api-End]
#include "../asmjit_apiend.h"

// [Guard]
#endif // _ASMJIT_X86_X86OPERAND_H
