// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_X86_X86DEFS_H
#define _ASMJIT_X86_X86DEFS_H

// [Dependencies - AsmJit]
#include "../base/assembler.h"
#include "../base/compiler.h"
#include "../base/defs.h"
#include "../base/globals.h"
#include "../base/intutil.h"
#include "../base/vectypes.h"

// [Api-Begin]
#include "../apibegin.h"

namespace asmjit {
namespace x86x64 {

//! @addtogroup asmjit_x86x64_codegen
//! @{

// ============================================================================
// [Forward Declarations]
// ============================================================================

struct InstInfo;
struct VarInfo;

struct X86Reg;
struct X86Var;

struct GpReg;
struct GpVar;

struct MmReg;
struct MmVar;

struct XmmReg;
struct XmmVar;

struct YmmReg;
struct YmmVar;

// ============================================================================
// [asmjit::x86x64::Typedefs]
// ============================================================================

typedef Vec64Data MmData;
typedef Vec128Data XmmData;
typedef Vec256Data YmmData;

// ============================================================================
// [asmjit::x86x64::Instruction and Condition Codes]
// ============================================================================

//! @internal
//!
//! X86/X64 instructions' names.
ASMJIT_VAR const char _instName[];

//! @internal
//!
//! X86/X64 instructions' information.
ASMJIT_VAR const InstInfo _instInfo[];

//! @internal
//!
//! X86/X64 condition codes to reversed condition codes map.
ASMJIT_VAR const uint32_t _reverseCond[20];

//! @internal
//!
//! X86X64 condition codes to "cmovcc" group map.
ASMJIT_VAR const uint32_t _condToCmovcc[20];

//! @internal
//!
//! X86X64 condition codes to "jcc" group map.
ASMJIT_VAR const uint32_t _condToJcc[20];

//! @internal
//!
//! X86X64 condition codes to "setcc" group map.
ASMJIT_VAR const uint32_t _condToSetcc[20];

// ============================================================================
// [asmjit::x86x64::Variables]
// ============================================================================

//! @internal
ASMJIT_VAR const VarInfo _varInfo[];

// ============================================================================
// [asmjit::x86x64::kRegClass]
// ============================================================================

//! X86/X64 variable class.
ASMJIT_ENUM(kRegClass) {
  // kRegClassGp defined in base/defs.h; it's used by all implementations.

  //! X86/X64 Fp register class.
  kRegClassFp = 1,
  //! X86/X64 Mm register class.
  kRegClassMm = 2,
  //! X86/X64 Xmm/Ymm register class.
  kRegClassXy = 3,

  //! Count of X86/X64 register classes.
  kRegClassCount = 4
};

// ============================================================================
// [asmjit::x86x64::kRegCount]
// ============================================================================

//! X86/X64 registers count.
ASMJIT_ENUM(kRegCount) {
  //! Count of Fp registers (8).
  kRegCountFp = 8,
  //! Count of Mm registers (8).
  kRegCountMm = 8,
  //! Count of segment registers (6).
  kRegCountSeg = 6
};

// ============================================================================
// [asmjit::x86x64::kRegType]
// ============================================================================

//! X86/X64 register types.
ASMJIT_ENUM(kRegType) {
  //! Gpb-lo register (AL, BL, CL, DL, ...).
  kRegTypeGpbLo = 0x01,
  //! Gpb-hi register (AH, BH, CH, DH only).
  kRegTypeGpbHi = 0x02,

  //! @internal
  //!
  //! Gpb-hi register patched to native index (4-7).
  kRegTypePatchedGpbHi = kRegTypeGpbLo | kRegTypeGpbHi,

  //! Gpw register.
  kRegTypeGpw = 0x10,
  //! Gpd register.
  kRegTypeGpd = 0x20,
  //! Gpq register.
  kRegTypeGpq = 0x30,

  //! Fp register.
  kRegTypeFp = 0x50,
  //! Mm register.
  kRegTypeMm = 0x60,

  //! Xmm register.
  kRegTypeXmm = 0x70,
  //! Ymm register.
  kRegTypeYmm = 0x80,
  //! Zmm register.
  kRegTypeZmm = 0x90,

  //! Segment register.
  kRegTypeSeg = 0xF0
};

// ============================================================================
// [asmjit::x86x64::kRegIndex]
// ============================================================================

//! X86/X64 register indexes.
ASMJIT_ENUM(kRegIndex) {
  //! Index of AL/AH/AX/EAX/RAX registers.
  kRegIndexAx = 0,
  //! Index of CL/CH/CX/ECX/RCX registers.
  kRegIndexCx = 1,
  //! Index of DL/DH/DX/EDX/RDX registers.
  kRegIndexDx = 2,
  //! Index of BL/BH/BX/EBX/RBX registers.
  kRegIndexBx = 3,
  //! Index of SPL/SP/ESP/RSP registers.
  kRegIndexSp = 4,
  //! Index of BPL/BP/EBP/RBP registers.
  kRegIndexBp = 5,
  //! Index of SIL/SI/ESI/RSI registers.
  kRegIndexSi = 6,
  //! Index of DIL/DI/EDI/RDI registers.
  kRegIndexDi = 7,
  //! Index of R8B/R8W/R8D/R8 registers (64-bit only).
  kRegIndexR8 = 8,
  //! Index of R9B/R9W/R9D/R9 registers (64-bit only).
  kRegIndexR9 = 9,
  //! Index of R10B/R10W/R10D/R10 registers (64-bit only).
  kRegIndexR10 = 10,
  //! Index of R11B/R11W/R11D/R11 registers (64-bit only).
  kRegIndexR11 = 11,
  //! Index of R12B/R12W/R12D/R12 registers (64-bit only).
  kRegIndexR12 = 12,
  //! Index of R13B/R13W/R13D/R13 registers (64-bit only).
  kRegIndexR13 = 13,
  //! Index of R14B/R14W/R14D/R14 registers (64-bit only).
  kRegIndexR14 = 14,
  //! Index of R15B/R15W/R15D/R15 registers (64-bit only).
  kRegIndexR15 = 15,

  //! Index of FP0 register.
  kRegIndexFp0 = 0,
  //! Index of FP1 register.
  kRegIndexFp1 = 1,
  //! Index of FP2 register.
  kRegIndexFp2 = 2,
  //! Index of FP3 register.
  kRegIndexFp3 = 3,
  //! Index of FP4 register.
  kRegIndexFp4 = 4,
  //! Index of FP5 register.
  kRegIndexFp5 = 5,
  //! Index of FP6 register.
  kRegIndexFp6 = 6,
  //! Index of FP7 register.
  kRegIndexFp7 = 7,

  //! Index of MM0 register.
  kRegIndexMm0 = 0,
  //! Index of MM1 register.
  kRegIndexMm1 = 1,
  //! Index of MM2 register.
  kRegIndexMm2 = 2,
  //! Index of MM3 register.
  kRegIndexMm3 = 3,
  //! Index of MM4 register.
  kRegIndexMm4 = 4,
  //! Index of MM5 register.
  kRegIndexMm5 = 5,
  //! Index of MM6 register.
  kRegIndexMm6 = 6,
  //! Index of MM7 register.
  kRegIndexMm7 = 7,

  //! Index of XMM0 register.
  kRegIndexXmm0 = 0,
  //! Index of XMM1 register.
  kRegIndexXmm1 = 1,
  //! Index of XMM2 register.
  kRegIndexXmm2 = 2,
  //! Index of XMM3 register.
  kRegIndexXmm3 = 3,
  //! Index of XMM4 register.
  kRegIndexXmm4 = 4,
  //! Index of XMM5 register.
  kRegIndexXmm5 = 5,
  //! Index of XMM6 register.
  kRegIndexXmm6 = 6,
  //! Index of XMM7 register.
  kRegIndexXmm7 = 7,
  //! Index of XMM8 register (64-bit only).
  kRegIndexXmm8 = 8,
  //! Index of XMM9 register (64-bit only).
  kRegIndexXmm9 = 9,
  //! Index of XMM10 register (64-bit only).
  kRegIndexXmm10 = 10,
  //! Index of XMM11 register (64-bit only).
  kRegIndexXmm11 = 11,
  //! Index of XMM12 register (64-bit only).
  kRegIndexXmm12 = 12,
  //! Index of XMM13 register (64-bit only).
  kRegIndexXmm13 = 13,
  //! Index of XMM14 register (64-bit only).
  kRegIndexXmm14 = 14,
  //! Index of XMM15 register (64-bit only).
  kRegIndexXmm15 = 15,

  //! Index of YMM0 register.
  kRegIndexYmm0 = 0,
  //! Index of YMM1 register.
  kRegIndexYmm1 = 1,
  //! Index of YMM2 register.
  kRegIndexYmm2 = 2,
  //! Index of YMM3 register.
  kRegIndexYmm3 = 3,
  //! Index of YMM4 register.
  kRegIndexYmm4 = 4,
  //! Index of YMM5 register.
  kRegIndexYmm5 = 5,
  //! Index of YMM6 register.
  kRegIndexYmm6 = 6,
  //! Index of YMM7 register.
  kRegIndexYmm7 = 7,
  //! Index of YMM8 register (64-bit only).
  kRegIndexYmm8 = 8,
  //! Index of YMM9 register (64-bit only).
  kRegIndexYmm9 = 9,
  //! Index of YMM10 register (64-bit only).
  kRegIndexYmm10 = 10,
  //! Index of YMM11 register (64-bit only).
  kRegIndexYmm11 = 11,
  //! Index of YMM12 register (64-bit only).
  kRegIndexYmm12 = 12,
  //! Index of YMM13 register (64-bit only).
  kRegIndexYmm13 = 13,
  //! Index of YMM14 register (64-bit only).
  kRegIndexYmm14 = 14,
  //! Index of YMM15 register (64-bit only).
  kRegIndexYmm15 = 15
};

// ============================================================================
// [asmjit::x86x64::kSeg]
// ============================================================================

//! X86/X64 segment codes.
ASMJIT_ENUM(kSeg) {
  //! No segment.
  kSegDefault = 0,
  //! Es segment.
  kSegEs = 1,
  //! Cs segment.
  kSegCs = 2,
  //! Ss segment.
  kSegSs = 3,
  //! Ds segment.
  kSegDs = 4,
  //! Fs segment.
  kSegFs = 5,
  //! Gs segment.
  kSegGs = 6
};

// ============================================================================
// [asmjit::x86x64::kMemVSib]
// ============================================================================

//! X86/X64 index register legacy and AVX2 (VSIB) support.
ASMJIT_ENUM(kMemVSib) {
  //! Memory operand uses Gp or no index register.
  kMemVSibGpz = 0,
  //! Memory operand uses Xmm or no index register.
  kMemVSibXmm = 1,
  //! Memory operand uses Ymm or no index register.
  kMemVSibYmm = 2
};

// ============================================================================
// [asmjit::x86x64::kMemFlags]
// ============================================================================

//! @internal
//!
//! X86/X64 specific memory flags.
ASMJIT_ENUM(kMemFlags) {
  kMemSegBits    = 0x7,
  kMemSegIndex   = 0,
  kMemSegMask    = kMemSegBits << kMemSegIndex,

  kMemGpdBits    = 0x1,
  kMemGpdIndex   = 3,
  kMemGpdMask    = kMemGpdBits << kMemGpdIndex,

  kMemVSibBits   = 0x3,
  kMemVSibIndex  = 4,
  kMemVSibMask   = kMemVSibBits << kMemVSibIndex,

  kMemShiftBits  = 0x3,
  kMemShiftIndex = 6,
  kMemShiftMask  = kMemShiftBits << kMemShiftIndex
};

// ============================================================================
// [asmjit::x86x64::kPrefetchHint]
// ============================================================================

//! X86/X64 Prefetch hints.
ASMJIT_ENUM(kPrefetchHint) {
  //! Prefetch using NT hint.
  kPrefetchNta = 0,
  //! Prefetch to L0 cache.
  kPrefetchT0 = 1,
  //! Prefetch to L1 cache.
  kPrefetchT1 = 2,
  //! Prefetch to L2 cache.
  kPrefetchT2 = 3
};

// ============================================================================
// [asmjit::x86x64::kFPSW]
// ============================================================================

//! X86/X64 FPU status Word.
ASMJIT_ENUM(kFPSW) {
  kFPSW_Invalid        = 0x0001,
  kFPSW_Denormalized   = 0x0002,
  kFPSW_DivByZero      = 0x0004,
  kFPSW_Overflow       = 0x0008,
  kFPSW_Underflow      = 0x0010,
  kFPSW_Precision      = 0x0020,
  kFPSW_StackFault     = 0x0040,
  kFPSW_Interrupt      = 0x0080,
  kFPSW_C0             = 0x0100,
  kFPSW_C1             = 0x0200,
  kFPSW_C2             = 0x0400,
  kFPSW_Top            = 0x3800,
  kFPSW_C3             = 0x4000,
  kFPSW_Busy           = 0x8000
};

// ============================================================================
// [asmjit::x86x64::kFPCW]
// ============================================================================

//! X86/X64 FPU control Word.
ASMJIT_ENUM(kFPCW) {
  kFPCW_EM_Mask        = 0x003F, // Bits 0-5.
  kFPCW_EM_Invalid     = 0x0001,
  kFPCW_EM_Denormal    = 0x0002,
  kFPCW_EM_DivByZero   = 0x0004,
  kFPCW_EM_Overflow    = 0x0008,
  kFPCW_EM_Underflow   = 0x0010,
  kFPCW_EM_Inexact     = 0x0020,

  kFPCW_PC_Mask        = 0x0300, // Bits 8-9.
  kFPCW_PC_Float       = 0x0000,
  kFPCW_PC_Reserved    = 0x0100,
  kFPCW_PC_Double      = 0x0200,
  kFPCW_PC_Extended    = 0x0300,

  kFPCW_RC_Mask        = 0x0C00, // Bits 10-11.
  kFPCW_RC_Nearest     = 0x0000,
  kFPCW_RC_Down        = 0x0400,
  kFPCW_RC_Up          = 0x0800,
  kFPCW_RC_Truncate    = 0x0C00,

  kFPCW_IC_Mask        = 0x1000, // Bit 12.
  kFPCW_IC_Projective  = 0x0000,
  kFPCW_IC_Affine      = 0x1000
};

// ============================================================================
// [asmjit::x86x64::kInstCode]
// ============================================================================

//! X86/X64 instruction codes.
//!
//! Note that these instruction codes are AsmJit specific. Each instruction has
//! a unique ID that is used as an index to AsmJit instruction table.
ASMJIT_ENUM(kInstCode) {
  kInstAdc = 1,         // X86/X64
  kInstAdd,             // X86/X64
  kInstAddpd,           // SSE2
  kInstAddps,           // SSE
  kInstAddsd,           // SSE2
  kInstAddss,           // SSE
  kInstAddsubpd,        // SSE3
  kInstAddsubps,        // SSE3
  kInstAesdec,          // AESNI
  kInstAesdeclast,      // AESNI
  kInstAesenc,          // AESNI
  kInstAesenclast,      // AESNI
  kInstAesimc,          // AESNI
  kInstAeskeygenassist, // AESNI
  kInstAnd,             // X86/X64
  kInstAndn,            // BMI
  kInstAndnpd,          // SSE2
  kInstAndnps,          // SSE
  kInstAndpd,           // SSE2
  kInstAndps,           // SSE
  kInstBextr,           // BMI
  kInstBlendpd,         // SSE4.1
  kInstBlendps,         // SSE4.1
  kInstBlendvpd,        // SSE4.1
  kInstBlendvps,        // SSE4.1
  kInstBlsi,            // BMI
  kInstBlsmsk,          // BMI
  kInstBlsr,            // BMI
  kInstBsf,             // X86/X64
  kInstBsr,             // X86/X64
  kInstBswap,           // X86/X64 (i486)
  kInstBt,              // X86/X64
  kInstBtc,             // X86/X64
  kInstBtr,             // X86/X64
  kInstBts,             // X86/X64
  kInstBzhi,            // BMI2
  kInstCall,            // X86/X64
  kInstCbw,             // X86/X64
  kInstCdq,             // X86/X64
  kInstCdqe,            // X64 only
  kInstClc,             // X86/X64
  kInstCld,             // X86/X64
  kInstClflush,         // SSE2
  kInstCmc,             // X86/X64
  kInstCmova,           // X86/X64 (cmovcc) (i586)
  kInstCmovae,          // X86/X64 (cmovcc) (i586)
  kInstCmovb,           // X86/X64 (cmovcc) (i586)
  kInstCmovbe,          // X86/X64 (cmovcc) (i586)
  kInstCmovc,           // X86/X64 (cmovcc) (i586)
  kInstCmove,           // X86/X64 (cmovcc) (i586)
  kInstCmovg,           // X86/X64 (cmovcc) (i586)
  kInstCmovge,          // X86/X64 (cmovcc) (i586)
  kInstCmovl,           // X86/X64 (cmovcc) (i586)
  kInstCmovle,          // X86/X64 (cmovcc) (i586)
  kInstCmovna,          // X86/X64 (cmovcc) (i586)
  kInstCmovnae,         // X86/X64 (cmovcc) (i586)
  kInstCmovnb,          // X86/X64 (cmovcc) (i586)
  kInstCmovnbe,         // X86/X64 (cmovcc) (i586)
  kInstCmovnc,          // X86/X64 (cmovcc) (i586)
  kInstCmovne,          // X86/X64 (cmovcc) (i586)
  kInstCmovng,          // X86/X64 (cmovcc) (i586)
  kInstCmovnge,         // X86/X64 (cmovcc) (i586)
  kInstCmovnl,          // X86/X64 (cmovcc) (i586)
  kInstCmovnle,         // X86/X64 (cmovcc) (i586)
  kInstCmovno,          // X86/X64 (cmovcc) (i586)
  kInstCmovnp,          // X86/X64 (cmovcc) (i586)
  kInstCmovns,          // X86/X64 (cmovcc) (i586)
  kInstCmovnz,          // X86/X64 (cmovcc) (i586)
  kInstCmovo,           // X86/X64 (cmovcc) (i586)
  kInstCmovp,           // X86/X64 (cmovcc) (i586)
  kInstCmovpe,          // X86/X64 (cmovcc) (i586)
  kInstCmovpo,          // X86/X64 (cmovcc) (i586)
  kInstCmovs,           // X86/X64 (cmovcc) (i586)
  kInstCmovz,           // X86/X64 (cmovcc) (i586)
  kInstCmp,             // X86/X64
  kInstCmppd,           // SSE2
  kInstCmpps,           // SSE
  kInstCmpsd,           // SSE2
  kInstCmpss,           // SSE
  kInstCmpxchg,         // X86/X64 (i486)
  kInstCmpxchg16b,      // X64 only
  kInstCmpxchg8b,       // X86/X64 (i586)
  kInstComisd,          // SSE2
  kInstComiss,          // SSE
  kInstCpuid,           // X86/X64 (i486)
  kInstCqo,             // X64 only
  kInstCrc32,           // SSE4.2
  kInstCvtdq2pd,        // SSE2
  kInstCvtdq2ps,        // SSE2
  kInstCvtpd2dq,        // SSE2
  kInstCvtpd2pi,        // SSE2
  kInstCvtpd2ps,        // SSE2
  kInstCvtpi2pd,        // SSE2
  kInstCvtpi2ps,        // SSE
  kInstCvtps2dq,        // SSE2
  kInstCvtps2pd,        // SSE2
  kInstCvtps2pi,        // SSE
  kInstCvtsd2si,        // SSE2
  kInstCvtsd2ss,        // SSE2
  kInstCvtsi2sd,        // SSE2
  kInstCvtsi2ss,        // SSE
  kInstCvtss2sd,        // SSE2
  kInstCvtss2si,        // SSE
  kInstCvttpd2dq,       // SSE2
  kInstCvttpd2pi,       // SSE2
  kInstCvttps2dq,       // SSE2
  kInstCvttps2pi,       // SSE
  kInstCvttsd2si,       // SSE2
  kInstCvttss2si,       // SSE
  kInstCwd,             // X86/X64
  kInstCwde,            // X86/X64
  kInstDaa,             // X86 only
  kInstDas,             // X86 only
  kInstDec,             // X86/X64
  kInstDiv,             // X86/X64
  kInstDivpd,           // SSE2
  kInstDivps,           // SSE
  kInstDivsd,           // SSE2
  kInstDivss,           // SSE
  kInstDppd,            // SSE4.1
  kInstDpps,            // SSE4.1
  kInstEmms,            // MMX
  kInstEnter,           // X86/X64
  kInstExtractps,       // SSE4.1
  kInstF2xm1,           // FPU
  kInstFabs,            // FPU
  kInstFadd,            // FPU
  kInstFaddp,           // FPU
  kInstFbld,            // FPU
  kInstFbstp,           // FPU
  kInstFchs,            // FPU
  kInstFclex,           // FPU
  kInstFcmovb,          // FPU
  kInstFcmovbe,         // FPU
  kInstFcmove,          // FPU
  kInstFcmovnb,         // FPU
  kInstFcmovnbe,        // FPU
  kInstFcmovne,         // FPU
  kInstFcmovnu,         // FPU
  kInstFcmovu,          // FPU
  kInstFcom,            // FPU
  kInstFcomi,           // FPU
  kInstFcomip,          // FPU
  kInstFcomp,           // FPU
  kInstFcompp,          // FPU
  kInstFcos,            // FPU
  kInstFdecstp,         // FPU
  kInstFdiv,            // FPU
  kInstFdivp,           // FPU
  kInstFdivr,           // FPU
  kInstFdivrp,          // FPU
  kInstFemms,           // 3dNow!
  kInstFfree,           // FPU
  kInstFiadd,           // FPU
  kInstFicom,           // FPU
  kInstFicomp,          // FPU
  kInstFidiv,           // FPU
  kInstFidivr,          // FPU
  kInstFild,            // FPU
  kInstFimul,           // FPU
  kInstFincstp,         // FPU
  kInstFinit,           // FPU
  kInstFist,            // FPU
  kInstFistp,           // FPU
  kInstFisttp,          // SSE3
  kInstFisub,           // FPU
  kInstFisubr,          // FPU
  kInstFld,             // FPU
  kInstFld1,            // FPU
  kInstFldcw,           // FPU
  kInstFldenv,          // FPU
  kInstFldl2e,          // FPU
  kInstFldl2t,          // FPU
  kInstFldlg2,          // FPU
  kInstFldln2,          // FPU
  kInstFldpi,           // FPU
  kInstFldz,            // FPU
  kInstFmul,            // FPU
  kInstFmulp,           // FPU
  kInstFnclex,          // FPU
  kInstFninit,          // FPU
  kInstFnop,            // FPU
  kInstFnsave,          // FPU
  kInstFnstcw,          // FPU
  kInstFnstenv,         // FPU
  kInstFnstsw,          // FPU
  kInstFpatan,          // FPU
  kInstFprem,           // FPU
  kInstFprem1,          // FPU
  kInstFptan,           // FPU
  kInstFrndint,         // FPU
  kInstFrstor,          // FPU
  kInstFsave,           // FPU
  kInstFscale,          // FPU
  kInstFsin,            // FPU
  kInstFsincos,         // FPU
  kInstFsqrt,           // FPU
  kInstFst,             // FPU
  kInstFstcw,           // FPU
  kInstFstenv,          // FPU
  kInstFstp,            // FPU
  kInstFstsw,           // FPU
  kInstFsub,            // FPU
  kInstFsubp,           // FPU
  kInstFsubr,           // FPU
  kInstFsubrp,          // FPU
  kInstFtst,            // FPU
  kInstFucom,           // FPU
  kInstFucomi,          // FPU
  kInstFucomip,         // FPU
  kInstFucomp,          // FPU
  kInstFucompp,         // FPU
  kInstFwait,           // FPU
  kInstFxam,            // FPU
  kInstFxch,            // FPU
  kInstFxrstor,         // FPU
  kInstFxsave,          // FPU
  kInstFxtract,         // FPU
  kInstFyl2x,           // FPU
  kInstFyl2xp1,         // FPU
  kInstHaddpd,          // SSE3
  kInstHaddps,          // SSE3
  kInstHsubpd,          // SSE3
  kInstHsubps,          // SSE3
  kInstIdiv,            // X86/X64
  kInstImul,            // X86/X64
  kInstInc,             // X86/X64
  kInstInsertps,        // SSE4.1
  kInstInt,             // X86/X64
  kInstJa,              // X86/X64 (jcc)
  kInstJae,             // X86/X64 (jcc)
  kInstJb,              // X86/X64 (jcc)
  kInstJbe,             // X86/X64 (jcc)
  kInstJc,              // X86/X64 (jcc)
  kInstJe,              // X86/X64 (jcc)
  kInstJg,              // X86/X64 (jcc)
  kInstJge,             // X86/X64 (jcc)
  kInstJl,              // X86/X64 (jcc)
  kInstJle,             // X86/X64 (jcc)
  kInstJna,             // X86/X64 (jcc)
  kInstJnae,            // X86/X64 (jcc)
  kInstJnb,             // X86/X64 (jcc)
  kInstJnbe,            // X86/X64 (jcc)
  kInstJnc,             // X86/X64 (jcc)
  kInstJne,             // X86/X64 (jcc)
  kInstJng,             // X86/X64 (jcc)
  kInstJnge,            // X86/X64 (jcc)
  kInstJnl,             // X86/X64 (jcc)
  kInstJnle,            // X86/X64 (jcc)
  kInstJno,             // X86/X64 (jcc)
  kInstJnp,             // X86/X64 (jcc)
  kInstJns,             // X86/X64 (jcc)
  kInstJnz,             // X86/X64 (jcc)
  kInstJo,              // X86/X64 (jcc)
  kInstJp,              // X86/X64 (jcc)
  kInstJpe,             // X86/X64 (jcc)
  kInstJpo,             // X86/X64 (jcc)
  kInstJs,              // X86/X64 (jcc)
  kInstJz,              // X86/X64 (jcc)
  kInstJmp,             // X86/X64 (jmp)
  kInstLahf,            // X86/X64 (CPUID NEEDED)
  kInstLddqu,           // SSE3
  kInstLdmxcsr,         // SSE
  kInstLea,             // X86/X64
  kInstLeave,           // X86/X64
  kInstLfence,          // SSE2
  kInstLzcnt,           // LZCNT
  kInstMaskmovdqu,      // SSE2
  kInstMaskmovq,        // MMX-Ext
  kInstMaxpd,           // SSE2
  kInstMaxps,           // SSE
  kInstMaxsd,           // SSE2
  kInstMaxss,           // SSE
  kInstMfence,          // SSE2
  kInstMinpd,           // SSE2
  kInstMinps,           // SSE
  kInstMinsd,           // SSE2
  kInstMinss,           // SSE
  kInstMonitor,         // SSE3
  kInstMov,             // X86/X64
  kInstMovapd,          // SSE2
  kInstMovaps,          // SSE
  kInstMovbe,           // SSE3 - Intel-Atom
  kInstMovd,            // MMX/SSE2
  kInstMovddup,         // SSE3
  kInstMovdq2q,         // SSE2
  kInstMovdqa,          // SSE2
  kInstMovdqu,          // SSE2
  kInstMovhlps,         // SSE
  kInstMovhpd,          // SSE2
  kInstMovhps,          // SSE
  kInstMovlhps,         // SSE
  kInstMovlpd,          // SSE2
  kInstMovlps,          // SSE
  kInstMovmskpd,        // SSE2
  kInstMovmskps,        // SSE2
  kInstMovntdq,         // SSE2
  kInstMovntdqa,        // SSE4.1
  kInstMovnti,          // SSE2
  kInstMovntpd,         // SSE2
  kInstMovntps,         // SSE
  kInstMovntq,          // MMX-Ext
  kInstMovptr,          // X86/X64
  kInstMovq,            // MMX/SSE/SSE2
  kInstMovq2dq,         // SSE2
  kInstMovsd,           // SSE2
  kInstMovshdup,        // SSE3
  kInstMovsldup,        // SSE3
  kInstMovss,           // SSE
  kInstMovsx,           // X86/X64
  kInstMovsxd,          // X86/X64
  kInstMovupd,          // SSE2
  kInstMovups,          // SSE
  kInstMovzx,           // X86/X64
  kInstMpsadbw,         // SSE4.1
  kInstMul,             // X86/X64
  kInstMulpd,           // SSE2
  kInstMulps,           // SSE
  kInstMulsd,           // SSE2
  kInstMulss,           // SSE
  kInstMulx,            // BMI2
  kInstMwait,           // SSE3
  kInstNeg,             // X86/X64
  kInstNop,             // X86/X64
  kInstNot,             // X86/X64
  kInstOr,              // X86/X64
  kInstOrpd,            // SSE2
  kInstOrps,            // SSE
  kInstPabsb,           // SSSE3
  kInstPabsd,           // SSSE3
  kInstPabsw,           // SSSE3
  kInstPackssdw,        // MMX/SSE2
  kInstPacksswb,        // MMX/SSE2
  kInstPackusdw,        // SSE4.1
  kInstPackuswb,        // MMX/SSE2
  kInstPaddb,           // MMX/SSE2
  kInstPaddd,           // MMX/SSE2
  kInstPaddq,           // SSE2
  kInstPaddsb,          // MMX/SSE2
  kInstPaddsw,          // MMX/SSE2
  kInstPaddusb,         // MMX/SSE2
  kInstPaddusw,         // MMX/SSE2
  kInstPaddw,           // MMX/SSE2
  kInstPalignr,         // SSSE3
  kInstPand,            // MMX/SSE2
  kInstPandn,           // MMX/SSE2
  kInstPause,           // SSE2.
  kInstPavgb,           // MMX-Ext
  kInstPavgw,           // MMX-Ext
  kInstPblendvb,        // SSE4.1
  kInstPblendw,         // SSE4.1
  kInstPclmulqdq,       // PCLMULQDQ
  kInstPcmpeqb,         // MMX/SSE2
  kInstPcmpeqd,         // MMX/SSE2
  kInstPcmpeqq,         // SSE4.1
  kInstPcmpeqw,         // MMX/SSE2
  kInstPcmpestri,       // SSE4.2
  kInstPcmpestrm,       // SSE4.2
  kInstPcmpgtb,         // MMX/SSE2
  kInstPcmpgtd,         // MMX/SSE2
  kInstPcmpgtq,         // SSE4.2
  kInstPcmpgtw,         // MMX/SSE2
  kInstPcmpistri,       // SSE4.2
  kInstPcmpistrm,       // SSE4.2
  kInstPdep,            // BMI2
  kInstPext,            // BMI2
  kInstPextrb,          // SSE4.1
  kInstPextrd,          // SSE4.1
  kInstPextrq,          // SSE4.1
  kInstPextrw,          // MMX-Ext/SSE2
  kInstPf2id,           // 3dNow!
  kInstPf2iw,           // Enhanced 3dNow!
  kInstPfacc,           // 3dNow!
  kInstPfadd,           // 3dNow!
  kInstPfcmpeq,         // 3dNow!
  kInstPfcmpge,         // 3dNow!
  kInstPfcmpgt,         // 3dNow!
  kInstPfmax,           // 3dNow!
  kInstPfmin,           // 3dNow!
  kInstPfmul,           // 3dNow!
  kInstPfnacc,          // Enhanced 3dNow!
  kInstPfpnacc,         // Enhanced 3dNow!
  kInstPfrcp,           // 3dNow!
  kInstPfrcpit1,        // 3dNow!
  kInstPfrcpit2,        // 3dNow!
  kInstPfrsqit1,        // 3dNow!
  kInstPfrsqrt,         // 3dNow!
  kInstPfsub,           // 3dNow!
  kInstPfsubr,          // 3dNow!
  kInstPhaddd,          // SSSE3
  kInstPhaddsw,         // SSSE3
  kInstPhaddw,          // SSSE3
  kInstPhminposuw,      // SSE4.1
  kInstPhsubd,          // SSSE3
  kInstPhsubsw,         // SSSE3
  kInstPhsubw,          // SSSE3
  kInstPi2fd,           // 3dNow!
  kInstPi2fw,           // Enhanced 3dNow!
  kInstPinsrb,          // SSE4.1
  kInstPinsrd,          // SSE4.1
  kInstPinsrq,          // SSE4.1
  kInstPinsrw,          // MMX-Ext
  kInstPmaddubsw,       // SSSE3
  kInstPmaddwd,         // MMX/SSE2
  kInstPmaxsb,          // SSE4.1
  kInstPmaxsd,          // SSE4.1
  kInstPmaxsw,          // MMX-Ext
  kInstPmaxub,          // MMX-Ext
  kInstPmaxud,          // SSE4.1
  kInstPmaxuw,          // SSE4.1
  kInstPminsb,          // SSE4.1
  kInstPminsd,          // SSE4.1
  kInstPminsw,          // MMX-Ext
  kInstPminub,          // MMX-Ext
  kInstPminud,          // SSE4.1
  kInstPminuw,          // SSE4.1
  kInstPmovmskb,        // MMX-Ext
  kInstPmovsxbd,        // SSE4.1
  kInstPmovsxbq,        // SSE4.1
  kInstPmovsxbw,        // SSE4.1
  kInstPmovsxdq,        // SSE4.1
  kInstPmovsxwd,        // SSE4.1
  kInstPmovsxwq,        // SSE4.1
  kInstPmovzxbd,        // SSE4.1
  kInstPmovzxbq,        // SSE4.1
  kInstPmovzxbw,        // SSE4.1
  kInstPmovzxdq,        // SSE4.1
  kInstPmovzxwd,        // SSE4.1
  kInstPmovzxwq,        // SSE4.1
  kInstPmuldq,          // SSE4.1
  kInstPmulhrsw,        // SSSE3
  kInstPmulhuw,         // MMX-Ext
  kInstPmulhw,          // MMX/SSE2
  kInstPmulld,          // SSE4.1
  kInstPmullw,          // MMX/SSE2
  kInstPmuludq,         // SSE2
  kInstPop,             // X86/X64
  kInstPopa,            // X86 only
  kInstPopcnt,          // SSE4.2
  kInstPopf,            // X86/X64
  kInstPor,             // MMX/SSE2
  kInstPrefetch,        // MMX-Ext/SSE
  kInstPrefetch3dNow,   // 3dNow!
  kInstPrefetchw3dNow,  // 3dNow!
  kInstPsadbw,          // MMX-Ext
  kInstPshufb,          // SSSE3
  kInstPshufd,          // SSE2
  kInstPshufhw,         // SSE2
  kInstPshuflw,         // SSE2
  kInstPshufw,          // MMX-Ext
  kInstPsignb,          // SSSE3
  kInstPsignd,          // SSSE3
  kInstPsignw,          // SSSE3
  kInstPslld,           // MMX/SSE2
  kInstPslldq,          // SSE2
  kInstPsllq,           // MMX/SSE2
  kInstPsllw,           // MMX/SSE2
  kInstPsrad,           // MMX/SSE2
  kInstPsraw,           // MMX/SSE2
  kInstPsrld,           // MMX/SSE2
  kInstPsrldq,          // SSE2
  kInstPsrlq,           // MMX/SSE2
  kInstPsrlw,           // MMX/SSE2
  kInstPsubb,           // MMX/SSE2
  kInstPsubd,           // MMX/SSE2
  kInstPsubq,           // SSE2
  kInstPsubsb,          // MMX/SSE2
  kInstPsubsw,          // MMX/SSE2
  kInstPsubusb,         // MMX/SSE2
  kInstPsubusw,         // MMX/SSE2
  kInstPsubw,           // MMX/SSE2
  kInstPswapd,          // Enhanced 3dNow!
  kInstPtest,           // SSE4.1
  kInstPunpckhbw,       // MMX/SSE2
  kInstPunpckhdq,       // MMX/SSE2
  kInstPunpckhqdq,      // SSE2
  kInstPunpckhwd,       // MMX/SSE2
  kInstPunpcklbw,       // MMX/SSE2
  kInstPunpckldq,       // MMX/SSE2
  kInstPunpcklqdq,      // SSE2
  kInstPunpcklwd,       // MMX/SSE2
  kInstPush,            // X86/X64
  kInstPusha,           // X86 only
  kInstPushf,           // X86/X64
  kInstPxor,            // MMX/SSE2
  kInstRcl,             // X86/X64
  kInstRcpps,           // SSE
  kInstRcpss,           // SSE
  kInstRcr,             // X86/X64
  kInstRdfsbase,        // FSGSBASE (x64)
  kInstRdgsbase,        // FSGSBASE (x64)
  kInstRdrand,          // RDRAND
  kInstRdtsc,           // X86/X64
  kInstRdtscp,          // X86/X64
  kInstRepLodsb,        // X86/X64 (REP)
  kInstRepLodsd,        // X86/X64 (REP)
  kInstRepLodsq,        // X64 only (REP)
  kInstRepLodsw,        // X86/X64 (REP)
  kInstRepMovsb,        // X86/X64 (REP)
  kInstRepMovsd,        // X86/X64 (REP)
  kInstRepMovsq,        // X64 only (REP)
  kInstRepMovsw,        // X86/X64 (REP)
  kInstRepStosb,        // X86/X64 (REP)
  kInstRepStosd,        // X86/X64 (REP)
  kInstRepStosq,        // X64 only (REP)
  kInstRepStosw,        // X86/X64 (REP)
  kInstRepeCmpsb,       // X86/X64 (REP)
  kInstRepeCmpsd,       // X86/X64 (REP)
  kInstRepeCmpsq,       // X64 only (REP)
  kInstRepeCmpsw,       // X86/X64 (REP)
  kInstRepeScasb,       // X86/X64 (REP)
  kInstRepeScasd,       // X86/X64 (REP)
  kInstRepeScasq,       // X64 only (REP)
  kInstRepeScasw,       // X86/X64 (REP)
  kInstRepneCmpsb,      // X86/X64 (REP)
  kInstRepneCmpsd,      // X86/X64 (REP)
  kInstRepneCmpsq,      // X64 only (REP)
  kInstRepneCmpsw,      // X86/X64 (REP)
  kInstRepneScasb,      // X86/X64 (REP)
  kInstRepneScasd,      // X86/X64 (REP)
  kInstRepneScasq,      // X64 only (REP)
  kInstRepneScasw,      // X86/X64 (REP)
  kInstRet,             // X86/X64
  kInstRol,             // X86/X64
  kInstRor,             // X86/X64
  kInstRorx,            // BMI2
  kInstRoundpd,         // SSE4.1
  kInstRoundps,         // SSE4.1
  kInstRoundsd,         // SSE4.1
  kInstRoundss,         // SSE4.1
  kInstRsqrtps,         // SSE
  kInstRsqrtss,         // SSE
  kInstSahf,            // X86/X64 (CPUID NEEDED)
  kInstSal,             // X86/X64
  kInstSar,             // X86/X64
  kInstSarx,            // BMI2
  kInstSbb,             // X86/X64
  kInstSeta,            // X86/X64 (setcc)
  kInstSetae,           // X86/X64 (setcc)
  kInstSetb,            // X86/X64 (setcc)
  kInstSetbe,           // X86/X64 (setcc)
  kInstSetc,            // X86/X64 (setcc)
  kInstSete,            // X86/X64 (setcc)
  kInstSetg,            // X86/X64 (setcc)
  kInstSetge,           // X86/X64 (setcc)
  kInstSetl,            // X86/X64 (setcc)
  kInstSetle,           // X86/X64 (setcc)
  kInstSetna,           // X86/X64 (setcc)
  kInstSetnae,          // X86/X64 (setcc)
  kInstSetnb,           // X86/X64 (setcc)
  kInstSetnbe,          // X86/X64 (setcc)
  kInstSetnc,           // X86/X64 (setcc)
  kInstSetne,           // X86/X64 (setcc)
  kInstSetng,           // X86/X64 (setcc)
  kInstSetnge,          // X86/X64 (setcc)
  kInstSetnl,           // X86/X64 (setcc)
  kInstSetnle,          // X86/X64 (setcc)
  kInstSetno,           // X86/X64 (setcc)
  kInstSetnp,           // X86/X64 (setcc)
  kInstSetns,           // X86/X64 (setcc)
  kInstSetnz,           // X86/X64 (setcc)
  kInstSeto,            // X86/X64 (setcc)
  kInstSetp,            // X86/X64 (setcc)
  kInstSetpe,           // X86/X64 (setcc)
  kInstSetpo,           // X86/X64 (setcc)
  kInstSets,            // X86/X64 (setcc)
  kInstSetz,            // X86/X64 (setcc)
  kInstSfence,          // MMX-Ext/SSE
  kInstShl,             // X86/X64
  kInstShld,            // X86/X64
  kInstShlx,            // BMI2
  kInstShr,             // X86/X64
  kInstShrd,            // X86/X64
  kInstShrx,            // BMI2
  kInstShufpd,          // SSE2
  kInstShufps,          // SSE
  kInstSqrtpd,          // SSE2
  kInstSqrtps,          // SSE
  kInstSqrtsd,          // SSE2
  kInstSqrtss,          // SSE
  kInstStc,             // X86/X64
  kInstStd,             // X86/X64
  kInstStmxcsr,         // SSE
  kInstSub,             // X86/X64
  kInstSubpd,           // SSE2
  kInstSubps,           // SSE
  kInstSubsd,           // SSE2
  kInstSubss,           // SSE
  kInstTest,            // X86/X64
  kInstTzcnt,           // TZCNT
  kInstUcomisd,         // SSE2
  kInstUcomiss,         // SSE
  kInstUd2,             // X86/X64
  kInstUnpckhpd,        // SSE2
  kInstUnpckhps,        // SSE
  kInstUnpcklpd,        // SSE2
  kInstUnpcklps,        // SSE
  kInstVaddpd,          // AVX
  kInstVaddps,          // AVX
  kInstVaddsd,          // AVX
  kInstVaddss,          // AVX
  kInstVaddsubpd,       // AVX
  kInstVaddsubps,       // AVX
  kInstVaesdec,         // AVX+AESNI
  kInstVaesdeclast,     // AVX+AESNI
  kInstVaesenc,         // AVX+AESNI
  kInstVaesenclast,     // AVX+AESNI
  kInstVaesimc,         // AVX+AESNI
  kInstVaeskeygenassist,// AVX+AESNI
  kInstVandnpd,         // AVX
  kInstVandnps,         // AVX
  kInstVandpd,          // AVX
  kInstVandps,          // AVX
  kInstVblendpd,        // AVX
  kInstVblendps,        // AVX
  kInstVblendvpd,       // AVX
  kInstVblendvps,       // AVX
  kInstVbroadcastf128,  // AVX
  kInstVbroadcasti128,  // AVX2
  kInstVbroadcastsd,    // AVX/AVX2
  kInstVbroadcastss,    // AVX/AVX2
  kInstVcmppd,          // AVX
  kInstVcmpps,          // AVX
  kInstVcmpsd,          // AVX
  kInstVcmpss,          // AVX
  kInstVcomisd,         // AVX
  kInstVcomiss,         // AVX
  kInstVcvtdq2pd,       // AVX
  kInstVcvtdq2ps,       // AVX
  kInstVcvtpd2dq,       // AVX
  kInstVcvtpd2ps,       // AVX
  kInstVcvtph2ps,       // F16C
  kInstVcvtps2dq,       // AVX
  kInstVcvtps2pd,       // AVX
  kInstVcvtps2ph,       // F16C
  kInstVcvtsd2si,       // AVX
  kInstVcvtsd2ss,       // AVX
  kInstVcvtsi2sd,       // AVX
  kInstVcvtsi2ss,       // AVX
  kInstVcvtss2sd,       // AVX
  kInstVcvtss2si,       // AVX
  kInstVcvttpd2dq,      // AVX
  kInstVcvttps2dq,      // AVX
  kInstVcvttsd2si,      // AVX
  kInstVcvttss2si,      // AVX
  kInstVdivpd,          // AVX
  kInstVdivps,          // AVX
  kInstVdivsd,          // AVX
  kInstVdivss,          // AVX
  kInstVdppd,           // AVX
  kInstVdpps,           // AVX
  kInstVextractf128,    // AVX
  kInstVextracti128,    // AVX2
  kInstVextractps,      // AVX
  kInstVfmadd132pd,     // FMA3
  kInstVfmadd132ps,     // FMA3
  kInstVfmadd132sd,     // FMA3
  kInstVfmadd132ss,     // FMA3
  kInstVfmadd213pd,     // FMA3
  kInstVfmadd213ps,     // FMA3
  kInstVfmadd213sd,     // FMA3
  kInstVfmadd213ss,     // FMA3
  kInstVfmadd231pd,     // FMA3
  kInstVfmadd231ps,     // FMA3
  kInstVfmadd231sd,     // FMA3
  kInstVfmadd231ss,     // FMA3
  kInstVfmaddpd,        // FMA4
  kInstVfmaddps,        // FMA4
  kInstVfmaddsd,        // FMA4
  kInstVfmaddss,        // FMA4
  kInstVfmaddsubpd,     // FMA4
  kInstVfmaddsubps,     // FMA4
  kInstVfmaddsub132pd,  // FMA3
  kInstVfmaddsub132ps,  // FMA3
  kInstVfmaddsub213pd,  // FMA3
  kInstVfmaddsub213ps,  // FMA3
  kInstVfmaddsub231pd,  // FMA3
  kInstVfmaddsub231ps,  // FMA3
  kInstVfmsub132pd,     // FMA3
  kInstVfmsub132ps,     // FMA3
  kInstVfmsub132sd,     // FMA3
  kInstVfmsub132ss,     // FMA3
  kInstVfmsub213pd,     // FMA3
  kInstVfmsub213ps,     // FMA3
  kInstVfmsub213sd,     // FMA3
  kInstVfmsub213ss,     // FMA3
  kInstVfmsub231pd,     // FMA3
  kInstVfmsub231ps,     // FMA3
  kInstVfmsub231sd,     // FMA3
  kInstVfmsub231ss,     // FMA3
  kInstVfmsubadd132pd,  // FMA3
  kInstVfmsubadd132ps,  // FMA3
  kInstVfmsubadd213pd,  // FMA3
  kInstVfmsubadd213ps,  // FMA3
  kInstVfmsubadd231pd,  // FMA3
  kInstVfmsubadd231ps,  // FMA3
  kInstVfmsubaddpd,     // FMA4
  kInstVfmsubaddps,     // FMA4
  kInstVfmsubpd,        // FMA4
  kInstVfmsubps,        // FMA4
  kInstVfmsubsd,        // FMA4
  kInstVfmsubss,        // FMA4
  kInstVfnmadd132pd,    // FMA3
  kInstVfnmadd132ps,    // FMA3
  kInstVfnmadd132sd,    // FMA3
  kInstVfnmadd132ss,    // FMA3
  kInstVfnmadd213pd,    // FMA3
  kInstVfnmadd213ps,    // FMA3
  kInstVfnmadd213sd,    // FMA3
  kInstVfnmadd213ss,    // FMA3
  kInstVfnmadd231pd,    // FMA3
  kInstVfnmadd231ps,    // FMA3
  kInstVfnmadd231sd,    // FMA3
  kInstVfnmadd231ss,    // FMA3
  kInstVfnmaddpd,       // FMA4
  kInstVfnmaddps,       // FMA4
  kInstVfnmaddsd,       // FMA4
  kInstVfnmaddss,       // FMA4
  kInstVfnmsub132pd,    // FMA3
  kInstVfnmsub132ps,    // FMA3
  kInstVfnmsub132sd,    // FMA3
  kInstVfnmsub132ss,    // FMA3
  kInstVfnmsub213pd,    // FMA3
  kInstVfnmsub213ps,    // FMA3
  kInstVfnmsub213sd,    // FMA3
  kInstVfnmsub213ss,    // FMA3
  kInstVfnmsub231pd,    // FMA3
  kInstVfnmsub231ps,    // FMA3
  kInstVfnmsub231sd,    // FMA3
  kInstVfnmsub231ss,    // FMA3
  kInstVfnmsubpd,       // FMA4
  kInstVfnmsubps,       // FMA4
  kInstVfnmsubsd,       // FMA4
  kInstVfnmsubss,       // FMA4
  kInstVfrczpd,         // XOP
  kInstVfrczps,         // XOP
  kInstVfrczsd,         // XOP
  kInstVfrczss,         // XOP
  kInstVgatherdpd,      // AVX2
  kInstVgatherdps,      // AVX2
  kInstVgatherqpd,      // AVX2
  kInstVgatherqps,      // AVX2
  kInstVhaddpd,         // AVX
  kInstVhaddps,         // AVX
  kInstVhsubpd,         // AVX
  kInstVhsubps,         // AVX
  kInstVinsertf128,     // AVX
  kInstVinserti128,     // AVX2
  kInstVinsertps,       // AVX
  kInstVlddqu,          // AVX
  kInstVldmxcsr,        // AVX
  kInstVmaskmovdqu,     // AVX
  kInstVmaskmovpd,      // AVX
  kInstVmaskmovps,      // AVX
  kInstVmaxpd,          // AVX
  kInstVmaxps,          // AVX
  kInstVmaxsd,          // AVX
  kInstVmaxss,          // AVX
  kInstVminpd,          // AVX
  kInstVminps,          // AVX
  kInstVminsd,          // AVX
  kInstVminss,          // AVX
  kInstVmovapd,         // AVX
  kInstVmovaps,         // AVX
  kInstVmovd,           // AVX
  kInstVmovddup,        // AVX
  kInstVmovdqa,         // AVX
  kInstVmovdqu,         // AVX
  kInstVmovhlps,        // AVX
  kInstVmovhpd,         // AVX
  kInstVmovhps,         // AVX
  kInstVmovlhps,        // AVX
  kInstVmovlpd,         // AVX
  kInstVmovlps,         // AVX
  kInstVmovmskpd,       // AVX
  kInstVmovmskps,       // AVX
  kInstVmovntdq,        // AVX
  kInstVmovntdqa,       // AVX/AVX2
  kInstVmovntpd,        // AVX
  kInstVmovntps,        // AVX
  kInstVmovq,           // AVX
  kInstVmovsd,          // AVX
  kInstVmovshdup,       // AVX
  kInstVmovsldup,       // AVX
  kInstVmovss,          // AVX
  kInstVmovupd,         // AVX
  kInstVmovups,         // AVX
  kInstVmpsadbw,        // AVX/AVX2
  kInstVmulpd,          // AVX
  kInstVmulps,          // AVX
  kInstVmulsd,          // AVX
  kInstVmulss,          // AVX
  kInstVorpd,           // AVX
  kInstVorps,           // AVX
  kInstVpabsb,          // AVX2
  kInstVpabsd,          // AVX2
  kInstVpabsw,          // AVX2
  kInstVpackssdw,       // AVX2
  kInstVpacksswb,       // AVX2
  kInstVpackusdw,       // AVX2
  kInstVpackuswb,       // AVX2
  kInstVpaddb,          // AVX2
  kInstVpaddd,          // AVX2
  kInstVpaddq,          // AVX2
  kInstVpaddsb,         // AVX2
  kInstVpaddsw,         // AVX2
  kInstVpaddusb,        // AVX2
  kInstVpaddusw,        // AVX2
  kInstVpaddw,          // AVX2
  kInstVpalignr,        // AVX2
  kInstVpand,           // AVX2
  kInstVpandn,          // AVX2
  kInstVpavgb,          // AVX2
  kInstVpavgw,          // AVX2
  kInstVpblendd,        // AVX2
  kInstVpblendvb,       // AVX2
  kInstVpblendw,        // AVX2
  kInstVpbroadcastb,    // AVX2
  kInstVpbroadcastd,    // AVX2
  kInstVpbroadcastq,    // AVX2
  kInstVpbroadcastw,    // AVX2
  kInstVpclmulqdq,      // AVX+PCLMULQDQ
  kInstVpcmov,          // XOP
  kInstVpcmpeqb,        // AVX2
  kInstVpcmpeqd,        // AVX2
  kInstVpcmpeqq,        // AVX2
  kInstVpcmpeqw,        // AVX2
  kInstVpcmpestri,      // AVX
  kInstVpcmpestrm,      // AVX
  kInstVpcmpgtb,        // AVX2
  kInstVpcmpgtd,        // AVX2
  kInstVpcmpgtq,        // AVX2
  kInstVpcmpgtw,        // AVX2
  kInstVpcmpistri,      // AVX
  kInstVpcmpistrm,      // AVX
  kInstVpcomb,          // XOP
  kInstVpcomd,          // XOP
  kInstVpcomq,          // XOP
  kInstVpcomub,         // XOP
  kInstVpcomud,         // XOP
  kInstVpcomuq,         // XOP
  kInstVpcomuw,         // XOP
  kInstVpcomw,          // XOP
  kInstVperm2f128,      // AVX
  kInstVperm2i128,      // AVX2
  kInstVpermd,          // AVX2
  kInstVpermil2pd,      // XOP
  kInstVpermil2ps,      // XOP
  kInstVpermilpd,       // AVX
  kInstVpermilps,       // AVX
  kInstVpermpd,         // AVX2
  kInstVpermps,         // AVX2
  kInstVpermq,          // AVX2
  kInstVpextrb,         // AVX
  kInstVpextrd,         // AVX
  kInstVpextrq,         // AVX (x64 only)
  kInstVpextrw,         // AVX
  kInstVpgatherdd,      // AVX2
  kInstVpgatherdq,      // AVX2
  kInstVpgatherqd,      // AVX2
  kInstVpgatherqq,      // AVX2
  kInstVphaddbd,        // XOP
  kInstVphaddbq,        // XOP
  kInstVphaddbw,        // XOP
  kInstVphaddd,         // AVX2
  kInstVphadddq,        // XOP
  kInstVphaddsw,        // AVX2
  kInstVphaddubd,       // XOP
  kInstVphaddubq,       // XOP
  kInstVphaddubw,       // XOP
  kInstVphaddudq,       // XOP
  kInstVphadduwd,       // XOP
  kInstVphadduwq,       // XOP
  kInstVphaddw,         // AVX2
  kInstVphaddwd,        // XOP
  kInstVphaddwq,        // XOP
  kInstVphminposuw,     // AVX
  kInstVphsubbw,        // XOP
  kInstVphsubdq,        // XOP
  kInstVphsubd,         // AVX2
  kInstVphsubsw,        // AVX2
  kInstVphsubw,         // AVX2
  kInstVphsubwd,        // XOP
  kInstVpinsrb,         // AVX
  kInstVpinsrd,         // AVX
  kInstVpinsrq,         // AVX (x64 only)
  kInstVpinsrw,         // AVX
  kInstVpmacsdd,        // XOP
  kInstVpmacsdqh,       // XOP
  kInstVpmacsdql,       // XOP
  kInstVpmacssdd,       // XOP
  kInstVpmacssdqh,      // XOP
  kInstVpmacssdql,      // XOP
  kInstVpmacsswd,       // XOP
  kInstVpmacssww,       // XOP
  kInstVpmacswd,        // XOP
  kInstVpmacsww,        // XOP
  kInstVpmadcsswd,      // XOP
  kInstVpmadcswd,       // XOP
  kInstVpmaddubsw,      // AVX/AVX2
  kInstVpmaddwd,        // AVX/AVX2
  kInstVpmaskmovd,      // AVX2
  kInstVpmaskmovq,      // AVX2
  kInstVpmaxsb,         // AVX/AVX2
  kInstVpmaxsd,         // AVX/AVX2
  kInstVpmaxsw,         // AVX/AVX2
  kInstVpmaxub,         // AVX/AVX2
  kInstVpmaxud,         // AVX/AVX2
  kInstVpmaxuw,         // AVX/AVX2
  kInstVpminsb,         // AVX/AVX2
  kInstVpminsd,         // AVX/AVX2
  kInstVpminsw,         // AVX/AVX2
  kInstVpminub,         // AVX/AVX2
  kInstVpminud,         // AVX/AVX2
  kInstVpminuw,         // AVX/AVX2
  kInstVpmovmskb,       // AVX/AVX2
  kInstVpmovsxbd,       // AVX/AVX2
  kInstVpmovsxbq,       // AVX/AVX2
  kInstVpmovsxbw,       // AVX/AVX2
  kInstVpmovsxdq,       // AVX/AVX2
  kInstVpmovsxwd,       // AVX/AVX2
  kInstVpmovsxwq,       // AVX/AVX2
  kInstVpmovzxbd,       // AVX/AVX2
  kInstVpmovzxbq,       // AVX/AVX2
  kInstVpmovzxbw,       // AVX/AVX2
  kInstVpmovzxdq,       // AVX/AVX2
  kInstVpmovzxwd,       // AVX/AVX2
  kInstVpmovzxwq,       // AVX/AVX2
  kInstVpmuldq,         // AVX/AVX2
  kInstVpmulhrsw,       // AVX/AVX2
  kInstVpmulhuw,        // AVX/AVX2
  kInstVpmulhw,         // AVX/AVX2
  kInstVpmulld,         // AVX/AVX2
  kInstVpmullw,         // AVX/AVX2
  kInstVpmuludq,        // AVX/AVX2
  kInstVpor,            // AVX/AVX2
  kInstVpperm,          // XOP
  kInstVprotb,          // XOP
  kInstVprotd,          // XOP
  kInstVprotq,          // XOP
  kInstVprotw,          // XOP
  kInstVpsadbw,         // AVX/AVX2
  kInstVpshab,          // XOP
  kInstVpshad,          // XOP
  kInstVpshaq,          // XOP
  kInstVpshaw,          // XOP
  kInstVpshlb,          // XOP
  kInstVpshld,          // XOP
  kInstVpshlq,          // XOP
  kInstVpshlw,          // XOP
  kInstVpshufb,         // AVX/AVX2
  kInstVpshufd,         // AVX/AVX2
  kInstVpshufhw,        // AVX/AVX2
  kInstVpshuflw,        // AVX/AVX2
  kInstVpsignb,         // AVX/AVX2
  kInstVpsignd,         // AVX/AVX2
  kInstVpsignw,         // AVX/AVX2
  kInstVpslld,          // AVX/AVX2
  kInstVpslldq,         // AVX/AVX2
  kInstVpsllq,          // AVX/AVX2
  kInstVpsllvd,         // AVX2
  kInstVpsllvq,         // AVX2
  kInstVpsllw,          // AVX/AVX2
  kInstVpsrad,          // AVX/AVX2
  kInstVpsravd,         // AVX2
  kInstVpsraw,          // AVX/AVX2
  kInstVpsrld,          // AVX/AVX2
  kInstVpsrldq,         // AVX/AVX2
  kInstVpsrlq,          // AVX/AVX2
  kInstVpsrlvd,         // AVX2
  kInstVpsrlvq,         // AVX2
  kInstVpsrlw,          // AVX/AVX2
  kInstVpsubb,          // AVX/AVX2
  kInstVpsubd,          // AVX/AVX2
  kInstVpsubq,          // AVX/AVX2
  kInstVpsubsb,         // AVX/AVX2
  kInstVpsubsw,         // AVX/AVX2
  kInstVpsubusb,        // AVX/AVX2
  kInstVpsubusw,        // AVX/AVX2
  kInstVpsubw,          // AVX/AVX2
  kInstVptest,          // AVX
  kInstVpunpckhbw,      // AVX/AVX2
  kInstVpunpckhdq,      // AVX/AVX2
  kInstVpunpckhqdq,     // AVX/AVX2
  kInstVpunpckhwd,      // AVX/AVX2
  kInstVpunpcklbw,      // AVX/AVX2
  kInstVpunpckldq,      // AVX/AVX2
  kInstVpunpcklqdq,     // AVX/AVX2
  kInstVpunpcklwd,      // AVX/AVX2
  kInstVpxor,           // AVX/AVX2
  kInstVrcpps,          // AVX
  kInstVrcpss,          // AVX
  kInstVroundpd,        // AVX
  kInstVroundps,        // AVX
  kInstVroundsd,        // AVX
  kInstVroundss,        // AVX
  kInstVrsqrtps,        // AVX
  kInstVrsqrtss,        // AVX
  kInstVshufpd,         // AVX
  kInstVshufps,         // AVX
  kInstVsqrtpd,         // AVX
  kInstVsqrtps,         // AVX
  kInstVsqrtsd,         // AVX
  kInstVsqrtss,         // AVX
  kInstVstmxcsr,        // AVX
  kInstVsubpd,          // AVX
  kInstVsubps,          // AVX
  kInstVsubsd,          // AVX
  kInstVsubss,          // AVX
  kInstVtestpd,         // AVX
  kInstVtestps,         // AVX
  kInstVucomisd,        // AVX
  kInstVucomiss,        // AVX
  kInstVunpckhpd,       // AVX
  kInstVunpckhps,       // AVX
  kInstVunpcklpd,       // AVX
  kInstVunpcklps,       // AVX
  kInstVxorpd,          // AVX
  kInstVxorps,          // AVX
  kInstVzeroall,        // AVX
  kInstVzeroupper,      // AVX
  kInstWrfsbase,        // FSGSBASE (x64)
  kInstWrgsbase,        // FSGSBASE (x64)
  kInstXadd,            // X86/X64 (i486)
  kInstXchg,            // X86/X64 (i386)
  kInstXor,             // X86/X64
  kInstXorpd,           // SSE2
  kInstXorps,           // SSE

  _kInstCount,

  _kInstCmovcc = kInstCmova,
  _kInstJcc = kInstJa,
  _kInstSetcc = kInstSeta,

  _kInstJbegin = kInstJa,
  _kInstJend = kInstJmp
};

// ============================================================================
// [asmjit::x86x64::kInstOptions]
// ============================================================================

//! X86/X64 instruction emit options, mainly for internal purposes.
ASMJIT_ENUM(kInstOptions) {
  //! Emit instruction with LOCK prefix.
  //!
  //! If this option is used and instruction doesn't support LOCK prefix an
  //! invalid instruction error is generated.
  kInstOptionLock = 0x10,

  //! Force REX prefix to be emitted.
  //!
  //! This option should be used carefully, because there are unencodable
  //! combinations. If you want to access ah, bh, ch or dh registers the REX
  //! prefix can't be emitted, otherwise illegal instruction error will be
  //! returned.
  kInstOptionRex = 0x40,

  //! Force three-byte VEX prefix to be emitted (instead of more compact
  //! two-byte VEX prefix).
  //!
  //! Ignored if the instruction is not AVX instruction or extension to the
  //! instruction set that is encoded by using VEX prefix.
  kInstOptionVex3 = 0x80
};

// ============================================================================
// [asmjit::x86x64::kInstGroup]
// ============================================================================

//! @internal
//!
//! X86/X64 instruction groups.
//!
//! This group is specific to AsmJit and only used by `x86x64::X86X64Assembler`.
ASMJIT_ENUM(kInstGroup) {
  //! Never used.
  kInstGroupNone,

  kInstGroupX86Op,
  kInstGroupX86Rm,
  kInstGroupX86Rm_B,
  kInstGroupX86RmReg,
  kInstGroupX86RegRm,
  kInstGroupX86M,
  //! Adc/Add/And/Cmp/Or/Sbb/Sub/Xor.
  kInstGroupX86Arith,
  //! Bswap.
  kInstGroupX86BSwap,
  //! Bt/Btc/Btr/Bts.
  kInstGroupX86BTest,
  //! Call.
  kInstGroupX86Call,
  //! Enter.
  kInstGroupX86Enter,
  //! Imul.
  kInstGroupX86Imul,
  //! Inc/Dec.
  kInstGroupX86IncDec,
  //! Int.
  kInstGroupX86Int,
  //! Jcc.
  kInstGroupX86Jcc,
  //! Jmp.
  kInstGroupX86Jmp,
  //! Lea.
  kInstGroupX86Lea,
  //! Mov.
  kInstGroupX86Mov,
  //! Movsx/Movzx.
  kInstGroupX86MovSxZx,
  //! Movsxd.
  kInstGroupX86MovSxd,
  //! Mov having absolute memory operand (x86/x64).
  kInstGroupX86MovPtr,
  //! Push.
  kInstGroupX86Push,
  //! Pop.
  kInstGroupX86Pop,
  //! Rep/Repe/Repne LodsX/MovsX/StosX/CmpsX/ScasX.
  kInstGroupX86Rep,
  //! Ret.
  kInstGroupX86Ret,
  //! Rcl/Rcr/Rol/Ror/Sal/Sar/Shl/Shr.
  kInstGroupX86Rot,
  //! Setcc.
  kInstGroupX86Set,
  //! Shld/Rhrd.
  kInstGroupX86Shlrd,
  //! Test.
  kInstGroupX86Test,
  //! Xadd.
  kInstGroupX86Xadd,
  //! Xchg.
  kInstGroupX86Xchg,

  //! Fincstp/Finit/FldX/Fnclex/Fninit/Fnop/Fpatan/Fprem/Fprem1/Fptan/Frndint/Fscale/Fsin/Fsincos/Fsqrt/Ftst/Fucompp/Fxam/Fxtract/Fyl2x/Fyl2xp1.
  kInstGroupFpuOp,
  //! Fadd/Fdiv/Fdivr/Fmul/Fsub/Fsubr.
  kInstGroupFpuArith,
  //! Fcom/Fcomp.
  kInstGroupFpuCom,
  //! Fld/Fst/Fstp.
  kInstGroupFpuFldFst,
  //! Fiadd/Ficom/Ficomp/Fidiv/Fidivr/Fild/Fimul/Fist/Fistp/Fisttp/Fisub/Fisubr.
  kInstGroupFpuM,
  //! Fcmov/Fcomi/Fcomip/Ffree/Fucom/Fucomi/Fucomip/Fucomp/Fxch.
  kInstGroupFpuR,
  //! Faddp/Fdivp/Fdivrp/Fmulp/Fsubp/Fsubrp.
  kInstGroupFpuRDef,
  //! Fnstsw/Fstsw.
  kInstGroupFpuStsw,

  //! Mm/Xmm instruction.
  kInstGroupExtRm,
  //! Mm/Xmm instruction (propagates 66H if the instruction uses XMM register).
  kInstGroupExtRm_P,
  //! Mm/Xmm instruction (propagates REX.W if GPQ is used).
  kInstGroupExtRm_Q,
  //! Mm/Xmm instruction (propagates 66H and REX.W).
  kInstGroupExtRm_PQ,
  //! Mm/Xmm instruction having Rm/Ri encodings.
  kInstGroupExtRmRi,
  //! Mm/Xmm instruction having Rm/Ri encodings (propagates 66H if the instruction uses XMM register).
  kInstGroupExtRmRi_P,
  //! Mm/Xmm instruction having Rmi encoding.
  kInstGroupExtRmi,
  //! Mm/Xmm instruction having Rmi encoding (propagates 66H if the instruction uses XMM register).
  kInstGroupExtRmi_P,
  //! Crc32.
  kInstGroupExtCrc,
  //! Pextrb/Pextrw/Pextrd/Pextrq/Extractps.
  kInstGroupExtExtract,
  //! Lfence/Mfence/Sfence.
  kInstGroupExtFence,
  //! Mov Mm/Xmm.
  //!
  //! 0x66 prefix must be set manually in opcodes.
  //!
  //! - Primary opcode is used for instructions in (X)Mm <- (X)Mm/Mem format,
  //! - Secondary opcode is used for instructions in (X)Mm/Mem <- (X)Mm format.
  kInstGroupExtMov,
  //! Mov Mm/Xmm.
  kInstGroupExtMovNoRexW,
  //! Movbe.
  kInstGroupExtMovBe,
  //! Movd.
  kInstGroupExtMovD,
  //! Movq.
  kInstGroupExtMovQ,
  //! Prefetch.
  kInstGroupExtPrefetch,

  //! 3dNow instruction.
  kInstGroup3dNow,

  //! AVX instruction without operands.
  kInstGroupAvxOp,
  //! AVX instruction encoded as 'M'.
  kInstGroupAvxM,
  //! AVX instruction encoded as 'MR'.
  kInstGroupAvxMr,
  //! AVX instruction encoded as 'MR'.
  //!
  //! Propagates AVX.L if the instruction uses YMM register.
  kInstGroupAvxMr_P,

  //! AVX instruction encoded as 'MRI'.
  kInstGroupAvxMri,
  //! AVX instruction encoded as 'MRI'.
  //!
  //! Propagates AVX.L if the instruction uses YMM register.
  kInstGroupAvxMri_P,

  //! AVX instruction encoded as 'RM'.
  kInstGroupAvxRm,
  //! AVX instruction encoded as 'RM'.
  //!
  //! Propagates AVX.L if the instruction uses YMM register.
  kInstGroupAvxRm_P,

  //! AVX instruction encoded as 'RMI'.
  kInstGroupAvxRmi,
  //! AVX instruction encoded as 'RMI'.
  //!
  //! Propagates AVX.L if the instruction uses YMM register.
  kInstGroupAvxRmi_P,

  //! AVX instruction encoded as 'RVM'.
  kInstGroupAvxRvm,
  //! AVX instruction encoded as 'RVM'.
  //!
  //! Propagates AVX.L if the instruction uses YMM register.
  kInstGroupAvxRvm_P,

  //! AVX instruction encoded as 'RVMR'.
  kInstGroupAvxRvmr,
  //! AVX instruction encoded as 'RVMR'.
  //!
  //! Propagates AVX.L if the instruction uses YMM register.
  kInstGroupAvxRvmr_P,
  //! AVX instruction encoded as 'RVMI'.

  kInstGroupAvxRvmi,
  //! AVX instruction encoded as 'RVMI'.
  //!
  //! Propagates AVX.L if the instruction uses YMM register.
  kInstGroupAvxRvmi_P,
  //! AVX instruction encoded as 'RMV'.

  kInstGroupAvxRmv,
  //! AVX instruction encoded as 'RMVI'.
  kInstGroupAvxRmvi,

  //! AVX instruction encoded as 'RM' or 'MR'.
  kInstGroupAvxRmMr,
  //! AVX instruction encoded as 'RM' or 'MR'.
  //!
  //! Propagates AVX.L if the instruction uses YMM register.
  kInstGroupAvxRmMr_P,

  //! AVX instruction encoded as 'RVM' or 'RMI'.
  kInstGroupAvxRvmRmi,
  //! AVX instruction encoded as 'RVM' or 'RMI'.
  //!
  //! Propagates AVX.L if the instruction uses YMM register.
  kInstGroupAvxRvmRmi_P,

  //! AVX instruction encoded as 'RVM' or 'MR'.
  kInstGroupAvxRvmMr,

  //! AVX instruction encoded as 'RVM' or 'MVR'.
  kInstGroupAvxRvmMvr,
  //! AVX instruction encoded as 'RVM' or 'MVR'.
  //!
  //! Propagates AVX.L if the instruction uses YMM register.
  kInstGroupAvxRvmMvr_P,

  //! AVX instruction encoded as 'RVM' or 'VMI'.
  kInstGroupAvxRvmVmi,
  //! AVX instruction encoded as 'RVM' or 'VMI'.
  //!
  //! Propagates AVX.L if the instruction uses YMM register.
  kInstGroupAvxRvmVmi_P,

  //! AVX instruction encoded as 'VM'.
  kInstGroupAvxVm,
  //! AVX instruction encoded as 'VMI'.
  kInstGroupAvxVmi,
  //! AVX instruction encoded as 'VMI'.
  //!
  //! Propagates AVX.L if the instruction uses YMM register.
  kInstGroupAvxVmi_P,

  //! AVX instruction encoded as 'RVRM' or 'RVMR'.
  kInstGroupAvxRvrmRvmr,
  //! AVX instruction encoded as 'RVRM' or 'RVMR'.
  //!
  //! Propagates AVX.L if the instruction uses YMM register).
  kInstGroupAvxRvrmRvmr_P,

  //! Vmovss/Vmovsd.
  kInstGroupAvxMovSsSd,

  //! AVX2 gather family instructions (VSIB).
  kInstGroupAvxGather,
  //! AVX2 gather family instructions (VSIB), differs only in mem operand.
  kInstGroupAvxGatherEx,

  //! FMA4 supporting XMM in form [R, R, R/M, R/M].
  kInstGroupFma4,
  //! FMA4 supporting XMM/YMM in form [R, R, R/M, R/M].
  kInstGroupFma4_P,

  //! XOP instruction encoded as 'RM'.
  kInstGroupXopRm,
  //! XOP instruction encoded as 'RM'.
  //!
  //! Propagates XOP.L if the instruction uses YMM register).
  kInstGroupXopRm_P,

  //! XOP instruction encoded as 'RVM' or 'RMV'.
  kInstGroupXopRvmRmv,

  //! XOP instruction encoded as 'RVM' or 'RMI'.
  kInstGroupXopRvmRmi,

  //! XOP instruction encoded as 'RVMR'.
  kInstGroupXopRvmr,
  //! XOP instruction encoded as 'RVMR'.
  //!
  //! Propagates XOP.L if the instruction uses YMM register.
  kInstGroupXopRvmr_P,

  //! XOP instruction encoded as 'RVMI'.
  kInstGroupXopRvmi,
  //! XOP instruction encoded as 'RVMI'.
  //!
  //! Propagates XOP.L if the instruction uses YMM register.
  kInstGroupXopRvmi_P,

  //! XOP instruction encoded as 'RVRM' or 'RVMR'.
  kInstGroupXopRvrmRvmr,
  //! XOP instruction encoded as 'RVRM' or 'RVMR'.
  //!
  //! Propagates XOP.L if the instruction uses YMM register).
  kInstGroupXopRvrmRvmr_P
};

// ============================================================================
// [asmjit::x86x64::kInstOpCode]
// ============================================================================

//! @internal
//!
//! Instruction OpCode encoding used by asmjit 'InstInfo' table.
//!
//! The schema was inspired by AVX/AVX2 features.
ASMJIT_ENUM(kInstOpCode) {
  // 'MMMMM' field in AVX/XOP instruction.
  // 'OpCode' leading bytes in legacy encoding.
  kInstOpCode_MM_Shift = 16,
  kInstOpCode_MM_Mask  = 0x0FU << kInstOpCode_MM_Shift,
  kInstOpCode_MM_00    = 0x00U << kInstOpCode_MM_Shift,
  kInstOpCode_MM_0F    = 0x01U << kInstOpCode_MM_Shift,
  kInstOpCode_MM_0F38  = 0x02U << kInstOpCode_MM_Shift,
  kInstOpCode_MM_0F3A  = 0x03U << kInstOpCode_MM_Shift,
  kInstOpCode_MM_0F01  = 0x0FU << kInstOpCode_MM_Shift, // Ext/Not part of AVX.

  kInstOpCode_MM_00011 = 0x03U << kInstOpCode_MM_Shift,
  kInstOpCode_MM_01000 = 0x08U << kInstOpCode_MM_Shift,
  kInstOpCode_MM_01001 = 0x09U << kInstOpCode_MM_Shift,

  // 'PP' field in AVX/XOP instruction.
  // 'Mandatory Prefix' in legacy encoding.
  kInstOpCode_PP_Shift = 21,
  kInstOpCode_PP_Mask  = 0x07U << kInstOpCode_PP_Shift,
  kInstOpCode_PP_00    = 0x00U << kInstOpCode_PP_Shift,
  kInstOpCode_PP_66    = 0x01U << kInstOpCode_PP_Shift,
  kInstOpCode_PP_F3    = 0x02U << kInstOpCode_PP_Shift,
  kInstOpCode_PP_F2    = 0x03U << kInstOpCode_PP_Shift,
  kInstOpCode_PP_9B    = 0x07U << kInstOpCode_PP_Shift, //Ext/Not part of AVX.

  // 'L' field in AVX/XOP instruction.
  kInstOpCode_L_Shift  = 24,
  kInstOpCode_L_Mask   = 0x01U << kInstOpCode_L_Shift,
  kInstOpCode_L_False  = 0x00U << kInstOpCode_L_Shift,
  kInstOpCode_L_True   = 0x01U << kInstOpCode_L_Shift,

  // 'O' field.
  kInstOpCode_O_Shift  = 29,
  kInstOpCode_O_Mask   = 0x07U << kInstOpCode_O_Shift
};

// ============================================================================
// [asmjit::x86x64::kInstFlags]
// ============================================================================

//! @internal
//!
//! X86/X64 instruction type flags.
ASMJIT_ENUM(kInstFlags) {
  //! No flags.
  kInstFlagNone        = 0x0000,

  //! Instruction is a control-flow instruction.
  //!
  //! Control flow instructions are jmp, jcc, call and ret.
  kInstFlagFlow        = 0x0001,

  //! Instruction is a compare/test like instruction.
  kInstFlagTest        = 0x0002,

  //! Instruction is a move like instruction.
  //!
  //! Move instructions typically overwrite the first operand by the second
  //! operand. The first operand can be the exact copy of the second operand
  //! or it can be any kind of conversion or shuffling.
  //!
  //! Mov instructions are 'mov', 'movd', 'movq', movdq', 'lea', multimedia
  //! instructions like 'cvtdq2pd', shuffle instructions like 'pshufb' and
  //! SSE/SSE2 mathematic instructions like 'rcp?', 'round?' and 'rsqrt?'.
  //!
  //! There are some MOV instructions that do only a partial move (for example
  //! 'cvtsi2ss'), register allocator has to know the variable size and use
  //! the flag accordingly to it.
  kInstFlagMove        = 0x0004,

  //! Instruction is an exchange like instruction.
  //!
  //! Exchange instruction typically overwrite first and second operand. So
  //! far only the instructions 'xchg' and 'xadd' are considered.
  kInstFlagXchg        = 0x0008,

  //! Instruction accesses Fp register(s).
  kInstFlagFp          = 0x0010,

  //! Instruction can be prefixed by using the LOCK prefix.
  kInstFlagLock        = 0x0020,

  //! Instruction is special, this is for `BaseCompiler`.
  kInstFlagSpecial     = 0x0040,

  //! Instruction always performs memory access.
  //!
  //! This flag is always combined with `kInstFlagSpecial` and signalizes
  //! that there is an implicit address which is accessed (usually EDI/RDI or
  //! ESI/EDI).
  kInstFlagSpecialMem  = 0x0080,

  //! Instruction memory operand can refer to 16-bit address (used by FPU).
  kInstFlagMem2        = 0x0100,
  //! Instruction memory operand can refer to 32-bit address (used by FPU).
  kInstFlagMem4        = 0x0200,
  //! Instruction memory operand can refer to 64-bit address (used by FPU).
  kInstFlagMem8        = 0x0400,
  //! Instruction memory operand can refer to 80-bit address (used by FPU).
  kInstFlagMem10       = 0x0800,

  //! Combination of `kInstFlagMem2` and `kInstFlagMem4`.
  kInstFlagMem2_4      = kInstFlagMem2   | kInstFlagMem4,
  //! Combination of `kInstFlagMem2` and `kInstFlagMem4` and `kInstFlagMem8`.
  kInstFlagMem2_4_8    = kInstFlagMem2_4 | kInstFlagMem8,
  //! Combination of `kInstFlagMem4` and `kInstFlagMem8`.
  kInstFlagMem4_8      = kInstFlagMem4   | kInstFlagMem8,
  //! Combination of `kInstFlagMem4` and `kInstFlagMem8` and `kInstFlagMem10`.
  kInstFlagMem4_8_10   = kInstFlagMem4_8 | kInstFlagMem10,

  //! Zeroes the rest of the register if the source operand is memory.
  kInstFlagZeroIfMem   = 0x1000,

  //! REX.W/VEX.W by default.
  kInstFlagW           = 0x8000
};

// ============================================================================
// [asmjit::x86x64::kInstOp]
// ============================================================================

//! @internal
//!
//! X86/X64 instruction operand flags.
ASMJIT_ENUM(kInstOp) {
  // Gp, Fp, Mm, Xmm, Ymm, Zmm.
  kInstOpGb            = 0x0001,
  kInstOpGw            = 0x0002,
  kInstOpGd            = 0x0004,
  kInstOpGq            = 0x0008,
  kInstOpFp            = 0x0010,
  kInstOpMm            = 0x0020,
  kInstOpXmm           = 0x0100,
  kInstOpYmm           = 0x0200,
  kInstOpZmm           = 0x0400,

  // Mem, Imm.
  kInstOpMem           = 0x4000,
  kInstOpImm           = 0x8000,

  // Combined.
  kInstOpGwb           = kInstOpGw     | kInstOpGb,
  kInstOpGqd           = kInstOpGq     | kInstOpGd,
  kInstOpGqdw          = kInstOpGq     | kInstOpGd | kInstOpGw,
  kInstOpGqdwb         = kInstOpGq     | kInstOpGd | kInstOpGw | kInstOpGb,

  kInstOpGbMem         = kInstOpGb     | kInstOpMem,
  kInstOpGwMem         = kInstOpGw     | kInstOpMem,
  kInstOpGdMem         = kInstOpGd     | kInstOpMem,
  kInstOpGqMem         = kInstOpGq     | kInstOpMem,
  kInstOpGwbMem        = kInstOpGwb    | kInstOpMem,
  kInstOpGqdMem        = kInstOpGqd    | kInstOpMem,
  kInstOpGqdwMem       = kInstOpGqdw   | kInstOpMem,
  kInstOpGqdwbMem      = kInstOpGqdwb  | kInstOpMem,

  kInstOpFpMem         = kInstOpFp     | kInstOpMem,
  kInstOpMmMem         = kInstOpMm     | kInstOpMem,
  kInstOpXmmMem        = kInstOpXmm    | kInstOpMem,
  kInstOpYmmMem        = kInstOpYmm    | kInstOpMem,

  kInstOpMmXmm         = kInstOpMm     | kInstOpXmm,
  kInstOpMmXmmMem      = kInstOpMmXmm  | kInstOpMem,

  kInstOpXmmYmm        = kInstOpXmm    | kInstOpYmm,
  kInstOpXmmYmmMem     = kInstOpXmmYmm | kInstOpMem
};

// ============================================================================
// [asmjit::x86x64::kCond]
// ============================================================================

//! X86/X64 Condition codes.
ASMJIT_ENUM(kCond) {
  // Condition codes from processor manuals.
  kCondA               = 0x07, // CF==0 & ZF==0          (unsigned)
  kCondAE              = 0x03, // CF==0                  (unsigned)
  kCondB               = 0x02, // CF==1                  (unsigned)
  kCondBE              = 0x06, // CF==1 | ZF==1          (unsigned)
  kCondC               = 0x02, // CF==1
  kCondE               = 0x04, //         ZF==1          (signed/unsigned)
  kCondG               = 0x0F, //         ZF==0 & SF==OF (signed)
  kCondGE              = 0x0D, //                 SF==OF (signed)
  kCondL               = 0x0C, //                 SF!=OF (signed)
  kCondLE              = 0x0E, //         ZF==1 | SF!=OF (signed)
  kCondNA              = 0x06, // CF==1 | ZF==1          (unsigned)
  kCondNAE             = 0x02, // CF==1                  (unsigned)
  kCondNB              = 0x03, // CF==0                  (unsigned)
  kCondNBE             = 0x07, // CF==0 & ZF==0          (unsigned)
  kCondNC              = 0x03, // CF==0
  kCondNE              = 0x05, //         ZF==0          (signed/unsigned)
  kCondNG              = 0x0E, //         ZF==1 | SF!=OF (signed)
  kCondNGE             = 0x0C, //                 SF!=OF (signed)
  kCondNL              = 0x0D, //                 SF==OF (signed)
  kCondNLE             = 0x0F, //         ZF==0 & SF==OF (signed)
  kCondNO              = 0x01, //                 OF==0
  kCondNP              = 0x0B, // PF==0
  kCondNS              = 0x09, //                 SF==0
  kCondNZ              = 0x05, //         ZF==0
  kCondO               = 0x00, //                 OF==1
  kCondP               = 0x0A, // PF==1
  kCondPE              = 0x0A, // PF==1
  kCondPO              = 0x0B, // PF==0
  kCondS               = 0x08, //                 SF==1
  kCondZ               = 0x04, //         ZF==1

  // Simplified condition codes.
  kCondOverflow        = 0x00,
  kCondNotOverflow     = 0x01,
  kCondBelow           = 0x02, // Unsigned.
  kCondAboveEqual      = 0x03, // Unsigned.
  kCondEqual           = 0x04,
  kCondNotEqual        = 0x05,
  kCondBelowEqual      = 0x06, // Unsigned.
  kCondAbove           = 0x07, // Unsigned.
  kCondSign            = 0x08,
  kCondNotSign         = 0x09,
  kCondParityEven      = 0x0A,
  kCondParityOdd       = 0x0B,
  kCondLess            = 0x0C, // Signed.
  kCondGreaterEqual    = 0x0D, // Signed.
  kCondLessEqual       = 0x0E, // Signed.
  kCondGreater         = 0x0F, // Signed.

  // Aliases.
  kCondZero            = 0x04,
  kCondNotZero         = 0x05,
  kCondNegative        = 0x08,
  kCondPositive        = 0x09,

  // Fpu-only.
  kCondFpuUnordered    = 0x10,
  kCondFpuNotUnordered = 0x11,

  //! No condition code.
  kCondNone            = 0x12
};

// ============================================================================
// [asmjit::x86x64::kVarType]
// ============================================================================

//! X86/X64 variable type.
ASMJIT_ENUM(kVarType) {
  //! Variable is Mm (MMX).
  kVarTypeMm = 12,

  //! Variable is Xmm (SSE/SSE2).
  kVarTypeXmm,
  //! Variable is SSE scalar SP-FP number.
  kVarTypeXmmSs,
  //! Variable is SSE packed SP-FP number (4 floats).
  kVarTypeXmmPs,
  //! Variable is SSE2 scalar DP-FP number.
  kVarTypeXmmSd,
  //! Variable is SSE2 packed DP-FP number (2 doubles).
  kVarTypeXmmPd,

  //! Variable is Ymm (AVX).
  kVarTypeYmm,
  //! Variable is AVX packed SP-FP number (8 floats).
  kVarTypeYmmPs,
  //! Variable is AVX packed DP-FP number (4 doubles).
  kVarTypeYmmPd,

  //! Count of variable types.
  kVarTypeCount,

  //! @internal
  _kVarTypeMmStart = kVarTypeMm,
  //! @internal
  _kVarTypeMmEnd = kVarTypeMm,

  //! @internal
  _kVarTypeXmmStart = kVarTypeXmm,
  //! @internal
  _kVarTypeXmmEnd = kVarTypeXmmPd,

  //! @internal
  _kVarTypeYmmStart = kVarTypeYmm,
  //! @internal
  _kVarTypeYmmEnd = kVarTypeYmmPd
};

// ============================================================================
// [asmjit::x86x64::kVarDesc]
// ============================================================================

//! @internal
//!
//! X86/X64 variable description.
ASMJIT_ENUM(kVarDesc) {
  //! Variable contains single-precision floating-point(s).
  kVarDescSp = 0x10,
  //! Variable contains double-precision floating-point(s).
  kVarDescDp = 0x20,
  //! Variable is packed (for example float4x, double2x, ...).
  kVarDescPacked = 0x40
};

// ============================================================================
// [asmjit::x86x64::InstInfo]
// ============================================================================

//! @internal
//!
//! X86 instruction information.
struct InstInfo {
  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! Get instruction name string - a null terminated string.
  ASMJIT_INLINE const char* getName() const {
    return _instName + static_cast<uint32_t>(_nameIndex);
  }

  //! Get instruction name index to `_instName` array.
  ASMJIT_INLINE uint32_t _getNameIndex() const {
    return _nameIndex;
  }

  //! Get instruction group, see `kInstGroup`.
  ASMJIT_INLINE uint32_t getGroup() const {
    return _group;
  }

  //! Get size of move instruction in bytes.
  //!
  //! If zero, the size of MOV instruction is determined by the size of the
  //! destination register (applies mostly for x86 arithmetic). This value is
  //! useful for register allocator when determining if a variable is going to
  //! be overwritten or not. Basically if the move size is equal or greater
  //! than a variable itself it is considered overwritten.
  ASMJIT_INLINE uint32_t getMoveSize() const {
    return _moveSize;
  }

  // --------------------------------------------------------------------------
  // [Flags]
  // --------------------------------------------------------------------------

  //! Get instruction flags, see `kInstFlags`.
  ASMJIT_INLINE uint32_t getFlags() const {
    return _flags;
  }

  //! Get whether the instruction is a control-flow intruction.
  //!
  //! Control flow instruction is instruction that modifies instruction pointer,
  //! typically jmp, jcc, call, or ret.
  ASMJIT_INLINE bool isFlow() const {
    return (_flags & kInstFlagFlow) != 0;
  }

  //! Get whether the instruction is a compare/test like intruction.
  ASMJIT_INLINE bool isTest() const {
    return (_flags & kInstFlagTest) != 0;
  }

  //! Get whether the instruction is a typical move instruction.
  //!
  //! Move instructions overwrite the first operand or at least part of it,
  //! This is a very useful hint that is used by variable liveness analysis
  //! and `BaseCompiler` in general to know which variable is completely 
  //! overwritten.
  //!
  //! All AVX/XOP instructions that have 3 or more operands are considered to
  //! have move semantics move by default.
  ASMJIT_INLINE bool isMove() const {
    return (_flags & kInstFlagMove) != 0;
  }

  //! Get whether the instruction is a typical Exchange instruction.
  //!
  //! Exchange instructios are 'xchg' and 'xadd'.
  ASMJIT_INLINE bool isXchg() const {
    return (_flags & kInstFlagXchg) != 0;
  }

  //! Get whether the instruction accesses Fp register(s).
  ASMJIT_INLINE bool isFp() const {
    return (_flags & kInstFlagFp) != 0;
  }

  //! Get whether the instruction can be prefixed by LOCK prefix.
  ASMJIT_INLINE bool isLockable() const {
    return (_flags & kInstFlagLock) != 0;
  }

  //! Get whether the instruction is special type (this is used by
  //! `BaseCompiler` to manage additional variables or functionality).
  ASMJIT_INLINE bool isSpecial() const {
    return (_flags & kInstFlagSpecial) != 0;
  }

  //! Get whether the instruction is special type and it performs
  //! memory access.
  ASMJIT_INLINE bool isSpecialMem() const {
    return (_flags & kInstFlagSpecialMem) != 0;
  }

  //! Get whether the move instruction zeroes the rest of the register
  //! if the source is memory operand.
  //!
  //! Basically flag needed by 'movsd' and 'movss' instructions.
  ASMJIT_INLINE bool isZeroIfMem() const {
    return (_flags & kInstFlagZeroIfMem) != 0;
  }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! Instruction name index in _instName[] array.
  uint16_t _nameIndex;
  //! Instruction flags.
  uint16_t _flags;
  //! Instruction group, used by `BaseAssembler`.
  uint8_t _group;
  //! Count of bytes overritten by a move instruction.
  //!
  //! Only used when kInstFlagMove flag is set. If this value is zero move
  //! depends on the destination register size.
  uint8_t _moveSize;
  //! Reserved for future use.
  uint8_t _reserved[2];
  //! Operands' flags.
  uint16_t _opFlags[4];
  //! Primary and secondary opcodes.
  uint32_t _opCode[2];
};

// ============================================================================
// [asmjit::x86x64::VarInfo]
// ============================================================================

//! @internal
//!
//! X86 variable information.
struct VarInfo {
  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! Get register type, see `kRegType`.
  ASMJIT_INLINE uint32_t getReg() const { return _reg; }
  //! Get register size in bytes.
  ASMJIT_INLINE uint32_t getSize() const { return _size; }
  //! Get variable class, see `kRegClass`.
  ASMJIT_INLINE uint32_t getClass() const { return _class; }
  //! Get variable description, see `kVarDesc`.
  ASMJIT_INLINE uint32_t getDesc() const { return _desc; }
  //! Get variable type name.
  ASMJIT_INLINE const char* getName() const { return _name; }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! Register type, see `kRegType`.
  uint8_t _reg;
  //! Register size in bytes.
  uint8_t _size;
  //! Register class, see `kRegClass`.
  uint8_t _class;
  //! Variable flags, see `kVarDesc`.
  uint8_t _desc;
  //! Variable type name.
  char _name[4];
};

// ============================================================================
// [asmjit::x86x64::RegCount]
// ============================================================================

//! @internal
//!
//! X86/X64 registers count (Gp, Fp, Mm, Xmm).
struct RegCount {
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
    ASMJIT_ASSERT(c < kRegClassCount);

    return _regs[c];
  }

  // --------------------------------------------------------------------------
  // [Set]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE void set(uint32_t c, uint32_t n) {
    ASMJIT_ASSERT(c < kRegClassCount);
    ASMJIT_ASSERT(n < 0x100);

    _regs[c] = static_cast<uint8_t>(n);
  }

  // --------------------------------------------------------------------------
  // [Add]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE void add(uint32_t c, uint32_t n = 1) {
    ASMJIT_ASSERT(c < kRegClassCount);
    ASMJIT_ASSERT(n < 0x100);

    _regs[c] += static_cast<uint8_t>(n);
  }

  // --------------------------------------------------------------------------
  // [Misc]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE void makeIndex(const RegCount& count) {
    _regs[0] = 0;
    _regs[1] = count._regs[0];
    _regs[2] = count._regs[1];
    _regs[3] = count._regs[2];

    _regs[2] += _regs[1];
    _regs[3] += _regs[2];
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
// [asmjit::x86x64::RegMask]
// ============================================================================

//! @internal
//!
//! X86/X64 registers mask (Gp, Fp, Mm, Xmm/Ymm/Zmm).
struct RegMask {
  // --------------------------------------------------------------------------
  // [Zero]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE void zero(uint32_t c) {
    _packed.u16[c] = 0;
  }

  // --------------------------------------------------------------------------
  // [Get]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE uint32_t get(uint32_t c) const {
    return _packed.u16[c];
  }

  // --------------------------------------------------------------------------
  // [Set]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE void set(uint32_t c, uint32_t mask) {
    _packed.u16[c] = static_cast<uint16_t>(mask);
  }

  ASMJIT_INLINE void set(const RegMask& other) {
    _packed.setUInt64(other._packed);
  }

  // --------------------------------------------------------------------------
  // [Add]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE void add(uint32_t c, uint32_t mask) {
    _packed.u16[c] |= static_cast<uint16_t>(mask);
  }

  ASMJIT_INLINE void add(const RegMask& other) {
    _packed.or_(other._packed);
  }

  // --------------------------------------------------------------------------
  // [Del]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE void del(uint32_t c, uint32_t mask) {
    _packed.u16[c] &= ~static_cast<uint16_t>(mask);
  }

  ASMJIT_INLINE void del(const RegMask& other) {
    _packed.del(other._packed);
  }

  // --------------------------------------------------------------------------
  // [And]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE void and_(uint32_t c, uint32_t mask) {
    _packed.u16[c] &= static_cast<uint16_t>(mask);
  }

  ASMJIT_INLINE void and_(const RegMask& other) {
    _packed.and_(other._packed);
  }

  // --------------------------------------------------------------------------
  // [Xor]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE void xor_(uint32_t c, uint32_t mask) {
    _packed.u16[c] ^= static_cast<uint16_t>(mask);
  }

  ASMJIT_INLINE void xor_(const RegMask& other) {
    _packed.xor_(other._packed);
  }

  // --------------------------------------------------------------------------
  // [IsEmpty / Has]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE bool isEmpty() const {
    return _packed.isZero();
  }

  ASMJIT_INLINE bool has(uint32_t c, uint32_t mask = 0xFFFFFFFF) const {
    return (static_cast<uint32_t>(_packed.u16[c]) & mask) != 0;
  }

  // --------------------------------------------------------------------------
  // [Reset]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE void reset() {
    _packed.reset();
  }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  union {
    struct {
      //! Gp registers mask.
      uint16_t _gp;
      //! Fp registers mask.
      uint16_t _fp;
      //! Mm registers mask.
      uint16_t _mm;
      //! Xmm/Ymm registers mask.
      uint16_t _xy;
    };

    uint16_t _regs[4];

    //! All masks as 64-bit integer.
    UInt64 _packed;
  };
};

// ============================================================================
// [asmjit::x86x64::X86Reg]
// ============================================================================

//! X86/X64 register.
struct X86Reg : public BaseReg {
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Create a dummy X86 register.
  ASMJIT_INLINE X86Reg() : BaseReg() {}
  //! Create a custom X86 register.
  ASMJIT_INLINE X86Reg(uint32_t type, uint32_t index, uint32_t size) : BaseReg(type, index, size) {}
  //! Create a reference to `other` X86 register.
  ASMJIT_INLINE X86Reg(const X86Reg& other) : BaseReg(other) {}
  //! Create non-initialized X86 register.
  explicit ASMJIT_INLINE X86Reg(const _NoInit&) : BaseReg(NoInit) {}

  // --------------------------------------------------------------------------
  // [X86Reg Specific]
  // --------------------------------------------------------------------------

  ASMJIT_REG_OP(X86Reg)

  //! Get whether the register is Gp register.
  ASMJIT_INLINE bool isGp() const { return _vreg.type <= kRegTypeGpq; }
  //! Get whether the register is Gp byte (8-bit) register.
  ASMJIT_INLINE bool isGpb() const { return _vreg.type <= kRegTypeGpbHi; }
  //! Get whether the register is Gp lo-byte (8-bit) register.
  ASMJIT_INLINE bool isGpbLo() const { return _vreg.type == kRegTypeGpbLo; }
  //! Get whether the register is Gp hi-byte (8-bit) register.
  ASMJIT_INLINE bool isGpbHi() const { return _vreg.type == kRegTypeGpbHi; }
  //! Get whether the register is Gp word (16-bit) register.
  ASMJIT_INLINE bool isGpw() const { return _vreg.type == kRegTypeGpw; }
  //! Get whether the register is Gp dword (32-bit) register.
  ASMJIT_INLINE bool isGpd() const { return _vreg.type == kRegTypeGpd; }
  //! Get whether the register is Gp qword (64-bit) register.
  ASMJIT_INLINE bool isGpq() const { return _vreg.type == kRegTypeGpq; }

  //! Get whether the register is Fp register.
  ASMJIT_INLINE bool isFp() const { return _vreg.type == kRegTypeFp; }
  //! Get whether the register is Mm (64-bit) register.
  ASMJIT_INLINE bool isMm() const { return _vreg.type == kRegTypeMm; }
  //! Get whether the register is Xmm (128-bit) register.
  ASMJIT_INLINE bool isXmm() const { return _vreg.type == kRegTypeXmm; }
  //! Get whether the register is Ymm (256-bit) register.
  ASMJIT_INLINE bool isYmm() const { return _vreg.type == kRegTypeYmm; }

  //! Get whether the register is a segment.
  ASMJIT_INLINE bool isSeg() const { return _vreg.type == kRegTypeSeg; }
};

// ============================================================================
// [asmjit::x86x64::GpReg]
// ============================================================================

//! X86/X64 Gpb/Gpw/Gpd/Gpq register.
struct GpReg : public X86Reg {
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Create a dummy Gp register.
  ASMJIT_INLINE GpReg() : X86Reg() {}
  //! Create a reference to `other` Gp register.
  ASMJIT_INLINE GpReg(const GpReg& other) : X86Reg(other) {}
  //! Create a custom Gp register.
  ASMJIT_INLINE GpReg(uint32_t type, uint32_t index, uint32_t size) : X86Reg(type, index, size) {}
  //! Create non-initialized Gp register.
  explicit ASMJIT_INLINE GpReg(const _NoInit&) : X86Reg(NoInit) {}

  // --------------------------------------------------------------------------
  // [GpReg Specific]
  // --------------------------------------------------------------------------

  ASMJIT_REG_OP(GpReg)
};

// ============================================================================
// [asmjit::x86x64::FpReg]
// ============================================================================

//! X86/X64 80-bit Fp register.
struct FpReg : public X86Reg {
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Create a dummy Fp register.
  ASMJIT_INLINE FpReg() : X86Reg() {}
  //! Create a reference to `other` FPU register.
  ASMJIT_INLINE FpReg(const FpReg& other) : X86Reg(other) {}
  //! Create a custom Fp register.
  ASMJIT_INLINE FpReg(uint32_t type, uint32_t index, uint32_t size) : X86Reg(type, index, size) {}
  //! Create non-initialized Fp register.
  explicit ASMJIT_INLINE FpReg(const _NoInit&) : X86Reg(NoInit) {}

  // --------------------------------------------------------------------------
  // [FpReg Specific]
  // --------------------------------------------------------------------------

  ASMJIT_REG_OP(FpReg)
};

// ============================================================================
// [asmjit::x86x64::MmReg]
// ============================================================================

//! X86/X64 64-bit Mm register.
struct MmReg : public X86Reg {
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Create a dummy Mm register.
  ASMJIT_INLINE MmReg() : X86Reg() {}
  //! Create a reference to `other` Mm register.
  ASMJIT_INLINE MmReg(const MmReg& other) : X86Reg(other) {}
  //! Create a custom Mm register.
  ASMJIT_INLINE MmReg(uint32_t type, uint32_t index, uint32_t size) : X86Reg(type, index, size) {}
  //! Create non-initialized Mm register.
  explicit ASMJIT_INLINE MmReg(const _NoInit&) : X86Reg(NoInit) {}

  // --------------------------------------------------------------------------
  // [MmReg Specific]
  // --------------------------------------------------------------------------

  ASMJIT_REG_OP(MmReg)
};

// ============================================================================
// [asmjit::x86x64::XmmReg]
// ============================================================================

//! X86/X64 128-bit Xmm register.
struct XmmReg : public X86Reg {
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Create a dummy Xmm register.
  ASMJIT_INLINE XmmReg() : X86Reg() {}
  //! Create a reference to `other` Xmm register.
  ASMJIT_INLINE XmmReg(const XmmReg& other) : X86Reg(other) {}
  //! Create a custom Xmm register.
  ASMJIT_INLINE XmmReg(uint32_t type, uint32_t index, uint32_t size) : X86Reg(type, index, size) {}
  //! Create non-initialized Xmm register.
  explicit ASMJIT_INLINE XmmReg(const _NoInit&) : X86Reg(NoInit) {}

  // --------------------------------------------------------------------------
  // [XmmReg Specific]
  // --------------------------------------------------------------------------

  ASMJIT_REG_OP(XmmReg)
};

// ============================================================================
// [asmjit::x86x64::YmmReg]
// ============================================================================

//! X86/X64 256-bit Ymm register.
struct YmmReg : public X86Reg {
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Create a dummy Ymm register.
  ASMJIT_INLINE YmmReg() : X86Reg() {}
  //! Create a reference to `other` Xmm register.
  ASMJIT_INLINE YmmReg(const YmmReg& other) : X86Reg(other) {}
  //! Create a custom Ymm register.
  ASMJIT_INLINE YmmReg(uint32_t type, uint32_t index, uint32_t size) : X86Reg(type, index, size) {}
  //! Create non-initialized Ymm register.
  explicit ASMJIT_INLINE YmmReg(const _NoInit&) : X86Reg(NoInit) {}

  // --------------------------------------------------------------------------
  // [YmmReg Specific]
  // --------------------------------------------------------------------------

  ASMJIT_REG_OP(YmmReg)
};

// ============================================================================
// [asmjit::x86x64::SegReg]
// ============================================================================

//! X86/X64 segment register.
struct SegReg : public X86Reg {
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Create a dummy segment register.
  ASMJIT_INLINE SegReg() : X86Reg() {}
  //! Create a reference to `other` segment register.
  ASMJIT_INLINE SegReg(const SegReg& other) : X86Reg(other) {}
  //! Create a custom segment register.
  ASMJIT_INLINE SegReg(uint32_t type, uint32_t index, uint32_t size) : X86Reg(type, index, size) {}
  //! Create non-initialized segment register.
  explicit ASMJIT_INLINE SegReg(const _NoInit&) : X86Reg(NoInit) {}

  // --------------------------------------------------------------------------
  // [SegReg Specific]
  // --------------------------------------------------------------------------

  ASMJIT_REG_OP(SegReg)
};

// ============================================================================
// [asmjit::x86x64::Mem]
// ============================================================================

#define _OP_ID(_Op_) reinterpret_cast<const Operand&>(_Op_).getId()

//! X86 memory operand.
struct Mem : public BaseMem {
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE Mem() : BaseMem(NoInit) {
    reset();
  }

  ASMJIT_INLINE Mem(const Label& label, int32_t disp, uint32_t size = 0) : BaseMem(NoInit) {
    _init_packed_op_sz_b0_b1_id(kOperandTypeMem, size, kMemTypeLabel, 0, label._base.id);
    _init_packed_d2_d3(kInvalidValue, disp);
  }

  ASMJIT_INLINE Mem(const Label& label, const GpReg& index, uint32_t shift, int32_t disp, uint32_t size = 0) : BaseMem(NoInit) {
    ASMJIT_ASSERT(shift <= 3);

    _init_packed_op_sz_b0_b1_id(kOperandTypeMem, size, kMemTypeLabel,
      (kMemVSibGpz << kMemVSibIndex)
        + (shift << kMemShiftIndex),
      label.getId());
    _vmem.index = index.getRegIndex();
    _vmem.displacement = disp;
  }

  ASMJIT_INLINE Mem(const Label& label, const GpVar& index, uint32_t shift, int32_t disp, uint32_t size = 0) : BaseMem(NoInit) {
    ASMJIT_ASSERT(shift <= 3);

    _init_packed_op_sz_b0_b1_id(kOperandTypeMem, size, kMemTypeLabel,
      (kMemVSibGpz << kMemVSibIndex)
        + (shift << kMemShiftIndex),
      label.getId());
    _vmem.index = _OP_ID(index);
    _vmem.displacement = disp;
  }

  ASMJIT_INLINE Mem(const GpReg& base, int32_t disp, uint32_t size = 0) : BaseMem(NoInit) {
    _init_packed_op_sz_b0_b1_id(kOperandTypeMem, size, kMemTypeBaseIndex,
      _getGpdFlags(base)
        + (kMemVSibGpz << kMemVSibIndex),
      base.getRegIndex());
    _init_packed_d2_d3(kInvalidValue, disp);
  }

  ASMJIT_INLINE Mem(const GpReg& base, const GpReg& index, uint32_t shift, int32_t disp, uint32_t size = 0) : BaseMem(NoInit) {
    ASMJIT_ASSERT(shift <= 3);

    _init_packed_op_sz_b0_b1_id(kOperandTypeMem, size, kMemTypeBaseIndex,
      _getGpdFlags(base) + (shift << kMemShiftIndex),
      base.getRegIndex());
    _vmem.index = index.getRegIndex();
    _vmem.displacement = disp;
  }

  ASMJIT_INLINE Mem(const GpReg& base, const XmmReg& index, uint32_t shift, int32_t disp, uint32_t size = 0) : BaseMem(NoInit) {
    ASMJIT_ASSERT(shift <= 3);

    _init_packed_op_sz_b0_b1_id(kOperandTypeMem, size, kMemTypeBaseIndex,
      _getGpdFlags(base)
        + (kMemVSibXmm << kMemVSibIndex)
        + (shift << kMemShiftIndex),
      base.getRegIndex());
    _vmem.index = index.getRegIndex();
    _vmem.displacement = disp;
  }

  ASMJIT_INLINE Mem(const GpReg& base, const YmmReg& index, uint32_t shift, int32_t disp, uint32_t size = 0) : BaseMem(NoInit) {
    ASMJIT_ASSERT(shift <= 3);

    _init_packed_op_sz_b0_b1_id(kOperandTypeMem, size, kMemTypeBaseIndex,
      _getGpdFlags(base)
        + (kMemVSibYmm << kMemVSibIndex)
        + (shift << kMemShiftIndex),
      base.getRegIndex());
    _vmem.index = index.getRegIndex();
    _vmem.displacement = disp;
  }

  ASMJIT_INLINE Mem(const GpVar& base, int32_t disp, uint32_t size = 0) : BaseMem(NoInit) {
    _init_packed_op_sz_b0_b1_id(kOperandTypeMem, size, kMemTypeBaseIndex,
      _getGpdFlags(reinterpret_cast<const BaseVar&>(base))
        + (kMemVSibGpz << kMemVSibIndex),
      _OP_ID(base));
    _init_packed_d2_d3(kInvalidValue, disp);
  }


  ASMJIT_INLINE Mem(const GpVar& base, const GpVar& index, uint32_t shift, int32_t disp, uint32_t size = 0) : BaseMem(NoInit) {
    ASMJIT_ASSERT(shift <= 3);

    _init_packed_op_sz_b0_b1_id(kOperandTypeMem, size, kMemTypeBaseIndex,
      _getGpdFlags(reinterpret_cast<const BaseVar&>(base))
        + (shift << kMemShiftIndex),
      _OP_ID(base));
    _vmem.index = _OP_ID(index);
    _vmem.displacement = disp;
  }

  ASMJIT_INLINE Mem(const GpVar& base, const XmmVar& index, uint32_t shift, int32_t disp, uint32_t size = 0) : BaseMem(NoInit) {
    ASMJIT_ASSERT(shift <= 3);

    _init_packed_op_sz_b0_b1_id(kOperandTypeMem, size, kMemTypeBaseIndex,
      _getGpdFlags(reinterpret_cast<const BaseVar&>(base))
        + (kMemVSibXmm << kMemVSibIndex)
        + (shift << kMemShiftIndex),
      _OP_ID(base));
    _vmem.index = _OP_ID(index);
    _vmem.displacement = disp;
  }

  ASMJIT_INLINE Mem(const GpVar& base, const YmmVar& index, uint32_t shift, int32_t disp, uint32_t size = 0) : BaseMem(NoInit) {
    ASMJIT_ASSERT(shift <= 3);

    _init_packed_op_sz_b0_b1_id(kOperandTypeMem, size, kMemTypeBaseIndex,
      _getGpdFlags(reinterpret_cast<const BaseVar&>(base))
        + (kMemVSibYmm << kMemVSibIndex)
        + (shift << kMemShiftIndex),
      _OP_ID(base));
    _vmem.index = _OP_ID(index);
    _vmem.displacement = disp;
  }

  ASMJIT_INLINE Mem(const _Init&, uint32_t memType, const X86Var& base, int32_t disp, uint32_t size) : BaseMem(NoInit) {
    _init_packed_op_sz_b0_b1_id(kOperandTypeMem, size, memType, 0, _OP_ID(base));
    _vmem.index = kInvalidValue;
    _vmem.displacement = disp;
  }

  ASMJIT_INLINE Mem(const _Init&, uint32_t memType, const X86Var& base, const GpVar& index, uint32_t shift, int32_t disp, uint32_t size) : BaseMem(NoInit) {
    ASMJIT_ASSERT(shift <= 3);

    _init_packed_op_sz_b0_b1_id(kOperandTypeMem, size, memType, shift << kMemShiftIndex, _OP_ID(base));
    _vmem.index = _OP_ID(index);
    _vmem.displacement = disp;
  }

  ASMJIT_INLINE Mem(const Mem& other) : BaseMem(other) {}
  explicit ASMJIT_INLINE Mem(const _NoInit&) : BaseMem(NoInit) {}

  // --------------------------------------------------------------------------
  // [Mem Specific]
  // --------------------------------------------------------------------------

  //! Clone Mem operand.
  ASMJIT_INLINE Mem clone() const {
    return Mem(*this);
  }

  //! Reset Mem operand.
  ASMJIT_INLINE void reset() {
    _init_packed_op_sz_b0_b1_id(kOperandTypeMem, 0, kMemTypeBaseIndex, 0, kInvalidValue);
    _init_packed_d2_d3(kInvalidValue, 0);
  }

  //! @internal
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
    return (_vmem.flags & kMemSegMask) != (kSegDefault << kMemSegIndex);
  }

  //! Get memory operand segment, see `kSeg`.
  ASMJIT_INLINE uint32_t getSegment() const {
    return (static_cast<uint32_t>(_vmem.flags) >> kMemSegIndex) & kMemSegBits;
  }

  //! Set memory operand segment, see `kSeg`.
  ASMJIT_INLINE Mem& setSegment(uint32_t segIndex) {
    _vmem.flags = static_cast<uint8_t>(
      (static_cast<uint32_t>(_vmem.flags) & kMemSegMask) + (segIndex << kMemSegIndex));
    return *this;
  }

  //! Set memory operand segment, see `kSeg`.
  ASMJIT_INLINE Mem& setSegment(const SegReg& seg) {
    return setSegment(seg.getRegIndex());
  }

  // --------------------------------------------------------------------------
  // [Gpd]
  // --------------------------------------------------------------------------

  //! Get whether the memory operand has 32-bit GP base.
  ASMJIT_INLINE bool hasGpdBase() const {
    return (_packed[0].u32[0] & IntUtil::pack32_4x8(0x00, 0x00, 0x00, kMemGpdMask)) != 0;
  }

  //! Set whether the memory operand has 32-bit GP base.
  ASMJIT_INLINE Mem& setGpdBase() {
    _packed[0].u32[0] |= IntUtil::pack32_4x8(0x00, 0x00, 0x00, kMemGpdMask);
    return *this;
  }

  //! Set whether the memory operand has 32-bit GP base to `b`.
  ASMJIT_INLINE Mem& setGpdBase(uint32_t b) {
    _packed[0].u32[0] &=~IntUtil::pack32_4x8(0x00, 0x00, 0x00, kMemGpdMask);
    _packed[0].u32[0] |= IntUtil::pack32_4x8(0x00, 0x00, 0x00, b << kMemGpdIndex);
    return *this;
  }

  // --------------------------------------------------------------------------
  // [VSib]
  // --------------------------------------------------------------------------

  //! Get SIB type.
  ASMJIT_INLINE uint32_t getVSib() const {
    return (static_cast<uint32_t>(_vmem.flags) >> kMemVSibIndex) & kMemVSibBits;
  }

  //! Set SIB type.
  ASMJIT_INLINE Mem& _setVSib(uint32_t vsib) {
    _packed[0].u32[0] &=~IntUtil::pack32_4x8(0x00, 0x00, 0x00, kMemVSibMask);
    _packed[0].u32[0] |= IntUtil::pack32_4x8(0x00, 0x00, 0x00, vsib << kMemVSibIndex);
    return *this;
  }

  // --------------------------------------------------------------------------
  // [Size]
  // --------------------------------------------------------------------------

  //! Set memory operand size.
  ASMJIT_INLINE Mem& setSize(uint32_t size) {
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
  ASMJIT_INLINE Mem& setBase(uint32_t base) {
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
  ASMJIT_INLINE Mem& setIndex(uint32_t index) {
    _vmem.index = index;
    return *this;
  }

  //! Set memory index.
  ASMJIT_INLINE Mem& setIndex(const GpReg& index) {
    _vmem.index = index.getRegIndex();
    return _setVSib(kMemVSibGpz);
  }

  //! Set memory index.
  ASMJIT_INLINE Mem& setIndex(const GpReg& index, uint32_t shift) {
    _vmem.index = index.getRegIndex();
    return _setVSib(kMemVSibGpz).setShift(shift);
  }

  //! Set memory index.
  ASMJIT_INLINE Mem& setIndex(const GpVar& index) {
    _vmem.index = reinterpret_cast<const BaseVar&>(index).getId();
    return _setVSib(kMemVSibGpz);
  }

  //! Set memory index.
  ASMJIT_INLINE Mem& setIndex(const GpVar& index, uint32_t shift) {
    _vmem.index = reinterpret_cast<const BaseVar&>(index).getId();
    return _setVSib(kMemVSibGpz).setShift(shift);
  }

  //! Set memory index.
  ASMJIT_INLINE Mem& setIndex(const XmmReg& index) {
    _vmem.index = index.getRegIndex();
    return _setVSib(kMemVSibXmm);
  }

  //! Set memory index.
  ASMJIT_INLINE Mem& setIndex(const XmmReg& index, uint32_t shift) {
    _vmem.index = index.getRegIndex();
    return _setVSib(kMemVSibXmm).setShift(shift);
  }

  //! Set memory index.
  ASMJIT_INLINE Mem& setIndex(const XmmVar& index) {
    _vmem.index = reinterpret_cast<const BaseVar&>(index).getId();
    return _setVSib(kMemVSibXmm);
  }

  //! Set memory index.
  ASMJIT_INLINE Mem& setIndex(const XmmVar& index, uint32_t shift) {
    _vmem.index = reinterpret_cast<const BaseVar&>(index).getId();
    return _setVSib(kMemVSibXmm).setShift(shift);
  }

  //! Set memory index.
  ASMJIT_INLINE Mem& setIndex(const YmmReg& index) {
    _vmem.index = index.getRegIndex();
    return _setVSib(kMemVSibYmm);
  }

  //! Set memory index.
  ASMJIT_INLINE Mem& setIndex(const YmmReg& index, uint32_t shift) {
    _vmem.index = index.getRegIndex();
    return _setVSib(kMemVSibYmm).setShift(shift);
  }

  //! Set memory index.
  ASMJIT_INLINE Mem& setIndex(const YmmVar& index) {
    _vmem.index = reinterpret_cast<const BaseVar&>(index).getId();
    return _setVSib(kMemVSibYmm);
  }

  //! Set memory index.
  ASMJIT_INLINE Mem& setIndex(const YmmVar& index, uint32_t shift) {
    _vmem.index = reinterpret_cast<const BaseVar&>(index).getId();
    return _setVSib(kMemVSibYmm).setShift(shift);
  }

  //! Reset memory index.
  ASMJIT_INLINE Mem& resetIndex() {
    _vmem.index = kInvalidValue;
    return _setVSib(kMemVSibGpz);
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
    return (_vmem.flags & kMemShiftMask) != 0;
  }

  //! Get memory operand index scale (0, 1, 2 or 3).
  ASMJIT_INLINE uint32_t getShift() const {
    return _vmem.flags >> kMemShiftIndex;
  }

  //! Set memory operand index scale (0, 1, 2 or 3).
  ASMJIT_INLINE Mem& setShift(uint32_t shift) {
    _packed[0].u32[0] &=~IntUtil::pack32_4x8(0x00, 0x00, 0x00, kMemShiftMask);
    _packed[0].u32[0] |= IntUtil::pack32_4x8(0x00, 0x00, 0x00, shift << kMemShiftIndex);
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
  ASMJIT_INLINE Mem& setDisplacement(int32_t disp) {
    _vmem.displacement = disp;
    return *this;
  }

  //! Reset memory operand relative displacement.
  ASMJIT_INLINE Mem& resetDisplacement() {
    _vmem.displacement = 0;
    return *this;
  }

  //! Adjust memory operand relative displacement by `disp`.
  ASMJIT_INLINE Mem& adjust(int32_t disp) {
    _vmem.displacement += disp;
    return *this;
  }

  //! Get new memory operand adjusted by `disp`.
  ASMJIT_INLINE Mem adjusted(int32_t disp) const {
    Mem result(*this);
    result.adjust(disp);
    return result;
  }

  // --------------------------------------------------------------------------
  // [Operator Overload]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE Mem& operator=(const Mem& other) {
    _copy(other);
    return *this;
  }

  ASMJIT_INLINE bool operator==(const Mem& other) const {
    return (_packed[0] == other._packed[0]) & (_packed[1] == other._packed[1]) ;
  }

  ASMJIT_INLINE bool operator!=(const Mem& other) const {
    return !(*this == other);
  }

  // --------------------------------------------------------------------------
  // [Static]
  // --------------------------------------------------------------------------

  static ASMJIT_INLINE uint32_t _getGpdFlags(const Operand& base) {
    return (base._vreg.size & 0x4) << (kMemGpdIndex - 2);
  }
};

#undef _OP_ID

// ============================================================================
// [asmjit::x86x64::X86Var]
// ============================================================================

//! Base class for all variables.
struct X86Var : public BaseVar {
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE X86Var() : BaseVar(NoInit) {
    reset();
  }

  ASMJIT_INLINE X86Var(const X86Var& other) : BaseVar(other) {}

  explicit ASMJIT_INLINE X86Var(const _NoInit&) : BaseVar(NoInit) {}

  // --------------------------------------------------------------------------
  // [X86Var Specific]
  // --------------------------------------------------------------------------

  //! Clone X86Var operand.
  ASMJIT_INLINE X86Var clone() const {
    return X86Var(*this);
  }

  //! Reset X86Var operand.
  ASMJIT_INLINE void reset() {
    _init_packed_op_sz_b0_b1_id(kOperandTypeVar, 0, kInvalidReg, kInvalidReg, kInvalidValue);
    _init_packed_d2_d3(kInvalidValue, kInvalidValue);
  }

  // --------------------------------------------------------------------------
  // [Type]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE uint32_t getRegType() const { return _vreg.type; }
  ASMJIT_INLINE uint32_t getVarType() const { return _vreg.vType; }

  //! Get whether the variable is Gpb register.
  ASMJIT_INLINE bool isGp() const { return _vreg.type <= kRegTypeGpq; }
  //! Get whether the variable is Gpb register.
  ASMJIT_INLINE bool isGpb() const { return _vreg.type <= kRegTypeGpbHi; }
  //! Get whether the variable is Gpb-lo register.
  ASMJIT_INLINE bool isGpbLo() const { return _vreg.type == kRegTypeGpbLo; }
  //! Get whether the variable is Gpb-hi register.
  ASMJIT_INLINE bool isGpbHi() const { return _vreg.type == kRegTypeGpbHi; }
  //! Get whether the variable is Gpw register.
  ASMJIT_INLINE bool isGpw() const { return _vreg.type == kRegTypeGpw; }
  //! Get whether the variable is Gpd register.
  ASMJIT_INLINE bool isGpd() const { return _vreg.type == kRegTypeGpd; }
  //! Get whether the variable is Gpq register.
  ASMJIT_INLINE bool isGpq() const { return _vreg.type == kRegTypeGpq; }

  //! Get whether the variable is Fp register.
  ASMJIT_INLINE bool isFp() const { return _vreg.type == kRegTypeFp; }
  //! Get whether the variable is Mm type.
  ASMJIT_INLINE bool isMm() const { return _vreg.type == kRegTypeMm; }
  //! Get whether the variable is Xmm type.
  ASMJIT_INLINE bool isXmm() const { return _vreg.type == kRegTypeXmm; }
  //! Get whether the variable is Ymm type.
  ASMJIT_INLINE bool isYmm() const { return _vreg.type == kRegTypeYmm; }

  // --------------------------------------------------------------------------
  // [Memory Cast]
  // --------------------------------------------------------------------------

  //! Cast this variable to memory operand.
  //!
  //! @note Size of operand depends on native variable type, you can use other
  //! variants if you want specific one.
  ASMJIT_INLINE Mem m(int32_t disp = 0) const
  { return Mem(Init, kMemTypeStackIndex, *this, disp, getSize()); }

  //! @overload
  ASMJIT_INLINE Mem m(const GpVar& index, uint32_t shift = 0, int32_t disp = 0) const
  { return Mem(Init, kMemTypeStackIndex, *this, index, shift, disp, getSize()); }

  //! Cast this variable to 8-bit memory operand.
  ASMJIT_INLINE Mem m8(int32_t disp = 0) const
  { return Mem(Init, kMemTypeStackIndex, *this, disp, 1); }

  //! @overload
  ASMJIT_INLINE Mem m8(const GpVar& index, uint32_t shift = 0, int32_t disp = 0) const
  { return Mem(Init, kMemTypeStackIndex, *this, index, shift, disp, 1); }

  //! Cast this variable to 16-bit memory operand.
  ASMJIT_INLINE Mem m16(int32_t disp = 0) const
  { return Mem(Init, kMemTypeStackIndex, *this, disp, 2); }

  //! @overload
  ASMJIT_INLINE Mem m16(const GpVar& index, uint32_t shift = 0, int32_t disp = 0) const
  { return Mem(Init, kMemTypeStackIndex, *this, index, shift, disp, 2); }

  //! Cast this variable to 32-bit memory operand.
  ASMJIT_INLINE Mem m32(int32_t disp = 0) const
  { return Mem(Init, kMemTypeStackIndex, *this, disp, 4); }

  //! @overload
  ASMJIT_INLINE Mem m32(const GpVar& index, uint32_t shift = 0, int32_t disp = 0) const
  { return Mem(Init, kMemTypeStackIndex, *this, index, shift, disp, 4); }

  //! Cast this variable to 64-bit memory operand.
  ASMJIT_INLINE Mem m64(int32_t disp = 0) const
  { return Mem(Init, kMemTypeStackIndex, *this, disp, 8); }

  //! @overload
  ASMJIT_INLINE Mem m64(const GpVar& index, uint32_t shift = 0, int32_t disp = 0) const
  { return Mem(Init, kMemTypeStackIndex, *this, index, shift, disp, 8); }

  //! Cast this variable to 80-bit memory operand (long double).
  ASMJIT_INLINE Mem m80(int32_t disp = 0) const
  { return Mem(Init, kMemTypeStackIndex, *this, disp, 10); }

  //! @overload
  ASMJIT_INLINE Mem m80(const GpVar& index, uint32_t shift = 0, int32_t disp = 0) const
  { return Mem(Init, kMemTypeStackIndex, *this, index, shift, disp, 10); }

  //! Cast this variable to 128-bit memory operand.
  ASMJIT_INLINE Mem m128(int32_t disp = 0) const
  { return Mem(Init, kMemTypeStackIndex, *this, disp, 16); }

  //! @overload
  ASMJIT_INLINE Mem m128(const GpVar& index, uint32_t shift = 0, int32_t disp = 0) const
  { return Mem(Init, kMemTypeStackIndex, *this, index, shift, disp, 16); }

  //! Cast this variable to 256-bit memory operand.
  ASMJIT_INLINE Mem m256(int32_t disp = 0) const
  { return Mem(Init, kMemTypeStackIndex, *this, disp, 32); }

  //! @overload
  ASMJIT_INLINE Mem m256(const GpVar& index, uint32_t shift = 0, int32_t disp = 0) const
  { return Mem(Init, kMemTypeStackIndex, *this, index, shift, disp, 32); }

  // --------------------------------------------------------------------------
  // [Operator Overload]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE X86Var& operator=(const X86Var& other) { _copy(other); return *this; }

  ASMJIT_INLINE bool operator==(const X86Var& other) const { return _packed[0] == other._packed[0]; }
  ASMJIT_INLINE bool operator!=(const X86Var& other) const { return !operator==(other); }

  // --------------------------------------------------------------------------
  // [Private]
  // --------------------------------------------------------------------------

protected:
  ASMJIT_INLINE X86Var(const X86Var& other, uint32_t reg, uint32_t size) : BaseVar(NoInit)
  {
    _init_packed_op_sz_w0_id(kOperandTypeVar, size, (reg << 8) + other._vreg.index, other._base.id);
    _vreg.vType = other._vreg.vType;
  }
};

// ============================================================================
// [asmjit::x86x64::GpVar]
// ============================================================================

//! Gp variable.
struct GpVar : public X86Var {
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Create a new uninitialized `GpVar` instance.
  ASMJIT_INLINE GpVar() : X86Var() {}

  //! Create a new initialized `GpVar` instance.
  ASMJIT_INLINE GpVar(BaseCompiler& c, uint32_t type = kVarTypeIntPtr, const char* name = NULL) : X86Var(NoInit) {
    c._newVar(this, type, name);
  }

  //! Create a clone of `other`.
  ASMJIT_INLINE GpVar(const GpVar& other) : X86Var(other) {}

  //! Create a new uninitialized `GpVar` instance (internal).
  explicit ASMJIT_INLINE GpVar(const _NoInit&) : X86Var(NoInit) {}

  // --------------------------------------------------------------------------
  // [GpVar Specific]
  // --------------------------------------------------------------------------

  //! Clone GpVar operand.
  ASMJIT_INLINE GpVar clone() const {
    return GpVar(*this);
  }

  //! Reset GpVar operand.
  ASMJIT_INLINE void reset() {
    X86Var::reset();
  }

  // --------------------------------------------------------------------------
  // [GpVar Cast]
  // --------------------------------------------------------------------------

  //! Cast this variable to 8-bit (LO) part of variable
  ASMJIT_INLINE GpVar r8() const { return GpVar(*this, kRegTypeGpbLo, 1); }
  //! Cast this variable to 8-bit (LO) part of variable
  ASMJIT_INLINE GpVar r8Lo() const { return GpVar(*this, kRegTypeGpbLo, 1); }
  //! Cast this variable to 8-bit (HI) part of variable
  ASMJIT_INLINE GpVar r8Hi() const { return GpVar(*this, kRegTypeGpbHi, 1); }

  //! Cast this variable to 16-bit part of variable
  ASMJIT_INLINE GpVar r16() const { return GpVar(*this, kRegTypeGpw, 2); }
  //! Cast this variable to 32-bit part of variable
  ASMJIT_INLINE GpVar r32() const { return GpVar(*this, kRegTypeGpd, 4); }
  //! Cast this variable to 64-bit part of variable
  ASMJIT_INLINE GpVar r64() const { return GpVar(*this, kRegTypeGpq, 8); }

  // --------------------------------------------------------------------------
  // [Operator Overload]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE GpVar& operator=(const GpVar& other) { _copy(other); return *this; }

  ASMJIT_INLINE bool operator==(const GpVar& other) const { return X86Var::operator==(other); }
  ASMJIT_INLINE bool operator!=(const GpVar& other) const { return X86Var::operator!=(other); }

  // --------------------------------------------------------------------------
  // [Private]
  // --------------------------------------------------------------------------

protected:
  ASMJIT_INLINE GpVar(const GpVar& other, uint32_t reg, uint32_t size) : X86Var(other, reg, size) {}
};

// ============================================================================
// [asmjit::x86x64::FpVar]
// ============================================================================

//! Fpu variable.
struct FpVar : public X86Var {
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Create a new uninitialized `FpVar` instance.
  ASMJIT_INLINE FpVar() : X86Var() {}
  //! Create a new variable that links to `other`.
  ASMJIT_INLINE FpVar(const FpVar& other) : X86Var(other) {}

  //! Create a new uninitialized `FpVar` instance (internal).
  explicit ASMJIT_INLINE FpVar(const _NoInit&) : X86Var(NoInit) {}

  // --------------------------------------------------------------------------
  // [FpVar Specific]
  // --------------------------------------------------------------------------

  //! Clone FpVar operand.
  ASMJIT_INLINE FpVar clone() const {
    return FpVar(*this);
  }

  //! Reset FpVar operand.
  ASMJIT_INLINE void reset() {
    X86Var::reset();
  }

  // --------------------------------------------------------------------------
  // [Operator Overload]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE FpVar& operator=(const FpVar& other) { _copy(other); return *this; }

  ASMJIT_INLINE bool operator==(const FpVar& other) const { return X86Var::operator==(other); }
  ASMJIT_INLINE bool operator!=(const FpVar& other) const { return X86Var::operator!=(other); }
};

// ============================================================================
// [asmjit::x86x64::MmVar]
// ============================================================================

//! Mm variable.
struct MmVar : public X86Var {
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Create a new uninitialized `MmVar` instance.
  ASMJIT_INLINE MmVar() : X86Var() {}
  //! Create a new initialized `MmVar` instance.
  ASMJIT_INLINE MmVar(BaseCompiler& c, uint32_t type = kVarTypeMm, const char* name = NULL) : X86Var(NoInit) {
    c._newVar(this, type, name);
  }

  //! Create a clone of `other`.
  ASMJIT_INLINE MmVar(const MmVar& other) : X86Var(other) {}

  //! Create a new uninitialized `MmVar` instance (internal).
  explicit ASMJIT_INLINE MmVar(const _NoInit&) : X86Var(NoInit) {}

  // --------------------------------------------------------------------------
  // [MmVar Specific]
  // --------------------------------------------------------------------------

  //! Clone MmVar operand.
  ASMJIT_INLINE MmVar clone() const {
    return MmVar(*this);
  }

  //! Reset MmVar operand.
  ASMJIT_INLINE void reset() {
    X86Var::reset();
  }

  // --------------------------------------------------------------------------
  // [Operator Overload]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE MmVar& operator=(const MmVar& other) { _copy(other); return *this; }

  ASMJIT_INLINE bool operator==(const MmVar& other) const { return X86Var::operator==(other); }
  ASMJIT_INLINE bool operator!=(const MmVar& other) const { return X86Var::operator!=(other); }
};

// ============================================================================
// [asmjit::x86x64::XmmVar]
// ============================================================================

//! Xmm variable.
struct XmmVar : public X86Var {
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Create a new uninitialized `XmmVar` instance.
  ASMJIT_INLINE XmmVar() : X86Var() {}
  //! Create a new initialized `XmmVar` instance.
  ASMJIT_INLINE XmmVar(BaseCompiler& c, uint32_t type = kVarTypeXmm, const char* name = NULL) : X86Var(NoInit) {
    c._newVar(this, type, name);
  }

  //! Create a clone of `other`.
  ASMJIT_INLINE XmmVar(const XmmVar& other) : X86Var(other) {}

  //! Create a new uninitialized `XmmVar` instance (internal).
  explicit ASMJIT_INLINE XmmVar(const _NoInit&) : X86Var(NoInit) {}

  // --------------------------------------------------------------------------
  // [XmmVar Specific]
  // --------------------------------------------------------------------------

  //! Clone XmmVar operand.
  ASMJIT_INLINE XmmVar clone() const {
    return XmmVar(*this);
  }

  //! Reset XmmVar operand.
  ASMJIT_INLINE void reset() {
    X86Var::reset();
  }

  // --------------------------------------------------------------------------
  // [Operator Overload]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE XmmVar& operator=(const XmmVar& other) { _copy(other); return *this; }

  ASMJIT_INLINE bool operator==(const XmmVar& other) const { return X86Var::operator==(other); }
  ASMJIT_INLINE bool operator!=(const XmmVar& other) const { return X86Var::operator!=(other); }
};

// ============================================================================
// [asmjit::x86x64::YmmVar]
// ============================================================================

//! Ymm variable.
struct YmmVar : public X86Var {
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Create a new uninitialized `YmmVar` instance.
  ASMJIT_INLINE YmmVar() : X86Var() {}
  //! Create a new initialized `YmmVar` instance.
  ASMJIT_INLINE YmmVar(BaseCompiler& c, uint32_t type = kVarTypeYmm, const char* name = NULL) : X86Var(NoInit) {
    c._newVar(this, type, name);
  }

  //! Create a clone of `other`.
  ASMJIT_INLINE YmmVar(const YmmVar& other) : X86Var(other) {}

  //! Create a new uninitialized `YmmVar` instance (internal).
  explicit ASMJIT_INLINE YmmVar(const _NoInit&) : X86Var(NoInit) {}

  // --------------------------------------------------------------------------
  // [YmmVar Specific]
  // --------------------------------------------------------------------------

  //! Clone YmmVar operand.
  ASMJIT_INLINE YmmVar clone() const {
    return YmmVar(*this);
  }

  //! Reset YmmVar operand.
  ASMJIT_INLINE void reset() {
    X86Var::reset();
  }

  // --------------------------------------------------------------------------
  // [Operator Overload]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE YmmVar& operator=(const YmmVar& other) { _copy(other); return *this; }

  ASMJIT_INLINE bool operator==(const YmmVar& other) const { return X86Var::operator==(other); }
  ASMJIT_INLINE bool operator!=(const YmmVar& other) const { return X86Var::operator!=(other); }
};

// ============================================================================
// [asmjit::x86x64::Macros]
// ============================================================================

//! Create Shuffle Constant for MMX/SSE shuffle instrutions.
//!
//! @param z First component position, number at interval [0, 3] inclusive.
//! @param x Second component position, number at interval [0, 3] inclusive.
//! @param y Third component position, number at interval [0, 3] inclusive.
//! @param w Fourth component position, number at interval [0, 3] inclusive.
//!
//! Shuffle constants can be used to make immediate value for these intrinsics:
//! - `x86x64::X86X64Assembler::pshufw()` and `x86x64::X86X64Compiler::pshufw()`
//! - `x86x64::X86X64Assembler::pshufd()` and `x86x64::X86X64Compiler::pshufd()`
//! - `x86x64::X86X64Assembler::pshufhw()` and `x86x64::X86X64Compiler::pshufhw()`
//! - `x86x64::X86X64Assembler::pshuflw()` and `x86x64::X86X64Compiler::pshuflw()`
//! - `x86x64::X86X64Assembler::shufps()` and `x86x64::X86X64Compiler::shufps()`
static ASMJIT_INLINE uint8_t mm_shuffle(uint8_t z, uint8_t y, uint8_t x, uint8_t w) {
  return (z << 6) | (y << 4) | (x << 2) | w;
}

// ============================================================================
// [asmjit::x86x64::Cond - Reverse / Negate]
// ============================================================================

//! Corresponds to transposing the operands of a comparison.
static ASMJIT_INLINE uint32_t reverseCond(uint32_t cond) {
  ASMJIT_ASSERT(cond < ASMJIT_ARRAY_SIZE(_reverseCond));
  return _reverseCond[cond];
}

//! Get the equivalent of negated condition code.
static ASMJIT_INLINE uint32_t negateCond(uint32_t cond) {
  ASMJIT_ASSERT(cond < ASMJIT_ARRAY_SIZE(_reverseCond));
  return static_cast<kCond>(cond ^ static_cast<uint32_t>(cond < kCondNone));
}

// ============================================================================
// [asmjit::x86x64::Cond - ToJcc / ToMovcc / ToSetcc]
// ============================================================================

//! Translate condition code `cc` to cmovcc instruction code.
//! @sa `kInstCode`, `_kInstCmovcc`.
static ASMJIT_INLINE uint32_t condToCmovcc(uint32_t cond) {
  ASMJIT_ASSERT(static_cast<uint32_t>(cond) < ASMJIT_ARRAY_SIZE(_condToCmovcc));
  return _condToCmovcc[cond];
}

//! Translate condition code `cc` to jcc instruction code.
//! @sa `kInstCode`, `_kInstJcc`.
static ASMJIT_INLINE uint32_t condToJcc(uint32_t cond) {
  ASMJIT_ASSERT(static_cast<uint32_t>(cond) < ASMJIT_ARRAY_SIZE(_condToJcc));
  return _condToJcc[cond];
}

//! Translate condition code `cc` to setcc instruction code.
//! @sa `kInstCode`, `_kInstSetcc`.
static ASMJIT_INLINE uint32_t condToSetcc(uint32_t cond) {
  ASMJIT_ASSERT(static_cast<uint32_t>(cond) < ASMJIT_ARRAY_SIZE(_condToSetcc));
  return _condToSetcc[cond];
}

// ============================================================================
// [asmjit::x86x64::Registers]
// ============================================================================

//! No register, can be used only within `Mem` operand.
ASMJIT_VAR const GpReg noGpReg;

//! Gpb-lo register.
ASMJIT_VAR const GpReg al;
//! Gpb-lo register.
ASMJIT_VAR const GpReg cl;
//! Gpb-lo register.
ASMJIT_VAR const GpReg dl;
//! Gpb-lo register.
ASMJIT_VAR const GpReg bl;
//! Gpb-hi register.
ASMJIT_VAR const GpReg ah;
//! Gpb-hi register.
ASMJIT_VAR const GpReg ch;
//! Gpb-hi register.
ASMJIT_VAR const GpReg dh;
//! Gpb-hi register.
ASMJIT_VAR const GpReg bh;

//! Gpw register.
ASMJIT_VAR const GpReg ax;
//! Gpw register.
ASMJIT_VAR const GpReg cx;
//! Gpw register.
ASMJIT_VAR const GpReg dx;
//! Gpw register.
ASMJIT_VAR const GpReg bx;
//! Gpw register.
ASMJIT_VAR const GpReg sp;
//! Gpw register.
ASMJIT_VAR const GpReg bp;
//! Gpw register.
ASMJIT_VAR const GpReg si;
//! Gpw register.
ASMJIT_VAR const GpReg di;

//! Gpd register.
ASMJIT_VAR const GpReg eax;
//! Gpd register.
ASMJIT_VAR const GpReg ecx;
//! Gpd register.
ASMJIT_VAR const GpReg edx;
//! Gpd register.
ASMJIT_VAR const GpReg ebx;
//! Gpd register.
ASMJIT_VAR const GpReg esp;
//! Gpd register.
ASMJIT_VAR const GpReg ebp;
//! Gpd register.
ASMJIT_VAR const GpReg esi;
//! Gpd register.
ASMJIT_VAR const GpReg edi;

//! Fp register.
ASMJIT_VAR const FpReg fp0;
//! Fp register.
ASMJIT_VAR const FpReg fp1;
//! Fp register.
ASMJIT_VAR const FpReg fp2;
//! Fp register.
ASMJIT_VAR const FpReg fp3;
//! Fp register.
ASMJIT_VAR const FpReg fp4;
//! Fp register.
ASMJIT_VAR const FpReg fp5;
//! Fp register.
ASMJIT_VAR const FpReg fp6;
//! Fp register.
ASMJIT_VAR const FpReg fp7;

//! Mm register.
ASMJIT_VAR const MmReg mm0;
//! Mm register.
ASMJIT_VAR const MmReg mm1;
//! Mm register.
ASMJIT_VAR const MmReg mm2;
//! Mm register.
ASMJIT_VAR const MmReg mm3;
//! Mm register.
ASMJIT_VAR const MmReg mm4;
//! Mm register.
ASMJIT_VAR const MmReg mm5;
//! Mm register.
ASMJIT_VAR const MmReg mm6;
//! Mm register.
ASMJIT_VAR const MmReg mm7;

//! Xmm register.
ASMJIT_VAR const XmmReg xmm0;
//! Xmm register.
ASMJIT_VAR const XmmReg xmm1;
//! Xmm register.
ASMJIT_VAR const XmmReg xmm2;
//! Xmm register.
ASMJIT_VAR const XmmReg xmm3;
//! Xmm register.
ASMJIT_VAR const XmmReg xmm4;
//! Xmm register.
ASMJIT_VAR const XmmReg xmm5;
//! Xmm register.
ASMJIT_VAR const XmmReg xmm6;
//! Xmm register.
ASMJIT_VAR const XmmReg xmm7;

//! Ymm register.
ASMJIT_VAR const YmmReg ymm0;
//! Ymm register.
ASMJIT_VAR const YmmReg ymm1;
//! Ymm register.
ASMJIT_VAR const YmmReg ymm2;
//! Ymm register.
ASMJIT_VAR const YmmReg ymm3;
//! Ymm register.
ASMJIT_VAR const YmmReg ymm4;
//! Ymm register.
ASMJIT_VAR const YmmReg ymm5;
//! Ymm register.
ASMJIT_VAR const YmmReg ymm6;
//! Ymm register.
ASMJIT_VAR const YmmReg ymm7;

//! Cs segment register.
ASMJIT_VAR const SegReg cs;
//! Ss segment register.
ASMJIT_VAR const SegReg ss;
//! Ds segment register.
ASMJIT_VAR const SegReg ds;
//! Es segment register.
ASMJIT_VAR const SegReg es;
//! Fs segment register.
ASMJIT_VAR const SegReg fs;
//! Gs segment register.
ASMJIT_VAR const SegReg gs;

//! Get Gpb-lo register.
static ASMJIT_INLINE GpReg gpb_lo(uint32_t index) { return GpReg(kRegTypeGpbLo, index, 1); }
//! Get Gpb-hi register.
static ASMJIT_INLINE GpReg gpb_hi(uint32_t index) { return GpReg(kRegTypeGpbHi, index, 1); }
//! Get Gpw register.
static ASMJIT_INLINE GpReg gpw(uint32_t index) { return GpReg(kRegTypeGpw, index, 2); }
//! Get Gpd register.
static ASMJIT_INLINE GpReg gpd(uint32_t index) { return GpReg(kRegTypeGpd, index, 4); }
//! Get Fp register.
static ASMJIT_INLINE FpReg fp(uint32_t index) { return FpReg(kRegTypeFp, index, 10); }
//! Get Mm register.
static ASMJIT_INLINE MmReg mm(uint32_t index) { return MmReg(kRegTypeMm, index, 8); }
//! Get Xmm register.
static ASMJIT_INLINE XmmReg xmm(uint32_t index) { return XmmReg(kRegTypeXmm, index, 16); }
//! Get Xmm register.
static ASMJIT_INLINE YmmReg ymm(uint32_t index) { return YmmReg(kRegTypeYmm, index, 32); }

// ============================================================================
// [asmjit::x86x64::Mem - ptr[base + disp]]
// ============================================================================

//! Create a custom pointer operand.
static ASMJIT_INLINE Mem ptr(const GpReg& base, int32_t disp = 0, uint32_t size = 0) { return Mem(base, disp, size); }
//! Create a byte pointer operand.
static ASMJIT_INLINE Mem byte_ptr(const GpReg& base, int32_t disp = 0) { return ptr(base, disp, kSizeByte); }
//! Create a word pointer operand.
static ASMJIT_INLINE Mem word_ptr(const GpReg& base, int32_t disp = 0) { return ptr(base, disp, kSizeWord); }
//! Create a dword pointer operand.
static ASMJIT_INLINE Mem dword_ptr(const GpReg& base, int32_t disp = 0) { return ptr(base, disp, kSizeDWord); }
//! Create a qword pointer operand.
static ASMJIT_INLINE Mem qword_ptr(const GpReg& base, int32_t disp = 0) { return ptr(base, disp, kSizeQWord); }
//! Create a tword pointer operand.
static ASMJIT_INLINE Mem tword_ptr(const GpReg& base, int32_t disp = 0) { return ptr(base, disp, kSizeTWord); }
//! Create a oword pointer operand.
static ASMJIT_INLINE Mem oword_ptr(const GpReg& base, int32_t disp = 0) { return ptr(base, disp, kSizeOWord); }
//! Create a yword pointer operand.
static ASMJIT_INLINE Mem yword_ptr(const GpReg& base, int32_t disp = 0) { return ptr(base, disp, kSizeYWord); }

//! Create a custom pointer operand.
static ASMJIT_INLINE Mem ptr(const GpVar& base, int32_t disp = 0, uint32_t size = 0) { return Mem(base, disp, size); }
//! Create a byte pointer operand.
static ASMJIT_INLINE Mem byte_ptr(const GpVar& base, int32_t disp = 0) { return ptr(base, disp, kSizeByte); }
//! Create a word pointer operand.
static ASMJIT_INLINE Mem word_ptr(const GpVar& base, int32_t disp = 0) { return ptr(base, disp, kSizeWord); }
//! Create a dword pointer operand.
static ASMJIT_INLINE Mem dword_ptr(const GpVar& base, int32_t disp = 0) { return ptr(base, disp, kSizeDWord); }
//! Create a qword pointer operand.
static ASMJIT_INLINE Mem qword_ptr(const GpVar& base, int32_t disp = 0) { return ptr(base, disp, kSizeQWord); }
//! Create a tword pointer operand.
static ASMJIT_INLINE Mem tword_ptr(const GpVar& base, int32_t disp = 0) { return ptr(base, disp, kSizeTWord); }
//! Create a oword pointer operand.
static ASMJIT_INLINE Mem oword_ptr(const GpVar& base, int32_t disp = 0) { return ptr(base, disp, kSizeOWord); }
//! Create a yword pointer operand.
static ASMJIT_INLINE Mem yword_ptr(const GpVar& base, int32_t disp = 0) { return ptr(base, disp, kSizeYWord); }

// ============================================================================
// [asmjit::x86x64::Mem - ptr[base + (index << shift) + disp]]
// ============================================================================

//! Create a custom pointer operand.
static ASMJIT_INLINE Mem ptr(const GpReg& base, const GpReg& index, uint32_t shift = 0, int32_t disp = 0, uint32_t size = 0) { return Mem(base, index, shift, disp, size); }
//! Create a byte pointer operand.
static ASMJIT_INLINE Mem byte_ptr(const GpReg& base, const GpReg& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, kSizeByte); }
//! Create a word pointer operand.
static ASMJIT_INLINE Mem word_ptr(const GpReg& base, const GpReg& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, kSizeWord); }
//! Create a dword pointer operand.
static ASMJIT_INLINE Mem dword_ptr(const GpReg& base, const GpReg& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, kSizeDWord); }
//! Create a qword pointer operand.
static ASMJIT_INLINE Mem qword_ptr(const GpReg& base, const GpReg& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, kSizeQWord); }
//! Create a tword pointer operand.
static ASMJIT_INLINE Mem tword_ptr(const GpReg& base, const GpReg& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, kSizeTWord); }
//! Create a oword pointer operand.
static ASMJIT_INLINE Mem oword_ptr(const GpReg& base, const GpReg& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, kSizeOWord); }
//! Create a yword pointer operand.
static ASMJIT_INLINE Mem yword_ptr(const GpReg& base, const GpReg& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, kSizeYWord); }

//! Create a custom pointer operand.
static ASMJIT_INLINE Mem ptr(const GpVar& base, const GpVar& index, uint32_t shift = 0, int32_t disp = 0, uint32_t size = 0) { return Mem(base, index, shift, disp, size); }
//! Create a byte pointer operand.
static ASMJIT_INLINE Mem byte_ptr(const GpVar& base, const GpVar& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, kSizeByte); }
//! Create a word pointer operand.
static ASMJIT_INLINE Mem word_ptr(const GpVar& base, const GpVar& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, kSizeWord); }
//! Create a dword pointer operand.
static ASMJIT_INLINE Mem dword_ptr(const GpVar& base, const GpVar& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, kSizeDWord); }
//! Create a qword pointer operand.
static ASMJIT_INLINE Mem qword_ptr(const GpVar& base, const GpVar& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, kSizeQWord); }
//! Create a tword pointer operand.
static ASMJIT_INLINE Mem tword_ptr(const GpVar& base, const GpVar& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, kSizeTWord); }
//! Create a oword pointer operand.
static ASMJIT_INLINE Mem oword_ptr(const GpVar& base, const GpVar& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, kSizeOWord); }
//! Create a yword pointer operand.
static ASMJIT_INLINE Mem yword_ptr(const GpVar& base, const GpVar& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, kSizeYWord); }

// ============================================================================
// [asmjit::x86x64::Mem - ptr[base + (vex << shift) + disp]]
// ============================================================================

//! Create a custom pointer operand.
static ASMJIT_INLINE Mem ptr(const GpReg& base, const XmmReg& index, uint32_t shift = 0, int32_t disp = 0, uint32_t size = 0) { return Mem(base, index, shift, disp, size); }
//! Create a byte pointer operand.
static ASMJIT_INLINE Mem byte_ptr(const GpReg& base, const XmmReg& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, kSizeByte); }
//! Create a word pointer operand.
static ASMJIT_INLINE Mem word_ptr(const GpReg& base, const XmmReg& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, kSizeWord); }
//! Create a dword pointer operand.
static ASMJIT_INLINE Mem dword_ptr(const GpReg& base, const XmmReg& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, kSizeDWord); }
//! Create a qword pointer operand.
static ASMJIT_INLINE Mem qword_ptr(const GpReg& base, const XmmReg& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, kSizeQWord); }
//! Create a tword pointer operand.
static ASMJIT_INLINE Mem tword_ptr(const GpReg& base, const XmmReg& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, kSizeTWord); }
//! Create a oword pointer operand.
static ASMJIT_INLINE Mem oword_ptr(const GpReg& base, const XmmReg& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, kSizeOWord); }
//! Create a yword pointer operand.
static ASMJIT_INLINE Mem yword_ptr(const GpReg& base, const XmmReg& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, kSizeYWord); }

//! Create a custom pointer operand.
static ASMJIT_INLINE Mem ptr(const GpReg& base, const YmmReg& index, uint32_t shift = 0, int32_t disp = 0, uint32_t size = 0) { return Mem(base, index, shift, disp, size); }
//! Create a byte pointer operand.
static ASMJIT_INLINE Mem byte_ptr(const GpReg& base, const YmmReg& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, kSizeByte); }
//! Create a word pointer operand.
static ASMJIT_INLINE Mem word_ptr(const GpReg& base, const YmmReg& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, kSizeWord); }
//! Create a dword pointer operand.
static ASMJIT_INLINE Mem dword_ptr(const GpReg& base, const YmmReg& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, kSizeDWord); }
//! Create a qword pointer operand.
static ASMJIT_INLINE Mem qword_ptr(const GpReg& base, const YmmReg& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, kSizeQWord); }
//! Create a tword pointer operand.
static ASMJIT_INLINE Mem tword_ptr(const GpReg& base, const YmmReg& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, kSizeTWord); }
//! Create a oword pointer operand.
static ASMJIT_INLINE Mem oword_ptr(const GpReg& base, const YmmReg& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, kSizeOWord); }
//! Create a yword pointer operand.
static ASMJIT_INLINE Mem yword_ptr(const GpReg& base, const YmmReg& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, kSizeYWord); }

//! Create a custom pointer operand.
static ASMJIT_INLINE Mem ptr(const GpVar& base, const XmmVar& index, uint32_t shift = 0, int32_t disp = 0, uint32_t size = 0) { return Mem(base, index, shift, disp, size); }
//! Create a byte pointer operand.
static ASMJIT_INLINE Mem byte_ptr(const GpVar& base, const XmmVar& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, kSizeByte); }
//! Create a word pointer operand.
static ASMJIT_INLINE Mem word_ptr(const GpVar& base, const XmmVar& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, kSizeWord); }
//! Create a dword pointer operand.
static ASMJIT_INLINE Mem dword_ptr(const GpVar& base, const XmmVar& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, kSizeDWord); }
//! Create a qword pointer operand.
static ASMJIT_INLINE Mem qword_ptr(const GpVar& base, const XmmVar& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, kSizeQWord); }
//! Create a tword pointer operand.
static ASMJIT_INLINE Mem tword_ptr(const GpVar& base, const XmmVar& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, kSizeTWord); }
//! Create a oword pointer operand.
static ASMJIT_INLINE Mem oword_ptr(const GpVar& base, const XmmVar& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, kSizeOWord); }
//! Create a yword pointer operand.
static ASMJIT_INLINE Mem yword_ptr(const GpVar& base, const XmmVar& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, kSizeYWord); }

//! Create a custom pointer operand.
static ASMJIT_INLINE Mem ptr(const GpVar& base, const YmmVar& index, uint32_t shift = 0, int32_t disp = 0, uint32_t size = 0) { return Mem(base, index, shift, disp, size); }
//! Create a byte pointer operand.
static ASMJIT_INLINE Mem byte_ptr(const GpVar& base, const YmmVar& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, kSizeByte); }
//! Create a word pointer operand.
static ASMJIT_INLINE Mem word_ptr(const GpVar& base, const YmmVar& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, kSizeWord); }
//! Create a dword pointer operand.
static ASMJIT_INLINE Mem dword_ptr(const GpVar& base, const YmmVar& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, kSizeDWord); }
//! Create a qword pointer operand.
static ASMJIT_INLINE Mem qword_ptr(const GpVar& base, const YmmVar& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, kSizeQWord); }
//! Create a tword pointer operand.
static ASMJIT_INLINE Mem tword_ptr(const GpVar& base, const YmmVar& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, kSizeTWord); }
//! Create a oword pointer operand.
static ASMJIT_INLINE Mem oword_ptr(const GpVar& base, const YmmVar& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, kSizeOWord); }
//! Create a yword pointer operand.
static ASMJIT_INLINE Mem yword_ptr(const GpVar& base, const YmmVar& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, kSizeYWord); }

// ============================================================================
// [asmjit::x86x64::Mem - [label + disp]]
// ============================================================================

//! Create a custom pointer operand.
static ASMJIT_INLINE Mem ptr(const Label& label, int32_t disp = 0, uint32_t size = 0) { return Mem(label, disp, size); }
//! Create a byte pointer operand.
static ASMJIT_INLINE Mem byte_ptr(const Label& label, int32_t disp = 0) { return ptr(label, disp, kSizeByte); }
//! Create a word pointer operand.
static ASMJIT_INLINE Mem word_ptr(const Label& label, int32_t disp = 0) { return ptr(label, disp, kSizeWord); }
//! Create a dword pointer operand.
static ASMJIT_INLINE Mem dword_ptr(const Label& label, int32_t disp = 0) { return ptr(label, disp, kSizeDWord); }
//! Create a qword pointer operand.
static ASMJIT_INLINE Mem qword_ptr(const Label& label, int32_t disp = 0) { return ptr(label, disp, kSizeQWord); }
//! Create a tword pointer operand.
static ASMJIT_INLINE Mem tword_ptr(const Label& label, int32_t disp = 0) { return ptr(label, disp, kSizeTWord); }
//! Create a oword pointer operand.
static ASMJIT_INLINE Mem oword_ptr(const Label& label, int32_t disp = 0) { return ptr(label, disp, kSizeOWord); }
//! Create a yword pointer operand.
static ASMJIT_INLINE Mem yword_ptr(const Label& label, int32_t disp = 0) { return ptr(label, disp, kSizeYWord); }

// ============================================================================
// [asmjit::x86x64::Mem - [label + index << shift + disp]]
// ============================================================================

//! Create a custom pointer operand.
static ASMJIT_INLINE Mem ptr(const Label& label, const GpReg& index, uint32_t shift, int32_t disp = 0, uint32_t size = 0) { return Mem(label, index, shift, disp, size); }
//! Create a byte pointer operand.
static ASMJIT_INLINE Mem byte_ptr(const Label& label, const GpReg& index, uint32_t shift, int32_t disp = 0) { return ptr(label, index, shift, disp, kSizeByte); }
//! Create a word pointer operand.
static ASMJIT_INLINE Mem word_ptr(const Label& label, const GpReg& index, uint32_t shift, int32_t disp = 0) { return ptr(label, index, shift, disp, kSizeWord); }
//! Create a dword pointer operand.
static ASMJIT_INLINE Mem dword_ptr(const Label& label, const GpReg& index, uint32_t shift, int32_t disp = 0) { return ptr(label, index, shift, disp, kSizeDWord); }
//! Create a qword pointer operand.
static ASMJIT_INLINE Mem qword_ptr(const Label& label, const GpReg& index, uint32_t shift, int32_t disp = 0) { return ptr(label, index, shift, disp, kSizeQWord); }
//! Create a tword pointer operand.
static ASMJIT_INLINE Mem tword_ptr(const Label& label, const GpReg& index, uint32_t shift, int32_t disp = 0) { return ptr(label, index, shift, disp, kSizeTWord); }
//! Create a oword pointer operand.
static ASMJIT_INLINE Mem oword_ptr(const Label& label, const GpReg& index, uint32_t shift, int32_t disp = 0) { return ptr(label, index, shift, disp, kSizeOWord); }
//! Create a yword pointer operand.
static ASMJIT_INLINE Mem yword_ptr(const Label& label, const GpReg& index, uint32_t shift, int32_t disp = 0) { return ptr(label, index, shift, disp, kSizeYWord); }

//! Create a custom pointer operand.
static ASMJIT_INLINE Mem ptr(const Label& label, const GpVar& index, uint32_t shift, int32_t disp = 0, uint32_t size = 0) { return Mem(label, index, shift, disp, size); }
//! Create a byte pointer operand.
static ASMJIT_INLINE Mem byte_ptr(const Label& label, const GpVar& index, uint32_t shift, int32_t disp = 0) { return ptr(label, index, shift, disp, kSizeByte); }
//! Create a word pointer operand.
static ASMJIT_INLINE Mem word_ptr(const Label& label, const GpVar& index, uint32_t shift, int32_t disp = 0) { return ptr(label, index, shift, disp, kSizeWord); }
//! Create a dword pointer operand.
static ASMJIT_INLINE Mem dword_ptr(const Label& label, const GpVar& index, uint32_t shift, int32_t disp = 0) { return ptr(label, index, shift, disp, kSizeDWord); }
//! Create a qword pointer operand.
static ASMJIT_INLINE Mem qword_ptr(const Label& label, const GpVar& index, uint32_t shift, int32_t disp = 0) { return ptr(label, index, shift, disp, kSizeQWord); }
//! Create a tword pointer operand.
static ASMJIT_INLINE Mem tword_ptr(const Label& label, const GpVar& index, uint32_t shift, int32_t disp = 0) { return ptr(label, index, shift, disp, kSizeTWord); }
//! Create a oword pointer operand.
static ASMJIT_INLINE Mem oword_ptr(const Label& label, const GpVar& index, uint32_t shift, int32_t disp = 0) { return ptr(label, index, shift, disp, kSizeOWord); }
//! Create a yword pointer operand.
static ASMJIT_INLINE Mem yword_ptr(const Label& label, const GpVar& index, uint32_t shift, int32_t disp = 0) { return ptr(label, index, shift, disp, kSizeYWord); }

// ============================================================================
// [asmjit::x86x64::Mem - [ptr + disp]
// ============================================================================

//! Create a custom pointer operand.
ASMJIT_API Mem ptr_abs(Ptr pAbs, int32_t disp = 0, uint32_t size = 0);

//! Create a byte pointer operand.
static ASMJIT_INLINE Mem byte_ptr_abs(Ptr pAbs, int32_t disp = 0) { return ptr_abs(pAbs, disp, kSizeByte); }
//! Create a word pointer operand.
static ASMJIT_INLINE Mem word_ptr_abs(Ptr pAbs, int32_t disp = 0) { return ptr_abs(pAbs, disp, kSizeWord); }
//! Create a dword pointer operand.
static ASMJIT_INLINE Mem dword_ptr_abs(Ptr pAbs, int32_t disp = 0) { return ptr_abs(pAbs, disp, kSizeDWord); }
//! Create a qword pointer operand.
static ASMJIT_INLINE Mem qword_ptr_abs(Ptr pAbs, int32_t disp = 0) { return ptr_abs(pAbs, disp, kSizeQWord); }
//! Create a tword pointer operand (used for 80-bit floating points).
static ASMJIT_INLINE Mem tword_ptr_abs(Ptr pAbs, int32_t disp = 0) { return ptr_abs(pAbs, disp, kSizeTWord); }
//! Create a oword pointer operand.
static ASMJIT_INLINE Mem oword_ptr_abs(Ptr pAbs, int32_t disp = 0) { return ptr_abs(pAbs, disp, kSizeOWord); }
//! Create a yword pointer operand.
static ASMJIT_INLINE Mem yword_ptr_abs(Ptr pAbs, int32_t disp = 0) { return ptr_abs(pAbs, disp, kSizeYWord); }

// ============================================================================
// [asmjit::x86x64::Mem - [pAbs + index << shift + disp]
// ============================================================================

//! Create a custom pointer operand.
ASMJIT_API Mem ptr_abs(Ptr pAbs, const X86Reg& index, uint32_t shift = 0, int32_t disp = 0, uint32_t size = 0);
//! Create a custom pointer operand.
ASMJIT_API Mem ptr_abs(Ptr pAbs, const X86Var& index, uint32_t shift = 0, int32_t disp = 0, uint32_t size = 0);

//! Create a Byte pointer operand.
static ASMJIT_INLINE Mem byte_ptr_abs(Ptr pAbs, const GpReg& index, uint32_t shift = 0, int32_t disp = 0) { return ptr_abs(pAbs, index, shift, disp, kSizeByte); }
//! Create a word pointer operand.
static ASMJIT_INLINE Mem word_ptr_abs(Ptr pAbs, const GpReg& index, uint32_t shift = 0, int32_t disp = 0) { return ptr_abs(pAbs, index, shift, disp, kSizeWord); }
//! Create a dword pointer operand.
static ASMJIT_INLINE Mem dword_ptr_abs(Ptr pAbs, const GpReg& index, uint32_t shift = 0, int32_t disp = 0) { return ptr_abs(pAbs, index, shift, disp, kSizeDWord); }
//! Create a qword pointer operand.
static ASMJIT_INLINE Mem qword_ptr_abs(Ptr pAbs, const GpReg& index, uint32_t shift = 0, int32_t disp = 0) { return ptr_abs(pAbs, index, shift, disp, kSizeQWord); }
//! Create a tword pointer operand.
static ASMJIT_INLINE Mem tword_ptr_abs(Ptr pAbs, const GpReg& index, uint32_t shift = 0, int32_t disp = 0) { return ptr_abs(pAbs, index, shift, disp, kSizeTWord); }
//! Create a oword pointer operand.
static ASMJIT_INLINE Mem oword_ptr_abs(Ptr pAbs, const GpReg& index, uint32_t shift = 0, int32_t disp = 0) { return ptr_abs(pAbs, index, shift, disp, kSizeOWord); }
//! Create a yword pointer operand.
static ASMJIT_INLINE Mem yword_ptr_abs(Ptr pAbs, const GpReg& index, uint32_t shift = 0, int32_t disp = 0) { return ptr_abs(pAbs, index, shift, disp, kSizeYWord); }

//! Create a byte pointer operand.
static ASMJIT_INLINE Mem byte_ptr_abs(Ptr pAbs, const XmmReg& index, uint32_t shift = 0, int32_t disp = 0) { return ptr_abs(pAbs, index, shift, disp, kSizeByte); }
//! Create a word pointer operand.
static ASMJIT_INLINE Mem word_ptr_abs(Ptr pAbs, const XmmReg& index, uint32_t shift = 0, int32_t disp = 0) { return ptr_abs(pAbs, index, shift, disp, kSizeWord); }
//! Create a dword pointer operand.
static ASMJIT_INLINE Mem dword_ptr_abs(Ptr pAbs, const XmmReg& index, uint32_t shift = 0, int32_t disp = 0) { return ptr_abs(pAbs, index, shift, disp, kSizeDWord); }
//! Create a qword pointer operand.
static ASMJIT_INLINE Mem qword_ptr_abs(Ptr pAbs, const XmmReg& index, uint32_t shift = 0, int32_t disp = 0) { return ptr_abs(pAbs, index, shift, disp, kSizeQWord); }
//! Create a tword pointer operand.
static ASMJIT_INLINE Mem tword_ptr_abs(Ptr pAbs, const XmmReg& index, uint32_t shift = 0, int32_t disp = 0) { return ptr_abs(pAbs, index, shift, disp, kSizeTWord); }
//! Create a oword pointer operand.
static ASMJIT_INLINE Mem oword_ptr_abs(Ptr pAbs, const XmmReg& index, uint32_t shift = 0, int32_t disp = 0) { return ptr_abs(pAbs, index, shift, disp, kSizeOWord); }
//! Create a yword pointer operand.
static ASMJIT_INLINE Mem yword_ptr_abs(Ptr pAbs, const XmmReg& index, uint32_t shift = 0, int32_t disp = 0) { return ptr_abs(pAbs, index, shift, disp, kSizeYWord); }

//! Create a byte pointer operand.
static ASMJIT_INLINE Mem byte_ptr_abs(Ptr pAbs, const YmmReg& index, uint32_t shift = 0, int32_t disp = 0) { return ptr_abs(pAbs, index, shift, disp, kSizeByte); }
//! Create a word pointer operand.
static ASMJIT_INLINE Mem word_ptr_abs(Ptr pAbs, const YmmReg& index, uint32_t shift = 0, int32_t disp = 0) { return ptr_abs(pAbs, index, shift, disp, kSizeWord); }
//! Create a dword pointer operand.
static ASMJIT_INLINE Mem dword_ptr_abs(Ptr pAbs, const YmmReg& index, uint32_t shift = 0, int32_t disp = 0) { return ptr_abs(pAbs, index, shift, disp, kSizeDWord); }
//! Create a qword pointer operand.
static ASMJIT_INLINE Mem qword_ptr_abs(Ptr pAbs, const YmmReg& index, uint32_t shift = 0, int32_t disp = 0) { return ptr_abs(pAbs, index, shift, disp, kSizeQWord); }
//! Create a tword pointer operand.
static ASMJIT_INLINE Mem tword_ptr_abs(Ptr pAbs, const YmmReg& index, uint32_t shift = 0, int32_t disp = 0) { return ptr_abs(pAbs, index, shift, disp, kSizeTWord); }
//! Create a oword pointer operand.
static ASMJIT_INLINE Mem oword_ptr_abs(Ptr pAbs, const YmmReg& index, uint32_t shift = 0, int32_t disp = 0) { return ptr_abs(pAbs, index, shift, disp, kSizeOWord); }
//! Create a yword pointer operand.
static ASMJIT_INLINE Mem yword_ptr_abs(Ptr pAbs, const YmmReg& index, uint32_t shift = 0, int32_t disp = 0) { return ptr_abs(pAbs, index, shift, disp, kSizeYWord); }

//! Create a byte pointer operand.
static ASMJIT_INLINE Mem byte_ptr_abs(Ptr pAbs, const XmmVar& index, uint32_t shift = 0, int32_t disp = 0) { return ptr_abs(pAbs, index, shift, disp, kSizeByte); }
//! Create a word pointer operand.
static ASMJIT_INLINE Mem word_ptr_abs(Ptr pAbs, const XmmVar& index, uint32_t shift = 0, int32_t disp = 0) { return ptr_abs(pAbs, index, shift, disp, kSizeWord); }
//! Create a dword pointer operand.
static ASMJIT_INLINE Mem dword_ptr_abs(Ptr pAbs, const XmmVar& index, uint32_t shift = 0, int32_t disp = 0) { return ptr_abs(pAbs, index, shift, disp, kSizeDWord); }
//! Create a qword pointer operand.
static ASMJIT_INLINE Mem qword_ptr_abs(Ptr pAbs, const XmmVar& index, uint32_t shift = 0, int32_t disp = 0) { return ptr_abs(pAbs, index, shift, disp, kSizeQWord); }
//! Create a tword pointer operand.
static ASMJIT_INLINE Mem tword_ptr_abs(Ptr pAbs, const XmmVar& index, uint32_t shift = 0, int32_t disp = 0) { return ptr_abs(pAbs, index, shift, disp, kSizeTWord); }
//! Create a oword pointer operand.
static ASMJIT_INLINE Mem oword_ptr_abs(Ptr pAbs, const XmmVar& index, uint32_t shift = 0, int32_t disp = 0) { return ptr_abs(pAbs, index, shift, disp, kSizeOWord); }
//! Create a yword pointer operand.
static ASMJIT_INLINE Mem yword_ptr_abs(Ptr pAbs, const XmmVar& index, uint32_t shift = 0, int32_t disp = 0) { return ptr_abs(pAbs, index, shift, disp, kSizeYWord); }

//! Create a byte pointer operand.
static ASMJIT_INLINE Mem byte_ptr_abs(Ptr pAbs, const YmmVar& index, uint32_t shift = 0, int32_t disp = 0) { return ptr_abs(pAbs, index, shift, disp, kSizeByte); }
//! Create a word pointer operand.
static ASMJIT_INLINE Mem word_ptr_abs(Ptr pAbs, const YmmVar& index, uint32_t shift = 0, int32_t disp = 0) { return ptr_abs(pAbs, index, shift, disp, kSizeWord); }
//! Create a dword pointer operand.
static ASMJIT_INLINE Mem dword_ptr_abs(Ptr pAbs, const YmmVar& index, uint32_t shift = 0, int32_t disp = 0) { return ptr_abs(pAbs, index, shift, disp, kSizeDWord); }
//! Create a qword pointer operand.
static ASMJIT_INLINE Mem qword_ptr_abs(Ptr pAbs, const YmmVar& index, uint32_t shift = 0, int32_t disp = 0) { return ptr_abs(pAbs, index, shift, disp, kSizeQWord); }
//! Create a tword pointer operand.
static ASMJIT_INLINE Mem tword_ptr_abs(Ptr pAbs, const YmmVar& index, uint32_t shift = 0, int32_t disp = 0) { return ptr_abs(pAbs, index, shift, disp, kSizeTWord); }
//! Create a oword pointer operand.
static ASMJIT_INLINE Mem oword_ptr_abs(Ptr pAbs, const YmmVar& index, uint32_t shift = 0, int32_t disp = 0) { return ptr_abs(pAbs, index, shift, disp, kSizeOWord); }
//! Create a yword pointer operand.
static ASMJIT_INLINE Mem yword_ptr_abs(Ptr pAbs, const YmmVar& index, uint32_t shift = 0, int32_t disp = 0) { return ptr_abs(pAbs, index, shift, disp, kSizeYWord); }

// ============================================================================
// [asmjit::x86x64::Util]
// ============================================================================

static ASMJIT_INLINE bool x86IsGpbRegOp(const Operand* op) {
  const uint32_t mask = IntUtil::pack32_2x8_1x16(0xFF, 0xFF, ~(kRegTypePatchedGpbHi << 8) & 0xFF00);
  return (op->_packed[0].u32[0] & mask) == IntUtil::pack32_2x8_1x16(kOperandTypeReg, 1, 0x0000);
}

static ASMJIT_INLINE uint32_t x86VarTypeToClass(uint32_t vType) {
  // Getting varClass is the only safe operation when dealing with denormalized
  // varType. Any other property would require to map vType to the architecture
  // specific one.
  ASMJIT_ASSERT(vType < kVarTypeCount);
  return _varInfo[vType].getClass();
}

//! @}

} // x86x64 namespace
} // asmjit namespace

// ============================================================================
// [asmjit::x86]
// ============================================================================

#if defined(ASMJIT_BUILD_X86)

namespace asmjit {
namespace x86 {

// This is the only place where the x86x64 namespace is included into x86.
using namespace ::asmjit::x86x64;

//! @addtogroup asmjit_x86x64
//! @{

// ============================================================================
// [asmjit::x86::kRegType]
// ============================================================================

//! @internal
ASMJIT_ENUM(kRegType) {
  //! Gpd register.
  kRegTypeGpz = kRegTypeGpd
};

// ============================================================================
// [asmjit::x86::kRegCount]
// ============================================================================

//! X86 registers count per class.
ASMJIT_ENUM(kRegCount) {
  //! Base count of registers (8).
  kRegCountBase = 8,
  //! Count of Gp registers (8).
  kRegCountGp = kRegCountBase,
  //! Count of Xmm registers (8).
  kRegCountXmm = kRegCountBase,
  //! Count of Ymm registers (8).
  kRegCountYmm = kRegCountBase
};

// ============================================================================
// [asmjit::x86::Variables]
// ============================================================================

//! @internal
//!
//! Mapping of x86 variables into their real IDs.
//!
//! This mapping translates the following:
//! - `kVarTypeInt64` to `kVarTypeInvalid`.
//! - `kVarTypeUInt64` to `kVarTypeInvalid`.
//! - `kVarTypeIntPtr` to `kVarTypeInt32`.
//! - `kVarTypeUIntPtr` to `kVarTypeUInt32`.
ASMJIT_VAR const uint8_t _varMapping[kVarTypeCount];

// ============================================================================
// [asmjit::x86::Registers]
// ============================================================================

//! Gpd register.
ASMJIT_VAR const GpReg zax;
//! Gpd register.
ASMJIT_VAR const GpReg zcx;
//! Gpd register.
ASMJIT_VAR const GpReg zdx;
//! Gpd register.
ASMJIT_VAR const GpReg zbx;
//! Gpd register.
ASMJIT_VAR const GpReg zsp;
//! Gpd register.
ASMJIT_VAR const GpReg zbp;
//! Gpd register.
ASMJIT_VAR const GpReg zsi;
//! Gpd register.
ASMJIT_VAR const GpReg zdi;

//! Get Gp qword register.
static ASMJIT_INLINE GpReg gpz(uint32_t index) { return GpReg(kRegTypeGpd, index, 4); }

// ============================================================================
// [asmjit::x86::Mem]
// ============================================================================

//! Create an intptr_t 32-bit pointer operand.
static ASMJIT_INLINE Mem intptr_ptr(const GpReg& base, int32_t disp = 0) { return ptr(base, disp, 4); }
//! Create an intptr_t 32-bit pointer operand.
static ASMJIT_INLINE Mem intptr_ptr(const GpVar& base, int32_t disp = 0) { return ptr(base, disp, 4); }
//! Create an intptr_t 32-bit pointer operand.
static ASMJIT_INLINE Mem intptr_ptr(const GpReg& base, const GpReg& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, 4); }
//! Create an intptr_t 32-bit pointer operand.
static ASMJIT_INLINE Mem intptr_ptr(const GpVar& base, const GpVar& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, 4); }

//! Create an intptr_t 32-bit pointer operand.
static ASMJIT_INLINE Mem intptr_ptr(const Label& label, int32_t disp = 0) { return ptr(label, disp, 4); }
//! Create an intptr_t 32-bit pointer operand.
static ASMJIT_INLINE Mem intptr_ptr(const Label& label, const GpReg& index, uint32_t shift, int32_t disp = 0) { return ptr(label, index, shift, disp, 4); }
//! Create an intptr_t 32-bit pointer operand.
static ASMJIT_INLINE Mem intptr_ptr(const Label& label, const GpVar& index, uint32_t shift, int32_t disp = 0) { return ptr(label, index, shift, disp, 4); }

//! Create an intptr_t 32-bit pointer operand.
static ASMJIT_INLINE Mem intptr_ptr_abs(Ptr pAbs, int32_t disp = 0) { return ptr_abs(pAbs, disp, 4); }
//! Create an intptr_t 32-bit pointer operand.
static ASMJIT_INLINE Mem intptr_ptr_abs(Ptr pAbs, const GpReg& index, uint32_t shift, int32_t disp = 0) { return ptr_abs(pAbs, index, shift, disp, 4); }
//! Create an intptr_t 32-bit pointer operand.
static ASMJIT_INLINE Mem intptr_ptr_abs(Ptr pAbs, const GpVar& index, uint32_t shift, int32_t disp = 0) { return ptr_abs(pAbs, index, shift, disp, 4); }

//! @}

} // x86 namespace
} // asmjit namespace

#endif // ASMJIT_BUILD_X86

// ============================================================================
// [asmjit::x64]
// ============================================================================

#if defined(ASMJIT_BUILD_X64)

namespace asmjit {
namespace x64 {

// This is the only place where the x86x64 namespace is included into x64.
using namespace ::asmjit::x86x64;

//! @addtogroup asmjit_x86x64
//! @{

// ============================================================================
// [asmjit::x64::kRegType]
// ============================================================================

//! @internal
ASMJIT_ENUM(kRegType) {
  //! Gpq register.
  kRegTypeGpz = kRegTypeGpq
};

// ============================================================================
// [asmjit::x64::kRegCount]
// ============================================================================

//! X86 registers count per class.
ASMJIT_ENUM(kRegCount) {
  //! Base count of registers (16).
  kRegCountBase = 16,
  //! Count of Gp registers (16).
  kRegCountGp = kRegCountBase,
  //! Count of Xmm registers (16).
  kRegCountXmm = kRegCountBase,
  //! Count of Ymm registers (16).
  kRegCountYmm = kRegCountBase
};

// ============================================================================
// [asmjit::x64::Variables]
// ============================================================================

//! @internal
//!
//! Mapping of x64 variables into their real IDs.
//!
//! This mapping translates the following:
//! - `kVarTypeIntPtr` to `kVarTypeInt64`.
//! - `kVarTypeUIntPtr` to `kVarTypeUInt64`.
ASMJIT_VAR const uint8_t _varMapping[kVarTypeCount];

// ============================================================================
// [asmjit::x64::Registers]
// ============================================================================

//! Gpb register.
ASMJIT_VAR const GpReg spl;
//! Gpb register.
ASMJIT_VAR const GpReg bpl;
//! Gpb register.
ASMJIT_VAR const GpReg sil;
//! Gpb register.
ASMJIT_VAR const GpReg dil;
//! Gpb register.
ASMJIT_VAR const GpReg r8b;
//! Gpb register.
ASMJIT_VAR const GpReg r9b;
//! Gpb register.
ASMJIT_VAR const GpReg r10b;
//! Gpb register.
ASMJIT_VAR const GpReg r11b;
//! Gpb register.
ASMJIT_VAR const GpReg r12b;
//! Gpb register.
ASMJIT_VAR const GpReg r13b;
//! Gpb register.
ASMJIT_VAR const GpReg r14b;
//! Gpb register.
ASMJIT_VAR const GpReg r15b;

//! Gpw register.
ASMJIT_VAR const GpReg r8w;
//! Gpw register.
ASMJIT_VAR const GpReg r9w;
//! Gpw register.
ASMJIT_VAR const GpReg r10w;
//! Gpw register.
ASMJIT_VAR const GpReg r11w;
//! Gpw register.
ASMJIT_VAR const GpReg r12w;
//! Gpw register.
ASMJIT_VAR const GpReg r13w;
//! Gpw register.
ASMJIT_VAR const GpReg r14w;
//! Gpw register.
ASMJIT_VAR const GpReg r15w;

//! Gpd register.
ASMJIT_VAR const GpReg r8d;
//! Gpd register.
ASMJIT_VAR const GpReg r9d;
//! Gpd register.
ASMJIT_VAR const GpReg r10d;
//! Gpd register.
ASMJIT_VAR const GpReg r11d;
//! Gpd register.
ASMJIT_VAR const GpReg r12d;
//! Gpd register.
ASMJIT_VAR const GpReg r13d;
//! Gpd register.
ASMJIT_VAR const GpReg r14d;
//! Gpd register.
ASMJIT_VAR const GpReg r15d;

//! Gpq register.
ASMJIT_VAR const GpReg rax;
//! Gpq register.
ASMJIT_VAR const GpReg rcx;
//! Gpq register.
ASMJIT_VAR const GpReg rdx;
//! Gpq register.
ASMJIT_VAR const GpReg rbx;
//! Gpq register.
ASMJIT_VAR const GpReg rsp;
//! Gpq register.
ASMJIT_VAR const GpReg rbp;
//! Gpq register.
ASMJIT_VAR const GpReg rsi;
//! Gpq register.
ASMJIT_VAR const GpReg rdi;

//! Gpq register.
ASMJIT_VAR const GpReg r8;
//! Gpq register.
ASMJIT_VAR const GpReg r9;
//! Gpq register.
ASMJIT_VAR const GpReg r10;
//! Gpq register.
ASMJIT_VAR const GpReg r11;
//! Gpq register.
ASMJIT_VAR const GpReg r12;
//! Gpq register.
ASMJIT_VAR const GpReg r13;
//! Gpq register.
ASMJIT_VAR const GpReg r14;
//! Gpq register.
ASMJIT_VAR const GpReg r15;

//! Gpq register.
ASMJIT_VAR const GpReg zax;
//! Gpq register.
ASMJIT_VAR const GpReg zcx;
//! Gpq register.
ASMJIT_VAR const GpReg zdx;
//! Gpq register.
ASMJIT_VAR const GpReg zbx;
//! Gpq register.
ASMJIT_VAR const GpReg zsp;
//! Gpq register.
ASMJIT_VAR const GpReg zbp;
//! Gpq register.
ASMJIT_VAR const GpReg zsi;
//! Gpq register.
ASMJIT_VAR const GpReg zdi;

//! Xmm register.
ASMJIT_VAR const XmmReg xmm8;
//! Xmm register.
ASMJIT_VAR const XmmReg xmm9;
//! Xmm register.
ASMJIT_VAR const XmmReg xmm10;
//! Xmm register.
ASMJIT_VAR const XmmReg xmm11;
//! Xmm register.
ASMJIT_VAR const XmmReg xmm12;
//! Xmm register.
ASMJIT_VAR const XmmReg xmm13;
//! Xmm register.
ASMJIT_VAR const XmmReg xmm14;
//! Xmm register.
ASMJIT_VAR const XmmReg xmm15;

//! Ymm register.
ASMJIT_VAR const YmmReg ymm8;
//! Ymm register.
ASMJIT_VAR const YmmReg ymm9;
//! Ymm register.
ASMJIT_VAR const YmmReg ymm10;
//! Ymm register.
ASMJIT_VAR const YmmReg ymm11;
//! Ymm register.
ASMJIT_VAR const YmmReg ymm12;
//! Ymm register.
ASMJIT_VAR const YmmReg ymm13;
//! Ymm register.
ASMJIT_VAR const YmmReg ymm14;
//! Ymm register.
ASMJIT_VAR const YmmReg ymm15;

//! Get Gpq register.
static ASMJIT_INLINE GpReg gpq(uint32_t index) { return GpReg(kRegTypeGpq, index, 8); }
//! Get Gpq register.
static ASMJIT_INLINE GpReg gpz(uint32_t index) { return GpReg(kRegTypeGpq, index, 8); }

// ============================================================================
// [asmjit::x64::Mem]
// ============================================================================

//! Create an intptr_t 64-bit pointer operand.
static ASMJIT_INLINE Mem intptr_ptr(const Label& label, int32_t disp = 0) { return ptr(label, disp, 8); }
//! Create an intptr_t 64-bit pointer operand.
static ASMJIT_INLINE Mem intptr_ptr(const Label& label, const GpReg& index, uint32_t shift, int32_t disp = 0) { return ptr(label, index, shift, disp, 8); }
//! Create an intptr_t 64-bit pointer operand.
static ASMJIT_INLINE Mem intptr_ptr(const Label& label, const GpVar& index, uint32_t shift, int32_t disp = 0) { return ptr(label, index, shift, disp, 8); }

//! Create an intptr_t 64-bit pointer operand.
static ASMJIT_INLINE Mem intptr_ptr_abs(Ptr pAbs, int32_t disp = 0) { return ptr_abs(pAbs, disp, 8); }
//! Create an intptr_t 64-bit pointer operand.
static ASMJIT_INLINE Mem intptr_ptr_abs(Ptr pAbs, const GpReg& index, uint32_t shift, int32_t disp = 0) { return ptr_abs(pAbs, index, shift, disp, 8); }
//! Create an intptr_t 64-bit pointer operand.
static ASMJIT_INLINE Mem intptr_ptr_abs(Ptr pAbs, const GpVar& index, uint32_t shift, int32_t disp = 0) { return ptr_abs(pAbs, index, shift, disp, 8); }

//! Create an intptr_t 64-bit pointer operand.
static ASMJIT_INLINE Mem intptr_ptr(const GpReg& base, int32_t disp = 0) { return ptr(base, disp, 8); }
//! Create an intptr_t 64-bit pointer operand.
static ASMJIT_INLINE Mem intptr_ptr(const GpVar& base, int32_t disp = 0) { return ptr(base, disp, 8); }
//! Create an intptr_t 64-bit pointer operand.
static ASMJIT_INLINE Mem intptr_ptr(const GpReg& base, const GpReg& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, 8); }
//! Create an intptr_t 64-bit pointer operand.
static ASMJIT_INLINE Mem intptr_ptr(const GpVar& base, const GpVar& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, 8); }

//! @}

} // x64 namespace
} // asmjit namespace

#endif // ASMJIT_BUILD_X64

// [Api-End]
#include "../apiend.h"

// [Guard]
#endif // _ASMJIT_X86_X86DEFS_H
