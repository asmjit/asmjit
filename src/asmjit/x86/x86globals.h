// [AsmJit]
// Machine Code Generation for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

#ifndef _ASMJIT_X86_X86GLOBALS_H
#define _ASMJIT_X86_X86GLOBALS_H

#include "../core/arch.h"
#include "../core/inst.h"

ASMJIT_BEGIN_SUB_NAMESPACE(x86)

//! \namespace asmjit::x86
//! \ingroup asmjit_x86
//!
//! X86/X64 API.

//! \addtogroup asmjit_x86
//! \{

// ============================================================================
// [asmjit::x86::Inst]
// ============================================================================

//! Instruction (X86).
//!
//! \note Only used to hold x86-specific enumerations and static functions.
struct Inst : public BaseInst {
  //! Instruction id (X86).
  enum Id : uint32_t {
    // ${InstId:Begin}
    kIdNone = 0,
    kIdAaa,                              // <X86>
    kIdAad,                              // <X86>
    kIdAam,                              // <X86>
    kIdAas,                              // <X86>
    kIdAdc,                              // <ANY>
    kIdAdcx,                             // {ADX}
    kIdAdd,                              // <ANY>
    kIdAddpd,                            // {SSE2}
    kIdAddps,                            // {SSE}
    kIdAddsd,                            // {SSE2}
    kIdAddss,                            // {SSE}
    kIdAddsubpd,                         // {SSE3}
    kIdAddsubps,                         // {SSE3}
    kIdAdox,                             // {ADX}
    kIdAesdec,                           // {AESNI}
    kIdAesdeclast,                       // {AESNI}
    kIdAesenc,                           // {AESNI}
    kIdAesenclast,                       // {AESNI}
    kIdAesimc,                           // {AESNI}
    kIdAeskeygenassist,                  // {AESNI}
    kIdAnd,                              // <ANY>
    kIdAndn,                             // {BMI}
    kIdAndnpd,                           // {SSE2}
    kIdAndnps,                           // {SSE}
    kIdAndpd,                            // {SSE2}
    kIdAndps,                            // {SSE}
    kIdArpl,                             // <X86>
    kIdBextr,                            // {BMI}
    kIdBlcfill,                          // {TBM}
    kIdBlci,                             // {TBM}
    kIdBlcic,                            // {TBM}
    kIdBlcmsk,                           // {TBM}
    kIdBlcs,                             // {TBM}
    kIdBlendpd,                          // {SSE4_1}
    kIdBlendps,                          // {SSE4_1}
    kIdBlendvpd,                         // {SSE4_1}
    kIdBlendvps,                         // {SSE4_1}
    kIdBlsfill,                          // {TBM}
    kIdBlsi,                             // {BMI}
    kIdBlsic,                            // {TBM}
    kIdBlsmsk,                           // {BMI}
    kIdBlsr,                             // {BMI}
    kIdBndcl,                            // {MPX}
    kIdBndcn,                            // {MPX}
    kIdBndcu,                            // {MPX}
    kIdBndldx,                           // {MPX}
    kIdBndmk,                            // {MPX}
    kIdBndmov,                           // {MPX}
    kIdBndstx,                           // {MPX}
    kIdBound,                            // <X86>
    kIdBsf,                              // <ANY>
    kIdBsr,                              // <ANY>
    kIdBswap,                            // <ANY>
    kIdBt,                               // <ANY>
    kIdBtc,                              // <ANY>
    kIdBtr,                              // <ANY>
    kIdBts,                              // <ANY>
    kIdBzhi,                             // {BMI2}
    kIdCall,                             // <ANY>
    kIdCbw,                              // <ANY>
    kIdCdq,                              // <ANY>
    kIdCdqe,                             // <X64>
    kIdClac,                             // {SMAP}
    kIdClc,                              // <ANY>
    kIdCld,                              // <ANY>
    kIdCldemote,                         // {CLDEMOTE}
    kIdClflush,                          // {CLFLUSH}
    kIdClflushopt,                       // {CLFLUSHOPT}
    kIdClgi,                             // {SVM}
    kIdCli,                              // <ANY>
    kIdClts,                             // <ANY>
    kIdClwb,                             // {CLWB}
    kIdClzero,                           // {CLZERO}
    kIdCmc,                              // <ANY>
    kIdCmova,                            // {CMOV}
    kIdCmovae,                           // {CMOV}
    kIdCmovb,                            // {CMOV}
    kIdCmovbe,                           // {CMOV}
    kIdCmovc,                            // {CMOV}
    kIdCmove,                            // {CMOV}
    kIdCmovg,                            // {CMOV}
    kIdCmovge,                           // {CMOV}
    kIdCmovl,                            // {CMOV}
    kIdCmovle,                           // {CMOV}
    kIdCmovna,                           // {CMOV}
    kIdCmovnae,                          // {CMOV}
    kIdCmovnb,                           // {CMOV}
    kIdCmovnbe,                          // {CMOV}
    kIdCmovnc,                           // {CMOV}
    kIdCmovne,                           // {CMOV}
    kIdCmovng,                           // {CMOV}
    kIdCmovnge,                          // {CMOV}
    kIdCmovnl,                           // {CMOV}
    kIdCmovnle,                          // {CMOV}
    kIdCmovno,                           // {CMOV}
    kIdCmovnp,                           // {CMOV}
    kIdCmovns,                           // {CMOV}
    kIdCmovnz,                           // {CMOV}
    kIdCmovo,                            // {CMOV}
    kIdCmovp,                            // {CMOV}
    kIdCmovpe,                           // {CMOV}
    kIdCmovpo,                           // {CMOV}
    kIdCmovs,                            // {CMOV}
    kIdCmovz,                            // {CMOV}
    kIdCmp,                              // <ANY>
    kIdCmppd,                            // {SSE2}
    kIdCmpps,                            // {SSE}
    kIdCmps,                             // <ANY>
    kIdCmpsd,                            // {SSE2}
    kIdCmpss,                            // {SSE}
    kIdCmpxchg,                          // {I486}
    kIdCmpxchg16b,                       // {CMPXCHG16B} & <X64>
    kIdCmpxchg8b,                        // {CMPXCHG8B}
    kIdComisd,                           // {SSE2}
    kIdComiss,                           // {SSE}
    kIdCpuid,                            // {I486}
    kIdCqo,                              // <X64>
    kIdCrc32,                            // {SSE4_2}
    kIdCvtdq2pd,                         // {SSE2}
    kIdCvtdq2ps,                         // {SSE2}
    kIdCvtpd2dq,                         // {SSE2}
    kIdCvtpd2pi,                         // {SSE2}
    kIdCvtpd2ps,                         // {SSE2}
    kIdCvtpi2pd,                         // {SSE2}
    kIdCvtpi2ps,                         // {SSE}
    kIdCvtps2dq,                         // {SSE2}
    kIdCvtps2pd,                         // {SSE2}
    kIdCvtps2pi,                         // {SSE}
    kIdCvtsd2si,                         // {SSE2}
    kIdCvtsd2ss,                         // {SSE2}
    kIdCvtsi2sd,                         // {SSE2}
    kIdCvtsi2ss,                         // {SSE}
    kIdCvtss2sd,                         // {SSE2}
    kIdCvtss2si,                         // {SSE}
    kIdCvttpd2dq,                        // {SSE2}
    kIdCvttpd2pi,                        // {SSE2}
    kIdCvttps2dq,                        // {SSE2}
    kIdCvttps2pi,                        // {SSE}
    kIdCvttsd2si,                        // {SSE2}
    kIdCvttss2si,                        // {SSE}
    kIdCwd,                              // <ANY>
    kIdCwde,                             // <ANY>
    kIdDaa,                              // <X86>
    kIdDas,                              // <X86>
    kIdDec,                              // <ANY>
    kIdDiv,                              // <ANY>
    kIdDivpd,                            // {SSE2}
    kIdDivps,                            // {SSE}
    kIdDivsd,                            // {SSE2}
    kIdDivss,                            // {SSE}
    kIdDppd,                             // {SSE4_1}
    kIdDpps,                             // {SSE4_1}
    kIdEmms,                             // {MMX}
    kIdEnqcmd,                           // {ENQCMD}
    kIdEnqcmds,                          // {ENQCMD}
    kIdEnter,                            // <ANY>
    kIdExtractps,                        // {SSE4_1}
    kIdExtrq,                            // {SSE4A}
    kIdF2xm1,                            // <ANY>
    kIdFabs,                             // <ANY>
    kIdFadd,                             // <ANY>
    kIdFaddp,                            // <ANY>
    kIdFbld,                             // <ANY>
    kIdFbstp,                            // <ANY>
    kIdFchs,                             // <ANY>
    kIdFclex,                            // <ANY>
    kIdFcmovb,                           // {CMOV}
    kIdFcmovbe,                          // {CMOV}
    kIdFcmove,                           // {CMOV}
    kIdFcmovnb,                          // {CMOV}
    kIdFcmovnbe,                         // {CMOV}
    kIdFcmovne,                          // {CMOV}
    kIdFcmovnu,                          // {CMOV}
    kIdFcmovu,                           // {CMOV}
    kIdFcom,                             // <ANY>
    kIdFcomi,                            // <ANY>
    kIdFcomip,                           // <ANY>
    kIdFcomp,                            // <ANY>
    kIdFcompp,                           // <ANY>
    kIdFcos,                             // <ANY>
    kIdFdecstp,                          // <ANY>
    kIdFdiv,                             // <ANY>
    kIdFdivp,                            // <ANY>
    kIdFdivr,                            // <ANY>
    kIdFdivrp,                           // <ANY>
    kIdFemms,                            // {3DNOW}
    kIdFfree,                            // <ANY>
    kIdFiadd,                            // <ANY>
    kIdFicom,                            // <ANY>
    kIdFicomp,                           // <ANY>
    kIdFidiv,                            // <ANY>
    kIdFidivr,                           // <ANY>
    kIdFild,                             // <ANY>
    kIdFimul,                            // <ANY>
    kIdFincstp,                          // <ANY>
    kIdFinit,                            // <ANY>
    kIdFist,                             // <ANY>
    kIdFistp,                            // <ANY>
    kIdFisttp,                           // {SSE3}
    kIdFisub,                            // <ANY>
    kIdFisubr,                           // <ANY>
    kIdFld,                              // <ANY>
    kIdFld1,                             // <ANY>
    kIdFldcw,                            // <ANY>
    kIdFldenv,                           // <ANY>
    kIdFldl2e,                           // <ANY>
    kIdFldl2t,                           // <ANY>
    kIdFldlg2,                           // <ANY>
    kIdFldln2,                           // <ANY>
    kIdFldpi,                            // <ANY>
    kIdFldz,                             // <ANY>
    kIdFmul,                             // <ANY>
    kIdFmulp,                            // <ANY>
    kIdFnclex,                           // <ANY>
    kIdFninit,                           // <ANY>
    kIdFnop,                             // <ANY>
    kIdFnsave,                           // <ANY>
    kIdFnstcw,                           // <ANY>
    kIdFnstenv,                          // <ANY>
    kIdFnstsw,                           // <ANY>
    kIdFpatan,                           // <ANY>
    kIdFprem,                            // <ANY>
    kIdFprem1,                           // <ANY>
    kIdFptan,                            // <ANY>
    kIdFrndint,                          // <ANY>
    kIdFrstor,                           // <ANY>
    kIdFsave,                            // <ANY>
    kIdFscale,                           // <ANY>
    kIdFsin,                             // <ANY>
    kIdFsincos,                          // <ANY>
    kIdFsqrt,                            // <ANY>
    kIdFst,                              // <ANY>
    kIdFstcw,                            // <ANY>
    kIdFstenv,                           // <ANY>
    kIdFstp,                             // <ANY>
    kIdFstsw,                            // <ANY>
    kIdFsub,                             // <ANY>
    kIdFsubp,                            // <ANY>
    kIdFsubr,                            // <ANY>
    kIdFsubrp,                           // <ANY>
    kIdFtst,                             // <ANY>
    kIdFucom,                            // <ANY>
    kIdFucomi,                           // <ANY>
    kIdFucomip,                          // <ANY>
    kIdFucomp,                           // <ANY>
    kIdFucompp,                          // <ANY>
    kIdFwait,                            // <ANY>
    kIdFxam,                             // <ANY>
    kIdFxch,                             // <ANY>
    kIdFxrstor,                          // {FXSR}
    kIdFxrstor64,                        // {FXSR} & <X64>
    kIdFxsave,                           // {FXSR}
    kIdFxsave64,                         // {FXSR} & <X64>
    kIdFxtract,                          // <ANY>
    kIdFyl2x,                            // <ANY>
    kIdFyl2xp1,                          // <ANY>
    kIdGetsec,                           // {SMX}
    kIdGf2p8affineinvqb,                 // {GFNI}
    kIdGf2p8affineqb,                    // {GFNI}
    kIdGf2p8mulb,                        // {GFNI}
    kIdHaddpd,                           // {SSE3}
    kIdHaddps,                           // {SSE3}
    kIdHlt,                              // <ANY>
    kIdHsubpd,                           // {SSE3}
    kIdHsubps,                           // {SSE3}
    kIdIdiv,                             // <ANY>
    kIdImul,                             // <ANY>
    kIdIn,                               // <ANY>
    kIdInc,                              // <ANY>
    kIdIns,                              // <ANY>
    kIdInsertps,                         // {SSE4_1}
    kIdInsertq,                          // {SSE4A}
    kIdInt,                              // <ANY>
    kIdInt3,                             // <ANY>
    kIdInto,                             // <X86>
    kIdInvd,                             // {I486}
    kIdInvept,                           // {VMX}
    kIdInvlpg,                           // {I486}
    kIdInvlpga,                          // {SVM}
    kIdInvpcid,                          // {I486}
    kIdInvvpid,                          // {VMX}
    kIdIret,                             // <ANY>
    kIdIretd,                            // <ANY>
    kIdIretq,                            // <X64>
    kIdIretw,                            // <ANY>
    kIdJa,                               // <ANY>
    kIdJae,                              // <ANY>
    kIdJb,                               // <ANY>
    kIdJbe,                              // <ANY>
    kIdJc,                               // <ANY>
    kIdJe,                               // <ANY>
    kIdJecxz,                            // <ANY>
    kIdJg,                               // <ANY>
    kIdJge,                              // <ANY>
    kIdJl,                               // <ANY>
    kIdJle,                              // <ANY>
    kIdJmp,                              // <ANY>
    kIdJna,                              // <ANY>
    kIdJnae,                             // <ANY>
    kIdJnb,                              // <ANY>
    kIdJnbe,                             // <ANY>
    kIdJnc,                              // <ANY>
    kIdJne,                              // <ANY>
    kIdJng,                              // <ANY>
    kIdJnge,                             // <ANY>
    kIdJnl,                              // <ANY>
    kIdJnle,                             // <ANY>
    kIdJno,                              // <ANY>
    kIdJnp,                              // <ANY>
    kIdJns,                              // <ANY>
    kIdJnz,                              // <ANY>
    kIdJo,                               // <ANY>
    kIdJp,                               // <ANY>
    kIdJpe,                              // <ANY>
    kIdJpo,                              // <ANY>
    kIdJs,                               // <ANY>
    kIdJz,                               // <ANY>
    kIdKaddb,                            // {AVX512_DQ}
    kIdKaddd,                            // {AVX512_BW}
    kIdKaddq,                            // {AVX512_BW}
    kIdKaddw,                            // {AVX512_DQ}
    kIdKandb,                            // {AVX512_DQ}
    kIdKandd,                            // {AVX512_BW}
    kIdKandnb,                           // {AVX512_DQ}
    kIdKandnd,                           // {AVX512_BW}
    kIdKandnq,                           // {AVX512_BW}
    kIdKandnw,                           // {AVX512_F}
    kIdKandq,                            // {AVX512_BW}
    kIdKandw,                            // {AVX512_F}
    kIdKmovb,                            // {AVX512_DQ}
    kIdKmovd,                            // {AVX512_BW}
    kIdKmovq,                            // {AVX512_BW}
    kIdKmovw,                            // {AVX512_F}
    kIdKnotb,                            // {AVX512_DQ}
    kIdKnotd,                            // {AVX512_BW}
    kIdKnotq,                            // {AVX512_BW}
    kIdKnotw,                            // {AVX512_F}
    kIdKorb,                             // {AVX512_DQ}
    kIdKord,                             // {AVX512_BW}
    kIdKorq,                             // {AVX512_BW}
    kIdKortestb,                         // {AVX512_DQ}
    kIdKortestd,                         // {AVX512_BW}
    kIdKortestq,                         // {AVX512_BW}
    kIdKortestw,                         // {AVX512_F}
    kIdKorw,                             // {AVX512_F}
    kIdKshiftlb,                         // {AVX512_DQ}
    kIdKshiftld,                         // {AVX512_BW}
    kIdKshiftlq,                         // {AVX512_BW}
    kIdKshiftlw,                         // {AVX512_F}
    kIdKshiftrb,                         // {AVX512_DQ}
    kIdKshiftrd,                         // {AVX512_BW}
    kIdKshiftrq,                         // {AVX512_BW}
    kIdKshiftrw,                         // {AVX512_F}
    kIdKtestb,                           // {AVX512_DQ}
    kIdKtestd,                           // {AVX512_BW}
    kIdKtestq,                           // {AVX512_BW}
    kIdKtestw,                           // {AVX512_DQ}
    kIdKunpckbw,                         // {AVX512_F}
    kIdKunpckdq,                         // {AVX512_BW}
    kIdKunpckwd,                         // {AVX512_BW}
    kIdKxnorb,                           // {AVX512_DQ}
    kIdKxnord,                           // {AVX512_BW}
    kIdKxnorq,                           // {AVX512_BW}
    kIdKxnorw,                           // {AVX512_F}
    kIdKxorb,                            // {AVX512_DQ}
    kIdKxord,                            // {AVX512_BW}
    kIdKxorq,                            // {AVX512_BW}
    kIdKxorw,                            // {AVX512_F}
    kIdLahf,                             // {LAHFSAHF}
    kIdLar,                              // <ANY>
    kIdLddqu,                            // {SSE3}
    kIdLdmxcsr,                          // {SSE}
    kIdLds,                              // <X86>
    kIdLea,                              // <ANY>
    kIdLeave,                            // <ANY>
    kIdLes,                              // <X86>
    kIdLfence,                           // {SSE2}
    kIdLfs,                              // <ANY>
    kIdLgdt,                             // <ANY>
    kIdLgs,                              // <ANY>
    kIdLidt,                             // <ANY>
    kIdLldt,                             // <ANY>
    kIdLlwpcb,                           // {LWP}
    kIdLmsw,                             // <ANY>
    kIdLods,                             // <ANY>
    kIdLoop,                             // <ANY>
    kIdLoope,                            // <ANY>
    kIdLoopne,                           // <ANY>
    kIdLsl,                              // <ANY>
    kIdLss,                              // <ANY>
    kIdLtr,                              // <ANY>
    kIdLwpins,                           // {LWP}
    kIdLwpval,                           // {LWP}
    kIdLzcnt,                            // {LZCNT}
    kIdMaskmovdqu,                       // {SSE2}
    kIdMaskmovq,                         // {MMX2}
    kIdMaxpd,                            // {SSE2}
    kIdMaxps,                            // {SSE}
    kIdMaxsd,                            // {SSE2}
    kIdMaxss,                            // {SSE}
    kIdMfence,                           // {SSE2}
    kIdMinpd,                            // {SSE2}
    kIdMinps,                            // {SSE}
    kIdMinsd,                            // {SSE2}
    kIdMinss,                            // {SSE}
    kIdMonitor,                          // {MONITOR}
    kIdMonitorx,                         // {MONITORX}
    kIdMov,                              // <ANY>
    kIdMovapd,                           // {SSE2}
    kIdMovaps,                           // {SSE}
    kIdMovbe,                            // {MOVBE}
    kIdMovd,                             // {MMX|SSE2}
    kIdMovddup,                          // {SSE3}
    kIdMovdir64b,                        // {MOVDIR64B}
    kIdMovdiri,                          // {MOVDIRI}
    kIdMovdq2q,                          // {SSE2}
    kIdMovdqa,                           // {SSE2}
    kIdMovdqu,                           // {SSE2}
    kIdMovhlps,                          // {SSE}
    kIdMovhpd,                           // {SSE2}
    kIdMovhps,                           // {SSE}
    kIdMovlhps,                          // {SSE}
    kIdMovlpd,                           // {SSE2}
    kIdMovlps,                           // {SSE}
    kIdMovmskpd,                         // {SSE2}
    kIdMovmskps,                         // {SSE}
    kIdMovntdq,                          // {SSE2}
    kIdMovntdqa,                         // {SSE4_1}
    kIdMovnti,                           // {SSE2}
    kIdMovntpd,                          // {SSE2}
    kIdMovntps,                          // {SSE}
    kIdMovntq,                           // {MMX2}
    kIdMovntsd,                          // {SSE4A}
    kIdMovntss,                          // {SSE4A}
    kIdMovq,                             // {MMX|SSE2}
    kIdMovq2dq,                          // {SSE2}
    kIdMovs,                             // <ANY>
    kIdMovsd,                            // {SSE2}
    kIdMovshdup,                         // {SSE3}
    kIdMovsldup,                         // {SSE3}
    kIdMovss,                            // {SSE}
    kIdMovsx,                            // <ANY>
    kIdMovsxd,                           // <X64>
    kIdMovupd,                           // {SSE2}
    kIdMovups,                           // {SSE}
    kIdMovzx,                            // <ANY>
    kIdMpsadbw,                          // {SSE4_1}
    kIdMul,                              // <ANY>
    kIdMulpd,                            // {SSE2}
    kIdMulps,                            // {SSE}
    kIdMulsd,                            // {SSE2}
    kIdMulss,                            // {SSE}
    kIdMulx,                             // {BMI2}
    kIdMwait,                            // {MONITOR}
    kIdMwaitx,                           // {MONITORX}
    kIdNeg,                              // <ANY>
    kIdNop,                              // <ANY>
    kIdNot,                              // <ANY>
    kIdOr,                               // <ANY>
    kIdOrpd,                             // {SSE2}
    kIdOrps,                             // {SSE}
    kIdOut,                              // <ANY>
    kIdOuts,                             // <ANY>
    kIdPabsb,                            // {SSSE3}
    kIdPabsd,                            // {SSSE3}
    kIdPabsw,                            // {SSSE3}
    kIdPackssdw,                         // {MMX|SSE2}
    kIdPacksswb,                         // {MMX|SSE2}
    kIdPackusdw,                         // {SSE4_1}
    kIdPackuswb,                         // {MMX|SSE2}
    kIdPaddb,                            // {MMX|SSE2}
    kIdPaddd,                            // {MMX|SSE2}
    kIdPaddq,                            // {SSE2}
    kIdPaddsb,                           // {MMX|SSE2}
    kIdPaddsw,                           // {MMX|SSE2}
    kIdPaddusb,                          // {MMX|SSE2}
    kIdPaddusw,                          // {MMX|SSE2}
    kIdPaddw,                            // {MMX|SSE2}
    kIdPalignr,                          // {SSE3}
    kIdPand,                             // {MMX|SSE2}
    kIdPandn,                            // {MMX|SSE2}
    kIdPause,                            // <ANY>
    kIdPavgb,                            // {MMX2|SSE2}
    kIdPavgusb,                          // {3DNOW}
    kIdPavgw,                            // {MMX2|SSE2}
    kIdPblendvb,                         // {SSE4_1}
    kIdPblendw,                          // {SSE4_1}
    kIdPclmulqdq,                        // {PCLMULQDQ}
    kIdPcmpeqb,                          // {MMX|SSE2}
    kIdPcmpeqd,                          // {MMX|SSE2}
    kIdPcmpeqq,                          // {SSE4_1}
    kIdPcmpeqw,                          // {MMX|SSE2}
    kIdPcmpestri,                        // {SSE4_2}
    kIdPcmpestrm,                        // {SSE4_2}
    kIdPcmpgtb,                          // {MMX|SSE2}
    kIdPcmpgtd,                          // {MMX|SSE2}
    kIdPcmpgtq,                          // {SSE4_2}
    kIdPcmpgtw,                          // {MMX|SSE2}
    kIdPcmpistri,                        // {SSE4_2}
    kIdPcmpistrm,                        // {SSE4_2}
    kIdPcommit,                          // {PCOMMIT}
    kIdPdep,                             // {BMI2}
    kIdPext,                             // {BMI2}
    kIdPextrb,                           // {SSE4_1}
    kIdPextrd,                           // {SSE4_1}
    kIdPextrq,                           // {SSE4_1} & <X64>
    kIdPextrw,                           // {MMX2|SSE2|SSE4_1}
    kIdPf2id,                            // {3DNOW}
    kIdPf2iw,                            // {3DNOW2}
    kIdPfacc,                            // {3DNOW}
    kIdPfadd,                            // {3DNOW}
    kIdPfcmpeq,                          // {3DNOW}
    kIdPfcmpge,                          // {3DNOW}
    kIdPfcmpgt,                          // {3DNOW}
    kIdPfmax,                            // {3DNOW}
    kIdPfmin,                            // {3DNOW}
    kIdPfmul,                            // {3DNOW}
    kIdPfnacc,                           // {3DNOW2}
    kIdPfpnacc,                          // {3DNOW2}
    kIdPfrcp,                            // {3DNOW}
    kIdPfrcpit1,                         // {3DNOW}
    kIdPfrcpit2,                         // {3DNOW}
    kIdPfrcpv,                           // {GEODE}
    kIdPfrsqit1,                         // {3DNOW}
    kIdPfrsqrt,                          // {3DNOW}
    kIdPfrsqrtv,                         // {GEODE}
    kIdPfsub,                            // {3DNOW}
    kIdPfsubr,                           // {3DNOW}
    kIdPhaddd,                           // {SSSE3}
    kIdPhaddsw,                          // {SSSE3}
    kIdPhaddw,                           // {SSSE3}
    kIdPhminposuw,                       // {SSE4_1}
    kIdPhsubd,                           // {SSSE3}
    kIdPhsubsw,                          // {SSSE3}
    kIdPhsubw,                           // {SSSE3}
    kIdPi2fd,                            // {3DNOW}
    kIdPi2fw,                            // {3DNOW2}
    kIdPinsrb,                           // {SSE4_1}
    kIdPinsrd,                           // {SSE4_1}
    kIdPinsrq,                           // {SSE4_1} & <X64>
    kIdPinsrw,                           // {MMX2|SSE2}
    kIdPmaddubsw,                        // {SSSE3}
    kIdPmaddwd,                          // {MMX|SSE2}
    kIdPmaxsb,                           // {SSE4_1}
    kIdPmaxsd,                           // {SSE4_1}
    kIdPmaxsw,                           // {MMX2|SSE2}
    kIdPmaxub,                           // {MMX2|SSE2}
    kIdPmaxud,                           // {SSE4_1}
    kIdPmaxuw,                           // {SSE4_1}
    kIdPminsb,                           // {SSE4_1}
    kIdPminsd,                           // {SSE4_1}
    kIdPminsw,                           // {MMX2|SSE2}
    kIdPminub,                           // {MMX2|SSE2}
    kIdPminud,                           // {SSE4_1}
    kIdPminuw,                           // {SSE4_1}
    kIdPmovmskb,                         // {MMX2|SSE2}
    kIdPmovsxbd,                         // {SSE4_1}
    kIdPmovsxbq,                         // {SSE4_1}
    kIdPmovsxbw,                         // {SSE4_1}
    kIdPmovsxdq,                         // {SSE4_1}
    kIdPmovsxwd,                         // {SSE4_1}
    kIdPmovsxwq,                         // {SSE4_1}
    kIdPmovzxbd,                         // {SSE4_1}
    kIdPmovzxbq,                         // {SSE4_1}
    kIdPmovzxbw,                         // {SSE4_1}
    kIdPmovzxdq,                         // {SSE4_1}
    kIdPmovzxwd,                         // {SSE4_1}
    kIdPmovzxwq,                         // {SSE4_1}
    kIdPmuldq,                           // {SSE4_1}
    kIdPmulhrsw,                         // {SSSE3}
    kIdPmulhrw,                          // {3DNOW}
    kIdPmulhuw,                          // {MMX2|SSE2}
    kIdPmulhw,                           // {MMX|SSE2}
    kIdPmulld,                           // {SSE4_1}
    kIdPmullw,                           // {MMX|SSE2}
    kIdPmuludq,                          // {SSE2}
    kIdPop,                              // <ANY>
    kIdPopa,                             // <X86>
    kIdPopad,                            // <X86>
    kIdPopcnt,                           // {POPCNT}
    kIdPopf,                             // <ANY>
    kIdPopfd,                            // <X86>
    kIdPopfq,                            // <X64>
    kIdPor,                              // {MMX|SSE2}
    kIdPrefetch,                         // {3DNOW}
    kIdPrefetchnta,                      // {MMX2}
    kIdPrefetcht0,                       // {MMX2}
    kIdPrefetcht1,                       // {MMX2}
    kIdPrefetcht2,                       // {MMX2}
    kIdPrefetchw,                        // {PREFETCHW}
    kIdPrefetchwt1,                      // {PREFETCHWT1}
    kIdPsadbw,                           // {MMX2|SSE2}
    kIdPshufb,                           // {SSSE3}
    kIdPshufd,                           // {SSE2}
    kIdPshufhw,                          // {SSE2}
    kIdPshuflw,                          // {SSE2}
    kIdPshufw,                           // {MMX2}
    kIdPsignb,                           // {SSSE3}
    kIdPsignd,                           // {SSSE3}
    kIdPsignw,                           // {SSSE3}
    kIdPslld,                            // {MMX|SSE2}
    kIdPslldq,                           // {SSE2}
    kIdPsllq,                            // {MMX|SSE2}
    kIdPsllw,                            // {MMX|SSE2}
    kIdPsrad,                            // {MMX|SSE2}
    kIdPsraw,                            // {MMX|SSE2}
    kIdPsrld,                            // {MMX|SSE2}
    kIdPsrldq,                           // {SSE2}
    kIdPsrlq,                            // {MMX|SSE2}
    kIdPsrlw,                            // {MMX|SSE2}
    kIdPsubb,                            // {MMX|SSE2}
    kIdPsubd,                            // {MMX|SSE2}
    kIdPsubq,                            // {SSE2}
    kIdPsubsb,                           // {MMX|SSE2}
    kIdPsubsw,                           // {MMX|SSE2}
    kIdPsubusb,                          // {MMX|SSE2}
    kIdPsubusw,                          // {MMX|SSE2}
    kIdPsubw,                            // {MMX|SSE2}
    kIdPswapd,                           // {3DNOW2}
    kIdPtest,                            // {SSE4_1}
    kIdPunpckhbw,                        // {MMX|SSE2}
    kIdPunpckhdq,                        // {MMX|SSE2}
    kIdPunpckhqdq,                       // {SSE2}
    kIdPunpckhwd,                        // {MMX|SSE2}
    kIdPunpcklbw,                        // {MMX|SSE2}
    kIdPunpckldq,                        // {MMX|SSE2}
    kIdPunpcklqdq,                       // {SSE2}
    kIdPunpcklwd,                        // {MMX|SSE2}
    kIdPush,                             // <ANY>
    kIdPusha,                            // <X86>
    kIdPushad,                           // <X86>
    kIdPushf,                            // <ANY>
    kIdPushfd,                           // <X86>
    kIdPushfq,                           // <X64>
    kIdPxor,                             // {MMX|SSE2}
    kIdRcl,                              // <ANY>
    kIdRcpps,                            // {SSE}
    kIdRcpss,                            // {SSE}
    kIdRcr,                              // <ANY>
    kIdRdfsbase,                         // {FSGSBASE} & <X64>
    kIdRdgsbase,                         // {FSGSBASE} & <X64>
    kIdRdmsr,                            // {MSR}
    kIdRdpid,                            // {RDPID}
    kIdRdpmc,                            // <ANY>
    kIdRdrand,                           // {RDRAND}
    kIdRdseed,                           // {RDSEED}
    kIdRdtsc,                            // {RDTSC}
    kIdRdtscp,                           // {RDTSCP}
    kIdRet,                              // <ANY>
    kIdRol,                              // <ANY>
    kIdRor,                              // <ANY>
    kIdRorx,                             // {BMI2}
    kIdRoundpd,                          // {SSE4_1}
    kIdRoundps,                          // {SSE4_1}
    kIdRoundsd,                          // {SSE4_1}
    kIdRoundss,                          // {SSE4_1}
    kIdRsm,                              // <X86>
    kIdRsqrtps,                          // {SSE}
    kIdRsqrtss,                          // {SSE}
    kIdSahf,                             // {LAHFSAHF}
    kIdSal,                              // <ANY>
    kIdSar,                              // <ANY>
    kIdSarx,                             // {BMI2}
    kIdSbb,                              // <ANY>
    kIdScas,                             // <ANY>
    kIdSeta,                             // <ANY>
    kIdSetae,                            // <ANY>
    kIdSetb,                             // <ANY>
    kIdSetbe,                            // <ANY>
    kIdSetc,                             // <ANY>
    kIdSete,                             // <ANY>
    kIdSetg,                             // <ANY>
    kIdSetge,                            // <ANY>
    kIdSetl,                             // <ANY>
    kIdSetle,                            // <ANY>
    kIdSetna,                            // <ANY>
    kIdSetnae,                           // <ANY>
    kIdSetnb,                            // <ANY>
    kIdSetnbe,                           // <ANY>
    kIdSetnc,                            // <ANY>
    kIdSetne,                            // <ANY>
    kIdSetng,                            // <ANY>
    kIdSetnge,                           // <ANY>
    kIdSetnl,                            // <ANY>
    kIdSetnle,                           // <ANY>
    kIdSetno,                            // <ANY>
    kIdSetnp,                            // <ANY>
    kIdSetns,                            // <ANY>
    kIdSetnz,                            // <ANY>
    kIdSeto,                             // <ANY>
    kIdSetp,                             // <ANY>
    kIdSetpe,                            // <ANY>
    kIdSetpo,                            // <ANY>
    kIdSets,                             // <ANY>
    kIdSetz,                             // <ANY>
    kIdSfence,                           // {MMX2}
    kIdSgdt,                             // <ANY>
    kIdSha1msg1,                         // {SHA}
    kIdSha1msg2,                         // {SHA}
    kIdSha1nexte,                        // {SHA}
    kIdSha1rnds4,                        // {SHA}
    kIdSha256msg1,                       // {SHA}
    kIdSha256msg2,                       // {SHA}
    kIdSha256rnds2,                      // {SHA}
    kIdShl,                              // <ANY>
    kIdShld,                             // <ANY>
    kIdShlx,                             // {BMI2}
    kIdShr,                              // <ANY>
    kIdShrd,                             // <ANY>
    kIdShrx,                             // {BMI2}
    kIdShufpd,                           // {SSE2}
    kIdShufps,                           // {SSE}
    kIdSidt,                             // <ANY>
    kIdSkinit,                           // {SKINIT}
    kIdSldt,                             // <ANY>
    kIdSlwpcb,                           // {LWP}
    kIdSmsw,                             // <ANY>
    kIdSqrtpd,                           // {SSE2}
    kIdSqrtps,                           // {SSE}
    kIdSqrtsd,                           // {SSE2}
    kIdSqrtss,                           // {SSE}
    kIdStac,                             // {SMAP}
    kIdStc,                              // <ANY>
    kIdStd,                              // <ANY>
    kIdStgi,                             // {SKINIT}
    kIdSti,                              // <ANY>
    kIdStmxcsr,                          // {SSE}
    kIdStos,                             // <ANY>
    kIdStr,                              // <ANY>
    kIdSub,                              // <ANY>
    kIdSubpd,                            // {SSE2}
    kIdSubps,                            // {SSE}
    kIdSubsd,                            // {SSE2}
    kIdSubss,                            // {SSE}
    kIdSwapgs,                           // <X64>
    kIdSyscall,                          // <X64>
    kIdSysenter,                         // <ANY>
    kIdSysexit,                          // <ANY>
    kIdSysexit64,                        // <ANY>
    kIdSysret,                           // <X64>
    kIdSysret64,                         // <X64>
    kIdT1mskc,                           // {TBM}
    kIdTest,                             // <ANY>
    kIdTzcnt,                            // {BMI}
    kIdTzmsk,                            // {TBM}
    kIdUcomisd,                          // {SSE2}
    kIdUcomiss,                          // {SSE}
    kIdUd2,                              // <ANY>
    kIdUnpckhpd,                         // {SSE2}
    kIdUnpckhps,                         // {SSE}
    kIdUnpcklpd,                         // {SSE2}
    kIdUnpcklps,                         // {SSE}
    kIdV4fmaddps,                        // {AVX512_4FMAPS}
    kIdV4fmaddss,                        // {AVX512_4FMAPS}
    kIdV4fnmaddps,                       // {AVX512_4FMAPS}
    kIdV4fnmaddss,                       // {AVX512_4FMAPS}
    kIdVaddpd,                           // {AVX|AVX512_F+VL}
    kIdVaddps,                           // {AVX|AVX512_F+VL}
    kIdVaddsd,                           // {AVX|AVX512_F}
    kIdVaddss,                           // {AVX|AVX512_F}
    kIdVaddsubpd,                        // {AVX}
    kIdVaddsubps,                        // {AVX}
    kIdVaesdec,                          // {AVX|AVX512_F+VL & AESNI|VAES}
    kIdVaesdeclast,                      // {AVX|AVX512_F+VL & AESNI|VAES}
    kIdVaesenc,                          // {AVX|AVX512_F+VL & AESNI|VAES}
    kIdVaesenclast,                      // {AVX|AVX512_F+VL & AESNI|VAES}
    kIdVaesimc,                          // {AVX & AESNI}
    kIdVaeskeygenassist,                 // {AVX & AESNI}
    kIdValignd,                          // {AVX512_F+VL}
    kIdValignq,                          // {AVX512_F+VL}
    kIdVandnpd,                          // {AVX|AVX512_DQ+VL}
    kIdVandnps,                          // {AVX|AVX512_DQ+VL}
    kIdVandpd,                           // {AVX|AVX512_DQ+VL}
    kIdVandps,                           // {AVX|AVX512_DQ+VL}
    kIdVblendmb,                         // {AVX512_BW+VL}
    kIdVblendmd,                         // {AVX512_F+VL}
    kIdVblendmpd,                        // {AVX512_F+VL}
    kIdVblendmps,                        // {AVX512_F+VL}
    kIdVblendmq,                         // {AVX512_F+VL}
    kIdVblendmw,                         // {AVX512_BW+VL}
    kIdVblendpd,                         // {AVX}
    kIdVblendps,                         // {AVX}
    kIdVblendvpd,                        // {AVX}
    kIdVblendvps,                        // {AVX}
    kIdVbroadcastf128,                   // {AVX}
    kIdVbroadcastf32x2,                  // {AVX512_DQ+VL}
    kIdVbroadcastf32x4,                  // {AVX512_F}
    kIdVbroadcastf32x8,                  // {AVX512_DQ}
    kIdVbroadcastf64x2,                  // {AVX512_DQ+VL}
    kIdVbroadcastf64x4,                  // {AVX512_F}
    kIdVbroadcasti128,                   // {AVX2}
    kIdVbroadcasti32x2,                  // {AVX512_DQ+VL}
    kIdVbroadcasti32x4,                  // {AVX512_F+VL}
    kIdVbroadcasti32x8,                  // {AVX512_DQ}
    kIdVbroadcasti64x2,                  // {AVX512_DQ+VL}
    kIdVbroadcasti64x4,                  // {AVX512_F}
    kIdVbroadcastsd,                     // {AVX|AVX2|AVX512_F+VL}
    kIdVbroadcastss,                     // {AVX|AVX2|AVX512_F+VL}
    kIdVcmppd,                           // {AVX|AVX512_F+VL}
    kIdVcmpps,                           // {AVX|AVX512_F+VL}
    kIdVcmpsd,                           // {AVX|AVX512_F}
    kIdVcmpss,                           // {AVX|AVX512_F}
    kIdVcomisd,                          // {AVX|AVX512_F}
    kIdVcomiss,                          // {AVX|AVX512_F}
    kIdVcompresspd,                      // {AVX512_F+VL}
    kIdVcompressps,                      // {AVX512_F+VL}
    kIdVcvtdq2pd,                        // {AVX|AVX512_F+VL}
    kIdVcvtdq2ps,                        // {AVX|AVX512_F+VL}
    kIdVcvtne2ps2bf16,                   // {AVX512_BF16+VL}
    kIdVcvtneps2bf16,                    // {AVX512_BF16+VL}
    kIdVcvtpd2dq,                        // {AVX|AVX512_F+VL}
    kIdVcvtpd2ps,                        // {AVX|AVX512_F+VL}
    kIdVcvtpd2qq,                        // {AVX512_DQ+VL}
    kIdVcvtpd2udq,                       // {AVX512_F+VL}
    kIdVcvtpd2uqq,                       // {AVX512_DQ+VL}
    kIdVcvtph2ps,                        // {AVX512_F+VL & F16C}
    kIdVcvtps2dq,                        // {AVX|AVX512_F+VL}
    kIdVcvtps2pd,                        // {AVX|AVX512_F+VL}
    kIdVcvtps2ph,                        // {AVX512_F+VL & F16C}
    kIdVcvtps2qq,                        // {AVX512_DQ+VL}
    kIdVcvtps2udq,                       // {AVX512_F+VL}
    kIdVcvtps2uqq,                       // {AVX512_DQ+VL}
    kIdVcvtqq2pd,                        // {AVX512_DQ+VL}
    kIdVcvtqq2ps,                        // {AVX512_DQ+VL}
    kIdVcvtsd2si,                        // {AVX|AVX512_F}
    kIdVcvtsd2ss,                        // {AVX|AVX512_F}
    kIdVcvtsd2usi,                       // {AVX512_F}
    kIdVcvtsi2sd,                        // {AVX|AVX512_F}
    kIdVcvtsi2ss,                        // {AVX|AVX512_F}
    kIdVcvtss2sd,                        // {AVX|AVX512_F}
    kIdVcvtss2si,                        // {AVX|AVX512_F}
    kIdVcvtss2usi,                       // {AVX512_F}
    kIdVcvttpd2dq,                       // {AVX|AVX512_F+VL}
    kIdVcvttpd2qq,                       // {AVX512_F+VL}
    kIdVcvttpd2udq,                      // {AVX512_F+VL}
    kIdVcvttpd2uqq,                      // {AVX512_DQ+VL}
    kIdVcvttps2dq,                       // {AVX|AVX512_F+VL}
    kIdVcvttps2qq,                       // {AVX512_DQ+VL}
    kIdVcvttps2udq,                      // {AVX512_F+VL}
    kIdVcvttps2uqq,                      // {AVX512_DQ+VL}
    kIdVcvttsd2si,                       // {AVX|AVX512_F}
    kIdVcvttsd2usi,                      // {AVX512_F}
    kIdVcvttss2si,                       // {AVX|AVX512_F}
    kIdVcvttss2usi,                      // {AVX512_F}
    kIdVcvtudq2pd,                       // {AVX512_F+VL}
    kIdVcvtudq2ps,                       // {AVX512_F+VL}
    kIdVcvtuqq2pd,                       // {AVX512_DQ+VL}
    kIdVcvtuqq2ps,                       // {AVX512_DQ+VL}
    kIdVcvtusi2sd,                       // {AVX512_F}
    kIdVcvtusi2ss,                       // {AVX512_F}
    kIdVdbpsadbw,                        // {AVX512_BW+VL}
    kIdVdivpd,                           // {AVX|AVX512_F+VL}
    kIdVdivps,                           // {AVX|AVX512_F+VL}
    kIdVdivsd,                           // {AVX|AVX512_F}
    kIdVdivss,                           // {AVX|AVX512_F}
    kIdVdpbf16ps,                        // {AVX512_BF16+VL}
    kIdVdppd,                            // {AVX}
    kIdVdpps,                            // {AVX}
    kIdVerr,                             // <ANY>
    kIdVerw,                             // <ANY>
    kIdVexp2pd,                          // {AVX512_ERI}
    kIdVexp2ps,                          // {AVX512_ERI}
    kIdVexpandpd,                        // {AVX512_F+VL}
    kIdVexpandps,                        // {AVX512_F+VL}
    kIdVextractf128,                     // {AVX}
    kIdVextractf32x4,                    // {AVX512_F+VL}
    kIdVextractf32x8,                    // {AVX512_DQ}
    kIdVextractf64x2,                    // {AVX512_DQ+VL}
    kIdVextractf64x4,                    // {AVX512_F}
    kIdVextracti128,                     // {AVX2}
    kIdVextracti32x4,                    // {AVX512_F+VL}
    kIdVextracti32x8,                    // {AVX512_DQ}
    kIdVextracti64x2,                    // {AVX512_DQ+VL}
    kIdVextracti64x4,                    // {AVX512_F}
    kIdVextractps,                       // {AVX|AVX512_F}
    kIdVfixupimmpd,                      // {AVX512_F+VL}
    kIdVfixupimmps,                      // {AVX512_F+VL}
    kIdVfixupimmsd,                      // {AVX512_F}
    kIdVfixupimmss,                      // {AVX512_F}
    kIdVfmadd132pd,                      // {FMA|AVX512_F+VL}
    kIdVfmadd132ps,                      // {FMA|AVX512_F+VL}
    kIdVfmadd132sd,                      // {FMA|AVX512_F}
    kIdVfmadd132ss,                      // {FMA|AVX512_F}
    kIdVfmadd213pd,                      // {FMA|AVX512_F+VL}
    kIdVfmadd213ps,                      // {FMA|AVX512_F+VL}
    kIdVfmadd213sd,                      // {FMA|AVX512_F}
    kIdVfmadd213ss,                      // {FMA|AVX512_F}
    kIdVfmadd231pd,                      // {FMA|AVX512_F+VL}
    kIdVfmadd231ps,                      // {FMA|AVX512_F+VL}
    kIdVfmadd231sd,                      // {FMA|AVX512_F}
    kIdVfmadd231ss,                      // {FMA|AVX512_F}
    kIdVfmaddpd,                         // {FMA4}
    kIdVfmaddps,                         // {FMA4}
    kIdVfmaddsd,                         // {FMA4}
    kIdVfmaddss,                         // {FMA4}
    kIdVfmaddsub132pd,                   // {FMA|AVX512_F+VL}
    kIdVfmaddsub132ps,                   // {FMA|AVX512_F+VL}
    kIdVfmaddsub213pd,                   // {FMA|AVX512_F+VL}
    kIdVfmaddsub213ps,                   // {FMA|AVX512_F+VL}
    kIdVfmaddsub231pd,                   // {FMA|AVX512_F+VL}
    kIdVfmaddsub231ps,                   // {FMA|AVX512_F+VL}
    kIdVfmaddsubpd,                      // {FMA4}
    kIdVfmaddsubps,                      // {FMA4}
    kIdVfmsub132pd,                      // {FMA|AVX512_F+VL}
    kIdVfmsub132ps,                      // {FMA|AVX512_F+VL}
    kIdVfmsub132sd,                      // {FMA|AVX512_F}
    kIdVfmsub132ss,                      // {FMA|AVX512_F}
    kIdVfmsub213pd,                      // {FMA|AVX512_F+VL}
    kIdVfmsub213ps,                      // {FMA|AVX512_F+VL}
    kIdVfmsub213sd,                      // {FMA|AVX512_F}
    kIdVfmsub213ss,                      // {FMA|AVX512_F}
    kIdVfmsub231pd,                      // {FMA|AVX512_F+VL}
    kIdVfmsub231ps,                      // {FMA|AVX512_F+VL}
    kIdVfmsub231sd,                      // {FMA|AVX512_F}
    kIdVfmsub231ss,                      // {FMA|AVX512_F}
    kIdVfmsubadd132pd,                   // {FMA|AVX512_F+VL}
    kIdVfmsubadd132ps,                   // {FMA|AVX512_F+VL}
    kIdVfmsubadd213pd,                   // {FMA|AVX512_F+VL}
    kIdVfmsubadd213ps,                   // {FMA|AVX512_F+VL}
    kIdVfmsubadd231pd,                   // {FMA|AVX512_F+VL}
    kIdVfmsubadd231ps,                   // {FMA|AVX512_F+VL}
    kIdVfmsubaddpd,                      // {FMA4}
    kIdVfmsubaddps,                      // {FMA4}
    kIdVfmsubpd,                         // {FMA4}
    kIdVfmsubps,                         // {FMA4}
    kIdVfmsubsd,                         // {FMA4}
    kIdVfmsubss,                         // {FMA4}
    kIdVfnmadd132pd,                     // {FMA|AVX512_F+VL}
    kIdVfnmadd132ps,                     // {FMA|AVX512_F+VL}
    kIdVfnmadd132sd,                     // {FMA|AVX512_F}
    kIdVfnmadd132ss,                     // {FMA|AVX512_F}
    kIdVfnmadd213pd,                     // {FMA|AVX512_F+VL}
    kIdVfnmadd213ps,                     // {FMA|AVX512_F+VL}
    kIdVfnmadd213sd,                     // {FMA|AVX512_F}
    kIdVfnmadd213ss,                     // {FMA|AVX512_F}
    kIdVfnmadd231pd,                     // {FMA|AVX512_F+VL}
    kIdVfnmadd231ps,                     // {FMA|AVX512_F+VL}
    kIdVfnmadd231sd,                     // {FMA|AVX512_F}
    kIdVfnmadd231ss,                     // {FMA|AVX512_F}
    kIdVfnmaddpd,                        // {FMA4}
    kIdVfnmaddps,                        // {FMA4}
    kIdVfnmaddsd,                        // {FMA4}
    kIdVfnmaddss,                        // {FMA4}
    kIdVfnmsub132pd,                     // {FMA|AVX512_F+VL}
    kIdVfnmsub132ps,                     // {FMA|AVX512_F+VL}
    kIdVfnmsub132sd,                     // {FMA|AVX512_F}
    kIdVfnmsub132ss,                     // {FMA|AVX512_F}
    kIdVfnmsub213pd,                     // {FMA|AVX512_F+VL}
    kIdVfnmsub213ps,                     // {FMA|AVX512_F+VL}
    kIdVfnmsub213sd,                     // {FMA|AVX512_F}
    kIdVfnmsub213ss,                     // {FMA|AVX512_F}
    kIdVfnmsub231pd,                     // {FMA|AVX512_F+VL}
    kIdVfnmsub231ps,                     // {FMA|AVX512_F+VL}
    kIdVfnmsub231sd,                     // {FMA|AVX512_F}
    kIdVfnmsub231ss,                     // {FMA|AVX512_F}
    kIdVfnmsubpd,                        // {FMA4}
    kIdVfnmsubps,                        // {FMA4}
    kIdVfnmsubsd,                        // {FMA4}
    kIdVfnmsubss,                        // {FMA4}
    kIdVfpclasspd,                       // {AVX512_DQ+VL}
    kIdVfpclassps,                       // {AVX512_DQ+VL}
    kIdVfpclasssd,                       // {AVX512_DQ}
    kIdVfpclassss,                       // {AVX512_DQ}
    kIdVfrczpd,                          // {XOP}
    kIdVfrczps,                          // {XOP}
    kIdVfrczsd,                          // {XOP}
    kIdVfrczss,                          // {XOP}
    kIdVgatherdpd,                       // {AVX2|AVX512_F+VL}
    kIdVgatherdps,                       // {AVX2|AVX512_F+VL}
    kIdVgatherpf0dpd,                    // {AVX512_PFI}
    kIdVgatherpf0dps,                    // {AVX512_PFI}
    kIdVgatherpf0qpd,                    // {AVX512_PFI}
    kIdVgatherpf0qps,                    // {AVX512_PFI}
    kIdVgatherpf1dpd,                    // {AVX512_PFI}
    kIdVgatherpf1dps,                    // {AVX512_PFI}
    kIdVgatherpf1qpd,                    // {AVX512_PFI}
    kIdVgatherpf1qps,                    // {AVX512_PFI}
    kIdVgatherqpd,                       // {AVX2|AVX512_F+VL}
    kIdVgatherqps,                       // {AVX2|AVX512_F+VL}
    kIdVgetexppd,                        // {AVX512_F+VL}
    kIdVgetexpps,                        // {AVX512_F+VL}
    kIdVgetexpsd,                        // {AVX512_F}
    kIdVgetexpss,                        // {AVX512_F}
    kIdVgetmantpd,                       // {AVX512_F+VL}
    kIdVgetmantps,                       // {AVX512_F+VL}
    kIdVgetmantsd,                       // {AVX512_F}
    kIdVgetmantss,                       // {AVX512_F}
    kIdVgf2p8affineinvqb,                // {AVX|AVX512_F+VL & GFNI}
    kIdVgf2p8affineqb,                   // {AVX|AVX512_F+VL & GFNI}
    kIdVgf2p8mulb,                       // {AVX|AVX512_F+VL & GFNI}
    kIdVhaddpd,                          // {AVX}
    kIdVhaddps,                          // {AVX}
    kIdVhsubpd,                          // {AVX}
    kIdVhsubps,                          // {AVX}
    kIdVinsertf128,                      // {AVX}
    kIdVinsertf32x4,                     // {AVX512_F+VL}
    kIdVinsertf32x8,                     // {AVX512_DQ}
    kIdVinsertf64x2,                     // {AVX512_DQ+VL}
    kIdVinsertf64x4,                     // {AVX512_F}
    kIdVinserti128,                      // {AVX2}
    kIdVinserti32x4,                     // {AVX512_F+VL}
    kIdVinserti32x8,                     // {AVX512_DQ}
    kIdVinserti64x2,                     // {AVX512_DQ+VL}
    kIdVinserti64x4,                     // {AVX512_F}
    kIdVinsertps,                        // {AVX|AVX512_F}
    kIdVlddqu,                           // {AVX}
    kIdVldmxcsr,                         // {AVX}
    kIdVmaskmovdqu,                      // {AVX}
    kIdVmaskmovpd,                       // {AVX}
    kIdVmaskmovps,                       // {AVX}
    kIdVmaxpd,                           // {AVX|AVX512_F+VL}
    kIdVmaxps,                           // {AVX|AVX512_F+VL}
    kIdVmaxsd,                           // {AVX|AVX512_F+VL}
    kIdVmaxss,                           // {AVX|AVX512_F+VL}
    kIdVmcall,                           // {VMX}
    kIdVmclear,                          // {VMX}
    kIdVmfunc,                           // {VMX}
    kIdVminpd,                           // {AVX|AVX512_F+VL}
    kIdVminps,                           // {AVX|AVX512_F+VL}
    kIdVminsd,                           // {AVX|AVX512_F+VL}
    kIdVminss,                           // {AVX|AVX512_F+VL}
    kIdVmlaunch,                         // {VMX}
    kIdVmload,                           // {SVM}
    kIdVmmcall,                          // {SVM}
    kIdVmovapd,                          // {AVX|AVX512_F+VL}
    kIdVmovaps,                          // {AVX|AVX512_F+VL}
    kIdVmovd,                            // {AVX|AVX512_F}
    kIdVmovddup,                         // {AVX|AVX512_F+VL}
    kIdVmovdqa,                          // {AVX}
    kIdVmovdqa32,                        // {AVX512_F+VL}
    kIdVmovdqa64,                        // {AVX512_F+VL}
    kIdVmovdqu,                          // {AVX}
    kIdVmovdqu16,                        // {AVX512_BW+VL}
    kIdVmovdqu32,                        // {AVX512_F+VL}
    kIdVmovdqu64,                        // {AVX512_F+VL}
    kIdVmovdqu8,                         // {AVX512_BW+VL}
    kIdVmovhlps,                         // {AVX|AVX512_F}
    kIdVmovhpd,                          // {AVX|AVX512_F}
    kIdVmovhps,                          // {AVX|AVX512_F}
    kIdVmovlhps,                         // {AVX|AVX512_F}
    kIdVmovlpd,                          // {AVX|AVX512_F}
    kIdVmovlps,                          // {AVX|AVX512_F}
    kIdVmovmskpd,                        // {AVX}
    kIdVmovmskps,                        // {AVX}
    kIdVmovntdq,                         // {AVX|AVX512_F+VL}
    kIdVmovntdqa,                        // {AVX|AVX2|AVX512_F+VL}
    kIdVmovntpd,                         // {AVX|AVX512_F+VL}
    kIdVmovntps,                         // {AVX|AVX512_F+VL}
    kIdVmovq,                            // {AVX|AVX512_F}
    kIdVmovsd,                           // {AVX|AVX512_F}
    kIdVmovshdup,                        // {AVX|AVX512_F+VL}
    kIdVmovsldup,                        // {AVX|AVX512_F+VL}
    kIdVmovss,                           // {AVX|AVX512_F}
    kIdVmovupd,                          // {AVX|AVX512_F+VL}
    kIdVmovups,                          // {AVX|AVX512_F+VL}
    kIdVmpsadbw,                         // {AVX|AVX2}
    kIdVmptrld,                          // {VMX}
    kIdVmptrst,                          // {VMX}
    kIdVmread,                           // {VMX}
    kIdVmresume,                         // {VMX}
    kIdVmrun,                            // {SVM}
    kIdVmsave,                           // {SVM}
    kIdVmulpd,                           // {AVX|AVX512_F+VL}
    kIdVmulps,                           // {AVX|AVX512_F+VL}
    kIdVmulsd,                           // {AVX|AVX512_F}
    kIdVmulss,                           // {AVX|AVX512_F}
    kIdVmwrite,                          // {VMX}
    kIdVmxon,                            // {VMX}
    kIdVorpd,                            // {AVX|AVX512_DQ+VL}
    kIdVorps,                            // {AVX|AVX512_DQ+VL}
    kIdVp4dpwssd,                        // {AVX512_4VNNIW}
    kIdVp4dpwssds,                       // {AVX512_4VNNIW}
    kIdVpabsb,                           // {AVX|AVX2|AVX512_BW+VL}
    kIdVpabsd,                           // {AVX|AVX2|AVX512_F+VL}
    kIdVpabsq,                           // {AVX512_F+VL}
    kIdVpabsw,                           // {AVX|AVX2|AVX512_BW+VL}
    kIdVpackssdw,                        // {AVX|AVX2|AVX512_BW+VL}
    kIdVpacksswb,                        // {AVX|AVX2|AVX512_BW+VL}
    kIdVpackusdw,                        // {AVX|AVX2|AVX512_BW+VL}
    kIdVpackuswb,                        // {AVX|AVX2|AVX512_BW+VL}
    kIdVpaddb,                           // {AVX|AVX2|AVX512_BW+VL}
    kIdVpaddd,                           // {AVX|AVX2|AVX512_F+VL}
    kIdVpaddq,                           // {AVX|AVX2|AVX512_F+VL}
    kIdVpaddsb,                          // {AVX|AVX2|AVX512_BW+VL}
    kIdVpaddsw,                          // {AVX|AVX2|AVX512_BW+VL}
    kIdVpaddusb,                         // {AVX|AVX2|AVX512_BW+VL}
    kIdVpaddusw,                         // {AVX|AVX2|AVX512_BW+VL}
    kIdVpaddw,                           // {AVX|AVX2|AVX512_BW+VL}
    kIdVpalignr,                         // {AVX|AVX2|AVX512_BW+VL}
    kIdVpand,                            // {AVX|AVX2}
    kIdVpandd,                           // {AVX512_F+VL}
    kIdVpandn,                           // {AVX|AVX2}
    kIdVpandnd,                          // {AVX512_F+VL}
    kIdVpandnq,                          // {AVX512_F+VL}
    kIdVpandq,                           // {AVX512_F+VL}
    kIdVpavgb,                           // {AVX|AVX2|AVX512_BW+VL}
    kIdVpavgw,                           // {AVX|AVX2|AVX512_BW+VL}
    kIdVpblendd,                         // {AVX2}
    kIdVpblendvb,                        // {AVX|AVX2}
    kIdVpblendw,                         // {AVX|AVX2}
    kIdVpbroadcastb,                     // {AVX2|AVX512_BW+VL}
    kIdVpbroadcastd,                     // {AVX2|AVX512_F+VL}
    kIdVpbroadcastmb2d,                  // {AVX512_CDI+VL}
    kIdVpbroadcastmb2q,                  // {AVX512_CDI+VL}
    kIdVpbroadcastq,                     // {AVX2|AVX512_F+VL}
    kIdVpbroadcastw,                     // {AVX2|AVX512_BW+VL}
    kIdVpclmulqdq,                       // {AVX|AVX512_F+VL & PCLMULQDQ|VPCLMULQDQ}
    kIdVpcmov,                           // {XOP}
    kIdVpcmpb,                           // {AVX512_BW+VL}
    kIdVpcmpd,                           // {AVX512_F+VL}
    kIdVpcmpeqb,                         // {AVX|AVX2|AVX512_BW+VL}
    kIdVpcmpeqd,                         // {AVX|AVX2|AVX512_F+VL}
    kIdVpcmpeqq,                         // {AVX|AVX2|AVX512_F+VL}
    kIdVpcmpeqw,                         // {AVX|AVX2|AVX512_BW+VL}
    kIdVpcmpestri,                       // {AVX}
    kIdVpcmpestrm,                       // {AVX}
    kIdVpcmpgtb,                         // {AVX|AVX2|AVX512_BW+VL}
    kIdVpcmpgtd,                         // {AVX|AVX2|AVX512_F+VL}
    kIdVpcmpgtq,                         // {AVX|AVX2|AVX512_F+VL}
    kIdVpcmpgtw,                         // {AVX|AVX2|AVX512_BW+VL}
    kIdVpcmpistri,                       // {AVX}
    kIdVpcmpistrm,                       // {AVX}
    kIdVpcmpq,                           // {AVX512_F+VL}
    kIdVpcmpub,                          // {AVX512_BW+VL}
    kIdVpcmpud,                          // {AVX512_F+VL}
    kIdVpcmpuq,                          // {AVX512_F+VL}
    kIdVpcmpuw,                          // {AVX512_BW+VL}
    kIdVpcmpw,                           // {AVX512_BW+VL}
    kIdVpcomb,                           // {XOP}
    kIdVpcomd,                           // {XOP}
    kIdVpcompressb,                      // {AVX512_VBMI2+VL}
    kIdVpcompressd,                      // {AVX512_F+VL}
    kIdVpcompressq,                      // {AVX512_F+VL}
    kIdVpcompressw,                      // {AVX512_VBMI2+VL}
    kIdVpcomq,                           // {XOP}
    kIdVpcomub,                          // {XOP}
    kIdVpcomud,                          // {XOP}
    kIdVpcomuq,                          // {XOP}
    kIdVpcomuw,                          // {XOP}
    kIdVpcomw,                           // {XOP}
    kIdVpconflictd,                      // {AVX512_CDI+VL}
    kIdVpconflictq,                      // {AVX512_CDI+VL}
    kIdVpdpbusd,                         // {AVX512_VNNI+VL}
    kIdVpdpbusds,                        // {AVX512_VNNI+VL}
    kIdVpdpwssd,                         // {AVX512_VNNI+VL}
    kIdVpdpwssds,                        // {AVX512_VNNI+VL}
    kIdVperm2f128,                       // {AVX}
    kIdVperm2i128,                       // {AVX2}
    kIdVpermb,                           // {AVX512_VBMI+VL}
    kIdVpermd,                           // {AVX2|AVX512_F+VL}
    kIdVpermi2b,                         // {AVX512_VBMI+VL}
    kIdVpermi2d,                         // {AVX512_F+VL}
    kIdVpermi2pd,                        // {AVX512_F+VL}
    kIdVpermi2ps,                        // {AVX512_F+VL}
    kIdVpermi2q,                         // {AVX512_F+VL}
    kIdVpermi2w,                         // {AVX512_BW+VL}
    kIdVpermil2pd,                       // {XOP}
    kIdVpermil2ps,                       // {XOP}
    kIdVpermilpd,                        // {AVX|AVX512_F+VL}
    kIdVpermilps,                        // {AVX|AVX512_F+VL}
    kIdVpermpd,                          // {AVX2}
    kIdVpermps,                          // {AVX2}
    kIdVpermq,                           // {AVX2|AVX512_F+VL}
    kIdVpermt2b,                         // {AVX512_VBMI+VL}
    kIdVpermt2d,                         // {AVX512_F+VL}
    kIdVpermt2pd,                        // {AVX512_F+VL}
    kIdVpermt2ps,                        // {AVX512_F+VL}
    kIdVpermt2q,                         // {AVX512_F+VL}
    kIdVpermt2w,                         // {AVX512_BW+VL}
    kIdVpermw,                           // {AVX512_BW+VL}
    kIdVpexpandb,                        // {AVX512_VBMI2+VL}
    kIdVpexpandd,                        // {AVX512_F+VL}
    kIdVpexpandq,                        // {AVX512_F+VL}
    kIdVpexpandw,                        // {AVX512_VBMI2+VL}
    kIdVpextrb,                          // {AVX|AVX512_BW}
    kIdVpextrd,                          // {AVX|AVX512_DQ}
    kIdVpextrq,                          // {AVX|AVX512_DQ} & <X64>
    kIdVpextrw,                          // {AVX|AVX512_BW}
    kIdVpgatherdd,                       // {AVX2|AVX512_F+VL}
    kIdVpgatherdq,                       // {AVX2|AVX512_F+VL}
    kIdVpgatherqd,                       // {AVX2|AVX512_F+VL}
    kIdVpgatherqq,                       // {AVX2|AVX512_F+VL}
    kIdVphaddbd,                         // {XOP}
    kIdVphaddbq,                         // {XOP}
    kIdVphaddbw,                         // {XOP}
    kIdVphaddd,                          // {AVX|AVX2}
    kIdVphadddq,                         // {XOP}
    kIdVphaddsw,                         // {AVX|AVX2}
    kIdVphaddubd,                        // {XOP}
    kIdVphaddubq,                        // {XOP}
    kIdVphaddubw,                        // {XOP}
    kIdVphaddudq,                        // {XOP}
    kIdVphadduwd,                        // {XOP}
    kIdVphadduwq,                        // {XOP}
    kIdVphaddw,                          // {AVX|AVX2}
    kIdVphaddwd,                         // {XOP}
    kIdVphaddwq,                         // {XOP}
    kIdVphminposuw,                      // {AVX}
    kIdVphsubbw,                         // {XOP}
    kIdVphsubd,                          // {AVX|AVX2}
    kIdVphsubdq,                         // {XOP}
    kIdVphsubsw,                         // {AVX|AVX2}
    kIdVphsubw,                          // {AVX|AVX2}
    kIdVphsubwd,                         // {XOP}
    kIdVpinsrb,                          // {AVX|AVX512_BW}
    kIdVpinsrd,                          // {AVX|AVX512_DQ}
    kIdVpinsrq,                          // {AVX|AVX512_DQ} & <X64>
    kIdVpinsrw,                          // {AVX|AVX512_BW}
    kIdVplzcntd,                         // {AVX512_CDI+VL}
    kIdVplzcntq,                         // {AVX512_CDI+VL}
    kIdVpmacsdd,                         // {XOP}
    kIdVpmacsdqh,                        // {XOP}
    kIdVpmacsdql,                        // {XOP}
    kIdVpmacssdd,                        // {XOP}
    kIdVpmacssdqh,                       // {XOP}
    kIdVpmacssdql,                       // {XOP}
    kIdVpmacsswd,                        // {XOP}
    kIdVpmacssww,                        // {XOP}
    kIdVpmacswd,                         // {XOP}
    kIdVpmacsww,                         // {XOP}
    kIdVpmadcsswd,                       // {XOP}
    kIdVpmadcswd,                        // {XOP}
    kIdVpmadd52huq,                      // {AVX512_IFMA+VL}
    kIdVpmadd52luq,                      // {AVX512_IFMA+VL}
    kIdVpmaddubsw,                       // {AVX|AVX2|AVX512_BW+VL}
    kIdVpmaddwd,                         // {AVX|AVX2|AVX512_BW+VL}
    kIdVpmaskmovd,                       // {AVX2}
    kIdVpmaskmovq,                       // {AVX2}
    kIdVpmaxsb,                          // {AVX|AVX2|AVX512_BW+VL}
    kIdVpmaxsd,                          // {AVX|AVX2|AVX512_F+VL}
    kIdVpmaxsq,                          // {AVX512_F+VL}
    kIdVpmaxsw,                          // {AVX|AVX2|AVX512_BW+VL}
    kIdVpmaxub,                          // {AVX|AVX2|AVX512_BW+VL}
    kIdVpmaxud,                          // {AVX|AVX2|AVX512_F+VL}
    kIdVpmaxuq,                          // {AVX512_F+VL}
    kIdVpmaxuw,                          // {AVX|AVX2|AVX512_BW+VL}
    kIdVpminsb,                          // {AVX|AVX2|AVX512_BW+VL}
    kIdVpminsd,                          // {AVX|AVX2|AVX512_F+VL}
    kIdVpminsq,                          // {AVX512_F+VL}
    kIdVpminsw,                          // {AVX|AVX2|AVX512_BW+VL}
    kIdVpminub,                          // {AVX|AVX2|AVX512_BW+VL}
    kIdVpminud,                          // {AVX|AVX2|AVX512_F+VL}
    kIdVpminuq,                          // {AVX512_F+VL}
    kIdVpminuw,                          // {AVX|AVX2|AVX512_BW+VL}
    kIdVpmovb2m,                         // {AVX512_BW+VL}
    kIdVpmovd2m,                         // {AVX512_DQ+VL}
    kIdVpmovdb,                          // {AVX512_F+VL}
    kIdVpmovdw,                          // {AVX512_F+VL}
    kIdVpmovm2b,                         // {AVX512_BW+VL}
    kIdVpmovm2d,                         // {AVX512_DQ+VL}
    kIdVpmovm2q,                         // {AVX512_DQ+VL}
    kIdVpmovm2w,                         // {AVX512_BW+VL}
    kIdVpmovmskb,                        // {AVX|AVX2}
    kIdVpmovq2m,                         // {AVX512_DQ+VL}
    kIdVpmovqb,                          // {AVX512_F+VL}
    kIdVpmovqd,                          // {AVX512_F+VL}
    kIdVpmovqw,                          // {AVX512_F+VL}
    kIdVpmovsdb,                         // {AVX512_F+VL}
    kIdVpmovsdw,                         // {AVX512_F+VL}
    kIdVpmovsqb,                         // {AVX512_F+VL}
    kIdVpmovsqd,                         // {AVX512_F+VL}
    kIdVpmovsqw,                         // {AVX512_F+VL}
    kIdVpmovswb,                         // {AVX512_BW+VL}
    kIdVpmovsxbd,                        // {AVX|AVX2|AVX512_F+VL}
    kIdVpmovsxbq,                        // {AVX|AVX2|AVX512_F+VL}
    kIdVpmovsxbw,                        // {AVX|AVX2|AVX512_BW+VL}
    kIdVpmovsxdq,                        // {AVX|AVX2|AVX512_F+VL}
    kIdVpmovsxwd,                        // {AVX|AVX2|AVX512_F+VL}
    kIdVpmovsxwq,                        // {AVX|AVX2|AVX512_F+VL}
    kIdVpmovusdb,                        // {AVX512_F+VL}
    kIdVpmovusdw,                        // {AVX512_F+VL}
    kIdVpmovusqb,                        // {AVX512_F+VL}
    kIdVpmovusqd,                        // {AVX512_F+VL}
    kIdVpmovusqw,                        // {AVX512_F+VL}
    kIdVpmovuswb,                        // {AVX512_BW+VL}
    kIdVpmovw2m,                         // {AVX512_BW+VL}
    kIdVpmovwb,                          // {AVX512_BW+VL}
    kIdVpmovzxbd,                        // {AVX|AVX2|AVX512_F+VL}
    kIdVpmovzxbq,                        // {AVX|AVX2|AVX512_F+VL}
    kIdVpmovzxbw,                        // {AVX|AVX2|AVX512_BW+VL}
    kIdVpmovzxdq,                        // {AVX|AVX2|AVX512_F+VL}
    kIdVpmovzxwd,                        // {AVX|AVX2|AVX512_F+VL}
    kIdVpmovzxwq,                        // {AVX|AVX2|AVX512_F+VL}
    kIdVpmuldq,                          // {AVX|AVX2|AVX512_F+VL}
    kIdVpmulhrsw,                        // {AVX|AVX2|AVX512_BW+VL}
    kIdVpmulhuw,                         // {AVX|AVX2|AVX512_BW+VL}
    kIdVpmulhw,                          // {AVX|AVX2|AVX512_BW+VL}
    kIdVpmulld,                          // {AVX|AVX2|AVX512_F+VL}
    kIdVpmullq,                          // {AVX512_DQ+VL}
    kIdVpmullw,                          // {AVX|AVX2|AVX512_BW+VL}
    kIdVpmultishiftqb,                   // {AVX512_VBMI+VL}
    kIdVpmuludq,                         // {AVX|AVX2|AVX512_F+VL}
    kIdVpopcntb,                         // {AVX512_BITALG+VL}
    kIdVpopcntd,                         // {AVX512_VPOPCNTDQ+VL}
    kIdVpopcntq,                         // {AVX512_VPOPCNTDQ+VL}
    kIdVpopcntw,                         // {AVX512_BITALG+VL}
    kIdVpor,                             // {AVX|AVX2}
    kIdVpord,                            // {AVX512_F+VL}
    kIdVporq,                            // {AVX512_F+VL}
    kIdVpperm,                           // {XOP}
    kIdVprold,                           // {AVX512_F+VL}
    kIdVprolq,                           // {AVX512_F+VL}
    kIdVprolvd,                          // {AVX512_F+VL}
    kIdVprolvq,                          // {AVX512_F+VL}
    kIdVprord,                           // {AVX512_F+VL}
    kIdVprorq,                           // {AVX512_F+VL}
    kIdVprorvd,                          // {AVX512_F+VL}
    kIdVprorvq,                          // {AVX512_F+VL}
    kIdVprotb,                           // {XOP}
    kIdVprotd,                           // {XOP}
    kIdVprotq,                           // {XOP}
    kIdVprotw,                           // {XOP}
    kIdVpsadbw,                          // {AVX|AVX2|AVX512_BW+VL}
    kIdVpscatterdd,                      // {AVX512_F+VL}
    kIdVpscatterdq,                      // {AVX512_F+VL}
    kIdVpscatterqd,                      // {AVX512_F+VL}
    kIdVpscatterqq,                      // {AVX512_F+VL}
    kIdVpshab,                           // {XOP}
    kIdVpshad,                           // {XOP}
    kIdVpshaq,                           // {XOP}
    kIdVpshaw,                           // {XOP}
    kIdVpshlb,                           // {XOP}
    kIdVpshld,                           // {XOP}
    kIdVpshldd,                          // {AVX512_VBMI2+VL}
    kIdVpshldq,                          // {AVX512_VBMI2+VL}
    kIdVpshldvd,                         // {AVX512_VBMI2+VL}
    kIdVpshldvq,                         // {AVX512_VBMI2+VL}
    kIdVpshldvw,                         // {AVX512_VBMI2+VL}
    kIdVpshldw,                          // {AVX512_VBMI2+VL}
    kIdVpshlq,                           // {XOP}
    kIdVpshlw,                           // {XOP}
    kIdVpshrdd,                          // {AVX512_VBMI2+VL}
    kIdVpshrdq,                          // {AVX512_VBMI2+VL}
    kIdVpshrdvd,                         // {AVX512_VBMI2+VL}
    kIdVpshrdvq,                         // {AVX512_VBMI2+VL}
    kIdVpshrdvw,                         // {AVX512_VBMI2+VL}
    kIdVpshrdw,                          // {AVX512_VBMI2+VL}
    kIdVpshufb,                          // {AVX|AVX2|AVX512_BW+VL}
    kIdVpshufbitqmb,                     // {AVX512_BITALG+VL}
    kIdVpshufd,                          // {AVX|AVX2|AVX512_F+VL}
    kIdVpshufhw,                         // {AVX|AVX2|AVX512_BW+VL}
    kIdVpshuflw,                         // {AVX|AVX2|AVX512_BW+VL}
    kIdVpsignb,                          // {AVX|AVX2}
    kIdVpsignd,                          // {AVX|AVX2}
    kIdVpsignw,                          // {AVX|AVX2}
    kIdVpslld,                           // {AVX|AVX2|AVX512_F+VL}
    kIdVpslldq,                          // {AVX|AVX2|AVX512_BW+VL}
    kIdVpsllq,                           // {AVX|AVX2|AVX512_F+VL}
    kIdVpsllvd,                          // {AVX2|AVX512_F+VL}
    kIdVpsllvq,                          // {AVX2|AVX512_F+VL}
    kIdVpsllvw,                          // {AVX512_BW+VL}
    kIdVpsllw,                           // {AVX|AVX2|AVX512_BW+VL}
    kIdVpsrad,                           // {AVX|AVX2|AVX512_F+VL}
    kIdVpsraq,                           // {AVX512_F+VL}
    kIdVpsravd,                          // {AVX2|AVX512_F+VL}
    kIdVpsravq,                          // {AVX512_F+VL}
    kIdVpsravw,                          // {AVX512_BW+VL}
    kIdVpsraw,                           // {AVX|AVX2|AVX512_BW+VL}
    kIdVpsrld,                           // {AVX|AVX2|AVX512_F+VL}
    kIdVpsrldq,                          // {AVX|AVX2|AVX512_BW+VL}
    kIdVpsrlq,                           // {AVX|AVX2|AVX512_F+VL}
    kIdVpsrlvd,                          // {AVX2|AVX512_F+VL}
    kIdVpsrlvq,                          // {AVX2|AVX512_F+VL}
    kIdVpsrlvw,                          // {AVX512_BW+VL}
    kIdVpsrlw,                           // {AVX|AVX2|AVX512_BW+VL}
    kIdVpsubb,                           // {AVX|AVX2|AVX512_BW+VL}
    kIdVpsubd,                           // {AVX|AVX2|AVX512_F+VL}
    kIdVpsubq,                           // {AVX|AVX2|AVX512_F+VL}
    kIdVpsubsb,                          // {AVX|AVX2|AVX512_BW+VL}
    kIdVpsubsw,                          // {AVX|AVX2|AVX512_BW+VL}
    kIdVpsubusb,                         // {AVX|AVX2|AVX512_BW+VL}
    kIdVpsubusw,                         // {AVX|AVX2|AVX512_BW+VL}
    kIdVpsubw,                           // {AVX|AVX2|AVX512_BW+VL}
    kIdVpternlogd,                       // {AVX512_F+VL}
    kIdVpternlogq,                       // {AVX512_F+VL}
    kIdVptest,                           // {AVX}
    kIdVptestmb,                         // {AVX512_BW+VL}
    kIdVptestmd,                         // {AVX512_F+VL}
    kIdVptestmq,                         // {AVX512_F+VL}
    kIdVptestmw,                         // {AVX512_BW+VL}
    kIdVptestnmb,                        // {AVX512_BW+VL}
    kIdVptestnmd,                        // {AVX512_F+VL}
    kIdVptestnmq,                        // {AVX512_F+VL}
    kIdVptestnmw,                        // {AVX512_BW+VL}
    kIdVpunpckhbw,                       // {AVX|AVX2|AVX512_BW+VL}
    kIdVpunpckhdq,                       // {AVX|AVX2|AVX512_F+VL}
    kIdVpunpckhqdq,                      // {AVX|AVX2|AVX512_F+VL}
    kIdVpunpckhwd,                       // {AVX|AVX2|AVX512_BW+VL}
    kIdVpunpcklbw,                       // {AVX|AVX2|AVX512_BW+VL}
    kIdVpunpckldq,                       // {AVX|AVX2|AVX512_F+VL}
    kIdVpunpcklqdq,                      // {AVX|AVX2|AVX512_F+VL}
    kIdVpunpcklwd,                       // {AVX|AVX2|AVX512_BW+VL}
    kIdVpxor,                            // {AVX|AVX2}
    kIdVpxord,                           // {AVX512_F+VL}
    kIdVpxorq,                           // {AVX512_F+VL}
    kIdVrangepd,                         // {AVX512_DQ+VL}
    kIdVrangeps,                         // {AVX512_DQ+VL}
    kIdVrangesd,                         // {AVX512_DQ}
    kIdVrangess,                         // {AVX512_DQ}
    kIdVrcp14pd,                         // {AVX512_F+VL}
    kIdVrcp14ps,                         // {AVX512_F+VL}
    kIdVrcp14sd,                         // {AVX512_F}
    kIdVrcp14ss,                         // {AVX512_F}
    kIdVrcp28pd,                         // {AVX512_ERI}
    kIdVrcp28ps,                         // {AVX512_ERI}
    kIdVrcp28sd,                         // {AVX512_ERI}
    kIdVrcp28ss,                         // {AVX512_ERI}
    kIdVrcpps,                           // {AVX}
    kIdVrcpss,                           // {AVX}
    kIdVreducepd,                        // {AVX512_DQ+VL}
    kIdVreduceps,                        // {AVX512_DQ+VL}
    kIdVreducesd,                        // {AVX512_DQ}
    kIdVreducess,                        // {AVX512_DQ}
    kIdVrndscalepd,                      // {AVX512_F+VL}
    kIdVrndscaleps,                      // {AVX512_F+VL}
    kIdVrndscalesd,                      // {AVX512_F}
    kIdVrndscaless,                      // {AVX512_F}
    kIdVroundpd,                         // {AVX}
    kIdVroundps,                         // {AVX}
    kIdVroundsd,                         // {AVX}
    kIdVroundss,                         // {AVX}
    kIdVrsqrt14pd,                       // {AVX512_F+VL}
    kIdVrsqrt14ps,                       // {AVX512_F+VL}
    kIdVrsqrt14sd,                       // {AVX512_F}
    kIdVrsqrt14ss,                       // {AVX512_F}
    kIdVrsqrt28pd,                       // {AVX512_ERI}
    kIdVrsqrt28ps,                       // {AVX512_ERI}
    kIdVrsqrt28sd,                       // {AVX512_ERI}
    kIdVrsqrt28ss,                       // {AVX512_ERI}
    kIdVrsqrtps,                         // {AVX}
    kIdVrsqrtss,                         // {AVX}
    kIdVscalefpd,                        // {AVX512_F+VL}
    kIdVscalefps,                        // {AVX512_F+VL}
    kIdVscalefsd,                        // {AVX512_F}
    kIdVscalefss,                        // {AVX512_F}
    kIdVscatterdpd,                      // {AVX512_F+VL}
    kIdVscatterdps,                      // {AVX512_F+VL}
    kIdVscatterpf0dpd,                   // {AVX512_PFI}
    kIdVscatterpf0dps,                   // {AVX512_PFI}
    kIdVscatterpf0qpd,                   // {AVX512_PFI}
    kIdVscatterpf0qps,                   // {AVX512_PFI}
    kIdVscatterpf1dpd,                   // {AVX512_PFI}
    kIdVscatterpf1dps,                   // {AVX512_PFI}
    kIdVscatterpf1qpd,                   // {AVX512_PFI}
    kIdVscatterpf1qps,                   // {AVX512_PFI}
    kIdVscatterqpd,                      // {AVX512_F+VL}
    kIdVscatterqps,                      // {AVX512_F+VL}
    kIdVshuff32x4,                       // {AVX512_F+VL}
    kIdVshuff64x2,                       // {AVX512_F+VL}
    kIdVshufi32x4,                       // {AVX512_F+VL}
    kIdVshufi64x2,                       // {AVX512_F+VL}
    kIdVshufpd,                          // {AVX|AVX512_F+VL}
    kIdVshufps,                          // {AVX|AVX512_F+VL}
    kIdVsqrtpd,                          // {AVX|AVX512_F+VL}
    kIdVsqrtps,                          // {AVX|AVX512_F+VL}
    kIdVsqrtsd,                          // {AVX|AVX512_F}
    kIdVsqrtss,                          // {AVX|AVX512_F}
    kIdVstmxcsr,                         // {AVX}
    kIdVsubpd,                           // {AVX|AVX512_F+VL}
    kIdVsubps,                           // {AVX|AVX512_F+VL}
    kIdVsubsd,                           // {AVX|AVX512_F}
    kIdVsubss,                           // {AVX|AVX512_F}
    kIdVtestpd,                          // {AVX}
    kIdVtestps,                          // {AVX}
    kIdVucomisd,                         // {AVX|AVX512_F}
    kIdVucomiss,                         // {AVX|AVX512_F}
    kIdVunpckhpd,                        // {AVX|AVX512_F+VL}
    kIdVunpckhps,                        // {AVX|AVX512_F+VL}
    kIdVunpcklpd,                        // {AVX|AVX512_F+VL}
    kIdVunpcklps,                        // {AVX|AVX512_F+VL}
    kIdVxorpd,                           // {AVX|AVX512_DQ+VL}
    kIdVxorps,                           // {AVX|AVX512_DQ+VL}
    kIdVzeroall,                         // {AVX}
    kIdVzeroupper,                       // {AVX}
    kIdWbinvd,                           // <ANY>
    kIdWbnoinvd,                         // {WBNOINVD}
    kIdWrfsbase,                         // {FSGSBASE} & <X64>
    kIdWrgsbase,                         // {FSGSBASE} & <X64>
    kIdWrmsr,                            // {MSR}
    kIdXabort,                           // {RTM}
    kIdXadd,                             // {I486}
    kIdXbegin,                           // {RTM}
    kIdXchg,                             // <ANY>
    kIdXend,                             // {RTM}
    kIdXgetbv,                           // {XSAVE}
    kIdXlatb,                            // <ANY>
    kIdXor,                              // <ANY>
    kIdXorpd,                            // {SSE2}
    kIdXorps,                            // {SSE}
    kIdXrstor,                           // {XSAVE}
    kIdXrstor64,                         // {XSAVE} & <X64>
    kIdXrstors,                          // {XSAVES}
    kIdXrstors64,                        // {XSAVES} & <X64>
    kIdXsave,                            // {XSAVE}
    kIdXsave64,                          // {XSAVE} & <X64>
    kIdXsavec,                           // {XSAVEC}
    kIdXsavec64,                         // {XSAVEC} & <X64>
    kIdXsaveopt,                         // {XSAVEOPT}
    kIdXsaveopt64,                       // {XSAVEOPT} & <X64>
    kIdXsaves,                           // {XSAVES}
    kIdXsaves64,                         // {XSAVES} & <X64>
    kIdXsetbv,                           // {XSAVE}
    kIdXtest,                            // {TSX}
    _kIdCount
    // ${InstId:End}
  };

  //! Instruction options (X86).
  enum Options : uint32_t {
    kOptionVex3           = 0x00000400u, //!< Use 3-byte VEX prefix if possible (AVX) (must be 0x00000400).
    kOptionModMR          = 0x00000800u, //!< Use ModMR instead of ModRM when it's available.
    kOptionEvex           = 0x00001000u, //!< Use 4-byte EVEX prefix if possible (AVX-512) (must be 0x00001000).

    kOptionLock           = 0x00002000u, //!< LOCK prefix (lock-enabled instructions only).
    kOptionRep            = 0x00004000u, //!< REP prefix (string instructions only).
    kOptionRepne          = 0x00008000u, //!< REPNE prefix (string instructions only).

    kOptionXAcquire       = 0x00010000u, //!< XACQUIRE prefix (only allowed instructions).
    kOptionXRelease       = 0x00020000u, //!< XRELEASE prefix (only allowed instructions).

    kOptionER             = 0x00040000u, //!< AVX-512: embedded-rounding {er} and implicit {sae}.
    kOptionSAE            = 0x00080000u, //!< AVX-512: suppress-all-exceptions {sae}.
    kOptionRN_SAE         = 0x00000000u, //!< AVX-512: round-to-nearest (even)      {rn-sae} (bits 00).
    kOptionRD_SAE         = 0x00200000u, //!< AVX-512: round-down (toward -inf)     {rd-sae} (bits 01).
    kOptionRU_SAE         = 0x00400000u, //!< AVX-512: round-up (toward +inf)       {ru-sae} (bits 10).
    kOptionRZ_SAE         = 0x00600000u, //!< AVX-512: round-toward-zero (truncate) {rz-sae} (bits 11).
    kOptionZMask          = 0x00800000u, //!< AVX-512: Use zeroing {k}{z} instead of merging {k}.
    _kOptionAvx512Mask    = 0x00FC0000u, //!< AVX-512: Mask of all possible AVX-512 options except EVEX prefix flag.

    kOptionOpCodeB        = 0x01000000u, //!< REX.B and/or VEX.B field (X64).
    kOptionOpCodeX        = 0x02000000u, //!< REX.X and/or VEX.X field (X64).
    kOptionOpCodeR        = 0x04000000u, //!< REX.R and/or VEX.R field (X64).
    kOptionOpCodeW        = 0x08000000u, //!< REX.W and/or VEX.W field (X64).
    kOptionRex            = 0x40000000u, //!< Force REX prefix (X64).
    _kOptionInvalidRex    = 0x80000000u  //!< Invalid REX prefix (set by X86 or when AH|BH|CH|DH regs are used on X64).
  };

  // --------------------------------------------------------------------------
  // [Statics]
  // --------------------------------------------------------------------------

  //! Tests whether the `instId` is defined (counts also Inst::kIdNone, which must be zero).
  static inline bool isDefinedId(uint32_t instId) noexcept { return instId < _kIdCount; }

  //! Converts `size` to a 'kmov?' instructio.
  static inline uint32_t kmovFromSize(uint32_t size) noexcept {
    switch (size) {
      case  1: return kIdKmovb;
      case  2: return kIdKmovw;
      case  4: return kIdKmovd;
      case  8: return kIdKmovq;
      default: return kIdNone;
    }
  }
};

// ============================================================================
// [asmjit::x86::Cond]
// ============================================================================

namespace Cond {
  //! Condition code.
  enum Value : uint32_t {
    kO                    = 0x00u,       //!<                 OF==1
    kNO                   = 0x01u,       //!<                 OF==0
    kB                    = 0x02u,       //!< CF==1                  (unsigned < )
    kC                    = 0x02u,       //!< CF==1
    kNAE                  = 0x02u,       //!< CF==1                  (unsigned < )
    kAE                   = 0x03u,       //!< CF==0                  (unsigned >=)
    kNB                   = 0x03u,       //!< CF==0                  (unsigned >=)
    kNC                   = 0x03u,       //!< CF==0
    kE                    = 0x04u,       //!<         ZF==1          (any_sign ==)
    kZ                    = 0x04u,       //!<         ZF==1          (any_sign ==)
    kNE                   = 0x05u,       //!<         ZF==0          (any_sign !=)
    kNZ                   = 0x05u,       //!<         ZF==0          (any_sign !=)
    kBE                   = 0x06u,       //!< CF==1 | ZF==1          (unsigned <=)
    kNA                   = 0x06u,       //!< CF==1 | ZF==1          (unsigned <=)
    kA                    = 0x07u,       //!< CF==0 & ZF==0          (unsigned > )
    kNBE                  = 0x07u,       //!< CF==0 & ZF==0          (unsigned > )
    kS                    = 0x08u,       //!<                 SF==1  (is negative)
    kNS                   = 0x09u,       //!<                 SF==0  (is positive or zero)
    kP                    = 0x0Au,       //!< PF==1
    kPE                   = 0x0Au,       //!< PF==1
    kPO                   = 0x0Bu,       //!< PF==0
    kNP                   = 0x0Bu,       //!< PF==0
    kL                    = 0x0Cu,       //!<                 SF!=OF (signed   < )
    kNGE                  = 0x0Cu,       //!<                 SF!=OF (signed   < )
    kGE                   = 0x0Du,       //!<                 SF==OF (signed   >=)
    kNL                   = 0x0Du,       //!<                 SF==OF (signed   >=)
    kLE                   = 0x0Eu,       //!<         ZF==1 | SF!=OF (signed   <=)
    kNG                   = 0x0Eu,       //!<         ZF==1 | SF!=OF (signed   <=)
    kG                    = 0x0Fu,       //!<         ZF==0 & SF==OF (signed   > )
    kNLE                  = 0x0Fu,       //!<         ZF==0 & SF==OF (signed   > )
    kCount                = 0x10u,

    kSign                 = kS,          //!< Sign.
    kNotSign              = kNS,         //!< Not Sign.

    kOverflow             = kO,          //!< Signed overflow.
    kNotOverflow          = kNO,         //!< Not signed overflow.

    kEqual                = kE,          //!< Equal      `a == b`.
    kNotEqual             = kNE,         //!< Not Equal  `a != b`.

    kSignedLT             = kL,          //!< Signed     `a <  b`.
    kSignedLE             = kLE,         //!< Signed     `a <= b`.
    kSignedGT             = kG,          //!< Signed     `a >  b`.
    kSignedGE             = kGE,         //!< Signed     `a >= b`.

    kUnsignedLT           = kB,          //!< Unsigned   `a <  b`.
    kUnsignedLE           = kBE,         //!< Unsigned   `a <= b`.
    kUnsignedGT           = kA,          //!< Unsigned   `a >  b`.
    kUnsignedGE           = kAE,         //!< Unsigned   `a >= b`.

    kZero                 = kZ,
    kNotZero              = kNZ,

    kNegative             = kS,
    kPositive             = kNS,

    kParityEven           = kP,
    kParityOdd            = kPO
  };

  static constexpr uint8_t reverseTable[kCount] = {
    kO, kNO, kA , kBE, // O|NO|B |AE
    kE, kNE, kAE, kB , // E|NE|BE|A
    kS, kNS, kPE, kPO, // S|NS|PE|PO
    kG, kLE, kGE, kL   // L|GE|LE|G
  };

  #define ASMJIT_INST_FROM_COND(ID) \
    ID##o, ID##no, ID##b , ID##ae,  \
    ID##e, ID##ne, ID##be, ID##a ,  \
    ID##s, ID##ns, ID##pe, ID##po,  \
    ID##l, ID##ge, ID##le, ID##g
  static constexpr uint16_t jccTable[] = { ASMJIT_INST_FROM_COND(Inst::kIdJ) };
  static constexpr uint16_t setccTable[] = { ASMJIT_INST_FROM_COND(Inst::kIdSet) };
  static constexpr uint16_t cmovccTable[] = { ASMJIT_INST_FROM_COND(Inst::kIdCmov) };
  #undef ASMJIT_INST_FROM_COND

  //! Reverse a condition code (reverses the corresponding operands of a comparison).
  static constexpr uint32_t reverse(uint32_t cond) noexcept { return reverseTable[cond]; }
  //! Negate a condition code.
  static constexpr uint32_t negate(uint32_t cond) noexcept { return cond ^ 1u; }

  //! Translate a condition code `cond` to a `jcc` instruction id.
  static constexpr uint32_t toJcc(uint32_t cond) noexcept { return jccTable[cond]; }
  //! Translate a condition code `cond` to a `setcc` instruction id.
  static constexpr uint32_t toSetcc(uint32_t cond) noexcept { return setccTable[cond]; }
  //! Translate a condition code `cond` to a `cmovcc` instruction id.
  static constexpr uint32_t toCmovcc(uint32_t cond) noexcept { return cmovccTable[cond]; }
}

// ============================================================================
// [asmjit::x86::FpuWord]
// ============================================================================

//! FPU control and status word (X86).
namespace FpuWord {
  //! FPU status word.
  enum Status : uint32_t {
    kStatusInvalid        = 0x0001u,
    kStatusDenormalized   = 0x0002u,
    kStatusDivByZero      = 0x0004u,
    kStatusOverflow       = 0x0008u,
    kStatusUnderflow      = 0x0010u,
    kStatusPrecision      = 0x0020u,
    kStatusStackFault     = 0x0040u,
    kStatusInterrupt      = 0x0080u,
    kStatusC0             = 0x0100u,
    kStatusC1             = 0x0200u,
    kStatusC2             = 0x0400u,
    kStatusTop            = 0x3800u,
    kStatusC3             = 0x4000u,
    kStatusBusy           = 0x8000u
  };

  //! FPU control word.
  enum Control : uint32_t {
    // Bits 0-5.
    kControlEM_Mask       = 0x003Fu,
    kControlEM_Invalid    = 0x0001u,
    kControlEM_Denormal   = 0x0002u,
    kControlEM_DivByZero  = 0x0004u,
    kControlEM_Overflow   = 0x0008u,
    kControlEM_Underflow  = 0x0010u,
    kControlEM_Inexact    = 0x0020u,

    // Bits 8-9.
    kControlPC_Mask       = 0x0300u,
    kControlPC_Float      = 0x0000u,
    kControlPC_Reserved   = 0x0100u,
    kControlPC_Double     = 0x0200u,
    kControlPC_Extended   = 0x0300u,

    // Bits 10-11.
    kControlRC_Mask       = 0x0C00u,
    kControlRC_Nearest    = 0x0000u,
    kControlRC_Down       = 0x0400u,
    kControlRC_Up         = 0x0800u,
    kControlRC_Truncate   = 0x0C00u,

    // Bit 12.
    kControlIC_Mask       = 0x1000u,
    kControlIC_Projective = 0x0000u,
    kControlIC_Affine     = 0x1000u
  };
}

// ============================================================================
// [asmjit::x86::Status]
// ============================================================================

//! CPU and FPU status (X86).
namespace Status {
  //! Mask of status flags of both CPU and FPU.
  enum Flags : uint32_t {
    // ------------------------------------------------------------------------
    // [Architecture Neutral Flags]
    // ------------------------------------------------------------------------

    kCF = 0x00000001u, //!< Carry flag.
    kOF = 0x00000002u, //!< Signed overflow flag.
    kSF = 0x00000004u, //!< Sign flag (negative/sign, if set).
    kZF = 0x00000008u, //!< Zero and/or equality flag (1 if zero/equal).

    // ------------------------------------------------------------------------
    // [Architecture Specific Flags]
    // ------------------------------------------------------------------------

    kAF = 0x00000100u, //!< Adjust flag.
    kPF = 0x00000200u, //!< Parity flag.
    kDF = 0x00000400u, //!< Direction flag.
    kIF = 0x00000800u, //!< Interrupt enable flag.

    kAC = 0x00001000u, //!< Alignment check.

    kC0 = 0x00010000u, //!< FPU status word C0 flag.
    kC1 = 0x00020000u, //!< FPU status word C1 flag.
    kC2 = 0x00040000u, //!< FPU status word C2 flag.
    kC3 = 0x00080000u  //!< FPU status word C3 flag.
  };
}

// ============================================================================
// [asmjit::x86::Predicate]
// ============================================================================

//! Contains predicates used by SIMD instructions (X86).
namespace Predicate {
  //! A predicate used by CMP[PD|PS|SD|SS] instructions.
  enum Cmp : uint32_t {
    kCmpEQ                = 0x00u,       //!< Equal (Quiet).
    kCmpLT                = 0x01u,       //!< Less (Signaling).
    kCmpLE                = 0x02u,       //!< Less/Equal (Signaling).
    kCmpUNORD             = 0x03u,       //!< Unordered (Quiet).
    kCmpNEQ               = 0x04u,       //!< Not Equal (Quiet).
    kCmpNLT               = 0x05u,       //!< Not Less (Signaling).
    kCmpNLE               = 0x06u,       //!< Not Less/Equal (Signaling).
    kCmpORD               = 0x07u        //!< Ordered (Quiet).
  };

  //! A predicate used by [V]PCMP[I|E]STR[I|M] instructions.
  enum PCmpStr : uint32_t {
    // Source data format:
    kPCmpStrUB            = 0x00u << 0,  //!< The source data format is unsigned bytes.
    kPCmpStrUW            = 0x01u << 0,  //!< The source data format is unsigned words.
    kPCmpStrSB            = 0x02u << 0,  //!< The source data format is signed bytes.
    kPCmpStrSW            = 0x03u << 0,  //!< The source data format is signed words.

    // Aggregation operation:
    kPCmpStrEqualAny      = 0x00u << 2,  //!< The arithmetic comparison is "equal".
    kPCmpStrRanges        = 0x01u << 2,  //!< The arithmetic comparison is "greater than or equal"
                                         //!< between even indexed elements and "less than or equal"
                                         //!< between odd indexed elements.
    kPCmpStrEqualEach     = 0x02u << 2,  //!< The arithmetic comparison is "equal".
    kPCmpStrEqualOrdered  = 0x03u << 2,  //!< The arithmetic comparison is "equal".

    // Polarity:
    kPCmpStrPosPolarity   = 0x00u << 4,  //!< IntRes2 = IntRes1.
    kPCmpStrNegPolarity   = 0x01u << 4,  //!< IntRes2 = -1 XOR IntRes1.
    kPCmpStrPosMasked     = 0x02u << 4,  //!< IntRes2 = IntRes1.
    kPCmpStrNegMasked     = 0x03u << 4,  //!< IntRes2[i] = second[i] == invalid ? IntRes1[i] : ~IntRes1[i].

    // Output selection (pcmpstri):
    kPCmpStrOutputLSI     = 0x00u << 6,  //!< The index returned to ECX is of the least significant set bit in IntRes2.
    kPCmpStrOutputMSI     = 0x01u << 6,  //!< The index returned to ECX is of the most significant set bit in IntRes2.

    // Output selection (pcmpstrm):
    kPCmpStrBitMask       = 0x00u << 6,  //!< IntRes2 is returned as the mask to the least significant bits of XMM0.
    kPCmpStrIndexMask     = 0x01u << 6   //!< IntRes2 is expanded into a byte/word mask and placed in XMM0.
  };

  //! A predicate used by ROUND[PD|PS|SD|SS] instructions.
  enum Round : uint32_t {
    kRoundNearest         = 0x00u,       //!< Round to nearest (even).
    kRoundDown            = 0x01u,       //!< Round to down toward -INF (floor),
    kRoundUp              = 0x02u,       //!< Round to up toward +INF (ceil).
    kRoundTrunc           = 0x03u,       //!< Round toward zero (truncate).
    kRoundCurrent         = 0x04u,       //!< Round to the current rounding mode set (ignores other RC bits).
    kRoundInexact         = 0x08u        //!< Avoids inexact exception, if set.
  };

  //! A predicate used by VCMP[PD|PS|SD|SS] instructions.
  //!
  //! The first 8 values are compatible with `Cmp`.
  enum VCmp : uint32_t {
    kVCmpEQ_OQ            = kCmpEQ,      //!< Equal             (Quiet    , Ordered).
    kVCmpLT_OS            = kCmpLT,      //!< Less              (Signaling, Ordered).
    kVCmpLE_OS            = kCmpLE,      //!< Less/Equal        (Signaling, Ordered).
    kVCmpUNORD_Q          = kCmpUNORD,   //!< Unordered         (Quiet).
    kVCmpNEQ_UQ           = kCmpNEQ,     //!< Not Equal         (Quiet    , Unordered).
    kVCmpNLT_US           = kCmpNLT,     //!< Not Less          (Signaling, Unordered).
    kVCmpNLE_US           = kCmpNLE,     //!< Not Less/Equal    (Signaling, Unordered).
    kVCmpORD_Q            = kCmpORD,     //!< Ordered           (Quiet).
    kVCmpEQ_UQ            = 0x08u,       //!< Equal             (Quiet    , Unordered).
    kVCmpNGE_US           = 0x09u,       //!< Not Greater/Equal (Signaling, Unordered).
    kVCmpNGT_US           = 0x0Au,       //!< Not Greater       (Signaling, Unordered).
    kVCmpFALSE_OQ         = 0x0Bu,       //!< False             (Quiet    , Ordered).
    kVCmpNEQ_OQ           = 0x0Cu,       //!< Not Equal         (Quiet    , Ordered).
    kVCmpGE_OS            = 0x0Du,       //!< Greater/Equal     (Signaling, Ordered).
    kVCmpGT_OS            = 0x0Eu,       //!< Greater           (Signaling, Ordered).
    kVCmpTRUE_UQ          = 0x0Fu,       //!< True              (Quiet    , Unordered).
    kVCmpEQ_OS            = 0x10u,       //!< Equal             (Signaling, Ordered).
    kVCmpLT_OQ            = 0x11u,       //!< Less              (Quiet    , Ordered).
    kVCmpLE_OQ            = 0x12u,       //!< Less/Equal        (Quiet    , Ordered).
    kVCmpUNORD_S          = 0x13u,       //!< Unordered         (Signaling).
    kVCmpNEQ_US           = 0x14u,       //!< Not Equal         (Signaling, Unordered).
    kVCmpNLT_UQ           = 0x15u,       //!< Not Less          (Quiet    , Unordered).
    kVCmpNLE_UQ           = 0x16u,       //!< Not Less/Equal    (Quiet    , Unordered).
    kVCmpORD_S            = 0x17u,       //!< Ordered           (Signaling).
    kVCmpEQ_US            = 0x18u,       //!< Equal             (Signaling, Unordered).
    kVCmpNGE_UQ           = 0x19u,       //!< Not Greater/Equal (Quiet    , Unordered).
    kVCmpNGT_UQ           = 0x1Au,       //!< Not Greater       (Quiet    , Unordered).
    kVCmpFALSE_OS         = 0x1Bu,       //!< False             (Signaling, Ordered).
    kVCmpNEQ_OS           = 0x1Cu,       //!< Not Equal         (Signaling, Ordered).
    kVCmpGE_OQ            = 0x1Du,       //!< Greater/Equal     (Quiet    , Ordered).
    kVCmpGT_OQ            = 0x1Eu,       //!< Greater           (Quiet    , Ordered).
    kVCmpTRUE_US          = 0x1Fu        //!< True              (Signaling, Unordered).
  };

  //! A predicate used by VFIXUPIMM[PD|PS|SD|SS] instructions (AVX-512).
  enum VFixupImm : uint32_t {
    kVFixupImmZEOnZero    = 0x01u,
    kVFixupImmIEOnZero    = 0x02u,
    kVFixupImmZEOnOne     = 0x04u,
    kVFixupImmIEOnOne     = 0x08u,
    kVFixupImmIEOnSNaN    = 0x10u,
    kVFixupImmIEOnNInf    = 0x20u,
    kVFixupImmIEOnNegative= 0x40u,
    kVFixupImmIEOnPInf    = 0x80u
  };

  //! A predicate used by VFPCLASS[PD|PS|SD|SS] instructions (AVX-512).
  enum VFPClass : uint32_t {
    kVFPClassQNaN         = 0x00u,
    kVFPClassPZero        = 0x01u,
    kVFPClassNZero        = 0x02u,
    kVFPClassPInf         = 0x03u,
    kVFPClassNInf         = 0x04u,
    kVFPClassDenormal     = 0x05u,
    kVFPClassNegative     = 0x06u,
    kVFPClassSNaN         = 0x07u
  };

  //! A predicate used by VGETMANT[PD|PS|SD|SS] instructions (AVX-512).
  enum VGetMant : uint32_t {
    kVGetMant1To2         = 0x00u,
    kVGetMant1Div2To2     = 0x01u,
    kVGetMant1Div2To1     = 0x02u,
    kVGetMant3Div4To3Div2 = 0x03u,
    kVGetMantNoSign       = 0x04u,
    kVGetMantQNaNIfSign   = 0x08u
  };

  //! A predicate used by VPCMP[U][B|W|D|Q] instructions (AVX-512).
  enum VPCmp : uint32_t {
    kVPCmpEQ              = 0x00u,       //!< Equal.
    kVPCmpLT              = 0x01u,       //!< Less.
    kVPCmpLE              = 0x02u,       //!< Less/Equal.
    kVPCmpFALSE           = 0x03u,       //!< False.
    kVPCmpNE              = 0x04u,       //!< Not Equal.
    kVPCmpGE              = 0x05u,       //!< Greater/Equal.
    kVPCmpGT              = 0x06u,       //!< Greater.
    kVPCmpTRUE            = 0x07u        //!< True.
  };

  //! A predicate used by VPCOM[U][B|W|D|Q] instructions (XOP).
  enum VPCom : uint32_t {
    kVPComLT              = 0x00u,       //!< Less.
    kVPComLE              = 0x01u,       //!< Less/Equal
    kVPComGT              = 0x02u,       //!< Greater.
    kVPComGE              = 0x03u,       //!< Greater/Equal.
    kVPComEQ              = 0x04u,       //!< Equal.
    kVPComNE              = 0x05u,       //!< Not Equal.
    kVPComFALSE           = 0x06u,       //!< False.
    kVPComTRUE            = 0x07u        //!< True.
  };

  //! A predicate used by VRANGE[PD|PS|SD|SS] instructions (AVX-512).
  enum VRange : uint32_t {
    kVRangeSelectMin      = 0x00u,       //!< Select minimum value.
    kVRangeSelectMax      = 0x01u,       //!< Select maximum value.
    kVRangeSelectAbsMin   = 0x02u,       //!< Select minimum absolute value.
    kVRangeSelectAbsMax   = 0x03u,       //!< Select maximum absolute value.
    kVRangeSignSrc1       = 0x00u,       //!< Select sign of SRC1.
    kVRangeSignSrc2       = 0x04u,       //!< Select sign of SRC2.
    kVRangeSign0          = 0x08u,       //!< Set sign to 0.
    kVRangeSign1          = 0x0Cu        //!< Set sign to 1.
  };

  //! A predicate used by VREDUCE[PD|PS|SD|SS] instructions (AVX-512).
  enum VReduce : uint32_t {
    kVReduceRoundCurrent  = 0x00u,       //!< Round to the current mode set.
    kVReduceRoundEven     = 0x04u,       //!< Round to nearest even.
    kVReduceRoundDown     = 0x05u,       //!< Round down.
    kVReduceRoundUp       = 0x06u,       //!< Round up.
    kVReduceRoundTrunc    = 0x07u,       //!< Truncate.
    kVReduceSuppress      = 0x08u        //!< Suppress exceptions.
  };

  //! Pack a shuffle constant to be used by SSE/AVX/AVX-512 instructions (2 values).
  //!
  //! \param a Position of the first  component [0, 1].
  //! \param b Position of the second component [0, 1].
  //!
  //! Shuffle constants can be used to encode an immediate for these instructions:
  //!   - `shufpd|vshufpd`
  static constexpr uint32_t shuf(uint32_t a, uint32_t b) noexcept {
    return (a << 1) | b;
  }

  //! Pack a shuffle constant to be used by SSE/AVX/AVX-512 instructions (4 values).
  //!
  //! \param a Position of the first  component [0, 3].
  //! \param b Position of the second component [0, 3].
  //! \param c Position of the third  component [0, 3].
  //! \param d Position of the fourth component [0, 3].
  //!
  //! Shuffle constants can be used to encode an immediate for these instructions:
  //!   - `pshufw`
  //!   - `pshuflw|vpshuflw`
  //!   - `pshufhw|vpshufhw`
  //!   - `pshufd|vpshufd`
  //!   - `shufps|vshufps`
  static constexpr uint32_t shuf(uint32_t a, uint32_t b, uint32_t c, uint32_t d) noexcept {
    return (a << 6) | (b << 4) | (c << 2) | d;
  }
}

// ============================================================================
// [asmjit::x86::TLog]
// ============================================================================

//! Bitwise ternary logic between 3 operands introduced by AVX-512 (X86).
namespace TLog {
  //! A predicate that can be used to create a common predicate for VPTERNLOG[D|Q].
  enum Operator : uint32_t {
    k0                    = 0x00u,
    k1                    = 0xFFu,
    kA                    = 0xF0u,
    kB                    = 0xCCu,
    kC                    = 0xAAu,
    kNotA                 = kA ^ k1,
    kNotB                 = kB ^ k1,
    kNotC                 = kC ^ k1,

    kAB                   = kA & kB,
    kAC                   = kA & kC,
    kBC                   = kB & kC,
    kNotAB                = kAB ^ k1,
    kNotAC                = kAC ^ k1,
    kNotBC                = kBC ^ k1,

    kABC                  = kA & kB & kC,
    kNotABC               = kABC ^ k1
  };

  //! Creates an immediate that can be used by VPTERNLOG[D|Q] instructions.
  static constexpr uint32_t make(uint32_t b000, uint32_t b001, uint32_t b010, uint32_t b011, uint32_t b100, uint32_t b101, uint32_t b110, uint32_t b111) noexcept {
    return (b000 << 0) | (b001 << 1) | (b010 << 2) | (b011 << 3) | (b100 << 4) | (b101 << 5) | (b110 << 6) | (b111 << 7);
  }

  //! Creates an immediate that can be used by VPTERNLOG[D|Q] instructions.
  static constexpr uint32_t value(uint32_t x) noexcept { return x & 0xFF; }
  //! Negate an immediate that can be used by VPTERNLOG[D|Q] instructions.
  static constexpr uint32_t negate(uint32_t x) noexcept { return x ^ 0xFF; }
  //! Creates an if/else logic that can be used by VPTERNLOG[D|Q] instructions.
  static constexpr uint32_t ifElse(uint32_t condition, uint32_t a, uint32_t b) noexcept { return (condition & a) | (negate(condition) & b); }
}

//! \}

ASMJIT_END_SUB_NAMESPACE

#endif // _ASMJIT_X86_X86GLOBALS_H
