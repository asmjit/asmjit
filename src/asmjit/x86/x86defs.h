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
#include "../base/assert.h"
#include "../base/compiler.h"
#include "../base/defs.h"
#include "../base/intutil.h"
#include "../base/vectypes.h"

// [Api-Begin]
#include "../base/apibegin.h"

namespace asmjit {
namespace x86x64 {

//! @addtogroup asmjit_x86x64
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
//! @brief X86/X64 instructions' names.
ASMJIT_VAR const char _instName[];

//! @internal
//!
//! @brief X86/X64 instructions' information.
ASMJIT_VAR const InstInfo _instInfo[];

//! @internal
//!
//! @brief X86/X64 condition codes to reversed condition codes map.
ASMJIT_VAR const uint32_t _reverseCond[20];

//! @internal
//!
//! @brief X86X64 condition codes to "cmovcc" group map.
ASMJIT_VAR const uint32_t _condToCmovcc[20];

//! @internal
//!
//! @brief X86X64 condition codes to "jcc" group map.
ASMJIT_VAR const uint32_t _condToJcc[20];

//! @internal
//!
//! @brief X86X64 condition codes to "setcc" group map.
ASMJIT_VAR const uint32_t _condToSetcc[20];

// ============================================================================
// [asmjit::x86x64::Variables]
// ============================================================================

ASMJIT_VAR const VarInfo _varInfo[];

// ============================================================================
// [asmjit::x86x64::kRegClass]
// ============================================================================

//! @brief X86 variable class.
ASMJIT_ENUM(kRegClass) {
  // kRegClassGp defined in base/defs.h; it's used by all implementations.

  //! @brief X86/X64 Fp register class.
  kRegClassFp = 1,
  //! @brief X86/X64 Mm register class.
  kRegClassMm = 2,
  //! @brief X86/X64 Xmm/Ymm register class.
  kRegClassXy = 3,

  //! @brief Count of X86/X64 register classes.
  kRegClassCount = 4
};

// ============================================================================
// [asmjit::x86x64::kRegCount]
// ============================================================================

ASMJIT_ENUM(kRegCount) {
  //! @brief Count of Fp registers (8).
  kRegCountFp = 8,
  //! @brief Count of Mm registers (8).
  kRegCountMm = 8,
  //! @brief Count of segment registers (6).
  kRegCountSeg = 6
};

// ============================================================================
// [asmjit::x86x64::kRegType]
// ============================================================================

//! @brief X86 register types.
ASMJIT_ENUM(kRegType) {
  //! @brief Gpb-lo register (AL, BL, CL, DL, ...).
  kRegTypeGpbLo = 0x01,
  //! @brief Gpb-hi register (AH, BH, CH, DH only).
  kRegTypeGpbHi = 0x02,

  //! @internal
  //!
  //! @brief Gpb-hi register patched to native index (4-7).
  kRegTypePatchedGpbHi = kRegTypeGpbLo | kRegTypeGpbHi,

  //! @brief Gpw register.
  kRegTypeGpw = 0x10,
  //! @brief Gpd register.
  kRegTypeGpd = 0x20,
  //! @brief Gpq register.
  kRegTypeGpq = 0x30,

  //! @brief Fp register.
  kRegTypeFp = 0x50,
  //! @brief Mm register.
  kRegTypeMm = 0x60,

  //! @brief Xmm register.
  kRegTypeXmm = 0x70,
  //! @brief Ymm register.
  kRegTypeYmm = 0x80,
  //! @brief Zmm register.
  kRegTypeZmm = 0x90,

  //! @brief Segment register.
  kRegTypeSeg = 0xF0
};

// ============================================================================
// [asmjit::x86x64::kRegIndex]
// ============================================================================

//! @brief X86 register indices.
//!
//! These codes are real, don't miss with @c REG enum! and don't use these
//! values if you are not writing AsmJit code.
ASMJIT_ENUM(kRegIndex) {
  //! @brief Index of AL/AH/AX/EAX/RAX registers.
  kRegIndexAx = 0,
  //! @brief Index of CL/CH/CX/ECX/RCX registers.
  kRegIndexCx = 1,
  //! @brief Index of DL/DH/DX/EDX/RDX registers.
  kRegIndexDx = 2,
  //! @brief Index of BL/BH/BX/EBX/RBX registers.
  kRegIndexBx = 3,
  //! @brief Index of SPL/SP/ESP/RSP registers.
  kRegIndexSp = 4,
  //! @brief Index of BPL/BP/EBP/RBP registers.
  kRegIndexBp = 5,
  //! @brief Index of SIL/SI/ESI/RSI registers.
  kRegIndexSi = 6,
  //! @brief Index of DIL/DI/EDI/RDI registers.
  kRegIndexDi = 7,
  //! @brief Index of R8B/R8W/R8D/R8 registers (64-bit only).
  kRegIndexR8 = 8,
  //! @brief Index of R9B/R9W/R9D/R9 registers (64-bit only).
  kRegIndexR9 = 9,
  //! @brief Index of R10B/R10W/R10D/R10 registers (64-bit only).
  kRegIndexR10 = 10,
  //! @brief Index of R11B/R11W/R11D/R11 registers (64-bit only).
  kRegIndexR11 = 11,
  //! @brief Index of R12B/R12W/R12D/R12 registers (64-bit only).
  kRegIndexR12 = 12,
  //! @brief Index of R13B/R13W/R13D/R13 registers (64-bit only).
  kRegIndexR13 = 13,
  //! @brief Index of R14B/R14W/R14D/R14 registers (64-bit only).
  kRegIndexR14 = 14,
  //! @brief Index of R15B/R15W/R15D/R15 registers (64-bit only).
  kRegIndexR15 = 15,

  //! @brief Index of FP0 register.
  kRegIndexFp0 = 0,
  //! @brief Index of FP1 register.
  kRegIndexFp1 = 1,
  //! @brief Index of FP2 register.
  kRegIndexFp2 = 2,
  //! @brief Index of FP3 register.
  kRegIndexFp3 = 3,
  //! @brief Index of FP4 register.
  kRegIndexFp4 = 4,
  //! @brief Index of FP5 register.
  kRegIndexFp5 = 5,
  //! @brief Index of FP6 register.
  kRegIndexFp6 = 6,
  //! @brief Index of FP7 register.
  kRegIndexFp7 = 7,

  //! @brief Index of MM0 register.
  kRegIndexMm0 = 0,
  //! @brief Index of MM1 register.
  kRegIndexMm1 = 1,
  //! @brief Index of MM2 register.
  kRegIndexMm2 = 2,
  //! @brief Index of MM3 register.
  kRegIndexMm3 = 3,
  //! @brief Index of MM4 register.
  kRegIndexMm4 = 4,
  //! @brief Index of MM5 register.
  kRegIndexMm5 = 5,
  //! @brief Index of MM6 register.
  kRegIndexMm6 = 6,
  //! @brief Index of MM7 register.
  kRegIndexMm7 = 7,

  //! @brief Index of XMM0 register.
  kRegIndexXmm0 = 0,
  //! @brief Index of XMM1 register.
  kRegIndexXmm1 = 1,
  //! @brief Index of XMM2 register.
  kRegIndexXmm2 = 2,
  //! @brief Index of XMM3 register.
  kRegIndexXmm3 = 3,
  //! @brief Index of XMM4 register.
  kRegIndexXmm4 = 4,
  //! @brief Index of XMM5 register.
  kRegIndexXmm5 = 5,
  //! @brief Index of XMM6 register.
  kRegIndexXmm6 = 6,
  //! @brief Index of XMM7 register.
  kRegIndexXmm7 = 7,
  //! @brief Index of XMM8 register (64-bit only).
  kRegIndexXmm8 = 8,
  //! @brief Index of XMM9 register (64-bit only).
  kRegIndexXmm9 = 9,
  //! @brief Index of XMM10 register (64-bit only).
  kRegIndexXmm10 = 10,
  //! @brief Index of XMM11 register (64-bit only).
  kRegIndexXmm11 = 11,
  //! @brief Index of XMM12 register (64-bit only).
  kRegIndexXmm12 = 12,
  //! @brief Index of XMM13 register (64-bit only).
  kRegIndexXmm13 = 13,
  //! @brief Index of XMM14 register (64-bit only).
  kRegIndexXmm14 = 14,
  //! @brief Index of XMM15 register (64-bit only).
  kRegIndexXmm15 = 15,

  //! @brief Index of YMM0 register.
  kRegIndexYmm0 = 0,
  //! @brief Index of YMM1 register.
  kRegIndexYmm1 = 1,
  //! @brief Index of YMM2 register.
  kRegIndexYmm2 = 2,
  //! @brief Index of YMM3 register.
  kRegIndexYmm3 = 3,
  //! @brief Index of YMM4 register.
  kRegIndexYmm4 = 4,
  //! @brief Index of YMM5 register.
  kRegIndexYmm5 = 5,
  //! @brief Index of YMM6 register.
  kRegIndexYmm6 = 6,
  //! @brief Index of YMM7 register.
  kRegIndexYmm7 = 7,
  //! @brief Index of YMM8 register (64-bit only).
  kRegIndexYmm8 = 8,
  //! @brief Index of YMM9 register (64-bit only).
  kRegIndexYmm9 = 9,
  //! @brief Index of YMM10 register (64-bit only).
  kRegIndexYmm10 = 10,
  //! @brief Index of YMM11 register (64-bit only).
  kRegIndexYmm11 = 11,
  //! @brief Index of YMM12 register (64-bit only).
  kRegIndexYmm12 = 12,
  //! @brief Index of YMM13 register (64-bit only).
  kRegIndexYmm13 = 13,
  //! @brief Index of YMM14 register (64-bit only).
  kRegIndexYmm14 = 14,
  //! @brief Index of YMM15 register (64-bit only).
  kRegIndexYmm15 = 15
};

// ============================================================================
// [asmjit::x86x64::kSeg]
// ============================================================================

//! @brief X86 segment codes.
ASMJIT_ENUM(kSeg) {
  //! @brief No segment.
  kSegDefault = 0,
  //! @brief Es segment.
  kSegEs = 1,
  //! @brief Cs segment.
  kSegCs = 2,
  //! @brief Ss segment.
  kSegSs = 3,
  //! @brief Ds segment.
  kSegDs = 4,
  //! @brief Fs segment.
  kSegFs = 5,
  //! @brief Gs segment.
  kSegGs = 6
};

// ============================================================================
// [asmjit::x86x64::kMemVSib]
// ============================================================================

//! @brief X86 index register legacy and AVX2 (VSIB) support.
ASMJIT_ENUM(kMemVSib) {
  //! @brief Memory operand uses Gp or no index register.
  kMemVSibGpz = 0,
  //! @brief Memory operand uses Xmm or no index register.
  kMemVSibXmm = 1,
  //! @brief Memory operand uses Ymm or no index register.
  kMemVSibYmm = 2
};

// ============================================================================
// [asmjit::x86x64::kMemFlags]
// ============================================================================

//! @internal
//!
//! @brief X86 specific memory flags.
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

//! @brief X86 Prefetch hints.
ASMJIT_ENUM(kPrefetchHint) {
  //! @brief Prefetch using NT hint.
  kPrefetchNta = 0,
  //! @brief Prefetch to L0 cache.
  kPrefetchT0 = 1,
  //! @brief Prefetch to L1 cache.
  kPrefetchT1 = 2,
  //! @brief Prefetch to L2 cache.
  kPrefetchT2 = 3
};

// ============================================================================
// [asmjit::x86x64::kFPSW]
// ============================================================================

//! @brief X86 FPU status Word.
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

//! @brief X86 FPU control Word.
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

//! @brief X86 instruction codes.
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
  kInstVperm2f128,      // AVX
  kInstVperm2i128,      // AVX2
  kInstVpermd,          // AVX2
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
  kInstVphaddd,         // AVX2
  kInstVphaddsw,        // AVX2
  kInstVphaddw,         // AVX2
  kInstVphminposuw,     // AVX
  kInstVphsubd,         // AVX2
  kInstVphsubsw,        // AVX2
  kInstVphsubw,         // AVX2
  kInstVpinsrb,         // AVX
  kInstVpinsrd,         // AVX
  kInstVpinsrq,         // AVX (x64 only)
  kInstVpinsrw,         // AVX
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
  kInstVpsadbw,         // AVX/AVX2
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

//! @brief Instruction emit options, mainly for internal purposes.
ASMJIT_ENUM(kInstOptions) {
  //! @brief Emit instruction with LOCK prefix.
  //!
  //! If this option is used and instruction doesn't support LOCK prefix an
  //! invalid instruction error is generated.
  kInstOptionLock = 0x10,

  //! @brief Force REX prefix to be emitted.
  //!
  //! This option should be used carefully, because there are unencodable
  //! combinations. If you want to access ah, bh, ch or dh registers the REX
  //! prefix can't be emitted, otherwise illegal instruction error will be
  //! returned.
  kInstOptionRex = 0x40,

  //! @brief Force three-byte VEX prefix to be emitted (instead of more compact
  //! two-byte VEX prefix).
  //!
  //! Ignored if the instruction is not AVX instruction or extension to the
  //! instruction set that is encoded by using VEX prefix.
  kInstOptionVex3 = 0x80
};

// ============================================================================
// [asmjit::x86x64::kInstGroup]
// ============================================================================

//! @brief X86 instruction groups.
//!
//! This should be only used by assembler, because it's @c asmjit::Assembler
//! specific grouping. Each group represents one 'case' in the Assembler's
//! main emit method.
ASMJIT_ENUM(kInstGroup) {
  //! @brief Never used.
  kInstGroupNone,

  kInstGroupX86Op,
  kInstGroupX86Rm,
  kInstGroupX86Rm_B,
  kInstGroupX86RmReg,
  kInstGroupX86RegRm,
  kInstGroupX86M,
  //! @brief Adc/Add/And/Cmp/Or/Sbb/Sub/Xor.
  kInstGroupX86Arith,
  //! @brief Bswap.
  kInstGroupX86BSwap,
  //! @brief Bt/Btc/Btr/Bts.
  kInstGroupX86BTest,
  //! @brief Call.
  kInstGroupX86Call,
  //! @brief Enter.
  kInstGroupX86Enter,
  //! @brief Imul.
  kInstGroupX86Imul,
  //! @brief Inc/Dec.
  kInstGroupX86IncDec,
  //! @brief Int.
  kInstGroupX86Int,
  //! @brief Jcc.
  kInstGroupX86Jcc,
  //! @brief Jmp.
  kInstGroupX86Jmp,
  //! @brief Lea.
  kInstGroupX86Lea,
  //! @brief Mov.
  kInstGroupX86Mov,
  //! @brief Movsx/Movsxd/Movzx.
  kInstGroupX86MovSxZx,
  //! @brief Mov having absolute memory operand (x86/x64).
  kInstGroupX86MovPtr,
  //! @brief Push.
  kInstGroupX86Push,
  //! @brief Pop.
  kInstGroupX86Pop,
  //! @brief Rep/Repe/Repne LodsX/MovsX/StosX/CmpsX/ScasX.
  kInstGroupX86Rep,
  //! @brief Ret.
  kInstGroupX86Ret,
  //! @brief Rcl/Rcr/Rol/Ror/Sal/Sar/Shl/Shr.
  kInstGroupX86Rot,
  //! @brief Setcc.
  kInstGroupX86Set,
  //! @brief Shld/Rhrd.
  kInstGroupX86Shlrd,
  //! @brief Test.
  kInstGroupX86Test,
  //! @brief Xadd.
  kInstGroupX86Xadd,
  //! @brief Xchg.
  kInstGroupX86Xchg,

  //! @brief Fincstp/Finit/FldX/Fnclex/Fninit/Fnop/Fpatan/Fprem/Fprem1/Fptan/Frndint/Fscale/Fsin/Fsincos/Fsqrt/Ftst/Fucompp/Fxam/Fxtract/Fyl2x/Fyl2xp1.
  kInstGroupFpuOp,
  //! @brief Fadd/Fdiv/Fdivr/Fmul/Fsub/Fsubr.
  kInstGroupFpuArith,
  //! @brief Fcom/Fcomp.
  kInstGroupFpuCom,
  //! @brief Fld/Fst/Fstp.
  kInstGroupFpuFldFst,
  //! @brief Fiadd/Ficom/Ficomp/Fidiv/Fidivr/Fild/Fimul/Fist/Fistp/Fisttp/Fisub/Fisubr.
  kInstGroupFpuM,
  //! @brief Fcmov/Fcomi/Fcomip/Ffree/Fucom/Fucomi/Fucomip/Fucomp/Fxch.
  kInstGroupFpuR,
  //! @brief Faddp/Fdivp/Fdivrp/Fmulp/Fsubp/Fsubrp.
  kInstGroupFpuRDef,
  //! @brief Fnstsw/Fstsw.
  kInstGroupFpuStsw,

  //! @brief Mm/Xmm instruction.
  kInstGroupExtRm,
  //! @brief Mm/Xmm instruction (propagates 66H if the instruction uses XMM register).
  kInstGroupExtRm_P,
  //! @brief Mm/Xmm instruction (propagates REX.W if GPQ is used).
  kInstGroupExtRm_Q,
  //! @brief Mm/Xmm instruction having Rm/Ri encodings.
  kInstGroupExtRmRi,
  //! @brief Mm/Xmm instruction having Rm/Ri encodings (propagates 66H if the instruction uses XMM register).
  kInstGroupExtRmRi_P,
  //! @brief Mm/Xmm instruction having Rmi encoding.
  kInstGroupExtRmi,
  //! @brief Mm/Xmm instruction having Rmi encoding (propagates 66H if the instruction uses XMM register).
  kInstGroupExtRmi_P,
  //! @brief Crc32.
  kInstGroupExtCrc,
  //! @brief Pextrb/Pextrw/Pextrd/Pextrq/Extractps.
  kInstGroupExtExtract,
  //! @brief Lfence/Mfence/Sfence.
  kInstGroupExtFence,
  //! @brief Mov Mm/Xmm.
  //!
  //! 0x66 prefix must be set manually in opcodes.
  //!
  //! - Primary opcode is used for instructions in (X)Mm <- (X)Mm/Mem format,
  //! - Secondary opcode is used for instructions in (X)Mm/Mem <- (X)Mm format.
  kInstGroupExtMov,
  //! @brief Mov Mm/Xmm.
  kInstGroupExtMovNoRexW,
  //! @brief Movbe.
  kInstGroupExtMovBe,
  //! @brief Movd.
  kInstGroupExtMovD,
  //! @brief Movq.
  kInstGroupExtMovQ,
  //! @brief Prefetch.
  kInstGroupExtPrefetch,

  //! @brief 3dNow instruction.
  kInstGroup3dNow,

  //! @brief AVX instruction without operands.
  kInstGroupAvxOp,
  //! @brief AVX instruction encoded as 'M'.
  kInstGroupAvxM,
  //! @brief AVX instruction encoded as 'MR'.
  kInstGroupAvxMr,
  //! @brief AVX instruction encoded as 'MR' (propagates AVX.L if the instruction uses YMM register).
  kInstGroupAvxMr_P,
  //! @brief AVX instruction encoded as 'MRI'.
  kInstGroupAvxMri,
  //! @brief AVX instruction encoded as 'MRI' (propagates AVX.L if the instruction uses YMM register).
  kInstGroupAvxMri_P,
  //! @brief AVX instruction encoded as 'RM'.
  kInstGroupAvxRm,
  //! @brief AVX instruction encoded as 'RM' (propagates AVX.L if the instruction uses YMM register).
  kInstGroupAvxRm_P,
  //! @brief AVX instruction encoded as 'RMI'.
  kInstGroupAvxRmi,
  //! @brief AVX instruction encoded as 'RMI' (propagates AVX.L if the instruction uses YMM register)..
  kInstGroupAvxRmi_P,
  //! @brief AVX instruction encoded as 'RVM'.
  kInstGroupAvxRvm,
  //! @brief AVX instruction encoded as 'RVM' (propagates AVX.L if the instruction uses YMM register).
  kInstGroupAvxRvm_P,
  //! @brief AVX instruction encoded as 'RVMR'.
  kInstGroupAvxRvmr,
  //! @brief AVX instruction encoded as 'RVMR' (propagates AVX.L if the instruction uses YMM register).
  kInstGroupAvxRvmr_P,
  //! @brief AVX instruction encoded as 'RVMI'.
  kInstGroupAvxRvmi,
  //! @brief AVX instruction encoded as 'RVMI' (propagates AVX.L if the instruction uses YMM register).
  kInstGroupAvxRvmi_P,
  //! @brief AVX instruction encoded as 'RMV'.
  kInstGroupAvxRmv,
  //! @brief AVX instruction encoded as 'RMVI'.
  kInstGroupAvxRmvi,
  //! @brief AVX instruction encoded as 'RM' or 'MR'.
  kInstGroupAvxRmMr,
  //! @brief AVX instruction encoded as 'RM' or 'MR' (propagates AVX.L if the instruction uses YMM register).
  kInstGroupAvxRmMr_P,
  //! @brief AVX instruction encoded as 'RVM' or 'RMI'.
  kInstGroupAvxRvmRmi,
  //! @brief AVX instruction encoded as 'RVM' or 'RMI' (propagates AVX.L if the instruction uses YMM register).
  kInstGroupAvxRvmRmi_P,
  //! @brief AVX instruction encoded as 'RVM' or 'MR'.
  kInstGroupAvxRvmMr,
  //! @brief AVX instruction encoded as 'RVM' or 'MVR'.
  kInstGroupAvxRvmMvr,
  //! @brief AVX instruction encoded as 'RVM' or 'MVR' (propagates AVX.L if the instruction uses YMM register).
  kInstGroupAvxRvmMvr_P,
  //! @brief AVX instruction encoded as 'RVM' or 'VMI'.
  kInstGroupAvxRvmVmi,
  //! @brief AVX instruction encoded as 'RVM' or 'VMI' (propagates AVX.L if the instruction uses YMM register).
  kInstGroupAvxRvmVmi_P,
  //! @brief AVX instruction encoded as 'VM'.
  kInstGroupAvxVm,
  //! @brief AVX instruction encoded as 'VMI'.
  kInstGroupAvxVmi,
  //! @brief AVX instruction encoded as 'VMI' (propagates AVX.L if the instruction uses YMM register).
  kInstGroupAvxVmi_P,
  //! @brief Vmovss/Vmovsd.
  kInstGroupAvxMovSsSd,
  //! @brief AVX2 gather family instructions (VSIB).
  kInstGroupAvxGather,
  //! @brief AVX2 gather family instructions (VSIB), differs only in mem operand.
  kInstGroupAvxGatherEx
};

// ============================================================================
// [asmjit::x86x64::kInstOpCode]
// ============================================================================

//! @brief Instruction OpCode encoding used by asmjit 'InstInfo' table.
//!
//! The schema was inspired by AVX/AVX2 features.
ASMJIT_ENUM(kInstOpCode) {
  // 'MMMMM' field in AVX instruction.
  // 'OpCode' leading bytes in legacy encoding.
  kInstOpCode_MM_Shift = 16,
  kInstOpCode_MM_Mask  = 0x0FU << kInstOpCode_MM_Shift,
  kInstOpCode_MM_00    = 0x00U << kInstOpCode_MM_Shift,
  kInstOpCode_MM_0F    = 0x01U << kInstOpCode_MM_Shift,
  kInstOpCode_MM_0F38  = 0x02U << kInstOpCode_MM_Shift,
  kInstOpCode_MM_0F3A  = 0x03U << kInstOpCode_MM_Shift,
  kInstOpCode_MM_0F01  = 0x0FU << kInstOpCode_MM_Shift, // Ext/Not part of AVX.

  // 'PP' field in AVX instruction.
  // 'Mandatory Prefix' in legacy encoding.
  kInstOpCode_PP_Shift = 21,
  kInstOpCode_PP_Mask  = 0x07U << kInstOpCode_PP_Shift,
  kInstOpCode_PP_00    = 0x00U << kInstOpCode_PP_Shift,
  kInstOpCode_PP_66    = 0x01U << kInstOpCode_PP_Shift,
  kInstOpCode_PP_F3    = 0x02U << kInstOpCode_PP_Shift,
  kInstOpCode_PP_F2    = 0x03U << kInstOpCode_PP_Shift,
  kInstOpCode_PP_9B    = 0x07U << kInstOpCode_PP_Shift, //Ext/Not part of AVX.

  // 'L' field in AVX instruction.
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

//! @brief X86 instruction type flags.
ASMJIT_ENUM(kInstFlags) {
  //! @brief No flags.
  kInstFlagNone        = 0x0000,

  //! @brief Instruction is a control-flow instruction.
  //!
  //! Control flow instructions are jmp, jcc, call and ret.
  kInstFlagFlow        = 0x0001,

  //! @brief Instruction is a compare/test like instruction.
  kInstFlagTest        = 0x0002,

  //! @brief Instruction is a move like instruction.
  //!
  //! Move instructions typically overwrite the first operand by the second
  //! operand. The first operand can be the exact copy of the second operand
  //! or it can be any kind of conversion. Mov instructions are typically
  //! 'mov', 'movd', 'movq', 'movdq?', 'cmov??' like instructions, but we also
  //! consider 'lea' (Load Effective Address), multimedia  instructions like
  //! 'cvtdq2pd', shuffle instructions like 'pshufb' and SSE/SSE2 mathematic
  //! instructions like 'rcp??', 'round??' and 'rsqrt??'.
  kInstFlagMove        = 0x0004,

  //! @brief Instruction is an exchange like instruction.
  //!
  //! Exchange instruction typically overwrite first and second operand, we
  //! count 'xchg' and 'xadd' instructions right now.
  kInstFlagXchg        = 0x0008,

  //! @brief Instruction accesses Fp register(s).
  kInstFlagFp          = 0x0010,

  //! @brief Instruction can be prefixed by using the LOCK prefix.
  kInstFlagLock        = 0x0020,

  //! @brief Instruction is special, this is for @c Compiler.
  kInstFlagSpecial     = 0x0040,

  //! @brief Instruction always performs memory access.
  //!
  //! This flag is always combined with @c kInstFlagSpecial and signalizes
  //! that there is an implicit address which is accessed (usually EDI/RDI or
  //! ESI/EDI).
  kInstFlagSpecialMem  = 0x0080,

  //! @brief Instruction memory operand can refer to 16-bit address (used by FPU).
  kInstFlagMem2        = 0x0100,
  //! @brief Instruction memory operand can refer to 32-bit address (used by FPU).
  kInstFlagMem4        = 0x0200,
  //! @brief Instruction memory operand can refer to 64-bit address (used by FPU).
  kInstFlagMem8        = 0x0400,
  //! @brief Instruction memory operand can refer to 80-bit address (used by FPU).
  kInstFlagMem10       = 0x0800,

  //! @brief Combination of @c kInstFlagMem2 and @c kInstFlagMem4.
  kInstFlagMem2_4      = kInstFlagMem2   | kInstFlagMem4,
  //! @brief Combination of @c kInstFlagMem2 and @c kInstFlagMem4 and @c kInstFlagMem8.
  kInstFlagMem2_4_8    = kInstFlagMem2_4 | kInstFlagMem8,
  //! @brief Combination of @c kInstFlagMem4 and @c kInstFlagMem8.
  kInstFlagMem4_8      = kInstFlagMem4   | kInstFlagMem8,
  //! @brief Combination of @c kInstFlagMem4 and @c kInstFlagMem8 and @c kInstFlagMem10.
  kInstFlagMem4_8_10   = kInstFlagMem4_8 | kInstFlagMem10,

  //! @brief REX.W/VEX.W by default.
  kInstFlagW           = 0x8000
};

// ============================================================================
// [asmjit::x86x64::kInstOp]
// ============================================================================

//! @brief X86 instruction operand flags.
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

//! @brief X86 Condition codes.
ASMJIT_ENUM(kCond) {
  // Condition codes from processor manuals.
  kCondA               = 0x07, // CF==0 & ZF==0
  kCondAE              = 0x03, // CF==0
  kCondB               = 0x02, // CF==1
  kCondBE              = 0x06, // CF==1 | ZF==1
  kCondC               = 0x02, // CF==1
  kCondE               = 0x04, //         ZF==1
  kCondG               = 0x0F, //         ZF==0 & SF==OF
  kCondGE              = 0x0D, //                 SF==OF
  kCondL               = 0x0C, //                 SF!=OF
  kCondLE              = 0x0E, //         ZF==1 | SF!=OF
  kCondNA              = 0x06, // CF==1 | ZF==1
  kCondNAE             = 0x02, // CF==1
  kCondNB              = 0x03, // CF==0
  kCondNBE             = 0x07, // CF==0 & ZF==0
  kCondNC              = 0x03, // CF==0
  kCondNE              = 0x05, //         ZF==0
  kCondNG              = 0x0E, //         ZF==1 | SF!=OF
  kCondNGE             = 0x0C, //                 SF!=OF
  kCondNL              = 0x0D, //                 SF==OF
  kCondNLE             = 0x0F, //         ZF==0 & SF==OF
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

  //! @brief No condition code.
  kCondNone            = 0x12
};

// ============================================================================
// [asmjit::x86x64::kVarType]
// ============================================================================

//! @brief X86 variable type.
ASMJIT_ENUM(kVarType) {
  //! @brief Variable is Mm (MMX).
  kVarTypeMm = 13,

  //! @brief Variable is Xmm (SSE/SSE2).
  kVarTypeXmm = 14,
  //! @brief Variable is SSE scalar SP-FP number.
  kVarTypeXmmSs = 15,
  //! @brief Variable is SSE packed SP-FP number (4 floats).
  kVarTypeXmmPs = 16,
  //! @brief Variable is SSE2 scalar DP-FP number.
  kVarTypeXmmSd = 17,
  //! @brief Variable is SSE2 packed DP-FP number (2 doubles).
  kVarTypeXmmPd = 18,

  //! @brief Variable is Ymm (AVX).
  kVarTypeYmm = 19,
  //! @brief Variable is AVX packed SP-FP number (8 floats).
  kVarTypeYmmPs = 20,
  //! @brief Variable is AVX packed DP-FP number (4 doubles).
  kVarTypeYmmPd = 21,

  //! @brief Count of variable types.
  kVarTypeCount = 22,

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

//! @brief X86 variable description.
ASMJIT_ENUM(kVarDesc) {
  //! @brief Variable contains single-precision floating-point(s).
  kVarDescSp = 0x10,
  //! @brief Variable contains double-precision floating-point(s).
  kVarDescDp = 0x20,
  //! @brief Variable is packed (for example float4x, double2x, ...).
  kVarDescPacked = 0x40
};

// ============================================================================
// [asmjit::x86x64::InstInfo]
// ============================================================================

//! @brief X86 instruction information.
struct InstInfo {
  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! @brief Get instruction name string (null terminated string).
  ASMJIT_INLINE const char* getName() const { return _instName + static_cast<uint32_t>(_nameIndex); }
  //! @brief Get instruction name index (index to @ref _instName array).
  ASMJIT_INLINE uint32_t _getNameIndex() const { return _nameIndex; }

  //! @brief Get instruction group, see @ref kInstGroup.
  ASMJIT_INLINE uint32_t getGroup() const { return _group; }

  // --------------------------------------------------------------------------
  // [Flags]
  // --------------------------------------------------------------------------

  //! @brief Get instruction flags, see @ref kInstFlags.
  ASMJIT_INLINE uint32_t getFlags() const { return _flags; }

  //! @brief Get whether the instruction is a control-flow intruction.
  //!
  //! Control flow instruction is instruction that modifies instruction pointer,
  //! typically jmp, jcc, call, or ret.
  ASMJIT_INLINE bool isFlow() const { return (_flags & kInstFlagFlow) != 0; }

  //! @brief Get whether the instruction is a compare/test like intruction.
  ASMJIT_INLINE bool isTest() const { return (_flags & kInstFlagTest) != 0; }

  //! @brief Get whether the instruction is a typical Move instruction.
  //!
  //! Move instructions typically overwrite the first operand, so it's an useful
  //! hint for @ref Compiler. Applies also to multimedia instruction - MMX,
  //! SSE, SSE2 and AVX moves).
  ASMJIT_INLINE bool isMove() const { return (_flags & kInstFlagMove) != 0; }

  //! @brief Get whether the instruction is a typical Exchange instruction.
  //!
  //! Exchange instructios are 'xchg' and 'xadd'.
  ASMJIT_INLINE bool isXchg() const { return (_flags & kInstFlagXchg) != 0; }

  //! @brief Get whether the instruction accesses Fp register(s).
  ASMJIT_INLINE bool isFp() const { return (_flags & kInstFlagFp) != 0; }

  //! @brief Get whether the instruction can be prefixed by LOCK prefix.
  ASMJIT_INLINE bool isLockable() const { return (_flags & kInstFlagLock) != 0; }

  //! @brief Get whether the instruction is special type (this is used by
  //! @c Compiler to manage additional variables or functionality).
  ASMJIT_INLINE bool isSpecial() const { return (_flags & kInstFlagSpecial) != 0; }

  //! @brief Get whether the instruction is special type and it performs
  //! memory access.
  ASMJIT_INLINE bool isSpecialMem() const { return (_flags & kInstFlagSpecialMem) != 0; }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! @brief Instruction name index in _instName[] array.
  uint16_t _nameIndex;
  //! @brief Instruction flags.
  uint16_t _flags;
  //! @brief Instruction group, used also by @c Compiler.
  uint8_t _group;
  //! @brief Reserved for future use.
  uint8_t _reserved[3];
  //! @brief Operands' flags.
  uint16_t _opFlags[4];
  //! @brief Primary and secondary opcodes.
  uint32_t _opCode[2];
};

// ============================================================================
// [asmjit::x86x64::VarInfo]
// ============================================================================

//! @brief X86 variable information.
struct VarInfo {
  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! @brief Get register type, see @ref kRegType.
  ASMJIT_INLINE uint32_t getReg() const { return _reg; }
  //! @brief Get register size in bytes.
  ASMJIT_INLINE uint32_t getSize() const { return _size; }
  //! @brief Get variable class, see @ref kRegClass.
  ASMJIT_INLINE uint32_t getClass() const { return _class; }
  //! @brief Get variable description, see @ref kVarDesc.
  ASMJIT_INLINE uint32_t getDesc() const { return _desc; }
  //! @brief Get variable type name.
  ASMJIT_INLINE const char* getName() const { return _name; }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! @brief Register type, see @ref kRegType.
  uint8_t _reg;
  //! @brief Register size in bytes.
  uint8_t _size;
  //! @brief Register class, see @ref kRegClass.
  uint8_t _class;
  //! @brief Variable flags, see @ref kVarDesc.
  uint8_t _desc;
  //! @brief Variable type name.
  char _name[4];
};

// ============================================================================
// [asmjit::x86x64::RegCount]
// ============================================================================

//! @brief X86/X64 registers count (Gp, Fp, Mm, Xmm).
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

//! @brief X86/X64 registers mask (Gp, Fp, Mm, Xmm/Ymm/Zmm).
struct RegMask {
  // --------------------------------------------------------------------------
  // [Zero]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE void zero(uint32_t c)
  { _packed.u16[c] = 0; }

  // --------------------------------------------------------------------------
  // [Get]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE uint32_t get(uint32_t c) const
  { return _packed.u16[c]; }

  // --------------------------------------------------------------------------
  // [Set]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE void set(uint32_t c, uint32_t mask)
  { _packed.u16[c] = static_cast<uint16_t>(mask); }

  ASMJIT_INLINE void set(const RegMask& other)
  { _packed.setUInt64(other._packed); }

  // --------------------------------------------------------------------------
  // [Add]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE void add(uint32_t c, uint32_t mask)
  { _packed.u16[c] |= static_cast<uint16_t>(mask); }

  ASMJIT_INLINE void add(const RegMask& other)
  { _packed.or_(other._packed); }

  // --------------------------------------------------------------------------
  // [Del]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE void del(uint32_t c, uint32_t mask)
  { _packed.u16[c] &= ~static_cast<uint16_t>(mask); }

  ASMJIT_INLINE void del(const RegMask& other)
  { _packed.del(other._packed); }

  // --------------------------------------------------------------------------
  // [And]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE void and_(uint32_t c, uint32_t mask)
  { _packed.u16[c] &= static_cast<uint16_t>(mask); }

  ASMJIT_INLINE void and_(const RegMask& other)
  { _packed.and_(other._packed); }

  // --------------------------------------------------------------------------
  // [Xor]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE void xor_(uint32_t c, uint32_t mask)
  { _packed.u16[c] ^= static_cast<uint16_t>(mask); }

  ASMJIT_INLINE void xor_(const RegMask& other)
  { _packed.xor_(other._packed); }

  // --------------------------------------------------------------------------
  // [IsEmpty / Has]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE bool isEmpty() const
  { return _packed.isZero(); }

  ASMJIT_INLINE bool has(uint32_t c, uint32_t mask = 0xFFFFFFFF) const
  { return (static_cast<uint32_t>(_packed.u16[c]) & mask) != 0; }

  // --------------------------------------------------------------------------
  // [Reset]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE void reset()
  { _packed.reset(); }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  union {
    struct {
      //! @brief Gp registers mask.
      uint16_t _gp;
      //! @brief Fp registers mask.
      uint16_t _fp;
      //! @brief Mm registers mask.
      uint16_t _mm;
      //! @brief Xmm/Ymm registers mask.
      uint16_t _xy;
    };

    uint16_t _regs[4];

    //! @brief All masks as 64-bit integer.
    UInt64 _packed;
  };
};

// ============================================================================
// [asmjit::x86x64::X86Reg]
// ============================================================================

//! @brief X86/X64 register.
struct X86Reg : public BaseReg {
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! @brief Create a dummy X86 register.
  ASMJIT_INLINE X86Reg() : BaseReg() {}
  //! @brief Create a custom X86 register.
  ASMJIT_INLINE X86Reg(uint32_t type, uint32_t index, uint32_t size) : BaseReg(type, index, size) {}
  //! @brief Create a reference to @a other X86 register.
  ASMJIT_INLINE X86Reg(const X86Reg& other) : BaseReg(other) {}
  //! @brief Create non-initialized X86 register.
  explicit ASMJIT_INLINE X86Reg(const _DontInitialize&) : BaseReg(DontInitialize) {}

  // --------------------------------------------------------------------------
  // [X86Reg Specific]
  // --------------------------------------------------------------------------

  ASMJIT_REG_OP(X86Reg)

  //! @brief Get whether the register is Gp register.
  ASMJIT_INLINE bool isGp() const { return _vreg.type <= kRegTypeGpq; }
  //! @brief Get whether the register is Gp byte (8-bit) register.
  ASMJIT_INLINE bool isGpb() const { return _vreg.type <= kRegTypeGpbHi; }
  //! @brief Get whether the register is Gp lo-byte (8-bit) register.
  ASMJIT_INLINE bool isGpbLo() const { return _vreg.type == kRegTypeGpbLo; }
  //! @brief Get whether the register is Gp hi-byte (8-bit) register.
  ASMJIT_INLINE bool isGpbHi() const { return _vreg.type == kRegTypeGpbHi; }
  //! @brief Get whether the register is Gp word (16-bit) register.
  ASMJIT_INLINE bool isGpw() const { return _vreg.type == kRegTypeGpw; }
  //! @brief Get whether the register is Gp dword (32-bit) register.
  ASMJIT_INLINE bool isGpd() const { return _vreg.type == kRegTypeGpd; }
  //! @brief Get whether the register is Gp qword (64-bit) register.
  ASMJIT_INLINE bool isGpq() const { return _vreg.type == kRegTypeGpq; }

  //! @brief Get whether the register is Fp register.
  ASMJIT_INLINE bool isFp() const { return _vreg.type == kRegTypeFp; }
  //! @brief Get whether the register is Mm (64-bit) register.
  ASMJIT_INLINE bool isMm() const { return _vreg.type == kRegTypeMm; }
  //! @brief Get whether the register is Xmm (128-bit) register.
  ASMJIT_INLINE bool isXmm() const { return _vreg.type == kRegTypeXmm; }
  //! @brief Get whether the register is Ymm (256-bit) register.
  ASMJIT_INLINE bool isYmm() const { return _vreg.type == kRegTypeYmm; }

  //! @brief Get whether the register is a segment.
  ASMJIT_INLINE bool isSeg() const { return _vreg.type == kRegTypeSeg; }
};

// ============================================================================
// [asmjit::x86x64::GpReg]
// ============================================================================

//! @brief X86/X64 Gpb/Gpw/Gpd/Gpq register.
struct GpReg : public X86Reg {
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! @brief Create a dummy Gp register.
  ASMJIT_INLINE GpReg() : X86Reg() {}
  //! @brief Create a reference to @a other Gp register.
  ASMJIT_INLINE GpReg(const GpReg& other) : X86Reg(other) {}
  //! @brief Create a custom Gp register.
  ASMJIT_INLINE GpReg(uint32_t type, uint32_t index, uint32_t size) : X86Reg(type, index, size) {}
  //! @brief Create non-initialized Gp register.
  explicit ASMJIT_INLINE GpReg(const _DontInitialize&) : X86Reg(DontInitialize) {}

  // --------------------------------------------------------------------------
  // [GpReg Specific]
  // --------------------------------------------------------------------------

  ASMJIT_REG_OP(GpReg)
};

// ============================================================================
// [asmjit::x86x64::FpReg]
// ============================================================================

//! @brief X86/X64 80-bit Fp register.
struct FpReg : public X86Reg {
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! @brief Create a dummy Fp register.
  ASMJIT_INLINE FpReg() : X86Reg() {}
  //! @brief Create a reference to @a other FPU register.
  ASMJIT_INLINE FpReg(const FpReg& other) : X86Reg(other) {}
  //! @brief Create a custom Fp register.
  ASMJIT_INLINE FpReg(uint32_t type, uint32_t index, uint32_t size) : X86Reg(type, index, size) {}
  //! @brief Create non-initialized Fp register.
  explicit ASMJIT_INLINE FpReg(const _DontInitialize&) : X86Reg(DontInitialize) {}

  // --------------------------------------------------------------------------
  // [FpReg Specific]
  // --------------------------------------------------------------------------

  ASMJIT_REG_OP(FpReg)
};

// ============================================================================
// [asmjit::x86x64::MmReg]
// ============================================================================

//! @brief X86/X64 64-bit Mm register.
struct MmReg : public X86Reg {
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! @brief Create a dummy Mm register.
  ASMJIT_INLINE MmReg() : X86Reg() {}
  //! @brief Create a reference to @a other Mm register.
  ASMJIT_INLINE MmReg(const MmReg& other) : X86Reg(other) {}
  //! @brief Create a custom Mm register.
  ASMJIT_INLINE MmReg(uint32_t type, uint32_t index, uint32_t size) : X86Reg(type, index, size) {}
  //! @brief Create non-initialized Mm register.
  explicit ASMJIT_INLINE MmReg(const _DontInitialize&) : X86Reg(DontInitialize) {}

  // --------------------------------------------------------------------------
  // [MmReg Specific]
  // --------------------------------------------------------------------------

  ASMJIT_REG_OP(MmReg)
};

// ============================================================================
// [asmjit::x86x64::XmmReg]
// ============================================================================

//! @brief X86/X64 128-bit Xmm register.
struct XmmReg : public X86Reg {
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! @brief Create a dummy Xmm register.
  ASMJIT_INLINE XmmReg() : X86Reg() {}
  //! @brief Create a reference to @a other Xmm register.
  ASMJIT_INLINE XmmReg(const XmmReg& other) : X86Reg(other) {}
  //! @brief Create a custom Xmm register.
  ASMJIT_INLINE XmmReg(uint32_t type, uint32_t index, uint32_t size) : X86Reg(type, index, size) {}
  //! @brief Create non-initialized Xmm register.
  explicit ASMJIT_INLINE XmmReg(const _DontInitialize&) : X86Reg(DontInitialize) {}

  // --------------------------------------------------------------------------
  // [XmmReg Specific]
  // --------------------------------------------------------------------------

  ASMJIT_REG_OP(XmmReg)
};

// ============================================================================
// [asmjit::x86x64::YmmReg]
// ============================================================================

//! @brief X86/X64 256-bit Ymm register.
struct YmmReg : public X86Reg {
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! @brief Create a dummy Ymm register.
  ASMJIT_INLINE YmmReg() : X86Reg() {}
  //! @brief Create a reference to @a other Xmm register.
  ASMJIT_INLINE YmmReg(const YmmReg& other) : X86Reg(other) {}
  //! @brief Create a custom Ymm register.
  ASMJIT_INLINE YmmReg(uint32_t type, uint32_t index, uint32_t size) : X86Reg(type, index, size) {}
  //! @brief Create non-initialized Ymm register.
  explicit ASMJIT_INLINE YmmReg(const _DontInitialize&) : X86Reg(DontInitialize) {}

  // --------------------------------------------------------------------------
  // [YmmReg Specific]
  // --------------------------------------------------------------------------

  ASMJIT_REG_OP(YmmReg)
};

// ============================================================================
// [asmjit::x86x64::SegReg]
// ============================================================================

//! @brief X86/X64 segment register.
struct SegReg : public X86Reg {
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! @brief Create a dummy segment register.
  ASMJIT_INLINE SegReg() : X86Reg() {}
  //! @brief Create a reference to @a other segment register.
  ASMJIT_INLINE SegReg(const SegReg& other) : X86Reg(other) {}
  //! @brief Create a custom segment register.
  ASMJIT_INLINE SegReg(uint32_t type, uint32_t index, uint32_t size) : X86Reg(type, index, size) {}
  //! @brief Create non-initialized segment register.
  explicit ASMJIT_INLINE SegReg(const _DontInitialize&) : X86Reg(DontInitialize) {}

  // --------------------------------------------------------------------------
  // [SegReg Specific]
  // --------------------------------------------------------------------------

  ASMJIT_REG_OP(SegReg)
};

// ============================================================================
// [asmjit::x86x64::Mem]
// ============================================================================

#define _OP_ID(_Op_) reinterpret_cast<const Operand&>(_Op_).getId()

//! @brief X86 memory operand.
struct Mem : public BaseMem {
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE Mem() : BaseMem(DontInitialize) {
    reset();
  }

  ASMJIT_INLINE Mem(const Label& label, int32_t disp, uint32_t size = 0) : BaseMem(DontInitialize) {
    _init_packed_op_sz_b0_b1_id(kOperandTypeMem, size, kMemTypeLabel, 0, label._base.id);
    _init_packed_d2_d3(kInvalidValue, disp);
  }

  ASMJIT_INLINE Mem(const Label& label, const GpReg& index, uint32_t shift, int32_t disp, uint32_t size = 0) : BaseMem(DontInitialize) {
    ASMJIT_ASSERT(shift <= 3);

    _init_packed_op_sz_b0_b1_id(kOperandTypeMem, size, kMemTypeLabel,
      (kMemVSibGpz << kMemVSibIndex)
        + (shift << kMemShiftIndex),
      label.getId());
    _vmem.index = index.getRegIndex();
    _vmem.displacement = disp;
  }

  ASMJIT_INLINE Mem(const Label& label, const GpVar& index, uint32_t shift, int32_t disp, uint32_t size = 0) : BaseMem(DontInitialize) {
    ASMJIT_ASSERT(shift <= 3);

    _init_packed_op_sz_b0_b1_id(kOperandTypeMem, size, kMemTypeLabel,
      (kMemVSibGpz << kMemVSibIndex)
        + (shift << kMemShiftIndex),
      label.getId());
    _vmem.index = _OP_ID(index);
    _vmem.displacement = disp;
  }

  ASMJIT_INLINE Mem(const GpReg& base, int32_t disp, uint32_t size = 0) : BaseMem(DontInitialize) {
    _init_packed_op_sz_b0_b1_id(kOperandTypeMem, size, kMemTypeBaseIndex,
      _getGpdFlags(base)
        + (kMemVSibGpz << kMemVSibIndex),
      base.getRegIndex());
    _init_packed_d2_d3(kInvalidValue, disp);
  }

  ASMJIT_INLINE Mem(const GpReg& base, const GpReg& index, uint32_t shift, int32_t disp, uint32_t size = 0) : BaseMem(DontInitialize) {
    ASMJIT_ASSERT(shift <= 3);

    _init_packed_op_sz_b0_b1_id(kOperandTypeMem, size, kMemTypeBaseIndex,
      _getGpdFlags(base) + (shift << kMemShiftIndex),
      base.getRegIndex());
    _vmem.index = index.getRegIndex();
    _vmem.displacement = disp;
  }

  ASMJIT_INLINE Mem(const GpReg& base, const XmmReg& index, uint32_t shift, int32_t disp, uint32_t size = 0) : BaseMem(DontInitialize) {
    ASMJIT_ASSERT(shift <= 3);

    _init_packed_op_sz_b0_b1_id(kOperandTypeMem, size, kMemTypeBaseIndex,
      _getGpdFlags(base)
        + (kMemVSibXmm << kMemVSibIndex)
        + (shift << kMemShiftIndex),
      base.getRegIndex());
    _vmem.index = index.getRegIndex();
    _vmem.displacement = disp;
  }

  ASMJIT_INLINE Mem(const GpReg& base, const YmmReg& index, uint32_t shift, int32_t disp, uint32_t size = 0) : BaseMem(DontInitialize) {
    ASMJIT_ASSERT(shift <= 3);

    _init_packed_op_sz_b0_b1_id(kOperandTypeMem, size, kMemTypeBaseIndex,
      _getGpdFlags(base)
        + (kMemVSibYmm << kMemVSibIndex)
        + (shift << kMemShiftIndex),
      base.getRegIndex());
    _vmem.index = index.getRegIndex();
    _vmem.displacement = disp;
  }

  ASMJIT_INLINE Mem(const GpVar& base, int32_t disp, uint32_t size = 0) : BaseMem(DontInitialize) {
    _init_packed_op_sz_b0_b1_id(kOperandTypeMem, size, kMemTypeBaseIndex,
      _getGpdFlags(reinterpret_cast<const BaseVar&>(base))
        + (kMemVSibGpz << kMemVSibIndex),
      _OP_ID(base));
    _init_packed_d2_d3(kInvalidValue, disp);
  }


  ASMJIT_INLINE Mem(const GpVar& base, const GpVar& index, uint32_t shift, int32_t disp, uint32_t size = 0) : BaseMem(DontInitialize) {
    ASMJIT_ASSERT(shift <= 3);

    _init_packed_op_sz_b0_b1_id(kOperandTypeMem, size, kMemTypeBaseIndex,
      _getGpdFlags(reinterpret_cast<const BaseVar&>(base))
        + (shift << kMemShiftIndex),
      _OP_ID(base));
    _vmem.index = _OP_ID(index);
    _vmem.displacement = disp;
  }

  ASMJIT_INLINE Mem(const GpVar& base, const XmmVar& index, uint32_t shift, int32_t disp, uint32_t size = 0) : BaseMem(DontInitialize) {
    ASMJIT_ASSERT(shift <= 3);

    _init_packed_op_sz_b0_b1_id(kOperandTypeMem, size, kMemTypeBaseIndex,
      _getGpdFlags(reinterpret_cast<const BaseVar&>(base))
        + (kMemVSibXmm << kMemVSibIndex)
        + (shift << kMemShiftIndex),
      _OP_ID(base));
    _vmem.index = _OP_ID(index);
    _vmem.displacement = disp;
  }

  ASMJIT_INLINE Mem(const GpVar& base, const YmmVar& index, uint32_t shift, int32_t disp, uint32_t size = 0) : BaseMem(DontInitialize) {
    ASMJIT_ASSERT(shift <= 3);

    _init_packed_op_sz_b0_b1_id(kOperandTypeMem, size, kMemTypeBaseIndex,
      _getGpdFlags(reinterpret_cast<const BaseVar&>(base))
        + (kMemVSibYmm << kMemVSibIndex)
        + (shift << kMemShiftIndex),
      _OP_ID(base));
    _vmem.index = _OP_ID(index);
    _vmem.displacement = disp;
  }

  ASMJIT_INLINE Mem(const _Initialize&, uint32_t memType, const X86Var& base, int32_t disp, uint32_t size) : BaseMem(DontInitialize) {
    _init_packed_op_sz_b0_b1_id(kOperandTypeMem, size, memType, 0, _OP_ID(base));
    _vmem.index = kInvalidValue;
    _vmem.displacement = disp;
  }

  ASMJIT_INLINE Mem(const _Initialize&, uint32_t memType, const X86Var& base, const GpVar& index, uint32_t shift, int32_t disp, uint32_t size) : BaseMem(DontInitialize) {
    ASMJIT_ASSERT(shift <= 3);

    _init_packed_op_sz_b0_b1_id(kOperandTypeMem, size, memType, shift << kMemShiftIndex, _OP_ID(base));
    _vmem.index = _OP_ID(index);
    _vmem.displacement = disp;
  }

  ASMJIT_INLINE Mem(const Mem& other) : BaseMem(other) {}
  explicit ASMJIT_INLINE Mem(const _DontInitialize&) : BaseMem(DontInitialize) {}

  // --------------------------------------------------------------------------
  // [Mem Specific]
  // --------------------------------------------------------------------------

  //! @brief Clone Mem operand.
  ASMJIT_INLINE Mem clone() const {
    return Mem(*this);
  }

  //! @brief Reset Mem operand.
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

  //! @brief Get whether the memory operand has segment override prefix.
  ASMJIT_INLINE bool hasSegment() const {
    return (_vmem.flags & kMemSegMask) != (kSegDefault << kMemSegIndex);
  }

  //! @brief Get memory operand segment, see @c kSeg.
  ASMJIT_INLINE uint32_t getSegment() const {
    return (static_cast<uint32_t>(_vmem.flags) >> kMemSegIndex) & kMemSegBits;
  }

  //! @brief Set memory operand segment, see @c kSeg.
  ASMJIT_INLINE Mem& setSegment(uint32_t segIndex) {
    _vmem.flags = static_cast<uint8_t>(
      (static_cast<uint32_t>(_vmem.flags) & kMemSegMask) + (segIndex << kMemSegIndex));
    return *this;
  }

  //! @brief Set memory operand segment, see @c kSeg.
  ASMJIT_INLINE Mem& setSegment(const SegReg& seg) {
    return setSegment(seg.getRegIndex());
  }

  // --------------------------------------------------------------------------
  // [Gpd]
  // --------------------------------------------------------------------------

  //! @brief Get whether the memory operand has 32-bit GP base.
  ASMJIT_INLINE bool hasGpdBase() const {
    return (_packed[0].u32[0] & IntUtil::pack32_4x8(0x00, 0x00, 0x00, kMemGpdMask)) != 0;
  }

  //! @brief Set whether the memory operand has 32-bit GP base.
  ASMJIT_INLINE Mem& setGpdBase() {
    _packed[0].u32[0] |= IntUtil::pack32_4x8(0x00, 0x00, 0x00, kMemGpdMask);
    return *this;
  }

  //! @brief Set whether the memory operand has 32-bit GP base to @a b.
  ASMJIT_INLINE Mem& setGpdBase(uint32_t b) {
    _packed[0].u32[0] &=~IntUtil::pack32_4x8(0x00, 0x00, 0x00, kMemGpdMask);
    _packed[0].u32[0] |= IntUtil::pack32_4x8(0x00, 0x00, 0x00, b << kMemGpdIndex);
    return *this;
  }

  // --------------------------------------------------------------------------
  // [VSib]
  // --------------------------------------------------------------------------

  //! @brief Get SIB type.
  ASMJIT_INLINE uint32_t getVSib() const {
    return (static_cast<uint32_t>(_vmem.flags) >> kMemVSibIndex) & kMemVSibBits;
  }

  //! @brief Set SIB type.
  ASMJIT_INLINE Mem& _setVSib(uint32_t vsib) {
    _packed[0].u32[0] &=~IntUtil::pack32_4x8(0x00, 0x00, 0x00, kMemVSibMask);
    _packed[0].u32[0] |= IntUtil::pack32_4x8(0x00, 0x00, 0x00, vsib << kMemVSibIndex);
    return *this;
  }

  // --------------------------------------------------------------------------
  // [Size]
  // --------------------------------------------------------------------------

  //! @brief Set memory operand size.
  ASMJIT_INLINE Mem& setSize(uint32_t size) {
    _vmem.size = static_cast<uint8_t>(size);
    return *this;
  }

  // --------------------------------------------------------------------------
  // [Base]
  // --------------------------------------------------------------------------

  //! @brief Get whether the memory operand has base register.
  ASMJIT_INLINE bool hasBase() const
  { return _vmem.base != kInvalidValue; }

  //! @brief Get memory operand base register code, variable id, or @ref kInvalidValue.
  ASMJIT_INLINE uint32_t getBase() const
  { return _vmem.base; }

  //! @brief Set memory operand base register code, variable id, or @ref kInvalidValue.
  ASMJIT_INLINE Mem& setBase(uint32_t base) {
    _vmem.base = base;
    return *this;
  }

  // --------------------------------------------------------------------------
  // [Index]
  // --------------------------------------------------------------------------

  //! @brief Get whether the memory operand has index.
  ASMJIT_INLINE bool hasIndex() const {
    return _vmem.index != kInvalidValue;
  }

  //! @brief Get memory operand index register code, variable id, or @ref kInvalidValue.
  ASMJIT_INLINE uint32_t getIndex() const {
    return _vmem.index;
  }

  //! @brief Set memory operand index register code, variable id, or @ref kInvalidValue.
  ASMJIT_INLINE Mem& setIndex(uint32_t index) {
    _vmem.index = index;
    return *this;
  }

  //! @brief Set memory index.
  ASMJIT_INLINE Mem& setIndex(const GpReg& index) {
    _vmem.index = index.getRegIndex();
    return _setVSib(kMemVSibGpz);
  }

  //! @brief Set memory index.
  ASMJIT_INLINE Mem& setIndex(const GpReg& index, uint32_t shift) {
    _vmem.index = index.getRegIndex();
    return _setVSib(kMemVSibGpz).setShift(shift);
  }

  //! @brief Set memory index.
  ASMJIT_INLINE Mem& setIndex(const GpVar& index) {
    _vmem.index = reinterpret_cast<const BaseVar&>(index).getId();
    return _setVSib(kMemVSibGpz);
  }

  //! @brief Set memory index.
  ASMJIT_INLINE Mem& setIndex(const GpVar& index, uint32_t shift) {
    _vmem.index = reinterpret_cast<const BaseVar&>(index).getId();
    return _setVSib(kMemVSibGpz).setShift(shift);
  }

  //! @brief Set memory index.
  ASMJIT_INLINE Mem& setIndex(const XmmReg& index) {
    _vmem.index = index.getRegIndex();
    return _setVSib(kMemVSibXmm);
  }

  //! @brief Set memory index.
  ASMJIT_INLINE Mem& setIndex(const XmmReg& index, uint32_t shift) {
    _vmem.index = index.getRegIndex();
    return _setVSib(kMemVSibXmm).setShift(shift);
  }

  //! @brief Set memory index.
  ASMJIT_INLINE Mem& setIndex(const XmmVar& index) {
    _vmem.index = reinterpret_cast<const BaseVar&>(index).getId();
    return _setVSib(kMemVSibXmm);
  }

  //! @brief Set memory index.
  ASMJIT_INLINE Mem& setIndex(const XmmVar& index, uint32_t shift) {
    _vmem.index = reinterpret_cast<const BaseVar&>(index).getId();
    return _setVSib(kMemVSibXmm).setShift(shift);
  }

  //! @brief Set memory index.
  ASMJIT_INLINE Mem& setIndex(const YmmReg& index) {
    _vmem.index = index.getRegIndex();
    return _setVSib(kMemVSibYmm);
  }

  //! @brief Set memory index.
  ASMJIT_INLINE Mem& setIndex(const YmmReg& index, uint32_t shift) {
    _vmem.index = index.getRegIndex();
    return _setVSib(kMemVSibYmm).setShift(shift);
  }

  //! @brief Set memory index.
  ASMJIT_INLINE Mem& setIndex(const YmmVar& index) {
    _vmem.index = reinterpret_cast<const BaseVar&>(index).getId();
    return _setVSib(kMemVSibYmm);
  }

  //! @brief Set memory index.
  ASMJIT_INLINE Mem& setIndex(const YmmVar& index, uint32_t shift) {
    _vmem.index = reinterpret_cast<const BaseVar&>(index).getId();
    return _setVSib(kMemVSibYmm).setShift(shift);
  }

  //! @brief Reset memory index.
  ASMJIT_INLINE Mem& resetIndex() {
    _vmem.index = kInvalidValue;
    return _setVSib(kMemVSibGpz);
  }

  // --------------------------------------------------------------------------
  // [Shift]
  // --------------------------------------------------------------------------

  //! @brief Get whether the memory operand has shift used.
  ASMJIT_INLINE bool hasShift() const {
    return (_vmem.flags & kMemShiftMask) != 0;
  }

  //! @brief Get memory operand index scale (0, 1, 2 or 3).
  ASMJIT_INLINE uint32_t getShift() const {
    return _vmem.flags >> kMemShiftIndex;
  }

  //! @brief Set memory operand index scale (0, 1, 2 or 3).
  ASMJIT_INLINE Mem& setShift(uint32_t shift) {
    _packed[0].u32[0] &=~IntUtil::pack32_4x8(0x00, 0x00, 0x00, kMemShiftMask);
    _packed[0].u32[0] |= IntUtil::pack32_4x8(0x00, 0x00, 0x00, shift << kMemShiftIndex);
    return *this;
  }

  // --------------------------------------------------------------------------
  // [Displacement]
  // --------------------------------------------------------------------------

  //! @brief Get memory operand relative displacement.
  ASMJIT_INLINE int32_t getDisplacement() const {
    return _vmem.displacement;
  }

  //! @brief Set memory operand relative displacement.
  ASMJIT_INLINE Mem& setDisplacement(int32_t disp) {
    _vmem.displacement = disp;
    return *this;
  }

  //! @brief Reset memory operand relative displacement.
  ASMJIT_INLINE Mem& resetDisplacement(int32_t disp) {
    _vmem.displacement = 0;
    return *this;
  }

  //! @brief Adjust memory operand relative displacement by @a displacement.
  ASMJIT_INLINE Mem& adjust(int32_t disp) {
    _vmem.displacement += disp;
    return *this;
  }

  //! @brief Get new memory operand adjusted by @a displacement.
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

//! @brief Base class for all variables.
struct X86Var : public BaseVar {
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE X86Var() : BaseVar(DontInitialize) {
    reset();
  }

  ASMJIT_INLINE X86Var(const X86Var& other) : BaseVar(other) {}

  explicit ASMJIT_INLINE X86Var(const _DontInitialize&) : BaseVar(DontInitialize) {}

  // --------------------------------------------------------------------------
  // [X86Var Specific]
  // --------------------------------------------------------------------------

  //! @brief Clone X86Var operand.
  ASMJIT_INLINE X86Var clone() const {
    return X86Var(*this);
  }

  //! @brief Reset X86Var operand.
  ASMJIT_INLINE void reset() {
    _init_packed_op_sz_b0_b1_id(kOperandTypeVar, 0, kInvalidReg, kInvalidReg, kInvalidValue);
    _init_packed_d2_d3(kInvalidValue, kInvalidValue);
  }

  // --------------------------------------------------------------------------
  // [Type]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE uint32_t getRegType() const { return _vreg.type; }
  ASMJIT_INLINE uint32_t getVarType() const { return _vreg.vType; }

  //! @brief Get whether the variable is Gpb register.
  ASMJIT_INLINE bool isGp() const { return _vreg.type <= kRegTypeGpq; }
  //! @brief Get whether the variable is Gpb register.
  ASMJIT_INLINE bool isGpb() const { return _vreg.type <= kRegTypeGpbHi; }
  //! @brief Get whether the variable is Gpb-lo register.
  ASMJIT_INLINE bool isGpbLo() const { return _vreg.type == kRegTypeGpbLo; }
  //! @brief Get whether the variable is Gpb-hi register.
  ASMJIT_INLINE bool isGpbHi() const { return _vreg.type == kRegTypeGpbHi; }
  //! @brief Get whether the variable is Gpw register.
  ASMJIT_INLINE bool isGpw() const { return _vreg.type == kRegTypeGpw; }
  //! @brief Get whether the variable is Gpd register.
  ASMJIT_INLINE bool isGpd() const { return _vreg.type == kRegTypeGpd; }
  //! @brief Get whether the variable is Gpq register.
  ASMJIT_INLINE bool isGpq() const { return _vreg.type == kRegTypeGpq; }

  //! @brief Get whether the variable is Fp register.
  ASMJIT_INLINE bool isFp() const { return _vreg.type == kRegTypeFp; }
  //! @brief Get whether the variable is Mm type.
  ASMJIT_INLINE bool isMm() const { return _vreg.type == kRegTypeMm; }
  //! @brief Get whether the variable is Xmm type.
  ASMJIT_INLINE bool isXmm() const { return _vreg.type == kRegTypeXmm; }
  //! @brief Get whether the variable is Ymm type.
  ASMJIT_INLINE bool isYmm() const { return _vreg.type == kRegTypeYmm; }

  // --------------------------------------------------------------------------
  // [Memory Cast]
  // --------------------------------------------------------------------------

  //! @brief Cast this variable to memory operand.
  //!
  //! @note Size of operand depends on native variable type, you can use other
  //! variants if you want specific one.
  ASMJIT_INLINE Mem m(int32_t disp = 0) const
  { return Mem(Initialize, kMemTypeStackIndex, *this, disp, getSize()); }

  //! @overload
  ASMJIT_INLINE Mem m(const GpVar& index, uint32_t shift = 0, int32_t disp = 0) const
  { return Mem(Initialize, kMemTypeStackIndex, *this, index, shift, disp, getSize()); }

  //! @brief Cast this variable to 8-bit memory operand.
  ASMJIT_INLINE Mem m8(int32_t disp = 0) const
  { return Mem(Initialize, kMemTypeStackIndex, *this, disp, 1); }

  //! @overload
  ASMJIT_INLINE Mem m8(const GpVar& index, uint32_t shift = 0, int32_t disp = 0) const
  { return Mem(Initialize, kMemTypeStackIndex, *this, index, shift, disp, 1); }

  //! @brief Cast this variable to 16-bit memory operand.
  ASMJIT_INLINE Mem m16(int32_t disp = 0) const
  { return Mem(Initialize, kMemTypeStackIndex, *this, disp, 2); }

  //! @overload
  ASMJIT_INLINE Mem m16(const GpVar& index, uint32_t shift = 0, int32_t disp = 0) const
  { return Mem(Initialize, kMemTypeStackIndex, *this, index, shift, disp, 2); }

  //! @brief Cast this variable to 32-bit memory operand.
  ASMJIT_INLINE Mem m32(int32_t disp = 0) const
  { return Mem(Initialize, kMemTypeStackIndex, *this, disp, 4); }

  //! @overload
  ASMJIT_INLINE Mem m32(const GpVar& index, uint32_t shift = 0, int32_t disp = 0) const
  { return Mem(Initialize, kMemTypeStackIndex, *this, index, shift, disp, 4); }

  //! @brief Cast this variable to 64-bit memory operand.
  ASMJIT_INLINE Mem m64(int32_t disp = 0) const
  { return Mem(Initialize, kMemTypeStackIndex, *this, disp, 8); }

  //! @overload
  ASMJIT_INLINE Mem m64(const GpVar& index, uint32_t shift = 0, int32_t disp = 0) const
  { return Mem(Initialize, kMemTypeStackIndex, *this, index, shift, disp, 8); }

  //! @brief Cast this variable to 80-bit memory operand (long double).
  ASMJIT_INLINE Mem m80(int32_t disp = 0) const
  { return Mem(Initialize, kMemTypeStackIndex, *this, disp, 10); }

  //! @overload
  ASMJIT_INLINE Mem m80(const GpVar& index, uint32_t shift = 0, int32_t disp = 0) const
  { return Mem(Initialize, kMemTypeStackIndex, *this, index, shift, disp, 10); }

  //! @brief Cast this variable to 128-bit memory operand.
  ASMJIT_INLINE Mem m128(int32_t disp = 0) const
  { return Mem(Initialize, kMemTypeStackIndex, *this, disp, 16); }

  //! @overload
  ASMJIT_INLINE Mem m128(const GpVar& index, uint32_t shift = 0, int32_t disp = 0) const
  { return Mem(Initialize, kMemTypeStackIndex, *this, index, shift, disp, 16); }

  //! @brief Cast this variable to 256-bit memory operand.
  ASMJIT_INLINE Mem m256(int32_t disp = 0) const
  { return Mem(Initialize, kMemTypeStackIndex, *this, disp, 32); }

  //! @overload
  ASMJIT_INLINE Mem m256(const GpVar& index, uint32_t shift = 0, int32_t disp = 0) const
  { return Mem(Initialize, kMemTypeStackIndex, *this, index, shift, disp, 32); }

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
  ASMJIT_INLINE X86Var(const X86Var& other, uint32_t reg, uint32_t size) : BaseVar(DontInitialize)
  {
    _init_packed_op_sz_w0_id(kOperandTypeVar, size, (reg << 8) + other._vreg.index, other._base.id);
    _vreg.vType = other._vreg.vType;
  }
};

// ============================================================================
// [asmjit::x86x64::GpVar]
// ============================================================================

//! @brief Gp variable.
struct GpVar : public X86Var {
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! @brief Create new uninitialized @c GpVar instance.
  ASMJIT_INLINE GpVar() : X86Var() {}

  //! @brief Create new initialized @c GpVar instance.
  ASMJIT_INLINE GpVar(BaseCompiler& c, uint32_t type = kVarTypeIntPtr, const char* name = NULL) : X86Var(DontInitialize) {
    c._newVar(this, type, name);
  }

  //! @brief Create new @c GpVar instance using @a other.
  //!
  //! Note this will not create a different variable, use @c Compiler::newGpVar()
  //! if you want to do so. This is only copy-constructor that allows to store
  //! the same variable in different places.
  ASMJIT_INLINE GpVar(const GpVar& other) : X86Var(other) {}

  //! @brief Create new uninitialized @c GpVar instance (internal).
  explicit ASMJIT_INLINE GpVar(const _DontInitialize&) : X86Var(DontInitialize) {}

  // --------------------------------------------------------------------------
  // [GpVar Specific]
  // --------------------------------------------------------------------------

  //! @brief Clone GpVar operand.
  ASMJIT_INLINE GpVar clone() const {
    return GpVar(*this);
  }

  //! @brief Reset GpVar operand.
  ASMJIT_INLINE void reset() {
    X86Var::reset();
  }

  // --------------------------------------------------------------------------
  // [GpVar Cast]
  // --------------------------------------------------------------------------

  //! @brief Cast this variable to 8-bit (LO) part of variable
  ASMJIT_INLINE GpVar r8() const { return GpVar(*this, kRegTypeGpbLo, 1); }
  //! @brief Cast this variable to 8-bit (LO) part of variable
  ASMJIT_INLINE GpVar r8Lo() const { return GpVar(*this, kRegTypeGpbLo, 1); }
  //! @brief Cast this variable to 8-bit (HI) part of variable
  ASMJIT_INLINE GpVar r8Hi() const { return GpVar(*this, kRegTypeGpbHi, 1); }

  //! @brief Cast this variable to 16-bit part of variable
  ASMJIT_INLINE GpVar r16() const { return GpVar(*this, kRegTypeGpw, 2); }
  //! @brief Cast this variable to 32-bit part of variable
  ASMJIT_INLINE GpVar r32() const { return GpVar(*this, kRegTypeGpd, 4); }
  //! @brief Cast this variable to 64-bit part of variable
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

//! @brief Fpu variable.
struct FpVar : public X86Var {
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! @brief Create new uninitialized @c FpVar instance.
  ASMJIT_INLINE FpVar() : X86Var() {}
  ASMJIT_INLINE FpVar(const FpVar& other) : X86Var(other) {}

  //! @brief Create new uninitialized @c FpVar instance (internal).
  explicit ASMJIT_INLINE FpVar(const _DontInitialize&) : X86Var(DontInitialize) {}

  // --------------------------------------------------------------------------
  // [FpVar Specific]
  // --------------------------------------------------------------------------

  //! @brief Clone FpVar operand.
  ASMJIT_INLINE FpVar clone() const {
    return FpVar(*this);
  }

  //! @brief Reset FpVar operand.
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

//! @brief Mm variable.
struct MmVar : public X86Var {
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! @brief Create new uninitialized @c MmVar instance.
  ASMJIT_INLINE MmVar() : X86Var() {}
  //! @brief Create new initialized @c MmVar instance.
  ASMJIT_INLINE MmVar(BaseCompiler& c, uint32_t type = kVarTypeMm, const char* name = NULL) : X86Var(DontInitialize) {
    c._newVar(this, type, name);
  }

  //! @brief Create new @c MmVar instance using @a other.
  //!
  //! Note this will not create a different variable, use @c Compiler::newMmVar()
  //! if you want to do so. This is only copy-constructor that allows to store
  //! the same variable in different places.
  ASMJIT_INLINE MmVar(const MmVar& other) : X86Var(other) {}

  //! @brief Create new uninitialized @c MmVar instance (internal).
  explicit ASMJIT_INLINE MmVar(const _DontInitialize&) : X86Var(DontInitialize) {}

  // --------------------------------------------------------------------------
  // [MmVar Specific]
  // --------------------------------------------------------------------------

  //! @brief Clone MmVar operand.
  ASMJIT_INLINE MmVar clone() const {
    return MmVar(*this);
  }

  //! @brief Reset MmVar operand.
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

//! @brief Xmm variable.
struct XmmVar : public X86Var {
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! @brief Create new uninitialized @c XmmVar instance.
  ASMJIT_INLINE XmmVar() : X86Var() {}
  //! @brief Create new initialized @c XmmVar instance.
  ASMJIT_INLINE XmmVar(BaseCompiler& c, uint32_t type = kVarTypeXmm, const char* name = NULL) : X86Var(DontInitialize) {
    c._newVar(this, type, name);
  }

  ASMJIT_INLINE XmmVar(const XmmVar& other) : X86Var(other) {}

  //! @brief Create new uninitialized @c XmmVar instance (internal).
  explicit ASMJIT_INLINE XmmVar(const _DontInitialize&) : X86Var(DontInitialize) {}

  // --------------------------------------------------------------------------
  // [XmmVar Specific]
  // --------------------------------------------------------------------------

  //! @brief Clone XmmVar operand.
  ASMJIT_INLINE XmmVar clone() const {
    return XmmVar(*this);
  }

  //! @brief Reset XmmVar operand.
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

//! @brief Ymm variable.
struct YmmVar : public X86Var {
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! @brief Create new uninitialized @c YmmVar instance.
  ASMJIT_INLINE YmmVar() : X86Var() {}
  //! @brief Create new initialized @c YmmVar instance.
  ASMJIT_INLINE YmmVar(BaseCompiler& c, uint32_t type = kVarTypeYmm, const char* name = NULL) : X86Var(DontInitialize) {
    c._newVar(this, type, name);
  }

  ASMJIT_INLINE YmmVar(const YmmVar& other) : X86Var(other) {}

  //! @brief Create new uninitialized @c YmmVar instance (internal).
  explicit ASMJIT_INLINE YmmVar(const _DontInitialize&) : X86Var(DontInitialize) {}

  // --------------------------------------------------------------------------
  // [YmmVar Specific]
  // --------------------------------------------------------------------------

  //! @brief Clone YmmVar operand.
  ASMJIT_INLINE YmmVar clone() const {
    return YmmVar(*this);
  }

  //! @brief Reset YmmVar operand.
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

//! @brief Create Shuffle Constant for MMX/SSE shuffle instrutions.
//!
//! @param z First component position, number at interval [0, 3] inclusive.
//! @param x Second component position, number at interval [0, 3] inclusive.
//! @param y Third component position, number at interval [0, 3] inclusive.
//! @param w Fourth component position, number at interval [0, 3] inclusive.
//!
//! Shuffle constants can be used to make immediate value for these intrinsics:
//! - @ref Assembler::pshufw()
//! - @ref Assembler::pshufd()
//! - @ref Assembler::pshufhw()
//! - @ref Assembler::pshuflw()
//! - @ref Assembler::shufps()
static ASMJIT_INLINE uint8_t mm_shuffle(uint8_t z, uint8_t y, uint8_t x, uint8_t w)
{ return (z << 6) | (y << 4) | (x << 2) | w; }

// ============================================================================
// [asmjit::x86x64::Cond - Reverse / Negate]
// ============================================================================

//! @brief Corresponds to transposing the operands of a comparison.
static ASMJIT_INLINE uint32_t reverseCond(uint32_t cond) {
  ASMJIT_ASSERT(cond < ASMJIT_ARRAY_SIZE(_reverseCond));
  return _reverseCond[cond];
}

//! @brief Get the equivalent of negated condition code.
static ASMJIT_INLINE uint32_t negateCond(uint32_t cond) {
  ASMJIT_ASSERT(cond < ASMJIT_ARRAY_SIZE(_reverseCond));
  return static_cast<kCond>(cond ^ static_cast<uint32_t>(cond < kCondNone));
}

// ============================================================================
// [asmjit::x86x64::Cond - ToJcc / ToMovcc / ToSetcc]
// ============================================================================

//! @brief Translate condition code @a cc to cmovcc instruction code.
//! @sa @c kInstCode, @c _kInstCmovcc.
static ASMJIT_INLINE uint32_t condToCmovcc(uint32_t cond) {
  ASMJIT_ASSERT(static_cast<uint32_t>(cond) < ASMJIT_ARRAY_SIZE(_condToCmovcc));
  return _condToCmovcc[cond];
}

//! @brief Translate condition code @a cc to jcc instruction code.
//! @sa @c kInstCode, @c _kInstJcc.
static ASMJIT_INLINE uint32_t condToJcc(uint32_t cond) {
  ASMJIT_ASSERT(static_cast<uint32_t>(cond) < ASMJIT_ARRAY_SIZE(_condToJcc));
  return _condToJcc[cond];
}

//! @brief Translate condition code @a cc to setcc instruction code.
//! @sa @c kInstCode, @c _kInstSetcc.
static ASMJIT_INLINE uint32_t condToSetcc(uint32_t cond) {
  ASMJIT_ASSERT(static_cast<uint32_t>(cond) < ASMJIT_ARRAY_SIZE(_condToSetcc));
  return _condToSetcc[cond];
}

// ============================================================================
// [asmjit::x86x64::Registers]
// ============================================================================

//! @brief No register, can be used only within @c Mem operand.
ASMJIT_VAR const GpReg noGpReg;

//! @brief Gpb-lo register.
ASMJIT_VAR const GpReg al;
//! @brief Gpb-lo register.
ASMJIT_VAR const GpReg cl;
//! @brief Gpb-lo register.
ASMJIT_VAR const GpReg dl;
//! @brief Gpb-lo register.
ASMJIT_VAR const GpReg bl;
//! @brief Gpb-hi register.
ASMJIT_VAR const GpReg ah;
//! @brief Gpb-hi register.
ASMJIT_VAR const GpReg ch;
//! @brief Gpb-hi register.
ASMJIT_VAR const GpReg dh;
//! @brief Gpb-hi register.
ASMJIT_VAR const GpReg bh;

//! @brief Gpw register.
ASMJIT_VAR const GpReg ax;
//! @brief Gpw register.
ASMJIT_VAR const GpReg cx;
//! @brief Gpw register.
ASMJIT_VAR const GpReg dx;
//! @brief Gpw register.
ASMJIT_VAR const GpReg bx;
//! @brief Gpw register.
ASMJIT_VAR const GpReg sp;
//! @brief Gpw register.
ASMJIT_VAR const GpReg bp;
//! @brief Gpw register.
ASMJIT_VAR const GpReg si;
//! @brief Gpw register.
ASMJIT_VAR const GpReg di;

//! @brief Gpd register.
ASMJIT_VAR const GpReg eax;
//! @brief Gpd register.
ASMJIT_VAR const GpReg ecx;
//! @brief Gpd register.
ASMJIT_VAR const GpReg edx;
//! @brief Gpd register.
ASMJIT_VAR const GpReg ebx;
//! @brief Gpd register.
ASMJIT_VAR const GpReg esp;
//! @brief Gpd register.
ASMJIT_VAR const GpReg ebp;
//! @brief Gpd register.
ASMJIT_VAR const GpReg esi;
//! @brief Gpd register.
ASMJIT_VAR const GpReg edi;

//! @brief Fp register.
ASMJIT_VAR const FpReg fp0;
//! @brief Fp register.
ASMJIT_VAR const FpReg fp1;
//! @brief Fp register.
ASMJIT_VAR const FpReg fp2;
//! @brief Fp register.
ASMJIT_VAR const FpReg fp3;
//! @brief Fp register.
ASMJIT_VAR const FpReg fp4;
//! @brief Fp register.
ASMJIT_VAR const FpReg fp5;
//! @brief Fp register.
ASMJIT_VAR const FpReg fp6;
//! @brief Fp register.
ASMJIT_VAR const FpReg fp7;

//! @brief Mm register.
ASMJIT_VAR const MmReg mm0;
//! @brief Mm register.
ASMJIT_VAR const MmReg mm1;
//! @brief Mm register.
ASMJIT_VAR const MmReg mm2;
//! @brief Mm register.
ASMJIT_VAR const MmReg mm3;
//! @brief Mm register.
ASMJIT_VAR const MmReg mm4;
//! @brief Mm register.
ASMJIT_VAR const MmReg mm5;
//! @brief Mm register.
ASMJIT_VAR const MmReg mm6;
//! @brief Mm register.
ASMJIT_VAR const MmReg mm7;

//! @brief Xmm register.
ASMJIT_VAR const XmmReg xmm0;
//! @brief Xmm register.
ASMJIT_VAR const XmmReg xmm1;
//! @brief Xmm register.
ASMJIT_VAR const XmmReg xmm2;
//! @brief Xmm register.
ASMJIT_VAR const XmmReg xmm3;
//! @brief Xmm register.
ASMJIT_VAR const XmmReg xmm4;
//! @brief Xmm register.
ASMJIT_VAR const XmmReg xmm5;
//! @brief Xmm register.
ASMJIT_VAR const XmmReg xmm6;
//! @brief Xmm register.
ASMJIT_VAR const XmmReg xmm7;

//! @brief Ymm register.
ASMJIT_VAR const YmmReg ymm0;
//! @brief Ymm register.
ASMJIT_VAR const YmmReg ymm1;
//! @brief Ymm register.
ASMJIT_VAR const YmmReg ymm2;
//! @brief Ymm register.
ASMJIT_VAR const YmmReg ymm3;
//! @brief Ymm register.
ASMJIT_VAR const YmmReg ymm4;
//! @brief Ymm register.
ASMJIT_VAR const YmmReg ymm5;
//! @brief Ymm register.
ASMJIT_VAR const YmmReg ymm6;
//! @brief Ymm register.
ASMJIT_VAR const YmmReg ymm7;

//! @brief Cs segment register.
ASMJIT_VAR const SegReg cs;
//! @brief Ss segment register.
ASMJIT_VAR const SegReg ss;
//! @brief Ds segment register.
ASMJIT_VAR const SegReg ds;
//! @brief Es segment register.
ASMJIT_VAR const SegReg es;
//! @brief Fs segment register.
ASMJIT_VAR const SegReg fs;
//! @brief Gs segment register.
ASMJIT_VAR const SegReg gs;

//! @brief Get Gpb-lo register.
static ASMJIT_INLINE GpReg gpb_lo(uint32_t index) { return GpReg(kRegTypeGpbLo, index, 1); }
//! @brief Get Gpb-hi register.
static ASMJIT_INLINE GpReg gpb_hi(uint32_t index) { return GpReg(kRegTypeGpbHi, index, 1); }
//! @brief Get Gpw register.
static ASMJIT_INLINE GpReg gpw(uint32_t index) { return GpReg(kRegTypeGpw, index, 2); }
//! @brief Get Gpd register.
static ASMJIT_INLINE GpReg gpd(uint32_t index) { return GpReg(kRegTypeGpd, index, 4); }
//! @brief Get Fp register.
static ASMJIT_INLINE FpReg fp(uint32_t index) { return FpReg(kRegTypeFp, index, 10); }
//! @brief Get Mm register.
static ASMJIT_INLINE MmReg mm(uint32_t index) { return MmReg(kRegTypeMm, index, 8); }
//! @brief Get Xmm register.
static ASMJIT_INLINE XmmReg xmm(uint32_t index) { return XmmReg(kRegTypeXmm, index, 16); }
//! @brief Get Xmm register.
static ASMJIT_INLINE YmmReg ymm(uint32_t index) { return YmmReg(kRegTypeYmm, index, 32); }

// ============================================================================
// [asmjit::x86x64::Mem - ptr[base + disp]]
// ============================================================================

//! @brief Create a custom pointer operand.
static ASMJIT_INLINE Mem ptr(const GpReg& base, int32_t disp = 0, uint32_t size = 0) { return Mem(base, disp, size); }
//! @brief Create a byte pointer operand.
static ASMJIT_INLINE Mem byte_ptr(const GpReg& base, int32_t disp = 0) { return ptr(base, disp, kSizeByte); }
//! @brief Create a word pointer operand.
static ASMJIT_INLINE Mem word_ptr(const GpReg& base, int32_t disp = 0) { return ptr(base, disp, kSizeWord); }
//! @brief Create a dword pointer operand.
static ASMJIT_INLINE Mem dword_ptr(const GpReg& base, int32_t disp = 0) { return ptr(base, disp, kSizeDWord); }
//! @brief Create a qword pointer operand.
static ASMJIT_INLINE Mem qword_ptr(const GpReg& base, int32_t disp = 0) { return ptr(base, disp, kSizeQWord); }
//! @brief Create a tword pointer operand.
static ASMJIT_INLINE Mem tword_ptr(const GpReg& base, int32_t disp = 0) { return ptr(base, disp, kSizeTWord); }
//! @brief Create a oword pointer operand.
static ASMJIT_INLINE Mem oword_ptr(const GpReg& base, int32_t disp = 0) { return ptr(base, disp, kSizeOWord); }
//! @brief Create a yword pointer operand.
static ASMJIT_INLINE Mem yword_ptr(const GpReg& base, int32_t disp = 0) { return ptr(base, disp, kSizeYWord); }

//! @brief Create a custom pointer operand.
static ASMJIT_INLINE Mem ptr(const GpVar& base, int32_t disp = 0, uint32_t size = 0) { return Mem(base, disp, size); }
//! @brief Create a byte pointer operand.
static ASMJIT_INLINE Mem byte_ptr(const GpVar& base, int32_t disp = 0) { return ptr(base, disp, kSizeByte); }
//! @brief Create a word pointer operand.
static ASMJIT_INLINE Mem word_ptr(const GpVar& base, int32_t disp = 0) { return ptr(base, disp, kSizeWord); }
//! @brief Create a dword pointer operand.
static ASMJIT_INLINE Mem dword_ptr(const GpVar& base, int32_t disp = 0) { return ptr(base, disp, kSizeDWord); }
//! @brief Create a qword pointer operand.
static ASMJIT_INLINE Mem qword_ptr(const GpVar& base, int32_t disp = 0) { return ptr(base, disp, kSizeQWord); }
//! @brief Create a tword pointer operand.
static ASMJIT_INLINE Mem tword_ptr(const GpVar& base, int32_t disp = 0) { return ptr(base, disp, kSizeTWord); }
//! @brief Create a oword pointer operand.
static ASMJIT_INLINE Mem oword_ptr(const GpVar& base, int32_t disp = 0) { return ptr(base, disp, kSizeOWord); }
//! @brief Create a yword pointer operand.
static ASMJIT_INLINE Mem yword_ptr(const GpVar& base, int32_t disp = 0) { return ptr(base, disp, kSizeYWord); }

// ============================================================================
// [asmjit::x86x64::Mem - ptr[base + (index << shift) + disp]]
// ============================================================================

//! @brief Create a custom pointer operand.
static ASMJIT_INLINE Mem ptr(const GpReg& base, const GpReg& index, uint32_t shift = 0, int32_t disp = 0, uint32_t size = 0) { return Mem(base, index, shift, disp, size); }
//! @brief Create a byte pointer operand.
static ASMJIT_INLINE Mem byte_ptr(const GpReg& base, const GpReg& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, kSizeByte); }
//! @brief Create a word pointer operand.
static ASMJIT_INLINE Mem word_ptr(const GpReg& base, const GpReg& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, kSizeWord); }
//! @brief Create a dword pointer operand.
static ASMJIT_INLINE Mem dword_ptr(const GpReg& base, const GpReg& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, kSizeDWord); }
//! @brief Create a qword pointer operand.
static ASMJIT_INLINE Mem qword_ptr(const GpReg& base, const GpReg& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, kSizeQWord); }
//! @brief Create a tword pointer operand.
static ASMJIT_INLINE Mem tword_ptr(const GpReg& base, const GpReg& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, kSizeTWord); }
//! @brief Create a oword pointer operand.
static ASMJIT_INLINE Mem oword_ptr(const GpReg& base, const GpReg& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, kSizeOWord); }
//! @brief Create a yword pointer operand.
static ASMJIT_INLINE Mem yword_ptr(const GpReg& base, const GpReg& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, kSizeYWord); }

//! @brief Create a custom pointer operand.
static ASMJIT_INLINE Mem ptr(const GpVar& base, const GpVar& index, uint32_t shift = 0, int32_t disp = 0, uint32_t size = 0) { return Mem(base, index, shift, disp, size); }
//! @brief Create a byte pointer operand.
static ASMJIT_INLINE Mem byte_ptr(const GpVar& base, const GpVar& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, kSizeByte); }
//! @brief Create a word pointer operand.
static ASMJIT_INLINE Mem word_ptr(const GpVar& base, const GpVar& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, kSizeWord); }
//! @brief Create a dword pointer operand.
static ASMJIT_INLINE Mem dword_ptr(const GpVar& base, const GpVar& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, kSizeDWord); }
//! @brief Create a qword pointer operand.
static ASMJIT_INLINE Mem qword_ptr(const GpVar& base, const GpVar& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, kSizeQWord); }
//! @brief Create a tword pointer operand.
static ASMJIT_INLINE Mem tword_ptr(const GpVar& base, const GpVar& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, kSizeTWord); }
//! @brief Create a oword pointer operand.
static ASMJIT_INLINE Mem oword_ptr(const GpVar& base, const GpVar& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, kSizeOWord); }
//! @brief Create a yword pointer operand.
static ASMJIT_INLINE Mem yword_ptr(const GpVar& base, const GpVar& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, kSizeYWord); }

// ============================================================================
// [asmjit::x86x64::Mem - ptr[base + (vex << shift) + disp]]
// ============================================================================

//! @brief Create a custom pointer operand.
static ASMJIT_INLINE Mem ptr(const GpReg& base, const XmmReg& index, uint32_t shift = 0, int32_t disp = 0, uint32_t size = 0) { return Mem(base, index, shift, disp, size); }
//! @brief Create a byte pointer operand.
static ASMJIT_INLINE Mem byte_ptr(const GpReg& base, const XmmReg& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, kSizeByte); }
//! @brief Create a word pointer operand.
static ASMJIT_INLINE Mem word_ptr(const GpReg& base, const XmmReg& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, kSizeWord); }
//! @brief Create a dword pointer operand.
static ASMJIT_INLINE Mem dword_ptr(const GpReg& base, const XmmReg& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, kSizeDWord); }
//! @brief Create a qword pointer operand.
static ASMJIT_INLINE Mem qword_ptr(const GpReg& base, const XmmReg& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, kSizeQWord); }
//! @brief Create a tword pointer operand.
static ASMJIT_INLINE Mem tword_ptr(const GpReg& base, const XmmReg& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, kSizeTWord); }
//! @brief Create a oword pointer operand.
static ASMJIT_INLINE Mem oword_ptr(const GpReg& base, const XmmReg& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, kSizeOWord); }
//! @brief Create a yword pointer operand.
static ASMJIT_INLINE Mem yword_ptr(const GpReg& base, const XmmReg& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, kSizeYWord); }

//! @brief Create a custom pointer operand.
static ASMJIT_INLINE Mem ptr(const GpReg& base, const YmmReg& index, uint32_t shift = 0, int32_t disp = 0, uint32_t size = 0) { return Mem(base, index, shift, disp, size); }
//! @brief Create a byte pointer operand.
static ASMJIT_INLINE Mem byte_ptr(const GpReg& base, const YmmReg& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, kSizeByte); }
//! @brief Create a word pointer operand.
static ASMJIT_INLINE Mem word_ptr(const GpReg& base, const YmmReg& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, kSizeWord); }
//! @brief Create a dword pointer operand.
static ASMJIT_INLINE Mem dword_ptr(const GpReg& base, const YmmReg& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, kSizeDWord); }
//! @brief Create a qword pointer operand.
static ASMJIT_INLINE Mem qword_ptr(const GpReg& base, const YmmReg& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, kSizeQWord); }
//! @brief Create a tword pointer operand.
static ASMJIT_INLINE Mem tword_ptr(const GpReg& base, const YmmReg& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, kSizeTWord); }
//! @brief Create a oword pointer operand.
static ASMJIT_INLINE Mem oword_ptr(const GpReg& base, const YmmReg& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, kSizeOWord); }
//! @brief Create a yword pointer operand.
static ASMJIT_INLINE Mem yword_ptr(const GpReg& base, const YmmReg& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, kSizeYWord); }

//! @brief Create a custom pointer operand.
static ASMJIT_INLINE Mem ptr(const GpVar& base, const XmmVar& index, uint32_t shift = 0, int32_t disp = 0, uint32_t size = 0) { return Mem(base, index, shift, disp, size); }
//! @brief Create a byte pointer operand.
static ASMJIT_INLINE Mem byte_ptr(const GpVar& base, const XmmVar& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, kSizeByte); }
//! @brief Create a word pointer operand.
static ASMJIT_INLINE Mem word_ptr(const GpVar& base, const XmmVar& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, kSizeWord); }
//! @brief Create a dword pointer operand.
static ASMJIT_INLINE Mem dword_ptr(const GpVar& base, const XmmVar& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, kSizeDWord); }
//! @brief Create a qword pointer operand.
static ASMJIT_INLINE Mem qword_ptr(const GpVar& base, const XmmVar& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, kSizeQWord); }
//! @brief Create a tword pointer operand.
static ASMJIT_INLINE Mem tword_ptr(const GpVar& base, const XmmVar& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, kSizeTWord); }
//! @brief Create a oword pointer operand.
static ASMJIT_INLINE Mem oword_ptr(const GpVar& base, const XmmVar& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, kSizeOWord); }
//! @brief Create a yword pointer operand.
static ASMJIT_INLINE Mem yword_ptr(const GpVar& base, const XmmVar& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, kSizeYWord); }

//! @brief Create a custom pointer operand.
static ASMJIT_INLINE Mem ptr(const GpVar& base, const YmmVar& index, uint32_t shift = 0, int32_t disp = 0, uint32_t size = 0) { return Mem(base, index, shift, disp, size); }
//! @brief Create a byte pointer operand.
static ASMJIT_INLINE Mem byte_ptr(const GpVar& base, const YmmVar& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, kSizeByte); }
//! @brief Create a word pointer operand.
static ASMJIT_INLINE Mem word_ptr(const GpVar& base, const YmmVar& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, kSizeWord); }
//! @brief Create a dword pointer operand.
static ASMJIT_INLINE Mem dword_ptr(const GpVar& base, const YmmVar& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, kSizeDWord); }
//! @brief Create a qword pointer operand.
static ASMJIT_INLINE Mem qword_ptr(const GpVar& base, const YmmVar& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, kSizeQWord); }
//! @brief Create a tword pointer operand.
static ASMJIT_INLINE Mem tword_ptr(const GpVar& base, const YmmVar& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, kSizeTWord); }
//! @brief Create a oword pointer operand.
static ASMJIT_INLINE Mem oword_ptr(const GpVar& base, const YmmVar& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, kSizeOWord); }
//! @brief Create a yword pointer operand.
static ASMJIT_INLINE Mem yword_ptr(const GpVar& base, const YmmVar& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, kSizeYWord); }

// ============================================================================
// [asmjit::x86x64::Mem - [label + disp]]
// ============================================================================

//! @brief Create a custom pointer operand.
static ASMJIT_INLINE Mem ptr(const Label& label, int32_t disp = 0, uint32_t size = 0) { return Mem(label, disp, size); }
//! @brief Create a byte pointer operand.
static ASMJIT_INLINE Mem byte_ptr(const Label& label, int32_t disp = 0) { return ptr(label, disp, kSizeByte); }
//! @brief Create a word pointer operand.
static ASMJIT_INLINE Mem word_ptr(const Label& label, int32_t disp = 0) { return ptr(label, disp, kSizeWord); }
//! @brief Create a dword pointer operand.
static ASMJIT_INLINE Mem dword_ptr(const Label& label, int32_t disp = 0) { return ptr(label, disp, kSizeDWord); }
//! @brief Create a qword pointer operand.
static ASMJIT_INLINE Mem qword_ptr(const Label& label, int32_t disp = 0) { return ptr(label, disp, kSizeQWord); }
//! @brief Create a tword pointer operand.
static ASMJIT_INLINE Mem tword_ptr(const Label& label, int32_t disp = 0) { return ptr(label, disp, kSizeTWord); }
//! @brief Create a oword pointer operand.
static ASMJIT_INLINE Mem oword_ptr(const Label& label, int32_t disp = 0) { return ptr(label, disp, kSizeOWord); }
//! @brief Create a yword pointer operand.
static ASMJIT_INLINE Mem yword_ptr(const Label& label, int32_t disp = 0) { return ptr(label, disp, kSizeYWord); }

// ============================================================================
// [asmjit::x86x64::Mem - [label + index << shift + disp]]
// ============================================================================

//! @brief Create a custom pointer operand.
static ASMJIT_INLINE Mem ptr(const Label& label, const GpReg& index, uint32_t shift, int32_t disp = 0, uint32_t size = 0) { return Mem(label, index, shift, disp, size); }
//! @brief Create a byte pointer operand.
static ASMJIT_INLINE Mem byte_ptr(const Label& label, const GpReg& index, uint32_t shift, int32_t disp = 0) { return ptr(label, index, shift, disp, kSizeByte); }
//! @brief Create a word pointer operand.
static ASMJIT_INLINE Mem word_ptr(const Label& label, const GpReg& index, uint32_t shift, int32_t disp = 0) { return ptr(label, index, shift, disp, kSizeWord); }
//! @brief Create a dword pointer operand.
static ASMJIT_INLINE Mem dword_ptr(const Label& label, const GpReg& index, uint32_t shift, int32_t disp = 0) { return ptr(label, index, shift, disp, kSizeDWord); }
//! @brief Create a qword pointer operand.
static ASMJIT_INLINE Mem qword_ptr(const Label& label, const GpReg& index, uint32_t shift, int32_t disp = 0) { return ptr(label, index, shift, disp, kSizeQWord); }
//! @brief Create a tword pointer operand.
static ASMJIT_INLINE Mem tword_ptr(const Label& label, const GpReg& index, uint32_t shift, int32_t disp = 0) { return ptr(label, index, shift, disp, kSizeTWord); }
//! @brief Create a oword pointer operand.
static ASMJIT_INLINE Mem oword_ptr(const Label& label, const GpReg& index, uint32_t shift, int32_t disp = 0) { return ptr(label, index, shift, disp, kSizeOWord); }
//! @brief Create a yword pointer operand.
static ASMJIT_INLINE Mem yword_ptr(const Label& label, const GpReg& index, uint32_t shift, int32_t disp = 0) { return ptr(label, index, shift, disp, kSizeYWord); }

//! @brief Create a custom pointer operand.
static ASMJIT_INLINE Mem ptr(const Label& label, const GpVar& index, uint32_t shift, int32_t disp = 0, uint32_t size = 0) { return Mem(label, index, shift, disp, size); }
//! @brief Create a byte pointer operand.
static ASMJIT_INLINE Mem byte_ptr(const Label& label, const GpVar& index, uint32_t shift, int32_t disp = 0) { return ptr(label, index, shift, disp, kSizeByte); }
//! @brief Create a word pointer operand.
static ASMJIT_INLINE Mem word_ptr(const Label& label, const GpVar& index, uint32_t shift, int32_t disp = 0) { return ptr(label, index, shift, disp, kSizeWord); }
//! @brief Create a dword pointer operand.
static ASMJIT_INLINE Mem dword_ptr(const Label& label, const GpVar& index, uint32_t shift, int32_t disp = 0) { return ptr(label, index, shift, disp, kSizeDWord); }
//! @brief Create a qword pointer operand.
static ASMJIT_INLINE Mem qword_ptr(const Label& label, const GpVar& index, uint32_t shift, int32_t disp = 0) { return ptr(label, index, shift, disp, kSizeQWord); }
//! @brief Create a tword pointer operand.
static ASMJIT_INLINE Mem tword_ptr(const Label& label, const GpVar& index, uint32_t shift, int32_t disp = 0) { return ptr(label, index, shift, disp, kSizeTWord); }
//! @brief Create a oword pointer operand.
static ASMJIT_INLINE Mem oword_ptr(const Label& label, const GpVar& index, uint32_t shift, int32_t disp = 0) { return ptr(label, index, shift, disp, kSizeOWord); }
//! @brief Create a yword pointer operand.
static ASMJIT_INLINE Mem yword_ptr(const Label& label, const GpVar& index, uint32_t shift, int32_t disp = 0) { return ptr(label, index, shift, disp, kSizeYWord); }

// ============================================================================
// [asmjit::x86x64::Mem - [ptr + disp]
// ============================================================================

//! @brief Create a custom pointer operand.
ASMJIT_API Mem ptr_abs(Ptr pAbs, int32_t disp = 0, uint32_t size = 0);

//! @brief Create a byte pointer operand.
static ASMJIT_INLINE Mem byte_ptr_abs(Ptr pAbs, int32_t disp = 0) { return ptr_abs(pAbs, disp, kSizeByte); }
//! @brief Create a word pointer operand.
static ASMJIT_INLINE Mem word_ptr_abs(Ptr pAbs, int32_t disp = 0) { return ptr_abs(pAbs, disp, kSizeWord); }
//! @brief Create a dword pointer operand.
static ASMJIT_INLINE Mem dword_ptr_abs(Ptr pAbs, int32_t disp = 0) { return ptr_abs(pAbs, disp, kSizeDWord); }
//! @brief Create a qword pointer operand.
static ASMJIT_INLINE Mem qword_ptr_abs(Ptr pAbs, int32_t disp = 0) { return ptr_abs(pAbs, disp, kSizeQWord); }
//! @brief Create a tword pointer operand (used for 80-bit floating points).
static ASMJIT_INLINE Mem tword_ptr_abs(Ptr pAbs, int32_t disp = 0) { return ptr_abs(pAbs, disp, kSizeTWord); }
//! @brief Create a oword pointer operand.
static ASMJIT_INLINE Mem oword_ptr_abs(Ptr pAbs, int32_t disp = 0) { return ptr_abs(pAbs, disp, kSizeOWord); }
//! @brief Create a yword pointer operand.
static ASMJIT_INLINE Mem yword_ptr_abs(Ptr pAbs, int32_t disp = 0) { return ptr_abs(pAbs, disp, kSizeYWord); }

// ============================================================================
// [asmjit::x86x64::Mem - [pAbs + index << shift + disp]
// ============================================================================

//! @brief Create a custom pointer operand.
ASMJIT_API Mem ptr_abs(Ptr pAbs, const X86Reg& index, uint32_t shift = 0, int32_t disp = 0, uint32_t size = 0);
//! @brief Create a custom pointer operand.
ASMJIT_API Mem ptr_abs(Ptr pAbs, const X86Var& index, uint32_t shift = 0, int32_t disp = 0, uint32_t size = 0);

//! @brief Create a Byte pointer operand.
static ASMJIT_INLINE Mem byte_ptr_abs(Ptr pAbs, const GpReg& index, uint32_t shift = 0, int32_t disp = 0) { return ptr_abs(pAbs, index, shift, disp, kSizeByte); }
//! @brief Create a word pointer operand.
static ASMJIT_INLINE Mem word_ptr_abs(Ptr pAbs, const GpReg& index, uint32_t shift = 0, int32_t disp = 0) { return ptr_abs(pAbs, index, shift, disp, kSizeWord); }
//! @brief Create a dword pointer operand.
static ASMJIT_INLINE Mem dword_ptr_abs(Ptr pAbs, const GpReg& index, uint32_t shift = 0, int32_t disp = 0) { return ptr_abs(pAbs, index, shift, disp, kSizeDWord); }
//! @brief Create a qword pointer operand.
static ASMJIT_INLINE Mem qword_ptr_abs(Ptr pAbs, const GpReg& index, uint32_t shift = 0, int32_t disp = 0) { return ptr_abs(pAbs, index, shift, disp, kSizeQWord); }
//! @brief Create a tword pointer operand.
static ASMJIT_INLINE Mem tword_ptr_abs(Ptr pAbs, const GpReg& index, uint32_t shift = 0, int32_t disp = 0) { return ptr_abs(pAbs, index, shift, disp, kSizeTWord); }
//! @brief Create a oword pointer operand.
static ASMJIT_INLINE Mem oword_ptr_abs(Ptr pAbs, const GpReg& index, uint32_t shift = 0, int32_t disp = 0) { return ptr_abs(pAbs, index, shift, disp, kSizeOWord); }
//! @brief Create a yword pointer operand.
static ASMJIT_INLINE Mem yword_ptr_abs(Ptr pAbs, const GpReg& index, uint32_t shift = 0, int32_t disp = 0) { return ptr_abs(pAbs, index, shift, disp, kSizeYWord); }

//! @brief Create a byte pointer operand.
static ASMJIT_INLINE Mem byte_ptr_abs(Ptr pAbs, const XmmReg& index, uint32_t shift = 0, int32_t disp = 0) { return ptr_abs(pAbs, index, shift, disp, kSizeByte); }
//! @brief Create a word pointer operand.
static ASMJIT_INLINE Mem word_ptr_abs(Ptr pAbs, const XmmReg& index, uint32_t shift = 0, int32_t disp = 0) { return ptr_abs(pAbs, index, shift, disp, kSizeWord); }
//! @brief Create a dword pointer operand.
static ASMJIT_INLINE Mem dword_ptr_abs(Ptr pAbs, const XmmReg& index, uint32_t shift = 0, int32_t disp = 0) { return ptr_abs(pAbs, index, shift, disp, kSizeDWord); }
//! @brief Create a qword pointer operand.
static ASMJIT_INLINE Mem qword_ptr_abs(Ptr pAbs, const XmmReg& index, uint32_t shift = 0, int32_t disp = 0) { return ptr_abs(pAbs, index, shift, disp, kSizeQWord); }
//! @brief Create a tword pointer operand.
static ASMJIT_INLINE Mem tword_ptr_abs(Ptr pAbs, const XmmReg& index, uint32_t shift = 0, int32_t disp = 0) { return ptr_abs(pAbs, index, shift, disp, kSizeTWord); }
//! @brief Create a oword pointer operand.
static ASMJIT_INLINE Mem oword_ptr_abs(Ptr pAbs, const XmmReg& index, uint32_t shift = 0, int32_t disp = 0) { return ptr_abs(pAbs, index, shift, disp, kSizeOWord); }
//! @brief Create a yword pointer operand.
static ASMJIT_INLINE Mem yword_ptr_abs(Ptr pAbs, const XmmReg& index, uint32_t shift = 0, int32_t disp = 0) { return ptr_abs(pAbs, index, shift, disp, kSizeYWord); }

//! @brief Create a byte pointer operand.
static ASMJIT_INLINE Mem byte_ptr_abs(Ptr pAbs, const YmmReg& index, uint32_t shift = 0, int32_t disp = 0) { return ptr_abs(pAbs, index, shift, disp, kSizeByte); }
//! @brief Create a word pointer operand.
static ASMJIT_INLINE Mem word_ptr_abs(Ptr pAbs, const YmmReg& index, uint32_t shift = 0, int32_t disp = 0) { return ptr_abs(pAbs, index, shift, disp, kSizeWord); }
//! @brief Create a dword pointer operand.
static ASMJIT_INLINE Mem dword_ptr_abs(Ptr pAbs, const YmmReg& index, uint32_t shift = 0, int32_t disp = 0) { return ptr_abs(pAbs, index, shift, disp, kSizeDWord); }
//! @brief Create a qword pointer operand.
static ASMJIT_INLINE Mem qword_ptr_abs(Ptr pAbs, const YmmReg& index, uint32_t shift = 0, int32_t disp = 0) { return ptr_abs(pAbs, index, shift, disp, kSizeQWord); }
//! @brief Create a tword pointer operand.
static ASMJIT_INLINE Mem tword_ptr_abs(Ptr pAbs, const YmmReg& index, uint32_t shift = 0, int32_t disp = 0) { return ptr_abs(pAbs, index, shift, disp, kSizeTWord); }
//! @brief Create a oword pointer operand.
static ASMJIT_INLINE Mem oword_ptr_abs(Ptr pAbs, const YmmReg& index, uint32_t shift = 0, int32_t disp = 0) { return ptr_abs(pAbs, index, shift, disp, kSizeOWord); }
//! @brief Create a yword pointer operand.
static ASMJIT_INLINE Mem yword_ptr_abs(Ptr pAbs, const YmmReg& index, uint32_t shift = 0, int32_t disp = 0) { return ptr_abs(pAbs, index, shift, disp, kSizeYWord); }

//! @brief Create a byte pointer operand.
static ASMJIT_INLINE Mem byte_ptr_abs(Ptr pAbs, const XmmVar& index, uint32_t shift = 0, int32_t disp = 0) { return ptr_abs(pAbs, index, shift, disp, kSizeByte); }
//! @brief Create a word pointer operand.
static ASMJIT_INLINE Mem word_ptr_abs(Ptr pAbs, const XmmVar& index, uint32_t shift = 0, int32_t disp = 0) { return ptr_abs(pAbs, index, shift, disp, kSizeWord); }
//! @brief Create a dword pointer operand.
static ASMJIT_INLINE Mem dword_ptr_abs(Ptr pAbs, const XmmVar& index, uint32_t shift = 0, int32_t disp = 0) { return ptr_abs(pAbs, index, shift, disp, kSizeDWord); }
//! @brief Create a qword pointer operand.
static ASMJIT_INLINE Mem qword_ptr_abs(Ptr pAbs, const XmmVar& index, uint32_t shift = 0, int32_t disp = 0) { return ptr_abs(pAbs, index, shift, disp, kSizeQWord); }
//! @brief Create a tword pointer operand.
static ASMJIT_INLINE Mem tword_ptr_abs(Ptr pAbs, const XmmVar& index, uint32_t shift = 0, int32_t disp = 0) { return ptr_abs(pAbs, index, shift, disp, kSizeTWord); }
//! @brief Create a oword pointer operand.
static ASMJIT_INLINE Mem oword_ptr_abs(Ptr pAbs, const XmmVar& index, uint32_t shift = 0, int32_t disp = 0) { return ptr_abs(pAbs, index, shift, disp, kSizeOWord); }
//! @brief Create a yword pointer operand.
static ASMJIT_INLINE Mem yword_ptr_abs(Ptr pAbs, const XmmVar& index, uint32_t shift = 0, int32_t disp = 0) { return ptr_abs(pAbs, index, shift, disp, kSizeYWord); }

//! @brief Create a byte pointer operand.
static ASMJIT_INLINE Mem byte_ptr_abs(Ptr pAbs, const YmmVar& index, uint32_t shift = 0, int32_t disp = 0) { return ptr_abs(pAbs, index, shift, disp, kSizeByte); }
//! @brief Create a word pointer operand.
static ASMJIT_INLINE Mem word_ptr_abs(Ptr pAbs, const YmmVar& index, uint32_t shift = 0, int32_t disp = 0) { return ptr_abs(pAbs, index, shift, disp, kSizeWord); }
//! @brief Create a dword pointer operand.
static ASMJIT_INLINE Mem dword_ptr_abs(Ptr pAbs, const YmmVar& index, uint32_t shift = 0, int32_t disp = 0) { return ptr_abs(pAbs, index, shift, disp, kSizeDWord); }
//! @brief Create a qword pointer operand.
static ASMJIT_INLINE Mem qword_ptr_abs(Ptr pAbs, const YmmVar& index, uint32_t shift = 0, int32_t disp = 0) { return ptr_abs(pAbs, index, shift, disp, kSizeQWord); }
//! @brief Create a tword pointer operand.
static ASMJIT_INLINE Mem tword_ptr_abs(Ptr pAbs, const YmmVar& index, uint32_t shift = 0, int32_t disp = 0) { return ptr_abs(pAbs, index, shift, disp, kSizeTWord); }
//! @brief Create a oword pointer operand.
static ASMJIT_INLINE Mem oword_ptr_abs(Ptr pAbs, const YmmVar& index, uint32_t shift = 0, int32_t disp = 0) { return ptr_abs(pAbs, index, shift, disp, kSizeOWord); }
//! @brief Create a yword pointer operand.
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

static ASMJIT_INLINE bool x86VarIsInt(uint32_t vType) {
  ASMJIT_ASSERT(vType < kVarTypeCount);
  return IntUtil::inInterval<uint32_t>(vType, _kVarTypeIntStart, _kVarTypeIntEnd);
}

static ASMJIT_INLINE bool x86VarIsFloat(uint32_t vType) {
  ASMJIT_ASSERT(vType < kVarTypeCount);
  return (_varInfo[vType].getDesc() & (kVarDescSp | kVarDescDp)) != 0;
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

ASMJIT_ENUM(kRegType) {
  //! @brief Gpd register.
  kRegTypeGpz = kRegTypeGpd
};

// ============================================================================
// [asmjit::x86::kRegCount]
// ============================================================================

//! @brief X86 registers count per class.
ASMJIT_ENUM(kRegCount) {
  //! @brief Base count of registers (8).
  kRegCountBase = 8,
  //! @brief Count of Gp registers (8).
  kRegCountGp = kRegCountBase,
  //! @brief Count of Xmm registers (8).
  kRegCountXmm = kRegCountBase,
  //! @brief Count of Ymm registers (8).
  kRegCountYmm = kRegCountBase
};

// ============================================================================
// [asmjit::x86::Variables]
// ============================================================================

//! @internal
//!
//! @brief Mapping of x64 variables into their real IDs.
//!
//! This mapping translates the following:
//! - @c kVarTypeIntPtr to @c kVarTypeInt32.
//! - @c kVarTypeUIntPtr to @c kVarTypeUInt32.
ASMJIT_VAR const uint8_t _varMapping[kVarTypeCount];

// ============================================================================
// [asmjit::x86::Registers]
// ============================================================================

//! @brief Gpd register.
ASMJIT_VAR const GpReg zax;
//! @brief Gpd register.
ASMJIT_VAR const GpReg zcx;
//! @brief Gpd register.
ASMJIT_VAR const GpReg zdx;
//! @brief Gpd register.
ASMJIT_VAR const GpReg zbx;
//! @brief Gpd register.
ASMJIT_VAR const GpReg zsp;
//! @brief Gpd register.
ASMJIT_VAR const GpReg zbp;
//! @brief Gpd register.
ASMJIT_VAR const GpReg zsi;
//! @brief Gpd register.
ASMJIT_VAR const GpReg zdi;

//! @brief Get Gp qword register.
static ASMJIT_INLINE GpReg gpz(uint32_t index) { return GpReg(kRegTypeGpd, index, 4); }

// ============================================================================
// [asmjit::x86::Mem]
// ============================================================================

//! @brief Create an intptr_t 32-bit pointer operand.
static ASMJIT_INLINE Mem intptr_ptr(const GpReg& base, int32_t disp = 0) { return ptr(base, disp, 4); }
//! @brief Create an intptr_t 32-bit pointer operand.
static ASMJIT_INLINE Mem intptr_ptr(const GpVar& base, int32_t disp = 0) { return ptr(base, disp, 4); }
//! @brief Create an intptr_t 32-bit pointer operand.
static ASMJIT_INLINE Mem intptr_ptr(const GpReg& base, const GpReg& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, 4); }
//! @brief Create an intptr_t 32-bit pointer operand.
static ASMJIT_INLINE Mem intptr_ptr(const GpVar& base, const GpVar& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, 4); }

//! @brief Create an intptr_t 32-bit pointer operand.
static ASMJIT_INLINE Mem intptr_ptr(const Label& label, int32_t disp = 0) { return ptr(label, disp, 4); }
//! @brief Create an intptr_t 32-bit pointer operand.
static ASMJIT_INLINE Mem intptr_ptr(const Label& label, const GpReg& index, uint32_t shift, int32_t disp = 0) { return ptr(label, index, shift, disp, 4); }
//! @brief Create an intptr_t 32-bit pointer operand.
static ASMJIT_INLINE Mem intptr_ptr(const Label& label, const GpVar& index, uint32_t shift, int32_t disp = 0) { return ptr(label, index, shift, disp, 4); }

//! @brief Create an intptr_t 32-bit pointer operand.
static ASMJIT_INLINE Mem intptr_ptr_abs(Ptr pAbs, int32_t disp = 0) { return ptr_abs(pAbs, disp, 4); }
//! @brief Create an intptr_t 32-bit pointer operand.
static ASMJIT_INLINE Mem intptr_ptr_abs(Ptr pAbs, const GpReg& index, uint32_t shift, int32_t disp = 0) { return ptr_abs(pAbs, index, shift, disp, 4); }
//! @brief Create an intptr_t 32-bit pointer operand.
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

ASMJIT_ENUM(kRegType) {
  //! @brief Gpq register.
  kRegTypeGpz = kRegTypeGpq
};

// ============================================================================
// [asmjit::x64::kRegCount]
// ============================================================================

//! @brief X86 registers count per class.
ASMJIT_ENUM(kRegCount) {
  //! @brief Base count of registers (16).
  kRegCountBase = 16,
  //! @brief Count of Gp registers (16).
  kRegCountGp = kRegCountBase,
  //! @brief Count of Xmm registers (16).
  kRegCountXmm = kRegCountBase,
  //! @brief Count of Ymm registers (16).
  kRegCountYmm = kRegCountBase
};

// ============================================================================
// [asmjit::x64::Variables]
// ============================================================================

//! @internal
//!
//! @brief Mapping of x64 variables into their real IDs.
//!
//! This mapping translates the following:
//! - @c kVarTypeIntPtr to @c kVarTypeInt64.
//! - @c kVarTypeUIntPtr to @c kVarTypeUInt64.
ASMJIT_VAR const uint8_t _varMapping[kVarTypeCount];

// ============================================================================
// [asmjit::x64::Registers]
// ============================================================================

//! @brief Gpb register.
ASMJIT_VAR const GpReg spl;
//! @brief Gpb register.
ASMJIT_VAR const GpReg bpl;
//! @brief Gpb register.
ASMJIT_VAR const GpReg sil;
//! @brief Gpb register.
ASMJIT_VAR const GpReg dil;
//! @brief Gpb register.
ASMJIT_VAR const GpReg r8b;
//! @brief Gpb register.
ASMJIT_VAR const GpReg r9b;
//! @brief Gpb register.
ASMJIT_VAR const GpReg r10b;
//! @brief Gpb register.
ASMJIT_VAR const GpReg r11b;
//! @brief Gpb register.
ASMJIT_VAR const GpReg r12b;
//! @brief Gpb register.
ASMJIT_VAR const GpReg r13b;
//! @brief Gpb register.
ASMJIT_VAR const GpReg r14b;
//! @brief Gpb register.
ASMJIT_VAR const GpReg r15b;

//! @brief Gpw register.
ASMJIT_VAR const GpReg r8w;
//! @brief Gpw register.
ASMJIT_VAR const GpReg r9w;
//! @brief Gpw register.
ASMJIT_VAR const GpReg r10w;
//! @brief Gpw register.
ASMJIT_VAR const GpReg r11w;
//! @brief Gpw register.
ASMJIT_VAR const GpReg r12w;
//! @brief Gpw register.
ASMJIT_VAR const GpReg r13w;
//! @brief Gpw register.
ASMJIT_VAR const GpReg r14w;
//! @brief Gpw register.
ASMJIT_VAR const GpReg r15w;

//! @brief Gpd register.
ASMJIT_VAR const GpReg r8d;
//! @brief Gpd register.
ASMJIT_VAR const GpReg r9d;
//! @brief Gpd register.
ASMJIT_VAR const GpReg r10d;
//! @brief Gpd register.
ASMJIT_VAR const GpReg r11d;
//! @brief Gpd register.
ASMJIT_VAR const GpReg r12d;
//! @brief Gpd register.
ASMJIT_VAR const GpReg r13d;
//! @brief Gpd register.
ASMJIT_VAR const GpReg r14d;
//! @brief Gpd register.
ASMJIT_VAR const GpReg r15d;

//! @brief Gpq register.
ASMJIT_VAR const GpReg rax;
//! @brief Gpq register.
ASMJIT_VAR const GpReg rcx;
//! @brief Gpq register.
ASMJIT_VAR const GpReg rdx;
//! @brief Gpq register.
ASMJIT_VAR const GpReg rbx;
//! @brief Gpq register.
ASMJIT_VAR const GpReg rsp;
//! @brief Gpq register.
ASMJIT_VAR const GpReg rbp;
//! @brief Gpq register.
ASMJIT_VAR const GpReg rsi;
//! @brief Gpq register.
ASMJIT_VAR const GpReg rdi;

//! @brief Gpq register.
ASMJIT_VAR const GpReg r8;
//! @brief Gpq register.
ASMJIT_VAR const GpReg r9;
//! @brief Gpq register.
ASMJIT_VAR const GpReg r10;
//! @brief Gpq register.
ASMJIT_VAR const GpReg r11;
//! @brief Gpq register.
ASMJIT_VAR const GpReg r12;
//! @brief Gpq register.
ASMJIT_VAR const GpReg r13;
//! @brief Gpq register.
ASMJIT_VAR const GpReg r14;
//! @brief Gpq register.
ASMJIT_VAR const GpReg r15;

//! @brief Gpq register.
ASMJIT_VAR const GpReg zax;
//! @brief Gpq register.
ASMJIT_VAR const GpReg zcx;
//! @brief Gpq register.
ASMJIT_VAR const GpReg zdx;
//! @brief Gpq register.
ASMJIT_VAR const GpReg zbx;
//! @brief Gpq register.
ASMJIT_VAR const GpReg zsp;
//! @brief Gpq register.
ASMJIT_VAR const GpReg zbp;
//! @brief Gpq register.
ASMJIT_VAR const GpReg zsi;
//! @brief Gpq register.
ASMJIT_VAR const GpReg zdi;

//! @brief Xmm register.
ASMJIT_VAR const XmmReg xmm8;
//! @brief Xmm register.
ASMJIT_VAR const XmmReg xmm9;
//! @brief Xmm register.
ASMJIT_VAR const XmmReg xmm10;
//! @brief Xmm register.
ASMJIT_VAR const XmmReg xmm11;
//! @brief Xmm register.
ASMJIT_VAR const XmmReg xmm12;
//! @brief Xmm register.
ASMJIT_VAR const XmmReg xmm13;
//! @brief Xmm register.
ASMJIT_VAR const XmmReg xmm14;
//! @brief Xmm register.
ASMJIT_VAR const XmmReg xmm15;

//! @brief Ymm register.
ASMJIT_VAR const YmmReg ymm8;
//! @brief Ymm register.
ASMJIT_VAR const YmmReg ymm9;
//! @brief Ymm register.
ASMJIT_VAR const YmmReg ymm10;
//! @brief Ymm register.
ASMJIT_VAR const YmmReg ymm11;
//! @brief Ymm register.
ASMJIT_VAR const YmmReg ymm12;
//! @brief Ymm register.
ASMJIT_VAR const YmmReg ymm13;
//! @brief Ymm register.
ASMJIT_VAR const YmmReg ymm14;
//! @brief Ymm register.
ASMJIT_VAR const YmmReg ymm15;

//! @brief Get Gpq register.
static ASMJIT_INLINE GpReg gpq(uint32_t index) { return GpReg(kRegTypeGpq, index, 8); }
//! @brief Get Gpq register.
static ASMJIT_INLINE GpReg gpz(uint32_t index) { return GpReg(kRegTypeGpq, index, 8); }

// ============================================================================
// [asmjit::x64::Mem]
// ============================================================================

//! @brief Create an intptr_t 64-bit pointer operand.
static ASMJIT_INLINE Mem intptr_ptr(const Label& label, int32_t disp = 0) { return ptr(label, disp, 8); }
//! @brief Create an intptr_t 64-bit pointer operand.
static ASMJIT_INLINE Mem intptr_ptr(const Label& label, const GpReg& index, uint32_t shift, int32_t disp = 0) { return ptr(label, index, shift, disp, 8); }
//! @brief Create an intptr_t 64-bit pointer operand.
static ASMJIT_INLINE Mem intptr_ptr(const Label& label, const GpVar& index, uint32_t shift, int32_t disp = 0) { return ptr(label, index, shift, disp, 8); }

//! @brief Create an intptr_t 64-bit pointer operand.
static ASMJIT_INLINE Mem intptr_ptr_abs(Ptr pAbs, int32_t disp = 0) { return ptr_abs(pAbs, disp, 8); }
//! @brief Create an intptr_t 64-bit pointer operand.
static ASMJIT_INLINE Mem intptr_ptr_abs(Ptr pAbs, const GpReg& index, uint32_t shift, int32_t disp = 0) { return ptr_abs(pAbs, index, shift, disp, 8); }
//! @brief Create an intptr_t 64-bit pointer operand.
static ASMJIT_INLINE Mem intptr_ptr_abs(Ptr pAbs, const GpVar& index, uint32_t shift, int32_t disp = 0) { return ptr_abs(pAbs, index, shift, disp, 8); }

//! @brief Create an intptr_t 64-bit pointer operand.
static ASMJIT_INLINE Mem intptr_ptr(const GpReg& base, int32_t disp = 0) { return ptr(base, disp, 8); }
//! @brief Create an intptr_t 64-bit pointer operand.
static ASMJIT_INLINE Mem intptr_ptr(const GpVar& base, int32_t disp = 0) { return ptr(base, disp, 8); }
//! @brief Create an intptr_t 64-bit pointer operand.
static ASMJIT_INLINE Mem intptr_ptr(const GpReg& base, const GpReg& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, 8); }
//! @brief Create an intptr_t 64-bit pointer operand.
static ASMJIT_INLINE Mem intptr_ptr(const GpVar& base, const GpVar& index, uint32_t shift = 0, int32_t disp = 0) { return ptr(base, index, shift, disp, 8); }

//! @}

} // x64 namespace
} // asmjit namespace

#endif // ASMJIT_BUILD_X64

// [Api-End]
#include "../base/apiend.h"

// [Guard]
#endif // _ASMJIT_X86_X86DEFS_H
