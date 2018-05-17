// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// ZLIB - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_X86_X86GLOBALS_H
#define _ASMJIT_X86_X86GLOBALS_H

// [Dependencies]
#include "../core/arch.h"
#include "../core/inst.h"

ASMJIT_BEGIN_SUB_NAMESPACE(x86)

//! \addtogroup asmjit_x86_api
//! \{

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

  //! Reverse a condition code (reverses the corresponding operands of a comparison).
  static constexpr uint32_t reverse(uint32_t cond) noexcept { return reverseTable[cond]; }
  //! Negate a condition code.
  static constexpr uint32_t negate(uint32_t cond) noexcept { return cond ^ 1u; }
}

// ============================================================================
// [asmjit::x86::Inst]
// ============================================================================

//! Instruction (X86).
//!
//! NOTE: Only used to hold x86-specific enumerations and static functions.
struct Inst : public BaseInst {
  //! Instruction id (X86).
  enum Id : uint32_t {
    // ${InstId:Begin}
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
    kIdAesdec,                           // [ANY] {AESNI}
    kIdAesdeclast,                       // [ANY] {AESNI}
    kIdAesenc,                           // [ANY] {AESNI}
    kIdAesenclast,                       // [ANY] {AESNI}
    kIdAesimc,                           // [ANY] {AESNI}
    kIdAeskeygenassist,                  // [ANY] {AESNI}
    kIdAnd,                              // [ANY]
    kIdAndn,                             // [ANY] {BMI}
    kIdAndnpd,                           // [ANY] {SSE2}
    kIdAndnps,                           // [ANY] {SSE}
    kIdAndpd,                            // [ANY] {SSE2}
    kIdAndps,                            // [ANY] {SSE}
    kIdArpl,                             // [X86]
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
    kIdBndcl,                            // [ANY] {MPX}
    kIdBndcn,                            // [ANY] {MPX}
    kIdBndcu,                            // [ANY] {MPX}
    kIdBndldx,                           // [ANY] {MPX}
    kIdBndmk,                            // [ANY] {MPX}
    kIdBndmov,                           // [ANY] {MPX}
    kIdBndstx,                           // [ANY] {MPX}
    kIdBound,                            // [X86]
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
    kIdClflushopt,                       // [ANY] {CLFLUSHOPT}
    kIdCli,                              // [ANY]
    kIdClts,                             // [ANY]
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
    kIdHlt,                              // [ANY]
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
    kIdInto,                             // [X86]
    kIdInvd,                             // [ANY] {I486}
    kIdInvlpg,                           // [ANY] {I486}
    kIdInvpcid,                          // [ANY] {I486}
    kIdIret,                             // [ANY]
    kIdIretd,                            // [ANY]
    kIdIretq,                            // [X64]
    kIdIretw,                            // [ANY]
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
    kIdLar,                              // [ANY]
    kIdLddqu,                            // [ANY] {SSE3}
    kIdLdmxcsr,                          // [ANY] {SSE}
    kIdLds,                              // [X86]
    kIdLea,                              // [ANY]
    kIdLeave,                            // [ANY]
    kIdLes,                              // [X86]
    kIdLfence,                           // [ANY] {SSE2}
    kIdLfs,                              // [ANY]
    kIdLgdt,                             // [ANY]
    kIdLgs,                              // [ANY]
    kIdLidt,                             // [ANY]
    kIdLldt,                             // [ANY]
    kIdLmsw,                             // [ANY]
    kIdLods,                             // [ANY]
    kIdLoop,                             // [ANY]
    kIdLoope,                            // [ANY]
    kIdLoopne,                           // [ANY]
    kIdLsl,                              // [ANY]
    kIdLss,                              // [ANY]
    kIdLtr,                              // [ANY]
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
    kIdMonitor,                          // [ANY] {MONITOR}
    kIdMonitorx,                         // [ANY] {MONITORX}
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
    kIdMwait,                            // [ANY] {MONITOR}
    kIdMwaitx,                           // [ANY] {MONITORX}
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
    kIdPcmpgtq,                          // [ANY] {SSE4_2}
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
    kIdRdmsr,                            // [ANY] {MSR}
    kIdRdpmc,                            // [ANY]
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
    kIdRsm,                              // [X86]
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
    kIdSgdt,                             // [ANY]
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
    kIdSidt,                             // [ANY]
    kIdSldt,                             // [ANY]
    kIdSmsw,                             // [ANY]
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
    kIdStr,                              // [ANY]
    kIdSub,                              // [ANY]
    kIdSubpd,                            // [ANY] {SSE2}
    kIdSubps,                            // [ANY] {SSE}
    kIdSubsd,                            // [ANY] {SSE2}
    kIdSubss,                            // [ANY] {SSE}
    kIdSwapgs,                           // [X64]
    kIdSyscall,                          // [X64]
    kIdSysenter,                         // [ANY]
    kIdSysexit,                          // [ANY]
    kIdSysexit64,                        // [ANY]
    kIdSysret,                           // [X64]
    kIdSysret64,                         // [X64]
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
    kIdV4fmaddps,                        // [ANY] {AVX512_4FMAPS}
    kIdV4fmaddss,                        // [ANY] {AVX512_4FMAPS}
    kIdV4fnmaddps,                       // [ANY] {AVX512_4FMAPS}
    kIdV4fnmaddss,                       // [ANY] {AVX512_4FMAPS}
    kIdVaddpd,                           // [ANY] {AVX|AVX512_F+VL}
    kIdVaddps,                           // [ANY] {AVX|AVX512_F+VL}
    kIdVaddsd,                           // [ANY] {AVX|AVX512_F}
    kIdVaddss,                           // [ANY] {AVX|AVX512_F}
    kIdVaddsubpd,                        // [ANY] {AVX}
    kIdVaddsubps,                        // [ANY] {AVX}
    kIdVaesdec,                          // [ANY] {AESNI|AVX|AVX512_F|VAES+VL}
    kIdVaesdeclast,                      // [ANY] {AESNI|AVX|AVX512_F|VAES+VL}
    kIdVaesenc,                          // [ANY] {AESNI|AVX|AVX512_F|VAES+VL}
    kIdVaesenclast,                      // [ANY] {AESNI|AVX|AVX512_F|VAES+VL}
    kIdVaesimc,                          // [ANY] {AESNI|AVX}
    kIdVaeskeygenassist,                 // [ANY] {AESNI|AVX}
    kIdValignd,                          // [ANY] {AVX512_F+VL}
    kIdValignq,                          // [ANY] {AVX512_F+VL}
    kIdVandnpd,                          // [ANY] {AVX|AVX512_DQ+VL}
    kIdVandnps,                          // [ANY] {AVX|AVX512_DQ+VL}
    kIdVandpd,                           // [ANY] {AVX|AVX512_DQ+VL}
    kIdVandps,                           // [ANY] {AVX|AVX512_DQ+VL}
    kIdVblendmb,                         // [ANY] {AVX512_BW+VL}
    kIdVblendmd,                         // [ANY] {AVX512_F+VL}
    kIdVblendmpd,                        // [ANY] {AVX512_F+VL}
    kIdVblendmps,                        // [ANY] {AVX512_F+VL}
    kIdVblendmq,                         // [ANY] {AVX512_F+VL}
    kIdVblendmw,                         // [ANY] {AVX512_BW+VL}
    kIdVblendpd,                         // [ANY] {AVX}
    kIdVblendps,                         // [ANY] {AVX}
    kIdVblendvpd,                        // [ANY] {AVX}
    kIdVblendvps,                        // [ANY] {AVX}
    kIdVbroadcastf128,                   // [ANY] {AVX}
    kIdVbroadcastf32x2,                  // [ANY] {AVX512_DQ+VL}
    kIdVbroadcastf32x4,                  // [ANY] {AVX512_F}
    kIdVbroadcastf32x8,                  // [ANY] {AVX512_DQ}
    kIdVbroadcastf64x2,                  // [ANY] {AVX512_DQ+VL}
    kIdVbroadcastf64x4,                  // [ANY] {AVX512_F}
    kIdVbroadcasti128,                   // [ANY] {AVX2}
    kIdVbroadcasti32x2,                  // [ANY] {AVX512_DQ+VL}
    kIdVbroadcasti32x4,                  // [ANY] {AVX512_F+VL}
    kIdVbroadcasti32x8,                  // [ANY] {AVX512_DQ}
    kIdVbroadcasti64x2,                  // [ANY] {AVX512_DQ+VL}
    kIdVbroadcasti64x4,                  // [ANY] {AVX512_F}
    kIdVbroadcastsd,                     // [ANY] {AVX|AVX2|AVX512_F+VL}
    kIdVbroadcastss,                     // [ANY] {AVX|AVX2|AVX512_F+VL}
    kIdVcmppd,                           // [ANY] {AVX|AVX512_F+VL}
    kIdVcmpps,                           // [ANY] {AVX|AVX512_F+VL}
    kIdVcmpsd,                           // [ANY] {AVX|AVX512_F}
    kIdVcmpss,                           // [ANY] {AVX|AVX512_F}
    kIdVcomisd,                          // [ANY] {AVX|AVX512_F}
    kIdVcomiss,                          // [ANY] {AVX|AVX512_F}
    kIdVcompresspd,                      // [ANY] {AVX512_F+VL}
    kIdVcompressps,                      // [ANY] {AVX512_F+VL}
    kIdVcvtdq2pd,                        // [ANY] {AVX|AVX512_F+VL}
    kIdVcvtdq2ps,                        // [ANY] {AVX|AVX512_F+VL}
    kIdVcvtpd2dq,                        // [ANY] {AVX|AVX512_F+VL}
    kIdVcvtpd2ps,                        // [ANY] {AVX|AVX512_F+VL}
    kIdVcvtpd2qq,                        // [ANY] {AVX512_DQ+VL}
    kIdVcvtpd2udq,                       // [ANY] {AVX512_F+VL}
    kIdVcvtpd2uqq,                       // [ANY] {AVX512_DQ+VL}
    kIdVcvtph2ps,                        // [ANY] {AVX512_F|F16C+VL}
    kIdVcvtps2dq,                        // [ANY] {AVX|AVX512_F+VL}
    kIdVcvtps2pd,                        // [ANY] {AVX|AVX512_F+VL}
    kIdVcvtps2ph,                        // [ANY] {AVX512_F|F16C+VL}
    kIdVcvtps2qq,                        // [ANY] {AVX512_DQ+VL}
    kIdVcvtps2udq,                       // [ANY] {AVX512_F+VL}
    kIdVcvtps2uqq,                       // [ANY] {AVX512_DQ+VL}
    kIdVcvtqq2pd,                        // [ANY] {AVX512_DQ+VL}
    kIdVcvtqq2ps,                        // [ANY] {AVX512_DQ+VL}
    kIdVcvtsd2si,                        // [ANY] {AVX|AVX512_F}
    kIdVcvtsd2ss,                        // [ANY] {AVX|AVX512_F}
    kIdVcvtsd2usi,                       // [ANY] {AVX512_F}
    kIdVcvtsi2sd,                        // [ANY] {AVX|AVX512_F}
    kIdVcvtsi2ss,                        // [ANY] {AVX|AVX512_F}
    kIdVcvtss2sd,                        // [ANY] {AVX|AVX512_F}
    kIdVcvtss2si,                        // [ANY] {AVX|AVX512_F}
    kIdVcvtss2usi,                       // [ANY] {AVX512_F}
    kIdVcvttpd2dq,                       // [ANY] {AVX|AVX512_F+VL}
    kIdVcvttpd2qq,                       // [ANY] {AVX512_F+VL}
    kIdVcvttpd2udq,                      // [ANY] {AVX512_F+VL}
    kIdVcvttpd2uqq,                      // [ANY] {AVX512_DQ+VL}
    kIdVcvttps2dq,                       // [ANY] {AVX|AVX512_F+VL}
    kIdVcvttps2qq,                       // [ANY] {AVX512_DQ+VL}
    kIdVcvttps2udq,                      // [ANY] {AVX512_F+VL}
    kIdVcvttps2uqq,                      // [ANY] {AVX512_DQ+VL}
    kIdVcvttsd2si,                       // [ANY] {AVX|AVX512_F}
    kIdVcvttsd2usi,                      // [ANY] {AVX512_F}
    kIdVcvttss2si,                       // [ANY] {AVX|AVX512_F}
    kIdVcvttss2usi,                      // [ANY] {AVX512_F}
    kIdVcvtudq2pd,                       // [ANY] {AVX512_F+VL}
    kIdVcvtudq2ps,                       // [ANY] {AVX512_F+VL}
    kIdVcvtuqq2pd,                       // [ANY] {AVX512_DQ+VL}
    kIdVcvtuqq2ps,                       // [ANY] {AVX512_DQ+VL}
    kIdVcvtusi2sd,                       // [ANY] {AVX512_F}
    kIdVcvtusi2ss,                       // [ANY] {AVX512_F}
    kIdVdbpsadbw,                        // [ANY] {AVX512_BW+VL}
    kIdVdivpd,                           // [ANY] {AVX|AVX512_F+VL}
    kIdVdivps,                           // [ANY] {AVX|AVX512_F+VL}
    kIdVdivsd,                           // [ANY] {AVX|AVX512_F}
    kIdVdivss,                           // [ANY] {AVX|AVX512_F}
    kIdVdppd,                            // [ANY] {AVX}
    kIdVdpps,                            // [ANY] {AVX}
    kIdVerr,                             // [ANY]
    kIdVerw,                             // [ANY]
    kIdVexp2pd,                          // [ANY] {AVX512_ERI}
    kIdVexp2ps,                          // [ANY] {AVX512_ERI}
    kIdVexpandpd,                        // [ANY] {AVX512_F+VL}
    kIdVexpandps,                        // [ANY] {AVX512_F+VL}
    kIdVextractf128,                     // [ANY] {AVX}
    kIdVextractf32x4,                    // [ANY] {AVX512_F+VL}
    kIdVextractf32x8,                    // [ANY] {AVX512_DQ}
    kIdVextractf64x2,                    // [ANY] {AVX512_DQ+VL}
    kIdVextractf64x4,                    // [ANY] {AVX512_F}
    kIdVextracti128,                     // [ANY] {AVX2}
    kIdVextracti32x4,                    // [ANY] {AVX512_F+VL}
    kIdVextracti32x8,                    // [ANY] {AVX512_DQ}
    kIdVextracti64x2,                    // [ANY] {AVX512_DQ+VL}
    kIdVextracti64x4,                    // [ANY] {AVX512_F}
    kIdVextractps,                       // [ANY] {AVX|AVX512_F}
    kIdVfixupimmpd,                      // [ANY] {AVX512_F+VL}
    kIdVfixupimmps,                      // [ANY] {AVX512_F+VL}
    kIdVfixupimmsd,                      // [ANY] {AVX512_F}
    kIdVfixupimmss,                      // [ANY] {AVX512_F}
    kIdVfmadd132pd,                      // [ANY] {AVX512_F|FMA+VL}
    kIdVfmadd132ps,                      // [ANY] {AVX512_F|FMA+VL}
    kIdVfmadd132sd,                      // [ANY] {AVX512_F|FMA}
    kIdVfmadd132ss,                      // [ANY] {AVX512_F|FMA}
    kIdVfmadd213pd,                      // [ANY] {AVX512_F|FMA+VL}
    kIdVfmadd213ps,                      // [ANY] {AVX512_F|FMA+VL}
    kIdVfmadd213sd,                      // [ANY] {AVX512_F|FMA}
    kIdVfmadd213ss,                      // [ANY] {AVX512_F|FMA}
    kIdVfmadd231pd,                      // [ANY] {AVX512_F|FMA+VL}
    kIdVfmadd231ps,                      // [ANY] {AVX512_F|FMA+VL}
    kIdVfmadd231sd,                      // [ANY] {AVX512_F|FMA}
    kIdVfmadd231ss,                      // [ANY] {AVX512_F|FMA}
    kIdVfmaddpd,                         // [ANY] {FMA4}
    kIdVfmaddps,                         // [ANY] {FMA4}
    kIdVfmaddsd,                         // [ANY] {FMA4}
    kIdVfmaddss,                         // [ANY] {FMA4}
    kIdVfmaddsub132pd,                   // [ANY] {AVX512_F|FMA+VL}
    kIdVfmaddsub132ps,                   // [ANY] {AVX512_F|FMA+VL}
    kIdVfmaddsub213pd,                   // [ANY] {AVX512_F|FMA+VL}
    kIdVfmaddsub213ps,                   // [ANY] {AVX512_F|FMA+VL}
    kIdVfmaddsub231pd,                   // [ANY] {AVX512_F|FMA+VL}
    kIdVfmaddsub231ps,                   // [ANY] {AVX512_F|FMA+VL}
    kIdVfmaddsubpd,                      // [ANY] {FMA4}
    kIdVfmaddsubps,                      // [ANY] {FMA4}
    kIdVfmsub132pd,                      // [ANY] {AVX512_F|FMA+VL}
    kIdVfmsub132ps,                      // [ANY] {AVX512_F|FMA+VL}
    kIdVfmsub132sd,                      // [ANY] {AVX512_F|FMA}
    kIdVfmsub132ss,                      // [ANY] {AVX512_F|FMA}
    kIdVfmsub213pd,                      // [ANY] {AVX512_F|FMA+VL}
    kIdVfmsub213ps,                      // [ANY] {AVX512_F|FMA+VL}
    kIdVfmsub213sd,                      // [ANY] {AVX512_F|FMA}
    kIdVfmsub213ss,                      // [ANY] {AVX512_F|FMA}
    kIdVfmsub231pd,                      // [ANY] {AVX512_F|FMA+VL}
    kIdVfmsub231ps,                      // [ANY] {AVX512_F|FMA+VL}
    kIdVfmsub231sd,                      // [ANY] {AVX512_F|FMA}
    kIdVfmsub231ss,                      // [ANY] {AVX512_F|FMA}
    kIdVfmsubadd132pd,                   // [ANY] {AVX512_F|FMA+VL}
    kIdVfmsubadd132ps,                   // [ANY] {AVX512_F|FMA+VL}
    kIdVfmsubadd213pd,                   // [ANY] {AVX512_F|FMA+VL}
    kIdVfmsubadd213ps,                   // [ANY] {AVX512_F|FMA+VL}
    kIdVfmsubadd231pd,                   // [ANY] {AVX512_F|FMA+VL}
    kIdVfmsubadd231ps,                   // [ANY] {AVX512_F|FMA+VL}
    kIdVfmsubaddpd,                      // [ANY] {FMA4}
    kIdVfmsubaddps,                      // [ANY] {FMA4}
    kIdVfmsubpd,                         // [ANY] {FMA4}
    kIdVfmsubps,                         // [ANY] {FMA4}
    kIdVfmsubsd,                         // [ANY] {FMA4}
    kIdVfmsubss,                         // [ANY] {FMA4}
    kIdVfnmadd132pd,                     // [ANY] {AVX512_F|FMA+VL}
    kIdVfnmadd132ps,                     // [ANY] {AVX512_F|FMA+VL}
    kIdVfnmadd132sd,                     // [ANY] {AVX512_F|FMA}
    kIdVfnmadd132ss,                     // [ANY] {AVX512_F|FMA}
    kIdVfnmadd213pd,                     // [ANY] {AVX512_F|FMA+VL}
    kIdVfnmadd213ps,                     // [ANY] {AVX512_F|FMA+VL}
    kIdVfnmadd213sd,                     // [ANY] {AVX512_F|FMA}
    kIdVfnmadd213ss,                     // [ANY] {AVX512_F|FMA}
    kIdVfnmadd231pd,                     // [ANY] {AVX512_F|FMA+VL}
    kIdVfnmadd231ps,                     // [ANY] {AVX512_F|FMA+VL}
    kIdVfnmadd231sd,                     // [ANY] {AVX512_F|FMA}
    kIdVfnmadd231ss,                     // [ANY] {AVX512_F|FMA}
    kIdVfnmaddpd,                        // [ANY] {FMA4}
    kIdVfnmaddps,                        // [ANY] {FMA4}
    kIdVfnmaddsd,                        // [ANY] {FMA4}
    kIdVfnmaddss,                        // [ANY] {FMA4}
    kIdVfnmsub132pd,                     // [ANY] {AVX512_F|FMA+VL}
    kIdVfnmsub132ps,                     // [ANY] {AVX512_F|FMA+VL}
    kIdVfnmsub132sd,                     // [ANY] {AVX512_F|FMA}
    kIdVfnmsub132ss,                     // [ANY] {AVX512_F|FMA}
    kIdVfnmsub213pd,                     // [ANY] {AVX512_F|FMA+VL}
    kIdVfnmsub213ps,                     // [ANY] {AVX512_F|FMA+VL}
    kIdVfnmsub213sd,                     // [ANY] {AVX512_F|FMA}
    kIdVfnmsub213ss,                     // [ANY] {AVX512_F|FMA}
    kIdVfnmsub231pd,                     // [ANY] {AVX512_F|FMA+VL}
    kIdVfnmsub231ps,                     // [ANY] {AVX512_F|FMA+VL}
    kIdVfnmsub231sd,                     // [ANY] {AVX512_F|FMA}
    kIdVfnmsub231ss,                     // [ANY] {AVX512_F|FMA}
    kIdVfnmsubpd,                        // [ANY] {FMA4}
    kIdVfnmsubps,                        // [ANY] {FMA4}
    kIdVfnmsubsd,                        // [ANY] {FMA4}
    kIdVfnmsubss,                        // [ANY] {FMA4}
    kIdVfpclasspd,                       // [ANY] {AVX512_DQ+VL}
    kIdVfpclassps,                       // [ANY] {AVX512_DQ+VL}
    kIdVfpclasssd,                       // [ANY] {AVX512_DQ}
    kIdVfpclassss,                       // [ANY] {AVX512_DQ}
    kIdVfrczpd,                          // [ANY] {XOP}
    kIdVfrczps,                          // [ANY] {XOP}
    kIdVfrczsd,                          // [ANY] {XOP}
    kIdVfrczss,                          // [ANY] {XOP}
    kIdVgatherdpd,                       // [ANY] {AVX2|AVX512_F+VL}
    kIdVgatherdps,                       // [ANY] {AVX2|AVX512_F+VL}
    kIdVgatherpf0dpd,                    // [ANY] {AVX512_PFI}
    kIdVgatherpf0dps,                    // [ANY] {AVX512_PFI}
    kIdVgatherpf0qpd,                    // [ANY] {AVX512_PFI}
    kIdVgatherpf0qps,                    // [ANY] {AVX512_PFI}
    kIdVgatherpf1dpd,                    // [ANY] {AVX512_PFI}
    kIdVgatherpf1dps,                    // [ANY] {AVX512_PFI}
    kIdVgatherpf1qpd,                    // [ANY] {AVX512_PFI}
    kIdVgatherpf1qps,                    // [ANY] {AVX512_PFI}
    kIdVgatherqpd,                       // [ANY] {AVX2|AVX512_F+VL}
    kIdVgatherqps,                       // [ANY] {AVX2|AVX512_F+VL}
    kIdVgetexppd,                        // [ANY] {AVX512_F+VL}
    kIdVgetexpps,                        // [ANY] {AVX512_F+VL}
    kIdVgetexpsd,                        // [ANY] {AVX512_F}
    kIdVgetexpss,                        // [ANY] {AVX512_F}
    kIdVgetmantpd,                       // [ANY] {AVX512_F+VL}
    kIdVgetmantps,                       // [ANY] {AVX512_F+VL}
    kIdVgetmantsd,                       // [ANY] {AVX512_F}
    kIdVgetmantss,                       // [ANY] {AVX512_F}
    kIdVhaddpd,                          // [ANY] {AVX}
    kIdVhaddps,                          // [ANY] {AVX}
    kIdVhsubpd,                          // [ANY] {AVX}
    kIdVhsubps,                          // [ANY] {AVX}
    kIdVinsertf128,                      // [ANY] {AVX}
    kIdVinsertf32x4,                     // [ANY] {AVX512_F+VL}
    kIdVinsertf32x8,                     // [ANY] {AVX512_DQ}
    kIdVinsertf64x2,                     // [ANY] {AVX512_DQ+VL}
    kIdVinsertf64x4,                     // [ANY] {AVX512_F}
    kIdVinserti128,                      // [ANY] {AVX2}
    kIdVinserti32x4,                     // [ANY] {AVX512_F+VL}
    kIdVinserti32x8,                     // [ANY] {AVX512_DQ}
    kIdVinserti64x2,                     // [ANY] {AVX512_DQ+VL}
    kIdVinserti64x4,                     // [ANY] {AVX512_F}
    kIdVinsertps,                        // [ANY] {AVX|AVX512_F}
    kIdVlddqu,                           // [ANY] {AVX}
    kIdVldmxcsr,                         // [ANY] {AVX}
    kIdVmaskmovdqu,                      // [ANY] {AVX}
    kIdVmaskmovpd,                       // [ANY] {AVX}
    kIdVmaskmovps,                       // [ANY] {AVX}
    kIdVmaxpd,                           // [ANY] {AVX|AVX512_F+VL}
    kIdVmaxps,                           // [ANY] {AVX|AVX512_F+VL}
    kIdVmaxsd,                           // [ANY] {AVX|AVX512_F+VL}
    kIdVmaxss,                           // [ANY] {AVX|AVX512_F+VL}
    kIdVminpd,                           // [ANY] {AVX|AVX512_F+VL}
    kIdVminps,                           // [ANY] {AVX|AVX512_F+VL}
    kIdVminsd,                           // [ANY] {AVX|AVX512_F+VL}
    kIdVminss,                           // [ANY] {AVX|AVX512_F+VL}
    kIdVmovapd,                          // [ANY] {AVX|AVX512_F+VL}
    kIdVmovaps,                          // [ANY] {AVX|AVX512_F+VL}
    kIdVmovd,                            // [ANY] {AVX|AVX512_F}
    kIdVmovddup,                         // [ANY] {AVX|AVX512_F+VL}
    kIdVmovdqa,                          // [ANY] {AVX}
    kIdVmovdqa32,                        // [ANY] {AVX512_F+VL}
    kIdVmovdqa64,                        // [ANY] {AVX512_F+VL}
    kIdVmovdqu,                          // [ANY] {AVX}
    kIdVmovdqu16,                        // [ANY] {AVX512_BW+VL}
    kIdVmovdqu32,                        // [ANY] {AVX512_F+VL}
    kIdVmovdqu64,                        // [ANY] {AVX512_F+VL}
    kIdVmovdqu8,                         // [ANY] {AVX512_BW+VL}
    kIdVmovhlps,                         // [ANY] {AVX|AVX512_F}
    kIdVmovhpd,                          // [ANY] {AVX|AVX512_F}
    kIdVmovhps,                          // [ANY] {AVX|AVX512_F}
    kIdVmovlhps,                         // [ANY] {AVX|AVX512_F}
    kIdVmovlpd,                          // [ANY] {AVX|AVX512_F}
    kIdVmovlps,                          // [ANY] {AVX|AVX512_F}
    kIdVmovmskpd,                        // [ANY] {AVX}
    kIdVmovmskps,                        // [ANY] {AVX}
    kIdVmovntdq,                         // [ANY] {AVX|AVX512_F+VL}
    kIdVmovntdqa,                        // [ANY] {AVX|AVX2|AVX512_F+VL}
    kIdVmovntpd,                         // [ANY] {AVX|AVX512_F+VL}
    kIdVmovntps,                         // [ANY] {AVX|AVX512_F+VL}
    kIdVmovq,                            // [ANY] {AVX|AVX512_F}
    kIdVmovsd,                           // [ANY] {AVX|AVX512_F}
    kIdVmovshdup,                        // [ANY] {AVX|AVX512_F+VL}
    kIdVmovsldup,                        // [ANY] {AVX|AVX512_F+VL}
    kIdVmovss,                           // [ANY] {AVX|AVX512_F}
    kIdVmovupd,                          // [ANY] {AVX|AVX512_F+VL}
    kIdVmovups,                          // [ANY] {AVX|AVX512_F+VL}
    kIdVmpsadbw,                         // [ANY] {AVX|AVX2}
    kIdVmulpd,                           // [ANY] {AVX|AVX512_F+VL}
    kIdVmulps,                           // [ANY] {AVX|AVX512_F+VL}
    kIdVmulsd,                           // [ANY] {AVX|AVX512_F}
    kIdVmulss,                           // [ANY] {AVX|AVX512_F}
    kIdVorpd,                            // [ANY] {AVX|AVX512_DQ+VL}
    kIdVorps,                            // [ANY] {AVX|AVX512_DQ+VL}
    kIdVp4dpwssd,                        // [ANY] {AVX512_4VNNIW}
    kIdVp4dpwssds,                       // [ANY] {AVX512_4VNNIW}
    kIdVpabsb,                           // [ANY] {AVX|AVX2|AVX512_BW+VL}
    kIdVpabsd,                           // [ANY] {AVX|AVX2|AVX512_F+VL}
    kIdVpabsq,                           // [ANY] {AVX512_F+VL}
    kIdVpabsw,                           // [ANY] {AVX|AVX2|AVX512_BW+VL}
    kIdVpackssdw,                        // [ANY] {AVX|AVX2|AVX512_BW+VL}
    kIdVpacksswb,                        // [ANY] {AVX|AVX2|AVX512_BW+VL}
    kIdVpackusdw,                        // [ANY] {AVX|AVX2|AVX512_BW+VL}
    kIdVpackuswb,                        // [ANY] {AVX|AVX2|AVX512_BW+VL}
    kIdVpaddb,                           // [ANY] {AVX|AVX2|AVX512_BW+VL}
    kIdVpaddd,                           // [ANY] {AVX|AVX2|AVX512_F+VL}
    kIdVpaddq,                           // [ANY] {AVX|AVX2|AVX512_F+VL}
    kIdVpaddsb,                          // [ANY] {AVX|AVX2|AVX512_BW+VL}
    kIdVpaddsw,                          // [ANY] {AVX|AVX2|AVX512_BW+VL}
    kIdVpaddusb,                         // [ANY] {AVX|AVX2|AVX512_BW+VL}
    kIdVpaddusw,                         // [ANY] {AVX|AVX2|AVX512_BW+VL}
    kIdVpaddw,                           // [ANY] {AVX|AVX2|AVX512_BW+VL}
    kIdVpalignr,                         // [ANY] {AVX|AVX2|AVX512_BW+VL}
    kIdVpand,                            // [ANY] {AVX|AVX2}
    kIdVpandd,                           // [ANY] {AVX512_F+VL}
    kIdVpandn,                           // [ANY] {AVX|AVX2}
    kIdVpandnd,                          // [ANY] {AVX512_F+VL}
    kIdVpandnq,                          // [ANY] {AVX512_F+VL}
    kIdVpandq,                           // [ANY] {AVX512_F+VL}
    kIdVpavgb,                           // [ANY] {AVX|AVX2|AVX512_BW+VL}
    kIdVpavgw,                           // [ANY] {AVX|AVX2|AVX512_BW+VL}
    kIdVpblendd,                         // [ANY] {AVX2}
    kIdVpblendvb,                        // [ANY] {AVX|AVX2}
    kIdVpblendw,                         // [ANY] {AVX|AVX2}
    kIdVpbroadcastb,                     // [ANY] {AVX2|AVX512_BW+VL}
    kIdVpbroadcastd,                     // [ANY] {AVX2|AVX512_F+VL}
    kIdVpbroadcastmb2d,                  // [ANY] {AVX512_CDI+VL}
    kIdVpbroadcastmb2q,                  // [ANY] {AVX512_CDI+VL}
    kIdVpbroadcastq,                     // [ANY] {AVX2|AVX512_F+VL}
    kIdVpbroadcastw,                     // [ANY] {AVX2|AVX512_BW+VL}
    kIdVpclmulqdq,                       // [ANY] {AVX|AVX512_F|PCLMULQDQ|VPCLMULQDQ+VL}
    kIdVpcmov,                           // [ANY] {XOP}
    kIdVpcmpb,                           // [ANY] {AVX512_BW+VL}
    kIdVpcmpd,                           // [ANY] {AVX512_F+VL}
    kIdVpcmpeqb,                         // [ANY] {AVX|AVX2|AVX512_BW+VL}
    kIdVpcmpeqd,                         // [ANY] {AVX|AVX2|AVX512_F+VL}
    kIdVpcmpeqq,                         // [ANY] {AVX|AVX2|AVX512_F+VL}
    kIdVpcmpeqw,                         // [ANY] {AVX|AVX2|AVX512_BW+VL}
    kIdVpcmpestri,                       // [ANY] {AVX}
    kIdVpcmpestrm,                       // [ANY] {AVX}
    kIdVpcmpgtb,                         // [ANY] {AVX|AVX2|AVX512_BW+VL}
    kIdVpcmpgtd,                         // [ANY] {AVX|AVX2|AVX512_F+VL}
    kIdVpcmpgtq,                         // [ANY] {AVX|AVX2|AVX512_F+VL}
    kIdVpcmpgtw,                         // [ANY] {AVX|AVX2|AVX512_BW+VL}
    kIdVpcmpistri,                       // [ANY] {AVX}
    kIdVpcmpistrm,                       // [ANY] {AVX}
    kIdVpcmpq,                           // [ANY] {AVX512_F+VL}
    kIdVpcmpub,                          // [ANY] {AVX512_BW+VL}
    kIdVpcmpud,                          // [ANY] {AVX512_F+VL}
    kIdVpcmpuq,                          // [ANY] {AVX512_F+VL}
    kIdVpcmpuw,                          // [ANY] {AVX512_BW+VL}
    kIdVpcmpw,                           // [ANY] {AVX512_BW+VL}
    kIdVpcomb,                           // [ANY] {XOP}
    kIdVpcomd,                           // [ANY] {XOP}
    kIdVpcompressb,                      // [ANY] {AVX512_VBMI2+VL}
    kIdVpcompressd,                      // [ANY] {AVX512_F+VL}
    kIdVpcompressq,                      // [ANY] {AVX512_F+VL}
    kIdVpcompressw,                      // [ANY] {AVX512_VBMI2+VL}
    kIdVpcomq,                           // [ANY] {XOP}
    kIdVpcomub,                          // [ANY] {XOP}
    kIdVpcomud,                          // [ANY] {XOP}
    kIdVpcomuq,                          // [ANY] {XOP}
    kIdVpcomuw,                          // [ANY] {XOP}
    kIdVpcomw,                           // [ANY] {XOP}
    kIdVpconflictd,                      // [ANY] {AVX512_CDI+VL}
    kIdVpconflictq,                      // [ANY] {AVX512_CDI+VL}
    kIdVperm2f128,                       // [ANY] {AVX}
    kIdVperm2i128,                       // [ANY] {AVX2}
    kIdVpermb,                           // [ANY] {AVX512_VBMI+VL}
    kIdVpermd,                           // [ANY] {AVX2|AVX512_F+VL}
    kIdVpermi2b,                         // [ANY] {AVX512_VBMI+VL}
    kIdVpermi2d,                         // [ANY] {AVX512_F+VL}
    kIdVpermi2pd,                        // [ANY] {AVX512_F+VL}
    kIdVpermi2ps,                        // [ANY] {AVX512_F+VL}
    kIdVpermi2q,                         // [ANY] {AVX512_F+VL}
    kIdVpermi2w,                         // [ANY] {AVX512_BW+VL}
    kIdVpermil2pd,                       // [ANY] {XOP}
    kIdVpermil2ps,                       // [ANY] {XOP}
    kIdVpermilpd,                        // [ANY] {AVX|AVX512_F+VL}
    kIdVpermilps,                        // [ANY] {AVX|AVX512_F+VL}
    kIdVpermpd,                          // [ANY] {AVX2}
    kIdVpermps,                          // [ANY] {AVX2}
    kIdVpermq,                           // [ANY] {AVX2|AVX512_F+VL}
    kIdVpermt2b,                         // [ANY] {AVX512_VBMI+VL}
    kIdVpermt2d,                         // [ANY] {AVX512_F+VL}
    kIdVpermt2pd,                        // [ANY] {AVX512_F+VL}
    kIdVpermt2ps,                        // [ANY] {AVX512_F+VL}
    kIdVpermt2q,                         // [ANY] {AVX512_F+VL}
    kIdVpermt2w,                         // [ANY] {AVX512_BW+VL}
    kIdVpermw,                           // [ANY] {AVX512_BW+VL}
    kIdVpexpandb,                        // [ANY] {AVX512_VBMI2+VL}
    kIdVpexpandd,                        // [ANY] {AVX512_F+VL}
    kIdVpexpandq,                        // [ANY] {AVX512_F+VL}
    kIdVpexpandw,                        // [ANY] {AVX512_VBMI2+VL}
    kIdVpextrb,                          // [ANY] {AVX|AVX512_BW}
    kIdVpextrd,                          // [ANY] {AVX|AVX512_DQ}
    kIdVpextrq,                          // [X64] {AVX|AVX512_DQ}
    kIdVpextrw,                          // [ANY] {AVX|AVX512_BW}
    kIdVpgatherdd,                       // [ANY] {AVX2|AVX512_F+VL}
    kIdVpgatherdq,                       // [ANY] {AVX2|AVX512_F+VL}
    kIdVpgatherqd,                       // [ANY] {AVX2|AVX512_F+VL}
    kIdVpgatherqq,                       // [ANY] {AVX2|AVX512_F+VL}
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
    kIdVplzcntd,                         // [ANY] {AVX512_CDI+VL}
    kIdVplzcntq,                         // [ANY] {AVX512_CDI+VL}
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
    kIdVpmadd52huq,                      // [ANY] {AVX512_IFMA+VL}
    kIdVpmadd52luq,                      // [ANY] {AVX512_IFMA+VL}
    kIdVpmaddubsw,                       // [ANY] {AVX|AVX2|AVX512_BW+VL}
    kIdVpmaddwd,                         // [ANY] {AVX|AVX2|AVX512_BW+VL}
    kIdVpmaskmovd,                       // [ANY] {AVX2}
    kIdVpmaskmovq,                       // [ANY] {AVX2}
    kIdVpmaxsb,                          // [ANY] {AVX|AVX2|AVX512_BW+VL}
    kIdVpmaxsd,                          // [ANY] {AVX|AVX2|AVX512_F+VL}
    kIdVpmaxsq,                          // [ANY] {AVX512_F+VL}
    kIdVpmaxsw,                          // [ANY] {AVX|AVX2|AVX512_BW+VL}
    kIdVpmaxub,                          // [ANY] {AVX|AVX2|AVX512_BW+VL}
    kIdVpmaxud,                          // [ANY] {AVX|AVX2|AVX512_F+VL}
    kIdVpmaxuq,                          // [ANY] {AVX512_F+VL}
    kIdVpmaxuw,                          // [ANY] {AVX|AVX2|AVX512_BW+VL}
    kIdVpminsb,                          // [ANY] {AVX|AVX2|AVX512_BW+VL}
    kIdVpminsd,                          // [ANY] {AVX|AVX2|AVX512_F+VL}
    kIdVpminsq,                          // [ANY] {AVX512_F+VL}
    kIdVpminsw,                          // [ANY] {AVX|AVX2|AVX512_BW+VL}
    kIdVpminub,                          // [ANY] {AVX|AVX2|AVX512_BW+VL}
    kIdVpminud,                          // [ANY] {AVX|AVX2|AVX512_F+VL}
    kIdVpminuq,                          // [ANY] {AVX512_F+VL}
    kIdVpminuw,                          // [ANY] {AVX|AVX2|AVX512_BW+VL}
    kIdVpmovb2m,                         // [ANY] {AVX512_BW+VL}
    kIdVpmovd2m,                         // [ANY] {AVX512_DQ+VL}
    kIdVpmovdb,                          // [ANY] {AVX512_F+VL}
    kIdVpmovdw,                          // [ANY] {AVX512_F+VL}
    kIdVpmovm2b,                         // [ANY] {AVX512_BW+VL}
    kIdVpmovm2d,                         // [ANY] {AVX512_DQ+VL}
    kIdVpmovm2q,                         // [ANY] {AVX512_DQ+VL}
    kIdVpmovm2w,                         // [ANY] {AVX512_BW+VL}
    kIdVpmovmskb,                        // [ANY] {AVX|AVX2}
    kIdVpmovq2m,                         // [ANY] {AVX512_DQ+VL}
    kIdVpmovqb,                          // [ANY] {AVX512_F+VL}
    kIdVpmovqd,                          // [ANY] {AVX512_F+VL}
    kIdVpmovqw,                          // [ANY] {AVX512_F+VL}
    kIdVpmovsdb,                         // [ANY] {AVX512_F+VL}
    kIdVpmovsdw,                         // [ANY] {AVX512_F+VL}
    kIdVpmovsqb,                         // [ANY] {AVX512_F+VL}
    kIdVpmovsqd,                         // [ANY] {AVX512_F+VL}
    kIdVpmovsqw,                         // [ANY] {AVX512_F+VL}
    kIdVpmovswb,                         // [ANY] {AVX512_BW+VL}
    kIdVpmovsxbd,                        // [ANY] {AVX|AVX2|AVX512_F+VL}
    kIdVpmovsxbq,                        // [ANY] {AVX|AVX2|AVX512_F+VL}
    kIdVpmovsxbw,                        // [ANY] {AVX|AVX2|AVX512_BW+VL}
    kIdVpmovsxdq,                        // [ANY] {AVX|AVX2|AVX512_F+VL}
    kIdVpmovsxwd,                        // [ANY] {AVX|AVX2|AVX512_F+VL}
    kIdVpmovsxwq,                        // [ANY] {AVX|AVX2|AVX512_F+VL}
    kIdVpmovusdb,                        // [ANY] {AVX512_F+VL}
    kIdVpmovusdw,                        // [ANY] {AVX512_F+VL}
    kIdVpmovusqb,                        // [ANY] {AVX512_F+VL}
    kIdVpmovusqd,                        // [ANY] {AVX512_F+VL}
    kIdVpmovusqw,                        // [ANY] {AVX512_F+VL}
    kIdVpmovuswb,                        // [ANY] {AVX512_BW+VL}
    kIdVpmovw2m,                         // [ANY] {AVX512_BW+VL}
    kIdVpmovwb,                          // [ANY] {AVX512_BW+VL}
    kIdVpmovzxbd,                        // [ANY] {AVX|AVX2|AVX512_F+VL}
    kIdVpmovzxbq,                        // [ANY] {AVX|AVX2|AVX512_F+VL}
    kIdVpmovzxbw,                        // [ANY] {AVX|AVX2|AVX512_BW+VL}
    kIdVpmovzxdq,                        // [ANY] {AVX|AVX2|AVX512_F+VL}
    kIdVpmovzxwd,                        // [ANY] {AVX|AVX2|AVX512_F+VL}
    kIdVpmovzxwq,                        // [ANY] {AVX|AVX2|AVX512_F+VL}
    kIdVpmuldq,                          // [ANY] {AVX|AVX2|AVX512_F+VL}
    kIdVpmulhrsw,                        // [ANY] {AVX|AVX2|AVX512_BW+VL}
    kIdVpmulhuw,                         // [ANY] {AVX|AVX2|AVX512_BW+VL}
    kIdVpmulhw,                          // [ANY] {AVX|AVX2|AVX512_BW+VL}
    kIdVpmulld,                          // [ANY] {AVX|AVX2|AVX512_F+VL}
    kIdVpmullq,                          // [ANY] {AVX512_DQ+VL}
    kIdVpmullw,                          // [ANY] {AVX|AVX2|AVX512_BW+VL}
    kIdVpmultishiftqb,                   // [ANY] {AVX512_VBMI+VL}
    kIdVpmuludq,                         // [ANY] {AVX|AVX2|AVX512_F+VL}
    kIdVpopcntb,                         // [ANY] {AVX512_BITALG+VL}
    kIdVpopcntd,                         // [ANY] {AVX512_VPOPCNTDQ+VL}
    kIdVpopcntq,                         // [ANY] {AVX512_VPOPCNTDQ+VL}
    kIdVpopcntw,                         // [ANY] {AVX512_BITALG+VL}
    kIdVpor,                             // [ANY] {AVX|AVX2}
    kIdVpord,                            // [ANY] {AVX512_F+VL}
    kIdVporq,                            // [ANY] {AVX512_F+VL}
    kIdVpperm,                           // [ANY] {XOP}
    kIdVprold,                           // [ANY] {AVX512_F+VL}
    kIdVprolq,                           // [ANY] {AVX512_F+VL}
    kIdVprolvd,                          // [ANY] {AVX512_F+VL}
    kIdVprolvq,                          // [ANY] {AVX512_F+VL}
    kIdVprord,                           // [ANY] {AVX512_F+VL}
    kIdVprorq,                           // [ANY] {AVX512_F+VL}
    kIdVprorvd,                          // [ANY] {AVX512_F+VL}
    kIdVprorvq,                          // [ANY] {AVX512_F+VL}
    kIdVprotb,                           // [ANY] {XOP}
    kIdVprotd,                           // [ANY] {XOP}
    kIdVprotq,                           // [ANY] {XOP}
    kIdVprotw,                           // [ANY] {XOP}
    kIdVpsadbw,                          // [ANY] {AVX|AVX2|AVX512_BW+VL}
    kIdVpscatterdd,                      // [ANY] {AVX512_F+VL}
    kIdVpscatterdq,                      // [ANY] {AVX512_F+VL}
    kIdVpscatterqd,                      // [ANY] {AVX512_F+VL}
    kIdVpscatterqq,                      // [ANY] {AVX512_F+VL}
    kIdVpshab,                           // [ANY] {XOP}
    kIdVpshad,                           // [ANY] {XOP}
    kIdVpshaq,                           // [ANY] {XOP}
    kIdVpshaw,                           // [ANY] {XOP}
    kIdVpshlb,                           // [ANY] {XOP}
    kIdVpshld,                           // [ANY] {XOP}
    kIdVpshldd,                          // [ANY] {AVX512_VBMI2+VL}
    kIdVpshldq,                          // [ANY] {AVX512_VBMI2+VL}
    kIdVpshldvd,                         // [ANY] {AVX512_VBMI2+VL}
    kIdVpshldvq,                         // [ANY] {AVX512_VBMI2+VL}
    kIdVpshldvw,                         // [ANY] {AVX512_VBMI2+VL}
    kIdVpshldw,                          // [ANY] {AVX512_VBMI2+VL}
    kIdVpshlq,                           // [ANY] {XOP}
    kIdVpshlw,                           // [ANY] {XOP}
    kIdVpshrdd,                          // [ANY] {AVX512_VBMI2+VL}
    kIdVpshrdq,                          // [ANY] {AVX512_VBMI2+VL}
    kIdVpshrdvd,                         // [ANY] {AVX512_VBMI2+VL}
    kIdVpshrdvq,                         // [ANY] {AVX512_VBMI2+VL}
    kIdVpshrdvw,                         // [ANY] {AVX512_VBMI2+VL}
    kIdVpshrdw,                          // [ANY] {AVX512_VBMI2+VL}
    kIdVpshufb,                          // [ANY] {AVX|AVX2|AVX512_BW+VL}
    kIdVpshufbitqmb,                     // [ANY] {AVX512_BITALG+VL}
    kIdVpshufd,                          // [ANY] {AVX|AVX2|AVX512_F+VL}
    kIdVpshufhw,                         // [ANY] {AVX|AVX2|AVX512_BW+VL}
    kIdVpshuflw,                         // [ANY] {AVX|AVX2|AVX512_BW+VL}
    kIdVpsignb,                          // [ANY] {AVX|AVX2}
    kIdVpsignd,                          // [ANY] {AVX|AVX2}
    kIdVpsignw,                          // [ANY] {AVX|AVX2}
    kIdVpslld,                           // [ANY] {AVX|AVX2|AVX512_F+VL}
    kIdVpslldq,                          // [ANY] {AVX|AVX2|AVX512_BW+VL}
    kIdVpsllq,                           // [ANY] {AVX|AVX2|AVX512_F+VL}
    kIdVpsllvd,                          // [ANY] {AVX2|AVX512_F+VL}
    kIdVpsllvq,                          // [ANY] {AVX2|AVX512_F+VL}
    kIdVpsllvw,                          // [ANY] {AVX512_BW+VL}
    kIdVpsllw,                           // [ANY] {AVX|AVX2|AVX512_BW+VL}
    kIdVpsrad,                           // [ANY] {AVX|AVX2|AVX512_F+VL}
    kIdVpsraq,                           // [ANY] {AVX512_F+VL}
    kIdVpsravd,                          // [ANY] {AVX2|AVX512_F+VL}
    kIdVpsravq,                          // [ANY] {AVX512_F+VL}
    kIdVpsravw,                          // [ANY] {AVX512_BW+VL}
    kIdVpsraw,                           // [ANY] {AVX|AVX2|AVX512_BW+VL}
    kIdVpsrld,                           // [ANY] {AVX|AVX2|AVX512_F+VL}
    kIdVpsrldq,                          // [ANY] {AVX|AVX2|AVX512_BW+VL}
    kIdVpsrlq,                           // [ANY] {AVX|AVX2|AVX512_F+VL}
    kIdVpsrlvd,                          // [ANY] {AVX2|AVX512_F+VL}
    kIdVpsrlvq,                          // [ANY] {AVX2|AVX512_F+VL}
    kIdVpsrlvw,                          // [ANY] {AVX512_BW+VL}
    kIdVpsrlw,                           // [ANY] {AVX|AVX2|AVX512_BW+VL}
    kIdVpsubb,                           // [ANY] {AVX|AVX2|AVX512_BW+VL}
    kIdVpsubd,                           // [ANY] {AVX|AVX2|AVX512_F+VL}
    kIdVpsubq,                           // [ANY] {AVX|AVX2|AVX512_F+VL}
    kIdVpsubsb,                          // [ANY] {AVX|AVX2|AVX512_BW+VL}
    kIdVpsubsw,                          // [ANY] {AVX|AVX2|AVX512_BW+VL}
    kIdVpsubusb,                         // [ANY] {AVX|AVX2|AVX512_BW+VL}
    kIdVpsubusw,                         // [ANY] {AVX|AVX2|AVX512_BW+VL}
    kIdVpsubw,                           // [ANY] {AVX|AVX2|AVX512_BW+VL}
    kIdVpternlogd,                       // [ANY] {AVX512_F+VL}
    kIdVpternlogq,                       // [ANY] {AVX512_F+VL}
    kIdVptest,                           // [ANY] {AVX}
    kIdVptestmb,                         // [ANY] {AVX512_BW+VL}
    kIdVptestmd,                         // [ANY] {AVX512_F+VL}
    kIdVptestmq,                         // [ANY] {AVX512_F+VL}
    kIdVptestmw,                         // [ANY] {AVX512_BW+VL}
    kIdVptestnmb,                        // [ANY] {AVX512_BW+VL}
    kIdVptestnmd,                        // [ANY] {AVX512_F+VL}
    kIdVptestnmq,                        // [ANY] {AVX512_F+VL}
    kIdVptestnmw,                        // [ANY] {AVX512_BW+VL}
    kIdVpunpckhbw,                       // [ANY] {AVX|AVX2|AVX512_BW+VL}
    kIdVpunpckhdq,                       // [ANY] {AVX|AVX2|AVX512_F+VL}
    kIdVpunpckhqdq,                      // [ANY] {AVX|AVX2|AVX512_F+VL}
    kIdVpunpckhwd,                       // [ANY] {AVX|AVX2|AVX512_BW+VL}
    kIdVpunpcklbw,                       // [ANY] {AVX|AVX2|AVX512_BW+VL}
    kIdVpunpckldq,                       // [ANY] {AVX|AVX2|AVX512_F+VL}
    kIdVpunpcklqdq,                      // [ANY] {AVX|AVX2|AVX512_F+VL}
    kIdVpunpcklwd,                       // [ANY] {AVX|AVX2|AVX512_BW+VL}
    kIdVpxor,                            // [ANY] {AVX|AVX2}
    kIdVpxord,                           // [ANY] {AVX512_F+VL}
    kIdVpxorq,                           // [ANY] {AVX512_F+VL}
    kIdVrangepd,                         // [ANY] {AVX512_DQ+VL}
    kIdVrangeps,                         // [ANY] {AVX512_DQ+VL}
    kIdVrangesd,                         // [ANY] {AVX512_DQ}
    kIdVrangess,                         // [ANY] {AVX512_DQ}
    kIdVrcp14pd,                         // [ANY] {AVX512_F+VL}
    kIdVrcp14ps,                         // [ANY] {AVX512_F+VL}
    kIdVrcp14sd,                         // [ANY] {AVX512_F}
    kIdVrcp14ss,                         // [ANY] {AVX512_F}
    kIdVrcp28pd,                         // [ANY] {AVX512_ERI}
    kIdVrcp28ps,                         // [ANY] {AVX512_ERI}
    kIdVrcp28sd,                         // [ANY] {AVX512_ERI}
    kIdVrcp28ss,                         // [ANY] {AVX512_ERI}
    kIdVrcpps,                           // [ANY] {AVX}
    kIdVrcpss,                           // [ANY] {AVX}
    kIdVreducepd,                        // [ANY] {AVX512_DQ+VL}
    kIdVreduceps,                        // [ANY] {AVX512_DQ+VL}
    kIdVreducesd,                        // [ANY] {AVX512_DQ}
    kIdVreducess,                        // [ANY] {AVX512_DQ}
    kIdVrndscalepd,                      // [ANY] {AVX512_F+VL}
    kIdVrndscaleps,                      // [ANY] {AVX512_F+VL}
    kIdVrndscalesd,                      // [ANY] {AVX512_F}
    kIdVrndscaless,                      // [ANY] {AVX512_F}
    kIdVroundpd,                         // [ANY] {AVX}
    kIdVroundps,                         // [ANY] {AVX}
    kIdVroundsd,                         // [ANY] {AVX}
    kIdVroundss,                         // [ANY] {AVX}
    kIdVrsqrt14pd,                       // [ANY] {AVX512_F+VL}
    kIdVrsqrt14ps,                       // [ANY] {AVX512_F+VL}
    kIdVrsqrt14sd,                       // [ANY] {AVX512_F}
    kIdVrsqrt14ss,                       // [ANY] {AVX512_F}
    kIdVrsqrt28pd,                       // [ANY] {AVX512_ERI}
    kIdVrsqrt28ps,                       // [ANY] {AVX512_ERI}
    kIdVrsqrt28sd,                       // [ANY] {AVX512_ERI}
    kIdVrsqrt28ss,                       // [ANY] {AVX512_ERI}
    kIdVrsqrtps,                         // [ANY] {AVX}
    kIdVrsqrtss,                         // [ANY] {AVX}
    kIdVscalefpd,                        // [ANY] {AVX512_F+VL}
    kIdVscalefps,                        // [ANY] {AVX512_F+VL}
    kIdVscalefsd,                        // [ANY] {AVX512_F}
    kIdVscalefss,                        // [ANY] {AVX512_F}
    kIdVscatterdpd,                      // [ANY] {AVX512_F+VL}
    kIdVscatterdps,                      // [ANY] {AVX512_F+VL}
    kIdVscatterpf0dpd,                   // [ANY] {AVX512_PFI}
    kIdVscatterpf0dps,                   // [ANY] {AVX512_PFI}
    kIdVscatterpf0qpd,                   // [ANY] {AVX512_PFI}
    kIdVscatterpf0qps,                   // [ANY] {AVX512_PFI}
    kIdVscatterpf1dpd,                   // [ANY] {AVX512_PFI}
    kIdVscatterpf1dps,                   // [ANY] {AVX512_PFI}
    kIdVscatterpf1qpd,                   // [ANY] {AVX512_PFI}
    kIdVscatterpf1qps,                   // [ANY] {AVX512_PFI}
    kIdVscatterqpd,                      // [ANY] {AVX512_F+VL}
    kIdVscatterqps,                      // [ANY] {AVX512_F+VL}
    kIdVshuff32x4,                       // [ANY] {AVX512_F+VL}
    kIdVshuff64x2,                       // [ANY] {AVX512_F+VL}
    kIdVshufi32x4,                       // [ANY] {AVX512_F+VL}
    kIdVshufi64x2,                       // [ANY] {AVX512_F+VL}
    kIdVshufpd,                          // [ANY] {AVX|AVX512_F+VL}
    kIdVshufps,                          // [ANY] {AVX|AVX512_F+VL}
    kIdVsqrtpd,                          // [ANY] {AVX|AVX512_F+VL}
    kIdVsqrtps,                          // [ANY] {AVX|AVX512_F+VL}
    kIdVsqrtsd,                          // [ANY] {AVX|AVX512_F}
    kIdVsqrtss,                          // [ANY] {AVX|AVX512_F}
    kIdVstmxcsr,                         // [ANY] {AVX}
    kIdVsubpd,                           // [ANY] {AVX|AVX512_F+VL}
    kIdVsubps,                           // [ANY] {AVX|AVX512_F+VL}
    kIdVsubsd,                           // [ANY] {AVX|AVX512_F}
    kIdVsubss,                           // [ANY] {AVX|AVX512_F}
    kIdVtestpd,                          // [ANY] {AVX}
    kIdVtestps,                          // [ANY] {AVX}
    kIdVucomisd,                         // [ANY] {AVX|AVX512_F}
    kIdVucomiss,                         // [ANY] {AVX|AVX512_F}
    kIdVunpckhpd,                        // [ANY] {AVX|AVX512_F+VL}
    kIdVunpckhps,                        // [ANY] {AVX|AVX512_F+VL}
    kIdVunpcklpd,                        // [ANY] {AVX|AVX512_F+VL}
    kIdVunpcklps,                        // [ANY] {AVX|AVX512_F+VL}
    kIdVxorpd,                           // [ANY] {AVX|AVX512_DQ+VL}
    kIdVxorps,                           // [ANY] {AVX|AVX512_DQ+VL}
    kIdVzeroall,                         // [ANY] {AVX}
    kIdVzeroupper,                       // [ANY] {AVX}
    kIdWbinvd,                           // [ANY]
    kIdWrfsbase,                         // [X64] {FSGSBASE}
    kIdWrgsbase,                         // [X64] {FSGSBASE}
    kIdWrmsr,                            // [ANY] {MSR}
    kIdXabort,                           // [ANY] {RTM}
    kIdXadd,                             // [ANY] {I486}
    kIdXbegin,                           // [ANY] {RTM}
    kIdXchg,                             // [ANY]
    kIdXend,                             // [ANY] {RTM}
    kIdXgetbv,                           // [ANY] {XSAVE}
    kIdXlatb,                            // [ANY]
    kIdXor,                              // [ANY]
    kIdXorpd,                            // [ANY] {SSE2}
    kIdXorps,                            // [ANY] {SSE}
    kIdXrstor,                           // [ANY] {XSAVE}
    kIdXrstor64,                         // [X64] {XSAVE}
    kIdXrstors,                          // [ANY] {XSAVES}
    kIdXrstors64,                        // [X64] {XSAVES}
    kIdXsave,                            // [ANY] {XSAVE}
    kIdXsave64,                          // [X64] {XSAVE}
    kIdXsavec,                           // [ANY] {XSAVEC}
    kIdXsavec64,                         // [X64] {XSAVEC}
    kIdXsaveopt,                         // [ANY] {XSAVEOPT}
    kIdXsaveopt64,                       // [X64] {XSAVEOPT}
    kIdXsaves,                           // [ANY] {XSAVES}
    kIdXsaves64,                         // [X64] {XSAVES}
    kIdXsetbv,                           // [ANY] {XSAVE}
    kIdXtest,                            // [ANY] {TSX}
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

  //! Get whether the `instId` is defined (counts also Inst::kIdNone, which must be zero).
  static inline bool isDefinedId(uint32_t instId) noexcept { return instId < _kIdCount; }

  //! Get a 'kmov?' instruction from a `size`.
  static inline uint32_t kmovFromSize(uint32_t size) noexcept {
    switch (size) {
      case  1: return kIdKmovb;
      case  2: return kIdKmovw;
      case  4: return kIdKmovd;
      case  8: return kIdKmovq;
      default: return kIdNone;
    }
  }

  #define ASMJIT_INST_FROM_COND(ID) \
    ID##o, ID##no, ID##b , ID##ae,  \
    ID##e, ID##ne, ID##be, ID##a ,  \
    ID##s, ID##ns, ID##pe, ID##po,  \
    ID##l, ID##ge, ID##le, ID##g
  static constexpr uint16_t jccTable[] = { ASMJIT_INST_FROM_COND(Inst::kIdJ) };
  static constexpr uint16_t setccTable[] = { ASMJIT_INST_FROM_COND(Inst::kIdSet) };
  static constexpr uint16_t cmovccTable[] = { ASMJIT_INST_FROM_COND(Inst::kIdCmov) };
  #undef ASMJIT_INST_FROM_COND

  //! Translate a condition code `cond` to a `jcc` instruction id.
  static constexpr uint32_t jccFromCond(uint32_t cond) noexcept { return jccTable[cond]; }
  //! Translate a condition code `cond` to a `setcc` instruction id.
  static constexpr uint32_t setccFromCond(uint32_t cond) noexcept { return setccTable[cond]; }
  //! Translate a condition code `cond` to a `cmovcc` instruction id.
  static constexpr uint32_t cmovccFromCond(uint32_t cond) noexcept { return cmovccTable[cond]; }
};

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
// [asmjit::x86::SpecialRegs]
// ============================================================================

// TODO: Move into a namespace.
//! Flags describing special registers and/or their parts.
enum SpecialRegs : uint32_t {
  kSpecialReg_FLAGS_CF    = 0x00000001u, //!< [R|E]FLAGS - Carry flag.
  kSpecialReg_FLAGS_PF    = 0x00000002u, //!< [R|E]FLAGS - Parity flag.
  kSpecialReg_FLAGS_AF    = 0x00000004u, //!< [R|E]FLAGS - Adjust flag.
  kSpecialReg_FLAGS_ZF    = 0x00000008u, //!< [R|E]FLAGS - Zero flag.
  kSpecialReg_FLAGS_SF    = 0x00000010u, //!< [R|E]FLAGS - Sign flag.
  kSpecialReg_FLAGS_TF    = 0x00000020u, //!< [R|E]FLAGS - Trap flag.
  kSpecialReg_FLAGS_IF    = 0x00000040u, //!< [R|E]FLAGS - Interrupt enable flag.
  kSpecialReg_FLAGS_DF    = 0x00000080u, //!< [R|E]FLAGS - Direction flag.
  kSpecialReg_FLAGS_OF    = 0x00000100u, //!< [R|E]FLAGS - Overflow flag.
  kSpecialReg_FLAGS_AC    = 0x00000200u, //!< [R|E]FLAGS - Alignment check.
  kSpecialReg_FLAGS_SYS   = 0x00000400u, //!< [R|E]FLAGS - System flags.

  kSpecialReg_X87CW_EXC   = 0x00000800u, //!< X87 Control Word - Exception control.
  kSpecialReg_X87CW_PC    = 0x00001000u, //!< X87 Control Word - Precision control.
  kSpecialReg_X87CW_RC    = 0x00002000u, //!< X87 Control Word - Rounding control.

  kSpecialReg_X87SW_EXC   = 0x00004000u, //!< X87 Status Word - Exception flags.
  kSpecialReg_X87SW_C0    = 0x00008000u, //!< X87 Status Word - C0 flag.
  kSpecialReg_X87SW_C1    = 0x00010000u, //!< X87 Status Word - C1 flag.
  kSpecialReg_X87SW_C2    = 0x00020000u, //!< X87 Status Word - C2 flag.
  kSpecialReg_X87SW_TOP   = 0x00040000u, //!< X87 Status Word - Top of the FPU stack.
  kSpecialReg_X87SW_C3    = 0x00080000u, //!< X87 Status Word - C3 flag.

  kSpecialReg_MSR         = 0x00100000u, //!< MSR register.
  kSpecialReg_XCR         = 0x00200000u  //!< XCR register.
};

// ============================================================================
// [asmjit::x86::Predicate]
// ============================================================================

//! Contains predicates used by SIMD instructions (X86).
namespace Predicate {
  //! A predicate used by CMP[PD|PS|SD|SS] instructions.
  enum Cmp : uint32_t {
    kCmpEQ                = 0x00u,       //!< Equal             (Quiet).
    kCmpLT                = 0x01u,       //!< Less              (Signaling).
    kCmpLE                = 0x02u,       //!< Less/Equal        (Signaling).
    kCmpUNORD             = 0x03u,       //!< Unordered         (Quiet).
    kCmpNEQ               = 0x04u,       //!< Not Equal         (Quiet).
    kCmpNLT               = 0x05u,       //!< Not Less          (Signaling).
    kCmpNLE               = 0x06u,       //!< Not Less/Equal    (Signaling).
    kCmpORD               = 0x07u        //!< Ordered           (Quiet).
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
    kPCmpStrRanges        = 0x01u << 2,  //!< The arithmetic comparison is “greater than or equal”
                                         //!< between even indexed elements and “less than or equal”
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
  //!   - `shufpd`
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
  //!   - `pshufw()`
  //!   - `pshufd()`
  //!   - `pshuflw()`
  //!   - `pshufhw()`
  //!   - `shufps()`
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

  //! Create an immediate that can be used by VPTERNLOG[D|Q] instructions.
  static constexpr uint32_t make(uint32_t b000, uint32_t b001, uint32_t b010, uint32_t b011, uint32_t b100, uint32_t b101, uint32_t b110, uint32_t b111) noexcept {
    return (b000 << 0) | (b001 << 1) | (b010 << 2) | (b011 << 3) | (b100 << 4) | (b101 << 5) | (b110 << 6) | (b111 << 7);
  }

  //! Create an immediate that can be used by VPTERNLOG[D|Q] instructions.
  static constexpr uint32_t value(uint32_t x) noexcept { return x & 0xFF; }
  //! Negate an immediate that can be used by VPTERNLOG[D|Q] instructions.
  static constexpr uint32_t negate(uint32_t x) noexcept { return x ^ 0xFF; }
  //! Create an if/else logic that can be used by VPTERNLOG[D|Q] instructions.
  static constexpr uint32_t ifElse(uint32_t condition, uint32_t a, uint32_t b) noexcept { return (condition & a) | (negate(condition) & b); }
}

//! \}

ASMJIT_END_SUB_NAMESPACE

// [Guard]
#endif // _ASMJIT_X86_X86GLOBALS_H
