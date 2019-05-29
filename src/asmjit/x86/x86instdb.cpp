// [AsmJit]
// Machine Code Generation for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// ----------------------------------------------------------------------------
// IMPORTANT: AsmJit now uses an external instruction database to populate
// static tables within this file. Perform the following steps to regenerate
// all tables enclosed by ${...}:
//
//   1. Install node.js environment <https://nodejs.org>
//   2. Go to asmjit/tools directory
//   3. Get the latest asmdb from <https://github.com/asmjit/asmdb> and
//      copy/link the `asmdb` directory to `asmjit/tools/asmdb`.
//   4. Execute `node tablegen-x86.js`
//
// Instruction encoding and opcodes were added to the `x86inst.cpp` database
// manually in the past and they are not updated by the script as it became
// tricky. However, everything else is updated including instruction operands
// and tables required to validate them, instruction read/write information
// (including registers and flags), and all indexes to all tables.
// ----------------------------------------------------------------------------

#define ASMJIT_EXPORTS

#include "../core/build.h"
#ifdef ASMJIT_BUILD_X86

#include "../core/cpuinfo.h"
#include "../core/misc_p.h"
#include "../core/support.h"
#include "../x86/x86features.h"
#include "../x86/x86instdb_p.h"
#include "../x86/x86opcode_p.h"
#include "../x86/x86operand.h"

ASMJIT_BEGIN_SUB_NAMESPACE(x86)

// ============================================================================
// [asmjit::x86::InstDB - InstInfo]
// ============================================================================

// Instruction opcode definitions:
//   - `O` encodes X86|MMX|SSE instructions.
//   - `V` encodes VEX|XOP|EVEX instructions.
#define O_ENCODE(VEX, PREFIX, OPCODE, O, L, W, EvexW, N, TT) \
  ((PREFIX) | (OPCODE) | (O) | (L) | (W) | (EvexW) | (N) | (TT) | \
   (VEX && ((PREFIX) & Opcode::kMM_Mask) != Opcode::kMM_0F ? int(Opcode::kMM_ForceVex3) : 0))

#define O(PREFIX, OPCODE, O, LL, W, EvexW, N, TT) (O_ENCODE(0, Opcode::k##PREFIX, 0x##OPCODE, Opcode::kO_##O, Opcode::kLL_##LL, Opcode::kW_##W, Opcode::kEvex_W_##EvexW, Opcode::kCDSHL_##N, Opcode::kCDTT_##TT))
#define V(PREFIX, OPCODE, O, LL, W, EvexW, N, TT) (O_ENCODE(1, Opcode::k##PREFIX, 0x##OPCODE, Opcode::kO_##O, Opcode::kLL_##LL, Opcode::kW_##W, Opcode::kEvex_W_##EvexW, Opcode::kCDSHL_##N, Opcode::kCDTT_##TT))
#define O_FPU(PREFIX, OPCODE, O) (Opcode::kFPU_##PREFIX | (0x##OPCODE & 0xFFu) | ((0x##OPCODE >> 8) << Opcode::kFPU_2B_Shift) | Opcode::kO_##O)

// Don't store `_nameDataIndex` if instruction names are disabled. Since some
// APIs can use `_nameDataIndex` it's much safer if it's zero if it's not defined.
#ifndef ASMJIT_DISABLE_TEXT
  #define NAME_DATA_INDEX(X) X
#else
  #define NAME_DATA_INDEX(X) 0
#endif

// Defines an X86 instruction.
#define INST(id, encoding, opcode0, opcode1, nameDataIndex, commonDataIndex, operationDataIndex) { \
  uint32_t(NAME_DATA_INDEX(nameDataIndex)), \
  uint32_t(commonDataIndex),                \
  uint32_t(operationDataIndex)              \
}

const InstDB::InstInfo InstDB::_instInfoTable[] = {
  // <------------------+--------------------+------------------+--------+------------------+--------+------+----+----+
  //                    |                    |    Main Opcode   |#0 EVEX |Alternative Opcode|#1 EVEX |      |    |    |
  //     Instruction    |   Inst. Encoding   |                  +--------+                  +--------+Name-X|ComX|OpnX|
  //                    |                    |#0:PP-MMM OP/O L|W|W|N|TT. |#1:PP-MMM OP/O L|W|W|N|TT. |      |    |    |
  // <------------------+--------------------+------------------+--------+------------------+--------+------+----+----+
  //                                                                                                  (auto-generated)
  // ${InstInfo:Begin}
  INST(None             , None               , 0                         , 0                         , 0    , 0  , 0  ), // #0
  INST(Aaa              , X86Op_xAX          , O(000000,37,_,_,_,_,_,_  ), 0                         , 1    , 1  , 1  ), // #1
  INST(Aad              , X86I_xAX           , O(000000,D5,_,_,_,_,_,_  ), 0                         , 5    , 2  , 1  ), // #2
  INST(Aam              , X86I_xAX           , O(000000,D4,_,_,_,_,_,_  ), 0                         , 9    , 2  , 1  ), // #3
  INST(Aas              , X86Op_xAX          , O(000000,3F,_,_,_,_,_,_  ), 0                         , 13   , 1  , 1  ), // #4
  INST(Adc              , X86Arith           , O(000000,10,2,_,x,_,_,_  ), 0                         , 17   , 3  , 2  ), // #5
  INST(Adcx             , X86Rm              , O(660F38,F6,_,_,x,_,_,_  ), 0                         , 21   , 4  , 3  ), // #6
  INST(Add              , X86Arith           , O(000000,00,0,_,x,_,_,_  ), 0                         , 746  , 3  , 1  ), // #7
  INST(Addpd            , ExtRm              , O(660F00,58,_,_,_,_,_,_  ), 0                         , 4799 , 5  , 4  ), // #8
  INST(Addps            , ExtRm              , O(000F00,58,_,_,_,_,_,_  ), 0                         , 4811 , 5  , 5  ), // #9
  INST(Addsd            , ExtRm              , O(F20F00,58,_,_,_,_,_,_  ), 0                         , 5033 , 6  , 4  ), // #10
  INST(Addss            , ExtRm              , O(F30F00,58,_,_,_,_,_,_  ), 0                         , 2940 , 7  , 5  ), // #11
  INST(Addsubpd         , ExtRm              , O(660F00,D0,_,_,_,_,_,_  ), 0                         , 4538 , 5  , 6  ), // #12
  INST(Addsubps         , ExtRm              , O(F20F00,D0,_,_,_,_,_,_  ), 0                         , 4550 , 5  , 6  ), // #13
  INST(Adox             , X86Rm              , O(F30F38,F6,_,_,x,_,_,_  ), 0                         , 26   , 4  , 7  ), // #14
  INST(Aesdec           , ExtRm              , O(660F38,DE,_,_,_,_,_,_  ), 0                         , 2995 , 5  , 8  ), // #15
  INST(Aesdeclast       , ExtRm              , O(660F38,DF,_,_,_,_,_,_  ), 0                         , 3003 , 5  , 8  ), // #16
  INST(Aesenc           , ExtRm              , O(660F38,DC,_,_,_,_,_,_  ), 0                         , 3015 , 5  , 8  ), // #17
  INST(Aesenclast       , ExtRm              , O(660F38,DD,_,_,_,_,_,_  ), 0                         , 3023 , 5  , 8  ), // #18
  INST(Aesimc           , ExtRm              , O(660F38,DB,_,_,_,_,_,_  ), 0                         , 3035 , 5  , 8  ), // #19
  INST(Aeskeygenassist  , ExtRmi             , O(660F3A,DF,_,_,_,_,_,_  ), 0                         , 3043 , 8  , 8  ), // #20
  INST(And              , X86Arith           , O(000000,20,4,_,x,_,_,_  ), 0                         , 2418 , 9  , 1  ), // #21
  INST(Andn             , VexRvm_Wx          , V(000F38,F2,_,0,x,_,_,_  ), 0                         , 6479 , 10 , 9  ), // #22
  INST(Andnpd           , ExtRm              , O(660F00,55,_,_,_,_,_,_  ), 0                         , 3076 , 5  , 4  ), // #23
  INST(Andnps           , ExtRm              , O(000F00,55,_,_,_,_,_,_  ), 0                         , 3084 , 5  , 5  ), // #24
  INST(Andpd            , ExtRm              , O(660F00,54,_,_,_,_,_,_  ), 0                         , 4052 , 11 , 4  ), // #25
  INST(Andps            , ExtRm              , O(000F00,54,_,_,_,_,_,_  ), 0                         , 4062 , 11 , 5  ), // #26
  INST(Arpl             , X86Mr_NoSize       , O(000000,63,_,_,_,_,_,_  ), 0                         , 31   , 12 , 10 ), // #27
  INST(Bextr            , VexRmv_Wx          , V(000F38,F7,_,0,x,_,_,_  ), 0                         , 36   , 13 , 9  ), // #28
  INST(Blcfill          , VexVm_Wx           , V(XOP_M9,01,1,0,x,_,_,_  ), 0                         , 42   , 14 , 11 ), // #29
  INST(Blci             , VexVm_Wx           , V(XOP_M9,02,6,0,x,_,_,_  ), 0                         , 50   , 14 , 11 ), // #30
  INST(Blcic            , VexVm_Wx           , V(XOP_M9,01,5,0,x,_,_,_  ), 0                         , 55   , 14 , 11 ), // #31
  INST(Blcmsk           , VexVm_Wx           , V(XOP_M9,02,1,0,x,_,_,_  ), 0                         , 61   , 14 , 11 ), // #32
  INST(Blcs             , VexVm_Wx           , V(XOP_M9,01,3,0,x,_,_,_  ), 0                         , 68   , 14 , 11 ), // #33
  INST(Blendpd          , ExtRmi             , O(660F3A,0D,_,_,_,_,_,_  ), 0                         , 3162 , 8  , 12 ), // #34
  INST(Blendps          , ExtRmi             , O(660F3A,0C,_,_,_,_,_,_  ), 0                         , 3171 , 8  , 12 ), // #35
  INST(Blendvpd         , ExtRm_XMM0         , O(660F38,15,_,_,_,_,_,_  ), 0                         , 3180 , 15 , 12 ), // #36
  INST(Blendvps         , ExtRm_XMM0         , O(660F38,14,_,_,_,_,_,_  ), 0                         , 3190 , 15 , 12 ), // #37
  INST(Blsfill          , VexVm_Wx           , V(XOP_M9,01,2,0,x,_,_,_  ), 0                         , 73   , 14 , 11 ), // #38
  INST(Blsi             , VexVm_Wx           , V(000F38,F3,3,0,x,_,_,_  ), 0                         , 81   , 14 , 9  ), // #39
  INST(Blsic            , VexVm_Wx           , V(XOP_M9,01,6,0,x,_,_,_  ), 0                         , 86   , 14 , 11 ), // #40
  INST(Blsmsk           , VexVm_Wx           , V(000F38,F3,2,0,x,_,_,_  ), 0                         , 92   , 14 , 9  ), // #41
  INST(Blsr             , VexVm_Wx           , V(000F38,F3,1,0,x,_,_,_  ), 0                         , 99   , 14 , 9  ), // #42
  INST(Bndcl            , X86Rm              , O(F30F00,1A,_,_,_,_,_,_  ), 0                         , 104  , 16 , 13 ), // #43
  INST(Bndcn            , X86Rm              , O(F20F00,1B,_,_,_,_,_,_  ), 0                         , 110  , 16 , 13 ), // #44
  INST(Bndcu            , X86Rm              , O(F20F00,1A,_,_,_,_,_,_  ), 0                         , 116  , 16 , 13 ), // #45
  INST(Bndldx           , X86Rm              , O(000F00,1A,_,_,_,_,_,_  ), 0                         , 122  , 17 , 13 ), // #46
  INST(Bndmk            , X86Rm              , O(F30F00,1B,_,_,_,_,_,_  ), 0                         , 129  , 18 , 13 ), // #47
  INST(Bndmov           , X86Bndmov          , O(660F00,1A,_,_,_,_,_,_  ), O(660F00,1B,_,_,_,_,_,_  ), 135  , 19 , 13 ), // #48
  INST(Bndstx           , X86Mr              , O(000F00,1B,_,_,_,_,_,_  ), 0                         , 142  , 20 , 13 ), // #49
  INST(Bound            , X86Rm              , O(000000,62,_,_,_,_,_,_  ), 0                         , 149  , 21 , 0  ), // #50
  INST(Bsf              , X86Rm              , O(000F00,BC,_,_,x,_,_,_  ), 0                         , 155  , 22 , 1  ), // #51
  INST(Bsr              , X86Rm              , O(000F00,BD,_,_,x,_,_,_  ), 0                         , 159  , 22 , 1  ), // #52
  INST(Bswap            , X86Bswap           , O(000F00,C8,_,_,x,_,_,_  ), 0                         , 163  , 23 , 0  ), // #53
  INST(Bt               , X86Bt              , O(000F00,A3,_,_,x,_,_,_  ), O(000F00,BA,4,_,x,_,_,_  ), 169  , 24 , 14 ), // #54
  INST(Btc              , X86Bt              , O(000F00,BB,_,_,x,_,_,_  ), O(000F00,BA,7,_,x,_,_,_  ), 172  , 25 , 15 ), // #55
  INST(Btr              , X86Bt              , O(000F00,B3,_,_,x,_,_,_  ), O(000F00,BA,6,_,x,_,_,_  ), 176  , 25 , 15 ), // #56
  INST(Bts              , X86Bt              , O(000F00,AB,_,_,x,_,_,_  ), O(000F00,BA,5,_,x,_,_,_  ), 180  , 25 , 15 ), // #57
  INST(Bzhi             , VexRmv_Wx          , V(000F38,F5,_,0,x,_,_,_  ), 0                         , 184  , 13 , 16 ), // #58
  INST(Call             , X86Call            , O(000000,FF,2,_,_,_,_,_  ), 0                         , 2833 , 26 , 1  ), // #59
  INST(Cbw              , X86Op_xAX          , O(660000,98,_,_,_,_,_,_  ), 0                         , 189  , 27 , 0  ), // #60
  INST(Cdq              , X86Op_xDX_xAX      , O(000000,99,_,_,_,_,_,_  ), 0                         , 193  , 28 , 0  ), // #61
  INST(Cdqe             , X86Op_xAX          , O(000000,98,_,_,1,_,_,_  ), 0                         , 197  , 29 , 0  ), // #62
  INST(Clac             , X86Op              , O(000F01,CA,_,_,_,_,_,_  ), 0                         , 202  , 30 , 17 ), // #63
  INST(Clc              , X86Op              , O(000000,F8,_,_,_,_,_,_  ), 0                         , 207  , 30 , 18 ), // #64
  INST(Cld              , X86Op              , O(000000,FC,_,_,_,_,_,_  ), 0                         , 211  , 30 , 19 ), // #65
  INST(Cldemote         , X86M_Only          , O(000F00,1C,0,_,_,_,_,_  ), 0                         , 215  , 31 , 20 ), // #66
  INST(Clflush          , X86M_Only          , O(000F00,AE,7,_,_,_,_,_  ), 0                         , 224  , 31 , 21 ), // #67
  INST(Clflushopt       , X86M_Only          , O(660F00,AE,7,_,_,_,_,_  ), 0                         , 232  , 31 , 22 ), // #68
  INST(Clgi             , X86Op              , O(000F01,DD,_,_,_,_,_,_  ), 0                         , 243  , 30 , 23 ), // #69
  INST(Cli              , X86Op              , O(000000,FA,_,_,_,_,_,_  ), 0                         , 248  , 30 , 24 ), // #70
  INST(Clts             , X86Op              , O(000F00,06,_,_,_,_,_,_  ), 0                         , 252  , 30 , 0  ), // #71
  INST(Clwb             , X86M_Only          , O(660F00,AE,6,_,_,_,_,_  ), 0                         , 257  , 31 , 25 ), // #72
  INST(Clzero           , X86Op_MemZAX       , O(000F01,FC,_,_,_,_,_,_  ), 0                         , 262  , 32 , 26 ), // #73
  INST(Cmc              , X86Op              , O(000000,F5,_,_,_,_,_,_  ), 0                         , 269  , 30 , 27 ), // #74
  INST(Cmova            , X86Rm              , O(000F00,47,_,_,x,_,_,_  ), 0                         , 273  , 22 , 28 ), // #75
  INST(Cmovae           , X86Rm              , O(000F00,43,_,_,x,_,_,_  ), 0                         , 279  , 22 , 29 ), // #76
  INST(Cmovb            , X86Rm              , O(000F00,42,_,_,x,_,_,_  ), 0                         , 603  , 22 , 29 ), // #77
  INST(Cmovbe           , X86Rm              , O(000F00,46,_,_,x,_,_,_  ), 0                         , 610  , 22 , 28 ), // #78
  INST(Cmovc            , X86Rm              , O(000F00,42,_,_,x,_,_,_  ), 0                         , 286  , 22 , 29 ), // #79
  INST(Cmove            , X86Rm              , O(000F00,44,_,_,x,_,_,_  ), 0                         , 618  , 22 , 30 ), // #80
  INST(Cmovg            , X86Rm              , O(000F00,4F,_,_,x,_,_,_  ), 0                         , 292  , 22 , 31 ), // #81
  INST(Cmovge           , X86Rm              , O(000F00,4D,_,_,x,_,_,_  ), 0                         , 298  , 22 , 32 ), // #82
  INST(Cmovl            , X86Rm              , O(000F00,4C,_,_,x,_,_,_  ), 0                         , 305  , 22 , 32 ), // #83
  INST(Cmovle           , X86Rm              , O(000F00,4E,_,_,x,_,_,_  ), 0                         , 311  , 22 , 31 ), // #84
  INST(Cmovna           , X86Rm              , O(000F00,46,_,_,x,_,_,_  ), 0                         , 318  , 22 , 28 ), // #85
  INST(Cmovnae          , X86Rm              , O(000F00,42,_,_,x,_,_,_  ), 0                         , 325  , 22 , 29 ), // #86
  INST(Cmovnb           , X86Rm              , O(000F00,43,_,_,x,_,_,_  ), 0                         , 625  , 22 , 29 ), // #87
  INST(Cmovnbe          , X86Rm              , O(000F00,47,_,_,x,_,_,_  ), 0                         , 633  , 22 , 28 ), // #88
  INST(Cmovnc           , X86Rm              , O(000F00,43,_,_,x,_,_,_  ), 0                         , 333  , 22 , 29 ), // #89
  INST(Cmovne           , X86Rm              , O(000F00,45,_,_,x,_,_,_  ), 0                         , 642  , 22 , 30 ), // #90
  INST(Cmovng           , X86Rm              , O(000F00,4E,_,_,x,_,_,_  ), 0                         , 340  , 22 , 31 ), // #91
  INST(Cmovnge          , X86Rm              , O(000F00,4C,_,_,x,_,_,_  ), 0                         , 347  , 22 , 32 ), // #92
  INST(Cmovnl           , X86Rm              , O(000F00,4D,_,_,x,_,_,_  ), 0                         , 355  , 22 , 32 ), // #93
  INST(Cmovnle          , X86Rm              , O(000F00,4F,_,_,x,_,_,_  ), 0                         , 362  , 22 , 31 ), // #94
  INST(Cmovno           , X86Rm              , O(000F00,41,_,_,x,_,_,_  ), 0                         , 370  , 22 , 33 ), // #95
  INST(Cmovnp           , X86Rm              , O(000F00,4B,_,_,x,_,_,_  ), 0                         , 377  , 22 , 34 ), // #96
  INST(Cmovns           , X86Rm              , O(000F00,49,_,_,x,_,_,_  ), 0                         , 384  , 22 , 35 ), // #97
  INST(Cmovnz           , X86Rm              , O(000F00,45,_,_,x,_,_,_  ), 0                         , 391  , 22 , 30 ), // #98
  INST(Cmovo            , X86Rm              , O(000F00,40,_,_,x,_,_,_  ), 0                         , 398  , 22 , 33 ), // #99
  INST(Cmovp            , X86Rm              , O(000F00,4A,_,_,x,_,_,_  ), 0                         , 404  , 22 , 34 ), // #100
  INST(Cmovpe           , X86Rm              , O(000F00,4A,_,_,x,_,_,_  ), 0                         , 410  , 22 , 34 ), // #101
  INST(Cmovpo           , X86Rm              , O(000F00,4B,_,_,x,_,_,_  ), 0                         , 417  , 22 , 34 ), // #102
  INST(Cmovs            , X86Rm              , O(000F00,48,_,_,x,_,_,_  ), 0                         , 424  , 22 , 35 ), // #103
  INST(Cmovz            , X86Rm              , O(000F00,44,_,_,x,_,_,_  ), 0                         , 430  , 22 , 30 ), // #104
  INST(Cmp              , X86Arith           , O(000000,38,7,_,x,_,_,_  ), 0                         , 436  , 33 , 1  ), // #105
  INST(Cmppd            , ExtRmi             , O(660F00,C2,_,_,_,_,_,_  ), 0                         , 3416 , 8  , 4  ), // #106
  INST(Cmpps            , ExtRmi             , O(000F00,C2,_,_,_,_,_,_  ), 0                         , 3423 , 8  , 5  ), // #107
  INST(Cmps             , X86StrMm           , O(000000,A6,_,_,_,_,_,_  ), 0                         , 440  , 34 , 36 ), // #108
  INST(Cmpsd            , ExtRmi             , O(F20F00,C2,_,_,_,_,_,_  ), 0                         , 3430 , 35 , 4  ), // #109
  INST(Cmpss            , ExtRmi             , O(F30F00,C2,_,_,_,_,_,_  ), 0                         , 3437 , 36 , 5  ), // #110
  INST(Cmpxchg          , X86Cmpxchg         , O(000F00,B0,_,_,x,_,_,_  ), 0                         , 445  , 37 , 37 ), // #111
  INST(Cmpxchg16b       , X86Cmpxchg8b_16b   , O(000F00,C7,1,_,1,_,_,_  ), 0                         , 453  , 38 , 38 ), // #112
  INST(Cmpxchg8b        , X86Cmpxchg8b_16b   , O(000F00,C7,1,_,_,_,_,_  ), 0                         , 464  , 39 , 39 ), // #113
  INST(Comisd           , ExtRm              , O(660F00,2F,_,_,_,_,_,_  ), 0                         , 9915 , 6  , 40 ), // #114
  INST(Comiss           , ExtRm              , O(000F00,2F,_,_,_,_,_,_  ), 0                         , 9924 , 7  , 41 ), // #115
  INST(Cpuid            , X86Op              , O(000F00,A2,_,_,_,_,_,_  ), 0                         , 474  , 40 , 42 ), // #116
  INST(Cqo              , X86Op_xDX_xAX      , O(000000,99,_,_,1,_,_,_  ), 0                         , 480  , 41 , 0  ), // #117
  INST(Crc32            , X86Crc             , O(F20F38,F0,_,_,x,_,_,_  ), 0                         , 484  , 42 , 43 ), // #118
  INST(Cvtdq2pd         , ExtRm              , O(F30F00,E6,_,_,_,_,_,_  ), 0                         , 3484 , 6  , 4  ), // #119
  INST(Cvtdq2ps         , ExtRm              , O(000F00,5B,_,_,_,_,_,_  ), 0                         , 3494 , 5  , 4  ), // #120
  INST(Cvtpd2dq         , ExtRm              , O(F20F00,E6,_,_,_,_,_,_  ), 0                         , 3533 , 5  , 4  ), // #121
  INST(Cvtpd2pi         , ExtRm              , O(660F00,2D,_,_,_,_,_,_  ), 0                         , 490  , 43 , 4  ), // #122
  INST(Cvtpd2ps         , ExtRm              , O(660F00,5A,_,_,_,_,_,_  ), 0                         , 3543 , 5  , 4  ), // #123
  INST(Cvtpi2pd         , ExtRm              , O(660F00,2A,_,_,_,_,_,_  ), 0                         , 499  , 44 , 4  ), // #124
  INST(Cvtpi2ps         , ExtRm              , O(000F00,2A,_,_,_,_,_,_  ), 0                         , 508  , 44 , 5  ), // #125
  INST(Cvtps2dq         , ExtRm              , O(660F00,5B,_,_,_,_,_,_  ), 0                         , 3595 , 5  , 4  ), // #126
  INST(Cvtps2pd         , ExtRm              , O(000F00,5A,_,_,_,_,_,_  ), 0                         , 3605 , 6  , 4  ), // #127
  INST(Cvtps2pi         , ExtRm              , O(000F00,2D,_,_,_,_,_,_  ), 0                         , 517  , 45 , 5  ), // #128
  INST(Cvtsd2si         , ExtRm_Wx           , O(F20F00,2D,_,_,x,_,_,_  ), 0                         , 3677 , 46 , 4  ), // #129
  INST(Cvtsd2ss         , ExtRm              , O(F20F00,5A,_,_,_,_,_,_  ), 0                         , 3687 , 6  , 4  ), // #130
  INST(Cvtsi2sd         , ExtRm_Wx           , O(F20F00,2A,_,_,x,_,_,_  ), 0                         , 3708 , 47 , 4  ), // #131
  INST(Cvtsi2ss         , ExtRm_Wx           , O(F30F00,2A,_,_,x,_,_,_  ), 0                         , 3718 , 47 , 5  ), // #132
  INST(Cvtss2sd         , ExtRm              , O(F30F00,5A,_,_,_,_,_,_  ), 0                         , 3728 , 7  , 4  ), // #133
  INST(Cvtss2si         , ExtRm_Wx           , O(F30F00,2D,_,_,x,_,_,_  ), 0                         , 3738 , 48 , 5  ), // #134
  INST(Cvttpd2dq        , ExtRm              , O(660F00,E6,_,_,_,_,_,_  ), 0                         , 3759 , 5  , 4  ), // #135
  INST(Cvttpd2pi        , ExtRm              , O(660F00,2C,_,_,_,_,_,_  ), 0                         , 526  , 43 , 4  ), // #136
  INST(Cvttps2dq        , ExtRm              , O(F30F00,5B,_,_,_,_,_,_  ), 0                         , 3805 , 5  , 4  ), // #137
  INST(Cvttps2pi        , ExtRm              , O(000F00,2C,_,_,_,_,_,_  ), 0                         , 536  , 45 , 5  ), // #138
  INST(Cvttsd2si        , ExtRm_Wx           , O(F20F00,2C,_,_,x,_,_,_  ), 0                         , 3851 , 46 , 4  ), // #139
  INST(Cvttss2si        , ExtRm_Wx           , O(F30F00,2C,_,_,x,_,_,_  ), 0                         , 3874 , 48 , 5  ), // #140
  INST(Cwd              , X86Op_xDX_xAX      , O(660000,99,_,_,_,_,_,_  ), 0                         , 546  , 49 , 0  ), // #141
  INST(Cwde             , X86Op_xAX          , O(000000,98,_,_,_,_,_,_  ), 0                         , 550  , 50 , 0  ), // #142
  INST(Daa              , X86Op              , O(000000,27,_,_,_,_,_,_  ), 0                         , 555  , 1  , 1  ), // #143
  INST(Das              , X86Op              , O(000000,2F,_,_,_,_,_,_  ), 0                         , 559  , 1  , 1  ), // #144
  INST(Dec              , X86IncDec          , O(000000,FE,1,_,x,_,_,_  ), O(000000,48,_,_,x,_,_,_  ), 2998 , 51 , 44 ), // #145
  INST(Div              , X86M_GPB_MulDiv    , O(000000,F6,6,_,x,_,_,_  ), 0                         , 765  , 52 , 1  ), // #146
  INST(Divpd            , ExtRm              , O(660F00,5E,_,_,_,_,_,_  ), 0                         , 3973 , 5  , 4  ), // #147
  INST(Divps            , ExtRm              , O(000F00,5E,_,_,_,_,_,_  ), 0                         , 3980 , 5  , 5  ), // #148
  INST(Divsd            , ExtRm              , O(F20F00,5E,_,_,_,_,_,_  ), 0                         , 3987 , 6  , 4  ), // #149
  INST(Divss            , ExtRm              , O(F30F00,5E,_,_,_,_,_,_  ), 0                         , 3994 , 7  , 5  ), // #150
  INST(Dppd             , ExtRmi             , O(660F3A,41,_,_,_,_,_,_  ), 0                         , 4011 , 8  , 12 ), // #151
  INST(Dpps             , ExtRmi             , O(660F3A,40,_,_,_,_,_,_  ), 0                         , 4017 , 8  , 12 ), // #152
  INST(Emms             , X86Op              , O(000F00,77,_,_,_,_,_,_  ), 0                         , 733  , 53 , 45 ), // #153
  INST(Enter            , X86Enter           , O(000000,C8,_,_,_,_,_,_  ), 0                         , 2841 , 54 , 0  ), // #154
  INST(Extractps        , ExtExtract         , O(660F3A,17,_,_,_,_,_,_  ), 0                         , 4207 , 55 , 12 ), // #155
  INST(Extrq            , ExtExtrq           , O(660F00,79,_,_,_,_,_,_  ), O(660F00,78,0,_,_,_,_,_  ), 7275 , 56 , 46 ), // #156
  INST(F2xm1            , FpuOp              , O_FPU(00,D9F0,_)          , 0                         , 563  , 30 , 47 ), // #157
  INST(Fabs             , FpuOp              , O_FPU(00,D9E1,_)          , 0                         , 569  , 30 , 47 ), // #158
  INST(Fadd             , FpuArith           , O_FPU(00,C0C0,0)          , 0                         , 2052 , 57 , 47 ), // #159
  INST(Faddp            , FpuRDef            , O_FPU(00,DEC0,_)          , 0                         , 574  , 58 , 47 ), // #160
  INST(Fbld             , X86M_Only          , O_FPU(00,00DF,4)          , 0                         , 580  , 59 , 47 ), // #161
  INST(Fbstp            , X86M_Only          , O_FPU(00,00DF,6)          , 0                         , 585  , 59 , 47 ), // #162
  INST(Fchs             , FpuOp              , O_FPU(00,D9E0,_)          , 0                         , 591  , 30 , 47 ), // #163
  INST(Fclex            , FpuOp              , O_FPU(9B,DBE2,_)          , 0                         , 596  , 30 , 47 ), // #164
  INST(Fcmovb           , FpuR               , O_FPU(00,DAC0,_)          , 0                         , 602  , 60 , 48 ), // #165
  INST(Fcmovbe          , FpuR               , O_FPU(00,DAD0,_)          , 0                         , 609  , 60 , 48 ), // #166
  INST(Fcmove           , FpuR               , O_FPU(00,DAC8,_)          , 0                         , 617  , 60 , 48 ), // #167
  INST(Fcmovnb          , FpuR               , O_FPU(00,DBC0,_)          , 0                         , 624  , 60 , 48 ), // #168
  INST(Fcmovnbe         , FpuR               , O_FPU(00,DBD0,_)          , 0                         , 632  , 60 , 48 ), // #169
  INST(Fcmovne          , FpuR               , O_FPU(00,DBC8,_)          , 0                         , 641  , 60 , 48 ), // #170
  INST(Fcmovnu          , FpuR               , O_FPU(00,DBD8,_)          , 0                         , 649  , 60 , 48 ), // #171
  INST(Fcmovu           , FpuR               , O_FPU(00,DAD8,_)          , 0                         , 657  , 60 , 48 ), // #172
  INST(Fcom             , FpuCom             , O_FPU(00,D0D0,2)          , 0                         , 664  , 61 , 47 ), // #173
  INST(Fcomi            , FpuR               , O_FPU(00,DBF0,_)          , 0                         , 669  , 60 , 49 ), // #174
  INST(Fcomip           , FpuR               , O_FPU(00,DFF0,_)          , 0                         , 675  , 60 , 49 ), // #175
  INST(Fcomp            , FpuCom             , O_FPU(00,D8D8,3)          , 0                         , 682  , 61 , 47 ), // #176
  INST(Fcompp           , FpuOp              , O_FPU(00,DED9,_)          , 0                         , 688  , 30 , 47 ), // #177
  INST(Fcos             , FpuOp              , O_FPU(00,D9FF,_)          , 0                         , 695  , 30 , 47 ), // #178
  INST(Fdecstp          , FpuOp              , O_FPU(00,D9F6,_)          , 0                         , 700  , 30 , 47 ), // #179
  INST(Fdiv             , FpuArith           , O_FPU(00,F0F8,6)          , 0                         , 708  , 57 , 47 ), // #180
  INST(Fdivp            , FpuRDef            , O_FPU(00,DEF8,_)          , 0                         , 713  , 58 , 47 ), // #181
  INST(Fdivr            , FpuArith           , O_FPU(00,F8F0,7)          , 0                         , 719  , 57 , 47 ), // #182
  INST(Fdivrp           , FpuRDef            , O_FPU(00,DEF0,_)          , 0                         , 725  , 58 , 47 ), // #183
  INST(Femms            , X86Op              , O(000F00,0E,_,_,_,_,_,_  ), 0                         , 732  , 30 , 50 ), // #184
  INST(Ffree            , FpuR               , O_FPU(00,DDC0,_)          , 0                         , 738  , 60 , 47 ), // #185
  INST(Fiadd            , FpuM               , O_FPU(00,00DA,0)          , 0                         , 744  , 62 , 47 ), // #186
  INST(Ficom            , FpuM               , O_FPU(00,00DA,2)          , 0                         , 750  , 62 , 47 ), // #187
  INST(Ficomp           , FpuM               , O_FPU(00,00DA,3)          , 0                         , 756  , 62 , 47 ), // #188
  INST(Fidiv            , FpuM               , O_FPU(00,00DA,6)          , 0                         , 763  , 62 , 47 ), // #189
  INST(Fidivr           , FpuM               , O_FPU(00,00DA,7)          , 0                         , 769  , 62 , 47 ), // #190
  INST(Fild             , FpuM               , O_FPU(00,00DB,0)          , O_FPU(00,00DF,5)          , 776  , 63 , 47 ), // #191
  INST(Fimul            , FpuM               , O_FPU(00,00DA,1)          , 0                         , 781  , 62 , 47 ), // #192
  INST(Fincstp          , FpuOp              , O_FPU(00,D9F7,_)          , 0                         , 787  , 30 , 47 ), // #193
  INST(Finit            , FpuOp              , O_FPU(9B,DBE3,_)          , 0                         , 795  , 30 , 47 ), // #194
  INST(Fist             , FpuM               , O_FPU(00,00DB,2)          , 0                         , 801  , 62 , 47 ), // #195
  INST(Fistp            , FpuM               , O_FPU(00,00DB,3)          , O_FPU(00,00DF,7)          , 806  , 63 , 47 ), // #196
  INST(Fisttp           , FpuM               , O_FPU(00,00DB,1)          , O_FPU(00,00DD,1)          , 812  , 63 , 51 ), // #197
  INST(Fisub            , FpuM               , O_FPU(00,00DA,4)          , 0                         , 819  , 62 , 47 ), // #198
  INST(Fisubr           , FpuM               , O_FPU(00,00DA,5)          , 0                         , 825  , 62 , 47 ), // #199
  INST(Fld              , FpuFldFst          , O_FPU(00,00D9,0)          , O_FPU(00,00DB,5)          , 832  , 64 , 47 ), // #200
  INST(Fld1             , FpuOp              , O_FPU(00,D9E8,_)          , 0                         , 836  , 30 , 47 ), // #201
  INST(Fldcw            , X86M_Only          , O_FPU(00,00D9,5)          , 0                         , 841  , 65 , 47 ), // #202
  INST(Fldenv           , X86M_Only          , O_FPU(00,00D9,4)          , 0                         , 847  , 31 , 47 ), // #203
  INST(Fldl2e           , FpuOp              , O_FPU(00,D9EA,_)          , 0                         , 854  , 30 , 47 ), // #204
  INST(Fldl2t           , FpuOp              , O_FPU(00,D9E9,_)          , 0                         , 861  , 30 , 47 ), // #205
  INST(Fldlg2           , FpuOp              , O_FPU(00,D9EC,_)          , 0                         , 868  , 30 , 47 ), // #206
  INST(Fldln2           , FpuOp              , O_FPU(00,D9ED,_)          , 0                         , 875  , 30 , 47 ), // #207
  INST(Fldpi            , FpuOp              , O_FPU(00,D9EB,_)          , 0                         , 882  , 30 , 47 ), // #208
  INST(Fldz             , FpuOp              , O_FPU(00,D9EE,_)          , 0                         , 888  , 30 , 47 ), // #209
  INST(Fmul             , FpuArith           , O_FPU(00,C8C8,1)          , 0                         , 2094 , 57 , 47 ), // #210
  INST(Fmulp            , FpuRDef            , O_FPU(00,DEC8,_)          , 0                         , 893  , 58 , 47 ), // #211
  INST(Fnclex           , FpuOp              , O_FPU(00,DBE2,_)          , 0                         , 899  , 30 , 47 ), // #212
  INST(Fninit           , FpuOp              , O_FPU(00,DBE3,_)          , 0                         , 906  , 30 , 47 ), // #213
  INST(Fnop             , FpuOp              , O_FPU(00,D9D0,_)          , 0                         , 913  , 30 , 47 ), // #214
  INST(Fnsave           , X86M_Only          , O_FPU(00,00DD,6)          , 0                         , 918  , 31 , 47 ), // #215
  INST(Fnstcw           , X86M_Only          , O_FPU(00,00D9,7)          , 0                         , 925  , 65 , 47 ), // #216
  INST(Fnstenv          , X86M_Only          , O_FPU(00,00D9,6)          , 0                         , 932  , 31 , 47 ), // #217
  INST(Fnstsw           , FpuStsw            , O_FPU(00,00DD,7)          , O_FPU(00,DFE0,_)          , 940  , 66 , 47 ), // #218
  INST(Fpatan           , FpuOp              , O_FPU(00,D9F3,_)          , 0                         , 947  , 30 , 47 ), // #219
  INST(Fprem            , FpuOp              , O_FPU(00,D9F8,_)          , 0                         , 954  , 30 , 47 ), // #220
  INST(Fprem1           , FpuOp              , O_FPU(00,D9F5,_)          , 0                         , 960  , 30 , 47 ), // #221
  INST(Fptan            , FpuOp              , O_FPU(00,D9F2,_)          , 0                         , 967  , 30 , 47 ), // #222
  INST(Frndint          , FpuOp              , O_FPU(00,D9FC,_)          , 0                         , 973  , 30 , 47 ), // #223
  INST(Frstor           , X86M_Only          , O_FPU(00,00DD,4)          , 0                         , 981  , 31 , 47 ), // #224
  INST(Fsave            , X86M_Only          , O_FPU(9B,00DD,6)          , 0                         , 988  , 31 , 47 ), // #225
  INST(Fscale           , FpuOp              , O_FPU(00,D9FD,_)          , 0                         , 994  , 30 , 47 ), // #226
  INST(Fsin             , FpuOp              , O_FPU(00,D9FE,_)          , 0                         , 1001 , 30 , 47 ), // #227
  INST(Fsincos          , FpuOp              , O_FPU(00,D9FB,_)          , 0                         , 1006 , 30 , 47 ), // #228
  INST(Fsqrt            , FpuOp              , O_FPU(00,D9FA,_)          , 0                         , 1014 , 30 , 47 ), // #229
  INST(Fst              , FpuFldFst          , O_FPU(00,00D9,2)          , 0                         , 1020 , 67 , 47 ), // #230
  INST(Fstcw            , X86M_Only          , O_FPU(9B,00D9,7)          , 0                         , 1024 , 65 , 47 ), // #231
  INST(Fstenv           , X86M_Only          , O_FPU(9B,00D9,6)          , 0                         , 1030 , 31 , 47 ), // #232
  INST(Fstp             , FpuFldFst          , O_FPU(00,00D9,3)          , O(000000,DB,7,_,_,_,_,_  ), 1037 , 64 , 47 ), // #233
  INST(Fstsw            , FpuStsw            , O_FPU(9B,00DD,7)          , O_FPU(9B,DFE0,_)          , 1042 , 66 , 47 ), // #234
  INST(Fsub             , FpuArith           , O_FPU(00,E0E8,4)          , 0                         , 2172 , 57 , 47 ), // #235
  INST(Fsubp            , FpuRDef            , O_FPU(00,DEE8,_)          , 0                         , 1048 , 58 , 47 ), // #236
  INST(Fsubr            , FpuArith           , O_FPU(00,E8E0,5)          , 0                         , 2178 , 57 , 47 ), // #237
  INST(Fsubrp           , FpuRDef            , O_FPU(00,DEE0,_)          , 0                         , 1054 , 58 , 47 ), // #238
  INST(Ftst             , FpuOp              , O_FPU(00,D9E4,_)          , 0                         , 1061 , 30 , 47 ), // #239
  INST(Fucom            , FpuRDef            , O_FPU(00,DDE0,_)          , 0                         , 1066 , 58 , 47 ), // #240
  INST(Fucomi           , FpuR               , O_FPU(00,DBE8,_)          , 0                         , 1072 , 60 , 49 ), // #241
  INST(Fucomip          , FpuR               , O_FPU(00,DFE8,_)          , 0                         , 1079 , 60 , 49 ), // #242
  INST(Fucomp           , FpuRDef            , O_FPU(00,DDE8,_)          , 0                         , 1087 , 58 , 47 ), // #243
  INST(Fucompp          , FpuOp              , O_FPU(00,DAE9,_)          , 0                         , 1094 , 30 , 47 ), // #244
  INST(Fwait            , X86Op              , O_FPU(00,00DB,_)          , 0                         , 1102 , 30 , 47 ), // #245
  INST(Fxam             , FpuOp              , O_FPU(00,D9E5,_)          , 0                         , 1108 , 30 , 47 ), // #246
  INST(Fxch             , FpuR               , O_FPU(00,D9C8,_)          , 0                         , 1113 , 58 , 47 ), // #247
  INST(Fxrstor          , X86M_Only          , O(000F00,AE,1,_,_,_,_,_  ), 0                         , 1118 , 31 , 52 ), // #248
  INST(Fxrstor64        , X86M_Only          , O(000F00,AE,1,_,1,_,_,_  ), 0                         , 1126 , 68 , 52 ), // #249
  INST(Fxsave           , X86M_Only          , O(000F00,AE,0,_,_,_,_,_  ), 0                         , 1136 , 31 , 53 ), // #250
  INST(Fxsave64         , X86M_Only          , O(000F00,AE,0,_,1,_,_,_  ), 0                         , 1143 , 68 , 53 ), // #251
  INST(Fxtract          , FpuOp              , O_FPU(00,D9F4,_)          , 0                         , 1152 , 30 , 47 ), // #252
  INST(Fyl2x            , FpuOp              , O_FPU(00,D9F1,_)          , 0                         , 1160 , 30 , 47 ), // #253
  INST(Fyl2xp1          , FpuOp              , O_FPU(00,D9F9,_)          , 0                         , 1166 , 30 , 47 ), // #254
  INST(Getsec           , X86Op              , O(000F00,37,_,_,_,_,_,_  ), 0                         , 1174 , 30 , 54 ), // #255
  INST(Gf2p8affineinvqb , ExtRmi             , O(660F3A,CF,_,_,_,_,_,_  ), 0                         , 5562 , 8  , 55 ), // #256
  INST(Gf2p8affineqb    , ExtRmi             , O(660F3A,CE,_,_,_,_,_,_  ), 0                         , 5580 , 8  , 55 ), // #257
  INST(Gf2p8mulb        , ExtRm              , O(660F38,CF,_,_,_,_,_,_  ), 0                         , 5595 , 5  , 55 ), // #258
  INST(Haddpd           , ExtRm              , O(660F00,7C,_,_,_,_,_,_  ), 0                         , 5606 , 5  , 6  ), // #259
  INST(Haddps           , ExtRm              , O(F20F00,7C,_,_,_,_,_,_  ), 0                         , 5614 , 5  , 6  ), // #260
  INST(Hlt              , X86Op              , O(000000,F4,_,_,_,_,_,_  ), 0                         , 1181 , 30 , 0  ), // #261
  INST(Hsubpd           , ExtRm              , O(660F00,7D,_,_,_,_,_,_  ), 0                         , 5622 , 5  , 6  ), // #262
  INST(Hsubps           , ExtRm              , O(F20F00,7D,_,_,_,_,_,_  ), 0                         , 5630 , 5  , 6  ), // #263
  INST(Idiv             , X86M_GPB_MulDiv    , O(000000,F6,7,_,x,_,_,_  ), 0                         , 764  , 52 , 1  ), // #264
  INST(Imul             , X86Imul            , O(000000,F6,5,_,x,_,_,_  ), 0                         , 782  , 69 , 1  ), // #265
  INST(In               , X86In              , O(000000,EC,_,_,_,_,_,_  ), O(000000,E4,_,_,_,_,_,_  ), 10061, 70 , 0  ), // #266
  INST(Inc              , X86IncDec          , O(000000,FE,0,_,x,_,_,_  ), O(000000,40,_,_,x,_,_,_  ), 1185 , 51 , 44 ), // #267
  INST(Ins              , X86Ins             , O(000000,6C,_,_,_,_,_,_  ), 0                         , 1842 , 71 , 0  ), // #268
  INST(Insertps         , ExtRmi             , O(660F3A,21,_,_,_,_,_,_  ), 0                         , 5766 , 36 , 12 ), // #269
  INST(Insertq          , ExtInsertq         , O(F20F00,79,_,_,_,_,_,_  ), O(F20F00,78,_,_,_,_,_,_  ), 1189 , 72 , 46 ), // #270
  INST(Int              , X86Int             , O(000000,CD,_,_,_,_,_,_  ), 0                         , 977  , 73 , 0  ), // #271
  INST(Int3             , X86Op              , O(000000,CC,_,_,_,_,_,_  ), 0                         , 1197 , 30 , 0  ), // #272
  INST(Into             , X86Op              , O(000000,CE,_,_,_,_,_,_  ), 0                         , 1202 , 74 , 56 ), // #273
  INST(Invd             , X86Op              , O(000F00,08,_,_,_,_,_,_  ), 0                         , 10016, 30 , 42 ), // #274
  INST(Invept           , X86Rm_NoSize       , O(660F38,80,_,_,_,_,_,_  ), 0                         , 1207 , 75 , 57 ), // #275
  INST(Invlpg           , X86M_Only          , O(000F00,01,7,_,_,_,_,_  ), 0                         , 1214 , 31 , 42 ), // #276
  INST(Invlpga          , X86Op_xAddr        , O(000F01,DF,_,_,_,_,_,_  ), 0                         , 1221 , 76 , 23 ), // #277
  INST(Invpcid          , X86Rm_NoSize       , O(660F38,82,_,_,_,_,_,_  ), 0                         , 1229 , 75 , 42 ), // #278
  INST(Invvpid          , X86Rm_NoSize       , O(660F38,81,_,_,_,_,_,_  ), 0                         , 1237 , 75 , 57 ), // #279
  INST(Iret             , X86Op              , O(000000,CF,_,_,_,_,_,_  ), 0                         , 1245 , 77 , 1  ), // #280
  INST(Iretd            , X86Op              , O(000000,CF,_,_,_,_,_,_  ), 0                         , 1250 , 77 , 1  ), // #281
  INST(Iretq            , X86Op              , O(000000,CF,_,_,1,_,_,_  ), 0                         , 1256 , 78 , 1  ), // #282
  INST(Iretw            , X86Op              , O(660000,CF,_,_,_,_,_,_  ), 0                         , 1262 , 77 , 1  ), // #283
  INST(Ja               , X86Jcc             , O(000F00,87,_,_,_,_,_,_  ), O(000000,77,_,_,_,_,_,_  ), 1268 , 79 , 58 ), // #284
  INST(Jae              , X86Jcc             , O(000F00,83,_,_,_,_,_,_  ), O(000000,73,_,_,_,_,_,_  ), 1271 , 79 , 59 ), // #285
  INST(Jb               , X86Jcc             , O(000F00,82,_,_,_,_,_,_  ), O(000000,72,_,_,_,_,_,_  ), 1275 , 79 , 59 ), // #286
  INST(Jbe              , X86Jcc             , O(000F00,86,_,_,_,_,_,_  ), O(000000,76,_,_,_,_,_,_  ), 1278 , 79 , 58 ), // #287
  INST(Jc               , X86Jcc             , O(000F00,82,_,_,_,_,_,_  ), O(000000,72,_,_,_,_,_,_  ), 1282 , 79 , 59 ), // #288
  INST(Je               , X86Jcc             , O(000F00,84,_,_,_,_,_,_  ), O(000000,74,_,_,_,_,_,_  ), 1285 , 79 , 60 ), // #289
  INST(Jecxz            , X86JecxzLoop       , 0                         , O(000000,E3,_,_,_,_,_,_  ), 1288 , 80 , 0  ), // #290
  INST(Jg               , X86Jcc             , O(000F00,8F,_,_,_,_,_,_  ), O(000000,7F,_,_,_,_,_,_  ), 1294 , 79 , 61 ), // #291
  INST(Jge              , X86Jcc             , O(000F00,8D,_,_,_,_,_,_  ), O(000000,7D,_,_,_,_,_,_  ), 1297 , 79 , 62 ), // #292
  INST(Jl               , X86Jcc             , O(000F00,8C,_,_,_,_,_,_  ), O(000000,7C,_,_,_,_,_,_  ), 1301 , 79 , 62 ), // #293
  INST(Jle              , X86Jcc             , O(000F00,8E,_,_,_,_,_,_  ), O(000000,7E,_,_,_,_,_,_  ), 1304 , 79 , 61 ), // #294
  INST(Jmp              , X86Jmp             , O(000000,FF,4,_,_,_,_,_  ), O(000000,EB,_,_,_,_,_,_  ), 1308 , 81 , 0  ), // #295
  INST(Jna              , X86Jcc             , O(000F00,86,_,_,_,_,_,_  ), O(000000,76,_,_,_,_,_,_  ), 1312 , 79 , 58 ), // #296
  INST(Jnae             , X86Jcc             , O(000F00,82,_,_,_,_,_,_  ), O(000000,72,_,_,_,_,_,_  ), 1316 , 79 , 59 ), // #297
  INST(Jnb              , X86Jcc             , O(000F00,83,_,_,_,_,_,_  ), O(000000,73,_,_,_,_,_,_  ), 1321 , 79 , 59 ), // #298
  INST(Jnbe             , X86Jcc             , O(000F00,87,_,_,_,_,_,_  ), O(000000,77,_,_,_,_,_,_  ), 1325 , 79 , 58 ), // #299
  INST(Jnc              , X86Jcc             , O(000F00,83,_,_,_,_,_,_  ), O(000000,73,_,_,_,_,_,_  ), 1330 , 79 , 59 ), // #300
  INST(Jne              , X86Jcc             , O(000F00,85,_,_,_,_,_,_  ), O(000000,75,_,_,_,_,_,_  ), 1334 , 79 , 60 ), // #301
  INST(Jng              , X86Jcc             , O(000F00,8E,_,_,_,_,_,_  ), O(000000,7E,_,_,_,_,_,_  ), 1338 , 79 , 61 ), // #302
  INST(Jnge             , X86Jcc             , O(000F00,8C,_,_,_,_,_,_  ), O(000000,7C,_,_,_,_,_,_  ), 1342 , 79 , 62 ), // #303
  INST(Jnl              , X86Jcc             , O(000F00,8D,_,_,_,_,_,_  ), O(000000,7D,_,_,_,_,_,_  ), 1347 , 79 , 62 ), // #304
  INST(Jnle             , X86Jcc             , O(000F00,8F,_,_,_,_,_,_  ), O(000000,7F,_,_,_,_,_,_  ), 1351 , 79 , 61 ), // #305
  INST(Jno              , X86Jcc             , O(000F00,81,_,_,_,_,_,_  ), O(000000,71,_,_,_,_,_,_  ), 1356 , 79 , 56 ), // #306
  INST(Jnp              , X86Jcc             , O(000F00,8B,_,_,_,_,_,_  ), O(000000,7B,_,_,_,_,_,_  ), 1360 , 79 , 63 ), // #307
  INST(Jns              , X86Jcc             , O(000F00,89,_,_,_,_,_,_  ), O(000000,79,_,_,_,_,_,_  ), 1364 , 79 , 64 ), // #308
  INST(Jnz              , X86Jcc             , O(000F00,85,_,_,_,_,_,_  ), O(000000,75,_,_,_,_,_,_  ), 1368 , 79 , 60 ), // #309
  INST(Jo               , X86Jcc             , O(000F00,80,_,_,_,_,_,_  ), O(000000,70,_,_,_,_,_,_  ), 1372 , 79 , 56 ), // #310
  INST(Jp               , X86Jcc             , O(000F00,8A,_,_,_,_,_,_  ), O(000000,7A,_,_,_,_,_,_  ), 1375 , 79 , 63 ), // #311
  INST(Jpe              , X86Jcc             , O(000F00,8A,_,_,_,_,_,_  ), O(000000,7A,_,_,_,_,_,_  ), 1378 , 79 , 63 ), // #312
  INST(Jpo              , X86Jcc             , O(000F00,8B,_,_,_,_,_,_  ), O(000000,7B,_,_,_,_,_,_  ), 1382 , 79 , 63 ), // #313
  INST(Js               , X86Jcc             , O(000F00,88,_,_,_,_,_,_  ), O(000000,78,_,_,_,_,_,_  ), 1386 , 79 , 64 ), // #314
  INST(Jz               , X86Jcc             , O(000F00,84,_,_,_,_,_,_  ), O(000000,74,_,_,_,_,_,_  ), 1389 , 79 , 60 ), // #315
  INST(Kaddb            , VexRvm             , V(660F00,4A,_,1,0,_,_,_  ), 0                         , 1392 , 82 , 65 ), // #316
  INST(Kaddd            , VexRvm             , V(660F00,4A,_,1,1,_,_,_  ), 0                         , 1398 , 82 , 66 ), // #317
  INST(Kaddq            , VexRvm             , V(000F00,4A,_,1,1,_,_,_  ), 0                         , 1404 , 82 , 66 ), // #318
  INST(Kaddw            , VexRvm             , V(000F00,4A,_,1,0,_,_,_  ), 0                         , 1410 , 82 , 65 ), // #319
  INST(Kandb            , VexRvm             , V(660F00,41,_,1,0,_,_,_  ), 0                         , 1416 , 82 , 65 ), // #320
  INST(Kandd            , VexRvm             , V(660F00,41,_,1,1,_,_,_  ), 0                         , 1422 , 82 , 66 ), // #321
  INST(Kandnb           , VexRvm             , V(660F00,42,_,1,0,_,_,_  ), 0                         , 1428 , 82 , 65 ), // #322
  INST(Kandnd           , VexRvm             , V(660F00,42,_,1,1,_,_,_  ), 0                         , 1435 , 82 , 66 ), // #323
  INST(Kandnq           , VexRvm             , V(000F00,42,_,1,1,_,_,_  ), 0                         , 1442 , 82 , 66 ), // #324
  INST(Kandnw           , VexRvm             , V(000F00,42,_,1,0,_,_,_  ), 0                         , 1449 , 82 , 67 ), // #325
  INST(Kandq            , VexRvm             , V(000F00,41,_,1,1,_,_,_  ), 0                         , 1456 , 82 , 66 ), // #326
  INST(Kandw            , VexRvm             , V(000F00,41,_,1,0,_,_,_  ), 0                         , 1462 , 82 , 67 ), // #327
  INST(Kmovb            , VexKmov            , V(660F00,90,_,0,0,_,_,_  ), V(660F00,92,_,0,0,_,_,_  ), 1468 , 83 , 65 ), // #328
  INST(Kmovd            , VexKmov            , V(660F00,90,_,0,1,_,_,_  ), V(F20F00,92,_,0,0,_,_,_  ), 7755 , 84 , 66 ), // #329
  INST(Kmovq            , VexKmov            , V(000F00,90,_,0,1,_,_,_  ), V(F20F00,92,_,0,1,_,_,_  ), 7766 , 85 , 66 ), // #330
  INST(Kmovw            , VexKmov            , V(000F00,90,_,0,0,_,_,_  ), V(000F00,92,_,0,0,_,_,_  ), 1474 , 86 , 67 ), // #331
  INST(Knotb            , VexRm              , V(660F00,44,_,0,0,_,_,_  ), 0                         , 1480 , 87 , 65 ), // #332
  INST(Knotd            , VexRm              , V(660F00,44,_,0,1,_,_,_  ), 0                         , 1486 , 87 , 66 ), // #333
  INST(Knotq            , VexRm              , V(000F00,44,_,0,1,_,_,_  ), 0                         , 1492 , 87 , 66 ), // #334
  INST(Knotw            , VexRm              , V(000F00,44,_,0,0,_,_,_  ), 0                         , 1498 , 87 , 67 ), // #335
  INST(Korb             , VexRvm             , V(660F00,45,_,1,0,_,_,_  ), 0                         , 1504 , 82 , 65 ), // #336
  INST(Kord             , VexRvm             , V(660F00,45,_,1,1,_,_,_  ), 0                         , 1509 , 82 , 66 ), // #337
  INST(Korq             , VexRvm             , V(000F00,45,_,1,1,_,_,_  ), 0                         , 1514 , 82 , 66 ), // #338
  INST(Kortestb         , VexRm              , V(660F00,98,_,0,0,_,_,_  ), 0                         , 1519 , 87 , 68 ), // #339
  INST(Kortestd         , VexRm              , V(660F00,98,_,0,1,_,_,_  ), 0                         , 1528 , 87 , 69 ), // #340
  INST(Kortestq         , VexRm              , V(000F00,98,_,0,1,_,_,_  ), 0                         , 1537 , 87 , 69 ), // #341
  INST(Kortestw         , VexRm              , V(000F00,98,_,0,0,_,_,_  ), 0                         , 1546 , 87 , 70 ), // #342
  INST(Korw             , VexRvm             , V(000F00,45,_,1,0,_,_,_  ), 0                         , 1555 , 82 , 67 ), // #343
  INST(Kshiftlb         , VexRmi             , V(660F3A,32,_,0,0,_,_,_  ), 0                         , 1560 , 88 , 65 ), // #344
  INST(Kshiftld         , VexRmi             , V(660F3A,33,_,0,0,_,_,_  ), 0                         , 1569 , 88 , 66 ), // #345
  INST(Kshiftlq         , VexRmi             , V(660F3A,33,_,0,1,_,_,_  ), 0                         , 1578 , 88 , 66 ), // #346
  INST(Kshiftlw         , VexRmi             , V(660F3A,32,_,0,1,_,_,_  ), 0                         , 1587 , 88 , 67 ), // #347
  INST(Kshiftrb         , VexRmi             , V(660F3A,30,_,0,0,_,_,_  ), 0                         , 1596 , 88 , 65 ), // #348
  INST(Kshiftrd         , VexRmi             , V(660F3A,31,_,0,0,_,_,_  ), 0                         , 1605 , 88 , 66 ), // #349
  INST(Kshiftrq         , VexRmi             , V(660F3A,31,_,0,1,_,_,_  ), 0                         , 1614 , 88 , 66 ), // #350
  INST(Kshiftrw         , VexRmi             , V(660F3A,30,_,0,1,_,_,_  ), 0                         , 1623 , 88 , 67 ), // #351
  INST(Ktestb           , VexRm              , V(660F00,99,_,0,0,_,_,_  ), 0                         , 1632 , 87 , 68 ), // #352
  INST(Ktestd           , VexRm              , V(660F00,99,_,0,1,_,_,_  ), 0                         , 1639 , 87 , 69 ), // #353
  INST(Ktestq           , VexRm              , V(000F00,99,_,0,1,_,_,_  ), 0                         , 1646 , 87 , 69 ), // #354
  INST(Ktestw           , VexRm              , V(000F00,99,_,0,0,_,_,_  ), 0                         , 1653 , 87 , 68 ), // #355
  INST(Kunpckbw         , VexRvm             , V(660F00,4B,_,1,0,_,_,_  ), 0                         , 1660 , 82 , 67 ), // #356
  INST(Kunpckdq         , VexRvm             , V(000F00,4B,_,1,1,_,_,_  ), 0                         , 1669 , 82 , 66 ), // #357
  INST(Kunpckwd         , VexRvm             , V(000F00,4B,_,1,0,_,_,_  ), 0                         , 1678 , 82 , 66 ), // #358
  INST(Kxnorb           , VexRvm             , V(660F00,46,_,1,0,_,_,_  ), 0                         , 1687 , 82 , 65 ), // #359
  INST(Kxnord           , VexRvm             , V(660F00,46,_,1,1,_,_,_  ), 0                         , 1694 , 82 , 66 ), // #360
  INST(Kxnorq           , VexRvm             , V(000F00,46,_,1,1,_,_,_  ), 0                         , 1701 , 82 , 66 ), // #361
  INST(Kxnorw           , VexRvm             , V(000F00,46,_,1,0,_,_,_  ), 0                         , 1708 , 82 , 67 ), // #362
  INST(Kxorb            , VexRvm             , V(660F00,47,_,1,0,_,_,_  ), 0                         , 1715 , 82 , 65 ), // #363
  INST(Kxord            , VexRvm             , V(660F00,47,_,1,1,_,_,_  ), 0                         , 1721 , 82 , 66 ), // #364
  INST(Kxorq            , VexRvm             , V(000F00,47,_,1,1,_,_,_  ), 0                         , 1727 , 82 , 66 ), // #365
  INST(Kxorw            , VexRvm             , V(000F00,47,_,1,0,_,_,_  ), 0                         , 1733 , 82 , 67 ), // #366
  INST(Lahf             , X86Op              , O(000000,9F,_,_,_,_,_,_  ), 0                         , 1739 , 89 , 71 ), // #367
  INST(Lar              , X86Rm              , O(000F00,02,_,_,_,_,_,_  ), 0                         , 1744 , 90 , 10 ), // #368
  INST(Lddqu            , ExtRm              , O(F20F00,F0,_,_,_,_,_,_  ), 0                         , 5776 , 91 , 6  ), // #369
  INST(Ldmxcsr          , X86M_Only          , O(000F00,AE,2,_,_,_,_,_  ), 0                         , 5783 , 92 , 5  ), // #370
  INST(Lds              , X86Rm              , O(000000,C5,_,_,_,_,_,_  ), 0                         , 1748 , 93 , 0  ), // #371
  INST(Lea              , X86Lea             , O(000000,8D,_,_,x,_,_,_  ), 0                         , 1752 , 94 , 0  ), // #372
  INST(Leave            , X86Op              , O(000000,C9,_,_,_,_,_,_  ), 0                         , 1756 , 30 , 0  ), // #373
  INST(Les              , X86Rm              , O(000000,C4,_,_,_,_,_,_  ), 0                         , 1762 , 93 , 0  ), // #374
  INST(Lfence           , X86Fence           , O(000F00,AE,5,_,_,_,_,_  ), 0                         , 1766 , 30 , 4  ), // #375
  INST(Lfs              , X86Rm              , O(000F00,B4,_,_,_,_,_,_  ), 0                         , 1773 , 95 , 0  ), // #376
  INST(Lgdt             , X86M_Only          , O(000F00,01,2,_,_,_,_,_  ), 0                         , 1777 , 31 , 0  ), // #377
  INST(Lgs              , X86Rm              , O(000F00,B5,_,_,_,_,_,_  ), 0                         , 1782 , 95 , 0  ), // #378
  INST(Lidt             , X86M_Only          , O(000F00,01,3,_,_,_,_,_  ), 0                         , 1786 , 31 , 0  ), // #379
  INST(Lldt             , X86M_NoSize        , O(000F00,00,2,_,_,_,_,_  ), 0                         , 1791 , 96 , 0  ), // #380
  INST(Llwpcb           , VexR_Wx            , V(XOP_M9,12,0,0,x,_,_,_  ), 0                         , 1796 , 97 , 72 ), // #381
  INST(Lmsw             , X86M_NoSize        , O(000F00,01,6,_,_,_,_,_  ), 0                         , 1803 , 96 , 0  ), // #382
  INST(Lods             , X86StrRm           , O(000000,AC,_,_,_,_,_,_  ), 0                         , 1808 , 98 , 0  ), // #383
  INST(Loop             , X86JecxzLoop       , 0                         , O(000000,E2,_,_,_,_,_,_  ), 1813 , 80 , 0  ), // #384
  INST(Loope            , X86JecxzLoop       , 0                         , O(000000,E1,_,_,_,_,_,_  ), 1818 , 80 , 60 ), // #385
  INST(Loopne           , X86JecxzLoop       , 0                         , O(000000,E0,_,_,_,_,_,_  ), 1824 , 80 , 60 ), // #386
  INST(Lsl              , X86Rm              , O(000F00,03,_,_,_,_,_,_  ), 0                         , 1831 , 99 , 10 ), // #387
  INST(Lss              , X86Rm              , O(000F00,B2,_,_,_,_,_,_  ), 0                         , 6274 , 95 , 0  ), // #388
  INST(Ltr              , X86M_NoSize        , O(000F00,00,3,_,_,_,_,_  ), 0                         , 1835 , 96 , 0  ), // #389
  INST(Lwpins           , VexVmi4_Wx         , V(XOP_MA,12,0,0,x,_,_,_  ), 0                         , 1839 , 100, 72 ), // #390
  INST(Lwpval           , VexVmi4_Wx         , V(XOP_MA,12,1,0,x,_,_,_  ), 0                         , 1846 , 100, 72 ), // #391
  INST(Lzcnt            , X86Rm_Raw66H       , O(F30F00,BD,_,_,x,_,_,_  ), 0                         , 1853 , 22 , 73 ), // #392
  INST(Maskmovdqu       , ExtRm_ZDI          , O(660F00,57,_,_,_,_,_,_  ), 0                         , 5792 , 101, 4  ), // #393
  INST(Maskmovq         , ExtRm_ZDI          , O(000F00,F7,_,_,_,_,_,_  ), 0                         , 7763 , 102, 74 ), // #394
  INST(Maxpd            , ExtRm              , O(660F00,5F,_,_,_,_,_,_  ), 0                         , 5826 , 5  , 4  ), // #395
  INST(Maxps            , ExtRm              , O(000F00,5F,_,_,_,_,_,_  ), 0                         , 5833 , 5  , 5  ), // #396
  INST(Maxsd            , ExtRm              , O(F20F00,5F,_,_,_,_,_,_  ), 0                         , 7782 , 6  , 4  ), // #397
  INST(Maxss            , ExtRm              , O(F30F00,5F,_,_,_,_,_,_  ), 0                         , 5847 , 7  , 5  ), // #398
  INST(Mfence           , X86Fence           , O(000F00,AE,6,_,_,_,_,_  ), 0                         , 1859 , 30 , 4  ), // #399
  INST(Minpd            , ExtRm              , O(660F00,5D,_,_,_,_,_,_  ), 0                         , 5876 , 5  , 4  ), // #400
  INST(Minps            , ExtRm              , O(000F00,5D,_,_,_,_,_,_  ), 0                         , 5883 , 5  , 5  ), // #401
  INST(Minsd            , ExtRm              , O(F20F00,5D,_,_,_,_,_,_  ), 0                         , 7846 , 6  , 4  ), // #402
  INST(Minss            , ExtRm              , O(F30F00,5D,_,_,_,_,_,_  ), 0                         , 5897 , 7  , 5  ), // #403
  INST(Monitor          , X86Op              , O(000F01,C8,_,_,_,_,_,_  ), 0                         , 1866 , 103, 75 ), // #404
  INST(Monitorx         , X86Op              , O(000F01,FA,_,_,_,_,_,_  ), 0                         , 1874 , 103, 76 ), // #405
  INST(Mov              , X86Mov             , 0                         , 0                         , 138  , 104, 0  ), // #406
  INST(Movapd           , ExtMov             , O(660F00,28,_,_,_,_,_,_  ), O(660F00,29,_,_,_,_,_,_  ), 5928 , 105, 4  ), // #407
  INST(Movaps           , ExtMov             , O(000F00,28,_,_,_,_,_,_  ), O(000F00,29,_,_,_,_,_,_  ), 5936 , 105, 5  ), // #408
  INST(Movbe            , ExtMovbe           , O(000F38,F0,_,_,x,_,_,_  ), O(000F38,F1,_,_,x,_,_,_  ), 611  , 106, 77 ), // #409
  INST(Movd             , ExtMovd            , O(000F00,6E,_,_,_,_,_,_  ), O(000F00,7E,_,_,_,_,_,_  ), 7756 , 107, 78 ), // #410
  INST(Movddup          , ExtMov             , O(F20F00,12,_,_,_,_,_,_  ), 0                         , 5950 , 6  , 6  ), // #411
  INST(Movdir64b        , X86Movdir64b       , O(660F38,F8,_,_,_,_,_,_  ), 0                         , 1883 , 108, 79 ), // #412
  INST(Movdiri          , X86MovntiMovdiri   , O(000F38,F9,_,_,_,_,_,_  ), 0                         , 1893 , 109, 80 ), // #413
  INST(Movdq2q          , ExtMov             , O(F20F00,D6,_,_,_,_,_,_  ), 0                         , 1901 , 110, 4  ), // #414
  INST(Movdqa           , ExtMov             , O(660F00,6F,_,_,_,_,_,_  ), O(660F00,7F,_,_,_,_,_,_  ), 5959 , 105, 4  ), // #415
  INST(Movdqu           , ExtMov             , O(F30F00,6F,_,_,_,_,_,_  ), O(F30F00,7F,_,_,_,_,_,_  ), 5796 , 105, 4  ), // #416
  INST(Movhlps          , ExtMov             , O(000F00,12,_,_,_,_,_,_  ), 0                         , 6034 , 111, 5  ), // #417
  INST(Movhpd           , ExtMov             , O(660F00,16,_,_,_,_,_,_  ), O(660F00,17,_,_,_,_,_,_  ), 6043 , 112, 4  ), // #418
  INST(Movhps           , ExtMov             , O(000F00,16,_,_,_,_,_,_  ), O(000F00,17,_,_,_,_,_,_  ), 6051 , 112, 5  ), // #419
  INST(Movlhps          , ExtMov             , O(000F00,16,_,_,_,_,_,_  ), 0                         , 6059 , 111, 5  ), // #420
  INST(Movlpd           , ExtMov             , O(660F00,12,_,_,_,_,_,_  ), O(660F00,13,_,_,_,_,_,_  ), 6068 , 112, 4  ), // #421
  INST(Movlps           , ExtMov             , O(000F00,12,_,_,_,_,_,_  ), O(000F00,13,_,_,_,_,_,_  ), 6076 , 112, 5  ), // #422
  INST(Movmskpd         , ExtMov             , O(660F00,50,_,_,_,_,_,_  ), 0                         , 6084 , 113, 4  ), // #423
  INST(Movmskps         , ExtMov             , O(000F00,50,_,_,_,_,_,_  ), 0                         , 6094 , 113, 5  ), // #424
  INST(Movntdq          , ExtMov             , 0                         , O(660F00,E7,_,_,_,_,_,_  ), 6104 , 114, 4  ), // #425
  INST(Movntdqa         , ExtMov             , O(660F38,2A,_,_,_,_,_,_  ), 0                         , 6113 , 91 , 12 ), // #426
  INST(Movnti           , X86MovntiMovdiri   , O(000F00,C3,_,_,x,_,_,_  ), 0                         , 1909 , 109, 4  ), // #427
  INST(Movntpd          , ExtMov             , 0                         , O(660F00,2B,_,_,_,_,_,_  ), 6123 , 114, 4  ), // #428
  INST(Movntps          , ExtMov             , 0                         , O(000F00,2B,_,_,_,_,_,_  ), 6132 , 114, 5  ), // #429
  INST(Movntq           , ExtMov             , 0                         , O(000F00,E7,_,_,_,_,_,_  ), 1916 , 115, 74 ), // #430
  INST(Movntsd          , ExtMov             , 0                         , O(F20F00,2B,_,_,_,_,_,_  ), 1923 , 116, 46 ), // #431
  INST(Movntss          , ExtMov             , 0                         , O(F30F00,2B,_,_,_,_,_,_  ), 1931 , 117, 46 ), // #432
  INST(Movq             , ExtMovq            , O(000F00,6E,_,_,x,_,_,_  ), O(000F00,7E,_,_,x,_,_,_  ), 7767 , 118, 78 ), // #433
  INST(Movq2dq          , ExtRm              , O(F30F00,D6,_,_,_,_,_,_  ), 0                         , 1939 , 119, 4  ), // #434
  INST(Movs             , X86StrMm           , O(000000,A4,_,_,_,_,_,_  ), 0                         , 425  , 120, 0  ), // #435
  INST(Movsd            , ExtMov             , O(F20F00,10,_,_,_,_,_,_  ), O(F20F00,11,_,_,_,_,_,_  ), 6147 , 121, 4  ), // #436
  INST(Movshdup         , ExtRm              , O(F30F00,16,_,_,_,_,_,_  ), 0                         , 6154 , 5  , 6  ), // #437
  INST(Movsldup         , ExtRm              , O(F30F00,12,_,_,_,_,_,_  ), 0                         , 6164 , 5  , 6  ), // #438
  INST(Movss            , ExtMov             , O(F30F00,10,_,_,_,_,_,_  ), O(F30F00,11,_,_,_,_,_,_  ), 6174 , 122, 5  ), // #439
  INST(Movsx            , X86MovsxMovzx      , O(000F00,BE,_,_,x,_,_,_  ), 0                         , 1947 , 123, 0  ), // #440
  INST(Movsxd           , X86Rm              , O(000000,63,_,_,1,_,_,_  ), 0                         , 1953 , 124, 0  ), // #441
  INST(Movupd           , ExtMov             , O(660F00,10,_,_,_,_,_,_  ), O(660F00,11,_,_,_,_,_,_  ), 6181 , 105, 4  ), // #442
  INST(Movups           , ExtMov             , O(000F00,10,_,_,_,_,_,_  ), O(000F00,11,_,_,_,_,_,_  ), 6189 , 105, 5  ), // #443
  INST(Movzx            , X86MovsxMovzx      , O(000F00,B6,_,_,x,_,_,_  ), 0                         , 1960 , 123, 0  ), // #444
  INST(Mpsadbw          , ExtRmi             , O(660F3A,42,_,_,_,_,_,_  ), 0                         , 6197 , 8  , 12 ), // #445
  INST(Mul              , X86M_GPB_MulDiv    , O(000000,F6,4,_,x,_,_,_  ), 0                         , 783  , 52 , 1  ), // #446
  INST(Mulpd            , ExtRm              , O(660F00,59,_,_,_,_,_,_  ), 0                         , 6251 , 5  , 4  ), // #447
  INST(Mulps            , ExtRm              , O(000F00,59,_,_,_,_,_,_  ), 0                         , 6258 , 5  , 5  ), // #448
  INST(Mulsd            , ExtRm              , O(F20F00,59,_,_,_,_,_,_  ), 0                         , 6265 , 6  , 4  ), // #449
  INST(Mulss            , ExtRm              , O(F30F00,59,_,_,_,_,_,_  ), 0                         , 6272 , 7  , 5  ), // #450
  INST(Mulx             , VexRvm_ZDX_Wx      , V(F20F38,F6,_,0,x,_,_,_  ), 0                         , 1966 , 125, 81 ), // #451
  INST(Mwait            , X86Op              , O(000F01,C9,_,_,_,_,_,_  ), 0                         , 1971 , 126, 75 ), // #452
  INST(Mwaitx           , X86Op              , O(000F01,FB,_,_,_,_,_,_  ), 0                         , 1977 , 127, 76 ), // #453
  INST(Neg              , X86M_GPB           , O(000000,F6,3,_,x,_,_,_  ), 0                         , 1984 , 128, 82 ), // #454
  INST(Nop              , X86M_Nop           , O(000000,90,_,_,_,_,_,_  ), 0                         , 914  , 129, 0  ), // #455
  INST(Not              , X86M_GPB           , O(000000,F6,2,_,x,_,_,_  ), 0                         , 1988 , 128, 0  ), // #456
  INST(Or               , X86Arith           , O(000000,08,1,_,x,_,_,_  ), 0                         , 1123 , 130, 1  ), // #457
  INST(Orpd             , ExtRm              , O(660F00,56,_,_,_,_,_,_  ), 0                         , 9973 , 11 , 4  ), // #458
  INST(Orps             , ExtRm              , O(000F00,56,_,_,_,_,_,_  ), 0                         , 9980 , 11 , 5  ), // #459
  INST(Out              , X86Out             , O(000000,EE,_,_,_,_,_,_  ), O(000000,E6,_,_,_,_,_,_  ), 1992 , 131, 0  ), // #460
  INST(Outs             , X86Outs            , O(000000,6E,_,_,_,_,_,_  ), 0                         , 1996 , 132, 0  ), // #461
  INST(Pabsb            , ExtRm_P            , O(000F38,1C,_,_,_,_,_,_  ), 0                         , 6326 , 133, 83 ), // #462
  INST(Pabsd            , ExtRm_P            , O(000F38,1E,_,_,_,_,_,_  ), 0                         , 6333 , 133, 83 ), // #463
  INST(Pabsw            , ExtRm_P            , O(000F38,1D,_,_,_,_,_,_  ), 0                         , 6347 , 133, 83 ), // #464
  INST(Packssdw         , ExtRm_P            , O(000F00,6B,_,_,_,_,_,_  ), 0                         , 6354 , 133, 78 ), // #465
  INST(Packsswb         , ExtRm_P            , O(000F00,63,_,_,_,_,_,_  ), 0                         , 6364 , 133, 78 ), // #466
  INST(Packusdw         , ExtRm              , O(660F38,2B,_,_,_,_,_,_  ), 0                         , 6374 , 5  , 12 ), // #467
  INST(Packuswb         , ExtRm_P            , O(000F00,67,_,_,_,_,_,_  ), 0                         , 6384 , 133, 78 ), // #468
  INST(Paddb            , ExtRm_P            , O(000F00,FC,_,_,_,_,_,_  ), 0                         , 6394 , 133, 78 ), // #469
  INST(Paddd            , ExtRm_P            , O(000F00,FE,_,_,_,_,_,_  ), 0                         , 6401 , 133, 78 ), // #470
  INST(Paddq            , ExtRm_P            , O(000F00,D4,_,_,_,_,_,_  ), 0                         , 6408 , 133, 4  ), // #471
  INST(Paddsb           , ExtRm_P            , O(000F00,EC,_,_,_,_,_,_  ), 0                         , 6415 , 133, 78 ), // #472
  INST(Paddsw           , ExtRm_P            , O(000F00,ED,_,_,_,_,_,_  ), 0                         , 6423 , 133, 78 ), // #473
  INST(Paddusb          , ExtRm_P            , O(000F00,DC,_,_,_,_,_,_  ), 0                         , 6431 , 133, 78 ), // #474
  INST(Paddusw          , ExtRm_P            , O(000F00,DD,_,_,_,_,_,_  ), 0                         , 6440 , 133, 78 ), // #475
  INST(Paddw            , ExtRm_P            , O(000F00,FD,_,_,_,_,_,_  ), 0                         , 6449 , 133, 78 ), // #476
  INST(Palignr          , ExtRmi_P           , O(000F3A,0F,_,_,_,_,_,_  ), 0                         , 6456 , 134, 6  ), // #477
  INST(Pand             , ExtRm_P            , O(000F00,DB,_,_,_,_,_,_  ), 0                         , 6465 , 135, 78 ), // #478
  INST(Pandn            , ExtRm_P            , O(000F00,DF,_,_,_,_,_,_  ), 0                         , 6478 , 136, 78 ), // #479
  INST(Pause            , X86Op              , O(F30000,90,_,_,_,_,_,_  ), 0                         , 2001 , 30 , 0  ), // #480
  INST(Pavgb            , ExtRm_P            , O(000F00,E0,_,_,_,_,_,_  ), 0                         , 6508 , 133, 84 ), // #481
  INST(Pavgusb          , Ext3dNow           , O(000F0F,BF,_,_,_,_,_,_  ), 0                         , 2007 , 137, 50 ), // #482
  INST(Pavgw            , ExtRm_P            , O(000F00,E3,_,_,_,_,_,_  ), 0                         , 6515 , 133, 84 ), // #483
  INST(Pblendvb         , ExtRm_XMM0         , O(660F38,10,_,_,_,_,_,_  ), 0                         , 6531 , 15 , 12 ), // #484
  INST(Pblendw          , ExtRmi             , O(660F3A,0E,_,_,_,_,_,_  ), 0                         , 6541 , 8  , 12 ), // #485
  INST(Pclmulqdq        , ExtRmi             , O(660F3A,44,_,_,_,_,_,_  ), 0                         , 6634 , 8  , 85 ), // #486
  INST(Pcmpeqb          , ExtRm_P            , O(000F00,74,_,_,_,_,_,_  ), 0                         , 6666 , 136, 78 ), // #487
  INST(Pcmpeqd          , ExtRm_P            , O(000F00,76,_,_,_,_,_,_  ), 0                         , 6675 , 136, 78 ), // #488
  INST(Pcmpeqq          , ExtRm              , O(660F38,29,_,_,_,_,_,_  ), 0                         , 6684 , 138, 12 ), // #489
  INST(Pcmpeqw          , ExtRm_P            , O(000F00,75,_,_,_,_,_,_  ), 0                         , 6693 , 136, 78 ), // #490
  INST(Pcmpestri        , ExtRmi             , O(660F3A,61,_,_,_,_,_,_  ), 0                         , 6702 , 139, 86 ), // #491
  INST(Pcmpestrm        , ExtRmi             , O(660F3A,60,_,_,_,_,_,_  ), 0                         , 6713 , 140, 86 ), // #492
  INST(Pcmpgtb          , ExtRm_P            , O(000F00,64,_,_,_,_,_,_  ), 0                         , 6724 , 136, 78 ), // #493
  INST(Pcmpgtd          , ExtRm_P            , O(000F00,66,_,_,_,_,_,_  ), 0                         , 6733 , 136, 78 ), // #494
  INST(Pcmpgtq          , ExtRm              , O(660F38,37,_,_,_,_,_,_  ), 0                         , 6742 , 138, 43 ), // #495
  INST(Pcmpgtw          , ExtRm_P            , O(000F00,65,_,_,_,_,_,_  ), 0                         , 6751 , 136, 78 ), // #496
  INST(Pcmpistri        , ExtRmi             , O(660F3A,63,_,_,_,_,_,_  ), 0                         , 6760 , 141, 86 ), // #497
  INST(Pcmpistrm        , ExtRmi             , O(660F3A,62,_,_,_,_,_,_  ), 0                         , 6771 , 142, 86 ), // #498
  INST(Pcommit          , X86Op_O            , O(660F00,AE,7,_,_,_,_,_  ), 0                         , 2015 , 30 , 87 ), // #499
  INST(Pdep             , VexRvm_Wx          , V(F20F38,F5,_,0,x,_,_,_  ), 0                         , 2023 , 10 , 81 ), // #500
  INST(Pext             , VexRvm_Wx          , V(F30F38,F5,_,0,x,_,_,_  ), 0                         , 2028 , 10 , 81 ), // #501
  INST(Pextrb           , ExtExtract         , O(000F3A,14,_,_,_,_,_,_  ), 0                         , 7258 , 143, 12 ), // #502
  INST(Pextrd           , ExtExtract         , O(000F3A,16,_,_,_,_,_,_  ), 0                         , 7266 , 55 , 12 ), // #503
  INST(Pextrq           , ExtExtract         , O(000F3A,16,_,_,1,_,_,_  ), 0                         , 7274 , 144, 12 ), // #504
  INST(Pextrw           , ExtPextrw          , O(000F00,C5,_,_,_,_,_,_  ), O(000F3A,15,_,_,_,_,_,_  ), 7282 , 145, 88 ), // #505
  INST(Pf2id            , Ext3dNow           , O(000F0F,1D,_,_,_,_,_,_  ), 0                         , 2033 , 137, 50 ), // #506
  INST(Pf2iw            , Ext3dNow           , O(000F0F,1C,_,_,_,_,_,_  ), 0                         , 2039 , 137, 89 ), // #507
  INST(Pfacc            , Ext3dNow           , O(000F0F,AE,_,_,_,_,_,_  ), 0                         , 2045 , 137, 50 ), // #508
  INST(Pfadd            , Ext3dNow           , O(000F0F,9E,_,_,_,_,_,_  ), 0                         , 2051 , 137, 50 ), // #509
  INST(Pfcmpeq          , Ext3dNow           , O(000F0F,B0,_,_,_,_,_,_  ), 0                         , 2057 , 137, 50 ), // #510
  INST(Pfcmpge          , Ext3dNow           , O(000F0F,90,_,_,_,_,_,_  ), 0                         , 2065 , 137, 50 ), // #511
  INST(Pfcmpgt          , Ext3dNow           , O(000F0F,A0,_,_,_,_,_,_  ), 0                         , 2073 , 137, 50 ), // #512
  INST(Pfmax            , Ext3dNow           , O(000F0F,A4,_,_,_,_,_,_  ), 0                         , 2081 , 137, 50 ), // #513
  INST(Pfmin            , Ext3dNow           , O(000F0F,94,_,_,_,_,_,_  ), 0                         , 2087 , 137, 50 ), // #514
  INST(Pfmul            , Ext3dNow           , O(000F0F,B4,_,_,_,_,_,_  ), 0                         , 2093 , 137, 50 ), // #515
  INST(Pfnacc           , Ext3dNow           , O(000F0F,8A,_,_,_,_,_,_  ), 0                         , 2099 , 137, 89 ), // #516
  INST(Pfpnacc          , Ext3dNow           , O(000F0F,8E,_,_,_,_,_,_  ), 0                         , 2106 , 137, 89 ), // #517
  INST(Pfrcp            , Ext3dNow           , O(000F0F,96,_,_,_,_,_,_  ), 0                         , 2114 , 137, 50 ), // #518
  INST(Pfrcpit1         , Ext3dNow           , O(000F0F,A6,_,_,_,_,_,_  ), 0                         , 2120 , 137, 50 ), // #519
  INST(Pfrcpit2         , Ext3dNow           , O(000F0F,B6,_,_,_,_,_,_  ), 0                         , 2129 , 137, 50 ), // #520
  INST(Pfrcpv           , Ext3dNow           , O(000F0F,86,_,_,_,_,_,_  ), 0                         , 2138 , 137, 90 ), // #521
  INST(Pfrsqit1         , Ext3dNow           , O(000F0F,A7,_,_,_,_,_,_  ), 0                         , 2145 , 137, 50 ), // #522
  INST(Pfrsqrt          , Ext3dNow           , O(000F0F,97,_,_,_,_,_,_  ), 0                         , 2154 , 137, 50 ), // #523
  INST(Pfrsqrtv         , Ext3dNow           , O(000F0F,87,_,_,_,_,_,_  ), 0                         , 2162 , 137, 90 ), // #524
  INST(Pfsub            , Ext3dNow           , O(000F0F,9A,_,_,_,_,_,_  ), 0                         , 2171 , 137, 50 ), // #525
  INST(Pfsubr           , Ext3dNow           , O(000F0F,AA,_,_,_,_,_,_  ), 0                         , 2177 , 137, 50 ), // #526
  INST(Phaddd           , ExtRm_P            , O(000F38,02,_,_,_,_,_,_  ), 0                         , 7361 , 133, 83 ), // #527
  INST(Phaddsw          , ExtRm_P            , O(000F38,03,_,_,_,_,_,_  ), 0                         , 7378 , 133, 83 ), // #528
  INST(Phaddw           , ExtRm_P            , O(000F38,01,_,_,_,_,_,_  ), 0                         , 7447 , 133, 83 ), // #529
  INST(Phminposuw       , ExtRm              , O(660F38,41,_,_,_,_,_,_  ), 0                         , 7473 , 5  , 12 ), // #530
  INST(Phsubd           , ExtRm_P            , O(000F38,06,_,_,_,_,_,_  ), 0                         , 7494 , 133, 83 ), // #531
  INST(Phsubsw          , ExtRm_P            , O(000F38,07,_,_,_,_,_,_  ), 0                         , 7511 , 133, 83 ), // #532
  INST(Phsubw           , ExtRm_P            , O(000F38,05,_,_,_,_,_,_  ), 0                         , 7520 , 133, 83 ), // #533
  INST(Pi2fd            , Ext3dNow           , O(000F0F,0D,_,_,_,_,_,_  ), 0                         , 2184 , 137, 50 ), // #534
  INST(Pi2fw            , Ext3dNow           , O(000F0F,0C,_,_,_,_,_,_  ), 0                         , 2190 , 137, 89 ), // #535
  INST(Pinsrb           , ExtRmi             , O(660F3A,20,_,_,_,_,_,_  ), 0                         , 7537 , 146, 12 ), // #536
  INST(Pinsrd           , ExtRmi             , O(660F3A,22,_,_,_,_,_,_  ), 0                         , 7545 , 147, 12 ), // #537
  INST(Pinsrq           , ExtRmi             , O(660F3A,22,_,_,1,_,_,_  ), 0                         , 7553 , 148, 12 ), // #538
  INST(Pinsrw           , ExtRmi_P           , O(000F00,C4,_,_,_,_,_,_  ), 0                         , 7561 , 149, 84 ), // #539
  INST(Pmaddubsw        , ExtRm_P            , O(000F38,04,_,_,_,_,_,_  ), 0                         , 7731 , 133, 83 ), // #540
  INST(Pmaddwd          , ExtRm_P            , O(000F00,F5,_,_,_,_,_,_  ), 0                         , 7742 , 133, 78 ), // #541
  INST(Pmaxsb           , ExtRm              , O(660F38,3C,_,_,_,_,_,_  ), 0                         , 7773 , 11 , 12 ), // #542
  INST(Pmaxsd           , ExtRm              , O(660F38,3D,_,_,_,_,_,_  ), 0                         , 7781 , 11 , 12 ), // #543
  INST(Pmaxsw           , ExtRm_P            , O(000F00,EE,_,_,_,_,_,_  ), 0                         , 7797 , 135, 84 ), // #544
  INST(Pmaxub           , ExtRm_P            , O(000F00,DE,_,_,_,_,_,_  ), 0                         , 7805 , 135, 84 ), // #545
  INST(Pmaxud           , ExtRm              , O(660F38,3F,_,_,_,_,_,_  ), 0                         , 7813 , 11 , 12 ), // #546
  INST(Pmaxuw           , ExtRm              , O(660F38,3E,_,_,_,_,_,_  ), 0                         , 7829 , 11 , 12 ), // #547
  INST(Pminsb           , ExtRm              , O(660F38,38,_,_,_,_,_,_  ), 0                         , 7837 , 11 , 12 ), // #548
  INST(Pminsd           , ExtRm              , O(660F38,39,_,_,_,_,_,_  ), 0                         , 7845 , 11 , 12 ), // #549
  INST(Pminsw           , ExtRm_P            , O(000F00,EA,_,_,_,_,_,_  ), 0                         , 7861 , 135, 84 ), // #550
  INST(Pminub           , ExtRm_P            , O(000F00,DA,_,_,_,_,_,_  ), 0                         , 7869 , 135, 84 ), // #551
  INST(Pminud           , ExtRm              , O(660F38,3B,_,_,_,_,_,_  ), 0                         , 7877 , 11 , 12 ), // #552
  INST(Pminuw           , ExtRm              , O(660F38,3A,_,_,_,_,_,_  ), 0                         , 7893 , 11 , 12 ), // #553
  INST(Pmovmskb         , ExtRm_P            , O(000F00,D7,_,_,_,_,_,_  ), 0                         , 7971 , 150, 84 ), // #554
  INST(Pmovsxbd         , ExtRm              , O(660F38,21,_,_,_,_,_,_  ), 0                         , 8068 , 7  , 12 ), // #555
  INST(Pmovsxbq         , ExtRm              , O(660F38,22,_,_,_,_,_,_  ), 0                         , 8078 , 151, 12 ), // #556
  INST(Pmovsxbw         , ExtRm              , O(660F38,20,_,_,_,_,_,_  ), 0                         , 8088 , 6  , 12 ), // #557
  INST(Pmovsxdq         , ExtRm              , O(660F38,25,_,_,_,_,_,_  ), 0                         , 8098 , 6  , 12 ), // #558
  INST(Pmovsxwd         , ExtRm              , O(660F38,23,_,_,_,_,_,_  ), 0                         , 8108 , 6  , 12 ), // #559
  INST(Pmovsxwq         , ExtRm              , O(660F38,24,_,_,_,_,_,_  ), 0                         , 8118 , 7  , 12 ), // #560
  INST(Pmovzxbd         , ExtRm              , O(660F38,31,_,_,_,_,_,_  ), 0                         , 8205 , 7  , 12 ), // #561
  INST(Pmovzxbq         , ExtRm              , O(660F38,32,_,_,_,_,_,_  ), 0                         , 8215 , 151, 12 ), // #562
  INST(Pmovzxbw         , ExtRm              , O(660F38,30,_,_,_,_,_,_  ), 0                         , 8225 , 6  , 12 ), // #563
  INST(Pmovzxdq         , ExtRm              , O(660F38,35,_,_,_,_,_,_  ), 0                         , 8235 , 6  , 12 ), // #564
  INST(Pmovzxwd         , ExtRm              , O(660F38,33,_,_,_,_,_,_  ), 0                         , 8245 , 6  , 12 ), // #565
  INST(Pmovzxwq         , ExtRm              , O(660F38,34,_,_,_,_,_,_  ), 0                         , 8255 , 7  , 12 ), // #566
  INST(Pmuldq           , ExtRm              , O(660F38,28,_,_,_,_,_,_  ), 0                         , 8265 , 5  , 12 ), // #567
  INST(Pmulhrsw         , ExtRm_P            , O(000F38,0B,_,_,_,_,_,_  ), 0                         , 8273 , 133, 83 ), // #568
  INST(Pmulhrw          , Ext3dNow           , O(000F0F,B7,_,_,_,_,_,_  ), 0                         , 2196 , 137, 50 ), // #569
  INST(Pmulhuw          , ExtRm_P            , O(000F00,E4,_,_,_,_,_,_  ), 0                         , 8283 , 133, 84 ), // #570
  INST(Pmulhw           , ExtRm_P            , O(000F00,E5,_,_,_,_,_,_  ), 0                         , 8292 , 133, 78 ), // #571
  INST(Pmulld           , ExtRm              , O(660F38,40,_,_,_,_,_,_  ), 0                         , 8300 , 5  , 12 ), // #572
  INST(Pmullw           , ExtRm_P            , O(000F00,D5,_,_,_,_,_,_  ), 0                         , 8316 , 133, 78 ), // #573
  INST(Pmuludq          , ExtRm_P            , O(000F00,F4,_,_,_,_,_,_  ), 0                         , 8339 , 133, 4  ), // #574
  INST(Pop              , X86Pop             , O(000000,8F,0,_,_,_,_,_  ), O(000000,58,_,_,_,_,_,_  ), 2204 , 152, 0  ), // #575
  INST(Popa             , X86Op              , O(660000,61,_,_,_,_,_,_  ), 0                         , 2208 , 74 , 0  ), // #576
  INST(Popad            , X86Op              , O(000000,61,_,_,_,_,_,_  ), 0                         , 2213 , 74 , 0  ), // #577
  INST(Popcnt           , X86Rm_Raw66H       , O(F30F00,B8,_,_,x,_,_,_  ), 0                         , 2219 , 22 , 91 ), // #578
  INST(Popf             , X86Op              , O(660000,9D,_,_,_,_,_,_  ), 0                         , 2226 , 30 , 1  ), // #579
  INST(Popfd            , X86Op              , O(000000,9D,_,_,_,_,_,_  ), 0                         , 2231 , 74 , 1  ), // #580
  INST(Popfq            , X86Op              , O(000000,9D,_,_,_,_,_,_  ), 0                         , 2237 , 153, 1  ), // #581
  INST(Por              , ExtRm_P            , O(000F00,EB,_,_,_,_,_,_  ), 0                         , 8384 , 135, 78 ), // #582
  INST(Prefetch         , X86M_Only          , O(000F00,0D,0,_,_,_,_,_  ), 0                         , 2243 , 31 , 50 ), // #583
  INST(Prefetchnta      , X86M_Only          , O(000F00,18,0,_,_,_,_,_  ), 0                         , 2252 , 31 , 74 ), // #584
  INST(Prefetcht0       , X86M_Only          , O(000F00,18,1,_,_,_,_,_  ), 0                         , 2264 , 31 , 74 ), // #585
  INST(Prefetcht1       , X86M_Only          , O(000F00,18,2,_,_,_,_,_  ), 0                         , 2275 , 31 , 74 ), // #586
  INST(Prefetcht2       , X86M_Only          , O(000F00,18,3,_,_,_,_,_  ), 0                         , 2286 , 31 , 74 ), // #587
  INST(Prefetchw        , X86M_Only          , O(000F00,0D,1,_,_,_,_,_  ), 0                         , 2297 , 31 , 92 ), // #588
  INST(Prefetchwt1      , X86M_Only          , O(000F00,0D,2,_,_,_,_,_  ), 0                         , 2307 , 31 , 93 ), // #589
  INST(Psadbw           , ExtRm_P            , O(000F00,F6,_,_,_,_,_,_  ), 0                         , 3965 , 133, 84 ), // #590
  INST(Pshufb           , ExtRm_P            , O(000F38,00,_,_,_,_,_,_  ), 0                         , 8710 , 133, 83 ), // #591
  INST(Pshufd           , ExtRmi             , O(660F00,70,_,_,_,_,_,_  ), 0                         , 8731 , 8  , 4  ), // #592
  INST(Pshufhw          , ExtRmi             , O(F30F00,70,_,_,_,_,_,_  ), 0                         , 8739 , 8  , 4  ), // #593
  INST(Pshuflw          , ExtRmi             , O(F20F00,70,_,_,_,_,_,_  ), 0                         , 8748 , 8  , 4  ), // #594
  INST(Pshufw           , ExtRmi_P           , O(000F00,70,_,_,_,_,_,_  ), 0                         , 2319 , 154, 74 ), // #595
  INST(Psignb           , ExtRm_P            , O(000F38,08,_,_,_,_,_,_  ), 0                         , 8757 , 133, 83 ), // #596
  INST(Psignd           , ExtRm_P            , O(000F38,0A,_,_,_,_,_,_  ), 0                         , 8765 , 133, 83 ), // #597
  INST(Psignw           , ExtRm_P            , O(000F38,09,_,_,_,_,_,_  ), 0                         , 8773 , 133, 83 ), // #598
  INST(Pslld            , ExtRmRi_P          , O(000F00,F2,_,_,_,_,_,_  ), O(000F00,72,6,_,_,_,_,_  ), 8781 , 155, 78 ), // #599
  INST(Pslldq           , ExtRmRi            , 0                         , O(660F00,73,7,_,_,_,_,_  ), 8788 , 156, 4  ), // #600
  INST(Psllq            , ExtRmRi_P          , O(000F00,F3,_,_,_,_,_,_  ), O(000F00,73,6,_,_,_,_,_  ), 8796 , 155, 78 ), // #601
  INST(Psllw            , ExtRmRi_P          , O(000F00,F1,_,_,_,_,_,_  ), O(000F00,71,6,_,_,_,_,_  ), 8827 , 155, 78 ), // #602
  INST(Psrad            , ExtRmRi_P          , O(000F00,E2,_,_,_,_,_,_  ), O(000F00,72,4,_,_,_,_,_  ), 8834 , 155, 78 ), // #603
  INST(Psraw            , ExtRmRi_P          , O(000F00,E1,_,_,_,_,_,_  ), O(000F00,71,4,_,_,_,_,_  ), 8872 , 155, 78 ), // #604
  INST(Psrld            , ExtRmRi_P          , O(000F00,D2,_,_,_,_,_,_  ), O(000F00,72,2,_,_,_,_,_  ), 8879 , 155, 78 ), // #605
  INST(Psrldq           , ExtRmRi            , 0                         , O(660F00,73,3,_,_,_,_,_  ), 8886 , 156, 4  ), // #606
  INST(Psrlq            , ExtRmRi_P          , O(000F00,D3,_,_,_,_,_,_  ), O(000F00,73,2,_,_,_,_,_  ), 8894 , 155, 78 ), // #607
  INST(Psrlw            , ExtRmRi_P          , O(000F00,D1,_,_,_,_,_,_  ), O(000F00,71,2,_,_,_,_,_  ), 8925 , 155, 78 ), // #608
  INST(Psubb            , ExtRm_P            , O(000F00,F8,_,_,_,_,_,_  ), 0                         , 8932 , 136, 78 ), // #609
  INST(Psubd            , ExtRm_P            , O(000F00,FA,_,_,_,_,_,_  ), 0                         , 8939 , 136, 78 ), // #610
  INST(Psubq            , ExtRm_P            , O(000F00,FB,_,_,_,_,_,_  ), 0                         , 8946 , 136, 4  ), // #611
  INST(Psubsb           , ExtRm_P            , O(000F00,E8,_,_,_,_,_,_  ), 0                         , 8953 , 136, 78 ), // #612
  INST(Psubsw           , ExtRm_P            , O(000F00,E9,_,_,_,_,_,_  ), 0                         , 8961 , 136, 78 ), // #613
  INST(Psubusb          , ExtRm_P            , O(000F00,D8,_,_,_,_,_,_  ), 0                         , 8969 , 136, 78 ), // #614
  INST(Psubusw          , ExtRm_P            , O(000F00,D9,_,_,_,_,_,_  ), 0                         , 8978 , 136, 78 ), // #615
  INST(Psubw            , ExtRm_P            , O(000F00,F9,_,_,_,_,_,_  ), 0                         , 8987 , 136, 78 ), // #616
  INST(Pswapd           , Ext3dNow           , O(000F0F,BB,_,_,_,_,_,_  ), 0                         , 2326 , 137, 89 ), // #617
  INST(Ptest            , ExtRm              , O(660F38,17,_,_,_,_,_,_  ), 0                         , 9016 , 5  , 94 ), // #618
  INST(Punpckhbw        , ExtRm_P            , O(000F00,68,_,_,_,_,_,_  ), 0                         , 9099 , 133, 78 ), // #619
  INST(Punpckhdq        , ExtRm_P            , O(000F00,6A,_,_,_,_,_,_  ), 0                         , 9110 , 133, 78 ), // #620
  INST(Punpckhqdq       , ExtRm              , O(660F00,6D,_,_,_,_,_,_  ), 0                         , 9121 , 5  , 4  ), // #621
  INST(Punpckhwd        , ExtRm_P            , O(000F00,69,_,_,_,_,_,_  ), 0                         , 9133 , 133, 78 ), // #622
  INST(Punpcklbw        , ExtRm_P            , O(000F00,60,_,_,_,_,_,_  ), 0                         , 9144 , 133, 78 ), // #623
  INST(Punpckldq        , ExtRm_P            , O(000F00,62,_,_,_,_,_,_  ), 0                         , 9155 , 133, 78 ), // #624
  INST(Punpcklqdq       , ExtRm              , O(660F00,6C,_,_,_,_,_,_  ), 0                         , 9166 , 5  , 4  ), // #625
  INST(Punpcklwd        , ExtRm_P            , O(000F00,61,_,_,_,_,_,_  ), 0                         , 9178 , 133, 78 ), // #626
  INST(Push             , X86Push            , O(000000,FF,6,_,_,_,_,_  ), O(000000,50,_,_,_,_,_,_  ), 2333 , 157, 0  ), // #627
  INST(Pusha            , X86Op              , O(660000,60,_,_,_,_,_,_  ), 0                         , 2338 , 74 , 0  ), // #628
  INST(Pushad           , X86Op              , O(000000,60,_,_,_,_,_,_  ), 0                         , 2344 , 74 , 0  ), // #629
  INST(Pushf            , X86Op              , O(660000,9C,_,_,_,_,_,_  ), 0                         , 2351 , 30 , 0  ), // #630
  INST(Pushfd           , X86Op              , O(000000,9C,_,_,_,_,_,_  ), 0                         , 2357 , 74 , 0  ), // #631
  INST(Pushfq           , X86Op              , O(000000,9C,_,_,_,_,_,_  ), 0                         , 2364 , 153, 0  ), // #632
  INST(Pxor             , ExtRm_P            , O(000F00,EF,_,_,_,_,_,_  ), 0                         , 9189 , 136, 78 ), // #633
  INST(Rcl              , X86Rot             , O(000000,D0,2,_,x,_,_,_  ), 0                         , 2371 , 158, 95 ), // #634
  INST(Rcpps            , ExtRm              , O(000F00,53,_,_,_,_,_,_  ), 0                         , 9317 , 5  , 5  ), // #635
  INST(Rcpss            , ExtRm              , O(F30F00,53,_,_,_,_,_,_  ), 0                         , 9324 , 7  , 5  ), // #636
  INST(Rcr              , X86Rot             , O(000000,D0,3,_,x,_,_,_  ), 0                         , 2375 , 158, 95 ), // #637
  INST(Rdfsbase         , X86M               , O(F30F00,AE,0,_,x,_,_,_  ), 0                         , 2379 , 159, 96 ), // #638
  INST(Rdgsbase         , X86M               , O(F30F00,AE,1,_,x,_,_,_  ), 0                         , 2388 , 159, 96 ), // #639
  INST(Rdmsr            , X86Op              , O(000F00,32,_,_,_,_,_,_  ), 0                         , 2397 , 160, 97 ), // #640
  INST(Rdpid            , X86R_Native        , O(F30F00,C7,7,_,_,_,_,_  ), 0                         , 2403 , 161, 98 ), // #641
  INST(Rdpmc            , X86Op              , O(000F00,33,_,_,_,_,_,_  ), 0                         , 2409 , 160, 0  ), // #642
  INST(Rdrand           , X86M               , O(000F00,C7,6,_,x,_,_,_  ), 0                         , 2415 , 162, 99 ), // #643
  INST(Rdseed           , X86M               , O(000F00,C7,7,_,x,_,_,_  ), 0                         , 2422 , 162, 100), // #644
  INST(Rdtsc            , X86Op              , O(000F00,31,_,_,_,_,_,_  ), 0                         , 2429 , 28 , 101), // #645
  INST(Rdtscp           , X86Op              , O(000F01,F9,_,_,_,_,_,_  ), 0                         , 2435 , 160, 102), // #646
  INST(Ret              , X86Ret             , O(000000,C2,_,_,_,_,_,_  ), 0                         , 2868 , 163, 0  ), // #647
  INST(Rol              , X86Rot             , O(000000,D0,0,_,x,_,_,_  ), 0                         , 2442 , 158, 103), // #648
  INST(Ror              , X86Rot             , O(000000,D0,1,_,x,_,_,_  ), 0                         , 2446 , 158, 103), // #649
  INST(Rorx             , VexRmi_Wx          , V(F20F3A,F0,_,0,x,_,_,_  ), 0                         , 2450 , 164, 81 ), // #650
  INST(Roundpd          , ExtRmi             , O(660F3A,09,_,_,_,_,_,_  ), 0                         , 9419 , 8  , 12 ), // #651
  INST(Roundps          , ExtRmi             , O(660F3A,08,_,_,_,_,_,_  ), 0                         , 9428 , 8  , 12 ), // #652
  INST(Roundsd          , ExtRmi             , O(660F3A,0B,_,_,_,_,_,_  ), 0                         , 9437 , 35 , 12 ), // #653
  INST(Roundss          , ExtRmi             , O(660F3A,0A,_,_,_,_,_,_  ), 0                         , 9446 , 36 , 12 ), // #654
  INST(Rsm              , X86Op              , O(000F00,AA,_,_,_,_,_,_  ), 0                         , 2455 , 74 , 1  ), // #655
  INST(Rsqrtps          , ExtRm              , O(000F00,52,_,_,_,_,_,_  ), 0                         , 9543 , 5  , 5  ), // #656
  INST(Rsqrtss          , ExtRm              , O(F30F00,52,_,_,_,_,_,_  ), 0                         , 9552 , 7  , 5  ), // #657
  INST(Sahf             , X86Op              , O(000000,9E,_,_,_,_,_,_  ), 0                         , 2459 , 89 , 104), // #658
  INST(Sal              , X86Rot             , O(000000,D0,4,_,x,_,_,_  ), 0                         , 2464 , 158, 1  ), // #659
  INST(Sar              , X86Rot             , O(000000,D0,7,_,x,_,_,_  ), 0                         , 2468 , 158, 1  ), // #660
  INST(Sarx             , VexRmv_Wx          , V(F30F38,F7,_,0,x,_,_,_  ), 0                         , 2472 , 13 , 81 ), // #661
  INST(Sbb              , X86Arith           , O(000000,18,3,_,x,_,_,_  ), 0                         , 2477 , 3  , 2  ), // #662
  INST(Scas             , X86StrRm           , O(000000,AE,_,_,_,_,_,_  ), 0                         , 2481 , 165, 36 ), // #663
  INST(Seta             , X86Set             , O(000F00,97,_,_,_,_,_,_  ), 0                         , 2486 , 166, 58 ), // #664
  INST(Setae            , X86Set             , O(000F00,93,_,_,_,_,_,_  ), 0                         , 2491 , 166, 59 ), // #665
  INST(Setb             , X86Set             , O(000F00,92,_,_,_,_,_,_  ), 0                         , 2497 , 166, 59 ), // #666
  INST(Setbe            , X86Set             , O(000F00,96,_,_,_,_,_,_  ), 0                         , 2502 , 166, 58 ), // #667
  INST(Setc             , X86Set             , O(000F00,92,_,_,_,_,_,_  ), 0                         , 2508 , 166, 59 ), // #668
  INST(Sete             , X86Set             , O(000F00,94,_,_,_,_,_,_  ), 0                         , 2513 , 166, 60 ), // #669
  INST(Setg             , X86Set             , O(000F00,9F,_,_,_,_,_,_  ), 0                         , 2518 , 166, 61 ), // #670
  INST(Setge            , X86Set             , O(000F00,9D,_,_,_,_,_,_  ), 0                         , 2523 , 166, 62 ), // #671
  INST(Setl             , X86Set             , O(000F00,9C,_,_,_,_,_,_  ), 0                         , 2529 , 166, 62 ), // #672
  INST(Setle            , X86Set             , O(000F00,9E,_,_,_,_,_,_  ), 0                         , 2534 , 166, 61 ), // #673
  INST(Setna            , X86Set             , O(000F00,96,_,_,_,_,_,_  ), 0                         , 2540 , 166, 58 ), // #674
  INST(Setnae           , X86Set             , O(000F00,92,_,_,_,_,_,_  ), 0                         , 2546 , 166, 59 ), // #675
  INST(Setnb            , X86Set             , O(000F00,93,_,_,_,_,_,_  ), 0                         , 2553 , 166, 59 ), // #676
  INST(Setnbe           , X86Set             , O(000F00,97,_,_,_,_,_,_  ), 0                         , 2559 , 166, 58 ), // #677
  INST(Setnc            , X86Set             , O(000F00,93,_,_,_,_,_,_  ), 0                         , 2566 , 166, 59 ), // #678
  INST(Setne            , X86Set             , O(000F00,95,_,_,_,_,_,_  ), 0                         , 2572 , 166, 60 ), // #679
  INST(Setng            , X86Set             , O(000F00,9E,_,_,_,_,_,_  ), 0                         , 2578 , 166, 61 ), // #680
  INST(Setnge           , X86Set             , O(000F00,9C,_,_,_,_,_,_  ), 0                         , 2584 , 166, 62 ), // #681
  INST(Setnl            , X86Set             , O(000F00,9D,_,_,_,_,_,_  ), 0                         , 2591 , 166, 62 ), // #682
  INST(Setnle           , X86Set             , O(000F00,9F,_,_,_,_,_,_  ), 0                         , 2597 , 166, 61 ), // #683
  INST(Setno            , X86Set             , O(000F00,91,_,_,_,_,_,_  ), 0                         , 2604 , 166, 56 ), // #684
  INST(Setnp            , X86Set             , O(000F00,9B,_,_,_,_,_,_  ), 0                         , 2610 , 166, 63 ), // #685
  INST(Setns            , X86Set             , O(000F00,99,_,_,_,_,_,_  ), 0                         , 2616 , 166, 64 ), // #686
  INST(Setnz            , X86Set             , O(000F00,95,_,_,_,_,_,_  ), 0                         , 2622 , 166, 60 ), // #687
  INST(Seto             , X86Set             , O(000F00,90,_,_,_,_,_,_  ), 0                         , 2628 , 166, 56 ), // #688
  INST(Setp             , X86Set             , O(000F00,9A,_,_,_,_,_,_  ), 0                         , 2633 , 166, 63 ), // #689
  INST(Setpe            , X86Set             , O(000F00,9A,_,_,_,_,_,_  ), 0                         , 2638 , 166, 63 ), // #690
  INST(Setpo            , X86Set             , O(000F00,9B,_,_,_,_,_,_  ), 0                         , 2644 , 166, 63 ), // #691
  INST(Sets             , X86Set             , O(000F00,98,_,_,_,_,_,_  ), 0                         , 2650 , 166, 64 ), // #692
  INST(Setz             , X86Set             , O(000F00,94,_,_,_,_,_,_  ), 0                         , 2655 , 166, 60 ), // #693
  INST(Sfence           , X86Fence           , O(000F00,AE,7,_,_,_,_,_  ), 0                         , 2660 , 30 , 74 ), // #694
  INST(Sgdt             , X86M_Only          , O(000F00,01,0,_,_,_,_,_  ), 0                         , 2667 , 31 , 0  ), // #695
  INST(Sha1msg1         , ExtRm              , O(000F38,C9,_,_,_,_,_,_  ), 0                         , 2672 , 5  , 105), // #696
  INST(Sha1msg2         , ExtRm              , O(000F38,CA,_,_,_,_,_,_  ), 0                         , 2681 , 5  , 105), // #697
  INST(Sha1nexte        , ExtRm              , O(000F38,C8,_,_,_,_,_,_  ), 0                         , 2690 , 5  , 105), // #698
  INST(Sha1rnds4        , ExtRmi             , O(000F3A,CC,_,_,_,_,_,_  ), 0                         , 2700 , 8  , 105), // #699
  INST(Sha256msg1       , ExtRm              , O(000F38,CC,_,_,_,_,_,_  ), 0                         , 2710 , 5  , 105), // #700
  INST(Sha256msg2       , ExtRm              , O(000F38,CD,_,_,_,_,_,_  ), 0                         , 2721 , 5  , 105), // #701
  INST(Sha256rnds2      , ExtRm_XMM0         , O(000F38,CB,_,_,_,_,_,_  ), 0                         , 2732 , 15 , 105), // #702
  INST(Shl              , X86Rot             , O(000000,D0,4,_,x,_,_,_  ), 0                         , 2744 , 158, 1  ), // #703
  INST(Shld             , X86ShldShrd        , O(000F00,A4,_,_,x,_,_,_  ), 0                         , 8588 , 167, 1  ), // #704
  INST(Shlx             , VexRmv_Wx          , V(660F38,F7,_,0,x,_,_,_  ), 0                         , 2748 , 13 , 81 ), // #705
  INST(Shr              , X86Rot             , O(000000,D0,5,_,x,_,_,_  ), 0                         , 2753 , 158, 1  ), // #706
  INST(Shrd             , X86ShldShrd        , O(000F00,AC,_,_,x,_,_,_  ), 0                         , 2757 , 167, 1  ), // #707
  INST(Shrx             , VexRmv_Wx          , V(F20F38,F7,_,0,x,_,_,_  ), 0                         , 2762 , 13 , 81 ), // #708
  INST(Shufpd           , ExtRmi             , O(660F00,C6,_,_,_,_,_,_  ), 0                         , 9813 , 8  , 4  ), // #709
  INST(Shufps           , ExtRmi             , O(000F00,C6,_,_,_,_,_,_  ), 0                         , 9821 , 8  , 5  ), // #710
  INST(Sidt             , X86M_Only          , O(000F00,01,1,_,_,_,_,_  ), 0                         , 2767 , 31 , 0  ), // #711
  INST(Skinit           , X86Op_xAX          , O(000F01,DE,_,_,_,_,_,_  ), 0                         , 2772 , 50 , 106), // #712
  INST(Sldt             , X86M               , O(000F00,00,0,_,_,_,_,_  ), 0                         , 2779 , 168, 0  ), // #713
  INST(Slwpcb           , VexR_Wx            , V(XOP_M9,12,1,0,x,_,_,_  ), 0                         , 2784 , 97 , 72 ), // #714
  INST(Smsw             , X86M               , O(000F00,01,4,_,_,_,_,_  ), 0                         , 2791 , 168, 0  ), // #715
  INST(Sqrtpd           , ExtRm              , O(660F00,51,_,_,_,_,_,_  ), 0                         , 9829 , 5  , 4  ), // #716
  INST(Sqrtps           , ExtRm              , O(000F00,51,_,_,_,_,_,_  ), 0                         , 9544 , 5  , 5  ), // #717
  INST(Sqrtsd           , ExtRm              , O(F20F00,51,_,_,_,_,_,_  ), 0                         , 9845 , 6  , 4  ), // #718
  INST(Sqrtss           , ExtRm              , O(F30F00,51,_,_,_,_,_,_  ), 0                         , 9553 , 7  , 5  ), // #719
  INST(Stac             , X86Op              , O(000F01,CB,_,_,_,_,_,_  ), 0                         , 2796 , 30 , 17 ), // #720
  INST(Stc              , X86Op              , O(000000,F9,_,_,_,_,_,_  ), 0                         , 2801 , 30 , 18 ), // #721
  INST(Std              , X86Op              , O(000000,FD,_,_,_,_,_,_  ), 0                         , 6571 , 30 , 19 ), // #722
  INST(Stgi             , X86Op              , O(000F01,DC,_,_,_,_,_,_  ), 0                         , 2805 , 30 , 106), // #723
  INST(Sti              , X86Op              , O(000000,FB,_,_,_,_,_,_  ), 0                         , 2810 , 30 , 24 ), // #724
  INST(Stmxcsr          , X86M_Only          , O(000F00,AE,3,_,_,_,_,_  ), 0                         , 9861 , 92 , 5  ), // #725
  INST(Stos             , X86StrMr           , O(000000,AA,_,_,_,_,_,_  ), 0                         , 2814 , 169, 0  ), // #726
  INST(Str              , X86M               , O(000F00,00,1,_,_,_,_,_  ), 0                         , 2819 , 168, 0  ), // #727
  INST(Sub              , X86Arith           , O(000000,28,5,_,x,_,_,_  ), 0                         , 821  , 170, 1  ), // #728
  INST(Subpd            , ExtRm              , O(660F00,5C,_,_,_,_,_,_  ), 0                         , 4541 , 5  , 4  ), // #729
  INST(Subps            , ExtRm              , O(000F00,5C,_,_,_,_,_,_  ), 0                         , 4553 , 5  , 5  ), // #730
  INST(Subsd            , ExtRm              , O(F20F00,5C,_,_,_,_,_,_  ), 0                         , 5229 , 6  , 4  ), // #731
  INST(Subss            , ExtRm              , O(F30F00,5C,_,_,_,_,_,_  ), 0                         , 5239 , 7  , 5  ), // #732
  INST(Swapgs           , X86Op              , O(000F01,F8,_,_,_,_,_,_  ), 0                         , 2823 , 153, 0  ), // #733
  INST(Syscall          , X86Op              , O(000F00,05,_,_,_,_,_,_  ), 0                         , 2830 , 153, 0  ), // #734
  INST(Sysenter         , X86Op              , O(000F00,34,_,_,_,_,_,_  ), 0                         , 2838 , 30 , 0  ), // #735
  INST(Sysexit          , X86Op              , O(000F00,35,_,_,_,_,_,_  ), 0                         , 2847 , 30 , 0  ), // #736
  INST(Sysexit64        , X86Op              , O(000F00,35,_,_,_,_,_,_  ), 0                         , 2855 , 30 , 0  ), // #737
  INST(Sysret           , X86Op              , O(000F00,07,_,_,_,_,_,_  ), 0                         , 2865 , 153, 0  ), // #738
  INST(Sysret64         , X86Op              , O(000F00,07,_,_,_,_,_,_  ), 0                         , 2872 , 153, 0  ), // #739
  INST(T1mskc           , VexVm_Wx           , V(XOP_M9,01,7,0,x,_,_,_  ), 0                         , 2881 , 14 , 11 ), // #740
  INST(Test             , X86Test            , O(000000,84,_,_,x,_,_,_  ), O(000000,F6,_,_,x,_,_,_  ), 9017 , 171, 1  ), // #741
  INST(Tzcnt            , X86Rm_Raw66H       , O(F30F00,BC,_,_,x,_,_,_  ), 0                         , 2888 , 22 , 9  ), // #742
  INST(Tzmsk            , VexVm_Wx           , V(XOP_M9,01,4,0,x,_,_,_  ), 0                         , 2894 , 14 , 11 ), // #743
  INST(Ucomisd          , ExtRm              , O(660F00,2E,_,_,_,_,_,_  ), 0                         , 9914 , 6  , 40 ), // #744
  INST(Ucomiss          , ExtRm              , O(000F00,2E,_,_,_,_,_,_  ), 0                         , 9923 , 7  , 41 ), // #745
  INST(Ud2              , X86Op              , O(000F00,0B,_,_,_,_,_,_  ), 0                         , 2900 , 30 , 0  ), // #746
  INST(Unpckhpd         , ExtRm              , O(660F00,15,_,_,_,_,_,_  ), 0                         , 9932 , 5  , 4  ), // #747
  INST(Unpckhps         , ExtRm              , O(000F00,15,_,_,_,_,_,_  ), 0                         , 9942 , 5  , 5  ), // #748
  INST(Unpcklpd         , ExtRm              , O(660F00,14,_,_,_,_,_,_  ), 0                         , 9952 , 5  , 4  ), // #749
  INST(Unpcklps         , ExtRm              , O(000F00,14,_,_,_,_,_,_  ), 0                         , 9962 , 5  , 5  ), // #750
  INST(V4fmaddps        , VexRm_T1_4X        , V(F20F38,9A,_,2,_,0,2,T4X), 0                         , 2904 , 172, 107), // #751
  INST(V4fmaddss        , VexRm_T1_4X        , V(F20F38,9B,_,2,_,0,2,T4X), 0                         , 2914 , 173, 107), // #752
  INST(V4fnmaddps       , VexRm_T1_4X        , V(F20F38,AA,_,2,_,0,2,T4X), 0                         , 2924 , 172, 107), // #753
  INST(V4fnmaddss       , VexRm_T1_4X        , V(F20F38,AB,_,2,_,0,2,T4X), 0                         , 2935 , 173, 107), // #754
  INST(Vaddpd           , VexRvm_Lx          , V(660F00,58,_,x,I,1,4,FV ), 0                         , 2946 , 174, 108), // #755
  INST(Vaddps           , VexRvm_Lx          , V(000F00,58,_,x,I,0,4,FV ), 0                         , 2953 , 175, 108), // #756
  INST(Vaddsd           , VexRvm             , V(F20F00,58,_,I,I,1,3,T1S), 0                         , 2960 , 176, 109), // #757
  INST(Vaddss           , VexRvm             , V(F30F00,58,_,I,I,0,2,T1S), 0                         , 2967 , 177, 109), // #758
  INST(Vaddsubpd        , VexRvm_Lx          , V(660F00,D0,_,x,I,_,_,_  ), 0                         , 2974 , 178, 110), // #759
  INST(Vaddsubps        , VexRvm_Lx          , V(F20F00,D0,_,x,I,_,_,_  ), 0                         , 2984 , 178, 110), // #760
  INST(Vaesdec          , VexRvm_Lx          , V(660F38,DE,_,x,I,_,4,FVM), 0                         , 2994 , 179, 111), // #761
  INST(Vaesdeclast      , VexRvm_Lx          , V(660F38,DF,_,x,I,_,4,FVM), 0                         , 3002 , 179, 111), // #762
  INST(Vaesenc          , VexRvm_Lx          , V(660F38,DC,_,x,I,_,4,FVM), 0                         , 3014 , 179, 111), // #763
  INST(Vaesenclast      , VexRvm_Lx          , V(660F38,DD,_,x,I,_,4,FVM), 0                         , 3022 , 179, 111), // #764
  INST(Vaesimc          , VexRm              , V(660F38,DB,_,0,I,_,_,_  ), 0                         , 3034 , 180, 112), // #765
  INST(Vaeskeygenassist , VexRmi             , V(660F3A,DF,_,0,I,_,_,_  ), 0                         , 3042 , 181, 112), // #766
  INST(Valignd          , VexRvmi_Lx         , V(660F3A,03,_,x,_,0,4,FV ), 0                         , 3059 , 182, 113), // #767
  INST(Valignq          , VexRvmi_Lx         , V(660F3A,03,_,x,_,1,4,FV ), 0                         , 3067 , 183, 113), // #768
  INST(Vandnpd          , VexRvm_Lx          , V(660F00,55,_,x,I,1,4,FV ), 0                         , 3075 , 184, 114), // #769
  INST(Vandnps          , VexRvm_Lx          , V(000F00,55,_,x,I,0,4,FV ), 0                         , 3083 , 185, 114), // #770
  INST(Vandpd           , VexRvm_Lx          , V(660F00,54,_,x,I,1,4,FV ), 0                         , 3091 , 186, 114), // #771
  INST(Vandps           , VexRvm_Lx          , V(000F00,54,_,x,I,0,4,FV ), 0                         , 3098 , 187, 114), // #772
  INST(Vblendmb         , VexRvm_Lx          , V(660F38,66,_,x,_,0,4,FVM), 0                         , 3105 , 188, 115), // #773
  INST(Vblendmd         , VexRvm_Lx          , V(660F38,64,_,x,_,0,4,FV ), 0                         , 3114 , 189, 113), // #774
  INST(Vblendmpd        , VexRvm_Lx          , V(660F38,65,_,x,_,1,4,FV ), 0                         , 3123 , 190, 113), // #775
  INST(Vblendmps        , VexRvm_Lx          , V(660F38,65,_,x,_,0,4,FV ), 0                         , 3133 , 189, 113), // #776
  INST(Vblendmq         , VexRvm_Lx          , V(660F38,64,_,x,_,1,4,FV ), 0                         , 3143 , 190, 113), // #777
  INST(Vblendmw         , VexRvm_Lx          , V(660F38,66,_,x,_,1,4,FVM), 0                         , 3152 , 188, 115), // #778
  INST(Vblendpd         , VexRvmi_Lx         , V(660F3A,0D,_,x,I,_,_,_  ), 0                         , 3161 , 191, 110), // #779
  INST(Vblendps         , VexRvmi_Lx         , V(660F3A,0C,_,x,I,_,_,_  ), 0                         , 3170 , 191, 110), // #780
  INST(Vblendvpd        , VexRvmr_Lx         , V(660F3A,4B,_,x,0,_,_,_  ), 0                         , 3179 , 192, 110), // #781
  INST(Vblendvps        , VexRvmr_Lx         , V(660F3A,4A,_,x,0,_,_,_  ), 0                         , 3189 , 192, 110), // #782
  INST(Vbroadcastf128   , VexRm              , V(660F38,1A,_,1,0,_,_,_  ), 0                         , 3199 , 193, 110), // #783
  INST(Vbroadcastf32x2  , VexRm_Lx           , V(660F38,19,_,x,_,0,3,T2 ), 0                         , 3214 , 194, 116), // #784
  INST(Vbroadcastf32x4  , VexRm_Lx           , V(660F38,1A,_,x,_,0,4,T4 ), 0                         , 3230 , 195, 67 ), // #785
  INST(Vbroadcastf32x8  , VexRm              , V(660F38,1B,_,2,_,0,5,T8 ), 0                         , 3246 , 196, 65 ), // #786
  INST(Vbroadcastf64x2  , VexRm_Lx           , V(660F38,1A,_,x,_,1,4,T2 ), 0                         , 3262 , 195, 116), // #787
  INST(Vbroadcastf64x4  , VexRm              , V(660F38,1B,_,2,_,1,5,T4 ), 0                         , 3278 , 196, 67 ), // #788
  INST(Vbroadcasti128   , VexRm              , V(660F38,5A,_,1,0,_,_,_  ), 0                         , 3294 , 193, 117), // #789
  INST(Vbroadcasti32x2  , VexRm_Lx           , V(660F38,59,_,x,_,0,3,T2 ), 0                         , 3309 , 197, 116), // #790
  INST(Vbroadcasti32x4  , VexRm_Lx           , V(660F38,5A,_,x,_,0,4,T4 ), 0                         , 3325 , 195, 113), // #791
  INST(Vbroadcasti32x8  , VexRm              , V(660F38,5B,_,2,_,0,5,T8 ), 0                         , 3341 , 196, 65 ), // #792
  INST(Vbroadcasti64x2  , VexRm_Lx           , V(660F38,5A,_,x,_,1,4,T2 ), 0                         , 3357 , 195, 116), // #793
  INST(Vbroadcasti64x4  , VexRm              , V(660F38,5B,_,2,_,1,5,T4 ), 0                         , 3373 , 196, 67 ), // #794
  INST(Vbroadcastsd     , VexRm_Lx           , V(660F38,19,_,x,0,1,3,T1S), 0                         , 3389 , 198, 118), // #795
  INST(Vbroadcastss     , VexRm_Lx           , V(660F38,18,_,x,0,0,2,T1S), 0                         , 3402 , 199, 118), // #796
  INST(Vcmppd           , VexRvmi_Lx         , V(660F00,C2,_,x,I,1,4,FV ), 0                         , 3415 , 200, 108), // #797
  INST(Vcmpps           , VexRvmi_Lx         , V(000F00,C2,_,x,I,0,4,FV ), 0                         , 3422 , 201, 108), // #798
  INST(Vcmpsd           , VexRvmi            , V(F20F00,C2,_,I,I,1,3,T1S), 0                         , 3429 , 202, 109), // #799
  INST(Vcmpss           , VexRvmi            , V(F30F00,C2,_,I,I,0,2,T1S), 0                         , 3436 , 203, 109), // #800
  INST(Vcomisd          , VexRm              , V(660F00,2F,_,I,I,1,3,T1S), 0                         , 3443 , 204, 119), // #801
  INST(Vcomiss          , VexRm              , V(000F00,2F,_,I,I,0,2,T1S), 0                         , 3451 , 205, 119), // #802
  INST(Vcompresspd      , VexMr_Lx           , V(660F38,8A,_,x,_,1,3,T1S), 0                         , 3459 , 206, 113), // #803
  INST(Vcompressps      , VexMr_Lx           , V(660F38,8A,_,x,_,0,2,T1S), 0                         , 3471 , 206, 113), // #804
  INST(Vcvtdq2pd        , VexRm_Lx           , V(F30F00,E6,_,x,I,0,3,HV ), 0                         , 3483 , 207, 108), // #805
  INST(Vcvtdq2ps        , VexRm_Lx           , V(000F00,5B,_,x,I,0,4,FV ), 0                         , 3493 , 208, 108), // #806
  INST(Vcvtne2ps2bf16   , VexRvm             , V(F20F38,72,_,_,_,0,_,_  ), 0                         , 3503 , 189, 120), // #807
  INST(Vcvtneps2bf16    , VexRm              , V(F30F38,72,_,_,_,0,_,_  ), 0                         , 3518 , 209, 120), // #808
  INST(Vcvtpd2dq        , VexRm_Lx           , V(F20F00,E6,_,x,I,1,4,FV ), 0                         , 3532 , 210, 108), // #809
  INST(Vcvtpd2ps        , VexRm_Lx           , V(660F00,5A,_,x,I,1,4,FV ), 0                         , 3542 , 210, 108), // #810
  INST(Vcvtpd2qq        , VexRm_Lx           , V(660F00,7B,_,x,_,1,4,FV ), 0                         , 3552 , 211, 116), // #811
  INST(Vcvtpd2udq       , VexRm_Lx           , V(000F00,79,_,x,_,1,4,FV ), 0                         , 3562 , 212, 113), // #812
  INST(Vcvtpd2uqq       , VexRm_Lx           , V(660F00,79,_,x,_,1,4,FV ), 0                         , 3573 , 211, 116), // #813
  INST(Vcvtph2ps        , VexRm_Lx           , V(660F38,13,_,x,0,0,3,HVM), 0                         , 3584 , 213, 121), // #814
  INST(Vcvtps2dq        , VexRm_Lx           , V(660F00,5B,_,x,I,0,4,FV ), 0                         , 3594 , 208, 108), // #815
  INST(Vcvtps2pd        , VexRm_Lx           , V(000F00,5A,_,x,I,0,4,HV ), 0                         , 3604 , 214, 108), // #816
  INST(Vcvtps2ph        , VexMri_Lx          , V(660F3A,1D,_,x,0,0,3,HVM), 0                         , 3614 , 215, 121), // #817
  INST(Vcvtps2qq        , VexRm_Lx           , V(660F00,7B,_,x,_,0,3,HV ), 0                         , 3624 , 216, 116), // #818
  INST(Vcvtps2udq       , VexRm_Lx           , V(000F00,79,_,x,_,0,4,FV ), 0                         , 3634 , 217, 113), // #819
  INST(Vcvtps2uqq       , VexRm_Lx           , V(660F00,79,_,x,_,0,3,HV ), 0                         , 3645 , 216, 116), // #820
  INST(Vcvtqq2pd        , VexRm_Lx           , V(F30F00,E6,_,x,_,1,4,FV ), 0                         , 3656 , 211, 116), // #821
  INST(Vcvtqq2ps        , VexRm_Lx           , V(000F00,5B,_,x,_,1,4,FV ), 0                         , 3666 , 212, 116), // #822
  INST(Vcvtsd2si        , VexRm_Wx           , V(F20F00,2D,_,I,x,x,3,T1F), 0                         , 3676 , 218, 109), // #823
  INST(Vcvtsd2ss        , VexRvm             , V(F20F00,5A,_,I,I,1,3,T1S), 0                         , 3686 , 176, 109), // #824
  INST(Vcvtsd2usi       , VexRm_Wx           , V(F20F00,79,_,I,_,x,3,T1F), 0                         , 3696 , 219, 67 ), // #825
  INST(Vcvtsi2sd        , VexRvm_Wx          , V(F20F00,2A,_,I,x,x,2,T1W), 0                         , 3707 , 220, 109), // #826
  INST(Vcvtsi2ss        , VexRvm_Wx          , V(F30F00,2A,_,I,x,x,2,T1W), 0                         , 3717 , 220, 109), // #827
  INST(Vcvtss2sd        , VexRvm             , V(F30F00,5A,_,I,I,0,2,T1S), 0                         , 3727 , 221, 109), // #828
  INST(Vcvtss2si        , VexRm_Wx           , V(F30F00,2D,_,I,x,x,2,T1F), 0                         , 3737 , 222, 109), // #829
  INST(Vcvtss2usi       , VexRm_Wx           , V(F30F00,79,_,I,_,x,2,T1F), 0                         , 3747 , 223, 67 ), // #830
  INST(Vcvttpd2dq       , VexRm_Lx           , V(660F00,E6,_,x,I,1,4,FV ), 0                         , 3758 , 224, 108), // #831
  INST(Vcvttpd2qq       , VexRm_Lx           , V(660F00,7A,_,x,_,1,4,FV ), 0                         , 3769 , 225, 113), // #832
  INST(Vcvttpd2udq      , VexRm_Lx           , V(000F00,78,_,x,_,1,4,FV ), 0                         , 3780 , 226, 113), // #833
  INST(Vcvttpd2uqq      , VexRm_Lx           , V(660F00,78,_,x,_,1,4,FV ), 0                         , 3792 , 225, 116), // #834
  INST(Vcvttps2dq       , VexRm_Lx           , V(F30F00,5B,_,x,I,0,4,FV ), 0                         , 3804 , 227, 108), // #835
  INST(Vcvttps2qq       , VexRm_Lx           , V(660F00,7A,_,x,_,0,3,HV ), 0                         , 3815 , 228, 116), // #836
  INST(Vcvttps2udq      , VexRm_Lx           , V(000F00,78,_,x,_,0,4,FV ), 0                         , 3826 , 229, 113), // #837
  INST(Vcvttps2uqq      , VexRm_Lx           , V(660F00,78,_,x,_,0,3,HV ), 0                         , 3838 , 228, 116), // #838
  INST(Vcvttsd2si       , VexRm_Wx           , V(F20F00,2C,_,I,x,x,3,T1F), 0                         , 3850 , 230, 109), // #839
  INST(Vcvttsd2usi      , VexRm_Wx           , V(F20F00,78,_,I,_,x,3,T1F), 0                         , 3861 , 231, 67 ), // #840
  INST(Vcvttss2si       , VexRm_Wx           , V(F30F00,2C,_,I,x,x,2,T1F), 0                         , 3873 , 232, 109), // #841
  INST(Vcvttss2usi      , VexRm_Wx           , V(F30F00,78,_,I,_,x,2,T1F), 0                         , 3884 , 233, 67 ), // #842
  INST(Vcvtudq2pd       , VexRm_Lx           , V(F30F00,7A,_,x,_,0,3,HV ), 0                         , 3896 , 234, 113), // #843
  INST(Vcvtudq2ps       , VexRm_Lx           , V(F20F00,7A,_,x,_,0,4,FV ), 0                         , 3907 , 217, 113), // #844
  INST(Vcvtuqq2pd       , VexRm_Lx           , V(F30F00,7A,_,x,_,1,4,FV ), 0                         , 3918 , 211, 116), // #845
  INST(Vcvtuqq2ps       , VexRm_Lx           , V(F20F00,7A,_,x,_,1,4,FV ), 0                         , 3929 , 212, 116), // #846
  INST(Vcvtusi2sd       , VexRvm_Wx          , V(F20F00,7B,_,I,_,x,2,T1W), 0                         , 3940 , 235, 67 ), // #847
  INST(Vcvtusi2ss       , VexRvm_Wx          , V(F30F00,7B,_,I,_,x,2,T1W), 0                         , 3951 , 235, 67 ), // #848
  INST(Vdbpsadbw        , VexRvmi_Lx         , V(660F3A,42,_,x,_,0,4,FVM), 0                         , 3962 , 236, 115), // #849
  INST(Vdivpd           , VexRvm_Lx          , V(660F00,5E,_,x,I,1,4,FV ), 0                         , 3972 , 174, 108), // #850
  INST(Vdivps           , VexRvm_Lx          , V(000F00,5E,_,x,I,0,4,FV ), 0                         , 3979 , 175, 108), // #851
  INST(Vdivsd           , VexRvm             , V(F20F00,5E,_,I,I,1,3,T1S), 0                         , 3986 , 176, 109), // #852
  INST(Vdivss           , VexRvm             , V(F30F00,5E,_,I,I,0,2,T1S), 0                         , 3993 , 177, 109), // #853
  INST(Vdpbf16ps        , VexRvm             , V(F30F38,52,_,_,_,0,_,_  ), 0                         , 4000 , 189, 120), // #854
  INST(Vdppd            , VexRvmi_Lx         , V(660F3A,41,_,x,I,_,_,_  ), 0                         , 4010 , 237, 110), // #855
  INST(Vdpps            , VexRvmi_Lx         , V(660F3A,40,_,x,I,_,_,_  ), 0                         , 4016 , 191, 110), // #856
  INST(Verr             , X86M_NoSize        , O(000F00,00,4,_,_,_,_,_  ), 0                         , 4022 , 96 , 10 ), // #857
  INST(Verw             , X86M_NoSize        , O(000F00,00,5,_,_,_,_,_  ), 0                         , 4027 , 96 , 10 ), // #858
  INST(Vexp2pd          , VexRm              , V(660F38,C8,_,2,_,1,4,FV ), 0                         , 4032 , 238, 122), // #859
  INST(Vexp2ps          , VexRm              , V(660F38,C8,_,2,_,0,4,FV ), 0                         , 4040 , 239, 122), // #860
  INST(Vexpandpd        , VexRm_Lx           , V(660F38,88,_,x,_,1,3,T1S), 0                         , 4048 , 240, 113), // #861
  INST(Vexpandps        , VexRm_Lx           , V(660F38,88,_,x,_,0,2,T1S), 0                         , 4058 , 240, 113), // #862
  INST(Vextractf128     , VexMri             , V(660F3A,19,_,1,0,_,_,_  ), 0                         , 4068 , 241, 110), // #863
  INST(Vextractf32x4    , VexMri_Lx          , V(660F3A,19,_,x,_,0,4,T4 ), 0                         , 4081 , 242, 113), // #864
  INST(Vextractf32x8    , VexMri             , V(660F3A,1B,_,2,_,0,5,T8 ), 0                         , 4095 , 243, 65 ), // #865
  INST(Vextractf64x2    , VexMri_Lx          , V(660F3A,19,_,x,_,1,4,T2 ), 0                         , 4109 , 242, 116), // #866
  INST(Vextractf64x4    , VexMri             , V(660F3A,1B,_,2,_,1,5,T4 ), 0                         , 4123 , 243, 67 ), // #867
  INST(Vextracti128     , VexMri             , V(660F3A,39,_,1,0,_,_,_  ), 0                         , 4137 , 241, 117), // #868
  INST(Vextracti32x4    , VexMri_Lx          , V(660F3A,39,_,x,_,0,4,T4 ), 0                         , 4150 , 242, 113), // #869
  INST(Vextracti32x8    , VexMri             , V(660F3A,3B,_,2,_,0,5,T8 ), 0                         , 4164 , 243, 65 ), // #870
  INST(Vextracti64x2    , VexMri_Lx          , V(660F3A,39,_,x,_,1,4,T2 ), 0                         , 4178 , 242, 116), // #871
  INST(Vextracti64x4    , VexMri             , V(660F3A,3B,_,2,_,1,5,T4 ), 0                         , 4192 , 243, 67 ), // #872
  INST(Vextractps       , VexMri             , V(660F3A,17,_,0,I,I,2,T1S), 0                         , 4206 , 244, 109), // #873
  INST(Vfixupimmpd      , VexRvmi_Lx         , V(660F3A,54,_,x,_,1,4,FV ), 0                         , 4217 , 245, 113), // #874
  INST(Vfixupimmps      , VexRvmi_Lx         , V(660F3A,54,_,x,_,0,4,FV ), 0                         , 4229 , 246, 113), // #875
  INST(Vfixupimmsd      , VexRvmi            , V(660F3A,55,_,I,_,1,3,T1S), 0                         , 4241 , 247, 67 ), // #876
  INST(Vfixupimmss      , VexRvmi            , V(660F3A,55,_,I,_,0,2,T1S), 0                         , 4253 , 248, 67 ), // #877
  INST(Vfmadd132pd      , VexRvm_Lx          , V(660F38,98,_,x,1,1,4,FV ), 0                         , 4265 , 174, 123), // #878
  INST(Vfmadd132ps      , VexRvm_Lx          , V(660F38,98,_,x,0,0,4,FV ), 0                         , 4277 , 175, 123), // #879
  INST(Vfmadd132sd      , VexRvm             , V(660F38,99,_,I,1,1,3,T1S), 0                         , 4289 , 176, 124), // #880
  INST(Vfmadd132ss      , VexRvm             , V(660F38,99,_,I,0,0,2,T1S), 0                         , 4301 , 177, 124), // #881
  INST(Vfmadd213pd      , VexRvm_Lx          , V(660F38,A8,_,x,1,1,4,FV ), 0                         , 4313 , 174, 123), // #882
  INST(Vfmadd213ps      , VexRvm_Lx          , V(660F38,A8,_,x,0,0,4,FV ), 0                         , 4325 , 175, 123), // #883
  INST(Vfmadd213sd      , VexRvm             , V(660F38,A9,_,I,1,1,3,T1S), 0                         , 4337 , 176, 124), // #884
  INST(Vfmadd213ss      , VexRvm             , V(660F38,A9,_,I,0,0,2,T1S), 0                         , 4349 , 177, 124), // #885
  INST(Vfmadd231pd      , VexRvm_Lx          , V(660F38,B8,_,x,1,1,4,FV ), 0                         , 4361 , 174, 123), // #886
  INST(Vfmadd231ps      , VexRvm_Lx          , V(660F38,B8,_,x,0,0,4,FV ), 0                         , 4373 , 175, 123), // #887
  INST(Vfmadd231sd      , VexRvm             , V(660F38,B9,_,I,1,1,3,T1S), 0                         , 4385 , 176, 124), // #888
  INST(Vfmadd231ss      , VexRvm             , V(660F38,B9,_,I,0,0,2,T1S), 0                         , 4397 , 177, 124), // #889
  INST(Vfmaddpd         , Fma4_Lx            , V(660F3A,69,_,x,x,_,_,_  ), 0                         , 4409 , 249, 125), // #890
  INST(Vfmaddps         , Fma4_Lx            , V(660F3A,68,_,x,x,_,_,_  ), 0                         , 4418 , 249, 125), // #891
  INST(Vfmaddsd         , Fma4               , V(660F3A,6B,_,0,x,_,_,_  ), 0                         , 4427 , 250, 125), // #892
  INST(Vfmaddss         , Fma4               , V(660F3A,6A,_,0,x,_,_,_  ), 0                         , 4436 , 251, 125), // #893
  INST(Vfmaddsub132pd   , VexRvm_Lx          , V(660F38,96,_,x,1,1,4,FV ), 0                         , 4445 , 174, 123), // #894
  INST(Vfmaddsub132ps   , VexRvm_Lx          , V(660F38,96,_,x,0,0,4,FV ), 0                         , 4460 , 175, 123), // #895
  INST(Vfmaddsub213pd   , VexRvm_Lx          , V(660F38,A6,_,x,1,1,4,FV ), 0                         , 4475 , 174, 123), // #896
  INST(Vfmaddsub213ps   , VexRvm_Lx          , V(660F38,A6,_,x,0,0,4,FV ), 0                         , 4490 , 175, 123), // #897
  INST(Vfmaddsub231pd   , VexRvm_Lx          , V(660F38,B6,_,x,1,1,4,FV ), 0                         , 4505 , 174, 123), // #898
  INST(Vfmaddsub231ps   , VexRvm_Lx          , V(660F38,B6,_,x,0,0,4,FV ), 0                         , 4520 , 175, 123), // #899
  INST(Vfmaddsubpd      , Fma4_Lx            , V(660F3A,5D,_,x,x,_,_,_  ), 0                         , 4535 , 249, 125), // #900
  INST(Vfmaddsubps      , Fma4_Lx            , V(660F3A,5C,_,x,x,_,_,_  ), 0                         , 4547 , 249, 125), // #901
  INST(Vfmsub132pd      , VexRvm_Lx          , V(660F38,9A,_,x,1,1,4,FV ), 0                         , 4559 , 174, 123), // #902
  INST(Vfmsub132ps      , VexRvm_Lx          , V(660F38,9A,_,x,0,0,4,FV ), 0                         , 4571 , 175, 123), // #903
  INST(Vfmsub132sd      , VexRvm             , V(660F38,9B,_,I,1,1,3,T1S), 0                         , 4583 , 176, 124), // #904
  INST(Vfmsub132ss      , VexRvm             , V(660F38,9B,_,I,0,0,2,T1S), 0                         , 4595 , 177, 124), // #905
  INST(Vfmsub213pd      , VexRvm_Lx          , V(660F38,AA,_,x,1,1,4,FV ), 0                         , 4607 , 174, 123), // #906
  INST(Vfmsub213ps      , VexRvm_Lx          , V(660F38,AA,_,x,0,0,4,FV ), 0                         , 4619 , 175, 123), // #907
  INST(Vfmsub213sd      , VexRvm             , V(660F38,AB,_,I,1,1,3,T1S), 0                         , 4631 , 176, 124), // #908
  INST(Vfmsub213ss      , VexRvm             , V(660F38,AB,_,I,0,0,2,T1S), 0                         , 4643 , 177, 124), // #909
  INST(Vfmsub231pd      , VexRvm_Lx          , V(660F38,BA,_,x,1,1,4,FV ), 0                         , 4655 , 174, 123), // #910
  INST(Vfmsub231ps      , VexRvm_Lx          , V(660F38,BA,_,x,0,0,4,FV ), 0                         , 4667 , 175, 123), // #911
  INST(Vfmsub231sd      , VexRvm             , V(660F38,BB,_,I,1,1,3,T1S), 0                         , 4679 , 176, 124), // #912
  INST(Vfmsub231ss      , VexRvm             , V(660F38,BB,_,I,0,0,2,T1S), 0                         , 4691 , 177, 124), // #913
  INST(Vfmsubadd132pd   , VexRvm_Lx          , V(660F38,97,_,x,1,1,4,FV ), 0                         , 4703 , 174, 123), // #914
  INST(Vfmsubadd132ps   , VexRvm_Lx          , V(660F38,97,_,x,0,0,4,FV ), 0                         , 4718 , 175, 123), // #915
  INST(Vfmsubadd213pd   , VexRvm_Lx          , V(660F38,A7,_,x,1,1,4,FV ), 0                         , 4733 , 174, 123), // #916
  INST(Vfmsubadd213ps   , VexRvm_Lx          , V(660F38,A7,_,x,0,0,4,FV ), 0                         , 4748 , 175, 123), // #917
  INST(Vfmsubadd231pd   , VexRvm_Lx          , V(660F38,B7,_,x,1,1,4,FV ), 0                         , 4763 , 174, 123), // #918
  INST(Vfmsubadd231ps   , VexRvm_Lx          , V(660F38,B7,_,x,0,0,4,FV ), 0                         , 4778 , 175, 123), // #919
  INST(Vfmsubaddpd      , Fma4_Lx            , V(660F3A,5F,_,x,x,_,_,_  ), 0                         , 4793 , 249, 125), // #920
  INST(Vfmsubaddps      , Fma4_Lx            , V(660F3A,5E,_,x,x,_,_,_  ), 0                         , 4805 , 249, 125), // #921
  INST(Vfmsubpd         , Fma4_Lx            , V(660F3A,6D,_,x,x,_,_,_  ), 0                         , 4817 , 249, 125), // #922
  INST(Vfmsubps         , Fma4_Lx            , V(660F3A,6C,_,x,x,_,_,_  ), 0                         , 4826 , 249, 125), // #923
  INST(Vfmsubsd         , Fma4               , V(660F3A,6F,_,0,x,_,_,_  ), 0                         , 4835 , 250, 125), // #924
  INST(Vfmsubss         , Fma4               , V(660F3A,6E,_,0,x,_,_,_  ), 0                         , 4844 , 251, 125), // #925
  INST(Vfnmadd132pd     , VexRvm_Lx          , V(660F38,9C,_,x,1,1,4,FV ), 0                         , 4853 , 174, 123), // #926
  INST(Vfnmadd132ps     , VexRvm_Lx          , V(660F38,9C,_,x,0,0,4,FV ), 0                         , 4866 , 175, 123), // #927
  INST(Vfnmadd132sd     , VexRvm             , V(660F38,9D,_,I,1,1,3,T1S), 0                         , 4879 , 176, 124), // #928
  INST(Vfnmadd132ss     , VexRvm             , V(660F38,9D,_,I,0,0,2,T1S), 0                         , 4892 , 177, 124), // #929
  INST(Vfnmadd213pd     , VexRvm_Lx          , V(660F38,AC,_,x,1,1,4,FV ), 0                         , 4905 , 174, 123), // #930
  INST(Vfnmadd213ps     , VexRvm_Lx          , V(660F38,AC,_,x,0,0,4,FV ), 0                         , 4918 , 175, 123), // #931
  INST(Vfnmadd213sd     , VexRvm             , V(660F38,AD,_,I,1,1,3,T1S), 0                         , 4931 , 176, 124), // #932
  INST(Vfnmadd213ss     , VexRvm             , V(660F38,AD,_,I,0,0,2,T1S), 0                         , 4944 , 177, 124), // #933
  INST(Vfnmadd231pd     , VexRvm_Lx          , V(660F38,BC,_,x,1,1,4,FV ), 0                         , 4957 , 174, 123), // #934
  INST(Vfnmadd231ps     , VexRvm_Lx          , V(660F38,BC,_,x,0,0,4,FV ), 0                         , 4970 , 175, 123), // #935
  INST(Vfnmadd231sd     , VexRvm             , V(660F38,BC,_,I,1,1,3,T1S), 0                         , 4983 , 176, 124), // #936
  INST(Vfnmadd231ss     , VexRvm             , V(660F38,BC,_,I,0,0,2,T1S), 0                         , 4996 , 177, 124), // #937
  INST(Vfnmaddpd        , Fma4_Lx            , V(660F3A,79,_,x,x,_,_,_  ), 0                         , 5009 , 249, 125), // #938
  INST(Vfnmaddps        , Fma4_Lx            , V(660F3A,78,_,x,x,_,_,_  ), 0                         , 5019 , 249, 125), // #939
  INST(Vfnmaddsd        , Fma4               , V(660F3A,7B,_,0,x,_,_,_  ), 0                         , 5029 , 250, 125), // #940
  INST(Vfnmaddss        , Fma4               , V(660F3A,7A,_,0,x,_,_,_  ), 0                         , 5039 , 251, 125), // #941
  INST(Vfnmsub132pd     , VexRvm_Lx          , V(660F38,9E,_,x,1,1,4,FV ), 0                         , 5049 , 174, 123), // #942
  INST(Vfnmsub132ps     , VexRvm_Lx          , V(660F38,9E,_,x,0,0,4,FV ), 0                         , 5062 , 175, 123), // #943
  INST(Vfnmsub132sd     , VexRvm             , V(660F38,9F,_,I,1,1,3,T1S), 0                         , 5075 , 176, 124), // #944
  INST(Vfnmsub132ss     , VexRvm             , V(660F38,9F,_,I,0,0,2,T1S), 0                         , 5088 , 177, 124), // #945
  INST(Vfnmsub213pd     , VexRvm_Lx          , V(660F38,AE,_,x,1,1,4,FV ), 0                         , 5101 , 174, 123), // #946
  INST(Vfnmsub213ps     , VexRvm_Lx          , V(660F38,AE,_,x,0,0,4,FV ), 0                         , 5114 , 175, 123), // #947
  INST(Vfnmsub213sd     , VexRvm             , V(660F38,AF,_,I,1,1,3,T1S), 0                         , 5127 , 176, 124), // #948
  INST(Vfnmsub213ss     , VexRvm             , V(660F38,AF,_,I,0,0,2,T1S), 0                         , 5140 , 177, 124), // #949
  INST(Vfnmsub231pd     , VexRvm_Lx          , V(660F38,BE,_,x,1,1,4,FV ), 0                         , 5153 , 174, 123), // #950
  INST(Vfnmsub231ps     , VexRvm_Lx          , V(660F38,BE,_,x,0,0,4,FV ), 0                         , 5166 , 175, 123), // #951
  INST(Vfnmsub231sd     , VexRvm             , V(660F38,BF,_,I,1,1,3,T1S), 0                         , 5179 , 176, 124), // #952
  INST(Vfnmsub231ss     , VexRvm             , V(660F38,BF,_,I,0,0,2,T1S), 0                         , 5192 , 177, 124), // #953
  INST(Vfnmsubpd        , Fma4_Lx            , V(660F3A,7D,_,x,x,_,_,_  ), 0                         , 5205 , 249, 125), // #954
  INST(Vfnmsubps        , Fma4_Lx            , V(660F3A,7C,_,x,x,_,_,_  ), 0                         , 5215 , 249, 125), // #955
  INST(Vfnmsubsd        , Fma4               , V(660F3A,7F,_,0,x,_,_,_  ), 0                         , 5225 , 250, 125), // #956
  INST(Vfnmsubss        , Fma4               , V(660F3A,7E,_,0,x,_,_,_  ), 0                         , 5235 , 251, 125), // #957
  INST(Vfpclasspd       , VexRmi_Lx          , V(660F3A,66,_,x,_,1,4,FV ), 0                         , 5245 , 252, 116), // #958
  INST(Vfpclassps       , VexRmi_Lx          , V(660F3A,66,_,x,_,0,4,FV ), 0                         , 5256 , 253, 116), // #959
  INST(Vfpclasssd       , VexRmi_Lx          , V(660F3A,67,_,I,_,1,3,T1S), 0                         , 5267 , 254, 65 ), // #960
  INST(Vfpclassss       , VexRmi_Lx          , V(660F3A,67,_,I,_,0,2,T1S), 0                         , 5278 , 255, 65 ), // #961
  INST(Vfrczpd          , VexRm_Lx           , V(XOP_M9,81,_,x,0,_,_,_  ), 0                         , 5289 , 256, 126), // #962
  INST(Vfrczps          , VexRm_Lx           , V(XOP_M9,80,_,x,0,_,_,_  ), 0                         , 5297 , 256, 126), // #963
  INST(Vfrczsd          , VexRm              , V(XOP_M9,83,_,0,0,_,_,_  ), 0                         , 5305 , 257, 126), // #964
  INST(Vfrczss          , VexRm              , V(XOP_M9,82,_,0,0,_,_,_  ), 0                         , 5313 , 258, 126), // #965
  INST(Vgatherdpd       , VexRmvRm_VM        , V(660F38,92,_,x,1,_,_,_  ), V(660F38,92,_,x,_,1,3,T1S), 5321 , 259, 127), // #966
  INST(Vgatherdps       , VexRmvRm_VM        , V(660F38,92,_,x,0,_,_,_  ), V(660F38,92,_,x,_,0,2,T1S), 5332 , 260, 127), // #967
  INST(Vgatherpf0dpd    , VexM_VM            , V(660F38,C6,1,2,_,1,3,T1S), 0                         , 5343 , 261, 128), // #968
  INST(Vgatherpf0dps    , VexM_VM            , V(660F38,C6,1,2,_,0,2,T1S), 0                         , 5357 , 262, 128), // #969
  INST(Vgatherpf0qpd    , VexM_VM            , V(660F38,C7,1,2,_,1,3,T1S), 0                         , 5371 , 263, 128), // #970
  INST(Vgatherpf0qps    , VexM_VM            , V(660F38,C7,1,2,_,0,2,T1S), 0                         , 5385 , 263, 128), // #971
  INST(Vgatherpf1dpd    , VexM_VM            , V(660F38,C6,2,2,_,1,3,T1S), 0                         , 5399 , 261, 128), // #972
  INST(Vgatherpf1dps    , VexM_VM            , V(660F38,C6,2,2,_,0,2,T1S), 0                         , 5413 , 262, 128), // #973
  INST(Vgatherpf1qpd    , VexM_VM            , V(660F38,C7,2,2,_,1,3,T1S), 0                         , 5427 , 263, 128), // #974
  INST(Vgatherpf1qps    , VexM_VM            , V(660F38,C7,2,2,_,0,2,T1S), 0                         , 5441 , 263, 128), // #975
  INST(Vgatherqpd       , VexRmvRm_VM        , V(660F38,93,_,x,1,_,_,_  ), V(660F38,93,_,x,_,1,3,T1S), 5455 , 264, 127), // #976
  INST(Vgatherqps       , VexRmvRm_VM        , V(660F38,93,_,x,0,_,_,_  ), V(660F38,93,_,x,_,0,2,T1S), 5466 , 265, 127), // #977
  INST(Vgetexppd        , VexRm_Lx           , V(660F38,42,_,x,_,1,4,FV ), 0                         , 5477 , 225, 113), // #978
  INST(Vgetexpps        , VexRm_Lx           , V(660F38,42,_,x,_,0,4,FV ), 0                         , 5487 , 229, 113), // #979
  INST(Vgetexpsd        , VexRvm             , V(660F38,43,_,I,_,1,3,T1S), 0                         , 5497 , 266, 67 ), // #980
  INST(Vgetexpss        , VexRvm             , V(660F38,43,_,I,_,0,2,T1S), 0                         , 5507 , 267, 67 ), // #981
  INST(Vgetmantpd       , VexRmi_Lx          , V(660F3A,26,_,x,_,1,4,FV ), 0                         , 5517 , 268, 113), // #982
  INST(Vgetmantps       , VexRmi_Lx          , V(660F3A,26,_,x,_,0,4,FV ), 0                         , 5528 , 269, 113), // #983
  INST(Vgetmantsd       , VexRvmi            , V(660F3A,27,_,I,_,1,3,T1S), 0                         , 5539 , 247, 67 ), // #984
  INST(Vgetmantss       , VexRvmi            , V(660F3A,27,_,I,_,0,2,T1S), 0                         , 5550 , 248, 67 ), // #985
  INST(Vgf2p8affineinvqb, VexRvmi_Lx         , V(660F3A,CF,_,x,1,1,_,FV ), 0                         , 5561 , 270, 129), // #986
  INST(Vgf2p8affineqb   , VexRvmi_Lx         , V(660F3A,CE,_,x,1,1,_,FV ), 0                         , 5579 , 270, 129), // #987
  INST(Vgf2p8mulb       , VexRvm_Lx          , V(660F38,CF,_,x,0,0,_,FV ), 0                         , 5594 , 271, 129), // #988
  INST(Vhaddpd          , VexRvm_Lx          , V(660F00,7C,_,x,I,_,_,_  ), 0                         , 5605 , 178, 110), // #989
  INST(Vhaddps          , VexRvm_Lx          , V(F20F00,7C,_,x,I,_,_,_  ), 0                         , 5613 , 178, 110), // #990
  INST(Vhsubpd          , VexRvm_Lx          , V(660F00,7D,_,x,I,_,_,_  ), 0                         , 5621 , 178, 110), // #991
  INST(Vhsubps          , VexRvm_Lx          , V(F20F00,7D,_,x,I,_,_,_  ), 0                         , 5629 , 178, 110), // #992
  INST(Vinsertf128      , VexRvmi            , V(660F3A,18,_,1,0,_,_,_  ), 0                         , 5637 , 272, 110), // #993
  INST(Vinsertf32x4     , VexRvmi_Lx         , V(660F3A,18,_,x,_,0,4,T4 ), 0                         , 5649 , 273, 113), // #994
  INST(Vinsertf32x8     , VexRvmi            , V(660F3A,1A,_,2,_,0,5,T8 ), 0                         , 5662 , 274, 65 ), // #995
  INST(Vinsertf64x2     , VexRvmi_Lx         , V(660F3A,18,_,x,_,1,4,T2 ), 0                         , 5675 , 273, 116), // #996
  INST(Vinsertf64x4     , VexRvmi            , V(660F3A,1A,_,2,_,1,5,T4 ), 0                         , 5688 , 274, 67 ), // #997
  INST(Vinserti128      , VexRvmi            , V(660F3A,38,_,1,0,_,_,_  ), 0                         , 5701 , 272, 117), // #998
  INST(Vinserti32x4     , VexRvmi_Lx         , V(660F3A,38,_,x,_,0,4,T4 ), 0                         , 5713 , 273, 113), // #999
  INST(Vinserti32x8     , VexRvmi            , V(660F3A,3A,_,2,_,0,5,T8 ), 0                         , 5726 , 274, 65 ), // #1000
  INST(Vinserti64x2     , VexRvmi_Lx         , V(660F3A,38,_,x,_,1,4,T2 ), 0                         , 5739 , 273, 116), // #1001
  INST(Vinserti64x4     , VexRvmi            , V(660F3A,3A,_,2,_,1,5,T4 ), 0                         , 5752 , 274, 67 ), // #1002
  INST(Vinsertps        , VexRvmi            , V(660F3A,21,_,0,I,0,2,T1S), 0                         , 5765 , 275, 109), // #1003
  INST(Vlddqu           , VexRm_Lx           , V(F20F00,F0,_,x,I,_,_,_  ), 0                         , 5775 , 276, 110), // #1004
  INST(Vldmxcsr         , VexM               , V(000F00,AE,2,0,I,_,_,_  ), 0                         , 5782 , 277, 110), // #1005
  INST(Vmaskmovdqu      , VexRm_ZDI          , V(660F00,F7,_,0,I,_,_,_  ), 0                         , 5791 , 278, 110), // #1006
  INST(Vmaskmovpd       , VexRvmMvr_Lx       , V(660F38,2D,_,x,0,_,_,_  ), V(660F38,2F,_,x,0,_,_,_  ), 5803 , 279, 110), // #1007
  INST(Vmaskmovps       , VexRvmMvr_Lx       , V(660F38,2C,_,x,0,_,_,_  ), V(660F38,2E,_,x,0,_,_,_  ), 5814 , 279, 110), // #1008
  INST(Vmaxpd           , VexRvm_Lx          , V(660F00,5F,_,x,I,1,4,FV ), 0                         , 5825 , 280, 108), // #1009
  INST(Vmaxps           , VexRvm_Lx          , V(000F00,5F,_,x,I,0,4,FV ), 0                         , 5832 , 281, 108), // #1010
  INST(Vmaxsd           , VexRvm             , V(F20F00,5F,_,I,I,1,3,T1S), 0                         , 5839 , 282, 108), // #1011
  INST(Vmaxss           , VexRvm             , V(F30F00,5F,_,I,I,0,2,T1S), 0                         , 5846 , 221, 108), // #1012
  INST(Vmcall           , X86Op              , O(000F01,C1,_,_,_,_,_,_  ), 0                         , 5853 , 30 , 57 ), // #1013
  INST(Vmclear          , X86M_Only          , O(660F00,C7,6,_,_,_,_,_  ), 0                         , 5860 , 283, 57 ), // #1014
  INST(Vmfunc           , X86Op              , O(000F01,D4,_,_,_,_,_,_  ), 0                         , 5868 , 30 , 57 ), // #1015
  INST(Vminpd           , VexRvm_Lx          , V(660F00,5D,_,x,I,1,4,FV ), 0                         , 5875 , 280, 108), // #1016
  INST(Vminps           , VexRvm_Lx          , V(000F00,5D,_,x,I,0,4,FV ), 0                         , 5882 , 281, 108), // #1017
  INST(Vminsd           , VexRvm             , V(F20F00,5D,_,I,I,1,3,T1S), 0                         , 5889 , 282, 108), // #1018
  INST(Vminss           , VexRvm             , V(F30F00,5D,_,I,I,0,2,T1S), 0                         , 5896 , 221, 108), // #1019
  INST(Vmlaunch         , X86Op              , O(000F01,C2,_,_,_,_,_,_  ), 0                         , 5903 , 30 , 57 ), // #1020
  INST(Vmload           , X86Op_xAX          , O(000F01,DA,_,_,_,_,_,_  ), 0                         , 5912 , 284, 23 ), // #1021
  INST(Vmmcall          , X86Op              , O(000F01,D9,_,_,_,_,_,_  ), 0                         , 5919 , 30 , 23 ), // #1022
  INST(Vmovapd          , VexRmMr_Lx         , V(660F00,28,_,x,I,1,4,FVM), V(660F00,29,_,x,I,1,4,FVM), 5927 , 285, 108), // #1023
  INST(Vmovaps          , VexRmMr_Lx         , V(000F00,28,_,x,I,0,4,FVM), V(000F00,29,_,x,I,0,4,FVM), 5935 , 285, 108), // #1024
  INST(Vmovd            , VexMovdMovq        , V(660F00,6E,_,0,0,0,2,T1S), V(660F00,7E,_,0,0,0,2,T1S), 5943 , 286, 109), // #1025
  INST(Vmovddup         , VexRm_Lx           , V(F20F00,12,_,x,I,1,3,DUP), 0                         , 5949 , 287, 108), // #1026
  INST(Vmovdqa          , VexRmMr_Lx         , V(660F00,6F,_,x,I,_,_,_  ), V(660F00,7F,_,x,I,_,_,_  ), 5958 , 288, 110), // #1027
  INST(Vmovdqa32        , VexRmMr_Lx         , V(660F00,6F,_,x,_,0,4,FVM), V(660F00,7F,_,x,_,0,4,FVM), 5966 , 289, 113), // #1028
  INST(Vmovdqa64        , VexRmMr_Lx         , V(660F00,6F,_,x,_,1,4,FVM), V(660F00,7F,_,x,_,1,4,FVM), 5976 , 289, 113), // #1029
  INST(Vmovdqu          , VexRmMr_Lx         , V(F30F00,6F,_,x,I,_,_,_  ), V(F30F00,7F,_,x,I,_,_,_  ), 5986 , 288, 110), // #1030
  INST(Vmovdqu16        , VexRmMr_Lx         , V(F20F00,6F,_,x,_,1,4,FVM), V(F20F00,7F,_,x,_,1,4,FVM), 5994 , 289, 115), // #1031
  INST(Vmovdqu32        , VexRmMr_Lx         , V(F30F00,6F,_,x,_,0,4,FVM), V(F30F00,7F,_,x,_,0,4,FVM), 6004 , 289, 113), // #1032
  INST(Vmovdqu64        , VexRmMr_Lx         , V(F30F00,6F,_,x,_,1,4,FVM), V(F30F00,7F,_,x,_,1,4,FVM), 6014 , 289, 113), // #1033
  INST(Vmovdqu8         , VexRmMr_Lx         , V(F20F00,6F,_,x,_,0,4,FVM), V(F20F00,7F,_,x,_,0,4,FVM), 6024 , 289, 115), // #1034
  INST(Vmovhlps         , VexRvm             , V(000F00,12,_,0,I,0,_,_  ), 0                         , 6033 , 290, 109), // #1035
  INST(Vmovhpd          , VexRvmMr           , V(660F00,16,_,0,I,1,3,T1S), V(660F00,17,_,0,I,1,3,T1S), 6042 , 291, 109), // #1036
  INST(Vmovhps          , VexRvmMr           , V(000F00,16,_,0,I,0,3,T2 ), V(000F00,17,_,0,I,0,3,T2 ), 6050 , 291, 109), // #1037
  INST(Vmovlhps         , VexRvm             , V(000F00,16,_,0,I,0,_,_  ), 0                         , 6058 , 290, 109), // #1038
  INST(Vmovlpd          , VexRvmMr           , V(660F00,12,_,0,I,1,3,T1S), V(660F00,13,_,0,I,1,3,T1S), 6067 , 291, 109), // #1039
  INST(Vmovlps          , VexRvmMr           , V(000F00,12,_,0,I,0,3,T2 ), V(000F00,13,_,0,I,0,3,T2 ), 6075 , 291, 109), // #1040
  INST(Vmovmskpd        , VexRm_Lx           , V(660F00,50,_,x,I,_,_,_  ), 0                         , 6083 , 292, 110), // #1041
  INST(Vmovmskps        , VexRm_Lx           , V(000F00,50,_,x,I,_,_,_  ), 0                         , 6093 , 292, 110), // #1042
  INST(Vmovntdq         , VexMr_Lx           , V(660F00,E7,_,x,I,0,4,FVM), 0                         , 6103 , 293, 108), // #1043
  INST(Vmovntdqa        , VexRm_Lx           , V(660F38,2A,_,x,I,0,4,FVM), 0                         , 6112 , 294, 118), // #1044
  INST(Vmovntpd         , VexMr_Lx           , V(660F00,2B,_,x,I,1,4,FVM), 0                         , 6122 , 293, 108), // #1045
  INST(Vmovntps         , VexMr_Lx           , V(000F00,2B,_,x,I,0,4,FVM), 0                         , 6131 , 293, 108), // #1046
  INST(Vmovq            , VexMovdMovq        , V(660F00,6E,_,0,I,1,3,T1S), V(660F00,7E,_,0,I,1,3,T1S), 6140 , 295, 109), // #1047
  INST(Vmovsd           , VexMovssMovsd      , V(F20F00,10,_,I,I,1,3,T1S), V(F20F00,11,_,I,I,1,3,T1S), 6146 , 296, 109), // #1048
  INST(Vmovshdup        , VexRm_Lx           , V(F30F00,16,_,x,I,0,4,FVM), 0                         , 6153 , 297, 108), // #1049
  INST(Vmovsldup        , VexRm_Lx           , V(F30F00,12,_,x,I,0,4,FVM), 0                         , 6163 , 297, 108), // #1050
  INST(Vmovss           , VexMovssMovsd      , V(F30F00,10,_,I,I,0,2,T1S), V(F30F00,11,_,I,I,0,2,T1S), 6173 , 298, 109), // #1051
  INST(Vmovupd          , VexRmMr_Lx         , V(660F00,10,_,x,I,1,4,FVM), V(660F00,11,_,x,I,1,4,FVM), 6180 , 285, 108), // #1052
  INST(Vmovups          , VexRmMr_Lx         , V(000F00,10,_,x,I,0,4,FVM), V(000F00,11,_,x,I,0,4,FVM), 6188 , 285, 108), // #1053
  INST(Vmpsadbw         , VexRvmi_Lx         , V(660F3A,42,_,x,I,_,_,_  ), 0                         , 6196 , 191, 130), // #1054
  INST(Vmptrld          , X86M_Only          , O(000F00,C7,6,_,_,_,_,_  ), 0                         , 6205 , 283, 57 ), // #1055
  INST(Vmptrst          , X86M_Only          , O(000F00,C7,7,_,_,_,_,_  ), 0                         , 6213 , 283, 57 ), // #1056
  INST(Vmread           , X86Mr_NoSize       , O(000F00,78,_,_,_,_,_,_  ), 0                         , 6221 , 299, 57 ), // #1057
  INST(Vmresume         , X86Op              , O(000F01,C3,_,_,_,_,_,_  ), 0                         , 6228 , 30 , 57 ), // #1058
  INST(Vmrun            , X86Op_xAX          , O(000F01,D8,_,_,_,_,_,_  ), 0                         , 6237 , 284, 23 ), // #1059
  INST(Vmsave           , X86Op_xAX          , O(000F01,DB,_,_,_,_,_,_  ), 0                         , 6243 , 284, 23 ), // #1060
  INST(Vmulpd           , VexRvm_Lx          , V(660F00,59,_,x,I,1,4,FV ), 0                         , 6250 , 174, 108), // #1061
  INST(Vmulps           , VexRvm_Lx          , V(000F00,59,_,x,I,0,4,FV ), 0                         , 6257 , 175, 108), // #1062
  INST(Vmulsd           , VexRvm_Lx          , V(F20F00,59,_,I,I,1,3,T1S), 0                         , 6264 , 176, 109), // #1063
  INST(Vmulss           , VexRvm_Lx          , V(F30F00,59,_,I,I,0,2,T1S), 0                         , 6271 , 177, 109), // #1064
  INST(Vmwrite          , X86Rm_NoSize       , O(000F00,79,_,_,_,_,_,_  ), 0                         , 6278 , 300, 57 ), // #1065
  INST(Vmxon            , X86M_Only          , O(F30F00,C7,6,_,_,_,_,_  ), 0                         , 6286 , 283, 57 ), // #1066
  INST(Vorpd            , VexRvm_Lx          , V(660F00,56,_,x,I,1,4,FV ), 0                         , 6292 , 186, 114), // #1067
  INST(Vorps            , VexRvm_Lx          , V(000F00,56,_,x,I,0,4,FV ), 0                         , 6298 , 187, 114), // #1068
  INST(Vp4dpwssd        , VexRm_T1_4X        , V(F20F38,52,_,2,_,0,2,T4X), 0                         , 6304 , 172, 131), // #1069
  INST(Vp4dpwssds       , VexRm_T1_4X        , V(F20F38,53,_,2,_,0,2,T4X), 0                         , 6314 , 172, 131), // #1070
  INST(Vpabsb           , VexRm_Lx           , V(660F38,1C,_,x,I,_,4,FVM), 0                         , 6325 , 297, 132), // #1071
  INST(Vpabsd           , VexRm_Lx           , V(660F38,1E,_,x,I,0,4,FV ), 0                         , 6332 , 297, 118), // #1072
  INST(Vpabsq           , VexRm_Lx           , V(660F38,1F,_,x,_,1,4,FV ), 0                         , 6339 , 240, 113), // #1073
  INST(Vpabsw           , VexRm_Lx           , V(660F38,1D,_,x,I,_,4,FVM), 0                         , 6346 , 297, 132), // #1074
  INST(Vpackssdw        , VexRvm_Lx          , V(660F00,6B,_,x,I,0,4,FV ), 0                         , 6353 , 185, 132), // #1075
  INST(Vpacksswb        , VexRvm_Lx          , V(660F00,63,_,x,I,I,4,FVM), 0                         , 6363 , 271, 132), // #1076
  INST(Vpackusdw        , VexRvm_Lx          , V(660F38,2B,_,x,I,0,4,FV ), 0                         , 6373 , 185, 132), // #1077
  INST(Vpackuswb        , VexRvm_Lx          , V(660F00,67,_,x,I,I,4,FVM), 0                         , 6383 , 271, 132), // #1078
  INST(Vpaddb           , VexRvm_Lx          , V(660F00,FC,_,x,I,I,4,FVM), 0                         , 6393 , 271, 132), // #1079
  INST(Vpaddd           , VexRvm_Lx          , V(660F00,FE,_,x,I,0,4,FV ), 0                         , 6400 , 185, 118), // #1080
  INST(Vpaddq           , VexRvm_Lx          , V(660F00,D4,_,x,I,1,4,FV ), 0                         , 6407 , 184, 118), // #1081
  INST(Vpaddsb          , VexRvm_Lx          , V(660F00,EC,_,x,I,I,4,FVM), 0                         , 6414 , 271, 132), // #1082
  INST(Vpaddsw          , VexRvm_Lx          , V(660F00,ED,_,x,I,I,4,FVM), 0                         , 6422 , 271, 132), // #1083
  INST(Vpaddusb         , VexRvm_Lx          , V(660F00,DC,_,x,I,I,4,FVM), 0                         , 6430 , 271, 132), // #1084
  INST(Vpaddusw         , VexRvm_Lx          , V(660F00,DD,_,x,I,I,4,FVM), 0                         , 6439 , 271, 132), // #1085
  INST(Vpaddw           , VexRvm_Lx          , V(660F00,FD,_,x,I,I,4,FVM), 0                         , 6448 , 271, 132), // #1086
  INST(Vpalignr         , VexRvmi_Lx         , V(660F3A,0F,_,x,I,I,4,FVM), 0                         , 6455 , 270, 132), // #1087
  INST(Vpand            , VexRvm_Lx          , V(660F00,DB,_,x,I,_,_,_  ), 0                         , 6464 , 301, 130), // #1088
  INST(Vpandd           , VexRvm_Lx          , V(660F00,DB,_,x,_,0,4,FV ), 0                         , 6470 , 302, 113), // #1089
  INST(Vpandn           , VexRvm_Lx          , V(660F00,DF,_,x,I,_,_,_  ), 0                         , 6477 , 303, 130), // #1090
  INST(Vpandnd          , VexRvm_Lx          , V(660F00,DF,_,x,_,0,4,FV ), 0                         , 6484 , 304, 113), // #1091
  INST(Vpandnq          , VexRvm_Lx          , V(660F00,DF,_,x,_,1,4,FV ), 0                         , 6492 , 305, 113), // #1092
  INST(Vpandq           , VexRvm_Lx          , V(660F00,DB,_,x,_,1,4,FV ), 0                         , 6500 , 306, 113), // #1093
  INST(Vpavgb           , VexRvm_Lx          , V(660F00,E0,_,x,I,I,4,FVM), 0                         , 6507 , 271, 132), // #1094
  INST(Vpavgw           , VexRvm_Lx          , V(660F00,E3,_,x,I,I,4,FVM), 0                         , 6514 , 271, 132), // #1095
  INST(Vpblendd         , VexRvmi_Lx         , V(660F3A,02,_,x,0,_,_,_  ), 0                         , 6521 , 191, 117), // #1096
  INST(Vpblendvb        , VexRvmr            , V(660F3A,4C,_,x,0,_,_,_  ), 0                         , 6530 , 192, 130), // #1097
  INST(Vpblendw         , VexRvmi_Lx         , V(660F3A,0E,_,x,I,_,_,_  ), 0                         , 6540 , 191, 130), // #1098
  INST(Vpbroadcastb     , VexRm_Lx           , V(660F38,78,_,x,0,0,0,T1S), 0                         , 6549 , 307, 133), // #1099
  INST(Vpbroadcastd     , VexRm_Lx           , V(660F38,58,_,x,0,0,2,T1S), 0                         , 6562 , 308, 127), // #1100
  INST(Vpbroadcastmb2d  , VexRm_Lx           , V(F30F38,3A,_,x,_,0,_,_  ), 0                         , 6575 , 309, 134), // #1101
  INST(Vpbroadcastmb2q  , VexRm_Lx           , V(F30F38,2A,_,x,_,1,_,_  ), 0                         , 6591 , 309, 134), // #1102
  INST(Vpbroadcastq     , VexRm_Lx           , V(660F38,59,_,x,0,1,3,T1S), 0                         , 6607 , 310, 127), // #1103
  INST(Vpbroadcastw     , VexRm_Lx           , V(660F38,79,_,x,0,0,1,T1S), 0                         , 6620 , 311, 133), // #1104
  INST(Vpclmulqdq       , VexRvmi_Lx         , V(660F3A,44,_,x,I,_,4,FVM), 0                         , 6633 , 312, 135), // #1105
  INST(Vpcmov           , VexRvrmRvmr_Lx     , V(XOP_M8,A2,_,x,x,_,_,_  ), 0                         , 6644 , 249, 126), // #1106
  INST(Vpcmpb           , VexRvmi_Lx         , V(660F3A,3F,_,x,_,0,4,FVM), 0                         , 6651 , 313, 115), // #1107
  INST(Vpcmpd           , VexRvmi_Lx         , V(660F3A,1F,_,x,_,0,4,FV ), 0                         , 6658 , 314, 113), // #1108
  INST(Vpcmpeqb         , VexRvm_Lx          , V(660F00,74,_,x,I,I,4,FV ), 0                         , 6665 , 315, 132), // #1109
  INST(Vpcmpeqd         , VexRvm_Lx          , V(660F00,76,_,x,I,0,4,FVM), 0                         , 6674 , 316, 118), // #1110
  INST(Vpcmpeqq         , VexRvm_Lx          , V(660F38,29,_,x,I,1,4,FVM), 0                         , 6683 , 317, 118), // #1111
  INST(Vpcmpeqw         , VexRvm_Lx          , V(660F00,75,_,x,I,I,4,FV ), 0                         , 6692 , 315, 132), // #1112
  INST(Vpcmpestri       , VexRmi             , V(660F3A,61,_,0,I,_,_,_  ), 0                         , 6701 , 318, 136), // #1113
  INST(Vpcmpestrm       , VexRmi             , V(660F3A,60,_,0,I,_,_,_  ), 0                         , 6712 , 319, 136), // #1114
  INST(Vpcmpgtb         , VexRvm_Lx          , V(660F00,64,_,x,I,I,4,FV ), 0                         , 6723 , 315, 132), // #1115
  INST(Vpcmpgtd         , VexRvm_Lx          , V(660F00,66,_,x,I,0,4,FVM), 0                         , 6732 , 316, 118), // #1116
  INST(Vpcmpgtq         , VexRvm_Lx          , V(660F38,37,_,x,I,1,4,FVM), 0                         , 6741 , 317, 118), // #1117
  INST(Vpcmpgtw         , VexRvm_Lx          , V(660F00,65,_,x,I,I,4,FV ), 0                         , 6750 , 315, 132), // #1118
  INST(Vpcmpistri       , VexRmi             , V(660F3A,63,_,0,I,_,_,_  ), 0                         , 6759 , 320, 136), // #1119
  INST(Vpcmpistrm       , VexRmi             , V(660F3A,62,_,0,I,_,_,_  ), 0                         , 6770 , 321, 136), // #1120
  INST(Vpcmpq           , VexRvmi_Lx         , V(660F3A,1F,_,x,_,1,4,FV ), 0                         , 6781 , 322, 113), // #1121
  INST(Vpcmpub          , VexRvmi_Lx         , V(660F3A,3E,_,x,_,0,4,FVM), 0                         , 6788 , 313, 115), // #1122
  INST(Vpcmpud          , VexRvmi_Lx         , V(660F3A,1E,_,x,_,0,4,FV ), 0                         , 6796 , 314, 113), // #1123
  INST(Vpcmpuq          , VexRvmi_Lx         , V(660F3A,1E,_,x,_,1,4,FV ), 0                         , 6804 , 322, 113), // #1124
  INST(Vpcmpuw          , VexRvmi_Lx         , V(660F3A,3E,_,x,_,1,4,FVM), 0                         , 6812 , 322, 115), // #1125
  INST(Vpcmpw           , VexRvmi_Lx         , V(660F3A,3F,_,x,_,1,4,FVM), 0                         , 6820 , 322, 115), // #1126
  INST(Vpcomb           , VexRvmi            , V(XOP_M8,CC,_,0,0,_,_,_  ), 0                         , 6827 , 237, 126), // #1127
  INST(Vpcomd           , VexRvmi            , V(XOP_M8,CE,_,0,0,_,_,_  ), 0                         , 6834 , 237, 126), // #1128
  INST(Vpcompressb      , VexMr_Lx           , V(660F38,63,_,x,_,0,0,T1S), 0                         , 6841 , 206, 137), // #1129
  INST(Vpcompressd      , VexMr_Lx           , V(660F38,8B,_,x,_,0,2,T1S), 0                         , 6853 , 206, 113), // #1130
  INST(Vpcompressq      , VexMr_Lx           , V(660F38,8B,_,x,_,1,3,T1S), 0                         , 6865 , 206, 113), // #1131
  INST(Vpcompressw      , VexMr_Lx           , V(660F38,63,_,x,_,1,1,T1S), 0                         , 6877 , 206, 137), // #1132
  INST(Vpcomq           , VexRvmi            , V(XOP_M8,CF,_,0,0,_,_,_  ), 0                         , 6889 , 237, 126), // #1133
  INST(Vpcomub          , VexRvmi            , V(XOP_M8,EC,_,0,0,_,_,_  ), 0                         , 6896 , 237, 126), // #1134
  INST(Vpcomud          , VexRvmi            , V(XOP_M8,EE,_,0,0,_,_,_  ), 0                         , 6904 , 237, 126), // #1135
  INST(Vpcomuq          , VexRvmi            , V(XOP_M8,EF,_,0,0,_,_,_  ), 0                         , 6912 , 237, 126), // #1136
  INST(Vpcomuw          , VexRvmi            , V(XOP_M8,ED,_,0,0,_,_,_  ), 0                         , 6920 , 237, 126), // #1137
  INST(Vpcomw           , VexRvmi            , V(XOP_M8,CD,_,0,0,_,_,_  ), 0                         , 6928 , 237, 126), // #1138
  INST(Vpconflictd      , VexRm_Lx           , V(660F38,C4,_,x,_,0,4,FV ), 0                         , 6935 , 323, 134), // #1139
  INST(Vpconflictq      , VexRm_Lx           , V(660F38,C4,_,x,_,1,4,FV ), 0                         , 6947 , 323, 134), // #1140
  INST(Vpdpbusd         , VexRvm_Lx          , V(660F38,50,_,x,_,0,_,FV ), 0                         , 6959 , 189, 138), // #1141
  INST(Vpdpbusds        , VexRvm_Lx          , V(660F38,51,_,x,_,0,_,FV ), 0                         , 6968 , 189, 138), // #1142
  INST(Vpdpwssd         , VexRvm_Lx          , V(660F38,52,_,x,_,0,_,FV ), 0                         , 6978 , 189, 138), // #1143
  INST(Vpdpwssds        , VexRvm_Lx          , V(660F38,53,_,x,_,0,_,FV ), 0                         , 6987 , 189, 138), // #1144
  INST(Vperm2f128       , VexRvmi            , V(660F3A,06,_,1,0,_,_,_  ), 0                         , 6997 , 324, 110), // #1145
  INST(Vperm2i128       , VexRvmi            , V(660F3A,46,_,1,0,_,_,_  ), 0                         , 7008 , 324, 117), // #1146
  INST(Vpermb           , VexRvm_Lx          , V(660F38,8D,_,x,_,0,4,FVM), 0                         , 7019 , 188, 139), // #1147
  INST(Vpermd           , VexRvm_Lx          , V(660F38,36,_,x,0,0,4,FV ), 0                         , 7026 , 325, 127), // #1148
  INST(Vpermi2b         , VexRvm_Lx          , V(660F38,75,_,x,_,0,4,FVM), 0                         , 7033 , 188, 139), // #1149
  INST(Vpermi2d         , VexRvm_Lx          , V(660F38,76,_,x,_,0,4,FV ), 0                         , 7042 , 189, 113), // #1150
  INST(Vpermi2pd        , VexRvm_Lx          , V(660F38,77,_,x,_,1,4,FV ), 0                         , 7051 , 190, 113), // #1151
  INST(Vpermi2ps        , VexRvm_Lx          , V(660F38,77,_,x,_,0,4,FV ), 0                         , 7061 , 189, 113), // #1152
  INST(Vpermi2q         , VexRvm_Lx          , V(660F38,76,_,x,_,1,4,FV ), 0                         , 7071 , 190, 113), // #1153
  INST(Vpermi2w         , VexRvm_Lx          , V(660F38,75,_,x,_,1,4,FVM), 0                         , 7080 , 188, 115), // #1154
  INST(Vpermil2pd       , VexRvrmiRvmri_Lx   , V(660F3A,49,_,x,x,_,_,_  ), 0                         , 7089 , 326, 126), // #1155
  INST(Vpermil2ps       , VexRvrmiRvmri_Lx   , V(660F3A,48,_,x,x,_,_,_  ), 0                         , 7100 , 326, 126), // #1156
  INST(Vpermilpd        , VexRvmRmi_Lx       , V(660F38,0D,_,x,0,1,4,FV ), V(660F3A,05,_,x,0,1,4,FV ), 7111 , 327, 108), // #1157
  INST(Vpermilps        , VexRvmRmi_Lx       , V(660F38,0C,_,x,0,0,4,FV ), V(660F3A,04,_,x,0,0,4,FV ), 7121 , 327, 108), // #1158
  INST(Vpermpd          , VexRmi             , V(660F3A,01,_,1,1,_,_,_  ), 0                         , 7131 , 328, 117), // #1159
  INST(Vpermps          , VexRvm             , V(660F38,16,_,1,0,_,_,_  ), 0                         , 7139 , 329, 117), // #1160
  INST(Vpermq           , VexRvmRmi_Lx       , V(660F38,36,_,x,_,1,4,FV ), V(660F3A,00,_,x,1,1,4,FV ), 7147 , 330, 127), // #1161
  INST(Vpermt2b         , VexRvm_Lx          , V(660F38,7D,_,x,_,0,4,FVM), 0                         , 7154 , 188, 139), // #1162
  INST(Vpermt2d         , VexRvm_Lx          , V(660F38,7E,_,x,_,0,4,FV ), 0                         , 7163 , 189, 113), // #1163
  INST(Vpermt2pd        , VexRvm_Lx          , V(660F38,7F,_,x,_,1,4,FV ), 0                         , 7172 , 190, 113), // #1164
  INST(Vpermt2ps        , VexRvm_Lx          , V(660F38,7F,_,x,_,0,4,FV ), 0                         , 7182 , 189, 113), // #1165
  INST(Vpermt2q         , VexRvm_Lx          , V(660F38,7E,_,x,_,1,4,FV ), 0                         , 7192 , 190, 113), // #1166
  INST(Vpermt2w         , VexRvm_Lx          , V(660F38,7D,_,x,_,1,4,FVM), 0                         , 7201 , 188, 115), // #1167
  INST(Vpermw           , VexRvm_Lx          , V(660F38,8D,_,x,_,1,4,FVM), 0                         , 7210 , 188, 115), // #1168
  INST(Vpexpandb        , VexRm_Lx           , V(660F38,62,_,x,_,0,0,T1S), 0                         , 7217 , 240, 137), // #1169
  INST(Vpexpandd        , VexRm_Lx           , V(660F38,89,_,x,_,0,2,T1S), 0                         , 7227 , 240, 113), // #1170
  INST(Vpexpandq        , VexRm_Lx           , V(660F38,89,_,x,_,1,3,T1S), 0                         , 7237 , 240, 113), // #1171
  INST(Vpexpandw        , VexRm_Lx           , V(660F38,62,_,x,_,1,1,T1S), 0                         , 7247 , 240, 137), // #1172
  INST(Vpextrb          , VexMri             , V(660F3A,14,_,0,0,I,0,T1S), 0                         , 7257 , 331, 140), // #1173
  INST(Vpextrd          , VexMri             , V(660F3A,16,_,0,0,0,2,T1S), 0                         , 7265 , 244, 141), // #1174
  INST(Vpextrq          , VexMri             , V(660F3A,16,_,0,1,1,3,T1S), 0                         , 7273 , 332, 141), // #1175
  INST(Vpextrw          , VexMri             , V(660F3A,15,_,0,0,I,1,T1S), 0                         , 7281 , 333, 140), // #1176
  INST(Vpgatherdd       , VexRmvRm_VM        , V(660F38,90,_,x,0,_,_,_  ), V(660F38,90,_,x,_,0,2,T1S), 7289 , 260, 127), // #1177
  INST(Vpgatherdq       , VexRmvRm_VM        , V(660F38,90,_,x,1,_,_,_  ), V(660F38,90,_,x,_,1,3,T1S), 7300 , 259, 127), // #1178
  INST(Vpgatherqd       , VexRmvRm_VM        , V(660F38,91,_,x,0,_,_,_  ), V(660F38,91,_,x,_,0,2,T1S), 7311 , 265, 127), // #1179
  INST(Vpgatherqq       , VexRmvRm_VM        , V(660F38,91,_,x,1,_,_,_  ), V(660F38,91,_,x,_,1,3,T1S), 7322 , 264, 127), // #1180
  INST(Vphaddbd         , VexRm              , V(XOP_M9,C2,_,0,0,_,_,_  ), 0                         , 7333 , 180, 126), // #1181
  INST(Vphaddbq         , VexRm              , V(XOP_M9,C3,_,0,0,_,_,_  ), 0                         , 7342 , 180, 126), // #1182
  INST(Vphaddbw         , VexRm              , V(XOP_M9,C1,_,0,0,_,_,_  ), 0                         , 7351 , 180, 126), // #1183
  INST(Vphaddd          , VexRvm_Lx          , V(660F38,02,_,x,I,_,_,_  ), 0                         , 7360 , 178, 130), // #1184
  INST(Vphadddq         , VexRm              , V(XOP_M9,CB,_,0,0,_,_,_  ), 0                         , 7368 , 180, 126), // #1185
  INST(Vphaddsw         , VexRvm_Lx          , V(660F38,03,_,x,I,_,_,_  ), 0                         , 7377 , 178, 130), // #1186
  INST(Vphaddubd        , VexRm              , V(XOP_M9,D2,_,0,0,_,_,_  ), 0                         , 7386 , 180, 126), // #1187
  INST(Vphaddubq        , VexRm              , V(XOP_M9,D3,_,0,0,_,_,_  ), 0                         , 7396 , 180, 126), // #1188
  INST(Vphaddubw        , VexRm              , V(XOP_M9,D1,_,0,0,_,_,_  ), 0                         , 7406 , 180, 126), // #1189
  INST(Vphaddudq        , VexRm              , V(XOP_M9,DB,_,0,0,_,_,_  ), 0                         , 7416 , 180, 126), // #1190
  INST(Vphadduwd        , VexRm              , V(XOP_M9,D6,_,0,0,_,_,_  ), 0                         , 7426 , 180, 126), // #1191
  INST(Vphadduwq        , VexRm              , V(XOP_M9,D7,_,0,0,_,_,_  ), 0                         , 7436 , 180, 126), // #1192
  INST(Vphaddw          , VexRvm_Lx          , V(660F38,01,_,x,I,_,_,_  ), 0                         , 7446 , 178, 130), // #1193
  INST(Vphaddwd         , VexRm              , V(XOP_M9,C6,_,0,0,_,_,_  ), 0                         , 7454 , 180, 126), // #1194
  INST(Vphaddwq         , VexRm              , V(XOP_M9,C7,_,0,0,_,_,_  ), 0                         , 7463 , 180, 126), // #1195
  INST(Vphminposuw      , VexRm              , V(660F38,41,_,0,I,_,_,_  ), 0                         , 7472 , 180, 110), // #1196
  INST(Vphsubbw         , VexRm              , V(XOP_M9,E1,_,0,0,_,_,_  ), 0                         , 7484 , 180, 126), // #1197
  INST(Vphsubd          , VexRvm_Lx          , V(660F38,06,_,x,I,_,_,_  ), 0                         , 7493 , 178, 130), // #1198
  INST(Vphsubdq         , VexRm              , V(XOP_M9,E3,_,0,0,_,_,_  ), 0                         , 7501 , 180, 126), // #1199
  INST(Vphsubsw         , VexRvm_Lx          , V(660F38,07,_,x,I,_,_,_  ), 0                         , 7510 , 178, 130), // #1200
  INST(Vphsubw          , VexRvm_Lx          , V(660F38,05,_,x,I,_,_,_  ), 0                         , 7519 , 178, 130), // #1201
  INST(Vphsubwd         , VexRm              , V(XOP_M9,E2,_,0,0,_,_,_  ), 0                         , 7527 , 180, 126), // #1202
  INST(Vpinsrb          , VexRvmi            , V(660F3A,20,_,0,0,I,0,T1S), 0                         , 7536 , 334, 140), // #1203
  INST(Vpinsrd          , VexRvmi            , V(660F3A,22,_,0,0,0,2,T1S), 0                         , 7544 , 335, 141), // #1204
  INST(Vpinsrq          , VexRvmi            , V(660F3A,22,_,0,1,1,3,T1S), 0                         , 7552 , 336, 141), // #1205
  INST(Vpinsrw          , VexRvmi            , V(660F00,C4,_,0,0,I,1,T1S), 0                         , 7560 , 337, 140), // #1206
  INST(Vplzcntd         , VexRm_Lx           , V(660F38,44,_,x,_,0,4,FV ), 0                         , 7568 , 323, 134), // #1207
  INST(Vplzcntq         , VexRm_Lx           , V(660F38,44,_,x,_,1,4,FV ), 0                         , 7577 , 338, 134), // #1208
  INST(Vpmacsdd         , VexRvmr            , V(XOP_M8,9E,_,0,0,_,_,_  ), 0                         , 7586 , 339, 126), // #1209
  INST(Vpmacsdqh        , VexRvmr            , V(XOP_M8,9F,_,0,0,_,_,_  ), 0                         , 7595 , 339, 126), // #1210
  INST(Vpmacsdql        , VexRvmr            , V(XOP_M8,97,_,0,0,_,_,_  ), 0                         , 7605 , 339, 126), // #1211
  INST(Vpmacssdd        , VexRvmr            , V(XOP_M8,8E,_,0,0,_,_,_  ), 0                         , 7615 , 339, 126), // #1212
  INST(Vpmacssdqh       , VexRvmr            , V(XOP_M8,8F,_,0,0,_,_,_  ), 0                         , 7625 , 339, 126), // #1213
  INST(Vpmacssdql       , VexRvmr            , V(XOP_M8,87,_,0,0,_,_,_  ), 0                         , 7636 , 339, 126), // #1214
  INST(Vpmacsswd        , VexRvmr            , V(XOP_M8,86,_,0,0,_,_,_  ), 0                         , 7647 , 339, 126), // #1215
  INST(Vpmacssww        , VexRvmr            , V(XOP_M8,85,_,0,0,_,_,_  ), 0                         , 7657 , 339, 126), // #1216
  INST(Vpmacswd         , VexRvmr            , V(XOP_M8,96,_,0,0,_,_,_  ), 0                         , 7667 , 339, 126), // #1217
  INST(Vpmacsww         , VexRvmr            , V(XOP_M8,95,_,0,0,_,_,_  ), 0                         , 7676 , 339, 126), // #1218
  INST(Vpmadcsswd       , VexRvmr            , V(XOP_M8,A6,_,0,0,_,_,_  ), 0                         , 7685 , 339, 126), // #1219
  INST(Vpmadcswd        , VexRvmr            , V(XOP_M8,B6,_,0,0,_,_,_  ), 0                         , 7696 , 339, 126), // #1220
  INST(Vpmadd52huq      , VexRvm_Lx          , V(660F38,B5,_,x,_,1,4,FV ), 0                         , 7706 , 190, 142), // #1221
  INST(Vpmadd52luq      , VexRvm_Lx          , V(660F38,B4,_,x,_,1,4,FV ), 0                         , 7718 , 190, 142), // #1222
  INST(Vpmaddubsw       , VexRvm_Lx          , V(660F38,04,_,x,I,I,4,FVM), 0                         , 7730 , 271, 132), // #1223
  INST(Vpmaddwd         , VexRvm_Lx          , V(660F00,F5,_,x,I,I,4,FVM), 0                         , 7741 , 271, 132), // #1224
  INST(Vpmaskmovd       , VexRvmMvr_Lx       , V(660F38,8C,_,x,0,_,_,_  ), V(660F38,8E,_,x,0,_,_,_  ), 7750 , 279, 117), // #1225
  INST(Vpmaskmovq       , VexRvmMvr_Lx       , V(660F38,8C,_,x,1,_,_,_  ), V(660F38,8E,_,x,1,_,_,_  ), 7761 , 279, 117), // #1226
  INST(Vpmaxsb          , VexRvm_Lx          , V(660F38,3C,_,x,I,I,4,FVM), 0                         , 7772 , 340, 132), // #1227
  INST(Vpmaxsd          , VexRvm_Lx          , V(660F38,3D,_,x,I,0,4,FV ), 0                         , 7780 , 187, 118), // #1228
  INST(Vpmaxsq          , VexRvm_Lx          , V(660F38,3D,_,x,_,1,4,FV ), 0                         , 7788 , 190, 113), // #1229
  INST(Vpmaxsw          , VexRvm_Lx          , V(660F00,EE,_,x,I,I,4,FVM), 0                         , 7796 , 340, 132), // #1230
  INST(Vpmaxub          , VexRvm_Lx          , V(660F00,DE,_,x,I,I,4,FVM), 0                         , 7804 , 340, 132), // #1231
  INST(Vpmaxud          , VexRvm_Lx          , V(660F38,3F,_,x,I,0,4,FV ), 0                         , 7812 , 187, 118), // #1232
  INST(Vpmaxuq          , VexRvm_Lx          , V(660F38,3F,_,x,_,1,4,FV ), 0                         , 7820 , 190, 113), // #1233
  INST(Vpmaxuw          , VexRvm_Lx          , V(660F38,3E,_,x,I,I,4,FVM), 0                         , 7828 , 340, 132), // #1234
  INST(Vpminsb          , VexRvm_Lx          , V(660F38,38,_,x,I,I,4,FVM), 0                         , 7836 , 340, 132), // #1235
  INST(Vpminsd          , VexRvm_Lx          , V(660F38,39,_,x,I,0,4,FV ), 0                         , 7844 , 187, 118), // #1236
  INST(Vpminsq          , VexRvm_Lx          , V(660F38,39,_,x,_,1,4,FV ), 0                         , 7852 , 190, 113), // #1237
  INST(Vpminsw          , VexRvm_Lx          , V(660F00,EA,_,x,I,I,4,FVM), 0                         , 7860 , 340, 132), // #1238
  INST(Vpminub          , VexRvm_Lx          , V(660F00,DA,_,x,I,_,4,FVM), 0                         , 7868 , 340, 132), // #1239
  INST(Vpminud          , VexRvm_Lx          , V(660F38,3B,_,x,I,0,4,FV ), 0                         , 7876 , 187, 118), // #1240
  INST(Vpminuq          , VexRvm_Lx          , V(660F38,3B,_,x,_,1,4,FV ), 0                         , 7884 , 190, 113), // #1241
  INST(Vpminuw          , VexRvm_Lx          , V(660F38,3A,_,x,I,_,4,FVM), 0                         , 7892 , 340, 132), // #1242
  INST(Vpmovb2m         , VexRm_Lx           , V(F30F38,29,_,x,_,0,_,_  ), 0                         , 7900 , 341, 115), // #1243
  INST(Vpmovd2m         , VexRm_Lx           , V(F30F38,39,_,x,_,0,_,_  ), 0                         , 7909 , 341, 116), // #1244
  INST(Vpmovdb          , VexMr_Lx           , V(F30F38,31,_,x,_,0,2,QVM), 0                         , 7918 , 342, 113), // #1245
  INST(Vpmovdw          , VexMr_Lx           , V(F30F38,33,_,x,_,0,3,HVM), 0                         , 7926 , 343, 113), // #1246
  INST(Vpmovm2b         , VexRm_Lx           , V(F30F38,28,_,x,_,0,_,_  ), 0                         , 7934 , 309, 115), // #1247
  INST(Vpmovm2d         , VexRm_Lx           , V(F30F38,38,_,x,_,0,_,_  ), 0                         , 7943 , 309, 116), // #1248
  INST(Vpmovm2q         , VexRm_Lx           , V(F30F38,38,_,x,_,1,_,_  ), 0                         , 7952 , 309, 116), // #1249
  INST(Vpmovm2w         , VexRm_Lx           , V(F30F38,28,_,x,_,1,_,_  ), 0                         , 7961 , 309, 115), // #1250
  INST(Vpmovmskb        , VexRm_Lx           , V(660F00,D7,_,x,I,_,_,_  ), 0                         , 7970 , 292, 130), // #1251
  INST(Vpmovq2m         , VexRm_Lx           , V(F30F38,39,_,x,_,1,_,_  ), 0                         , 7980 , 341, 116), // #1252
  INST(Vpmovqb          , VexMr_Lx           , V(F30F38,32,_,x,_,0,1,OVM), 0                         , 7989 , 344, 113), // #1253
  INST(Vpmovqd          , VexMr_Lx           , V(F30F38,35,_,x,_,0,3,HVM), 0                         , 7997 , 343, 113), // #1254
  INST(Vpmovqw          , VexMr_Lx           , V(F30F38,34,_,x,_,0,2,QVM), 0                         , 8005 , 342, 113), // #1255
  INST(Vpmovsdb         , VexMr_Lx           , V(F30F38,21,_,x,_,0,2,QVM), 0                         , 8013 , 342, 113), // #1256
  INST(Vpmovsdw         , VexMr_Lx           , V(F30F38,23,_,x,_,0,3,HVM), 0                         , 8022 , 343, 113), // #1257
  INST(Vpmovsqb         , VexMr_Lx           , V(F30F38,22,_,x,_,0,1,OVM), 0                         , 8031 , 344, 113), // #1258
  INST(Vpmovsqd         , VexMr_Lx           , V(F30F38,25,_,x,_,0,3,HVM), 0                         , 8040 , 343, 113), // #1259
  INST(Vpmovsqw         , VexMr_Lx           , V(F30F38,24,_,x,_,0,2,QVM), 0                         , 8049 , 342, 113), // #1260
  INST(Vpmovswb         , VexMr_Lx           , V(F30F38,20,_,x,_,0,3,HVM), 0                         , 8058 , 343, 115), // #1261
  INST(Vpmovsxbd        , VexRm_Lx           , V(660F38,21,_,x,I,I,2,QVM), 0                         , 8067 , 345, 118), // #1262
  INST(Vpmovsxbq        , VexRm_Lx           , V(660F38,22,_,x,I,I,1,OVM), 0                         , 8077 , 346, 118), // #1263
  INST(Vpmovsxbw        , VexRm_Lx           , V(660F38,20,_,x,I,I,3,HVM), 0                         , 8087 , 347, 132), // #1264
  INST(Vpmovsxdq        , VexRm_Lx           , V(660F38,25,_,x,I,0,3,HVM), 0                         , 8097 , 347, 118), // #1265
  INST(Vpmovsxwd        , VexRm_Lx           , V(660F38,23,_,x,I,I,3,HVM), 0                         , 8107 , 347, 118), // #1266
  INST(Vpmovsxwq        , VexRm_Lx           , V(660F38,24,_,x,I,I,2,QVM), 0                         , 8117 , 345, 118), // #1267
  INST(Vpmovusdb        , VexMr_Lx           , V(F30F38,11,_,x,_,0,2,QVM), 0                         , 8127 , 342, 113), // #1268
  INST(Vpmovusdw        , VexMr_Lx           , V(F30F38,13,_,x,_,0,3,HVM), 0                         , 8137 , 343, 113), // #1269
  INST(Vpmovusqb        , VexMr_Lx           , V(F30F38,12,_,x,_,0,1,OVM), 0                         , 8147 , 344, 113), // #1270
  INST(Vpmovusqd        , VexMr_Lx           , V(F30F38,15,_,x,_,0,3,HVM), 0                         , 8157 , 343, 113), // #1271
  INST(Vpmovusqw        , VexMr_Lx           , V(F30F38,14,_,x,_,0,2,QVM), 0                         , 8167 , 342, 113), // #1272
  INST(Vpmovuswb        , VexMr_Lx           , V(F30F38,10,_,x,_,0,3,HVM), 0                         , 8177 , 343, 115), // #1273
  INST(Vpmovw2m         , VexRm_Lx           , V(F30F38,29,_,x,_,1,_,_  ), 0                         , 8187 , 341, 115), // #1274
  INST(Vpmovwb          , VexMr_Lx           , V(F30F38,30,_,x,_,0,3,HVM), 0                         , 8196 , 343, 115), // #1275
  INST(Vpmovzxbd        , VexRm_Lx           , V(660F38,31,_,x,I,I,2,QVM), 0                         , 8204 , 345, 118), // #1276
  INST(Vpmovzxbq        , VexRm_Lx           , V(660F38,32,_,x,I,I,1,OVM), 0                         , 8214 , 346, 118), // #1277
  INST(Vpmovzxbw        , VexRm_Lx           , V(660F38,30,_,x,I,I,3,HVM), 0                         , 8224 , 347, 132), // #1278
  INST(Vpmovzxdq        , VexRm_Lx           , V(660F38,35,_,x,I,0,3,HVM), 0                         , 8234 , 347, 118), // #1279
  INST(Vpmovzxwd        , VexRm_Lx           , V(660F38,33,_,x,I,I,3,HVM), 0                         , 8244 , 347, 118), // #1280
  INST(Vpmovzxwq        , VexRm_Lx           , V(660F38,34,_,x,I,I,2,QVM), 0                         , 8254 , 345, 118), // #1281
  INST(Vpmuldq          , VexRvm_Lx          , V(660F38,28,_,x,I,1,4,FV ), 0                         , 8264 , 184, 118), // #1282
  INST(Vpmulhrsw        , VexRvm_Lx          , V(660F38,0B,_,x,I,I,4,FVM), 0                         , 8272 , 271, 132), // #1283
  INST(Vpmulhuw         , VexRvm_Lx          , V(660F00,E4,_,x,I,I,4,FVM), 0                         , 8282 , 271, 132), // #1284
  INST(Vpmulhw          , VexRvm_Lx          , V(660F00,E5,_,x,I,I,4,FVM), 0                         , 8291 , 271, 132), // #1285
  INST(Vpmulld          , VexRvm_Lx          , V(660F38,40,_,x,I,0,4,FV ), 0                         , 8299 , 185, 118), // #1286
  INST(Vpmullq          , VexRvm_Lx          , V(660F38,40,_,x,_,1,4,FV ), 0                         , 8307 , 190, 116), // #1287
  INST(Vpmullw          , VexRvm_Lx          , V(660F00,D5,_,x,I,I,4,FVM), 0                         , 8315 , 271, 132), // #1288
  INST(Vpmultishiftqb   , VexRvm_Lx          , V(660F38,83,_,x,_,1,4,FV ), 0                         , 8323 , 190, 139), // #1289
  INST(Vpmuludq         , VexRvm_Lx          , V(660F00,F4,_,x,I,1,4,FV ), 0                         , 8338 , 184, 118), // #1290
  INST(Vpopcntb         , VexRm_Lx           , V(660F38,54,_,x,_,0,4,FV ), 0                         , 8347 , 240, 143), // #1291
  INST(Vpopcntd         , VexRm_Lx           , V(660F38,55,_,x,_,0,4,FVM), 0                         , 8356 , 323, 144), // #1292
  INST(Vpopcntq         , VexRm_Lx           , V(660F38,55,_,x,_,1,4,FVM), 0                         , 8365 , 338, 144), // #1293
  INST(Vpopcntw         , VexRm_Lx           , V(660F38,54,_,x,_,1,4,FV ), 0                         , 8374 , 240, 143), // #1294
  INST(Vpor             , VexRvm_Lx          , V(660F00,EB,_,x,I,_,_,_  ), 0                         , 8383 , 301, 130), // #1295
  INST(Vpord            , VexRvm_Lx          , V(660F00,EB,_,x,_,0,4,FV ), 0                         , 8388 , 302, 113), // #1296
  INST(Vporq            , VexRvm_Lx          , V(660F00,EB,_,x,_,1,4,FV ), 0                         , 8394 , 306, 113), // #1297
  INST(Vpperm           , VexRvrmRvmr        , V(XOP_M8,A3,_,0,x,_,_,_  ), 0                         , 8400 , 348, 126), // #1298
  INST(Vprold           , VexVmi_Lx          , V(660F00,72,1,x,_,0,4,FV ), 0                         , 8407 , 349, 113), // #1299
  INST(Vprolq           , VexVmi_Lx          , V(660F00,72,1,x,_,1,4,FV ), 0                         , 8414 , 350, 113), // #1300
  INST(Vprolvd          , VexRvm_Lx          , V(660F38,15,_,x,_,0,4,FV ), 0                         , 8421 , 189, 113), // #1301
  INST(Vprolvq          , VexRvm_Lx          , V(660F38,15,_,x,_,1,4,FV ), 0                         , 8429 , 190, 113), // #1302
  INST(Vprord           , VexVmi_Lx          , V(660F00,72,0,x,_,0,4,FV ), 0                         , 8437 , 349, 113), // #1303
  INST(Vprorq           , VexVmi_Lx          , V(660F00,72,0,x,_,1,4,FV ), 0                         , 8444 , 350, 113), // #1304
  INST(Vprorvd          , VexRvm_Lx          , V(660F38,14,_,x,_,0,4,FV ), 0                         , 8451 , 189, 113), // #1305
  INST(Vprorvq          , VexRvm_Lx          , V(660F38,14,_,x,_,1,4,FV ), 0                         , 8459 , 190, 113), // #1306
  INST(Vprotb           , VexRvmRmvRmi       , V(XOP_M9,90,_,0,x,_,_,_  ), V(XOP_M8,C0,_,0,x,_,_,_  ), 8467 , 351, 126), // #1307
  INST(Vprotd           , VexRvmRmvRmi       , V(XOP_M9,92,_,0,x,_,_,_  ), V(XOP_M8,C2,_,0,x,_,_,_  ), 8474 , 351, 126), // #1308
  INST(Vprotq           , VexRvmRmvRmi       , V(XOP_M9,93,_,0,x,_,_,_  ), V(XOP_M8,C3,_,0,x,_,_,_  ), 8481 , 351, 126), // #1309
  INST(Vprotw           , VexRvmRmvRmi       , V(XOP_M9,91,_,0,x,_,_,_  ), V(XOP_M8,C1,_,0,x,_,_,_  ), 8488 , 351, 126), // #1310
  INST(Vpsadbw          , VexRvm_Lx          , V(660F00,F6,_,x,I,I,4,FVM), 0                         , 8495 , 179, 132), // #1311
  INST(Vpscatterdd      , VexMr_VM           , V(660F38,A0,_,x,_,0,2,T1S), 0                         , 8503 , 352, 113), // #1312
  INST(Vpscatterdq      , VexMr_VM           , V(660F38,A0,_,x,_,1,3,T1S), 0                         , 8515 , 352, 113), // #1313
  INST(Vpscatterqd      , VexMr_VM           , V(660F38,A1,_,x,_,0,2,T1S), 0                         , 8527 , 353, 113), // #1314
  INST(Vpscatterqq      , VexMr_VM           , V(660F38,A1,_,x,_,1,3,T1S), 0                         , 8539 , 354, 113), // #1315
  INST(Vpshab           , VexRvmRmv          , V(XOP_M9,98,_,0,x,_,_,_  ), 0                         , 8551 , 355, 126), // #1316
  INST(Vpshad           , VexRvmRmv          , V(XOP_M9,9A,_,0,x,_,_,_  ), 0                         , 8558 , 355, 126), // #1317
  INST(Vpshaq           , VexRvmRmv          , V(XOP_M9,9B,_,0,x,_,_,_  ), 0                         , 8565 , 355, 126), // #1318
  INST(Vpshaw           , VexRvmRmv          , V(XOP_M9,99,_,0,x,_,_,_  ), 0                         , 8572 , 355, 126), // #1319
  INST(Vpshlb           , VexRvmRmv          , V(XOP_M9,94,_,0,x,_,_,_  ), 0                         , 8579 , 355, 126), // #1320
  INST(Vpshld           , VexRvmRmv          , V(XOP_M9,96,_,0,x,_,_,_  ), 0                         , 8586 , 355, 126), // #1321
  INST(Vpshldd          , VexRvmi_Lx         , V(660F3A,71,_,x,_,0,4,FV ), 0                         , 8593 , 182, 137), // #1322
  INST(Vpshldq          , VexRvmi_Lx         , V(660F3A,71,_,x,_,1,4,FV ), 0                         , 8601 , 183, 137), // #1323
  INST(Vpshldvd         , VexRvm_Lx          , V(660F38,71,_,x,_,0,4,FV ), 0                         , 8609 , 189, 137), // #1324
  INST(Vpshldvq         , VexRvm_Lx          , V(660F38,71,_,x,_,1,4,FV ), 0                         , 8618 , 190, 137), // #1325
  INST(Vpshldvw         , VexRvm_Lx          , V(660F38,70,_,x,_,0,4,FVM), 0                         , 8627 , 188, 137), // #1326
  INST(Vpshldw          , VexRvmi_Lx         , V(660F3A,70,_,x,_,0,4,FVM), 0                         , 8636 , 236, 137), // #1327
  INST(Vpshlq           , VexRvmRmv          , V(XOP_M9,97,_,0,x,_,_,_  ), 0                         , 8644 , 355, 126), // #1328
  INST(Vpshlw           , VexRvmRmv          , V(XOP_M9,95,_,0,x,_,_,_  ), 0                         , 8651 , 355, 126), // #1329
  INST(Vpshrdd          , VexRvmi_Lx         , V(660F3A,73,_,x,_,0,4,FV ), 0                         , 8658 , 182, 137), // #1330
  INST(Vpshrdq          , VexRvmi_Lx         , V(660F3A,73,_,x,_,1,4,FV ), 0                         , 8666 , 183, 137), // #1331
  INST(Vpshrdvd         , VexRvm_Lx          , V(660F38,73,_,x,_,0,4,FV ), 0                         , 8674 , 189, 137), // #1332
  INST(Vpshrdvq         , VexRvm_Lx          , V(660F38,73,_,x,_,1,4,FV ), 0                         , 8683 , 190, 137), // #1333
  INST(Vpshrdvw         , VexRvm_Lx          , V(660F38,72,_,x,_,0,4,FVM), 0                         , 8692 , 188, 137), // #1334
  INST(Vpshrdw          , VexRvmi_Lx         , V(660F3A,72,_,x,_,0,4,FVM), 0                         , 8701 , 236, 137), // #1335
  INST(Vpshufb          , VexRvm_Lx          , V(660F38,00,_,x,I,I,4,FVM), 0                         , 8709 , 271, 132), // #1336
  INST(Vpshufbitqmb     , VexRvm_Lx          , V(660F38,8F,_,x,0,0,4,FVM), 0                         , 8717 , 356, 143), // #1337
  INST(Vpshufd          , VexRmi_Lx          , V(660F00,70,_,x,I,0,4,FV ), 0                         , 8730 , 357, 118), // #1338
  INST(Vpshufhw         , VexRmi_Lx          , V(F30F00,70,_,x,I,I,4,FVM), 0                         , 8738 , 358, 132), // #1339
  INST(Vpshuflw         , VexRmi_Lx          , V(F20F00,70,_,x,I,I,4,FVM), 0                         , 8747 , 358, 132), // #1340
  INST(Vpsignb          , VexRvm_Lx          , V(660F38,08,_,x,I,_,_,_  ), 0                         , 8756 , 178, 130), // #1341
  INST(Vpsignd          , VexRvm_Lx          , V(660F38,0A,_,x,I,_,_,_  ), 0                         , 8764 , 178, 130), // #1342
  INST(Vpsignw          , VexRvm_Lx          , V(660F38,09,_,x,I,_,_,_  ), 0                         , 8772 , 178, 130), // #1343
  INST(Vpslld           , VexRvmVmi_Lx       , V(660F00,F2,_,x,I,0,4,128), V(660F00,72,6,x,I,0,4,FV ), 8780 , 359, 118), // #1344
  INST(Vpslldq          , VexEvexVmi_Lx      , V(660F00,73,7,x,I,I,4,FVM), 0                         , 8787 , 360, 132), // #1345
  INST(Vpsllq           , VexRvmVmi_Lx       , V(660F00,F3,_,x,I,1,4,128), V(660F00,73,6,x,I,1,4,FV ), 8795 , 361, 118), // #1346
  INST(Vpsllvd          , VexRvm_Lx          , V(660F38,47,_,x,0,0,4,FV ), 0                         , 8802 , 185, 127), // #1347
  INST(Vpsllvq          , VexRvm_Lx          , V(660F38,47,_,x,1,1,4,FV ), 0                         , 8810 , 184, 127), // #1348
  INST(Vpsllvw          , VexRvm_Lx          , V(660F38,12,_,x,_,1,4,FVM), 0                         , 8818 , 188, 115), // #1349
  INST(Vpsllw           , VexRvmVmi_Lx       , V(660F00,F1,_,x,I,I,4,FVM), V(660F00,71,6,x,I,I,4,FVM), 8826 , 362, 132), // #1350
  INST(Vpsrad           , VexRvmVmi_Lx       , V(660F00,E2,_,x,I,0,4,128), V(660F00,72,4,x,I,0,4,FV ), 8833 , 359, 118), // #1351
  INST(Vpsraq           , VexRvmVmi_Lx       , V(660F00,E2,_,x,_,1,4,128), V(660F00,72,4,x,_,1,4,FV ), 8840 , 363, 113), // #1352
  INST(Vpsravd          , VexRvm_Lx          , V(660F38,46,_,x,0,0,4,FV ), 0                         , 8847 , 185, 127), // #1353
  INST(Vpsravq          , VexRvm_Lx          , V(660F38,46,_,x,_,1,4,FV ), 0                         , 8855 , 190, 113), // #1354
  INST(Vpsravw          , VexRvm_Lx          , V(660F38,11,_,x,_,1,4,FVM), 0                         , 8863 , 188, 115), // #1355
  INST(Vpsraw           , VexRvmVmi_Lx       , V(660F00,E1,_,x,I,I,4,128), V(660F00,71,4,x,I,I,4,FVM), 8871 , 362, 132), // #1356
  INST(Vpsrld           , VexRvmVmi_Lx       , V(660F00,D2,_,x,I,0,4,128), V(660F00,72,2,x,I,0,4,FV ), 8878 , 359, 118), // #1357
  INST(Vpsrldq          , VexEvexVmi_Lx      , V(660F00,73,3,x,I,I,4,FVM), 0                         , 8885 , 360, 132), // #1358
  INST(Vpsrlq           , VexRvmVmi_Lx       , V(660F00,D3,_,x,I,1,4,128), V(660F00,73,2,x,I,1,4,FV ), 8893 , 361, 118), // #1359
  INST(Vpsrlvd          , VexRvm_Lx          , V(660F38,45,_,x,0,0,4,FV ), 0                         , 8900 , 185, 127), // #1360
  INST(Vpsrlvq          , VexRvm_Lx          , V(660F38,45,_,x,1,1,4,FV ), 0                         , 8908 , 184, 127), // #1361
  INST(Vpsrlvw          , VexRvm_Lx          , V(660F38,10,_,x,_,1,4,FVM), 0                         , 8916 , 188, 115), // #1362
  INST(Vpsrlw           , VexRvmVmi_Lx       , V(660F00,D1,_,x,I,I,4,128), V(660F00,71,2,x,I,I,4,FVM), 8924 , 362, 132), // #1363
  INST(Vpsubb           , VexRvm_Lx          , V(660F00,F8,_,x,I,I,4,FVM), 0                         , 8931 , 364, 132), // #1364
  INST(Vpsubd           , VexRvm_Lx          , V(660F00,FA,_,x,I,0,4,FV ), 0                         , 8938 , 365, 118), // #1365
  INST(Vpsubq           , VexRvm_Lx          , V(660F00,FB,_,x,I,1,4,FV ), 0                         , 8945 , 366, 118), // #1366
  INST(Vpsubsb          , VexRvm_Lx          , V(660F00,E8,_,x,I,I,4,FVM), 0                         , 8952 , 364, 132), // #1367
  INST(Vpsubsw          , VexRvm_Lx          , V(660F00,E9,_,x,I,I,4,FVM), 0                         , 8960 , 364, 132), // #1368
  INST(Vpsubusb         , VexRvm_Lx          , V(660F00,D8,_,x,I,I,4,FVM), 0                         , 8968 , 364, 132), // #1369
  INST(Vpsubusw         , VexRvm_Lx          , V(660F00,D9,_,x,I,I,4,FVM), 0                         , 8977 , 364, 132), // #1370
  INST(Vpsubw           , VexRvm_Lx          , V(660F00,F9,_,x,I,I,4,FVM), 0                         , 8986 , 364, 132), // #1371
  INST(Vpternlogd       , VexRvmi_Lx         , V(660F3A,25,_,x,_,0,4,FV ), 0                         , 8993 , 182, 113), // #1372
  INST(Vpternlogq       , VexRvmi_Lx         , V(660F3A,25,_,x,_,1,4,FV ), 0                         , 9004 , 183, 113), // #1373
  INST(Vptest           , VexRm_Lx           , V(660F38,17,_,x,I,_,_,_  ), 0                         , 9015 , 256, 136), // #1374
  INST(Vptestmb         , VexRvm_Lx          , V(660F38,26,_,x,_,0,4,FVM), 0                         , 9022 , 356, 115), // #1375
  INST(Vptestmd         , VexRvm_Lx          , V(660F38,27,_,x,_,0,4,FV ), 0                         , 9031 , 367, 113), // #1376
  INST(Vptestmq         , VexRvm_Lx          , V(660F38,27,_,x,_,1,4,FV ), 0                         , 9040 , 368, 113), // #1377
  INST(Vptestmw         , VexRvm_Lx          , V(660F38,26,_,x,_,1,4,FVM), 0                         , 9049 , 356, 115), // #1378
  INST(Vptestnmb        , VexRvm_Lx          , V(F30F38,26,_,x,_,0,4,FVM), 0                         , 9058 , 356, 115), // #1379
  INST(Vptestnmd        , VexRvm_Lx          , V(F30F38,27,_,x,_,0,4,FV ), 0                         , 9068 , 367, 113), // #1380
  INST(Vptestnmq        , VexRvm_Lx          , V(F30F38,27,_,x,_,1,4,FV ), 0                         , 9078 , 368, 113), // #1381
  INST(Vptestnmw        , VexRvm_Lx          , V(F30F38,26,_,x,_,1,4,FVM), 0                         , 9088 , 356, 115), // #1382
  INST(Vpunpckhbw       , VexRvm_Lx          , V(660F00,68,_,x,I,I,4,FVM), 0                         , 9098 , 271, 132), // #1383
  INST(Vpunpckhdq       , VexRvm_Lx          , V(660F00,6A,_,x,I,0,4,FV ), 0                         , 9109 , 185, 118), // #1384
  INST(Vpunpckhqdq      , VexRvm_Lx          , V(660F00,6D,_,x,I,1,4,FV ), 0                         , 9120 , 184, 118), // #1385
  INST(Vpunpckhwd       , VexRvm_Lx          , V(660F00,69,_,x,I,I,4,FVM), 0                         , 9132 , 271, 132), // #1386
  INST(Vpunpcklbw       , VexRvm_Lx          , V(660F00,60,_,x,I,I,4,FVM), 0                         , 9143 , 271, 132), // #1387
  INST(Vpunpckldq       , VexRvm_Lx          , V(660F00,62,_,x,I,0,4,FV ), 0                         , 9154 , 185, 118), // #1388
  INST(Vpunpcklqdq      , VexRvm_Lx          , V(660F00,6C,_,x,I,1,4,FV ), 0                         , 9165 , 184, 118), // #1389
  INST(Vpunpcklwd       , VexRvm_Lx          , V(660F00,61,_,x,I,I,4,FVM), 0                         , 9177 , 271, 132), // #1390
  INST(Vpxor            , VexRvm_Lx          , V(660F00,EF,_,x,I,_,_,_  ), 0                         , 9188 , 303, 130), // #1391
  INST(Vpxord           , VexRvm_Lx          , V(660F00,EF,_,x,_,0,4,FV ), 0                         , 9194 , 304, 113), // #1392
  INST(Vpxorq           , VexRvm_Lx          , V(660F00,EF,_,x,_,1,4,FV ), 0                         , 9201 , 305, 113), // #1393
  INST(Vrangepd         , VexRvmi_Lx         , V(660F3A,50,_,x,_,1,4,FV ), 0                         , 9208 , 245, 116), // #1394
  INST(Vrangeps         , VexRvmi_Lx         , V(660F3A,50,_,x,_,0,4,FV ), 0                         , 9217 , 246, 116), // #1395
  INST(Vrangesd         , VexRvmi            , V(660F3A,51,_,I,_,1,3,T1S), 0                         , 9226 , 247, 65 ), // #1396
  INST(Vrangess         , VexRvmi            , V(660F3A,51,_,I,_,0,2,T1S), 0                         , 9235 , 248, 65 ), // #1397
  INST(Vrcp14pd         , VexRm_Lx           , V(660F38,4C,_,x,_,1,4,FV ), 0                         , 9244 , 338, 113), // #1398
  INST(Vrcp14ps         , VexRm_Lx           , V(660F38,4C,_,x,_,0,4,FV ), 0                         , 9253 , 323, 113), // #1399
  INST(Vrcp14sd         , VexRvm             , V(660F38,4D,_,I,_,1,3,T1S), 0                         , 9262 , 369, 67 ), // #1400
  INST(Vrcp14ss         , VexRvm             , V(660F38,4D,_,I,_,0,2,T1S), 0                         , 9271 , 370, 67 ), // #1401
  INST(Vrcp28pd         , VexRm              , V(660F38,CA,_,2,_,1,4,FV ), 0                         , 9280 , 238, 122), // #1402
  INST(Vrcp28ps         , VexRm              , V(660F38,CA,_,2,_,0,4,FV ), 0                         , 9289 , 239, 122), // #1403
  INST(Vrcp28sd         , VexRvm             , V(660F38,CB,_,I,_,1,3,T1S), 0                         , 9298 , 266, 122), // #1404
  INST(Vrcp28ss         , VexRvm             , V(660F38,CB,_,I,_,0,2,T1S), 0                         , 9307 , 267, 122), // #1405
  INST(Vrcpps           , VexRm_Lx           , V(000F00,53,_,x,I,_,_,_  ), 0                         , 9316 , 256, 110), // #1406
  INST(Vrcpss           , VexRvm             , V(F30F00,53,_,I,I,_,_,_  ), 0                         , 9323 , 371, 110), // #1407
  INST(Vreducepd        , VexRmi_Lx          , V(660F3A,56,_,x,_,1,4,FV ), 0                         , 9330 , 350, 116), // #1408
  INST(Vreduceps        , VexRmi_Lx          , V(660F3A,56,_,x,_,0,4,FV ), 0                         , 9340 , 349, 116), // #1409
  INST(Vreducesd        , VexRvmi            , V(660F3A,57,_,I,_,1,3,T1S), 0                         , 9350 , 372, 65 ), // #1410
  INST(Vreducess        , VexRvmi            , V(660F3A,57,_,I,_,0,2,T1S), 0                         , 9360 , 373, 65 ), // #1411
  INST(Vrndscalepd      , VexRmi_Lx          , V(660F3A,09,_,x,_,1,4,FV ), 0                         , 9370 , 268, 113), // #1412
  INST(Vrndscaleps      , VexRmi_Lx          , V(660F3A,08,_,x,_,0,4,FV ), 0                         , 9382 , 269, 113), // #1413
  INST(Vrndscalesd      , VexRvmi            , V(660F3A,0B,_,I,_,1,3,T1S), 0                         , 9394 , 247, 67 ), // #1414
  INST(Vrndscaless      , VexRvmi            , V(660F3A,0A,_,I,_,0,2,T1S), 0                         , 9406 , 248, 67 ), // #1415
  INST(Vroundpd         , VexRmi_Lx          , V(660F3A,09,_,x,I,_,_,_  ), 0                         , 9418 , 374, 110), // #1416
  INST(Vroundps         , VexRmi_Lx          , V(660F3A,08,_,x,I,_,_,_  ), 0                         , 9427 , 374, 110), // #1417
  INST(Vroundsd         , VexRvmi            , V(660F3A,0B,_,I,I,_,_,_  ), 0                         , 9436 , 375, 110), // #1418
  INST(Vroundss         , VexRvmi            , V(660F3A,0A,_,I,I,_,_,_  ), 0                         , 9445 , 376, 110), // #1419
  INST(Vrsqrt14pd       , VexRm_Lx           , V(660F38,4E,_,x,_,1,4,FV ), 0                         , 9454 , 338, 113), // #1420
  INST(Vrsqrt14ps       , VexRm_Lx           , V(660F38,4E,_,x,_,0,4,FV ), 0                         , 9465 , 323, 113), // #1421
  INST(Vrsqrt14sd       , VexRvm             , V(660F38,4F,_,I,_,1,3,T1S), 0                         , 9476 , 369, 67 ), // #1422
  INST(Vrsqrt14ss       , VexRvm             , V(660F38,4F,_,I,_,0,2,T1S), 0                         , 9487 , 370, 67 ), // #1423
  INST(Vrsqrt28pd       , VexRm              , V(660F38,CC,_,2,_,1,4,FV ), 0                         , 9498 , 238, 122), // #1424
  INST(Vrsqrt28ps       , VexRm              , V(660F38,CC,_,2,_,0,4,FV ), 0                         , 9509 , 239, 122), // #1425
  INST(Vrsqrt28sd       , VexRvm             , V(660F38,CD,_,I,_,1,3,T1S), 0                         , 9520 , 266, 122), // #1426
  INST(Vrsqrt28ss       , VexRvm             , V(660F38,CD,_,I,_,0,2,T1S), 0                         , 9531 , 267, 122), // #1427
  INST(Vrsqrtps         , VexRm_Lx           , V(000F00,52,_,x,I,_,_,_  ), 0                         , 9542 , 256, 110), // #1428
  INST(Vrsqrtss         , VexRvm             , V(F30F00,52,_,I,I,_,_,_  ), 0                         , 9551 , 371, 110), // #1429
  INST(Vscalefpd        , VexRvm_Lx          , V(660F38,2C,_,x,_,1,4,FV ), 0                         , 9560 , 377, 113), // #1430
  INST(Vscalefps        , VexRvm_Lx          , V(660F38,2C,_,x,_,0,4,FV ), 0                         , 9570 , 378, 113), // #1431
  INST(Vscalefsd        , VexRvm             , V(660F38,2D,_,I,_,1,3,T1S), 0                         , 9580 , 379, 67 ), // #1432
  INST(Vscalefss        , VexRvm             , V(660F38,2D,_,I,_,0,2,T1S), 0                         , 9590 , 380, 67 ), // #1433
  INST(Vscatterdpd      , VexMr_Lx           , V(660F38,A2,_,x,_,1,3,T1S), 0                         , 9600 , 381, 113), // #1434
  INST(Vscatterdps      , VexMr_Lx           , V(660F38,A2,_,x,_,0,2,T1S), 0                         , 9612 , 352, 113), // #1435
  INST(Vscatterpf0dpd   , VexM_VM            , V(660F38,C6,5,2,_,1,3,T1S), 0                         , 9624 , 261, 128), // #1436
  INST(Vscatterpf0dps   , VexM_VM            , V(660F38,C6,5,2,_,0,2,T1S), 0                         , 9639 , 262, 128), // #1437
  INST(Vscatterpf0qpd   , VexM_VM            , V(660F38,C7,5,2,_,1,3,T1S), 0                         , 9654 , 263, 128), // #1438
  INST(Vscatterpf0qps   , VexM_VM            , V(660F38,C7,5,2,_,0,2,T1S), 0                         , 9669 , 263, 128), // #1439
  INST(Vscatterpf1dpd   , VexM_VM            , V(660F38,C6,6,2,_,1,3,T1S), 0                         , 9684 , 261, 128), // #1440
  INST(Vscatterpf1dps   , VexM_VM            , V(660F38,C6,6,2,_,0,2,T1S), 0                         , 9699 , 262, 128), // #1441
  INST(Vscatterpf1qpd   , VexM_VM            , V(660F38,C7,6,2,_,1,3,T1S), 0                         , 9714 , 263, 128), // #1442
  INST(Vscatterpf1qps   , VexM_VM            , V(660F38,C7,6,2,_,0,2,T1S), 0                         , 9729 , 263, 128), // #1443
  INST(Vscatterqpd      , VexMr_Lx           , V(660F38,A3,_,x,_,1,3,T1S), 0                         , 9744 , 354, 113), // #1444
  INST(Vscatterqps      , VexMr_Lx           , V(660F38,A3,_,x,_,0,2,T1S), 0                         , 9756 , 353, 113), // #1445
  INST(Vshuff32x4       , VexRvmi_Lx         , V(660F3A,23,_,x,_,0,4,FV ), 0                         , 9768 , 382, 113), // #1446
  INST(Vshuff64x2       , VexRvmi_Lx         , V(660F3A,23,_,x,_,1,4,FV ), 0                         , 9779 , 383, 113), // #1447
  INST(Vshufi32x4       , VexRvmi_Lx         , V(660F3A,43,_,x,_,0,4,FV ), 0                         , 9790 , 382, 113), // #1448
  INST(Vshufi64x2       , VexRvmi_Lx         , V(660F3A,43,_,x,_,1,4,FV ), 0                         , 9801 , 383, 113), // #1449
  INST(Vshufpd          , VexRvmi_Lx         , V(660F00,C6,_,x,I,1,4,FV ), 0                         , 9812 , 384, 108), // #1450
  INST(Vshufps          , VexRvmi_Lx         , V(000F00,C6,_,x,I,0,4,FV ), 0                         , 9820 , 385, 108), // #1451
  INST(Vsqrtpd          , VexRm_Lx           , V(660F00,51,_,x,I,1,4,FV ), 0                         , 9828 , 386, 108), // #1452
  INST(Vsqrtps          , VexRm_Lx           , V(000F00,51,_,x,I,0,4,FV ), 0                         , 9836 , 208, 108), // #1453
  INST(Vsqrtsd          , VexRvm             , V(F20F00,51,_,I,I,1,3,T1S), 0                         , 9844 , 176, 109), // #1454
  INST(Vsqrtss          , VexRvm             , V(F30F00,51,_,I,I,0,2,T1S), 0                         , 9852 , 177, 109), // #1455
  INST(Vstmxcsr         , VexM               , V(000F00,AE,3,0,I,_,_,_  ), 0                         , 9860 , 277, 110), // #1456
  INST(Vsubpd           , VexRvm_Lx          , V(660F00,5C,_,x,I,1,4,FV ), 0                         , 9869 , 174, 108), // #1457
  INST(Vsubps           , VexRvm_Lx          , V(000F00,5C,_,x,I,0,4,FV ), 0                         , 9876 , 175, 108), // #1458
  INST(Vsubsd           , VexRvm             , V(F20F00,5C,_,I,I,1,3,T1S), 0                         , 9883 , 176, 109), // #1459
  INST(Vsubss           , VexRvm             , V(F30F00,5C,_,I,I,0,2,T1S), 0                         , 9890 , 177, 109), // #1460
  INST(Vtestpd          , VexRm_Lx           , V(660F38,0F,_,x,0,_,_,_  ), 0                         , 9897 , 256, 136), // #1461
  INST(Vtestps          , VexRm_Lx           , V(660F38,0E,_,x,0,_,_,_  ), 0                         , 9905 , 256, 136), // #1462
  INST(Vucomisd         , VexRm              , V(660F00,2E,_,I,I,1,3,T1S), 0                         , 9913 , 204, 119), // #1463
  INST(Vucomiss         , VexRm              , V(000F00,2E,_,I,I,0,2,T1S), 0                         , 9922 , 205, 119), // #1464
  INST(Vunpckhpd        , VexRvm_Lx          , V(660F00,15,_,x,I,1,4,FV ), 0                         , 9931 , 184, 108), // #1465
  INST(Vunpckhps        , VexRvm_Lx          , V(000F00,15,_,x,I,0,4,FV ), 0                         , 9941 , 185, 108), // #1466
  INST(Vunpcklpd        , VexRvm_Lx          , V(660F00,14,_,x,I,1,4,FV ), 0                         , 9951 , 184, 108), // #1467
  INST(Vunpcklps        , VexRvm_Lx          , V(000F00,14,_,x,I,0,4,FV ), 0                         , 9961 , 185, 108), // #1468
  INST(Vxorpd           , VexRvm_Lx          , V(660F00,57,_,x,I,1,4,FV ), 0                         , 9971 , 366, 114), // #1469
  INST(Vxorps           , VexRvm_Lx          , V(000F00,57,_,x,I,0,4,FV ), 0                         , 9978 , 365, 114), // #1470
  INST(Vzeroall         , VexOp              , V(000F00,77,_,1,I,_,_,_  ), 0                         , 9985 , 387, 110), // #1471
  INST(Vzeroupper       , VexOp              , V(000F00,77,_,0,I,_,_,_  ), 0                         , 9994 , 387, 110), // #1472
  INST(Wbinvd           , X86Op              , O(000F00,09,_,_,_,_,_,_  ), 0                         , 10005, 30 , 0  ), // #1473
  INST(Wbnoinvd         , X86Op              , O(F30F00,09,_,_,_,_,_,_  ), 0                         , 10012, 30 , 145), // #1474
  INST(Wrfsbase         , X86M               , O(F30F00,AE,2,_,x,_,_,_  ), 0                         , 10021, 159, 96 ), // #1475
  INST(Wrgsbase         , X86M               , O(F30F00,AE,3,_,x,_,_,_  ), 0                         , 10030, 159, 96 ), // #1476
  INST(Wrmsr            , X86Op              , O(000F00,30,_,_,_,_,_,_  ), 0                         , 10039, 160, 146), // #1477
  INST(Xabort           , X86Op_O_I8         , O(000000,C6,7,_,_,_,_,_  ), 0                         , 10045, 73 , 147), // #1478
  INST(Xadd             , X86Xadd            , O(000F00,C0,_,_,x,_,_,_  ), 0                         , 10052, 388, 148), // #1479
  INST(Xbegin           , X86JmpRel          , O(000000,C7,7,_,_,_,_,_  ), 0                         , 10057, 389, 147), // #1480
  INST(Xchg             , X86Xchg            , O(000000,86,_,_,x,_,_,_  ), 0                         , 448  , 390, 0  ), // #1481
  INST(Xend             , X86Op              , O(000F01,D5,_,_,_,_,_,_  ), 0                         , 10064, 30 , 147), // #1482
  INST(Xgetbv           , X86Op              , O(000F01,D0,_,_,_,_,_,_  ), 0                         , 10069, 160, 149), // #1483
  INST(Xlatb            , X86Op              , O(000000,D7,_,_,_,_,_,_  ), 0                         , 10076, 30 , 0  ), // #1484
  INST(Xor              , X86Arith           , O(000000,30,6,_,x,_,_,_  ), 0                         , 9190 , 170, 1  ), // #1485
  INST(Xorpd            , ExtRm              , O(660F00,57,_,_,_,_,_,_  ), 0                         , 9972 , 138, 4  ), // #1486
  INST(Xorps            , ExtRm              , O(000F00,57,_,_,_,_,_,_  ), 0                         , 9979 , 138, 5  ), // #1487
  INST(Xrstor           , X86M_Only          , O(000F00,AE,5,_,_,_,_,_  ), 0                         , 1119 , 391, 149), // #1488
  INST(Xrstor64         , X86M_Only          , O(000F00,AE,5,_,1,_,_,_  ), 0                         , 1127 , 392, 149), // #1489
  INST(Xrstors          , X86M_Only          , O(000F00,C7,3,_,_,_,_,_  ), 0                         , 10082, 391, 150), // #1490
  INST(Xrstors64        , X86M_Only          , O(000F00,C7,3,_,1,_,_,_  ), 0                         , 10090, 392, 150), // #1491
  INST(Xsave            , X86M_Only          , O(000F00,AE,4,_,_,_,_,_  ), 0                         , 1137 , 391, 149), // #1492
  INST(Xsave64          , X86M_Only          , O(000F00,AE,4,_,1,_,_,_  ), 0                         , 1144 , 392, 149), // #1493
  INST(Xsavec           , X86M_Only          , O(000F00,C7,4,_,_,_,_,_  ), 0                         , 10100, 391, 151), // #1494
  INST(Xsavec64         , X86M_Only          , O(000F00,C7,4,_,1,_,_,_  ), 0                         , 10107, 392, 151), // #1495
  INST(Xsaveopt         , X86M_Only          , O(000F00,AE,6,_,_,_,_,_  ), 0                         , 10116, 391, 152), // #1496
  INST(Xsaveopt64       , X86M_Only          , O(000F00,AE,6,_,1,_,_,_  ), 0                         , 10125, 392, 152), // #1497
  INST(Xsaves           , X86M_Only          , O(000F00,C7,5,_,_,_,_,_  ), 0                         , 10136, 391, 150), // #1498
  INST(Xsaves64         , X86M_Only          , O(000F00,C7,5,_,1,_,_,_  ), 0                         , 10143, 392, 150), // #1499
  INST(Xsetbv           , X86Op              , O(000F01,D1,_,_,_,_,_,_  ), 0                         , 10152, 160, 153), // #1500
  INST(Xtest            , X86Op              , O(000F01,D6,_,_,_,_,_,_  ), 0                         , 10159, 30 , 154)  // #1501
  // ${InstInfo:End}
};
#undef NAME_DATA_INDEX
#undef INST

// ============================================================================
// [asmjit::x86::InstDB - EncodingTable]
// ============================================================================

// ${EncodingTable:Begin}
// ------------------- Automatically generated, do not edit -------------------
#define E(VAL) InstDB::kEncoding##VAL
const uint8_t InstDB::_encodingTable[] = {
  E(None), E(X86Op_xAX), E(X86I_xAX), E(X86I_xAX), E(X86Op_xAX), E(X86Arith), E(X86Rm),
  E(X86Arith), E(ExtRm), E(ExtRm), E(ExtRm), E(ExtRm), E(ExtRm), E(ExtRm),
  E(X86Rm), E(ExtRm), E(ExtRm), E(ExtRm), E(ExtRm), E(ExtRm), E(ExtRmi), E(X86Arith),
  E(VexRvm_Wx), E(ExtRm), E(ExtRm), E(ExtRm), E(ExtRm), E(X86Mr_NoSize),
  E(VexRmv_Wx), E(VexVm_Wx), E(VexVm_Wx), E(VexVm_Wx), E(VexVm_Wx), E(VexVm_Wx),
  E(ExtRmi), E(ExtRmi), E(ExtRm_XMM0), E(ExtRm_XMM0), E(VexVm_Wx), E(VexVm_Wx),
  E(VexVm_Wx), E(VexVm_Wx), E(VexVm_Wx), E(X86Rm), E(X86Rm), E(X86Rm), E(X86Rm),
  E(X86Rm), E(X86Bndmov), E(X86Mr), E(X86Rm), E(X86Rm), E(X86Rm), E(X86Bswap),
  E(X86Bt), E(X86Bt), E(X86Bt), E(X86Bt), E(VexRmv_Wx), E(X86Call), E(X86Op_xAX),
  E(X86Op_xDX_xAX), E(X86Op_xAX), E(X86Op), E(X86Op), E(X86Op), E(X86M_Only),
  E(X86M_Only), E(X86M_Only), E(X86Op), E(X86Op), E(X86Op), E(X86M_Only), E(X86Op_MemZAX),
  E(X86Op), E(X86Rm), E(X86Rm), E(X86Rm), E(X86Rm), E(X86Rm), E(X86Rm),
  E(X86Rm), E(X86Rm), E(X86Rm), E(X86Rm), E(X86Rm), E(X86Rm), E(X86Rm), E(X86Rm),
  E(X86Rm), E(X86Rm), E(X86Rm), E(X86Rm), E(X86Rm), E(X86Rm), E(X86Rm),
  E(X86Rm), E(X86Rm), E(X86Rm), E(X86Rm), E(X86Rm), E(X86Rm), E(X86Rm), E(X86Rm),
  E(X86Rm), E(X86Arith), E(ExtRmi), E(ExtRmi), E(X86StrMm), E(ExtRmi), E(ExtRmi),
  E(X86Cmpxchg), E(X86Cmpxchg8b_16b), E(X86Cmpxchg8b_16b), E(ExtRm), E(ExtRm),
  E(X86Op), E(X86Op_xDX_xAX), E(X86Crc), E(ExtRm), E(ExtRm), E(ExtRm), E(ExtRm),
  E(ExtRm), E(ExtRm), E(ExtRm), E(ExtRm), E(ExtRm), E(ExtRm), E(ExtRm_Wx), E(ExtRm),
  E(ExtRm_Wx), E(ExtRm_Wx), E(ExtRm), E(ExtRm_Wx), E(ExtRm), E(ExtRm),
  E(ExtRm), E(ExtRm), E(ExtRm_Wx), E(ExtRm_Wx), E(X86Op_xDX_xAX), E(X86Op_xAX),
  E(X86Op), E(X86Op), E(X86IncDec), E(X86M_GPB_MulDiv), E(ExtRm), E(ExtRm), E(ExtRm),
  E(ExtRm), E(ExtRmi), E(ExtRmi), E(X86Op), E(X86Enter), E(ExtExtract), E(ExtExtrq),
  E(FpuOp), E(FpuOp), E(FpuArith), E(FpuRDef), E(X86M_Only), E(X86M_Only),
  E(FpuOp), E(FpuOp), E(FpuR), E(FpuR), E(FpuR), E(FpuR), E(FpuR), E(FpuR),
  E(FpuR), E(FpuR), E(FpuCom), E(FpuR), E(FpuR), E(FpuCom), E(FpuOp), E(FpuOp),
  E(FpuOp), E(FpuArith), E(FpuRDef), E(FpuArith), E(FpuRDef), E(X86Op), E(FpuR),
  E(FpuM), E(FpuM), E(FpuM), E(FpuM), E(FpuM), E(FpuM), E(FpuM), E(FpuOp), E(FpuOp),
  E(FpuM), E(FpuM), E(FpuM), E(FpuM), E(FpuM), E(FpuFldFst), E(FpuOp), E(X86M_Only),
  E(X86M_Only), E(FpuOp), E(FpuOp), E(FpuOp), E(FpuOp), E(FpuOp), E(FpuOp),
  E(FpuArith), E(FpuRDef), E(FpuOp), E(FpuOp), E(FpuOp), E(X86M_Only),
  E(X86M_Only), E(X86M_Only), E(FpuStsw), E(FpuOp), E(FpuOp), E(FpuOp), E(FpuOp),
  E(FpuOp), E(X86M_Only), E(X86M_Only), E(FpuOp), E(FpuOp), E(FpuOp), E(FpuOp),
  E(FpuFldFst), E(X86M_Only), E(X86M_Only), E(FpuFldFst), E(FpuStsw), E(FpuArith),
  E(FpuRDef), E(FpuArith), E(FpuRDef), E(FpuOp), E(FpuRDef), E(FpuR), E(FpuR),
  E(FpuRDef), E(FpuOp), E(X86Op), E(FpuOp), E(FpuR), E(X86M_Only), E(X86M_Only),
  E(X86M_Only), E(X86M_Only), E(FpuOp), E(FpuOp), E(FpuOp), E(X86Op), E(ExtRmi),
  E(ExtRmi), E(ExtRm), E(ExtRm), E(ExtRm), E(X86Op), E(ExtRm), E(ExtRm), E(X86M_GPB_MulDiv),
  E(X86Imul), E(X86In), E(X86IncDec), E(X86Ins), E(ExtRmi), E(ExtInsertq),
  E(X86Int), E(X86Op), E(X86Op), E(X86Op), E(X86Rm_NoSize), E(X86M_Only),
  E(X86Op_xAddr), E(X86Rm_NoSize), E(X86Rm_NoSize), E(X86Op), E(X86Op),
  E(X86Op), E(X86Op), E(X86Jcc), E(X86Jcc), E(X86Jcc), E(X86Jcc), E(X86Jcc), E(X86Jcc),
  E(X86JecxzLoop), E(X86Jcc), E(X86Jcc), E(X86Jcc), E(X86Jcc), E(X86Jmp),
  E(X86Jcc), E(X86Jcc), E(X86Jcc), E(X86Jcc), E(X86Jcc), E(X86Jcc), E(X86Jcc),
  E(X86Jcc), E(X86Jcc), E(X86Jcc), E(X86Jcc), E(X86Jcc), E(X86Jcc), E(X86Jcc),
  E(X86Jcc), E(X86Jcc), E(X86Jcc), E(X86Jcc), E(X86Jcc), E(X86Jcc), E(VexRvm), E(VexRvm),
  E(VexRvm), E(VexRvm), E(VexRvm), E(VexRvm), E(VexRvm), E(VexRvm), E(VexRvm),
  E(VexRvm), E(VexRvm), E(VexRvm), E(VexKmov), E(VexKmov), E(VexKmov),
  E(VexKmov), E(VexRm), E(VexRm), E(VexRm), E(VexRm), E(VexRvm), E(VexRvm), E(VexRvm),
  E(VexRm), E(VexRm), E(VexRm), E(VexRm), E(VexRvm), E(VexRmi), E(VexRmi),
  E(VexRmi), E(VexRmi), E(VexRmi), E(VexRmi), E(VexRmi), E(VexRmi), E(VexRm),
  E(VexRm), E(VexRm), E(VexRm), E(VexRvm), E(VexRvm), E(VexRvm), E(VexRvm), E(VexRvm),
  E(VexRvm), E(VexRvm), E(VexRvm), E(VexRvm), E(VexRvm), E(VexRvm), E(X86Op),
  E(X86Rm), E(ExtRm), E(X86M_Only), E(X86Rm), E(X86Lea), E(X86Op), E(X86Rm),
  E(X86Fence), E(X86Rm), E(X86M_Only), E(X86Rm), E(X86M_Only), E(X86M_NoSize),
  E(VexR_Wx), E(X86M_NoSize), E(X86StrRm), E(X86JecxzLoop), E(X86JecxzLoop), E(X86JecxzLoop),
  E(X86Rm), E(X86Rm), E(X86M_NoSize), E(VexVmi4_Wx), E(VexVmi4_Wx),
  E(X86Rm_Raw66H), E(ExtRm_ZDI), E(ExtRm_ZDI), E(ExtRm), E(ExtRm), E(ExtRm),
  E(ExtRm), E(X86Fence), E(ExtRm), E(ExtRm), E(ExtRm), E(ExtRm), E(X86Op), E(X86Op),
  E(X86Mov), E(ExtMov), E(ExtMov), E(ExtMovbe), E(ExtMovd), E(ExtMov), E(X86Movdir64b),
  E(X86MovntiMovdiri), E(ExtMov), E(ExtMov), E(ExtMov), E(ExtMov),
  E(ExtMov), E(ExtMov), E(ExtMov), E(ExtMov), E(ExtMov), E(ExtMov), E(ExtMov), E(ExtMov),
  E(ExtMov), E(X86MovntiMovdiri), E(ExtMov), E(ExtMov), E(ExtMov), E(ExtMov),
  E(ExtMov), E(ExtMovq), E(ExtRm), E(X86StrMm), E(ExtMov), E(ExtRm), E(ExtRm),
  E(ExtMov), E(X86MovsxMovzx), E(X86Rm), E(ExtMov), E(ExtMov), E(X86MovsxMovzx),
  E(ExtRmi), E(X86M_GPB_MulDiv), E(ExtRm), E(ExtRm), E(ExtRm), E(ExtRm),
  E(VexRvm_ZDX_Wx), E(X86Op), E(X86Op), E(X86M_GPB), E(X86M_Nop), E(X86M_GPB),
  E(X86Arith), E(ExtRm), E(ExtRm), E(X86Out), E(X86Outs), E(ExtRm_P), E(ExtRm_P),
  E(ExtRm_P), E(ExtRm_P), E(ExtRm_P), E(ExtRm), E(ExtRm_P), E(ExtRm_P), E(ExtRm_P),
  E(ExtRm_P), E(ExtRm_P), E(ExtRm_P), E(ExtRm_P), E(ExtRm_P), E(ExtRm_P),
  E(ExtRmi_P), E(ExtRm_P), E(ExtRm_P), E(X86Op), E(ExtRm_P), E(Ext3dNow), E(ExtRm_P),
  E(ExtRm_XMM0), E(ExtRmi), E(ExtRmi), E(ExtRm_P), E(ExtRm_P), E(ExtRm), E(ExtRm_P),
  E(ExtRmi), E(ExtRmi), E(ExtRm_P), E(ExtRm_P), E(ExtRm), E(ExtRm_P),
  E(ExtRmi), E(ExtRmi), E(X86Op_O), E(VexRvm_Wx), E(VexRvm_Wx), E(ExtExtract),
  E(ExtExtract), E(ExtExtract), E(ExtPextrw), E(Ext3dNow), E(Ext3dNow), E(Ext3dNow),
  E(Ext3dNow), E(Ext3dNow), E(Ext3dNow), E(Ext3dNow), E(Ext3dNow), E(Ext3dNow),
  E(Ext3dNow), E(Ext3dNow), E(Ext3dNow), E(Ext3dNow), E(Ext3dNow), E(Ext3dNow),
  E(Ext3dNow), E(Ext3dNow), E(Ext3dNow), E(Ext3dNow), E(Ext3dNow), E(Ext3dNow),
  E(ExtRm_P), E(ExtRm_P), E(ExtRm_P), E(ExtRm), E(ExtRm_P), E(ExtRm_P), E(ExtRm_P),
  E(Ext3dNow), E(Ext3dNow), E(ExtRmi), E(ExtRmi), E(ExtRmi), E(ExtRmi_P),
  E(ExtRm_P), E(ExtRm_P), E(ExtRm), E(ExtRm), E(ExtRm_P), E(ExtRm_P), E(ExtRm),
  E(ExtRm), E(ExtRm), E(ExtRm), E(ExtRm_P), E(ExtRm_P), E(ExtRm), E(ExtRm), E(ExtRm_P),
  E(ExtRm), E(ExtRm), E(ExtRm), E(ExtRm), E(ExtRm), E(ExtRm), E(ExtRm),
  E(ExtRm), E(ExtRm), E(ExtRm), E(ExtRm), E(ExtRm), E(ExtRm), E(ExtRm_P), E(Ext3dNow),
  E(ExtRm_P), E(ExtRm_P), E(ExtRm), E(ExtRm_P), E(ExtRm_P), E(X86Pop),
  E(X86Op), E(X86Op), E(X86Rm_Raw66H), E(X86Op), E(X86Op), E(X86Op), E(ExtRm_P),
  E(X86M_Only), E(X86M_Only), E(X86M_Only), E(X86M_Only), E(X86M_Only), E(X86M_Only),
  E(X86M_Only), E(ExtRm_P), E(ExtRm_P), E(ExtRmi), E(ExtRmi), E(ExtRmi),
  E(ExtRmi_P), E(ExtRm_P), E(ExtRm_P), E(ExtRm_P), E(ExtRmRi_P), E(ExtRmRi), E(ExtRmRi_P),
  E(ExtRmRi_P), E(ExtRmRi_P), E(ExtRmRi_P), E(ExtRmRi_P), E(ExtRmRi),
  E(ExtRmRi_P), E(ExtRmRi_P), E(ExtRm_P), E(ExtRm_P), E(ExtRm_P), E(ExtRm_P),
  E(ExtRm_P), E(ExtRm_P), E(ExtRm_P), E(ExtRm_P), E(Ext3dNow), E(ExtRm), E(ExtRm_P),
  E(ExtRm_P), E(ExtRm), E(ExtRm_P), E(ExtRm_P), E(ExtRm_P), E(ExtRm), E(ExtRm_P),
  E(X86Push), E(X86Op), E(X86Op), E(X86Op), E(X86Op), E(X86Op), E(ExtRm_P),
  E(X86Rot), E(ExtRm), E(ExtRm), E(X86Rot), E(X86M), E(X86M), E(X86Op), E(X86R_Native),
  E(X86Op), E(X86M), E(X86M), E(X86Op), E(X86Op), E(X86Ret), E(X86Rot),
  E(X86Rot), E(VexRmi_Wx), E(ExtRmi), E(ExtRmi), E(ExtRmi), E(ExtRmi), E(X86Op),
  E(ExtRm), E(ExtRm), E(X86Op), E(X86Rot), E(X86Rot), E(VexRmv_Wx), E(X86Arith),
  E(X86StrRm), E(X86Set), E(X86Set), E(X86Set), E(X86Set), E(X86Set), E(X86Set),
  E(X86Set), E(X86Set), E(X86Set), E(X86Set), E(X86Set), E(X86Set), E(X86Set),
  E(X86Set), E(X86Set), E(X86Set), E(X86Set), E(X86Set), E(X86Set), E(X86Set),
  E(X86Set), E(X86Set), E(X86Set), E(X86Set), E(X86Set), E(X86Set), E(X86Set),
  E(X86Set), E(X86Set), E(X86Set), E(X86Fence), E(X86M_Only), E(ExtRm), E(ExtRm),
  E(ExtRm), E(ExtRmi), E(ExtRm), E(ExtRm), E(ExtRm_XMM0), E(X86Rot), E(X86ShldShrd),
  E(VexRmv_Wx), E(X86Rot), E(X86ShldShrd), E(VexRmv_Wx), E(ExtRmi), E(ExtRmi),
  E(X86M_Only), E(X86Op_xAX), E(X86M), E(VexR_Wx), E(X86M), E(ExtRm), E(ExtRm),
  E(ExtRm), E(ExtRm), E(X86Op), E(X86Op), E(X86Op), E(X86Op), E(X86Op),
  E(X86M_Only), E(X86StrMr), E(X86M), E(X86Arith), E(ExtRm), E(ExtRm), E(ExtRm),
  E(ExtRm), E(X86Op), E(X86Op), E(X86Op), E(X86Op), E(X86Op), E(X86Op), E(X86Op),
  E(VexVm_Wx), E(X86Test), E(X86Rm_Raw66H), E(VexVm_Wx), E(ExtRm), E(ExtRm),
  E(X86Op), E(ExtRm), E(ExtRm), E(ExtRm), E(ExtRm), E(VexRm_T1_4X), E(VexRm_T1_4X),
  E(VexRm_T1_4X), E(VexRm_T1_4X), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm), E(VexRvm),
  E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx),
  E(VexRvm_Lx), E(VexRm), E(VexRmi), E(VexRvmi_Lx), E(VexRvmi_Lx), E(VexRvm_Lx),
  E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx),
  E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvmi_Lx), E(VexRvmi_Lx),
  E(VexRvmr_Lx), E(VexRvmr_Lx), E(VexRm), E(VexRm_Lx), E(VexRm_Lx), E(VexRm),
  E(VexRm_Lx), E(VexRm), E(VexRm), E(VexRm_Lx), E(VexRm_Lx), E(VexRm), E(VexRm_Lx),
  E(VexRm), E(VexRm_Lx), E(VexRm_Lx), E(VexRvmi_Lx), E(VexRvmi_Lx), E(VexRvmi),
  E(VexRvmi), E(VexRm), E(VexRm), E(VexMr_Lx), E(VexMr_Lx), E(VexRm_Lx), E(VexRm_Lx),
  E(VexRvm), E(VexRm), E(VexRm_Lx), E(VexRm_Lx), E(VexRm_Lx), E(VexRm_Lx),
  E(VexRm_Lx), E(VexRm_Lx), E(VexRm_Lx), E(VexRm_Lx), E(VexMri_Lx), E(VexRm_Lx),
  E(VexRm_Lx), E(VexRm_Lx), E(VexRm_Lx), E(VexRm_Lx), E(VexRm_Wx), E(VexRvm),
  E(VexRm_Wx), E(VexRvm_Wx), E(VexRvm_Wx), E(VexRvm), E(VexRm_Wx), E(VexRm_Wx),
  E(VexRm_Lx), E(VexRm_Lx), E(VexRm_Lx), E(VexRm_Lx), E(VexRm_Lx), E(VexRm_Lx),
  E(VexRm_Lx), E(VexRm_Lx), E(VexRm_Wx), E(VexRm_Wx), E(VexRm_Wx), E(VexRm_Wx),
  E(VexRm_Lx), E(VexRm_Lx), E(VexRm_Lx), E(VexRm_Lx), E(VexRvm_Wx), E(VexRvm_Wx),
  E(VexRvmi_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm), E(VexRvm), E(VexRvm),
  E(VexRvmi_Lx), E(VexRvmi_Lx), E(X86M_NoSize), E(X86M_NoSize), E(VexRm), E(VexRm),
  E(VexRm_Lx), E(VexRm_Lx), E(VexMri), E(VexMri_Lx), E(VexMri), E(VexMri_Lx),
  E(VexMri), E(VexMri), E(VexMri_Lx), E(VexMri), E(VexMri_Lx), E(VexMri),
  E(VexMri), E(VexRvmi_Lx), E(VexRvmi_Lx), E(VexRvmi), E(VexRvmi), E(VexRvm_Lx),
  E(VexRvm_Lx), E(VexRvm), E(VexRvm), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm), E(VexRvm),
  E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm), E(VexRvm), E(Fma4_Lx), E(Fma4_Lx),
  E(Fma4), E(Fma4), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx),
  E(VexRvm_Lx), E(VexRvm_Lx), E(Fma4_Lx), E(Fma4_Lx), E(VexRvm_Lx), E(VexRvm_Lx),
  E(VexRvm), E(VexRvm), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm), E(VexRvm), E(VexRvm_Lx),
  E(VexRvm_Lx), E(VexRvm), E(VexRvm), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx),
  E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(Fma4_Lx), E(Fma4_Lx), E(Fma4_Lx),
  E(Fma4_Lx), E(Fma4), E(Fma4), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm),
  E(VexRvm), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm), E(VexRvm), E(VexRvm_Lx), E(VexRvm_Lx),
  E(VexRvm), E(VexRvm), E(Fma4_Lx), E(Fma4_Lx), E(Fma4), E(Fma4), E(VexRvm_Lx),
  E(VexRvm_Lx), E(VexRvm), E(VexRvm), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm),
  E(VexRvm), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm), E(VexRvm), E(Fma4_Lx),
  E(Fma4_Lx), E(Fma4), E(Fma4), E(VexRmi_Lx), E(VexRmi_Lx), E(VexRmi_Lx), E(VexRmi_Lx),
  E(VexRm_Lx), E(VexRm_Lx), E(VexRm), E(VexRm), E(VexRmvRm_VM), E(VexRmvRm_VM),
  E(VexM_VM), E(VexM_VM), E(VexM_VM), E(VexM_VM), E(VexM_VM), E(VexM_VM),
  E(VexM_VM), E(VexM_VM), E(VexRmvRm_VM), E(VexRmvRm_VM), E(VexRm_Lx), E(VexRm_Lx),
  E(VexRvm), E(VexRvm), E(VexRmi_Lx), E(VexRmi_Lx), E(VexRvmi), E(VexRvmi),
  E(VexRvmi_Lx), E(VexRvmi_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx),
  E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvmi), E(VexRvmi_Lx), E(VexRvmi), E(VexRvmi_Lx),
  E(VexRvmi), E(VexRvmi), E(VexRvmi_Lx), E(VexRvmi), E(VexRvmi_Lx), E(VexRvmi),
  E(VexRvmi), E(VexRm_Lx), E(VexM), E(VexRm_ZDI), E(VexRvmMvr_Lx), E(VexRvmMvr_Lx),
  E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm), E(VexRvm), E(X86Op), E(X86M_Only),
  E(X86Op), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm), E(VexRvm), E(X86Op), E(X86Op_xAX),
  E(X86Op), E(VexRmMr_Lx), E(VexRmMr_Lx), E(VexMovdMovq), E(VexRm_Lx),
  E(VexRmMr_Lx), E(VexRmMr_Lx), E(VexRmMr_Lx), E(VexRmMr_Lx), E(VexRmMr_Lx), E(VexRmMr_Lx),
  E(VexRmMr_Lx), E(VexRmMr_Lx), E(VexRvm), E(VexRvmMr), E(VexRvmMr),
  E(VexRvm), E(VexRvmMr), E(VexRvmMr), E(VexRm_Lx), E(VexRm_Lx), E(VexMr_Lx),
  E(VexRm_Lx), E(VexMr_Lx), E(VexMr_Lx), E(VexMovdMovq), E(VexMovssMovsd), E(VexRm_Lx),
  E(VexRm_Lx), E(VexMovssMovsd), E(VexRmMr_Lx), E(VexRmMr_Lx), E(VexRvmi_Lx),
  E(X86M_Only), E(X86M_Only), E(X86Mr_NoSize), E(X86Op), E(X86Op_xAX), E(X86Op_xAX),
  E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(X86Rm_NoSize),
  E(X86M_Only), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRm_T1_4X), E(VexRm_T1_4X),
  E(VexRm_Lx), E(VexRm_Lx), E(VexRm_Lx), E(VexRm_Lx), E(VexRvm_Lx), E(VexRvm_Lx),
  E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx),
  E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvmi_Lx),
  E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx),
  E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvmi_Lx), E(VexRvmr), E(VexRvmi_Lx),
  E(VexRm_Lx), E(VexRm_Lx), E(VexRm_Lx), E(VexRm_Lx), E(VexRm_Lx), E(VexRm_Lx),
  E(VexRvmi_Lx), E(VexRvrmRvmr_Lx), E(VexRvmi_Lx), E(VexRvmi_Lx), E(VexRvm_Lx),
  E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRmi), E(VexRmi), E(VexRvm_Lx),
  E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRmi), E(VexRmi), E(VexRvmi_Lx),
  E(VexRvmi_Lx), E(VexRvmi_Lx), E(VexRvmi_Lx), E(VexRvmi_Lx), E(VexRvmi_Lx),
  E(VexRvmi), E(VexRvmi), E(VexMr_Lx), E(VexMr_Lx), E(VexMr_Lx), E(VexMr_Lx), E(VexRvmi),
  E(VexRvmi), E(VexRvmi), E(VexRvmi), E(VexRvmi), E(VexRvmi), E(VexRm_Lx),
  E(VexRm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvmi),
  E(VexRvmi), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx),
  E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvrmiRvmri_Lx), E(VexRvrmiRvmri_Lx),
  E(VexRvmRmi_Lx), E(VexRvmRmi_Lx), E(VexRmi), E(VexRvm), E(VexRvmRmi_Lx),
  E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx),
  E(VexRvm_Lx), E(VexRvm_Lx), E(VexRm_Lx), E(VexRm_Lx), E(VexRm_Lx), E(VexRm_Lx),
  E(VexMri), E(VexMri), E(VexMri), E(VexMri), E(VexRmvRm_VM), E(VexRmvRm_VM),
  E(VexRmvRm_VM), E(VexRmvRm_VM), E(VexRm), E(VexRm), E(VexRm), E(VexRvm_Lx),
  E(VexRm), E(VexRvm_Lx), E(VexRm), E(VexRm), E(VexRm), E(VexRm), E(VexRm), E(VexRm),
  E(VexRvm_Lx), E(VexRm), E(VexRm), E(VexRm), E(VexRm), E(VexRvm_Lx), E(VexRm),
  E(VexRvm_Lx), E(VexRvm_Lx), E(VexRm), E(VexRvmi), E(VexRvmi), E(VexRvmi),
  E(VexRvmi), E(VexRm_Lx), E(VexRm_Lx), E(VexRvmr), E(VexRvmr), E(VexRvmr),
  E(VexRvmr), E(VexRvmr), E(VexRvmr), E(VexRvmr), E(VexRvmr), E(VexRvmr), E(VexRvmr),
  E(VexRvmr), E(VexRvmr), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx),
  E(VexRvmMvr_Lx), E(VexRvmMvr_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx),
  E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx),
  E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx),
  E(VexRvm_Lx), E(VexRvm_Lx), E(VexRm_Lx), E(VexRm_Lx), E(VexMr_Lx), E(VexMr_Lx),
  E(VexRm_Lx), E(VexRm_Lx), E(VexRm_Lx), E(VexRm_Lx), E(VexRm_Lx), E(VexRm_Lx),
  E(VexMr_Lx), E(VexMr_Lx), E(VexMr_Lx), E(VexMr_Lx), E(VexMr_Lx), E(VexMr_Lx),
  E(VexMr_Lx), E(VexMr_Lx), E(VexMr_Lx), E(VexRm_Lx), E(VexRm_Lx), E(VexRm_Lx),
  E(VexRm_Lx), E(VexRm_Lx), E(VexRm_Lx), E(VexMr_Lx), E(VexMr_Lx), E(VexMr_Lx),
  E(VexMr_Lx), E(VexMr_Lx), E(VexMr_Lx), E(VexRm_Lx), E(VexMr_Lx), E(VexRm_Lx),
  E(VexRm_Lx), E(VexRm_Lx), E(VexRm_Lx), E(VexRm_Lx), E(VexRm_Lx), E(VexRvm_Lx),
  E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx),
  E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRm_Lx), E(VexRm_Lx), E(VexRm_Lx),
  E(VexRm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvrmRvmr), E(VexVmi_Lx),
  E(VexVmi_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexVmi_Lx), E(VexVmi_Lx),
  E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvmRmvRmi), E(VexRvmRmvRmi), E(VexRvmRmvRmi),
  E(VexRvmRmvRmi), E(VexRvm_Lx), E(VexMr_VM), E(VexMr_VM), E(VexMr_VM), E(VexMr_VM),
  E(VexRvmRmv), E(VexRvmRmv), E(VexRvmRmv), E(VexRvmRmv), E(VexRvmRmv),
  E(VexRvmRmv), E(VexRvmi_Lx), E(VexRvmi_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx),
  E(VexRvmi_Lx), E(VexRvmRmv), E(VexRvmRmv), E(VexRvmi_Lx), E(VexRvmi_Lx),
  E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvmi_Lx), E(VexRvm_Lx), E(VexRvm_Lx),
  E(VexRmi_Lx), E(VexRmi_Lx), E(VexRmi_Lx), E(VexRvm_Lx), E(VexRvm_Lx),
  E(VexRvm_Lx), E(VexRvmVmi_Lx), E(VexEvexVmi_Lx), E(VexRvmVmi_Lx), E(VexRvm_Lx),
  E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvmVmi_Lx), E(VexRvmVmi_Lx), E(VexRvmVmi_Lx),
  E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvmVmi_Lx), E(VexRvmVmi_Lx),
  E(VexEvexVmi_Lx), E(VexRvmVmi_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx),
  E(VexRvmVmi_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx),
  E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvmi_Lx), E(VexRvmi_Lx),
  E(VexRm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx),
  E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx),
  E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx),
  E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvmi_Lx), E(VexRvmi_Lx),
  E(VexRvmi), E(VexRvmi), E(VexRm_Lx), E(VexRm_Lx), E(VexRvm), E(VexRvm), E(VexRm),
  E(VexRm), E(VexRvm), E(VexRvm), E(VexRm_Lx), E(VexRvm), E(VexRmi_Lx), E(VexRmi_Lx),
  E(VexRvmi), E(VexRvmi), E(VexRmi_Lx), E(VexRmi_Lx), E(VexRvmi), E(VexRvmi),
  E(VexRmi_Lx), E(VexRmi_Lx), E(VexRvmi), E(VexRvmi), E(VexRm_Lx), E(VexRm_Lx),
  E(VexRvm), E(VexRvm), E(VexRm), E(VexRm), E(VexRvm), E(VexRvm), E(VexRm_Lx),
  E(VexRvm), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm), E(VexRvm), E(VexMr_Lx),
  E(VexMr_Lx), E(VexM_VM), E(VexM_VM), E(VexM_VM), E(VexM_VM), E(VexM_VM),
  E(VexM_VM), E(VexM_VM), E(VexM_VM), E(VexMr_Lx), E(VexMr_Lx), E(VexRvmi_Lx), E(VexRvmi_Lx),
  E(VexRvmi_Lx), E(VexRvmi_Lx), E(VexRvmi_Lx), E(VexRvmi_Lx), E(VexRm_Lx),
  E(VexRm_Lx), E(VexRvm), E(VexRvm), E(VexM), E(VexRvm_Lx), E(VexRvm_Lx),
  E(VexRvm), E(VexRvm), E(VexRm_Lx), E(VexRm_Lx), E(VexRm), E(VexRm), E(VexRvm_Lx),
  E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexOp),
  E(VexOp), E(X86Op), E(X86Op), E(X86M), E(X86M), E(X86Op), E(X86Op_O_I8),
  E(X86Xadd), E(X86JmpRel), E(X86Xchg), E(X86Op), E(X86Op), E(X86Op), E(X86Arith),
  E(ExtRm), E(ExtRm), E(X86M_Only), E(X86M_Only), E(X86M_Only), E(X86M_Only),
  E(X86M_Only), E(X86M_Only), E(X86M_Only), E(X86M_Only), E(X86M_Only), E(X86M_Only),
  E(X86M_Only), E(X86M_Only), E(X86Op), E(X86Op)
};
#undef E
// ----------------------------------------------------------------------------
// ${EncodingTable:End}

// ============================================================================
// [asmjit::x86::InstDB - MainOpcodeTable]
// ============================================================================

// ${MainOpcodeTable:Begin}
// ------------------- Automatically generated, do not edit -------------------
const uint32_t InstDB::_mainOpcodeTable[] = {
  0                         , // #0
  O(000000,37,_,_,_,_,_,_  ), // #1
  O(000000,D5,_,_,_,_,_,_  ), // #2
  O(000000,D4,_,_,_,_,_,_  ), // #3
  O(000000,3F,_,_,_,_,_,_  ), // #4
  O(000000,10,2,_,x,_,_,_  ), // #5
  O(660F38,F6,_,_,x,_,_,_  ), // #6
  O(000000,00,0,_,x,_,_,_  ), // #7
  O(660F00,58,_,_,_,_,_,_  ), // #8
  O(000F00,58,_,_,_,_,_,_  ), // #9
  O(F20F00,58,_,_,_,_,_,_  ), // #10
  O(F30F00,58,_,_,_,_,_,_  ), // #11
  O(660F00,D0,_,_,_,_,_,_  ), // #12
  O(F20F00,D0,_,_,_,_,_,_  ), // #13
  O(F30F38,F6,_,_,x,_,_,_  ), // #14
  O(660F38,DE,_,_,_,_,_,_  ), // #15
  O(660F38,DF,_,_,_,_,_,_  ), // #16
  O(660F38,DC,_,_,_,_,_,_  ), // #17
  O(660F38,DD,_,_,_,_,_,_  ), // #18
  O(660F38,DB,_,_,_,_,_,_  ), // #19
  O(660F3A,DF,_,_,_,_,_,_  ), // #20
  O(000000,20,4,_,x,_,_,_  ), // #21
  V(000F38,F2,_,0,x,_,_,_  ), // #22
  O(660F00,55,_,_,_,_,_,_  ), // #23
  O(000F00,55,_,_,_,_,_,_  ), // #24
  O(660F00,54,_,_,_,_,_,_  ), // #25
  O(000F00,54,_,_,_,_,_,_  ), // #26
  O(000000,63,_,_,_,_,_,_  ), // #27
  V(000F38,F7,_,0,x,_,_,_  ), // #28
  V(XOP_M9,01,1,0,x,_,_,_  ), // #29
  V(XOP_M9,02,6,0,x,_,_,_  ), // #30
  V(XOP_M9,01,5,0,x,_,_,_  ), // #31
  V(XOP_M9,02,1,0,x,_,_,_  ), // #32
  V(XOP_M9,01,3,0,x,_,_,_  ), // #33
  O(660F3A,0D,_,_,_,_,_,_  ), // #34
  O(660F3A,0C,_,_,_,_,_,_  ), // #35
  O(660F38,15,_,_,_,_,_,_  ), // #36
  O(660F38,14,_,_,_,_,_,_  ), // #37
  V(XOP_M9,01,2,0,x,_,_,_  ), // #38
  V(000F38,F3,3,0,x,_,_,_  ), // #39
  V(XOP_M9,01,6,0,x,_,_,_  ), // #40
  V(000F38,F3,2,0,x,_,_,_  ), // #41
  V(000F38,F3,1,0,x,_,_,_  ), // #42
  O(F30F00,1A,_,_,_,_,_,_  ), // #43
  O(F20F00,1B,_,_,_,_,_,_  ), // #44
  O(F20F00,1A,_,_,_,_,_,_  ), // #45
  O(000F00,1A,_,_,_,_,_,_  ), // #46
  O(F30F00,1B,_,_,_,_,_,_  ), // #47
  O(660F00,1A,_,_,_,_,_,_  ), // #48
  O(000F00,1B,_,_,_,_,_,_  ), // #49
  O(000000,62,_,_,_,_,_,_  ), // #50
  O(000F00,BC,_,_,x,_,_,_  ), // #51
  O(000F00,BD,_,_,x,_,_,_  ), // #52
  O(000F00,C8,_,_,x,_,_,_  ), // #53
  O(000F00,A3,_,_,x,_,_,_  ), // #54
  O(000F00,BB,_,_,x,_,_,_  ), // #55
  O(000F00,B3,_,_,x,_,_,_  ), // #56
  O(000F00,AB,_,_,x,_,_,_  ), // #57
  V(000F38,F5,_,0,x,_,_,_  ), // #58
  O(000000,FF,2,_,_,_,_,_  ), // #59
  O(660000,98,_,_,_,_,_,_  ), // #60
  O(000000,99,_,_,_,_,_,_  ), // #61
  O(000000,98,_,_,1,_,_,_  ), // #62
  O(000F01,CA,_,_,_,_,_,_  ), // #63
  O(000000,F8,_,_,_,_,_,_  ), // #64
  O(000000,FC,_,_,_,_,_,_  ), // #65
  O(000F00,1C,0,_,_,_,_,_  ), // #66
  O(000F00,AE,7,_,_,_,_,_  ), // #67
  O(660F00,AE,7,_,_,_,_,_  ), // #68
  O(000F01,DD,_,_,_,_,_,_  ), // #69
  O(000000,FA,_,_,_,_,_,_  ), // #70
  O(000F00,06,_,_,_,_,_,_  ), // #71
  O(660F00,AE,6,_,_,_,_,_  ), // #72
  O(000F01,FC,_,_,_,_,_,_  ), // #73
  O(000000,F5,_,_,_,_,_,_  ), // #74
  O(000F00,47,_,_,x,_,_,_  ), // #75
  O(000F00,43,_,_,x,_,_,_  ), // #76
  O(000F00,42,_,_,x,_,_,_  ), // #77
  O(000F00,46,_,_,x,_,_,_  ), // #78
  O(000F00,42,_,_,x,_,_,_  ), // #79
  O(000F00,44,_,_,x,_,_,_  ), // #80
  O(000F00,4F,_,_,x,_,_,_  ), // #81
  O(000F00,4D,_,_,x,_,_,_  ), // #82
  O(000F00,4C,_,_,x,_,_,_  ), // #83
  O(000F00,4E,_,_,x,_,_,_  ), // #84
  O(000F00,46,_,_,x,_,_,_  ), // #85
  O(000F00,42,_,_,x,_,_,_  ), // #86
  O(000F00,43,_,_,x,_,_,_  ), // #87
  O(000F00,47,_,_,x,_,_,_  ), // #88
  O(000F00,43,_,_,x,_,_,_  ), // #89
  O(000F00,45,_,_,x,_,_,_  ), // #90
  O(000F00,4E,_,_,x,_,_,_  ), // #91
  O(000F00,4C,_,_,x,_,_,_  ), // #92
  O(000F00,4D,_,_,x,_,_,_  ), // #93
  O(000F00,4F,_,_,x,_,_,_  ), // #94
  O(000F00,41,_,_,x,_,_,_  ), // #95
  O(000F00,4B,_,_,x,_,_,_  ), // #96
  O(000F00,49,_,_,x,_,_,_  ), // #97
  O(000F00,45,_,_,x,_,_,_  ), // #98
  O(000F00,40,_,_,x,_,_,_  ), // #99
  O(000F00,4A,_,_,x,_,_,_  ), // #100
  O(000F00,4A,_,_,x,_,_,_  ), // #101
  O(000F00,4B,_,_,x,_,_,_  ), // #102
  O(000F00,48,_,_,x,_,_,_  ), // #103
  O(000F00,44,_,_,x,_,_,_  ), // #104
  O(000000,38,7,_,x,_,_,_  ), // #105
  O(660F00,C2,_,_,_,_,_,_  ), // #106
  O(000F00,C2,_,_,_,_,_,_  ), // #107
  O(000000,A6,_,_,_,_,_,_  ), // #108
  O(F20F00,C2,_,_,_,_,_,_  ), // #109
  O(F30F00,C2,_,_,_,_,_,_  ), // #110
  O(000F00,B0,_,_,x,_,_,_  ), // #111
  O(000F00,C7,1,_,1,_,_,_  ), // #112
  O(000F00,C7,1,_,_,_,_,_  ), // #113
  O(660F00,2F,_,_,_,_,_,_  ), // #114
  O(000F00,2F,_,_,_,_,_,_  ), // #115
  O(000F00,A2,_,_,_,_,_,_  ), // #116
  O(000000,99,_,_,1,_,_,_  ), // #117
  O(F20F38,F0,_,_,x,_,_,_  ), // #118
  O(F30F00,E6,_,_,_,_,_,_  ), // #119
  O(000F00,5B,_,_,_,_,_,_  ), // #120
  O(F20F00,E6,_,_,_,_,_,_  ), // #121
  O(660F00,2D,_,_,_,_,_,_  ), // #122
  O(660F00,5A,_,_,_,_,_,_  ), // #123
  O(660F00,2A,_,_,_,_,_,_  ), // #124
  O(000F00,2A,_,_,_,_,_,_  ), // #125
  O(660F00,5B,_,_,_,_,_,_  ), // #126
  O(000F00,5A,_,_,_,_,_,_  ), // #127
  O(000F00,2D,_,_,_,_,_,_  ), // #128
  O(F20F00,2D,_,_,x,_,_,_  ), // #129
  O(F20F00,5A,_,_,_,_,_,_  ), // #130
  O(F20F00,2A,_,_,x,_,_,_  ), // #131
  O(F30F00,2A,_,_,x,_,_,_  ), // #132
  O(F30F00,5A,_,_,_,_,_,_  ), // #133
  O(F30F00,2D,_,_,x,_,_,_  ), // #134
  O(660F00,E6,_,_,_,_,_,_  ), // #135
  O(660F00,2C,_,_,_,_,_,_  ), // #136
  O(F30F00,5B,_,_,_,_,_,_  ), // #137
  O(000F00,2C,_,_,_,_,_,_  ), // #138
  O(F20F00,2C,_,_,x,_,_,_  ), // #139
  O(F30F00,2C,_,_,x,_,_,_  ), // #140
  O(660000,99,_,_,_,_,_,_  ), // #141
  O(000000,98,_,_,_,_,_,_  ), // #142
  O(000000,27,_,_,_,_,_,_  ), // #143
  O(000000,2F,_,_,_,_,_,_  ), // #144
  O(000000,FE,1,_,x,_,_,_  ), // #145
  O(000000,F6,6,_,x,_,_,_  ), // #146
  O(660F00,5E,_,_,_,_,_,_  ), // #147
  O(000F00,5E,_,_,_,_,_,_  ), // #148
  O(F20F00,5E,_,_,_,_,_,_  ), // #149
  O(F30F00,5E,_,_,_,_,_,_  ), // #150
  O(660F3A,41,_,_,_,_,_,_  ), // #151
  O(660F3A,40,_,_,_,_,_,_  ), // #152
  O(000F00,77,_,_,_,_,_,_  ), // #153
  O(000000,C8,_,_,_,_,_,_  ), // #154
  O(660F3A,17,_,_,_,_,_,_  ), // #155
  O(660F00,79,_,_,_,_,_,_  ), // #156
  O_FPU(00,D9F0,_)          , // #157
  O_FPU(00,D9E1,_)          , // #158
  O_FPU(00,C0C0,0)          , // #159
  O_FPU(00,DEC0,_)          , // #160
  O_FPU(00,00DF,4)          , // #161
  O_FPU(00,00DF,6)          , // #162
  O_FPU(00,D9E0,_)          , // #163
  O_FPU(9B,DBE2,_)          , // #164
  O_FPU(00,DAC0,_)          , // #165
  O_FPU(00,DAD0,_)          , // #166
  O_FPU(00,DAC8,_)          , // #167
  O_FPU(00,DBC0,_)          , // #168
  O_FPU(00,DBD0,_)          , // #169
  O_FPU(00,DBC8,_)          , // #170
  O_FPU(00,DBD8,_)          , // #171
  O_FPU(00,DAD8,_)          , // #172
  O_FPU(00,D0D0,2)          , // #173
  O_FPU(00,DBF0,_)          , // #174
  O_FPU(00,DFF0,_)          , // #175
  O_FPU(00,D8D8,3)          , // #176
  O_FPU(00,DED9,_)          , // #177
  O_FPU(00,D9FF,_)          , // #178
  O_FPU(00,D9F6,_)          , // #179
  O_FPU(00,F0F8,6)          , // #180
  O_FPU(00,DEF8,_)          , // #181
  O_FPU(00,F8F0,7)          , // #182
  O_FPU(00,DEF0,_)          , // #183
  O(000F00,0E,_,_,_,_,_,_  ), // #184
  O_FPU(00,DDC0,_)          , // #185
  O_FPU(00,00DA,0)          , // #186
  O_FPU(00,00DA,2)          , // #187
  O_FPU(00,00DA,3)          , // #188
  O_FPU(00,00DA,6)          , // #189
  O_FPU(00,00DA,7)          , // #190
  O_FPU(00,00DB,0)          , // #191
  O_FPU(00,00DA,1)          , // #192
  O_FPU(00,D9F7,_)          , // #193
  O_FPU(9B,DBE3,_)          , // #194
  O_FPU(00,00DB,2)          , // #195
  O_FPU(00,00DB,3)          , // #196
  O_FPU(00,00DB,1)          , // #197
  O_FPU(00,00DA,4)          , // #198
  O_FPU(00,00DA,5)          , // #199
  O_FPU(00,00D9,0)          , // #200
  O_FPU(00,D9E8,_)          , // #201
  O_FPU(00,00D9,5)          , // #202
  O_FPU(00,00D9,4)          , // #203
  O_FPU(00,D9EA,_)          , // #204
  O_FPU(00,D9E9,_)          , // #205
  O_FPU(00,D9EC,_)          , // #206
  O_FPU(00,D9ED,_)          , // #207
  O_FPU(00,D9EB,_)          , // #208
  O_FPU(00,D9EE,_)          , // #209
  O_FPU(00,C8C8,1)          , // #210
  O_FPU(00,DEC8,_)          , // #211
  O_FPU(00,DBE2,_)          , // #212
  O_FPU(00,DBE3,_)          , // #213
  O_FPU(00,D9D0,_)          , // #214
  O_FPU(00,00DD,6)          , // #215
  O_FPU(00,00D9,7)          , // #216
  O_FPU(00,00D9,6)          , // #217
  O_FPU(00,00DD,7)          , // #218
  O_FPU(00,D9F3,_)          , // #219
  O_FPU(00,D9F8,_)          , // #220
  O_FPU(00,D9F5,_)          , // #221
  O_FPU(00,D9F2,_)          , // #222
  O_FPU(00,D9FC,_)          , // #223
  O_FPU(00,00DD,4)          , // #224
  O_FPU(9B,00DD,6)          , // #225
  O_FPU(00,D9FD,_)          , // #226
  O_FPU(00,D9FE,_)          , // #227
  O_FPU(00,D9FB,_)          , // #228
  O_FPU(00,D9FA,_)          , // #229
  O_FPU(00,00D9,2)          , // #230
  O_FPU(9B,00D9,7)          , // #231
  O_FPU(9B,00D9,6)          , // #232
  O_FPU(00,00D9,3)          , // #233
  O_FPU(9B,00DD,7)          , // #234
  O_FPU(00,E0E8,4)          , // #235
  O_FPU(00,DEE8,_)          , // #236
  O_FPU(00,E8E0,5)          , // #237
  O_FPU(00,DEE0,_)          , // #238
  O_FPU(00,D9E4,_)          , // #239
  O_FPU(00,DDE0,_)          , // #240
  O_FPU(00,DBE8,_)          , // #241
  O_FPU(00,DFE8,_)          , // #242
  O_FPU(00,DDE8,_)          , // #243
  O_FPU(00,DAE9,_)          , // #244
  O_FPU(00,00DB,_)          , // #245
  O_FPU(00,D9E5,_)          , // #246
  O_FPU(00,D9C8,_)          , // #247
  O(000F00,AE,1,_,_,_,_,_  ), // #248
  O(000F00,AE,1,_,1,_,_,_  ), // #249
  O(000F00,AE,0,_,_,_,_,_  ), // #250
  O(000F00,AE,0,_,1,_,_,_  ), // #251
  O_FPU(00,D9F4,_)          , // #252
  O_FPU(00,D9F1,_)          , // #253
  O_FPU(00,D9F9,_)          , // #254
  O(000F00,37,_,_,_,_,_,_  ), // #255
  O(660F3A,CF,_,_,_,_,_,_  ), // #256
  O(660F3A,CE,_,_,_,_,_,_  ), // #257
  O(660F38,CF,_,_,_,_,_,_  ), // #258
  O(660F00,7C,_,_,_,_,_,_  ), // #259
  O(F20F00,7C,_,_,_,_,_,_  ), // #260
  O(000000,F4,_,_,_,_,_,_  ), // #261
  O(660F00,7D,_,_,_,_,_,_  ), // #262
  O(F20F00,7D,_,_,_,_,_,_  ), // #263
  O(000000,F6,7,_,x,_,_,_  ), // #264
  O(000000,F6,5,_,x,_,_,_  ), // #265
  O(000000,EC,_,_,_,_,_,_  ), // #266
  O(000000,FE,0,_,x,_,_,_  ), // #267
  O(000000,6C,_,_,_,_,_,_  ), // #268
  O(660F3A,21,_,_,_,_,_,_  ), // #269
  O(F20F00,79,_,_,_,_,_,_  ), // #270
  O(000000,CD,_,_,_,_,_,_  ), // #271
  O(000000,CC,_,_,_,_,_,_  ), // #272
  O(000000,CE,_,_,_,_,_,_  ), // #273
  O(000F00,08,_,_,_,_,_,_  ), // #274
  O(660F38,80,_,_,_,_,_,_  ), // #275
  O(000F00,01,7,_,_,_,_,_  ), // #276
  O(000F01,DF,_,_,_,_,_,_  ), // #277
  O(660F38,82,_,_,_,_,_,_  ), // #278
  O(660F38,81,_,_,_,_,_,_  ), // #279
  O(000000,CF,_,_,_,_,_,_  ), // #280
  O(000000,CF,_,_,_,_,_,_  ), // #281
  O(000000,CF,_,_,1,_,_,_  ), // #282
  O(660000,CF,_,_,_,_,_,_  ), // #283
  O(000F00,87,_,_,_,_,_,_  ), // #284
  O(000F00,83,_,_,_,_,_,_  ), // #285
  O(000F00,82,_,_,_,_,_,_  ), // #286
  O(000F00,86,_,_,_,_,_,_  ), // #287
  O(000F00,82,_,_,_,_,_,_  ), // #288
  O(000F00,84,_,_,_,_,_,_  ), // #289
  0                         , // #290
  O(000F00,8F,_,_,_,_,_,_  ), // #291
  O(000F00,8D,_,_,_,_,_,_  ), // #292
  O(000F00,8C,_,_,_,_,_,_  ), // #293
  O(000F00,8E,_,_,_,_,_,_  ), // #294
  O(000000,FF,4,_,_,_,_,_  ), // #295
  O(000F00,86,_,_,_,_,_,_  ), // #296
  O(000F00,82,_,_,_,_,_,_  ), // #297
  O(000F00,83,_,_,_,_,_,_  ), // #298
  O(000F00,87,_,_,_,_,_,_  ), // #299
  O(000F00,83,_,_,_,_,_,_  ), // #300
  O(000F00,85,_,_,_,_,_,_  ), // #301
  O(000F00,8E,_,_,_,_,_,_  ), // #302
  O(000F00,8C,_,_,_,_,_,_  ), // #303
  O(000F00,8D,_,_,_,_,_,_  ), // #304
  O(000F00,8F,_,_,_,_,_,_  ), // #305
  O(000F00,81,_,_,_,_,_,_  ), // #306
  O(000F00,8B,_,_,_,_,_,_  ), // #307
  O(000F00,89,_,_,_,_,_,_  ), // #308
  O(000F00,85,_,_,_,_,_,_  ), // #309
  O(000F00,80,_,_,_,_,_,_  ), // #310
  O(000F00,8A,_,_,_,_,_,_  ), // #311
  O(000F00,8A,_,_,_,_,_,_  ), // #312
  O(000F00,8B,_,_,_,_,_,_  ), // #313
  O(000F00,88,_,_,_,_,_,_  ), // #314
  O(000F00,84,_,_,_,_,_,_  ), // #315
  V(660F00,4A,_,1,0,_,_,_  ), // #316
  V(660F00,4A,_,1,1,_,_,_  ), // #317
  V(000F00,4A,_,1,1,_,_,_  ), // #318
  V(000F00,4A,_,1,0,_,_,_  ), // #319
  V(660F00,41,_,1,0,_,_,_  ), // #320
  V(660F00,41,_,1,1,_,_,_  ), // #321
  V(660F00,42,_,1,0,_,_,_  ), // #322
  V(660F00,42,_,1,1,_,_,_  ), // #323
  V(000F00,42,_,1,1,_,_,_  ), // #324
  V(000F00,42,_,1,0,_,_,_  ), // #325
  V(000F00,41,_,1,1,_,_,_  ), // #326
  V(000F00,41,_,1,0,_,_,_  ), // #327
  V(660F00,90,_,0,0,_,_,_  ), // #328
  V(660F00,90,_,0,1,_,_,_  ), // #329
  V(000F00,90,_,0,1,_,_,_  ), // #330
  V(000F00,90,_,0,0,_,_,_  ), // #331
  V(660F00,44,_,0,0,_,_,_  ), // #332
  V(660F00,44,_,0,1,_,_,_  ), // #333
  V(000F00,44,_,0,1,_,_,_  ), // #334
  V(000F00,44,_,0,0,_,_,_  ), // #335
  V(660F00,45,_,1,0,_,_,_  ), // #336
  V(660F00,45,_,1,1,_,_,_  ), // #337
  V(000F00,45,_,1,1,_,_,_  ), // #338
  V(660F00,98,_,0,0,_,_,_  ), // #339
  V(660F00,98,_,0,1,_,_,_  ), // #340
  V(000F00,98,_,0,1,_,_,_  ), // #341
  V(000F00,98,_,0,0,_,_,_  ), // #342
  V(000F00,45,_,1,0,_,_,_  ), // #343
  V(660F3A,32,_,0,0,_,_,_  ), // #344
  V(660F3A,33,_,0,0,_,_,_  ), // #345
  V(660F3A,33,_,0,1,_,_,_  ), // #346
  V(660F3A,32,_,0,1,_,_,_  ), // #347
  V(660F3A,30,_,0,0,_,_,_  ), // #348
  V(660F3A,31,_,0,0,_,_,_  ), // #349
  V(660F3A,31,_,0,1,_,_,_  ), // #350
  V(660F3A,30,_,0,1,_,_,_  ), // #351
  V(660F00,99,_,0,0,_,_,_  ), // #352
  V(660F00,99,_,0,1,_,_,_  ), // #353
  V(000F00,99,_,0,1,_,_,_  ), // #354
  V(000F00,99,_,0,0,_,_,_  ), // #355
  V(660F00,4B,_,1,0,_,_,_  ), // #356
  V(000F00,4B,_,1,1,_,_,_  ), // #357
  V(000F00,4B,_,1,0,_,_,_  ), // #358
  V(660F00,46,_,1,0,_,_,_  ), // #359
  V(660F00,46,_,1,1,_,_,_  ), // #360
  V(000F00,46,_,1,1,_,_,_  ), // #361
  V(000F00,46,_,1,0,_,_,_  ), // #362
  V(660F00,47,_,1,0,_,_,_  ), // #363
  V(660F00,47,_,1,1,_,_,_  ), // #364
  V(000F00,47,_,1,1,_,_,_  ), // #365
  V(000F00,47,_,1,0,_,_,_  ), // #366
  O(000000,9F,_,_,_,_,_,_  ), // #367
  O(000F00,02,_,_,_,_,_,_  ), // #368
  O(F20F00,F0,_,_,_,_,_,_  ), // #369
  O(000F00,AE,2,_,_,_,_,_  ), // #370
  O(000000,C5,_,_,_,_,_,_  ), // #371
  O(000000,8D,_,_,x,_,_,_  ), // #372
  O(000000,C9,_,_,_,_,_,_  ), // #373
  O(000000,C4,_,_,_,_,_,_  ), // #374
  O(000F00,AE,5,_,_,_,_,_  ), // #375
  O(000F00,B4,_,_,_,_,_,_  ), // #376
  O(000F00,01,2,_,_,_,_,_  ), // #377
  O(000F00,B5,_,_,_,_,_,_  ), // #378
  O(000F00,01,3,_,_,_,_,_  ), // #379
  O(000F00,00,2,_,_,_,_,_  ), // #380
  V(XOP_M9,12,0,0,x,_,_,_  ), // #381
  O(000F00,01,6,_,_,_,_,_  ), // #382
  O(000000,AC,_,_,_,_,_,_  ), // #383
  0                         , // #384
  0                         , // #385
  0                         , // #386
  O(000F00,03,_,_,_,_,_,_  ), // #387
  O(000F00,B2,_,_,_,_,_,_  ), // #388
  O(000F00,00,3,_,_,_,_,_  ), // #389
  V(XOP_MA,12,0,0,x,_,_,_  ), // #390
  V(XOP_MA,12,1,0,x,_,_,_  ), // #391
  O(F30F00,BD,_,_,x,_,_,_  ), // #392
  O(660F00,57,_,_,_,_,_,_  ), // #393
  O(000F00,F7,_,_,_,_,_,_  ), // #394
  O(660F00,5F,_,_,_,_,_,_  ), // #395
  O(000F00,5F,_,_,_,_,_,_  ), // #396
  O(F20F00,5F,_,_,_,_,_,_  ), // #397
  O(F30F00,5F,_,_,_,_,_,_  ), // #398
  O(000F00,AE,6,_,_,_,_,_  ), // #399
  O(660F00,5D,_,_,_,_,_,_  ), // #400
  O(000F00,5D,_,_,_,_,_,_  ), // #401
  O(F20F00,5D,_,_,_,_,_,_  ), // #402
  O(F30F00,5D,_,_,_,_,_,_  ), // #403
  O(000F01,C8,_,_,_,_,_,_  ), // #404
  O(000F01,FA,_,_,_,_,_,_  ), // #405
  0                         , // #406
  O(660F00,28,_,_,_,_,_,_  ), // #407
  O(000F00,28,_,_,_,_,_,_  ), // #408
  O(000F38,F0,_,_,x,_,_,_  ), // #409
  O(000F00,6E,_,_,_,_,_,_  ), // #410
  O(F20F00,12,_,_,_,_,_,_  ), // #411
  O(660F38,F8,_,_,_,_,_,_  ), // #412
  O(000F38,F9,_,_,_,_,_,_  ), // #413
  O(F20F00,D6,_,_,_,_,_,_  ), // #414
  O(660F00,6F,_,_,_,_,_,_  ), // #415
  O(F30F00,6F,_,_,_,_,_,_  ), // #416
  O(000F00,12,_,_,_,_,_,_  ), // #417
  O(660F00,16,_,_,_,_,_,_  ), // #418
  O(000F00,16,_,_,_,_,_,_  ), // #419
  O(000F00,16,_,_,_,_,_,_  ), // #420
  O(660F00,12,_,_,_,_,_,_  ), // #421
  O(000F00,12,_,_,_,_,_,_  ), // #422
  O(660F00,50,_,_,_,_,_,_  ), // #423
  O(000F00,50,_,_,_,_,_,_  ), // #424
  0                         , // #425
  O(660F38,2A,_,_,_,_,_,_  ), // #426
  O(000F00,C3,_,_,x,_,_,_  ), // #427
  0                         , // #428
  0                         , // #429
  0                         , // #430
  0                         , // #431
  0                         , // #432
  O(000F00,6E,_,_,x,_,_,_  ), // #433
  O(F30F00,D6,_,_,_,_,_,_  ), // #434
  O(000000,A4,_,_,_,_,_,_  ), // #435
  O(F20F00,10,_,_,_,_,_,_  ), // #436
  O(F30F00,16,_,_,_,_,_,_  ), // #437
  O(F30F00,12,_,_,_,_,_,_  ), // #438
  O(F30F00,10,_,_,_,_,_,_  ), // #439
  O(000F00,BE,_,_,x,_,_,_  ), // #440
  O(000000,63,_,_,1,_,_,_  ), // #441
  O(660F00,10,_,_,_,_,_,_  ), // #442
  O(000F00,10,_,_,_,_,_,_  ), // #443
  O(000F00,B6,_,_,x,_,_,_  ), // #444
  O(660F3A,42,_,_,_,_,_,_  ), // #445
  O(000000,F6,4,_,x,_,_,_  ), // #446
  O(660F00,59,_,_,_,_,_,_  ), // #447
  O(000F00,59,_,_,_,_,_,_  ), // #448
  O(F20F00,59,_,_,_,_,_,_  ), // #449
  O(F30F00,59,_,_,_,_,_,_  ), // #450
  V(F20F38,F6,_,0,x,_,_,_  ), // #451
  O(000F01,C9,_,_,_,_,_,_  ), // #452
  O(000F01,FB,_,_,_,_,_,_  ), // #453
  O(000000,F6,3,_,x,_,_,_  ), // #454
  O(000000,90,_,_,_,_,_,_  ), // #455
  O(000000,F6,2,_,x,_,_,_  ), // #456
  O(000000,08,1,_,x,_,_,_  ), // #457
  O(660F00,56,_,_,_,_,_,_  ), // #458
  O(000F00,56,_,_,_,_,_,_  ), // #459
  O(000000,EE,_,_,_,_,_,_  ), // #460
  O(000000,6E,_,_,_,_,_,_  ), // #461
  O(000F38,1C,_,_,_,_,_,_  ), // #462
  O(000F38,1E,_,_,_,_,_,_  ), // #463
  O(000F38,1D,_,_,_,_,_,_  ), // #464
  O(000F00,6B,_,_,_,_,_,_  ), // #465
  O(000F00,63,_,_,_,_,_,_  ), // #466
  O(660F38,2B,_,_,_,_,_,_  ), // #467
  O(000F00,67,_,_,_,_,_,_  ), // #468
  O(000F00,FC,_,_,_,_,_,_  ), // #469
  O(000F00,FE,_,_,_,_,_,_  ), // #470
  O(000F00,D4,_,_,_,_,_,_  ), // #471
  O(000F00,EC,_,_,_,_,_,_  ), // #472
  O(000F00,ED,_,_,_,_,_,_  ), // #473
  O(000F00,DC,_,_,_,_,_,_  ), // #474
  O(000F00,DD,_,_,_,_,_,_  ), // #475
  O(000F00,FD,_,_,_,_,_,_  ), // #476
  O(000F3A,0F,_,_,_,_,_,_  ), // #477
  O(000F00,DB,_,_,_,_,_,_  ), // #478
  O(000F00,DF,_,_,_,_,_,_  ), // #479
  O(F30000,90,_,_,_,_,_,_  ), // #480
  O(000F00,E0,_,_,_,_,_,_  ), // #481
  O(000F0F,BF,_,_,_,_,_,_  ), // #482
  O(000F00,E3,_,_,_,_,_,_  ), // #483
  O(660F38,10,_,_,_,_,_,_  ), // #484
  O(660F3A,0E,_,_,_,_,_,_  ), // #485
  O(660F3A,44,_,_,_,_,_,_  ), // #486
  O(000F00,74,_,_,_,_,_,_  ), // #487
  O(000F00,76,_,_,_,_,_,_  ), // #488
  O(660F38,29,_,_,_,_,_,_  ), // #489
  O(000F00,75,_,_,_,_,_,_  ), // #490
  O(660F3A,61,_,_,_,_,_,_  ), // #491
  O(660F3A,60,_,_,_,_,_,_  ), // #492
  O(000F00,64,_,_,_,_,_,_  ), // #493
  O(000F00,66,_,_,_,_,_,_  ), // #494
  O(660F38,37,_,_,_,_,_,_  ), // #495
  O(000F00,65,_,_,_,_,_,_  ), // #496
  O(660F3A,63,_,_,_,_,_,_  ), // #497
  O(660F3A,62,_,_,_,_,_,_  ), // #498
  O(660F00,AE,7,_,_,_,_,_  ), // #499
  V(F20F38,F5,_,0,x,_,_,_  ), // #500
  V(F30F38,F5,_,0,x,_,_,_  ), // #501
  O(000F3A,14,_,_,_,_,_,_  ), // #502
  O(000F3A,16,_,_,_,_,_,_  ), // #503
  O(000F3A,16,_,_,1,_,_,_  ), // #504
  O(000F00,C5,_,_,_,_,_,_  ), // #505
  O(000F0F,1D,_,_,_,_,_,_  ), // #506
  O(000F0F,1C,_,_,_,_,_,_  ), // #507
  O(000F0F,AE,_,_,_,_,_,_  ), // #508
  O(000F0F,9E,_,_,_,_,_,_  ), // #509
  O(000F0F,B0,_,_,_,_,_,_  ), // #510
  O(000F0F,90,_,_,_,_,_,_  ), // #511
  O(000F0F,A0,_,_,_,_,_,_  ), // #512
  O(000F0F,A4,_,_,_,_,_,_  ), // #513
  O(000F0F,94,_,_,_,_,_,_  ), // #514
  O(000F0F,B4,_,_,_,_,_,_  ), // #515
  O(000F0F,8A,_,_,_,_,_,_  ), // #516
  O(000F0F,8E,_,_,_,_,_,_  ), // #517
  O(000F0F,96,_,_,_,_,_,_  ), // #518
  O(000F0F,A6,_,_,_,_,_,_  ), // #519
  O(000F0F,B6,_,_,_,_,_,_  ), // #520
  O(000F0F,86,_,_,_,_,_,_  ), // #521
  O(000F0F,A7,_,_,_,_,_,_  ), // #522
  O(000F0F,97,_,_,_,_,_,_  ), // #523
  O(000F0F,87,_,_,_,_,_,_  ), // #524
  O(000F0F,9A,_,_,_,_,_,_  ), // #525
  O(000F0F,AA,_,_,_,_,_,_  ), // #526
  O(000F38,02,_,_,_,_,_,_  ), // #527
  O(000F38,03,_,_,_,_,_,_  ), // #528
  O(000F38,01,_,_,_,_,_,_  ), // #529
  O(660F38,41,_,_,_,_,_,_  ), // #530
  O(000F38,06,_,_,_,_,_,_  ), // #531
  O(000F38,07,_,_,_,_,_,_  ), // #532
  O(000F38,05,_,_,_,_,_,_  ), // #533
  O(000F0F,0D,_,_,_,_,_,_  ), // #534
  O(000F0F,0C,_,_,_,_,_,_  ), // #535
  O(660F3A,20,_,_,_,_,_,_  ), // #536
  O(660F3A,22,_,_,_,_,_,_  ), // #537
  O(660F3A,22,_,_,1,_,_,_  ), // #538
  O(000F00,C4,_,_,_,_,_,_  ), // #539
  O(000F38,04,_,_,_,_,_,_  ), // #540
  O(000F00,F5,_,_,_,_,_,_  ), // #541
  O(660F38,3C,_,_,_,_,_,_  ), // #542
  O(660F38,3D,_,_,_,_,_,_  ), // #543
  O(000F00,EE,_,_,_,_,_,_  ), // #544
  O(000F00,DE,_,_,_,_,_,_  ), // #545
  O(660F38,3F,_,_,_,_,_,_  ), // #546
  O(660F38,3E,_,_,_,_,_,_  ), // #547
  O(660F38,38,_,_,_,_,_,_  ), // #548
  O(660F38,39,_,_,_,_,_,_  ), // #549
  O(000F00,EA,_,_,_,_,_,_  ), // #550
  O(000F00,DA,_,_,_,_,_,_  ), // #551
  O(660F38,3B,_,_,_,_,_,_  ), // #552
  O(660F38,3A,_,_,_,_,_,_  ), // #553
  O(000F00,D7,_,_,_,_,_,_  ), // #554
  O(660F38,21,_,_,_,_,_,_  ), // #555
  O(660F38,22,_,_,_,_,_,_  ), // #556
  O(660F38,20,_,_,_,_,_,_  ), // #557
  O(660F38,25,_,_,_,_,_,_  ), // #558
  O(660F38,23,_,_,_,_,_,_  ), // #559
  O(660F38,24,_,_,_,_,_,_  ), // #560
  O(660F38,31,_,_,_,_,_,_  ), // #561
  O(660F38,32,_,_,_,_,_,_  ), // #562
  O(660F38,30,_,_,_,_,_,_  ), // #563
  O(660F38,35,_,_,_,_,_,_  ), // #564
  O(660F38,33,_,_,_,_,_,_  ), // #565
  O(660F38,34,_,_,_,_,_,_  ), // #566
  O(660F38,28,_,_,_,_,_,_  ), // #567
  O(000F38,0B,_,_,_,_,_,_  ), // #568
  O(000F0F,B7,_,_,_,_,_,_  ), // #569
  O(000F00,E4,_,_,_,_,_,_  ), // #570
  O(000F00,E5,_,_,_,_,_,_  ), // #571
  O(660F38,40,_,_,_,_,_,_  ), // #572
  O(000F00,D5,_,_,_,_,_,_  ), // #573
  O(000F00,F4,_,_,_,_,_,_  ), // #574
  O(000000,8F,0,_,_,_,_,_  ), // #575
  O(660000,61,_,_,_,_,_,_  ), // #576
  O(000000,61,_,_,_,_,_,_  ), // #577
  O(F30F00,B8,_,_,x,_,_,_  ), // #578
  O(660000,9D,_,_,_,_,_,_  ), // #579
  O(000000,9D,_,_,_,_,_,_  ), // #580
  O(000000,9D,_,_,_,_,_,_  ), // #581
  O(000F00,EB,_,_,_,_,_,_  ), // #582
  O(000F00,0D,0,_,_,_,_,_  ), // #583
  O(000F00,18,0,_,_,_,_,_  ), // #584
  O(000F00,18,1,_,_,_,_,_  ), // #585
  O(000F00,18,2,_,_,_,_,_  ), // #586
  O(000F00,18,3,_,_,_,_,_  ), // #587
  O(000F00,0D,1,_,_,_,_,_  ), // #588
  O(000F00,0D,2,_,_,_,_,_  ), // #589
  O(000F00,F6,_,_,_,_,_,_  ), // #590
  O(000F38,00,_,_,_,_,_,_  ), // #591
  O(660F00,70,_,_,_,_,_,_  ), // #592
  O(F30F00,70,_,_,_,_,_,_  ), // #593
  O(F20F00,70,_,_,_,_,_,_  ), // #594
  O(000F00,70,_,_,_,_,_,_  ), // #595
  O(000F38,08,_,_,_,_,_,_  ), // #596
  O(000F38,0A,_,_,_,_,_,_  ), // #597
  O(000F38,09,_,_,_,_,_,_  ), // #598
  O(000F00,F2,_,_,_,_,_,_  ), // #599
  0                         , // #600
  O(000F00,F3,_,_,_,_,_,_  ), // #601
  O(000F00,F1,_,_,_,_,_,_  ), // #602
  O(000F00,E2,_,_,_,_,_,_  ), // #603
  O(000F00,E1,_,_,_,_,_,_  ), // #604
  O(000F00,D2,_,_,_,_,_,_  ), // #605
  0                         , // #606
  O(000F00,D3,_,_,_,_,_,_  ), // #607
  O(000F00,D1,_,_,_,_,_,_  ), // #608
  O(000F00,F8,_,_,_,_,_,_  ), // #609
  O(000F00,FA,_,_,_,_,_,_  ), // #610
  O(000F00,FB,_,_,_,_,_,_  ), // #611
  O(000F00,E8,_,_,_,_,_,_  ), // #612
  O(000F00,E9,_,_,_,_,_,_  ), // #613
  O(000F00,D8,_,_,_,_,_,_  ), // #614
  O(000F00,D9,_,_,_,_,_,_  ), // #615
  O(000F00,F9,_,_,_,_,_,_  ), // #616
  O(000F0F,BB,_,_,_,_,_,_  ), // #617
  O(660F38,17,_,_,_,_,_,_  ), // #618
  O(000F00,68,_,_,_,_,_,_  ), // #619
  O(000F00,6A,_,_,_,_,_,_  ), // #620
  O(660F00,6D,_,_,_,_,_,_  ), // #621
  O(000F00,69,_,_,_,_,_,_  ), // #622
  O(000F00,60,_,_,_,_,_,_  ), // #623
  O(000F00,62,_,_,_,_,_,_  ), // #624
  O(660F00,6C,_,_,_,_,_,_  ), // #625
  O(000F00,61,_,_,_,_,_,_  ), // #626
  O(000000,FF,6,_,_,_,_,_  ), // #627
  O(660000,60,_,_,_,_,_,_  ), // #628
  O(000000,60,_,_,_,_,_,_  ), // #629
  O(660000,9C,_,_,_,_,_,_  ), // #630
  O(000000,9C,_,_,_,_,_,_  ), // #631
  O(000000,9C,_,_,_,_,_,_  ), // #632
  O(000F00,EF,_,_,_,_,_,_  ), // #633
  O(000000,D0,2,_,x,_,_,_  ), // #634
  O(000F00,53,_,_,_,_,_,_  ), // #635
  O(F30F00,53,_,_,_,_,_,_  ), // #636
  O(000000,D0,3,_,x,_,_,_  ), // #637
  O(F30F00,AE,0,_,x,_,_,_  ), // #638
  O(F30F00,AE,1,_,x,_,_,_  ), // #639
  O(000F00,32,_,_,_,_,_,_  ), // #640
  O(F30F00,C7,7,_,_,_,_,_  ), // #641
  O(000F00,33,_,_,_,_,_,_  ), // #642
  O(000F00,C7,6,_,x,_,_,_  ), // #643
  O(000F00,C7,7,_,x,_,_,_  ), // #644
  O(000F00,31,_,_,_,_,_,_  ), // #645
  O(000F01,F9,_,_,_,_,_,_  ), // #646
  O(000000,C2,_,_,_,_,_,_  ), // #647
  O(000000,D0,0,_,x,_,_,_  ), // #648
  O(000000,D0,1,_,x,_,_,_  ), // #649
  V(F20F3A,F0,_,0,x,_,_,_  ), // #650
  O(660F3A,09,_,_,_,_,_,_  ), // #651
  O(660F3A,08,_,_,_,_,_,_  ), // #652
  O(660F3A,0B,_,_,_,_,_,_  ), // #653
  O(660F3A,0A,_,_,_,_,_,_  ), // #654
  O(000F00,AA,_,_,_,_,_,_  ), // #655
  O(000F00,52,_,_,_,_,_,_  ), // #656
  O(F30F00,52,_,_,_,_,_,_  ), // #657
  O(000000,9E,_,_,_,_,_,_  ), // #658
  O(000000,D0,4,_,x,_,_,_  ), // #659
  O(000000,D0,7,_,x,_,_,_  ), // #660
  V(F30F38,F7,_,0,x,_,_,_  ), // #661
  O(000000,18,3,_,x,_,_,_  ), // #662
  O(000000,AE,_,_,_,_,_,_  ), // #663
  O(000F00,97,_,_,_,_,_,_  ), // #664
  O(000F00,93,_,_,_,_,_,_  ), // #665
  O(000F00,92,_,_,_,_,_,_  ), // #666
  O(000F00,96,_,_,_,_,_,_  ), // #667
  O(000F00,92,_,_,_,_,_,_  ), // #668
  O(000F00,94,_,_,_,_,_,_  ), // #669
  O(000F00,9F,_,_,_,_,_,_  ), // #670
  O(000F00,9D,_,_,_,_,_,_  ), // #671
  O(000F00,9C,_,_,_,_,_,_  ), // #672
  O(000F00,9E,_,_,_,_,_,_  ), // #673
  O(000F00,96,_,_,_,_,_,_  ), // #674
  O(000F00,92,_,_,_,_,_,_  ), // #675
  O(000F00,93,_,_,_,_,_,_  ), // #676
  O(000F00,97,_,_,_,_,_,_  ), // #677
  O(000F00,93,_,_,_,_,_,_  ), // #678
  O(000F00,95,_,_,_,_,_,_  ), // #679
  O(000F00,9E,_,_,_,_,_,_  ), // #680
  O(000F00,9C,_,_,_,_,_,_  ), // #681
  O(000F00,9D,_,_,_,_,_,_  ), // #682
  O(000F00,9F,_,_,_,_,_,_  ), // #683
  O(000F00,91,_,_,_,_,_,_  ), // #684
  O(000F00,9B,_,_,_,_,_,_  ), // #685
  O(000F00,99,_,_,_,_,_,_  ), // #686
  O(000F00,95,_,_,_,_,_,_  ), // #687
  O(000F00,90,_,_,_,_,_,_  ), // #688
  O(000F00,9A,_,_,_,_,_,_  ), // #689
  O(000F00,9A,_,_,_,_,_,_  ), // #690
  O(000F00,9B,_,_,_,_,_,_  ), // #691
  O(000F00,98,_,_,_,_,_,_  ), // #692
  O(000F00,94,_,_,_,_,_,_  ), // #693
  O(000F00,AE,7,_,_,_,_,_  ), // #694
  O(000F00,01,0,_,_,_,_,_  ), // #695
  O(000F38,C9,_,_,_,_,_,_  ), // #696
  O(000F38,CA,_,_,_,_,_,_  ), // #697
  O(000F38,C8,_,_,_,_,_,_  ), // #698
  O(000F3A,CC,_,_,_,_,_,_  ), // #699
  O(000F38,CC,_,_,_,_,_,_  ), // #700
  O(000F38,CD,_,_,_,_,_,_  ), // #701
  O(000F38,CB,_,_,_,_,_,_  ), // #702
  O(000000,D0,4,_,x,_,_,_  ), // #703
  O(000F00,A4,_,_,x,_,_,_  ), // #704
  V(660F38,F7,_,0,x,_,_,_  ), // #705
  O(000000,D0,5,_,x,_,_,_  ), // #706
  O(000F00,AC,_,_,x,_,_,_  ), // #707
  V(F20F38,F7,_,0,x,_,_,_  ), // #708
  O(660F00,C6,_,_,_,_,_,_  ), // #709
  O(000F00,C6,_,_,_,_,_,_  ), // #710
  O(000F00,01,1,_,_,_,_,_  ), // #711
  O(000F01,DE,_,_,_,_,_,_  ), // #712
  O(000F00,00,0,_,_,_,_,_  ), // #713
  V(XOP_M9,12,1,0,x,_,_,_  ), // #714
  O(000F00,01,4,_,_,_,_,_  ), // #715
  O(660F00,51,_,_,_,_,_,_  ), // #716
  O(000F00,51,_,_,_,_,_,_  ), // #717
  O(F20F00,51,_,_,_,_,_,_  ), // #718
  O(F30F00,51,_,_,_,_,_,_  ), // #719
  O(000F01,CB,_,_,_,_,_,_  ), // #720
  O(000000,F9,_,_,_,_,_,_  ), // #721
  O(000000,FD,_,_,_,_,_,_  ), // #722
  O(000F01,DC,_,_,_,_,_,_  ), // #723
  O(000000,FB,_,_,_,_,_,_  ), // #724
  O(000F00,AE,3,_,_,_,_,_  ), // #725
  O(000000,AA,_,_,_,_,_,_  ), // #726
  O(000F00,00,1,_,_,_,_,_  ), // #727
  O(000000,28,5,_,x,_,_,_  ), // #728
  O(660F00,5C,_,_,_,_,_,_  ), // #729
  O(000F00,5C,_,_,_,_,_,_  ), // #730
  O(F20F00,5C,_,_,_,_,_,_  ), // #731
  O(F30F00,5C,_,_,_,_,_,_  ), // #732
  O(000F01,F8,_,_,_,_,_,_  ), // #733
  O(000F00,05,_,_,_,_,_,_  ), // #734
  O(000F00,34,_,_,_,_,_,_  ), // #735
  O(000F00,35,_,_,_,_,_,_  ), // #736
  O(000F00,35,_,_,_,_,_,_  ), // #737
  O(000F00,07,_,_,_,_,_,_  ), // #738
  O(000F00,07,_,_,_,_,_,_  ), // #739
  V(XOP_M9,01,7,0,x,_,_,_  ), // #740
  O(000000,84,_,_,x,_,_,_  ), // #741
  O(F30F00,BC,_,_,x,_,_,_  ), // #742
  V(XOP_M9,01,4,0,x,_,_,_  ), // #743
  O(660F00,2E,_,_,_,_,_,_  ), // #744
  O(000F00,2E,_,_,_,_,_,_  ), // #745
  O(000F00,0B,_,_,_,_,_,_  ), // #746
  O(660F00,15,_,_,_,_,_,_  ), // #747
  O(000F00,15,_,_,_,_,_,_  ), // #748
  O(660F00,14,_,_,_,_,_,_  ), // #749
  O(000F00,14,_,_,_,_,_,_  ), // #750
  V(F20F38,9A,_,2,_,0,2,T4X), // #751
  V(F20F38,9B,_,2,_,0,2,T4X), // #752
  V(F20F38,AA,_,2,_,0,2,T4X), // #753
  V(F20F38,AB,_,2,_,0,2,T4X), // #754
  V(660F00,58,_,x,I,1,4,FV ), // #755
  V(000F00,58,_,x,I,0,4,FV ), // #756
  V(F20F00,58,_,I,I,1,3,T1S), // #757
  V(F30F00,58,_,I,I,0,2,T1S), // #758
  V(660F00,D0,_,x,I,_,_,_  ), // #759
  V(F20F00,D0,_,x,I,_,_,_  ), // #760
  V(660F38,DE,_,x,I,_,4,FVM), // #761
  V(660F38,DF,_,x,I,_,4,FVM), // #762
  V(660F38,DC,_,x,I,_,4,FVM), // #763
  V(660F38,DD,_,x,I,_,4,FVM), // #764
  V(660F38,DB,_,0,I,_,_,_  ), // #765
  V(660F3A,DF,_,0,I,_,_,_  ), // #766
  V(660F3A,03,_,x,_,0,4,FV ), // #767
  V(660F3A,03,_,x,_,1,4,FV ), // #768
  V(660F00,55,_,x,I,1,4,FV ), // #769
  V(000F00,55,_,x,I,0,4,FV ), // #770
  V(660F00,54,_,x,I,1,4,FV ), // #771
  V(000F00,54,_,x,I,0,4,FV ), // #772
  V(660F38,66,_,x,_,0,4,FVM), // #773
  V(660F38,64,_,x,_,0,4,FV ), // #774
  V(660F38,65,_,x,_,1,4,FV ), // #775
  V(660F38,65,_,x,_,0,4,FV ), // #776
  V(660F38,64,_,x,_,1,4,FV ), // #777
  V(660F38,66,_,x,_,1,4,FVM), // #778
  V(660F3A,0D,_,x,I,_,_,_  ), // #779
  V(660F3A,0C,_,x,I,_,_,_  ), // #780
  V(660F3A,4B,_,x,0,_,_,_  ), // #781
  V(660F3A,4A,_,x,0,_,_,_  ), // #782
  V(660F38,1A,_,1,0,_,_,_  ), // #783
  V(660F38,19,_,x,_,0,3,T2 ), // #784
  V(660F38,1A,_,x,_,0,4,T4 ), // #785
  V(660F38,1B,_,2,_,0,5,T8 ), // #786
  V(660F38,1A,_,x,_,1,4,T2 ), // #787
  V(660F38,1B,_,2,_,1,5,T4 ), // #788
  V(660F38,5A,_,1,0,_,_,_  ), // #789
  V(660F38,59,_,x,_,0,3,T2 ), // #790
  V(660F38,5A,_,x,_,0,4,T4 ), // #791
  V(660F38,5B,_,2,_,0,5,T8 ), // #792
  V(660F38,5A,_,x,_,1,4,T2 ), // #793
  V(660F38,5B,_,2,_,1,5,T4 ), // #794
  V(660F38,19,_,x,0,1,3,T1S), // #795
  V(660F38,18,_,x,0,0,2,T1S), // #796
  V(660F00,C2,_,x,I,1,4,FV ), // #797
  V(000F00,C2,_,x,I,0,4,FV ), // #798
  V(F20F00,C2,_,I,I,1,3,T1S), // #799
  V(F30F00,C2,_,I,I,0,2,T1S), // #800
  V(660F00,2F,_,I,I,1,3,T1S), // #801
  V(000F00,2F,_,I,I,0,2,T1S), // #802
  V(660F38,8A,_,x,_,1,3,T1S), // #803
  V(660F38,8A,_,x,_,0,2,T1S), // #804
  V(F30F00,E6,_,x,I,0,3,HV ), // #805
  V(000F00,5B,_,x,I,0,4,FV ), // #806
  V(F20F38,72,_,_,_,0,_,_  ), // #807
  V(F30F38,72,_,_,_,0,_,_  ), // #808
  V(F20F00,E6,_,x,I,1,4,FV ), // #809
  V(660F00,5A,_,x,I,1,4,FV ), // #810
  V(660F00,7B,_,x,_,1,4,FV ), // #811
  V(000F00,79,_,x,_,1,4,FV ), // #812
  V(660F00,79,_,x,_,1,4,FV ), // #813
  V(660F38,13,_,x,0,0,3,HVM), // #814
  V(660F00,5B,_,x,I,0,4,FV ), // #815
  V(000F00,5A,_,x,I,0,4,HV ), // #816
  V(660F3A,1D,_,x,0,0,3,HVM), // #817
  V(660F00,7B,_,x,_,0,3,HV ), // #818
  V(000F00,79,_,x,_,0,4,FV ), // #819
  V(660F00,79,_,x,_,0,3,HV ), // #820
  V(F30F00,E6,_,x,_,1,4,FV ), // #821
  V(000F00,5B,_,x,_,1,4,FV ), // #822
  V(F20F00,2D,_,I,x,x,3,T1F), // #823
  V(F20F00,5A,_,I,I,1,3,T1S), // #824
  V(F20F00,79,_,I,_,x,3,T1F), // #825
  V(F20F00,2A,_,I,x,x,2,T1W), // #826
  V(F30F00,2A,_,I,x,x,2,T1W), // #827
  V(F30F00,5A,_,I,I,0,2,T1S), // #828
  V(F30F00,2D,_,I,x,x,2,T1F), // #829
  V(F30F00,79,_,I,_,x,2,T1F), // #830
  V(660F00,E6,_,x,I,1,4,FV ), // #831
  V(660F00,7A,_,x,_,1,4,FV ), // #832
  V(000F00,78,_,x,_,1,4,FV ), // #833
  V(660F00,78,_,x,_,1,4,FV ), // #834
  V(F30F00,5B,_,x,I,0,4,FV ), // #835
  V(660F00,7A,_,x,_,0,3,HV ), // #836
  V(000F00,78,_,x,_,0,4,FV ), // #837
  V(660F00,78,_,x,_,0,3,HV ), // #838
  V(F20F00,2C,_,I,x,x,3,T1F), // #839
  V(F20F00,78,_,I,_,x,3,T1F), // #840
  V(F30F00,2C,_,I,x,x,2,T1F), // #841
  V(F30F00,78,_,I,_,x,2,T1F), // #842
  V(F30F00,7A,_,x,_,0,3,HV ), // #843
  V(F20F00,7A,_,x,_,0,4,FV ), // #844
  V(F30F00,7A,_,x,_,1,4,FV ), // #845
  V(F20F00,7A,_,x,_,1,4,FV ), // #846
  V(F20F00,7B,_,I,_,x,2,T1W), // #847
  V(F30F00,7B,_,I,_,x,2,T1W), // #848
  V(660F3A,42,_,x,_,0,4,FVM), // #849
  V(660F00,5E,_,x,I,1,4,FV ), // #850
  V(000F00,5E,_,x,I,0,4,FV ), // #851
  V(F20F00,5E,_,I,I,1,3,T1S), // #852
  V(F30F00,5E,_,I,I,0,2,T1S), // #853
  V(F30F38,52,_,_,_,0,_,_  ), // #854
  V(660F3A,41,_,x,I,_,_,_  ), // #855
  V(660F3A,40,_,x,I,_,_,_  ), // #856
  O(000F00,00,4,_,_,_,_,_  ), // #857
  O(000F00,00,5,_,_,_,_,_  ), // #858
  V(660F38,C8,_,2,_,1,4,FV ), // #859
  V(660F38,C8,_,2,_,0,4,FV ), // #860
  V(660F38,88,_,x,_,1,3,T1S), // #861
  V(660F38,88,_,x,_,0,2,T1S), // #862
  V(660F3A,19,_,1,0,_,_,_  ), // #863
  V(660F3A,19,_,x,_,0,4,T4 ), // #864
  V(660F3A,1B,_,2,_,0,5,T8 ), // #865
  V(660F3A,19,_,x,_,1,4,T2 ), // #866
  V(660F3A,1B,_,2,_,1,5,T4 ), // #867
  V(660F3A,39,_,1,0,_,_,_  ), // #868
  V(660F3A,39,_,x,_,0,4,T4 ), // #869
  V(660F3A,3B,_,2,_,0,5,T8 ), // #870
  V(660F3A,39,_,x,_,1,4,T2 ), // #871
  V(660F3A,3B,_,2,_,1,5,T4 ), // #872
  V(660F3A,17,_,0,I,I,2,T1S), // #873
  V(660F3A,54,_,x,_,1,4,FV ), // #874
  V(660F3A,54,_,x,_,0,4,FV ), // #875
  V(660F3A,55,_,I,_,1,3,T1S), // #876
  V(660F3A,55,_,I,_,0,2,T1S), // #877
  V(660F38,98,_,x,1,1,4,FV ), // #878
  V(660F38,98,_,x,0,0,4,FV ), // #879
  V(660F38,99,_,I,1,1,3,T1S), // #880
  V(660F38,99,_,I,0,0,2,T1S), // #881
  V(660F38,A8,_,x,1,1,4,FV ), // #882
  V(660F38,A8,_,x,0,0,4,FV ), // #883
  V(660F38,A9,_,I,1,1,3,T1S), // #884
  V(660F38,A9,_,I,0,0,2,T1S), // #885
  V(660F38,B8,_,x,1,1,4,FV ), // #886
  V(660F38,B8,_,x,0,0,4,FV ), // #887
  V(660F38,B9,_,I,1,1,3,T1S), // #888
  V(660F38,B9,_,I,0,0,2,T1S), // #889
  V(660F3A,69,_,x,x,_,_,_  ), // #890
  V(660F3A,68,_,x,x,_,_,_  ), // #891
  V(660F3A,6B,_,0,x,_,_,_  ), // #892
  V(660F3A,6A,_,0,x,_,_,_  ), // #893
  V(660F38,96,_,x,1,1,4,FV ), // #894
  V(660F38,96,_,x,0,0,4,FV ), // #895
  V(660F38,A6,_,x,1,1,4,FV ), // #896
  V(660F38,A6,_,x,0,0,4,FV ), // #897
  V(660F38,B6,_,x,1,1,4,FV ), // #898
  V(660F38,B6,_,x,0,0,4,FV ), // #899
  V(660F3A,5D,_,x,x,_,_,_  ), // #900
  V(660F3A,5C,_,x,x,_,_,_  ), // #901
  V(660F38,9A,_,x,1,1,4,FV ), // #902
  V(660F38,9A,_,x,0,0,4,FV ), // #903
  V(660F38,9B,_,I,1,1,3,T1S), // #904
  V(660F38,9B,_,I,0,0,2,T1S), // #905
  V(660F38,AA,_,x,1,1,4,FV ), // #906
  V(660F38,AA,_,x,0,0,4,FV ), // #907
  V(660F38,AB,_,I,1,1,3,T1S), // #908
  V(660F38,AB,_,I,0,0,2,T1S), // #909
  V(660F38,BA,_,x,1,1,4,FV ), // #910
  V(660F38,BA,_,x,0,0,4,FV ), // #911
  V(660F38,BB,_,I,1,1,3,T1S), // #912
  V(660F38,BB,_,I,0,0,2,T1S), // #913
  V(660F38,97,_,x,1,1,4,FV ), // #914
  V(660F38,97,_,x,0,0,4,FV ), // #915
  V(660F38,A7,_,x,1,1,4,FV ), // #916
  V(660F38,A7,_,x,0,0,4,FV ), // #917
  V(660F38,B7,_,x,1,1,4,FV ), // #918
  V(660F38,B7,_,x,0,0,4,FV ), // #919
  V(660F3A,5F,_,x,x,_,_,_  ), // #920
  V(660F3A,5E,_,x,x,_,_,_  ), // #921
  V(660F3A,6D,_,x,x,_,_,_  ), // #922
  V(660F3A,6C,_,x,x,_,_,_  ), // #923
  V(660F3A,6F,_,0,x,_,_,_  ), // #924
  V(660F3A,6E,_,0,x,_,_,_  ), // #925
  V(660F38,9C,_,x,1,1,4,FV ), // #926
  V(660F38,9C,_,x,0,0,4,FV ), // #927
  V(660F38,9D,_,I,1,1,3,T1S), // #928
  V(660F38,9D,_,I,0,0,2,T1S), // #929
  V(660F38,AC,_,x,1,1,4,FV ), // #930
  V(660F38,AC,_,x,0,0,4,FV ), // #931
  V(660F38,AD,_,I,1,1,3,T1S), // #932
  V(660F38,AD,_,I,0,0,2,T1S), // #933
  V(660F38,BC,_,x,1,1,4,FV ), // #934
  V(660F38,BC,_,x,0,0,4,FV ), // #935
  V(660F38,BC,_,I,1,1,3,T1S), // #936
  V(660F38,BC,_,I,0,0,2,T1S), // #937
  V(660F3A,79,_,x,x,_,_,_  ), // #938
  V(660F3A,78,_,x,x,_,_,_  ), // #939
  V(660F3A,7B,_,0,x,_,_,_  ), // #940
  V(660F3A,7A,_,0,x,_,_,_  ), // #941
  V(660F38,9E,_,x,1,1,4,FV ), // #942
  V(660F38,9E,_,x,0,0,4,FV ), // #943
  V(660F38,9F,_,I,1,1,3,T1S), // #944
  V(660F38,9F,_,I,0,0,2,T1S), // #945
  V(660F38,AE,_,x,1,1,4,FV ), // #946
  V(660F38,AE,_,x,0,0,4,FV ), // #947
  V(660F38,AF,_,I,1,1,3,T1S), // #948
  V(660F38,AF,_,I,0,0,2,T1S), // #949
  V(660F38,BE,_,x,1,1,4,FV ), // #950
  V(660F38,BE,_,x,0,0,4,FV ), // #951
  V(660F38,BF,_,I,1,1,3,T1S), // #952
  V(660F38,BF,_,I,0,0,2,T1S), // #953
  V(660F3A,7D,_,x,x,_,_,_  ), // #954
  V(660F3A,7C,_,x,x,_,_,_  ), // #955
  V(660F3A,7F,_,0,x,_,_,_  ), // #956
  V(660F3A,7E,_,0,x,_,_,_  ), // #957
  V(660F3A,66,_,x,_,1,4,FV ), // #958
  V(660F3A,66,_,x,_,0,4,FV ), // #959
  V(660F3A,67,_,I,_,1,3,T1S), // #960
  V(660F3A,67,_,I,_,0,2,T1S), // #961
  V(XOP_M9,81,_,x,0,_,_,_  ), // #962
  V(XOP_M9,80,_,x,0,_,_,_  ), // #963
  V(XOP_M9,83,_,0,0,_,_,_  ), // #964
  V(XOP_M9,82,_,0,0,_,_,_  ), // #965
  V(660F38,92,_,x,1,_,_,_  ), // #966
  V(660F38,92,_,x,0,_,_,_  ), // #967
  V(660F38,C6,1,2,_,1,3,T1S), // #968
  V(660F38,C6,1,2,_,0,2,T1S), // #969
  V(660F38,C7,1,2,_,1,3,T1S), // #970
  V(660F38,C7,1,2,_,0,2,T1S), // #971
  V(660F38,C6,2,2,_,1,3,T1S), // #972
  V(660F38,C6,2,2,_,0,2,T1S), // #973
  V(660F38,C7,2,2,_,1,3,T1S), // #974
  V(660F38,C7,2,2,_,0,2,T1S), // #975
  V(660F38,93,_,x,1,_,_,_  ), // #976
  V(660F38,93,_,x,0,_,_,_  ), // #977
  V(660F38,42,_,x,_,1,4,FV ), // #978
  V(660F38,42,_,x,_,0,4,FV ), // #979
  V(660F38,43,_,I,_,1,3,T1S), // #980
  V(660F38,43,_,I,_,0,2,T1S), // #981
  V(660F3A,26,_,x,_,1,4,FV ), // #982
  V(660F3A,26,_,x,_,0,4,FV ), // #983
  V(660F3A,27,_,I,_,1,3,T1S), // #984
  V(660F3A,27,_,I,_,0,2,T1S), // #985
  V(660F3A,CF,_,x,1,1,_,FV ), // #986
  V(660F3A,CE,_,x,1,1,_,FV ), // #987
  V(660F38,CF,_,x,0,0,_,FV ), // #988
  V(660F00,7C,_,x,I,_,_,_  ), // #989
  V(F20F00,7C,_,x,I,_,_,_  ), // #990
  V(660F00,7D,_,x,I,_,_,_  ), // #991
  V(F20F00,7D,_,x,I,_,_,_  ), // #992
  V(660F3A,18,_,1,0,_,_,_  ), // #993
  V(660F3A,18,_,x,_,0,4,T4 ), // #994
  V(660F3A,1A,_,2,_,0,5,T8 ), // #995
  V(660F3A,18,_,x,_,1,4,T2 ), // #996
  V(660F3A,1A,_,2,_,1,5,T4 ), // #997
  V(660F3A,38,_,1,0,_,_,_  ), // #998
  V(660F3A,38,_,x,_,0,4,T4 ), // #999
  V(660F3A,3A,_,2,_,0,5,T8 ), // #1000
  V(660F3A,38,_,x,_,1,4,T2 ), // #1001
  V(660F3A,3A,_,2,_,1,5,T4 ), // #1002
  V(660F3A,21,_,0,I,0,2,T1S), // #1003
  V(F20F00,F0,_,x,I,_,_,_  ), // #1004
  V(000F00,AE,2,0,I,_,_,_  ), // #1005
  V(660F00,F7,_,0,I,_,_,_  ), // #1006
  V(660F38,2D,_,x,0,_,_,_  ), // #1007
  V(660F38,2C,_,x,0,_,_,_  ), // #1008
  V(660F00,5F,_,x,I,1,4,FV ), // #1009
  V(000F00,5F,_,x,I,0,4,FV ), // #1010
  V(F20F00,5F,_,I,I,1,3,T1S), // #1011
  V(F30F00,5F,_,I,I,0,2,T1S), // #1012
  O(000F01,C1,_,_,_,_,_,_  ), // #1013
  O(660F00,C7,6,_,_,_,_,_  ), // #1014
  O(000F01,D4,_,_,_,_,_,_  ), // #1015
  V(660F00,5D,_,x,I,1,4,FV ), // #1016
  V(000F00,5D,_,x,I,0,4,FV ), // #1017
  V(F20F00,5D,_,I,I,1,3,T1S), // #1018
  V(F30F00,5D,_,I,I,0,2,T1S), // #1019
  O(000F01,C2,_,_,_,_,_,_  ), // #1020
  O(000F01,DA,_,_,_,_,_,_  ), // #1021
  O(000F01,D9,_,_,_,_,_,_  ), // #1022
  V(660F00,28,_,x,I,1,4,FVM), // #1023
  V(000F00,28,_,x,I,0,4,FVM), // #1024
  V(660F00,6E,_,0,0,0,2,T1S), // #1025
  V(F20F00,12,_,x,I,1,3,DUP), // #1026
  V(660F00,6F,_,x,I,_,_,_  ), // #1027
  V(660F00,6F,_,x,_,0,4,FVM), // #1028
  V(660F00,6F,_,x,_,1,4,FVM), // #1029
  V(F30F00,6F,_,x,I,_,_,_  ), // #1030
  V(F20F00,6F,_,x,_,1,4,FVM), // #1031
  V(F30F00,6F,_,x,_,0,4,FVM), // #1032
  V(F30F00,6F,_,x,_,1,4,FVM), // #1033
  V(F20F00,6F,_,x,_,0,4,FVM), // #1034
  V(000F00,12,_,0,I,0,_,_  ), // #1035
  V(660F00,16,_,0,I,1,3,T1S), // #1036
  V(000F00,16,_,0,I,0,3,T2 ), // #1037
  V(000F00,16,_,0,I,0,_,_  ), // #1038
  V(660F00,12,_,0,I,1,3,T1S), // #1039
  V(000F00,12,_,0,I,0,3,T2 ), // #1040
  V(660F00,50,_,x,I,_,_,_  ), // #1041
  V(000F00,50,_,x,I,_,_,_  ), // #1042
  V(660F00,E7,_,x,I,0,4,FVM), // #1043
  V(660F38,2A,_,x,I,0,4,FVM), // #1044
  V(660F00,2B,_,x,I,1,4,FVM), // #1045
  V(000F00,2B,_,x,I,0,4,FVM), // #1046
  V(660F00,6E,_,0,I,1,3,T1S), // #1047
  V(F20F00,10,_,I,I,1,3,T1S), // #1048
  V(F30F00,16,_,x,I,0,4,FVM), // #1049
  V(F30F00,12,_,x,I,0,4,FVM), // #1050
  V(F30F00,10,_,I,I,0,2,T1S), // #1051
  V(660F00,10,_,x,I,1,4,FVM), // #1052
  V(000F00,10,_,x,I,0,4,FVM), // #1053
  V(660F3A,42,_,x,I,_,_,_  ), // #1054
  O(000F00,C7,6,_,_,_,_,_  ), // #1055
  O(000F00,C7,7,_,_,_,_,_  ), // #1056
  O(000F00,78,_,_,_,_,_,_  ), // #1057
  O(000F01,C3,_,_,_,_,_,_  ), // #1058
  O(000F01,D8,_,_,_,_,_,_  ), // #1059
  O(000F01,DB,_,_,_,_,_,_  ), // #1060
  V(660F00,59,_,x,I,1,4,FV ), // #1061
  V(000F00,59,_,x,I,0,4,FV ), // #1062
  V(F20F00,59,_,I,I,1,3,T1S), // #1063
  V(F30F00,59,_,I,I,0,2,T1S), // #1064
  O(000F00,79,_,_,_,_,_,_  ), // #1065
  O(F30F00,C7,6,_,_,_,_,_  ), // #1066
  V(660F00,56,_,x,I,1,4,FV ), // #1067
  V(000F00,56,_,x,I,0,4,FV ), // #1068
  V(F20F38,52,_,2,_,0,2,T4X), // #1069
  V(F20F38,53,_,2,_,0,2,T4X), // #1070
  V(660F38,1C,_,x,I,_,4,FVM), // #1071
  V(660F38,1E,_,x,I,0,4,FV ), // #1072
  V(660F38,1F,_,x,_,1,4,FV ), // #1073
  V(660F38,1D,_,x,I,_,4,FVM), // #1074
  V(660F00,6B,_,x,I,0,4,FV ), // #1075
  V(660F00,63,_,x,I,I,4,FVM), // #1076
  V(660F38,2B,_,x,I,0,4,FV ), // #1077
  V(660F00,67,_,x,I,I,4,FVM), // #1078
  V(660F00,FC,_,x,I,I,4,FVM), // #1079
  V(660F00,FE,_,x,I,0,4,FV ), // #1080
  V(660F00,D4,_,x,I,1,4,FV ), // #1081
  V(660F00,EC,_,x,I,I,4,FVM), // #1082
  V(660F00,ED,_,x,I,I,4,FVM), // #1083
  V(660F00,DC,_,x,I,I,4,FVM), // #1084
  V(660F00,DD,_,x,I,I,4,FVM), // #1085
  V(660F00,FD,_,x,I,I,4,FVM), // #1086
  V(660F3A,0F,_,x,I,I,4,FVM), // #1087
  V(660F00,DB,_,x,I,_,_,_  ), // #1088
  V(660F00,DB,_,x,_,0,4,FV ), // #1089
  V(660F00,DF,_,x,I,_,_,_  ), // #1090
  V(660F00,DF,_,x,_,0,4,FV ), // #1091
  V(660F00,DF,_,x,_,1,4,FV ), // #1092
  V(660F00,DB,_,x,_,1,4,FV ), // #1093
  V(660F00,E0,_,x,I,I,4,FVM), // #1094
  V(660F00,E3,_,x,I,I,4,FVM), // #1095
  V(660F3A,02,_,x,0,_,_,_  ), // #1096
  V(660F3A,4C,_,x,0,_,_,_  ), // #1097
  V(660F3A,0E,_,x,I,_,_,_  ), // #1098
  V(660F38,78,_,x,0,0,0,T1S), // #1099
  V(660F38,58,_,x,0,0,2,T1S), // #1100
  V(F30F38,3A,_,x,_,0,_,_  ), // #1101
  V(F30F38,2A,_,x,_,1,_,_  ), // #1102
  V(660F38,59,_,x,0,1,3,T1S), // #1103
  V(660F38,79,_,x,0,0,1,T1S), // #1104
  V(660F3A,44,_,x,I,_,4,FVM), // #1105
  V(XOP_M8,A2,_,x,x,_,_,_  ), // #1106
  V(660F3A,3F,_,x,_,0,4,FVM), // #1107
  V(660F3A,1F,_,x,_,0,4,FV ), // #1108
  V(660F00,74,_,x,I,I,4,FV ), // #1109
  V(660F00,76,_,x,I,0,4,FVM), // #1110
  V(660F38,29,_,x,I,1,4,FVM), // #1111
  V(660F00,75,_,x,I,I,4,FV ), // #1112
  V(660F3A,61,_,0,I,_,_,_  ), // #1113
  V(660F3A,60,_,0,I,_,_,_  ), // #1114
  V(660F00,64,_,x,I,I,4,FV ), // #1115
  V(660F00,66,_,x,I,0,4,FVM), // #1116
  V(660F38,37,_,x,I,1,4,FVM), // #1117
  V(660F00,65,_,x,I,I,4,FV ), // #1118
  V(660F3A,63,_,0,I,_,_,_  ), // #1119
  V(660F3A,62,_,0,I,_,_,_  ), // #1120
  V(660F3A,1F,_,x,_,1,4,FV ), // #1121
  V(660F3A,3E,_,x,_,0,4,FVM), // #1122
  V(660F3A,1E,_,x,_,0,4,FV ), // #1123
  V(660F3A,1E,_,x,_,1,4,FV ), // #1124
  V(660F3A,3E,_,x,_,1,4,FVM), // #1125
  V(660F3A,3F,_,x,_,1,4,FVM), // #1126
  V(XOP_M8,CC,_,0,0,_,_,_  ), // #1127
  V(XOP_M8,CE,_,0,0,_,_,_  ), // #1128
  V(660F38,63,_,x,_,0,0,T1S), // #1129
  V(660F38,8B,_,x,_,0,2,T1S), // #1130
  V(660F38,8B,_,x,_,1,3,T1S), // #1131
  V(660F38,63,_,x,_,1,1,T1S), // #1132
  V(XOP_M8,CF,_,0,0,_,_,_  ), // #1133
  V(XOP_M8,EC,_,0,0,_,_,_  ), // #1134
  V(XOP_M8,EE,_,0,0,_,_,_  ), // #1135
  V(XOP_M8,EF,_,0,0,_,_,_  ), // #1136
  V(XOP_M8,ED,_,0,0,_,_,_  ), // #1137
  V(XOP_M8,CD,_,0,0,_,_,_  ), // #1138
  V(660F38,C4,_,x,_,0,4,FV ), // #1139
  V(660F38,C4,_,x,_,1,4,FV ), // #1140
  V(660F38,50,_,x,_,0,_,FV ), // #1141
  V(660F38,51,_,x,_,0,_,FV ), // #1142
  V(660F38,52,_,x,_,0,_,FV ), // #1143
  V(660F38,53,_,x,_,0,_,FV ), // #1144
  V(660F3A,06,_,1,0,_,_,_  ), // #1145
  V(660F3A,46,_,1,0,_,_,_  ), // #1146
  V(660F38,8D,_,x,_,0,4,FVM), // #1147
  V(660F38,36,_,x,0,0,4,FV ), // #1148
  V(660F38,75,_,x,_,0,4,FVM), // #1149
  V(660F38,76,_,x,_,0,4,FV ), // #1150
  V(660F38,77,_,x,_,1,4,FV ), // #1151
  V(660F38,77,_,x,_,0,4,FV ), // #1152
  V(660F38,76,_,x,_,1,4,FV ), // #1153
  V(660F38,75,_,x,_,1,4,FVM), // #1154
  V(660F3A,49,_,x,x,_,_,_  ), // #1155
  V(660F3A,48,_,x,x,_,_,_  ), // #1156
  V(660F38,0D,_,x,0,1,4,FV ), // #1157
  V(660F38,0C,_,x,0,0,4,FV ), // #1158
  V(660F3A,01,_,1,1,_,_,_  ), // #1159
  V(660F38,16,_,1,0,_,_,_  ), // #1160
  V(660F38,36,_,x,_,1,4,FV ), // #1161
  V(660F38,7D,_,x,_,0,4,FVM), // #1162
  V(660F38,7E,_,x,_,0,4,FV ), // #1163
  V(660F38,7F,_,x,_,1,4,FV ), // #1164
  V(660F38,7F,_,x,_,0,4,FV ), // #1165
  V(660F38,7E,_,x,_,1,4,FV ), // #1166
  V(660F38,7D,_,x,_,1,4,FVM), // #1167
  V(660F38,8D,_,x,_,1,4,FVM), // #1168
  V(660F38,62,_,x,_,0,0,T1S), // #1169
  V(660F38,89,_,x,_,0,2,T1S), // #1170
  V(660F38,89,_,x,_,1,3,T1S), // #1171
  V(660F38,62,_,x,_,1,1,T1S), // #1172
  V(660F3A,14,_,0,0,I,0,T1S), // #1173
  V(660F3A,16,_,0,0,0,2,T1S), // #1174
  V(660F3A,16,_,0,1,1,3,T1S), // #1175
  V(660F3A,15,_,0,0,I,1,T1S), // #1176
  V(660F38,90,_,x,0,_,_,_  ), // #1177
  V(660F38,90,_,x,1,_,_,_  ), // #1178
  V(660F38,91,_,x,0,_,_,_  ), // #1179
  V(660F38,91,_,x,1,_,_,_  ), // #1180
  V(XOP_M9,C2,_,0,0,_,_,_  ), // #1181
  V(XOP_M9,C3,_,0,0,_,_,_  ), // #1182
  V(XOP_M9,C1,_,0,0,_,_,_  ), // #1183
  V(660F38,02,_,x,I,_,_,_  ), // #1184
  V(XOP_M9,CB,_,0,0,_,_,_  ), // #1185
  V(660F38,03,_,x,I,_,_,_  ), // #1186
  V(XOP_M9,D2,_,0,0,_,_,_  ), // #1187
  V(XOP_M9,D3,_,0,0,_,_,_  ), // #1188
  V(XOP_M9,D1,_,0,0,_,_,_  ), // #1189
  V(XOP_M9,DB,_,0,0,_,_,_  ), // #1190
  V(XOP_M9,D6,_,0,0,_,_,_  ), // #1191
  V(XOP_M9,D7,_,0,0,_,_,_  ), // #1192
  V(660F38,01,_,x,I,_,_,_  ), // #1193
  V(XOP_M9,C6,_,0,0,_,_,_  ), // #1194
  V(XOP_M9,C7,_,0,0,_,_,_  ), // #1195
  V(660F38,41,_,0,I,_,_,_  ), // #1196
  V(XOP_M9,E1,_,0,0,_,_,_  ), // #1197
  V(660F38,06,_,x,I,_,_,_  ), // #1198
  V(XOP_M9,E3,_,0,0,_,_,_  ), // #1199
  V(660F38,07,_,x,I,_,_,_  ), // #1200
  V(660F38,05,_,x,I,_,_,_  ), // #1201
  V(XOP_M9,E2,_,0,0,_,_,_  ), // #1202
  V(660F3A,20,_,0,0,I,0,T1S), // #1203
  V(660F3A,22,_,0,0,0,2,T1S), // #1204
  V(660F3A,22,_,0,1,1,3,T1S), // #1205
  V(660F00,C4,_,0,0,I,1,T1S), // #1206
  V(660F38,44,_,x,_,0,4,FV ), // #1207
  V(660F38,44,_,x,_,1,4,FV ), // #1208
  V(XOP_M8,9E,_,0,0,_,_,_  ), // #1209
  V(XOP_M8,9F,_,0,0,_,_,_  ), // #1210
  V(XOP_M8,97,_,0,0,_,_,_  ), // #1211
  V(XOP_M8,8E,_,0,0,_,_,_  ), // #1212
  V(XOP_M8,8F,_,0,0,_,_,_  ), // #1213
  V(XOP_M8,87,_,0,0,_,_,_  ), // #1214
  V(XOP_M8,86,_,0,0,_,_,_  ), // #1215
  V(XOP_M8,85,_,0,0,_,_,_  ), // #1216
  V(XOP_M8,96,_,0,0,_,_,_  ), // #1217
  V(XOP_M8,95,_,0,0,_,_,_  ), // #1218
  V(XOP_M8,A6,_,0,0,_,_,_  ), // #1219
  V(XOP_M8,B6,_,0,0,_,_,_  ), // #1220
  V(660F38,B5,_,x,_,1,4,FV ), // #1221
  V(660F38,B4,_,x,_,1,4,FV ), // #1222
  V(660F38,04,_,x,I,I,4,FVM), // #1223
  V(660F00,F5,_,x,I,I,4,FVM), // #1224
  V(660F38,8C,_,x,0,_,_,_  ), // #1225
  V(660F38,8C,_,x,1,_,_,_  ), // #1226
  V(660F38,3C,_,x,I,I,4,FVM), // #1227
  V(660F38,3D,_,x,I,0,4,FV ), // #1228
  V(660F38,3D,_,x,_,1,4,FV ), // #1229
  V(660F00,EE,_,x,I,I,4,FVM), // #1230
  V(660F00,DE,_,x,I,I,4,FVM), // #1231
  V(660F38,3F,_,x,I,0,4,FV ), // #1232
  V(660F38,3F,_,x,_,1,4,FV ), // #1233
  V(660F38,3E,_,x,I,I,4,FVM), // #1234
  V(660F38,38,_,x,I,I,4,FVM), // #1235
  V(660F38,39,_,x,I,0,4,FV ), // #1236
  V(660F38,39,_,x,_,1,4,FV ), // #1237
  V(660F00,EA,_,x,I,I,4,FVM), // #1238
  V(660F00,DA,_,x,I,_,4,FVM), // #1239
  V(660F38,3B,_,x,I,0,4,FV ), // #1240
  V(660F38,3B,_,x,_,1,4,FV ), // #1241
  V(660F38,3A,_,x,I,_,4,FVM), // #1242
  V(F30F38,29,_,x,_,0,_,_  ), // #1243
  V(F30F38,39,_,x,_,0,_,_  ), // #1244
  V(F30F38,31,_,x,_,0,2,QVM), // #1245
  V(F30F38,33,_,x,_,0,3,HVM), // #1246
  V(F30F38,28,_,x,_,0,_,_  ), // #1247
  V(F30F38,38,_,x,_,0,_,_  ), // #1248
  V(F30F38,38,_,x,_,1,_,_  ), // #1249
  V(F30F38,28,_,x,_,1,_,_  ), // #1250
  V(660F00,D7,_,x,I,_,_,_  ), // #1251
  V(F30F38,39,_,x,_,1,_,_  ), // #1252
  V(F30F38,32,_,x,_,0,1,OVM), // #1253
  V(F30F38,35,_,x,_,0,3,HVM), // #1254
  V(F30F38,34,_,x,_,0,2,QVM), // #1255
  V(F30F38,21,_,x,_,0,2,QVM), // #1256
  V(F30F38,23,_,x,_,0,3,HVM), // #1257
  V(F30F38,22,_,x,_,0,1,OVM), // #1258
  V(F30F38,25,_,x,_,0,3,HVM), // #1259
  V(F30F38,24,_,x,_,0,2,QVM), // #1260
  V(F30F38,20,_,x,_,0,3,HVM), // #1261
  V(660F38,21,_,x,I,I,2,QVM), // #1262
  V(660F38,22,_,x,I,I,1,OVM), // #1263
  V(660F38,20,_,x,I,I,3,HVM), // #1264
  V(660F38,25,_,x,I,0,3,HVM), // #1265
  V(660F38,23,_,x,I,I,3,HVM), // #1266
  V(660F38,24,_,x,I,I,2,QVM), // #1267
  V(F30F38,11,_,x,_,0,2,QVM), // #1268
  V(F30F38,13,_,x,_,0,3,HVM), // #1269
  V(F30F38,12,_,x,_,0,1,OVM), // #1270
  V(F30F38,15,_,x,_,0,3,HVM), // #1271
  V(F30F38,14,_,x,_,0,2,QVM), // #1272
  V(F30F38,10,_,x,_,0,3,HVM), // #1273
  V(F30F38,29,_,x,_,1,_,_  ), // #1274
  V(F30F38,30,_,x,_,0,3,HVM), // #1275
  V(660F38,31,_,x,I,I,2,QVM), // #1276
  V(660F38,32,_,x,I,I,1,OVM), // #1277
  V(660F38,30,_,x,I,I,3,HVM), // #1278
  V(660F38,35,_,x,I,0,3,HVM), // #1279
  V(660F38,33,_,x,I,I,3,HVM), // #1280
  V(660F38,34,_,x,I,I,2,QVM), // #1281
  V(660F38,28,_,x,I,1,4,FV ), // #1282
  V(660F38,0B,_,x,I,I,4,FVM), // #1283
  V(660F00,E4,_,x,I,I,4,FVM), // #1284
  V(660F00,E5,_,x,I,I,4,FVM), // #1285
  V(660F38,40,_,x,I,0,4,FV ), // #1286
  V(660F38,40,_,x,_,1,4,FV ), // #1287
  V(660F00,D5,_,x,I,I,4,FVM), // #1288
  V(660F38,83,_,x,_,1,4,FV ), // #1289
  V(660F00,F4,_,x,I,1,4,FV ), // #1290
  V(660F38,54,_,x,_,0,4,FV ), // #1291
  V(660F38,55,_,x,_,0,4,FVM), // #1292
  V(660F38,55,_,x,_,1,4,FVM), // #1293
  V(660F38,54,_,x,_,1,4,FV ), // #1294
  V(660F00,EB,_,x,I,_,_,_  ), // #1295
  V(660F00,EB,_,x,_,0,4,FV ), // #1296
  V(660F00,EB,_,x,_,1,4,FV ), // #1297
  V(XOP_M8,A3,_,0,x,_,_,_  ), // #1298
  V(660F00,72,1,x,_,0,4,FV ), // #1299
  V(660F00,72,1,x,_,1,4,FV ), // #1300
  V(660F38,15,_,x,_,0,4,FV ), // #1301
  V(660F38,15,_,x,_,1,4,FV ), // #1302
  V(660F00,72,0,x,_,0,4,FV ), // #1303
  V(660F00,72,0,x,_,1,4,FV ), // #1304
  V(660F38,14,_,x,_,0,4,FV ), // #1305
  V(660F38,14,_,x,_,1,4,FV ), // #1306
  V(XOP_M9,90,_,0,x,_,_,_  ), // #1307
  V(XOP_M9,92,_,0,x,_,_,_  ), // #1308
  V(XOP_M9,93,_,0,x,_,_,_  ), // #1309
  V(XOP_M9,91,_,0,x,_,_,_  ), // #1310
  V(660F00,F6,_,x,I,I,4,FVM), // #1311
  V(660F38,A0,_,x,_,0,2,T1S), // #1312
  V(660F38,A0,_,x,_,1,3,T1S), // #1313
  V(660F38,A1,_,x,_,0,2,T1S), // #1314
  V(660F38,A1,_,x,_,1,3,T1S), // #1315
  V(XOP_M9,98,_,0,x,_,_,_  ), // #1316
  V(XOP_M9,9A,_,0,x,_,_,_  ), // #1317
  V(XOP_M9,9B,_,0,x,_,_,_  ), // #1318
  V(XOP_M9,99,_,0,x,_,_,_  ), // #1319
  V(XOP_M9,94,_,0,x,_,_,_  ), // #1320
  V(XOP_M9,96,_,0,x,_,_,_  ), // #1321
  V(660F3A,71,_,x,_,0,4,FV ), // #1322
  V(660F3A,71,_,x,_,1,4,FV ), // #1323
  V(660F38,71,_,x,_,0,4,FV ), // #1324
  V(660F38,71,_,x,_,1,4,FV ), // #1325
  V(660F38,70,_,x,_,0,4,FVM), // #1326
  V(660F3A,70,_,x,_,0,4,FVM), // #1327
  V(XOP_M9,97,_,0,x,_,_,_  ), // #1328
  V(XOP_M9,95,_,0,x,_,_,_  ), // #1329
  V(660F3A,73,_,x,_,0,4,FV ), // #1330
  V(660F3A,73,_,x,_,1,4,FV ), // #1331
  V(660F38,73,_,x,_,0,4,FV ), // #1332
  V(660F38,73,_,x,_,1,4,FV ), // #1333
  V(660F38,72,_,x,_,0,4,FVM), // #1334
  V(660F3A,72,_,x,_,0,4,FVM), // #1335
  V(660F38,00,_,x,I,I,4,FVM), // #1336
  V(660F38,8F,_,x,0,0,4,FVM), // #1337
  V(660F00,70,_,x,I,0,4,FV ), // #1338
  V(F30F00,70,_,x,I,I,4,FVM), // #1339
  V(F20F00,70,_,x,I,I,4,FVM), // #1340
  V(660F38,08,_,x,I,_,_,_  ), // #1341
  V(660F38,0A,_,x,I,_,_,_  ), // #1342
  V(660F38,09,_,x,I,_,_,_  ), // #1343
  V(660F00,F2,_,x,I,0,4,128), // #1344
  V(660F00,73,7,x,I,I,4,FVM), // #1345
  V(660F00,F3,_,x,I,1,4,128), // #1346
  V(660F38,47,_,x,0,0,4,FV ), // #1347
  V(660F38,47,_,x,1,1,4,FV ), // #1348
  V(660F38,12,_,x,_,1,4,FVM), // #1349
  V(660F00,F1,_,x,I,I,4,FVM), // #1350
  V(660F00,E2,_,x,I,0,4,128), // #1351
  V(660F00,E2,_,x,_,1,4,128), // #1352
  V(660F38,46,_,x,0,0,4,FV ), // #1353
  V(660F38,46,_,x,_,1,4,FV ), // #1354
  V(660F38,11,_,x,_,1,4,FVM), // #1355
  V(660F00,E1,_,x,I,I,4,128), // #1356
  V(660F00,D2,_,x,I,0,4,128), // #1357
  V(660F00,73,3,x,I,I,4,FVM), // #1358
  V(660F00,D3,_,x,I,1,4,128), // #1359
  V(660F38,45,_,x,0,0,4,FV ), // #1360
  V(660F38,45,_,x,1,1,4,FV ), // #1361
  V(660F38,10,_,x,_,1,4,FVM), // #1362
  V(660F00,D1,_,x,I,I,4,128), // #1363
  V(660F00,F8,_,x,I,I,4,FVM), // #1364
  V(660F00,FA,_,x,I,0,4,FV ), // #1365
  V(660F00,FB,_,x,I,1,4,FV ), // #1366
  V(660F00,E8,_,x,I,I,4,FVM), // #1367
  V(660F00,E9,_,x,I,I,4,FVM), // #1368
  V(660F00,D8,_,x,I,I,4,FVM), // #1369
  V(660F00,D9,_,x,I,I,4,FVM), // #1370
  V(660F00,F9,_,x,I,I,4,FVM), // #1371
  V(660F3A,25,_,x,_,0,4,FV ), // #1372
  V(660F3A,25,_,x,_,1,4,FV ), // #1373
  V(660F38,17,_,x,I,_,_,_  ), // #1374
  V(660F38,26,_,x,_,0,4,FVM), // #1375
  V(660F38,27,_,x,_,0,4,FV ), // #1376
  V(660F38,27,_,x,_,1,4,FV ), // #1377
  V(660F38,26,_,x,_,1,4,FVM), // #1378
  V(F30F38,26,_,x,_,0,4,FVM), // #1379
  V(F30F38,27,_,x,_,0,4,FV ), // #1380
  V(F30F38,27,_,x,_,1,4,FV ), // #1381
  V(F30F38,26,_,x,_,1,4,FVM), // #1382
  V(660F00,68,_,x,I,I,4,FVM), // #1383
  V(660F00,6A,_,x,I,0,4,FV ), // #1384
  V(660F00,6D,_,x,I,1,4,FV ), // #1385
  V(660F00,69,_,x,I,I,4,FVM), // #1386
  V(660F00,60,_,x,I,I,4,FVM), // #1387
  V(660F00,62,_,x,I,0,4,FV ), // #1388
  V(660F00,6C,_,x,I,1,4,FV ), // #1389
  V(660F00,61,_,x,I,I,4,FVM), // #1390
  V(660F00,EF,_,x,I,_,_,_  ), // #1391
  V(660F00,EF,_,x,_,0,4,FV ), // #1392
  V(660F00,EF,_,x,_,1,4,FV ), // #1393
  V(660F3A,50,_,x,_,1,4,FV ), // #1394
  V(660F3A,50,_,x,_,0,4,FV ), // #1395
  V(660F3A,51,_,I,_,1,3,T1S), // #1396
  V(660F3A,51,_,I,_,0,2,T1S), // #1397
  V(660F38,4C,_,x,_,1,4,FV ), // #1398
  V(660F38,4C,_,x,_,0,4,FV ), // #1399
  V(660F38,4D,_,I,_,1,3,T1S), // #1400
  V(660F38,4D,_,I,_,0,2,T1S), // #1401
  V(660F38,CA,_,2,_,1,4,FV ), // #1402
  V(660F38,CA,_,2,_,0,4,FV ), // #1403
  V(660F38,CB,_,I,_,1,3,T1S), // #1404
  V(660F38,CB,_,I,_,0,2,T1S), // #1405
  V(000F00,53,_,x,I,_,_,_  ), // #1406
  V(F30F00,53,_,I,I,_,_,_  ), // #1407
  V(660F3A,56,_,x,_,1,4,FV ), // #1408
  V(660F3A,56,_,x,_,0,4,FV ), // #1409
  V(660F3A,57,_,I,_,1,3,T1S), // #1410
  V(660F3A,57,_,I,_,0,2,T1S), // #1411
  V(660F3A,09,_,x,_,1,4,FV ), // #1412
  V(660F3A,08,_,x,_,0,4,FV ), // #1413
  V(660F3A,0B,_,I,_,1,3,T1S), // #1414
  V(660F3A,0A,_,I,_,0,2,T1S), // #1415
  V(660F3A,09,_,x,I,_,_,_  ), // #1416
  V(660F3A,08,_,x,I,_,_,_  ), // #1417
  V(660F3A,0B,_,I,I,_,_,_  ), // #1418
  V(660F3A,0A,_,I,I,_,_,_  ), // #1419
  V(660F38,4E,_,x,_,1,4,FV ), // #1420
  V(660F38,4E,_,x,_,0,4,FV ), // #1421
  V(660F38,4F,_,I,_,1,3,T1S), // #1422
  V(660F38,4F,_,I,_,0,2,T1S), // #1423
  V(660F38,CC,_,2,_,1,4,FV ), // #1424
  V(660F38,CC,_,2,_,0,4,FV ), // #1425
  V(660F38,CD,_,I,_,1,3,T1S), // #1426
  V(660F38,CD,_,I,_,0,2,T1S), // #1427
  V(000F00,52,_,x,I,_,_,_  ), // #1428
  V(F30F00,52,_,I,I,_,_,_  ), // #1429
  V(660F38,2C,_,x,_,1,4,FV ), // #1430
  V(660F38,2C,_,x,_,0,4,FV ), // #1431
  V(660F38,2D,_,I,_,1,3,T1S), // #1432
  V(660F38,2D,_,I,_,0,2,T1S), // #1433
  V(660F38,A2,_,x,_,1,3,T1S), // #1434
  V(660F38,A2,_,x,_,0,2,T1S), // #1435
  V(660F38,C6,5,2,_,1,3,T1S), // #1436
  V(660F38,C6,5,2,_,0,2,T1S), // #1437
  V(660F38,C7,5,2,_,1,3,T1S), // #1438
  V(660F38,C7,5,2,_,0,2,T1S), // #1439
  V(660F38,C6,6,2,_,1,3,T1S), // #1440
  V(660F38,C6,6,2,_,0,2,T1S), // #1441
  V(660F38,C7,6,2,_,1,3,T1S), // #1442
  V(660F38,C7,6,2,_,0,2,T1S), // #1443
  V(660F38,A3,_,x,_,1,3,T1S), // #1444
  V(660F38,A3,_,x,_,0,2,T1S), // #1445
  V(660F3A,23,_,x,_,0,4,FV ), // #1446
  V(660F3A,23,_,x,_,1,4,FV ), // #1447
  V(660F3A,43,_,x,_,0,4,FV ), // #1448
  V(660F3A,43,_,x,_,1,4,FV ), // #1449
  V(660F00,C6,_,x,I,1,4,FV ), // #1450
  V(000F00,C6,_,x,I,0,4,FV ), // #1451
  V(660F00,51,_,x,I,1,4,FV ), // #1452
  V(000F00,51,_,x,I,0,4,FV ), // #1453
  V(F20F00,51,_,I,I,1,3,T1S), // #1454
  V(F30F00,51,_,I,I,0,2,T1S), // #1455
  V(000F00,AE,3,0,I,_,_,_  ), // #1456
  V(660F00,5C,_,x,I,1,4,FV ), // #1457
  V(000F00,5C,_,x,I,0,4,FV ), // #1458
  V(F20F00,5C,_,I,I,1,3,T1S), // #1459
  V(F30F00,5C,_,I,I,0,2,T1S), // #1460
  V(660F38,0F,_,x,0,_,_,_  ), // #1461
  V(660F38,0E,_,x,0,_,_,_  ), // #1462
  V(660F00,2E,_,I,I,1,3,T1S), // #1463
  V(000F00,2E,_,I,I,0,2,T1S), // #1464
  V(660F00,15,_,x,I,1,4,FV ), // #1465
  V(000F00,15,_,x,I,0,4,FV ), // #1466
  V(660F00,14,_,x,I,1,4,FV ), // #1467
  V(000F00,14,_,x,I,0,4,FV ), // #1468
  V(660F00,57,_,x,I,1,4,FV ), // #1469
  V(000F00,57,_,x,I,0,4,FV ), // #1470
  V(000F00,77,_,1,I,_,_,_  ), // #1471
  V(000F00,77,_,0,I,_,_,_  ), // #1472
  O(000F00,09,_,_,_,_,_,_  ), // #1473
  O(F30F00,09,_,_,_,_,_,_  ), // #1474
  O(F30F00,AE,2,_,x,_,_,_  ), // #1475
  O(F30F00,AE,3,_,x,_,_,_  ), // #1476
  O(000F00,30,_,_,_,_,_,_  ), // #1477
  O(000000,C6,7,_,_,_,_,_  ), // #1478
  O(000F00,C0,_,_,x,_,_,_  ), // #1479
  O(000000,C7,7,_,_,_,_,_  ), // #1480
  O(000000,86,_,_,x,_,_,_  ), // #1481
  O(000F01,D5,_,_,_,_,_,_  ), // #1482
  O(000F01,D0,_,_,_,_,_,_  ), // #1483
  O(000000,D7,_,_,_,_,_,_  ), // #1484
  O(000000,30,6,_,x,_,_,_  ), // #1485
  O(660F00,57,_,_,_,_,_,_  ), // #1486
  O(000F00,57,_,_,_,_,_,_  ), // #1487
  O(000F00,AE,5,_,_,_,_,_  ), // #1488
  O(000F00,AE,5,_,1,_,_,_  ), // #1489
  O(000F00,C7,3,_,_,_,_,_  ), // #1490
  O(000F00,C7,3,_,1,_,_,_  ), // #1491
  O(000F00,AE,4,_,_,_,_,_  ), // #1492
  O(000F00,AE,4,_,1,_,_,_  ), // #1493
  O(000F00,C7,4,_,_,_,_,_  ), // #1494
  O(000F00,C7,4,_,1,_,_,_  ), // #1495
  O(000F00,AE,6,_,_,_,_,_  ), // #1496
  O(000F00,AE,6,_,1,_,_,_  ), // #1497
  O(000F00,C7,5,_,_,_,_,_  ), // #1498
  O(000F00,C7,5,_,1,_,_,_  ), // #1499
  O(000F01,D1,_,_,_,_,_,_  ), // #1500
  O(000F01,D6,_,_,_,_,_,_  )  // #1501
};
// ----------------------------------------------------------------------------
// ${MainOpcodeTable:End}

// ============================================================================
// [asmjit::x86::InstDB - AltOpcodeIndex / AltOpcodeTable]
// ============================================================================

// ${AltOpcodeIndex:Begin}
// ------------------- Automatically generated, do not edit -------------------
const uint8_t InstDB::_altOpcodeIndex[] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
  0, 0, 2, 3, 4, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, 9, 10, 0, 0, 11, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  13, 14, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 15, 16, 0, 0, 17, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  18, 19, 20, 21, 20, 22, 23, 24, 25, 26, 27, 28, 21, 20, 19, 18, 19, 29, 27,
  26, 25, 24, 30, 31, 32, 29, 33, 34, 34, 31, 35, 22, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 36, 37, 38, 39, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 40, 41, 42, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 43, 44, 45, 46, 0, 0, 0, 0, 47, 48, 0, 49, 50, 0, 51, 52,
  0, 0, 53, 0, 0, 54, 55, 56, 57, 58, 59, 0, 0, 60, 0, 0, 61, 0, 0, 62, 63, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 64, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 65, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  66, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 67,
  68, 69, 70, 71, 72, 73, 74, 75, 76, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 77, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 78, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 79, 80, 0, 0, 0, 0, 0, 0, 0, 0, 81, 82, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 83, 84, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 85, 86, 87, 0, 88, 89, 90, 91, 92, 93, 94,
  95, 0, 96, 97, 0, 98, 99, 0, 0, 0, 0, 0, 0, 100, 101, 0, 0, 102, 103, 104,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 105,
  106, 0, 0, 107, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 108, 109, 110,
  111, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 112, 113, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 114,
  115, 116, 117, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 118, 0, 119, 0, 0, 0, 120, 121, 122, 0,
  0, 0, 123, 124, 0, 125, 0, 0, 0, 126, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
// ----------------------------------------------------------------------------
// ${AltOpcodeIndex:End}

// ${AltOpcodeTable:Begin}
// ------------------- Automatically generated, do not edit -------------------
const uint32_t InstDB::_altOpcodeTable[] = {
  0                         , // #0 [ref=1362x]
  O(660F00,1B,_,_,_,_,_,_  ), // #1 [ref=1x]
  O(000F00,BA,4,_,x,_,_,_  ), // #2 [ref=1x]
  O(000F00,BA,7,_,x,_,_,_  ), // #3 [ref=1x]
  O(000F00,BA,6,_,x,_,_,_  ), // #4 [ref=1x]
  O(000F00,BA,5,_,x,_,_,_  ), // #5 [ref=1x]
  O(000000,48,_,_,x,_,_,_  ), // #6 [ref=1x]
  O(660F00,78,0,_,_,_,_,_  ), // #7 [ref=1x]
  O_FPU(00,00DF,5)          , // #8 [ref=1x]
  O_FPU(00,00DF,7)          , // #9 [ref=1x]
  O_FPU(00,00DD,1)          , // #10 [ref=1x]
  O_FPU(00,00DB,5)          , // #11 [ref=1x]
  O_FPU(00,DFE0,_)          , // #12 [ref=1x]
  O(000000,DB,7,_,_,_,_,_  ), // #13 [ref=1x]
  O_FPU(9B,DFE0,_)          , // #14 [ref=1x]
  O(000000,E4,_,_,_,_,_,_  ), // #15 [ref=1x]
  O(000000,40,_,_,x,_,_,_  ), // #16 [ref=1x]
  O(F20F00,78,_,_,_,_,_,_  ), // #17 [ref=1x]
  O(000000,77,_,_,_,_,_,_  ), // #18 [ref=2x]
  O(000000,73,_,_,_,_,_,_  ), // #19 [ref=3x]
  O(000000,72,_,_,_,_,_,_  ), // #20 [ref=3x]
  O(000000,76,_,_,_,_,_,_  ), // #21 [ref=2x]
  O(000000,74,_,_,_,_,_,_  ), // #22 [ref=2x]
  O(000000,E3,_,_,_,_,_,_  ), // #23 [ref=1x]
  O(000000,7F,_,_,_,_,_,_  ), // #24 [ref=2x]
  O(000000,7D,_,_,_,_,_,_  ), // #25 [ref=2x]
  O(000000,7C,_,_,_,_,_,_  ), // #26 [ref=2x]
  O(000000,7E,_,_,_,_,_,_  ), // #27 [ref=2x]
  O(000000,EB,_,_,_,_,_,_  ), // #28 [ref=1x]
  O(000000,75,_,_,_,_,_,_  ), // #29 [ref=2x]
  O(000000,71,_,_,_,_,_,_  ), // #30 [ref=1x]
  O(000000,7B,_,_,_,_,_,_  ), // #31 [ref=2x]
  O(000000,79,_,_,_,_,_,_  ), // #32 [ref=1x]
  O(000000,70,_,_,_,_,_,_  ), // #33 [ref=1x]
  O(000000,7A,_,_,_,_,_,_  ), // #34 [ref=2x]
  O(000000,78,_,_,_,_,_,_  ), // #35 [ref=1x]
  V(660F00,92,_,0,0,_,_,_  ), // #36 [ref=1x]
  V(F20F00,92,_,0,0,_,_,_  ), // #37 [ref=1x]
  V(F20F00,92,_,0,1,_,_,_  ), // #38 [ref=1x]
  V(000F00,92,_,0,0,_,_,_  ), // #39 [ref=1x]
  O(000000,E2,_,_,_,_,_,_  ), // #40 [ref=1x]
  O(000000,E1,_,_,_,_,_,_  ), // #41 [ref=1x]
  O(000000,E0,_,_,_,_,_,_  ), // #42 [ref=1x]
  O(660F00,29,_,_,_,_,_,_  ), // #43 [ref=1x]
  O(000F00,29,_,_,_,_,_,_  ), // #44 [ref=1x]
  O(000F38,F1,_,_,x,_,_,_  ), // #45 [ref=1x]
  O(000F00,7E,_,_,_,_,_,_  ), // #46 [ref=1x]
  O(660F00,7F,_,_,_,_,_,_  ), // #47 [ref=1x]
  O(F30F00,7F,_,_,_,_,_,_  ), // #48 [ref=1x]
  O(660F00,17,_,_,_,_,_,_  ), // #49 [ref=1x]
  O(000F00,17,_,_,_,_,_,_  ), // #50 [ref=1x]
  O(660F00,13,_,_,_,_,_,_  ), // #51 [ref=1x]
  O(000F00,13,_,_,_,_,_,_  ), // #52 [ref=1x]
  O(660F00,E7,_,_,_,_,_,_  ), // #53 [ref=1x]
  O(660F00,2B,_,_,_,_,_,_  ), // #54 [ref=1x]
  O(000F00,2B,_,_,_,_,_,_  ), // #55 [ref=1x]
  O(000F00,E7,_,_,_,_,_,_  ), // #56 [ref=1x]
  O(F20F00,2B,_,_,_,_,_,_  ), // #57 [ref=1x]
  O(F30F00,2B,_,_,_,_,_,_  ), // #58 [ref=1x]
  O(000F00,7E,_,_,x,_,_,_  ), // #59 [ref=1x]
  O(F20F00,11,_,_,_,_,_,_  ), // #60 [ref=1x]
  O(F30F00,11,_,_,_,_,_,_  ), // #61 [ref=1x]
  O(660F00,11,_,_,_,_,_,_  ), // #62 [ref=1x]
  O(000F00,11,_,_,_,_,_,_  ), // #63 [ref=1x]
  O(000000,E6,_,_,_,_,_,_  ), // #64 [ref=1x]
  O(000F3A,15,_,_,_,_,_,_  ), // #65 [ref=1x]
  O(000000,58,_,_,_,_,_,_  ), // #66 [ref=1x]
  O(000F00,72,6,_,_,_,_,_  ), // #67 [ref=1x]
  O(660F00,73,7,_,_,_,_,_  ), // #68 [ref=1x]
  O(000F00,73,6,_,_,_,_,_  ), // #69 [ref=1x]
  O(000F00,71,6,_,_,_,_,_  ), // #70 [ref=1x]
  O(000F00,72,4,_,_,_,_,_  ), // #71 [ref=1x]
  O(000F00,71,4,_,_,_,_,_  ), // #72 [ref=1x]
  O(000F00,72,2,_,_,_,_,_  ), // #73 [ref=1x]
  O(660F00,73,3,_,_,_,_,_  ), // #74 [ref=1x]
  O(000F00,73,2,_,_,_,_,_  ), // #75 [ref=1x]
  O(000F00,71,2,_,_,_,_,_  ), // #76 [ref=1x]
  O(000000,50,_,_,_,_,_,_  ), // #77 [ref=1x]
  O(000000,F6,_,_,x,_,_,_  ), // #78 [ref=1x]
  V(660F38,92,_,x,_,1,3,T1S), // #79 [ref=1x]
  V(660F38,92,_,x,_,0,2,T1S), // #80 [ref=1x]
  V(660F38,93,_,x,_,1,3,T1S), // #81 [ref=1x]
  V(660F38,93,_,x,_,0,2,T1S), // #82 [ref=1x]
  V(660F38,2F,_,x,0,_,_,_  ), // #83 [ref=1x]
  V(660F38,2E,_,x,0,_,_,_  ), // #84 [ref=1x]
  V(660F00,29,_,x,I,1,4,FVM), // #85 [ref=1x]
  V(000F00,29,_,x,I,0,4,FVM), // #86 [ref=1x]
  V(660F00,7E,_,0,0,0,2,T1S), // #87 [ref=1x]
  V(660F00,7F,_,x,I,_,_,_  ), // #88 [ref=1x]
  V(660F00,7F,_,x,_,0,4,FVM), // #89 [ref=1x]
  V(660F00,7F,_,x,_,1,4,FVM), // #90 [ref=1x]
  V(F30F00,7F,_,x,I,_,_,_  ), // #91 [ref=1x]
  V(F20F00,7F,_,x,_,1,4,FVM), // #92 [ref=1x]
  V(F30F00,7F,_,x,_,0,4,FVM), // #93 [ref=1x]
  V(F30F00,7F,_,x,_,1,4,FVM), // #94 [ref=1x]
  V(F20F00,7F,_,x,_,0,4,FVM), // #95 [ref=1x]
  V(660F00,17,_,0,I,1,3,T1S), // #96 [ref=1x]
  V(000F00,17,_,0,I,0,3,T2 ), // #97 [ref=1x]
  V(660F00,13,_,0,I,1,3,T1S), // #98 [ref=1x]
  V(000F00,13,_,0,I,0,3,T2 ), // #99 [ref=1x]
  V(660F00,7E,_,0,I,1,3,T1S), // #100 [ref=1x]
  V(F20F00,11,_,I,I,1,3,T1S), // #101 [ref=1x]
  V(F30F00,11,_,I,I,0,2,T1S), // #102 [ref=1x]
  V(660F00,11,_,x,I,1,4,FVM), // #103 [ref=1x]
  V(000F00,11,_,x,I,0,4,FVM), // #104 [ref=1x]
  V(660F3A,05,_,x,0,1,4,FV ), // #105 [ref=1x]
  V(660F3A,04,_,x,0,0,4,FV ), // #106 [ref=1x]
  V(660F3A,00,_,x,1,1,4,FV ), // #107 [ref=1x]
  V(660F38,90,_,x,_,0,2,T1S), // #108 [ref=1x]
  V(660F38,90,_,x,_,1,3,T1S), // #109 [ref=1x]
  V(660F38,91,_,x,_,0,2,T1S), // #110 [ref=1x]
  V(660F38,91,_,x,_,1,3,T1S), // #111 [ref=1x]
  V(660F38,8E,_,x,0,_,_,_  ), // #112 [ref=1x]
  V(660F38,8E,_,x,1,_,_,_  ), // #113 [ref=1x]
  V(XOP_M8,C0,_,0,x,_,_,_  ), // #114 [ref=1x]
  V(XOP_M8,C2,_,0,x,_,_,_  ), // #115 [ref=1x]
  V(XOP_M8,C3,_,0,x,_,_,_  ), // #116 [ref=1x]
  V(XOP_M8,C1,_,0,x,_,_,_  ), // #117 [ref=1x]
  V(660F00,72,6,x,I,0,4,FV ), // #118 [ref=1x]
  V(660F00,73,6,x,I,1,4,FV ), // #119 [ref=1x]
  V(660F00,71,6,x,I,I,4,FVM), // #120 [ref=1x]
  V(660F00,72,4,x,I,0,4,FV ), // #121 [ref=1x]
  V(660F00,72,4,x,_,1,4,FV ), // #122 [ref=1x]
  V(660F00,71,4,x,I,I,4,FVM), // #123 [ref=1x]
  V(660F00,72,2,x,I,0,4,FV ), // #124 [ref=1x]
  V(660F00,73,2,x,I,1,4,FV ), // #125 [ref=1x]
  V(660F00,71,2,x,I,I,4,FVM)  // #126 [ref=1x]
};
// ----------------------------------------------------------------------------
// ${AltOpcodeTable:End}

#undef O_FPU
#undef O
#undef V

// ============================================================================
// [asmjit::x86::InstDB - CommonInfo]
// ============================================================================

// ${InstCommonTable:Begin}
// ------------------- Automatically generated, do not edit -------------------
#define F(VAL) InstDB::kFlag##VAL
#define CONTROL(VAL) Inst::kControl##VAL
#define SINGLE_REG(VAL) InstDB::kSingleReg##VAL
#define SPECIAL_CASE(VAL) InstDB::kSpecialCase##VAL
const InstDB::CommonInfo InstDB::_commonInfoTable[] = {
  { 0                                                     , 0  , 0 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #0 [ref=1x]
  { F(FixedReg)                                           , 339, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #1 [ref=4x]
  { F(FixedReg)                                           , 340, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #2 [ref=2x]
  { F(Lock)|F(XAcquire)|F(XRelease)                       , 16 , 12, CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #3 [ref=3x]
  { 0                                                     , 151, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #4 [ref=2x]
  { F(Vec)                                                , 70 , 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #5 [ref=54x]
  { F(Vec)                                                , 97 , 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #6 [ref=19x]
  { F(Vec)                                                , 222, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #7 [ref=16x]
  { F(Vec)                                                , 183, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #8 [ref=20x]
  { F(Lock)|F(XAcquire)|F(XRelease)                       , 28 , 11, CONTROL(None)   , SINGLE_REG(RO)  , 0                   , 0 }, // #9 [ref=1x]
  { F(Vex)                                                , 237, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #10 [ref=3x]
  { F(Vec)                                                , 70 , 1 , CONTROL(None)   , SINGLE_REG(RO)  , 0                   , 0 }, // #11 [ref=12x]
  { 0                                                     , 341, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #12 [ref=1x]
  { F(Vex)                                                , 239, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #13 [ref=5x]
  { F(Vex)                                                , 151, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #14 [ref=12x]
  { F(FixedReg)|F(Vec)                                    , 342, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #15 [ref=4x]
  { 0                                                     , 241, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #16 [ref=3x]
  { F(Mib)                                                , 343, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #17 [ref=1x]
  { 0                                                     , 344, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #18 [ref=1x]
  { 0                                                     , 243, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #19 [ref=1x]
  { F(Mib)                                                , 345, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #20 [ref=1x]
  { 0                                                     , 245, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #21 [ref=1x]
  { 0                                                     , 150, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #22 [ref=35x]
  { 0                                                     , 346, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #23 [ref=1x]
  { 0                                                     , 114, 4 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #24 [ref=1x]
  { F(Lock)|F(XAcquire)|F(XRelease)                       , 114, 4 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #25 [ref=3x]
  { 0                                                     , 247, 2 , CONTROL(Call)   , SINGLE_REG(None), 0                   , 0 }, // #26 [ref=1x]
  { F(FixedReg)                                           , 347, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #27 [ref=1x]
  { F(FixedReg)                                           , 348, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #28 [ref=2x]
  { F(FixedReg)                                           , 322, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #29 [ref=1x]
  { 0                                                     , 255, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #30 [ref=74x]
  { 0                                                     , 349, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #31 [ref=24x]
  { F(FixedRM)                                            , 350, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #32 [ref=1x]
  { 0                                                     , 16 , 12, CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #33 [ref=1x]
  { F(FixedRM)|F(Rep)                                     , 351, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #34 [ref=1x]
  { F(Vec)                                                , 352, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #35 [ref=2x]
  { F(Vec)                                                , 353, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #36 [ref=3x]
  { F(FixedReg)|F(Lock)|F(XAcquire)|F(XRelease)           , 118, 4 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #37 [ref=1x]
  { F(FixedReg)|F(Lock)|F(XAcquire)|F(XRelease)           , 354, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #38 [ref=1x]
  { F(FixedReg)|F(Lock)|F(XAcquire)|F(XRelease)           , 355, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #39 [ref=1x]
  { F(FixedReg)                                           , 356, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #40 [ref=1x]
  { F(FixedReg)                                           , 357, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #41 [ref=1x]
  { 0                                                     , 249, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #42 [ref=1x]
  { F(Mmx)|F(Vec)                                         , 358, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #43 [ref=2x]
  { F(Mmx)|F(Vec)                                         , 359, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #44 [ref=2x]
  { F(Mmx)|F(Vec)                                         , 360, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #45 [ref=2x]
  { F(Vec)                                                , 361, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #46 [ref=2x]
  { F(Vec)                                                , 362, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #47 [ref=2x]
  { F(Vec)                                                , 363, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #48 [ref=2x]
  { F(FixedReg)                                           , 364, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #49 [ref=1x]
  { F(FixedReg)                                           , 365, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #50 [ref=2x]
  { F(Lock)|F(XAcquire)|F(XRelease)                       , 251, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #51 [ref=2x]
  { F(FixedReg)                                           , 39 , 4 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #52 [ref=3x]
  { F(Mmx)                                                , 255, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #53 [ref=1x]
  { 0                                                     , 366, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #54 [ref=1x]
  { F(Vec)                                                , 367, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #55 [ref=2x]
  { F(Vec)                                                , 253, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #56 [ref=1x]
  { F(FixedReg)|F(FpuM32)|F(FpuM64)                       , 153, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #57 [ref=6x]
  { 0                                                     , 255, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #58 [ref=9x]
  { F(FpuM80)                                             , 368, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #59 [ref=2x]
  { 0                                                     , 256, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #60 [ref=13x]
  { F(FpuM32)|F(FpuM64)                                   , 257, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #61 [ref=2x]
  { F(FpuM16)|F(FpuM32)                                   , 369, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #62 [ref=9x]
  { F(FpuM16)|F(FpuM32)|F(FpuM64)                         , 370, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #63 [ref=3x]
  { F(FpuM32)|F(FpuM64)|F(FpuM80)                         , 371, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #64 [ref=2x]
  { F(FpuM16)                                             , 372, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #65 [ref=3x]
  { F(FixedReg)|F(FpuM16)                                 , 373, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #66 [ref=2x]
  { F(FpuM32)|F(FpuM64)                                   , 258, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #67 [ref=1x]
  { 0                                                     , 374, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #68 [ref=2x]
  { F(FixedReg)                                           , 39 , 10, CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #69 [ref=1x]
  { F(FixedReg)                                           , 375, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #70 [ref=1x]
  { F(FixedRM)|F(Rep)                                     , 376, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #71 [ref=1x]
  { F(Vec)                                                , 259, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #72 [ref=1x]
  { 0                                                     , 377, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #73 [ref=2x]
  { 0                                                     , 378, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #74 [ref=8x]
  { 0                                                     , 261, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #75 [ref=3x]
  { F(FixedReg)                                           , 263, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #76 [ref=1x]
  { 0                                                     , 255, 1 , CONTROL(Return) , SINGLE_REG(None), 0                   , 0 }, // #77 [ref=3x]
  { 0                                                     , 379, 1 , CONTROL(Return) , SINGLE_REG(None), 0                   , 0 }, // #78 [ref=1x]
  { 0                                                     , 265, 2 , CONTROL(Branch) , SINGLE_REG(None), 0                   , 0 }, // #79 [ref=30x]
  { F(FixedReg)                                           , 267, 2 , CONTROL(Branch) , SINGLE_REG(None), 0                   , 0 }, // #80 [ref=4x]
  { 0                                                     , 269, 2 , CONTROL(Jump)   , SINGLE_REG(None), 0                   , 0 }, // #81 [ref=1x]
  { F(Vec)|F(Vex)                                         , 380, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #82 [ref=27x]
  { F(Vec)|F(Vex)                                         , 271, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #83 [ref=1x]
  { F(Vec)|F(Vex)                                         , 273, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #84 [ref=1x]
  { F(Vec)|F(Vex)                                         , 275, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #85 [ref=1x]
  { F(Vec)|F(Vex)                                         , 277, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #86 [ref=1x]
  { F(Vec)|F(Vex)                                         , 381, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #87 [ref=12x]
  { F(Vec)|F(Vex)                                         , 382, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #88 [ref=8x]
  { F(FixedReg)                                           , 383, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #89 [ref=2x]
  { 0                                                     , 279, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #90 [ref=1x]
  { F(Vec)                                                , 192, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #91 [ref=2x]
  { 0                                                     , 384, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #92 [ref=2x]
  { 0                                                     , 281, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #93 [ref=2x]
  { 0                                                     , 385, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #94 [ref=1x]
  { 0                                                     , 156, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #95 [ref=3x]
  { 0                                                     , 386, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #96 [ref=5x]
  { F(Vex)                                                , 346, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #97 [ref=2x]
  { F(FixedRM)|F(Rep)                                     , 387, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #98 [ref=1x]
  { 0                                                     , 283, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #99 [ref=1x]
  { F(Vex)                                                , 388, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #100 [ref=2x]
  { F(FixedRM)|F(Vec)                                     , 389, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #101 [ref=1x]
  { F(FixedRM)|F(Mmx)                                     , 390, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #102 [ref=1x]
  { F(FixedRM)                                            , 391, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #103 [ref=2x]
  { F(FixedReg)|F(XRelease)                               , 0  , 16, CONTROL(None)   , SINGLE_REG(None), SPECIAL_CASE(MovCrDr), 0 }, // #104 [ref=1x]
  { F(Vec)                                                , 70 , 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #105 [ref=6x]
  { 0                                                     , 64 , 6 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #106 [ref=1x]
  { F(Mmx)|F(Vec)                                         , 285, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #107 [ref=1x]
  { 0                                                     , 287, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #108 [ref=1x]
  { 0                                                     , 68 , 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #109 [ref=2x]
  { F(Mmx)|F(Vec)                                         , 392, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #110 [ref=1x]
  { F(Vec)                                                , 254, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #111 [ref=2x]
  { F(Vec)                                                , 198, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #112 [ref=4x]
  { F(Vec)                                                , 393, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #113 [ref=2x]
  { F(Vec)                                                , 71 , 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #114 [ref=3x]
  { F(Mmx)                                                , 394, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #115 [ref=1x]
  { F(Vec)                                                , 98 , 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #116 [ref=1x]
  { F(Vec)                                                , 201, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #117 [ref=1x]
  { F(Mmx)|F(Vec)                                         , 94 , 5 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #118 [ref=1x]
  { F(Mmx)|F(Vec)                                         , 395, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #119 [ref=1x]
  { F(FixedRM)|F(Rep)                                     , 396, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #120 [ref=1x]
  { F(Vec)                                                , 97 , 2 , CONTROL(None)   , SINGLE_REG(None), SPECIAL_CASE(MovSsSd), 0 }, // #121 [ref=1x]
  { F(Vec)                                                , 289, 2 , CONTROL(None)   , SINGLE_REG(None), SPECIAL_CASE(MovSsSd), 0 }, // #122 [ref=1x]
  { 0                                                     , 291, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #123 [ref=2x]
  { 0                                                     , 397, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #124 [ref=1x]
  { F(FixedReg)|F(Vex)                                    , 293, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #125 [ref=1x]
  { F(FixedReg)                                           , 398, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #126 [ref=1x]
  { F(FixedReg)                                           , 399, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #127 [ref=1x]
  { F(Lock)|F(XAcquire)|F(XRelease)                       , 252, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #128 [ref=2x]
  { 0                                                     , 295, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #129 [ref=1x]
  { F(Lock)|F(XAcquire)|F(XRelease)                       , 16 , 12, CONTROL(None)   , SINGLE_REG(RO)  , 0                   , 0 }, // #130 [ref=1x]
  { F(FixedReg)                                           , 400, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #131 [ref=1x]
  { F(FixedRM)|F(Rep)                                     , 401, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #132 [ref=1x]
  { F(Mmx)|F(Vec)                                         , 297, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #133 [ref=40x]
  { F(Mmx)|F(Vec)                                         , 299, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #134 [ref=1x]
  { F(Mmx)|F(Vec)                                         , 297, 2 , CONTROL(None)   , SINGLE_REG(RO)  , 0                   , 0 }, // #135 [ref=6x]
  { F(Mmx)|F(Vec)                                         , 297, 2 , CONTROL(None)   , SINGLE_REG(WO)  , 0                   , 0 }, // #136 [ref=16x]
  { F(Mmx)                                                , 297, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #137 [ref=26x]
  { F(Vec)                                                , 70 , 1 , CONTROL(None)   , SINGLE_REG(WO)  , 0                   , 0 }, // #138 [ref=4x]
  { F(FixedReg)|F(Vec)                                    , 402, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #139 [ref=1x]
  { F(FixedReg)|F(Vec)                                    , 403, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #140 [ref=1x]
  { F(FixedReg)|F(Vec)                                    , 404, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #141 [ref=1x]
  { F(FixedReg)|F(Vec)                                    , 405, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #142 [ref=1x]
  { F(Vec)                                                , 406, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #143 [ref=1x]
  { F(Vec)                                                , 407, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #144 [ref=1x]
  { F(Mmx)|F(Vec)                                         , 301, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #145 [ref=1x]
  { F(Vec)                                                , 408, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #146 [ref=1x]
  { F(Vec)                                                , 409, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #147 [ref=1x]
  { F(Vec)                                                , 410, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #148 [ref=1x]
  { F(Mmx)|F(Vec)                                         , 411, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #149 [ref=1x]
  { F(Mmx)|F(Vec)                                         , 412, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #150 [ref=1x]
  { F(Vec)                                                , 225, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #151 [ref=2x]
  { F(FixedReg)                                           , 122, 4 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #152 [ref=1x]
  { 0                                                     , 379, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #153 [ref=6x]
  { F(Mmx)                                                , 299, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #154 [ref=1x]
  { F(Mmx)|F(Vec)                                         , 303, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #155 [ref=8x]
  { F(Vec)                                                , 413, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #156 [ref=2x]
  { F(FixedReg)                                           , 126, 4 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #157 [ref=1x]
  { F(FixedReg)                                           , 414, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #158 [ref=8x]
  { 0                                                     , 415, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #159 [ref=4x]
  { F(FixedReg)                                           , 416, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #160 [ref=6x]
  { 0                                                     , 305, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #161 [ref=1x]
  { 0                                                     , 417, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #162 [ref=2x]
  { F(Rep)|F(RepIgnored)                                  , 307, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #163 [ref=1x]
  { F(Vex)                                                , 309, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #164 [ref=1x]
  { F(FixedRM)|F(Rep)                                     , 418, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #165 [ref=1x]
  { 0                                                     , 419, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #166 [ref=30x]
  { F(FixedReg)                                           , 159, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #167 [ref=2x]
  { 0                                                     , 420, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #168 [ref=3x]
  { F(FixedRM)|F(Rep)                                     , 421, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #169 [ref=1x]
  { F(Lock)|F(XAcquire)|F(XRelease)                       , 16 , 12, CONTROL(None)   , SINGLE_REG(WO)  , 0                   , 0 }, // #170 [ref=2x]
  { 0                                                     , 57 , 7 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #171 [ref=1x]
  { F(Vec)|F(Evex)|F(Avx512T4X)|F(Avx512KZ)               , 422, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #172 [ref=4x]
  { F(Vec)|F(Evex)|F(Avx512T4X)|F(Avx512KZ)               , 423, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #173 [ref=2x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_ER_SAE_B64)          , 162, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #174 [ref=22x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_ER_SAE_B32)          , 162, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #175 [ref=22x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_ER_SAE)              , 424, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #176 [ref=18x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_ER_SAE)              , 425, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #177 [ref=17x]
  { F(Vec)|F(Vex)                                         , 162, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #178 [ref=15x]
  { F(Vec)|F(Vex)|F(Evex)                                 , 162, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #179 [ref=5x]
  { F(Vec)|F(Vex)                                         , 70 , 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #180 [ref=17x]
  { F(Vec)|F(Vex)                                         , 183, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #181 [ref=1x]
  { F(Vec)|F(Evex)|F(Avx512KZ_B32)                        , 165, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #182 [ref=4x]
  { F(Vec)|F(Evex)|F(Avx512KZ_B64)                        , 165, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #183 [ref=4x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_B64)                 , 162, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #184 [ref=10x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_B32)                 , 162, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #185 [ref=12x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_B64)                 , 162, 3 , CONTROL(None)   , SINGLE_REG(RO)  , 0                   , 0 }, // #186 [ref=2x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_B32)                 , 162, 3 , CONTROL(None)   , SINGLE_REG(RO)  , 0                   , 0 }, // #187 [ref=6x]
  { F(Vec)|F(Evex)|F(Avx512KZ)                            , 162, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #188 [ref=13x]
  { F(Vec)|F(Evex)|F(Avx512KZ_B32)                        , 162, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #189 [ref=16x]
  { F(Vec)|F(Evex)|F(Avx512KZ_B64)                        , 162, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #190 [ref=19x]
  { F(Vec)|F(Vex)                                         , 165, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #191 [ref=6x]
  { F(Vec)|F(Vex)                                         , 311, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #192 [ref=3x]
  { F(Vec)|F(Vex)                                         , 426, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #193 [ref=2x]
  { F(Vec)|F(Evex)|F(Avx512KZ)                            , 427, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #194 [ref=1x]
  { F(Vec)|F(Evex)|F(Avx512KZ)                            , 428, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #195 [ref=4x]
  { F(Vec)|F(Evex)|F(Avx512KZ)                            , 429, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #196 [ref=4x]
  { F(Vec)|F(Evex)|F(Avx512KZ)                            , 430, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #197 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)                     , 427, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #198 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)                     , 431, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #199 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_SAE_B64)             , 168, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #200 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_SAE_B32)             , 168, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #201 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_SAE)                 , 432, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #202 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_SAE)                 , 433, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #203 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512SAE)                    , 97 , 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #204 [ref=2x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512SAE)                    , 222, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #205 [ref=2x]
  { F(Vec)|F(Evex)|F(Avx512KZ)                            , 171, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #206 [ref=6x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_B32)                 , 174, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #207 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_ER_SAE_B32)          , 177, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #208 [ref=3x]
  { F(Vec)|F(Evex)|F(Avx512KZ_B32)                        , 313, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #209 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_ER_SAE_B64)          , 313, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #210 [ref=2x]
  { F(Vec)|F(Evex)|F(Avx512KZ_ER_SAE_B64)                 , 177, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #211 [ref=4x]
  { F(Vec)|F(Evex)|F(Avx512KZ_ER_SAE_B64)                 , 313, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #212 [ref=3x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_SAE)                 , 174, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #213 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_ER_SAE_B32)          , 174, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #214 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_SAE)                 , 180, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #215 [ref=1x]
  { F(Vec)|F(Evex)|F(Avx512KZ_ER_SAE_B32)                 , 174, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #216 [ref=2x]
  { F(Vec)|F(Evex)|F(Avx512KZ_ER_SAE_B32)                 , 177, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #217 [ref=2x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512ER_SAE)                 , 361, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #218 [ref=1x]
  { F(Vec)|F(Evex)|F(Avx512ER_SAE)                        , 361, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #219 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512ER_SAE)                 , 434, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #220 [ref=2x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_SAE)                 , 425, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #221 [ref=3x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512ER_SAE)                 , 363, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #222 [ref=1x]
  { F(Vec)|F(Evex)|F(Avx512ER_SAE)                        , 363, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #223 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_SAE_B64)             , 313, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #224 [ref=1x]
  { F(Vec)|F(Evex)|F(Avx512KZ_SAE_B64)                    , 177, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #225 [ref=3x]
  { F(Vec)|F(Evex)|F(Avx512KZ_SAE_B64)                    , 313, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #226 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_SAE_B32)             , 177, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #227 [ref=1x]
  { F(Vec)|F(Evex)|F(Avx512KZ_SAE_B32)                    , 174, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #228 [ref=2x]
  { F(Vec)|F(Evex)|F(Avx512KZ_SAE_B32)                    , 177, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #229 [ref=2x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512SAE)                    , 361, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #230 [ref=1x]
  { F(Vec)|F(Evex)|F(Avx512SAE)                           , 361, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #231 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512SAE)                    , 363, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #232 [ref=1x]
  { F(Vec)|F(Evex)|F(Avx512SAE)                           , 363, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #233 [ref=1x]
  { F(Vec)|F(Evex)|F(Avx512KZ_B32)                        , 174, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #234 [ref=1x]
  { F(Vec)|F(Evex)|F(Avx512ER_SAE)                        , 434, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #235 [ref=2x]
  { F(Vec)|F(Evex)|F(Avx512KZ)                            , 165, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #236 [ref=3x]
  { F(Vec)|F(Vex)                                         , 165, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #237 [ref=9x]
  { F(Vec)|F(Evex)|F(Avx512KZ_SAE_B64)                    , 74 , 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #238 [ref=3x]
  { F(Vec)|F(Evex)|F(Avx512KZ_SAE_B32)                    , 74 , 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #239 [ref=3x]
  { F(Vec)|F(Evex)|F(Avx512KZ)                            , 177, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #240 [ref=9x]
  { F(Vec)|F(Vex)                                         , 181, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #241 [ref=2x]
  { F(Vec)|F(Evex)|F(Avx512KZ)                            , 435, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #242 [ref=4x]
  { F(Vec)|F(Evex)|F(Avx512KZ)                            , 182, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #243 [ref=4x]
  { F(Vec)|F(Vex)|F(Evex)                                 , 367, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #244 [ref=2x]
  { F(Vec)|F(Evex)|F(Avx512KZ_SAE_B64)                    , 165, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #245 [ref=2x]
  { F(Vec)|F(Evex)|F(Avx512KZ_SAE_B32)                    , 165, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #246 [ref=2x]
  { F(Vec)|F(Evex)|F(Avx512KZ_SAE)                        , 436, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #247 [ref=4x]
  { F(Vec)|F(Evex)|F(Avx512KZ_SAE)                        , 437, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #248 [ref=4x]
  { F(Vec)|F(Vex)                                         , 130, 4 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #249 [ref=13x]
  { F(Vec)|F(Vex)                                         , 315, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #250 [ref=4x]
  { F(Vec)|F(Vex)                                         , 317, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #251 [ref=4x]
  { F(Vec)|F(Evex)|F(Avx512K_B64)                         , 438, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #252 [ref=1x]
  { F(Vec)|F(Evex)|F(Avx512K_B32)                         , 438, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #253 [ref=1x]
  { F(Vec)|F(Evex)|F(Avx512K)                             , 439, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #254 [ref=1x]
  { F(Vec)|F(Evex)|F(Avx512K)                             , 440, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #255 [ref=1x]
  { F(Vec)|F(Vex)                                         , 177, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #256 [ref=7x]
  { F(Vec)|F(Vex)                                         , 97 , 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #257 [ref=1x]
  { F(Vec)|F(Vex)                                         , 222, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #258 [ref=1x]
  { F(Vec)|F(Vsib)|F(Vex)|F(Evex)|F(Avx512K)              , 99 , 5 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #259 [ref=2x]
  { F(Vec)|F(Vsib)|F(Vex)|F(Evex)|F(Avx512K)              , 104, 5 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #260 [ref=2x]
  { F(Vsib)|F(Evex)|F(Avx512K)                            , 441, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #261 [ref=4x]
  { F(Vsib)|F(Evex)|F(Avx512K)                            , 442, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #262 [ref=4x]
  { F(Vsib)|F(Evex)|F(Avx512K)                            , 443, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #263 [ref=8x]
  { F(Vec)|F(Vsib)|F(Vex)|F(Evex)|F(Avx512K)              , 109, 5 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #264 [ref=2x]
  { F(Vec)|F(Vsib)|F(Vex)|F(Evex)|F(Avx512K)              , 134, 4 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #265 [ref=2x]
  { F(Vec)|F(Evex)|F(Avx512KZ_SAE)                        , 424, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #266 [ref=3x]
  { F(Vec)|F(Evex)|F(Avx512KZ_SAE)                        , 425, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #267 [ref=3x]
  { F(Vec)|F(Evex)|F(Avx512KZ_SAE_B64)                    , 183, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #268 [ref=2x]
  { F(Vec)|F(Evex)|F(Avx512KZ_SAE_B32)                    , 183, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #269 [ref=2x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)                     , 165, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #270 [ref=3x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)                     , 162, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #271 [ref=22x]
  { F(Vec)|F(Vex)                                         , 319, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #272 [ref=2x]
  { F(Vec)|F(Evex)|F(Avx512KZ)                            , 319, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #273 [ref=4x]
  { F(Vec)|F(Evex)|F(Avx512KZ)                            , 444, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #274 [ref=4x]
  { F(Vec)|F(Vex)|F(Evex)                                 , 437, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #275 [ref=1x]
  { F(Vec)|F(Vex)                                         , 192, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #276 [ref=1x]
  { F(Vex)                                                , 384, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #277 [ref=2x]
  { F(FixedRM)|F(Vec)|F(Vex)                              , 389, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #278 [ref=1x]
  { F(Vec)|F(Vex)                                         , 138, 4 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #279 [ref=4x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_SAE_B64)             , 162, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #280 [ref=2x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_SAE_B32)             , 162, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #281 [ref=2x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_SAE)                 , 424, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #282 [ref=2x]
  { 0                                                     , 445, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #283 [ref=4x]
  { F(FixedReg)                                           , 321, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #284 [ref=3x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)                     , 70 , 6 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #285 [ref=4x]
  { F(Vec)|F(Vex)|F(Evex)                                 , 323, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #286 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)                     , 186, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #287 [ref=1x]
  { F(Vec)|F(Vex)                                         , 70 , 4 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #288 [ref=2x]
  { F(Vec)|F(Evex)|F(Avx512KZ)                            , 70 , 6 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #289 [ref=6x]
  { F(Vec)|F(Vex)|F(Evex)                                 , 200, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #290 [ref=2x]
  { F(Vec)|F(Vex)|F(Evex)                                 , 325, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #291 [ref=4x]
  { F(Vec)|F(Vex)                                         , 446, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #292 [ref=3x]
  { F(Vec)|F(Vex)|F(Evex)                                 , 189, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #293 [ref=3x]
  { F(Vec)|F(Vex)|F(Evex)                                 , 192, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #294 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)                                 , 195, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #295 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)                     , 198, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #296 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)                     , 177, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #297 [ref=5x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)                     , 201, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #298 [ref=1x]
  { 0                                                     , 327, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #299 [ref=1x]
  { 0                                                     , 329, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #300 [ref=1x]
  { F(Vec)|F(Vex)                                         , 162, 2 , CONTROL(None)   , SINGLE_REG(RO)  , 0                   , 0 }, // #301 [ref=2x]
  { F(Vec)|F(Evex)|F(Avx512KZ_B32)                        , 162, 3 , CONTROL(None)   , SINGLE_REG(RO)  , 0                   , 0 }, // #302 [ref=2x]
  { F(Vec)|F(Vex)                                         , 162, 2 , CONTROL(None)   , SINGLE_REG(WO)  , 0                   , 0 }, // #303 [ref=2x]
  { F(Vec)|F(Evex)|F(Avx512KZ_B32)                        , 162, 3 , CONTROL(None)   , SINGLE_REG(WO)  , 0                   , 0 }, // #304 [ref=2x]
  { F(Vec)|F(Evex)|F(Avx512KZ_B64)                        , 162, 3 , CONTROL(None)   , SINGLE_REG(WO)  , 0                   , 0 }, // #305 [ref=2x]
  { F(Vec)|F(Evex)|F(Avx512KZ_B64)                        , 162, 3 , CONTROL(None)   , SINGLE_REG(RO)  , 0                   , 0 }, // #306 [ref=2x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)                     , 447, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #307 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)                     , 448, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #308 [ref=1x]
  { F(Vec)|F(Evex)                                        , 449, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #309 [ref=6x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)                     , 204, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #310 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)                     , 450, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #311 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)                                 , 165, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #312 [ref=1x]
  { F(Vec)|F(Evex)|F(Avx512K)                             , 207, 3 , CONTROL(None)   , SINGLE_REG(WO)  , 0                   , 0 }, // #313 [ref=2x]
  { F(Vec)|F(Evex)|F(Avx512K_B32)                         , 207, 3 , CONTROL(None)   , SINGLE_REG(WO)  , 0                   , 0 }, // #314 [ref=2x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512K)                      , 210, 3 , CONTROL(None)   , SINGLE_REG(WO)  , 0                   , 0 }, // #315 [ref=4x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512K_B32)                  , 210, 3 , CONTROL(None)   , SINGLE_REG(WO)  , 0                   , 0 }, // #316 [ref=2x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512K_B64)                  , 210, 3 , CONTROL(None)   , SINGLE_REG(WO)  , 0                   , 0 }, // #317 [ref=2x]
  { F(FixedReg)|F(Vec)|F(Vex)                             , 402, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #318 [ref=1x]
  { F(FixedReg)|F(Vec)|F(Vex)                             , 403, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #319 [ref=1x]
  { F(FixedReg)|F(Vec)|F(Vex)                             , 404, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #320 [ref=1x]
  { F(FixedReg)|F(Vec)|F(Vex)                             , 405, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #321 [ref=1x]
  { F(Vec)|F(Evex)|F(Avx512K_B64)                         , 207, 3 , CONTROL(None)   , SINGLE_REG(WO)  , 0                   , 0 }, // #322 [ref=4x]
  { F(Vec)|F(Evex)|F(Avx512KZ_B32)                        , 177, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #323 [ref=6x]
  { F(Vec)|F(Vex)                                         , 166, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #324 [ref=2x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_B32)                 , 163, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #325 [ref=1x]
  { F(Vec)|F(Vex)                                         , 142, 4 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #326 [ref=2x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_B64)                 , 76 , 6 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #327 [ref=2x]
  { F(Vec)|F(Vex)                                         , 146, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #328 [ref=1x]
  { F(Vec)|F(Vex)                                         , 147, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #329 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_B64)                 , 146, 4 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #330 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)                                 , 406, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #331 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)                                 , 407, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #332 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)                                 , 451, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #333 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)                     , 452, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #334 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)                     , 453, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #335 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)                     , 454, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #336 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)                     , 455, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #337 [ref=1x]
  { F(Vec)|F(Evex)|F(Avx512KZ_B64)                        , 177, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #338 [ref=4x]
  { F(Vec)|F(Vex)                                         , 311, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #339 [ref=12x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)                     , 162, 3 , CONTROL(None)   , SINGLE_REG(RO)  , 0                   , 0 }, // #340 [ref=8x]
  { F(Vec)|F(Evex)                                        , 456, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #341 [ref=4x]
  { F(Vec)|F(Evex)|F(Avx512KZ)                            , 213, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #342 [ref=6x]
  { F(Vec)|F(Evex)|F(Avx512KZ)                            , 216, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #343 [ref=9x]
  { F(Vec)|F(Evex)|F(Avx512KZ)                            , 219, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #344 [ref=3x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)                     , 222, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #345 [ref=4x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)                     , 225, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #346 [ref=2x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)                     , 174, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #347 [ref=6x]
  { F(Vec)|F(Vex)                                         , 130, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #348 [ref=1x]
  { F(Vec)|F(Evex)|F(Avx512KZ_B32)                        , 183, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #349 [ref=3x]
  { F(Vec)|F(Evex)|F(Avx512KZ_B64)                        , 183, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #350 [ref=3x]
  { F(Vec)|F(Vex)                                         , 331, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #351 [ref=4x]
  { F(Vec)|F(Vsib)|F(Evex)|F(Avx512K)                     , 228, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #352 [ref=3x]
  { F(Vec)|F(Vsib)|F(Evex)|F(Avx512K)                     , 333, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #353 [ref=2x]
  { F(Vec)|F(Vsib)|F(Evex)|F(Avx512K)                     , 231, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #354 [ref=2x]
  { F(Vec)|F(Vex)                                         , 335, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #355 [ref=8x]
  { F(Vec)|F(Evex)|F(Avx512K)                             , 234, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #356 [ref=5x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_B32)                 , 183, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #357 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)                     , 183, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #358 [ref=2x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_B32)                 , 82 , 6 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #359 [ref=3x]
  { F(Vec)|F(Vex)|F(Evex)                                 , 183, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #360 [ref=2x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_B64)                 , 82 , 6 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #361 [ref=2x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)                     , 82 , 6 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #362 [ref=3x]
  { F(Vec)|F(Evex)|F(Avx512KZ_B64)                        , 88 , 6 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #363 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)                     , 162, 3 , CONTROL(None)   , SINGLE_REG(WO)  , 0                   , 0 }, // #364 [ref=6x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_B32)                 , 162, 3 , CONTROL(None)   , SINGLE_REG(WO)  , 0                   , 0 }, // #365 [ref=2x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_B64)                 , 162, 3 , CONTROL(None)   , SINGLE_REG(WO)  , 0                   , 0 }, // #366 [ref=2x]
  { F(Vec)|F(Evex)|F(Avx512K_B32)                         , 234, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #367 [ref=2x]
  { F(Vec)|F(Evex)|F(Avx512K_B64)                         , 234, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #368 [ref=2x]
  { F(Vec)|F(Evex)|F(Avx512KZ)                            , 424, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #369 [ref=2x]
  { F(Vec)|F(Evex)|F(Avx512KZ)                            , 425, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #370 [ref=2x]
  { F(Vec)|F(Vex)                                         , 425, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #371 [ref=2x]
  { F(Vec)|F(Evex)|F(Avx512KZ)                            , 436, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #372 [ref=1x]
  { F(Vec)|F(Evex)|F(Avx512KZ)                            , 437, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #373 [ref=1x]
  { F(Vec)|F(Vex)                                         , 183, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #374 [ref=2x]
  { F(Vec)|F(Vex)                                         , 436, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #375 [ref=1x]
  { F(Vec)|F(Vex)                                         , 437, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #376 [ref=1x]
  { F(Vec)|F(Evex)|F(Avx512KZ_ER_SAE_B64)                 , 162, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #377 [ref=1x]
  { F(Vec)|F(Evex)|F(Avx512KZ_ER_SAE_B32)                 , 162, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #378 [ref=1x]
  { F(Vec)|F(Evex)|F(Avx512KZ_ER_SAE)                     , 424, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #379 [ref=1x]
  { F(Vec)|F(Evex)|F(Avx512KZ_ER_SAE)                     , 425, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #380 [ref=1x]
  { F(Vec)|F(Vsib)|F(Evex)|F(Avx512K)                     , 337, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #381 [ref=1x]
  { F(Vec)|F(Evex)|F(Avx512KZ_B32)                        , 166, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #382 [ref=2x]
  { F(Vec)|F(Evex)|F(Avx512KZ_B64)                        , 166, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #383 [ref=2x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_B32)                 , 165, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #384 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_B64)                 , 165, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #385 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_ER_SAE_B64)          , 177, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #386 [ref=1x]
  { F(Vec)|F(Vex)                                         , 255, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #387 [ref=2x]
  { F(Lock)|F(XAcquire)|F(XRelease)                       , 49 , 4 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #388 [ref=1x]
  { 0                                                     , 457, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #389 [ref=1x]
  { F(Lock)                                               , 49 , 8 , CONTROL(None)   , SINGLE_REG(RO)  , 0                   , 0 }, // #390 [ref=1x]
  { F(FixedReg)                                           , 458, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #391 [ref=6x]
  { F(FixedReg)                                           , 459, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }  // #392 [ref=6x]
};
#undef SPECIAL_CASE
#undef SINGLE_REG
#undef CONTROL
#undef F
// ----------------------------------------------------------------------------
// ${InstCommonTable:End}

// ============================================================================
// [asmjit::x86::InstDB - ExecutionInfoTable]
// ============================================================================

// ${InstExecutionTable:Begin}
// ------------------- Automatically generated, do not edit -------------------
#define OP_FLAG(VAL) uint32_t(InstInfo::kOperation##VAL)
#define FEATURE(VAL) uint32_t(Features::k##VAL)
#define SPECIAL(VAL) uint32_t(kSpecialReg_##VAL)
const InstDB::ExecutionInfo InstDB::_executionInfoTable[] = {
  { 0, { 0 }, 0, 0 }, // #0 [ref=70x]
  { 0, { 0 }, 0, SPECIAL(FLAGS_AF) | SPECIAL(FLAGS_CF) | SPECIAL(FLAGS_OF) | SPECIAL(FLAGS_PF) | SPECIAL(FLAGS_SF) | SPECIAL(FLAGS_ZF) }, // #1 [ref=34x]
  { 0, { 0 }, SPECIAL(FLAGS_CF), SPECIAL(FLAGS_AF) | SPECIAL(FLAGS_CF) | SPECIAL(FLAGS_OF) | SPECIAL(FLAGS_PF) | SPECIAL(FLAGS_SF) | SPECIAL(FLAGS_ZF) }, // #2 [ref=2x]
  { 0, { FEATURE(ADX) }, SPECIAL(FLAGS_CF), SPECIAL(FLAGS_CF) }, // #3 [ref=1x]
  { 0, { FEATURE(SSE2) }, 0, 0 }, // #4 [ref=65x]
  { 0, { FEATURE(SSE) }, 0, 0 }, // #5 [ref=44x]
  { 0, { FEATURE(SSE3) }, 0, 0 }, // #6 [ref=11x]
  { 0, { FEATURE(ADX) }, SPECIAL(FLAGS_OF), SPECIAL(FLAGS_OF) }, // #7 [ref=1x]
  { 0, { FEATURE(AESNI) }, 0, 0 }, // #8 [ref=6x]
  { 0, { FEATURE(BMI) }, 0, SPECIAL(FLAGS_AF) | SPECIAL(FLAGS_CF) | SPECIAL(FLAGS_OF) | SPECIAL(FLAGS_PF) | SPECIAL(FLAGS_SF) | SPECIAL(FLAGS_ZF) }, // #9 [ref=6x]
  { 0, { 0 }, 0, SPECIAL(FLAGS_ZF) }, // #10 [ref=5x]
  { 0, { FEATURE(TBM) }, 0, 0 }, // #11 [ref=9x]
  { 0, { FEATURE(SSE4_1) }, 0, 0 }, // #12 [ref=47x]
  { 0, { FEATURE(MPX) }, 0, 0 }, // #13 [ref=7x]
  { 0, { 0 }, 0, SPECIAL(FLAGS_AF) | SPECIAL(FLAGS_CF) | SPECIAL(FLAGS_OF) | SPECIAL(FLAGS_PF) | SPECIAL(FLAGS_SF) }, // #14 [ref=1x]
  { 0, { 0 }, 0, SPECIAL(FLAGS_AF) | SPECIAL(FLAGS_CF) | SPECIAL(FLAGS_PF) }, // #15 [ref=3x]
  { 0, { FEATURE(BMI2) }, 0, SPECIAL(FLAGS_AF) | SPECIAL(FLAGS_CF) | SPECIAL(FLAGS_OF) | SPECIAL(FLAGS_PF) | SPECIAL(FLAGS_SF) | SPECIAL(FLAGS_ZF) }, // #16 [ref=1x]
  { 0, { FEATURE(SMAP) }, 0, SPECIAL(FLAGS_Other) }, // #17 [ref=2x]
  { 0, { 0 }, 0, SPECIAL(FLAGS_CF) }, // #18 [ref=2x]
  { 0, { 0 }, 0, SPECIAL(FLAGS_DF) }, // #19 [ref=2x]
  { 0, { FEATURE(CLDEMOTE) }, 0, 0 }, // #20 [ref=1x]
  { 0, { FEATURE(CLFLUSH) }, 0, 0 }, // #21 [ref=1x]
  { 0, { FEATURE(CLFLUSHOPT) }, 0, 0 }, // #22 [ref=1x]
  { 0, { FEATURE(SVM) }, 0, 0 }, // #23 [ref=6x]
  { 0, { 0 }, 0, SPECIAL(FLAGS_IF) }, // #24 [ref=2x]
  { 0, { FEATURE(CLWB) }, 0, 0 }, // #25 [ref=1x]
  { 0, { FEATURE(CLZERO) }, 0, 0 }, // #26 [ref=1x]
  { 0, { 0 }, SPECIAL(FLAGS_CF), SPECIAL(FLAGS_CF) }, // #27 [ref=1x]
  { 0, { FEATURE(CMOV) }, SPECIAL(FLAGS_CF) | SPECIAL(FLAGS_ZF), 0 }, // #28 [ref=4x]
  { 0, { FEATURE(CMOV) }, SPECIAL(FLAGS_CF), 0 }, // #29 [ref=6x]
  { 0, { FEATURE(CMOV) }, SPECIAL(FLAGS_ZF), 0 }, // #30 [ref=4x]
  { 0, { FEATURE(CMOV) }, SPECIAL(FLAGS_OF) | SPECIAL(FLAGS_SF) | SPECIAL(FLAGS_ZF), 0 }, // #31 [ref=4x]
  { 0, { FEATURE(CMOV) }, SPECIAL(FLAGS_OF) | SPECIAL(FLAGS_SF), 0 }, // #32 [ref=4x]
  { 0, { FEATURE(CMOV) }, SPECIAL(FLAGS_OF), 0 }, // #33 [ref=2x]
  { 0, { FEATURE(CMOV) }, SPECIAL(FLAGS_PF), 0 }, // #34 [ref=4x]
  { 0, { FEATURE(CMOV) }, SPECIAL(FLAGS_SF), 0 }, // #35 [ref=2x]
  { 0, { 0 }, SPECIAL(FLAGS_DF), SPECIAL(FLAGS_AF) | SPECIAL(FLAGS_CF) | SPECIAL(FLAGS_PF) | SPECIAL(FLAGS_SF) | SPECIAL(FLAGS_ZF) }, // #36 [ref=2x]
  { 0, { FEATURE(I486) }, 0, SPECIAL(FLAGS_AF) | SPECIAL(FLAGS_CF) | SPECIAL(FLAGS_OF) | SPECIAL(FLAGS_PF) | SPECIAL(FLAGS_SF) | SPECIAL(FLAGS_ZF) }, // #37 [ref=1x]
  { 0, { FEATURE(CMPXCHG16B) }, 0, SPECIAL(FLAGS_ZF) }, // #38 [ref=1x]
  { 0, { FEATURE(CMPXCHG8B) }, 0, 0 }, // #39 [ref=1x]
  { 0, { FEATURE(SSE2) }, 0, SPECIAL(FLAGS_AF) | SPECIAL(FLAGS_CF) | SPECIAL(FLAGS_OF) | SPECIAL(FLAGS_PF) | SPECIAL(FLAGS_SF) | SPECIAL(FLAGS_ZF) }, // #40 [ref=2x]
  { 0, { FEATURE(SSE) }, 0, SPECIAL(FLAGS_AF) | SPECIAL(FLAGS_CF) | SPECIAL(FLAGS_OF) | SPECIAL(FLAGS_PF) | SPECIAL(FLAGS_SF) | SPECIAL(FLAGS_ZF) }, // #41 [ref=2x]
  { 0, { FEATURE(I486) }, 0, 0 }, // #42 [ref=4x]
  { 0, { FEATURE(SSE4_2) }, 0, 0 }, // #43 [ref=2x]
  { 0, { 0 }, 0, SPECIAL(FLAGS_AF) | SPECIAL(FLAGS_OF) | SPECIAL(FLAGS_PF) | SPECIAL(FLAGS_SF) | SPECIAL(FLAGS_ZF) }, // #44 [ref=2x]
  { 0, { FEATURE(MMX) }, 0, 0 }, // #45 [ref=1x]
  { 0, { FEATURE(SSE4A) }, 0, 0 }, // #46 [ref=4x]
  { 0, { 0 }, 0, SPECIAL(X87SW_C0) | SPECIAL(X87SW_C1) | SPECIAL(X87SW_C2) | SPECIAL(X87SW_C3) }, // #47 [ref=80x]
  { 0, { FEATURE(CMOV) }, 0, SPECIAL(X87SW_C0) | SPECIAL(X87SW_C1) | SPECIAL(X87SW_C2) | SPECIAL(X87SW_C3) }, // #48 [ref=8x]
  { 0, { 0 }, 0, SPECIAL(FLAGS_CF) | SPECIAL(FLAGS_PF) | SPECIAL(FLAGS_ZF) | SPECIAL(X87SW_C1) }, // #49 [ref=4x]
  { 0, { FEATURE(3DNOW) }, 0, 0 }, // #50 [ref=21x]
  { 0, { FEATURE(SSE3) }, 0, SPECIAL(X87SW_C0) | SPECIAL(X87SW_C1) | SPECIAL(X87SW_C2) | SPECIAL(X87SW_C3) }, // #51 [ref=1x]
  { 0, { FEATURE(FXSR) }, 0, SPECIAL(X87SW_C0) | SPECIAL(X87SW_C1) | SPECIAL(X87SW_C2) | SPECIAL(X87SW_C3) }, // #52 [ref=2x]
  { 0, { FEATURE(FXSR) }, 0, 0 }, // #53 [ref=2x]
  { 0, { FEATURE(SMX) }, 0, 0 }, // #54 [ref=1x]
  { 0, { FEATURE(GFNI) }, 0, 0 }, // #55 [ref=3x]
  { 0, { 0 }, SPECIAL(FLAGS_OF), 0 }, // #56 [ref=5x]
  { 0, { FEATURE(VMX) }, 0, 0 }, // #57 [ref=12x]
  { 0, { 0 }, SPECIAL(FLAGS_CF) | SPECIAL(FLAGS_ZF), 0 }, // #58 [ref=8x]
  { 0, { 0 }, SPECIAL(FLAGS_CF), 0 }, // #59 [ref=12x]
  { 0, { 0 }, SPECIAL(FLAGS_ZF), 0 }, // #60 [ref=10x]
  { 0, { 0 }, SPECIAL(FLAGS_OF) | SPECIAL(FLAGS_SF) | SPECIAL(FLAGS_ZF), 0 }, // #61 [ref=8x]
  { 0, { 0 }, SPECIAL(FLAGS_OF) | SPECIAL(FLAGS_SF), 0 }, // #62 [ref=8x]
  { 0, { 0 }, SPECIAL(FLAGS_PF), 0 }, // #63 [ref=8x]
  { 0, { 0 }, SPECIAL(FLAGS_SF), 0 }, // #64 [ref=4x]
  { 0, { FEATURE(AVX512_DQ) }, 0, 0 }, // #65 [ref=23x]
  { 0, { FEATURE(AVX512_BW) }, 0, 0 }, // #66 [ref=22x]
  { 0, { FEATURE(AVX512_F) }, 0, 0 }, // #67 [ref=37x]
  { 0, { FEATURE(AVX512_DQ) }, 0, SPECIAL(FLAGS_AF) | SPECIAL(FLAGS_CF) | SPECIAL(FLAGS_OF) | SPECIAL(FLAGS_PF) | SPECIAL(FLAGS_SF) | SPECIAL(FLAGS_ZF) }, // #68 [ref=3x]
  { 0, { FEATURE(AVX512_BW) }, 0, SPECIAL(FLAGS_AF) | SPECIAL(FLAGS_CF) | SPECIAL(FLAGS_OF) | SPECIAL(FLAGS_PF) | SPECIAL(FLAGS_SF) | SPECIAL(FLAGS_ZF) }, // #69 [ref=4x]
  { 0, { FEATURE(AVX512_F) }, 0, SPECIAL(FLAGS_AF) | SPECIAL(FLAGS_CF) | SPECIAL(FLAGS_OF) | SPECIAL(FLAGS_PF) | SPECIAL(FLAGS_SF) | SPECIAL(FLAGS_ZF) }, // #70 [ref=1x]
  { 0, { FEATURE(LAHFSAHF) }, SPECIAL(FLAGS_AF) | SPECIAL(FLAGS_CF) | SPECIAL(FLAGS_PF) | SPECIAL(FLAGS_SF) | SPECIAL(FLAGS_ZF), 0 }, // #71 [ref=1x]
  { 0, { FEATURE(LWP) }, 0, 0 }, // #72 [ref=4x]
  { 0, { FEATURE(LZCNT) }, 0, SPECIAL(FLAGS_AF) | SPECIAL(FLAGS_CF) | SPECIAL(FLAGS_OF) | SPECIAL(FLAGS_PF) | SPECIAL(FLAGS_SF) | SPECIAL(FLAGS_ZF) }, // #73 [ref=1x]
  { 0, { FEATURE(MMX2) }, 0, 0 }, // #74 [ref=8x]
  { 0, { FEATURE(MONITOR) }, 0, 0 }, // #75 [ref=2x]
  { 0, { FEATURE(MONITORX) }, 0, 0 }, // #76 [ref=2x]
  { 0, { FEATURE(MOVBE) }, 0, 0 }, // #77 [ref=1x]
  { 0, { FEATURE(MMX), FEATURE(SSE2) }, 0, 0 }, // #78 [ref=46x]
  { 0, { FEATURE(MOVDIR64B) }, 0, 0 }, // #79 [ref=1x]
  { 0, { FEATURE(MOVDIRI) }, 0, 0 }, // #80 [ref=1x]
  { 0, { FEATURE(BMI2) }, 0, 0 }, // #81 [ref=7x]
  { 0, { 0 }, 0, SPECIAL(FLAGS_AF) | SPECIAL(FLAGS_CF) | SPECIAL(FLAGS_PF) | SPECIAL(FLAGS_ZF) }, // #82 [ref=1x]
  { 0, { FEATURE(SSSE3) }, 0, 0 }, // #83 [ref=15x]
  { 0, { FEATURE(MMX2), FEATURE(SSE2) }, 0, 0 }, // #84 [ref=10x]
  { 0, { FEATURE(PCLMULQDQ) }, 0, 0 }, // #85 [ref=1x]
  { 0, { FEATURE(SSE4_2) }, 0, SPECIAL(FLAGS_AF) | SPECIAL(FLAGS_CF) | SPECIAL(FLAGS_OF) | SPECIAL(FLAGS_PF) | SPECIAL(FLAGS_SF) | SPECIAL(FLAGS_ZF) }, // #86 [ref=4x]
  { 0, { FEATURE(PCOMMIT) }, 0, 0 }, // #87 [ref=1x]
  { 0, { FEATURE(MMX2), FEATURE(SSE2), FEATURE(SSE4_1) }, 0, 0 }, // #88 [ref=1x]
  { 0, { FEATURE(3DNOW2) }, 0, 0 }, // #89 [ref=5x]
  { 0, { FEATURE(GEODE) }, 0, 0 }, // #90 [ref=2x]
  { 0, { FEATURE(POPCNT) }, 0, SPECIAL(FLAGS_AF) | SPECIAL(FLAGS_CF) | SPECIAL(FLAGS_OF) | SPECIAL(FLAGS_PF) | SPECIAL(FLAGS_SF) | SPECIAL(FLAGS_ZF) }, // #91 [ref=1x]
  { 0, { FEATURE(PREFETCHW) }, 0, SPECIAL(FLAGS_AF) | SPECIAL(FLAGS_CF) | SPECIAL(FLAGS_OF) | SPECIAL(FLAGS_PF) | SPECIAL(FLAGS_SF) | SPECIAL(FLAGS_ZF) }, // #92 [ref=1x]
  { 0, { FEATURE(PREFETCHWT1) }, 0, SPECIAL(FLAGS_AF) | SPECIAL(FLAGS_CF) | SPECIAL(FLAGS_OF) | SPECIAL(FLAGS_PF) | SPECIAL(FLAGS_SF) | SPECIAL(FLAGS_ZF) }, // #93 [ref=1x]
  { 0, { FEATURE(SSE4_1) }, 0, SPECIAL(FLAGS_AF) | SPECIAL(FLAGS_CF) | SPECIAL(FLAGS_OF) | SPECIAL(FLAGS_PF) | SPECIAL(FLAGS_SF) | SPECIAL(FLAGS_ZF) }, // #94 [ref=1x]
  { 0, { 0 }, SPECIAL(FLAGS_CF) | SPECIAL(FLAGS_OF), SPECIAL(FLAGS_CF) | SPECIAL(FLAGS_OF) }, // #95 [ref=2x]
  { 0, { FEATURE(FSGSBASE) }, 0, 0 }, // #96 [ref=4x]
  { 0, { FEATURE(MSR) }, SPECIAL(MSR), 0 }, // #97 [ref=1x]
  { 0, { FEATURE(RDPID) }, 0, 0 }, // #98 [ref=1x]
  { 0, { FEATURE(RDRAND) }, 0, SPECIAL(FLAGS_AF) | SPECIAL(FLAGS_CF) | SPECIAL(FLAGS_OF) | SPECIAL(FLAGS_PF) | SPECIAL(FLAGS_SF) | SPECIAL(FLAGS_ZF) }, // #99 [ref=1x]
  { 0, { FEATURE(RDSEED) }, 0, SPECIAL(FLAGS_AF) | SPECIAL(FLAGS_CF) | SPECIAL(FLAGS_OF) | SPECIAL(FLAGS_PF) | SPECIAL(FLAGS_SF) | SPECIAL(FLAGS_ZF) }, // #100 [ref=1x]
  { 0, { FEATURE(RDTSC) }, 0, 0 }, // #101 [ref=1x]
  { 0, { FEATURE(RDTSCP) }, 0, 0 }, // #102 [ref=1x]
  { 0, { 0 }, 0, SPECIAL(FLAGS_CF) | SPECIAL(FLAGS_OF) }, // #103 [ref=2x]
  { 0, { FEATURE(LAHFSAHF) }, 0, SPECIAL(FLAGS_AF) | SPECIAL(FLAGS_CF) | SPECIAL(FLAGS_PF) | SPECIAL(FLAGS_SF) | SPECIAL(FLAGS_ZF) }, // #104 [ref=1x]
  { 0, { FEATURE(SHA) }, 0, 0 }, // #105 [ref=7x]
  { 0, { FEATURE(SKINIT) }, 0, 0 }, // #106 [ref=2x]
  { 0, { FEATURE(AVX512_4FMAPS) }, 0, 0 }, // #107 [ref=4x]
  { 0, { FEATURE(AVX), FEATURE(AVX512_F), FEATURE(AVX512_VL) }, 0, 0 }, // #108 [ref=46x]
  { 0, { FEATURE(AVX), FEATURE(AVX512_F) }, 0, 0 }, // #109 [ref=32x]
  { 0, { FEATURE(AVX) }, 0, 0 }, // #110 [ref=37x]
  { 0, { FEATURE(AESNI), FEATURE(AVX), FEATURE(AVX512_F), FEATURE(AVX512_VL), FEATURE(VAES) }, 0, 0 }, // #111 [ref=4x]
  { 0, { FEATURE(AESNI), FEATURE(AVX) }, 0, 0 }, // #112 [ref=2x]
  { 0, { FEATURE(AVX512_F), FEATURE(AVX512_VL) }, 0, 0 }, // #113 [ref=112x]
  { 0, { FEATURE(AVX), FEATURE(AVX512_DQ), FEATURE(AVX512_VL) }, 0, 0 }, // #114 [ref=8x]
  { 0, { FEATURE(AVX512_BW), FEATURE(AVX512_VL) }, 0, 0 }, // #115 [ref=26x]
  { 0, { FEATURE(AVX512_DQ), FEATURE(AVX512_VL) }, 0, 0 }, // #116 [ref=30x]
  { 0, { FEATURE(AVX2) }, 0, 0 }, // #117 [ref=9x]
  { 0, { FEATURE(AVX), FEATURE(AVX2), FEATURE(AVX512_F), FEATURE(AVX512_VL) }, 0, 0 }, // #118 [ref=39x]
  { 0, { FEATURE(AVX), FEATURE(AVX512_F) }, 0, SPECIAL(FLAGS_AF) | SPECIAL(FLAGS_CF) | SPECIAL(FLAGS_OF) | SPECIAL(FLAGS_PF) | SPECIAL(FLAGS_SF) | SPECIAL(FLAGS_ZF) }, // #119 [ref=4x]
  { 0, { FEATURE(AVX512_BF16), FEATURE(AVX512_VL) }, 0, 0 }, // #120 [ref=3x]
  { 0, { FEATURE(AVX512_F), FEATURE(AVX512_VL), FEATURE(F16C) }, 0, 0 }, // #121 [ref=2x]
  { 0, { FEATURE(AVX512_ERI) }, 0, 0 }, // #122 [ref=10x]
  { 0, { FEATURE(AVX512_F), FEATURE(AVX512_VL), FEATURE(FMA) }, 0, 0 }, // #123 [ref=36x]
  { 0, { FEATURE(AVX512_F), FEATURE(FMA) }, 0, 0 }, // #124 [ref=24x]
  { 0, { FEATURE(FMA4) }, 0, 0 }, // #125 [ref=20x]
  { 0, { FEATURE(XOP) }, 0, 0 }, // #126 [ref=55x]
  { 0, { FEATURE(AVX2), FEATURE(AVX512_F), FEATURE(AVX512_VL) }, 0, 0 }, // #127 [ref=17x]
  { 0, { FEATURE(AVX512_PFI) }, 0, 0 }, // #128 [ref=16x]
  { 0, { FEATURE(AVX), FEATURE(AVX512_F), FEATURE(AVX512_VL), FEATURE(GFNI) }, 0, 0 }, // #129 [ref=3x]
  { 0, { FEATURE(AVX), FEATURE(AVX2) }, 0, 0 }, // #130 [ref=17x]
  { 0, { FEATURE(AVX512_4VNNIW) }, 0, 0 }, // #131 [ref=2x]
  { 0, { FEATURE(AVX), FEATURE(AVX2), FEATURE(AVX512_BW), FEATURE(AVX512_VL) }, 0, 0 }, // #132 [ref=54x]
  { 0, { FEATURE(AVX2), FEATURE(AVX512_BW), FEATURE(AVX512_VL) }, 0, 0 }, // #133 [ref=2x]
  { 0, { FEATURE(AVX512_CDI), FEATURE(AVX512_VL) }, 0, 0 }, // #134 [ref=6x]
  { 0, { FEATURE(AVX), FEATURE(AVX512_F), FEATURE(AVX512_VL), FEATURE(PCLMULQDQ), FEATURE(VPCLMULQDQ) }, 0, 0 }, // #135 [ref=1x]
  { 0, { FEATURE(AVX) }, 0, SPECIAL(FLAGS_AF) | SPECIAL(FLAGS_CF) | SPECIAL(FLAGS_OF) | SPECIAL(FLAGS_PF) | SPECIAL(FLAGS_SF) | SPECIAL(FLAGS_ZF) }, // #136 [ref=7x]
  { 0, { FEATURE(AVX512_VBMI2), FEATURE(AVX512_VL) }, 0, 0 }, // #137 [ref=16x]
  { 0, { FEATURE(AVX512_VL), FEATURE(AVX512_VNNI) }, 0, 0 }, // #138 [ref=4x]
  { 0, { FEATURE(AVX512_VBMI), FEATURE(AVX512_VL) }, 0, 0 }, // #139 [ref=4x]
  { 0, { FEATURE(AVX), FEATURE(AVX512_BW) }, 0, 0 }, // #140 [ref=4x]
  { 0, { FEATURE(AVX), FEATURE(AVX512_DQ) }, 0, 0 }, // #141 [ref=4x]
  { 0, { FEATURE(AVX512_IFMA), FEATURE(AVX512_VL) }, 0, 0 }, // #142 [ref=2x]
  { 0, { FEATURE(AVX512_BITALG), FEATURE(AVX512_VL) }, 0, 0 }, // #143 [ref=3x]
  { 0, { FEATURE(AVX512_VL), FEATURE(AVX512_VPOPCNTDQ) }, 0, 0 }, // #144 [ref=2x]
  { 0, { FEATURE(WBNOINVD) }, 0, 0 }, // #145 [ref=1x]
  { 0, { FEATURE(MSR) }, 0, SPECIAL(MSR) }, // #146 [ref=1x]
  { 0, { FEATURE(RTM) }, 0, 0 }, // #147 [ref=3x]
  { 0, { FEATURE(I486) }, 0, SPECIAL(FLAGS_AF) | SPECIAL(FLAGS_CF) | SPECIAL(FLAGS_PF) | SPECIAL(FLAGS_SF) | SPECIAL(FLAGS_ZF) }, // #148 [ref=1x]
  { 0, { FEATURE(XSAVE) }, SPECIAL(XCR), 0 }, // #149 [ref=5x]
  { 0, { FEATURE(XSAVES) }, SPECIAL(XCR), 0 }, // #150 [ref=4x]
  { 0, { FEATURE(XSAVEC) }, SPECIAL(XCR), 0 }, // #151 [ref=2x]
  { 0, { FEATURE(XSAVEOPT) }, SPECIAL(XCR), 0 }, // #152 [ref=2x]
  { 0, { FEATURE(XSAVE) }, 0, SPECIAL(XCR) }, // #153 [ref=1x]
  { 0, { FEATURE(TSX) }, 0, SPECIAL(FLAGS_AF) | SPECIAL(FLAGS_CF) | SPECIAL(FLAGS_OF) | SPECIAL(FLAGS_PF) | SPECIAL(FLAGS_SF) | SPECIAL(FLAGS_ZF) }  // #154 [ref=1x]
};
#undef SPECIAL
#undef FEATURE
#undef OP_FLAG
// ----------------------------------------------------------------------------
// ${InstExecutionTable:End}

// ============================================================================
// [asmjit::Inst - NameData]
// ============================================================================

#ifndef ASMJIT_DISABLE_TEXT
// ${NameData:Begin}
// ------------------- Automatically generated, do not edit -------------------
const char InstDB::_nameData[] =
  "\0" "aaa\0" "aad\0" "aam\0" "aas\0" "adc\0" "adcx\0" "adox\0" "arpl\0" "bextr\0" "blcfill\0" "blci\0" "blcic\0"
  "blcmsk\0" "blcs\0" "blsfill\0" "blsi\0" "blsic\0" "blsmsk\0" "blsr\0" "bndcl\0" "bndcn\0" "bndcu\0" "bndldx\0"
  "bndmk\0" "bndmov\0" "bndstx\0" "bound\0" "bsf\0" "bsr\0" "bswap\0" "bt\0" "btc\0" "btr\0" "bts\0" "bzhi\0" "cbw\0"
  "cdq\0" "cdqe\0" "clac\0" "clc\0" "cld\0" "cldemote\0" "clflush\0" "clflushopt\0" "clgi\0" "cli\0" "clts\0" "clwb\0"
  "clzero\0" "cmc\0" "cmova\0" "cmovae\0" "cmovc\0" "cmovg\0" "cmovge\0" "cmovl\0" "cmovle\0" "cmovna\0" "cmovnae\0"
  "cmovnc\0" "cmovng\0" "cmovnge\0" "cmovnl\0" "cmovnle\0" "cmovno\0" "cmovnp\0" "cmovns\0" "cmovnz\0" "cmovo\0"
  "cmovp\0" "cmovpe\0" "cmovpo\0" "cmovs\0" "cmovz\0" "cmp\0" "cmps\0" "cmpxchg\0" "cmpxchg16b\0" "cmpxchg8b\0"
  "cpuid\0" "cqo\0" "crc32\0" "cvtpd2pi\0" "cvtpi2pd\0" "cvtpi2ps\0" "cvtps2pi\0" "cvttpd2pi\0" "cvttps2pi\0" "cwd\0"
  "cwde\0" "daa\0" "das\0" "f2xm1\0" "fabs\0" "faddp\0" "fbld\0" "fbstp\0" "fchs\0" "fclex\0" "fcmovb\0" "fcmovbe\0"
  "fcmove\0" "fcmovnb\0" "fcmovnbe\0" "fcmovne\0" "fcmovnu\0" "fcmovu\0" "fcom\0" "fcomi\0" "fcomip\0" "fcomp\0"
  "fcompp\0" "fcos\0" "fdecstp\0" "fdiv\0" "fdivp\0" "fdivr\0" "fdivrp\0" "femms\0" "ffree\0" "fiadd\0" "ficom\0"
  "ficomp\0" "fidiv\0" "fidivr\0" "fild\0" "fimul\0" "fincstp\0" "finit\0" "fist\0" "fistp\0" "fisttp\0" "fisub\0"
  "fisubr\0" "fld\0" "fld1\0" "fldcw\0" "fldenv\0" "fldl2e\0" "fldl2t\0" "fldlg2\0" "fldln2\0" "fldpi\0" "fldz\0"
  "fmulp\0" "fnclex\0" "fninit\0" "fnop\0" "fnsave\0" "fnstcw\0" "fnstenv\0" "fnstsw\0" "fpatan\0" "fprem\0" "fprem1\0"
  "fptan\0" "frndint\0" "frstor\0" "fsave\0" "fscale\0" "fsin\0" "fsincos\0" "fsqrt\0" "fst\0" "fstcw\0" "fstenv\0"
  "fstp\0" "fstsw\0" "fsubp\0" "fsubrp\0" "ftst\0" "fucom\0" "fucomi\0" "fucomip\0" "fucomp\0" "fucompp\0" "fwait\0"
  "fxam\0" "fxch\0" "fxrstor\0" "fxrstor64\0" "fxsave\0" "fxsave64\0" "fxtract\0" "fyl2x\0" "fyl2xp1\0" "getsec\0"
  "hlt\0" "inc\0" "insertq\0" "int3\0" "into\0" "invept\0" "invlpg\0" "invlpga\0" "invpcid\0" "invvpid\0" "iret\0"
  "iretd\0" "iretq\0" "iretw\0" "ja\0" "jae\0" "jb\0" "jbe\0" "jc\0" "je\0" "jecxz\0" "jg\0" "jge\0" "jl\0" "jle\0"
  "jmp\0" "jna\0" "jnae\0" "jnb\0" "jnbe\0" "jnc\0" "jne\0" "jng\0" "jnge\0" "jnl\0" "jnle\0" "jno\0" "jnp\0" "jns\0"
  "jnz\0" "jo\0" "jp\0" "jpe\0" "jpo\0" "js\0" "jz\0" "kaddb\0" "kaddd\0" "kaddq\0" "kaddw\0" "kandb\0" "kandd\0"
  "kandnb\0" "kandnd\0" "kandnq\0" "kandnw\0" "kandq\0" "kandw\0" "kmovb\0" "kmovw\0" "knotb\0" "knotd\0" "knotq\0"
  "knotw\0" "korb\0" "kord\0" "korq\0" "kortestb\0" "kortestd\0" "kortestq\0" "kortestw\0" "korw\0" "kshiftlb\0"
  "kshiftld\0" "kshiftlq\0" "kshiftlw\0" "kshiftrb\0" "kshiftrd\0" "kshiftrq\0" "kshiftrw\0" "ktestb\0" "ktestd\0"
  "ktestq\0" "ktestw\0" "kunpckbw\0" "kunpckdq\0" "kunpckwd\0" "kxnorb\0" "kxnord\0" "kxnorq\0" "kxnorw\0" "kxorb\0"
  "kxord\0" "kxorq\0" "kxorw\0" "lahf\0" "lar\0" "lds\0" "lea\0" "leave\0" "les\0" "lfence\0" "lfs\0" "lgdt\0" "lgs\0"
  "lidt\0" "lldt\0" "llwpcb\0" "lmsw\0" "lods\0" "loop\0" "loope\0" "loopne\0" "lsl\0" "ltr\0" "lwpins\0" "lwpval\0"
  "lzcnt\0" "mfence\0" "monitor\0" "monitorx\0" "movdir64b\0" "movdiri\0" "movdq2q\0" "movnti\0" "movntq\0" "movntsd\0"
  "movntss\0" "movq2dq\0" "movsx\0" "movsxd\0" "movzx\0" "mulx\0" "mwait\0" "mwaitx\0" "neg\0" "not\0" "out\0" "outs\0"
  "pause\0" "pavgusb\0" "pcommit\0" "pdep\0" "pext\0" "pf2id\0" "pf2iw\0" "pfacc\0" "pfadd\0" "pfcmpeq\0" "pfcmpge\0"
  "pfcmpgt\0" "pfmax\0" "pfmin\0" "pfmul\0" "pfnacc\0" "pfpnacc\0" "pfrcp\0" "pfrcpit1\0" "pfrcpit2\0" "pfrcpv\0"
  "pfrsqit1\0" "pfrsqrt\0" "pfrsqrtv\0" "pfsub\0" "pfsubr\0" "pi2fd\0" "pi2fw\0" "pmulhrw\0" "pop\0" "popa\0" "popad\0"
  "popcnt\0" "popf\0" "popfd\0" "popfq\0" "prefetch\0" "prefetchnta\0" "prefetcht0\0" "prefetcht1\0" "prefetcht2\0"
  "prefetchw\0" "prefetchwt1\0" "pshufw\0" "pswapd\0" "push\0" "pusha\0" "pushad\0" "pushf\0" "pushfd\0" "pushfq\0"
  "rcl\0" "rcr\0" "rdfsbase\0" "rdgsbase\0" "rdmsr\0" "rdpid\0" "rdpmc\0" "rdrand\0" "rdseed\0" "rdtsc\0" "rdtscp\0"
  "rol\0" "ror\0" "rorx\0" "rsm\0" "sahf\0" "sal\0" "sar\0" "sarx\0" "sbb\0" "scas\0" "seta\0" "setae\0" "setb\0"
  "setbe\0" "setc\0" "sete\0" "setg\0" "setge\0" "setl\0" "setle\0" "setna\0" "setnae\0" "setnb\0" "setnbe\0" "setnc\0"
  "setne\0" "setng\0" "setnge\0" "setnl\0" "setnle\0" "setno\0" "setnp\0" "setns\0" "setnz\0" "seto\0" "setp\0"
  "setpe\0" "setpo\0" "sets\0" "setz\0" "sfence\0" "sgdt\0" "sha1msg1\0" "sha1msg2\0" "sha1nexte\0" "sha1rnds4\0"
  "sha256msg1\0" "sha256msg2\0" "sha256rnds2\0" "shl\0" "shlx\0" "shr\0" "shrd\0" "shrx\0" "sidt\0" "skinit\0" "sldt\0"
  "slwpcb\0" "smsw\0" "stac\0" "stc\0" "stgi\0" "sti\0" "stos\0" "str\0" "swapgs\0" "syscall\0" "sysenter\0"
  "sysexit\0" "sysexit64\0" "sysret\0" "sysret64\0" "t1mskc\0" "tzcnt\0" "tzmsk\0" "ud2\0" "v4fmaddps\0" "v4fmaddss\0"
  "v4fnmaddps\0" "v4fnmaddss\0" "vaddpd\0" "vaddps\0" "vaddsd\0" "vaddss\0" "vaddsubpd\0" "vaddsubps\0" "vaesdec\0"
  "vaesdeclast\0" "vaesenc\0" "vaesenclast\0" "vaesimc\0" "vaeskeygenassist\0" "valignd\0" "valignq\0" "vandnpd\0"
  "vandnps\0" "vandpd\0" "vandps\0" "vblendmb\0" "vblendmd\0" "vblendmpd\0" "vblendmps\0" "vblendmq\0" "vblendmw\0"
  "vblendpd\0" "vblendps\0" "vblendvpd\0" "vblendvps\0" "vbroadcastf128\0" "vbroadcastf32x2\0" "vbroadcastf32x4\0"
  "vbroadcastf32x8\0" "vbroadcastf64x2\0" "vbroadcastf64x4\0" "vbroadcasti128\0" "vbroadcasti32x2\0"
  "vbroadcasti32x4\0" "vbroadcasti32x8\0" "vbroadcasti64x2\0" "vbroadcasti64x4\0" "vbroadcastsd\0" "vbroadcastss\0"
  "vcmppd\0" "vcmpps\0" "vcmpsd\0" "vcmpss\0" "vcomisd\0" "vcomiss\0" "vcompresspd\0" "vcompressps\0" "vcvtdq2pd\0"
  "vcvtdq2ps\0" "vcvtne2ps2bf16\0" "vcvtneps2bf16\0" "vcvtpd2dq\0" "vcvtpd2ps\0" "vcvtpd2qq\0" "vcvtpd2udq\0"
  "vcvtpd2uqq\0" "vcvtph2ps\0" "vcvtps2dq\0" "vcvtps2pd\0" "vcvtps2ph\0" "vcvtps2qq\0" "vcvtps2udq\0" "vcvtps2uqq\0"
  "vcvtqq2pd\0" "vcvtqq2ps\0" "vcvtsd2si\0" "vcvtsd2ss\0" "vcvtsd2usi\0" "vcvtsi2sd\0" "vcvtsi2ss\0" "vcvtss2sd\0"
  "vcvtss2si\0" "vcvtss2usi\0" "vcvttpd2dq\0" "vcvttpd2qq\0" "vcvttpd2udq\0" "vcvttpd2uqq\0" "vcvttps2dq\0"
  "vcvttps2qq\0" "vcvttps2udq\0" "vcvttps2uqq\0" "vcvttsd2si\0" "vcvttsd2usi\0" "vcvttss2si\0" "vcvttss2usi\0"
  "vcvtudq2pd\0" "vcvtudq2ps\0" "vcvtuqq2pd\0" "vcvtuqq2ps\0" "vcvtusi2sd\0" "vcvtusi2ss\0" "vdbpsadbw\0" "vdivpd\0"
  "vdivps\0" "vdivsd\0" "vdivss\0" "vdpbf16ps\0" "vdppd\0" "vdpps\0" "verr\0" "verw\0" "vexp2pd\0" "vexp2ps\0"
  "vexpandpd\0" "vexpandps\0" "vextractf128\0" "vextractf32x4\0" "vextractf32x8\0" "vextractf64x2\0" "vextractf64x4\0"
  "vextracti128\0" "vextracti32x4\0" "vextracti32x8\0" "vextracti64x2\0" "vextracti64x4\0" "vextractps\0"
  "vfixupimmpd\0" "vfixupimmps\0" "vfixupimmsd\0" "vfixupimmss\0" "vfmadd132pd\0" "vfmadd132ps\0" "vfmadd132sd\0"
  "vfmadd132ss\0" "vfmadd213pd\0" "vfmadd213ps\0" "vfmadd213sd\0" "vfmadd213ss\0" "vfmadd231pd\0" "vfmadd231ps\0"
  "vfmadd231sd\0" "vfmadd231ss\0" "vfmaddpd\0" "vfmaddps\0" "vfmaddsd\0" "vfmaddss\0" "vfmaddsub132pd\0"
  "vfmaddsub132ps\0" "vfmaddsub213pd\0" "vfmaddsub213ps\0" "vfmaddsub231pd\0" "vfmaddsub231ps\0" "vfmaddsubpd\0"
  "vfmaddsubps\0" "vfmsub132pd\0" "vfmsub132ps\0" "vfmsub132sd\0" "vfmsub132ss\0" "vfmsub213pd\0" "vfmsub213ps\0"
  "vfmsub213sd\0" "vfmsub213ss\0" "vfmsub231pd\0" "vfmsub231ps\0" "vfmsub231sd\0" "vfmsub231ss\0" "vfmsubadd132pd\0"
  "vfmsubadd132ps\0" "vfmsubadd213pd\0" "vfmsubadd213ps\0" "vfmsubadd231pd\0" "vfmsubadd231ps\0" "vfmsubaddpd\0"
  "vfmsubaddps\0" "vfmsubpd\0" "vfmsubps\0" "vfmsubsd\0" "vfmsubss\0" "vfnmadd132pd\0" "vfnmadd132ps\0"
  "vfnmadd132sd\0" "vfnmadd132ss\0" "vfnmadd213pd\0" "vfnmadd213ps\0" "vfnmadd213sd\0" "vfnmadd213ss\0"
  "vfnmadd231pd\0" "vfnmadd231ps\0" "vfnmadd231sd\0" "vfnmadd231ss\0" "vfnmaddpd\0" "vfnmaddps\0" "vfnmaddsd\0"
  "vfnmaddss\0" "vfnmsub132pd\0" "vfnmsub132ps\0" "vfnmsub132sd\0" "vfnmsub132ss\0" "vfnmsub213pd\0" "vfnmsub213ps\0"
  "vfnmsub213sd\0" "vfnmsub213ss\0" "vfnmsub231pd\0" "vfnmsub231ps\0" "vfnmsub231sd\0" "vfnmsub231ss\0" "vfnmsubpd\0"
  "vfnmsubps\0" "vfnmsubsd\0" "vfnmsubss\0" "vfpclasspd\0" "vfpclassps\0" "vfpclasssd\0" "vfpclassss\0" "vfrczpd\0"
  "vfrczps\0" "vfrczsd\0" "vfrczss\0" "vgatherdpd\0" "vgatherdps\0" "vgatherpf0dpd\0" "vgatherpf0dps\0"
  "vgatherpf0qpd\0" "vgatherpf0qps\0" "vgatherpf1dpd\0" "vgatherpf1dps\0" "vgatherpf1qpd\0" "vgatherpf1qps\0"
  "vgatherqpd\0" "vgatherqps\0" "vgetexppd\0" "vgetexpps\0" "vgetexpsd\0" "vgetexpss\0" "vgetmantpd\0" "vgetmantps\0"
  "vgetmantsd\0" "vgetmantss\0" "vgf2p8affineinvqb\0" "vgf2p8affineqb\0" "vgf2p8mulb\0" "vhaddpd\0" "vhaddps\0"
  "vhsubpd\0" "vhsubps\0" "vinsertf128\0" "vinsertf32x4\0" "vinsertf32x8\0" "vinsertf64x2\0" "vinsertf64x4\0"
  "vinserti128\0" "vinserti32x4\0" "vinserti32x8\0" "vinserti64x2\0" "vinserti64x4\0" "vinsertps\0" "vlddqu\0"
  "vldmxcsr\0" "vmaskmovdqu\0" "vmaskmovpd\0" "vmaskmovps\0" "vmaxpd\0" "vmaxps\0" "vmaxsd\0" "vmaxss\0" "vmcall\0"
  "vmclear\0" "vmfunc\0" "vminpd\0" "vminps\0" "vminsd\0" "vminss\0" "vmlaunch\0" "vmload\0" "vmmcall\0" "vmovapd\0"
  "vmovaps\0" "vmovd\0" "vmovddup\0" "vmovdqa\0" "vmovdqa32\0" "vmovdqa64\0" "vmovdqu\0" "vmovdqu16\0" "vmovdqu32\0"
  "vmovdqu64\0" "vmovdqu8\0" "vmovhlps\0" "vmovhpd\0" "vmovhps\0" "vmovlhps\0" "vmovlpd\0" "vmovlps\0" "vmovmskpd\0"
  "vmovmskps\0" "vmovntdq\0" "vmovntdqa\0" "vmovntpd\0" "vmovntps\0" "vmovq\0" "vmovsd\0" "vmovshdup\0" "vmovsldup\0"
  "vmovss\0" "vmovupd\0" "vmovups\0" "vmpsadbw\0" "vmptrld\0" "vmptrst\0" "vmread\0" "vmresume\0" "vmrun\0" "vmsave\0"
  "vmulpd\0" "vmulps\0" "vmulsd\0" "vmulss\0" "vmwrite\0" "vmxon\0" "vorpd\0" "vorps\0" "vp4dpwssd\0" "vp4dpwssds\0"
  "vpabsb\0" "vpabsd\0" "vpabsq\0" "vpabsw\0" "vpackssdw\0" "vpacksswb\0" "vpackusdw\0" "vpackuswb\0" "vpaddb\0"
  "vpaddd\0" "vpaddq\0" "vpaddsb\0" "vpaddsw\0" "vpaddusb\0" "vpaddusw\0" "vpaddw\0" "vpalignr\0" "vpand\0" "vpandd\0"
  "vpandn\0" "vpandnd\0" "vpandnq\0" "vpandq\0" "vpavgb\0" "vpavgw\0" "vpblendd\0" "vpblendvb\0" "vpblendw\0"
  "vpbroadcastb\0" "vpbroadcastd\0" "vpbroadcastmb2d\0" "vpbroadcastmb2q\0" "vpbroadcastq\0" "vpbroadcastw\0"
  "vpclmulqdq\0" "vpcmov\0" "vpcmpb\0" "vpcmpd\0" "vpcmpeqb\0" "vpcmpeqd\0" "vpcmpeqq\0" "vpcmpeqw\0" "vpcmpestri\0"
  "vpcmpestrm\0" "vpcmpgtb\0" "vpcmpgtd\0" "vpcmpgtq\0" "vpcmpgtw\0" "vpcmpistri\0" "vpcmpistrm\0" "vpcmpq\0"
  "vpcmpub\0" "vpcmpud\0" "vpcmpuq\0" "vpcmpuw\0" "vpcmpw\0" "vpcomb\0" "vpcomd\0" "vpcompressb\0" "vpcompressd\0"
  "vpcompressq\0" "vpcompressw\0" "vpcomq\0" "vpcomub\0" "vpcomud\0" "vpcomuq\0" "vpcomuw\0" "vpcomw\0" "vpconflictd\0"
  "vpconflictq\0" "vpdpbusd\0" "vpdpbusds\0" "vpdpwssd\0" "vpdpwssds\0" "vperm2f128\0" "vperm2i128\0" "vpermb\0"
  "vpermd\0" "vpermi2b\0" "vpermi2d\0" "vpermi2pd\0" "vpermi2ps\0" "vpermi2q\0" "vpermi2w\0" "vpermil2pd\0"
  "vpermil2ps\0" "vpermilpd\0" "vpermilps\0" "vpermpd\0" "vpermps\0" "vpermq\0" "vpermt2b\0" "vpermt2d\0" "vpermt2pd\0"
  "vpermt2ps\0" "vpermt2q\0" "vpermt2w\0" "vpermw\0" "vpexpandb\0" "vpexpandd\0" "vpexpandq\0" "vpexpandw\0"
  "vpextrb\0" "vpextrd\0" "vpextrq\0" "vpextrw\0" "vpgatherdd\0" "vpgatherdq\0" "vpgatherqd\0" "vpgatherqq\0"
  "vphaddbd\0" "vphaddbq\0" "vphaddbw\0" "vphaddd\0" "vphadddq\0" "vphaddsw\0" "vphaddubd\0" "vphaddubq\0"
  "vphaddubw\0" "vphaddudq\0" "vphadduwd\0" "vphadduwq\0" "vphaddw\0" "vphaddwd\0" "vphaddwq\0" "vphminposuw\0"
  "vphsubbw\0" "vphsubd\0" "vphsubdq\0" "vphsubsw\0" "vphsubw\0" "vphsubwd\0" "vpinsrb\0" "vpinsrd\0" "vpinsrq\0"
  "vpinsrw\0" "vplzcntd\0" "vplzcntq\0" "vpmacsdd\0" "vpmacsdqh\0" "vpmacsdql\0" "vpmacssdd\0" "vpmacssdqh\0"
  "vpmacssdql\0" "vpmacsswd\0" "vpmacssww\0" "vpmacswd\0" "vpmacsww\0" "vpmadcsswd\0" "vpmadcswd\0" "vpmadd52huq\0"
  "vpmadd52luq\0" "vpmaddubsw\0" "vpmaddwd\0" "vpmaskmovd\0" "vpmaskmovq\0" "vpmaxsb\0" "vpmaxsd\0" "vpmaxsq\0"
  "vpmaxsw\0" "vpmaxub\0" "vpmaxud\0" "vpmaxuq\0" "vpmaxuw\0" "vpminsb\0" "vpminsd\0" "vpminsq\0" "vpminsw\0"
  "vpminub\0" "vpminud\0" "vpminuq\0" "vpminuw\0" "vpmovb2m\0" "vpmovd2m\0" "vpmovdb\0" "vpmovdw\0" "vpmovm2b\0"
  "vpmovm2d\0" "vpmovm2q\0" "vpmovm2w\0" "vpmovmskb\0" "vpmovq2m\0" "vpmovqb\0" "vpmovqd\0" "vpmovqw\0" "vpmovsdb\0"
  "vpmovsdw\0" "vpmovsqb\0" "vpmovsqd\0" "vpmovsqw\0" "vpmovswb\0" "vpmovsxbd\0" "vpmovsxbq\0" "vpmovsxbw\0"
  "vpmovsxdq\0" "vpmovsxwd\0" "vpmovsxwq\0" "vpmovusdb\0" "vpmovusdw\0" "vpmovusqb\0" "vpmovusqd\0" "vpmovusqw\0"
  "vpmovuswb\0" "vpmovw2m\0" "vpmovwb\0" "vpmovzxbd\0" "vpmovzxbq\0" "vpmovzxbw\0" "vpmovzxdq\0" "vpmovzxwd\0"
  "vpmovzxwq\0" "vpmuldq\0" "vpmulhrsw\0" "vpmulhuw\0" "vpmulhw\0" "vpmulld\0" "vpmullq\0" "vpmullw\0"
  "vpmultishiftqb\0" "vpmuludq\0" "vpopcntb\0" "vpopcntd\0" "vpopcntq\0" "vpopcntw\0" "vpor\0" "vpord\0" "vporq\0"
  "vpperm\0" "vprold\0" "vprolq\0" "vprolvd\0" "vprolvq\0" "vprord\0" "vprorq\0" "vprorvd\0" "vprorvq\0" "vprotb\0"
  "vprotd\0" "vprotq\0" "vprotw\0" "vpsadbw\0" "vpscatterdd\0" "vpscatterdq\0" "vpscatterqd\0" "vpscatterqq\0"
  "vpshab\0" "vpshad\0" "vpshaq\0" "vpshaw\0" "vpshlb\0" "vpshld\0" "vpshldd\0" "vpshldq\0" "vpshldvd\0" "vpshldvq\0"
  "vpshldvw\0" "vpshldw\0" "vpshlq\0" "vpshlw\0" "vpshrdd\0" "vpshrdq\0" "vpshrdvd\0" "vpshrdvq\0" "vpshrdvw\0"
  "vpshrdw\0" "vpshufb\0" "vpshufbitqmb\0" "vpshufd\0" "vpshufhw\0" "vpshuflw\0" "vpsignb\0" "vpsignd\0" "vpsignw\0"
  "vpslld\0" "vpslldq\0" "vpsllq\0" "vpsllvd\0" "vpsllvq\0" "vpsllvw\0" "vpsllw\0" "vpsrad\0" "vpsraq\0" "vpsravd\0"
  "vpsravq\0" "vpsravw\0" "vpsraw\0" "vpsrld\0" "vpsrldq\0" "vpsrlq\0" "vpsrlvd\0" "vpsrlvq\0" "vpsrlvw\0" "vpsrlw\0"
  "vpsubb\0" "vpsubd\0" "vpsubq\0" "vpsubsb\0" "vpsubsw\0" "vpsubusb\0" "vpsubusw\0" "vpsubw\0" "vpternlogd\0"
  "vpternlogq\0" "vptest\0" "vptestmb\0" "vptestmd\0" "vptestmq\0" "vptestmw\0" "vptestnmb\0" "vptestnmd\0"
  "vptestnmq\0" "vptestnmw\0" "vpunpckhbw\0" "vpunpckhdq\0" "vpunpckhqdq\0" "vpunpckhwd\0" "vpunpcklbw\0"
  "vpunpckldq\0" "vpunpcklqdq\0" "vpunpcklwd\0" "vpxor\0" "vpxord\0" "vpxorq\0" "vrangepd\0" "vrangeps\0" "vrangesd\0"
  "vrangess\0" "vrcp14pd\0" "vrcp14ps\0" "vrcp14sd\0" "vrcp14ss\0" "vrcp28pd\0" "vrcp28ps\0" "vrcp28sd\0" "vrcp28ss\0"
  "vrcpps\0" "vrcpss\0" "vreducepd\0" "vreduceps\0" "vreducesd\0" "vreducess\0" "vrndscalepd\0" "vrndscaleps\0"
  "vrndscalesd\0" "vrndscaless\0" "vroundpd\0" "vroundps\0" "vroundsd\0" "vroundss\0" "vrsqrt14pd\0" "vrsqrt14ps\0"
  "vrsqrt14sd\0" "vrsqrt14ss\0" "vrsqrt28pd\0" "vrsqrt28ps\0" "vrsqrt28sd\0" "vrsqrt28ss\0" "vrsqrtps\0" "vrsqrtss\0"
  "vscalefpd\0" "vscalefps\0" "vscalefsd\0" "vscalefss\0" "vscatterdpd\0" "vscatterdps\0" "vscatterpf0dpd\0"
  "vscatterpf0dps\0" "vscatterpf0qpd\0" "vscatterpf0qps\0" "vscatterpf1dpd\0" "vscatterpf1dps\0" "vscatterpf1qpd\0"
  "vscatterpf1qps\0" "vscatterqpd\0" "vscatterqps\0" "vshuff32x4\0" "vshuff64x2\0" "vshufi32x4\0" "vshufi64x2\0"
  "vshufpd\0" "vshufps\0" "vsqrtpd\0" "vsqrtps\0" "vsqrtsd\0" "vsqrtss\0" "vstmxcsr\0" "vsubpd\0" "vsubps\0" "vsubsd\0"
  "vsubss\0" "vtestpd\0" "vtestps\0" "vucomisd\0" "vucomiss\0" "vunpckhpd\0" "vunpckhps\0" "vunpcklpd\0" "vunpcklps\0"
  "vxorpd\0" "vxorps\0" "vzeroall\0" "vzeroupper\0" "wbinvd\0" "wbnoinvd\0" "wrfsbase\0" "wrgsbase\0" "wrmsr\0"
  "xabort\0" "xadd\0" "xbegin\0" "xend\0" "xgetbv\0" "xlatb\0" "xrstors\0" "xrstors64\0" "xsavec\0" "xsavec64\0"
  "xsaveopt\0" "xsaveopt64\0" "xsaves\0" "xsaves64\0" "xsetbv\0" "xtest";

enum : uint32_t {
  kX86InstMaxSize = 17
};

struct InstNameIndex {
  uint16_t start;
  uint16_t end;
};

static const InstNameIndex X86InstNameIndex[26] = {
  { Inst::kIdAaa          , Inst::kIdArpl          + 1 },
  { Inst::kIdBextr        , Inst::kIdBzhi          + 1 },
  { Inst::kIdCall         , Inst::kIdCwde          + 1 },
  { Inst::kIdDaa          , Inst::kIdDpps          + 1 },
  { Inst::kIdEmms         , Inst::kIdExtrq         + 1 },
  { Inst::kIdF2xm1        , Inst::kIdFyl2xp1       + 1 },
  { Inst::kIdGetsec       , Inst::kIdGf2p8mulb     + 1 },
  { Inst::kIdHaddpd       , Inst::kIdHsubps        + 1 },
  { Inst::kIdIdiv         , Inst::kIdIretw         + 1 },
  { Inst::kIdJa           , Inst::kIdJz            + 1 },
  { Inst::kIdKaddb        , Inst::kIdKxorw         + 1 },
  { Inst::kIdLahf         , Inst::kIdLzcnt         + 1 },
  { Inst::kIdMaskmovdqu   , Inst::kIdMwaitx        + 1 },
  { Inst::kIdNeg          , Inst::kIdNot           + 1 },
  { Inst::kIdOr           , Inst::kIdOuts          + 1 },
  { Inst::kIdPabsb        , Inst::kIdPxor          + 1 },
  { Inst::kIdNone         , Inst::kIdNone          + 1 },
  { Inst::kIdRcl          , Inst::kIdRsqrtss       + 1 },
  { Inst::kIdSahf         , Inst::kIdSysret64      + 1 },
  { Inst::kIdT1mskc       , Inst::kIdTzmsk         + 1 },
  { Inst::kIdUcomisd      , Inst::kIdUnpcklps      + 1 },
  { Inst::kIdV4fmaddps    , Inst::kIdVzeroupper    + 1 },
  { Inst::kIdWbinvd       , Inst::kIdWrmsr         + 1 },
  { Inst::kIdXabort       , Inst::kIdXtest         + 1 },
  { Inst::kIdNone         , Inst::kIdNone          + 1 },
  { Inst::kIdNone         , Inst::kIdNone          + 1 }
};
// ----------------------------------------------------------------------------
// ${NameData:End}

uint32_t InstDB::idByName(const char* name, size_t nameSize) noexcept {
  if (ASMJIT_UNLIKELY(!name))
    return Inst::kIdNone;

  if (nameSize == SIZE_MAX)
    nameSize = ::strlen(name);

  if (ASMJIT_UNLIKELY(nameSize == 0 || nameSize > kX86InstMaxSize))
    return Inst::kIdNone;

  uint32_t prefix = uint32_t(name[0]) - 'a';
  if (ASMJIT_UNLIKELY(prefix > 'z' - 'a'))
    return Inst::kIdNone;

  uint32_t index = X86InstNameIndex[prefix].start;
  if (ASMJIT_UNLIKELY(!index))
    return Inst::kIdNone;

  const char* nameData = InstDB::_nameData;
  const InstInfo* table = InstDB::_instInfoTable;

  const InstInfo* base = table + index;
  const InstInfo* end  = table + X86InstNameIndex[prefix].end;

  for (size_t lim = (size_t)(end - base); lim != 0; lim >>= 1) {
    const InstInfo* cur = base + (lim >> 1);
    int result = Support::cmpInstName(nameData + cur[0]._nameDataIndex, name, nameSize);

    if (result < 0) {
      base = cur + 1;
      lim--;
      continue;
    }

    if (result > 0)
      continue;

    return uint32_t((size_t)(cur - table));
  }

  return Inst::kIdNone;
}

const char* InstDB::nameById(uint32_t id) noexcept {
  if (ASMJIT_UNLIKELY(!Inst::isDefinedId(id)))
    return nullptr;
  return InstDB::infoById(id).name();
}
#else
const char InstDB::_nameData[] = "";
#endif

// ============================================================================
// [asmjit::x86::InstDB - InstSignature / OpSignature]
// ============================================================================

#ifndef ASMJIT_DISABLE_INST_API
// ${InstSignatureTable:Begin}
// ------------------- Automatically generated, do not edit -------------------
#define ROW(count, x86, x64, implicit, o0, o1, o2, o3, o4, o5)  \
  { count, (x86 ? uint8_t(InstDB::kModeX86) : uint8_t(0)) |     \
           (x64 ? uint8_t(InstDB::kModeX64) : uint8_t(0)) ,     \
    implicit,                                                   \
    0,                                                          \
    { o0, o1, o2, o3, o4, o5 }                                  \
  }
const InstDB::InstSignature InstDB::_instSignatureTable[] = {
  ROW(2, 1, 1, 0, 1  , 2  , 0  , 0  , 0  , 0  ), // #0   {r8lo|r8hi|m8|mem, r8lo|r8hi}
  ROW(2, 1, 1, 0, 3  , 4  , 0  , 0  , 0  , 0  ), //      {r16|m16|mem|sreg, r16}
  ROW(2, 1, 1, 0, 5  , 6  , 0  , 0  , 0  , 0  ), //      {r32|m32|mem|sreg, r32}
  ROW(2, 0, 1, 0, 7  , 8  , 0  , 0  , 0  , 0  ), //      {r64|m64|mem|sreg|creg|dreg, r64}
  ROW(2, 1, 1, 0, 9  , 10 , 0  , 0  , 0  , 0  ), //      {r8lo|r8hi|m8, i8|u8}
  ROW(2, 1, 1, 0, 11 , 12 , 0  , 0  , 0  , 0  ), //      {r16|m16, i16|u16}
  ROW(2, 1, 1, 0, 13 , 14 , 0  , 0  , 0  , 0  ), //      {r32|m32, i32|u32}
  ROW(2, 0, 1, 0, 15 , 16 , 0  , 0  , 0  , 0  ), //      {r64|m64|mem, i32}
  ROW(2, 0, 1, 0, 8  , 17 , 0  , 0  , 0  , 0  ), //      {r64, i64|u64|m64|mem|sreg|creg|dreg}
  ROW(2, 1, 1, 0, 2  , 18 , 0  , 0  , 0  , 0  ), //      {r8lo|r8hi, m8|mem}
  ROW(2, 1, 1, 0, 4  , 19 , 0  , 0  , 0  , 0  ), //      {r16, m16|mem|sreg}
  ROW(2, 1, 1, 0, 6  , 20 , 0  , 0  , 0  , 0  ), //      {r32, m32|mem|sreg}
  ROW(2, 1, 1, 0, 21 , 22 , 0  , 0  , 0  , 0  ), //      {m16|mem, sreg}
  ROW(2, 1, 1, 0, 22 , 21 , 0  , 0  , 0  , 0  ), //      {sreg, m16|mem}
  ROW(2, 1, 0, 0, 6  , 23 , 0  , 0  , 0  , 0  ), //      {r32, creg|dreg}
  ROW(2, 1, 0, 0, 23 , 6  , 0  , 0  , 0  , 0  ), //      {creg|dreg, r32}
  ROW(2, 1, 1, 0, 9  , 10 , 0  , 0  , 0  , 0  ), // #16  {r8lo|r8hi|m8, i8|u8}
  ROW(2, 1, 1, 0, 11 , 12 , 0  , 0  , 0  , 0  ), //      {r16|m16, i16|u16}
  ROW(2, 1, 1, 0, 13 , 14 , 0  , 0  , 0  , 0  ), //      {r32|m32, i32|u32}
  ROW(2, 0, 1, 0, 15 , 24 , 0  , 0  , 0  , 0  ), //      {r64|m64|mem, i32|r64}
  ROW(2, 1, 1, 0, 25 , 26 , 0  , 0  , 0  , 0  ), //      {r16|m16|r32|m32|r64|m64|mem, i8}
  ROW(2, 1, 1, 0, 1  , 2  , 0  , 0  , 0  , 0  ), //      {r8lo|r8hi|m8|mem, r8lo|r8hi}
  ROW(2, 1, 1, 0, 27 , 4  , 0  , 0  , 0  , 0  ), //      {r16|m16|mem, r16}
  ROW(2, 1, 1, 0, 28 , 6  , 0  , 0  , 0  , 0  ), //      {r32|m32|mem, r32}
  ROW(2, 1, 1, 0, 2  , 18 , 0  , 0  , 0  , 0  ), //      {r8lo|r8hi, m8|mem}
  ROW(2, 1, 1, 0, 4  , 21 , 0  , 0  , 0  , 0  ), //      {r16, m16|mem}
  ROW(2, 1, 1, 0, 6  , 29 , 0  , 0  , 0  , 0  ), //      {r32, m32|mem}
  ROW(2, 0, 1, 0, 8  , 30 , 0  , 0  , 0  , 0  ), //      {r64, m64|mem}
  ROW(2, 1, 1, 0, 31 , 10 , 0  , 0  , 0  , 0  ), // #28  {r8lo|r8hi|m8|r16|m16|r32|m32|r64|m64|mem, i8|u8}
  ROW(2, 1, 1, 0, 11 , 12 , 0  , 0  , 0  , 0  ), //      {r16|m16, i16|u16}
  ROW(2, 1, 1, 0, 13 , 14 , 0  , 0  , 0  , 0  ), //      {r32|m32, i32|u32}
  ROW(2, 0, 1, 0, 8  , 32 , 0  , 0  , 0  , 0  ), //      {r64, u32|i32|r64|m64|mem}
  ROW(2, 0, 1, 0, 30 , 24 , 0  , 0  , 0  , 0  ), //      {m64|mem, i32|r64}
  ROW(2, 1, 1, 0, 1  , 2  , 0  , 0  , 0  , 0  ), //      {r8lo|r8hi|m8|mem, r8lo|r8hi}
  ROW(2, 1, 1, 0, 27 , 4  , 0  , 0  , 0  , 0  ), //      {r16|m16|mem, r16}
  ROW(2, 1, 1, 0, 28 , 6  , 0  , 0  , 0  , 0  ), //      {r32|m32|mem, r32}
  ROW(2, 1, 1, 0, 2  , 18 , 0  , 0  , 0  , 0  ), //      {r8lo|r8hi, m8|mem}
  ROW(2, 1, 1, 0, 4  , 21 , 0  , 0  , 0  , 0  ), //      {r16, m16|mem}
  ROW(2, 1, 1, 0, 6  , 29 , 0  , 0  , 0  , 0  ), //      {r32, m32|mem}
  ROW(2, 1, 1, 1, 33 , 1  , 0  , 0  , 0  , 0  ), // #39  {<ax>, r8lo|r8hi|m8|mem}
  ROW(3, 1, 1, 2, 34 , 33 , 27 , 0  , 0  , 0  ), //      {<dx>, <ax>, r16|m16|mem}
  ROW(3, 1, 1, 2, 35 , 36 , 28 , 0  , 0  , 0  ), //      {<edx>, <eax>, r32|m32|mem}
  ROW(3, 0, 1, 2, 37 , 38 , 15 , 0  , 0  , 0  ), //      {<rdx>, <rax>, r64|m64|mem}
  ROW(2, 1, 1, 0, 4  , 39 , 0  , 0  , 0  , 0  ), //      {r16, r16|m16|mem|i8|i16}
  ROW(2, 1, 1, 0, 6  , 40 , 0  , 0  , 0  , 0  ), //      {r32, r32|m32|mem|i8|i32}
  ROW(2, 0, 1, 0, 8  , 41 , 0  , 0  , 0  , 0  ), //      {r64, r64|m64|mem|i8|i32}
  ROW(3, 1, 1, 0, 4  , 27 , 42 , 0  , 0  , 0  ), //      {r16, r16|m16|mem, i8|i16|u16}
  ROW(3, 1, 1, 0, 6  , 28 , 43 , 0  , 0  , 0  ), //      {r32, r32|m32|mem, i8|i32|u32}
  ROW(3, 0, 1, 0, 8  , 15 , 44 , 0  , 0  , 0  ), //      {r64, r64|m64|mem, i8|i32}
  ROW(2, 1, 1, 0, 1  , 2  , 0  , 0  , 0  , 0  ), // #49  {r8lo|r8hi|m8|mem, r8lo|r8hi}
  ROW(2, 1, 1, 0, 27 , 4  , 0  , 0  , 0  , 0  ), //      {r16|m16|mem, r16}
  ROW(2, 1, 1, 0, 28 , 6  , 0  , 0  , 0  , 0  ), //      {r32|m32|mem, r32}
  ROW(2, 0, 1, 0, 15 , 8  , 0  , 0  , 0  , 0  ), //      {r64|m64|mem, r64}
  ROW(2, 1, 1, 0, 2  , 18 , 0  , 0  , 0  , 0  ), //      {r8lo|r8hi, m8|mem}
  ROW(2, 1, 1, 0, 4  , 21 , 0  , 0  , 0  , 0  ), //      {r16, m16|mem}
  ROW(2, 1, 1, 0, 6  , 29 , 0  , 0  , 0  , 0  ), //      {r32, m32|mem}
  ROW(2, 0, 1, 0, 8  , 30 , 0  , 0  , 0  , 0  ), //      {r64, m64|mem}
  ROW(2, 1, 1, 0, 9  , 10 , 0  , 0  , 0  , 0  ), // #57  {r8lo|r8hi|m8, i8|u8}
  ROW(2, 1, 1, 0, 11 , 12 , 0  , 0  , 0  , 0  ), //      {r16|m16, i16|u16}
  ROW(2, 1, 1, 0, 13 , 14 , 0  , 0  , 0  , 0  ), //      {r32|m32, i32|u32}
  ROW(2, 0, 1, 0, 15 , 24 , 0  , 0  , 0  , 0  ), //      {r64|m64|mem, i32|r64}
  ROW(2, 1, 1, 0, 1  , 2  , 0  , 0  , 0  , 0  ), //      {r8lo|r8hi|m8|mem, r8lo|r8hi}
  ROW(2, 1, 1, 0, 27 , 4  , 0  , 0  , 0  , 0  ), //      {r16|m16|mem, r16}
  ROW(2, 1, 1, 0, 28 , 6  , 0  , 0  , 0  , 0  ), //      {r32|m32|mem, r32}
  ROW(2, 1, 1, 0, 4  , 21 , 0  , 0  , 0  , 0  ), // #64  {r16, m16|mem}
  ROW(2, 1, 1, 0, 6  , 29 , 0  , 0  , 0  , 0  ), //      {r32, m32|mem}
  ROW(2, 0, 1, 0, 8  , 30 , 0  , 0  , 0  , 0  ), //      {r64, m64|mem}
  ROW(2, 1, 1, 0, 21 , 4  , 0  , 0  , 0  , 0  ), //      {m16|mem, r16}
  ROW(2, 1, 1, 0, 29 , 6  , 0  , 0  , 0  , 0  ), // #68  {m32|mem, r32}
  ROW(2, 0, 1, 0, 30 , 8  , 0  , 0  , 0  , 0  ), //      {m64|mem, r64}
  ROW(2, 1, 1, 0, 45 , 46 , 0  , 0  , 0  , 0  ), // #70  {xmm, xmm|m128|mem}
  ROW(2, 1, 1, 0, 47 , 45 , 0  , 0  , 0  , 0  ), // #71  {m128|mem, xmm}
  ROW(2, 1, 1, 0, 48 , 49 , 0  , 0  , 0  , 0  ), //      {ymm, ymm|m256|mem}
  ROW(2, 1, 1, 0, 50 , 48 , 0  , 0  , 0  , 0  ), //      {m256|mem, ymm}
  ROW(2, 1, 1, 0, 51 , 52 , 0  , 0  , 0  , 0  ), // #74  {zmm, zmm|m512|mem}
  ROW(2, 1, 1, 0, 53 , 51 , 0  , 0  , 0  , 0  ), //      {m512|mem, zmm}
  ROW(3, 1, 1, 0, 45 , 45 , 54 , 0  , 0  , 0  ), // #76  {xmm, xmm, xmm|m128|mem|i8|u8}
  ROW(3, 1, 1, 0, 45 , 47 , 10 , 0  , 0  , 0  ), //      {xmm, m128|mem, i8|u8}
  ROW(3, 1, 1, 0, 48 , 48 , 55 , 0  , 0  , 0  ), //      {ymm, ymm, ymm|m256|mem|i8|u8}
  ROW(3, 1, 1, 0, 48 , 50 , 10 , 0  , 0  , 0  ), //      {ymm, m256|mem, i8|u8}
  ROW(3, 1, 1, 0, 51 , 51 , 56 , 0  , 0  , 0  ), //      {zmm, zmm, zmm|m512|mem|i8|u8}
  ROW(3, 1, 1, 0, 51 , 53 , 10 , 0  , 0  , 0  ), //      {zmm, m512|mem, i8|u8}
  ROW(3, 1, 1, 0, 45 , 45 , 54 , 0  , 0  , 0  ), // #82  {xmm, xmm, i8|u8|xmm|m128|mem}
  ROW(3, 1, 1, 0, 48 , 48 , 54 , 0  , 0  , 0  ), //      {ymm, ymm, i8|u8|xmm|m128|mem}
  ROW(3, 1, 1, 0, 45 , 47 , 10 , 0  , 0  , 0  ), //      {xmm, m128|mem, i8|u8}
  ROW(3, 1, 1, 0, 48 , 50 , 10 , 0  , 0  , 0  ), //      {ymm, m256|mem, i8|u8}
  ROW(3, 1, 1, 0, 51 , 51 , 54 , 0  , 0  , 0  ), //      {zmm, zmm, xmm|m128|mem|i8|u8}
  ROW(3, 1, 1, 0, 51 , 53 , 10 , 0  , 0  , 0  ), //      {zmm, m512|mem, i8|u8}
  ROW(3, 1, 1, 0, 45 , 45 , 54 , 0  , 0  , 0  ), // #88  {xmm, xmm, xmm|m128|mem|i8|u8}
  ROW(3, 1, 1, 0, 45 , 47 , 10 , 0  , 0  , 0  ), //      {xmm, m128|mem, i8|u8}
  ROW(3, 1, 1, 0, 48 , 48 , 54 , 0  , 0  , 0  ), //      {ymm, ymm, xmm|m128|mem|i8|u8}
  ROW(3, 1, 1, 0, 48 , 50 , 10 , 0  , 0  , 0  ), //      {ymm, m256|mem, i8|u8}
  ROW(3, 1, 1, 0, 51 , 51 , 54 , 0  , 0  , 0  ), //      {zmm, zmm, xmm|m128|mem|i8|u8}
  ROW(3, 1, 1, 0, 51 , 53 , 10 , 0  , 0  , 0  ), //      {zmm, m512|mem, i8|u8}
  ROW(2, 1, 1, 0, 57 , 58 , 0  , 0  , 0  , 0  ), // #94  {mm, mm|m64|mem|r64}
  ROW(2, 1, 1, 0, 15 , 59 , 0  , 0  , 0  , 0  ), //      {m64|mem|r64, mm|xmm}
  ROW(2, 0, 1, 0, 45 , 15 , 0  , 0  , 0  , 0  ), //      {xmm, r64|m64|mem}
  ROW(2, 1, 1, 0, 45 , 60 , 0  , 0  , 0  , 0  ), // #97  {xmm, xmm|m64|mem}
  ROW(2, 1, 1, 0, 30 , 45 , 0  , 0  , 0  , 0  ), // #98  {m64|mem, xmm}
  ROW(3, 1, 1, 0, 45 , 61 , 45 , 0  , 0  , 0  ), // #99  {xmm, vm32x, xmm}
  ROW(3, 1, 1, 0, 48 , 61 , 48 , 0  , 0  , 0  ), //      {ymm, vm32x, ymm}
  ROW(2, 1, 1, 0, 45 , 61 , 0  , 0  , 0  , 0  ), //      {xmm, vm32x}
  ROW(2, 1, 1, 0, 48 , 62 , 0  , 0  , 0  , 0  ), //      {ymm, vm32y}
  ROW(2, 1, 1, 0, 51 , 63 , 0  , 0  , 0  , 0  ), //      {zmm, vm32z}
  ROW(3, 1, 1, 0, 45 , 61 , 45 , 0  , 0  , 0  ), // #104 {xmm, vm32x, xmm}
  ROW(3, 1, 1, 0, 48 , 62 , 48 , 0  , 0  , 0  ), //      {ymm, vm32y, ymm}
  ROW(2, 1, 1, 0, 45 , 61 , 0  , 0  , 0  , 0  ), //      {xmm, vm32x}
  ROW(2, 1, 1, 0, 48 , 62 , 0  , 0  , 0  , 0  ), //      {ymm, vm32y}
  ROW(2, 1, 1, 0, 51 , 63 , 0  , 0  , 0  , 0  ), //      {zmm, vm32z}
  ROW(3, 1, 1, 0, 45 , 64 , 45 , 0  , 0  , 0  ), // #109 {xmm, vm64x, xmm}
  ROW(3, 1, 1, 0, 48 , 65 , 48 , 0  , 0  , 0  ), //      {ymm, vm64y, ymm}
  ROW(2, 1, 1, 0, 45 , 64 , 0  , 0  , 0  , 0  ), //      {xmm, vm64x}
  ROW(2, 1, 1, 0, 48 , 65 , 0  , 0  , 0  , 0  ), //      {ymm, vm64y}
  ROW(2, 1, 1, 0, 51 , 66 , 0  , 0  , 0  , 0  ), //      {zmm, vm64z}
  ROW(2, 1, 1, 0, 25 , 10 , 0  , 0  , 0  , 0  ), // #114 {r16|m16|r32|m32|r64|m64|mem, i8|u8}
  ROW(2, 1, 1, 0, 27 , 4  , 0  , 0  , 0  , 0  ), //      {r16|m16|mem, r16}
  ROW(2, 1, 1, 0, 28 , 6  , 0  , 0  , 0  , 0  ), //      {r32|m32|mem, r32}
  ROW(2, 0, 1, 0, 15 , 8  , 0  , 0  , 0  , 0  ), //      {r64|m64|mem, r64}
  ROW(3, 1, 1, 1, 1  , 2  , 67 , 0  , 0  , 0  ), // #118 {r8lo|r8hi|m8|mem, r8lo|r8hi, <al>}
  ROW(3, 1, 1, 1, 27 , 4  , 33 , 0  , 0  , 0  ), //      {r16|m16|mem, r16, <ax>}
  ROW(3, 1, 1, 1, 28 , 6  , 36 , 0  , 0  , 0  ), //      {r32|m32|mem, r32, <eax>}
  ROW(3, 0, 1, 1, 15 , 8  , 38 , 0  , 0  , 0  ), //      {r64|m64|mem, r64, <rax>}
  ROW(1, 1, 1, 0, 68 , 0  , 0  , 0  , 0  , 0  ), // #122 {r16|m16|r64|m64|mem}
  ROW(1, 1, 0, 0, 13 , 0  , 0  , 0  , 0  , 0  ), //      {r32|m32}
  ROW(1, 1, 0, 0, 69 , 0  , 0  , 0  , 0  , 0  ), //      {ds|es|ss}
  ROW(1, 1, 1, 0, 70 , 0  , 0  , 0  , 0  , 0  ), //      {fs|gs}
  ROW(1, 1, 1, 0, 71 , 0  , 0  , 0  , 0  , 0  ), // #126 {r16|m16|r64|m64|mem|i8|i16|i32}
  ROW(1, 1, 0, 0, 72 , 0  , 0  , 0  , 0  , 0  ), //      {r32|m32|i32|u32}
  ROW(1, 1, 0, 0, 73 , 0  , 0  , 0  , 0  , 0  ), //      {cs|ss|ds|es}
  ROW(1, 1, 1, 0, 70 , 0  , 0  , 0  , 0  , 0  ), //      {fs|gs}
  ROW(4, 1, 1, 0, 45 , 45 , 45 , 46 , 0  , 0  ), // #130 {xmm, xmm, xmm, xmm|m128|mem}
  ROW(4, 1, 1, 0, 45 , 45 , 47 , 45 , 0  , 0  ), //      {xmm, xmm, m128|mem, xmm}
  ROW(4, 1, 1, 0, 48 , 48 , 48 , 49 , 0  , 0  ), //      {ymm, ymm, ymm, ymm|m256|mem}
  ROW(4, 1, 1, 0, 48 , 48 , 50 , 48 , 0  , 0  ), //      {ymm, ymm, m256|mem, ymm}
  ROW(3, 1, 1, 0, 45 , 74 , 45 , 0  , 0  , 0  ), // #134 {xmm, vm64x|vm64y, xmm}
  ROW(2, 1, 1, 0, 45 , 64 , 0  , 0  , 0  , 0  ), //      {xmm, vm64x}
  ROW(2, 1, 1, 0, 48 , 65 , 0  , 0  , 0  , 0  ), //      {ymm, vm64y}
  ROW(2, 1, 1, 0, 51 , 66 , 0  , 0  , 0  , 0  ), //      {zmm, vm64z}
  ROW(3, 1, 1, 0, 47 , 45 , 45 , 0  , 0  , 0  ), // #138 {m128|mem, xmm, xmm}
  ROW(3, 1, 1, 0, 50 , 48 , 48 , 0  , 0  , 0  ), //      {m256|mem, ymm, ymm}
  ROW(3, 1, 1, 0, 45 , 45 , 47 , 0  , 0  , 0  ), //      {xmm, xmm, m128|mem}
  ROW(3, 1, 1, 0, 48 , 48 , 50 , 0  , 0  , 0  ), //      {ymm, ymm, m256|mem}
  ROW(5, 1, 1, 0, 45 , 45 , 46 , 45 , 75 , 0  ), // #142 {xmm, xmm, xmm|m128|mem, xmm, i4|u4}
  ROW(5, 1, 1, 0, 45 , 45 , 45 , 47 , 75 , 0  ), //      {xmm, xmm, xmm, m128|mem, i4|u4}
  ROW(5, 1, 1, 0, 48 , 48 , 49 , 48 , 75 , 0  ), //      {ymm, ymm, ymm|m256|mem, ymm, i4|u4}
  ROW(5, 1, 1, 0, 48 , 48 , 48 , 50 , 75 , 0  ), //      {ymm, ymm, ymm, m256|mem, i4|u4}
  ROW(3, 1, 1, 0, 48 , 49 , 10 , 0  , 0  , 0  ), // #146 {ymm, ymm|m256|mem, i8|u8}
  ROW(3, 1, 1, 0, 48 , 48 , 49 , 0  , 0  , 0  ), // #147 {ymm, ymm, ymm|m256|mem}
  ROW(3, 1, 1, 0, 51 , 51 , 56 , 0  , 0  , 0  ), //      {zmm, zmm, zmm|m512|mem|i8|u8}
  ROW(3, 1, 1, 0, 51 , 53 , 10 , 0  , 0  , 0  ), //      {zmm, m512|mem, i8|u8}
  ROW(2, 1, 1, 0, 4  , 27 , 0  , 0  , 0  , 0  ), // #150 {r16, r16|m16|mem}
  ROW(2, 1, 1, 0, 6  , 28 , 0  , 0  , 0  , 0  ), // #151 {r32, r32|m32|mem}
  ROW(2, 0, 1, 0, 8  , 15 , 0  , 0  , 0  , 0  ), //      {r64, r64|m64|mem}
  ROW(1, 1, 1, 0, 76 , 0  , 0  , 0  , 0  , 0  ), // #153 {m32|m64}
  ROW(2, 1, 1, 0, 77 , 78 , 0  , 0  , 0  , 0  ), //      {st0, st}
  ROW(2, 1, 1, 0, 78 , 77 , 0  , 0  , 0  , 0  ), //      {st, st0}
  ROW(2, 1, 1, 0, 4  , 29 , 0  , 0  , 0  , 0  ), // #156 {r16, m32|mem}
  ROW(2, 1, 1, 0, 6  , 79 , 0  , 0  , 0  , 0  ), //      {r32, m48|mem}
  ROW(2, 0, 1, 0, 8  , 80 , 0  , 0  , 0  , 0  ), //      {r64, m80|mem}
  ROW(3, 1, 1, 0, 27 , 4  , 81 , 0  , 0  , 0  ), // #159 {r16|m16|mem, r16, cl|i8|u8}
  ROW(3, 1, 1, 0, 28 , 6  , 81 , 0  , 0  , 0  ), //      {r32|m32|mem, r32, cl|i8|u8}
  ROW(3, 0, 1, 0, 15 , 8  , 81 , 0  , 0  , 0  ), //      {r64|m64|mem, r64, cl|i8|u8}
  ROW(3, 1, 1, 0, 45 , 45 , 46 , 0  , 0  , 0  ), // #162 {xmm, xmm, xmm|m128|mem}
  ROW(3, 1, 1, 0, 48 , 48 , 49 , 0  , 0  , 0  ), // #163 {ymm, ymm, ymm|m256|mem}
  ROW(3, 1, 1, 0, 51 , 51 , 52 , 0  , 0  , 0  ), //      {zmm, zmm, zmm|m512|mem}
  ROW(4, 1, 1, 0, 45 , 45 , 46 , 10 , 0  , 0  ), // #165 {xmm, xmm, xmm|m128|mem, i8|u8}
  ROW(4, 1, 1, 0, 48 , 48 , 49 , 10 , 0  , 0  ), // #166 {ymm, ymm, ymm|m256|mem, i8|u8}
  ROW(4, 1, 1, 0, 51 , 51 , 52 , 10 , 0  , 0  ), //      {zmm, zmm, zmm|m512|mem, i8|u8}
  ROW(4, 1, 1, 0, 82 , 45 , 46 , 10 , 0  , 0  ), // #168 {xmm|k, xmm, xmm|m128|mem, i8|u8}
  ROW(4, 1, 1, 0, 83 , 48 , 49 , 10 , 0  , 0  ), //      {ymm|k, ymm, ymm|m256|mem, i8|u8}
  ROW(4, 1, 1, 0, 84 , 51 , 52 , 10 , 0  , 0  ), //      {k, zmm, zmm|m512|mem, i8|u8}
  ROW(2, 1, 1, 0, 46 , 45 , 0  , 0  , 0  , 0  ), // #171 {xmm|m128|mem, xmm}
  ROW(2, 1, 1, 0, 49 , 48 , 0  , 0  , 0  , 0  ), //      {ymm|m256|mem, ymm}
  ROW(2, 1, 1, 0, 52 , 51 , 0  , 0  , 0  , 0  ), //      {zmm|m512|mem, zmm}
  ROW(2, 1, 1, 0, 45 , 60 , 0  , 0  , 0  , 0  ), // #174 {xmm, xmm|m64|mem}
  ROW(2, 1, 1, 0, 48 , 46 , 0  , 0  , 0  , 0  ), //      {ymm, xmm|m128|mem}
  ROW(2, 1, 1, 0, 51 , 49 , 0  , 0  , 0  , 0  ), //      {zmm, ymm|m256|mem}
  ROW(2, 1, 1, 0, 45 , 46 , 0  , 0  , 0  , 0  ), // #177 {xmm, xmm|m128|mem}
  ROW(2, 1, 1, 0, 48 , 49 , 0  , 0  , 0  , 0  ), //      {ymm, ymm|m256|mem}
  ROW(2, 1, 1, 0, 51 , 52 , 0  , 0  , 0  , 0  ), //      {zmm, zmm|m512|mem}
  ROW(3, 1, 1, 0, 60 , 45 , 10 , 0  , 0  , 0  ), // #180 {xmm|m64|mem, xmm, i8|u8}
  ROW(3, 1, 1, 0, 46 , 48 , 10 , 0  , 0  , 0  ), // #181 {xmm|m128|mem, ymm, i8|u8}
  ROW(3, 1, 1, 0, 49 , 51 , 10 , 0  , 0  , 0  ), // #182 {ymm|m256|mem, zmm, i8|u8}
  ROW(3, 1, 1, 0, 45 , 46 , 10 , 0  , 0  , 0  ), // #183 {xmm, xmm|m128|mem, i8|u8}
  ROW(3, 1, 1, 0, 48 , 49 , 10 , 0  , 0  , 0  ), //      {ymm, ymm|m256|mem, i8|u8}
  ROW(3, 1, 1, 0, 51 , 52 , 10 , 0  , 0  , 0  ), //      {zmm, zmm|m512|mem, i8|u8}
  ROW(2, 1, 1, 0, 45 , 60 , 0  , 0  , 0  , 0  ), // #186 {xmm, xmm|m64|mem}
  ROW(2, 1, 1, 0, 48 , 49 , 0  , 0  , 0  , 0  ), //      {ymm, ymm|m256|mem}
  ROW(2, 1, 1, 0, 51 , 52 , 0  , 0  , 0  , 0  ), //      {zmm, zmm|m512|mem}
  ROW(2, 1, 1, 0, 47 , 45 , 0  , 0  , 0  , 0  ), // #189 {m128|mem, xmm}
  ROW(2, 1, 1, 0, 50 , 48 , 0  , 0  , 0  , 0  ), //      {m256|mem, ymm}
  ROW(2, 1, 1, 0, 53 , 51 , 0  , 0  , 0  , 0  ), //      {m512|mem, zmm}
  ROW(2, 1, 1, 0, 45 , 47 , 0  , 0  , 0  , 0  ), // #192 {xmm, m128|mem}
  ROW(2, 1, 1, 0, 48 , 50 , 0  , 0  , 0  , 0  ), //      {ymm, m256|mem}
  ROW(2, 1, 1, 0, 51 , 53 , 0  , 0  , 0  , 0  ), //      {zmm, m512|mem}
  ROW(2, 0, 1, 0, 15 , 45 , 0  , 0  , 0  , 0  ), // #195 {r64|m64|mem, xmm}
  ROW(2, 1, 1, 0, 45 , 85 , 0  , 0  , 0  , 0  ), //      {xmm, xmm|m64|mem|r64}
  ROW(2, 1, 1, 0, 30 , 45 , 0  , 0  , 0  , 0  ), //      {m64|mem, xmm}
  ROW(2, 1, 1, 0, 30 , 45 , 0  , 0  , 0  , 0  ), // #198 {m64|mem, xmm}
  ROW(2, 1, 1, 0, 45 , 30 , 0  , 0  , 0  , 0  ), //      {xmm, m64|mem}
  ROW(3, 1, 1, 0, 45 , 45 , 45 , 0  , 0  , 0  ), // #200 {xmm, xmm, xmm}
  ROW(2, 1, 1, 0, 29 , 45 , 0  , 0  , 0  , 0  ), // #201 {m32|mem, xmm}
  ROW(2, 1, 1, 0, 45 , 29 , 0  , 0  , 0  , 0  ), //      {xmm, m32|mem}
  ROW(3, 1, 1, 0, 45 , 45 , 45 , 0  , 0  , 0  ), //      {xmm, xmm, xmm}
  ROW(2, 1, 1, 0, 86 , 85 , 0  , 0  , 0  , 0  ), // #204 {xmm|ymm, xmm|m64|mem|r64}
  ROW(2, 0, 1, 0, 51 , 8  , 0  , 0  , 0  , 0  ), //      {zmm, r64}
  ROW(2, 1, 1, 0, 51 , 60 , 0  , 0  , 0  , 0  ), //      {zmm, xmm|m64|mem}
  ROW(4, 1, 1, 0, 84 , 45 , 46 , 10 , 0  , 0  ), // #207 {k, xmm, xmm|m128|mem, i8|u8}
  ROW(4, 1, 1, 0, 84 , 48 , 49 , 10 , 0  , 0  ), //      {k, ymm, ymm|m256|mem, i8|u8}
  ROW(4, 1, 1, 0, 84 , 51 , 52 , 10 , 0  , 0  ), //      {k, zmm, zmm|m512|mem, i8|u8}
  ROW(3, 1, 1, 0, 82 , 45 , 46 , 0  , 0  , 0  ), // #210 {xmm|k, xmm, xmm|m128|mem}
  ROW(3, 1, 1, 0, 83 , 48 , 49 , 0  , 0  , 0  ), //      {ymm|k, ymm, ymm|m256|mem}
  ROW(3, 1, 1, 0, 84 , 51 , 52 , 0  , 0  , 0  ), //      {k, zmm, zmm|m512|mem}
  ROW(2, 1, 1, 0, 87 , 45 , 0  , 0  , 0  , 0  ), // #213 {xmm|m32|mem, xmm}
  ROW(2, 1, 1, 0, 60 , 48 , 0  , 0  , 0  , 0  ), //      {xmm|m64|mem, ymm}
  ROW(2, 1, 1, 0, 46 , 51 , 0  , 0  , 0  , 0  ), //      {xmm|m128|mem, zmm}
  ROW(2, 1, 1, 0, 60 , 45 , 0  , 0  , 0  , 0  ), // #216 {xmm|m64|mem, xmm}
  ROW(2, 1, 1, 0, 46 , 48 , 0  , 0  , 0  , 0  ), //      {xmm|m128|mem, ymm}
  ROW(2, 1, 1, 0, 49 , 51 , 0  , 0  , 0  , 0  ), //      {ymm|m256|mem, zmm}
  ROW(2, 1, 1, 0, 88 , 45 , 0  , 0  , 0  , 0  ), // #219 {xmm|m16|mem, xmm}
  ROW(2, 1, 1, 0, 87 , 48 , 0  , 0  , 0  , 0  ), //      {xmm|m32|mem, ymm}
  ROW(2, 1, 1, 0, 60 , 51 , 0  , 0  , 0  , 0  ), //      {xmm|m64|mem, zmm}
  ROW(2, 1, 1, 0, 45 , 87 , 0  , 0  , 0  , 0  ), // #222 {xmm, xmm|m32|mem}
  ROW(2, 1, 1, 0, 48 , 60 , 0  , 0  , 0  , 0  ), //      {ymm, xmm|m64|mem}
  ROW(2, 1, 1, 0, 51 , 46 , 0  , 0  , 0  , 0  ), //      {zmm, xmm|m128|mem}
  ROW(2, 1, 1, 0, 45 , 88 , 0  , 0  , 0  , 0  ), // #225 {xmm, xmm|m16|mem}
  ROW(2, 1, 1, 0, 48 , 87 , 0  , 0  , 0  , 0  ), //      {ymm, xmm|m32|mem}
  ROW(2, 1, 1, 0, 51 , 60 , 0  , 0  , 0  , 0  ), //      {zmm, xmm|m64|mem}
  ROW(2, 1, 1, 0, 61 , 45 , 0  , 0  , 0  , 0  ), // #228 {vm32x, xmm}
  ROW(2, 1, 1, 0, 62 , 48 , 0  , 0  , 0  , 0  ), //      {vm32y, ymm}
  ROW(2, 1, 1, 0, 63 , 51 , 0  , 0  , 0  , 0  ), //      {vm32z, zmm}
  ROW(2, 1, 1, 0, 64 , 45 , 0  , 0  , 0  , 0  ), // #231 {vm64x, xmm}
  ROW(2, 1, 1, 0, 65 , 48 , 0  , 0  , 0  , 0  ), //      {vm64y, ymm}
  ROW(2, 1, 1, 0, 66 , 51 , 0  , 0  , 0  , 0  ), //      {vm64z, zmm}
  ROW(3, 1, 1, 0, 84 , 45 , 46 , 0  , 0  , 0  ), // #234 {k, xmm, xmm|m128|mem}
  ROW(3, 1, 1, 0, 84 , 48 , 49 , 0  , 0  , 0  ), //      {k, ymm, ymm|m256|mem}
  ROW(3, 1, 1, 0, 84 , 51 , 52 , 0  , 0  , 0  ), //      {k, zmm, zmm|m512|mem}
  ROW(3, 1, 1, 0, 6  , 6  , 28 , 0  , 0  , 0  ), // #237 {r32, r32, r32|m32|mem}
  ROW(3, 0, 1, 0, 8  , 8  , 15 , 0  , 0  , 0  ), //      {r64, r64, r64|m64|mem}
  ROW(3, 1, 1, 0, 6  , 28 , 6  , 0  , 0  , 0  ), // #239 {r32, r32|m32|mem, r32}
  ROW(3, 0, 1, 0, 8  , 15 , 8  , 0  , 0  , 0  ), //      {r64, r64|m64|mem, r64}
  ROW(2, 1, 0, 0, 89 , 28 , 0  , 0  , 0  , 0  ), // #241 {bnd, r32|m32|mem}
  ROW(2, 0, 1, 0, 89 , 15 , 0  , 0  , 0  , 0  ), //      {bnd, r64|m64|mem}
  ROW(2, 1, 1, 0, 89 , 90 , 0  , 0  , 0  , 0  ), // #243 {bnd, bnd|mem}
  ROW(2, 1, 1, 0, 91 , 89 , 0  , 0  , 0  , 0  ), //      {mem, bnd}
  ROW(2, 1, 0, 0, 4  , 29 , 0  , 0  , 0  , 0  ), // #245 {r16, m32|mem}
  ROW(2, 1, 0, 0, 6  , 30 , 0  , 0  , 0  , 0  ), //      {r32, m64|mem}
  ROW(1, 1, 0, 0, 92 , 0  , 0  , 0  , 0  , 0  ), // #247 {rel16|r16|m16|r32|m32}
  ROW(1, 1, 1, 0, 93 , 0  , 0  , 0  , 0  , 0  ), //      {rel32|r64|m64|mem}
  ROW(2, 1, 1, 0, 6  , 94 , 0  , 0  , 0  , 0  ), // #249 {r32, r8lo|r8hi|m8|r16|m16|r32|m32}
  ROW(2, 0, 1, 0, 8  , 95 , 0  , 0  , 0  , 0  ), //      {r64, r8lo|r8hi|m8|r64|m64}
  ROW(1, 1, 0, 0, 96 , 0  , 0  , 0  , 0  , 0  ), // #251 {r16|r32}
  ROW(1, 1, 1, 0, 31 , 0  , 0  , 0  , 0  , 0  ), // #252 {r8lo|r8hi|m8|r16|m16|r32|m32|r64|m64|mem}
  ROW(3, 1, 1, 0, 45 , 10 , 10 , 0  , 0  , 0  ), // #253 {xmm, i8|u8, i8|u8}
  ROW(2, 1, 1, 0, 45 , 45 , 0  , 0  , 0  , 0  ), // #254 {xmm, xmm}
  ROW(0, 1, 1, 0, 0  , 0  , 0  , 0  , 0  , 0  ), // #255 {}
  ROW(1, 1, 1, 0, 78 , 0  , 0  , 0  , 0  , 0  ), // #256 {st}
  ROW(0, 1, 1, 0, 0  , 0  , 0  , 0  , 0  , 0  ), // #257 {}
  ROW(1, 1, 1, 0, 97 , 0  , 0  , 0  , 0  , 0  ), // #258 {m32|m64|st}
  ROW(2, 1, 1, 0, 45 , 45 , 0  , 0  , 0  , 0  ), // #259 {xmm, xmm}
  ROW(4, 1, 1, 0, 45 , 45 , 10 , 10 , 0  , 0  ), //      {xmm, xmm, i8|u8, i8|u8}
  ROW(2, 1, 0, 0, 6  , 47 , 0  , 0  , 0  , 0  ), // #261 {r32, m128|mem}
  ROW(2, 0, 1, 0, 8  , 47 , 0  , 0  , 0  , 0  ), //      {r64, m128|mem}
  ROW(2, 1, 0, 2, 36 , 98 , 0  , 0  , 0  , 0  ), // #263 {<eax>, <ecx>}
  ROW(2, 0, 1, 2, 99 , 98 , 0  , 0  , 0  , 0  ), //      {<eax|rax>, <ecx>}
  ROW(1, 1, 1, 0, 100, 0  , 0  , 0  , 0  , 0  ), // #265 {rel8|rel32}
  ROW(1, 1, 0, 0, 101, 0  , 0  , 0  , 0  , 0  ), //      {rel16}
  ROW(2, 1, 0, 1, 102, 103, 0  , 0  , 0  , 0  ), // #267 {<cx|ecx>, rel8}
  ROW(2, 0, 1, 1, 104, 103, 0  , 0  , 0  , 0  ), //      {<ecx|rcx>, rel8}
  ROW(1, 1, 1, 0, 105, 0  , 0  , 0  , 0  , 0  ), // #269 {rel8|rel32|r64|m64|mem}
  ROW(1, 1, 0, 0, 106, 0  , 0  , 0  , 0  , 0  ), //      {rel16|r32|m32|mem}
  ROW(2, 1, 1, 0, 84 , 107, 0  , 0  , 0  , 0  ), // #271 {k, k|m8|mem|r32|r8lo|r8hi|r16}
  ROW(2, 1, 1, 0, 108, 84 , 0  , 0  , 0  , 0  ), //      {m8|mem|r32|r8lo|r8hi|r16, k}
  ROW(2, 1, 1, 0, 84 , 109, 0  , 0  , 0  , 0  ), // #273 {k, k|m32|mem|r32}
  ROW(2, 1, 1, 0, 28 , 84 , 0  , 0  , 0  , 0  ), //      {m32|mem|r32, k}
  ROW(2, 1, 1, 0, 84 , 110, 0  , 0  , 0  , 0  ), // #275 {k, k|m64|mem|r64}
  ROW(2, 1, 1, 0, 15 , 84 , 0  , 0  , 0  , 0  ), //      {m64|mem|r64, k}
  ROW(2, 1, 1, 0, 84 , 111, 0  , 0  , 0  , 0  ), // #277 {k, k|m16|mem|r32|r16}
  ROW(2, 1, 1, 0, 112, 84 , 0  , 0  , 0  , 0  ), //      {m16|mem|r32|r16, k}
  ROW(2, 1, 1, 0, 4  , 27 , 0  , 0  , 0  , 0  ), // #279 {r16, r16|m16|mem}
  ROW(2, 1, 1, 0, 6  , 112, 0  , 0  , 0  , 0  ), //      {r32, r32|m16|mem|r16}
  ROW(2, 1, 0, 0, 4  , 29 , 0  , 0  , 0  , 0  ), // #281 {r16, m32|mem}
  ROW(2, 1, 0, 0, 6  , 79 , 0  , 0  , 0  , 0  ), //      {r32, m48|mem}
  ROW(2, 1, 1, 0, 4  , 27 , 0  , 0  , 0  , 0  ), // #283 {r16, r16|m16|mem}
  ROW(2, 1, 1, 0, 113, 112, 0  , 0  , 0  , 0  ), //      {r32|r64, r32|m16|mem|r16}
  ROW(2, 1, 1, 0, 59 , 28 , 0  , 0  , 0  , 0  ), // #285 {mm|xmm, r32|m32|mem}
  ROW(2, 1, 1, 0, 28 , 59 , 0  , 0  , 0  , 0  ), //      {r32|m32|mem, mm|xmm}
  ROW(2, 1, 0, 0, 114, 53 , 0  , 0  , 0  , 0  ), // #287 {es:[memBase], m512|mem}
  ROW(2, 0, 1, 0, 114, 53 , 0  , 0  , 0  , 0  ), //      {es:[memBase], m512|mem}
  ROW(2, 1, 1, 0, 45 , 87 , 0  , 0  , 0  , 0  ), // #289 {xmm, xmm|m32|mem}
  ROW(2, 1, 1, 0, 29 , 45 , 0  , 0  , 0  , 0  ), //      {m32|mem, xmm}
  ROW(2, 1, 1, 0, 4  , 9  , 0  , 0  , 0  , 0  ), // #291 {r16, r8lo|r8hi|m8}
  ROW(2, 1, 1, 0, 113, 115, 0  , 0  , 0  , 0  ), //      {r32|r64, r8lo|r8hi|m8|r16|m16}
  ROW(4, 1, 1, 1, 6  , 6  , 28 , 35 , 0  , 0  ), // #293 {r32, r32, r32|m32|mem, <edx>}
  ROW(4, 0, 1, 1, 8  , 8  , 15 , 37 , 0  , 0  ), //      {r64, r64, r64|m64|mem, <rdx>}
  ROW(0, 1, 1, 0, 0  , 0  , 0  , 0  , 0  , 0  ), // #295 {}
  ROW(1, 1, 1, 0, 116, 0  , 0  , 0  , 0  , 0  ), //      {r16|m16|r32|m32}
  ROW(2, 1, 1, 0, 57 , 117, 0  , 0  , 0  , 0  ), // #297 {mm, mm|m64|mem}
  ROW(2, 1, 1, 0, 45 , 46 , 0  , 0  , 0  , 0  ), //      {xmm, xmm|m128|mem}
  ROW(3, 1, 1, 0, 57 , 117, 10 , 0  , 0  , 0  ), // #299 {mm, mm|m64|mem, i8|u8}
  ROW(3, 1, 1, 0, 45 , 46 , 10 , 0  , 0  , 0  ), //      {xmm, xmm|m128|mem, i8|u8}
  ROW(3, 1, 1, 0, 6  , 59 , 10 , 0  , 0  , 0  ), // #301 {r32, mm|xmm, i8|u8}
  ROW(3, 1, 1, 0, 21 , 45 , 10 , 0  , 0  , 0  ), //      {m16|mem, xmm, i8|u8}
  ROW(2, 1, 1, 0, 57 , 118, 0  , 0  , 0  , 0  ), // #303 {mm, i8|u8|mm|m64|mem}
  ROW(2, 1, 1, 0, 45 , 54 , 0  , 0  , 0  , 0  ), //      {xmm, i8|u8|xmm|m128|mem}
  ROW(1, 1, 0, 0, 6  , 0  , 0  , 0  , 0  , 0  ), // #305 {r32}
  ROW(1, 0, 1, 0, 8  , 0  , 0  , 0  , 0  , 0  ), //      {r64}
  ROW(0, 1, 1, 0, 0  , 0  , 0  , 0  , 0  , 0  ), // #307 {}
  ROW(1, 1, 1, 0, 119, 0  , 0  , 0  , 0  , 0  ), //      {u16}
  ROW(3, 1, 1, 0, 6  , 28 , 10 , 0  , 0  , 0  ), // #309 {r32, r32|m32|mem, i8|u8}
  ROW(3, 0, 1, 0, 8  , 15 , 10 , 0  , 0  , 0  ), //      {r64, r64|m64|mem, i8|u8}
  ROW(4, 1, 1, 0, 45 , 45 , 46 , 45 , 0  , 0  ), // #311 {xmm, xmm, xmm|m128|mem, xmm}
  ROW(4, 1, 1, 0, 48 , 48 , 49 , 48 , 0  , 0  ), //      {ymm, ymm, ymm|m256|mem, ymm}
  ROW(2, 1, 1, 0, 45 , 120, 0  , 0  , 0  , 0  ), // #313 {xmm, xmm|m128|ymm|m256}
  ROW(2, 1, 1, 0, 48 , 52 , 0  , 0  , 0  , 0  ), //      {ymm, zmm|m512|mem}
  ROW(4, 1, 1, 0, 45 , 45 , 45 , 60 , 0  , 0  ), // #315 {xmm, xmm, xmm, xmm|m64|mem}
  ROW(4, 1, 1, 0, 45 , 45 , 30 , 45 , 0  , 0  ), //      {xmm, xmm, m64|mem, xmm}
  ROW(4, 1, 1, 0, 45 , 45 , 45 , 87 , 0  , 0  ), // #317 {xmm, xmm, xmm, xmm|m32|mem}
  ROW(4, 1, 1, 0, 45 , 45 , 29 , 45 , 0  , 0  ), //      {xmm, xmm, m32|mem, xmm}
  ROW(4, 1, 1, 0, 48 , 48 , 46 , 10 , 0  , 0  ), // #319 {ymm, ymm, xmm|m128|mem, i8|u8}
  ROW(4, 1, 1, 0, 51 , 51 , 46 , 10 , 0  , 0  ), //      {zmm, zmm, xmm|m128|mem, i8|u8}
  ROW(1, 1, 0, 1, 36 , 0  , 0  , 0  , 0  , 0  ), // #321 {<eax>}
  ROW(1, 0, 1, 1, 38 , 0  , 0  , 0  , 0  , 0  ), // #322 {<rax>}
  ROW(2, 1, 1, 0, 28 , 45 , 0  , 0  , 0  , 0  ), // #323 {r32|m32|mem, xmm}
  ROW(2, 1, 1, 0, 45 , 28 , 0  , 0  , 0  , 0  ), //      {xmm, r32|m32|mem}
  ROW(2, 1, 1, 0, 30 , 45 , 0  , 0  , 0  , 0  ), // #325 {m64|mem, xmm}
  ROW(3, 1, 1, 0, 45 , 45 , 30 , 0  , 0  , 0  ), //      {xmm, xmm, m64|mem}
  ROW(2, 1, 0, 0, 28 , 6  , 0  , 0  , 0  , 0  ), // #327 {r32|m32|mem, r32}
  ROW(2, 0, 1, 0, 15 , 8  , 0  , 0  , 0  , 0  ), //      {r64|m64|mem, r64}
  ROW(2, 1, 0, 0, 6  , 28 , 0  , 0  , 0  , 0  ), // #329 {r32, r32|m32|mem}
  ROW(2, 0, 1, 0, 8  , 15 , 0  , 0  , 0  , 0  ), //      {r64, r64|m64|mem}
  ROW(3, 1, 1, 0, 45 , 45 , 54 , 0  , 0  , 0  ), // #331 {xmm, xmm, xmm|m128|mem|i8|u8}
  ROW(3, 1, 1, 0, 45 , 47 , 121, 0  , 0  , 0  ), //      {xmm, m128|mem, i8|u8|xmm}
  ROW(2, 1, 1, 0, 74 , 45 , 0  , 0  , 0  , 0  ), // #333 {vm64x|vm64y, xmm}
  ROW(2, 1, 1, 0, 66 , 48 , 0  , 0  , 0  , 0  ), //      {vm64z, ymm}
  ROW(3, 1, 1, 0, 45 , 45 , 46 , 0  , 0  , 0  ), // #335 {xmm, xmm, xmm|m128|mem}
  ROW(3, 1, 1, 0, 45 , 47 , 45 , 0  , 0  , 0  ), //      {xmm, m128|mem, xmm}
  ROW(2, 1, 1, 0, 61 , 86 , 0  , 0  , 0  , 0  ), // #337 {vm32x, xmm|ymm}
  ROW(2, 1, 1, 0, 62 , 51 , 0  , 0  , 0  , 0  ), //      {vm32y, zmm}
  ROW(1, 1, 0, 1, 33 , 0  , 0  , 0  , 0  , 0  ), // #339 {<ax>}
  ROW(2, 1, 0, 1, 33 , 10 , 0  , 0  , 0  , 0  ), // #340 {<ax>, i8|u8}
  ROW(2, 1, 0, 0, 27 , 4  , 0  , 0  , 0  , 0  ), // #341 {r16|m16|mem, r16}
  ROW(3, 1, 1, 1, 45 , 46 , 122, 0  , 0  , 0  ), // #342 {xmm, xmm|m128|mem, <xmm0>}
  ROW(2, 1, 1, 0, 89 , 123, 0  , 0  , 0  , 0  ), // #343 {bnd, mib}
  ROW(2, 1, 1, 0, 89 , 91 , 0  , 0  , 0  , 0  ), // #344 {bnd, mem}
  ROW(2, 1, 1, 0, 123, 89 , 0  , 0  , 0  , 0  ), // #345 {mib, bnd}
  ROW(1, 1, 1, 0, 113, 0  , 0  , 0  , 0  , 0  ), // #346 {r32|r64}
  ROW(1, 1, 1, 1, 33 , 0  , 0  , 0  , 0  , 0  ), // #347 {<ax>}
  ROW(2, 1, 1, 2, 35 , 36 , 0  , 0  , 0  , 0  ), // #348 {<edx>, <eax>}
  ROW(1, 1, 1, 0, 91 , 0  , 0  , 0  , 0  , 0  ), // #349 {mem}
  ROW(1, 1, 1, 1, 124, 0  , 0  , 0  , 0  , 0  ), // #350 {<ds:[memBase|zax]>}
  ROW(2, 1, 1, 2, 125, 126, 0  , 0  , 0  , 0  ), // #351 {<ds:[memBase|zsi]>, <es:[memBase|zdi]>}
  ROW(3, 1, 1, 0, 45 , 60 , 10 , 0  , 0  , 0  ), // #352 {xmm, xmm|m64|mem, i8|u8}
  ROW(3, 1, 1, 0, 45 , 87 , 10 , 0  , 0  , 0  ), // #353 {xmm, xmm|m32|mem, i8|u8}
  ROW(5, 0, 1, 4, 47 , 37 , 38 , 127, 128, 0  ), // #354 {m128|mem, <rdx>, <rax>, <rcx>, <rbx>}
  ROW(5, 1, 1, 4, 30 , 35 , 36 , 98 , 129, 0  ), // #355 {m64|mem, <edx>, <eax>, <ecx>, <ebx>}
  ROW(4, 1, 1, 4, 36 , 129, 98 , 35 , 0  , 0  ), // #356 {<eax>, <ebx>, <ecx>, <edx>}
  ROW(2, 0, 1, 2, 37 , 38 , 0  , 0  , 0  , 0  ), // #357 {<rdx>, <rax>}
  ROW(2, 1, 1, 0, 57 , 46 , 0  , 0  , 0  , 0  ), // #358 {mm, xmm|m128|mem}
  ROW(2, 1, 1, 0, 45 , 117, 0  , 0  , 0  , 0  ), // #359 {xmm, mm|m64|mem}
  ROW(2, 1, 1, 0, 57 , 60 , 0  , 0  , 0  , 0  ), // #360 {mm, xmm|m64|mem}
  ROW(2, 1, 1, 0, 113, 60 , 0  , 0  , 0  , 0  ), // #361 {r32|r64, xmm|m64|mem}
  ROW(2, 1, 1, 0, 45 , 130, 0  , 0  , 0  , 0  ), // #362 {xmm, r32|m32|mem|r64|m64}
  ROW(2, 1, 1, 0, 113, 87 , 0  , 0  , 0  , 0  ), // #363 {r32|r64, xmm|m32|mem}
  ROW(2, 1, 1, 2, 34 , 33 , 0  , 0  , 0  , 0  ), // #364 {<dx>, <ax>}
  ROW(1, 1, 1, 1, 36 , 0  , 0  , 0  , 0  , 0  ), // #365 {<eax>}
  ROW(2, 1, 1, 0, 12 , 10 , 0  , 0  , 0  , 0  ), // #366 {i16|u16, i8|u8}
  ROW(3, 1, 1, 0, 28 , 45 , 10 , 0  , 0  , 0  ), // #367 {r32|m32|mem, xmm, i8|u8}
  ROW(1, 1, 1, 0, 80 , 0  , 0  , 0  , 0  , 0  ), // #368 {m80|mem}
  ROW(1, 1, 1, 0, 131, 0  , 0  , 0  , 0  , 0  ), // #369 {m16|m32}
  ROW(1, 1, 1, 0, 132, 0  , 0  , 0  , 0  , 0  ), // #370 {m16|m32|m64}
  ROW(1, 1, 1, 0, 133, 0  , 0  , 0  , 0  , 0  ), // #371 {m32|m64|m80|st}
  ROW(1, 1, 1, 0, 21 , 0  , 0  , 0  , 0  , 0  ), // #372 {m16|mem}
  ROW(1, 1, 1, 0, 134, 0  , 0  , 0  , 0  , 0  ), // #373 {ax|m16|mem}
  ROW(1, 0, 1, 0, 91 , 0  , 0  , 0  , 0  , 0  ), // #374 {mem}
  ROW(2, 1, 1, 0, 135, 136, 0  , 0  , 0  , 0  ), // #375 {al|ax|eax, i8|u8|dx}
  ROW(2, 1, 1, 0, 137, 138, 0  , 0  , 0  , 0  ), // #376 {es:[memBase|zdi], dx}
  ROW(1, 1, 1, 0, 10 , 0  , 0  , 0  , 0  , 0  ), // #377 {i8|u8}
  ROW(0, 1, 0, 0, 0  , 0  , 0  , 0  , 0  , 0  ), // #378 {}
  ROW(0, 0, 1, 0, 0  , 0  , 0  , 0  , 0  , 0  ), // #379 {}
  ROW(3, 1, 1, 0, 84 , 84 , 84 , 0  , 0  , 0  ), // #380 {k, k, k}
  ROW(2, 1, 1, 0, 84 , 84 , 0  , 0  , 0  , 0  ), // #381 {k, k}
  ROW(3, 1, 1, 0, 84 , 84 , 10 , 0  , 0  , 0  ), // #382 {k, k, i8|u8}
  ROW(1, 1, 1, 1, 139, 0  , 0  , 0  , 0  , 0  ), // #383 {<ah>}
  ROW(1, 1, 1, 0, 29 , 0  , 0  , 0  , 0  , 0  ), // #384 {m32|mem}
  ROW(2, 1, 1, 0, 140, 141, 0  , 0  , 0  , 0  ), // #385 {r16|r32|r64, mem|m8|m16|m32|m48|m64|m80|m128|m256|m512|m1024}
  ROW(1, 1, 1, 0, 27 , 0  , 0  , 0  , 0  , 0  ), // #386 {r16|m16|mem}
  ROW(2, 1, 1, 2, 142, 125, 0  , 0  , 0  , 0  ), // #387 {<al|ax|eax|rax>, <ds:[memBase|zsi]>}
  ROW(3, 1, 1, 0, 113, 28 , 14 , 0  , 0  , 0  ), // #388 {r32|r64, r32|m32|mem, i32|u32}
  ROW(3, 1, 1, 1, 45 , 45 , 143, 0  , 0  , 0  ), // #389 {xmm, xmm, <ds:[memBase|zdi]>}
  ROW(3, 1, 1, 1, 57 , 57 , 143, 0  , 0  , 0  ), // #390 {mm, mm, <ds:[memBase|zdi]>}
  ROW(3, 1, 1, 3, 124, 98 , 35 , 0  , 0  , 0  ), // #391 {<ds:[memBase|zax]>, <ecx>, <edx>}
  ROW(2, 1, 1, 0, 57 , 45 , 0  , 0  , 0  , 0  ), // #392 {mm, xmm}
  ROW(2, 1, 1, 0, 6  , 45 , 0  , 0  , 0  , 0  ), // #393 {r32, xmm}
  ROW(2, 1, 1, 0, 30 , 57 , 0  , 0  , 0  , 0  ), // #394 {m64|mem, mm}
  ROW(2, 1, 1, 0, 45 , 57 , 0  , 0  , 0  , 0  ), // #395 {xmm, mm}
  ROW(2, 1, 1, 2, 126, 125, 0  , 0  , 0  , 0  ), // #396 {<es:[memBase|zdi]>, <ds:[memBase|zsi]>}
  ROW(2, 0, 1, 0, 8  , 28 , 0  , 0  , 0  , 0  ), // #397 {r64, r32|m32|mem}
  ROW(2, 1, 1, 2, 36 , 98 , 0  , 0  , 0  , 0  ), // #398 {<eax>, <ecx>}
  ROW(3, 1, 1, 3, 36 , 98 , 129, 0  , 0  , 0  ), // #399 {<eax>, <ecx>, <ebx>}
  ROW(2, 1, 1, 0, 144, 135, 0  , 0  , 0  , 0  ), // #400 {u8|dx, al|ax|eax}
  ROW(2, 1, 1, 0, 138, 145, 0  , 0  , 0  , 0  ), // #401 {dx, ds:[memBase|zsi]}
  ROW(6, 1, 1, 3, 45 , 46 , 10 , 98 , 36 , 35 ), // #402 {xmm, xmm|m128|mem, i8|u8, <ecx>, <eax>, <edx>}
  ROW(6, 1, 1, 3, 45 , 46 , 10 , 122, 36 , 35 ), // #403 {xmm, xmm|m128|mem, i8|u8, <xmm0>, <eax>, <edx>}
  ROW(4, 1, 1, 1, 45 , 46 , 10 , 98 , 0  , 0  ), // #404 {xmm, xmm|m128|mem, i8|u8, <ecx>}
  ROW(4, 1, 1, 1, 45 , 46 , 10 , 122, 0  , 0  ), // #405 {xmm, xmm|m128|mem, i8|u8, <xmm0>}
  ROW(3, 1, 1, 0, 108, 45 , 10 , 0  , 0  , 0  ), // #406 {r32|m8|mem|r8lo|r8hi|r16, xmm, i8|u8}
  ROW(3, 0, 1, 0, 15 , 45 , 10 , 0  , 0  , 0  ), // #407 {r64|m64|mem, xmm, i8|u8}
  ROW(3, 1, 1, 0, 45 , 108, 10 , 0  , 0  , 0  ), // #408 {xmm, r32|m8|mem|r8lo|r8hi|r16, i8|u8}
  ROW(3, 1, 1, 0, 45 , 28 , 10 , 0  , 0  , 0  ), // #409 {xmm, r32|m32|mem, i8|u8}
  ROW(3, 0, 1, 0, 45 , 15 , 10 , 0  , 0  , 0  ), // #410 {xmm, r64|m64|mem, i8|u8}
  ROW(3, 1, 1, 0, 59 , 112, 10 , 0  , 0  , 0  ), // #411 {mm|xmm, r32|m16|mem|r16, i8|u8}
  ROW(2, 1, 1, 0, 6  , 59 , 0  , 0  , 0  , 0  ), // #412 {r32, mm|xmm}
  ROW(2, 1, 1, 0, 45 , 10 , 0  , 0  , 0  , 0  ), // #413 {xmm, i8|u8}
  ROW(2, 1, 1, 0, 31 , 81 , 0  , 0  , 0  , 0  ), // #414 {r8lo|r8hi|m8|r16|m16|r32|m32|r64|m64|mem, cl|i8|u8}
  ROW(1, 0, 1, 0, 113, 0  , 0  , 0  , 0  , 0  ), // #415 {r32|r64}
  ROW(3, 1, 1, 3, 35 , 36 , 98 , 0  , 0  , 0  ), // #416 {<edx>, <eax>, <ecx>}
  ROW(1, 1, 1, 0, 140, 0  , 0  , 0  , 0  , 0  ), // #417 {r16|r32|r64}
  ROW(2, 1, 1, 2, 142, 126, 0  , 0  , 0  , 0  ), // #418 {<al|ax|eax|rax>, <es:[memBase|zdi]>}
  ROW(1, 1, 1, 0, 1  , 0  , 0  , 0  , 0  , 0  ), // #419 {r8lo|r8hi|m8|mem}
  ROW(1, 1, 1, 0, 146, 0  , 0  , 0  , 0  , 0  ), // #420 {r16|m16|mem|r32|r64}
  ROW(2, 1, 1, 2, 126, 142, 0  , 0  , 0  , 0  ), // #421 {<es:[memBase|zdi]>, <al|ax|eax|rax>}
  ROW(6, 1, 1, 0, 51 , 51 , 51 , 51 , 51 , 47 ), // #422 {zmm, zmm, zmm, zmm, zmm, m128|mem}
  ROW(6, 1, 1, 0, 45 , 45 , 45 , 45 , 45 , 47 ), // #423 {xmm, xmm, xmm, xmm, xmm, m128|mem}
  ROW(3, 1, 1, 0, 45 , 45 , 60 , 0  , 0  , 0  ), // #424 {xmm, xmm, xmm|m64|mem}
  ROW(3, 1, 1, 0, 45 , 45 , 87 , 0  , 0  , 0  ), // #425 {xmm, xmm, xmm|m32|mem}
  ROW(2, 1, 1, 0, 48 , 47 , 0  , 0  , 0  , 0  ), // #426 {ymm, m128|mem}
  ROW(2, 1, 1, 0, 147, 60 , 0  , 0  , 0  , 0  ), // #427 {ymm|zmm, xmm|m64|mem}
  ROW(2, 1, 1, 0, 147, 47 , 0  , 0  , 0  , 0  ), // #428 {ymm|zmm, m128|mem}
  ROW(2, 1, 1, 0, 51 , 50 , 0  , 0  , 0  , 0  ), // #429 {zmm, m256|mem}
  ROW(2, 1, 1, 0, 148, 60 , 0  , 0  , 0  , 0  ), // #430 {xmm|ymm|zmm, xmm|m64|mem}
  ROW(2, 1, 1, 0, 148, 87 , 0  , 0  , 0  , 0  ), // #431 {xmm|ymm|zmm, m32|mem|xmm}
  ROW(4, 1, 1, 0, 82 , 45 , 60 , 10 , 0  , 0  ), // #432 {xmm|k, xmm, xmm|m64|mem, i8|u8}
  ROW(4, 1, 1, 0, 82 , 45 , 87 , 10 , 0  , 0  ), // #433 {xmm|k, xmm, xmm|m32|mem, i8|u8}
  ROW(3, 1, 1, 0, 45 , 45 , 130, 0  , 0  , 0  ), // #434 {xmm, xmm, r32|m32|mem|r64|m64}
  ROW(3, 1, 1, 0, 46 , 147, 10 , 0  , 0  , 0  ), // #435 {xmm|m128|mem, ymm|zmm, i8|u8}
  ROW(4, 1, 1, 0, 45 , 45 , 60 , 10 , 0  , 0  ), // #436 {xmm, xmm, xmm|m64|mem, i8|u8}
  ROW(4, 1, 1, 0, 45 , 45 , 87 , 10 , 0  , 0  ), // #437 {xmm, xmm, xmm|m32|mem, i8|u8}
  ROW(3, 1, 1, 0, 84 , 149, 10 , 0  , 0  , 0  ), // #438 {k, xmm|m128|ymm|m256|zmm|m512, i8|u8}
  ROW(3, 1, 1, 0, 84 , 60 , 10 , 0  , 0  , 0  ), // #439 {k, xmm|m64|mem, i8|u8}
  ROW(3, 1, 1, 0, 84 , 87 , 10 , 0  , 0  , 0  ), // #440 {k, xmm|m32|mem, i8|u8}
  ROW(1, 1, 1, 0, 62 , 0  , 0  , 0  , 0  , 0  ), // #441 {vm32y}
  ROW(1, 1, 1, 0, 63 , 0  , 0  , 0  , 0  , 0  ), // #442 {vm32z}
  ROW(1, 1, 1, 0, 66 , 0  , 0  , 0  , 0  , 0  ), // #443 {vm64z}
  ROW(4, 1, 1, 0, 51 , 51 , 49 , 10 , 0  , 0  ), // #444 {zmm, zmm, ymm|m256|mem, i8|u8}
  ROW(1, 1, 1, 0, 30 , 0  , 0  , 0  , 0  , 0  ), // #445 {m64|mem}
  ROW(2, 1, 1, 0, 6  , 86 , 0  , 0  , 0  , 0  ), // #446 {r32, xmm|ymm}
  ROW(2, 1, 1, 0, 148, 150, 0  , 0  , 0  , 0  ), // #447 {xmm|ymm|zmm, xmm|m8|mem|r32|r8lo|r8hi|r16}
  ROW(2, 1, 1, 0, 148, 151, 0  , 0  , 0  , 0  ), // #448 {xmm|ymm|zmm, xmm|m32|mem|r32}
  ROW(2, 1, 1, 0, 148, 84 , 0  , 0  , 0  , 0  ), // #449 {xmm|ymm|zmm, k}
  ROW(2, 1, 1, 0, 148, 152, 0  , 0  , 0  , 0  ), // #450 {xmm|ymm|zmm, xmm|m16|mem|r32|r16}
  ROW(3, 1, 1, 0, 112, 45 , 10 , 0  , 0  , 0  ), // #451 {r32|m16|mem|r16, xmm, i8|u8}
  ROW(4, 1, 1, 0, 45 , 45 , 108, 10 , 0  , 0  ), // #452 {xmm, xmm, r32|m8|mem|r8lo|r8hi|r16, i8|u8}
  ROW(4, 1, 1, 0, 45 , 45 , 28 , 10 , 0  , 0  ), // #453 {xmm, xmm, r32|m32|mem, i8|u8}
  ROW(4, 0, 1, 0, 45 , 45 , 15 , 10 , 0  , 0  ), // #454 {xmm, xmm, r64|m64|mem, i8|u8}
  ROW(4, 1, 1, 0, 45 , 45 , 112, 10 , 0  , 0  ), // #455 {xmm, xmm, r32|m16|mem|r16, i8|u8}
  ROW(2, 1, 1, 0, 84 , 148, 0  , 0  , 0  , 0  ), // #456 {k, xmm|ymm|zmm}
  ROW(1, 1, 1, 0, 101, 0  , 0  , 0  , 0  , 0  ), // #457 {rel16|rel32}
  ROW(3, 1, 1, 2, 91 , 35 , 36 , 0  , 0  , 0  ), // #458 {mem, <edx>, <eax>}
  ROW(3, 0, 1, 2, 91 , 35 , 36 , 0  , 0  , 0  )  // #459 {mem, <edx>, <eax>}
};
#undef ROW

#define ROW(flags, mFlags, extFlags, regId) { uint32_t(flags), uint16_t(mFlags), uint8_t(extFlags), uint8_t(regId) }
#define F(VAL) InstDB::kOp##VAL
#define M(VAL) InstDB::kMemOp##VAL
const InstDB::OpSignature InstDB::_opSignatureTable[] = {
  ROW(0, 0, 0, 0xFF),
  ROW(F(GpbLo) | F(GpbHi) | F(Mem), M(M8) | M(Any), 0, 0x00),
  ROW(F(GpbLo) | F(GpbHi), 0, 0, 0x00),
  ROW(F(Gpw) | F(SReg) | F(Mem), M(M16) | M(Any), 0, 0x00),
  ROW(F(Gpw), 0, 0, 0x00),
  ROW(F(Gpd) | F(SReg) | F(Mem), M(M32) | M(Any), 0, 0x00),
  ROW(F(Gpd), 0, 0, 0x00),
  ROW(F(Gpq) | F(SReg) | F(CReg) | F(DReg) | F(Mem), M(M64) | M(Any), 0, 0x00),
  ROW(F(Gpq), 0, 0, 0x00),
  ROW(F(GpbLo) | F(GpbHi) | F(Mem), M(M8), 0, 0x00),
  ROW(F(I8) | F(U8), 0, 0, 0x00),
  ROW(F(Gpw) | F(Mem), M(M16), 0, 0x00),
  ROW(F(I16) | F(U16), 0, 0, 0x00),
  ROW(F(Gpd) | F(Mem), M(M32), 0, 0x00),
  ROW(F(I32) | F(U32), 0, 0, 0x00),
  ROW(F(Gpq) | F(Mem), M(M64) | M(Any), 0, 0x00),
  ROW(F(I32), 0, 0, 0x00),
  ROW(F(SReg) | F(CReg) | F(DReg) | F(Mem) | F(I64) | F(U64), M(M64) | M(Any), 0, 0x00),
  ROW(F(Mem), M(M8) | M(Any), 0, 0x00),
  ROW(F(SReg) | F(Mem), M(M16) | M(Any), 0, 0x00),
  ROW(F(SReg) | F(Mem), M(M32) | M(Any), 0, 0x00),
  ROW(F(Mem), M(M16) | M(Any), 0, 0x00),
  ROW(F(SReg), 0, 0, 0x00),
  ROW(F(CReg) | F(DReg), 0, 0, 0x00),
  ROW(F(Gpq) | F(I32), 0, 0, 0x00),
  ROW(F(Gpw) | F(Gpd) | F(Gpq) | F(Mem), M(M16) | M(M32) | M(M64) | M(Any), 0, 0x00),
  ROW(F(I8), 0, 0, 0x00),
  ROW(F(Gpw) | F(Mem), M(M16) | M(Any), 0, 0x00),
  ROW(F(Gpd) | F(Mem), M(M32) | M(Any), 0, 0x00),
  ROW(F(Mem), M(M32) | M(Any), 0, 0x00),
  ROW(F(Mem), M(M64) | M(Any), 0, 0x00),
  ROW(F(GpbLo) | F(GpbHi) | F(Gpw) | F(Gpd) | F(Gpq) | F(Mem), M(M8) | M(M16) | M(M32) | M(M64) | M(Any), 0, 0x00),
  ROW(F(Gpq) | F(Mem) | F(I32) | F(U32), M(M64) | M(Any), 0, 0x00),
  ROW(F(Gpw) | F(Implicit), 0, 0, 0x01),
  ROW(F(Gpw) | F(Implicit), 0, 0, 0x04),
  ROW(F(Gpd) | F(Implicit), 0, 0, 0x04),
  ROW(F(Gpd) | F(Implicit), 0, 0, 0x01),
  ROW(F(Gpq) | F(Implicit), 0, 0, 0x04),
  ROW(F(Gpq) | F(Implicit), 0, 0, 0x01),
  ROW(F(Gpw) | F(Mem) | F(I8) | F(I16), M(M16) | M(Any), 0, 0x00),
  ROW(F(Gpd) | F(Mem) | F(I8) | F(I32), M(M32) | M(Any), 0, 0x00),
  ROW(F(Gpq) | F(Mem) | F(I8) | F(I32), M(M64) | M(Any), 0, 0x00),
  ROW(F(I8) | F(I16) | F(U16), 0, 0, 0x00),
  ROW(F(I8) | F(I32) | F(U32), 0, 0, 0x00),
  ROW(F(I8) | F(I32), 0, 0, 0x00),
  ROW(F(Xmm), 0, 0, 0x00),
  ROW(F(Xmm) | F(Mem), M(M128) | M(Any), 0, 0x00),
  ROW(F(Mem), M(M128) | M(Any), 0, 0x00),
  ROW(F(Ymm), 0, 0, 0x00),
  ROW(F(Ymm) | F(Mem), M(M256) | M(Any), 0, 0x00),
  ROW(F(Mem), M(M256) | M(Any), 0, 0x00),
  ROW(F(Zmm), 0, 0, 0x00),
  ROW(F(Zmm) | F(Mem), M(M512) | M(Any), 0, 0x00),
  ROW(F(Mem), M(M512) | M(Any), 0, 0x00),
  ROW(F(Xmm) | F(Mem) | F(I8) | F(U8), M(M128) | M(Any), 0, 0x00),
  ROW(F(Ymm) | F(Mem) | F(I8) | F(U8), M(M256) | M(Any), 0, 0x00),
  ROW(F(Zmm) | F(Mem) | F(I8) | F(U8), M(M512) | M(Any), 0, 0x00),
  ROW(F(Mm), 0, 0, 0x00),
  ROW(F(Gpq) | F(Mm) | F(Mem), M(M64) | M(Any), 0, 0x00),
  ROW(F(Xmm) | F(Mm), 0, 0, 0x00),
  ROW(F(Xmm) | F(Mem), M(M64) | M(Any), 0, 0x00),
  ROW(F(Vm), M(Vm32x), 0, 0x00),
  ROW(F(Vm), M(Vm32y), 0, 0x00),
  ROW(F(Vm), M(Vm32z), 0, 0x00),
  ROW(F(Vm), M(Vm64x), 0, 0x00),
  ROW(F(Vm), M(Vm64y), 0, 0x00),
  ROW(F(Vm), M(Vm64z), 0, 0x00),
  ROW(F(GpbLo) | F(Implicit), 0, 0, 0x01),
  ROW(F(Gpw) | F(Gpq) | F(Mem), M(M16) | M(M64) | M(Any), 0, 0x00),
  ROW(F(SReg), 0, 0, 0x1A),
  ROW(F(SReg), 0, 0, 0x60),
  ROW(F(Gpw) | F(Gpq) | F(Mem) | F(I8) | F(I16) | F(I32), M(M16) | M(M64) | M(Any), 0, 0x00),
  ROW(F(Gpd) | F(Mem) | F(I32) | F(U32), M(M32), 0, 0x00),
  ROW(F(SReg), 0, 0, 0x1E),
  ROW(F(Vm), M(Vm64x) | M(Vm64y), 0, 0x00),
  ROW(F(I4) | F(U4), 0, 0, 0x00),
  ROW(F(Mem), M(M32) | M(M64), 0, 0x00),
  ROW(F(St), 0, 0, 0x01),
  ROW(F(St), 0, 0, 0x00),
  ROW(F(Mem), M(M48) | M(Any), 0, 0x00),
  ROW(F(Mem), M(M80) | M(Any), 0, 0x00),
  ROW(F(GpbLo) | F(I8) | F(U8), 0, 0, 0x02),
  ROW(F(Xmm) | F(KReg), 0, 0, 0x00),
  ROW(F(Ymm) | F(KReg), 0, 0, 0x00),
  ROW(F(KReg), 0, 0, 0x00),
  ROW(F(Gpq) | F(Xmm) | F(Mem), M(M64) | M(Any), 0, 0x00),
  ROW(F(Xmm) | F(Ymm), 0, 0, 0x00),
  ROW(F(Xmm) | F(Mem), M(M32) | M(Any), 0, 0x00),
  ROW(F(Xmm) | F(Mem), M(M16) | M(Any), 0, 0x00),
  ROW(F(Bnd), 0, 0, 0x00),
  ROW(F(Bnd) | F(Mem), M(Any), 0, 0x00),
  ROW(F(Mem), M(Any), 0, 0x00),
  ROW(F(Gpw) | F(Gpd) | F(Mem) | F(I32) | F(I64) | F(Rel32), M(M16) | M(M32), 0, 0x00),
  ROW(F(Gpq) | F(Mem) | F(I32) | F(I64) | F(Rel32), M(M64) | M(Any), 0, 0x00),
  ROW(F(GpbLo) | F(GpbHi) | F(Gpw) | F(Gpd) | F(Mem), M(M8) | M(M16) | M(M32), 0, 0x00),
  ROW(F(GpbLo) | F(GpbHi) | F(Gpq) | F(Mem), M(M8) | M(M64), 0, 0x00),
  ROW(F(Gpw) | F(Gpd), 0, 0, 0x00),
  ROW(F(St) | F(Mem), M(M32) | M(M64), 0, 0x00),
  ROW(F(Gpd) | F(Implicit), 0, 0, 0x02),
  ROW(F(Gpd) | F(Gpq) | F(Implicit), 0, 0, 0x01),
  ROW(F(I32) | F(I64) | F(Rel8) | F(Rel32), 0, 0, 0x00),
  ROW(F(I32) | F(I64) | F(Rel32), 0, 0, 0x00),
  ROW(F(Gpw) | F(Gpd) | F(Implicit), 0, 0, 0x02),
  ROW(F(I32) | F(I64) | F(Rel8), 0, 0, 0x00),
  ROW(F(Gpd) | F(Gpq) | F(Implicit), 0, 0, 0x02),
  ROW(F(Gpq) | F(Mem) | F(I32) | F(I64) | F(Rel8) | F(Rel32), M(M64) | M(Any), 0, 0x00),
  ROW(F(Gpd) | F(Mem) | F(I32) | F(I64) | F(Rel32), M(M32) | M(Any), 0, 0x00),
  ROW(F(GpbLo) | F(GpbHi) | F(Gpw) | F(Gpd) | F(KReg) | F(Mem), M(M8) | M(Any), 0, 0x00),
  ROW(F(GpbLo) | F(GpbHi) | F(Gpw) | F(Gpd) | F(Mem), M(M8) | M(Any), 0, 0x00),
  ROW(F(Gpd) | F(KReg) | F(Mem), M(M32) | M(Any), 0, 0x00),
  ROW(F(Gpq) | F(KReg) | F(Mem), M(M64) | M(Any), 0, 0x00),
  ROW(F(Gpw) | F(Gpd) | F(KReg) | F(Mem), M(M16) | M(Any), 0, 0x00),
  ROW(F(Gpw) | F(Gpd) | F(Mem), M(M16) | M(Any), 0, 0x00),
  ROW(F(Gpd) | F(Gpq), 0, 0, 0x00),
  ROW(F(Mem), M(BaseOnly) | M(Es), 0, 0x00),
  ROW(F(GpbLo) | F(GpbHi) | F(Gpw) | F(Mem), M(M8) | M(M16), 0, 0x00),
  ROW(F(Gpw) | F(Gpd) | F(Mem), M(M16) | M(M32), 0, 0x00),
  ROW(F(Mm) | F(Mem), M(M64) | M(Any), 0, 0x00),
  ROW(F(Mm) | F(Mem) | F(I8) | F(U8), M(M64) | M(Any), 0, 0x00),
  ROW(F(U16), 0, 0, 0x00),
  ROW(F(Xmm) | F(Ymm) | F(Mem), M(M128) | M(M256), 0, 0x00),
  ROW(F(Xmm) | F(I8) | F(U8), 0, 0, 0x00),
  ROW(F(Xmm) | F(Implicit), 0, 0, 0x01),
  ROW(F(Mem), M(Mib), 0, 0x00),
  ROW(F(Mem) | F(Implicit), M(BaseOnly) | M(Ds), 0, 0x01),
  ROW(F(Mem) | F(Implicit), M(BaseOnly) | M(Ds), 0, 0x40),
  ROW(F(Mem) | F(Implicit), M(BaseOnly) | M(Es), 0, 0x80),
  ROW(F(Gpq) | F(Implicit), 0, 0, 0x02),
  ROW(F(Gpq) | F(Implicit), 0, 0, 0x08),
  ROW(F(Gpd) | F(Implicit), 0, 0, 0x08),
  ROW(F(Gpd) | F(Gpq) | F(Mem), M(M32) | M(M64) | M(Any), 0, 0x00),
  ROW(F(Mem), M(M16) | M(M32), 0, 0x00),
  ROW(F(Mem), M(M16) | M(M32) | M(M64), 0, 0x00),
  ROW(F(St) | F(Mem), M(M32) | M(M64) | M(M80), 0, 0x00),
  ROW(F(Gpw) | F(Mem), M(M16) | M(Any), 0, 0x01),
  ROW(F(GpbLo) | F(Gpw) | F(Gpd), 0, 0, 0x01),
  ROW(F(Gpw) | F(I8) | F(U8), 0, 0, 0x04),
  ROW(F(Mem), M(BaseOnly) | M(Es), 0, 0x80),
  ROW(F(Gpw), 0, 0, 0x04),
  ROW(F(GpbHi) | F(Implicit), 0, 0, 0x01),
  ROW(F(Gpw) | F(Gpd) | F(Gpq), 0, 0, 0x00),
  ROW(F(Mem), M(M8) | M(M16) | M(M32) | M(M48) | M(M64) | M(M80) | M(M128) | M(M256) | M(M512) | M(M1024) | M(Any), 0, 0x00),
  ROW(F(GpbLo) | F(Gpw) | F(Gpd) | F(Gpq) | F(Implicit), 0, 0, 0x01),
  ROW(F(Mem) | F(Implicit), M(BaseOnly) | M(Ds), 0, 0x80),
  ROW(F(Gpw) | F(U8), 0, 0, 0x04),
  ROW(F(Mem), M(BaseOnly) | M(Ds), 0, 0x40),
  ROW(F(Gpw) | F(Gpd) | F(Gpq) | F(Mem), M(M16) | M(Any), 0, 0x00),
  ROW(F(Ymm) | F(Zmm), 0, 0, 0x00),
  ROW(F(Xmm) | F(Ymm) | F(Zmm), 0, 0, 0x00),
  ROW(F(Xmm) | F(Ymm) | F(Zmm) | F(Mem), M(M128) | M(M256) | M(M512), 0, 0x00),
  ROW(F(GpbLo) | F(GpbHi) | F(Gpw) | F(Gpd) | F(Xmm) | F(Mem), M(M8) | M(Any), 0, 0x00),
  ROW(F(Gpd) | F(Xmm) | F(Mem), M(M32) | M(Any), 0, 0x00),
  ROW(F(Gpw) | F(Gpd) | F(Xmm) | F(Mem), M(M16) | M(Any), 0, 0x00)
};
#undef M
#undef F
#undef ROW
// ----------------------------------------------------------------------------
// ${InstSignatureTable:End}
#endif

// ============================================================================
// [asmjit::x86::InstInternal - Validate]
// ============================================================================

#ifndef ASMJIT_DISABLE_INST_API
struct X86ValidationData {
  //! Allowed registers by reg-type (x86::Reg::kType...).
  uint32_t allowedRegMask[Reg::kTypeMax + 1];
  uint32_t allowedMemBaseRegs;
  uint32_t allowedMemIndexRegs;
};

#define VALUE(X) \
  (X == Reg::kTypeGpbLo) ? InstDB::kOpGpbLo : \
  (X == Reg::kTypeGpbHi) ? InstDB::kOpGpbHi : \
  (X == Reg::kTypeGpw  ) ? InstDB::kOpGpw   : \
  (X == Reg::kTypeGpd  ) ? InstDB::kOpGpd   : \
  (X == Reg::kTypeGpq  ) ? InstDB::kOpGpq   : \
  (X == Reg::kTypeXmm  ) ? InstDB::kOpXmm   : \
  (X == Reg::kTypeYmm  ) ? InstDB::kOpYmm   : \
  (X == Reg::kTypeZmm  ) ? InstDB::kOpZmm   : \
  (X == Reg::kTypeMm   ) ? InstDB::kOpMm    : \
  (X == Reg::kTypeKReg ) ? InstDB::kOpKReg  : \
  (X == Reg::kTypeSReg ) ? InstDB::kOpSReg  : \
  (X == Reg::kTypeCReg ) ? InstDB::kOpCReg  : \
  (X == Reg::kTypeDReg ) ? InstDB::kOpDReg  : \
  (X == Reg::kTypeSt   ) ? InstDB::kOpSt    : \
  (X == Reg::kTypeBnd  ) ? InstDB::kOpBnd   : \
  (X == Reg::kTypeRip  ) ? InstDB::kOpNone  : InstDB::kOpNone
static const uint32_t _x86OpFlagFromRegType[Reg::kTypeMax + 1] = { ASMJIT_LOOKUP_TABLE_32(VALUE, 0) };
#undef VALUE

#define REG_MASK_FROM_REG_TYPE_X86(X) \
  (X == Reg::kTypeGpbLo) ? 0x0000000Fu : \
  (X == Reg::kTypeGpbHi) ? 0x0000000Fu : \
  (X == Reg::kTypeGpw  ) ? 0x000000FFu : \
  (X == Reg::kTypeGpd  ) ? 0x000000FFu : \
  (X == Reg::kTypeGpq  ) ? 0x000000FFu : \
  (X == Reg::kTypeXmm  ) ? 0x000000FFu : \
  (X == Reg::kTypeYmm  ) ? 0x000000FFu : \
  (X == Reg::kTypeZmm  ) ? 0x000000FFu : \
  (X == Reg::kTypeMm   ) ? 0x000000FFu : \
  (X == Reg::kTypeKReg ) ? 0x000000FFu : \
  (X == Reg::kTypeSReg ) ? 0x0000007Eu : \
  (X == Reg::kTypeCReg ) ? 0x0000FFFFu : \
  (X == Reg::kTypeDReg ) ? 0x000000FFu : \
  (X == Reg::kTypeSt   ) ? 0x000000FFu : \
  (X == Reg::kTypeBnd  ) ? 0x0000000Fu : \
  (X == Reg::kTypeRip  ) ? 0x00000001u : 0u

#define REG_MASK_FROM_REG_TYPE_X64(X) \
  (X == Reg::kTypeGpbLo) ? 0x0000FFFFu : \
  (X == Reg::kTypeGpbHi) ? 0x0000000Fu : \
  (X == Reg::kTypeGpw  ) ? 0x0000FFFFu : \
  (X == Reg::kTypeGpd  ) ? 0x0000FFFFu : \
  (X == Reg::kTypeGpq  ) ? 0x0000FFFFu : \
  (X == Reg::kTypeXmm  ) ? 0xFFFFFFFFu : \
  (X == Reg::kTypeYmm  ) ? 0xFFFFFFFFu : \
  (X == Reg::kTypeZmm  ) ? 0xFFFFFFFFu : \
  (X == Reg::kTypeMm   ) ? 0x000000FFu : \
  (X == Reg::kTypeKReg ) ? 0x000000FFu : \
  (X == Reg::kTypeSReg ) ? 0x0000007Eu : \
  (X == Reg::kTypeCReg ) ? 0x0000FFFFu : \
  (X == Reg::kTypeDReg ) ? 0x0000FFFFu : \
  (X == Reg::kTypeSt   ) ? 0x000000FFu : \
  (X == Reg::kTypeBnd  ) ? 0x0000000Fu : \
  (X == Reg::kTypeRip  ) ? 0x00000001u : 0u

static const X86ValidationData _x86ValidationData = {
  { ASMJIT_LOOKUP_TABLE_32(REG_MASK_FROM_REG_TYPE_X86, 0) },
  (1u << Reg::kTypeGpw) | (1u << Reg::kTypeGpd) | (1u << Reg::kTypeRip) | (1u << Label::kLabelTag),
  (1u << Reg::kTypeGpw) | (1u << Reg::kTypeGpd) | (1u << Reg::kTypeXmm) | (1u << Reg::kTypeYmm) | (1u << Reg::kTypeZmm)
};

static const X86ValidationData _x64ValidationData = {
  { ASMJIT_LOOKUP_TABLE_32(REG_MASK_FROM_REG_TYPE_X64, 0) },
  (1u << Reg::kTypeGpd) | (1u << Reg::kTypeGpq) | (1u << Reg::kTypeRip) | (1u << Label::kLabelTag),
  (1u << Reg::kTypeGpd) | (1u << Reg::kTypeGpq) | (1u << Reg::kTypeXmm) | (1u << Reg::kTypeYmm) | (1u << Reg::kTypeZmm)
};

#undef REG_MASK_FROM_REG_TYPE_X64
#undef REG_MASK_FROM_REG_TYPE_X86

static ASMJIT_INLINE bool x86IsZmmOrM512(const Operand_& op) noexcept {
  return Reg::isZmm(op) || (op.isMem() && op.size() == 64);
}

static ASMJIT_INLINE bool x86CheckOSig(const InstDB::OpSignature& op, const InstDB::OpSignature& ref, bool& immOutOfRange) noexcept {
  // Fail if operand types are incompatible.
  uint32_t opFlags = op.opFlags;
  if ((opFlags & ref.opFlags) == 0) {
    // Mark temporarily `immOutOfRange` so we can return a more descriptive error later.
    if ((opFlags & InstDB::kOpAllImm) && (ref.opFlags & InstDB::kOpAllImm)) {
      immOutOfRange = true;
      return true;
    }

    return false;
  }

  // Fail if memory specific flags and sizes do not match the signature.
  uint32_t opMemFlags = op.memFlags;
  if (opMemFlags != 0) {
    uint32_t refMemFlags = ref.memFlags;
    if ((refMemFlags & opMemFlags) == 0)
      return false;

    if ((refMemFlags & InstDB::kMemOpBaseOnly) && !(opMemFlags & InstDB::kMemOpBaseOnly))
      return false;
  }

  // Specific register index.
  if (opFlags & InstDB::kOpAllRegs) {
    uint32_t refRegMask = ref.regMask;
    if (refRegMask && !(op.regMask & refRegMask))
      return false;
  }

  return true;
}

ASMJIT_FAVOR_SIZE Error InstInternal::validate(uint32_t archId, const BaseInst& inst, const Operand_* operands, uint32_t opCount) noexcept {
  // Only called when `archId` matches X86 family.
  ASMJIT_ASSERT(ArchInfo::isX86Family(archId));

  const X86ValidationData* vd;
  if (archId == ArchInfo::kIdX86)
    vd = &_x86ValidationData;
  else
    vd = &_x64ValidationData;

  uint32_t i;
  uint32_t mode = InstDB::modeFromArchId(archId);

  // Get the instruction data.
  uint32_t instId = inst.id();
  uint32_t options = inst.options();

  if (ASMJIT_UNLIKELY(!Inst::isDefinedId(instId)))
    return DebugUtils::errored(kErrorInvalidInstruction);

  const InstDB::InstInfo& instInfo = InstDB::infoById(instId);
  const InstDB::CommonInfo& commonInfo = instInfo.commonInfo();

  uint32_t iFlags = instInfo.flags();

  // --------------------------------------------------------------------------
  // [Validate LOCK|XACQUIRE|XRELEASE]
  // --------------------------------------------------------------------------

  const uint32_t kLockXAcqRel = Inst::kOptionXAcquire | Inst::kOptionXRelease;
  if (options & (Inst::kOptionLock | kLockXAcqRel)) {
    if (options & Inst::kOptionLock) {
      if (ASMJIT_UNLIKELY(!(iFlags & InstDB::kFlagLock) && !(options & kLockXAcqRel)))
        return DebugUtils::errored(kErrorInvalidLockPrefix);

      if (ASMJIT_UNLIKELY(opCount < 1 || !operands[0].isMem()))
        return DebugUtils::errored(kErrorInvalidLockPrefix);
    }

    if (options & kLockXAcqRel) {
      if (ASMJIT_UNLIKELY(!(options & Inst::kOptionLock) || (options & kLockXAcqRel) == kLockXAcqRel))
        return DebugUtils::errored(kErrorInvalidPrefixCombination);

      if (ASMJIT_UNLIKELY((options & Inst::kOptionXAcquire) && !(iFlags & InstDB::kFlagXAcquire)))
        return DebugUtils::errored(kErrorInvalidXAcquirePrefix);

      if (ASMJIT_UNLIKELY((options & Inst::kOptionXRelease) && !(iFlags & InstDB::kFlagXRelease)))
        return DebugUtils::errored(kErrorInvalidXReleasePrefix);
    }
  }

  // Validate REP and REPNE prefixes.
  const uint32_t kRepAny = Inst::kOptionRep | Inst::kOptionRepne;
  if (options & kRepAny) {
    if (ASMJIT_UNLIKELY((options & kRepAny) == kRepAny))
      return DebugUtils::errored(kErrorInvalidPrefixCombination);

    if (ASMJIT_UNLIKELY(!(iFlags & InstDB::kFlagRep)))
      return DebugUtils::errored(kErrorInvalidRepPrefix);
  }

  // --------------------------------------------------------------------------
  // [Translate Each Operand to the Corresponding OpSignature]
  // --------------------------------------------------------------------------

  InstDB::OpSignature oSigTranslated[Globals::kMaxOpCount];
  uint32_t combinedOpFlags = 0;
  uint32_t combinedRegMask = 0;
  const Mem* memOp = nullptr;

  for (i = 0; i < opCount; i++) {
    const Operand_& op = operands[i];
    if (op.opType() == Operand::kOpNone)
      break;

    uint32_t opFlags = 0;
    uint32_t memFlags = 0;
    uint32_t regMask = 0;

    switch (op.opType()) {
      case Operand::kOpReg: {
        uint32_t regType = op.as<BaseReg>().type();
        if (ASMJIT_UNLIKELY(regType >= Reg::kTypeCount))
          return DebugUtils::errored(kErrorInvalidRegType);

        opFlags = _x86OpFlagFromRegType[regType];
        if (ASMJIT_UNLIKELY(opFlags == 0))
          return DebugUtils::errored(kErrorInvalidRegType);

        // If `regId` is equal or greater than Operand::kVirtIdMin it means
        // that the register is virtual and its index will be assigned later
        // by the register allocator. We must pass unless asked to disallow
        // virtual registers.
        // TODO: We need an option to refuse virtual regs here.
        uint32_t regId = op.id();
        if (regId < Operand::kVirtIdMin) {
          if (ASMJIT_UNLIKELY(regId >= 32))
            return DebugUtils::errored(kErrorInvalidPhysId);

          if (ASMJIT_UNLIKELY(Support::bitTest(vd->allowedRegMask[regType], regId) == 0))
            return DebugUtils::errored(kErrorInvalidPhysId);

          regMask = Support::bitMask(regId);
          combinedRegMask |= regMask;
        }
        else {
          regMask = 0xFFFFFFFFu;
        }
        break;
      }

      // TODO: Validate base and index and combine these with `combinedRegMask`.
      case Operand::kOpMem: {
        const Mem& m = op.as<Mem>();
        memOp = &m;

        uint32_t memSize = m.size();
        uint32_t baseType = m.baseType();
        uint32_t indexType = m.indexType();

        if (m.segmentId() > 6)
          return DebugUtils::errored(kErrorInvalidSegment);

        // Validate AVX-512 broadcast {1tox}.
        if (m.hasBroadcast()) {
          if (memSize != 0) {
            // If the size is specified it has to match the broadcast size.
            if (ASMJIT_UNLIKELY(commonInfo.hasAvx512B32() && memSize != 4))
              return DebugUtils::errored(kErrorInvalidBroadcast);

            if (ASMJIT_UNLIKELY(commonInfo.hasAvx512B64() && memSize != 8))
              return DebugUtils::errored(kErrorInvalidBroadcast);
          }
          else {
            // If there is no size we implicitly calculate it so we can validate N in {1toN} properly.
            memSize = commonInfo.hasAvx512B32() ? 4 : 8;
          }

          memSize <<= m.getBroadcast();
        }

        if (baseType) {
          uint32_t baseId = m.baseId();

          if (m.isRegHome()) {
            // Home address of a virtual register. In such case we don't want to
            // validate the type of the base register as it will always be patched
            // to ESP|RSP.
          }
          else {
            if (ASMJIT_UNLIKELY((vd->allowedMemBaseRegs & (1u << baseType)) == 0))
              return DebugUtils::errored(kErrorInvalidAddress);
          }

          // Create information that will be validated only if this is an implicit
          // memory operand. Basically only usable for string instructions and other
          // instructions where memory operand is implicit and has 'seg:[reg]' form.
          if (baseId < Operand::kVirtIdMin) {
            // Physical base id.
            regMask = Support::bitMask(baseId);
            combinedRegMask |= regMask;
          }
          else {
            // Virtual base id - fill the whole mask for implicit mem validation.
            // The register is not assigned yet, so we cannot predict the phys id.
            regMask = 0xFFFFFFFFu;
          }

          if (!indexType && !m.offsetLo32())
            memFlags |= InstDB::kMemOpBaseOnly;
        }
        else {
          // Base is a 64-bit address.
          int64_t offset = m.offset();
          if (!Support::isInt32(offset)) {
            if (mode == InstDB::kModeX86) {
              // 32-bit mode: Make sure that the address is either `int32_t` or `uint32_t`.
              if (!Support::isUInt32(offset))
                return DebugUtils::errored(kErrorInvalidAddress64Bit);
            }
            else {
              // 64-bit mode: Zero extension is allowed if the address has 32-bit index
              // register or the address has no index register (it's still encodable).
              if (indexType) {
                if (!Support::isUInt32(offset))
                  return DebugUtils::errored(kErrorInvalidAddress64Bit);

                if (indexType != Reg::kTypeGpd)
                  return DebugUtils::errored(kErrorInvalidAddress64BitZeroExtension);
              }
              else {
                // We don't validate absolute 64-bit addresses without an index register
                // as this also depends on the target's base address. We don't have the
                // information to do it at this moment.
              }
            }
          }
        }

        if (indexType) {
          if (ASMJIT_UNLIKELY((vd->allowedMemIndexRegs & (1u << indexType)) == 0))
            return DebugUtils::errored(kErrorInvalidAddress);

          if (indexType == Reg::kTypeXmm) {
            opFlags |= InstDB::kOpVm;
            memFlags |= InstDB::kMemOpVm32x | InstDB::kMemOpVm64x;
          }
          else if (indexType == Reg::kTypeYmm) {
            opFlags |= InstDB::kOpVm;
            memFlags |= InstDB::kMemOpVm32y | InstDB::kMemOpVm64y;
          }
          else if (indexType == Reg::kTypeZmm) {
            opFlags |= InstDB::kOpVm;
            memFlags |= InstDB::kMemOpVm32z | InstDB::kMemOpVm64z;
          }
          else {
            opFlags |= InstDB::kOpMem;
            if (baseType)
              memFlags |= InstDB::kMemOpMib;
          }

          // [RIP + {XMM|YMM|ZMM}] is not allowed.
          if (baseType == Reg::kTypeRip && (opFlags & InstDB::kOpVm))
            return DebugUtils::errored(kErrorInvalidAddress);

          uint32_t indexId = m.indexId();
          if (indexId < Operand::kVirtIdMin)
            combinedRegMask |= Support::bitMask(indexId);

          // Only used for implicit memory operands having 'seg:[reg]' form, so clear it.
          regMask = 0;
        }
        else {
          opFlags |= InstDB::kOpMem;
        }

        switch (memSize) {
          case  0: memFlags |= InstDB::kMemOpAny ; break;
          case  1: memFlags |= InstDB::kMemOpM8  ; break;
          case  2: memFlags |= InstDB::kMemOpM16 ; break;
          case  4: memFlags |= InstDB::kMemOpM32 ; break;
          case  6: memFlags |= InstDB::kMemOpM48 ; break;
          case  8: memFlags |= InstDB::kMemOpM64 ; break;
          case 10: memFlags |= InstDB::kMemOpM80 ; break;
          case 16: memFlags |= InstDB::kMemOpM128; break;
          case 32: memFlags |= InstDB::kMemOpM256; break;
          case 64: memFlags |= InstDB::kMemOpM512; break;
          default:
            return DebugUtils::errored(kErrorInvalidOperandSize);
        }

        break;
      }

      case Operand::kOpImm: {
        uint64_t immValue = op.as<Imm>().u64();
        uint32_t immFlags = 0;

        if (int64_t(immValue) >= 0) {
          if (immValue <= 0x7u)
            immFlags = InstDB::kOpI64 | InstDB::kOpU64 | InstDB::kOpI32 | InstDB::kOpU32 |
                       InstDB::kOpI16 | InstDB::kOpU16 | InstDB::kOpI8  | InstDB::kOpU8  |
                       InstDB::kOpI4  | InstDB::kOpU4  ;
          else if (immValue <= 0xFu)
            immFlags = InstDB::kOpI64 | InstDB::kOpU64 | InstDB::kOpI32 | InstDB::kOpU32 |
                       InstDB::kOpI16 | InstDB::kOpU16 | InstDB::kOpI8  | InstDB::kOpU8  |
                       InstDB::kOpU4  ;
          else if (immValue <= 0x7Fu)
            immFlags = InstDB::kOpI64 | InstDB::kOpU64 | InstDB::kOpI32 | InstDB::kOpU32 |
                       InstDB::kOpI16 | InstDB::kOpU16 | InstDB::kOpI8  | InstDB::kOpU8  ;
          else if (immValue <= 0xFFu)
            immFlags = InstDB::kOpI64 | InstDB::kOpU64 | InstDB::kOpI32 | InstDB::kOpU32 |
                       InstDB::kOpI16 | InstDB::kOpU16 | InstDB::kOpU8  ;
          else if (immValue <= 0x7FFFu)
            immFlags = InstDB::kOpI64 | InstDB::kOpU64 | InstDB::kOpI32 | InstDB::kOpU32 |
                       InstDB::kOpI16 | InstDB::kOpU16 ;
          else if (immValue <= 0xFFFFu)
            immFlags = InstDB::kOpI64 | InstDB::kOpU64 | InstDB::kOpI32 | InstDB::kOpU32 |
                       InstDB::kOpU16 ;
          else if (immValue <= 0x7FFFFFFFu)
            immFlags = InstDB::kOpI64 | InstDB::kOpU64 | InstDB::kOpI32 | InstDB::kOpU32;
          else if (immValue <= 0xFFFFFFFFu)
            immFlags = InstDB::kOpI64 | InstDB::kOpU64 | InstDB::kOpU32;
          else if (immValue <= 0x7FFFFFFFFFFFFFFFu)
            immFlags = InstDB::kOpI64 | InstDB::kOpU64;
          else
            immFlags = InstDB::kOpU64;
        }
        else {
          immValue = Support::neg(immValue);
          if (immValue <= 0x8u)
            immFlags = InstDB::kOpI64 | InstDB::kOpI32 | InstDB::kOpI16 | InstDB::kOpI8 | InstDB::kOpI4;
          else if (immValue <= 0x80u)
            immFlags = InstDB::kOpI64 | InstDB::kOpI32 | InstDB::kOpI16 | InstDB::kOpI8;
          else if (immValue <= 0x8000u)
            immFlags = InstDB::kOpI64 | InstDB::kOpI32 | InstDB::kOpI16;
          else if (immValue <= 0x80000000u)
            immFlags = InstDB::kOpI64 | InstDB::kOpI32;
          else
            immFlags = InstDB::kOpI64;
        }
        opFlags |= immFlags;
        break;
      }

      case Operand::kOpLabel: {
        opFlags |= InstDB::kOpRel8 | InstDB::kOpRel32;
        break;
      }

      default:
        return DebugUtils::errored(kErrorInvalidState);
    }

    InstDB::OpSignature& oSigDst = oSigTranslated[i];
    oSigDst.opFlags = opFlags;
    oSigDst.memFlags = uint16_t(memFlags);
    oSigDst.regMask = uint8_t(regMask & 0xFFu);
    combinedOpFlags |= opFlags;
  }

  // Decrease the number of operands of those that are none. This is important
  // as Assembler and Compiler may just pass more operands padded with none
  // (which means that no operand is given at that index). However, validate
  // that there are no gaps (like [reg, none, reg] or [none, reg]).
  if (i < opCount) {
    while (--opCount > i)
      if (ASMJIT_UNLIKELY(!operands[opCount].isNone()))
        return DebugUtils::errored(kErrorInvalidInstruction);
  }

  // Validate X86 and X64 specific cases.
  if (mode == InstDB::kModeX86) {
    // Illegal use of 64-bit register in 32-bit mode.
    if (ASMJIT_UNLIKELY((combinedOpFlags & InstDB::kOpGpq) != 0))
      return DebugUtils::errored(kErrorInvalidUseOfGpq);
  }
  else {
    // Illegal use of a high 8-bit register with REX prefix.
    if (ASMJIT_UNLIKELY((combinedOpFlags & InstDB::kOpGpbHi) != 0 && (combinedRegMask & 0xFFFFFF00u) != 0))
      return DebugUtils::errored(kErrorInvalidUseOfGpbHi);
  }

  // --------------------------------------------------------------------------
  // [Validate Instruction Signature by Comparing Against All `iSig` Rows]
  // --------------------------------------------------------------------------

  const InstDB::InstSignature* iSig = InstDB::_instSignatureTable + commonInfo._iSignatureIndex;
  const InstDB::InstSignature* iEnd = iSig + commonInfo._iSignatureCount;

  if (iSig != iEnd) {
    const InstDB::OpSignature* opSignatureTable = InstDB::_opSignatureTable;

    // If set it means that we matched a signature where only immediate value
    // was out of bounds. We can return a more descriptive error if we know this.
    bool globalImmOutOfRange = false;

    do {
      // Check if the architecture is compatible.
      if ((iSig->modes & mode) == 0)
        continue;

      // Compare the operands table with reference operands.
      uint32_t j = 0;
      uint32_t iSigCount = iSig->opCount;
      bool localImmOutOfRange = false;

      if (iSigCount == opCount) {
        for (j = 0; j < opCount; j++)
          if (!x86CheckOSig(oSigTranslated[j], opSignatureTable[iSig->operands[j]], localImmOutOfRange))
            break;
      }
      else if (iSigCount - iSig->implicit == opCount) {
        uint32_t r = 0;
        for (j = 0; j < opCount && r < iSigCount; j++, r++) {
          const InstDB::OpSignature* oChk = oSigTranslated + j;
          const InstDB::OpSignature* oRef;
Next:
          oRef = opSignatureTable + iSig->operands[r];
          // Skip implicit.
          if ((oRef->opFlags & InstDB::kOpImplicit) != 0) {
            if (++r >= iSigCount)
              break;
            else
              goto Next;
          }

          if (!x86CheckOSig(*oChk, *oRef, localImmOutOfRange))
            break;
        }
      }

      if (j == opCount) {
        if (!localImmOutOfRange) {
          // Match, must clear possible `globalImmOutOfRange`.
          globalImmOutOfRange = false;
          break;
        }
        globalImmOutOfRange = localImmOutOfRange;
      }
    } while (++iSig != iEnd);

    if (iSig == iEnd) {
      if (globalImmOutOfRange)
        return DebugUtils::errored(kErrorInvalidImmediate);
      else
        return DebugUtils::errored(kErrorInvalidInstruction);
    }
  }

  // --------------------------------------------------------------------------
  // [Validate AVX512 Options]
  // --------------------------------------------------------------------------

  const RegOnly& extraReg = inst.extraReg();
  const uint32_t kAvx512Options = Inst::kOptionZMask   |
                                  Inst::kOptionER      |
                                  Inst::kOptionSAE     ;

  if (options & kAvx512Options) {
    if (commonInfo.hasFlag(InstDB::kFlagEvex)) {
      // Validate AVX-512 {z}.
      if ((options & Inst::kOptionZMask)) {
        if (ASMJIT_UNLIKELY((options & Inst::kOptionZMask) != 0 && !commonInfo.hasAvx512Z()))
          return DebugUtils::errored(kErrorInvalidKZeroUse);
      }

      // Validate AVX-512 {sae} and {er}.
      if (options & (Inst::kOptionSAE | Inst::kOptionER)) {
        // Rounding control is impossible if the instruction is not reg-to-reg.
        if (ASMJIT_UNLIKELY(memOp))
          return DebugUtils::errored(kErrorInvalidEROrSAE);

        // Check if {sae} or {er} is supported by the instruction.
        if (options & Inst::kOptionER) {
          // NOTE: if both {sae} and {er} are set, we don't care, as {sae} is implied.
          if (ASMJIT_UNLIKELY(!commonInfo.hasAvx512ER()))
            return DebugUtils::errored(kErrorInvalidEROrSAE);
        }
        else {
          if (ASMJIT_UNLIKELY(!commonInfo.hasAvx512SAE()))
            return DebugUtils::errored(kErrorInvalidEROrSAE);
        }

        // {sae} and {er} are defined for either scalar ops or vector ops that
        // require LL to be 10 (512-bit vector operations). We don't need any
        // more bits in the instruction database to be able to validate this, as
        // each AVX512 instruction that has broadcast is vector instruction (in
        // this case we require zmm registers), otherwise it's a scalar instruction,
        // which is valid.
        if (commonInfo.hasAvx512B()) {
          // Supports broadcast, thus we require LL to be '10', which means there
          // have to be ZMM registers used. We don't calculate LL here, but we know
          // that it would be '10' if there is at least one ZMM register used.

          // There is no {er}/{sae}-enabled instruction with less than two operands.
          ASMJIT_ASSERT(opCount >= 2);
          if (ASMJIT_UNLIKELY(!x86IsZmmOrM512(operands[0]) && !x86IsZmmOrM512(operands[1])))
            return DebugUtils::errored(kErrorInvalidEROrSAE);
        }
      }
    }
    else {
      // Not AVX512 instruction - maybe OpExtra is xCX register used by REP/REPNE
      // prefix. Otherwise the instruction is invalid.
      if ((options & kAvx512Options) || (options & kRepAny) == 0)
        return DebugUtils::errored(kErrorInvalidInstruction);
    }
  }

  // --------------------------------------------------------------------------
  // [Validate {Extra} Register]
  // --------------------------------------------------------------------------

  if (extraReg.isReg()) {
    if (options & kRepAny) {
      // Validate REP|REPNE {cx|ecx|rcx}.
      if (ASMJIT_UNLIKELY(iFlags & InstDB::kFlagRepIgnored))
        return DebugUtils::errored(kErrorInvalidExtraReg);

      if (extraReg.isPhysReg()) {
        if (ASMJIT_UNLIKELY(extraReg.id() != Gp::kIdCx))
          return DebugUtils::errored(kErrorInvalidExtraReg);
      }

      // The type of the {...} register must match the type of the base register
      // of memory operand. So if the memory operand uses 32-bit register the
      // count register must also be 32-bit, etc...
      if (ASMJIT_UNLIKELY(!memOp || extraReg.type() != memOp->baseType()))
        return DebugUtils::errored(kErrorInvalidExtraReg);
    }
    else if (commonInfo.hasFlag(InstDB::kFlagEvex)) {
      // Validate AVX-512 {k}.
      if (ASMJIT_UNLIKELY(extraReg.type() != Reg::kTypeKReg))
        return DebugUtils::errored(kErrorInvalidExtraReg);

      if (ASMJIT_UNLIKELY(extraReg.id() == 0 || !commonInfo.hasAvx512K()))
        return DebugUtils::errored(kErrorInvalidKMaskUse);
    }
    else {
      return DebugUtils::errored(kErrorInvalidExtraReg);
    }
  }

  return kErrorOk;
}
#endif

// ============================================================================
// [asmjit::x86::InstInternal - QueryRWInfo]
// ============================================================================

struct RWInfo {
  enum Category : uint8_t {
    kCategoryGeneric,
    kCategoryImul,
    kCategoryMov,
    kCategoryMovh64,
    kCategoryVmaskmov,
    kCategoryVmovddup,
    kCategoryVmovmskpd,
    kCategoryVmovmskps,
    kCategoryVmov1_2,
    kCategoryVmov1_4,
    kCategoryVmov1_8,
    kCategoryVmov2_1,
    kCategoryVmov4_1,
    kCategoryVmov8_1
  };

  uint8_t category;
  uint8_t rmInfo;
  uint8_t opInfoIndex[6];
};

struct RWInfoOp {
  uint64_t rByteMask;
  uint64_t wByteMask;
  uint8_t physId;
  uint8_t reserved[3];
  uint32_t flags;
};

//! R/M information.
//!
//! This data is used to replace register operand by a memory operand reliably.
struct RWInfoRm {
  enum Category : uint8_t {
    kCategoryNone = 0,
    kCategoryFixed,
    kCategoryConsistent,
    kCategoryHalf,
    kCategoryQuarter,
    kCategoryEighth
  };

  enum Flags : uint8_t {
    kFlagAmbiguous = 0x01
  };

  uint8_t category;
  uint8_t rmOpsMask;
  uint8_t fixedSize;
  uint8_t flags;
};

// ${InstRWInfoTable:Begin}
// ------------------- Automatically generated, do not edit -------------------
static const uint8_t rwInfoIndex[Inst::_kIdCount * 2] = {
  0, 0, 0, 1, 1, 0, 1, 0, 0, 1, 2, 0, 3, 0, 2, 0, 4, 0, 4, 0, 5, 0, 6, 0, 4, 0,
  4, 0, 3, 0, 4, 0, 4, 0, 4, 0, 4, 0, 7, 0, 0, 7, 2, 0, 0, 8, 4, 0, 4, 0, 4, 0,
  4, 0, 9, 0, 0, 10, 11, 0, 11, 0, 11, 0, 11, 0, 11, 0, 0, 4, 0, 4, 0, 12, 0, 12,
  11, 0, 11, 0, 11, 0, 11, 0, 11, 0, 13, 0, 13, 0, 13, 0, 14, 0, 14, 0, 15, 0,
  16, 0, 17, 0, 11, 0, 11, 0, 0, 18, 19, 0, 20, 0, 20, 0, 20, 0, 0, 10, 0, 21,
  0, 1, 22, 0, 0, 23, 0, 0, 0, 0, 0, 0, 0, 24, 0, 24, 0, 24, 0, 0, 0, 0, 0, 0, 0,
  24, 0, 25, 0, 0, 3, 0, 3, 0, 3, 0, 3, 0, 3, 0, 3, 0, 3, 0, 3, 0, 3, 0, 3, 0,
  3, 0, 3, 0, 3, 0, 3, 0, 3, 0, 3, 0, 3, 0, 3, 0, 3, 0, 3, 0, 3, 0, 3, 0, 3, 0,
  3, 0, 3, 0, 3, 0, 3, 0, 3, 0, 3, 0, 3, 0, 26, 0, 0, 4, 0, 4, 27, 0, 0, 5, 0,
  6, 0, 28, 0, 29, 0, 30, 31, 0, 32, 0, 0, 33, 34, 0, 35, 0, 36, 0, 7, 0, 37, 0,
  37, 0, 37, 0, 36, 0, 38, 0, 7, 0, 36, 0, 39, 0, 40, 0, 41, 0, 42, 0, 43, 0, 44,
  0, 45, 0, 37, 0, 37, 0, 7, 0, 39, 0, 40, 0, 45, 0, 46, 0, 0, 47, 0, 1, 0, 1,
  0, 48, 49, 50, 4, 0, 4, 0, 5, 0, 6, 0, 0, 4, 0, 4, 0, 0, 0, 0, 0, 51, 52, 53,
  0, 0, 0, 0, 54, 55, 0, 56, 0, 57, 0, 58, 0, 0, 0, 0, 0, 56, 0, 56, 0, 56, 0,
  56, 0, 56, 0, 56, 0, 56, 0, 56, 0, 59, 0, 60, 0, 60, 0, 59, 0, 0, 0, 0, 0, 0,
  54, 55, 0, 56, 54, 55, 0, 56, 0, 0, 0, 56, 0, 55, 0, 55, 0, 55, 0, 55, 0, 55,
  0, 55, 0, 55, 0, 0, 0, 0, 0, 61, 0, 61, 0, 61, 0, 55, 0, 55, 0, 59, 0, 0, 0,
  62, 0, 24, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 54, 55, 0, 56, 0, 0, 0, 0, 0, 0,
  0, 63, 0, 64, 0, 63, 0, 65, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 24, 0, 63, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 66, 0, 64, 0, 63, 0, 66, 0, 65, 54, 55, 0, 56, 54, 55,
  0, 56, 0, 0, 0, 60, 0, 60, 0, 60, 0, 60, 0, 0, 0, 0, 0, 0, 0, 56, 0, 24, 0, 24,
  0, 63, 0, 63, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 4, 4, 0, 4, 0, 4, 0, 0, 0, 4,
  0, 4, 0, 49, 50, 67, 68, 69, 0, 0, 48, 70, 0, 0, 71, 52, 52, 0, 0, 0, 0, 0, 0,
  0, 0, 72, 0, 0, 24, 73, 0, 72, 0, 72, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 74, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 21, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 75, 0, 76, 0, 77, 0, 78, 0, 75, 0, 76, 0, 75, 0,
  76, 0, 77, 0, 78, 0, 77, 0, 78, 79, 0, 80, 0, 81, 0, 82, 0, 83, 0, 84, 0, 85,
  0, 86, 0, 0, 75, 0, 76, 0, 77, 87, 0, 88, 0, 89, 0, 90, 0, 0, 78, 0, 83, 0, 84,
  0, 85, 0, 86, 0, 83, 0, 84, 0, 85, 0, 86, 87, 0, 88, 0, 89, 0, 90, 0, 0, 91,
  0, 92, 0, 93, 0, 75, 0, 76, 0, 77, 0, 78, 0, 75, 0, 76, 0, 77, 0, 78, 0, 94,
  95, 0, 96, 0, 0, 97, 98, 0, 99, 0, 0, 0, 98, 0, 0, 0, 98, 0, 0, 24, 98, 0, 0,
  24, 0, 100, 0, 101, 0, 100, 102, 0, 103, 0, 103, 0, 103, 0, 95, 0, 98, 0, 0,
  100, 0, 104, 0, 104, 11, 0, 0, 105, 0, 106, 4, 0, 4, 0, 5, 0, 6, 0, 0, 0, 4, 0,
  4, 0, 5, 0, 6, 0, 0, 107, 0, 107, 108, 0, 109, 0, 109, 0, 110, 0, 80, 0, 36,
  0, 111, 0, 110, 0, 85, 0, 109, 0, 109, 0, 112, 0, 113, 0, 113, 0, 114, 0, 115,
  0, 115, 0, 116, 0, 116, 0, 96, 0, 96, 0, 110, 0, 96, 0, 96, 0, 115, 0, 115,
  0, 117, 0, 81, 0, 85, 0, 118, 0, 81, 0, 7, 0, 7, 0, 80, 0, 119, 0, 120, 0, 109,
  0, 109, 0, 119, 0, 0, 4, 49, 121, 4, 0, 4, 0, 5, 0, 6, 0, 0, 122, 123, 0, 0,
  124, 0, 48, 0, 125, 0, 48, 2, 0, 4, 0, 4, 0, 126, 0, 127, 0, 11, 0, 11, 0, 11,
  0, 3, 0, 3, 0, 4, 0, 3, 0, 3, 0, 3, 0, 3, 0, 3, 0, 3, 0, 3, 0, 3, 0, 3, 0, 0,
  3, 3, 0, 3, 0, 0, 0, 3, 0, 128, 0, 3, 0, 0, 12, 0, 4, 0, 4, 3, 0, 3, 0, 4, 0,
  3, 0, 0, 129, 0, 130, 3, 0, 3, 0, 4, 0, 3, 0, 0, 131, 0, 132, 0, 0, 0, 8, 0,
  8, 0, 133, 0, 51, 0, 134, 0, 135, 39, 0, 39, 0, 128, 0, 128, 0, 128, 0, 128,
  0, 128, 0, 128, 0, 128, 0, 128, 0, 128, 0, 128, 0, 39, 0, 128, 0, 128, 0, 128,
  0, 39, 0, 39, 0, 128, 0, 128, 0, 128, 0, 3, 0, 3, 0, 3, 0, 136, 0, 3, 0, 3, 0,
  3, 0, 39, 0, 39, 0, 0, 137, 0, 71, 0, 138, 0, 139, 3, 0, 3, 0, 4, 0, 4, 0, 3,
  0, 3, 0, 4, 0, 4, 0, 4, 0, 4, 0, 3, 0, 3, 0, 4, 0, 4, 0, 140, 0, 141, 0, 142,
  0, 36, 0, 36, 0, 36, 0, 141, 0, 141, 0, 142, 0, 36, 0, 36, 0, 36, 0, 141, 0,
  4, 0, 3, 0, 128, 0, 3, 0, 3, 0, 4, 0, 3, 0, 3, 0, 0, 143, 0, 0, 0, 0, 11, 0,
  0, 0, 0, 0, 0, 0, 3, 0, 0, 24, 0, 24, 0, 24, 0, 24, 0, 24, 0, 24, 0, 24, 3, 0,
  3, 0, 0, 7, 0, 7, 0, 7, 0, 39, 3, 0, 3, 0, 3, 0, 3, 0, 53, 0, 3, 0, 3, 0, 3,
  0, 3, 0, 3, 0, 53, 0, 3, 0, 3, 0, 3, 0, 3, 0, 3, 0, 3, 0, 3, 0, 3, 0, 3, 0, 3,
  0, 39, 0, 144, 0, 3, 0, 3, 0, 4, 0, 3, 0, 3, 0, 3, 0, 4, 0, 3, 0, 0, 145, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 146, 0, 7, 0, 147, 0, 146, 0, 0, 148, 0, 148,
  0, 149, 0, 148, 0, 149, 0, 148, 0, 148, 150, 0, 0, 151, 0, 0, 146, 0, 146, 0,
  0, 11, 0, 7, 0, 7, 0, 38, 0, 147, 0, 0, 7, 0, 147, 0, 0, 152, 146, 0, 146, 0,
  0, 10, 2, 0, 153, 0, 0, 154, 0, 154, 0, 154, 0, 154, 0, 154, 0, 154, 0, 154,
  0, 154, 0, 154, 0, 154, 0, 154, 0, 154, 0, 154, 0, 154, 0, 154, 0, 154, 0, 154,
  0, 154, 0, 154, 0, 154, 0, 154, 0, 154, 0, 154, 0, 154, 0, 154, 0, 154, 0, 154,
  0, 154, 0, 154, 0, 154, 0, 0, 0, 63, 4, 0, 4, 0, 4, 0, 0, 4, 4, 0, 4, 0, 0,
  12, 146, 0, 0, 155, 0, 10, 146, 0, 0, 155, 0, 10, 0, 4, 0, 4, 0, 63, 0, 47,
  0, 156, 0, 148, 0, 156, 7, 0, 7, 0, 38, 0, 147, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 157, 158, 0, 0, 156, 2, 0, 4, 0, 4, 0, 5, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 11, 0, 19, 0, 11, 0, 11, 0, 31, 0, 32, 0, 0, 0, 4, 0, 4,
  0, 4, 0, 4, 0, 0, 159, 0, 160, 0, 159, 0, 160, 0, 8, 0, 8, 0, 161, 0, 162, 0,
  8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 7, 0, 0, 7, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0,
  8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 163, 0, 163, 164, 0, 40,
  0, 165, 0, 166, 0, 165, 0, 166, 0, 164, 0, 40, 0, 165, 0, 166, 0, 165, 0, 166,
  0, 167, 0, 168, 0, 0, 8, 0, 8, 0, 169, 0, 170, 31, 0, 32, 0, 171, 0, 171, 0,
  172, 0, 11, 0, 0, 8, 119, 0, 173, 0, 173, 0, 11, 0, 173, 0, 11, 0, 172, 0, 11,
  0, 172, 0, 0, 174, 172, 0, 11, 0, 172, 0, 11, 0, 173, 0, 40, 0, 0, 175, 40,
  0, 0, 176, 0, 177, 0, 178, 45, 0, 45, 0, 173, 0, 11, 0, 173, 0, 11, 0, 11, 0,
  172, 0, 11, 0, 172, 0, 40, 0, 40, 0, 45, 0, 45, 0, 172, 0, 11, 0, 11, 0, 173,
  0, 0, 176, 0, 177, 0, 8, 0, 8, 0, 8, 0, 161, 0, 162, 0, 8, 0, 179, 0, 8, 0, 100,
  0, 100, 180, 0, 180, 0, 11, 0, 11, 0, 0, 181, 0, 182, 0, 183, 0, 182, 0, 183,
  0, 181, 0, 182, 0, 183, 0, 182, 0, 183, 0, 51, 0, 184, 0, 184, 0, 185, 0, 186,
  0, 184, 0, 184, 0, 187, 0, 188, 0, 184, 0, 184, 0, 187, 0, 188, 0, 184, 0,
  184, 0, 187, 0, 188, 0, 189, 0, 189, 0, 190, 0, 191, 0, 184, 0, 184, 0, 184,
  0, 184, 0, 184, 0, 184, 0, 189, 0, 189, 0, 184, 0, 184, 0, 187, 0, 188, 0, 184,
  0, 184, 0, 187, 0, 188, 0, 184, 0, 184, 0, 187, 0, 188, 0, 184, 0, 184, 0, 184,
  0, 184, 0, 184, 0, 184, 0, 189, 0, 189, 0, 189, 0, 189, 0, 190, 0, 191, 0,
  184, 0, 184, 0, 187, 0, 188, 0, 184, 0, 184, 0, 187, 0, 188, 0, 184, 0, 184,
  0, 187, 0, 188, 0, 189, 0, 189, 0, 190, 0, 191, 0, 184, 0, 184, 0, 187, 0, 188,
  0, 184, 0, 184, 0, 187, 0, 188, 0, 184, 0, 184, 0, 192, 0, 193, 0, 189, 0, 189,
  0, 190, 0, 191, 0, 194, 0, 194, 0, 39, 0, 120, 11, 0, 11, 0, 39, 0, 195, 0,
  98, 196, 98, 197, 0, 24, 0, 24, 0, 24, 0, 24, 0, 24, 0, 24, 0, 24, 0, 24, 98,
  197, 98, 198, 11, 0, 11, 0, 0, 199, 0, 200, 0, 11, 0, 11, 0, 199, 0, 200, 0,
  8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 201, 0, 202, 0, 203, 0, 202, 0, 203,
  0, 201, 0, 202, 0, 203, 0, 202, 0, 203, 0, 162, 110, 0, 0, 97, 0, 105, 0, 204,
  0, 204, 0, 8, 0, 8, 0, 161, 0, 162, 0, 0, 0, 205, 0, 0, 0, 8, 0, 8, 0, 161,
  0, 162, 0, 0, 0, 206, 0, 0, 207, 0, 207, 0, 80, 0, 208, 0, 207, 0, 207, 0, 207,
  0, 207, 0, 207, 0, 207, 0, 207, 0, 207, 0, 0, 209, 210, 211, 210, 211, 0, 212,
  115, 213, 115, 213, 214, 0, 215, 0, 110, 0, 110, 0, 110, 0, 110, 0, 216, 0,
  115, 217, 11, 0, 11, 0, 117, 218, 207, 0, 207, 0, 0, 8, 0, 219, 0, 205, 171,
  0, 0, 0, 0, 220, 0, 206, 0, 8, 0, 8, 0, 161, 0, 162, 221, 0, 0, 219, 0, 8, 0,
  8, 0, 222, 0, 222, 11, 0, 11, 0, 11, 0, 11, 0, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8,
  0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8,
  0, 8, 0, 8, 0, 8, 0, 8, 0, 163, 0, 8, 223, 0, 45, 0, 224, 0, 224, 0, 40, 0, 225,
  0, 0, 8, 0, 189, 0, 226, 0, 226, 0, 8, 0, 8, 0, 8, 0, 8, 0, 129, 0, 130, 0,
  8, 0, 8, 0, 8, 0, 8, 0, 131, 0, 132, 0, 226, 0, 226, 0, 226, 0, 226, 0, 226,
  0, 226, 0, 179, 0, 179, 171, 0, 171, 0, 171, 0, 171, 0, 0, 179, 0, 179, 0, 179,
  0, 179, 0, 179, 0, 179, 11, 0, 11, 0, 0, 184, 0, 184, 0, 184, 0, 184, 0, 227,
  0, 227, 0, 8, 0, 8, 0, 8, 0, 184, 0, 8, 0, 8, 0, 184, 0, 184, 0, 189, 0, 189,
  0, 228, 0, 228, 0, 229, 0, 227, 0, 228, 0, 8, 0, 184, 0, 184, 0, 184, 0, 184,
  0, 184, 0, 8, 11, 0, 11, 0, 11, 0, 11, 0, 0, 133, 0, 51, 0, 134, 0, 230, 98,
  197, 98, 196, 98, 198, 98, 197, 7, 0, 7, 0, 7, 0, 0, 8, 7, 0, 0, 8, 7, 0, 7,
  0, 7, 0, 7, 0, 7, 0, 7, 0, 0, 8, 7, 0, 7, 0, 136, 0, 7, 0, 0, 8, 7, 0, 0, 8, 0,
  8, 7, 0, 0, 231, 0, 162, 0, 161, 0, 232, 11, 0, 11, 0, 0, 233, 0, 233, 0, 233,
  0, 233, 0, 233, 0, 233, 0, 233, 0, 233, 0, 233, 0, 233, 0, 233, 0, 233, 0,
  184, 0, 184, 0, 8, 0, 8, 0, 204, 0, 204, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0,
  8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 234, 0, 234, 0, 235,
  0, 174, 0, 224, 0, 224, 0, 224, 0, 224, 0, 140, 0, 234, 0, 236, 0, 174, 0, 235,
  0, 235, 0, 174, 0, 236, 0, 174, 0, 235, 0, 174, 0, 237, 0, 238, 0, 172, 0,
  172, 0, 172, 0, 237, 0, 235, 0, 174, 0, 236, 0, 174, 0, 235, 0, 174, 0, 234,
  0, 174, 0, 237, 0, 238, 0, 172, 0, 172, 0, 172, 0, 237, 0, 0, 8, 0, 8, 0, 8, 0,
  8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 11, 0, 11, 0, 11, 0, 11, 0, 0, 8, 0, 8, 0,
  8, 0, 239, 0, 11, 0, 11, 0, 8, 0, 8, 0, 11, 0, 11, 0, 8, 0, 8, 0, 240, 0, 240,
  0, 240, 0, 240, 0, 8, 110, 0, 110, 0, 241, 0, 110, 0, 0, 240, 0, 240, 0, 240,
  0, 240, 0, 240, 0, 240, 0, 8, 0, 8, 0, 184, 0, 184, 0, 184, 0, 8, 0, 240, 0,
  240, 0, 8, 0, 8, 0, 184, 0, 184, 0, 184, 0, 8, 0, 8, 0, 226, 0, 11, 0, 11, 0,
  11, 0, 8, 0, 8, 0, 8, 0, 242, 0, 11, 0, 242, 0, 8, 0, 8, 0, 8, 0, 242, 0, 242,
  0, 242, 0, 8, 0, 8, 0, 8, 0, 242, 0, 242, 0, 11, 0, 242, 0, 8, 0, 8, 0, 8, 0,
  242, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 184, 0, 184, 221, 0,
  0, 226, 0, 226, 0, 226, 0, 226, 0, 226, 0, 226, 0, 226, 0, 226, 0, 8, 0, 8, 0,
  8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 199, 0, 200,
  11, 0, 11, 0, 0, 199, 0, 200, 180, 0, 180, 0, 0, 199, 0, 200, 11, 0, 0, 200,
  0, 11, 0, 11, 0, 199, 0, 200, 0, 11, 0, 11, 0, 199, 0, 200, 0, 11, 0, 11, 0,
  199, 0, 200, 11, 0, 11, 0, 0, 199, 0, 200, 180, 0, 180, 0, 0, 199, 0, 200, 11,
  0, 0, 200, 0, 8, 0, 8, 0, 161, 0, 162, 110, 0, 110, 0, 0, 24, 0, 24, 0, 24, 0,
  24, 0, 24, 0, 24, 0, 24, 0, 24, 110, 0, 241, 0, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8,
  0, 8, 11, 0, 11, 0, 0, 199, 0, 200, 0, 157, 0, 8, 0, 8, 0, 161, 0, 162, 221,
  0, 221, 0, 31, 0, 32, 0, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 101, 0, 101, 0, 243, 0, 0, 244, 0, 0, 0, 245, 0, 0, 0, 0, 149, 0, 0,
  2, 0, 4, 0, 4, 0, 0, 246, 0, 246, 0, 246, 0, 246, 0, 247, 0, 247, 0, 247, 0,
  247, 0, 247, 0, 247, 0, 247, 0, 247, 0, 243, 0, 0
};

static const RWInfo rwInfo[] = {
  { RWInfo::kCategoryGeneric   , 0 , { 0 , 0 , 0 , 0 , 0 , 0  } }, // #0 [ref=1607x]
  { RWInfo::kCategoryGeneric   , 0 , { 1 , 0 , 0 , 0 , 0 , 0  } }, // #1 [ref=7x]
  { RWInfo::kCategoryGeneric   , 1 , { 2 , 3 , 0 , 0 , 0 , 0  } }, // #2 [ref=7x]
  { RWInfo::kCategoryGeneric   , 2 , { 2 , 3 , 0 , 0 , 0 , 0  } }, // #3 [ref=100x]
  { RWInfo::kCategoryGeneric   , 3 , { 4 , 5 , 0 , 0 , 0 , 0  } }, // #4 [ref=69x]
  { RWInfo::kCategoryGeneric   , 4 , { 6 , 7 , 0 , 0 , 0 , 0  } }, // #5 [ref=7x]
  { RWInfo::kCategoryGeneric   , 5 , { 8 , 9 , 0 , 0 , 0 , 0  } }, // #6 [ref=7x]
  { RWInfo::kCategoryGeneric   , 3 , { 10, 5 , 0 , 0 , 0 , 0  } }, // #7 [ref=33x]
  { RWInfo::kCategoryGeneric   , 6 , { 11, 3 , 3 , 0 , 0 , 0  } }, // #8 [ref=185x]
  { RWInfo::kCategoryGeneric   , 7 , { 12, 13, 0 , 0 , 0 , 0  } }, // #9 [ref=1x]
  { RWInfo::kCategoryGeneric   , 2 , { 11, 3 , 3 , 0 , 0 , 0  } }, // #10 [ref=5x]
  { RWInfo::kCategoryGeneric   , 2 , { 11, 3 , 0 , 0 , 0 , 0  } }, // #11 [ref=82x]
  { RWInfo::kCategoryGeneric   , 3 , { 4 , 5 , 14, 0 , 0 , 0  } }, // #12 [ref=4x]
  { RWInfo::kCategoryGeneric   , 2 , { 5 , 3 , 0 , 0 , 0 , 0  } }, // #13 [ref=3x]
  { RWInfo::kCategoryGeneric   , 8 , { 10, 3 , 0 , 0 , 0 , 0  } }, // #14 [ref=2x]
  { RWInfo::kCategoryGeneric   , 9 , { 10, 5 , 0 , 0 , 0 , 0  } }, // #15 [ref=1x]
  { RWInfo::kCategoryGeneric   , 8 , { 11, 5 , 0 , 0 , 0 , 0  } }, // #16 [ref=1x]
  { RWInfo::kCategoryGeneric   , 0 , { 3 , 3 , 0 , 0 , 0 , 0  } }, // #17 [ref=1x]
  { RWInfo::kCategoryGeneric   , 0 , { 2 , 0 , 0 , 0 , 0 , 0  } }, // #18 [ref=1x]
  { RWInfo::kCategoryGeneric   , 10, { 3 , 3 , 0 , 0 , 0 , 0  } }, // #19 [ref=2x]
  { RWInfo::kCategoryGeneric   , 10, { 2 , 3 , 0 , 0 , 0 , 0  } }, // #20 [ref=3x]
  { RWInfo::kCategoryGeneric   , 11, { 3 , 0 , 0 , 0 , 0 , 0  } }, // #21 [ref=2x]
  { RWInfo::kCategoryGeneric   , 0 , { 15, 16, 0 , 0 , 0 , 0  } }, // #22 [ref=1x]
  { RWInfo::kCategoryGeneric   , 0 , { 17, 0 , 0 , 0 , 0 , 0  } }, // #23 [ref=1x]
  { RWInfo::kCategoryGeneric   , 8 , { 3 , 0 , 0 , 0 , 0 , 0  } }, // #24 [ref=34x]
  { RWInfo::kCategoryGeneric   , 0 , { 18, 0 , 0 , 0 , 0 , 0  } }, // #25 [ref=1x]
  { RWInfo::kCategoryGeneric   , 1 , { 3 , 3 , 0 , 0 , 0 , 0  } }, // #26 [ref=1x]
  { RWInfo::kCategoryGeneric   , 0 , { 19, 20, 0 , 0 , 0 , 0  } }, // #27 [ref=1x]
  { RWInfo::kCategoryGeneric   , 11, { 2 , 3 , 21, 0 , 0 , 0  } }, // #28 [ref=1x]
  { RWInfo::kCategoryGeneric   , 12, { 4 , 22, 17, 23, 24, 0  } }, // #29 [ref=1x]
  { RWInfo::kCategoryGeneric   , 13, { 25, 26, 27, 28, 29, 0  } }, // #30 [ref=1x]
  { RWInfo::kCategoryGeneric   , 4 , { 7 , 7 , 0 , 0 , 0 , 0  } }, // #31 [ref=4x]
  { RWInfo::kCategoryGeneric   , 5 , { 9 , 9 , 0 , 0 , 0 , 0  } }, // #32 [ref=4x]
  { RWInfo::kCategoryGeneric   , 0 , { 27, 30, 31, 15, 0 , 0  } }, // #33 [ref=1x]
  { RWInfo::kCategoryGeneric   , 0 , { 32, 33, 0 , 0 , 0 , 0  } }, // #34 [ref=1x]
  { RWInfo::kCategoryGeneric   , 14, { 2 , 3 , 0 , 0 , 0 , 0  } }, // #35 [ref=1x]
  { RWInfo::kCategoryGeneric   , 4 , { 10, 7 , 0 , 0 , 0 , 0  } }, // #36 [ref=10x]
  { RWInfo::kCategoryGeneric   , 3 , { 34, 5 , 0 , 0 , 0 , 0  } }, // #37 [ref=5x]
  { RWInfo::kCategoryGeneric   , 4 , { 35, 7 , 0 , 0 , 0 , 0  } }, // #38 [ref=3x]
  { RWInfo::kCategoryGeneric   , 4 , { 34, 7 , 0 , 0 , 0 , 0  } }, // #39 [ref=13x]
  { RWInfo::kCategoryGeneric   , 4 , { 11, 7 , 0 , 0 , 0 , 0  } }, // #40 [ref=9x]
  { RWInfo::kCategoryGeneric   , 4 , { 36, 7 , 0 , 0 , 0 , 0  } }, // #41 [ref=1x]
  { RWInfo::kCategoryGeneric   , 14, { 35, 3 , 0 , 0 , 0 , 0  } }, // #42 [ref=1x]
  { RWInfo::kCategoryGeneric   , 14, { 36, 3 , 0 , 0 , 0 , 0  } }, // #43 [ref=1x]
  { RWInfo::kCategoryGeneric   , 5 , { 35, 9 , 0 , 0 , 0 , 0  } }, // #44 [ref=1x]
  { RWInfo::kCategoryGeneric   , 5 , { 11, 9 , 0 , 0 , 0 , 0  } }, // #45 [ref=7x]
  { RWInfo::kCategoryGeneric   , 0 , { 37, 38, 0 , 0 , 0 , 0  } }, // #46 [ref=1x]
  { RWInfo::kCategoryGeneric   , 0 , { 27, 0 , 0 , 0 , 0 , 0  } }, // #47 [ref=2x]
  { RWInfo::kCategoryGeneric   , 10, { 2 , 0 , 0 , 0 , 0 , 0  } }, // #48 [ref=4x]
  { RWInfo::kCategoryGeneric   , 15, { 1 , 39, 0 , 0 , 0 , 0  } }, // #49 [ref=3x]
  { RWInfo::kCategoryGeneric   , 6 , { 40, 41, 3 , 0 , 0 , 0  } }, // #50 [ref=2x]
  { RWInfo::kCategoryGeneric   , 16, { 42, 5 , 0 , 0 , 0 , 0  } }, // #51 [ref=4x]
  { RWInfo::kCategoryGeneric   , 0 , { 4 , 5 , 0 , 0 , 0 , 0  } }, // #52 [ref=3x]
  { RWInfo::kCategoryGeneric   , 0 , { 4 , 0 , 0 , 0 , 0 , 0  } }, // #53 [ref=3x]
  { RWInfo::kCategoryGeneric   , 0 , { 43, 44, 0 , 0 , 0 , 0  } }, // #54 [ref=6x]
  { RWInfo::kCategoryGeneric   , 17, { 3 , 0 , 0 , 0 , 0 , 0  } }, // #55 [ref=15x]
  { RWInfo::kCategoryGeneric   , 0 , { 43, 0 , 0 , 0 , 0 , 0  } }, // #56 [ref=16x]
  { RWInfo::kCategoryGeneric   , 18, { 44, 0 , 0 , 0 , 0 , 0  } }, // #57 [ref=1x]
  { RWInfo::kCategoryGeneric   , 18, { 45, 0 , 0 , 0 , 0 , 0  } }, // #58 [ref=1x]
  { RWInfo::kCategoryGeneric   , 19, { 3 , 0 , 0 , 0 , 0 , 0  } }, // #59 [ref=3x]
  { RWInfo::kCategoryGeneric   , 0 , { 44, 0 , 0 , 0 , 0 , 0  } }, // #60 [ref=6x]
  { RWInfo::kCategoryGeneric   , 17, { 11, 0 , 0 , 0 , 0 , 0  } }, // #61 [ref=3x]
  { RWInfo::kCategoryGeneric   , 20, { 13, 0 , 0 , 0 , 0 , 0  } }, // #62 [ref=1x]
  { RWInfo::kCategoryGeneric   , 8 , { 11, 0 , 0 , 0 , 0 , 0  } }, // #63 [ref=8x]
  { RWInfo::kCategoryGeneric   , 20, { 46, 0 , 0 , 0 , 0 , 0  } }, // #64 [ref=2x]
  { RWInfo::kCategoryGeneric   , 7 , { 47, 0 , 0 , 0 , 0 , 0  } }, // #65 [ref=2x]
  { RWInfo::kCategoryGeneric   , 19, { 11, 0 , 0 , 0 , 0 , 0  } }, // #66 [ref=2x]
  { RWInfo::kCategoryImul      , 2 , { 0 , 0 , 0 , 0 , 0 , 0  } }, // #67 [ref=1x]
  { RWInfo::kCategoryImul      , 21, { 0 , 0 , 0 , 0 , 0 , 0  } }, // #68 [ref=1x]
  { RWInfo::kCategoryGeneric   , 0 , { 48, 49, 0 , 0 , 0 , 0  } }, // #69 [ref=1x]
  { RWInfo::kCategoryGeneric   , 0 , { 50, 49, 0 , 0 , 0 , 0  } }, // #70 [ref=1x]
  { RWInfo::kCategoryGeneric   , 5 , { 4 , 9 , 0 , 0 , 0 , 0  } }, // #71 [ref=2x]
  { RWInfo::kCategoryGeneric   , 12, { 3 , 5 , 0 , 0 , 0 , 0  } }, // #72 [ref=3x]
  { RWInfo::kCategoryGeneric   , 0 , { 21, 28, 0 , 0 , 0 , 0  } }, // #73 [ref=1x]
  { RWInfo::kCategoryGeneric   , 0 , { 51, 0 , 0 , 0 , 0 , 0  } }, // #74 [ref=1x]
  { RWInfo::kCategoryGeneric   , 0 , { 52, 39, 39, 0 , 0 , 0  } }, // #75 [ref=6x]
  { RWInfo::kCategoryGeneric   , 0 , { 42, 9 , 9 , 0 , 0 , 0  } }, // #76 [ref=6x]
  { RWInfo::kCategoryGeneric   , 0 , { 34, 7 , 7 , 0 , 0 , 0  } }, // #77 [ref=6x]
  { RWInfo::kCategoryGeneric   , 0 , { 46, 13, 13, 0 , 0 , 0  } }, // #78 [ref=6x]
  { RWInfo::kCategoryGeneric   , 22, { 52, 39, 0 , 0 , 0 , 0  } }, // #79 [ref=1x]
  { RWInfo::kCategoryGeneric   , 23, { 42, 9 , 0 , 0 , 0 , 0  } }, // #80 [ref=4x]
  { RWInfo::kCategoryGeneric   , 24, { 34, 7 , 0 , 0 , 0 , 0  } }, // #81 [ref=3x]
  { RWInfo::kCategoryGeneric   , 25, { 46, 13, 0 , 0 , 0 , 0  } }, // #82 [ref=1x]
  { RWInfo::kCategoryGeneric   , 0 , { 52, 39, 0 , 0 , 0 , 0  } }, // #83 [ref=3x]
  { RWInfo::kCategoryGeneric   , 0 , { 42, 9 , 0 , 0 , 0 , 0  } }, // #84 [ref=3x]
  { RWInfo::kCategoryGeneric   , 0 , { 34, 7 , 0 , 0 , 0 , 0  } }, // #85 [ref=5x]
  { RWInfo::kCategoryGeneric   , 0 , { 46, 13, 0 , 0 , 0 , 0  } }, // #86 [ref=3x]
  { RWInfo::kCategoryGeneric   , 0 , { 39, 39, 0 , 0 , 0 , 0  } }, // #87 [ref=2x]
  { RWInfo::kCategoryGeneric   , 0 , { 9 , 9 , 0 , 0 , 0 , 0  } }, // #88 [ref=2x]
  { RWInfo::kCategoryGeneric   , 0 , { 7 , 7 , 0 , 0 , 0 , 0  } }, // #89 [ref=2x]
  { RWInfo::kCategoryGeneric   , 0 , { 13, 13, 0 , 0 , 0 , 0  } }, // #90 [ref=2x]
  { RWInfo::kCategoryGeneric   , 0 , { 46, 39, 39, 0 , 0 , 0  } }, // #91 [ref=1x]
  { RWInfo::kCategoryGeneric   , 0 , { 34, 9 , 9 , 0 , 0 , 0  } }, // #92 [ref=1x]
  { RWInfo::kCategoryGeneric   , 0 , { 42, 13, 13, 0 , 0 , 0  } }, // #93 [ref=1x]
  { RWInfo::kCategoryGeneric   , 0 , { 53, 0 , 0 , 0 , 0 , 0  } }, // #94 [ref=1x]
  { RWInfo::kCategoryGeneric   , 26, { 11, 3 , 0 , 0 , 0 , 0  } }, // #95 [ref=2x]
  { RWInfo::kCategoryGeneric   , 12, { 10, 5 , 0 , 0 , 0 , 0  } }, // #96 [ref=5x]
  { RWInfo::kCategoryGeneric   , 27, { 9 , 0 , 0 , 0 , 0 , 0  } }, // #97 [ref=2x]
  { RWInfo::kCategoryGeneric   , 0 , { 2 , 3 , 0 , 0 , 0 , 0  } }, // #98 [ref=13x]
  { RWInfo::kCategoryGeneric   , 8 , { 11, 3 , 0 , 0 , 0 , 0  } }, // #99 [ref=1x]
  { RWInfo::kCategoryGeneric   , 7 , { 13, 0 , 0 , 0 , 0 , 0  } }, // #100 [ref=5x]
  { RWInfo::kCategoryGeneric   , 0 , { 3 , 0 , 0 , 0 , 0 , 0  } }, // #101 [ref=3x]
  { RWInfo::kCategoryGeneric   , 0 , { 48, 19, 0 , 0 , 0 , 0  } }, // #102 [ref=1x]
  { RWInfo::kCategoryGeneric   , 0 , { 54, 0 , 0 , 0 , 0 , 0  } }, // #103 [ref=3x]
  { RWInfo::kCategoryGeneric   , 5 , { 3 , 9 , 0 , 0 , 0 , 0  } }, // #104 [ref=2x]
  { RWInfo::kCategoryGeneric   , 0 , { 5 , 5 , 20, 0 , 0 , 0  } }, // #105 [ref=2x]
  { RWInfo::kCategoryGeneric   , 0 , { 7 , 7 , 20, 0 , 0 , 0  } }, // #106 [ref=1x]
  { RWInfo::kCategoryGeneric   , 0 , { 18, 28, 55, 0 , 0 , 0  } }, // #107 [ref=2x]
  { RWInfo::kCategoryMov       , 28, { 0 , 0 , 0 , 0 , 0 , 0  } }, // #108 [ref=1x]
  { RWInfo::kCategoryGeneric   , 29, { 10, 5 , 0 , 0 , 0 , 0  } }, // #109 [ref=6x]
  { RWInfo::kCategoryGeneric   , 0 , { 11, 3 , 0 , 0 , 0 , 0  } }, // #110 [ref=14x]
  { RWInfo::kCategoryGeneric   , 30, { 11, 56, 0 , 0 , 0 , 0  } }, // #111 [ref=1x]
  { RWInfo::kCategoryGeneric   , 0 , { 35, 57, 0 , 0 , 0 , 0  } }, // #112 [ref=1x]
  { RWInfo::kCategoryMovh64    , 13, { 0 , 0 , 0 , 0 , 0 , 0  } }, // #113 [ref=2x]
  { RWInfo::kCategoryGeneric   , 0 , { 58, 7 , 0 , 0 , 0 , 0  } }, // #114 [ref=1x]
  { RWInfo::kCategoryGeneric   , 13, { 34, 7 , 0 , 0 , 0 , 0  } }, // #115 [ref=7x]
  { RWInfo::kCategoryGeneric   , 0 , { 52, 5 , 0 , 0 , 0 , 0  } }, // #116 [ref=2x]
  { RWInfo::kCategoryGeneric   , 27, { 42, 9 , 0 , 0 , 0 , 0  } }, // #117 [ref=2x]
  { RWInfo::kCategoryGeneric   , 0 , { 20, 19, 0 , 0 , 0 , 0  } }, // #118 [ref=1x]
  { RWInfo::kCategoryGeneric   , 14, { 11, 3 , 0 , 0 , 0 , 0  } }, // #119 [ref=3x]
  { RWInfo::kCategoryGeneric   , 5 , { 34, 9 , 0 , 0 , 0 , 0  } }, // #120 [ref=2x]
  { RWInfo::kCategoryGeneric   , 6 , { 59, 41, 3 , 0 , 0 , 0  } }, // #121 [ref=1x]
  { RWInfo::kCategoryGeneric   , 6 , { 11, 11, 3 , 60, 0 , 0  } }, // #122 [ref=1x]
  { RWInfo::kCategoryGeneric   , 0 , { 16, 28, 0 , 0 , 0 , 0  } }, // #123 [ref=1x]
  { RWInfo::kCategoryGeneric   , 0 , { 16, 28, 29, 0 , 0 , 0  } }, // #124 [ref=1x]
  { RWInfo::kCategoryGeneric   , 10, { 3 , 0 , 0 , 0 , 0 , 0  } }, // #125 [ref=1x]
  { RWInfo::kCategoryGeneric   , 0 , { 49, 21, 0 , 0 , 0 , 0  } }, // #126 [ref=1x]
  { RWInfo::kCategoryGeneric   , 0 , { 49, 61, 0 , 0 , 0 , 0  } }, // #127 [ref=1x]
  { RWInfo::kCategoryGeneric   , 4 , { 25, 7 , 0 , 0 , 0 , 0  } }, // #128 [ref=18x]
  { RWInfo::kCategoryGeneric   , 3 , { 5 , 5 , 0 , 62, 16, 55 } }, // #129 [ref=2x]
  { RWInfo::kCategoryGeneric   , 3 , { 5 , 5 , 0 , 63, 16, 55 } }, // #130 [ref=2x]
  { RWInfo::kCategoryGeneric   , 3 , { 5 , 5 , 0 , 62, 0 , 0  } }, // #131 [ref=2x]
  { RWInfo::kCategoryGeneric   , 3 , { 5 , 5 , 0 , 63, 0 , 0  } }, // #132 [ref=2x]
  { RWInfo::kCategoryGeneric   , 31, { 52, 5 , 0 , 0 , 0 , 0  } }, // #133 [ref=2x]
  { RWInfo::kCategoryGeneric   , 32, { 34, 5 , 0 , 0 , 0 , 0  } }, // #134 [ref=2x]
  { RWInfo::kCategoryGeneric   , 7 , { 46, 3 , 0 , 0 , 0 , 0  } }, // #135 [ref=1x]
  { RWInfo::kCategoryGeneric   , 3 , { 64, 5 , 0 , 0 , 0 , 0  } }, // #136 [ref=2x]
  { RWInfo::kCategoryGeneric   , 15, { 4 , 39, 0 , 0 , 0 , 0  } }, // #137 [ref=1x]
  { RWInfo::kCategoryGeneric   , 4 , { 4 , 7 , 0 , 0 , 0 , 0  } }, // #138 [ref=1x]
  { RWInfo::kCategoryGeneric   , 26, { 2 , 13, 0 , 0 , 0 , 0  } }, // #139 [ref=1x]
  { RWInfo::kCategoryVmov1_8   , 0 , { 0 , 0 , 0 , 0 , 0 , 0  } }, // #140 [ref=2x]
  { RWInfo::kCategoryGeneric   , 5 , { 10, 9 , 0 , 0 , 0 , 0  } }, // #141 [ref=4x]
  { RWInfo::kCategoryGeneric   , 26, { 10, 13, 0 , 0 , 0 , 0  } }, // #142 [ref=2x]
  { RWInfo::kCategoryGeneric   , 10, { 65, 0 , 0 , 0 , 0 , 0  } }, // #143 [ref=1x]
  { RWInfo::kCategoryGeneric   , 3 , { 5 , 5 , 0 , 0 , 0 , 0  } }, // #144 [ref=1x]
  { RWInfo::kCategoryGeneric   , 10, { 60, 0 , 0 , 0 , 0 , 0  } }, // #145 [ref=1x]
  { RWInfo::kCategoryGeneric   , 10, { 2 , 66, 0 , 0 , 0 , 0  } }, // #146 [ref=8x]
  { RWInfo::kCategoryGeneric   , 5 , { 36, 9 , 0 , 0 , 0 , 0  } }, // #147 [ref=4x]
  { RWInfo::kCategoryGeneric   , 0 , { 11, 0 , 0 , 0 , 0 , 0  } }, // #148 [ref=6x]
  { RWInfo::kCategoryGeneric   , 0 , { 15, 67, 28, 0 , 0 , 0  } }, // #149 [ref=3x]
  { RWInfo::kCategoryGeneric   , 0 , { 15, 67, 0 , 0 , 0 , 0  } }, // #150 [ref=1x]
  { RWInfo::kCategoryGeneric   , 0 , { 15, 67, 62, 0 , 0 , 0  } }, // #151 [ref=1x]
  { RWInfo::kCategoryGeneric   , 0 , { 68, 0 , 0 , 0 , 0 , 0  } }, // #152 [ref=1x]
  { RWInfo::kCategoryGeneric   , 0 , { 21, 20, 0 , 0 , 0 , 0  } }, // #153 [ref=1x]
  { RWInfo::kCategoryGeneric   , 31, { 69, 0 , 0 , 0 , 0 , 0  } }, // #154 [ref=30x]
  { RWInfo::kCategoryGeneric   , 11, { 2 , 3 , 66, 0 , 0 , 0  } }, // #155 [ref=2x]
  { RWInfo::kCategoryGeneric   , 33, { 11, 0 , 0 , 0 , 0 , 0  } }, // #156 [ref=3x]
  { RWInfo::kCategoryGeneric   , 27, { 42, 0 , 0 , 0 , 0 , 0  } }, // #157 [ref=2x]
  { RWInfo::kCategoryGeneric   , 0 , { 20, 21, 0 , 0 , 0 , 0  } }, // #158 [ref=1x]
  { RWInfo::kCategoryGeneric   , 12, { 70, 56, 56, 56, 56, 5  } }, // #159 [ref=2x]
  { RWInfo::kCategoryGeneric   , 12, { 4 , 5 , 5 , 5 , 5 , 5  } }, // #160 [ref=2x]
  { RWInfo::kCategoryGeneric   , 34, { 10, 5 , 7 , 0 , 0 , 0  } }, // #161 [ref=8x]
  { RWInfo::kCategoryGeneric   , 35, { 10, 5 , 9 , 0 , 0 , 0  } }, // #162 [ref=9x]
  { RWInfo::kCategoryGeneric   , 6 , { 11, 3 , 3 , 3 , 0 , 0  } }, // #163 [ref=3x]
  { RWInfo::kCategoryGeneric   , 12, { 71, 5 , 0 , 0 , 0 , 0  } }, // #164 [ref=2x]
  { RWInfo::kCategoryGeneric   , 12, { 11, 5 , 0 , 0 , 0 , 0  } }, // #165 [ref=4x]
  { RWInfo::kCategoryGeneric   , 36, { 72, 73, 0 , 0 , 0 , 0  } }, // #166 [ref=4x]
  { RWInfo::kCategoryGeneric   , 37, { 11, 7 , 0 , 0 , 0 , 0  } }, // #167 [ref=1x]
  { RWInfo::kCategoryGeneric   , 38, { 11, 9 , 0 , 0 , 0 , 0  } }, // #168 [ref=1x]
  { RWInfo::kCategoryGeneric   , 34, { 11, 5 , 7 , 0 , 0 , 0  } }, // #169 [ref=1x]
  { RWInfo::kCategoryGeneric   , 35, { 11, 5 , 9 , 0 , 0 , 0  } }, // #170 [ref=1x]
  { RWInfo::kCategoryGeneric   , 11, { 11, 3 , 0 , 0 , 0 , 0  } }, // #171 [ref=7x]
  { RWInfo::kCategoryVmov2_1   , 39, { 0 , 0 , 0 , 0 , 0 , 0  } }, // #172 [ref=14x]
  { RWInfo::kCategoryVmov1_2   , 14, { 0 , 0 , 0 , 0 , 0 , 0  } }, // #173 [ref=7x]
  { RWInfo::kCategoryVmov1_2   , 40, { 0 , 0 , 0 , 0 , 0 , 0  } }, // #174 [ref=10x]
  { RWInfo::kCategoryGeneric   , 34, { 10, 74, 7 , 0 , 0 , 0  } }, // #175 [ref=1x]
  { RWInfo::kCategoryGeneric   , 41, { 10, 57, 3 , 0 , 0 , 0  } }, // #176 [ref=2x]
  { RWInfo::kCategoryGeneric   , 41, { 10, 74, 3 , 0 , 0 , 0  } }, // #177 [ref=2x]
  { RWInfo::kCategoryGeneric   , 35, { 10, 57, 9 , 0 , 0 , 0  } }, // #178 [ref=1x]
  { RWInfo::kCategoryGeneric   , 42, { 10, 5 , 5 , 0 , 0 , 0  } }, // #179 [ref=9x]
  { RWInfo::kCategoryGeneric   , 43, { 72, 56, 0 , 0 , 0 , 0  } }, // #180 [ref=6x]
  { RWInfo::kCategoryGeneric   , 44, { 10, 73, 0 , 0 , 0 , 0  } }, // #181 [ref=2x]
  { RWInfo::kCategoryGeneric   , 44, { 10, 3 , 0 , 0 , 0 , 0  } }, // #182 [ref=4x]
  { RWInfo::kCategoryGeneric   , 45, { 71, 56, 0 , 0 , 0 , 0  } }, // #183 [ref=4x]
  { RWInfo::kCategoryGeneric   , 6 , { 2 , 3 , 3 , 0 , 0 , 0  } }, // #184 [ref=60x]
  { RWInfo::kCategoryGeneric   , 34, { 4 , 57, 7 , 0 , 0 , 0  } }, // #185 [ref=1x]
  { RWInfo::kCategoryGeneric   , 35, { 4 , 74, 9 , 0 , 0 , 0  } }, // #186 [ref=1x]
  { RWInfo::kCategoryGeneric   , 34, { 6 , 7 , 7 , 0 , 0 , 0  } }, // #187 [ref=11x]
  { RWInfo::kCategoryGeneric   , 35, { 8 , 9 , 9 , 0 , 0 , 0  } }, // #188 [ref=11x]
  { RWInfo::kCategoryGeneric   , 46, { 11, 3 , 3 , 3 , 0 , 0  } }, // #189 [ref=15x]
  { RWInfo::kCategoryGeneric   , 47, { 34, 7 , 7 , 7 , 0 , 0  } }, // #190 [ref=4x]
  { RWInfo::kCategoryGeneric   , 48, { 42, 9 , 9 , 9 , 0 , 0  } }, // #191 [ref=4x]
  { RWInfo::kCategoryGeneric   , 34, { 25, 7 , 7 , 0 , 0 , 0  } }, // #192 [ref=1x]
  { RWInfo::kCategoryGeneric   , 35, { 75, 9 , 9 , 0 , 0 , 0  } }, // #193 [ref=1x]
  { RWInfo::kCategoryGeneric   , 14, { 34, 3 , 0 , 0 , 0 , 0  } }, // #194 [ref=2x]
  { RWInfo::kCategoryGeneric   , 5 , { 42, 9 , 0 , 0 , 0 , 0  } }, // #195 [ref=1x]
  { RWInfo::kCategoryGeneric   , 8 , { 2 , 3 , 2 , 0 , 0 , 0  } }, // #196 [ref=2x]
  { RWInfo::kCategoryGeneric   , 0 , { 2 , 3 , 2 , 0 , 0 , 0  } }, // #197 [ref=4x]
  { RWInfo::kCategoryGeneric   , 17, { 4 , 3 , 4 , 0 , 0 , 0  } }, // #198 [ref=2x]
  { RWInfo::kCategoryGeneric   , 34, { 10, 57, 7 , 0 , 0 , 0  } }, // #199 [ref=11x]
  { RWInfo::kCategoryGeneric   , 35, { 10, 74, 9 , 0 , 0 , 0  } }, // #200 [ref=13x]
  { RWInfo::kCategoryGeneric   , 42, { 71, 73, 5 , 0 , 0 , 0  } }, // #201 [ref=2x]
  { RWInfo::kCategoryGeneric   , 42, { 11, 3 , 5 , 0 , 0 , 0  } }, // #202 [ref=4x]
  { RWInfo::kCategoryGeneric   , 49, { 72, 56, 73, 0 , 0 , 0  } }, // #203 [ref=4x]
  { RWInfo::kCategoryVmaskmov  , 0 , { 0 , 0 , 0 , 0 , 0 , 0  } }, // #204 [ref=4x]
  { RWInfo::kCategoryGeneric   , 13, { 34, 0 , 0 , 0 , 0 , 0  } }, // #205 [ref=2x]
  { RWInfo::kCategoryGeneric   , 0 , { 21, 0 , 0 , 0 , 0 , 0  } }, // #206 [ref=2x]
  { RWInfo::kCategoryGeneric   , 50, { 11, 3 , 0 , 0 , 0 , 0  } }, // #207 [ref=12x]
  { RWInfo::kCategoryVmovddup  , 51, { 0 , 0 , 0 , 0 , 0 , 0  } }, // #208 [ref=1x]
  { RWInfo::kCategoryGeneric   , 0 , { 10, 57, 57, 0 , 0 , 0  } }, // #209 [ref=1x]
  { RWInfo::kCategoryGeneric   , 13, { 34, 57, 0 , 0 , 0 , 0  } }, // #210 [ref=2x]
  { RWInfo::kCategoryGeneric   , 13, { 10, 7 , 7 , 0 , 0 , 0  } }, // #211 [ref=2x]
  { RWInfo::kCategoryGeneric   , 0 , { 10, 7 , 7 , 0 , 0 , 0  } }, // #212 [ref=1x]
  { RWInfo::kCategoryGeneric   , 13, { 10, 57, 7 , 0 , 0 , 0  } }, // #213 [ref=2x]
  { RWInfo::kCategoryVmovmskpd , 0 , { 0 , 0 , 0 , 0 , 0 , 0  } }, // #214 [ref=1x]
  { RWInfo::kCategoryVmovmskps , 0 , { 0 , 0 , 0 , 0 , 0 , 0  } }, // #215 [ref=1x]
  { RWInfo::kCategoryGeneric   , 52, { 34, 7 , 0 , 0 , 0 , 0  } }, // #216 [ref=1x]
  { RWInfo::kCategoryGeneric   , 0 , { 10, 57, 7 , 0 , 0 , 0  } }, // #217 [ref=1x]
  { RWInfo::kCategoryGeneric   , 0 , { 10, 74, 9 , 0 , 0 , 0  } }, // #218 [ref=1x]
  { RWInfo::kCategoryGeneric   , 13, { 7 , 0 , 0 , 0 , 0 , 0  } }, // #219 [ref=2x]
  { RWInfo::kCategoryGeneric   , 0 , { 76, 0 , 0 , 0 , 0 , 0  } }, // #220 [ref=1x]
  { RWInfo::kCategoryGeneric   , 2 , { 3 , 3 , 0 , 0 , 0 , 0  } }, // #221 [ref=4x]
  { RWInfo::kCategoryGeneric   , 12, { 72, 56, 56, 56, 56, 5  } }, // #222 [ref=2x]
  { RWInfo::kCategoryGeneric   , 15, { 11, 39, 0 , 0 , 0 , 0  } }, // #223 [ref=1x]
  { RWInfo::kCategoryGeneric   , 0 , { 11, 7 , 0 , 0 , 0 , 0  } }, // #224 [ref=6x]
  { RWInfo::kCategoryGeneric   , 26, { 11, 13, 0 , 0 , 0 , 0  } }, // #225 [ref=1x]
  { RWInfo::kCategoryGeneric   , 6 , { 34, 3 , 3 , 0 , 0 , 0  } }, // #226 [ref=17x]
  { RWInfo::kCategoryGeneric   , 49, { 71, 73, 73, 0 , 0 , 0  } }, // #227 [ref=3x]
  { RWInfo::kCategoryGeneric   , 21, { 11, 3 , 3 , 0 , 0 , 0  } }, // #228 [ref=3x]
  { RWInfo::kCategoryGeneric   , 53, { 71, 73, 0 , 0 , 0 , 0  } }, // #229 [ref=1x]
  { RWInfo::kCategoryGeneric   , 7 , { 46, 5 , 0 , 0 , 0 , 0  } }, // #230 [ref=1x]
  { RWInfo::kCategoryGeneric   , 54, { 10, 5 , 39, 0 , 0 , 0  } }, // #231 [ref=1x]
  { RWInfo::kCategoryGeneric   , 55, { 10, 5 , 13, 0 , 0 , 0  } }, // #232 [ref=1x]
  { RWInfo::kCategoryGeneric   , 42, { 10, 5 , 5 , 5 , 0 , 0  } }, // #233 [ref=12x]
  { RWInfo::kCategoryGeneric   , 0 , { 34, 3 , 0 , 0 , 0 , 0  } }, // #234 [ref=4x]
  { RWInfo::kCategoryVmov1_4   , 56, { 0 , 0 , 0 , 0 , 0 , 0  } }, // #235 [ref=6x]
  { RWInfo::kCategoryVmov1_8   , 57, { 0 , 0 , 0 , 0 , 0 , 0  } }, // #236 [ref=3x]
  { RWInfo::kCategoryVmov4_1   , 58, { 0 , 0 , 0 , 0 , 0 , 0  } }, // #237 [ref=4x]
  { RWInfo::kCategoryVmov8_1   , 59, { 0 , 0 , 0 , 0 , 0 , 0  } }, // #238 [ref=2x]
  { RWInfo::kCategoryGeneric   , 60, { 10, 5 , 5 , 5 , 0 , 0  } }, // #239 [ref=1x]
  { RWInfo::kCategoryGeneric   , 61, { 10, 5 , 5 , 0 , 0 , 0  } }, // #240 [ref=12x]
  { RWInfo::kCategoryGeneric   , 17, { 11, 3 , 0 , 0 , 0 , 0  } }, // #241 [ref=2x]
  { RWInfo::kCategoryGeneric   , 21, { 11, 3 , 5 , 0 , 0 , 0  } }, // #242 [ref=9x]
  { RWInfo::kCategoryGeneric   , 0 , { 55, 16, 28, 0 , 0 , 0  } }, // #243 [ref=2x]
  { RWInfo::kCategoryGeneric   , 11, { 2 , 2 , 0 , 0 , 0 , 0  } }, // #244 [ref=1x]
  { RWInfo::kCategoryGeneric   , 50, { 2 , 2 , 0 , 0 , 0 , 0  } }, // #245 [ref=1x]
  { RWInfo::kCategoryGeneric   , 8 , { 3 , 55, 16, 0 , 0 , 0  } }, // #246 [ref=4x]
  { RWInfo::kCategoryGeneric   , 8 , { 11, 55, 16, 0 , 0 , 0  } }  // #247 [ref=8x]
};

static const RWInfoOp rwInfoOp[] = {
  { 0x0000000000000000u, 0x0000000000000000u, 0xFF, { 0 }, 0 }, // #0 [ref=14938x]
  { 0x0000000000000003u, 0x0000000000000003u, 0x00, { 0 }, OpRWInfo::kRW | OpRWInfo::kRegPhysId }, // #1 [ref=10x]
  { 0x0000000000000000u, 0x0000000000000000u, 0xFF, { 0 }, OpRWInfo::kRW | OpRWInfo::kZExt }, // #2 [ref=217x]
  { 0x0000000000000000u, 0x0000000000000000u, 0xFF, { 0 }, OpRWInfo::kRead }, // #3 [ref=974x]
  { 0x000000000000FFFFu, 0x000000000000FFFFu, 0xFF, { 0 }, OpRWInfo::kRW | OpRWInfo::kZExt }, // #4 [ref=92x]
  { 0x000000000000FFFFu, 0x0000000000000000u, 0xFF, { 0 }, OpRWInfo::kRead }, // #5 [ref=305x]
  { 0x00000000000000FFu, 0x00000000000000FFu, 0xFF, { 0 }, OpRWInfo::kRW }, // #6 [ref=18x]
  { 0x00000000000000FFu, 0x0000000000000000u, 0xFF, { 0 }, OpRWInfo::kRead }, // #7 [ref=181x]
  { 0x000000000000000Fu, 0x000000000000000Fu, 0xFF, { 0 }, OpRWInfo::kRW }, // #8 [ref=18x]
  { 0x000000000000000Fu, 0x0000000000000000u, 0xFF, { 0 }, OpRWInfo::kRead }, // #9 [ref=130x]
  { 0x0000000000000000u, 0x000000000000FFFFu, 0xFF, { 0 }, OpRWInfo::kWrite | OpRWInfo::kZExt }, // #10 [ref=160x]
  { 0x0000000000000000u, 0x0000000000000000u, 0xFF, { 0 }, OpRWInfo::kWrite | OpRWInfo::kZExt }, // #11 [ref=413x]
  { 0x0000000000000003u, 0x0000000000000003u, 0xFF, { 0 }, OpRWInfo::kRW }, // #12 [ref=1x]
  { 0x0000000000000003u, 0x0000000000000000u, 0xFF, { 0 }, OpRWInfo::kRead }, // #13 [ref=34x]
  { 0x000000000000FFFFu, 0x0000000000000000u, 0x00, { 0 }, OpRWInfo::kRead | OpRWInfo::kRegPhysId }, // #14 [ref=4x]
  { 0x0000000000000000u, 0x000000000000000Fu, 0x02, { 0 }, OpRWInfo::kWrite | OpRWInfo::kZExt | OpRWInfo::kRegPhysId }, // #15 [ref=7x]
  { 0x000000000000000Fu, 0x0000000000000000u, 0x00, { 0 }, OpRWInfo::kRead | OpRWInfo::kRegPhysId }, // #16 [ref=21x]
  { 0x00000000000000FFu, 0x00000000000000FFu, 0x00, { 0 }, OpRWInfo::kRW | OpRWInfo::kZExt | OpRWInfo::kRegPhysId }, // #17 [ref=2x]
  { 0x0000000000000000u, 0x0000000000000000u, 0x00, { 0 }, OpRWInfo::kRead | OpRWInfo::kMemPhysId }, // #18 [ref=3x]
  { 0x0000000000000000u, 0x0000000000000000u, 0x06, { 0 }, OpRWInfo::kRW | OpRWInfo::kZExt | OpRWInfo::kMemPhysId }, // #19 [ref=3x]
  { 0x0000000000000000u, 0x0000000000000000u, 0x07, { 0 }, OpRWInfo::kRW | OpRWInfo::kZExt | OpRWInfo::kMemPhysId }, // #20 [ref=7x]
  { 0x0000000000000000u, 0x0000000000000000u, 0x00, { 0 }, OpRWInfo::kRead | OpRWInfo::kRegPhysId }, // #21 [ref=7x]
  { 0x00000000000000FFu, 0x00000000000000FFu, 0x02, { 0 }, OpRWInfo::kRW | OpRWInfo::kZExt | OpRWInfo::kRegPhysId }, // #22 [ref=1x]
  { 0x00000000000000FFu, 0x0000000000000000u, 0x01, { 0 }, OpRWInfo::kRead | OpRWInfo::kRegPhysId }, // #23 [ref=1x]
  { 0x00000000000000FFu, 0x0000000000000000u, 0x03, { 0 }, OpRWInfo::kRead | OpRWInfo::kRegPhysId }, // #24 [ref=1x]
  { 0x00000000000000FFu, 0x00000000000000FFu, 0xFF, { 0 }, OpRWInfo::kRW | OpRWInfo::kZExt }, // #25 [ref=20x]
  { 0x000000000000000Fu, 0x000000000000000Fu, 0x02, { 0 }, OpRWInfo::kRW | OpRWInfo::kZExt | OpRWInfo::kRegPhysId }, // #26 [ref=1x]
  { 0x000000000000000Fu, 0x000000000000000Fu, 0x00, { 0 }, OpRWInfo::kRW | OpRWInfo::kZExt | OpRWInfo::kRegPhysId }, // #27 [ref=4x]
  { 0x000000000000000Fu, 0x0000000000000000u, 0x01, { 0 }, OpRWInfo::kRead | OpRWInfo::kRegPhysId }, // #28 [ref=11x]
  { 0x000000000000000Fu, 0x0000000000000000u, 0x03, { 0 }, OpRWInfo::kRead | OpRWInfo::kRegPhysId }, // #29 [ref=2x]
  { 0x0000000000000000u, 0x000000000000000Fu, 0x03, { 0 }, OpRWInfo::kWrite | OpRWInfo::kZExt | OpRWInfo::kRegPhysId }, // #30 [ref=1x]
  { 0x000000000000000Fu, 0x000000000000000Fu, 0x01, { 0 }, OpRWInfo::kRW | OpRWInfo::kZExt | OpRWInfo::kRegPhysId }, // #31 [ref=1x]
  { 0x0000000000000000u, 0x00000000000000FFu, 0x02, { 0 }, OpRWInfo::kWrite | OpRWInfo::kZExt | OpRWInfo::kRegPhysId }, // #32 [ref=1x]
  { 0x00000000000000FFu, 0x0000000000000000u, 0x00, { 0 }, OpRWInfo::kRead | OpRWInfo::kRegPhysId }, // #33 [ref=1x]
  { 0x0000000000000000u, 0x00000000000000FFu, 0xFF, { 0 }, OpRWInfo::kWrite | OpRWInfo::kZExt }, // #34 [ref=76x]
  { 0x0000000000000000u, 0x00000000000000FFu, 0xFF, { 0 }, OpRWInfo::kWrite }, // #35 [ref=6x]
  { 0x0000000000000000u, 0x000000000000000Fu, 0xFF, { 0 }, OpRWInfo::kWrite }, // #36 [ref=6x]
  { 0x0000000000000000u, 0x0000000000000003u, 0x02, { 0 }, OpRWInfo::kWrite | OpRWInfo::kRegPhysId }, // #37 [ref=1x]
  { 0x0000000000000003u, 0x0000000000000000u, 0x00, { 0 }, OpRWInfo::kRead | OpRWInfo::kRegPhysId }, // #38 [ref=1x]
  { 0x0000000000000001u, 0x0000000000000000u, 0xFF, { 0 }, OpRWInfo::kRead }, // #39 [ref=28x]
  { 0x0000000000000000u, 0x0000000000000000u, 0x02, { 0 }, OpRWInfo::kRW | OpRWInfo::kRegPhysId | OpRWInfo::kZExt }, // #40 [ref=2x]
  { 0x0000000000000000u, 0x0000000000000000u, 0x00, { 0 }, OpRWInfo::kRW | OpRWInfo::kRegPhysId | OpRWInfo::kZExt }, // #41 [ref=3x]
  { 0x0000000000000000u, 0x000000000000000Fu, 0xFF, { 0 }, OpRWInfo::kWrite | OpRWInfo::kZExt }, // #42 [ref=27x]
  { 0x00000000000003FFu, 0x00000000000003FFu, 0xFF, { 0 }, OpRWInfo::kRW | OpRWInfo::kZExt }, // #43 [ref=22x]
  { 0x00000000000003FFu, 0x0000000000000000u, 0xFF, { 0 }, OpRWInfo::kRead }, // #44 [ref=13x]
  { 0x0000000000000000u, 0x00000000000003FFu, 0xFF, { 0 }, OpRWInfo::kWrite | OpRWInfo::kZExt }, // #45 [ref=1x]
  { 0x0000000000000000u, 0x0000000000000003u, 0xFF, { 0 }, OpRWInfo::kWrite | OpRWInfo::kZExt }, // #46 [ref=15x]
  { 0x0000000000000000u, 0x0000000000000003u, 0x00, { 0 }, OpRWInfo::kWrite | OpRWInfo::kRegPhysId | OpRWInfo::kZExt }, // #47 [ref=2x]
  { 0x0000000000000000u, 0x0000000000000000u, 0x00, { 0 }, OpRWInfo::kWrite | OpRWInfo::kRegPhysId | OpRWInfo::kZExt }, // #48 [ref=2x]
  { 0x0000000000000003u, 0x0000000000000000u, 0x02, { 0 }, OpRWInfo::kRead | OpRWInfo::kRegPhysId }, // #49 [ref=4x]
  { 0x0000000000000000u, 0x0000000000000000u, 0x07, { 0 }, OpRWInfo::kWrite | OpRWInfo::kZExt | OpRWInfo::kMemPhysId }, // #50 [ref=1x]
  { 0x0000000000000000u, 0x0000000000000000u, 0x01, { 0 }, OpRWInfo::kRead | OpRWInfo::kRegPhysId }, // #51 [ref=1x]
  { 0x0000000000000000u, 0x0000000000000001u, 0xFF, { 0 }, OpRWInfo::kWrite | OpRWInfo::kZExt }, // #52 [ref=14x]
  { 0x0000000000000000u, 0x0000000000000001u, 0x00, { 0 }, OpRWInfo::kWrite | OpRWInfo::kRegPhysId }, // #53 [ref=1x]
  { 0x0000000000000000u, 0x0000000000000000u, 0x01, { 0 }, OpRWInfo::kRW | OpRWInfo::kRegPhysId | OpRWInfo::kZExt }, // #54 [ref=3x]
  { 0x000000000000000Fu, 0x0000000000000000u, 0x02, { 0 }, OpRWInfo::kRead | OpRWInfo::kRegPhysId }, // #55 [ref=20x]
  { 0xFFFFFFFFFFFFFFFFu, 0x0000000000000000u, 0xFF, { 0 }, OpRWInfo::kRead }, // #56 [ref=31x]
  { 0x000000000000FF00u, 0x0000000000000000u, 0xFF, { 0 }, OpRWInfo::kRead }, // #57 [ref=23x]
  { 0x0000000000000000u, 0x000000000000FF00u, 0xFF, { 0 }, OpRWInfo::kWrite }, // #58 [ref=1x]
  { 0x0000000000000000u, 0x0000000000000000u, 0x02, { 0 }, OpRWInfo::kWrite | OpRWInfo::kRegPhysId | OpRWInfo::kZExt }, // #59 [ref=1x]
  { 0x0000000000000000u, 0x0000000000000000u, 0x02, { 0 }, OpRWInfo::kRead | OpRWInfo::kRegPhysId }, // #60 [ref=2x]
  { 0x0000000000000000u, 0x0000000000000000u, 0x06, { 0 }, OpRWInfo::kRead | OpRWInfo::kMemPhysId }, // #61 [ref=1x]
  { 0x0000000000000000u, 0x000000000000000Fu, 0x01, { 0 }, OpRWInfo::kWrite | OpRWInfo::kZExt | OpRWInfo::kRegPhysId }, // #62 [ref=5x]
  { 0x0000000000000000u, 0x000000000000FFFFu, 0x00, { 0 }, OpRWInfo::kWrite | OpRWInfo::kZExt | OpRWInfo::kRegPhysId }, // #63 [ref=4x]
  { 0x0000000000000000u, 0x0000000000000007u, 0xFF, { 0 }, OpRWInfo::kWrite | OpRWInfo::kZExt }, // #64 [ref=2x]
  { 0x0000000000000000u, 0x0000000000000000u, 0x04, { 0 }, OpRWInfo::kWrite | OpRWInfo::kZExt | OpRWInfo::kRegPhysId }, // #65 [ref=1x]
  { 0x0000000000000001u, 0x0000000000000000u, 0x01, { 0 }, OpRWInfo::kRead | OpRWInfo::kRegPhysId }, // #66 [ref=10x]
  { 0x0000000000000000u, 0x000000000000000Fu, 0x00, { 0 }, OpRWInfo::kWrite | OpRWInfo::kZExt | OpRWInfo::kRegPhysId }, // #67 [ref=5x]
  { 0x0000000000000001u, 0x0000000000000000u, 0x00, { 0 }, OpRWInfo::kRead | OpRWInfo::kRegPhysId }, // #68 [ref=1x]
  { 0x0000000000000000u, 0x0000000000000001u, 0xFF, { 0 }, OpRWInfo::kWrite }, // #69 [ref=30x]
  { 0xFFFFFFFFFFFFFFFFu, 0xFFFFFFFFFFFFFFFFu, 0xFF, { 0 }, OpRWInfo::kRW | OpRWInfo::kZExt }, // #70 [ref=2x]
  { 0x0000000000000000u, 0x00000000FFFFFFFFu, 0xFF, { 0 }, OpRWInfo::kWrite | OpRWInfo::kZExt }, // #71 [ref=12x]
  { 0x0000000000000000u, 0xFFFFFFFFFFFFFFFFu, 0xFF, { 0 }, OpRWInfo::kWrite | OpRWInfo::kZExt }, // #72 [ref=16x]
  { 0x00000000FFFFFFFFu, 0x0000000000000000u, 0xFF, { 0 }, OpRWInfo::kRead }, // #73 [ref=19x]
  { 0x000000000000FFF0u, 0x0000000000000000u, 0xFF, { 0 }, OpRWInfo::kRead }, // #74 [ref=18x]
  { 0x000000000000000Fu, 0x000000000000000Fu, 0xFF, { 0 }, OpRWInfo::kRW | OpRWInfo::kZExt }, // #75 [ref=1x]
  { 0x0000000000000000u, 0x0000000000000000u, 0x00, { 0 }, OpRWInfo::kRW | OpRWInfo::kZExt | OpRWInfo::kRegPhysId }  // #76 [ref=1x]
};

static const RWInfoRm rwInfoRm[] = {
  { RWInfoRm::kCategoryNone      , 0x00, 0 , 0 }, // #0 [ref=1807x]
  { RWInfoRm::kCategoryConsistent, 0x03, 0 , RWInfoRm::kFlagAmbiguous }, // #1 [ref=8x]
  { RWInfoRm::kCategoryConsistent, 0x02, 0 , 0 }, // #2 [ref=195x]
  { RWInfoRm::kCategoryFixed     , 0x02, 16, 0 }, // #3 [ref=122x]
  { RWInfoRm::kCategoryFixed     , 0x02, 8 , 0 }, // #4 [ref=66x]
  { RWInfoRm::kCategoryFixed     , 0x02, 4 , 0 }, // #5 [ref=34x]
  { RWInfoRm::kCategoryConsistent, 0x04, 0 , 0 }, // #6 [ref=269x]
  { RWInfoRm::kCategoryFixed     , 0x01, 2 , 0 }, // #7 [ref=10x]
  { RWInfoRm::kCategoryFixed     , 0x00, 0 , 0 }, // #8 [ref=60x]
  { RWInfoRm::kCategoryFixed     , 0x03, 0 , 0 }, // #9 [ref=1x]
  { RWInfoRm::kCategoryConsistent, 0x01, 0 , RWInfoRm::kFlagAmbiguous }, // #10 [ref=20x]
  { RWInfoRm::kCategoryConsistent, 0x01, 0 , 0 }, // #11 [ref=13x]
  { RWInfoRm::kCategoryFixed     , 0x00, 16, 0 }, // #12 [ref=21x]
  { RWInfoRm::kCategoryFixed     , 0x00, 8 , 0 }, // #13 [ref=20x]
  { RWInfoRm::kCategoryConsistent, 0x02, 0 , RWInfoRm::kFlagAmbiguous }, // #14 [ref=15x]
  { RWInfoRm::kCategoryFixed     , 0x02, 1 , 0 }, // #15 [ref=5x]
  { RWInfoRm::kCategoryFixed     , 0x01, 4 , 0 }, // #16 [ref=4x]
  { RWInfoRm::kCategoryNone      , 0x00, 0 , RWInfoRm::kFlagAmbiguous }, // #17 [ref=22x]
  { RWInfoRm::kCategoryFixed     , 0x00, 10, 0 }, // #18 [ref=2x]
  { RWInfoRm::kCategoryNone      , 0x01, 0 , RWInfoRm::kFlagAmbiguous }, // #19 [ref=5x]
  { RWInfoRm::kCategoryFixed     , 0x00, 2 , 0 }, // #20 [ref=3x]
  { RWInfoRm::kCategoryConsistent, 0x06, 0 , 0 }, // #21 [ref=13x]
  { RWInfoRm::kCategoryFixed     , 0x03, 1 , 0 }, // #22 [ref=1x]
  { RWInfoRm::kCategoryFixed     , 0x03, 4 , 0 }, // #23 [ref=4x]
  { RWInfoRm::kCategoryFixed     , 0x03, 8 , 0 }, // #24 [ref=3x]
  { RWInfoRm::kCategoryFixed     , 0x03, 2 , 0 }, // #25 [ref=1x]
  { RWInfoRm::kCategoryFixed     , 0x02, 2 , 0 }, // #26 [ref=6x]
  { RWInfoRm::kCategoryFixed     , 0x00, 4 , 0 }, // #27 [ref=6x]
  { RWInfoRm::kCategoryNone      , 0x03, 0 , RWInfoRm::kFlagAmbiguous }, // #28 [ref=1x]
  { RWInfoRm::kCategoryFixed     , 0x03, 16, 0 }, // #29 [ref=6x]
  { RWInfoRm::kCategoryFixed     , 0x00, 64, 0 }, // #30 [ref=1x]
  { RWInfoRm::kCategoryFixed     , 0x01, 1 , 0 }, // #31 [ref=32x]
  { RWInfoRm::kCategoryFixed     , 0x01, 8 , 0 }, // #32 [ref=2x]
  { RWInfoRm::kCategoryFixed     , 0x01, 2 , RWInfoRm::kFlagAmbiguous }, // #33 [ref=3x]
  { RWInfoRm::kCategoryFixed     , 0x04, 8 , 0 }, // #34 [ref=34x]
  { RWInfoRm::kCategoryFixed     , 0x04, 4 , 0 }, // #35 [ref=37x]
  { RWInfoRm::kCategoryFixed     , 0x00, 32, 0 }, // #36 [ref=4x]
  { RWInfoRm::kCategoryFixed     , 0x02, 8 , RWInfoRm::kFlagAmbiguous }, // #37 [ref=1x]
  { RWInfoRm::kCategoryFixed     , 0x02, 4 , RWInfoRm::kFlagAmbiguous }, // #38 [ref=1x]
  { RWInfoRm::kCategoryHalf      , 0x02, 0 , 0 }, // #39 [ref=14x]
  { RWInfoRm::kCategoryHalf      , 0x01, 0 , 0 }, // #40 [ref=10x]
  { RWInfoRm::kCategoryConsistent, 0x04, 0 , RWInfoRm::kFlagAmbiguous }, // #41 [ref=4x]
  { RWInfoRm::kCategoryFixed     , 0x04, 16, 0 }, // #42 [ref=27x]
  { RWInfoRm::kCategoryFixed     , 0x02, 64, 0 }, // #43 [ref=6x]
  { RWInfoRm::kCategoryFixed     , 0x01, 16, 0 }, // #44 [ref=6x]
  { RWInfoRm::kCategoryFixed     , 0x01, 32, 0 }, // #45 [ref=4x]
  { RWInfoRm::kCategoryConsistent, 0x0C, 0 , 0 }, // #46 [ref=15x]
  { RWInfoRm::kCategoryFixed     , 0x0C, 8 , 0 }, // #47 [ref=4x]
  { RWInfoRm::kCategoryFixed     , 0x0C, 4 , 0 }, // #48 [ref=4x]
  { RWInfoRm::kCategoryFixed     , 0x04, 32, 0 }, // #49 [ref=7x]
  { RWInfoRm::kCategoryConsistent, 0x03, 0 , 0 }, // #50 [ref=13x]
  { RWInfoRm::kCategoryNone      , 0x02, 0 , 0 }, // #51 [ref=1x]
  { RWInfoRm::kCategoryFixed     , 0x03, 8 , RWInfoRm::kFlagAmbiguous }, // #52 [ref=1x]
  { RWInfoRm::kCategoryFixed     , 0x02, 32, 0 }, // #53 [ref=1x]
  { RWInfoRm::kCategoryFixed     , 0x04, 1 , 0 }, // #54 [ref=1x]
  { RWInfoRm::kCategoryFixed     , 0x04, 2 , 0 }, // #55 [ref=1x]
  { RWInfoRm::kCategoryQuarter   , 0x01, 0 , 0 }, // #56 [ref=6x]
  { RWInfoRm::kCategoryEighth    , 0x01, 0 , 0 }, // #57 [ref=3x]
  { RWInfoRm::kCategoryQuarter   , 0x02, 0 , 0 }, // #58 [ref=4x]
  { RWInfoRm::kCategoryEighth    , 0x02, 0 , 0 }, // #59 [ref=2x]
  { RWInfoRm::kCategoryFixed     , 0x0C, 16, 0 }, // #60 [ref=1x]
  { RWInfoRm::kCategoryFixed     , 0x06, 16, 0 }  // #61 [ref=12x]
};

// ----------------------------------------------------------------------------
// ${InstRWInfoTable:End}

#ifndef ASMJIT_DISABLE_INST_API

static const uint64_t rwRegGroupByteMask[Reg::kGroupCount] = {
  0x00000000000000FFu, // GP.
  0xFFFFFFFFFFFFFFFFu, // XMM|YMM|ZMM.
  0x00000000000000FFu, // MM.
  0x00000000000000FFu, // KReg.
  0x0000000000000003u, // SReg.
  0x00000000000000FFu, // CReg.
  0x00000000000000FFu, // DReg.
  0x00000000000003FFu, // St().
  0x000000000000FFFFu, // BND.
  0x00000000000000FFu  // RIP.
};

// TODO: Make universal.
static ASMJIT_INLINE uint32_t gpRegSizeByArchId(uint32_t archId) noexcept {
  static const uint8_t table[] = { 0, 4, 8, 4, 8 };
  return table[archId];
}

static ASMJIT_INLINE void rwZeroExtendGp(OpRWInfo& opRwInfo, const Gp& reg, uint32_t nativeGpSize) noexcept {
  ASMJIT_ASSERT(BaseReg::isGp(reg.as<Operand>()));
  if (reg.size() + 4 == nativeGpSize) {
    opRwInfo.addOpFlags(OpRWInfo::kZExt);
    opRwInfo.setExtendByteMask(~opRwInfo.writeByteMask() & 0xFFu);
  }
}

static ASMJIT_INLINE void rwZeroExtendAvxVec(OpRWInfo& opRwInfo, const Vec& reg) noexcept {
  ASMJIT_UNUSED(reg);

  uint64_t msk = ~Support::fillTrailingBits(opRwInfo.writeByteMask());
  if (msk) {
    opRwInfo.addOpFlags(OpRWInfo::kZExt);
    opRwInfo.setExtendByteMask(msk);
  }
}

static ASMJIT_INLINE void rwZeroExtendNonVec(OpRWInfo& opRwInfo, const Reg& reg) noexcept {
  uint64_t msk = ~Support::fillTrailingBits(opRwInfo.writeByteMask()) & rwRegGroupByteMask[reg.group()];
  if (msk) {
    opRwInfo.addOpFlags(OpRWInfo::kZExt);
    opRwInfo.setExtendByteMask(msk);
  }
}

ASMJIT_FAVOR_SIZE Error InstInternal::queryRWInfo(uint32_t archId, const BaseInst& inst, const Operand_* operands, uint32_t opCount, InstRWInfo& out) noexcept {
  // Only called when `archId` matches X86 family.
  ASMJIT_ASSERT(ArchInfo::isX86Family(archId));

  // Get the instruction data.
  uint32_t instId = inst.id();
  if (ASMJIT_UNLIKELY(!Inst::isDefinedId(instId)))
    return DebugUtils::errored(kErrorInvalidInstruction);

  uint32_t nativeGpSize = gpRegSizeByArchId(archId);

  out._instFlags = 0;
  out._opCount = uint8_t(opCount);
  out._extraReg.reset();

  // Each RWInfo contains two indexes
  //   [0] - OpCount == 2
  //   [1] - OpCount != 2
  // They are used this way as there are instructions that have 2 and 3
  // operand overloads that use different semantics. So instead of adding
  // more special cases we just separated their data tables.
  const RWInfo& instRwInfo = rwInfo[rwInfoIndex[instId * 2u + uint32_t(opCount != 2)]];
  const RWInfoRm& instRmInfo = rwInfoRm[instRwInfo.rmInfo];

  constexpr uint32_t R = OpRWInfo::kRead;
  constexpr uint32_t W = OpRWInfo::kWrite;
  constexpr uint32_t X = OpRWInfo::kRW;
  constexpr uint32_t RegM = OpRWInfo::kRegMem;
  constexpr uint32_t RegPhys = OpRWInfo::kRegPhysId;
  constexpr uint32_t MibRead = OpRWInfo::kMemBaseRead | OpRWInfo::kMemIndexRead;

  switch (instRwInfo.category) {
    case RWInfo::kCategoryGeneric: {
      // TODO: Flags...
      uint32_t i;
      uint32_t rmOpsMask = 0;
      uint32_t rmMaxSize = 0;

      for (i = 0; i < opCount; i++) {
        OpRWInfo& op = out._operands[i];
        const Operand_& srcOp = operands[i];
        const RWInfoOp& rwOpData = rwInfoOp[instRwInfo.opInfoIndex[i]];

        if (!srcOp.isRegOrMem()) {
          op.reset();
          continue;
        }

        op._opFlags = rwOpData.flags & ~(OpRWInfo::kZExt);
        op._physId = rwOpData.physId;
        op._rmSize = 0;
        op._resetReserved();

        uint64_t rByteMask = rwOpData.rByteMask;
        uint64_t wByteMask = rwOpData.wByteMask;

        if (op.isRead()  && !rByteMask) rByteMask = Support::lsbMask<uint64_t>(srcOp.size());
        if (op.isWrite() && !wByteMask) wByteMask = Support::lsbMask<uint64_t>(srcOp.size());

        op._readByteMask = rByteMask;
        op._writeByteMask = wByteMask;
        op._extendByteMask = 0;

        if (srcOp.isReg()) {
          // Zero extension.
          if (op.isWrite()) {
            if (srcOp.as<Reg>().isGp()) {
              // GP registers on X64 are special:
              //   - 8-bit and 16-bit writes aren't zero extended.
              //   - 32-bit writes ARE zero extended.
              rwZeroExtendGp(op, srcOp.as<Gp>(), nativeGpSize);
            }
            else if (rwOpData.flags & OpRWInfo::kZExt) {
              // Otherwise follow ZExt.
              rwZeroExtendNonVec(op, srcOp.as<Gp>());
            }
          }

          // Aggregate values required to calculate valid Reg/M info.
          rmMaxSize  = Support::max(rmMaxSize, srcOp.size());
          rmOpsMask |= Support::bitMask<uint32_t>(i);
        }
        else {
          op.addOpFlags(MibRead);
        }
      }

      rmOpsMask &= instRmInfo.rmOpsMask;
      if (rmOpsMask) {
        Support::BitWordIterator<uint32_t> it(rmOpsMask);
        do {
          i = it.next();

          OpRWInfo& op = out._operands[i];
          op.addOpFlags(RegM);

          switch (instRmInfo.category) {
            case RWInfoRm::kCategoryFixed:
              op.setRmSize(instRmInfo.fixedSize);
              break;
            case RWInfoRm::kCategoryConsistent:
              op.setRmSize(operands[i].size());
              break;
            case RWInfoRm::kCategoryHalf:
              op.setRmSize(rmMaxSize / 2u);
              break;
            case RWInfoRm::kCategoryQuarter:
              op.setRmSize(rmMaxSize / 4u);
              break;
            case RWInfoRm::kCategoryEighth:
              op.setRmSize(rmMaxSize / 8u);
              break;
          }
        } while (it.hasNext());
      }

      return kErrorOk;
    }

    case RWInfo::kCategoryImul: {
      // Special case for 'imul' instruction.
      //
      // There are 3 variants in general:
      //
      //   1. Standard multiplication: 'A = A * B'.
      //   2. Multiplication with imm: 'A = B * C'.
      //   3. Extended multiplication: 'A:B = B * C'.

      // TODO: Flags...
      if (opCount == 2) {
        if (operands[0].isReg() && operands[1].isImm()) {
          out._operands[0].reset(X, operands[0].size());
          out._operands[1].reset();

          rwZeroExtendGp(out._operands[0], operands[0].as<Gp>(), nativeGpSize);
          return kErrorOk;
        }

        if (Reg::isGpw(operands[0]) && operands[1].size() == 1) {
          // imul ax, r8/m8 <- AX = AL * r8/m8
          out._operands[0].reset(X | RegPhys, 2, Gp::kIdAx);
          out._operands[0].setReadByteMask(Support::lsbMask<uint64_t>(1));
          out._operands[1].reset(R | RegM, 1);
        }
        else {
          // imul r?, r?/m?
          out._operands[0].reset(X, operands[0].size());
          out._operands[1].reset(R | RegM, operands[0].size());
          rwZeroExtendGp(out._operands[0], operands[0].as<Gp>(), nativeGpSize);
        }

        if (operands[1].isMem())
          out._operands[1].addOpFlags(MibRead);
        return kErrorOk;
      }

      if (opCount == 3) {
        if (operands[2].isImm()) {
          out._operands[0].reset(W, operands[0].size());
          out._operands[1].reset(R | RegM, operands[1].size());
          out._operands[2].reset();

          rwZeroExtendGp(out._operands[0], operands[0].as<Gp>(), nativeGpSize);
          if (operands[1].isMem())
            out._operands[1].addOpFlags(MibRead);
          return kErrorOk;
        }
        else {
          out._operands[0].reset(W | RegPhys, operands[0].size(), Gp::kIdDx);
          out._operands[1].reset(X | RegPhys, operands[1].size(), Gp::kIdAx);
          out._operands[2].reset(R | RegM, operands[2].size());

          rwZeroExtendGp(out._operands[0], operands[0].as<Gp>(), nativeGpSize);
          rwZeroExtendGp(out._operands[1], operands[1].as<Gp>(), nativeGpSize);
          if (operands[2].isMem())
            out._operands[2].addOpFlags(MibRead);
          return kErrorOk;
        }
      }
      break;
    }

    case RWInfo::kCategoryMov: {
      // Special case for 'movhpd' instruction. Here there are some variants that
      // we have to handle as mov can be used to move between GP, segment, control
      // and debug registers. Moving between GP registers also allow to use memory
      // operand.

      // TODO: Flags...
      if (opCount == 2) {
        if (operands[0].isReg() && operands[1].isReg()) {
          const Reg& o0 = operands[0].as<Reg>();
          const Reg& o1 = operands[1].as<Reg>();

          if (o0.isGp() && o1.isGp()) {
            out._operands[0].reset(W | RegM, operands[0].size());
            out._operands[1].reset(R | RegM, operands[1].size());

            rwZeroExtendGp(out._operands[0], operands[0].as<Gp>(), nativeGpSize);
            return kErrorOk;
          }

          if (o0.isGp() && o1.isSReg()) {
            out._operands[0].reset(W | RegM, nativeGpSize);
            out._operands[0].setRmSize(2);
            out._operands[1].reset(R, 2);
            return kErrorOk;
          }

          if (o0.isSReg() && o1.isGp()) {
            out._operands[0].reset(W, 2);
            out._operands[1].reset(R | RegM, 2);
            out._operands[1].setRmSize(2);
            return kErrorOk;
          }

          if (o0.isGp() && (o1.isCReg() || o1.isDReg())) {
            out._operands[0].reset(W, nativeGpSize);
            out._operands[1].reset(R, nativeGpSize);
            return kErrorOk;
          }

          if ((o0.isCReg() || o0.isDReg()) && o1.isGp()) {
            out._operands[0].reset(W, nativeGpSize);
            out._operands[1].reset(R, nativeGpSize);
            return kErrorOk;
          }
        }

        if (operands[0].isReg() && operands[1].isMem()) {
          const Reg& o0 = operands[0].as<Reg>();
          const Mem& o1 = operands[1].as<Mem>();

          if (o0.isGp()) {
            if (!o1.isOffset64Bit())
              out._operands[0].reset(W, o0.size());
            else
              out._operands[0].reset(W | RegPhys, o0.size(), Gp::kIdAx);

            out._operands[1].reset(R | MibRead, o0.size());
            rwZeroExtendGp(out._operands[0], operands[0].as<Gp>(), nativeGpSize);
            return kErrorOk;
          }

          if (o0.isSReg()) {
            out._operands[0].reset(W, 2);
            out._operands[1].reset(R, 2);
            return kErrorOk;
          }
        }

        if (operands[0].isMem() && operands[1].isReg()) {
          const Mem& o0 = operands[0].as<Mem>();
          const Reg& o1 = operands[1].as<Reg>();

          if (o1.isGp()) {
            out._operands[0].reset(W | MibRead, o1.size());
            if (!o0.isOffset64Bit())
              out._operands[1].reset(R, o1.size());
            else
              out._operands[1].reset(R | RegPhys, o1.size(), Gp::kIdAx);
            return kErrorOk;
          }

          if (o1.isSReg()) {
            out._operands[0].reset(W | MibRead, 2);
            out._operands[1].reset(R, 2);
            return kErrorOk;
          }
        }

        if (Reg::isGp(operands[0]) && operands[1].isImm()) {
          const Reg& o0 = operands[0].as<Reg>();
          out._operands[0].reset(W | RegM, o0.size());
          out._operands[1].reset();

          rwZeroExtendGp(out._operands[0], operands[0].as<Gp>(), nativeGpSize);
          return kErrorOk;
        }

        if (operands[0].isMem() && operands[1].isImm()) {
          const Reg& o0 = operands[0].as<Reg>();
          out._operands[0].reset(W | MibRead, o0.size());
          out._operands[1].reset();
          return kErrorOk;
        }
      }
      break;
    }

    case RWInfo::kCategoryMovh64: {
      // Special case for 'movhpd|movhps' instructions. Note that this is only
      // required for legacy (non-AVX) variants as AVX instructions use either
      // 2 or 3 operands that are use `kCategoryGeneric`.
      if (opCount == 2) {
        if (BaseReg::isVec(operands[0]) && operands[1].isMem()) {
          out._operands[0].reset(W, 8);
          out._operands[0].setWriteByteMask(Support::lsbMask<uint64_t>(8) << 8);
          out._operands[1].reset(R | MibRead, 8);
          return kErrorOk;
        }

        if (operands[0].isMem() && BaseReg::isVec(operands[1])) {
          out._operands[0].reset(W | MibRead, 8);
          out._operands[1].reset(R, 8);
          out._operands[1].setReadByteMask(Support::lsbMask<uint64_t>(8) << 8);
          return kErrorOk;
        }
      }
      break;
    }

    case RWInfo::kCategoryVmaskmov: {
      // Special case for 'vmaskmovpd|vmaskmovps|vpmaskmovd|vpmaskmovq' instructions.
      if (opCount == 3) {
        if (BaseReg::isVec(operands[0]) && BaseReg::isVec(operands[1]) && operands[2].isMem()) {
          out._operands[0].reset(W, operands[0].size());
          out._operands[1].reset(R, operands[1].size());
          out._operands[2].reset(R | MibRead, operands[1].size());

          rwZeroExtendAvxVec(out._operands[0], operands[0].as<Vec>());
          return kErrorOk;
        }

        if (operands[0].isMem() && BaseReg::isVec(operands[1]) && BaseReg::isVec(operands[2])) {
          out._operands[0].reset(X | MibRead, operands[1].size());
          out._operands[1].reset(R, operands[1].size());
          out._operands[2].reset(R, operands[2].size());
          return kErrorOk;
        }
      }
      break;
    }

    case RWInfo::kCategoryVmovddup: {
      // Special case for 'vmovddup' instruction. This instruction has an
      // interesting semantic as 128-bit XMM version only uses 64-bit memory
      // operand (m64), however, 256/512-bit versions use 256/512-bit memory
      // operand, respectively.
      if (opCount == 2) {
        if (BaseReg::isVec(operands[0]) && BaseReg::isVec(operands[1])) {
          uint32_t o0Size = operands[0].size();
          uint32_t o1Size = o0Size == 16 ? 8 : o0Size;

          out._operands[0].reset(W, o0Size);
          out._operands[1].reset(R | RegM, o1Size);
          out._operands[1]._readByteMask &= 0x00FF00FF00FF00FFu;

          rwZeroExtendAvxVec(out._operands[0], operands[0].as<Vec>());
          return kErrorOk;
        }

        if (BaseReg::isVec(operands[0]) && operands[1].isMem()) {
          uint32_t o0Size = operands[0].size();
          uint32_t o1Size = o0Size == 16 ? 8 : o0Size;

          out._operands[0].reset(W, o0Size);
          out._operands[1].reset(R | MibRead, o1Size);

          rwZeroExtendAvxVec(out._operands[0], operands[0].as<Vec>());
          return kErrorOk;
        }
      }
      break;
    }

    case RWInfo::kCategoryVmovmskpd:
    case RWInfo::kCategoryVmovmskps: {
      // Special case for 'vmovmskpd|vmovmskps' instructions.
      if (opCount == 2) {
        if (BaseReg::isGp(operands[0]) && BaseReg::isVec(operands[1])) {
          out._operands[0].reset(W, 1);
          out._operands[0].setExtendByteMask(Support::lsbMask<uint32_t>(nativeGpSize - 1) << 1);
          out._operands[1].reset(R, operands[1].size());
          return kErrorOk;
        }
      }
      break;
    }

    case RWInfo::kCategoryVmov1_2:
    case RWInfo::kCategoryVmov1_4:
    case RWInfo::kCategoryVmov1_8: {
      // Special case for instructions where the destination is 1:N (narrowing).
      //
      // Vmov1_2:
      //   vcvtpd2dq|vcvttpd2dq
      //   vcvtpd2udq|vcvttpd2udq
      //   vcvtpd2ps|vcvtps2ph
      //   vcvtqq2ps|vcvtuqq2ps
      //   vpmovwb|vpmovswb|vpmovuswb
      //   vpmovdw|vpmovsdw|vpmovusdw
      //   vpmovqd|vpmovsqd|vpmovusqd
      //
      // Vmov1_4:
      //   vpmovdb|vpmovsdb|vpmovusdb
      //   vpmovqw|vpmovsqw|vpmovusqw
      //
      // Vmov1_8:
      //   pmovmskb|vpmovmskb
      //   vpmovqb|vpmovsqb|vpmovusqb
      uint32_t shift = instRwInfo.category - RWInfo::kCategoryVmov1_2 + 1;

      if (opCount >= 2) {
        if (opCount >= 3) {
          if (opCount > 3)
            return DebugUtils::errored(kErrorInvalidInstruction);
          out._operands[2].reset();
        }

        if (operands[0].isReg() && operands[1].isReg()) {
          uint32_t size1 = operands[1].size();
          uint32_t size0 = size1 >> shift;

          out._operands[0].reset(W, size0);
          out._operands[1].reset(R, size1);

          if (instRmInfo.rmOpsMask & 0x1) {
            out._operands[0].addOpFlags(RegM);
            out._operands[0].setRmSize(size0);
          }

          if (instRmInfo.rmOpsMask & 0x2) {
            out._operands[1].addOpFlags(RegM);
            out._operands[1].setRmSize(size1);
          }

          // Handle 'pmovmskb|vpmovmskb'.
          if (BaseReg::isGp(operands[0]))
            rwZeroExtendGp(out._operands[0], operands[0].as<Gp>(), nativeGpSize);

          if (BaseReg::isVec(operands[0]))
            rwZeroExtendAvxVec(out._operands[0], operands[0].as<Vec>());

          return kErrorOk;
        }

        if (operands[0].isReg() && operands[1].isMem()) {
          uint32_t size1 = operands[1].size() ? operands[1].size() : uint32_t(16);
          uint32_t size0 = size1 >> shift;

          out._operands[0].reset(W, size0);
          out._operands[1].reset(R | MibRead, size1);
          return kErrorOk;
        }

        if (operands[0].isMem() && operands[1].isReg()) {
          uint32_t size1 = operands[1].size();
          uint32_t size0 = size1 >> shift;

          out._operands[0].reset(W | MibRead, size0);
          out._operands[1].reset(R, size1);
          return kErrorOk;
        }
      }
      break;
    }

    case RWInfo::kCategoryVmov2_1:
    case RWInfo::kCategoryVmov4_1:
    case RWInfo::kCategoryVmov8_1: {
      // Special case for instructions where the destination is N:1 (widening).
      //
      // Vmov2_1:
      //   vcvtdq2pd|vcvtudq2pd
      //   vcvtps2pd|vcvtph2ps
      //   vcvtps2qq|vcvtps2uqq
      //   vcvttps2qq|vcvttps2uqq
      //   vpmovsxbw|vpmovzxbw
      //   vpmovsxwd|vpmovzxwd
      //   vpmovsxdq|vpmovzxdq
      //
      // Vmov4_1:
      //   vpmovsxbd|vpmovzxbd
      //   vpmovsxwq|vpmovzxwq
      //
      // Vmov8_1:
      //   vpmovsxbq|vpmovzxbq
      uint32_t shift = instRwInfo.category - RWInfo::kCategoryVmov2_1 + 1;

      if (opCount >= 2) {
        if (opCount >= 3) {
          if (opCount > 3)
            return DebugUtils::errored(kErrorInvalidInstruction);
          out._operands[2].reset();
        }

        uint32_t size0 = operands[0].size();
        uint32_t size1 = size0 >> shift;

        out._operands[0].reset(W, size0);
        out._operands[1].reset(R, size1);

        if (operands[0].isReg() && operands[1].isReg()) {
          if (instRmInfo.rmOpsMask & 0x1) {
            out._operands[0].addOpFlags(RegM);
            out._operands[0].setRmSize(size0);
          }

          if (instRmInfo.rmOpsMask & 0x2) {
            out._operands[1].addOpFlags(RegM);
            out._operands[1].setRmSize(size1);
          }
          return kErrorOk;
        }

        if (operands[0].isReg() && operands[1].isMem()) {
          out._operands[1].addOpFlags(MibRead);
          return kErrorOk;
        }
      }
      break;
    }
  }

  return DebugUtils::errored(kErrorInvalidInstruction);
}
#endif // ASMJIT_DISABLE_INST_API

// ============================================================================
// [asmjit::x86::InstInternal - QueryFeatures]
// ============================================================================

#ifndef ASMJIT_DISABLE_INST_API
struct RegAnalysis {
  uint32_t regTypeMask;
  uint32_t highVecUsed;

  inline bool hasRegType(uint32_t regType) const noexcept {
    return Support::bitTest(regTypeMask, regType);
  }
};

static RegAnalysis InstInternal_regAnalysis(const Operand_* operands, uint32_t opCount) noexcept {
  uint32_t mask = 0;
  uint32_t highVecUsed = 0;

  for (uint32_t i = 0; i < opCount; i++) {
    const Operand_& op = operands[i];
    if (op.isReg()) {
      const BaseReg& reg = op.as<BaseReg>();
      mask |= Support::bitMask(reg.type());
      if (reg.isVec())
        highVecUsed |= uint32_t(reg.id() >= 16 && reg.id() < 32);
    }
    else if (op.isMem()) {
      const BaseMem& mem = op.as<BaseMem>();
      if (mem.hasBaseReg()) mask |= Support::bitMask(mem.baseType());
      if (mem.hasIndexReg()) mask |= Support::bitMask(mem.indexType());
    }
  }

  return RegAnalysis { mask, highVecUsed };
}

ASMJIT_FAVOR_SIZE Error InstInternal::queryFeatures(uint32_t archId, const BaseInst& inst, const Operand_* operands, uint32_t opCount, BaseFeatures& out) noexcept {
  // Only called when `archId` matches X86 family.
  ASMJIT_UNUSED(archId);
  ASMJIT_ASSERT(ArchInfo::isX86Family(archId));

  // Get the instruction data.
  uint32_t instId = inst.id();
  uint32_t options = inst.options();

  if (ASMJIT_UNLIKELY(!Inst::isDefinedId(instId)))
    return DebugUtils::errored(kErrorInvalidInstruction);

  const InstDB::InstInfo& instInfo = InstDB::infoById(instId);
  const InstDB::ExecutionInfo& executionInfo = instInfo.executionInfo();

  const uint8_t* fData = executionInfo.featuresData();
  const uint8_t* fEnd = executionInfo.featuresEnd();

  // Copy all features to `out`.
  out.reset();
  do {
    uint32_t feature = fData[0];
    if (!feature)
      break;
    out.add(feature);
  } while (++fData != fEnd);

  // Since AsmJit aggregates instructions that share the same name we have to
  // deal with some special cases and also with MMX/SSE and AVX/AVX2 overlaps.
  if (fData != executionInfo.featuresData()) {
    RegAnalysis regAnalysis = InstInternal_regAnalysis(operands, opCount);

    // Handle MMX vs SSE overlap.
    if (out.has(Features::kMMX) || out.has(Features::kMMX2)) {
      // Only instructions defined by SSE and SSE2 overlap. Instructions
      // introduced by newer instruction sets like SSE3+ don't state MMX as
      // they require SSE3+.
      if (out.has(Features::kSSE) || out.has(Features::kSSE2)) {
        if (!regAnalysis.hasRegType(Reg::kTypeXmm)) {
          // The instruction doesn't use XMM register(s), thus it's MMX/MMX2 only.
          out.remove(Features::kSSE);
          out.remove(Features::kSSE2);
        }
        else {
          out.remove(Features::kMMX);
          out.remove(Features::kMMX2);
        }

        // Special case: PEXTRW instruction is MMX/SSE2 instruction. However,
        // MMX/SSE version cannot access memory (only register to register
        // extract) so when SSE4.1 introduced the whole family of PEXTR/PINSR
        // instructions they also introduced PEXTRW with a new opcode 0x15 that
        // can extract directly to memory. This instruction is, of course, not
        // compatible with MMX/SSE2 and would #UD if SSE4.1 is not supported.
        if (instId == Inst::kIdPextrw) {
          ASMJIT_ASSERT(out.has(Features::kSSE2));
          ASMJIT_ASSERT(out.has(Features::kSSE4_1));

          if (opCount > 0 && operands[0].isMem())
            out.remove(Features::kSSE2);
          else
            out.remove(Features::kSSE4_1);
        }
      }
    }

    // Handle PCLMULQDQ vs VPCLMULQDQ.
    if (out.has(Features::kVPCLMULQDQ)) {
      if (regAnalysis.hasRegType(Reg::kTypeZmm) || Support::bitTest(options, Inst::kOptionEvex)) {
        // AVX512_F & VPCLMULQDQ.
        out.remove(Features::kAVX, Features::kPCLMULQDQ);
      }
      else if (regAnalysis.hasRegType(Reg::kTypeYmm)) {
        out.remove(Features::kAVX512_F, Features::kAVX512_VL);
      }
      else {
        // AVX & PCLMULQDQ.
        out.remove(Features::kAVX512_F, Features::kAVX512_VL, Features::kVPCLMULQDQ);
      }
    }

    // Handle AVX vs AVX2 overlap.
    if (out.has(Features::kAVX) && out.has(Features::kAVX2)) {
      bool isAVX2 = true;
      // Special case: VBROADCASTSS and VBROADCASTSD were introduced in AVX, but
      // only version that uses memory as a source operand. AVX2 then added support
      // for register source operand.
      if (instId == Inst::kIdVbroadcastss || instId == Inst::kIdVbroadcastsd) {
        if (opCount > 1 && operands[1].isMem())
          isAVX2 = false;
      }
      else {
        // AVX instruction set doesn't support integer operations on YMM registers
        // as these were later introcuced by AVX2. In our case we have to check if
        // YMM register(s) are in use and if that is the case this is an AVX2 instruction.
        if (!(regAnalysis.regTypeMask & Support::bitMask(Reg::kTypeYmm, Reg::kTypeZmm)))
          isAVX2 = false;
      }

      if (isAVX2)
        out.remove(Features::kAVX);
      else
        out.remove(Features::kAVX2);
    }

    // Handle AVX|AVX2|FMA|F16C vs AVX512 overlap.
    if (out.has(Features::kAVX) || out.has(Features::kAVX2) || out.has(Features::kFMA) || out.has(Features::kF16C)) {
      // Only AVX512-F|BW|DQ allow to encode AVX/AVX2/FMA/F16C instructions
      if (out.has(Features::kAVX512_F) || out.has(Features::kAVX512_BW) || out.has(Features::kAVX512_DQ)) {
        uint32_t hasEvex = options & (Inst::kOptionEvex | Inst::_kOptionAvx512Mask);
        uint32_t hasKMask = inst.extraReg().type() == Reg::kTypeKReg;
        uint32_t hasKOrZmm = regAnalysis.regTypeMask & Support::bitMask(Reg::kTypeZmm, Reg::kTypeKReg);

        if (!(hasEvex | hasKMask | hasKOrZmm | regAnalysis.highVecUsed))
          out.remove(Features::kAVX512_F, Features::kAVX512_BW, Features::kAVX512_DQ, Features::kAVX512_VL);
      }
    }

    // Clear AVX512_VL if ZMM register is used.
    if (regAnalysis.hasRegType(Reg::kTypeZmm))
      out.remove(Features::kAVX512_VL);
  }

  return kErrorOk;
}
#endif

// ============================================================================
// [asmjit::Inst - Unit]
// ============================================================================

#if defined(ASMJIT_TEST)
UNIT(asmjit_x86_inst_bits) {
  INFO("Checking validity of Inst enums");

  // Cross-validate prefixes.
  EXPECT(Inst::kOptionRex  == 0x40000000u, "REX prefix must be at 0x40000000");
  EXPECT(Inst::kOptionVex3 == 0x00000400u, "VEX3 prefix must be at 0x00000400");
  EXPECT(Inst::kOptionEvex == 0x00001000u, "EVEX prefix must be at 0x00001000");

  // These could be combined together to form a valid REX prefix, they must match.
  EXPECT(uint32_t(Inst::kOptionOpCodeB) == uint32_t(Opcode::kB));
  EXPECT(uint32_t(Inst::kOptionOpCodeX) == uint32_t(Opcode::kX));
  EXPECT(uint32_t(Inst::kOptionOpCodeR) == uint32_t(Opcode::kR));
  EXPECT(uint32_t(Inst::kOptionOpCodeW) == uint32_t(Opcode::kW));

  uint32_t rex_rb = (Opcode::kR >> Opcode::kREX_Shift) | (Opcode::kB >> Opcode::kREX_Shift) | 0x40;
  uint32_t rex_rw = (Opcode::kR >> Opcode::kREX_Shift) | (Opcode::kW >> Opcode::kREX_Shift) | 0x40;
  EXPECT(rex_rb == 0x45, "Opcode::kR|B must form a valid REX prefix (0x45) if combined with 0x40");
  EXPECT(rex_rw == 0x4C, "Opcode::kR|W must form a valid REX prefix (0x4C) if combined with 0x40");
}
#endif

#if defined(ASMJIT_TEST) && !defined(ASMJIT_DISABLE_TEXT)
UNIT(asmjit_x86_inst_names) {
  // All known instructions should be matched.
  INFO("Matching all X86 instructions");
  for (uint32_t a = 0; a < Inst::_kIdCount; a++) {
    uint32_t b = InstDB::idByName(InstDB::infoById(a).name());
    EXPECT(a == b,
      "Should match existing instruction \"%s\" {id:%u} != \"%s\" {id:%u}",
        InstDB::infoById(a).name(), a,
        InstDB::infoById(b).name(), b);
  }

  // Everything else should return `Inst::kIdNone`.
  INFO("Trying to look-up instructions that don't exist");
  EXPECT(InstDB::idByName(nullptr)  == Inst::kIdNone, "Should return Inst::kIdNone for null input");
  EXPECT(InstDB::idByName("")       == Inst::kIdNone, "Should return Inst::kIdNone for empty string");
  EXPECT(InstDB::idByName("_")      == Inst::kIdNone, "Should return Inst::kIdNone for unknown instruction");
  EXPECT(InstDB::idByName("123xyz") == Inst::kIdNone, "Should return Inst::kIdNone for unknown instruction");
}
#endif

ASMJIT_END_SUB_NAMESPACE

#endif // ASMJIT_BUILD_X86
