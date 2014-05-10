// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Export]
#define ASMJIT_EXPORTS

// [Guard]
#include "../build.h"
#if defined(ASMJIT_BUILD_X86) || defined(ASMJIT_BUILD_X64)

// [Dependencies - AsmJit]
#include "../x86/x86operand.h"

// [Api-Begin]
#include "../apibegin.h"

namespace asmjit {
namespace x86x64 {

// ============================================================================
// [asmjit::x86x64::Variables]
// ============================================================================

#define C(_Class_) kRegClass##_Class_
#define D(_Desc_) kVarDesc##_Desc_

const VarInfo _varInfo[] = {
  /* 00: kVarTypeInt8    */ { kRegTypeGpbLo, 1 , C(Gp) , 0                , "gpb" },
  /* 01: kVarTypeUInt8   */ { kRegTypeGpbLo, 1 , C(Gp) , 0                , "gpb" },
  /* 02: kVarTypeInt16   */ { kRegTypeGpw  , 2 , C(Gp) , 0                , "gpw" },
  /* 03: kVarTypeUInt16  */ { kRegTypeGpw  , 2 , C(Gp) , 0                , "gpw" },
  /* 04: kVarTypeInt32   */ { kRegTypeGpd  , 4 , C(Gp) , 0                , "gpd" },
  /* 05: kVarTypeUInt32  */ { kRegTypeGpd  , 4 , C(Gp) , 0                , "gpd" },
  /* 06: kVarTypeInt64   */ { kRegTypeGpq  , 8 , C(Gp) , 0                , "gpq" },
  /* 07: kVarTypeUInt64  */ { kRegTypeGpq  , 8 , C(Gp) , 0                , "gpq" },
  /* 08: kVarTypeIntPtr  */ { 0            , 0 , C(Gp) , 0                , ""    }, // Remapped.
  /* 09: kVarTypeUIntPtr */ { 0            , 0 , C(Gp) , 0                , ""    }, // Remapped.
  /* 10: kVarTypeFp32    */ { kRegTypeFp   , 4 , C(Fp) , D(Sp)            , "fp"  },
  /* 11: kVarTypeFp64    */ { kRegTypeFp   , 8 , C(Fp) , D(Dp)            , "fp"  },
  /* 12: kVarTypeMm      */ { kRegTypeMm   , 8 , C(Mm) , 0                , "mm"  },
  /* 13: kVarTypeXmm     */ { kRegTypeXmm  , 16, C(Xyz), 0                , "xmm" },
  /* 14: kVarTypeXmmSs   */ { kRegTypeXmm  , 4 , C(Xyz), D(Sp)            , "xmm" },
  /* 15: kVarTypeXmmPs   */ { kRegTypeXmm  , 16, C(Xyz), D(Sp) | D(Packed), "xmm" },
  /* 16: kVarTypeXmmSd   */ { kRegTypeXmm  , 8 , C(Xyz), D(Dp)            , "xmm" },
  /* 17: kVarTypeXmmPd   */ { kRegTypeXmm  , 16, C(Xyz), D(Dp) | D(Packed), "xmm" },
  /* 18: kVarTypeYmm     */ { kRegTypeYmm  , 32, C(Xyz), 0                , "ymm" },
  /* 19: kVarTypeYmmPs   */ { kRegTypeYmm  , 32, C(Xyz), D(Sp) | D(Packed), "ymm" },
  /* 20: kVarTypeYmmPd   */ { kRegTypeYmm  , 32, C(Xyz), D(Dp) | D(Packed), "ymm" }
};

#undef D
#undef C

// ============================================================================
// [asmjit::x86x64::Registers]
// ============================================================================

const GpReg noGpReg(kInvalidReg, kInvalidReg, 0);

const GpReg al(kRegTypeGpbLo, kRegIndexAx, 1);
const GpReg cl(kRegTypeGpbLo, kRegIndexCx, 1);
const GpReg dl(kRegTypeGpbLo, kRegIndexDx, 1);
const GpReg bl(kRegTypeGpbLo, kRegIndexBx, 1);
const GpReg spl(kRegTypeGpbLo, kRegIndexSp, 1);
const GpReg bpl(kRegTypeGpbLo, kRegIndexBp, 1);
const GpReg sil(kRegTypeGpbLo, kRegIndexSi, 1);
const GpReg dil(kRegTypeGpbLo, kRegIndexDi, 1);
const GpReg r8b(kRegTypeGpbLo, 8, 1);
const GpReg r9b(kRegTypeGpbLo, 9, 1);
const GpReg r10b(kRegTypeGpbLo, 10, 1);
const GpReg r11b(kRegTypeGpbLo, 11, 1);
const GpReg r12b(kRegTypeGpbLo, 12, 1);
const GpReg r13b(kRegTypeGpbLo, 13, 1);
const GpReg r14b(kRegTypeGpbLo, 14, 1);
const GpReg r15b(kRegTypeGpbLo, 15, 1);

const GpReg ah(kRegTypeGpbHi, kRegIndexAx, 1);
const GpReg ch(kRegTypeGpbHi, kRegIndexCx, 1);
const GpReg dh(kRegTypeGpbHi, kRegIndexDx, 1);
const GpReg bh(kRegTypeGpbHi, kRegIndexBx, 1);

const GpReg ax(kRegTypeGpw, kRegIndexAx, 2);
const GpReg cx(kRegTypeGpw, kRegIndexCx, 2);
const GpReg dx(kRegTypeGpw, kRegIndexDx, 2);
const GpReg bx(kRegTypeGpw, kRegIndexBx, 2);
const GpReg sp(kRegTypeGpw, kRegIndexSp, 2);
const GpReg bp(kRegTypeGpw, kRegIndexBp, 2);
const GpReg si(kRegTypeGpw, kRegIndexSi, 2);
const GpReg di(kRegTypeGpw, kRegIndexDi, 2);
const GpReg r8w(kRegTypeGpw, 8, 2);
const GpReg r9w(kRegTypeGpw, 9, 2);
const GpReg r10w(kRegTypeGpw, 10, 2);
const GpReg r11w(kRegTypeGpw, 11, 2);
const GpReg r12w(kRegTypeGpw, 12, 2);
const GpReg r13w(kRegTypeGpw, 13, 2);
const GpReg r14w(kRegTypeGpw, 14, 2);
const GpReg r15w(kRegTypeGpw, 15, 2);

const GpReg eax(kRegTypeGpd, kRegIndexAx, 4);
const GpReg ecx(kRegTypeGpd, kRegIndexCx, 4);
const GpReg edx(kRegTypeGpd, kRegIndexDx, 4);
const GpReg ebx(kRegTypeGpd, kRegIndexBx, 4);
const GpReg esp(kRegTypeGpd, kRegIndexSp, 4);
const GpReg ebp(kRegTypeGpd, kRegIndexBp, 4);
const GpReg esi(kRegTypeGpd, kRegIndexSi, 4);
const GpReg edi(kRegTypeGpd, kRegIndexDi, 4);
const GpReg r8d(kRegTypeGpd, 8, 4);
const GpReg r9d(kRegTypeGpd, 9, 4);
const GpReg r10d(kRegTypeGpd, 10, 4);
const GpReg r11d(kRegTypeGpd, 11, 4);
const GpReg r12d(kRegTypeGpd, 12, 4);
const GpReg r13d(kRegTypeGpd, 13, 4);
const GpReg r14d(kRegTypeGpd, 14, 4);
const GpReg r15d(kRegTypeGpd, 15, 4);

const GpReg rax(kRegTypeGpq, kRegIndexAx, 8);
const GpReg rcx(kRegTypeGpq, kRegIndexCx, 8);
const GpReg rdx(kRegTypeGpq, kRegIndexDx, 8);
const GpReg rbx(kRegTypeGpq, kRegIndexBx, 8);
const GpReg rsp(kRegTypeGpq, kRegIndexSp, 8);
const GpReg rbp(kRegTypeGpq, kRegIndexBp, 8);
const GpReg rsi(kRegTypeGpq, kRegIndexSi, 8);
const GpReg rdi(kRegTypeGpq, kRegIndexDi, 8);
const GpReg r8(kRegTypeGpq, 8, 8);
const GpReg r9(kRegTypeGpq, 9, 8);
const GpReg r10(kRegTypeGpq, 10, 8);
const GpReg r11(kRegTypeGpq, 11, 8);
const GpReg r12(kRegTypeGpq, 12, 8);
const GpReg r13(kRegTypeGpq, 13, 8);
const GpReg r14(kRegTypeGpq, 14, 8);
const GpReg r15(kRegTypeGpq, 15, 8);

const FpReg fp0(kRegTypeFp, 0, 10);
const FpReg fp1(kRegTypeFp, 1, 10);
const FpReg fp2(kRegTypeFp, 2, 10);
const FpReg fp3(kRegTypeFp, 3, 10);
const FpReg fp4(kRegTypeFp, 4, 10);
const FpReg fp5(kRegTypeFp, 5, 10);
const FpReg fp6(kRegTypeFp, 6, 10);
const FpReg fp7(kRegTypeFp, 7, 10);

const MmReg mm0(kRegTypeMm, 0, 8);
const MmReg mm1(kRegTypeMm, 1, 8);
const MmReg mm2(kRegTypeMm, 2, 8);
const MmReg mm3(kRegTypeMm, 3, 8);
const MmReg mm4(kRegTypeMm, 4, 8);
const MmReg mm5(kRegTypeMm, 5, 8);
const MmReg mm6(kRegTypeMm, 6, 8);
const MmReg mm7(kRegTypeMm, 7, 8);

const XmmReg xmm0(kRegTypeXmm, 0, 16);
const XmmReg xmm1(kRegTypeXmm, 1, 16);
const XmmReg xmm2(kRegTypeXmm, 2, 16);
const XmmReg xmm3(kRegTypeXmm, 3, 16);
const XmmReg xmm4(kRegTypeXmm, 4, 16);
const XmmReg xmm5(kRegTypeXmm, 5, 16);
const XmmReg xmm6(kRegTypeXmm, 6, 16);
const XmmReg xmm7(kRegTypeXmm, 7, 16);
const XmmReg xmm8(kRegTypeXmm, 8, 16);
const XmmReg xmm9(kRegTypeXmm, 9, 16);
const XmmReg xmm10(kRegTypeXmm, 10, 16);
const XmmReg xmm11(kRegTypeXmm, 11, 16);
const XmmReg xmm12(kRegTypeXmm, 12, 16);
const XmmReg xmm13(kRegTypeXmm, 13, 16);
const XmmReg xmm14(kRegTypeXmm, 14, 16);
const XmmReg xmm15(kRegTypeXmm, 15, 16);

const YmmReg ymm0(kRegTypeYmm, 0, 32);
const YmmReg ymm1(kRegTypeYmm, 1, 32);
const YmmReg ymm2(kRegTypeYmm, 2, 32);
const YmmReg ymm3(kRegTypeYmm, 3, 32);
const YmmReg ymm4(kRegTypeYmm, 4, 32);
const YmmReg ymm5(kRegTypeYmm, 5, 32);
const YmmReg ymm6(kRegTypeYmm, 6, 32);
const YmmReg ymm7(kRegTypeYmm, 7, 32);
const YmmReg ymm8(kRegTypeYmm, 8, 32);
const YmmReg ymm9(kRegTypeYmm, 9, 32);
const YmmReg ymm10(kRegTypeYmm, 10, 32);
const YmmReg ymm11(kRegTypeYmm, 11, 32);
const YmmReg ymm12(kRegTypeYmm, 12, 32);
const YmmReg ymm13(kRegTypeYmm, 13, 32);
const YmmReg ymm14(kRegTypeYmm, 14, 32);
const YmmReg ymm15(kRegTypeYmm, 15, 32);

const SegReg cs(kRegTypeSeg, kSegCs, 2);
const SegReg ss(kRegTypeSeg, kSegSs, 2);
const SegReg ds(kRegTypeSeg, kSegDs, 2);
const SegReg es(kRegTypeSeg, kSegEs, 2);
const SegReg fs(kRegTypeSeg, kSegFs, 2);
const SegReg gs(kRegTypeSeg, kSegGs, 2);

// ============================================================================
// [asmjit::Mem - abs[]]
// ============================================================================

Mem ptr_abs(Ptr pAbs, int32_t disp, uint32_t size) {
  Mem m(NoInit);

  m._init_packed_op_sz_b0_b1_id(kOperandTypeMem, size, kMemTypeAbsolute, 0, kInvalidValue);
  m._vmem.index = kInvalidValue;
  m._vmem.displacement = static_cast<int32_t>((intptr_t)(pAbs + disp));

  return m;
}

Mem ptr_abs(Ptr pAbs, const X86Reg& index, uint32_t shift, int32_t disp, uint32_t size) {
  Mem m(NoInit);
  uint32_t flags = shift << kMemShiftIndex;

  if (index.isGp()) flags |= Mem::_getGpdFlags(reinterpret_cast<const BaseVar&>(index));
  if (index.isXmm()) flags |= kMemVSibXmm << kMemVSibIndex;
  if (index.isYmm()) flags |= kMemVSibYmm << kMemVSibIndex;

  m._init_packed_op_sz_b0_b1_id(kOperandTypeMem, size, kMemTypeAbsolute, flags, kInvalidValue);
  m._vmem.index = index.getRegIndex();
  m._vmem.displacement = static_cast<int32_t>((intptr_t)(pAbs + disp));

  return m;
}

Mem ptr_abs(Ptr pAbs, const X86Var& index, uint32_t shift, int32_t disp, uint32_t size) {
  Mem m(NoInit);
  uint32_t flags = shift << kMemShiftIndex;

  if (index.isGp()) flags |= Mem::_getGpdFlags(reinterpret_cast<const BaseVar&>(index));
  if (index.isXmm()) flags |= kMemVSibXmm << kMemVSibIndex;
  if (index.isYmm()) flags |= kMemVSibYmm << kMemVSibIndex;

  m._init_packed_op_sz_b0_b1_id(kOperandTypeMem, size, kMemTypeAbsolute, flags, kInvalidValue);
  m._vmem.index = index.getId();
  m._vmem.displacement = static_cast<int32_t>((intptr_t)(pAbs + disp));

  return m;
}

} // x86x64 namespace
} // asmjit namespace

// ============================================================================
// [asmjit::x86]
// ============================================================================

#if defined(ASMJIT_BUILD_X86)

namespace asmjit {
namespace x86 {

const uint8_t _varMapping[kVarTypeCount] = {
  /* 00: kVarTypeInt8    */ kVarTypeInt8,
  /* 01: kVarTypeUInt8   */ kVarTypeUInt8,
  /* 02: kVarTypeInt16   */ kVarTypeInt16,
  /* 03: kVarTypeUInt16  */ kVarTypeUInt16,
  /* 04: kVarTypeInt32   */ kVarTypeInt32,
  /* 05: kVarTypeUInt32  */ kVarTypeUInt32,
  /* 06: kVarTypeInt64   */ kVarTypeInvalid, // Invalid in 32-bit mode.
  /* 07: kVarTypeUInt64  */ kVarTypeInvalid, // Invalid in 32-bit mode.
  /* 08: kVarTypeIntPtr  */ kVarTypeInt32,   // Remapped.
  /* 09: kVarTypeUIntPtr */ kVarTypeUInt32,  // Remapped.
  /* 10: kVarTypeFp32    */ kVarTypeFp32,
  /* 11: kVarTypeFp64    */ kVarTypeFp64,
  /* 12: kVarTypeMm      */ kVarTypeMm,
  /* 13: kVarTypeXmm     */ kVarTypeXmm,
  /* 14: kVarTypeXmmSs   */ kVarTypeXmmSs,
  /* 15: kVarTypeXmmPs   */ kVarTypeXmmPs,
  /* 16: kVarTypeXmmSd   */ kVarTypeXmmSd,
  /* 17: kVarTypeXmmPd   */ kVarTypeXmmPd,
  /* 18: kVarTypeYmm     */ kVarTypeYmm,
  /* 19: kVarTypeYmmPs   */ kVarTypeYmmPs,
  /* 20: kVarTypeYmmPd   */ kVarTypeYmmPd
};

const GpReg zax(kRegTypeGpd, kRegIndexAx, 4);
const GpReg zcx(kRegTypeGpd, kRegIndexCx, 4);
const GpReg zdx(kRegTypeGpd, kRegIndexDx, 4);
const GpReg zbx(kRegTypeGpd, kRegIndexBx, 4);
const GpReg zsp(kRegTypeGpd, kRegIndexSp, 4);
const GpReg zbp(kRegTypeGpd, kRegIndexBp, 4);
const GpReg zsi(kRegTypeGpd, kRegIndexSi, 4);
const GpReg zdi(kRegTypeGpd, kRegIndexDi, 4);

} // x86 namespace
} // asmjit namespace

#endif // ASMJIT_BUILD_X86

// ============================================================================
// [asmjit::x64]
// ============================================================================

#if defined(ASMJIT_BUILD_X64)
namespace asmjit {
namespace x64 {

const uint8_t _varMapping[kVarTypeCount] = {
  /* 00: kVarTypeInt8    */ kVarTypeInt8,
  /* 01: kVarTypeUInt8   */ kVarTypeUInt8,
  /* 02: kVarTypeInt16   */ kVarTypeInt16,
  /* 03: kVarTypeUInt16  */ kVarTypeUInt16,
  /* 04: kVarTypeInt32   */ kVarTypeInt32,
  /* 05: kVarTypeUInt32  */ kVarTypeUInt32,
  /* 06: kVarTypeInt64   */ kVarTypeInt64,
  /* 07: kVarTypeUInt64  */ kVarTypeUInt64,
  /* 08: kVarTypeIntPtr  */ kVarTypeInt64,   // Remapped.
  /* 09: kVarTypeUIntPtr */ kVarTypeUInt64,  // Remapped.
  /* 10: kVarTypeFp32    */ kVarTypeFp32,
  /* 11: kVarTypeFp64    */ kVarTypeFp64,
  /* 12: kVarTypeMm      */ kVarTypeMm,
  /* 13: kVarTypeXmm     */ kVarTypeXmm,
  /* 14: kVarTypeXmmSs   */ kVarTypeXmmSs,
  /* 15: kVarTypeXmmPs   */ kVarTypeXmmPs,
  /* 16: kVarTypeXmmSd   */ kVarTypeXmmSd,
  /* 17: kVarTypeXmmPd   */ kVarTypeXmmPd,
  /* 18: kVarTypeYmm     */ kVarTypeYmm,
  /* 19: kVarTypeYmmPs   */ kVarTypeYmmPs,
  /* 20: kVarTypeYmmPd   */ kVarTypeYmmPd
};

const GpReg zax(kRegTypeGpq, kRegIndexAx, 8);
const GpReg zcx(kRegTypeGpq, kRegIndexCx, 8);
const GpReg zdx(kRegTypeGpq, kRegIndexDx, 8);
const GpReg zbx(kRegTypeGpq, kRegIndexBx, 8);
const GpReg zsp(kRegTypeGpq, kRegIndexSp, 8);
const GpReg zbp(kRegTypeGpq, kRegIndexBp, 8);
const GpReg zsi(kRegTypeGpq, kRegIndexSi, 8);
const GpReg zdi(kRegTypeGpq, kRegIndexDi, 8);

} // x64 namespace
} // asmjit namespace

#endif // ASMJIT_BUILD_X64

#include "../apiend.h"

// [Guard]
#endif // ASMJIT_BUILD_X86 || ASMJIT_BUILD_X64
