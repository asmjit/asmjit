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
#include "../base/vectypes.h"

// [Api-Begin]
#include "../apibegin.h"

namespace asmjit {

// ============================================================================
// [Forward Declarations]
// ============================================================================

struct X86InstInfo;
struct X86InstExtendedInfo;

//! \addtogroup asmjit_x86
//! \{

// ============================================================================
// [asmjit::X86Inst/X86Cond - Globals]
// ============================================================================

//! \internal
//!
//! X86/X64 instructions' extended information, accessible through `X86InstInfo`.
ASMJIT_VARAPI const X86InstExtendedInfo _x86InstExtendedInfo[];

//! \internal
//!
//! X86/X64 instructions' information.
ASMJIT_VARAPI const X86InstInfo _x86InstInfo[];

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
// [asmjit::X86InstId]
// ============================================================================

//! X86/X64 instruction IDs.
//!
//! Note that these instruction codes are AsmJit specific. Each instruction has
//! a unique ID that is used as an index to AsmJit instruction table. The list
//! is sorted alphabetically except instructions starting with `j`, because the
//! `jcc` instruction is composition of an opcode and condition code. It means
//! that these instructions are sorted as `jcc`, `jecxz` and `jmp`. Please use
//! \ref X86Util::getInstIdByName() if you need instruction name to ID mapping
//! and are not aware on how to handle such case.
ASMJIT_ENUM(X86InstId) {
  kX86InstIdNone = 0,
  kX86InstIdAdc,                         // X86/X64
  kX86InstIdAdcx,                        // ADX
  kX86InstIdAdd,                         // X86/X64
  kX86InstIdAddpd,                       // SSE2
  kX86InstIdAddps,                       // SSE
  kX86InstIdAddsd,                       // SSE2
  kX86InstIdAddss,                       // SSE
  kX86InstIdAddsubpd,                    // SSE3
  kX86InstIdAddsubps,                    // SSE3
  kX86InstIdAdox,                        // ADX
  kX86InstIdAesdec,                      // AESNI
  kX86InstIdAesdeclast,                  // AESNI
  kX86InstIdAesenc,                      // AESNI
  kX86InstIdAesenclast,                  // AESNI
  kX86InstIdAesimc,                      // AESNI
  kX86InstIdAeskeygenassist,             // AESNI
  kX86InstIdAnd,                         // X86/X64
  kX86InstIdAndn,                        // BMI
  kX86InstIdAndnpd,                      // SSE2
  kX86InstIdAndnps,                      // SSE
  kX86InstIdAndpd,                       // SSE2
  kX86InstIdAndps,                       // SSE
  kX86InstIdBextr,                       // BMI
  kX86InstIdBlcfill,                     // TBM
  kX86InstIdBlci,                        // TBM
  kX86InstIdBlcic,                       // TBM
  kX86InstIdBlcmsk,                      // TBM
  kX86InstIdBlcs,                        // TBM
  kX86InstIdBlendpd,                     // SSE4.1
  kX86InstIdBlendps,                     // SSE4.1
  kX86InstIdBlendvpd,                    // SSE4.1
  kX86InstIdBlendvps,                    // SSE4.1
  kX86InstIdBlsfill,                     // TBM
  kX86InstIdBlsi,                        // BMI
  kX86InstIdBlsic,                       // TBM
  kX86InstIdBlsmsk,                      // BMI
  kX86InstIdBlsr,                        // BMI
  kX86InstIdBsf,                         // X86/X64
  kX86InstIdBsr,                         // X86/X64
  kX86InstIdBswap,                       // X86/X64 (i486+)
  kX86InstIdBt,                          // X86/X64
  kX86InstIdBtc,                         // X86/X64
  kX86InstIdBtr,                         // X86/X64
  kX86InstIdBts,                         // X86/X64
  kX86InstIdBzhi,                        // BMI2
  kX86InstIdCall,                        // X86/X64
  kX86InstIdCbw,                         // X86/X64
  kX86InstIdCdq,                         // X86/X64
  kX86InstIdCdqe,                        // X64 only
  kX86InstIdClc,                         // X86/X64
  kX86InstIdCld,                         // X86/X64
  kX86InstIdClflush,                     // CLFLUSH
  kX86InstIdClflushopt,                  // CLFLUSH_OPT
  kX86InstIdCmc,                         // X86/X64
  kX86InstIdCmova,                       // X86/X64 (cmovcc) (i586+)
  kX86InstIdCmovae,                      // X86/X64 (cmovcc) (i586+)
  kX86InstIdCmovb,                       // X86/X64 (cmovcc) (i586+)
  kX86InstIdCmovbe,                      // X86/X64 (cmovcc) (i586+)
  kX86InstIdCmovc,                       // X86/X64 (cmovcc) (i586+)
  kX86InstIdCmove,                       // X86/X64 (cmovcc) (i586+)
  kX86InstIdCmovg,                       // X86/X64 (cmovcc) (i586+)
  kX86InstIdCmovge,                      // X86/X64 (cmovcc) (i586+)
  kX86InstIdCmovl,                       // X86/X64 (cmovcc) (i586+)
  kX86InstIdCmovle,                      // X86/X64 (cmovcc) (i586+)
  kX86InstIdCmovna,                      // X86/X64 (cmovcc) (i586+)
  kX86InstIdCmovnae,                     // X86/X64 (cmovcc) (i586+)
  kX86InstIdCmovnb,                      // X86/X64 (cmovcc) (i586+)
  kX86InstIdCmovnbe,                     // X86/X64 (cmovcc) (i586+)
  kX86InstIdCmovnc,                      // X86/X64 (cmovcc) (i586+)
  kX86InstIdCmovne,                      // X86/X64 (cmovcc) (i586+)
  kX86InstIdCmovng,                      // X86/X64 (cmovcc) (i586+)
  kX86InstIdCmovnge,                     // X86/X64 (cmovcc) (i586+)
  kX86InstIdCmovnl,                      // X86/X64 (cmovcc) (i586+)
  kX86InstIdCmovnle,                     // X86/X64 (cmovcc) (i586+)
  kX86InstIdCmovno,                      // X86/X64 (cmovcc) (i586+)
  kX86InstIdCmovnp,                      // X86/X64 (cmovcc) (i586+)
  kX86InstIdCmovns,                      // X86/X64 (cmovcc) (i586+)
  kX86InstIdCmovnz,                      // X86/X64 (cmovcc) (i586+)
  kX86InstIdCmovo,                       // X86/X64 (cmovcc) (i586+)
  kX86InstIdCmovp,                       // X86/X64 (cmovcc) (i586+)
  kX86InstIdCmovpe,                      // X86/X64 (cmovcc) (i586+)
  kX86InstIdCmovpo,                      // X86/X64 (cmovcc) (i586+)
  kX86InstIdCmovs,                       // X86/X64 (cmovcc) (i586+)
  kX86InstIdCmovz,                       // X86/X64 (cmovcc) (i586+)
  kX86InstIdCmp,                         // X86/X64
  kX86InstIdCmppd,                       // SSE2
  kX86InstIdCmpps,                       // SSE
  kX86InstIdCmpsB,                       // CMPS
  kX86InstIdCmpsD,                       // CMPS
  kX86InstIdCmpsQ,                       // CMPS (X64)
  kX86InstIdCmpsW,                       // CMPS
  kX86InstIdCmpsd,                       // SSE2
  kX86InstIdCmpss,                       // SSE
  kX86InstIdCmpxchg,                     // X86/X64 (i486+)
  kX86InstIdCmpxchg16b,                  // X64 only
  kX86InstIdCmpxchg8b,                   // X86/X64 (i586+)
  kX86InstIdComisd,                      // SSE2
  kX86InstIdComiss,                      // SSE
  kX86InstIdCpuid,                       // X86/X64 (i486/i586+)
  kX86InstIdCqo,                         // X64 only
  kX86InstIdCrc32,                       // SSE4.2
  kX86InstIdCvtdq2pd,                    // SSE2
  kX86InstIdCvtdq2ps,                    // SSE2
  kX86InstIdCvtpd2dq,                    // SSE2
  kX86InstIdCvtpd2pi,                    // SSE2
  kX86InstIdCvtpd2ps,                    // SSE2
  kX86InstIdCvtpi2pd,                    // SSE2
  kX86InstIdCvtpi2ps,                    // SSE
  kX86InstIdCvtps2dq,                    // SSE2
  kX86InstIdCvtps2pd,                    // SSE2
  kX86InstIdCvtps2pi,                    // SSE
  kX86InstIdCvtsd2si,                    // SSE2
  kX86InstIdCvtsd2ss,                    // SSE2
  kX86InstIdCvtsi2sd,                    // SSE2
  kX86InstIdCvtsi2ss,                    // SSE
  kX86InstIdCvtss2sd,                    // SSE2
  kX86InstIdCvtss2si,                    // SSE
  kX86InstIdCvttpd2dq,                   // SSE2
  kX86InstIdCvttpd2pi,                   // SSE2
  kX86InstIdCvttps2dq,                   // SSE2
  kX86InstIdCvttps2pi,                   // SSE
  kX86InstIdCvttsd2si,                   // SSE2
  kX86InstIdCvttss2si,                   // SSE
  kX86InstIdCwd,                         // X86/X64
  kX86InstIdCwde,                        // X86/X64
  kX86InstIdDaa,                         // X86 only
  kX86InstIdDas,                         // X86 only
  kX86InstIdDec,                         // X86/X64
  kX86InstIdDiv,                         // X86/X64
  kX86InstIdDivpd,                       // SSE2
  kX86InstIdDivps,                       // SSE
  kX86InstIdDivsd,                       // SSE2
  kX86InstIdDivss,                       // SSE
  kX86InstIdDppd,                        // SSE4.1
  kX86InstIdDpps,                        // SSE4.1
  kX86InstIdEmms,                        // MMX
  kX86InstIdEnter,                       // X86/X64
  kX86InstIdExtractps,                   // SSE4.1
  kX86InstIdExtrq,                       // SSE4a
  kX86InstIdF2xm1,                       // FPU
  kX86InstIdFabs,                        // FPU
  kX86InstIdFadd,                        // FPU
  kX86InstIdFaddp,                       // FPU
  kX86InstIdFbld,                        // FPU
  kX86InstIdFbstp,                       // FPU
  kX86InstIdFchs,                        // FPU
  kX86InstIdFclex,                       // FPU
  kX86InstIdFcmovb,                      // FPU
  kX86InstIdFcmovbe,                     // FPU
  kX86InstIdFcmove,                      // FPU
  kX86InstIdFcmovnb,                     // FPU
  kX86InstIdFcmovnbe,                    // FPU
  kX86InstIdFcmovne,                     // FPU
  kX86InstIdFcmovnu,                     // FPU
  kX86InstIdFcmovu,                      // FPU
  kX86InstIdFcom,                        // FPU
  kX86InstIdFcomi,                       // FPU
  kX86InstIdFcomip,                      // FPU
  kX86InstIdFcomp,                       // FPU
  kX86InstIdFcompp,                      // FPU
  kX86InstIdFcos,                        // FPU
  kX86InstIdFdecstp,                     // FPU
  kX86InstIdFdiv,                        // FPU
  kX86InstIdFdivp,                       // FPU
  kX86InstIdFdivr,                       // FPU
  kX86InstIdFdivrp,                      // FPU
  kX86InstIdFemms,                       // 3DNOW
  kX86InstIdFfree,                       // FPU
  kX86InstIdFiadd,                       // FPU
  kX86InstIdFicom,                       // FPU
  kX86InstIdFicomp,                      // FPU
  kX86InstIdFidiv,                       // FPU
  kX86InstIdFidivr,                      // FPU
  kX86InstIdFild,                        // FPU
  kX86InstIdFimul,                       // FPU
  kX86InstIdFincstp,                     // FPU
  kX86InstIdFinit,                       // FPU
  kX86InstIdFist,                        // FPU
  kX86InstIdFistp,                       // FPU
  kX86InstIdFisttp,                      // SSE3
  kX86InstIdFisub,                       // FPU
  kX86InstIdFisubr,                      // FPU
  kX86InstIdFld,                         // FPU
  kX86InstIdFld1,                        // FPU
  kX86InstIdFldcw,                       // FPU
  kX86InstIdFldenv,                      // FPU
  kX86InstIdFldl2e,                      // FPU
  kX86InstIdFldl2t,                      // FPU
  kX86InstIdFldlg2,                      // FPU
  kX86InstIdFldln2,                      // FPU
  kX86InstIdFldpi,                       // FPU
  kX86InstIdFldz,                        // FPU
  kX86InstIdFmul,                        // FPU
  kX86InstIdFmulp,                       // FPU
  kX86InstIdFnclex,                      // FPU
  kX86InstIdFninit,                      // FPU
  kX86InstIdFnop,                        // FPU
  kX86InstIdFnsave,                      // FPU
  kX86InstIdFnstcw,                      // FPU
  kX86InstIdFnstenv,                     // FPU
  kX86InstIdFnstsw,                      // FPU
  kX86InstIdFpatan,                      // FPU
  kX86InstIdFprem,                       // FPU
  kX86InstIdFprem1,                      // FPU
  kX86InstIdFptan,                       // FPU
  kX86InstIdFrndint,                     // FPU
  kX86InstIdFrstor,                      // FPU
  kX86InstIdFsave,                       // FPU
  kX86InstIdFscale,                      // FPU
  kX86InstIdFsin,                        // FPU
  kX86InstIdFsincos,                     // FPU
  kX86InstIdFsqrt,                       // FPU
  kX86InstIdFst,                         // FPU
  kX86InstIdFstcw,                       // FPU
  kX86InstIdFstenv,                      // FPU
  kX86InstIdFstp,                        // FPU
  kX86InstIdFstsw,                       // FPU
  kX86InstIdFsub,                        // FPU
  kX86InstIdFsubp,                       // FPU
  kX86InstIdFsubr,                       // FPU
  kX86InstIdFsubrp,                      // FPU
  kX86InstIdFtst,                        // FPU
  kX86InstIdFucom,                       // FPU
  kX86InstIdFucomi,                      // FPU
  kX86InstIdFucomip,                     // FPU
  kX86InstIdFucomp,                      // FPU
  kX86InstIdFucompp,                     // FPU
  kX86InstIdFwait,                       // FPU
  kX86InstIdFxam,                        // FPU
  kX86InstIdFxch,                        // FPU
  kX86InstIdFxrstor,                     // FPU
  kX86InstIdFxrstor64,                   // FPU (X64)
  kX86InstIdFxsave,                      // FPU
  kX86InstIdFxsave64,                    // FPU (X64)
  kX86InstIdFxtract,                     // FPU
  kX86InstIdFyl2x,                       // FPU
  kX86InstIdFyl2xp1,                     // FPU
  kX86InstIdHaddpd,                      // SSE3
  kX86InstIdHaddps,                      // SSE3
  kX86InstIdHsubpd,                      // SSE3
  kX86InstIdHsubps,                      // SSE3
  kX86InstIdIdiv,                        // X86/X64
  kX86InstIdImul,                        // X86/X64
  kX86InstIdInc,                         // X86/X64
  kX86InstIdInsertps,                    // SSE4.1
  kX86InstIdInsertq,                     // SSE4a
  kX86InstIdInt,                         // X86/X64
  kX86InstIdJa,                          // X86/X64 (jcc)
  kX86InstIdJae,                         // X86/X64 (jcc)
  kX86InstIdJb,                          // X86/X64 (jcc)
  kX86InstIdJbe,                         // X86/X64 (jcc)
  kX86InstIdJc,                          // X86/X64 (jcc)
  kX86InstIdJe,                          // X86/X64 (jcc)
  kX86InstIdJg,                          // X86/X64 (jcc)
  kX86InstIdJge,                         // X86/X64 (jcc)
  kX86InstIdJl,                          // X86/X64 (jcc)
  kX86InstIdJle,                         // X86/X64 (jcc)
  kX86InstIdJna,                         // X86/X64 (jcc)
  kX86InstIdJnae,                        // X86/X64 (jcc)
  kX86InstIdJnb,                         // X86/X64 (jcc)
  kX86InstIdJnbe,                        // X86/X64 (jcc)
  kX86InstIdJnc,                         // X86/X64 (jcc)
  kX86InstIdJne,                         // X86/X64 (jcc)
  kX86InstIdJng,                         // X86/X64 (jcc)
  kX86InstIdJnge,                        // X86/X64 (jcc)
  kX86InstIdJnl,                         // X86/X64 (jcc)
  kX86InstIdJnle,                        // X86/X64 (jcc)
  kX86InstIdJno,                         // X86/X64 (jcc)
  kX86InstIdJnp,                         // X86/X64 (jcc)
  kX86InstIdJns,                         // X86/X64 (jcc)
  kX86InstIdJnz,                         // X86/X64 (jcc)
  kX86InstIdJo,                          // X86/X64 (jcc)
  kX86InstIdJp,                          // X86/X64 (jcc)
  kX86InstIdJpe,                         // X86/X64 (jcc)
  kX86InstIdJpo,                         // X86/X64 (jcc)
  kX86InstIdJs,                          // X86/X64 (jcc)
  kX86InstIdJz,                          // X86/X64 (jcc)
  kX86InstIdJecxz,                       // X86/X64 (jcxz/jecxz/jrcxz)
  kX86InstIdJmp,                         // X86/X64 (jmp)
  kX86InstIdLahf,                        // X86/X64 (LAHF/SAHF)
  kX86InstIdLddqu,                       // SSE3
  kX86InstIdLdmxcsr,                     // SSE
  kX86InstIdLea,                         // X86/X64
  kX86InstIdLeave,                       // X86/X64
  kX86InstIdLfence,                      // SSE2
  kX86InstIdLodsB,                       // LODS
  kX86InstIdLodsD,                       // LODS
  kX86InstIdLodsQ,                       // LODS (X64)
  kX86InstIdLodsW,                       // LODS
  kX86InstIdLzcnt,                       // LZCNT
  kX86InstIdMaskmovdqu,                  // SSE2
  kX86InstIdMaskmovq,                    // MMX2
  kX86InstIdMaxpd,                       // SSE2
  kX86InstIdMaxps,                       // SSE
  kX86InstIdMaxsd,                       // SSE2
  kX86InstIdMaxss,                       // SSE
  kX86InstIdMfence,                      // SSE2
  kX86InstIdMinpd,                       // SSE2
  kX86InstIdMinps,                       // SSE
  kX86InstIdMinsd,                       // SSE2
  kX86InstIdMinss,                       // SSE
  kX86InstIdMonitor,                     // SSE3
  kX86InstIdMov,                         // X86/X64
  kX86InstIdMovPtr,                      // X86/X64
  kX86InstIdMovapd,                      // SSE2
  kX86InstIdMovaps,                      // SSE
  kX86InstIdMovbe,                       // SSE3 (Atom)
  kX86InstIdMovd,                        // MMX/SSE2
  kX86InstIdMovddup,                     // SSE3
  kX86InstIdMovdq2q,                     // SSE2
  kX86InstIdMovdqa,                      // SSE2
  kX86InstIdMovdqu,                      // SSE2
  kX86InstIdMovhlps,                     // SSE
  kX86InstIdMovhpd,                      // SSE2
  kX86InstIdMovhps,                      // SSE
  kX86InstIdMovlhps,                     // SSE
  kX86InstIdMovlpd,                      // SSE2
  kX86InstIdMovlps,                      // SSE
  kX86InstIdMovmskpd,                    // SSE2
  kX86InstIdMovmskps,                    // SSE2
  kX86InstIdMovntdq,                     // SSE2
  kX86InstIdMovntdqa,                    // SSE4.1
  kX86InstIdMovnti,                      // SSE2
  kX86InstIdMovntpd,                     // SSE2
  kX86InstIdMovntps,                     // SSE
  kX86InstIdMovntq,                      // MMX2
  kX86InstIdMovntsd,                     // SSE4a
  kX86InstIdMovntss,                     // SSE4a
  kX86InstIdMovq,                        // MMX/SSE/SSE2
  kX86InstIdMovq2dq,                     // SSE2
  kX86InstIdMovsB,                       // MOVS
  kX86InstIdMovsD,                       // MOVS
  kX86InstIdMovsQ,                       // MOVS (X64)
  kX86InstIdMovsW,                       // MOVS
  kX86InstIdMovsd,                       // SSE2
  kX86InstIdMovshdup,                    // SSE3
  kX86InstIdMovsldup,                    // SSE3
  kX86InstIdMovss,                       // SSE
  kX86InstIdMovsx,                       // X86/X64
  kX86InstIdMovsxd,                      // X86/X64
  kX86InstIdMovupd,                      // SSE2
  kX86InstIdMovups,                      // SSE
  kX86InstIdMovzx,                       // X86/X64
  kX86InstIdMpsadbw,                     // SSE4.1
  kX86InstIdMul,                         // X86/X64
  kX86InstIdMulpd,                       // SSE2
  kX86InstIdMulps,                       // SSE
  kX86InstIdMulsd,                       // SSE2
  kX86InstIdMulss,                       // SSE
  kX86InstIdMulx,                        // BMI2
  kX86InstIdMwait,                       // SSE3
  kX86InstIdNeg,                         // X86/X64
  kX86InstIdNop,                         // X86/X64
  kX86InstIdNot,                         // X86/X64
  kX86InstIdOr,                          // X86/X64
  kX86InstIdOrpd,                        // SSE2
  kX86InstIdOrps,                        // SSE
  kX86InstIdPabsb,                       // SSSE3
  kX86InstIdPabsd,                       // SSSE3
  kX86InstIdPabsw,                       // SSSE3
  kX86InstIdPackssdw,                    // MMX/SSE2
  kX86InstIdPacksswb,                    // MMX/SSE2
  kX86InstIdPackusdw,                    // SSE4.1
  kX86InstIdPackuswb,                    // MMX/SSE2
  kX86InstIdPaddb,                       // MMX/SSE2
  kX86InstIdPaddd,                       // MMX/SSE2
  kX86InstIdPaddq,                       // SSE2
  kX86InstIdPaddsb,                      // MMX/SSE2
  kX86InstIdPaddsw,                      // MMX/SSE2
  kX86InstIdPaddusb,                     // MMX/SSE2
  kX86InstIdPaddusw,                     // MMX/SSE2
  kX86InstIdPaddw,                       // MMX/SSE2
  kX86InstIdPalignr,                     // SSSE3
  kX86InstIdPand,                        // MMX/SSE2
  kX86InstIdPandn,                       // MMX/SSE2
  kX86InstIdPause,                       // SSE2.
  kX86InstIdPavgb,                       // MMX2
  kX86InstIdPavgusb,                     // 3DNOW
  kX86InstIdPavgw,                       // MMX2
  kX86InstIdPblendvb,                    // SSE4.1
  kX86InstIdPblendw,                     // SSE4.1
  kX86InstIdPclmulqdq,                   // PCLMULQDQ
  kX86InstIdPcmpeqb,                     // MMX/SSE2
  kX86InstIdPcmpeqd,                     // MMX/SSE2
  kX86InstIdPcmpeqq,                     // SSE4.1
  kX86InstIdPcmpeqw,                     // MMX/SSE2
  kX86InstIdPcmpestri,                   // SSE4.2
  kX86InstIdPcmpestrm,                   // SSE4.2
  kX86InstIdPcmpgtb,                     // MMX/SSE2
  kX86InstIdPcmpgtd,                     // MMX/SSE2
  kX86InstIdPcmpgtq,                     // SSE4.2
  kX86InstIdPcmpgtw,                     // MMX/SSE2
  kX86InstIdPcmpistri,                   // SSE4.2
  kX86InstIdPcmpistrm,                   // SSE4.2
  kX86InstIdPdep,                        // BMI2
  kX86InstIdPext,                        // BMI2
  kX86InstIdPextrb,                      // SSE4.1
  kX86InstIdPextrd,                      // SSE4.1
  kX86InstIdPextrq,                      // SSE4.1
  kX86InstIdPextrw,                      // MMX2/SSE2
  kX86InstIdPf2id,                       // 3DNOW
  kX86InstIdPf2iw,                       // 3DNOW2
  kX86InstIdPfacc,                       // 3DNOW
  kX86InstIdPfadd,                       // 3DNOW
  kX86InstIdPfcmpeq,                     // 3DNOW
  kX86InstIdPfcmpge,                     // 3DNOW
  kX86InstIdPfcmpgt,                     // 3DNOW
  kX86InstIdPfmax,                       // 3DNOW
  kX86InstIdPfmin,                       // 3DNOW
  kX86InstIdPfmul,                       // 3DNOW
  kX86InstIdPfnacc,                      // 3DNOW2
  kX86InstIdPfpnacc,                     // 3DNOW2
  kX86InstIdPfrcp,                       // 3DNOW
  kX86InstIdPfrcpit1,                    // 3DNOW
  kX86InstIdPfrcpit2,                    // 3DNOW
  kX86InstIdPfrsqit1,                    // 3DNOW
  kX86InstIdPfrsqrt,                     // 3DNOW
  kX86InstIdPfsub,                       // 3DNOW
  kX86InstIdPfsubr,                      // 3DNOW
  kX86InstIdPhaddd,                      // SSSE3
  kX86InstIdPhaddsw,                     // SSSE3
  kX86InstIdPhaddw,                      // SSSE3
  kX86InstIdPhminposuw,                  // SSE4.1
  kX86InstIdPhsubd,                      // SSSE3
  kX86InstIdPhsubsw,                     // SSSE3
  kX86InstIdPhsubw,                      // SSSE3
  kX86InstIdPi2fd,                       // 3DNOW
  kX86InstIdPi2fw,                       // 3DNOW2
  kX86InstIdPinsrb,                      // SSE4.1
  kX86InstIdPinsrd,                      // SSE4.1
  kX86InstIdPinsrq,                      // SSE4.1
  kX86InstIdPinsrw,                      // MMX2
  kX86InstIdPmaddubsw,                   // SSSE3
  kX86InstIdPmaddwd,                     // MMX/SSE2
  kX86InstIdPmaxsb,                      // SSE4.1
  kX86InstIdPmaxsd,                      // SSE4.1
  kX86InstIdPmaxsw,                      // MMX2
  kX86InstIdPmaxub,                      // MMX2
  kX86InstIdPmaxud,                      // SSE4.1
  kX86InstIdPmaxuw,                      // SSE4.1
  kX86InstIdPminsb,                      // SSE4.1
  kX86InstIdPminsd,                      // SSE4.1
  kX86InstIdPminsw,                      // MMX2
  kX86InstIdPminub,                      // MMX2
  kX86InstIdPminud,                      // SSE4.1
  kX86InstIdPminuw,                      // SSE4.1
  kX86InstIdPmovmskb,                    // MMX2
  kX86InstIdPmovsxbd,                    // SSE4.1
  kX86InstIdPmovsxbq,                    // SSE4.1
  kX86InstIdPmovsxbw,                    // SSE4.1
  kX86InstIdPmovsxdq,                    // SSE4.1
  kX86InstIdPmovsxwd,                    // SSE4.1
  kX86InstIdPmovsxwq,                    // SSE4.1
  kX86InstIdPmovzxbd,                    // SSE4.1
  kX86InstIdPmovzxbq,                    // SSE4.1
  kX86InstIdPmovzxbw,                    // SSE4.1
  kX86InstIdPmovzxdq,                    // SSE4.1
  kX86InstIdPmovzxwd,                    // SSE4.1
  kX86InstIdPmovzxwq,                    // SSE4.1
  kX86InstIdPmuldq,                      // SSE4.1
  kX86InstIdPmulhrsw,                    // SSSE3
  kX86InstIdPmulhrw,                     // 3DNOW
  kX86InstIdPmulhuw,                     // MMX2
  kX86InstIdPmulhw,                      // MMX/SSE2
  kX86InstIdPmulld,                      // SSE4.1
  kX86InstIdPmullw,                      // MMX/SSE2
  kX86InstIdPmuludq,                     // SSE2
  kX86InstIdPop,                         // X86/X64
  kX86InstIdPopa,                        // X86 only
  kX86InstIdPopcnt,                      // SSE4.2
  kX86InstIdPopf,                        // X86/X64
  kX86InstIdPor,                         // MMX/SSE2
  kX86InstIdPrefetch,                    // MMX2/SSE
  kX86InstIdPrefetch3dNow,               // 3DNOW
  kX86InstIdPrefetchw,                   // PREFETCHW
  kX86InstIdPrefetchwt1,                 // PREFETCHWT1
  kX86InstIdPsadbw,                      // MMX2
  kX86InstIdPshufb,                      // SSSE3
  kX86InstIdPshufd,                      // SSE2
  kX86InstIdPshufhw,                     // SSE2
  kX86InstIdPshuflw,                     // SSE2
  kX86InstIdPshufw,                      // MMX2
  kX86InstIdPsignb,                      // SSSE3
  kX86InstIdPsignd,                      // SSSE3
  kX86InstIdPsignw,                      // SSSE3
  kX86InstIdPslld,                       // MMX/SSE2
  kX86InstIdPslldq,                      // SSE2
  kX86InstIdPsllq,                       // MMX/SSE2
  kX86InstIdPsllw,                       // MMX/SSE2
  kX86InstIdPsrad,                       // MMX/SSE2
  kX86InstIdPsraw,                       // MMX/SSE2
  kX86InstIdPsrld,                       // MMX/SSE2
  kX86InstIdPsrldq,                      // SSE2
  kX86InstIdPsrlq,                       // MMX/SSE2
  kX86InstIdPsrlw,                       // MMX/SSE2
  kX86InstIdPsubb,                       // MMX/SSE2
  kX86InstIdPsubd,                       // MMX/SSE2
  kX86InstIdPsubq,                       // SSE2
  kX86InstIdPsubsb,                      // MMX/SSE2
  kX86InstIdPsubsw,                      // MMX/SSE2
  kX86InstIdPsubusb,                     // MMX/SSE2
  kX86InstIdPsubusw,                     // MMX/SSE2
  kX86InstIdPsubw,                       // MMX/SSE2
  kX86InstIdPswapd,                      // 3DNOW2
  kX86InstIdPtest,                       // SSE4.1
  kX86InstIdPunpckhbw,                   // MMX/SSE2
  kX86InstIdPunpckhdq,                   // MMX/SSE2
  kX86InstIdPunpckhqdq,                  // SSE2
  kX86InstIdPunpckhwd,                   // MMX/SSE2
  kX86InstIdPunpcklbw,                   // MMX/SSE2
  kX86InstIdPunpckldq,                   // MMX/SSE2
  kX86InstIdPunpcklqdq,                  // SSE2
  kX86InstIdPunpcklwd,                   // MMX/SSE2
  kX86InstIdPush,                        // X86/X64
  kX86InstIdPusha,                       // X86 only
  kX86InstIdPushf,                       // X86/X64
  kX86InstIdPxor,                        // MMX/SSE2
  kX86InstIdRcl,                         // X86/X64
  kX86InstIdRcpps,                       // SSE
  kX86InstIdRcpss,                       // SSE
  kX86InstIdRcr,                         // X86/X64
  kX86InstIdRdfsbase,                    // FSGSBASE (X64)
  kX86InstIdRdgsbase,                    // FSGSBASE (X64)
  kX86InstIdRdrand,                      // RDRAND (RDRAND)
  kX86InstIdRdseed,                      // RDSEED (RDSEED)
  kX86InstIdRdtsc,                       // X86/X64
  kX86InstIdRdtscp,                      // X86/X64
  kX86InstIdRepLodsB,                    // X86/X64 (REP)
  kX86InstIdRepLodsD,                    // X86/X64 (REP)
  kX86InstIdRepLodsQ,                    // X64 only (REP)
  kX86InstIdRepLodsW,                    // X86/X64 (REP)
  kX86InstIdRepMovsB,                    // X86/X64 (REP)
  kX86InstIdRepMovsD,                    // X86/X64 (REP)
  kX86InstIdRepMovsQ,                    // X64 only (REP)
  kX86InstIdRepMovsW,                    // X86/X64 (REP)
  kX86InstIdRepStosB,                    // X86/X64 (REP)
  kX86InstIdRepStosD,                    // X86/X64 (REP)
  kX86InstIdRepStosQ,                    // X64 only (REP)
  kX86InstIdRepStosW,                    // X86/X64 (REP)
  kX86InstIdRepeCmpsB,                   // X86/X64 (REP)
  kX86InstIdRepeCmpsD,                   // X86/X64 (REP)
  kX86InstIdRepeCmpsQ,                   // X64 only (REP)
  kX86InstIdRepeCmpsW,                   // X86/X64 (REP)
  kX86InstIdRepeScasB,                   // X86/X64 (REP)
  kX86InstIdRepeScasD,                   // X86/X64 (REP)
  kX86InstIdRepeScasQ,                   // X64 only (REP)
  kX86InstIdRepeScasW,                   // X86/X64 (REP)
  kX86InstIdRepneCmpsB,                  // X86/X64 (REP)
  kX86InstIdRepneCmpsD,                  // X86/X64 (REP)
  kX86InstIdRepneCmpsQ,                  // X64 only (REP)
  kX86InstIdRepneCmpsW,                  // X86/X64 (REP)
  kX86InstIdRepneScasB,                  // X86/X64 (REP)
  kX86InstIdRepneScasD,                  // X86/X64 (REP)
  kX86InstIdRepneScasQ,                  // X64 only (REP)
  kX86InstIdRepneScasW,                  // X86/X64 (REP)
  kX86InstIdRet,                         // X86/X64
  kX86InstIdRol,                         // X86/X64
  kX86InstIdRor,                         // X86/X64
  kX86InstIdRorx,                        // BMI2
  kX86InstIdRoundpd,                     // SSE4.1
  kX86InstIdRoundps,                     // SSE4.1
  kX86InstIdRoundsd,                     // SSE4.1
  kX86InstIdRoundss,                     // SSE4.1
  kX86InstIdRsqrtps,                     // SSE
  kX86InstIdRsqrtss,                     // SSE
  kX86InstIdSahf,                        // X86/X64 (LAHF/SAHF)
  kX86InstIdSal,                         // X86/X64
  kX86InstIdSar,                         // X86/X64
  kX86InstIdSarx,                        // BMI2
  kX86InstIdSbb,                         // X86/X64
  kX86InstIdScasB,                       // SCAS
  kX86InstIdScasD,                       // SCAS
  kX86InstIdScasQ,                       // SCAS (X64)
  kX86InstIdScasW,                       // SCAS
  kX86InstIdSeta,                        // X86/X64 (setcc)
  kX86InstIdSetae,                       // X86/X64 (setcc)
  kX86InstIdSetb,                        // X86/X64 (setcc)
  kX86InstIdSetbe,                       // X86/X64 (setcc)
  kX86InstIdSetc,                        // X86/X64 (setcc)
  kX86InstIdSete,                        // X86/X64 (setcc)
  kX86InstIdSetg,                        // X86/X64 (setcc)
  kX86InstIdSetge,                       // X86/X64 (setcc)
  kX86InstIdSetl,                        // X86/X64 (setcc)
  kX86InstIdSetle,                       // X86/X64 (setcc)
  kX86InstIdSetna,                       // X86/X64 (setcc)
  kX86InstIdSetnae,                      // X86/X64 (setcc)
  kX86InstIdSetnb,                       // X86/X64 (setcc)
  kX86InstIdSetnbe,                      // X86/X64 (setcc)
  kX86InstIdSetnc,                       // X86/X64 (setcc)
  kX86InstIdSetne,                       // X86/X64 (setcc)
  kX86InstIdSetng,                       // X86/X64 (setcc)
  kX86InstIdSetnge,                      // X86/X64 (setcc)
  kX86InstIdSetnl,                       // X86/X64 (setcc)
  kX86InstIdSetnle,                      // X86/X64 (setcc)
  kX86InstIdSetno,                       // X86/X64 (setcc)
  kX86InstIdSetnp,                       // X86/X64 (setcc)
  kX86InstIdSetns,                       // X86/X64 (setcc)
  kX86InstIdSetnz,                       // X86/X64 (setcc)
  kX86InstIdSeto,                        // X86/X64 (setcc)
  kX86InstIdSetp,                        // X86/X64 (setcc)
  kX86InstIdSetpe,                       // X86/X64 (setcc)
  kX86InstIdSetpo,                       // X86/X64 (setcc)
  kX86InstIdSets,                        // X86/X64 (setcc)
  kX86InstIdSetz,                        // X86/X64 (setcc)
  kX86InstIdSfence,                      // MMX2/SSE
  kX86InstIdSha1msg1,                    // SHA
  kX86InstIdSha1msg2,                    // SHA
  kX86InstIdSha1nexte,                   // SHA
  kX86InstIdSha1rnds4,                   // SHA
  kX86InstIdSha256msg1,                  // SHA
  kX86InstIdSha256msg2,                  // SHA
  kX86InstIdSha256rnds2,                 // SHA
  kX86InstIdShl,                         // X86/X64
  kX86InstIdShld,                        // X86/X64
  kX86InstIdShlx,                        // BMI2
  kX86InstIdShr,                         // X86/X64
  kX86InstIdShrd,                        // X86/X64
  kX86InstIdShrx,                        // BMI2
  kX86InstIdShufpd,                      // SSE2
  kX86InstIdShufps,                      // SSE
  kX86InstIdSqrtpd,                      // SSE2
  kX86InstIdSqrtps,                      // SSE
  kX86InstIdSqrtsd,                      // SSE2
  kX86InstIdSqrtss,                      // SSE
  kX86InstIdStc,                         // X86/X64
  kX86InstIdStd,                         // X86/X64
  kX86InstIdStmxcsr,                     // SSE
  kX86InstIdStosB,                       // STOS
  kX86InstIdStosD,                       // STOS
  kX86InstIdStosQ,                       // STOS (X64)
  kX86InstIdStosW,                       // STOS
  kX86InstIdSub,                         // X86/X64
  kX86InstIdSubpd,                       // SSE2
  kX86InstIdSubps,                       // SSE
  kX86InstIdSubsd,                       // SSE2
  kX86InstIdSubss,                       // SSE
  kX86InstIdT1mskc,                      // TBM
  kX86InstIdTest,                        // X86/X64
  kX86InstIdTzcnt,                       // TZCNT
  kX86InstIdTzmsk,                       // TBM
  kX86InstIdUcomisd,                     // SSE2
  kX86InstIdUcomiss,                     // SSE
  kX86InstIdUd2,                         // X86/X64
  kX86InstIdUnpckhpd,                    // SSE2
  kX86InstIdUnpckhps,                    // SSE
  kX86InstIdUnpcklpd,                    // SSE2
  kX86InstIdUnpcklps,                    // SSE
  kX86InstIdVaddpd,                      // AVX
  kX86InstIdVaddps,                      // AVX
  kX86InstIdVaddsd,                      // AVX
  kX86InstIdVaddss,                      // AVX
  kX86InstIdVaddsubpd,                   // AVX
  kX86InstIdVaddsubps,                   // AVX
  kX86InstIdVaesdec,                     // AVX+AESNI
  kX86InstIdVaesdeclast,                 // AVX+AESNI
  kX86InstIdVaesenc,                     // AVX+AESNI
  kX86InstIdVaesenclast,                 // AVX+AESNI
  kX86InstIdVaesimc,                     // AVX+AESNI
  kX86InstIdVaeskeygenassist,            // AVX+AESNI
  kX86InstIdVandnpd,                     // AVX
  kX86InstIdVandnps,                     // AVX
  kX86InstIdVandpd,                      // AVX
  kX86InstIdVandps,                      // AVX
  kX86InstIdVblendpd,                    // AVX
  kX86InstIdVblendps,                    // AVX
  kX86InstIdVblendvpd,                   // AVX
  kX86InstIdVblendvps,                   // AVX
  kX86InstIdVbroadcastf128,              // AVX
  kX86InstIdVbroadcasti128,              // AVX2
  kX86InstIdVbroadcastsd,                // AVX/AVX2
  kX86InstIdVbroadcastss,                // AVX/AVX2
  kX86InstIdVcmppd,                      // AVX
  kX86InstIdVcmpps,                      // AVX
  kX86InstIdVcmpsd,                      // AVX
  kX86InstIdVcmpss,                      // AVX
  kX86InstIdVcomisd,                     // AVX
  kX86InstIdVcomiss,                     // AVX
  kX86InstIdVcvtdq2pd,                   // AVX
  kX86InstIdVcvtdq2ps,                   // AVX
  kX86InstIdVcvtpd2dq,                   // AVX
  kX86InstIdVcvtpd2ps,                   // AVX
  kX86InstIdVcvtph2ps,                   // F16C
  kX86InstIdVcvtps2dq,                   // AVX
  kX86InstIdVcvtps2pd,                   // AVX
  kX86InstIdVcvtps2ph,                   // F16C
  kX86InstIdVcvtsd2si,                   // AVX
  kX86InstIdVcvtsd2ss,                   // AVX
  kX86InstIdVcvtsi2sd,                   // AVX
  kX86InstIdVcvtsi2ss,                   // AVX
  kX86InstIdVcvtss2sd,                   // AVX
  kX86InstIdVcvtss2si,                   // AVX
  kX86InstIdVcvttpd2dq,                  // AVX
  kX86InstIdVcvttps2dq,                  // AVX
  kX86InstIdVcvttsd2si,                  // AVX
  kX86InstIdVcvttss2si,                  // AVX
  kX86InstIdVdivpd,                      // AVX
  kX86InstIdVdivps,                      // AVX
  kX86InstIdVdivsd,                      // AVX
  kX86InstIdVdivss,                      // AVX
  kX86InstIdVdppd,                       // AVX
  kX86InstIdVdpps,                       // AVX
  kX86InstIdVextractf128,                // AVX
  kX86InstIdVextracti128,                // AVX2
  kX86InstIdVextractps,                  // AVX
  kX86InstIdVfmadd132pd,                 // FMA3
  kX86InstIdVfmadd132ps,                 // FMA3
  kX86InstIdVfmadd132sd,                 // FMA3
  kX86InstIdVfmadd132ss,                 // FMA3
  kX86InstIdVfmadd213pd,                 // FMA3
  kX86InstIdVfmadd213ps,                 // FMA3
  kX86InstIdVfmadd213sd,                 // FMA3
  kX86InstIdVfmadd213ss,                 // FMA3
  kX86InstIdVfmadd231pd,                 // FMA3
  kX86InstIdVfmadd231ps,                 // FMA3
  kX86InstIdVfmadd231sd,                 // FMA3
  kX86InstIdVfmadd231ss,                 // FMA3
  kX86InstIdVfmaddpd,                    // FMA4
  kX86InstIdVfmaddps,                    // FMA4
  kX86InstIdVfmaddsd,                    // FMA4
  kX86InstIdVfmaddss,                    // FMA4
  kX86InstIdVfmaddsub132pd,              // FMA3
  kX86InstIdVfmaddsub132ps,              // FMA3
  kX86InstIdVfmaddsub213pd,              // FMA3
  kX86InstIdVfmaddsub213ps,              // FMA3
  kX86InstIdVfmaddsub231pd,              // FMA3
  kX86InstIdVfmaddsub231ps,              // FMA3
  kX86InstIdVfmaddsubpd,                 // FMA4
  kX86InstIdVfmaddsubps,                 // FMA4
  kX86InstIdVfmsub132pd,                 // FMA3
  kX86InstIdVfmsub132ps,                 // FMA3
  kX86InstIdVfmsub132sd,                 // FMA3
  kX86InstIdVfmsub132ss,                 // FMA3
  kX86InstIdVfmsub213pd,                 // FMA3
  kX86InstIdVfmsub213ps,                 // FMA3
  kX86InstIdVfmsub213sd,                 // FMA3
  kX86InstIdVfmsub213ss,                 // FMA3
  kX86InstIdVfmsub231pd,                 // FMA3
  kX86InstIdVfmsub231ps,                 // FMA3
  kX86InstIdVfmsub231sd,                 // FMA3
  kX86InstIdVfmsub231ss,                 // FMA3
  kX86InstIdVfmsubadd132pd,              // FMA3
  kX86InstIdVfmsubadd132ps,              // FMA3
  kX86InstIdVfmsubadd213pd,              // FMA3
  kX86InstIdVfmsubadd213ps,              // FMA3
  kX86InstIdVfmsubadd231pd,              // FMA3
  kX86InstIdVfmsubadd231ps,              // FMA3
  kX86InstIdVfmsubaddpd,                 // FMA4
  kX86InstIdVfmsubaddps,                 // FMA4
  kX86InstIdVfmsubpd,                    // FMA4
  kX86InstIdVfmsubps,                    // FMA4
  kX86InstIdVfmsubsd,                    // FMA4
  kX86InstIdVfmsubss,                    // FMA4
  kX86InstIdVfnmadd132pd,                // FMA3
  kX86InstIdVfnmadd132ps,                // FMA3
  kX86InstIdVfnmadd132sd,                // FMA3
  kX86InstIdVfnmadd132ss,                // FMA3
  kX86InstIdVfnmadd213pd,                // FMA3
  kX86InstIdVfnmadd213ps,                // FMA3
  kX86InstIdVfnmadd213sd,                // FMA3
  kX86InstIdVfnmadd213ss,                // FMA3
  kX86InstIdVfnmadd231pd,                // FMA3
  kX86InstIdVfnmadd231ps,                // FMA3
  kX86InstIdVfnmadd231sd,                // FMA3
  kX86InstIdVfnmadd231ss,                // FMA3
  kX86InstIdVfnmaddpd,                   // FMA4
  kX86InstIdVfnmaddps,                   // FMA4
  kX86InstIdVfnmaddsd,                   // FMA4
  kX86InstIdVfnmaddss,                   // FMA4
  kX86InstIdVfnmsub132pd,                // FMA3
  kX86InstIdVfnmsub132ps,                // FMA3
  kX86InstIdVfnmsub132sd,                // FMA3
  kX86InstIdVfnmsub132ss,                // FMA3
  kX86InstIdVfnmsub213pd,                // FMA3
  kX86InstIdVfnmsub213ps,                // FMA3
  kX86InstIdVfnmsub213sd,                // FMA3
  kX86InstIdVfnmsub213ss,                // FMA3
  kX86InstIdVfnmsub231pd,                // FMA3
  kX86InstIdVfnmsub231ps,                // FMA3
  kX86InstIdVfnmsub231sd,                // FMA3
  kX86InstIdVfnmsub231ss,                // FMA3
  kX86InstIdVfnmsubpd,                   // FMA4
  kX86InstIdVfnmsubps,                   // FMA4
  kX86InstIdVfnmsubsd,                   // FMA4
  kX86InstIdVfnmsubss,                   // FMA4
  kX86InstIdVfrczpd,                     // XOP
  kX86InstIdVfrczps,                     // XOP
  kX86InstIdVfrczsd,                     // XOP
  kX86InstIdVfrczss,                     // XOP
  kX86InstIdVgatherdpd,                  // AVX2
  kX86InstIdVgatherdps,                  // AVX2
  kX86InstIdVgatherqpd,                  // AVX2
  kX86InstIdVgatherqps,                  // AVX2
  kX86InstIdVhaddpd,                     // AVX
  kX86InstIdVhaddps,                     // AVX
  kX86InstIdVhsubpd,                     // AVX
  kX86InstIdVhsubps,                     // AVX
  kX86InstIdVinsertf128,                 // AVX
  kX86InstIdVinserti128,                 // AVX2
  kX86InstIdVinsertps,                   // AVX
  kX86InstIdVlddqu,                      // AVX
  kX86InstIdVldmxcsr,                    // AVX
  kX86InstIdVmaskmovdqu,                 // AVX
  kX86InstIdVmaskmovpd,                  // AVX
  kX86InstIdVmaskmovps,                  // AVX
  kX86InstIdVmaxpd,                      // AVX
  kX86InstIdVmaxps,                      // AVX
  kX86InstIdVmaxsd,                      // AVX
  kX86InstIdVmaxss,                      // AVX
  kX86InstIdVminpd,                      // AVX
  kX86InstIdVminps,                      // AVX
  kX86InstIdVminsd,                      // AVX
  kX86InstIdVminss,                      // AVX
  kX86InstIdVmovapd,                     // AVX
  kX86InstIdVmovaps,                     // AVX
  kX86InstIdVmovd,                       // AVX
  kX86InstIdVmovddup,                    // AVX
  kX86InstIdVmovdqa,                     // AVX
  kX86InstIdVmovdqu,                     // AVX
  kX86InstIdVmovhlps,                    // AVX
  kX86InstIdVmovhpd,                     // AVX
  kX86InstIdVmovhps,                     // AVX
  kX86InstIdVmovlhps,                    // AVX
  kX86InstIdVmovlpd,                     // AVX
  kX86InstIdVmovlps,                     // AVX
  kX86InstIdVmovmskpd,                   // AVX
  kX86InstIdVmovmskps,                   // AVX
  kX86InstIdVmovntdq,                    // AVX
  kX86InstIdVmovntdqa,                   // AVX/AVX2
  kX86InstIdVmovntpd,                    // AVX
  kX86InstIdVmovntps,                    // AVX
  kX86InstIdVmovq,                       // AVX
  kX86InstIdVmovsd,                      // AVX
  kX86InstIdVmovshdup,                   // AVX
  kX86InstIdVmovsldup,                   // AVX
  kX86InstIdVmovss,                      // AVX
  kX86InstIdVmovupd,                     // AVX
  kX86InstIdVmovups,                     // AVX
  kX86InstIdVmpsadbw,                    // AVX/AVX2
  kX86InstIdVmulpd,                      // AVX
  kX86InstIdVmulps,                      // AVX
  kX86InstIdVmulsd,                      // AVX
  kX86InstIdVmulss,                      // AVX
  kX86InstIdVorpd,                       // AVX
  kX86InstIdVorps,                       // AVX
  kX86InstIdVpabsb,                      // AVX2
  kX86InstIdVpabsd,                      // AVX2
  kX86InstIdVpabsw,                      // AVX2
  kX86InstIdVpackssdw,                   // AVX2
  kX86InstIdVpacksswb,                   // AVX2
  kX86InstIdVpackusdw,                   // AVX2
  kX86InstIdVpackuswb,                   // AVX2
  kX86InstIdVpaddb,                      // AVX2
  kX86InstIdVpaddd,                      // AVX2
  kX86InstIdVpaddq,                      // AVX2
  kX86InstIdVpaddsb,                     // AVX2
  kX86InstIdVpaddsw,                     // AVX2
  kX86InstIdVpaddusb,                    // AVX2
  kX86InstIdVpaddusw,                    // AVX2
  kX86InstIdVpaddw,                      // AVX2
  kX86InstIdVpalignr,                    // AVX2
  kX86InstIdVpand,                       // AVX2
  kX86InstIdVpandn,                      // AVX2
  kX86InstIdVpavgb,                      // AVX2
  kX86InstIdVpavgw,                      // AVX2
  kX86InstIdVpblendd,                    // AVX2
  kX86InstIdVpblendvb,                   // AVX2
  kX86InstIdVpblendw,                    // AVX2
  kX86InstIdVpbroadcastb,                // AVX2
  kX86InstIdVpbroadcastd,                // AVX2
  kX86InstIdVpbroadcastq,                // AVX2
  kX86InstIdVpbroadcastw,                // AVX2
  kX86InstIdVpclmulqdq,                  // AVX+PCLMULQDQ
  kX86InstIdVpcmov,                      // XOP
  kX86InstIdVpcmpeqb,                    // AVX2
  kX86InstIdVpcmpeqd,                    // AVX2
  kX86InstIdVpcmpeqq,                    // AVX2
  kX86InstIdVpcmpeqw,                    // AVX2
  kX86InstIdVpcmpestri,                  // AVX
  kX86InstIdVpcmpestrm,                  // AVX
  kX86InstIdVpcmpgtb,                    // AVX2
  kX86InstIdVpcmpgtd,                    // AVX2
  kX86InstIdVpcmpgtq,                    // AVX2
  kX86InstIdVpcmpgtw,                    // AVX2
  kX86InstIdVpcmpistri,                  // AVX
  kX86InstIdVpcmpistrm,                  // AVX
  kX86InstIdVpcomb,                      // XOP
  kX86InstIdVpcomd,                      // XOP
  kX86InstIdVpcomq,                      // XOP
  kX86InstIdVpcomub,                     // XOP
  kX86InstIdVpcomud,                     // XOP
  kX86InstIdVpcomuq,                     // XOP
  kX86InstIdVpcomuw,                     // XOP
  kX86InstIdVpcomw,                      // XOP
  kX86InstIdVperm2f128,                  // AVX
  kX86InstIdVperm2i128,                  // AVX2
  kX86InstIdVpermd,                      // AVX2
  kX86InstIdVpermil2pd,                  // XOP
  kX86InstIdVpermil2ps,                  // XOP
  kX86InstIdVpermilpd,                   // AVX
  kX86InstIdVpermilps,                   // AVX
  kX86InstIdVpermpd,                     // AVX2
  kX86InstIdVpermps,                     // AVX2
  kX86InstIdVpermq,                      // AVX2
  kX86InstIdVpextrb,                     // AVX
  kX86InstIdVpextrd,                     // AVX
  kX86InstIdVpextrq,                     // AVX (X64)
  kX86InstIdVpextrw,                     // AVX
  kX86InstIdVpgatherdd,                  // AVX2
  kX86InstIdVpgatherdq,                  // AVX2
  kX86InstIdVpgatherqd,                  // AVX2
  kX86InstIdVpgatherqq,                  // AVX2
  kX86InstIdVphaddbd,                    // XOP
  kX86InstIdVphaddbq,                    // XOP
  kX86InstIdVphaddbw,                    // XOP
  kX86InstIdVphaddd,                     // AVX2
  kX86InstIdVphadddq,                    // XOP
  kX86InstIdVphaddsw,                    // AVX2
  kX86InstIdVphaddubd,                   // XOP
  kX86InstIdVphaddubq,                   // XOP
  kX86InstIdVphaddubw,                   // XOP
  kX86InstIdVphaddudq,                   // XOP
  kX86InstIdVphadduwd,                   // XOP
  kX86InstIdVphadduwq,                   // XOP
  kX86InstIdVphaddw,                     // AVX2
  kX86InstIdVphaddwd,                    // XOP
  kX86InstIdVphaddwq,                    // XOP
  kX86InstIdVphminposuw,                 // AVX
  kX86InstIdVphsubbw,                    // XOP
  kX86InstIdVphsubd,                     // AVX2
  kX86InstIdVphsubdq,                    // XOP
  kX86InstIdVphsubsw,                    // AVX2
  kX86InstIdVphsubw,                     // AVX2
  kX86InstIdVphsubwd,                    // XOP
  kX86InstIdVpinsrb,                     // AVX
  kX86InstIdVpinsrd,                     // AVX
  kX86InstIdVpinsrq,                     // AVX (X64)
  kX86InstIdVpinsrw,                     // AVX
  kX86InstIdVpmacsdd,                    // XOP
  kX86InstIdVpmacsdqh,                   // XOP
  kX86InstIdVpmacsdql,                   // XOP
  kX86InstIdVpmacssdd,                   // XOP
  kX86InstIdVpmacssdqh,                  // XOP
  kX86InstIdVpmacssdql,                  // XOP
  kX86InstIdVpmacsswd,                   // XOP
  kX86InstIdVpmacssww,                   // XOP
  kX86InstIdVpmacswd,                    // XOP
  kX86InstIdVpmacsww,                    // XOP
  kX86InstIdVpmadcsswd,                  // XOP
  kX86InstIdVpmadcswd,                   // XOP
  kX86InstIdVpmaddubsw,                  // AVX/AVX2
  kX86InstIdVpmaddwd,                    // AVX/AVX2
  kX86InstIdVpmaskmovd,                  // AVX2
  kX86InstIdVpmaskmovq,                  // AVX2
  kX86InstIdVpmaxsb,                     // AVX/AVX2
  kX86InstIdVpmaxsd,                     // AVX/AVX2
  kX86InstIdVpmaxsw,                     // AVX/AVX2
  kX86InstIdVpmaxub,                     // AVX/AVX2
  kX86InstIdVpmaxud,                     // AVX/AVX2
  kX86InstIdVpmaxuw,                     // AVX/AVX2
  kX86InstIdVpminsb,                     // AVX/AVX2
  kX86InstIdVpminsd,                     // AVX/AVX2
  kX86InstIdVpminsw,                     // AVX/AVX2
  kX86InstIdVpminub,                     // AVX/AVX2
  kX86InstIdVpminud,                     // AVX/AVX2
  kX86InstIdVpminuw,                     // AVX/AVX2
  kX86InstIdVpmovmskb,                   // AVX/AVX2
  kX86InstIdVpmovsxbd,                   // AVX/AVX2
  kX86InstIdVpmovsxbq,                   // AVX/AVX2
  kX86InstIdVpmovsxbw,                   // AVX/AVX2
  kX86InstIdVpmovsxdq,                   // AVX/AVX2
  kX86InstIdVpmovsxwd,                   // AVX/AVX2
  kX86InstIdVpmovsxwq,                   // AVX/AVX2
  kX86InstIdVpmovzxbd,                   // AVX/AVX2
  kX86InstIdVpmovzxbq,                   // AVX/AVX2
  kX86InstIdVpmovzxbw,                   // AVX/AVX2
  kX86InstIdVpmovzxdq,                   // AVX/AVX2
  kX86InstIdVpmovzxwd,                   // AVX/AVX2
  kX86InstIdVpmovzxwq,                   // AVX/AVX2
  kX86InstIdVpmuldq,                     // AVX/AVX2
  kX86InstIdVpmulhrsw,                   // AVX/AVX2
  kX86InstIdVpmulhuw,                    // AVX/AVX2
  kX86InstIdVpmulhw,                     // AVX/AVX2
  kX86InstIdVpmulld,                     // AVX/AVX2
  kX86InstIdVpmullw,                     // AVX/AVX2
  kX86InstIdVpmuludq,                    // AVX/AVX2
  kX86InstIdVpor,                        // AVX/AVX2
  kX86InstIdVpperm,                      // XOP
  kX86InstIdVprotb,                      // XOP
  kX86InstIdVprotd,                      // XOP
  kX86InstIdVprotq,                      // XOP
  kX86InstIdVprotw,                      // XOP
  kX86InstIdVpsadbw,                     // AVX/AVX2
  kX86InstIdVpshab,                      // XOP
  kX86InstIdVpshad,                      // XOP
  kX86InstIdVpshaq,                      // XOP
  kX86InstIdVpshaw,                      // XOP
  kX86InstIdVpshlb,                      // XOP
  kX86InstIdVpshld,                      // XOP
  kX86InstIdVpshlq,                      // XOP
  kX86InstIdVpshlw,                      // XOP
  kX86InstIdVpshufb,                     // AVX/AVX2
  kX86InstIdVpshufd,                     // AVX/AVX2
  kX86InstIdVpshufhw,                    // AVX/AVX2
  kX86InstIdVpshuflw,                    // AVX/AVX2
  kX86InstIdVpsignb,                     // AVX/AVX2
  kX86InstIdVpsignd,                     // AVX/AVX2
  kX86InstIdVpsignw,                     // AVX/AVX2
  kX86InstIdVpslld,                      // AVX/AVX2
  kX86InstIdVpslldq,                     // AVX/AVX2
  kX86InstIdVpsllq,                      // AVX/AVX2
  kX86InstIdVpsllvd,                     // AVX2
  kX86InstIdVpsllvq,                     // AVX2
  kX86InstIdVpsllw,                      // AVX/AVX2
  kX86InstIdVpsrad,                      // AVX/AVX2
  kX86InstIdVpsravd,                     // AVX2
  kX86InstIdVpsraw,                      // AVX/AVX2
  kX86InstIdVpsrld,                      // AVX/AVX2
  kX86InstIdVpsrldq,                     // AVX/AVX2
  kX86InstIdVpsrlq,                      // AVX/AVX2
  kX86InstIdVpsrlvd,                     // AVX2
  kX86InstIdVpsrlvq,                     // AVX2
  kX86InstIdVpsrlw,                      // AVX/AVX2
  kX86InstIdVpsubb,                      // AVX/AVX2
  kX86InstIdVpsubd,                      // AVX/AVX2
  kX86InstIdVpsubq,                      // AVX/AVX2
  kX86InstIdVpsubsb,                     // AVX/AVX2
  kX86InstIdVpsubsw,                     // AVX/AVX2
  kX86InstIdVpsubusb,                    // AVX/AVX2
  kX86InstIdVpsubusw,                    // AVX/AVX2
  kX86InstIdVpsubw,                      // AVX/AVX2
  kX86InstIdVptest,                      // AVX
  kX86InstIdVpunpckhbw,                  // AVX/AVX2
  kX86InstIdVpunpckhdq,                  // AVX/AVX2
  kX86InstIdVpunpckhqdq,                 // AVX/AVX2
  kX86InstIdVpunpckhwd,                  // AVX/AVX2
  kX86InstIdVpunpcklbw,                  // AVX/AVX2
  kX86InstIdVpunpckldq,                  // AVX/AVX2
  kX86InstIdVpunpcklqdq,                 // AVX/AVX2
  kX86InstIdVpunpcklwd,                  // AVX/AVX2
  kX86InstIdVpxor,                       // AVX/AVX2
  kX86InstIdVrcpps,                      // AVX
  kX86InstIdVrcpss,                      // AVX
  kX86InstIdVroundpd,                    // AVX
  kX86InstIdVroundps,                    // AVX
  kX86InstIdVroundsd,                    // AVX
  kX86InstIdVroundss,                    // AVX
  kX86InstIdVrsqrtps,                    // AVX
  kX86InstIdVrsqrtss,                    // AVX
  kX86InstIdVshufpd,                     // AVX
  kX86InstIdVshufps,                     // AVX
  kX86InstIdVsqrtpd,                     // AVX
  kX86InstIdVsqrtps,                     // AVX
  kX86InstIdVsqrtsd,                     // AVX
  kX86InstIdVsqrtss,                     // AVX
  kX86InstIdVstmxcsr,                    // AVX
  kX86InstIdVsubpd,                      // AVX
  kX86InstIdVsubps,                      // AVX
  kX86InstIdVsubsd,                      // AVX
  kX86InstIdVsubss,                      // AVX
  kX86InstIdVtestpd,                     // AVX
  kX86InstIdVtestps,                     // AVX
  kX86InstIdVucomisd,                    // AVX
  kX86InstIdVucomiss,                    // AVX
  kX86InstIdVunpckhpd,                   // AVX
  kX86InstIdVunpckhps,                   // AVX
  kX86InstIdVunpcklpd,                   // AVX
  kX86InstIdVunpcklps,                   // AVX
  kX86InstIdVxorpd,                      // AVX
  kX86InstIdVxorps,                      // AVX
  kX86InstIdVzeroall,                    // AVX
  kX86InstIdVzeroupper,                  // AVX
  kX86InstIdWrfsbase,                    // FSGSBASE (X64)
  kX86InstIdWrgsbase,                    // FSGSBASE (X64)
  kX86InstIdXadd,                        // X86/X64 (i486+)
  kX86InstIdXchg,                        // X86/X64
  kX86InstIdXgetbv,                      // XSAVE
  kX86InstIdXor,                         // X86/X64
  kX86InstIdXorpd,                       // SSE2
  kX86InstIdXorps,                       // SSE
  kX86InstIdXrstor,                      // XSAVE
  kX86InstIdXrstor64,                    // XSAVE
  kX86InstIdXsave,                       // XSAVE
  kX86InstIdXsave64,                     // XSAVE
  kX86InstIdXsaveopt,                    // XSAVE
  kX86InstIdXsaveopt64,                  // XSAVE
  kX86InstIdXsetbv,                      // XSAVE

  _kX86InstIdCount,

  _kX86InstIdCmovcc = kX86InstIdCmova,
  _kX86InstIdJcc = kX86InstIdJa,
  _kX86InstIdSetcc = kX86InstIdSeta,

  _kX86InstIdJbegin = kX86InstIdJa,
  _kX86InstIdJend = kX86InstIdJmp
};

// ============================================================================
// [asmjit::X86InstOptions]
// ============================================================================

//! X86/X64 instruction emit options, mainly for internal purposes.
ASMJIT_ENUM(X86InstOptions) {
  kX86InstOptionRex       = 0x00000040,  //!< Force REX prefix (X64)
  _kX86InstOptionNoRex    = 0x00000080,  //!< Do not use, internal of `X86Assembler`.
  kX86InstOptionLock      = 0x00000100,  //!< Force LOCK prefix (lock-enabled instructions).
  kX86InstOptionVex3      = 0x00000200,  //!< Force 3-byte VEX prefix (AVX)
  kX86InstOptionEvex      = 0x00010000,  //!< Force 4-byte EVEX prefix (AVX-512).
  kX86InstOptionEvexZero  = 0x00020000,  //!< EVEX use zeroing instead of merging.
  kX86InstOptionEvexOneN  = 0x00040000,  //!< EVEX broadcast the first element to all.
  kX86InstOptionEvexSae   = 0x00080000,  //!< EVEX suppress all exceptions (SAE).
  kX86InstOptionEvexRnSae = 0x00100000,  //!< EVEX 'round-to-nearest' (even) and `SAE`.
  kX86InstOptionEvexRdSae = 0x00200000,  //!< EVEX 'round-down' (toward -inf) and 'SAE'.
  kX86InstOptionEvexRuSae = 0x00400000,  //!< EVEX 'round-up' (toward +inf) and 'SAE'.
  kX86InstOptionEvexRzSae = 0x00800000   //!< EVEX 'round-toward-zero' (truncate) and 'SAE'.
};

// ============================================================================
// [asmjit::X86InstEncoding]
// ============================================================================

//! \internal
//!
//! X86/X64 instruction groups.
//!
//! This group is specific to AsmJit and only used by `X86Assembler`.
ASMJIT_ENUM(X86InstEncoding) {
  kX86InstEncodingNone = 0,              //!< Never used.

  kX86InstEncodingX86Op,
  kX86InstEncodingX86Op_66H,
  kX86InstEncodingX86Rm,
  kX86InstEncodingX86Rm_B,
  kX86InstEncodingX86RmReg,
  kX86InstEncodingX86RegRm,
  kX86InstEncodingX86M,
  kX86InstEncodingX86Arith,              //!< X86 encoding - adc, add, and, cmp, or, sbb, sub, xor.
  kX86InstEncodingX86BSwap,              //!< X86 encoding - bswap.
  kX86InstEncodingX86BTest,              //!< X86 encoding - bt, btc, btr, bts.
  kX86InstEncodingX86Call,               //!< X86 encoding - call.
  kX86InstEncodingX86Enter,              //!< X86 encoding - enter.
  kX86InstEncodingX86Imul,               //!< X86 encoding - imul.
  kX86InstEncodingX86IncDec,             //!< X86 encoding - inc, dec.
  kX86InstEncodingX86Int,                //!< X86 encoding - int (interrupt).
  kX86InstEncodingX86Jcc,                //!< X86 encoding - jcc.
  kX86InstEncodingX86Jecxz,              //!< X86 encoding - jcxz, jecxz, jrcxz.
  kX86InstEncodingX86Jmp,                //!< X86 encoding - jmp.
  kX86InstEncodingX86Lea,                //!< X86 encoding - lea.
  kX86InstEncodingX86Mov,                //!< X86 encoding - mov.
  kX86InstEncodingX86MovsxMovzx,         //!< X86 encoding - movsx, movzx.
  kX86InstEncodingX86Movsxd,             //!< X86 encoding - movsxd.
  kX86InstEncodingX86MovPtr,             //!< X86 encoding - mov with absolute memory operand (x86/x64).
  kX86InstEncodingX86Push,               //!< X86 encoding - push.
  kX86InstEncodingX86Pop,                //!< X86 encoding - pop.
  kX86InstEncodingX86Rep,                //!< X86 encoding - rep|repe|repne lods?, movs?, stos?, cmps?, scas?.
  kX86InstEncodingX86Ret,                //!< X86 encoding - ret.
  kX86InstEncodingX86Rot,                //!< X86 encoding - rcl, rcr, rol, ror, sal, sar, shl, shr.
  kX86InstEncodingX86Set,                //!< X86 encoding - setcc.
  kX86InstEncodingX86ShldShrd,           //!< X86 encoding - shld, shrd.
  kX86InstEncodingX86Test,               //!< X86 encoding - test.
  kX86InstEncodingX86Xadd,               //!< X86 encoding - xadd.
  kX86InstEncodingX86Xchg,               //!< X86 encoding - xchg.
  kX86InstEncodingX86Crc,                //!< X86 encoding - crc32.
  kX86InstEncodingX86Prefetch,           //!< X86 encoding - prefetch.
  kX86InstEncodingX86Fence,              //!< X86 encoding - lfence, mfence, sfence.
  kX86InstEncodingFpuOp,                 //!< FPU encoding - [OP].
  kX86InstEncodingFpuArith,              //!< FPU encoding - fadd, fdiv, fdivr, fmul, fsub, fsubr.
  kX86InstEncodingFpuCom,                //!< FPU encoding - fcom, fcomp.
  kX86InstEncodingFpuFldFst,             //!< FPU encoding - fld, fst, fstp.
  kX86InstEncodingFpuM,                  //!< FPU encoding - fiadd, ficom, ficomp, fidiv, fidivr, fild, fimul, fist, fistp, fisttp, fisub, fisubr.
  kX86InstEncodingFpuR,                  //!< FPU encoding - fcmov, fcomi, fcomip, ffree, fucom, fucomi, fucomip, fucomp, fxch.
  kX86InstEncodingFpuRDef,               //!< FPU encoding - faddp, fdivp, fdivrp, fmulp, fsubp, fsubrp.
  kX86InstEncodingFpuStsw,               //!< FPU encoding - fnstsw, Fstsw.
  kX86InstEncodingSimdRm,                //!< SIMD encoding - [RM].
  kX86InstEncodingSimdRm_P,              //!< SIMD encoding - [RM] (propagates 66H if the instruction uses XMM register).
  kX86InstEncodingSimdRm_Q,              //!< SIMD encoding - [RM] (propagates REX.W if GPQ is used).
  kX86InstEncodingSimdRm_PQ,             //!< SIMD encoding - [RM] (propagates 66H and REX.W).
  kX86InstEncodingSimdRmRi,              //!< SIMD encoding - [RM|RI].
  kX86InstEncodingSimdRmRi_P,            //!< SIMD encoding - [RM|RI] (propagates 66H if the instruction uses XMM register).
  kX86InstEncodingSimdRmi,               //!< SIMD encoding - [RMI].
  kX86InstEncodingSimdRmi_P,             //!< SIMD encoding - [RMI] (propagates 66H if the instruction uses XMM register).
  kX86InstEncodingSimdPextrw,            //!< SIMD encoding - pextrw.
  kX86InstEncodingSimdExtract,           //!< SIMD encoding - pextrb, pextrd, pextrq, extractps.
  kX86InstEncodingSimdMov,               //!< SIMD encoding - mov - primary opcode means `(X)MM <- (X)MM/Mem`, secondary `(X)Mm/Mem <- (X)Mm format`.
  kX86InstEncodingSimdMovNoRexW,         //!< SIMD encoding - movmskpd, movmskps.
  kX86InstEncodingSimdMovBe,             //!< Used by movbe.
  kX86InstEncodingSimdMovD,              //!< SIMD encoding - movd.
  kX86InstEncodingSimdMovQ,              //!< SIMD encoding - movq.
  kX86InstEncodingSimdExtrq,             //!< SIMD encoding - extrq (SSE4a).
  kX86InstEncodingSimdInsertq,           //!< SIMD encoding - insrq (SSE4a).
  kX86InstEncodingSimd3dNow,             //!< SIMD encoding - 3dnow instructions.
  kX86InstEncodingAvxOp,                 //!< AVX encoding - [OP].
  kX86InstEncodingAvxM,                  //!< AVX encoding - [M].
  kX86InstEncodingAvxMr,                 //!< AVX encoding - [MR].
  kX86InstEncodingAvxMr_OptL,            //!< AVX encoding - [MR] (Propagates AVX.L if YMM used).
  kX86InstEncodingAvxMri,                //!< AVX encoding - [MRI].
  kX86InstEncodingAvxMri_OptL,           //!< AVX encoding - [MRI] (Propagates AVX.L if YMM used).
  kX86InstEncodingAvxRm,                 //!< AVX encoding - [RM].
  kX86InstEncodingAvxRm_OptL,            //!< AVX encoding - [RM] (Propagates AVX.L if YMM used).
  kX86InstEncodingAvxRmi,                //!< AVX encoding - [RMI].
  kX86InstEncodingAvxRmi_OptW,           //!< AVX encoding - [RMI] (Propagates AVX.W if GPQ used).
  kX86InstEncodingAvxRmi_OptL,           //!< AVX encoding - [RMI] (Propagates AVX.L if YMM used).
  kX86InstEncodingAvxRvm,                //!< AVX encoding - [RVM].
  kX86InstEncodingAvxRvm_OptW,           //!< AVX encoding - [RVM] (Propagates AVX.W if GPQ used).
  kX86InstEncodingAvxRvm_OptL,           //!< AVX encoding - [RVM] (Propagates AVX.L if YMM used).
  kX86InstEncodingAvxRvmr,               //!< AVX encoding - [RVMR].
  kX86InstEncodingAvxRvmr_OptL,          //!< AVX encoding - [RVMR] (Propagates AVX.L if YMM used).
  kX86InstEncodingAvxRvmi,               //!< AVX encoding - [RVMI].
  kX86InstEncodingAvxRvmi_OptL,          //!< AVX encoding - [RVMI] (Propagates AVX.L if YMM used).
  kX86InstEncodingAvxRmv,                //!< AVX encoding - [RMV].
  kX86InstEncodingAvxRmv_OptW,           //!< AVX encoding - [RMV] (Propagates AVX.W if GPQ used).
  kX86InstEncodingAvxRmvi,               //!< AVX encoding - [RMVI].
  kX86InstEncodingAvxRmMr,               //!< AVX encoding - [RM|MR].
  kX86InstEncodingAvxRmMr_OptL,          //!< AVX encoding - [RM|MR] (Propagates AVX.L if YMM used).
  kX86InstEncodingAvxRvmRmi,             //!< AVX encoding - [RVM|RMI].
  kX86InstEncodingAvxRvmRmi_OptL,        //!< AVX encoding - [RVM|RMI] (Propagates AVX.L if YMM used).
  kX86InstEncodingAvxRvmMr,              //!< AVX encoding - [RVM|MR].
  kX86InstEncodingAvxRvmMvr,             //!< AVX encoding - [RVM|MVR].
  kX86InstEncodingAvxRvmMvr_OptL,        //!< AVX encoding - [RVM|MVR] (Propagates AVX.L if YMM used).
  kX86InstEncodingAvxRvmVmi,             //!< AVX encoding - [RVM|VMI].
  kX86InstEncodingAvxRvmVmi_OptL,        //!< AVX encoding - [RVM|VMI] (Propagates AVX.L if YMM used).
  kX86InstEncodingAvxVm,                 //!< AVX encoding - [VM].
  kX86InstEncodingAvxVm_OptW,            //!< AVX encoding - [VM] (Propagates AVX.W if GPQ used).
  kX86InstEncodingAvxVmi,                //!< AVX encoding - [VMI].
  kX86InstEncodingAvxVmi_OptL,           //!< AVX encoding - [VMI] (Propagates AVX.L if YMM used).
  kX86InstEncodingAvxRvrmRvmr,           //!< AVX encoding - [RVRM|RVMR].
  kX86InstEncodingAvxRvrmRvmr_OptL,      //!< AVX encoding - [RVRM|RVMR] (Propagates AVX.L if YMM used).
  kX86InstEncodingAvxMovDQ,              //!< AVX encoding - vmovd, vmovq.
  kX86InstEncodingAvxMovSsSd,            //!< AVX encoding - vmovss, vmovsd.
  kX86InstEncodingAvxGather,             //!< AVX encoding - gather (VSIB).
  kX86InstEncodingAvxGatherEx,           //!< AVX encoding - gather (VSIB), differs only in MEM operand.
  kX86InstEncodingFma4,                  //!< FMA4 encoding - [R, R, R/M, R/M].
  kX86InstEncodingFma4_OptL,             //!< FMA4 encoding - [R, R, R/M, R/M] (Propagates AVX.L if YMM used).
  kX86InstEncodingXopRm,                 //!< XOP encoding - [RM].
  kX86InstEncodingXopRm_OptL,            //!< XOP encoding - [RM] (Propagates AVX.L if YMM used).
  kX86InstEncodingXopRvmRmv,             //!< XOP encoding - [RVM | RMV].
  kX86InstEncodingXopRvmRmi,             //!< XOP encoding - [RVM | RMI].
  kX86InstEncodingXopRvmr,               //!< XOP encoding - [RVMR].
  kX86InstEncodingXopRvmr_OptL,          //!< XOP encoding - [RVMR] (Propagates AVX.L if YMM used).
  kX86InstEncodingXopRvmi,               //!< XOP encoding - [RVMI].
  kX86InstEncodingXopRvmi_OptL,          //!< XOP encoding - [RVMI] (Propagates AVX.L if YMM used).
  kX86InstEncodingXopRvrmRvmr,           //!< XOP encoding - [RVRM | RVMR].
  kX86InstEncodingXopRvrmRvmr_OptL,      //!< XOP encoding - [RVRM | RVMR] (Propagates AVX.L if YMM used).
  kX86InstEncodingXopVm_OptW,            //!< XOP encoding - [VM].

  _kX86InstEncodingCount                 //!< Count of X86 instruction encodings.
};

// ============================================================================
// [asmjit::X86InstOpCodeFlags]
// ============================================================================

//! \internal
//!
//! X86/X64 Instruction opcode encoding used by asmjit 'X86InstInfo' table.
//!
//! This schema is AsmJit specific and has been designed to allow encoding of
//! all X86 instructions available. X86, MMX, and SSE+ instructions always use
//! `MMMMM` and `PP` fields, which are encoded to corresponding prefixes needed
//! by X86 or SIMD instructions. AVX+ instructions embed `MMMMM` and `PP` fields
//! in a VEX prefix.
//!
//! The instruction opcode definition uses 1 or 2 bytes as an opcode value. 1
//! byte is needed by most of the instructions, 2 bytes are only used by legacy
//! X87-FPU instructions. This means that a second byte is free to by used by
//! AVX and AVX-512 instructions.
//!
//! The fields description:
//!
//! - `MMMMM` field is used to encode prefixes needed by the instruction or as
//!   a part of VEX/EVEX prefix.
//!
//! - `PP` field is used to encode prefixes needed by the instruction or as a
//!   part of VEX/EVEX prefix.
//!
//! - `L` field is used exclusively by AVX+ and AVX512+ instruction sets. It
//!   describes vector size, which is 128-bit for XMM register `L_128`, 256
//!   for YMM register `L_256` and 512-bit for ZMM register `L_512`. The `L`
//!   field is omitted in case that instruction supports multiple vector lengths,
//!   however, if the instruction requires specific `L` value it's specified as
//!   a part of the opcode.
//!
//! - `W` field is the most complicated. It was added by 64-bit architecture
//!   to promote default operation width (instructions that perform 32-bit
//!   operation by default require to override the width to 64-bit explicitly).
//!   There is nothing wrong on this, however, some instructions introduced
//!   implicit `W` override, for example a `cdqe` instruction is basically a
//!   `cwde` instruction with overridden `W` (set to 1). There are some others
//!   in the base X86 instruction set. More recent instruction sets started
//!   using `W` field more often:
//!
//!   - AVX instructions started using `W` field as an extended opcode for FMA,
//!     GATHER, PERM, and other instructions. It also uses `W` field to override
//!     the default operation width in instructions like `vmovq`.
//!
//!   - AVX-512 instructions started using `W` field as an extended opcode for
//!     all new instructions. This wouldn't have been an issue if the `W` field
//!     of AVX-512 have matched AVX, but this is not always the case.
//!
//! - `O` field is an extended opcode field (3) bytes used by ModR/M BYTE.
ASMJIT_ENUM(X86InstOpCodeFlags) {
  // `MMMMM` field in AVX/XOP/AVX-512 instruction (5 bits).
  //
  // `OpCode` leading bytes in legacy encoding.
  //
  // AVX reserves 5 bits for `MMMMM` field, however AVX instructions only use
  // 2 bits and XOP 4 bits. AVX-512 shrinks `MMMMM` field into `MM` so it's
  // safe to assume that `MM` field won't grow in the future as EVEX doesn't
  // use more than 2 bits. There is always a way how a fifth bit can be stored
  // if needed.
  kX86InstOpCode_MM_Shift  = 16,
  kX86InstOpCode_MM_Mask   = 0x0FU << kX86InstOpCode_MM_Shift,
  kX86InstOpCode_MM_00     = 0x00U << kX86InstOpCode_MM_Shift,
  kX86InstOpCode_MM_0F     = 0x01U << kX86InstOpCode_MM_Shift,
  kX86InstOpCode_MM_0F38   = 0x02U << kX86InstOpCode_MM_Shift,
  kX86InstOpCode_MM_0F3A   = 0x03U << kX86InstOpCode_MM_Shift,
  kX86InstOpCode_MM_00011  = 0x03U << kX86InstOpCode_MM_Shift, // XOP.
  kX86InstOpCode_MM_01000  = 0x08U << kX86InstOpCode_MM_Shift, // XOP.
  kX86InstOpCode_MM_01001  = 0x09U << kX86InstOpCode_MM_Shift, // XOP.
  kX86InstOpCode_MM_0F01   = 0x0FU << kX86InstOpCode_MM_Shift, // AsmJit specific, not part of AVX.

  // `PP` field in AVX/XOP/AVX-512 instruction.
  //
  // `Mandatory Prefix` in legacy encoding.
  //
  // AVX reserves 2 bits for `PP` field, but AsmJit extends the storage by 1
  // more bit that is used to emit 9B prefix for some X87-FPU instructions.
  kX86InstOpCode_PP_Shift  = 20,
  kX86InstOpCode_PP_Mask   = 0x07U << kX86InstOpCode_PP_Shift,
  kX86InstOpCode_PP_00     = 0x00U << kX86InstOpCode_PP_Shift,
  kX86InstOpCode_PP_66     = 0x01U << kX86InstOpCode_PP_Shift,
  kX86InstOpCode_PP_F3     = 0x02U << kX86InstOpCode_PP_Shift,
  kX86InstOpCode_PP_F2     = 0x03U << kX86InstOpCode_PP_Shift,
  kX86InstOpCode_PP_9B     = 0x07U << kX86InstOpCode_PP_Shift, // AsmJit specific, not part of AVX.

  // `L` field in AVX/XOP/AVX-512 instruction.
  //
  // AVX/XOP can only use the first bit `L.128` or `L.256`. AVX-512 makes it
  // possible to use also `L.512`.

  // NOTE: If the instruction set manual describes an instruction by using `LIG`
  // it means that the `L` field is ignored. AsmJit emits `0` in such case.
  kX86InstOpCode_L_Shift   = 23,
  kX86InstOpCode_L_Mask    = 0x03U << kX86InstOpCode_L_Shift,
  kX86InstOpCode_L_128     = 0x00U << kX86InstOpCode_L_Shift,
  kX86InstOpCode_L_256     = 0x01U << kX86InstOpCode_L_Shift,
  kX86InstOpCode_L_512     = 0x02U << kX86InstOpCode_L_Shift,

  // `O` field (ModR/M).
  kX86InstOpCode_O_Shift   = 25,
  kX86InstOpCode_O_Mask    = 0x07U << kX86InstOpCode_O_Shift,

  // `W` field used by EVEX instruction encoding.
  kX86InstOpCode_EW_Shift  = 30,
  kX86InstOpCode_EW_Mask   = 0x01U << kX86InstOpCode_EW_Shift,
  kX86InstOpCode_EW        = 0x01U << kX86InstOpCode_EW_Shift,

  // `W` field used by REX/VEX instruction encoding.
  //
  // NOTE: If the instruction set manual describes an instruction by using `WIG`
  // it means that the `W` field is ignored. AsmJit emits `0` in such case.
  kX86InstOpCode_W_Shift   = 31,
  kX86InstOpCode_W_Mask    = 0x01U << kX86InstOpCode_W_Shift,
  kX86InstOpCode_W         = 0x01U << kX86InstOpCode_W_Shift,
};

// ============================================================================
// [asmjit::X86InstFlags]
// ============================================================================

//! \internal
//!
//! X86/X64 instruction flags.
ASMJIT_ENUM(X86InstFlags) {
  kX86InstFlagNone        = 0x00000000,  //!< No flags.

  kX86InstFlagRO          = 0x00000001,  //!< The first operand is read (read-only without `kX86InstFlagWO`).
  kX86InstFlagWO          = 0x00000002,  //!< The first operand is written (write-only without `kX86InstFlagRO`).
  kX86InstFlagRW          = 0x00000003,  //!< The first operand is read-write.

  kX86InstFlagXchg        = 0x00000004,  //!< Instruction is an exchange like instruction (xchg, xadd).
  kX86InstFlagFlow        = 0x00000008,  //!< Control-flow instruction (jmp, jcc, call, ret).

  kX86InstFlagFp          = 0x00000010,  //!< Instruction accesses FPU register(s).
  kX86InstFlagLock        = 0x00000020,  //!< Instruction can be prefixed by using the LOCK prefix.
  kX86InstFlagSpecial     = 0x00000040,  //!< Instruction requires special handling (implicit operands), used by \ref Compiler.

  //! Instruction always performs memory access.
  //!
  //! This flag is always combined with `kX86InstFlagSpecial` and describes
  //! that there is an implicit address which is accessed (usually EDI/RDI
  //! and/or ESI/RSI).
  kX86InstFlagSpecialMem  = 0x00000080,

  kX86InstFlagMem2        = 0x00000100,  //!< Instruction memory operand can refer to 16-bit address (used by FPU).
  kX86InstFlagMem4        = 0x00000200,  //!< Instruction memory operand can refer to 32-bit address (used by FPU).
  kX86InstFlagMem8        = 0x00000400,  //!< Instruction memory operand can refer to 64-bit address (used by FPU).
  kX86InstFlagMem10       = 0x00000800,  //!< Instruction memory operand can refer to 80-bit address (used by FPU).

  kX86InstFlagZeroIfMem   = 0x00001000,  //!< Cleans the rest of destination if source is memory (movss, movsd).
  kX86InstFlagVolatile    = 0x00002000,  //!< Hint for instruction scheduler to not reorder this instruction.

  kX86InstFlagAvx         = 0x00010000,  //!< AVX/AVX2 instruction.
  kX86InstFlagXop         = 0x00020000,  //!< XOP instruction.

  kX86InstFlagAvx512F     = 0x00100000,  //!< Supported by AVX-512 F (ZMM).
  kX86InstFlagAvx512CD    = 0x00200000,  //!< Supported by AVX-512 CD (ZMM).
  kX86InstFlagAvx512PF    = 0x00400000,  //!< Supported by AVX-512 PF (ZMM).
  kX86InstFlagAvx512ER    = 0x00800000,  //!< Supported by AVX-512 ER (ZMM).
  kX86InstFlagAvx512DQ    = 0x01000000,  //!< Supported by AVX-512 DQ (ZMM).
  kX86InstFlagAvx512BW    = 0x02000000,  //!< Supported by AVX-512 BW (ZMM).
  kX86InstFlagAvx512VL    = 0x04000000,  //!< Supported by AVX-512 VL (XMM/YMM).

  kX86InstFlagAvx512KMask = 0x08000000,  //!< Supports masking {k0..k7}.
  kX86InstFlagAvx512KZero = 0x10000000,  //!< Supports zeroing of elements {k0z..k7z}.
  kX86InstFlagAvx512BCast = 0x20000000,  //!< Supports broadcast {1..N}.
  kX86InstFlagAvx512Sae   = 0x40000000,  //!< Supports suppressing all exceptions {sae}.
  kX86InstFlagAvx512Rnd   = 0x80000000   //!< Supports static rounding control & SAE {rnd-sae},
};

// ============================================================================
// [asmjit::X86InstOp]
// ============================================================================

//! \internal
//!
//! X86/X64 instruction operand flags.
ASMJIT_ENUM(X86InstOp) {
  kX86InstOpGb            = 0x0001,      //!< Operand can be 8-bit GPB register.
  kX86InstOpGw            = 0x0002,      //!< Operand can be 16-bit GPW register.
  kX86InstOpGd            = 0x0004,      //!< Operand operand can be 32-bit GPD register.
  kX86InstOpGq            = 0x0008,      //!< Operand can be 64-bit GPQ register.
  kX86InstOpFp            = 0x0010,      //!< Operand can be FPU register.
  kX86InstOpMm            = 0x0020,      //!< Operand can be 64-bit MMX register.
  kX86InstOpK             = 0x0040,      //!< Operand can be 64-bit K register.

  kX86InstOpXmm           = 0x0100,      //!< Operand can be 128-bit XMM register.
  kX86InstOpYmm           = 0x0200,      //!< Operand can be 256-bit YMM register.
  kX86InstOpZmm           = 0x0400,      //!< Operand can be 512-bit ZMM register.

  kX86InstOpMem           = 0x1000,      //!< Operand can be memory.
  kX86InstOpImm           = 0x2000,      //!< Operand can be immediate.
  kX86InstOpLabel         = 0x4000,      //!< Operand can be label.

  //! Instruction operand doesn't have to be used.
  //!
  //! NOTE: If no operand is specified the meaning is clear (the operand at the
  //! particular index doesn't exist), however, when one or more operand is
  //! specified, it's not clear whether the operand can be omitted or not. When
  //! `kX86InstOpNone` is used it means that the operand is not used in some
  //! cases.
  kX86InstOpNone          = 0x8000
};

// ============================================================================
// [asmjit::X86Cond]
// ============================================================================

//! X86/X64 Condition codes.
ASMJIT_ENUM(X86Cond) {
  kX86CondA               = 0x07,        // CF==0 & ZF==0          (unsigned)
  kX86CondAE              = 0x03,        // CF==0                  (unsigned)
  kX86CondB               = 0x02,        // CF==1                  (unsigned)
  kX86CondBE              = 0x06,        // CF==1 | ZF==1          (unsigned)
  kX86CondC               = 0x02,        // CF==1
  kX86CondE               = 0x04,        //         ZF==1          (signed/unsigned)
  kX86CondG               = 0x0F,        //         ZF==0 & SF==OF (signed)
  kX86CondGE              = 0x0D,        //                 SF==OF (signed)
  kX86CondL               = 0x0C,        //                 SF!=OF (signed)
  kX86CondLE              = 0x0E,        //         ZF==1 | SF!=OF (signed)
  kX86CondNA              = 0x06,        // CF==1 | ZF==1          (unsigned)
  kX86CondNAE             = 0x02,        // CF==1                  (unsigned)
  kX86CondNB              = 0x03,        // CF==0                  (unsigned)
  kX86CondNBE             = 0x07,        // CF==0 & ZF==0          (unsigned)
  kX86CondNC              = 0x03,        // CF==0
  kX86CondNE              = 0x05,        //         ZF==0          (signed/unsigned)
  kX86CondNG              = 0x0E,        //         ZF==1 | SF!=OF (signed)
  kX86CondNGE             = 0x0C,        //                 SF!=OF (signed)
  kX86CondNL              = 0x0D,        //                 SF==OF (signed)
  kX86CondNLE             = 0x0F,        //         ZF==0 & SF==OF (signed)
  kX86CondNO              = 0x01,        //                 OF==0
  kX86CondNP              = 0x0B,        // PF==0
  kX86CondNS              = 0x09,        //                 SF==0
  kX86CondNZ              = 0x05,        //         ZF==0
  kX86CondO               = 0x00,        //                 OF==1
  kX86CondP               = 0x0A,        // PF==1
  kX86CondPE              = 0x0A,        // PF==1
  kX86CondPO              = 0x0B,        // PF==0
  kX86CondS               = 0x08,        //                 SF==1
  kX86CondZ               = 0x04,        //         ZF==1

  // Simplified condition codes.
  kX86CondSign            = kX86CondS,   //!< Sign (S).
  kX86CondNotSign         = kX86CondNS,  //!< Not Sign (NS).

  kX86CondOverflow        = kX86CondO,   //!< Signed  Overflow (O)
  kX86CondNotOverflow     = kX86CondNO,  //!< Not Signed Overflow (NO)

  kX86CondLess            = kX86CondL,   //!< Signed     `a <  b` (L  or NGE).
  kX86CondLessEqual       = kX86CondLE,  //!< Signed     `a <= b` (LE or NG ).
  kX86CondGreater         = kX86CondG,   //!< Signed     `a >  b` (G  or NLE).
  kX86CondGreaterEqual    = kX86CondGE,  //!< Signed     `a >= b` (GE or NL ).
  kX86CondBelow           = kX86CondB,   //!< Unsigned   `a <  b` (B  or NAE).
  kX86CondBelowEqual      = kX86CondBE,  //!< Unsigned   `a <= b` (BE or NA ).
  kX86CondAbove           = kX86CondA,   //!< Unsigned   `a >  b` (A  or NBE).
  kX86CondAboveEqual      = kX86CondAE,  //!< Unsigned   `a >= b` (AE or NB ).
  kX86CondEqual           = kX86CondE,   //!< Equal      `a == b` (E  or Z  ).
  kX86CondNotEqual        = kX86CondNE,  //!< Not Equal  `a != b` (NE or NZ ).

  kX86CondParityEven      = kX86CondP,
  kX86CondParityOdd       = kX86CondPO,

  // Aliases.
  kX86CondZero            = kX86CondZ,
  kX86CondNotZero         = kX86CondNZ,
  kX86CondNegative        = kX86CondS,
  kX86CondPositive        = kX86CondNS,

  // FPU-only.
  kX86CondFpuUnordered    = 0x10,
  kX86CondFpuNotUnordered = 0x11,

  //! No condition code.
  kX86CondNone            = 0x12
};

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
  // src-gendefs.js relies on the values of these masks, the tool has to be
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

//  ============================================================================
// [asmjit::X86Cmp]
//  ============================================================================

//! X86/X64 Comparison predicate used by CMP[PD/PS/SD/SS] family instructions.
ASMJIT_ENUM(X86Cmp) {
  kX86CmpEQ               = 0x00,        //!< Equal             (Quite).
  kX86CmpLT               = 0x01,        //!< Less              (Signaling).
  kX86CmpLE               = 0x02,        //!< Less/Equal        (Signaling).
  kX86CmpUNORD            = 0x03,        //!< Unordered         (Quite).
  kX86CmpNEQ              = 0x04,        //!< Not Equal         (Quite).
  kX86CmpNLT              = 0x05,        //!< Not Less          (Signaling).
  kX86CmpNLE              = 0x06,        //!< Not Less/Equal    (Signaling).
  kX86CmpORD              = 0x07         //!< Ordered           (Quite).
};

//  ============================================================================
// [asmjit::X86VCmp]
//  ============================================================================

//! X86/X64 Comparison predicate used by VCMP[PD/PS/SD/SS] family instructions.
//!
//! The first 8 are compatible with \ref X86Cmp.
ASMJIT_ENUM(X86VCmp) {
  kX86VCmpEQ_OQ           = 0x00,        //!< Equal             (Quite, Ordered).
  kX86VCmpLT_OS           = 0x01,        //!< Less              (Signaling, Ordered).
  kX86VCmpLE_OS           = 0x02,        //!< Less/Equal        (Signaling, Ordered).
  kX86VCmpUNORD_Q         = 0x03,        //!< Unordered         (Quite).
  kX86VCmpNEQ_UQ          = 0x04,        //!< Not Equal         (Quite, Unordered).
  kX86VCmpNLT_US          = 0x05,        //!< Not Less          (Signaling, Unordered).
  kX86VCmpNLE_US          = 0x06,        //!< Not Less/Equal    (Signaling, Unordered).
  kX86VCmpORD_Q           = 0x07,        //!< Ordered           (Quite).

  kX86VCmpEQ_UQ           = 0x08,        //!< Equal             (Quite, Unordered).
  kX86VCmpNGE_US          = 0x09,        //!< Not Greater/Equal (Signaling, Unordered).
  kX86VCmpNGT_US          = 0x0A,        //!< Not Greater       (Signaling, Unordered).
  kX86VCmpFALSE_OQ        = 0x0B,        //!< False             (Quite, Ordered).
  kX86VCmpNEQ_OQ          = 0x0C,        //!< Not Equal         (Quite, Ordered).
  kX86VCmpGE_OS           = 0x0D,        //!< Greater/Equal     (Signaling, Ordered).
  kX86VCmpGT_OS           = 0x0E,        //!< Greater           (Signaling, Ordered).
  kX86VCmpTRUE_UQ         = 0x0F,        //!< True              (Quite, Unordered).
  kX86VCmpEQ_OS           = 0x10,        //!< Equal             (Signaling, Ordered).
  kX86VCmpLT_OQ           = 0x11,        //!< Less              (Quite, Ordered).
  kX86VCmpLE_OQ           = 0x12,        //!< Less/Equal        (Quite, Ordered).
  kX86VCmpUNORD_S         = 0x13,        //!< Unordered         (Signaling).
  kX86VCmpNEQ_US          = 0x14,        //!< Not Equal         (Signaling, Unordered).
  kX86VCmpNLT_UQ          = 0x15,        //!< Not Less          (Quite, Unordered).
  kX86VCmpNLE_UQ          = 0x16,        //!< Not Less/Equal    (Quite, Unordered).
  kX86VCmpORD_S           = 0x17,        //!< Ordered           (Signaling).
  kX86VCmpEQ_US           = 0x18,        //!< Equal             (Signaling, Unordered).
  kX86VCmpNGE_UQ          = 0x19,        //!< Not Greater/Equal (Quite, Unordered).
  kX86VCmpNGT_UQ          = 0x1A,        //!< Not Greater       (Quite, Unordered).
  kX86VCmpFALSE_OS        = 0x1B,        //!< False             (Signaling, Ordered).
  kX86VCmpNEQ_OS          = 0x1C,        //!< Not Equal         (Signaling, Ordered).
  kX86VCmpGE_OQ           = 0x1D,        //!< Greater/Equal     (Quite, Ordered).
  kX86VCmpGT_OQ           = 0x1E,        //!< Greater           (Quite, Ordered).
  kX86VCmpTRUE_US         = 0x1F         //!< True              (Signaling, Unordered).
};

//  ============================================================================
// [asmjit::X86Round]
//  ============================================================================

//! X86/X64 round encoding used by ROUND[PD/PS/SD/SS] family instructions.
ASMJIT_ENUM(X86Round) {
  kX86RoundNearest        = 0x00,        //!< Round to nearest (even).
  kX86RoundDown           = 0x01,        //!< Round to down toward -INF (floor),
  kX86RoundUp             = 0x02,        //!< Round to up toward +INF (ceil).
  kX86RoundTrunc          = 0x03,        //!< Round toward zero (truncate).
  kX86RoundCurrent        = 0x04,        //!< Round to the current rounding mode set (ignores other RC bits).
  kX86RoundInexact        = 0x08         //!< Avoid the inexact exception, if set.
};

// ============================================================================
// [asmjit::X86Prefetch]
// ============================================================================

//! X86/X64 Prefetch hints.
ASMJIT_ENUM(X86Prefetch) {
  kX86PrefetchNTA         = 0,           //!< Prefetch by using NT hint.
  kX86PrefetchT0          = 1,           //!< Prefetch to L0 cache.
  kX86PrefetchT1          = 2,           //!< Prefetch to L1 cache.
  kX86PrefetchT2          = 3            //!< Prefetch to L2 cache.
};

// ============================================================================
// [asmjit::X86InstExtendedInfo]
// ============================================================================

//! X86/X64 instruction extended information.
//!
//! Extended information has been introduced to minimize data needed for a
//! single instruction, because two or more instructions can share the common
//! data, for example operands definition or secondary opcode, which is only
//! used by few instructions.
struct X86InstExtendedInfo {
  // --------------------------------------------------------------------------
  // [Accessors - Instruction Encoding]
  // --------------------------------------------------------------------------

  //! Get instruction encoding, see \ref kX86InstEncoding.
  ASMJIT_INLINE uint32_t getEncoding() const noexcept {
    return _encoding;
  }

  // --------------------------------------------------------------------------
  // [Accessors - Instruction Flags]
  // --------------------------------------------------------------------------

  //! Get whether the instruction has a `flag`, see `X86InstFlags`.
  ASMJIT_INLINE bool hasFlag(uint32_t flag) const noexcept {
    return (_instFlags & flag) != 0;
  }

  //! Get all instruction flags, see `X86InstFlags`.
  ASMJIT_INLINE uint32_t getFlags() const noexcept {
    return _instFlags;
  }

  //! Get if the first operand is read-only.
  ASMJIT_INLINE bool isRO() const noexcept {
    return (getFlags() & kX86InstFlagRW) == kX86InstFlagRO;
  }

  //! Get if the first operand is write-only.
  ASMJIT_INLINE bool isWO() const noexcept {
    return (getFlags() & kX86InstFlagRW) == kX86InstFlagWO;
  }

  //! Get if the first operand is read-write.
  ASMJIT_INLINE bool isRW() const noexcept {
    return (getFlags() & kX86InstFlagRW) == kX86InstFlagRW;
  }

  //! Get whether the instruction is a typical Exchange instruction.
  //!
  //! Exchange instructions are 'xchg' and 'xadd'.
  ASMJIT_INLINE bool isXchg() const noexcept {
    return hasFlag(kX86InstFlagXchg);
  }

  //! Get whether the instruction is a control-flow instruction.
  //!
  //! Control flow instruction is instruction that can perform a branch,
  //! typically `jmp`, `jcc`, `call`, or `ret`.
  ASMJIT_INLINE bool isFlow() const noexcept {
    return hasFlag(kX86InstFlagFlow);
  }

  //! Get whether the instruction accesses Fp register(s).
  ASMJIT_INLINE bool isFp() const noexcept {
    return hasFlag(kX86InstFlagFp);
  }

  //! Get whether the instruction can be prefixed by LOCK prefix.
  ASMJIT_INLINE bool isLockable() const noexcept {
    return hasFlag(kX86InstFlagLock);
  }

  //! Get whether the instruction is special type (this is used by `Compiler`
  //! to manage additional variables or functionality).
  ASMJIT_INLINE bool isSpecial() const noexcept {
    return hasFlag(kX86InstFlagSpecial);
  }

  //! Get whether the instruction is special type and it performs memory access.
  ASMJIT_INLINE bool isSpecialMem() const noexcept {
    return hasFlag(kX86InstFlagSpecialMem);
  }

  //! Get whether the move instruction zeroes the rest of the register
  //! if the source is memory operand.
  //!
  //! Basically flag needed only to support `movsd` and `movss` instructions.
  ASMJIT_INLINE bool isZeroIfMem() const noexcept {
    return hasFlag(kX86InstFlagZeroIfMem);
  }

  // --------------------------------------------------------------------------
  // [Accessors - EFlags]
  // --------------------------------------------------------------------------

  //! Get EFLAGS that the instruction reads, see \ref X86EFlags.
  ASMJIT_INLINE uint32_t getEFlagsIn() const noexcept {
    return _eflagsIn;
  }

  //! Get EFLAGS that the instruction writes, see \ref X86EFlags.
  ASMJIT_INLINE uint32_t getEFlagsOut() const noexcept {
    return _eflagsOut;
  }

  // --------------------------------------------------------------------------
  // [Accessors - Write Index/Size]
  // --------------------------------------------------------------------------

  //! Get the destination index of WRITE operation.
  ASMJIT_INLINE uint32_t getWriteIndex() const noexcept {
    return _writeIndex;
  }

  //! Get the number of bytes that will be written by a WRITE operation.
  ASMJIT_INLINE uint32_t getWriteSize() const noexcept {
    return _writeSize;
  }

  // --------------------------------------------------------------------------
  // [Accessors - Operand-Flags]
  // --------------------------------------------------------------------------

  //! Get flags of operand at index `index`.
  //!
  //! See \ref X86InstInfo::getOperandFlags() for more details.
  ASMJIT_INLINE uint16_t getOperandFlags(uint32_t index) const noexcept {
    ASMJIT_ASSERT(index < ASMJIT_ARRAY_SIZE(_opFlags));
    return _opFlags[index];
  }

  // --------------------------------------------------------------------------
  // [Accessors - OpCode]
  // --------------------------------------------------------------------------

  //! Get the secondary instruction opcode, see \ref X86InstOpCodeFlags.
  //!
  //! See \ref X86InstInfo::getSecondaryOpCode() for more details.
  ASMJIT_INLINE uint32_t getSecondaryOpCode() const noexcept {
    return _secondaryOpCode;
  }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! Instruction encoding.
  uint8_t _encoding;

  //! Destination byte of WRITE operation, default 0.
  uint8_t _writeIndex;

  //! Count of bytes affected by a write operation, needed by analysis for all
  //! instructions that do not read the overwritten register. Only used with
  //! `kX86InstFlagWO` flag. If `_writeSize` is zero it is automatically deduced
  //! from the size of the destination register.
  //!
  //! In general most of SSE write-only instructions should use 16 bytes as
  //! this is the size of the register (and of YMM/ZMM registers). This means
  //! that 16-bytes of the register are changed, the rest remains unchanged.
  //! However, AVX instructions should use the size of ZMM register as every
  //! AVX instruction clears the rest of the register (AVX/AVX2 instructions
  //! zero the HI part of ZMM if available).
  uint8_t _writeSize;

  //! EFlags read by the instruction.
  uint8_t _eflagsIn;
  //! EFlags written by the instruction.
  uint8_t _eflagsOut;

  //! \internal
  uint8_t _reserved;

  //! Operands' flags, up to 5 operands.
  uint16_t _opFlags[5];

  //! Instruction flags.
  uint32_t _instFlags;

  //! Secondary opcode.
  uint32_t _secondaryOpCode;
};

// ============================================================================
// [asmjit::X86InstInfo]
// ============================================================================

//! X86/X64 instruction information.
struct X86InstInfo {
  // --------------------------------------------------------------------------
  // [Accessors - Extended-Info]
  // --------------------------------------------------------------------------

  //! Get `X86InstExtendedInfo` for this instruction.
  ASMJIT_INLINE const X86InstExtendedInfo& getExtendedInfo() const noexcept {
    return _x86InstExtendedInfo[_extendedIndex];
  }

  //! Get index to the `_x86InstExtendedInfo` table.
  ASMJIT_INLINE uint32_t _getExtendedIndex() const noexcept {
    return _extendedIndex;
  }

  // --------------------------------------------------------------------------
  // [Accessors - Instruction Encoding]
  // --------------------------------------------------------------------------

  //! Get instruction group, see \ref X86InstEncoding.
  ASMJIT_INLINE uint32_t getEncoding() const noexcept {
    return getExtendedInfo().getEncoding();
  }

  // --------------------------------------------------------------------------
  // [Accessors - Instruction Flags]
  // --------------------------------------------------------------------------

  //! Get whether the instruction has flag `flag`, see `X86InstFlags`.
  ASMJIT_INLINE bool hasFlag(uint32_t flag) const noexcept {
    return (getFlags() & flag) != 0;
  }

  //! Get instruction flags, see `X86InstFlags`.
  ASMJIT_INLINE uint32_t getFlags() const noexcept {
    return getExtendedInfo().getFlags();
  }

  // --------------------------------------------------------------------------
  // [Accessors - EFlags]
  // --------------------------------------------------------------------------

  //! Get EFLAGS that the instruction reads, see \ref X86EFlags.
  ASMJIT_INLINE uint32_t getEFlagsIn() const noexcept {
    return getExtendedInfo().getEFlagsIn();
  }

  //! Get EFLAGS that the instruction writes, see \ref X86EFlags.
  ASMJIT_INLINE uint32_t getEFlagsOut() const noexcept {
    return getExtendedInfo().getEFlagsOut();
  }

  // --------------------------------------------------------------------------
  // [Accessors - Write Index/Size]
  // --------------------------------------------------------------------------

  //! Get the destination index of WRITE operation.
  ASMJIT_INLINE uint32_t getWriteIndex() const noexcept {
    return getExtendedInfo().getWriteIndex();
  }

  //! Get the number of bytes that will be written by a WRITE operation.
  ASMJIT_INLINE uint32_t getWriteSize() const noexcept {
    return getExtendedInfo().getWriteSize();
  }

  // --------------------------------------------------------------------------
  // [Accessors - Operand-Flags]
  // --------------------------------------------------------------------------

  //! Get flags of operand at index `index`.
  ASMJIT_INLINE uint32_t getOperandFlags(uint32_t index) const noexcept {
    return getExtendedInfo().getOperandFlags(index);
  }

  // --------------------------------------------------------------------------
  // [Accessors - OpCode]
  // --------------------------------------------------------------------------

  //! Get the primary instruction opcode, see \ref X86InstOpCodeFlags.
  ASMJIT_INLINE uint32_t getPrimaryOpCode() const noexcept {
    return _primaryOpCode;
  }

  //! Get the secondary instruction opcode, see \ref X86InstOpCodeFlags.
  ASMJIT_INLINE uint32_t getSecondaryOpCode() const noexcept {
    return getExtendedInfo().getSecondaryOpCode();
  }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! \internal
  uint16_t _reserved;
  //! Extended information name index in `_x86InstExtendedInfo[]` array.
  uint16_t _extendedIndex;

  //! Primary opcode, secondary opcode is stored in `X86InstExtendedInfo` table.
  uint32_t _primaryOpCode;
};

// ============================================================================
// [asmjit::X86Util]
// ============================================================================

struct X86Util {
  // --------------------------------------------------------------------------
  // [Instruction Id <-> Name]
  // --------------------------------------------------------------------------

#if !defined(ASMJIT_DISABLE_TEXT)
  //! Get an instruction ID from a given instruction `name`.
  //!
  //! If there is an exact match the instruction id is returned, otherwise
  //! `kInstIdNone` (zero) is returned.
  //!
  //! The given `name` doesn't have to be null-terminated if `len` is provided.
  ASMJIT_API static uint32_t getInstIdByName(const char* name, size_t len = kInvalidIndex) noexcept;

  //! Get an instruction name from a given instruction `id`.
  ASMJIT_API static const char* getInstNameById(uint32_t id) noexcept;
#endif // !ASMJIT_DISABLE_TEXT

  // --------------------------------------------------------------------------
  // [Instruction Info]
  // --------------------------------------------------------------------------

  //! Get instruction information based on `instId`.
  //!
  //! NOTE: `instId` has to be valid instruction ID, it can't be greater than
  //! or equal to `_kX86InstIdCount`. It asserts in debug mode.
  static ASMJIT_INLINE const X86InstInfo& getInstInfo(uint32_t instId) noexcept {
    ASMJIT_ASSERT(instId < _kX86InstIdCount);
    return _x86InstInfo[instId];
  }

  // --------------------------------------------------------------------------
  // [Condition Codes]
  // --------------------------------------------------------------------------

  //! Corresponds to transposing the operands of a comparison.
  static ASMJIT_INLINE uint32_t reverseCond(uint32_t cond) noexcept {
    ASMJIT_ASSERT(cond < ASMJIT_ARRAY_SIZE(_x86ReverseCond));
    return _x86ReverseCond[cond];
  }

  //! Get the equivalent of negated condition code.
  static ASMJIT_INLINE uint32_t negateCond(uint32_t cond) noexcept {
    ASMJIT_ASSERT(cond < ASMJIT_ARRAY_SIZE(_x86ReverseCond));
    return cond ^ static_cast<uint32_t>(cond < kX86CondNone);
  }

  //! Translate condition code `cc` to `cmovcc` instruction code.
  //! \sa \ref X86InstId, \ref _kX86InstIdCmovcc.
  static ASMJIT_INLINE uint32_t condToCmovcc(uint32_t cond) noexcept {
    ASMJIT_ASSERT(static_cast<uint32_t>(cond) < ASMJIT_ARRAY_SIZE(_x86CondToCmovcc));
    return _x86CondToCmovcc[cond];
  }

  //! Translate condition code `cc` to `jcc` instruction code.
  //! \sa \ref X86InstId, \ref _kX86InstIdJcc.
  static ASMJIT_INLINE uint32_t condToJcc(uint32_t cond) noexcept {
    ASMJIT_ASSERT(static_cast<uint32_t>(cond) < ASMJIT_ARRAY_SIZE(_x86CondToJcc));
    return _x86CondToJcc[cond];
  }

  //! Translate condition code `cc` to `setcc` instruction code.
  //! \sa \ref X86InstId, \ref _kX86InstIdSetcc.
  static ASMJIT_INLINE uint32_t condToSetcc(uint32_t cond) noexcept {
    ASMJIT_ASSERT(static_cast<uint32_t>(cond) < ASMJIT_ARRAY_SIZE(_x86CondToSetcc));
    return _x86CondToSetcc[cond];
  }

  // --------------------------------------------------------------------------
  // [Shuffle (SIMD)]
  // --------------------------------------------------------------------------

  //! Pack a shuffle constant to be used with multimedia instructions (2 values).
  //!
  //! \param a Position of the first component [0, 1], inclusive.
  //! \param b Position of the second component [0, 1], inclusive.
  //!
  //! Shuffle constants can be used to encode an immediate for these instructions:
  //!   - `shufpd`
  static ASMJIT_INLINE int shuffle(uint32_t a, uint32_t b) noexcept {
    ASMJIT_ASSERT(a <= 0x1 && b <= 0x1);
    uint32_t result = (a << 1) | b;
    return static_cast<int>(result);
  }

  //! Pack a shuffle constant to be used with multimedia instructions (4 values).
  //!
  //! \param a Position of the first component [0, 3], inclusive.
  //! \param b Position of the second component [0, 3], inclusive.
  //! \param c Position of the third component [0, 3], inclusive.
  //! \param d Position of the fourth component [0, 3], inclusive.
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
#include "../apiend.h"

// [Guard]
#endif // _ASMJIT_X86_X86INST_H
