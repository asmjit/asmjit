// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_X86_X86OPERAND_H
#define _ASMJIT_X86_X86OPERAND_H

// [Dependencies - AsmJit]
#include "../base/assembler.h"
#include "../base/compiler.h"
#include "../base/globals.h"
#include "../base/intutil.h"
#include "../base/operand.h"
#include "../base/vectypes.h"

// [Api-Begin]
#include "../apibegin.h"

//! \internal
//!
//! Internal macro to get an operand ID casting it to `Operand`. Basically
//! allows to get an id of operand that has been just 'typedef'ed.
#define _OP_ID(_Op_) reinterpret_cast<const Operand&>(_Op_).getId()

namespace asmjit {

// ============================================================================
// [Forward Declarations]
// ============================================================================

struct X86Reg;
struct X86GpReg;
struct X86FpReg;
struct X86MmReg;
struct X86XmmReg;
struct X86YmmReg;
struct X86SegReg;

#if !defined(ASMJIT_DISABLE_COMPILER)
struct X86Var;
struct X86GpVar;
struct X86MmVar;
struct X86XmmVar;
struct X86YmmVar;
#endif // !ASMJIT_DISABLE_COMPILER

//! \addtogroup asmjit_x86_general
//! \{

// ============================================================================
// [asmjit::kX86RegClass]
// ============================================================================

//! X86/X64 variable class.
ASMJIT_ENUM(kX86RegClass) {
  //! X86/X64 Gp register class (compatible with universal \ref kRegClassGp).
  kX86RegClassGp = kRegClassGp,
  //! X86/X64 Fp register class.
  kX86RegClassFp = 1,
  //! X86/X64 Mm register class.
  kX86RegClassMm = 2,
  //! X86/X64 Xmm/Ymm/Zmm register class.
  kX86RegClassXyz = 3,

  //! Count of X86/X64 register classes.
  kX86RegClassCount = 4
};

// ============================================================================
// [asmjit::kX86RegType]
// ============================================================================

//! X86/X64 register type.
ASMJIT_ENUM(kX86RegType) {
  //! Gpb-lo register (AL, BL, CL, DL, ...).
  kX86RegTypeGpbLo = 0x01,
  //! Gpb-hi register (AH, BH, CH, DH only).
  kX86RegTypeGpbHi = 0x02,

  //! \internal
  //!
  //! Gpb-hi register patched to native index (4-7).
  _kX86RegTypePatchedGpbHi = kX86RegTypeGpbLo | kX86RegTypeGpbHi,

  //! Gpw register.
  kX86RegTypeGpw = 0x10,
  //! Gpd register.
  kX86RegTypeGpd = 0x20,
  //! Gpq register.
  kX86RegTypeGpq = 0x30,

  //! Fp register.
  kX86RegTypeFp = 0x50,
  //! Mm register.
  kX86RegTypeMm = 0x60,

  //! Xmm register.
  kX86RegTypeXmm = 0x70,
  //! Ymm register.
  kX86RegTypeYmm = 0x80,
  //! Zmm register.
  kX86RegTypeZmm = 0x90,

  //! Segment register.
  kX86RegTypeSeg = 0xF0
};

// ============================================================================
// [asmjit::kX86RegIndex]
// ============================================================================

//! X86/X64 register indexes.
//!
//! \note Register indexes have been reduced to only support general purpose
//! registers. There is no need to have enumerations with number suffix that
//! expands to the exactly same value as the suffix value itself.
ASMJIT_ENUM(kX86RegIndex) {
  //! Index of Al/Ah/Ax/Eax/Rax registers.
  kX86RegIndexAx = 0,
  //! Index of Cl/Ch/Cx/Ecx/Rcx registers.
  kX86RegIndexCx = 1,
  //! Index of Dl/Dh/Dx/Edx/Rdx registers.
  kX86RegIndexDx = 2,
  //! Index of Bl/Bh/Bx/Ebx/Rbx registers.
  kX86RegIndexBx = 3,
  //! Index of Spl/Sp/Esp/Rsp registers.
  kX86RegIndexSp = 4,
  //! Index of Bpl/Bp/Ebp/Rbp registers.
  kX86RegIndexBp = 5,
  //! Index of Sil/Si/Esi/Rsi registers.
  kX86RegIndexSi = 6,
  //! Index of Dil/Di/Edi/Rdi registers.
  kX86RegIndexDi = 7,
  //! Index of R8b/R8w/R8d/R8 registers (64-bit only).
  kX86RegIndexR8 = 8,
  //! Index of R9B/R9w/R9d/R9 registers (64-bit only).
  kX86RegIndexR9 = 9,
  //! Index of R10B/R10w/R10D/R10 registers (64-bit only).
  kX86RegIndexR10 = 10,
  //! Index of R11B/R11w/R11d/R11 registers (64-bit only).
  kX86RegIndexR11 = 11,
  //! Index of R12B/R12w/R12d/R12 registers (64-bit only).
  kX86RegIndexR12 = 12,
  //! Index of R13B/R13w/R13d/R13 registers (64-bit only).
  kX86RegIndexR13 = 13,
  //! Index of R14B/R14w/R14d/R14 registers (64-bit only).
  kX86RegIndexR14 = 14,
  //! Index of R15B/R15w/R15d/R15 registers (64-bit only).
  kX86RegIndexR15 = 15
};

// ============================================================================
// [asmjit::kX86Seg]
// ============================================================================

//! X86/X64 segment codes.
ASMJIT_ENUM(kX86Seg) {
  //! No/Default segment.
  kX86SegDefault = 0,
  //! Es segment.
  kX86SegEs = 1,
  //! Cs segment.
  kX86SegCs = 2,
  //! Ss segment.
  kX86SegSs = 3,
  //! Ds segment.
  kX86SegDs = 4,
  //! Fs segment.
  kX86SegFs = 5,
  //! Gs segment.
  kX86SegGs = 6,

  //! Count of X86 segment registers supported by AsmJit.
  //!
  //! \note X86 architecture has 6 segment registers - ES, CS, SS, DS, FS, GS.
  //! X64 architecture lowers them down to just FS and GS. AsmJit supports 7
  //! segment registers - all addressable in both X86 and X64 modes and one
  //! extra called `kX86SegDefault`, which is AsmJit specific and means that there
  //! is no segment register specified so the segment prefix will not be emitted.
  kX86SegCount = 7
};

// ============================================================================
// [asmjit::kX86MemVSib]
// ============================================================================

//! X86/X64 index register legacy and AVX2 (VSIB) support.
ASMJIT_ENUM(kX86MemVSib) {
  //! Memory operand uses Gp or no index register.
  kX86MemVSibGpz = 0,
  //! Memory operand uses Xmm or no index register.
  kX86MemVSibXmm = 1,
  //! Memory operand uses Ymm or no index register.
  kX86MemVSibYmm = 2
};

// ============================================================================
// [asmjit::kX86MemFlags]
// ============================================================================

//! \internal
//!
//! X86/X64 specific memory flags.
ASMJIT_ENUM(kX86MemFlags) {
  kX86MemSegBits    = 0x7,
  kX86MemSegIndex   = 0,
  kX86MemSegMask    = kX86MemSegBits << kX86MemSegIndex,

  kX86MemGpdBits    = 0x1,
  kX86MemGpdIndex   = 3,
  kX86MemGpdMask    = kX86MemGpdBits << kX86MemGpdIndex,

  kX86MemVSibBits   = 0x3,
  kX86MemVSibIndex  = 4,
  kX86MemVSibMask   = kX86MemVSibBits << kX86MemVSibIndex,

  kX86MemShiftBits  = 0x3,
  kX86MemShiftIndex = 6,
  kX86MemShiftMask  = kX86MemShiftBits << kX86MemShiftIndex
};

// This is only defined by `x86operand_regs.cpp` when exporting registers.
#if !defined(ASMJIT_EXPORTS_X86OPERAND_REGS)

// ============================================================================
// [asmjit::X86RegCount]
// ============================================================================

//! \internal
//!
//! X86/X64 registers count (Gp, Fp, Mm, Xmm).
struct X86RegCount {
  // --------------------------------------------------------------------------
  // [Zero]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE void reset() {
    _packed = 0;
  }

  // --------------------------------------------------------------------------
  // [Get]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE uint32_t get(uint32_t c) const {
    ASMJIT_ASSERT(c < kX86RegClassCount);
    return _regs[c];
  }

  ASMJIT_INLINE uint32_t getGp() const { return _regs[kX86RegClassGp]; }
  ASMJIT_INLINE uint32_t getFp() const { return _regs[kX86RegClassFp]; }
  ASMJIT_INLINE uint32_t getMm() const { return _regs[kX86RegClassMm]; }
  ASMJIT_INLINE uint32_t getXyz() const { return _regs[kX86RegClassXyz]; }

  // --------------------------------------------------------------------------
  // [Set]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE void set(uint32_t c, uint32_t n) {
    ASMJIT_ASSERT(c < kX86RegClassCount);
    ASMJIT_ASSERT(n < 0x100);

    _regs[c] = static_cast<uint8_t>(n);
  }

  ASMJIT_INLINE void setGp(uint32_t n) { set(kX86RegClassGp, n); }
  ASMJIT_INLINE void setFp(uint32_t n) { set(kX86RegClassFp, n); }
  ASMJIT_INLINE void setMm(uint32_t n) { set(kX86RegClassMm, n); }
  ASMJIT_INLINE void setXyz(uint32_t n) { set(kX86RegClassXyz, n); }

  // --------------------------------------------------------------------------
  // [Add]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE void add(uint32_t c, uint32_t n = 1) {
    ASMJIT_ASSERT(c < kX86RegClassCount);
    ASMJIT_ASSERT(n < 0x100);

    _regs[c] += static_cast<uint8_t>(n);
  }

  ASMJIT_INLINE void addGp(uint32_t n) { add(kX86RegClassGp, n); }
  ASMJIT_INLINE void addFp(uint32_t n) { add(kX86RegClassFp, n); }
  ASMJIT_INLINE void addMm(uint32_t n) { add(kX86RegClassMm, n); }
  ASMJIT_INLINE void addXyz(uint32_t n) { add(kX86RegClassXyz, n); }

  // --------------------------------------------------------------------------
  // [Misc]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE void makeIndex(const X86RegCount& count) {
    uint8_t a = count._regs[0];
    uint8_t b = count._regs[1];
    uint8_t c = count._regs[2];

    _regs[0] = 0;
    _regs[1] = a;
    _regs[2] = a + b;
    _regs[3] = a + b + c;
  }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  union {
    struct {
      uint8_t _gp;
      uint8_t _fp;
      uint8_t _mm;
      uint8_t _xy;
    };

    uint8_t _regs[4];
    uint32_t _packed;
  };
};

// ============================================================================
// [asmjit::X86RegMask]
// ============================================================================

//! \internal
//!
//! X86/X64 registers mask (Gp, Fp, Mm, Xmm/Ymm/Zmm).
struct X86RegMask {
  // --------------------------------------------------------------------------
  // [Reset]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE void reset() {
    _packed.reset();
  }

  // --------------------------------------------------------------------------
  // [IsEmpty / Has]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE bool isEmpty() const {
    return _packed.isZero();
  }

  ASMJIT_INLINE bool has(uint32_t c, uint32_t mask = 0xFFFFFFFF) const {
    switch (c) {
      case kX86RegClassGp : return (static_cast<uint32_t>(_gp ) & mask) != 0;
      case kX86RegClassFp : return (static_cast<uint32_t>(_fp ) & mask) != 0;
      case kX86RegClassMm : return (static_cast<uint32_t>(_mm ) & mask) != 0;
      case kX86RegClassXyz: return (static_cast<uint32_t>(_xyz) & mask) != 0;
    }

    ASMJIT_ASSERT(!"Reached");
    return false;
  }

  // --------------------------------------------------------------------------
  // [Zero]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE void zero(uint32_t c) {
    switch (c) {
      case kX86RegClassGp : _gp  = 0; break;
      case kX86RegClassFp : _fp  = 0; break;
      case kX86RegClassMm : _mm  = 0; break;
      case kX86RegClassXyz: _xyz = 0; break;
    }
  }

  // --------------------------------------------------------------------------
  // [Get]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE uint32_t get(uint32_t c) const {
    switch (c) {
      case kX86RegClassGp : return _gp;
      case kX86RegClassFp : return _fp;
      case kX86RegClassMm : return _mm;
      case kX86RegClassXyz: return _xyz;
    }

    ASMJIT_ASSERT(!"Reached");
    return 0;
  }

  // --------------------------------------------------------------------------
  // [Set]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE void set(uint32_t c, uint32_t mask) {
    switch (c) {
      case kX86RegClassGp : _gp  = static_cast<uint16_t>(mask); break;
      case kX86RegClassFp : _fp  = static_cast<uint8_t >(mask); break;
      case kX86RegClassMm : _mm  = static_cast<uint8_t >(mask); break;
      case kX86RegClassXyz: _xyz = static_cast<uint32_t>(mask); break;
    }
  }

  ASMJIT_INLINE void set(const X86RegMask& other) {
    _packed.setUInt64(other._packed);
  }

  // --------------------------------------------------------------------------
  // [Add]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE void add(uint32_t c, uint32_t mask) {
    switch (c) {
      case kX86RegClassGp : _gp  |= static_cast<uint16_t>(mask); break;
      case kX86RegClassFp : _fp  |= static_cast<uint8_t >(mask); break;
      case kX86RegClassMm : _mm  |= static_cast<uint8_t >(mask); break;
      case kX86RegClassXyz: _xyz |= static_cast<uint32_t>(mask); break;
    }
  }

  ASMJIT_INLINE void add(const X86RegMask& other) {
    _packed.or_(other._packed);
  }

  // --------------------------------------------------------------------------
  // [Del]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE void del(uint32_t c, uint32_t mask) {
    switch (c) {
      case kX86RegClassGp : _gp  &= ~static_cast<uint16_t>(mask); break;
      case kX86RegClassFp : _fp  &= ~static_cast<uint8_t >(mask); break;
      case kX86RegClassMm : _mm  &= ~static_cast<uint8_t >(mask); break;
      case kX86RegClassXyz: _xyz &= ~static_cast<uint32_t>(mask); break;
    }
  }

  ASMJIT_INLINE void del(const X86RegMask& other) {
    _packed.del(other._packed);
  }

  // --------------------------------------------------------------------------
  // [And]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE void and_(uint32_t c, uint32_t mask) {
    switch (c) {
      case kX86RegClassGp : _gp  &= static_cast<uint16_t>(mask); break;
      case kX86RegClassFp : _fp  &= static_cast<uint8_t >(mask); break;
      case kX86RegClassMm : _mm  &= static_cast<uint8_t >(mask); break;
      case kX86RegClassXyz: _xyz &= static_cast<uint32_t>(mask); break;
    }
  }

  ASMJIT_INLINE void and_(const X86RegMask& other) {
    _packed.and_(other._packed);
  }

  // --------------------------------------------------------------------------
  // [Xor]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE void xor_(uint32_t c, uint32_t mask) {
    switch (c) {
      case kX86RegClassGp : _gp  ^= static_cast<uint16_t>(mask); break;
      case kX86RegClassFp : _fp  ^= static_cast<uint8_t >(mask); break;
      case kX86RegClassMm : _mm  ^= static_cast<uint8_t >(mask); break;
      case kX86RegClassXyz: _xyz ^= static_cast<uint32_t>(mask); break;
    }
  }

  ASMJIT_INLINE void xor_(const X86RegMask& other) {
    _packed.xor_(other._packed);
  }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  union {
    struct {
      //! Gp mask (16-bit).
      uint16_t _gp;
      //! Fp mask (8-bit).
      uint8_t _fp;
      //! Mm mask (8-bit).
      uint8_t _mm;
      //! Xmm/Ymm/Zmm mask (32-bit).
      uint32_t _xyz;
    };

    //! All masks as 64-bit integer.
    UInt64 _packed;
  };
};

// ============================================================================
// [asmjit::X86Reg]
// ============================================================================

//! Base class for all X86 registers.
struct X86Reg : public Reg {
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Create a dummy X86 register.
  ASMJIT_INLINE X86Reg() : Reg() {}
  //! Create a reference to `other` X86 register.
  ASMJIT_INLINE X86Reg(const X86Reg& other) : Reg(other) {}
  //! Create a reference to `other` X86 register and change the index to `index`.
  ASMJIT_INLINE X86Reg(const X86Reg& other, uint32_t index) : Reg(other, index) {}
  //! Create a custom X86 register.
  ASMJIT_INLINE X86Reg(uint32_t type, uint32_t index, uint32_t size) : Reg(type, index, size) {}
  //! Create non-initialized X86 register.
  explicit ASMJIT_INLINE X86Reg(const _NoInit&) : Reg(NoInit) {}

  // --------------------------------------------------------------------------
  // [X86Reg Specific]
  // --------------------------------------------------------------------------

  ASMJIT_REG_OP(X86Reg)

  //! Get whether the register is Gp register.
  ASMJIT_INLINE bool isGp() const { return _vreg.type <= kX86RegTypeGpq; }
  //! Get whether the register is Gp byte (8-bit) register.
  ASMJIT_INLINE bool isGpb() const { return _vreg.type <= kX86RegTypeGpbHi; }
  //! Get whether the register is Gp lo-byte (8-bit) register.
  ASMJIT_INLINE bool isGpbLo() const { return _vreg.type == kX86RegTypeGpbLo; }
  //! Get whether the register is Gp hi-byte (8-bit) register.
  ASMJIT_INLINE bool isGpbHi() const { return _vreg.type == kX86RegTypeGpbHi; }
  //! Get whether the register is Gp word (16-bit) register.
  ASMJIT_INLINE bool isGpw() const { return _vreg.type == kX86RegTypeGpw; }
  //! Get whether the register is Gp dword (32-bit) register.
  ASMJIT_INLINE bool isGpd() const { return _vreg.type == kX86RegTypeGpd; }
  //! Get whether the register is Gp qword (64-bit) register.
  ASMJIT_INLINE bool isGpq() const { return _vreg.type == kX86RegTypeGpq; }

  //! Get whether the register is Fp register.
  ASMJIT_INLINE bool isFp() const { return _vreg.type == kX86RegTypeFp; }
  //! Get whether the register is Mm (64-bit) register.
  ASMJIT_INLINE bool isMm() const { return _vreg.type == kX86RegTypeMm; }
  //! Get whether the register is Xmm (128-bit) register.
  ASMJIT_INLINE bool isXmm() const { return _vreg.type == kX86RegTypeXmm; }
  //! Get whether the register is Ymm (256-bit) register.
  ASMJIT_INLINE bool isYmm() const { return _vreg.type == kX86RegTypeYmm; }
  //! Get whether the register is Zmm (512-bit) register.
  ASMJIT_INLINE bool isZmm() const { return _vreg.type == kX86RegTypeZmm; }

  //! Get whether the register is a segment.
  ASMJIT_INLINE bool isSeg() const { return _vreg.type == kX86RegTypeSeg; }

  // --------------------------------------------------------------------------
  // [Statics]
  // --------------------------------------------------------------------------

  //! Get whether the `op` operand is Gpb-Lo or Gpb-Hi register.
  static ASMJIT_INLINE bool isGpbReg(const Operand& op) {
    const uint32_t mask = IntUtil::pack32_2x8_1x16(
      0xFF, 0xFF, ~(_kX86RegTypePatchedGpbHi << 8) & 0xFF00);

    return (op._packed[0].u32[0] & mask) == IntUtil::pack32_2x8_1x16(kOperandTypeReg, 1, 0x0000);
  }
};

// ============================================================================
// [asmjit::X86GpReg]
// ============================================================================

//! X86/X64 Gpb/Gpw/Gpd/Gpq register.
struct X86GpReg : public X86Reg {
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Create a dummy Gp register.
  ASMJIT_INLINE X86GpReg() : X86Reg() {}
  //! Create a reference to `other` Gp register.
  ASMJIT_INLINE X86GpReg(const X86GpReg& other) : X86Reg(other) {}
  //! Create a reference to `other` Gp register and change the index to `index`.
  ASMJIT_INLINE X86GpReg(const X86GpReg& other, uint32_t index) : X86Reg(other, index) {}
  //! Create a custom Gp register.
  ASMJIT_INLINE X86GpReg(uint32_t type, uint32_t index, uint32_t size) : X86Reg(type, index, size) {}
  //! Create non-initialized Gp register.
  explicit ASMJIT_INLINE X86GpReg(const _NoInit&) : X86Reg(NoInit) {}

  // --------------------------------------------------------------------------
  // [X86GpReg Specific]
  // --------------------------------------------------------------------------

  ASMJIT_REG_OP(X86GpReg)
};

// ============================================================================
// [asmjit::X86FpReg]
// ============================================================================

//! X86/X64 80-bit Fp register.
struct X86FpReg : public X86Reg {
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Create a dummy Fp register.
  ASMJIT_INLINE X86FpReg() : X86Reg() {}
  //! Create a reference to `other` Fp register.
  ASMJIT_INLINE X86FpReg(const X86FpReg& other) : X86Reg(other) {}
  //! Create a reference to `other` Fp register and change the index to `index`.
  ASMJIT_INLINE X86FpReg(const X86FpReg& other, uint32_t index) : X86Reg(other, index) {}
  //! Create a custom Fp register.
  ASMJIT_INLINE X86FpReg(uint32_t type, uint32_t index, uint32_t size) : X86Reg(type, index, size) {}
  //! Create non-initialized Fp register.
  explicit ASMJIT_INLINE X86FpReg(const _NoInit&) : X86Reg(NoInit) {}

  // --------------------------------------------------------------------------
  // [X86FpReg Specific]
  // --------------------------------------------------------------------------

  ASMJIT_REG_OP(X86FpReg)
};

// ============================================================================
// [asmjit::X86MmReg]
// ============================================================================

//! X86/X64 64-bit Mm register.
struct X86MmReg : public X86Reg {
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Create a dummy Mm register.
  ASMJIT_INLINE X86MmReg() : X86Reg() {}
  //! Create a reference to `other` Mm register.
  ASMJIT_INLINE X86MmReg(const X86MmReg& other) : X86Reg(other) {}
  //! Create a reference to `other` Mm register and change the index to `index`.
  ASMJIT_INLINE X86MmReg(const X86MmReg& other, uint32_t index) : X86Reg(other, index) {}
  //! Create a custom Mm register.
  ASMJIT_INLINE X86MmReg(uint32_t type, uint32_t index, uint32_t size) : X86Reg(type, index, size) {}
  //! Create non-initialized Mm register.
  explicit ASMJIT_INLINE X86MmReg(const _NoInit&) : X86Reg(NoInit) {}

  // --------------------------------------------------------------------------
  // [X86MmReg Specific]
  // --------------------------------------------------------------------------

  ASMJIT_REG_OP(X86MmReg)
};

// ============================================================================
// [asmjit::X86XmmReg]
// ============================================================================

//! X86/X64 128-bit Xmm register.
struct X86XmmReg : public X86Reg {
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Create a dummy Xmm register.
  ASMJIT_INLINE X86XmmReg() : X86Reg() {}
  //! Create a reference to `other` Xmm register.
  ASMJIT_INLINE X86XmmReg(const X86XmmReg& other) : X86Reg(other) {}
  //! Create a reference to `other` Xmm register and change the index to `index`.
  ASMJIT_INLINE X86XmmReg(const X86XmmReg& other, uint32_t index) : X86Reg(other, index) {}
  //! Create a custom Xmm register.
  ASMJIT_INLINE X86XmmReg(uint32_t type, uint32_t index, uint32_t size) : X86Reg(type, index, size) {}
  //! Create non-initialized Xmm register.
  explicit ASMJIT_INLINE X86XmmReg(const _NoInit&) : X86Reg(NoInit) {}

  // --------------------------------------------------------------------------
  // [X86XmmReg Specific]
  // --------------------------------------------------------------------------

  ASMJIT_REG_OP(X86XmmReg)
};

// ============================================================================
// [asmjit::X86YmmReg]
// ============================================================================

//! X86/X64 256-bit Ymm register.
struct X86YmmReg : public X86Reg {
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Create a dummy Ymm register.
  ASMJIT_INLINE X86YmmReg() : X86Reg() {}
  //! Create a reference to `other` Xmm register.
  ASMJIT_INLINE X86YmmReg(const X86YmmReg& other) : X86Reg(other) {}
  //! Create a reference to `other` Ymm register and change the index to `index`.
  ASMJIT_INLINE X86YmmReg(const X86YmmReg& other, uint32_t index) : X86Reg(other, index) {}
  //! Create a custom Ymm register.
  ASMJIT_INLINE X86YmmReg(uint32_t type, uint32_t index, uint32_t size) : X86Reg(type, index, size) {}
  //! Create non-initialized Ymm register.
  explicit ASMJIT_INLINE X86YmmReg(const _NoInit&) : X86Reg(NoInit) {}

  // --------------------------------------------------------------------------
  // [X86YmmReg Specific]
  // --------------------------------------------------------------------------

  ASMJIT_REG_OP(X86YmmReg)
};

// ============================================================================
// [asmjit::X86SegReg]
// ============================================================================

//! X86/X64 segment register.
struct X86SegReg : public X86Reg {
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Create a dummy segment register.
  ASMJIT_INLINE X86SegReg() : X86Reg() {}
  //! Create a reference to `other` segment register.
  ASMJIT_INLINE X86SegReg(const X86SegReg& other) : X86Reg(other) {}
  //! Create a reference to `other` segment register and change the index to `index`.
  ASMJIT_INLINE X86SegReg(const X86SegReg& other, uint32_t index) : X86Reg(other, index) {}
  //! Create a custom segment register.
  ASMJIT_INLINE X86SegReg(uint32_t type, uint32_t index, uint32_t size) : X86Reg(type, index, size) {}
  //! Create non-initialized segment register.
  explicit ASMJIT_INLINE X86SegReg(const _NoInit&) : X86Reg(NoInit) {}

  // --------------------------------------------------------------------------
  // [X86SegReg Specific]
  // --------------------------------------------------------------------------

  ASMJIT_REG_OP(X86SegReg)
};

// ============================================================================
// [asmjit::X86Mem]
// ============================================================================

//! X86 memory operand.
struct X86Mem : public BaseMem {
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE X86Mem() : BaseMem(NoInit) {
    reset();
  }

  ASMJIT_INLINE X86Mem(const Label& label, int32_t disp, uint32_t size = 0) : BaseMem(NoInit) {
    _init_packed_op_sz_b0_b1_id(kOperandTypeMem, size, kMemTypeLabel, 0, label._base.id);
    _init_packed_d2_d3(kInvalidValue, disp);
  }

  ASMJIT_INLINE X86Mem(const Label& label, const X86GpReg& index, uint32_t shift, int32_t disp, uint32_t size = 0) : BaseMem(NoInit) {
    ASMJIT_ASSERT(shift <= 3);

    _init_packed_op_sz_b0_b1_id(kOperandTypeMem, size, kMemTypeLabel,
      (kX86MemVSibGpz << kX86MemVSibIndex)
        + (shift << kX86MemShiftIndex),
      label.getId());
    _vmem.index = index.getRegIndex();
    _vmem.displacement = disp;
  }

  ASMJIT_INLINE X86Mem(const X86GpReg& base, int32_t disp, uint32_t size = 0) : BaseMem(NoInit) {
    _init_packed_op_sz_b0_b1_id(kOperandTypeMem, size, kMemTypeBaseIndex,
      _getGpdFlags(base)
        + (kX86MemVSibGpz << kX86MemVSibIndex),
      base.getRegIndex());
    _init_packed_d2_d3(kInvalidValue, disp);
  }

  ASMJIT_INLINE X86Mem(const X86GpReg& base, const X86GpReg& index, uint32_t shift, int32_t disp, uint32_t size = 0) : BaseMem(NoInit) {
    ASMJIT_ASSERT(shift <= 3);

    _init_packed_op_sz_b0_b1_id(kOperandTypeMem, size, kMemTypeBaseIndex,
      _getGpdFlags(base) + (shift << kX86MemShiftIndex),
      base.getRegIndex());
    _vmem.index = index.getRegIndex();
    _vmem.displacement = disp;
  }

  ASMJIT_INLINE X86Mem(const X86GpReg& base, const X86XmmReg& index, uint32_t shift, int32_t disp, uint32_t size = 0) : BaseMem(NoInit) {
    ASMJIT_ASSERT(shift <= 3);

    _init_packed_op_sz_b0_b1_id(kOperandTypeMem, size, kMemTypeBaseIndex,
      _getGpdFlags(base)
        + (kX86MemVSibXmm << kX86MemVSibIndex)
        + (shift << kX86MemShiftIndex),
      base.getRegIndex());
    _vmem.index = index.getRegIndex();
    _vmem.displacement = disp;
  }

  ASMJIT_INLINE X86Mem(const X86GpReg& base, const X86YmmReg& index, uint32_t shift, int32_t disp, uint32_t size = 0) : BaseMem(NoInit) {
    ASMJIT_ASSERT(shift <= 3);

    _init_packed_op_sz_b0_b1_id(kOperandTypeMem, size, kMemTypeBaseIndex,
      _getGpdFlags(base)
        + (kX86MemVSibYmm << kX86MemVSibIndex)
        + (shift << kX86MemShiftIndex),
      base.getRegIndex());
    _vmem.index = index.getRegIndex();
    _vmem.displacement = disp;
  }

#if !defined(ASMJIT_DISABLE_COMPILER)
  ASMJIT_INLINE X86Mem(const Label& label, const X86GpVar& index, uint32_t shift, int32_t disp, uint32_t size = 0) : BaseMem(NoInit) {
    ASMJIT_ASSERT(shift <= 3);

    _init_packed_op_sz_b0_b1_id(kOperandTypeMem, size, kMemTypeLabel,
      (kX86MemVSibGpz << kX86MemVSibIndex)
        + (shift << kX86MemShiftIndex),
      label.getId());
    _vmem.index = _OP_ID(index);
    _vmem.displacement = disp;
  }

  ASMJIT_INLINE X86Mem(const X86GpVar& base, int32_t disp, uint32_t size = 0) : BaseMem(NoInit) {
    _init_packed_op_sz_b0_b1_id(kOperandTypeMem, size, kMemTypeBaseIndex,
      _getGpdFlags(reinterpret_cast<const Var&>(base))
        + (kX86MemVSibGpz << kX86MemVSibIndex),
      _OP_ID(base));
    _init_packed_d2_d3(kInvalidValue, disp);
  }

  ASMJIT_INLINE X86Mem(const X86GpVar& base, const X86GpVar& index, uint32_t shift, int32_t disp, uint32_t size = 0) : BaseMem(NoInit) {
    ASMJIT_ASSERT(shift <= 3);

    _init_packed_op_sz_b0_b1_id(kOperandTypeMem, size, kMemTypeBaseIndex,
      _getGpdFlags(reinterpret_cast<const Var&>(base))
        + (shift << kX86MemShiftIndex),
      _OP_ID(base));
    _vmem.index = _OP_ID(index);
    _vmem.displacement = disp;
  }

  ASMJIT_INLINE X86Mem(const X86GpVar& base, const X86XmmVar& index, uint32_t shift, int32_t disp, uint32_t size = 0) : BaseMem(NoInit) {
    ASMJIT_ASSERT(shift <= 3);

    _init_packed_op_sz_b0_b1_id(kOperandTypeMem, size, kMemTypeBaseIndex,
      _getGpdFlags(reinterpret_cast<const Var&>(base))
        + (kX86MemVSibXmm << kX86MemVSibIndex)
        + (shift << kX86MemShiftIndex),
      _OP_ID(base));
    _vmem.index = _OP_ID(index);
    _vmem.displacement = disp;
  }

  ASMJIT_INLINE X86Mem(const X86GpVar& base, const X86YmmVar& index, uint32_t shift, int32_t disp, uint32_t size = 0) : BaseMem(NoInit) {
    ASMJIT_ASSERT(shift <= 3);

    _init_packed_op_sz_b0_b1_id(kOperandTypeMem, size, kMemTypeBaseIndex,
      _getGpdFlags(reinterpret_cast<const Var&>(base))
        + (kX86MemVSibYmm << kX86MemVSibIndex)
        + (shift << kX86MemShiftIndex),
      _OP_ID(base));
    _vmem.index = _OP_ID(index);
    _vmem.displacement = disp;
  }

  ASMJIT_INLINE X86Mem(const _Init&, uint32_t memType, const X86Var& base, int32_t disp, uint32_t size) : BaseMem(NoInit) {
    _init_packed_op_sz_b0_b1_id(kOperandTypeMem, size, memType, 0, _OP_ID(base));
    _vmem.index = kInvalidValue;
    _vmem.displacement = disp;
  }

  ASMJIT_INLINE X86Mem(const _Init&, uint32_t memType, const X86Var& base, const X86GpVar& index, uint32_t shift, int32_t disp, uint32_t size) : BaseMem(NoInit) {
    ASMJIT_ASSERT(shift <= 3);

    _init_packed_op_sz_b0_b1_id(kOperandTypeMem, size, memType, shift << kX86MemShiftIndex, _OP_ID(base));
    _vmem.index = _OP_ID(index);
    _vmem.displacement = disp;
  }
#endif // !ASMJIT_DISABLE_COMPILER

  ASMJIT_INLINE X86Mem(const X86Mem& other) : BaseMem(other) {}
  explicit ASMJIT_INLINE X86Mem(const _NoInit&) : BaseMem(NoInit) {}

  // --------------------------------------------------------------------------
  // [X86Mem Specific]
  // --------------------------------------------------------------------------

  //! Clone X86Mem operand.
  ASMJIT_INLINE X86Mem clone() const {
    return X86Mem(*this);
  }

  //! Reset X86Mem operand.
  ASMJIT_INLINE void reset() {
    _init_packed_op_sz_b0_b1_id(kOperandTypeMem, 0, kMemTypeBaseIndex, 0, kInvalidValue);
    _init_packed_d2_d3(kInvalidValue, 0);
  }

  //! \internal
  ASMJIT_INLINE void _init(uint32_t memType, uint32_t base, int32_t disp, uint32_t size) {
    _init_packed_op_sz_b0_b1_id(kOperandTypeMem, size, memType, 0, base);
    _vmem.index = kInvalidValue;
    _vmem.displacement = disp;
  }

  // --------------------------------------------------------------------------
  // [Segment]
  // --------------------------------------------------------------------------

  //! Get whether the memory operand has segment override prefix.
  ASMJIT_INLINE bool hasSegment() const {
    return (_vmem.flags & kX86MemSegMask) != (kX86SegDefault << kX86MemSegIndex);
  }

  //! Get memory operand segment, see `kX86Seg`.
  ASMJIT_INLINE uint32_t getSegment() const {
    return (static_cast<uint32_t>(_vmem.flags) >> kX86MemSegIndex) & kX86MemSegBits;
  }

  //! Set memory operand segment, see `kX86Seg`.
  ASMJIT_INLINE X86Mem& setSegment(uint32_t segIndex) {
    _vmem.flags = static_cast<uint8_t>(
      (static_cast<uint32_t>(_vmem.flags) & kX86MemSegMask) + (segIndex << kX86MemSegIndex));
    return *this;
  }

  //! Set memory operand segment, see `kX86Seg`.
  ASMJIT_INLINE X86Mem& setSegment(const X86SegReg& seg) {
    return setSegment(seg.getRegIndex());
  }

  // --------------------------------------------------------------------------
  // [Gpd]
  // --------------------------------------------------------------------------

  //! Get whether the memory operand has 32-bit GP base.
  ASMJIT_INLINE bool hasGpdBase() const {
    return (_packed[0].u32[0] & IntUtil::pack32_4x8(0x00, 0x00, 0x00, kX86MemGpdMask)) != 0;
  }

  //! Set whether the memory operand has 32-bit GP base.
  ASMJIT_INLINE X86Mem& setGpdBase() {
    _packed[0].u32[0] |= IntUtil::pack32_4x8(0x00, 0x00, 0x00, kX86MemGpdMask);
    return *this;
  }

  //! Set whether the memory operand has 32-bit GP base to `b`.
  ASMJIT_INLINE X86Mem& setGpdBase(uint32_t b) {
    _packed[0].u32[0] &=~IntUtil::pack32_4x8(0x00, 0x00, 0x00, kX86MemGpdMask);
    _packed[0].u32[0] |= IntUtil::pack32_4x8(0x00, 0x00, 0x00, b << kX86MemGpdIndex);
    return *this;
  }

  // --------------------------------------------------------------------------
  // [VSib]
  // --------------------------------------------------------------------------

  //! Get SIB type.
  ASMJIT_INLINE uint32_t getVSib() const {
    return (static_cast<uint32_t>(_vmem.flags) >> kX86MemVSibIndex) & kX86MemVSibBits;
  }

  //! Set SIB type.
  ASMJIT_INLINE X86Mem& _setVSib(uint32_t vsib) {
    _packed[0].u32[0] &=~IntUtil::pack32_4x8(0x00, 0x00, 0x00, kX86MemVSibMask);
    _packed[0].u32[0] |= IntUtil::pack32_4x8(0x00, 0x00, 0x00, vsib << kX86MemVSibIndex);
    return *this;
  }

  // --------------------------------------------------------------------------
  // [Size]
  // --------------------------------------------------------------------------

  //! Set memory operand size.
  ASMJIT_INLINE X86Mem& setSize(uint32_t size) {
    _vmem.size = static_cast<uint8_t>(size);
    return *this;
  }

  // --------------------------------------------------------------------------
  // [Base]
  // --------------------------------------------------------------------------

  //! Get whether the memory operand has base register.
  ASMJIT_INLINE bool hasBase() const {
    return _vmem.base != kInvalidValue;
  }

  //! Get memory operand base register code, variable id, or `kInvalidValue`.
  ASMJIT_INLINE uint32_t getBase() const {
    return _vmem.base;
  }

  //! Set memory operand base register code, variable id, or `kInvalidValue`.
  ASMJIT_INLINE X86Mem& setBase(uint32_t base) {
    _vmem.base = base;
    return *this;
  }

  // --------------------------------------------------------------------------
  // [Index]
  // --------------------------------------------------------------------------

  //! Get whether the memory operand has index.
  ASMJIT_INLINE bool hasIndex() const {
    return _vmem.index != kInvalidValue;
  }

  //! Get memory operand index register code, variable id, or `kInvalidValue`.
  ASMJIT_INLINE uint32_t getIndex() const {
    return _vmem.index;
  }

  //! Set memory operand index register code, variable id, or `kInvalidValue`.
  ASMJIT_INLINE X86Mem& setIndex(uint32_t index) {
    _vmem.index = index;
    return *this;
  }

  //! Set memory index.
  ASMJIT_INLINE X86Mem& setIndex(const X86GpReg& index) {
    _vmem.index = index.getRegIndex();
    return _setVSib(kX86MemVSibGpz);
  }

  //! Set memory index.
  ASMJIT_INLINE X86Mem& setIndex(const X86GpReg& index, uint32_t shift) {
    _vmem.index = index.getRegIndex();
    return _setVSib(kX86MemVSibGpz).setShift(shift);
  }

  //! Set memory index.
  ASMJIT_INLINE X86Mem& setIndex(const X86XmmReg& index) {
    _vmem.index = index.getRegIndex();
    return _setVSib(kX86MemVSibXmm);
  }

  //! Set memory index.
  ASMJIT_INLINE X86Mem& setIndex(const X86XmmReg& index, uint32_t shift) {
    _vmem.index = index.getRegIndex();
    return _setVSib(kX86MemVSibXmm).setShift(shift);
  }

  //! Set memory index.
  ASMJIT_INLINE X86Mem& setIndex(const X86YmmReg& index) {
    _vmem.index = index.getRegIndex();
    return _setVSib(kX86MemVSibYmm);
  }

  //! Set memory index.
  ASMJIT_INLINE X86Mem& setIndex(const X86YmmReg& index, uint32_t shift) {
    _vmem.index = index.getRegIndex();
    return _setVSib(kX86MemVSibYmm).setShift(shift);
  }

#if !defined(ASMJIT_DISABLE_COMPILER)
  //! Set memory index.
  ASMJIT_INLINE X86Mem& setIndex(const X86GpVar& index) {
    _vmem.index = _OP_ID(index);
    return _setVSib(kX86MemVSibGpz);
  }

  //! Set memory index.
  ASMJIT_INLINE X86Mem& setIndex(const X86GpVar& index, uint32_t shift) {
    _vmem.index = _OP_ID(index);
    return _setVSib(kX86MemVSibGpz).setShift(shift);
  }


  //! Set memory index.
  ASMJIT_INLINE X86Mem& setIndex(const X86XmmVar& index) {
    _vmem.index = _OP_ID(index);
    return _setVSib(kX86MemVSibXmm);
  }

  //! Set memory index.
  ASMJIT_INLINE X86Mem& setIndex(const X86XmmVar& index, uint32_t shift) {
    _vmem.index = _OP_ID(index);
    return _setVSib(kX86MemVSibXmm).setShift(shift);
  }

  //! Set memory index.
  ASMJIT_INLINE X86Mem& setIndex(const X86YmmVar& index) {
    _vmem.index = _OP_ID(index);
    return _setVSib(kX86MemVSibYmm);
  }

  //! Set memory index.
  ASMJIT_INLINE X86Mem& setIndex(const X86YmmVar& index, uint32_t shift) {
    _vmem.index = _OP_ID(index);
    return _setVSib(kX86MemVSibYmm).setShift(shift);
  }
#endif // !ASMJIT_DISABLE_COMPILER

  //! Reset memory index.
  ASMJIT_INLINE X86Mem& resetIndex() {
    _vmem.index = kInvalidValue;
    return _setVSib(kX86MemVSibGpz);
  }

  // --------------------------------------------------------------------------
  // [Misc]
  // --------------------------------------------------------------------------

  //! Get whether the memory operand has base and index register.
  ASMJIT_INLINE bool hasBaseOrIndex() const {
    return _vmem.base != kInvalidValue || _vmem.index != kInvalidValue;
  }

  //! Get whether the memory operand has base and index register.
  ASMJIT_INLINE bool hasBaseAndIndex() const {
    return _vmem.base != kInvalidValue && _vmem.index != kInvalidValue;
  }

  // --------------------------------------------------------------------------
  // [Shift]
  // --------------------------------------------------------------------------

  //! Get whether the memory operand has shift used.
  ASMJIT_INLINE bool hasShift() const {
    return (_vmem.flags & kX86MemShiftMask) != 0;
  }

  //! Get memory operand index scale (0, 1, 2 or 3).
  ASMJIT_INLINE uint32_t getShift() const {
    return _vmem.flags >> kX86MemShiftIndex;
  }

  //! Set memory operand index scale (0, 1, 2 or 3).
  ASMJIT_INLINE X86Mem& setShift(uint32_t shift) {
    _packed[0].u32[0] &=~IntUtil::pack32_4x8(0x00, 0x00, 0x00, kX86MemShiftMask);
    _packed[0].u32[0] |= IntUtil::pack32_4x8(0x00, 0x00, 0x00, shift << kX86MemShiftIndex);
    return *this;
  }

  // --------------------------------------------------------------------------
  // [Displacement]
  // --------------------------------------------------------------------------

  //! Get memory operand relative displacement.
  ASMJIT_INLINE int32_t getDisplacement() const {
    return _vmem.displacement;
  }

  //! Set memory operand relative displacement.
  ASMJIT_INLINE X86Mem& setDisplacement(int32_t disp) {
    _vmem.displacement = disp;
    return *this;
  }

  //! Reset memory operand relative displacement.
  ASMJIT_INLINE X86Mem& resetDisplacement(int32_t disp) {
    _vmem.displacement = 0;
    return *this;
  }

  //! Adjust memory operand relative displacement by `disp`.
  ASMJIT_INLINE X86Mem& adjust(int32_t disp) {
    _vmem.displacement += disp;
    return *this;
  }

  //! Get new memory operand adjusted by `disp`.
  ASMJIT_INLINE X86Mem adjusted(int32_t disp) const {
    X86Mem result(*this);
    result.adjust(disp);
    return result;
  }

  // --------------------------------------------------------------------------
  // [Operator Overload]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE X86Mem& operator=(const X86Mem& other) {
    _copy(other);
    return *this;
  }

  ASMJIT_INLINE bool operator==(const X86Mem& other) const {
    return (_packed[0] == other._packed[0]) & (_packed[1] == other._packed[1]) ;
  }

  ASMJIT_INLINE bool operator!=(const X86Mem& other) const {
    return !(*this == other);
  }

  // --------------------------------------------------------------------------
  // [Static]
  // --------------------------------------------------------------------------

  static ASMJIT_INLINE uint32_t _getGpdFlags(const Operand& base) {
    return (base._vreg.size & 0x4) << (kX86MemGpdIndex - 2);
  }
};
#endif // !ASMJIT_EXPORTS_X86OPERAND_REGS

// ============================================================================
// [asmjit::x86]
// ============================================================================

namespace x86 {

// ============================================================================
// [asmjit::x86 - Reg]
// ============================================================================

//! No Gp register, can be used only within `X86Mem` operand.
ASMJIT_VAR const X86GpReg noGpReg;

ASMJIT_VAR const X86GpReg al;     //!< 8-bit Gpb-lo register.
ASMJIT_VAR const X86GpReg cl;     //!< 8-bit Gpb-lo register.
ASMJIT_VAR const X86GpReg dl;     //!< 8-bit Gpb-lo register.
ASMJIT_VAR const X86GpReg bl;     //!< 8-bit Gpb-lo register.
ASMJIT_VAR const X86GpReg spl;    //!< 8-bit Gpb-lo register (X64).
ASMJIT_VAR const X86GpReg bpl;    //!< 8-bit Gpb-lo register (X64).
ASMJIT_VAR const X86GpReg sil;    //!< 8-bit Gpb-lo register (X64).
ASMJIT_VAR const X86GpReg dil;    //!< 8-bit Gpb-lo register (X64).
ASMJIT_VAR const X86GpReg r8b;    //!< 8-bit Gpb-lo register (X64).
ASMJIT_VAR const X86GpReg r9b;    //!< 8-bit Gpb-lo register (X64).
ASMJIT_VAR const X86GpReg r10b;   //!< 8-bit Gpb-lo register (X64).
ASMJIT_VAR const X86GpReg r11b;   //!< 8-bit Gpb-lo register (X64).
ASMJIT_VAR const X86GpReg r12b;   //!< 8-bit Gpb-lo register (X64).
ASMJIT_VAR const X86GpReg r13b;   //!< 8-bit Gpb-lo register (X64).
ASMJIT_VAR const X86GpReg r14b;   //!< 8-bit Gpb-lo register (X64).
ASMJIT_VAR const X86GpReg r15b;   //!< 8-bit Gpb-lo register (X64).

ASMJIT_VAR const X86GpReg ah;     //!< 8-bit Gpb-hi register.
ASMJIT_VAR const X86GpReg ch;     //!< 8-bit Gpb-hi register.
ASMJIT_VAR const X86GpReg dh;     //!< 8-bit Gpb-hi register.
ASMJIT_VAR const X86GpReg bh;     //!< 8-bit Gpb-hi register.

ASMJIT_VAR const X86GpReg ax;     //!< 16-bit Gpw register.
ASMJIT_VAR const X86GpReg cx;     //!< 16-bit Gpw register.
ASMJIT_VAR const X86GpReg dx;     //!< 16-bit Gpw register.
ASMJIT_VAR const X86GpReg bx;     //!< 16-bit Gpw register.
ASMJIT_VAR const X86GpReg sp;     //!< 16-bit Gpw register.
ASMJIT_VAR const X86GpReg bp;     //!< 16-bit Gpw register.
ASMJIT_VAR const X86GpReg si;     //!< 16-bit Gpw register.
ASMJIT_VAR const X86GpReg di;     //!< 16-bit Gpw register.
ASMJIT_VAR const X86GpReg r8w;    //!< 16-bit Gpw register (X64).
ASMJIT_VAR const X86GpReg r9w;    //!< 16-bit Gpw register (X64).
ASMJIT_VAR const X86GpReg r10w;   //!< 16-bit Gpw register (X64).
ASMJIT_VAR const X86GpReg r11w;   //!< 16-bit Gpw register (X64).
ASMJIT_VAR const X86GpReg r12w;   //!< 16-bit Gpw register (X64).
ASMJIT_VAR const X86GpReg r13w;   //!< 16-bit Gpw register (X64).
ASMJIT_VAR const X86GpReg r14w;   //!< 16-bit Gpw register (X64).
ASMJIT_VAR const X86GpReg r15w;   //!< 16-bit Gpw register (X64).

ASMJIT_VAR const X86GpReg eax;    //!< 32-bit Gpd register.
ASMJIT_VAR const X86GpReg ecx;    //!< 32-bit Gpd register.
ASMJIT_VAR const X86GpReg edx;    //!< 32-bit Gpd register.
ASMJIT_VAR const X86GpReg ebx;    //!< 32-bit Gpd register.
ASMJIT_VAR const X86GpReg esp;    //!< 32-bit Gpd register.
ASMJIT_VAR const X86GpReg ebp;    //!< 32-bit Gpd register.
ASMJIT_VAR const X86GpReg esi;    //!< 32-bit Gpd register.
ASMJIT_VAR const X86GpReg edi;    //!< 32-bit Gpd register.
ASMJIT_VAR const X86GpReg r8d;    //!< 32-bit Gpd register (X64).
ASMJIT_VAR const X86GpReg r9d;    //!< 32-bit Gpd register (X64).
ASMJIT_VAR const X86GpReg r10d;   //!< 32-bit Gpd register (X64).
ASMJIT_VAR const X86GpReg r11d;   //!< 32-bit Gpd register (X64).
ASMJIT_VAR const X86GpReg r12d;   //!< 32-bit Gpd register (X64).
ASMJIT_VAR const X86GpReg r13d;   //!< 32-bit Gpd register (X64).
ASMJIT_VAR const X86GpReg r14d;   //!< 32-bit Gpd register (X64).
ASMJIT_VAR const X86GpReg r15d;   //!< 32-bit Gpd register (X64).

ASMJIT_VAR const X86GpReg rax;    //!< 64-bit Gpq register (X64).
ASMJIT_VAR const X86GpReg rcx;    //!< 64-bit Gpq register (X64)
ASMJIT_VAR const X86GpReg rdx;    //!< 64-bit Gpq register (X64)
ASMJIT_VAR const X86GpReg rbx;    //!< 64-bit Gpq register (X64)
ASMJIT_VAR const X86GpReg rsp;    //!< 64-bit Gpq register (X64)
ASMJIT_VAR const X86GpReg rbp;    //!< 64-bit Gpq register (X64)
ASMJIT_VAR const X86GpReg rsi;    //!< 64-bit Gpq register (X64)
ASMJIT_VAR const X86GpReg rdi;    //!< 64-bit Gpq register (X64)
ASMJIT_VAR const X86GpReg r8;     //!< 64-bit Gpq register (X64)
ASMJIT_VAR const X86GpReg r9;     //!< 64-bit Gpq register (X64)
ASMJIT_VAR const X86GpReg r10;    //!< 64-bit Gpq register (X64)
ASMJIT_VAR const X86GpReg r11;    //!< 64-bit Gpq register (X64)
ASMJIT_VAR const X86GpReg r12;    //!< 64-bit Gpq register (X64)
ASMJIT_VAR const X86GpReg r13;    //!< 64-bit Gpq register (X64)
ASMJIT_VAR const X86GpReg r14;    //!< 64-bit Gpq register (X64)
ASMJIT_VAR const X86GpReg r15;    //!< 64-bit Gpq register (X64)

ASMJIT_VAR const X86FpReg fp0;    //!< 80-bit Fp register.
ASMJIT_VAR const X86FpReg fp1;    //!< 80-bit Fp register.
ASMJIT_VAR const X86FpReg fp2;    //!< 80-bit Fp register.
ASMJIT_VAR const X86FpReg fp3;    //!< 80-bit Fp register.
ASMJIT_VAR const X86FpReg fp4;    //!< 80-bit Fp register.
ASMJIT_VAR const X86FpReg fp5;    //!< 80-bit Fp register.
ASMJIT_VAR const X86FpReg fp6;    //!< 80-bit Fp register.
ASMJIT_VAR const X86FpReg fp7;    //!< 80-bit Fp register.

ASMJIT_VAR const X86MmReg mm0;    //!< 64-bit Mm register.
ASMJIT_VAR const X86MmReg mm1;    //!< 64-bit Mm register.
ASMJIT_VAR const X86MmReg mm2;    //!< 64-bit Mm register.
ASMJIT_VAR const X86MmReg mm3;    //!< 64-bit Mm register.
ASMJIT_VAR const X86MmReg mm4;    //!< 64-bit Mm register.
ASMJIT_VAR const X86MmReg mm5;    //!< 64-bit Mm register.
ASMJIT_VAR const X86MmReg mm6;    //!< 64-bit Mm register.
ASMJIT_VAR const X86MmReg mm7;    //!< 64-bit Mm register.

ASMJIT_VAR const X86XmmReg xmm0;  //!< 128-bit Xmm register.
ASMJIT_VAR const X86XmmReg xmm1;  //!< 128-bit Xmm register.
ASMJIT_VAR const X86XmmReg xmm2;  //!< 128-bit Xmm register.
ASMJIT_VAR const X86XmmReg xmm3;  //!< 128-bit Xmm register.
ASMJIT_VAR const X86XmmReg xmm4;  //!< 128-bit Xmm register.
ASMJIT_VAR const X86XmmReg xmm5;  //!< 128-bit Xmm register.
ASMJIT_VAR const X86XmmReg xmm6;  //!< 128-bit Xmm register.
ASMJIT_VAR const X86XmmReg xmm7;  //!< 128-bit Xmm register.
ASMJIT_VAR const X86XmmReg xmm8;  //!< 128-bit Xmm register (X64).
ASMJIT_VAR const X86XmmReg xmm9;  //!< 128-bit Xmm register (X64).
ASMJIT_VAR const X86XmmReg xmm10; //!< 128-bit Xmm register (X64).
ASMJIT_VAR const X86XmmReg xmm11; //!< 128-bit Xmm register (X64).
ASMJIT_VAR const X86XmmReg xmm12; //!< 128-bit Xmm register (X64).
ASMJIT_VAR const X86XmmReg xmm13; //!< 128-bit Xmm register (X64).
ASMJIT_VAR const X86XmmReg xmm14; //!< 128-bit Xmm register (X64).
ASMJIT_VAR const X86XmmReg xmm15; //!< 128-bit Xmm register (X64).

ASMJIT_VAR const X86YmmReg ymm0;  //!< 256-bit Ymm register.
ASMJIT_VAR const X86YmmReg ymm1;  //!< 256-bit Ymm register.
ASMJIT_VAR const X86YmmReg ymm2;  //!< 256-bit Ymm register.
ASMJIT_VAR const X86YmmReg ymm3;  //!< 256-bit Ymm register.
ASMJIT_VAR const X86YmmReg ymm4;  //!< 256-bit Ymm register.
ASMJIT_VAR const X86YmmReg ymm5;  //!< 256-bit Ymm register.
ASMJIT_VAR const X86YmmReg ymm6;  //!< 256-bit Ymm register.
ASMJIT_VAR const X86YmmReg ymm7;  //!< 256-bit Ymm register.
ASMJIT_VAR const X86YmmReg ymm8;  //!< 256-bit Ymm register (X64).
ASMJIT_VAR const X86YmmReg ymm9;  //!< 256-bit Ymm register (X64).
ASMJIT_VAR const X86YmmReg ymm10; //!< 256-bit Ymm register (X64).
ASMJIT_VAR const X86YmmReg ymm11; //!< 256-bit Ymm register (X64).
ASMJIT_VAR const X86YmmReg ymm12; //!< 256-bit Ymm register (X64).
ASMJIT_VAR const X86YmmReg ymm13; //!< 256-bit Ymm register (X64).
ASMJIT_VAR const X86YmmReg ymm14; //!< 256-bit Ymm register (X64).
ASMJIT_VAR const X86YmmReg ymm15; //!< 256-bit Ymm register (X64).

ASMJIT_VAR const X86SegReg cs;    //!< Cs segment register.
ASMJIT_VAR const X86SegReg ss;    //!< Ss segment register.
ASMJIT_VAR const X86SegReg ds;    //!< Ds segment register.
ASMJIT_VAR const X86SegReg es;    //!< Es segment register.
ASMJIT_VAR const X86SegReg fs;    //!< Fs segment register.
ASMJIT_VAR const X86SegReg gs;    //!< Gs segment register.

// This is only defined by `x86operand_regs.cpp` when exporting registers.
#if !defined(ASMJIT_EXPORTS_X86OPERAND_REGS)

//! Create 8-bit Gpb-lo register operand.
static ASMJIT_INLINE X86GpReg gpb_lo(uint32_t index) { return X86GpReg(kX86RegTypeGpbLo, index, 1); }
//! Create 8-bit Gpb-hi register operand.
static ASMJIT_INLINE X86GpReg gpb_hi(uint32_t index) { return X86GpReg(kX86RegTypeGpbHi, index, 1); }
//! Create 16-bit Gpw register operand.
static ASMJIT_INLINE X86GpReg gpw(uint32_t index) { return X86GpReg(kX86RegTypeGpw, index, 2); }
//! Create 32-bit Gpd register operand.
static ASMJIT_INLINE X86GpReg gpd(uint32_t index) { return X86GpReg(kX86RegTypeGpd, index, 4); }
//! Create 64-bit Gpq register operand (X64).
static ASMJIT_INLINE X86GpReg gpq(uint32_t index) { return X86GpReg(kX86RegTypeGpq, index, 8); }
//! Create 80-bit Fp register operand.
static ASMJIT_INLINE X86FpReg fp(uint32_t index) { return X86FpReg(kX86RegTypeFp, index, 10); }
//! Create 64-bit Mm register operand.
static ASMJIT_INLINE X86MmReg mm(uint32_t index) { return X86MmReg(kX86RegTypeMm, index, 8); }
//! Create 128-bit Xmm register operand.
static ASMJIT_INLINE X86XmmReg xmm(uint32_t index) { return X86XmmReg(kX86RegTypeXmm, index, 16); }
//! Create 256-bit Ymm register operand.
static ASMJIT_INLINE X86YmmReg ymm(uint32_t index) { return X86YmmReg(kX86RegTypeYmm, index, 32); }

// ============================================================================
// [asmjit::x86 - Ptr (Reg)]
// ============================================================================

//! Create `[base.reg + disp]` memory operand with no/custom size information.
static ASMJIT_INLINE X86Mem ptr(const X86GpReg& base, int32_t disp = 0, uint32_t size = 0) {
  return X86Mem(base, disp, size);
}
//! Create `[base.reg + (index.reg << shift) + disp]` memory operand with no/custom size information.
static ASMJIT_INLINE X86Mem ptr(const X86GpReg& base, const X86GpReg& index, uint32_t shift = 0, int32_t disp = 0, uint32_t size = 0) {
  return X86Mem(base, index, shift, disp, size);
}
//! Create `[base.reg + (xmm.reg << shift) + disp]` memory operand with no/custom size information.
static ASMJIT_INLINE X86Mem ptr(const X86GpReg& base, const X86XmmReg& index, uint32_t shift = 0, int32_t disp = 0, uint32_t size = 0) {
  return X86Mem(base, index, shift, disp, size);
}
//! Create `[base.reg + (ymm.reg << shift) + disp]` memory operand with no/custom size information.
static ASMJIT_INLINE X86Mem ptr(const X86GpReg& base, const X86YmmReg& index, uint32_t shift = 0, int32_t disp = 0, uint32_t size = 0) {
  return X86Mem(base, index, shift, disp, size);
}
//! Create `[label + disp]` memory operand with no/custom size information.
static ASMJIT_INLINE X86Mem ptr(const Label& label, int32_t disp = 0, uint32_t size = 0) {
  return X86Mem(label, disp, size);
}
//! Create `[label + (index.reg << shift) + disp]` memory operand with no/custom size information.
static ASMJIT_INLINE X86Mem ptr(const Label& label, const X86GpReg& index, uint32_t shift, int32_t disp = 0, uint32_t size = 0) { \
  return X86Mem(label, index, shift, disp, size); \
}

//! Create `[pAbs + disp]` absolute memory operand with no/custom size information.
ASMJIT_API X86Mem ptr_abs(Ptr pAbs, int32_t disp = 0, uint32_t size = 0);
//! Create `[pAbs + (index.reg << shift) + disp]` absolute memory operand with no/custom size information.
ASMJIT_API X86Mem ptr_abs(Ptr pAbs, const X86Reg& index, uint32_t shift = 0, int32_t disp = 0, uint32_t size = 0);

//! \internal
#define ASMJIT_EXPAND_PTR_REG(_Prefix_, _Size_) \
  /*! Create `[base.reg + disp]` memory operand. */ \
  static ASMJIT_INLINE X86Mem _Prefix_##_ptr(const X86GpReg& base, int32_t disp = 0) { \
    return X86Mem(base, disp, _Size_); \
  } \
  /*! Create `[base.reg + (index.reg << shift) + disp]` memory operand. */ \
  static ASMJIT_INLINE X86Mem _Prefix_##_ptr(const X86GpReg& base, const X86GpReg& index, uint32_t shift = 0, int32_t disp = 0) { \
    return ptr(base, index, shift, disp, _Size_); \
  } \
  /*! Create `[base.reg + (xmm.reg << shift) + disp]` memory operand. */ \
  static ASMJIT_INLINE X86Mem _Prefix_##_ptr(const X86GpReg& base, const X86XmmReg& index, uint32_t shift = 0, int32_t disp = 0) { \
    return ptr(base, index, shift, disp, _Size_); \
  } \
  /*! Create `[base.reg + (ymm.reg << shift) + disp]` memory operand. */ \
  static ASMJIT_INLINE X86Mem _Prefix_##_ptr(const X86GpReg& base, const X86YmmReg& index, uint32_t shift = 0, int32_t disp = 0) { \
    return ptr(base, index, shift, disp, _Size_); \
  } \
  /*! Create `[label + disp]` memory operand. */ \
  static ASMJIT_INLINE X86Mem _Prefix_##_ptr(const Label& label, int32_t disp = 0) { \
    return ptr(label, disp, _Size_); \
  } \
  /*! Create `[label + (index.reg << shift) + disp]` memory operand. */ \
  static ASMJIT_INLINE X86Mem _Prefix_##_ptr(const Label& label, const X86GpReg& index, uint32_t shift, int32_t disp = 0) { \
    return ptr(label, index, shift, disp, _Size_); \
  } \
  /*! Create `[pAbs + disp]` memory operand. */ \
  static ASMJIT_INLINE X86Mem _Prefix_##_ptr##_abs(Ptr pAbs, int32_t disp = 0) { \
    return ptr_abs(pAbs, disp, _Size_); \
  } \
  /*! Create `[pAbs + (index.reg << shift) + disp]` memory operand. */ \
  static ASMJIT_INLINE X86Mem _Prefix_##_ptr##_abs(Ptr pAbs, const X86GpReg& index, uint32_t shift = 0, int32_t disp = 0) { \
    return ptr_abs(pAbs, index, shift, disp, _Size_); \
  } \
  /*! Create `[pAbs + (xmm.reg << shift) + disp]` memory operand. */ \
  static ASMJIT_INLINE X86Mem _Prefix_##_ptr##_abs(Ptr pAbs, const X86XmmReg& index, uint32_t shift = 0, int32_t disp = 0) { \
    return ptr_abs(pAbs, index, shift, disp, _Size_); \
  } \
  /*! Create `[pAbs + (ymm.reg << shift) + disp]` memory operand. */ \
  static ASMJIT_INLINE X86Mem _Prefix_##_ptr##_abs(Ptr pAbs, const X86YmmReg& index, uint32_t shift = 0, int32_t disp = 0) { \
    return ptr_abs(pAbs, index, shift, disp, _Size_); \
  }

ASMJIT_EXPAND_PTR_REG(byte, 1)
ASMJIT_EXPAND_PTR_REG(word, 2)
ASMJIT_EXPAND_PTR_REG(dword, 4)
ASMJIT_EXPAND_PTR_REG(qword, 8)
ASMJIT_EXPAND_PTR_REG(tword, 10)
ASMJIT_EXPAND_PTR_REG(oword, 16)
ASMJIT_EXPAND_PTR_REG(yword, 32)
ASMJIT_EXPAND_PTR_REG(zword, 64)
#undef ASMJIT_EXPAND_PTR_REG

// ============================================================================
// [asmjit::x86 - Ptr (Var)]
// ============================================================================

#if !defined(ASMJIT_DISABLE_COMPILER)
//! Create `[base.var + disp]` memory operand with no/custom size information.
static ASMJIT_INLINE X86Mem ptr(const X86GpVar& base, int32_t disp = 0, uint32_t size = 0) {
  return X86Mem(base, disp, size);
}
//! Create `[base.var + (index.var << shift) + disp]` memory operand with no/custom size information.
static ASMJIT_INLINE X86Mem ptr(const X86GpVar& base, const X86GpVar& index, uint32_t shift = 0, int32_t disp = 0, uint32_t size = 0) {
  return X86Mem(base, index, shift, disp, size);
}
//! Create `[base.var + (xmm.var << shift) + disp]` memory operand with no/custom size information.
static ASMJIT_INLINE X86Mem ptr(const X86GpVar& base, const X86XmmVar& index, uint32_t shift = 0, int32_t disp = 0, uint32_t size = 0) {
  return X86Mem(base, index, shift, disp, size);
}
//! Create `[base.var + (ymm.var << shift) + disp]` memory operand with no/custom size information.
static ASMJIT_INLINE X86Mem ptr(const X86GpVar& base, const X86YmmVar& index, uint32_t shift = 0, int32_t disp = 0, uint32_t size = 0) {
  return X86Mem(base, index, shift, disp, size);
}
//! Create `[label + (index.var << shift) + disp]` memory operand with no/custom size information.
static ASMJIT_INLINE X86Mem ptr(const Label& label, const X86GpVar& index, uint32_t shift, int32_t disp = 0, uint32_t size = 0) { \
  return X86Mem(label, index, shift, disp, size); \
}

//! Create `[pAbs + (index.var << shift) + disp]` absolute memory operand with no/custom size information.
ASMJIT_API X86Mem ptr_abs(Ptr pAbs, const X86Var& index, uint32_t shift = 0, int32_t disp = 0, uint32_t size = 0);

//! \internal
#define ASMJIT_EXPAND_PTR_VAR(_Prefix_, _Size_) \
  /*! Create `[base.var + disp]` memory operand. */ \
  static ASMJIT_INLINE X86Mem _Prefix_##_ptr(const X86GpVar& base, int32_t disp = 0) { \
    return X86Mem(base, disp, _Size_); \
  } \
  /*! Create `[base.var + (index.var << shift) + disp]` memory operand. */ \
  static ASMJIT_INLINE X86Mem _Prefix_##_ptr(const X86GpVar& base, const X86GpVar& index, uint32_t shift = 0, int32_t disp = 0) { \
    return ptr(base, index, shift, disp, _Size_); \
  } \
  /*! Create `[base.var + (xmm.var << shift) + disp]` memory operand. */ \
  static ASMJIT_INLINE X86Mem _Prefix_##_ptr(const X86GpVar& base, const X86XmmVar& index, uint32_t shift = 0, int32_t disp = 0) { \
    return ptr(base, index, shift, disp, _Size_); \
  } \
  /*! Create `[base.var + (ymm.var << shift) + disp]` memory operand. */ \
  static ASMJIT_INLINE X86Mem _Prefix_##_ptr(const X86GpVar& base, const X86YmmVar& index, uint32_t shift = 0, int32_t disp = 0) { \
    return ptr(base, index, shift, disp, _Size_); \
  } \
  /*! Create `[label + (index.var << shift) + disp]` memory operand. */ \
  static ASMJIT_INLINE X86Mem _Prefix_##_ptr(const Label& label, const X86GpVar& index, uint32_t shift, int32_t disp = 0) { \
    return ptr(label, index, shift, disp, _Size_); \
  } \
  /*! Create `[pAbs + (index.var << shift) + disp]` memory operand. */ \
  static ASMJIT_INLINE X86Mem _Prefix_##_ptr##_abs(Ptr pAbs, const X86GpVar& index, uint32_t shift = 0, int32_t disp = 0) { \
    return ptr_abs(pAbs, reinterpret_cast<const X86Var&>(index), shift, disp, _Size_); \
  } \
  /*! Create `[pAbs + (xmm.var << shift) + disp]` memory operand. */ \
  static ASMJIT_INLINE X86Mem _Prefix_##_ptr##_abs(Ptr pAbs, const X86XmmVar& index, uint32_t shift = 0, int32_t disp = 0) { \
    return ptr_abs(pAbs, reinterpret_cast<const X86Var&>(index), shift, disp, _Size_); \
  } \
  /*! Create `[pAbs + (ymm.var << shift) + disp]` memory operand. */ \
  static ASMJIT_INLINE X86Mem _Prefix_##_ptr##_abs(Ptr pAbs, const X86YmmVar& index, uint32_t shift = 0, int32_t disp = 0) { \
    return ptr_abs(pAbs, reinterpret_cast<const X86Var&>(index), shift, disp, _Size_); \
  }

ASMJIT_EXPAND_PTR_VAR(byte, 1)
ASMJIT_EXPAND_PTR_VAR(word, 2)
ASMJIT_EXPAND_PTR_VAR(dword, 4)
ASMJIT_EXPAND_PTR_VAR(qword, 8)
ASMJIT_EXPAND_PTR_VAR(tword, 10)
ASMJIT_EXPAND_PTR_VAR(oword, 16)
ASMJIT_EXPAND_PTR_VAR(yword, 32)
ASMJIT_EXPAND_PTR_VAR(zword, 64)
#undef ASMJIT_EXPAND_PTR_VAR
#endif // !ASMJIT_DISABLE_COMPILER

#endif // !ASMJIT_EXPORTS_X86OPERAND_REGS

} // x86 namespace

//! \}

} // asmjit namespace

#undef _OP_ID

// [Api-End]
#include "../apiend.h"

// [Guard]
#endif // _ASMJIT_X86_X86OPERAND_H
