// This file is part of AsmJit project <https://asmjit.com>
//
// See asmjit.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_LA_LA64GLOBALS_H_INCLUDED
#define ASMJIT_LA_LA64GLOBALS_H_INCLUDED

#include "../loongarch/laglobals.h"

//! \namespace asmjit::a64
//! \ingroup asmjit_a64
//!
//! Loongarch64 backend.

ASMJIT_BEGIN_SUB_NAMESPACE(la64)

//! \addtogroup asmjit_a64
//! \{

//! Loongarch64 instruction.
//!
//! \note Only used to hold LA-specific enumerations and static functions.
struct Inst {
  //! Instruction id.
  enum Id : uint32_t {
    // ${InstId:Begin}
    kIdNone = 0,                         //!< Instruction ''.
    kIdAdd_d,                            //!< Instruction 'add_d'.
    kIdAdd_w,                            //!< Instruction 'add_w'.
    kIdAddi_d,                           //!< Instruction 'addi_d'.
    kIdAddi_w,                           //!< Instruction 'addi_w'.
    kIdAddu16i_d,                        //!< Instruction 'addu16i_d'.
    kIdAlsl_d,                           //!< Instruction 'alsl_d'.
    kIdAlsl_w,                           //!< Instruction 'alsl_w'.
    kIdAlsl_wu,                          //!< Instruction 'alsl_wu'.
    kIdAmadd_d,                          //!< Instruction 'amadd_d'.
    kIdAmadd_db_d,                       //!< Instruction 'amadd_db_d'.
    kIdAmadd_db_w,                       //!< Instruction 'amadd_db_w'.
    kIdAmadd_w,                          //!< Instruction 'amadd_w'.
    kIdAmand_d,                          //!< Instruction 'amand_d'.
    kIdAmand_db_d,                       //!< Instruction 'amand_db_d'.
    kIdAmand_db_w,                       //!< Instruction 'amand_db_w'.
    kIdAmand_w,                          //!< Instruction 'amand_w'.
    kIdAmmax_d,                          //!< Instruction 'ammax_d'.
    kIdAmmax_db_d,                       //!< Instruction 'ammax_db_d'.
    kIdAmmax_db_du,                      //!< Instruction 'ammax_db_du'.
    kIdAmmax_db_w,                       //!< Instruction 'ammax_db_w'.
    kIdAmmax_db_wu,                      //!< Instruction 'ammax_db_wu'.
    kIdAmmax_du,                         //!< Instruction 'ammax_du'.
    kIdAmmax_w,                          //!< Instruction 'ammax_w'.
    kIdAmmax_wu,                         //!< Instruction 'ammax_wu'.
    kIdAmmin_d,                          //!< Instruction 'ammin_d'.
    kIdAmmin_db_d,                       //!< Instruction 'ammin_db_d'.
    kIdAmmin_db_du,                      //!< Instruction 'ammin_db_du'.
    kIdAmmin_db_w,                       //!< Instruction 'ammin_db_w'.
    kIdAmmin_db_wu,                      //!< Instruction 'ammin_db_wu'.
    kIdAmmin_du,                         //!< Instruction 'ammin_du'.
    kIdAmmin_w,                          //!< Instruction 'ammin_w'.
    kIdAmmin_wu,                         //!< Instruction 'ammin_wu'.
    kIdAmor_d,                           //!< Instruction 'amor_d'.
    kIdAmor_db_d,                        //!< Instruction 'amor_db_d'.
    kIdAmor_db_w,                        //!< Instruction 'amor_db_w'.
    kIdAmor_w,                           //!< Instruction 'amor_w'.
    kIdAmswap_d,                         //!< Instruction 'amswap_d'.
    kIdAmswap_db_d,                      //!< Instruction 'amswap_db_d'.
    kIdAmswap_db_w,                      //!< Instruction 'amswap_db_w'.
    kIdAmswap_w,                         //!< Instruction 'amswap_w'.
    kIdAmxor_d,                          //!< Instruction 'amxor_d'.
    kIdAmxor_db_d,                       //!< Instruction 'amxor_db_d'.
    kIdAmxor_db_w,                       //!< Instruction 'amxor_db_w'.
    kIdAmxor_w,                          //!< Instruction 'amxor_w'.
    kIdAnd_,                             //!< Instruction 'and_'.
    kIdAndi,                             //!< Instruction 'andi'.
    kIdAndn,                             //!< Instruction 'andn'.
    kIdAsrtgt_d,                         //!< Instruction 'asrtgt_d'.
    kIdAsrtle_d,                         //!< Instruction 'asrtle_d'.
    kIdB,                                //!< Instruction 'b'.
    kIdBceqz,                            //!< Instruction 'bceqz'.
    kIdBcnez,                            //!< Instruction 'bcnez'.
    kIdBeq,                              //!< Instruction 'beq'.
    kIdBge,                              //!< Instruction 'bge'.
    kIdBgeu,                             //!< Instruction 'bgeu'.
    kIdBitrev_4b,                        //!< Instruction 'bitrev_4b'.
    kIdBitrev_8b,                        //!< Instruction 'bitrev_8b'.
    kIdBitrev_d,                         //!< Instruction 'bitrev_d'.
    kIdBitrev_w,                         //!< Instruction 'bitrev_w'.
    kIdBl,                               //!< Instruction 'bl'.
    kIdBlt,                              //!< Instruction 'blt'.
    kIdBltu,                             //!< Instruction 'bltu'.
    kIdBne,                              //!< Instruction 'bne'.
    kIdBreak_,                           //!< Instruction 'break_'.
    kIdBstrins_d,                        //!< Instruction 'bstrins_d'.
    kIdBstrins_w,                        //!< Instruction 'bstrins_w'.
    kIdBstrpick_d,                       //!< Instruction 'bstrpick_d'.
    kIdBstrpick_w,                       //!< Instruction 'bstrpick_w'.
    kIdBytepick_d,                       //!< Instruction 'bytepick_d'.
    kIdBytepick_w,                       //!< Instruction 'bytepick_w'.
    kIdCacop,                            //!< Instruction 'cacop'.
    kIdClo_d,                            //!< Instruction 'clo_d'.
    kIdClo_w,                            //!< Instruction 'clo_w'.
    kIdClz_d,                            //!< Instruction 'clz_d'.
    kIdClz_w,                            //!< Instruction 'clz_w'.
    kIdCpucfg,                           //!< Instruction 'cpucfg'.
    kIdCrc_w_b_w,                        //!< Instruction 'crc_w_b_w'.
    kIdCrc_w_d_w,                        //!< Instruction 'crc_w_d_w'.
    kIdCrc_w_h_w,                        //!< Instruction 'crc_w_h_w'.
    kIdCrc_w_w_w,                        //!< Instruction 'crc_w_w_w'.
    kIdCrcc_w_b_w,                       //!< Instruction 'crcc_w_b_w'.
    kIdCrcc_w_d_w,                       //!< Instruction 'crcc_w_d_w'.
    kIdCrcc_w_h_w,                       //!< Instruction 'crcc_w_h_w'.
    kIdCrcc_w_w_w,                       //!< Instruction 'crcc_w_w_w'.
    kIdCsrrd,                            //!< Instruction 'csrrd'.
    kIdCsrwr,                            //!< Instruction 'csrwr'.
    kIdCsrxchg,                          //!< Instruction 'csrxchg'.
    kIdCto_d,                            //!< Instruction 'cto_d'.
    kIdCto_w,                            //!< Instruction 'cto_w'.
    kIdCtz_d,                            //!< Instruction 'ctz_d'.
    kIdCtz_w,                            //!< Instruction 'ctz_w'.
    kIdDbar,                             //!< Instruction 'dbar'.
    kIdDbcl,                             //!< Instruction 'dbcl'.
    kIdDiv_d,                            //!< Instruction 'div_d'.
    kIdDiv_du,                           //!< Instruction 'div_du'.
    kIdDiv_w,                            //!< Instruction 'div_w'.
    kIdDiv_wu,                           //!< Instruction 'div_wu'.
    kIdErtn,                             //!< Instruction 'ertn'.
    kIdExt_w_b,                          //!< Instruction 'ext_w_b'.
    kIdExt_w_h,                          //!< Instruction 'ext_w_h'.
    kIdFabs_d,                           //!< Instruction 'fabs_d'.
    kIdFabs_s,                           //!< Instruction 'fabs_s'.
    kIdFadd_d,                           //!< Instruction 'fadd_d'.
    kIdFadd_s,                           //!< Instruction 'fadd_s'.
    kIdFclass_d,                         //!< Instruction 'fclass_d'.
    kIdFclass_s,                         //!< Instruction 'fclass_s'.
    kIdFcmp_caf_s,                       //!< Instruction 'fcmp_caf_s'.
    kIdFcmp_ceq_s,                       //!< Instruction 'fcmp_ceq_s'.
    kIdFcmp_cle_d,                       //!< Instruction 'fcmp_cle_d'.
    kIdFcmp_clt_d,                       //!< Instruction 'fcmp_clt_d'.
    kIdFcmp_cne_d,                       //!< Instruction 'fcmp_cne_d'.
    kIdFcmp_cor_s,                       //!< Instruction 'fcmp_cor_s'.
    kIdFcmp_cueq_s,                      //!< Instruction 'fcmp_cueq_s'.
    kIdFcmp_cule_s,                      //!< Instruction 'fcmp_cule_s'.
    kIdFcmp_cult_s,                      //!< Instruction 'fcmp_cult_s'.
    kIdFcmp_cun_d,                       //!< Instruction 'fcmp_cun_d'.
    kIdFcmp_cune_d,                      //!< Instruction 'fcmp_cune_d'.
    kIdFcmp_saf_s,                       //!< Instruction 'fcmp_saf_s'.
    kIdFcmp_seq_s,                       //!< Instruction 'fcmp_seq_s'.
    kIdFcmp_sle_s,                       //!< Instruction 'fcmp_sle_s'.
    kIdFcmp_slt_s,                       //!< Instruction 'fcmp_slt_s'.
    kIdFcmp_sne_s,                       //!< Instruction 'fcmp_sne_s'.
    kIdFcmp_sor_d,                       //!< Instruction 'fcmp_sor_d'.
    kIdFcmp_sueq_d,                      //!< Instruction 'fcmp_sueq_d'.
    kIdFcmp_sule_d,                      //!< Instruction 'fcmp_sule_d'.
    kIdFcmp_sult_d,                      //!< Instruction 'fcmp_sult_d'.
    kIdFcmp_sun_d,                       //!< Instruction 'fcmp_sun_d'.
    kIdFcmp_sune_s,                      //!< Instruction 'fcmp_sune_s'.
    kIdFcopysign_d,                      //!< Instruction 'fcopysign_d'.
    kIdFcopysign_s,                      //!< Instruction 'fcopysign_s'.
    kIdFcvt_d_s,                         //!< Instruction 'fcvt_d_s'.
    kIdFcvt_s_d,                         //!< Instruction 'fcvt_s_d'.
    kIdFdiv_d,                           //!< Instruction 'fdiv_d'.
    kIdFdiv_s,                           //!< Instruction 'fdiv_s'.
    kIdFfint_d_l,                        //!< Instruction 'ffint_d_l'.
    kIdFfint_d_w,                        //!< Instruction 'ffint_d_w'.
    kIdFfint_s_l,                        //!< Instruction 'ffint_s_l'.
    kIdFfint_s_w,                        //!< Instruction 'ffint_s_w'.
    kIdFld_d,                            //!< Instruction 'fld_d'.
    kIdFld_s,                            //!< Instruction 'fld_s'.
    kIdFldgt_d,                          //!< Instruction 'fldgt_d'.
    kIdFldgt_s,                          //!< Instruction 'fldgt_s'.
    kIdFldle_d,                          //!< Instruction 'fldle_d'.
    kIdFldle_s,                          //!< Instruction 'fldle_s'.
    kIdFldx_d,                           //!< Instruction 'fldx_d'.
    kIdFldx_s,                           //!< Instruction 'fldx_s'.
    kIdFlogb_d,                          //!< Instruction 'flogb_d'.
    kIdFlogb_s,                          //!< Instruction 'flogb_s'.
    kIdFmadd_d,                          //!< Instruction 'fmadd_d'.
    kIdFmadd_s,                          //!< Instruction 'fmadd_s'.
    kIdFmax_d,                           //!< Instruction 'fmax_d'.
    kIdFmax_s,                           //!< Instruction 'fmax_s'.
    kIdFmaxa_d,                          //!< Instruction 'fmaxa_d'.
    kIdFmaxa_s,                          //!< Instruction 'fmaxa_s'.
    kIdFmin_d,                           //!< Instruction 'fmin_d'.
    kIdFmin_s,                           //!< Instruction 'fmin_s'.
    kIdFmina_d,                          //!< Instruction 'fmina_d'.
    kIdFmina_s,                          //!< Instruction 'fmina_s'.
    kIdFmov_d,                           //!< Instruction 'fmov_d'.
    kIdFmov_s,                           //!< Instruction 'fmov_s'.
    kIdFmul_d,                           //!< Instruction 'fmul_d'.
    kIdFmul_s,                           //!< Instruction 'fmul_s'.
    kIdFneg_d,                           //!< Instruction 'fneg_d'.
    kIdFneg_s,                           //!< Instruction 'fneg_s'.
    kIdFrecip_d,                         //!< Instruction 'frecip_d'.
    kIdFrecip_s,                         //!< Instruction 'frecip_s'.
    kIdFrint_d,                          //!< Instruction 'frint_d'.
    kIdFrint_s,                          //!< Instruction 'frint_s'.
    kIdFrsqrt_d,                         //!< Instruction 'frsqrt_d'.
    kIdFrsqrt_s,                         //!< Instruction 'frsqrt_s'.
    kIdFscaleb_d,                        //!< Instruction 'fscaleb_d'.
    kIdFscaleb_s,                        //!< Instruction 'fscaleb_s'.
    kIdFsel,                             //!< Instruction 'fsel'.
    kIdFsqrt_d,                          //!< Instruction 'fsqrt_d'.
    kIdFsqrt_s,                          //!< Instruction 'fsqrt_s'.
    kIdFst_d,                            //!< Instruction 'fst_d'.
    kIdFst_s,                            //!< Instruction 'fst_s'.
    kIdFstgt_d,                          //!< Instruction 'fstgt_d'.
    kIdFstgt_s,                          //!< Instruction 'fstgt_s'.
    kIdFstle_d,                          //!< Instruction 'fstle_d'.
    kIdFstle_s,                          //!< Instruction 'fstle_s'.
    kIdFstx_d,                           //!< Instruction 'fstx_d'.
    kIdFstx_s,                           //!< Instruction 'fstx_s'.
    kIdFsub_d,                           //!< Instruction 'fsub_d'.
    kIdFsub_s,                           //!< Instruction 'fsub_s'.
    kIdFtint_l_d,                        //!< Instruction 'ftint_l_d'.
    kIdFtint_l_s,                        //!< Instruction 'ftint_l_s'.
    kIdFtint_w_d,                        //!< Instruction 'ftint_w_d'.
    kIdFtint_w_s,                        //!< Instruction 'ftint_w_s'.
    kIdFtintrm_l_d,                      //!< Instruction 'ftintrm_l_d'.
    kIdFtintrm_l_s,                      //!< Instruction 'ftintrm_l_s'.
    kIdFtintrm_w_d,                      //!< Instruction 'ftintrm_w_d'.
    kIdFtintrm_w_s,                      //!< Instruction 'ftintrm_w_s'.
    kIdFtintrne_l_d,                     //!< Instruction 'ftintrne_l_d'.
    kIdFtintrne_l_s,                     //!< Instruction 'ftintrne_l_s'.
    kIdFtintrne_w_d,                     //!< Instruction 'ftintrne_w_d'.
    kIdFtintrne_w_s,                     //!< Instruction 'ftintrne_w_s'.
    kIdFtintrp_l_d,                      //!< Instruction 'ftintrp_l_d'.
    kIdFtintrp_l_s,                      //!< Instruction 'ftintrp_l_s'.
    kIdFtintrp_w_d,                      //!< Instruction 'ftintrp_w_d'.
    kIdFtintrp_w_s,                      //!< Instruction 'ftintrp_w_s'.
    kIdFtintrz_l_d,                      //!< Instruction 'ftintrz_l_d'.
    kIdFtintrz_l_s,                      //!< Instruction 'ftintrz_l_s'.
    kIdFtintrz_w_d,                      //!< Instruction 'ftintrz_w_d'.
    kIdFtintrz_w_s,                      //!< Instruction 'ftintrz_w_s'.
    kIdIbar,                             //!< Instruction 'ibar'.
    kIdIdle,                             //!< Instruction 'idle'.
    kIdInvtlb,                           //!< Instruction 'invtlb'.
    kIdIocsrrd_b,                        //!< Instruction 'iocsrrd_b'.
    kIdIocsrrd_d,                        //!< Instruction 'iocsrrd_d'.
    kIdIocsrrd_h,                        //!< Instruction 'iocsrrd_h'.
    kIdIocsrrd_w,                        //!< Instruction 'iocsrrd_w'.
    kIdIocsrwr_b,                        //!< Instruction 'iocsrwr_b'.
    kIdIocsrwr_d,                        //!< Instruction 'iocsrwr_d'.
    kIdIocsrwr_h,                        //!< Instruction 'iocsrwr_h'.
    kIdIocsrwr_w,                        //!< Instruction 'iocsrwr_w'.
    kIdJirl,                             //!< Instruction 'jirl'.
    kIdLd_b,                             //!< Instruction 'ld_b'.
    kIdLd_bu,                            //!< Instruction 'ld_bu'.
    kIdLd_d,                             //!< Instruction 'ld_d'.
    kIdLd_h,                             //!< Instruction 'ld_h'.
    kIdLd_hu,                            //!< Instruction 'ld_hu'.
    kIdLd_w,                             //!< Instruction 'ld_w'.
    kIdLd_wu,                            //!< Instruction 'ld_wu'.
    kIdLddir,                            //!< Instruction 'lddir'.
    kIdLdgt_b,                           //!< Instruction 'ldgt_b'.
    kIdLdgt_d,                           //!< Instruction 'ldgt_d'.
    kIdLdgt_h,                           //!< Instruction 'ldgt_h'.
    kIdLdgt_w,                           //!< Instruction 'ldgt_w'.
    kIdLdle_b,                           //!< Instruction 'ldle_b'.
    kIdLdle_d,                           //!< Instruction 'ldle_d'.
    kIdLdle_h,                           //!< Instruction 'ldle_h'.
    kIdLdle_w,                           //!< Instruction 'ldle_w'.
    kIdLdpte,                            //!< Instruction 'ldpte'.
    kIdLdptr_d,                          //!< Instruction 'ldptr_d'.
    kIdLdptr_w,                          //!< Instruction 'ldptr_w'.
    kIdLdx_b,                            //!< Instruction 'ldx_b'.
    kIdLdx_bu,                           //!< Instruction 'ldx_bu'.
    kIdLdx_d,                            //!< Instruction 'ldx_d'.
    kIdLdx_h,                            //!< Instruction 'ldx_h'.
    kIdLdx_hu,                           //!< Instruction 'ldx_hu'.
    kIdLdx_w,                            //!< Instruction 'ldx_w'.
    kIdLdx_wu,                           //!< Instruction 'ldx_wu'.
    kIdLl_d,                             //!< Instruction 'll_d'.
    kIdLl_w,                             //!< Instruction 'll_w'.
    kIdLu12i_w,                          //!< Instruction 'lu12i_w'.
    kIdLu32i_d,                          //!< Instruction 'lu32i_d'.
    kIdLu52i_d,                          //!< Instruction 'lu52i_d'.
    kIdMaskeqz,                          //!< Instruction 'maskeqz'.
    kIdMasknez,                          //!< Instruction 'masknez'.
    kIdMod_d,                            //!< Instruction 'mod_d'.
    kIdMod_du,                           //!< Instruction 'mod_du'.
    kIdMod_w,                            //!< Instruction 'mod_w'.
    kIdMod_wu,                           //!< Instruction 'mod_wu'.
    kIdMovcf2fr,                         //!< Instruction 'movcf2fr'.
    kIdMovcf2gr,                         //!< Instruction 'movcf2gr'.
    kIdMovfcsr2gr,                       //!< Instruction 'movfcsr2gr'.
    kIdMovfr2cf,                         //!< Instruction 'movfr2cf'.
    kIdMovfr2gr_d,                       //!< Instruction 'movfr2gr_d'.
    kIdMovfr2gr_s,                       //!< Instruction 'movfr2gr_s'.
    kIdMovfrh2gr_s,                      //!< Instruction 'movfrh2gr_s'.
    kIdMovgr2cf,                         //!< Instruction 'movgr2cf'.
    kIdMovgr2fcsr,                       //!< Instruction 'movgr2fcsr'.
    kIdMovgr2fr_d,                       //!< Instruction 'movgr2fr_d'.
    kIdMovgr2fr_w,                       //!< Instruction 'movgr2fr_w'.
    kIdMovgr2frh_w,                      //!< Instruction 'movgr2frh_w'.
    kIdMul_d,                            //!< Instruction 'mul_d'.
    kIdMul_w,                            //!< Instruction 'mul_w'.
    kIdMulh_d,                           //!< Instruction 'mulh_d'.
    kIdMulh_du,                          //!< Instruction 'mulh_du'.
    kIdMulh_w,                           //!< Instruction 'mulh_w'.
    kIdMulh_wu,                          //!< Instruction 'mulh_wu'.
    kIdMulw_d_w,                         //!< Instruction 'mulw_d_w'.
    kIdMulw_d_wu,                        //!< Instruction 'mulw_d_wu'.
    kIdNor,                              //!< Instruction 'nor'.
    kIdOr_,                              //!< Instruction 'or_'.
    kIdMove,                             //!< Instruction 'move'.
    kIdOri,                              //!< Instruction 'ori'.
    kIdOrn,                              //!< Instruction 'orn'.
    kIdPcaddi,                           //!< Instruction 'pcaddi'.
    kIdPcaddu12i,                        //!< Instruction 'pcaddu12i'.
    kIdPcaddu18i,                        //!< Instruction 'pcaddu18i'.
    kIdPcalau12i,                        //!< Instruction 'pcalau12i'.
    kIdPreld,                            //!< Instruction 'preld'.
    kIdRdtime_d,                         //!< Instruction 'rdtime_d'.
    kIdRdtimeh_w,                        //!< Instruction 'rdtimeh_w'.
    kIdRdtimel_w,                        //!< Instruction 'rdtimel_w'.
    kIdRevb_2h,                          //!< Instruction 'revb_2h'.
    kIdRevb_2w,                          //!< Instruction 'revb_2w'.
    kIdRevb_4h,                          //!< Instruction 'revb_4h'.
    kIdRevb_d,                           //!< Instruction 'revb_d'.
    kIdRevh_2w,                          //!< Instruction 'revh_2w'.
    kIdRevh_d,                           //!< Instruction 'revh_d'.
    kIdRotr_d,                           //!< Instruction 'rotr_d'.
    kIdRotr_w,                           //!< Instruction 'rotr_w'.
    kIdRotri_d,                          //!< Instruction 'rotri_d'.
    kIdRotri_w,                          //!< Instruction 'rotri_w'.
    kIdSc_d,                             //!< Instruction 'sc_d'.
    kIdSc_w,                             //!< Instruction 'sc_w'.
    kIdSll_d,                            //!< Instruction 'sll_d'.
    kIdSll_w,                            //!< Instruction 'sll_w'.
    kIdSlli_d,                           //!< Instruction 'slli_d'.
    kIdSlli_w,                           //!< Instruction 'slli_w'.
    kIdSlti,                             //!< Instruction 'slti'.
    kIdSltui,                            //!< Instruction 'sltui'.
    kIdSra_d,                            //!< Instruction 'sra_d'.
    kIdSra_w,                            //!< Instruction 'sra_w'.
    kIdSrai_d,                           //!< Instruction 'srai_d'.
    kIdSrai_w,                           //!< Instruction 'srai_w'.
    kIdSrl_d,                            //!< Instruction 'srl_d'.
    kIdSrl_w,                            //!< Instruction 'srl_w'.
    kIdSrli_d,                           //!< Instruction 'srli_d'.
    kIdSrli_w,                           //!< Instruction 'srli_w'.
    kIdSt_b,                             //!< Instruction 'st_b'.
    kIdSt_d,                             //!< Instruction 'st_d'.
    kIdSt_h,                             //!< Instruction 'st_h'.
    kIdSt_w,                             //!< Instruction 'st_w'.
    kIdStgt_b,                           //!< Instruction 'stgt_b'.
    kIdStgt_d,                           //!< Instruction 'stgt_d'.
    kIdStgt_h,                           //!< Instruction 'stgt_h'.
    kIdStgt_w,                           //!< Instruction 'stgt_w'.
    kIdStle_b,                           //!< Instruction 'stle_b'.
    kIdStle_d,                           //!< Instruction 'stle_d'.
    kIdStle_h,                           //!< Instruction 'stle_h'.
    kIdStle_w,                           //!< Instruction 'stle_w'.
    kIdStptr_d,                          //!< Instruction 'stptr_d'.
    kIdStptr_w,                          //!< Instruction 'stptr_w'.
    kIdStx_b,                            //!< Instruction 'stx_b'.
    kIdStx_d,                            //!< Instruction 'stx_d'.
    kIdStx_h,                            //!< Instruction 'stx_h'.
    kIdStx_w,                            //!< Instruction 'stx_w'.
    kIdSub_d,                            //!< Instruction 'sub_d'.
    kIdSub_w,                            //!< Instruction 'sub_w'.
    kIdSyscall,                          //!< Instruction 'syscall'.
    kIdTlbclr,                           //!< Instruction 'tlbclr'.
    kIdTlbfill,                          //!< Instruction 'tlbfill'.
    kIdTlbflush,                         //!< Instruction 'tlbflush'.
    kIdTlbrd,                            //!< Instruction 'tlbrd'.
    kIdTlbsrch,                          //!< Instruction 'tlbsrch'.
    kIdTlbwr,                            //!< Instruction 'tlbwr'.
    kIdXor_,                             //!< Instruction 'xor_'.
    kIdXori,                             //!< Instruction 'xori'.
    kIdVabsd_b,                          //!< Instruction 'vabsd_b'.
    kIdVabsd_bu,                         //!< Instruction 'vabsd_bu'.
    kIdVabsd_d,                          //!< Instruction 'vabsd_d'.
    kIdVabsd_du,                         //!< Instruction 'vabsd_du'.
    kIdVabsd_h,                          //!< Instruction 'vabsd_h'.
    kIdVabsd_hu,                         //!< Instruction 'vabsd_hu'.
    kIdVabsd_w,                          //!< Instruction 'vabsd_w'.
    kIdVabsd_wu,                         //!< Instruction 'vabsd_wu'.
    kIdVadd_b,                           //!< Instruction 'vadd_b'.
    kIdVadd_d,                           //!< Instruction 'vadd_d'.
    kIdVadd_h,                           //!< Instruction 'vadd_h'.
    kIdVadd_q,                           //!< Instruction 'vadd_q'.
    kIdVadd_w,                           //!< Instruction 'vadd_w'.
    kIdVadda_b,                          //!< Instruction 'vadda_b'.
    kIdVadda_d,                          //!< Instruction 'vadda_d'.
    kIdVadda_h,                          //!< Instruction 'vadda_h'.
    kIdVadda_w,                          //!< Instruction 'vadda_w'.
    kIdVaddi_bu,                         //!< Instruction 'vaddi_bu'.
    kIdVaddi_du,                         //!< Instruction 'vaddi_du'.
    kIdVaddi_hu,                         //!< Instruction 'vaddi_hu'.
    kIdVaddi_wu,                         //!< Instruction 'vaddi_wu'.
    kIdVaddwev_d_w,                      //!< Instruction 'vaddwev_d_w'.
    kIdVaddwev_d_wu,                     //!< Instruction 'vaddwev_d_wu'.
    kIdVaddwev_d_wu_w,                   //!< Instruction 'vaddwev_d_wu_w'.
    kIdVaddwev_h_b,                      //!< Instruction 'vaddwev_h_b'.
    kIdVaddwev_h_bu,                     //!< Instruction 'vaddwev_h_bu'.
    kIdVaddwev_h_bu_b,                   //!< Instruction 'vaddwev_h_bu_b'.
    kIdVaddwev_q_d,                      //!< Instruction 'vaddwev_q_d'.
    kIdVaddwev_q_du,                     //!< Instruction 'vaddwev_q_du'.
    kIdVaddwev_q_du_d,                   //!< Instruction 'vaddwev_q_du_d'.
    kIdVaddwev_w_h,                      //!< Instruction 'vaddwev_w_h'.
    kIdVaddwev_w_hu,                     //!< Instruction 'vaddwev_w_hu'.
    kIdVaddwev_w_hu_h,                   //!< Instruction 'vaddwev_w_hu_h'.
    kIdVaddwod_d_w,                      //!< Instruction 'vaddwod_d_w'.
    kIdVaddwod_d_wu,                     //!< Instruction 'vaddwod_d_wu'.
    kIdVaddwod_d_wu_w,                   //!< Instruction 'vaddwod_d_wu_w'.
    kIdVaddwod_h_b,                      //!< Instruction 'vaddwod_h_b'.
    kIdVaddwod_h_bu,                     //!< Instruction 'vaddwod_h_bu'.
    kIdVaddwod_h_bu_b,                   //!< Instruction 'vaddwod_h_bu_b'.
    kIdVaddwod_q_d,                      //!< Instruction 'vaddwod_q_d'.
    kIdVaddwod_q_du,                     //!< Instruction 'vaddwod_q_du'.
    kIdVaddwod_q_du_d,                   //!< Instruction 'vaddwod_q_du_d'.
    kIdVaddwod_w_h,                      //!< Instruction 'vaddwod_w_h'.
    kIdVaddwod_w_hu,                     //!< Instruction 'vaddwod_w_hu'.
    kIdVaddwod_w_hu_h,                   //!< Instruction 'vaddwod_w_hu_h'.
    kIdVand_v,                           //!< Instruction 'vand' {ASIMD}.
    kIdVandi_b,                          //!< Instruction 'vandi_b'.
    kIdVandn_v,                          //!< Instruction 'vandn' {ASIMD}.
    kIdVavg_b,                           //!< Instruction 'vavg_b'.
    kIdVavg_bu,                          //!< Instruction 'vavg_bu'.
    kIdVavg_d,                           //!< Instruction 'vavg_d'.
    kIdVavg_du,                          //!< Instruction 'vavg_du'.
    kIdVavg_h,                           //!< Instruction 'vavg_h'.
    kIdVavg_hu,                          //!< Instruction 'vavg_hu'.
    kIdVavg_w,                           //!< Instruction 'vavg_w'.
    kIdVavg_wu,                          //!< Instruction 'vavg_wu'.
    kIdVavgr_b,                          //!< Instruction 'vavgr_b'.
    kIdVavgr_bu,                         //!< Instruction 'vavgr_bu'.
    kIdVavgr_d,                          //!< Instruction 'vavgr_d'.
    kIdVavgr_du,                         //!< Instruction 'vavgr_du'.
    kIdVavgr_h,                          //!< Instruction 'vavgr_h'.
    kIdVavgr_hu,                         //!< Instruction 'vavgr_hu'.
    kIdVavgr_w,                          //!< Instruction 'vavgr_w'.
    kIdVavgr_wu,                         //!< Instruction 'vavgr_wu'.
    kIdVbitclr_b,                        //!< Instruction 'vbitclr_b'.
    kIdVbitclr_d,                        //!< Instruction 'vbitclr_d'.
    kIdVbitclr_h,                        //!< Instruction 'vbitclr_h'.
    kIdVbitclr_w,                        //!< Instruction 'vbitclr_w'.
    kIdVbitclri_b,                       //!< Instruction 'vbitclri_b'.
    kIdVbitclri_d,                       //!< Instruction 'vbitclri_d'.
    kIdVbitclri_h,                       //!< Instruction 'vbitclri_h'.
    kIdVbitclri_w,                       //!< Instruction 'vbitclri_w'.
    kIdVbitrev_b,                        //!< Instruction 'vbitrev_b'.
    kIdVbitrev_d,                        //!< Instruction 'vbitrev_d'.
    kIdVbitrev_h,                        //!< Instruction 'vbitrev_h'.
    kIdVbitrev_w,                        //!< Instruction 'vbitrev_w'.
    kIdVbitrevi_b,                       //!< Instruction 'vbitrevi_b'.
    kIdVbitrevi_d,                       //!< Instruction 'vbitrevi_d'.
    kIdVbitrevi_h,                       //!< Instruction 'vbitrevi_h'.
    kIdVbitrevi_w,                       //!< Instruction 'vbitrevi_w'.
    kIdVbitsel_v,                        //!< Instruction 'vbitsel' {ASIMD}.
    kIdVbitseli_b,                       //!< Instruction 'vbitseli_b'.
    kIdVbitset_b,                        //!< Instruction 'vbitset_b'.
    kIdVbitset_d,                        //!< Instruction 'vbitset_d'.
    kIdVbitset_h,                        //!< Instruction 'vbitset_h'.
    kIdVbitset_w,                        //!< Instruction 'vbitset_w'.
    kIdVbitseti_b,                       //!< Instruction 'vbitseti_b'.
    kIdVbitseti_d,                       //!< Instruction 'vbitseti_d'.
    kIdVbitseti_h,                       //!< Instruction 'vbitseti_h'.
    kIdVbitseti_w,                       //!< Instruction 'vbitseti_w'.
    kIdVbsll_v,                          //!< Instruction 'vbsll' {ASIMD}.
    kIdVbsrl_v,                          //!< Instruction 'vbsrl' {ASIMD}.
    kIdVclo_b,                           //!< Instruction 'vclo_b'.
    kIdVclo_d,                           //!< Instruction 'vclo_d'.
    kIdVclo_h,                           //!< Instruction 'vclo_h'.
    kIdVclo_w,                           //!< Instruction 'vclo_w'.
    kIdVclz_b,                           //!< Instruction 'vclz_b'.
    kIdVclz_d,                           //!< Instruction 'vclz_d'.
    kIdVclz_h,                           //!< Instruction 'vclz_h'.
    kIdVclz_w,                           //!< Instruction 'vclz_w'.
    kIdVdiv_b,                           //!< Instruction 'vdiv_b'.
    kIdVdiv_bu,                          //!< Instruction 'vdiv_bu'.
    kIdVdiv_d,                           //!< Instruction 'vdiv_d'.
    kIdVdiv_du,                          //!< Instruction 'vdiv_du'.
    kIdVdiv_h,                           //!< Instruction 'vdiv_h'.
    kIdVdiv_hu,                          //!< Instruction 'vdiv_hu'.
    kIdVdiv_w,                           //!< Instruction 'vdiv_w'.
    kIdVdiv_wu,                          //!< Instruction 'vdiv_wu'.
    kIdVext2xv_d_b,                      //!< Instruction 'vext2xv_d_b'.
    kIdVext2xv_d_h,                      //!< Instruction 'vext2xv_d_h'.
    kIdVext2xv_d_w,                      //!< Instruction 'vext2xv_d_w'.
    kIdVext2xv_du_bu,                    //!< Instruction 'vext2xv_du_bu'.
    kIdVext2xv_du_hu,                    //!< Instruction 'vext2xv_du_hu'.
    kIdVext2xv_du_wu,                    //!< Instruction 'vext2xv_du_wu'.
    kIdVext2xv_h_b,                      //!< Instruction 'vext2xv_h_b'.
    kIdVext2xv_hu_bu,                    //!< Instruction 'vext2xv_hu_bu'.
    kIdVext2xv_w_b,                      //!< Instruction 'vext2xv_w_b'.
    kIdVext2xv_w_h,                      //!< Instruction 'vext2xv_w_h'.
    kIdVext2xv_wu_bu,                    //!< Instruction 'vext2xv_wu_bu'.
    kIdVext2xv_wu_hu,                    //!< Instruction 'vext2xv_wu_hu'.
    kIdVexth_d_w,                        //!< Instruction 'vexth_d_w'.
    kIdVexth_du_wu,                      //!< Instruction 'vexth_du_wu'.
    kIdVexth_h_b,                        //!< Instruction 'vexth_h_b'.
    kIdVexth_hu_bu,                      //!< Instruction 'vexth_hu_bu'.
    kIdVexth_q_d,                        //!< Instruction 'vexth_q_d'.
    kIdVexth_qu_du,                      //!< Instruction 'vexth_qu_du'.
    kIdVexth_w_h,                        //!< Instruction 'vexth_w_h'.
    kIdVexth_wu_hu,                      //!< Instruction 'vexth_wu_hu'.
    kIdVextl_q_d,                        //!< Instruction 'vextl_q_d'.
    kIdVextl_qu_du,                      //!< Instruction 'vextl_qu_du'.
    kIdVextrins_b,                       //!< Instruction 'vextrins_b'.
    kIdVextrins_d,                       //!< Instruction 'vextrins_d'.
    kIdVextrins_h,                       //!< Instruction 'vextrins_h'.
    kIdVextrins_w,                       //!< Instruction 'vextrins_w'.
    kIdVfadd_d,                          //!< Instruction 'vfadd_d'.
    kIdVfadd_s,                          //!< Instruction 'vfadd_s'.
    kIdVfclass_d,                        //!< Instruction 'vfclass_d'.
    kIdVfclass_s,                        //!< Instruction 'vfclass_s'.
    kIdVfcvt_h_s,                        //!< Instruction 'vfcvt_h_s'.
    kIdVfcvt_s_d,                        //!< Instruction 'vfcvt_s_d'.
    kIdVfcvth_d_s,                       //!< Instruction 'vfcvth_d_s'.
    kIdVfcvth_s_h,                       //!< Instruction 'vfcvth_s_h'.
    kIdVfcvtl_d_s,                       //!< Instruction 'vfcvtl_d_s'.
    kIdVfcvtl_s_h,                       //!< Instruction 'vfcvtl_s_h'.
    kIdVfdiv_d,                          //!< Instruction 'vfdiv_d'.
    kIdVfdiv_s,                          //!< Instruction 'vfdiv_s'.
    kIdVffint_d_l,                       //!< Instruction 'vffint_d_l'.
    kIdVffint_d_lu,                      //!< Instruction 'vffint_d_lu'.
    kIdVffint_s_l,                       //!< Instruction 'vffint_s_l'.
    kIdVffint_s_w,                       //!< Instruction 'vffint_s_w'.
    kIdVffint_s_wu,                      //!< Instruction 'vffint_s_wu'.
    kIdVffinth_d_w,                      //!< Instruction 'vffinth_d_w'.
    kIdVffintl_d_w,                      //!< Instruction 'vffintl_d_w'.
    kIdVflogb_d,                         //!< Instruction 'vflogb_d'.
    kIdVflogb_s,                         //!< Instruction 'vflogb_s'.
    kIdVfmadd_d,                         //!< Instruction 'vfmadd_d'.
    kIdVfmadd_s,                         //!< Instruction 'vfmadd_s'.
    kIdVfmax_d,                          //!< Instruction 'vfmax_d'.
    kIdVfmax_s,                          //!< Instruction 'vfmax_s'.
    kIdVfmaxa_d,                         //!< Instruction 'vfmaxa_d'.
    kIdVfmaxa_s,                         //!< Instruction 'vfmaxa_s'.
    kIdVfmin_d,                          //!< Instruction 'vfmin_d'.
    kIdVfmin_s,                          //!< Instruction 'vfmin_s'.
    kIdVfmina_d,                         //!< Instruction 'vfmina_d'.
    kIdVfmina_s,                         //!< Instruction 'vfmina_s'.
    kIdVfmsub_d,                         //!< Instruction 'vfmsub_d'.
    kIdVfmsub_s,                         //!< Instruction 'vfmsub_s'.
    kIdVfmul_d,                          //!< Instruction 'vfmul_d'.
    kIdVfmul_s,                          //!< Instruction 'vfmul_s'.
    kIdVfnmadd_d,                        //!< Instruction 'vfnmadd_d'.
    kIdVfnmadd_s,                        //!< Instruction 'vfnmadd_s'.
    kIdVfnmsub_d,                        //!< Instruction 'vfnmsub_d'.
    kIdVfnmsub_s,                        //!< Instruction 'vfnmsub_s'.
    kIdVfrecip_d,                        //!< Instruction 'vfrecip_d'.
    kIdVfrecip_s,                        //!< Instruction 'vfrecip_s'.
    kIdVfrint_d,                         //!< Instruction 'vfrint_d'.
    kIdVfrint_s,                         //!< Instruction 'vfrint_s'.
    kIdVfrintrm_d,                       //!< Instruction 'vfrintrm_d'.
    kIdVfrintrm_s,                       //!< Instruction 'vfrintrm_s'.
    kIdVfrintrne_d,                      //!< Instruction 'vfrintrne_d'.
    kIdVfrintrne_s,                      //!< Instruction 'vfrintrne_s'.
    kIdVfrintrp_d,                       //!< Instruction 'vfrintrp_d'.
    kIdVfrintrp_s,                       //!< Instruction 'vfrintrp_s'.
    kIdVfrintrz_d,                       //!< Instruction 'vfrintrz_d'.
    kIdVfrintrz_s,                       //!< Instruction 'vfrintrz_s'.
    kIdVfrsqrt_d,                        //!< Instruction 'vfrsqrt_d'.
    kIdVfrsqrt_s,                        //!< Instruction 'vfrsqrt_s'.
    kIdVfrstp_b,                         //!< Instruction 'vfrstp_b'.
    kIdVfrstp_h,                         //!< Instruction 'vfrstp_h'.
    kIdVfrstpi_b,                        //!< Instruction 'vfrstpi_b'.
    kIdVfrstpi_h,                        //!< Instruction 'vfrstpi_h'.
    kIdVfscaleb_d,                       //!< Instruction 'vfscaleb_d'.
    kIdVfscaleb_s,                       //!< Instruction 'vfscaleb_s'.
    kIdVfsqrt_d,                         //!< Instruction 'vfsqrt_d'.
    kIdVfsqrt_s,                         //!< Instruction 'vfsqrt_s'.
    kIdVfsub_d,                          //!< Instruction 'vfsub_d'.
    kIdVfsub_s,                          //!< Instruction 'vfsub_s'.
    kIdVftint_l_d,                       //!< Instruction 'vftint_l_d'.
    kIdVftint_lu_d,                      //!< Instruction 'vftint_lu_d'.
    kIdVftint_w_d,                       //!< Instruction 'vftint_w_d'.
    kIdVftint_w_s,                       //!< Instruction 'vftint_w_s'.
    kIdVftint_wu_s,                      //!< Instruction 'vftint_wu_s'.
    kIdVftinth_l_s,                      //!< Instruction 'vftinth_l_s'.
    kIdVftintl_l_s,                      //!< Instruction 'vftintl_l_s'.
    kIdVftintrm_l_d,                     //!< Instruction 'vftintrm_l_d'.
    kIdVftintrm_w_d,                     //!< Instruction 'vftintrm_w_d'.
    kIdVftintrm_w_s,                     //!< Instruction 'vftintrm_w_s'.
    kIdVftintrmh_l_s,                    //!< Instruction 'vftintrmh_l_s'.
    kIdVftintrml_l_s,                    //!< Instruction 'vftintrml_l_s'.
    kIdVftintrne_l_d,                    //!< Instruction 'vftintrne_l_d'.
    kIdVftintrne_w_d,                    //!< Instruction 'vftintrne_w_d'.
    kIdVftintrne_w_s,                    //!< Instruction 'vftintrne_w_s'.
    kIdVftintrneh_l_s,                   //!< Instruction 'vftintrneh_l_s'.
    kIdVftintrnel_l_s,                   //!< Instruction 'vftintrnel_l_s'.
    kIdVftintrp_l_d,                     //!< Instruction 'vftintrp_l_d'.
    kIdVftintrp_w_d,                     //!< Instruction 'vftintrp_w_d'.
    kIdVftintrp_w_s,                     //!< Instruction 'vftintrp_w_s'.
    kIdVftintrph_l_s,                    //!< Instruction 'vftintrph_l_s'.
    kIdVftintrpl_l_s,                    //!< Instruction 'vftintrpl_l_s'.
    kIdVftintrz_l_d,                     //!< Instruction 'vftintrz_l_d'.
    kIdVftintrz_lu_d,                    //!< Instruction 'vftintrz_lu_d'.
    kIdVftintrz_w_d,                     //!< Instruction 'vftintrz_w_d'.
    kIdVftintrz_w_s,                     //!< Instruction 'vftintrz_w_s'.
    kIdVftintrz_wu_s,                    //!< Instruction 'vftintrz_wu_s'.
    kIdVftintrzh_l_s,                    //!< Instruction 'vftintrzh_l_s'.
    kIdVftintrzl_l_s,                    //!< Instruction 'vftintrzl_l_s'.
    kIdVhaddw_d_w,                       //!< Instruction 'vhaddw_d_w'.
    kIdVhaddw_du_wu,                     //!< Instruction 'vhaddw_du_wu'.
    kIdVhaddw_h_b,                       //!< Instruction 'vhaddw_h_b'.
    kIdVhaddw_hu_bu,                     //!< Instruction 'vhaddw_hu_bu'.
    kIdVhaddw_q_d,                       //!< Instruction 'vhaddw_q_d'.
    kIdVhaddw_qu_du,                     //!< Instruction 'vhaddw_qu_du'.
    kIdVhaddw_w_h,                       //!< Instruction 'vhaddw_w_h'.
    kIdVhaddw_wu_hu,                     //!< Instruction 'vhaddw_wu_hu'.
    kIdVhsubw_d_w,                       //!< Instruction 'vhsubw_d_w'.
    kIdVhsubw_du_wu,                     //!< Instruction 'vhsubw_du_wu'.
    kIdVhsubw_h_b,                       //!< Instruction 'vhsubw_h_b'.
    kIdVhsubw_hu_bu,                     //!< Instruction 'vhsubw_hu_bu'.
    kIdVhsubw_q_d,                       //!< Instruction 'vhsubw_q_d'.
    kIdVhsubw_qu_du,                     //!< Instruction 'vhsubw_qu_du'.
    kIdVhsubw_w_h,                       //!< Instruction 'vhsubw_w_h'.
    kIdVhsubw_wu_hu,                     //!< Instruction 'vhsubw_wu_hu'.
    kIdVilvh_b,                          //!< Instruction 'vilvh_b'.
    kIdVilvh_d,                          //!< Instruction 'vilvh_d'.
    kIdVilvh_h,                          //!< Instruction 'vilvh_h'.
    kIdVilvh_w,                          //!< Instruction 'vilvh_w'.
    kIdVilvl_b,                          //!< Instruction 'vilvl_b'.
    kIdVilvl_d,                          //!< Instruction 'vilvl_d'.
    kIdVilvl_h,                          //!< Instruction 'vilvl_h'.
    kIdVilvl_w,                          //!< Instruction 'vilvl_w'.
    kIdVinsgr2vr_b,                      //!< Instruction 'vinsgr2vr_b'.
    kIdVinsgr2vr_d,                      //!< Instruction 'vinsgr2vr_d'.
    kIdVinsgr2vr_h,                      //!< Instruction 'vinsgr2vr_h'.
    kIdVinsgr2vr_w,                      //!< Instruction 'vinsgr2vr_w'.
    kIdVld,                              //!< Instruction 'vld'.
    kIdVldi,                             //!< Instruction 'vldi'.
    kIdVldrepl_b,                        //!< Instruction 'vldrepl_b'.
    kIdVldrepl_d,                        //!< Instruction 'vldrepl_d'.
    kIdVldrepl_h,                        //!< Instruction 'vldrepl_h'.
    kIdVldrepl_w,                        //!< Instruction 'vldrepl_w'.
    kIdVldx,                             //!< Instruction 'vldx'.
    kIdVmadd_b,                          //!< Instruction 'vmadd_b'.
    kIdVmadd_d,                          //!< Instruction 'vmadd_d'.
    kIdVmadd_h,                          //!< Instruction 'vmadd_h'.
    kIdVmadd_w,                          //!< Instruction 'vmadd_w'.
    kIdVmaddwev_d_w,                     //!< Instruction 'vmaddwev_d_w'.
    kIdVmaddwev_d_wu,                    //!< Instruction 'vmaddwev_d_wu'.
    kIdVmaddwev_d_wu_w,                  //!< Instruction 'vmaddwev_d_wu_w'.
    kIdVmaddwev_h_b,                     //!< Instruction 'vmaddwev_h_b'.
    kIdVmaddwev_h_bu,                    //!< Instruction 'vmaddwev_h_bu'.
    kIdVmaddwev_h_bu_b,                  //!< Instruction 'vmaddwev_h_bu_b'.
    kIdVmaddwev_q_d,                     //!< Instruction 'vmaddwev_q_d'.
    kIdVmaddwev_q_du,                    //!< Instruction 'vmaddwev_q_du'.
    kIdVmaddwev_q_du_d,                  //!< Instruction 'vmaddwev_q_du_d'.
    kIdVmaddwev_w_h,                     //!< Instruction 'vmaddwev_w_h'.
    kIdVmaddwev_w_hu,                    //!< Instruction 'vmaddwev_w_hu'.
    kIdVmaddwev_w_hu_h,                  //!< Instruction 'vmaddwev_w_hu_h'.
    kIdVmaddwod_d_w,                     //!< Instruction 'vmaddwod_d_w'.
    kIdVmaddwod_d_wu,                    //!< Instruction 'vmaddwod_d_wu'.
    kIdVmaddwod_d_wu_w,                  //!< Instruction 'vmaddwod_d_wu_w'.
    kIdVmaddwod_h_b,                     //!< Instruction 'vmaddwod_h_b'.
    kIdVmaddwod_h_bu,                    //!< Instruction 'vmaddwod_h_bu'.
    kIdVmaddwod_h_bu_b,                  //!< Instruction 'vmaddwod_h_bu_b'.
    kIdVmaddwod_q_d,                     //!< Instruction 'vmaddwod_q_d'.
    kIdVmaddwod_q_du,                    //!< Instruction 'vmaddwod_q_du'.
    kIdVmaddwod_q_du_d,                  //!< Instruction 'vmaddwod_q_du_d'.
    kIdVmaddwod_w_h,                     //!< Instruction 'vmaddwod_w_h'.
    kIdVmaddwod_w_hu,                    //!< Instruction 'vmaddwod_w_hu'.
    kIdVmaddwod_w_hu_h,                  //!< Instruction 'vmaddwod_w_hu_h'.
    kIdVmax_b,                           //!< Instruction 'vmax_b'.
    kIdVmax_bu,                          //!< Instruction 'vmax_bu'.
    kIdVmax_d,                           //!< Instruction 'vmax_d'.
    kIdVmax_du,                          //!< Instruction 'vmax_du'.
    kIdVmax_h,                           //!< Instruction 'vmax_h'.
    kIdVmax_hu,                          //!< Instruction 'vmax_hu'.
    kIdVmax_w,                           //!< Instruction 'vmax_w'.
    kIdVmax_wu,                          //!< Instruction 'vmax_wu'.
    kIdVmaxi_b,                          //!< Instruction 'vmaxi_b'.
    kIdVmaxi_bu,                         //!< Instruction 'vmaxi_bu'.
    kIdVmaxi_d,                          //!< Instruction 'vmaxi_d'.
    kIdVmaxi_du,                         //!< Instruction 'vmaxi_du'.
    kIdVmaxi_h,                          //!< Instruction 'vmaxi_h'.
    kIdVmaxi_hu,                         //!< Instruction 'vmaxi_hu'.
    kIdVmaxi_w,                          //!< Instruction 'vmaxi_w'.
    kIdVmaxi_wu,                         //!< Instruction 'vmaxi_wu'.
    kIdVmepatmsk_v,                      //!< Instruction 'vmepatmsk' {ASIMD}.
    kIdVmin_b,                           //!< Instruction 'vmin_b'.
    kIdVmin_bu,                          //!< Instruction 'vmin_bu'.
    kIdVmin_d,                           //!< Instruction 'vmin_d'.
    kIdVmin_du,                          //!< Instruction 'vmin_du'.
    kIdVmin_h,                           //!< Instruction 'vmin_h'.
    kIdVmin_hu,                          //!< Instruction 'vmin_hu'.
    kIdVmin_w,                           //!< Instruction 'vmin_w'.
    kIdVmin_wu,                          //!< Instruction 'vmin_wu'.
    kIdVmini_b,                          //!< Instruction 'vmini_b'.
    kIdVmini_bu,                         //!< Instruction 'vmini_bu'.
    kIdVmini_d,                          //!< Instruction 'vmini_d'.
    kIdVmini_du,                         //!< Instruction 'vmini_du'.
    kIdVmini_h,                          //!< Instruction 'vmini_h'.
    kIdVmini_hu,                         //!< Instruction 'vmini_hu'.
    kIdVmini_w,                          //!< Instruction 'vmini_w'.
    kIdVmini_wu,                         //!< Instruction 'vmini_wu'.
    kIdVmod_b,                           //!< Instruction 'vmod_b'.
    kIdVmod_bu,                          //!< Instruction 'vmod_bu'.
    kIdVmod_d,                           //!< Instruction 'vmod_d'.
    kIdVmod_du,                          //!< Instruction 'vmod_du'.
    kIdVmod_h,                           //!< Instruction 'vmod_h'.
    kIdVmod_hu,                          //!< Instruction 'vmod_hu'.
    kIdVmod_w,                           //!< Instruction 'vmod_w'.
    kIdVmod_wu,                          //!< Instruction 'vmod_wu'.
    kIdVmskgez_b,                        //!< Instruction 'vmskgez_b'.
    kIdVmskltz_b,                        //!< Instruction 'vmskltz_b'.
    kIdVmskltz_d,                        //!< Instruction 'vmskltz_d'.
    kIdVmskltz_h,                        //!< Instruction 'vmskltz_h'.
    kIdVmskltz_w,                        //!< Instruction 'vmskltz_w'.
    kIdVmsknz_b,                         //!< Instruction 'vmsknz_b'.
    kIdVmsub_b,                          //!< Instruction 'vmsub_b'.
    kIdVmsub_d,                          //!< Instruction 'vmsub_d'.
    kIdVmsub_h,                          //!< Instruction 'vmsub_h'.
    kIdVmsub_w,                          //!< Instruction 'vmsub_w'.
    kIdVmuh_b,                           //!< Instruction 'vmuh_b'.
    kIdVmuh_bu,                          //!< Instruction 'vmuh_bu'.
    kIdVmuh_d,                           //!< Instruction 'vmuh_d'.
    kIdVmuh_du,                          //!< Instruction 'vmuh_du'.
    kIdVmuh_h,                           //!< Instruction 'vmuh_h'.
    kIdVmuh_hu,                          //!< Instruction 'vmuh_hu'.
    kIdVmuh_w,                           //!< Instruction 'vmuh_w'.
    kIdVmuh_wu,                          //!< Instruction 'vmuh_wu'.
    kIdVmul_b,                           //!< Instruction 'vmul_b'.
    kIdVmul_d,                           //!< Instruction 'vmul_d'.
    kIdVmul_h,                           //!< Instruction 'vmul_h'.
    kIdVmul_w,                           //!< Instruction 'vmul_w'.
    kIdVmulwev_d_w,                      //!< Instruction 'vmulwev_d_w'.
    kIdVmulwev_d_wu,                     //!< Instruction 'vmulwev_d_wu'.
    kIdVmulwev_d_wu_w,                   //!< Instruction 'vmulwev_d_wu_w'.
    kIdVmulwev_h_b,                      //!< Instruction 'vmulwev_h_b'.
    kIdVmulwev_h_bu,                     //!< Instruction 'vmulwev_h_bu'.
    kIdVmulwev_h_bu_b,                   //!< Instruction 'vmulwev_h_bu_b'.
    kIdVmulwev_q_d,                      //!< Instruction 'vmulwev_q_d'.
    kIdVmulwev_q_du,                     //!< Instruction 'vmulwev_q_du'.
    kIdVmulwev_q_du_d,                   //!< Instruction 'vmulwev_q_du_d'.
    kIdVmulwev_w_h,                      //!< Instruction 'vmulwev_w_h'.
    kIdVmulwev_w_hu,                     //!< Instruction 'vmulwev_w_hu'.
    kIdVmulwev_w_hu_h,                   //!< Instruction 'vmulwev_w_hu_h'.
    kIdVmulwod_d_w,                      //!< Instruction 'vmulwod_d_w'.
    kIdVmulwod_d_wu,                     //!< Instruction 'vmulwod_d_wu'.
    kIdVmulwod_d_wu_w,                   //!< Instruction 'vmulwod_d_wu_w'.
    kIdVmulwod_h_b,                      //!< Instruction 'vmulwod_h_b'.
    kIdVmulwod_h_bu,                     //!< Instruction 'vmulwod_h_bu'.
    kIdVmulwod_h_bu_b,                   //!< Instruction 'vmulwod_h_bu_b'.
    kIdVmulwod_q_d,                      //!< Instruction 'vmulwod_q_d'.
    kIdVmulwod_q_du,                     //!< Instruction 'vmulwod_q_du'.
    kIdVmulwod_q_du_d,                   //!< Instruction 'vmulwod_q_du_d'.
    kIdVmulwod_w_h,                      //!< Instruction 'vmulwod_w_h'.
    kIdVmulwod_w_hu,                     //!< Instruction 'vmulwod_w_hu'.
    kIdVmulwod_w_hu_h,                   //!< Instruction 'vmulwod_w_hu_h'.
    kIdVneg_b,                           //!< Instruction 'vneg_b'.
    kIdVneg_d,                           //!< Instruction 'vneg_d'.
    kIdVneg_h,                           //!< Instruction 'vneg_h'.
    kIdVneg_w,                           //!< Instruction 'vneg_w'.
    kIdVnor_v,                           //!< Instruction 'vnor' {ASIMD}.
    kIdVnori_b,                          //!< Instruction 'vnori_b'.
    kIdVor_v,                            //!< Instruction 'vor' {ASIMD}.
    kIdVori_b,                           //!< Instruction 'vori_b'.
    kIdVorn_v,                           //!< Instruction 'vorn' {ASIMD}.
    kIdVpackev_b,                        //!< Instruction 'vpackev_b'.
    kIdVpackev_d,                        //!< Instruction 'vpackev_d'.
    kIdVpackev_h,                        //!< Instruction 'vpackev_h'.
    kIdVpackev_w,                        //!< Instruction 'vpackev_w'.
    kIdVpackod_b,                        //!< Instruction 'vpackod_b'.
    kIdVpackod_d,                        //!< Instruction 'vpackod_d'.
    kIdVpackod_h,                        //!< Instruction 'vpackod_h'.
    kIdVpackod_w,                        //!< Instruction 'vpackod_w'.
    kIdVpcnt_b,                          //!< Instruction 'vpcnt_b'.
    kIdVpcnt_d,                          //!< Instruction 'vpcnt_d'.
    kIdVpcnt_h,                          //!< Instruction 'vpcnt_h'.
    kIdVpcnt_w,                          //!< Instruction 'vpcnt_w'.
    kIdVpermi_w,                         //!< Instruction 'vpermi_w'.
    kIdVpickev_b,                        //!< Instruction 'vpickev_b'.
    kIdVpickev_d,                        //!< Instruction 'vpickev_d'.
    kIdVpickev_h,                        //!< Instruction 'vpickev_h'.
    kIdVpickev_w,                        //!< Instruction 'vpickev_w'.
    kIdVpickod_b,                        //!< Instruction 'vpickod_b'.
    kIdVpickod_d,                        //!< Instruction 'vpickod_d'.
    kIdVpickod_h,                        //!< Instruction 'vpickod_h'.
    kIdVpickod_w,                        //!< Instruction 'vpickod_w'.
    kIdVpickve2gr_b,                     //!< Instruction 'vpickve2gr_b'.
    kIdVpickve2gr_bu,                    //!< Instruction 'vpickve2gr_bu'.
    kIdVpickve2gr_d,                     //!< Instruction 'vpickve2gr_d'.
    kIdVpickve2gr_du,                    //!< Instruction 'vpickve2gr_du'.
    kIdVpickve2gr_h,                     //!< Instruction 'vpickve2gr_h'.
    kIdVpickve2gr_hu,                    //!< Instruction 'vpickve2gr_hu'.
    kIdVpickve2gr_w,                     //!< Instruction 'vpickve2gr_w'.
    kIdVpickve2gr_wu,                    //!< Instruction 'vpickve2gr_wu'.
    kIdVreplgr2vr_b,                     //!< Instruction 'vreplgr2vr_b'.
    kIdVreplgr2vr_d,                     //!< Instruction 'vreplgr2vr_d'.
    kIdVreplgr2vr_h,                     //!< Instruction 'vreplgr2vr_h'.
    kIdVreplgr2vr_w,                     //!< Instruction 'vreplgr2vr_w'.
    kIdVreplve_b,                        //!< Instruction 'vreplve_b'.
    kIdVreplve_d,                        //!< Instruction 'vreplve_d'.
    kIdVreplve_h,                        //!< Instruction 'vreplve_h'.
    kIdVreplve_w,                        //!< Instruction 'vreplve_w'.
    kIdVreplvei_b,                       //!< Instruction 'vreplvei_b'.
    kIdVreplvei_d,                       //!< Instruction 'vreplvei_d'.
    kIdVreplvei_h,                       //!< Instruction 'vreplvei_h'.
    kIdVreplvei_w,                       //!< Instruction 'vreplvei_w'.
    kIdVrotr_b,                          //!< Instruction 'vrotr_b'.
    kIdVrotr_d,                          //!< Instruction 'vrotr_d'.
    kIdVrotr_h,                          //!< Instruction 'vrotr_h'.
    kIdVrotr_w,                          //!< Instruction 'vrotr_w'.
    kIdVrotri_b,                         //!< Instruction 'vrotri_b'.
    kIdVrotri_d,                         //!< Instruction 'vrotri_d'.
    kIdVrotri_h,                         //!< Instruction 'vrotri_h'.
    kIdVrotri_w,                         //!< Instruction 'vrotri_w'.
    kIdVsadd_b,                          //!< Instruction 'vsadd_b'.
    kIdVsadd_bu,                         //!< Instruction 'vsadd_bu'.
    kIdVsadd_d,                          //!< Instruction 'vsadd_d'.
    kIdVsadd_du,                         //!< Instruction 'vsadd_du'.
    kIdVsadd_h,                          //!< Instruction 'vsadd_h'.
    kIdVsadd_hu,                         //!< Instruction 'vsadd_hu'.
    kIdVsadd_w,                          //!< Instruction 'vsadd_w'.
    kIdVsadd_wu,                         //!< Instruction 'vsadd_wu'.
    kIdVsat_b,                           //!< Instruction 'vsat_b'.
    kIdVsat_bu,                          //!< Instruction 'vsat_bu'.
    kIdVsat_d,                           //!< Instruction 'vsat_d'.
    kIdVsat_du,                          //!< Instruction 'vsat_du'.
    kIdVsat_h,                           //!< Instruction 'vsat_h'.
    kIdVsat_hu,                          //!< Instruction 'vsat_hu'.
    kIdVsat_w,                           //!< Instruction 'vsat_w'.
    kIdVsat_wu,                          //!< Instruction 'vsat_wu'.
    kIdVseq_b,                           //!< Instruction 'vseq_b'.
    kIdVseq_d,                           //!< Instruction 'vseq_d'.
    kIdVseq_h,                           //!< Instruction 'vseq_h'.
    kIdVseq_w,                           //!< Instruction 'vseq_w'.
    kIdVseqi_b,                          //!< Instruction 'vseqi_b'.
    kIdVseqi_d,                          //!< Instruction 'vseqi_d'.
    kIdVseqi_h,                          //!< Instruction 'vseqi_h'.
    kIdVseqi_w,                          //!< Instruction 'vseqi_w'.
    kIdVsetallnez_b,                     //!< Instruction 'vsetallnez_b'.
    kIdVsetallnez_d,                     //!< Instruction 'vsetallnez_d'.
    kIdVsetallnez_h,                     //!< Instruction 'vsetallnez_h'.
    kIdVsetallnez_w,                     //!< Instruction 'vsetallnez_w'.
    kIdVsetanyeqz_b,                     //!< Instruction 'vsetanyeqz_b'.
    kIdVsetanyeqz_d,                     //!< Instruction 'vsetanyeqz_d'.
    kIdVsetanyeqz_h,                     //!< Instruction 'vsetanyeqz_h'.
    kIdVsetanyeqz_w,                     //!< Instruction 'vsetanyeqz_w'.
    kIdVseteqz_v,                        //!< Instruction 'vseteqz' {ASIMD}.
    kIdVsetnez_v,                        //!< Instruction 'vsetnez' {ASIMD}.
    kIdVshuf4i_b,                        //!< Instruction 'vshuf4i_b'.
    kIdVshuf4i_d,                        //!< Instruction 'vshuf4i_d'.
    kIdVshuf4i_h,                        //!< Instruction 'vshuf4i_h'.
    kIdVshuf4i_w,                        //!< Instruction 'vshuf4i_w'.
    kIdVshuf_b,                          //!< Instruction 'vshuf_b'.
    kIdVshuf_d,                          //!< Instruction 'vshuf_d'.
    kIdVshuf_h,                          //!< Instruction 'vshuf_h'.
    kIdVshuf_w,                          //!< Instruction 'vshuf_w'.
    kIdVsigncov_b,                       //!< Instruction 'vsigncov_b'.
    kIdVsigncov_d,                       //!< Instruction 'vsigncov_d'.
    kIdVsigncov_h,                       //!< Instruction 'vsigncov_h'.
    kIdVsigncov_w,                       //!< Instruction 'vsigncov_w'.
    kIdVsle_b,                           //!< Instruction 'vsle_b'.
    kIdVsle_bu,                          //!< Instruction 'vsle_bu'.
    kIdVsle_d,                           //!< Instruction 'vsle_d'.
    kIdVsle_du,                          //!< Instruction 'vsle_du'.
    kIdVsle_h,                           //!< Instruction 'vsle_h'.
    kIdVsle_hu,                          //!< Instruction 'vsle_hu'.
    kIdVsle_w,                           //!< Instruction 'vsle_w'.
    kIdVsle_wu,                          //!< Instruction 'vsle_wu'.
    kIdVslei_b,                          //!< Instruction 'vslei_b'.
    kIdVslei_bu,                         //!< Instruction 'vslei_bu'.
    kIdVslei_d,                          //!< Instruction 'vslei_d'.
    kIdVslei_du,                         //!< Instruction 'vslei_du'.
    kIdVslei_h,                          //!< Instruction 'vslei_h'.
    kIdVslei_hu,                         //!< Instruction 'vslei_hu'.
    kIdVslei_w,                          //!< Instruction 'vslei_w'.
    kIdVslei_wu,                         //!< Instruction 'vslei_wu'.
    kIdVsll_b,                           //!< Instruction 'vsll_b'.
    kIdVsll_d,                           //!< Instruction 'vsll_d'.
    kIdVsll_h,                           //!< Instruction 'vsll_h'.
    kIdVsll_w,                           //!< Instruction 'vsll_w'.
    kIdVslli_b,                          //!< Instruction 'vslli_b'.
    kIdVslli_d,                          //!< Instruction 'vslli_d'.
    kIdVslli_h,                          //!< Instruction 'vslli_h'.
    kIdVslli_w,                          //!< Instruction 'vslli_w'.
    kIdVsllwil_d_w,                      //!< Instruction 'vsllwil_d_w'.
    kIdVsllwil_du_wu,                    //!< Instruction 'vsllwil_du_wu'.
    kIdVsllwil_h_b,                      //!< Instruction 'vsllwil_h_b'.
    kIdVsllwil_hu_bu,                    //!< Instruction 'vsllwil_hu_bu'.
    kIdVsllwil_w_h,                      //!< Instruction 'vsllwil_w_h'.
    kIdVsllwil_wu_hu,                    //!< Instruction 'vsllwil_wu_hu'.
    kIdVslt_b,                           //!< Instruction 'vslt_b'.
    kIdVslt_bu,                          //!< Instruction 'vslt_bu'.
    kIdVslt_d,                           //!< Instruction 'vslt_d'.
    kIdVslt_du,                          //!< Instruction 'vslt_du'.
    kIdVslt_h,                           //!< Instruction 'vslt_h'.
    kIdVslt_hu,                          //!< Instruction 'vslt_hu'.
    kIdVslt_w,                           //!< Instruction 'vslt_w'.
    kIdVslt_wu,                          //!< Instruction 'vslt_wu'.
    kIdVslti_b,                          //!< Instruction 'vslti_b'.
    kIdVslti_bu,                         //!< Instruction 'vslti_bu'.
    kIdVslti_d,                          //!< Instruction 'vslti_d'.
    kIdVslti_du,                         //!< Instruction 'vslti_du'.
    kIdVslti_h,                          //!< Instruction 'vslti_h'.
    kIdVslti_hu,                         //!< Instruction 'vslti_hu'.
    kIdVslti_w,                          //!< Instruction 'vslti_w'.
    kIdVslti_wu,                         //!< Instruction 'vslti_wu'.
    kIdVsra_b,                           //!< Instruction 'vsra_b'.
    kIdVsra_d,                           //!< Instruction 'vsra_d'.
    kIdVsra_h,                           //!< Instruction 'vsra_h'.
    kIdVsra_w,                           //!< Instruction 'vsra_w'.
    kIdVsrai_b,                          //!< Instruction 'vsrai_b'.
    kIdVsrai_d,                          //!< Instruction 'vsrai_d'.
    kIdVsrai_h,                          //!< Instruction 'vsrai_h'.
    kIdVsrai_w,                          //!< Instruction 'vsrai_w'.
    kIdVsran_b_h,                        //!< Instruction 'vsran_b_h'.
    kIdVsran_h_w,                        //!< Instruction 'vsran_h_w'.
    kIdVsran_w_d,                        //!< Instruction 'vsran_w_d'.
    kIdVsrani_b_h,                       //!< Instruction 'vsrani_b_h'.
    kIdVsrani_d_q,                       //!< Instruction 'vsrani_d_q'.
    kIdVsrani_h_w,                       //!< Instruction 'vsrani_h_w'.
    kIdVsrani_w_d,                       //!< Instruction 'vsrani_w_d'.
    kIdVsrar_b,                          //!< Instruction 'vsrar_b'.
    kIdVsrar_d,                          //!< Instruction 'vsrar_d'.
    kIdVsrar_h,                          //!< Instruction 'vsrar_h'.
    kIdVsrar_w,                          //!< Instruction 'vsrar_w'.
    kIdVsrari_b,                         //!< Instruction 'vsrari_b'.
    kIdVsrari_d,                         //!< Instruction 'vsrari_d'.
    kIdVsrari_h,                         //!< Instruction 'vsrari_h'.
    kIdVsrari_w,                         //!< Instruction 'vsrari_w'.
    kIdVsrarn_b_h,                       //!< Instruction 'vsrarn_b_h'.
    kIdVsrarn_h_w,                       //!< Instruction 'vsrarn_h_w'.
    kIdVsrarn_w_d,                       //!< Instruction 'vsrarn_w_d'.
    kIdVsrarni_b_h,                      //!< Instruction 'vsrarni_b_h'.
    kIdVsrarni_d_q,                      //!< Instruction 'vsrarni_d_q'.
    kIdVsrarni_h_w,                      //!< Instruction 'vsrarni_h_w'.
    kIdVsrarni_w_d,                      //!< Instruction 'vsrarni_w_d'.
    kIdVsrl_b,                           //!< Instruction 'vsrl_b'.
    kIdVsrl_d,                           //!< Instruction 'vsrl_d'.
    kIdVsrl_h,                           //!< Instruction 'vsrl_h'.
    kIdVsrl_w,                           //!< Instruction 'vsrl_w'.
    kIdVsrli_b,                          //!< Instruction 'vsrli_b'.
    kIdVsrli_d,                          //!< Instruction 'vsrli_d'.
    kIdVsrli_h,                          //!< Instruction 'vsrli_h'.
    kIdVsrli_w,                          //!< Instruction 'vsrli_w'.
    kIdVsrln_b_h,                        //!< Instruction 'vsrln_b_h'.
    kIdVsrln_h_w,                        //!< Instruction 'vsrln_h_w'.
    kIdVsrln_w_d,                        //!< Instruction 'vsrln_w_d'.
    kIdVsrlni_b_h,                       //!< Instruction 'vsrlni_b_h'.
    kIdVsrlni_d_q,                       //!< Instruction 'vsrlni_d_q'.
    kIdVsrlni_h_w,                       //!< Instruction 'vsrlni_h_w'.
    kIdVsrlni_w_d,                       //!< Instruction 'vsrlni_w_d'.
    kIdVsrlr_b,                          //!< Instruction 'vsrlr_b'.
    kIdVsrlr_d,                          //!< Instruction 'vsrlr_d'.
    kIdVsrlr_h,                          //!< Instruction 'vsrlr_h'.
    kIdVsrlr_w,                          //!< Instruction 'vsrlr_w'.
    kIdVsrlri_b,                         //!< Instruction 'vsrlri_b'.
    kIdVsrlri_d,                         //!< Instruction 'vsrlri_d'.
    kIdVsrlri_h,                         //!< Instruction 'vsrlri_h'.
    kIdVsrlri_w,                         //!< Instruction 'vsrlri_w'.
    kIdVsrlrn_b_h,                       //!< Instruction 'vsrlrn_b_h'.
    kIdVsrlrn_h_w,                       //!< Instruction 'vsrlrn_h_w'.
    kIdVsrlrn_w_d,                       //!< Instruction 'vsrlrn_w_d'.
    kIdVssran_b_h,                       //!< Instruction 'vssran_b_h'.
    kIdVssran_bu_h,                      //!< Instruction 'vssran_bu_h'.
    kIdVssran_h_w,                       //!< Instruction 'vssran_h_w'.
    kIdVssran_hu_w,                      //!< Instruction 'vssran_hu_w'.
    kIdVssran_w_d,                       //!< Instruction 'vssran_w_d'.
    kIdVssran_wu_d,                      //!< Instruction 'vssran_wu_d'.
    kIdVssrani_b_h,                      //!< Instruction 'vssrani_b_h'.
    kIdVssrani_bu_h,                     //!< Instruction 'vssrani_bu_h'.
    kIdVssrani_d_q,                      //!< Instruction 'vssrani_d_q'.
    kIdVssrani_du_q,                     //!< Instruction 'vssrani_du_q'.
    kIdVssrani_h_w,                      //!< Instruction 'vssrani_h_w'.
    kIdVssrani_hu_w,                     //!< Instruction 'vssrani_hu_w'.
    kIdVssrani_w_d,                      //!< Instruction 'vssrani_w_d'.
    kIdVssrani_wu_d,                     //!< Instruction 'vssrani_wu_d'.
    kIdVssrarn_b_h,                      //!< Instruction 'vssrarn_b_h'.
    kIdVssrarn_bu_h,                     //!< Instruction 'vssrarn_bu_h'.
    kIdVssrarn_h_w,                      //!< Instruction 'vssrarn_h_w'.
    kIdVssrarn_hu_w,                     //!< Instruction 'vssrarn_hu_w'.
    kIdVssrarn_w_d,                      //!< Instruction 'vssrarn_w_d'.
    kIdVssrarn_wu_d,                     //!< Instruction 'vssrarn_wu_d'.
    kIdVssrarni_b_h,                     //!< Instruction 'vssrarni_b_h'.
    kIdVssrarni_bu_h,                    //!< Instruction 'vssrarni_bu_h'.
    kIdVssrarni_d_q,                     //!< Instruction 'vssrarni_d_q'.
    kIdVssrarni_du_q,                    //!< Instruction 'vssrarni_du_q'.
    kIdVssrarni_h_w,                     //!< Instruction 'vssrarni_h_w'.
    kIdVssrarni_hu_w,                    //!< Instruction 'vssrarni_hu_w'.
    kIdVssrarni_w_d,                     //!< Instruction 'vssrarni_w_d'.
    kIdVssrarni_wu_d,                    //!< Instruction 'vssrarni_wu_d'.
    kIdVssrln_b_h,                       //!< Instruction 'vssrln_b_h'.
    kIdVssrln_bu_h,                      //!< Instruction 'vssrln_bu_h'.
    kIdVssrln_h_w,                       //!< Instruction 'vssrln_h_w'.
    kIdVssrln_hu_w,                      //!< Instruction 'vssrln_hu_w'.
    kIdVssrln_w_d,                       //!< Instruction 'vssrln_w_d'.
    kIdVssrln_wu_d,                      //!< Instruction 'vssrln_wu_d'.
    kIdVssrlni_b_h,                      //!< Instruction 'vssrlni_b_h'.
    kIdVssrlni_bu_h,                     //!< Instruction 'vssrlni_bu_h'.
    kIdVssrlni_d_q,                      //!< Instruction 'vssrlni_d_q'.
    kIdVssrlni_du_q,                     //!< Instruction 'vssrlni_du_q'.
    kIdVssrlni_h_w,                      //!< Instruction 'vssrlni_h_w'.
    kIdVssrlni_hu_w,                     //!< Instruction 'vssrlni_hu_w'.
    kIdVssrlni_w_d,                      //!< Instruction 'vssrlni_w_d'.
    kIdVssrlni_wu_d,                     //!< Instruction 'vssrlni_wu_d'.
    kIdVssrlrn_b_h,                      //!< Instruction 'vssrlrn_b_h'.
    kIdVssrlrn_bu_h,                     //!< Instruction 'vssrlrn_bu_h'.
    kIdVssrlrn_h_w,                      //!< Instruction 'vssrlrn_h_w'.
    kIdVssrlrn_hu_w,                     //!< Instruction 'vssrlrn_hu_w'.
    kIdVssrlrn_w_d,                      //!< Instruction 'vssrlrn_w_d'.
    kIdVssrlrn_wu_d,                     //!< Instruction 'vssrlrn_wu_d'.
    kIdVssrlrni_b_h,                     //!< Instruction 'vssrlrni_b_h'.
    kIdVssrlrni_bu_h,                    //!< Instruction 'vssrlrni_bu_h'.
    kIdVssrlrni_d_q,                     //!< Instruction 'vssrlrni_d_q'.
    kIdVssrlrni_du_q,                    //!< Instruction 'vssrlrni_du_q'.
    kIdVssrlrni_h_w,                     //!< Instruction 'vssrlrni_h_w'.
    kIdVssrlrni_hu_w,                    //!< Instruction 'vssrlrni_hu_w'.
    kIdVssrlrni_w_d,                     //!< Instruction 'vssrlrni_w_d'.
    kIdVssrlrni_wu_d,                    //!< Instruction 'vssrlrni_wu_d'.
    kIdVssub_b,                          //!< Instruction 'vssub_b'.
    kIdVssub_bu,                         //!< Instruction 'vssub_bu'.
    kIdVssub_d,                          //!< Instruction 'vssub_d'.
    kIdVssub_du,                         //!< Instruction 'vssub_du'.
    kIdVssub_h,                          //!< Instruction 'vssub_h'.
    kIdVssub_hu,                         //!< Instruction 'vssub_hu'.
    kIdVssub_w,                          //!< Instruction 'vssub_w'.
    kIdVssub_wu,                         //!< Instruction 'vssub_wu'.
    kIdVst,                              //!< Instruction 'vst'.
    kIdVstelm_b,                         //!< Instruction 'vstelm_b'.
    kIdVstelm_d,                         //!< Instruction 'vstelm_d'.
    kIdVstelm_h,                         //!< Instruction 'vstelm_h'.
    kIdVstelm_w,                         //!< Instruction 'vstelm_w'.
    kIdVstx,                             //!< Instruction 'vstx'.
    kIdVsub_b,                           //!< Instruction 'vsub_b'.
    kIdVsub_d,                           //!< Instruction 'vsub_d'.
    kIdVsub_h,                           //!< Instruction 'vsub_h'.
    kIdVsub_q,                           //!< Instruction 'vsub_q'.
    kIdVsub_w,                           //!< Instruction 'vsub_w'.
    kIdVsubi_bu,                         //!< Instruction 'vsubi_bu'.
    kIdVsubi_du,                         //!< Instruction 'vsubi_du'.
    kIdVsubi_hu,                         //!< Instruction 'vsubi_hu'.
    kIdVsubi_wu,                         //!< Instruction 'vsubi_wu'.
    kIdVsubwev_d_w,                      //!< Instruction 'vsubwev_d_w'.
    kIdVsubwev_d_wu,                     //!< Instruction 'vsubwev_d_wu'.
    kIdVsubwev_h_b,                      //!< Instruction 'vsubwev_h_b'.
    kIdVsubwev_h_bu,                     //!< Instruction 'vsubwev_h_bu'.
    kIdVsubwev_q_d,                      //!< Instruction 'vsubwev_q_d'.
    kIdVsubwev_q_du,                     //!< Instruction 'vsubwev_q_du'.
    kIdVsubwev_w_h,                      //!< Instruction 'vsubwev_w_h'.
    kIdVsubwev_w_hu,                     //!< Instruction 'vsubwev_w_hu'.
    kIdVsubwod_d_w,                      //!< Instruction 'vsubwod_d_w'.
    kIdVsubwod_d_wu,                     //!< Instruction 'vsubwod_d_wu'.
    kIdVsubwod_h_b,                      //!< Instruction 'vsubwod_h_b'.
    kIdVsubwod_h_bu,                     //!< Instruction 'vsubwod_h_bu'.
    kIdVsubwod_q_d,                      //!< Instruction 'vsubwod_q_d'.
    kIdVsubwod_q_du,                     //!< Instruction 'vsubwod_q_du'.
    kIdVsubwod_w_h,                      //!< Instruction 'vsubwod_w_h'.
    kIdVsubwod_w_hu,                     //!< Instruction 'vsubwod_w_hu'.
    kIdVxor_v,                           //!< Instruction 'vxor' {ASIMD}.
    kIdVxori_b,                          //!< Instruction 'vxori_b'.
    kIdXvabsd_b,                         //!< Instruction 'xvabsd_b'.
    kIdXvabsd_bu,                        //!< Instruction 'xvabsd_bu'.
    kIdXvabsd_d,                         //!< Instruction 'xvabsd_d'.
    kIdXvabsd_du,                        //!< Instruction 'xvabsd_du'.
    kIdXvabsd_h,                         //!< Instruction 'xvabsd_h'.
    kIdXvabsd_hu,                        //!< Instruction 'xvabsd_hu'.
    kIdXvabsd_w,                         //!< Instruction 'xvabsd_w'.
    kIdXvabsd_wu,                        //!< Instruction 'xvabsd_wu'.
    kIdXvadd_b,                          //!< Instruction 'xvadd_b'.
    kIdXvadd_d,                          //!< Instruction 'xvadd_d'.
    kIdXvadd_h,                          //!< Instruction 'xvadd_h'.
    kIdXvadd_q,                          //!< Instruction 'xvadd_q'.
    kIdXvadd_w,                          //!< Instruction 'xvadd_w'.
    kIdXvadda_b,                         //!< Instruction 'xvadda_b'.
    kIdXvadda_d,                         //!< Instruction 'xvadda_d'.
    kIdXvadda_h,                         //!< Instruction 'xvadda_h'.
    kIdXvadda_w,                         //!< Instruction 'xvadda_w'.
    kIdXvaddi_bu,                        //!< Instruction 'xvaddi_bu'.
    kIdXvaddi_du,                        //!< Instruction 'xvaddi_du'.
    kIdXvaddi_hu,                        //!< Instruction 'xvaddi_hu'.
    kIdXvaddi_wu,                        //!< Instruction 'xvaddi_wu'.
    kIdXvaddwev_d_w,                     //!< Instruction 'xvaddwev_d_w'.
    kIdXvaddwev_d_wu,                    //!< Instruction 'xvaddwev_d_wu'.
    kIdXvaddwev_d_wu_w,                  //!< Instruction 'xvaddwev_d_wu_w'.
    kIdXvaddwev_h_b,                     //!< Instruction 'xvaddwev_h_b'.
    kIdXvaddwev_h_bu,                    //!< Instruction 'xvaddwev_h_bu'.
    kIdXvaddwev_h_bu_b,                  //!< Instruction 'xvaddwev_h_bu_b'.
    kIdXvaddwev_q_d,                     //!< Instruction 'xvaddwev_q_d'.
    kIdXvaddwev_q_du,                    //!< Instruction 'xvaddwev_q_du'.
    kIdXvaddwev_q_du_d,                  //!< Instruction 'xvaddwev_q_du_d'.
    kIdXvaddwev_w_h,                     //!< Instruction 'xvaddwev_w_h'.
    kIdXvaddwev_w_hu,                    //!< Instruction 'xvaddwev_w_hu'.
    kIdXvaddwev_w_hu_h,                  //!< Instruction 'xvaddwev_w_hu_h'.
    kIdXvaddwod_d_w,                     //!< Instruction 'xvaddwod_d_w'.
    kIdXvaddwod_d_wu,                    //!< Instruction 'xvaddwod_d_wu'.
    kIdXvaddwod_d_wu_w,                  //!< Instruction 'xvaddwod_d_wu_w'.
    kIdXvaddwod_h_b,                     //!< Instruction 'xvaddwod_h_b'.
    kIdXvaddwod_h_bu,                    //!< Instruction 'xvaddwod_h_bu'.
    kIdXvaddwod_h_bu_b,                  //!< Instruction 'xvaddwod_h_bu_b'.
    kIdXvaddwod_q_d,                     //!< Instruction 'xvaddwod_q_d'.
    kIdXvaddwod_q_du,                    //!< Instruction 'xvaddwod_q_du'.
    kIdXvaddwod_q_du_d,                  //!< Instruction 'xvaddwod_q_du_d'.
    kIdXvaddwod_w_h,                     //!< Instruction 'xvaddwod_w_h'.
    kIdXvaddwod_w_hu,                    //!< Instruction 'xvaddwod_w_hu'.
    kIdXvaddwod_w_hu_h,                  //!< Instruction 'xvaddwod_w_hu_h'.
    kIdXvand_v,                          //!< Instruction 'xvand' {ASIMD}.
    kIdXvandi_b,                         //!< Instruction 'xvandi_b'.
    kIdXvandn_v,                         //!< Instruction 'xvandn' {ASIMD}.
    kIdXvavg_b,                          //!< Instruction 'xvavg_b'.
    kIdXvavg_bu,                         //!< Instruction 'xvavg_bu'.
    kIdXvavg_d,                          //!< Instruction 'xvavg_d'.
    kIdXvavg_du,                         //!< Instruction 'xvavg_du'.
    kIdXvavg_h,                          //!< Instruction 'xvavg_h'.
    kIdXvavg_hu,                         //!< Instruction 'xvavg_hu'.
    kIdXvavg_w,                          //!< Instruction 'xvavg_w'.
    kIdXvavg_wu,                         //!< Instruction 'xvavg_wu'.
    kIdXvavgr_b,                         //!< Instruction 'xvavgr_b'.
    kIdXvavgr_bu,                        //!< Instruction 'xvavgr_bu'.
    kIdXvavgr_d,                         //!< Instruction 'xvavgr_d'.
    kIdXvavgr_du,                        //!< Instruction 'xvavgr_du'.
    kIdXvavgr_h,                         //!< Instruction 'xvavgr_h'.
    kIdXvavgr_hu,                        //!< Instruction 'xvavgr_hu'.
    kIdXvavgr_w,                         //!< Instruction 'xvavgr_w'.
    kIdXvavgr_wu,                        //!< Instruction 'xvavgr_wu'.
    kIdXvbitclr_b,                       //!< Instruction 'xvbitclr_b'.
    kIdXvbitclr_d,                       //!< Instruction 'xvbitclr_d'.
    kIdXvbitclr_h,                       //!< Instruction 'xvbitclr_h'.
    kIdXvbitclr_w,                       //!< Instruction 'xvbitclr_w'.
    kIdXvbitclri_b,                      //!< Instruction 'xvbitclri_b'.
    kIdXvbitclri_d,                      //!< Instruction 'xvbitclri_d'.
    kIdXvbitclri_h,                      //!< Instruction 'xvbitclri_h'.
    kIdXvbitclri_w,                      //!< Instruction 'xvbitclri_w'.
    kIdXvbitrev_b,                       //!< Instruction 'xvbitrev_b'.
    kIdXvbitrev_d,                       //!< Instruction 'xvbitrev_d'.
    kIdXvbitrev_h,                       //!< Instruction 'xvbitrev_h'.
    kIdXvbitrev_w,                       //!< Instruction 'xvbitrev_w'.
    kIdXvbitrevi_b,                      //!< Instruction 'xvbitrevi_b'.
    kIdXvbitrevi_d,                      //!< Instruction 'xvbitrevi_d'.
    kIdXvbitrevi_h,                      //!< Instruction 'xvbitrevi_h'.
    kIdXvbitrevi_w,                      //!< Instruction 'xvbitrevi_w'.
    kIdXvbitsel_v,                       //!< Instruction 'xvbitsel' {ASIMD}.
    kIdXvbitseli_b,                      //!< Instruction 'xvbitseli_b'.
    kIdXvbitset_b,                       //!< Instruction 'xvbitset_b'.
    kIdXvbitset_d,                       //!< Instruction 'xvbitset_d'.
    kIdXvbitset_h,                       //!< Instruction 'xvbitset_h'.
    kIdXvbitset_w,                       //!< Instruction 'xvbitset_w'.
    kIdXvbitseti_b,                      //!< Instruction 'xvbitseti_b'.
    kIdXvbitseti_d,                      //!< Instruction 'xvbitseti_d'.
    kIdXvbitseti_h,                      //!< Instruction 'xvbitseti_h'.
    kIdXvbitseti_w,                      //!< Instruction 'xvbitseti_w'.
    kIdXvbsll_v,                         //!< Instruction 'xvbsll' {ASIMD}.
    kIdXvbsrl_v,                         //!< Instruction 'xvbsrl' {ASIMD}.
    kIdXvclo_b,                          //!< Instruction 'xvclo_b'.
    kIdXvclo_d,                          //!< Instruction 'xvclo_d'.
    kIdXvclo_h,                          //!< Instruction 'xvclo_h'.
    kIdXvclo_w,                          //!< Instruction 'xvclo_w'.
    kIdXvclz_b,                          //!< Instruction 'xvclz_b'.
    kIdXvclz_d,                          //!< Instruction 'xvclz_d'.
    kIdXvclz_h,                          //!< Instruction 'xvclz_h'.
    kIdXvclz_w,                          //!< Instruction 'xvclz_w'.
    kIdXvdiv_b,                          //!< Instruction 'xvdiv_b'.
    kIdXvdiv_bu,                         //!< Instruction 'xvdiv_bu'.
    kIdXvdiv_d,                          //!< Instruction 'xvdiv_d'.
    kIdXvdiv_du,                         //!< Instruction 'xvdiv_du'.
    kIdXvdiv_h,                          //!< Instruction 'xvdiv_h'.
    kIdXvdiv_hu,                         //!< Instruction 'xvdiv_hu'.
    kIdXvdiv_w,                          //!< Instruction 'xvdiv_w'.
    kIdXvdiv_wu,                         //!< Instruction 'xvdiv_wu'.
    kIdXvexth_d_w,                       //!< Instruction 'xvexth_d_w'.
    kIdXvexth_du_wu,                     //!< Instruction 'xvexth_du_wu'.
    kIdXvexth_h_b,                       //!< Instruction 'xvexth_h_b'.
    kIdXvexth_hu_bu,                     //!< Instruction 'xvexth_hu_bu'.
    kIdXvexth_q_d,                       //!< Instruction 'xvexth_q_d'.
    kIdXvexth_qu_du,                     //!< Instruction 'xvexth_qu_du'.
    kIdXvexth_w_h,                       //!< Instruction 'xvexth_w_h'.
    kIdXvexth_wu_hu,                     //!< Instruction 'xvexth_wu_hu'.
    kIdXvextl_q_d,                       //!< Instruction 'xvextl_q_d'.
    kIdXvextl_qu_du,                     //!< Instruction 'xvextl_qu_du'.
    kIdXvextrins_b,                      //!< Instruction 'xvextrins_b'.
    kIdXvextrins_d,                      //!< Instruction 'xvextrins_d'.
    kIdXvextrins_h,                      //!< Instruction 'xvextrins_h'.
    kIdXvextrins_w,                      //!< Instruction 'xvextrins_w'.
    kIdXvfadd_d,                         //!< Instruction 'xvfadd_d'.
    kIdXvfadd_s,                         //!< Instruction 'xvfadd_s'.
    kIdXvfclass_d,                       //!< Instruction 'xvfclass_d'.
    kIdXvfclass_s,                       //!< Instruction 'xvfclass_s'.
    kIdXvfcvt_h_s,                       //!< Instruction 'xvfcvt_h_s'.
    kIdXvfcvt_s_d,                       //!< Instruction 'xvfcvt_s_d'.
    kIdXvfcvth_d_s,                      //!< Instruction 'xvfcvth_d_s'.
    kIdXvfcvth_s_h,                      //!< Instruction 'xvfcvth_s_h'.
    kIdXvfcvtl_d_s,                      //!< Instruction 'xvfcvtl_d_s'.
    kIdXvfcvtl_s_h,                      //!< Instruction 'xvfcvtl_s_h'.
    kIdXvfdiv_d,                         //!< Instruction 'xvfdiv_d'.
    kIdXvfdiv_s,                         //!< Instruction 'xvfdiv_s'.
    kIdXvffint_d_l,                      //!< Instruction 'xvffint_d_l'.
    kIdXvffint_d_lu,                     //!< Instruction 'xvffint_d_lu'.
    kIdXvffint_s_l,                      //!< Instruction 'xvffint_s_l'.
    kIdXvffint_s_w,                      //!< Instruction 'xvffint_s_w'.
    kIdXvffint_s_wu,                     //!< Instruction 'xvffint_s_wu'.
    kIdXvffinth_d_w,                     //!< Instruction 'xvffinth_d_w'.
    kIdXvffintl_d_w,                     //!< Instruction 'xvffintl_d_w'.
    kIdXvflogb_d,                        //!< Instruction 'xvflogb_d'.
    kIdXvflogb_s,                        //!< Instruction 'xvflogb_s'.
    kIdXvfmadd_d,                        //!< Instruction 'xvfmadd_d'.
    kIdXvfmadd_s,                        //!< Instruction 'xvfmadd_s'.
    kIdXvfmax_d,                         //!< Instruction 'xvfmax_d'.
    kIdXvfmax_s,                         //!< Instruction 'xvfmax_s'.
    kIdXvfmaxa_d,                        //!< Instruction 'xvfmaxa_d'.
    kIdXvfmaxa_s,                        //!< Instruction 'xvfmaxa_s'.
    kIdXvfmin_d,                         //!< Instruction 'xvfmin_d'.
    kIdXvfmin_s,                         //!< Instruction 'xvfmin_s'.
    kIdXvfmina_d,                        //!< Instruction 'xvfmina_d'.
    kIdXvfmina_s,                        //!< Instruction 'xvfmina_s'.
    kIdXvfmsub_d,                        //!< Instruction 'xvfmsub_d'.
    kIdXvfmsub_s,                        //!< Instruction 'xvfmsub_s'.
    kIdXvfmul_d,                         //!< Instruction 'xvfmul_d'.
    kIdXvfmul_s,                         //!< Instruction 'xvfmul_s'.
    kIdXvfnmadd_d,                       //!< Instruction 'xvfnmadd_d'.
    kIdXvfnmadd_s,                       //!< Instruction 'xvfnmadd_s'.
    kIdXvfnmsub_d,                       //!< Instruction 'xvfnmsub_d'.
    kIdXvfnmsub_s,                       //!< Instruction 'xvfnmsub_s'.
    kIdXvfrecip_d,                       //!< Instruction 'xvfrecip_d'.
    kIdXvfrecip_s,                       //!< Instruction 'xvfrecip_s'.
    kIdXvfrint_d,                        //!< Instruction 'xvfrint_d'.
    kIdXvfrint_s,                        //!< Instruction 'xvfrint_s'.
    kIdXvfrintrm_d,                      //!< Instruction 'xvfrintrm_d'.
    kIdXvfrintrm_s,                      //!< Instruction 'xvfrintrm_s'.
    kIdXvfrintrne_d,                     //!< Instruction 'xvfrintrne_d'.
    kIdXvfrintrne_s,                     //!< Instruction 'xvfrintrne_s'.
    kIdXvfrintrp_d,                      //!< Instruction 'xvfrintrp_d'.
    kIdXvfrintrp_s,                      //!< Instruction 'xvfrintrp_s'.
    kIdXvfrintrz_d,                      //!< Instruction 'xvfrintrz_d'.
    kIdXvfrintrz_s,                      //!< Instruction 'xvfrintrz_s'.
    kIdXvfrsqrt_d,                       //!< Instruction 'xvfrsqrt_d'.
    kIdXvfrsqrt_s,                       //!< Instruction 'xvfrsqrt_s'.
    kIdXvfrstp_b,                        //!< Instruction 'xvfrstp_b'.
    kIdXvfrstp_h,                        //!< Instruction 'xvfrstp_h'.
    kIdXvfrstpi_b,                       //!< Instruction 'xvfrstpi_b'.
    kIdXvfrstpi_h,                       //!< Instruction 'xvfrstpi_h'.
    kIdXvfscaleb_d,                      //!< Instruction 'xvfscaleb_d'.
    kIdXvfscaleb_s,                      //!< Instruction 'xvfscaleb_s'.
    kIdXvfsqrt_d,                        //!< Instruction 'xvfsqrt_d'.
    kIdXvfsqrt_s,                        //!< Instruction 'xvfsqrt_s'.
    kIdXvfsub_d,                         //!< Instruction 'xvfsub_d'.
    kIdXvfsub_s,                         //!< Instruction 'xvfsub_s'.
    kIdXvftint_l_d,                      //!< Instruction 'xvftint_l_d'.
    kIdXvftint_lu_d,                     //!< Instruction 'xvftint_lu_d'.
    kIdXvftint_w_d,                      //!< Instruction 'xvftint_w_d'.
    kIdXvftint_w_s,                      //!< Instruction 'xvftint_w_s'.
    kIdXvftint_wu_s,                     //!< Instruction 'xvftint_wu_s'.
    kIdXvftinth_l_s,                     //!< Instruction 'xvftinth_l_s'.
    kIdXvftintl_l_s,                     //!< Instruction 'xvftintl_l_s'.
    kIdXvftintrm_l_d,                    //!< Instruction 'xvftintrm_l_d'.
    kIdXvftintrm_w_d,                    //!< Instruction 'xvftintrm_w_d'.
    kIdXvftintrm_w_s,                    //!< Instruction 'xvftintrm_w_s'.
    kIdXvftintrmh_l_s,                   //!< Instruction 'xvftintrmh_l_s'.
    kIdXvftintrml_l_s,                   //!< Instruction 'xvftintrml_l_s'.
    kIdXvftintrne_l_d,                   //!< Instruction 'xvftintrne_l_d'.
    kIdXvftintrne_w_d,                   //!< Instruction 'xvftintrne_w_d'.
    kIdXvftintrne_w_s,                   //!< Instruction 'xvftintrne_w_s'.
    kIdXvftintrneh_l_s,                  //!< Instruction 'xvftintrneh_l_s'.
    kIdXvftintrnel_l_s,                  //!< Instruction 'xvftintrnel_l_s'.
    kIdXvftintrp_l_d,                    //!< Instruction 'xvftintrp_l_d'.
    kIdXvftintrp_w_d,                    //!< Instruction 'xvftintrp_w_d'.
    kIdXvftintrp_w_s,                    //!< Instruction 'xvftintrp_w_s'.
    kIdXvftintrph_l_s,                   //!< Instruction 'xvftintrph_l_s'.
    kIdXvftintrpl_l_s,                   //!< Instruction 'xvftintrpl_l_s'.
    kIdXvftintrz_l_d,                    //!< Instruction 'xvftintrz_l_d'.
    kIdXvftintrz_lu_d,                   //!< Instruction 'xvftintrz_lu_d'.
    kIdXvftintrz_w_d,                    //!< Instruction 'xvftintrz_w_d'.
    kIdXvftintrz_w_s,                    //!< Instruction 'xvftintrz_w_s'.
    kIdXvftintrz_wu_s,                   //!< Instruction 'xvftintrz_wu_s'.
    kIdXvftintrzh_l_s,                   //!< Instruction 'xvftintrzh_l_s'.
    kIdXvftintrzl_l_s,                   //!< Instruction 'xvftintrzl_l_s'.
    kIdXvhaddw_d_w,                      //!< Instruction 'xvhaddw_d_w'.
    kIdXvhaddw_du_wu,                    //!< Instruction 'xvhaddw_du_wu'.
    kIdXvhaddw_h_b,                      //!< Instruction 'xvhaddw_h_b'.
    kIdXvhaddw_hu_bu,                    //!< Instruction 'xvhaddw_hu_bu'.
    kIdXvhaddw_q_d,                      //!< Instruction 'xvhaddw_q_d'.
    kIdXvhaddw_qu_du,                    //!< Instruction 'xvhaddw_qu_du'.
    kIdXvhaddw_w_h,                      //!< Instruction 'xvhaddw_w_h'.
    kIdXvhaddw_wu_hu,                    //!< Instruction 'xvhaddw_wu_hu'.
    kIdXvhseli_d,                        //!< Instruction 'xvhseli_d'.
    kIdXvhsubw_d_w,                      //!< Instruction 'xvhsubw_d_w'.
    kIdXvhsubw_du_wu,                    //!< Instruction 'xvhsubw_du_wu'.
    kIdXvhsubw_h_b,                      //!< Instruction 'xvhsubw_h_b'.
    kIdXvhsubw_hu_bu,                    //!< Instruction 'xvhsubw_hu_bu'.
    kIdXvhsubw_q_d,                      //!< Instruction 'xvhsubw_q_d'.
    kIdXvhsubw_qu_du,                    //!< Instruction 'xvhsubw_qu_du'.
    kIdXvhsubw_w_h,                      //!< Instruction 'xvhsubw_w_h'.
    kIdXvhsubw_wu_hu,                    //!< Instruction 'xvhsubw_wu_hu'.
    kIdXvilvh_b,                         //!< Instruction 'xvilvh_b'.
    kIdXvilvh_d,                         //!< Instruction 'xvilvh_d'.
    kIdXvilvh_h,                         //!< Instruction 'xvilvh_h'.
    kIdXvilvh_w,                         //!< Instruction 'xvilvh_w'.
    kIdXvilvl_b,                         //!< Instruction 'xvilvl_b'.
    kIdXvilvl_d,                         //!< Instruction 'xvilvl_d'.
    kIdXvilvl_h,                         //!< Instruction 'xvilvl_h'.
    kIdXvilvl_w,                         //!< Instruction 'xvilvl_w'.
    kIdXvinsgr2vr_d,                     //!< Instruction 'xvinsgr2vr_d'.
    kIdXvinsgr2vr_w,                     //!< Instruction 'xvinsgr2vr_w'.
    kIdXvinsve0_d,                       //!< Instruction 'xvinsve0_d'.
    kIdXvinsve0_w,                       //!< Instruction 'xvinsve0_w'.
    kIdXvld,                             //!< Instruction 'xvld'.
    kIdXvldi,                            //!< Instruction 'xvldi'.
    kIdXvldrepl_b,                       //!< Instruction 'xvldrepl_b'.
    kIdXvldrepl_d,                       //!< Instruction 'xvldrepl_d'.
    kIdXvldrepl_h,                       //!< Instruction 'xvldrepl_h'.
    kIdXvldrepl_w,                       //!< Instruction 'xvldrepl_w'.
    kIdXvldx,                            //!< Instruction 'xvldx'.
    kIdXvmadd_b,                         //!< Instruction 'xvmadd_b'.
    kIdXvmadd_d,                         //!< Instruction 'xvmadd_d'.
    kIdXvmadd_h,                         //!< Instruction 'xvmadd_h'.
    kIdXvmadd_w,                         //!< Instruction 'xvmadd_w'.
    kIdXvmaddwev_d_w,                    //!< Instruction 'xvmaddwev_d_w'.
    kIdXvmaddwev_d_wu,                   //!< Instruction 'xvmaddwev_d_wu'.
    kIdXvmaddwev_d_wu_w,                 //!< Instruction 'xvmaddwev_d_wu_w'.
    kIdXvmaddwev_h_b,                    //!< Instruction 'xvmaddwev_h_b'.
    kIdXvmaddwev_h_bu,                   //!< Instruction 'xvmaddwev_h_bu'.
    kIdXvmaddwev_h_bu_b,                 //!< Instruction 'xvmaddwev_h_bu_b'.
    kIdXvmaddwev_q_d,                    //!< Instruction 'xvmaddwev_q_d'.
    kIdXvmaddwev_q_du,                   //!< Instruction 'xvmaddwev_q_du'.
    kIdXvmaddwev_q_du_d,                 //!< Instruction 'xvmaddwev_q_du_d'.
    kIdXvmaddwev_w_h,                    //!< Instruction 'xvmaddwev_w_h'.
    kIdXvmaddwev_w_hu,                   //!< Instruction 'xvmaddwev_w_hu'.
    kIdXvmaddwev_w_hu_h,                 //!< Instruction 'xvmaddwev_w_hu_h'.
    kIdXvmaddwod_d_w,                    //!< Instruction 'xvmaddwod_d_w'.
    kIdXvmaddwod_d_wu,                   //!< Instruction 'xvmaddwod_d_wu'.
    kIdXvmaddwod_d_wu_w,                 //!< Instruction 'xvmaddwod_d_wu_w'.
    kIdXvmaddwod_h_b,                    //!< Instruction 'xvmaddwod_h_b'.
    kIdXvmaddwod_h_bu,                   //!< Instruction 'xvmaddwod_h_bu'.
    kIdXvmaddwod_h_bu_b,                 //!< Instruction 'xvmaddwod_h_bu_b'.
    kIdXvmaddwod_q_d,                    //!< Instruction 'xvmaddwod_q_d'.
    kIdXvmaddwod_q_du,                   //!< Instruction 'xvmaddwod_q_du'.
    kIdXvmaddwod_q_du_d,                 //!< Instruction 'xvmaddwod_q_du_d'.
    kIdXvmaddwod_w_h,                    //!< Instruction 'xvmaddwod_w_h'.
    kIdXvmaddwod_w_hu,                   //!< Instruction 'xvmaddwod_w_hu'.
    kIdXvmaddwod_w_hu_h,                 //!< Instruction 'xvmaddwod_w_hu_h'.
    kIdXvmax_b,                          //!< Instruction 'xvmax_b'.
    kIdXvmax_bu,                         //!< Instruction 'xvmax_bu'.
    kIdXvmax_d,                          //!< Instruction 'xvmax_d'.
    kIdXvmax_du,                         //!< Instruction 'xvmax_du'.
    kIdXvmax_h,                          //!< Instruction 'xvmax_h'.
    kIdXvmax_hu,                         //!< Instruction 'xvmax_hu'.
    kIdXvmax_w,                          //!< Instruction 'xvmax_w'.
    kIdXvmax_wu,                         //!< Instruction 'xvmax_wu'.
    kIdXvmaxi_b,                         //!< Instruction 'xvmaxi_b'.
    kIdXvmaxi_bu,                        //!< Instruction 'xvmaxi_bu'.
    kIdXvmaxi_d,                         //!< Instruction 'xvmaxi_d'.
    kIdXvmaxi_du,                        //!< Instruction 'xvmaxi_du'.
    kIdXvmaxi_h,                         //!< Instruction 'xvmaxi_h'.
    kIdXvmaxi_hu,                        //!< Instruction 'xvmaxi_hu'.
    kIdXvmaxi_w,                         //!< Instruction 'xvmaxi_w'.
    kIdXvmaxi_wu,                        //!< Instruction 'xvmaxi_wu'.
    kIdXvmepatmsk_v,                     //!< Instruction 'xvmepatmsk' {ASIMD}.
    kIdXvmin_b,                          //!< Instruction 'xvmin_b'.
    kIdXvmin_bu,                         //!< Instruction 'xvmin_bu'.
    kIdXvmin_d,                          //!< Instruction 'xvmin_d'.
    kIdXvmin_du,                         //!< Instruction 'xvmin_du'.
    kIdXvmin_h,                          //!< Instruction 'xvmin_h'.
    kIdXvmin_hu,                         //!< Instruction 'xvmin_hu'.
    kIdXvmin_w,                          //!< Instruction 'xvmin_w'.
    kIdXvmin_wu,                         //!< Instruction 'xvmin_wu'.
    kIdXvmini_b,                         //!< Instruction 'xvmini_b'.
    kIdXvmini_bu,                        //!< Instruction 'xvmini_bu'.
    kIdXvmini_d,                         //!< Instruction 'xvmini_d'.
    kIdXvmini_du,                        //!< Instruction 'xvmini_du'.
    kIdXvmini_h,                         //!< Instruction 'xvmini_h'.
    kIdXvmini_hu,                        //!< Instruction 'xvmini_hu'.
    kIdXvmini_w,                         //!< Instruction 'xvmini_w'.
    kIdXvmini_wu,                        //!< Instruction 'xvmini_wu'.
    kIdXvmod_b,                          //!< Instruction 'xvmod_b'.
    kIdXvmod_bu,                         //!< Instruction 'xvmod_bu'.
    kIdXvmod_d,                          //!< Instruction 'xvmod_d'.
    kIdXvmod_du,                         //!< Instruction 'xvmod_du'.
    kIdXvmod_h,                          //!< Instruction 'xvmod_h'.
    kIdXvmod_hu,                         //!< Instruction 'xvmod_hu'.
    kIdXvmod_w,                          //!< Instruction 'xvmod_w'.
    kIdXvmod_wu,                         //!< Instruction 'xvmod_wu'.
    kIdXvmskgez_b,                       //!< Instruction 'xvmskgez_b'.
    kIdXvmskltz_b,                       //!< Instruction 'xvmskltz_b'.
    kIdXvmskltz_d,                       //!< Instruction 'xvmskltz_d'.
    kIdXvmskltz_h,                       //!< Instruction 'xvmskltz_h'.
    kIdXvmskltz_w,                       //!< Instruction 'xvmskltz_w'.
    kIdXvmsknz_b,                        //!< Instruction 'xvmsknz_b'.
    kIdXvmsub_b,                         //!< Instruction 'xvmsub_b'.
    kIdXvmsub_d,                         //!< Instruction 'xvmsub_d'.
    kIdXvmsub_h,                         //!< Instruction 'xvmsub_h'.
    kIdXvmsub_w,                         //!< Instruction 'xvmsub_w'.
    kIdXvmuh_b,                          //!< Instruction 'xvmuh_b'.
    kIdXvmuh_bu,                         //!< Instruction 'xvmuh_bu'.
    kIdXvmuh_d,                          //!< Instruction 'xvmuh_d'.
    kIdXvmuh_du,                         //!< Instruction 'xvmuh_du'.
    kIdXvmuh_h,                          //!< Instruction 'xvmuh_h'.
    kIdXvmuh_hu,                         //!< Instruction 'xvmuh_hu'.
    kIdXvmuh_w,                          //!< Instruction 'xvmuh_w'.
    kIdXvmuh_wu,                         //!< Instruction 'xvmuh_wu'.
    kIdXvmul_b,                          //!< Instruction 'xvmul_b'.
    kIdXvmul_d,                          //!< Instruction 'xvmul_d'.
    kIdXvmul_h,                          //!< Instruction 'xvmul_h'.
    kIdXvmul_w,                          //!< Instruction 'xvmul_w'.
    kIdXvmulwev_d_w,                     //!< Instruction 'xvmulwev_d_w'.
    kIdXvmulwev_d_wu,                    //!< Instruction 'xvmulwev_d_wu'.
    kIdXvmulwev_d_wu_w,                  //!< Instruction 'xvmulwev_d_wu_w'.
    kIdXvmulwev_h_b,                     //!< Instruction 'xvmulwev_h_b'.
    kIdXvmulwev_h_bu,                    //!< Instruction 'xvmulwev_h_bu'.
    kIdXvmulwev_h_bu_b,                  //!< Instruction 'xvmulwev_h_bu_b'.
    kIdXvmulwev_q_d,                     //!< Instruction 'xvmulwev_q_d'.
    kIdXvmulwev_q_du,                    //!< Instruction 'xvmulwev_q_du'.
    kIdXvmulwev_q_du_d,                  //!< Instruction 'xvmulwev_q_du_d'.
    kIdXvmulwev_w_h,                     //!< Instruction 'xvmulwev_w_h'.
    kIdXvmulwev_w_hu,                    //!< Instruction 'xvmulwev_w_hu'.
    kIdXvmulwev_w_hu_h,                  //!< Instruction 'xvmulwev_w_hu_h'.
    kIdXvmulwod_d_w,                     //!< Instruction 'xvmulwod_d_w'.
    kIdXvmulwod_d_wu,                    //!< Instruction 'xvmulwod_d_wu'.
    kIdXvmulwod_d_wu_w,                  //!< Instruction 'xvmulwod_d_wu_w'.
    kIdXvmulwod_h_b,                     //!< Instruction 'xvmulwod_h_b'.
    kIdXvmulwod_h_bu,                    //!< Instruction 'xvmulwod_h_bu'.
    kIdXvmulwod_h_bu_b,                  //!< Instruction 'xvmulwod_h_bu_b'.
    kIdXvmulwod_q_d,                     //!< Instruction 'xvmulwod_q_d'.
    kIdXvmulwod_q_du,                    //!< Instruction 'xvmulwod_q_du'.
    kIdXvmulwod_q_du_d,                  //!< Instruction 'xvmulwod_q_du_d'.
    kIdXvmulwod_w_h,                     //!< Instruction 'xvmulwod_w_h'.
    kIdXvmulwod_w_hu,                    //!< Instruction 'xvmulwod_w_hu'.
    kIdXvmulwod_w_hu_h,                  //!< Instruction 'xvmulwod_w_hu_h'.
    kIdXvneg_b,                          //!< Instruction 'xvneg_b'.
    kIdXvneg_d,                          //!< Instruction 'xvneg_d'.
    kIdXvneg_h,                          //!< Instruction 'xvneg_h'.
    kIdXvneg_w,                          //!< Instruction 'xvneg_w'.
    kIdXvnor_v,                          //!< Instruction 'xvnor' {ASIMD}.
    kIdXvnori_b,                         //!< Instruction 'xvnori_b'.
    kIdXvor_v,                           //!< Instruction 'xvor' {ASIMD}.
    kIdXvori_b,                          //!< Instruction 'xvori_b'.
    kIdXvorn_v,                          //!< Instruction 'xvorn' {ASIMD}.
    kIdXvpackev_b,                       //!< Instruction 'xvpackev_b'.
    kIdXvpackev_d,                       //!< Instruction 'xvpackev_d'.
    kIdXvpackev_h,                       //!< Instruction 'xvpackev_h'.
    kIdXvpackev_w,                       //!< Instruction 'xvpackev_w'.
    kIdXvpackod_b,                       //!< Instruction 'xvpackod_b'.
    kIdXvpackod_d,                       //!< Instruction 'xvpackod_d'.
    kIdXvpackod_h,                       //!< Instruction 'xvpackod_h'.
    kIdXvpackod_w,                       //!< Instruction 'xvpackod_w'.
    kIdXvpcnt_b,                         //!< Instruction 'xvpcnt_b'.
    kIdXvpcnt_d,                         //!< Instruction 'xvpcnt_d'.
    kIdXvpcnt_h,                         //!< Instruction 'xvpcnt_h'.
    kIdXvpcnt_w,                         //!< Instruction 'xvpcnt_w'.
    kIdXvperm_w,                         //!< Instruction 'xvperm_w'.
    kIdXvpermi_d,                        //!< Instruction 'xvpermi_d'.
    kIdXvpermi_q,                        //!< Instruction 'xvpermi_q'.
    kIdXvpermi_w,                        //!< Instruction 'xvpermi_w'.
    kIdXvpickev_b,                       //!< Instruction 'xvpickev_b'.
    kIdXvpickev_d,                       //!< Instruction 'xvpickev_d'.
    kIdXvpickev_h,                       //!< Instruction 'xvpickev_h'.
    kIdXvpickev_w,                       //!< Instruction 'xvpickev_w'.
    kIdXvpickod_b,                       //!< Instruction 'xvpickod_b'.
    kIdXvpickod_d,                       //!< Instruction 'xvpickod_d'.
    kIdXvpickod_h,                       //!< Instruction 'xvpickod_h'.
    kIdXvpickod_w,                       //!< Instruction 'xvpickod_w'.
    kIdXvpickve2gr_d,                    //!< Instruction 'xvpickve2gr_d'.
    kIdXvpickve2gr_du,                   //!< Instruction 'xvpickve2gr_du'.
    kIdXvpickve2gr_w,                    //!< Instruction 'xvpickve2gr_w'.
    kIdXvpickve2gr_wu,                   //!< Instruction 'xvpickve2gr_wu'.
    kIdXvpickve_d,                       //!< Instruction 'xvpickve_d'.
    kIdXvpickve_w,                       //!< Instruction 'xvpickve_w'.
    kIdXvrepl128vei_b,                   //!< Instruction 'xvrepl128vei_b'.
    kIdXvrepl128vei_d,                   //!< Instruction 'xvrepl128vei_d'.
    kIdXvrepl128vei_h,                   //!< Instruction 'xvrepl128vei_h'.
    kIdXvrepl128vei_w,                   //!< Instruction 'xvrepl128vei_w'.
    kIdXvreplgr2vr_b,                    //!< Instruction 'xvreplgr2vr_b'.
    kIdXvreplgr2vr_d,                    //!< Instruction 'xvreplgr2vr_d'.
    kIdXvreplgr2vr_h,                    //!< Instruction 'xvreplgr2vr_h'.
    kIdXvreplgr2vr_w,                    //!< Instruction 'xvreplgr2vr_w'.
    kIdXvreplve0_b,                      //!< Instruction 'xvreplve0_b'.
    kIdXvreplve0_d,                      //!< Instruction 'xvreplve0_d'.
    kIdXvreplve0_h,                      //!< Instruction 'xvreplve0_h'.
    kIdXvreplve0_q,                      //!< Instruction 'xvreplve0_q'.
    kIdXvreplve0_w,                      //!< Instruction 'xvreplve0_w'.
    kIdXvreplve_b,                       //!< Instruction 'xvreplve_b'.
    kIdXvreplve_d,                       //!< Instruction 'xvreplve_d'.
    kIdXvreplve_h,                       //!< Instruction 'xvreplve_h'.
    kIdXvreplve_w,                       //!< Instruction 'xvreplve_w'.
    kIdXvrotr_b,                         //!< Instruction 'xvrotr_b'.
    kIdXvrotr_d,                         //!< Instruction 'xvrotr_d'.
    kIdXvrotr_h,                         //!< Instruction 'xvrotr_h'.
    kIdXvrotr_w,                         //!< Instruction 'xvrotr_w'.
    kIdXvrotri_b,                        //!< Instruction 'xvrotri_b'.
    kIdXvrotri_d,                        //!< Instruction 'xvrotri_d'.
    kIdXvrotri_h,                        //!< Instruction 'xvrotri_h'.
    kIdXvrotri_w,                        //!< Instruction 'xvrotri_w'.
    kIdXvsadd_b,                         //!< Instruction 'xvsadd_b'.
    kIdXvsadd_bu,                        //!< Instruction 'xvsadd_bu'.
    kIdXvsadd_d,                         //!< Instruction 'xvsadd_d'.
    kIdXvsadd_du,                        //!< Instruction 'xvsadd_du'.
    kIdXvsadd_h,                         //!< Instruction 'xvsadd_h'.
    kIdXvsadd_hu,                        //!< Instruction 'xvsadd_hu'.
    kIdXvsadd_w,                         //!< Instruction 'xvsadd_w'.
    kIdXvsadd_wu,                        //!< Instruction 'xvsadd_wu'.
    kIdXvsat_b,                          //!< Instruction 'xvsat_b'.
    kIdXvsat_bu,                         //!< Instruction 'xvsat_bu'.
    kIdXvsat_d,                          //!< Instruction 'xvsat_d'.
    kIdXvsat_du,                         //!< Instruction 'xvsat_du'.
    kIdXvsat_h,                          //!< Instruction 'xvsat_h'.
    kIdXvsat_hu,                         //!< Instruction 'xvsat_hu'.
    kIdXvsat_w,                          //!< Instruction 'xvsat_w'.
    kIdXvsat_wu,                         //!< Instruction 'xvsat_wu'.
    kIdXvseq_b,                          //!< Instruction 'xvseq_b'.
    kIdXvseq_d,                          //!< Instruction 'xvseq_d'.
    kIdXvseq_h,                          //!< Instruction 'xvseq_h'.
    kIdXvseq_w,                          //!< Instruction 'xvseq_w'.
    kIdXvseqi_b,                         //!< Instruction 'xvseqi_b'.
    kIdXvseqi_d,                         //!< Instruction 'xvseqi_d'.
    kIdXvseqi_h,                         //!< Instruction 'xvseqi_h'.
    kIdXvseqi_w,                         //!< Instruction 'xvseqi_w'.
    kIdXvsetallnez_b,                    //!< Instruction 'xvsetallnez_b'.
    kIdXvsetallnez_d,                    //!< Instruction 'xvsetallnez_d'.
    kIdXvsetallnez_h,                    //!< Instruction 'xvsetallnez_h'.
    kIdXvsetallnez_w,                    //!< Instruction 'xvsetallnez_w'.
    kIdXvsetanyeqz_b,                    //!< Instruction 'xvsetanyeqz_b'.
    kIdXvsetanyeqz_d,                    //!< Instruction 'xvsetanyeqz_d'.
    kIdXvsetanyeqz_h,                    //!< Instruction 'xvsetanyeqz_h'.
    kIdXvsetanyeqz_w,                    //!< Instruction 'xvsetanyeqz_w'.
    kIdXvseteqz_v,                       //!< Instruction 'xvseteqz' {ASIMD}.
    kIdXvsetnez_v,                       //!< Instruction 'xvsetnez' {ASIMD}.
    kIdXvshuf4i_b,                       //!< Instruction 'xvshuf4i_b'.
    kIdXvshuf4i_d,                       //!< Instruction 'xvshuf4i_d'.
    kIdXvshuf4i_h,                       //!< Instruction 'xvshuf4i_h'.
    kIdXvshuf4i_w,                       //!< Instruction 'xvshuf4i_w'.
    kIdXvshuf_b,                         //!< Instruction 'xvshuf_b'.
    kIdXvshuf_d,                         //!< Instruction 'xvshuf_d'.
    kIdXvshuf_h,                         //!< Instruction 'xvshuf_h'.
    kIdXvshuf_w,                         //!< Instruction 'xvshuf_w'.
    kIdXvsigncov_b,                      //!< Instruction 'xvsigncov_b'.
    kIdXvsigncov_d,                      //!< Instruction 'xvsigncov_d'.
    kIdXvsigncov_h,                      //!< Instruction 'xvsigncov_h'.
    kIdXvsigncov_w,                      //!< Instruction 'xvsigncov_w'.
    kIdXvsle_b,                          //!< Instruction 'xvsle_b'.
    kIdXvsle_bu,                         //!< Instruction 'xvsle_bu'.
    kIdXvsle_d,                          //!< Instruction 'xvsle_d'.
    kIdXvsle_du,                         //!< Instruction 'xvsle_du'.
    kIdXvsle_h,                          //!< Instruction 'xvsle_h'.
    kIdXvsle_hu,                         //!< Instruction 'xvsle_hu'.
    kIdXvsle_w,                          //!< Instruction 'xvsle_w'.
    kIdXvsle_wu,                         //!< Instruction 'xvsle_wu'.
    kIdXvslei_b,                         //!< Instruction 'xvslei_b'.
    kIdXvslei_bu,                        //!< Instruction 'xvslei_bu'.
    kIdXvslei_d,                         //!< Instruction 'xvslei_d'.
    kIdXvslei_du,                        //!< Instruction 'xvslei_du'.
    kIdXvslei_h,                         //!< Instruction 'xvslei_h'.
    kIdXvslei_hu,                        //!< Instruction 'xvslei_hu'.
    kIdXvslei_w,                         //!< Instruction 'xvslei_w'.
    kIdXvslei_wu,                        //!< Instruction 'xvslei_wu'.
    kIdXvsll_b,                          //!< Instruction 'xvsll_b'.
    kIdXvsll_d,                          //!< Instruction 'xvsll_d'.
    kIdXvsll_h,                          //!< Instruction 'xvsll_h'.
    kIdXvsll_w,                          //!< Instruction 'xvsll_w'.
    kIdXvslli_b,                         //!< Instruction 'xvslli_b'.
    kIdXvslli_d,                         //!< Instruction 'xvslli_d'.
    kIdXvslli_h,                         //!< Instruction 'xvslli_h'.
    kIdXvslli_w,                         //!< Instruction 'xvslli_w'.
    kIdXvsllwil_d_w,                     //!< Instruction 'xvsllwil_d_w'.
    kIdXvsllwil_du_wu,                   //!< Instruction 'xvsllwil_du_wu'.
    kIdXvsllwil_h_b,                     //!< Instruction 'xvsllwil_h_b'.
    kIdXvsllwil_hu_bu,                   //!< Instruction 'xvsllwil_hu_bu'.
    kIdXvsllwil_w_h,                     //!< Instruction 'xvsllwil_w_h'.
    kIdXvsllwil_wu_hu,                   //!< Instruction 'xvsllwil_wu_hu'.
    kIdXvslt_b,                          //!< Instruction 'xvslt_b'.
    kIdXvslt_bu,                         //!< Instruction 'xvslt_bu'.
    kIdXvslt_d,                          //!< Instruction 'xvslt_d'.
    kIdXvslt_du,                         //!< Instruction 'xvslt_du'.
    kIdXvslt_h,                          //!< Instruction 'xvslt_h'.
    kIdXvslt_hu,                         //!< Instruction 'xvslt_hu'.
    kIdXvslt_w,                          //!< Instruction 'xvslt_w'.
    kIdXvslt_wu,                         //!< Instruction 'xvslt_wu'.
    kIdXvslti_b,                         //!< Instruction 'xvslti_b'.
    kIdXvslti_bu,                        //!< Instruction 'xvslti_bu'.
    kIdXvslti_d,                         //!< Instruction 'xvslti_d'.
    kIdXvslti_du,                        //!< Instruction 'xvslti_du'.
    kIdXvslti_h,                         //!< Instruction 'xvslti_h'.
    kIdXvslti_hu,                        //!< Instruction 'xvslti_hu'.
    kIdXvslti_w,                         //!< Instruction 'xvslti_w'.
    kIdXvslti_wu,                        //!< Instruction 'xvslti_wu'.
    kIdXvsra_b,                          //!< Instruction 'xvsra_b'.
    kIdXvsra_d,                          //!< Instruction 'xvsra_d'.
    kIdXvsra_h,                          //!< Instruction 'xvsra_h'.
    kIdXvsra_w,                          //!< Instruction 'xvsra_w'.
    kIdXvsrai_b,                         //!< Instruction 'xvsrai_b'.
    kIdXvsrai_d,                         //!< Instruction 'xvsrai_d'.
    kIdXvsrai_h,                         //!< Instruction 'xvsrai_h'.
    kIdXvsrai_w,                         //!< Instruction 'xvsrai_w'.
    kIdXvsran_b_h,                       //!< Instruction 'xvsran_b_h'.
    kIdXvsran_h_w,                       //!< Instruction 'xvsran_h_w'.
    kIdXvsran_w_d,                       //!< Instruction 'xvsran_w_d'.
    kIdXvsrani_b_h,                      //!< Instruction 'xvsrani_b_h'.
    kIdXvsrani_d_q,                      //!< Instruction 'xvsrani_d_q'.
    kIdXvsrani_h_w,                      //!< Instruction 'xvsrani_h_w'.
    kIdXvsrani_w_d,                      //!< Instruction 'xvsrani_w_d'.
    kIdXvsrar_b,                         //!< Instruction 'xvsrar_b'.
    kIdXvsrar_d,                         //!< Instruction 'xvsrar_d'.
    kIdXvsrar_h,                         //!< Instruction 'xvsrar_h'.
    kIdXvsrar_w,                         //!< Instruction 'xvsrar_w'.
    kIdXvsrari_b,                        //!< Instruction 'xvsrari_b'.
    kIdXvsrari_d,                        //!< Instruction 'xvsrari_d'.
    kIdXvsrari_h,                        //!< Instruction 'xvsrari_h'.
    kIdXvsrari_w,                        //!< Instruction 'xvsrari_w'.
    kIdXvsrarn_b_h,                      //!< Instruction 'xvsrarn_b_h'.
    kIdXvsrarn_h_w,                      //!< Instruction 'xvsrarn_h_w'.
    kIdXvsrarn_w_d,                      //!< Instruction 'xvsrarn_w_d'.
    kIdXvsrarni_b_h,                     //!< Instruction 'xvsrarni_b_h'.
    kIdXvsrarni_d_q,                     //!< Instruction 'xvsrarni_d_q'.
    kIdXvsrarni_h_w,                     //!< Instruction 'xvsrarni_h_w'.
    kIdXvsrarni_w_d,                     //!< Instruction 'xvsrarni_w_d'.
    kIdXvsrl_b,                          //!< Instruction 'xvsrl_b'.
    kIdXvsrl_d,                          //!< Instruction 'xvsrl_d'.
    kIdXvsrl_h,                          //!< Instruction 'xvsrl_h'.
    kIdXvsrl_w,                          //!< Instruction 'xvsrl_w'.
    kIdXvsrli_b,                         //!< Instruction 'xvsrli_b'.
    kIdXvsrli_d,                         //!< Instruction 'xvsrli_d'.
    kIdXvsrli_h,                         //!< Instruction 'xvsrli_h'.
    kIdXvsrli_w,                         //!< Instruction 'xvsrli_w'.
    kIdXvsrln_b_h,                       //!< Instruction 'xvsrln_b_h'.
    kIdXvsrln_h_w,                       //!< Instruction 'xvsrln_h_w'.
    kIdXvsrln_w_d,                       //!< Instruction 'xvsrln_w_d'.
    kIdXvsrlni_b_h,                      //!< Instruction 'xvsrlni_b_h'.
    kIdXvsrlni_d_q,                      //!< Instruction 'xvsrlni_d_q'.
    kIdXvsrlni_h_w,                      //!< Instruction 'xvsrlni_h_w'.
    kIdXvsrlni_w_d,                      //!< Instruction 'xvsrlni_w_d'.
    kIdXvsrlr_b,                         //!< Instruction 'xvsrlr_b'.
    kIdXvsrlr_d,                         //!< Instruction 'xvsrlr_d'.
    kIdXvsrlr_h,                         //!< Instruction 'xvsrlr_h'.
    kIdXvsrlr_w,                         //!< Instruction 'xvsrlr_w'.
    kIdXvsrlri_b,                        //!< Instruction 'xvsrlri_b'.
    kIdXvsrlri_d,                        //!< Instruction 'xvsrlri_d'.
    kIdXvsrlri_h,                        //!< Instruction 'xvsrlri_h'.
    kIdXvsrlri_w,                        //!< Instruction 'xvsrlri_w'.
    kIdXvsrlrn_b_h,                      //!< Instruction 'xvsrlrn_b_h'.
    kIdXvsrlrn_h_w,                      //!< Instruction 'xvsrlrn_h_w'.
    kIdXvsrlrn_w_d,                      //!< Instruction 'xvsrlrn_w_d'.
    kIdXvsrlrni_b_h,                     //!< Instruction 'xvsrlrni_b_h'.
    kIdXvsrlrni_d_q,                     //!< Instruction 'xvsrlrni_d_q'.
    kIdXvsrlrni_h_w,                     //!< Instruction 'xvsrlrni_h_w'.
    kIdXvsrlrni_w_d,                     //!< Instruction 'xvsrlrni_w_d'.
    kIdXvssran_b_h,                      //!< Instruction 'xvssran_b_h'.
    kIdXvssran_bu_h,                     //!< Instruction 'xvssran_bu_h'.
    kIdXvssran_h_w,                      //!< Instruction 'xvssran_h_w'.
    kIdXvssran_hu_w,                     //!< Instruction 'xvssran_hu_w'.
    kIdXvssran_w_d,                      //!< Instruction 'xvssran_w_d'.
    kIdXvssran_wu_d,                     //!< Instruction 'xvssran_wu_d'.
    kIdXvssrani_b_h,                     //!< Instruction 'xvssrani_b_h'.
    kIdXvssrani_bu_h,                    //!< Instruction 'xvssrani_bu_h'.
    kIdXvssrani_d_q,                     //!< Instruction 'xvssrani_d_q'.
    kIdXvssrani_du_q,                    //!< Instruction 'xvssrani_du_q'.
    kIdXvssrani_h_w,                     //!< Instruction 'xvssrani_h_w'.
    kIdXvssrani_hu_w,                    //!< Instruction 'xvssrani_hu_w'.
    kIdXvssrani_w_d,                     //!< Instruction 'xvssrani_w_d'.
    kIdXvssrani_wu_d,                    //!< Instruction 'xvssrani_wu_d'.
    kIdXvssrarn_b_h,                     //!< Instruction 'xvssrarn_b_h'.
    kIdXvssrarn_bu_h,                    //!< Instruction 'xvssrarn_bu_h'.
    kIdXvssrarn_h_w,                     //!< Instruction 'xvssrarn_h_w'.
    kIdXvssrarn_hu_w,                    //!< Instruction 'xvssrarn_hu_w'.
    kIdXvssrarn_w_d,                     //!< Instruction 'xvssrarn_w_d'.
    kIdXvssrarn_wu_d,                    //!< Instruction 'xvssrarn_wu_d'.
    kIdXvssrarni_b_h,                    //!< Instruction 'xvssrarni_b_h'.
    kIdXvssrarni_bu_h,                   //!< Instruction 'xvssrarni_bu_h'.
    kIdXvssrarni_d_q,                    //!< Instruction 'xvssrarni_d_q'.
    kIdXvssrarni_du_q,                   //!< Instruction 'xvssrarni_du_q'.
    kIdXvssrarni_h_w,                    //!< Instruction 'xvssrarni_h_w'.
    kIdXvssrarni_hu_w,                   //!< Instruction 'xvssrarni_hu_w'.
    kIdXvssrarni_w_d,                    //!< Instruction 'xvssrarni_w_d'.
    kIdXvssrarni_wu_d,                   //!< Instruction 'xvssrarni_wu_d'.
    kIdXvssrln_b_h,                      //!< Instruction 'xvssrln_b_h'.
    kIdXvssrln_bu_h,                     //!< Instruction 'xvssrln_bu_h'.
    kIdXvssrln_h_w,                      //!< Instruction 'xvssrln_h_w'.
    kIdXvssrln_hu_w,                     //!< Instruction 'xvssrln_hu_w'.
    kIdXvssrln_w_d,                      //!< Instruction 'xvssrln_w_d'.
    kIdXvssrln_wu_d,                     //!< Instruction 'xvssrln_wu_d'.
    kIdXvssrlni_b_h,                     //!< Instruction 'xvssrlni_b_h'.
    kIdXvssrlni_bu_h,                    //!< Instruction 'xvssrlni_bu_h'.
    kIdXvssrlni_d_q,                     //!< Instruction 'xvssrlni_d_q'.
    kIdXvssrlni_du_q,                    //!< Instruction 'xvssrlni_du_q'.
    kIdXvssrlni_h_w,                     //!< Instruction 'xvssrlni_h_w'.
    kIdXvssrlni_hu_w,                    //!< Instruction 'xvssrlni_hu_w'.
    kIdXvssrlni_w_d,                     //!< Instruction 'xvssrlni_w_d'.
    kIdXvssrlni_wu_d,                    //!< Instruction 'xvssrlni_wu_d'.
    kIdXvssrlrn_b_h,                     //!< Instruction 'xvssrlrn_b_h'.
    kIdXvssrlrn_bu_h,                    //!< Instruction 'xvssrlrn_bu_h'.
    kIdXvssrlrn_h_w,                     //!< Instruction 'xvssrlrn_h_w'.
    kIdXvssrlrn_hu_w,                    //!< Instruction 'xvssrlrn_hu_w'.
    kIdXvssrlrn_w_d,                     //!< Instruction 'xvssrlrn_w_d'.
    kIdXvssrlrn_wu_d,                    //!< Instruction 'xvssrlrn_wu_d'.
    kIdXvssrlrni_b_h,                    //!< Instruction 'xvssrlrni_b_h'.
    kIdXvssrlrni_bu_h,                   //!< Instruction 'xvssrlrni_bu_h'.
    kIdXvssrlrni_d_q,                    //!< Instruction 'xvssrlrni_d_q'.
    kIdXvssrlrni_du_q,                   //!< Instruction 'xvssrlrni_du_q'.
    kIdXvssrlrni_h_w,                    //!< Instruction 'xvssrlrni_h_w'.
    kIdXvssrlrni_hu_w,                   //!< Instruction 'xvssrlrni_hu_w'.
    kIdXvssrlrni_w_d,                    //!< Instruction 'xvssrlrni_w_d'.
    kIdXvssrlrni_wu_d,                   //!< Instruction 'xvssrlrni_wu_d'.
    kIdXvssub_b,                         //!< Instruction 'xvssub_b'.
    kIdXvssub_bu,                        //!< Instruction 'xvssub_bu'.
    kIdXvssub_d,                         //!< Instruction 'xvssub_d'.
    kIdXvssub_du,                        //!< Instruction 'xvssub_du'.
    kIdXvssub_h,                         //!< Instruction 'xvssub_h'.
    kIdXvssub_hu,                        //!< Instruction 'xvssub_hu'.
    kIdXvssub_w,                         //!< Instruction 'xvssub_w'.
    kIdXvssub_wu,                        //!< Instruction 'xvssub_wu'.
    kIdXvst,                             //!< Instruction 'xvst'.
    kIdXvstelm_b,                        //!< Instruction 'xvstelm_b'.
    kIdXvstelm_d,                        //!< Instruction 'xvstelm_d'.
    kIdXvstelm_h,                        //!< Instruction 'xvstelm_h'.
    kIdXvstelm_w,                        //!< Instruction 'xvstelm_w'.
    kIdXvstx,                            //!< Instruction 'xvstx'.
    kIdXvsub_b,                          //!< Instruction 'xvsub_b'.
    kIdXvsub_d,                          //!< Instruction 'xvsub_d'.
    kIdXvsub_h,                          //!< Instruction 'xvsub_h'.
    kIdXvsub_q,                          //!< Instruction 'xvsub_q'.
    kIdXvsub_w,                          //!< Instruction 'xvsub_w'.
    kIdXvsubi_bu,                        //!< Instruction 'xvsubi_bu'.
    kIdXvsubi_du,                        //!< Instruction 'xvsubi_du'.
    kIdXvsubi_hu,                        //!< Instruction 'xvsubi_hu'.
    kIdXvsubi_wu,                        //!< Instruction 'xvsubi_wu'.
    kIdXvsubwev_d_w,                     //!< Instruction 'xvsubwev_d_w'.
    kIdXvsubwev_d_wu,                    //!< Instruction 'xvsubwev_d_wu'.
    kIdXvsubwev_h_b,                     //!< Instruction 'xvsubwev_h_b'.
    kIdXvsubwev_h_bu,                    //!< Instruction 'xvsubwev_h_bu'.
    kIdXvsubwev_q_d,                     //!< Instruction 'xvsubwev_q_d'.
    kIdXvsubwev_q_du,                    //!< Instruction 'xvsubwev_q_du'.
    kIdXvsubwev_w_h,                     //!< Instruction 'xvsubwev_w_h'.
    kIdXvsubwev_w_hu,                    //!< Instruction 'xvsubwev_w_hu'.
    kIdXvsubwod_d_w,                     //!< Instruction 'xvsubwod_d_w'.
    kIdXvsubwod_d_wu,                    //!< Instruction 'xvsubwod_d_wu'.
    kIdXvsubwod_h_b,                     //!< Instruction 'xvsubwod_h_b'.
    kIdXvsubwod_h_bu,                    //!< Instruction 'xvsubwod_h_bu'.
    kIdXvsubwod_q_d,                     //!< Instruction 'xvsubwod_q_d'.
    kIdXvsubwod_q_du,                    //!< Instruction 'xvsubwod_q_du'.
    kIdXvsubwod_w_h,                     //!< Instruction 'xvsubwod_w_h'.
    kIdXvsubwod_w_hu,                    //!< Instruction 'xvsubwod_w_hu'.
    kIdXvxor_v,                          //!< Instruction 'xvxor' {ASIMD}.
    kIdXvxori_b,                         //!< Instruction 'xvxori_b'.
    kIdVfcmp_caf_s,                      //!< Instruction 'vfcmp_caf_s'.
    kIdVfcmp_cun_s,                      //!< Instruction 'vfcmp_cun_s'.
    kIdVfcmp_ceq_s,                      //!< Instruction 'vfcmp_ceq_s'.
    kIdVfcmp_cueq_s,                     //!< Instruction 'vfcmp_cueq_s'.
    kIdVfcmp_clt_s,                      //!< Instruction 'vfcmp_clt_s'.
    kIdVfcmp_cult_s,                     //!< Instruction 'vfcmp_cult_s'.
    kIdVfcmp_cle_s,                      //!< Instruction 'vfcmp_cle_s'.
    kIdVfcmp_cule_s,                     //!< Instruction 'vfcmp_cule_s'.
    kIdVfcmp_cne_s,                      //!< Instruction 'vfcmp_cne_s'.
    kIdVfcmp_cor_s,                      //!< Instruction 'vfcmp_cor_s'.
    kIdVfcmp_cune_s,                     //!< Instruction 'vfcmp_cune_s'.
    kIdVfcmp_saf_s,                      //!< Instruction 'vfcmp_saf_s'.
    kIdVfcmp_sun_s,                      //!< Instruction 'vfcmp_sun_s'.
    kIdVfcmp_seq_s,                      //!< Instruction 'vfcmp_seq_s'.
    kIdVfcmp_sueq_s,                     //!< Instruction 'vfcmp_sueq_s'.
    kIdVfcmp_slt_s,                      //!< Instruction 'vfcmp_slt_s'.
    kIdVfcmp_sult_s,                     //!< Instruction 'vfcmp_sult_s'.
    kIdVfcmp_sle_s,                      //!< Instruction 'vfcmp_sle_s'.
    kIdVfcmp_sule_s,                     //!< Instruction 'vfcmp_sule_s'.
    kIdVfcmp_sne_s,                      //!< Instruction 'vfcmp_sne_s'.
    kIdVfcmp_sor_s,                      //!< Instruction 'vfcmp_sor_s'.
    kIdVfcmp_sune_s,                     //!< Instruction 'vfcmp_sune_s'.
    kIdVfcmp_caf_d,                      //!< Instruction 'vfcmp_caf_d'.
    kIdVfcmp_cun_d,                      //!< Instruction 'vfcmp_cun_d'.
    kIdVfcmp_ceq_d,                      //!< Instruction 'vfcmp_ceq_d'.
    kIdVfcmp_cueq_d,                     //!< Instruction 'vfcmp_cueq_d'.
    kIdVfcmp_clt_d,                      //!< Instruction 'vfcmp_clt_d'.
    kIdVfcmp_cult_d,                     //!< Instruction 'vfcmp_cult_d'.
    kIdVfcmp_cle_d,                      //!< Instruction 'vfcmp_cle_d'.
    kIdVfcmp_cule_d,                     //!< Instruction 'vfcmp_cule_d'.
    kIdVfcmp_cne_d,                      //!< Instruction 'vfcmp_cne_d'.
    kIdVfcmp_cor_d,                      //!< Instruction 'vfcmp_cor_d'.
    kIdVfcmp_cune_d,                     //!< Instruction 'vfcmp_cune_d'.
    kIdVfcmp_saf_d,                      //!< Instruction 'vfcmp_saf_d'.
    kIdVfcmp_sun_d,                      //!< Instruction 'vfcmp_sun_d'.
    kIdVfcmp_seq_d,                      //!< Instruction 'vfcmp_seq_d'.
    kIdVfcmp_sueq_d,                     //!< Instruction 'vfcmp_sueq_d'.
    kIdVfcmp_slt_d,                      //!< Instruction 'vfcmp_slt_d'.
    kIdVfcmp_sult_d,                     //!< Instruction 'vfcmp_sult_d'.
    kIdVfcmp_sle_d,                      //!< Instruction 'vfcmp_sle_d'.
    kIdVfcmp_sule_d,                     //!< Instruction 'vfcmp_sule_d'.
    kIdVfcmp_sne_d,                      //!< Instruction 'vfcmp_sne_d'.
    kIdVfcmp_sor_d,                      //!< Instruction 'vfcmp_sor_d'.
    kIdVfcmp_sune_d,                     //!< Instruction 'vfcmp_sune_d'.
    kIdXvfcmp_caf_s,                     //!< Instruction 'xvfcmp_caf_s'.
    kIdXvfcmp_cun_s,                     //!< Instruction 'xvfcmp_cun_s'.
    kIdXvfcmp_ceq_s,                     //!< Instruction 'xvfcmp_ceq_s'.
    kIdXvfcmp_cueq_s,                    //!< Instruction 'xvfcmp_cueq_s'.
    kIdXvfcmp_clt_s,                     //!< Instruction 'xvfcmp_clt_s'.
    kIdXvfcmp_cult_s,                    //!< Instruction 'xvfcmp_cult_s'.
    kIdXvfcmp_cle_s,                     //!< Instruction 'xvfcmp_cle_s'.
    kIdXvfcmp_cule_s,                    //!< Instruction 'xvfcmp_cule_s'.
    kIdXvfcmp_cne_s,                     //!< Instruction 'xvfcmp_cne_s'.
    kIdXvfcmp_cor_s,                     //!< Instruction 'xvfcmp_cor_s'.
    kIdXvfcmp_cune_s,                    //!< Instruction 'xvfcmp_cune_s'.
    kIdXvfcmp_saf_s,                     //!< Instruction 'xvfcmp_saf_s'.
    kIdXvfcmp_sun_s,                     //!< Instruction 'xvfcmp_sun_s'.
    kIdXvfcmp_seq_s,                     //!< Instruction 'xvfcmp_seq_s'.
    kIdXvfcmp_sueq_s,                    //!< Instruction 'xvfcmp_sueq_s'.
    kIdXvfcmp_slt_s,                     //!< Instruction 'xvfcmp_slt_s'.
    kIdXvfcmp_sult_s,                    //!< Instruction 'xvfcmp_sult_s'.
    kIdXvfcmp_sle_s,                     //!< Instruction 'xvfcmp_sle_s'.
    kIdXvfcmp_sule_s,                    //!< Instruction 'xvfcmp_sule_s'.
    kIdXvfcmp_sne_s,                     //!< Instruction 'xvfcmp_sne_s'.
    kIdXvfcmp_sor_s,                     //!< Instruction 'xvfcmp_sor_s'.
    kIdXvfcmp_sune_s,                    //!< Instruction 'xvfcmp_sune_s'.
    kIdXvfcmp_caf_d,                     //!< Instruction 'xvfcmp_caf_d'.
    kIdXvfcmp_cun_d,                     //!< Instruction 'xvfcmp_cun_d'.
    kIdXvfcmp_ceq_d,                     //!< Instruction 'xvfcmp_ceq_d'.
    kIdXvfcmp_cueq_d,                    //!< Instruction 'xvfcmp_cueq_d'.
    kIdXvfcmp_clt_d,                     //!< Instruction 'xvfcmp_clt_d'.
    kIdXvfcmp_cult_d,                    //!< Instruction 'xvfcmp_cult_d'.
    kIdXvfcmp_cle_d,                     //!< Instruction 'xvfcmp_cle_d'.
    kIdXvfcmp_cule_d,                    //!< Instruction 'xvfcmp_cule_d'.
    kIdXvfcmp_cne_d,                     //!< Instruction 'xvfcmp_cne_d'.
    kIdXvfcmp_cor_d,                     //!< Instruction 'xvfcmp_cor_d'.
    kIdXvfcmp_cune_d,                    //!< Instruction 'xvfcmp_cune_d'.
    kIdXvfcmp_saf_d,                     //!< Instruction 'xvfcmp_saf_d'.
    kIdXvfcmp_sun_d,                     //!< Instruction 'xvfcmp_sun_d'.
    kIdXvfcmp_seq_d,                     //!< Instruction 'xvfcmp_seq_d'.
    kIdXvfcmp_sueq_d,                    //!< Instruction 'xvfcmp_sueq_d'.
    kIdXvfcmp_slt_d,                     //!< Instruction 'xvfcmp_slt_d'.
    kIdXvfcmp_sult_d,                    //!< Instruction 'xvfcmp_sult_d'.
    kIdXvfcmp_sle_d,                     //!< Instruction 'xvfcmp_sle_d'.
    kIdXvfcmp_sule_d,                    //!< Instruction 'xvfcmp_sule_d'.
    kIdXvfcmp_sne_d,                     //!< Instruction 'xvfcmp_sne_d'.
    kIdXvfcmp_sor_d,                     //!< Instruction 'xvfcmp_sor_d'.
    kIdXvfcmp_sune_d,                    //!< Instruction 'xvfcmp_sune_d'.
    _kIdCount
    // ${InstId:End}
  };

  //! Tests whether the `instId` is defined (counts also Inst::kIdNone, which must be zero).
  static ASMJIT_INLINE_NODEBUG bool isDefinedId(InstId instId) noexcept { return (instId & uint32_t(InstIdParts::kRealId)) < _kIdCount; }
};

namespace Predicate {

//! Address translate options (AT).
namespace AT {
  static ASMJIT_INLINE_NODEBUG constexpr uint32_t encode(uint32_t op1, uint32_t cRn, uint32_t cRm, uint32_t op2) noexcept {
    return (op1 << 11) | (cRn << 7) | (cRm << 3) | (op2 << 0);
  }

  enum Value : uint32_t {
    kS1E1R  = encode(0b000, 0b0111, 0b1000, 0b000),
    kS1E2R  = encode(0b100, 0b0111, 0b1000, 0b000),
    kS1E3R  = encode(0b110, 0b0111, 0b1000, 0b000),
    kS1E1W  = encode(0b000, 0b0111, 0b1000, 0b001),
    kS1E2W  = encode(0b100, 0b0111, 0b1000, 0b001),
    kS1E3W  = encode(0b110, 0b0111, 0b1000, 0b001),
    kS1E0R  = encode(0b000, 0b0111, 0b1000, 0b010),
    kS1E0W  = encode(0b000, 0b0111, 0b1000, 0b011),
    kS12E1R = encode(0b100, 0b0111, 0b1000, 0b100),
    kS12E1W = encode(0b100, 0b0111, 0b1000, 0b101),
    kS12E0R = encode(0b100, 0b0111, 0b1000, 0b110),
    kS12E0W = encode(0b100, 0b0111, 0b1000, 0b111),
    kS1E1RP = encode(0b000, 0b0111, 0b1001, 0b000),
    kS1E1WP = encode(0b000, 0b0111, 0b1001, 0b001)
  };
}

//! Data barrier options (DMB/DSB).
namespace DB {
  //! Data barrier immediate values.
  enum Value : uint32_t {
    //! Waits only for loads to complete, and only applies to the outer shareable domain.
    kOSHLD = 0x01u,
    //! Waits only for stores to complete, and only applies to the outer shareable domain.
    kOSHST = 0x02u,
    //! Only applies to the outer shareable domain.
    kOSH = 0x03u,

    //! Waits only for loads to complete and only applies out to the point of unification.
    kNSHLD = 0x05u,
    //! Waits only for stores to complete and only applies out to the point of unification.
    kNSHST = 0x06u,
    //! Only applies out to the point of unification.
    kNSH = 0x07u,

    //! Waits only for loads to complete, and only applies to the inner shareable domain.
    kISHLD = 0x09u,
    //! Waits only for stores to complete, and only applies to the inner shareable domain.
    kISHST = 0x0Au,
    //! Only applies to the inner shareable domain.
    kISH = 0x0Bu,

    //! Waits only for loads to complete.
    kLD = 0x0Du,
    //! Waits only for stores to complete.
    kST = 0x0Eu,
    //! Full system memory barrier operation.
    kSY = 0x0Fu
  };
}

//! Data cache maintenance options.
namespace DC {
  static ASMJIT_INLINE_NODEBUG constexpr uint32_t encode(uint32_t op1, uint32_t cRn, uint32_t cRm, uint32_t op2) noexcept {
    return (op1 << 11) | (cRn << 7) | (cRm << 3) | (op2 << 0);
  }

  //! Data cache maintenance immediate values.
  enum Value : uint32_t {
    kZVA     = encode(0b011, 0b0111, 0b0100, 0b001),
    kIVAC    = encode(0b000, 0b0111, 0b0110, 0b001),
    kISW     = encode(0b000, 0b0111, 0b0110, 0b010),
    kCVAC    = encode(0b011, 0b0111, 0b1010, 0b001),
    kCSW     = encode(0b000, 0b0111, 0b1010, 0b010),
    kCVAU    = encode(0b011, 0b0111, 0b1011, 0b001),
    kCIVAC   = encode(0b011, 0b0111, 0b1110, 0b001),
    kCISW    = encode(0b000, 0b0111, 0b1110, 0b010),
    kCVAP    = encode(0b011, 0b0111, 0b1100, 0b001),
    kCVADP   = encode(0b011, 0b0111, 0b1101, 0b001),
    kIGVAC   = encode(0b000, 0b0111, 0b0110, 0b011),
    kIGSW    = encode(0b000, 0b0111, 0b0110, 0b100),
    kCGSW    = encode(0b000, 0b0111, 0b1010, 0b100),
    kCIGSW   = encode(0b000, 0b0111, 0b1110, 0b100),
    kCGVAC   = encode(0b011, 0b0111, 0b1010, 0b011),
    kCGVAP   = encode(0b011, 0b0111, 0b1100, 0b011),
    kCGVADP  = encode(0b011, 0b0111, 0b1101, 0b011),
    kCIGVAC  = encode(0b011, 0b0111, 0b1110, 0b011),
    kGVA     = encode(0b011, 0b0111, 0b0100, 0b011),
    kIGDVAC  = encode(0b000, 0b0111, 0b0110, 0b101),
    kIGDSW   = encode(0b000, 0b0111, 0b0110, 0b110),
    kCGDSW   = encode(0b000, 0b0111, 0b1010, 0b110),
    kCIGDSW  = encode(0b000, 0b0111, 0b1110, 0b110),
    kCGDVAC  = encode(0b011, 0b0111, 0b1010, 0b101),
    kCGDVAP  = encode(0b011, 0b0111, 0b1100, 0b101),
    kCGDVADP = encode(0b011, 0b0111, 0b1101, 0b101),
    kCIGDVAC = encode(0b011, 0b0111, 0b1110, 0b101),
    kGZVA    = encode(0b011, 0b0111, 0b0100, 0b100)
  };
}

//! Instruction cache maintenance options.
namespace IC {
  static ASMJIT_INLINE_NODEBUG constexpr uint32_t encode(uint32_t op1, uint32_t cRn, uint32_t cRm, uint32_t op2) noexcept {
    return (op1 << 11) | (cRn << 7) | (cRm << 3) | (op2 << 0);
  }

  //! Instruction cache maintenance immediate values.
  enum Value : uint32_t {
    kIALLUIS = encode(0b000, 0b0111, 0b0001, 0b000),
    kIALLU   = encode(0b000, 0b0111, 0b0101, 0b000),
    kIVAU    = encode(0b011, 0b0111, 0b0101, 0b001)
  };
}

//! Instruction-fetch barrier options.
namespace ISB {
  //! Instruction-fetch barrier immediate values.
  enum Value : uint32_t {
    kSY = 0xF
  };
}

//! Prefetch options.
namespace PRFOp {
  //! Prefetch immediate values.
  enum Value : uint32_t {
    kPLDL1KEEP = 0x00,
    kPLDL1STRM = 0x01,
    kPLDL2KEEP = 0x02,
    kPLDL2STRM = 0x03,
    kPLDL3KEEP = 0x04,
    kPLDL3STRM = 0x05,
    kPLIL1KEEP = 0x08,
    kPLIL1STRM = 0x09,
    kPLIL2KEEP = 0x0A,
    kPLIL2STRM = 0x0B,
    kPLIL3KEEP = 0x0C,
    kPLIL3STRM = 0x0D,
    kPSTL1KEEP = 0x10,
    kPSTL1STRM = 0x11,
    kPSTL2KEEP = 0x12,
    kPSTL2STRM = 0x13,
    kPSTL3KEEP = 0x14,
    kPSTL3STRM = 0x15
  };
}

//! PSB instruction options.
namespace PSB {
  //! PSB immediate values.
  enum Value : uint32_t {
    kCSYNC = 0x11u
  };
}

namespace TLBI {
  static ASMJIT_INLINE_NODEBUG constexpr uint32_t encode(uint32_t op1, uint32_t cRn, uint32_t cRm, uint32_t op2) noexcept {
    return (op1 << 11) | (cRn << 7) | (cRm << 3) | (op2 << 0);
  }

  enum Value : uint32_t {
    kIPAS2E1IS    = encode(0b100, 0b1000, 0b0000, 0b001),
    kIPAS2LE1IS   = encode(0b100, 0b1000, 0b0000, 0b101),
    kVMALLE1IS    = encode(0b000, 0b1000, 0b0011, 0b000),
    kALLE2IS      = encode(0b100, 0b1000, 0b0011, 0b000),
    kALLE3IS      = encode(0b110, 0b1000, 0b0011, 0b000),
    kVAE1IS       = encode(0b000, 0b1000, 0b0011, 0b001),
    kVAE2IS       = encode(0b100, 0b1000, 0b0011, 0b001),
    kVAE3IS       = encode(0b110, 0b1000, 0b0011, 0b001),
    kASIDE1IS     = encode(0b000, 0b1000, 0b0011, 0b010),
    kVAAE1IS      = encode(0b000, 0b1000, 0b0011, 0b011),
    kALLE1IS      = encode(0b100, 0b1000, 0b0011, 0b100),
    kVALE1IS      = encode(0b000, 0b1000, 0b0011, 0b101),
    kVALE2IS      = encode(0b100, 0b1000, 0b0011, 0b101),
    kVALE3IS      = encode(0b110, 0b1000, 0b0011, 0b101),
    kVMALLS12E1IS = encode(0b100, 0b1000, 0b0011, 0b110),
    kVAALE1IS     = encode(0b000, 0b1000, 0b0011, 0b111),
    kIPAS2E1      = encode(0b100, 0b1000, 0b0100, 0b001),
    kIPAS2LE1     = encode(0b100, 0b1000, 0b0100, 0b101),
    kVMALLE1      = encode(0b000, 0b1000, 0b0111, 0b000),
    kALLE2        = encode(0b100, 0b1000, 0b0111, 0b000),
    kALLE3        = encode(0b110, 0b1000, 0b0111, 0b000),
    kVAE1         = encode(0b000, 0b1000, 0b0111, 0b001),
    kVAE2         = encode(0b100, 0b1000, 0b0111, 0b001),
    kVAE3         = encode(0b110, 0b1000, 0b0111, 0b001),
    kASIDE1       = encode(0b000, 0b1000, 0b0111, 0b010),
    kVAAE1        = encode(0b000, 0b1000, 0b0111, 0b011),
    kALLE1        = encode(0b100, 0b1000, 0b0111, 0b100),
    kVALE1        = encode(0b000, 0b1000, 0b0111, 0b101),
    kVALE2        = encode(0b100, 0b1000, 0b0111, 0b101),
    kVALE3        = encode(0b110, 0b1000, 0b0111, 0b101),
    kVMALLS12E1   = encode(0b100, 0b1000, 0b0111, 0b110),
    kVAALE1       = encode(0b000, 0b1000, 0b0111, 0b111),

    kVMALLE1OS    = encode(0b000, 0b1000, 0b0001, 0b000),
    kVAE1OS       = encode(0b000, 0b1000, 0b0001, 0b001),
    kASIDE1OS     = encode(0b000, 0b1000, 0b0001, 0b010),
    kVAAE1OS      = encode(0b000, 0b1000, 0b0001, 0b011),
    kVALE1OS      = encode(0b000, 0b1000, 0b0001, 0b101),
    kVAALE1OS     = encode(0b000, 0b1000, 0b0001, 0b111),
    kIPAS2E1OS    = encode(0b100, 0b1000, 0b0100, 0b000),
    kIPAS2LE1OS   = encode(0b100, 0b1000, 0b0100, 0b100),
    kVAE2OS       = encode(0b100, 0b1000, 0b0001, 0b001),
    kVALE2OS      = encode(0b100, 0b1000, 0b0001, 0b101),
    kVMALLS12E1OS = encode(0b100, 0b1000, 0b0001, 0b110),
    kVAE3OS       = encode(0b110, 0b1000, 0b0001, 0b001),
    kVALE3OS      = encode(0b110, 0b1000, 0b0001, 0b101),
    kALLE2OS      = encode(0b100, 0b1000, 0b0001, 0b000),
    kALLE1OS      = encode(0b100, 0b1000, 0b0001, 0b100),
    kALLE3OS      = encode(0b110, 0b1000, 0b0001, 0b000),

    kRVAE1        = encode(0b000, 0b1000, 0b0110, 0b001),
    kRVAAE1       = encode(0b000, 0b1000, 0b0110, 0b011),
    kRVALE1       = encode(0b000, 0b1000, 0b0110, 0b101),
    kRVAALE1      = encode(0b000, 0b1000, 0b0110, 0b111),
    kRVAE1IS      = encode(0b000, 0b1000, 0b0010, 0b001),
    kRVAAE1IS     = encode(0b000, 0b1000, 0b0010, 0b011),
    kRVALE1IS     = encode(0b000, 0b1000, 0b0010, 0b101),
    kRVAALE1IS    = encode(0b000, 0b1000, 0b0010, 0b111),
    kRVAE1OS      = encode(0b000, 0b1000, 0b0101, 0b001),
    kRVAAE1OS     = encode(0b000, 0b1000, 0b0101, 0b011),
    kRVALE1OS     = encode(0b000, 0b1000, 0b0101, 0b101),
    kRVAALE1OS    = encode(0b000, 0b1000, 0b0101, 0b111),
    kRIPAS2E1IS   = encode(0b100, 0b1000, 0b0000, 0b010),
    kRIPAS2LE1IS  = encode(0b100, 0b1000, 0b0000, 0b110),
    kRIPAS2E1     = encode(0b100, 0b1000, 0b0100, 0b010),
    kRIPAS2LE1    = encode(0b100, 0b1000, 0b0100, 0b110),
    kRIPAS2E1OS   = encode(0b100, 0b1000, 0b0100, 0b011),
    kRIPAS2LE1OS  = encode(0b100, 0b1000, 0b0100, 0b111),
    kRVAE2        = encode(0b100, 0b1000, 0b0110, 0b001),
    kRVALE2       = encode(0b100, 0b1000, 0b0110, 0b101),
    kRVAE2IS      = encode(0b100, 0b1000, 0b0010, 0b001),
    kRVALE2IS     = encode(0b100, 0b1000, 0b0010, 0b101),
    kRVAE2OS      = encode(0b100, 0b1000, 0b0101, 0b001),
    kRVALE2OS     = encode(0b100, 0b1000, 0b0101, 0b101),
    kRVAE3        = encode(0b110, 0b1000, 0b0110, 0b001),
    kRVALE3       = encode(0b110, 0b1000, 0b0110, 0b101),
    kRVAE3IS      = encode(0b110, 0b1000, 0b0010, 0b001),
    kRVALE3IS     = encode(0b110, 0b1000, 0b0010, 0b101),
    kRVAE3OS      = encode(0b110, 0b1000, 0b0101, 0b001),
    kRVALE3OS     = encode(0b110, 0b1000, 0b0101, 0b101),
  };
}

//! Trace synchronization barrier options.
namespace TSB {
  //! Trace synchronization immediate values.
  enum Value : uint32_t {
    kCSYNC = 0
  };
}

//! Processor state access through MSR.
namespace PState {
  //! Encodes a pstate from `op0` and `op1`.
  static ASMJIT_INLINE_NODEBUG constexpr uint32_t encode(uint32_t op0, uint32_t op1) noexcept {
    return (op0 << 3) | (op1 << 0);
  }

  //! Processor state access immediates.
  enum Value : uint32_t {
    kSPSel   = encode(0b000, 0b101),
    kDAIFSet = encode(0b011, 0b110),
    kDAIFClr = encode(0b011, 0b111),
    kPAN     = encode(0b000, 0b100),
    kUAO     = encode(0b000, 0b011),
    kDIT     = encode(0b011, 0b010),
    kSSBS    = encode(0b011, 0b001),
    kTCO     = encode(0b011, 0b100)
  };
};

//! System register identifiers and utilities (MSR/MRS).
namespace SysReg {
  //! System register fields.
  struct Fields {
    uint8_t op0;
    uint8_t op1;
    uint8_t cRn;
    uint8_t cRm;
    uint8_t op2;
  };

  //! Encodes a system register from `op0`, `op1`, `cRn`, `cRm`, and `op2` fields.
  static ASMJIT_INLINE_NODEBUG constexpr uint32_t encode(uint32_t op0, uint32_t op1, uint32_t cRn, uint32_t cRm, uint32_t op2) noexcept {
    return (op0 << 14) | (op1 << 11) | (cRn << 7) | (cRm << 3) | (op2 << 0);
  }

  //! Encodes a system register from `fields`.
  static ASMJIT_INLINE_NODEBUG constexpr uint32_t encode(const Fields& fields) noexcept {
    return encode(fields.op0, fields.op1, fields.cRn, fields.cRm, fields.op2);
  }

  //! Decodes a system register to \ref Fields.
  static ASMJIT_INLINE_NODEBUG constexpr Fields decode(uint32_t id) noexcept {
    return Fields {
      uint8_t((id >> 14) & 0x3u),
      uint8_t((id >> 11) & 0x7u),
      uint8_t((id >>  7) & 0xFu),
      uint8_t((id >>  3) & 0xFu),
      uint8_t((id >>  0) & 0x7u)
    };
  }

  //! System register identifiers.
  enum Id : uint32_t {
    kACTLR_EL1            = encode(0b11, 0b000, 0b0001, 0b0000, 0b001), // RW
    kACTLR_EL2            = encode(0b11, 0b100, 0b0001, 0b0000, 0b001), // RW
    kACTLR_EL3            = encode(0b11, 0b110, 0b0001, 0b0000, 0b001), // RW
    kAFSR0_EL1            = encode(0b11, 0b000, 0b0101, 0b0001, 0b000), // RW
    kAFSR0_EL12           = encode(0b11, 0b101, 0b0101, 0b0001, 0b000), // RW
    kAFSR0_EL2            = encode(0b11, 0b100, 0b0101, 0b0001, 0b000), // RW
    kAFSR0_EL3            = encode(0b11, 0b110, 0b0101, 0b0001, 0b000), // RW
    kAFSR1_EL1            = encode(0b11, 0b000, 0b0101, 0b0001, 0b001), // RW
    kAFSR1_EL12           = encode(0b11, 0b101, 0b0101, 0b0001, 0b001), // RW
    kAFSR1_EL2            = encode(0b11, 0b100, 0b0101, 0b0001, 0b001), // RW
    kAFSR1_EL3            = encode(0b11, 0b110, 0b0101, 0b0001, 0b001), // RW
    kAIDR_EL1             = encode(0b11, 0b001, 0b0000, 0b0000, 0b111), // RO
    kAMAIR_EL1            = encode(0b11, 0b000, 0b1010, 0b0011, 0b000), // RW
    kAMAIR_EL12           = encode(0b11, 0b101, 0b1010, 0b0011, 0b000), // RW
    kAMAIR_EL2            = encode(0b11, 0b100, 0b1010, 0b0011, 0b000), // RW
    kAMAIR_EL3            = encode(0b11, 0b110, 0b1010, 0b0011, 0b000), // RW
    kAMCFGR_EL0           = encode(0b11, 0b011, 0b1101, 0b0010, 0b001), // RO
    kAMCGCR_EL0           = encode(0b11, 0b011, 0b1101, 0b0010, 0b010), // RO
    kAMCNTENCLR0_EL0      = encode(0b11, 0b011, 0b1101, 0b0010, 0b100), // RW
    kAMCNTENCLR1_EL0      = encode(0b11, 0b011, 0b1101, 0b0011, 0b000), // RW
    kAMCNTENSET0_EL0      = encode(0b11, 0b011, 0b1101, 0b0010, 0b101), // RW
    kAMCNTENSET1_EL0      = encode(0b11, 0b011, 0b1101, 0b0011, 0b001), // RW
    kAMCR_EL0             = encode(0b11, 0b011, 0b1101, 0b0010, 0b000), // RW
    kAMEVCNTR00_EL0       = encode(0b11, 0b011, 0b1101, 0b0100, 0b000), // RW
    kAMEVCNTR01_EL0       = encode(0b11, 0b011, 0b1101, 0b0100, 0b001), // RW
    kAMEVCNTR02_EL0       = encode(0b11, 0b011, 0b1101, 0b0100, 0b010), // RW
    kAMEVCNTR03_EL0       = encode(0b11, 0b011, 0b1101, 0b0100, 0b011), // RW
    kAMEVCNTR10_EL0       = encode(0b11, 0b011, 0b1101, 0b1100, 0b000), // RW
    kAMEVCNTR110_EL0      = encode(0b11, 0b011, 0b1101, 0b1101, 0b010), // RW
    kAMEVCNTR111_EL0      = encode(0b11, 0b011, 0b1101, 0b1101, 0b011), // RW
    kAMEVCNTR112_EL0      = encode(0b11, 0b011, 0b1101, 0b1101, 0b100), // RW
    kAMEVCNTR113_EL0      = encode(0b11, 0b011, 0b1101, 0b1101, 0b101), // RW
    kAMEVCNTR114_EL0      = encode(0b11, 0b011, 0b1101, 0b1101, 0b110), // RW
    kAMEVCNTR115_EL0      = encode(0b11, 0b011, 0b1101, 0b1101, 0b111), // RW
    kAMEVCNTR11_EL0       = encode(0b11, 0b011, 0b1101, 0b1100, 0b001), // RW
    kAMEVCNTR12_EL0       = encode(0b11, 0b011, 0b1101, 0b1100, 0b010), // RW
    kAMEVCNTR13_EL0       = encode(0b11, 0b011, 0b1101, 0b1100, 0b011), // RW
    kAMEVCNTR14_EL0       = encode(0b11, 0b011, 0b1101, 0b1100, 0b100), // RW
    kAMEVCNTR15_EL0       = encode(0b11, 0b011, 0b1101, 0b1100, 0b101), // RW
    kAMEVCNTR16_EL0       = encode(0b11, 0b011, 0b1101, 0b1100, 0b110), // RW
    kAMEVCNTR17_EL0       = encode(0b11, 0b011, 0b1101, 0b1100, 0b111), // RW
    kAMEVCNTR18_EL0       = encode(0b11, 0b011, 0b1101, 0b1101, 0b000), // RW
    kAMEVCNTR19_EL0       = encode(0b11, 0b011, 0b1101, 0b1101, 0b001), // RW
    kAMEVTYPER00_EL0      = encode(0b11, 0b011, 0b1101, 0b0110, 0b000), // RO
    kAMEVTYPER01_EL0      = encode(0b11, 0b011, 0b1101, 0b0110, 0b001), // RO
    kAMEVTYPER02_EL0      = encode(0b11, 0b011, 0b1101, 0b0110, 0b010), // RO
    kAMEVTYPER03_EL0      = encode(0b11, 0b011, 0b1101, 0b0110, 0b011), // RO
    kAMEVTYPER10_EL0      = encode(0b11, 0b011, 0b1101, 0b1110, 0b000), // RW
    kAMEVTYPER110_EL0     = encode(0b11, 0b011, 0b1101, 0b1111, 0b010), // RW
    kAMEVTYPER111_EL0     = encode(0b11, 0b011, 0b1101, 0b1111, 0b011), // RW
    kAMEVTYPER112_EL0     = encode(0b11, 0b011, 0b1101, 0b1111, 0b100), // RW
    kAMEVTYPER113_EL0     = encode(0b11, 0b011, 0b1101, 0b1111, 0b101), // RW
    kAMEVTYPER114_EL0     = encode(0b11, 0b011, 0b1101, 0b1111, 0b110), // RW
    kAMEVTYPER115_EL0     = encode(0b11, 0b011, 0b1101, 0b1111, 0b111), // RW
    kAMEVTYPER11_EL0      = encode(0b11, 0b011, 0b1101, 0b1110, 0b001), // RW
    kAMEVTYPER12_EL0      = encode(0b11, 0b011, 0b1101, 0b1110, 0b010), // RW
    kAMEVTYPER13_EL0      = encode(0b11, 0b011, 0b1101, 0b1110, 0b011), // RW
    kAMEVTYPER14_EL0      = encode(0b11, 0b011, 0b1101, 0b1110, 0b100), // RW
    kAMEVTYPER15_EL0      = encode(0b11, 0b011, 0b1101, 0b1110, 0b101), // RW
    kAMEVTYPER16_EL0      = encode(0b11, 0b011, 0b1101, 0b1110, 0b110), // RW
    kAMEVTYPER17_EL0      = encode(0b11, 0b011, 0b1101, 0b1110, 0b111), // RW
    kAMEVTYPER18_EL0      = encode(0b11, 0b011, 0b1101, 0b1111, 0b000), // RW
    kAMEVTYPER19_EL0      = encode(0b11, 0b011, 0b1101, 0b1111, 0b001), // RW
    kAMUSERENR_EL0        = encode(0b11, 0b011, 0b1101, 0b0010, 0b011), // RW
    kAPDAKeyHi_EL1        = encode(0b11, 0b000, 0b0010, 0b0010, 0b001), // RW
    kAPDAKeyLo_EL1        = encode(0b11, 0b000, 0b0010, 0b0010, 0b000), // RW
    kAPDBKeyHi_EL1        = encode(0b11, 0b000, 0b0010, 0b0010, 0b011), // RW
    kAPDBKeyLo_EL1        = encode(0b11, 0b000, 0b0010, 0b0010, 0b010), // RW
    kAPGAKeyHi_EL1        = encode(0b11, 0b000, 0b0010, 0b0011, 0b001), // RW
    kAPGAKeyLo_EL1        = encode(0b11, 0b000, 0b0010, 0b0011, 0b000), // RW
    kAPIAKeyHi_EL1        = encode(0b11, 0b000, 0b0010, 0b0001, 0b001), // RW
    kAPIAKeyLo_EL1        = encode(0b11, 0b000, 0b0010, 0b0001, 0b000), // RW
    kAPIBKeyHi_EL1        = encode(0b11, 0b000, 0b0010, 0b0001, 0b011), // RW
    kAPIBKeyLo_EL1        = encode(0b11, 0b000, 0b0010, 0b0001, 0b010), // RW
    kCCSIDR2_EL1          = encode(0b11, 0b001, 0b0000, 0b0000, 0b010), // RO
    kCCSIDR_EL1           = encode(0b11, 0b001, 0b0000, 0b0000, 0b000), // RO
    kCLIDR_EL1            = encode(0b11, 0b001, 0b0000, 0b0000, 0b001), // RO
    kCNTFRQ_EL0           = encode(0b11, 0b011, 0b1110, 0b0000, 0b000), // RW
    kCNTHCTL_EL2          = encode(0b11, 0b100, 0b1110, 0b0001, 0b000), // RW
    kCNTHPS_CTL_EL2       = encode(0b11, 0b100, 0b1110, 0b0101, 0b001), // RW
    kCNTHPS_CVAL_EL2      = encode(0b11, 0b100, 0b1110, 0b0101, 0b010), // RW
    kCNTHPS_TVAL_EL2      = encode(0b11, 0b100, 0b1110, 0b0101, 0b000), // RW
    kCNTHP_CTL_EL2        = encode(0b11, 0b100, 0b1110, 0b0010, 0b001), // RW
    kCNTHP_CVAL_EL2       = encode(0b11, 0b100, 0b1110, 0b0010, 0b010), // RW
    kCNTHP_TVAL_EL2       = encode(0b11, 0b100, 0b1110, 0b0010, 0b000), // RW
    kCNTHVS_CTL_EL2       = encode(0b11, 0b100, 0b1110, 0b0100, 0b001), // RW
    kCNTHVS_CVAL_EL2      = encode(0b11, 0b100, 0b1110, 0b0100, 0b010), // RW
    kCNTHVS_TVAL_EL2      = encode(0b11, 0b100, 0b1110, 0b0100, 0b000), // RW
    kCNTHV_CTL_EL2        = encode(0b11, 0b100, 0b1110, 0b0011, 0b001), // RW
    kCNTHV_CVAL_EL2       = encode(0b11, 0b100, 0b1110, 0b0011, 0b010), // RW
    kCNTHV_TVAL_EL2       = encode(0b11, 0b100, 0b1110, 0b0011, 0b000), // RW
    kCNTISCALE_EL2        = encode(0b11, 0b100, 0b1110, 0b0000, 0b101), // RW
    kCNTKCTL_EL1          = encode(0b11, 0b000, 0b1110, 0b0001, 0b000), // RW
    kCNTKCTL_EL12         = encode(0b11, 0b101, 0b1110, 0b0001, 0b000), // RW
    kCNTPCTSS_EL0         = encode(0b11, 0b011, 0b1110, 0b0000, 0b101), // RW
    kCNTPCT_EL0           = encode(0b11, 0b011, 0b1110, 0b0000, 0b001), // RO
    kCNTPOFF_EL2          = encode(0b11, 0b100, 0b1110, 0b0000, 0b110), // RW
    kCNTPS_CTL_EL1        = encode(0b11, 0b111, 0b1110, 0b0010, 0b001), // RW
    kCNTPS_CVAL_EL1       = encode(0b11, 0b111, 0b1110, 0b0010, 0b010), // RW
    kCNTPS_TVAL_EL1       = encode(0b11, 0b111, 0b1110, 0b0010, 0b000), // RW
    kCNTP_CTL_EL0         = encode(0b11, 0b011, 0b1110, 0b0010, 0b001), // RW
    kCNTP_CTL_EL02        = encode(0b11, 0b101, 0b1110, 0b0010, 0b001), // RW
    kCNTP_CVAL_EL0        = encode(0b11, 0b011, 0b1110, 0b0010, 0b010), // RW
    kCNTP_CVAL_EL02       = encode(0b11, 0b101, 0b1110, 0b0010, 0b010), // RW
    kCNTP_TVAL_EL0        = encode(0b11, 0b011, 0b1110, 0b0010, 0b000), // RW
    kCNTP_TVAL_EL02       = encode(0b11, 0b101, 0b1110, 0b0010, 0b000), // RW
    kCNTSCALE_EL2         = encode(0b11, 0b100, 0b1110, 0b0000, 0b100), // RW
    kCNTVCTSS_EL0         = encode(0b11, 0b011, 0b1110, 0b0000, 0b110), // RW
    kCNTVCT_EL0           = encode(0b11, 0b011, 0b1110, 0b0000, 0b010), // RO
    kCNTVFRQ_EL2          = encode(0b11, 0b100, 0b1110, 0b0000, 0b111), // RW
    kCNTVOFF_EL2          = encode(0b11, 0b100, 0b1110, 0b0000, 0b011), // RW
    kCNTV_CTL_EL0         = encode(0b11, 0b011, 0b1110, 0b0011, 0b001), // RW
    kCNTV_CTL_EL02        = encode(0b11, 0b101, 0b1110, 0b0011, 0b001), // RW
    kCNTV_CVAL_EL0        = encode(0b11, 0b011, 0b1110, 0b0011, 0b010), // RW
    kCNTV_CVAL_EL02       = encode(0b11, 0b101, 0b1110, 0b0011, 0b010), // RW
    kCNTV_TVAL_EL0        = encode(0b11, 0b011, 0b1110, 0b0011, 0b000), // RW
    kCNTV_TVAL_EL02       = encode(0b11, 0b101, 0b1110, 0b0011, 0b000), // RW
    kCONTEXTIDR_EL1       = encode(0b11, 0b000, 0b1101, 0b0000, 0b001), // RW
    kCONTEXTIDR_EL12      = encode(0b11, 0b101, 0b1101, 0b0000, 0b001), // RW
    kCONTEXTIDR_EL2       = encode(0b11, 0b100, 0b1101, 0b0000, 0b001), // RW
    kCPACR_EL1            = encode(0b11, 0b000, 0b0001, 0b0000, 0b010), // RW
    kCPACR_EL12           = encode(0b11, 0b101, 0b0001, 0b0000, 0b010), // RW
    kCPM_IOACC_CTL_EL3    = encode(0b11, 0b111, 0b1111, 0b0010, 0b000), // RW
    kCPTR_EL2             = encode(0b11, 0b100, 0b0001, 0b0001, 0b010), // RW
    kCPTR_EL3             = encode(0b11, 0b110, 0b0001, 0b0001, 0b010), // RW
    kCSSELR_EL1           = encode(0b11, 0b010, 0b0000, 0b0000, 0b000), // RW
    kCTR_EL0              = encode(0b11, 0b011, 0b0000, 0b0000, 0b001), // RO
    kCurrentEL            = encode(0b11, 0b000, 0b0100, 0b0010, 0b010), // RO
    kDACR32_EL2           = encode(0b11, 0b100, 0b0011, 0b0000, 0b000), // RW
    kDAIF                 = encode(0b11, 0b011, 0b0100, 0b0010, 0b001), // RW
    kDBGAUTHSTATUS_EL1    = encode(0b10, 0b000, 0b0111, 0b1110, 0b110), // RO
    kDBGBCR0_EL1          = encode(0b10, 0b000, 0b0000, 0b0000, 0b101), // RW
    kDBGBCR10_EL1         = encode(0b10, 0b000, 0b0000, 0b1010, 0b101), // RW
    kDBGBCR11_EL1         = encode(0b10, 0b000, 0b0000, 0b1011, 0b101), // RW
    kDBGBCR12_EL1         = encode(0b10, 0b000, 0b0000, 0b1100, 0b101), // RW
    kDBGBCR13_EL1         = encode(0b10, 0b000, 0b0000, 0b1101, 0b101), // RW
    kDBGBCR14_EL1         = encode(0b10, 0b000, 0b0000, 0b1110, 0b101), // RW
    kDBGBCR15_EL1         = encode(0b10, 0b000, 0b0000, 0b1111, 0b101), // RW
    kDBGBCR1_EL1          = encode(0b10, 0b000, 0b0000, 0b0001, 0b101), // RW
    kDBGBCR2_EL1          = encode(0b10, 0b000, 0b0000, 0b0010, 0b101), // RW
    kDBGBCR3_EL1          = encode(0b10, 0b000, 0b0000, 0b0011, 0b101), // RW
    kDBGBCR4_EL1          = encode(0b10, 0b000, 0b0000, 0b0100, 0b101), // RW
    kDBGBCR5_EL1          = encode(0b10, 0b000, 0b0000, 0b0101, 0b101), // RW
    kDBGBCR6_EL1          = encode(0b10, 0b000, 0b0000, 0b0110, 0b101), // RW
    kDBGBCR7_EL1          = encode(0b10, 0b000, 0b0000, 0b0111, 0b101), // RW
    kDBGBCR8_EL1          = encode(0b10, 0b000, 0b0000, 0b1000, 0b101), // RW
    kDBGBCR9_EL1          = encode(0b10, 0b000, 0b0000, 0b1001, 0b101), // RW
    kDBGBVR0_EL1          = encode(0b10, 0b000, 0b0000, 0b0000, 0b100), // RW
    kDBGBVR10_EL1         = encode(0b10, 0b000, 0b0000, 0b1010, 0b100), // RW
    kDBGBVR11_EL1         = encode(0b10, 0b000, 0b0000, 0b1011, 0b100), // RW
    kDBGBVR12_EL1         = encode(0b10, 0b000, 0b0000, 0b1100, 0b100), // RW
    kDBGBVR13_EL1         = encode(0b10, 0b000, 0b0000, 0b1101, 0b100), // RW
    kDBGBVR14_EL1         = encode(0b10, 0b000, 0b0000, 0b1110, 0b100), // RW
    kDBGBVR15_EL1         = encode(0b10, 0b000, 0b0000, 0b1111, 0b100), // RW
    kDBGBVR1_EL1          = encode(0b10, 0b000, 0b0000, 0b0001, 0b100), // RW
    kDBGBVR2_EL1          = encode(0b10, 0b000, 0b0000, 0b0010, 0b100), // RW
    kDBGBVR3_EL1          = encode(0b10, 0b000, 0b0000, 0b0011, 0b100), // RW
    kDBGBVR4_EL1          = encode(0b10, 0b000, 0b0000, 0b0100, 0b100), // RW
    kDBGBVR5_EL1          = encode(0b10, 0b000, 0b0000, 0b0101, 0b100), // RW
    kDBGBVR6_EL1          = encode(0b10, 0b000, 0b0000, 0b0110, 0b100), // RW
    kDBGBVR7_EL1          = encode(0b10, 0b000, 0b0000, 0b0111, 0b100), // RW
    kDBGBVR8_EL1          = encode(0b10, 0b000, 0b0000, 0b1000, 0b100), // RW
    kDBGBVR9_EL1          = encode(0b10, 0b000, 0b0000, 0b1001, 0b100), // RW
    kDBGCLAIMCLR_EL1      = encode(0b10, 0b000, 0b0111, 0b1001, 0b110), // RW
    kDBGCLAIMSET_EL1      = encode(0b10, 0b000, 0b0111, 0b1000, 0b110), // RW
    kDBGDTRRX_EL0         = encode(0b10, 0b011, 0b0000, 0b0101, 0b000), // RO
    kDBGDTRTX_EL0         = encode(0b10, 0b011, 0b0000, 0b0101, 0b000), // WO
    kDBGDTR_EL0           = encode(0b10, 0b011, 0b0000, 0b0100, 0b000), // RW
    kDBGPRCR_EL1          = encode(0b10, 0b000, 0b0001, 0b0100, 0b100), // RW
    kDBGVCR32_EL2         = encode(0b10, 0b100, 0b0000, 0b0111, 0b000), // RW
    kDBGWCR0_EL1          = encode(0b10, 0b000, 0b0000, 0b0000, 0b111), // RW
    kDBGWCR10_EL1         = encode(0b10, 0b000, 0b0000, 0b1010, 0b111), // RW
    kDBGWCR11_EL1         = encode(0b10, 0b000, 0b0000, 0b1011, 0b111), // RW
    kDBGWCR12_EL1         = encode(0b10, 0b000, 0b0000, 0b1100, 0b111), // RW
    kDBGWCR13_EL1         = encode(0b10, 0b000, 0b0000, 0b1101, 0b111), // RW
    kDBGWCR14_EL1         = encode(0b10, 0b000, 0b0000, 0b1110, 0b111), // RW
    kDBGWCR15_EL1         = encode(0b10, 0b000, 0b0000, 0b1111, 0b111), // RW
    kDBGWCR1_EL1          = encode(0b10, 0b000, 0b0000, 0b0001, 0b111), // RW
    kDBGWCR2_EL1          = encode(0b10, 0b000, 0b0000, 0b0010, 0b111), // RW
    kDBGWCR3_EL1          = encode(0b10, 0b000, 0b0000, 0b0011, 0b111), // RW
    kDBGWCR4_EL1          = encode(0b10, 0b000, 0b0000, 0b0100, 0b111), // RW
    kDBGWCR5_EL1          = encode(0b10, 0b000, 0b0000, 0b0101, 0b111), // RW
    kDBGWCR6_EL1          = encode(0b10, 0b000, 0b0000, 0b0110, 0b111), // RW
    kDBGWCR7_EL1          = encode(0b10, 0b000, 0b0000, 0b0111, 0b111), // RW
    kDBGWCR8_EL1          = encode(0b10, 0b000, 0b0000, 0b1000, 0b111), // RW
    kDBGWCR9_EL1          = encode(0b10, 0b000, 0b0000, 0b1001, 0b111), // RW
    kDBGWVR0_EL1          = encode(0b10, 0b000, 0b0000, 0b0000, 0b110), // RW
    kDBGWVR10_EL1         = encode(0b10, 0b000, 0b0000, 0b1010, 0b110), // RW
    kDBGWVR11_EL1         = encode(0b10, 0b000, 0b0000, 0b1011, 0b110), // RW
    kDBGWVR12_EL1         = encode(0b10, 0b000, 0b0000, 0b1100, 0b110), // RW
    kDBGWVR13_EL1         = encode(0b10, 0b000, 0b0000, 0b1101, 0b110), // RW
    kDBGWVR14_EL1         = encode(0b10, 0b000, 0b0000, 0b1110, 0b110), // RW
    kDBGWVR15_EL1         = encode(0b10, 0b000, 0b0000, 0b1111, 0b110), // RW
    kDBGWVR1_EL1          = encode(0b10, 0b000, 0b0000, 0b0001, 0b110), // RW
    kDBGWVR2_EL1          = encode(0b10, 0b000, 0b0000, 0b0010, 0b110), // RW
    kDBGWVR3_EL1          = encode(0b10, 0b000, 0b0000, 0b0011, 0b110), // RW
    kDBGWVR4_EL1          = encode(0b10, 0b000, 0b0000, 0b0100, 0b110), // RW
    kDBGWVR5_EL1          = encode(0b10, 0b000, 0b0000, 0b0101, 0b110), // RW
    kDBGWVR6_EL1          = encode(0b10, 0b000, 0b0000, 0b0110, 0b110), // RW
    kDBGWVR7_EL1          = encode(0b10, 0b000, 0b0000, 0b0111, 0b110), // RW
    kDBGWVR8_EL1          = encode(0b10, 0b000, 0b0000, 0b1000, 0b110), // RW
    kDBGWVR9_EL1          = encode(0b10, 0b000, 0b0000, 0b1001, 0b110), // RW
    kDCZID_EL0            = encode(0b11, 0b011, 0b0000, 0b0000, 0b111), // RO
    kDISR_EL1             = encode(0b11, 0b000, 0b1100, 0b0001, 0b001), // RW
    kDIT                  = encode(0b11, 0b011, 0b0100, 0b0010, 0b101), // RW
    kDLR_EL0              = encode(0b11, 0b011, 0b0100, 0b0101, 0b001), // RW
    kDSPSR_EL0            = encode(0b11, 0b011, 0b0100, 0b0101, 0b000), // RW
    kELR_EL1              = encode(0b11, 0b000, 0b0100, 0b0000, 0b001), // RW
    kELR_EL12             = encode(0b11, 0b101, 0b0100, 0b0000, 0b001), // RW
    kELR_EL2              = encode(0b11, 0b100, 0b0100, 0b0000, 0b001), // RW
    kELR_EL3              = encode(0b11, 0b110, 0b0100, 0b0000, 0b001), // RW
    kERRIDR_EL1           = encode(0b11, 0b000, 0b0101, 0b0011, 0b000), // RO
    kERRSELR_EL1          = encode(0b11, 0b000, 0b0101, 0b0011, 0b001), // RW
    kERXADDR_EL1          = encode(0b11, 0b000, 0b0101, 0b0100, 0b011), // RW
    kERXCTLR_EL1          = encode(0b11, 0b000, 0b0101, 0b0100, 0b001), // RW
    kERXFR_EL1            = encode(0b11, 0b000, 0b0101, 0b0100, 0b000), // RO
    kERXMISC0_EL1         = encode(0b11, 0b000, 0b0101, 0b0101, 0b000), // RW
    kERXMISC1_EL1         = encode(0b11, 0b000, 0b0101, 0b0101, 0b001), // RW
    kERXMISC2_EL1         = encode(0b11, 0b000, 0b0101, 0b0101, 0b010), // RW
    kERXMISC3_EL1         = encode(0b11, 0b000, 0b0101, 0b0101, 0b011), // RW
    kERXPFGCDN_EL1        = encode(0b11, 0b000, 0b0101, 0b0100, 0b110), // RW
    kERXPFGCTL_EL1        = encode(0b11, 0b000, 0b0101, 0b0100, 0b101), // RW
    kERXPFGF_EL1          = encode(0b11, 0b000, 0b0101, 0b0100, 0b100), // RO
    kERXSTATUS_EL1        = encode(0b11, 0b000, 0b0101, 0b0100, 0b010), // RW
    kESR_EL1              = encode(0b11, 0b000, 0b0101, 0b0010, 0b000), // RW
    kESR_EL12             = encode(0b11, 0b101, 0b0101, 0b0010, 0b000), // RW
    kESR_EL2              = encode(0b11, 0b100, 0b0101, 0b0010, 0b000), // RW
    kESR_EL3              = encode(0b11, 0b110, 0b0101, 0b0010, 0b000), // RW
    kFAR_EL1              = encode(0b11, 0b000, 0b0110, 0b0000, 0b000), // RW
    kFAR_EL12             = encode(0b11, 0b101, 0b0110, 0b0000, 0b000), // RW
    kFAR_EL2              = encode(0b11, 0b100, 0b0110, 0b0000, 0b000), // RW
    kFAR_EL3              = encode(0b11, 0b110, 0b0110, 0b0000, 0b000), // RW
    kFPCR                 = encode(0b11, 0b011, 0b0100, 0b0100, 0b000), // RW
    kFPEXC32_EL2          = encode(0b11, 0b100, 0b0101, 0b0011, 0b000), // RW
    kFPSR                 = encode(0b11, 0b011, 0b0100, 0b0100, 0b001), // RW
    kGCR_EL1              = encode(0b11, 0b000, 0b0001, 0b0000, 0b110), // RW
    kGMID_EL1             = encode(0b11, 0b001, 0b0000, 0b0000, 0b100), // RO
    kHACR_EL2             = encode(0b11, 0b100, 0b0001, 0b0001, 0b111), // RW
    kHCR_EL2              = encode(0b11, 0b100, 0b0001, 0b0001, 0b000), // RW
    kHDFGRTR_EL2          = encode(0b11, 0b100, 0b0011, 0b0001, 0b100), // RW
    kHDFGWTR_EL2          = encode(0b11, 0b100, 0b0011, 0b0001, 0b101), // RW
    kHFGITR_EL2           = encode(0b11, 0b100, 0b0001, 0b0001, 0b110), // RW
    kHFGRTR_EL2           = encode(0b11, 0b100, 0b0001, 0b0001, 0b100), // RW
    kHFGWTR_EL2           = encode(0b11, 0b100, 0b0001, 0b0001, 0b101), // RW
    kHPFAR_EL2            = encode(0b11, 0b100, 0b0110, 0b0000, 0b100), // RW
    kHSTR_EL2             = encode(0b11, 0b100, 0b0001, 0b0001, 0b011), // RW
    kICC_AP0R0_EL1        = encode(0b11, 0b000, 0b1100, 0b1000, 0b100), // RW
    kICC_AP0R1_EL1        = encode(0b11, 0b000, 0b1100, 0b1000, 0b101), // RW
    kICC_AP0R2_EL1        = encode(0b11, 0b000, 0b1100, 0b1000, 0b110), // RW
    kICC_AP0R3_EL1        = encode(0b11, 0b000, 0b1100, 0b1000, 0b111), // RW
    kICC_AP1R0_EL1        = encode(0b11, 0b000, 0b1100, 0b1001, 0b000), // RW
    kICC_AP1R1_EL1        = encode(0b11, 0b000, 0b1100, 0b1001, 0b001), // RW
    kICC_AP1R2_EL1        = encode(0b11, 0b000, 0b1100, 0b1001, 0b010), // RW
    kICC_AP1R3_EL1        = encode(0b11, 0b000, 0b1100, 0b1001, 0b011), // RW
    kICC_ASGI1R_EL1       = encode(0b11, 0b000, 0b1100, 0b1011, 0b110), // WO
    kICC_BPR0_EL1         = encode(0b11, 0b000, 0b1100, 0b1000, 0b011), // RW
    kICC_BPR1_EL1         = encode(0b11, 0b000, 0b1100, 0b1100, 0b011), // RW
    kICC_CTLR_EL1         = encode(0b11, 0b000, 0b1100, 0b1100, 0b100), // RW
    kICC_CTLR_EL3         = encode(0b11, 0b110, 0b1100, 0b1100, 0b100), // RW
    kICC_DIR_EL1          = encode(0b11, 0b000, 0b1100, 0b1011, 0b001), // WO
    kICC_EOIR0_EL1        = encode(0b11, 0b000, 0b1100, 0b1000, 0b001), // WO
    kICC_EOIR1_EL1        = encode(0b11, 0b000, 0b1100, 0b1100, 0b001), // WO
    kICC_HPPIR0_EL1       = encode(0b11, 0b000, 0b1100, 0b1000, 0b010), // RO
    kICC_HPPIR1_EL1       = encode(0b11, 0b000, 0b1100, 0b1100, 0b010), // RO
    kICC_IAR0_EL1         = encode(0b11, 0b000, 0b1100, 0b1000, 0b000), // RO
    kICC_IAR1_EL1         = encode(0b11, 0b000, 0b1100, 0b1100, 0b000), // RO
    kICC_IGRPEN0_EL1      = encode(0b11, 0b000, 0b1100, 0b1100, 0b110), // RW
    kICC_IGRPEN1_EL1      = encode(0b11, 0b000, 0b1100, 0b1100, 0b111), // RW
    kICC_IGRPEN1_EL3      = encode(0b11, 0b110, 0b1100, 0b1100, 0b111), // RW
    kICC_PMR_EL1          = encode(0b11, 0b000, 0b0100, 0b0110, 0b000), // RW
    kICC_RPR_EL1          = encode(0b11, 0b000, 0b1100, 0b1011, 0b011), // RO
    kICC_SGI0R_EL1        = encode(0b11, 0b000, 0b1100, 0b1011, 0b111), // WO
    kICC_SGI1R_EL1        = encode(0b11, 0b000, 0b1100, 0b1011, 0b101), // WO
    kICC_SRE_EL1          = encode(0b11, 0b000, 0b1100, 0b1100, 0b101), // RW
    kICC_SRE_EL2          = encode(0b11, 0b100, 0b1100, 0b1001, 0b101), // RW
    kICC_SRE_EL3          = encode(0b11, 0b110, 0b1100, 0b1100, 0b101), // RW
    kICH_AP0R0_EL2        = encode(0b11, 0b100, 0b1100, 0b1000, 0b000), // RW
    kICH_AP0R1_EL2        = encode(0b11, 0b100, 0b1100, 0b1000, 0b001), // RW
    kICH_AP0R2_EL2        = encode(0b11, 0b100, 0b1100, 0b1000, 0b010), // RW
    kICH_AP0R3_EL2        = encode(0b11, 0b100, 0b1100, 0b1000, 0b011), // RW
    kICH_AP1R0_EL2        = encode(0b11, 0b100, 0b1100, 0b1001, 0b000), // RW
    kICH_AP1R1_EL2        = encode(0b11, 0b100, 0b1100, 0b1001, 0b001), // RW
    kICH_AP1R2_EL2        = encode(0b11, 0b100, 0b1100, 0b1001, 0b010), // RW
    kICH_AP1R3_EL2        = encode(0b11, 0b100, 0b1100, 0b1001, 0b011), // RW
    kICH_EISR_EL2         = encode(0b11, 0b100, 0b1100, 0b1011, 0b011), // RO
    kICH_ELRSR_EL2        = encode(0b11, 0b100, 0b1100, 0b1011, 0b101), // RO
    kICH_HCR_EL2          = encode(0b11, 0b100, 0b1100, 0b1011, 0b000), // RW
    kICH_LR0_EL2          = encode(0b11, 0b100, 0b1100, 0b1100, 0b000), // RW
    kICH_LR10_EL2         = encode(0b11, 0b100, 0b1100, 0b1101, 0b010), // RW
    kICH_LR11_EL2         = encode(0b11, 0b100, 0b1100, 0b1101, 0b011), // RW
    kICH_LR12_EL2         = encode(0b11, 0b100, 0b1100, 0b1101, 0b100), // RW
    kICH_LR13_EL2         = encode(0b11, 0b100, 0b1100, 0b1101, 0b101), // RW
    kICH_LR14_EL2         = encode(0b11, 0b100, 0b1100, 0b1101, 0b110), // RW
    kICH_LR15_EL2         = encode(0b11, 0b100, 0b1100, 0b1101, 0b111), // RW
    kICH_LR1_EL2          = encode(0b11, 0b100, 0b1100, 0b1100, 0b001), // RW
    kICH_LR2_EL2          = encode(0b11, 0b100, 0b1100, 0b1100, 0b010), // RW
    kICH_LR3_EL2          = encode(0b11, 0b100, 0b1100, 0b1100, 0b011), // RW
    kICH_LR4_EL2          = encode(0b11, 0b100, 0b1100, 0b1100, 0b100), // RW
    kICH_LR5_EL2          = encode(0b11, 0b100, 0b1100, 0b1100, 0b101), // RW
    kICH_LR6_EL2          = encode(0b11, 0b100, 0b1100, 0b1100, 0b110), // RW
    kICH_LR7_EL2          = encode(0b11, 0b100, 0b1100, 0b1100, 0b111), // RW
    kICH_LR8_EL2          = encode(0b11, 0b100, 0b1100, 0b1101, 0b000), // RW
    kICH_LR9_EL2          = encode(0b11, 0b100, 0b1100, 0b1101, 0b001), // RW
    kICH_MISR_EL2         = encode(0b11, 0b100, 0b1100, 0b1011, 0b010), // RO
    kICH_VMCR_EL2         = encode(0b11, 0b100, 0b1100, 0b1011, 0b111), // RW
    kICH_VTR_EL2          = encode(0b11, 0b100, 0b1100, 0b1011, 0b001), // RO
    kID_AA64AFR0_EL1      = encode(0b11, 0b000, 0b0000, 0b0101, 0b100), // RO
    kID_AA64AFR1_EL1      = encode(0b11, 0b000, 0b0000, 0b0101, 0b101), // RO
    kID_AA64DFR0_EL1      = encode(0b11, 0b000, 0b0000, 0b0101, 0b000), // RO
    kID_AA64DFR1_EL1      = encode(0b11, 0b000, 0b0000, 0b0101, 0b001), // RO
    kID_AA64ISAR0_EL1     = encode(0b11, 0b000, 0b0000, 0b0110, 0b000), // RO
    kID_AA64ISAR1_EL1     = encode(0b11, 0b000, 0b0000, 0b0110, 0b001), // RO
    kID_AA64ISAR2_EL1     = encode(0b11, 0b000, 0b0000, 0b0110, 0b010), // RO
    kID_AA64MMFR0_EL1     = encode(0b11, 0b000, 0b0000, 0b0111, 0b000), // RO
    kID_AA64MMFR1_EL1     = encode(0b11, 0b000, 0b0000, 0b0111, 0b001), // RO
    kID_AA64MMFR2_EL1     = encode(0b11, 0b000, 0b0000, 0b0111, 0b010), // RO
    kID_AA64MMFR3_EL1     = encode(0b11, 0b000, 0b0000, 0b0111, 0b011), // RO
    kID_AA64MMFR4_EL1     = encode(0b11, 0b000, 0b0000, 0b0111, 0b100), // RO
    kID_AA64PFR0_EL1      = encode(0b11, 0b000, 0b0000, 0b0100, 0b000), // RO
    kID_AA64PFR1_EL1      = encode(0b11, 0b000, 0b0000, 0b0100, 0b001), // RO
    kID_AA64ZFR0_EL1      = encode(0b11, 0b000, 0b0000, 0b0100, 0b100), // RO
    kID_AFR0_EL1          = encode(0b11, 0b000, 0b0000, 0b0001, 0b011), // RO
    kID_DFR0_EL1          = encode(0b11, 0b000, 0b0000, 0b0001, 0b010), // RO
    kID_ISAR0_EL1         = encode(0b11, 0b000, 0b0000, 0b0010, 0b000), // RO
    kID_ISAR1_EL1         = encode(0b11, 0b000, 0b0000, 0b0010, 0b001), // RO
    kID_ISAR2_EL1         = encode(0b11, 0b000, 0b0000, 0b0010, 0b010), // RO
    kID_ISAR3_EL1         = encode(0b11, 0b000, 0b0000, 0b0010, 0b011), // RO
    kID_ISAR4_EL1         = encode(0b11, 0b000, 0b0000, 0b0010, 0b100), // RO
    kID_ISAR5_EL1         = encode(0b11, 0b000, 0b0000, 0b0010, 0b101), // RO
    kID_ISAR6_EL1         = encode(0b11, 0b000, 0b0000, 0b0010, 0b111), // RO
    kID_MMFR0_EL1         = encode(0b11, 0b000, 0b0000, 0b0001, 0b100), // RO
    kID_MMFR1_EL1         = encode(0b11, 0b000, 0b0000, 0b0001, 0b101), // RO
    kID_MMFR2_EL1         = encode(0b11, 0b000, 0b0000, 0b0001, 0b110), // RO
    kID_MMFR3_EL1         = encode(0b11, 0b000, 0b0000, 0b0001, 0b111), // RO
    kID_MMFR4_EL1         = encode(0b11, 0b000, 0b0000, 0b0010, 0b110), // RO
    kID_MMFR5_EL1         = encode(0b11, 0b000, 0b0000, 0b0011, 0b110), // RO
    kID_PFR0_EL1          = encode(0b11, 0b000, 0b0000, 0b0001, 0b000), // RO
    kID_PFR1_EL1          = encode(0b11, 0b000, 0b0000, 0b0001, 0b001), // RO
    kID_PFR2_EL1          = encode(0b11, 0b000, 0b0000, 0b0011, 0b100), // RO
    kIFSR32_EL2           = encode(0b11, 0b100, 0b0101, 0b0000, 0b001), // RW
    kISR_EL1              = encode(0b11, 0b000, 0b1100, 0b0001, 0b000), // RO
    kLORC_EL1             = encode(0b11, 0b000, 0b1010, 0b0100, 0b011), // RW
    kLOREA_EL1            = encode(0b11, 0b000, 0b1010, 0b0100, 0b001), // RW
    kLORID_EL1            = encode(0b11, 0b000, 0b1010, 0b0100, 0b111), // RO
    kLORN_EL1             = encode(0b11, 0b000, 0b1010, 0b0100, 0b010), // RW
    kLORSA_EL1            = encode(0b11, 0b000, 0b1010, 0b0100, 0b000), // RW
    kMAIR_EL1             = encode(0b11, 0b000, 0b1010, 0b0010, 0b000), // RW
    kMAIR_EL12            = encode(0b11, 0b101, 0b1010, 0b0010, 0b000), // RW
    kMAIR_EL2             = encode(0b11, 0b100, 0b1010, 0b0010, 0b000), // RW
    kMAIR_EL3             = encode(0b11, 0b110, 0b1010, 0b0010, 0b000), // RW
    kMDCCINT_EL1          = encode(0b10, 0b000, 0b0000, 0b0010, 0b000), // RW
    kMDCCSR_EL0           = encode(0b10, 0b011, 0b0000, 0b0001, 0b000), // RO
    kMDCR_EL2             = encode(0b11, 0b100, 0b0001, 0b0001, 0b001), // RW
    kMDCR_EL3             = encode(0b11, 0b110, 0b0001, 0b0011, 0b001), // RW
    kMDRAR_EL1            = encode(0b10, 0b000, 0b0001, 0b0000, 0b000), // RO
    kMDSCR_EL1            = encode(0b10, 0b000, 0b0000, 0b0010, 0b010), // RW
    kMIDR_EL1             = encode(0b11, 0b000, 0b0000, 0b0000, 0b000), // RO
    kMPAM0_EL1            = encode(0b11, 0b000, 0b1010, 0b0101, 0b001), // RW
    kMPAM1_EL1            = encode(0b11, 0b000, 0b1010, 0b0101, 0b000), // RW
    kMPAM1_EL12           = encode(0b11, 0b101, 0b1010, 0b0101, 0b000), // RW
    kMPAM2_EL2            = encode(0b11, 0b100, 0b1010, 0b0101, 0b000), // RW
    kMPAM3_EL3            = encode(0b11, 0b110, 0b1010, 0b0101, 0b000), // RW
    kMPAMHCR_EL2          = encode(0b11, 0b100, 0b1010, 0b0100, 0b000), // RW
    kMPAMIDR_EL1          = encode(0b11, 0b000, 0b1010, 0b0100, 0b100), // RO
    kMPAMVPM0_EL2         = encode(0b11, 0b100, 0b1010, 0b0110, 0b000), // RW
    kMPAMVPM1_EL2         = encode(0b11, 0b100, 0b1010, 0b0110, 0b001), // RW
    kMPAMVPM2_EL2         = encode(0b11, 0b100, 0b1010, 0b0110, 0b010), // RW
    kMPAMVPM3_EL2         = encode(0b11, 0b100, 0b1010, 0b0110, 0b011), // RW
    kMPAMVPM4_EL2         = encode(0b11, 0b100, 0b1010, 0b0110, 0b100), // RW
    kMPAMVPM5_EL2         = encode(0b11, 0b100, 0b1010, 0b0110, 0b101), // RW
    kMPAMVPM6_EL2         = encode(0b11, 0b100, 0b1010, 0b0110, 0b110), // RW
    kMPAMVPM7_EL2         = encode(0b11, 0b100, 0b1010, 0b0110, 0b111), // RW
    kMPAMVPMV_EL2         = encode(0b11, 0b100, 0b1010, 0b0100, 0b001), // RW
    kMPIDR_EL1            = encode(0b11, 0b000, 0b0000, 0b0000, 0b101), // RO
    kMVFR0_EL1            = encode(0b11, 0b000, 0b0000, 0b0011, 0b000), // RO
    kMVFR1_EL1            = encode(0b11, 0b000, 0b0000, 0b0011, 0b001), // RO
    kMVFR2_EL1            = encode(0b11, 0b000, 0b0000, 0b0011, 0b010), // RO
    kNZCV                 = encode(0b11, 0b011, 0b0100, 0b0010, 0b000), // RW
    kOSDLR_EL1            = encode(0b10, 0b000, 0b0001, 0b0011, 0b100), // RW
    kOSDTRRX_EL1          = encode(0b10, 0b000, 0b0000, 0b0000, 0b010), // RW
    kOSDTRTX_EL1          = encode(0b10, 0b000, 0b0000, 0b0011, 0b010), // RW
    kOSECCR_EL1           = encode(0b10, 0b000, 0b0000, 0b0110, 0b010), // RW
    kOSLAR_EL1            = encode(0b10, 0b000, 0b0001, 0b0000, 0b100), // WO
    kOSLSR_EL1            = encode(0b10, 0b000, 0b0001, 0b0001, 0b100), // RO
    kPAN                  = encode(0b11, 0b000, 0b0100, 0b0010, 0b011), // RW
    kPAR_EL1              = encode(0b11, 0b000, 0b0111, 0b0100, 0b000), // RW
    kPMBIDR_EL1           = encode(0b11, 0b000, 0b1001, 0b1010, 0b111), // RO
    kPMBLIMITR_EL1        = encode(0b11, 0b000, 0b1001, 0b1010, 0b000), // RW
    kPMBPTR_EL1           = encode(0b11, 0b000, 0b1001, 0b1010, 0b001), // RW
    kPMBSR_EL1            = encode(0b11, 0b000, 0b1001, 0b1010, 0b011), // RW
    kPMCCFILTR_EL0        = encode(0b11, 0b011, 0b1110, 0b1111, 0b111), // RW
    kPMCCNTR_EL0          = encode(0b11, 0b011, 0b1001, 0b1101, 0b000), // RW
    kPMCEID0_EL0          = encode(0b11, 0b011, 0b1001, 0b1100, 0b110), // RO
    kPMCEID1_EL0          = encode(0b11, 0b011, 0b1001, 0b1100, 0b111), // RO
    kPMCNTENCLR_EL0       = encode(0b11, 0b011, 0b1001, 0b1100, 0b010), // RW
    kPMCNTENSET_EL0       = encode(0b11, 0b011, 0b1001, 0b1100, 0b001), // RW
    kPMCR_EL0             = encode(0b11, 0b011, 0b1001, 0b1100, 0b000), // RW
    kPMEVCNTR0_EL0        = encode(0b11, 0b011, 0b1110, 0b1000, 0b000), // RW
    kPMEVCNTR10_EL0       = encode(0b11, 0b011, 0b1110, 0b1001, 0b010), // RW
    kPMEVCNTR11_EL0       = encode(0b11, 0b011, 0b1110, 0b1001, 0b011), // RW
    kPMEVCNTR12_EL0       = encode(0b11, 0b011, 0b1110, 0b1001, 0b100), // RW
    kPMEVCNTR13_EL0       = encode(0b11, 0b011, 0b1110, 0b1001, 0b101), // RW
    kPMEVCNTR14_EL0       = encode(0b11, 0b011, 0b1110, 0b1001, 0b110), // RW
    kPMEVCNTR15_EL0       = encode(0b11, 0b011, 0b1110, 0b1001, 0b111), // RW
    kPMEVCNTR16_EL0       = encode(0b11, 0b011, 0b1110, 0b1010, 0b000), // RW
    kPMEVCNTR17_EL0       = encode(0b11, 0b011, 0b1110, 0b1010, 0b001), // RW
    kPMEVCNTR18_EL0       = encode(0b11, 0b011, 0b1110, 0b1010, 0b010), // RW
    kPMEVCNTR19_EL0       = encode(0b11, 0b011, 0b1110, 0b1010, 0b011), // RW
    kPMEVCNTR1_EL0        = encode(0b11, 0b011, 0b1110, 0b1000, 0b001), // RW
    kPMEVCNTR20_EL0       = encode(0b11, 0b011, 0b1110, 0b1010, 0b100), // RW
    kPMEVCNTR21_EL0       = encode(0b11, 0b011, 0b1110, 0b1010, 0b101), // RW
    kPMEVCNTR22_EL0       = encode(0b11, 0b011, 0b1110, 0b1010, 0b110), // RW
    kPMEVCNTR23_EL0       = encode(0b11, 0b011, 0b1110, 0b1010, 0b111), // RW
    kPMEVCNTR24_EL0       = encode(0b11, 0b011, 0b1110, 0b1011, 0b000), // RW
    kPMEVCNTR25_EL0       = encode(0b11, 0b011, 0b1110, 0b1011, 0b001), // RW
    kPMEVCNTR26_EL0       = encode(0b11, 0b011, 0b1110, 0b1011, 0b010), // RW
    kPMEVCNTR27_EL0       = encode(0b11, 0b011, 0b1110, 0b1011, 0b011), // RW
    kPMEVCNTR28_EL0       = encode(0b11, 0b011, 0b1110, 0b1011, 0b100), // RW
    kPMEVCNTR29_EL0       = encode(0b11, 0b011, 0b1110, 0b1011, 0b101), // RW
    kPMEVCNTR2_EL0        = encode(0b11, 0b011, 0b1110, 0b1000, 0b010), // RW
    kPMEVCNTR30_EL0       = encode(0b11, 0b011, 0b1110, 0b1011, 0b110), // RW
    kPMEVCNTR3_EL0        = encode(0b11, 0b011, 0b1110, 0b1000, 0b011), // RW
    kPMEVCNTR4_EL0        = encode(0b11, 0b011, 0b1110, 0b1000, 0b100), // RW
    kPMEVCNTR5_EL0        = encode(0b11, 0b011, 0b1110, 0b1000, 0b101), // RW
    kPMEVCNTR6_EL0        = encode(0b11, 0b011, 0b1110, 0b1000, 0b110), // RW
    kPMEVCNTR7_EL0        = encode(0b11, 0b011, 0b1110, 0b1000, 0b111), // RW
    kPMEVCNTR8_EL0        = encode(0b11, 0b011, 0b1110, 0b1001, 0b000), // RW
    kPMEVCNTR9_EL0        = encode(0b11, 0b011, 0b1110, 0b1001, 0b001), // RW
    kPMEVTYPER0_EL0       = encode(0b11, 0b011, 0b1110, 0b1100, 0b000), // RW
    kPMEVTYPER10_EL0      = encode(0b11, 0b011, 0b1110, 0b1101, 0b010), // RW
    kPMEVTYPER11_EL0      = encode(0b11, 0b011, 0b1110, 0b1101, 0b011), // RW
    kPMEVTYPER12_EL0      = encode(0b11, 0b011, 0b1110, 0b1101, 0b100), // RW
    kPMEVTYPER13_EL0      = encode(0b11, 0b011, 0b1110, 0b1101, 0b101), // RW
    kPMEVTYPER14_EL0      = encode(0b11, 0b011, 0b1110, 0b1101, 0b110), // RW
    kPMEVTYPER15_EL0      = encode(0b11, 0b011, 0b1110, 0b1101, 0b111), // RW
    kPMEVTYPER16_EL0      = encode(0b11, 0b011, 0b1110, 0b1110, 0b000), // RW
    kPMEVTYPER17_EL0      = encode(0b11, 0b011, 0b1110, 0b1110, 0b001), // RW
    kPMEVTYPER18_EL0      = encode(0b11, 0b011, 0b1110, 0b1110, 0b010), // RW
    kPMEVTYPER19_EL0      = encode(0b11, 0b011, 0b1110, 0b1110, 0b011), // RW
    kPMEVTYPER1_EL0       = encode(0b11, 0b011, 0b1110, 0b1100, 0b001), // RW
    kPMEVTYPER20_EL0      = encode(0b11, 0b011, 0b1110, 0b1110, 0b100), // RW
    kPMEVTYPER21_EL0      = encode(0b11, 0b011, 0b1110, 0b1110, 0b101), // RW
    kPMEVTYPER22_EL0      = encode(0b11, 0b011, 0b1110, 0b1110, 0b110), // RW
    kPMEVTYPER23_EL0      = encode(0b11, 0b011, 0b1110, 0b1110, 0b111), // RW
    kPMEVTYPER24_EL0      = encode(0b11, 0b011, 0b1110, 0b1111, 0b000), // RW
    kPMEVTYPER25_EL0      = encode(0b11, 0b011, 0b1110, 0b1111, 0b001), // RW
    kPMEVTYPER26_EL0      = encode(0b11, 0b011, 0b1110, 0b1111, 0b010), // RW
    kPMEVTYPER27_EL0      = encode(0b11, 0b011, 0b1110, 0b1111, 0b011), // RW
    kPMEVTYPER28_EL0      = encode(0b11, 0b011, 0b1110, 0b1111, 0b100), // RW
    kPMEVTYPER29_EL0      = encode(0b11, 0b011, 0b1110, 0b1111, 0b101), // RW
    kPMEVTYPER2_EL0       = encode(0b11, 0b011, 0b1110, 0b1100, 0b010), // RW
    kPMEVTYPER30_EL0      = encode(0b11, 0b011, 0b1110, 0b1111, 0b110), // RW
    kPMEVTYPER3_EL0       = encode(0b11, 0b011, 0b1110, 0b1100, 0b011), // RW
    kPMEVTYPER4_EL0       = encode(0b11, 0b011, 0b1110, 0b1100, 0b100), // RW
    kPMEVTYPER5_EL0       = encode(0b11, 0b011, 0b1110, 0b1100, 0b101), // RW
    kPMEVTYPER6_EL0       = encode(0b11, 0b011, 0b1110, 0b1100, 0b110), // RW
    kPMEVTYPER7_EL0       = encode(0b11, 0b011, 0b1110, 0b1100, 0b111), // RW
    kPMEVTYPER8_EL0       = encode(0b11, 0b011, 0b1110, 0b1101, 0b000), // RW
    kPMEVTYPER9_EL0       = encode(0b11, 0b011, 0b1110, 0b1101, 0b001), // RW
    kPMINTENCLR_EL1       = encode(0b11, 0b000, 0b1001, 0b1110, 0b010), // RW
    kPMINTENSET_EL1       = encode(0b11, 0b000, 0b1001, 0b1110, 0b001), // RW
    kPMMIR_EL1            = encode(0b11, 0b000, 0b1001, 0b1110, 0b110), // RW
    kPMOVSCLR_EL0         = encode(0b11, 0b011, 0b1001, 0b1100, 0b011), // RW
    kPMOVSSET_EL0         = encode(0b11, 0b011, 0b1001, 0b1110, 0b011), // RW
    kPMSCR_EL1            = encode(0b11, 0b000, 0b1001, 0b1001, 0b000), // RW
    kPMSCR_EL12           = encode(0b11, 0b101, 0b1001, 0b1001, 0b000), // RW
    kPMSCR_EL2            = encode(0b11, 0b100, 0b1001, 0b1001, 0b000), // RW
    kPMSELR_EL0           = encode(0b11, 0b011, 0b1001, 0b1100, 0b101), // RW
    kPMSEVFR_EL1          = encode(0b11, 0b000, 0b1001, 0b1001, 0b101), // RW
    kPMSFCR_EL1           = encode(0b11, 0b000, 0b1001, 0b1001, 0b100), // RW
    kPMSICR_EL1           = encode(0b11, 0b000, 0b1001, 0b1001, 0b010), // RW
    kPMSIDR_EL1           = encode(0b11, 0b000, 0b1001, 0b1001, 0b111), // RO
    kPMSIRR_EL1           = encode(0b11, 0b000, 0b1001, 0b1001, 0b011), // RW
    kPMSLATFR_EL1         = encode(0b11, 0b000, 0b1001, 0b1001, 0b110), // RW
    kPMSWINC_EL0          = encode(0b11, 0b011, 0b1001, 0b1100, 0b100), // WO
    kPMUSERENR_EL0        = encode(0b11, 0b011, 0b1001, 0b1110, 0b000), // RW
    kPMXEVCNTR_EL0        = encode(0b11, 0b011, 0b1001, 0b1101, 0b010), // RW
    kPMXEVTYPER_EL0       = encode(0b11, 0b011, 0b1001, 0b1101, 0b001), // RW
    kREVIDR_EL1           = encode(0b11, 0b000, 0b0000, 0b0000, 0b110), // RO
    kRGSR_EL1             = encode(0b11, 0b000, 0b0001, 0b0000, 0b101), // RW
    kRMR_EL1              = encode(0b11, 0b000, 0b1100, 0b0000, 0b010), // RW
    kRMR_EL2              = encode(0b11, 0b100, 0b1100, 0b0000, 0b010), // RW
    kRMR_EL3              = encode(0b11, 0b110, 0b1100, 0b0000, 0b010), // RW
    kRNDR                 = encode(0b11, 0b011, 0b0010, 0b0100, 0b000), // RO
    kRNDRRS               = encode(0b11, 0b011, 0b0010, 0b0100, 0b001), // RO
    kRVBAR_EL1            = encode(0b11, 0b000, 0b1100, 0b0000, 0b001), // RO
    kRVBAR_EL2            = encode(0b11, 0b100, 0b1100, 0b0000, 0b001), // RO
    kRVBAR_EL3            = encode(0b11, 0b110, 0b1100, 0b0000, 0b001), // RO
    kSCR_EL3              = encode(0b11, 0b110, 0b0001, 0b0001, 0b000), // RW
    kSCTLR_EL1            = encode(0b11, 0b000, 0b0001, 0b0000, 0b000), // RW
    kSCTLR_EL12           = encode(0b11, 0b101, 0b0001, 0b0000, 0b000), // RW
    kSCTLR_EL2            = encode(0b11, 0b100, 0b0001, 0b0000, 0b000), // RW
    kSCTLR_EL3            = encode(0b11, 0b110, 0b0001, 0b0000, 0b000), // RW
    kSCXTNUM_EL0          = encode(0b11, 0b011, 0b1101, 0b0000, 0b111), // RW
    kSCXTNUM_EL1          = encode(0b11, 0b000, 0b1101, 0b0000, 0b111), // RW
    kSCXTNUM_EL12         = encode(0b11, 0b101, 0b1101, 0b0000, 0b111), // RW
    kSCXTNUM_EL2          = encode(0b11, 0b100, 0b1101, 0b0000, 0b111), // RW
    kSCXTNUM_EL3          = encode(0b11, 0b110, 0b1101, 0b0000, 0b111), // RW
    kSDER32_EL2           = encode(0b11, 0b100, 0b0001, 0b0011, 0b001), // RW
    kSDER32_EL3           = encode(0b11, 0b110, 0b0001, 0b0001, 0b001), // RW
    kSPSR_EL1             = encode(0b11, 0b000, 0b0100, 0b0000, 0b000), // RW
    kSPSR_EL12            = encode(0b11, 0b101, 0b0100, 0b0000, 0b000), // RW
    kSPSR_EL2             = encode(0b11, 0b100, 0b0100, 0b0000, 0b000), // RW
    kSPSR_EL3             = encode(0b11, 0b110, 0b0100, 0b0000, 0b000), // RW
    kSPSR_abt             = encode(0b11, 0b100, 0b0100, 0b0011, 0b001), // RW
    kSPSR_fiq             = encode(0b11, 0b100, 0b0100, 0b0011, 0b011), // RW
    kSPSR_irq             = encode(0b11, 0b100, 0b0100, 0b0011, 0b000), // RW
    kSPSR_und             = encode(0b11, 0b100, 0b0100, 0b0011, 0b010), // RW
    kSPSel                = encode(0b11, 0b000, 0b0100, 0b0010, 0b000), // RW
    kSP_EL0               = encode(0b11, 0b000, 0b0100, 0b0001, 0b000), // RW
    kSP_EL1               = encode(0b11, 0b100, 0b0100, 0b0001, 0b000), // RW
    kSP_EL2               = encode(0b11, 0b110, 0b0100, 0b0001, 0b000), // RW
    kSSBS                 = encode(0b11, 0b011, 0b0100, 0b0010, 0b110), // RW
    kTCO                  = encode(0b11, 0b011, 0b0100, 0b0010, 0b111), // RW
    kTCR_EL1              = encode(0b11, 0b000, 0b0010, 0b0000, 0b010), // RW
    kTCR_EL12             = encode(0b11, 0b101, 0b0010, 0b0000, 0b010), // RW
    kTCR_EL2              = encode(0b11, 0b100, 0b0010, 0b0000, 0b010), // RW
    kTCR_EL3              = encode(0b11, 0b110, 0b0010, 0b0000, 0b010), // RW
    kTEECR32_EL1          = encode(0b10, 0b010, 0b0000, 0b0000, 0b000), // RW
    kTEEHBR32_EL1         = encode(0b10, 0b010, 0b0001, 0b0000, 0b000), // RW
    kTFSRE0_EL1           = encode(0b11, 0b000, 0b0101, 0b0110, 0b001), // RW
    kTFSR_EL1             = encode(0b11, 0b000, 0b0101, 0b0110, 0b000), // RW
    kTFSR_EL12            = encode(0b11, 0b101, 0b0101, 0b0110, 0b000), // RW
    kTFSR_EL2             = encode(0b11, 0b100, 0b0101, 0b0110, 0b000), // RW
    kTFSR_EL3             = encode(0b11, 0b110, 0b0101, 0b0110, 0b000), // RW
    kTPIDRRO_EL0          = encode(0b11, 0b011, 0b1101, 0b0000, 0b011), // RW
    kTPIDR_EL0            = encode(0b11, 0b011, 0b1101, 0b0000, 0b010), // RW
    kTPIDR_EL1            = encode(0b11, 0b000, 0b1101, 0b0000, 0b100), // RW
    kTPIDR_EL2            = encode(0b11, 0b100, 0b1101, 0b0000, 0b010), // RW
    kTPIDR_EL3            = encode(0b11, 0b110, 0b1101, 0b0000, 0b010), // RW
    kTRBBASER_EL1         = encode(0b11, 0b000, 0b1001, 0b1011, 0b010), // RW
    kTRBIDR_EL1           = encode(0b11, 0b000, 0b1001, 0b1011, 0b111), // RO
    kTRBLIMITR_EL1        = encode(0b11, 0b000, 0b1001, 0b1011, 0b000), // RW
    kTRBMAR_EL1           = encode(0b11, 0b000, 0b1001, 0b1011, 0b100), // RW
    kTRBPTR_EL1           = encode(0b11, 0b000, 0b1001, 0b1011, 0b001), // RW
    kTRBSR_EL1            = encode(0b11, 0b000, 0b1001, 0b1011, 0b011), // RW
    kTRBTRG_EL1           = encode(0b11, 0b000, 0b1001, 0b1011, 0b110), // RW
    kTRCACATR0            = encode(0b10, 0b001, 0b0010, 0b0000, 0b010), // RW
    kTRCACATR1            = encode(0b10, 0b001, 0b0010, 0b0010, 0b010), // RW
    kTRCACATR10           = encode(0b10, 0b001, 0b0010, 0b0100, 0b011), // RW
    kTRCACATR11           = encode(0b10, 0b001, 0b0010, 0b0110, 0b011), // RW
    kTRCACATR12           = encode(0b10, 0b001, 0b0010, 0b1000, 0b011), // RW
    kTRCACATR13           = encode(0b10, 0b001, 0b0010, 0b1010, 0b011), // RW
    kTRCACATR14           = encode(0b10, 0b001, 0b0010, 0b1100, 0b011), // RW
    kTRCACATR15           = encode(0b10, 0b001, 0b0010, 0b1110, 0b011), // RW
    kTRCACATR2            = encode(0b10, 0b001, 0b0010, 0b0100, 0b010), // RW
    kTRCACATR3            = encode(0b10, 0b001, 0b0010, 0b0110, 0b010), // RW
    kTRCACATR4            = encode(0b10, 0b001, 0b0010, 0b1000, 0b010), // RW
    kTRCACATR5            = encode(0b10, 0b001, 0b0010, 0b1010, 0b010), // RW
    kTRCACATR6            = encode(0b10, 0b001, 0b0010, 0b1100, 0b010), // RW
    kTRCACATR7            = encode(0b10, 0b001, 0b0010, 0b1110, 0b010), // RW
    kTRCACATR8            = encode(0b10, 0b001, 0b0010, 0b0000, 0b011), // RW
    kTRCACATR9            = encode(0b10, 0b001, 0b0010, 0b0010, 0b011), // RW
    kTRCACVR0             = encode(0b10, 0b001, 0b0010, 0b0000, 0b000), // RW
    kTRCACVR1             = encode(0b10, 0b001, 0b0010, 0b0010, 0b000), // RW
    kTRCACVR10            = encode(0b10, 0b001, 0b0010, 0b0100, 0b001), // RW
    kTRCACVR11            = encode(0b10, 0b001, 0b0010, 0b0110, 0b001), // RW
    kTRCACVR12            = encode(0b10, 0b001, 0b0010, 0b1000, 0b001), // RW
    kTRCACVR13            = encode(0b10, 0b001, 0b0010, 0b1010, 0b001), // RW
    kTRCACVR14            = encode(0b10, 0b001, 0b0010, 0b1100, 0b001), // RW
    kTRCACVR15            = encode(0b10, 0b001, 0b0010, 0b1110, 0b001), // RW
    kTRCACVR2             = encode(0b10, 0b001, 0b0010, 0b0100, 0b000), // RW
    kTRCACVR3             = encode(0b10, 0b001, 0b0010, 0b0110, 0b000), // RW
    kTRCACVR4             = encode(0b10, 0b001, 0b0010, 0b1000, 0b000), // RW
    kTRCACVR5             = encode(0b10, 0b001, 0b0010, 0b1010, 0b000), // RW
    kTRCACVR6             = encode(0b10, 0b001, 0b0010, 0b1100, 0b000), // RW
    kTRCACVR7             = encode(0b10, 0b001, 0b0010, 0b1110, 0b000), // RW
    kTRCACVR8             = encode(0b10, 0b001, 0b0010, 0b0000, 0b001), // RW
    kTRCACVR9             = encode(0b10, 0b001, 0b0010, 0b0010, 0b001), // RW
    kTRCAUTHSTATUS        = encode(0b10, 0b001, 0b0111, 0b1110, 0b110), // RO
    kTRCAUXCTLR           = encode(0b10, 0b001, 0b0000, 0b0110, 0b000), // RW
    kTRCBBCTLR            = encode(0b10, 0b001, 0b0000, 0b1111, 0b000), // RW
    kTRCCCCTLR            = encode(0b10, 0b001, 0b0000, 0b1110, 0b000), // RW
    kTRCCIDCCTLR0         = encode(0b10, 0b001, 0b0011, 0b0000, 0b010), // RW
    kTRCCIDCCTLR1         = encode(0b10, 0b001, 0b0011, 0b0001, 0b010), // RW
    kTRCCIDCVR0           = encode(0b10, 0b001, 0b0011, 0b0000, 0b000), // RW
    kTRCCIDCVR1           = encode(0b10, 0b001, 0b0011, 0b0010, 0b000), // RW
    kTRCCIDCVR2           = encode(0b10, 0b001, 0b0011, 0b0100, 0b000), // RW
    kTRCCIDCVR3           = encode(0b10, 0b001, 0b0011, 0b0110, 0b000), // RW
    kTRCCIDCVR4           = encode(0b10, 0b001, 0b0011, 0b1000, 0b000), // RW
    kTRCCIDCVR5           = encode(0b10, 0b001, 0b0011, 0b1010, 0b000), // RW
    kTRCCIDCVR6           = encode(0b10, 0b001, 0b0011, 0b1100, 0b000), // RW
    kTRCCIDCVR7           = encode(0b10, 0b001, 0b0011, 0b1110, 0b000), // RW
    kTRCCIDR0             = encode(0b10, 0b001, 0b0111, 0b1100, 0b111), // RO
    kTRCCIDR1             = encode(0b10, 0b001, 0b0111, 0b1101, 0b111), // RO
    kTRCCIDR2             = encode(0b10, 0b001, 0b0111, 0b1110, 0b111), // RO
    kTRCCIDR3             = encode(0b10, 0b001, 0b0111, 0b1111, 0b111), // RO
    kTRCCLAIMCLR          = encode(0b10, 0b001, 0b0111, 0b1001, 0b110), // RW
    kTRCCLAIMSET          = encode(0b10, 0b001, 0b0111, 0b1000, 0b110), // RW
    kTRCCNTCTLR0          = encode(0b10, 0b001, 0b0000, 0b0100, 0b101), // RW
    kTRCCNTCTLR1          = encode(0b10, 0b001, 0b0000, 0b0101, 0b101), // RW
    kTRCCNTCTLR2          = encode(0b10, 0b001, 0b0000, 0b0110, 0b101), // RW
    kTRCCNTCTLR3          = encode(0b10, 0b001, 0b0000, 0b0111, 0b101), // RW
    kTRCCNTRLDVR0         = encode(0b10, 0b001, 0b0000, 0b0000, 0b101), // RW
    kTRCCNTRLDVR1         = encode(0b10, 0b001, 0b0000, 0b0001, 0b101), // RW
    kTRCCNTRLDVR2         = encode(0b10, 0b001, 0b0000, 0b0010, 0b101), // RW
    kTRCCNTRLDVR3         = encode(0b10, 0b001, 0b0000, 0b0011, 0b101), // RW
    kTRCCNTVR0            = encode(0b10, 0b001, 0b0000, 0b1000, 0b101), // RW
    kTRCCNTVR1            = encode(0b10, 0b001, 0b0000, 0b1001, 0b101), // RW
    kTRCCNTVR2            = encode(0b10, 0b001, 0b0000, 0b1010, 0b101), // RW
    kTRCCNTVR3            = encode(0b10, 0b001, 0b0000, 0b1011, 0b101), // RW
    kTRCCONFIGR           = encode(0b10, 0b001, 0b0000, 0b0100, 0b000), // RW
    kTRCDEVAFF0           = encode(0b10, 0b001, 0b0111, 0b1010, 0b110), // RO
    kTRCDEVAFF1           = encode(0b10, 0b001, 0b0111, 0b1011, 0b110), // RO
    kTRCDEVARCH           = encode(0b10, 0b001, 0b0111, 0b1111, 0b110), // RO
    kTRCDEVID             = encode(0b10, 0b001, 0b0111, 0b0010, 0b111), // RO
    kTRCDEVTYPE           = encode(0b10, 0b001, 0b0111, 0b0011, 0b111), // RO
    kTRCDVCMR0            = encode(0b10, 0b001, 0b0010, 0b0000, 0b110), // RW
    kTRCDVCMR1            = encode(0b10, 0b001, 0b0010, 0b0100, 0b110), // RW
    kTRCDVCMR2            = encode(0b10, 0b001, 0b0010, 0b1000, 0b110), // RW
    kTRCDVCMR3            = encode(0b10, 0b001, 0b0010, 0b1100, 0b110), // RW
    kTRCDVCMR4            = encode(0b10, 0b001, 0b0010, 0b0000, 0b111), // RW
    kTRCDVCMR5            = encode(0b10, 0b001, 0b0010, 0b0100, 0b111), // RW
    kTRCDVCMR6            = encode(0b10, 0b001, 0b0010, 0b1000, 0b111), // RW
    kTRCDVCMR7            = encode(0b10, 0b001, 0b0010, 0b1100, 0b111), // RW
    kTRCDVCVR0            = encode(0b10, 0b001, 0b0010, 0b0000, 0b100), // RW
    kTRCDVCVR1            = encode(0b10, 0b001, 0b0010, 0b0100, 0b100), // RW
    kTRCDVCVR2            = encode(0b10, 0b001, 0b0010, 0b1000, 0b100), // RW
    kTRCDVCVR3            = encode(0b10, 0b001, 0b0010, 0b1100, 0b100), // RW
    kTRCDVCVR4            = encode(0b10, 0b001, 0b0010, 0b0000, 0b101), // RW
    kTRCDVCVR5            = encode(0b10, 0b001, 0b0010, 0b0100, 0b101), // RW
    kTRCDVCVR6            = encode(0b10, 0b001, 0b0010, 0b1000, 0b101), // RW
    kTRCDVCVR7            = encode(0b10, 0b001, 0b0010, 0b1100, 0b101), // RW
    kTRCEVENTCTL0R        = encode(0b10, 0b001, 0b0000, 0b1000, 0b000), // RW
    kTRCEVENTCTL1R        = encode(0b10, 0b001, 0b0000, 0b1001, 0b000), // RW
    kTRCEXTINSELR         = encode(0b10, 0b001, 0b0000, 0b1000, 0b100), // RW
    kTRCEXTINSELR0        = encode(0b10, 0b001, 0b0000, 0b1000, 0b100), // RW
    kTRCEXTINSELR1        = encode(0b10, 0b001, 0b0000, 0b1001, 0b100), // RW
    kTRCEXTINSELR2        = encode(0b10, 0b001, 0b0000, 0b1010, 0b100), // RW
    kTRCEXTINSELR3        = encode(0b10, 0b001, 0b0000, 0b1011, 0b100), // RW
    kTRCIDR0              = encode(0b10, 0b001, 0b0000, 0b1000, 0b111), // RO
    kTRCIDR1              = encode(0b10, 0b001, 0b0000, 0b1001, 0b111), // RO
    kTRCIDR10             = encode(0b10, 0b001, 0b0000, 0b0010, 0b110), // RO
    kTRCIDR11             = encode(0b10, 0b001, 0b0000, 0b0011, 0b110), // RO
    kTRCIDR12             = encode(0b10, 0b001, 0b0000, 0b0100, 0b110), // RO
    kTRCIDR13             = encode(0b10, 0b001, 0b0000, 0b0101, 0b110), // RO
    kTRCIDR2              = encode(0b10, 0b001, 0b0000, 0b1010, 0b111), // RO
    kTRCIDR3              = encode(0b10, 0b001, 0b0000, 0b1011, 0b111), // RO
    kTRCIDR4              = encode(0b10, 0b001, 0b0000, 0b1100, 0b111), // RO
    kTRCIDR5              = encode(0b10, 0b001, 0b0000, 0b1101, 0b111), // RO
    kTRCIDR6              = encode(0b10, 0b001, 0b0000, 0b1110, 0b111), // RO
    kTRCIDR7              = encode(0b10, 0b001, 0b0000, 0b1111, 0b111), // RO
    kTRCIDR8              = encode(0b10, 0b001, 0b0000, 0b0000, 0b110), // RO
    kTRCIDR9              = encode(0b10, 0b001, 0b0000, 0b0001, 0b110), // RO
    kTRCIMSPEC0           = encode(0b10, 0b001, 0b0000, 0b0000, 0b111), // RW
    kTRCIMSPEC1           = encode(0b10, 0b001, 0b0000, 0b0001, 0b111), // RW
    kTRCIMSPEC2           = encode(0b10, 0b001, 0b0000, 0b0010, 0b111), // RW
    kTRCIMSPEC3           = encode(0b10, 0b001, 0b0000, 0b0011, 0b111), // RW
    kTRCIMSPEC4           = encode(0b10, 0b001, 0b0000, 0b0100, 0b111), // RW
    kTRCIMSPEC5           = encode(0b10, 0b001, 0b0000, 0b0101, 0b111), // RW
    kTRCIMSPEC6           = encode(0b10, 0b001, 0b0000, 0b0110, 0b111), // RW
    kTRCIMSPEC7           = encode(0b10, 0b001, 0b0000, 0b0111, 0b111), // RW
    kTRCITCTRL            = encode(0b10, 0b001, 0b0111, 0b0000, 0b100), // RW
    kTRCLAR               = encode(0b10, 0b001, 0b0111, 0b1100, 0b110), // WO
    kTRCLSR               = encode(0b10, 0b001, 0b0111, 0b1101, 0b110), // RO
    kTRCOSLAR             = encode(0b10, 0b001, 0b0001, 0b0000, 0b100), // WO
    kTRCOSLSR             = encode(0b10, 0b001, 0b0001, 0b0001, 0b100), // RO
    kTRCPDCR              = encode(0b10, 0b001, 0b0001, 0b0100, 0b100), // RW
    kTRCPDSR              = encode(0b10, 0b001, 0b0001, 0b0101, 0b100), // RO
    kTRCPIDR0             = encode(0b10, 0b001, 0b0111, 0b1000, 0b111), // RO
    kTRCPIDR1             = encode(0b10, 0b001, 0b0111, 0b1001, 0b111), // RO
    kTRCPIDR2             = encode(0b10, 0b001, 0b0111, 0b1010, 0b111), // RO
    kTRCPIDR3             = encode(0b10, 0b001, 0b0111, 0b1011, 0b111), // RO
    kTRCPIDR4             = encode(0b10, 0b001, 0b0111, 0b0100, 0b111), // RO
    kTRCPIDR5             = encode(0b10, 0b001, 0b0111, 0b0101, 0b111), // RO
    kTRCPIDR6             = encode(0b10, 0b001, 0b0111, 0b0110, 0b111), // RO
    kTRCPIDR7             = encode(0b10, 0b001, 0b0111, 0b0111, 0b111), // RO
    kTRCPRGCTLR           = encode(0b10, 0b001, 0b0000, 0b0001, 0b000), // RW
    kTRCPROCSELR          = encode(0b10, 0b001, 0b0000, 0b0010, 0b000), // RW
    kTRCQCTLR             = encode(0b10, 0b001, 0b0000, 0b0001, 0b001), // RW
    kTRCRSCTLR10          = encode(0b10, 0b001, 0b0001, 0b1010, 0b000), // RW
    kTRCRSCTLR11          = encode(0b10, 0b001, 0b0001, 0b1011, 0b000), // RW
    kTRCRSCTLR12          = encode(0b10, 0b001, 0b0001, 0b1100, 0b000), // RW
    kTRCRSCTLR13          = encode(0b10, 0b001, 0b0001, 0b1101, 0b000), // RW
    kTRCRSCTLR14          = encode(0b10, 0b001, 0b0001, 0b1110, 0b000), // RW
    kTRCRSCTLR15          = encode(0b10, 0b001, 0b0001, 0b1111, 0b000), // RW
    kTRCRSCTLR16          = encode(0b10, 0b001, 0b0001, 0b0000, 0b001), // RW
    kTRCRSCTLR17          = encode(0b10, 0b001, 0b0001, 0b0001, 0b001), // RW
    kTRCRSCTLR18          = encode(0b10, 0b001, 0b0001, 0b0010, 0b001), // RW
    kTRCRSCTLR19          = encode(0b10, 0b001, 0b0001, 0b0011, 0b001), // RW
    kTRCRSCTLR2           = encode(0b10, 0b001, 0b0001, 0b0010, 0b000), // RW
    kTRCRSCTLR20          = encode(0b10, 0b001, 0b0001, 0b0100, 0b001), // RW
    kTRCRSCTLR21          = encode(0b10, 0b001, 0b0001, 0b0101, 0b001), // RW
    kTRCRSCTLR22          = encode(0b10, 0b001, 0b0001, 0b0110, 0b001), // RW
    kTRCRSCTLR23          = encode(0b10, 0b001, 0b0001, 0b0111, 0b001), // RW
    kTRCRSCTLR24          = encode(0b10, 0b001, 0b0001, 0b1000, 0b001), // RW
    kTRCRSCTLR25          = encode(0b10, 0b001, 0b0001, 0b1001, 0b001), // RW
    kTRCRSCTLR26          = encode(0b10, 0b001, 0b0001, 0b1010, 0b001), // RW
    kTRCRSCTLR27          = encode(0b10, 0b001, 0b0001, 0b1011, 0b001), // RW
    kTRCRSCTLR28          = encode(0b10, 0b001, 0b0001, 0b1100, 0b001), // RW
    kTRCRSCTLR29          = encode(0b10, 0b001, 0b0001, 0b1101, 0b001), // RW
    kTRCRSCTLR3           = encode(0b10, 0b001, 0b0001, 0b0011, 0b000), // RW
    kTRCRSCTLR30          = encode(0b10, 0b001, 0b0001, 0b1110, 0b001), // RW
    kTRCRSCTLR31          = encode(0b10, 0b001, 0b0001, 0b1111, 0b001), // RW
    kTRCRSCTLR4           = encode(0b10, 0b001, 0b0001, 0b0100, 0b000), // RW
    kTRCRSCTLR5           = encode(0b10, 0b001, 0b0001, 0b0101, 0b000), // RW
    kTRCRSCTLR6           = encode(0b10, 0b001, 0b0001, 0b0110, 0b000), // RW
    kTRCRSCTLR7           = encode(0b10, 0b001, 0b0001, 0b0111, 0b000), // RW
    kTRCRSCTLR8           = encode(0b10, 0b001, 0b0001, 0b1000, 0b000), // RW
    kTRCRSCTLR9           = encode(0b10, 0b001, 0b0001, 0b1001, 0b000), // RW
    kTRCRSR               = encode(0b10, 0b001, 0b0000, 0b1010, 0b000), // RW
    kTRCSEQEVR0           = encode(0b10, 0b001, 0b0000, 0b0000, 0b100), // RW
    kTRCSEQEVR1           = encode(0b10, 0b001, 0b0000, 0b0001, 0b100), // RW
    kTRCSEQEVR2           = encode(0b10, 0b001, 0b0000, 0b0010, 0b100), // RW
    kTRCSEQRSTEVR         = encode(0b10, 0b001, 0b0000, 0b0110, 0b100), // RW
    kTRCSEQSTR            = encode(0b10, 0b001, 0b0000, 0b0111, 0b100), // RW
    kTRCSSCCR0            = encode(0b10, 0b001, 0b0001, 0b0000, 0b010), // RW
    kTRCSSCCR1            = encode(0b10, 0b001, 0b0001, 0b0001, 0b010), // RW
    kTRCSSCCR2            = encode(0b10, 0b001, 0b0001, 0b0010, 0b010), // RW
    kTRCSSCCR3            = encode(0b10, 0b001, 0b0001, 0b0011, 0b010), // RW
    kTRCSSCCR4            = encode(0b10, 0b001, 0b0001, 0b0100, 0b010), // RW
    kTRCSSCCR5            = encode(0b10, 0b001, 0b0001, 0b0101, 0b010), // RW
    kTRCSSCCR6            = encode(0b10, 0b001, 0b0001, 0b0110, 0b010), // RW
    kTRCSSCCR7            = encode(0b10, 0b001, 0b0001, 0b0111, 0b010), // RW
    kTRCSSCSR0            = encode(0b10, 0b001, 0b0001, 0b1000, 0b010), // RW
    kTRCSSCSR1            = encode(0b10, 0b001, 0b0001, 0b1001, 0b010), // RW
    kTRCSSCSR2            = encode(0b10, 0b001, 0b0001, 0b1010, 0b010), // RW
    kTRCSSCSR3            = encode(0b10, 0b001, 0b0001, 0b1011, 0b010), // RW
    kTRCSSCSR4            = encode(0b10, 0b001, 0b0001, 0b1100, 0b010), // RW
    kTRCSSCSR5            = encode(0b10, 0b001, 0b0001, 0b1101, 0b010), // RW
    kTRCSSCSR6            = encode(0b10, 0b001, 0b0001, 0b1110, 0b010), // RW
    kTRCSSCSR7            = encode(0b10, 0b001, 0b0001, 0b1111, 0b010), // RW
    kTRCSSPCICR0          = encode(0b10, 0b001, 0b0001, 0b0000, 0b011), // RW
    kTRCSSPCICR1          = encode(0b10, 0b001, 0b0001, 0b0001, 0b011), // RW
    kTRCSSPCICR2          = encode(0b10, 0b001, 0b0001, 0b0010, 0b011), // RW
    kTRCSSPCICR3          = encode(0b10, 0b001, 0b0001, 0b0011, 0b011), // RW
    kTRCSSPCICR4          = encode(0b10, 0b001, 0b0001, 0b0100, 0b011), // RW
    kTRCSSPCICR5          = encode(0b10, 0b001, 0b0001, 0b0101, 0b011), // RW
    kTRCSSPCICR6          = encode(0b10, 0b001, 0b0001, 0b0110, 0b011), // RW
    kTRCSSPCICR7          = encode(0b10, 0b001, 0b0001, 0b0111, 0b011), // RW
    kTRCSTALLCTLR         = encode(0b10, 0b001, 0b0000, 0b1011, 0b000), // RW
    kTRCSTATR             = encode(0b10, 0b001, 0b0000, 0b0011, 0b000), // RO
    kTRCSYNCPR            = encode(0b10, 0b001, 0b0000, 0b1101, 0b000), // RW
    kTRCTRACEIDR          = encode(0b10, 0b001, 0b0000, 0b0000, 0b001), // RW
    kTRCTSCTLR            = encode(0b10, 0b001, 0b0000, 0b1100, 0b000), // RW
    kTRCVDARCCTLR         = encode(0b10, 0b001, 0b0000, 0b1010, 0b010), // RW
    kTRCVDCTLR            = encode(0b10, 0b001, 0b0000, 0b1000, 0b010), // RW
    kTRCVDSACCTLR         = encode(0b10, 0b001, 0b0000, 0b1001, 0b010), // RW
    kTRCVICTLR            = encode(0b10, 0b001, 0b0000, 0b0000, 0b010), // RW
    kTRCVIIECTLR          = encode(0b10, 0b001, 0b0000, 0b0001, 0b010), // RW
    kTRCVIPCSSCTLR        = encode(0b10, 0b001, 0b0000, 0b0011, 0b010), // RW
    kTRCVISSCTLR          = encode(0b10, 0b001, 0b0000, 0b0010, 0b010), // RW
    kTRCVMIDCCTLR0        = encode(0b10, 0b001, 0b0011, 0b0010, 0b010), // RW
    kTRCVMIDCCTLR1        = encode(0b10, 0b001, 0b0011, 0b0011, 0b010), // RW
    kTRCVMIDCVR0          = encode(0b10, 0b001, 0b0011, 0b0000, 0b001), // RW
    kTRCVMIDCVR1          = encode(0b10, 0b001, 0b0011, 0b0010, 0b001), // RW
    kTRCVMIDCVR2          = encode(0b10, 0b001, 0b0011, 0b0100, 0b001), // RW
    kTRCVMIDCVR3          = encode(0b10, 0b001, 0b0011, 0b0110, 0b001), // RW
    kTRCVMIDCVR4          = encode(0b10, 0b001, 0b0011, 0b1000, 0b001), // RW
    kTRCVMIDCVR5          = encode(0b10, 0b001, 0b0011, 0b1010, 0b001), // RW
    kTRCVMIDCVR6          = encode(0b10, 0b001, 0b0011, 0b1100, 0b001), // RW
    kTRCVMIDCVR7          = encode(0b10, 0b001, 0b0011, 0b1110, 0b001), // RW
    kTRFCR_EL1            = encode(0b11, 0b000, 0b0001, 0b0010, 0b001), // RW
    kTRFCR_EL12           = encode(0b11, 0b101, 0b0001, 0b0010, 0b001), // RW
    kTRFCR_EL2            = encode(0b11, 0b100, 0b0001, 0b0010, 0b001), // RW
    kTTBR0_EL1            = encode(0b11, 0b000, 0b0010, 0b0000, 0b000), // RW
    kTTBR0_EL12           = encode(0b11, 0b101, 0b0010, 0b0000, 0b000), // RW
    kTTBR0_EL2            = encode(0b11, 0b100, 0b0010, 0b0000, 0b000), // RW
    kTTBR0_EL3            = encode(0b11, 0b110, 0b0010, 0b0000, 0b000), // RW
    kTTBR1_EL1            = encode(0b11, 0b000, 0b0010, 0b0000, 0b001), // RW
    kTTBR1_EL12           = encode(0b11, 0b101, 0b0010, 0b0000, 0b001), // RW
    kTTBR1_EL2            = encode(0b11, 0b100, 0b0010, 0b0000, 0b001), // RW
    kUAO                  = encode(0b11, 0b000, 0b0100, 0b0010, 0b100), // RW
    kVBAR_EL1             = encode(0b11, 0b000, 0b1100, 0b0000, 0b000), // RW
    kVBAR_EL12            = encode(0b11, 0b101, 0b1100, 0b0000, 0b000), // RW
    kVBAR_EL2             = encode(0b11, 0b100, 0b1100, 0b0000, 0b000), // RW
    kVBAR_EL3             = encode(0b11, 0b110, 0b1100, 0b0000, 0b000), // RW
    kVDISR_EL2            = encode(0b11, 0b100, 0b1100, 0b0001, 0b001), // RW
    kVMPIDR_EL2           = encode(0b11, 0b100, 0b0000, 0b0000, 0b101), // RW
    kVNCR_EL2             = encode(0b11, 0b100, 0b0010, 0b0010, 0b000), // RW
    kVPIDR_EL2            = encode(0b11, 0b100, 0b0000, 0b0000, 0b000), // RW
    kVSESR_EL2            = encode(0b11, 0b100, 0b0101, 0b0010, 0b011), // RW
    kVSTCR_EL2            = encode(0b11, 0b100, 0b0010, 0b0110, 0b010), // RW
    kVSTTBR_EL2           = encode(0b11, 0b100, 0b0010, 0b0110, 0b000), // RW
    kVTCR_EL2             = encode(0b11, 0b100, 0b0010, 0b0001, 0b010), // RW
    kVTTBR_EL2            = encode(0b11, 0b100, 0b0010, 0b0001, 0b000), // RW
    kZCR_EL1              = encode(0b11, 0b000, 0b0001, 0b0010, 0b000), // RW
    kZCR_EL12             = encode(0b11, 0b101, 0b0001, 0b0010, 0b000), // RW
    kZCR_EL2              = encode(0b11, 0b100, 0b0001, 0b0010, 0b000), // RW
    kZCR_EL3              = encode(0b11, 0b110, 0b0001, 0b0010, 0b000)  // RW
  };
};

} // {Predicate}

//! \}

ASMJIT_END_SUB_NAMESPACE

#endif // ASMJIT_LA_A64GLOBALS_H_INCLUDED
