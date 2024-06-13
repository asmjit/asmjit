// This file is part of AsmJit project <https://asmjit.com>
//
// See asmjit.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include "../core/api-build_p.h"
#if !defined(ASMJIT_NO_LOONGARCH64)

#include "../core/codeholder.h"
#include "../core/support.h"
#include "../loongarch/la64instdb_p.h"
#include "../loongarch/la64operand.h"

ASMJIT_BEGIN_SUB_NAMESPACE(la64)

namespace InstDB {

// a64::InstDB - InstInfoTable
// ===========================

// Defines an Loongarch instruction.
#define INST(id, opcodeEncoding, opcodeData, rwInfoIndex, flags, opcodeDataIndex) { \
  uint32_t(kEncoding##opcodeEncoding),      \
  uint32_t(opcodeDataIndex),                \
  0,                                        \
  uint16_t(rwInfoIndex),                    \
  uint16_t(flags)                           \
}

#define F(flag) kInstFlag##flag

const InstInfo _instInfoTable[] = {
  // +------------------+---------------------+--------------------------------------------------------------------------------------+-----------+---------------------------+----+
  // | Instruction Id   | Encoding            | Opcode Data                                                                          | RW Info   | Instruction Flags         |DatX|
  // +------------------+---------------------+--------------------------------------------------------------------------------------+-----------+---------------------------+----+
  // ${InstInfo:Begin}
  INST(None             , None               , (_)                                                                                   , 0         , 0                         , 0  ), // #0
  INST(Add_d            , BaseLRRR           , (0b00000000000100001, kWX, 0, kWX, 5, kWX, 10, 0)                                     , 0         , 0                         , 0  ), // #1
  INST(Add_w            , BaseLRRR           , (0b00000000000100000, kWX, 0, kWX, 5, kWX, 10, 0)                                     , 0         , 0                         , 1  ), // #2
  INST(Addi_d           , BaseLRRI           , (0b0000001011, kX, 0, kSP, 5, 10, 0)                                                  , 0         , 0                         , 0  ), // #3
  INST(Addi_w           , BaseLRRI           , (0b0000001010, kX, 0, kSP, 5, 10, 0)                                                  , 0         , 0                         , 1  ), // #4
  INST(Addu16i_d        , BaseLRRIL          , (0b000100, kX, kSP, 10)                                                               , 0         , 0                         , 0  ), // #5
  INST(Alsl_d           , BaseLRRRI          , (0b000000000010110, kWX, 0, kWX, 5, kWX, 10, 15, 0)                                   , 0         , 0                         , 0  ), // #6
  INST(Alsl_w           , BaseLRRRI          , (0b000000000000010, kWX, 0, kWX, 5, kWX, 10, 15, 0)                                   , 0         , 0                         , 1  ), // #7
  INST(Alsl_wu          , BaseLRRRI          , (0b000000000000011, kWX, 0, kWX, 5, kWX, 10, 15, 0)                                   , 0         , 0                         , 2  ), // #8
  INST(Amadd_d          , BaseLRRRT          , (0b00111000011000011, kWX, 0, kWX, 10, kWX, 5, 0)                                     , 0         , 0                         , 0  ), // #9
  INST(Amadd_db_d       , BaseLRRRT          , (0b00111000011010101, kWX, 0, kWX, 10, kWX, 5, 0)                                     , 0         , 0                         , 1  ), // #10
  INST(Amadd_db_w       , BaseLRRRT          , (0b00111000011010100, kWX, 0, kWX, 10, kWX, 5, 0)                                     , 0         , 0                         , 2  ), // #11
  INST(Amadd_w          , BaseLRRRT          , (0b00111000011000010, kWX, 0, kWX, 10, kWX, 5, 0)                                     , 0         , 0                         , 3  ), // #12
  INST(Amand_d          , BaseLRRRT          , (0b00111000011000101, kWX, 0, kWX, 10, kWX, 5, 0)                                     , 0         , 0                         , 4  ), // #13
  INST(Amand_db_d       , BaseLRRRT          , (0b00111000011010111, kWX, 0, kWX, 10, kWX, 5, 0)                                     , 0         , 0                         , 5  ), // #14
  INST(Amand_db_w       , BaseLRRRT          , (0b00111000011010110, kWX, 0, kWX, 10, kWX, 5, 0)                                     , 0         , 0                         , 6  ), // #15
  INST(Amand_w          , BaseLRRRT          , (0b00111000011000100, kWX, 0, kWX, 10, kWX, 5, 0)                                     , 0         , 0                         , 7  ), // #16
  INST(Ammax_d          , BaseLRRRT          , (0b00111000011001011, kWX, 0, kWX, 10, kWX, 5, 0)                                     , 0         , 0                         , 8  ), // #17
  INST(Ammax_db_d       , BaseLRRRT          , (0b00111000011011101, kWX, 0, kWX, 10, kWX, 5, 0)                                     , 0         , 0                         , 9  ), // #18
  INST(Ammax_db_du      , BaseLRRRT          , (0b00111000011100001, kWX, 0, kWX, 10, kWX, 5, 0)                                     , 0         , 0                         , 10 ), // #19
  INST(Ammax_db_w       , BaseLRRRT          , (0b00111000011011100, kWX, 0, kWX, 10, kWX, 5, 0)                                     , 0         , 0                         , 11 ), // #20
  INST(Ammax_db_wu      , BaseLRRRT          , (0b00111000011100000, kWX, 0, kWX, 10, kWX, 5, 0)                                     , 0         , 0                         , 12 ), // #21
  INST(Ammax_du         , BaseLRRRT          , (0b00111000011001111, kWX, 0, kWX, 10, kWX, 5, 0)                                     , 0         , 0                         , 13 ), // #22
  INST(Ammax_w          , BaseLRRRT          , (0b00111000011001010, kWX, 0, kWX, 10, kWX, 5, 0)                                     , 0         , 0                         , 14 ), // #23
  INST(Ammax_wu         , BaseLRRRT          , (0b00111000011001110, kWX, 0, kWX, 10, kWX, 5, 0)                                     , 0         , 0                         , 15 ), // #24
  INST(Ammin_d          , BaseLRRRT          , (0b00111000011001101, kWX, 0, kWX, 10, kWX, 5, 0)                                     , 0         , 0                         , 16 ), // #25
  INST(Ammin_db_d       , BaseLRRRT          , (0b00111000011011111, kWX, 0, kWX, 10, kWX, 5, 0)                                     , 0         , 0                         , 17 ), // #26
  INST(Ammin_db_du      , BaseLRRRT          , (0b00111000011100011, kWX, 0, kWX, 10, kWX, 5, 0)                                     , 0         , 0                         , 18 ), // #27
  INST(Ammin_db_w       , BaseLRRRT          , (0b00111000011011110, kWX, 0, kWX, 10, kWX, 5, 0)                                     , 0         , 0                         , 19 ), // #28
  INST(Ammin_db_wu      , BaseLRRRT          , (0b00111000011100010, kWX, 0, kWX, 10, kWX, 5, 0)                                     , 0         , 0                         , 20 ), // #29
  INST(Ammin_du         , BaseLRRRT          , (0b00111000011010001, kWX, 0, kWX, 10, kWX, 5, 0)                                     , 0         , 0                         , 21 ), // #30
  INST(Ammin_w          , BaseLRRRT          , (0b00111000011001100, kWX, 0, kWX, 10, kWX, 5, 0)                                     , 0         , 0                         , 22 ), // #31
  INST(Ammin_wu         , BaseLRRRT          , (0b00111000011010000, kWX, 0, kWX, 10, kWX, 5, 0)                                     , 0         , 0                         , 23 ), // #32
  INST(Amor_d           , BaseLRRRT          , (0b00111000011000111, kWX, 0, kWX, 10, kWX, 5, 0)                                     , 0         , 0                         , 24 ), // #33
  INST(Amor_db_d        , BaseLRRRT          , (0b00111000011011001, kWX, 0, kWX, 10, kWX, 5, 0)                                     , 0         , 0                         , 25 ), // #34
  INST(Amor_db_w        , BaseLRRRT          , (0b00111000011011000, kWX, 0, kWX, 10, kWX, 5, 0)                                     , 0         , 0                         , 26 ), // #35
  INST(Amor_w           , BaseLRRRT          , (0b00111000011000110, kWX, 0, kWX, 10, kWX, 5, 0)                                     , 0         , 0                         , 27 ), // #36
  INST(Amswap_d         , BaseLRRRT          , (0b00111000011000001, kWX, 0, kWX, 10, kWX, 5, 0)                                     , 0         , 0                         , 28 ), // #37
  INST(Amswap_db_d      , BaseLRRRT          , (0b00111000011010011, kWX, 0, kWX, 10, kWX, 5, 0)                                     , 0         , 0                         , 29 ), // #38
  INST(Amswap_db_w      , BaseLRRRT          , (0b00111000011010010, kWX, 0, kWX, 10, kWX, 5, 0)                                     , 0         , 0                         , 30 ), // #39
  INST(Amswap_w         , BaseLRRRT          , (0b00111000011000000, kWX, 0, kWX, 10, kWX, 5, 0)                                     , 0         , 0                         , 31 ), // #40
  INST(Amxor_d          , BaseLRRRT          , (0b00111000011001001, kWX, 0, kWX, 10, kWX, 5, 0)                                     , 0         , 0                         , 32 ), // #41
  INST(Amxor_db_d       , BaseLRRRT          , (0b00111000011011011, kWX, 0, kWX, 10, kWX, 5, 0)                                     , 0         , 0                         , 33 ), // #42
  INST(Amxor_db_w       , BaseLRRRT          , (0b00111000011011010, kWX, 0, kWX, 10, kWX, 5, 0)                                     , 0         , 0                         , 34 ), // #43
  INST(Amxor_w          , BaseLRRRT          , (0b00111000011001000, kWX, 0, kWX, 10, kWX, 5, 0)                                     , 0         , 0                         , 35 ), // #44
  INST(And_             , BaseLRRR           , (0b00000000000101001, kWX, 0, kWX, 5, kWX, 10, 0)                                     , 0         , 0                         , 2  ), // #45
  INST(Andi             , BaseLRRI           , (0b0000001101, kX, 0, kSP, 5, 10, 5)                                                  , 0         , 0                         , 2  ), // #46
  INST(Andn             , BaseLRRR           , (0b00000000000101101, kWX, 0, kWX, 5, kWX, 10, 0)                                     , 0         , 0                         , 3  ), // #47
  INST(Asrtgt_d         , BaseLRRL           , (0b00000000000000011, kWX, 5, kWX, 10)                                                , 0         , 0                         , 0  ), // #48
  INST(Asrtle_d         , BaseLRRL           , (0b00000000000000010, kWX, 5, kWX, 10)                                                , 0         , 0                         , 1  ), // #49
  INST(B                , BaseBranchRel      , (0b010100, OffsetType::kTypeLa64_B26)                                                 , 0         , 0                         , 0  ), // #50
  INST(Bceqz            , BaseLII            , (0b010010, 5, 0, 3, 5, 16, 10, 0)                                                     , 0         , 0                         , 0  ), // #51
  INST(Bcnez            , BaseLII            , (0b010010, 5, 0, 3, 5, 16, 10, 1)                                                     , 0         , 0                         , 1  ), // #52
  INST(Beq              , BaseBranchRel      , (0b010110, OffsetType::kTypeLa64_B16)                                                 , 0         , 0                         , 1  ), // #53
  INST(Bge              , BaseBranchRel      , (0b011001, OffsetType::kTypeLa64_B16)                                                 , 0         , 0                         , 2  ), // #54
  INST(Bgeu             , BaseBranchRel      , (0b011011, OffsetType::kTypeLa64_B16)                                                 , 0         , 0                         , 3  ), // #55
  INST(Bitrev_4b        , BaseLRR            , (0b0000000000000000010010, kWX, 0, kWX, 5, 0)                                         , 0         , 0                         , 0  ), // #56
  INST(Bitrev_8b        , BaseLRR            , (0b0000000000000000010011, kWX, 0, kWX, 5, 0)                                         , 0         , 0                         , 1  ), // #57
  INST(Bitrev_d         , BaseLRR            , (0b0000000000000000010101, kWX, 0, kWX, 5, 0)                                         , 0         , 0                         , 2  ), // #58
  INST(Bitrev_w         , BaseLRR            , (0b0000000000000000010100, kWX, 0, kWX, 5, 0)                                         , 0         , 0                         , 3  ), // #59
  INST(Bl               , BaseBranchRel      , (0b010101, OffsetType::kTypeLa64_B26)                                                 , 0         , 0                         , 4  ), // #60
  INST(Blt              , BaseBranchRel      , (0b011000, OffsetType::kTypeLa64_B16)                                                 , 0         , 0                         , 5  ), // #61
  INST(Bltu             , BaseBranchRel      , (0b011010, OffsetType::kTypeLa64_B16)                                                 , 0         , 0                         , 6  ), // #62
  INST(Bne              , BaseBranchRel      , (0b010111, OffsetType::kTypeLa64_B16)                                                 , 0         , 0                         , 7  ), // #63
  INST(Break_           , BaseLIC            , (0b00000000001010100, 0)                                                              , 0         , 0                         , 0  ), // #64
  INST(Bstrins_d        , BaseLRRII          , (0b0000000010, kWX, 0, kWX, 5, 10, 16, 2)                                             , 0         , 0                         , 0  ), // #65
  INST(Bstrins_w        , BaseLRRII          , (0b00000000011, kWX, 0, kWX, 5, 10, 16, 0)                                            , 0         , 0                         , 1  ), // #66
  INST(Bstrpick_d       , BaseLRRII          , (0b0000000011, kWX, 0, kWX, 5, 10, 16, 2)                                             , 0         , 0                         , 2  ), // #67
  INST(Bstrpick_w       , BaseLRRII          , (0b00000000011, kWX, 0, kWX, 5, 10, 16, 1)                                            , 0         , 0                         , 3  ), // #68
  INST(Bytepick_d       , BaseLRRRI          , (0b00000000000011, kWX, 0, kWX, 5, kWX, 10, 15, 2)                                    , 0         , 0                         , 3  ), // #69
  INST(Bytepick_w       , BaseLRRRI          , (0b000000000000100, kWX, 0, kWX, 5, kWX, 10, 15, 1)                                   , 0         , 0                         , 4  ), // #70
  INST(Cacop            , LCldst             , (0b0000011000)                                                                        , 0         , 0                         , 0  ), // #71
  INST(Clo_d            , BaseLRR            , (0b0000000000000000001000, kWX, 0, kWX, 5, 0)                                         , 0         , 0                         , 4  ), // #72
  INST(Clo_w            , BaseLRR            , (0b0000000000000000000100, kWX, 0, kWX, 5, 0)                                         , 0         , 0                         , 5  ), // #73
  INST(Clz_d            , BaseLRR            , (0b0000000000000000001001, kWX, 0, kWX, 5, 0)                                         , 0         , 0                         , 6  ), // #74
  INST(Clz_w            , BaseLRR            , (0b0000000000000000000101, kWX, 0, kWX, 5, 0)                                         , 0         , 0                         , 7  ), // #75
  INST(Cpucfg           , BaseLRR            , (0b0000000000000000011011, kWX, 0, kWX, 5, 0)                                         , 0         , 0                         , 8  ), // #76
  INST(Crc_w_b_w        , BaseLRRR           , (0b00000000001001000, kWX, 0, kWX, 5, kWX, 10, 0)                                     , 0         , 0                         , 4  ), // #77
  INST(Crc_w_d_w        , BaseLRRR           , (0b00000000001001011, kWX, 0, kWX, 5, kWX, 10, 0)                                     , 0         , 0                         , 5  ), // #78
  INST(Crc_w_h_w        , BaseLRRR           , (0b00000000001001001, kWX, 0, kWX, 5, kWX, 10, 0)                                     , 0         , 0                         , 6  ), // #79
  INST(Crc_w_w_w        , BaseLRRR           , (0b00000000001001010, kWX, 0, kWX, 5, kWX, 10, 0)                                     , 0         , 0                         , 7  ), // #80
  INST(Crcc_w_b_w       , BaseLRRR           , (0b00000000001001100, kWX, 0, kWX, 5, kWX, 10, 0)                                     , 0         , 0                         , 8  ), // #81
  INST(Crcc_w_d_w       , BaseLRRR           , (0b00000000001001111, kWX, 0, kWX, 5, kWX, 10, 0)                                     , 0         , 0                         , 9  ), // #82
  INST(Crcc_w_h_w       , BaseLRRR           , (0b00000000001001101, kWX, 0, kWX, 5, kWX, 10, 0)                                     , 0         , 0                         , 10 ), // #83
  INST(Crcc_w_w_w       , BaseLRRR           , (0b00000000001001110, kWX, 0, kWX, 5, kWX, 10, 0)                                     , 0         , 0                         , 11 ), // #84
  INST(Csrrd            , BaseLRI            , (0b00000100, kWX, 0, 10, 2)                                                           , 0         , 0                         , 0  ), // #85
  INST(Csrwr            , BaseLRI            , (0b00000100, kWX, 0, 10, 3)                                                           , 0         , 0                         , 1  ), // #86
  INST(Csrxchg          , BaseLRRI           , (0b00000100, kWX, 0, kWX, 5, 10, 3)                                                   , 0         , 0                         , 3  ), // #87
  INST(Cto_d            , BaseLRR            , (0b0000000000000000001010, kWX, 0, kWX, 5, 0)                                         , 0         , 0                         , 9  ), // #88
  INST(Cto_w            , BaseLRR            , (0b0000000000000000000110, kWX, 0, kWX, 5, 0)                                         , 0         , 0                         , 10 ), // #89
  INST(Ctz_d            , BaseLRR            , (0b0000000000000000001011, kWX, 0, kWX, 5, 0)                                         , 0         , 0                         , 11 ), // #90
  INST(Ctz_w            , BaseLRR            , (0b0000000000000000000111, kWX, 0, kWX, 5, 0)                                         , 0         , 0                         , 12 ), // #91
  INST(Dbar             , BaseOpImm          , (0b00111000011100100, 15, 0)                                                          , 0         , 0                         , 0  ), // #92
  INST(Dbcl             , BaseLIC            , (0b00000000001010101, 0)                                                              , 0         , 0                         , 1  ), // #93
  INST(Div_d            , BaseLRRR           , (0b00000000001000100, kWX, 0, kWX, 5, kWX, 10, 0)                                     , 0         , 0                         , 12 ), // #94
  INST(Div_du           , BaseLRRR           , (0b00000000001000110, kWX, 0, kWX, 5, kWX, 10, 0)                                     , 0         , 0                         , 13 ), // #95
  INST(Div_w            , BaseLRRR           , (0b00000000001000000, kWX, 0, kWX, 5, kWX, 10, 0)                                     , 0         , 0                         , 14 ), // #96
  INST(Div_wu           , BaseLRRR           , (0b00000000001000010, kWX, 0, kWX, 5, kWX, 10, 0)                                     , 0         , 0                         , 15 ), // #97
  INST(Ertn             , BaseOp             , (0b00000110010010000011100000000000)                                                  , 0         , 0                         , 0  ), // #98
  INST(Ext_w_b          , BaseLRR            , (0b0000000000000000010111, kWX, 0, kWX, 5, 0)                                         , 0         , 0                         , 13 ), // #99
  INST(Ext_w_h          , BaseLRR            , (0b0000000000000000010110, kWX, 0, kWX, 5, 0)                                         , 0         , 0                         , 14 ), // #100
  INST(Fabs_d           , BaseLVV            , (0b0000000100010100000010, 0, 5, 0)                                                   , 0         , 0                         , 0  ), // #101
  INST(Fabs_s           , BaseLVV            , (0b0000000100010100000001, 0, 5, 0)                                                   , 0         , 0                         , 1  ), // #102
  INST(Fadd_d           , BaseLVVV           , (0b00000001000000010, 0, 5, 10, 0)                                                    , 0         , 0                         , 0  ), // #103
  INST(Fadd_s           , BaseLVVV           , (0b00000001000000001, 0, 5, 10, 0)                                                    , 0         , 0                         , 1  ), // #104
  INST(Fclass_d         , BaseLVV            , (0b0000000100010100001110, 0, 5, 0)                                                   , 0         , 0                         , 2  ), // #105
  INST(Fclass_s         , BaseLVV            , (0b0000000100010100001101, 0, 5, 0)                                                   , 0         , 0                         , 3  ), // #106
  INST(Fcmp_caf_s       , BaseLFIVV          , (0b00001100000100000, 0, 5, 10, 0)                                                    , 0         , 0                         , 0  ), // #107
  INST(Fcmp_ceq_s       , BaseLFIVV          , (0b00001100000100100, 0, 5, 10, 0)                                                    , 0         , 0                         , 1  ), // #108
  INST(Fcmp_cle_d       , BaseLFIVV          , (0b00001100001000110, 0, 5, 10, 0)                                                    , 0         , 0                         , 2  ), // #109
  INST(Fcmp_clt_d       , BaseLFIVV          , (0b00001100001000010, 0, 5, 10, 0)                                                    , 0         , 0                         , 3  ), // #110
  INST(Fcmp_cne_d       , BaseLFIVV          , (0b00001100001010000, 0, 5, 10, 0)                                                    , 0         , 0                         , 4  ), // #111
  INST(Fcmp_cor_s       , BaseLFIVV          , (0b00001100000110100, 0, 5, 10, 0)                                                    , 0         , 0                         , 5  ), // #112
  INST(Fcmp_cueq_s      , BaseLFIVV          , (0b00001100000101100, 0, 5, 10, 0)                                                    , 0         , 0                         , 6  ), // #113
  INST(Fcmp_cule_s      , BaseLFIVV          , (0b00001100000101110, 0, 5, 10, 0)                                                    , 0         , 0                         , 7  ), // #114
  INST(Fcmp_cult_s      , BaseLFIVV          , (0b00001100000101010, 0, 5, 10, 0)                                                    , 0         , 0                         , 8  ), // #115
  INST(Fcmp_cun_d       , BaseLFIVV          , (0b00001100001001000, 0, 5, 10, 0)                                                    , 0         , 0                         , 9  ), // #116
  INST(Fcmp_cune_d      , BaseLFIVV          , (0b00001100001011000, 0, 5, 10, 0)                                                    , 0         , 0                         , 10 ), // #117
  INST(Fcmp_saf_s       , BaseLFIVV          , (0b00001100000100001, 0, 5, 10, 0)                                                    , 0         , 0                         , 11 ), // #118
  INST(Fcmp_seq_s       , BaseLFIVV          , (0b00001100000100101, 0, 5, 10, 0)                                                    , 0         , 0                         , 12 ), // #119
  INST(Fcmp_sle_s       , BaseLFIVV          , (0b00001100000100111, 0, 5, 10, 0)                                                    , 0         , 0                         , 13 ), // #120
  INST(Fcmp_slt_s       , BaseLFIVV          , (0b00001100000100011, 0, 5, 10, 0)                                                    , 0         , 0                         , 14 ), // #121
  INST(Fcmp_sne_s       , BaseLFIVV          , (0b00001100000110001, 0, 5, 10, 0)                                                    , 0         , 0                         , 15 ), // #122
  INST(Fcmp_sor_d       , BaseLFIVV          , (0b00001100001010101, 0, 5, 10, 0)                                                    , 0         , 0                         , 16 ), // #123
  INST(Fcmp_sueq_d      , BaseLFIVV          , (0b00001100001001101, 0, 5, 10, 0)                                                    , 0         , 0                         , 17 ), // #124
  INST(Fcmp_sule_d      , BaseLFIVV          , (0b00001100001001111, 0, 5, 10, 0)                                                    , 0         , 0                         , 18 ), // #125
  INST(Fcmp_sult_d      , BaseLFIVV          , (0b00001100001001011, 0, 5, 10, 0)                                                    , 0         , 0                         , 19 ), // #126
  INST(Fcmp_sun_d       , BaseLFIVV          , (0b00001100001001001, 0, 5, 10, 0)                                                    , 0         , 0                         , 20 ), // #127
  INST(Fcmp_sune_s      , BaseLFIVV          , (0b00001100000111001, 0, 5, 10, 0)                                                    , 0         , 0                         , 21 ), // #128
  INST(Fcopysign_d      , BaseLVVV           , (0b00000001000100110, 0, 5, 10, 0)                                                    , 0         , 0                         , 2  ), // #129
  INST(Fcopysign_s      , BaseLVVV           , (0b00000001000100101, 0, 5, 10, 0)                                                    , 0         , 0                         , 3  ), // #130
  INST(Fcvt_d_s         , BaseLVV            , (0b0000000100011001001001, 0, 5, 0)                                                   , 0         , 0                         , 4  ), // #131
  INST(Fcvt_s_d         , BaseLVV            , (0b0000000100011001000110, 0, 5, 0)                                                   , 0         , 0                         , 5  ), // #132
  INST(Fdiv_d           , BaseLVVV           , (0b00000001000001110, 0, 5, 10, 0)                                                    , 0         , 0                         , 4  ), // #133
  INST(Fdiv_s           , BaseLVVV           , (0b00000001000001101, 0, 5, 10, 0)                                                    , 0         , 0                         , 5  ), // #134
  INST(Ffint_d_l        , BaseLVV            , (0b0000000100011101001010, 0, 5, 0)                                                   , 0         , 0                         , 6  ), // #135
  INST(Ffint_d_w        , BaseLVV            , (0b0000000100011101001000, 0, 5, 0)                                                   , 0         , 0                         , 7  ), // #136
  INST(Ffint_s_l        , BaseLVV            , (0b0000000100011101000110, 0, 5, 0)                                                   , 0         , 0                         , 8  ), // #137
  INST(Ffint_s_w        , BaseLVV            , (0b0000000100011101000100, 0, 5, 0)                                                   , 0         , 0                         , 9  ), // #138
  INST(Fld_d            , BaseLdSt           , (0b0010101110, 22)                                                                    , 0         , 0                         , 0  ), // #139
  INST(Fld_s            , BaseLdSt           , (0b0010101100, 22)                                                                    , 0         , 0                         , 1  ), // #140
  INST(Fldgt_d          , BaseLVRR           , (0b00111000011101001, 0, kWX, 5, kWX, 10, 0)                                          , 0         , 0                         , 0  ), // #141
  INST(Fldgt_s          , BaseLVRR           , (0b00111000011101000, 0, kWX, 5, kWX, 10, 0)                                          , 0         , 0                         , 1  ), // #142
  INST(Fldle_d          , BaseLVRR           , (0b00111000011101011, 0, kWX, 5, kWX, 10, 0)                                          , 0         , 0                         , 2  ), // #143
  INST(Fldle_s          , BaseLVRR           , (0b00111000011101010, 0, kWX, 5, kWX, 10, 0)                                          , 0         , 0                         , 3  ), // #144
  INST(Fldx_d           , BaseLdSt           , (0b00111000001101000, 15)                                                             , 0         , 0                         , 2  ), // #145
  INST(Fldx_s           , BaseLdSt           , (0b00111000001100000, 15)                                                             , 0         , 0                         , 3  ), // #146
  INST(Flogb_d          , BaseLVV            , (0b0000000100010100001010, 0, 5, 0)                                                   , 0         , 0                         , 10 ), // #147
  INST(Flogb_s          , BaseLVV            , (0b0000000100010100001001, 0, 5, 0)                                                   , 0         , 0                         , 11 ), // #148
  INST(Fmadd_d          , FpLVVVV            , (0b000010000010)                                                                      , 0         , 0                         , 0  ), // #149
  INST(Fmadd_s          , FpLVVVV            , (0b000010000001)                                                                      , 0         , 0                         , 1  ), // #150
  INST(Fmax_d           , BaseLVVV           , (0b00000001000010010, 0, 5, 10, 0)                                                    , 0         , 0                         , 6  ), // #151
  INST(Fmax_s           , BaseLVVV           , (0b00000001000010001, 0, 5, 10, 0)                                                    , 0         , 0                         , 7  ), // #152
  INST(Fmaxa_d          , BaseLVVV           , (0b00000001000011010, 0, 5, 10, 0)                                                    , 0         , 0                         , 8  ), // #153
  INST(Fmaxa_s          , BaseLVVV           , (0b00000001000011001, 0, 5, 10, 0)                                                    , 0         , 0                         , 9  ), // #154
  INST(Fmin_d           , BaseLVVV           , (0b00000001000010110, 0, 5, 10, 0)                                                    , 0         , 0                         , 10 ), // #155
  INST(Fmin_s           , BaseLVVV           , (0b00000001000010101, 0, 5, 10, 0)                                                    , 0         , 0                         , 11 ), // #156
  INST(Fmina_d          , BaseLVVV           , (0b00000001000011110, 0, 5, 10, 0)                                                    , 0         , 0                         , 12 ), // #157
  INST(Fmina_s          , BaseLVVV           , (0b00000001000011101, 0, 5, 10, 0)                                                    , 0         , 0                         , 13 ), // #158
  INST(Fmov_d           , BaseLVV            , (0b0000000100010100100110, 0, 5, 0)                                                   , 0         , 0                         , 12 ), // #159
  INST(Fmov_s           , BaseLVV            , (0b0000000100010100100101, 0, 5, 0)                                                   , 0         , 0                         , 13 ), // #160
  INST(Fmul_d           , BaseLVVV           , (0b00000001000001010, 0, 5, 10, 0)                                                    , 0         , 0                         , 14 ), // #161
  INST(Fmul_s           , BaseLVVV           , (0b00000001000001001, 0, 5, 10, 0)                                                    , 0         , 0                         , 15 ), // #162
  INST(Fneg_d           , BaseLVV            , (0b0000000100010100000110, 0, 5, 0)                                                   , 0         , 0                         , 14 ), // #163
  INST(Fneg_s           , BaseLVV            , (0b0000000100010100000101, 0, 5, 0)                                                   , 0         , 0                         , 15 ), // #164
  INST(Frecip_d         , BaseLVV            , (0b0000000100010100010110, 0, 5, 0)                                                   , 0         , 0                         , 16 ), // #165
  INST(Frecip_s         , BaseLVV            , (0b0000000100010100010101, 0, 5, 0)                                                   , 0         , 0                         , 17 ), // #166
  INST(Frint_d          , BaseLVV            , (0b0000000100011110010010, 0, 5, 0)                                                   , 0         , 0                         , 18 ), // #167
  INST(Frint_s          , BaseLVV            , (0b0000000100011110010001, 0, 5, 0)                                                   , 0         , 0                         , 19 ), // #168
  INST(Frsqrt_d         , BaseLVV            , (0b0000000100010100011010, 0, 5, 0)                                                   , 0         , 0                         , 20 ), // #169
  INST(Frsqrt_s         , BaseLVV            , (0b0000000100010100011001, 0, 5, 0)                                                   , 0         , 0                         , 21 ), // #170
  INST(Fscaleb_d        , BaseLVVV           , (0b00000001000100010, 0, 5, 10, 0)                                                    , 0         , 0                         , 16 ), // #171
  INST(Fscaleb_s        , BaseLVVV           , (0b00000001000100001, 0, 5, 10, 0)                                                    , 0         , 0                         , 17 ), // #172
  INST(Fsel             , LfVVVI             , (0b00001101000000, 0, 5, 10, 3, 15, 0)                                                , 0         , 0                         , 0  ), // #173
  INST(Fsqrt_d          , BaseLVV            , (0b0000000100010100010010, 0, 5, 0)                                                   , 0         , 0                         , 22 ), // #174
  INST(Fsqrt_s          , BaseLVV            , (0b0000000100010100010001, 0, 5, 0)                                                   , 0         , 0                         , 23 ), // #175
  INST(Fst_d            , BaseLdSt           , (0b0010101111, 22)                                                                    , 0         , 0                         , 4  ), // #176
  INST(Fst_s            , BaseLdSt           , (0b0010101101, 22)                                                                    , 0         , 0                         , 5  ), // #177
  INST(Fstgt_d          , BaseLVRR           , (0b00111000011101101, 0, kWX, 5, kWX, 10, 0)                                          , 0         , 0                         , 4  ), // #178
  INST(Fstgt_s          , BaseLVRR           , (0b00111000011101100, 0, kWX, 5, kWX, 10, 0)                                          , 0         , 0                         , 5  ), // #179
  INST(Fstle_d          , BaseLVRR           , (0b00111000011101111, 0, kWX, 5, kWX, 10, 0)                                          , 0         , 0                         , 6  ), // #180
  INST(Fstle_s          , BaseLVRR           , (0b00111000011101110, 0, kWX, 5, kWX, 10, 0)                                          , 0         , 0                         , 7  ), // #181
  INST(Fstx_d           , BaseLdSt           , (0b00111000001111000, 15)                                                             , 0         , 0                         , 6  ), // #182
  INST(Fstx_s           , BaseLdSt           , (0b00111000001110000, 15)                                                             , 0         , 0                         , 7  ), // #183
  INST(Fsub_d           , BaseLVVV           , (0b00000001000000110, 0, 5, 10, 0)                                                    , 0         , 0                         , 18 ), // #184
  INST(Fsub_s           , BaseLVVV           , (0b00000001000000101, 0, 5, 10, 0)                                                    , 0         , 0                         , 19 ), // #185
  INST(Ftint_l_d        , BaseLVV            , (0b0000000100011011001010, 0, 5, 0)                                                   , 0         , 0                         , 24 ), // #186
  INST(Ftint_l_s        , BaseLVV            , (0b0000000100011011001001, 0, 5, 0)                                                   , 0         , 0                         , 25 ), // #187
  INST(Ftint_w_d        , BaseLVV            , (0b0000000100011011000010, 0, 5, 0)                                                   , 0         , 0                         , 26 ), // #188
  INST(Ftint_w_s        , BaseLVV            , (0b0000000100011011000001, 0, 5, 0)                                                   , 0         , 0                         , 27 ), // #189
  INST(Ftintrm_l_d      , BaseLVV            , (0b0000000100011010001010, 0, 5, 0)                                                   , 0         , 0                         , 28 ), // #190
  INST(Ftintrm_l_s      , BaseLVV            , (0b0000000100011010001001, 0, 5, 0)                                                   , 0         , 0                         , 29 ), // #191
  INST(Ftintrm_w_d      , BaseLVV            , (0b0000000100011010000010, 0, 5, 0)                                                   , 0         , 0                         , 30 ), // #192
  INST(Ftintrm_w_s      , BaseLVV            , (0b0000000100011010000001, 0, 5, 0)                                                   , 0         , 0                         , 31 ), // #193
  INST(Ftintrne_l_d     , BaseLVV            , (0b0000000100011010111010, 0, 5, 0)                                                   , 0         , 0                         , 32 ), // #194
  INST(Ftintrne_l_s     , BaseLVV            , (0b0000000100011010111001, 0, 5, 0)                                                   , 0         , 0                         , 33 ), // #195
  INST(Ftintrne_w_d     , BaseLVV            , (0b0000000100011010110010, 0, 5, 0)                                                   , 0         , 0                         , 34 ), // #196
  INST(Ftintrne_w_s     , BaseLVV            , (0b0000000100011010110001, 0, 5, 0)                                                   , 0         , 0                         , 35 ), // #197
  INST(Ftintrp_l_d      , BaseLVV            , (0b0000000100011010011010, 0, 5, 0)                                                   , 0         , 0                         , 36 ), // #198
  INST(Ftintrp_l_s      , BaseLVV            , (0b0000000100011010011001, 0, 5, 0)                                                   , 0         , 0                         , 37 ), // #199
  INST(Ftintrp_w_d      , BaseLVV            , (0b0000000100011010010010, 0, 5, 0)                                                   , 0         , 0                         , 38 ), // #200
  INST(Ftintrp_w_s      , BaseLVV            , (0b0000000100011010010001, 0, 5, 0)                                                   , 0         , 0                         , 39 ), // #201
  INST(Ftintrz_l_d      , BaseLVV            , (0b0000000100011010101010, 0, 5, 0)                                                   , 0         , 0                         , 40 ), // #202
  INST(Ftintrz_l_s      , BaseLVV            , (0b0000000100011010101001, 0, 5, 0)                                                   , 0         , 0                         , 41 ), // #203
  INST(Ftintrz_w_d      , BaseLVV            , (0b0000000100011010100010, 0, 5, 0)                                                   , 0         , 0                         , 42 ), // #204
  INST(Ftintrz_w_s      , BaseLVV            , (0b0000000100011010100001, 0, 5, 0)                                                   , 0         , 0                         , 43 ), // #205
  INST(Ibar             , BaseOpImm          , (0b00111000011100101, 15, 0)                                                          , 0         , 0                         , 1  ), // #206
  INST(Idle             , BaseOpImm          , (0b00000110010010001, 15, 0)                                                          , 0         , 0                         , 2  ), // #207
  INST(Invtlb           , BaseLIRR           , (0b00000110010010011, kWX, 5, kWX, 10, 0, 0)                                          , 0         , 0                         , 0  ), // #208
  INST(Iocsrrd_b        , BaseLRR            , (0b0000011001001000000000, kWX, 0, kWX, 5, 0)                                         , 0         , 0                         , 15 ), // #209
  INST(Iocsrrd_d        , BaseLRR            , (0b0000011001001000000011, kWX, 0, kWX, 5, 0)                                         , 0         , 0                         , 16 ), // #210
  INST(Iocsrrd_h        , BaseLRR            , (0b0000011001001000000001, kWX, 0, kWX, 5, 0)                                         , 0         , 0                         , 17 ), // #211
  INST(Iocsrrd_w        , BaseLRR            , (0b0000011001001000000010, kWX, 0, kWX, 5, 0)                                         , 0         , 0                         , 18 ), // #212
  INST(Iocsrwr_b        , BaseLRR            , (0b0000011001001000000100, kWX, 0, kWX, 5, 0)                                         , 0         , 0                         , 19 ), // #213
  INST(Iocsrwr_d        , BaseLRR            , (0b0000011001001000000111, kWX, 0, kWX, 5, 0)                                         , 0         , 0                         , 20 ), // #214
  INST(Iocsrwr_h        , BaseLRR            , (0b0000011001001000000101, kWX, 0, kWX, 5, 0)                                         , 0         , 0                         , 21 ), // #215
  INST(Iocsrwr_w        , BaseLRR            , (0b0000011001001000000110, kWX, 0, kWX, 5, 0)                                         , 0         , 0                         , 22 ), // #216
  INST(Jirl             , JBTLRRI            , (0b010011)                                                                            , 0         , 0                         , 0  ), // #217
  INST(Ld_b             , BaseLdSt           , (0b0010100000, 22)                                                                    , 0         , 0                         , 8  ), // #218
  INST(Ld_bu            , BaseLdSt           , (0b0010101000, 22)                                                                    , 0         , 0                         , 9  ), // #219
  INST(Ld_d             , BaseLdSt           , (0b0010100011, 22)                                                                    , 0         , 0                         , 10 ), // #220
  INST(Ld_h             , BaseLdSt           , (0b0010100001, 22)                                                                    , 0         , 0                         , 11 ), // #221
  INST(Ld_hu            , BaseLdSt           , (0b0010101001, 22)                                                                    , 0         , 0                         , 12 ), // #222
  INST(Ld_w             , BaseLdSt           , (0b0010100010, 22)                                                                    , 0         , 0                         , 13 ), // #223
  INST(Ld_wu            , BaseLdSt           , (0b0010101010, 22)                                                                    , 0         , 0                         , 14 ), // #224
  INST(Lddir            , BaseLRRI           , (0b00000110010000, kWX, 0, kWX, 5, 10, 4)                                             , 0         , 0                         , 4  ), // #225
  INST(Ldgt_b           , BaseLRRR           , (0b00111000011110000, kWX, 0, kWX, 5, kWX, 10, 0)                                     , 0         , 0                         , 16 ), // #226
  INST(Ldgt_d           , BaseLRRR           , (0b00111000011110011, kWX, 0, kWX, 5, kWX, 10, 0)                                     , 0         , 0                         , 17 ), // #227
  INST(Ldgt_h           , BaseLRRR           , (0b00111000011110001, kWX, 0, kWX, 5, kWX, 10, 0)                                     , 0         , 0                         , 18 ), // #228
  INST(Ldgt_w           , BaseLRRR           , (0b00111000011110010, kWX, 0, kWX, 5, kWX, 10, 0)                                     , 0         , 0                         , 19 ), // #229
  INST(Ldle_b           , BaseLRRR           , (0b00111000011110100, kWX, 0, kWX, 5, kWX, 10, 0)                                     , 0         , 0                         , 20 ), // #230
  INST(Ldle_d           , BaseLRRR           , (0b00111000011110111, kWX, 0, kWX, 5, kWX, 10, 0)                                     , 0         , 0                         , 21 ), // #231
  INST(Ldle_h           , BaseLRRR           , (0b00111000011110101, kWX, 0, kWX, 5, kWX, 10, 0)                                     , 0         , 0                         , 22 ), // #232
  INST(Ldle_w           , BaseLRRR           , (0b00111000011110110, kWX, 0, kWX, 5, kWX, 10, 0)                                     , 0         , 0                         , 23 ), // #233
  INST(Ldpte            , BaseLRI            , (0b00000110010001, kWX, 5, 10, 4)                                                     , 0         , 0                         , 2  ), // #234
  INST(Ldptr_d          , LFPldst            , (0b00100110)                                                                          , 0         , 0                         , 0  ), // #235
  INST(Ldptr_w          , LFPldst            , (0b00100100)                                                                          , 0         , 0                         , 1  ), // #236
  INST(Ldx_b            , BaseLdSt           , (0b00111000000000000, 15)                                                             , 0         , 0                         , 15 ), // #237
  INST(Ldx_bu           , BaseLdSt           , (0b00111000001000000, 15)                                                             , 0         , 0                         , 16 ), // #238
  INST(Ldx_d            , BaseLdSt           , (0b00111000000011000, 15)                                                             , 0         , 0                         , 17 ), // #239
  INST(Ldx_h            , BaseLdSt           , (0b00111000000001000, 15)                                                             , 0         , 0                         , 18 ), // #240
  INST(Ldx_hu           , BaseLdSt           , (0b00111000001001000, 15)                                                             , 0         , 0                         , 19 ), // #241
  INST(Ldx_w            , BaseLdSt           , (0b00111000000010000, 15)                                                             , 0         , 0                         , 20 ), // #242
  INST(Ldx_wu           , BaseLdSt           , (0b00111000001010000, 15)                                                             , 0         , 0                         , 21 ), // #243
  INST(Ll_d             , LFPldst            , (0b00100010)                                                                          , 0         , 0                         , 2  ), // #244
  INST(Ll_w             , LFPldst            , (0b00100000)                                                                          , 0         , 0                         , 3  ), // #245
  INST(Lu12i_w          , BaseLRI            , (0b0001010, kWX, 0, 5, 0)                                                             , 0         , 0                         , 3  ), // #246
  INST(Lu32i_d          , BaseLRI            , (0b0001011, kWX, 0, 5, 0)                                                             , 0         , 0                         , 4  ), // #247
  INST(Lu52i_d          , BaseLRRI           , (0b0000001100, kX, 0, kSP, 5, 10, 0)                                                  , 0         , 0                         , 5  ), // #248
  INST(Maskeqz          , BaseLRRR           , (0b00000000000100110, kWX, 0, kWX, 5, kWX, 10, 0)                                     , 0         , 0                         , 24 ), // #249
  INST(Masknez          , BaseLRRR           , (0b00000000000100111, kWX, 0, kWX, 5, kWX, 10, 0)                                     , 0         , 0                         , 25 ), // #250
  INST(Mod_d            , BaseLRRR           , (0b00000000001000101, kWX, 0, kWX, 5, kWX, 10, 0)                                     , 0         , 0                         , 26 ), // #251
  INST(Mod_du           , BaseLRRR           , (0b00000000001000111, kWX, 0, kWX, 5, kWX, 10, 0)                                     , 0         , 0                         , 27 ), // #252
  INST(Mod_w            , BaseLRRR           , (0b00000000001000001, kWX, 0, kWX, 5, kWX, 10, 0)                                     , 0         , 0                         , 28 ), // #253
  INST(Mod_wu           , BaseLRRR           , (0b00000000001000011, kWX, 0, kWX, 5, kWX, 10, 0)                                     , 0         , 0                         , 29 ), // #254
  INST(Movcf2fr         , BaseLVI            , (0b0000000100010100110101, 0, 5)                                                      , 0         , 0                         , 0  ), // #255
  INST(Movcf2gr         , BaseLRI            , (0b0000000100010100110111, kWX, 0, 5, 1)                                              , 0         , 0                         , 5  ), // #256
  INST(Movfcsr2gr       , BaseLRR            , (0b0000000100010100110010, kWX, 0, kWX, 5, 0)                                         , 0         , 0                         , 23 ), // #257
  INST(Movfr2cf         , BaseLIV            , (0b0000000100010100110100, 5, 0)                                                      , 0         , 0                         , 0  ), // #258
  INST(Movfr2gr_d       , BaseLRV            , (0b0000000100010100101110, kWX, 0, 5, 0)                                              , 0         , 0                         , 0  ), // #259
  INST(Movfr2gr_s       , BaseLRV            , (0b0000000100010100101101, kWX, 0, 5, 0)                                              , 0         , 0                         , 1  ), // #260
  INST(Movfrh2gr_s      , BaseLRV            , (0b0000000100010100101111, kWX, 0, 5, 0)                                              , 0         , 0                         , 2  ), // #261
  INST(Movgr2cf         , BaseLIR            , (0b0000000100010100110110, kWX, 5, 0)                                                 , 0         , 0                         , 0  ), // #262
  INST(Movgr2fcsr       , BaseLRR            , (0b0000000100010100110000, kWX, 0, kWX, 5, 0)                                         , 0         , 0                         , 24 ), // #263
  INST(Movgr2fr_d       , BaseLVR            , (0b0000000100010100101010, 0, kWX, 5, 0)                                              , 0         , 0                         , 0  ), // #264
  INST(Movgr2fr_w       , BaseLVR            , (0b0000000100010100101001, 0, kWX, 5, 0)                                              , 0         , 0                         , 1  ), // #265
  INST(Movgr2frh_w      , BaseLVR            , (0b0000000100010100101011, 0, kWX, 5, 0)                                              , 0         , 0                         , 2  ), // #266
  INST(Mul_d            , BaseLRRR           , (0b00000000000111011, kWX, 0, kWX, 5, kWX, 10, 0)                                     , 0         , 0                         , 30 ), // #267
  INST(Mul_w            , BaseLRRR           , (0b00000000000111000, kWX, 0, kWX, 5, kWX, 10, 0)                                     , 0         , 0                         , 31 ), // #268
  INST(Mulh_d           , BaseLRRR           , (0b00000000000111100, kWX, 0, kWX, 5, kWX, 10, 0)                                     , 0         , 0                         , 32 ), // #269
  INST(Mulh_du          , BaseLRRR           , (0b00000000000111101, kWX, 0, kWX, 5, kWX, 10, 0)                                     , 0         , 0                         , 33 ), // #270
  INST(Mulh_w           , BaseLRRR           , (0b00000000000111001, kWX, 0, kWX, 5, kWX, 10, 0)                                     , 0         , 0                         , 34 ), // #271
  INST(Mulh_wu          , BaseLRRR           , (0b00000000000111010, kWX, 0, kWX, 5, kWX, 10, 0)                                     , 0         , 0                         , 35 ), // #272
  INST(Mulw_d_w         , BaseLRRR           , (0b00000000000111110, kWX, 0, kWX, 5, kWX, 10, 0)                                     , 0         , 0                         , 36 ), // #273
  INST(Mulw_d_wu        , BaseLRRR           , (0b00000000000111111, kWX, 0, kWX, 5, kWX, 10, 0)                                     , 0         , 0                         , 37 ), // #274
  INST(Nor              , BaseLRRR           , (0b00000000000101000, kWX, 0, kWX, 5, kWX, 10, 0)                                     , 0         , 0                         , 38 ), // #275
  INST(Or_              , BaseLRRR           , (0b00000000000101010, kWX, 0, kWX, 5, kWX, 10, 0)                                     , 0         , 0                         , 39 ), // #276
  INST(Move             , BaseLRR            , (0b0000000000010101000000, kWX, 0, kWX, 5, 0)                                         , 0         , 0                         , 25 ), // #277
  INST(Ori              , BaseLRRI           , (0b0000001110, kX, 0, kSP, 5, 10, 5)                                                  , 0         , 0                         , 6  ), // #278
  INST(Orn              , BaseLRRR           , (0b00000000000101100, kWX, 0, kWX, 5, kWX, 10, 0)                                     , 0         , 0                         , 40 ), // #279
  INST(Pcaddi           , BaseLRI            , (0b0001100, kWX, 0, 5, 0)                                                             , 0         , 0                         , 6  ), // #280
  INST(Pcaddu12i        , BaseLRI            , (0b0001110, kWX, 0, 5, 0)                                                             , 0         , 0                         , 7  ), // #281
  INST(Pcaddu18i        , BaseLRI            , (0b0001111, kWX, 0, 5, 0)                                                             , 0         , 0                         , 8  ), // #282
  INST(Pcalau12i        , BaseLRI            , (0b0001101, kWX, 0, 5, 0)                                                             , 0         , 0                         , 9  ), // #283
  INST(Preld            , LPldst             , (0b0010101011)                                                                        , 0         , 0                         , 0  ), // #284
  INST(Rdtime_d         , BaseLRR            , (0b0000000000000000011010, kWX, 0, kWX, 5, 0)                                         , 0         , 0                         , 26 ), // #285
  INST(Rdtimeh_w        , BaseLRR            , (0b0000000000000000011001, kWX, 0, kWX, 5, 0)                                         , 0         , 0                         , 27 ), // #286
  INST(Rdtimel_w        , BaseLRR            , (0b0000000000000000011000, kWX, 0, kWX, 5, 0)                                         , 0         , 0                         , 28 ), // #287
  INST(Revb_2h          , BaseLRR            , (0b0000000000000000001100, kWX, 0, kWX, 5, 0)                                         , 0         , 0                         , 29 ), // #288
  INST(Revb_2w          , BaseLRR            , (0b0000000000000000001110, kWX, 0, kWX, 5, 0)                                         , 0         , 0                         , 30 ), // #289
  INST(Revb_4h          , BaseLRR            , (0b0000000000000000001101, kWX, 0, kWX, 5, 0)                                         , 0         , 0                         , 31 ), // #290
  INST(Revb_d           , BaseLRR            , (0b0000000000000000001111, kWX, 0, kWX, 5, 0)                                         , 0         , 0                         , 32 ), // #291
  INST(Revh_2w          , BaseLRR            , (0b0000000000000000010000, kWX, 0, kWX, 5, 0)                                         , 0         , 0                         , 33 ), // #292
  INST(Revh_d           , BaseLRR            , (0b0000000000000000010001, kWX, 0, kWX, 5, 0)                                         , 0         , 0                         , 34 ), // #293
  INST(Rotr_d           , BaseLRRR           , (0b00000000000110111, kWX, 0, kWX, 5, kWX, 10, 0)                                     , 0         , 0                         , 41 ), // #294
  INST(Rotr_w           , BaseLRRR           , (0b00000000000110110, kWX, 0, kWX, 5, kWX, 10, 0)                                     , 0         , 0                         , 42 ), // #295
  INST(Rotri_d          , BaseLRRI           , (0b0000000001001101, kWX, 0, kWX, 5, 10, 2)                                           , 0         , 0                         , 7  ), // #296
  INST(Rotri_w          , BaseLRRI           , (0b00000000010011001, kWX, 0, kWX, 5, 10, 1)                                          , 0         , 0                         , 8  ), // #297
  INST(Sc_d             , LFPldst            , (0b00100011)                                                                          , 0         , 0                         , 4  ), // #298
  INST(Sc_w             , LFPldst            , (0b00100001)                                                                          , 0         , 0                         , 5  ), // #299
  INST(Sll_d            , BaseLRRR           , (0b00000000000110001, kWX, 0, kWX, 5, kWX, 10, 0)                                     , 0         , 0                         , 43 ), // #300
  INST(Sll_w            , BaseLRRR           , (0b00000000000101110, kWX, 0, kWX, 5, kWX, 10, 0)                                     , 0         , 0                         , 44 ), // #301
  INST(Slli_d           , BaseLRRI           , (0b0000000001000001, kWX, 0, kWX, 5, 10, 2)                                           , 0         , 0                         , 9  ), // #302
  INST(Slli_w           , BaseLRRI           , (0b00000000010000001, kWX, 0, kWX, 5, 10, 1)                                          , 0         , 0                         , 10 ), // #303
  INST(Slti             , BaseLRRI           , (0b0000001000, kX, 0, kSP, 5, 10, 0)                                                  , 0         , 0                         , 11 ), // #304
  INST(Sltui            , BaseLRRI           , (0b0000001001, kX, 0, kSP, 5, 10, 0)                                                  , 0         , 0                         , 12 ), // #305
  INST(Sra_d            , BaseLRRR           , (0b00000000000110011, kWX, 0, kWX, 5, kWX, 10, 0)                                     , 0         , 0                         , 45 ), // #306
  INST(Sra_w            , BaseLRRR           , (0b00000000000110000, kWX, 0, kWX, 5, kWX, 10, 0)                                     , 0         , 0                         , 46 ), // #307
  INST(Srai_d           , BaseLRRI           , (0b0000000001001001, kWX, 0, kWX, 5, 10, 2)                                           , 0         , 0                         , 13 ), // #308
  INST(Srai_w           , BaseLRRI           , (0b00000000010010001, kWX, 0, kWX, 5, 10, 1)                                          , 0         , 0                         , 14 ), // #309
  INST(Srl_d            , BaseLRRR           , (0b00000000000110010, kWX, 0, kWX, 5, kWX, 10, 0)                                     , 0         , 0                         , 47 ), // #310
  INST(Srl_w            , BaseLRRR           , (0b00000000000101111, kWX, 0, kWX, 5, kWX, 10, 0)                                     , 0         , 0                         , 48 ), // #311
  INST(Srli_d           , BaseLRRI           , (0b0000000001000101, kWX, 0, kWX, 5, 10, 2)                                           , 0         , 0                         , 15 ), // #312
  INST(Srli_w           , BaseLRRI           , (0b00000000010001001, kWX, 0, kWX, 5, 10, 1)                                          , 0         , 0                         , 16 ), // #313
  INST(St_b             , BaseLdSt           , (0b0010100100, 22)                                                                    , 0         , 0                         , 22 ), // #314
  INST(St_d             , BaseLdSt           , (0b0010100111, 22)                                                                    , 0         , 0                         , 23 ), // #315
  INST(St_h             , BaseLdSt           , (0b0010100101, 22)                                                                    , 0         , 0                         , 24 ), // #316
  INST(St_w             , BaseLdSt           , (0b0010100110, 22)                                                                    , 0         , 0                         , 25 ), // #317
  INST(Stgt_b           , BaseLRRR           , (0b00111000011111000, kWX, 0, kWX, 5, kWX, 10, 0)                                     , 0         , 0                         , 49 ), // #318
  INST(Stgt_d           , BaseLRRR           , (0b00111000011111011, kWX, 0, kWX, 5, kWX, 10, 0)                                     , 0         , 0                         , 50 ), // #319
  INST(Stgt_h           , BaseLRRR           , (0b00111000011111001, kWX, 0, kWX, 5, kWX, 10, 0)                                     , 0         , 0                         , 51 ), // #320
  INST(Stgt_w           , BaseLRRR           , (0b00111000011111010, kWX, 0, kWX, 5, kWX, 10, 0)                                     , 0         , 0                         , 52 ), // #321
  INST(Stle_b           , BaseLRRR           , (0b00111000011111100, kWX, 0, kWX, 5, kWX, 10, 0)                                     , 0         , 0                         , 53 ), // #322
  INST(Stle_d           , BaseLRRR           , (0b00111000011111111, kWX, 0, kWX, 5, kWX, 10, 0)                                     , 0         , 0                         , 54 ), // #323
  INST(Stle_h           , BaseLRRR           , (0b00111000011111101, kWX, 0, kWX, 5, kWX, 10, 0)                                     , 0         , 0                         , 55 ), // #324
  INST(Stle_w           , BaseLRRR           , (0b00111000011111110, kWX, 0, kWX, 5, kWX, 10, 0)                                     , 0         , 0                         , 56 ), // #325
  INST(Stptr_d          , LFPldst            , (0b00100111)                                                                          , 0         , 0                         , 6  ), // #326
  INST(Stptr_w          , LFPldst            , (0b00100101)                                                                          , 0         , 0                         , 7  ), // #327
  INST(Stx_b            , BaseLdSt           , (0b00111000000100000, 15)                                                             , 0         , 0                         , 26 ), // #328
  INST(Stx_d            , BaseLdSt           , (0b00111000000111000, 15)                                                             , 0         , 0                         , 27 ), // #329
  INST(Stx_h            , BaseLdSt           , (0b00111000000101000, 15)                                                             , 0         , 0                         , 28 ), // #330
  INST(Stx_w            , BaseLdSt           , (0b00111000000110000, 15)                                                             , 0         , 0                         , 29 ), // #331
  INST(Sub_d            , BaseLRRR           , (0b00000000000100011, kWX, 0, kWX, 5, kWX, 10, 0)                                     , 0         , 0                         , 57 ), // #332
  INST(Sub_w            , BaseLRRR           , (0b00000000000100010, kWX, 0, kWX, 5, kWX, 10, 0)                                     , 0         , 0                         , 58 ), // #333
  INST(Syscall          , BaseLIC            , (0b00000000001010110, 0)                                                              , 0         , 0                         , 2  ), // #334
  INST(Tlbclr           , BaseOp             , (0b00000110010010000010000000000000)                                                  , 0         , 0                         , 1  ), // #335
  INST(Tlbfill          , BaseOp             , (0b00000110010010000011010000000000)                                                  , 0         , 0                         , 2  ), // #336
  INST(Tlbflush         , BaseOp             , (0b00000110010010000010010000000000)                                                  , 0         , 0                         , 3  ), // #337
  INST(Tlbrd            , BaseOp             , (0b00000110010010000010110000000000)                                                  , 0         , 0                         , 4  ), // #338
  INST(Tlbsrch          , BaseOp             , (0b00000110010010000010100000000000)                                                  , 0         , 0                         , 5  ), // #339
  INST(Tlbwr            , BaseOp             , (0b00000110010010000011000000000000)                                                  , 0         , 0                         , 6  ), // #340
  INST(Xor_             , BaseLRRR           , (0b00000000000101011, kWX, 0, kWX, 5, kWX, 10, 0)                                     , 0         , 0                         , 59 ), // #341
  INST(Xori             , BaseLRRI           , (0b0000001111, kX, 0, kSP, 5, 10, 5)                                                  , 0         , 0                         , 17 ), // #342
  INST(Vabsd_b          , LsxVVV             , (0b01110000011000000, 0, 5, 10, 0)                                                    , 0         , 0                         , 0  ), // #343
  INST(Vabsd_bu         , LsxVVV             , (0b01110000011000100, 0, 5, 10, 0)                                                    , 0         , 0                         , 1  ), // #344
  INST(Vabsd_d          , LsxVVV             , (0b01110000011000011, 0, 5, 10, 0)                                                    , 0         , 0                         , 2  ), // #345
  INST(Vabsd_du         , LsxVVV             , (0b01110000011000111, 0, 5, 10, 0)                                                    , 0         , 0                         , 3  ), // #346
  INST(Vabsd_h          , LsxVVV             , (0b01110000011000001, 0, 5, 10, 0)                                                    , 0         , 0                         , 4  ), // #347
  INST(Vabsd_hu         , LsxVVV             , (0b01110000011000101, 0, 5, 10, 0)                                                    , 0         , 0                         , 5  ), // #348
  INST(Vabsd_w          , LsxVVV             , (0b01110000011000010, 0, 5, 10, 0)                                                    , 0         , 0                         , 6  ), // #349
  INST(Vabsd_wu         , LsxVVV             , (0b01110000011000110, 0, 5, 10, 0)                                                    , 0         , 0                         , 7  ), // #350
  INST(Vadd_b           , LsxVVV             , (0b01110000000010100, 0, 5, 10, 0)                                                    , 0         , 0                         , 8  ), // #351
  INST(Vadd_d           , LsxVVV             , (0b01110000000010111, 0, 5, 10, 0)                                                    , 0         , 0                         , 9  ), // #352
  INST(Vadd_h           , LsxVVV             , (0b01110000000010101, 0, 5, 10, 0)                                                    , 0         , 0                         , 10 ), // #353
  INST(Vadd_q           , LsxVVV             , (0b01110001001011010, 0, 5, 10, 0)                                                    , 0         , 0                         , 11 ), // #354
  INST(Vadd_w           , LsxVVV             , (0b01110000000010110, 0, 5, 10, 0)                                                    , 0         , 0                         , 12 ), // #355
  INST(Vadda_b          , LsxVVV             , (0b01110000010111000, 0, 5, 10, 0)                                                    , 0         , 0                         , 13 ), // #356
  INST(Vadda_d          , LsxVVV             , (0b01110000010111011, 0, 5, 10, 0)                                                    , 0         , 0                         , 14 ), // #357
  INST(Vadda_h          , LsxVVV             , (0b01110000010111001, 0, 5, 10, 0)                                                    , 0         , 0                         , 15 ), // #358
  INST(Vadda_w          , LsxVVV             , (0b01110000010111010, 0, 5, 10, 0)                                                    , 0         , 0                         , 16 ), // #359
  INST(Vaddi_bu         , LsxVVI             , (0b01110010100010100, 0, 5, 10, 0)                                                    , 0         , 0                         , 0  ), // #360
  INST(Vaddi_du         , LsxVVI             , (0b01110010100010111, 0, 5, 10, 0)                                                    , 0         , 0                         , 1  ), // #361
  INST(Vaddi_hu         , LsxVVI             , (0b01110010100010101, 0, 5, 10, 0)                                                    , 0         , 0                         , 2  ), // #362
  INST(Vaddi_wu         , LsxVVI             , (0b01110010100010110, 0, 5, 10, 0)                                                    , 0         , 0                         , 3  ), // #363
  INST(Vaddwev_d_w      , LsxVVV             , (0b01110000000111110, 0, 5, 10, 0)                                                    , 0         , 0                         , 17 ), // #364
  INST(Vaddwev_d_wu     , LsxVVV             , (0b01110000001011110, 0, 5, 10, 0)                                                    , 0         , 0                         , 18 ), // #365
  INST(Vaddwev_d_wu_w   , LsxVVV             , (0b01110000001111110, 0, 5, 10, 0)                                                    , 0         , 0                         , 19 ), // #366
  INST(Vaddwev_h_b      , LsxVVV             , (0b01110000000111100, 0, 5, 10, 0)                                                    , 0         , 0                         , 20 ), // #367
  INST(Vaddwev_h_bu     , LsxVVV             , (0b01110000001011100, 0, 5, 10, 0)                                                    , 0         , 0                         , 21 ), // #368
  INST(Vaddwev_h_bu_b   , LsxVVV             , (0b01110000001111100, 0, 5, 10, 0)                                                    , 0         , 0                         , 22 ), // #369
  INST(Vaddwev_q_d      , LsxVVV             , (0b01110000000111111, 0, 5, 10, 0)                                                    , 0         , 0                         , 23 ), // #370
  INST(Vaddwev_q_du     , LsxVVV             , (0b01110000001011111, 0, 5, 10, 0)                                                    , 0         , 0                         , 24 ), // #371
  INST(Vaddwev_q_du_d   , LsxVVV             , (0b01110000001111111, 0, 5, 10, 0)                                                    , 0         , 0                         , 25 ), // #372
  INST(Vaddwev_w_h      , LsxVVV             , (0b01110000000111101, 0, 5, 10, 0)                                                    , 0         , 0                         , 26 ), // #373
  INST(Vaddwev_w_hu     , LsxVVV             , (0b01110000001011101, 0, 5, 10, 0)                                                    , 0         , 0                         , 27 ), // #374
  INST(Vaddwev_w_hu_h   , LsxVVV             , (0b01110000001111101, 0, 5, 10, 0)                                                    , 0         , 0                         , 28 ), // #375
  INST(Vaddwod_d_w      , LsxVVV             , (0b01110000001000110, 0, 5, 10, 0)                                                    , 0         , 0                         , 29 ), // #376
  INST(Vaddwod_d_wu     , LsxVVV             , (0b01110000001100110, 0, 5, 10, 0)                                                    , 0         , 0                         , 30 ), // #377
  INST(Vaddwod_d_wu_w   , LsxVVV             , (0b01110000010000010, 0, 5, 10, 0)                                                    , 0         , 0                         , 31 ), // #378
  INST(Vaddwod_h_b      , LsxVVV             , (0b01110000001000100, 0, 5, 10, 0)                                                    , 0         , 0                         , 32 ), // #379
  INST(Vaddwod_h_bu     , LsxVVV             , (0b01110000001100100, 0, 5, 10, 0)                                                    , 0         , 0                         , 33 ), // #380
  INST(Vaddwod_h_bu_b   , LsxVVV             , (0b01110000010000000, 0, 5, 10, 0)                                                    , 0         , 0                         , 34 ), // #381
  INST(Vaddwod_q_d      , LsxVVV             , (0b01110000001000111, 0, 5, 10, 0)                                                    , 0         , 0                         , 35 ), // #382
  INST(Vaddwod_q_du     , LsxVVV             , (0b01110000001100111, 0, 5, 10, 0)                                                    , 0         , 0                         , 36 ), // #383
  INST(Vaddwod_q_du_d   , LsxVVV             , (0b01110000010000011, 0, 5, 10, 0)                                                    , 0         , 0                         , 37 ), // #384
  INST(Vaddwod_w_h      , LsxVVV             , (0b01110000001000101, 0, 5, 10, 0)                                                    , 0         , 0                         , 38 ), // #385
  INST(Vaddwod_w_hu     , LsxVVV             , (0b01110000001100101, 0, 5, 10, 0)                                                    , 0         , 0                         , 39 ), // #386
  INST(Vaddwod_w_hu_h   , LsxVVV             , (0b01110000010000001, 0, 5, 10, 0)                                                    , 0         , 0                         , 40 ), // #387
  INST(Vand_v           , LsxVVV             , (0b01110001001001100, 0, 5, 10, 0)                                                    , 0         , 0                         , 41 ), // #388
  INST(Vandi_b          , LsxVVI             , (0b01110011110100, 0, 5, 10, 0)                                                       , 0         , 0                         , 4  ), // #389
  INST(Vandn_v          , LsxVVV             , (0b01110001001010000, 0, 5, 10, 0)                                                    , 0         , 0                         , 42 ), // #390
  INST(Vavg_b           , LsxVVV             , (0b01110000011001000, 0, 5, 10, 0)                                                    , 0         , 0                         , 43 ), // #391
  INST(Vavg_bu          , LsxVVV             , (0b01110000011001100, 0, 5, 10, 0)                                                    , 0         , 0                         , 44 ), // #392
  INST(Vavg_d           , LsxVVV             , (0b01110000011001011, 0, 5, 10, 0)                                                    , 0         , 0                         , 45 ), // #393
  INST(Vavg_du          , LsxVVV             , (0b01110000011001111, 0, 5, 10, 0)                                                    , 0         , 0                         , 46 ), // #394
  INST(Vavg_h           , LsxVVV             , (0b01110000011001001, 0, 5, 10, 0)                                                    , 0         , 0                         , 47 ), // #395
  INST(Vavg_hu          , LsxVVV             , (0b01110000011001101, 0, 5, 10, 0)                                                    , 0         , 0                         , 48 ), // #396
  INST(Vavg_w           , LsxVVV             , (0b01110000011001010, 0, 5, 10, 0)                                                    , 0         , 0                         , 49 ), // #397
  INST(Vavg_wu          , LsxVVV             , (0b01110000011001110, 0, 5, 10, 0)                                                    , 0         , 0                         , 50 ), // #398
  INST(Vavgr_b          , LsxVVV             , (0b01110000011010000, 0, 5, 10, 0)                                                    , 0         , 0                         , 51 ), // #399
  INST(Vavgr_bu         , LsxVVV             , (0b01110000011010100, 0, 5, 10, 0)                                                    , 0         , 0                         , 52 ), // #400
  INST(Vavgr_d          , LsxVVV             , (0b01110000011010011, 0, 5, 10, 0)                                                    , 0         , 0                         , 53 ), // #401
  INST(Vavgr_du         , LsxVVV             , (0b01110000011010111, 0, 5, 10, 0)                                                    , 0         , 0                         , 54 ), // #402
  INST(Vavgr_h          , LsxVVV             , (0b01110000011010001, 0, 5, 10, 0)                                                    , 0         , 0                         , 55 ), // #403
  INST(Vavgr_hu         , LsxVVV             , (0b01110000011010101, 0, 5, 10, 0)                                                    , 0         , 0                         , 56 ), // #404
  INST(Vavgr_w          , LsxVVV             , (0b01110000011010010, 0, 5, 10, 0)                                                    , 0         , 0                         , 57 ), // #405
  INST(Vavgr_wu         , LsxVVV             , (0b01110000011010110, 0, 5, 10, 0)                                                    , 0         , 0                         , 58 ), // #406
  INST(Vbitclr_b        , LsxVVV             , (0b01110001000011000, 0, 5, 10, 0)                                                    , 0         , 0                         , 59 ), // #407
  INST(Vbitclr_d        , LsxVVV             , (0b01110001000011011, 0, 5, 10, 0)                                                    , 0         , 0                         , 60 ), // #408
  INST(Vbitclr_h        , LsxVVV             , (0b01110001000011001, 0, 5, 10, 0)                                                    , 0         , 0                         , 61 ), // #409
  INST(Vbitclr_w        , LsxVVV             , (0b01110001000011010, 0, 5, 10, 0)                                                    , 0         , 0                         , 62 ), // #410
  INST(Vbitclri_b       , LsxVVI             , (0b0111001100010000001, 0, 5, 10, 0)                                                  , 0         , 0                         , 5  ), // #411
  INST(Vbitclri_d       , LsxVVI             , (0b0111001100010001, 0, 5, 10, 0)                                                     , 0         , 0                         , 6  ), // #412
  INST(Vbitclri_h       , LsxVVI             , (0b011100110001000001, 0, 5, 10, 0)                                                   , 0         , 0                         , 7  ), // #413
  INST(Vbitclri_w       , LsxVVI             , (0b01110011000100001, 0, 5, 10, 0)                                                    , 0         , 0                         , 8  ), // #414
  INST(Vbitrev_b        , LsxVVV             , (0b01110001000100000, 0, 5, 10, 0)                                                    , 0         , 0                         , 63 ), // #415
  INST(Vbitrev_d        , LsxVVV             , (0b01110001000100011, 0, 5, 10, 0)                                                    , 0         , 0                         , 64 ), // #416
  INST(Vbitrev_h        , LsxVVV             , (0b01110001000100001, 0, 5, 10, 0)                                                    , 0         , 0                         , 65 ), // #417
  INST(Vbitrev_w        , LsxVVV             , (0b01110001000100010, 0, 5, 10, 0)                                                    , 0         , 0                         , 66 ), // #418
  INST(Vbitrevi_b       , LsxVVI             , (0b0111001100011000001, 0, 5, 10, 0)                                                  , 0         , 0                         , 9  ), // #419
  INST(Vbitrevi_d       , LsxVVI             , (0b0111001100011001, 0, 5, 10, 0)                                                     , 0         , 0                         , 10 ), // #420
  INST(Vbitrevi_h       , LsxVVI             , (0b011100110001100001, 0, 5, 10, 0)                                                   , 0         , 0                         , 11 ), // #421
  INST(Vbitrevi_w       , LsxVVI             , (0b01110011000110001, 0, 5, 10, 0)                                                    , 0         , 0                         , 12 ), // #422
  INST(Vbitsel_v        , LsxVVVV            , (0b000011010001, 0, 5, 10, 15, 0)                                                     , 0         , 0                         , 0  ), // #423
  INST(Vbitseli_b       , LsxVVI             , (0b01110011110001, 0, 5, 10, 0)                                                       , 0         , 0                         , 13 ), // #424
  INST(Vbitset_b        , LsxVVV             , (0b01110001000011100, 0, 5, 10, 0)                                                    , 0         , 0                         , 67 ), // #425
  INST(Vbitset_d        , LsxVVV             , (0b01110001000011111, 0, 5, 10, 0)                                                    , 0         , 0                         , 68 ), // #426
  INST(Vbitset_h        , LsxVVV             , (0b01110001000011101, 0, 5, 10, 0)                                                    , 0         , 0                         , 69 ), // #427
  INST(Vbitset_w        , LsxVVV             , (0b01110001000011110, 0, 5, 10, 0)                                                    , 0         , 0                         , 70 ), // #428
  INST(Vbitseti_b       , LsxVVI             , (0b0111001100010100001, 0, 5, 10, 0)                                                  , 0         , 0                         , 14 ), // #429
  INST(Vbitseti_d       , LsxVVI             , (0b0111001100010101, 0, 5, 10, 0)                                                     , 0         , 0                         , 15 ), // #430
  INST(Vbitseti_h       , LsxVVI             , (0b011100110001010001, 0, 5, 10, 0)                                                   , 0         , 0                         , 16 ), // #431
  INST(Vbitseti_w       , LsxVVI             , (0b01110011000101001, 0, 5, 10, 0)                                                    , 0         , 0                         , 17 ), // #432
  INST(Vbsll_v          , LsxVVI             , (0b01110010100011100, 0, 5, 10, 0)                                                    , 0         , 0                         , 18 ), // #433
  INST(Vbsrl_v          , LsxVVI             , (0b01110010100011101, 0, 5, 10, 0)                                                    , 0         , 0                         , 19 ), // #434
  INST(Vclo_b           , LsxVV              , (0b0111001010011100000000, 0, 5, 0)                                                   , 0         , 0                         , 0  ), // #435
  INST(Vclo_d           , LsxVV              , (0b0111001010011100000011, 0, 5, 0)                                                   , 0         , 0                         , 1  ), // #436
  INST(Vclo_h           , LsxVV              , (0b0111001010011100000001, 0, 5, 0)                                                   , 0         , 0                         , 2  ), // #437
  INST(Vclo_w           , LsxVV              , (0b0111001010011100000010, 0, 5, 0)                                                   , 0         , 0                         , 3  ), // #438
  INST(Vclz_b           , LsxVV              , (0b0111001010011100000100, 0, 5, 0)                                                   , 0         , 0                         , 4  ), // #439
  INST(Vclz_d           , LsxVV              , (0b0111001010011100000111, 0, 5, 0)                                                   , 0         , 0                         , 5  ), // #440
  INST(Vclz_h           , LsxVV              , (0b0111001010011100000101, 0, 5, 0)                                                   , 0         , 0                         , 6  ), // #441
  INST(Vclz_w           , LsxVV              , (0b0111001010011100000110, 0, 5, 0)                                                   , 0         , 0                         , 7  ), // #442
  INST(Vdiv_b           , LsxVVV             , (0b01110000111000000, 0, 5, 10, 0)                                                    , 0         , 0                         , 71 ), // #443
  INST(Vdiv_bu          , LsxVVV             , (0b01110000111001000, 0, 5, 10, 0)                                                    , 0         , 0                         , 72 ), // #444
  INST(Vdiv_d           , LsxVVV             , (0b01110000111000011, 0, 5, 10, 0)                                                    , 0         , 0                         , 73 ), // #445
  INST(Vdiv_du          , LsxVVV             , (0b01110000111001011, 0, 5, 10, 0)                                                    , 0         , 0                         , 74 ), // #446
  INST(Vdiv_h           , LsxVVV             , (0b01110000111000001, 0, 5, 10, 0)                                                    , 0         , 0                         , 75 ), // #447
  INST(Vdiv_hu          , LsxVVV             , (0b01110000111001001, 0, 5, 10, 0)                                                    , 0         , 0                         , 76 ), // #448
  INST(Vdiv_w           , LsxVVV             , (0b01110000111000010, 0, 5, 10, 0)                                                    , 0         , 0                         , 77 ), // #449
  INST(Vdiv_wu          , LsxVVV             , (0b01110000111001010, 0, 5, 10, 0)                                                    , 0         , 0                         , 78 ), // #450
  INST(Vext2xv_d_b      , LasxXX             , (0b0111011010011111000110, 0, 5, 0)                                                   , 0         , 0                         , 0  ), // #451
  INST(Vext2xv_d_h      , LasxXX             , (0b0111011010011111001000, 0, 5, 0)                                                   , 0         , 0                         , 1  ), // #452
  INST(Vext2xv_d_w      , LasxXX             , (0b0111011010011111001001, 0, 5, 0)                                                   , 0         , 0                         , 2  ), // #453
  INST(Vext2xv_du_bu    , LasxXX             , (0b0111011010011111001100, 0, 5, 0)                                                   , 0         , 0                         , 3  ), // #454
  INST(Vext2xv_du_hu    , LasxXX             , (0b0111011010011111001110, 0, 5, 0)                                                   , 0         , 0                         , 4  ), // #455
  INST(Vext2xv_du_wu    , LasxXX             , (0b0111011010011111001111, 0, 5, 0)                                                   , 0         , 0                         , 5  ), // #456
  INST(Vext2xv_h_b      , LasxXX             , (0b0111011010011111000100, 0, 5, 0)                                                   , 0         , 0                         , 6  ), // #457
  INST(Vext2xv_hu_bu    , LasxXX             , (0b0111011010011111001010, 0, 5, 0)                                                   , 0         , 0                         , 7  ), // #458
  INST(Vext2xv_w_b      , LasxXX             , (0b0111011010011111000101, 0, 5, 0)                                                   , 0         , 0                         , 8  ), // #459
  INST(Vext2xv_w_h      , LasxXX             , (0b0111011010011111000111, 0, 5, 0)                                                   , 0         , 0                         , 9  ), // #460
  INST(Vext2xv_wu_bu    , LasxXX             , (0b0111011010011111001011, 0, 5, 0)                                                   , 0         , 0                         , 10 ), // #461
  INST(Vext2xv_wu_hu    , LasxXX             , (0b0111011010011111001101, 0, 5, 0)                                                   , 0         , 0                         , 11 ), // #462
  INST(Vexth_d_w        , LsxVV              , (0b0111001010011110111010, 0, 5, 0)                                                   , 0         , 0                         , 8  ), // #463
  INST(Vexth_du_wu      , LsxVV              , (0b0111001010011110111110, 0, 5, 0)                                                   , 0         , 0                         , 9  ), // #464
  INST(Vexth_h_b        , LsxVV              , (0b0111001010011110111000, 0, 5, 0)                                                   , 0         , 0                         , 10 ), // #465
  INST(Vexth_hu_bu      , LsxVV              , (0b0111001010011110111100, 0, 5, 0)                                                   , 0         , 0                         , 11 ), // #466
  INST(Vexth_q_d        , LsxVV              , (0b0111001010011110111011, 0, 5, 0)                                                   , 0         , 0                         , 12 ), // #467
  INST(Vexth_qu_du      , LsxVV              , (0b0111001010011110111111, 0, 5, 0)                                                   , 0         , 0                         , 13 ), // #468
  INST(Vexth_w_h        , LsxVV              , (0b0111001010011110111001, 0, 5, 0)                                                   , 0         , 0                         , 14 ), // #469
  INST(Vexth_wu_hu      , LsxVV              , (0b0111001010011110111101, 0, 5, 0)                                                   , 0         , 0                         , 15 ), // #470
  INST(Vextl_q_d        , LsxVV              , (0b0111001100001001000000, 0, 5, 0)                                                   , 0         , 0                         , 16 ), // #471
  INST(Vextl_qu_du      , LsxVV              , (0b0111001100001101000000, 0, 5, 0)                                                   , 0         , 0                         , 17 ), // #472
  INST(Vextrins_b       , LsxVVI             , (0b01110011100011, 0, 5, 10, 0)                                                       , 0         , 0                         , 20 ), // #473
  INST(Vextrins_d       , LsxVVI             , (0b01110011100000, 0, 5, 10, 0)                                                       , 0         , 0                         , 21 ), // #474
  INST(Vextrins_h       , LsxVVI             , (0b01110011100010, 0, 5, 10, 0)                                                       , 0         , 0                         , 22 ), // #475
  INST(Vextrins_w       , LsxVVI             , (0b01110011100001, 0, 5, 10, 0)                                                       , 0         , 0                         , 23 ), // #476
  INST(Vfadd_d          , LsxVVV             , (0b01110001001100010, 0, 5, 10, 0)                                                    , 0         , 0                         , 79 ), // #477
  INST(Vfadd_s          , LsxVVV             , (0b01110001001100001, 0, 5, 10, 0)                                                    , 0         , 0                         , 80 ), // #478
  INST(Vfclass_d        , LsxVV              , (0b0111001010011100110110, 0, 5, 0)                                                   , 0         , 0                         , 18 ), // #479
  INST(Vfclass_s        , LsxVV              , (0b0111001010011100110101, 0, 5, 0)                                                   , 0         , 0                         , 19 ), // #480
  INST(Vfcvt_h_s        , LsxVVV             , (0b01110001010001100, 0, 5, 10, 0)                                                    , 0         , 0                         , 81 ), // #481
  INST(Vfcvt_s_d        , LsxVVV             , (0b01110001010001101, 0, 5, 10, 0)                                                    , 0         , 0                         , 82 ), // #482
  INST(Vfcvth_d_s       , LsxVV              , (0b0111001010011101111101, 0, 5, 0)                                                   , 0         , 0                         , 20 ), // #483
  INST(Vfcvth_s_h       , LsxVV              , (0b0111001010011101111011, 0, 5, 0)                                                   , 0         , 0                         , 21 ), // #484
  INST(Vfcvtl_d_s       , LsxVV              , (0b0111001010011101111100, 0, 5, 0)                                                   , 0         , 0                         , 22 ), // #485
  INST(Vfcvtl_s_h       , LsxVV              , (0b0111001010011101111010, 0, 5, 0)                                                   , 0         , 0                         , 23 ), // #486
  INST(Vfdiv_d          , LsxVVV             , (0b01110001001110110, 0, 5, 10, 0)                                                    , 0         , 0                         , 83 ), // #487
  INST(Vfdiv_s          , LsxVVV             , (0b01110001001110101, 0, 5, 10, 0)                                                    , 0         , 0                         , 84 ), // #488
  INST(Vffint_d_l       , LsxVV              , (0b0111001010011110000010, 0, 5, 0)                                                   , 0         , 0                         , 24 ), // #489
  INST(Vffint_d_lu      , LsxVV              , (0b0111001010011110000011, 0, 5, 0)                                                   , 0         , 0                         , 25 ), // #490
  INST(Vffint_s_l       , LsxVVV             , (0b01110001010010000, 0, 5, 10, 0)                                                    , 0         , 0                         , 85 ), // #491
  INST(Vffint_s_w       , LsxVV              , (0b0111001010011110000000, 0, 5, 0)                                                   , 0         , 0                         , 26 ), // #492
  INST(Vffint_s_wu      , LsxVV              , (0b0111001010011110000001, 0, 5, 0)                                                   , 0         , 0                         , 27 ), // #493
  INST(Vffinth_d_w      , LsxVV              , (0b0111001010011110000101, 0, 5, 0)                                                   , 0         , 0                         , 28 ), // #494
  INST(Vffintl_d_w      , LsxVV              , (0b0111001010011110000100, 0, 5, 0)                                                   , 0         , 0                         , 29 ), // #495
  INST(Vflogb_d         , LsxVV              , (0b0111001010011100110010, 0, 5, 0)                                                   , 0         , 0                         , 30 ), // #496
  INST(Vflogb_s         , LsxVV              , (0b0111001010011100110001, 0, 5, 0)                                                   , 0         , 0                         , 31 ), // #497
  INST(Vfmadd_d         , LsxVVVV            , (0b000010010010, 0, 5, 10, 15, 0)                                                     , 0         , 0                         , 1  ), // #498
  INST(Vfmadd_s         , LsxVVVV            , (0b000010010001, 0, 5, 10, 15, 0)                                                     , 0         , 0                         , 2  ), // #499
  INST(Vfmax_d          , LsxVVV             , (0b01110001001111010, 0, 5, 10, 0)                                                    , 0         , 0                         , 86 ), // #500
  INST(Vfmax_s          , LsxVVV             , (0b01110001001111001, 0, 5, 10, 0)                                                    , 0         , 0                         , 87 ), // #501
  INST(Vfmaxa_d         , LsxVVV             , (0b01110001010000010, 0, 5, 10, 0)                                                    , 0         , 0                         , 88 ), // #502
  INST(Vfmaxa_s         , LsxVVV             , (0b01110001010000001, 0, 5, 10, 0)                                                    , 0         , 0                         , 89 ), // #503
  INST(Vfmin_d          , LsxVVV             , (0b01110001001111110, 0, 5, 10, 0)                                                    , 0         , 0                         , 90 ), // #504
  INST(Vfmin_s          , LsxVVV             , (0b01110001001111101, 0, 5, 10, 0)                                                    , 0         , 0                         , 91 ), // #505
  INST(Vfmina_d         , LsxVVV             , (0b01110001010000110, 0, 5, 10, 0)                                                    , 0         , 0                         , 92 ), // #506
  INST(Vfmina_s         , LsxVVV             , (0b01110001010000101, 0, 5, 10, 0)                                                    , 0         , 0                         , 93 ), // #507
  INST(Vfmsub_d         , LsxVVVV            , (0b000010010110, 0, 5, 10, 15, 0)                                                     , 0         , 0                         , 3  ), // #508
  INST(Vfmsub_s         , LsxVVVV            , (0b000010010101, 0, 5, 10, 15, 0)                                                     , 0         , 0                         , 4  ), // #509
  INST(Vfmul_d          , LsxVVV             , (0b01110001001110010, 0, 5, 10, 0)                                                    , 0         , 0                         , 94 ), // #510
  INST(Vfmul_s          , LsxVVV             , (0b01110001001110001, 0, 5, 10, 0)                                                    , 0         , 0                         , 95 ), // #511
  INST(Vfnmadd_d        , LsxVVVV            , (0b000010011010, 0, 5, 10, 15, 0)                                                     , 0         , 0                         , 5  ), // #512
  INST(Vfnmadd_s        , LsxVVVV            , (0b000010011001, 0, 5, 10, 15, 0)                                                     , 0         , 0                         , 6  ), // #513
  INST(Vfnmsub_d        , LsxVVVV            , (0b000010011110, 0, 5, 10, 15, 0)                                                     , 0         , 0                         , 7  ), // #514
  INST(Vfnmsub_s        , LsxVVVV            , (0b000010011101, 0, 5, 10, 15, 0)                                                     , 0         , 0                         , 8  ), // #515
  INST(Vfrecip_d        , LsxVV              , (0b0111001010011100111110, 0, 5, 0)                                                   , 0         , 0                         , 32 ), // #516
  INST(Vfrecip_s        , LsxVV              , (0b0111001010011100111101, 0, 5, 0)                                                   , 0         , 0                         , 33 ), // #517
  INST(Vfrint_d         , LsxVV              , (0b0111001010011101001110, 0, 5, 0)                                                   , 0         , 0                         , 34 ), // #518
  INST(Vfrint_s         , LsxVV              , (0b0111001010011101001101, 0, 5, 0)                                                   , 0         , 0                         , 35 ), // #519
  INST(Vfrintrm_d       , LsxVV              , (0b0111001010011101010010, 0, 5, 0)                                                   , 0         , 0                         , 36 ), // #520
  INST(Vfrintrm_s       , LsxVV              , (0b0111001010011101010001, 0, 5, 0)                                                   , 0         , 0                         , 37 ), // #521
  INST(Vfrintrne_d      , LsxVV              , (0b0111001010011101011110, 0, 5, 0)                                                   , 0         , 0                         , 38 ), // #522
  INST(Vfrintrne_s      , LsxVV              , (0b0111001010011101011101, 0, 5, 0)                                                   , 0         , 0                         , 39 ), // #523
  INST(Vfrintrp_d       , LsxVV              , (0b0111001010011101010110, 0, 5, 0)                                                   , 0         , 0                         , 40 ), // #524
  INST(Vfrintrp_s       , LsxVV              , (0b0111001010011101010101, 0, 5, 0)                                                   , 0         , 0                         , 41 ), // #525
  INST(Vfrintrz_d       , LsxVV              , (0b0111001010011101011010, 0, 5, 0)                                                   , 0         , 0                         , 42 ), // #526
  INST(Vfrintrz_s       , LsxVV              , (0b0111001010011101011001, 0, 5, 0)                                                   , 0         , 0                         , 43 ), // #527
  INST(Vfrsqrt_d        , LsxVV              , (0b0111001010011101000010, 0, 5, 0)                                                   , 0         , 0                         , 44 ), // #528
  INST(Vfrsqrt_s        , LsxVV              , (0b0111001010011101000001, 0, 5, 0)                                                   , 0         , 0                         , 45 ), // #529
  INST(Vfrstp_b         , LsxVVV             , (0b01110001001010110, 0, 5, 10, 0)                                                    , 0         , 0                         , 96 ), // #530
  INST(Vfrstp_h         , LsxVVV             , (0b01110001001010111, 0, 5, 10, 0)                                                    , 0         , 0                         , 97 ), // #531
  INST(Vfrstpi_b        , LsxVVI             , (0b01110010100110100, 0, 5, 10, 0)                                                    , 0         , 0                         , 24 ), // #532
  INST(Vfrstpi_h        , LsxVVI             , (0b01110010100110101, 0, 5, 10, 0)                                                    , 0         , 0                         , 25 ), // #533
  INST(Vfscaleb_d       , LsxVVV             , (0b01110001010001010, 0, 5, 10, 0)                                                    , 0         , 0                         , 98 ), // #534
  INST(Vfscaleb_s       , LsxVVV             , (0b01110001010001001, 0, 5, 10, 0)                                                    , 0         , 0                         , 99 ), // #535
  INST(Vfsqrt_d         , LsxVV              , (0b0111001010011100111010, 0, 5, 0)                                                   , 0         , 0                         , 46 ), // #536
  INST(Vfsqrt_s         , LsxVV              , (0b0111001010011100111001, 0, 5, 0)                                                   , 0         , 0                         , 47 ), // #537
  INST(Vfsub_d          , LsxVVV             , (0b01110001001100110, 0, 5, 10, 0)                                                    , 0         , 0                         , 100), // #538
  INST(Vfsub_s          , LsxVVV             , (0b01110001001100101, 0, 5, 10, 0)                                                    , 0         , 0                         , 101), // #539
  INST(Vftint_l_d       , LsxVV              , (0b0111001010011110001101, 0, 5, 0)                                                   , 0         , 0                         , 48 ), // #540
  INST(Vftint_lu_d      , LsxVV              , (0b0111001010011110010111, 0, 5, 0)                                                   , 0         , 0                         , 49 ), // #541
  INST(Vftint_w_d       , LsxVVV             , (0b01110001010010011, 0, 5, 10, 0)                                                    , 0         , 0                         , 102), // #542
  INST(Vftint_w_s       , LsxVV              , (0b0111001010011110001100, 0, 5, 0)                                                   , 0         , 0                         , 50 ), // #543
  INST(Vftint_wu_s      , LsxVV              , (0b0111001010011110010110, 0, 5, 0)                                                   , 0         , 0                         , 51 ), // #544
  INST(Vftinth_l_s      , LsxVV              , (0b0111001010011110100001, 0, 5, 0)                                                   , 0         , 0                         , 52 ), // #545
  INST(Vftintl_l_s      , LsxVV              , (0b0111001010011110100000, 0, 5, 0)                                                   , 0         , 0                         , 53 ), // #546
  INST(Vftintrm_l_d     , LsxVV              , (0b0111001010011110001111, 0, 5, 0)                                                   , 0         , 0                         , 54 ), // #547
  INST(Vftintrm_w_d     , LsxVVV             , (0b01110001010010100, 0, 5, 10, 0)                                                    , 0         , 0                         , 103), // #548
  INST(Vftintrm_w_s     , LsxVV              , (0b0111001010011110001110, 0, 5, 0)                                                   , 0         , 0                         , 55 ), // #549
  INST(Vftintrmh_l_s    , LsxVV              , (0b0111001010011110100011, 0, 5, 0)                                                   , 0         , 0                         , 56 ), // #550
  INST(Vftintrml_l_s    , LsxVV              , (0b0111001010011110100010, 0, 5, 0)                                                   , 0         , 0                         , 57 ), // #551
  INST(Vftintrne_l_d    , LsxVV              , (0b0111001010011110010101, 0, 5, 0)                                                   , 0         , 0                         , 58 ), // #552
  INST(Vftintrne_w_d    , LsxVVV             , (0b01110001010010111, 0, 5, 10, 0)                                                    , 0         , 0                         , 104), // #553
  INST(Vftintrne_w_s    , LsxVV              , (0b0111001010011110010100, 0, 5, 0)                                                   , 0         , 0                         , 59 ), // #554
  INST(Vftintrneh_l_s   , LsxVV              , (0b0111001010011110101001, 0, 5, 0)                                                   , 0         , 0                         , 60 ), // #555
  INST(Vftintrnel_l_s   , LsxVV              , (0b0111001010011110101000, 0, 5, 0)                                                   , 0         , 0                         , 61 ), // #556
  INST(Vftintrp_l_d     , LsxVV              , (0b0111001010011110010001, 0, 5, 0)                                                   , 0         , 0                         , 62 ), // #557
  INST(Vftintrp_w_d     , LsxVVV             , (0b01110001010010101, 0, 5, 10, 0)                                                    , 0         , 0                         , 105), // #558
  INST(Vftintrp_w_s     , LsxVV              , (0b0111001010011110010000, 0, 5, 0)                                                   , 0         , 0                         , 63 ), // #559
  INST(Vftintrph_l_s    , LsxVV              , (0b0111001010011110100101, 0, 5, 0)                                                   , 0         , 0                         , 64 ), // #560
  INST(Vftintrpl_l_s    , LsxVV              , (0b0111001010011110100100, 0, 5, 0)                                                   , 0         , 0                         , 65 ), // #561
  INST(Vftintrz_l_d     , LsxVV              , (0b0111001010011110010011, 0, 5, 0)                                                   , 0         , 0                         , 66 ), // #562
  INST(Vftintrz_lu_d    , LsxVV              , (0b0111001010011110011101, 0, 5, 0)                                                   , 0         , 0                         , 67 ), // #563
  INST(Vftintrz_w_d     , LsxVVV             , (0b01110001010010110, 0, 5, 10, 0)                                                    , 0         , 0                         , 106), // #564
  INST(Vftintrz_w_s     , LsxVV              , (0b0111001010011110010010, 0, 5, 0)                                                   , 0         , 0                         , 68 ), // #565
  INST(Vftintrz_wu_s    , LsxVV              , (0b0111001010011110011100, 0, 5, 0)                                                   , 0         , 0                         , 69 ), // #566
  INST(Vftintrzh_l_s    , LsxVV              , (0b0111001010011110100111, 0, 5, 0)                                                   , 0         , 0                         , 70 ), // #567
  INST(Vftintrzl_l_s    , LsxVV              , (0b0111001010011110100110, 0, 5, 0)                                                   , 0         , 0                         , 71 ), // #568
  INST(Vhaddw_d_w       , LsxVVV             , (0b01110000010101010, 0, 5, 10, 0)                                                    , 0         , 0                         , 107), // #569
  INST(Vhaddw_du_wu     , LsxVVV             , (0b01110000010110010, 0, 5, 10, 0)                                                    , 0         , 0                         , 108), // #570
  INST(Vhaddw_h_b       , LsxVVV             , (0b01110000010101000, 0, 5, 10, 0)                                                    , 0         , 0                         , 109), // #571
  INST(Vhaddw_hu_bu     , LsxVVV             , (0b01110000010110000, 0, 5, 10, 0)                                                    , 0         , 0                         , 110), // #572
  INST(Vhaddw_q_d       , LsxVVV             , (0b01110000010101011, 0, 5, 10, 0)                                                    , 0         , 0                         , 111), // #573
  INST(Vhaddw_qu_du     , LsxVVV             , (0b01110000010110011, 0, 5, 10, 0)                                                    , 0         , 0                         , 112), // #574
  INST(Vhaddw_w_h       , LsxVVV             , (0b01110000010101001, 0, 5, 10, 0)                                                    , 0         , 0                         , 113), // #575
  INST(Vhaddw_wu_hu     , LsxVVV             , (0b01110000010110001, 0, 5, 10, 0)                                                    , 0         , 0                         , 114), // #576
  INST(Vhsubw_d_w       , LsxVVV             , (0b01110000010101110, 0, 5, 10, 0)                                                    , 0         , 0                         , 115), // #577
  INST(Vhsubw_du_wu     , LsxVVV             , (0b01110000010110110, 0, 5, 10, 0)                                                    , 0         , 0                         , 116), // #578
  INST(Vhsubw_h_b       , LsxVVV             , (0b01110000010101100, 0, 5, 10, 0)                                                    , 0         , 0                         , 117), // #579
  INST(Vhsubw_hu_bu     , LsxVVV             , (0b01110000010110100, 0, 5, 10, 0)                                                    , 0         , 0                         , 118), // #580
  INST(Vhsubw_q_d       , LsxVVV             , (0b01110000010101111, 0, 5, 10, 0)                                                    , 0         , 0                         , 119), // #581
  INST(Vhsubw_qu_du     , LsxVVV             , (0b01110000010110111, 0, 5, 10, 0)                                                    , 0         , 0                         , 120), // #582
  INST(Vhsubw_w_h       , LsxVVV             , (0b01110000010101101, 0, 5, 10, 0)                                                    , 0         , 0                         , 121), // #583
  INST(Vhsubw_wu_hu     , LsxVVV             , (0b01110000010110101, 0, 5, 10, 0)                                                    , 0         , 0                         , 122), // #584
  INST(Vilvh_b          , LsxVVV             , (0b01110001000111000, 0, 5, 10, 0)                                                    , 0         , 0                         , 123), // #585
  INST(Vilvh_d          , LsxVVV             , (0b01110001000111011, 0, 5, 10, 0)                                                    , 0         , 0                         , 124), // #586
  INST(Vilvh_h          , LsxVVV             , (0b01110001000111001, 0, 5, 10, 0)                                                    , 0         , 0                         , 125), // #587
  INST(Vilvh_w          , LsxVVV             , (0b01110001000111010, 0, 5, 10, 0)                                                    , 0         , 0                         , 126), // #588
  INST(Vilvl_b          , LsxVVV             , (0b01110001000110100, 0, 5, 10, 0)                                                    , 0         , 0                         , 127), // #589
  INST(Vilvl_d          , LsxVVV             , (0b01110001000110111, 0, 5, 10, 0)                                                    , 0         , 0                         , 128), // #590
  INST(Vilvl_h          , LsxVVV             , (0b01110001000110101, 0, 5, 10, 0)                                                    , 0         , 0                         , 129), // #591
  INST(Vilvl_w          , LsxVVV             , (0b01110001000110110, 0, 5, 10, 0)                                                    , 0         , 0                         , 130), // #592
  INST(Vinsgr2vr_b      , LsxVRI             , (0b011100101110101110, 0, kWX, 5, 10, 0)                                              , 0         , 0                         , 0  ), // #593
  INST(Vinsgr2vr_d      , LsxVRI             , (0b011100101110101111110, 0, kWX, 5, 10, 0)                                           , 0         , 0                         , 1  ), // #594
  INST(Vinsgr2vr_h      , LsxVRI             , (0b0111001011101011110, 0, kWX, 5, 10, 0)                                             , 0         , 0                         , 2  ), // #595
  INST(Vinsgr2vr_w      , LsxVRI             , (0b01110010111010111110, 0, kWX, 5, 10, 0)                                            , 0         , 0                         , 3  ), // #596
  INST(Vld              , SimdLdst           , (0b0010110000,        22, 12)                                                         , 0         , 0                         , 0  ), // #597
  INST(Vldi             , LsxVI              , (0b01110011111000, 0, 5, 0)                                                           , 0         , 0                         , 0  ), // #598
  INST(Vldrepl_b        , SimdLdst           , (0b0011000010,        22, 12)                                                         , 0         , 0                         , 1  ), // #599
  INST(Vldrepl_d        , SimdLdst           , (0b0011000000010,     19, 9 )                                                         , 0         , 0                         , 2  ), // #600
  INST(Vldrepl_h        , SimdLdst           , (0b00110000010,       21, 11)                                                         , 0         , 0                         , 3  ), // #601
  INST(Vldrepl_w        , SimdLdst           , (0b001100000010,      20, 10)                                                         , 0         , 0                         , 4  ), // #602
  INST(Vldx             , SimdLdst           , (0b00111000010000000, 15, 0 )                                                         , 0         , 0                         , 5  ), // #603
  INST(Vmadd_b          , LsxVVV             , (0b01110000101010000, 0, 5, 10, 0)                                                    , 0         , 0                         , 131), // #604
  INST(Vmadd_d          , LsxVVV             , (0b01110000101010011, 0, 5, 10, 0)                                                    , 0         , 0                         , 132), // #605
  INST(Vmadd_h          , LsxVVV             , (0b01110000101010001, 0, 5, 10, 0)                                                    , 0         , 0                         , 133), // #606
  INST(Vmadd_w          , LsxVVV             , (0b01110000101010010, 0, 5, 10, 0)                                                    , 0         , 0                         , 134), // #607
  INST(Vmaddwev_d_w     , LsxVVV             , (0b01110000101011010, 0, 5, 10, 0)                                                    , 0         , 0                         , 135), // #608
  INST(Vmaddwev_d_wu    , LsxVVV             , (0b01110000101101010, 0, 5, 10, 0)                                                    , 0         , 0                         , 136), // #609
  INST(Vmaddwev_d_wu_w  , LsxVVV             , (0b01110000101111010, 0, 5, 10, 0)                                                    , 0         , 0                         , 137), // #610
  INST(Vmaddwev_h_b     , LsxVVV             , (0b01110000101011000, 0, 5, 10, 0)                                                    , 0         , 0                         , 138), // #611
  INST(Vmaddwev_h_bu    , LsxVVV             , (0b01110000101101000, 0, 5, 10, 0)                                                    , 0         , 0                         , 139), // #612
  INST(Vmaddwev_h_bu_b  , LsxVVV             , (0b01110000101111000, 0, 5, 10, 0)                                                    , 0         , 0                         , 140), // #613
  INST(Vmaddwev_q_d     , LsxVVV             , (0b01110000101011011, 0, 5, 10, 0)                                                    , 0         , 0                         , 141), // #614
  INST(Vmaddwev_q_du    , LsxVVV             , (0b01110000101101011, 0, 5, 10, 0)                                                    , 0         , 0                         , 142), // #615
  INST(Vmaddwev_q_du_d  , LsxVVV             , (0b01110000101111011, 0, 5, 10, 0)                                                    , 0         , 0                         , 143), // #616
  INST(Vmaddwev_w_h     , LsxVVV             , (0b01110000101011001, 0, 5, 10, 0)                                                    , 0         , 0                         , 144), // #617
  INST(Vmaddwev_w_hu    , LsxVVV             , (0b01110000101101001, 0, 5, 10, 0)                                                    , 0         , 0                         , 145), // #618
  INST(Vmaddwev_w_hu_h  , LsxVVV             , (0b01110000101111001, 0, 5, 10, 0)                                                    , 0         , 0                         , 146), // #619
  INST(Vmaddwod_d_w     , LsxVVV             , (0b01110000101011110, 0, 5, 10, 0)                                                    , 0         , 0                         , 147), // #620
  INST(Vmaddwod_d_wu    , LsxVVV             , (0b01110000101101110, 0, 5, 10, 0)                                                    , 0         , 0                         , 148), // #621
  INST(Vmaddwod_d_wu_w  , LsxVVV             , (0b01110000101111110, 0, 5, 10, 0)                                                    , 0         , 0                         , 149), // #622
  INST(Vmaddwod_h_b     , LsxVVV             , (0b01110000101011100, 0, 5, 10, 0)                                                    , 0         , 0                         , 150), // #623
  INST(Vmaddwod_h_bu    , LsxVVV             , (0b01110000101101100, 0, 5, 10, 0)                                                    , 0         , 0                         , 151), // #624
  INST(Vmaddwod_h_bu_b  , LsxVVV             , (0b01110000101111100, 0, 5, 10, 0)                                                    , 0         , 0                         , 152), // #625
  INST(Vmaddwod_q_d     , LsxVVV             , (0b01110000101011111, 0, 5, 10, 0)                                                    , 0         , 0                         , 153), // #626
  INST(Vmaddwod_q_du    , LsxVVV             , (0b01110000101101111, 0, 5, 10, 0)                                                    , 0         , 0                         , 154), // #627
  INST(Vmaddwod_q_du_d  , LsxVVV             , (0b01110000101111111, 0, 5, 10, 0)                                                    , 0         , 0                         , 155), // #628
  INST(Vmaddwod_w_h     , LsxVVV             , (0b01110000101011101, 0, 5, 10, 0)                                                    , 0         , 0                         , 156), // #629
  INST(Vmaddwod_w_hu    , LsxVVV             , (0b01110000101101101, 0, 5, 10, 0)                                                    , 0         , 0                         , 157), // #630
  INST(Vmaddwod_w_hu_h  , LsxVVV             , (0b01110000101111101, 0, 5, 10, 0)                                                    , 0         , 0                         , 158), // #631
  INST(Vmax_b           , LsxVVV             , (0b01110000011100000, 0, 5, 10, 0)                                                    , 0         , 0                         , 159), // #632
  INST(Vmax_bu          , LsxVVV             , (0b01110000011101000, 0, 5, 10, 0)                                                    , 0         , 0                         , 160), // #633
  INST(Vmax_d           , LsxVVV             , (0b01110000011100011, 0, 5, 10, 0)                                                    , 0         , 0                         , 161), // #634
  INST(Vmax_du          , LsxVVV             , (0b01110000011101011, 0, 5, 10, 0)                                                    , 0         , 0                         , 162), // #635
  INST(Vmax_h           , LsxVVV             , (0b01110000011100001, 0, 5, 10, 0)                                                    , 0         , 0                         , 163), // #636
  INST(Vmax_hu          , LsxVVV             , (0b01110000011101001, 0, 5, 10, 0)                                                    , 0         , 0                         , 164), // #637
  INST(Vmax_w           , LsxVVV             , (0b01110000011100010, 0, 5, 10, 0)                                                    , 0         , 0                         , 165), // #638
  INST(Vmax_wu          , LsxVVV             , (0b01110000011101010, 0, 5, 10, 0)                                                    , 0         , 0                         , 166), // #639
  INST(Vmaxi_b          , LsxVVI             , (0b01110010100100000, 0, 5, 10, 0)                                                    , 0         , 0                         , 26 ), // #640
  INST(Vmaxi_bu         , LsxVVI             , (0b01110010100101000, 0, 5, 10, 0)                                                    , 0         , 0                         , 27 ), // #641
  INST(Vmaxi_d          , LsxVVI             , (0b01110010100100011, 0, 5, 10, 0)                                                    , 0         , 0                         , 28 ), // #642
  INST(Vmaxi_du         , LsxVVI             , (0b01110010100101011, 0, 5, 10, 0)                                                    , 0         , 0                         , 29 ), // #643
  INST(Vmaxi_h          , LsxVVI             , (0b01110010100100001, 0, 5, 10, 0)                                                    , 0         , 0                         , 30 ), // #644
  INST(Vmaxi_hu         , LsxVVI             , (0b01110010100101001, 0, 5, 10, 0)                                                    , 0         , 0                         , 31 ), // #645
  INST(Vmaxi_w          , LsxVVI             , (0b01110010100100010, 0, 5, 10, 0)                                                    , 0         , 0                         , 32 ), // #646
  INST(Vmaxi_wu         , LsxVVI             , (0b01110010100101010, 0, 5, 10, 0)                                                    , 0         , 0                         , 33 ), // #647
  INST(Vmepatmsk_v      , LsxVII             , (0b01110010100110111, 0, 5, 10, 0)                                                    , 0         , 0                         , 0  ), // #648
  INST(Vmin_b           , LsxVVV             , (0b01110000011100100, 0, 5, 10, 0)                                                    , 0         , 0                         , 167), // #649
  INST(Vmin_bu          , LsxVVV             , (0b01110000011101100, 0, 5, 10, 0)                                                    , 0         , 0                         , 168), // #650
  INST(Vmin_d           , LsxVVV             , (0b01110000011100111, 0, 5, 10, 0)                                                    , 0         , 0                         , 169), // #651
  INST(Vmin_du          , LsxVVV             , (0b01110000011101111, 0, 5, 10, 0)                                                    , 0         , 0                         , 170), // #652
  INST(Vmin_h           , LsxVVV             , (0b01110000011100101, 0, 5, 10, 0)                                                    , 0         , 0                         , 171), // #653
  INST(Vmin_hu          , LsxVVV             , (0b01110000011101101, 0, 5, 10, 0)                                                    , 0         , 0                         , 172), // #654
  INST(Vmin_w           , LsxVVV             , (0b01110000011100110, 0, 5, 10, 0)                                                    , 0         , 0                         , 173), // #655
  INST(Vmin_wu          , LsxVVV             , (0b01110000011101110, 0, 5, 10, 0)                                                    , 0         , 0                         , 174), // #656
  INST(Vmini_b          , LsxVVI             , (0b01110010100100100, 0, 5, 10, 0)                                                    , 0         , 0                         , 34 ), // #657
  INST(Vmini_bu         , LsxVVI             , (0b01110010100101100, 0, 5, 10, 0)                                                    , 0         , 0                         , 35 ), // #658
  INST(Vmini_d          , LsxVVI             , (0b01110010100100111, 0, 5, 10, 0)                                                    , 0         , 0                         , 36 ), // #659
  INST(Vmini_du         , LsxVVI             , (0b01110010100101111, 0, 5, 10, 0)                                                    , 0         , 0                         , 37 ), // #660
  INST(Vmini_h          , LsxVVI             , (0b01110010100100101, 0, 5, 10, 0)                                                    , 0         , 0                         , 38 ), // #661
  INST(Vmini_hu         , LsxVVI             , (0b01110010100101101, 0, 5, 10, 0)                                                    , 0         , 0                         , 39 ), // #662
  INST(Vmini_w          , LsxVVI             , (0b01110010100100110, 0, 5, 10, 0)                                                    , 0         , 0                         , 40 ), // #663
  INST(Vmini_wu         , LsxVVI             , (0b01110010100101110, 0, 5, 10, 0)                                                    , 0         , 0                         , 41 ), // #664
  INST(Vmod_b           , LsxVVV             , (0b01110000111000100, 0, 5, 10, 0)                                                    , 0         , 0                         , 175), // #665
  INST(Vmod_bu          , LsxVVV             , (0b01110000111001100, 0, 5, 10, 0)                                                    , 0         , 0                         , 176), // #666
  INST(Vmod_d           , LsxVVV             , (0b01110000111000111, 0, 5, 10, 0)                                                    , 0         , 0                         , 177), // #667
  INST(Vmod_du          , LsxVVV             , (0b01110000111001111, 0, 5, 10, 0)                                                    , 0         , 0                         , 178), // #668
  INST(Vmod_h           , LsxVVV             , (0b01110000111000101, 0, 5, 10, 0)                                                    , 0         , 0                         , 179), // #669
  INST(Vmod_hu          , LsxVVV             , (0b01110000111001101, 0, 5, 10, 0)                                                    , 0         , 0                         , 180), // #670
  INST(Vmod_w           , LsxVVV             , (0b01110000111000110, 0, 5, 10, 0)                                                    , 0         , 0                         , 181), // #671
  INST(Vmod_wu          , LsxVVV             , (0b01110000111001110, 0, 5, 10, 0)                                                    , 0         , 0                         , 182), // #672
  INST(Vmskgez_b        , LsxVV              , (0b0111001010011100010100, 0, 5, 0)                                                   , 0         , 0                         , 72 ), // #673
  INST(Vmskltz_b        , LsxVV              , (0b0111001010011100010000, 0, 5, 0)                                                   , 0         , 0                         , 73 ), // #674
  INST(Vmskltz_d        , LsxVV              , (0b0111001010011100010011, 0, 5, 0)                                                   , 0         , 0                         , 74 ), // #675
  INST(Vmskltz_h        , LsxVV              , (0b0111001010011100010001, 0, 5, 0)                                                   , 0         , 0                         , 75 ), // #676
  INST(Vmskltz_w        , LsxVV              , (0b0111001010011100010010, 0, 5, 0)                                                   , 0         , 0                         , 76 ), // #677
  INST(Vmsknz_b         , LsxVV              , (0b0111001010011100011000, 0, 5, 0)                                                   , 0         , 0                         , 77 ), // #678
  INST(Vmsub_b          , LsxVVV             , (0b01110000101010100, 0, 5, 10, 0)                                                    , 0         , 0                         , 183), // #679
  INST(Vmsub_d          , LsxVVV             , (0b01110000101010111, 0, 5, 10, 0)                                                    , 0         , 0                         , 184), // #680
  INST(Vmsub_h          , LsxVVV             , (0b01110000101010101, 0, 5, 10, 0)                                                    , 0         , 0                         , 185), // #681
  INST(Vmsub_w          , LsxVVV             , (0b01110000101010110, 0, 5, 10, 0)                                                    , 0         , 0                         , 186), // #682
  INST(Vmuh_b           , LsxVVV             , (0b01110000100001100, 0, 5, 10, 0)                                                    , 0         , 0                         , 187), // #683
  INST(Vmuh_bu          , LsxVVV             , (0b01110000100010000, 0, 5, 10, 0)                                                    , 0         , 0                         , 188), // #684
  INST(Vmuh_d           , LsxVVV             , (0b01110000100001111, 0, 5, 10, 0)                                                    , 0         , 0                         , 189), // #685
  INST(Vmuh_du          , LsxVVV             , (0b01110000100010011, 0, 5, 10, 0)                                                    , 0         , 0                         , 190), // #686
  INST(Vmuh_h           , LsxVVV             , (0b01110000100001101, 0, 5, 10, 0)                                                    , 0         , 0                         , 191), // #687
  INST(Vmuh_hu          , LsxVVV             , (0b01110000100010001, 0, 5, 10, 0)                                                    , 0         , 0                         , 192), // #688
  INST(Vmuh_w           , LsxVVV             , (0b01110000100001110, 0, 5, 10, 0)                                                    , 0         , 0                         , 193), // #689
  INST(Vmuh_wu          , LsxVVV             , (0b01110000100010010, 0, 5, 10, 0)                                                    , 0         , 0                         , 194), // #690
  INST(Vmul_b           , LsxVVV             , (0b01110000100001000, 0, 5, 10, 0)                                                    , 0         , 0                         , 195), // #691
  INST(Vmul_d           , LsxVVV             , (0b01110000100001011, 0, 5, 10, 0)                                                    , 0         , 0                         , 196), // #692
  INST(Vmul_h           , LsxVVV             , (0b01110000100001001, 0, 5, 10, 0)                                                    , 0         , 0                         , 197), // #693
  INST(Vmul_w           , LsxVVV             , (0b01110000100001010, 0, 5, 10, 0)                                                    , 0         , 0                         , 198), // #694
  INST(Vmulwev_d_w      , LsxVVV             , (0b01110000100100010, 0, 5, 10, 0)                                                    , 0         , 0                         , 199), // #695
  INST(Vmulwev_d_wu     , LsxVVV             , (0b01110000100110010, 0, 5, 10, 0)                                                    , 0         , 0                         , 200), // #696
  INST(Vmulwev_d_wu_w   , LsxVVV             , (0b01110000101000010, 0, 5, 10, 0)                                                    , 0         , 0                         , 201), // #697
  INST(Vmulwev_h_b      , LsxVVV             , (0b01110000100100000, 0, 5, 10, 0)                                                    , 0         , 0                         , 202), // #698
  INST(Vmulwev_h_bu     , LsxVVV             , (0b01110000100110000, 0, 5, 10, 0)                                                    , 0         , 0                         , 203), // #699
  INST(Vmulwev_h_bu_b   , LsxVVV             , (0b01110000101000000, 0, 5, 10, 0)                                                    , 0         , 0                         , 204), // #700
  INST(Vmulwev_q_d      , LsxVVV             , (0b01110000100100011, 0, 5, 10, 0)                                                    , 0         , 0                         , 205), // #701
  INST(Vmulwev_q_du     , LsxVVV             , (0b01110000100110011, 0, 5, 10, 0)                                                    , 0         , 0                         , 206), // #702
  INST(Vmulwev_q_du_d   , LsxVVV             , (0b01110000101000011, 0, 5, 10, 0)                                                    , 0         , 0                         , 207), // #703
  INST(Vmulwev_w_h      , LsxVVV             , (0b01110000100100001, 0, 5, 10, 0)                                                    , 0         , 0                         , 208), // #704
  INST(Vmulwev_w_hu     , LsxVVV             , (0b01110000100110001, 0, 5, 10, 0)                                                    , 0         , 0                         , 209), // #705
  INST(Vmulwev_w_hu_h   , LsxVVV             , (0b01110000101000001, 0, 5, 10, 0)                                                    , 0         , 0                         , 210), // #706
  INST(Vmulwod_d_w      , LsxVVV             , (0b01110000100100110, 0, 5, 10, 0)                                                    , 0         , 0                         , 211), // #707
  INST(Vmulwod_d_wu     , LsxVVV             , (0b01110000100110110, 0, 5, 10, 0)                                                    , 0         , 0                         , 212), // #708
  INST(Vmulwod_d_wu_w   , LsxVVV             , (0b01110000101000110, 0, 5, 10, 0)                                                    , 0         , 0                         , 213), // #709
  INST(Vmulwod_h_b      , LsxVVV             , (0b01110000100100100, 0, 5, 10, 0)                                                    , 0         , 0                         , 214), // #710
  INST(Vmulwod_h_bu     , LsxVVV             , (0b01110000100110100, 0, 5, 10, 0)                                                    , 0         , 0                         , 215), // #711
  INST(Vmulwod_h_bu_b   , LsxVVV             , (0b01110000101000100, 0, 5, 10, 0)                                                    , 0         , 0                         , 216), // #712
  INST(Vmulwod_q_d      , LsxVVV             , (0b01110000100100111, 0, 5, 10, 0)                                                    , 0         , 0                         , 217), // #713
  INST(Vmulwod_q_du     , LsxVVV             , (0b01110000100110111, 0, 5, 10, 0)                                                    , 0         , 0                         , 218), // #714
  INST(Vmulwod_q_du_d   , LsxVVV             , (0b01110000101000111, 0, 5, 10, 0)                                                    , 0         , 0                         , 219), // #715
  INST(Vmulwod_w_h      , LsxVVV             , (0b01110000100100101, 0, 5, 10, 0)                                                    , 0         , 0                         , 220), // #716
  INST(Vmulwod_w_hu     , LsxVVV             , (0b01110000100110101, 0, 5, 10, 0)                                                    , 0         , 0                         , 221), // #717
  INST(Vmulwod_w_hu_h   , LsxVVV             , (0b01110000101000101, 0, 5, 10, 0)                                                    , 0         , 0                         , 222), // #718
  INST(Vneg_b           , LsxVV              , (0b0111001010011100001100, 0, 5, 0)                                                   , 0         , 0                         , 78 ), // #719
  INST(Vneg_d           , LsxVV              , (0b0111001010011100001111, 0, 5, 0)                                                   , 0         , 0                         , 79 ), // #720
  INST(Vneg_h           , LsxVV              , (0b0111001010011100001101, 0, 5, 0)                                                   , 0         , 0                         , 80 ), // #721
  INST(Vneg_w           , LsxVV              , (0b0111001010011100001110, 0, 5, 0)                                                   , 0         , 0                         , 81 ), // #722
  INST(Vnor_v           , LsxVVV             , (0b01110001001001111, 0, 5, 10, 0)                                                    , 0         , 0                         , 223), // #723
  INST(Vnori_b          , LsxVVI             , (0b01110011110111, 0, 5, 10, 0)                                                       , 0         , 0                         , 42 ), // #724
  INST(Vor_v            , LsxVVV             , (0b01110001001001101, 0, 5, 10, 0)                                                    , 0         , 0                         , 224), // #725
  INST(Vori_b           , LsxVVI             , (0b01110011110101, 0, 5, 10, 0)                                                       , 0         , 0                         , 43 ), // #726
  INST(Vorn_v           , LsxVVV             , (0b01110001001010001, 0, 5, 10, 0)                                                    , 0         , 0                         , 225), // #727
  INST(Vpackev_b        , LsxVVV             , (0b01110001000101100, 0, 5, 10, 0)                                                    , 0         , 0                         , 226), // #728
  INST(Vpackev_d        , LsxVVV             , (0b01110001000101111, 0, 5, 10, 0)                                                    , 0         , 0                         , 227), // #729
  INST(Vpackev_h        , LsxVVV             , (0b01110001000101101, 0, 5, 10, 0)                                                    , 0         , 0                         , 228), // #730
  INST(Vpackev_w        , LsxVVV             , (0b01110001000101110, 0, 5, 10, 0)                                                    , 0         , 0                         , 229), // #731
  INST(Vpackod_b        , LsxVVV             , (0b01110001000110000, 0, 5, 10, 0)                                                    , 0         , 0                         , 230), // #732
  INST(Vpackod_d        , LsxVVV             , (0b01110001000110011, 0, 5, 10, 0)                                                    , 0         , 0                         , 231), // #733
  INST(Vpackod_h        , LsxVVV             , (0b01110001000110001, 0, 5, 10, 0)                                                    , 0         , 0                         , 232), // #734
  INST(Vpackod_w        , LsxVVV             , (0b01110001000110010, 0, 5, 10, 0)                                                    , 0         , 0                         , 233), // #735
  INST(Vpcnt_b          , LsxVV              , (0b0111001010011100001000, 0, 5, 0)                                                   , 0         , 0                         , 82 ), // #736
  INST(Vpcnt_d          , LsxVV              , (0b0111001010011100001011, 0, 5, 0)                                                   , 0         , 0                         , 83 ), // #737
  INST(Vpcnt_h          , LsxVV              , (0b0111001010011100001001, 0, 5, 0)                                                   , 0         , 0                         , 84 ), // #738
  INST(Vpcnt_w          , LsxVV              , (0b0111001010011100001010, 0, 5, 0)                                                   , 0         , 0                         , 85 ), // #739
  INST(Vpermi_w         , LsxVVI             , (0b01110011111001, 0, 5, 10, 0)                                                       , 0         , 0                         , 44 ), // #740
  INST(Vpickev_b        , LsxVVV             , (0b01110001000111100, 0, 5, 10, 0)                                                    , 0         , 0                         , 234), // #741
  INST(Vpickev_d        , LsxVVV             , (0b01110001000111111, 0, 5, 10, 0)                                                    , 0         , 0                         , 235), // #742
  INST(Vpickev_h        , LsxVVV             , (0b01110001000111101, 0, 5, 10, 0)                                                    , 0         , 0                         , 236), // #743
  INST(Vpickev_w        , LsxVVV             , (0b01110001000111110, 0, 5, 10, 0)                                                    , 0         , 0                         , 237), // #744
  INST(Vpickod_b        , LsxVVV             , (0b01110001001000000, 0, 5, 10, 0)                                                    , 0         , 0                         , 238), // #745
  INST(Vpickod_d        , LsxVVV             , (0b01110001001000011, 0, 5, 10, 0)                                                    , 0         , 0                         , 239), // #746
  INST(Vpickod_h        , LsxVVV             , (0b01110001001000001, 0, 5, 10, 0)                                                    , 0         , 0                         , 240), // #747
  INST(Vpickod_w        , LsxVVV             , (0b01110001001000010, 0, 5, 10, 0)                                                    , 0         , 0                         , 241), // #748
  INST(Vpickve2gr_b     , LsxRVI             , (0b011100101110111110, kWX, 0, 5, 10, 0)                                              , 0         , 0                         , 0  ), // #749
  INST(Vpickve2gr_bu    , LsxRVI             , (0b011100101111001110, kWX, 0, 5, 10, 0)                                              , 0         , 0                         , 1  ), // #750
  INST(Vpickve2gr_d     , LsxRVI             , (0b011100101110111111110, kWX, 0, 5, 10, 0)                                           , 0         , 0                         , 2  ), // #751
  INST(Vpickve2gr_du    , LsxRVI             , (0b011100101111001111110, kWX, 0, 5, 10, 0)                                           , 0         , 0                         , 3  ), // #752
  INST(Vpickve2gr_h     , LsxRVI             , (0b0111001011101111110, kWX, 0, 5, 10, 0)                                             , 0         , 0                         , 4  ), // #753
  INST(Vpickve2gr_hu    , LsxRVI             , (0b0111001011110011110, kWX, 0, 5, 10, 0)                                             , 0         , 0                         , 5  ), // #754
  INST(Vpickve2gr_w     , LsxRVI             , (0b01110010111011111110, kWX, 0, 5, 10, 0)                                            , 0         , 0                         , 6  ), // #755
  INST(Vpickve2gr_wu    , LsxRVI             , (0b01110010111100111110, kWX, 0, 5, 10, 0)                                            , 0         , 0                         , 7  ), // #756
  INST(Vreplgr2vr_b     , LsxVR              , (0b0111001010011111000000, 0, kWX, 5, 0)                                              , 0         , 0                         , 0  ), // #757
  INST(Vreplgr2vr_d     , LsxVR              , (0b0111001010011111000011, 0, kWX, 5, 0)                                              , 0         , 0                         , 1  ), // #758
  INST(Vreplgr2vr_h     , LsxVR              , (0b0111001010011111000001, 0, kWX, 5, 0)                                              , 0         , 0                         , 2  ), // #759
  INST(Vreplgr2vr_w     , LsxVR              , (0b0111001010011111000010, 0, kWX, 5, 0)                                              , 0         , 0                         , 3  ), // #760
  INST(Vreplve_b        , LsxVVR             , (0b01110001001000100, 0, 5, kWX, 10, 0)                                               , 0         , 0                         , 0  ), // #761
  INST(Vreplve_d        , LsxVVR             , (0b01110001001000111, 0, 5, kWX, 10, 0)                                               , 0         , 0                         , 1  ), // #762
  INST(Vreplve_h        , LsxVVR             , (0b01110001001000101, 0, 5, kWX, 10, 0)                                               , 0         , 0                         , 2  ), // #763
  INST(Vreplve_w        , LsxVVR             , (0b01110001001000110, 0, 5, kWX, 10, 0)                                               , 0         , 0                         , 3  ), // #764
  INST(Vreplvei_b       , LsxVVI             , (0b011100101111011110, 0, 5, 10, 0)                                                   , 0         , 0                         , 45 ), // #765
  INST(Vreplvei_d       , LsxVVI             , (0b011100101111011111110, 0, 5, 10, 0)                                                , 0         , 0                         , 46 ), // #766
  INST(Vreplvei_h       , LsxVVI             , (0b0111001011110111110, 0, 5, 10, 0)                                                  , 0         , 0                         , 47 ), // #767
  INST(Vreplvei_w       , LsxVVI             , (0b01110010111101111110, 0, 5, 10, 0)                                                 , 0         , 0                         , 48 ), // #768
  INST(Vrotr_b          , LsxVVV             , (0b01110000111011100, 0, 5, 10, 0)                                                    , 0         , 0                         , 242), // #769
  INST(Vrotr_d          , LsxVVV             , (0b01110000111011111, 0, 5, 10, 0)                                                    , 0         , 0                         , 243), // #770
  INST(Vrotr_h          , LsxVVV             , (0b01110000111011101, 0, 5, 10, 0)                                                    , 0         , 0                         , 244), // #771
  INST(Vrotr_w          , LsxVVV             , (0b01110000111011110, 0, 5, 10, 0)                                                    , 0         , 0                         , 245), // #772
  INST(Vrotri_b         , LsxVVI             , (0b0111001010100000001, 0, 5, 10, 0)                                                  , 0         , 0                         , 49 ), // #773
  INST(Vrotri_d         , LsxVVI             , (0b0111001010100001, 0, 5, 10, 0)                                                     , 0         , 0                         , 50 ), // #774
  INST(Vrotri_h         , LsxVVI             , (0b011100101010000001, 0, 5, 10, 0)                                                   , 0         , 0                         , 51 ), // #775
  INST(Vrotri_w         , LsxVVI             , (0b01110010101000001, 0, 5, 10, 0)                                                    , 0         , 0                         , 52 ), // #776
  INST(Vsadd_b          , LsxVVV             , (0b01110000010001100, 0, 5, 10, 0)                                                    , 0         , 0                         , 246), // #777
  INST(Vsadd_bu         , LsxVVV             , (0b01110000010010100, 0, 5, 10, 0)                                                    , 0         , 0                         , 247), // #778
  INST(Vsadd_d          , LsxVVV             , (0b01110000010001111, 0, 5, 10, 0)                                                    , 0         , 0                         , 248), // #779
  INST(Vsadd_du         , LsxVVV             , (0b01110000010010111, 0, 5, 10, 0)                                                    , 0         , 0                         , 249), // #780
  INST(Vsadd_h          , LsxVVV             , (0b01110000010001101, 0, 5, 10, 0)                                                    , 0         , 0                         , 250), // #781
  INST(Vsadd_hu         , LsxVVV             , (0b01110000010010101, 0, 5, 10, 0)                                                    , 0         , 0                         , 251), // #782
  INST(Vsadd_w          , LsxVVV             , (0b01110000010001110, 0, 5, 10, 0)                                                    , 0         , 0                         , 252), // #783
  INST(Vsadd_wu         , LsxVVV             , (0b01110000010010110, 0, 5, 10, 0)                                                    , 0         , 0                         , 253), // #784
  INST(Vsat_b           , LsxVVI             , (0b0111001100100100001, 0, 5, 10, 0)                                                  , 0         , 0                         , 53 ), // #785
  INST(Vsat_bu          , LsxVVI             , (0b0111001100101000001, 0, 5, 10, 0)                                                  , 0         , 0                         , 54 ), // #786
  INST(Vsat_d           , LsxVVI             , (0b0111001100100101, 0, 5, 10, 0)                                                     , 0         , 0                         , 55 ), // #787
  INST(Vsat_du          , LsxVVI             , (0b0111001100101001, 0, 5, 10, 0)                                                     , 0         , 0                         , 56 ), // #788
  INST(Vsat_h           , LsxVVI             , (0b011100110010010001, 0, 5, 10, 0)                                                   , 0         , 0                         , 57 ), // #789
  INST(Vsat_hu          , LsxVVI             , (0b011100110010100001, 0, 5, 10, 0)                                                   , 0         , 0                         , 58 ), // #790
  INST(Vsat_w           , LsxVVI             , (0b01110011001001001, 0, 5, 10, 0)                                                    , 0         , 0                         , 59 ), // #791
  INST(Vsat_wu          , LsxVVI             , (0b01110011001010001, 0, 5, 10, 0)                                                    , 0         , 0                         , 60 ), // #792
  INST(Vseq_b           , LsxVVV             , (0b01110000000000000, 0, 5, 10, 0)                                                    , 0         , 0                         , 254), // #793
  INST(Vseq_d           , LsxVVV             , (0b01110000000000011, 0, 5, 10, 0)                                                    , 0         , 0                         , 255), // #794
  INST(Vseq_h           , LsxVVV             , (0b01110000000000001, 0, 5, 10, 0)                                                    , 0         , 0                         , 256), // #795
  INST(Vseq_w           , LsxVVV             , (0b01110000000000010, 0, 5, 10, 0)                                                    , 0         , 0                         , 257), // #796
  INST(Vseqi_b          , LsxVVI             , (0b01110010100000000, 0, 5, 10, 0)                                                    , 0         , 0                         , 61 ), // #797
  INST(Vseqi_d          , LsxVVI             , (0b01110010100000011, 0, 5, 10, 0)                                                    , 0         , 0                         , 62 ), // #798
  INST(Vseqi_h          , LsxVVI             , (0b01110010100000001, 0, 5, 10, 0)                                                    , 0         , 0                         , 63 ), // #799
  INST(Vseqi_w          , LsxVVI             , (0b01110010100000010, 0, 5, 10, 0)                                                    , 0         , 0                         , 64 ), // #800
  INST(Vsetallnez_b     , LsxIV              , (0b0111001010011100101100, 0, 5, 0)                                                   , 0         , 0                         , 0  ), // #801
  INST(Vsetallnez_d     , LsxIV              , (0b0111001010011100101111, 0, 5, 0)                                                   , 0         , 0                         , 1  ), // #802
  INST(Vsetallnez_h     , LsxIV              , (0b0111001010011100101101, 0, 5, 0)                                                   , 0         , 0                         , 2  ), // #803
  INST(Vsetallnez_w     , LsxIV              , (0b0111001010011100101110, 0, 5, 0)                                                   , 0         , 0                         , 3  ), // #804
  INST(Vsetanyeqz_b     , LsxIV              , (0b0111001010011100101000, 0, 5, 0)                                                   , 0         , 0                         , 4  ), // #805
  INST(Vsetanyeqz_d     , LsxIV              , (0b0111001010011100101011, 0, 5, 0)                                                   , 0         , 0                         , 5  ), // #806
  INST(Vsetanyeqz_h     , LsxIV              , (0b0111001010011100101001, 0, 5, 0)                                                   , 0         , 0                         , 6  ), // #807
  INST(Vsetanyeqz_w     , LsxIV              , (0b0111001010011100101010, 0, 5, 0)                                                   , 0         , 0                         , 7  ), // #808
  INST(Vseteqz_v        , LsxIV              , (0b0111001010011100100110, 0, 5, 0)                                                   , 0         , 0                         , 8  ), // #809
  INST(Vsetnez_v        , LsxIV              , (0b0111001010011100100111, 0, 5, 0)                                                   , 0         , 0                         , 9  ), // #810
  INST(Vshuf4i_b        , LsxVVI             , (0b01110011100100, 0, 5, 10, 0)                                                       , 0         , 0                         , 65 ), // #811
  INST(Vshuf4i_d        , LsxVVI             , (0b01110011100111, 0, 5, 10, 0)                                                       , 0         , 0                         , 66 ), // #812
  INST(Vshuf4i_h        , LsxVVI             , (0b01110011100101, 0, 5, 10, 0)                                                       , 0         , 0                         , 67 ), // #813
  INST(Vshuf4i_w        , LsxVVI             , (0b01110011100110, 0, 5, 10, 0)                                                       , 0         , 0                         , 68 ), // #814
  INST(Vshuf_b          , LsxVVVV            , (0b000011010101, 0, 5, 10, 15, 0)                                                     , 0         , 0                         , 9  ), // #815
  INST(Vshuf_d          , LsxVVV             , (0b01110001011110111, 0, 5, 10, 0)                                                    , 0         , 0                         , 258), // #816
  INST(Vshuf_h          , LsxVVV             , (0b01110001011110101, 0, 5, 10, 0)                                                    , 0         , 0                         , 259), // #817
  INST(Vshuf_w          , LsxVVV             , (0b01110001011110110, 0, 5, 10, 0)                                                    , 0         , 0                         , 260), // #818
  INST(Vsigncov_b       , LsxVVV             , (0b01110001001011100, 0, 5, 10, 0)                                                    , 0         , 0                         , 261), // #819
  INST(Vsigncov_d       , LsxVVV             , (0b01110001001011111, 0, 5, 10, 0)                                                    , 0         , 0                         , 262), // #820
  INST(Vsigncov_h       , LsxVVV             , (0b01110001001011101, 0, 5, 10, 0)                                                    , 0         , 0                         , 263), // #821
  INST(Vsigncov_w       , LsxVVV             , (0b01110001001011110, 0, 5, 10, 0)                                                    , 0         , 0                         , 264), // #822
  INST(Vsle_b           , LsxVVV             , (0b01110000000000100, 0, 5, 10, 0)                                                    , 0         , 0                         , 265), // #823
  INST(Vsle_bu          , LsxVVV             , (0b01110000000001000, 0, 5, 10, 0)                                                    , 0         , 0                         , 266), // #824
  INST(Vsle_d           , LsxVVV             , (0b01110000000000111, 0, 5, 10, 0)                                                    , 0         , 0                         , 267), // #825
  INST(Vsle_du          , LsxVVV             , (0b01110000000001011, 0, 5, 10, 0)                                                    , 0         , 0                         , 268), // #826
  INST(Vsle_h           , LsxVVV             , (0b01110000000000101, 0, 5, 10, 0)                                                    , 0         , 0                         , 269), // #827
  INST(Vsle_hu          , LsxVVV             , (0b01110000000001001, 0, 5, 10, 0)                                                    , 0         , 0                         , 270), // #828
  INST(Vsle_w           , LsxVVV             , (0b01110000000000110, 0, 5, 10, 0)                                                    , 0         , 0                         , 271), // #829
  INST(Vsle_wu          , LsxVVV             , (0b01110000000001010, 0, 5, 10, 0)                                                    , 0         , 0                         , 272), // #830
  INST(Vslei_b          , LsxVVI             , (0b01110010100000100, 0, 5, 10, 0)                                                    , 0         , 0                         , 69 ), // #831
  INST(Vslei_bu         , LsxVVI             , (0b01110010100001000, 0, 5, 10, 0)                                                    , 0         , 0                         , 70 ), // #832
  INST(Vslei_d          , LsxVVI             , (0b01110010100000111, 0, 5, 10, 0)                                                    , 0         , 0                         , 71 ), // #833
  INST(Vslei_du         , LsxVVI             , (0b01110010100001011, 0, 5, 10, 0)                                                    , 0         , 0                         , 72 ), // #834
  INST(Vslei_h          , LsxVVI             , (0b01110010100000101, 0, 5, 10, 0)                                                    , 0         , 0                         , 73 ), // #835
  INST(Vslei_hu         , LsxVVI             , (0b01110010100001001, 0, 5, 10, 0)                                                    , 0         , 0                         , 74 ), // #836
  INST(Vslei_w          , LsxVVI             , (0b01110010100000110, 0, 5, 10, 0)                                                    , 0         , 0                         , 75 ), // #837
  INST(Vslei_wu         , LsxVVI             , (0b01110010100001010, 0, 5, 10, 0)                                                    , 0         , 0                         , 76 ), // #838
  INST(Vsll_b           , LsxVVV             , (0b01110000111010000, 0, 5, 10, 0)                                                    , 0         , 0                         , 273), // #839
  INST(Vsll_d           , LsxVVV             , (0b01110000111010011, 0, 5, 10, 0)                                                    , 0         , 0                         , 274), // #840
  INST(Vsll_h           , LsxVVV             , (0b01110000111010001, 0, 5, 10, 0)                                                    , 0         , 0                         , 275), // #841
  INST(Vsll_w           , LsxVVV             , (0b01110000111010010, 0, 5, 10, 0)                                                    , 0         , 0                         , 276), // #842
  INST(Vslli_b          , LsxVVI             , (0b0111001100101100001, 0, 5, 10, 0)                                                  , 0         , 0                         , 77 ), // #843
  INST(Vslli_d          , LsxVVI             , (0b0111001100101101, 0, 5, 10, 0)                                                     , 0         , 0                         , 78 ), // #844
  INST(Vslli_h          , LsxVVI             , (0b011100110010110001, 0, 5, 10, 0)                                                   , 0         , 0                         , 79 ), // #845
  INST(Vslli_w          , LsxVVI             , (0b01110011001011001, 0, 5, 10, 0)                                                    , 0         , 0                         , 80 ), // #846
  INST(Vsllwil_d_w      , LsxVVI             , (0b01110011000010001, 0, 5, 10, 0)                                                    , 0         , 0                         , 81 ), // #847
  INST(Vsllwil_du_wu    , LsxVVI             , (0b01110011000011001, 0, 5, 10, 0)                                                    , 0         , 0                         , 82 ), // #848
  INST(Vsllwil_h_b      , LsxVVI             , (0b0111001100001000001, 0, 5, 10, 0)                                                  , 0         , 0                         , 83 ), // #849
  INST(Vsllwil_hu_bu    , LsxVVI             , (0b0111001100001100001, 0, 5, 10, 0)                                                  , 0         , 0                         , 84 ), // #850
  INST(Vsllwil_w_h      , LsxVVI             , (0b011100110000100001, 0, 5, 10, 0)                                                   , 0         , 0                         , 85 ), // #851
  INST(Vsllwil_wu_hu    , LsxVVI             , (0b011100110000110001, 0, 5, 10, 0)                                                   , 0         , 0                         , 86 ), // #852
  INST(Vslt_b           , LsxVVV             , (0b01110000000001100, 0, 5, 10, 0)                                                    , 0         , 0                         , 277), // #853
  INST(Vslt_bu          , LsxVVV             , (0b01110000000010000, 0, 5, 10, 0)                                                    , 0         , 0                         , 278), // #854
  INST(Vslt_d           , LsxVVV             , (0b01110000000001111, 0, 5, 10, 0)                                                    , 0         , 0                         , 279), // #855
  INST(Vslt_du          , LsxVVV             , (0b01110000000010011, 0, 5, 10, 0)                                                    , 0         , 0                         , 280), // #856
  INST(Vslt_h           , LsxVVV             , (0b01110000000001101, 0, 5, 10, 0)                                                    , 0         , 0                         , 281), // #857
  INST(Vslt_hu          , LsxVVV             , (0b01110000000010001, 0, 5, 10, 0)                                                    , 0         , 0                         , 282), // #858
  INST(Vslt_w           , LsxVVV             , (0b01110000000001110, 0, 5, 10, 0)                                                    , 0         , 0                         , 283), // #859
  INST(Vslt_wu          , LsxVVV             , (0b01110000000010010, 0, 5, 10, 0)                                                    , 0         , 0                         , 284), // #860
  INST(Vslti_b          , LsxVVI             , (0b01110010100001100, 0, 5, 10, 0)                                                    , 0         , 0                         , 87 ), // #861
  INST(Vslti_bu         , LsxVVI             , (0b01110010100010000, 0, 5, 10, 0)                                                    , 0         , 0                         , 88 ), // #862
  INST(Vslti_d          , LsxVVI             , (0b01110010100001111, 0, 5, 10, 0)                                                    , 0         , 0                         , 89 ), // #863
  INST(Vslti_du         , LsxVVI             , (0b01110010100010011, 0, 5, 10, 0)                                                    , 0         , 0                         , 90 ), // #864
  INST(Vslti_h          , LsxVVI             , (0b01110010100001101, 0, 5, 10, 0)                                                    , 0         , 0                         , 91 ), // #865
  INST(Vslti_hu         , LsxVVI             , (0b01110010100010001, 0, 5, 10, 0)                                                    , 0         , 0                         , 92 ), // #866
  INST(Vslti_w          , LsxVVI             , (0b01110010100001110, 0, 5, 10, 0)                                                    , 0         , 0                         , 93 ), // #867
  INST(Vslti_wu         , LsxVVI             , (0b01110010100010010, 0, 5, 10, 0)                                                    , 0         , 0                         , 94 ), // #868
  INST(Vsra_b           , LsxVVV             , (0b01110000111011000, 0, 5, 10, 0)                                                    , 0         , 0                         , 285), // #869
  INST(Vsra_d           , LsxVVV             , (0b01110000111011011, 0, 5, 10, 0)                                                    , 0         , 0                         , 286), // #870
  INST(Vsra_h           , LsxVVV             , (0b01110000111011001, 0, 5, 10, 0)                                                    , 0         , 0                         , 287), // #871
  INST(Vsra_w           , LsxVVV             , (0b01110000111011010, 0, 5, 10, 0)                                                    , 0         , 0                         , 288), // #872
  INST(Vsrai_b          , LsxVVI             , (0b0111001100110100001, 0, 5, 10, 0)                                                  , 0         , 0                         , 95 ), // #873
  INST(Vsrai_d          , LsxVVI             , (0b0111001100110101, 0, 5, 10, 0)                                                     , 0         , 0                         , 96 ), // #874
  INST(Vsrai_h          , LsxVVI             , (0b011100110011010001, 0, 5, 10, 0)                                                   , 0         , 0                         , 97 ), // #875
  INST(Vsrai_w          , LsxVVI             , (0b01110011001101001, 0, 5, 10, 0)                                                    , 0         , 0                         , 98 ), // #876
  INST(Vsran_b_h        , LsxVVV             , (0b01110000111101101, 0, 5, 10, 0)                                                    , 0         , 0                         , 289), // #877
  INST(Vsran_h_w        , LsxVVV             , (0b01110000111101110, 0, 5, 10, 0)                                                    , 0         , 0                         , 290), // #878
  INST(Vsran_w_d        , LsxVVV             , (0b01110000111101111, 0, 5, 10, 0)                                                    , 0         , 0                         , 291), // #879
  INST(Vsrani_b_h       , LsxVVI             , (0b011100110101100001, 0, 5, 10, 0)                                                   , 0         , 0                         , 99 ), // #880
  INST(Vsrani_d_q       , LsxVVI             , (0b011100110101101, 0, 5, 10, 0)                                                      , 0         , 0                         , 100), // #881
  INST(Vsrani_h_w       , LsxVVI             , (0b01110011010110001, 0, 5, 10, 0)                                                    , 0         , 0                         , 101), // #882
  INST(Vsrani_w_d       , LsxVVI             , (0b0111001101011001, 0, 5, 10, 0)                                                     , 0         , 0                         , 102), // #883
  INST(Vsrar_b          , LsxVVV             , (0b01110000111100100, 0, 5, 10, 0)                                                    , 0         , 0                         , 292), // #884
  INST(Vsrar_d          , LsxVVV             , (0b01110000111100111, 0, 5, 10, 0)                                                    , 0         , 0                         , 293), // #885
  INST(Vsrar_h          , LsxVVV             , (0b01110000111100101, 0, 5, 10, 0)                                                    , 0         , 0                         , 294), // #886
  INST(Vsrar_w          , LsxVVV             , (0b01110000111100110, 0, 5, 10, 0)                                                    , 0         , 0                         , 295), // #887
  INST(Vsrari_b         , LsxVVI             , (0b0111001010101000001, 0, 5, 10, 0)                                                  , 0         , 0                         , 103), // #888
  INST(Vsrari_d         , LsxVVI             , (0b0111001010101001, 0, 5, 10, 0)                                                     , 0         , 0                         , 104), // #889
  INST(Vsrari_h         , LsxVVI             , (0b011100101010100001, 0, 5, 10, 0)                                                   , 0         , 0                         , 105), // #890
  INST(Vsrari_w         , LsxVVI             , (0b01110010101010001, 0, 5, 10, 0)                                                    , 0         , 0                         , 106), // #891
  INST(Vsrarn_b_h       , LsxVVV             , (0b01110000111110101, 0, 5, 10, 0)                                                    , 0         , 0                         , 296), // #892
  INST(Vsrarn_h_w       , LsxVVV             , (0b01110000111110110, 0, 5, 10, 0)                                                    , 0         , 0                         , 297), // #893
  INST(Vsrarn_w_d       , LsxVVV             , (0b01110000111110111, 0, 5, 10, 0)                                                    , 0         , 0                         , 298), // #894
  INST(Vsrarni_b_h      , LsxVVI             , (0b011100110101110001, 0, 5, 10, 0)                                                   , 0         , 0                         , 107), // #895
  INST(Vsrarni_d_q      , LsxVVI             , (0b011100110101111, 0, 5, 10, 0)                                                      , 0         , 0                         , 108), // #896
  INST(Vsrarni_h_w      , LsxVVI             , (0b01110011010111001, 0, 5, 10, 0)                                                    , 0         , 0                         , 109), // #897
  INST(Vsrarni_w_d      , LsxVVI             , (0b0111001101011101, 0, 5, 10, 0)                                                     , 0         , 0                         , 110), // #898
  INST(Vsrl_b           , LsxVVV             , (0b01110000111010100, 0, 5, 10, 0)                                                    , 0         , 0                         , 299), // #899
  INST(Vsrl_d           , LsxVVV             , (0b01110000111010111, 0, 5, 10, 0)                                                    , 0         , 0                         , 300), // #900
  INST(Vsrl_h           , LsxVVV             , (0b01110000111010101, 0, 5, 10, 0)                                                    , 0         , 0                         , 301), // #901
  INST(Vsrl_w           , LsxVVV             , (0b01110000111010110, 0, 5, 10, 0)                                                    , 0         , 0                         , 302), // #902
  INST(Vsrli_b          , LsxVVI             , (0b0111001100110000001, 0, 5, 10, 0)                                                  , 0         , 0                         , 111), // #903
  INST(Vsrli_d          , LsxVVI             , (0b0111001100110001, 0, 5, 10, 0)                                                     , 0         , 0                         , 112), // #904
  INST(Vsrli_h          , LsxVVI             , (0b011100110011000001, 0, 5, 10, 0)                                                   , 0         , 0                         , 113), // #905
  INST(Vsrli_w          , LsxVVI             , (0b01110011001100001, 0, 5, 10, 0)                                                    , 0         , 0                         , 114), // #906
  INST(Vsrln_b_h        , LsxVVV             , (0b01110000111101001, 0, 5, 10, 0)                                                    , 0         , 0                         , 303), // #907
  INST(Vsrln_h_w        , LsxVVV             , (0b01110000111101010, 0, 5, 10, 0)                                                    , 0         , 0                         , 304), // #908
  INST(Vsrln_w_d        , LsxVVV             , (0b01110000111101011, 0, 5, 10, 0)                                                    , 0         , 0                         , 305), // #909
  INST(Vsrlni_b_h       , LsxVVI             , (0b011100110100000001, 0, 5, 10, 0)                                                   , 0         , 0                         , 115), // #910
  INST(Vsrlni_d_q       , LsxVVI             , (0b011100110100001, 0, 5, 10, 0)                                                      , 0         , 0                         , 116), // #911
  INST(Vsrlni_h_w       , LsxVVI             , (0b01110011010000001, 0, 5, 10, 0)                                                    , 0         , 0                         , 117), // #912
  INST(Vsrlni_w_d       , LsxVVI             , (0b0111001101000001, 0, 5, 10, 0)                                                     , 0         , 0                         , 118), // #913
  INST(Vsrlr_b          , LsxVVV             , (0b01110000111100000, 0, 5, 10, 0)                                                    , 0         , 0                         , 306), // #914
  INST(Vsrlr_d          , LsxVVV             , (0b01110000111100011, 0, 5, 10, 0)                                                    , 0         , 0                         , 307), // #915
  INST(Vsrlr_h          , LsxVVV             , (0b01110000111100001, 0, 5, 10, 0)                                                    , 0         , 0                         , 308), // #916
  INST(Vsrlr_w          , LsxVVV             , (0b01110000111100010, 0, 5, 10, 0)                                                    , 0         , 0                         , 309), // #917
  INST(Vsrlri_b         , LsxVVI             , (0b0111001010100100001, 0, 5, 10, 0)                                                  , 0         , 0                         , 119), // #918
  INST(Vsrlri_d         , LsxVVI             , (0b0111001010100101, 0, 5, 10, 0)                                                     , 0         , 0                         , 120), // #919
  INST(Vsrlri_h         , LsxVVI             , (0b011100101010010001, 0, 5, 10, 0)                                                   , 0         , 0                         , 121), // #920
  INST(Vsrlri_w         , LsxVVI             , (0b01110010101001001, 0, 5, 10, 0)                                                    , 0         , 0                         , 122), // #921
  INST(Vsrlrn_b_h       , LsxVVV             , (0b01110000111110001, 0, 5, 10, 0)                                                    , 0         , 0                         , 310), // #922
  INST(Vsrlrn_h_w       , LsxVVV             , (0b01110000111110010, 0, 5, 10, 0)                                                    , 0         , 0                         , 311), // #923
  INST(Vsrlrn_w_d       , LsxVVV             , (0b01110000111110011, 0, 5, 10, 0)                                                    , 0         , 0                         , 312), // #924
  INST(Vssran_b_h       , LsxVVV             , (0b01110000111111101, 0, 5, 10, 0)                                                    , 0         , 0                         , 313), // #925
  INST(Vssran_bu_h      , LsxVVV             , (0b01110001000001101, 0, 5, 10, 0)                                                    , 0         , 0                         , 314), // #926
  INST(Vssran_h_w       , LsxVVV             , (0b01110000111111110, 0, 5, 10, 0)                                                    , 0         , 0                         , 315), // #927
  INST(Vssran_hu_w      , LsxVVV             , (0b01110001000001110, 0, 5, 10, 0)                                                    , 0         , 0                         , 316), // #928
  INST(Vssran_w_d       , LsxVVV             , (0b01110000111111111, 0, 5, 10, 0)                                                    , 0         , 0                         , 317), // #929
  INST(Vssran_wu_d      , LsxVVV             , (0b01110001000001111, 0, 5, 10, 0)                                                    , 0         , 0                         , 318), // #930
  INST(Vssrani_b_h      , LsxVVI             , (0b011100110110000001, 0, 5, 10, 0)                                                   , 0         , 0                         , 123), // #931
  INST(Vssrani_bu_h     , LsxVVI             , (0b011100110110010001, 0, 5, 10, 0)                                                   , 0         , 0                         , 124), // #932
  INST(Vssrani_d_q      , LsxVVI             , (0b011100110110001, 0, 5, 10, 0)                                                      , 0         , 0                         , 125), // #933
  INST(Vssrani_du_q     , LsxVVI             , (0b011100110110011, 0, 5, 10, 0)                                                      , 0         , 0                         , 126), // #934
  INST(Vssrani_h_w      , LsxVVI             , (0b01110011011000001, 0, 5, 10, 0)                                                    , 0         , 0                         , 127), // #935
  INST(Vssrani_hu_w     , LsxVVI             , (0b01110011011001001, 0, 5, 10, 0)                                                    , 0         , 0                         , 128), // #936
  INST(Vssrani_w_d      , LsxVVI             , (0b0111001101100001, 0, 5, 10, 0)                                                     , 0         , 0                         , 129), // #937
  INST(Vssrani_wu_d     , LsxVVI             , (0b0111001101100101, 0, 5, 10, 0)                                                     , 0         , 0                         , 130), // #938
  INST(Vssrarn_b_h      , LsxVVV             , (0b01110001000000101, 0, 5, 10, 0)                                                    , 0         , 0                         , 319), // #939
  INST(Vssrarn_bu_h     , LsxVVV             , (0b01110001000010101, 0, 5, 10, 0)                                                    , 0         , 0                         , 320), // #940
  INST(Vssrarn_h_w      , LsxVVV             , (0b01110001000000110, 0, 5, 10, 0)                                                    , 0         , 0                         , 321), // #941
  INST(Vssrarn_hu_w     , LsxVVV             , (0b01110001000010110, 0, 5, 10, 0)                                                    , 0         , 0                         , 322), // #942
  INST(Vssrarn_w_d      , LsxVVV             , (0b01110001000000111, 0, 5, 10, 0)                                                    , 0         , 0                         , 323), // #943
  INST(Vssrarn_wu_d     , LsxVVV             , (0b01110001000010111, 0, 5, 10, 0)                                                    , 0         , 0                         , 324), // #944
  INST(Vssrarni_b_h     , LsxVVI             , (0b011100110110100001, 0, 5, 10, 0)                                                   , 0         , 0                         , 131), // #945
  INST(Vssrarni_bu_h    , LsxVVI             , (0b011100110110110001, 0, 5, 10, 0)                                                   , 0         , 0                         , 132), // #946
  INST(Vssrarni_d_q     , LsxVVI             , (0b011100110110101, 0, 5, 10, 0)                                                      , 0         , 0                         , 133), // #947
  INST(Vssrarni_du_q    , LsxVVI             , (0b011100110110111, 0, 5, 10, 0)                                                      , 0         , 0                         , 134), // #948
  INST(Vssrarni_h_w     , LsxVVI             , (0b01110011011010001, 0, 5, 10, 0)                                                    , 0         , 0                         , 135), // #949
  INST(Vssrarni_hu_w    , LsxVVI             , (0b01110011011011001, 0, 5, 10, 0)                                                    , 0         , 0                         , 136), // #950
  INST(Vssrarni_w_d     , LsxVVI             , (0b0111001101101001, 0, 5, 10, 0)                                                     , 0         , 0                         , 137), // #951
  INST(Vssrarni_wu_d    , LsxVVI             , (0b0111001101101101, 0, 5, 10, 0)                                                     , 0         , 0                         , 138), // #952
  INST(Vssrln_b_h       , LsxVVV             , (0b01110000111111001, 0, 5, 10, 0)                                                    , 0         , 0                         , 325), // #953
  INST(Vssrln_bu_h      , LsxVVV             , (0b01110001000001001, 0, 5, 10, 0)                                                    , 0         , 0                         , 326), // #954
  INST(Vssrln_h_w       , LsxVVV             , (0b01110000111111010, 0, 5, 10, 0)                                                    , 0         , 0                         , 327), // #955
  INST(Vssrln_hu_w      , LsxVVV             , (0b01110001000001010, 0, 5, 10, 0)                                                    , 0         , 0                         , 328), // #956
  INST(Vssrln_w_d       , LsxVVV             , (0b01110000111111011, 0, 5, 10, 0)                                                    , 0         , 0                         , 329), // #957
  INST(Vssrln_wu_d      , LsxVVV             , (0b01110001000001011, 0, 5, 10, 0)                                                    , 0         , 0                         , 330), // #958
  INST(Vssrlni_b_h      , LsxVVI             , (0b011100110100100001, 0, 5, 10, 0)                                                   , 0         , 0                         , 139), // #959
  INST(Vssrlni_bu_h     , LsxVVI             , (0b011100110100110001, 0, 5, 10, 0)                                                   , 0         , 0                         , 140), // #960
  INST(Vssrlni_d_q      , LsxVVI             , (0b011100110100101, 0, 5, 10, 0)                                                      , 0         , 0                         , 141), // #961
  INST(Vssrlni_du_q     , LsxVVI             , (0b011100110100111, 0, 5, 10, 0)                                                      , 0         , 0                         , 142), // #962
  INST(Vssrlni_h_w      , LsxVVI             , (0b01110011010010001, 0, 5, 10, 0)                                                    , 0         , 0                         , 143), // #963
  INST(Vssrlni_hu_w     , LsxVVI             , (0b01110011010011001, 0, 5, 10, 0)                                                    , 0         , 0                         , 144), // #964
  INST(Vssrlni_w_d      , LsxVVI             , (0b0111001101001001, 0, 5, 10, 0)                                                     , 0         , 0                         , 145), // #965
  INST(Vssrlni_wu_d     , LsxVVI             , (0b0111001101001101, 0, 5, 10, 0)                                                     , 0         , 0                         , 146), // #966
  INST(Vssrlrn_b_h      , LsxVVV             , (0b01110001000000001, 0, 5, 10, 0)                                                    , 0         , 0                         , 331), // #967
  INST(Vssrlrn_bu_h     , LsxVVV             , (0b01110001000010001, 0, 5, 10, 0)                                                    , 0         , 0                         , 332), // #968
  INST(Vssrlrn_h_w      , LsxVVV             , (0b01110001000000010, 0, 5, 10, 0)                                                    , 0         , 0                         , 333), // #969
  INST(Vssrlrn_hu_w     , LsxVVV             , (0b01110001000010010, 0, 5, 10, 0)                                                    , 0         , 0                         , 334), // #970
  INST(Vssrlrn_w_d      , LsxVVV             , (0b01110001000000011, 0, 5, 10, 0)                                                    , 0         , 0                         , 335), // #971
  INST(Vssrlrn_wu_d     , LsxVVV             , (0b01110001000010011, 0, 5, 10, 0)                                                    , 0         , 0                         , 336), // #972
  INST(Vssrlrni_b_h     , LsxVVI             , (0b011100110101000001, 0, 5, 10, 0)                                                   , 0         , 0                         , 147), // #973
  INST(Vssrlrni_bu_h    , LsxVVI             , (0b011100110101010001, 0, 5, 10, 0)                                                   , 0         , 0                         , 148), // #974
  INST(Vssrlrni_d_q     , LsxVVI             , (0b011100110101001, 0, 5, 10, 0)                                                      , 0         , 0                         , 149), // #975
  INST(Vssrlrni_du_q    , LsxVVI             , (0b011100110101011, 0, 5, 10, 0)                                                      , 0         , 0                         , 150), // #976
  INST(Vssrlrni_h_w     , LsxVVI             , (0b01110011010100001, 0, 5, 10, 0)                                                    , 0         , 0                         , 151), // #977
  INST(Vssrlrni_hu_w    , LsxVVI             , (0b01110011010101001, 0, 5, 10, 0)                                                    , 0         , 0                         , 152), // #978
  INST(Vssrlrni_w_d     , LsxVVI             , (0b0111001101010001, 0, 5, 10, 0)                                                     , 0         , 0                         , 153), // #979
  INST(Vssrlrni_wu_d    , LsxVVI             , (0b0111001101010101, 0, 5, 10, 0)                                                     , 0         , 0                         , 154), // #980
  INST(Vssub_b          , LsxVVV             , (0b01110000010010000, 0, 5, 10, 0)                                                    , 0         , 0                         , 337), // #981
  INST(Vssub_bu         , LsxVVV             , (0b01110000010011000, 0, 5, 10, 0)                                                    , 0         , 0                         , 338), // #982
  INST(Vssub_d          , LsxVVV             , (0b01110000010010011, 0, 5, 10, 0)                                                    , 0         , 0                         , 339), // #983
  INST(Vssub_du         , LsxVVV             , (0b01110000010011011, 0, 5, 10, 0)                                                    , 0         , 0                         , 340), // #984
  INST(Vssub_h          , LsxVVV             , (0b01110000010010001, 0, 5, 10, 0)                                                    , 0         , 0                         , 341), // #985
  INST(Vssub_hu         , LsxVVV             , (0b01110000010011001, 0, 5, 10, 0)                                                    , 0         , 0                         , 342), // #986
  INST(Vssub_w          , LsxVVV             , (0b01110000010010010, 0, 5, 10, 0)                                                    , 0         , 0                         , 343), // #987
  INST(Vssub_wu         , LsxVVV             , (0b01110000010011010, 0, 5, 10, 0)                                                    , 0         , 0                         , 344), // #988
  INST(Vst              , SimdLdst           , (0b0010110001,        22, 12)                                                         , 0         , 0                         , 6  ), // #989
  INST(Vstelm_b         , LsxVRII            , (0b0011000110, 0, kWX, 5, 10, 18, 0)                                                  , 0         , 0                         , 0  ), // #990
  INST(Vstelm_d         , LsxVRII            , (0b0011000100010, 0, kWX, 5, 10, 18, 0)                                               , 0         , 0                         , 1  ), // #991
  INST(Vstelm_h         , LsxVRII            , (0b00110001010, 0, kWX, 5, 10, 18, 0)                                                 , 0         , 0                         , 2  ), // #992
  INST(Vstelm_w         , LsxVRII            , (0b001100010010, 0, kWX, 5, 10, 18, 0)                                                , 0         , 0                         , 3  ), // #993
  INST(Vstx             , SimdLdst           , (0b00111000010001000, 15, 0 )                                                         , 0         , 0                         , 7  ), // #994
  INST(Vsub_b           , LsxVVV             , (0b01110000000011000, 0, 5, 10, 0)                                                    , 0         , 0                         , 345), // #995
  INST(Vsub_d           , LsxVVV             , (0b01110000000011011, 0, 5, 10, 0)                                                    , 0         , 0                         , 346), // #996
  INST(Vsub_h           , LsxVVV             , (0b01110000000011001, 0, 5, 10, 0)                                                    , 0         , 0                         , 347), // #997
  INST(Vsub_q           , LsxVVV             , (0b01110001001011011, 0, 5, 10, 0)                                                    , 0         , 0                         , 348), // #998
  INST(Vsub_w           , LsxVVV             , (0b01110000000011010, 0, 5, 10, 0)                                                    , 0         , 0                         , 349), // #999
  INST(Vsubi_bu         , LsxVVI             , (0b01110010100011000, 0, 5, 10, 0)                                                    , 0         , 0                         , 155), // #1000
  INST(Vsubi_du         , LsxVVI             , (0b01110010100011011, 0, 5, 10, 0)                                                    , 0         , 0                         , 156), // #1001
  INST(Vsubi_hu         , LsxVVI             , (0b01110010100011001, 0, 5, 10, 0)                                                    , 0         , 0                         , 157), // #1002
  INST(Vsubi_wu         , LsxVVI             , (0b01110010100011010, 0, 5, 10, 0)                                                    , 0         , 0                         , 158), // #1003
  INST(Vsubwev_d_w      , LsxVVV             , (0b01110000001000010, 0, 5, 10, 0)                                                    , 0         , 0                         , 350), // #1004
  INST(Vsubwev_d_wu     , LsxVVV             , (0b01110000001100010, 0, 5, 10, 0)                                                    , 0         , 0                         , 351), // #1005
  INST(Vsubwev_h_b      , LsxVVV             , (0b01110000001000000, 0, 5, 10, 0)                                                    , 0         , 0                         , 352), // #1006
  INST(Vsubwev_h_bu     , LsxVVV             , (0b01110000001100000, 0, 5, 10, 0)                                                    , 0         , 0                         , 353), // #1007
  INST(Vsubwev_q_d      , LsxVVV             , (0b01110000001000011, 0, 5, 10, 0)                                                    , 0         , 0                         , 354), // #1008
  INST(Vsubwev_q_du     , LsxVVV             , (0b01110000001100011, 0, 5, 10, 0)                                                    , 0         , 0                         , 355), // #1009
  INST(Vsubwev_w_h      , LsxVVV             , (0b01110000001000001, 0, 5, 10, 0)                                                    , 0         , 0                         , 356), // #1010
  INST(Vsubwev_w_hu     , LsxVVV             , (0b01110000001100001, 0, 5, 10, 0)                                                    , 0         , 0                         , 357), // #1011
  INST(Vsubwod_d_w      , LsxVVV             , (0b01110000001001010, 0, 5, 10, 0)                                                    , 0         , 0                         , 358), // #1012
  INST(Vsubwod_d_wu     , LsxVVV             , (0b01110000001101010, 0, 5, 10, 0)                                                    , 0         , 0                         , 359), // #1013
  INST(Vsubwod_h_b      , LsxVVV             , (0b01110000001001000, 0, 5, 10, 0)                                                    , 0         , 0                         , 360), // #1014
  INST(Vsubwod_h_bu     , LsxVVV             , (0b01110000001101000, 0, 5, 10, 0)                                                    , 0         , 0                         , 361), // #1015
  INST(Vsubwod_q_d      , LsxVVV             , (0b01110000001001011, 0, 5, 10, 0)                                                    , 0         , 0                         , 362), // #1016
  INST(Vsubwod_q_du     , LsxVVV             , (0b01110000001101011, 0, 5, 10, 0)                                                    , 0         , 0                         , 363), // #1017
  INST(Vsubwod_w_h      , LsxVVV             , (0b01110000001001001, 0, 5, 10, 0)                                                    , 0         , 0                         , 364), // #1018
  INST(Vsubwod_w_hu     , LsxVVV             , (0b01110000001101001, 0, 5, 10, 0)                                                    , 0         , 0                         , 365), // #1019
  INST(Vxor_v           , LsxVVV             , (0b01110001001001110, 0, 5, 10, 0)                                                    , 0         , 0                         , 366), // #1020
  INST(Vxori_b          , LsxVVI             , (0b01110011110110, 0, 5, 10, 0)                                                       , 0         , 0                         , 159), // #1021
  INST(Xvabsd_b         , LasxXXX            , (0b01110100011000000, 0, 5, 10, 0)                                                    , 0         , 0                         , 0  ), // #1022
  INST(Xvabsd_bu        , LasxXXX            , (0b01110100011000100, 0, 5, 10, 0)                                                    , 0         , 0                         , 1  ), // #1023
  INST(Xvabsd_d         , LasxXXX            , (0b01110100011000011, 0, 5, 10, 0)                                                    , 0         , 0                         , 2  ), // #1024
  INST(Xvabsd_du        , LasxXXX            , (0b01110100011000111, 0, 5, 10, 0)                                                    , 0         , 0                         , 3  ), // #1025
  INST(Xvabsd_h         , LasxXXX            , (0b01110100011000001, 0, 5, 10, 0)                                                    , 0         , 0                         , 4  ), // #1026
  INST(Xvabsd_hu        , LasxXXX            , (0b01110100011000101, 0, 5, 10, 0)                                                    , 0         , 0                         , 5  ), // #1027
  INST(Xvabsd_w         , LasxXXX            , (0b01110100011000010, 0, 5, 10, 0)                                                    , 0         , 0                         , 6  ), // #1028
  INST(Xvabsd_wu        , LasxXXX            , (0b01110100011000110, 0, 5, 10, 0)                                                    , 0         , 0                         , 7  ), // #1029
  INST(Xvadd_b          , LasxXXX            , (0b01110100000010100, 0, 5, 10, 0)                                                    , 0         , 0                         , 8  ), // #1030
  INST(Xvadd_d          , LasxXXX            , (0b01110100000010111, 0, 5, 10, 0)                                                    , 0         , 0                         , 9  ), // #1031
  INST(Xvadd_h          , LasxXXX            , (0b01110100000010101, 0, 5, 10, 0)                                                    , 0         , 0                         , 10 ), // #1032
  INST(Xvadd_q          , LasxXXX            , (0b01110101001011010, 0, 5, 10, 0)                                                    , 0         , 0                         , 11 ), // #1033
  INST(Xvadd_w          , LasxXXX            , (0b01110100000010110, 0, 5, 10, 0)                                                    , 0         , 0                         , 12 ), // #1034
  INST(Xvadda_b         , LasxXXX            , (0b01110100010111000, 0, 5, 10, 0)                                                    , 0         , 0                         , 13 ), // #1035
  INST(Xvadda_d         , LasxXXX            , (0b01110100010111011, 0, 5, 10, 0)                                                    , 0         , 0                         , 14 ), // #1036
  INST(Xvadda_h         , LasxXXX            , (0b01110100010111001, 0, 5, 10, 0)                                                    , 0         , 0                         , 15 ), // #1037
  INST(Xvadda_w         , LasxXXX            , (0b01110100010111010, 0, 5, 10, 0)                                                    , 0         , 0                         , 16 ), // #1038
  INST(Xvaddi_bu        , LasxXXI            , (0b01110110100010100, 0, 5, 10, 0)                                                    , 0         , 0                         , 0  ), // #1039
  INST(Xvaddi_du        , LasxXXI            , (0b01110110100010111, 0, 5, 10, 0)                                                    , 0         , 0                         , 1  ), // #1040
  INST(Xvaddi_hu        , LasxXXI            , (0b01110110100010101, 0, 5, 10, 0)                                                    , 0         , 0                         , 2  ), // #1041
  INST(Xvaddi_wu        , LasxXXI            , (0b01110110100010110, 0, 5, 10, 0)                                                    , 0         , 0                         , 3  ), // #1042
  INST(Xvaddwev_d_w     , LasxXXX            , (0b01110100000111110, 0, 5, 10, 0)                                                    , 0         , 0                         , 17 ), // #1043
  INST(Xvaddwev_d_wu    , LasxXXX            , (0b01110100001011110, 0, 5, 10, 0)                                                    , 0         , 0                         , 18 ), // #1044
  INST(Xvaddwev_d_wu_w  , LasxXXX            , (0b01110100001111110, 0, 5, 10, 0)                                                    , 0         , 0                         , 19 ), // #1045
  INST(Xvaddwev_h_b     , LasxXXX            , (0b01110100000111100, 0, 5, 10, 0)                                                    , 0         , 0                         , 20 ), // #1046
  INST(Xvaddwev_h_bu    , LasxXXX            , (0b01110100001011100, 0, 5, 10, 0)                                                    , 0         , 0                         , 21 ), // #1047
  INST(Xvaddwev_h_bu_b  , LasxXXX            , (0b01110100001111100, 0, 5, 10, 0)                                                    , 0         , 0                         , 22 ), // #1048
  INST(Xvaddwev_q_d     , LasxXXX            , (0b01110100000111111, 0, 5, 10, 0)                                                    , 0         , 0                         , 23 ), // #1049
  INST(Xvaddwev_q_du    , LasxXXX            , (0b01110100001011111, 0, 5, 10, 0)                                                    , 0         , 0                         , 24 ), // #1050
  INST(Xvaddwev_q_du_d  , LasxXXX            , (0b01110100001111111, 0, 5, 10, 0)                                                    , 0         , 0                         , 25 ), // #1051
  INST(Xvaddwev_w_h     , LasxXXX            , (0b01110100000111101, 0, 5, 10, 0)                                                    , 0         , 0                         , 26 ), // #1052
  INST(Xvaddwev_w_hu    , LasxXXX            , (0b01110100001011101, 0, 5, 10, 0)                                                    , 0         , 0                         , 27 ), // #1053
  INST(Xvaddwev_w_hu_h  , LasxXXX            , (0b01110100001111101, 0, 5, 10, 0)                                                    , 0         , 0                         , 28 ), // #1054
  INST(Xvaddwod_d_w     , LasxXXX            , (0b01110100001000110, 0, 5, 10, 0)                                                    , 0         , 0                         , 29 ), // #1055
  INST(Xvaddwod_d_wu    , LasxXXX            , (0b01110100001100110, 0, 5, 10, 0)                                                    , 0         , 0                         , 30 ), // #1056
  INST(Xvaddwod_d_wu_w  , LasxXXX            , (0b01110100010000010, 0, 5, 10, 0)                                                    , 0         , 0                         , 31 ), // #1057
  INST(Xvaddwod_h_b     , LasxXXX            , (0b01110100001000100, 0, 5, 10, 0)                                                    , 0         , 0                         , 32 ), // #1058
  INST(Xvaddwod_h_bu    , LasxXXX            , (0b01110100001100100, 0, 5, 10, 0)                                                    , 0         , 0                         , 33 ), // #1059
  INST(Xvaddwod_h_bu_b  , LasxXXX            , (0b01110100010000000, 0, 5, 10, 0)                                                    , 0         , 0                         , 34 ), // #1060
  INST(Xvaddwod_q_d     , LasxXXX            , (0b01110100001000111, 0, 5, 10, 0)                                                    , 0         , 0                         , 35 ), // #1061
  INST(Xvaddwod_q_du    , LasxXXX            , (0b01110100001100111, 0, 5, 10, 0)                                                    , 0         , 0                         , 36 ), // #1062
  INST(Xvaddwod_q_du_d  , LasxXXX            , (0b01110100010000011, 0, 5, 10, 0)                                                    , 0         , 0                         , 37 ), // #1063
  INST(Xvaddwod_w_h     , LasxXXX            , (0b01110100001000101, 0, 5, 10, 0)                                                    , 0         , 0                         , 38 ), // #1064
  INST(Xvaddwod_w_hu    , LasxXXX            , (0b01110100001100101, 0, 5, 10, 0)                                                    , 0         , 0                         , 39 ), // #1065
  INST(Xvaddwod_w_hu_h  , LasxXXX            , (0b01110100010000001, 0, 5, 10, 0)                                                    , 0         , 0                         , 40 ), // #1066
  INST(Xvand_v          , LasxXXX            , (0b01110101001001100, 0, 5, 10, 0)                                                    , 0         , 0                         , 41 ), // #1067
  INST(Xvandi_b         , LasxXXI            , (0b01110111110100, 0, 5, 10, 0)                                                       , 0         , 0                         , 4  ), // #1068
  INST(Xvandn_v         , LasxXXX            , (0b01110101001010000, 0, 5, 10, 0)                                                    , 0         , 0                         , 42 ), // #1069
  INST(Xvavg_b          , LasxXXX            , (0b01110100011001000, 0, 5, 10, 0)                                                    , 0         , 0                         , 43 ), // #1070
  INST(Xvavg_bu         , LasxXXX            , (0b01110100011001100, 0, 5, 10, 0)                                                    , 0         , 0                         , 44 ), // #1071
  INST(Xvavg_d          , LasxXXX            , (0b01110100011001011, 0, 5, 10, 0)                                                    , 0         , 0                         , 45 ), // #1072
  INST(Xvavg_du         , LasxXXX            , (0b01110100011001111, 0, 5, 10, 0)                                                    , 0         , 0                         , 46 ), // #1073
  INST(Xvavg_h          , LasxXXX            , (0b01110100011001001, 0, 5, 10, 0)                                                    , 0         , 0                         , 47 ), // #1074
  INST(Xvavg_hu         , LasxXXX            , (0b01110100011001101, 0, 5, 10, 0)                                                    , 0         , 0                         , 48 ), // #1075
  INST(Xvavg_w          , LasxXXX            , (0b01110100011001010, 0, 5, 10, 0)                                                    , 0         , 0                         , 49 ), // #1076
  INST(Xvavg_wu         , LasxXXX            , (0b01110100011001110, 0, 5, 10, 0)                                                    , 0         , 0                         , 50 ), // #1077
  INST(Xvavgr_b         , LasxXXX            , (0b01110100011010000, 0, 5, 10, 0)                                                    , 0         , 0                         , 51 ), // #1078
  INST(Xvavgr_bu        , LasxXXX            , (0b01110100011010100, 0, 5, 10, 0)                                                    , 0         , 0                         , 52 ), // #1079
  INST(Xvavgr_d         , LasxXXX            , (0b01110100011010011, 0, 5, 10, 0)                                                    , 0         , 0                         , 53 ), // #1080
  INST(Xvavgr_du        , LasxXXX            , (0b01110100011010111, 0, 5, 10, 0)                                                    , 0         , 0                         , 54 ), // #1081
  INST(Xvavgr_h         , LasxXXX            , (0b01110100011010001, 0, 5, 10, 0)                                                    , 0         , 0                         , 55 ), // #1082
  INST(Xvavgr_hu        , LasxXXX            , (0b01110100011010101, 0, 5, 10, 0)                                                    , 0         , 0                         , 56 ), // #1083
  INST(Xvavgr_w         , LasxXXX            , (0b01110100011010010, 0, 5, 10, 0)                                                    , 0         , 0                         , 57 ), // #1084
  INST(Xvavgr_wu        , LasxXXX            , (0b01110100011010110, 0, 5, 10, 0)                                                    , 0         , 0                         , 58 ), // #1085
  INST(Xvbitclr_b       , LasxXXX            , (0b01110101000011000, 0, 5, 10, 0)                                                    , 0         , 0                         , 59 ), // #1086
  INST(Xvbitclr_d       , LasxXXX            , (0b01110101000011011, 0, 5, 10, 0)                                                    , 0         , 0                         , 60 ), // #1087
  INST(Xvbitclr_h       , LasxXXX            , (0b01110101000011001, 0, 5, 10, 0)                                                    , 0         , 0                         , 61 ), // #1088
  INST(Xvbitclr_w       , LasxXXX            , (0b01110101000011010, 0, 5, 10, 0)                                                    , 0         , 0                         , 62 ), // #1089
  INST(Xvbitclri_b      , LasxXXI            , (0b0111011100010000001, 0, 5, 10, 0)                                                  , 0         , 0                         , 5  ), // #1090
  INST(Xvbitclri_d      , LasxXXI            , (0b0111011100010001, 0, 5, 10, 0)                                                     , 0         , 0                         , 6  ), // #1091
  INST(Xvbitclri_h      , LasxXXI            , (0b011101110001000001, 0, 5, 10, 0)                                                   , 0         , 0                         , 7  ), // #1092
  INST(Xvbitclri_w      , LasxXXI            , (0b01110111000100001, 0, 5, 10, 0)                                                    , 0         , 0                         , 8  ), // #1093
  INST(Xvbitrev_b       , LasxXXX            , (0b01110101000100000, 0, 5, 10, 0)                                                    , 0         , 0                         , 63 ), // #1094
  INST(Xvbitrev_d       , LasxXXX            , (0b01110101000100011, 0, 5, 10, 0)                                                    , 0         , 0                         , 64 ), // #1095
  INST(Xvbitrev_h       , LasxXXX            , (0b01110101000100001, 0, 5, 10, 0)                                                    , 0         , 0                         , 65 ), // #1096
  INST(Xvbitrev_w       , LasxXXX            , (0b01110101000100010, 0, 5, 10, 0)                                                    , 0         , 0                         , 66 ), // #1097
  INST(Xvbitrevi_b      , LasxXXI            , (0b0111011100011000001, 0, 5, 10, 0)                                                  , 0         , 0                         , 9  ), // #1098
  INST(Xvbitrevi_d      , LasxXXI            , (0b0111011100011001, 0, 5, 10, 0)                                                     , 0         , 0                         , 10 ), // #1099
  INST(Xvbitrevi_h      , LasxXXI            , (0b011101110001100001, 0, 5, 10, 0)                                                   , 0         , 0                         , 11 ), // #1100
  INST(Xvbitrevi_w      , LasxXXI            , (0b01110111000110001, 0, 5, 10, 0)                                                    , 0         , 0                         , 12 ), // #1101
  INST(Xvbitsel_v       , LasxXXXX           , (0b000011010010, 0, 5, 10, 15, 0)                                                     , 0         , 0                         , 0  ), // #1102
  INST(Xvbitseli_b      , LasxXXI            , (0b01110111110001, 0, 5, 10, 0)                                                       , 0         , 0                         , 13 ), // #1103
  INST(Xvbitset_b       , LasxXXX            , (0b01110101000011100, 0, 5, 10, 0)                                                    , 0         , 0                         , 67 ), // #1104
  INST(Xvbitset_d       , LasxXXX            , (0b01110101000011111, 0, 5, 10, 0)                                                    , 0         , 0                         , 68 ), // #1105
  INST(Xvbitset_h       , LasxXXX            , (0b01110101000011101, 0, 5, 10, 0)                                                    , 0         , 0                         , 69 ), // #1106
  INST(Xvbitset_w       , LasxXXX            , (0b01110101000011110, 0, 5, 10, 0)                                                    , 0         , 0                         , 70 ), // #1107
  INST(Xvbitseti_b      , LasxXXI            , (0b0111011100010100001, 0, 5, 10, 0)                                                  , 0         , 0                         , 14 ), // #1108
  INST(Xvbitseti_d      , LasxXXI            , (0b0111011100010101, 0, 5, 10, 0)                                                     , 0         , 0                         , 15 ), // #1109
  INST(Xvbitseti_h      , LasxXXI            , (0b011101110001010001, 0, 5, 10, 0)                                                   , 0         , 0                         , 16 ), // #1110
  INST(Xvbitseti_w      , LasxXXI            , (0b01110111000101001, 0, 5, 10, 0)                                                    , 0         , 0                         , 17 ), // #1111
  INST(Xvbsll_v         , LasxXXI            , (0b01110110100011100, 0, 5, 10, 0)                                                    , 0         , 0                         , 18 ), // #1112
  INST(Xvbsrl_v         , LasxXXI            , (0b01110110100011101, 0, 5, 10, 0)                                                    , 0         , 0                         , 19 ), // #1113
  INST(Xvclo_b          , LasxXX             , (0b0111011010011100000000, 0, 5, 0)                                                   , 0         , 0                         , 12 ), // #1114
  INST(Xvclo_d          , LasxXX             , (0b0111011010011100000011, 0, 5, 0)                                                   , 0         , 0                         , 13 ), // #1115
  INST(Xvclo_h          , LasxXX             , (0b0111011010011100000001, 0, 5, 0)                                                   , 0         , 0                         , 14 ), // #1116
  INST(Xvclo_w          , LasxXX             , (0b0111011010011100000010, 0, 5, 0)                                                   , 0         , 0                         , 15 ), // #1117
  INST(Xvclz_b          , LasxXX             , (0b0111011010011100000100, 0, 5, 0)                                                   , 0         , 0                         , 16 ), // #1118
  INST(Xvclz_d          , LasxXX             , (0b0111011010011100000111, 0, 5, 0)                                                   , 0         , 0                         , 17 ), // #1119
  INST(Xvclz_h          , LasxXX             , (0b0111011010011100000101, 0, 5, 0)                                                   , 0         , 0                         , 18 ), // #1120
  INST(Xvclz_w          , LasxXX             , (0b0111011010011100000110, 0, 5, 0)                                                   , 0         , 0                         , 19 ), // #1121
  INST(Xvdiv_b          , LasxXXX            , (0b01110100111000000, 0, 5, 10, 0)                                                    , 0         , 0                         , 71 ), // #1122
  INST(Xvdiv_bu         , LasxXXX            , (0b01110100111001000, 0, 5, 10, 0)                                                    , 0         , 0                         , 72 ), // #1123
  INST(Xvdiv_d          , LasxXXX            , (0b01110100111000011, 0, 5, 10, 0)                                                    , 0         , 0                         , 73 ), // #1124
  INST(Xvdiv_du         , LasxXXX            , (0b01110100111001011, 0, 5, 10, 0)                                                    , 0         , 0                         , 74 ), // #1125
  INST(Xvdiv_h          , LasxXXX            , (0b01110100111000001, 0, 5, 10, 0)                                                    , 0         , 0                         , 75 ), // #1126
  INST(Xvdiv_hu         , LasxXXX            , (0b01110100111001001, 0, 5, 10, 0)                                                    , 0         , 0                         , 76 ), // #1127
  INST(Xvdiv_w          , LasxXXX            , (0b01110100111000010, 0, 5, 10, 0)                                                    , 0         , 0                         , 77 ), // #1128
  INST(Xvdiv_wu         , LasxXXX            , (0b01110100111001010, 0, 5, 10, 0)                                                    , 0         , 0                         , 78 ), // #1129
  INST(Xvexth_d_w       , LasxXX             , (0b0111011010011110111010, 0, 5, 0)                                                   , 0         , 0                         , 20 ), // #1130
  INST(Xvexth_du_wu     , LasxXX             , (0b0111011010011110111110, 0, 5, 0)                                                   , 0         , 0                         , 21 ), // #1131
  INST(Xvexth_h_b       , LasxXX             , (0b0111011010011110111000, 0, 5, 0)                                                   , 0         , 0                         , 22 ), // #1132
  INST(Xvexth_hu_bu     , LasxXX             , (0b0111011010011110111100, 0, 5, 0)                                                   , 0         , 0                         , 23 ), // #1133
  INST(Xvexth_q_d       , LasxXX             , (0b0111011010011110111011, 0, 5, 0)                                                   , 0         , 0                         , 24 ), // #1134
  INST(Xvexth_qu_du     , LasxXX             , (0b0111011010011110111111, 0, 5, 0)                                                   , 0         , 0                         , 25 ), // #1135
  INST(Xvexth_w_h       , LasxXX             , (0b0111011010011110111001, 0, 5, 0)                                                   , 0         , 0                         , 26 ), // #1136
  INST(Xvexth_wu_hu     , LasxXX             , (0b0111011010011110111101, 0, 5, 0)                                                   , 0         , 0                         , 27 ), // #1137
  INST(Xvextl_q_d       , LasxXX             , (0b0111011100001001000000, 0, 5, 0)                                                   , 0         , 0                         , 28 ), // #1138
  INST(Xvextl_qu_du     , LasxXX             , (0b0111011100001101000000, 0, 5, 0)                                                   , 0         , 0                         , 29 ), // #1139
  INST(Xvextrins_b      , LasxXXI            , (0b01110111100011, 0, 5, 10, 0)                                                       , 0         , 0                         , 20 ), // #1140
  INST(Xvextrins_d      , LasxXXI            , (0b01110111100000, 0, 5, 10, 0)                                                       , 0         , 0                         , 21 ), // #1141
  INST(Xvextrins_h      , LasxXXI            , (0b01110111100010, 0, 5, 10, 0)                                                       , 0         , 0                         , 22 ), // #1142
  INST(Xvextrins_w      , LasxXXI            , (0b01110111100001, 0, 5, 10, 0)                                                       , 0         , 0                         , 23 ), // #1143
  INST(Xvfadd_d         , LasxXXX            , (0b01110101001100010, 0, 5, 10, 0)                                                    , 0         , 0                         , 79 ), // #1144
  INST(Xvfadd_s         , LasxXXX            , (0b01110101001100001, 0, 5, 10, 0)                                                    , 0         , 0                         , 80 ), // #1145
  INST(Xvfclass_d       , LasxXX             , (0b0111011010011100110110, 0, 5, 0)                                                   , 0         , 0                         , 30 ), // #1146
  INST(Xvfclass_s       , LasxXX             , (0b0111011010011100110101, 0, 5, 0)                                                   , 0         , 0                         , 31 ), // #1147
  INST(Xvfcvt_h_s       , LasxXXX            , (0b01110101010001100, 0, 5, 10, 0)                                                    , 0         , 0                         , 81 ), // #1148
  INST(Xvfcvt_s_d       , LasxXXX            , (0b01110101010001101, 0, 5, 10, 0)                                                    , 0         , 0                         , 82 ), // #1149
  INST(Xvfcvth_d_s      , LasxXX             , (0b0111011010011101111101, 0, 5, 0)                                                   , 0         , 0                         , 32 ), // #1150
  INST(Xvfcvth_s_h      , LasxXX             , (0b0111011010011101111011, 0, 5, 0)                                                   , 0         , 0                         , 33 ), // #1151
  INST(Xvfcvtl_d_s      , LasxXX             , (0b0111011010011101111100, 0, 5, 0)                                                   , 0         , 0                         , 34 ), // #1152
  INST(Xvfcvtl_s_h      , LasxXX             , (0b0111011010011101111010, 0, 5, 0)                                                   , 0         , 0                         , 35 ), // #1153
  INST(Xvfdiv_d         , LasxXXX            , (0b01110101001110110, 0, 5, 10, 0)                                                    , 0         , 0                         , 83 ), // #1154
  INST(Xvfdiv_s         , LasxXXX            , (0b01110101001110101, 0, 5, 10, 0)                                                    , 0         , 0                         , 84 ), // #1155
  INST(Xvffint_d_l      , LasxXX             , (0b0111011010011110000010, 0, 5, 0)                                                   , 0         , 0                         , 36 ), // #1156
  INST(Xvffint_d_lu     , LasxXX             , (0b0111011010011110000011, 0, 5, 0)                                                   , 0         , 0                         , 37 ), // #1157
  INST(Xvffint_s_l      , LasxXXX            , (0b01110101010010000, 0, 5, 10, 0)                                                    , 0         , 0                         , 85 ), // #1158
  INST(Xvffint_s_w      , LasxXX             , (0b0111011010011110000000, 0, 5, 0)                                                   , 0         , 0                         , 38 ), // #1159
  INST(Xvffint_s_wu     , LasxXX             , (0b0111011010011110000001, 0, 5, 0)                                                   , 0         , 0                         , 39 ), // #1160
  INST(Xvffinth_d_w     , LasxXX             , (0b0111011010011110000101, 0, 5, 0)                                                   , 0         , 0                         , 40 ), // #1161
  INST(Xvffintl_d_w     , LasxXX             , (0b0111011010011110000100, 0, 5, 0)                                                   , 0         , 0                         , 41 ), // #1162
  INST(Xvflogb_d        , LasxXX             , (0b0111011010011100110010, 0, 5, 0)                                                   , 0         , 0                         , 42 ), // #1163
  INST(Xvflogb_s        , LasxXX             , (0b0111011010011100110001, 0, 5, 0)                                                   , 0         , 0                         , 43 ), // #1164
  INST(Xvfmadd_d        , LasxXXXX           , (0b000010100010, 0, 5, 10, 15, 0)                                                     , 0         , 0                         , 1  ), // #1165
  INST(Xvfmadd_s        , LasxXXXX           , (0b000010100001, 0, 5, 10, 15, 0)                                                     , 0         , 0                         , 2  ), // #1166
  INST(Xvfmax_d         , LasxXXX            , (0b01110101001111010, 0, 5, 10, 0)                                                    , 0         , 0                         , 86 ), // #1167
  INST(Xvfmax_s         , LasxXXX            , (0b01110101001111001, 0, 5, 10, 0)                                                    , 0         , 0                         , 87 ), // #1168
  INST(Xvfmaxa_d        , LasxXXX            , (0b01110101010000010, 0, 5, 10, 0)                                                    , 0         , 0                         , 88 ), // #1169
  INST(Xvfmaxa_s        , LasxXXX            , (0b01110101010000001, 0, 5, 10, 0)                                                    , 0         , 0                         , 89 ), // #1170
  INST(Xvfmin_d         , LasxXXX            , (0b01110101001111110, 0, 5, 10, 0)                                                    , 0         , 0                         , 90 ), // #1171
  INST(Xvfmin_s         , LasxXXX            , (0b01110101001111101, 0, 5, 10, 0)                                                    , 0         , 0                         , 91 ), // #1172
  INST(Xvfmina_d        , LasxXXX            , (0b01110101010000110, 0, 5, 10, 0)                                                    , 0         , 0                         , 92 ), // #1173
  INST(Xvfmina_s        , LasxXXX            , (0b01110101010000101, 0, 5, 10, 0)                                                    , 0         , 0                         , 93 ), // #1174
  INST(Xvfmsub_d        , LasxXXXX           , (0b000010100110, 0, 5, 10, 15, 0)                                                     , 0         , 0                         , 3  ), // #1175
  INST(Xvfmsub_s        , LasxXXXX           , (0b000010100101, 0, 5, 10, 15, 0)                                                     , 0         , 0                         , 4  ), // #1176
  INST(Xvfmul_d         , LasxXXX            , (0b01110101001110010, 0, 5, 10, 0)                                                    , 0         , 0                         , 94 ), // #1177
  INST(Xvfmul_s         , LasxXXX            , (0b01110101001110001, 0, 5, 10, 0)                                                    , 0         , 0                         , 95 ), // #1178
  INST(Xvfnmadd_d       , LasxXXXX           , (0b000010101010, 0, 5, 10, 15, 0)                                                     , 0         , 0                         , 5  ), // #1179
  INST(Xvfnmadd_s       , LasxXXXX           , (0b000010101001, 0, 5, 10, 15, 0)                                                     , 0         , 0                         , 6  ), // #1180
  INST(Xvfnmsub_d       , LasxXXXX           , (0b000010101110, 0, 5, 10, 15, 0)                                                     , 0         , 0                         , 7  ), // #1181
  INST(Xvfnmsub_s       , LasxXXXX           , (0b000010101101, 0, 5, 10, 15, 0)                                                     , 0         , 0                         , 8  ), // #1182
  INST(Xvfrecip_d       , LasxXX             , (0b0111011010011100111110, 0, 5, 0)                                                   , 0         , 0                         , 44 ), // #1183
  INST(Xvfrecip_s       , LasxXX             , (0b0111011010011100111101, 0, 5, 0)                                                   , 0         , 0                         , 45 ), // #1184
  INST(Xvfrint_d        , LasxXX             , (0b0111011010011101001110, 0, 5, 0)                                                   , 0         , 0                         , 46 ), // #1185
  INST(Xvfrint_s        , LasxXX             , (0b0111011010011101001101, 0, 5, 0)                                                   , 0         , 0                         , 47 ), // #1186
  INST(Xvfrintrm_d      , LasxXX             , (0b0111011010011101010010, 0, 5, 0)                                                   , 0         , 0                         , 48 ), // #1187
  INST(Xvfrintrm_s      , LasxXX             , (0b0111011010011101010001, 0, 5, 0)                                                   , 0         , 0                         , 49 ), // #1188
  INST(Xvfrintrne_d     , LasxXX             , (0b0111011010011101011110, 0, 5, 0)                                                   , 0         , 0                         , 50 ), // #1189
  INST(Xvfrintrne_s     , LasxXX             , (0b0111011010011101011101, 0, 5, 0)                                                   , 0         , 0                         , 51 ), // #1190
  INST(Xvfrintrp_d      , LasxXX             , (0b0111011010011101010110, 0, 5, 0)                                                   , 0         , 0                         , 52 ), // #1191
  INST(Xvfrintrp_s      , LasxXX             , (0b0111011010011101010101, 0, 5, 0)                                                   , 0         , 0                         , 53 ), // #1192
  INST(Xvfrintrz_d      , LasxXX             , (0b0111011010011101011010, 0, 5, 0)                                                   , 0         , 0                         , 54 ), // #1193
  INST(Xvfrintrz_s      , LasxXX             , (0b0111011010011101011001, 0, 5, 0)                                                   , 0         , 0                         , 55 ), // #1194
  INST(Xvfrsqrt_d       , LasxXX             , (0b0111011010011101000010, 0, 5, 0)                                                   , 0         , 0                         , 56 ), // #1195
  INST(Xvfrsqrt_s       , LasxXX             , (0b0111011010011101000001, 0, 5, 0)                                                   , 0         , 0                         , 57 ), // #1196
  INST(Xvfrstp_b        , LasxXXX            , (0b01110101001010110, 0, 5, 10, 0)                                                    , 0         , 0                         , 96 ), // #1197
  INST(Xvfrstp_h        , LasxXXX            , (0b01110101001010111, 0, 5, 10, 0)                                                    , 0         , 0                         , 97 ), // #1198
  INST(Xvfrstpi_b       , LasxXXI            , (0b01110110100110100, 0, 5, 10, 0)                                                    , 0         , 0                         , 24 ), // #1199
  INST(Xvfrstpi_h       , LasxXXI            , (0b01110110100110101, 0, 5, 10, 0)                                                    , 0         , 0                         , 25 ), // #1200
  INST(Xvfscaleb_d      , LasxXXX            , (0b01110101010001010, 0, 5, 10, 0)                                                    , 0         , 0                         , 98 ), // #1201
  INST(Xvfscaleb_s      , LasxXXX            , (0b01110101010001001, 0, 5, 10, 0)                                                    , 0         , 0                         , 99 ), // #1202
  INST(Xvfsqrt_d        , LasxXX             , (0b0111011010011100111010, 0, 5, 0)                                                   , 0         , 0                         , 58 ), // #1203
  INST(Xvfsqrt_s        , LasxXX             , (0b0111011010011100111001, 0, 5, 0)                                                   , 0         , 0                         , 59 ), // #1204
  INST(Xvfsub_d         , LasxXXX            , (0b01110101001100110, 0, 5, 10, 0)                                                    , 0         , 0                         , 100), // #1205
  INST(Xvfsub_s         , LasxXXX            , (0b01110101001100101, 0, 5, 10, 0)                                                    , 0         , 0                         , 101), // #1206
  INST(Xvftint_l_d      , LasxXX             , (0b0111011010011110001101, 0, 5, 0)                                                   , 0         , 0                         , 60 ), // #1207
  INST(Xvftint_lu_d     , LasxXX             , (0b0111011010011110010111, 0, 5, 0)                                                   , 0         , 0                         , 61 ), // #1208
  INST(Xvftint_w_d      , LasxXXX            , (0b01110101010010011, 0, 5, 10, 0)                                                    , 0         , 0                         , 102), // #1209
  INST(Xvftint_w_s      , LasxXX             , (0b0111011010011110001100, 0, 5, 0)                                                   , 0         , 0                         , 62 ), // #1210
  INST(Xvftint_wu_s     , LasxXX             , (0b0111011010011110010110, 0, 5, 0)                                                   , 0         , 0                         , 63 ), // #1211
  INST(Xvftinth_l_s     , LasxXX             , (0b0111011010011110100001, 0, 5, 0)                                                   , 0         , 0                         , 64 ), // #1212
  INST(Xvftintl_l_s     , LasxXX             , (0b0111011010011110100000, 0, 5, 0)                                                   , 0         , 0                         , 65 ), // #1213
  INST(Xvftintrm_l_d    , LasxXX             , (0b0111011010011110001111, 0, 5, 0)                                                   , 0         , 0                         , 66 ), // #1214
  INST(Xvftintrm_w_d    , LasxXXX            , (0b01110101010010100, 0, 5, 10, 0)                                                    , 0         , 0                         , 103), // #1215
  INST(Xvftintrm_w_s    , LasxXX             , (0b0111011010011110001110, 0, 5, 0)                                                   , 0         , 0                         , 67 ), // #1216
  INST(Xvftintrmh_l_s   , LasxXX             , (0b0111011010011110100011, 0, 5, 0)                                                   , 0         , 0                         , 68 ), // #1217
  INST(Xvftintrml_l_s   , LasxXX             , (0b0111011010011110100010, 0, 5, 0)                                                   , 0         , 0                         , 69 ), // #1218
  INST(Xvftintrne_l_d   , LasxXX             , (0b0111011010011110010101, 0, 5, 0)                                                   , 0         , 0                         , 70 ), // #1219
  INST(Xvftintrne_w_d   , LasxXXX            , (0b01110101010010111, 0, 5, 10, 0)                                                    , 0         , 0                         , 104), // #1220
  INST(Xvftintrne_w_s   , LasxXX             , (0b0111011010011110010100, 0, 5, 0)                                                   , 0         , 0                         , 71 ), // #1221
  INST(Xvftintrneh_l_s  , LasxXX             , (0b0111011010011110101001, 0, 5, 0)                                                   , 0         , 0                         , 72 ), // #1222
  INST(Xvftintrnel_l_s  , LasxXX             , (0b0111011010011110101000, 0, 5, 0)                                                   , 0         , 0                         , 73 ), // #1223
  INST(Xvftintrp_l_d    , LasxXX             , (0b0111011010011110010001, 0, 5, 0)                                                   , 0         , 0                         , 74 ), // #1224
  INST(Xvftintrp_w_d    , LasxXXX            , (0b01110101010010101, 0, 5, 10, 0)                                                    , 0         , 0                         , 105), // #1225
  INST(Xvftintrp_w_s    , LasxXX             , (0b0111011010011110010000, 0, 5, 0)                                                   , 0         , 0                         , 75 ), // #1226
  INST(Xvftintrph_l_s   , LasxXX             , (0b0111011010011110100101, 0, 5, 0)                                                   , 0         , 0                         , 76 ), // #1227
  INST(Xvftintrpl_l_s   , LasxXX             , (0b0111011010011110100100, 0, 5, 0)                                                   , 0         , 0                         , 77 ), // #1228
  INST(Xvftintrz_l_d    , LasxXX             , (0b0111011010011110010011, 0, 5, 0)                                                   , 0         , 0                         , 78 ), // #1229
  INST(Xvftintrz_lu_d   , LasxXX             , (0b0111011010011110011101, 0, 5, 0)                                                   , 0         , 0                         , 79 ), // #1230
  INST(Xvftintrz_w_d    , LasxXXX            , (0b01110101010010110, 0, 5, 10, 0)                                                    , 0         , 0                         , 106), // #1231
  INST(Xvftintrz_w_s    , LasxXX             , (0b0111011010011110010010, 0, 5, 0)                                                   , 0         , 0                         , 80 ), // #1232
  INST(Xvftintrz_wu_s   , LasxXX             , (0b0111011010011110011100, 0, 5, 0)                                                   , 0         , 0                         , 81 ), // #1233
  INST(Xvftintrzh_l_s   , LasxXX             , (0b0111011010011110100111, 0, 5, 0)                                                   , 0         , 0                         , 82 ), // #1234
  INST(Xvftintrzl_l_s   , LasxXX             , (0b0111011010011110100110, 0, 5, 0)                                                   , 0         , 0                         , 83 ), // #1235
  INST(Xvhaddw_d_w      , LasxXXX            , (0b01110100010101010, 0, 5, 10, 0)                                                    , 0         , 0                         , 107), // #1236
  INST(Xvhaddw_du_wu    , LasxXXX            , (0b01110100010110010, 0, 5, 10, 0)                                                    , 0         , 0                         , 108), // #1237
  INST(Xvhaddw_h_b      , LasxXXX            , (0b01110100010101000, 0, 5, 10, 0)                                                    , 0         , 0                         , 109), // #1238
  INST(Xvhaddw_hu_bu    , LasxXXX            , (0b01110100010110000, 0, 5, 10, 0)                                                    , 0         , 0                         , 110), // #1239
  INST(Xvhaddw_q_d      , LasxXXX            , (0b01110100010101011, 0, 5, 10, 0)                                                    , 0         , 0                         , 111), // #1240
  INST(Xvhaddw_qu_du    , LasxXXX            , (0b01110100010110011, 0, 5, 10, 0)                                                    , 0         , 0                         , 112), // #1241
  INST(Xvhaddw_w_h      , LasxXXX            , (0b01110100010101001, 0, 5, 10, 0)                                                    , 0         , 0                         , 113), // #1242
  INST(Xvhaddw_wu_hu    , LasxXXX            , (0b01110100010110001, 0, 5, 10, 0)                                                    , 0         , 0                         , 114), // #1243
  INST(Xvhseli_d        , LasxXXI            , (0b01110110100111111, 0, 5, 10, 0)                                                    , 0         , 0                         , 26 ), // #1244
  INST(Xvhsubw_d_w      , LasxXXX            , (0b01110100010101110, 0, 5, 10, 0)                                                    , 0         , 0                         , 115), // #1245
  INST(Xvhsubw_du_wu    , LasxXXX            , (0b01110100010110110, 0, 5, 10, 0)                                                    , 0         , 0                         , 116), // #1246
  INST(Xvhsubw_h_b      , LasxXXX            , (0b01110100010101100, 0, 5, 10, 0)                                                    , 0         , 0                         , 117), // #1247
  INST(Xvhsubw_hu_bu    , LasxXXX            , (0b01110100010110100, 0, 5, 10, 0)                                                    , 0         , 0                         , 118), // #1248
  INST(Xvhsubw_q_d      , LasxXXX            , (0b01110100010101111, 0, 5, 10, 0)                                                    , 0         , 0                         , 119), // #1249
  INST(Xvhsubw_qu_du    , LasxXXX            , (0b01110100010110111, 0, 5, 10, 0)                                                    , 0         , 0                         , 120), // #1250
  INST(Xvhsubw_w_h      , LasxXXX            , (0b01110100010101101, 0, 5, 10, 0)                                                    , 0         , 0                         , 121), // #1251
  INST(Xvhsubw_wu_hu    , LasxXXX            , (0b01110100010110101, 0, 5, 10, 0)                                                    , 0         , 0                         , 122), // #1252
  INST(Xvilvh_b         , LasxXXX            , (0b01110101000111000, 0, 5, 10, 0)                                                    , 0         , 0                         , 123), // #1253
  INST(Xvilvh_d         , LasxXXX            , (0b01110101000111011, 0, 5, 10, 0)                                                    , 0         , 0                         , 124), // #1254
  INST(Xvilvh_h         , LasxXXX            , (0b01110101000111001, 0, 5, 10, 0)                                                    , 0         , 0                         , 125), // #1255
  INST(Xvilvh_w         , LasxXXX            , (0b01110101000111010, 0, 5, 10, 0)                                                    , 0         , 0                         , 126), // #1256
  INST(Xvilvl_b         , LasxXXX            , (0b01110101000110100, 0, 5, 10, 0)                                                    , 0         , 0                         , 127), // #1257
  INST(Xvilvl_d         , LasxXXX            , (0b01110101000110111, 0, 5, 10, 0)                                                    , 0         , 0                         , 128), // #1258
  INST(Xvilvl_h         , LasxXXX            , (0b01110101000110101, 0, 5, 10, 0)                                                    , 0         , 0                         , 129), // #1259
  INST(Xvilvl_w         , LasxXXX            , (0b01110101000110110, 0, 5, 10, 0)                                                    , 0         , 0                         , 130), // #1260
  INST(Xvinsgr2vr_d     , LasxXRI            , (0b01110110111010111110, 0, kWX, 5, 10, 0)                                            , 0         , 0                         , 0  ), // #1261
  INST(Xvinsgr2vr_w     , LasxXRI            , (0b0111011011101011110, 0, kWX, 5, 10, 0)                                             , 0         , 0                         , 1  ), // #1262
  INST(Xvinsve0_d       , LasxXXI            , (0b01110110111111111110, 0, 5, 10, 0)                                                 , 0         , 0                         , 27 ), // #1263
  INST(Xvinsve0_w       , LasxXXI            , (0b0111011011111111110, 0, 5, 10, 0)                                                  , 0         , 0                         , 28 ), // #1264
  INST(Xvld             , SimdLdst           , (0b0010110010,       22, 12)                                                          , 0         , 0                         , 8  ), // #1265
  INST(Xvldi            , LasxXI             , (0b01110111111000, 0, 5, 0)                                                           , 0         , 0                         , 0  ), // #1266
  INST(Xvldrepl_b       , SimdLdst           , (0b0011001010,       22, 12)                                                          , 0         , 0                         , 9  ), // #1267
  INST(Xvldrepl_d       , SimdLdst           , (0b0011001000010,    19, 9 )                                                          , 0         , 0                         , 10 ), // #1268
  INST(Xvldrepl_h       , SimdLdst           , (0b00110010010,      21, 11)                                                          , 0         , 0                         , 11 ), // #1269
  INST(Xvldrepl_w       , SimdLdst           , (0b001100100010,     20, 10)                                                          , 0         , 0                         , 12 ), // #1270
  INST(Xvldx            , SimdLdst           , (0b00111000010010000,15, 0 )                                                          , 0         , 0                         , 13 ), // #1271
  INST(Xvmadd_b         , LasxXXX            , (0b01110100101010000, 0, 5, 10, 0)                                                    , 0         , 0                         , 131), // #1272
  INST(Xvmadd_d         , LasxXXX            , (0b01110100101010011, 0, 5, 10, 0)                                                    , 0         , 0                         , 132), // #1273
  INST(Xvmadd_h         , LasxXXX            , (0b01110100101010001, 0, 5, 10, 0)                                                    , 0         , 0                         , 133), // #1274
  INST(Xvmadd_w         , LasxXXX            , (0b01110100101010010, 0, 5, 10, 0)                                                    , 0         , 0                         , 134), // #1275
  INST(Xvmaddwev_d_w    , LasxXXX            , (0b01110100101011010, 0, 5, 10, 0)                                                    , 0         , 0                         , 135), // #1276
  INST(Xvmaddwev_d_wu   , LasxXXX            , (0b01110100101101010, 0, 5, 10, 0)                                                    , 0         , 0                         , 136), // #1277
  INST(Xvmaddwev_d_wu_w , LasxXXX            , (0b01110100101111010, 0, 5, 10, 0)                                                    , 0         , 0                         , 137), // #1278
  INST(Xvmaddwev_h_b    , LasxXXX            , (0b01110100101011000, 0, 5, 10, 0)                                                    , 0         , 0                         , 138), // #1279
  INST(Xvmaddwev_h_bu   , LasxXXX            , (0b01110100101101000, 0, 5, 10, 0)                                                    , 0         , 0                         , 139), // #1280
  INST(Xvmaddwev_h_bu_b , LasxXXX            , (0b01110100101111000, 0, 5, 10, 0)                                                    , 0         , 0                         , 140), // #1281
  INST(Xvmaddwev_q_d    , LasxXXX            , (0b01110100101011011, 0, 5, 10, 0)                                                    , 0         , 0                         , 141), // #1282
  INST(Xvmaddwev_q_du   , LasxXXX            , (0b01110100101101011, 0, 5, 10, 0)                                                    , 0         , 0                         , 142), // #1283
  INST(Xvmaddwev_q_du_d , LasxXXX            , (0b01110100101111011, 0, 5, 10, 0)                                                    , 0         , 0                         , 143), // #1284
  INST(Xvmaddwev_w_h    , LasxXXX            , (0b01110100101011001, 0, 5, 10, 0)                                                    , 0         , 0                         , 144), // #1285
  INST(Xvmaddwev_w_hu   , LasxXXX            , (0b01110100101101001, 0, 5, 10, 0)                                                    , 0         , 0                         , 145), // #1286
  INST(Xvmaddwev_w_hu_h , LasxXXX            , (0b01110100101111001, 0, 5, 10, 0)                                                    , 0         , 0                         , 146), // #1287
  INST(Xvmaddwod_d_w    , LasxXXX            , (0b01110100101011110, 0, 5, 10, 0)                                                    , 0         , 0                         , 147), // #1288
  INST(Xvmaddwod_d_wu   , LasxXXX            , (0b01110100101101110, 0, 5, 10, 0)                                                    , 0         , 0                         , 148), // #1289
  INST(Xvmaddwod_d_wu_w , LasxXXX            , (0b01110100101111110, 0, 5, 10, 0)                                                    , 0         , 0                         , 149), // #1290
  INST(Xvmaddwod_h_b    , LasxXXX            , (0b01110100101011100, 0, 5, 10, 0)                                                    , 0         , 0                         , 150), // #1291
  INST(Xvmaddwod_h_bu   , LasxXXX            , (0b01110100101101100, 0, 5, 10, 0)                                                    , 0         , 0                         , 151), // #1292
  INST(Xvmaddwod_h_bu_b , LasxXXX            , (0b01110100101111100, 0, 5, 10, 0)                                                    , 0         , 0                         , 152), // #1293
  INST(Xvmaddwod_q_d    , LasxXXX            , (0b01110100101011111, 0, 5, 10, 0)                                                    , 0         , 0                         , 153), // #1294
  INST(Xvmaddwod_q_du   , LasxXXX            , (0b01110100101101111, 0, 5, 10, 0)                                                    , 0         , 0                         , 154), // #1295
  INST(Xvmaddwod_q_du_d , LasxXXX            , (0b01110100101111111, 0, 5, 10, 0)                                                    , 0         , 0                         , 155), // #1296
  INST(Xvmaddwod_w_h    , LasxXXX            , (0b01110100101011101, 0, 5, 10, 0)                                                    , 0         , 0                         , 156), // #1297
  INST(Xvmaddwod_w_hu   , LasxXXX            , (0b01110100101101101, 0, 5, 10, 0)                                                    , 0         , 0                         , 157), // #1298
  INST(Xvmaddwod_w_hu_h , LasxXXX            , (0b01110100101111101, 0, 5, 10, 0)                                                    , 0         , 0                         , 158), // #1299
  INST(Xvmax_b          , LasxXXX            , (0b01110100011100000, 0, 5, 10, 0)                                                    , 0         , 0                         , 159), // #1300
  INST(Xvmax_bu         , LasxXXX            , (0b01110100011101000, 0, 5, 10, 0)                                                    , 0         , 0                         , 160), // #1301
  INST(Xvmax_d          , LasxXXX            , (0b01110100011100011, 0, 5, 10, 0)                                                    , 0         , 0                         , 161), // #1302
  INST(Xvmax_du         , LasxXXX            , (0b01110100011101011, 0, 5, 10, 0)                                                    , 0         , 0                         , 162), // #1303
  INST(Xvmax_h          , LasxXXX            , (0b01110100011100001, 0, 5, 10, 0)                                                    , 0         , 0                         , 163), // #1304
  INST(Xvmax_hu         , LasxXXX            , (0b01110100011101001, 0, 5, 10, 0)                                                    , 0         , 0                         , 164), // #1305
  INST(Xvmax_w          , LasxXXX            , (0b01110100011100010, 0, 5, 10, 0)                                                    , 0         , 0                         , 165), // #1306
  INST(Xvmax_wu         , LasxXXX            , (0b01110100011101010, 0, 5, 10, 0)                                                    , 0         , 0                         , 166), // #1307
  INST(Xvmaxi_b         , LasxXXI            , (0b01110110100100000, 0, 5, 10, 0)                                                    , 0         , 0                         , 29 ), // #1308
  INST(Xvmaxi_bu        , LasxXXI            , (0b01110110100101000, 0, 5, 10, 0)                                                    , 0         , 0                         , 30 ), // #1309
  INST(Xvmaxi_d         , LasxXXI            , (0b01110110100100011, 0, 5, 10, 0)                                                    , 0         , 0                         , 31 ), // #1310
  INST(Xvmaxi_du        , LasxXXI            , (0b01110110100101011, 0, 5, 10, 0)                                                    , 0         , 0                         , 32 ), // #1311
  INST(Xvmaxi_h         , LasxXXI            , (0b01110110100100001, 0, 5, 10, 0)                                                    , 0         , 0                         , 33 ), // #1312
  INST(Xvmaxi_hu        , LasxXXI            , (0b01110110100101001, 0, 5, 10, 0)                                                    , 0         , 0                         , 34 ), // #1313
  INST(Xvmaxi_w         , LasxXXI            , (0b01110110100100010, 0, 5, 10, 0)                                                    , 0         , 0                         , 35 ), // #1314
  INST(Xvmaxi_wu        , LasxXXI            , (0b01110110100101010, 0, 5, 10, 0)                                                    , 0         , 0                         , 36 ), // #1315
  INST(Xvmepatmsk_v     , LasxXII            , (0b01110110100110111, 0, 5, 10, 0)                                                    , 0         , 0                         , 0  ), // #1316
  INST(Xvmin_b          , LasxXXX            , (0b01110100011100100, 0, 5, 10, 0)                                                    , 0         , 0                         , 167), // #1317
  INST(Xvmin_bu         , LasxXXX            , (0b01110100011101100, 0, 5, 10, 0)                                                    , 0         , 0                         , 168), // #1318
  INST(Xvmin_d          , LasxXXX            , (0b01110100011100111, 0, 5, 10, 0)                                                    , 0         , 0                         , 169), // #1319
  INST(Xvmin_du         , LasxXXX            , (0b01110100011101111, 0, 5, 10, 0)                                                    , 0         , 0                         , 170), // #1320
  INST(Xvmin_h          , LasxXXX            , (0b01110100011100101, 0, 5, 10, 0)                                                    , 0         , 0                         , 171), // #1321
  INST(Xvmin_hu         , LasxXXX            , (0b01110100011101101, 0, 5, 10, 0)                                                    , 0         , 0                         , 172), // #1322
  INST(Xvmin_w          , LasxXXX            , (0b01110100011100110, 0, 5, 10, 0)                                                    , 0         , 0                         , 173), // #1323
  INST(Xvmin_wu         , LasxXXX            , (0b01110100011101110, 0, 5, 10, 0)                                                    , 0         , 0                         , 174), // #1324
  INST(Xvmini_b         , LasxXXI            , (0b01110110100100100, 0, 5, 10, 0)                                                    , 0         , 0                         , 37 ), // #1325
  INST(Xvmini_bu        , LasxXXI            , (0b01110110100101100, 0, 5, 10, 0)                                                    , 0         , 0                         , 38 ), // #1326
  INST(Xvmini_d         , LasxXXI            , (0b01110110100100111, 0, 5, 10, 0)                                                    , 0         , 0                         , 39 ), // #1327
  INST(Xvmini_du        , LasxXXI            , (0b01110110100101111, 0, 5, 10, 0)                                                    , 0         , 0                         , 40 ), // #1328
  INST(Xvmini_h         , LasxXXI            , (0b01110110100100101, 0, 5, 10, 0)                                                    , 0         , 0                         , 41 ), // #1329
  INST(Xvmini_hu        , LasxXXI            , (0b01110110100101101, 0, 5, 10, 0)                                                    , 0         , 0                         , 42 ), // #1330
  INST(Xvmini_w         , LasxXXI            , (0b01110110100100110, 0, 5, 10, 0)                                                    , 0         , 0                         , 43 ), // #1331
  INST(Xvmini_wu        , LasxXXI            , (0b01110110100101110, 0, 5, 10, 0)                                                    , 0         , 0                         , 44 ), // #1332
  INST(Xvmod_b          , LasxXXX            , (0b01110100111000100, 0, 5, 10, 0)                                                    , 0         , 0                         , 175), // #1333
  INST(Xvmod_bu         , LasxXXX            , (0b01110100111001100, 0, 5, 10, 0)                                                    , 0         , 0                         , 176), // #1334
  INST(Xvmod_d          , LasxXXX            , (0b01110100111000111, 0, 5, 10, 0)                                                    , 0         , 0                         , 177), // #1335
  INST(Xvmod_du         , LasxXXX            , (0b01110100111001111, 0, 5, 10, 0)                                                    , 0         , 0                         , 178), // #1336
  INST(Xvmod_h          , LasxXXX            , (0b01110100111000101, 0, 5, 10, 0)                                                    , 0         , 0                         , 179), // #1337
  INST(Xvmod_hu         , LasxXXX            , (0b01110100111001101, 0, 5, 10, 0)                                                    , 0         , 0                         , 180), // #1338
  INST(Xvmod_w          , LasxXXX            , (0b01110100111000110, 0, 5, 10, 0)                                                    , 0         , 0                         , 181), // #1339
  INST(Xvmod_wu         , LasxXXX            , (0b01110100111001110, 0, 5, 10, 0)                                                    , 0         , 0                         , 182), // #1340
  INST(Xvmskgez_b       , LasxXX             , (0b0111011010011100010100, 0, 5, 0)                                                   , 0         , 0                         , 84 ), // #1341
  INST(Xvmskltz_b       , LasxXX             , (0b0111011010011100010000, 0, 5, 0)                                                   , 0         , 0                         , 85 ), // #1342
  INST(Xvmskltz_d       , LasxXX             , (0b0111011010011100010011, 0, 5, 0)                                                   , 0         , 0                         , 86 ), // #1343
  INST(Xvmskltz_h       , LasxXX             , (0b0111011010011100010001, 0, 5, 0)                                                   , 0         , 0                         , 87 ), // #1344
  INST(Xvmskltz_w       , LasxXX             , (0b0111011010011100010010, 0, 5, 0)                                                   , 0         , 0                         , 88 ), // #1345
  INST(Xvmsknz_b        , LasxXX             , (0b0111011010011100011000, 0, 5, 0)                                                   , 0         , 0                         , 89 ), // #1346
  INST(Xvmsub_b         , LasxXXX            , (0b01110100101010100, 0, 5, 10, 0)                                                    , 0         , 0                         , 183), // #1347
  INST(Xvmsub_d         , LasxXXX            , (0b01110100101010111, 0, 5, 10, 0)                                                    , 0         , 0                         , 184), // #1348
  INST(Xvmsub_h         , LasxXXX            , (0b01110100101010101, 0, 5, 10, 0)                                                    , 0         , 0                         , 185), // #1349
  INST(Xvmsub_w         , LasxXXX            , (0b01110100101010110, 0, 5, 10, 0)                                                    , 0         , 0                         , 186), // #1350
  INST(Xvmuh_b          , LasxXXX            , (0b01110100100001100, 0, 5, 10, 0)                                                    , 0         , 0                         , 187), // #1351
  INST(Xvmuh_bu         , LasxXXX            , (0b01110100100010000, 0, 5, 10, 0)                                                    , 0         , 0                         , 188), // #1352
  INST(Xvmuh_d          , LasxXXX            , (0b01110100100001111, 0, 5, 10, 0)                                                    , 0         , 0                         , 189), // #1353
  INST(Xvmuh_du         , LasxXXX            , (0b01110100100010011, 0, 5, 10, 0)                                                    , 0         , 0                         , 190), // #1354
  INST(Xvmuh_h          , LasxXXX            , (0b01110100100001101, 0, 5, 10, 0)                                                    , 0         , 0                         , 191), // #1355
  INST(Xvmuh_hu         , LasxXXX            , (0b01110100100010001, 0, 5, 10, 0)                                                    , 0         , 0                         , 192), // #1356
  INST(Xvmuh_w          , LasxXXX            , (0b01110100100001110, 0, 5, 10, 0)                                                    , 0         , 0                         , 193), // #1357
  INST(Xvmuh_wu         , LasxXXX            , (0b01110100100010010, 0, 5, 10, 0)                                                    , 0         , 0                         , 194), // #1358
  INST(Xvmul_b          , LasxXXX            , (0b01110100100001000, 0, 5, 10, 0)                                                    , 0         , 0                         , 195), // #1359
  INST(Xvmul_d          , LasxXXX            , (0b01110100100001011, 0, 5, 10, 0)                                                    , 0         , 0                         , 196), // #1360
  INST(Xvmul_h          , LasxXXX            , (0b01110100100001001, 0, 5, 10, 0)                                                    , 0         , 0                         , 197), // #1361
  INST(Xvmul_w          , LasxXXX            , (0b01110100100001010, 0, 5, 10, 0)                                                    , 0         , 0                         , 198), // #1362
  INST(Xvmulwev_d_w     , LasxXXX            , (0b01110100100100010, 0, 5, 10, 0)                                                    , 0         , 0                         , 199), // #1363
  INST(Xvmulwev_d_wu    , LasxXXX            , (0b01110100100110010, 0, 5, 10, 0)                                                    , 0         , 0                         , 200), // #1364
  INST(Xvmulwev_d_wu_w  , LasxXXX            , (0b01110100101000010, 0, 5, 10, 0)                                                    , 0         , 0                         , 201), // #1365
  INST(Xvmulwev_h_b     , LasxXXX            , (0b01110100100100000, 0, 5, 10, 0)                                                    , 0         , 0                         , 202), // #1366
  INST(Xvmulwev_h_bu    , LasxXXX            , (0b01110100100110000, 0, 5, 10, 0)                                                    , 0         , 0                         , 203), // #1367
  INST(Xvmulwev_h_bu_b  , LasxXXX            , (0b01110100101000000, 0, 5, 10, 0)                                                    , 0         , 0                         , 204), // #1368
  INST(Xvmulwev_q_d     , LasxXXX            , (0b01110100100100011, 0, 5, 10, 0)                                                    , 0         , 0                         , 205), // #1369
  INST(Xvmulwev_q_du    , LasxXXX            , (0b01110100100110011, 0, 5, 10, 0)                                                    , 0         , 0                         , 206), // #1370
  INST(Xvmulwev_q_du_d  , LasxXXX            , (0b01110100101000011, 0, 5, 10, 0)                                                    , 0         , 0                         , 207), // #1371
  INST(Xvmulwev_w_h     , LasxXXX            , (0b01110100100100001, 0, 5, 10, 0)                                                    , 0         , 0                         , 208), // #1372
  INST(Xvmulwev_w_hu    , LasxXXX            , (0b01110100100110001, 0, 5, 10, 0)                                                    , 0         , 0                         , 209), // #1373
  INST(Xvmulwev_w_hu_h  , LasxXXX            , (0b01110100101000001, 0, 5, 10, 0)                                                    , 0         , 0                         , 210), // #1374
  INST(Xvmulwod_d_w     , LasxXXX            , (0b01110100100100110, 0, 5, 10, 0)                                                    , 0         , 0                         , 211), // #1375
  INST(Xvmulwod_d_wu    , LasxXXX            , (0b01110100100110110, 0, 5, 10, 0)                                                    , 0         , 0                         , 212), // #1376
  INST(Xvmulwod_d_wu_w  , LasxXXX            , (0b01110100101000110, 0, 5, 10, 0)                                                    , 0         , 0                         , 213), // #1377
  INST(Xvmulwod_h_b     , LasxXXX            , (0b01110100100100100, 0, 5, 10, 0)                                                    , 0         , 0                         , 214), // #1378
  INST(Xvmulwod_h_bu    , LasxXXX            , (0b01110100100110100, 0, 5, 10, 0)                                                    , 0         , 0                         , 215), // #1379
  INST(Xvmulwod_h_bu_b  , LasxXXX            , (0b01110100101000100, 0, 5, 10, 0)                                                    , 0         , 0                         , 216), // #1380
  INST(Xvmulwod_q_d     , LasxXXX            , (0b01110100100100111, 0, 5, 10, 0)                                                    , 0         , 0                         , 217), // #1381
  INST(Xvmulwod_q_du    , LasxXXX            , (0b01110100100110111, 0, 5, 10, 0)                                                    , 0         , 0                         , 218), // #1382
  INST(Xvmulwod_q_du_d  , LasxXXX            , (0b01110100101000111, 0, 5, 10, 0)                                                    , 0         , 0                         , 219), // #1383
  INST(Xvmulwod_w_h     , LasxXXX            , (0b01110100100100101, 0, 5, 10, 0)                                                    , 0         , 0                         , 220), // #1384
  INST(Xvmulwod_w_hu    , LasxXXX            , (0b01110100100110101, 0, 5, 10, 0)                                                    , 0         , 0                         , 221), // #1385
  INST(Xvmulwod_w_hu_h  , LasxXXX            , (0b01110100101000101, 0, 5, 10, 0)                                                    , 0         , 0                         , 222), // #1386
  INST(Xvneg_b          , LasxXX             , (0b0111011010011100001100, 0, 5, 0)                                                   , 0         , 0                         , 90 ), // #1387
  INST(Xvneg_d          , LasxXX             , (0b0111011010011100001111, 0, 5, 0)                                                   , 0         , 0                         , 91 ), // #1388
  INST(Xvneg_h          , LasxXX             , (0b0111011010011100001101, 0, 5, 0)                                                   , 0         , 0                         , 92 ), // #1389
  INST(Xvneg_w          , LasxXX             , (0b0111011010011100001110, 0, 5, 0)                                                   , 0         , 0                         , 93 ), // #1390
  INST(Xvnor_v          , LasxXXX            , (0b01110101001001111, 0, 5, 10, 0)                                                    , 0         , 0                         , 223), // #1391
  INST(Xvnori_b         , LasxXXI            , (0b01110111110111, 0, 5, 10, 0)                                                       , 0         , 0                         , 45 ), // #1392
  INST(Xvor_v           , LasxXXX            , (0b01110101001001101, 0, 5, 10, 0)                                                    , 0         , 0                         , 224), // #1393
  INST(Xvori_b          , LasxXXI            , (0b01110111110101, 0, 5, 10, 0)                                                       , 0         , 0                         , 46 ), // #1394
  INST(Xvorn_v          , LasxXXX            , (0b01110101001010001, 0, 5, 10, 0)                                                    , 0         , 0                         , 225), // #1395
  INST(Xvpackev_b       , LasxXXX            , (0b01110101000101100, 0, 5, 10, 0)                                                    , 0         , 0                         , 226), // #1396
  INST(Xvpackev_d       , LasxXXX            , (0b01110101000101111, 0, 5, 10, 0)                                                    , 0         , 0                         , 227), // #1397
  INST(Xvpackev_h       , LasxXXX            , (0b01110101000101101, 0, 5, 10, 0)                                                    , 0         , 0                         , 228), // #1398
  INST(Xvpackev_w       , LasxXXX            , (0b01110101000101110, 0, 5, 10, 0)                                                    , 0         , 0                         , 229), // #1399
  INST(Xvpackod_b       , LasxXXX            , (0b01110101000110000, 0, 5, 10, 0)                                                    , 0         , 0                         , 230), // #1400
  INST(Xvpackod_d       , LasxXXX            , (0b01110101000110011, 0, 5, 10, 0)                                                    , 0         , 0                         , 231), // #1401
  INST(Xvpackod_h       , LasxXXX            , (0b01110101000110001, 0, 5, 10, 0)                                                    , 0         , 0                         , 232), // #1402
  INST(Xvpackod_w       , LasxXXX            , (0b01110101000110010, 0, 5, 10, 0)                                                    , 0         , 0                         , 233), // #1403
  INST(Xvpcnt_b         , LasxXX             , (0b0111011010011100001000, 0, 5, 0)                                                   , 0         , 0                         , 94 ), // #1404
  INST(Xvpcnt_d         , LasxXX             , (0b0111011010011100001011, 0, 5, 0)                                                   , 0         , 0                         , 95 ), // #1405
  INST(Xvpcnt_h         , LasxXX             , (0b0111011010011100001001, 0, 5, 0)                                                   , 0         , 0                         , 96 ), // #1406
  INST(Xvpcnt_w         , LasxXX             , (0b0111011010011100001010, 0, 5, 0)                                                   , 0         , 0                         , 97 ), // #1407
  INST(Xvperm_w         , LasxXXX            , (0b01110101011111010, 0, 5, 10, 0)                                                    , 0         , 0                         , 234), // #1408
  INST(Xvpermi_d        , LasxXXI            , (0b01110111111010, 0, 5, 10, 0)                                                       , 0         , 0                         , 47 ), // #1409
  INST(Xvpermi_q        , LasxXXI            , (0b01110111111011, 0, 5, 10, 0)                                                       , 0         , 0                         , 48 ), // #1410
  INST(Xvpermi_w        , LasxXXI            , (0b01110111111001, 0, 5, 10, 0)                                                       , 0         , 0                         , 49 ), // #1411
  INST(Xvpickev_b       , LasxXXX            , (0b01110101000111100, 0, 5, 10, 0)                                                    , 0         , 0                         , 235), // #1412
  INST(Xvpickev_d       , LasxXXX            , (0b01110101000111111, 0, 5, 10, 0)                                                    , 0         , 0                         , 236), // #1413
  INST(Xvpickev_h       , LasxXXX            , (0b01110101000111101, 0, 5, 10, 0)                                                    , 0         , 0                         , 237), // #1414
  INST(Xvpickev_w       , LasxXXX            , (0b01110101000111110, 0, 5, 10, 0)                                                    , 0         , 0                         , 238), // #1415
  INST(Xvpickod_b       , LasxXXX            , (0b01110101001000000, 0, 5, 10, 0)                                                    , 0         , 0                         , 239), // #1416
  INST(Xvpickod_d       , LasxXXX            , (0b01110101001000011, 0, 5, 10, 0)                                                    , 0         , 0                         , 240), // #1417
  INST(Xvpickod_h       , LasxXXX            , (0b01110101001000001, 0, 5, 10, 0)                                                    , 0         , 0                         , 241), // #1418
  INST(Xvpickod_w       , LasxXXX            , (0b01110101001000010, 0, 5, 10, 0)                                                    , 0         , 0                         , 242), // #1419
  INST(Xvpickve2gr_d    , LasxRXI            , (0b01110110111011111110, kWX, 0, 5, 10, 0)                                            , 0         , 0                         , 0  ), // #1420
  INST(Xvpickve2gr_du   , LasxRXI            , (0b01110110111100111110, kWX, 0, 5, 10, 0)                                            , 0         , 0                         , 1  ), // #1421
  INST(Xvpickve2gr_w    , LasxRXI            , (0b0111011011101111110, kWX, 0, 5, 10, 0)                                             , 0         , 0                         , 2  ), // #1422
  INST(Xvpickve2gr_wu   , LasxRXI            , (0b0111011011110011110, kWX, 0, 5, 10, 0)                                             , 0         , 0                         , 3  ), // #1423
  INST(Xvpickve_d       , LasxXXI            , (0b01110111000000111110, 0, 5, 10, 0)                                                 , 0         , 0                         , 50 ), // #1424
  INST(Xvpickve_w       , LasxXXI            , (0b0111011100000011110, 0, 5, 10, 0)                                                  , 0         , 0                         , 51 ), // #1425
  INST(Xvrepl128vei_b   , LasxXXI            , (0b011101101111011110, 0, 5, 10, 0)                                                   , 0         , 0                         , 52 ), // #1426
  INST(Xvrepl128vei_d   , LasxXXI            , (0b011101101111011111110, 0, 5, 10, 0)                                                , 0         , 0                         , 53 ), // #1427
  INST(Xvrepl128vei_h   , LasxXXI            , (0b0111011011110111110, 0, 5, 10, 0)                                                  , 0         , 0                         , 54 ), // #1428
  INST(Xvrepl128vei_w   , LasxXXI            , (0b01110110111101111110, 0, 5, 10, 0)                                                 , 0         , 0                         , 55 ), // #1429
  INST(Xvreplgr2vr_b    , LasxXR             , (0b0111011010011111000000, 0, kWX, 5, 0)                                              , 0         , 0                         , 0  ), // #1430
  INST(Xvreplgr2vr_d    , LasxXR             , (0b0111011010011111000011, 0, kWX, 5, 0)                                              , 0         , 0                         , 1  ), // #1431
  INST(Xvreplgr2vr_h    , LasxXR             , (0b0111011010011111000001, 0, kWX, 5, 0)                                              , 0         , 0                         , 2  ), // #1432
  INST(Xvreplgr2vr_w    , LasxXR             , (0b0111011010011111000010, 0, kWX, 5, 0)                                              , 0         , 0                         , 3  ), // #1433
  INST(Xvreplve0_b      , LasxXX             , (0b0111011100000111000000, 0, 5, 0)                                                   , 0         , 0                         , 98 ), // #1434
  INST(Xvreplve0_d      , LasxXX             , (0b0111011100000111111000, 0, 5, 0)                                                   , 0         , 0                         , 99 ), // #1435
  INST(Xvreplve0_h      , LasxXX             , (0b0111011100000111100000, 0, 5, 0)                                                   , 0         , 0                         , 100), // #1436
  INST(Xvreplve0_q      , LasxXX             , (0b0111011100000111111100, 0, 5, 0)                                                   , 0         , 0                         , 101), // #1437
  INST(Xvreplve0_w      , LasxXX             , (0b0111011100000111110000, 0, 5, 0)                                                   , 0         , 0                         , 102), // #1438
  INST(Xvreplve_b       , LasxXXR            , (0b01110101001000100, 0, 5, kWX, 10, 0)                                               , 0         , 0                         , 0  ), // #1439
  INST(Xvreplve_d       , LasxXXR            , (0b01110101001000111, 0, 5, kWX, 10, 0)                                               , 0         , 0                         , 1  ), // #1440
  INST(Xvreplve_h       , LasxXXR            , (0b01110101001000101, 0, 5, kWX, 10, 0)                                               , 0         , 0                         , 2  ), // #1441
  INST(Xvreplve_w       , LasxXXR            , (0b01110101001000110, 0, 5, kWX, 10, 0)                                               , 0         , 0                         , 3  ), // #1442
  INST(Xvrotr_b         , LasxXXX            , (0b01110100111011100, 0, 5, 10, 0)                                                    , 0         , 0                         , 243), // #1443
  INST(Xvrotr_d         , LasxXXX            , (0b01110100111011111, 0, 5, 10, 0)                                                    , 0         , 0                         , 244), // #1444
  INST(Xvrotr_h         , LasxXXX            , (0b01110100111011101, 0, 5, 10, 0)                                                    , 0         , 0                         , 245), // #1445
  INST(Xvrotr_w         , LasxXXX            , (0b01110100111011110, 0, 5, 10, 0)                                                    , 0         , 0                         , 246), // #1446
  INST(Xvrotri_b        , LasxXXI            , (0b0111011010100000001, 0, 5, 10, 0)                                                  , 0         , 0                         , 56 ), // #1447
  INST(Xvrotri_d        , LasxXXI            , (0b0111011010100001, 0, 5, 10, 0)                                                     , 0         , 0                         , 57 ), // #1448
  INST(Xvrotri_h        , LasxXXI            , (0b011101101010000001, 0, 5, 10, 0)                                                   , 0         , 0                         , 58 ), // #1449
  INST(Xvrotri_w        , LasxXXI            , (0b01110110101000001, 0, 5, 10, 0)                                                    , 0         , 0                         , 59 ), // #1450
  INST(Xvsadd_b         , LasxXXX            , (0b01110100010001100, 0, 5, 10, 0)                                                    , 0         , 0                         , 247), // #1451
  INST(Xvsadd_bu        , LasxXXX            , (0b01110100010010100, 0, 5, 10, 0)                                                    , 0         , 0                         , 248), // #1452
  INST(Xvsadd_d         , LasxXXX            , (0b01110100010001111, 0, 5, 10, 0)                                                    , 0         , 0                         , 249), // #1453
  INST(Xvsadd_du        , LasxXXX            , (0b01110100010010111, 0, 5, 10, 0)                                                    , 0         , 0                         , 250), // #1454
  INST(Xvsadd_h         , LasxXXX            , (0b01110100010001101, 0, 5, 10, 0)                                                    , 0         , 0                         , 251), // #1455
  INST(Xvsadd_hu        , LasxXXX            , (0b01110100010010101, 0, 5, 10, 0)                                                    , 0         , 0                         , 252), // #1456
  INST(Xvsadd_w         , LasxXXX            , (0b01110100010001110, 0, 5, 10, 0)                                                    , 0         , 0                         , 253), // #1457
  INST(Xvsadd_wu        , LasxXXX            , (0b01110100010010110, 0, 5, 10, 0)                                                    , 0         , 0                         , 254), // #1458
  INST(Xvsat_b          , LasxXXI            , (0b0111011100100100001, 0, 5, 10, 0)                                                  , 0         , 0                         , 60 ), // #1459
  INST(Xvsat_bu         , LasxXXI            , (0b0111011100101000001, 0, 5, 10, 0)                                                  , 0         , 0                         , 61 ), // #1460
  INST(Xvsat_d          , LasxXXI            , (0b0111011100100101, 0, 5, 10, 0)                                                     , 0         , 0                         , 62 ), // #1461
  INST(Xvsat_du         , LasxXXI            , (0b0111011100101001, 0, 5, 10, 0)                                                     , 0         , 0                         , 63 ), // #1462
  INST(Xvsat_h          , LasxXXI            , (0b011101110010010001, 0, 5, 10, 0)                                                   , 0         , 0                         , 64 ), // #1463
  INST(Xvsat_hu         , LasxXXI            , (0b011101110010100001, 0, 5, 10, 0)                                                   , 0         , 0                         , 65 ), // #1464
  INST(Xvsat_w          , LasxXXI            , (0b01110111001001001, 0, 5, 10, 0)                                                    , 0         , 0                         , 66 ), // #1465
  INST(Xvsat_wu         , LasxXXI            , (0b01110111001010001, 0, 5, 10, 0)                                                    , 0         , 0                         , 67 ), // #1466
  INST(Xvseq_b          , LasxXXX            , (0b01110100000000000, 0, 5, 10, 0)                                                    , 0         , 0                         , 255), // #1467
  INST(Xvseq_d          , LasxXXX            , (0b01110100000000011, 0, 5, 10, 0)                                                    , 0         , 0                         , 256), // #1468
  INST(Xvseq_h          , LasxXXX            , (0b01110100000000001, 0, 5, 10, 0)                                                    , 0         , 0                         , 257), // #1469
  INST(Xvseq_w          , LasxXXX            , (0b01110100000000010, 0, 5, 10, 0)                                                    , 0         , 0                         , 258), // #1470
  INST(Xvseqi_b         , LasxXXI            , (0b01110110100000000, 0, 5, 10, 0)                                                    , 0         , 0                         , 68 ), // #1471
  INST(Xvseqi_d         , LasxXXI            , (0b01110110100000011, 0, 5, 10, 0)                                                    , 0         , 0                         , 69 ), // #1472
  INST(Xvseqi_h         , LasxXXI            , (0b01110110100000001, 0, 5, 10, 0)                                                    , 0         , 0                         , 70 ), // #1473
  INST(Xvseqi_w         , LasxXXI            , (0b01110110100000010, 0, 5, 10, 0)                                                    , 0         , 0                         , 71 ), // #1474
  INST(Xvsetallnez_b    , LasxIX             , (0b0111011010011100101100, 0, 5, 0)                                                   , 0         , 0                         , 0  ), // #1475
  INST(Xvsetallnez_d    , LasxIX             , (0b0111011010011100101111, 0, 5, 0)                                                   , 0         , 0                         , 1  ), // #1476
  INST(Xvsetallnez_h    , LasxIX             , (0b0111011010011100101101, 0, 5, 0)                                                   , 0         , 0                         , 2  ), // #1477
  INST(Xvsetallnez_w    , LasxIX             , (0b0111011010011100101110, 0, 5, 0)                                                   , 0         , 0                         , 3  ), // #1478
  INST(Xvsetanyeqz_b    , LasxIX             , (0b0111011010011100101000, 0, 5, 0)                                                   , 0         , 0                         , 4  ), // #1479
  INST(Xvsetanyeqz_d    , LasxIX             , (0b0111011010011100101011, 0, 5, 0)                                                   , 0         , 0                         , 5  ), // #1480
  INST(Xvsetanyeqz_h    , LasxIX             , (0b0111011010011100101001, 0, 5, 0)                                                   , 0         , 0                         , 6  ), // #1481
  INST(Xvsetanyeqz_w    , LasxIX             , (0b0111011010011100101010, 0, 5, 0)                                                   , 0         , 0                         , 7  ), // #1482
  INST(Xvseteqz_v       , LasxIX             , (0b0111011010011100100110, 0, 5, 0)                                                   , 0         , 0                         , 8  ), // #1483
  INST(Xvsetnez_v       , LasxIX             , (0b0111011010011100100111, 0, 5, 0)                                                   , 0         , 0                         , 9  ), // #1484
  INST(Xvshuf4i_b       , LasxXXI            , (0b01110111100100, 0, 5, 10, 0)                                                       , 0         , 0                         , 72 ), // #1485
  INST(Xvshuf4i_d       , LasxXXI            , (0b01110111100111, 0, 5, 10, 0)                                                       , 0         , 0                         , 73 ), // #1486
  INST(Xvshuf4i_h       , LasxXXI            , (0b01110111100101, 0, 5, 10, 0)                                                       , 0         , 0                         , 74 ), // #1487
  INST(Xvshuf4i_w       , LasxXXI            , (0b01110111100110, 0, 5, 10, 0)                                                       , 0         , 0                         , 75 ), // #1488
  INST(Xvshuf_b         , LasxXXXX           , (0b000011010110, 0, 5, 10, 15, 0)                                                     , 0         , 0                         , 9  ), // #1489
  INST(Xvshuf_d         , LasxXXX            , (0b01110101011110111, 0, 5, 10, 0)                                                    , 0         , 0                         , 259), // #1490
  INST(Xvshuf_h         , LasxXXX            , (0b01110101011110101, 0, 5, 10, 0)                                                    , 0         , 0                         , 260), // #1491
  INST(Xvshuf_w         , LasxXXX            , (0b01110101011110110, 0, 5, 10, 0)                                                    , 0         , 0                         , 261), // #1492
  INST(Xvsigncov_b      , LasxXXX            , (0b01110101001011100, 0, 5, 10, 0)                                                    , 0         , 0                         , 262), // #1493
  INST(Xvsigncov_d      , LasxXXX            , (0b01110101001011111, 0, 5, 10, 0)                                                    , 0         , 0                         , 263), // #1494
  INST(Xvsigncov_h      , LasxXXX            , (0b01110101001011101, 0, 5, 10, 0)                                                    , 0         , 0                         , 264), // #1495
  INST(Xvsigncov_w      , LasxXXX            , (0b01110101001011110, 0, 5, 10, 0)                                                    , 0         , 0                         , 265), // #1496
  INST(Xvsle_b          , LasxXXX            , (0b01110100000000100, 0, 5, 10, 0)                                                    , 0         , 0                         , 266), // #1497
  INST(Xvsle_bu         , LasxXXX            , (0b01110100000001000, 0, 5, 10, 0)                                                    , 0         , 0                         , 267), // #1498
  INST(Xvsle_d          , LasxXXX            , (0b01110100000000111, 0, 5, 10, 0)                                                    , 0         , 0                         , 268), // #1499
  INST(Xvsle_du         , LasxXXX            , (0b01110100000001011, 0, 5, 10, 0)                                                    , 0         , 0                         , 269), // #1500
  INST(Xvsle_h          , LasxXXX            , (0b01110100000000101, 0, 5, 10, 0)                                                    , 0         , 0                         , 270), // #1501
  INST(Xvsle_hu         , LasxXXX            , (0b01110100000001001, 0, 5, 10, 0)                                                    , 0         , 0                         , 271), // #1502
  INST(Xvsle_w          , LasxXXX            , (0b01110100000000110, 0, 5, 10, 0)                                                    , 0         , 0                         , 272), // #1503
  INST(Xvsle_wu         , LasxXXX            , (0b01110100000001010, 0, 5, 10, 0)                                                    , 0         , 0                         , 273), // #1504
  INST(Xvslei_b         , LasxXXI            , (0b01110110100000100, 0, 5, 10, 0)                                                    , 0         , 0                         , 76 ), // #1505
  INST(Xvslei_bu        , LasxXXI            , (0b01110110100001000, 0, 5, 10, 0)                                                    , 0         , 0                         , 77 ), // #1506
  INST(Xvslei_d         , LasxXXI            , (0b01110110100000111, 0, 5, 10, 0)                                                    , 0         , 0                         , 78 ), // #1507
  INST(Xvslei_du        , LasxXXI            , (0b01110110100001011, 0, 5, 10, 0)                                                    , 0         , 0                         , 79 ), // #1508
  INST(Xvslei_h         , LasxXXI            , (0b01110110100000101, 0, 5, 10, 0)                                                    , 0         , 0                         , 80 ), // #1509
  INST(Xvslei_hu        , LasxXXI            , (0b01110110100001001, 0, 5, 10, 0)                                                    , 0         , 0                         , 81 ), // #1510
  INST(Xvslei_w         , LasxXXI            , (0b01110110100000110, 0, 5, 10, 0)                                                    , 0         , 0                         , 82 ), // #1511
  INST(Xvslei_wu        , LasxXXI            , (0b01110110100001010, 0, 5, 10, 0)                                                    , 0         , 0                         , 83 ), // #1512
  INST(Xvsll_b          , LasxXXX            , (0b01110100111010000, 0, 5, 10, 0)                                                    , 0         , 0                         , 274), // #1513
  INST(Xvsll_d          , LasxXXX            , (0b01110100111010011, 0, 5, 10, 0)                                                    , 0         , 0                         , 275), // #1514
  INST(Xvsll_h          , LasxXXX            , (0b01110100111010001, 0, 5, 10, 0)                                                    , 0         , 0                         , 276), // #1515
  INST(Xvsll_w          , LasxXXX            , (0b01110100111010010, 0, 5, 10, 0)                                                    , 0         , 0                         , 277), // #1516
  INST(Xvslli_b         , LasxXXI            , (0b0111011100101100001, 0, 5, 10, 0)                                                  , 0         , 0                         , 84 ), // #1517
  INST(Xvslli_d         , LasxXXI            , (0b0111011100101101, 0, 5, 10, 0)                                                     , 0         , 0                         , 85 ), // #1518
  INST(Xvslli_h         , LasxXXI            , (0b011101110010110001, 0, 5, 10, 0)                                                   , 0         , 0                         , 86 ), // #1519
  INST(Xvslli_w         , LasxXXI            , (0b01110111001011001, 0, 5, 10, 0)                                                    , 0         , 0                         , 87 ), // #1520
  INST(Xvsllwil_d_w     , LasxXXI            , (0b01110111000010001, 0, 5, 10, 0)                                                    , 0         , 0                         , 88 ), // #1521
  INST(Xvsllwil_du_wu   , LasxXXI            , (0b01110111000011001, 0, 5, 10, 0)                                                    , 0         , 0                         , 89 ), // #1522
  INST(Xvsllwil_h_b     , LasxXXI            , (0b0111011100001000001, 0, 5, 10, 0)                                                  , 0         , 0                         , 90 ), // #1523
  INST(Xvsllwil_hu_bu   , LasxXXI            , (0b0111011100001100001, 0, 5, 10, 0)                                                  , 0         , 0                         , 91 ), // #1524
  INST(Xvsllwil_w_h     , LasxXXI            , (0b011101110000100001, 0, 5, 10, 0)                                                   , 0         , 0                         , 92 ), // #1525
  INST(Xvsllwil_wu_hu   , LasxXXI            , (0b011101110000110001, 0, 5, 10, 0)                                                   , 0         , 0                         , 93 ), // #1526
  INST(Xvslt_b          , LasxXXX            , (0b01110100000001100, 0, 5, 10, 0)                                                    , 0         , 0                         , 278), // #1527
  INST(Xvslt_bu         , LasxXXX            , (0b01110100000010000, 0, 5, 10, 0)                                                    , 0         , 0                         , 279), // #1528
  INST(Xvslt_d          , LasxXXX            , (0b01110100000001111, 0, 5, 10, 0)                                                    , 0         , 0                         , 280), // #1529
  INST(Xvslt_du         , LasxXXX            , (0b01110100000010011, 0, 5, 10, 0)                                                    , 0         , 0                         , 281), // #1530
  INST(Xvslt_h          , LasxXXX            , (0b01110100000001101, 0, 5, 10, 0)                                                    , 0         , 0                         , 282), // #1531
  INST(Xvslt_hu         , LasxXXX            , (0b01110100000010001, 0, 5, 10, 0)                                                    , 0         , 0                         , 283), // #1532
  INST(Xvslt_w          , LasxXXX            , (0b01110100000001110, 0, 5, 10, 0)                                                    , 0         , 0                         , 284), // #1533
  INST(Xvslt_wu         , LasxXXX            , (0b01110100000010010, 0, 5, 10, 0)                                                    , 0         , 0                         , 285), // #1534
  INST(Xvslti_b         , LasxXXI            , (0b01110110100001100, 0, 5, 10, 0)                                                    , 0         , 0                         , 94 ), // #1535
  INST(Xvslti_bu        , LasxXXI            , (0b01110110100010000, 0, 5, 10, 0)                                                    , 0         , 0                         , 95 ), // #1536
  INST(Xvslti_d         , LasxXXI            , (0b01110110100001111, 0, 5, 10, 0)                                                    , 0         , 0                         , 96 ), // #1537
  INST(Xvslti_du        , LasxXXI            , (0b01110110100010011, 0, 5, 10, 0)                                                    , 0         , 0                         , 97 ), // #1538
  INST(Xvslti_h         , LasxXXI            , (0b01110110100001101, 0, 5, 10, 0)                                                    , 0         , 0                         , 98 ), // #1539
  INST(Xvslti_hu        , LasxXXI            , (0b01110110100010001, 0, 5, 10, 0)                                                    , 0         , 0                         , 99 ), // #1540
  INST(Xvslti_w         , LasxXXI            , (0b01110110100001110, 0, 5, 10, 0)                                                    , 0         , 0                         , 100), // #1541
  INST(Xvslti_wu        , LasxXXI            , (0b01110110100010010, 0, 5, 10, 0)                                                    , 0         , 0                         , 101), // #1542
  INST(Xvsra_b          , LasxXXX            , (0b01110100111011000, 0, 5, 10, 0)                                                    , 0         , 0                         , 286), // #1543
  INST(Xvsra_d          , LasxXXX            , (0b01110100111011011, 0, 5, 10, 0)                                                    , 0         , 0                         , 287), // #1544
  INST(Xvsra_h          , LasxXXX            , (0b01110100111011001, 0, 5, 10, 0)                                                    , 0         , 0                         , 288), // #1545
  INST(Xvsra_w          , LasxXXX            , (0b01110100111011010, 0, 5, 10, 0)                                                    , 0         , 0                         , 289), // #1546
  INST(Xvsrai_b         , LasxXXI            , (0b0111011100110100001, 0, 5, 10, 0)                                                  , 0         , 0                         , 102), // #1547
  INST(Xvsrai_d         , LasxXXI            , (0b0111011100110101, 0, 5, 10, 0)                                                     , 0         , 0                         , 103), // #1548
  INST(Xvsrai_h         , LasxXXI            , (0b011101110011010001, 0, 5, 10, 0)                                                   , 0         , 0                         , 104), // #1549
  INST(Xvsrai_w         , LasxXXI            , (0b01110111001101001, 0, 5, 10, 0)                                                    , 0         , 0                         , 105), // #1550
  INST(Xvsran_b_h       , LasxXXX            , (0b01110100111101101, 0, 5, 10, 0)                                                    , 0         , 0                         , 290), // #1551
  INST(Xvsran_h_w       , LasxXXX            , (0b01110100111101110, 0, 5, 10, 0)                                                    , 0         , 0                         , 291), // #1552
  INST(Xvsran_w_d       , LasxXXX            , (0b01110100111101111, 0, 5, 10, 0)                                                    , 0         , 0                         , 292), // #1553
  INST(Xvsrani_b_h      , LasxXXI            , (0b011101110101100001, 0, 5, 10, 0)                                                   , 0         , 0                         , 106), // #1554
  INST(Xvsrani_d_q      , LasxXXI            , (0b011101110101101, 0, 5, 10, 0)                                                      , 0         , 0                         , 107), // #1555
  INST(Xvsrani_h_w      , LasxXXI            , (0b01110111010110001, 0, 5, 10, 0)                                                    , 0         , 0                         , 108), // #1556
  INST(Xvsrani_w_d      , LasxXXI            , (0b0111011101011001, 0, 5, 10, 0)                                                     , 0         , 0                         , 109), // #1557
  INST(Xvsrar_b         , LasxXXX            , (0b01110100111100100, 0, 5, 10, 0)                                                    , 0         , 0                         , 293), // #1558
  INST(Xvsrar_d         , LasxXXX            , (0b01110100111100111, 0, 5, 10, 0)                                                    , 0         , 0                         , 294), // #1559
  INST(Xvsrar_h         , LasxXXX            , (0b01110100111100101, 0, 5, 10, 0)                                                    , 0         , 0                         , 295), // #1560
  INST(Xvsrar_w         , LasxXXX            , (0b01110100111100110, 0, 5, 10, 0)                                                    , 0         , 0                         , 296), // #1561
  INST(Xvsrari_b        , LasxXXI            , (0b0111011010101000001, 0, 5, 10, 0)                                                  , 0         , 0                         , 110), // #1562
  INST(Xvsrari_d        , LasxXXI            , (0b0111011010101001, 0, 5, 10, 0)                                                     , 0         , 0                         , 111), // #1563
  INST(Xvsrari_h        , LasxXXI            , (0b011101101010100001, 0, 5, 10, 0)                                                   , 0         , 0                         , 112), // #1564
  INST(Xvsrari_w        , LasxXXI            , (0b01110110101010001, 0, 5, 10, 0)                                                    , 0         , 0                         , 113), // #1565
  INST(Xvsrarn_b_h      , LasxXXX            , (0b01110100111110101, 0, 5, 10, 0)                                                    , 0         , 0                         , 297), // #1566
  INST(Xvsrarn_h_w      , LasxXXX            , (0b01110100111110110, 0, 5, 10, 0)                                                    , 0         , 0                         , 298), // #1567
  INST(Xvsrarn_w_d      , LasxXXX            , (0b01110100111110111, 0, 5, 10, 0)                                                    , 0         , 0                         , 299), // #1568
  INST(Xvsrarni_b_h     , LasxXXI            , (0b011101110101110001, 0, 5, 10, 0)                                                   , 0         , 0                         , 114), // #1569
  INST(Xvsrarni_d_q     , LasxXXI            , (0b011101110101111, 0, 5, 10, 0)                                                      , 0         , 0                         , 115), // #1570
  INST(Xvsrarni_h_w     , LasxXXI            , (0b01110111010111001, 0, 5, 10, 0)                                                    , 0         , 0                         , 116), // #1571
  INST(Xvsrarni_w_d     , LasxXXI            , (0b0111011101011101, 0, 5, 10, 0)                                                     , 0         , 0                         , 117), // #1572
  INST(Xvsrl_b          , LasxXXX            , (0b01110100111010100, 0, 5, 10, 0)                                                    , 0         , 0                         , 300), // #1573
  INST(Xvsrl_d          , LasxXXX            , (0b01110100111010111, 0, 5, 10, 0)                                                    , 0         , 0                         , 301), // #1574
  INST(Xvsrl_h          , LasxXXX            , (0b01110100111010101, 0, 5, 10, 0)                                                    , 0         , 0                         , 302), // #1575
  INST(Xvsrl_w          , LasxXXX            , (0b01110100111010110, 0, 5, 10, 0)                                                    , 0         , 0                         , 303), // #1576
  INST(Xvsrli_b         , LasxXXI            , (0b0111011100110000001, 0, 5, 10, 0)                                                  , 0         , 0                         , 118), // #1577
  INST(Xvsrli_d         , LasxXXI            , (0b0111011100110001, 0, 5, 10, 0)                                                     , 0         , 0                         , 119), // #1578
  INST(Xvsrli_h         , LasxXXI            , (0b011101110011000001, 0, 5, 10, 0)                                                   , 0         , 0                         , 120), // #1579
  INST(Xvsrli_w         , LasxXXI            , (0b01110111001100001, 0, 5, 10, 0)                                                    , 0         , 0                         , 121), // #1580
  INST(Xvsrln_b_h       , LasxXXX            , (0b01110100111101001, 0, 5, 10, 0)                                                    , 0         , 0                         , 304), // #1581
  INST(Xvsrln_h_w       , LasxXXX            , (0b01110100111101010, 0, 5, 10, 0)                                                    , 0         , 0                         , 305), // #1582
  INST(Xvsrln_w_d       , LasxXXX            , (0b01110100111101011, 0, 5, 10, 0)                                                    , 0         , 0                         , 306), // #1583
  INST(Xvsrlni_b_h      , LasxXXI            , (0b011101110100000001, 0, 5, 10, 0)                                                   , 0         , 0                         , 122), // #1584
  INST(Xvsrlni_d_q      , LasxXXI            , (0b011101110100001, 0, 5, 10, 0)                                                      , 0         , 0                         , 123), // #1585
  INST(Xvsrlni_h_w      , LasxXXI            , (0b01110111010000001, 0, 5, 10, 0)                                                    , 0         , 0                         , 124), // #1586
  INST(Xvsrlni_w_d      , LasxXXI            , (0b0111011101000001, 0, 5, 10, 0)                                                     , 0         , 0                         , 125), // #1587
  INST(Xvsrlr_b         , LasxXXX            , (0b01110100111100000, 0, 5, 10, 0)                                                    , 0         , 0                         , 307), // #1588
  INST(Xvsrlr_d         , LasxXXX            , (0b01110100111100011, 0, 5, 10, 0)                                                    , 0         , 0                         , 308), // #1589
  INST(Xvsrlr_h         , LasxXXX            , (0b01110100111100001, 0, 5, 10, 0)                                                    , 0         , 0                         , 309), // #1590
  INST(Xvsrlr_w         , LasxXXX            , (0b01110100111100010, 0, 5, 10, 0)                                                    , 0         , 0                         , 310), // #1591
  INST(Xvsrlri_b        , LasxXXI            , (0b0111011010100100001, 0, 5, 10, 0)                                                  , 0         , 0                         , 126), // #1592
  INST(Xvsrlri_d        , LasxXXI            , (0b0111011010100101, 0, 5, 10, 0)                                                     , 0         , 0                         , 127), // #1593
  INST(Xvsrlri_h        , LasxXXI            , (0b011101101010010001, 0, 5, 10, 0)                                                   , 0         , 0                         , 128), // #1594
  INST(Xvsrlri_w        , LasxXXI            , (0b01110110101001001, 0, 5, 10, 0)                                                    , 0         , 0                         , 129), // #1595
  INST(Xvsrlrn_b_h      , LasxXXX            , (0b01110100111110001, 0, 5, 10, 0)                                                    , 0         , 0                         , 311), // #1596
  INST(Xvsrlrn_h_w      , LasxXXX            , (0b01110100111110010, 0, 5, 10, 0)                                                    , 0         , 0                         , 312), // #1597
  INST(Xvsrlrn_w_d      , LasxXXX            , (0b01110100111110011, 0, 5, 10, 0)                                                    , 0         , 0                         , 313), // #1598
  INST(Xvsrlrni_b_h     , LasxXXI            , (0b011101110100010001, 0, 5, 10, 0)                                                   , 0         , 0                         , 130), // #1599
  INST(Xvsrlrni_d_q     , LasxXXI            , (0b011101110100011, 0, 5, 10, 0)                                                      , 0         , 0                         , 131), // #1600
  INST(Xvsrlrni_h_w     , LasxXXI            , (0b01110111010001001, 0, 5, 10, 0)                                                    , 0         , 0                         , 132), // #1601
  INST(Xvsrlrni_w_d     , LasxXXI            , (0b0111011101000101, 0, 5, 10, 0)                                                     , 0         , 0                         , 133), // #1602
  INST(Xvssran_b_h      , LasxXXX            , (0b01110100111111101, 0, 5, 10, 0)                                                    , 0         , 0                         , 314), // #1603
  INST(Xvssran_bu_h     , LasxXXX            , (0b01110101000001101, 0, 5, 10, 0)                                                    , 0         , 0                         , 315), // #1604
  INST(Xvssran_h_w      , LasxXXX            , (0b01110100111111110, 0, 5, 10, 0)                                                    , 0         , 0                         , 316), // #1605
  INST(Xvssran_hu_w     , LasxXXX            , (0b01110101000001110, 0, 5, 10, 0)                                                    , 0         , 0                         , 317), // #1606
  INST(Xvssran_w_d      , LasxXXX            , (0b01110100111111111, 0, 5, 10, 0)                                                    , 0         , 0                         , 318), // #1607
  INST(Xvssran_wu_d     , LasxXXX            , (0b01110101000001111, 0, 5, 10, 0)                                                    , 0         , 0                         , 319), // #1608
  INST(Xvssrani_b_h     , LasxXXI            , (0b011101110110000001, 0, 5, 10, 0)                                                   , 0         , 0                         , 134), // #1609
  INST(Xvssrani_bu_h    , LasxXXI            , (0b011101110110010001, 0, 5, 10, 0)                                                   , 0         , 0                         , 135), // #1610
  INST(Xvssrani_d_q     , LasxXXI            , (0b011101110110001, 0, 5, 10, 0)                                                      , 0         , 0                         , 136), // #1611
  INST(Xvssrani_du_q    , LasxXXI            , (0b011101110110011, 0, 5, 10, 0)                                                      , 0         , 0                         , 137), // #1612
  INST(Xvssrani_h_w     , LasxXXI            , (0b01110111011000001, 0, 5, 10, 0)                                                    , 0         , 0                         , 138), // #1613
  INST(Xvssrani_hu_w    , LasxXXI            , (0b01110111011001001, 0, 5, 10, 0)                                                    , 0         , 0                         , 139), // #1614
  INST(Xvssrani_w_d     , LasxXXI            , (0b0111011101100001, 0, 5, 10, 0)                                                     , 0         , 0                         , 140), // #1615
  INST(Xvssrani_wu_d    , LasxXXI            , (0b0111011101100101, 0, 5, 10, 0)                                                     , 0         , 0                         , 141), // #1616
  INST(Xvssrarn_b_h     , LasxXXX            , (0b01110101000000101, 0, 5, 10, 0)                                                    , 0         , 0                         , 320), // #1617
  INST(Xvssrarn_bu_h    , LasxXXX            , (0b01110101000010101, 0, 5, 10, 0)                                                    , 0         , 0                         , 321), // #1618
  INST(Xvssrarn_h_w     , LasxXXX            , (0b01110101000000110, 0, 5, 10, 0)                                                    , 0         , 0                         , 322), // #1619
  INST(Xvssrarn_hu_w    , LasxXXX            , (0b01110101000010110, 0, 5, 10, 0)                                                    , 0         , 0                         , 323), // #1620
  INST(Xvssrarn_w_d     , LasxXXX            , (0b01110101000000111, 0, 5, 10, 0)                                                    , 0         , 0                         , 324), // #1621
  INST(Xvssrarn_wu_d    , LasxXXX            , (0b01110101000010111, 0, 5, 10, 0)                                                    , 0         , 0                         , 325), // #1622
  INST(Xvssrarni_b_h    , LasxXXI            , (0b011101110110100001, 0, 5, 10, 0)                                                   , 0         , 0                         , 142), // #1623
  INST(Xvssrarni_bu_h   , LasxXXI            , (0b011101110110110001, 0, 5, 10, 0)                                                   , 0         , 0                         , 143), // #1624
  INST(Xvssrarni_d_q    , LasxXXI            , (0b011101110110101, 0, 5, 10, 0)                                                      , 0         , 0                         , 144), // #1625
  INST(Xvssrarni_du_q   , LasxXXI            , (0b011101110110111, 0, 5, 10, 0)                                                      , 0         , 0                         , 145), // #1626
  INST(Xvssrarni_h_w    , LasxXXI            , (0b01110111011010001, 0, 5, 10, 0)                                                    , 0         , 0                         , 146), // #1627
  INST(Xvssrarni_hu_w   , LasxXXI            , (0b01110111011011001, 0, 5, 10, 0)                                                    , 0         , 0                         , 147), // #1628
  INST(Xvssrarni_w_d    , LasxXXI            , (0b0111011101101001, 0, 5, 10, 0)                                                     , 0         , 0                         , 148), // #1629
  INST(Xvssrarni_wu_d   , LasxXXI            , (0b0111011101101101, 0, 5, 10, 0)                                                     , 0         , 0                         , 149), // #1630
  INST(Xvssrln_b_h      , LasxXXX            , (0b01110100111111001, 0, 5, 10, 0)                                                    , 0         , 0                         , 326), // #1631
  INST(Xvssrln_bu_h     , LasxXXX            , (0b01110101000001001, 0, 5, 10, 0)                                                    , 0         , 0                         , 327), // #1632
  INST(Xvssrln_h_w      , LasxXXX            , (0b01110100111111010, 0, 5, 10, 0)                                                    , 0         , 0                         , 328), // #1633
  INST(Xvssrln_hu_w     , LasxXXX            , (0b01110101000001010, 0, 5, 10, 0)                                                    , 0         , 0                         , 329), // #1634
  INST(Xvssrln_w_d      , LasxXXX            , (0b01110100111111011, 0, 5, 10, 0)                                                    , 0         , 0                         , 330), // #1635
  INST(Xvssrln_wu_d     , LasxXXX            , (0b01110101000001011, 0, 5, 10, 0)                                                    , 0         , 0                         , 331), // #1636
  INST(Xvssrlni_b_h     , LasxXXI            , (0b011101110100100001, 0, 5, 10, 0)                                                   , 0         , 0                         , 150), // #1637
  INST(Xvssrlni_bu_h    , LasxXXI            , (0b011101110100110001, 0, 5, 10, 0)                                                   , 0         , 0                         , 151), // #1638
  INST(Xvssrlni_d_q     , LasxXXI            , (0b011101110100101, 0, 5, 10, 0)                                                      , 0         , 0                         , 152), // #1639
  INST(Xvssrlni_du_q    , LasxXXI            , (0b011101110100111, 0, 5, 10, 0)                                                      , 0         , 0                         , 153), // #1640
  INST(Xvssrlni_h_w     , LasxXXI            , (0b01110111010010001, 0, 5, 10, 0)                                                    , 0         , 0                         , 154), // #1641
  INST(Xvssrlni_hu_w    , LasxXXI            , (0b01110111010011001, 0, 5, 10, 0)                                                    , 0         , 0                         , 155), // #1642
  INST(Xvssrlni_w_d     , LasxXXI            , (0b0111011101001001, 0, 5, 10, 0)                                                     , 0         , 0                         , 156), // #1643
  INST(Xvssrlni_wu_d    , LasxXXI            , (0b0111011101001101, 0, 5, 10, 0)                                                     , 0         , 0                         , 157), // #1644
  INST(Xvssrlrn_b_h     , LasxXXX            , (0b01110101000000001, 0, 5, 10, 0)                                                    , 0         , 0                         , 332), // #1645
  INST(Xvssrlrn_bu_h    , LasxXXX            , (0b01110101000010001, 0, 5, 10, 0)                                                    , 0         , 0                         , 333), // #1646
  INST(Xvssrlrn_h_w     , LasxXXX            , (0b01110101000000010, 0, 5, 10, 0)                                                    , 0         , 0                         , 334), // #1647
  INST(Xvssrlrn_hu_w    , LasxXXX            , (0b01110101000010010, 0, 5, 10, 0)                                                    , 0         , 0                         , 335), // #1648
  INST(Xvssrlrn_w_d     , LasxXXX            , (0b01110101000000011, 0, 5, 10, 0)                                                    , 0         , 0                         , 336), // #1649
  INST(Xvssrlrn_wu_d    , LasxXXX            , (0b01110101000010011, 0, 5, 10, 0)                                                    , 0         , 0                         , 337), // #1650
  INST(Xvssrlrni_b_h    , LasxXXI            , (0b011101110101000001, 0, 5, 10, 0)                                                   , 0         , 0                         , 158), // #1651
  INST(Xvssrlrni_bu_h   , LasxXXI            , (0b011101110101010001, 0, 5, 10, 0)                                                   , 0         , 0                         , 159), // #1652
  INST(Xvssrlrni_d_q    , LasxXXI            , (0b011101110101001, 0, 5, 10, 0)                                                      , 0         , 0                         , 160), // #1653
  INST(Xvssrlrni_du_q   , LasxXXI            , (0b011101110101011, 0, 5, 10, 0)                                                      , 0         , 0                         , 161), // #1654
  INST(Xvssrlrni_h_w    , LasxXXI            , (0b01110111010100001, 0, 5, 10, 0)                                                    , 0         , 0                         , 162), // #1655
  INST(Xvssrlrni_hu_w   , LasxXXI            , (0b01110111010101001, 0, 5, 10, 0)                                                    , 0         , 0                         , 163), // #1656
  INST(Xvssrlrni_w_d    , LasxXXI            , (0b0111011101010001, 0, 5, 10, 0)                                                     , 0         , 0                         , 164), // #1657
  INST(Xvssrlrni_wu_d   , LasxXXI            , (0b0111011101010101, 0, 5, 10, 0)                                                     , 0         , 0                         , 165), // #1658
  INST(Xvssub_b         , LasxXXX            , (0b01110100010010000, 0, 5, 10, 0)                                                    , 0         , 0                         , 338), // #1659
  INST(Xvssub_bu        , LasxXXX            , (0b01110100010011000, 0, 5, 10, 0)                                                    , 0         , 0                         , 339), // #1660
  INST(Xvssub_d         , LasxXXX            , (0b01110100010010011, 0, 5, 10, 0)                                                    , 0         , 0                         , 340), // #1661
  INST(Xvssub_du        , LasxXXX            , (0b01110100010011011, 0, 5, 10, 0)                                                    , 0         , 0                         , 341), // #1662
  INST(Xvssub_h         , LasxXXX            , (0b01110100010010001, 0, 5, 10, 0)                                                    , 0         , 0                         , 342), // #1663
  INST(Xvssub_hu        , LasxXXX            , (0b01110100010011001, 0, 5, 10, 0)                                                    , 0         , 0                         , 343), // #1664
  INST(Xvssub_w         , LasxXXX            , (0b01110100010010010, 0, 5, 10, 0)                                                    , 0         , 0                         , 344), // #1665
  INST(Xvssub_wu        , LasxXXX            , (0b01110100010011010, 0, 5, 10, 0)                                                    , 0         , 0                         , 345), // #1666
  INST(Xvst             , SimdLdst           , (0b0010110011,        22, 12)                                                         , 0         , 0                         , 14 ), // #1667
  INST(Xvstelm_b        , LasxXRII           , (0b001100111, 0, kWX, 5, 10, 18, 0)                                                   , 0         , 0                         , 0  ), // #1668
  INST(Xvstelm_d        , LasxXRII           , (0b001100110001, 0, kWX, 5, 10, 18, 0)                                                , 0         , 0                         , 1  ), // #1669
  INST(Xvstelm_h        , LasxXRII           , (0b0011001101, 0, kWX, 5, 10, 18, 0)                                                  , 0         , 0                         , 2  ), // #1670
  INST(Xvstelm_w        , LasxXRII           , (0b00110011001, 0, kWX, 5, 10, 18, 0)                                                 , 0         , 0                         , 3  ), // #1671
  INST(Xvstx            , SimdLdst           , (0b00111000010011000, 15, 0 )                                                         , 0         , 0                         , 15 ), // #1672
  INST(Xvsub_b          , LasxXXX            , (0b01110100000011000, 0, 5, 10, 0)                                                    , 0         , 0                         , 346), // #1673
  INST(Xvsub_d          , LasxXXX            , (0b01110100000011011, 0, 5, 10, 0)                                                    , 0         , 0                         , 347), // #1674
  INST(Xvsub_h          , LasxXXX            , (0b01110100000011001, 0, 5, 10, 0)                                                    , 0         , 0                         , 348), // #1675
  INST(Xvsub_q          , LasxXXX            , (0b01110101001011011, 0, 5, 10, 0)                                                    , 0         , 0                         , 349), // #1676
  INST(Xvsub_w          , LasxXXX            , (0b01110100000011010, 0, 5, 10, 0)                                                    , 0         , 0                         , 350), // #1677
  INST(Xvsubi_bu        , LasxXXI            , (0b01110110100011000, 0, 5, 10, 0)                                                    , 0         , 0                         , 166), // #1678
  INST(Xvsubi_du        , LasxXXI            , (0b01110110100011011, 0, 5, 10, 0)                                                    , 0         , 0                         , 167), // #1679
  INST(Xvsubi_hu        , LasxXXI            , (0b01110110100011001, 0, 5, 10, 0)                                                    , 0         , 0                         , 168), // #1680
  INST(Xvsubi_wu        , LasxXXI            , (0b01110110100011010, 0, 5, 10, 0)                                                    , 0         , 0                         , 169), // #1681
  INST(Xvsubwev_d_w     , LasxXXX            , (0b01110100001000010, 0, 5, 10, 0)                                                    , 0         , 0                         , 351), // #1682
  INST(Xvsubwev_d_wu    , LasxXXX            , (0b01110100001100010, 0, 5, 10, 0)                                                    , 0         , 0                         , 352), // #1683
  INST(Xvsubwev_h_b     , LasxXXX            , (0b01110100001000000, 0, 5, 10, 0)                                                    , 0         , 0                         , 353), // #1684
  INST(Xvsubwev_h_bu    , LasxXXX            , (0b01110100001100000, 0, 5, 10, 0)                                                    , 0         , 0                         , 354), // #1685
  INST(Xvsubwev_q_d     , LasxXXX            , (0b01110100001000011, 0, 5, 10, 0)                                                    , 0         , 0                         , 355), // #1686
  INST(Xvsubwev_q_du    , LasxXXX            , (0b01110100001100011, 0, 5, 10, 0)                                                    , 0         , 0                         , 356), // #1687
  INST(Xvsubwev_w_h     , LasxXXX            , (0b01110100001000001, 0, 5, 10, 0)                                                    , 0         , 0                         , 357), // #1688
  INST(Xvsubwev_w_hu    , LasxXXX            , (0b01110100001100001, 0, 5, 10, 0)                                                    , 0         , 0                         , 358), // #1689
  INST(Xvsubwod_d_w     , LasxXXX            , (0b01110100001001010, 0, 5, 10, 0)                                                    , 0         , 0                         , 359), // #1690
  INST(Xvsubwod_d_wu    , LasxXXX            , (0b01110100001101010, 0, 5, 10, 0)                                                    , 0         , 0                         , 360), // #1691
  INST(Xvsubwod_h_b     , LasxXXX            , (0b01110100001001000, 0, 5, 10, 0)                                                    , 0         , 0                         , 361), // #1692
  INST(Xvsubwod_h_bu    , LasxXXX            , (0b01110100001101000, 0, 5, 10, 0)                                                    , 0         , 0                         , 362), // #1693
  INST(Xvsubwod_q_d     , LasxXXX            , (0b01110100001001011, 0, 5, 10, 0)                                                    , 0         , 0                         , 363), // #1694
  INST(Xvsubwod_q_du    , LasxXXX            , (0b01110100001101011, 0, 5, 10, 0)                                                    , 0         , 0                         , 364), // #1695
  INST(Xvsubwod_w_h     , LasxXXX            , (0b01110100001001001, 0, 5, 10, 0)                                                    , 0         , 0                         , 365), // #1696
  INST(Xvsubwod_w_hu    , LasxXXX            , (0b01110100001101001, 0, 5, 10, 0)                                                    , 0         , 0                         , 366), // #1697
  INST(Xvxor_v          , LasxXXX            , (0b01110101001001110, 0, 5, 10, 0)                                                    , 0         , 0                         , 367), // #1698
  INST(Xvxori_b         , LasxXXI            , (0b01110111110110, 0, 5, 10, 0)                                                       , 0         , 0                         , 170), // #1699
  INST(Vfcmp_caf_s      , LsxVVV             , (0b0000110001010, 0, 5, 10, 0)                                                        , 0         , 0                         , 367), // #1700
  INST(Vfcmp_cun_s      , LsxVVV             , (0b0000110001011000, 0, 5, 10, 0)                                                     , 0         , 0                         , 368), // #1701
  INST(Vfcmp_ceq_s      , LsxVVV             , (0b000011000101100, 0, 5, 10, 0)                                                      , 0         , 0                         , 369), // #1702
  INST(Vfcmp_cueq_s     , LsxVVV             , (0b0000110001011100, 0, 5, 10, 0)                                                     , 0         , 0                         , 370), // #1703
  INST(Vfcmp_clt_s      , LsxVVV             , (0b00001100010110, 0, 5, 10, 0)                                                       , 0         , 0                         , 371), // #1704
  INST(Vfcmp_cult_s     , LsxVVV             , (0b0000110001011010, 0, 5, 10, 0)                                                     , 0         , 0                         , 372), // #1705
  INST(Vfcmp_cle_s      , LsxVVV             , (0b000011000101110, 0, 5, 10, 0)                                                      , 0         , 0                         , 373), // #1706
  INST(Vfcmp_cule_s     , LsxVVV             , (0b0000110001011110, 0, 5, 10, 0)                                                     , 0         , 0                         , 374), // #1707
  INST(Vfcmp_cne_s      , LsxVVV             , (0b00001100010110000, 0, 5, 10, 0)                                                    , 0         , 0                         , 375), // #1708
  INST(Vfcmp_cor_s      , LsxVVV             , (0b00001100010110100, 0, 5, 10, 0)                                                    , 0         , 0                         , 376), // #1709
  INST(Vfcmp_cune_s     , LsxVVV             , (0b00001100010111000, 0, 5, 10, 0)                                                    , 0         , 0                         , 377), // #1710
  INST(Vfcmp_saf_s      , LsxVVV             , (0b0000110001011, 0, 5, 10, 0)                                                        , 0         , 0                         , 378), // #1711
  INST(Vfcmp_sun_s      , LsxVVV             , (0b0000110001011001, 0, 5, 10, 0)                                                     , 0         , 0                         , 379), // #1712
  INST(Vfcmp_seq_s      , LsxVVV             , (0b000011000101101, 0, 5, 10, 0)                                                      , 0         , 0                         , 380), // #1713
  INST(Vfcmp_sueq_s     , LsxVVV             , (0b0000110001011101, 0, 5, 10, 0)                                                     , 0         , 0                         , 381), // #1714
  INST(Vfcmp_slt_s      , LsxVVV             , (0b00001100010111, 0, 5, 10, 0)                                                       , 0         , 0                         , 382), // #1715
  INST(Vfcmp_sult_s     , LsxVVV             , (0b0000110001011011, 0, 5, 10, 0)                                                     , 0         , 0                         , 383), // #1716
  INST(Vfcmp_sle_s      , LsxVVV             , (0b000011000101111, 0, 5, 10, 0)                                                      , 0         , 0                         , 384), // #1717
  INST(Vfcmp_sule_s     , LsxVVV             , (0b0000110001011111, 0, 5, 10, 0)                                                     , 0         , 0                         , 385), // #1718
  INST(Vfcmp_sne_s      , LsxVVV             , (0b00001100010110001, 0, 5, 10, 0)                                                    , 0         , 0                         , 386), // #1719
  INST(Vfcmp_sor_s      , LsxVVV             , (0b00001100010110101, 0, 5, 10, 0)                                                    , 0         , 0                         , 387), // #1720
  INST(Vfcmp_sune_s     , LsxVVV             , (0b00001100010111001, 0, 5, 10, 0)                                                    , 0         , 0                         , 388), // #1721
  INST(Vfcmp_caf_d      , LsxVVV             , (0b0000110001100, 0, 5, 10, 0)                                                        , 0         , 0                         , 389), // #1722
  INST(Vfcmp_cun_d      , LsxVVV             , (0b0000110001101000, 0, 5, 10, 0)                                                     , 0         , 0                         , 390), // #1723
  INST(Vfcmp_ceq_d      , LsxVVV             , (0b000011000110100, 0, 5, 10, 0)                                                      , 0         , 0                         , 391), // #1724
  INST(Vfcmp_cueq_d     , LsxVVV             , (0b0000110001101100, 0, 5, 10, 0)                                                     , 0         , 0                         , 392), // #1725
  INST(Vfcmp_clt_d      , LsxVVV             , (0b00001100011010, 0, 5, 10, 0)                                                       , 0         , 0                         , 393), // #1726
  INST(Vfcmp_cult_d     , LsxVVV             , (0b0000110001101010, 0, 5, 10, 0)                                                     , 0         , 0                         , 394), // #1727
  INST(Vfcmp_cle_d      , LsxVVV             , (0b000011000110110, 0, 5, 10, 0)                                                      , 0         , 0                         , 395), // #1728
  INST(Vfcmp_cule_d     , LsxVVV             , (0b0000110001101110, 0, 5, 10, 0)                                                     , 0         , 0                         , 396), // #1729
  INST(Vfcmp_cne_d      , LsxVVV             , (0b00001100011010000, 0, 5, 10, 0)                                                    , 0         , 0                         , 397), // #1730
  INST(Vfcmp_cor_d      , LsxVVV             , (0b00001100011010100, 0, 5, 10, 0)                                                    , 0         , 0                         , 398), // #1731
  INST(Vfcmp_cune_d     , LsxVVV             , (0b00001100011011000, 0, 5, 10, 0)                                                    , 0         , 0                         , 399), // #1732
  INST(Vfcmp_saf_d      , LsxVVV             , (0b0000110001101, 0, 5, 10, 0)                                                        , 0         , 0                         , 400), // #1733
  INST(Vfcmp_sun_d      , LsxVVV             , (0b0000110001101001, 0, 5, 10, 0)                                                     , 0         , 0                         , 401), // #1734
  INST(Vfcmp_seq_d      , LsxVVV             , (0b000011000110101, 0, 5, 10, 0)                                                      , 0         , 0                         , 402), // #1735
  INST(Vfcmp_sueq_d     , LsxVVV             , (0b0000110001101101, 0, 5, 10, 0)                                                     , 0         , 0                         , 403), // #1736
  INST(Vfcmp_slt_d      , LsxVVV             , (0b00001100011011, 0, 5, 10, 0)                                                       , 0         , 0                         , 404), // #1737
  INST(Vfcmp_sult_d     , LsxVVV             , (0b0000110001101011, 0, 5, 10, 0)                                                     , 0         , 0                         , 405), // #1738
  INST(Vfcmp_sle_d      , LsxVVV             , (0b000011000110111, 0, 5, 10, 0)                                                      , 0         , 0                         , 406), // #1739
  INST(Vfcmp_sule_d     , LsxVVV             , (0b0000110001101111, 0, 5, 10, 0)                                                     , 0         , 0                         , 407), // #1740
  INST(Vfcmp_sne_d      , LsxVVV             , (0b00001100011010001, 0, 5, 10, 0)                                                    , 0         , 0                         , 408), // #1741
  INST(Vfcmp_sor_d      , LsxVVV             , (0b00001100011010101, 0, 5, 10, 0)                                                    , 0         , 0                         , 409), // #1742
  INST(Vfcmp_sune_d     , LsxVVV             , (0b00001100011011001, 0, 5, 10, 0)                                                    , 0         , 0                         , 410), // #1743
  INST(Xvfcmp_caf_s     , LasxXXX            , (0b0000110010010, 0, 5, 10, 0)                                                        , 0         , 0                         , 368), // #1744
  INST(Xvfcmp_cun_s     , LasxXXX            , (0b0000110010011000, 0, 5, 10, 0)                                                     , 0         , 0                         , 369), // #1745
  INST(Xvfcmp_ceq_s     , LasxXXX            , (0b000011001001100, 0, 5, 10, 0)                                                      , 0         , 0                         , 370), // #1746
  INST(Xvfcmp_cueq_s    , LasxXXX            , (0b0000110010011100, 0, 5, 10, 0)                                                     , 0         , 0                         , 371), // #1747
  INST(Xvfcmp_clt_s     , LasxXXX            , (0b00001100100110, 0, 5, 10, 0)                                                       , 0         , 0                         , 372), // #1748
  INST(Xvfcmp_cult_s    , LasxXXX            , (0b0000110010011010, 0, 5, 10, 0)                                                     , 0         , 0                         , 373), // #1749
  INST(Xvfcmp_cle_s     , LasxXXX            , (0b000011001001110, 0, 5, 10, 0)                                                      , 0         , 0                         , 374), // #1750
  INST(Xvfcmp_cule_s    , LasxXXX            , (0b0000110010011110, 0, 5, 10, 0)                                                     , 0         , 0                         , 375), // #1751
  INST(Xvfcmp_cne_s     , LasxXXX            , (0b00001100100110000, 0, 5, 10, 0)                                                    , 0         , 0                         , 376), // #1752
  INST(Xvfcmp_cor_s     , LasxXXX            , (0b00001100100110100, 0, 5, 10, 0)                                                    , 0         , 0                         , 377), // #1753
  INST(Xvfcmp_cune_s    , LasxXXX            , (0b00001100100111000, 0, 5, 10, 0)                                                    , 0         , 0                         , 378), // #1754
  INST(Xvfcmp_saf_s     , LasxXXX            , (0b0000110010011, 0, 5, 10, 0)                                                        , 0         , 0                         , 379), // #1755
  INST(Xvfcmp_sun_s     , LasxXXX            , (0b0000110010011001, 0, 5, 10, 0)                                                     , 0         , 0                         , 380), // #1756
  INST(Xvfcmp_seq_s     , LasxXXX            , (0b000011001001101, 0, 5, 10, 0)                                                      , 0         , 0                         , 381), // #1757
  INST(Xvfcmp_sueq_s    , LasxXXX            , (0b0000110010011101, 0, 5, 10, 0)                                                     , 0         , 0                         , 382), // #1758
  INST(Xvfcmp_slt_s     , LasxXXX            , (0b00001100100111, 0, 5, 10, 0)                                                       , 0         , 0                         , 383), // #1759
  INST(Xvfcmp_sult_s    , LasxXXX            , (0b0000110010011011, 0, 5, 10, 0)                                                     , 0         , 0                         , 384), // #1760
  INST(Xvfcmp_sle_s     , LasxXXX            , (0b000011001001111, 0, 5, 10, 0)                                                      , 0         , 0                         , 385), // #1761
  INST(Xvfcmp_sule_s    , LasxXXX            , (0b0000110010011111, 0, 5, 10, 0)                                                     , 0         , 0                         , 386), // #1762
  INST(Xvfcmp_sne_s     , LasxXXX            , (0b00001100100110001, 0, 5, 10, 0)                                                    , 0         , 0                         , 387), // #1763
  INST(Xvfcmp_sor_s     , LasxXXX            , (0b00001100100110101, 0, 5, 10, 0)                                                    , 0         , 0                         , 388), // #1764
  INST(Xvfcmp_sune_s    , LasxXXX            , (0b00001100100111001, 0, 5, 10, 0)                                                    , 0         , 0                         , 389), // #1765
  INST(Xvfcmp_caf_d     , LasxXXX            , (0b0000110010100, 0, 5, 10, 0)                                                        , 0         , 0                         , 390), // #1766
  INST(Xvfcmp_cun_d     , LasxXXX            , (0b0000110010101000, 0, 5, 10, 0)                                                     , 0         , 0                         , 391), // #1767
  INST(Xvfcmp_ceq_d     , LasxXXX            , (0b000011001010100, 0, 5, 10, 0)                                                      , 0         , 0                         , 392), // #1768
  INST(Xvfcmp_cueq_d    , LasxXXX            , (0b0000110010101100, 0, 5, 10, 0)                                                     , 0         , 0                         , 393), // #1769
  INST(Xvfcmp_clt_d     , LasxXXX            , (0b00001100101010, 0, 5, 10, 0)                                                       , 0         , 0                         , 394), // #1770
  INST(Xvfcmp_cult_d    , LasxXXX            , (0b0000110010101010, 0, 5, 10, 0)                                                     , 0         , 0                         , 395), // #1771
  INST(Xvfcmp_cle_d     , LasxXXX            , (0b000011001010110, 0, 5, 10, 0)                                                      , 0         , 0                         , 396), // #1772
  INST(Xvfcmp_cule_d    , LasxXXX            , (0b0000110010101110, 0, 5, 10, 0)                                                     , 0         , 0                         , 397), // #1773
  INST(Xvfcmp_cne_d     , LasxXXX            , (0b00001100101010000, 0, 5, 10, 0)                                                    , 0         , 0                         , 398), // #1774
  INST(Xvfcmp_cor_d     , LasxXXX            , (0b00001100101010100, 0, 5, 10, 0)                                                    , 0         , 0                         , 399), // #1775
  INST(Xvfcmp_cune_d    , LasxXXX            , (0b00001100101011000, 0, 5, 10, 0)                                                    , 0         , 0                         , 400), // #1776
  INST(Xvfcmp_saf_d     , LasxXXX            , (0b0000110010101, 0, 5, 10, 0)                                                        , 0         , 0                         , 401), // #1777
  INST(Xvfcmp_sun_d     , LasxXXX            , (0b0000110010101001, 0, 5, 10, 0)                                                     , 0         , 0                         , 402), // #1778
  INST(Xvfcmp_seq_d     , LasxXXX            , (0b000011001010101, 0, 5, 10, 0)                                                      , 0         , 0                         , 403), // #1779
  INST(Xvfcmp_sueq_d    , LasxXXX            , (0b0000110010101101, 0, 5, 10, 0)                                                     , 0         , 0                         , 404), // #1780
  INST(Xvfcmp_slt_d     , LasxXXX            , (0b00001100101011, 0, 5, 10, 0)                                                       , 0         , 0                         , 405), // #1781
  INST(Xvfcmp_sult_d    , LasxXXX            , (0b0000110010101011, 0, 5, 10, 0)                                                     , 0         , 0                         , 406), // #1782
  INST(Xvfcmp_sle_d     , LasxXXX            , (0b000011001010111, 0, 5, 10, 0)                                                      , 0         , 0                         , 407), // #1783
  INST(Xvfcmp_sule_d    , LasxXXX            , (0b0000110010101111, 0, 5, 10, 0)                                                     , 0         , 0                         , 408), // #1784
  INST(Xvfcmp_sne_d     , LasxXXX            , (0b00001100101010001, 0, 5, 10, 0)                                                    , 0         , 0                         , 409), // #1785
  INST(Xvfcmp_sor_d     , LasxXXX            , (0b00001100101010101, 0, 5, 10, 0)                                                    , 0         , 0                         , 410), // #1786
  INST(Xvfcmp_sune_d    , LasxXXX            , (0b00001100101011001, 0, 5, 10, 0)                                                    , 0         , 0                         , 411)  // #1787
  // ${InstInfo:End}
};

#undef F
#undef INST
#undef NAME_DATA_INDEX

namespace EncodingData {

// ${EncodingData:Begin}
// ------------------- Automatically generated, do not edit -------------------
const BaseBranchRel baseBranchRel[8] = {
  { 0b010100, OffsetType::kTypeLa64_B26 }, // b
  { 0b010110, OffsetType::kTypeLa64_B16 }, // beq
  { 0b011001, OffsetType::kTypeLa64_B16 }, // bge
  { 0b011011, OffsetType::kTypeLa64_B16 }, // bgeu
  { 0b010101, OffsetType::kTypeLa64_B26 }, // bl
  { 0b011000, OffsetType::kTypeLa64_B16 }, // blt
  { 0b011010, OffsetType::kTypeLa64_B16 }, // bltu
  { 0b010111, OffsetType::kTypeLa64_B16 }  // bne
};

const BaseLFIVV baseLFIVV[22] = {
  { 0b00001100000100000, 0, 5, 10, 0 }, // fcmp_caf_s
  { 0b00001100000100100, 0, 5, 10, 0 }, // fcmp_ceq_s
  { 0b00001100001000110, 0, 5, 10, 0 }, // fcmp_cle_d
  { 0b00001100001000010, 0, 5, 10, 0 }, // fcmp_clt_d
  { 0b00001100001010000, 0, 5, 10, 0 }, // fcmp_cne_d
  { 0b00001100000110100, 0, 5, 10, 0 }, // fcmp_cor_s
  { 0b00001100000101100, 0, 5, 10, 0 }, // fcmp_cueq_s
  { 0b00001100000101110, 0, 5, 10, 0 }, // fcmp_cule_s
  { 0b00001100000101010, 0, 5, 10, 0 }, // fcmp_cult_s
  { 0b00001100001001000, 0, 5, 10, 0 }, // fcmp_cun_d
  { 0b00001100001011000, 0, 5, 10, 0 }, // fcmp_cune_d
  { 0b00001100000100001, 0, 5, 10, 0 }, // fcmp_saf_s
  { 0b00001100000100101, 0, 5, 10, 0 }, // fcmp_seq_s
  { 0b00001100000100111, 0, 5, 10, 0 }, // fcmp_sle_s
  { 0b00001100000100011, 0, 5, 10, 0 }, // fcmp_slt_s
  { 0b00001100000110001, 0, 5, 10, 0 }, // fcmp_sne_s
  { 0b00001100001010101, 0, 5, 10, 0 }, // fcmp_sor_d
  { 0b00001100001001101, 0, 5, 10, 0 }, // fcmp_sueq_d
  { 0b00001100001001111, 0, 5, 10, 0 }, // fcmp_sule_d
  { 0b00001100001001011, 0, 5, 10, 0 }, // fcmp_sult_d
  { 0b00001100001001001, 0, 5, 10, 0 }, // fcmp_sun_d
  { 0b00001100000111001, 0, 5, 10, 0 }  // fcmp_sune_s
};

const BaseLIC baseLIC[3] = {
  { 0b00000000001010100, 0 }, // break_
  { 0b00000000001010101, 0 }, // dbcl
  { 0b00000000001010110, 0 }  // syscall
};

const BaseLII baseLII[2] = {
  { 0b010010, 5, 0, 3, 5, 16, 10, 0 }, // bceqz
  { 0b010010, 5, 0, 3, 5, 16, 10, 1 }  // bcnez
};

const BaseLIR baseLIR[1] = {
  { 0b0000000100010100110110, kWX, 5, 0 }  // movgr2cf
};

const BaseLIRR baseLIRR[1] = {
  { 0b00000110010010011, kWX, 5, kWX, 10, 0, 0 }  // invtlb
};

const BaseLIV baseLIV[1] = {
  { 0b0000000100010100110100, 5, 0 }  // movfr2cf
};

const BaseLRI baseLRI[10] = {
  { 0b00000100, kWX, 0, 10, 2 }, // csrrd
  { 0b00000100, kWX, 0, 10, 3 }, // csrwr
  { 0b00000110010001, kWX, 5, 10, 4 }, // ldpte
  { 0b0001010, kWX, 0, 5, 0 }, // lu12i_w
  { 0b0001011, kWX, 0, 5, 0 }, // lu32i_d
  { 0b0000000100010100110111, kWX, 0, 5, 1 }, // movcf2gr
  { 0b0001100, kWX, 0, 5, 0 }, // pcaddi
  { 0b0001110, kWX, 0, 5, 0 }, // pcaddu12i
  { 0b0001111, kWX, 0, 5, 0 }, // pcaddu18i
  { 0b0001101, kWX, 0, 5, 0 }  // pcalau12i
};

const BaseLRR baseLRR[35] = {
  { 0b0000000000000000010010, kWX, 0, kWX, 5, 0 }, // bitrev_4b
  { 0b0000000000000000010011, kWX, 0, kWX, 5, 0 }, // bitrev_8b
  { 0b0000000000000000010101, kWX, 0, kWX, 5, 0 }, // bitrev_d
  { 0b0000000000000000010100, kWX, 0, kWX, 5, 0 }, // bitrev_w
  { 0b0000000000000000001000, kWX, 0, kWX, 5, 0 }, // clo_d
  { 0b0000000000000000000100, kWX, 0, kWX, 5, 0 }, // clo_w
  { 0b0000000000000000001001, kWX, 0, kWX, 5, 0 }, // clz_d
  { 0b0000000000000000000101, kWX, 0, kWX, 5, 0 }, // clz_w
  { 0b0000000000000000011011, kWX, 0, kWX, 5, 0 }, // cpucfg
  { 0b0000000000000000001010, kWX, 0, kWX, 5, 0 }, // cto_d
  { 0b0000000000000000000110, kWX, 0, kWX, 5, 0 }, // cto_w
  { 0b0000000000000000001011, kWX, 0, kWX, 5, 0 }, // ctz_d
  { 0b0000000000000000000111, kWX, 0, kWX, 5, 0 }, // ctz_w
  { 0b0000000000000000010111, kWX, 0, kWX, 5, 0 }, // ext_w_b
  { 0b0000000000000000010110, kWX, 0, kWX, 5, 0 }, // ext_w_h
  { 0b0000011001001000000000, kWX, 0, kWX, 5, 0 }, // iocsrrd_b
  { 0b0000011001001000000011, kWX, 0, kWX, 5, 0 }, // iocsrrd_d
  { 0b0000011001001000000001, kWX, 0, kWX, 5, 0 }, // iocsrrd_h
  { 0b0000011001001000000010, kWX, 0, kWX, 5, 0 }, // iocsrrd_w
  { 0b0000011001001000000100, kWX, 0, kWX, 5, 0 }, // iocsrwr_b
  { 0b0000011001001000000111, kWX, 0, kWX, 5, 0 }, // iocsrwr_d
  { 0b0000011001001000000101, kWX, 0, kWX, 5, 0 }, // iocsrwr_h
  { 0b0000011001001000000110, kWX, 0, kWX, 5, 0 }, // iocsrwr_w
  { 0b0000000100010100110010, kWX, 0, kWX, 5, 0 }, // movfcsr2gr
  { 0b0000000100010100110000, kWX, 0, kWX, 5, 0 }, // movgr2fcsr
  { 0b0000000000010101000000, kWX, 0, kWX, 5, 0 }, // move
  { 0b0000000000000000011010, kWX, 0, kWX, 5, 0 }, // rdtime_d
  { 0b0000000000000000011001, kWX, 0, kWX, 5, 0 }, // rdtimeh_w
  { 0b0000000000000000011000, kWX, 0, kWX, 5, 0 }, // rdtimel_w
  { 0b0000000000000000001100, kWX, 0, kWX, 5, 0 }, // revb_2h
  { 0b0000000000000000001110, kWX, 0, kWX, 5, 0 }, // revb_2w
  { 0b0000000000000000001101, kWX, 0, kWX, 5, 0 }, // revb_4h
  { 0b0000000000000000001111, kWX, 0, kWX, 5, 0 }, // revb_d
  { 0b0000000000000000010000, kWX, 0, kWX, 5, 0 }, // revh_2w
  { 0b0000000000000000010001, kWX, 0, kWX, 5, 0 }  // revh_d
};

const BaseLRRI baseLRRI[18] = {
  { 0b0000001011, kX, 0, kSP, 5, 10, 0 }, // addi_d
  { 0b0000001010, kX, 0, kSP, 5, 10, 0 }, // addi_w
  { 0b0000001101, kX, 0, kSP, 5, 10, 5 }, // andi
  { 0b00000100, kWX, 0, kWX, 5, 10, 3 }, // csrxchg
  { 0b00000110010000, kWX, 0, kWX, 5, 10, 4 }, // lddir
  { 0b0000001100, kX, 0, kSP, 5, 10, 0 }, // lu52i_d
  { 0b0000001110, kX, 0, kSP, 5, 10, 5 }, // ori
  { 0b0000000001001101, kWX, 0, kWX, 5, 10, 2 }, // rotri_d
  { 0b00000000010011001, kWX, 0, kWX, 5, 10, 1 }, // rotri_w
  { 0b0000000001000001, kWX, 0, kWX, 5, 10, 2 }, // slli_d
  { 0b00000000010000001, kWX, 0, kWX, 5, 10, 1 }, // slli_w
  { 0b0000001000, kX, 0, kSP, 5, 10, 0 }, // slti
  { 0b0000001001, kX, 0, kSP, 5, 10, 0 }, // sltui
  { 0b0000000001001001, kWX, 0, kWX, 5, 10, 2 }, // srai_d
  { 0b00000000010010001, kWX, 0, kWX, 5, 10, 1 }, // srai_w
  { 0b0000000001000101, kWX, 0, kWX, 5, 10, 2 }, // srli_d
  { 0b00000000010001001, kWX, 0, kWX, 5, 10, 1 }, // srli_w
  { 0b0000001111, kX, 0, kSP, 5, 10, 5 }  // xori
};

const BaseLRRII baseLRRII[4] = {
  { 0b0000000010, kWX, 0, kWX, 5, 10, 16, 2 }, // bstrins_d
  { 0b00000000011, kWX, 0, kWX, 5, 10, 16, 0 }, // bstrins_w
  { 0b0000000011, kWX, 0, kWX, 5, 10, 16, 2 }, // bstrpick_d
  { 0b00000000011, kWX, 0, kWX, 5, 10, 16, 1 }  // bstrpick_w
};

const BaseLRRIL baseLRRIL[1] = {
  { 0b000100, kX, kSP, 10 }  // addu16i_d
};

const BaseLRRL baseLRRL[2] = {
  { 0b00000000000000011, kWX, 5, kWX, 10 }, // asrtgt_d
  { 0b00000000000000010, kWX, 5, kWX, 10 }  // asrtle_d
};

const BaseLRRR baseLRRR[60] = {
  { 0b00000000000100001, kWX, 0, kWX, 5, kWX, 10, 0 }, // add_d
  { 0b00000000000100000, kWX, 0, kWX, 5, kWX, 10, 0 }, // add_w
  { 0b00000000000101001, kWX, 0, kWX, 5, kWX, 10, 0 }, // and_
  { 0b00000000000101101, kWX, 0, kWX, 5, kWX, 10, 0 }, // andn
  { 0b00000000001001000, kWX, 0, kWX, 5, kWX, 10, 0 }, // crc_w_b_w
  { 0b00000000001001011, kWX, 0, kWX, 5, kWX, 10, 0 }, // crc_w_d_w
  { 0b00000000001001001, kWX, 0, kWX, 5, kWX, 10, 0 }, // crc_w_h_w
  { 0b00000000001001010, kWX, 0, kWX, 5, kWX, 10, 0 }, // crc_w_w_w
  { 0b00000000001001100, kWX, 0, kWX, 5, kWX, 10, 0 }, // crcc_w_b_w
  { 0b00000000001001111, kWX, 0, kWX, 5, kWX, 10, 0 }, // crcc_w_d_w
  { 0b00000000001001101, kWX, 0, kWX, 5, kWX, 10, 0 }, // crcc_w_h_w
  { 0b00000000001001110, kWX, 0, kWX, 5, kWX, 10, 0 }, // crcc_w_w_w
  { 0b00000000001000100, kWX, 0, kWX, 5, kWX, 10, 0 }, // div_d
  { 0b00000000001000110, kWX, 0, kWX, 5, kWX, 10, 0 }, // div_du
  { 0b00000000001000000, kWX, 0, kWX, 5, kWX, 10, 0 }, // div_w
  { 0b00000000001000010, kWX, 0, kWX, 5, kWX, 10, 0 }, // div_wu
  { 0b00111000011110000, kWX, 0, kWX, 5, kWX, 10, 0 }, // ldgt_b
  { 0b00111000011110011, kWX, 0, kWX, 5, kWX, 10, 0 }, // ldgt_d
  { 0b00111000011110001, kWX, 0, kWX, 5, kWX, 10, 0 }, // ldgt_h
  { 0b00111000011110010, kWX, 0, kWX, 5, kWX, 10, 0 }, // ldgt_w
  { 0b00111000011110100, kWX, 0, kWX, 5, kWX, 10, 0 }, // ldle_b
  { 0b00111000011110111, kWX, 0, kWX, 5, kWX, 10, 0 }, // ldle_d
  { 0b00111000011110101, kWX, 0, kWX, 5, kWX, 10, 0 }, // ldle_h
  { 0b00111000011110110, kWX, 0, kWX, 5, kWX, 10, 0 }, // ldle_w
  { 0b00000000000100110, kWX, 0, kWX, 5, kWX, 10, 0 }, // maskeqz
  { 0b00000000000100111, kWX, 0, kWX, 5, kWX, 10, 0 }, // masknez
  { 0b00000000001000101, kWX, 0, kWX, 5, kWX, 10, 0 }, // mod_d
  { 0b00000000001000111, kWX, 0, kWX, 5, kWX, 10, 0 }, // mod_du
  { 0b00000000001000001, kWX, 0, kWX, 5, kWX, 10, 0 }, // mod_w
  { 0b00000000001000011, kWX, 0, kWX, 5, kWX, 10, 0 }, // mod_wu
  { 0b00000000000111011, kWX, 0, kWX, 5, kWX, 10, 0 }, // mul_d
  { 0b00000000000111000, kWX, 0, kWX, 5, kWX, 10, 0 }, // mul_w
  { 0b00000000000111100, kWX, 0, kWX, 5, kWX, 10, 0 }, // mulh_d
  { 0b00000000000111101, kWX, 0, kWX, 5, kWX, 10, 0 }, // mulh_du
  { 0b00000000000111001, kWX, 0, kWX, 5, kWX, 10, 0 }, // mulh_w
  { 0b00000000000111010, kWX, 0, kWX, 5, kWX, 10, 0 }, // mulh_wu
  { 0b00000000000111110, kWX, 0, kWX, 5, kWX, 10, 0 }, // mulw_d_w
  { 0b00000000000111111, kWX, 0, kWX, 5, kWX, 10, 0 }, // mulw_d_wu
  { 0b00000000000101000, kWX, 0, kWX, 5, kWX, 10, 0 }, // nor
  { 0b00000000000101010, kWX, 0, kWX, 5, kWX, 10, 0 }, // or_
  { 0b00000000000101100, kWX, 0, kWX, 5, kWX, 10, 0 }, // orn
  { 0b00000000000110111, kWX, 0, kWX, 5, kWX, 10, 0 }, // rotr_d
  { 0b00000000000110110, kWX, 0, kWX, 5, kWX, 10, 0 }, // rotr_w
  { 0b00000000000110001, kWX, 0, kWX, 5, kWX, 10, 0 }, // sll_d
  { 0b00000000000101110, kWX, 0, kWX, 5, kWX, 10, 0 }, // sll_w
  { 0b00000000000110011, kWX, 0, kWX, 5, kWX, 10, 0 }, // sra_d
  { 0b00000000000110000, kWX, 0, kWX, 5, kWX, 10, 0 }, // sra_w
  { 0b00000000000110010, kWX, 0, kWX, 5, kWX, 10, 0 }, // srl_d
  { 0b00000000000101111, kWX, 0, kWX, 5, kWX, 10, 0 }, // srl_w
  { 0b00111000011111000, kWX, 0, kWX, 5, kWX, 10, 0 }, // stgt_b
  { 0b00111000011111011, kWX, 0, kWX, 5, kWX, 10, 0 }, // stgt_d
  { 0b00111000011111001, kWX, 0, kWX, 5, kWX, 10, 0 }, // stgt_h
  { 0b00111000011111010, kWX, 0, kWX, 5, kWX, 10, 0 }, // stgt_w
  { 0b00111000011111100, kWX, 0, kWX, 5, kWX, 10, 0 }, // stle_b
  { 0b00111000011111111, kWX, 0, kWX, 5, kWX, 10, 0 }, // stle_d
  { 0b00111000011111101, kWX, 0, kWX, 5, kWX, 10, 0 }, // stle_h
  { 0b00111000011111110, kWX, 0, kWX, 5, kWX, 10, 0 }, // stle_w
  { 0b00000000000100011, kWX, 0, kWX, 5, kWX, 10, 0 }, // sub_d
  { 0b00000000000100010, kWX, 0, kWX, 5, kWX, 10, 0 }, // sub_w
  { 0b00000000000101011, kWX, 0, kWX, 5, kWX, 10, 0 }  // xor_
};

const BaseLRRRI baseLRRRI[5] = {
  { 0b000000000010110, kWX, 0, kWX, 5, kWX, 10, 15, 0 }, // alsl_d
  { 0b000000000000010, kWX, 0, kWX, 5, kWX, 10, 15, 0 }, // alsl_w
  { 0b000000000000011, kWX, 0, kWX, 5, kWX, 10, 15, 0 }, // alsl_wu
  { 0b00000000000011, kWX, 0, kWX, 5, kWX, 10, 15, 2 }, // bytepick_d
  { 0b000000000000100, kWX, 0, kWX, 5, kWX, 10, 15, 1 }  // bytepick_w
};

const BaseLRRRT baseLRRRT[36] = {
  { 0b00111000011000011, kWX, 0, kWX, 10, kWX, 5, 0 }, // amadd_d
  { 0b00111000011010101, kWX, 0, kWX, 10, kWX, 5, 0 }, // amadd_db_d
  { 0b00111000011010100, kWX, 0, kWX, 10, kWX, 5, 0 }, // amadd_db_w
  { 0b00111000011000010, kWX, 0, kWX, 10, kWX, 5, 0 }, // amadd_w
  { 0b00111000011000101, kWX, 0, kWX, 10, kWX, 5, 0 }, // amand_d
  { 0b00111000011010111, kWX, 0, kWX, 10, kWX, 5, 0 }, // amand_db_d
  { 0b00111000011010110, kWX, 0, kWX, 10, kWX, 5, 0 }, // amand_db_w
  { 0b00111000011000100, kWX, 0, kWX, 10, kWX, 5, 0 }, // amand_w
  { 0b00111000011001011, kWX, 0, kWX, 10, kWX, 5, 0 }, // ammax_d
  { 0b00111000011011101, kWX, 0, kWX, 10, kWX, 5, 0 }, // ammax_db_d
  { 0b00111000011100001, kWX, 0, kWX, 10, kWX, 5, 0 }, // ammax_db_du
  { 0b00111000011011100, kWX, 0, kWX, 10, kWX, 5, 0 }, // ammax_db_w
  { 0b00111000011100000, kWX, 0, kWX, 10, kWX, 5, 0 }, // ammax_db_wu
  { 0b00111000011001111, kWX, 0, kWX, 10, kWX, 5, 0 }, // ammax_du
  { 0b00111000011001010, kWX, 0, kWX, 10, kWX, 5, 0 }, // ammax_w
  { 0b00111000011001110, kWX, 0, kWX, 10, kWX, 5, 0 }, // ammax_wu
  { 0b00111000011001101, kWX, 0, kWX, 10, kWX, 5, 0 }, // ammin_d
  { 0b00111000011011111, kWX, 0, kWX, 10, kWX, 5, 0 }, // ammin_db_d
  { 0b00111000011100011, kWX, 0, kWX, 10, kWX, 5, 0 }, // ammin_db_du
  { 0b00111000011011110, kWX, 0, kWX, 10, kWX, 5, 0 }, // ammin_db_w
  { 0b00111000011100010, kWX, 0, kWX, 10, kWX, 5, 0 }, // ammin_db_wu
  { 0b00111000011010001, kWX, 0, kWX, 10, kWX, 5, 0 }, // ammin_du
  { 0b00111000011001100, kWX, 0, kWX, 10, kWX, 5, 0 }, // ammin_w
  { 0b00111000011010000, kWX, 0, kWX, 10, kWX, 5, 0 }, // ammin_wu
  { 0b00111000011000111, kWX, 0, kWX, 10, kWX, 5, 0 }, // amor_d
  { 0b00111000011011001, kWX, 0, kWX, 10, kWX, 5, 0 }, // amor_db_d
  { 0b00111000011011000, kWX, 0, kWX, 10, kWX, 5, 0 }, // amor_db_w
  { 0b00111000011000110, kWX, 0, kWX, 10, kWX, 5, 0 }, // amor_w
  { 0b00111000011000001, kWX, 0, kWX, 10, kWX, 5, 0 }, // amswap_d
  { 0b00111000011010011, kWX, 0, kWX, 10, kWX, 5, 0 }, // amswap_db_d
  { 0b00111000011010010, kWX, 0, kWX, 10, kWX, 5, 0 }, // amswap_db_w
  { 0b00111000011000000, kWX, 0, kWX, 10, kWX, 5, 0 }, // amswap_w
  { 0b00111000011001001, kWX, 0, kWX, 10, kWX, 5, 0 }, // amxor_d
  { 0b00111000011011011, kWX, 0, kWX, 10, kWX, 5, 0 }, // amxor_db_d
  { 0b00111000011011010, kWX, 0, kWX, 10, kWX, 5, 0 }, // amxor_db_w
  { 0b00111000011001000, kWX, 0, kWX, 10, kWX, 5, 0 }  // amxor_w
};

const BaseLRV baseLRV[3] = {
  { 0b0000000100010100101110, kWX, 0, 5, 0 }, // movfr2gr_d
  { 0b0000000100010100101101, kWX, 0, 5, 0 }, // movfr2gr_s
  { 0b0000000100010100101111, kWX, 0, 5, 0 }  // movfrh2gr_s
};

const BaseLVI baseLVI[1] = {
  { 0b0000000100010100110101, 0, 5 }  // movcf2fr
};

const BaseLVR baseLVR[3] = {
  { 0b0000000100010100101010, 0, kWX, 5, 0 }, // movgr2fr_d
  { 0b0000000100010100101001, 0, kWX, 5, 0 }, // movgr2fr_w
  { 0b0000000100010100101011, 0, kWX, 5, 0 }  // movgr2frh_w
};

const BaseLVRR baseLVRR[8] = {
  { 0b00111000011101001, 0, kWX, 5, kWX, 10, 0 }, // fldgt_d
  { 0b00111000011101000, 0, kWX, 5, kWX, 10, 0 }, // fldgt_s
  { 0b00111000011101011, 0, kWX, 5, kWX, 10, 0 }, // fldle_d
  { 0b00111000011101010, 0, kWX, 5, kWX, 10, 0 }, // fldle_s
  { 0b00111000011101101, 0, kWX, 5, kWX, 10, 0 }, // fstgt_d
  { 0b00111000011101100, 0, kWX, 5, kWX, 10, 0 }, // fstgt_s
  { 0b00111000011101111, 0, kWX, 5, kWX, 10, 0 }, // fstle_d
  { 0b00111000011101110, 0, kWX, 5, kWX, 10, 0 }  // fstle_s
};

const BaseLVV baseLVV[44] = {
  { 0b0000000100010100000010, 0, 5, 0 }, // fabs_d
  { 0b0000000100010100000001, 0, 5, 0 }, // fabs_s
  { 0b0000000100010100001110, 0, 5, 0 }, // fclass_d
  { 0b0000000100010100001101, 0, 5, 0 }, // fclass_s
  { 0b0000000100011001001001, 0, 5, 0 }, // fcvt_d_s
  { 0b0000000100011001000110, 0, 5, 0 }, // fcvt_s_d
  { 0b0000000100011101001010, 0, 5, 0 }, // ffint_d_l
  { 0b0000000100011101001000, 0, 5, 0 }, // ffint_d_w
  { 0b0000000100011101000110, 0, 5, 0 }, // ffint_s_l
  { 0b0000000100011101000100, 0, 5, 0 }, // ffint_s_w
  { 0b0000000100010100001010, 0, 5, 0 }, // flogb_d
  { 0b0000000100010100001001, 0, 5, 0 }, // flogb_s
  { 0b0000000100010100100110, 0, 5, 0 }, // fmov_d
  { 0b0000000100010100100101, 0, 5, 0 }, // fmov_s
  { 0b0000000100010100000110, 0, 5, 0 }, // fneg_d
  { 0b0000000100010100000101, 0, 5, 0 }, // fneg_s
  { 0b0000000100010100010110, 0, 5, 0 }, // frecip_d
  { 0b0000000100010100010101, 0, 5, 0 }, // frecip_s
  { 0b0000000100011110010010, 0, 5, 0 }, // frint_d
  { 0b0000000100011110010001, 0, 5, 0 }, // frint_s
  { 0b0000000100010100011010, 0, 5, 0 }, // frsqrt_d
  { 0b0000000100010100011001, 0, 5, 0 }, // frsqrt_s
  { 0b0000000100010100010010, 0, 5, 0 }, // fsqrt_d
  { 0b0000000100010100010001, 0, 5, 0 }, // fsqrt_s
  { 0b0000000100011011001010, 0, 5, 0 }, // ftint_l_d
  { 0b0000000100011011001001, 0, 5, 0 }, // ftint_l_s
  { 0b0000000100011011000010, 0, 5, 0 }, // ftint_w_d
  { 0b0000000100011011000001, 0, 5, 0 }, // ftint_w_s
  { 0b0000000100011010001010, 0, 5, 0 }, // ftintrm_l_d
  { 0b0000000100011010001001, 0, 5, 0 }, // ftintrm_l_s
  { 0b0000000100011010000010, 0, 5, 0 }, // ftintrm_w_d
  { 0b0000000100011010000001, 0, 5, 0 }, // ftintrm_w_s
  { 0b0000000100011010111010, 0, 5, 0 }, // ftintrne_l_d
  { 0b0000000100011010111001, 0, 5, 0 }, // ftintrne_l_s
  { 0b0000000100011010110010, 0, 5, 0 }, // ftintrne_w_d
  { 0b0000000100011010110001, 0, 5, 0 }, // ftintrne_w_s
  { 0b0000000100011010011010, 0, 5, 0 }, // ftintrp_l_d
  { 0b0000000100011010011001, 0, 5, 0 }, // ftintrp_l_s
  { 0b0000000100011010010010, 0, 5, 0 }, // ftintrp_w_d
  { 0b0000000100011010010001, 0, 5, 0 }, // ftintrp_w_s
  { 0b0000000100011010101010, 0, 5, 0 }, // ftintrz_l_d
  { 0b0000000100011010101001, 0, 5, 0 }, // ftintrz_l_s
  { 0b0000000100011010100010, 0, 5, 0 }, // ftintrz_w_d
  { 0b0000000100011010100001, 0, 5, 0 }  // ftintrz_w_s
};

const BaseLVVV baseLVVV[20] = {
  { 0b00000001000000010, 0, 5, 10, 0 }, // fadd_d
  { 0b00000001000000001, 0, 5, 10, 0 }, // fadd_s
  { 0b00000001000100110, 0, 5, 10, 0 }, // fcopysign_d
  { 0b00000001000100101, 0, 5, 10, 0 }, // fcopysign_s
  { 0b00000001000001110, 0, 5, 10, 0 }, // fdiv_d
  { 0b00000001000001101, 0, 5, 10, 0 }, // fdiv_s
  { 0b00000001000010010, 0, 5, 10, 0 }, // fmax_d
  { 0b00000001000010001, 0, 5, 10, 0 }, // fmax_s
  { 0b00000001000011010, 0, 5, 10, 0 }, // fmaxa_d
  { 0b00000001000011001, 0, 5, 10, 0 }, // fmaxa_s
  { 0b00000001000010110, 0, 5, 10, 0 }, // fmin_d
  { 0b00000001000010101, 0, 5, 10, 0 }, // fmin_s
  { 0b00000001000011110, 0, 5, 10, 0 }, // fmina_d
  { 0b00000001000011101, 0, 5, 10, 0 }, // fmina_s
  { 0b00000001000001010, 0, 5, 10, 0 }, // fmul_d
  { 0b00000001000001001, 0, 5, 10, 0 }, // fmul_s
  { 0b00000001000100010, 0, 5, 10, 0 }, // fscaleb_d
  { 0b00000001000100001, 0, 5, 10, 0 }, // fscaleb_s
  { 0b00000001000000110, 0, 5, 10, 0 }, // fsub_d
  { 0b00000001000000101, 0, 5, 10, 0 }  // fsub_s
};

const BaseLdSt baseLdSt[30] = {
  { 0b0010101110, 22 }, // fld_d
  { 0b0010101100, 22 }, // fld_s
  { 0b00111000001101000, 15 }, // fldx_d
  { 0b00111000001100000, 15 }, // fldx_s
  { 0b0010101111, 22 }, // fst_d
  { 0b0010101101, 22 }, // fst_s
  { 0b00111000001111000, 15 }, // fstx_d
  { 0b00111000001110000, 15 }, // fstx_s
  { 0b0010100000, 22 }, // ld_b
  { 0b0010101000, 22 }, // ld_bu
  { 0b0010100011, 22 }, // ld_d
  { 0b0010100001, 22 }, // ld_h
  { 0b0010101001, 22 }, // ld_hu
  { 0b0010100010, 22 }, // ld_w
  { 0b0010101010, 22 }, // ld_wu
  { 0b00111000000000000, 15 }, // ldx_b
  { 0b00111000001000000, 15 }, // ldx_bu
  { 0b00111000000011000, 15 }, // ldx_d
  { 0b00111000000001000, 15 }, // ldx_h
  { 0b00111000001001000, 15 }, // ldx_hu
  { 0b00111000000010000, 15 }, // ldx_w
  { 0b00111000001010000, 15 }, // ldx_wu
  { 0b0010100100, 22 }, // st_b
  { 0b0010100111, 22 }, // st_d
  { 0b0010100101, 22 }, // st_h
  { 0b0010100110, 22 }, // st_w
  { 0b00111000000100000, 15 }, // stx_b
  { 0b00111000000111000, 15 }, // stx_d
  { 0b00111000000101000, 15 }, // stx_h
  { 0b00111000000110000, 15 }  // stx_w
};

const BaseOp baseOp[7] = {
  { 0b00000110010010000011100000000000 }, // ertn
  { 0b00000110010010000010000000000000 }, // tlbclr
  { 0b00000110010010000011010000000000 }, // tlbfill
  { 0b00000110010010000010010000000000 }, // tlbflush
  { 0b00000110010010000010110000000000 }, // tlbrd
  { 0b00000110010010000010100000000000 }, // tlbsrch
  { 0b00000110010010000011000000000000 }  // tlbwr
};

const BaseOpImm baseOpImm[3] = {
  { 0b00111000011100100, 15, 0 }, // dbar
  { 0b00111000011100101, 15, 0 }, // ibar
  { 0b00000110010010001, 15, 0 }  // idle
};

const FpLVVVV fpLVVVV[2] = {
  { 0b000010000010 }, // fmadd_d
  { 0b000010000001 }  // fmadd_s
};

const JBTLRRI jBTLRRI[1] = {
  { 0b010011 }  // jirl
};

const LCldst lCldst[1] = {
  { 0b0000011000 }  // cacop
};

const LFPldst lFPldst[8] = {
  { 0b00100110 }, // ldptr_d
  { 0b00100100 }, // ldptr_w
  { 0b00100010 }, // ll_d
  { 0b00100000 }, // ll_w
  { 0b00100011 }, // sc_d
  { 0b00100001 }, // sc_w
  { 0b00100111 }, // stptr_d
  { 0b00100101 }  // stptr_w
};

const LPldst lPldst[1] = {
  { 0b0010101011 }  // preld
};

const LasxIX lasxIX[10] = {
  { 0b0111011010011100101100, 0, 5, 0 }, // xvsetallnez_b
  { 0b0111011010011100101111, 0, 5, 0 }, // xvsetallnez_d
  { 0b0111011010011100101101, 0, 5, 0 }, // xvsetallnez_h
  { 0b0111011010011100101110, 0, 5, 0 }, // xvsetallnez_w
  { 0b0111011010011100101000, 0, 5, 0 }, // xvsetanyeqz_b
  { 0b0111011010011100101011, 0, 5, 0 }, // xvsetanyeqz_d
  { 0b0111011010011100101001, 0, 5, 0 }, // xvsetanyeqz_h
  { 0b0111011010011100101010, 0, 5, 0 }, // xvsetanyeqz_w
  { 0b0111011010011100100110, 0, 5, 0 }, // xvseteqz_v
  { 0b0111011010011100100111, 0, 5, 0 }  // xvsetnez_v
};

const LasxRXI lasxRXI[4] = {
  { 0b01110110111011111110, kWX, 0, 5, 10, 0 }, // xvpickve2gr_d
  { 0b01110110111100111110, kWX, 0, 5, 10, 0 }, // xvpickve2gr_du
  { 0b0111011011101111110, kWX, 0, 5, 10, 0 }, // xvpickve2gr_w
  { 0b0111011011110011110, kWX, 0, 5, 10, 0 }  // xvpickve2gr_wu
};

const LasxXI lasxXI[1] = {
  { 0b01110111111000, 0, 5, 0 }  // xvldi
};

const LasxXII lasxXII[1] = {
  { 0b01110110100110111, 0, 5, 10, 0 }  // xvmepatmsk_v
};

const LasxXR lasxXR[4] = {
  { 0b0111011010011111000000, 0, kWX, 5, 0 }, // xvreplgr2vr_b
  { 0b0111011010011111000011, 0, kWX, 5, 0 }, // xvreplgr2vr_d
  { 0b0111011010011111000001, 0, kWX, 5, 0 }, // xvreplgr2vr_h
  { 0b0111011010011111000010, 0, kWX, 5, 0 }  // xvreplgr2vr_w
};

const LasxXRI lasxXRI[2] = {
  { 0b01110110111010111110, 0, kWX, 5, 10, 0 }, // xvinsgr2vr_d
  { 0b0111011011101011110, 0, kWX, 5, 10, 0 }  // xvinsgr2vr_w
};

const LasxXRII lasxXRII[4] = {
  { 0b001100111, 0, kWX, 5, 10, 18, 0 }, // xvstelm_b
  { 0b001100110001, 0, kWX, 5, 10, 18, 0 }, // xvstelm_d
  { 0b0011001101, 0, kWX, 5, 10, 18, 0 }, // xvstelm_h
  { 0b00110011001, 0, kWX, 5, 10, 18, 0 }  // xvstelm_w
};

const LasxXX lasxXX[103] = {
  { 0b0111011010011111000110, 0, 5, 0 }, // vext2xv_d_b
  { 0b0111011010011111001000, 0, 5, 0 }, // vext2xv_d_h
  { 0b0111011010011111001001, 0, 5, 0 }, // vext2xv_d_w
  { 0b0111011010011111001100, 0, 5, 0 }, // vext2xv_du_bu
  { 0b0111011010011111001110, 0, 5, 0 }, // vext2xv_du_hu
  { 0b0111011010011111001111, 0, 5, 0 }, // vext2xv_du_wu
  { 0b0111011010011111000100, 0, 5, 0 }, // vext2xv_h_b
  { 0b0111011010011111001010, 0, 5, 0 }, // vext2xv_hu_bu
  { 0b0111011010011111000101, 0, 5, 0 }, // vext2xv_w_b
  { 0b0111011010011111000111, 0, 5, 0 }, // vext2xv_w_h
  { 0b0111011010011111001011, 0, 5, 0 }, // vext2xv_wu_bu
  { 0b0111011010011111001101, 0, 5, 0 }, // vext2xv_wu_hu
  { 0b0111011010011100000000, 0, 5, 0 }, // xvclo_b
  { 0b0111011010011100000011, 0, 5, 0 }, // xvclo_d
  { 0b0111011010011100000001, 0, 5, 0 }, // xvclo_h
  { 0b0111011010011100000010, 0, 5, 0 }, // xvclo_w
  { 0b0111011010011100000100, 0, 5, 0 }, // xvclz_b
  { 0b0111011010011100000111, 0, 5, 0 }, // xvclz_d
  { 0b0111011010011100000101, 0, 5, 0 }, // xvclz_h
  { 0b0111011010011100000110, 0, 5, 0 }, // xvclz_w
  { 0b0111011010011110111010, 0, 5, 0 }, // xvexth_d_w
  { 0b0111011010011110111110, 0, 5, 0 }, // xvexth_du_wu
  { 0b0111011010011110111000, 0, 5, 0 }, // xvexth_h_b
  { 0b0111011010011110111100, 0, 5, 0 }, // xvexth_hu_bu
  { 0b0111011010011110111011, 0, 5, 0 }, // xvexth_q_d
  { 0b0111011010011110111111, 0, 5, 0 }, // xvexth_qu_du
  { 0b0111011010011110111001, 0, 5, 0 }, // xvexth_w_h
  { 0b0111011010011110111101, 0, 5, 0 }, // xvexth_wu_hu
  { 0b0111011100001001000000, 0, 5, 0 }, // xvextl_q_d
  { 0b0111011100001101000000, 0, 5, 0 }, // xvextl_qu_du
  { 0b0111011010011100110110, 0, 5, 0 }, // xvfclass_d
  { 0b0111011010011100110101, 0, 5, 0 }, // xvfclass_s
  { 0b0111011010011101111101, 0, 5, 0 }, // xvfcvth_d_s
  { 0b0111011010011101111011, 0, 5, 0 }, // xvfcvth_s_h
  { 0b0111011010011101111100, 0, 5, 0 }, // xvfcvtl_d_s
  { 0b0111011010011101111010, 0, 5, 0 }, // xvfcvtl_s_h
  { 0b0111011010011110000010, 0, 5, 0 }, // xvffint_d_l
  { 0b0111011010011110000011, 0, 5, 0 }, // xvffint_d_lu
  { 0b0111011010011110000000, 0, 5, 0 }, // xvffint_s_w
  { 0b0111011010011110000001, 0, 5, 0 }, // xvffint_s_wu
  { 0b0111011010011110000101, 0, 5, 0 }, // xvffinth_d_w
  { 0b0111011010011110000100, 0, 5, 0 }, // xvffintl_d_w
  { 0b0111011010011100110010, 0, 5, 0 }, // xvflogb_d
  { 0b0111011010011100110001, 0, 5, 0 }, // xvflogb_s
  { 0b0111011010011100111110, 0, 5, 0 }, // xvfrecip_d
  { 0b0111011010011100111101, 0, 5, 0 }, // xvfrecip_s
  { 0b0111011010011101001110, 0, 5, 0 }, // xvfrint_d
  { 0b0111011010011101001101, 0, 5, 0 }, // xvfrint_s
  { 0b0111011010011101010010, 0, 5, 0 }, // xvfrintrm_d
  { 0b0111011010011101010001, 0, 5, 0 }, // xvfrintrm_s
  { 0b0111011010011101011110, 0, 5, 0 }, // xvfrintrne_d
  { 0b0111011010011101011101, 0, 5, 0 }, // xvfrintrne_s
  { 0b0111011010011101010110, 0, 5, 0 }, // xvfrintrp_d
  { 0b0111011010011101010101, 0, 5, 0 }, // xvfrintrp_s
  { 0b0111011010011101011010, 0, 5, 0 }, // xvfrintrz_d
  { 0b0111011010011101011001, 0, 5, 0 }, // xvfrintrz_s
  { 0b0111011010011101000010, 0, 5, 0 }, // xvfrsqrt_d
  { 0b0111011010011101000001, 0, 5, 0 }, // xvfrsqrt_s
  { 0b0111011010011100111010, 0, 5, 0 }, // xvfsqrt_d
  { 0b0111011010011100111001, 0, 5, 0 }, // xvfsqrt_s
  { 0b0111011010011110001101, 0, 5, 0 }, // xvftint_l_d
  { 0b0111011010011110010111, 0, 5, 0 }, // xvftint_lu_d
  { 0b0111011010011110001100, 0, 5, 0 }, // xvftint_w_s
  { 0b0111011010011110010110, 0, 5, 0 }, // xvftint_wu_s
  { 0b0111011010011110100001, 0, 5, 0 }, // xvftinth_l_s
  { 0b0111011010011110100000, 0, 5, 0 }, // xvftintl_l_s
  { 0b0111011010011110001111, 0, 5, 0 }, // xvftintrm_l_d
  { 0b0111011010011110001110, 0, 5, 0 }, // xvftintrm_w_s
  { 0b0111011010011110100011, 0, 5, 0 }, // xvftintrmh_l_s
  { 0b0111011010011110100010, 0, 5, 0 }, // xvftintrml_l_s
  { 0b0111011010011110010101, 0, 5, 0 }, // xvftintrne_l_d
  { 0b0111011010011110010100, 0, 5, 0 }, // xvftintrne_w_s
  { 0b0111011010011110101001, 0, 5, 0 }, // xvftintrneh_l_s
  { 0b0111011010011110101000, 0, 5, 0 }, // xvftintrnel_l_s
  { 0b0111011010011110010001, 0, 5, 0 }, // xvftintrp_l_d
  { 0b0111011010011110010000, 0, 5, 0 }, // xvftintrp_w_s
  { 0b0111011010011110100101, 0, 5, 0 }, // xvftintrph_l_s
  { 0b0111011010011110100100, 0, 5, 0 }, // xvftintrpl_l_s
  { 0b0111011010011110010011, 0, 5, 0 }, // xvftintrz_l_d
  { 0b0111011010011110011101, 0, 5, 0 }, // xvftintrz_lu_d
  { 0b0111011010011110010010, 0, 5, 0 }, // xvftintrz_w_s
  { 0b0111011010011110011100, 0, 5, 0 }, // xvftintrz_wu_s
  { 0b0111011010011110100111, 0, 5, 0 }, // xvftintrzh_l_s
  { 0b0111011010011110100110, 0, 5, 0 }, // xvftintrzl_l_s
  { 0b0111011010011100010100, 0, 5, 0 }, // xvmskgez_b
  { 0b0111011010011100010000, 0, 5, 0 }, // xvmskltz_b
  { 0b0111011010011100010011, 0, 5, 0 }, // xvmskltz_d
  { 0b0111011010011100010001, 0, 5, 0 }, // xvmskltz_h
  { 0b0111011010011100010010, 0, 5, 0 }, // xvmskltz_w
  { 0b0111011010011100011000, 0, 5, 0 }, // xvmsknz_b
  { 0b0111011010011100001100, 0, 5, 0 }, // xvneg_b
  { 0b0111011010011100001111, 0, 5, 0 }, // xvneg_d
  { 0b0111011010011100001101, 0, 5, 0 }, // xvneg_h
  { 0b0111011010011100001110, 0, 5, 0 }, // xvneg_w
  { 0b0111011010011100001000, 0, 5, 0 }, // xvpcnt_b
  { 0b0111011010011100001011, 0, 5, 0 }, // xvpcnt_d
  { 0b0111011010011100001001, 0, 5, 0 }, // xvpcnt_h
  { 0b0111011010011100001010, 0, 5, 0 }, // xvpcnt_w
  { 0b0111011100000111000000, 0, 5, 0 }, // xvreplve0_b
  { 0b0111011100000111111000, 0, 5, 0 }, // xvreplve0_d
  { 0b0111011100000111100000, 0, 5, 0 }, // xvreplve0_h
  { 0b0111011100000111111100, 0, 5, 0 }, // xvreplve0_q
  { 0b0111011100000111110000, 0, 5, 0 }  // xvreplve0_w
};

const LasxXXI lasxXXI[171] = {
  { 0b01110110100010100, 0, 5, 10, 0 }, // xvaddi_bu
  { 0b01110110100010111, 0, 5, 10, 0 }, // xvaddi_du
  { 0b01110110100010101, 0, 5, 10, 0 }, // xvaddi_hu
  { 0b01110110100010110, 0, 5, 10, 0 }, // xvaddi_wu
  { 0b01110111110100, 0, 5, 10, 0 }, // xvandi_b
  { 0b0111011100010000001, 0, 5, 10, 0 }, // xvbitclri_b
  { 0b0111011100010001, 0, 5, 10, 0 }, // xvbitclri_d
  { 0b011101110001000001, 0, 5, 10, 0 }, // xvbitclri_h
  { 0b01110111000100001, 0, 5, 10, 0 }, // xvbitclri_w
  { 0b0111011100011000001, 0, 5, 10, 0 }, // xvbitrevi_b
  { 0b0111011100011001, 0, 5, 10, 0 }, // xvbitrevi_d
  { 0b011101110001100001, 0, 5, 10, 0 }, // xvbitrevi_h
  { 0b01110111000110001, 0, 5, 10, 0 }, // xvbitrevi_w
  { 0b01110111110001, 0, 5, 10, 0 }, // xvbitseli_b
  { 0b0111011100010100001, 0, 5, 10, 0 }, // xvbitseti_b
  { 0b0111011100010101, 0, 5, 10, 0 }, // xvbitseti_d
  { 0b011101110001010001, 0, 5, 10, 0 }, // xvbitseti_h
  { 0b01110111000101001, 0, 5, 10, 0 }, // xvbitseti_w
  { 0b01110110100011100, 0, 5, 10, 0 }, // xvbsll_v
  { 0b01110110100011101, 0, 5, 10, 0 }, // xvbsrl_v
  { 0b01110111100011, 0, 5, 10, 0 }, // xvextrins_b
  { 0b01110111100000, 0, 5, 10, 0 }, // xvextrins_d
  { 0b01110111100010, 0, 5, 10, 0 }, // xvextrins_h
  { 0b01110111100001, 0, 5, 10, 0 }, // xvextrins_w
  { 0b01110110100110100, 0, 5, 10, 0 }, // xvfrstpi_b
  { 0b01110110100110101, 0, 5, 10, 0 }, // xvfrstpi_h
  { 0b01110110100111111, 0, 5, 10, 0 }, // xvhseli_d
  { 0b01110110111111111110, 0, 5, 10, 0 }, // xvinsve0_d
  { 0b0111011011111111110, 0, 5, 10, 0 }, // xvinsve0_w
  { 0b01110110100100000, 0, 5, 10, 0 }, // xvmaxi_b
  { 0b01110110100101000, 0, 5, 10, 0 }, // xvmaxi_bu
  { 0b01110110100100011, 0, 5, 10, 0 }, // xvmaxi_d
  { 0b01110110100101011, 0, 5, 10, 0 }, // xvmaxi_du
  { 0b01110110100100001, 0, 5, 10, 0 }, // xvmaxi_h
  { 0b01110110100101001, 0, 5, 10, 0 }, // xvmaxi_hu
  { 0b01110110100100010, 0, 5, 10, 0 }, // xvmaxi_w
  { 0b01110110100101010, 0, 5, 10, 0 }, // xvmaxi_wu
  { 0b01110110100100100, 0, 5, 10, 0 }, // xvmini_b
  { 0b01110110100101100, 0, 5, 10, 0 }, // xvmini_bu
  { 0b01110110100100111, 0, 5, 10, 0 }, // xvmini_d
  { 0b01110110100101111, 0, 5, 10, 0 }, // xvmini_du
  { 0b01110110100100101, 0, 5, 10, 0 }, // xvmini_h
  { 0b01110110100101101, 0, 5, 10, 0 }, // xvmini_hu
  { 0b01110110100100110, 0, 5, 10, 0 }, // xvmini_w
  { 0b01110110100101110, 0, 5, 10, 0 }, // xvmini_wu
  { 0b01110111110111, 0, 5, 10, 0 }, // xvnori_b
  { 0b01110111110101, 0, 5, 10, 0 }, // xvori_b
  { 0b01110111111010, 0, 5, 10, 0 }, // xvpermi_d
  { 0b01110111111011, 0, 5, 10, 0 }, // xvpermi_q
  { 0b01110111111001, 0, 5, 10, 0 }, // xvpermi_w
  { 0b01110111000000111110, 0, 5, 10, 0 }, // xvpickve_d
  { 0b0111011100000011110, 0, 5, 10, 0 }, // xvpickve_w
  { 0b011101101111011110, 0, 5, 10, 0 }, // xvrepl128vei_b
  { 0b011101101111011111110, 0, 5, 10, 0 }, // xvrepl128vei_d
  { 0b0111011011110111110, 0, 5, 10, 0 }, // xvrepl128vei_h
  { 0b01110110111101111110, 0, 5, 10, 0 }, // xvrepl128vei_w
  { 0b0111011010100000001, 0, 5, 10, 0 }, // xvrotri_b
  { 0b0111011010100001, 0, 5, 10, 0 }, // xvrotri_d
  { 0b011101101010000001, 0, 5, 10, 0 }, // xvrotri_h
  { 0b01110110101000001, 0, 5, 10, 0 }, // xvrotri_w
  { 0b0111011100100100001, 0, 5, 10, 0 }, // xvsat_b
  { 0b0111011100101000001, 0, 5, 10, 0 }, // xvsat_bu
  { 0b0111011100100101, 0, 5, 10, 0 }, // xvsat_d
  { 0b0111011100101001, 0, 5, 10, 0 }, // xvsat_du
  { 0b011101110010010001, 0, 5, 10, 0 }, // xvsat_h
  { 0b011101110010100001, 0, 5, 10, 0 }, // xvsat_hu
  { 0b01110111001001001, 0, 5, 10, 0 }, // xvsat_w
  { 0b01110111001010001, 0, 5, 10, 0 }, // xvsat_wu
  { 0b01110110100000000, 0, 5, 10, 0 }, // xvseqi_b
  { 0b01110110100000011, 0, 5, 10, 0 }, // xvseqi_d
  { 0b01110110100000001, 0, 5, 10, 0 }, // xvseqi_h
  { 0b01110110100000010, 0, 5, 10, 0 }, // xvseqi_w
  { 0b01110111100100, 0, 5, 10, 0 }, // xvshuf4i_b
  { 0b01110111100111, 0, 5, 10, 0 }, // xvshuf4i_d
  { 0b01110111100101, 0, 5, 10, 0 }, // xvshuf4i_h
  { 0b01110111100110, 0, 5, 10, 0 }, // xvshuf4i_w
  { 0b01110110100000100, 0, 5, 10, 0 }, // xvslei_b
  { 0b01110110100001000, 0, 5, 10, 0 }, // xvslei_bu
  { 0b01110110100000111, 0, 5, 10, 0 }, // xvslei_d
  { 0b01110110100001011, 0, 5, 10, 0 }, // xvslei_du
  { 0b01110110100000101, 0, 5, 10, 0 }, // xvslei_h
  { 0b01110110100001001, 0, 5, 10, 0 }, // xvslei_hu
  { 0b01110110100000110, 0, 5, 10, 0 }, // xvslei_w
  { 0b01110110100001010, 0, 5, 10, 0 }, // xvslei_wu
  { 0b0111011100101100001, 0, 5, 10, 0 }, // xvslli_b
  { 0b0111011100101101, 0, 5, 10, 0 }, // xvslli_d
  { 0b011101110010110001, 0, 5, 10, 0 }, // xvslli_h
  { 0b01110111001011001, 0, 5, 10, 0 }, // xvslli_w
  { 0b01110111000010001, 0, 5, 10, 0 }, // xvsllwil_d_w
  { 0b01110111000011001, 0, 5, 10, 0 }, // xvsllwil_du_wu
  { 0b0111011100001000001, 0, 5, 10, 0 }, // xvsllwil_h_b
  { 0b0111011100001100001, 0, 5, 10, 0 }, // xvsllwil_hu_bu
  { 0b011101110000100001, 0, 5, 10, 0 }, // xvsllwil_w_h
  { 0b011101110000110001, 0, 5, 10, 0 }, // xvsllwil_wu_hu
  { 0b01110110100001100, 0, 5, 10, 0 }, // xvslti_b
  { 0b01110110100010000, 0, 5, 10, 0 }, // xvslti_bu
  { 0b01110110100001111, 0, 5, 10, 0 }, // xvslti_d
  { 0b01110110100010011, 0, 5, 10, 0 }, // xvslti_du
  { 0b01110110100001101, 0, 5, 10, 0 }, // xvslti_h
  { 0b01110110100010001, 0, 5, 10, 0 }, // xvslti_hu
  { 0b01110110100001110, 0, 5, 10, 0 }, // xvslti_w
  { 0b01110110100010010, 0, 5, 10, 0 }, // xvslti_wu
  { 0b0111011100110100001, 0, 5, 10, 0 }, // xvsrai_b
  { 0b0111011100110101, 0, 5, 10, 0 }, // xvsrai_d
  { 0b011101110011010001, 0, 5, 10, 0 }, // xvsrai_h
  { 0b01110111001101001, 0, 5, 10, 0 }, // xvsrai_w
  { 0b011101110101100001, 0, 5, 10, 0 }, // xvsrani_b_h
  { 0b011101110101101, 0, 5, 10, 0 }, // xvsrani_d_q
  { 0b01110111010110001, 0, 5, 10, 0 }, // xvsrani_h_w
  { 0b0111011101011001, 0, 5, 10, 0 }, // xvsrani_w_d
  { 0b0111011010101000001, 0, 5, 10, 0 }, // xvsrari_b
  { 0b0111011010101001, 0, 5, 10, 0 }, // xvsrari_d
  { 0b011101101010100001, 0, 5, 10, 0 }, // xvsrari_h
  { 0b01110110101010001, 0, 5, 10, 0 }, // xvsrari_w
  { 0b011101110101110001, 0, 5, 10, 0 }, // xvsrarni_b_h
  { 0b011101110101111, 0, 5, 10, 0 }, // xvsrarni_d_q
  { 0b01110111010111001, 0, 5, 10, 0 }, // xvsrarni_h_w
  { 0b0111011101011101, 0, 5, 10, 0 }, // xvsrarni_w_d
  { 0b0111011100110000001, 0, 5, 10, 0 }, // xvsrli_b
  { 0b0111011100110001, 0, 5, 10, 0 }, // xvsrli_d
  { 0b011101110011000001, 0, 5, 10, 0 }, // xvsrli_h
  { 0b01110111001100001, 0, 5, 10, 0 }, // xvsrli_w
  { 0b011101110100000001, 0, 5, 10, 0 }, // xvsrlni_b_h
  { 0b011101110100001, 0, 5, 10, 0 }, // xvsrlni_d_q
  { 0b01110111010000001, 0, 5, 10, 0 }, // xvsrlni_h_w
  { 0b0111011101000001, 0, 5, 10, 0 }, // xvsrlni_w_d
  { 0b0111011010100100001, 0, 5, 10, 0 }, // xvsrlri_b
  { 0b0111011010100101, 0, 5, 10, 0 }, // xvsrlri_d
  { 0b011101101010010001, 0, 5, 10, 0 }, // xvsrlri_h
  { 0b01110110101001001, 0, 5, 10, 0 }, // xvsrlri_w
  { 0b011101110100010001, 0, 5, 10, 0 }, // xvsrlrni_b_h
  { 0b011101110100011, 0, 5, 10, 0 }, // xvsrlrni_d_q
  { 0b01110111010001001, 0, 5, 10, 0 }, // xvsrlrni_h_w
  { 0b0111011101000101, 0, 5, 10, 0 }, // xvsrlrni_w_d
  { 0b011101110110000001, 0, 5, 10, 0 }, // xvssrani_b_h
  { 0b011101110110010001, 0, 5, 10, 0 }, // xvssrani_bu_h
  { 0b011101110110001, 0, 5, 10, 0 }, // xvssrani_d_q
  { 0b011101110110011, 0, 5, 10, 0 }, // xvssrani_du_q
  { 0b01110111011000001, 0, 5, 10, 0 }, // xvssrani_h_w
  { 0b01110111011001001, 0, 5, 10, 0 }, // xvssrani_hu_w
  { 0b0111011101100001, 0, 5, 10, 0 }, // xvssrani_w_d
  { 0b0111011101100101, 0, 5, 10, 0 }, // xvssrani_wu_d
  { 0b011101110110100001, 0, 5, 10, 0 }, // xvssrarni_b_h
  { 0b011101110110110001, 0, 5, 10, 0 }, // xvssrarni_bu_h
  { 0b011101110110101, 0, 5, 10, 0 }, // xvssrarni_d_q
  { 0b011101110110111, 0, 5, 10, 0 }, // xvssrarni_du_q
  { 0b01110111011010001, 0, 5, 10, 0 }, // xvssrarni_h_w
  { 0b01110111011011001, 0, 5, 10, 0 }, // xvssrarni_hu_w
  { 0b0111011101101001, 0, 5, 10, 0 }, // xvssrarni_w_d
  { 0b0111011101101101, 0, 5, 10, 0 }, // xvssrarni_wu_d
  { 0b011101110100100001, 0, 5, 10, 0 }, // xvssrlni_b_h
  { 0b011101110100110001, 0, 5, 10, 0 }, // xvssrlni_bu_h
  { 0b011101110100101, 0, 5, 10, 0 }, // xvssrlni_d_q
  { 0b011101110100111, 0, 5, 10, 0 }, // xvssrlni_du_q
  { 0b01110111010010001, 0, 5, 10, 0 }, // xvssrlni_h_w
  { 0b01110111010011001, 0, 5, 10, 0 }, // xvssrlni_hu_w
  { 0b0111011101001001, 0, 5, 10, 0 }, // xvssrlni_w_d
  { 0b0111011101001101, 0, 5, 10, 0 }, // xvssrlni_wu_d
  { 0b011101110101000001, 0, 5, 10, 0 }, // xvssrlrni_b_h
  { 0b011101110101010001, 0, 5, 10, 0 }, // xvssrlrni_bu_h
  { 0b011101110101001, 0, 5, 10, 0 }, // xvssrlrni_d_q
  { 0b011101110101011, 0, 5, 10, 0 }, // xvssrlrni_du_q
  { 0b01110111010100001, 0, 5, 10, 0 }, // xvssrlrni_h_w
  { 0b01110111010101001, 0, 5, 10, 0 }, // xvssrlrni_hu_w
  { 0b0111011101010001, 0, 5, 10, 0 }, // xvssrlrni_w_d
  { 0b0111011101010101, 0, 5, 10, 0 }, // xvssrlrni_wu_d
  { 0b01110110100011000, 0, 5, 10, 0 }, // xvsubi_bu
  { 0b01110110100011011, 0, 5, 10, 0 }, // xvsubi_du
  { 0b01110110100011001, 0, 5, 10, 0 }, // xvsubi_hu
  { 0b01110110100011010, 0, 5, 10, 0 }, // xvsubi_wu
  { 0b01110111110110, 0, 5, 10, 0 }  // xvxori_b
};

const LasxXXR lasxXXR[4] = {
  { 0b01110101001000100, 0, 5, kWX, 10, 0 }, // xvreplve_b
  { 0b01110101001000111, 0, 5, kWX, 10, 0 }, // xvreplve_d
  { 0b01110101001000101, 0, 5, kWX, 10, 0 }, // xvreplve_h
  { 0b01110101001000110, 0, 5, kWX, 10, 0 }  // xvreplve_w
};

const LasxXXX lasxXXX[412] = {
  { 0b01110100011000000, 0, 5, 10, 0 }, // xvabsd_b
  { 0b01110100011000100, 0, 5, 10, 0 }, // xvabsd_bu
  { 0b01110100011000011, 0, 5, 10, 0 }, // xvabsd_d
  { 0b01110100011000111, 0, 5, 10, 0 }, // xvabsd_du
  { 0b01110100011000001, 0, 5, 10, 0 }, // xvabsd_h
  { 0b01110100011000101, 0, 5, 10, 0 }, // xvabsd_hu
  { 0b01110100011000010, 0, 5, 10, 0 }, // xvabsd_w
  { 0b01110100011000110, 0, 5, 10, 0 }, // xvabsd_wu
  { 0b01110100000010100, 0, 5, 10, 0 }, // xvadd_b
  { 0b01110100000010111, 0, 5, 10, 0 }, // xvadd_d
  { 0b01110100000010101, 0, 5, 10, 0 }, // xvadd_h
  { 0b01110101001011010, 0, 5, 10, 0 }, // xvadd_q
  { 0b01110100000010110, 0, 5, 10, 0 }, // xvadd_w
  { 0b01110100010111000, 0, 5, 10, 0 }, // xvadda_b
  { 0b01110100010111011, 0, 5, 10, 0 }, // xvadda_d
  { 0b01110100010111001, 0, 5, 10, 0 }, // xvadda_h
  { 0b01110100010111010, 0, 5, 10, 0 }, // xvadda_w
  { 0b01110100000111110, 0, 5, 10, 0 }, // xvaddwev_d_w
  { 0b01110100001011110, 0, 5, 10, 0 }, // xvaddwev_d_wu
  { 0b01110100001111110, 0, 5, 10, 0 }, // xvaddwev_d_wu_w
  { 0b01110100000111100, 0, 5, 10, 0 }, // xvaddwev_h_b
  { 0b01110100001011100, 0, 5, 10, 0 }, // xvaddwev_h_bu
  { 0b01110100001111100, 0, 5, 10, 0 }, // xvaddwev_h_bu_b
  { 0b01110100000111111, 0, 5, 10, 0 }, // xvaddwev_q_d
  { 0b01110100001011111, 0, 5, 10, 0 }, // xvaddwev_q_du
  { 0b01110100001111111, 0, 5, 10, 0 }, // xvaddwev_q_du_d
  { 0b01110100000111101, 0, 5, 10, 0 }, // xvaddwev_w_h
  { 0b01110100001011101, 0, 5, 10, 0 }, // xvaddwev_w_hu
  { 0b01110100001111101, 0, 5, 10, 0 }, // xvaddwev_w_hu_h
  { 0b01110100001000110, 0, 5, 10, 0 }, // xvaddwod_d_w
  { 0b01110100001100110, 0, 5, 10, 0 }, // xvaddwod_d_wu
  { 0b01110100010000010, 0, 5, 10, 0 }, // xvaddwod_d_wu_w
  { 0b01110100001000100, 0, 5, 10, 0 }, // xvaddwod_h_b
  { 0b01110100001100100, 0, 5, 10, 0 }, // xvaddwod_h_bu
  { 0b01110100010000000, 0, 5, 10, 0 }, // xvaddwod_h_bu_b
  { 0b01110100001000111, 0, 5, 10, 0 }, // xvaddwod_q_d
  { 0b01110100001100111, 0, 5, 10, 0 }, // xvaddwod_q_du
  { 0b01110100010000011, 0, 5, 10, 0 }, // xvaddwod_q_du_d
  { 0b01110100001000101, 0, 5, 10, 0 }, // xvaddwod_w_h
  { 0b01110100001100101, 0, 5, 10, 0 }, // xvaddwod_w_hu
  { 0b01110100010000001, 0, 5, 10, 0 }, // xvaddwod_w_hu_h
  { 0b01110101001001100, 0, 5, 10, 0 }, // xvand_v
  { 0b01110101001010000, 0, 5, 10, 0 }, // xvandn_v
  { 0b01110100011001000, 0, 5, 10, 0 }, // xvavg_b
  { 0b01110100011001100, 0, 5, 10, 0 }, // xvavg_bu
  { 0b01110100011001011, 0, 5, 10, 0 }, // xvavg_d
  { 0b01110100011001111, 0, 5, 10, 0 }, // xvavg_du
  { 0b01110100011001001, 0, 5, 10, 0 }, // xvavg_h
  { 0b01110100011001101, 0, 5, 10, 0 }, // xvavg_hu
  { 0b01110100011001010, 0, 5, 10, 0 }, // xvavg_w
  { 0b01110100011001110, 0, 5, 10, 0 }, // xvavg_wu
  { 0b01110100011010000, 0, 5, 10, 0 }, // xvavgr_b
  { 0b01110100011010100, 0, 5, 10, 0 }, // xvavgr_bu
  { 0b01110100011010011, 0, 5, 10, 0 }, // xvavgr_d
  { 0b01110100011010111, 0, 5, 10, 0 }, // xvavgr_du
  { 0b01110100011010001, 0, 5, 10, 0 }, // xvavgr_h
  { 0b01110100011010101, 0, 5, 10, 0 }, // xvavgr_hu
  { 0b01110100011010010, 0, 5, 10, 0 }, // xvavgr_w
  { 0b01110100011010110, 0, 5, 10, 0 }, // xvavgr_wu
  { 0b01110101000011000, 0, 5, 10, 0 }, // xvbitclr_b
  { 0b01110101000011011, 0, 5, 10, 0 }, // xvbitclr_d
  { 0b01110101000011001, 0, 5, 10, 0 }, // xvbitclr_h
  { 0b01110101000011010, 0, 5, 10, 0 }, // xvbitclr_w
  { 0b01110101000100000, 0, 5, 10, 0 }, // xvbitrev_b
  { 0b01110101000100011, 0, 5, 10, 0 }, // xvbitrev_d
  { 0b01110101000100001, 0, 5, 10, 0 }, // xvbitrev_h
  { 0b01110101000100010, 0, 5, 10, 0 }, // xvbitrev_w
  { 0b01110101000011100, 0, 5, 10, 0 }, // xvbitset_b
  { 0b01110101000011111, 0, 5, 10, 0 }, // xvbitset_d
  { 0b01110101000011101, 0, 5, 10, 0 }, // xvbitset_h
  { 0b01110101000011110, 0, 5, 10, 0 }, // xvbitset_w
  { 0b01110100111000000, 0, 5, 10, 0 }, // xvdiv_b
  { 0b01110100111001000, 0, 5, 10, 0 }, // xvdiv_bu
  { 0b01110100111000011, 0, 5, 10, 0 }, // xvdiv_d
  { 0b01110100111001011, 0, 5, 10, 0 }, // xvdiv_du
  { 0b01110100111000001, 0, 5, 10, 0 }, // xvdiv_h
  { 0b01110100111001001, 0, 5, 10, 0 }, // xvdiv_hu
  { 0b01110100111000010, 0, 5, 10, 0 }, // xvdiv_w
  { 0b01110100111001010, 0, 5, 10, 0 }, // xvdiv_wu
  { 0b01110101001100010, 0, 5, 10, 0 }, // xvfadd_d
  { 0b01110101001100001, 0, 5, 10, 0 }, // xvfadd_s
  { 0b01110101010001100, 0, 5, 10, 0 }, // xvfcvt_h_s
  { 0b01110101010001101, 0, 5, 10, 0 }, // xvfcvt_s_d
  { 0b01110101001110110, 0, 5, 10, 0 }, // xvfdiv_d
  { 0b01110101001110101, 0, 5, 10, 0 }, // xvfdiv_s
  { 0b01110101010010000, 0, 5, 10, 0 }, // xvffint_s_l
  { 0b01110101001111010, 0, 5, 10, 0 }, // xvfmax_d
  { 0b01110101001111001, 0, 5, 10, 0 }, // xvfmax_s
  { 0b01110101010000010, 0, 5, 10, 0 }, // xvfmaxa_d
  { 0b01110101010000001, 0, 5, 10, 0 }, // xvfmaxa_s
  { 0b01110101001111110, 0, 5, 10, 0 }, // xvfmin_d
  { 0b01110101001111101, 0, 5, 10, 0 }, // xvfmin_s
  { 0b01110101010000110, 0, 5, 10, 0 }, // xvfmina_d
  { 0b01110101010000101, 0, 5, 10, 0 }, // xvfmina_s
  { 0b01110101001110010, 0, 5, 10, 0 }, // xvfmul_d
  { 0b01110101001110001, 0, 5, 10, 0 }, // xvfmul_s
  { 0b01110101001010110, 0, 5, 10, 0 }, // xvfrstp_b
  { 0b01110101001010111, 0, 5, 10, 0 }, // xvfrstp_h
  { 0b01110101010001010, 0, 5, 10, 0 }, // xvfscaleb_d
  { 0b01110101010001001, 0, 5, 10, 0 }, // xvfscaleb_s
  { 0b01110101001100110, 0, 5, 10, 0 }, // xvfsub_d
  { 0b01110101001100101, 0, 5, 10, 0 }, // xvfsub_s
  { 0b01110101010010011, 0, 5, 10, 0 }, // xvftint_w_d
  { 0b01110101010010100, 0, 5, 10, 0 }, // xvftintrm_w_d
  { 0b01110101010010111, 0, 5, 10, 0 }, // xvftintrne_w_d
  { 0b01110101010010101, 0, 5, 10, 0 }, // xvftintrp_w_d
  { 0b01110101010010110, 0, 5, 10, 0 }, // xvftintrz_w_d
  { 0b01110100010101010, 0, 5, 10, 0 }, // xvhaddw_d_w
  { 0b01110100010110010, 0, 5, 10, 0 }, // xvhaddw_du_wu
  { 0b01110100010101000, 0, 5, 10, 0 }, // xvhaddw_h_b
  { 0b01110100010110000, 0, 5, 10, 0 }, // xvhaddw_hu_bu
  { 0b01110100010101011, 0, 5, 10, 0 }, // xvhaddw_q_d
  { 0b01110100010110011, 0, 5, 10, 0 }, // xvhaddw_qu_du
  { 0b01110100010101001, 0, 5, 10, 0 }, // xvhaddw_w_h
  { 0b01110100010110001, 0, 5, 10, 0 }, // xvhaddw_wu_hu
  { 0b01110100010101110, 0, 5, 10, 0 }, // xvhsubw_d_w
  { 0b01110100010110110, 0, 5, 10, 0 }, // xvhsubw_du_wu
  { 0b01110100010101100, 0, 5, 10, 0 }, // xvhsubw_h_b
  { 0b01110100010110100, 0, 5, 10, 0 }, // xvhsubw_hu_bu
  { 0b01110100010101111, 0, 5, 10, 0 }, // xvhsubw_q_d
  { 0b01110100010110111, 0, 5, 10, 0 }, // xvhsubw_qu_du
  { 0b01110100010101101, 0, 5, 10, 0 }, // xvhsubw_w_h
  { 0b01110100010110101, 0, 5, 10, 0 }, // xvhsubw_wu_hu
  { 0b01110101000111000, 0, 5, 10, 0 }, // xvilvh_b
  { 0b01110101000111011, 0, 5, 10, 0 }, // xvilvh_d
  { 0b01110101000111001, 0, 5, 10, 0 }, // xvilvh_h
  { 0b01110101000111010, 0, 5, 10, 0 }, // xvilvh_w
  { 0b01110101000110100, 0, 5, 10, 0 }, // xvilvl_b
  { 0b01110101000110111, 0, 5, 10, 0 }, // xvilvl_d
  { 0b01110101000110101, 0, 5, 10, 0 }, // xvilvl_h
  { 0b01110101000110110, 0, 5, 10, 0 }, // xvilvl_w
  { 0b01110100101010000, 0, 5, 10, 0 }, // xvmadd_b
  { 0b01110100101010011, 0, 5, 10, 0 }, // xvmadd_d
  { 0b01110100101010001, 0, 5, 10, 0 }, // xvmadd_h
  { 0b01110100101010010, 0, 5, 10, 0 }, // xvmadd_w
  { 0b01110100101011010, 0, 5, 10, 0 }, // xvmaddwev_d_w
  { 0b01110100101101010, 0, 5, 10, 0 }, // xvmaddwev_d_wu
  { 0b01110100101111010, 0, 5, 10, 0 }, // xvmaddwev_d_wu_w
  { 0b01110100101011000, 0, 5, 10, 0 }, // xvmaddwev_h_b
  { 0b01110100101101000, 0, 5, 10, 0 }, // xvmaddwev_h_bu
  { 0b01110100101111000, 0, 5, 10, 0 }, // xvmaddwev_h_bu_b
  { 0b01110100101011011, 0, 5, 10, 0 }, // xvmaddwev_q_d
  { 0b01110100101101011, 0, 5, 10, 0 }, // xvmaddwev_q_du
  { 0b01110100101111011, 0, 5, 10, 0 }, // xvmaddwev_q_du_d
  { 0b01110100101011001, 0, 5, 10, 0 }, // xvmaddwev_w_h
  { 0b01110100101101001, 0, 5, 10, 0 }, // xvmaddwev_w_hu
  { 0b01110100101111001, 0, 5, 10, 0 }, // xvmaddwev_w_hu_h
  { 0b01110100101011110, 0, 5, 10, 0 }, // xvmaddwod_d_w
  { 0b01110100101101110, 0, 5, 10, 0 }, // xvmaddwod_d_wu
  { 0b01110100101111110, 0, 5, 10, 0 }, // xvmaddwod_d_wu_w
  { 0b01110100101011100, 0, 5, 10, 0 }, // xvmaddwod_h_b
  { 0b01110100101101100, 0, 5, 10, 0 }, // xvmaddwod_h_bu
  { 0b01110100101111100, 0, 5, 10, 0 }, // xvmaddwod_h_bu_b
  { 0b01110100101011111, 0, 5, 10, 0 }, // xvmaddwod_q_d
  { 0b01110100101101111, 0, 5, 10, 0 }, // xvmaddwod_q_du
  { 0b01110100101111111, 0, 5, 10, 0 }, // xvmaddwod_q_du_d
  { 0b01110100101011101, 0, 5, 10, 0 }, // xvmaddwod_w_h
  { 0b01110100101101101, 0, 5, 10, 0 }, // xvmaddwod_w_hu
  { 0b01110100101111101, 0, 5, 10, 0 }, // xvmaddwod_w_hu_h
  { 0b01110100011100000, 0, 5, 10, 0 }, // xvmax_b
  { 0b01110100011101000, 0, 5, 10, 0 }, // xvmax_bu
  { 0b01110100011100011, 0, 5, 10, 0 }, // xvmax_d
  { 0b01110100011101011, 0, 5, 10, 0 }, // xvmax_du
  { 0b01110100011100001, 0, 5, 10, 0 }, // xvmax_h
  { 0b01110100011101001, 0, 5, 10, 0 }, // xvmax_hu
  { 0b01110100011100010, 0, 5, 10, 0 }, // xvmax_w
  { 0b01110100011101010, 0, 5, 10, 0 }, // xvmax_wu
  { 0b01110100011100100, 0, 5, 10, 0 }, // xvmin_b
  { 0b01110100011101100, 0, 5, 10, 0 }, // xvmin_bu
  { 0b01110100011100111, 0, 5, 10, 0 }, // xvmin_d
  { 0b01110100011101111, 0, 5, 10, 0 }, // xvmin_du
  { 0b01110100011100101, 0, 5, 10, 0 }, // xvmin_h
  { 0b01110100011101101, 0, 5, 10, 0 }, // xvmin_hu
  { 0b01110100011100110, 0, 5, 10, 0 }, // xvmin_w
  { 0b01110100011101110, 0, 5, 10, 0 }, // xvmin_wu
  { 0b01110100111000100, 0, 5, 10, 0 }, // xvmod_b
  { 0b01110100111001100, 0, 5, 10, 0 }, // xvmod_bu
  { 0b01110100111000111, 0, 5, 10, 0 }, // xvmod_d
  { 0b01110100111001111, 0, 5, 10, 0 }, // xvmod_du
  { 0b01110100111000101, 0, 5, 10, 0 }, // xvmod_h
  { 0b01110100111001101, 0, 5, 10, 0 }, // xvmod_hu
  { 0b01110100111000110, 0, 5, 10, 0 }, // xvmod_w
  { 0b01110100111001110, 0, 5, 10, 0 }, // xvmod_wu
  { 0b01110100101010100, 0, 5, 10, 0 }, // xvmsub_b
  { 0b01110100101010111, 0, 5, 10, 0 }, // xvmsub_d
  { 0b01110100101010101, 0, 5, 10, 0 }, // xvmsub_h
  { 0b01110100101010110, 0, 5, 10, 0 }, // xvmsub_w
  { 0b01110100100001100, 0, 5, 10, 0 }, // xvmuh_b
  { 0b01110100100010000, 0, 5, 10, 0 }, // xvmuh_bu
  { 0b01110100100001111, 0, 5, 10, 0 }, // xvmuh_d
  { 0b01110100100010011, 0, 5, 10, 0 }, // xvmuh_du
  { 0b01110100100001101, 0, 5, 10, 0 }, // xvmuh_h
  { 0b01110100100010001, 0, 5, 10, 0 }, // xvmuh_hu
  { 0b01110100100001110, 0, 5, 10, 0 }, // xvmuh_w
  { 0b01110100100010010, 0, 5, 10, 0 }, // xvmuh_wu
  { 0b01110100100001000, 0, 5, 10, 0 }, // xvmul_b
  { 0b01110100100001011, 0, 5, 10, 0 }, // xvmul_d
  { 0b01110100100001001, 0, 5, 10, 0 }, // xvmul_h
  { 0b01110100100001010, 0, 5, 10, 0 }, // xvmul_w
  { 0b01110100100100010, 0, 5, 10, 0 }, // xvmulwev_d_w
  { 0b01110100100110010, 0, 5, 10, 0 }, // xvmulwev_d_wu
  { 0b01110100101000010, 0, 5, 10, 0 }, // xvmulwev_d_wu_w
  { 0b01110100100100000, 0, 5, 10, 0 }, // xvmulwev_h_b
  { 0b01110100100110000, 0, 5, 10, 0 }, // xvmulwev_h_bu
  { 0b01110100101000000, 0, 5, 10, 0 }, // xvmulwev_h_bu_b
  { 0b01110100100100011, 0, 5, 10, 0 }, // xvmulwev_q_d
  { 0b01110100100110011, 0, 5, 10, 0 }, // xvmulwev_q_du
  { 0b01110100101000011, 0, 5, 10, 0 }, // xvmulwev_q_du_d
  { 0b01110100100100001, 0, 5, 10, 0 }, // xvmulwev_w_h
  { 0b01110100100110001, 0, 5, 10, 0 }, // xvmulwev_w_hu
  { 0b01110100101000001, 0, 5, 10, 0 }, // xvmulwev_w_hu_h
  { 0b01110100100100110, 0, 5, 10, 0 }, // xvmulwod_d_w
  { 0b01110100100110110, 0, 5, 10, 0 }, // xvmulwod_d_wu
  { 0b01110100101000110, 0, 5, 10, 0 }, // xvmulwod_d_wu_w
  { 0b01110100100100100, 0, 5, 10, 0 }, // xvmulwod_h_b
  { 0b01110100100110100, 0, 5, 10, 0 }, // xvmulwod_h_bu
  { 0b01110100101000100, 0, 5, 10, 0 }, // xvmulwod_h_bu_b
  { 0b01110100100100111, 0, 5, 10, 0 }, // xvmulwod_q_d
  { 0b01110100100110111, 0, 5, 10, 0 }, // xvmulwod_q_du
  { 0b01110100101000111, 0, 5, 10, 0 }, // xvmulwod_q_du_d
  { 0b01110100100100101, 0, 5, 10, 0 }, // xvmulwod_w_h
  { 0b01110100100110101, 0, 5, 10, 0 }, // xvmulwod_w_hu
  { 0b01110100101000101, 0, 5, 10, 0 }, // xvmulwod_w_hu_h
  { 0b01110101001001111, 0, 5, 10, 0 }, // xvnor_v
  { 0b01110101001001101, 0, 5, 10, 0 }, // xvor_v
  { 0b01110101001010001, 0, 5, 10, 0 }, // xvorn_v
  { 0b01110101000101100, 0, 5, 10, 0 }, // xvpackev_b
  { 0b01110101000101111, 0, 5, 10, 0 }, // xvpackev_d
  { 0b01110101000101101, 0, 5, 10, 0 }, // xvpackev_h
  { 0b01110101000101110, 0, 5, 10, 0 }, // xvpackev_w
  { 0b01110101000110000, 0, 5, 10, 0 }, // xvpackod_b
  { 0b01110101000110011, 0, 5, 10, 0 }, // xvpackod_d
  { 0b01110101000110001, 0, 5, 10, 0 }, // xvpackod_h
  { 0b01110101000110010, 0, 5, 10, 0 }, // xvpackod_w
  { 0b01110101011111010, 0, 5, 10, 0 }, // xvperm_w
  { 0b01110101000111100, 0, 5, 10, 0 }, // xvpickev_b
  { 0b01110101000111111, 0, 5, 10, 0 }, // xvpickev_d
  { 0b01110101000111101, 0, 5, 10, 0 }, // xvpickev_h
  { 0b01110101000111110, 0, 5, 10, 0 }, // xvpickev_w
  { 0b01110101001000000, 0, 5, 10, 0 }, // xvpickod_b
  { 0b01110101001000011, 0, 5, 10, 0 }, // xvpickod_d
  { 0b01110101001000001, 0, 5, 10, 0 }, // xvpickod_h
  { 0b01110101001000010, 0, 5, 10, 0 }, // xvpickod_w
  { 0b01110100111011100, 0, 5, 10, 0 }, // xvrotr_b
  { 0b01110100111011111, 0, 5, 10, 0 }, // xvrotr_d
  { 0b01110100111011101, 0, 5, 10, 0 }, // xvrotr_h
  { 0b01110100111011110, 0, 5, 10, 0 }, // xvrotr_w
  { 0b01110100010001100, 0, 5, 10, 0 }, // xvsadd_b
  { 0b01110100010010100, 0, 5, 10, 0 }, // xvsadd_bu
  { 0b01110100010001111, 0, 5, 10, 0 }, // xvsadd_d
  { 0b01110100010010111, 0, 5, 10, 0 }, // xvsadd_du
  { 0b01110100010001101, 0, 5, 10, 0 }, // xvsadd_h
  { 0b01110100010010101, 0, 5, 10, 0 }, // xvsadd_hu
  { 0b01110100010001110, 0, 5, 10, 0 }, // xvsadd_w
  { 0b01110100010010110, 0, 5, 10, 0 }, // xvsadd_wu
  { 0b01110100000000000, 0, 5, 10, 0 }, // xvseq_b
  { 0b01110100000000011, 0, 5, 10, 0 }, // xvseq_d
  { 0b01110100000000001, 0, 5, 10, 0 }, // xvseq_h
  { 0b01110100000000010, 0, 5, 10, 0 }, // xvseq_w
  { 0b01110101011110111, 0, 5, 10, 0 }, // xvshuf_d
  { 0b01110101011110101, 0, 5, 10, 0 }, // xvshuf_h
  { 0b01110101011110110, 0, 5, 10, 0 }, // xvshuf_w
  { 0b01110101001011100, 0, 5, 10, 0 }, // xvsigncov_b
  { 0b01110101001011111, 0, 5, 10, 0 }, // xvsigncov_d
  { 0b01110101001011101, 0, 5, 10, 0 }, // xvsigncov_h
  { 0b01110101001011110, 0, 5, 10, 0 }, // xvsigncov_w
  { 0b01110100000000100, 0, 5, 10, 0 }, // xvsle_b
  { 0b01110100000001000, 0, 5, 10, 0 }, // xvsle_bu
  { 0b01110100000000111, 0, 5, 10, 0 }, // xvsle_d
  { 0b01110100000001011, 0, 5, 10, 0 }, // xvsle_du
  { 0b01110100000000101, 0, 5, 10, 0 }, // xvsle_h
  { 0b01110100000001001, 0, 5, 10, 0 }, // xvsle_hu
  { 0b01110100000000110, 0, 5, 10, 0 }, // xvsle_w
  { 0b01110100000001010, 0, 5, 10, 0 }, // xvsle_wu
  { 0b01110100111010000, 0, 5, 10, 0 }, // xvsll_b
  { 0b01110100111010011, 0, 5, 10, 0 }, // xvsll_d
  { 0b01110100111010001, 0, 5, 10, 0 }, // xvsll_h
  { 0b01110100111010010, 0, 5, 10, 0 }, // xvsll_w
  { 0b01110100000001100, 0, 5, 10, 0 }, // xvslt_b
  { 0b01110100000010000, 0, 5, 10, 0 }, // xvslt_bu
  { 0b01110100000001111, 0, 5, 10, 0 }, // xvslt_d
  { 0b01110100000010011, 0, 5, 10, 0 }, // xvslt_du
  { 0b01110100000001101, 0, 5, 10, 0 }, // xvslt_h
  { 0b01110100000010001, 0, 5, 10, 0 }, // xvslt_hu
  { 0b01110100000001110, 0, 5, 10, 0 }, // xvslt_w
  { 0b01110100000010010, 0, 5, 10, 0 }, // xvslt_wu
  { 0b01110100111011000, 0, 5, 10, 0 }, // xvsra_b
  { 0b01110100111011011, 0, 5, 10, 0 }, // xvsra_d
  { 0b01110100111011001, 0, 5, 10, 0 }, // xvsra_h
  { 0b01110100111011010, 0, 5, 10, 0 }, // xvsra_w
  { 0b01110100111101101, 0, 5, 10, 0 }, // xvsran_b_h
  { 0b01110100111101110, 0, 5, 10, 0 }, // xvsran_h_w
  { 0b01110100111101111, 0, 5, 10, 0 }, // xvsran_w_d
  { 0b01110100111100100, 0, 5, 10, 0 }, // xvsrar_b
  { 0b01110100111100111, 0, 5, 10, 0 }, // xvsrar_d
  { 0b01110100111100101, 0, 5, 10, 0 }, // xvsrar_h
  { 0b01110100111100110, 0, 5, 10, 0 }, // xvsrar_w
  { 0b01110100111110101, 0, 5, 10, 0 }, // xvsrarn_b_h
  { 0b01110100111110110, 0, 5, 10, 0 }, // xvsrarn_h_w
  { 0b01110100111110111, 0, 5, 10, 0 }, // xvsrarn_w_d
  { 0b01110100111010100, 0, 5, 10, 0 }, // xvsrl_b
  { 0b01110100111010111, 0, 5, 10, 0 }, // xvsrl_d
  { 0b01110100111010101, 0, 5, 10, 0 }, // xvsrl_h
  { 0b01110100111010110, 0, 5, 10, 0 }, // xvsrl_w
  { 0b01110100111101001, 0, 5, 10, 0 }, // xvsrln_b_h
  { 0b01110100111101010, 0, 5, 10, 0 }, // xvsrln_h_w
  { 0b01110100111101011, 0, 5, 10, 0 }, // xvsrln_w_d
  { 0b01110100111100000, 0, 5, 10, 0 }, // xvsrlr_b
  { 0b01110100111100011, 0, 5, 10, 0 }, // xvsrlr_d
  { 0b01110100111100001, 0, 5, 10, 0 }, // xvsrlr_h
  { 0b01110100111100010, 0, 5, 10, 0 }, // xvsrlr_w
  { 0b01110100111110001, 0, 5, 10, 0 }, // xvsrlrn_b_h
  { 0b01110100111110010, 0, 5, 10, 0 }, // xvsrlrn_h_w
  { 0b01110100111110011, 0, 5, 10, 0 }, // xvsrlrn_w_d
  { 0b01110100111111101, 0, 5, 10, 0 }, // xvssran_b_h
  { 0b01110101000001101, 0, 5, 10, 0 }, // xvssran_bu_h
  { 0b01110100111111110, 0, 5, 10, 0 }, // xvssran_h_w
  { 0b01110101000001110, 0, 5, 10, 0 }, // xvssran_hu_w
  { 0b01110100111111111, 0, 5, 10, 0 }, // xvssran_w_d
  { 0b01110101000001111, 0, 5, 10, 0 }, // xvssran_wu_d
  { 0b01110101000000101, 0, 5, 10, 0 }, // xvssrarn_b_h
  { 0b01110101000010101, 0, 5, 10, 0 }, // xvssrarn_bu_h
  { 0b01110101000000110, 0, 5, 10, 0 }, // xvssrarn_h_w
  { 0b01110101000010110, 0, 5, 10, 0 }, // xvssrarn_hu_w
  { 0b01110101000000111, 0, 5, 10, 0 }, // xvssrarn_w_d
  { 0b01110101000010111, 0, 5, 10, 0 }, // xvssrarn_wu_d
  { 0b01110100111111001, 0, 5, 10, 0 }, // xvssrln_b_h
  { 0b01110101000001001, 0, 5, 10, 0 }, // xvssrln_bu_h
  { 0b01110100111111010, 0, 5, 10, 0 }, // xvssrln_h_w
  { 0b01110101000001010, 0, 5, 10, 0 }, // xvssrln_hu_w
  { 0b01110100111111011, 0, 5, 10, 0 }, // xvssrln_w_d
  { 0b01110101000001011, 0, 5, 10, 0 }, // xvssrln_wu_d
  { 0b01110101000000001, 0, 5, 10, 0 }, // xvssrlrn_b_h
  { 0b01110101000010001, 0, 5, 10, 0 }, // xvssrlrn_bu_h
  { 0b01110101000000010, 0, 5, 10, 0 }, // xvssrlrn_h_w
  { 0b01110101000010010, 0, 5, 10, 0 }, // xvssrlrn_hu_w
  { 0b01110101000000011, 0, 5, 10, 0 }, // xvssrlrn_w_d
  { 0b01110101000010011, 0, 5, 10, 0 }, // xvssrlrn_wu_d
  { 0b01110100010010000, 0, 5, 10, 0 }, // xvssub_b
  { 0b01110100010011000, 0, 5, 10, 0 }, // xvssub_bu
  { 0b01110100010010011, 0, 5, 10, 0 }, // xvssub_d
  { 0b01110100010011011, 0, 5, 10, 0 }, // xvssub_du
  { 0b01110100010010001, 0, 5, 10, 0 }, // xvssub_h
  { 0b01110100010011001, 0, 5, 10, 0 }, // xvssub_hu
  { 0b01110100010010010, 0, 5, 10, 0 }, // xvssub_w
  { 0b01110100010011010, 0, 5, 10, 0 }, // xvssub_wu
  { 0b01110100000011000, 0, 5, 10, 0 }, // xvsub_b
  { 0b01110100000011011, 0, 5, 10, 0 }, // xvsub_d
  { 0b01110100000011001, 0, 5, 10, 0 }, // xvsub_h
  { 0b01110101001011011, 0, 5, 10, 0 }, // xvsub_q
  { 0b01110100000011010, 0, 5, 10, 0 }, // xvsub_w
  { 0b01110100001000010, 0, 5, 10, 0 }, // xvsubwev_d_w
  { 0b01110100001100010, 0, 5, 10, 0 }, // xvsubwev_d_wu
  { 0b01110100001000000, 0, 5, 10, 0 }, // xvsubwev_h_b
  { 0b01110100001100000, 0, 5, 10, 0 }, // xvsubwev_h_bu
  { 0b01110100001000011, 0, 5, 10, 0 }, // xvsubwev_q_d
  { 0b01110100001100011, 0, 5, 10, 0 }, // xvsubwev_q_du
  { 0b01110100001000001, 0, 5, 10, 0 }, // xvsubwev_w_h
  { 0b01110100001100001, 0, 5, 10, 0 }, // xvsubwev_w_hu
  { 0b01110100001001010, 0, 5, 10, 0 }, // xvsubwod_d_w
  { 0b01110100001101010, 0, 5, 10, 0 }, // xvsubwod_d_wu
  { 0b01110100001001000, 0, 5, 10, 0 }, // xvsubwod_h_b
  { 0b01110100001101000, 0, 5, 10, 0 }, // xvsubwod_h_bu
  { 0b01110100001001011, 0, 5, 10, 0 }, // xvsubwod_q_d
  { 0b01110100001101011, 0, 5, 10, 0 }, // xvsubwod_q_du
  { 0b01110100001001001, 0, 5, 10, 0 }, // xvsubwod_w_h
  { 0b01110100001101001, 0, 5, 10, 0 }, // xvsubwod_w_hu
  { 0b01110101001001110, 0, 5, 10, 0 }, // xvxor_v
  { 0b0000110010010, 0, 5, 10, 0 }, // xvfcmp_caf_s
  { 0b0000110010011000, 0, 5, 10, 0 }, // xvfcmp_cun_s
  { 0b000011001001100, 0, 5, 10, 0 }, // xvfcmp_ceq_s
  { 0b0000110010011100, 0, 5, 10, 0 }, // xvfcmp_cueq_s
  { 0b00001100100110, 0, 5, 10, 0 }, // xvfcmp_clt_s
  { 0b0000110010011010, 0, 5, 10, 0 }, // xvfcmp_cult_s
  { 0b000011001001110, 0, 5, 10, 0 }, // xvfcmp_cle_s
  { 0b0000110010011110, 0, 5, 10, 0 }, // xvfcmp_cule_s
  { 0b00001100100110000, 0, 5, 10, 0 }, // xvfcmp_cne_s
  { 0b00001100100110100, 0, 5, 10, 0 }, // xvfcmp_cor_s
  { 0b00001100100111000, 0, 5, 10, 0 }, // xvfcmp_cune_s
  { 0b0000110010011, 0, 5, 10, 0 }, // xvfcmp_saf_s
  { 0b0000110010011001, 0, 5, 10, 0 }, // xvfcmp_sun_s
  { 0b000011001001101, 0, 5, 10, 0 }, // xvfcmp_seq_s
  { 0b0000110010011101, 0, 5, 10, 0 }, // xvfcmp_sueq_s
  { 0b00001100100111, 0, 5, 10, 0 }, // xvfcmp_slt_s
  { 0b0000110010011011, 0, 5, 10, 0 }, // xvfcmp_sult_s
  { 0b000011001001111, 0, 5, 10, 0 }, // xvfcmp_sle_s
  { 0b0000110010011111, 0, 5, 10, 0 }, // xvfcmp_sule_s
  { 0b00001100100110001, 0, 5, 10, 0 }, // xvfcmp_sne_s
  { 0b00001100100110101, 0, 5, 10, 0 }, // xvfcmp_sor_s
  { 0b00001100100111001, 0, 5, 10, 0 }, // xvfcmp_sune_s
  { 0b0000110010100, 0, 5, 10, 0 }, // xvfcmp_caf_d
  { 0b0000110010101000, 0, 5, 10, 0 }, // xvfcmp_cun_d
  { 0b000011001010100, 0, 5, 10, 0 }, // xvfcmp_ceq_d
  { 0b0000110010101100, 0, 5, 10, 0 }, // xvfcmp_cueq_d
  { 0b00001100101010, 0, 5, 10, 0 }, // xvfcmp_clt_d
  { 0b0000110010101010, 0, 5, 10, 0 }, // xvfcmp_cult_d
  { 0b000011001010110, 0, 5, 10, 0 }, // xvfcmp_cle_d
  { 0b0000110010101110, 0, 5, 10, 0 }, // xvfcmp_cule_d
  { 0b00001100101010000, 0, 5, 10, 0 }, // xvfcmp_cne_d
  { 0b00001100101010100, 0, 5, 10, 0 }, // xvfcmp_cor_d
  { 0b00001100101011000, 0, 5, 10, 0 }, // xvfcmp_cune_d
  { 0b0000110010101, 0, 5, 10, 0 }, // xvfcmp_saf_d
  { 0b0000110010101001, 0, 5, 10, 0 }, // xvfcmp_sun_d
  { 0b000011001010101, 0, 5, 10, 0 }, // xvfcmp_seq_d
  { 0b0000110010101101, 0, 5, 10, 0 }, // xvfcmp_sueq_d
  { 0b00001100101011, 0, 5, 10, 0 }, // xvfcmp_slt_d
  { 0b0000110010101011, 0, 5, 10, 0 }, // xvfcmp_sult_d
  { 0b000011001010111, 0, 5, 10, 0 }, // xvfcmp_sle_d
  { 0b0000110010101111, 0, 5, 10, 0 }, // xvfcmp_sule_d
  { 0b00001100101010001, 0, 5, 10, 0 }, // xvfcmp_sne_d
  { 0b00001100101010101, 0, 5, 10, 0 }, // xvfcmp_sor_d
  { 0b00001100101011001, 0, 5, 10, 0 }  // xvfcmp_sune_d
};

const LasxXXXX lasxXXXX[10] = {
  { 0b000011010010, 0, 5, 10, 15, 0 }, // xvbitsel_v
  { 0b000010100010, 0, 5, 10, 15, 0 }, // xvfmadd_d
  { 0b000010100001, 0, 5, 10, 15, 0 }, // xvfmadd_s
  { 0b000010100110, 0, 5, 10, 15, 0 }, // xvfmsub_d
  { 0b000010100101, 0, 5, 10, 15, 0 }, // xvfmsub_s
  { 0b000010101010, 0, 5, 10, 15, 0 }, // xvfnmadd_d
  { 0b000010101001, 0, 5, 10, 15, 0 }, // xvfnmadd_s
  { 0b000010101110, 0, 5, 10, 15, 0 }, // xvfnmsub_d
  { 0b000010101101, 0, 5, 10, 15, 0 }, // xvfnmsub_s
  { 0b000011010110, 0, 5, 10, 15, 0 }  // xvshuf_b
};

const LfVVVI lfVVVI[1] = {
  { 0b00001101000000, 0, 5, 10, 3, 15, 0 }  // fsel
};

const LsxIV lsxIV[10] = {
  { 0b0111001010011100101100, 0, 5, 0 }, // vsetallnez_b
  { 0b0111001010011100101111, 0, 5, 0 }, // vsetallnez_d
  { 0b0111001010011100101101, 0, 5, 0 }, // vsetallnez_h
  { 0b0111001010011100101110, 0, 5, 0 }, // vsetallnez_w
  { 0b0111001010011100101000, 0, 5, 0 }, // vsetanyeqz_b
  { 0b0111001010011100101011, 0, 5, 0 }, // vsetanyeqz_d
  { 0b0111001010011100101001, 0, 5, 0 }, // vsetanyeqz_h
  { 0b0111001010011100101010, 0, 5, 0 }, // vsetanyeqz_w
  { 0b0111001010011100100110, 0, 5, 0 }, // vseteqz_v
  { 0b0111001010011100100111, 0, 5, 0 }  // vsetnez_v
};

const LsxRVI lsxRVI[8] = {
  { 0b011100101110111110, kWX, 0, 5, 10, 0 }, // vpickve2gr_b
  { 0b011100101111001110, kWX, 0, 5, 10, 0 }, // vpickve2gr_bu
  { 0b011100101110111111110, kWX, 0, 5, 10, 0 }, // vpickve2gr_d
  { 0b011100101111001111110, kWX, 0, 5, 10, 0 }, // vpickve2gr_du
  { 0b0111001011101111110, kWX, 0, 5, 10, 0 }, // vpickve2gr_h
  { 0b0111001011110011110, kWX, 0, 5, 10, 0 }, // vpickve2gr_hu
  { 0b01110010111011111110, kWX, 0, 5, 10, 0 }, // vpickve2gr_w
  { 0b01110010111100111110, kWX, 0, 5, 10, 0 }  // vpickve2gr_wu
};

const LsxVI lsxVI[1] = {
  { 0b01110011111000, 0, 5, 0 }  // vldi
};

const LsxVII lsxVII[1] = {
  { 0b01110010100110111, 0, 5, 10, 0 }  // vmepatmsk_v
};

const LsxVR lsxVR[4] = {
  { 0b0111001010011111000000, 0, kWX, 5, 0 }, // vreplgr2vr_b
  { 0b0111001010011111000011, 0, kWX, 5, 0 }, // vreplgr2vr_d
  { 0b0111001010011111000001, 0, kWX, 5, 0 }, // vreplgr2vr_h
  { 0b0111001010011111000010, 0, kWX, 5, 0 }  // vreplgr2vr_w
};

const LsxVRI lsxVRI[4] = {
  { 0b011100101110101110, 0, kWX, 5, 10, 0 }, // vinsgr2vr_b
  { 0b011100101110101111110, 0, kWX, 5, 10, 0 }, // vinsgr2vr_d
  { 0b0111001011101011110, 0, kWX, 5, 10, 0 }, // vinsgr2vr_h
  { 0b01110010111010111110, 0, kWX, 5, 10, 0 }  // vinsgr2vr_w
};

const LsxVRII lsxVRII[4] = {
  { 0b0011000110, 0, kWX, 5, 10, 18, 0 }, // vstelm_b
  { 0b0011000100010, 0, kWX, 5, 10, 18, 0 }, // vstelm_d
  { 0b00110001010, 0, kWX, 5, 10, 18, 0 }, // vstelm_h
  { 0b001100010010, 0, kWX, 5, 10, 18, 0 }  // vstelm_w
};

const LsxVV lsxVV[86] = {
  { 0b0111001010011100000000, 0, 5, 0 }, // vclo_b
  { 0b0111001010011100000011, 0, 5, 0 }, // vclo_d
  { 0b0111001010011100000001, 0, 5, 0 }, // vclo_h
  { 0b0111001010011100000010, 0, 5, 0 }, // vclo_w
  { 0b0111001010011100000100, 0, 5, 0 }, // vclz_b
  { 0b0111001010011100000111, 0, 5, 0 }, // vclz_d
  { 0b0111001010011100000101, 0, 5, 0 }, // vclz_h
  { 0b0111001010011100000110, 0, 5, 0 }, // vclz_w
  { 0b0111001010011110111010, 0, 5, 0 }, // vexth_d_w
  { 0b0111001010011110111110, 0, 5, 0 }, // vexth_du_wu
  { 0b0111001010011110111000, 0, 5, 0 }, // vexth_h_b
  { 0b0111001010011110111100, 0, 5, 0 }, // vexth_hu_bu
  { 0b0111001010011110111011, 0, 5, 0 }, // vexth_q_d
  { 0b0111001010011110111111, 0, 5, 0 }, // vexth_qu_du
  { 0b0111001010011110111001, 0, 5, 0 }, // vexth_w_h
  { 0b0111001010011110111101, 0, 5, 0 }, // vexth_wu_hu
  { 0b0111001100001001000000, 0, 5, 0 }, // vextl_q_d
  { 0b0111001100001101000000, 0, 5, 0 }, // vextl_qu_du
  { 0b0111001010011100110110, 0, 5, 0 }, // vfclass_d
  { 0b0111001010011100110101, 0, 5, 0 }, // vfclass_s
  { 0b0111001010011101111101, 0, 5, 0 }, // vfcvth_d_s
  { 0b0111001010011101111011, 0, 5, 0 }, // vfcvth_s_h
  { 0b0111001010011101111100, 0, 5, 0 }, // vfcvtl_d_s
  { 0b0111001010011101111010, 0, 5, 0 }, // vfcvtl_s_h
  { 0b0111001010011110000010, 0, 5, 0 }, // vffint_d_l
  { 0b0111001010011110000011, 0, 5, 0 }, // vffint_d_lu
  { 0b0111001010011110000000, 0, 5, 0 }, // vffint_s_w
  { 0b0111001010011110000001, 0, 5, 0 }, // vffint_s_wu
  { 0b0111001010011110000101, 0, 5, 0 }, // vffinth_d_w
  { 0b0111001010011110000100, 0, 5, 0 }, // vffintl_d_w
  { 0b0111001010011100110010, 0, 5, 0 }, // vflogb_d
  { 0b0111001010011100110001, 0, 5, 0 }, // vflogb_s
  { 0b0111001010011100111110, 0, 5, 0 }, // vfrecip_d
  { 0b0111001010011100111101, 0, 5, 0 }, // vfrecip_s
  { 0b0111001010011101001110, 0, 5, 0 }, // vfrint_d
  { 0b0111001010011101001101, 0, 5, 0 }, // vfrint_s
  { 0b0111001010011101010010, 0, 5, 0 }, // vfrintrm_d
  { 0b0111001010011101010001, 0, 5, 0 }, // vfrintrm_s
  { 0b0111001010011101011110, 0, 5, 0 }, // vfrintrne_d
  { 0b0111001010011101011101, 0, 5, 0 }, // vfrintrne_s
  { 0b0111001010011101010110, 0, 5, 0 }, // vfrintrp_d
  { 0b0111001010011101010101, 0, 5, 0 }, // vfrintrp_s
  { 0b0111001010011101011010, 0, 5, 0 }, // vfrintrz_d
  { 0b0111001010011101011001, 0, 5, 0 }, // vfrintrz_s
  { 0b0111001010011101000010, 0, 5, 0 }, // vfrsqrt_d
  { 0b0111001010011101000001, 0, 5, 0 }, // vfrsqrt_s
  { 0b0111001010011100111010, 0, 5, 0 }, // vfsqrt_d
  { 0b0111001010011100111001, 0, 5, 0 }, // vfsqrt_s
  { 0b0111001010011110001101, 0, 5, 0 }, // vftint_l_d
  { 0b0111001010011110010111, 0, 5, 0 }, // vftint_lu_d
  { 0b0111001010011110001100, 0, 5, 0 }, // vftint_w_s
  { 0b0111001010011110010110, 0, 5, 0 }, // vftint_wu_s
  { 0b0111001010011110100001, 0, 5, 0 }, // vftinth_l_s
  { 0b0111001010011110100000, 0, 5, 0 }, // vftintl_l_s
  { 0b0111001010011110001111, 0, 5, 0 }, // vftintrm_l_d
  { 0b0111001010011110001110, 0, 5, 0 }, // vftintrm_w_s
  { 0b0111001010011110100011, 0, 5, 0 }, // vftintrmh_l_s
  { 0b0111001010011110100010, 0, 5, 0 }, // vftintrml_l_s
  { 0b0111001010011110010101, 0, 5, 0 }, // vftintrne_l_d
  { 0b0111001010011110010100, 0, 5, 0 }, // vftintrne_w_s
  { 0b0111001010011110101001, 0, 5, 0 }, // vftintrneh_l_s
  { 0b0111001010011110101000, 0, 5, 0 }, // vftintrnel_l_s
  { 0b0111001010011110010001, 0, 5, 0 }, // vftintrp_l_d
  { 0b0111001010011110010000, 0, 5, 0 }, // vftintrp_w_s
  { 0b0111001010011110100101, 0, 5, 0 }, // vftintrph_l_s
  { 0b0111001010011110100100, 0, 5, 0 }, // vftintrpl_l_s
  { 0b0111001010011110010011, 0, 5, 0 }, // vftintrz_l_d
  { 0b0111001010011110011101, 0, 5, 0 }, // vftintrz_lu_d
  { 0b0111001010011110010010, 0, 5, 0 }, // vftintrz_w_s
  { 0b0111001010011110011100, 0, 5, 0 }, // vftintrz_wu_s
  { 0b0111001010011110100111, 0, 5, 0 }, // vftintrzh_l_s
  { 0b0111001010011110100110, 0, 5, 0 }, // vftintrzl_l_s
  { 0b0111001010011100010100, 0, 5, 0 }, // vmskgez_b
  { 0b0111001010011100010000, 0, 5, 0 }, // vmskltz_b
  { 0b0111001010011100010011, 0, 5, 0 }, // vmskltz_d
  { 0b0111001010011100010001, 0, 5, 0 }, // vmskltz_h
  { 0b0111001010011100010010, 0, 5, 0 }, // vmskltz_w
  { 0b0111001010011100011000, 0, 5, 0 }, // vmsknz_b
  { 0b0111001010011100001100, 0, 5, 0 }, // vneg_b
  { 0b0111001010011100001111, 0, 5, 0 }, // vneg_d
  { 0b0111001010011100001101, 0, 5, 0 }, // vneg_h
  { 0b0111001010011100001110, 0, 5, 0 }, // vneg_w
  { 0b0111001010011100001000, 0, 5, 0 }, // vpcnt_b
  { 0b0111001010011100001011, 0, 5, 0 }, // vpcnt_d
  { 0b0111001010011100001001, 0, 5, 0 }, // vpcnt_h
  { 0b0111001010011100001010, 0, 5, 0 }  // vpcnt_w
};

const LsxVVI lsxVVI[160] = {
  { 0b01110010100010100, 0, 5, 10, 0 }, // vaddi_bu
  { 0b01110010100010111, 0, 5, 10, 0 }, // vaddi_du
  { 0b01110010100010101, 0, 5, 10, 0 }, // vaddi_hu
  { 0b01110010100010110, 0, 5, 10, 0 }, // vaddi_wu
  { 0b01110011110100, 0, 5, 10, 0 }, // vandi_b
  { 0b0111001100010000001, 0, 5, 10, 0 }, // vbitclri_b
  { 0b0111001100010001, 0, 5, 10, 0 }, // vbitclri_d
  { 0b011100110001000001, 0, 5, 10, 0 }, // vbitclri_h
  { 0b01110011000100001, 0, 5, 10, 0 }, // vbitclri_w
  { 0b0111001100011000001, 0, 5, 10, 0 }, // vbitrevi_b
  { 0b0111001100011001, 0, 5, 10, 0 }, // vbitrevi_d
  { 0b011100110001100001, 0, 5, 10, 0 }, // vbitrevi_h
  { 0b01110011000110001, 0, 5, 10, 0 }, // vbitrevi_w
  { 0b01110011110001, 0, 5, 10, 0 }, // vbitseli_b
  { 0b0111001100010100001, 0, 5, 10, 0 }, // vbitseti_b
  { 0b0111001100010101, 0, 5, 10, 0 }, // vbitseti_d
  { 0b011100110001010001, 0, 5, 10, 0 }, // vbitseti_h
  { 0b01110011000101001, 0, 5, 10, 0 }, // vbitseti_w
  { 0b01110010100011100, 0, 5, 10, 0 }, // vbsll_v
  { 0b01110010100011101, 0, 5, 10, 0 }, // vbsrl_v
  { 0b01110011100011, 0, 5, 10, 0 }, // vextrins_b
  { 0b01110011100000, 0, 5, 10, 0 }, // vextrins_d
  { 0b01110011100010, 0, 5, 10, 0 }, // vextrins_h
  { 0b01110011100001, 0, 5, 10, 0 }, // vextrins_w
  { 0b01110010100110100, 0, 5, 10, 0 }, // vfrstpi_b
  { 0b01110010100110101, 0, 5, 10, 0 }, // vfrstpi_h
  { 0b01110010100100000, 0, 5, 10, 0 }, // vmaxi_b
  { 0b01110010100101000, 0, 5, 10, 0 }, // vmaxi_bu
  { 0b01110010100100011, 0, 5, 10, 0 }, // vmaxi_d
  { 0b01110010100101011, 0, 5, 10, 0 }, // vmaxi_du
  { 0b01110010100100001, 0, 5, 10, 0 }, // vmaxi_h
  { 0b01110010100101001, 0, 5, 10, 0 }, // vmaxi_hu
  { 0b01110010100100010, 0, 5, 10, 0 }, // vmaxi_w
  { 0b01110010100101010, 0, 5, 10, 0 }, // vmaxi_wu
  { 0b01110010100100100, 0, 5, 10, 0 }, // vmini_b
  { 0b01110010100101100, 0, 5, 10, 0 }, // vmini_bu
  { 0b01110010100100111, 0, 5, 10, 0 }, // vmini_d
  { 0b01110010100101111, 0, 5, 10, 0 }, // vmini_du
  { 0b01110010100100101, 0, 5, 10, 0 }, // vmini_h
  { 0b01110010100101101, 0, 5, 10, 0 }, // vmini_hu
  { 0b01110010100100110, 0, 5, 10, 0 }, // vmini_w
  { 0b01110010100101110, 0, 5, 10, 0 }, // vmini_wu
  { 0b01110011110111, 0, 5, 10, 0 }, // vnori_b
  { 0b01110011110101, 0, 5, 10, 0 }, // vori_b
  { 0b01110011111001, 0, 5, 10, 0 }, // vpermi_w
  { 0b011100101111011110, 0, 5, 10, 0 }, // vreplvei_b
  { 0b011100101111011111110, 0, 5, 10, 0 }, // vreplvei_d
  { 0b0111001011110111110, 0, 5, 10, 0 }, // vreplvei_h
  { 0b01110010111101111110, 0, 5, 10, 0 }, // vreplvei_w
  { 0b0111001010100000001, 0, 5, 10, 0 }, // vrotri_b
  { 0b0111001010100001, 0, 5, 10, 0 }, // vrotri_d
  { 0b011100101010000001, 0, 5, 10, 0 }, // vrotri_h
  { 0b01110010101000001, 0, 5, 10, 0 }, // vrotri_w
  { 0b0111001100100100001, 0, 5, 10, 0 }, // vsat_b
  { 0b0111001100101000001, 0, 5, 10, 0 }, // vsat_bu
  { 0b0111001100100101, 0, 5, 10, 0 }, // vsat_d
  { 0b0111001100101001, 0, 5, 10, 0 }, // vsat_du
  { 0b011100110010010001, 0, 5, 10, 0 }, // vsat_h
  { 0b011100110010100001, 0, 5, 10, 0 }, // vsat_hu
  { 0b01110011001001001, 0, 5, 10, 0 }, // vsat_w
  { 0b01110011001010001, 0, 5, 10, 0 }, // vsat_wu
  { 0b01110010100000000, 0, 5, 10, 0 }, // vseqi_b
  { 0b01110010100000011, 0, 5, 10, 0 }, // vseqi_d
  { 0b01110010100000001, 0, 5, 10, 0 }, // vseqi_h
  { 0b01110010100000010, 0, 5, 10, 0 }, // vseqi_w
  { 0b01110011100100, 0, 5, 10, 0 }, // vshuf4i_b
  { 0b01110011100111, 0, 5, 10, 0 }, // vshuf4i_d
  { 0b01110011100101, 0, 5, 10, 0 }, // vshuf4i_h
  { 0b01110011100110, 0, 5, 10, 0 }, // vshuf4i_w
  { 0b01110010100000100, 0, 5, 10, 0 }, // vslei_b
  { 0b01110010100001000, 0, 5, 10, 0 }, // vslei_bu
  { 0b01110010100000111, 0, 5, 10, 0 }, // vslei_d
  { 0b01110010100001011, 0, 5, 10, 0 }, // vslei_du
  { 0b01110010100000101, 0, 5, 10, 0 }, // vslei_h
  { 0b01110010100001001, 0, 5, 10, 0 }, // vslei_hu
  { 0b01110010100000110, 0, 5, 10, 0 }, // vslei_w
  { 0b01110010100001010, 0, 5, 10, 0 }, // vslei_wu
  { 0b0111001100101100001, 0, 5, 10, 0 }, // vslli_b
  { 0b0111001100101101, 0, 5, 10, 0 }, // vslli_d
  { 0b011100110010110001, 0, 5, 10, 0 }, // vslli_h
  { 0b01110011001011001, 0, 5, 10, 0 }, // vslli_w
  { 0b01110011000010001, 0, 5, 10, 0 }, // vsllwil_d_w
  { 0b01110011000011001, 0, 5, 10, 0 }, // vsllwil_du_wu
  { 0b0111001100001000001, 0, 5, 10, 0 }, // vsllwil_h_b
  { 0b0111001100001100001, 0, 5, 10, 0 }, // vsllwil_hu_bu
  { 0b011100110000100001, 0, 5, 10, 0 }, // vsllwil_w_h
  { 0b011100110000110001, 0, 5, 10, 0 }, // vsllwil_wu_hu
  { 0b01110010100001100, 0, 5, 10, 0 }, // vslti_b
  { 0b01110010100010000, 0, 5, 10, 0 }, // vslti_bu
  { 0b01110010100001111, 0, 5, 10, 0 }, // vslti_d
  { 0b01110010100010011, 0, 5, 10, 0 }, // vslti_du
  { 0b01110010100001101, 0, 5, 10, 0 }, // vslti_h
  { 0b01110010100010001, 0, 5, 10, 0 }, // vslti_hu
  { 0b01110010100001110, 0, 5, 10, 0 }, // vslti_w
  { 0b01110010100010010, 0, 5, 10, 0 }, // vslti_wu
  { 0b0111001100110100001, 0, 5, 10, 0 }, // vsrai_b
  { 0b0111001100110101, 0, 5, 10, 0 }, // vsrai_d
  { 0b011100110011010001, 0, 5, 10, 0 }, // vsrai_h
  { 0b01110011001101001, 0, 5, 10, 0 }, // vsrai_w
  { 0b011100110101100001, 0, 5, 10, 0 }, // vsrani_b_h
  { 0b011100110101101, 0, 5, 10, 0 }, // vsrani_d_q
  { 0b01110011010110001, 0, 5, 10, 0 }, // vsrani_h_w
  { 0b0111001101011001, 0, 5, 10, 0 }, // vsrani_w_d
  { 0b0111001010101000001, 0, 5, 10, 0 }, // vsrari_b
  { 0b0111001010101001, 0, 5, 10, 0 }, // vsrari_d
  { 0b011100101010100001, 0, 5, 10, 0 }, // vsrari_h
  { 0b01110010101010001, 0, 5, 10, 0 }, // vsrari_w
  { 0b011100110101110001, 0, 5, 10, 0 }, // vsrarni_b_h
  { 0b011100110101111, 0, 5, 10, 0 }, // vsrarni_d_q
  { 0b01110011010111001, 0, 5, 10, 0 }, // vsrarni_h_w
  { 0b0111001101011101, 0, 5, 10, 0 }, // vsrarni_w_d
  { 0b0111001100110000001, 0, 5, 10, 0 }, // vsrli_b
  { 0b0111001100110001, 0, 5, 10, 0 }, // vsrli_d
  { 0b011100110011000001, 0, 5, 10, 0 }, // vsrli_h
  { 0b01110011001100001, 0, 5, 10, 0 }, // vsrli_w
  { 0b011100110100000001, 0, 5, 10, 0 }, // vsrlni_b_h
  { 0b011100110100001, 0, 5, 10, 0 }, // vsrlni_d_q
  { 0b01110011010000001, 0, 5, 10, 0 }, // vsrlni_h_w
  { 0b0111001101000001, 0, 5, 10, 0 }, // vsrlni_w_d
  { 0b0111001010100100001, 0, 5, 10, 0 }, // vsrlri_b
  { 0b0111001010100101, 0, 5, 10, 0 }, // vsrlri_d
  { 0b011100101010010001, 0, 5, 10, 0 }, // vsrlri_h
  { 0b01110010101001001, 0, 5, 10, 0 }, // vsrlri_w
  { 0b011100110110000001, 0, 5, 10, 0 }, // vssrani_b_h
  { 0b011100110110010001, 0, 5, 10, 0 }, // vssrani_bu_h
  { 0b011100110110001, 0, 5, 10, 0 }, // vssrani_d_q
  { 0b011100110110011, 0, 5, 10, 0 }, // vssrani_du_q
  { 0b01110011011000001, 0, 5, 10, 0 }, // vssrani_h_w
  { 0b01110011011001001, 0, 5, 10, 0 }, // vssrani_hu_w
  { 0b0111001101100001, 0, 5, 10, 0 }, // vssrani_w_d
  { 0b0111001101100101, 0, 5, 10, 0 }, // vssrani_wu_d
  { 0b011100110110100001, 0, 5, 10, 0 }, // vssrarni_b_h
  { 0b011100110110110001, 0, 5, 10, 0 }, // vssrarni_bu_h
  { 0b011100110110101, 0, 5, 10, 0 }, // vssrarni_d_q
  { 0b011100110110111, 0, 5, 10, 0 }, // vssrarni_du_q
  { 0b01110011011010001, 0, 5, 10, 0 }, // vssrarni_h_w
  { 0b01110011011011001, 0, 5, 10, 0 }, // vssrarni_hu_w
  { 0b0111001101101001, 0, 5, 10, 0 }, // vssrarni_w_d
  { 0b0111001101101101, 0, 5, 10, 0 }, // vssrarni_wu_d
  { 0b011100110100100001, 0, 5, 10, 0 }, // vssrlni_b_h
  { 0b011100110100110001, 0, 5, 10, 0 }, // vssrlni_bu_h
  { 0b011100110100101, 0, 5, 10, 0 }, // vssrlni_d_q
  { 0b011100110100111, 0, 5, 10, 0 }, // vssrlni_du_q
  { 0b01110011010010001, 0, 5, 10, 0 }, // vssrlni_h_w
  { 0b01110011010011001, 0, 5, 10, 0 }, // vssrlni_hu_w
  { 0b0111001101001001, 0, 5, 10, 0 }, // vssrlni_w_d
  { 0b0111001101001101, 0, 5, 10, 0 }, // vssrlni_wu_d
  { 0b011100110101000001, 0, 5, 10, 0 }, // vssrlrni_b_h
  { 0b011100110101010001, 0, 5, 10, 0 }, // vssrlrni_bu_h
  { 0b011100110101001, 0, 5, 10, 0 }, // vssrlrni_d_q
  { 0b011100110101011, 0, 5, 10, 0 }, // vssrlrni_du_q
  { 0b01110011010100001, 0, 5, 10, 0 }, // vssrlrni_h_w
  { 0b01110011010101001, 0, 5, 10, 0 }, // vssrlrni_hu_w
  { 0b0111001101010001, 0, 5, 10, 0 }, // vssrlrni_w_d
  { 0b0111001101010101, 0, 5, 10, 0 }, // vssrlrni_wu_d
  { 0b01110010100011000, 0, 5, 10, 0 }, // vsubi_bu
  { 0b01110010100011011, 0, 5, 10, 0 }, // vsubi_du
  { 0b01110010100011001, 0, 5, 10, 0 }, // vsubi_hu
  { 0b01110010100011010, 0, 5, 10, 0 }, // vsubi_wu
  { 0b01110011110110, 0, 5, 10, 0 }  // vxori_b
};

const LsxVVR lsxVVR[4] = {
  { 0b01110001001000100, 0, 5, kWX, 10, 0 }, // vreplve_b
  { 0b01110001001000111, 0, 5, kWX, 10, 0 }, // vreplve_d
  { 0b01110001001000101, 0, 5, kWX, 10, 0 }, // vreplve_h
  { 0b01110001001000110, 0, 5, kWX, 10, 0 }  // vreplve_w
};

const LsxVVV lsxVVV[411] = {
  { 0b01110000011000000, 0, 5, 10, 0 }, // vabsd_b
  { 0b01110000011000100, 0, 5, 10, 0 }, // vabsd_bu
  { 0b01110000011000011, 0, 5, 10, 0 }, // vabsd_d
  { 0b01110000011000111, 0, 5, 10, 0 }, // vabsd_du
  { 0b01110000011000001, 0, 5, 10, 0 }, // vabsd_h
  { 0b01110000011000101, 0, 5, 10, 0 }, // vabsd_hu
  { 0b01110000011000010, 0, 5, 10, 0 }, // vabsd_w
  { 0b01110000011000110, 0, 5, 10, 0 }, // vabsd_wu
  { 0b01110000000010100, 0, 5, 10, 0 }, // vadd_b
  { 0b01110000000010111, 0, 5, 10, 0 }, // vadd_d
  { 0b01110000000010101, 0, 5, 10, 0 }, // vadd_h
  { 0b01110001001011010, 0, 5, 10, 0 }, // vadd_q
  { 0b01110000000010110, 0, 5, 10, 0 }, // vadd_w
  { 0b01110000010111000, 0, 5, 10, 0 }, // vadda_b
  { 0b01110000010111011, 0, 5, 10, 0 }, // vadda_d
  { 0b01110000010111001, 0, 5, 10, 0 }, // vadda_h
  { 0b01110000010111010, 0, 5, 10, 0 }, // vadda_w
  { 0b01110000000111110, 0, 5, 10, 0 }, // vaddwev_d_w
  { 0b01110000001011110, 0, 5, 10, 0 }, // vaddwev_d_wu
  { 0b01110000001111110, 0, 5, 10, 0 }, // vaddwev_d_wu_w
  { 0b01110000000111100, 0, 5, 10, 0 }, // vaddwev_h_b
  { 0b01110000001011100, 0, 5, 10, 0 }, // vaddwev_h_bu
  { 0b01110000001111100, 0, 5, 10, 0 }, // vaddwev_h_bu_b
  { 0b01110000000111111, 0, 5, 10, 0 }, // vaddwev_q_d
  { 0b01110000001011111, 0, 5, 10, 0 }, // vaddwev_q_du
  { 0b01110000001111111, 0, 5, 10, 0 }, // vaddwev_q_du_d
  { 0b01110000000111101, 0, 5, 10, 0 }, // vaddwev_w_h
  { 0b01110000001011101, 0, 5, 10, 0 }, // vaddwev_w_hu
  { 0b01110000001111101, 0, 5, 10, 0 }, // vaddwev_w_hu_h
  { 0b01110000001000110, 0, 5, 10, 0 }, // vaddwod_d_w
  { 0b01110000001100110, 0, 5, 10, 0 }, // vaddwod_d_wu
  { 0b01110000010000010, 0, 5, 10, 0 }, // vaddwod_d_wu_w
  { 0b01110000001000100, 0, 5, 10, 0 }, // vaddwod_h_b
  { 0b01110000001100100, 0, 5, 10, 0 }, // vaddwod_h_bu
  { 0b01110000010000000, 0, 5, 10, 0 }, // vaddwod_h_bu_b
  { 0b01110000001000111, 0, 5, 10, 0 }, // vaddwod_q_d
  { 0b01110000001100111, 0, 5, 10, 0 }, // vaddwod_q_du
  { 0b01110000010000011, 0, 5, 10, 0 }, // vaddwod_q_du_d
  { 0b01110000001000101, 0, 5, 10, 0 }, // vaddwod_w_h
  { 0b01110000001100101, 0, 5, 10, 0 }, // vaddwod_w_hu
  { 0b01110000010000001, 0, 5, 10, 0 }, // vaddwod_w_hu_h
  { 0b01110001001001100, 0, 5, 10, 0 }, // vand_v
  { 0b01110001001010000, 0, 5, 10, 0 }, // vandn_v
  { 0b01110000011001000, 0, 5, 10, 0 }, // vavg_b
  { 0b01110000011001100, 0, 5, 10, 0 }, // vavg_bu
  { 0b01110000011001011, 0, 5, 10, 0 }, // vavg_d
  { 0b01110000011001111, 0, 5, 10, 0 }, // vavg_du
  { 0b01110000011001001, 0, 5, 10, 0 }, // vavg_h
  { 0b01110000011001101, 0, 5, 10, 0 }, // vavg_hu
  { 0b01110000011001010, 0, 5, 10, 0 }, // vavg_w
  { 0b01110000011001110, 0, 5, 10, 0 }, // vavg_wu
  { 0b01110000011010000, 0, 5, 10, 0 }, // vavgr_b
  { 0b01110000011010100, 0, 5, 10, 0 }, // vavgr_bu
  { 0b01110000011010011, 0, 5, 10, 0 }, // vavgr_d
  { 0b01110000011010111, 0, 5, 10, 0 }, // vavgr_du
  { 0b01110000011010001, 0, 5, 10, 0 }, // vavgr_h
  { 0b01110000011010101, 0, 5, 10, 0 }, // vavgr_hu
  { 0b01110000011010010, 0, 5, 10, 0 }, // vavgr_w
  { 0b01110000011010110, 0, 5, 10, 0 }, // vavgr_wu
  { 0b01110001000011000, 0, 5, 10, 0 }, // vbitclr_b
  { 0b01110001000011011, 0, 5, 10, 0 }, // vbitclr_d
  { 0b01110001000011001, 0, 5, 10, 0 }, // vbitclr_h
  { 0b01110001000011010, 0, 5, 10, 0 }, // vbitclr_w
  { 0b01110001000100000, 0, 5, 10, 0 }, // vbitrev_b
  { 0b01110001000100011, 0, 5, 10, 0 }, // vbitrev_d
  { 0b01110001000100001, 0, 5, 10, 0 }, // vbitrev_h
  { 0b01110001000100010, 0, 5, 10, 0 }, // vbitrev_w
  { 0b01110001000011100, 0, 5, 10, 0 }, // vbitset_b
  { 0b01110001000011111, 0, 5, 10, 0 }, // vbitset_d
  { 0b01110001000011101, 0, 5, 10, 0 }, // vbitset_h
  { 0b01110001000011110, 0, 5, 10, 0 }, // vbitset_w
  { 0b01110000111000000, 0, 5, 10, 0 }, // vdiv_b
  { 0b01110000111001000, 0, 5, 10, 0 }, // vdiv_bu
  { 0b01110000111000011, 0, 5, 10, 0 }, // vdiv_d
  { 0b01110000111001011, 0, 5, 10, 0 }, // vdiv_du
  { 0b01110000111000001, 0, 5, 10, 0 }, // vdiv_h
  { 0b01110000111001001, 0, 5, 10, 0 }, // vdiv_hu
  { 0b01110000111000010, 0, 5, 10, 0 }, // vdiv_w
  { 0b01110000111001010, 0, 5, 10, 0 }, // vdiv_wu
  { 0b01110001001100010, 0, 5, 10, 0 }, // vfadd_d
  { 0b01110001001100001, 0, 5, 10, 0 }, // vfadd_s
  { 0b01110001010001100, 0, 5, 10, 0 }, // vfcvt_h_s
  { 0b01110001010001101, 0, 5, 10, 0 }, // vfcvt_s_d
  { 0b01110001001110110, 0, 5, 10, 0 }, // vfdiv_d
  { 0b01110001001110101, 0, 5, 10, 0 }, // vfdiv_s
  { 0b01110001010010000, 0, 5, 10, 0 }, // vffint_s_l
  { 0b01110001001111010, 0, 5, 10, 0 }, // vfmax_d
  { 0b01110001001111001, 0, 5, 10, 0 }, // vfmax_s
  { 0b01110001010000010, 0, 5, 10, 0 }, // vfmaxa_d
  { 0b01110001010000001, 0, 5, 10, 0 }, // vfmaxa_s
  { 0b01110001001111110, 0, 5, 10, 0 }, // vfmin_d
  { 0b01110001001111101, 0, 5, 10, 0 }, // vfmin_s
  { 0b01110001010000110, 0, 5, 10, 0 }, // vfmina_d
  { 0b01110001010000101, 0, 5, 10, 0 }, // vfmina_s
  { 0b01110001001110010, 0, 5, 10, 0 }, // vfmul_d
  { 0b01110001001110001, 0, 5, 10, 0 }, // vfmul_s
  { 0b01110001001010110, 0, 5, 10, 0 }, // vfrstp_b
  { 0b01110001001010111, 0, 5, 10, 0 }, // vfrstp_h
  { 0b01110001010001010, 0, 5, 10, 0 }, // vfscaleb_d
  { 0b01110001010001001, 0, 5, 10, 0 }, // vfscaleb_s
  { 0b01110001001100110, 0, 5, 10, 0 }, // vfsub_d
  { 0b01110001001100101, 0, 5, 10, 0 }, // vfsub_s
  { 0b01110001010010011, 0, 5, 10, 0 }, // vftint_w_d
  { 0b01110001010010100, 0, 5, 10, 0 }, // vftintrm_w_d
  { 0b01110001010010111, 0, 5, 10, 0 }, // vftintrne_w_d
  { 0b01110001010010101, 0, 5, 10, 0 }, // vftintrp_w_d
  { 0b01110001010010110, 0, 5, 10, 0 }, // vftintrz_w_d
  { 0b01110000010101010, 0, 5, 10, 0 }, // vhaddw_d_w
  { 0b01110000010110010, 0, 5, 10, 0 }, // vhaddw_du_wu
  { 0b01110000010101000, 0, 5, 10, 0 }, // vhaddw_h_b
  { 0b01110000010110000, 0, 5, 10, 0 }, // vhaddw_hu_bu
  { 0b01110000010101011, 0, 5, 10, 0 }, // vhaddw_q_d
  { 0b01110000010110011, 0, 5, 10, 0 }, // vhaddw_qu_du
  { 0b01110000010101001, 0, 5, 10, 0 }, // vhaddw_w_h
  { 0b01110000010110001, 0, 5, 10, 0 }, // vhaddw_wu_hu
  { 0b01110000010101110, 0, 5, 10, 0 }, // vhsubw_d_w
  { 0b01110000010110110, 0, 5, 10, 0 }, // vhsubw_du_wu
  { 0b01110000010101100, 0, 5, 10, 0 }, // vhsubw_h_b
  { 0b01110000010110100, 0, 5, 10, 0 }, // vhsubw_hu_bu
  { 0b01110000010101111, 0, 5, 10, 0 }, // vhsubw_q_d
  { 0b01110000010110111, 0, 5, 10, 0 }, // vhsubw_qu_du
  { 0b01110000010101101, 0, 5, 10, 0 }, // vhsubw_w_h
  { 0b01110000010110101, 0, 5, 10, 0 }, // vhsubw_wu_hu
  { 0b01110001000111000, 0, 5, 10, 0 }, // vilvh_b
  { 0b01110001000111011, 0, 5, 10, 0 }, // vilvh_d
  { 0b01110001000111001, 0, 5, 10, 0 }, // vilvh_h
  { 0b01110001000111010, 0, 5, 10, 0 }, // vilvh_w
  { 0b01110001000110100, 0, 5, 10, 0 }, // vilvl_b
  { 0b01110001000110111, 0, 5, 10, 0 }, // vilvl_d
  { 0b01110001000110101, 0, 5, 10, 0 }, // vilvl_h
  { 0b01110001000110110, 0, 5, 10, 0 }, // vilvl_w
  { 0b01110000101010000, 0, 5, 10, 0 }, // vmadd_b
  { 0b01110000101010011, 0, 5, 10, 0 }, // vmadd_d
  { 0b01110000101010001, 0, 5, 10, 0 }, // vmadd_h
  { 0b01110000101010010, 0, 5, 10, 0 }, // vmadd_w
  { 0b01110000101011010, 0, 5, 10, 0 }, // vmaddwev_d_w
  { 0b01110000101101010, 0, 5, 10, 0 }, // vmaddwev_d_wu
  { 0b01110000101111010, 0, 5, 10, 0 }, // vmaddwev_d_wu_w
  { 0b01110000101011000, 0, 5, 10, 0 }, // vmaddwev_h_b
  { 0b01110000101101000, 0, 5, 10, 0 }, // vmaddwev_h_bu
  { 0b01110000101111000, 0, 5, 10, 0 }, // vmaddwev_h_bu_b
  { 0b01110000101011011, 0, 5, 10, 0 }, // vmaddwev_q_d
  { 0b01110000101101011, 0, 5, 10, 0 }, // vmaddwev_q_du
  { 0b01110000101111011, 0, 5, 10, 0 }, // vmaddwev_q_du_d
  { 0b01110000101011001, 0, 5, 10, 0 }, // vmaddwev_w_h
  { 0b01110000101101001, 0, 5, 10, 0 }, // vmaddwev_w_hu
  { 0b01110000101111001, 0, 5, 10, 0 }, // vmaddwev_w_hu_h
  { 0b01110000101011110, 0, 5, 10, 0 }, // vmaddwod_d_w
  { 0b01110000101101110, 0, 5, 10, 0 }, // vmaddwod_d_wu
  { 0b01110000101111110, 0, 5, 10, 0 }, // vmaddwod_d_wu_w
  { 0b01110000101011100, 0, 5, 10, 0 }, // vmaddwod_h_b
  { 0b01110000101101100, 0, 5, 10, 0 }, // vmaddwod_h_bu
  { 0b01110000101111100, 0, 5, 10, 0 }, // vmaddwod_h_bu_b
  { 0b01110000101011111, 0, 5, 10, 0 }, // vmaddwod_q_d
  { 0b01110000101101111, 0, 5, 10, 0 }, // vmaddwod_q_du
  { 0b01110000101111111, 0, 5, 10, 0 }, // vmaddwod_q_du_d
  { 0b01110000101011101, 0, 5, 10, 0 }, // vmaddwod_w_h
  { 0b01110000101101101, 0, 5, 10, 0 }, // vmaddwod_w_hu
  { 0b01110000101111101, 0, 5, 10, 0 }, // vmaddwod_w_hu_h
  { 0b01110000011100000, 0, 5, 10, 0 }, // vmax_b
  { 0b01110000011101000, 0, 5, 10, 0 }, // vmax_bu
  { 0b01110000011100011, 0, 5, 10, 0 }, // vmax_d
  { 0b01110000011101011, 0, 5, 10, 0 }, // vmax_du
  { 0b01110000011100001, 0, 5, 10, 0 }, // vmax_h
  { 0b01110000011101001, 0, 5, 10, 0 }, // vmax_hu
  { 0b01110000011100010, 0, 5, 10, 0 }, // vmax_w
  { 0b01110000011101010, 0, 5, 10, 0 }, // vmax_wu
  { 0b01110000011100100, 0, 5, 10, 0 }, // vmin_b
  { 0b01110000011101100, 0, 5, 10, 0 }, // vmin_bu
  { 0b01110000011100111, 0, 5, 10, 0 }, // vmin_d
  { 0b01110000011101111, 0, 5, 10, 0 }, // vmin_du
  { 0b01110000011100101, 0, 5, 10, 0 }, // vmin_h
  { 0b01110000011101101, 0, 5, 10, 0 }, // vmin_hu
  { 0b01110000011100110, 0, 5, 10, 0 }, // vmin_w
  { 0b01110000011101110, 0, 5, 10, 0 }, // vmin_wu
  { 0b01110000111000100, 0, 5, 10, 0 }, // vmod_b
  { 0b01110000111001100, 0, 5, 10, 0 }, // vmod_bu
  { 0b01110000111000111, 0, 5, 10, 0 }, // vmod_d
  { 0b01110000111001111, 0, 5, 10, 0 }, // vmod_du
  { 0b01110000111000101, 0, 5, 10, 0 }, // vmod_h
  { 0b01110000111001101, 0, 5, 10, 0 }, // vmod_hu
  { 0b01110000111000110, 0, 5, 10, 0 }, // vmod_w
  { 0b01110000111001110, 0, 5, 10, 0 }, // vmod_wu
  { 0b01110000101010100, 0, 5, 10, 0 }, // vmsub_b
  { 0b01110000101010111, 0, 5, 10, 0 }, // vmsub_d
  { 0b01110000101010101, 0, 5, 10, 0 }, // vmsub_h
  { 0b01110000101010110, 0, 5, 10, 0 }, // vmsub_w
  { 0b01110000100001100, 0, 5, 10, 0 }, // vmuh_b
  { 0b01110000100010000, 0, 5, 10, 0 }, // vmuh_bu
  { 0b01110000100001111, 0, 5, 10, 0 }, // vmuh_d
  { 0b01110000100010011, 0, 5, 10, 0 }, // vmuh_du
  { 0b01110000100001101, 0, 5, 10, 0 }, // vmuh_h
  { 0b01110000100010001, 0, 5, 10, 0 }, // vmuh_hu
  { 0b01110000100001110, 0, 5, 10, 0 }, // vmuh_w
  { 0b01110000100010010, 0, 5, 10, 0 }, // vmuh_wu
  { 0b01110000100001000, 0, 5, 10, 0 }, // vmul_b
  { 0b01110000100001011, 0, 5, 10, 0 }, // vmul_d
  { 0b01110000100001001, 0, 5, 10, 0 }, // vmul_h
  { 0b01110000100001010, 0, 5, 10, 0 }, // vmul_w
  { 0b01110000100100010, 0, 5, 10, 0 }, // vmulwev_d_w
  { 0b01110000100110010, 0, 5, 10, 0 }, // vmulwev_d_wu
  { 0b01110000101000010, 0, 5, 10, 0 }, // vmulwev_d_wu_w
  { 0b01110000100100000, 0, 5, 10, 0 }, // vmulwev_h_b
  { 0b01110000100110000, 0, 5, 10, 0 }, // vmulwev_h_bu
  { 0b01110000101000000, 0, 5, 10, 0 }, // vmulwev_h_bu_b
  { 0b01110000100100011, 0, 5, 10, 0 }, // vmulwev_q_d
  { 0b01110000100110011, 0, 5, 10, 0 }, // vmulwev_q_du
  { 0b01110000101000011, 0, 5, 10, 0 }, // vmulwev_q_du_d
  { 0b01110000100100001, 0, 5, 10, 0 }, // vmulwev_w_h
  { 0b01110000100110001, 0, 5, 10, 0 }, // vmulwev_w_hu
  { 0b01110000101000001, 0, 5, 10, 0 }, // vmulwev_w_hu_h
  { 0b01110000100100110, 0, 5, 10, 0 }, // vmulwod_d_w
  { 0b01110000100110110, 0, 5, 10, 0 }, // vmulwod_d_wu
  { 0b01110000101000110, 0, 5, 10, 0 }, // vmulwod_d_wu_w
  { 0b01110000100100100, 0, 5, 10, 0 }, // vmulwod_h_b
  { 0b01110000100110100, 0, 5, 10, 0 }, // vmulwod_h_bu
  { 0b01110000101000100, 0, 5, 10, 0 }, // vmulwod_h_bu_b
  { 0b01110000100100111, 0, 5, 10, 0 }, // vmulwod_q_d
  { 0b01110000100110111, 0, 5, 10, 0 }, // vmulwod_q_du
  { 0b01110000101000111, 0, 5, 10, 0 }, // vmulwod_q_du_d
  { 0b01110000100100101, 0, 5, 10, 0 }, // vmulwod_w_h
  { 0b01110000100110101, 0, 5, 10, 0 }, // vmulwod_w_hu
  { 0b01110000101000101, 0, 5, 10, 0 }, // vmulwod_w_hu_h
  { 0b01110001001001111, 0, 5, 10, 0 }, // vnor_v
  { 0b01110001001001101, 0, 5, 10, 0 }, // vor_v
  { 0b01110001001010001, 0, 5, 10, 0 }, // vorn_v
  { 0b01110001000101100, 0, 5, 10, 0 }, // vpackev_b
  { 0b01110001000101111, 0, 5, 10, 0 }, // vpackev_d
  { 0b01110001000101101, 0, 5, 10, 0 }, // vpackev_h
  { 0b01110001000101110, 0, 5, 10, 0 }, // vpackev_w
  { 0b01110001000110000, 0, 5, 10, 0 }, // vpackod_b
  { 0b01110001000110011, 0, 5, 10, 0 }, // vpackod_d
  { 0b01110001000110001, 0, 5, 10, 0 }, // vpackod_h
  { 0b01110001000110010, 0, 5, 10, 0 }, // vpackod_w
  { 0b01110001000111100, 0, 5, 10, 0 }, // vpickev_b
  { 0b01110001000111111, 0, 5, 10, 0 }, // vpickev_d
  { 0b01110001000111101, 0, 5, 10, 0 }, // vpickev_h
  { 0b01110001000111110, 0, 5, 10, 0 }, // vpickev_w
  { 0b01110001001000000, 0, 5, 10, 0 }, // vpickod_b
  { 0b01110001001000011, 0, 5, 10, 0 }, // vpickod_d
  { 0b01110001001000001, 0, 5, 10, 0 }, // vpickod_h
  { 0b01110001001000010, 0, 5, 10, 0 }, // vpickod_w
  { 0b01110000111011100, 0, 5, 10, 0 }, // vrotr_b
  { 0b01110000111011111, 0, 5, 10, 0 }, // vrotr_d
  { 0b01110000111011101, 0, 5, 10, 0 }, // vrotr_h
  { 0b01110000111011110, 0, 5, 10, 0 }, // vrotr_w
  { 0b01110000010001100, 0, 5, 10, 0 }, // vsadd_b
  { 0b01110000010010100, 0, 5, 10, 0 }, // vsadd_bu
  { 0b01110000010001111, 0, 5, 10, 0 }, // vsadd_d
  { 0b01110000010010111, 0, 5, 10, 0 }, // vsadd_du
  { 0b01110000010001101, 0, 5, 10, 0 }, // vsadd_h
  { 0b01110000010010101, 0, 5, 10, 0 }, // vsadd_hu
  { 0b01110000010001110, 0, 5, 10, 0 }, // vsadd_w
  { 0b01110000010010110, 0, 5, 10, 0 }, // vsadd_wu
  { 0b01110000000000000, 0, 5, 10, 0 }, // vseq_b
  { 0b01110000000000011, 0, 5, 10, 0 }, // vseq_d
  { 0b01110000000000001, 0, 5, 10, 0 }, // vseq_h
  { 0b01110000000000010, 0, 5, 10, 0 }, // vseq_w
  { 0b01110001011110111, 0, 5, 10, 0 }, // vshuf_d
  { 0b01110001011110101, 0, 5, 10, 0 }, // vshuf_h
  { 0b01110001011110110, 0, 5, 10, 0 }, // vshuf_w
  { 0b01110001001011100, 0, 5, 10, 0 }, // vsigncov_b
  { 0b01110001001011111, 0, 5, 10, 0 }, // vsigncov_d
  { 0b01110001001011101, 0, 5, 10, 0 }, // vsigncov_h
  { 0b01110001001011110, 0, 5, 10, 0 }, // vsigncov_w
  { 0b01110000000000100, 0, 5, 10, 0 }, // vsle_b
  { 0b01110000000001000, 0, 5, 10, 0 }, // vsle_bu
  { 0b01110000000000111, 0, 5, 10, 0 }, // vsle_d
  { 0b01110000000001011, 0, 5, 10, 0 }, // vsle_du
  { 0b01110000000000101, 0, 5, 10, 0 }, // vsle_h
  { 0b01110000000001001, 0, 5, 10, 0 }, // vsle_hu
  { 0b01110000000000110, 0, 5, 10, 0 }, // vsle_w
  { 0b01110000000001010, 0, 5, 10, 0 }, // vsle_wu
  { 0b01110000111010000, 0, 5, 10, 0 }, // vsll_b
  { 0b01110000111010011, 0, 5, 10, 0 }, // vsll_d
  { 0b01110000111010001, 0, 5, 10, 0 }, // vsll_h
  { 0b01110000111010010, 0, 5, 10, 0 }, // vsll_w
  { 0b01110000000001100, 0, 5, 10, 0 }, // vslt_b
  { 0b01110000000010000, 0, 5, 10, 0 }, // vslt_bu
  { 0b01110000000001111, 0, 5, 10, 0 }, // vslt_d
  { 0b01110000000010011, 0, 5, 10, 0 }, // vslt_du
  { 0b01110000000001101, 0, 5, 10, 0 }, // vslt_h
  { 0b01110000000010001, 0, 5, 10, 0 }, // vslt_hu
  { 0b01110000000001110, 0, 5, 10, 0 }, // vslt_w
  { 0b01110000000010010, 0, 5, 10, 0 }, // vslt_wu
  { 0b01110000111011000, 0, 5, 10, 0 }, // vsra_b
  { 0b01110000111011011, 0, 5, 10, 0 }, // vsra_d
  { 0b01110000111011001, 0, 5, 10, 0 }, // vsra_h
  { 0b01110000111011010, 0, 5, 10, 0 }, // vsra_w
  { 0b01110000111101101, 0, 5, 10, 0 }, // vsran_b_h
  { 0b01110000111101110, 0, 5, 10, 0 }, // vsran_h_w
  { 0b01110000111101111, 0, 5, 10, 0 }, // vsran_w_d
  { 0b01110000111100100, 0, 5, 10, 0 }, // vsrar_b
  { 0b01110000111100111, 0, 5, 10, 0 }, // vsrar_d
  { 0b01110000111100101, 0, 5, 10, 0 }, // vsrar_h
  { 0b01110000111100110, 0, 5, 10, 0 }, // vsrar_w
  { 0b01110000111110101, 0, 5, 10, 0 }, // vsrarn_b_h
  { 0b01110000111110110, 0, 5, 10, 0 }, // vsrarn_h_w
  { 0b01110000111110111, 0, 5, 10, 0 }, // vsrarn_w_d
  { 0b01110000111010100, 0, 5, 10, 0 }, // vsrl_b
  { 0b01110000111010111, 0, 5, 10, 0 }, // vsrl_d
  { 0b01110000111010101, 0, 5, 10, 0 }, // vsrl_h
  { 0b01110000111010110, 0, 5, 10, 0 }, // vsrl_w
  { 0b01110000111101001, 0, 5, 10, 0 }, // vsrln_b_h
  { 0b01110000111101010, 0, 5, 10, 0 }, // vsrln_h_w
  { 0b01110000111101011, 0, 5, 10, 0 }, // vsrln_w_d
  { 0b01110000111100000, 0, 5, 10, 0 }, // vsrlr_b
  { 0b01110000111100011, 0, 5, 10, 0 }, // vsrlr_d
  { 0b01110000111100001, 0, 5, 10, 0 }, // vsrlr_h
  { 0b01110000111100010, 0, 5, 10, 0 }, // vsrlr_w
  { 0b01110000111110001, 0, 5, 10, 0 }, // vsrlrn_b_h
  { 0b01110000111110010, 0, 5, 10, 0 }, // vsrlrn_h_w
  { 0b01110000111110011, 0, 5, 10, 0 }, // vsrlrn_w_d
  { 0b01110000111111101, 0, 5, 10, 0 }, // vssran_b_h
  { 0b01110001000001101, 0, 5, 10, 0 }, // vssran_bu_h
  { 0b01110000111111110, 0, 5, 10, 0 }, // vssran_h_w
  { 0b01110001000001110, 0, 5, 10, 0 }, // vssran_hu_w
  { 0b01110000111111111, 0, 5, 10, 0 }, // vssran_w_d
  { 0b01110001000001111, 0, 5, 10, 0 }, // vssran_wu_d
  { 0b01110001000000101, 0, 5, 10, 0 }, // vssrarn_b_h
  { 0b01110001000010101, 0, 5, 10, 0 }, // vssrarn_bu_h
  { 0b01110001000000110, 0, 5, 10, 0 }, // vssrarn_h_w
  { 0b01110001000010110, 0, 5, 10, 0 }, // vssrarn_hu_w
  { 0b01110001000000111, 0, 5, 10, 0 }, // vssrarn_w_d
  { 0b01110001000010111, 0, 5, 10, 0 }, // vssrarn_wu_d
  { 0b01110000111111001, 0, 5, 10, 0 }, // vssrln_b_h
  { 0b01110001000001001, 0, 5, 10, 0 }, // vssrln_bu_h
  { 0b01110000111111010, 0, 5, 10, 0 }, // vssrln_h_w
  { 0b01110001000001010, 0, 5, 10, 0 }, // vssrln_hu_w
  { 0b01110000111111011, 0, 5, 10, 0 }, // vssrln_w_d
  { 0b01110001000001011, 0, 5, 10, 0 }, // vssrln_wu_d
  { 0b01110001000000001, 0, 5, 10, 0 }, // vssrlrn_b_h
  { 0b01110001000010001, 0, 5, 10, 0 }, // vssrlrn_bu_h
  { 0b01110001000000010, 0, 5, 10, 0 }, // vssrlrn_h_w
  { 0b01110001000010010, 0, 5, 10, 0 }, // vssrlrn_hu_w
  { 0b01110001000000011, 0, 5, 10, 0 }, // vssrlrn_w_d
  { 0b01110001000010011, 0, 5, 10, 0 }, // vssrlrn_wu_d
  { 0b01110000010010000, 0, 5, 10, 0 }, // vssub_b
  { 0b01110000010011000, 0, 5, 10, 0 }, // vssub_bu
  { 0b01110000010010011, 0, 5, 10, 0 }, // vssub_d
  { 0b01110000010011011, 0, 5, 10, 0 }, // vssub_du
  { 0b01110000010010001, 0, 5, 10, 0 }, // vssub_h
  { 0b01110000010011001, 0, 5, 10, 0 }, // vssub_hu
  { 0b01110000010010010, 0, 5, 10, 0 }, // vssub_w
  { 0b01110000010011010, 0, 5, 10, 0 }, // vssub_wu
  { 0b01110000000011000, 0, 5, 10, 0 }, // vsub_b
  { 0b01110000000011011, 0, 5, 10, 0 }, // vsub_d
  { 0b01110000000011001, 0, 5, 10, 0 }, // vsub_h
  { 0b01110001001011011, 0, 5, 10, 0 }, // vsub_q
  { 0b01110000000011010, 0, 5, 10, 0 }, // vsub_w
  { 0b01110000001000010, 0, 5, 10, 0 }, // vsubwev_d_w
  { 0b01110000001100010, 0, 5, 10, 0 }, // vsubwev_d_wu
  { 0b01110000001000000, 0, 5, 10, 0 }, // vsubwev_h_b
  { 0b01110000001100000, 0, 5, 10, 0 }, // vsubwev_h_bu
  { 0b01110000001000011, 0, 5, 10, 0 }, // vsubwev_q_d
  { 0b01110000001100011, 0, 5, 10, 0 }, // vsubwev_q_du
  { 0b01110000001000001, 0, 5, 10, 0 }, // vsubwev_w_h
  { 0b01110000001100001, 0, 5, 10, 0 }, // vsubwev_w_hu
  { 0b01110000001001010, 0, 5, 10, 0 }, // vsubwod_d_w
  { 0b01110000001101010, 0, 5, 10, 0 }, // vsubwod_d_wu
  { 0b01110000001001000, 0, 5, 10, 0 }, // vsubwod_h_b
  { 0b01110000001101000, 0, 5, 10, 0 }, // vsubwod_h_bu
  { 0b01110000001001011, 0, 5, 10, 0 }, // vsubwod_q_d
  { 0b01110000001101011, 0, 5, 10, 0 }, // vsubwod_q_du
  { 0b01110000001001001, 0, 5, 10, 0 }, // vsubwod_w_h
  { 0b01110000001101001, 0, 5, 10, 0 }, // vsubwod_w_hu
  { 0b01110001001001110, 0, 5, 10, 0 }, // vxor_v
  { 0b0000110001010, 0, 5, 10, 0 }, // vfcmp_caf_s
  { 0b0000110001011000, 0, 5, 10, 0 }, // vfcmp_cun_s
  { 0b000011000101100, 0, 5, 10, 0 }, // vfcmp_ceq_s
  { 0b0000110001011100, 0, 5, 10, 0 }, // vfcmp_cueq_s
  { 0b00001100010110, 0, 5, 10, 0 }, // vfcmp_clt_s
  { 0b0000110001011010, 0, 5, 10, 0 }, // vfcmp_cult_s
  { 0b000011000101110, 0, 5, 10, 0 }, // vfcmp_cle_s
  { 0b0000110001011110, 0, 5, 10, 0 }, // vfcmp_cule_s
  { 0b00001100010110000, 0, 5, 10, 0 }, // vfcmp_cne_s
  { 0b00001100010110100, 0, 5, 10, 0 }, // vfcmp_cor_s
  { 0b00001100010111000, 0, 5, 10, 0 }, // vfcmp_cune_s
  { 0b0000110001011, 0, 5, 10, 0 }, // vfcmp_saf_s
  { 0b0000110001011001, 0, 5, 10, 0 }, // vfcmp_sun_s
  { 0b000011000101101, 0, 5, 10, 0 }, // vfcmp_seq_s
  { 0b0000110001011101, 0, 5, 10, 0 }, // vfcmp_sueq_s
  { 0b00001100010111, 0, 5, 10, 0 }, // vfcmp_slt_s
  { 0b0000110001011011, 0, 5, 10, 0 }, // vfcmp_sult_s
  { 0b000011000101111, 0, 5, 10, 0 }, // vfcmp_sle_s
  { 0b0000110001011111, 0, 5, 10, 0 }, // vfcmp_sule_s
  { 0b00001100010110001, 0, 5, 10, 0 }, // vfcmp_sne_s
  { 0b00001100010110101, 0, 5, 10, 0 }, // vfcmp_sor_s
  { 0b00001100010111001, 0, 5, 10, 0 }, // vfcmp_sune_s
  { 0b0000110001100, 0, 5, 10, 0 }, // vfcmp_caf_d
  { 0b0000110001101000, 0, 5, 10, 0 }, // vfcmp_cun_d
  { 0b000011000110100, 0, 5, 10, 0 }, // vfcmp_ceq_d
  { 0b0000110001101100, 0, 5, 10, 0 }, // vfcmp_cueq_d
  { 0b00001100011010, 0, 5, 10, 0 }, // vfcmp_clt_d
  { 0b0000110001101010, 0, 5, 10, 0 }, // vfcmp_cult_d
  { 0b000011000110110, 0, 5, 10, 0 }, // vfcmp_cle_d
  { 0b0000110001101110, 0, 5, 10, 0 }, // vfcmp_cule_d
  { 0b00001100011010000, 0, 5, 10, 0 }, // vfcmp_cne_d
  { 0b00001100011010100, 0, 5, 10, 0 }, // vfcmp_cor_d
  { 0b00001100011011000, 0, 5, 10, 0 }, // vfcmp_cune_d
  { 0b0000110001101, 0, 5, 10, 0 }, // vfcmp_saf_d
  { 0b0000110001101001, 0, 5, 10, 0 }, // vfcmp_sun_d
  { 0b000011000110101, 0, 5, 10, 0 }, // vfcmp_seq_d
  { 0b0000110001101101, 0, 5, 10, 0 }, // vfcmp_sueq_d
  { 0b00001100011011, 0, 5, 10, 0 }, // vfcmp_slt_d
  { 0b0000110001101011, 0, 5, 10, 0 }, // vfcmp_sult_d
  { 0b000011000110111, 0, 5, 10, 0 }, // vfcmp_sle_d
  { 0b0000110001101111, 0, 5, 10, 0 }, // vfcmp_sule_d
  { 0b00001100011010001, 0, 5, 10, 0 }, // vfcmp_sne_d
  { 0b00001100011010101, 0, 5, 10, 0 }, // vfcmp_sor_d
  { 0b00001100011011001, 0, 5, 10, 0 }  // vfcmp_sune_d
};

const LsxVVVV lsxVVVV[10] = {
  { 0b000011010001, 0, 5, 10, 15, 0 }, // vbitsel_v
  { 0b000010010010, 0, 5, 10, 15, 0 }, // vfmadd_d
  { 0b000010010001, 0, 5, 10, 15, 0 }, // vfmadd_s
  { 0b000010010110, 0, 5, 10, 15, 0 }, // vfmsub_d
  { 0b000010010101, 0, 5, 10, 15, 0 }, // vfmsub_s
  { 0b000010011010, 0, 5, 10, 15, 0 }, // vfnmadd_d
  { 0b000010011001, 0, 5, 10, 15, 0 }, // vfnmadd_s
  { 0b000010011110, 0, 5, 10, 15, 0 }, // vfnmsub_d
  { 0b000010011101, 0, 5, 10, 15, 0 }, // vfnmsub_s
  { 0b000011010101, 0, 5, 10, 15, 0 }  // vshuf_b
};

const SimdLdst simdLdst[16] = {
  { 0b0010110000,        22, 12 }, // vld
  { 0b0011000010,        22, 12 }, // vldrepl_b
  { 0b0011000000010,     19, 9  }, // vldrepl_d
  { 0b00110000010,       21, 11 }, // vldrepl_h
  { 0b001100000010,      20, 10 }, // vldrepl_w
  { 0b00111000010000000, 15, 0  }, // vldx
  { 0b0010110001,        22, 12 }, // vst
  { 0b00111000010001000, 15, 0  }, // vstx
  { 0b0010110010,       22, 12 }, // xvld
  { 0b0011001010,       22, 12 }, // xvldrepl_b
  { 0b0011001000010,    19, 9  }, // xvldrepl_d
  { 0b00110010010,      21, 11 }, // xvldrepl_h
  { 0b001100100010,     20, 10 }, // xvldrepl_w
  { 0b00111000010010000,15, 0  }, // xvldx
  { 0b0010110011,        22, 12 }, // xvst
  { 0b00111000010011000, 15, 0  }  // xvstx
};
// ----------------------------------------------------------------------------
// ${EncodingData:End}

} // {EncodingData}
} // {InstDB}

/*
// ${CommonData:Begin}
// ${CommonData:End}
*/

// LaUtil - Id <-> Name
// =====================

#ifndef ASMJIT_NO_TEXT
// ${NameData:Begin}
// ------------------- Automatically generated, do not edit -------------------
const InstNameIndex InstDB::instNameIndex = {{
  { Inst::kIdAdd_d        , Inst::kIdAsrtle_d      + 1 },
  { Inst::kIdB            , Inst::kIdBytepick_w    + 1 },
  { Inst::kIdCacop        , Inst::kIdCtz_w         + 1 },
  { Inst::kIdDbar         , Inst::kIdDiv_wu        + 1 },
  { Inst::kIdErtn         , Inst::kIdExt_w_h       + 1 },
  { Inst::kIdFabs_d       , Inst::kIdFtintrz_w_s   + 1 },
  { Inst::kIdNone         , Inst::kIdNone          + 1 },
  { Inst::kIdNone         , Inst::kIdNone          + 1 },
  { Inst::kIdIbar         , Inst::kIdIocsrwr_w     + 1 },
  { Inst::kIdJirl         , Inst::kIdJirl          + 1 },
  { Inst::kIdNone         , Inst::kIdNone          + 1 },
  { Inst::kIdLd_b         , Inst::kIdLu52i_d       + 1 },
  { Inst::kIdMaskeqz      , Inst::kIdMove          + 1 },
  { Inst::kIdNor          , Inst::kIdNor           + 1 },
  { Inst::kIdOr_          , Inst::kIdOrn           + 1 },
  { Inst::kIdPcaddi       , Inst::kIdPreld         + 1 },
  { Inst::kIdNone         , Inst::kIdNone          + 1 },
  { Inst::kIdRdtime_d     , Inst::kIdRotri_w       + 1 },
  { Inst::kIdSc_d         , Inst::kIdSyscall       + 1 },
  { Inst::kIdTlbclr       , Inst::kIdTlbwr         + 1 },
  { Inst::kIdNone         , Inst::kIdNone          + 1 },
  { Inst::kIdVabsd_b      , Inst::kIdVfcmp_sune_d  + 1 },
  { Inst::kIdNone         , Inst::kIdNone          + 1 },
  { Inst::kIdXor_         , Inst::kIdXvfcmp_sune_d + 1 },
  { Inst::kIdNone         , Inst::kIdNone          + 1 },
  { Inst::kIdNone         , Inst::kIdNone          + 1 }
}, uint16_t(16)};

const char InstDB::_instNameStringTable[] =
  "xvmaddwev_d_wu_h_bu_bq_du_dw_hu_hxvmaddwod_d_wuxvaddwev_d_wuxvaddwod_d_wuxvftint"
  "rneh_l_sl_l_sxvmulwev_d_wuxvmulwod_d_wumh_l_sml_l_s_l_d_w_d_w_sph_l_spl_l_sz_lu_"
  "dz_wu_szh_l_szl_l_sxvpickve2grxvrepl128veixvsllwil_duhu_buwu_huxvssrarni_bdu_qhu"
  "_wwu_dxvssrlrni_bvext2xv_du_wuwu_bum_l_dm_w_dm_w_sp_l_dp_w_dp_w_sz_l_dz_w_dz_w_s"
  "xvhaddwxvhaddw_qu_duxvhsubwxvhsubw_xvreplgr2vrxvsetallnezxvsetanyeqzxvssrani_b_b"
  "u_h_hu_w_wu_dd_qh_wxvssrlni_du_qxvsubwxvsubwev_q_duxvsubwod_w_huxvfcmp_cueqlt_sl"
  "e_sne_sxvfcmp_sueqlt_dle_dne_dxvexthxvexth_xvextlxvffint_ds_wuh_d_wxvfrintrxvins"
  "gr2vrxvsrarni_bxvsrlrni_b_b_h_h_waf_sn_seq_sor_saf_dn_deq_dor_dammax_dbammin_dba"
  "mswap_dbfcopysigftintrmftintrpftintrzmovfrh2gmovgr2frw_bxvbitclri_hxvbitrevi_wxv"
  "bitselti_bti_dti_hti_wxvextrinsxvfcvth_ds_hxvfcvtl_dm_dm_sz_dz_sxvfscalebxvreplv"
  "e0xvsigncovxvsrani_d_qxvsrlniamadd_dbamand_dbamxor_dbbstrpickbytepickcrcc_w_bmov"
  "fcsrmovfr2gt_bt_ht_wxvfclass_h_s_s_dxvfnxvfnmaddxvfnmsubxvfrecixvfrsqrxvfrstpxvi"
  "nsve0xvldreplxvmepatmsxvmskgezxvmskltzxvpackevod_bod_hev_bev_hev_wxvshuf4addu16a"
  "mor_db_4b_8bbstrincrc_w_biocsrriocsrwrpcaddu128ipcalau12rdtimel_wxvabsdxvaddixva"
  "vgrxvflogbxvfmaddxvfmaxaxvfminaxvfmsubxvfsqrxvhselxvmaxixvminixvmsknzxvpermi_qxv"
  "rotrxvsaddxvsleixvsltixvssuxvssubxvstelmxvsubiasrtgfcvt_dmovcftlbflua_ba_da_ha_w"
  "xvandxvdivxvfaddxvfdiv_sxvfmuxvfsubxvilvl_bxvmodxvmsub_dxvmuhxvnorxvpcnxvsatxvse"
  "qxvxoralslcsrxchfldgfstgldptldptrlu12lu32lu52maskmulhrevb_24hrevh_2stptstptrsysc"
  "tlbfitlbsrxvcloxvclzxvnegxvorbreakfabfldxfmofnegfstxldlestlecto";


const uint32_t InstDB::_instNameIndexTable[] = {
  0x80000000, // Small ''.
  0x0000533F, // Large 'add_d'.
  0x1006433F, // Large 'add_|w'.
  0x20094459, // Large 'addi|_d'.
  0x200B4459, // Large 'addi|_w'.
  0x31AA6409, // Large 'addu16|i_d'.
  0x20094556, // Large 'alsl|_d'.
  0x200B4556, // Large 'alsl|_w'.
  0x300B4556, // Large 'alsl|_wu'.
  0x0000733D, // Large 'amadd_d'.
  0x2009833D, // Large 'amadd_db|_d'.
  0x200B833D, // Large 'amadd_db|_w'.
  0x1006633D, // Large 'amadd_|w'.
  0x00007345, // Large 'amand_d'.
  0x20098345, // Large 'amand_db|_d'.
  0x200B8345, // Large 'amand_db|_w'.
  0x10066345, // Large 'amand_|w'.
  0x0000726F, // Large 'ammax_d'.
  0x2009826F, // Large 'ammax_db|_d'.
  0x3016826F, // Large 'ammax_db|_du'.
  0x200B826F, // Large 'ammax_db|_w'.
  0x300B826F, // Large 'ammax_db|_wu'.
  0x100D726F, // Large 'ammax_d|u'.
  0x1006626F, // Large 'ammax_|w'.
  0x200C626F, // Large 'ammax_|wu'.
  0x00007277, // Large 'ammin_d'.
  0x20098277, // Large 'ammin_db|_d'.
  0x30168277, // Large 'ammin_db|_du'.
  0x200B8277, // Large 'ammin_db|_w'.
  0x300B8277, // Large 'ammin_db|_wu'.
  0x100D7277, // Large 'ammin_d|u'.
  0x10066277, // Large 'ammin_|w'.
  0x200C6277, // Large 'ammin_|wu'.
  0x0000640F, // Large 'amor_d'.
  0x2009740F, // Large 'amor_db|_d'.
  0x200B740F, // Large 'amor_db|_w'.
  0x1006540F, // Large 'amor_|w'.
  0x0000827F, // Large 'amswap_d'.
  0x2009927F, // Large 'amswap_db|_d'.
  0x200B927F, // Large 'amswap_db|_w'.
  0x1006727F, // Large 'amswap_|w'.
  0x0000734D, // Large 'amxor_d'.
  0x2009834D, // Large 'amxor_db|_d'.
  0x200B834D, // Large 'amxor_db|_w'.
  0x1006634D, // Large 'amxor_|w'.
  0x00004347, // Large 'and_'.
  0x800491C1, // Small 'andi'.
  0x800711C1, // Small 'andn'.
  0x31F354DE, // Large 'asrtg|t_d'.
  0x41F644DE, // Large 'asrt|le_d'.
  0x80000002, // Small 'b'.
  0x81A89462, // Small 'bceqz'.
  0x81A2B862, // Small 'bcnez'.
  0x800044A2, // Small 'beq'.
  0x800014E2, // Small 'bge'.
  0x800A94E2, // Small 'bgeu'.
  0x341662C5, // Large 'bitrev|_4b'.
  0x341962C5, // Large 'bitrev|_8b'.
  0x200962C5, // Large 'bitrev|_d'.
  0x200B62C5, // Large 'bitrev|_w'.
  0x80000182, // Small 'bl'.
  0x80005182, // Small 'blt'.
  0x800AD182, // Small 'bltu'.
  0x800015C2, // Small 'bne'.
  0x100955BD, // Large 'break|_'.
  0x3391641C, // Large 'bstrin|s_d'.
  0x321A641C, // Large 'bstrin|s_w'.
  0x20098355, // Large 'bstrpick|_d'.
  0x200B8355, // Large 'bstrpick|_w'.
  0x2009835D, // Large 'bytepick|_d'.
  0x200B835D, // Large 'bytepick|_w'.
  0x81078C23, // Small 'cacop'.
  0x200935AC, // Large 'clo|_d'.
  0x200B35AC, // Large 'clo|_w'.
  0x200935B1, // Large 'clz|_d'.
  0x200B35B1, // Large 'clz|_w'.
  0x8E61D603, // Small 'cpucfg'.
  0x200B7422, // Large 'crc_w_b|_w'.
  0x300A6422, // Large 'crc_w_|d_w'.
  0x31A06422, // Large 'crc_w_|h_w'.
  0x31976422, // Large 'crc_w_|w_w'.
  0x200B8365, // Large 'crcc_w_b|_w'.
  0x300A7365, // Large 'crcc_w_|d_w'.
  0x31A07365, // Large 'crcc_w_|h_w'.
  0x31977365, // Large 'crcc_w_|w_w'.
  0x80494A63, // Small 'csrrd'.
  0x812BCA63, // Small 'csrwr'.
  0x10BC655A, // Large 'csrxch|g'.
  0x200935DC, // Large 'cto|_d'.
  0x200B35DC, // Large 'cto|_w'.
  0x330A259F, // Large 'ct|z_d'.
  0x30A1259F, // Large 'ct|z_w'.
  0x80090444, // Small 'dbar'.
  0x80060C44, // Small 'dbcl'.
  0x10044513, // Large 'div_|d'.
  0x20174513, // Large 'div_|du'.
  0x10064513, // Large 'div_|w'.
  0x200C4513, // Large 'div_|wu'.
  0x80075245, // Small 'ertn'.
  0x43693102, // Large 'ext|_w_b'.
  0x41CB3102, // Large 'ext|_w_h'.
  0x339135C2, // Large 'fab|s_d'.
  0x338F35C2, // Large 'fab|s_s'.
  0x2009450C, // Large 'fadd|_d'.
  0x2056450C, // Large 'fadd|_s'.
  0x10047386, // Large 'fclass_|d'.
  0x10577386, // Large 'fclass_|s'.
  0x425161D2, // Large 'fcmp_c|af_s'.
  0x425861D2, // Large 'fcmp_c|eq_s'.
  0x41F661D2, // Large 'fcmp_c|le_d'.
  0x41F261D2, // Large 'fcmp_c|lt_d'.
  0x41FA61D2, // Large 'fcmp_c|ne_d'.
  0x425C61D2, // Large 'fcmp_c|or_s'.
  0x205691D2, // Large 'fcmp_cueq|_s'.
  0x41DF71D2, // Large 'fcmp_cu|le_s'.
  0x41DB71D2, // Large 'fcmp_cu|lt_s'.
  0x326471D2, // Large 'fcmp_cu|n_d'.
  0x41FA71D2, // Large 'fcmp_cu|ne_d'.
  0x425161E9, // Large 'fcmp_s|af_s'.
  0x425861E9, // Large 'fcmp_s|eq_s'.
  0x41DF61E9, // Large 'fcmp_s|le_s'.
  0x41DB61E9, // Large 'fcmp_s|lt_s'.
  0x41E361E9, // Large 'fcmp_s|ne_s'.
  0x426B61E9, // Large 'fcmp_s|or_d'.
  0x200991E9, // Large 'fcmp_sueq|_d'.
  0x41F671E9, // Large 'fcmp_su|le_d'.
  0x41F271E9, // Large 'fcmp_su|lt_d'.
  0x326471E9, // Large 'fcmp_su|n_d'.
  0x41E371E9, // Large 'fcmp_su|ne_s'.
  0x32648288, // Large 'fcopysig|n_d'.
  0x32558288, // Large 'fcopysig|n_s'.
  0x205664E3, // Large 'fcvt_d|_s'.
  0x339154E3, // Large 'fcvt_|s_d'.
  0x10045512, // Large 'fdiv_|d'.
  0x00006512, // Large 'fdiv_s'.
  0x20547213, // Large 'ffint_d|_l'.
  0x200B7213, // Large 'ffint_d|_w'.
  0x30826213, // Large 'ffint_|s_l'.
  0x321A6213, // Large 'ffint_|s_w'.
  0x20093560, // Large 'fld|_d'.
  0x20563560, // Large 'fld|_s'.
  0x31F34560, // Large 'fldg|t_d'.
  0x31DC4560, // Large 'fldg|t_s'.
  0x41F63560, // Large 'fld|le_d'.
  0x41DF3560, // Large 'fld|le_s'.
  0x32733560, // Large 'fld|x_d'.
  0x205645C5, // Large 'fldx|_s'.
  0x20095465, // Large 'flogb|_d'.
  0x20565465, // Large 'flogb|_s'.
  0x2009546C, // Large 'fmadd|_d'.
  0x2056546C, // Large 'fmadd|_s'.
  0x20094473, // Large 'fmax|_d'.
  0x20564473, // Large 'fmax|_s'.
  0x20095473, // Large 'fmaxa|_d'.
  0x20565473, // Large 'fmaxa|_s'.
  0x2009447A, // Large 'fmin|_d'.
  0x2056447A, // Large 'fmin|_s'.
  0x2009547A, // Large 'fmina|_d'.
  0x2056547A, // Large 'fmina|_s'.
  0x300835C9, // Large 'fmo|v_d'.
  0x351535C9, // Large 'fmo|v_s'.
  0x3084351A, // Large 'fmu|l_d'.
  0x3055351A, // Large 'fmu|l_s'.
  0x200945CC, // Large 'fneg|_d'.
  0x205645CC, // Large 'fneg|_s'.
  0x328453AA, // Large 'freci|p_d'.
  0x31EC53AA, // Large 'freci|p_s'.
  0x20095225, // Large 'frint|_d'.
  0x20565225, // Large 'frint|_s'.
  0x31F353B1, // Large 'frsqr|t_d'.
  0x31DC53B1, // Large 'frsqr|t_s'.
  0x20097312, // Large 'fscaleb|_d'.
  0x20567312, // Large 'fscaleb|_s'.
  0x80061666, // Small 'fsel'.
  0x31F34488, // Large 'fsqr|t_d'.
  0x31DC4488, // Large 'fsqr|t_s'.
  0x20093564, // Large 'fst|_d'.
  0x20563564, // Large 'fst|_s'.
  0x31F34564, // Large 'fstg|t_d'.
  0x31DC4564, // Large 'fstg|t_s'.
  0x41F63564, // Large 'fst|le_d'.
  0x41DF3564, // Large 'fst|le_s'.
  0x32733564, // Large 'fst|x_d'.
  0x205645D0, // Large 'fstx|_s'.
  0x2009451F, // Large 'fsub|_d'.
  0x2056451F, // Large 'fsub|_s'.
  0x4083504B, // Large 'ftint|_l_d'.
  0x4054504B, // Large 'ftint|_l_s'.
  0x4087504B, // Large 'ftint|_w_d'.
  0x408B504B, // Large 'ftint|_w_s'.
  0x5113604B, // Large 'ftintr|m_l_d'.
  0x40547290, // Large 'ftintrm|_l_s'.
  0x40877290, // Large 'ftintrm|_w_d'.
  0x408B7290, // Large 'ftintrm|_w_s'.
  0x4083804B, // Large 'ftintrne|_l_d'.
  0x4054804B, // Large 'ftintrne|_l_s'.
  0x4087804B, // Large 'ftintrne|_w_d'.
  0x408B804B, // Large 'ftintrne|_w_s'.
  0x5122604B, // Large 'ftintr|p_l_d'.
  0x40547297, // Large 'ftintrp|_l_s'.
  0x40877297, // Large 'ftintrp|_w_d'.
  0x408B7297, // Large 'ftintrp|_w_s'.
  0x5131604B, // Large 'ftintr|z_l_d'.
  0x4054729E, // Large 'ftintrz|_l_s'.
  0x4087729E, // Large 'ftintrz|_w_d'.
  0x408B729E, // Large 'ftintrz|_w_s'.
  0x80090449, // Small 'ibar'.
  0x8002B089, // Small 'idle'.
  0x84CA59C9, // Small 'invtlb'.
  0x33EF6429, // Large 'iocsrr|d_b'.
  0x30296429, // Large 'iocsrr|d_d'.
  0x33F36429, // Large 'iocsrr|d_h'.
  0x300A6429, // Large 'iocsrr|d_w'.
  0x2010742F, // Large 'iocsrwr|_b'.
  0x2009742F, // Large 'iocsrwr|_d'.
  0x200E742F, // Large 'iocsrwr|_h'.
  0x200B742F, // Large 'iocsrwr|_w'.
  0x8006492A, // Small 'jirl'.
  0x201023C7, // Large 'ld|_b'.
  0x301023C7, // Large 'ld|_bu'.
  0x200923C7, // Large 'ld|_d'.
  0x200E23C7, // Large 'ld|_h'.
  0x301C23C7, // Large 'ld|_hu'.
  0x200B23C7, // Large 'ld|_w'.
  0x300B23C7, // Large 'ld|_wu'.
  0x8124908C, // Small 'lddir'.
  0x337B3561, // Large 'ldg|t_b'.
  0x31F33561, // Large 'ldg|t_d'.
  0x337E3561, // Large 'ldg|t_h'.
  0x33813561, // Large 'ldg|t_w'.
  0x201045D4, // Large 'ldle|_b'.
  0x200945D4, // Large 'ldle|_d'.
  0x200E45D4, // Large 'ldle|_h'.
  0x200B45D4, // Large 'ldle|_w'.
  0x805A408C, // Small 'ldpte'.
  0x326C4568, // Large 'ldpt|r_d'.
  0x200B556C, // Large 'ldptr|_w'.
  0x201035C6, // Large 'ldx|_b'.
  0x301035C6, // Large 'ldx|_bu'.
  0x200935C6, // Large 'ldx|_d'.
  0x200E35C6, // Large 'ldx|_h'.
  0x301C35C6, // Large 'ldx|_hu'.
  0x200B35C6, // Large 'ldx|_w'.
  0x300B35C6, // Large 'ldx|_wu'.
  0x200920CD, // Large 'll|_d'.
  0x200B20CD, // Large 'll|_w'.
  0x32CB4571, // Large 'lu12|i_w'.
  0x31AA4575, // Large 'lu32|i_d'.
  0x31AA4579, // Large 'lu52|i_d'.
  0x3181457D, // Large 'mask|eqz'.
  0x3176457D, // Large 'mask|nez'.
  0x2009352D, // Large 'mod|_d'.
  0x3016352D, // Large 'mod|_du'.
  0x200B352D, // Large 'mod|_w'.
  0x300B352D, // Large 'mod|_wu'.
  0x32B254E9, // Large 'movcf|2fr'.
  0x30BB54E9, // Large 'movcf|2gr'.
  0x30BB736D, // Large 'movfcsr|2gr'.
  0x24EC6374, // Large 'movfr2|cf'.
  0x326C7374, // Large 'movfr2g|r_d'.
  0x325D7374, // Large 'movfr2g|r_s'.
  0x325D82A5, // Large 'movfrh2g|r_s'.
  0x24EC62AD, // Large 'movgr2|cf'.
  0x337172AD, // Large 'movgr2f|csr'.
  0x200982AD, // Large 'movgr2fr|_d'.
  0x200B82AD, // Large 'movgr2fr|_w'.
  0x31A082AD, // Large 'movgr2fr|h_w'.
  0x2009305F, // Large 'mul|_d'.
  0x200B305F, // Large 'mul|_w'.
  0x20094581, // Large 'mulh|_d'.
  0x30164581, // Large 'mulh|_du'.
  0x200B4581, // Large 'mulh|_w'.
  0x300B4581, // Large 'mulh|_wu'.
  0x4009405F, // Large 'mulw|_d_w'.
  0x5009405F, // Large 'mulw|_d_wu'.
  0x800049EE, // Small 'nor'.
  0x0000325C, // Large 'or_'.
  0x8002D9ED, // Small 'move'.
  0x8000264F, // Small 'ori'.
  0x80003A4F, // Small 'orn'.
  0x92420470, // Small 'pcaddi'.
  0x104D8436, // Large 'pcaddu12|i'.
  0x243E7436, // Large 'pcaddu1|8i'.
  0x104D8440, // Large 'pcalau12|i'.
  0x80461650, // Small 'preld'.
  0x20096448, // Large 'rdtime|_d'.
  0x31A06448, // Large 'rdtime|h_w'.
  0x344E6448, // Large 'rdtime|l_w'.
  0x100F6585, // Large 'revb_2|h'.
  0x10066585, // Large 'revb_2|w'.
  0x258B5585, // Large 'revb_|4h'.
  0x10045585, // Large 'revb_|d'.
  0x1006658D, // Large 'revh_2|w'.
  0x1004558D, // Large 'revh_|d'.
  0x200944B0, // Large 'rotr|_d'.
  0x200B44B0, // Large 'rotr|_w'.
  0x31AA44B0, // Large 'rotr|i_d'.
  0x32CB44B0, // Large 'rotr|i_w'.
  0x20092313, // Large 'sc|_d'.
  0x200B2313, // Large 'sc|_w'.
  0x200930CC, // Large 'sll|_d'.
  0x200B30CC, // Large 'sll|_w'.
  0x31AA30CC, // Large 'sll|i_d'.
  0x32CB30CC, // Large 'sll|i_w'.
  0x8004D193, // Small 'slti'.
  0x809AD193, // Small 'sltui'.
  0x200930E2, // Large 'sra|_d'.
  0x200B30E2, // Large 'sra|_w'.
  0x31AA30E2, // Large 'sra|i_d'.
  0x32CB30E2, // Large 'sra|i_w'.
  0x200930F9, // Large 'srl|_d'.
  0x200B30F9, // Large 'srl|_w'.
  0x31AA30F9, // Large 'srl|i_d'.
  0x32CB30F9, // Large 'srl|i_w'.
  0x20102356, // Large 'st|_b'.
  0x20092356, // Large 'st|_d'.
  0x200E2356, // Large 'st|_h'.
  0x200B2356, // Large 'st|_w'.
  0x337B3565, // Large 'stg|t_b'.
  0x31F33565, // Large 'stg|t_d'.
  0x337E3565, // Large 'stg|t_h'.
  0x33813565, // Large 'stg|t_w'.
  0x201045D8, // Large 'stle|_b'.
  0x200945D8, // Large 'stle|_d'.
  0x200E45D8, // Large 'stle|_h'.
  0x200B45D8, // Large 'stle|_w'.
  0x326C4593, // Large 'stpt|r_d'.
  0x200B5597, // Large 'stptr|_w'.
  0x201035D1, // Large 'stx|_b'.
  0x200935D1, // Large 'stx|_d'.
  0x200E35D1, // Large 'stx|_h'.
  0x200B35D1, // Large 'stx|_w'.
  0x00005533, // Large 'sub_d'.
  0x10064533, // Large 'sub_|w'.
  0x3173459C, // Large 'sysc|all'.
  0xA4C18994, // Small 'tlbclr'.
  0x20CD55A0, // Large 'tlbfi|ll'.
  0x240464EE, // Large 'tlbflu|sh'.
  0x80490994, // Small 'tlbrd'.
  0x255E55A5, // Large 'tlbsr|ch'.
  0x812B8994, // Small 'tlbwr'.
  0x0000434F, // Large 'xor_'.
  0x8004C9F8, // Small 'xori'.
  0x20105452, // Large 'vabsd|_b'.
  0x30105452, // Large 'vabsd|_bu'.
  0x20095452, // Large 'vabsd|_d'.
  0x30165452, // Large 'vabsd|_du'.
  0x200E5452, // Large 'vabsd|_h'.
  0x301C5452, // Large 'vabsd|_hu'.
  0x200B5452, // Large 'vabsd|_w'.
  0x300B5452, // Large 'vabsd|_wu'.
  0x20104030, // Large 'vadd|_b'.
  0x20094030, // Large 'vadd|_d'.
  0x200E4030, // Large 'vadd|_h'.
  0x20EC4030, // Large 'vadd|_q'.
  0x200B4030, // Large 'vadd|_w'.
  0x34F44030, // Large 'vadd|a_b'.
  0x34F74030, // Large 'vadd|a_d'.
  0x34FA4030, // Large 'vadd|a_h'.
  0x34FD4030, // Large 'vadd|a_w'.
  0x30105458, // Large 'vaddi|_bu'.
  0x30165458, // Large 'vaddi|_du'.
  0x301C5458, // Large 'vaddi|_hu'.
  0x300B5458, // Large 'vaddi|_wu'.
  0x0000B030, // Large 'vaddwev_d_w'.
  0x0000C030, // Large 'vaddwev_d_wu'.
  0x200BC030, // Large 'vaddwev_d_wu|_w'.
  0x300F8030, // Large 'vaddwev_|h_b'.
  0x400F8030, // Large 'vaddwev_|h_bu'.
  0x600F8030, // Large 'vaddwev_|h_bu_b'.
  0x30158030, // Large 'vaddwev_|q_d'.
  0x40158030, // Large 'vaddwev_|q_du'.
  0x60158030, // Large 'vaddwev_|q_du_d'.
  0x301B8030, // Large 'vaddwev_|w_h'.
  0x401B8030, // Large 'vaddwev_|w_hu'.
  0x601B8030, // Large 'vaddwev_|w_hu_h'.
  0x0000B03D, // Large 'vaddwod_d_w'.
  0x0000C03D, // Large 'vaddwod_d_wu'.
  0x200BC03D, // Large 'vaddwod_d_wu|_w'.
  0x300F803D, // Large 'vaddwod_|h_b'.
  0x400F803D, // Large 'vaddwod_|h_bu'.
  0x600F803D, // Large 'vaddwod_|h_bu_b'.
  0x3015803D, // Large 'vaddwod_|q_d'.
  0x4015803D, // Large 'vaddwod_|q_du'.
  0x6015803D, // Large 'vaddwod_|q_du_d'.
  0x301B803D, // Large 'vaddwod_|w_h'.
  0x401B803D, // Large 'vaddwod_|w_hu'.
  0x601B803D, // Large 'vaddwod_|w_hu_h'.
  0x80023836, // Small 'vand'.
  0x30E74501, // Large 'vand|i_b'.
  0x80E23836, // Small 'vandn'.
  0x2010445E, // Large 'vavg|_b'.
  0x3010445E, // Large 'vavg|_bu'.
  0x2009445E, // Large 'vavg|_d'.
  0x3016445E, // Large 'vavg|_du'.
  0x200E445E, // Large 'vavg|_h'.
  0x301C445E, // Large 'vavg|_hu'.
  0x200B445E, // Large 'vavg|_w'.
  0x300B445E, // Large 'vavg|_wu'.
  0x2010545E, // Large 'vavgr|_b'.
  0x3010545E, // Large 'vavgr|_bu'.
  0x2009545E, // Large 'vavgr|_d'.
  0x3016545E, // Large 'vavgr|_du'.
  0x200E545E, // Large 'vavgr|_h'.
  0x301C545E, // Large 'vavgr|_hu'.
  0x200B545E, // Large 'vavgr|_w'.
  0x300B545E, // Large 'vavgr|_wu'.
  0x201072B9, // Large 'vbitclr|_b'.
  0x200972B9, // Large 'vbitclr|_d'.
  0x200E72B9, // Large 'vbitclr|_h'.
  0x200B72B9, // Large 'vbitclr|_w'.
  0x101192B9, // Large 'vbitclri_|b'.
  0x100492B9, // Large 'vbitclri_|d'.
  0x0000A2B9, // Large 'vbitclri_h'.
  0x100692B9, // Large 'vbitclri_|w'.
  0x201072C4, // Large 'vbitrev|_b'.
  0x200972C4, // Large 'vbitrev|_d'.
  0x200E72C4, // Large 'vbitrev|_h'.
  0x200B72C4, // Large 'vbitrev|_w'.
  0x101192C4, // Large 'vbitrevi_|b'.
  0x100492C4, // Large 'vbitrevi_|d'.
  0x100F92C4, // Large 'vbitrevi_|h'.
  0x0000A2C4, // Large 'vbitrevi_w'.
  0x000072CF, // Large 'vbitsel'.
  0x30E772CF, // Large 'vbitsel|i_b'.
  0x337B62CF, // Large 'vbitse|t_b'.
  0x31F362CF, // Large 'vbitse|t_d'.
  0x337E62CF, // Large 'vbitse|t_h'.
  0x338162CF, // Large 'vbitse|t_w'.
  0x42D662CF, // Large 'vbitse|ti_b'.
  0x42DA62CF, // Large 'vbitse|ti_d'.
  0x42DE62CF, // Large 'vbitse|ti_h'.
  0x42E262CF, // Large 'vbitse|ti_w'.
  0x80C64C56, // Small 'vbsll'.
  0x80C94C56, // Small 'vbsrl'.
  0x201045AB, // Large 'vclo|_b'.
  0x200945AB, // Large 'vclo|_d'.
  0x200E45AB, // Large 'vclo|_h'.
  0x200B45AB, // Large 'vclo|_w'.
  0x201045B0, // Large 'vclz|_b'.
  0x200945B0, // Large 'vclz|_d'.
  0x200E45B0, // Large 'vclz|_h'.
  0x200B45B0, // Large 'vclz|_w'.
  0x20104506, // Large 'vdiv|_b'.
  0x30104506, // Large 'vdiv|_bu'.
  0x20094506, // Large 'vdiv|_d'.
  0x30164506, // Large 'vdiv|_du'.
  0x200E4506, // Large 'vdiv|_h'.
  0x301C4506, // Large 'vdiv|_hu'.
  0x200B4506, // Large 'vdiv|_w'.
  0x300B4506, // Large 'vdiv|_wu'.
  0x20109101, // Large 'vext2xv_d|_b'.
  0x200E9101, // Large 'vext2xv_d|_h'.
  0x200B9101, // Large 'vext2xv_d|_w'.
  0x40D69101, // Large 'vext2xv_d|u_bu'.
  0x40DB9101, // Large 'vext2xv_d|u_hu'.
  0x410A9101, // Large 'vext2xv_d|u_wu'.
  0x300F8101, // Large 'vext2xv_|h_b'.
  0x50D58101, // Large 'vext2xv_|hu_bu'.
  0x32B58101, // Large 'vext2xv_|w_b'.
  0x301B8101, // Large 'vext2xv_|w_h'.
  0x510E8101, // Large 'vext2xv_|wu_bu'.
  0x50DA8101, // Large 'vext2xv_|wu_hu'.
  0x300A6205, // Large 'vexth_|d_w'.
  0x51096205, // Large 'vexth_|du_wu'.
  0x300F6205, // Large 'vexth_|h_b'.
  0x50D56205, // Large 'vexth_|hu_bu'.
  0x30156205, // Large 'vexth_|q_d'.
  0x514F6205, // Large 'vexth_|qu_du'.
  0x301B6205, // Large 'vexth_|w_h'.
  0x50DA6205, // Large 'vexth_|wu_hu'.
  0x41BE520C, // Large 'vextl|_q_d'.
  0x614E520C, // Large 'vextl|_qu_du'.
  0x201082E7, // Large 'vextrins|_b'.
  0x200982E7, // Large 'vextrins|_d'.
  0x200E82E7, // Large 'vextrins|_h'.
  0x200B82E7, // Large 'vextrins|_w'.
  0x2009550B, // Large 'vfadd|_d'.
  0x2056550B, // Large 'vfadd|_s'.
  0x10048385, // Large 'vfclass_|d'.
  0x10578385, // Large 'vfclass_|s'.
  0x438C52F0, // Large 'vfcvt|_h_s'.
  0x439052F0, // Large 'vfcvt|_s_d'.
  0x205682F0, // Large 'vfcvth_d|_s'.
  0x32F872F0, // Large 'vfcvth_|s_h'.
  0x205682FC, // Large 'vfcvtl_d|_s'.
  0x32F872FC, // Large 'vfcvtl_|s_h'.
  0x10046511, // Large 'vfdiv_|d'.
  0x00007511, // Large 'vfdiv_s'.
  0x20548212, // Large 'vffint_d|_l'.
  0x309C8212, // Large 'vffint_d|_lu'.
  0x30827212, // Large 'vffint_|s_l'.
  0x321A7212, // Large 'vffint_|s_w'.
  0x421A7212, // Large 'vffint_|s_wu'.
  0x521E6212, // Large 'vffint|h_d_w'.
  0x50846212, // Large 'vffint|l_d_w'.
  0x20096464, // Large 'vflogb|_d'.
  0x20566464, // Large 'vflogb|_s'.
  0x2009646B, // Large 'vfmadd|_d'.
  0x2056646B, // Large 'vfmadd|_s'.
  0x20095472, // Large 'vfmax|_d'.
  0x20565472, // Large 'vfmax|_s'.
  0x20096472, // Large 'vfmaxa|_d'.
  0x20566472, // Large 'vfmaxa|_s'.
  0x20095479, // Large 'vfmin|_d'.
  0x20565479, // Large 'vfmin|_s'.
  0x20096479, // Large 'vfmina|_d'.
  0x20566479, // Large 'vfmina|_s'.
  0x20096480, // Large 'vfmsub|_d'.
  0x20566480, // Large 'vfmsub|_s'.
  0x30844519, // Large 'vfmu|l_d'.
  0x30554519, // Large 'vfmu|l_s'.
  0x20097399, // Large 'vfnmadd|_d'.
  0x20567399, // Large 'vfnmadd|_s'.
  0x200973A1, // Large 'vfnmsub|_d'.
  0x205673A1, // Large 'vfnmsub|_s'.
  0x328463A9, // Large 'vfreci|p_d'.
  0x31EC63A9, // Large 'vfreci|p_s'.
  0x20096224, // Large 'vfrint|_d'.
  0x20566224, // Large 'vfrint|_s'.
  0x33047224, // Large 'vfrintr|m_d'.
  0x33077224, // Large 'vfrintr|m_s'.
  0x41FA7224, // Large 'vfrintr|ne_d'.
  0x41E37224, // Large 'vfrintr|ne_s'.
  0x32847224, // Large 'vfrintr|p_d'.
  0x31EC7224, // Large 'vfrintr|p_s'.
  0x330A7224, // Large 'vfrintr|z_d'.
  0x330D7224, // Large 'vfrintr|z_s'.
  0x31F363B0, // Large 'vfrsqr|t_d'.
  0x31DC63B0, // Large 'vfrsqr|t_s'.
  0x201063B7, // Large 'vfrstp|_b'.
  0x200E63B7, // Large 'vfrstp|_h'.
  0x30E763B7, // Large 'vfrstp|i_b'.
  0x32C063B7, // Large 'vfrstp|i_h'.
  0x20098311, // Large 'vfscaleb|_d'.
  0x20568311, // Large 'vfscaleb|_s'.
  0x31F35487, // Large 'vfsqr|t_d'.
  0x31DC5487, // Large 'vfsqr|t_s'.
  0x2009551E, // Large 'vfsub|_d'.
  0x2056551E, // Large 'vfsub|_s'.
  0x4083604A, // Large 'vftint|_l_d'.
  0x509C604A, // Large 'vftint|_lu_d'.
  0x4087604A, // Large 'vftint|_w_d'.
  0x408B604A, // Large 'vftint|_w_s'.
  0x50A2604A, // Large 'vftint|_wu_s'.
  0x5053604A, // Large 'vftint|h_l_s'.
  0x5058604A, // Large 'vftint|l_l_s'.
  0x5113704A, // Large 'vftintr|m_l_d'.
  0x5118704A, // Large 'vftintr|m_w_d'.
  0x511D704A, // Large 'vftintr|m_w_s'.
  0x6077704A, // Large 'vftintr|mh_l_s'.
  0x607D704A, // Large 'vftintr|ml_l_s'.
  0x4083904A, // Large 'vftintrne|_l_d'.
  0x4087904A, // Large 'vftintrne|_w_d'.
  0x408B904A, // Large 'vftintrne|_w_s'.
  0x0000E04A, // Large 'vftintrneh_l_s'.
  0x5058904A, // Large 'vftintrne|l_l_s'.
  0x5122704A, // Large 'vftintr|p_l_d'.
  0x5127704A, // Large 'vftintr|p_w_d'.
  0x512C704A, // Large 'vftintr|p_w_s'.
  0x608F704A, // Large 'vftintr|ph_l_s'.
  0x6095704A, // Large 'vftintr|pl_l_s'.
  0x5131704A, // Large 'vftintr|z_l_d'.
  0x609B704A, // Large 'vftintr|z_lu_d'.
  0x5136704A, // Large 'vftintr|z_w_d'.
  0x513B704A, // Large 'vftintr|z_w_s'.
  0x60A1704A, // Large 'vftintr|z_wu_s'.
  0x60A7704A, // Large 'vftintr|zh_l_s'.
  0x60AD704A, // Large 'vftintr|zl_l_s'.
  0x300A7148, // Large 'vhaddw_|d_w'.
  0x51097148, // Large 'vhaddw_|du_wu'.
  0x300F7148, // Large 'vhaddw_|h_b'.
  0x50D57148, // Large 'vhaddw_|hu_bu'.
  0x20098148, // Large 'vhaddw_q|_d'.
  0x0000C148, // Large 'vhaddw_qu_du'.
  0x301B7148, // Large 'vhaddw_|w_h'.
  0x50DA7148, // Large 'vhaddw_|wu_hu'.
  0x300A715C, // Large 'vhsubw_|d_w'.
  0x5109715C, // Large 'vhsubw_|du_wu'.
  0x300F715C, // Large 'vhsubw_|h_b'.
  0x50D5715C, // Large 'vhsubw_|hu_bu'.
  0x3015715C, // Large 'vhsubw_|q_d'.
  0x514F715C, // Large 'vhsubw_|qu_du'.
  0x301B715C, // Large 'vhsubw_|w_h'.
  0x50DA715C, // Large 'vhsubw_|wu_hu'.
  0x300F4524, // Large 'vilv|h_b'.
  0x321E4524, // Large 'vilv|h_d'.
  0x31924524, // Large 'vilv|h_h'.
  0x31A04524, // Large 'vilv|h_w'.
  0x00007524, // Large 'vilvl_b'.
  0x10046524, // Large 'vilvl_|d'.
  0x100F6524, // Large 'vilvl_|h'.
  0x10066524, // Large 'vilvl_|w'.
  0x2010922C, // Large 'vinsgr2vr|_b'.
  0x2009922C, // Large 'vinsgr2vr|_d'.
  0x200E922C, // Large 'vinsgr2vr|_h'.
  0x200B922C, // Large 'vinsgr2vr|_w'.
  0x80001196, // Small 'vld'.
  0x80049196, // Small 'vldi'.
  0x201073C6, // Large 'vldrepl|_b'.
  0x200973C6, // Large 'vldrepl|_d'.
  0x200E73C6, // Large 'vldrepl|_h'.
  0x200B73C6, // Large 'vldrepl|_w'.
  0x800C1196, // Small 'vldx'.
  0x20105001, // Large 'vmadd|_b'.
  0x20095001, // Large 'vmadd|_d'.
  0x200E5001, // Large 'vmadd|_h'.
  0x200B5001, // Large 'vmadd|_w'.
  0x0000C001, // Large 'vmaddwev_d_w'.
  0x0000D001, // Large 'vmaddwev_d_wu'.
  0x1006E001, // Large 'vmaddwev_d_wu_|w'.
  0x300F9001, // Large 'vmaddwev_|h_b'.
  0x400F9001, // Large 'vmaddwev_|h_bu'.
  0x600F9001, // Large 'vmaddwev_|h_bu_b'.
  0x30159001, // Large 'vmaddwev_|q_d'.
  0x40159001, // Large 'vmaddwev_|q_du'.
  0x60159001, // Large 'vmaddwev_|q_du_d'.
  0x301B9001, // Large 'vmaddwev_|w_h'.
  0x401B9001, // Large 'vmaddwev_|w_hu'.
  0x601B9001, // Large 'vmaddwev_|w_hu_h'.
  0x0000C022, // Large 'vmaddwod_d_w'.
  0x0000D022, // Large 'vmaddwod_d_wu'.
  0x200BD022, // Large 'vmaddwod_d_wu|_w'.
  0x300F9022, // Large 'vmaddwod_|h_b'.
  0x400F9022, // Large 'vmaddwod_|h_bu'.
  0x600F9022, // Large 'vmaddwod_|h_bu_b'.
  0x30159022, // Large 'vmaddwod_|q_d'.
  0x40159022, // Large 'vmaddwod_|q_du'.
  0x60159022, // Large 'vmaddwod_|q_du_d'.
  0x301B9022, // Large 'vmaddwod_|w_h'.
  0x401B9022, // Large 'vmaddwod_|w_hu'.
  0x601B9022, // Large 'vmaddwod_|w_hu_h'.
  0x20104493, // Large 'vmax|_b'.
  0x30104493, // Large 'vmax|_bu'.
  0x20094493, // Large 'vmax|_d'.
  0x30164493, // Large 'vmax|_du'.
  0x200E4493, // Large 'vmax|_h'.
  0x301C4493, // Large 'vmax|_hu'.
  0x200B4493, // Large 'vmax|_w'.
  0x300B4493, // Large 'vmax|_wu'.
  0x20105493, // Large 'vmaxi|_b'.
  0x30105493, // Large 'vmaxi|_bu'.
  0x20095493, // Large 'vmaxi|_d'.
  0x30165493, // Large 'vmaxi|_du'.
  0x200E5493, // Large 'vmaxi|_h'.
  0x301C5493, // Large 'vmaxi|_hu'.
  0x200B5493, // Large 'vmaxi|_w'.
  0x300B5493, // Large 'vmaxi|_wu'.
  0x10B883CE, // Large 'vmepatms|k'.
  0x20104499, // Large 'vmin|_b'.
  0x30104499, // Large 'vmin|_bu'.
  0x20094499, // Large 'vmin|_d'.
  0x30164499, // Large 'vmin|_du'.
  0x200E4499, // Large 'vmin|_h'.
  0x301C4499, // Large 'vmin|_hu'.
  0x200B4499, // Large 'vmin|_w'.
  0x300B4499, // Large 'vmin|_wu'.
  0x20105499, // Large 'vmini|_b'.
  0x30105499, // Large 'vmini|_bu'.
  0x20095499, // Large 'vmini|_d'.
  0x30165499, // Large 'vmini|_du'.
  0x200E5499, // Large 'vmini|_h'.
  0x301C5499, // Large 'vmini|_hu'.
  0x200B5499, // Large 'vmini|_w'.
  0x300B5499, // Large 'vmini|_wu'.
  0x2010452C, // Large 'vmod|_b'.
  0x3010452C, // Large 'vmod|_bu'.
  0x2009452C, // Large 'vmod|_d'.
  0x3016452C, // Large 'vmod|_du'.
  0x200E452C, // Large 'vmod|_h'.
  0x301C452C, // Large 'vmod|_hu'.
  0x200B452C, // Large 'vmod|_w'.
  0x300B452C, // Large 'vmod|_wu'.
  0x201073D7, // Large 'vmskgez|_b'.
  0x201073DF, // Large 'vmskltz|_b'.
  0x200973DF, // Large 'vmskltz|_d'.
  0x200E73DF, // Large 'vmskltz|_h'.
  0x200B73DF, // Large 'vmskltz|_w'.
  0x2010649F, // Large 'vmsknz|_b'.
  0x10116531, // Large 'vmsub_|b'.
  0x00007531, // Large 'vmsub_d'.
  0x100F6531, // Large 'vmsub_|h'.
  0x10066531, // Large 'vmsub_|w'.
  0x20104539, // Large 'vmuh|_b'.
  0x30104539, // Large 'vmuh|_bu'.
  0x20094539, // Large 'vmuh|_d'.
  0x30164539, // Large 'vmuh|_du'.
  0x200E4539, // Large 'vmuh|_h'.
  0x301C4539, // Large 'vmuh|_hu'.
  0x200B4539, // Large 'vmuh|_w'.
  0x300B4539, // Large 'vmuh|_wu'.
  0x2010405E, // Large 'vmul|_b'.
  0x2009405E, // Large 'vmul|_d'.
  0x200E405E, // Large 'vmul|_h'.
  0x200B405E, // Large 'vmul|_w'.
  0x0000B05E, // Large 'vmulwev_d_w'.
  0x0000C05E, // Large 'vmulwev_d_wu'.
  0x200BC05E, // Large 'vmulwev_d_wu|_w'.
  0x300F805E, // Large 'vmulwev_|h_b'.
  0x400F805E, // Large 'vmulwev_|h_bu'.
  0x600F805E, // Large 'vmulwev_|h_bu_b'.
  0x3015805E, // Large 'vmulwev_|q_d'.
  0x4015805E, // Large 'vmulwev_|q_du'.
  0x6015805E, // Large 'vmulwev_|q_du_d'.
  0x301B805E, // Large 'vmulwev_|w_h'.
  0x401B805E, // Large 'vmulwev_|w_hu'.
  0x601B805E, // Large 'vmulwev_|w_hu_h'.
  0x0000B06B, // Large 'vmulwod_d_w'.
  0x0000C06B, // Large 'vmulwod_d_wu'.
  0x200BC06B, // Large 'vmulwod_d_wu|_w'.
  0x300F806B, // Large 'vmulwod_|h_b'.
  0x400F806B, // Large 'vmulwod_|h_bu'.
  0x600F806B, // Large 'vmulwod_|h_bu_b'.
  0x3015806B, // Large 'vmulwod_|q_d'.
  0x4015806B, // Large 'vmulwod_|q_du'.
  0x6015806B, // Large 'vmulwod_|q_du_d'.
  0x301B806B, // Large 'vmulwod_|w_h'.
  0x401B806B, // Large 'vmulwod_|w_hu'.
  0x601B806B, // Large 'vmulwod_|w_hu_h'.
  0x201045B5, // Large 'vneg|_b'.
  0x200945B5, // Large 'vneg|_d'.
  0x200E45B5, // Large 'vneg|_h'.
  0x200B45B5, // Large 'vneg|_w'.
  0x80093DD6, // Small 'vnor'.
  0x30E7453E, // Large 'vnor|i_b'.
  0x800049F6, // Small 'vor'.
  0x30E735BA, // Large 'vor|i_b'.
  0x800749F6, // Small 'vorn'.
  0x201073E7, // Large 'vpackev|_b'.
  0x200973E7, // Large 'vpackev|_d'.
  0x200E73E7, // Large 'vpackev|_h'.
  0x200B73E7, // Large 'vpackev|_w'.
  0x43EE53E7, // Large 'vpack|od_b'.
  0x402853E7, // Large 'vpack|od_d'.
  0x43F253E7, // Large 'vpack|od_h'.
  0x41C953E7, // Large 'vpack|od_w'.
  0x337B4543, // Large 'vpcn|t_b'.
  0x31F34543, // Large 'vpcn|t_d'.
  0x337E4543, // Large 'vpcn|t_h'.
  0x33814543, // Large 'vpcn|t_w'.
  0x100674A6, // Large 'vpermi_|w'.
  0x43F650B4, // Large 'vpick|ev_b'.
  0x400750B4, // Large 'vpick|ev_d'.
  0x43FA50B4, // Large 'vpick|ev_h'.
  0x43FE50B4, // Large 'vpick|ev_w'.
  0x43EE50B4, // Large 'vpick|od_b'.
  0x402850B4, // Large 'vpick|od_d'.
  0x43F250B4, // Large 'vpick|od_h'.
  0x41C950B4, // Large 'vpick|od_w'.
  0x2010A0B4, // Large 'vpickve2gr|_b'.
  0x3010A0B4, // Large 'vpickve2gr|_bu'.
  0x2009A0B4, // Large 'vpickve2gr|_d'.
  0x3016A0B4, // Large 'vpickve2gr|_du'.
  0x200EA0B4, // Large 'vpickve2gr|_h'.
  0x301CA0B4, // Large 'vpickve2gr|_hu'.
  0x200BA0B4, // Large 'vpickve2gr|_w'.
  0x300BA0B4, // Large 'vpickve2gr|_wu'.
  0x2010A164, // Large 'vreplgr2vr|_b'.
  0x2009A164, // Large 'vreplgr2vr|_d'.
  0x200EA164, // Large 'vreplgr2vr|_h'.
  0x200BA164, // Large 'vreplgr2vr|_w'.
  0x2010731A, // Large 'vreplve|_b'.
  0x2009731A, // Large 'vreplve|_d'.
  0x200E731A, // Large 'vreplve|_h'.
  0x200B731A, // Large 'vreplve|_w'.
  0x30E7731A, // Large 'vreplve|i_b'.
  0x31AA731A, // Large 'vreplve|i_d'.
  0x32C0731A, // Large 'vreplve|i_h'.
  0x32CB731A, // Large 'vreplve|i_w'.
  0x201054AF, // Large 'vrotr|_b'.
  0x200954AF, // Large 'vrotr|_d'.
  0x200E54AF, // Large 'vrotr|_h'.
  0x200B54AF, // Large 'vrotr|_w'.
  0x30E754AF, // Large 'vrotr|i_b'.
  0x31AA54AF, // Large 'vrotr|i_d'.
  0x32C054AF, // Large 'vrotr|i_h'.
  0x32CB54AF, // Large 'vrotr|i_w'.
  0x201054B5, // Large 'vsadd|_b'.
  0x301054B5, // Large 'vsadd|_bu'.
  0x200954B5, // Large 'vsadd|_d'.
  0x301654B5, // Large 'vsadd|_du'.
  0x200E54B5, // Large 'vsadd|_h'.
  0x301C54B5, // Large 'vsadd|_hu'.
  0x200B54B5, // Large 'vsadd|_w'.
  0x300B54B5, // Large 'vsadd|_wu'.
  0x20104548, // Large 'vsat|_b'.
  0x30104548, // Large 'vsat|_bu'.
  0x20094548, // Large 'vsat|_d'.
  0x30164548, // Large 'vsat|_du'.
  0x200E4548, // Large 'vsat|_h'.
  0x301C4548, // Large 'vsat|_hu'.
  0x200B4548, // Large 'vsat|_w'.
  0x300B4548, // Large 'vsat|_wu'.
  0x2010454D, // Large 'vseq|_b'.
  0x2009454D, // Large 'vseq|_d'.
  0x200E454D, // Large 'vseq|_h'.
  0x200B454D, // Large 'vseq|_w'.
  0x30E7454D, // Large 'vseq|i_b'.
  0x31AA454D, // Large 'vseq|i_d'.
  0x32C0454D, // Large 'vseq|i_h'.
  0x32CB454D, // Large 'vseq|i_w'.
  0x2010A16F, // Large 'vsetallnez|_b'.
  0x2009A16F, // Large 'vsetallnez|_d'.
  0x200EA16F, // Large 'vsetallnez|_h'.
  0x200BA16F, // Large 'vsetallnez|_w'.
  0x2010A17A, // Large 'vsetanyeqz|_b'.
  0x2009A17A, // Large 'vsetanyeqz|_d'.
  0x200EA17A, // Large 'vsetanyeqz|_h'.
  0x200BA17A, // Large 'vsetanyeqz|_w'.
  0x3181416F, // Large 'vset|eqz'.
  0x3176416F, // Large 'vset|nez'.
  0x30E76403, // Large 'vshuf4|i_b'.
  0x31AA6403, // Large 'vshuf4|i_d'.
  0x32C06403, // Large 'vshuf4|i_h'.
  0x32CB6403, // Large 'vshuf4|i_w'.
  0x20105403, // Large 'vshuf|_b'.
  0x20095403, // Large 'vshuf|_d'.
  0x200E5403, // Large 'vshuf|_h'.
  0x200B5403, // Large 'vshuf|_w'.
  0x20108323, // Large 'vsigncov|_b'.
  0x20098323, // Large 'vsigncov|_d'.
  0x200E8323, // Large 'vsigncov|_h'.
  0x200B8323, // Large 'vsigncov|_w'.
  0x201044BB, // Large 'vsle|_b'.
  0x301044BB, // Large 'vsle|_bu'.
  0x200944BB, // Large 'vsle|_d'.
  0x301644BB, // Large 'vsle|_du'.
  0x200E44BB, // Large 'vsle|_h'.
  0x301C44BB, // Large 'vsle|_hu'.
  0x200B44BB, // Large 'vsle|_w'.
  0x300B44BB, // Large 'vsle|_wu'.
  0x201054BB, // Large 'vslei|_b'.
  0x301054BB, // Large 'vslei|_bu'.
  0x200954BB, // Large 'vslei|_d'.
  0x301654BB, // Large 'vslei|_du'.
  0x200E54BB, // Large 'vslei|_h'.
  0x301C54BB, // Large 'vslei|_hu'.
  0x200B54BB, // Large 'vslei|_w'.
  0x300B54BB, // Large 'vslei|_wu'.
  0x201040CB, // Large 'vsll|_b'.
  0x200940CB, // Large 'vsll|_d'.
  0x200E40CB, // Large 'vsll|_h'.
  0x200B40CB, // Large 'vsll|_w'.
  0x30E740CB, // Large 'vsll|i_b'.
  0x31AA40CB, // Large 'vsll|i_d'.
  0x32C040CB, // Large 'vsll|i_h'.
  0x32CB40CB, // Large 'vsll|i_w'.
  0x200B90CB, // Large 'vsllwil_d|_w'.
  0x300BA0CB, // Large 'vsllwil_du|_wu'.
  0x300F80CB, // Large 'vsllwil_|h_b'.
  0x50D580CB, // Large 'vsllwil_|hu_bu'.
  0x301B80CB, // Large 'vsllwil_|w_h'.
  0x50DA80CB, // Large 'vsllwil_|wu_hu'.
  0x201044C1, // Large 'vslt|_b'.
  0x301044C1, // Large 'vslt|_bu'.
  0x200944C1, // Large 'vslt|_d'.
  0x301644C1, // Large 'vslt|_du'.
  0x200E44C1, // Large 'vslt|_h'.
  0x301C44C1, // Large 'vslt|_hu'.
  0x200B44C1, // Large 'vslt|_w'.
  0x300B44C1, // Large 'vslt|_wu'.
  0x201054C1, // Large 'vslti|_b'.
  0x301054C1, // Large 'vslti|_bu'.
  0x200954C1, // Large 'vslti|_d'.
  0x301654C1, // Large 'vslti|_du'.
  0x200E54C1, // Large 'vslti|_h'.
  0x301C54C1, // Large 'vslti|_hu'.
  0x200B54C1, // Large 'vslti|_w'.
  0x300B54C1, // Large 'vslti|_wu'.
  0x20104236, // Large 'vsra|_b'.
  0x20094236, // Large 'vsra|_d'.
  0x200E4236, // Large 'vsra|_h'.
  0x200B4236, // Large 'vsra|_w'.
  0x30E74236, // Large 'vsra|i_b'.
  0x31AA4236, // Large 'vsra|i_d'.
  0x32C04236, // Large 'vsra|i_h'.
  0x32CB4236, // Large 'vsra|i_w'.
  0x4249532C, // Large 'vsran|_b_h'.
  0x424D532C, // Large 'vsran|_h_w'.
  0x4087532C, // Large 'vsran|_w_d'.
  0x324A732C, // Large 'vsrani_|b_h'.
  0x0000A32C, // Large 'vsrani_d_q'.
  0x31A0732C, // Large 'vsrani_|h_w'.
  0x3088732C, // Large 'vsrani_|w_d'.
  0x20105236, // Large 'vsrar|_b'.
  0x20095236, // Large 'vsrar|_d'.
  0x200E5236, // Large 'vsrar|_h'.
  0x200B5236, // Large 'vsrar|_w'.
  0x30E75236, // Large 'vsrar|i_b'.
  0x31AA5236, // Large 'vsrar|i_d'.
  0x32C05236, // Large 'vsrar|i_h'.
  0x32CB5236, // Large 'vsrar|i_w'.
  0x42496236, // Large 'vsrarn|_b_h'.
  0x424D6236, // Large 'vsrarn|_h_w'.
  0x40876236, // Large 'vsrarn|_w_d'.
  0x200E9236, // Large 'vsrarni_b|_h'.
  0x319D8236, // Large 'vsrarni_|d_q'.
  0x31A08236, // Large 'vsrarni_|h_w'.
  0x30888236, // Large 'vsrarni_|w_d'.
  0x20104240, // Large 'vsrl|_b'.
  0x20094240, // Large 'vsrl|_d'.
  0x200E4240, // Large 'vsrl|_h'.
  0x200B4240, // Large 'vsrl|_w'.
  0x30E74240, // Large 'vsrl|i_b'.
  0x31AA4240, // Large 'vsrl|i_d'.
  0x32C04240, // Large 'vsrl|i_h'.
  0x32CB4240, // Large 'vsrl|i_w'.
  0x42495337, // Large 'vsrln|_b_h'.
  0x424D5337, // Large 'vsrln|_h_w'.
  0x40875337, // Large 'vsrln|_w_d'.
  0x42496337, // Large 'vsrlni|_b_h'.
  0x43326337, // Large 'vsrlni|_d_q'.
  0x424D6337, // Large 'vsrlni|_h_w'.
  0x40876337, // Large 'vsrlni|_w_d'.
  0x20105240, // Large 'vsrlr|_b'.
  0x20095240, // Large 'vsrlr|_d'.
  0x200E5240, // Large 'vsrlr|_h'.
  0x200B5240, // Large 'vsrlr|_w'.
  0x30E75240, // Large 'vsrlr|i_b'.
  0x31AA5240, // Large 'vsrlr|i_d'.
  0x32C05240, // Large 'vsrlr|i_h'.
  0x32CB5240, // Large 'vsrlr|i_w'.
  0x42496240, // Large 'vsrlrn|_b_h'.
  0x424D6240, // Large 'vsrlrn|_h_w'.
  0x40876240, // Large 'vsrlrn|_w_d'.
  0x42496185, // Large 'vssran|_b_h'.
  0x518E6185, // Large 'vssran|_bu_h'.
  0x424D6185, // Large 'vssran|_h_w'.
  0x51936185, // Large 'vssran|_hu_w'.
  0x40876185, // Large 'vssran|_w_d'.
  0x51986185, // Large 'vssran|_wu_d'.
  0x100FA185, // Large 'vssrani_b_|h'.
  0x300D9185, // Large 'vssrani_b|u_h'.
  0x319D8185, // Large 'vssrani_|d_q'.
  0x40EA8185, // Large 'vssrani_|du_q'.
  0x31A08185, // Large 'vssrani_|h_w'.
  0x40EE8185, // Large 'vssrani_|hu_w'.
  0x30888185, // Large 'vssrani_|w_d'.
  0x40F28185, // Large 'vssrani_|wu_d'.
  0x424970E0, // Large 'vssrarn|_b_h'.
  0x518E70E0, // Large 'vssrarn|_bu_h'.
  0x424D70E0, // Large 'vssrarn|_h_w'.
  0x519370E0, // Large 'vssrarn|_hu_w'.
  0x408770E0, // Large 'vssrarn|_w_d'.
  0x519870E0, // Large 'vssrarn|_wu_d'.
  0x200EA0E0, // Large 'vssrarni_b|_h'.
  0x300DA0E0, // Large 'vssrarni_b|u_h'.
  0x319D90E0, // Large 'vssrarni_|d_q'.
  0x40EA90E0, // Large 'vssrarni_|du_q'.
  0x31A090E0, // Large 'vssrarni_|h_w'.
  0x40EE90E0, // Large 'vssrarni_|hu_w'.
  0x308890E0, // Large 'vssrarni_|w_d'.
  0x40F290E0, // Large 'vssrarni_|wu_d'.
  0x424961A4, // Large 'vssrln|_b_h'.
  0x518E61A4, // Large 'vssrln|_bu_h'.
  0x424D61A4, // Large 'vssrln|_h_w'.
  0x519361A4, // Large 'vssrln|_hu_w'.
  0x408761A4, // Large 'vssrln|_w_d'.
  0x519861A4, // Large 'vssrln|_wu_d'.
  0x324A81A4, // Large 'vssrlni_|b_h'.
  0x418F81A4, // Large 'vssrlni_|bu_h'.
  0x20EC91A4, // Large 'vssrlni_d|_q'.
  0x0000C1A4, // Large 'vssrlni_du_q'.
  0x31A081A4, // Large 'vssrlni_|h_w'.
  0x40EE81A4, // Large 'vssrlni_|hu_w'.
  0x308881A4, // Large 'vssrlni_|w_d'.
  0x40F281A4, // Large 'vssrlni_|wu_d'.
  0x424970F7, // Large 'vssrlrn|_b_h'.
  0x518E70F7, // Large 'vssrlrn|_bu_h'.
  0x424D70F7, // Large 'vssrlrn|_h_w'.
  0x519370F7, // Large 'vssrlrn|_hu_w'.
  0x408770F7, // Large 'vssrlrn|_w_d'.
  0x519870F7, // Large 'vssrlrn|_wu_d'.
  0x200EA0F7, // Large 'vssrlrni_b|_h'.
  0x300DA0F7, // Large 'vssrlrni_b|u_h'.
  0x319D90F7, // Large 'vssrlrni_|d_q'.
  0x40EA90F7, // Large 'vssrlrni_|du_q'.
  0x31A090F7, // Large 'vssrlrni_|h_w'.
  0x40EE90F7, // Large 'vssrlrni_|hu_w'.
  0x308890F7, // Large 'vssrlrni_|w_d'.
  0x40F290F7, // Large 'vssrlrni_|wu_d'.
  0x201054CC, // Large 'vssub|_b'.
  0x301054CC, // Large 'vssub|_bu'.
  0x200954CC, // Large 'vssub|_d'.
  0x301654CC, // Large 'vssub|_du'.
  0x200E54CC, // Large 'vssub|_h'.
  0x301C54CC, // Large 'vssub|_hu'.
  0x200B54CC, // Large 'vssub|_w'.
  0x300B54CC, // Large 'vssub|_wu'.
  0x80005276, // Small 'vst'.
  0x201064D2, // Large 'vstelm|_b'.
  0x200964D2, // Large 'vstelm|_d'.
  0x200E64D2, // Large 'vstelm|_h'.
  0x200B64D2, // Large 'vstelm|_w'.
  0x800C5276, // Small 'vstx'.
  0x201041B1, // Large 'vsub|_b'.
  0x200941B1, // Large 'vsub|_d'.
  0x200E41B1, // Large 'vsub|_h'.
  0x20EC41B1, // Large 'vsub|_q'.
  0x200B41B1, // Large 'vsub|_w'.
  0x301054D9, // Large 'vsubi|_bu'.
  0x301654D9, // Large 'vsubi|_du'.
  0x301C54D9, // Large 'vsubi|_hu'.
  0x300B54D9, // Large 'vsubi|_wu'.
  0x300A81B7, // Large 'vsubwev_|d_w'.
  0x400A81B7, // Large 'vsubwev_|d_wu'.
  0x300F81B7, // Large 'vsubwev_|h_b'.
  0x400F81B7, // Large 'vsubwev_|h_bu'.
  0x0000B1B7, // Large 'vsubwev_q_d'.
  0x0000C1B7, // Large 'vsubwev_q_du'.
  0x301B81B7, // Large 'vsubwev_|w_h'.
  0x401B81B7, // Large 'vsubwev_|w_hu'.
  0x300A81C4, // Large 'vsubwod_|d_w'.
  0x400A81C4, // Large 'vsubwod_|d_wu'.
  0x300F81C4, // Large 'vsubwod_|h_b'.
  0x400F81C4, // Large 'vsubwod_|h_bu'.
  0x301581C4, // Large 'vsubwod_|q_d'.
  0x401581C4, // Large 'vsubwod_|q_du'.
  0x0000B1C4, // Large 'vsubwod_w_h'.
  0x0000C1C4, // Large 'vsubwod_w_hu'.
  0x80093F16, // Small 'vxor'.
  0x30E74552, // Large 'vxor|i_b'.
  0x20106451, // Large 'xvabsd|_b'.
  0x30106451, // Large 'xvabsd|_bu'.
  0x20096451, // Large 'xvabsd|_d'.
  0x30166451, // Large 'xvabsd|_du'.
  0x200E6451, // Large 'xvabsd|_h'.
  0x301C6451, // Large 'xvabsd|_hu'.
  0x200B6451, // Large 'xvabsd|_w'.
  0x300B6451, // Large 'xvabsd|_wu'.
  0x2010502F, // Large 'xvadd|_b'.
  0x2009502F, // Large 'xvadd|_d'.
  0x200E502F, // Large 'xvadd|_h'.
  0x20EC502F, // Large 'xvadd|_q'.
  0x200B502F, // Large 'xvadd|_w'.
  0x34F4502F, // Large 'xvadd|a_b'.
  0x34F7502F, // Large 'xvadd|a_d'.
  0x34FA502F, // Large 'xvadd|a_h'.
  0x34FD502F, // Large 'xvadd|a_w'.
  0x30106457, // Large 'xvaddi|_bu'.
  0x30166457, // Large 'xvaddi|_du'.
  0x301C6457, // Large 'xvaddi|_hu'.
  0x300B6457, // Large 'xvaddi|_wu'.
  0x0000C02F, // Large 'xvaddwev_d_w'.
  0x0000D02F, // Large 'xvaddwev_d_wu'.
  0x200BD02F, // Large 'xvaddwev_d_wu|_w'.
  0x300F902F, // Large 'xvaddwev_|h_b'.
  0x400F902F, // Large 'xvaddwev_|h_bu'.
  0x600F902F, // Large 'xvaddwev_|h_bu_b'.
  0x3015902F, // Large 'xvaddwev_|q_d'.
  0x4015902F, // Large 'xvaddwev_|q_du'.
  0x6015902F, // Large 'xvaddwev_|q_du_d'.
  0x301B902F, // Large 'xvaddwev_|w_h'.
  0x401B902F, // Large 'xvaddwev_|w_hu'.
  0x601B902F, // Large 'xvaddwev_|w_hu_h'.
  0x0000C03C, // Large 'xvaddwod_d_w'.
  0x0000D03C, // Large 'xvaddwod_d_wu'.
  0x200BD03C, // Large 'xvaddwod_d_wu|_w'.
  0x300F903C, // Large 'xvaddwod_|h_b'.
  0x400F903C, // Large 'xvaddwod_|h_bu'.
  0x600F903C, // Large 'xvaddwod_|h_bu_b'.
  0x3015903C, // Large 'xvaddwod_|q_d'.
  0x4015903C, // Large 'xvaddwod_|q_du'.
  0x6015903C, // Large 'xvaddwod_|q_du_d'.
  0x301B903C, // Large 'xvaddwod_|w_h'.
  0x401B903C, // Large 'xvaddwod_|w_hu'.
  0x601B903C, // Large 'xvaddwod_|w_hu_h'.
  0x804706D8, // Small 'xvand'.
  0x30E75500, // Large 'xvand|i_b'.
  0x9C4706D8, // Small 'xvandn'.
  0x2010545D, // Large 'xvavg|_b'.
  0x3010545D, // Large 'xvavg|_bu'.
  0x2009545D, // Large 'xvavg|_d'.
  0x3016545D, // Large 'xvavg|_du'.
  0x200E545D, // Large 'xvavg|_h'.
  0x301C545D, // Large 'xvavg|_hu'.
  0x200B545D, // Large 'xvavg|_w'.
  0x300B545D, // Large 'xvavg|_wu'.
  0x2010645D, // Large 'xvavgr|_b'.
  0x3010645D, // Large 'xvavgr|_bu'.
  0x2009645D, // Large 'xvavgr|_d'.
  0x3016645D, // Large 'xvavgr|_du'.
  0x200E645D, // Large 'xvavgr|_h'.
  0x301C645D, // Large 'xvavgr|_hu'.
  0x200B645D, // Large 'xvavgr|_w'.
  0x300B645D, // Large 'xvavgr|_wu'.
  0x201082B8, // Large 'xvbitclr|_b'.
  0x200982B8, // Large 'xvbitclr|_d'.
  0x200E82B8, // Large 'xvbitclr|_h'.
  0x200B82B8, // Large 'xvbitclr|_w'.
  0x30E782B8, // Large 'xvbitclr|i_b'.
  0x31AA82B8, // Large 'xvbitclr|i_d'.
  0x32C082B8, // Large 'xvbitclr|i_h'.
  0x1006A2B8, // Large 'xvbitclri_|w'.
  0x201082C3, // Large 'xvbitrev|_b'.
  0x200982C3, // Large 'xvbitrev|_d'.
  0x200E82C3, // Large 'xvbitrev|_h'.
  0x200B82C3, // Large 'xvbitrev|_w'.
  0x30E782C3, // Large 'xvbitrev|i_b'.
  0x31AA82C3, // Large 'xvbitrev|i_d'.
  0x32C082C3, // Large 'xvbitrev|i_h'.
  0x32CB82C3, // Large 'xvbitrev|i_w'.
  0x000082CE, // Large 'xvbitsel'.
  0x30E782CE, // Large 'xvbitsel|i_b'.
  0x337B72CE, // Large 'xvbitse|t_b'.
  0x31F372CE, // Large 'xvbitse|t_d'.
  0x337E72CE, // Large 'xvbitse|t_h'.
  0x338172CE, // Large 'xvbitse|t_w'.
  0x42D672CE, // Large 'xvbitse|ti_b'.
  0x42DA72CE, // Large 'xvbitse|ti_d'.
  0x42DE72CE, // Large 'xvbitse|ti_h'.
  0x42E272CE, // Large 'xvbitse|ti_w'.
  0x98C98AD8, // Small 'xvbsll'.
  0x99298AD8, // Small 'xvbsrl'.
  0x201055AA, // Large 'xvclo|_b'.
  0x200955AA, // Large 'xvclo|_d'.
  0x200E55AA, // Large 'xvclo|_h'.
  0x200B55AA, // Large 'xvclo|_w'.
  0x201055AF, // Large 'xvclz|_b'.
  0x200955AF, // Large 'xvclz|_d'.
  0x200E55AF, // Large 'xvclz|_h'.
  0x200B55AF, // Large 'xvclz|_w'.
  0x20105505, // Large 'xvdiv|_b'.
  0x30105505, // Large 'xvdiv|_bu'.
  0x20095505, // Large 'xvdiv|_d'.
  0x30165505, // Large 'xvdiv|_du'.
  0x200E5505, // Large 'xvdiv|_h'.
  0x301C5505, // Large 'xvdiv|_hu'.
  0x200B5505, // Large 'xvdiv|_w'.
  0x300B5505, // Large 'xvdiv|_wu'.
  0x300A7204, // Large 'xvexth_|d_w'.
  0x610861FE, // Large 'xvexth|_du_wu'.
  0x300F7204, // Large 'xvexth_|h_b'.
  0x50D57204, // Large 'xvexth_|hu_bu'.
  0x30157204, // Large 'xvexth_|q_d'.
  0x514F7204, // Large 'xvexth_|qu_du'.
  0x301B7204, // Large 'xvexth_|w_h'.
  0x50DA7204, // Large 'xvexth_|wu_hu'.
  0x41BE620B, // Large 'xvextl|_q_d'.
  0x614E620B, // Large 'xvextl|_qu_du'.
  0x201092E6, // Large 'xvextrins|_b'.
  0x200992E6, // Large 'xvextrins|_d'.
  0x200E92E6, // Large 'xvextrins|_h'.
  0x200B92E6, // Large 'xvextrins|_w'.
  0x533F3049, // Large 'xvf|add_d'.
  0x2056650A, // Large 'xvfadd|_s'.
  0x20098384, // Large 'xvfclass|_d'.
  0x20568384, // Large 'xvfclass|_s'.
  0x438C62EF, // Large 'xvfcvt|_h_s'.
  0x439062EF, // Large 'xvfcvt|_s_d'.
  0x205692EF, // Large 'xvfcvth_d|_s'.
  0x32F882EF, // Large 'xvfcvth_|s_h'.
  0x205692FB, // Large 'xvfcvtl_d|_s'.
  0x32F882FB, // Large 'xvfcvtl_|s_h'.
  0x30085510, // Large 'xvfdi|v_d'.
  0x35155510, // Large 'xvfdi|v_s'.
  0x20549211, // Large 'xvffint_d|_l'.
  0x309C9211, // Large 'xvffint_d|_lu'.
  0x30828211, // Large 'xvffint_|s_l'.
  0x321A8211, // Large 'xvffint_|s_w'.
  0x421A8211, // Large 'xvffint_|s_wu'.
  0x521E7211, // Large 'xvffint|h_d_w'.
  0x50847211, // Large 'xvffint|l_d_w'.
  0x20097463, // Large 'xvflogb|_d'.
  0x20567463, // Large 'xvflogb|_s'.
  0x633E3049, // Large 'xvf|madd_d'.
  0x2056746A, // Large 'xvfmadd|_s'.
  0x20096471, // Large 'xvfmax|_d'.
  0x20566471, // Large 'xvfmax|_s'.
  0x20097471, // Large 'xvfmaxa|_d'.
  0x20567471, // Large 'xvfmaxa|_s'.
  0x20096478, // Large 'xvfmin|_d'.
  0x20566478, // Large 'xvfmin|_s'.
  0x20097478, // Large 'xvfmina|_d'.
  0x20567478, // Large 'xvfmina|_s'.
  0x2009747F, // Large 'xvfmsub|_d'.
  0x2056747F, // Large 'xvfmsub|_s'.
  0x30845518, // Large 'xvfmu|l_d'.
  0x30555518, // Large 'xvfmu|l_s'.
  0x633E4394, // Large 'xvfn|madd_d'.
  0x20568398, // Large 'xvfnmadd|_s'.
  0x200983A0, // Large 'xvfnmsub|_d'.
  0x205683A0, // Large 'xvfnmsub|_s'.
  0x328473A8, // Large 'xvfreci|p_d'.
  0x31EC73A8, // Large 'xvfreci|p_s'.
  0x20097223, // Large 'xvfrint|_d'.
  0x20567223, // Large 'xvfrint|_s'.
  0x33048223, // Large 'xvfrintr|m_d'.
  0x33078223, // Large 'xvfrintr|m_s'.
  0x41FA8223, // Large 'xvfrintr|ne_d'.
  0x41E38223, // Large 'xvfrintr|ne_s'.
  0x32848223, // Large 'xvfrintr|p_d'.
  0x31EC8223, // Large 'xvfrintr|p_s'.
  0x330A8223, // Large 'xvfrintr|z_d'.
  0x330D8223, // Large 'xvfrintr|z_s'.
  0x31F373AF, // Large 'xvfrsqr|t_d'.
  0x31DC73AF, // Large 'xvfrsqr|t_s'.
  0x201073B6, // Large 'xvfrstp|_b'.
  0x200E73B6, // Large 'xvfrstp|_h'.
  0x30E773B6, // Large 'xvfrstp|i_b'.
  0x32C073B6, // Large 'xvfrstp|i_h'.
  0x20099310, // Large 'xvfscaleb|_d'.
  0x20569310, // Large 'xvfscaleb|_s'.
  0x31F36486, // Large 'xvfsqr|t_d'.
  0x31DC6486, // Large 'xvfsqr|t_s'.
  0x2009651D, // Large 'xvfsub|_d'.
  0x2056651D, // Large 'xvfsub|_s'.
  0x40837049, // Large 'xvftint|_l_d'.
  0x509C7049, // Large 'xvftint|_lu_d'.
  0x40877049, // Large 'xvftint|_w_d'.
  0x408B7049, // Large 'xvftint|_w_s'.
  0x50A27049, // Large 'xvftint|_wu_s'.
  0x50537049, // Large 'xvftint|h_l_s'.
  0x50587049, // Large 'xvftint|l_l_s'.
  0x51138049, // Large 'xvftintr|m_l_d'.
  0x51188049, // Large 'xvftintr|m_w_d'.
  0x511D8049, // Large 'xvftintr|m_w_s'.
  0x60778049, // Large 'xvftintr|mh_l_s'.
  0x607D8049, // Large 'xvftintr|ml_l_s'.
  0x4083A049, // Large 'xvftintrne|_l_d'.
  0x4087A049, // Large 'xvftintrne|_w_d'.
  0x408BA049, // Large 'xvftintrne|_w_s'.
  0x0000F049, // Large 'xvftintrneh_l_s'.
  0x5058A049, // Large 'xvftintrne|l_l_s'.
  0x51228049, // Large 'xvftintr|p_l_d'.
  0x51278049, // Large 'xvftintr|p_w_d'.
  0x512C8049, // Large 'xvftintr|p_w_s'.
  0x608F8049, // Large 'xvftintr|ph_l_s'.
  0x60958049, // Large 'xvftintr|pl_l_s'.
  0x51318049, // Large 'xvftintr|z_l_d'.
  0x609B8049, // Large 'xvftintr|z_lu_d'.
  0x51368049, // Large 'xvftintr|z_w_d'.
  0x513B8049, // Large 'xvftintr|z_w_s'.
  0x60A18049, // Large 'xvftintr|z_wu_s'.
  0x60A78049, // Large 'xvftintr|zh_l_s'.
  0x60AD8049, // Large 'xvftintr|zl_l_s'.
  0x300A8147, // Large 'xvhaddw_|d_w'.
  0x61087140, // Large 'xvhaddw|_du_wu'.
  0x300F8147, // Large 'xvhaddw_|h_b'.
  0x50D58147, // Large 'xvhaddw_|hu_bu'.
  0x20099147, // Large 'xvhaddw_q|_d'.
  0x514F8147, // Large 'xvhaddw_|qu_du'.
  0x301B8147, // Large 'xvhaddw_|w_h'.
  0x50DA8147, // Large 'xvhaddw_|wu_hu'.
  0x31AA648C, // Large 'xvhsel|i_d'.
  0x300A815B, // Large 'xvhsubw_|d_w'.
  0x61087154, // Large 'xvhsubw|_du_wu'.
  0x300F815B, // Large 'xvhsubw_|h_b'.
  0x50D5815B, // Large 'xvhsubw_|hu_bu'.
  0x3015815B, // Large 'xvhsubw_|q_d'.
  0x514F815B, // Large 'xvhsubw_|qu_du'.
  0x301B815B, // Large 'xvhsubw_|w_h'.
  0x50DA815B, // Large 'xvhsubw_|wu_hu'.
  0x300F5523, // Large 'xvilv|h_b'.
  0x321E5523, // Large 'xvilv|h_d'.
  0x31925523, // Large 'xvilv|h_h'.
  0x31A05523, // Large 'xvilv|h_w'.
  0x35285523, // Large 'xvilv|l_b'.
  0x10047523, // Large 'xvilvl_|d'.
  0x100F7523, // Large 'xvilvl_|h'.
  0x10067523, // Large 'xvilvl_|w'.
  0x2009A22B, // Large 'xvinsgr2vr|_d'.
  0x200BA22B, // Large 'xvinsgr2vr|_w'.
  0x200983BD, // Large 'xvinsve0|_d'.
  0x200B83BD, // Large 'xvinsve0|_w'.
  0x800232D8, // Small 'xvld'.
  0x809232D8, // Small 'xvldi'.
  0x201083C5, // Large 'xvldrepl|_b'.
  0x200983C5, // Large 'xvldrepl|_d'.
  0x200E83C5, // Large 'xvldrepl|_h'.
  0x200B83C5, // Large 'xvldrepl|_w'.
  0x818232D8, // Small 'xvldx'.
  0x20106000, // Large 'xvmadd|_b'.
  0x20096000, // Large 'xvmadd|_d'.
  0x200E6000, // Large 'xvmadd|_h'.
  0x200B6000, // Large 'xvmadd|_w'.
  0x0000D000, // Large 'xvmaddwev_d_w'.
  0x0000E000, // Large 'xvmaddwev_d_wu'.
  0x1006F000, // Large 'xvmaddwev_d_wu_|w'.
  0x300FA000, // Large 'xvmaddwev_|h_b'.
  0x400FA000, // Large 'xvmaddwev_|h_bu'.
  0x600FA000, // Large 'xvmaddwev_|h_bu_b'.
  0x3015A000, // Large 'xvmaddwev_|q_d'.
  0x4015A000, // Large 'xvmaddwev_|q_du'.
  0x6015A000, // Large 'xvmaddwev_|q_du_d'.
  0x301BA000, // Large 'xvmaddwev_|w_h'.
  0x401BA000, // Large 'xvmaddwev_|w_hu'.
  0x601BA000, // Large 'xvmaddwev_|w_hu_h'.
  0x0000D021, // Large 'xvmaddwod_d_w'.
  0x0000E021, // Large 'xvmaddwod_d_wu'.
  0x200BE021, // Large 'xvmaddwod_d_wu|_w'.
  0x300FA021, // Large 'xvmaddwod_|h_b'.
  0x400FA021, // Large 'xvmaddwod_|h_bu'.
  0x600FA021, // Large 'xvmaddwod_|h_bu_b'.
  0x3015A021, // Large 'xvmaddwod_|q_d'.
  0x4015A021, // Large 'xvmaddwod_|q_du'.
  0x6015A021, // Large 'xvmaddwod_|q_du_d'.
  0x301BA021, // Large 'xvmaddwod_|w_h'.
  0x401BA021, // Large 'xvmaddwod_|w_hu'.
  0x601BA021, // Large 'xvmaddwod_|w_hu_h'.
  0x20105492, // Large 'xvmax|_b'.
  0x30105492, // Large 'xvmax|_bu'.
  0x20095492, // Large 'xvmax|_d'.
  0x30165492, // Large 'xvmax|_du'.
  0x200E5492, // Large 'xvmax|_h'.
  0x301C5492, // Large 'xvmax|_hu'.
  0x200B5492, // Large 'xvmax|_w'.
  0x300B5492, // Large 'xvmax|_wu'.
  0x20106492, // Large 'xvmaxi|_b'.
  0x30106492, // Large 'xvmaxi|_bu'.
  0x20096492, // Large 'xvmaxi|_d'.
  0x30166492, // Large 'xvmaxi|_du'.
  0x200E6492, // Large 'xvmaxi|_h'.
  0x301C6492, // Large 'xvmaxi|_hu'.
  0x200B6492, // Large 'xvmaxi|_w'.
  0x300B6492, // Large 'xvmaxi|_wu'.
  0x10B893CD, // Large 'xvmepatms|k'.
  0x20105498, // Large 'xvmin|_b'.
  0x30105498, // Large 'xvmin|_bu'.
  0x20095498, // Large 'xvmin|_d'.
  0x30165498, // Large 'xvmin|_du'.
  0x200E5498, // Large 'xvmin|_h'.
  0x301C5498, // Large 'xvmin|_hu'.
  0x200B5498, // Large 'xvmin|_w'.
  0x300B5498, // Large 'xvmin|_wu'.
  0x20106498, // Large 'xvmini|_b'.
  0x30106498, // Large 'xvmini|_bu'.
  0x20096498, // Large 'xvmini|_d'.
  0x30166498, // Large 'xvmini|_du'.
  0x200E6498, // Large 'xvmini|_h'.
  0x301C6498, // Large 'xvmini|_hu'.
  0x200B6498, // Large 'xvmini|_w'.
  0x300B6498, // Large 'xvmini|_wu'.
  0x2010552B, // Large 'xvmod|_b'.
  0x3010552B, // Large 'xvmod|_bu'.
  0x2009552B, // Large 'xvmod|_d'.
  0x3016552B, // Large 'xvmod|_du'.
  0x200E552B, // Large 'xvmod|_h'.
  0x301C552B, // Large 'xvmod|_hu'.
  0x200B552B, // Large 'xvmod|_w'.
  0x300B552B, // Large 'xvmod|_wu'.
  0x201083D6, // Large 'xvmskgez|_b'.
  0x201083DE, // Large 'xvmskltz|_b'.
  0x200983DE, // Large 'xvmskltz|_d'.
  0x200E83DE, // Large 'xvmskltz|_h'.
  0x200B83DE, // Large 'xvmskltz|_w'.
  0x2010749E, // Large 'xvmsknz|_b'.
  0x318D5530, // Large 'xvmsu|b_b'.
  0x35355530, // Large 'xvmsu|b_d'.
  0x100F7530, // Large 'xvmsub_|h'.
  0x10067530, // Large 'xvmsub_|w'.
  0x20105538, // Large 'xvmuh|_b'.
  0x400F405D, // Large 'xvmu|h_bu'.
  0x20095538, // Large 'xvmuh|_d'.
  0x30165538, // Large 'xvmuh|_du'.
  0x200E5538, // Large 'xvmuh|_h'.
  0x301C5538, // Large 'xvmuh|_hu'.
  0x200B5538, // Large 'xvmuh|_w'.
  0x300B5538, // Large 'xvmuh|_wu'.
  0x2010505D, // Large 'xvmul|_b'.
  0x2009505D, // Large 'xvmul|_d'.
  0x200E505D, // Large 'xvmul|_h'.
  0x200B505D, // Large 'xvmul|_w'.
  0x0000C05D, // Large 'xvmulwev_d_w'.
  0x0000D05D, // Large 'xvmulwev_d_wu'.
  0x200BD05D, // Large 'xvmulwev_d_wu|_w'.
  0x300F905D, // Large 'xvmulwev_|h_b'.
  0x400F905D, // Large 'xvmulwev_|h_bu'.
  0x600F905D, // Large 'xvmulwev_|h_bu_b'.
  0x3015905D, // Large 'xvmulwev_|q_d'.
  0x4015905D, // Large 'xvmulwev_|q_du'.
  0x6015905D, // Large 'xvmulwev_|q_du_d'.
  0x301B905D, // Large 'xvmulwev_|w_h'.
  0x401B905D, // Large 'xvmulwev_|w_hu'.
  0x601B905D, // Large 'xvmulwev_|w_hu_h'.
  0x0000C06A, // Large 'xvmulwod_d_w'.
  0x0000D06A, // Large 'xvmulwod_d_wu'.
  0x200BD06A, // Large 'xvmulwod_d_wu|_w'.
  0x300F906A, // Large 'xvmulwod_|h_b'.
  0x400F906A, // Large 'xvmulwod_|h_bu'.
  0x600F906A, // Large 'xvmulwod_|h_bu_b'.
  0x3015906A, // Large 'xvmulwod_|q_d'.
  0x4015906A, // Large 'xvmulwod_|q_du'.
  0x6015906A, // Large 'xvmulwod_|q_du_d'.
  0x301B906A, // Large 'xvmulwod_|w_h'.
  0x401B906A, // Large 'xvmulwod_|w_hu'.
  0x601B906A, // Large 'xvmulwod_|w_hu_h'.
  0x201055B4, // Large 'xvneg|_b'.
  0x200955B4, // Large 'xvneg|_d'.
  0x200E55B4, // Large 'xvneg|_h'.
  0x200B55B4, // Large 'xvneg|_w'.
  0x8127BAD8, // Small 'xvnor'.
  0x30E7553D, // Large 'xvnor|i_b'.
  0x80093ED8, // Small 'xvor'.
  0x30E745B9, // Large 'xvor|i_b'.
  0x80E93ED8, // Small 'xvorn'.
  0x201083E6, // Large 'xvpackev|_b'.
  0x200983E6, // Large 'xvpackev|_d'.
  0x200E83E6, // Large 'xvpackev|_h'.
  0x200B83E6, // Large 'xvpackev|_w'.
  0x43EE63E6, // Large 'xvpack|od_b'.
  0x402863E6, // Large 'xvpack|od_d'.
  0x43F263E6, // Large 'xvpack|od_h'.
  0x41C963E6, // Large 'xvpack|od_w'.
  0x337B5542, // Large 'xvpcn|t_b'.
  0x31F35542, // Large 'xvpcn|t_d'.
  0x337E5542, // Large 'xvpcn|t_h'.
  0x33815542, // Large 'xvpcn|t_w'.
  0x200B64A5, // Large 'xvperm|_w'.
  0x31AA64A5, // Large 'xvperm|i_d'.
  0x34AB64A5, // Large 'xvperm|i_q'.
  0x100684A5, // Large 'xvpermi_|w'.
  0x43F660B3, // Large 'xvpick|ev_b'.
  0x400760B3, // Large 'xvpick|ev_d'.
  0x43FA60B3, // Large 'xvpick|ev_h'.
  0x43FE60B3, // Large 'xvpick|ev_w'.
  0x43EE60B3, // Large 'xvpick|od_b'.
  0x402860B3, // Large 'xvpick|od_d'.
  0x43F260B3, // Large 'xvpick|od_h'.
  0x41C960B3, // Large 'xvpick|od_w'.
  0x2009B0B3, // Large 'xvpickve2gr|_d'.
  0x3016B0B3, // Large 'xvpickve2gr|_du'.
  0x200BB0B3, // Large 'xvpickve2gr|_w'.
  0x300BB0B3, // Large 'xvpickve2gr|_wu'.
  0x200980B3, // Large 'xvpickve|_d'.
  0x200B80B3, // Large 'xvpickve|_w'.
  0x2010C0BE, // Large 'xvrepl128vei|_b'.
  0x2009C0BE, // Large 'xvrepl128vei|_d'.
  0x200EC0BE, // Large 'xvrepl128vei|_h'.
  0x200BC0BE, // Large 'xvrepl128vei|_w'.
  0x2010B163, // Large 'xvreplgr2vr|_b'.
  0x2009B163, // Large 'xvreplgr2vr|_d'.
  0x200EB163, // Large 'xvreplgr2vr|_h'.
  0x200BB163, // Large 'xvreplgr2vr|_w'.
  0x20109319, // Large 'xvreplve0|_b'.
  0x20099319, // Large 'xvreplve0|_d'.
  0x200E9319, // Large 'xvreplve0|_h'.
  0x20EC9319, // Large 'xvreplve0|_q'.
  0x200B9319, // Large 'xvreplve0|_w'.
  0x20108319, // Large 'xvreplve|_b'.
  0x20098319, // Large 'xvreplve|_d'.
  0x200E8319, // Large 'xvreplve|_h'.
  0x200B8319, // Large 'xvreplve|_w'.
  0x201064AE, // Large 'xvrotr|_b'.
  0x200964AE, // Large 'xvrotr|_d'.
  0x200E64AE, // Large 'xvrotr|_h'.
  0x200B64AE, // Large 'xvrotr|_w'.
  0x30E764AE, // Large 'xvrotr|i_b'.
  0x31AA64AE, // Large 'xvrotr|i_d'.
  0x32C064AE, // Large 'xvrotr|i_h'.
  0x32CB64AE, // Large 'xvrotr|i_w'.
  0x201064B4, // Large 'xvsadd|_b'.
  0x301064B4, // Large 'xvsadd|_bu'.
  0x200964B4, // Large 'xvsadd|_d'.
  0x301664B4, // Large 'xvsadd|_du'.
  0x200E64B4, // Large 'xvsadd|_h'.
  0x301C64B4, // Large 'xvsadd|_hu'.
  0x200B64B4, // Large 'xvsadd|_w'.
  0x300B64B4, // Large 'xvsadd|_wu'.
  0x20105547, // Large 'xvsat|_b'.
  0x30105547, // Large 'xvsat|_bu'.
  0x20095547, // Large 'xvsat|_d'.
  0x30165547, // Large 'xvsat|_du'.
  0x200E5547, // Large 'xvsat|_h'.
  0x301C5547, // Large 'xvsat|_hu'.
  0x200B5547, // Large 'xvsat|_w'.
  0x300B5547, // Large 'xvsat|_wu'.
  0x2010554C, // Large 'xvseq|_b'.
  0x2009554C, // Large 'xvseq|_d'.
  0x200E554C, // Large 'xvseq|_h'.
  0x200B554C, // Large 'xvseq|_w'.
  0x30E7554C, // Large 'xvseq|i_b'.
  0x31AA554C, // Large 'xvseq|i_d'.
  0x32C0554C, // Large 'xvseq|i_h'.
  0x32CB554C, // Large 'xvseq|i_w'.
  0x2010B16E, // Large 'xvsetallnez|_b'.
  0x2009B16E, // Large 'xvsetallnez|_d'.
  0x200EB16E, // Large 'xvsetallnez|_h'.
  0x200BB16E, // Large 'xvsetallnez|_w'.
  0x2010B179, // Large 'xvsetanyeqz|_b'.
  0x2009B179, // Large 'xvsetanyeqz|_d'.
  0x200EB179, // Large 'xvsetanyeqz|_h'.
  0x200BB179, // Large 'xvsetanyeqz|_w'.
  0x3181516E, // Large 'xvset|eqz'.
  0x3176516E, // Large 'xvset|nez'.
  0x30E77402, // Large 'xvshuf4|i_b'.
  0x31AA7402, // Large 'xvshuf4|i_d'.
  0x32C07402, // Large 'xvshuf4|i_h'.
  0x32CB7402, // Large 'xvshuf4|i_w'.
  0x20106402, // Large 'xvshuf|_b'.
  0x20096402, // Large 'xvshuf|_d'.
  0x200E6402, // Large 'xvshuf|_h'.
  0x200B6402, // Large 'xvshuf|_w'.
  0x20109322, // Large 'xvsigncov|_b'.
  0x20099322, // Large 'xvsigncov|_d'.
  0x200E9322, // Large 'xvsigncov|_h'.
  0x200B9322, // Large 'xvsigncov|_w'.
  0x201054BA, // Large 'xvsle|_b'.
  0x301054BA, // Large 'xvsle|_bu'.
  0x200954BA, // Large 'xvsle|_d'.
  0x301654BA, // Large 'xvsle|_du'.
  0x200E54BA, // Large 'xvsle|_h'.
  0x301C54BA, // Large 'xvsle|_hu'.
  0x200B54BA, // Large 'xvsle|_w'.
  0x300B54BA, // Large 'xvsle|_wu'.
  0x201064BA, // Large 'xvslei|_b'.
  0x301064BA, // Large 'xvslei|_bu'.
  0x200964BA, // Large 'xvslei|_d'.
  0x301664BA, // Large 'xvslei|_du'.
  0x200E64BA, // Large 'xvslei|_h'.
  0x301C64BA, // Large 'xvslei|_hu'.
  0x200B64BA, // Large 'xvslei|_w'.
  0x300B64BA, // Large 'xvslei|_wu'.
  0x201050CA, // Large 'xvsll|_b'.
  0x200950CA, // Large 'xvsll|_d'.
  0x200E50CA, // Large 'xvsll|_h'.
  0x200B50CA, // Large 'xvsll|_w'.
  0x30E750CA, // Large 'xvsll|i_b'.
  0x31AA50CA, // Large 'xvsll|i_d'.
  0x32C050CA, // Large 'xvsll|i_h'.
  0x32CB50CA, // Large 'xvsll|i_w'.
  0x200BA0CA, // Large 'xvsllwil_d|_w'.
  0x300BB0CA, // Large 'xvsllwil_du|_wu'.
  0x300F90CA, // Large 'xvsllwil_|h_b'.
  0x50D590CA, // Large 'xvsllwil_|hu_bu'.
  0x301B90CA, // Large 'xvsllwil_|w_h'.
  0x50DA90CA, // Large 'xvsllwil_|wu_hu'.
  0x201054C0, // Large 'xvslt|_b'.
  0x301054C0, // Large 'xvslt|_bu'.
  0x200954C0, // Large 'xvslt|_d'.
  0x301654C0, // Large 'xvslt|_du'.
  0x200E54C0, // Large 'xvslt|_h'.
  0x301C54C0, // Large 'xvslt|_hu'.
  0x200B54C0, // Large 'xvslt|_w'.
  0x300B54C0, // Large 'xvslt|_wu'.
  0x201064C0, // Large 'xvslti|_b'.
  0x301064C0, // Large 'xvslti|_bu'.
  0x200964C0, // Large 'xvslti|_d'.
  0x301664C0, // Large 'xvslti|_du'.
  0x200E64C0, // Large 'xvslti|_h'.
  0x301C64C0, // Large 'xvslti|_hu'.
  0x200B64C0, // Large 'xvslti|_w'.
  0x300B64C0, // Large 'xvslti|_wu'.
  0x20105235, // Large 'xvsra|_b'.
  0x20095235, // Large 'xvsra|_d'.
  0x200E5235, // Large 'xvsra|_h'.
  0x200B5235, // Large 'xvsra|_w'.
  0x30E75235, // Large 'xvsra|i_b'.
  0x31AA5235, // Large 'xvsra|i_d'.
  0x32C05235, // Large 'xvsra|i_h'.
  0x32CB5235, // Large 'xvsra|i_w'.
  0x4249632B, // Large 'xvsran|_b_h'.
  0x424D632B, // Large 'xvsran|_h_w'.
  0x4087632B, // Large 'xvsran|_w_d'.
  0x4249732B, // Large 'xvsrani|_b_h'.
  0x4332732B, // Large 'xvsrani|_d_q'.
  0x31A0832B, // Large 'xvsrani_|h_w'.
  0x3088832B, // Large 'xvsrani_|w_d'.
  0x20106235, // Large 'xvsrar|_b'.
  0x20096235, // Large 'xvsrar|_d'.
  0x200E6235, // Large 'xvsrar|_h'.
  0x200B6235, // Large 'xvsrar|_w'.
  0x30E76235, // Large 'xvsrar|i_b'.
  0x31AA6235, // Large 'xvsrar|i_d'.
  0x32C06235, // Large 'xvsrar|i_h'.
  0x32CB6235, // Large 'xvsrar|i_w'.
  0x42497235, // Large 'xvsrarn|_b_h'.
  0x424D7235, // Large 'xvsrarn|_h_w'.
  0x40877235, // Large 'xvsrarn|_w_d'.
  0x200EA235, // Large 'xvsrarni_b|_h'.
  0x319D9235, // Large 'xvsrarni_|d_q'.
  0x31A09235, // Large 'xvsrarni_|h_w'.
  0x30889235, // Large 'xvsrarni_|w_d'.
  0x2010523F, // Large 'xvsrl|_b'.
  0x2009523F, // Large 'xvsrl|_d'.
  0x200E523F, // Large 'xvsrl|_h'.
  0x200B523F, // Large 'xvsrl|_w'.
  0x30E7523F, // Large 'xvsrl|i_b'.
  0x31AA523F, // Large 'xvsrl|i_d'.
  0x32C0523F, // Large 'xvsrl|i_h'.
  0x32CB523F, // Large 'xvsrl|i_w'.
  0x42496336, // Large 'xvsrln|_b_h'.
  0x424D6336, // Large 'xvsrln|_h_w'.
  0x40876336, // Large 'xvsrln|_w_d'.
  0x42497336, // Large 'xvsrlni|_b_h'.
  0x43327336, // Large 'xvsrlni|_d_q'.
  0x424D7336, // Large 'xvsrlni|_h_w'.
  0x40877336, // Large 'xvsrlni|_w_d'.
  0x2010623F, // Large 'xvsrlr|_b'.
  0x2009623F, // Large 'xvsrlr|_d'.
  0x200E623F, // Large 'xvsrlr|_h'.
  0x200B623F, // Large 'xvsrlr|_w'.
  0x30E7623F, // Large 'xvsrlr|i_b'.
  0x31AA623F, // Large 'xvsrlr|i_d'.
  0x32C0623F, // Large 'xvsrlr|i_h'.
  0x32CB623F, // Large 'xvsrlr|i_w'.
  0x4249723F, // Large 'xvsrlrn|_b_h'.
  0x424D723F, // Large 'xvsrlrn|_h_w'.
  0x4087723F, // Large 'xvsrlrn|_w_d'.
  0x200EA23F, // Large 'xvsrlrni_b|_h'.
  0x319D923F, // Large 'xvsrlrni_|d_q'.
  0x31A0923F, // Large 'xvsrlrni_|h_w'.
  0x3088923F, // Large 'xvsrlrni_|w_d'.
  0x42497184, // Large 'xvssran|_b_h'.
  0x518E7184, // Large 'xvssran|_bu_h'.
  0x424D7184, // Large 'xvssran|_h_w'.
  0x51937184, // Large 'xvssran|_hu_w'.
  0x40877184, // Large 'xvssran|_w_d'.
  0x51987184, // Large 'xvssran|_wu_d'.
  0x100FB184, // Large 'xvssrani_b_|h'.
  0x300DA184, // Large 'xvssrani_b|u_h'.
  0x319D9184, // Large 'xvssrani_|d_q'.
  0x40EA9184, // Large 'xvssrani_|du_q'.
  0x31A09184, // Large 'xvssrani_|h_w'.
  0x40EE9184, // Large 'xvssrani_|hu_w'.
  0x30889184, // Large 'xvssrani_|w_d'.
  0x40F29184, // Large 'xvssrani_|wu_d'.
  0x424980DF, // Large 'xvssrarn|_b_h'.
  0x518E80DF, // Large 'xvssrarn|_bu_h'.
  0x424D80DF, // Large 'xvssrarn|_h_w'.
  0x519380DF, // Large 'xvssrarn|_hu_w'.
  0x408780DF, // Large 'xvssrarn|_w_d'.
  0x519880DF, // Large 'xvssrarn|_wu_d'.
  0x200EB0DF, // Large 'xvssrarni_b|_h'.
  0x300DB0DF, // Large 'xvssrarni_b|u_h'.
  0x319DA0DF, // Large 'xvssrarni_|d_q'.
  0x40EAA0DF, // Large 'xvssrarni_|du_q'.
  0x31A0A0DF, // Large 'xvssrarni_|h_w'.
  0x40EEA0DF, // Large 'xvssrarni_|hu_w'.
  0x3088A0DF, // Large 'xvssrarni_|w_d'.
  0x40F2A0DF, // Large 'xvssrarni_|wu_d'.
  0x424971A3, // Large 'xvssrln|_b_h'.
  0x518E71A3, // Large 'xvssrln|_bu_h'.
  0x424D71A3, // Large 'xvssrln|_h_w'.
  0x519371A3, // Large 'xvssrln|_hu_w'.
  0x408771A3, // Large 'xvssrln|_w_d'.
  0x519871A3, // Large 'xvssrln|_wu_d'.
  0x324A91A3, // Large 'xvssrlni_|b_h'.
  0x518E81A3, // Large 'xvssrlni|_bu_h'.
  0x20ECA1A3, // Large 'xvssrlni_d|_q'.
  0x51AB81A3, // Large 'xvssrlni|_du_q'.
  0x31A091A3, // Large 'xvssrlni_|h_w'.
  0x40EE91A3, // Large 'xvssrlni_|hu_w'.
  0x308891A3, // Large 'xvssrlni_|w_d'.
  0x40F291A3, // Large 'xvssrlni_|wu_d'.
  0x424980F6, // Large 'xvssrlrn|_b_h'.
  0x518E80F6, // Large 'xvssrlrn|_bu_h'.
  0x424D80F6, // Large 'xvssrlrn|_h_w'.
  0x519380F6, // Large 'xvssrlrn|_hu_w'.
  0x408780F6, // Large 'xvssrlrn|_w_d'.
  0x519880F6, // Large 'xvssrlrn|_wu_d'.
  0x200EB0F6, // Large 'xvssrlrni_b|_h'.
  0x300DB0F6, // Large 'xvssrlrni_b|u_h'.
  0x319DA0F6, // Large 'xvssrlrni_|d_q'.
  0x40EAA0F6, // Large 'xvssrlrni_|du_q'.
  0x31A0A0F6, // Large 'xvssrlrni_|h_w'.
  0x40EEA0F6, // Large 'xvssrlrni_|hu_w'.
  0x3088A0F6, // Large 'xvssrlrni_|w_d'.
  0x40F2A0F6, // Large 'xvssrlrni_|wu_d'.
  0x201064CB, // Large 'xvssub|_b'.
  0x418D54C6, // Large 'xvssu|b_bu'.
  0x200964CB, // Large 'xvssub|_d'.
  0x301664CB, // Large 'xvssub|_du'.
  0x200E64CB, // Large 'xvssub|_h'.
  0x301C64CB, // Large 'xvssub|_hu'.
  0x200B64CB, // Large 'xvssub|_w'.
  0x300B64CB, // Large 'xvssub|_wu'.
  0x800A4ED8, // Small 'xvst'.
  0x201074D1, // Large 'xvstelm|_b'.
  0x200974D1, // Large 'xvstelm|_d'.
  0x200E74D1, // Large 'xvstelm|_h'.
  0x200B74D1, // Large 'xvstelm|_w'.
  0x818A4ED8, // Small 'xvstx'.
  0x201051B0, // Large 'xvsub|_b'.
  0x200951B0, // Large 'xvsub|_d'.
  0x200E51B0, // Large 'xvsub|_h'.
  0x20EC51B0, // Large 'xvsub|_q'.
  0x200B51B0, // Large 'xvsub|_w'.
  0x301064D8, // Large 'xvsubi|_bu'.
  0x301664D8, // Large 'xvsubi|_du'.
  0x301C64D8, // Large 'xvsubi|_hu'.
  0x300B64D8, // Large 'xvsubi|_wu'.
  0x300A91B6, // Large 'xvsubwev_|d_w'.
  0x700761B0, // Large 'xvsubw|ev_d_wu'.
  0x300F91B6, // Large 'xvsubwev_|h_b'.
  0x500E81B6, // Large 'xvsubwev|_h_bu'.
  0x0000C1B6, // Large 'xvsubwev_q_d'.
  0x51BE81B6, // Large 'xvsubwev|_q_du'.
  0x301B91B6, // Large 'xvsubwev_|w_h'.
  0x401B91B6, // Large 'xvsubwev_|w_hu'.
  0x300A91C3, // Large 'xvsubwod_|d_w'.
  0x702861B0, // Large 'xvsubw|od_d_wu'.
  0x300F91C3, // Large 'xvsubwod_|h_b'.
  0x500E81C3, // Large 'xvsubwod|_h_bu'.
  0x301591C3, // Large 'xvsubwod_|q_d'.
  0x51BE81C3, // Large 'xvsubwod|_q_du'.
  0x0000C1C3, // Large 'xvsubwod_w_h'.
  0x51CB81C3, // Large 'xvsubwod|_w_hu'.
  0x8127E2D8, // Small 'xvxor'.
  0x30E75551, // Large 'xvxor|i_b'.
  0x425171D1, // Large 'vfcmp_c|af_s'.
  0x325581D1, // Large 'vfcmp_cu|n_s'.
  0x425871D1, // Large 'vfcmp_c|eq_s'.
  0x2056A1D1, // Large 'vfcmp_cueq|_s'.
  0x41DB71D1, // Large 'vfcmp_c|lt_s'.
  0x41DB81D1, // Large 'vfcmp_cu|lt_s'.
  0x41DF71D1, // Large 'vfcmp_c|le_s'.
  0x41DF81D1, // Large 'vfcmp_cu|le_s'.
  0x41E371D1, // Large 'vfcmp_c|ne_s'.
  0x425C71D1, // Large 'vfcmp_c|or_s'.
  0x41E381D1, // Large 'vfcmp_cu|ne_s'.
  0x425171E8, // Large 'vfcmp_s|af_s'.
  0x325581E8, // Large 'vfcmp_su|n_s'.
  0x425871E8, // Large 'vfcmp_s|eq_s'.
  0x2056A1E8, // Large 'vfcmp_sueq|_s'.
  0x41DB71E8, // Large 'vfcmp_s|lt_s'.
  0x41DB81E8, // Large 'vfcmp_su|lt_s'.
  0x41DF71E8, // Large 'vfcmp_s|le_s'.
  0x41DF81E8, // Large 'vfcmp_su|le_s'.
  0x41E371E8, // Large 'vfcmp_s|ne_s'.
  0x425C71E8, // Large 'vfcmp_s|or_s'.
  0x41E381E8, // Large 'vfcmp_su|ne_s'.
  0x426071D1, // Large 'vfcmp_c|af_d'.
  0x326481D1, // Large 'vfcmp_cu|n_d'.
  0x426771D1, // Large 'vfcmp_c|eq_d'.
  0x2009A1D1, // Large 'vfcmp_cueq|_d'.
  0x41F271D1, // Large 'vfcmp_c|lt_d'.
  0x41F281D1, // Large 'vfcmp_cu|lt_d'.
  0x41F671D1, // Large 'vfcmp_c|le_d'.
  0x41F681D1, // Large 'vfcmp_cu|le_d'.
  0x41FA71D1, // Large 'vfcmp_c|ne_d'.
  0x426B71D1, // Large 'vfcmp_c|or_d'.
  0x41FA81D1, // Large 'vfcmp_cu|ne_d'.
  0x426071E8, // Large 'vfcmp_s|af_d'.
  0x326481E8, // Large 'vfcmp_su|n_d'.
  0x426771E8, // Large 'vfcmp_s|eq_d'.
  0x2009A1E8, // Large 'vfcmp_sueq|_d'.
  0x41F271E8, // Large 'vfcmp_s|lt_d'.
  0x41F281E8, // Large 'vfcmp_su|lt_d'.
  0x41F671E8, // Large 'vfcmp_s|le_d'.
  0x41F681E8, // Large 'vfcmp_su|le_d'.
  0x41FA71E8, // Large 'vfcmp_s|ne_d'.
  0x426B71E8, // Large 'vfcmp_s|or_d'.
  0x41FA81E8, // Large 'vfcmp_su|ne_d'.
  0x425181D0, // Large 'xvfcmp_c|af_s'.
  0x325591D0, // Large 'xvfcmp_cu|n_s'.
  0x425881D0, // Large 'xvfcmp_c|eq_s'.
  0x2056B1D0, // Large 'xvfcmp_cueq|_s'.
  0x41DB81D0, // Large 'xvfcmp_c|lt_s'.
  0x41DB91D0, // Large 'xvfcmp_cu|lt_s'.
  0x41DF81D0, // Large 'xvfcmp_c|le_s'.
  0x41DF91D0, // Large 'xvfcmp_cu|le_s'.
  0x41E381D0, // Large 'xvfcmp_c|ne_s'.
  0x425C81D0, // Large 'xvfcmp_c|or_s'.
  0x41E391D0, // Large 'xvfcmp_cu|ne_s'.
  0x425181E7, // Large 'xvfcmp_s|af_s'.
  0x325591E7, // Large 'xvfcmp_su|n_s'.
  0x425881E7, // Large 'xvfcmp_s|eq_s'.
  0x2056B1E7, // Large 'xvfcmp_sueq|_s'.
  0x41DB81E7, // Large 'xvfcmp_s|lt_s'.
  0x41DB91E7, // Large 'xvfcmp_su|lt_s'.
  0x41DF81E7, // Large 'xvfcmp_s|le_s'.
  0x41DF91E7, // Large 'xvfcmp_su|le_s'.
  0x41E381E7, // Large 'xvfcmp_s|ne_s'.
  0x425C81E7, // Large 'xvfcmp_s|or_s'.
  0x41E391E7, // Large 'xvfcmp_su|ne_s'.
  0x426081D0, // Large 'xvfcmp_c|af_d'.
  0x326491D0, // Large 'xvfcmp_cu|n_d'.
  0x426781D0, // Large 'xvfcmp_c|eq_d'.
  0x2009B1D0, // Large 'xvfcmp_cueq|_d'.
  0x41F281D0, // Large 'xvfcmp_c|lt_d'.
  0x41F291D0, // Large 'xvfcmp_cu|lt_d'.
  0x41F681D0, // Large 'xvfcmp_c|le_d'.
  0x41F691D0, // Large 'xvfcmp_cu|le_d'.
  0x41FA81D0, // Large 'xvfcmp_c|ne_d'.
  0x426B81D0, // Large 'xvfcmp_c|or_d'.
  0x41FA91D0, // Large 'xvfcmp_cu|ne_d'.
  0x426081E7, // Large 'xvfcmp_s|af_d'.
  0x326491E7, // Large 'xvfcmp_su|n_d'.
  0x426781E7, // Large 'xvfcmp_s|eq_d'.
  0x2009B1E7, // Large 'xvfcmp_sueq|_d'.
  0x41F281E7, // Large 'xvfcmp_s|lt_d'.
  0x41F291E7, // Large 'xvfcmp_su|lt_d'.
  0x41F681E7, // Large 'xvfcmp_s|le_d'.
  0x41F691E7, // Large 'xvfcmp_su|le_d'.
  0x41FA81E7, // Large 'xvfcmp_s|ne_d'.
  0x426B81E7, // Large 'xvfcmp_s|or_d'.
  0x41FA91E7  // Large 'xvfcmp_su|ne_d'.
};
// ----------------------------------------------------------------------------
// ${NameData:End}
#endif // !ASMJIT_NO_TEXT

ASMJIT_END_SUB_NAMESPACE

#endif // !ASMJIT_NO_LOONGARCH64
