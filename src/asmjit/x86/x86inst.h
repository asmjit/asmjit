// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_X86_X86INST_H
#define _ASMJIT_X86_X86INST_H

// [Dependencies]
#include "../base/assembler.h"
#include "../base/globals.h"
#include "../base/operand.h"
#include "../base/utils.h"

// [Api-Begin]
#include "../asmjit_apibegin.h"

namespace asmjit {

//! \addtogroup asmjit_x86
//! \{

// ============================================================================
// [asmjit::X86Cond - Globals]
// ============================================================================

//! \internal
//!
//! X86/X64 condition codes to reversed condition codes map.
ASMJIT_VARAPI const uint32_t _x86ReverseCond[20];

//! \internal
//!
//! X86/X64 condition codes to "cmovcc" group map.
ASMJIT_VARAPI const uint32_t _x86CondToCmovcc[20];

//! \internal
//!
//! X86/X64 condition codes to "jcc" group map.
ASMJIT_VARAPI const uint32_t _x86CondToJcc[20];

//! \internal
//!
//! X86/X64 condition codes to "setcc" group map.
ASMJIT_VARAPI const uint32_t _x86CondToSetcc[20];

// ============================================================================
// [asmjit::X86EFlags]
// ============================================================================

//! X86/X64 EFLAGs bits (AsmJit specific).
//!
//! Each instruction stored in AsmJit database contains flags that instruction
//! uses (reads) and flags that instruction modifies (writes). This is used by
//! instruction reordering, but can be used by third parties as it's part of
//! AsmJit API.
//!
//! NOTE: Flags defined here don't correspond to real flags used by X86/X64
//! architecture, defined in Intel's Manual Section `3.4.3 - EFLAGS Register`.
//!
//! NOTE: Flags are designed to fit in an 8-bit integer.
ASMJIT_ENUM(X86EFlags) {
  // --------------------------------------------------------------------------
  // TODO: This is wrong.
  // generate-x86.js relies on the values of these masks, the tool has to be
  // changed as you plan to modify `X86EFlags`.
  // --------------------------------------------------------------------------

  kX86EFlagO              = 0x01,        //!< Overflow flag (OF).
  kX86EFlagS              = 0x02,        //!< Sign flag (SF).
  kX86EFlagZ              = 0x04,        //!< Zero flag (ZF).
  kX86EFlagA              = 0x08,        //!< Adjust flag (AF).
  kX86EFlagP              = 0x10,        //!< Parity flag (PF).
  kX86EFlagC              = 0x20,        //!< Carry flag (CF).
  kX86EFlagD              = 0x40,        //!< Direction flag (DF).
  kX86EFlagX              = 0x80         //!< Any other flag that AsmJit doesn't use.
};

// ============================================================================
// [asmjit::X86FpSw]
// ============================================================================

//! X86/X64 FPU status word.
ASMJIT_ENUM(X86FpSw) {
  kX86FpSw_Invalid        = 0x0001,
  kX86FpSw_Denormalized   = 0x0002,
  kX86FpSw_DivByZero      = 0x0004,
  kX86FpSw_Overflow       = 0x0008,
  kX86FpSw_Underflow      = 0x0010,
  kX86FpSw_Precision      = 0x0020,
  kX86FpSw_StackFault     = 0x0040,
  kX86FpSw_Interrupt      = 0x0080,
  kX86FpSw_C0             = 0x0100,
  kX86FpSw_C1             = 0x0200,
  kX86FpSw_C2             = 0x0400,
  kX86FpSw_Top            = 0x3800,
  kX86FpSw_C3             = 0x4000,
  kX86FpSw_Busy           = 0x8000
};

//  ============================================================================
// [asmjit::X86FpCw]
//  ============================================================================

//! X86/X64 FPU control word.
ASMJIT_ENUM(X86FpCw) {
  // Bits 0-5.
  kX86FpCw_EM_Mask        = 0x003F,
  kX86FpCw_EM_Invalid     = 0x0001,
  kX86FpCw_EM_Denormal    = 0x0002,
  kX86FpCw_EM_DivByZero   = 0x0004,
  kX86FpCw_EM_Overflow    = 0x0008,
  kX86FpCw_EM_Underflow   = 0x0010,
  kX86FpCw_EM_Inexact     = 0x0020,

  // Bits 8-9.
  kX86FpCw_PC_Mask        = 0x0300,
  kX86FpCw_PC_Float       = 0x0000,
  kX86FpCw_PC_Reserved    = 0x0100,
  kX86FpCw_PC_Double      = 0x0200,
  kX86FpCw_PC_Extended    = 0x0300,

  // Bits 10-11.
  kX86FpCw_RC_Mask        = 0x0C00,
  kX86FpCw_RC_Nearest     = 0x0000,
  kX86FpCw_RC_Down        = 0x0400,
  kX86FpCw_RC_Up          = 0x0800,
  kX86FpCw_RC_Truncate    = 0x0C00,

  // Bit 12.
  kX86FpCw_IC_Mask        = 0x1000,
  kX86FpCw_IC_Projective  = 0x0000,
  kX86FpCw_IC_Affine      = 0x1000
};

// ============================================================================
// [asmjit::X86Inst]
// ============================================================================

//! X86/X64 instruction data.
struct X86Inst {
  //! Instruction id.
  //!
  //! Note that these instruction codes are AsmJit specific. Each instruction
  //! has a unique ID that is used as an index to AsmJit instruction table. The
  //! list is sorted alphabetically except instructions starting with `j`,
  //! because the `jcc` instruction is a composition of an opcode and condition
  //! code. It means that these instructions are sorted as `jcc`, `jecxz` and
  //! `jmp`. Please use \ref `X86Inst::getIdByName()` if you need instruction
  //! name to ID mapping and are not aware on how to handle such case.
  ASMJIT_ENUM(Id) {
    kIdNone = 0,
    kIdAaa,                              // X86
    kIdAad,                              // X86
    kIdAam,                              // X86
    kIdAas,                              // X86
    kIdAdc,                              // ANY
    kIdAdcx,                             // ADX
    kIdAdd,                              // ANY
    kIdAddpd,                            // SSE2
    kIdAddps,                            // SSE
    kIdAddsd,                            // SSE2
    kIdAddss,                            // SSE
    kIdAddsubpd,                         // SSE3
    kIdAddsubps,                         // SSE3
    kIdAdox,                             // ADX
    kIdAesdec,                           // AESNI
    kIdAesdeclast,                       // AESNI
    kIdAesenc,                           // AESNI
    kIdAesenclast,                       // AESNI
    kIdAesimc,                           // AESNI
    kIdAeskeygenassist,                  // AESNI
    kIdAnd,                              // ANY
    kIdAndn,                             // BMI
    kIdAndnpd,                           // SSE2
    kIdAndnps,                           // SSE
    kIdAndpd,                            // SSE2
    kIdAndps,                            // SSE
    kIdBextr,                            // BMI
    kIdBlcfill,                          // TBM
    kIdBlci,                             // TBM
    kIdBlcic,                            // TBM
    kIdBlcmsk,                           // TBM
    kIdBlcs,                             // TBM
    kIdBlendpd,                          // SSE4.1
    kIdBlendps,                          // SSE4.1
    kIdBlendvpd,                         // SSE4.1
    kIdBlendvps,                         // SSE4.1
    kIdBlsfill,                          // TBM
    kIdBlsi,                             // BMI
    kIdBlsic,                            // TBM
    kIdBlsmsk,                           // BMI
    kIdBlsr,                             // BMI
    kIdBsf,                              // ANY
    kIdBsr,                              // ANY
    kIdBswap,                            // I486+
    kIdBt,                               // ANY
    kIdBtc,                              // ANY
    kIdBtr,                              // ANY
    kIdBts,                              // ANY
    kIdBzhi,                             // BMI2
    kIdCall,                             // ANY
    kIdCbw,                              // ANY
    kIdCdq,                              // ANY
    kIdCdqe,                             // X64
    kIdClac,                             // SMAP
    kIdClc,                              // ANY
    kIdCld,                              // ANY
    kIdClflush,                          // CLFLUSH
    kIdClflushopt,                       // CLFLUSH_OPT
    kIdClwb,                             // CLWB
    kIdClzero,                           // CLZERO
    kIdCmc,                              // ANY
    kIdCmova,                            // I586+ (cmovcc)
    kIdCmovae,                           // I586+ (cmovcc)
    kIdCmovb,                            // I586+ (cmovcc)
    kIdCmovbe,                           // I586+ (cmovcc)
    kIdCmovc,                            // I586+ (cmovcc)
    kIdCmove,                            // I586+ (cmovcc)
    kIdCmovg,                            // I586+ (cmovcc)
    kIdCmovge,                           // I586+ (cmovcc)
    kIdCmovl,                            // I586+ (cmovcc)
    kIdCmovle,                           // I586+ (cmovcc)
    kIdCmovna,                           // I586+ (cmovcc)
    kIdCmovnae,                          // I586+ (cmovcc)
    kIdCmovnb,                           // I586+ (cmovcc)
    kIdCmovnbe,                          // I586+ (cmovcc)
    kIdCmovnc,                           // I586+ (cmovcc)
    kIdCmovne,                           // I586+ (cmovcc)
    kIdCmovng,                           // I586+ (cmovcc)
    kIdCmovnge,                          // I586+ (cmovcc)
    kIdCmovnl,                           // I586+ (cmovcc)
    kIdCmovnle,                          // I586+ (cmovcc)
    kIdCmovno,                           // I586+ (cmovcc)
    kIdCmovnp,                           // I586+ (cmovcc)
    kIdCmovns,                           // I586+ (cmovcc)
    kIdCmovnz,                           // I586+ (cmovcc)
    kIdCmovo,                            // I586+ (cmovcc)
    kIdCmovp,                            // I586+ (cmovcc)
    kIdCmovpe,                           // I586+ (cmovcc)
    kIdCmovpo,                           // I586+ (cmovcc)
    kIdCmovs,                            // I586+ (cmovcc)
    kIdCmovz,                            // I586+ (cmovcc)
    kIdCmp,                              // ANY
    kIdCmppd,                            // SSE2
    kIdCmpps,                            // SSE
    kIdCmps,                             // ANY
    kIdCmpsd,                            // SSE2
    kIdCmpss,                            // SSE
    kIdCmpxchg,                          // I486+
    kIdCmpxchg16b,                       // X64
    kIdCmpxchg8b,                        // I586+
    kIdComisd,                           // SSE2
    kIdComiss,                           // SSE
    kIdCpuid,                            // I486+
    kIdCqo,                              // X64
    kIdCrc32,                            // SSE4.2
    kIdCvtdq2pd,                         // SSE2
    kIdCvtdq2ps,                         // SSE2
    kIdCvtpd2dq,                         // SSE2
    kIdCvtpd2pi,                         // SSE2
    kIdCvtpd2ps,                         // SSE2
    kIdCvtpi2pd,                         // SSE2
    kIdCvtpi2ps,                         // SSE
    kIdCvtps2dq,                         // SSE2
    kIdCvtps2pd,                         // SSE2
    kIdCvtps2pi,                         // SSE
    kIdCvtsd2si,                         // SSE2
    kIdCvtsd2ss,                         // SSE2
    kIdCvtsi2sd,                         // SSE2
    kIdCvtsi2ss,                         // SSE
    kIdCvtss2sd,                         // SSE2
    kIdCvtss2si,                         // SSE
    kIdCvttpd2dq,                        // SSE2
    kIdCvttpd2pi,                        // SSE2
    kIdCvttps2dq,                        // SSE2
    kIdCvttps2pi,                        // SSE
    kIdCvttsd2si,                        // SSE2
    kIdCvttss2si,                        // SSE
    kIdCwd,                              // ANY
    kIdCwde,                             // ANY
    kIdDaa,                              // X86
    kIdDas,                              // X86
    kIdDec,                              // ANY
    kIdDiv,                              // ANY
    kIdDivpd,                            // SSE2
    kIdDivps,                            // SSE
    kIdDivsd,                            // SSE2
    kIdDivss,                            // SSE
    kIdDppd,                             // SSE4.1
    kIdDpps,                             // SSE4.1
    kIdEmms,                             // MMX
    kIdEnter,                            // ANY
    kIdExtractps,                        // SSE4.1
    kIdExtrq,                            // SSE4A
    kIdF2xm1,                            // FPU
    kIdFabs,                             // FPU
    kIdFadd,                             // FPU
    kIdFaddp,                            // FPU
    kIdFbld,                             // FPU
    kIdFbstp,                            // FPU
    kIdFchs,                             // FPU
    kIdFclex,                            // FPU
    kIdFcmovb,                           // FPU
    kIdFcmovbe,                          // FPU
    kIdFcmove,                           // FPU
    kIdFcmovnb,                          // FPU
    kIdFcmovnbe,                         // FPU
    kIdFcmovne,                          // FPU
    kIdFcmovnu,                          // FPU
    kIdFcmovu,                           // FPU
    kIdFcom,                             // FPU
    kIdFcomi,                            // FPU
    kIdFcomip,                           // FPU
    kIdFcomp,                            // FPU
    kIdFcompp,                           // FPU
    kIdFcos,                             // FPU
    kIdFdecstp,                          // FPU
    kIdFdiv,                             // FPU
    kIdFdivp,                            // FPU
    kIdFdivr,                            // FPU
    kIdFdivrp,                           // FPU
    kIdFemms,                            // 3DNOW
    kIdFfree,                            // FPU
    kIdFiadd,                            // FPU
    kIdFicom,                            // FPU
    kIdFicomp,                           // FPU
    kIdFidiv,                            // FPU
    kIdFidivr,                           // FPU
    kIdFild,                             // FPU
    kIdFimul,                            // FPU
    kIdFincstp,                          // FPU
    kIdFinit,                            // FPU
    kIdFist,                             // FPU
    kIdFistp,                            // FPU
    kIdFisttp,                           // SSE3
    kIdFisub,                            // FPU
    kIdFisubr,                           // FPU
    kIdFld,                              // FPU
    kIdFld1,                             // FPU
    kIdFldcw,                            // FPU
    kIdFldenv,                           // FPU
    kIdFldl2e,                           // FPU
    kIdFldl2t,                           // FPU
    kIdFldlg2,                           // FPU
    kIdFldln2,                           // FPU
    kIdFldpi,                            // FPU
    kIdFldz,                             // FPU
    kIdFmul,                             // FPU
    kIdFmulp,                            // FPU
    kIdFnclex,                           // FPU
    kIdFninit,                           // FPU
    kIdFnop,                             // FPU
    kIdFnsave,                           // FPU
    kIdFnstcw,                           // FPU
    kIdFnstenv,                          // FPU
    kIdFnstsw,                           // FPU
    kIdFpatan,                           // FPU
    kIdFprem,                            // FPU
    kIdFprem1,                           // FPU
    kIdFptan,                            // FPU
    kIdFrndint,                          // FPU
    kIdFrstor,                           // FPU
    kIdFsave,                            // FPU
    kIdFscale,                           // FPU
    kIdFsin,                             // FPU
    kIdFsincos,                          // FPU
    kIdFsqrt,                            // FPU
    kIdFst,                              // FPU
    kIdFstcw,                            // FPU
    kIdFstenv,                           // FPU
    kIdFstp,                             // FPU
    kIdFstsw,                            // FPU
    kIdFsub,                             // FPU
    kIdFsubp,                            // FPU
    kIdFsubr,                            // FPU
    kIdFsubrp,                           // FPU
    kIdFtst,                             // FPU
    kIdFucom,                            // FPU
    kIdFucomi,                           // FPU
    kIdFucomip,                          // FPU
    kIdFucomp,                           // FPU
    kIdFucompp,                          // FPU
    kIdFwait,                            // FPU
    kIdFxam,                             // FPU
    kIdFxch,                             // FPU
    kIdFxrstor,                          // FPU
    kIdFxrstor64,                        // FPU & X64
    kIdFxsave,                           // FPU
    kIdFxsave64,                         // FPU & X64
    kIdFxtract,                          // FPU
    kIdFyl2x,                            // FPU
    kIdFyl2xp1,                          // FPU
    kIdHaddpd,                           // SSE3
    kIdHaddps,                           // SSE3
    kIdHsubpd,                           // SSE3
    kIdHsubps,                           // SSE3
    kIdIdiv,                             // ANY
    kIdImul,                             // ANY
    kIdIn,                               // ANY
    kIdInc,                              // ANY
    kIdIns,                              // ANY
    kIdInsertps,                         // SSE4.1
    kIdInsertq,                          // SSE4A
    kIdInt,                              // ANY
    kIdInt3,                             // ANY
    kIdInto,                             // ANY
    kIdJa,                               // ANY (jcc)
    kIdJae,                              // ANY (jcc)
    kIdJb,                               // ANY (jcc)
    kIdJbe,                              // ANY (jcc)
    kIdJc,                               // ANY (jcc)
    kIdJe,                               // ANY (jcc)
    kIdJg,                               // ANY (jcc)
    kIdJge,                              // ANY (jcc)
    kIdJl,                               // ANY (jcc)
    kIdJle,                              // ANY (jcc)
    kIdJna,                              // ANY (jcc)
    kIdJnae,                             // ANY (jcc)
    kIdJnb,                              // ANY (jcc)
    kIdJnbe,                             // ANY (jcc)
    kIdJnc,                              // ANY (jcc)
    kIdJne,                              // ANY (jcc)
    kIdJng,                              // ANY (jcc)
    kIdJnge,                             // ANY (jcc)
    kIdJnl,                              // ANY (jcc)
    kIdJnle,                             // ANY (jcc)
    kIdJno,                              // ANY (jcc)
    kIdJnp,                              // ANY (jcc)
    kIdJns,                              // ANY (jcc)
    kIdJnz,                              // ANY (jcc)
    kIdJo,                               // ANY (jcc)
    kIdJp,                               // ANY (jcc)
    kIdJpe,                              // ANY (jcc)
    kIdJpo,                              // ANY (jcc)
    kIdJs,                               // ANY (jcc)
    kIdJz,                               // ANY (jcc)
    kIdJecxz,                            // ANY (jcxz/jecxz/jrcxz)
    kIdJmp,                              // ANY (jmp)
    kIdKaddb,                            // AVX512DQ
    kIdKaddd,                            // AVX512BW
    kIdKaddq,                            // AVX512BW
    kIdKaddw,                            // AVX512DQ
    kIdKandb,                            // AVX512DQ
    kIdKandd,                            // AVX512BW
    kIdKandnb,                           // AVX512DQ
    kIdKandnd,                           // AVX512BW
    kIdKandnq,                           // AVX512BW
    kIdKandnw,                           // AVX512F
    kIdKandq,                            // AVX512BW
    kIdKandw,                            // AVX512F
    kIdKmovb,                            // AVX512DQ
    kIdKmovd,                            // AVX512BW
    kIdKmovq,                            // AVX512BW
    kIdKmovw,                            // AVX512F
    kIdKnotb,                            // AVX512DQ
    kIdKnotd,                            // AVX512BW
    kIdKnotq,                            // AVX512BW
    kIdKnotw,                            // AVX512F
    kIdKorb,                             // AVX512DQ
    kIdKord,                             // AVX512BW
    kIdKorq,                             // AVX512BW
    kIdKortestb,                         // AVX512DQ
    kIdKortestd,                         // AVX512BW
    kIdKortestq,                         // AVX512BW
    kIdKortestw,                         // AVX512F
    kIdKorw,                             // AVX512F
    kIdKshiftlb,                         // AVX512DQ
    kIdKshiftld,                         // AVX512BW
    kIdKshiftlq,                         // AVX512BW
    kIdKshiftlw,                         // AVX512F
    kIdKshiftrb,                         // AVX512DQ
    kIdKshiftrd,                         // AVX512BW
    kIdKshiftrq,                         // AVX512BW
    kIdKshiftrw,                         // AVX512F
    kIdKtestb,                           // AVX512DQ
    kIdKtestd,                           // AVX512BW
    kIdKtestq,                           // AVX512BW
    kIdKtestw,                           // AVX512DQ
    kIdKunpckbw,                         // AVX512F
    kIdKunpckdq,                         // AVX512BW
    kIdKunpckwd,                         // AVX512BW
    kIdKxnorb,                           // AVX512DQ
    kIdKxnord,                           // AVX512BW
    kIdKxnorq,                           // AVX512BW
    kIdKxnorw,                           // AVX512F
    kIdKxorb,                            // AVX512DQ
    kIdKxord,                            // AVX512BW
    kIdKxorq,                            // AVX512BW
    kIdKxorw,                            // AVX512F
    kIdLahf,                             // LAHF_SAHF
    kIdLddqu,                            // SSE3
    kIdLdmxcsr,                          // SSE
    kIdLea,                              // ANY
    kIdLeave,                            // ANY
    kIdLfence,                           // SSE2
    kIdLods,                             // ANY
    kIdLoop,                             // ANY
    kIdLoope,                            // ANY
    kIdLoopne,                           // ANY
    kIdLzcnt,                            // LZCNT
    kIdMaskmovdqu,                       // SSE2
    kIdMaskmovq,                         // MMX2
    kIdMaxpd,                            // SSE2
    kIdMaxps,                            // SSE
    kIdMaxsd,                            // SSE2
    kIdMaxss,                            // SSE
    kIdMfence,                           // SSE2
    kIdMinpd,                            // SSE2
    kIdMinps,                            // SSE
    kIdMinsd,                            // SSE2
    kIdMinss,                            // SSE
    kIdMonitor,                          // SSE3
    kIdMov,                              // ANY
    kIdMovapd,                           // SSE2
    kIdMovaps,                           // SSE
    kIdMovbe,                            // MOVBE
    kIdMovd,                             // MMX|SSE2
    kIdMovddup,                          // SSE3
    kIdMovdq2q,                          // SSE2
    kIdMovdqa,                           // SSE2
    kIdMovdqu,                           // SSE2
    kIdMovhlps,                          // SSE
    kIdMovhpd,                           // SSE2
    kIdMovhps,                           // SSE
    kIdMovlhps,                          // SSE
    kIdMovlpd,                           // SSE2
    kIdMovlps,                           // SSE
    kIdMovmskpd,                         // SSE2
    kIdMovmskps,                         // SSE2
    kIdMovntdq,                          // SSE2
    kIdMovntdqa,                         // SSE4.1
    kIdMovnti,                           // SSE2
    kIdMovntpd,                          // SSE2
    kIdMovntps,                          // SSE
    kIdMovntq,                           // MMX2
    kIdMovntsd,                          // SSE4A
    kIdMovntss,                          // SSE4A
    kIdMovq,                             // MMX|SSE|SSE2
    kIdMovq2dq,                          // SSE2
    kIdMovs,                             // ANY
    kIdMovsd,                            // SSE2
    kIdMovshdup,                         // SSE3
    kIdMovsldup,                         // SSE3
    kIdMovss,                            // SSE
    kIdMovsx,                            // ANY
    kIdMovsxd,                           // ANY
    kIdMovupd,                           // SSE2
    kIdMovups,                           // SSE
    kIdMovzx,                            // ANY
    kIdMpsadbw,                          // SSE4.1
    kIdMul,                              // ANY
    kIdMulpd,                            // SSE2
    kIdMulps,                            // SSE
    kIdMulsd,                            // SSE2
    kIdMulss,                            // SSE
    kIdMulx,                             // BMI2
    kIdMwait,                            // SSE3
    kIdNeg,                              // ANY
    kIdNop,                              // ANY
    kIdNot,                              // ANY
    kIdOr,                               // ANY
    kIdOrpd,                             // SSE2
    kIdOrps,                             // SSE
    kIdOut,                              // ANY
    kIdOuts,                             // ANY
    kIdPabsb,                            // SSSE3
    kIdPabsd,                            // SSSE3
    kIdPabsw,                            // SSSE3
    kIdPackssdw,                         // MMX|SSE2
    kIdPacksswb,                         // MMX|SSE2
    kIdPackusdw,                         // SSE4.1
    kIdPackuswb,                         // MMX|SSE2
    kIdPaddb,                            // MMX|SSE2
    kIdPaddd,                            // MMX|SSE2
    kIdPaddq,                            // SSE2
    kIdPaddsb,                           // MMX|SSE2
    kIdPaddsw,                           // MMX|SSE2
    kIdPaddusb,                          // MMX|SSE2
    kIdPaddusw,                          // MMX|SSE2
    kIdPaddw,                            // MMX|SSE2
    kIdPalignr,                          // SSSE3
    kIdPand,                             // MMX|SSE2
    kIdPandn,                            // MMX|SSE2
    kIdPause,                            // SSE2.
    kIdPavgb,                            // MMX2
    kIdPavgusb,                          // 3DNOW
    kIdPavgw,                            // MMX2
    kIdPblendvb,                         // SSE4.1
    kIdPblendw,                          // SSE4.1
    kIdPclmulqdq,                        // PCLMULQDQ
    kIdPcmpeqb,                          // MMX|SSE2
    kIdPcmpeqd,                          // MMX|SSE2
    kIdPcmpeqq,                          // SSE4.1
    kIdPcmpeqw,                          // MMX|SSE2
    kIdPcmpestri,                        // SSE4.2
    kIdPcmpestrm,                        // SSE4.2
    kIdPcmpgtb,                          // MMX|SSE2
    kIdPcmpgtd,                          // MMX|SSE2
    kIdPcmpgtq,                          // SSE4.2
    kIdPcmpgtw,                          // MMX|SSE2
    kIdPcmpistri,                        // SSE4.2
    kIdPcmpistrm,                        // SSE4.2
    kIdPcommit,                          // PCOMMIT
    kIdPdep,                             // BMI2
    kIdPext,                             // BMI2
    kIdPextrb,                           // SSE4.1
    kIdPextrd,                           // SSE4.1
    kIdPextrq,                           // SSE4.1
    kIdPextrw,                           // MMX2|SSE2
    kIdPf2id,                            // 3DNOW
    kIdPf2iw,                            // 3DNOW2
    kIdPfacc,                            // 3DNOW
    kIdPfadd,                            // 3DNOW
    kIdPfcmpeq,                          // 3DNOW
    kIdPfcmpge,                          // 3DNOW
    kIdPfcmpgt,                          // 3DNOW
    kIdPfmax,                            // 3DNOW
    kIdPfmin,                            // 3DNOW
    kIdPfmul,                            // 3DNOW
    kIdPfnacc,                           // 3DNOW2
    kIdPfpnacc,                          // 3DNOW2
    kIdPfrcp,                            // 3DNOW
    kIdPfrcpit1,                         // 3DNOW
    kIdPfrcpit2,                         // 3DNOW
    kIdPfrcpv,                           // GEODE
    kIdPfrsqit1,                         // 3DNOW
    kIdPfrsqrt,                          // 3DNOW
    kIdPfrsqrtv,                         // GEODE
    kIdPfsub,                            // 3DNOW
    kIdPfsubr,                           // 3DNOW
    kIdPhaddd,                           // SSSE3
    kIdPhaddsw,                          // SSSE3
    kIdPhaddw,                           // SSSE3
    kIdPhminposuw,                       // SSE4.1
    kIdPhsubd,                           // SSSE3
    kIdPhsubsw,                          // SSSE3
    kIdPhsubw,                           // SSSE3
    kIdPi2fd,                            // 3DNOW
    kIdPi2fw,                            // 3DNOW2
    kIdPinsrb,                           // SSE4.1
    kIdPinsrd,                           // SSE4.1
    kIdPinsrq,                           // SSE4.1
    kIdPinsrw,                           // MMX2
    kIdPmaddubsw,                        // SSSE3
    kIdPmaddwd,                          // MMX|SSE2
    kIdPmaxsb,                           // SSE4.1
    kIdPmaxsd,                           // SSE4.1
    kIdPmaxsw,                           // MMX2
    kIdPmaxub,                           // MMX2
    kIdPmaxud,                           // SSE4.1
    kIdPmaxuw,                           // SSE4.1
    kIdPminsb,                           // SSE4.1
    kIdPminsd,                           // SSE4.1
    kIdPminsw,                           // MMX2
    kIdPminub,                           // MMX2
    kIdPminud,                           // SSE4.1
    kIdPminuw,                           // SSE4.1
    kIdPmovmskb,                         // MMX2
    kIdPmovsxbd,                         // SSE4.1
    kIdPmovsxbq,                         // SSE4.1
    kIdPmovsxbw,                         // SSE4.1
    kIdPmovsxdq,                         // SSE4.1
    kIdPmovsxwd,                         // SSE4.1
    kIdPmovsxwq,                         // SSE4.1
    kIdPmovzxbd,                         // SSE4.1
    kIdPmovzxbq,                         // SSE4.1
    kIdPmovzxbw,                         // SSE4.1
    kIdPmovzxdq,                         // SSE4.1
    kIdPmovzxwd,                         // SSE4.1
    kIdPmovzxwq,                         // SSE4.1
    kIdPmuldq,                           // SSE4.1
    kIdPmulhrsw,                         // SSSE3
    kIdPmulhrw,                          // 3DNOW
    kIdPmulhuw,                          // MMX2
    kIdPmulhw,                           // MMX|SSE2
    kIdPmulld,                           // SSE4.1
    kIdPmullw,                           // MMX|SSE2
    kIdPmuludq,                          // SSE2
    kIdPop,                              // ANY
    kIdPopa,                             // X86
    kIdPopad,                            // X86
    kIdPopcnt,                           // SSE4.2
    kIdPopf,                             // ANY
    kIdPopfd,                            // X86
    kIdPopfq,                            // X64
    kIdPor,                              // MMX|SSE2
    kIdPrefetch,                         // 3DNOW
    kIdPrefetchnta,                      // MMX2|SSE
    kIdPrefetcht0,                       // MMX2|SSE
    kIdPrefetcht1,                       // MMX2|SSE
    kIdPrefetcht2,                       // MMX2|SSE
    kIdPrefetchw,                        // PREFETCHW
    kIdPrefetchwt1,                      // PREFETCHWT1
    kIdPsadbw,                           // MMX2
    kIdPshufb,                           // SSSE3
    kIdPshufd,                           // SSE2
    kIdPshufhw,                          // SSE2
    kIdPshuflw,                          // SSE2
    kIdPshufw,                           // MMX2
    kIdPsignb,                           // SSSE3
    kIdPsignd,                           // SSSE3
    kIdPsignw,                           // SSSE3
    kIdPslld,                            // MMX|SSE2
    kIdPslldq,                           // SSE2
    kIdPsllq,                            // MMX|SSE2
    kIdPsllw,                            // MMX|SSE2
    kIdPsrad,                            // MMX|SSE2
    kIdPsraw,                            // MMX|SSE2
    kIdPsrld,                            // MMX|SSE2
    kIdPsrldq,                           // SSE2
    kIdPsrlq,                            // MMX|SSE2
    kIdPsrlw,                            // MMX|SSE2
    kIdPsubb,                            // MMX|SSE2
    kIdPsubd,                            // MMX|SSE2
    kIdPsubq,                            // SSE2
    kIdPsubsb,                           // MMX|SSE2
    kIdPsubsw,                           // MMX|SSE2
    kIdPsubusb,                          // MMX|SSE2
    kIdPsubusw,                          // MMX|SSE2
    kIdPsubw,                            // MMX|SSE2
    kIdPswapd,                           // 3DNOW2
    kIdPtest,                            // SSE4.1
    kIdPunpckhbw,                        // MMX|SSE2
    kIdPunpckhdq,                        // MMX|SSE2
    kIdPunpckhqdq,                       // SSE2
    kIdPunpckhwd,                        // MMX|SSE2
    kIdPunpcklbw,                        // MMX|SSE2
    kIdPunpckldq,                        // MMX|SSE2
    kIdPunpcklqdq,                       // SSE2
    kIdPunpcklwd,                        // MMX|SSE2
    kIdPush,                             // ANY
    kIdPusha,                            // X86
    kIdPushad,                           // X86
    kIdPushf,                            // ANY
    kIdPushfd,                           // X86
    kIdPushfq,                           // X64
    kIdPxor,                             // MMX|SSE2
    kIdRcl,                              // ANY
    kIdRcpps,                            // SSE
    kIdRcpss,                            // SSE
    kIdRcr,                              // ANY
    kIdRdfsbase,                         // FSGSBASE & X64
    kIdRdgsbase,                         // FSGSBASE & X64
    kIdRdrand,                           // RDRAND
    kIdRdseed,                           // RDSEED
    kIdRdtsc,                            // ANY
    kIdRdtscp,                           // ANY
    kIdRet,                              // ANY
    kIdRol,                              // ANY
    kIdRor,                              // ANY
    kIdRorx,                             // BMI2
    kIdRoundpd,                          // SSE4.1
    kIdRoundps,                          // SSE4.1
    kIdRoundsd,                          // SSE4.1
    kIdRoundss,                          // SSE4.1
    kIdRsqrtps,                          // SSE
    kIdRsqrtss,                          // SSE
    kIdSahf,                             // LAHF_SAHF
    kIdSal,                              // ANY
    kIdSar,                              // ANY
    kIdSarx,                             // BMI2
    kIdSbb,                              // ANY
    kIdScas,                             // ANY
    kIdSeta,                             // ANY (setcc)
    kIdSetae,                            // ANY (setcc)
    kIdSetb,                             // ANY (setcc)
    kIdSetbe,                            // ANY (setcc)
    kIdSetc,                             // ANY (setcc)
    kIdSete,                             // ANY (setcc)
    kIdSetg,                             // ANY (setcc)
    kIdSetge,                            // ANY (setcc)
    kIdSetl,                             // ANY (setcc)
    kIdSetle,                            // ANY (setcc)
    kIdSetna,                            // ANY (setcc)
    kIdSetnae,                           // ANY (setcc)
    kIdSetnb,                            // ANY (setcc)
    kIdSetnbe,                           // ANY (setcc)
    kIdSetnc,                            // ANY (setcc)
    kIdSetne,                            // ANY (setcc)
    kIdSetng,                            // ANY (setcc)
    kIdSetnge,                           // ANY (setcc)
    kIdSetnl,                            // ANY (setcc)
    kIdSetnle,                           // ANY (setcc)
    kIdSetno,                            // ANY (setcc)
    kIdSetnp,                            // ANY (setcc)
    kIdSetns,                            // ANY (setcc)
    kIdSetnz,                            // ANY (setcc)
    kIdSeto,                             // ANY (setcc)
    kIdSetp,                             // ANY (setcc)
    kIdSetpe,                            // ANY (setcc)
    kIdSetpo,                            // ANY (setcc)
    kIdSets,                             // ANY (setcc)
    kIdSetz,                             // ANY (setcc)
    kIdSfence,                           // MMX2|SSE
    kIdSha1msg1,                         // SHA
    kIdSha1msg2,                         // SHA
    kIdSha1nexte,                        // SHA
    kIdSha1rnds4,                        // SHA
    kIdSha256msg1,                       // SHA
    kIdSha256msg2,                       // SHA
    kIdSha256rnds2,                      // SHA
    kIdShl,                              // ANY
    kIdShld,                             // ANY
    kIdShlx,                             // BMI2
    kIdShr,                              // ANY
    kIdShrd,                             // ANY
    kIdShrx,                             // BMI2
    kIdShufpd,                           // SSE2
    kIdShufps,                           // SSE
    kIdSqrtpd,                           // SSE2
    kIdSqrtps,                           // SSE
    kIdSqrtsd,                           // SSE2
    kIdSqrtss,                           // SSE
    kIdStac,                             // SMAP
    kIdStc,                              // ANY
    kIdStd,                              // ANY
    kIdSti,                              // ANY
    kIdStmxcsr,                          // SSE
    kIdStos,                             // ANY
    kIdSub,                              // ANY
    kIdSubpd,                            // SSE2
    kIdSubps,                            // SSE
    kIdSubsd,                            // SSE2
    kIdSubss,                            // SSE
    kIdSwapgs,                           // X64
    kIdT1mskc,                           // TBM
    kIdTest,                             // ANY
    kIdTzcnt,                            // TZCNT
    kIdTzmsk,                            // TBM
    kIdUcomisd,                          // SSE2
    kIdUcomiss,                          // SSE
    kIdUd2,                              // ANY
    kIdUnpckhpd,                         // SSE2
    kIdUnpckhps,                         // SSE
    kIdUnpcklpd,                         // SSE2
    kIdUnpcklps,                         // SSE
    kIdVaddpd,                           // AVX|AVX512F-VL
    kIdVaddps,                           // AVX|AVX512F-VL
    kIdVaddsd,                           // AVX|AVX512F
    kIdVaddss,                           // AVX|AVX512F
    kIdVaddsubpd,                        // AVX
    kIdVaddsubps,                        // AVX
    kIdVaesdec,                          // AVX|AES
    kIdVaesdeclast,                      // AVX|AES
    kIdVaesenc,                          // AVX|AES
    kIdVaesenclast,                      // AVX|AES
    kIdVaesimc,                          // AVX|AES
    kIdVaeskeygenassist,                 // AVX|AES
    kIdValignd,                          // AVX512F-VL
    kIdValignq,                          // AVX512F-VL
    kIdVandnpd,                          // AVX|AVX512DQ-VL
    kIdVandnps,                          // AVX|AVX512DQ-VL
    kIdVandpd,                           // AVX|AVX512DQ-VL
    kIdVandps,                           // AVX|AVX512DQ-VL
    kIdVblendmb,                         // AVX512BW-VL
    kIdVblendmd,                         // AVX512F-VL
    kIdVblendmpd,                        // AVX512F-VL
    kIdVblendmps,                        // AVX512F-VL
    kIdVblendmq,                         // AVX512F-VL
    kIdVblendmw,                         // AVX512BW-VL
    kIdVblendpd,                         // AVX
    kIdVblendps,                         // AVX
    kIdVblendvpd,                        // AVX
    kIdVblendvps,                        // AVX
    kIdVbroadcastf128,                   // AVX
    kIdVbroadcastf32x2,                  // AVX512DQ-VL
    kIdVbroadcastf32x4,                  // AVX512F
    kIdVbroadcastf32x8,                  // AVX512DQ
    kIdVbroadcastf64x2,                  // AVX512DQ-VL
    kIdVbroadcastf64x4,                  // AVX512F
    kIdVbroadcasti128,                   // AVX2
    kIdVbroadcasti32x2,                  // AVX512DQ-VL
    kIdVbroadcasti32x4,                  // AVX512F-VL
    kIdVbroadcasti32x8,                  // AVX512DQ
    kIdVbroadcasti64x2,                  // AVX512DQ-VL
    kIdVbroadcasti64x4,                  // AVX512F
    kIdVbroadcastsd,                     // AVX|AVX2|AVX512F-VL
    kIdVbroadcastss,                     // AVX|AVX2|AVX512F-VL
    kIdVcmppd,                           // AVX|AVX512F-VL
    kIdVcmpps,                           // AVX|AVX512F-VL
    kIdVcmpsd,                           // AVX|AVX512F
    kIdVcmpss,                           // AVX|AVX512F
    kIdVcomisd,                          // AVX|AVX512F
    kIdVcomiss,                          // AVX|AVX512F
    kIdVcompresspd,                      // AVX512F-VL
    kIdVcompressps,                      // AVX512F-VL
    kIdVcvtdq2pd,                        // AVX|AVX512F-VL
    kIdVcvtdq2ps,                        // AVX|AVX512F-VL
    kIdVcvtpd2dq,                        // AVX|AVX512F-VL
    kIdVcvtpd2ps,                        // AVX
    kIdVcvtpd2qq,                        // AVX512DQ-VL
    kIdVcvtpd2udq,                       // AVX512F-VL
    kIdVcvtpd2uqq,                       // AVX512DQ-VL
    kIdVcvtph2ps,                        // F16C|AVX512F-VL
    kIdVcvtps2dq,                        // AVX|AVX512F-VL
    kIdVcvtps2pd,                        // AVX|AVX512F-VL
    kIdVcvtps2ph,                        // F16C|AVX512F-VL
    kIdVcvtps2qq,                        // AVX512DQ-VL
    kIdVcvtps2udq,                       // AVX512F-VL
    kIdVcvtps2uqq,                       // AVX512DQ-VL
    kIdVcvtqq2pd,                        // AVX512DQ-VL
    kIdVcvtqq2ps,                        // AVX512DQ-VL
    kIdVcvtsd2si,                        // AVX|AVX512F
    kIdVcvtsd2ss,                        // AVX|AVX512F
    kIdVcvtsd2usi,                       // AVX512F
    kIdVcvtsi2sd,                        // AVX|AVX512F
    kIdVcvtsi2ss,                        // AVX|AVX512F
    kIdVcvtss2sd,                        // AVX|AVX512F
    kIdVcvtss2si,                        // AVX|AVX512F
    kIdVcvtss2usi,                       // AVX512F
    kIdVcvttpd2dq,                       // AVX|AVX512F-VL
    kIdVcvttpd2qq,                       // AVX512F-VL
    kIdVcvttpd2udq,                      // AVX512F-VL
    kIdVcvttpd2uqq,                      // AVX512DQ-VL
    kIdVcvttps2dq,                       // AVX|AVX512F-VL
    kIdVcvttps2qq,                       // AVX512DQ-VL
    kIdVcvttps2udq,                      // AVX512F-VL
    kIdVcvttps2uqq,                      // AVX512DQ-VL
    kIdVcvttsd2si,                       // AVX|AVX512F
    kIdVcvttsd2usi,                      // AVX512F
    kIdVcvttss2si,                       // AVX|AVX512F
    kIdVcvttss2usi,                      // AVX512F
    kIdVcvtudq2pd,                       // AVX512F-VL
    kIdVcvtudq2ps,                       // AVX512F-VL
    kIdVcvtuqq2pd,                       // AVX512DQ-VL
    kIdVcvtuqq2ps,                       // AVX512DQ-VL
    kIdVcvtusi2sd,                       // AVX512F
    kIdVcvtusi2ss,                       // AVX512F
    kIdVdbpsadbw,                        // AVX512BW-VL
    kIdVdivpd,                           // AVX|AVX512F-VL
    kIdVdivps,                           // AVX|AVX512F-VL
    kIdVdivsd,                           // AVX|AVX512F
    kIdVdivss,                           // AVX|AVX512F
    kIdVdppd,                            // AVX
    kIdVdpps,                            // AVX
    kIdVexp2pd,                          // AVX512ER
    kIdVexp2ps,                          // AVX512ER
    kIdVexpandpd,                        // AVX512F-VL
    kIdVexpandps,                        // AVX512F-VL
    kIdVextractf128,                     // AVX
    kIdVextractf32x4,                    // AVX512F-VL
    kIdVextractf32x8,                    // AVX512DQ
    kIdVextractf64x2,                    // AVX512DQ-VL
    kIdVextractf64x4,                    // AVX512F
    kIdVextracti128,                     // AVX2
    kIdVextracti32x4,                    // AVX512F-VL
    kIdVextracti32x8,                    // AVX512DQ
    kIdVextracti64x2,                    // AVX512DQ-VL
    kIdVextracti64x4,                    // AVX512F
    kIdVextractps,                       // AVX|AVX512F
    kIdVfixupimmpd,                      // AVX512F-VL
    kIdVfixupimmps,                      // AVX512F-VL
    kIdVfixupimmsd,                      // AVX512F
    kIdVfixupimmss,                      // AVX512F
    kIdVfmadd132pd,                      // FMA3|AVX512F-VL
    kIdVfmadd132ps,                      // FMA3|AVX512F-VL
    kIdVfmadd132sd,                      // FMA3|AVX512F
    kIdVfmadd132ss,                      // FMA3|AVX512F
    kIdVfmadd213pd,                      // FMA3|AVX512F-VL
    kIdVfmadd213ps,                      // FMA3|AVX512F-VL
    kIdVfmadd213sd,                      // FMA3|AVX512F
    kIdVfmadd213ss,                      // FMA3|AVX512F
    kIdVfmadd231pd,                      // FMA3|AVX512F-VL
    kIdVfmadd231ps,                      // FMA3|AVX512F-VL
    kIdVfmadd231sd,                      // FMA3|AVX512F
    kIdVfmadd231ss,                      // FMA3|AVX512F
    kIdVfmaddpd,                         // FMA4
    kIdVfmaddps,                         // FMA4
    kIdVfmaddsd,                         // FMA4
    kIdVfmaddss,                         // FMA4
    kIdVfmaddsub132pd,                   // FMA3|AVX512F-VL
    kIdVfmaddsub132ps,                   // FMA3|AVX512F-VL
    kIdVfmaddsub213pd,                   // FMA3|AVX512F-VL
    kIdVfmaddsub213ps,                   // FMA3|AVX512F-VL
    kIdVfmaddsub231pd,                   // FMA3|AVX512F-VL
    kIdVfmaddsub231ps,                   // FMA3|AVX512F-VL
    kIdVfmaddsubpd,                      // FMA4
    kIdVfmaddsubps,                      // FMA4
    kIdVfmsub132pd,                      // FMA3|AVX512F-VL
    kIdVfmsub132ps,                      // FMA3|AVX512F-VL
    kIdVfmsub132sd,                      // FMA3|AVX512F
    kIdVfmsub132ss,                      // FMA3|AVX512F
    kIdVfmsub213pd,                      // FMA3|AVX512F-VL
    kIdVfmsub213ps,                      // FMA3|AVX512F-VL
    kIdVfmsub213sd,                      // FMA3|AVX512F
    kIdVfmsub213ss,                      // FMA3|AVX512F
    kIdVfmsub231pd,                      // FMA3|AVX512F-VL
    kIdVfmsub231ps,                      // FMA3|AVX512F-VL
    kIdVfmsub231sd,                      // FMA3|AVX512F
    kIdVfmsub231ss,                      // FMA3|AVX512F
    kIdVfmsubadd132pd,                   // FMA3|AVX512F-VL
    kIdVfmsubadd132ps,                   // FMA3|AVX512F-VL
    kIdVfmsubadd213pd,                   // FMA3|AVX512F-VL
    kIdVfmsubadd213ps,                   // FMA3|AVX512F-VL
    kIdVfmsubadd231pd,                   // FMA3|AVX512F-VL
    kIdVfmsubadd231ps,                   // FMA3|AVX512F-VL
    kIdVfmsubaddpd,                      // FMA4
    kIdVfmsubaddps,                      // FMA4
    kIdVfmsubpd,                         // FMA4
    kIdVfmsubps,                         // FMA4
    kIdVfmsubsd,                         // FMA4
    kIdVfmsubss,                         // FMA4
    kIdVfnmadd132pd,                     // FMA3|AVX512F-VL
    kIdVfnmadd132ps,                     // FMA3|AVX512F-VL
    kIdVfnmadd132sd,                     // FMA3|AVX512F
    kIdVfnmadd132ss,                     // FMA3|AVX512F
    kIdVfnmadd213pd,                     // FMA3|AVX512F-VL
    kIdVfnmadd213ps,                     // FMA3|AVX512F-VL
    kIdVfnmadd213sd,                     // FMA3|AVX512F
    kIdVfnmadd213ss,                     // FMA3|AVX512F
    kIdVfnmadd231pd,                     // FMA3|AVX512F-VL
    kIdVfnmadd231ps,                     // FMA3|AVX512F-VL
    kIdVfnmadd231sd,                     // FMA3|AVX512F
    kIdVfnmadd231ss,                     // FMA3|AVX512F
    kIdVfnmaddpd,                        // FMA4
    kIdVfnmaddps,                        // FMA4
    kIdVfnmaddsd,                        // FMA4
    kIdVfnmaddss,                        // FMA4
    kIdVfnmsub132pd,                     // FMA3|AVX512F-VL
    kIdVfnmsub132ps,                     // FMA3|AVX512F-VL
    kIdVfnmsub132sd,                     // FMA3|AVX512F
    kIdVfnmsub132ss,                     // FMA3|AVX512F
    kIdVfnmsub213pd,                     // FMA3|AVX512F-VL
    kIdVfnmsub213ps,                     // FMA3|AVX512F-VL
    kIdVfnmsub213sd,                     // FMA3|AVX512F
    kIdVfnmsub213ss,                     // FMA3|AVX512F
    kIdVfnmsub231pd,                     // FMA3|AVX512F-VL
    kIdVfnmsub231ps,                     // FMA3|AVX512F-VL
    kIdVfnmsub231sd,                     // FMA3|AVX512F
    kIdVfnmsub231ss,                     // FMA3|AVX512F
    kIdVfnmsubpd,                        // FMA4
    kIdVfnmsubps,                        // FMA4
    kIdVfnmsubsd,                        // FMA4
    kIdVfnmsubss,                        // FMA4
    kIdVfpclasspd,                       // AVX512DQ-VL
    kIdVfpclassps,                       // AVX512DQ-VL
    kIdVfpclasssd,                       // AVX512DQ
    kIdVfpclassss,                       // AVX512DQ
    kIdVfrczpd,                          // XOP
    kIdVfrczps,                          // XOP
    kIdVfrczsd,                          // XOP
    kIdVfrczss,                          // XOP
    kIdVgatherdpd,                       // AVX2|AVX512F-VL
    kIdVgatherdps,                       // AVX2|AVX512F-VL
    kIdVgatherpf0dpd,                    // AVX512PF
    kIdVgatherpf0dps,                    // AVX512PF
    kIdVgatherpf0qpd,                    // AVX512PF
    kIdVgatherpf0qps,                    // AVX512PF
    kIdVgatherpf1dpd,                    // AVX512PF
    kIdVgatherpf1dps,                    // AVX512PF
    kIdVgatherpf1qpd,                    // AVX512PF
    kIdVgatherpf1qps,                    // AVX512PF
    kIdVgatherqpd,                       // AVX2|AVX512F-VL
    kIdVgatherqps,                       // AVX2|AVX512F-VL
    kIdVgetexppd,                        // AVX512F-VL
    kIdVgetexpps,                        // AVX512F-VL
    kIdVgetexpsd,                        // AVX512F
    kIdVgetexpss,                        // AVX512F
    kIdVgetmantpd,                       // AVX512F-VL
    kIdVgetmantps,                       // AVX512F-VL
    kIdVgetmantsd,                       // AVX512F
    kIdVgetmantss,                       // AVX512F
    kIdVhaddpd,                          // AVX
    kIdVhaddps,                          // AVX
    kIdVhsubpd,                          // AVX
    kIdVhsubps,                          // AVX
    kIdVinsertf128,                      // AVX
    kIdVinsertf32x4,                     // AVX512F-VL
    kIdVinsertf32x8,                     // AVX512DQ
    kIdVinsertf64x2,                     // AVX512DQ-VL
    kIdVinsertf64x4,                     // AVX512F
    kIdVinserti128,                      // AVX2
    kIdVinserti32x4,                     // AVX512F-VL
    kIdVinserti32x8,                     // AVX512DQ
    kIdVinserti64x2,                     // AVX512DQ-VL
    kIdVinserti64x4,                     // AVX512F
    kIdVinsertps,                        // AVX|AVX512F
    kIdVlddqu,                           // AVX
    kIdVldmxcsr,                         // AVX
    kIdVmaskmovdqu,                      // AVX
    kIdVmaskmovpd,                       // AVX
    kIdVmaskmovps,                       // AVX
    kIdVmaxpd,                           // AVX|AVX512F-VL
    kIdVmaxps,                           // AVX|AVX512F-VL
    kIdVmaxsd,                           // AVX|AVX512F-VL
    kIdVmaxss,                           // AVX|AVX512F-VL
    kIdVminpd,                           // AVX|AVX512F-VL
    kIdVminps,                           // AVX|AVX512F-VL
    kIdVminsd,                           // AVX|AVX512F-VL
    kIdVminss,                           // AVX|AVX512F-VL
    kIdVmovapd,                          // AVX|AVX512F-VL
    kIdVmovaps,                          // AVX|AVX512F-VL
    kIdVmovd,                            // AVX|AVX512F
    kIdVmovddup,                         // AVX|AVX512F-VL
    kIdVmovdqa,                          // AVX
    kIdVmovdqa32,                        // AVX512F-VL
    kIdVmovdqa64,                        // AVX512F-VL
    kIdVmovdqu,                          // AVX
    kIdVmovdqu16,                        // AVX512BW-VL
    kIdVmovdqu32,                        // AVX512F-VL
    kIdVmovdqu64,                        // AVX512F-VL
    kIdVmovdqu8,                         // AVX512BW-VL
    kIdVmovhlps,                         // AVX|AVX512F
    kIdVmovhpd,                          // AVX|AVX512F
    kIdVmovhps,                          // AVX|AVX512F
    kIdVmovlhps,                         // AVX|AVX512F
    kIdVmovlpd,                          // AVX|AVX512F
    kIdVmovlps,                          // AVX|AVX512F
    kIdVmovmskpd,                        // AVX
    kIdVmovmskps,                        // AVX
    kIdVmovntdq,                         // AVX|AVX512F-VL
    kIdVmovntdqa,                        // AVX|AVX2|AVX512F-VL
    kIdVmovntpd,                         // AVX|AVX512F-VL
    kIdVmovntps,                         // AVX|AVX512F-VL
    kIdVmovq,                            // AVX|AVX512F
    kIdVmovsd,                           // AVX|AVX512F
    kIdVmovshdup,                        // AVX|AVX512F-VL
    kIdVmovsldup,                        // AVX|AVX512F-VL
    kIdVmovss,                           // AVX|AVX512F
    kIdVmovupd,                          // AVX|AVX512F-VL
    kIdVmovups,                          // AVX|AVX512F-VL
    kIdVmpsadbw,                         // AVX|AVX2
    kIdVmulpd,                           // AVX|AVX2|AVX512F-VL
    kIdVmulps,                           // AVX|AVX2|AVX512F-VL
    kIdVmulsd,                           // AVX|AVX512F
    kIdVmulss,                           // AVX|AVX512F
    kIdVorpd,                            // AVX|AVX512DQ-VL
    kIdVorps,                            // AVX|AVX512F-VL
    kIdVpabsb,                           // AVX|AVX2|AVX512BW-VL
    kIdVpabsd,                           // AVX|AVX2|AVX512F-VL
    kIdVpabsq,                           // AVX512F-VL
    kIdVpabsw,                           // AVX|AVX2|AVX512BW-VL
    kIdVpackssdw,                        // AVX|AVX2|AVX512BW-VL
    kIdVpacksswb,                        // AVX|AVX2|AVX512BW-VL
    kIdVpackusdw,                        // AVX|AVX2|AVX512BW-VL
    kIdVpackuswb,                        // AVX|AVX2|AVX512BW-VL
    kIdVpaddb,                           // AVX|AVX2|AVX512BW-VL
    kIdVpaddd,                           // AVX|AVX2|AVX512F-VL
    kIdVpaddq,                           // AVX|AVX2|AVX512F-VL
    kIdVpaddsb,                          // AVX|AVX2|AVX512BW-VL
    kIdVpaddsw,                          // AVX|AVX2|AVX512BW-VL
    kIdVpaddusb,                         // AVX|AVX2|AVX512BW-VL
    kIdVpaddusw,                         // AVX|AVX2|AVX512BW-VL
    kIdVpaddw,                           // AVX|AVX2|AVX512BW-VL
    kIdVpalignr,                         // AVX|AVX2|AVX512BW-VL
    kIdVpand,                            // AVX|AVX2
    kIdVpandd,                           // AVX512F-VL
    kIdVpandn,                           // AVX|AVX2
    kIdVpandnd,                          // AVX512F-VL
    kIdVpandnq,                          // AVX512F-VL
    kIdVpandq,                           // AVX512F-VL
    kIdVpavgb,                           // AVX|AVX512BW-VL
    kIdVpavgw,                           // AVX2|AVX512BW-VL
    kIdVpblendd,                         // AVX2
    kIdVpblendvb,                        // AVX|AVX2
    kIdVpblendw,                         // AVX|AVX2
    kIdVpbroadcastb,                     // AVX2|AVX512BW-VL
    kIdVpbroadcastd,                     // AVX2|AVX512F-VL
    kIdVpbroadcastmb2d,                  // AVX512CD-VL
    kIdVpbroadcastmb2q,                  // AVX512CD-VL
    kIdVpbroadcastq,                     // AVX2|AVX512F-VL
    kIdVpbroadcastw,                     // AVX2|AVX512BW-VL
    kIdVpclmulqdq,                       // AVX|PCLMULQDQ
    kIdVpcmov,                           // XOP
    kIdVpcmpb,                           // AVX512BW-VL
    kIdVpcmpd,                           // AVX512F-VL
    kIdVpcmpeqb,                         // AVX|AVX2|AVX512BW-VL
    kIdVpcmpeqd,                         // AVX|AVX2|AVX512F-VL
    kIdVpcmpeqq,                         // AVX|AVX2|AVX512F-VL
    kIdVpcmpeqw,                         // AVX|AVX2|AVX512BW-VL
    kIdVpcmpestri,                       // AVX
    kIdVpcmpestrm,                       // AVX
    kIdVpcmpgtb,                         // AVX|AVX2|AVX512BW-VL
    kIdVpcmpgtd,                         // AVX|AVX2|AVX512F-VL
    kIdVpcmpgtq,                         // AVX|AVX2|AVX512F-VL
    kIdVpcmpgtw,                         // AVX|AVX2|AVX512BW-VL
    kIdVpcmpistri,                       // AVX
    kIdVpcmpistrm,                       // AVX
    kIdVpcmpq,                           // AVX512F-VL
    kIdVpcmpub,                          // AVX512BW-VL
    kIdVpcmpud,                          // AVX512F-VL
    kIdVpcmpuq,                          // AVX512F-VL
    kIdVpcmpuw,                          // AVX512BW-VL
    kIdVpcmpw,                           // AVX512BW-VL
    kIdVpcomb,                           // XOP
    kIdVpcomd,                           // XOP
    kIdVpcompressd,                      // AVX512F-VL
    kIdVpcompressq,                      // AVX512F-VL
    kIdVpcomq,                           // XOP
    kIdVpcomub,                          // XOP
    kIdVpcomud,                          // XOP
    kIdVpcomuq,                          // XOP
    kIdVpcomuw,                          // XOP
    kIdVpcomw,                           // XOP
    kIdVpconflictd,                      // AVX512CD-VL
    kIdVpconflictq,                      // AVX512CD-VL
    kIdVperm2f128,                       // AVX
    kIdVperm2i128,                       // AVX2
    kIdVpermb,                           // AVX512VBMI-VL
    kIdVpermd,                           // AVX2|AVX512F-VL
    kIdVpermi2b,                         // AVX512VBMI-VL
    kIdVpermi2d,                         // AVX512F-VL
    kIdVpermi2pd,                        // AVX512F-VL
    kIdVpermi2ps,                        // AVX512F-VL
    kIdVpermi2q,                         // AVX512F-VL
    kIdVpermi2w,                         // AVX512BW-VL
    kIdVpermil2pd,                       // XOP
    kIdVpermil2ps,                       // XOP
    kIdVpermilpd,                        // AVX|AVX512F-VL
    kIdVpermilps,                        // AVX|AVX512F-VL
    kIdVpermpd,                          // AVX2
    kIdVpermps,                          // AVX2
    kIdVpermq,                           // AVX2|AVX512F-VL
    kIdVpermt2b,                         // AVX512VBMI-VL
    kIdVpermt2d,                         // AVX512F-VL
    kIdVpermt2pd,                        // AVX512F-VL
    kIdVpermt2ps,                        // AVX512F-VL
    kIdVpermt2q,                         // AVX512F-VL
    kIdVpermt2w,                         // AVX512BW-VL
    kIdVpermw,                           // AVX512BW-VL
    kIdVpexpandd,                        // AVX512F-VL
    kIdVpexpandq,                        // AVX512F-VL
    kIdVpextrb,                          // AVX|AVX512BW
    kIdVpextrd,                          // AVX|AVX512DQ
    kIdVpextrq,                          // AVX|AVX512DQ
    kIdVpextrw,                          // AVX|AVX512BW
    kIdVpgatherdd,                       // AVX2|AVX512F-VL
    kIdVpgatherdq,                       // AVX2|AVX512F-VL
    kIdVpgatherqd,                       // AVX2|AVX512F-VL
    kIdVpgatherqq,                       // AVX2|AVX512F-VL
    kIdVphaddbd,                         // XOP
    kIdVphaddbq,                         // XOP
    kIdVphaddbw,                         // XOP
    kIdVphaddd,                          // AVX|AVX2
    kIdVphadddq,                         // XOP
    kIdVphaddsw,                         // AVX|AVX2
    kIdVphaddubd,                        // XOP
    kIdVphaddubq,                        // XOP
    kIdVphaddubw,                        // XOP
    kIdVphaddudq,                        // XOP
    kIdVphadduwd,                        // XOP
    kIdVphadduwq,                        // XOP
    kIdVphaddw,                          // AVX|AVX2
    kIdVphaddwd,                         // XOP
    kIdVphaddwq,                         // XOP
    kIdVphminposuw,                      // AVX
    kIdVphsubbw,                         // XOP
    kIdVphsubd,                          // AVX|AVX2
    kIdVphsubdq,                         // XOP
    kIdVphsubsw,                         // AVX|AVX2
    kIdVphsubw,                          // AVX|AVX2
    kIdVphsubwd,                         // XOP
    kIdVpinsrb,                          // AVX|AVX512BW
    kIdVpinsrd,                          // AVX|AVX512DQ
    kIdVpinsrq,                          // AVX|AVX512DQ
    kIdVpinsrw,                          // AVX|AVX512BW
    kIdVplzcntd,                         // AVX512CD-VL
    kIdVplzcntq,                         // AVX512CD-VL
    kIdVpmacsdd,                         // XOP
    kIdVpmacsdqh,                        // XOP
    kIdVpmacsdql,                        // XOP
    kIdVpmacssdd,                        // XOP
    kIdVpmacssdqh,                       // XOP
    kIdVpmacssdql,                       // XOP
    kIdVpmacsswd,                        // XOP
    kIdVpmacssww,                        // XOP
    kIdVpmacswd,                         // XOP
    kIdVpmacsww,                         // XOP
    kIdVpmadcsswd,                       // XOP
    kIdVpmadcswd,                        // XOP
    kIdVpmadd52huq,                      // AVX512IFMA-VL
    kIdVpmadd52luq,                      // AVX512IFMA-VL
    kIdVpmaddubsw,                       // AVX|AVX2|AVX512BW-VL
    kIdVpmaddwd,                         // AVX|AVX2|AVX512BW-VL
    kIdVpmaskmovd,                       // AVX2
    kIdVpmaskmovq,                       // AVX2
    kIdVpmaxsb,                          // AVX|AVX2|AVX512BW-VL
    kIdVpmaxsd,                          // AVX|AVX2|AVX512F-VL
    kIdVpmaxsq,                          // AVX512F-VL
    kIdVpmaxsw,                          // AVX|AVX2|AVX512BW-VL
    kIdVpmaxub,                          // AVX|AVX2|AVX512BW-VL
    kIdVpmaxud,                          // AVX|AVX2|AVX512F-VL
    kIdVpmaxuq,                          // AVX512F-VL
    kIdVpmaxuw,                          // AVX|AVX2|AVX512BW-VL
    kIdVpminsb,                          // AVX|AVX2|AVX512BW-VL
    kIdVpminsd,                          // AVX|AVX2|AVX512F-VL
    kIdVpminsq,                          // AVX512F-VL
    kIdVpminsw,                          // AVX|AVX2|AVX512BW-VL
    kIdVpminub,                          // AVX|AVX2|AVX512BW-VL
    kIdVpminud,                          // AVX|AVX2|AVX512F-VL
    kIdVpminuq,                          // AVX512F-VL
    kIdVpminuw,                          // AVX|AVX2|AVX512BW-VL
    kIdVpmovb2m,                         // AVX512BW-VL
    kIdVpmovd2m,                         // AVX512DQ-VL
    kIdVpmovdb,                          // AVX512F-VL
    kIdVpmovdw,                          // AVX512F-VL
    kIdVpmovm2b,                         // AVX512BW-VL
    kIdVpmovm2d,                         // AVX512DQ-VL
    kIdVpmovm2q,                         // AVX512DQ-VL
    kIdVpmovm2w,                         // AVX512BW-VL
    kIdVpmovmskb,                        // AVX|AVX2
    kIdVpmovq2m,                         // AVX512DQ-VL
    kIdVpmovqb,                          // AVX512F-VL
    kIdVpmovqd,                          // AVX512F-VL
    kIdVpmovqw,                          // AVX512F-VL
    kIdVpmovsdb,                         // AVX512F-VL
    kIdVpmovsdw,                         // AVX512F-VL
    kIdVpmovsqb,                         // AVX512F-VL
    kIdVpmovsqd,                         // AVX512F-VL
    kIdVpmovsqw,                         // AVX512F-VL
    kIdVpmovswb,                         // AVX512BW-VL
    kIdVpmovsxbd,                        // AVX|AVX2|AVX512F-VL
    kIdVpmovsxbq,                        // AVX|AVX2|AVX512F-VL
    kIdVpmovsxbw,                        // AVX|AVX2|AVX512BW-VL
    kIdVpmovsxdq,                        // AVX|AVX2|AVX512F-VL
    kIdVpmovsxwd,                        // AVX|AVX2|AVX512F-VL
    kIdVpmovsxwq,                        // AVX|AVX2|AVX512F-VL
    kIdVpmovusdb,                        // AVX512F-VL
    kIdVpmovusdw,                        // AVX512F-VL
    kIdVpmovusqb,                        // AVX512F-VL
    kIdVpmovusqd,                        // AVX512F-VL
    kIdVpmovusqw,                        // AVX512F-VL
    kIdVpmovuswb,                        // AVX512BW-VL
    kIdVpmovw2m,                         // AVX512BW-VL
    kIdVpmovwb,                          // AVX512BW-VL
    kIdVpmovzxbd,                        // AVX|AVX2|AVX512F-VL
    kIdVpmovzxbq,                        // AVX|AVX2|AVX512F-VL
    kIdVpmovzxbw,                        // AVX|AVX2|AVX512BW-VL
    kIdVpmovzxdq,                        // AVX|AVX2|AVX512F-VL
    kIdVpmovzxwd,                        // AVX|AVX2|AVX512F-VL
    kIdVpmovzxwq,                        // AVX|AVX2|AVX512F-VL
    kIdVpmuldq,                          // AVX|AVX2|AVX512F-VL
    kIdVpmulhrsw,                        // AVX|AVX2|AVX512BW-VL
    kIdVpmulhuw,                         // AVX|AVX2|AVX512BW-VL
    kIdVpmulhw,                          // AVX|AVX2|AVX512BW-VL
    kIdVpmulld,                          // AVX|AVX2|AVX512F-VL
    kIdVpmullq,                          // AVX512DQ-VL
    kIdVpmullw,                          // AVX|AVX2|AVX512BW-VL
    kIdVpmultishiftqb,                   // AVX512VBMI-VL
    kIdVpmuludq,                         // AVX|AVX2|AVX512F-VL
    kIdVpor,                             // AVX|AVX2
    kIdVpord,                            // AVX512F-VL
    kIdVporq,                            // AVX512F-VL
    kIdVpperm,                           // XOP
    kIdVprold,                           // AVX512F-VL
    kIdVprolq,                           // AVX512F-VL
    kIdVprolvd,                          // AVX512F-VL
    kIdVprolvq,                          // AVX512F-VL
    kIdVprord,                           // AVX512F-VL
    kIdVprorq,                           // AVX512F-VL
    kIdVprorvd,                          // AVX512F-VL
    kIdVprorvq,                          // AVX512F-VL
    kIdVprotb,                           // XOP
    kIdVprotd,                           // XOP
    kIdVprotq,                           // XOP
    kIdVprotw,                           // XOP
    kIdVpsadbw,                          // AVX|AVX2|AVX512BW-VL
    kIdVpscatterdd,                      // AVX512F-VL
    kIdVpscatterdq,                      // AVX512F-VL
    kIdVpscatterqd,                      // AVX512F-VL
    kIdVpscatterqq,                      // AVX512F-VL
    kIdVpshab,                           // XOP
    kIdVpshad,                           // XOP
    kIdVpshaq,                           // XOP
    kIdVpshaw,                           // XOP
    kIdVpshlb,                           // XOP
    kIdVpshld,                           // XOP
    kIdVpshlq,                           // XOP
    kIdVpshlw,                           // XOP
    kIdVpshufb,                          // AVX|AVX2|AVX512BW-VL
    kIdVpshufd,                          // AVX|AVX2|AVX512F-VL
    kIdVpshufhw,                         // AVX|AVX2|AVX512BW-VL
    kIdVpshuflw,                         // AVX|AVX2|AVX512BW-VL
    kIdVpsignb,                          // AVX|AVX2
    kIdVpsignd,                          // AVX|AVX2
    kIdVpsignw,                          // AVX|AVX2
    kIdVpslld,                           // AVX|AVX2|AVX512F-VL
    kIdVpslldq,                          // AVX|AVX2|AVX512BW-VL
    kIdVpsllq,                           // AVX|AVX2|AVX512F-VL
    kIdVpsllvd,                          // AVX2|AVX512F-VL
    kIdVpsllvq,                          // AVX2|AVX512F-VL
    kIdVpsllvw,                          // AVX512BW-VL
    kIdVpsllw,                           // AVX|AVX2|AVX512BW-VL
    kIdVpsrad,                           // AVX|AVX2|AVX512F-VL
    kIdVpsraq,                           // AVX512F-VL
    kIdVpsravd,                          // AVX2|AVX512F-VL
    kIdVpsravq,                          // AVX512F-VL
    kIdVpsravw,                          // AVX512BW-VL
    kIdVpsraw,                           // AVX|AVX2|AVX512BW-VL
    kIdVpsrld,                           // AVX|AVX2|AVX512F-VL
    kIdVpsrldq,                          // AVX|AVX2|AVX512BW-VL
    kIdVpsrlq,                           // AVX|AVX2|AVX512F-VL
    kIdVpsrlvd,                          // AVX2|AVX512F-VL
    kIdVpsrlvq,                          // AVX2|AVX512F-VL
    kIdVpsrlvw,                          // AVX512BW-VL
    kIdVpsrlw,                           // AVX|AVX2|AVX512BW-VL
    kIdVpsubb,                           // AVX|AVX2|AVX512BW-VL
    kIdVpsubd,                           // AVX|AVX2|AVX512F-VL
    kIdVpsubq,                           // AVX|AVX2|AVX512F-VL
    kIdVpsubsb,                          // AVX|AVX2|AVX512BW-VL
    kIdVpsubsw,                          // AVX|AVX2|AVX512BW-VL
    kIdVpsubusb,                         // AVX|AVX2|AVX512BW-VL
    kIdVpsubusw,                         // AVX|AVX2|AVX512BW-VL
    kIdVpsubw,                           // AVX|AVX2|AVX512BW-VL
    kIdVpternlogd,                       // AVX512F-VL
    kIdVpternlogq,                       // AVX512F-VL
    kIdVptest,                           // AVX
    kIdVptestmb,                         // AVX512BW-VL
    kIdVptestmd,                         // AVX512F-VL
    kIdVptestmq,                         // AVX512F-VL
    kIdVptestmw,                         // AVX512BW-VL
    kIdVptestnmb,                        // AVX512BW-VL
    kIdVptestnmd,                        // AVX512F-VL
    kIdVptestnmq,                        // AVX512F-VL
    kIdVptestnmw,                        // AVX512BW-VL
    kIdVpunpckhbw,                       // AVX|AVX2|AVX512BW-VL
    kIdVpunpckhdq,                       // AVX|AVX2|AVX512F-VL
    kIdVpunpckhqdq,                      // AVX|AVX2|AVX512F-VL
    kIdVpunpckhwd,                       // AVX|AVX2|AVX512BW-VL
    kIdVpunpcklbw,                       // AVX|AVX2|AVX512BW-VL
    kIdVpunpckldq,                       // AVX|AVX2|AVX512F-VL
    kIdVpunpcklqdq,                      // AVX|AVX2|AVX512F-VL
    kIdVpunpcklwd,                       // AVX|AVX2|AVX512BW-VL
    kIdVpxor,                            // AVX|AVX2
    kIdVpxord,                           // AVX512F-VL
    kIdVpxorq,                           // AVX512F-VL
    kIdVrangepd,                         // AVX512DQ-VL
    kIdVrangeps,                         // AVX512DQ-VL
    kIdVrangesd,                         // AVX512DQ
    kIdVrangess,                         // AVX512DQ
    kIdVrcp14pd,                         // AVX512F-VL
    kIdVrcp14ps,                         // AVX512F-VL
    kIdVrcp14sd,                         // AVX512F
    kIdVrcp14ss,                         // AVX512F
    kIdVrcp28pd,                         // AVX512ER
    kIdVrcp28ps,                         // AVX512ER
    kIdVrcp28sd,                         // AVX512ER
    kIdVrcp28ss,                         // AVX512ER
    kIdVrcpps,                           // AVX
    kIdVrcpss,                           // AVX
    kIdVreducepd,                        // AVX512DQ-VL
    kIdVreduceps,                        // AVX512DQ-VL
    kIdVreducesd,                        // AVX512DQ
    kIdVreducess,                        // AVX512DQ
    kIdVrndscalepd,                      // AVX512F-VL
    kIdVrndscaleps,                      // AVX512F-VL
    kIdVrndscalesd,                      // AVX512F
    kIdVrndscaless,                      // AVX512F
    kIdVroundpd,                         // AVX
    kIdVroundps,                         // AVX
    kIdVroundsd,                         // AVX
    kIdVroundss,                         // AVX
    kIdVrsqrt14pd,                       // AVX512F-VL
    kIdVrsqrt14ps,                       // AVX512F-VL
    kIdVrsqrt14sd,                       // AVX512F
    kIdVrsqrt14ss,                       // AVX512F
    kIdVrsqrt28pd,                       // AVX512ER
    kIdVrsqrt28ps,                       // AVX512ER
    kIdVrsqrt28sd,                       // AVX512ER
    kIdVrsqrt28ss,                       // AVX512ER
    kIdVrsqrtps,                         // AVX
    kIdVrsqrtss,                         // AVX
    kIdVscalefpd,                        // AVX512F-VL
    kIdVscalefps,                        // AVX512F-VL
    kIdVscalefsd,                        // AVX512F
    kIdVscalefss,                        // AVX512F
    kIdVscatterdpd,                      // AVX512F-VL
    kIdVscatterdps,                      // AVX512F-VL
    kIdVscatterpf0dpd,                   // AVX512PF
    kIdVscatterpf0dps,                   // AVX512PF
    kIdVscatterpf0qpd,                   // AVX512PF
    kIdVscatterpf0qps,                   // AVX512PF
    kIdVscatterpf1dpd,                   // AVX512PF
    kIdVscatterpf1dps,                   // AVX512PF
    kIdVscatterpf1qpd,                   // AVX512PF
    kIdVscatterpf1qps,                   // AVX512PF
    kIdVscatterqpd,                      // AVX512F-VL
    kIdVscatterqps,                      // AVX512F-VL
    kIdVshuff32x4,                       // AVX512F-VL
    kIdVshuff64x2,                       // AVX512F-VL
    kIdVshufi32x4,                       // AVX512F-VL
    kIdVshufi64x2,                       // AVX512F-VL
    kIdVshufpd,                          // AVX|AVX512F-VL
    kIdVshufps,                          // AVX|AVX512F-VL
    kIdVsqrtpd,                          // AVX|AVX512F-VL
    kIdVsqrtps,                          // AVX|AVX512F-VL
    kIdVsqrtsd,                          // AVX|AVX512F
    kIdVsqrtss,                          // AVX|AVX512F
    kIdVstmxcsr,                         // AVX
    kIdVsubpd,                           // AVX|AVX2|AVX512F-VL
    kIdVsubps,                           // AVX|AVX2|AVX512F-VL
    kIdVsubsd,                           // AVX|AVX512F
    kIdVsubss,                           // AVX|AVX512F
    kIdVtestpd,                          // AVX
    kIdVtestps,                          // AVX
    kIdVucomisd,                         // AVX|AVX512F
    kIdVucomiss,                         // AVX|AVX512F
    kIdVunpckhpd,                        // AVX|AVX512F-VL
    kIdVunpckhps,                        // AVX|AVX512F-VL
    kIdVunpcklpd,                        // AVX|AVX512F-VL
    kIdVunpcklps,                        // AVX|AVX512F-VL
    kIdVxorpd,                           // AVX|AVX512DQ-VL
    kIdVxorps,                           // AVX|AVX512DQ-VL
    kIdVzeroall,                         // AVX
    kIdVzeroupper,                       // AVX
    kIdWrfsbase,                         // FSGSBASE & X64
    kIdWrgsbase,                         // FSGSBASE & X64
    kIdXadd,                             // I486+
    kIdXchg,                             // ANY
    kIdXgetbv,                           // XSAVE
    kIdXor,                              // ANY
    kIdXorpd,                            // SSE2
    kIdXorps,                            // SSE
    kIdXrstor,                           // XSAVE
    kIdXrstor64,                         // XSAVE
    kIdXrstors,                          // XSAVE
    kIdXrstors64,                        // XSAVE
    kIdXsave,                            // XSAVE
    kIdXsave64,                          // XSAVE
    kIdXsavec,                           // XSAVE
    kIdXsavec64,                         // XSAVE
    kIdXsaveopt,                         // XSAVE
    kIdXsaveopt64,                       // XSAVE
    kIdXsaves,                           // XSAVE
    kIdXsaves64,                         // XSAVE
    kIdXsetbv,                           // XSAVE

    _kIdCount,

    _kIdCmovcc = kIdCmova,
    _kIdJcc    = kIdJa,
    _kIdSetcc  = kIdSeta,

    _kIdJbegin = kIdJa,
    _kIdJend   = kIdJmp
  };

  //! Instruction encodings, used by \ref X86Assembler.
  ASMJIT_ENUM(EncodingType) {
    kEncodingNone = 0,                   //!< Never used.
    kEncodingX86Op,                      //!< X86 [OP].
    kEncodingX86Op_O,                    //!< X86 [OP] (opcode and /0-7).
    kEncodingX86Op_xAX,                  //!< X86 [OP] (implicit or explicit '?AX' form).
    kEncodingX86Op_xDX_xAX,              //!< X86 [OP] (implicit or explicit '?DX, ?AX' form).
    kEncodingX86Op_ZAX,                  //!< X86 [OP] (implicit or explicit '[EAX|RDX]' form).
    kEncodingX86I_xAX,                   //!< X86 [I] (implicit or explicit '?AX' form).
    kEncodingX86M,                       //!< X86 [M] (handles 2|4|8-bytes size).
    kEncodingX86M_GPB,                   //!< X86 [M] (handles single-byte size).
    kEncodingX86M_GPB_MulDiv,            //!< X86 [M] (like GPB, handles implicit|explicit MUL|DIV|IDIV).
    kEncodingX86M_Only,                  //!< X86 [M] (restricted to memory operand of any size).
    kEncodingX86Rm,                      //!< X86 [RM] (doesn't handle single-byte size).
    kEncodingX86Arith,                   //!< X86 adc, add, and, cmp, or, sbb, sub, xor.
    kEncodingX86Bswap,                   //!< X86 bswap.
    kEncodingX86Bt,                      //!< X86 bt, btc, btr, bts.
    kEncodingX86Call,                    //!< X86 call.
    kEncodingX86Cmpxchg,                 //!< X86 [MR] cmpxchg.
    kEncodingX86Crc,                     //!< X86 crc32.
    kEncodingX86Enter,                   //!< X86 enter.
    kEncodingX86Imul,                    //!< X86 imul.
    kEncodingX86In,                      //!< X86 in.
    kEncodingX86Ins,                     //!< X86 ins[b|q|d].
    kEncodingX86IncDec,                  //!< X86 inc, dec.
    kEncodingX86Int,                     //!< X86 int (interrupt).
    kEncodingX86Jcc,                     //!< X86 jcc.
    kEncodingX86JecxzLoop,               //!< X86 jcxz, jecxz, jrcxz, loop, loope, loopne.
    kEncodingX86Jmp,                     //!< X86 jmp.
    kEncodingX86Lea,                     //!< X86 lea.
    kEncodingX86Mov,                     //!< X86 mov (all possible cases).
    kEncodingX86MovsxMovzx,              //!< X86 movsx, movzx.
    kEncodingX86Out,                     //!< X86 out.
    kEncodingX86Outs,                    //!< X86 out[b|q|d].
    kEncodingX86Push,                    //!< X86 push.
    kEncodingX86Pop,                     //!< X86 pop.
    kEncodingX86Ret,                     //!< X86 ret.
    kEncodingX86Rot,                     //!< X86 rcl, rcr, rol, ror, sal, sar, shl, shr.
    kEncodingX86Set,                     //!< X86 setcc.
    kEncodingX86ShldShrd,                //!< X86 shld, shrd.
    kEncodingX86StrRm,                   //!< X86 lods.
    kEncodingX86StrMr,                   //!< X86 scas, stos.
    kEncodingX86StrMm,                   //!< X86 cmps, movs.
    kEncodingX86Test,                    //!< X86 test.
    kEncodingX86Xadd,                    //!< X86 xadd.
    kEncodingX86Xchg,                    //!< X86 xchg.
    kEncodingX86Fence,                   //!< X86 lfence, mfence, sfence.
    kEncodingFpuOp,                      //!< FPU [OP].
    kEncodingFpuArith,                   //!< FPU fadd, fdiv, fdivr, fmul, fsub, fsubr.
    kEncodingFpuCom,                     //!< FPU fcom, fcomp.
    kEncodingFpuFldFst,                  //!< FPU fld, fst, fstp.
    kEncodingFpuM,                       //!< FPU fiadd, ficom, ficomp, fidiv, fidivr, fild, fimul, fist, fistp, fisttp, fisub, fisubr.
    kEncodingFpuR,                       //!< FPU fcmov, fcomi, fcomip, ffree, fucom, fucomi, fucomip, fucomp, fxch.
    kEncodingFpuRDef,                    //!< FPU faddp, fdivp, fdivrp, fmulp, fsubp, fsubrp.
    kEncodingFpuStsw,                    //!< FPU fnstsw, Fstsw.
    kEncodingExtRm,                      //!< EXT [RM].
    kEncodingExtRm_XMM0,                 //!< EXT [RM<XMM0>].
    kEncodingExtRm_ZDI,                  //!< EXT [RM<ZDI>].
    kEncodingExtRm_P,                    //!< EXT [RM] (propagates 66H if the instruction uses XMM register).
    kEncodingExtRm_Wx,                   //!< EXT [RM] (propagates REX.W if GPQ is used).
    kEncodingExtRmRi,                    //!< EXT [RM|RI].
    kEncodingExtRmRi_P,                  //!< EXT [RM|RI] (propagates 66H if the instruction uses XMM register).
    kEncodingExtRmi,                     //!< EXT [RMI].
    kEncodingExtRmi_P,                   //!< EXT [RMI] (propagates 66H if the instruction uses XMM register).
    kEncodingExtPextrw,                  //!< EXT pextrw.
    kEncodingExtExtract,                 //!< EXT pextrb, pextrd, pextrq, extractps.
    kEncodingExtMov,                     //!< EXT mov?? - #1:[MM|XMM, MM|XMM|Mem] #2:[MM|XMM|Mem, MM|XMM].
    kEncodingExtMovnti,                  //!< EXT movnti.
    kEncodingExtMovbe,                   //!< EXT movbe.
    kEncodingExtMovd,                    //!< EXT movd.
    kEncodingExtMovq,                    //!< EXT movq.
    kEncodingExtExtrq,                   //!< EXT extrq (SSE4A).
    kEncodingExtInsertq,                 //!< EXT insrq (SSE4A).
    kEncodingExt3dNow,                   //!< EXT [RMI] (3DNOW specific).
    kEncodingVexOp,                      //!< VEX [OP].
    kEncodingVexKmov,                    //!< VEX [RM|MR] (used by kmov[b|w|d|q]).
    kEncodingVexM,                       //!< VEX|EVEX [M].
    kEncodingVexM_VM,                    //!< VEX|EVEX [M] (propagates VEX|EVEX.L, VSIB support).
    kEncodingVexMr_Lx,                   //!< VEX|EVEX [MR] (propagates VEX|EVEX.L if YMM used).
    kEncodingVexMr_VM,                   //!< VEX|EVEX [MR] (propagates VEX|EVEX.L, VSIB support).
    kEncodingVexMri,                     //!< VEX|EVEX [MRI].
    kEncodingVexMri_Lx,                  //!< VEX|EVEX [MRI] (propagates VEX|EVEX.L if YMM used).
    kEncodingVexRm,                      //!< VEX|EVEX [RM].
    kEncodingVexRm_ZDI,                  //!< VEX|EVEX [RM<ZDI>].
    kEncodingVexRm_Lx,                   //!< VEX|EVEX [RM] (propagates VEX|EVEX.L if YMM used).
    kEncodingVexRm_VM,                   //!< VEX|EVEX [RM] (propagates VEX|EVEX.L, VSIB support).
    kEncodingVexRmi,                     //!< VEX|EVEX [RMI].
    kEncodingVexRmi_Wx,                  //!< VEX|EVEX [RMI] (propagates VEX|EVEX.W if GPQ used).
    kEncodingVexRmi_Lx,                  //!< VEX|EVEX [RMI] (propagates VEX|EVEX.L if YMM used).
    kEncodingVexRvm,                     //!< VEX|EVEX [RVM].
    kEncodingVexRvm_Wx,                  //!< VEX|EVEX [RVM] (propagates VEX|EVEX.W if GPQ used).
    kEncodingVexRvm_ZDX_Wx,              //!< VEX|EVEX [RVM<ZDX>] (propagates VEX|EVEX.W if GPQ used).
    kEncodingVexRvm_Lx,                  //!< VEX|EVEX [RVM] (propagates VEX|EVEX.L if YMM used).
    kEncodingVexRvmr,                    //!< VEX|EVEX [RVMR].
    kEncodingVexRvmr_Lx,                 //!< VEX|EVEX [RVMR] (propagates VEX|EVEX.L if YMM used).
    kEncodingVexRvmi,                    //!< VEX|EVEX [RVMI].
    kEncodingVexRvmi_Lx,                 //!< VEX|EVEX [RVMI] (propagates VEX|EVEX.L if YMM used).
    kEncodingVexRmv,                     //!< VEX|EVEX [RMV].
    kEncodingVexRmv_Wx,                  //!< VEX|EVEX [RMV] (propagates VEX|EVEX.W if GPQ used).
    kEncodingVexRmv_VM,                  //!< VEX|EVEX [RMV] (propagates VEX|EVEX.L, VSIB support).
    kEncodingVexRmvRm_VM,                //!< VEX|EVEX [RMV|RM] (propagates VEX|EVEX.L, VSIB support).
    kEncodingVexRmvi,                    //!< VEX|EVEX [RMVI].
    kEncodingVexRmMr,                    //!< VEX|EVEX [RM|MR].
    kEncodingVexRmMr_Lx,                 //!< VEX|EVEX [RM|MR] (propagates VEX|EVEX.L if YMM used).
    kEncodingVexRvmRmv,                  //!< VEX|EVEX [RVM|RMV].
    kEncodingVexRvmRmi,                  //!< VEX|EVEX [RVM|RMI].
    kEncodingVexRvmRmi_Lx,               //!< VEX|EVEX [RVM|RMI] (propagates VEX|EVEX.L if YMM used).
    kEncodingVexRvmRmvRmi,               //!< VEX|EVEX [RVM|RMV|RMI].
    kEncodingVexRvmMr,                   //!< VEX|EVEX [RVM|MR].
    kEncodingVexRvmMvr,                  //!< VEX|EVEX [RVM|MVR].
    kEncodingVexRvmMvr_Lx,               //!< VEX|EVEX [RVM|MVR] (propagates VEX|EVEX.L if YMM used).
    kEncodingVexRvmVmi,                  //!< VEX|EVEX [RVM|VMI].
    kEncodingVexRvmVmi_Lx,               //!< VEX|EVEX [RVM|VMI] (propagates VEX|EVEX.L if YMM used).
    kEncodingVexVm,                      //!< VEX|EVEX [VM].
    kEncodingVexVm_Wx,                   //!< VEX|EVEX [VM] (propagates VEX|EVEX.W if GPQ used).
    kEncodingVexVmi,                     //!< VEX|EVEX [VMI].
    kEncodingVexVmi_Lx,                  //!< VEX|EVEX [VMI] (propagates VEX|EVEX.L if YMM used).
    kEncodingVexEvexVmi_Lx,              //!< VEX|EVEX [VMI] (special, used by vpsrldq and vpslldq)
    kEncodingVexRvrmRvmr,                //!< VEX|EVEX [RVRM|RVMR].
    kEncodingVexRvrmRvmr_Lx,             //!< VEX|EVEX [RVRM|RVMR] (propagates VEX|EVEX.L if YMM used).
    kEncodingVexRvrmiRvmri_Lx,           //!< VEX|EVEX [RVRMI|RVMRI] (propagates VEX|EVEX.L if YMM used).
    kEncodingVexMovdMovq,                //!< VEX|EVEX vmovd, vmovq.
    kEncodingVexMovssMovsd,              //!< VEX|EVEX vmovss, vmovsd.
    kEncodingFma4,                       //!< FMA4 [R, R, R/M, R/M].
    kEncodingFma4_Lx,                    //!< FMA4 [R, R, R/M, R/M] (propagates AVX.L if YMM used).
    _kEncodingCount                      //!< Count of instruction encodings.
  };

  //! Instruction family.
  //!
  //! Specifies which table should be used to interpret `_familyDataIndex`.
  ASMJIT_ENUM(FamilyType) {
    kFamilyNone           = 0,           //!< General purpose or special instruction.
    kFamilyFpu            = 1,           //!< FPU family instruction.
    kFamilySse            = 2,           //!< MMX+/SSE+ family instruction (including SHA/SSE4A).
    kFamilyAvx            = 3            //!< AVX+/FMA+ family instruction (including AVX-512).
  };

  //! \internal
  //!
  //! Instruction flags.
  ASMJIT_ENUM(InstFlags) {
    kInstFlagNone         = 0x00000000U, //!< No flags.

    kInstFlagRO           = 0x00000001U, //!< The first operand is read (read-only without `kInstFlagWO`).
    kInstFlagWO           = 0x00000002U, //!< The first operand is written (write-only without `kInstFlagRO`).
    kInstFlagRW           = 0x00000003U, //!< The first operand is read-write.

    kInstFlagXchg         = 0x00000004U, //!< Instruction is an exchange like instruction (xchg, xadd).
    kInstFlagFlow         = 0x00000008U, //!< Control-flow instruction (jmp, jcc, call, ret).

    kInstFlagVolatile     = 0x00000010U, //!< Volatile instruction, never reorder.
    kInstFlagLock         = 0x00000020U, //!< Instruction can be prefixed by using the LOCK prefix.
    kInstFlagRep          = 0x00000040U, //!< Instruction can be prefixed by using the REP/REPZ/REPNZ prefix.
    kInstFlagRepnz        = 0x00000080U, //!< Instruction can be prefixed by using the REPNZ prefix.

    kInstFlagFp           = 0x00000100U, //!< Instruction accesses FPU register(s).
    kInstFlagSpecial      = 0x00000200U, //!< Instruction requires special handling (implicit operands), used by \ref X86Compiler.

    //! Instruction always performs memory access.
    //!
    //! This flag is always combined with `kInstFlagSpecial` and describes
    //! that there is an implicit address which is accessed (usually EDI/RDI
    //! and/or ESI/RSI).
    kInstFlagSpecialMem   = 0x00000400U,
    kInstFlagZeroIfMem    = 0x00000800U, //!< Cleans the rest of destination if source is memory (movss, movsd).

    kInstFlagFPU_M10      = 0x00001000U, //!< FPU instruction can address tword_ptr (shared with M2).
    kInstFlagFPU_M2       = 0x00001000U, //!< FPU instruction can address word_ptr (shared with M10).
    kInstFlagFPU_M4       = 0x00002000U, //!< FPU instruction can address dword_ptr.
    kInstFlagFPU_M8       = 0x00004000U, //!< FPU instruction can address qword_ptr.

    // ------------------------------------------------------------------------
    // [VEX/EVEX VSIB]
    // ------------------------------------------------------------------------

    // NOTE: If both `kInstFlagVex` and `kInstFlagEvex` flags are specified it
    // means that the instructions is defined by both AVX and AVX512, and can be
    // encoded by either VEX or EVEX prefix. In that case AsmJit checks global
    // options and also instruction options to decide whether to emit EVEX prefix
    // or not.

    kInstFlagVM           = 0x00010000U, //!< Instruction uses a vector memory index (VSIB).
    kInstFlagVex          = 0x00020000U, //!< Instruction can be encoded by VEX (AVX|AVX2|BMI|...).
    kInstFlagVex_VM       = 0x00030000U, //!< Combination of `kInstFlagVex` and `kInstFlagVM`.

    kInstFlagEvex         = 0x00040000U, //!< Instruction can be encoded by EVEX (AVX-512).
    /*
    kInstFlagEvex0        = 0U,          //!< Used inside macros.

    kInstFlagEvexK_       = 0x00100000U, //!< Supports masking {k0..k7}.
    kInstFlagEvexKZ       = 0x00200000U, //!< Supports zeroing of elements {k0..k7}{z}.
    kInstFlagEvexB0       = 0x00000000U, //!< No broadcast (used by instruction tables).
    kInstFlagEvexB4       = 0x00400000U, //!< Supports broadcast 'b32'.
    kInstFlagEvexB8       = 0x00800000U, //!< Supports broadcast 'b64'.
    kInstFlagEvexSAE      = 0x01000000U, //!< Supports 'suppress-all-exceptions' {sae}.
    kInstFlagEvexER       = 0x02000000U, //!< Supports 'embedded-rounding-control' {rc} with implicit {sae},
    kInstFlagEvexVL       = 0x04000000U, //!< Supports access to XMM|YMM registers (AVX512-VL).

    kInstFlagEvexSet_Shift= 28,
    kInstFlagEvexSet_Mask = 0xF0000000U, //!< AVX-512 feature set required to execute the instruction.
    kInstFlagEvexSet_F_   = 0x00000000U, //!< Supported by AVX512-F (no extra requirements).
    kInstFlagEvexSet_CDI  = 0x10000000U, //!< Supported by AVX512-CDI.
    kInstFlagEvexSet_PFI  = 0x20000000U, //!< Supported by AVX512-PFI.
    kInstFlagEvexSet_ERI  = 0x30000000U, //!< Supported by AVX512-ERI.
    kInstFlagEvexSet_DQ   = 0x40000000U, //!< Supported by AVX512-DQ.
    kInstFlagEvexSet_BW   = 0x50000000U, //!< Supported by AVX512-BW.
    kInstFlagEvexSet_IFMA = 0x60000000U, //!< Supported by AVX512-IFMA.
    kInstFlagEvexSet_VBMI = 0x70000000U  //!< Supported by AVX512-VBMI.
    */
  };

  //! Specifies meaning of all bits in an opcode (AsmJit specific).
  //!
  //! This schema is AsmJit specific and has been designed to allow encoding of
  //! all X86 instructions available. X86, MMX, and SSE+ instructions always use
  //! 'MM' and 'PP' fields, which are encoded to corresponding prefixes needed
  //! by X86 or SIMD instructions. AVX+ instructions embed 'MMMMM' and 'PP' fields
  //! in a VEX prefix, and AVX-512 instructions embed 'MM' and 'PP' in EVEX prefix.
  //!
  //! The instruction opcode definition uses 1 or 2 bytes as an opcode value. 1
  //! byte is needed by most of the instructions, 2 bytes are only used by legacy
  //! X87-FPU instructions. This means that a second byte is free to by used by
  //! instructions encoded by using VEX and/or EVEX prefix.
  //!
  //! The fields description:
  //!
  //! - 'MM' field is used to encode prefixes needed by the instruction or as
  //!   a part of VEX/EVEX prefix. Described as 'mm' and 'mmmmm' in instruction
  //!   manuals.
  //!
  //!   NOTE: Since 'MM' field is defined as 'mmmmm' (5 bits), but only 2 least
  //!   significant bits are used by VEX and EVEX prefixes, and additional 4th
  //!   bit is used by XOP prefix, AsmJit uses the 3rd and 5th bit for it's own
  //!   purposes. These bits will probably never be used in future encodings as
  //!   AVX512 uses only '000mm' from 'mmmmm'.
  //!
  //! - 'PP' field is used to encode prefixes needed by the instruction or as a
  //!   part of VEX/EVEX prefix. Described as 'pp' in instruction manuals.
  //!
  //! - 'LL' field is used exclusively by AVX+ and AVX512+ instruction sets. It
  //!   describes vector size, which is 'L.128' for XMM register, 'L.256' for
  //!   for YMM register, and 'L.512' for ZMM register. The 'LL' field is omitted
  //!   in case that instruction supports multiple vector lengths, however, if the
  //!   instruction requires specific `L` value it must be specified as a part of
  //!   the opcode.
  //!
  //!   NOTE: 'LL' having value '11' is not defined yet.
  //!
  //! - 'W' field is the most complicated. It was added by 64-bit architecture
  //!   to promote default operation width (instructions that perform 32-bit
  //!   operation by default require to override the width to 64-bit explicitly).
  //!   There is nothing wrong on this, however, some instructions introduced
  //!   implicit 'W' override, for example a 'cdqe' instruction is basically a
  //!   'cwde' instruction with overridden 'W' (set to 1). There are some others
  //!   in the base X86 instruction set. More recent instruction sets started
  //!   using 'W' field more often:
  //!
  //!   - AVX instructions started using 'W' field as an extended opcode for FMA,
  //!     GATHER, PERM, and other instructions. It also uses 'W' field to override
  //!     the default operation width in instructions like 'vmovq'.
  //!
  //!   - AVX-512 instructions started using 'W' field as an extended opcode for
  //!     all new instructions. This wouldn't have been an issue if the 'W' field
  //!     of AVX-512 have matched AVX, but this is not always the case.
  //!
  //! - 'O' field is an extended opcode field (3 bits) embedded in ModR/M BYTE.
  //!
  //! - 'CDSHL' and 'CDTT' fields describe 'compressed-displacement'. 'CDSHL' is
  //!   defined for each instruction that is AVX-512 encodable (EVEX) and contains
  //!   a base N shift (base shift to perform the calculation). The 'CDTT' field
  //!   is derived from instruction specification and describes additional shift
  //!   to calculate the final 'CDSHL' that will be used in SIB byte.
  //!
  //! NOTE: Don't reorder any fields here, the shifts and masks were defined
  //! carefully to make encoding of X86|X64 instructions fast, especially to
  //! construct REX, VEX, and EVEX prefixes in the most efficient way. Changing
  //! values defined by these enums many cause AsmJit to emit invalid binary
  //! representations of instructions passed to `X86Assembler::_emit`.
  ASMJIT_ENUM(OpCodeBits) {
    // ------------------------------------------------------------------------
    // [MM|VEX|EVEX|XOP]
    // ------------------------------------------------------------------------

    // Two meanings:
    //  * "MMMMM field in AVX/XOP/AVX-512 instruction.
    //  * Part of the opcode in legacy encoding (bytes emitted before the main
    //    opcode byte).
    //
    // AVX reserves 5 bits for `MMMMM` field, however AVX instructions only use
    // 2 bits and XOP 3 bits. AVX-512 shrinks `MMMMM` field into `MM` so it's
    // safe to assume that bits [4:2] of `MM` field won't be used in future
    // extensions, which will most probably use EVEX encoding. AsmJit divides
    // MM field into this layout:
    //
    // [1:0] - Used to describe 0F, 0F38 and 0F3A legacy prefix bytes and
    //         2 bits of MM field.
    // [2]   - Used to force 3-BYTE VEX prefix, but then cleared to zero before
    //         the prefix is emitted. This bit is not used by any instruction
    //         so it can be used for any purpose by AsmJit. Also, this bit is
    //         used as an extension to MM field describing 0F|0F38|0F3A to also
    //         describe 0F01 as used by some legacy instructions (instructions
    //         not using VEX/EVEX prefix).
    // [3]   - Required by XOP instructions, so we use this bit also to indicate
    //         that this is a XOP opcode.
    kOpCode_MM_Shift      = 8,
    kOpCode_MM_Mask       = 0x1FU << kOpCode_MM_Shift,
    kOpCode_MM_00         = 0x00U << kOpCode_MM_Shift,
    kOpCode_MM_0F         = 0x01U << kOpCode_MM_Shift,
    kOpCode_MM_0F38       = 0x02U << kOpCode_MM_Shift,
    kOpCode_MM_0F3A       = 0x03U << kOpCode_MM_Shift, // Described also as XOP.M3 in AMD manuals.
    kOpCode_MM_0F01       = 0x04U << kOpCode_MM_Shift, // AsmJit way to describe 0F01 (never VEX/EVEX).

    // `XOP` field is only used to force XOP prefix instead of VEX3 prefix. We
    // know that only XOP encoding uses bit 0b1000 of MM field and that no VEX
    // and EVEX instruction uses such bit, so we can use this bit to force XOP
    // prefix to be emitted instead of VEX3 prefix. See `x86VEXPrefix` defined
    // in `x86assembler.cpp`.
    kOpCode_MM_XOP08      = 0x08U << kOpCode_MM_Shift, // XOP.M8.
    kOpCode_MM_XOP09      = 0x09U << kOpCode_MM_Shift, // XOP.M9.

    kOpCode_MM_IsXOP_Shift= kOpCode_MM_Shift + 3,
    kOpCode_MM_IsXOP      = kOpCode_MM_XOP08,

    // NOTE: Force VEX3 allows to force to emit VEX3 instead of VEX2 in some
    // cases (similar to forcing REX prefix). Force EVEX will force emitting
    // EVEX prefix instead of VEX2|VEX3. EVEX-only instructions will have
    // ForceEvex always set, however. instructions that can be encoded by
    // either VEX or EVEX prefix shall not have ForceEvex set.

    kOpCode_MM_ForceVex3  = 0x04U << kOpCode_MM_Shift, // Force 3-BYTE VEX prefix.
    kOpCode_MM_ForceEvex  = 0x10U << kOpCode_MM_Shift, // Force 4-BYTE EVEX prefix.

    // ------------------------------------------------------------------------
    // [FPU_2B (Second-Byte of OpCode used by FPU)]
    // ------------------------------------------------------------------------

    // Second byte opcode. This BYTE is ONLY used by FPU instructions and
    // collides with 3 bits from 'MM' and 5 bits from 'CDSHL' and 'CDTT'.
    // It's fine as FPU and AVX512 flags are never used at the same time.
    kOpCode_FPU_2B_Shift  = 10,
    kOpCode_FPU_2B_Mask   = 0xFF << kOpCode_FPU_2B_Shift,

    // ------------------------------------------------------------------------
    // [CDSHL | CDTT]
    // ------------------------------------------------------------------------

    // Compressed displacement bits.
    //
    // Each opcode defines the base size (N) shift:
    //   0: BYTE  (1 byte).
    //   1: WORD  (2 bytes).
    //   2: DWORD (4 bytes - float/int32).
    //   3: QWORD (8 bytes - double/int64).
    //   4: OWORD (16 bytes - used by FV|FVM|M128).
    // Which is then scaled by the instruction's TT (TupleType) into possible:
    //   5: YWORD (32 bytes)
    //   6: ZWORD (64 bytes)
    //
    // These bits are then adjusted before calling EmitModSib or EmitModVSib.
    kOpCode_CDSHL_Shift   = 13,
    kOpCode_CDSHL_Mask    = 0x7 << kOpCode_CDSHL_Shift,

    // Compressed displacement tuple-type (specific to AsmJit).
    //
    // Since we store the base offset independently of CDTT we can simplify the
    // number of 'TUPLE_TYPE' kinds significantly and just handle special cases.
    kOpCode_CDTT_Shift    = 16,
    kOpCode_CDTT_Mask     = 0x3 << kOpCode_CDTT_Shift,
    kOpCode_CDTT_None     = 0x0 << kOpCode_CDTT_Shift, // Does nothing.
    kOpCode_CDTT_ByLL     = 0x1 << kOpCode_CDTT_Shift, // Scales by LL (1x 2x 4x).
    kOpCode_CDTT_T1W      = 0x2 << kOpCode_CDTT_Shift, // Used to add 'W' to the shift.
    kOpCode_CDTT_DUP      = 0x3 << kOpCode_CDTT_Shift, // Special 'VMOVDDUP' case.

    // Aliases that match names used in instruction manuals.
    kOpCode_CDTT_FV       = kOpCode_CDTT_ByLL,
    kOpCode_CDTT_HV       = kOpCode_CDTT_ByLL,
    kOpCode_CDTT_FVM      = kOpCode_CDTT_ByLL,
    kOpCode_CDTT_T1S      = kOpCode_CDTT_None,
    kOpCode_CDTT_T1F      = kOpCode_CDTT_None,
    kOpCode_CDTT_T2       = kOpCode_CDTT_None,
    kOpCode_CDTT_T4       = kOpCode_CDTT_None,
    kOpCode_CDTT_T8       = kOpCode_CDTT_None,
    kOpCode_CDTT_HVM      = kOpCode_CDTT_ByLL,
    kOpCode_CDTT_QVM      = kOpCode_CDTT_ByLL,
    kOpCode_CDTT_OVM      = kOpCode_CDTT_ByLL,
    kOpCode_CDTT_128      = kOpCode_CDTT_None,

    // ------------------------------------------------------------------------
    // [O]
    // ------------------------------------------------------------------------

    // "O' field in ModR/M.
    kOpCode_O_Shift       = 18,
    kOpCode_O_Mask        = 0x07U << kOpCode_O_Shift,

    // ------------------------------------------------------------------------
    // [PP and L]
    // ------------------------------------------------------------------------

    // These fields are stored deliberately right after each other as it makes
    // it easier to construct VEX prefix from the opcode value stored in the
    // instruction database.

    // Two meanings:
    //   * "PP" field in AVX/XOP/AVX-512 instruction.
    //   * Mandatory Prefix in legacy encoding.
    //
    // AVX reserves 2 bits for `PP` field, but AsmJit extends the storage by 1
    // more bit that is used to emit 9B prefix for some X87-FPU instructions.
    kOpCode_PP_Shift      = 21,
    kOpCode_PP_VEXMask    = 0x03U << kOpCode_PP_Shift, // PP field mask used by VEX/EVEX.
    kOpCode_PP_FPUMask    = 0x07U << kOpCode_PP_Shift, // Mask used by EMIT_PP, also includes 0x9B.
    kOpCode_PP_00         = 0x00U << kOpCode_PP_Shift,
    kOpCode_PP_66         = 0x01U << kOpCode_PP_Shift,
    kOpCode_PP_F3         = 0x02U << kOpCode_PP_Shift,
    kOpCode_PP_F2         = 0x03U << kOpCode_PP_Shift,

    // AsmJit specific to emit FPU's 9B byte.
    kOpCode_PP_9B         = 0x07U << kOpCode_PP_Shift,

    // ------------------------------------------------------------------------
    // [EVEX.W]
    // ------------------------------------------------------------------------

    // `W` field used by EVEX instruction encoding.
    kOpCode_EW_Shift      = 24,
    kOpCode_EW            = 0x01U << kOpCode_EW_Shift,

    // ------------------------------------------------------------------------
    // [REX BXRW bits (part of REX prefix)]
    //
    // NOTE: REX.[B|X|R] are never stored within the opcode itself, they are
    // reserved by AsmJit are are added dynamically to the opcode to represent
    // [REX|VEX|EVEX].[B|X|R] bits. REX.W can be stored in DB as it's sometimes
    // part of the opcode itself.
    // ------------------------------------------------------------------------

    // These must be binary compatible with instruction options.
    kOpCode_REX_Shift     = 25,
    kOpCode_REX_Mask      = 0x0FU << kOpCode_REX_Shift,
    kOpCode_B             = 0x01U << kOpCode_REX_Shift, // Never stored in DB.
    kOpCode_X             = 0x02U << kOpCode_REX_Shift, // Never stored in DB.
    kOpCode_R             = 0x04U << kOpCode_REX_Shift, // Never stored in DB.
    kOpCode_W             = 0x08U << kOpCode_REX_Shift,
    kOpCode_W_Shift       = kOpCode_REX_Shift + 3,

    // `L` field in AVX/XOP/AVX-512 instruction.
    //
    // VEX/XOP prefix can only use the first bit `L.128` or `L.256`. EVEX prefix
    // prefix makes it possible to use also `L.512`.
    //
    // If the instruction set manual describes an instruction by `LIG` it means
    // that the `L` field is ignored and AsmJit defaults to `0` in such case.
    kOpCode_LL_Shift      = 29,
    kOpCode_LL_Mask       = 0x03U << kOpCode_LL_Shift,
    kOpCode_LL_128        = 0x00U << kOpCode_LL_Shift,
    kOpCode_LL_256        = 0x01U << kOpCode_LL_Shift,
    kOpCode_LL_512        = 0x02U << kOpCode_LL_Shift
  };

  //! Instruction options.
  ASMJIT_ENUM(Options) {
    // NOTE: Don't collide with reserved bits used by CodeEmitter (0x000000FF).

    kOptionOp4            = CodeEmitter::kOptionOp4,
    kOptionOp5            = CodeEmitter::kOptionOp5,
    kOptionOpExtra        = CodeEmitter::kOptionOpExtra,

    kOptionShortForm      = 0x00000100U, //!< Emit short-form of the instruction.
    kOptionLongForm       = 0x00000200U, //!< Emit long-form of the instruction.

    kOptionVex3           = 0x00000400U, //!< Use 3-byte VEX prefix if possible (AVX) (must be 0x00000400).
    kOptionModMR          = 0x00000800U, //!< Use ModMR instead of ModRM when it's available.
    kOptionEvex           = 0x00001000U, //!< Use 4-byte EVEX prefix if possible (AVX-512) (must be 0x00001000).

    kOptionLock           = 0x00002000U, //!< LOCK prefix (lock-enabled instructions only).
    kOptionRep            = 0x00004000U, //!< REP/REPZ prefix (string instructions only).
    kOptionRepnz          = 0x00008000U, //!< REPNZ prefix (string instructions only).

    kOptionTaken          = 0x00010000U, //!< JCC likely to be taken (historic, only takes effect on P4).
    kOptionNotTaken       = 0x00020000U, //!< JCC unlikely to be taken (historic, only takes effect on P4).

    kOptionSAE            = 0x00040000U, //!< AVX-512: 'suppress-all-exceptions' {sae}.
    kOptionER             = 0x00080000U, //!< AVX-512: 'rounding-control' {rc} and {sae}.

    kOption1ToX           = 0x00100000U, //!< AVX-512: broadcast the first element to all {1tox}.
    kOptionRN_SAE         = 0x00000000U, //!< AVX-512: round-to-nearest (even)      {rn-sae} (bits 00).
    kOptionRD_SAE         = 0x00200000U, //!< AVX-512: round-down (toward -inf)     {rd-sae} (bits 01).
    kOptionRU_SAE         = 0x00400000U, //!< AVX-512: round-up (toward +inf)       {ru-sae} (bits 10).
    kOptionRZ_SAE         = 0x00600000U, //!< AVX-512: round-toward-zero (truncate) {rz-sae} (bits 11).
    kOptionKZ             = 0x00800000U, //!< AVX-512: Use zeroing {k}{z} instead of merging {k}.

    _kOptionInvalidRex    = 0x01000000U, //!< REX prefix can't be emitted (internal).
    kOptionOpCodeB        = 0x02000000U, //!< REX.B and/or VEX.B field (X64).
    kOptionOpCodeX        = 0x04000000U, //!< REX.X and/or VEX.X field (X64).
    kOptionOpCodeR        = 0x08000000U, //!< REX.R and/or VEX.R field (X64).
    kOptionOpCodeW        = 0x10000000U, //!< REX.W and/or VEX.W field (X64).
    kOptionRex            = 0x80000000U  //!< Use REX prefix (X64) (must be 0x80000000).
  };

  //! Condition codes.
  ASMJIT_ENUM(Cond) {
    kCondA                = 0x07U,       // CF==0 & ZF==0          (unsigned)
    kCondAE               = 0x03U,       // CF==0                  (unsigned)
    kCondB                = 0x02U,       // CF==1                  (unsigned)
    kCondBE               = 0x06U,       // CF==1 | ZF==1          (unsigned)
    kCondC                = 0x02U,       // CF==1
    kCondE                = 0x04U,       //         ZF==1          (signed/unsigned)
    kCondG                = 0x0FU,       //         ZF==0 & SF==OF (signed)
    kCondGE               = 0x0DU,       //                 SF==OF (signed)
    kCondL                = 0x0CU,       //                 SF!=OF (signed)
    kCondLE               = 0x0EU,       //         ZF==1 | SF!=OF (signed)
    kCondNA               = 0x06U,       // CF==1 | ZF==1          (unsigned)
    kCondNAE              = 0x02U,       // CF==1                  (unsigned)
    kCondNB               = 0x03U,       // CF==0                  (unsigned)
    kCondNBE              = 0x07U,       // CF==0 & ZF==0          (unsigned)
    kCondNC               = 0x03U,       // CF==0
    kCondNE               = 0x05U,       //         ZF==0          (signed/unsigned)
    kCondNG               = 0x0EU,       //         ZF==1 | SF!=OF (signed)
    kCondNGE              = 0x0CU,       //                 SF!=OF (signed)
    kCondNL               = 0x0DU,       //                 SF==OF (signed)
    kCondNLE              = 0x0FU,       //         ZF==0 & SF==OF (signed)
    kCondNO               = 0x01U,       //                 OF==0
    kCondNP               = 0x0BU,       // PF==0
    kCondNS               = 0x09U,       //                 SF==0
    kCondNZ               = 0x05U,       //         ZF==0
    kCondO                = 0x00U,       //                 OF==1
    kCondP                = 0x0AU,       // PF==1
    kCondPE               = 0x0AU,       // PF==1
    kCondPO               = 0x0BU,       // PF==0
    kCondS                = 0x08U,       //                 SF==1
    kCondZ                = 0x04U,       //         ZF==1

    // Simplified condition codes.
    kCondSign             = kCondS,      //!< Sign (S).
    kCondNotSign          = kCondNS,     //!< Not Sign (NS).

    kCondOverflow         = kCondO,      //!< Signed  Overflow (O)
    kCondNotOverflow      = kCondNO,     //!< Not Signed Overflow (NO)

    kCondLess             = kCondL,      //!< Signed     `a <  b` (L  or NGE).
    kCondLessEqual        = kCondLE,     //!< Signed     `a <= b` (LE or NG ).
    kCondGreater          = kCondG,      //!< Signed     `a >  b` (G  or NLE).
    kCondGreaterEqual     = kCondGE,     //!< Signed     `a >= b` (GE or NL ).
    kCondBelow            = kCondB,      //!< Unsigned   `a <  b` (B  or NAE).
    kCondBelowEqual       = kCondBE,     //!< Unsigned   `a <= b` (BE or NA ).
    kCondAbove            = kCondA,      //!< Unsigned   `a >  b` (A  or NBE).
    kCondAboveEqual       = kCondAE,     //!< Unsigned   `a >= b` (AE or NB ).
    kCondEqual            = kCondE,      //!< Equal      `a == b` (E  or Z  ).
    kCondNotEqual         = kCondNE,     //!< Not Equal  `a != b` (NE or NZ ).

    kCondParityEven       = kCondP,
    kCondParityOdd        = kCondPO,

    kCondZero             = kCondZ,
    kCondNotZero          = kCondNZ,
    kCondNegative         = kCondS,
    kCondPositive         = kCondNS,

    // FPU-only.
    kCondFpuUnordered     = 0x10,
    kCondFpuNotUnordered  = 0x11,

    //! No condition code.
    kCondNone             = 0x12
  };

  //! Comparison predicate used by CMP[PD|PS|SD|SS] instructions.
  ASMJIT_ENUM(CmpPredicate) {
    kCmpEQ                = 0x00,        //!< Equal             (Quiet).
    kCmpLT                = 0x01,        //!< Less              (Signaling).
    kCmpLE                = 0x02,        //!< Less/Equal        (Signaling).
    kCmpUNORD             = 0x03,        //!< Unordered         (Quiet).
    kCmpNEQ               = 0x04,        //!< Not Equal         (Quiet).
    kCmpNLT               = 0x05,        //!< Not Less          (Signaling).
    kCmpNLE               = 0x06,        //!< Not Less/Equal    (Signaling).
    kCmpORD               = 0x07         //!< Ordered           (Quiet).
  };

  //! Comparison predicate used by VCMP[PD|PS|SD|SS] instructions.
  //!
  //! The first 8 values are compatible with \ref CmpPredicate.
  ASMJIT_ENUM(VCmpPredicate) {
    kVCmpEQ_OQ            = 0x00,        //!< Equal             (Quiet    , Ordered).
    kVCmpLT_OS            = 0x01,        //!< Less              (Signaling, Ordered).
    kVCmpLE_OS            = 0x02,        //!< Less/Equal        (Signaling, Ordered).
    kVCmpUNORD_Q          = 0x03,        //!< Unordered         (Quiet).
    kVCmpNEQ_UQ           = 0x04,        //!< Not Equal         (Quiet    , Unordered).
    kVCmpNLT_US           = 0x05,        //!< Not Less          (Signaling, Unordered).
    kVCmpNLE_US           = 0x06,        //!< Not Less/Equal    (Signaling, Unordered).
    kVCmpORD_Q            = 0x07,        //!< Ordered           (Quiet).
    kVCmpEQ_UQ            = 0x08,        //!< Equal             (Quiet    , Unordered).
    kVCmpNGE_US           = 0x09,        //!< Not Greater/Equal (Signaling, Unordered).
    kVCmpNGT_US           = 0x0A,        //!< Not Greater       (Signaling, Unordered).
    kVCmpFALSE_OQ         = 0x0B,        //!< False             (Quiet    , Ordered).
    kVCmpNEQ_OQ           = 0x0C,        //!< Not Equal         (Quiet    , Ordered).
    kVCmpGE_OS            = 0x0D,        //!< Greater/Equal     (Signaling, Ordered).
    kVCmpGT_OS            = 0x0E,        //!< Greater           (Signaling, Ordered).
    kVCmpTRUE_UQ          = 0x0F,        //!< True              (Quiet    , Unordered).
    kVCmpEQ_OS            = 0x10,        //!< Equal             (Signaling, Ordered).
    kVCmpLT_OQ            = 0x11,        //!< Less              (Quiet    , Ordered).
    kVCmpLE_OQ            = 0x12,        //!< Less/Equal        (Quiet    , Ordered).
    kVCmpUNORD_S          = 0x13,        //!< Unordered         (Signaling).
    kVCmpNEQ_US           = 0x14,        //!< Not Equal         (Signaling, Unordered).
    kVCmpNLT_UQ           = 0x15,        //!< Not Less          (Quiet    , Unordered).
    kVCmpNLE_UQ           = 0x16,        //!< Not Less/Equal    (Quiet    , Unordered).
    kVCmpORD_S            = 0x17,        //!< Ordered           (Signaling).
    kVCmpEQ_US            = 0x18,        //!< Equal             (Signaling, Unordered).
    kVCmpNGE_UQ           = 0x19,        //!< Not Greater/Equal (Quiet    , Unordered).
    kVCmpNGT_UQ           = 0x1A,        //!< Not Greater       (Quiet    , Unordered).
    kVCmpFALSE_OS         = 0x1B,        //!< False             (Signaling, Ordered).
    kVCmpNEQ_OS           = 0x1C,        //!< Not Equal         (Signaling, Ordered).
    kVCmpGE_OQ            = 0x1D,        //!< Greater/Equal     (Quiet    , Ordered).
    kVCmpGT_OQ            = 0x1E,        //!< Greater           (Quiet    , Ordered).
    kVCmpTRUE_US          = 0x1F         //!< True              (Signaling, Unordered).
  };

  //! Round predicate used by ROUND[PD|PS|SD|SS] instructions.
  ASMJIT_ENUM(RoundPredicate) {
    kRoundNearest         = 0x00,        //!< Round to nearest (even).
    kRoundDown            = 0x01,        //!< Round to down toward -INF (floor),
    kRoundUp              = 0x02,        //!< Round to up toward +INF (ceil).
    kRoundTrunc           = 0x03,        //!< Round toward zero (truncate).
    kRoundCurrent         = 0x04,        //!< Round to the current rounding mode set (ignores other RC bits).
    kRoundInexact         = 0x08         //!< Avoids inexact exception, if set.
  };

  //! Supported architectures.
  ASMJIT_ENUM(ArchMask) {
    kArchMaskX86          = 0x01,        //!< X86 mode supported.
    kArchMaskX64          = 0x02         //!< X64 mode supported.
  };

  //! Instruction's operand flags.
  enum OpFlags {
    kOpNone               = 0x00000000U, //!< No operand.

    kOpGpbLo              = 0x00000001U, //!< Operand can be a low 8-bit GPB register.
    kOpGpbHi              = 0x00000002U, //!< Operand can be a high 8-bit GPB register.
    kOpGpw                = 0x00000004U, //!< Operand can be a 16-bit GPW register.
    kOpGpd                = 0x00000008U, //!< Operand can be a 32-bit GPD register.
    kOpGpq                = 0x00000010U, //!< Operand can be a 64-bit GPQ register.
    kOpFp                 = 0x00000020U, //!< Operand can be an FPU register.
    kOpMm                 = 0x00000040U, //!< Operand can be a 64-bit MM register.
    kOpK                  = 0x00000080U, //!< Operand can be a 64-bit K register.
    kOpCr                 = 0x00000100U, //!< Operand can be a control register.
    kOpDr                 = 0x00000200U, //!< Operand can be a debug register.
    kOpBnd                = 0x00000400U, //!< Operand can be a BND register.
    kOpSeg                = 0x00000800U, //!< Operand can be a segment register.
    kOpXmm                = 0x00001000U, //!< Operand can be a 128-bit XMM register.
    kOpYmm                = 0x00002000U, //!< Operand can be a 256-bit YMM register.
    kOpZmm                = 0x00004000U, //!< Operand can be a 512-bit ZMM register.

    kOpAllRegs            = 0x00007FFFU, //!< Combination of all possible registers.

    kOpMem                = 0x00010000U, //!< Operand can be a scalar memory pointer.
    kOpVm                 = 0x00020000U, //!< Operand can be a vector memory pointer.
    kOpI4                 = 0x00040000U, //!< Operand can be a 4-bit immediate.
    kOpI8                 = 0x00080000U, //!< Operand can be an 8-bit immediate.
    kOpI16                = 0x00100000U, //!< Operand can be a 16-bit immediate.
    kOpI32                = 0x00200000U, //!< Operand can be a 32-bit immediate.
    kOpI64                = 0x00400000U, //!< Operand can be a 64-bit immediate.
    kOpRel8               = 0x01000000U, //!< Operand can be an 8-bit displacement.
    kOpRel32              = 0x02000000U, //!< Operand can be a 32-bit displacement.

    kOpR                  = 0x10000000U, //!< Operand is read.
    kOpW                  = 0x20000000U, //!< Operand is written.
    kOpX                  = 0x30000000U, //!< Operand is read & written.
    kOpImplicit           = 0x80000000U  //!< Operand is implicit.
  };

  //! Instruction's memory operand flags.
  enum MemOpFlags {
    // NOTE: Instruction uses either scalar or vector memory operands, they
    // never collide, this is the reason "M" and "Vm" can share bits here.

    kMemOpM8              = 0x0001U,     //!< Operand can be an 8-bit memory pointer.
    kMemOpM16             = 0x0002U,     //!< Operand can be a 16-bit memory pointer.
    kMemOpM32             = 0x0004U,     //!< Operand can be a 32-bit memory pointer.
    kMemOpM64             = 0x0008U,     //!< Operand can be a 64-bit memory pointer.
    kMemOpM80             = 0x0010U,     //!< Operand can be an 80-bit memory pointer.
    kMemOpM128            = 0x0020U,     //!< Operand can be a 128-bit memory pointer.
    kMemOpM256            = 0x0040U,     //!< Operand can be a 256-bit memory pointer.
    kMemOpM512            = 0x0080U,     //!< Operand can be a 512-bit memory pointer.
    kMemOpM1024           = 0x0100U,     //!< Operand can be a 1024-bit memory pointer.

    kMemOpVm32x           = 0x0001U,     //!< Operand can be a vm32x (vector) pointer.
    kMemOpVm32y           = 0x0002U,     //!< Operand can be a vm32y (vector) pointer.
    kMemOpVm32z           = 0x0004U,     //!< Operand can be a vm32z (vector) pointer.
    kMemOpVm64x           = 0x0010U,     //!< Operand can be a vm64x (vector) pointer.
    kMemOpVm64y           = 0x0020U,     //!< Operand can be a vm64y (vector) pointer.
    kMemOpVm64z           = 0x0040U,     //!< Operand can be a vm64z (vector) pointer.

    kMemOpBaseOnly        = 0x0200U,     //!< Only memory base is allowed (no index, no offset).
    kMemOpDs              = 0x0400U,     //!< Implicit memory operand's DS segment.
    kMemOpEs              = 0x0800U,     //!< Implicit memory operand's ES segment.

    kMemOpMib             = 0x4000U,     //!< Operand must be MIB (base+index) pointer.
    kMemOpAny             = 0x8000U      //!< Operand can be any scalar memory pointer.
  };

  //! Common data - aggregated data that is shared across many instructions.
  struct CommonData {
    // ------------------------------------------------------------------------
    // [Accessors]
    // ------------------------------------------------------------------------

    //! Get all instruction flags, see \ref InstFlags.
    ASMJIT_INLINE uint32_t getFlags() const noexcept { return _flags; }
    //! Get whether the instruction has a `flag`, see `InstFlags`.
    ASMJIT_INLINE bool hasFlag(uint32_t flag) const noexcept { return (_flags & flag) != 0; }

    //! Get if the first operand is read-only.
    ASMJIT_INLINE bool isRO() const noexcept { return (getFlags() & kInstFlagRW) == kInstFlagRO; }
    //! Get if the first operand is write-only.
    ASMJIT_INLINE bool isWO() const noexcept { return (getFlags() & kInstFlagRW) == kInstFlagWO; }
    //! Get if the first operand is read-write.
    ASMJIT_INLINE bool isRW() const noexcept { return (getFlags() & kInstFlagRW) == kInstFlagRW; }

    //! Get whether the instruction is a typical Exchange instruction.
    //!
    //! Exchange instructions are 'xchg' and 'xadd'.
    ASMJIT_INLINE bool isXchg() const noexcept { return hasFlag(kInstFlagXchg); }

    //! Get whether the instruction is a control-flow instruction.
    //!
    //! Control flow instruction is instruction that can perform a branch,
    //! typically `jmp`, `jcc`, `call`, or `ret`.
    ASMJIT_INLINE bool isFlow() const noexcept { return hasFlag(kInstFlagFlow); }

    //! Get whether the instruction accesses Fp register(s).
    ASMJIT_INLINE bool isFp() const noexcept { return hasFlag(kInstFlagFp); }

    //! Get whether the instruction can be prefixed by LOCK prefix.
    ASMJIT_INLINE bool isLockable() const noexcept { return hasFlag(kInstFlagLock); }

    //! Get whether the instruction is special type (this is used by `Compiler`
    //! to manage additional variables or functionality).
    ASMJIT_INLINE bool isSpecial() const noexcept { return hasFlag(kInstFlagSpecial); }

    //! Get whether the instruction is special type and it performs memory access.
    ASMJIT_INLINE bool isSpecialMem() const noexcept { return hasFlag(kInstFlagSpecialMem); }

    //! Get whether the move instruction clears the rest of the register
    //! if the source is memory operand.
    //!
    //! Basically flag needed only to support `movsd` and `movss` instructions.
    ASMJIT_INLINE bool isZeroIfMem() const noexcept { return hasFlag(kInstFlagZeroIfMem); }

    //! Get EFLAGS that the instruction reads, see \ref X86EFlags.
    ASMJIT_INLINE uint32_t getEFlagsIn() const noexcept { return _eflagsIn; }
    //! Get EFLAGS that the instruction writes, see \ref X86EFlags.
    ASMJIT_INLINE uint32_t getEFlagsOut() const noexcept { return _eflagsOut; }

    //! Get the destination index of WRITE operation.
    ASMJIT_INLINE uint32_t getWriteIndex() const noexcept { return _writeIndex; }
    //! Get the number of bytes that will be written by a WRITE operation.
    //!
    //! This information is required by a liveness analysis to mark virtual
    //! registers dead even if the instruction doesn't completely overwrite
    //! the whole register. If the analysis keeps which bytes are completely
    //! overwritten by the instruction it can find the where a register becomes
    //! dead by simply checking if the instruction overwrites all remaining
    //! bytes.
    ASMJIT_INLINE uint32_t getWriteSize() const noexcept { return _writeSize; }

    //! Get if the instruction has alternative opcode.
    ASMJIT_INLINE bool hasAltOpCode() const noexcept { return _altOpCodeIndex != 0; }
    //! Get alternative opcode, see \ref OpCodeBits.
    ASMJIT_INLINE uint32_t getAltOpCode() const noexcept;

    // ------------------------------------------------------------------------
    // [Members]
    // ------------------------------------------------------------------------

    uint32_t _flags;                     //!< Instruction flags.

    uint32_t _writeIndex      : 8;       //!< First DST byte of a WRITE operation (default 0).
    uint32_t _writeSize       : 8;       //!< number of bytes to be written in DST.
    uint32_t _eflagsIn        : 8;       //!< EFLAGS read by the instruction.
    uint32_t _eflagsOut       : 8;       //!< EFLAGS modified by the instruction.

    uint32_t _altOpCodeIndex  : 8;       //!< Index to table with alternative opcodes.
    uint32_t _iSignatureIndex : 9;       //!< First `ISignature` entry in the database.
    uint32_t _iSignatureCount : 4;       //!< Number of relevant `ISignature` entries.
    uint32_t _reserved        : 19;      //!< \internal
  };

  //! Data specific to FPU family instructions that access FPU stack.
  struct FpuData {
  };

  //! Data specific to MMX+ and SSE+ family instructions.
  struct SseData {
    enum Features {
      kFeatureMMX             = 0x0001U, //!< Supported by MMX.
      kFeatureMMX2            = 0x0002U, //!< Supported by MMX2 (MMX-Ext).
      kFeature3DNOW           = 0x0004U, //!< Supported by 3DNOW.
      kFeature3DNOW2          = 0x0008U, //!< Supported by 3DNOW2 (Enhanced).
      kFeatureGEODE           = 0x0010U, //!< Supported by GEODE (deprecated).
      kFeatureSSE             = 0x0020U, //!< Supported by SSE.
      kFeatureSSE2            = 0x0040U, //!< Supported by SSE2.
      kFeatureSSE3            = 0x0080U, //!< Supported by SSE3.
      kFeatureSSSE3           = 0x0100U, //!< Supported by SSSE3.
      kFeatureSSE4_1          = 0x0200U, //!< Supported by SSE4.1.
      kFeatureSSE4_2          = 0x0400U, //!< Supported by SSE4.2.
      kFeatureSSE4A           = 0x0800U, //!< Supported by SSE4A.
      kFeaturePCLMULQDQ       = 0x1000U, //!< Supported by PCLMULQDQ.
      kFeatureAES             = 0x2000U, //!< Supported by AES.
      kFeatureSHA             = 0x4000U  //!< Supported by SHA.
    };

    //! SSE to AVX conversion mode.
    enum AvxConvMode {
      kAvxConvNone            = 0,       //!< No translation possible (MMX/SSE4A/SHA instruction).
      kAvxConvNonDestructive  = 1,       //!< The first SSE operand becomes first and second AVX operand.
      kAvxConvMove            = 2,       //!< No change (no operands changed).
      kAvxConvMoveIfMem       = 3,       //!< No change if second operand is a memory, otherwise NonDestructive.
      kAvxConvBlend           = 4        //!< Special case for 'vblendvpd', 'vblendvps', and 'vpblendvb'.
    };

    uint32_t features         : 16;      //!< CPU features.
    uint32_t avxConvMode      :  3;      //!< SSE to AVX conversion mode, see \ref AvxConvMode.
    int32_t avxConvDelta      : 13;      //!< Delta to get a corresponding AVX instruction.
  };

  //! Data specific to AVX+ and FMA+ family instructions.
  struct AvxData {
    //! AVX/AVX512 features.
    enum Features {
      kFeatureAVX         = 0x00000001U, //!< Supported by AVX.
      kFeatureAVX2        = 0x00000002U, //!< Supported by AVX2.
      kFeatureAES         = 0x00000004U, //!< Supported by AVX & AES.
      kFeatureF16C        = 0x00000008U, //!< Supported by F16C.
      kFeatureFMA         = 0x00000010U, //!< Supported by FMA.
      kFeatureFMA4        = 0x00000020U, //!< Supported by FMA4.
      kFeaturePCLMULQDQ   = 0x00000040U, //!< Supported by PCLMULQDQ & AVX.
      kFeatureXOP         = 0x00000080U, //!< Supported by XOP.
      kFeatureAVX512F     = 0x00001000U, //!< Supported by AVX512-F (foundation).
      kFeatureAVX512VL    = 0x00002000U, //!< Supports access to XMM|YMM registers if AVX512VL is present.
      kFeatureAVX512CDI   = 0x00004000U, //!< Supported by AVX512-CDI (conflict detection).
      kFeatureAVX512PFI   = 0x00008000U, //!< Supported by AVX512-PFI (prefetch).
      kFeatureAVX512ERI   = 0x00010000U, //!< Supported by AVX512-ERI (exponential and reciprocal).
      kFeatureAVX512DQ    = 0x00020000U, //!< Supported by AVX512-DQ (dword/qword).
      kFeatureAVX512BW    = 0x00040000U, //!< Supported by AVX512-BW (byte/word).
      kFeatureAVX512IFMA  = 0x00080000U, //!< Supported by AVX512-IFMA (integer fused-multiply-add).
      kFeatureAVX512VBMI  = 0x00100000U  //!< Supported by AVX512-VBMI (vector byte manipulation).
    };

    //!< Additional flags (AVX512).
    enum Flags {
      kFlagMasking            = 0x0010U, //!< Supports masking {k0..k7}.
      kFlagZeroing            = 0x0020U, //!< Supports zeroing of elements {k0..k7}{z} (must be used together with 'K').
      kFlagBroadcast32        = 0x0040U, //!< Supports 32-bit broadcast 'b32'.
      kFlagBroadcast64        = 0x0080U, //!< Supports 64-bit broadcast 'b64'.
      kFlagER                 = 0x0100U, //!< Supports 'embedded-rounding-control' {rc} with implicit {sae},
      kFlagSAE                = 0x0200U, //!< Supports 'suppress-all-exceptions' {sae}.
    };

    ASMJIT_INLINE bool hasFlag(uint32_t flag) const noexcept { return (flags & flag) != 0; }
    ASMJIT_INLINE bool hasMasking() const noexcept { return hasFlag(kFlagMasking); }
    ASMJIT_INLINE bool hasZeroing() const noexcept { return hasFlag(kFlagZeroing); }

    ASMJIT_INLINE bool hasER() const noexcept { return hasFlag(kFlagER); }
    ASMJIT_INLINE bool hasSAE() const noexcept { return hasFlag(kFlagSAE); }
    ASMJIT_INLINE bool hasEROrSAE() const noexcept { return hasFlag(kFlagER | kFlagSAE); }

    ASMJIT_INLINE bool hasBroadcast32() const noexcept { return hasFlag(kFlagBroadcast32); }
    ASMJIT_INLINE bool hasBroadcast64() const noexcept { return hasFlag(kFlagBroadcast64); }
    ASMJIT_INLINE bool hasBroadcast() const noexcept { return hasFlag(kFlagBroadcast32 | kFlagBroadcast64); }

    uint32_t features;                   //!< CPU features.
    uint32_t flags;                      //!< Flags (AVX-512).
  };

  //! Instruction signature.
  //!
  //! Contains a sequence of operands' combinations and other metadata that defines
  //! a single instruction. This data is used by instruction validator.
  struct ISignature {
    uint8_t opCount           : 3;       //!< Count of operands in `opIndex` (0..6).
    uint8_t archMask          : 2;       //!< Architecture mask of this record.
    uint8_t implicit          : 3;       //!< Number of implicit operands.
    uint8_t reserved;                    //!< Reserved for future use.
    uint8_t operands[6];                 //!< Indexes to `OSignature` table.
  };

  //! Operand signature, used by \ref ISignature.
  //!
  //! Contains all possible operand combinations, memory size information,
  //! and register index (or \ref kInvalidReg if not mandatory).
  struct OSignature {
    uint32_t flags;                      //!< Operand flags.
    uint16_t memFlags;                   //!< Memory flags.
    uint8_t extFlags;                    //!< Extra flags.
    uint8_t regMask;                     //!< Mask of possible register IDs.
  };

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! Get instruction's name (null terminated).
  //!
  //! NOTE: If AsmJit was compiled with `ASMJIT_DISABLE_TEXT` then this will
  //! return an empty string (null terminated string of zero length).
  ASMJIT_INLINE const char* getName() const noexcept;
  //! Get index to `X86InstDB::nameData` of this instruction.
  //!
  //! NOTE: If AsmJit was compiled with `ASMJIT_DISABLE_TEXT` then this will
  //! always return zero.
  ASMJIT_INLINE uint32_t getNameDataIndex() const noexcept { return _nameDataIndex; }

  //! Get \ref CommonData of the instruction.
  ASMJIT_INLINE const CommonData& getCommonData() const noexcept;
  //! Get index to `X86InstDB::commonData` of this instruction.
  ASMJIT_INLINE uint32_t getCommonDataIndex() const noexcept { return _commonDataIndex; }

  //! Get instruction encoding, see \ref EncodingType.
  ASMJIT_INLINE uint32_t getEncodingType() const noexcept { return _encodingType; }

  //! Get instruction family, see \ref FamilyType.
  ASMJIT_INLINE uint32_t getFamilyType() const noexcept { return _familyType; }
  //! Get index to an instruction family dependent data.
  ASMJIT_INLINE uint32_t getFamilyDataIndex() const noexcept { return _familyDataIndex; }

  //! Get if the instruction's family is \ref kFamilyFpu.
  ASMJIT_INLINE bool isFpuFamily() const noexcept { return _familyType == kFamilyFpu; }
  //! Get if the instruction's family is \ref kFamilySse.
  ASMJIT_INLINE bool isSseFamily() const noexcept { return _familyType == kFamilySse; }
  //! Get if the instruction's family is \ref kFamilyAvx.
  ASMJIT_INLINE bool isAvxFamily() const noexcept { return _familyType == kFamilyAvx; }

  //! Get data specific to MMX/SSE instructions.
  //!
  //! NOTE: Always check the instruction family, it will assert if it's not an SSE instruction.
  ASMJIT_INLINE const SseData& getSseData() const noexcept;

  //! Get data specific to AVX instructions.
  //!
  //! NOTE: Always check the instruction family, it will assert if it's not an AVX instruction.
  ASMJIT_INLINE const AvxData& getAvxData() const noexcept;

  //! Get if the instruction has main opcode (rare, but it's possible it doesn't have).
  ASMJIT_INLINE bool hasMainOpCode() const noexcept { return _mainOpCode != 0; }
  //! Get main opcode, see \ref OpCodeBits.
  ASMJIT_INLINE uint32_t getMainOpCode() const noexcept { return _mainOpCode; }

  //! Get if the instruction has alternative opcode.
  ASMJIT_INLINE bool hasAltOpCode() const noexcept { return getCommonData().hasAltOpCode(); }
  //! Get alternative opcode, see \ref OpCodeBits.
  ASMJIT_INLINE uint32_t getAltOpCode() const noexcept { return getCommonData().getAltOpCode(); }

  //! Get whether the instruction has flag `flag`, see \ref InstFlags.
  ASMJIT_INLINE bool hasFlag(uint32_t flag) const noexcept { return getCommonData().hasFlag(flag); }
  //! Get instruction flags, see \ref InstFlags.
  ASMJIT_INLINE uint32_t getFlags() const noexcept { return getCommonData().getFlags(); }

  // --------------------------------------------------------------------------
  // [Get]
  // --------------------------------------------------------------------------

  //! Get if the `instId` is defined (counts also kInvalidInst, which is zero).
  static ASMJIT_INLINE bool isDefinedId(uint32_t instId) noexcept { return instId < _kIdCount; }

  //! Get instruction information based on the instruction `instId`.
  //!
  //! NOTE: `instId` has to be a valid instruction ID, it can't be greater than
  //! or equal to `X86Inst::_kIdCount`. It asserts in debug mode.
  static ASMJIT_INLINE const X86Inst& getInst(uint32_t instId) noexcept;

  // --------------------------------------------------------------------------
  // [Utilities]
  // --------------------------------------------------------------------------

  //! Get a 'kmov?' instruction by register `size`.
  static ASMJIT_INLINE uint32_t kmovIdFromSize(uint32_t size) noexcept {
    return size == 1 ? X86Inst::kIdKmovb :
           size == 2 ? X86Inst::kIdKmovw :
           size == 4 ? X86Inst::kIdKmovd : X86Inst::kIdKmovq;
  }

  // --------------------------------------------------------------------------
  // [Id <-> Name]
  // --------------------------------------------------------------------------

#if !defined(ASMJIT_DISABLE_TEXT)
  //! Get an instruction ID from a given instruction `name`.
  //!
  //! NOTE: Instruction name MUST BE in lowercase, otherwise there will be no
  //! match. If there is an exact match the instruction id is returned, otherwise
  //! `kInvalidInst` (zero) is returned instead. The given `name` doesn't have to
  //! be null-terminated if `len` is provided.
  ASMJIT_API static uint32_t getIdByName(const char* name, size_t len = kInvalidIndex) noexcept;

  //! Get an instruction name from a given instruction id `instId`.
  ASMJIT_API static const char* getNameById(uint32_t instId) noexcept;
#endif // !ASMJIT_DISABLE_TEXT

  // --------------------------------------------------------------------------
  // [Condition Codes]
  // --------------------------------------------------------------------------

  //! Convert a condition code into a condition code that reverses the
  //! corresponding operands of a comparison.
  static ASMJIT_INLINE uint32_t reverseCond(uint32_t cond) noexcept {
    ASMJIT_ASSERT(cond < ASMJIT_ARRAY_SIZE(_x86ReverseCond));
    return _x86ReverseCond[cond];
  }

  //! Get the equivalent of a negated condition code.
  static ASMJIT_INLINE uint32_t negateCond(uint32_t cond) noexcept {
    ASMJIT_ASSERT(cond < ASMJIT_ARRAY_SIZE(_x86ReverseCond));
    return cond ^ static_cast<uint32_t>(cond < X86Inst::kCondNone);
  }

  //! Translate a condition code `cc` to a "cmovcc" instruction id.
  static ASMJIT_INLINE uint32_t condToCmovcc(uint32_t cond) noexcept {
    ASMJIT_ASSERT(static_cast<uint32_t>(cond) < ASMJIT_ARRAY_SIZE(_x86CondToCmovcc));
    return _x86CondToCmovcc[cond];
  }

  //! Translate a condition code `cc` to a "jcc" instruction id.
  static ASMJIT_INLINE uint32_t condToJcc(uint32_t cond) noexcept {
    ASMJIT_ASSERT(static_cast<uint32_t>(cond) < ASMJIT_ARRAY_SIZE(_x86CondToJcc));
    return _x86CondToJcc[cond];
  }

  //! Translate a condition code `cc` to a "setcc" instruction id.
  static ASMJIT_INLINE uint32_t condToSetcc(uint32_t cond) noexcept {
    ASMJIT_ASSERT(static_cast<uint32_t>(cond) < ASMJIT_ARRAY_SIZE(_x86CondToSetcc));
    return _x86CondToSetcc[cond];
  }

  // --------------------------------------------------------------------------
  // [Validation]
  // --------------------------------------------------------------------------

#if !defined(ASMJIT_DISABLE_VALIDATION)
  ASMJIT_API static Error validate(
    uint32_t archType, uint32_t instId, uint32_t options,
    const Operand_& opExtra,
    const Operand_* opArray, uint32_t opCount) noexcept;
#endif // !ASMJIT_DISABLE_VALIDATION

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  uint32_t _encodingType    : 8;         //!< Instruction encoding.
  uint32_t _nameDataIndex   : 14;        //!< Index to `X86InstDB::nameData` table.
  uint32_t _commonDataIndex : 10;        //!< Index to `X86InstDB::commonData` table.
  uint32_t _familyType      : 2;         //!< Instruction family type.
  uint32_t _familyDataIndex : 8;         //!< Index to `fpuData`, `sseData`, or `avxData`.
  uint32_t _reserved        : 22;
  uint32_t _mainOpCode;                  //!< Instruction's primary opcode.
};

//! X86 instruction data under a single namespace.
struct X86InstDB {
  ASMJIT_API static const X86Inst instData[];
  ASMJIT_API static const X86Inst::CommonData commonData[];
  ASMJIT_API static const X86Inst::FpuData fpuData[];
  ASMJIT_API static const X86Inst::SseData sseData[];
  ASMJIT_API static const X86Inst::AvxData avxData[];
  ASMJIT_API static const uint32_t altOpCodeData[];
  ASMJIT_API static const char nameData[];
};

ASMJIT_INLINE const X86Inst& X86Inst::getInst(uint32_t instId) noexcept {
  ASMJIT_ASSERT(instId < X86Inst::_kIdCount);
  return X86InstDB::instData[instId];
}

ASMJIT_INLINE const char* X86Inst::getName() const noexcept {
  return &X86InstDB::nameData[_nameDataIndex];
}

ASMJIT_INLINE const X86Inst::CommonData& X86Inst::getCommonData() const noexcept {
  return X86InstDB::commonData[_commonDataIndex];
}

ASMJIT_INLINE const X86Inst::SseData& X86Inst::getSseData() const noexcept {
  ASMJIT_ASSERT(isSseFamily());
  return X86InstDB::sseData[_familyDataIndex];
}

ASMJIT_INLINE const X86Inst::AvxData& X86Inst::getAvxData() const noexcept {
  ASMJIT_ASSERT(isAvxFamily());
  return X86InstDB::avxData[_familyDataIndex];
}

ASMJIT_INLINE uint32_t X86Inst::CommonData::getAltOpCode() const noexcept {
  return X86InstDB::altOpCodeData[_altOpCodeIndex];
}

// ============================================================================
// [asmjit::X86Util]
// ============================================================================

struct X86Util {
  //! Pack a shuffle constant to be used with SSE/AVX instruction (2 values).
  //!
  //! \param a Position of the first  component [0, 1].
  //! \param b Position of the second component [0, 1].
  //!
  //! Shuffle constants can be used to encode an immediate for these instructions:
  //!   - `shufpd`
  static ASMJIT_INLINE int shuffle(uint32_t a, uint32_t b) noexcept {
    ASMJIT_ASSERT(a <= 0x1 && b <= 0x1);
    uint32_t result = (a << 1) | b;
    return static_cast<int>(result);
  }

  //! Pack a shuffle constant to be used with SSE/AVX instruction (4 values).
  //!
  //! \param a Position of the first  component [0, 3].
  //! \param b Position of the second component [0, 3].
  //! \param c Position of the third  component [0, 3].
  //! \param d Position of the fourth component [0, 3].
  //!
  //! Shuffle constants can be used to encode an immediate for these instructions:
  //!   - `pshufw()`
  //!   - `pshufd()`
  //!   - `pshuflw()`
  //!   - `pshufhw()`
  //!   - `shufps()`
  static ASMJIT_INLINE int shuffle(uint32_t a, uint32_t b, uint32_t c, uint32_t d) noexcept {
    ASMJIT_ASSERT(a <= 0x3 && b <= 0x3 && c <= 0x3 && d <= 0x3);
    uint32_t result = (a << 6) | (b << 4) | (c << 2) | d;
    return static_cast<int>(result);
  }
};

//! \}

} // asmjit namespace

#undef _OP_ID

// [Api-End]
#include "../asmjit_apiend.h"

// [Guard]
#endif // _ASMJIT_X86_X86INST_H
