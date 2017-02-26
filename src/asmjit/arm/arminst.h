// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_ARM_ARMINST_H
#define _ASMJIT_ARM_ARMINST_H

// [Dependencies]
#include "../core/assembler.h"
#include "../core/operand.h"
#include "../arm/armglobals.h"

ASMJIT_BEGIN_NAMESPACE

//! \addtogroup asmjit_arm
//! \{

// ============================================================================
// [asmjit::ArmInst]
// ============================================================================

//! ARM instruction data (ARM32 and/or THUMBx).
struct ArmInst {
  //! Instruction id.
  //!
  //! Each instruction has a unique ID that is used as an index to AsmJit's
  //! instruction table. Instructions are sorted alphabetically.
  enum Id : uint32_t {
    // ${idData:Begin}
    kIdNone = 0,                         // [--- --- ---]
    kIdAdc,                              // [Txx A32 ---]
    kIdAdcs,                             // [Txx A32 ---]
    kIdAdd,                              // [Txx A32 ---]
    kIdAdds,                             // [Txx A32 ---]
    kIdAdr,                              // [Txx A32 ---]
    kIdAesd,                             // [T32 A32 ---] {AES}
    kIdAese,                             // [T32 A32 ---] {AES}
    kIdAesimc,                           // [T32 A32 ---] {AES}
    kIdAesmc,                            // [T32 A32 ---] {AES}
    kIdAnd,                              // [Txx A32 ---]
    kIdAnds,                             // [Txx A32 ---]
    kIdAsr,                              // [Txx A32 ---]
    kIdAsrs,                             // [Txx A32 ---]
    kIdB,                                // [Txx A32 ---]
    kIdBfc,                              // [T32 A32 ---]
    kIdBfi,                              // [T32 A32 ---]
    kIdBic,                              // [Txx A32 ---]
    kIdBics,                             // [Txx A32 ---]
    kIdBkpt,                             // [T16 A32 ---]
    kIdBl,                               // [T32 A32 ---]
    kIdBlx,                              // [Txx A32 ---]
    kIdBx,                               // [T16 A32 ---]
    kIdBxj,                              // [T32 A32 ---]
    kIdCbnz,                             // [T16 --- ---]
    kIdCbz,                              // [T16 --- ---]
    kIdClrex,                            // [T32 A32 ---]
    kIdClz,                              // [T32 A32 ---]
    kIdCmn,                              // [Txx A32 ---]
    kIdCmp,                              // [Txx A32 ---]
    kIdCps,                              // [--- A32 ---]
    kIdCpsid,                            // [--- A32 ---]
    kIdCpsie,                            // [--- A32 ---]
    kIdCrc32b,                           // [T32 A32 ---] {CRC32}
    kIdCrc32cb,                          // [T32 A32 ---] {CRC32}
    kIdCrc32ch,                          // [T32 A32 ---] {CRC32}
    kIdCrc32cw,                          // [T32 A32 ---] {CRC32}
    kIdCrc32h,                           // [T32 A32 ---] {CRC32}
    kIdCrc32w,                           // [T32 A32 ---] {CRC32}
    kIdDbg,                              // [T32 A32 ---]
    kIdDmb,                              // [T32 A32 ---]
    kIdDsb,                              // [T32 A32 ---]
    kIdEor,                              // [Txx A32 ---]
    kIdEors,                             // [Txx A32 ---]
    kIdEret,                             // [--- A32 ---]
    kIdFldmdbx,                          // [T32 A32 ---]
    kIdFldmiax,                          // [T32 A32 ---]
    kIdFstmdbx,                          // [T32 A32 ---]
    kIdFstmiax,                          // [T32 A32 ---]
    kIdHlt,                              // [--- A32 ---]
    kIdHvc,                              // [--- A32 ---]
    kIdIsb,                              // [T32 A32 ---]
    kIdIt,                               // [T16 --- ---]
    kIdIte,                              // [T16 --- ---]
    kIdItee,                             // [T16 --- ---]
    kIdIteee,                            // [T16 --- ---]
    kIdIteet,                            // [T16 --- ---]
    kIdItet,                             // [T16 --- ---]
    kIdItete,                            // [T16 --- ---]
    kIdItett,                            // [T16 --- ---]
    kIdItt,                              // [T16 --- ---]
    kIdItte,                             // [T16 --- ---]
    kIdIttee,                            // [T16 --- ---]
    kIdIttet,                            // [T16 --- ---]
    kIdIttt,                             // [T16 --- ---]
    kIdIttte,                            // [T16 --- ---]
    kIdItttt,                            // [T16 --- ---]
    kIdLda,                              // [T32 A32 ---]
    kIdLdab,                             // [T32 A32 ---]
    kIdLdaex,                            // [T32 A32 ---]
    kIdLdaexb,                           // [T32 A32 ---]
    kIdLdaexd,                           // [T32 A32 ---]
    kIdLdaexh,                           // [T32 A32 ---]
    kIdLdah,                             // [T32 A32 ---]
    kIdLdm,                              // [Txx A32 ---]
    kIdLdmda,                            // [--- A32 ---]
    kIdLdmdb,                            // [T32 A32 ---]
    kIdLdmib,                            // [--- A32 ---]
    kIdLdr,                              // [Txx A32 ---]
    kIdLdrb,                             // [Txx A32 ---]
    kIdLdrbt,                            // [T32 A32 ---]
    kIdLdrd,                             // [T32 A32 ---]
    kIdLdrex,                            // [T32 A32 ---]
    kIdLdrexb,                           // [T32 A32 ---]
    kIdLdrexd,                           // [T32 A32 ---]
    kIdLdrexh,                           // [T32 A32 ---]
    kIdLdrh,                             // [Txx A32 ---]
    kIdLdrht,                            // [T32 A32 ---]
    kIdLdrsb,                            // [Txx A32 ---]
    kIdLdrsbt,                           // [T32 A32 ---]
    kIdLdrsh,                            // [Txx A32 ---]
    kIdLdrsht,                           // [T32 A32 ---]
    kIdLdrt,                             // [T32 A32 ---]
    kIdLsl,                              // [Txx A32 ---]
    kIdLsls,                             // [Txx A32 ---]
    kIdLsr,                              // [Txx A32 ---]
    kIdLsrs,                             // [Txx A32 ---]
    kIdMcr,                              // [T32 A32 ---]
    kIdMcr2,                             // [T32 A32 ---]
    kIdMcrr,                             // [T32 A32 ---]
    kIdMcrr2,                            // [T32 A32 ---]
    kIdMla,                              // [T32 A32 ---]
    kIdMlas,                             // [--- A32 ---]
    kIdMls,                              // [T32 A32 ---]
    kIdMov,                              // [Txx A32 ---]
    kIdMovs,                             // [Txx A32 ---]
    kIdMovt,                             // [T32 A32 ---]
    kIdMovw,                             // [T32 A32 ---]
    kIdMrc,                              // [T32 A32 ---]
    kIdMrc2,                             // [T32 A32 ---]
    kIdMrrc,                             // [T32 A32 ---]
    kIdMrrc2,                            // [T32 A32 ---]
    kIdMrs,                              // [T32 A32 ---]
    kIdMsr,                              // [T32 A32 ---]
    kIdMul,                              // [Txx A32 ---]
    kIdMuls,                             // [T16 A32 ---]
    kIdMvn,                              // [Txx A32 ---]
    kIdMvns,                             // [Txx A32 ---]
    kIdNop,                              // [Txx A32 ---]
    kIdOrn,                              // [T32 --- ---]
    kIdOrns,                             // [T32 --- ---]
    kIdOrr,                              // [Txx A32 ---]
    kIdOrrs,                             // [Txx A32 ---]
    kIdPkhbt,                            // [T32 A32 ---]
    kIdPkhtb,                            // [T32 A32 ---]
    kIdPld,                              // [T32 A32 ---]
    kIdPldw,                             // [T32 A32 ---]
    kIdPli,                              // [T32 A32 ---]
    kIdPop,                              // [Txx A32 ---]
    kIdPush,                             // [Txx A32 ---]
    kIdQadd,                             // [T32 A32 ---]
    kIdQadd16,                           // [T32 A32 ---]
    kIdQadd8,                            // [T32 A32 ---]
    kIdQasx,                             // [T32 A32 ---]
    kIdQdadd,                            // [T32 A32 ---]
    kIdQdsub,                            // [T32 A32 ---]
    kIdQsax,                             // [T32 A32 ---]
    kIdQsub,                             // [T32 A32 ---]
    kIdQsub16,                           // [T32 A32 ---]
    kIdQsub8,                            // [T32 A32 ---]
    kIdRbit,                             // [T32 A32 ---]
    kIdRev,                              // [Txx A32 ---]
    kIdRev16,                            // [Txx A32 ---]
    kIdRevsh,                            // [Txx A32 ---]
    kIdRfe,                              // [--- A32 ---]
    kIdRfeda,                            // [--- A32 ---]
    kIdRfedb,                            // [--- A32 ---]
    kIdRfeib,                            // [--- A32 ---]
    kIdRor,                              // [Txx A32 ---]
    kIdRors,                             // [Txx A32 ---]
    kIdRrx,                              // [T32 A32 ---]
    kIdRrxs,                             // [T32 A32 ---]
    kIdRsb,                              // [Txx A32 ---]
    kIdRsbs,                             // [Txx A32 ---]
    kIdRsc,                              // [--- A32 ---]
    kIdRscs,                             // [--- A32 ---]
    kIdSadd16,                           // [T32 A32 ---]
    kIdSadd8,                            // [T32 A32 ---]
    kIdSasx,                             // [T32 A32 ---]
    kIdSbc,                              // [Txx A32 ---]
    kIdSbcs,                             // [Txx A32 ---]
    kIdSbfx,                             // [T32 A32 ---]
    kIdSdiv,                             // [T32 A32 ---] {IDIVA|IDIVT}
    kIdSel,                              // [T32 A32 ---]
    kIdSetend,                           // [T16 A32 ---]
    kIdSev,                              // [Txx A32 ---]
    kIdSevl,                             // [--- A32 ---]
    kIdSha1c,                            // [T32 A32 ---] {SHA1}
    kIdSha1h,                            // [T32 A32 ---] {SHA1}
    kIdSha1m,                            // [T32 A32 ---] {SHA1}
    kIdSha1p,                            // [T32 A32 ---] {SHA1}
    kIdSha1su0,                          // [T32 A32 ---] {SHA1}
    kIdSha1su1,                          // [T32 A32 ---] {SHA1}
    kIdSha256h,                          // [T32 A32 ---] {SHA256}
    kIdSha256h2,                         // [T32 A32 ---] {SHA256}
    kIdSha256su0,                        // [T32 A32 ---] {SHA256}
    kIdSha256su1,                        // [T32 A32 ---] {SHA256}
    kIdShadd16,                          // [T32 A32 ---]
    kIdShadd8,                           // [T32 A32 ---]
    kIdShasx,                            // [T32 A32 ---]
    kIdShsax,                            // [T32 A32 ---]
    kIdShsub16,                          // [T32 A32 ---]
    kIdShsub8,                           // [T32 A32 ---]
    kIdSmc,                              // [--- A32 ---] {SECURITY}
    kIdSmlabb,                           // [T32 A32 ---]
    kIdSmlabt,                           // [T32 A32 ---]
    kIdSmlad,                            // [T32 A32 ---]
    kIdSmladx,                           // [T32 A32 ---]
    kIdSmlal,                            // [T32 A32 ---]
    kIdSmlalbb,                          // [T32 A32 ---]
    kIdSmlalbt,                          // [T32 A32 ---]
    kIdSmlald,                           // [T32 A32 ---]
    kIdSmlaldx,                          // [T32 A32 ---]
    kIdSmlals,                           // [--- A32 ---]
    kIdSmlaltb,                          // [T32 A32 ---]
    kIdSmlaltt,                          // [T32 A32 ---]
    kIdSmlatb,                           // [T32 A32 ---]
    kIdSmlatt,                           // [T32 A32 ---]
    kIdSmlawb,                           // [T32 A32 ---]
    kIdSmlawt,                           // [T32 A32 ---]
    kIdSmlsd,                            // [T32 A32 ---]
    kIdSmlsdx,                           // [T32 A32 ---]
    kIdSmlsld,                           // [T32 A32 ---]
    kIdSmlsldx,                          // [T32 A32 ---]
    kIdSmmla,                            // [T32 A32 ---]
    kIdSmmlar,                           // [T32 A32 ---]
    kIdSmmls,                            // [T32 A32 ---]
    kIdSmmlsr,                           // [T32 A32 ---]
    kIdSmmul,                            // [T32 A32 ---]
    kIdSmmulr,                           // [T32 A32 ---]
    kIdSmuad,                            // [T32 A32 ---]
    kIdSmuadx,                           // [T32 A32 ---]
    kIdSmulbb,                           // [T32 A32 ---]
    kIdSmulbt,                           // [T32 A32 ---]
    kIdSmull,                            // [T32 A32 ---]
    kIdSmulls,                           // [--- A32 ---]
    kIdSmultb,                           // [T32 A32 ---]
    kIdSmultt,                           // [T32 A32 ---]
    kIdSmulwb,                           // [T32 A32 ---]
    kIdSmulwt,                           // [T32 A32 ---]
    kIdSmusd,                            // [T32 A32 ---]
    kIdSmusdx,                           // [T32 A32 ---]
    kIdSrs,                              // [--- A32 ---]
    kIdSrsda,                            // [--- A32 ---]
    kIdSrsdb,                            // [--- A32 ---]
    kIdSrsib,                            // [--- A32 ---]
    kIdSsat,                             // [T32 A32 ---]
    kIdSsat16,                           // [T32 A32 ---]
    kIdSsax,                             // [T32 A32 ---]
    kIdSsub16,                           // [T32 A32 ---]
    kIdSsub8,                            // [T32 A32 ---]
    kIdStl,                              // [T32 A32 ---]
    kIdStlb,                             // [T32 A32 ---]
    kIdStlex,                            // [T32 A32 ---]
    kIdStlexb,                           // [T32 A32 ---]
    kIdStlexd,                           // [T32 A32 ---]
    kIdStlexh,                           // [T32 A32 ---]
    kIdStlh,                             // [T32 A32 ---]
    kIdStm,                              // [Txx A32 ---]
    kIdStmda,                            // [--- A32 ---]
    kIdStmdb,                            // [T32 A32 ---]
    kIdStmib,                            // [--- A32 ---]
    kIdStr,                              // [Txx A32 ---]
    kIdStrb,                             // [Txx A32 ---]
    kIdStrbt,                            // [T32 A32 ---]
    kIdStrd,                             // [T32 A32 ---]
    kIdStrex,                            // [T32 A32 ---]
    kIdStrexb,                           // [T32 A32 ---]
    kIdStrexd,                           // [T32 A32 ---]
    kIdStrexh,                           // [T32 A32 ---]
    kIdStrh,                             // [Txx A32 ---]
    kIdStrht,                            // [T32 A32 ---]
    kIdStrt,                             // [T32 A32 ---]
    kIdSub,                              // [Txx A32 ---]
    kIdSubs,                             // [Txx A32 ---]
    kIdSvc,                              // [T16 A32 ---]
    kIdSwp,                              // [--- A32 ---]
    kIdSwpb,                             // [--- A32 ---]
    kIdSxtab,                            // [T32 A32 ---]
    kIdSxtab16,                          // [T32 A32 ---]
    kIdSxtah,                            // [T32 A32 ---]
    kIdSxtb,                             // [Txx A32 ---]
    kIdSxtb16,                           // [T32 A32 ---]
    kIdSxth,                             // [Txx A32 ---]
    kIdTbb,                              // [T32 --- ---]
    kIdTbh,                              // [T32 --- ---]
    kIdTeq,                              // [T32 A32 ---]
    kIdTst,                              // [Txx A32 ---]
    kIdUadd16,                           // [T32 A32 ---]
    kIdUadd8,                            // [T32 A32 ---]
    kIdUasx,                             // [T32 A32 ---]
    kIdUbfx,                             // [T32 A32 ---]
    kIdUdf,                              // [--- A32 ---]
    kIdUdiv,                             // [T32 A32 ---] {IDIVA|IDIVT}
    kIdUhadd16,                          // [T32 A32 ---]
    kIdUhadd8,                           // [T32 A32 ---]
    kIdUhasx,                            // [T32 A32 ---]
    kIdUhsax,                            // [T32 A32 ---]
    kIdUhsub16,                          // [T32 A32 ---]
    kIdUhsub8,                           // [T32 A32 ---]
    kIdUmaal,                            // [T32 A32 ---]
    kIdUmlal,                            // [T32 A32 ---]
    kIdUmlals,                           // [--- A32 ---]
    kIdUmull,                            // [T32 A32 ---]
    kIdUmulls,                           // [--- A32 ---]
    kIdUqadd16,                          // [T32 A32 ---]
    kIdUqadd8,                           // [T32 A32 ---]
    kIdUqasx,                            // [T32 A32 ---]
    kIdUqsax,                            // [T32 A32 ---]
    kIdUqsub16,                          // [T32 A32 ---]
    kIdUqsub8,                           // [T32 A32 ---]
    kIdUsad8,                            // [T32 A32 ---]
    kIdUsada8,                           // [T32 A32 ---]
    kIdUsat,                             // [T32 A32 ---]
    kIdUsat16,                           // [T32 A32 ---]
    kIdUsax,                             // [T32 A32 ---]
    kIdUsub16,                           // [T32 A32 ---]
    kIdUsub8,                            // [T32 A32 ---]
    kIdUxtab,                            // [T32 A32 ---]
    kIdUxtab16,                          // [T32 A32 ---]
    kIdUxtah,                            // [T32 A32 ---]
    kIdUxtb,                             // [Txx A32 ---]
    kIdUxtb16,                           // [T32 A32 ---]
    kIdUxth,                             // [Txx A32 ---]
    kIdVaba,                             // [T32 A32 ---] {ASIMD}
    kIdVabal,                            // [T32 A32 ---] {ASIMD}
    kIdVabd,                             // [T32 A32 ---] {ASIMD}
    kIdVabdl,                            // [T32 A32 ---] {ASIMD}
    kIdVabs,                             // [T32 A32 ---] {ASIMD|VFPv2}
    kIdVacge,                            // [T32 A32 ---] {ASIMD}
    kIdVacgt,                            // [T32 A32 ---] {ASIMD}
    kIdVacle,                            // [T32 A32 ---] {ASIMD}
    kIdVaclt,                            // [T32 A32 ---] {ASIMD}
    kIdVadd,                             // [T32 A32 ---] {ASIMD|VFPv2}
    kIdVaddhn,                           // [T32 A32 ---] {ASIMD}
    kIdVaddl,                            // [T32 A32 ---] {ASIMD}
    kIdVaddw,                            // [T32 A32 ---] {ASIMD}
    kIdVand,                             // [T32 A32 ---] {ASIMD}
    kIdVbic,                             // [T32 A32 ---] {ASIMD}
    kIdVbif,                             // [T32 A32 ---] {ASIMD}
    kIdVbit,                             // [T32 A32 ---] {ASIMD}
    kIdVbsl,                             // [T32 A32 ---] {ASIMD}
    kIdVceq,                             // [T32 A32 ---] {ASIMD}
    kIdVcge,                             // [T32 A32 ---] {ASIMD}
    kIdVcgt,                             // [T32 A32 ---] {ASIMD}
    kIdVcle,                             // [T32 A32 ---] {ASIMD}
    kIdVcls,                             // [T32 A32 ---] {ASIMD}
    kIdVclt,                             // [T32 A32 ---] {ASIMD}
    kIdVclz,                             // [T32 A32 ---] {ASIMD}
    kIdVcmp,                             // [T32 A32 ---] {VFPv2}
    kIdVcmpe,                            // [T32 A32 ---] {VFPv2}
    kIdVcnt,                             // [T32 A32 ---] {ASIMD}
    kIdVcvt,                             // [T32 A32 ---] {ASIMD|VFPv2|VFPv3|VFPv3_FP16}
    kIdVcvta,                            // [T32 A32 ---]
    kIdVcvtb,                            // [T32 A32 ---] {VFPv3_FP16}
    kIdVcvtm,                            // [T32 A32 ---]
    kIdVcvtn,                            // [T32 A32 ---]
    kIdVcvtp,                            // [T32 A32 ---]
    kIdVcvtr,                            // [T32 A32 ---] {VFPv2}
    kIdVcvtt,                            // [T32 A32 ---] {VFPv3_FP16}
    kIdVdiv,                             // [T32 A32 ---] {VFPv2}
    kIdVdup,                             // [T32 A32 ---] {ASIMD}
    kIdVeor,                             // [T32 A32 ---] {ASIMD}
    kIdVext,                             // [T32 A32 ---] {ASIMD}
    kIdVfma,                             // [T32 A32 ---] {ASIMD|VFPv4}
    kIdVfms,                             // [T32 A32 ---] {ASIMD|VFPv4}
    kIdVfnma,                            // [T32 A32 ---] {VFPv4}
    kIdVfnms,                            // [T32 A32 ---] {VFPv4}
    kIdVhadd,                            // [T32 A32 ---] {ASIMD}
    kIdVhsub,                            // [T32 A32 ---] {ASIMD}
    kIdVmax,                             // [T32 A32 ---] {ASIMD}
    kIdVmaxnm,                           // [T32 A32 ---] {ASIMD}
    kIdVmin,                             // [T32 A32 ---] {ASIMD}
    kIdVminnm,                           // [T32 A32 ---] {ASIMD}
    kIdVmla,                             // [T32 A32 ---] {ASIMD|VFPv2}
    kIdVmlal,                            // [T32 A32 ---] {ASIMD}
    kIdVmls,                             // [T32 A32 ---] {ASIMD|VFPv2}
    kIdVmlsl,                            // [T32 A32 ---] {ASIMD}
    kIdVmov,                             // [T32 A32 ---] {ASIMD|VFPv2|VFPv3}
    kIdVmovl,                            // [T32 A32 ---] {ASIMD}
    kIdVmovn,                            // [T32 A32 ---] {ASIMD}
    kIdVmul,                             // [T32 A32 ---] {ASIMD|VFPv2}
    kIdVmull,                            // [T32 A32 ---] {ASIMD}
    kIdVmvn,                             // [T32 A32 ---] {ASIMD}
    kIdVneg,                             // [T32 A32 ---] {ASIMD|VFPv2}
    kIdVnmla,                            // [T32 A32 ---] {VFPv2}
    kIdVnmls,                            // [T32 A32 ---] {VFPv2}
    kIdVnmul,                            // [T32 A32 ---] {VFPv2}
    kIdVorn,                             // [T32 A32 ---] {ASIMD}
    kIdVorr,                             // [T32 A32 ---] {ASIMD}
    kIdVpadal,                           // [T32 A32 ---] {ASIMD}
    kIdVpadd,                            // [T32 A32 ---] {ASIMD}
    kIdVpaddl,                           // [T32 A32 ---] {ASIMD}
    kIdVpmax,                            // [T32 A32 ---] {ASIMD}
    kIdVpmin,                            // [T32 A32 ---] {ASIMD}
    kIdVqabs,                            // [T32 A32 ---] {ASIMD}
    kIdVqadd,                            // [T32 A32 ---] {ASIMD}
    kIdVqdmlal,                          // [T32 A32 ---] {ASIMD}
    kIdVqdmlsl,                          // [T32 A32 ---] {ASIMD}
    kIdVqdmulh,                          // [T32 A32 ---] {ASIMD}
    kIdVqdmull,                          // [T32 A32 ---] {ASIMD}
    kIdVqmovn,                           // [T32 A32 ---] {ASIMD}
    kIdVqmovun,                          // [T32 A32 ---] {ASIMD}
    kIdVqneg,                            // [T32 A32 ---] {ASIMD}
    kIdVqrdmulh,                         // [T32 A32 ---] {ASIMD}
    kIdVqrshl,                           // [T32 A32 ---] {ASIMD}
    kIdVqrshrn,                          // [T32 A32 ---] {ASIMD}
    kIdVqrshrun,                         // [T32 A32 ---] {ASIMD}
    kIdVqshl,                            // [T32 A32 ---] {ASIMD}
    kIdVqshlu,                           // [T32 A32 ---] {ASIMD}
    kIdVqshrn,                           // [T32 A32 ---] {ASIMD}
    kIdVqshrun,                          // [T32 A32 ---] {ASIMD}
    kIdVqsub,                            // [T32 A32 ---] {ASIMD}
    kIdVraddhn,                          // [T32 A32 ---] {ASIMD}
    kIdVrecpe,                           // [T32 A32 ---] {ASIMD}
    kIdVrecps,                           // [T32 A32 ---] {ASIMD}
    kIdVrev16,                           // [T32 A32 ---] {ASIMD}
    kIdVrev32,                           // [T32 A32 ---] {ASIMD}
    kIdVrev64,                           // [T32 A32 ---] {ASIMD}
    kIdVrhadd,                           // [T32 A32 ---] {ASIMD}
    kIdVrinta,                           // [T32 A32 ---]
    kIdVrintm,                           // [T32 A32 ---]
    kIdVrintn,                           // [T32 A32 ---]
    kIdVrintp,                           // [T32 A32 ---]
    kIdVrintr,                           // [T32 A32 ---]
    kIdVrintx,                           // [T32 A32 ---]
    kIdVrintz,                           // [T32 A32 ---]
    kIdVrshl,                            // [T32 A32 ---] {ASIMD}
    kIdVrshr,                            // [T32 A32 ---] {ASIMD}
    kIdVrshrn,                           // [T32 A32 ---] {ASIMD}
    kIdVrsqrte,                          // [T32 A32 ---] {ASIMD}
    kIdVrsqrts,                          // [T32 A32 ---] {ASIMD}
    kIdVrsra,                            // [T32 A32 ---] {ASIMD}
    kIdVrsubhn,                          // [T32 A32 ---] {ASIMD}
    kIdVseleq,                           // [T32 A32 ---]
    kIdVselge,                           // [T32 A32 ---]
    kIdVselgt,                           // [T32 A32 ---]
    kIdVselvs,                           // [T32 A32 ---]
    kIdVshl,                             // [T32 A32 ---] {ASIMD}
    kIdVshll,                            // [T32 A32 ---] {ASIMD}
    kIdVshr,                             // [T32 A32 ---] {ASIMD}
    kIdVshrl,                            // [T32 A32 ---] {ASIMD}
    kIdVsli,                             // [T32 A32 ---] {ASIMD}
    kIdVsqrt,                            // [T32 A32 ---] {VFPv2}
    kIdVsra,                             // [T32 A32 ---] {ASIMD}
    kIdVsri,                             // [T32 A32 ---] {ASIMD}
    kIdVsub,                             // [T32 A32 ---] {ASIMD|VFPv2}
    kIdVsubhn,                           // [T32 A32 ---] {ASIMD}
    kIdVsubl,                            // [T32 A32 ---] {ASIMD}
    kIdVsubw,                            // [T32 A32 ---] {ASIMD}
    kIdVswp,                             // [T32 A32 ---] {ASIMD}
    kIdVtbl,                             // [T32 A32 ---] {ASIMD}
    kIdVtbx,                             // [T32 A32 ---] {ASIMD}
    kIdVtrn,                             // [T32 A32 ---] {ASIMD}
    kIdVtst,                             // [T32 A32 ---] {ASIMD}
    kIdVuzp,                             // [T32 A32 ---] {ASIMD}
    kIdVzip,                             // [T32 A32 ---] {ASIMD}
    kIdWfe,                              // [Txx A32 ---]
    kIdWfi,                              // [Txx A32 ---]
    kIdYield,                            // [Txx A32 ---]
    _kIdCount
    // ${idData:End}
  };

  //! Instruction encodings, used by \ref ArmAssembler.
  enum EncodingType : uint32_t {
    kEncodingNone = 0,                   //!< Never used.
    _kEncodingCount                      //!< Count of instruction encodings.
  };

  //! Instruction family.
  //!
  //! Specifies which table should be used to interpret `_familyDataIndex`.
  enum FamilyType : uint32_t {
    kFamilyNone           = 0,           //!< General purpose or special instruction.
    kFamilyNeon           = 1            //!< NEON family instruction.
  };

  //! \internal
  //!
  //! Instruction flags.
  enum InstFlags : uint32_t {
    kInstFlagNone         = 0x00000000U  //!< No flags.
  };

  //! Specifies meaning of all bits in an opcode (AsmJit specific).
  enum OpCodeBits : uint32_t {
  };

  //! Instruction options.
  enum Options : uint32_t {
    // NOTE: Don't collide with reserved bits used by CodeEmitter (0x000000FF).
    kOptionOp4            = CodeEmitter::kOptionOp4,
    kOptionOp5            = CodeEmitter::kOptionOp5,
    kOptionOpExtra        = CodeEmitter::kOptionOpExtra
  };

  //! Supported architectures.
  enum ArchMask : uint32_t {
    kArchMaskArm32        = 0x01,        //!< ARM32 mode supported.
    kArchMaskArm64        = 0x02         //!< ARM64 mode supported.
  };

  //! Common data - aggregated data that is shared across many instructions.
  struct CommonData {
    // ------------------------------------------------------------------------
    // [Accessors]
    // ------------------------------------------------------------------------

    //! Get all instruction flags, see \ref InstFlags.
    inline uint32_t getFlags() const noexcept { return _flags; }
    //! Get whether the instruction has a `flag`, see `InstFlags`.
    inline bool hasFlag(uint32_t flag) const noexcept { return (_flags & flag) != 0; }

    // ------------------------------------------------------------------------
    // [Members]
    // ------------------------------------------------------------------------

    uint32_t _flags;                     //!< Instruction flags.
  };

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! Get instruction name (null terminated).
  //!
  //! NOTE: If AsmJit was compiled with `ASMJIT_DISABLE_TEXT` then this will
  //! return an empty string (null terminated string of zero length).
  inline const char* getName() const noexcept;
  //! Get index to `ArmInstDB::nameData` of this instruction.
  //!
  //! NOTE: If AsmJit was compiled with `ASMJIT_DISABLE_TEXT` then this will
  //! always return zero.
  inline uint32_t getNameDataIndex() const noexcept { return _nameDataIndex; }

  //! Get \ref CommonData of the instruction.
  inline const CommonData& getCommonData() const noexcept;
  //! Get index to `ArmInstDB::commonData` of this instruction.
  inline uint32_t getCommonDataIndex() const noexcept { return _commonDataIndex; }

  //! Get instruction encoding, see \ref EncodingType.
  inline uint32_t getEncodingType() const noexcept { return _encodingType; }

  //! Get instruction opcode, see \ref OpCodeBits.
  inline uint32_t getOpCode() const noexcept { return _opCode; }

  //! Get whether the instruction has flag `flag`, see \ref InstFlags.
  inline bool hasFlag(uint32_t flag) const noexcept { return getCommonData().hasFlag(flag); }
  //! Get instruction flags, see \ref InstFlags.
  inline uint32_t getFlags() const noexcept { return getCommonData().getFlags(); }

  // --------------------------------------------------------------------------
  // [Get]
  // --------------------------------------------------------------------------

  //! Get whether the `instId` is defined (counts also kInvalidInstId, which is zero).
  static inline bool isDefinedId(uint32_t instId) noexcept { return instId < _kIdCount; }

  //! Get instruction information based on the instruction `instId`.
  //!
  //! NOTE: `instId` has to be a valid instruction ID, it can't be greater than
  //! or equal to `ArmInst::_kIdCount`. It asserts in debug mode.
  static inline const ArmInst& getInst(uint32_t instId) noexcept;

  // --------------------------------------------------------------------------
  // [Id <-> Name]
  // --------------------------------------------------------------------------

  #ifndef ASMJIT_DISABLE_TEXT
  //! Get an instruction ID from a given instruction `name`.
  //!
  //! NOTE: Instruction name MUST BE in lowercase, otherwise there will be no
  //! match. If there is an exact match the instruction id is returned, otherwise
  //! `kInvalidInstId` (zero) is returned instead. The given `name` doesn't have
  //! to be null-terminated if `len` is provided.
  ASMJIT_API static uint32_t getIdByName(const char* name, size_t len = Globals::kNullTerminated) noexcept;

  //! Get an instruction name from a given instruction id `instId`.
  ASMJIT_API static const char* getNameById(uint32_t instId) noexcept;
  #endif

  // --------------------------------------------------------------------------
  // [Validation]
  // --------------------------------------------------------------------------

  #ifndef ASMJIT_DISABLE_INST_API
  ASMJIT_API static Error validate(
    uint32_t archType, uint32_t instId, uint32_t options,
    const Operand_& opExtra,
    const Operand_* operands, uint32_t count) noexcept;
  #endif

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  uint32_t _encodingType    : 8;         //!< Instruction encoding.
  uint32_t _nameDataIndex   : 14;        //!< Index to `X86InstDB::nameData` table.
  uint32_t _commonDataIndex : 10;        //!< Index to `X86InstDB::commonData` table.
  uint32_t _opCode;                      //!< Instruction opcode.
};

//! ARM instruction data under a single namespace.
struct ArmInstDB {
  ASMJIT_API static const ArmInst instData[];
  ASMJIT_API static const ArmInst::CommonData commonData[];
  ASMJIT_API static const char nameData[];
};

//! \}

ASMJIT_END_NAMESPACE

// [Guard]
#endif // _ASMJIT_ARM_ARMINST_H
