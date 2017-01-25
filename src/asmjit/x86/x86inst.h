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
#include "../base/operand.h"
#include "../base/utils.h"
#include "../x86/x86globals.h"

// [Api-Begin]
#include "../asmjit_apibegin.h"

namespace asmjit {

//! \addtogroup asmjit_x86
//! \{

// ============================================================================
// [asmjit::X86Inst]
// ============================================================================

//! X86/X64 instruction data.
struct X86Inst {
  //! Instruction id (AsmJit specific).
  //!
  //! Each instruction has a unique ID that is used as an index to AsmJit's
  //! instruction table. Instructions are sorted alphabetically.
  ASMJIT_ENUM(Id) {
    // ${idData:Begin}
    kIdNone = 0,
    kIdAaa,                              // [X86]
    kIdAad,                              // [X86]
    kIdAam,                              // [X86]
    kIdAas,                              // [X86]
    kIdAdc,                              // [ANY]
    kIdAdcx,                             // [ANY] {ADX}
    kIdAdd,                              // [ANY]
    kIdAddpd,                            // [ANY] {SSE2}
    kIdAddps,                            // [ANY] {SSE}
    kIdAddsd,                            // [ANY] {SSE2}
    kIdAddss,                            // [ANY] {SSE}
    kIdAddsubpd,                         // [ANY] {SSE3}
    kIdAddsubps,                         // [ANY] {SSE3}
    kIdAdox,                             // [ANY] {ADX}
    kIdAesdec,                           // [ANY] {AES}
    kIdAesdeclast,                       // [ANY] {AES}
    kIdAesenc,                           // [ANY] {AES}
    kIdAesenclast,                       // [ANY] {AES}
    kIdAesimc,                           // [ANY] {AES}
    kIdAeskeygenassist,                  // [ANY] {AES}
    kIdAnd,                              // [ANY]
    kIdAndn,                             // [ANY] {BMI}
    kIdAndnpd,                           // [ANY] {SSE2}
    kIdAndnps,                           // [ANY] {SSE}
    kIdAndpd,                            // [ANY] {SSE2}
    kIdAndps,                            // [ANY] {SSE}
    kIdBextr,                            // [ANY] {BMI}
    kIdBlcfill,                          // [ANY] {TBM}
    kIdBlci,                             // [ANY] {TBM}
    kIdBlcic,                            // [ANY] {TBM}
    kIdBlcmsk,                           // [ANY] {TBM}
    kIdBlcs,                             // [ANY] {TBM}
    kIdBlendpd,                          // [ANY] {SSE4_1}
    kIdBlendps,                          // [ANY] {SSE4_1}
    kIdBlendvpd,                         // [ANY] {SSE4_1}
    kIdBlendvps,                         // [ANY] {SSE4_1}
    kIdBlsfill,                          // [ANY] {TBM}
    kIdBlsi,                             // [ANY] {BMI}
    kIdBlsic,                            // [ANY] {TBM}
    kIdBlsmsk,                           // [ANY] {BMI}
    kIdBlsr,                             // [ANY] {BMI}
    kIdBsf,                              // [ANY]
    kIdBsr,                              // [ANY]
    kIdBswap,                            // [ANY]
    kIdBt,                               // [ANY]
    kIdBtc,                              // [ANY]
    kIdBtr,                              // [ANY]
    kIdBts,                              // [ANY]
    kIdBzhi,                             // [ANY] {BMI2}
    kIdCall,                             // [ANY]
    kIdCbw,                              // [ANY]
    kIdCdq,                              // [ANY]
    kIdCdqe,                             // [X64]
    kIdClac,                             // [ANY] {SMAP}
    kIdClc,                              // [ANY]
    kIdCld,                              // [ANY]
    kIdClflush,                          // [ANY] {CLFLUSH}
    kIdClflushopt,                       // [ANY] {CLFLUSH_OPT}
    kIdClwb,                             // [ANY] {CLWB}
    kIdClzero,                           // [ANY] {CLZERO}
    kIdCmc,                              // [ANY]
    kIdCmova,                            // [ANY] {CMOV}
    kIdCmovae,                           // [ANY] {CMOV}
    kIdCmovb,                            // [ANY] {CMOV}
    kIdCmovbe,                           // [ANY] {CMOV}
    kIdCmovc,                            // [ANY] {CMOV}
    kIdCmove,                            // [ANY] {CMOV}
    kIdCmovg,                            // [ANY] {CMOV}
    kIdCmovge,                           // [ANY] {CMOV}
    kIdCmovl,                            // [ANY] {CMOV}
    kIdCmovle,                           // [ANY] {CMOV}
    kIdCmovna,                           // [ANY] {CMOV}
    kIdCmovnae,                          // [ANY] {CMOV}
    kIdCmovnb,                           // [ANY] {CMOV}
    kIdCmovnbe,                          // [ANY] {CMOV}
    kIdCmovnc,                           // [ANY] {CMOV}
    kIdCmovne,                           // [ANY] {CMOV}
    kIdCmovng,                           // [ANY] {CMOV}
    kIdCmovnge,                          // [ANY] {CMOV}
    kIdCmovnl,                           // [ANY] {CMOV}
    kIdCmovnle,                          // [ANY] {CMOV}
    kIdCmovno,                           // [ANY] {CMOV}
    kIdCmovnp,                           // [ANY] {CMOV}
    kIdCmovns,                           // [ANY] {CMOV}
    kIdCmovnz,                           // [ANY] {CMOV}
    kIdCmovo,                            // [ANY] {CMOV}
    kIdCmovp,                            // [ANY] {CMOV}
    kIdCmovpe,                           // [ANY] {CMOV}
    kIdCmovpo,                           // [ANY] {CMOV}
    kIdCmovs,                            // [ANY] {CMOV}
    kIdCmovz,                            // [ANY] {CMOV}
    kIdCmp,                              // [ANY]
    kIdCmppd,                            // [ANY] {SSE2}
    kIdCmpps,                            // [ANY] {SSE}
    kIdCmps,                             // [ANY]
    kIdCmpsd,                            // [ANY] {SSE2}
    kIdCmpss,                            // [ANY] {SSE}
    kIdCmpxchg,                          // [ANY] {I486}
    kIdCmpxchg16b,                       // [X64] {CMPXCHG16B}
    kIdCmpxchg8b,                        // [ANY] {CMPXCHG8B}
    kIdComisd,                           // [ANY] {SSE2}
    kIdComiss,                           // [ANY] {SSE}
    kIdCpuid,                            // [ANY] {I486}
    kIdCqo,                              // [X64]
    kIdCrc32,                            // [ANY] {SSE4_2}
    kIdCvtdq2pd,                         // [ANY] {SSE2}
    kIdCvtdq2ps,                         // [ANY] {SSE2}
    kIdCvtpd2dq,                         // [ANY] {SSE2}
    kIdCvtpd2pi,                         // [ANY] {SSE2}
    kIdCvtpd2ps,                         // [ANY] {SSE2}
    kIdCvtpi2pd,                         // [ANY] {SSE2}
    kIdCvtpi2ps,                         // [ANY] {SSE}
    kIdCvtps2dq,                         // [ANY] {SSE2}
    kIdCvtps2pd,                         // [ANY] {SSE2}
    kIdCvtps2pi,                         // [ANY] {SSE}
    kIdCvtsd2si,                         // [ANY] {SSE2}
    kIdCvtsd2ss,                         // [ANY] {SSE2}
    kIdCvtsi2sd,                         // [ANY] {SSE2}
    kIdCvtsi2ss,                         // [ANY] {SSE}
    kIdCvtss2sd,                         // [ANY] {SSE2}
    kIdCvtss2si,                         // [ANY] {SSE}
    kIdCvttpd2dq,                        // [ANY] {SSE2}
    kIdCvttpd2pi,                        // [ANY] {SSE2}
    kIdCvttps2dq,                        // [ANY] {SSE2}
    kIdCvttps2pi,                        // [ANY] {SSE}
    kIdCvttsd2si,                        // [ANY] {SSE2}
    kIdCvttss2si,                        // [ANY] {SSE}
    kIdCwd,                              // [ANY]
    kIdCwde,                             // [ANY]
    kIdDaa,                              // [X86]
    kIdDas,                              // [X86]
    kIdDec,                              // [ANY]
    kIdDiv,                              // [ANY]
    kIdDivpd,                            // [ANY] {SSE2}
    kIdDivps,                            // [ANY] {SSE}
    kIdDivsd,                            // [ANY] {SSE2}
    kIdDivss,                            // [ANY] {SSE}
    kIdDppd,                             // [ANY] {SSE4_1}
    kIdDpps,                             // [ANY] {SSE4_1}
    kIdEmms,                             // [ANY] {MMX}
    kIdEnter,                            // [ANY]
    kIdExtractps,                        // [ANY] {SSE4_1}
    kIdExtrq,                            // [ANY] {SSE4A}
    kIdF2xm1,                            // [ANY]
    kIdFabs,                             // [ANY]
    kIdFadd,                             // [ANY]
    kIdFaddp,                            // [ANY]
    kIdFbld,                             // [ANY]
    kIdFbstp,                            // [ANY]
    kIdFchs,                             // [ANY]
    kIdFclex,                            // [ANY]
    kIdFcmovb,                           // [ANY] {CMOV}
    kIdFcmovbe,                          // [ANY] {CMOV}
    kIdFcmove,                           // [ANY] {CMOV}
    kIdFcmovnb,                          // [ANY] {CMOV}
    kIdFcmovnbe,                         // [ANY] {CMOV}
    kIdFcmovne,                          // [ANY] {CMOV}
    kIdFcmovnu,                          // [ANY] {CMOV}
    kIdFcmovu,                           // [ANY] {CMOV}
    kIdFcom,                             // [ANY]
    kIdFcomi,                            // [ANY]
    kIdFcomip,                           // [ANY]
    kIdFcomp,                            // [ANY]
    kIdFcompp,                           // [ANY]
    kIdFcos,                             // [ANY]
    kIdFdecstp,                          // [ANY]
    kIdFdiv,                             // [ANY]
    kIdFdivp,                            // [ANY]
    kIdFdivr,                            // [ANY]
    kIdFdivrp,                           // [ANY]
    kIdFemms,                            // [ANY] {3DNOW}
    kIdFfree,                            // [ANY]
    kIdFiadd,                            // [ANY]
    kIdFicom,                            // [ANY]
    kIdFicomp,                           // [ANY]
    kIdFidiv,                            // [ANY]
    kIdFidivr,                           // [ANY]
    kIdFild,                             // [ANY]
    kIdFimul,                            // [ANY]
    kIdFincstp,                          // [ANY]
    kIdFinit,                            // [ANY]
    kIdFist,                             // [ANY]
    kIdFistp,                            // [ANY]
    kIdFisttp,                           // [ANY] {SSE3}
    kIdFisub,                            // [ANY]
    kIdFisubr,                           // [ANY]
    kIdFld,                              // [ANY]
    kIdFld1,                             // [ANY]
    kIdFldcw,                            // [ANY]
    kIdFldenv,                           // [ANY]
    kIdFldl2e,                           // [ANY]
    kIdFldl2t,                           // [ANY]
    kIdFldlg2,                           // [ANY]
    kIdFldln2,                           // [ANY]
    kIdFldpi,                            // [ANY]
    kIdFldz,                             // [ANY]
    kIdFmul,                             // [ANY]
    kIdFmulp,                            // [ANY]
    kIdFnclex,                           // [ANY]
    kIdFninit,                           // [ANY]
    kIdFnop,                             // [ANY]
    kIdFnsave,                           // [ANY]
    kIdFnstcw,                           // [ANY]
    kIdFnstenv,                          // [ANY]
    kIdFnstsw,                           // [ANY]
    kIdFpatan,                           // [ANY]
    kIdFprem,                            // [ANY]
    kIdFprem1,                           // [ANY]
    kIdFptan,                            // [ANY]
    kIdFrndint,                          // [ANY]
    kIdFrstor,                           // [ANY]
    kIdFsave,                            // [ANY]
    kIdFscale,                           // [ANY]
    kIdFsin,                             // [ANY]
    kIdFsincos,                          // [ANY]
    kIdFsqrt,                            // [ANY]
    kIdFst,                              // [ANY]
    kIdFstcw,                            // [ANY]
    kIdFstenv,                           // [ANY]
    kIdFstp,                             // [ANY]
    kIdFstsw,                            // [ANY]
    kIdFsub,                             // [ANY]
    kIdFsubp,                            // [ANY]
    kIdFsubr,                            // [ANY]
    kIdFsubrp,                           // [ANY]
    kIdFtst,                             // [ANY]
    kIdFucom,                            // [ANY]
    kIdFucomi,                           // [ANY]
    kIdFucomip,                          // [ANY]
    kIdFucomp,                           // [ANY]
    kIdFucompp,                          // [ANY]
    kIdFwait,                            // [ANY]
    kIdFxam,                             // [ANY]
    kIdFxch,                             // [ANY]
    kIdFxrstor,                          // [ANY] {FXSR}
    kIdFxrstor64,                        // [X64] {FXSR}
    kIdFxsave,                           // [ANY] {FXSR}
    kIdFxsave64,                         // [X64] {FXSR}
    kIdFxtract,                          // [ANY]
    kIdFyl2x,                            // [ANY]
    kIdFyl2xp1,                          // [ANY]
    kIdHaddpd,                           // [ANY] {SSE3}
    kIdHaddps,                           // [ANY] {SSE3}
    kIdHsubpd,                           // [ANY] {SSE3}
    kIdHsubps,                           // [ANY] {SSE3}
    kIdIdiv,                             // [ANY]
    kIdImul,                             // [ANY]
    kIdIn,                               // [ANY]
    kIdInc,                              // [ANY]
    kIdIns,                              // [ANY]
    kIdInsertps,                         // [ANY] {SSE4_1}
    kIdInsertq,                          // [ANY] {SSE4A}
    kIdInt,                              // [ANY]
    kIdInt3,                             // [ANY]
    kIdInto,                             // [ANY]
    kIdJa,                               // [ANY]
    kIdJae,                              // [ANY]
    kIdJb,                               // [ANY]
    kIdJbe,                              // [ANY]
    kIdJc,                               // [ANY]
    kIdJe,                               // [ANY]
    kIdJecxz,                            // [ANY]
    kIdJg,                               // [ANY]
    kIdJge,                              // [ANY]
    kIdJl,                               // [ANY]
    kIdJle,                              // [ANY]
    kIdJmp,                              // [ANY]
    kIdJna,                              // [ANY]
    kIdJnae,                             // [ANY]
    kIdJnb,                              // [ANY]
    kIdJnbe,                             // [ANY]
    kIdJnc,                              // [ANY]
    kIdJne,                              // [ANY]
    kIdJng,                              // [ANY]
    kIdJnge,                             // [ANY]
    kIdJnl,                              // [ANY]
    kIdJnle,                             // [ANY]
    kIdJno,                              // [ANY]
    kIdJnp,                              // [ANY]
    kIdJns,                              // [ANY]
    kIdJnz,                              // [ANY]
    kIdJo,                               // [ANY]
    kIdJp,                               // [ANY]
    kIdJpe,                              // [ANY]
    kIdJpo,                              // [ANY]
    kIdJs,                               // [ANY]
    kIdJz,                               // [ANY]
    kIdKaddb,                            // [ANY] {AVX512_DQ}
    kIdKaddd,                            // [ANY] {AVX512_BW}
    kIdKaddq,                            // [ANY] {AVX512_BW}
    kIdKaddw,                            // [ANY] {AVX512_DQ}
    kIdKandb,                            // [ANY] {AVX512_DQ}
    kIdKandd,                            // [ANY] {AVX512_BW}
    kIdKandnb,                           // [ANY] {AVX512_DQ}
    kIdKandnd,                           // [ANY] {AVX512_BW}
    kIdKandnq,                           // [ANY] {AVX512_BW}
    kIdKandnw,                           // [ANY] {AVX512_F}
    kIdKandq,                            // [ANY] {AVX512_BW}
    kIdKandw,                            // [ANY] {AVX512_F}
    kIdKmovb,                            // [ANY] {AVX512_DQ}
    kIdKmovd,                            // [ANY] {AVX512_BW}
    kIdKmovq,                            // [ANY] {AVX512_BW}
    kIdKmovw,                            // [ANY] {AVX512_F}
    kIdKnotb,                            // [ANY] {AVX512_DQ}
    kIdKnotd,                            // [ANY] {AVX512_BW}
    kIdKnotq,                            // [ANY] {AVX512_BW}
    kIdKnotw,                            // [ANY] {AVX512_F}
    kIdKorb,                             // [ANY] {AVX512_DQ}
    kIdKord,                             // [ANY] {AVX512_BW}
    kIdKorq,                             // [ANY] {AVX512_BW}
    kIdKortestb,                         // [ANY] {AVX512_DQ}
    kIdKortestd,                         // [ANY] {AVX512_BW}
    kIdKortestq,                         // [ANY] {AVX512_BW}
    kIdKortestw,                         // [ANY] {AVX512_F}
    kIdKorw,                             // [ANY] {AVX512_F}
    kIdKshiftlb,                         // [ANY] {AVX512_DQ}
    kIdKshiftld,                         // [ANY] {AVX512_BW}
    kIdKshiftlq,                         // [ANY] {AVX512_BW}
    kIdKshiftlw,                         // [ANY] {AVX512_F}
    kIdKshiftrb,                         // [ANY] {AVX512_DQ}
    kIdKshiftrd,                         // [ANY] {AVX512_BW}
    kIdKshiftrq,                         // [ANY] {AVX512_BW}
    kIdKshiftrw,                         // [ANY] {AVX512_F}
    kIdKtestb,                           // [ANY] {AVX512_DQ}
    kIdKtestd,                           // [ANY] {AVX512_BW}
    kIdKtestq,                           // [ANY] {AVX512_BW}
    kIdKtestw,                           // [ANY] {AVX512_DQ}
    kIdKunpckbw,                         // [ANY] {AVX512_F}
    kIdKunpckdq,                         // [ANY] {AVX512_BW}
    kIdKunpckwd,                         // [ANY] {AVX512_BW}
    kIdKxnorb,                           // [ANY] {AVX512_DQ}
    kIdKxnord,                           // [ANY] {AVX512_BW}
    kIdKxnorq,                           // [ANY] {AVX512_BW}
    kIdKxnorw,                           // [ANY] {AVX512_F}
    kIdKxorb,                            // [ANY] {AVX512_DQ}
    kIdKxord,                            // [ANY] {AVX512_BW}
    kIdKxorq,                            // [ANY] {AVX512_BW}
    kIdKxorw,                            // [ANY] {AVX512_F}
    kIdLahf,                             // [ANY] {LAHFSAHF}
    kIdLddqu,                            // [ANY] {SSE3}
    kIdLdmxcsr,                          // [ANY] {SSE}
    kIdLea,                              // [ANY]
    kIdLeave,                            // [ANY]
    kIdLfence,                           // [ANY] {SSE2}
    kIdLods,                             // [ANY]
    kIdLoop,                             // [ANY]
    kIdLoope,                            // [ANY]
    kIdLoopne,                           // [ANY]
    kIdLzcnt,                            // [ANY] {LZCNT}
    kIdMaskmovdqu,                       // [ANY] {SSE2}
    kIdMaskmovq,                         // [ANY] {MMX2}
    kIdMaxpd,                            // [ANY] {SSE2}
    kIdMaxps,                            // [ANY] {SSE}
    kIdMaxsd,                            // [ANY] {SSE2}
    kIdMaxss,                            // [ANY] {SSE}
    kIdMfence,                           // [ANY] {SSE2}
    kIdMinpd,                            // [ANY] {SSE2}
    kIdMinps,                            // [ANY] {SSE}
    kIdMinsd,                            // [ANY] {SSE2}
    kIdMinss,                            // [ANY] {SSE}
    kIdMonitor,
    kIdMov,                              // [ANY]
    kIdMovapd,                           // [ANY] {SSE2}
    kIdMovaps,                           // [ANY] {SSE}
    kIdMovbe,                            // [ANY] {MOVBE}
    kIdMovd,                             // [ANY] {MMX|SSE2}
    kIdMovddup,                          // [ANY] {SSE3}
    kIdMovdq2q,                          // [ANY] {SSE2}
    kIdMovdqa,                           // [ANY] {SSE2}
    kIdMovdqu,                           // [ANY] {SSE2}
    kIdMovhlps,                          // [ANY] {SSE}
    kIdMovhpd,                           // [ANY] {SSE2}
    kIdMovhps,                           // [ANY] {SSE}
    kIdMovlhps,                          // [ANY] {SSE}
    kIdMovlpd,                           // [ANY] {SSE2}
    kIdMovlps,                           // [ANY] {SSE}
    kIdMovmskpd,                         // [ANY] {SSE2}
    kIdMovmskps,                         // [ANY] {SSE}
    kIdMovntdq,                          // [ANY] {SSE2}
    kIdMovntdqa,                         // [ANY] {SSE4_1}
    kIdMovnti,                           // [ANY] {SSE2}
    kIdMovntpd,                          // [ANY] {SSE2}
    kIdMovntps,                          // [ANY] {SSE}
    kIdMovntq,                           // [ANY] {MMX2}
    kIdMovntsd,                          // [ANY] {SSE4A}
    kIdMovntss,                          // [ANY] {SSE4A}
    kIdMovq,                             // [ANY] {MMX|SSE2}
    kIdMovq2dq,                          // [ANY] {SSE2}
    kIdMovs,                             // [ANY]
    kIdMovsd,                            // [ANY] {SSE2}
    kIdMovshdup,                         // [ANY] {SSE3}
    kIdMovsldup,                         // [ANY] {SSE3}
    kIdMovss,                            // [ANY] {SSE}
    kIdMovsx,                            // [ANY]
    kIdMovsxd,                           // [X64]
    kIdMovupd,                           // [ANY] {SSE2}
    kIdMovups,                           // [ANY] {SSE}
    kIdMovzx,                            // [ANY]
    kIdMpsadbw,                          // [ANY] {SSE4_1}
    kIdMul,                              // [ANY]
    kIdMulpd,                            // [ANY] {SSE2}
    kIdMulps,                            // [ANY] {SSE}
    kIdMulsd,                            // [ANY] {SSE2}
    kIdMulss,                            // [ANY] {SSE}
    kIdMulx,                             // [ANY] {BMI2}
    kIdMwait,
    kIdNeg,                              // [ANY]
    kIdNop,                              // [ANY]
    kIdNot,                              // [ANY]
    kIdOr,                               // [ANY]
    kIdOrpd,                             // [ANY] {SSE2}
    kIdOrps,                             // [ANY] {SSE}
    kIdOut,                              // [ANY]
    kIdOuts,                             // [ANY]
    kIdPabsb,                            // [ANY] {SSSE3}
    kIdPabsd,                            // [ANY] {SSSE3}
    kIdPabsw,                            // [ANY] {SSSE3}
    kIdPackssdw,                         // [ANY] {MMX|SSE2}
    kIdPacksswb,                         // [ANY] {MMX|SSE2}
    kIdPackusdw,                         // [ANY] {SSE4_1}
    kIdPackuswb,                         // [ANY] {MMX|SSE2}
    kIdPaddb,                            // [ANY] {MMX|SSE2}
    kIdPaddd,                            // [ANY] {MMX|SSE2}
    kIdPaddq,                            // [ANY] {SSE2}
    kIdPaddsb,                           // [ANY] {MMX|SSE2}
    kIdPaddsw,                           // [ANY] {MMX|SSE2}
    kIdPaddusb,                          // [ANY] {MMX|SSE2}
    kIdPaddusw,                          // [ANY] {MMX|SSE2}
    kIdPaddw,                            // [ANY] {MMX|SSE2}
    kIdPalignr,                          // [ANY] {SSE3}
    kIdPand,                             // [ANY] {MMX|SSE2}
    kIdPandn,                            // [ANY] {MMX|SSE2}
    kIdPause,                            // [ANY]
    kIdPavgb,                            // [ANY] {MMX2|SSE2}
    kIdPavgusb,                          // [ANY] {3DNOW}
    kIdPavgw,                            // [ANY] {MMX2|SSE2}
    kIdPblendvb,                         // [ANY] {SSE4_1}
    kIdPblendw,                          // [ANY] {SSE4_1}
    kIdPclmulqdq,                        // [ANY] {PCLMULQDQ}
    kIdPcmpeqb,                          // [ANY] {MMX|SSE2}
    kIdPcmpeqd,                          // [ANY] {MMX|SSE2}
    kIdPcmpeqq,                          // [ANY] {SSE4_1}
    kIdPcmpeqw,                          // [ANY] {MMX|SSE2}
    kIdPcmpestri,                        // [ANY] {SSE4_2}
    kIdPcmpestrm,                        // [ANY] {SSE4_2}
    kIdPcmpgtb,                          // [ANY] {MMX|SSE2}
    kIdPcmpgtd,                          // [ANY] {MMX|SSE2}
    kIdPcmpgtq,                          // [ANY] {SSE4_1}
    kIdPcmpgtw,                          // [ANY] {MMX|SSE2}
    kIdPcmpistri,                        // [ANY] {SSE4_2}
    kIdPcmpistrm,                        // [ANY] {SSE4_2}
    kIdPcommit,                          // [ANY] {PCOMMIT}
    kIdPdep,                             // [ANY] {BMI2}
    kIdPext,                             // [ANY] {BMI2}
    kIdPextrb,                           // [ANY] {SSE4_1}
    kIdPextrd,                           // [ANY] {SSE4_1}
    kIdPextrq,                           // [X64] {SSE4_1}
    kIdPextrw,                           // [ANY] {MMX2|SSE2|SSE4_1}
    kIdPf2id,                            // [ANY] {3DNOW}
    kIdPf2iw,                            // [ANY] {3DNOW2}
    kIdPfacc,                            // [ANY] {3DNOW}
    kIdPfadd,                            // [ANY] {3DNOW}
    kIdPfcmpeq,                          // [ANY] {3DNOW}
    kIdPfcmpge,                          // [ANY] {3DNOW}
    kIdPfcmpgt,                          // [ANY] {3DNOW}
    kIdPfmax,                            // [ANY] {3DNOW}
    kIdPfmin,                            // [ANY] {3DNOW}
    kIdPfmul,                            // [ANY] {3DNOW}
    kIdPfnacc,                           // [ANY] {3DNOW2}
    kIdPfpnacc,                          // [ANY] {3DNOW2}
    kIdPfrcp,                            // [ANY] {3DNOW}
    kIdPfrcpit1,                         // [ANY] {3DNOW}
    kIdPfrcpit2,                         // [ANY] {3DNOW}
    kIdPfrcpv,                           // [ANY] {GEODE}
    kIdPfrsqit1,                         // [ANY] {3DNOW}
    kIdPfrsqrt,                          // [ANY] {3DNOW}
    kIdPfrsqrtv,                         // [ANY] {GEODE}
    kIdPfsub,                            // [ANY] {3DNOW}
    kIdPfsubr,                           // [ANY] {3DNOW}
    kIdPhaddd,                           // [ANY] {SSSE3}
    kIdPhaddsw,                          // [ANY] {SSSE3}
    kIdPhaddw,                           // [ANY] {SSSE3}
    kIdPhminposuw,                       // [ANY] {SSE4_1}
    kIdPhsubd,                           // [ANY] {SSSE3}
    kIdPhsubsw,                          // [ANY] {SSSE3}
    kIdPhsubw,                           // [ANY] {SSSE3}
    kIdPi2fd,                            // [ANY] {3DNOW}
    kIdPi2fw,                            // [ANY] {3DNOW2}
    kIdPinsrb,                           // [ANY] {SSE4_1}
    kIdPinsrd,                           // [ANY] {SSE4_1}
    kIdPinsrq,                           // [X64] {SSE4_1}
    kIdPinsrw,                           // [ANY] {MMX2|SSE2}
    kIdPmaddubsw,                        // [ANY] {SSSE3}
    kIdPmaddwd,                          // [ANY] {MMX|SSE2}
    kIdPmaxsb,                           // [ANY] {SSE4_1}
    kIdPmaxsd,                           // [ANY] {SSE4_1}
    kIdPmaxsw,                           // [ANY] {MMX2|SSE2}
    kIdPmaxub,                           // [ANY] {MMX2|SSE2}
    kIdPmaxud,                           // [ANY] {SSE4_1}
    kIdPmaxuw,                           // [ANY] {SSE4_1}
    kIdPminsb,                           // [ANY] {SSE4_1}
    kIdPminsd,                           // [ANY] {SSE4_1}
    kIdPminsw,                           // [ANY] {MMX2|SSE2}
    kIdPminub,                           // [ANY] {MMX2|SSE2}
    kIdPminud,                           // [ANY] {SSE4_1}
    kIdPminuw,                           // [ANY] {SSE4_1}
    kIdPmovmskb,                         // [ANY] {MMX2|SSE2}
    kIdPmovsxbd,                         // [ANY] {SSE4_1}
    kIdPmovsxbq,                         // [ANY] {SSE4_1}
    kIdPmovsxbw,                         // [ANY] {SSE4_1}
    kIdPmovsxdq,                         // [ANY] {SSE4_1}
    kIdPmovsxwd,                         // [ANY] {SSE4_1}
    kIdPmovsxwq,                         // [ANY] {SSE4_1}
    kIdPmovzxbd,                         // [ANY] {SSE4_1}
    kIdPmovzxbq,                         // [ANY] {SSE4_1}
    kIdPmovzxbw,                         // [ANY] {SSE4_1}
    kIdPmovzxdq,                         // [ANY] {SSE4_1}
    kIdPmovzxwd,                         // [ANY] {SSE4_1}
    kIdPmovzxwq,                         // [ANY] {SSE4_1}
    kIdPmuldq,                           // [ANY] {SSE4_1}
    kIdPmulhrsw,                         // [ANY] {SSSE3}
    kIdPmulhrw,                          // [ANY] {3DNOW}
    kIdPmulhuw,                          // [ANY] {MMX2|SSE2}
    kIdPmulhw,                           // [ANY] {MMX|SSE2}
    kIdPmulld,                           // [ANY] {SSE4_1}
    kIdPmullw,                           // [ANY] {MMX|SSE2}
    kIdPmuludq,                          // [ANY] {SSE2}
    kIdPop,                              // [ANY]
    kIdPopa,                             // [X86]
    kIdPopad,                            // [X86]
    kIdPopcnt,                           // [ANY] {POPCNT}
    kIdPopf,                             // [ANY]
    kIdPopfd,                            // [X86]
    kIdPopfq,                            // [X64]
    kIdPor,                              // [ANY] {MMX|SSE2}
    kIdPrefetch,                         // [ANY] {3DNOW}
    kIdPrefetchnta,                      // [ANY] {MMX2}
    kIdPrefetcht0,                       // [ANY] {MMX2}
    kIdPrefetcht1,                       // [ANY] {MMX2}
    kIdPrefetcht2,                       // [ANY] {MMX2}
    kIdPrefetchw,                        // [ANY] {PREFETCHW}
    kIdPrefetchwt1,                      // [ANY] {PREFETCHWT1}
    kIdPsadbw,                           // [ANY] {MMX2|SSE2}
    kIdPshufb,                           // [ANY] {SSSE3}
    kIdPshufd,                           // [ANY] {SSE2}
    kIdPshufhw,                          // [ANY] {SSE2}
    kIdPshuflw,                          // [ANY] {SSE2}
    kIdPshufw,                           // [ANY] {MMX2}
    kIdPsignb,                           // [ANY] {SSSE3}
    kIdPsignd,                           // [ANY] {SSSE3}
    kIdPsignw,                           // [ANY] {SSSE3}
    kIdPslld,                            // [ANY] {MMX|SSE2}
    kIdPslldq,                           // [ANY] {SSE2}
    kIdPsllq,                            // [ANY] {MMX|SSE2}
    kIdPsllw,                            // [ANY] {MMX|SSE2}
    kIdPsrad,                            // [ANY] {MMX|SSE2}
    kIdPsraw,                            // [ANY] {MMX|SSE2}
    kIdPsrld,                            // [ANY] {MMX|SSE2}
    kIdPsrldq,                           // [ANY] {SSE2}
    kIdPsrlq,                            // [ANY] {MMX|SSE2}
    kIdPsrlw,                            // [ANY] {MMX|SSE2}
    kIdPsubb,                            // [ANY] {MMX|SSE2}
    kIdPsubd,                            // [ANY] {MMX|SSE2}
    kIdPsubq,                            // [ANY] {SSE2}
    kIdPsubsb,                           // [ANY] {MMX|SSE2}
    kIdPsubsw,                           // [ANY] {MMX|SSE2}
    kIdPsubusb,                          // [ANY] {MMX|SSE2}
    kIdPsubusw,                          // [ANY] {MMX|SSE2}
    kIdPsubw,                            // [ANY] {MMX|SSE2}
    kIdPswapd,                           // [ANY] {3DNOW2}
    kIdPtest,                            // [ANY] {SSE4_1}
    kIdPunpckhbw,                        // [ANY] {MMX|SSE2}
    kIdPunpckhdq,                        // [ANY] {MMX|SSE2}
    kIdPunpckhqdq,                       // [ANY] {SSE2}
    kIdPunpckhwd,                        // [ANY] {MMX|SSE2}
    kIdPunpcklbw,                        // [ANY] {MMX|SSE2}
    kIdPunpckldq,                        // [ANY] {MMX|SSE2}
    kIdPunpcklqdq,                       // [ANY] {SSE2}
    kIdPunpcklwd,                        // [ANY] {MMX|SSE2}
    kIdPush,                             // [ANY]
    kIdPusha,                            // [X86]
    kIdPushad,                           // [X86]
    kIdPushf,                            // [ANY]
    kIdPushfd,                           // [X86]
    kIdPushfq,                           // [X64]
    kIdPxor,                             // [ANY] {MMX|SSE2}
    kIdRcl,                              // [ANY]
    kIdRcpps,                            // [ANY] {SSE}
    kIdRcpss,                            // [ANY] {SSE}
    kIdRcr,                              // [ANY]
    kIdRdfsbase,                         // [X64] {FSGSBASE}
    kIdRdgsbase,                         // [X64] {FSGSBASE}
    kIdRdrand,                           // [ANY] {RDRAND}
    kIdRdseed,                           // [ANY] {RDSEED}
    kIdRdtsc,                            // [ANY] {RDTSC}
    kIdRdtscp,                           // [ANY] {RDTSCP}
    kIdRet,                              // [ANY]
    kIdRol,                              // [ANY]
    kIdRor,                              // [ANY]
    kIdRorx,                             // [ANY] {BMI2}
    kIdRoundpd,                          // [ANY] {SSE4_1}
    kIdRoundps,                          // [ANY] {SSE4_1}
    kIdRoundsd,                          // [ANY] {SSE4_1}
    kIdRoundss,                          // [ANY] {SSE4_1}
    kIdRsqrtps,                          // [ANY] {SSE}
    kIdRsqrtss,                          // [ANY] {SSE}
    kIdSahf,                             // [ANY] {LAHFSAHF}
    kIdSal,                              // [ANY]
    kIdSar,                              // [ANY]
    kIdSarx,                             // [ANY] {BMI2}
    kIdSbb,                              // [ANY]
    kIdScas,                             // [ANY]
    kIdSeta,                             // [ANY]
    kIdSetae,                            // [ANY]
    kIdSetb,                             // [ANY]
    kIdSetbe,                            // [ANY]
    kIdSetc,                             // [ANY]
    kIdSete,                             // [ANY]
    kIdSetg,                             // [ANY]
    kIdSetge,                            // [ANY]
    kIdSetl,                             // [ANY]
    kIdSetle,                            // [ANY]
    kIdSetna,                            // [ANY]
    kIdSetnae,                           // [ANY]
    kIdSetnb,                            // [ANY]
    kIdSetnbe,                           // [ANY]
    kIdSetnc,                            // [ANY]
    kIdSetne,                            // [ANY]
    kIdSetng,                            // [ANY]
    kIdSetnge,                           // [ANY]
    kIdSetnl,                            // [ANY]
    kIdSetnle,                           // [ANY]
    kIdSetno,                            // [ANY]
    kIdSetnp,                            // [ANY]
    kIdSetns,                            // [ANY]
    kIdSetnz,                            // [ANY]
    kIdSeto,                             // [ANY]
    kIdSetp,                             // [ANY]
    kIdSetpe,                            // [ANY]
    kIdSetpo,                            // [ANY]
    kIdSets,                             // [ANY]
    kIdSetz,                             // [ANY]
    kIdSfence,                           // [ANY] {MMX2}
    kIdSha1msg1,                         // [ANY] {SHA}
    kIdSha1msg2,                         // [ANY] {SHA}
    kIdSha1nexte,                        // [ANY] {SHA}
    kIdSha1rnds4,                        // [ANY] {SHA}
    kIdSha256msg1,                       // [ANY] {SHA}
    kIdSha256msg2,                       // [ANY] {SHA}
    kIdSha256rnds2,                      // [ANY] {SHA}
    kIdShl,                              // [ANY]
    kIdShld,                             // [ANY]
    kIdShlx,                             // [ANY] {BMI2}
    kIdShr,                              // [ANY]
    kIdShrd,                             // [ANY]
    kIdShrx,                             // [ANY] {BMI2}
    kIdShufpd,                           // [ANY] {SSE2}
    kIdShufps,                           // [ANY] {SSE}
    kIdSqrtpd,                           // [ANY] {SSE2}
    kIdSqrtps,                           // [ANY] {SSE}
    kIdSqrtsd,                           // [ANY] {SSE2}
    kIdSqrtss,                           // [ANY] {SSE}
    kIdStac,                             // [ANY] {SMAP}
    kIdStc,                              // [ANY]
    kIdStd,                              // [ANY]
    kIdSti,                              // [ANY]
    kIdStmxcsr,                          // [ANY] {SSE}
    kIdStos,                             // [ANY]
    kIdSub,                              // [ANY]
    kIdSubpd,                            // [ANY] {SSE2}
    kIdSubps,                            // [ANY] {SSE}
    kIdSubsd,                            // [ANY] {SSE2}
    kIdSubss,                            // [ANY] {SSE}
    kIdSwapgs,                           // [X64]
    kIdT1mskc,                           // [ANY] {TBM}
    kIdTest,                             // [ANY]
    kIdTzcnt,                            // [ANY] {BMI}
    kIdTzmsk,                            // [ANY] {TBM}
    kIdUcomisd,                          // [ANY] {SSE2}
    kIdUcomiss,                          // [ANY] {SSE}
    kIdUd2,                              // [ANY]
    kIdUnpckhpd,                         // [ANY] {SSE2}
    kIdUnpckhps,                         // [ANY] {SSE}
    kIdUnpcklpd,                         // [ANY] {SSE2}
    kIdUnpcklps,                         // [ANY] {SSE}
    kIdVaddpd,                           // [ANY] {AVX|AVX512_F (VL)}
    kIdVaddps,                           // [ANY] {AVX|AVX512_F (VL)}
    kIdVaddsd,                           // [ANY] {AVX|AVX512_F}
    kIdVaddss,                           // [ANY] {AVX|AVX512_F}
    kIdVaddsubpd,                        // [ANY] {AVX}
    kIdVaddsubps,                        // [ANY] {AVX}
    kIdVaesdec,                          // [ANY] {AES|AVX}
    kIdVaesdeclast,                      // [ANY] {AES|AVX}
    kIdVaesenc,                          // [ANY] {AES|AVX}
    kIdVaesenclast,                      // [ANY] {AES|AVX}
    kIdVaesimc,                          // [ANY] {AES|AVX}
    kIdVaeskeygenassist,                 // [ANY] {AES|AVX}
    kIdValignd,                          // [ANY] {AVX512_F (VL)}
    kIdValignq,                          // [ANY] {AVX512_F (VL)}
    kIdVandnpd,                          // [ANY] {AVX|AVX512_DQ (VL)}
    kIdVandnps,                          // [ANY] {AVX|AVX512_DQ (VL)}
    kIdVandpd,                           // [ANY] {AVX|AVX512_DQ (VL)}
    kIdVandps,                           // [ANY] {AVX|AVX512_DQ (VL)}
    kIdVblendmb,                         // [ANY] {AVX512_BW (VL)}
    kIdVblendmd,                         // [ANY] {AVX512_F (VL)}
    kIdVblendmpd,                        // [ANY] {AVX512_F (VL)}
    kIdVblendmps,                        // [ANY] {AVX512_F (VL)}
    kIdVblendmq,                         // [ANY] {AVX512_F (VL)}
    kIdVblendmw,                         // [ANY] {AVX512_BW (VL)}
    kIdVblendpd,                         // [ANY] {AVX}
    kIdVblendps,                         // [ANY] {AVX}
    kIdVblendvpd,                        // [ANY] {AVX}
    kIdVblendvps,                        // [ANY] {AVX}
    kIdVbroadcastf128,                   // [ANY] {AVX}
    kIdVbroadcastf32x2,                  // [ANY] {AVX512_DQ (VL)}
    kIdVbroadcastf32x4,                  // [ANY] {AVX512_F}
    kIdVbroadcastf32x8,                  // [ANY] {AVX512_DQ}
    kIdVbroadcastf64x2,                  // [ANY] {AVX512_DQ (VL)}
    kIdVbroadcastf64x4,                  // [ANY] {AVX512_F}
    kIdVbroadcasti128,                   // [ANY] {AVX2}
    kIdVbroadcasti32x2,                  // [ANY] {AVX512_DQ (VL)}
    kIdVbroadcasti32x4,                  // [ANY] {AVX512_F (VL)}
    kIdVbroadcasti32x8,                  // [ANY] {AVX512_DQ}
    kIdVbroadcasti64x2,                  // [ANY] {AVX512_DQ (VL)}
    kIdVbroadcasti64x4,                  // [ANY] {AVX512_F}
    kIdVbroadcastsd,                     // [ANY] {AVX|AVX2|AVX512_F (VL)}
    kIdVbroadcastss,                     // [ANY] {AVX|AVX2|AVX512_F (VL)}
    kIdVcmppd,                           // [ANY] {AVX|AVX512_F (VL)}
    kIdVcmpps,                           // [ANY] {AVX|AVX512_F (VL)}
    kIdVcmpsd,                           // [ANY] {AVX|AVX512_F}
    kIdVcmpss,                           // [ANY] {AVX|AVX512_F}
    kIdVcomisd,                          // [ANY] {AVX|AVX512_F}
    kIdVcomiss,                          // [ANY] {AVX|AVX512_F}
    kIdVcompresspd,                      // [ANY] {AVX512_F (VL)}
    kIdVcompressps,                      // [ANY] {AVX512_F (VL)}
    kIdVcvtdq2pd,                        // [ANY] {AVX|AVX512_F (VL)}
    kIdVcvtdq2ps,                        // [ANY] {AVX|AVX512_F (VL)}
    kIdVcvtpd2dq,                        // [ANY] {AVX|AVX512_F (VL)}
    kIdVcvtpd2ps,                        // [ANY] {AVX|AVX512_F (VL)}
    kIdVcvtpd2qq,                        // [ANY] {AVX512_DQ (VL)}
    kIdVcvtpd2udq,                       // [ANY] {AVX512_F (VL)}
    kIdVcvtpd2uqq,                       // [ANY] {AVX512_DQ (VL)}
    kIdVcvtph2ps,                        // [ANY] {AVX512_F|F16C (VL)}
    kIdVcvtps2dq,                        // [ANY] {AVX|AVX512_F (VL)}
    kIdVcvtps2pd,                        // [ANY] {AVX|AVX512_F (VL)}
    kIdVcvtps2ph,                        // [ANY] {AVX512_F|F16C (VL)}
    kIdVcvtps2qq,                        // [ANY] {AVX512_DQ (VL)}
    kIdVcvtps2udq,                       // [ANY] {AVX512_F (VL)}
    kIdVcvtps2uqq,                       // [ANY] {AVX512_DQ (VL)}
    kIdVcvtqq2pd,                        // [ANY] {AVX512_DQ (VL)}
    kIdVcvtqq2ps,                        // [ANY] {AVX512_DQ (VL)}
    kIdVcvtsd2si,                        // [ANY] {AVX|AVX512_F}
    kIdVcvtsd2ss,                        // [ANY] {AVX|AVX512_F}
    kIdVcvtsd2usi,                       // [ANY] {AVX512_F}
    kIdVcvtsi2sd,                        // [ANY] {AVX|AVX512_F}
    kIdVcvtsi2ss,                        // [ANY] {AVX|AVX512_F}
    kIdVcvtss2sd,                        // [ANY] {AVX|AVX512_F}
    kIdVcvtss2si,                        // [ANY] {AVX|AVX512_F}
    kIdVcvtss2usi,                       // [ANY] {AVX512_F}
    kIdVcvttpd2dq,                       // [ANY] {AVX|AVX512_F (VL)}
    kIdVcvttpd2qq,                       // [ANY] {AVX512_F (VL)}
    kIdVcvttpd2udq,                      // [ANY] {AVX512_F (VL)}
    kIdVcvttpd2uqq,                      // [ANY] {AVX512_DQ (VL)}
    kIdVcvttps2dq,                       // [ANY] {AVX|AVX512_F (VL)}
    kIdVcvttps2qq,                       // [ANY] {AVX512_DQ (VL)}
    kIdVcvttps2udq,                      // [ANY] {AVX512_F (VL)}
    kIdVcvttps2uqq,                      // [ANY] {AVX512_DQ (VL)}
    kIdVcvttsd2si,                       // [ANY] {AVX|AVX512_F}
    kIdVcvttsd2usi,                      // [ANY] {AVX512_F}
    kIdVcvttss2si,                       // [ANY] {AVX|AVX512_F}
    kIdVcvttss2usi,                      // [ANY] {AVX512_F}
    kIdVcvtudq2pd,                       // [ANY] {AVX512_F (VL)}
    kIdVcvtudq2ps,                       // [ANY] {AVX512_F (VL)}
    kIdVcvtuqq2pd,                       // [ANY] {AVX512_DQ (VL)}
    kIdVcvtuqq2ps,                       // [ANY] {AVX512_DQ (VL)}
    kIdVcvtusi2sd,                       // [ANY] {AVX512_F}
    kIdVcvtusi2ss,                       // [ANY] {AVX512_F}
    kIdVdbpsadbw,                        // [ANY] {AVX512_BW (VL)}
    kIdVdivpd,                           // [ANY] {AVX|AVX512_F (VL)}
    kIdVdivps,                           // [ANY] {AVX|AVX512_F (VL)}
    kIdVdivsd,                           // [ANY] {AVX|AVX512_F}
    kIdVdivss,                           // [ANY] {AVX|AVX512_F}
    kIdVdppd,                            // [ANY] {AVX}
    kIdVdpps,                            // [ANY] {AVX}
    kIdVexp2pd,                          // [ANY] {AVX512_ERI}
    kIdVexp2ps,                          // [ANY] {AVX512_ERI}
    kIdVexpandpd,                        // [ANY] {AVX512_F (VL)}
    kIdVexpandps,                        // [ANY] {AVX512_F (VL)}
    kIdVextractf128,                     // [ANY] {AVX}
    kIdVextractf32x4,                    // [ANY] {AVX512_F (VL)}
    kIdVextractf32x8,                    // [ANY] {AVX512_DQ}
    kIdVextractf64x2,                    // [ANY] {AVX512_DQ (VL)}
    kIdVextractf64x4,                    // [ANY] {AVX512_F}
    kIdVextracti128,                     // [ANY] {AVX2}
    kIdVextracti32x4,                    // [ANY] {AVX512_F (VL)}
    kIdVextracti32x8,                    // [ANY] {AVX512_DQ}
    kIdVextracti64x2,                    // [ANY] {AVX512_DQ (VL)}
    kIdVextracti64x4,                    // [ANY] {AVX512_F}
    kIdVextractps,                       // [ANY] {AVX|AVX512_F}
    kIdVfixupimmpd,                      // [ANY] {AVX512_F (VL)}
    kIdVfixupimmps,                      // [ANY] {AVX512_F (VL)}
    kIdVfixupimmsd,                      // [ANY] {AVX512_F}
    kIdVfixupimmss,                      // [ANY] {AVX512_F}
    kIdVfmadd132pd,                      // [ANY] {AVX512_F|FMA (VL)}
    kIdVfmadd132ps,                      // [ANY] {AVX512_F|FMA (VL)}
    kIdVfmadd132sd,                      // [ANY] {AVX512_F|FMA}
    kIdVfmadd132ss,                      // [ANY] {AVX512_F|FMA}
    kIdVfmadd213pd,                      // [ANY] {AVX512_F|FMA (VL)}
    kIdVfmadd213ps,                      // [ANY] {AVX512_F|FMA (VL)}
    kIdVfmadd213sd,                      // [ANY] {AVX512_F|FMA}
    kIdVfmadd213ss,                      // [ANY] {AVX512_F|FMA}
    kIdVfmadd231pd,                      // [ANY] {AVX512_F|FMA (VL)}
    kIdVfmadd231ps,                      // [ANY] {AVX512_F|FMA (VL)}
    kIdVfmadd231sd,                      // [ANY] {AVX512_F|FMA}
    kIdVfmadd231ss,                      // [ANY] {AVX512_F|FMA}
    kIdVfmaddpd,                         // [ANY] {FMA4}
    kIdVfmaddps,                         // [ANY] {FMA4}
    kIdVfmaddsd,                         // [ANY] {FMA4}
    kIdVfmaddss,                         // [ANY] {FMA4}
    kIdVfmaddsub132pd,                   // [ANY] {AVX512_F|FMA (VL)}
    kIdVfmaddsub132ps,                   // [ANY] {AVX512_F|FMA (VL)}
    kIdVfmaddsub213pd,                   // [ANY] {AVX512_F|FMA (VL)}
    kIdVfmaddsub213ps,                   // [ANY] {AVX512_F|FMA (VL)}
    kIdVfmaddsub231pd,                   // [ANY] {AVX512_F|FMA (VL)}
    kIdVfmaddsub231ps,                   // [ANY] {AVX512_F|FMA (VL)}
    kIdVfmaddsubpd,                      // [ANY] {FMA4}
    kIdVfmaddsubps,                      // [ANY] {FMA4}
    kIdVfmsub132pd,                      // [ANY] {AVX512_F|FMA (VL)}
    kIdVfmsub132ps,                      // [ANY] {AVX512_F|FMA (VL)}
    kIdVfmsub132sd,                      // [ANY] {AVX512_F|FMA}
    kIdVfmsub132ss,                      // [ANY] {AVX512_F|FMA}
    kIdVfmsub213pd,                      // [ANY] {AVX512_F|FMA (VL)}
    kIdVfmsub213ps,                      // [ANY] {AVX512_F|FMA (VL)}
    kIdVfmsub213sd,                      // [ANY] {AVX512_F|FMA}
    kIdVfmsub213ss,                      // [ANY] {AVX512_F|FMA}
    kIdVfmsub231pd,                      // [ANY] {AVX512_F|FMA (VL)}
    kIdVfmsub231ps,                      // [ANY] {AVX512_F|FMA (VL)}
    kIdVfmsub231sd,                      // [ANY] {AVX512_F|FMA}
    kIdVfmsub231ss,                      // [ANY] {AVX512_F|FMA}
    kIdVfmsubadd132pd,                   // [ANY] {AVX512_F|FMA (VL)}
    kIdVfmsubadd132ps,                   // [ANY] {AVX512_F|FMA (VL)}
    kIdVfmsubadd213pd,                   // [ANY] {AVX512_F|FMA (VL)}
    kIdVfmsubadd213ps,                   // [ANY] {AVX512_F|FMA (VL)}
    kIdVfmsubadd231pd,                   // [ANY] {AVX512_F|FMA (VL)}
    kIdVfmsubadd231ps,                   // [ANY] {AVX512_F|FMA (VL)}
    kIdVfmsubaddpd,                      // [ANY] {FMA4}
    kIdVfmsubaddps,                      // [ANY] {FMA4}
    kIdVfmsubpd,                         // [ANY] {FMA4}
    kIdVfmsubps,                         // [ANY] {FMA4}
    kIdVfmsubsd,                         // [ANY] {FMA4}
    kIdVfmsubss,                         // [ANY] {FMA4}
    kIdVfnmadd132pd,                     // [ANY] {AVX512_F|FMA (VL)}
    kIdVfnmadd132ps,                     // [ANY] {AVX512_F|FMA (VL)}
    kIdVfnmadd132sd,                     // [ANY] {AVX512_F|FMA}
    kIdVfnmadd132ss,                     // [ANY] {AVX512_F|FMA}
    kIdVfnmadd213pd,                     // [ANY] {AVX512_F|FMA (VL)}
    kIdVfnmadd213ps,                     // [ANY] {AVX512_F|FMA (VL)}
    kIdVfnmadd213sd,                     // [ANY] {AVX512_F|FMA}
    kIdVfnmadd213ss,                     // [ANY] {AVX512_F|FMA}
    kIdVfnmadd231pd,                     // [ANY] {AVX512_F|FMA (VL)}
    kIdVfnmadd231ps,                     // [ANY] {AVX512_F|FMA (VL)}
    kIdVfnmadd231sd,                     // [ANY] {AVX512_F|FMA}
    kIdVfnmadd231ss,                     // [ANY] {AVX512_F|FMA}
    kIdVfnmaddpd,                        // [ANY] {FMA4}
    kIdVfnmaddps,                        // [ANY] {FMA4}
    kIdVfnmaddsd,                        // [ANY] {FMA4}
    kIdVfnmaddss,                        // [ANY] {FMA4}
    kIdVfnmsub132pd,                     // [ANY] {AVX512_F|FMA (VL)}
    kIdVfnmsub132ps,                     // [ANY] {AVX512_F|FMA (VL)}
    kIdVfnmsub132sd,                     // [ANY] {AVX512_F|FMA}
    kIdVfnmsub132ss,                     // [ANY] {AVX512_F|FMA}
    kIdVfnmsub213pd,                     // [ANY] {AVX512_F|FMA (VL)}
    kIdVfnmsub213ps,                     // [ANY] {AVX512_F|FMA (VL)}
    kIdVfnmsub213sd,                     // [ANY] {AVX512_F|FMA}
    kIdVfnmsub213ss,                     // [ANY] {AVX512_F|FMA}
    kIdVfnmsub231pd,                     // [ANY] {AVX512_F|FMA (VL)}
    kIdVfnmsub231ps,                     // [ANY] {AVX512_F|FMA (VL)}
    kIdVfnmsub231sd,                     // [ANY] {AVX512_F|FMA}
    kIdVfnmsub231ss,                     // [ANY] {AVX512_F|FMA}
    kIdVfnmsubpd,                        // [ANY] {FMA4}
    kIdVfnmsubps,                        // [ANY] {FMA4}
    kIdVfnmsubsd,                        // [ANY] {FMA4}
    kIdVfnmsubss,                        // [ANY] {FMA4}
    kIdVfpclasspd,                       // [ANY] {AVX512_DQ (VL)}
    kIdVfpclassps,                       // [ANY] {AVX512_DQ (VL)}
    kIdVfpclasssd,                       // [ANY] {AVX512_DQ}
    kIdVfpclassss,                       // [ANY] {AVX512_DQ}
    kIdVfrczpd,                          // [ANY] {XOP}
    kIdVfrczps,                          // [ANY] {XOP}
    kIdVfrczsd,                          // [ANY] {XOP}
    kIdVfrczss,                          // [ANY] {XOP}
    kIdVgatherdpd,                       // [ANY] {AVX2|AVX512_F (VL)}
    kIdVgatherdps,                       // [ANY] {AVX2|AVX512_F (VL)}
    kIdVgatherpf0dpd,                    // [ANY] {AVX512_PFI}
    kIdVgatherpf0dps,                    // [ANY] {AVX512_PFI}
    kIdVgatherpf0qpd,                    // [ANY] {AVX512_PFI}
    kIdVgatherpf0qps,                    // [ANY] {AVX512_PFI}
    kIdVgatherpf1dpd,                    // [ANY] {AVX512_PFI}
    kIdVgatherpf1dps,                    // [ANY] {AVX512_PFI}
    kIdVgatherpf1qpd,                    // [ANY] {AVX512_PFI}
    kIdVgatherpf1qps,                    // [ANY] {AVX512_PFI}
    kIdVgatherqpd,                       // [ANY] {AVX2|AVX512_F (VL)}
    kIdVgatherqps,                       // [ANY] {AVX2|AVX512_F (VL)}
    kIdVgetexppd,                        // [ANY] {AVX512_F (VL)}
    kIdVgetexpps,                        // [ANY] {AVX512_F (VL)}
    kIdVgetexpsd,                        // [ANY] {AVX512_F}
    kIdVgetexpss,                        // [ANY] {AVX512_F}
    kIdVgetmantpd,                       // [ANY] {AVX512_F (VL)}
    kIdVgetmantps,                       // [ANY] {AVX512_F (VL)}
    kIdVgetmantsd,                       // [ANY] {AVX512_F}
    kIdVgetmantss,                       // [ANY] {AVX512_F}
    kIdVhaddpd,                          // [ANY] {AVX}
    kIdVhaddps,                          // [ANY] {AVX}
    kIdVhsubpd,                          // [ANY] {AVX}
    kIdVhsubps,                          // [ANY] {AVX}
    kIdVinsertf128,                      // [ANY] {AVX}
    kIdVinsertf32x4,                     // [ANY] {AVX512_F (VL)}
    kIdVinsertf32x8,                     // [ANY] {AVX512_DQ}
    kIdVinsertf64x2,                     // [ANY] {AVX512_DQ (VL)}
    kIdVinsertf64x4,                     // [ANY] {AVX512_F}
    kIdVinserti128,                      // [ANY] {AVX2}
    kIdVinserti32x4,                     // [ANY] {AVX512_F (VL)}
    kIdVinserti32x8,                     // [ANY] {AVX512_DQ}
    kIdVinserti64x2,                     // [ANY] {AVX512_DQ (VL)}
    kIdVinserti64x4,                     // [ANY] {AVX512_F}
    kIdVinsertps,                        // [ANY] {AVX|AVX512_F}
    kIdVlddqu,                           // [ANY] {AVX}
    kIdVldmxcsr,                         // [ANY] {AVX}
    kIdVmaskmovdqu,                      // [ANY] {AVX}
    kIdVmaskmovpd,                       // [ANY] {AVX}
    kIdVmaskmovps,                       // [ANY] {AVX}
    kIdVmaxpd,                           // [ANY] {AVX|AVX512_F (VL)}
    kIdVmaxps,                           // [ANY] {AVX|AVX512_F (VL)}
    kIdVmaxsd,                           // [ANY] {AVX|AVX512_F (VL)}
    kIdVmaxss,                           // [ANY] {AVX|AVX512_F (VL)}
    kIdVminpd,                           // [ANY] {AVX|AVX512_F (VL)}
    kIdVminps,                           // [ANY] {AVX|AVX512_F (VL)}
    kIdVminsd,                           // [ANY] {AVX|AVX512_F (VL)}
    kIdVminss,                           // [ANY] {AVX|AVX512_F (VL)}
    kIdVmovapd,                          // [ANY] {AVX|AVX512_F (VL)}
    kIdVmovaps,                          // [ANY] {AVX|AVX512_F (VL)}
    kIdVmovd,                            // [ANY] {AVX|AVX512_F}
    kIdVmovddup,                         // [ANY] {AVX|AVX512_F (VL)}
    kIdVmovdqa,                          // [ANY] {AVX}
    kIdVmovdqa32,                        // [ANY] {AVX512_F (VL)}
    kIdVmovdqa64,                        // [ANY] {AVX512_F (VL)}
    kIdVmovdqu,                          // [ANY] {AVX}
    kIdVmovdqu16,                        // [ANY] {AVX512_BW (VL)}
    kIdVmovdqu32,                        // [ANY] {AVX512_F (VL)}
    kIdVmovdqu64,                        // [ANY] {AVX512_F (VL)}
    kIdVmovdqu8,                         // [ANY] {AVX512_BW (VL)}
    kIdVmovhlps,                         // [ANY] {AVX|AVX512_F}
    kIdVmovhpd,                          // [ANY] {AVX|AVX512_F}
    kIdVmovhps,                          // [ANY] {AVX|AVX512_F}
    kIdVmovlhps,                         // [ANY] {AVX|AVX512_F}
    kIdVmovlpd,                          // [ANY] {AVX|AVX512_F}
    kIdVmovlps,                          // [ANY] {AVX|AVX512_F}
    kIdVmovmskpd,                        // [ANY] {AVX}
    kIdVmovmskps,                        // [ANY] {AVX}
    kIdVmovntdq,                         // [ANY] {AVX|AVX512_F (VL)}
    kIdVmovntdqa,                        // [ANY] {AVX|AVX2|AVX512_F (VL)}
    kIdVmovntpd,                         // [ANY] {AVX|AVX512_F (VL)}
    kIdVmovntps,                         // [ANY] {AVX|AVX512_F (VL)}
    kIdVmovq,                            // [ANY] {AVX|AVX512_F}
    kIdVmovsd,                           // [ANY] {AVX|AVX512_F}
    kIdVmovshdup,                        // [ANY] {AVX|AVX512_F (VL)}
    kIdVmovsldup,                        // [ANY] {AVX|AVX512_F (VL)}
    kIdVmovss,                           // [ANY] {AVX|AVX512_F}
    kIdVmovupd,                          // [ANY] {AVX|AVX512_F (VL)}
    kIdVmovups,                          // [ANY] {AVX|AVX512_F (VL)}
    kIdVmpsadbw,                         // [ANY] {AVX|AVX2}
    kIdVmulpd,                           // [ANY] {AVX|AVX2|AVX512_F (VL)}
    kIdVmulps,                           // [ANY] {AVX|AVX2|AVX512_F (VL)}
    kIdVmulsd,                           // [ANY] {AVX|AVX512_F}
    kIdVmulss,                           // [ANY] {AVX|AVX512_F}
    kIdVorpd,                            // [ANY] {AVX|AVX512_DQ (VL)}
    kIdVorps,                            // [ANY] {AVX|AVX512_F (VL)}
    kIdVpabsb,                           // [ANY] {AVX|AVX2|AVX512_BW (VL)}
    kIdVpabsd,                           // [ANY] {AVX|AVX2|AVX512_F (VL)}
    kIdVpabsq,                           // [ANY] {AVX512_F (VL)}
    kIdVpabsw,                           // [ANY] {AVX|AVX2|AVX512_BW (VL)}
    kIdVpackssdw,                        // [ANY] {AVX|AVX2|AVX512_BW (VL)}
    kIdVpacksswb,                        // [ANY] {AVX|AVX2|AVX512_BW (VL)}
    kIdVpackusdw,                        // [ANY] {AVX|AVX2|AVX512_BW (VL)}
    kIdVpackuswb,                        // [ANY] {AVX|AVX2|AVX512_BW (VL)}
    kIdVpaddb,                           // [ANY] {AVX|AVX2|AVX512_BW (VL)}
    kIdVpaddd,                           // [ANY] {AVX|AVX2|AVX512_F (VL)}
    kIdVpaddq,                           // [ANY] {AVX|AVX2|AVX512_F (VL)}
    kIdVpaddsb,                          // [ANY] {AVX|AVX2|AVX512_BW (VL)}
    kIdVpaddsw,                          // [ANY] {AVX|AVX2|AVX512_BW (VL)}
    kIdVpaddusb,                         // [ANY] {AVX|AVX2|AVX512_BW (VL)}
    kIdVpaddusw,                         // [ANY] {AVX|AVX2|AVX512_BW (VL)}
    kIdVpaddw,                           // [ANY] {AVX|AVX2|AVX512_BW (VL)}
    kIdVpalignr,                         // [ANY] {AVX|AVX2|AVX512_BW (VL)}
    kIdVpand,                            // [ANY] {AVX|AVX2}
    kIdVpandd,                           // [ANY] {AVX512_F (VL)}
    kIdVpandn,                           // [ANY] {AVX|AVX2}
    kIdVpandnd,                          // [ANY] {AVX512_F (VL)}
    kIdVpandnq,                          // [ANY] {AVX512_F (VL)}
    kIdVpandq,                           // [ANY] {AVX512_F (VL)}
    kIdVpavgb,                           // [ANY] {AVX|AVX512_BW (VL)}
    kIdVpavgw,                           // [ANY] {AVX2|AVX512_BW (VL)}
    kIdVpblendd,                         // [ANY] {AVX2}
    kIdVpblendvb,                        // [ANY] {AVX|AVX2}
    kIdVpblendw,                         // [ANY] {AVX|AVX2}
    kIdVpbroadcastb,                     // [ANY] {AVX2|AVX512_BW (VL)}
    kIdVpbroadcastd,                     // [ANY] {AVX2|AVX512_F (VL)}
    kIdVpbroadcastmb2d,                  // [ANY] {AVX512_CDI (VL)}
    kIdVpbroadcastmb2q,                  // [ANY] {AVX512_CDI (VL)}
    kIdVpbroadcastq,                     // [ANY] {AVX2|AVX512_F (VL)}
    kIdVpbroadcastw,                     // [ANY] {AVX2|AVX512_BW (VL)}
    kIdVpclmulqdq,                       // [ANY] {AVX|PCLMULQDQ}
    kIdVpcmov,                           // [ANY] {XOP}
    kIdVpcmpb,                           // [ANY] {AVX512_BW (VL)}
    kIdVpcmpd,                           // [ANY] {AVX512_F (VL)}
    kIdVpcmpeqb,                         // [ANY] {AVX|AVX2|AVX512_BW (VL)}
    kIdVpcmpeqd,                         // [ANY] {AVX|AVX2|AVX512_F (VL)}
    kIdVpcmpeqq,                         // [ANY] {AVX|AVX2|AVX512_F (VL)}
    kIdVpcmpeqw,                         // [ANY] {AVX|AVX2|AVX512_BW (VL)}
    kIdVpcmpestri,                       // [ANY] {AVX}
    kIdVpcmpestrm,                       // [ANY] {AVX}
    kIdVpcmpgtb,                         // [ANY] {AVX|AVX2|AVX512_BW (VL)}
    kIdVpcmpgtd,                         // [ANY] {AVX|AVX2|AVX512_F (VL)}
    kIdVpcmpgtq,                         // [ANY] {AVX|AVX2|AVX512_F (VL)}
    kIdVpcmpgtw,                         // [ANY] {AVX|AVX2|AVX512_BW (VL)}
    kIdVpcmpistri,                       // [ANY] {AVX}
    kIdVpcmpistrm,                       // [ANY] {AVX}
    kIdVpcmpq,                           // [ANY] {AVX512_F (VL)}
    kIdVpcmpub,                          // [ANY] {AVX512_BW (VL)}
    kIdVpcmpud,                          // [ANY] {AVX512_F (VL)}
    kIdVpcmpuq,                          // [ANY] {AVX512_F (VL)}
    kIdVpcmpuw,                          // [ANY] {AVX512_BW (VL)}
    kIdVpcmpw,                           // [ANY] {AVX512_BW (VL)}
    kIdVpcomb,                           // [ANY] {XOP}
    kIdVpcomd,                           // [ANY] {XOP}
    kIdVpcompressd,                      // [ANY] {AVX512_F (VL)}
    kIdVpcompressq,                      // [ANY] {AVX512_F (VL)}
    kIdVpcomq,                           // [ANY] {XOP}
    kIdVpcomub,                          // [ANY] {XOP}
    kIdVpcomud,                          // [ANY] {XOP}
    kIdVpcomuq,                          // [ANY] {XOP}
    kIdVpcomuw,                          // [ANY] {XOP}
    kIdVpcomw,                           // [ANY] {XOP}
    kIdVpconflictd,                      // [ANY] {AVX512_CDI (VL)}
    kIdVpconflictq,                      // [ANY] {AVX512_CDI (VL)}
    kIdVperm2f128,                       // [ANY] {AVX}
    kIdVperm2i128,                       // [ANY] {AVX2}
    kIdVpermb,                           // [ANY] {AVX512_VBMI (VL)}
    kIdVpermd,                           // [ANY] {AVX2|AVX512_F (VL)}
    kIdVpermi2b,                         // [ANY] {AVX512_VBMI (VL)}
    kIdVpermi2d,                         // [ANY] {AVX512_F (VL)}
    kIdVpermi2pd,                        // [ANY] {AVX512_F (VL)}
    kIdVpermi2ps,                        // [ANY] {AVX512_F (VL)}
    kIdVpermi2q,                         // [ANY] {AVX512_F (VL)}
    kIdVpermi2w,                         // [ANY] {AVX512_BW (VL)}
    kIdVpermil2pd,                       // [ANY] {XOP}
    kIdVpermil2ps,                       // [ANY] {XOP}
    kIdVpermilpd,                        // [ANY] {AVX|AVX512_F (VL)}
    kIdVpermilps,                        // [ANY] {AVX|AVX512_F (VL)}
    kIdVpermpd,                          // [ANY] {AVX2}
    kIdVpermps,                          // [ANY] {AVX2}
    kIdVpermq,                           // [ANY] {AVX2|AVX512_F (VL)}
    kIdVpermt2b,                         // [ANY] {AVX512_VBMI (VL)}
    kIdVpermt2d,                         // [ANY] {AVX512_F (VL)}
    kIdVpermt2pd,                        // [ANY] {AVX512_F (VL)}
    kIdVpermt2ps,                        // [ANY] {AVX512_F (VL)}
    kIdVpermt2q,                         // [ANY] {AVX512_F (VL)}
    kIdVpermt2w,                         // [ANY] {AVX512_BW (VL)}
    kIdVpermw,                           // [ANY] {AVX512_BW (VL)}
    kIdVpexpandd,                        // [ANY] {AVX512_F (VL)}
    kIdVpexpandq,                        // [ANY] {AVX512_F (VL)}
    kIdVpextrb,                          // [ANY] {AVX|AVX512_BW}
    kIdVpextrd,                          // [ANY] {AVX|AVX512_DQ}
    kIdVpextrq,                          // [X64] {AVX|AVX512_DQ}
    kIdVpextrw,                          // [ANY] {AVX|AVX512_BW}
    kIdVpgatherdd,                       // [ANY] {AVX2|AVX512_F (VL)}
    kIdVpgatherdq,                       // [ANY] {AVX2|AVX512_F (VL)}
    kIdVpgatherqd,                       // [ANY] {AVX2|AVX512_F (VL)}
    kIdVpgatherqq,                       // [ANY] {AVX2|AVX512_F (VL)}
    kIdVphaddbd,                         // [ANY] {XOP}
    kIdVphaddbq,                         // [ANY] {XOP}
    kIdVphaddbw,                         // [ANY] {XOP}
    kIdVphaddd,                          // [ANY] {AVX|AVX2}
    kIdVphadddq,                         // [ANY] {XOP}
    kIdVphaddsw,                         // [ANY] {AVX|AVX2}
    kIdVphaddubd,                        // [ANY] {XOP}
    kIdVphaddubq,                        // [ANY] {XOP}
    kIdVphaddubw,                        // [ANY] {XOP}
    kIdVphaddudq,                        // [ANY] {XOP}
    kIdVphadduwd,                        // [ANY] {XOP}
    kIdVphadduwq,                        // [ANY] {XOP}
    kIdVphaddw,                          // [ANY] {AVX|AVX2}
    kIdVphaddwd,                         // [ANY] {XOP}
    kIdVphaddwq,                         // [ANY] {XOP}
    kIdVphminposuw,                      // [ANY] {AVX}
    kIdVphsubbw,                         // [ANY] {XOP}
    kIdVphsubd,                          // [ANY] {AVX|AVX2}
    kIdVphsubdq,                         // [ANY] {XOP}
    kIdVphsubsw,                         // [ANY] {AVX|AVX2}
    kIdVphsubw,                          // [ANY] {AVX|AVX2}
    kIdVphsubwd,                         // [ANY] {XOP}
    kIdVpinsrb,                          // [ANY] {AVX|AVX512_BW}
    kIdVpinsrd,                          // [ANY] {AVX|AVX512_DQ}
    kIdVpinsrq,                          // [X64] {AVX|AVX512_DQ}
    kIdVpinsrw,                          // [ANY] {AVX|AVX512_BW}
    kIdVplzcntd,                         // [ANY] {AVX512_CDI (VL)}
    kIdVplzcntq,                         // [ANY] {AVX512_CDI (VL)}
    kIdVpmacsdd,                         // [ANY] {XOP}
    kIdVpmacsdqh,                        // [ANY] {XOP}
    kIdVpmacsdql,                        // [ANY] {XOP}
    kIdVpmacssdd,                        // [ANY] {XOP}
    kIdVpmacssdqh,                       // [ANY] {XOP}
    kIdVpmacssdql,                       // [ANY] {XOP}
    kIdVpmacsswd,                        // [ANY] {XOP}
    kIdVpmacssww,                        // [ANY] {XOP}
    kIdVpmacswd,                         // [ANY] {XOP}
    kIdVpmacsww,                         // [ANY] {XOP}
    kIdVpmadcsswd,                       // [ANY] {XOP}
    kIdVpmadcswd,                        // [ANY] {XOP}
    kIdVpmadd52huq,                      // [ANY] {AVX512_IFMA (VL)}
    kIdVpmadd52luq,                      // [ANY] {AVX512_IFMA (VL)}
    kIdVpmaddubsw,                       // [ANY] {AVX|AVX2|AVX512_BW (VL)}
    kIdVpmaddwd,                         // [ANY] {AVX|AVX2|AVX512_BW (VL)}
    kIdVpmaskmovd,                       // [ANY] {AVX2}
    kIdVpmaskmovq,                       // [ANY] {AVX2}
    kIdVpmaxsb,                          // [ANY] {AVX|AVX2|AVX512_BW (VL)}
    kIdVpmaxsd,                          // [ANY] {AVX|AVX2|AVX512_F (VL)}
    kIdVpmaxsq,                          // [ANY] {AVX512_F (VL)}
    kIdVpmaxsw,                          // [ANY] {AVX|AVX2|AVX512_BW (VL)}
    kIdVpmaxub,                          // [ANY] {AVX|AVX2|AVX512_BW (VL)}
    kIdVpmaxud,                          // [ANY] {AVX|AVX2|AVX512_F (VL)}
    kIdVpmaxuq,                          // [ANY] {AVX512_F (VL)}
    kIdVpmaxuw,                          // [ANY] {AVX|AVX2|AVX512_BW (VL)}
    kIdVpminsb,                          // [ANY] {AVX|AVX2|AVX512_BW (VL)}
    kIdVpminsd,                          // [ANY] {AVX|AVX2|AVX512_F (VL)}
    kIdVpminsq,                          // [ANY] {AVX512_F (VL)}
    kIdVpminsw,                          // [ANY] {AVX|AVX2|AVX512_BW (VL)}
    kIdVpminub,                          // [ANY] {AVX|AVX2|AVX512_BW (VL)}
    kIdVpminud,                          // [ANY] {AVX|AVX2|AVX512_F (VL)}
    kIdVpminuq,                          // [ANY] {AVX512_F (VL)}
    kIdVpminuw,                          // [ANY] {AVX|AVX2|AVX512_BW (VL)}
    kIdVpmovb2m,                         // [ANY] {AVX512_BW (VL)}
    kIdVpmovd2m,                         // [ANY] {AVX512_DQ (VL)}
    kIdVpmovdb,                          // [ANY] {AVX512_F (VL)}
    kIdVpmovdw,                          // [ANY] {AVX512_F (VL)}
    kIdVpmovm2b,                         // [ANY] {AVX512_BW (VL)}
    kIdVpmovm2d,                         // [ANY] {AVX512_DQ (VL)}
    kIdVpmovm2q,                         // [ANY] {AVX512_DQ (VL)}
    kIdVpmovm2w,                         // [ANY] {AVX512_BW (VL)}
    kIdVpmovmskb,                        // [ANY] {AVX|AVX2}
    kIdVpmovq2m,                         // [ANY] {AVX512_DQ (VL)}
    kIdVpmovqb,                          // [ANY] {AVX512_F (VL)}
    kIdVpmovqd,                          // [ANY] {AVX512_F (VL)}
    kIdVpmovqw,                          // [ANY] {AVX512_F (VL)}
    kIdVpmovsdb,                         // [ANY] {AVX512_F (VL)}
    kIdVpmovsdw,                         // [ANY] {AVX512_F (VL)}
    kIdVpmovsqb,                         // [ANY] {AVX512_F (VL)}
    kIdVpmovsqd,                         // [ANY] {AVX512_F (VL)}
    kIdVpmovsqw,                         // [ANY] {AVX512_F (VL)}
    kIdVpmovswb,                         // [ANY] {AVX512_BW (VL)}
    kIdVpmovsxbd,                        // [ANY] {AVX|AVX2|AVX512_F (VL)}
    kIdVpmovsxbq,                        // [ANY] {AVX|AVX2|AVX512_F (VL)}
    kIdVpmovsxbw,                        // [ANY] {AVX|AVX2|AVX512_BW (VL)}
    kIdVpmovsxdq,                        // [ANY] {AVX|AVX2|AVX512_F (VL)}
    kIdVpmovsxwd,                        // [ANY] {AVX|AVX2|AVX512_F (VL)}
    kIdVpmovsxwq,                        // [ANY] {AVX|AVX2|AVX512_F (VL)}
    kIdVpmovusdb,                        // [ANY] {AVX512_F (VL)}
    kIdVpmovusdw,                        // [ANY] {AVX512_F (VL)}
    kIdVpmovusqb,                        // [ANY] {AVX512_F (VL)}
    kIdVpmovusqd,                        // [ANY] {AVX512_F (VL)}
    kIdVpmovusqw,                        // [ANY] {AVX512_F (VL)}
    kIdVpmovuswb,                        // [ANY] {AVX512_BW (VL)}
    kIdVpmovw2m,                         // [ANY] {AVX512_BW (VL)}
    kIdVpmovwb,                          // [ANY] {AVX512_BW (VL)}
    kIdVpmovzxbd,                        // [ANY] {AVX|AVX2|AVX512_F (VL)}
    kIdVpmovzxbq,                        // [ANY] {AVX|AVX2|AVX512_F (VL)}
    kIdVpmovzxbw,                        // [ANY] {AVX|AVX2|AVX512_BW (VL)}
    kIdVpmovzxdq,                        // [ANY] {AVX|AVX2|AVX512_F (VL)}
    kIdVpmovzxwd,                        // [ANY] {AVX|AVX2|AVX512_F (VL)}
    kIdVpmovzxwq,                        // [ANY] {AVX|AVX2|AVX512_F (VL)}
    kIdVpmuldq,                          // [ANY] {AVX|AVX2|AVX512_F (VL)}
    kIdVpmulhrsw,                        // [ANY] {AVX|AVX2|AVX512_BW (VL)}
    kIdVpmulhuw,                         // [ANY] {AVX|AVX2|AVX512_BW (VL)}
    kIdVpmulhw,                          // [ANY] {AVX|AVX2|AVX512_BW (VL)}
    kIdVpmulld,                          // [ANY] {AVX|AVX2|AVX512_F (VL)}
    kIdVpmullq,                          // [ANY] {AVX512_DQ (VL)}
    kIdVpmullw,                          // [ANY] {AVX|AVX2|AVX512_BW (VL)}
    kIdVpmultishiftqb,                   // [ANY] {AVX512_VBMI (VL)}
    kIdVpmuludq,                         // [ANY] {AVX|AVX2|AVX512_F (VL)}
    kIdVpor,                             // [ANY] {AVX|AVX2}
    kIdVpord,                            // [ANY] {AVX512_F (VL)}
    kIdVporq,                            // [ANY] {AVX512_F (VL)}
    kIdVpperm,                           // [ANY] {XOP}
    kIdVprold,                           // [ANY] {AVX512_F (VL)}
    kIdVprolq,                           // [ANY] {AVX512_F (VL)}
    kIdVprolvd,                          // [ANY] {AVX512_F (VL)}
    kIdVprolvq,                          // [ANY] {AVX512_F (VL)}
    kIdVprord,                           // [ANY] {AVX512_F (VL)}
    kIdVprorq,                           // [ANY] {AVX512_F (VL)}
    kIdVprorvd,                          // [ANY] {AVX512_F (VL)}
    kIdVprorvq,                          // [ANY] {AVX512_F (VL)}
    kIdVprotb,                           // [ANY] {XOP}
    kIdVprotd,                           // [ANY] {XOP}
    kIdVprotq,                           // [ANY] {XOP}
    kIdVprotw,                           // [ANY] {XOP}
    kIdVpsadbw,                          // [ANY] {AVX|AVX2|AVX512_BW (VL)}
    kIdVpscatterdd,                      // [ANY] {AVX512_F (VL)}
    kIdVpscatterdq,                      // [ANY] {AVX512_F (VL)}
    kIdVpscatterqd,                      // [ANY] {AVX512_F (VL)}
    kIdVpscatterqq,                      // [ANY] {AVX512_F (VL)}
    kIdVpshab,                           // [ANY] {XOP}
    kIdVpshad,                           // [ANY] {XOP}
    kIdVpshaq,                           // [ANY] {XOP}
    kIdVpshaw,                           // [ANY] {XOP}
    kIdVpshlb,                           // [ANY] {XOP}
    kIdVpshld,                           // [ANY] {XOP}
    kIdVpshlq,                           // [ANY] {XOP}
    kIdVpshlw,                           // [ANY] {XOP}
    kIdVpshufb,                          // [ANY] {AVX|AVX2|AVX512_BW (VL)}
    kIdVpshufd,                          // [ANY] {AVX|AVX2|AVX512_F (VL)}
    kIdVpshufhw,                         // [ANY] {AVX|AVX2|AVX512_BW (VL)}
    kIdVpshuflw,                         // [ANY] {AVX|AVX2|AVX512_BW (VL)}
    kIdVpsignb,                          // [ANY] {AVX|AVX2}
    kIdVpsignd,                          // [ANY] {AVX|AVX2}
    kIdVpsignw,                          // [ANY] {AVX|AVX2}
    kIdVpslld,                           // [ANY] {AVX|AVX2|AVX512_F (VL)}
    kIdVpslldq,                          // [ANY] {AVX|AVX2|AVX512_BW (VL)}
    kIdVpsllq,                           // [ANY] {AVX|AVX2|AVX512_F (VL)}
    kIdVpsllvd,                          // [ANY] {AVX2|AVX512_F (VL)}
    kIdVpsllvq,                          // [ANY] {AVX2|AVX512_F (VL)}
    kIdVpsllvw,                          // [ANY] {AVX512_BW (VL)}
    kIdVpsllw,                           // [ANY] {AVX|AVX2|AVX512_BW (VL)}
    kIdVpsrad,                           // [ANY] {AVX|AVX2|AVX512_F (VL)}
    kIdVpsraq,                           // [ANY] {AVX512_F (VL)}
    kIdVpsravd,                          // [ANY] {AVX2|AVX512_F (VL)}
    kIdVpsravq,                          // [ANY] {AVX512_F (VL)}
    kIdVpsravw,                          // [ANY] {AVX512_BW (VL)}
    kIdVpsraw,                           // [ANY] {AVX|AVX2|AVX512_BW (VL)}
    kIdVpsrld,                           // [ANY] {AVX|AVX2|AVX512_F (VL)}
    kIdVpsrldq,                          // [ANY] {AVX|AVX2|AVX512_BW (VL)}
    kIdVpsrlq,                           // [ANY] {AVX|AVX2|AVX512_F (VL)}
    kIdVpsrlvd,                          // [ANY] {AVX2|AVX512_F (VL)}
    kIdVpsrlvq,                          // [ANY] {AVX2|AVX512_F (VL)}
    kIdVpsrlvw,                          // [ANY] {AVX512_BW (VL)}
    kIdVpsrlw,                           // [ANY] {AVX|AVX2|AVX512_BW (VL)}
    kIdVpsubb,                           // [ANY] {AVX|AVX2|AVX512_BW (VL)}
    kIdVpsubd,                           // [ANY] {AVX|AVX2|AVX512_F (VL)}
    kIdVpsubq,                           // [ANY] {AVX|AVX2|AVX512_F (VL)}
    kIdVpsubsb,                          // [ANY] {AVX|AVX2|AVX512_BW (VL)}
    kIdVpsubsw,                          // [ANY] {AVX|AVX2|AVX512_BW (VL)}
    kIdVpsubusb,                         // [ANY] {AVX|AVX2|AVX512_BW (VL)}
    kIdVpsubusw,                         // [ANY] {AVX|AVX2|AVX512_BW (VL)}
    kIdVpsubw,                           // [ANY] {AVX|AVX2|AVX512_BW (VL)}
    kIdVpternlogd,                       // [ANY] {AVX512_F (VL)}
    kIdVpternlogq,                       // [ANY] {AVX512_F (VL)}
    kIdVptest,                           // [ANY] {AVX}
    kIdVptestmb,                         // [ANY] {AVX512_BW (VL)}
    kIdVptestmd,                         // [ANY] {AVX512_F (VL)}
    kIdVptestmq,                         // [ANY] {AVX512_F (VL)}
    kIdVptestmw,                         // [ANY] {AVX512_BW (VL)}
    kIdVptestnmb,                        // [ANY] {AVX512_BW (VL)}
    kIdVptestnmd,                        // [ANY] {AVX512_F (VL)}
    kIdVptestnmq,                        // [ANY] {AVX512_F (VL)}
    kIdVptestnmw,                        // [ANY] {AVX512_BW (VL)}
    kIdVpunpckhbw,                       // [ANY] {AVX|AVX2|AVX512_BW (VL)}
    kIdVpunpckhdq,                       // [ANY] {AVX|AVX2|AVX512_F (VL)}
    kIdVpunpckhqdq,                      // [ANY] {AVX|AVX2|AVX512_F (VL)}
    kIdVpunpckhwd,                       // [ANY] {AVX|AVX2|AVX512_BW (VL)}
    kIdVpunpcklbw,                       // [ANY] {AVX|AVX2|AVX512_BW (VL)}
    kIdVpunpckldq,                       // [ANY] {AVX|AVX2|AVX512_F (VL)}
    kIdVpunpcklqdq,                      // [ANY] {AVX|AVX2|AVX512_F (VL)}
    kIdVpunpcklwd,                       // [ANY] {AVX|AVX2|AVX512_BW (VL)}
    kIdVpxor,                            // [ANY] {AVX|AVX2}
    kIdVpxord,                           // [ANY] {AVX512_F (VL)}
    kIdVpxorq,                           // [ANY] {AVX512_F (VL)}
    kIdVrangepd,                         // [ANY] {AVX512_DQ (VL)}
    kIdVrangeps,                         // [ANY] {AVX512_DQ (VL)}
    kIdVrangesd,                         // [ANY] {AVX512_DQ}
    kIdVrangess,                         // [ANY] {AVX512_DQ}
    kIdVrcp14pd,                         // [ANY] {AVX512_F (VL)}
    kIdVrcp14ps,                         // [ANY] {AVX512_F (VL)}
    kIdVrcp14sd,                         // [ANY] {AVX512_F}
    kIdVrcp14ss,                         // [ANY] {AVX512_F}
    kIdVrcp28pd,                         // [ANY] {AVX512_ERI}
    kIdVrcp28ps,                         // [ANY] {AVX512_ERI}
    kIdVrcp28sd,                         // [ANY] {AVX512_ERI}
    kIdVrcp28ss,                         // [ANY] {AVX512_ERI}
    kIdVrcpps,                           // [ANY] {AVX}
    kIdVrcpss,                           // [ANY] {AVX}
    kIdVreducepd,                        // [ANY] {AVX512_DQ (VL)}
    kIdVreduceps,                        // [ANY] {AVX512_DQ (VL)}
    kIdVreducesd,                        // [ANY] {AVX512_DQ}
    kIdVreducess,                        // [ANY] {AVX512_DQ}
    kIdVrndscalepd,                      // [ANY] {AVX512_F (VL)}
    kIdVrndscaleps,                      // [ANY] {AVX512_F (VL)}
    kIdVrndscalesd,                      // [ANY] {AVX512_F}
    kIdVrndscaless,                      // [ANY] {AVX512_F}
    kIdVroundpd,                         // [ANY] {AVX}
    kIdVroundps,                         // [ANY] {AVX}
    kIdVroundsd,                         // [ANY] {AVX}
    kIdVroundss,                         // [ANY] {AVX}
    kIdVrsqrt14pd,                       // [ANY] {AVX512_F (VL)}
    kIdVrsqrt14ps,                       // [ANY] {AVX512_F (VL)}
    kIdVrsqrt14sd,                       // [ANY] {AVX512_F}
    kIdVrsqrt14ss,                       // [ANY] {AVX512_F}
    kIdVrsqrt28pd,                       // [ANY] {AVX512_ERI}
    kIdVrsqrt28ps,                       // [ANY] {AVX512_ERI}
    kIdVrsqrt28sd,                       // [ANY] {AVX512_ERI}
    kIdVrsqrt28ss,                       // [ANY] {AVX512_ERI}
    kIdVrsqrtps,                         // [ANY] {AVX}
    kIdVrsqrtss,                         // [ANY] {AVX}
    kIdVscalefpd,                        // [ANY] {AVX512_F (VL)}
    kIdVscalefps,                        // [ANY] {AVX512_F (VL)}
    kIdVscalefsd,                        // [ANY] {AVX512_F}
    kIdVscalefss,                        // [ANY] {AVX512_F}
    kIdVscatterdpd,                      // [ANY] {AVX512_F (VL)}
    kIdVscatterdps,                      // [ANY] {AVX512_F (VL)}
    kIdVscatterpf0dpd,                   // [ANY] {AVX512_PFI}
    kIdVscatterpf0dps,                   // [ANY] {AVX512_PFI}
    kIdVscatterpf0qpd,                   // [ANY] {AVX512_PFI}
    kIdVscatterpf0qps,                   // [ANY] {AVX512_PFI}
    kIdVscatterpf1dpd,                   // [ANY] {AVX512_PFI}
    kIdVscatterpf1dps,                   // [ANY] {AVX512_PFI}
    kIdVscatterpf1qpd,                   // [ANY] {AVX512_PFI}
    kIdVscatterpf1qps,                   // [ANY] {AVX512_PFI}
    kIdVscatterqpd,                      // [ANY] {AVX512_F (VL)}
    kIdVscatterqps,                      // [ANY] {AVX512_F (VL)}
    kIdVshuff32x4,                       // [ANY] {AVX512_F (VL)}
    kIdVshuff64x2,                       // [ANY] {AVX512_F (VL)}
    kIdVshufi32x4,                       // [ANY] {AVX512_F (VL)}
    kIdVshufi64x2,                       // [ANY] {AVX512_F (VL)}
    kIdVshufpd,                          // [ANY] {AVX|AVX512_F (VL)}
    kIdVshufps,                          // [ANY] {AVX|AVX512_F (VL)}
    kIdVsqrtpd,                          // [ANY] {AVX|AVX512_F (VL)}
    kIdVsqrtps,                          // [ANY] {AVX|AVX512_F (VL)}
    kIdVsqrtsd,                          // [ANY] {AVX|AVX512_F}
    kIdVsqrtss,                          // [ANY] {AVX|AVX512_F}
    kIdVstmxcsr,                         // [ANY] {AVX}
    kIdVsubpd,                           // [ANY] {AVX|AVX2|AVX512_F (VL)}
    kIdVsubps,                           // [ANY] {AVX|AVX2|AVX512_F (VL)}
    kIdVsubsd,                           // [ANY] {AVX|AVX512_F}
    kIdVsubss,                           // [ANY] {AVX|AVX512_F}
    kIdVtestpd,                          // [ANY] {AVX}
    kIdVtestps,                          // [ANY] {AVX}
    kIdVucomisd,                         // [ANY] {AVX|AVX512_F}
    kIdVucomiss,                         // [ANY] {AVX|AVX512_F}
    kIdVunpckhpd,                        // [ANY] {AVX|AVX512_F (VL)}
    kIdVunpckhps,                        // [ANY] {AVX|AVX512_F (VL)}
    kIdVunpcklpd,                        // [ANY] {AVX|AVX512_F (VL)}
    kIdVunpcklps,                        // [ANY] {AVX|AVX512_F (VL)}
    kIdVxorpd,                           // [ANY] {AVX|AVX512_DQ (VL)}
    kIdVxorps,                           // [ANY] {AVX|AVX512_DQ (VL)}
    kIdVzeroall,                         // [ANY] {AVX}
    kIdVzeroupper,                       // [ANY] {AVX}
    kIdWrfsbase,                         // [X64] {FSGSBASE}
    kIdWrgsbase,                         // [X64] {FSGSBASE}
    kIdXadd,                             // [ANY] {I486}
    kIdXchg,                             // [ANY]
    kIdXgetbv,                           // [ANY] {XSAVE}
    kIdXor,                              // [ANY]
    kIdXorpd,                            // [ANY] {SSE2}
    kIdXorps,                            // [ANY] {SSE}
    kIdXrstor,                           // [ANY] {XSAVE}
    kIdXrstor64,                         // [X64] {XSAVE}
    kIdXrstors,                          // [ANY] {XSAVE}
    kIdXrstors64,                        // [X64] {XSAVE}
    kIdXsave,                            // [ANY] {XSAVE}
    kIdXsave64,                          // [X64] {XSAVE}
    kIdXsavec,                           // [ANY] {XSAVE}
    kIdXsavec64,                         // [X64] {XSAVE}
    kIdXsaveopt,                         // [ANY] {XSAVE_OPT}
    kIdXsaveopt64,                       // [X64] {XSAVE_OPT}
    kIdXsaves,                           // [ANY] {XSAVE}
    kIdXsaves64,                         // [X64] {XSAVE}
    kIdXsetbv,                           // [ANY] {XSAVE}
    _kIdCount
    // ${idData:End}
  };

  //! Instruction encodings, used by \ref X86Assembler (AsmJit specific).
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
  //! Instruction flags (AsmJit specific).
  ASMJIT_ENUM(InstFlags) {
    kInstFlagNone         = 0x00000000U, //!< No flags.

    kInstFlagRO           = 0x00000001U, //!< The first operand is read (read-only without `kInstFlagWO`).
    kInstFlagWO           = 0x00000002U, //!< The first operand is written (write-only without `kInstFlagRO`).
    kInstFlagRW           = 0x00000003U, //!< The first operand is read-write.
    kInstFlagXchg         = 0x00000004U, //!< Instruction is an exchange like instruction (xchg, xadd).

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
    kInstFlagEvex         = 0x00040000U  //!< Instruction can be encoded by EVEX (AVX-512).
  };

  //! Describes a meaning of all bits of AsmJit's 32-bit opcode (AsmJit specific).
  //!
  //! This schema is AsmJit specific and has been designed to allow encoding of
  //! all X86 instructions available. X86, MMX, and SSE+ instructions always use
  //! `MM` and `PP` fields, which are encoded to corresponding prefixes needed
  //! by X86 or SIMD instructions. AVX+ instructions embed `MMMMM` and `PP` fields
  //! in a VEX prefix, and AVX-512 instructions embed `MM` and `PP` in EVEX prefix.
  //!
  //! The instruction opcode definition uses 1 or 2 bytes as an opcode value. 1
  //! byte is needed by most of the instructions, 2 bytes are only used by legacy
  //! X87-FPU instructions. This means that a second byte is free to by used by
  //! instructions encoded by using VEX and/or EVEX prefix.
  //!
  //! The fields description:
  //!
  //! - `MM` field is used to encode prefixes needed by the instruction or as
  //!   a part of VEX/EVEX prefix. Described as `mm` and `mmmmm` in instruction
  //!   manuals.
  //!
  //!   NOTE: Since `MM` field is defined as `mmmmm` (5 bits), but only 2 least
  //!   significant bits are used by VEX and EVEX prefixes, and additional 4th
  //!   bit is used by XOP prefix, AsmJit uses the 3rd and 5th bit for it's own
  //!   purposes. These bits will probably never be used in future encodings as
  //!   AVX512 uses only `000mm` from `mmmmm`.
  //!
  //! - `PP` field is used to encode prefixes needed by the instruction or as a
  //!   part of VEX/EVEX prefix. Described as `pp` in instruction manuals.
  //!
  //! - `LL` field is used exclusively by AVX+ and AVX512+ instruction sets. It
  //!   describes vector size, which is `L.128` for XMM register, `L.256` for
  //!   for YMM register, and `L.512` for ZMM register. The `LL` field is omitted
  //!   in case that instruction supports multiple vector lengths, however, if the
  //!   instruction requires specific `L` value it must be specified as a part of
  //!   the opcode.
  //!
  //!   NOTE: `LL` having value `11` is not defined yet.
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
  //! - `O` field is an extended opcode field (3 bits) embedded in ModR/M BYTE.
  //!
  //! - `CDSHL` and `CDTT` fields describe 'compressed-displacement'. `CDSHL` is
  //!   defined for each instruction that is AVX-512 encodable (EVEX) and contains
  //!   a base N shift (base shift to perform the calculation). The `CDTT` field
  //!   is derived from instruction specification and describes additional shift
  //!   to calculate the final `CDSHL` that will be used in SIB byte.
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
    //  * `MMMMM` field in AVX/XOP/AVX-512 instruction.
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
    //         used as an extension to `MM` field describing 0F|0F38|0F3A to also
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
    // collides with 3 bits from `MM` and 5 bits from 'CDSHL' and 'CDTT'.
    // It's fine as FPU and AVX512 flags are never used at the same time.
    kOpCode_FPU_2B_Shift  = 10,
    kOpCode_FPU_2B_Mask   = 0xFF << kOpCode_FPU_2B_Shift,

    // ------------------------------------------------------------------------
    // [CDSHL | CDTT]
    // ------------------------------------------------------------------------

    // Compressed displacement bits.
    //
    // Each opcode defines the base size (N) shift:
    //   [0]: BYTE  (1 byte).
    //   [1]: WORD  (2 bytes).
    //   [2]: DWORD (4 bytes - float/int32).
    //   [3]: QWORD (8 bytes - double/int64).
    //   [4]: OWORD (16 bytes - used by FV|FVM|M128).
    //
    // Which is then scaled by the instruction's TT (TupleType) into possible:
    //   [5]: YWORD (32 bytes)
    //   [6]: ZWORD (64 bytes)
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

  //! Instruction options (AsmJit specific).
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

  //! Supported architectures.
  ASMJIT_ENUM(ArchMask) {
    kArchMaskX86          = 0x01,        //!< X86 mode supported.
    kArchMaskX64          = 0x02         //!< X64 mode supported.
  };

  ASMJIT_ENUM(SingleRegCase) {
    kSingleRegNone        = 0,           //!< No special handling.
    kSingleRegRO          = 1,           //!< Operands become read-only  - `REG & REG` and similar.
    kSingleRegWO          = 2            //!< Operands become write-only - `REG ^ REG` and similar.
  };

  //! Instruction's operand flags.
  ASMJIT_ENUM(OpFlags) {
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
  ASMJIT_ENUM(MemOpFlags) {
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

    //! Get if the instruction may or will jump (returns true also for calls and returns).
    ASMJIT_INLINE bool doesJump() const noexcept { return _jumpType != AnyInst::kJumpTypeNone; }

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

    ASMJIT_INLINE uint32_t getJumpType() const noexcept { return _jumpType; }
    ASMJIT_INLINE uint32_t getSingleRegCase() const noexcept { return _singleRegCase; }

    uint32_t _flags;                     //!< Instruction flags.

    uint32_t _writeIndex      : 8;       //!< First DST byte of a WRITE operation (default 0).
    uint32_t _writeSize       : 8;       //!< number of bytes to be written in DST.
    uint32_t _eflagsIn        : 8;       //!< EFLAGS read by the instruction.
    uint32_t _eflagsOut       : 8;       //!< EFLAGS modified by the instruction.

    uint32_t _altOpCodeIndex  : 8;       //!< Index to table with alternative opcodes.
    uint32_t _iSignatureIndex : 9;       //!< First `ISignature` entry in the database.
    uint32_t _iSignatureCount : 4;       //!< Number of relevant `ISignature` entries.
    uint32_t _jumpType        : 3;       //!< Jump type, see `AnyInst::JumpType`.
    uint32_t _singleRegCase   : 2;       //!< Specifies what happens if all operands share the same register.
    uint32_t _reserved        : 6;       //!< \internal
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
      kAvxConvMove            = 1,       //!< No change (no operands changed).
      kAvxConvMoveIfMem       = 2,       //!< No change if second operand is a memory, otherwise Extend.
      kAvxConvExtend          = 3,       //!< The first SSE operand becomes first and second AVX operand.
      kAvxConvBlend           = 4        //!< Special case for 'vblendvpd', 'vblendvps', and 'vpblendvb'.
    };

    uint32_t features         : 16;      //!< CPU features.
    uint32_t avxConvMode      :  3;      //!< SSE to AVX conversion mode, see \ref AvxConvMode.
    int32_t avxConvDelta      : 13;      //!< Delta to get a corresponding AVX instruction.
  };

  //! Data specific to AVX+ (including XOP and AVX-512), FMA+ (FMA3 and FMA4), and F16C instructions.
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
      kFeatureAVX512_F    = 0x00001000U, //!< Supported by AVX512-F (foundation).
      kFeatureAVX512_VL   = 0x00002000U, //!< Supports access to XMM|YMM registers if AVX512VL is present.
      kFeatureAVX512_CDI  = 0x00004000U, //!< Supported by AVX512-CDI (conflict detection).
      kFeatureAVX512_PFI  = 0x00008000U, //!< Supported by AVX512-PFI (prefetch).
      kFeatureAVX512_ERI  = 0x00010000U, //!< Supported by AVX512-ERI (exponential and reciprocal).
      kFeatureAVX512_DQ   = 0x00020000U, //!< Supported by AVX512-DQ (dword/qword).
      kFeatureAVX512_BW   = 0x00040000U, //!< Supported by AVX512-BW (byte/word).
      kFeatureAVX512_IFMA = 0x00080000U, //!< Supported by AVX512-IFMA (integer fused-multiply-add).
      kFeatureAVX512_VBMI = 0x00100000U  //!< Supported by AVX512-VBMI (vector byte manipulation).
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
    uint8_t opCount  : 3;                //!< Count of operands in `opIndex` (0..6).
    uint8_t archMask : 2;                //!< Architecture mask of this record.
    uint8_t implicit : 3;                //!< Number of implicit operands.
    uint8_t reserved;                    //!< Reserved for future use.
    uint8_t operands[6];                 //!< Indexes to `OSignature` table.
  };

  //! Operand signature, used by \ref ISignature.
  //!
  //! Contains all possible operand combinations, memory size information,
  //! and register index (or \ref Globals::kInvalidRegId if not mandatory).
  struct OSignature {
    uint32_t flags;                      //!< Operand flags.
    uint16_t memFlags;                   //!< Memory flags.
    uint8_t extFlags;                    //!< Extra flags.
    uint8_t regMask;                     //!< Mask of possible register IDs.
  };

  //! Data that is not related to a specific X86 instruction (not referenced by
  //! any tables).
  struct MiscData {
    uint32_t condToJcc[x86::kCondCount];
    uint32_t condToSetcc[x86::kCondCount];
    uint32_t condToCmovcc[x86::kCondCount];
    uint32_t reversedCond[x86::kCondCount];
  };

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! Get instruction name (null terminated).
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

  //! Get if the `instId` is defined (counts also kInvalidInstId, which is zero).
  static ASMJIT_INLINE bool isDefinedId(uint32_t instId) noexcept { return instId < _kIdCount; }

  //! Get instruction information based on the instruction `instId`.
  //!
  //! NOTE: `instId` has to be a valid instruction ID, it can't be greater than
  //! or equal to `X86Inst::_kIdCount`. It asserts in debug mode.
  static ASMJIT_INLINE const X86Inst& getInst(uint32_t instId) noexcept;

  // --------------------------------------------------------------------------
  // [Utilities]
  // --------------------------------------------------------------------------

  static ASMJIT_INLINE const MiscData& getMiscData() noexcept;

  //! Get the equivalent of a negated condition code.
  static ASMJIT_INLINE uint32_t negateCond(uint32_t cond) noexcept {
    ASMJIT_ASSERT(cond < x86::kCondCount);
    return cond ^ 1;
  }

  //! Convert a condition code into a condition code that reverses the
  //! corresponding operands of a comparison.
  static ASMJIT_INLINE uint32_t reverseCond(uint32_t cond) noexcept {
    ASMJIT_ASSERT(cond < x86::kCondCount);
    return getMiscData().reversedCond[cond];
  }

  //! Translate a condition code `cc` to a "cmovcc" instruction id.
  static ASMJIT_INLINE uint32_t condToCmovcc(uint32_t cond) noexcept {
    ASMJIT_ASSERT(cond < x86::kCondCount);
    return getMiscData().condToCmovcc[cond];
  }

  //! Translate a condition code `cc` to a "jcc" instruction id.
  static ASMJIT_INLINE uint32_t condToJcc(uint32_t cond) noexcept {
    ASMJIT_ASSERT(cond < x86::kCondCount);
    return getMiscData().condToJcc[cond];
  }

  //! Translate a condition code `cc` to a "setcc" instruction id.
  static ASMJIT_INLINE uint32_t condToSetcc(uint32_t cond) noexcept {
    ASMJIT_ASSERT(cond < x86::kCondCount);
    return getMiscData().condToSetcc[cond];
  }

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
  //! `kInvalidInstId` (zero) is returned instead. The given `name` doesn't have
  //! to be null-terminated if `len` is provided.
  ASMJIT_API static uint32_t getIdByName(const char* name, size_t len = Globals::kInvalidIndex) noexcept;

  //! Get an instruction name from a given instruction id `instId`.
  ASMJIT_API static const char* getNameById(uint32_t instId) noexcept;
#endif // !ASMJIT_DISABLE_TEXT

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
  ASMJIT_API static const X86Inst::MiscData miscData;
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

ASMJIT_INLINE const X86Inst::MiscData& X86Inst::getMiscData() noexcept {
  return X86InstDB::miscData;
}

//! \}

} // asmjit namespace

// [Api-End]
#include "../asmjit_apiend.h"

// [Guard]
#endif // _ASMJIT_X86_X86INST_H
