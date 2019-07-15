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
#ifndef ASMJIT_NO_TEXT
  #define NAME_DATA_INDEX(X) X
#else
  #define NAME_DATA_INDEX(X) 0
#endif

// Defines an X86 instruction.
#define INST(id, encoding, opcode0, opcode1, mainOpcodeIndex, altOpcodeIndex, nameDataIndex, commomInfoIndexA, commomInfoIndexB) { \
  uint32_t(NAME_DATA_INDEX(nameDataIndex)), \
  uint32_t(commomInfoIndexA),               \
  uint32_t(commomInfoIndexB),               \
  uint8_t(InstDB::kEncoding##encoding),     \
  uint8_t((opcode0) & 0xFFu),               \
  uint8_t(mainOpcodeIndex),                 \
  uint8_t(altOpcodeIndex)                   \
}

const InstDB::InstInfo InstDB::_instInfoTable[] = {
  /*--------------------+--------------------+------------------+--------+------------------+--------+----+----+------+----+----+
  |    Instruction      |    Instruction     |    Main Opcode   |  EVEX  |Alternative Opcode|  EVEX  |Op0X|Op1X|Name-X|IdxA|IdxB|
  |     Id & Name       |      Encoding      |  (pp+mmm|op/o|L|w|W|N|TT.)|--(pp+mmm|op/o|L|w|W|N|TT.)|     (auto-generated)     |
  +---------------------+--------------------+---------+----+-+-+-+-+----+---------+----+-+-+-+-+----+----+----+------+----+---*/
  // ${InstInfo:Begin}
  INST(None             , None               , 0                         , 0                         , 0  , 0  , 0    , 0  , 0  ), // #0
  INST(Aaa              , X86Op_xAX          , O(000000,37,_,_,_,_,_,_  ), 0                         , 0  , 0  , 1    , 1  , 1  ), // #1
  INST(Aad              , X86I_xAX           , O(000000,D5,_,_,_,_,_,_  ), 0                         , 0  , 0  , 5    , 2  , 1  ), // #2
  INST(Aam              , X86I_xAX           , O(000000,D4,_,_,_,_,_,_  ), 0                         , 0  , 0  , 9    , 2  , 1  ), // #3
  INST(Aas              , X86Op_xAX          , O(000000,3F,_,_,_,_,_,_  ), 0                         , 0  , 0  , 13   , 1  , 1  ), // #4
  INST(Adc              , X86Arith           , O(000000,10,2,_,x,_,_,_  ), 0                         , 1  , 0  , 17   , 3  , 2  ), // #5
  INST(Adcx             , X86Rm              , O(660F38,F6,_,_,x,_,_,_  ), 0                         , 2  , 0  , 21   , 4  , 3  ), // #6
  INST(Add              , X86Arith           , O(000000,00,0,_,x,_,_,_  ), 0                         , 0  , 0  , 761  , 3  , 1  ), // #7
  INST(Addpd            , ExtRm              , O(660F00,58,_,_,_,_,_,_  ), 0                         , 3  , 0  , 4814 , 5  , 4  ), // #8
  INST(Addps            , ExtRm              , O(000F00,58,_,_,_,_,_,_  ), 0                         , 4  , 0  , 4826 , 5  , 5  ), // #9
  INST(Addsd            , ExtRm              , O(F20F00,58,_,_,_,_,_,_  ), 0                         , 5  , 0  , 5048 , 6  , 4  ), // #10
  INST(Addss            , ExtRm              , O(F30F00,58,_,_,_,_,_,_  ), 0                         , 6  , 0  , 2955 , 7  , 5  ), // #11
  INST(Addsubpd         , ExtRm              , O(660F00,D0,_,_,_,_,_,_  ), 0                         , 3  , 0  , 4553 , 5  , 6  ), // #12
  INST(Addsubps         , ExtRm              , O(F20F00,D0,_,_,_,_,_,_  ), 0                         , 5  , 0  , 4565 , 5  , 6  ), // #13
  INST(Adox             , X86Rm              , O(F30F38,F6,_,_,x,_,_,_  ), 0                         , 7  , 0  , 26   , 4  , 7  ), // #14
  INST(Aesdec           , ExtRm              , O(660F38,DE,_,_,_,_,_,_  ), 0                         , 2  , 0  , 3010 , 5  , 8  ), // #15
  INST(Aesdeclast       , ExtRm              , O(660F38,DF,_,_,_,_,_,_  ), 0                         , 2  , 0  , 3018 , 5  , 8  ), // #16
  INST(Aesenc           , ExtRm              , O(660F38,DC,_,_,_,_,_,_  ), 0                         , 2  , 0  , 3030 , 5  , 8  ), // #17
  INST(Aesenclast       , ExtRm              , O(660F38,DD,_,_,_,_,_,_  ), 0                         , 2  , 0  , 3038 , 5  , 8  ), // #18
  INST(Aesimc           , ExtRm              , O(660F38,DB,_,_,_,_,_,_  ), 0                         , 2  , 0  , 3050 , 5  , 8  ), // #19
  INST(Aeskeygenassist  , ExtRmi             , O(660F3A,DF,_,_,_,_,_,_  ), 0                         , 8  , 0  , 3058 , 8  , 8  ), // #20
  INST(And              , X86Arith           , O(000000,20,4,_,x,_,_,_  ), 0                         , 9  , 0  , 2433 , 9  , 1  ), // #21
  INST(Andn             , VexRvm_Wx          , V(000F38,F2,_,0,x,_,_,_  ), 0                         , 10 , 0  , 6494 , 10 , 9  ), // #22
  INST(Andnpd           , ExtRm              , O(660F00,55,_,_,_,_,_,_  ), 0                         , 3  , 0  , 3091 , 5  , 4  ), // #23
  INST(Andnps           , ExtRm              , O(000F00,55,_,_,_,_,_,_  ), 0                         , 4  , 0  , 3099 , 5  , 5  ), // #24
  INST(Andpd            , ExtRm              , O(660F00,54,_,_,_,_,_,_  ), 0                         , 3  , 0  , 4067 , 11 , 4  ), // #25
  INST(Andps            , ExtRm              , O(000F00,54,_,_,_,_,_,_  ), 0                         , 4  , 0  , 4077 , 11 , 5  ), // #26
  INST(Arpl             , X86Mr_NoSize       , O(000000,63,_,_,_,_,_,_  ), 0                         , 0  , 0  , 31   , 12 , 10 ), // #27
  INST(Bextr            , VexRmv_Wx          , V(000F38,F7,_,0,x,_,_,_  ), 0                         , 10 , 0  , 36   , 13 , 9  ), // #28
  INST(Blcfill          , VexVm_Wx           , V(XOP_M9,01,1,0,x,_,_,_  ), 0                         , 11 , 0  , 42   , 14 , 11 ), // #29
  INST(Blci             , VexVm_Wx           , V(XOP_M9,02,6,0,x,_,_,_  ), 0                         , 12 , 0  , 50   , 14 , 11 ), // #30
  INST(Blcic            , VexVm_Wx           , V(XOP_M9,01,5,0,x,_,_,_  ), 0                         , 13 , 0  , 55   , 14 , 11 ), // #31
  INST(Blcmsk           , VexVm_Wx           , V(XOP_M9,02,1,0,x,_,_,_  ), 0                         , 11 , 0  , 61   , 14 , 11 ), // #32
  INST(Blcs             , VexVm_Wx           , V(XOP_M9,01,3,0,x,_,_,_  ), 0                         , 14 , 0  , 68   , 14 , 11 ), // #33
  INST(Blendpd          , ExtRmi             , O(660F3A,0D,_,_,_,_,_,_  ), 0                         , 8  , 0  , 3177 , 8  , 12 ), // #34
  INST(Blendps          , ExtRmi             , O(660F3A,0C,_,_,_,_,_,_  ), 0                         , 8  , 0  , 3186 , 8  , 12 ), // #35
  INST(Blendvpd         , ExtRm_XMM0         , O(660F38,15,_,_,_,_,_,_  ), 0                         , 2  , 0  , 3195 , 15 , 12 ), // #36
  INST(Blendvps         , ExtRm_XMM0         , O(660F38,14,_,_,_,_,_,_  ), 0                         , 2  , 0  , 3205 , 15 , 12 ), // #37
  INST(Blsfill          , VexVm_Wx           , V(XOP_M9,01,2,0,x,_,_,_  ), 0                         , 15 , 0  , 73   , 14 , 11 ), // #38
  INST(Blsi             , VexVm_Wx           , V(000F38,F3,3,0,x,_,_,_  ), 0                         , 16 , 0  , 81   , 14 , 9  ), // #39
  INST(Blsic            , VexVm_Wx           , V(XOP_M9,01,6,0,x,_,_,_  ), 0                         , 12 , 0  , 86   , 14 , 11 ), // #40
  INST(Blsmsk           , VexVm_Wx           , V(000F38,F3,2,0,x,_,_,_  ), 0                         , 17 , 0  , 92   , 14 , 9  ), // #41
  INST(Blsr             , VexVm_Wx           , V(000F38,F3,1,0,x,_,_,_  ), 0                         , 18 , 0  , 99   , 14 , 9  ), // #42
  INST(Bndcl            , X86Rm              , O(F30F00,1A,_,_,_,_,_,_  ), 0                         , 6  , 0  , 104  , 16 , 13 ), // #43
  INST(Bndcn            , X86Rm              , O(F20F00,1B,_,_,_,_,_,_  ), 0                         , 5  , 0  , 110  , 16 , 13 ), // #44
  INST(Bndcu            , X86Rm              , O(F20F00,1A,_,_,_,_,_,_  ), 0                         , 5  , 0  , 116  , 16 , 13 ), // #45
  INST(Bndldx           , X86Rm              , O(000F00,1A,_,_,_,_,_,_  ), 0                         , 4  , 0  , 122  , 17 , 13 ), // #46
  INST(Bndmk            , X86Rm              , O(F30F00,1B,_,_,_,_,_,_  ), 0                         , 6  , 0  , 129  , 18 , 13 ), // #47
  INST(Bndmov           , X86Bndmov          , O(660F00,1A,_,_,_,_,_,_  ), O(660F00,1B,_,_,_,_,_,_  ), 3  , 1  , 135  , 19 , 13 ), // #48
  INST(Bndstx           , X86Mr              , O(000F00,1B,_,_,_,_,_,_  ), 0                         , 4  , 0  , 142  , 20 , 13 ), // #49
  INST(Bound            , X86Rm              , O(000000,62,_,_,_,_,_,_  ), 0                         , 0  , 0  , 149  , 21 , 0  ), // #50
  INST(Bsf              , X86Rm              , O(000F00,BC,_,_,x,_,_,_  ), 0                         , 4  , 0  , 155  , 22 , 1  ), // #51
  INST(Bsr              , X86Rm              , O(000F00,BD,_,_,x,_,_,_  ), 0                         , 4  , 0  , 159  , 22 , 1  ), // #52
  INST(Bswap            , X86Bswap           , O(000F00,C8,_,_,x,_,_,_  ), 0                         , 4  , 0  , 163  , 23 , 0  ), // #53
  INST(Bt               , X86Bt              , O(000F00,A3,_,_,x,_,_,_  ), O(000F00,BA,4,_,x,_,_,_  ), 4  , 2  , 169  , 24 , 14 ), // #54
  INST(Btc              , X86Bt              , O(000F00,BB,_,_,x,_,_,_  ), O(000F00,BA,7,_,x,_,_,_  ), 4  , 3  , 172  , 25 , 15 ), // #55
  INST(Btr              , X86Bt              , O(000F00,B3,_,_,x,_,_,_  ), O(000F00,BA,6,_,x,_,_,_  ), 4  , 4  , 176  , 25 , 15 ), // #56
  INST(Bts              , X86Bt              , O(000F00,AB,_,_,x,_,_,_  ), O(000F00,BA,5,_,x,_,_,_  ), 4  , 5  , 180  , 25 , 15 ), // #57
  INST(Bzhi             , VexRmv_Wx          , V(000F38,F5,_,0,x,_,_,_  ), 0                         , 10 , 0  , 184  , 13 , 16 ), // #58
  INST(Call             , X86Call            , O(000000,FF,2,_,_,_,_,_  ), 0                         , 1  , 0  , 2848 , 26 , 1  ), // #59
  INST(Cbw              , X86Op_xAX          , O(660000,98,_,_,_,_,_,_  ), 0                         , 19 , 0  , 189  , 27 , 0  ), // #60
  INST(Cdq              , X86Op_xDX_xAX      , O(000000,99,_,_,_,_,_,_  ), 0                         , 0  , 0  , 193  , 28 , 0  ), // #61
  INST(Cdqe             , X86Op_xAX          , O(000000,98,_,_,1,_,_,_  ), 0                         , 20 , 0  , 197  , 29 , 0  ), // #62
  INST(Clac             , X86Op              , O(000F01,CA,_,_,_,_,_,_  ), 0                         , 21 , 0  , 202  , 30 , 17 ), // #63
  INST(Clc              , X86Op              , O(000000,F8,_,_,_,_,_,_  ), 0                         , 0  , 0  , 207  , 30 , 18 ), // #64
  INST(Cld              , X86Op              , O(000000,FC,_,_,_,_,_,_  ), 0                         , 0  , 0  , 211  , 30 , 19 ), // #65
  INST(Cldemote         , X86M_Only          , O(000F00,1C,0,_,_,_,_,_  ), 0                         , 4  , 0  , 215  , 31 , 20 ), // #66
  INST(Clflush          , X86M_Only          , O(000F00,AE,7,_,_,_,_,_  ), 0                         , 22 , 0  , 224  , 31 , 21 ), // #67
  INST(Clflushopt       , X86M_Only          , O(660F00,AE,7,_,_,_,_,_  ), 0                         , 23 , 0  , 232  , 31 , 22 ), // #68
  INST(Clgi             , X86Op              , O(000F01,DD,_,_,_,_,_,_  ), 0                         , 21 , 0  , 243  , 30 , 23 ), // #69
  INST(Cli              , X86Op              , O(000000,FA,_,_,_,_,_,_  ), 0                         , 0  , 0  , 248  , 30 , 24 ), // #70
  INST(Clts             , X86Op              , O(000F00,06,_,_,_,_,_,_  ), 0                         , 4  , 0  , 252  , 30 , 0  ), // #71
  INST(Clwb             , X86M_Only          , O(660F00,AE,6,_,_,_,_,_  ), 0                         , 24 , 0  , 257  , 31 , 25 ), // #72
  INST(Clzero           , X86Op_MemZAX       , O(000F01,FC,_,_,_,_,_,_  ), 0                         , 21 , 0  , 262  , 32 , 26 ), // #73
  INST(Cmc              , X86Op              , O(000000,F5,_,_,_,_,_,_  ), 0                         , 0  , 0  , 269  , 30 , 27 ), // #74
  INST(Cmova            , X86Rm              , O(000F00,47,_,_,x,_,_,_  ), 0                         , 4  , 0  , 273  , 22 , 28 ), // #75
  INST(Cmovae           , X86Rm              , O(000F00,43,_,_,x,_,_,_  ), 0                         , 4  , 0  , 279  , 22 , 29 ), // #76
  INST(Cmovb            , X86Rm              , O(000F00,42,_,_,x,_,_,_  ), 0                         , 4  , 0  , 618  , 22 , 29 ), // #77
  INST(Cmovbe           , X86Rm              , O(000F00,46,_,_,x,_,_,_  ), 0                         , 4  , 0  , 625  , 22 , 28 ), // #78
  INST(Cmovc            , X86Rm              , O(000F00,42,_,_,x,_,_,_  ), 0                         , 4  , 0  , 286  , 22 , 29 ), // #79
  INST(Cmove            , X86Rm              , O(000F00,44,_,_,x,_,_,_  ), 0                         , 4  , 0  , 633  , 22 , 30 ), // #80
  INST(Cmovg            , X86Rm              , O(000F00,4F,_,_,x,_,_,_  ), 0                         , 4  , 0  , 292  , 22 , 31 ), // #81
  INST(Cmovge           , X86Rm              , O(000F00,4D,_,_,x,_,_,_  ), 0                         , 4  , 0  , 298  , 22 , 32 ), // #82
  INST(Cmovl            , X86Rm              , O(000F00,4C,_,_,x,_,_,_  ), 0                         , 4  , 0  , 305  , 22 , 32 ), // #83
  INST(Cmovle           , X86Rm              , O(000F00,4E,_,_,x,_,_,_  ), 0                         , 4  , 0  , 311  , 22 , 31 ), // #84
  INST(Cmovna           , X86Rm              , O(000F00,46,_,_,x,_,_,_  ), 0                         , 4  , 0  , 318  , 22 , 28 ), // #85
  INST(Cmovnae          , X86Rm              , O(000F00,42,_,_,x,_,_,_  ), 0                         , 4  , 0  , 325  , 22 , 29 ), // #86
  INST(Cmovnb           , X86Rm              , O(000F00,43,_,_,x,_,_,_  ), 0                         , 4  , 0  , 640  , 22 , 29 ), // #87
  INST(Cmovnbe          , X86Rm              , O(000F00,47,_,_,x,_,_,_  ), 0                         , 4  , 0  , 648  , 22 , 28 ), // #88
  INST(Cmovnc           , X86Rm              , O(000F00,43,_,_,x,_,_,_  ), 0                         , 4  , 0  , 333  , 22 , 29 ), // #89
  INST(Cmovne           , X86Rm              , O(000F00,45,_,_,x,_,_,_  ), 0                         , 4  , 0  , 657  , 22 , 30 ), // #90
  INST(Cmovng           , X86Rm              , O(000F00,4E,_,_,x,_,_,_  ), 0                         , 4  , 0  , 340  , 22 , 31 ), // #91
  INST(Cmovnge          , X86Rm              , O(000F00,4C,_,_,x,_,_,_  ), 0                         , 4  , 0  , 347  , 22 , 32 ), // #92
  INST(Cmovnl           , X86Rm              , O(000F00,4D,_,_,x,_,_,_  ), 0                         , 4  , 0  , 355  , 22 , 32 ), // #93
  INST(Cmovnle          , X86Rm              , O(000F00,4F,_,_,x,_,_,_  ), 0                         , 4  , 0  , 362  , 22 , 31 ), // #94
  INST(Cmovno           , X86Rm              , O(000F00,41,_,_,x,_,_,_  ), 0                         , 4  , 0  , 370  , 22 , 33 ), // #95
  INST(Cmovnp           , X86Rm              , O(000F00,4B,_,_,x,_,_,_  ), 0                         , 4  , 0  , 377  , 22 , 34 ), // #96
  INST(Cmovns           , X86Rm              , O(000F00,49,_,_,x,_,_,_  ), 0                         , 4  , 0  , 384  , 22 , 35 ), // #97
  INST(Cmovnz           , X86Rm              , O(000F00,45,_,_,x,_,_,_  ), 0                         , 4  , 0  , 391  , 22 , 30 ), // #98
  INST(Cmovo            , X86Rm              , O(000F00,40,_,_,x,_,_,_  ), 0                         , 4  , 0  , 398  , 22 , 33 ), // #99
  INST(Cmovp            , X86Rm              , O(000F00,4A,_,_,x,_,_,_  ), 0                         , 4  , 0  , 404  , 22 , 34 ), // #100
  INST(Cmovpe           , X86Rm              , O(000F00,4A,_,_,x,_,_,_  ), 0                         , 4  , 0  , 410  , 22 , 34 ), // #101
  INST(Cmovpo           , X86Rm              , O(000F00,4B,_,_,x,_,_,_  ), 0                         , 4  , 0  , 417  , 22 , 34 ), // #102
  INST(Cmovs            , X86Rm              , O(000F00,48,_,_,x,_,_,_  ), 0                         , 4  , 0  , 424  , 22 , 35 ), // #103
  INST(Cmovz            , X86Rm              , O(000F00,44,_,_,x,_,_,_  ), 0                         , 4  , 0  , 430  , 22 , 30 ), // #104
  INST(Cmp              , X86Arith           , O(000000,38,7,_,x,_,_,_  ), 0                         , 25 , 0  , 436  , 33 , 1  ), // #105
  INST(Cmppd            , ExtRmi             , O(660F00,C2,_,_,_,_,_,_  ), 0                         , 3  , 0  , 3431 , 8  , 4  ), // #106
  INST(Cmpps            , ExtRmi             , O(000F00,C2,_,_,_,_,_,_  ), 0                         , 4  , 0  , 3438 , 8  , 5  ), // #107
  INST(Cmps             , X86StrMm           , O(000000,A6,_,_,_,_,_,_  ), 0                         , 0  , 0  , 440  , 34 , 36 ), // #108
  INST(Cmpsd            , ExtRmi             , O(F20F00,C2,_,_,_,_,_,_  ), 0                         , 5  , 0  , 3445 , 35 , 4  ), // #109
  INST(Cmpss            , ExtRmi             , O(F30F00,C2,_,_,_,_,_,_  ), 0                         , 6  , 0  , 3452 , 36 , 5  ), // #110
  INST(Cmpxchg          , X86Cmpxchg         , O(000F00,B0,_,_,x,_,_,_  ), 0                         , 4  , 0  , 445  , 37 , 37 ), // #111
  INST(Cmpxchg16b       , X86Cmpxchg8b_16b   , O(000F00,C7,1,_,1,_,_,_  ), 0                         , 26 , 0  , 453  , 38 , 38 ), // #112
  INST(Cmpxchg8b        , X86Cmpxchg8b_16b   , O(000F00,C7,1,_,_,_,_,_  ), 0                         , 27 , 0  , 464  , 39 , 39 ), // #113
  INST(Comisd           , ExtRm              , O(660F00,2F,_,_,_,_,_,_  ), 0                         , 3  , 0  , 9930 , 6  , 40 ), // #114
  INST(Comiss           , ExtRm              , O(000F00,2F,_,_,_,_,_,_  ), 0                         , 4  , 0  , 9939 , 7  , 41 ), // #115
  INST(Cpuid            , X86Op              , O(000F00,A2,_,_,_,_,_,_  ), 0                         , 4  , 0  , 474  , 40 , 42 ), // #116
  INST(Cqo              , X86Op_xDX_xAX      , O(000000,99,_,_,1,_,_,_  ), 0                         , 20 , 0  , 480  , 41 , 0  ), // #117
  INST(Crc32            , X86Crc             , O(F20F38,F0,_,_,x,_,_,_  ), 0                         , 28 , 0  , 484  , 42 , 43 ), // #118
  INST(Cvtdq2pd         , ExtRm              , O(F30F00,E6,_,_,_,_,_,_  ), 0                         , 6  , 0  , 3499 , 6  , 4  ), // #119
  INST(Cvtdq2ps         , ExtRm              , O(000F00,5B,_,_,_,_,_,_  ), 0                         , 4  , 0  , 3509 , 5  , 4  ), // #120
  INST(Cvtpd2dq         , ExtRm              , O(F20F00,E6,_,_,_,_,_,_  ), 0                         , 5  , 0  , 3548 , 5  , 4  ), // #121
  INST(Cvtpd2pi         , ExtRm              , O(660F00,2D,_,_,_,_,_,_  ), 0                         , 3  , 0  , 490  , 43 , 4  ), // #122
  INST(Cvtpd2ps         , ExtRm              , O(660F00,5A,_,_,_,_,_,_  ), 0                         , 3  , 0  , 3558 , 5  , 4  ), // #123
  INST(Cvtpi2pd         , ExtRm              , O(660F00,2A,_,_,_,_,_,_  ), 0                         , 3  , 0  , 499  , 44 , 4  ), // #124
  INST(Cvtpi2ps         , ExtRm              , O(000F00,2A,_,_,_,_,_,_  ), 0                         , 4  , 0  , 508  , 44 , 5  ), // #125
  INST(Cvtps2dq         , ExtRm              , O(660F00,5B,_,_,_,_,_,_  ), 0                         , 3  , 0  , 3610 , 5  , 4  ), // #126
  INST(Cvtps2pd         , ExtRm              , O(000F00,5A,_,_,_,_,_,_  ), 0                         , 4  , 0  , 3620 , 6  , 4  ), // #127
  INST(Cvtps2pi         , ExtRm              , O(000F00,2D,_,_,_,_,_,_  ), 0                         , 4  , 0  , 517  , 45 , 5  ), // #128
  INST(Cvtsd2si         , ExtRm_Wx           , O(F20F00,2D,_,_,x,_,_,_  ), 0                         , 5  , 0  , 3692 , 46 , 4  ), // #129
  INST(Cvtsd2ss         , ExtRm              , O(F20F00,5A,_,_,_,_,_,_  ), 0                         , 5  , 0  , 3702 , 6  , 4  ), // #130
  INST(Cvtsi2sd         , ExtRm_Wx           , O(F20F00,2A,_,_,x,_,_,_  ), 0                         , 5  , 0  , 3723 , 47 , 4  ), // #131
  INST(Cvtsi2ss         , ExtRm_Wx           , O(F30F00,2A,_,_,x,_,_,_  ), 0                         , 6  , 0  , 3733 , 47 , 5  ), // #132
  INST(Cvtss2sd         , ExtRm              , O(F30F00,5A,_,_,_,_,_,_  ), 0                         , 6  , 0  , 3743 , 7  , 4  ), // #133
  INST(Cvtss2si         , ExtRm_Wx           , O(F30F00,2D,_,_,x,_,_,_  ), 0                         , 6  , 0  , 3753 , 48 , 5  ), // #134
  INST(Cvttpd2dq        , ExtRm              , O(660F00,E6,_,_,_,_,_,_  ), 0                         , 3  , 0  , 3774 , 5  , 4  ), // #135
  INST(Cvttpd2pi        , ExtRm              , O(660F00,2C,_,_,_,_,_,_  ), 0                         , 3  , 0  , 526  , 43 , 4  ), // #136
  INST(Cvttps2dq        , ExtRm              , O(F30F00,5B,_,_,_,_,_,_  ), 0                         , 6  , 0  , 3820 , 5  , 4  ), // #137
  INST(Cvttps2pi        , ExtRm              , O(000F00,2C,_,_,_,_,_,_  ), 0                         , 4  , 0  , 536  , 45 , 5  ), // #138
  INST(Cvttsd2si        , ExtRm_Wx           , O(F20F00,2C,_,_,x,_,_,_  ), 0                         , 5  , 0  , 3866 , 46 , 4  ), // #139
  INST(Cvttss2si        , ExtRm_Wx           , O(F30F00,2C,_,_,x,_,_,_  ), 0                         , 6  , 0  , 3889 , 48 , 5  ), // #140
  INST(Cwd              , X86Op_xDX_xAX      , O(660000,99,_,_,_,_,_,_  ), 0                         , 19 , 0  , 546  , 49 , 0  ), // #141
  INST(Cwde             , X86Op_xAX          , O(000000,98,_,_,_,_,_,_  ), 0                         , 0  , 0  , 550  , 50 , 0  ), // #142
  INST(Daa              , X86Op              , O(000000,27,_,_,_,_,_,_  ), 0                         , 0  , 0  , 555  , 1  , 1  ), // #143
  INST(Das              , X86Op              , O(000000,2F,_,_,_,_,_,_  ), 0                         , 0  , 0  , 559  , 1  , 1  ), // #144
  INST(Dec              , X86IncDec          , O(000000,FE,1,_,x,_,_,_  ), O(000000,48,_,_,x,_,_,_  ), 29 , 6  , 3013 , 51 , 44 ), // #145
  INST(Div              , X86M_GPB_MulDiv    , O(000000,F6,6,_,x,_,_,_  ), 0                         , 30 , 0  , 780  , 52 , 1  ), // #146
  INST(Divpd            , ExtRm              , O(660F00,5E,_,_,_,_,_,_  ), 0                         , 3  , 0  , 3988 , 5  , 4  ), // #147
  INST(Divps            , ExtRm              , O(000F00,5E,_,_,_,_,_,_  ), 0                         , 4  , 0  , 3995 , 5  , 5  ), // #148
  INST(Divsd            , ExtRm              , O(F20F00,5E,_,_,_,_,_,_  ), 0                         , 5  , 0  , 4002 , 6  , 4  ), // #149
  INST(Divss            , ExtRm              , O(F30F00,5E,_,_,_,_,_,_  ), 0                         , 6  , 0  , 4009 , 7  , 5  ), // #150
  INST(Dppd             , ExtRmi             , O(660F3A,41,_,_,_,_,_,_  ), 0                         , 8  , 0  , 4026 , 8  , 12 ), // #151
  INST(Dpps             , ExtRmi             , O(660F3A,40,_,_,_,_,_,_  ), 0                         , 8  , 0  , 4032 , 8  , 12 ), // #152
  INST(Emms             , X86Op              , O(000F00,77,_,_,_,_,_,_  ), 0                         , 4  , 0  , 748  , 53 , 45 ), // #153
  INST(Enqcmd           , X86EnqcmdMovdir64b , O(F20F38,F8,_,_,_,_,_,_  ), 0                         , 28 , 0  , 563  , 54 , 46 ), // #154
  INST(Enqcmds          , X86EnqcmdMovdir64b , O(F30F38,F8,_,_,_,_,_,_  ), 0                         , 7  , 0  , 570  , 54 , 46 ), // #155
  INST(Enter            , X86Enter           , O(000000,C8,_,_,_,_,_,_  ), 0                         , 0  , 0  , 2856 , 55 , 0  ), // #156
  INST(Extractps        , ExtExtract         , O(660F3A,17,_,_,_,_,_,_  ), 0                         , 8  , 0  , 4222 , 56 , 12 ), // #157
  INST(Extrq            , ExtExtrq           , O(660F00,79,_,_,_,_,_,_  ), O(660F00,78,0,_,_,_,_,_  ), 3  , 7  , 7290 , 57 , 47 ), // #158
  INST(F2xm1            , FpuOp              , O_FPU(00,D9F0,_)          , 0                         , 31 , 0  , 578  , 30 , 0  ), // #159
  INST(Fabs             , FpuOp              , O_FPU(00,D9E1,_)          , 0                         , 31 , 0  , 584  , 30 , 0  ), // #160
  INST(Fadd             , FpuArith           , O_FPU(00,C0C0,0)          , 0                         , 32 , 0  , 2067 , 58 , 0  ), // #161
  INST(Faddp            , FpuRDef            , O_FPU(00,DEC0,_)          , 0                         , 33 , 0  , 589  , 59 , 0  ), // #162
  INST(Fbld             , X86M_Only          , O_FPU(00,00DF,4)          , 0                         , 34 , 0  , 595  , 60 , 0  ), // #163
  INST(Fbstp            , X86M_Only          , O_FPU(00,00DF,6)          , 0                         , 35 , 0  , 600  , 60 , 0  ), // #164
  INST(Fchs             , FpuOp              , O_FPU(00,D9E0,_)          , 0                         , 31 , 0  , 606  , 30 , 0  ), // #165
  INST(Fclex            , FpuOp              , O_FPU(9B,DBE2,_)          , 0                         , 36 , 0  , 611  , 30 , 0  ), // #166
  INST(Fcmovb           , FpuR               , O_FPU(00,DAC0,_)          , 0                         , 37 , 0  , 617  , 61 , 29 ), // #167
  INST(Fcmovbe          , FpuR               , O_FPU(00,DAD0,_)          , 0                         , 37 , 0  , 624  , 61 , 28 ), // #168
  INST(Fcmove           , FpuR               , O_FPU(00,DAC8,_)          , 0                         , 37 , 0  , 632  , 61 , 30 ), // #169
  INST(Fcmovnb          , FpuR               , O_FPU(00,DBC0,_)          , 0                         , 38 , 0  , 639  , 61 , 29 ), // #170
  INST(Fcmovnbe         , FpuR               , O_FPU(00,DBD0,_)          , 0                         , 38 , 0  , 647  , 61 , 28 ), // #171
  INST(Fcmovne          , FpuR               , O_FPU(00,DBC8,_)          , 0                         , 38 , 0  , 656  , 61 , 30 ), // #172
  INST(Fcmovnu          , FpuR               , O_FPU(00,DBD8,_)          , 0                         , 38 , 0  , 664  , 61 , 34 ), // #173
  INST(Fcmovu           , FpuR               , O_FPU(00,DAD8,_)          , 0                         , 37 , 0  , 672  , 61 , 34 ), // #174
  INST(Fcom             , FpuCom             , O_FPU(00,D0D0,2)          , 0                         , 39 , 0  , 679  , 62 , 0  ), // #175
  INST(Fcomi            , FpuR               , O_FPU(00,DBF0,_)          , 0                         , 38 , 0  , 684  , 61 , 48 ), // #176
  INST(Fcomip           , FpuR               , O_FPU(00,DFF0,_)          , 0                         , 40 , 0  , 690  , 61 , 48 ), // #177
  INST(Fcomp            , FpuCom             , O_FPU(00,D8D8,3)          , 0                         , 41 , 0  , 697  , 62 , 0  ), // #178
  INST(Fcompp           , FpuOp              , O_FPU(00,DED9,_)          , 0                         , 33 , 0  , 703  , 30 , 0  ), // #179
  INST(Fcos             , FpuOp              , O_FPU(00,D9FF,_)          , 0                         , 31 , 0  , 710  , 30 , 0  ), // #180
  INST(Fdecstp          , FpuOp              , O_FPU(00,D9F6,_)          , 0                         , 31 , 0  , 715  , 30 , 0  ), // #181
  INST(Fdiv             , FpuArith           , O_FPU(00,F0F8,6)          , 0                         , 42 , 0  , 723  , 58 , 0  ), // #182
  INST(Fdivp            , FpuRDef            , O_FPU(00,DEF8,_)          , 0                         , 33 , 0  , 728  , 59 , 0  ), // #183
  INST(Fdivr            , FpuArith           , O_FPU(00,F8F0,7)          , 0                         , 43 , 0  , 734  , 58 , 0  ), // #184
  INST(Fdivrp           , FpuRDef            , O_FPU(00,DEF0,_)          , 0                         , 33 , 0  , 740  , 59 , 0  ), // #185
  INST(Femms            , X86Op              , O(000F00,0E,_,_,_,_,_,_  ), 0                         , 4  , 0  , 747  , 30 , 49 ), // #186
  INST(Ffree            , FpuR               , O_FPU(00,DDC0,_)          , 0                         , 44 , 0  , 753  , 61 , 0  ), // #187
  INST(Fiadd            , FpuM               , O_FPU(00,00DA,0)          , 0                         , 45 , 0  , 759  , 63 , 0  ), // #188
  INST(Ficom            , FpuM               , O_FPU(00,00DA,2)          , 0                         , 46 , 0  , 765  , 63 , 0  ), // #189
  INST(Ficomp           , FpuM               , O_FPU(00,00DA,3)          , 0                         , 47 , 0  , 771  , 63 , 0  ), // #190
  INST(Fidiv            , FpuM               , O_FPU(00,00DA,6)          , 0                         , 35 , 0  , 778  , 63 , 0  ), // #191
  INST(Fidivr           , FpuM               , O_FPU(00,00DA,7)          , 0                         , 48 , 0  , 784  , 63 , 0  ), // #192
  INST(Fild             , FpuM               , O_FPU(00,00DB,0)          , O_FPU(00,00DF,5)          , 45 , 8  , 791  , 64 , 0  ), // #193
  INST(Fimul            , FpuM               , O_FPU(00,00DA,1)          , 0                         , 49 , 0  , 796  , 63 , 0  ), // #194
  INST(Fincstp          , FpuOp              , O_FPU(00,D9F7,_)          , 0                         , 31 , 0  , 802  , 30 , 0  ), // #195
  INST(Finit            , FpuOp              , O_FPU(9B,DBE3,_)          , 0                         , 36 , 0  , 810  , 30 , 0  ), // #196
  INST(Fist             , FpuM               , O_FPU(00,00DB,2)          , 0                         , 46 , 0  , 816  , 63 , 0  ), // #197
  INST(Fistp            , FpuM               , O_FPU(00,00DB,3)          , O_FPU(00,00DF,7)          , 47 , 9  , 821  , 64 , 0  ), // #198
  INST(Fisttp           , FpuM               , O_FPU(00,00DB,1)          , O_FPU(00,00DD,1)          , 49 , 10 , 827  , 64 , 6  ), // #199
  INST(Fisub            , FpuM               , O_FPU(00,00DA,4)          , 0                         , 34 , 0  , 834  , 63 , 0  ), // #200
  INST(Fisubr           , FpuM               , O_FPU(00,00DA,5)          , 0                         , 50 , 0  , 840  , 63 , 0  ), // #201
  INST(Fld              , FpuFldFst          , O_FPU(00,00D9,0)          , O_FPU(00,00DB,5)          , 45 , 11 , 847  , 65 , 0  ), // #202
  INST(Fld1             , FpuOp              , O_FPU(00,D9E8,_)          , 0                         , 31 , 0  , 851  , 30 , 0  ), // #203
  INST(Fldcw            , X86M_Only          , O_FPU(00,00D9,5)          , 0                         , 50 , 0  , 856  , 66 , 0  ), // #204
  INST(Fldenv           , X86M_Only          , O_FPU(00,00D9,4)          , 0                         , 34 , 0  , 862  , 31 , 0  ), // #205
  INST(Fldl2e           , FpuOp              , O_FPU(00,D9EA,_)          , 0                         , 31 , 0  , 869  , 30 , 0  ), // #206
  INST(Fldl2t           , FpuOp              , O_FPU(00,D9E9,_)          , 0                         , 31 , 0  , 876  , 30 , 0  ), // #207
  INST(Fldlg2           , FpuOp              , O_FPU(00,D9EC,_)          , 0                         , 31 , 0  , 883  , 30 , 0  ), // #208
  INST(Fldln2           , FpuOp              , O_FPU(00,D9ED,_)          , 0                         , 31 , 0  , 890  , 30 , 0  ), // #209
  INST(Fldpi            , FpuOp              , O_FPU(00,D9EB,_)          , 0                         , 31 , 0  , 897  , 30 , 0  ), // #210
  INST(Fldz             , FpuOp              , O_FPU(00,D9EE,_)          , 0                         , 31 , 0  , 903  , 30 , 0  ), // #211
  INST(Fmul             , FpuArith           , O_FPU(00,C8C8,1)          , 0                         , 51 , 0  , 2109 , 58 , 0  ), // #212
  INST(Fmulp            , FpuRDef            , O_FPU(00,DEC8,_)          , 0                         , 33 , 0  , 908  , 59 , 0  ), // #213
  INST(Fnclex           , FpuOp              , O_FPU(00,DBE2,_)          , 0                         , 38 , 0  , 914  , 30 , 0  ), // #214
  INST(Fninit           , FpuOp              , O_FPU(00,DBE3,_)          , 0                         , 38 , 0  , 921  , 30 , 0  ), // #215
  INST(Fnop             , FpuOp              , O_FPU(00,D9D0,_)          , 0                         , 31 , 0  , 928  , 30 , 0  ), // #216
  INST(Fnsave           , X86M_Only          , O_FPU(00,00DD,6)          , 0                         , 35 , 0  , 933  , 31 , 0  ), // #217
  INST(Fnstcw           , X86M_Only          , O_FPU(00,00D9,7)          , 0                         , 48 , 0  , 940  , 66 , 0  ), // #218
  INST(Fnstenv          , X86M_Only          , O_FPU(00,00D9,6)          , 0                         , 35 , 0  , 947  , 31 , 0  ), // #219
  INST(Fnstsw           , FpuStsw            , O_FPU(00,00DD,7)          , O_FPU(00,DFE0,_)          , 48 , 12 , 955  , 67 , 0  ), // #220
  INST(Fpatan           , FpuOp              , O_FPU(00,D9F3,_)          , 0                         , 31 , 0  , 962  , 30 , 0  ), // #221
  INST(Fprem            , FpuOp              , O_FPU(00,D9F8,_)          , 0                         , 31 , 0  , 969  , 30 , 0  ), // #222
  INST(Fprem1           , FpuOp              , O_FPU(00,D9F5,_)          , 0                         , 31 , 0  , 975  , 30 , 0  ), // #223
  INST(Fptan            , FpuOp              , O_FPU(00,D9F2,_)          , 0                         , 31 , 0  , 982  , 30 , 0  ), // #224
  INST(Frndint          , FpuOp              , O_FPU(00,D9FC,_)          , 0                         , 31 , 0  , 988  , 30 , 0  ), // #225
  INST(Frstor           , X86M_Only          , O_FPU(00,00DD,4)          , 0                         , 34 , 0  , 996  , 31 , 0  ), // #226
  INST(Fsave            , X86M_Only          , O_FPU(9B,00DD,6)          , 0                         , 52 , 0  , 1003 , 31 , 0  ), // #227
  INST(Fscale           , FpuOp              , O_FPU(00,D9FD,_)          , 0                         , 31 , 0  , 1009 , 30 , 0  ), // #228
  INST(Fsin             , FpuOp              , O_FPU(00,D9FE,_)          , 0                         , 31 , 0  , 1016 , 30 , 0  ), // #229
  INST(Fsincos          , FpuOp              , O_FPU(00,D9FB,_)          , 0                         , 31 , 0  , 1021 , 30 , 0  ), // #230
  INST(Fsqrt            , FpuOp              , O_FPU(00,D9FA,_)          , 0                         , 31 , 0  , 1029 , 30 , 0  ), // #231
  INST(Fst              , FpuFldFst          , O_FPU(00,00D9,2)          , 0                         , 46 , 0  , 1035 , 68 , 0  ), // #232
  INST(Fstcw            , X86M_Only          , O_FPU(9B,00D9,7)          , 0                         , 53 , 0  , 1039 , 66 , 0  ), // #233
  INST(Fstenv           , X86M_Only          , O_FPU(9B,00D9,6)          , 0                         , 52 , 0  , 1045 , 31 , 0  ), // #234
  INST(Fstp             , FpuFldFst          , O_FPU(00,00D9,3)          , O(000000,DB,7,_,_,_,_,_  ), 47 , 13 , 1052 , 65 , 0  ), // #235
  INST(Fstsw            , FpuStsw            , O_FPU(9B,00DD,7)          , O_FPU(9B,DFE0,_)          , 53 , 14 , 1057 , 67 , 0  ), // #236
  INST(Fsub             , FpuArith           , O_FPU(00,E0E8,4)          , 0                         , 54 , 0  , 2187 , 58 , 0  ), // #237
  INST(Fsubp            , FpuRDef            , O_FPU(00,DEE8,_)          , 0                         , 33 , 0  , 1063 , 59 , 0  ), // #238
  INST(Fsubr            , FpuArith           , O_FPU(00,E8E0,5)          , 0                         , 55 , 0  , 2193 , 58 , 0  ), // #239
  INST(Fsubrp           , FpuRDef            , O_FPU(00,DEE0,_)          , 0                         , 33 , 0  , 1069 , 59 , 0  ), // #240
  INST(Ftst             , FpuOp              , O_FPU(00,D9E4,_)          , 0                         , 31 , 0  , 1076 , 30 , 0  ), // #241
  INST(Fucom            , FpuRDef            , O_FPU(00,DDE0,_)          , 0                         , 44 , 0  , 1081 , 59 , 0  ), // #242
  INST(Fucomi           , FpuR               , O_FPU(00,DBE8,_)          , 0                         , 38 , 0  , 1087 , 61 , 48 ), // #243
  INST(Fucomip          , FpuR               , O_FPU(00,DFE8,_)          , 0                         , 40 , 0  , 1094 , 61 , 48 ), // #244
  INST(Fucomp           , FpuRDef            , O_FPU(00,DDE8,_)          , 0                         , 44 , 0  , 1102 , 59 , 0  ), // #245
  INST(Fucompp          , FpuOp              , O_FPU(00,DAE9,_)          , 0                         , 37 , 0  , 1109 , 30 , 0  ), // #246
  INST(Fwait            , X86Op              , O_FPU(00,00DB,_)          , 0                         , 56 , 0  , 1117 , 30 , 0  ), // #247
  INST(Fxam             , FpuOp              , O_FPU(00,D9E5,_)          , 0                         , 31 , 0  , 1123 , 30 , 0  ), // #248
  INST(Fxch             , FpuR               , O_FPU(00,D9C8,_)          , 0                         , 31 , 0  , 1128 , 59 , 0  ), // #249
  INST(Fxrstor          , X86M_Only          , O(000F00,AE,1,_,_,_,_,_  ), 0                         , 27 , 0  , 1133 , 31 , 50 ), // #250
  INST(Fxrstor64        , X86M_Only          , O(000F00,AE,1,_,1,_,_,_  ), 0                         , 26 , 0  , 1141 , 69 , 50 ), // #251
  INST(Fxsave           , X86M_Only          , O(000F00,AE,0,_,_,_,_,_  ), 0                         , 4  , 0  , 1151 , 31 , 50 ), // #252
  INST(Fxsave64         , X86M_Only          , O(000F00,AE,0,_,1,_,_,_  ), 0                         , 57 , 0  , 1158 , 69 , 50 ), // #253
  INST(Fxtract          , FpuOp              , O_FPU(00,D9F4,_)          , 0                         , 31 , 0  , 1167 , 30 , 0  ), // #254
  INST(Fyl2x            , FpuOp              , O_FPU(00,D9F1,_)          , 0                         , 31 , 0  , 1175 , 30 , 0  ), // #255
  INST(Fyl2xp1          , FpuOp              , O_FPU(00,D9F9,_)          , 0                         , 31 , 0  , 1181 , 30 , 0  ), // #256
  INST(Getsec           , X86Op              , O(000F00,37,_,_,_,_,_,_  ), 0                         , 4  , 0  , 1189 , 30 , 51 ), // #257
  INST(Gf2p8affineinvqb , ExtRmi             , O(660F3A,CF,_,_,_,_,_,_  ), 0                         , 8  , 0  , 5577 , 8  , 52 ), // #258
  INST(Gf2p8affineqb    , ExtRmi             , O(660F3A,CE,_,_,_,_,_,_  ), 0                         , 8  , 0  , 5595 , 8  , 52 ), // #259
  INST(Gf2p8mulb        , ExtRm              , O(660F38,CF,_,_,_,_,_,_  ), 0                         , 2  , 0  , 5610 , 5  , 52 ), // #260
  INST(Haddpd           , ExtRm              , O(660F00,7C,_,_,_,_,_,_  ), 0                         , 3  , 0  , 5621 , 5  , 6  ), // #261
  INST(Haddps           , ExtRm              , O(F20F00,7C,_,_,_,_,_,_  ), 0                         , 5  , 0  , 5629 , 5  , 6  ), // #262
  INST(Hlt              , X86Op              , O(000000,F4,_,_,_,_,_,_  ), 0                         , 0  , 0  , 1196 , 30 , 0  ), // #263
  INST(Hsubpd           , ExtRm              , O(660F00,7D,_,_,_,_,_,_  ), 0                         , 3  , 0  , 5637 , 5  , 6  ), // #264
  INST(Hsubps           , ExtRm              , O(F20F00,7D,_,_,_,_,_,_  ), 0                         , 5  , 0  , 5645 , 5  , 6  ), // #265
  INST(Idiv             , X86M_GPB_MulDiv    , O(000000,F6,7,_,x,_,_,_  ), 0                         , 25 , 0  , 779  , 52 , 1  ), // #266
  INST(Imul             , X86Imul            , O(000000,F6,5,_,x,_,_,_  ), 0                         , 58 , 0  , 797  , 70 , 1  ), // #267
  INST(In               , X86In              , O(000000,EC,_,_,_,_,_,_  ), O(000000,E4,_,_,_,_,_,_  ), 0  , 15 , 10076, 71 , 0  ), // #268
  INST(Inc              , X86IncDec          , O(000000,FE,0,_,x,_,_,_  ), O(000000,40,_,_,x,_,_,_  ), 0  , 16 , 1200 , 51 , 44 ), // #269
  INST(Ins              , X86Ins             , O(000000,6C,_,_,_,_,_,_  ), 0                         , 0  , 0  , 1857 , 72 , 0  ), // #270
  INST(Insertps         , ExtRmi             , O(660F3A,21,_,_,_,_,_,_  ), 0                         , 8  , 0  , 5781 , 36 , 12 ), // #271
  INST(Insertq          , ExtInsertq         , O(F20F00,79,_,_,_,_,_,_  ), O(F20F00,78,_,_,_,_,_,_  ), 5  , 17 , 1204 , 73 , 47 ), // #272
  INST(Int              , X86Int             , O(000000,CD,_,_,_,_,_,_  ), 0                         , 0  , 0  , 992  , 74 , 0  ), // #273
  INST(Int3             , X86Op              , O(000000,CC,_,_,_,_,_,_  ), 0                         , 0  , 0  , 1212 , 30 , 0  ), // #274
  INST(Into             , X86Op              , O(000000,CE,_,_,_,_,_,_  ), 0                         , 0  , 0  , 1217 , 75 , 53 ), // #275
  INST(Invd             , X86Op              , O(000F00,08,_,_,_,_,_,_  ), 0                         , 4  , 0  , 10031, 30 , 42 ), // #276
  INST(Invept           , X86Rm_NoSize       , O(660F38,80,_,_,_,_,_,_  ), 0                         , 2  , 0  , 1222 , 76 , 54 ), // #277
  INST(Invlpg           , X86M_Only          , O(000F00,01,7,_,_,_,_,_  ), 0                         , 22 , 0  , 1229 , 31 , 42 ), // #278
  INST(Invlpga          , X86Op_xAddr        , O(000F01,DF,_,_,_,_,_,_  ), 0                         , 21 , 0  , 1236 , 77 , 23 ), // #279
  INST(Invpcid          , X86Rm_NoSize       , O(660F38,82,_,_,_,_,_,_  ), 0                         , 2  , 0  , 1244 , 76 , 42 ), // #280
  INST(Invvpid          , X86Rm_NoSize       , O(660F38,81,_,_,_,_,_,_  ), 0                         , 2  , 0  , 1252 , 76 , 54 ), // #281
  INST(Iret             , X86Op              , O(000000,CF,_,_,_,_,_,_  ), 0                         , 0  , 0  , 1260 , 78 , 1  ), // #282
  INST(Iretd            , X86Op              , O(000000,CF,_,_,_,_,_,_  ), 0                         , 0  , 0  , 1265 , 78 , 1  ), // #283
  INST(Iretq            , X86Op              , O(000000,CF,_,_,1,_,_,_  ), 0                         , 20 , 0  , 1271 , 79 , 1  ), // #284
  INST(Iretw            , X86Op              , O(660000,CF,_,_,_,_,_,_  ), 0                         , 19 , 0  , 1277 , 78 , 1  ), // #285
  INST(Ja               , X86Jcc             , O(000F00,87,_,_,_,_,_,_  ), O(000000,77,_,_,_,_,_,_  ), 4  , 18 , 1283 , 80 , 55 ), // #286
  INST(Jae              , X86Jcc             , O(000F00,83,_,_,_,_,_,_  ), O(000000,73,_,_,_,_,_,_  ), 4  , 19 , 1286 , 80 , 56 ), // #287
  INST(Jb               , X86Jcc             , O(000F00,82,_,_,_,_,_,_  ), O(000000,72,_,_,_,_,_,_  ), 4  , 20 , 1290 , 80 , 56 ), // #288
  INST(Jbe              , X86Jcc             , O(000F00,86,_,_,_,_,_,_  ), O(000000,76,_,_,_,_,_,_  ), 4  , 21 , 1293 , 80 , 55 ), // #289
  INST(Jc               , X86Jcc             , O(000F00,82,_,_,_,_,_,_  ), O(000000,72,_,_,_,_,_,_  ), 4  , 20 , 1297 , 80 , 56 ), // #290
  INST(Je               , X86Jcc             , O(000F00,84,_,_,_,_,_,_  ), O(000000,74,_,_,_,_,_,_  ), 4  , 22 , 1300 , 80 , 57 ), // #291
  INST(Jecxz            , X86JecxzLoop       , 0                         , O(000000,E3,_,_,_,_,_,_  ), 0  , 23 , 1303 , 81 , 0  ), // #292
  INST(Jg               , X86Jcc             , O(000F00,8F,_,_,_,_,_,_  ), O(000000,7F,_,_,_,_,_,_  ), 4  , 24 , 1309 , 80 , 58 ), // #293
  INST(Jge              , X86Jcc             , O(000F00,8D,_,_,_,_,_,_  ), O(000000,7D,_,_,_,_,_,_  ), 4  , 25 , 1312 , 80 , 59 ), // #294
  INST(Jl               , X86Jcc             , O(000F00,8C,_,_,_,_,_,_  ), O(000000,7C,_,_,_,_,_,_  ), 4  , 26 , 1316 , 80 , 59 ), // #295
  INST(Jle              , X86Jcc             , O(000F00,8E,_,_,_,_,_,_  ), O(000000,7E,_,_,_,_,_,_  ), 4  , 27 , 1319 , 80 , 58 ), // #296
  INST(Jmp              , X86Jmp             , O(000000,FF,4,_,_,_,_,_  ), O(000000,EB,_,_,_,_,_,_  ), 9  , 28 , 1323 , 82 , 0  ), // #297
  INST(Jna              , X86Jcc             , O(000F00,86,_,_,_,_,_,_  ), O(000000,76,_,_,_,_,_,_  ), 4  , 21 , 1327 , 80 , 55 ), // #298
  INST(Jnae             , X86Jcc             , O(000F00,82,_,_,_,_,_,_  ), O(000000,72,_,_,_,_,_,_  ), 4  , 20 , 1331 , 80 , 56 ), // #299
  INST(Jnb              , X86Jcc             , O(000F00,83,_,_,_,_,_,_  ), O(000000,73,_,_,_,_,_,_  ), 4  , 19 , 1336 , 80 , 56 ), // #300
  INST(Jnbe             , X86Jcc             , O(000F00,87,_,_,_,_,_,_  ), O(000000,77,_,_,_,_,_,_  ), 4  , 18 , 1340 , 80 , 55 ), // #301
  INST(Jnc              , X86Jcc             , O(000F00,83,_,_,_,_,_,_  ), O(000000,73,_,_,_,_,_,_  ), 4  , 19 , 1345 , 80 , 56 ), // #302
  INST(Jne              , X86Jcc             , O(000F00,85,_,_,_,_,_,_  ), O(000000,75,_,_,_,_,_,_  ), 4  , 29 , 1349 , 80 , 57 ), // #303
  INST(Jng              , X86Jcc             , O(000F00,8E,_,_,_,_,_,_  ), O(000000,7E,_,_,_,_,_,_  ), 4  , 27 , 1353 , 80 , 58 ), // #304
  INST(Jnge             , X86Jcc             , O(000F00,8C,_,_,_,_,_,_  ), O(000000,7C,_,_,_,_,_,_  ), 4  , 26 , 1357 , 80 , 59 ), // #305
  INST(Jnl              , X86Jcc             , O(000F00,8D,_,_,_,_,_,_  ), O(000000,7D,_,_,_,_,_,_  ), 4  , 25 , 1362 , 80 , 59 ), // #306
  INST(Jnle             , X86Jcc             , O(000F00,8F,_,_,_,_,_,_  ), O(000000,7F,_,_,_,_,_,_  ), 4  , 24 , 1366 , 80 , 58 ), // #307
  INST(Jno              , X86Jcc             , O(000F00,81,_,_,_,_,_,_  ), O(000000,71,_,_,_,_,_,_  ), 4  , 30 , 1371 , 80 , 53 ), // #308
  INST(Jnp              , X86Jcc             , O(000F00,8B,_,_,_,_,_,_  ), O(000000,7B,_,_,_,_,_,_  ), 4  , 31 , 1375 , 80 , 60 ), // #309
  INST(Jns              , X86Jcc             , O(000F00,89,_,_,_,_,_,_  ), O(000000,79,_,_,_,_,_,_  ), 4  , 32 , 1379 , 80 , 61 ), // #310
  INST(Jnz              , X86Jcc             , O(000F00,85,_,_,_,_,_,_  ), O(000000,75,_,_,_,_,_,_  ), 4  , 29 , 1383 , 80 , 57 ), // #311
  INST(Jo               , X86Jcc             , O(000F00,80,_,_,_,_,_,_  ), O(000000,70,_,_,_,_,_,_  ), 4  , 33 , 1387 , 80 , 53 ), // #312
  INST(Jp               , X86Jcc             , O(000F00,8A,_,_,_,_,_,_  ), O(000000,7A,_,_,_,_,_,_  ), 4  , 34 , 1390 , 80 , 60 ), // #313
  INST(Jpe              , X86Jcc             , O(000F00,8A,_,_,_,_,_,_  ), O(000000,7A,_,_,_,_,_,_  ), 4  , 34 , 1393 , 80 , 60 ), // #314
  INST(Jpo              , X86Jcc             , O(000F00,8B,_,_,_,_,_,_  ), O(000000,7B,_,_,_,_,_,_  ), 4  , 31 , 1397 , 80 , 60 ), // #315
  INST(Js               , X86Jcc             , O(000F00,88,_,_,_,_,_,_  ), O(000000,78,_,_,_,_,_,_  ), 4  , 35 , 1401 , 80 , 61 ), // #316
  INST(Jz               , X86Jcc             , O(000F00,84,_,_,_,_,_,_  ), O(000000,74,_,_,_,_,_,_  ), 4  , 22 , 1404 , 80 , 57 ), // #317
  INST(Kaddb            , VexRvm             , V(660F00,4A,_,1,0,_,_,_  ), 0                         , 59 , 0  , 1407 , 83 , 62 ), // #318
  INST(Kaddd            , VexRvm             , V(660F00,4A,_,1,1,_,_,_  ), 0                         , 60 , 0  , 1413 , 83 , 63 ), // #319
  INST(Kaddq            , VexRvm             , V(000F00,4A,_,1,1,_,_,_  ), 0                         , 61 , 0  , 1419 , 83 , 63 ), // #320
  INST(Kaddw            , VexRvm             , V(000F00,4A,_,1,0,_,_,_  ), 0                         , 62 , 0  , 1425 , 83 , 62 ), // #321
  INST(Kandb            , VexRvm             , V(660F00,41,_,1,0,_,_,_  ), 0                         , 59 , 0  , 1431 , 83 , 62 ), // #322
  INST(Kandd            , VexRvm             , V(660F00,41,_,1,1,_,_,_  ), 0                         , 60 , 0  , 1437 , 83 , 63 ), // #323
  INST(Kandnb           , VexRvm             , V(660F00,42,_,1,0,_,_,_  ), 0                         , 59 , 0  , 1443 , 83 , 62 ), // #324
  INST(Kandnd           , VexRvm             , V(660F00,42,_,1,1,_,_,_  ), 0                         , 60 , 0  , 1450 , 83 , 63 ), // #325
  INST(Kandnq           , VexRvm             , V(000F00,42,_,1,1,_,_,_  ), 0                         , 61 , 0  , 1457 , 83 , 63 ), // #326
  INST(Kandnw           , VexRvm             , V(000F00,42,_,1,0,_,_,_  ), 0                         , 62 , 0  , 1464 , 83 , 64 ), // #327
  INST(Kandq            , VexRvm             , V(000F00,41,_,1,1,_,_,_  ), 0                         , 61 , 0  , 1471 , 83 , 63 ), // #328
  INST(Kandw            , VexRvm             , V(000F00,41,_,1,0,_,_,_  ), 0                         , 62 , 0  , 1477 , 83 , 64 ), // #329
  INST(Kmovb            , VexKmov            , V(660F00,90,_,0,0,_,_,_  ), V(660F00,92,_,0,0,_,_,_  ), 63 , 36 , 1483 , 84 , 62 ), // #330
  INST(Kmovd            , VexKmov            , V(660F00,90,_,0,1,_,_,_  ), V(F20F00,92,_,0,0,_,_,_  ), 64 , 37 , 7770 , 85 , 63 ), // #331
  INST(Kmovq            , VexKmov            , V(000F00,90,_,0,1,_,_,_  ), V(F20F00,92,_,0,1,_,_,_  ), 65 , 38 , 7781 , 86 , 63 ), // #332
  INST(Kmovw            , VexKmov            , V(000F00,90,_,0,0,_,_,_  ), V(000F00,92,_,0,0,_,_,_  ), 66 , 39 , 1489 , 87 , 64 ), // #333
  INST(Knotb            , VexRm              , V(660F00,44,_,0,0,_,_,_  ), 0                         , 63 , 0  , 1495 , 88 , 62 ), // #334
  INST(Knotd            , VexRm              , V(660F00,44,_,0,1,_,_,_  ), 0                         , 64 , 0  , 1501 , 88 , 63 ), // #335
  INST(Knotq            , VexRm              , V(000F00,44,_,0,1,_,_,_  ), 0                         , 65 , 0  , 1507 , 88 , 63 ), // #336
  INST(Knotw            , VexRm              , V(000F00,44,_,0,0,_,_,_  ), 0                         , 66 , 0  , 1513 , 88 , 64 ), // #337
  INST(Korb             , VexRvm             , V(660F00,45,_,1,0,_,_,_  ), 0                         , 59 , 0  , 1519 , 83 , 62 ), // #338
  INST(Kord             , VexRvm             , V(660F00,45,_,1,1,_,_,_  ), 0                         , 60 , 0  , 1524 , 83 , 63 ), // #339
  INST(Korq             , VexRvm             , V(000F00,45,_,1,1,_,_,_  ), 0                         , 61 , 0  , 1529 , 83 , 63 ), // #340
  INST(Kortestb         , VexRm              , V(660F00,98,_,0,0,_,_,_  ), 0                         , 63 , 0  , 1534 , 88 , 65 ), // #341
  INST(Kortestd         , VexRm              , V(660F00,98,_,0,1,_,_,_  ), 0                         , 64 , 0  , 1543 , 88 , 66 ), // #342
  INST(Kortestq         , VexRm              , V(000F00,98,_,0,1,_,_,_  ), 0                         , 65 , 0  , 1552 , 88 , 66 ), // #343
  INST(Kortestw         , VexRm              , V(000F00,98,_,0,0,_,_,_  ), 0                         , 66 , 0  , 1561 , 88 , 67 ), // #344
  INST(Korw             , VexRvm             , V(000F00,45,_,1,0,_,_,_  ), 0                         , 62 , 0  , 1570 , 83 , 64 ), // #345
  INST(Kshiftlb         , VexRmi             , V(660F3A,32,_,0,0,_,_,_  ), 0                         , 67 , 0  , 1575 , 89 , 62 ), // #346
  INST(Kshiftld         , VexRmi             , V(660F3A,33,_,0,0,_,_,_  ), 0                         , 67 , 0  , 1584 , 89 , 63 ), // #347
  INST(Kshiftlq         , VexRmi             , V(660F3A,33,_,0,1,_,_,_  ), 0                         , 68 , 0  , 1593 , 89 , 63 ), // #348
  INST(Kshiftlw         , VexRmi             , V(660F3A,32,_,0,1,_,_,_  ), 0                         , 68 , 0  , 1602 , 89 , 64 ), // #349
  INST(Kshiftrb         , VexRmi             , V(660F3A,30,_,0,0,_,_,_  ), 0                         , 67 , 0  , 1611 , 89 , 62 ), // #350
  INST(Kshiftrd         , VexRmi             , V(660F3A,31,_,0,0,_,_,_  ), 0                         , 67 , 0  , 1620 , 89 , 63 ), // #351
  INST(Kshiftrq         , VexRmi             , V(660F3A,31,_,0,1,_,_,_  ), 0                         , 68 , 0  , 1629 , 89 , 63 ), // #352
  INST(Kshiftrw         , VexRmi             , V(660F3A,30,_,0,1,_,_,_  ), 0                         , 68 , 0  , 1638 , 89 , 64 ), // #353
  INST(Ktestb           , VexRm              , V(660F00,99,_,0,0,_,_,_  ), 0                         , 63 , 0  , 1647 , 88 , 65 ), // #354
  INST(Ktestd           , VexRm              , V(660F00,99,_,0,1,_,_,_  ), 0                         , 64 , 0  , 1654 , 88 , 66 ), // #355
  INST(Ktestq           , VexRm              , V(000F00,99,_,0,1,_,_,_  ), 0                         , 65 , 0  , 1661 , 88 , 66 ), // #356
  INST(Ktestw           , VexRm              , V(000F00,99,_,0,0,_,_,_  ), 0                         , 66 , 0  , 1668 , 88 , 65 ), // #357
  INST(Kunpckbw         , VexRvm             , V(660F00,4B,_,1,0,_,_,_  ), 0                         , 59 , 0  , 1675 , 83 , 64 ), // #358
  INST(Kunpckdq         , VexRvm             , V(000F00,4B,_,1,1,_,_,_  ), 0                         , 61 , 0  , 1684 , 83 , 63 ), // #359
  INST(Kunpckwd         , VexRvm             , V(000F00,4B,_,1,0,_,_,_  ), 0                         , 62 , 0  , 1693 , 83 , 63 ), // #360
  INST(Kxnorb           , VexRvm             , V(660F00,46,_,1,0,_,_,_  ), 0                         , 59 , 0  , 1702 , 83 , 62 ), // #361
  INST(Kxnord           , VexRvm             , V(660F00,46,_,1,1,_,_,_  ), 0                         , 60 , 0  , 1709 , 83 , 63 ), // #362
  INST(Kxnorq           , VexRvm             , V(000F00,46,_,1,1,_,_,_  ), 0                         , 61 , 0  , 1716 , 83 , 63 ), // #363
  INST(Kxnorw           , VexRvm             , V(000F00,46,_,1,0,_,_,_  ), 0                         , 62 , 0  , 1723 , 83 , 64 ), // #364
  INST(Kxorb            , VexRvm             , V(660F00,47,_,1,0,_,_,_  ), 0                         , 59 , 0  , 1730 , 83 , 62 ), // #365
  INST(Kxord            , VexRvm             , V(660F00,47,_,1,1,_,_,_  ), 0                         , 60 , 0  , 1736 , 83 , 63 ), // #366
  INST(Kxorq            , VexRvm             , V(000F00,47,_,1,1,_,_,_  ), 0                         , 61 , 0  , 1742 , 83 , 63 ), // #367
  INST(Kxorw            , VexRvm             , V(000F00,47,_,1,0,_,_,_  ), 0                         , 62 , 0  , 1748 , 83 , 64 ), // #368
  INST(Lahf             , X86Op              , O(000000,9F,_,_,_,_,_,_  ), 0                         , 0  , 0  , 1754 , 90 , 68 ), // #369
  INST(Lar              , X86Rm              , O(000F00,02,_,_,_,_,_,_  ), 0                         , 4  , 0  , 1759 , 91 , 10 ), // #370
  INST(Lddqu            , ExtRm              , O(F20F00,F0,_,_,_,_,_,_  ), 0                         , 5  , 0  , 5791 , 92 , 6  ), // #371
  INST(Ldmxcsr          , X86M_Only          , O(000F00,AE,2,_,_,_,_,_  ), 0                         , 69 , 0  , 5798 , 93 , 5  ), // #372
  INST(Lds              , X86Rm              , O(000000,C5,_,_,_,_,_,_  ), 0                         , 0  , 0  , 1763 , 94 , 0  ), // #373
  INST(Lea              , X86Lea             , O(000000,8D,_,_,x,_,_,_  ), 0                         , 0  , 0  , 1767 , 95 , 0  ), // #374
  INST(Leave            , X86Op              , O(000000,C9,_,_,_,_,_,_  ), 0                         , 0  , 0  , 1771 , 30 , 0  ), // #375
  INST(Les              , X86Rm              , O(000000,C4,_,_,_,_,_,_  ), 0                         , 0  , 0  , 1777 , 94 , 0  ), // #376
  INST(Lfence           , X86Fence           , O(000F00,AE,5,_,_,_,_,_  ), 0                         , 70 , 0  , 1781 , 30 , 4  ), // #377
  INST(Lfs              , X86Rm              , O(000F00,B4,_,_,_,_,_,_  ), 0                         , 4  , 0  , 1788 , 96 , 0  ), // #378
  INST(Lgdt             , X86M_Only          , O(000F00,01,2,_,_,_,_,_  ), 0                         , 69 , 0  , 1792 , 31 , 0  ), // #379
  INST(Lgs              , X86Rm              , O(000F00,B5,_,_,_,_,_,_  ), 0                         , 4  , 0  , 1797 , 96 , 0  ), // #380
  INST(Lidt             , X86M_Only          , O(000F00,01,3,_,_,_,_,_  ), 0                         , 71 , 0  , 1801 , 31 , 0  ), // #381
  INST(Lldt             , X86M_NoSize        , O(000F00,00,2,_,_,_,_,_  ), 0                         , 69 , 0  , 1806 , 97 , 0  ), // #382
  INST(Llwpcb           , VexR_Wx            , V(XOP_M9,12,0,0,x,_,_,_  ), 0                         , 72 , 0  , 1811 , 98 , 69 ), // #383
  INST(Lmsw             , X86M_NoSize        , O(000F00,01,6,_,_,_,_,_  ), 0                         , 73 , 0  , 1818 , 97 , 0  ), // #384
  INST(Lods             , X86StrRm           , O(000000,AC,_,_,_,_,_,_  ), 0                         , 0  , 0  , 1823 , 99 , 0  ), // #385
  INST(Loop             , X86JecxzLoop       , 0                         , O(000000,E2,_,_,_,_,_,_  ), 0  , 40 , 1828 , 81 , 0  ), // #386
  INST(Loope            , X86JecxzLoop       , 0                         , O(000000,E1,_,_,_,_,_,_  ), 0  , 41 , 1833 , 81 , 57 ), // #387
  INST(Loopne           , X86JecxzLoop       , 0                         , O(000000,E0,_,_,_,_,_,_  ), 0  , 42 , 1839 , 81 , 57 ), // #388
  INST(Lsl              , X86Rm              , O(000F00,03,_,_,_,_,_,_  ), 0                         , 4  , 0  , 1846 , 100, 10 ), // #389
  INST(Lss              , X86Rm              , O(000F00,B2,_,_,_,_,_,_  ), 0                         , 4  , 0  , 6289 , 96 , 0  ), // #390
  INST(Ltr              , X86M_NoSize        , O(000F00,00,3,_,_,_,_,_  ), 0                         , 71 , 0  , 1850 , 97 , 0  ), // #391
  INST(Lwpins           , VexVmi4_Wx         , V(XOP_MA,12,0,0,x,_,_,_  ), 0                         , 74 , 0  , 1854 , 101, 69 ), // #392
  INST(Lwpval           , VexVmi4_Wx         , V(XOP_MA,12,1,0,x,_,_,_  ), 0                         , 75 , 0  , 1861 , 101, 69 ), // #393
  INST(Lzcnt            , X86Rm_Raw66H       , O(F30F00,BD,_,_,x,_,_,_  ), 0                         , 6  , 0  , 1868 , 22 , 70 ), // #394
  INST(Maskmovdqu       , ExtRm_ZDI          , O(660F00,57,_,_,_,_,_,_  ), 0                         , 3  , 0  , 5807 , 102, 4  ), // #395
  INST(Maskmovq         , ExtRm_ZDI          , O(000F00,F7,_,_,_,_,_,_  ), 0                         , 4  , 0  , 7778 , 103, 71 ), // #396
  INST(Maxpd            , ExtRm              , O(660F00,5F,_,_,_,_,_,_  ), 0                         , 3  , 0  , 5841 , 5  , 4  ), // #397
  INST(Maxps            , ExtRm              , O(000F00,5F,_,_,_,_,_,_  ), 0                         , 4  , 0  , 5848 , 5  , 5  ), // #398
  INST(Maxsd            , ExtRm              , O(F20F00,5F,_,_,_,_,_,_  ), 0                         , 5  , 0  , 7797 , 6  , 4  ), // #399
  INST(Maxss            , ExtRm              , O(F30F00,5F,_,_,_,_,_,_  ), 0                         , 6  , 0  , 5862 , 7  , 5  ), // #400
  INST(Mfence           , X86Fence           , O(000F00,AE,6,_,_,_,_,_  ), 0                         , 73 , 0  , 1874 , 30 , 4  ), // #401
  INST(Minpd            , ExtRm              , O(660F00,5D,_,_,_,_,_,_  ), 0                         , 3  , 0  , 5891 , 5  , 4  ), // #402
  INST(Minps            , ExtRm              , O(000F00,5D,_,_,_,_,_,_  ), 0                         , 4  , 0  , 5898 , 5  , 5  ), // #403
  INST(Minsd            , ExtRm              , O(F20F00,5D,_,_,_,_,_,_  ), 0                         , 5  , 0  , 7861 , 6  , 4  ), // #404
  INST(Minss            , ExtRm              , O(F30F00,5D,_,_,_,_,_,_  ), 0                         , 6  , 0  , 5912 , 7  , 5  ), // #405
  INST(Monitor          , X86Op              , O(000F01,C8,_,_,_,_,_,_  ), 0                         , 21 , 0  , 1881 , 104, 72 ), // #406
  INST(Monitorx         , X86Op              , O(000F01,FA,_,_,_,_,_,_  ), 0                         , 21 , 0  , 1889 , 104, 73 ), // #407
  INST(Mov              , X86Mov             , 0                         , 0                         , 0  , 0  , 138  , 105, 0  ), // #408
  INST(Movapd           , ExtMov             , O(660F00,28,_,_,_,_,_,_  ), O(660F00,29,_,_,_,_,_,_  ), 3  , 43 , 5943 , 106, 4  ), // #409
  INST(Movaps           , ExtMov             , O(000F00,28,_,_,_,_,_,_  ), O(000F00,29,_,_,_,_,_,_  ), 4  , 44 , 5951 , 106, 5  ), // #410
  INST(Movbe            , ExtMovbe           , O(000F38,F0,_,_,x,_,_,_  ), O(000F38,F1,_,_,x,_,_,_  ), 76 , 45 , 626  , 107, 74 ), // #411
  INST(Movd             , ExtMovd            , O(000F00,6E,_,_,_,_,_,_  ), O(000F00,7E,_,_,_,_,_,_  ), 4  , 46 , 7771 , 108, 75 ), // #412
  INST(Movddup          , ExtMov             , O(F20F00,12,_,_,_,_,_,_  ), 0                         , 5  , 0  , 5965 , 6  , 6  ), // #413
  INST(Movdir64b        , X86EnqcmdMovdir64b , O(660F38,F8,_,_,_,_,_,_  ), 0                         , 2  , 0  , 1898 , 109, 76 ), // #414
  INST(Movdiri          , X86MovntiMovdiri   , O(000F38,F9,_,_,_,_,_,_  ), 0                         , 76 , 0  , 1908 , 110, 77 ), // #415
  INST(Movdq2q          , ExtMov             , O(F20F00,D6,_,_,_,_,_,_  ), 0                         , 5  , 0  , 1916 , 111, 4  ), // #416
  INST(Movdqa           , ExtMov             , O(660F00,6F,_,_,_,_,_,_  ), O(660F00,7F,_,_,_,_,_,_  ), 3  , 47 , 5974 , 106, 4  ), // #417
  INST(Movdqu           , ExtMov             , O(F30F00,6F,_,_,_,_,_,_  ), O(F30F00,7F,_,_,_,_,_,_  ), 6  , 48 , 5811 , 106, 4  ), // #418
  INST(Movhlps          , ExtMov             , O(000F00,12,_,_,_,_,_,_  ), 0                         , 4  , 0  , 6049 , 112, 5  ), // #419
  INST(Movhpd           , ExtMov             , O(660F00,16,_,_,_,_,_,_  ), O(660F00,17,_,_,_,_,_,_  ), 3  , 49 , 6058 , 113, 4  ), // #420
  INST(Movhps           , ExtMov             , O(000F00,16,_,_,_,_,_,_  ), O(000F00,17,_,_,_,_,_,_  ), 4  , 50 , 6066 , 113, 5  ), // #421
  INST(Movlhps          , ExtMov             , O(000F00,16,_,_,_,_,_,_  ), 0                         , 4  , 0  , 6074 , 112, 5  ), // #422
  INST(Movlpd           , ExtMov             , O(660F00,12,_,_,_,_,_,_  ), O(660F00,13,_,_,_,_,_,_  ), 3  , 51 , 6083 , 113, 4  ), // #423
  INST(Movlps           , ExtMov             , O(000F00,12,_,_,_,_,_,_  ), O(000F00,13,_,_,_,_,_,_  ), 4  , 52 , 6091 , 113, 5  ), // #424
  INST(Movmskpd         , ExtMov             , O(660F00,50,_,_,_,_,_,_  ), 0                         , 3  , 0  , 6099 , 114, 4  ), // #425
  INST(Movmskps         , ExtMov             , O(000F00,50,_,_,_,_,_,_  ), 0                         , 4  , 0  , 6109 , 114, 5  ), // #426
  INST(Movntdq          , ExtMov             , 0                         , O(660F00,E7,_,_,_,_,_,_  ), 0  , 53 , 6119 , 115, 4  ), // #427
  INST(Movntdqa         , ExtMov             , O(660F38,2A,_,_,_,_,_,_  ), 0                         , 2  , 0  , 6128 , 92 , 12 ), // #428
  INST(Movnti           , X86MovntiMovdiri   , O(000F00,C3,_,_,x,_,_,_  ), 0                         , 4  , 0  , 1924 , 110, 4  ), // #429
  INST(Movntpd          , ExtMov             , 0                         , O(660F00,2B,_,_,_,_,_,_  ), 0  , 54 , 6138 , 115, 4  ), // #430
  INST(Movntps          , ExtMov             , 0                         , O(000F00,2B,_,_,_,_,_,_  ), 0  , 55 , 6147 , 115, 5  ), // #431
  INST(Movntq           , ExtMov             , 0                         , O(000F00,E7,_,_,_,_,_,_  ), 0  , 56 , 1931 , 116, 71 ), // #432
  INST(Movntsd          , ExtMov             , 0                         , O(F20F00,2B,_,_,_,_,_,_  ), 0  , 57 , 1938 , 117, 47 ), // #433
  INST(Movntss          , ExtMov             , 0                         , O(F30F00,2B,_,_,_,_,_,_  ), 0  , 58 , 1946 , 118, 47 ), // #434
  INST(Movq             , ExtMovq            , O(000F00,6E,_,_,x,_,_,_  ), O(000F00,7E,_,_,x,_,_,_  ), 4  , 59 , 7782 , 119, 75 ), // #435
  INST(Movq2dq          , ExtRm              , O(F30F00,D6,_,_,_,_,_,_  ), 0                         , 6  , 0  , 1954 , 120, 4  ), // #436
  INST(Movs             , X86StrMm           , O(000000,A4,_,_,_,_,_,_  ), 0                         , 0  , 0  , 425  , 121, 0  ), // #437
  INST(Movsd            , ExtMov             , O(F20F00,10,_,_,_,_,_,_  ), O(F20F00,11,_,_,_,_,_,_  ), 5  , 60 , 6162 , 122, 4  ), // #438
  INST(Movshdup         , ExtRm              , O(F30F00,16,_,_,_,_,_,_  ), 0                         , 6  , 0  , 6169 , 5  , 6  ), // #439
  INST(Movsldup         , ExtRm              , O(F30F00,12,_,_,_,_,_,_  ), 0                         , 6  , 0  , 6179 , 5  , 6  ), // #440
  INST(Movss            , ExtMov             , O(F30F00,10,_,_,_,_,_,_  ), O(F30F00,11,_,_,_,_,_,_  ), 6  , 61 , 6189 , 123, 5  ), // #441
  INST(Movsx            , X86MovsxMovzx      , O(000F00,BE,_,_,x,_,_,_  ), 0                         , 4  , 0  , 1962 , 124, 0  ), // #442
  INST(Movsxd           , X86Rm              , O(000000,63,_,_,1,_,_,_  ), 0                         , 20 , 0  , 1968 , 125, 0  ), // #443
  INST(Movupd           , ExtMov             , O(660F00,10,_,_,_,_,_,_  ), O(660F00,11,_,_,_,_,_,_  ), 3  , 62 , 6196 , 106, 4  ), // #444
  INST(Movups           , ExtMov             , O(000F00,10,_,_,_,_,_,_  ), O(000F00,11,_,_,_,_,_,_  ), 4  , 63 , 6204 , 106, 5  ), // #445
  INST(Movzx            , X86MovsxMovzx      , O(000F00,B6,_,_,x,_,_,_  ), 0                         , 4  , 0  , 1975 , 124, 0  ), // #446
  INST(Mpsadbw          , ExtRmi             , O(660F3A,42,_,_,_,_,_,_  ), 0                         , 8  , 0  , 6212 , 8  , 12 ), // #447
  INST(Mul              , X86M_GPB_MulDiv    , O(000000,F6,4,_,x,_,_,_  ), 0                         , 9  , 0  , 798  , 52 , 1  ), // #448
  INST(Mulpd            , ExtRm              , O(660F00,59,_,_,_,_,_,_  ), 0                         , 3  , 0  , 6266 , 5  , 4  ), // #449
  INST(Mulps            , ExtRm              , O(000F00,59,_,_,_,_,_,_  ), 0                         , 4  , 0  , 6273 , 5  , 5  ), // #450
  INST(Mulsd            , ExtRm              , O(F20F00,59,_,_,_,_,_,_  ), 0                         , 5  , 0  , 6280 , 6  , 4  ), // #451
  INST(Mulss            , ExtRm              , O(F30F00,59,_,_,_,_,_,_  ), 0                         , 6  , 0  , 6287 , 7  , 5  ), // #452
  INST(Mulx             , VexRvm_ZDX_Wx      , V(F20F38,F6,_,0,x,_,_,_  ), 0                         , 77 , 0  , 1981 , 126, 78 ), // #453
  INST(Mwait            , X86Op              , O(000F01,C9,_,_,_,_,_,_  ), 0                         , 21 , 0  , 1986 , 127, 72 ), // #454
  INST(Mwaitx           , X86Op              , O(000F01,FB,_,_,_,_,_,_  ), 0                         , 21 , 0  , 1992 , 128, 73 ), // #455
  INST(Neg              , X86M_GPB           , O(000000,F6,3,_,x,_,_,_  ), 0                         , 78 , 0  , 1999 , 129, 79 ), // #456
  INST(Nop              , X86M_Nop           , O(000000,90,_,_,_,_,_,_  ), 0                         , 0  , 0  , 929  , 130, 0  ), // #457
  INST(Not              , X86M_GPB           , O(000000,F6,2,_,x,_,_,_  ), 0                         , 1  , 0  , 2003 , 129, 0  ), // #458
  INST(Or               , X86Arith           , O(000000,08,1,_,x,_,_,_  ), 0                         , 29 , 0  , 1138 , 131, 1  ), // #459
  INST(Orpd             , ExtRm              , O(660F00,56,_,_,_,_,_,_  ), 0                         , 3  , 0  , 9988 , 11 , 4  ), // #460
  INST(Orps             , ExtRm              , O(000F00,56,_,_,_,_,_,_  ), 0                         , 4  , 0  , 9995 , 11 , 5  ), // #461
  INST(Out              , X86Out             , O(000000,EE,_,_,_,_,_,_  ), O(000000,E6,_,_,_,_,_,_  ), 0  , 64 , 2007 , 132, 0  ), // #462
  INST(Outs             , X86Outs            , O(000000,6E,_,_,_,_,_,_  ), 0                         , 0  , 0  , 2011 , 133, 0  ), // #463
  INST(Pabsb            , ExtRm_P            , O(000F38,1C,_,_,_,_,_,_  ), 0                         , 76 , 0  , 6341 , 134, 80 ), // #464
  INST(Pabsd            , ExtRm_P            , O(000F38,1E,_,_,_,_,_,_  ), 0                         , 76 , 0  , 6348 , 134, 80 ), // #465
  INST(Pabsw            , ExtRm_P            , O(000F38,1D,_,_,_,_,_,_  ), 0                         , 76 , 0  , 6362 , 134, 80 ), // #466
  INST(Packssdw         , ExtRm_P            , O(000F00,6B,_,_,_,_,_,_  ), 0                         , 4  , 0  , 6369 , 134, 75 ), // #467
  INST(Packsswb         , ExtRm_P            , O(000F00,63,_,_,_,_,_,_  ), 0                         , 4  , 0  , 6379 , 134, 75 ), // #468
  INST(Packusdw         , ExtRm              , O(660F38,2B,_,_,_,_,_,_  ), 0                         , 2  , 0  , 6389 , 5  , 12 ), // #469
  INST(Packuswb         , ExtRm_P            , O(000F00,67,_,_,_,_,_,_  ), 0                         , 4  , 0  , 6399 , 134, 75 ), // #470
  INST(Paddb            , ExtRm_P            , O(000F00,FC,_,_,_,_,_,_  ), 0                         , 4  , 0  , 6409 , 134, 75 ), // #471
  INST(Paddd            , ExtRm_P            , O(000F00,FE,_,_,_,_,_,_  ), 0                         , 4  , 0  , 6416 , 134, 75 ), // #472
  INST(Paddq            , ExtRm_P            , O(000F00,D4,_,_,_,_,_,_  ), 0                         , 4  , 0  , 6423 , 134, 4  ), // #473
  INST(Paddsb           , ExtRm_P            , O(000F00,EC,_,_,_,_,_,_  ), 0                         , 4  , 0  , 6430 , 134, 75 ), // #474
  INST(Paddsw           , ExtRm_P            , O(000F00,ED,_,_,_,_,_,_  ), 0                         , 4  , 0  , 6438 , 134, 75 ), // #475
  INST(Paddusb          , ExtRm_P            , O(000F00,DC,_,_,_,_,_,_  ), 0                         , 4  , 0  , 6446 , 134, 75 ), // #476
  INST(Paddusw          , ExtRm_P            , O(000F00,DD,_,_,_,_,_,_  ), 0                         , 4  , 0  , 6455 , 134, 75 ), // #477
  INST(Paddw            , ExtRm_P            , O(000F00,FD,_,_,_,_,_,_  ), 0                         , 4  , 0  , 6464 , 134, 75 ), // #478
  INST(Palignr          , ExtRmi_P           , O(000F3A,0F,_,_,_,_,_,_  ), 0                         , 79 , 0  , 6471 , 135, 6  ), // #479
  INST(Pand             , ExtRm_P            , O(000F00,DB,_,_,_,_,_,_  ), 0                         , 4  , 0  , 6480 , 136, 75 ), // #480
  INST(Pandn            , ExtRm_P            , O(000F00,DF,_,_,_,_,_,_  ), 0                         , 4  , 0  , 6493 , 137, 75 ), // #481
  INST(Pause            , X86Op              , O(F30000,90,_,_,_,_,_,_  ), 0                         , 80 , 0  , 2016 , 30 , 0  ), // #482
  INST(Pavgb            , ExtRm_P            , O(000F00,E0,_,_,_,_,_,_  ), 0                         , 4  , 0  , 6523 , 134, 81 ), // #483
  INST(Pavgusb          , Ext3dNow           , O(000F0F,BF,_,_,_,_,_,_  ), 0                         , 81 , 0  , 2022 , 138, 49 ), // #484
  INST(Pavgw            , ExtRm_P            , O(000F00,E3,_,_,_,_,_,_  ), 0                         , 4  , 0  , 6530 , 134, 81 ), // #485
  INST(Pblendvb         , ExtRm_XMM0         , O(660F38,10,_,_,_,_,_,_  ), 0                         , 2  , 0  , 6546 , 15 , 12 ), // #486
  INST(Pblendw          , ExtRmi             , O(660F3A,0E,_,_,_,_,_,_  ), 0                         , 8  , 0  , 6556 , 8  , 12 ), // #487
  INST(Pclmulqdq        , ExtRmi             , O(660F3A,44,_,_,_,_,_,_  ), 0                         , 8  , 0  , 6649 , 8  , 82 ), // #488
  INST(Pcmpeqb          , ExtRm_P            , O(000F00,74,_,_,_,_,_,_  ), 0                         , 4  , 0  , 6681 , 137, 75 ), // #489
  INST(Pcmpeqd          , ExtRm_P            , O(000F00,76,_,_,_,_,_,_  ), 0                         , 4  , 0  , 6690 , 137, 75 ), // #490
  INST(Pcmpeqq          , ExtRm              , O(660F38,29,_,_,_,_,_,_  ), 0                         , 2  , 0  , 6699 , 139, 12 ), // #491
  INST(Pcmpeqw          , ExtRm_P            , O(000F00,75,_,_,_,_,_,_  ), 0                         , 4  , 0  , 6708 , 137, 75 ), // #492
  INST(Pcmpestri        , ExtRmi             , O(660F3A,61,_,_,_,_,_,_  ), 0                         , 8  , 0  , 6717 , 140, 83 ), // #493
  INST(Pcmpestrm        , ExtRmi             , O(660F3A,60,_,_,_,_,_,_  ), 0                         , 8  , 0  , 6728 , 141, 83 ), // #494
  INST(Pcmpgtb          , ExtRm_P            , O(000F00,64,_,_,_,_,_,_  ), 0                         , 4  , 0  , 6739 , 137, 75 ), // #495
  INST(Pcmpgtd          , ExtRm_P            , O(000F00,66,_,_,_,_,_,_  ), 0                         , 4  , 0  , 6748 , 137, 75 ), // #496
  INST(Pcmpgtq          , ExtRm              , O(660F38,37,_,_,_,_,_,_  ), 0                         , 2  , 0  , 6757 , 139, 43 ), // #497
  INST(Pcmpgtw          , ExtRm_P            , O(000F00,65,_,_,_,_,_,_  ), 0                         , 4  , 0  , 6766 , 137, 75 ), // #498
  INST(Pcmpistri        , ExtRmi             , O(660F3A,63,_,_,_,_,_,_  ), 0                         , 8  , 0  , 6775 , 142, 83 ), // #499
  INST(Pcmpistrm        , ExtRmi             , O(660F3A,62,_,_,_,_,_,_  ), 0                         , 8  , 0  , 6786 , 143, 83 ), // #500
  INST(Pcommit          , X86Op_O            , O(660F00,AE,7,_,_,_,_,_  ), 0                         , 23 , 0  , 2030 , 30 , 84 ), // #501
  INST(Pdep             , VexRvm_Wx          , V(F20F38,F5,_,0,x,_,_,_  ), 0                         , 77 , 0  , 2038 , 10 , 78 ), // #502
  INST(Pext             , VexRvm_Wx          , V(F30F38,F5,_,0,x,_,_,_  ), 0                         , 82 , 0  , 2043 , 10 , 78 ), // #503
  INST(Pextrb           , ExtExtract         , O(000F3A,14,_,_,_,_,_,_  ), 0                         , 79 , 0  , 7273 , 144, 12 ), // #504
  INST(Pextrd           , ExtExtract         , O(000F3A,16,_,_,_,_,_,_  ), 0                         , 79 , 0  , 7281 , 56 , 12 ), // #505
  INST(Pextrq           , ExtExtract         , O(000F3A,16,_,_,1,_,_,_  ), 0                         , 83 , 0  , 7289 , 145, 12 ), // #506
  INST(Pextrw           , ExtPextrw          , O(000F00,C5,_,_,_,_,_,_  ), O(000F3A,15,_,_,_,_,_,_  ), 4  , 65 , 7297 , 146, 85 ), // #507
  INST(Pf2id            , Ext3dNow           , O(000F0F,1D,_,_,_,_,_,_  ), 0                         , 81 , 0  , 2048 , 138, 49 ), // #508
  INST(Pf2iw            , Ext3dNow           , O(000F0F,1C,_,_,_,_,_,_  ), 0                         , 81 , 0  , 2054 , 138, 86 ), // #509
  INST(Pfacc            , Ext3dNow           , O(000F0F,AE,_,_,_,_,_,_  ), 0                         , 81 , 0  , 2060 , 138, 49 ), // #510
  INST(Pfadd            , Ext3dNow           , O(000F0F,9E,_,_,_,_,_,_  ), 0                         , 81 , 0  , 2066 , 138, 49 ), // #511
  INST(Pfcmpeq          , Ext3dNow           , O(000F0F,B0,_,_,_,_,_,_  ), 0                         , 81 , 0  , 2072 , 138, 49 ), // #512
  INST(Pfcmpge          , Ext3dNow           , O(000F0F,90,_,_,_,_,_,_  ), 0                         , 81 , 0  , 2080 , 138, 49 ), // #513
  INST(Pfcmpgt          , Ext3dNow           , O(000F0F,A0,_,_,_,_,_,_  ), 0                         , 81 , 0  , 2088 , 138, 49 ), // #514
  INST(Pfmax            , Ext3dNow           , O(000F0F,A4,_,_,_,_,_,_  ), 0                         , 81 , 0  , 2096 , 138, 49 ), // #515
  INST(Pfmin            , Ext3dNow           , O(000F0F,94,_,_,_,_,_,_  ), 0                         , 81 , 0  , 2102 , 138, 49 ), // #516
  INST(Pfmul            , Ext3dNow           , O(000F0F,B4,_,_,_,_,_,_  ), 0                         , 81 , 0  , 2108 , 138, 49 ), // #517
  INST(Pfnacc           , Ext3dNow           , O(000F0F,8A,_,_,_,_,_,_  ), 0                         , 81 , 0  , 2114 , 138, 86 ), // #518
  INST(Pfpnacc          , Ext3dNow           , O(000F0F,8E,_,_,_,_,_,_  ), 0                         , 81 , 0  , 2121 , 138, 86 ), // #519
  INST(Pfrcp            , Ext3dNow           , O(000F0F,96,_,_,_,_,_,_  ), 0                         , 81 , 0  , 2129 , 138, 49 ), // #520
  INST(Pfrcpit1         , Ext3dNow           , O(000F0F,A6,_,_,_,_,_,_  ), 0                         , 81 , 0  , 2135 , 138, 49 ), // #521
  INST(Pfrcpit2         , Ext3dNow           , O(000F0F,B6,_,_,_,_,_,_  ), 0                         , 81 , 0  , 2144 , 138, 49 ), // #522
  INST(Pfrcpv           , Ext3dNow           , O(000F0F,86,_,_,_,_,_,_  ), 0                         , 81 , 0  , 2153 , 138, 87 ), // #523
  INST(Pfrsqit1         , Ext3dNow           , O(000F0F,A7,_,_,_,_,_,_  ), 0                         , 81 , 0  , 2160 , 138, 49 ), // #524
  INST(Pfrsqrt          , Ext3dNow           , O(000F0F,97,_,_,_,_,_,_  ), 0                         , 81 , 0  , 2169 , 138, 49 ), // #525
  INST(Pfrsqrtv         , Ext3dNow           , O(000F0F,87,_,_,_,_,_,_  ), 0                         , 81 , 0  , 2177 , 138, 87 ), // #526
  INST(Pfsub            , Ext3dNow           , O(000F0F,9A,_,_,_,_,_,_  ), 0                         , 81 , 0  , 2186 , 138, 49 ), // #527
  INST(Pfsubr           , Ext3dNow           , O(000F0F,AA,_,_,_,_,_,_  ), 0                         , 81 , 0  , 2192 , 138, 49 ), // #528
  INST(Phaddd           , ExtRm_P            , O(000F38,02,_,_,_,_,_,_  ), 0                         , 76 , 0  , 7376 , 134, 80 ), // #529
  INST(Phaddsw          , ExtRm_P            , O(000F38,03,_,_,_,_,_,_  ), 0                         , 76 , 0  , 7393 , 134, 80 ), // #530
  INST(Phaddw           , ExtRm_P            , O(000F38,01,_,_,_,_,_,_  ), 0                         , 76 , 0  , 7462 , 134, 80 ), // #531
  INST(Phminposuw       , ExtRm              , O(660F38,41,_,_,_,_,_,_  ), 0                         , 2  , 0  , 7488 , 5  , 12 ), // #532
  INST(Phsubd           , ExtRm_P            , O(000F38,06,_,_,_,_,_,_  ), 0                         , 76 , 0  , 7509 , 134, 80 ), // #533
  INST(Phsubsw          , ExtRm_P            , O(000F38,07,_,_,_,_,_,_  ), 0                         , 76 , 0  , 7526 , 134, 80 ), // #534
  INST(Phsubw           , ExtRm_P            , O(000F38,05,_,_,_,_,_,_  ), 0                         , 76 , 0  , 7535 , 134, 80 ), // #535
  INST(Pi2fd            , Ext3dNow           , O(000F0F,0D,_,_,_,_,_,_  ), 0                         , 81 , 0  , 2199 , 138, 49 ), // #536
  INST(Pi2fw            , Ext3dNow           , O(000F0F,0C,_,_,_,_,_,_  ), 0                         , 81 , 0  , 2205 , 138, 86 ), // #537
  INST(Pinsrb           , ExtRmi             , O(660F3A,20,_,_,_,_,_,_  ), 0                         , 8  , 0  , 7552 , 147, 12 ), // #538
  INST(Pinsrd           , ExtRmi             , O(660F3A,22,_,_,_,_,_,_  ), 0                         , 8  , 0  , 7560 , 148, 12 ), // #539
  INST(Pinsrq           , ExtRmi             , O(660F3A,22,_,_,1,_,_,_  ), 0                         , 84 , 0  , 7568 , 149, 12 ), // #540
  INST(Pinsrw           , ExtRmi_P           , O(000F00,C4,_,_,_,_,_,_  ), 0                         , 4  , 0  , 7576 , 150, 81 ), // #541
  INST(Pmaddubsw        , ExtRm_P            , O(000F38,04,_,_,_,_,_,_  ), 0                         , 76 , 0  , 7746 , 134, 80 ), // #542
  INST(Pmaddwd          , ExtRm_P            , O(000F00,F5,_,_,_,_,_,_  ), 0                         , 4  , 0  , 7757 , 134, 75 ), // #543
  INST(Pmaxsb           , ExtRm              , O(660F38,3C,_,_,_,_,_,_  ), 0                         , 2  , 0  , 7788 , 11 , 12 ), // #544
  INST(Pmaxsd           , ExtRm              , O(660F38,3D,_,_,_,_,_,_  ), 0                         , 2  , 0  , 7796 , 11 , 12 ), // #545
  INST(Pmaxsw           , ExtRm_P            , O(000F00,EE,_,_,_,_,_,_  ), 0                         , 4  , 0  , 7812 , 136, 81 ), // #546
  INST(Pmaxub           , ExtRm_P            , O(000F00,DE,_,_,_,_,_,_  ), 0                         , 4  , 0  , 7820 , 136, 81 ), // #547
  INST(Pmaxud           , ExtRm              , O(660F38,3F,_,_,_,_,_,_  ), 0                         , 2  , 0  , 7828 , 11 , 12 ), // #548
  INST(Pmaxuw           , ExtRm              , O(660F38,3E,_,_,_,_,_,_  ), 0                         , 2  , 0  , 7844 , 11 , 12 ), // #549
  INST(Pminsb           , ExtRm              , O(660F38,38,_,_,_,_,_,_  ), 0                         , 2  , 0  , 7852 , 11 , 12 ), // #550
  INST(Pminsd           , ExtRm              , O(660F38,39,_,_,_,_,_,_  ), 0                         , 2  , 0  , 7860 , 11 , 12 ), // #551
  INST(Pminsw           , ExtRm_P            , O(000F00,EA,_,_,_,_,_,_  ), 0                         , 4  , 0  , 7876 , 136, 81 ), // #552
  INST(Pminub           , ExtRm_P            , O(000F00,DA,_,_,_,_,_,_  ), 0                         , 4  , 0  , 7884 , 136, 81 ), // #553
  INST(Pminud           , ExtRm              , O(660F38,3B,_,_,_,_,_,_  ), 0                         , 2  , 0  , 7892 , 11 , 12 ), // #554
  INST(Pminuw           , ExtRm              , O(660F38,3A,_,_,_,_,_,_  ), 0                         , 2  , 0  , 7908 , 11 , 12 ), // #555
  INST(Pmovmskb         , ExtRm_P            , O(000F00,D7,_,_,_,_,_,_  ), 0                         , 4  , 0  , 7986 , 151, 81 ), // #556
  INST(Pmovsxbd         , ExtRm              , O(660F38,21,_,_,_,_,_,_  ), 0                         , 2  , 0  , 8083 , 7  , 12 ), // #557
  INST(Pmovsxbq         , ExtRm              , O(660F38,22,_,_,_,_,_,_  ), 0                         , 2  , 0  , 8093 , 152, 12 ), // #558
  INST(Pmovsxbw         , ExtRm              , O(660F38,20,_,_,_,_,_,_  ), 0                         , 2  , 0  , 8103 , 6  , 12 ), // #559
  INST(Pmovsxdq         , ExtRm              , O(660F38,25,_,_,_,_,_,_  ), 0                         , 2  , 0  , 8113 , 6  , 12 ), // #560
  INST(Pmovsxwd         , ExtRm              , O(660F38,23,_,_,_,_,_,_  ), 0                         , 2  , 0  , 8123 , 6  , 12 ), // #561
  INST(Pmovsxwq         , ExtRm              , O(660F38,24,_,_,_,_,_,_  ), 0                         , 2  , 0  , 8133 , 7  , 12 ), // #562
  INST(Pmovzxbd         , ExtRm              , O(660F38,31,_,_,_,_,_,_  ), 0                         , 2  , 0  , 8220 , 7  , 12 ), // #563
  INST(Pmovzxbq         , ExtRm              , O(660F38,32,_,_,_,_,_,_  ), 0                         , 2  , 0  , 8230 , 152, 12 ), // #564
  INST(Pmovzxbw         , ExtRm              , O(660F38,30,_,_,_,_,_,_  ), 0                         , 2  , 0  , 8240 , 6  , 12 ), // #565
  INST(Pmovzxdq         , ExtRm              , O(660F38,35,_,_,_,_,_,_  ), 0                         , 2  , 0  , 8250 , 6  , 12 ), // #566
  INST(Pmovzxwd         , ExtRm              , O(660F38,33,_,_,_,_,_,_  ), 0                         , 2  , 0  , 8260 , 6  , 12 ), // #567
  INST(Pmovzxwq         , ExtRm              , O(660F38,34,_,_,_,_,_,_  ), 0                         , 2  , 0  , 8270 , 7  , 12 ), // #568
  INST(Pmuldq           , ExtRm              , O(660F38,28,_,_,_,_,_,_  ), 0                         , 2  , 0  , 8280 , 5  , 12 ), // #569
  INST(Pmulhrsw         , ExtRm_P            , O(000F38,0B,_,_,_,_,_,_  ), 0                         , 76 , 0  , 8288 , 134, 80 ), // #570
  INST(Pmulhrw          , Ext3dNow           , O(000F0F,B7,_,_,_,_,_,_  ), 0                         , 81 , 0  , 2211 , 138, 49 ), // #571
  INST(Pmulhuw          , ExtRm_P            , O(000F00,E4,_,_,_,_,_,_  ), 0                         , 4  , 0  , 8298 , 134, 81 ), // #572
  INST(Pmulhw           , ExtRm_P            , O(000F00,E5,_,_,_,_,_,_  ), 0                         , 4  , 0  , 8307 , 134, 75 ), // #573
  INST(Pmulld           , ExtRm              , O(660F38,40,_,_,_,_,_,_  ), 0                         , 2  , 0  , 8315 , 5  , 12 ), // #574
  INST(Pmullw           , ExtRm_P            , O(000F00,D5,_,_,_,_,_,_  ), 0                         , 4  , 0  , 8331 , 134, 75 ), // #575
  INST(Pmuludq          , ExtRm_P            , O(000F00,F4,_,_,_,_,_,_  ), 0                         , 4  , 0  , 8354 , 134, 4  ), // #576
  INST(Pop              , X86Pop             , O(000000,8F,0,_,_,_,_,_  ), O(000000,58,_,_,_,_,_,_  ), 0  , 66 , 2219 , 153, 0  ), // #577
  INST(Popa             , X86Op              , O(660000,61,_,_,_,_,_,_  ), 0                         , 19 , 0  , 2223 , 75 , 0  ), // #578
  INST(Popad            , X86Op              , O(000000,61,_,_,_,_,_,_  ), 0                         , 0  , 0  , 2228 , 75 , 0  ), // #579
  INST(Popcnt           , X86Rm_Raw66H       , O(F30F00,B8,_,_,x,_,_,_  ), 0                         , 6  , 0  , 2234 , 22 , 88 ), // #580
  INST(Popf             , X86Op              , O(660000,9D,_,_,_,_,_,_  ), 0                         , 19 , 0  , 2241 , 30 , 89 ), // #581
  INST(Popfd            , X86Op              , O(000000,9D,_,_,_,_,_,_  ), 0                         , 0  , 0  , 2246 , 75 , 89 ), // #582
  INST(Popfq            , X86Op              , O(000000,9D,_,_,_,_,_,_  ), 0                         , 0  , 0  , 2252 , 154, 89 ), // #583
  INST(Por              , ExtRm_P            , O(000F00,EB,_,_,_,_,_,_  ), 0                         , 4  , 0  , 8399 , 136, 75 ), // #584
  INST(Prefetch         , X86M_Only          , O(000F00,0D,0,_,_,_,_,_  ), 0                         , 4  , 0  , 2258 , 31 , 49 ), // #585
  INST(Prefetchnta      , X86M_Only          , O(000F00,18,0,_,_,_,_,_  ), 0                         , 4  , 0  , 2267 , 31 , 71 ), // #586
  INST(Prefetcht0       , X86M_Only          , O(000F00,18,1,_,_,_,_,_  ), 0                         , 27 , 0  , 2279 , 31 , 71 ), // #587
  INST(Prefetcht1       , X86M_Only          , O(000F00,18,2,_,_,_,_,_  ), 0                         , 69 , 0  , 2290 , 31 , 71 ), // #588
  INST(Prefetcht2       , X86M_Only          , O(000F00,18,3,_,_,_,_,_  ), 0                         , 71 , 0  , 2301 , 31 , 71 ), // #589
  INST(Prefetchw        , X86M_Only          , O(000F00,0D,1,_,_,_,_,_  ), 0                         , 27 , 0  , 2312 , 31 , 90 ), // #590
  INST(Prefetchwt1      , X86M_Only          , O(000F00,0D,2,_,_,_,_,_  ), 0                         , 69 , 0  , 2322 , 31 , 91 ), // #591
  INST(Psadbw           , ExtRm_P            , O(000F00,F6,_,_,_,_,_,_  ), 0                         , 4  , 0  , 3980 , 134, 81 ), // #592
  INST(Pshufb           , ExtRm_P            , O(000F38,00,_,_,_,_,_,_  ), 0                         , 76 , 0  , 8725 , 134, 80 ), // #593
  INST(Pshufd           , ExtRmi             , O(660F00,70,_,_,_,_,_,_  ), 0                         , 3  , 0  , 8746 , 8  , 4  ), // #594
  INST(Pshufhw          , ExtRmi             , O(F30F00,70,_,_,_,_,_,_  ), 0                         , 6  , 0  , 8754 , 8  , 4  ), // #595
  INST(Pshuflw          , ExtRmi             , O(F20F00,70,_,_,_,_,_,_  ), 0                         , 5  , 0  , 8763 , 8  , 4  ), // #596
  INST(Pshufw           , ExtRmi_P           , O(000F00,70,_,_,_,_,_,_  ), 0                         , 4  , 0  , 2334 , 155, 71 ), // #597
  INST(Psignb           , ExtRm_P            , O(000F38,08,_,_,_,_,_,_  ), 0                         , 76 , 0  , 8772 , 134, 80 ), // #598
  INST(Psignd           , ExtRm_P            , O(000F38,0A,_,_,_,_,_,_  ), 0                         , 76 , 0  , 8780 , 134, 80 ), // #599
  INST(Psignw           , ExtRm_P            , O(000F38,09,_,_,_,_,_,_  ), 0                         , 76 , 0  , 8788 , 134, 80 ), // #600
  INST(Pslld            , ExtRmRi_P          , O(000F00,F2,_,_,_,_,_,_  ), O(000F00,72,6,_,_,_,_,_  ), 4  , 67 , 8796 , 156, 75 ), // #601
  INST(Pslldq           , ExtRmRi            , 0                         , O(660F00,73,7,_,_,_,_,_  ), 0  , 68 , 8803 , 157, 4  ), // #602
  INST(Psllq            , ExtRmRi_P          , O(000F00,F3,_,_,_,_,_,_  ), O(000F00,73,6,_,_,_,_,_  ), 4  , 69 , 8811 , 156, 75 ), // #603
  INST(Psllw            , ExtRmRi_P          , O(000F00,F1,_,_,_,_,_,_  ), O(000F00,71,6,_,_,_,_,_  ), 4  , 70 , 8842 , 156, 75 ), // #604
  INST(Psrad            , ExtRmRi_P          , O(000F00,E2,_,_,_,_,_,_  ), O(000F00,72,4,_,_,_,_,_  ), 4  , 71 , 8849 , 156, 75 ), // #605
  INST(Psraw            , ExtRmRi_P          , O(000F00,E1,_,_,_,_,_,_  ), O(000F00,71,4,_,_,_,_,_  ), 4  , 72 , 8887 , 156, 75 ), // #606
  INST(Psrld            , ExtRmRi_P          , O(000F00,D2,_,_,_,_,_,_  ), O(000F00,72,2,_,_,_,_,_  ), 4  , 73 , 8894 , 156, 75 ), // #607
  INST(Psrldq           , ExtRmRi            , 0                         , O(660F00,73,3,_,_,_,_,_  ), 0  , 74 , 8901 , 157, 4  ), // #608
  INST(Psrlq            , ExtRmRi_P          , O(000F00,D3,_,_,_,_,_,_  ), O(000F00,73,2,_,_,_,_,_  ), 4  , 75 , 8909 , 156, 75 ), // #609
  INST(Psrlw            , ExtRmRi_P          , O(000F00,D1,_,_,_,_,_,_  ), O(000F00,71,2,_,_,_,_,_  ), 4  , 76 , 8940 , 156, 75 ), // #610
  INST(Psubb            , ExtRm_P            , O(000F00,F8,_,_,_,_,_,_  ), 0                         , 4  , 0  , 8947 , 137, 75 ), // #611
  INST(Psubd            , ExtRm_P            , O(000F00,FA,_,_,_,_,_,_  ), 0                         , 4  , 0  , 8954 , 137, 75 ), // #612
  INST(Psubq            , ExtRm_P            , O(000F00,FB,_,_,_,_,_,_  ), 0                         , 4  , 0  , 8961 , 137, 4  ), // #613
  INST(Psubsb           , ExtRm_P            , O(000F00,E8,_,_,_,_,_,_  ), 0                         , 4  , 0  , 8968 , 137, 75 ), // #614
  INST(Psubsw           , ExtRm_P            , O(000F00,E9,_,_,_,_,_,_  ), 0                         , 4  , 0  , 8976 , 137, 75 ), // #615
  INST(Psubusb          , ExtRm_P            , O(000F00,D8,_,_,_,_,_,_  ), 0                         , 4  , 0  , 8984 , 137, 75 ), // #616
  INST(Psubusw          , ExtRm_P            , O(000F00,D9,_,_,_,_,_,_  ), 0                         , 4  , 0  , 8993 , 137, 75 ), // #617
  INST(Psubw            , ExtRm_P            , O(000F00,F9,_,_,_,_,_,_  ), 0                         , 4  , 0  , 9002 , 137, 75 ), // #618
  INST(Pswapd           , Ext3dNow           , O(000F0F,BB,_,_,_,_,_,_  ), 0                         , 81 , 0  , 2341 , 138, 86 ), // #619
  INST(Ptest            , ExtRm              , O(660F38,17,_,_,_,_,_,_  ), 0                         , 2  , 0  , 9031 , 5  , 92 ), // #620
  INST(Punpckhbw        , ExtRm_P            , O(000F00,68,_,_,_,_,_,_  ), 0                         , 4  , 0  , 9114 , 134, 75 ), // #621
  INST(Punpckhdq        , ExtRm_P            , O(000F00,6A,_,_,_,_,_,_  ), 0                         , 4  , 0  , 9125 , 134, 75 ), // #622
  INST(Punpckhqdq       , ExtRm              , O(660F00,6D,_,_,_,_,_,_  ), 0                         , 3  , 0  , 9136 , 5  , 4  ), // #623
  INST(Punpckhwd        , ExtRm_P            , O(000F00,69,_,_,_,_,_,_  ), 0                         , 4  , 0  , 9148 , 134, 75 ), // #624
  INST(Punpcklbw        , ExtRm_P            , O(000F00,60,_,_,_,_,_,_  ), 0                         , 4  , 0  , 9159 , 134, 75 ), // #625
  INST(Punpckldq        , ExtRm_P            , O(000F00,62,_,_,_,_,_,_  ), 0                         , 4  , 0  , 9170 , 134, 75 ), // #626
  INST(Punpcklqdq       , ExtRm              , O(660F00,6C,_,_,_,_,_,_  ), 0                         , 3  , 0  , 9181 , 5  , 4  ), // #627
  INST(Punpcklwd        , ExtRm_P            , O(000F00,61,_,_,_,_,_,_  ), 0                         , 4  , 0  , 9193 , 134, 75 ), // #628
  INST(Push             , X86Push            , O(000000,FF,6,_,_,_,_,_  ), O(000000,50,_,_,_,_,_,_  ), 30 , 77 , 2348 , 158, 0  ), // #629
  INST(Pusha            , X86Op              , O(660000,60,_,_,_,_,_,_  ), 0                         , 19 , 0  , 2353 , 75 , 0  ), // #630
  INST(Pushad           , X86Op              , O(000000,60,_,_,_,_,_,_  ), 0                         , 0  , 0  , 2359 , 75 , 0  ), // #631
  INST(Pushf            , X86Op              , O(660000,9C,_,_,_,_,_,_  ), 0                         , 19 , 0  , 2366 , 30 , 93 ), // #632
  INST(Pushfd           , X86Op              , O(000000,9C,_,_,_,_,_,_  ), 0                         , 0  , 0  , 2372 , 75 , 93 ), // #633
  INST(Pushfq           , X86Op              , O(000000,9C,_,_,_,_,_,_  ), 0                         , 0  , 0  , 2379 , 154, 93 ), // #634
  INST(Pxor             , ExtRm_P            , O(000F00,EF,_,_,_,_,_,_  ), 0                         , 4  , 0  , 9204 , 137, 75 ), // #635
  INST(Rcl              , X86Rot             , O(000000,D0,2,_,x,_,_,_  ), 0                         , 1  , 0  , 2386 , 159, 94 ), // #636
  INST(Rcpps            , ExtRm              , O(000F00,53,_,_,_,_,_,_  ), 0                         , 4  , 0  , 9332 , 5  , 5  ), // #637
  INST(Rcpss            , ExtRm              , O(F30F00,53,_,_,_,_,_,_  ), 0                         , 6  , 0  , 9339 , 7  , 5  ), // #638
  INST(Rcr              , X86Rot             , O(000000,D0,3,_,x,_,_,_  ), 0                         , 78 , 0  , 2390 , 159, 94 ), // #639
  INST(Rdfsbase         , X86M               , O(F30F00,AE,0,_,x,_,_,_  ), 0                         , 6  , 0  , 2394 , 160, 95 ), // #640
  INST(Rdgsbase         , X86M               , O(F30F00,AE,1,_,x,_,_,_  ), 0                         , 85 , 0  , 2403 , 160, 95 ), // #641
  INST(Rdmsr            , X86Op              , O(000F00,32,_,_,_,_,_,_  ), 0                         , 4  , 0  , 2412 , 161, 96 ), // #642
  INST(Rdpid            , X86R_Native        , O(F30F00,C7,7,_,_,_,_,_  ), 0                         , 86 , 0  , 2418 , 162, 97 ), // #643
  INST(Rdpmc            , X86Op              , O(000F00,33,_,_,_,_,_,_  ), 0                         , 4  , 0  , 2424 , 161, 0  ), // #644
  INST(Rdrand           , X86M               , O(000F00,C7,6,_,x,_,_,_  ), 0                         , 73 , 0  , 2430 , 163, 98 ), // #645
  INST(Rdseed           , X86M               , O(000F00,C7,7,_,x,_,_,_  ), 0                         , 22 , 0  , 2437 , 163, 99 ), // #646
  INST(Rdtsc            , X86Op              , O(000F00,31,_,_,_,_,_,_  ), 0                         , 4  , 0  , 2444 , 28 , 100), // #647
  INST(Rdtscp           , X86Op              , O(000F01,F9,_,_,_,_,_,_  ), 0                         , 21 , 0  , 2450 , 161, 101), // #648
  INST(Ret              , X86Ret             , O(000000,C2,_,_,_,_,_,_  ), 0                         , 0  , 0  , 2883 , 164, 0  ), // #649
  INST(Rol              , X86Rot             , O(000000,D0,0,_,x,_,_,_  ), 0                         , 0  , 0  , 2457 , 159, 102), // #650
  INST(Ror              , X86Rot             , O(000000,D0,1,_,x,_,_,_  ), 0                         , 29 , 0  , 2461 , 159, 102), // #651
  INST(Rorx             , VexRmi_Wx          , V(F20F3A,F0,_,0,x,_,_,_  ), 0                         , 87 , 0  , 2465 , 165, 78 ), // #652
  INST(Roundpd          , ExtRmi             , O(660F3A,09,_,_,_,_,_,_  ), 0                         , 8  , 0  , 9434 , 8  , 12 ), // #653
  INST(Roundps          , ExtRmi             , O(660F3A,08,_,_,_,_,_,_  ), 0                         , 8  , 0  , 9443 , 8  , 12 ), // #654
  INST(Roundsd          , ExtRmi             , O(660F3A,0B,_,_,_,_,_,_  ), 0                         , 8  , 0  , 9452 , 35 , 12 ), // #655
  INST(Roundss          , ExtRmi             , O(660F3A,0A,_,_,_,_,_,_  ), 0                         , 8  , 0  , 9461 , 36 , 12 ), // #656
  INST(Rsm              , X86Op              , O(000F00,AA,_,_,_,_,_,_  ), 0                         , 4  , 0  , 2470 , 75 , 1  ), // #657
  INST(Rsqrtps          , ExtRm              , O(000F00,52,_,_,_,_,_,_  ), 0                         , 4  , 0  , 9558 , 5  , 5  ), // #658
  INST(Rsqrtss          , ExtRm              , O(F30F00,52,_,_,_,_,_,_  ), 0                         , 6  , 0  , 9567 , 7  , 5  ), // #659
  INST(Sahf             , X86Op              , O(000000,9E,_,_,_,_,_,_  ), 0                         , 0  , 0  , 2474 , 90 , 103), // #660
  INST(Sal              , X86Rot             , O(000000,D0,4,_,x,_,_,_  ), 0                         , 9  , 0  , 2479 , 159, 1  ), // #661
  INST(Sar              , X86Rot             , O(000000,D0,7,_,x,_,_,_  ), 0                         , 25 , 0  , 2483 , 159, 1  ), // #662
  INST(Sarx             , VexRmv_Wx          , V(F30F38,F7,_,0,x,_,_,_  ), 0                         , 82 , 0  , 2487 , 13 , 78 ), // #663
  INST(Sbb              , X86Arith           , O(000000,18,3,_,x,_,_,_  ), 0                         , 78 , 0  , 2492 , 3  , 2  ), // #664
  INST(Scas             , X86StrRm           , O(000000,AE,_,_,_,_,_,_  ), 0                         , 0  , 0  , 2496 , 166, 36 ), // #665
  INST(Seta             , X86Set             , O(000F00,97,_,_,_,_,_,_  ), 0                         , 4  , 0  , 2501 , 167, 55 ), // #666
  INST(Setae            , X86Set             , O(000F00,93,_,_,_,_,_,_  ), 0                         , 4  , 0  , 2506 , 167, 56 ), // #667
  INST(Setb             , X86Set             , O(000F00,92,_,_,_,_,_,_  ), 0                         , 4  , 0  , 2512 , 167, 56 ), // #668
  INST(Setbe            , X86Set             , O(000F00,96,_,_,_,_,_,_  ), 0                         , 4  , 0  , 2517 , 167, 55 ), // #669
  INST(Setc             , X86Set             , O(000F00,92,_,_,_,_,_,_  ), 0                         , 4  , 0  , 2523 , 167, 56 ), // #670
  INST(Sete             , X86Set             , O(000F00,94,_,_,_,_,_,_  ), 0                         , 4  , 0  , 2528 , 167, 57 ), // #671
  INST(Setg             , X86Set             , O(000F00,9F,_,_,_,_,_,_  ), 0                         , 4  , 0  , 2533 , 167, 58 ), // #672
  INST(Setge            , X86Set             , O(000F00,9D,_,_,_,_,_,_  ), 0                         , 4  , 0  , 2538 , 167, 59 ), // #673
  INST(Setl             , X86Set             , O(000F00,9C,_,_,_,_,_,_  ), 0                         , 4  , 0  , 2544 , 167, 59 ), // #674
  INST(Setle            , X86Set             , O(000F00,9E,_,_,_,_,_,_  ), 0                         , 4  , 0  , 2549 , 167, 58 ), // #675
  INST(Setna            , X86Set             , O(000F00,96,_,_,_,_,_,_  ), 0                         , 4  , 0  , 2555 , 167, 55 ), // #676
  INST(Setnae           , X86Set             , O(000F00,92,_,_,_,_,_,_  ), 0                         , 4  , 0  , 2561 , 167, 56 ), // #677
  INST(Setnb            , X86Set             , O(000F00,93,_,_,_,_,_,_  ), 0                         , 4  , 0  , 2568 , 167, 56 ), // #678
  INST(Setnbe           , X86Set             , O(000F00,97,_,_,_,_,_,_  ), 0                         , 4  , 0  , 2574 , 167, 55 ), // #679
  INST(Setnc            , X86Set             , O(000F00,93,_,_,_,_,_,_  ), 0                         , 4  , 0  , 2581 , 167, 56 ), // #680
  INST(Setne            , X86Set             , O(000F00,95,_,_,_,_,_,_  ), 0                         , 4  , 0  , 2587 , 167, 57 ), // #681
  INST(Setng            , X86Set             , O(000F00,9E,_,_,_,_,_,_  ), 0                         , 4  , 0  , 2593 , 167, 58 ), // #682
  INST(Setnge           , X86Set             , O(000F00,9C,_,_,_,_,_,_  ), 0                         , 4  , 0  , 2599 , 167, 59 ), // #683
  INST(Setnl            , X86Set             , O(000F00,9D,_,_,_,_,_,_  ), 0                         , 4  , 0  , 2606 , 167, 59 ), // #684
  INST(Setnle           , X86Set             , O(000F00,9F,_,_,_,_,_,_  ), 0                         , 4  , 0  , 2612 , 167, 58 ), // #685
  INST(Setno            , X86Set             , O(000F00,91,_,_,_,_,_,_  ), 0                         , 4  , 0  , 2619 , 167, 53 ), // #686
  INST(Setnp            , X86Set             , O(000F00,9B,_,_,_,_,_,_  ), 0                         , 4  , 0  , 2625 , 167, 60 ), // #687
  INST(Setns            , X86Set             , O(000F00,99,_,_,_,_,_,_  ), 0                         , 4  , 0  , 2631 , 167, 61 ), // #688
  INST(Setnz            , X86Set             , O(000F00,95,_,_,_,_,_,_  ), 0                         , 4  , 0  , 2637 , 167, 57 ), // #689
  INST(Seto             , X86Set             , O(000F00,90,_,_,_,_,_,_  ), 0                         , 4  , 0  , 2643 , 167, 53 ), // #690
  INST(Setp             , X86Set             , O(000F00,9A,_,_,_,_,_,_  ), 0                         , 4  , 0  , 2648 , 167, 60 ), // #691
  INST(Setpe            , X86Set             , O(000F00,9A,_,_,_,_,_,_  ), 0                         , 4  , 0  , 2653 , 167, 60 ), // #692
  INST(Setpo            , X86Set             , O(000F00,9B,_,_,_,_,_,_  ), 0                         , 4  , 0  , 2659 , 167, 60 ), // #693
  INST(Sets             , X86Set             , O(000F00,98,_,_,_,_,_,_  ), 0                         , 4  , 0  , 2665 , 167, 61 ), // #694
  INST(Setz             , X86Set             , O(000F00,94,_,_,_,_,_,_  ), 0                         , 4  , 0  , 2670 , 167, 57 ), // #695
  INST(Sfence           , X86Fence           , O(000F00,AE,7,_,_,_,_,_  ), 0                         , 22 , 0  , 2675 , 30 , 71 ), // #696
  INST(Sgdt             , X86M_Only          , O(000F00,01,0,_,_,_,_,_  ), 0                         , 4  , 0  , 2682 , 31 , 0  ), // #697
  INST(Sha1msg1         , ExtRm              , O(000F38,C9,_,_,_,_,_,_  ), 0                         , 76 , 0  , 2687 , 5  , 104), // #698
  INST(Sha1msg2         , ExtRm              , O(000F38,CA,_,_,_,_,_,_  ), 0                         , 76 , 0  , 2696 , 5  , 104), // #699
  INST(Sha1nexte        , ExtRm              , O(000F38,C8,_,_,_,_,_,_  ), 0                         , 76 , 0  , 2705 , 5  , 104), // #700
  INST(Sha1rnds4        , ExtRmi             , O(000F3A,CC,_,_,_,_,_,_  ), 0                         , 79 , 0  , 2715 , 8  , 104), // #701
  INST(Sha256msg1       , ExtRm              , O(000F38,CC,_,_,_,_,_,_  ), 0                         , 76 , 0  , 2725 , 5  , 104), // #702
  INST(Sha256msg2       , ExtRm              , O(000F38,CD,_,_,_,_,_,_  ), 0                         , 76 , 0  , 2736 , 5  , 104), // #703
  INST(Sha256rnds2      , ExtRm_XMM0         , O(000F38,CB,_,_,_,_,_,_  ), 0                         , 76 , 0  , 2747 , 15 , 104), // #704
  INST(Shl              , X86Rot             , O(000000,D0,4,_,x,_,_,_  ), 0                         , 9  , 0  , 2759 , 159, 1  ), // #705
  INST(Shld             , X86ShldShrd        , O(000F00,A4,_,_,x,_,_,_  ), 0                         , 4  , 0  , 8603 , 168, 1  ), // #706
  INST(Shlx             , VexRmv_Wx          , V(660F38,F7,_,0,x,_,_,_  ), 0                         , 88 , 0  , 2763 , 13 , 78 ), // #707
  INST(Shr              , X86Rot             , O(000000,D0,5,_,x,_,_,_  ), 0                         , 58 , 0  , 2768 , 159, 1  ), // #708
  INST(Shrd             , X86ShldShrd        , O(000F00,AC,_,_,x,_,_,_  ), 0                         , 4  , 0  , 2772 , 168, 1  ), // #709
  INST(Shrx             , VexRmv_Wx          , V(F20F38,F7,_,0,x,_,_,_  ), 0                         , 77 , 0  , 2777 , 13 , 78 ), // #710
  INST(Shufpd           , ExtRmi             , O(660F00,C6,_,_,_,_,_,_  ), 0                         , 3  , 0  , 9828 , 8  , 4  ), // #711
  INST(Shufps           , ExtRmi             , O(000F00,C6,_,_,_,_,_,_  ), 0                         , 4  , 0  , 9836 , 8  , 5  ), // #712
  INST(Sidt             , X86M_Only          , O(000F00,01,1,_,_,_,_,_  ), 0                         , 27 , 0  , 2782 , 31 , 0  ), // #713
  INST(Skinit           , X86Op_xAX          , O(000F01,DE,_,_,_,_,_,_  ), 0                         , 21 , 0  , 2787 , 50 , 105), // #714
  INST(Sldt             , X86M               , O(000F00,00,0,_,_,_,_,_  ), 0                         , 4  , 0  , 2794 , 169, 0  ), // #715
  INST(Slwpcb           , VexR_Wx            , V(XOP_M9,12,1,0,x,_,_,_  ), 0                         , 11 , 0  , 2799 , 98 , 69 ), // #716
  INST(Smsw             , X86M               , O(000F00,01,4,_,_,_,_,_  ), 0                         , 89 , 0  , 2806 , 169, 0  ), // #717
  INST(Sqrtpd           , ExtRm              , O(660F00,51,_,_,_,_,_,_  ), 0                         , 3  , 0  , 9844 , 5  , 4  ), // #718
  INST(Sqrtps           , ExtRm              , O(000F00,51,_,_,_,_,_,_  ), 0                         , 4  , 0  , 9559 , 5  , 5  ), // #719
  INST(Sqrtsd           , ExtRm              , O(F20F00,51,_,_,_,_,_,_  ), 0                         , 5  , 0  , 9860 , 6  , 4  ), // #720
  INST(Sqrtss           , ExtRm              , O(F30F00,51,_,_,_,_,_,_  ), 0                         , 6  , 0  , 9568 , 7  , 5  ), // #721
  INST(Stac             , X86Op              , O(000F01,CB,_,_,_,_,_,_  ), 0                         , 21 , 0  , 2811 , 30 , 17 ), // #722
  INST(Stc              , X86Op              , O(000000,F9,_,_,_,_,_,_  ), 0                         , 0  , 0  , 2816 , 30 , 18 ), // #723
  INST(Std              , X86Op              , O(000000,FD,_,_,_,_,_,_  ), 0                         , 0  , 0  , 6586 , 30 , 19 ), // #724
  INST(Stgi             , X86Op              , O(000F01,DC,_,_,_,_,_,_  ), 0                         , 21 , 0  , 2820 , 30 , 105), // #725
  INST(Sti              , X86Op              , O(000000,FB,_,_,_,_,_,_  ), 0                         , 0  , 0  , 2825 , 30 , 24 ), // #726
  INST(Stmxcsr          , X86M_Only          , O(000F00,AE,3,_,_,_,_,_  ), 0                         , 71 , 0  , 9876 , 93 , 5  ), // #727
  INST(Stos             , X86StrMr           , O(000000,AA,_,_,_,_,_,_  ), 0                         , 0  , 0  , 2829 , 170, 0  ), // #728
  INST(Str              , X86M               , O(000F00,00,1,_,_,_,_,_  ), 0                         , 27 , 0  , 2834 , 169, 0  ), // #729
  INST(Sub              , X86Arith           , O(000000,28,5,_,x,_,_,_  ), 0                         , 58 , 0  , 836  , 171, 1  ), // #730
  INST(Subpd            , ExtRm              , O(660F00,5C,_,_,_,_,_,_  ), 0                         , 3  , 0  , 4556 , 5  , 4  ), // #731
  INST(Subps            , ExtRm              , O(000F00,5C,_,_,_,_,_,_  ), 0                         , 4  , 0  , 4568 , 5  , 5  ), // #732
  INST(Subsd            , ExtRm              , O(F20F00,5C,_,_,_,_,_,_  ), 0                         , 5  , 0  , 5244 , 6  , 4  ), // #733
  INST(Subss            , ExtRm              , O(F30F00,5C,_,_,_,_,_,_  ), 0                         , 6  , 0  , 5254 , 7  , 5  ), // #734
  INST(Swapgs           , X86Op              , O(000F01,F8,_,_,_,_,_,_  ), 0                         , 21 , 0  , 2838 , 154, 0  ), // #735
  INST(Syscall          , X86Op              , O(000F00,05,_,_,_,_,_,_  ), 0                         , 4  , 0  , 2845 , 154, 0  ), // #736
  INST(Sysenter         , X86Op              , O(000F00,34,_,_,_,_,_,_  ), 0                         , 4  , 0  , 2853 , 30 , 0  ), // #737
  INST(Sysexit          , X86Op              , O(000F00,35,_,_,_,_,_,_  ), 0                         , 4  , 0  , 2862 , 30 , 0  ), // #738
  INST(Sysexit64        , X86Op              , O(000F00,35,_,_,_,_,_,_  ), 0                         , 4  , 0  , 2870 , 30 , 0  ), // #739
  INST(Sysret           , X86Op              , O(000F00,07,_,_,_,_,_,_  ), 0                         , 4  , 0  , 2880 , 154, 0  ), // #740
  INST(Sysret64         , X86Op              , O(000F00,07,_,_,_,_,_,_  ), 0                         , 4  , 0  , 2887 , 154, 0  ), // #741
  INST(T1mskc           , VexVm_Wx           , V(XOP_M9,01,7,0,x,_,_,_  ), 0                         , 90 , 0  , 2896 , 14 , 11 ), // #742
  INST(Test             , X86Test            , O(000000,84,_,_,x,_,_,_  ), O(000000,F6,_,_,x,_,_,_  ), 0  , 78 , 9032 , 172, 1  ), // #743
  INST(Tzcnt            , X86Rm_Raw66H       , O(F30F00,BC,_,_,x,_,_,_  ), 0                         , 6  , 0  , 2903 , 22 , 9  ), // #744
  INST(Tzmsk            , VexVm_Wx           , V(XOP_M9,01,4,0,x,_,_,_  ), 0                         , 91 , 0  , 2909 , 14 , 11 ), // #745
  INST(Ucomisd          , ExtRm              , O(660F00,2E,_,_,_,_,_,_  ), 0                         , 3  , 0  , 9929 , 6  , 40 ), // #746
  INST(Ucomiss          , ExtRm              , O(000F00,2E,_,_,_,_,_,_  ), 0                         , 4  , 0  , 9938 , 7  , 41 ), // #747
  INST(Ud2              , X86Op              , O(000F00,0B,_,_,_,_,_,_  ), 0                         , 4  , 0  , 2915 , 30 , 0  ), // #748
  INST(Unpckhpd         , ExtRm              , O(660F00,15,_,_,_,_,_,_  ), 0                         , 3  , 0  , 9947 , 5  , 4  ), // #749
  INST(Unpckhps         , ExtRm              , O(000F00,15,_,_,_,_,_,_  ), 0                         , 4  , 0  , 9957 , 5  , 5  ), // #750
  INST(Unpcklpd         , ExtRm              , O(660F00,14,_,_,_,_,_,_  ), 0                         , 3  , 0  , 9967 , 5  , 4  ), // #751
  INST(Unpcklps         , ExtRm              , O(000F00,14,_,_,_,_,_,_  ), 0                         , 4  , 0  , 9977 , 5  , 5  ), // #752
  INST(V4fmaddps        , VexRm_T1_4X        , V(F20F38,9A,_,2,_,0,2,T4X), 0                         , 92 , 0  , 2919 , 173, 106), // #753
  INST(V4fmaddss        , VexRm_T1_4X        , V(F20F38,9B,_,2,_,0,2,T4X), 0                         , 92 , 0  , 2929 , 174, 106), // #754
  INST(V4fnmaddps       , VexRm_T1_4X        , V(F20F38,AA,_,2,_,0,2,T4X), 0                         , 92 , 0  , 2939 , 173, 106), // #755
  INST(V4fnmaddss       , VexRm_T1_4X        , V(F20F38,AB,_,2,_,0,2,T4X), 0                         , 92 , 0  , 2950 , 174, 106), // #756
  INST(Vaddpd           , VexRvm_Lx          , V(660F00,58,_,x,I,1,4,FV ), 0                         , 93 , 0  , 2961 , 175, 107), // #757
  INST(Vaddps           , VexRvm_Lx          , V(000F00,58,_,x,I,0,4,FV ), 0                         , 94 , 0  , 2968 , 176, 107), // #758
  INST(Vaddsd           , VexRvm             , V(F20F00,58,_,I,I,1,3,T1S), 0                         , 95 , 0  , 2975 , 177, 108), // #759
  INST(Vaddss           , VexRvm             , V(F30F00,58,_,I,I,0,2,T1S), 0                         , 96 , 0  , 2982 , 178, 108), // #760
  INST(Vaddsubpd        , VexRvm_Lx          , V(660F00,D0,_,x,I,_,_,_  ), 0                         , 63 , 0  , 2989 , 179, 109), // #761
  INST(Vaddsubps        , VexRvm_Lx          , V(F20F00,D0,_,x,I,_,_,_  ), 0                         , 97 , 0  , 2999 , 179, 109), // #762
  INST(Vaesdec          , VexRvm_Lx          , V(660F38,DE,_,x,I,_,4,FVM), 0                         , 98 , 0  , 3009 , 180, 110), // #763
  INST(Vaesdeclast      , VexRvm_Lx          , V(660F38,DF,_,x,I,_,4,FVM), 0                         , 98 , 0  , 3017 , 180, 110), // #764
  INST(Vaesenc          , VexRvm_Lx          , V(660F38,DC,_,x,I,_,4,FVM), 0                         , 98 , 0  , 3029 , 180, 110), // #765
  INST(Vaesenclast      , VexRvm_Lx          , V(660F38,DD,_,x,I,_,4,FVM), 0                         , 98 , 0  , 3037 , 180, 110), // #766
  INST(Vaesimc          , VexRm              , V(660F38,DB,_,0,I,_,_,_  ), 0                         , 88 , 0  , 3049 , 181, 111), // #767
  INST(Vaeskeygenassist , VexRmi             , V(660F3A,DF,_,0,I,_,_,_  ), 0                         , 67 , 0  , 3057 , 182, 111), // #768
  INST(Valignd          , VexRvmi_Lx         , V(660F3A,03,_,x,_,0,4,FV ), 0                         , 99 , 0  , 3074 , 183, 112), // #769
  INST(Valignq          , VexRvmi_Lx         , V(660F3A,03,_,x,_,1,4,FV ), 0                         , 100, 0  , 3082 , 184, 112), // #770
  INST(Vandnpd          , VexRvm_Lx          , V(660F00,55,_,x,I,1,4,FV ), 0                         , 93 , 0  , 3090 , 185, 113), // #771
  INST(Vandnps          , VexRvm_Lx          , V(000F00,55,_,x,I,0,4,FV ), 0                         , 94 , 0  , 3098 , 186, 113), // #772
  INST(Vandpd           , VexRvm_Lx          , V(660F00,54,_,x,I,1,4,FV ), 0                         , 93 , 0  , 3106 , 187, 113), // #773
  INST(Vandps           , VexRvm_Lx          , V(000F00,54,_,x,I,0,4,FV ), 0                         , 94 , 0  , 3113 , 188, 113), // #774
  INST(Vblendmb         , VexRvm_Lx          , V(660F38,66,_,x,_,0,4,FVM), 0                         , 98 , 0  , 3120 , 189, 114), // #775
  INST(Vblendmd         , VexRvm_Lx          , V(660F38,64,_,x,_,0,4,FV ), 0                         , 101, 0  , 3129 , 190, 112), // #776
  INST(Vblendmpd        , VexRvm_Lx          , V(660F38,65,_,x,_,1,4,FV ), 0                         , 102, 0  , 3138 , 191, 112), // #777
  INST(Vblendmps        , VexRvm_Lx          , V(660F38,65,_,x,_,0,4,FV ), 0                         , 101, 0  , 3148 , 190, 112), // #778
  INST(Vblendmq         , VexRvm_Lx          , V(660F38,64,_,x,_,1,4,FV ), 0                         , 102, 0  , 3158 , 191, 112), // #779
  INST(Vblendmw         , VexRvm_Lx          , V(660F38,66,_,x,_,1,4,FVM), 0                         , 103, 0  , 3167 , 189, 114), // #780
  INST(Vblendpd         , VexRvmi_Lx         , V(660F3A,0D,_,x,I,_,_,_  ), 0                         , 67 , 0  , 3176 , 192, 109), // #781
  INST(Vblendps         , VexRvmi_Lx         , V(660F3A,0C,_,x,I,_,_,_  ), 0                         , 67 , 0  , 3185 , 192, 109), // #782
  INST(Vblendvpd        , VexRvmr_Lx         , V(660F3A,4B,_,x,0,_,_,_  ), 0                         , 67 , 0  , 3194 , 193, 109), // #783
  INST(Vblendvps        , VexRvmr_Lx         , V(660F3A,4A,_,x,0,_,_,_  ), 0                         , 67 , 0  , 3204 , 193, 109), // #784
  INST(Vbroadcastf128   , VexRm              , V(660F38,1A,_,1,0,_,_,_  ), 0                         , 104, 0  , 3214 , 194, 109), // #785
  INST(Vbroadcastf32x2  , VexRm_Lx           , V(660F38,19,_,x,_,0,3,T2 ), 0                         , 105, 0  , 3229 , 195, 115), // #786
  INST(Vbroadcastf32x4  , VexRm_Lx           , V(660F38,1A,_,x,_,0,4,T4 ), 0                         , 106, 0  , 3245 , 196, 64 ), // #787
  INST(Vbroadcastf32x8  , VexRm              , V(660F38,1B,_,2,_,0,5,T8 ), 0                         , 107, 0  , 3261 , 197, 62 ), // #788
  INST(Vbroadcastf64x2  , VexRm_Lx           , V(660F38,1A,_,x,_,1,4,T2 ), 0                         , 108, 0  , 3277 , 196, 115), // #789
  INST(Vbroadcastf64x4  , VexRm              , V(660F38,1B,_,2,_,1,5,T4 ), 0                         , 109, 0  , 3293 , 197, 64 ), // #790
  INST(Vbroadcasti128   , VexRm              , V(660F38,5A,_,1,0,_,_,_  ), 0                         , 104, 0  , 3309 , 194, 116), // #791
  INST(Vbroadcasti32x2  , VexRm_Lx           , V(660F38,59,_,x,_,0,3,T2 ), 0                         , 105, 0  , 3324 , 198, 115), // #792
  INST(Vbroadcasti32x4  , VexRm_Lx           , V(660F38,5A,_,x,_,0,4,T4 ), 0                         , 106, 0  , 3340 , 196, 112), // #793
  INST(Vbroadcasti32x8  , VexRm              , V(660F38,5B,_,2,_,0,5,T8 ), 0                         , 107, 0  , 3356 , 197, 62 ), // #794
  INST(Vbroadcasti64x2  , VexRm_Lx           , V(660F38,5A,_,x,_,1,4,T2 ), 0                         , 108, 0  , 3372 , 196, 115), // #795
  INST(Vbroadcasti64x4  , VexRm              , V(660F38,5B,_,2,_,1,5,T4 ), 0                         , 109, 0  , 3388 , 197, 64 ), // #796
  INST(Vbroadcastsd     , VexRm_Lx           , V(660F38,19,_,x,0,1,3,T1S), 0                         , 110, 0  , 3404 , 199, 117), // #797
  INST(Vbroadcastss     , VexRm_Lx           , V(660F38,18,_,x,0,0,2,T1S), 0                         , 111, 0  , 3417 , 200, 117), // #798
  INST(Vcmppd           , VexRvmi_Lx         , V(660F00,C2,_,x,I,1,4,FV ), 0                         , 93 , 0  , 3430 , 201, 107), // #799
  INST(Vcmpps           , VexRvmi_Lx         , V(000F00,C2,_,x,I,0,4,FV ), 0                         , 94 , 0  , 3437 , 202, 107), // #800
  INST(Vcmpsd           , VexRvmi            , V(F20F00,C2,_,I,I,1,3,T1S), 0                         , 95 , 0  , 3444 , 203, 108), // #801
  INST(Vcmpss           , VexRvmi            , V(F30F00,C2,_,I,I,0,2,T1S), 0                         , 96 , 0  , 3451 , 204, 108), // #802
  INST(Vcomisd          , VexRm              , V(660F00,2F,_,I,I,1,3,T1S), 0                         , 112, 0  , 3458 , 205, 118), // #803
  INST(Vcomiss          , VexRm              , V(000F00,2F,_,I,I,0,2,T1S), 0                         , 113, 0  , 3466 , 206, 118), // #804
  INST(Vcompresspd      , VexMr_Lx           , V(660F38,8A,_,x,_,1,3,T1S), 0                         , 110, 0  , 3474 , 207, 112), // #805
  INST(Vcompressps      , VexMr_Lx           , V(660F38,8A,_,x,_,0,2,T1S), 0                         , 111, 0  , 3486 , 207, 112), // #806
  INST(Vcvtdq2pd        , VexRm_Lx           , V(F30F00,E6,_,x,I,0,3,HV ), 0                         , 114, 0  , 3498 , 208, 107), // #807
  INST(Vcvtdq2ps        , VexRm_Lx           , V(000F00,5B,_,x,I,0,4,FV ), 0                         , 94 , 0  , 3508 , 209, 107), // #808
  INST(Vcvtne2ps2bf16   , VexRvm             , V(F20F38,72,_,_,_,0,_,_  ), 0                         , 77 , 0  , 3518 , 190, 119), // #809
  INST(Vcvtneps2bf16    , VexRm              , V(F30F38,72,_,_,_,0,_,_  ), 0                         , 82 , 0  , 3533 , 210, 119), // #810
  INST(Vcvtpd2dq        , VexRm_Lx           , V(F20F00,E6,_,x,I,1,4,FV ), 0                         , 115, 0  , 3547 , 211, 107), // #811
  INST(Vcvtpd2ps        , VexRm_Lx           , V(660F00,5A,_,x,I,1,4,FV ), 0                         , 93 , 0  , 3557 , 211, 107), // #812
  INST(Vcvtpd2qq        , VexRm_Lx           , V(660F00,7B,_,x,_,1,4,FV ), 0                         , 93 , 0  , 3567 , 212, 115), // #813
  INST(Vcvtpd2udq       , VexRm_Lx           , V(000F00,79,_,x,_,1,4,FV ), 0                         , 116, 0  , 3577 , 213, 112), // #814
  INST(Vcvtpd2uqq       , VexRm_Lx           , V(660F00,79,_,x,_,1,4,FV ), 0                         , 93 , 0  , 3588 , 212, 115), // #815
  INST(Vcvtph2ps        , VexRm_Lx           , V(660F38,13,_,x,0,0,3,HVM), 0                         , 117, 0  , 3599 , 214, 120), // #816
  INST(Vcvtps2dq        , VexRm_Lx           , V(660F00,5B,_,x,I,0,4,FV ), 0                         , 118, 0  , 3609 , 209, 107), // #817
  INST(Vcvtps2pd        , VexRm_Lx           , V(000F00,5A,_,x,I,0,4,HV ), 0                         , 119, 0  , 3619 , 215, 107), // #818
  INST(Vcvtps2ph        , VexMri_Lx          , V(660F3A,1D,_,x,0,0,3,HVM), 0                         , 120, 0  , 3629 , 216, 120), // #819
  INST(Vcvtps2qq        , VexRm_Lx           , V(660F00,7B,_,x,_,0,3,HV ), 0                         , 121, 0  , 3639 , 217, 115), // #820
  INST(Vcvtps2udq       , VexRm_Lx           , V(000F00,79,_,x,_,0,4,FV ), 0                         , 94 , 0  , 3649 , 218, 112), // #821
  INST(Vcvtps2uqq       , VexRm_Lx           , V(660F00,79,_,x,_,0,3,HV ), 0                         , 121, 0  , 3660 , 217, 115), // #822
  INST(Vcvtqq2pd        , VexRm_Lx           , V(F30F00,E6,_,x,_,1,4,FV ), 0                         , 122, 0  , 3671 , 212, 115), // #823
  INST(Vcvtqq2ps        , VexRm_Lx           , V(000F00,5B,_,x,_,1,4,FV ), 0                         , 116, 0  , 3681 , 213, 115), // #824
  INST(Vcvtsd2si        , VexRm_Wx           , V(F20F00,2D,_,I,x,x,3,T1F), 0                         , 123, 0  , 3691 , 219, 108), // #825
  INST(Vcvtsd2ss        , VexRvm             , V(F20F00,5A,_,I,I,1,3,T1S), 0                         , 95 , 0  , 3701 , 177, 108), // #826
  INST(Vcvtsd2usi       , VexRm_Wx           , V(F20F00,79,_,I,_,x,3,T1F), 0                         , 123, 0  , 3711 , 220, 64 ), // #827
  INST(Vcvtsi2sd        , VexRvm_Wx          , V(F20F00,2A,_,I,x,x,2,T1W), 0                         , 124, 0  , 3722 , 221, 108), // #828
  INST(Vcvtsi2ss        , VexRvm_Wx          , V(F30F00,2A,_,I,x,x,2,T1W), 0                         , 125, 0  , 3732 , 221, 108), // #829
  INST(Vcvtss2sd        , VexRvm             , V(F30F00,5A,_,I,I,0,2,T1S), 0                         , 96 , 0  , 3742 , 222, 108), // #830
  INST(Vcvtss2si        , VexRm_Wx           , V(F30F00,2D,_,I,x,x,2,T1F), 0                         , 126, 0  , 3752 , 223, 108), // #831
  INST(Vcvtss2usi       , VexRm_Wx           , V(F30F00,79,_,I,_,x,2,T1F), 0                         , 126, 0  , 3762 , 224, 64 ), // #832
  INST(Vcvttpd2dq       , VexRm_Lx           , V(660F00,E6,_,x,I,1,4,FV ), 0                         , 93 , 0  , 3773 , 225, 107), // #833
  INST(Vcvttpd2qq       , VexRm_Lx           , V(660F00,7A,_,x,_,1,4,FV ), 0                         , 93 , 0  , 3784 , 226, 112), // #834
  INST(Vcvttpd2udq      , VexRm_Lx           , V(000F00,78,_,x,_,1,4,FV ), 0                         , 116, 0  , 3795 , 227, 112), // #835
  INST(Vcvttpd2uqq      , VexRm_Lx           , V(660F00,78,_,x,_,1,4,FV ), 0                         , 93 , 0  , 3807 , 226, 115), // #836
  INST(Vcvttps2dq       , VexRm_Lx           , V(F30F00,5B,_,x,I,0,4,FV ), 0                         , 127, 0  , 3819 , 228, 107), // #837
  INST(Vcvttps2qq       , VexRm_Lx           , V(660F00,7A,_,x,_,0,3,HV ), 0                         , 121, 0  , 3830 , 229, 115), // #838
  INST(Vcvttps2udq      , VexRm_Lx           , V(000F00,78,_,x,_,0,4,FV ), 0                         , 94 , 0  , 3841 , 230, 112), // #839
  INST(Vcvttps2uqq      , VexRm_Lx           , V(660F00,78,_,x,_,0,3,HV ), 0                         , 121, 0  , 3853 , 229, 115), // #840
  INST(Vcvttsd2si       , VexRm_Wx           , V(F20F00,2C,_,I,x,x,3,T1F), 0                         , 123, 0  , 3865 , 231, 108), // #841
  INST(Vcvttsd2usi      , VexRm_Wx           , V(F20F00,78,_,I,_,x,3,T1F), 0                         , 123, 0  , 3876 , 232, 64 ), // #842
  INST(Vcvttss2si       , VexRm_Wx           , V(F30F00,2C,_,I,x,x,2,T1F), 0                         , 126, 0  , 3888 , 233, 108), // #843
  INST(Vcvttss2usi      , VexRm_Wx           , V(F30F00,78,_,I,_,x,2,T1F), 0                         , 126, 0  , 3899 , 234, 64 ), // #844
  INST(Vcvtudq2pd       , VexRm_Lx           , V(F30F00,7A,_,x,_,0,3,HV ), 0                         , 114, 0  , 3911 , 235, 112), // #845
  INST(Vcvtudq2ps       , VexRm_Lx           , V(F20F00,7A,_,x,_,0,4,FV ), 0                         , 128, 0  , 3922 , 218, 112), // #846
  INST(Vcvtuqq2pd       , VexRm_Lx           , V(F30F00,7A,_,x,_,1,4,FV ), 0                         , 122, 0  , 3933 , 212, 115), // #847
  INST(Vcvtuqq2ps       , VexRm_Lx           , V(F20F00,7A,_,x,_,1,4,FV ), 0                         , 115, 0  , 3944 , 213, 115), // #848
  INST(Vcvtusi2sd       , VexRvm_Wx          , V(F20F00,7B,_,I,_,x,2,T1W), 0                         , 124, 0  , 3955 , 236, 64 ), // #849
  INST(Vcvtusi2ss       , VexRvm_Wx          , V(F30F00,7B,_,I,_,x,2,T1W), 0                         , 125, 0  , 3966 , 236, 64 ), // #850
  INST(Vdbpsadbw        , VexRvmi_Lx         , V(660F3A,42,_,x,_,0,4,FVM), 0                         , 129, 0  , 3977 , 237, 114), // #851
  INST(Vdivpd           , VexRvm_Lx          , V(660F00,5E,_,x,I,1,4,FV ), 0                         , 93 , 0  , 3987 , 175, 107), // #852
  INST(Vdivps           , VexRvm_Lx          , V(000F00,5E,_,x,I,0,4,FV ), 0                         , 94 , 0  , 3994 , 176, 107), // #853
  INST(Vdivsd           , VexRvm             , V(F20F00,5E,_,I,I,1,3,T1S), 0                         , 95 , 0  , 4001 , 177, 108), // #854
  INST(Vdivss           , VexRvm             , V(F30F00,5E,_,I,I,0,2,T1S), 0                         , 96 , 0  , 4008 , 178, 108), // #855
  INST(Vdpbf16ps        , VexRvm             , V(F30F38,52,_,_,_,0,_,_  ), 0                         , 82 , 0  , 4015 , 190, 119), // #856
  INST(Vdppd            , VexRvmi_Lx         , V(660F3A,41,_,x,I,_,_,_  ), 0                         , 67 , 0  , 4025 , 238, 109), // #857
  INST(Vdpps            , VexRvmi_Lx         , V(660F3A,40,_,x,I,_,_,_  ), 0                         , 67 , 0  , 4031 , 192, 109), // #858
  INST(Verr             , X86M_NoSize        , O(000F00,00,4,_,_,_,_,_  ), 0                         , 89 , 0  , 4037 , 97 , 10 ), // #859
  INST(Verw             , X86M_NoSize        , O(000F00,00,5,_,_,_,_,_  ), 0                         , 70 , 0  , 4042 , 97 , 10 ), // #860
  INST(Vexp2pd          , VexRm              , V(660F38,C8,_,2,_,1,4,FV ), 0                         , 130, 0  , 4047 , 239, 121), // #861
  INST(Vexp2ps          , VexRm              , V(660F38,C8,_,2,_,0,4,FV ), 0                         , 131, 0  , 4055 , 240, 121), // #862
  INST(Vexpandpd        , VexRm_Lx           , V(660F38,88,_,x,_,1,3,T1S), 0                         , 110, 0  , 4063 , 241, 112), // #863
  INST(Vexpandps        , VexRm_Lx           , V(660F38,88,_,x,_,0,2,T1S), 0                         , 111, 0  , 4073 , 241, 112), // #864
  INST(Vextractf128     , VexMri             , V(660F3A,19,_,1,0,_,_,_  ), 0                         , 132, 0  , 4083 , 242, 109), // #865
  INST(Vextractf32x4    , VexMri_Lx          , V(660F3A,19,_,x,_,0,4,T4 ), 0                         , 133, 0  , 4096 , 243, 112), // #866
  INST(Vextractf32x8    , VexMri             , V(660F3A,1B,_,2,_,0,5,T8 ), 0                         , 134, 0  , 4110 , 244, 62 ), // #867
  INST(Vextractf64x2    , VexMri_Lx          , V(660F3A,19,_,x,_,1,4,T2 ), 0                         , 135, 0  , 4124 , 243, 115), // #868
  INST(Vextractf64x4    , VexMri             , V(660F3A,1B,_,2,_,1,5,T4 ), 0                         , 136, 0  , 4138 , 244, 64 ), // #869
  INST(Vextracti128     , VexMri             , V(660F3A,39,_,1,0,_,_,_  ), 0                         , 132, 0  , 4152 , 242, 116), // #870
  INST(Vextracti32x4    , VexMri_Lx          , V(660F3A,39,_,x,_,0,4,T4 ), 0                         , 133, 0  , 4165 , 243, 112), // #871
  INST(Vextracti32x8    , VexMri             , V(660F3A,3B,_,2,_,0,5,T8 ), 0                         , 134, 0  , 4179 , 244, 62 ), // #872
  INST(Vextracti64x2    , VexMri_Lx          , V(660F3A,39,_,x,_,1,4,T2 ), 0                         , 135, 0  , 4193 , 243, 115), // #873
  INST(Vextracti64x4    , VexMri             , V(660F3A,3B,_,2,_,1,5,T4 ), 0                         , 136, 0  , 4207 , 244, 64 ), // #874
  INST(Vextractps       , VexMri             , V(660F3A,17,_,0,I,I,2,T1S), 0                         , 137, 0  , 4221 , 245, 108), // #875
  INST(Vfixupimmpd      , VexRvmi_Lx         , V(660F3A,54,_,x,_,1,4,FV ), 0                         , 100, 0  , 4232 , 246, 112), // #876
  INST(Vfixupimmps      , VexRvmi_Lx         , V(660F3A,54,_,x,_,0,4,FV ), 0                         , 99 , 0  , 4244 , 247, 112), // #877
  INST(Vfixupimmsd      , VexRvmi            , V(660F3A,55,_,I,_,1,3,T1S), 0                         , 138, 0  , 4256 , 248, 64 ), // #878
  INST(Vfixupimmss      , VexRvmi            , V(660F3A,55,_,I,_,0,2,T1S), 0                         , 137, 0  , 4268 , 249, 64 ), // #879
  INST(Vfmadd132pd      , VexRvm_Lx          , V(660F38,98,_,x,1,1,4,FV ), 0                         , 139, 0  , 4280 , 175, 122), // #880
  INST(Vfmadd132ps      , VexRvm_Lx          , V(660F38,98,_,x,0,0,4,FV ), 0                         , 101, 0  , 4292 , 176, 122), // #881
  INST(Vfmadd132sd      , VexRvm             , V(660F38,99,_,I,1,1,3,T1S), 0                         , 140, 0  , 4304 , 177, 123), // #882
  INST(Vfmadd132ss      , VexRvm             , V(660F38,99,_,I,0,0,2,T1S), 0                         , 111, 0  , 4316 , 178, 123), // #883
  INST(Vfmadd213pd      , VexRvm_Lx          , V(660F38,A8,_,x,1,1,4,FV ), 0                         , 139, 0  , 4328 , 175, 122), // #884
  INST(Vfmadd213ps      , VexRvm_Lx          , V(660F38,A8,_,x,0,0,4,FV ), 0                         , 101, 0  , 4340 , 176, 122), // #885
  INST(Vfmadd213sd      , VexRvm             , V(660F38,A9,_,I,1,1,3,T1S), 0                         , 140, 0  , 4352 , 177, 123), // #886
  INST(Vfmadd213ss      , VexRvm             , V(660F38,A9,_,I,0,0,2,T1S), 0                         , 111, 0  , 4364 , 178, 123), // #887
  INST(Vfmadd231pd      , VexRvm_Lx          , V(660F38,B8,_,x,1,1,4,FV ), 0                         , 139, 0  , 4376 , 175, 122), // #888
  INST(Vfmadd231ps      , VexRvm_Lx          , V(660F38,B8,_,x,0,0,4,FV ), 0                         , 101, 0  , 4388 , 176, 122), // #889
  INST(Vfmadd231sd      , VexRvm             , V(660F38,B9,_,I,1,1,3,T1S), 0                         , 140, 0  , 4400 , 177, 123), // #890
  INST(Vfmadd231ss      , VexRvm             , V(660F38,B9,_,I,0,0,2,T1S), 0                         , 111, 0  , 4412 , 178, 123), // #891
  INST(Vfmaddpd         , Fma4_Lx            , V(660F3A,69,_,x,x,_,_,_  ), 0                         , 67 , 0  , 4424 , 250, 124), // #892
  INST(Vfmaddps         , Fma4_Lx            , V(660F3A,68,_,x,x,_,_,_  ), 0                         , 67 , 0  , 4433 , 250, 124), // #893
  INST(Vfmaddsd         , Fma4               , V(660F3A,6B,_,0,x,_,_,_  ), 0                         , 67 , 0  , 4442 , 251, 124), // #894
  INST(Vfmaddss         , Fma4               , V(660F3A,6A,_,0,x,_,_,_  ), 0                         , 67 , 0  , 4451 , 252, 124), // #895
  INST(Vfmaddsub132pd   , VexRvm_Lx          , V(660F38,96,_,x,1,1,4,FV ), 0                         , 139, 0  , 4460 , 175, 122), // #896
  INST(Vfmaddsub132ps   , VexRvm_Lx          , V(660F38,96,_,x,0,0,4,FV ), 0                         , 101, 0  , 4475 , 176, 122), // #897
  INST(Vfmaddsub213pd   , VexRvm_Lx          , V(660F38,A6,_,x,1,1,4,FV ), 0                         , 139, 0  , 4490 , 175, 122), // #898
  INST(Vfmaddsub213ps   , VexRvm_Lx          , V(660F38,A6,_,x,0,0,4,FV ), 0                         , 101, 0  , 4505 , 176, 122), // #899
  INST(Vfmaddsub231pd   , VexRvm_Lx          , V(660F38,B6,_,x,1,1,4,FV ), 0                         , 139, 0  , 4520 , 175, 122), // #900
  INST(Vfmaddsub231ps   , VexRvm_Lx          , V(660F38,B6,_,x,0,0,4,FV ), 0                         , 101, 0  , 4535 , 176, 122), // #901
  INST(Vfmaddsubpd      , Fma4_Lx            , V(660F3A,5D,_,x,x,_,_,_  ), 0                         , 67 , 0  , 4550 , 250, 124), // #902
  INST(Vfmaddsubps      , Fma4_Lx            , V(660F3A,5C,_,x,x,_,_,_  ), 0                         , 67 , 0  , 4562 , 250, 124), // #903
  INST(Vfmsub132pd      , VexRvm_Lx          , V(660F38,9A,_,x,1,1,4,FV ), 0                         , 139, 0  , 4574 , 175, 122), // #904
  INST(Vfmsub132ps      , VexRvm_Lx          , V(660F38,9A,_,x,0,0,4,FV ), 0                         , 101, 0  , 4586 , 176, 122), // #905
  INST(Vfmsub132sd      , VexRvm             , V(660F38,9B,_,I,1,1,3,T1S), 0                         , 140, 0  , 4598 , 177, 123), // #906
  INST(Vfmsub132ss      , VexRvm             , V(660F38,9B,_,I,0,0,2,T1S), 0                         , 111, 0  , 4610 , 178, 123), // #907
  INST(Vfmsub213pd      , VexRvm_Lx          , V(660F38,AA,_,x,1,1,4,FV ), 0                         , 139, 0  , 4622 , 175, 122), // #908
  INST(Vfmsub213ps      , VexRvm_Lx          , V(660F38,AA,_,x,0,0,4,FV ), 0                         , 101, 0  , 4634 , 176, 122), // #909
  INST(Vfmsub213sd      , VexRvm             , V(660F38,AB,_,I,1,1,3,T1S), 0                         , 140, 0  , 4646 , 177, 123), // #910
  INST(Vfmsub213ss      , VexRvm             , V(660F38,AB,_,I,0,0,2,T1S), 0                         , 111, 0  , 4658 , 178, 123), // #911
  INST(Vfmsub231pd      , VexRvm_Lx          , V(660F38,BA,_,x,1,1,4,FV ), 0                         , 139, 0  , 4670 , 175, 122), // #912
  INST(Vfmsub231ps      , VexRvm_Lx          , V(660F38,BA,_,x,0,0,4,FV ), 0                         , 101, 0  , 4682 , 176, 122), // #913
  INST(Vfmsub231sd      , VexRvm             , V(660F38,BB,_,I,1,1,3,T1S), 0                         , 140, 0  , 4694 , 177, 123), // #914
  INST(Vfmsub231ss      , VexRvm             , V(660F38,BB,_,I,0,0,2,T1S), 0                         , 111, 0  , 4706 , 178, 123), // #915
  INST(Vfmsubadd132pd   , VexRvm_Lx          , V(660F38,97,_,x,1,1,4,FV ), 0                         , 139, 0  , 4718 , 175, 122), // #916
  INST(Vfmsubadd132ps   , VexRvm_Lx          , V(660F38,97,_,x,0,0,4,FV ), 0                         , 101, 0  , 4733 , 176, 122), // #917
  INST(Vfmsubadd213pd   , VexRvm_Lx          , V(660F38,A7,_,x,1,1,4,FV ), 0                         , 139, 0  , 4748 , 175, 122), // #918
  INST(Vfmsubadd213ps   , VexRvm_Lx          , V(660F38,A7,_,x,0,0,4,FV ), 0                         , 101, 0  , 4763 , 176, 122), // #919
  INST(Vfmsubadd231pd   , VexRvm_Lx          , V(660F38,B7,_,x,1,1,4,FV ), 0                         , 139, 0  , 4778 , 175, 122), // #920
  INST(Vfmsubadd231ps   , VexRvm_Lx          , V(660F38,B7,_,x,0,0,4,FV ), 0                         , 101, 0  , 4793 , 176, 122), // #921
  INST(Vfmsubaddpd      , Fma4_Lx            , V(660F3A,5F,_,x,x,_,_,_  ), 0                         , 67 , 0  , 4808 , 250, 124), // #922
  INST(Vfmsubaddps      , Fma4_Lx            , V(660F3A,5E,_,x,x,_,_,_  ), 0                         , 67 , 0  , 4820 , 250, 124), // #923
  INST(Vfmsubpd         , Fma4_Lx            , V(660F3A,6D,_,x,x,_,_,_  ), 0                         , 67 , 0  , 4832 , 250, 124), // #924
  INST(Vfmsubps         , Fma4_Lx            , V(660F3A,6C,_,x,x,_,_,_  ), 0                         , 67 , 0  , 4841 , 250, 124), // #925
  INST(Vfmsubsd         , Fma4               , V(660F3A,6F,_,0,x,_,_,_  ), 0                         , 67 , 0  , 4850 , 251, 124), // #926
  INST(Vfmsubss         , Fma4               , V(660F3A,6E,_,0,x,_,_,_  ), 0                         , 67 , 0  , 4859 , 252, 124), // #927
  INST(Vfnmadd132pd     , VexRvm_Lx          , V(660F38,9C,_,x,1,1,4,FV ), 0                         , 139, 0  , 4868 , 175, 122), // #928
  INST(Vfnmadd132ps     , VexRvm_Lx          , V(660F38,9C,_,x,0,0,4,FV ), 0                         , 101, 0  , 4881 , 176, 122), // #929
  INST(Vfnmadd132sd     , VexRvm             , V(660F38,9D,_,I,1,1,3,T1S), 0                         , 140, 0  , 4894 , 177, 123), // #930
  INST(Vfnmadd132ss     , VexRvm             , V(660F38,9D,_,I,0,0,2,T1S), 0                         , 111, 0  , 4907 , 178, 123), // #931
  INST(Vfnmadd213pd     , VexRvm_Lx          , V(660F38,AC,_,x,1,1,4,FV ), 0                         , 139, 0  , 4920 , 175, 122), // #932
  INST(Vfnmadd213ps     , VexRvm_Lx          , V(660F38,AC,_,x,0,0,4,FV ), 0                         , 101, 0  , 4933 , 176, 122), // #933
  INST(Vfnmadd213sd     , VexRvm             , V(660F38,AD,_,I,1,1,3,T1S), 0                         , 140, 0  , 4946 , 177, 123), // #934
  INST(Vfnmadd213ss     , VexRvm             , V(660F38,AD,_,I,0,0,2,T1S), 0                         , 111, 0  , 4959 , 178, 123), // #935
  INST(Vfnmadd231pd     , VexRvm_Lx          , V(660F38,BC,_,x,1,1,4,FV ), 0                         , 139, 0  , 4972 , 175, 122), // #936
  INST(Vfnmadd231ps     , VexRvm_Lx          , V(660F38,BC,_,x,0,0,4,FV ), 0                         , 101, 0  , 4985 , 176, 122), // #937
  INST(Vfnmadd231sd     , VexRvm             , V(660F38,BC,_,I,1,1,3,T1S), 0                         , 140, 0  , 4998 , 177, 123), // #938
  INST(Vfnmadd231ss     , VexRvm             , V(660F38,BC,_,I,0,0,2,T1S), 0                         , 111, 0  , 5011 , 178, 123), // #939
  INST(Vfnmaddpd        , Fma4_Lx            , V(660F3A,79,_,x,x,_,_,_  ), 0                         , 67 , 0  , 5024 , 250, 124), // #940
  INST(Vfnmaddps        , Fma4_Lx            , V(660F3A,78,_,x,x,_,_,_  ), 0                         , 67 , 0  , 5034 , 250, 124), // #941
  INST(Vfnmaddsd        , Fma4               , V(660F3A,7B,_,0,x,_,_,_  ), 0                         , 67 , 0  , 5044 , 251, 124), // #942
  INST(Vfnmaddss        , Fma4               , V(660F3A,7A,_,0,x,_,_,_  ), 0                         , 67 , 0  , 5054 , 252, 124), // #943
  INST(Vfnmsub132pd     , VexRvm_Lx          , V(660F38,9E,_,x,1,1,4,FV ), 0                         , 139, 0  , 5064 , 175, 122), // #944
  INST(Vfnmsub132ps     , VexRvm_Lx          , V(660F38,9E,_,x,0,0,4,FV ), 0                         , 101, 0  , 5077 , 176, 122), // #945
  INST(Vfnmsub132sd     , VexRvm             , V(660F38,9F,_,I,1,1,3,T1S), 0                         , 140, 0  , 5090 , 177, 123), // #946
  INST(Vfnmsub132ss     , VexRvm             , V(660F38,9F,_,I,0,0,2,T1S), 0                         , 111, 0  , 5103 , 178, 123), // #947
  INST(Vfnmsub213pd     , VexRvm_Lx          , V(660F38,AE,_,x,1,1,4,FV ), 0                         , 139, 0  , 5116 , 175, 122), // #948
  INST(Vfnmsub213ps     , VexRvm_Lx          , V(660F38,AE,_,x,0,0,4,FV ), 0                         , 101, 0  , 5129 , 176, 122), // #949
  INST(Vfnmsub213sd     , VexRvm             , V(660F38,AF,_,I,1,1,3,T1S), 0                         , 140, 0  , 5142 , 177, 123), // #950
  INST(Vfnmsub213ss     , VexRvm             , V(660F38,AF,_,I,0,0,2,T1S), 0                         , 111, 0  , 5155 , 178, 123), // #951
  INST(Vfnmsub231pd     , VexRvm_Lx          , V(660F38,BE,_,x,1,1,4,FV ), 0                         , 139, 0  , 5168 , 175, 122), // #952
  INST(Vfnmsub231ps     , VexRvm_Lx          , V(660F38,BE,_,x,0,0,4,FV ), 0                         , 101, 0  , 5181 , 176, 122), // #953
  INST(Vfnmsub231sd     , VexRvm             , V(660F38,BF,_,I,1,1,3,T1S), 0                         , 140, 0  , 5194 , 177, 123), // #954
  INST(Vfnmsub231ss     , VexRvm             , V(660F38,BF,_,I,0,0,2,T1S), 0                         , 111, 0  , 5207 , 178, 123), // #955
  INST(Vfnmsubpd        , Fma4_Lx            , V(660F3A,7D,_,x,x,_,_,_  ), 0                         , 67 , 0  , 5220 , 250, 124), // #956
  INST(Vfnmsubps        , Fma4_Lx            , V(660F3A,7C,_,x,x,_,_,_  ), 0                         , 67 , 0  , 5230 , 250, 124), // #957
  INST(Vfnmsubsd        , Fma4               , V(660F3A,7F,_,0,x,_,_,_  ), 0                         , 67 , 0  , 5240 , 251, 124), // #958
  INST(Vfnmsubss        , Fma4               , V(660F3A,7E,_,0,x,_,_,_  ), 0                         , 67 , 0  , 5250 , 252, 124), // #959
  INST(Vfpclasspd       , VexRmi_Lx          , V(660F3A,66,_,x,_,1,4,FV ), 0                         , 100, 0  , 5260 , 253, 115), // #960
  INST(Vfpclassps       , VexRmi_Lx          , V(660F3A,66,_,x,_,0,4,FV ), 0                         , 99 , 0  , 5271 , 254, 115), // #961
  INST(Vfpclasssd       , VexRmi_Lx          , V(660F3A,67,_,I,_,1,3,T1S), 0                         , 138, 0  , 5282 , 255, 62 ), // #962
  INST(Vfpclassss       , VexRmi_Lx          , V(660F3A,67,_,I,_,0,2,T1S), 0                         , 137, 0  , 5293 , 256, 62 ), // #963
  INST(Vfrczpd          , VexRm_Lx           , V(XOP_M9,81,_,x,0,_,_,_  ), 0                         , 72 , 0  , 5304 , 257, 125), // #964
  INST(Vfrczps          , VexRm_Lx           , V(XOP_M9,80,_,x,0,_,_,_  ), 0                         , 72 , 0  , 5312 , 257, 125), // #965
  INST(Vfrczsd          , VexRm              , V(XOP_M9,83,_,0,0,_,_,_  ), 0                         , 72 , 0  , 5320 , 258, 125), // #966
  INST(Vfrczss          , VexRm              , V(XOP_M9,82,_,0,0,_,_,_  ), 0                         , 72 , 0  , 5328 , 259, 125), // #967
  INST(Vgatherdpd       , VexRmvRm_VM        , V(660F38,92,_,x,1,_,_,_  ), V(660F38,92,_,x,_,1,3,T1S), 141, 79 , 5336 , 260, 126), // #968
  INST(Vgatherdps       , VexRmvRm_VM        , V(660F38,92,_,x,0,_,_,_  ), V(660F38,92,_,x,_,0,2,T1S), 88 , 80 , 5347 , 261, 126), // #969
  INST(Vgatherpf0dpd    , VexM_VM            , V(660F38,C6,1,2,_,1,3,T1S), 0                         , 142, 0  , 5358 , 262, 127), // #970
  INST(Vgatherpf0dps    , VexM_VM            , V(660F38,C6,1,2,_,0,2,T1S), 0                         , 143, 0  , 5372 , 263, 127), // #971
  INST(Vgatherpf0qpd    , VexM_VM            , V(660F38,C7,1,2,_,1,3,T1S), 0                         , 142, 0  , 5386 , 264, 127), // #972
  INST(Vgatherpf0qps    , VexM_VM            , V(660F38,C7,1,2,_,0,2,T1S), 0                         , 143, 0  , 5400 , 264, 127), // #973
  INST(Vgatherpf1dpd    , VexM_VM            , V(660F38,C6,2,2,_,1,3,T1S), 0                         , 144, 0  , 5414 , 262, 127), // #974
  INST(Vgatherpf1dps    , VexM_VM            , V(660F38,C6,2,2,_,0,2,T1S), 0                         , 145, 0  , 5428 , 263, 127), // #975
  INST(Vgatherpf1qpd    , VexM_VM            , V(660F38,C7,2,2,_,1,3,T1S), 0                         , 144, 0  , 5442 , 264, 127), // #976
  INST(Vgatherpf1qps    , VexM_VM            , V(660F38,C7,2,2,_,0,2,T1S), 0                         , 145, 0  , 5456 , 264, 127), // #977
  INST(Vgatherqpd       , VexRmvRm_VM        , V(660F38,93,_,x,1,_,_,_  ), V(660F38,93,_,x,_,1,3,T1S), 141, 81 , 5470 , 265, 126), // #978
  INST(Vgatherqps       , VexRmvRm_VM        , V(660F38,93,_,x,0,_,_,_  ), V(660F38,93,_,x,_,0,2,T1S), 88 , 82 , 5481 , 266, 126), // #979
  INST(Vgetexppd        , VexRm_Lx           , V(660F38,42,_,x,_,1,4,FV ), 0                         , 102, 0  , 5492 , 226, 112), // #980
  INST(Vgetexpps        , VexRm_Lx           , V(660F38,42,_,x,_,0,4,FV ), 0                         , 101, 0  , 5502 , 230, 112), // #981
  INST(Vgetexpsd        , VexRvm             , V(660F38,43,_,I,_,1,3,T1S), 0                         , 110, 0  , 5512 , 267, 64 ), // #982
  INST(Vgetexpss        , VexRvm             , V(660F38,43,_,I,_,0,2,T1S), 0                         , 111, 0  , 5522 , 268, 64 ), // #983
  INST(Vgetmantpd       , VexRmi_Lx          , V(660F3A,26,_,x,_,1,4,FV ), 0                         , 100, 0  , 5532 , 269, 112), // #984
  INST(Vgetmantps       , VexRmi_Lx          , V(660F3A,26,_,x,_,0,4,FV ), 0                         , 99 , 0  , 5543 , 270, 112), // #985
  INST(Vgetmantsd       , VexRvmi            , V(660F3A,27,_,I,_,1,3,T1S), 0                         , 138, 0  , 5554 , 248, 64 ), // #986
  INST(Vgetmantss       , VexRvmi            , V(660F3A,27,_,I,_,0,2,T1S), 0                         , 137, 0  , 5565 , 249, 64 ), // #987
  INST(Vgf2p8affineinvqb, VexRvmi_Lx         , V(660F3A,CF,_,x,1,1,_,FV ), 0                         , 146, 0  , 5576 , 271, 128), // #988
  INST(Vgf2p8affineqb   , VexRvmi_Lx         , V(660F3A,CE,_,x,1,1,_,FV ), 0                         , 146, 0  , 5594 , 271, 128), // #989
  INST(Vgf2p8mulb       , VexRvm_Lx          , V(660F38,CF,_,x,0,0,_,FV ), 0                         , 147, 0  , 5609 , 272, 128), // #990
  INST(Vhaddpd          , VexRvm_Lx          , V(660F00,7C,_,x,I,_,_,_  ), 0                         , 63 , 0  , 5620 , 179, 109), // #991
  INST(Vhaddps          , VexRvm_Lx          , V(F20F00,7C,_,x,I,_,_,_  ), 0                         , 97 , 0  , 5628 , 179, 109), // #992
  INST(Vhsubpd          , VexRvm_Lx          , V(660F00,7D,_,x,I,_,_,_  ), 0                         , 63 , 0  , 5636 , 179, 109), // #993
  INST(Vhsubps          , VexRvm_Lx          , V(F20F00,7D,_,x,I,_,_,_  ), 0                         , 97 , 0  , 5644 , 179, 109), // #994
  INST(Vinsertf128      , VexRvmi            , V(660F3A,18,_,1,0,_,_,_  ), 0                         , 132, 0  , 5652 , 273, 109), // #995
  INST(Vinsertf32x4     , VexRvmi_Lx         , V(660F3A,18,_,x,_,0,4,T4 ), 0                         , 133, 0  , 5664 , 274, 112), // #996
  INST(Vinsertf32x8     , VexRvmi            , V(660F3A,1A,_,2,_,0,5,T8 ), 0                         , 134, 0  , 5677 , 275, 62 ), // #997
  INST(Vinsertf64x2     , VexRvmi_Lx         , V(660F3A,18,_,x,_,1,4,T2 ), 0                         , 135, 0  , 5690 , 274, 115), // #998
  INST(Vinsertf64x4     , VexRvmi            , V(660F3A,1A,_,2,_,1,5,T4 ), 0                         , 136, 0  , 5703 , 275, 64 ), // #999
  INST(Vinserti128      , VexRvmi            , V(660F3A,38,_,1,0,_,_,_  ), 0                         , 132, 0  , 5716 , 273, 116), // #1000
  INST(Vinserti32x4     , VexRvmi_Lx         , V(660F3A,38,_,x,_,0,4,T4 ), 0                         , 133, 0  , 5728 , 274, 112), // #1001
  INST(Vinserti32x8     , VexRvmi            , V(660F3A,3A,_,2,_,0,5,T8 ), 0                         , 134, 0  , 5741 , 275, 62 ), // #1002
  INST(Vinserti64x2     , VexRvmi_Lx         , V(660F3A,38,_,x,_,1,4,T2 ), 0                         , 135, 0  , 5754 , 274, 115), // #1003
  INST(Vinserti64x4     , VexRvmi            , V(660F3A,3A,_,2,_,1,5,T4 ), 0                         , 136, 0  , 5767 , 275, 64 ), // #1004
  INST(Vinsertps        , VexRvmi            , V(660F3A,21,_,0,I,0,2,T1S), 0                         , 137, 0  , 5780 , 276, 108), // #1005
  INST(Vlddqu           , VexRm_Lx           , V(F20F00,F0,_,x,I,_,_,_  ), 0                         , 97 , 0  , 5790 , 277, 109), // #1006
  INST(Vldmxcsr         , VexM               , V(000F00,AE,2,0,I,_,_,_  ), 0                         , 148, 0  , 5797 , 278, 109), // #1007
  INST(Vmaskmovdqu      , VexRm_ZDI          , V(660F00,F7,_,0,I,_,_,_  ), 0                         , 63 , 0  , 5806 , 279, 109), // #1008
  INST(Vmaskmovpd       , VexRvmMvr_Lx       , V(660F38,2D,_,x,0,_,_,_  ), V(660F38,2F,_,x,0,_,_,_  ), 88 , 83 , 5818 , 280, 109), // #1009
  INST(Vmaskmovps       , VexRvmMvr_Lx       , V(660F38,2C,_,x,0,_,_,_  ), V(660F38,2E,_,x,0,_,_,_  ), 88 , 84 , 5829 , 280, 109), // #1010
  INST(Vmaxpd           , VexRvm_Lx          , V(660F00,5F,_,x,I,1,4,FV ), 0                         , 93 , 0  , 5840 , 281, 107), // #1011
  INST(Vmaxps           , VexRvm_Lx          , V(000F00,5F,_,x,I,0,4,FV ), 0                         , 94 , 0  , 5847 , 282, 107), // #1012
  INST(Vmaxsd           , VexRvm             , V(F20F00,5F,_,I,I,1,3,T1S), 0                         , 95 , 0  , 5854 , 283, 107), // #1013
  INST(Vmaxss           , VexRvm             , V(F30F00,5F,_,I,I,0,2,T1S), 0                         , 96 , 0  , 5861 , 222, 107), // #1014
  INST(Vmcall           , X86Op              , O(000F01,C1,_,_,_,_,_,_  ), 0                         , 21 , 0  , 5868 , 30 , 54 ), // #1015
  INST(Vmclear          , X86M_Only          , O(660F00,C7,6,_,_,_,_,_  ), 0                         , 24 , 0  , 5875 , 284, 54 ), // #1016
  INST(Vmfunc           , X86Op              , O(000F01,D4,_,_,_,_,_,_  ), 0                         , 21 , 0  , 5883 , 30 , 54 ), // #1017
  INST(Vminpd           , VexRvm_Lx          , V(660F00,5D,_,x,I,1,4,FV ), 0                         , 93 , 0  , 5890 , 281, 107), // #1018
  INST(Vminps           , VexRvm_Lx          , V(000F00,5D,_,x,I,0,4,FV ), 0                         , 94 , 0  , 5897 , 282, 107), // #1019
  INST(Vminsd           , VexRvm             , V(F20F00,5D,_,I,I,1,3,T1S), 0                         , 95 , 0  , 5904 , 283, 107), // #1020
  INST(Vminss           , VexRvm             , V(F30F00,5D,_,I,I,0,2,T1S), 0                         , 96 , 0  , 5911 , 222, 107), // #1021
  INST(Vmlaunch         , X86Op              , O(000F01,C2,_,_,_,_,_,_  ), 0                         , 21 , 0  , 5918 , 30 , 54 ), // #1022
  INST(Vmload           , X86Op_xAX          , O(000F01,DA,_,_,_,_,_,_  ), 0                         , 21 , 0  , 5927 , 285, 23 ), // #1023
  INST(Vmmcall          , X86Op              , O(000F01,D9,_,_,_,_,_,_  ), 0                         , 21 , 0  , 5934 , 30 , 23 ), // #1024
  INST(Vmovapd          , VexRmMr_Lx         , V(660F00,28,_,x,I,1,4,FVM), V(660F00,29,_,x,I,1,4,FVM), 149, 85 , 5942 , 286, 107), // #1025
  INST(Vmovaps          , VexRmMr_Lx         , V(000F00,28,_,x,I,0,4,FVM), V(000F00,29,_,x,I,0,4,FVM), 150, 86 , 5950 , 286, 107), // #1026
  INST(Vmovd            , VexMovdMovq        , V(660F00,6E,_,0,0,0,2,T1S), V(660F00,7E,_,0,0,0,2,T1S), 151, 87 , 5958 , 287, 108), // #1027
  INST(Vmovddup         , VexRm_Lx           , V(F20F00,12,_,x,I,1,3,DUP), 0                         , 152, 0  , 5964 , 288, 107), // #1028
  INST(Vmovdqa          , VexRmMr_Lx         , V(660F00,6F,_,x,I,_,_,_  ), V(660F00,7F,_,x,I,_,_,_  ), 63 , 88 , 5973 , 289, 109), // #1029
  INST(Vmovdqa32        , VexRmMr_Lx         , V(660F00,6F,_,x,_,0,4,FVM), V(660F00,7F,_,x,_,0,4,FVM), 153, 89 , 5981 , 290, 112), // #1030
  INST(Vmovdqa64        , VexRmMr_Lx         , V(660F00,6F,_,x,_,1,4,FVM), V(660F00,7F,_,x,_,1,4,FVM), 149, 90 , 5991 , 290, 112), // #1031
  INST(Vmovdqu          , VexRmMr_Lx         , V(F30F00,6F,_,x,I,_,_,_  ), V(F30F00,7F,_,x,I,_,_,_  ), 154, 91 , 6001 , 289, 109), // #1032
  INST(Vmovdqu16        , VexRmMr_Lx         , V(F20F00,6F,_,x,_,1,4,FVM), V(F20F00,7F,_,x,_,1,4,FVM), 155, 92 , 6009 , 290, 114), // #1033
  INST(Vmovdqu32        , VexRmMr_Lx         , V(F30F00,6F,_,x,_,0,4,FVM), V(F30F00,7F,_,x,_,0,4,FVM), 156, 93 , 6019 , 290, 112), // #1034
  INST(Vmovdqu64        , VexRmMr_Lx         , V(F30F00,6F,_,x,_,1,4,FVM), V(F30F00,7F,_,x,_,1,4,FVM), 157, 94 , 6029 , 290, 112), // #1035
  INST(Vmovdqu8         , VexRmMr_Lx         , V(F20F00,6F,_,x,_,0,4,FVM), V(F20F00,7F,_,x,_,0,4,FVM), 158, 95 , 6039 , 290, 114), // #1036
  INST(Vmovhlps         , VexRvm             , V(000F00,12,_,0,I,0,_,_  ), 0                         , 66 , 0  , 6048 , 291, 108), // #1037
  INST(Vmovhpd          , VexRvmMr           , V(660F00,16,_,0,I,1,3,T1S), V(660F00,17,_,0,I,1,3,T1S), 112, 96 , 6057 , 292, 108), // #1038
  INST(Vmovhps          , VexRvmMr           , V(000F00,16,_,0,I,0,3,T2 ), V(000F00,17,_,0,I,0,3,T2 ), 159, 97 , 6065 , 292, 108), // #1039
  INST(Vmovlhps         , VexRvm             , V(000F00,16,_,0,I,0,_,_  ), 0                         , 66 , 0  , 6073 , 291, 108), // #1040
  INST(Vmovlpd          , VexRvmMr           , V(660F00,12,_,0,I,1,3,T1S), V(660F00,13,_,0,I,1,3,T1S), 112, 98 , 6082 , 292, 108), // #1041
  INST(Vmovlps          , VexRvmMr           , V(000F00,12,_,0,I,0,3,T2 ), V(000F00,13,_,0,I,0,3,T2 ), 159, 99 , 6090 , 292, 108), // #1042
  INST(Vmovmskpd        , VexRm_Lx           , V(660F00,50,_,x,I,_,_,_  ), 0                         , 63 , 0  , 6098 , 293, 109), // #1043
  INST(Vmovmskps        , VexRm_Lx           , V(000F00,50,_,x,I,_,_,_  ), 0                         , 66 , 0  , 6108 , 293, 109), // #1044
  INST(Vmovntdq         , VexMr_Lx           , V(660F00,E7,_,x,I,0,4,FVM), 0                         , 153, 0  , 6118 , 294, 107), // #1045
  INST(Vmovntdqa        , VexRm_Lx           , V(660F38,2A,_,x,I,0,4,FVM), 0                         , 98 , 0  , 6127 , 295, 117), // #1046
  INST(Vmovntpd         , VexMr_Lx           , V(660F00,2B,_,x,I,1,4,FVM), 0                         , 149, 0  , 6137 , 294, 107), // #1047
  INST(Vmovntps         , VexMr_Lx           , V(000F00,2B,_,x,I,0,4,FVM), 0                         , 150, 0  , 6146 , 294, 107), // #1048
  INST(Vmovq            , VexMovdMovq        , V(660F00,6E,_,0,I,1,3,T1S), V(660F00,7E,_,0,I,1,3,T1S), 112, 100, 6155 , 296, 108), // #1049
  INST(Vmovsd           , VexMovssMovsd      , V(F20F00,10,_,I,I,1,3,T1S), V(F20F00,11,_,I,I,1,3,T1S), 95 , 101, 6161 , 297, 108), // #1050
  INST(Vmovshdup        , VexRm_Lx           , V(F30F00,16,_,x,I,0,4,FVM), 0                         , 156, 0  , 6168 , 298, 107), // #1051
  INST(Vmovsldup        , VexRm_Lx           , V(F30F00,12,_,x,I,0,4,FVM), 0                         , 156, 0  , 6178 , 298, 107), // #1052
  INST(Vmovss           , VexMovssMovsd      , V(F30F00,10,_,I,I,0,2,T1S), V(F30F00,11,_,I,I,0,2,T1S), 96 , 102, 6188 , 299, 108), // #1053
  INST(Vmovupd          , VexRmMr_Lx         , V(660F00,10,_,x,I,1,4,FVM), V(660F00,11,_,x,I,1,4,FVM), 149, 103, 6195 , 286, 107), // #1054
  INST(Vmovups          , VexRmMr_Lx         , V(000F00,10,_,x,I,0,4,FVM), V(000F00,11,_,x,I,0,4,FVM), 150, 104, 6203 , 286, 107), // #1055
  INST(Vmpsadbw         , VexRvmi_Lx         , V(660F3A,42,_,x,I,_,_,_  ), 0                         , 67 , 0  , 6211 , 192, 129), // #1056
  INST(Vmptrld          , X86M_Only          , O(000F00,C7,6,_,_,_,_,_  ), 0                         , 73 , 0  , 6220 , 284, 54 ), // #1057
  INST(Vmptrst          , X86M_Only          , O(000F00,C7,7,_,_,_,_,_  ), 0                         , 22 , 0  , 6228 , 284, 54 ), // #1058
  INST(Vmread           , X86Mr_NoSize       , O(000F00,78,_,_,_,_,_,_  ), 0                         , 4  , 0  , 6236 , 300, 54 ), // #1059
  INST(Vmresume         , X86Op              , O(000F01,C3,_,_,_,_,_,_  ), 0                         , 21 , 0  , 6243 , 30 , 54 ), // #1060
  INST(Vmrun            , X86Op_xAX          , O(000F01,D8,_,_,_,_,_,_  ), 0                         , 21 , 0  , 6252 , 285, 23 ), // #1061
  INST(Vmsave           , X86Op_xAX          , O(000F01,DB,_,_,_,_,_,_  ), 0                         , 21 , 0  , 6258 , 285, 23 ), // #1062
  INST(Vmulpd           , VexRvm_Lx          , V(660F00,59,_,x,I,1,4,FV ), 0                         , 93 , 0  , 6265 , 175, 107), // #1063
  INST(Vmulps           , VexRvm_Lx          , V(000F00,59,_,x,I,0,4,FV ), 0                         , 94 , 0  , 6272 , 176, 107), // #1064
  INST(Vmulsd           , VexRvm_Lx          , V(F20F00,59,_,I,I,1,3,T1S), 0                         , 95 , 0  , 6279 , 177, 108), // #1065
  INST(Vmulss           , VexRvm_Lx          , V(F30F00,59,_,I,I,0,2,T1S), 0                         , 96 , 0  , 6286 , 178, 108), // #1066
  INST(Vmwrite          , X86Rm_NoSize       , O(000F00,79,_,_,_,_,_,_  ), 0                         , 4  , 0  , 6293 , 301, 54 ), // #1067
  INST(Vmxon            , X86M_Only          , O(F30F00,C7,6,_,_,_,_,_  ), 0                         , 160, 0  , 6301 , 284, 54 ), // #1068
  INST(Vorpd            , VexRvm_Lx          , V(660F00,56,_,x,I,1,4,FV ), 0                         , 93 , 0  , 6307 , 187, 113), // #1069
  INST(Vorps            , VexRvm_Lx          , V(000F00,56,_,x,I,0,4,FV ), 0                         , 94 , 0  , 6313 , 188, 113), // #1070
  INST(Vp4dpwssd        , VexRm_T1_4X        , V(F20F38,52,_,2,_,0,2,T4X), 0                         , 92 , 0  , 6319 , 173, 130), // #1071
  INST(Vp4dpwssds       , VexRm_T1_4X        , V(F20F38,53,_,2,_,0,2,T4X), 0                         , 92 , 0  , 6329 , 173, 130), // #1072
  INST(Vpabsb           , VexRm_Lx           , V(660F38,1C,_,x,I,_,4,FVM), 0                         , 98 , 0  , 6340 , 298, 131), // #1073
  INST(Vpabsd           , VexRm_Lx           , V(660F38,1E,_,x,I,0,4,FV ), 0                         , 101, 0  , 6347 , 298, 117), // #1074
  INST(Vpabsq           , VexRm_Lx           , V(660F38,1F,_,x,_,1,4,FV ), 0                         , 102, 0  , 6354 , 241, 112), // #1075
  INST(Vpabsw           , VexRm_Lx           , V(660F38,1D,_,x,I,_,4,FVM), 0                         , 98 , 0  , 6361 , 298, 131), // #1076
  INST(Vpackssdw        , VexRvm_Lx          , V(660F00,6B,_,x,I,0,4,FV ), 0                         , 118, 0  , 6368 , 186, 131), // #1077
  INST(Vpacksswb        , VexRvm_Lx          , V(660F00,63,_,x,I,I,4,FVM), 0                         , 153, 0  , 6378 , 272, 131), // #1078
  INST(Vpackusdw        , VexRvm_Lx          , V(660F38,2B,_,x,I,0,4,FV ), 0                         , 101, 0  , 6388 , 186, 131), // #1079
  INST(Vpackuswb        , VexRvm_Lx          , V(660F00,67,_,x,I,I,4,FVM), 0                         , 153, 0  , 6398 , 272, 131), // #1080
  INST(Vpaddb           , VexRvm_Lx          , V(660F00,FC,_,x,I,I,4,FVM), 0                         , 153, 0  , 6408 , 272, 131), // #1081
  INST(Vpaddd           , VexRvm_Lx          , V(660F00,FE,_,x,I,0,4,FV ), 0                         , 118, 0  , 6415 , 186, 117), // #1082
  INST(Vpaddq           , VexRvm_Lx          , V(660F00,D4,_,x,I,1,4,FV ), 0                         , 93 , 0  , 6422 , 185, 117), // #1083
  INST(Vpaddsb          , VexRvm_Lx          , V(660F00,EC,_,x,I,I,4,FVM), 0                         , 153, 0  , 6429 , 272, 131), // #1084
  INST(Vpaddsw          , VexRvm_Lx          , V(660F00,ED,_,x,I,I,4,FVM), 0                         , 153, 0  , 6437 , 272, 131), // #1085
  INST(Vpaddusb         , VexRvm_Lx          , V(660F00,DC,_,x,I,I,4,FVM), 0                         , 153, 0  , 6445 , 272, 131), // #1086
  INST(Vpaddusw         , VexRvm_Lx          , V(660F00,DD,_,x,I,I,4,FVM), 0                         , 153, 0  , 6454 , 272, 131), // #1087
  INST(Vpaddw           , VexRvm_Lx          , V(660F00,FD,_,x,I,I,4,FVM), 0                         , 153, 0  , 6463 , 272, 131), // #1088
  INST(Vpalignr         , VexRvmi_Lx         , V(660F3A,0F,_,x,I,I,4,FVM), 0                         , 129, 0  , 6470 , 271, 131), // #1089
  INST(Vpand            , VexRvm_Lx          , V(660F00,DB,_,x,I,_,_,_  ), 0                         , 63 , 0  , 6479 , 302, 129), // #1090
  INST(Vpandd           , VexRvm_Lx          , V(660F00,DB,_,x,_,0,4,FV ), 0                         , 118, 0  , 6485 , 303, 112), // #1091
  INST(Vpandn           , VexRvm_Lx          , V(660F00,DF,_,x,I,_,_,_  ), 0                         , 63 , 0  , 6492 , 304, 129), // #1092
  INST(Vpandnd          , VexRvm_Lx          , V(660F00,DF,_,x,_,0,4,FV ), 0                         , 118, 0  , 6499 , 305, 112), // #1093
  INST(Vpandnq          , VexRvm_Lx          , V(660F00,DF,_,x,_,1,4,FV ), 0                         , 93 , 0  , 6507 , 306, 112), // #1094
  INST(Vpandq           , VexRvm_Lx          , V(660F00,DB,_,x,_,1,4,FV ), 0                         , 93 , 0  , 6515 , 307, 112), // #1095
  INST(Vpavgb           , VexRvm_Lx          , V(660F00,E0,_,x,I,I,4,FVM), 0                         , 153, 0  , 6522 , 272, 131), // #1096
  INST(Vpavgw           , VexRvm_Lx          , V(660F00,E3,_,x,I,I,4,FVM), 0                         , 153, 0  , 6529 , 272, 131), // #1097
  INST(Vpblendd         , VexRvmi_Lx         , V(660F3A,02,_,x,0,_,_,_  ), 0                         , 67 , 0  , 6536 , 192, 116), // #1098
  INST(Vpblendvb        , VexRvmr            , V(660F3A,4C,_,x,0,_,_,_  ), 0                         , 67 , 0  , 6545 , 193, 129), // #1099
  INST(Vpblendw         , VexRvmi_Lx         , V(660F3A,0E,_,x,I,_,_,_  ), 0                         , 67 , 0  , 6555 , 192, 129), // #1100
  INST(Vpbroadcastb     , VexRm_Lx           , V(660F38,78,_,x,0,0,0,T1S), 0                         , 161, 0  , 6564 , 308, 132), // #1101
  INST(Vpbroadcastd     , VexRm_Lx           , V(660F38,58,_,x,0,0,2,T1S), 0                         , 111, 0  , 6577 , 309, 126), // #1102
  INST(Vpbroadcastmb2d  , VexRm_Lx           , V(F30F38,3A,_,x,_,0,_,_  ), 0                         , 82 , 0  , 6590 , 310, 133), // #1103
  INST(Vpbroadcastmb2q  , VexRm_Lx           , V(F30F38,2A,_,x,_,1,_,_  ), 0                         , 162, 0  , 6606 , 310, 133), // #1104
  INST(Vpbroadcastq     , VexRm_Lx           , V(660F38,59,_,x,0,1,3,T1S), 0                         , 110, 0  , 6622 , 311, 126), // #1105
  INST(Vpbroadcastw     , VexRm_Lx           , V(660F38,79,_,x,0,0,1,T1S), 0                         , 163, 0  , 6635 , 312, 132), // #1106
  INST(Vpclmulqdq       , VexRvmi_Lx         , V(660F3A,44,_,x,I,_,4,FVM), 0                         , 129, 0  , 6648 , 313, 134), // #1107
  INST(Vpcmov           , VexRvrmRvmr_Lx     , V(XOP_M8,A2,_,x,x,_,_,_  ), 0                         , 164, 0  , 6659 , 250, 125), // #1108
  INST(Vpcmpb           , VexRvmi_Lx         , V(660F3A,3F,_,x,_,0,4,FVM), 0                         , 129, 0  , 6666 , 314, 114), // #1109
  INST(Vpcmpd           , VexRvmi_Lx         , V(660F3A,1F,_,x,_,0,4,FV ), 0                         , 99 , 0  , 6673 , 315, 112), // #1110
  INST(Vpcmpeqb         , VexRvm_Lx          , V(660F00,74,_,x,I,I,4,FV ), 0                         , 118, 0  , 6680 , 316, 131), // #1111
  INST(Vpcmpeqd         , VexRvm_Lx          , V(660F00,76,_,x,I,0,4,FVM), 0                         , 153, 0  , 6689 , 317, 117), // #1112
  INST(Vpcmpeqq         , VexRvm_Lx          , V(660F38,29,_,x,I,1,4,FVM), 0                         , 103, 0  , 6698 , 318, 117), // #1113
  INST(Vpcmpeqw         , VexRvm_Lx          , V(660F00,75,_,x,I,I,4,FV ), 0                         , 118, 0  , 6707 , 316, 131), // #1114
  INST(Vpcmpestri       , VexRmi             , V(660F3A,61,_,0,I,_,_,_  ), 0                         , 67 , 0  , 6716 , 319, 135), // #1115
  INST(Vpcmpestrm       , VexRmi             , V(660F3A,60,_,0,I,_,_,_  ), 0                         , 67 , 0  , 6727 , 320, 135), // #1116
  INST(Vpcmpgtb         , VexRvm_Lx          , V(660F00,64,_,x,I,I,4,FV ), 0                         , 118, 0  , 6738 , 316, 131), // #1117
  INST(Vpcmpgtd         , VexRvm_Lx          , V(660F00,66,_,x,I,0,4,FVM), 0                         , 153, 0  , 6747 , 317, 117), // #1118
  INST(Vpcmpgtq         , VexRvm_Lx          , V(660F38,37,_,x,I,1,4,FVM), 0                         , 103, 0  , 6756 , 318, 117), // #1119
  INST(Vpcmpgtw         , VexRvm_Lx          , V(660F00,65,_,x,I,I,4,FV ), 0                         , 118, 0  , 6765 , 316, 131), // #1120
  INST(Vpcmpistri       , VexRmi             , V(660F3A,63,_,0,I,_,_,_  ), 0                         , 67 , 0  , 6774 , 321, 135), // #1121
  INST(Vpcmpistrm       , VexRmi             , V(660F3A,62,_,0,I,_,_,_  ), 0                         , 67 , 0  , 6785 , 322, 135), // #1122
  INST(Vpcmpq           , VexRvmi_Lx         , V(660F3A,1F,_,x,_,1,4,FV ), 0                         , 100, 0  , 6796 , 323, 112), // #1123
  INST(Vpcmpub          , VexRvmi_Lx         , V(660F3A,3E,_,x,_,0,4,FVM), 0                         , 129, 0  , 6803 , 314, 114), // #1124
  INST(Vpcmpud          , VexRvmi_Lx         , V(660F3A,1E,_,x,_,0,4,FV ), 0                         , 99 , 0  , 6811 , 315, 112), // #1125
  INST(Vpcmpuq          , VexRvmi_Lx         , V(660F3A,1E,_,x,_,1,4,FV ), 0                         , 100, 0  , 6819 , 323, 112), // #1126
  INST(Vpcmpuw          , VexRvmi_Lx         , V(660F3A,3E,_,x,_,1,4,FVM), 0                         , 165, 0  , 6827 , 323, 114), // #1127
  INST(Vpcmpw           , VexRvmi_Lx         , V(660F3A,3F,_,x,_,1,4,FVM), 0                         , 165, 0  , 6835 , 323, 114), // #1128
  INST(Vpcomb           , VexRvmi            , V(XOP_M8,CC,_,0,0,_,_,_  ), 0                         , 164, 0  , 6842 , 238, 125), // #1129
  INST(Vpcomd           , VexRvmi            , V(XOP_M8,CE,_,0,0,_,_,_  ), 0                         , 164, 0  , 6849 , 238, 125), // #1130
  INST(Vpcompressb      , VexMr_Lx           , V(660F38,63,_,x,_,0,0,T1S), 0                         , 161, 0  , 6856 , 207, 136), // #1131
  INST(Vpcompressd      , VexMr_Lx           , V(660F38,8B,_,x,_,0,2,T1S), 0                         , 111, 0  , 6868 , 207, 112), // #1132
  INST(Vpcompressq      , VexMr_Lx           , V(660F38,8B,_,x,_,1,3,T1S), 0                         , 110, 0  , 6880 , 207, 112), // #1133
  INST(Vpcompressw      , VexMr_Lx           , V(660F38,63,_,x,_,1,1,T1S), 0                         , 166, 0  , 6892 , 207, 136), // #1134
  INST(Vpcomq           , VexRvmi            , V(XOP_M8,CF,_,0,0,_,_,_  ), 0                         , 164, 0  , 6904 , 238, 125), // #1135
  INST(Vpcomub          , VexRvmi            , V(XOP_M8,EC,_,0,0,_,_,_  ), 0                         , 164, 0  , 6911 , 238, 125), // #1136
  INST(Vpcomud          , VexRvmi            , V(XOP_M8,EE,_,0,0,_,_,_  ), 0                         , 164, 0  , 6919 , 238, 125), // #1137
  INST(Vpcomuq          , VexRvmi            , V(XOP_M8,EF,_,0,0,_,_,_  ), 0                         , 164, 0  , 6927 , 238, 125), // #1138
  INST(Vpcomuw          , VexRvmi            , V(XOP_M8,ED,_,0,0,_,_,_  ), 0                         , 164, 0  , 6935 , 238, 125), // #1139
  INST(Vpcomw           , VexRvmi            , V(XOP_M8,CD,_,0,0,_,_,_  ), 0                         , 164, 0  , 6943 , 238, 125), // #1140
  INST(Vpconflictd      , VexRm_Lx           , V(660F38,C4,_,x,_,0,4,FV ), 0                         , 101, 0  , 6950 , 324, 133), // #1141
  INST(Vpconflictq      , VexRm_Lx           , V(660F38,C4,_,x,_,1,4,FV ), 0                         , 102, 0  , 6962 , 324, 133), // #1142
  INST(Vpdpbusd         , VexRvm_Lx          , V(660F38,50,_,x,_,0,_,FV ), 0                         , 147, 0  , 6974 , 190, 137), // #1143
  INST(Vpdpbusds        , VexRvm_Lx          , V(660F38,51,_,x,_,0,_,FV ), 0                         , 147, 0  , 6983 , 190, 137), // #1144
  INST(Vpdpwssd         , VexRvm_Lx          , V(660F38,52,_,x,_,0,_,FV ), 0                         , 147, 0  , 6993 , 190, 137), // #1145
  INST(Vpdpwssds        , VexRvm_Lx          , V(660F38,53,_,x,_,0,_,FV ), 0                         , 147, 0  , 7002 , 190, 137), // #1146
  INST(Vperm2f128       , VexRvmi            , V(660F3A,06,_,1,0,_,_,_  ), 0                         , 132, 0  , 7012 , 325, 109), // #1147
  INST(Vperm2i128       , VexRvmi            , V(660F3A,46,_,1,0,_,_,_  ), 0                         , 132, 0  , 7023 , 325, 116), // #1148
  INST(Vpermb           , VexRvm_Lx          , V(660F38,8D,_,x,_,0,4,FVM), 0                         , 98 , 0  , 7034 , 189, 138), // #1149
  INST(Vpermd           , VexRvm_Lx          , V(660F38,36,_,x,0,0,4,FV ), 0                         , 101, 0  , 7041 , 326, 126), // #1150
  INST(Vpermi2b         , VexRvm_Lx          , V(660F38,75,_,x,_,0,4,FVM), 0                         , 98 , 0  , 7048 , 189, 138), // #1151
  INST(Vpermi2d         , VexRvm_Lx          , V(660F38,76,_,x,_,0,4,FV ), 0                         , 101, 0  , 7057 , 190, 112), // #1152
  INST(Vpermi2pd        , VexRvm_Lx          , V(660F38,77,_,x,_,1,4,FV ), 0                         , 102, 0  , 7066 , 191, 112), // #1153
  INST(Vpermi2ps        , VexRvm_Lx          , V(660F38,77,_,x,_,0,4,FV ), 0                         , 101, 0  , 7076 , 190, 112), // #1154
  INST(Vpermi2q         , VexRvm_Lx          , V(660F38,76,_,x,_,1,4,FV ), 0                         , 102, 0  , 7086 , 191, 112), // #1155
  INST(Vpermi2w         , VexRvm_Lx          , V(660F38,75,_,x,_,1,4,FVM), 0                         , 103, 0  , 7095 , 189, 114), // #1156
  INST(Vpermil2pd       , VexRvrmiRvmri_Lx   , V(660F3A,49,_,x,x,_,_,_  ), 0                         , 67 , 0  , 7104 , 327, 125), // #1157
  INST(Vpermil2ps       , VexRvrmiRvmri_Lx   , V(660F3A,48,_,x,x,_,_,_  ), 0                         , 67 , 0  , 7115 , 327, 125), // #1158
  INST(Vpermilpd        , VexRvmRmi_Lx       , V(660F38,0D,_,x,0,1,4,FV ), V(660F3A,05,_,x,0,1,4,FV ), 102, 105, 7126 , 328, 107), // #1159
  INST(Vpermilps        , VexRvmRmi_Lx       , V(660F38,0C,_,x,0,0,4,FV ), V(660F3A,04,_,x,0,0,4,FV ), 101, 106, 7136 , 328, 107), // #1160
  INST(Vpermpd          , VexRmi             , V(660F3A,01,_,1,1,_,_,_  ), 0                         , 167, 0  , 7146 , 329, 116), // #1161
  INST(Vpermps          , VexRvm             , V(660F38,16,_,1,0,_,_,_  ), 0                         , 104, 0  , 7154 , 330, 116), // #1162
  INST(Vpermq           , VexRvmRmi_Lx       , V(660F38,36,_,x,_,1,4,FV ), V(660F3A,00,_,x,1,1,4,FV ), 102, 107, 7162 , 331, 126), // #1163
  INST(Vpermt2b         , VexRvm_Lx          , V(660F38,7D,_,x,_,0,4,FVM), 0                         , 98 , 0  , 7169 , 189, 138), // #1164
  INST(Vpermt2d         , VexRvm_Lx          , V(660F38,7E,_,x,_,0,4,FV ), 0                         , 101, 0  , 7178 , 190, 112), // #1165
  INST(Vpermt2pd        , VexRvm_Lx          , V(660F38,7F,_,x,_,1,4,FV ), 0                         , 102, 0  , 7187 , 191, 112), // #1166
  INST(Vpermt2ps        , VexRvm_Lx          , V(660F38,7F,_,x,_,0,4,FV ), 0                         , 101, 0  , 7197 , 190, 112), // #1167
  INST(Vpermt2q         , VexRvm_Lx          , V(660F38,7E,_,x,_,1,4,FV ), 0                         , 102, 0  , 7207 , 191, 112), // #1168
  INST(Vpermt2w         , VexRvm_Lx          , V(660F38,7D,_,x,_,1,4,FVM), 0                         , 103, 0  , 7216 , 189, 114), // #1169
  INST(Vpermw           , VexRvm_Lx          , V(660F38,8D,_,x,_,1,4,FVM), 0                         , 103, 0  , 7225 , 189, 114), // #1170
  INST(Vpexpandb        , VexRm_Lx           , V(660F38,62,_,x,_,0,0,T1S), 0                         , 161, 0  , 7232 , 241, 136), // #1171
  INST(Vpexpandd        , VexRm_Lx           , V(660F38,89,_,x,_,0,2,T1S), 0                         , 111, 0  , 7242 , 241, 112), // #1172
  INST(Vpexpandq        , VexRm_Lx           , V(660F38,89,_,x,_,1,3,T1S), 0                         , 110, 0  , 7252 , 241, 112), // #1173
  INST(Vpexpandw        , VexRm_Lx           , V(660F38,62,_,x,_,1,1,T1S), 0                         , 166, 0  , 7262 , 241, 136), // #1174
  INST(Vpextrb          , VexMri             , V(660F3A,14,_,0,0,I,0,T1S), 0                         , 168, 0  , 7272 , 332, 139), // #1175
  INST(Vpextrd          , VexMri             , V(660F3A,16,_,0,0,0,2,T1S), 0                         , 137, 0  , 7280 , 245, 140), // #1176
  INST(Vpextrq          , VexMri             , V(660F3A,16,_,0,1,1,3,T1S), 0                         , 169, 0  , 7288 , 333, 140), // #1177
  INST(Vpextrw          , VexMri             , V(660F3A,15,_,0,0,I,1,T1S), 0                         , 170, 0  , 7296 , 334, 139), // #1178
  INST(Vpgatherdd       , VexRmvRm_VM        , V(660F38,90,_,x,0,_,_,_  ), V(660F38,90,_,x,_,0,2,T1S), 88 , 108, 7304 , 261, 126), // #1179
  INST(Vpgatherdq       , VexRmvRm_VM        , V(660F38,90,_,x,1,_,_,_  ), V(660F38,90,_,x,_,1,3,T1S), 141, 109, 7315 , 260, 126), // #1180
  INST(Vpgatherqd       , VexRmvRm_VM        , V(660F38,91,_,x,0,_,_,_  ), V(660F38,91,_,x,_,0,2,T1S), 88 , 110, 7326 , 266, 126), // #1181
  INST(Vpgatherqq       , VexRmvRm_VM        , V(660F38,91,_,x,1,_,_,_  ), V(660F38,91,_,x,_,1,3,T1S), 141, 111, 7337 , 265, 126), // #1182
  INST(Vphaddbd         , VexRm              , V(XOP_M9,C2,_,0,0,_,_,_  ), 0                         , 72 , 0  , 7348 , 181, 125), // #1183
  INST(Vphaddbq         , VexRm              , V(XOP_M9,C3,_,0,0,_,_,_  ), 0                         , 72 , 0  , 7357 , 181, 125), // #1184
  INST(Vphaddbw         , VexRm              , V(XOP_M9,C1,_,0,0,_,_,_  ), 0                         , 72 , 0  , 7366 , 181, 125), // #1185
  INST(Vphaddd          , VexRvm_Lx          , V(660F38,02,_,x,I,_,_,_  ), 0                         , 88 , 0  , 7375 , 179, 129), // #1186
  INST(Vphadddq         , VexRm              , V(XOP_M9,CB,_,0,0,_,_,_  ), 0                         , 72 , 0  , 7383 , 181, 125), // #1187
  INST(Vphaddsw         , VexRvm_Lx          , V(660F38,03,_,x,I,_,_,_  ), 0                         , 88 , 0  , 7392 , 179, 129), // #1188
  INST(Vphaddubd        , VexRm              , V(XOP_M9,D2,_,0,0,_,_,_  ), 0                         , 72 , 0  , 7401 , 181, 125), // #1189
  INST(Vphaddubq        , VexRm              , V(XOP_M9,D3,_,0,0,_,_,_  ), 0                         , 72 , 0  , 7411 , 181, 125), // #1190
  INST(Vphaddubw        , VexRm              , V(XOP_M9,D1,_,0,0,_,_,_  ), 0                         , 72 , 0  , 7421 , 181, 125), // #1191
  INST(Vphaddudq        , VexRm              , V(XOP_M9,DB,_,0,0,_,_,_  ), 0                         , 72 , 0  , 7431 , 181, 125), // #1192
  INST(Vphadduwd        , VexRm              , V(XOP_M9,D6,_,0,0,_,_,_  ), 0                         , 72 , 0  , 7441 , 181, 125), // #1193
  INST(Vphadduwq        , VexRm              , V(XOP_M9,D7,_,0,0,_,_,_  ), 0                         , 72 , 0  , 7451 , 181, 125), // #1194
  INST(Vphaddw          , VexRvm_Lx          , V(660F38,01,_,x,I,_,_,_  ), 0                         , 88 , 0  , 7461 , 179, 129), // #1195
  INST(Vphaddwd         , VexRm              , V(XOP_M9,C6,_,0,0,_,_,_  ), 0                         , 72 , 0  , 7469 , 181, 125), // #1196
  INST(Vphaddwq         , VexRm              , V(XOP_M9,C7,_,0,0,_,_,_  ), 0                         , 72 , 0  , 7478 , 181, 125), // #1197
  INST(Vphminposuw      , VexRm              , V(660F38,41,_,0,I,_,_,_  ), 0                         , 88 , 0  , 7487 , 181, 109), // #1198
  INST(Vphsubbw         , VexRm              , V(XOP_M9,E1,_,0,0,_,_,_  ), 0                         , 72 , 0  , 7499 , 181, 125), // #1199
  INST(Vphsubd          , VexRvm_Lx          , V(660F38,06,_,x,I,_,_,_  ), 0                         , 88 , 0  , 7508 , 179, 129), // #1200
  INST(Vphsubdq         , VexRm              , V(XOP_M9,E3,_,0,0,_,_,_  ), 0                         , 72 , 0  , 7516 , 181, 125), // #1201
  INST(Vphsubsw         , VexRvm_Lx          , V(660F38,07,_,x,I,_,_,_  ), 0                         , 88 , 0  , 7525 , 179, 129), // #1202
  INST(Vphsubw          , VexRvm_Lx          , V(660F38,05,_,x,I,_,_,_  ), 0                         , 88 , 0  , 7534 , 179, 129), // #1203
  INST(Vphsubwd         , VexRm              , V(XOP_M9,E2,_,0,0,_,_,_  ), 0                         , 72 , 0  , 7542 , 181, 125), // #1204
  INST(Vpinsrb          , VexRvmi            , V(660F3A,20,_,0,0,I,0,T1S), 0                         , 168, 0  , 7551 , 335, 139), // #1205
  INST(Vpinsrd          , VexRvmi            , V(660F3A,22,_,0,0,0,2,T1S), 0                         , 137, 0  , 7559 , 336, 140), // #1206
  INST(Vpinsrq          , VexRvmi            , V(660F3A,22,_,0,1,1,3,T1S), 0                         , 169, 0  , 7567 , 337, 140), // #1207
  INST(Vpinsrw          , VexRvmi            , V(660F00,C4,_,0,0,I,1,T1S), 0                         , 171, 0  , 7575 , 338, 139), // #1208
  INST(Vplzcntd         , VexRm_Lx           , V(660F38,44,_,x,_,0,4,FV ), 0                         , 101, 0  , 7583 , 324, 133), // #1209
  INST(Vplzcntq         , VexRm_Lx           , V(660F38,44,_,x,_,1,4,FV ), 0                         , 102, 0  , 7592 , 339, 133), // #1210
  INST(Vpmacsdd         , VexRvmr            , V(XOP_M8,9E,_,0,0,_,_,_  ), 0                         , 164, 0  , 7601 , 340, 125), // #1211
  INST(Vpmacsdqh        , VexRvmr            , V(XOP_M8,9F,_,0,0,_,_,_  ), 0                         , 164, 0  , 7610 , 340, 125), // #1212
  INST(Vpmacsdql        , VexRvmr            , V(XOP_M8,97,_,0,0,_,_,_  ), 0                         , 164, 0  , 7620 , 340, 125), // #1213
  INST(Vpmacssdd        , VexRvmr            , V(XOP_M8,8E,_,0,0,_,_,_  ), 0                         , 164, 0  , 7630 , 340, 125), // #1214
  INST(Vpmacssdqh       , VexRvmr            , V(XOP_M8,8F,_,0,0,_,_,_  ), 0                         , 164, 0  , 7640 , 340, 125), // #1215
  INST(Vpmacssdql       , VexRvmr            , V(XOP_M8,87,_,0,0,_,_,_  ), 0                         , 164, 0  , 7651 , 340, 125), // #1216
  INST(Vpmacsswd        , VexRvmr            , V(XOP_M8,86,_,0,0,_,_,_  ), 0                         , 164, 0  , 7662 , 340, 125), // #1217
  INST(Vpmacssww        , VexRvmr            , V(XOP_M8,85,_,0,0,_,_,_  ), 0                         , 164, 0  , 7672 , 340, 125), // #1218
  INST(Vpmacswd         , VexRvmr            , V(XOP_M8,96,_,0,0,_,_,_  ), 0                         , 164, 0  , 7682 , 340, 125), // #1219
  INST(Vpmacsww         , VexRvmr            , V(XOP_M8,95,_,0,0,_,_,_  ), 0                         , 164, 0  , 7691 , 340, 125), // #1220
  INST(Vpmadcsswd       , VexRvmr            , V(XOP_M8,A6,_,0,0,_,_,_  ), 0                         , 164, 0  , 7700 , 340, 125), // #1221
  INST(Vpmadcswd        , VexRvmr            , V(XOP_M8,B6,_,0,0,_,_,_  ), 0                         , 164, 0  , 7711 , 340, 125), // #1222
  INST(Vpmadd52huq      , VexRvm_Lx          , V(660F38,B5,_,x,_,1,4,FV ), 0                         , 102, 0  , 7721 , 191, 141), // #1223
  INST(Vpmadd52luq      , VexRvm_Lx          , V(660F38,B4,_,x,_,1,4,FV ), 0                         , 102, 0  , 7733 , 191, 141), // #1224
  INST(Vpmaddubsw       , VexRvm_Lx          , V(660F38,04,_,x,I,I,4,FVM), 0                         , 98 , 0  , 7745 , 272, 131), // #1225
  INST(Vpmaddwd         , VexRvm_Lx          , V(660F00,F5,_,x,I,I,4,FVM), 0                         , 153, 0  , 7756 , 272, 131), // #1226
  INST(Vpmaskmovd       , VexRvmMvr_Lx       , V(660F38,8C,_,x,0,_,_,_  ), V(660F38,8E,_,x,0,_,_,_  ), 88 , 112, 7765 , 280, 116), // #1227
  INST(Vpmaskmovq       , VexRvmMvr_Lx       , V(660F38,8C,_,x,1,_,_,_  ), V(660F38,8E,_,x,1,_,_,_  ), 141, 113, 7776 , 280, 116), // #1228
  INST(Vpmaxsb          , VexRvm_Lx          , V(660F38,3C,_,x,I,I,4,FVM), 0                         , 98 , 0  , 7787 , 341, 131), // #1229
  INST(Vpmaxsd          , VexRvm_Lx          , V(660F38,3D,_,x,I,0,4,FV ), 0                         , 101, 0  , 7795 , 188, 117), // #1230
  INST(Vpmaxsq          , VexRvm_Lx          , V(660F38,3D,_,x,_,1,4,FV ), 0                         , 102, 0  , 7803 , 191, 112), // #1231
  INST(Vpmaxsw          , VexRvm_Lx          , V(660F00,EE,_,x,I,I,4,FVM), 0                         , 153, 0  , 7811 , 341, 131), // #1232
  INST(Vpmaxub          , VexRvm_Lx          , V(660F00,DE,_,x,I,I,4,FVM), 0                         , 153, 0  , 7819 , 341, 131), // #1233
  INST(Vpmaxud          , VexRvm_Lx          , V(660F38,3F,_,x,I,0,4,FV ), 0                         , 101, 0  , 7827 , 188, 117), // #1234
  INST(Vpmaxuq          , VexRvm_Lx          , V(660F38,3F,_,x,_,1,4,FV ), 0                         , 102, 0  , 7835 , 191, 112), // #1235
  INST(Vpmaxuw          , VexRvm_Lx          , V(660F38,3E,_,x,I,I,4,FVM), 0                         , 98 , 0  , 7843 , 341, 131), // #1236
  INST(Vpminsb          , VexRvm_Lx          , V(660F38,38,_,x,I,I,4,FVM), 0                         , 98 , 0  , 7851 , 341, 131), // #1237
  INST(Vpminsd          , VexRvm_Lx          , V(660F38,39,_,x,I,0,4,FV ), 0                         , 101, 0  , 7859 , 188, 117), // #1238
  INST(Vpminsq          , VexRvm_Lx          , V(660F38,39,_,x,_,1,4,FV ), 0                         , 102, 0  , 7867 , 191, 112), // #1239
  INST(Vpminsw          , VexRvm_Lx          , V(660F00,EA,_,x,I,I,4,FVM), 0                         , 153, 0  , 7875 , 341, 131), // #1240
  INST(Vpminub          , VexRvm_Lx          , V(660F00,DA,_,x,I,_,4,FVM), 0                         , 153, 0  , 7883 , 341, 131), // #1241
  INST(Vpminud          , VexRvm_Lx          , V(660F38,3B,_,x,I,0,4,FV ), 0                         , 101, 0  , 7891 , 188, 117), // #1242
  INST(Vpminuq          , VexRvm_Lx          , V(660F38,3B,_,x,_,1,4,FV ), 0                         , 102, 0  , 7899 , 191, 112), // #1243
  INST(Vpminuw          , VexRvm_Lx          , V(660F38,3A,_,x,I,_,4,FVM), 0                         , 98 , 0  , 7907 , 341, 131), // #1244
  INST(Vpmovb2m         , VexRm_Lx           , V(F30F38,29,_,x,_,0,_,_  ), 0                         , 82 , 0  , 7915 , 342, 114), // #1245
  INST(Vpmovd2m         , VexRm_Lx           , V(F30F38,39,_,x,_,0,_,_  ), 0                         , 82 , 0  , 7924 , 342, 115), // #1246
  INST(Vpmovdb          , VexMr_Lx           , V(F30F38,31,_,x,_,0,2,QVM), 0                         , 172, 0  , 7933 , 343, 112), // #1247
  INST(Vpmovdw          , VexMr_Lx           , V(F30F38,33,_,x,_,0,3,HVM), 0                         , 173, 0  , 7941 , 344, 112), // #1248
  INST(Vpmovm2b         , VexRm_Lx           , V(F30F38,28,_,x,_,0,_,_  ), 0                         , 82 , 0  , 7949 , 310, 114), // #1249
  INST(Vpmovm2d         , VexRm_Lx           , V(F30F38,38,_,x,_,0,_,_  ), 0                         , 82 , 0  , 7958 , 310, 115), // #1250
  INST(Vpmovm2q         , VexRm_Lx           , V(F30F38,38,_,x,_,1,_,_  ), 0                         , 162, 0  , 7967 , 310, 115), // #1251
  INST(Vpmovm2w         , VexRm_Lx           , V(F30F38,28,_,x,_,1,_,_  ), 0                         , 162, 0  , 7976 , 310, 114), // #1252
  INST(Vpmovmskb        , VexRm_Lx           , V(660F00,D7,_,x,I,_,_,_  ), 0                         , 63 , 0  , 7985 , 293, 129), // #1253
  INST(Vpmovq2m         , VexRm_Lx           , V(F30F38,39,_,x,_,1,_,_  ), 0                         , 162, 0  , 7995 , 342, 115), // #1254
  INST(Vpmovqb          , VexMr_Lx           , V(F30F38,32,_,x,_,0,1,OVM), 0                         , 174, 0  , 8004 , 345, 112), // #1255
  INST(Vpmovqd          , VexMr_Lx           , V(F30F38,35,_,x,_,0,3,HVM), 0                         , 173, 0  , 8012 , 344, 112), // #1256
  INST(Vpmovqw          , VexMr_Lx           , V(F30F38,34,_,x,_,0,2,QVM), 0                         , 172, 0  , 8020 , 343, 112), // #1257
  INST(Vpmovsdb         , VexMr_Lx           , V(F30F38,21,_,x,_,0,2,QVM), 0                         , 172, 0  , 8028 , 343, 112), // #1258
  INST(Vpmovsdw         , VexMr_Lx           , V(F30F38,23,_,x,_,0,3,HVM), 0                         , 173, 0  , 8037 , 344, 112), // #1259
  INST(Vpmovsqb         , VexMr_Lx           , V(F30F38,22,_,x,_,0,1,OVM), 0                         , 174, 0  , 8046 , 345, 112), // #1260
  INST(Vpmovsqd         , VexMr_Lx           , V(F30F38,25,_,x,_,0,3,HVM), 0                         , 173, 0  , 8055 , 344, 112), // #1261
  INST(Vpmovsqw         , VexMr_Lx           , V(F30F38,24,_,x,_,0,2,QVM), 0                         , 172, 0  , 8064 , 343, 112), // #1262
  INST(Vpmovswb         , VexMr_Lx           , V(F30F38,20,_,x,_,0,3,HVM), 0                         , 173, 0  , 8073 , 344, 114), // #1263
  INST(Vpmovsxbd        , VexRm_Lx           , V(660F38,21,_,x,I,I,2,QVM), 0                         , 175, 0  , 8082 , 346, 117), // #1264
  INST(Vpmovsxbq        , VexRm_Lx           , V(660F38,22,_,x,I,I,1,OVM), 0                         , 176, 0  , 8092 , 347, 117), // #1265
  INST(Vpmovsxbw        , VexRm_Lx           , V(660F38,20,_,x,I,I,3,HVM), 0                         , 117, 0  , 8102 , 348, 131), // #1266
  INST(Vpmovsxdq        , VexRm_Lx           , V(660F38,25,_,x,I,0,3,HVM), 0                         , 117, 0  , 8112 , 348, 117), // #1267
  INST(Vpmovsxwd        , VexRm_Lx           , V(660F38,23,_,x,I,I,3,HVM), 0                         , 117, 0  , 8122 , 348, 117), // #1268
  INST(Vpmovsxwq        , VexRm_Lx           , V(660F38,24,_,x,I,I,2,QVM), 0                         , 175, 0  , 8132 , 346, 117), // #1269
  INST(Vpmovusdb        , VexMr_Lx           , V(F30F38,11,_,x,_,0,2,QVM), 0                         , 172, 0  , 8142 , 343, 112), // #1270
  INST(Vpmovusdw        , VexMr_Lx           , V(F30F38,13,_,x,_,0,3,HVM), 0                         , 173, 0  , 8152 , 344, 112), // #1271
  INST(Vpmovusqb        , VexMr_Lx           , V(F30F38,12,_,x,_,0,1,OVM), 0                         , 174, 0  , 8162 , 345, 112), // #1272
  INST(Vpmovusqd        , VexMr_Lx           , V(F30F38,15,_,x,_,0,3,HVM), 0                         , 173, 0  , 8172 , 344, 112), // #1273
  INST(Vpmovusqw        , VexMr_Lx           , V(F30F38,14,_,x,_,0,2,QVM), 0                         , 172, 0  , 8182 , 343, 112), // #1274
  INST(Vpmovuswb        , VexMr_Lx           , V(F30F38,10,_,x,_,0,3,HVM), 0                         , 173, 0  , 8192 , 344, 114), // #1275
  INST(Vpmovw2m         , VexRm_Lx           , V(F30F38,29,_,x,_,1,_,_  ), 0                         , 162, 0  , 8202 , 342, 114), // #1276
  INST(Vpmovwb          , VexMr_Lx           , V(F30F38,30,_,x,_,0,3,HVM), 0                         , 173, 0  , 8211 , 344, 114), // #1277
  INST(Vpmovzxbd        , VexRm_Lx           , V(660F38,31,_,x,I,I,2,QVM), 0                         , 175, 0  , 8219 , 346, 117), // #1278
  INST(Vpmovzxbq        , VexRm_Lx           , V(660F38,32,_,x,I,I,1,OVM), 0                         , 176, 0  , 8229 , 347, 117), // #1279
  INST(Vpmovzxbw        , VexRm_Lx           , V(660F38,30,_,x,I,I,3,HVM), 0                         , 117, 0  , 8239 , 348, 131), // #1280
  INST(Vpmovzxdq        , VexRm_Lx           , V(660F38,35,_,x,I,0,3,HVM), 0                         , 117, 0  , 8249 , 348, 117), // #1281
  INST(Vpmovzxwd        , VexRm_Lx           , V(660F38,33,_,x,I,I,3,HVM), 0                         , 117, 0  , 8259 , 348, 117), // #1282
  INST(Vpmovzxwq        , VexRm_Lx           , V(660F38,34,_,x,I,I,2,QVM), 0                         , 175, 0  , 8269 , 346, 117), // #1283
  INST(Vpmuldq          , VexRvm_Lx          , V(660F38,28,_,x,I,1,4,FV ), 0                         , 102, 0  , 8279 , 185, 117), // #1284
  INST(Vpmulhrsw        , VexRvm_Lx          , V(660F38,0B,_,x,I,I,4,FVM), 0                         , 98 , 0  , 8287 , 272, 131), // #1285
  INST(Vpmulhuw         , VexRvm_Lx          , V(660F00,E4,_,x,I,I,4,FVM), 0                         , 153, 0  , 8297 , 272, 131), // #1286
  INST(Vpmulhw          , VexRvm_Lx          , V(660F00,E5,_,x,I,I,4,FVM), 0                         , 153, 0  , 8306 , 272, 131), // #1287
  INST(Vpmulld          , VexRvm_Lx          , V(660F38,40,_,x,I,0,4,FV ), 0                         , 101, 0  , 8314 , 186, 117), // #1288
  INST(Vpmullq          , VexRvm_Lx          , V(660F38,40,_,x,_,1,4,FV ), 0                         , 102, 0  , 8322 , 191, 115), // #1289
  INST(Vpmullw          , VexRvm_Lx          , V(660F00,D5,_,x,I,I,4,FVM), 0                         , 153, 0  , 8330 , 272, 131), // #1290
  INST(Vpmultishiftqb   , VexRvm_Lx          , V(660F38,83,_,x,_,1,4,FV ), 0                         , 102, 0  , 8338 , 191, 138), // #1291
  INST(Vpmuludq         , VexRvm_Lx          , V(660F00,F4,_,x,I,1,4,FV ), 0                         , 93 , 0  , 8353 , 185, 117), // #1292
  INST(Vpopcntb         , VexRm_Lx           , V(660F38,54,_,x,_,0,4,FV ), 0                         , 101, 0  , 8362 , 241, 142), // #1293
  INST(Vpopcntd         , VexRm_Lx           , V(660F38,55,_,x,_,0,4,FVM), 0                         , 98 , 0  , 8371 , 324, 143), // #1294
  INST(Vpopcntq         , VexRm_Lx           , V(660F38,55,_,x,_,1,4,FVM), 0                         , 103, 0  , 8380 , 339, 143), // #1295
  INST(Vpopcntw         , VexRm_Lx           , V(660F38,54,_,x,_,1,4,FV ), 0                         , 102, 0  , 8389 , 241, 142), // #1296
  INST(Vpor             , VexRvm_Lx          , V(660F00,EB,_,x,I,_,_,_  ), 0                         , 63 , 0  , 8398 , 302, 129), // #1297
  INST(Vpord            , VexRvm_Lx          , V(660F00,EB,_,x,_,0,4,FV ), 0                         , 118, 0  , 8403 , 303, 112), // #1298
  INST(Vporq            , VexRvm_Lx          , V(660F00,EB,_,x,_,1,4,FV ), 0                         , 93 , 0  , 8409 , 307, 112), // #1299
  INST(Vpperm           , VexRvrmRvmr        , V(XOP_M8,A3,_,0,x,_,_,_  ), 0                         , 164, 0  , 8415 , 349, 125), // #1300
  INST(Vprold           , VexVmi_Lx          , V(660F00,72,1,x,_,0,4,FV ), 0                         , 177, 0  , 8422 , 350, 112), // #1301
  INST(Vprolq           , VexVmi_Lx          , V(660F00,72,1,x,_,1,4,FV ), 0                         , 178, 0  , 8429 , 351, 112), // #1302
  INST(Vprolvd          , VexRvm_Lx          , V(660F38,15,_,x,_,0,4,FV ), 0                         , 101, 0  , 8436 , 190, 112), // #1303
  INST(Vprolvq          , VexRvm_Lx          , V(660F38,15,_,x,_,1,4,FV ), 0                         , 102, 0  , 8444 , 191, 112), // #1304
  INST(Vprord           , VexVmi_Lx          , V(660F00,72,0,x,_,0,4,FV ), 0                         , 118, 0  , 8452 , 350, 112), // #1305
  INST(Vprorq           , VexVmi_Lx          , V(660F00,72,0,x,_,1,4,FV ), 0                         , 93 , 0  , 8459 , 351, 112), // #1306
  INST(Vprorvd          , VexRvm_Lx          , V(660F38,14,_,x,_,0,4,FV ), 0                         , 101, 0  , 8466 , 190, 112), // #1307
  INST(Vprorvq          , VexRvm_Lx          , V(660F38,14,_,x,_,1,4,FV ), 0                         , 102, 0  , 8474 , 191, 112), // #1308
  INST(Vprotb           , VexRvmRmvRmi       , V(XOP_M9,90,_,0,x,_,_,_  ), V(XOP_M8,C0,_,0,x,_,_,_  ), 72 , 114, 8482 , 352, 125), // #1309
  INST(Vprotd           , VexRvmRmvRmi       , V(XOP_M9,92,_,0,x,_,_,_  ), V(XOP_M8,C2,_,0,x,_,_,_  ), 72 , 115, 8489 , 352, 125), // #1310
  INST(Vprotq           , VexRvmRmvRmi       , V(XOP_M9,93,_,0,x,_,_,_  ), V(XOP_M8,C3,_,0,x,_,_,_  ), 72 , 116, 8496 , 352, 125), // #1311
  INST(Vprotw           , VexRvmRmvRmi       , V(XOP_M9,91,_,0,x,_,_,_  ), V(XOP_M8,C1,_,0,x,_,_,_  ), 72 , 117, 8503 , 352, 125), // #1312
  INST(Vpsadbw          , VexRvm_Lx          , V(660F00,F6,_,x,I,I,4,FVM), 0                         , 153, 0  , 8510 , 180, 131), // #1313
  INST(Vpscatterdd      , VexMr_VM           , V(660F38,A0,_,x,_,0,2,T1S), 0                         , 111, 0  , 8518 , 353, 112), // #1314
  INST(Vpscatterdq      , VexMr_VM           , V(660F38,A0,_,x,_,1,3,T1S), 0                         , 110, 0  , 8530 , 353, 112), // #1315
  INST(Vpscatterqd      , VexMr_VM           , V(660F38,A1,_,x,_,0,2,T1S), 0                         , 111, 0  , 8542 , 354, 112), // #1316
  INST(Vpscatterqq      , VexMr_VM           , V(660F38,A1,_,x,_,1,3,T1S), 0                         , 110, 0  , 8554 , 355, 112), // #1317
  INST(Vpshab           , VexRvmRmv          , V(XOP_M9,98,_,0,x,_,_,_  ), 0                         , 72 , 0  , 8566 , 356, 125), // #1318
  INST(Vpshad           , VexRvmRmv          , V(XOP_M9,9A,_,0,x,_,_,_  ), 0                         , 72 , 0  , 8573 , 356, 125), // #1319
  INST(Vpshaq           , VexRvmRmv          , V(XOP_M9,9B,_,0,x,_,_,_  ), 0                         , 72 , 0  , 8580 , 356, 125), // #1320
  INST(Vpshaw           , VexRvmRmv          , V(XOP_M9,99,_,0,x,_,_,_  ), 0                         , 72 , 0  , 8587 , 356, 125), // #1321
  INST(Vpshlb           , VexRvmRmv          , V(XOP_M9,94,_,0,x,_,_,_  ), 0                         , 72 , 0  , 8594 , 356, 125), // #1322
  INST(Vpshld           , VexRvmRmv          , V(XOP_M9,96,_,0,x,_,_,_  ), 0                         , 72 , 0  , 8601 , 356, 125), // #1323
  INST(Vpshldd          , VexRvmi_Lx         , V(660F3A,71,_,x,_,0,4,FV ), 0                         , 99 , 0  , 8608 , 183, 136), // #1324
  INST(Vpshldq          , VexRvmi_Lx         , V(660F3A,71,_,x,_,1,4,FV ), 0                         , 100, 0  , 8616 , 184, 136), // #1325
  INST(Vpshldvd         , VexRvm_Lx          , V(660F38,71,_,x,_,0,4,FV ), 0                         , 101, 0  , 8624 , 190, 136), // #1326
  INST(Vpshldvq         , VexRvm_Lx          , V(660F38,71,_,x,_,1,4,FV ), 0                         , 102, 0  , 8633 , 191, 136), // #1327
  INST(Vpshldvw         , VexRvm_Lx          , V(660F38,70,_,x,_,0,4,FVM), 0                         , 98 , 0  , 8642 , 189, 136), // #1328
  INST(Vpshldw          , VexRvmi_Lx         , V(660F3A,70,_,x,_,0,4,FVM), 0                         , 129, 0  , 8651 , 237, 136), // #1329
  INST(Vpshlq           , VexRvmRmv          , V(XOP_M9,97,_,0,x,_,_,_  ), 0                         , 72 , 0  , 8659 , 356, 125), // #1330
  INST(Vpshlw           , VexRvmRmv          , V(XOP_M9,95,_,0,x,_,_,_  ), 0                         , 72 , 0  , 8666 , 356, 125), // #1331
  INST(Vpshrdd          , VexRvmi_Lx         , V(660F3A,73,_,x,_,0,4,FV ), 0                         , 99 , 0  , 8673 , 183, 136), // #1332
  INST(Vpshrdq          , VexRvmi_Lx         , V(660F3A,73,_,x,_,1,4,FV ), 0                         , 100, 0  , 8681 , 184, 136), // #1333
  INST(Vpshrdvd         , VexRvm_Lx          , V(660F38,73,_,x,_,0,4,FV ), 0                         , 101, 0  , 8689 , 190, 136), // #1334
  INST(Vpshrdvq         , VexRvm_Lx          , V(660F38,73,_,x,_,1,4,FV ), 0                         , 102, 0  , 8698 , 191, 136), // #1335
  INST(Vpshrdvw         , VexRvm_Lx          , V(660F38,72,_,x,_,0,4,FVM), 0                         , 98 , 0  , 8707 , 189, 136), // #1336
  INST(Vpshrdw          , VexRvmi_Lx         , V(660F3A,72,_,x,_,0,4,FVM), 0                         , 129, 0  , 8716 , 237, 136), // #1337
  INST(Vpshufb          , VexRvm_Lx          , V(660F38,00,_,x,I,I,4,FVM), 0                         , 98 , 0  , 8724 , 272, 131), // #1338
  INST(Vpshufbitqmb     , VexRvm_Lx          , V(660F38,8F,_,x,0,0,4,FVM), 0                         , 98 , 0  , 8732 , 357, 142), // #1339
  INST(Vpshufd          , VexRmi_Lx          , V(660F00,70,_,x,I,0,4,FV ), 0                         , 118, 0  , 8745 , 358, 117), // #1340
  INST(Vpshufhw         , VexRmi_Lx          , V(F30F00,70,_,x,I,I,4,FVM), 0                         , 156, 0  , 8753 , 359, 131), // #1341
  INST(Vpshuflw         , VexRmi_Lx          , V(F20F00,70,_,x,I,I,4,FVM), 0                         , 158, 0  , 8762 , 359, 131), // #1342
  INST(Vpsignb          , VexRvm_Lx          , V(660F38,08,_,x,I,_,_,_  ), 0                         , 88 , 0  , 8771 , 179, 129), // #1343
  INST(Vpsignd          , VexRvm_Lx          , V(660F38,0A,_,x,I,_,_,_  ), 0                         , 88 , 0  , 8779 , 179, 129), // #1344
  INST(Vpsignw          , VexRvm_Lx          , V(660F38,09,_,x,I,_,_,_  ), 0                         , 88 , 0  , 8787 , 179, 129), // #1345
  INST(Vpslld           , VexRvmVmi_Lx       , V(660F00,F2,_,x,I,0,4,128), V(660F00,72,6,x,I,0,4,FV ), 179, 118, 8795 , 360, 117), // #1346
  INST(Vpslldq          , VexEvexVmi_Lx      , V(660F00,73,7,x,I,I,4,FVM), 0                         , 180, 0  , 8802 , 361, 131), // #1347
  INST(Vpsllq           , VexRvmVmi_Lx       , V(660F00,F3,_,x,I,1,4,128), V(660F00,73,6,x,I,1,4,FV ), 181, 119, 8810 , 362, 117), // #1348
  INST(Vpsllvd          , VexRvm_Lx          , V(660F38,47,_,x,0,0,4,FV ), 0                         , 101, 0  , 8817 , 186, 126), // #1349
  INST(Vpsllvq          , VexRvm_Lx          , V(660F38,47,_,x,1,1,4,FV ), 0                         , 139, 0  , 8825 , 185, 126), // #1350
  INST(Vpsllvw          , VexRvm_Lx          , V(660F38,12,_,x,_,1,4,FVM), 0                         , 103, 0  , 8833 , 189, 114), // #1351
  INST(Vpsllw           , VexRvmVmi_Lx       , V(660F00,F1,_,x,I,I,4,FVM), V(660F00,71,6,x,I,I,4,FVM), 153, 120, 8841 , 363, 131), // #1352
  INST(Vpsrad           , VexRvmVmi_Lx       , V(660F00,E2,_,x,I,0,4,128), V(660F00,72,4,x,I,0,4,FV ), 179, 121, 8848 , 360, 117), // #1353
  INST(Vpsraq           , VexRvmVmi_Lx       , V(660F00,E2,_,x,_,1,4,128), V(660F00,72,4,x,_,1,4,FV ), 181, 122, 8855 , 364, 112), // #1354
  INST(Vpsravd          , VexRvm_Lx          , V(660F38,46,_,x,0,0,4,FV ), 0                         , 101, 0  , 8862 , 186, 126), // #1355
  INST(Vpsravq          , VexRvm_Lx          , V(660F38,46,_,x,_,1,4,FV ), 0                         , 102, 0  , 8870 , 191, 112), // #1356
  INST(Vpsravw          , VexRvm_Lx          , V(660F38,11,_,x,_,1,4,FVM), 0                         , 103, 0  , 8878 , 189, 114), // #1357
  INST(Vpsraw           , VexRvmVmi_Lx       , V(660F00,E1,_,x,I,I,4,128), V(660F00,71,4,x,I,I,4,FVM), 179, 123, 8886 , 363, 131), // #1358
  INST(Vpsrld           , VexRvmVmi_Lx       , V(660F00,D2,_,x,I,0,4,128), V(660F00,72,2,x,I,0,4,FV ), 179, 124, 8893 , 360, 117), // #1359
  INST(Vpsrldq          , VexEvexVmi_Lx      , V(660F00,73,3,x,I,I,4,FVM), 0                         , 182, 0  , 8900 , 361, 131), // #1360
  INST(Vpsrlq           , VexRvmVmi_Lx       , V(660F00,D3,_,x,I,1,4,128), V(660F00,73,2,x,I,1,4,FV ), 181, 125, 8908 , 362, 117), // #1361
  INST(Vpsrlvd          , VexRvm_Lx          , V(660F38,45,_,x,0,0,4,FV ), 0                         , 101, 0  , 8915 , 186, 126), // #1362
  INST(Vpsrlvq          , VexRvm_Lx          , V(660F38,45,_,x,1,1,4,FV ), 0                         , 139, 0  , 8923 , 185, 126), // #1363
  INST(Vpsrlvw          , VexRvm_Lx          , V(660F38,10,_,x,_,1,4,FVM), 0                         , 103, 0  , 8931 , 189, 114), // #1364
  INST(Vpsrlw           , VexRvmVmi_Lx       , V(660F00,D1,_,x,I,I,4,128), V(660F00,71,2,x,I,I,4,FVM), 179, 126, 8939 , 363, 131), // #1365
  INST(Vpsubb           , VexRvm_Lx          , V(660F00,F8,_,x,I,I,4,FVM), 0                         , 153, 0  , 8946 , 365, 131), // #1366
  INST(Vpsubd           , VexRvm_Lx          , V(660F00,FA,_,x,I,0,4,FV ), 0                         , 118, 0  , 8953 , 366, 117), // #1367
  INST(Vpsubq           , VexRvm_Lx          , V(660F00,FB,_,x,I,1,4,FV ), 0                         , 93 , 0  , 8960 , 367, 117), // #1368
  INST(Vpsubsb          , VexRvm_Lx          , V(660F00,E8,_,x,I,I,4,FVM), 0                         , 153, 0  , 8967 , 365, 131), // #1369
  INST(Vpsubsw          , VexRvm_Lx          , V(660F00,E9,_,x,I,I,4,FVM), 0                         , 153, 0  , 8975 , 365, 131), // #1370
  INST(Vpsubusb         , VexRvm_Lx          , V(660F00,D8,_,x,I,I,4,FVM), 0                         , 153, 0  , 8983 , 365, 131), // #1371
  INST(Vpsubusw         , VexRvm_Lx          , V(660F00,D9,_,x,I,I,4,FVM), 0                         , 153, 0  , 8992 , 365, 131), // #1372
  INST(Vpsubw           , VexRvm_Lx          , V(660F00,F9,_,x,I,I,4,FVM), 0                         , 153, 0  , 9001 , 365, 131), // #1373
  INST(Vpternlogd       , VexRvmi_Lx         , V(660F3A,25,_,x,_,0,4,FV ), 0                         , 99 , 0  , 9008 , 183, 112), // #1374
  INST(Vpternlogq       , VexRvmi_Lx         , V(660F3A,25,_,x,_,1,4,FV ), 0                         , 100, 0  , 9019 , 184, 112), // #1375
  INST(Vptest           , VexRm_Lx           , V(660F38,17,_,x,I,_,_,_  ), 0                         , 88 , 0  , 9030 , 257, 135), // #1376
  INST(Vptestmb         , VexRvm_Lx          , V(660F38,26,_,x,_,0,4,FVM), 0                         , 98 , 0  , 9037 , 357, 114), // #1377
  INST(Vptestmd         , VexRvm_Lx          , V(660F38,27,_,x,_,0,4,FV ), 0                         , 101, 0  , 9046 , 368, 112), // #1378
  INST(Vptestmq         , VexRvm_Lx          , V(660F38,27,_,x,_,1,4,FV ), 0                         , 102, 0  , 9055 , 369, 112), // #1379
  INST(Vptestmw         , VexRvm_Lx          , V(660F38,26,_,x,_,1,4,FVM), 0                         , 103, 0  , 9064 , 357, 114), // #1380
  INST(Vptestnmb        , VexRvm_Lx          , V(F30F38,26,_,x,_,0,4,FVM), 0                         , 183, 0  , 9073 , 357, 114), // #1381
  INST(Vptestnmd        , VexRvm_Lx          , V(F30F38,27,_,x,_,0,4,FV ), 0                         , 184, 0  , 9083 , 368, 112), // #1382
  INST(Vptestnmq        , VexRvm_Lx          , V(F30F38,27,_,x,_,1,4,FV ), 0                         , 185, 0  , 9093 , 369, 112), // #1383
  INST(Vptestnmw        , VexRvm_Lx          , V(F30F38,26,_,x,_,1,4,FVM), 0                         , 186, 0  , 9103 , 357, 114), // #1384
  INST(Vpunpckhbw       , VexRvm_Lx          , V(660F00,68,_,x,I,I,4,FVM), 0                         , 153, 0  , 9113 , 272, 131), // #1385
  INST(Vpunpckhdq       , VexRvm_Lx          , V(660F00,6A,_,x,I,0,4,FV ), 0                         , 118, 0  , 9124 , 186, 117), // #1386
  INST(Vpunpckhqdq      , VexRvm_Lx          , V(660F00,6D,_,x,I,1,4,FV ), 0                         , 93 , 0  , 9135 , 185, 117), // #1387
  INST(Vpunpckhwd       , VexRvm_Lx          , V(660F00,69,_,x,I,I,4,FVM), 0                         , 153, 0  , 9147 , 272, 131), // #1388
  INST(Vpunpcklbw       , VexRvm_Lx          , V(660F00,60,_,x,I,I,4,FVM), 0                         , 153, 0  , 9158 , 272, 131), // #1389
  INST(Vpunpckldq       , VexRvm_Lx          , V(660F00,62,_,x,I,0,4,FV ), 0                         , 118, 0  , 9169 , 186, 117), // #1390
  INST(Vpunpcklqdq      , VexRvm_Lx          , V(660F00,6C,_,x,I,1,4,FV ), 0                         , 93 , 0  , 9180 , 185, 117), // #1391
  INST(Vpunpcklwd       , VexRvm_Lx          , V(660F00,61,_,x,I,I,4,FVM), 0                         , 153, 0  , 9192 , 272, 131), // #1392
  INST(Vpxor            , VexRvm_Lx          , V(660F00,EF,_,x,I,_,_,_  ), 0                         , 63 , 0  , 9203 , 304, 129), // #1393
  INST(Vpxord           , VexRvm_Lx          , V(660F00,EF,_,x,_,0,4,FV ), 0                         , 118, 0  , 9209 , 305, 112), // #1394
  INST(Vpxorq           , VexRvm_Lx          , V(660F00,EF,_,x,_,1,4,FV ), 0                         , 93 , 0  , 9216 , 306, 112), // #1395
  INST(Vrangepd         , VexRvmi_Lx         , V(660F3A,50,_,x,_,1,4,FV ), 0                         , 100, 0  , 9223 , 246, 115), // #1396
  INST(Vrangeps         , VexRvmi_Lx         , V(660F3A,50,_,x,_,0,4,FV ), 0                         , 99 , 0  , 9232 , 247, 115), // #1397
  INST(Vrangesd         , VexRvmi            , V(660F3A,51,_,I,_,1,3,T1S), 0                         , 138, 0  , 9241 , 248, 62 ), // #1398
  INST(Vrangess         , VexRvmi            , V(660F3A,51,_,I,_,0,2,T1S), 0                         , 137, 0  , 9250 , 249, 62 ), // #1399
  INST(Vrcp14pd         , VexRm_Lx           , V(660F38,4C,_,x,_,1,4,FV ), 0                         , 102, 0  , 9259 , 339, 112), // #1400
  INST(Vrcp14ps         , VexRm_Lx           , V(660F38,4C,_,x,_,0,4,FV ), 0                         , 101, 0  , 9268 , 324, 112), // #1401
  INST(Vrcp14sd         , VexRvm             , V(660F38,4D,_,I,_,1,3,T1S), 0                         , 110, 0  , 9277 , 370, 64 ), // #1402
  INST(Vrcp14ss         , VexRvm             , V(660F38,4D,_,I,_,0,2,T1S), 0                         , 111, 0  , 9286 , 371, 64 ), // #1403
  INST(Vrcp28pd         , VexRm              , V(660F38,CA,_,2,_,1,4,FV ), 0                         , 130, 0  , 9295 , 239, 121), // #1404
  INST(Vrcp28ps         , VexRm              , V(660F38,CA,_,2,_,0,4,FV ), 0                         , 131, 0  , 9304 , 240, 121), // #1405
  INST(Vrcp28sd         , VexRvm             , V(660F38,CB,_,I,_,1,3,T1S), 0                         , 110, 0  , 9313 , 267, 121), // #1406
  INST(Vrcp28ss         , VexRvm             , V(660F38,CB,_,I,_,0,2,T1S), 0                         , 111, 0  , 9322 , 268, 121), // #1407
  INST(Vrcpps           , VexRm_Lx           , V(000F00,53,_,x,I,_,_,_  ), 0                         , 66 , 0  , 9331 , 257, 109), // #1408
  INST(Vrcpss           , VexRvm             , V(F30F00,53,_,I,I,_,_,_  ), 0                         , 154, 0  , 9338 , 372, 109), // #1409
  INST(Vreducepd        , VexRmi_Lx          , V(660F3A,56,_,x,_,1,4,FV ), 0                         , 100, 0  , 9345 , 351, 115), // #1410
  INST(Vreduceps        , VexRmi_Lx          , V(660F3A,56,_,x,_,0,4,FV ), 0                         , 99 , 0  , 9355 , 350, 115), // #1411
  INST(Vreducesd        , VexRvmi            , V(660F3A,57,_,I,_,1,3,T1S), 0                         , 138, 0  , 9365 , 373, 62 ), // #1412
  INST(Vreducess        , VexRvmi            , V(660F3A,57,_,I,_,0,2,T1S), 0                         , 137, 0  , 9375 , 374, 62 ), // #1413
  INST(Vrndscalepd      , VexRmi_Lx          , V(660F3A,09,_,x,_,1,4,FV ), 0                         , 100, 0  , 9385 , 269, 112), // #1414
  INST(Vrndscaleps      , VexRmi_Lx          , V(660F3A,08,_,x,_,0,4,FV ), 0                         , 99 , 0  , 9397 , 270, 112), // #1415
  INST(Vrndscalesd      , VexRvmi            , V(660F3A,0B,_,I,_,1,3,T1S), 0                         , 138, 0  , 9409 , 248, 64 ), // #1416
  INST(Vrndscaless      , VexRvmi            , V(660F3A,0A,_,I,_,0,2,T1S), 0                         , 137, 0  , 9421 , 249, 64 ), // #1417
  INST(Vroundpd         , VexRmi_Lx          , V(660F3A,09,_,x,I,_,_,_  ), 0                         , 67 , 0  , 9433 , 375, 109), // #1418
  INST(Vroundps         , VexRmi_Lx          , V(660F3A,08,_,x,I,_,_,_  ), 0                         , 67 , 0  , 9442 , 375, 109), // #1419
  INST(Vroundsd         , VexRvmi            , V(660F3A,0B,_,I,I,_,_,_  ), 0                         , 67 , 0  , 9451 , 376, 109), // #1420
  INST(Vroundss         , VexRvmi            , V(660F3A,0A,_,I,I,_,_,_  ), 0                         , 67 , 0  , 9460 , 377, 109), // #1421
  INST(Vrsqrt14pd       , VexRm_Lx           , V(660F38,4E,_,x,_,1,4,FV ), 0                         , 102, 0  , 9469 , 339, 112), // #1422
  INST(Vrsqrt14ps       , VexRm_Lx           , V(660F38,4E,_,x,_,0,4,FV ), 0                         , 101, 0  , 9480 , 324, 112), // #1423
  INST(Vrsqrt14sd       , VexRvm             , V(660F38,4F,_,I,_,1,3,T1S), 0                         , 110, 0  , 9491 , 370, 64 ), // #1424
  INST(Vrsqrt14ss       , VexRvm             , V(660F38,4F,_,I,_,0,2,T1S), 0                         , 111, 0  , 9502 , 371, 64 ), // #1425
  INST(Vrsqrt28pd       , VexRm              , V(660F38,CC,_,2,_,1,4,FV ), 0                         , 130, 0  , 9513 , 239, 121), // #1426
  INST(Vrsqrt28ps       , VexRm              , V(660F38,CC,_,2,_,0,4,FV ), 0                         , 131, 0  , 9524 , 240, 121), // #1427
  INST(Vrsqrt28sd       , VexRvm             , V(660F38,CD,_,I,_,1,3,T1S), 0                         , 110, 0  , 9535 , 267, 121), // #1428
  INST(Vrsqrt28ss       , VexRvm             , V(660F38,CD,_,I,_,0,2,T1S), 0                         , 111, 0  , 9546 , 268, 121), // #1429
  INST(Vrsqrtps         , VexRm_Lx           , V(000F00,52,_,x,I,_,_,_  ), 0                         , 66 , 0  , 9557 , 257, 109), // #1430
  INST(Vrsqrtss         , VexRvm             , V(F30F00,52,_,I,I,_,_,_  ), 0                         , 154, 0  , 9566 , 372, 109), // #1431
  INST(Vscalefpd        , VexRvm_Lx          , V(660F38,2C,_,x,_,1,4,FV ), 0                         , 102, 0  , 9575 , 378, 112), // #1432
  INST(Vscalefps        , VexRvm_Lx          , V(660F38,2C,_,x,_,0,4,FV ), 0                         , 101, 0  , 9585 , 379, 112), // #1433
  INST(Vscalefsd        , VexRvm             , V(660F38,2D,_,I,_,1,3,T1S), 0                         , 110, 0  , 9595 , 380, 64 ), // #1434
  INST(Vscalefss        , VexRvm             , V(660F38,2D,_,I,_,0,2,T1S), 0                         , 111, 0  , 9605 , 381, 64 ), // #1435
  INST(Vscatterdpd      , VexMr_Lx           , V(660F38,A2,_,x,_,1,3,T1S), 0                         , 110, 0  , 9615 , 382, 112), // #1436
  INST(Vscatterdps      , VexMr_Lx           , V(660F38,A2,_,x,_,0,2,T1S), 0                         , 111, 0  , 9627 , 353, 112), // #1437
  INST(Vscatterpf0dpd   , VexM_VM            , V(660F38,C6,5,2,_,1,3,T1S), 0                         , 187, 0  , 9639 , 262, 127), // #1438
  INST(Vscatterpf0dps   , VexM_VM            , V(660F38,C6,5,2,_,0,2,T1S), 0                         , 188, 0  , 9654 , 263, 127), // #1439
  INST(Vscatterpf0qpd   , VexM_VM            , V(660F38,C7,5,2,_,1,3,T1S), 0                         , 187, 0  , 9669 , 264, 127), // #1440
  INST(Vscatterpf0qps   , VexM_VM            , V(660F38,C7,5,2,_,0,2,T1S), 0                         , 188, 0  , 9684 , 264, 127), // #1441
  INST(Vscatterpf1dpd   , VexM_VM            , V(660F38,C6,6,2,_,1,3,T1S), 0                         , 189, 0  , 9699 , 262, 127), // #1442
  INST(Vscatterpf1dps   , VexM_VM            , V(660F38,C6,6,2,_,0,2,T1S), 0                         , 190, 0  , 9714 , 263, 127), // #1443
  INST(Vscatterpf1qpd   , VexM_VM            , V(660F38,C7,6,2,_,1,3,T1S), 0                         , 189, 0  , 9729 , 264, 127), // #1444
  INST(Vscatterpf1qps   , VexM_VM            , V(660F38,C7,6,2,_,0,2,T1S), 0                         , 190, 0  , 9744 , 264, 127), // #1445
  INST(Vscatterqpd      , VexMr_Lx           , V(660F38,A3,_,x,_,1,3,T1S), 0                         , 110, 0  , 9759 , 355, 112), // #1446
  INST(Vscatterqps      , VexMr_Lx           , V(660F38,A3,_,x,_,0,2,T1S), 0                         , 111, 0  , 9771 , 354, 112), // #1447
  INST(Vshuff32x4       , VexRvmi_Lx         , V(660F3A,23,_,x,_,0,4,FV ), 0                         , 99 , 0  , 9783 , 383, 112), // #1448
  INST(Vshuff64x2       , VexRvmi_Lx         , V(660F3A,23,_,x,_,1,4,FV ), 0                         , 100, 0  , 9794 , 384, 112), // #1449
  INST(Vshufi32x4       , VexRvmi_Lx         , V(660F3A,43,_,x,_,0,4,FV ), 0                         , 99 , 0  , 9805 , 383, 112), // #1450
  INST(Vshufi64x2       , VexRvmi_Lx         , V(660F3A,43,_,x,_,1,4,FV ), 0                         , 100, 0  , 9816 , 384, 112), // #1451
  INST(Vshufpd          , VexRvmi_Lx         , V(660F00,C6,_,x,I,1,4,FV ), 0                         , 93 , 0  , 9827 , 385, 107), // #1452
  INST(Vshufps          , VexRvmi_Lx         , V(000F00,C6,_,x,I,0,4,FV ), 0                         , 94 , 0  , 9835 , 386, 107), // #1453
  INST(Vsqrtpd          , VexRm_Lx           , V(660F00,51,_,x,I,1,4,FV ), 0                         , 93 , 0  , 9843 , 387, 107), // #1454
  INST(Vsqrtps          , VexRm_Lx           , V(000F00,51,_,x,I,0,4,FV ), 0                         , 94 , 0  , 9851 , 209, 107), // #1455
  INST(Vsqrtsd          , VexRvm             , V(F20F00,51,_,I,I,1,3,T1S), 0                         , 95 , 0  , 9859 , 177, 108), // #1456
  INST(Vsqrtss          , VexRvm             , V(F30F00,51,_,I,I,0,2,T1S), 0                         , 96 , 0  , 9867 , 178, 108), // #1457
  INST(Vstmxcsr         , VexM               , V(000F00,AE,3,0,I,_,_,_  ), 0                         , 191, 0  , 9875 , 278, 109), // #1458
  INST(Vsubpd           , VexRvm_Lx          , V(660F00,5C,_,x,I,1,4,FV ), 0                         , 93 , 0  , 9884 , 175, 107), // #1459
  INST(Vsubps           , VexRvm_Lx          , V(000F00,5C,_,x,I,0,4,FV ), 0                         , 94 , 0  , 9891 , 176, 107), // #1460
  INST(Vsubsd           , VexRvm             , V(F20F00,5C,_,I,I,1,3,T1S), 0                         , 95 , 0  , 9898 , 177, 108), // #1461
  INST(Vsubss           , VexRvm             , V(F30F00,5C,_,I,I,0,2,T1S), 0                         , 96 , 0  , 9905 , 178, 108), // #1462
  INST(Vtestpd          , VexRm_Lx           , V(660F38,0F,_,x,0,_,_,_  ), 0                         , 88 , 0  , 9912 , 257, 135), // #1463
  INST(Vtestps          , VexRm_Lx           , V(660F38,0E,_,x,0,_,_,_  ), 0                         , 88 , 0  , 9920 , 257, 135), // #1464
  INST(Vucomisd         , VexRm              , V(660F00,2E,_,I,I,1,3,T1S), 0                         , 112, 0  , 9928 , 205, 118), // #1465
  INST(Vucomiss         , VexRm              , V(000F00,2E,_,I,I,0,2,T1S), 0                         , 113, 0  , 9937 , 206, 118), // #1466
  INST(Vunpckhpd        , VexRvm_Lx          , V(660F00,15,_,x,I,1,4,FV ), 0                         , 93 , 0  , 9946 , 185, 107), // #1467
  INST(Vunpckhps        , VexRvm_Lx          , V(000F00,15,_,x,I,0,4,FV ), 0                         , 94 , 0  , 9956 , 186, 107), // #1468
  INST(Vunpcklpd        , VexRvm_Lx          , V(660F00,14,_,x,I,1,4,FV ), 0                         , 93 , 0  , 9966 , 185, 107), // #1469
  INST(Vunpcklps        , VexRvm_Lx          , V(000F00,14,_,x,I,0,4,FV ), 0                         , 94 , 0  , 9976 , 186, 107), // #1470
  INST(Vxorpd           , VexRvm_Lx          , V(660F00,57,_,x,I,1,4,FV ), 0                         , 93 , 0  , 9986 , 367, 113), // #1471
  INST(Vxorps           , VexRvm_Lx          , V(000F00,57,_,x,I,0,4,FV ), 0                         , 94 , 0  , 9993 , 366, 113), // #1472
  INST(Vzeroall         , VexOp              , V(000F00,77,_,1,I,_,_,_  ), 0                         , 62 , 0  , 10000, 388, 109), // #1473
  INST(Vzeroupper       , VexOp              , V(000F00,77,_,0,I,_,_,_  ), 0                         , 66 , 0  , 10009, 388, 109), // #1474
  INST(Wbinvd           , X86Op              , O(000F00,09,_,_,_,_,_,_  ), 0                         , 4  , 0  , 10020, 30 , 0  ), // #1475
  INST(Wbnoinvd         , X86Op              , O(F30F00,09,_,_,_,_,_,_  ), 0                         , 6  , 0  , 10027, 30 , 144), // #1476
  INST(Wrfsbase         , X86M               , O(F30F00,AE,2,_,x,_,_,_  ), 0                         , 192, 0  , 10036, 160, 95 ), // #1477
  INST(Wrgsbase         , X86M               , O(F30F00,AE,3,_,x,_,_,_  ), 0                         , 193, 0  , 10045, 160, 95 ), // #1478
  INST(Wrmsr            , X86Op              , O(000F00,30,_,_,_,_,_,_  ), 0                         , 4  , 0  , 10054, 161, 96 ), // #1479
  INST(Xabort           , X86Op_O_I8         , O(000000,C6,7,_,_,_,_,_  ), 0                         , 25 , 0  , 10060, 74 , 145), // #1480
  INST(Xadd             , X86Xadd            , O(000F00,C0,_,_,x,_,_,_  ), 0                         , 4  , 0  , 10067, 389, 146), // #1481
  INST(Xbegin           , X86JmpRel          , O(000000,C7,7,_,_,_,_,_  ), 0                         , 25 , 0  , 10072, 390, 145), // #1482
  INST(Xchg             , X86Xchg            , O(000000,86,_,_,x,_,_,_  ), 0                         , 0  , 0  , 448  , 391, 0  ), // #1483
  INST(Xend             , X86Op              , O(000F01,D5,_,_,_,_,_,_  ), 0                         , 21 , 0  , 10079, 30 , 145), // #1484
  INST(Xgetbv           , X86Op              , O(000F01,D0,_,_,_,_,_,_  ), 0                         , 21 , 0  , 10084, 161, 147), // #1485
  INST(Xlatb            , X86Op              , O(000000,D7,_,_,_,_,_,_  ), 0                         , 0  , 0  , 10091, 30 , 0  ), // #1486
  INST(Xor              , X86Arith           , O(000000,30,6,_,x,_,_,_  ), 0                         , 30 , 0  , 9205 , 171, 1  ), // #1487
  INST(Xorpd            , ExtRm              , O(660F00,57,_,_,_,_,_,_  ), 0                         , 3  , 0  , 9987 , 139, 4  ), // #1488
  INST(Xorps            , ExtRm              , O(000F00,57,_,_,_,_,_,_  ), 0                         , 4  , 0  , 9994 , 139, 5  ), // #1489
  INST(Xrstor           , X86M_Only          , O(000F00,AE,5,_,_,_,_,_  ), 0                         , 70 , 0  , 1134 , 392, 147), // #1490
  INST(Xrstor64         , X86M_Only          , O(000F00,AE,5,_,1,_,_,_  ), 0                         , 194, 0  , 1142 , 393, 147), // #1491
  INST(Xrstors          , X86M_Only          , O(000F00,C7,3,_,_,_,_,_  ), 0                         , 71 , 0  , 10097, 392, 148), // #1492
  INST(Xrstors64        , X86M_Only          , O(000F00,C7,3,_,1,_,_,_  ), 0                         , 195, 0  , 10105, 393, 148), // #1493
  INST(Xsave            , X86M_Only          , O(000F00,AE,4,_,_,_,_,_  ), 0                         , 89 , 0  , 1152 , 392, 147), // #1494
  INST(Xsave64          , X86M_Only          , O(000F00,AE,4,_,1,_,_,_  ), 0                         , 196, 0  , 1159 , 393, 147), // #1495
  INST(Xsavec           , X86M_Only          , O(000F00,C7,4,_,_,_,_,_  ), 0                         , 89 , 0  , 10115, 392, 149), // #1496
  INST(Xsavec64         , X86M_Only          , O(000F00,C7,4,_,1,_,_,_  ), 0                         , 196, 0  , 10122, 393, 149), // #1497
  INST(Xsaveopt         , X86M_Only          , O(000F00,AE,6,_,_,_,_,_  ), 0                         , 73 , 0  , 10131, 392, 150), // #1498
  INST(Xsaveopt64       , X86M_Only          , O(000F00,AE,6,_,1,_,_,_  ), 0                         , 197, 0  , 10140, 393, 150), // #1499
  INST(Xsaves           , X86M_Only          , O(000F00,C7,5,_,_,_,_,_  ), 0                         , 70 , 0  , 10151, 392, 148), // #1500
  INST(Xsaves64         , X86M_Only          , O(000F00,C7,5,_,1,_,_,_  ), 0                         , 194, 0  , 10158, 393, 148), // #1501
  INST(Xsetbv           , X86Op              , O(000F01,D1,_,_,_,_,_,_  ), 0                         , 21 , 0  , 10167, 161, 147), // #1502
  INST(Xtest            , X86Op              , O(000F01,D6,_,_,_,_,_,_  ), 0                         , 21 , 0  , 10174, 30 , 151)  // #1503
  // ${InstInfo:End}
};
#undef NAME_DATA_INDEX
#undef INST

// ============================================================================
// [asmjit::x86::InstDB - Opcode Tables]
// ============================================================================

// ${MainOpcodeTable:Begin}
// ------------------- Automatically generated, do not edit -------------------
const uint32_t InstDB::_mainOpcodeTable[] = {
  O(000000,00,0,0,0,0,0,_  ), // #0 [ref=55x]
  O(000000,00,2,0,0,0,0,_  ), // #1 [ref=4x]
  O(660F38,00,0,0,0,0,0,_  ), // #2 [ref=42x]
  O(660F00,00,0,0,0,0,0,_  ), // #3 [ref=38x]
  O(000F00,00,0,0,0,0,0,_  ), // #4 [ref=231x]
  O(F20F00,00,0,0,0,0,0,_  ), // #5 [ref=24x]
  O(F30F00,00,0,0,0,0,0,_  ), // #6 [ref=29x]
  O(F30F38,00,0,0,0,0,0,_  ), // #7 [ref=2x]
  O(660F3A,00,0,0,0,0,0,_  ), // #8 [ref=22x]
  O(000000,00,4,0,0,0,0,_  ), // #9 [ref=5x]
  V(000F38,00,0,0,0,0,0,_  ), // #10 [ref=3x]
  V(XOP_M9,00,1,0,0,0,0,_  ), // #11 [ref=3x]
  V(XOP_M9,00,6,0,0,0,0,_  ), // #12 [ref=2x]
  V(XOP_M9,00,5,0,0,0,0,_  ), // #13 [ref=1x]
  V(XOP_M9,00,3,0,0,0,0,_  ), // #14 [ref=1x]
  V(XOP_M9,00,2,0,0,0,0,_  ), // #15 [ref=1x]
  V(000F38,00,3,0,0,0,0,_  ), // #16 [ref=1x]
  V(000F38,00,2,0,0,0,0,_  ), // #17 [ref=1x]
  V(000F38,00,1,0,0,0,0,_  ), // #18 [ref=1x]
  O(660000,00,0,0,0,0,0,_  ), // #19 [ref=7x]
  O(000000,00,0,0,1,0,0,_  ), // #20 [ref=4x]
  O(000F01,00,0,0,0,0,0,_  ), // #21 [ref=25x]
  O(000F00,00,7,0,0,0,0,_  ), // #22 [ref=5x]
  O(660F00,00,7,0,0,0,0,_  ), // #23 [ref=2x]
  O(660F00,00,6,0,0,0,0,_  ), // #24 [ref=2x]
  O(000000,00,7,0,0,0,0,_  ), // #25 [ref=5x]
  O(000F00,00,1,0,1,0,0,_  ), // #26 [ref=2x]
  O(000F00,00,1,0,0,0,0,_  ), // #27 [ref=6x]
  O(F20F38,00,0,0,0,0,0,_  ), // #28 [ref=2x]
  O(000000,00,1,0,0,0,0,_  ), // #29 [ref=3x]
  O(000000,00,6,0,0,0,0,_  ), // #30 [ref=3x]
  O_FPU(00,D900,_)          , // #31 [ref=29x]
  O_FPU(00,C000,0)          , // #32 [ref=1x]
  O_FPU(00,DE00,_)          , // #33 [ref=7x]
  O_FPU(00,0000,4)          , // #34 [ref=4x]
  O_FPU(00,0000,6)          , // #35 [ref=4x]
  O_FPU(9B,DB00,_)          , // #36 [ref=2x]
  O_FPU(00,DA00,_)          , // #37 [ref=5x]
  O_FPU(00,DB00,_)          , // #38 [ref=8x]
  O_FPU(00,D000,2)          , // #39 [ref=1x]
  O_FPU(00,DF00,_)          , // #40 [ref=2x]
  O_FPU(00,D800,3)          , // #41 [ref=1x]
  O_FPU(00,F000,6)          , // #42 [ref=1x]
  O_FPU(00,F800,7)          , // #43 [ref=1x]
  O_FPU(00,DD00,_)          , // #44 [ref=3x]
  O_FPU(00,0000,0)          , // #45 [ref=3x]
  O_FPU(00,0000,2)          , // #46 [ref=3x]
  O_FPU(00,0000,3)          , // #47 [ref=3x]
  O_FPU(00,0000,7)          , // #48 [ref=3x]
  O_FPU(00,0000,1)          , // #49 [ref=2x]
  O_FPU(00,0000,5)          , // #50 [ref=2x]
  O_FPU(00,C800,1)          , // #51 [ref=1x]
  O_FPU(9B,0000,6)          , // #52 [ref=2x]
  O_FPU(9B,0000,7)          , // #53 [ref=2x]
  O_FPU(00,E000,4)          , // #54 [ref=1x]
  O_FPU(00,E800,5)          , // #55 [ref=1x]
  O_FPU(00,0000,_)          , // #56 [ref=1x]
  O(000F00,00,0,0,1,0,0,_  ), // #57 [ref=1x]
  O(000000,00,5,0,0,0,0,_  ), // #58 [ref=3x]
  V(660F00,00,0,1,0,0,0,_  ), // #59 [ref=7x]
  V(660F00,00,0,1,1,0,0,_  ), // #60 [ref=6x]
  V(000F00,00,0,1,1,0,0,_  ), // #61 [ref=7x]
  V(000F00,00,0,1,0,0,0,_  ), // #62 [ref=8x]
  V(660F00,00,0,0,0,0,0,_  ), // #63 [ref=15x]
  V(660F00,00,0,0,1,0,0,_  ), // #64 [ref=4x]
  V(000F00,00,0,0,1,0,0,_  ), // #65 [ref=4x]
  V(000F00,00,0,0,0,0,0,_  ), // #66 [ref=10x]
  V(660F3A,00,0,0,0,0,0,_  ), // #67 [ref=45x]
  V(660F3A,00,0,0,1,0,0,_  ), // #68 [ref=4x]
  O(000F00,00,2,0,0,0,0,_  ), // #69 [ref=5x]
  O(000F00,00,5,0,0,0,0,_  ), // #70 [ref=4x]
  O(000F00,00,3,0,0,0,0,_  ), // #71 [ref=5x]
  V(XOP_M9,00,0,0,0,0,0,_  ), // #72 [ref=32x]
  O(000F00,00,6,0,0,0,0,_  ), // #73 [ref=5x]
  V(XOP_MA,00,0,0,0,0,0,_  ), // #74 [ref=1x]
  V(XOP_MA,00,1,0,0,0,0,_  ), // #75 [ref=1x]
  O(000F38,00,0,0,0,0,0,_  ), // #76 [ref=23x]
  V(F20F38,00,0,0,0,0,0,_  ), // #77 [ref=4x]
  O(000000,00,3,0,0,0,0,_  ), // #78 [ref=3x]
  O(000F3A,00,0,0,0,0,0,_  ), // #79 [ref=4x]
  O(F30000,00,0,0,0,0,0,_  ), // #80 [ref=1x]
  O(000F0F,00,0,0,0,0,0,_  ), // #81 [ref=26x]
  V(F30F38,00,0,0,0,0,0,_  ), // #82 [ref=9x]
  O(000F3A,00,0,0,1,0,0,_  ), // #83 [ref=1x]
  O(660F3A,00,0,0,1,0,0,_  ), // #84 [ref=1x]
  O(F30F00,00,1,0,0,0,0,_  ), // #85 [ref=1x]
  O(F30F00,00,7,0,0,0,0,_  ), // #86 [ref=1x]
  V(F20F3A,00,0,0,0,0,0,_  ), // #87 [ref=1x]
  V(660F38,00,0,0,0,0,0,_  ), // #88 [ref=22x]
  O(000F00,00,4,0,0,0,0,_  ), // #89 [ref=4x]
  V(XOP_M9,00,7,0,0,0,0,_  ), // #90 [ref=1x]
  V(XOP_M9,00,4,0,0,0,0,_  ), // #91 [ref=1x]
  V(F20F38,00,0,2,0,0,2,T4X), // #92 [ref=6x]
  V(660F00,00,0,0,0,1,4,FV ), // #93 [ref=31x]
  V(000F00,00,0,0,0,0,4,FV ), // #94 [ref=18x]
  V(F20F00,00,0,0,0,1,3,T1S), // #95 [ref=10x]
  V(F30F00,00,0,0,0,0,2,T1S), // #96 [ref=10x]
  V(F20F00,00,0,0,0,0,0,_  ), // #97 [ref=4x]
  V(660F38,00,0,0,0,0,4,FVM), // #98 [ref=23x]
  V(660F3A,00,0,0,0,0,4,FV ), // #99 [ref=14x]
  V(660F3A,00,0,0,0,1,4,FV ), // #100 [ref=14x]
  V(660F38,00,0,0,0,0,4,FV ), // #101 [ref=48x]
  V(660F38,00,0,0,0,1,4,FV ), // #102 [ref=31x]
  V(660F38,00,0,0,0,1,4,FVM), // #103 [ref=11x]
  V(660F38,00,0,1,0,0,0,_  ), // #104 [ref=3x]
  V(660F38,00,0,0,0,0,3,T2 ), // #105 [ref=2x]
  V(660F38,00,0,0,0,0,4,T4 ), // #106 [ref=2x]
  V(660F38,00,0,2,0,0,5,T8 ), // #107 [ref=2x]
  V(660F38,00,0,0,0,1,4,T2 ), // #108 [ref=2x]
  V(660F38,00,0,2,0,1,5,T4 ), // #109 [ref=2x]
  V(660F38,00,0,0,0,1,3,T1S), // #110 [ref=16x]
  V(660F38,00,0,0,0,0,2,T1S), // #111 [ref=28x]
  V(660F00,00,0,0,0,1,3,T1S), // #112 [ref=5x]
  V(000F00,00,0,0,0,0,2,T1S), // #113 [ref=2x]
  V(F30F00,00,0,0,0,0,3,HV ), // #114 [ref=2x]
  V(F20F00,00,0,0,0,1,4,FV ), // #115 [ref=2x]
  V(000F00,00,0,0,0,1,4,FV ), // #116 [ref=3x]
  V(660F38,00,0,0,0,0,3,HVM), // #117 [ref=7x]
  V(660F00,00,0,0,0,0,4,FV ), // #118 [ref=16x]
  V(000F00,00,0,0,0,0,4,HV ), // #119 [ref=1x]
  V(660F3A,00,0,0,0,0,3,HVM), // #120 [ref=1x]
  V(660F00,00,0,0,0,0,3,HV ), // #121 [ref=4x]
  V(F30F00,00,0,0,0,1,4,FV ), // #122 [ref=2x]
  V(F20F00,00,0,0,0,0,3,T1F), // #123 [ref=4x]
  V(F20F00,00,0,0,0,0,2,T1W), // #124 [ref=2x]
  V(F30F00,00,0,0,0,0,2,T1W), // #125 [ref=2x]
  V(F30F00,00,0,0,0,0,2,T1F), // #126 [ref=4x]
  V(F30F00,00,0,0,0,0,4,FV ), // #127 [ref=1x]
  V(F20F00,00,0,0,0,0,4,FV ), // #128 [ref=1x]
  V(660F3A,00,0,0,0,0,4,FVM), // #129 [ref=7x]
  V(660F38,00,0,2,0,1,4,FV ), // #130 [ref=3x]
  V(660F38,00,0,2,0,0,4,FV ), // #131 [ref=3x]
  V(660F3A,00,0,1,0,0,0,_  ), // #132 [ref=6x]
  V(660F3A,00,0,0,0,0,4,T4 ), // #133 [ref=4x]
  V(660F3A,00,0,2,0,0,5,T8 ), // #134 [ref=4x]
  V(660F3A,00,0,0,0,1,4,T2 ), // #135 [ref=4x]
  V(660F3A,00,0,2,0,1,5,T4 ), // #136 [ref=4x]
  V(660F3A,00,0,0,0,0,2,T1S), // #137 [ref=10x]
  V(660F3A,00,0,0,0,1,3,T1S), // #138 [ref=6x]
  V(660F38,00,0,0,1,1,4,FV ), // #139 [ref=20x]
  V(660F38,00,0,0,1,1,3,T1S), // #140 [ref=12x]
  V(660F38,00,0,0,1,0,0,_  ), // #141 [ref=5x]
  V(660F38,00,1,2,0,1,3,T1S), // #142 [ref=2x]
  V(660F38,00,1,2,0,0,2,T1S), // #143 [ref=2x]
  V(660F38,00,2,2,0,1,3,T1S), // #144 [ref=2x]
  V(660F38,00,2,2,0,0,2,T1S), // #145 [ref=2x]
  V(660F3A,00,0,0,1,1,0,FV ), // #146 [ref=2x]
  V(660F38,00,0,0,0,0,0,FV ), // #147 [ref=5x]
  V(000F00,00,2,0,0,0,0,_  ), // #148 [ref=1x]
  V(660F00,00,0,0,0,1,4,FVM), // #149 [ref=4x]
  V(000F00,00,0,0,0,0,4,FVM), // #150 [ref=3x]
  V(660F00,00,0,0,0,0,2,T1S), // #151 [ref=1x]
  V(F20F00,00,0,0,0,1,3,DUP), // #152 [ref=1x]
  V(660F00,00,0,0,0,0,4,FVM), // #153 [ref=34x]
  V(F30F00,00,0,0,0,0,0,_  ), // #154 [ref=3x]
  V(F20F00,00,0,0,0,1,4,FVM), // #155 [ref=1x]
  V(F30F00,00,0,0,0,0,4,FVM), // #156 [ref=4x]
  V(F30F00,00,0,0,0,1,4,FVM), // #157 [ref=1x]
  V(F20F00,00,0,0,0,0,4,FVM), // #158 [ref=2x]
  V(000F00,00,0,0,0,0,3,T2 ), // #159 [ref=2x]
  O(F30F00,00,6,0,0,0,0,_  ), // #160 [ref=1x]
  V(660F38,00,0,0,0,0,0,T1S), // #161 [ref=3x]
  V(F30F38,00,0,0,0,1,0,_  ), // #162 [ref=5x]
  V(660F38,00,0,0,0,0,1,T1S), // #163 [ref=1x]
  V(XOP_M8,00,0,0,0,0,0,_  ), // #164 [ref=22x]
  V(660F3A,00,0,0,0,1,4,FVM), // #165 [ref=2x]
  V(660F38,00,0,0,0,1,1,T1S), // #166 [ref=2x]
  V(660F3A,00,0,1,1,0,0,_  ), // #167 [ref=1x]
  V(660F3A,00,0,0,0,0,0,T1S), // #168 [ref=2x]
  V(660F3A,00,0,0,1,1,3,T1S), // #169 [ref=2x]
  V(660F3A,00,0,0,0,0,1,T1S), // #170 [ref=1x]
  V(660F00,00,0,0,0,0,1,T1S), // #171 [ref=1x]
  V(F30F38,00,0,0,0,0,2,QVM), // #172 [ref=6x]
  V(F30F38,00,0,0,0,0,3,HVM), // #173 [ref=9x]
  V(F30F38,00,0,0,0,0,1,OVM), // #174 [ref=3x]
  V(660F38,00,0,0,0,0,2,QVM), // #175 [ref=4x]
  V(660F38,00,0,0,0,0,1,OVM), // #176 [ref=2x]
  V(660F00,00,1,0,0,0,4,FV ), // #177 [ref=1x]
  V(660F00,00,1,0,0,1,4,FV ), // #178 [ref=1x]
  V(660F00,00,0,0,0,0,4,128), // #179 [ref=5x]
  V(660F00,00,7,0,0,0,4,FVM), // #180 [ref=1x]
  V(660F00,00,0,0,0,1,4,128), // #181 [ref=3x]
  V(660F00,00,3,0,0,0,4,FVM), // #182 [ref=1x]
  V(F30F38,00,0,0,0,0,4,FVM), // #183 [ref=1x]
  V(F30F38,00,0,0,0,0,4,FV ), // #184 [ref=1x]
  V(F30F38,00,0,0,0,1,4,FV ), // #185 [ref=1x]
  V(F30F38,00,0,0,0,1,4,FVM), // #186 [ref=1x]
  V(660F38,00,5,2,0,1,3,T1S), // #187 [ref=2x]
  V(660F38,00,5,2,0,0,2,T1S), // #188 [ref=2x]
  V(660F38,00,6,2,0,1,3,T1S), // #189 [ref=2x]
  V(660F38,00,6,2,0,0,2,T1S), // #190 [ref=2x]
  V(000F00,00,3,0,0,0,0,_  ), // #191 [ref=1x]
  O(F30F00,00,2,0,0,0,0,_  ), // #192 [ref=1x]
  O(F30F00,00,3,0,0,0,0,_  ), // #193 [ref=1x]
  O(000F00,00,5,0,1,0,0,_  ), // #194 [ref=2x]
  O(000F00,00,3,0,1,0,0,_  ), // #195 [ref=1x]
  O(000F00,00,4,0,1,0,0,_  ), // #196 [ref=2x]
  O(000F00,00,6,0,1,0,0,_  )  // #197 [ref=1x]
};
// ----------------------------------------------------------------------------
// ${MainOpcodeTable:End}

// ${AltOpcodeTable:Begin}
// ------------------- Automatically generated, do not edit -------------------
const uint32_t InstDB::_altOpcodeTable[] = {
  0                         , // #0 [ref=1364x]
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
// [asmjit::x86::InstDB - CommonInfoTableA]
// ============================================================================

// ${InstCommonTable:Begin}
// ------------------- Automatically generated, do not edit -------------------
#define F(VAL) InstDB::kFlag##VAL
#define CONTROL(VAL) Inst::kControl##VAL
#define SINGLE_REG(VAL) InstDB::kSingleReg##VAL
const InstDB::CommonInfo InstDB::_commonInfoTable[] = {
  { 0                                                     , 0  , 0 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #0 [ref=1x]
  { 0                                                     , 339, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #1 [ref=4x]
  { 0                                                     , 340, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #2 [ref=2x]
  { F(Lock)|F(XAcquire)|F(XRelease)                       , 16 , 12, CONTROL(None)   , SINGLE_REG(None), 0 }, // #3 [ref=3x]
  { 0                                                     , 151, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #4 [ref=2x]
  { F(Vec)                                                , 70 , 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #5 [ref=54x]
  { F(Vec)                                                , 97 , 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #6 [ref=19x]
  { F(Vec)                                                , 222, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #7 [ref=16x]
  { F(Vec)                                                , 183, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #8 [ref=20x]
  { F(Lock)|F(XAcquire)|F(XRelease)                       , 28 , 11, CONTROL(None)   , SINGLE_REG(RO)  , 0 }, // #9 [ref=1x]
  { F(Vex)                                                , 237, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #10 [ref=3x]
  { F(Vec)                                                , 70 , 1 , CONTROL(None)   , SINGLE_REG(RO)  , 0 }, // #11 [ref=12x]
  { 0                                                     , 341, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #12 [ref=1x]
  { F(Vex)                                                , 239, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #13 [ref=5x]
  { F(Vex)                                                , 151, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #14 [ref=12x]
  { F(Vec)                                                , 342, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #15 [ref=4x]
  { 0                                                     , 241, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #16 [ref=3x]
  { F(Mib)                                                , 343, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #17 [ref=1x]
  { 0                                                     , 344, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #18 [ref=1x]
  { 0                                                     , 243, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #19 [ref=1x]
  { F(Mib)                                                , 345, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #20 [ref=1x]
  { 0                                                     , 245, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #21 [ref=1x]
  { 0                                                     , 150, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #22 [ref=35x]
  { 0                                                     , 346, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #23 [ref=1x]
  { 0                                                     , 114, 4 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #24 [ref=1x]
  { F(Lock)|F(XAcquire)|F(XRelease)                       , 114, 4 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #25 [ref=3x]
  { 0                                                     , 247, 2 , CONTROL(Call)   , SINGLE_REG(None), 0 }, // #26 [ref=1x]
  { 0                                                     , 347, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #27 [ref=1x]
  { 0                                                     , 348, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #28 [ref=2x]
  { 0                                                     , 322, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #29 [ref=1x]
  { 0                                                     , 257, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #30 [ref=74x]
  { 0                                                     , 349, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #31 [ref=24x]
  { 0                                                     , 350, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #32 [ref=1x]
  { 0                                                     , 16 , 12, CONTROL(None)   , SINGLE_REG(None), 0 }, // #33 [ref=1x]
  { F(Rep)                                                , 351, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #34 [ref=1x]
  { F(Vec)                                                , 352, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #35 [ref=2x]
  { F(Vec)                                                , 353, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #36 [ref=3x]
  { F(Lock)|F(XAcquire)|F(XRelease)                       , 118, 4 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #37 [ref=1x]
  { F(Lock)|F(XAcquire)|F(XRelease)                       , 354, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #38 [ref=1x]
  { F(Lock)|F(XAcquire)|F(XRelease)                       , 355, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #39 [ref=1x]
  { 0                                                     , 356, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #40 [ref=1x]
  { 0                                                     , 357, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #41 [ref=1x]
  { 0                                                     , 249, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #42 [ref=1x]
  { F(Mmx)|F(Vec)                                         , 358, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #43 [ref=2x]
  { F(Mmx)|F(Vec)                                         , 359, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #44 [ref=2x]
  { F(Mmx)|F(Vec)                                         , 360, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #45 [ref=2x]
  { F(Vec)                                                , 361, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #46 [ref=2x]
  { F(Vec)                                                , 362, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #47 [ref=2x]
  { F(Vec)                                                , 363, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #48 [ref=2x]
  { 0                                                     , 364, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #49 [ref=1x]
  { 0                                                     , 365, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #50 [ref=2x]
  { F(Lock)|F(XAcquire)|F(XRelease)                       , 251, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #51 [ref=2x]
  { 0                                                     , 39 , 4 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #52 [ref=3x]
  { F(Mmx)                                                , 257, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #53 [ref=1x]
  { 0                                                     , 253, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #54 [ref=2x]
  { 0                                                     , 366, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #55 [ref=1x]
  { F(Vec)                                                , 367, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #56 [ref=2x]
  { F(Vec)                                                , 255, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #57 [ref=1x]
  { F(FpuM32)|F(FpuM64)                                   , 153, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #58 [ref=6x]
  { 0                                                     , 257, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #59 [ref=9x]
  { F(FpuM80)                                             , 368, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #60 [ref=2x]
  { 0                                                     , 258, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #61 [ref=13x]
  { F(FpuM32)|F(FpuM64)                                   , 259, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #62 [ref=2x]
  { F(FpuM16)|F(FpuM32)                                   , 369, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #63 [ref=9x]
  { F(FpuM16)|F(FpuM32)|F(FpuM64)                         , 370, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #64 [ref=3x]
  { F(FpuM32)|F(FpuM64)|F(FpuM80)                         , 371, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #65 [ref=2x]
  { F(FpuM16)                                             , 372, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #66 [ref=3x]
  { F(FpuM16)                                             , 373, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #67 [ref=2x]
  { F(FpuM32)|F(FpuM64)                                   , 260, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #68 [ref=1x]
  { 0                                                     , 374, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #69 [ref=2x]
  { 0                                                     , 39 , 10, CONTROL(None)   , SINGLE_REG(None), 0 }, // #70 [ref=1x]
  { 0                                                     , 375, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #71 [ref=1x]
  { F(Rep)                                                , 376, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #72 [ref=1x]
  { F(Vec)                                                , 261, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #73 [ref=1x]
  { 0                                                     , 377, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #74 [ref=2x]
  { 0                                                     , 378, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #75 [ref=8x]
  { 0                                                     , 263, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #76 [ref=3x]
  { 0                                                     , 265, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #77 [ref=1x]
  { 0                                                     , 257, 1 , CONTROL(Return) , SINGLE_REG(None), 0 }, // #78 [ref=3x]
  { 0                                                     , 379, 1 , CONTROL(Return) , SINGLE_REG(None), 0 }, // #79 [ref=1x]
  { 0                                                     , 267, 2 , CONTROL(Branch) , SINGLE_REG(None), 0 }, // #80 [ref=30x]
  { 0                                                     , 269, 2 , CONTROL(Branch) , SINGLE_REG(None), 0 }, // #81 [ref=4x]
  { 0                                                     , 271, 2 , CONTROL(Jump)   , SINGLE_REG(None), 0 }, // #82 [ref=1x]
  { F(Vec)|F(Vex)                                         , 380, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #83 [ref=27x]
  { F(Vec)|F(Vex)                                         , 273, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #84 [ref=1x]
  { F(Vec)|F(Vex)                                         , 275, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #85 [ref=1x]
  { F(Vec)|F(Vex)                                         , 277, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #86 [ref=1x]
  { F(Vec)|F(Vex)                                         , 279, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #87 [ref=1x]
  { F(Vec)|F(Vex)                                         , 381, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #88 [ref=12x]
  { F(Vec)|F(Vex)                                         , 382, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #89 [ref=8x]
  { 0                                                     , 383, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #90 [ref=2x]
  { 0                                                     , 281, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #91 [ref=1x]
  { F(Vec)                                                , 192, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #92 [ref=2x]
  { 0                                                     , 384, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #93 [ref=2x]
  { 0                                                     , 283, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #94 [ref=2x]
  { 0                                                     , 385, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #95 [ref=1x]
  { 0                                                     , 156, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #96 [ref=3x]
  { 0                                                     , 386, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #97 [ref=5x]
  { F(Vex)                                                , 346, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #98 [ref=2x]
  { F(Rep)                                                , 387, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #99 [ref=1x]
  { 0                                                     , 285, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #100 [ref=1x]
  { F(Vex)                                                , 388, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #101 [ref=2x]
  { F(Vec)                                                , 389, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #102 [ref=1x]
  { F(Mmx)                                                , 390, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #103 [ref=1x]
  { 0                                                     , 391, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #104 [ref=2x]
  { F(XRelease)                                           , 0  , 16, CONTROL(None)   , SINGLE_REG(None), 0 }, // #105 [ref=1x]
  { F(Vec)                                                , 70 , 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #106 [ref=6x]
  { 0                                                     , 64 , 6 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #107 [ref=1x]
  { F(Mmx)|F(Vec)                                         , 287, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #108 [ref=1x]
  { 0                                                     , 392, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #109 [ref=1x]
  { 0                                                     , 68 , 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #110 [ref=2x]
  { F(Mmx)|F(Vec)                                         , 393, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #111 [ref=1x]
  { F(Vec)                                                , 256, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #112 [ref=2x]
  { F(Vec)                                                , 198, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #113 [ref=4x]
  { F(Vec)                                                , 394, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #114 [ref=2x]
  { F(Vec)                                                , 71 , 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #115 [ref=3x]
  { F(Mmx)                                                , 395, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #116 [ref=1x]
  { F(Vec)                                                , 98 , 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #117 [ref=1x]
  { F(Vec)                                                , 201, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #118 [ref=1x]
  { F(Mmx)|F(Vec)                                         , 94 , 5 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #119 [ref=1x]
  { F(Mmx)|F(Vec)                                         , 396, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #120 [ref=1x]
  { F(Rep)                                                , 397, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #121 [ref=1x]
  { F(Vec)                                                , 97 , 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #122 [ref=1x]
  { F(Vec)                                                , 289, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #123 [ref=1x]
  { 0                                                     , 291, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #124 [ref=2x]
  { 0                                                     , 398, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #125 [ref=1x]
  { F(Vex)                                                , 293, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #126 [ref=1x]
  { 0                                                     , 399, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #127 [ref=1x]
  { 0                                                     , 400, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #128 [ref=1x]
  { F(Lock)|F(XAcquire)|F(XRelease)                       , 252, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #129 [ref=2x]
  { 0                                                     , 295, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #130 [ref=1x]
  { F(Lock)|F(XAcquire)|F(XRelease)                       , 16 , 12, CONTROL(None)   , SINGLE_REG(RO)  , 0 }, // #131 [ref=1x]
  { 0                                                     , 401, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #132 [ref=1x]
  { F(Rep)                                                , 402, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #133 [ref=1x]
  { F(Mmx)|F(Vec)                                         , 297, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #134 [ref=40x]
  { F(Mmx)|F(Vec)                                         , 299, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #135 [ref=1x]
  { F(Mmx)|F(Vec)                                         , 297, 2 , CONTROL(None)   , SINGLE_REG(RO)  , 0 }, // #136 [ref=6x]
  { F(Mmx)|F(Vec)                                         , 297, 2 , CONTROL(None)   , SINGLE_REG(WO)  , 0 }, // #137 [ref=16x]
  { F(Mmx)                                                , 297, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #138 [ref=26x]
  { F(Vec)                                                , 70 , 1 , CONTROL(None)   , SINGLE_REG(WO)  , 0 }, // #139 [ref=4x]
  { F(Vec)                                                , 403, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #140 [ref=1x]
  { F(Vec)                                                , 404, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #141 [ref=1x]
  { F(Vec)                                                , 405, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #142 [ref=1x]
  { F(Vec)                                                , 406, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #143 [ref=1x]
  { F(Vec)                                                , 407, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #144 [ref=1x]
  { F(Vec)                                                , 408, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #145 [ref=1x]
  { F(Mmx)|F(Vec)                                         , 301, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #146 [ref=1x]
  { F(Vec)                                                , 409, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #147 [ref=1x]
  { F(Vec)                                                , 410, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #148 [ref=1x]
  { F(Vec)                                                , 411, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #149 [ref=1x]
  { F(Mmx)|F(Vec)                                         , 412, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #150 [ref=1x]
  { F(Mmx)|F(Vec)                                         , 413, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #151 [ref=1x]
  { F(Vec)                                                , 225, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #152 [ref=2x]
  { 0                                                     , 122, 4 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #153 [ref=1x]
  { 0                                                     , 379, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #154 [ref=6x]
  { F(Mmx)                                                , 299, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #155 [ref=1x]
  { F(Mmx)|F(Vec)                                         , 303, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #156 [ref=8x]
  { F(Vec)                                                , 414, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #157 [ref=2x]
  { 0                                                     , 126, 4 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #158 [ref=1x]
  { 0                                                     , 415, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #159 [ref=8x]
  { 0                                                     , 416, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #160 [ref=4x]
  { 0                                                     , 417, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #161 [ref=6x]
  { 0                                                     , 305, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #162 [ref=1x]
  { 0                                                     , 418, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #163 [ref=2x]
  { F(Rep)|F(RepIgnored)                                  , 307, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #164 [ref=1x]
  { F(Vex)                                                , 309, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #165 [ref=1x]
  { F(Rep)                                                , 419, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #166 [ref=1x]
  { 0                                                     , 420, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #167 [ref=30x]
  { 0                                                     , 159, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #168 [ref=2x]
  { 0                                                     , 421, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #169 [ref=3x]
  { F(Rep)                                                , 422, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #170 [ref=1x]
  { F(Lock)|F(XAcquire)|F(XRelease)                       , 16 , 12, CONTROL(None)   , SINGLE_REG(WO)  , 0 }, // #171 [ref=2x]
  { 0                                                     , 57 , 7 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #172 [ref=1x]
  { F(Vec)|F(Evex)|F(Avx512T4X)|F(Avx512KZ)               , 423, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #173 [ref=4x]
  { F(Vec)|F(Evex)|F(Avx512T4X)|F(Avx512KZ)               , 424, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #174 [ref=2x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_ER_SAE_B64)          , 162, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #175 [ref=22x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_ER_SAE_B32)          , 162, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #176 [ref=22x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_ER_SAE)              , 425, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #177 [ref=18x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_ER_SAE)              , 426, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #178 [ref=17x]
  { F(Vec)|F(Vex)                                         , 162, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #179 [ref=15x]
  { F(Vec)|F(Vex)|F(Evex)                                 , 162, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #180 [ref=5x]
  { F(Vec)|F(Vex)                                         , 70 , 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #181 [ref=17x]
  { F(Vec)|F(Vex)                                         , 183, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #182 [ref=1x]
  { F(Vec)|F(Evex)|F(Avx512KZ_B32)                        , 165, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #183 [ref=4x]
  { F(Vec)|F(Evex)|F(Avx512KZ_B64)                        , 165, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #184 [ref=4x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_B64)                 , 162, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #185 [ref=10x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_B32)                 , 162, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #186 [ref=12x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_B64)                 , 162, 3 , CONTROL(None)   , SINGLE_REG(RO)  , 0 }, // #187 [ref=2x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_B32)                 , 162, 3 , CONTROL(None)   , SINGLE_REG(RO)  , 0 }, // #188 [ref=6x]
  { F(Vec)|F(Evex)|F(Avx512KZ)                            , 162, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #189 [ref=13x]
  { F(Vec)|F(Evex)|F(Avx512KZ_B32)                        , 162, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #190 [ref=16x]
  { F(Vec)|F(Evex)|F(Avx512KZ_B64)                        , 162, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #191 [ref=19x]
  { F(Vec)|F(Vex)                                         , 165, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #192 [ref=6x]
  { F(Vec)|F(Vex)                                         , 311, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #193 [ref=3x]
  { F(Vec)|F(Vex)                                         , 427, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #194 [ref=2x]
  { F(Vec)|F(Evex)|F(Avx512KZ)                            , 428, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #195 [ref=1x]
  { F(Vec)|F(Evex)|F(Avx512KZ)                            , 429, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #196 [ref=4x]
  { F(Vec)|F(Evex)|F(Avx512KZ)                            , 430, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #197 [ref=4x]
  { F(Vec)|F(Evex)|F(Avx512KZ)                            , 431, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #198 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)                     , 428, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #199 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)                     , 432, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #200 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_SAE_B64)             , 168, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #201 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_SAE_B32)             , 168, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #202 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_SAE)                 , 433, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #203 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_SAE)                 , 434, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #204 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512SAE)                    , 97 , 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #205 [ref=2x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512SAE)                    , 222, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #206 [ref=2x]
  { F(Vec)|F(Evex)|F(Avx512KZ)                            , 171, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #207 [ref=6x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_B32)                 , 174, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #208 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_ER_SAE_B32)          , 177, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #209 [ref=3x]
  { F(Vec)|F(Evex)|F(Avx512KZ_B32)                        , 313, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #210 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_ER_SAE_B64)          , 313, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #211 [ref=2x]
  { F(Vec)|F(Evex)|F(Avx512KZ_ER_SAE_B64)                 , 177, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #212 [ref=4x]
  { F(Vec)|F(Evex)|F(Avx512KZ_ER_SAE_B64)                 , 313, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #213 [ref=3x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_SAE)                 , 174, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #214 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_ER_SAE_B32)          , 174, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #215 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_SAE)                 , 180, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #216 [ref=1x]
  { F(Vec)|F(Evex)|F(Avx512KZ_ER_SAE_B32)                 , 174, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #217 [ref=2x]
  { F(Vec)|F(Evex)|F(Avx512KZ_ER_SAE_B32)                 , 177, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #218 [ref=2x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512ER_SAE)                 , 361, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #219 [ref=1x]
  { F(Vec)|F(Evex)|F(Avx512ER_SAE)                        , 361, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #220 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512ER_SAE)                 , 435, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #221 [ref=2x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_SAE)                 , 426, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #222 [ref=3x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512ER_SAE)                 , 363, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #223 [ref=1x]
  { F(Vec)|F(Evex)|F(Avx512ER_SAE)                        , 363, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #224 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_SAE_B64)             , 313, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #225 [ref=1x]
  { F(Vec)|F(Evex)|F(Avx512KZ_SAE_B64)                    , 177, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #226 [ref=3x]
  { F(Vec)|F(Evex)|F(Avx512KZ_SAE_B64)                    , 313, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #227 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_SAE_B32)             , 177, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #228 [ref=1x]
  { F(Vec)|F(Evex)|F(Avx512KZ_SAE_B32)                    , 174, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #229 [ref=2x]
  { F(Vec)|F(Evex)|F(Avx512KZ_SAE_B32)                    , 177, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #230 [ref=2x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512SAE)                    , 361, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #231 [ref=1x]
  { F(Vec)|F(Evex)|F(Avx512SAE)                           , 361, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #232 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512SAE)                    , 363, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #233 [ref=1x]
  { F(Vec)|F(Evex)|F(Avx512SAE)                           , 363, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #234 [ref=1x]
  { F(Vec)|F(Evex)|F(Avx512KZ_B32)                        , 174, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #235 [ref=1x]
  { F(Vec)|F(Evex)|F(Avx512ER_SAE)                        , 435, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #236 [ref=2x]
  { F(Vec)|F(Evex)|F(Avx512KZ)                            , 165, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #237 [ref=3x]
  { F(Vec)|F(Vex)                                         , 165, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #238 [ref=9x]
  { F(Vec)|F(Evex)|F(Avx512KZ_SAE_B64)                    , 74 , 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #239 [ref=3x]
  { F(Vec)|F(Evex)|F(Avx512KZ_SAE_B32)                    , 74 , 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #240 [ref=3x]
  { F(Vec)|F(Evex)|F(Avx512KZ)                            , 177, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #241 [ref=9x]
  { F(Vec)|F(Vex)                                         , 181, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #242 [ref=2x]
  { F(Vec)|F(Evex)|F(Avx512KZ)                            , 436, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #243 [ref=4x]
  { F(Vec)|F(Evex)|F(Avx512KZ)                            , 182, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #244 [ref=4x]
  { F(Vec)|F(Vex)|F(Evex)                                 , 367, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #245 [ref=2x]
  { F(Vec)|F(Evex)|F(Avx512KZ_SAE_B64)                    , 165, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #246 [ref=2x]
  { F(Vec)|F(Evex)|F(Avx512KZ_SAE_B32)                    , 165, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #247 [ref=2x]
  { F(Vec)|F(Evex)|F(Avx512KZ_SAE)                        , 437, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #248 [ref=4x]
  { F(Vec)|F(Evex)|F(Avx512KZ_SAE)                        , 438, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #249 [ref=4x]
  { F(Vec)|F(Vex)                                         , 130, 4 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #250 [ref=13x]
  { F(Vec)|F(Vex)                                         , 315, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #251 [ref=4x]
  { F(Vec)|F(Vex)                                         , 317, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #252 [ref=4x]
  { F(Vec)|F(Evex)|F(Avx512K_B64)                         , 439, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #253 [ref=1x]
  { F(Vec)|F(Evex)|F(Avx512K_B32)                         , 439, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #254 [ref=1x]
  { F(Vec)|F(Evex)|F(Avx512K)                             , 440, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #255 [ref=1x]
  { F(Vec)|F(Evex)|F(Avx512K)                             , 441, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #256 [ref=1x]
  { F(Vec)|F(Vex)                                         , 177, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #257 [ref=7x]
  { F(Vec)|F(Vex)                                         , 97 , 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #258 [ref=1x]
  { F(Vec)|F(Vex)                                         , 222, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #259 [ref=1x]
  { F(Vec)|F(Vsib)|F(Vex)|F(Evex)|F(Avx512K)              , 99 , 5 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #260 [ref=2x]
  { F(Vec)|F(Vsib)|F(Vex)|F(Evex)|F(Avx512K)              , 104, 5 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #261 [ref=2x]
  { F(Vsib)|F(Evex)|F(Avx512K)                            , 442, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #262 [ref=4x]
  { F(Vsib)|F(Evex)|F(Avx512K)                            , 443, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #263 [ref=4x]
  { F(Vsib)|F(Evex)|F(Avx512K)                            , 444, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #264 [ref=8x]
  { F(Vec)|F(Vsib)|F(Vex)|F(Evex)|F(Avx512K)              , 109, 5 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #265 [ref=2x]
  { F(Vec)|F(Vsib)|F(Vex)|F(Evex)|F(Avx512K)              , 134, 4 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #266 [ref=2x]
  { F(Vec)|F(Evex)|F(Avx512KZ_SAE)                        , 425, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #267 [ref=3x]
  { F(Vec)|F(Evex)|F(Avx512KZ_SAE)                        , 426, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #268 [ref=3x]
  { F(Vec)|F(Evex)|F(Avx512KZ_SAE_B64)                    , 183, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #269 [ref=2x]
  { F(Vec)|F(Evex)|F(Avx512KZ_SAE_B32)                    , 183, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #270 [ref=2x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)                     , 165, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #271 [ref=3x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)                     , 162, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #272 [ref=22x]
  { F(Vec)|F(Vex)                                         , 319, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #273 [ref=2x]
  { F(Vec)|F(Evex)|F(Avx512KZ)                            , 319, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #274 [ref=4x]
  { F(Vec)|F(Evex)|F(Avx512KZ)                            , 445, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #275 [ref=4x]
  { F(Vec)|F(Vex)|F(Evex)                                 , 438, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #276 [ref=1x]
  { F(Vec)|F(Vex)                                         , 192, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #277 [ref=1x]
  { F(Vex)                                                , 384, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #278 [ref=2x]
  { F(Vec)|F(Vex)                                         , 389, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #279 [ref=1x]
  { F(Vec)|F(Vex)                                         , 138, 4 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #280 [ref=4x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_SAE_B64)             , 162, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #281 [ref=2x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_SAE_B32)             , 162, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #282 [ref=2x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_SAE)                 , 425, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #283 [ref=2x]
  { 0                                                     , 446, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #284 [ref=4x]
  { 0                                                     , 321, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #285 [ref=3x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)                     , 70 , 6 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #286 [ref=4x]
  { F(Vec)|F(Vex)|F(Evex)                                 , 323, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #287 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)                     , 186, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #288 [ref=1x]
  { F(Vec)|F(Vex)                                         , 70 , 4 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #289 [ref=2x]
  { F(Vec)|F(Evex)|F(Avx512KZ)                            , 70 , 6 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #290 [ref=6x]
  { F(Vec)|F(Vex)|F(Evex)                                 , 200, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #291 [ref=2x]
  { F(Vec)|F(Vex)|F(Evex)                                 , 325, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #292 [ref=4x]
  { F(Vec)|F(Vex)                                         , 447, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #293 [ref=3x]
  { F(Vec)|F(Vex)|F(Evex)                                 , 189, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #294 [ref=3x]
  { F(Vec)|F(Vex)|F(Evex)                                 , 192, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #295 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)                                 , 195, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #296 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)                     , 198, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #297 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)                     , 177, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #298 [ref=5x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)                     , 201, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #299 [ref=1x]
  { 0                                                     , 327, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #300 [ref=1x]
  { 0                                                     , 329, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #301 [ref=1x]
  { F(Vec)|F(Vex)                                         , 162, 2 , CONTROL(None)   , SINGLE_REG(RO)  , 0 }, // #302 [ref=2x]
  { F(Vec)|F(Evex)|F(Avx512KZ_B32)                        , 162, 3 , CONTROL(None)   , SINGLE_REG(RO)  , 0 }, // #303 [ref=2x]
  { F(Vec)|F(Vex)                                         , 162, 2 , CONTROL(None)   , SINGLE_REG(WO)  , 0 }, // #304 [ref=2x]
  { F(Vec)|F(Evex)|F(Avx512KZ_B32)                        , 162, 3 , CONTROL(None)   , SINGLE_REG(WO)  , 0 }, // #305 [ref=2x]
  { F(Vec)|F(Evex)|F(Avx512KZ_B64)                        , 162, 3 , CONTROL(None)   , SINGLE_REG(WO)  , 0 }, // #306 [ref=2x]
  { F(Vec)|F(Evex)|F(Avx512KZ_B64)                        , 162, 3 , CONTROL(None)   , SINGLE_REG(RO)  , 0 }, // #307 [ref=2x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)                     , 448, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #308 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)                     , 449, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #309 [ref=1x]
  { F(Vec)|F(Evex)                                        , 450, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #310 [ref=6x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)                     , 204, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #311 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)                     , 451, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #312 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)                                 , 165, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #313 [ref=1x]
  { F(Vec)|F(Evex)|F(Avx512K)                             , 207, 3 , CONTROL(None)   , SINGLE_REG(WO)  , 0 }, // #314 [ref=2x]
  { F(Vec)|F(Evex)|F(Avx512K_B32)                         , 207, 3 , CONTROL(None)   , SINGLE_REG(WO)  , 0 }, // #315 [ref=2x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512K)                      , 210, 3 , CONTROL(None)   , SINGLE_REG(WO)  , 0 }, // #316 [ref=4x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512K_B32)                  , 210, 3 , CONTROL(None)   , SINGLE_REG(WO)  , 0 }, // #317 [ref=2x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512K_B64)                  , 210, 3 , CONTROL(None)   , SINGLE_REG(WO)  , 0 }, // #318 [ref=2x]
  { F(Vec)|F(Vex)                                         , 403, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #319 [ref=1x]
  { F(Vec)|F(Vex)                                         , 404, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #320 [ref=1x]
  { F(Vec)|F(Vex)                                         , 405, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #321 [ref=1x]
  { F(Vec)|F(Vex)                                         , 406, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #322 [ref=1x]
  { F(Vec)|F(Evex)|F(Avx512K_B64)                         , 207, 3 , CONTROL(None)   , SINGLE_REG(WO)  , 0 }, // #323 [ref=4x]
  { F(Vec)|F(Evex)|F(Avx512KZ_B32)                        , 177, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #324 [ref=6x]
  { F(Vec)|F(Vex)                                         , 166, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #325 [ref=2x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_B32)                 , 163, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #326 [ref=1x]
  { F(Vec)|F(Vex)                                         , 142, 4 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #327 [ref=2x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_B64)                 , 76 , 6 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #328 [ref=2x]
  { F(Vec)|F(Vex)                                         , 146, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #329 [ref=1x]
  { F(Vec)|F(Vex)                                         , 147, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #330 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_B64)                 , 146, 4 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #331 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)                                 , 407, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #332 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)                                 , 408, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #333 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)                                 , 452, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #334 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)                     , 453, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #335 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)                     , 454, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #336 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)                     , 455, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #337 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)                     , 456, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #338 [ref=1x]
  { F(Vec)|F(Evex)|F(Avx512KZ_B64)                        , 177, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #339 [ref=4x]
  { F(Vec)|F(Vex)                                         , 311, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #340 [ref=12x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)                     , 162, 3 , CONTROL(None)   , SINGLE_REG(RO)  , 0 }, // #341 [ref=8x]
  { F(Vec)|F(Evex)                                        , 457, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #342 [ref=4x]
  { F(Vec)|F(Evex)|F(Avx512KZ)                            , 213, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #343 [ref=6x]
  { F(Vec)|F(Evex)|F(Avx512KZ)                            , 216, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #344 [ref=9x]
  { F(Vec)|F(Evex)|F(Avx512KZ)                            , 219, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #345 [ref=3x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)                     , 222, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #346 [ref=4x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)                     , 225, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #347 [ref=2x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)                     , 174, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #348 [ref=6x]
  { F(Vec)|F(Vex)                                         , 130, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #349 [ref=1x]
  { F(Vec)|F(Evex)|F(Avx512KZ_B32)                        , 183, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #350 [ref=3x]
  { F(Vec)|F(Evex)|F(Avx512KZ_B64)                        , 183, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #351 [ref=3x]
  { F(Vec)|F(Vex)                                         , 331, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #352 [ref=4x]
  { F(Vec)|F(Vsib)|F(Evex)|F(Avx512K)                     , 228, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #353 [ref=3x]
  { F(Vec)|F(Vsib)|F(Evex)|F(Avx512K)                     , 333, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #354 [ref=2x]
  { F(Vec)|F(Vsib)|F(Evex)|F(Avx512K)                     , 231, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #355 [ref=2x]
  { F(Vec)|F(Vex)                                         , 335, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #356 [ref=8x]
  { F(Vec)|F(Evex)|F(Avx512K)                             , 234, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #357 [ref=5x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_B32)                 , 183, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #358 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)                     , 183, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #359 [ref=2x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_B32)                 , 82 , 6 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #360 [ref=3x]
  { F(Vec)|F(Vex)|F(Evex)                                 , 183, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #361 [ref=2x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_B64)                 , 82 , 6 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #362 [ref=2x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)                     , 82 , 6 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #363 [ref=3x]
  { F(Vec)|F(Evex)|F(Avx512KZ_B64)                        , 88 , 6 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #364 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)                     , 162, 3 , CONTROL(None)   , SINGLE_REG(WO)  , 0 }, // #365 [ref=6x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_B32)                 , 162, 3 , CONTROL(None)   , SINGLE_REG(WO)  , 0 }, // #366 [ref=2x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_B64)                 , 162, 3 , CONTROL(None)   , SINGLE_REG(WO)  , 0 }, // #367 [ref=2x]
  { F(Vec)|F(Evex)|F(Avx512K_B32)                         , 234, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #368 [ref=2x]
  { F(Vec)|F(Evex)|F(Avx512K_B64)                         , 234, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #369 [ref=2x]
  { F(Vec)|F(Evex)|F(Avx512KZ)                            , 425, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #370 [ref=2x]
  { F(Vec)|F(Evex)|F(Avx512KZ)                            , 426, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #371 [ref=2x]
  { F(Vec)|F(Vex)                                         , 426, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #372 [ref=2x]
  { F(Vec)|F(Evex)|F(Avx512KZ)                            , 437, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #373 [ref=1x]
  { F(Vec)|F(Evex)|F(Avx512KZ)                            , 438, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #374 [ref=1x]
  { F(Vec)|F(Vex)                                         , 183, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #375 [ref=2x]
  { F(Vec)|F(Vex)                                         , 437, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #376 [ref=1x]
  { F(Vec)|F(Vex)                                         , 438, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #377 [ref=1x]
  { F(Vec)|F(Evex)|F(Avx512KZ_ER_SAE_B64)                 , 162, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #378 [ref=1x]
  { F(Vec)|F(Evex)|F(Avx512KZ_ER_SAE_B32)                 , 162, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #379 [ref=1x]
  { F(Vec)|F(Evex)|F(Avx512KZ_ER_SAE)                     , 425, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #380 [ref=1x]
  { F(Vec)|F(Evex)|F(Avx512KZ_ER_SAE)                     , 426, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #381 [ref=1x]
  { F(Vec)|F(Vsib)|F(Evex)|F(Avx512K)                     , 337, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #382 [ref=1x]
  { F(Vec)|F(Evex)|F(Avx512KZ_B32)                        , 166, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #383 [ref=2x]
  { F(Vec)|F(Evex)|F(Avx512KZ_B64)                        , 166, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #384 [ref=2x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_B32)                 , 165, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #385 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_B64)                 , 165, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #386 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_ER_SAE_B64)          , 177, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #387 [ref=1x]
  { F(Vec)|F(Vex)                                         , 257, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #388 [ref=2x]
  { F(Lock)|F(XAcquire)|F(XRelease)                       , 49 , 4 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #389 [ref=1x]
  { 0                                                     , 458, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #390 [ref=1x]
  { F(Lock)                                               , 49 , 8 , CONTROL(None)   , SINGLE_REG(RO)  , 0 }, // #391 [ref=1x]
  { 0                                                     , 459, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #392 [ref=6x]
  { 0                                                     , 460, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }  // #393 [ref=6x]
};
#undef SINGLE_REG
#undef CONTROL
#undef F
// ----------------------------------------------------------------------------
// ${InstCommonTable:End}

// ============================================================================
// [asmjit::x86::InstDB - CommonInfoTableB]
// ============================================================================

// ${InstCommonInfoTableB:Begin}
// ------------------- Automatically generated, do not edit -------------------
#define EXT(VAL) uint32_t(Features::k##VAL)
const InstDB::CommonInfoTableB InstDB::_commonInfoTableB[] = {
  { { 0 }, 0, 0 }, // #0 [ref=147x]
  { { 0 }, 1, 0 }, // #1 [ref=31x]
  { { 0 }, 2, 0 }, // #2 [ref=2x]
  { { EXT(ADX) }, 3, 0 }, // #3 [ref=1x]
  { { EXT(SSE2) }, 0, 0 }, // #4 [ref=65x]
  { { EXT(SSE) }, 0, 0 }, // #5 [ref=44x]
  { { EXT(SSE3) }, 0, 0 }, // #6 [ref=12x]
  { { EXT(ADX) }, 4, 0 }, // #7 [ref=1x]
  { { EXT(AESNI) }, 0, 0 }, // #8 [ref=6x]
  { { EXT(BMI) }, 1, 0 }, // #9 [ref=6x]
  { { 0 }, 5, 0 }, // #10 [ref=5x]
  { { EXT(TBM) }, 0, 0 }, // #11 [ref=9x]
  { { EXT(SSE4_1) }, 0, 0 }, // #12 [ref=47x]
  { { EXT(MPX) }, 0, 0 }, // #13 [ref=7x]
  { { 0 }, 6, 0 }, // #14 [ref=1x]
  { { 0 }, 7, 0 }, // #15 [ref=3x]
  { { EXT(BMI2) }, 1, 0 }, // #16 [ref=1x]
  { { EXT(SMAP) }, 8, 0 }, // #17 [ref=2x]
  { { 0 }, 9, 0 }, // #18 [ref=2x]
  { { 0 }, 10, 0 }, // #19 [ref=2x]
  { { EXT(CLDEMOTE) }, 0, 0 }, // #20 [ref=1x]
  { { EXT(CLFLUSH) }, 0, 0 }, // #21 [ref=1x]
  { { EXT(CLFLUSHOPT) }, 0, 0 }, // #22 [ref=1x]
  { { EXT(SVM) }, 0, 0 }, // #23 [ref=6x]
  { { 0 }, 11, 0 }, // #24 [ref=2x]
  { { EXT(CLWB) }, 0, 0 }, // #25 [ref=1x]
  { { EXT(CLZERO) }, 0, 0 }, // #26 [ref=1x]
  { { 0 }, 3, 0 }, // #27 [ref=1x]
  { { EXT(CMOV) }, 12, 0 }, // #28 [ref=6x]
  { { EXT(CMOV) }, 13, 0 }, // #29 [ref=8x]
  { { EXT(CMOV) }, 14, 0 }, // #30 [ref=6x]
  { { EXT(CMOV) }, 15, 0 }, // #31 [ref=4x]
  { { EXT(CMOV) }, 16, 0 }, // #32 [ref=4x]
  { { EXT(CMOV) }, 17, 0 }, // #33 [ref=2x]
  { { EXT(CMOV) }, 18, 0 }, // #34 [ref=6x]
  { { EXT(CMOV) }, 19, 0 }, // #35 [ref=2x]
  { { 0 }, 20, 0 }, // #36 [ref=2x]
  { { EXT(I486) }, 1, 0 }, // #37 [ref=1x]
  { { EXT(CMPXCHG16B) }, 5, 0 }, // #38 [ref=1x]
  { { EXT(CMPXCHG8B) }, 0, 0 }, // #39 [ref=1x]
  { { EXT(SSE2) }, 1, 0 }, // #40 [ref=2x]
  { { EXT(SSE) }, 1, 0 }, // #41 [ref=2x]
  { { EXT(I486) }, 0, 0 }, // #42 [ref=4x]
  { { EXT(SSE4_2) }, 0, 0 }, // #43 [ref=2x]
  { { 0 }, 21, 0 }, // #44 [ref=2x]
  { { EXT(MMX) }, 0, 0 }, // #45 [ref=1x]
  { { EXT(ENQCMD) }, 0, 0 }, // #46 [ref=2x]
  { { EXT(SSE4A) }, 0, 0 }, // #47 [ref=4x]
  { { 0 }, 22, 0 }, // #48 [ref=4x]
  { { EXT(3DNOW) }, 0, 0 }, // #49 [ref=21x]
  { { EXT(FXSR) }, 0, 0 }, // #50 [ref=4x]
  { { EXT(SMX) }, 0, 0 }, // #51 [ref=1x]
  { { EXT(GFNI) }, 0, 0 }, // #52 [ref=3x]
  { { 0 }, 17, 0 }, // #53 [ref=5x]
  { { EXT(VMX) }, 0, 0 }, // #54 [ref=12x]
  { { 0 }, 12, 0 }, // #55 [ref=8x]
  { { 0 }, 13, 0 }, // #56 [ref=12x]
  { { 0 }, 14, 0 }, // #57 [ref=10x]
  { { 0 }, 15, 0 }, // #58 [ref=8x]
  { { 0 }, 16, 0 }, // #59 [ref=8x]
  { { 0 }, 18, 0 }, // #60 [ref=8x]
  { { 0 }, 19, 0 }, // #61 [ref=4x]
  { { EXT(AVX512_DQ) }, 0, 0 }, // #62 [ref=23x]
  { { EXT(AVX512_BW) }, 0, 0 }, // #63 [ref=22x]
  { { EXT(AVX512_F) }, 0, 0 }, // #64 [ref=37x]
  { { EXT(AVX512_DQ) }, 1, 0 }, // #65 [ref=3x]
  { { EXT(AVX512_BW) }, 1, 0 }, // #66 [ref=4x]
  { { EXT(AVX512_F) }, 1, 0 }, // #67 [ref=1x]
  { { EXT(LAHFSAHF) }, 23, 0 }, // #68 [ref=1x]
  { { EXT(LWP) }, 0, 0 }, // #69 [ref=4x]
  { { EXT(LZCNT) }, 1, 0 }, // #70 [ref=1x]
  { { EXT(MMX2) }, 0, 0 }, // #71 [ref=8x]
  { { EXT(MONITOR) }, 0, 0 }, // #72 [ref=2x]
  { { EXT(MONITORX) }, 0, 0 }, // #73 [ref=2x]
  { { EXT(MOVBE) }, 0, 0 }, // #74 [ref=1x]
  { { EXT(MMX), EXT(SSE2) }, 0, 0 }, // #75 [ref=46x]
  { { EXT(MOVDIR64B) }, 0, 0 }, // #76 [ref=1x]
  { { EXT(MOVDIRI) }, 0, 0 }, // #77 [ref=1x]
  { { EXT(BMI2) }, 0, 0 }, // #78 [ref=7x]
  { { 0 }, 24, 0 }, // #79 [ref=1x]
  { { EXT(SSSE3) }, 0, 0 }, // #80 [ref=15x]
  { { EXT(MMX2), EXT(SSE2) }, 0, 0 }, // #81 [ref=10x]
  { { EXT(PCLMULQDQ) }, 0, 0 }, // #82 [ref=1x]
  { { EXT(SSE4_2) }, 1, 0 }, // #83 [ref=4x]
  { { EXT(PCOMMIT) }, 0, 0 }, // #84 [ref=1x]
  { { EXT(MMX2), EXT(SSE2), EXT(SSE4_1) }, 0, 0 }, // #85 [ref=1x]
  { { EXT(3DNOW2) }, 0, 0 }, // #86 [ref=5x]
  { { EXT(GEODE) }, 0, 0 }, // #87 [ref=2x]
  { { EXT(POPCNT) }, 1, 0 }, // #88 [ref=1x]
  { { 0 }, 25, 0 }, // #89 [ref=3x]
  { { EXT(PREFETCHW) }, 1, 0 }, // #90 [ref=1x]
  { { EXT(PREFETCHWT1) }, 1, 0 }, // #91 [ref=1x]
  { { EXT(SSE4_1) }, 1, 0 }, // #92 [ref=1x]
  { { 0 }, 26, 0 }, // #93 [ref=3x]
  { { 0 }, 27, 0 }, // #94 [ref=2x]
  { { EXT(FSGSBASE) }, 0, 0 }, // #95 [ref=4x]
  { { EXT(MSR) }, 0, 0 }, // #96 [ref=2x]
  { { EXT(RDPID) }, 0, 0 }, // #97 [ref=1x]
  { { EXT(RDRAND) }, 1, 0 }, // #98 [ref=1x]
  { { EXT(RDSEED) }, 1, 0 }, // #99 [ref=1x]
  { { EXT(RDTSC) }, 0, 0 }, // #100 [ref=1x]
  { { EXT(RDTSCP) }, 0, 0 }, // #101 [ref=1x]
  { { 0 }, 28, 0 }, // #102 [ref=2x]
  { { EXT(LAHFSAHF) }, 29, 0 }, // #103 [ref=1x]
  { { EXT(SHA) }, 0, 0 }, // #104 [ref=7x]
  { { EXT(SKINIT) }, 0, 0 }, // #105 [ref=2x]
  { { EXT(AVX512_4FMAPS) }, 0, 0 }, // #106 [ref=4x]
  { { EXT(AVX), EXT(AVX512_F), EXT(AVX512_VL) }, 0, 0 }, // #107 [ref=46x]
  { { EXT(AVX), EXT(AVX512_F) }, 0, 0 }, // #108 [ref=32x]
  { { EXT(AVX) }, 0, 0 }, // #109 [ref=37x]
  { { EXT(AESNI), EXT(AVX), EXT(AVX512_F), EXT(AVX512_VL), EXT(VAES) }, 0, 0 }, // #110 [ref=4x]
  { { EXT(AESNI), EXT(AVX) }, 0, 0 }, // #111 [ref=2x]
  { { EXT(AVX512_F), EXT(AVX512_VL) }, 0, 0 }, // #112 [ref=112x]
  { { EXT(AVX), EXT(AVX512_DQ), EXT(AVX512_VL) }, 0, 0 }, // #113 [ref=8x]
  { { EXT(AVX512_BW), EXT(AVX512_VL) }, 0, 0 }, // #114 [ref=26x]
  { { EXT(AVX512_DQ), EXT(AVX512_VL) }, 0, 0 }, // #115 [ref=30x]
  { { EXT(AVX2) }, 0, 0 }, // #116 [ref=9x]
  { { EXT(AVX), EXT(AVX2), EXT(AVX512_F), EXT(AVX512_VL) }, 0, 0 }, // #117 [ref=39x]
  { { EXT(AVX), EXT(AVX512_F) }, 1, 0 }, // #118 [ref=4x]
  { { EXT(AVX512_BF16), EXT(AVX512_VL) }, 0, 0 }, // #119 [ref=3x]
  { { EXT(AVX512_F), EXT(AVX512_VL), EXT(F16C) }, 0, 0 }, // #120 [ref=2x]
  { { EXT(AVX512_ERI) }, 0, 0 }, // #121 [ref=10x]
  { { EXT(AVX512_F), EXT(AVX512_VL), EXT(FMA) }, 0, 0 }, // #122 [ref=36x]
  { { EXT(AVX512_F), EXT(FMA) }, 0, 0 }, // #123 [ref=24x]
  { { EXT(FMA4) }, 0, 0 }, // #124 [ref=20x]
  { { EXT(XOP) }, 0, 0 }, // #125 [ref=55x]
  { { EXT(AVX2), EXT(AVX512_F), EXT(AVX512_VL) }, 0, 0 }, // #126 [ref=17x]
  { { EXT(AVX512_PFI) }, 0, 0 }, // #127 [ref=16x]
  { { EXT(AVX), EXT(AVX512_F), EXT(AVX512_VL), EXT(GFNI) }, 0, 0 }, // #128 [ref=3x]
  { { EXT(AVX), EXT(AVX2) }, 0, 0 }, // #129 [ref=17x]
  { { EXT(AVX512_4VNNIW) }, 0, 0 }, // #130 [ref=2x]
  { { EXT(AVX), EXT(AVX2), EXT(AVX512_BW), EXT(AVX512_VL) }, 0, 0 }, // #131 [ref=54x]
  { { EXT(AVX2), EXT(AVX512_BW), EXT(AVX512_VL) }, 0, 0 }, // #132 [ref=2x]
  { { EXT(AVX512_CDI), EXT(AVX512_VL) }, 0, 0 }, // #133 [ref=6x]
  { { EXT(AVX), EXT(AVX512_F), EXT(AVX512_VL), EXT(PCLMULQDQ), EXT(VPCLMULQDQ) }, 0, 0 }, // #134 [ref=1x]
  { { EXT(AVX) }, 1, 0 }, // #135 [ref=7x]
  { { EXT(AVX512_VBMI2), EXT(AVX512_VL) }, 0, 0 }, // #136 [ref=16x]
  { { EXT(AVX512_VL), EXT(AVX512_VNNI) }, 0, 0 }, // #137 [ref=4x]
  { { EXT(AVX512_VBMI), EXT(AVX512_VL) }, 0, 0 }, // #138 [ref=4x]
  { { EXT(AVX), EXT(AVX512_BW) }, 0, 0 }, // #139 [ref=4x]
  { { EXT(AVX), EXT(AVX512_DQ) }, 0, 0 }, // #140 [ref=4x]
  { { EXT(AVX512_IFMA), EXT(AVX512_VL) }, 0, 0 }, // #141 [ref=2x]
  { { EXT(AVX512_BITALG), EXT(AVX512_VL) }, 0, 0 }, // #142 [ref=3x]
  { { EXT(AVX512_VL), EXT(AVX512_VPOPCNTDQ) }, 0, 0 }, // #143 [ref=2x]
  { { EXT(WBNOINVD) }, 0, 0 }, // #144 [ref=1x]
  { { EXT(RTM) }, 0, 0 }, // #145 [ref=3x]
  { { EXT(I486) }, 29, 0 }, // #146 [ref=1x]
  { { EXT(XSAVE) }, 0, 0 }, // #147 [ref=6x]
  { { EXT(XSAVES) }, 0, 0 }, // #148 [ref=4x]
  { { EXT(XSAVEC) }, 0, 0 }, // #149 [ref=2x]
  { { EXT(XSAVEOPT) }, 0, 0 }, // #150 [ref=2x]
  { { EXT(TSX) }, 1, 0 }  // #151 [ref=1x]
};
#undef EXT

#define FLAG(VAL) uint32_t(Status::k##VAL)
const InstDB::RWFlagsInfoTable InstDB::_rwFlagsInfoTable[] = {
  { 0, 0 }, // #0 [ref=1285x]
  { 0, FLAG(AF) | FLAG(CF) | FLAG(OF) | FLAG(PF) | FLAG(SF) | FLAG(ZF) }, // #1 [ref=74x]
  { FLAG(CF), FLAG(AF) | FLAG(CF) | FLAG(OF) | FLAG(PF) | FLAG(SF) | FLAG(ZF) }, // #2 [ref=2x]
  { FLAG(CF), FLAG(CF) }, // #3 [ref=2x]
  { FLAG(OF), FLAG(OF) }, // #4 [ref=1x]
  { 0, FLAG(ZF) }, // #5 [ref=6x]
  { 0, FLAG(AF) | FLAG(CF) | FLAG(OF) | FLAG(PF) | FLAG(SF) }, // #6 [ref=1x]
  { 0, FLAG(AF) | FLAG(CF) | FLAG(PF) }, // #7 [ref=3x]
  { 0, FLAG(AC) }, // #8 [ref=2x]
  { 0, FLAG(CF) }, // #9 [ref=2x]
  { 0, FLAG(DF) }, // #10 [ref=2x]
  { 0, FLAG(IF) }, // #11 [ref=2x]
  { FLAG(CF) | FLAG(ZF), 0 }, // #12 [ref=14x]
  { FLAG(CF), 0 }, // #13 [ref=20x]
  { FLAG(ZF), 0 }, // #14 [ref=16x]
  { FLAG(OF) | FLAG(SF) | FLAG(ZF), 0 }, // #15 [ref=12x]
  { FLAG(OF) | FLAG(SF), 0 }, // #16 [ref=12x]
  { FLAG(OF), 0 }, // #17 [ref=7x]
  { FLAG(PF), 0 }, // #18 [ref=14x]
  { FLAG(SF), 0 }, // #19 [ref=6x]
  { FLAG(DF), FLAG(AF) | FLAG(CF) | FLAG(PF) | FLAG(SF) | FLAG(ZF) }, // #20 [ref=2x]
  { 0, FLAG(AF) | FLAG(OF) | FLAG(PF) | FLAG(SF) | FLAG(ZF) }, // #21 [ref=2x]
  { 0, FLAG(CF) | FLAG(PF) | FLAG(ZF) }, // #22 [ref=4x]
  { FLAG(AF) | FLAG(CF) | FLAG(PF) | FLAG(SF) | FLAG(ZF), 0 }, // #23 [ref=1x]
  { 0, FLAG(AF) | FLAG(CF) | FLAG(PF) | FLAG(ZF) }, // #24 [ref=1x]
  { 0, FLAG(AF) | FLAG(CF) | FLAG(DF) | FLAG(IF) | FLAG(OF) | FLAG(PF) | FLAG(SF) | FLAG(ZF) }, // #25 [ref=3x]
  { FLAG(AF) | FLAG(CF) | FLAG(DF) | FLAG(IF) | FLAG(OF) | FLAG(PF) | FLAG(SF) | FLAG(ZF), 0 }, // #26 [ref=3x]
  { FLAG(CF) | FLAG(OF), FLAG(CF) | FLAG(OF) }, // #27 [ref=2x]
  { 0, FLAG(CF) | FLAG(OF) }, // #28 [ref=2x]
  { 0, FLAG(AF) | FLAG(CF) | FLAG(PF) | FLAG(SF) | FLAG(ZF) }  // #29 [ref=2x]
};
#undef FLAG
// ----------------------------------------------------------------------------
// ${InstCommonInfoTableB:End}

// ============================================================================
// [asmjit::Inst - NameData]
// ============================================================================

#ifndef ASMJIT_NO_TEXT
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
  "cwde\0" "daa\0" "das\0" "enqcmd\0" "enqcmds\0" "f2xm1\0" "fabs\0" "faddp\0" "fbld\0" "fbstp\0" "fchs\0" "fclex\0"
  "fcmovb\0" "fcmovbe\0" "fcmove\0" "fcmovnb\0" "fcmovnbe\0" "fcmovne\0" "fcmovnu\0" "fcmovu\0" "fcom\0" "fcomi\0"
  "fcomip\0" "fcomp\0" "fcompp\0" "fcos\0" "fdecstp\0" "fdiv\0" "fdivp\0" "fdivr\0" "fdivrp\0" "femms\0" "ffree\0"
  "fiadd\0" "ficom\0" "ficomp\0" "fidiv\0" "fidivr\0" "fild\0" "fimul\0" "fincstp\0" "finit\0" "fist\0" "fistp\0"
  "fisttp\0" "fisub\0" "fisubr\0" "fld\0" "fld1\0" "fldcw\0" "fldenv\0" "fldl2e\0" "fldl2t\0" "fldlg2\0" "fldln2\0"
  "fldpi\0" "fldz\0" "fmulp\0" "fnclex\0" "fninit\0" "fnop\0" "fnsave\0" "fnstcw\0" "fnstenv\0" "fnstsw\0" "fpatan\0"
  "fprem\0" "fprem1\0" "fptan\0" "frndint\0" "frstor\0" "fsave\0" "fscale\0" "fsin\0" "fsincos\0" "fsqrt\0" "fst\0"
  "fstcw\0" "fstenv\0" "fstp\0" "fstsw\0" "fsubp\0" "fsubrp\0" "ftst\0" "fucom\0" "fucomi\0" "fucomip\0" "fucomp\0"
  "fucompp\0" "fwait\0" "fxam\0" "fxch\0" "fxrstor\0" "fxrstor64\0" "fxsave\0" "fxsave64\0" "fxtract\0" "fyl2x\0"
  "fyl2xp1\0" "getsec\0" "hlt\0" "inc\0" "insertq\0" "int3\0" "into\0" "invept\0" "invlpg\0" "invlpga\0" "invpcid\0"
  "invvpid\0" "iret\0" "iretd\0" "iretq\0" "iretw\0" "ja\0" "jae\0" "jb\0" "jbe\0" "jc\0" "je\0" "jecxz\0" "jg\0"
  "jge\0" "jl\0" "jle\0" "jmp\0" "jna\0" "jnae\0" "jnb\0" "jnbe\0" "jnc\0" "jne\0" "jng\0" "jnge\0" "jnl\0" "jnle\0"
  "jno\0" "jnp\0" "jns\0" "jnz\0" "jo\0" "jp\0" "jpe\0" "jpo\0" "js\0" "jz\0" "kaddb\0" "kaddd\0" "kaddq\0" "kaddw\0"
  "kandb\0" "kandd\0" "kandnb\0" "kandnd\0" "kandnq\0" "kandnw\0" "kandq\0" "kandw\0" "kmovb\0" "kmovw\0" "knotb\0"
  "knotd\0" "knotq\0" "knotw\0" "korb\0" "kord\0" "korq\0" "kortestb\0" "kortestd\0" "kortestq\0" "kortestw\0" "korw\0"
  "kshiftlb\0" "kshiftld\0" "kshiftlq\0" "kshiftlw\0" "kshiftrb\0" "kshiftrd\0" "kshiftrq\0" "kshiftrw\0" "ktestb\0"
  "ktestd\0" "ktestq\0" "ktestw\0" "kunpckbw\0" "kunpckdq\0" "kunpckwd\0" "kxnorb\0" "kxnord\0" "kxnorq\0" "kxnorw\0"
  "kxorb\0" "kxord\0" "kxorq\0" "kxorw\0" "lahf\0" "lar\0" "lds\0" "lea\0" "leave\0" "les\0" "lfence\0" "lfs\0"
  "lgdt\0" "lgs\0" "lidt\0" "lldt\0" "llwpcb\0" "lmsw\0" "lods\0" "loop\0" "loope\0" "loopne\0" "lsl\0" "ltr\0"
  "lwpins\0" "lwpval\0" "lzcnt\0" "mfence\0" "monitor\0" "monitorx\0" "movdir64b\0" "movdiri\0" "movdq2q\0" "movnti\0"
  "movntq\0" "movntsd\0" "movntss\0" "movq2dq\0" "movsx\0" "movsxd\0" "movzx\0" "mulx\0" "mwait\0" "mwaitx\0" "neg\0"
  "not\0" "out\0" "outs\0" "pause\0" "pavgusb\0" "pcommit\0" "pdep\0" "pext\0" "pf2id\0" "pf2iw\0" "pfacc\0" "pfadd\0"
  "pfcmpeq\0" "pfcmpge\0" "pfcmpgt\0" "pfmax\0" "pfmin\0" "pfmul\0" "pfnacc\0" "pfpnacc\0" "pfrcp\0" "pfrcpit1\0"
  "pfrcpit2\0" "pfrcpv\0" "pfrsqit1\0" "pfrsqrt\0" "pfrsqrtv\0" "pfsub\0" "pfsubr\0" "pi2fd\0" "pi2fw\0" "pmulhrw\0"
  "pop\0" "popa\0" "popad\0" "popcnt\0" "popf\0" "popfd\0" "popfq\0" "prefetch\0" "prefetchnta\0" "prefetcht0\0"
  "prefetcht1\0" "prefetcht2\0" "prefetchw\0" "prefetchwt1\0" "pshufw\0" "pswapd\0" "push\0" "pusha\0" "pushad\0"
  "pushf\0" "pushfd\0" "pushfq\0" "rcl\0" "rcr\0" "rdfsbase\0" "rdgsbase\0" "rdmsr\0" "rdpid\0" "rdpmc\0" "rdrand\0"
  "rdseed\0" "rdtsc\0" "rdtscp\0" "rol\0" "ror\0" "rorx\0" "rsm\0" "sahf\0" "sal\0" "sar\0" "sarx\0" "sbb\0" "scas\0"
  "seta\0" "setae\0" "setb\0" "setbe\0" "setc\0" "sete\0" "setg\0" "setge\0" "setl\0" "setle\0" "setna\0" "setnae\0"
  "setnb\0" "setnbe\0" "setnc\0" "setne\0" "setng\0" "setnge\0" "setnl\0" "setnle\0" "setno\0" "setnp\0" "setns\0"
  "setnz\0" "seto\0" "setp\0" "setpe\0" "setpo\0" "sets\0" "setz\0" "sfence\0" "sgdt\0" "sha1msg1\0" "sha1msg2\0"
  "sha1nexte\0" "sha1rnds4\0" "sha256msg1\0" "sha256msg2\0" "sha256rnds2\0" "shl\0" "shlx\0" "shr\0" "shrd\0" "shrx\0"
  "sidt\0" "skinit\0" "sldt\0" "slwpcb\0" "smsw\0" "stac\0" "stc\0" "stgi\0" "sti\0" "stos\0" "str\0" "swapgs\0"
  "syscall\0" "sysenter\0" "sysexit\0" "sysexit64\0" "sysret\0" "sysret64\0" "t1mskc\0" "tzcnt\0" "tzmsk\0" "ud2\0"
  "v4fmaddps\0" "v4fmaddss\0" "v4fnmaddps\0" "v4fnmaddss\0" "vaddpd\0" "vaddps\0" "vaddsd\0" "vaddss\0" "vaddsubpd\0"
  "vaddsubps\0" "vaesdec\0" "vaesdeclast\0" "vaesenc\0" "vaesenclast\0" "vaesimc\0" "vaeskeygenassist\0" "valignd\0"
  "valignq\0" "vandnpd\0" "vandnps\0" "vandpd\0" "vandps\0" "vblendmb\0" "vblendmd\0" "vblendmpd\0" "vblendmps\0"
  "vblendmq\0" "vblendmw\0" "vblendpd\0" "vblendps\0" "vblendvpd\0" "vblendvps\0" "vbroadcastf128\0"
  "vbroadcastf32x2\0" "vbroadcastf32x4\0" "vbroadcastf32x8\0" "vbroadcastf64x2\0" "vbroadcastf64x4\0"
  "vbroadcasti128\0" "vbroadcasti32x2\0" "vbroadcasti32x4\0" "vbroadcasti32x8\0" "vbroadcasti64x2\0"
  "vbroadcasti64x4\0" "vbroadcastsd\0" "vbroadcastss\0" "vcmppd\0" "vcmpps\0" "vcmpsd\0" "vcmpss\0" "vcomisd\0"
  "vcomiss\0" "vcompresspd\0" "vcompressps\0" "vcvtdq2pd\0" "vcvtdq2ps\0" "vcvtne2ps2bf16\0" "vcvtneps2bf16\0"
  "vcvtpd2dq\0" "vcvtpd2ps\0" "vcvtpd2qq\0" "vcvtpd2udq\0" "vcvtpd2uqq\0" "vcvtph2ps\0" "vcvtps2dq\0" "vcvtps2pd\0"
  "vcvtps2ph\0" "vcvtps2qq\0" "vcvtps2udq\0" "vcvtps2uqq\0" "vcvtqq2pd\0" "vcvtqq2ps\0" "vcvtsd2si\0" "vcvtsd2ss\0"
  "vcvtsd2usi\0" "vcvtsi2sd\0" "vcvtsi2ss\0" "vcvtss2sd\0" "vcvtss2si\0" "vcvtss2usi\0" "vcvttpd2dq\0" "vcvttpd2qq\0"
  "vcvttpd2udq\0" "vcvttpd2uqq\0" "vcvttps2dq\0" "vcvttps2qq\0" "vcvttps2udq\0" "vcvttps2uqq\0" "vcvttsd2si\0"
  "vcvttsd2usi\0" "vcvttss2si\0" "vcvttss2usi\0" "vcvtudq2pd\0" "vcvtudq2ps\0" "vcvtuqq2pd\0" "vcvtuqq2ps\0"
  "vcvtusi2sd\0" "vcvtusi2ss\0" "vdbpsadbw\0" "vdivpd\0" "vdivps\0" "vdivsd\0" "vdivss\0" "vdpbf16ps\0" "vdppd\0"
  "vdpps\0" "verr\0" "verw\0" "vexp2pd\0" "vexp2ps\0" "vexpandpd\0" "vexpandps\0" "vextractf128\0" "vextractf32x4\0"
  "vextractf32x8\0" "vextractf64x2\0" "vextractf64x4\0" "vextracti128\0" "vextracti32x4\0" "vextracti32x8\0"
  "vextracti64x2\0" "vextracti64x4\0" "vextractps\0" "vfixupimmpd\0" "vfixupimmps\0" "vfixupimmsd\0" "vfixupimmss\0"
  "vfmadd132pd\0" "vfmadd132ps\0" "vfmadd132sd\0" "vfmadd132ss\0" "vfmadd213pd\0" "vfmadd213ps\0" "vfmadd213sd\0"
  "vfmadd213ss\0" "vfmadd231pd\0" "vfmadd231ps\0" "vfmadd231sd\0" "vfmadd231ss\0" "vfmaddpd\0" "vfmaddps\0"
  "vfmaddsd\0" "vfmaddss\0" "vfmaddsub132pd\0" "vfmaddsub132ps\0" "vfmaddsub213pd\0" "vfmaddsub213ps\0"
  "vfmaddsub231pd\0" "vfmaddsub231ps\0" "vfmaddsubpd\0" "vfmaddsubps\0" "vfmsub132pd\0" "vfmsub132ps\0" "vfmsub132sd\0"
  "vfmsub132ss\0" "vfmsub213pd\0" "vfmsub213ps\0" "vfmsub213sd\0" "vfmsub213ss\0" "vfmsub231pd\0" "vfmsub231ps\0"
  "vfmsub231sd\0" "vfmsub231ss\0" "vfmsubadd132pd\0" "vfmsubadd132ps\0" "vfmsubadd213pd\0" "vfmsubadd213ps\0"
  "vfmsubadd231pd\0" "vfmsubadd231ps\0" "vfmsubaddpd\0" "vfmsubaddps\0" "vfmsubpd\0" "vfmsubps\0" "vfmsubsd\0"
  "vfmsubss\0" "vfnmadd132pd\0" "vfnmadd132ps\0" "vfnmadd132sd\0" "vfnmadd132ss\0" "vfnmadd213pd\0" "vfnmadd213ps\0"
  "vfnmadd213sd\0" "vfnmadd213ss\0" "vfnmadd231pd\0" "vfnmadd231ps\0" "vfnmadd231sd\0" "vfnmadd231ss\0" "vfnmaddpd\0"
  "vfnmaddps\0" "vfnmaddsd\0" "vfnmaddss\0" "vfnmsub132pd\0" "vfnmsub132ps\0" "vfnmsub132sd\0" "vfnmsub132ss\0"
  "vfnmsub213pd\0" "vfnmsub213ps\0" "vfnmsub213sd\0" "vfnmsub213ss\0" "vfnmsub231pd\0" "vfnmsub231ps\0"
  "vfnmsub231sd\0" "vfnmsub231ss\0" "vfnmsubpd\0" "vfnmsubps\0" "vfnmsubsd\0" "vfnmsubss\0" "vfpclasspd\0"
  "vfpclassps\0" "vfpclasssd\0" "vfpclassss\0" "vfrczpd\0" "vfrczps\0" "vfrczsd\0" "vfrczss\0" "vgatherdpd\0"
  "vgatherdps\0" "vgatherpf0dpd\0" "vgatherpf0dps\0" "vgatherpf0qpd\0" "vgatherpf0qps\0" "vgatherpf1dpd\0"
  "vgatherpf1dps\0" "vgatherpf1qpd\0" "vgatherpf1qps\0" "vgatherqpd\0" "vgatherqps\0" "vgetexppd\0" "vgetexpps\0"
  "vgetexpsd\0" "vgetexpss\0" "vgetmantpd\0" "vgetmantps\0" "vgetmantsd\0" "vgetmantss\0" "vgf2p8affineinvqb\0"
  "vgf2p8affineqb\0" "vgf2p8mulb\0" "vhaddpd\0" "vhaddps\0" "vhsubpd\0" "vhsubps\0" "vinsertf128\0" "vinsertf32x4\0"
  "vinsertf32x8\0" "vinsertf64x2\0" "vinsertf64x4\0" "vinserti128\0" "vinserti32x4\0" "vinserti32x8\0" "vinserti64x2\0"
  "vinserti64x4\0" "vinsertps\0" "vlddqu\0" "vldmxcsr\0" "vmaskmovdqu\0" "vmaskmovpd\0" "vmaskmovps\0" "vmaxpd\0"
  "vmaxps\0" "vmaxsd\0" "vmaxss\0" "vmcall\0" "vmclear\0" "vmfunc\0" "vminpd\0" "vminps\0" "vminsd\0" "vminss\0"
  "vmlaunch\0" "vmload\0" "vmmcall\0" "vmovapd\0" "vmovaps\0" "vmovd\0" "vmovddup\0" "vmovdqa\0" "vmovdqa32\0"
  "vmovdqa64\0" "vmovdqu\0" "vmovdqu16\0" "vmovdqu32\0" "vmovdqu64\0" "vmovdqu8\0" "vmovhlps\0" "vmovhpd\0" "vmovhps\0"
  "vmovlhps\0" "vmovlpd\0" "vmovlps\0" "vmovmskpd\0" "vmovmskps\0" "vmovntdq\0" "vmovntdqa\0" "vmovntpd\0" "vmovntps\0"
  "vmovq\0" "vmovsd\0" "vmovshdup\0" "vmovsldup\0" "vmovss\0" "vmovupd\0" "vmovups\0" "vmpsadbw\0" "vmptrld\0"
  "vmptrst\0" "vmread\0" "vmresume\0" "vmrun\0" "vmsave\0" "vmulpd\0" "vmulps\0" "vmulsd\0" "vmulss\0" "vmwrite\0"
  "vmxon\0" "vorpd\0" "vorps\0" "vp4dpwssd\0" "vp4dpwssds\0" "vpabsb\0" "vpabsd\0" "vpabsq\0" "vpabsw\0" "vpackssdw\0"
  "vpacksswb\0" "vpackusdw\0" "vpackuswb\0" "vpaddb\0" "vpaddd\0" "vpaddq\0" "vpaddsb\0" "vpaddsw\0" "vpaddusb\0"
  "vpaddusw\0" "vpaddw\0" "vpalignr\0" "vpand\0" "vpandd\0" "vpandn\0" "vpandnd\0" "vpandnq\0" "vpandq\0" "vpavgb\0"
  "vpavgw\0" "vpblendd\0" "vpblendvb\0" "vpblendw\0" "vpbroadcastb\0" "vpbroadcastd\0" "vpbroadcastmb2d\0"
  "vpbroadcastmb2q\0" "vpbroadcastq\0" "vpbroadcastw\0" "vpclmulqdq\0" "vpcmov\0" "vpcmpb\0" "vpcmpd\0" "vpcmpeqb\0"
  "vpcmpeqd\0" "vpcmpeqq\0" "vpcmpeqw\0" "vpcmpestri\0" "vpcmpestrm\0" "vpcmpgtb\0" "vpcmpgtd\0" "vpcmpgtq\0"
  "vpcmpgtw\0" "vpcmpistri\0" "vpcmpistrm\0" "vpcmpq\0" "vpcmpub\0" "vpcmpud\0" "vpcmpuq\0" "vpcmpuw\0" "vpcmpw\0"
  "vpcomb\0" "vpcomd\0" "vpcompressb\0" "vpcompressd\0" "vpcompressq\0" "vpcompressw\0" "vpcomq\0" "vpcomub\0"
  "vpcomud\0" "vpcomuq\0" "vpcomuw\0" "vpcomw\0" "vpconflictd\0" "vpconflictq\0" "vpdpbusd\0" "vpdpbusds\0"
  "vpdpwssd\0" "vpdpwssds\0" "vperm2f128\0" "vperm2i128\0" "vpermb\0" "vpermd\0" "vpermi2b\0" "vpermi2d\0"
  "vpermi2pd\0" "vpermi2ps\0" "vpermi2q\0" "vpermi2w\0" "vpermil2pd\0" "vpermil2ps\0" "vpermilpd\0" "vpermilps\0"
  "vpermpd\0" "vpermps\0" "vpermq\0" "vpermt2b\0" "vpermt2d\0" "vpermt2pd\0" "vpermt2ps\0" "vpermt2q\0" "vpermt2w\0"
  "vpermw\0" "vpexpandb\0" "vpexpandd\0" "vpexpandq\0" "vpexpandw\0" "vpextrb\0" "vpextrd\0" "vpextrq\0" "vpextrw\0"
  "vpgatherdd\0" "vpgatherdq\0" "vpgatherqd\0" "vpgatherqq\0" "vphaddbd\0" "vphaddbq\0" "vphaddbw\0" "vphaddd\0"
  "vphadddq\0" "vphaddsw\0" "vphaddubd\0" "vphaddubq\0" "vphaddubw\0" "vphaddudq\0" "vphadduwd\0" "vphadduwq\0"
  "vphaddw\0" "vphaddwd\0" "vphaddwq\0" "vphminposuw\0" "vphsubbw\0" "vphsubd\0" "vphsubdq\0" "vphsubsw\0" "vphsubw\0"
  "vphsubwd\0" "vpinsrb\0" "vpinsrd\0" "vpinsrq\0" "vpinsrw\0" "vplzcntd\0" "vplzcntq\0" "vpmacsdd\0" "vpmacsdqh\0"
  "vpmacsdql\0" "vpmacssdd\0" "vpmacssdqh\0" "vpmacssdql\0" "vpmacsswd\0" "vpmacssww\0" "vpmacswd\0" "vpmacsww\0"
  "vpmadcsswd\0" "vpmadcswd\0" "vpmadd52huq\0" "vpmadd52luq\0" "vpmaddubsw\0" "vpmaddwd\0" "vpmaskmovd\0"
  "vpmaskmovq\0" "vpmaxsb\0" "vpmaxsd\0" "vpmaxsq\0" "vpmaxsw\0" "vpmaxub\0" "vpmaxud\0" "vpmaxuq\0" "vpmaxuw\0"
  "vpminsb\0" "vpminsd\0" "vpminsq\0" "vpminsw\0" "vpminub\0" "vpminud\0" "vpminuq\0" "vpminuw\0" "vpmovb2m\0"
  "vpmovd2m\0" "vpmovdb\0" "vpmovdw\0" "vpmovm2b\0" "vpmovm2d\0" "vpmovm2q\0" "vpmovm2w\0" "vpmovmskb\0" "vpmovq2m\0"
  "vpmovqb\0" "vpmovqd\0" "vpmovqw\0" "vpmovsdb\0" "vpmovsdw\0" "vpmovsqb\0" "vpmovsqd\0" "vpmovsqw\0" "vpmovswb\0"
  "vpmovsxbd\0" "vpmovsxbq\0" "vpmovsxbw\0" "vpmovsxdq\0" "vpmovsxwd\0" "vpmovsxwq\0" "vpmovusdb\0" "vpmovusdw\0"
  "vpmovusqb\0" "vpmovusqd\0" "vpmovusqw\0" "vpmovuswb\0" "vpmovw2m\0" "vpmovwb\0" "vpmovzxbd\0" "vpmovzxbq\0"
  "vpmovzxbw\0" "vpmovzxdq\0" "vpmovzxwd\0" "vpmovzxwq\0" "vpmuldq\0" "vpmulhrsw\0" "vpmulhuw\0" "vpmulhw\0"
  "vpmulld\0" "vpmullq\0" "vpmullw\0" "vpmultishiftqb\0" "vpmuludq\0" "vpopcntb\0" "vpopcntd\0" "vpopcntq\0"
  "vpopcntw\0" "vpor\0" "vpord\0" "vporq\0" "vpperm\0" "vprold\0" "vprolq\0" "vprolvd\0" "vprolvq\0" "vprord\0"
  "vprorq\0" "vprorvd\0" "vprorvq\0" "vprotb\0" "vprotd\0" "vprotq\0" "vprotw\0" "vpsadbw\0" "vpscatterdd\0"
  "vpscatterdq\0" "vpscatterqd\0" "vpscatterqq\0" "vpshab\0" "vpshad\0" "vpshaq\0" "vpshaw\0" "vpshlb\0" "vpshld\0"
  "vpshldd\0" "vpshldq\0" "vpshldvd\0" "vpshldvq\0" "vpshldvw\0" "vpshldw\0" "vpshlq\0" "vpshlw\0" "vpshrdd\0"
  "vpshrdq\0" "vpshrdvd\0" "vpshrdvq\0" "vpshrdvw\0" "vpshrdw\0" "vpshufb\0" "vpshufbitqmb\0" "vpshufd\0" "vpshufhw\0"
  "vpshuflw\0" "vpsignb\0" "vpsignd\0" "vpsignw\0" "vpslld\0" "vpslldq\0" "vpsllq\0" "vpsllvd\0" "vpsllvq\0"
  "vpsllvw\0" "vpsllw\0" "vpsrad\0" "vpsraq\0" "vpsravd\0" "vpsravq\0" "vpsravw\0" "vpsraw\0" "vpsrld\0" "vpsrldq\0"
  "vpsrlq\0" "vpsrlvd\0" "vpsrlvq\0" "vpsrlvw\0" "vpsrlw\0" "vpsubb\0" "vpsubd\0" "vpsubq\0" "vpsubsb\0" "vpsubsw\0"
  "vpsubusb\0" "vpsubusw\0" "vpsubw\0" "vpternlogd\0" "vpternlogq\0" "vptest\0" "vptestmb\0" "vptestmd\0" "vptestmq\0"
  "vptestmw\0" "vptestnmb\0" "vptestnmd\0" "vptestnmq\0" "vptestnmw\0" "vpunpckhbw\0" "vpunpckhdq\0" "vpunpckhqdq\0"
  "vpunpckhwd\0" "vpunpcklbw\0" "vpunpckldq\0" "vpunpcklqdq\0" "vpunpcklwd\0" "vpxor\0" "vpxord\0" "vpxorq\0"
  "vrangepd\0" "vrangeps\0" "vrangesd\0" "vrangess\0" "vrcp14pd\0" "vrcp14ps\0" "vrcp14sd\0" "vrcp14ss\0" "vrcp28pd\0"
  "vrcp28ps\0" "vrcp28sd\0" "vrcp28ss\0" "vrcpps\0" "vrcpss\0" "vreducepd\0" "vreduceps\0" "vreducesd\0" "vreducess\0"
  "vrndscalepd\0" "vrndscaleps\0" "vrndscalesd\0" "vrndscaless\0" "vroundpd\0" "vroundps\0" "vroundsd\0" "vroundss\0"
  "vrsqrt14pd\0" "vrsqrt14ps\0" "vrsqrt14sd\0" "vrsqrt14ss\0" "vrsqrt28pd\0" "vrsqrt28ps\0" "vrsqrt28sd\0"
  "vrsqrt28ss\0" "vrsqrtps\0" "vrsqrtss\0" "vscalefpd\0" "vscalefps\0" "vscalefsd\0" "vscalefss\0" "vscatterdpd\0"
  "vscatterdps\0" "vscatterpf0dpd\0" "vscatterpf0dps\0" "vscatterpf0qpd\0" "vscatterpf0qps\0" "vscatterpf1dpd\0"
  "vscatterpf1dps\0" "vscatterpf1qpd\0" "vscatterpf1qps\0" "vscatterqpd\0" "vscatterqps\0" "vshuff32x4\0"
  "vshuff64x2\0" "vshufi32x4\0" "vshufi64x2\0" "vshufpd\0" "vshufps\0" "vsqrtpd\0" "vsqrtps\0" "vsqrtsd\0" "vsqrtss\0"
  "vstmxcsr\0" "vsubpd\0" "vsubps\0" "vsubsd\0" "vsubss\0" "vtestpd\0" "vtestps\0" "vucomisd\0" "vucomiss\0"
  "vunpckhpd\0" "vunpckhps\0" "vunpcklpd\0" "vunpcklps\0" "vxorpd\0" "vxorps\0" "vzeroall\0" "vzeroupper\0" "wbinvd\0"
  "wbnoinvd\0" "wrfsbase\0" "wrgsbase\0" "wrmsr\0" "xabort\0" "xadd\0" "xbegin\0" "xend\0" "xgetbv\0" "xlatb\0"
  "xrstors\0" "xrstors64\0" "xsavec\0" "xsavec64\0" "xsaveopt\0" "xsaveopt64\0" "xsaves\0" "xsaves64\0" "xsetbv\0"
  "xtest";

const InstDB::InstNameIndex InstDB::instNameIndex[26] = {
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
#endif // !ASMJIT_NO_TEXT

// ============================================================================
// [asmjit::x86::InstDB - InstSignature / OpSignature]
// ============================================================================

#ifndef ASMJIT_NO_VALIDATION
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
  ROW(2, 1, 0, 0, 97 , 53 , 0  , 0  , 0  , 0  ), // #253 {es:[memBase], m512|mem}
  ROW(2, 0, 1, 0, 97 , 53 , 0  , 0  , 0  , 0  ), //      {es:[memBase], m512|mem}
  ROW(3, 1, 1, 0, 45 , 10 , 10 , 0  , 0  , 0  ), // #255 {xmm, i8|u8, i8|u8}
  ROW(2, 1, 1, 0, 45 , 45 , 0  , 0  , 0  , 0  ), // #256 {xmm, xmm}
  ROW(0, 1, 1, 0, 0  , 0  , 0  , 0  , 0  , 0  ), // #257 {}
  ROW(1, 1, 1, 0, 78 , 0  , 0  , 0  , 0  , 0  ), // #258 {st}
  ROW(0, 1, 1, 0, 0  , 0  , 0  , 0  , 0  , 0  ), // #259 {}
  ROW(1, 1, 1, 0, 98 , 0  , 0  , 0  , 0  , 0  ), // #260 {m32|m64|st}
  ROW(2, 1, 1, 0, 45 , 45 , 0  , 0  , 0  , 0  ), // #261 {xmm, xmm}
  ROW(4, 1, 1, 0, 45 , 45 , 10 , 10 , 0  , 0  ), //      {xmm, xmm, i8|u8, i8|u8}
  ROW(2, 1, 0, 0, 6  , 47 , 0  , 0  , 0  , 0  ), // #263 {r32, m128|mem}
  ROW(2, 0, 1, 0, 8  , 47 , 0  , 0  , 0  , 0  ), //      {r64, m128|mem}
  ROW(2, 1, 0, 2, 36 , 99 , 0  , 0  , 0  , 0  ), // #265 {<eax>, <ecx>}
  ROW(2, 0, 1, 2, 100, 99 , 0  , 0  , 0  , 0  ), //      {<eax|rax>, <ecx>}
  ROW(1, 1, 1, 0, 101, 0  , 0  , 0  , 0  , 0  ), // #267 {rel8|rel32}
  ROW(1, 1, 0, 0, 102, 0  , 0  , 0  , 0  , 0  ), //      {rel16}
  ROW(2, 1, 0, 1, 103, 104, 0  , 0  , 0  , 0  ), // #269 {<cx|ecx>, rel8}
  ROW(2, 0, 1, 1, 105, 104, 0  , 0  , 0  , 0  ), //      {<ecx|rcx>, rel8}
  ROW(1, 1, 1, 0, 106, 0  , 0  , 0  , 0  , 0  ), // #271 {rel8|rel32|r64|m64|mem}
  ROW(1, 1, 0, 0, 107, 0  , 0  , 0  , 0  , 0  ), //      {rel16|r32|m32|mem}
  ROW(2, 1, 1, 0, 84 , 108, 0  , 0  , 0  , 0  ), // #273 {k, k|m8|mem|r32|r8lo|r8hi|r16}
  ROW(2, 1, 1, 0, 109, 84 , 0  , 0  , 0  , 0  ), //      {m8|mem|r32|r8lo|r8hi|r16, k}
  ROW(2, 1, 1, 0, 84 , 110, 0  , 0  , 0  , 0  ), // #275 {k, k|m32|mem|r32}
  ROW(2, 1, 1, 0, 28 , 84 , 0  , 0  , 0  , 0  ), //      {m32|mem|r32, k}
  ROW(2, 1, 1, 0, 84 , 111, 0  , 0  , 0  , 0  ), // #277 {k, k|m64|mem|r64}
  ROW(2, 1, 1, 0, 15 , 84 , 0  , 0  , 0  , 0  ), //      {m64|mem|r64, k}
  ROW(2, 1, 1, 0, 84 , 112, 0  , 0  , 0  , 0  ), // #279 {k, k|m16|mem|r32|r16}
  ROW(2, 1, 1, 0, 113, 84 , 0  , 0  , 0  , 0  ), //      {m16|mem|r32|r16, k}
  ROW(2, 1, 1, 0, 4  , 27 , 0  , 0  , 0  , 0  ), // #281 {r16, r16|m16|mem}
  ROW(2, 1, 1, 0, 6  , 113, 0  , 0  , 0  , 0  ), //      {r32, r32|m16|mem|r16}
  ROW(2, 1, 0, 0, 4  , 29 , 0  , 0  , 0  , 0  ), // #283 {r16, m32|mem}
  ROW(2, 1, 0, 0, 6  , 79 , 0  , 0  , 0  , 0  ), //      {r32, m48|mem}
  ROW(2, 1, 1, 0, 4  , 27 , 0  , 0  , 0  , 0  ), // #285 {r16, r16|m16|mem}
  ROW(2, 1, 1, 0, 114, 113, 0  , 0  , 0  , 0  ), //      {r32|r64, r32|m16|mem|r16}
  ROW(2, 1, 1, 0, 59 , 28 , 0  , 0  , 0  , 0  ), // #287 {mm|xmm, r32|m32|mem}
  ROW(2, 1, 1, 0, 28 , 59 , 0  , 0  , 0  , 0  ), //      {r32|m32|mem, mm|xmm}
  ROW(2, 1, 1, 0, 45 , 87 , 0  , 0  , 0  , 0  ), // #289 {xmm, xmm|m32|mem}
  ROW(2, 1, 1, 0, 29 , 45 , 0  , 0  , 0  , 0  ), //      {m32|mem, xmm}
  ROW(2, 1, 1, 0, 4  , 9  , 0  , 0  , 0  , 0  ), // #291 {r16, r8lo|r8hi|m8}
  ROW(2, 1, 1, 0, 114, 115, 0  , 0  , 0  , 0  ), //      {r32|r64, r8lo|r8hi|m8|r16|m16}
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
  ROW(1, 1, 1, 0, 114, 0  , 0  , 0  , 0  , 0  ), // #346 {r32|r64}
  ROW(1, 1, 1, 1, 33 , 0  , 0  , 0  , 0  , 0  ), // #347 {<ax>}
  ROW(2, 1, 1, 2, 35 , 36 , 0  , 0  , 0  , 0  ), // #348 {<edx>, <eax>}
  ROW(1, 1, 1, 0, 91 , 0  , 0  , 0  , 0  , 0  ), // #349 {mem}
  ROW(1, 1, 1, 1, 124, 0  , 0  , 0  , 0  , 0  ), // #350 {<ds:[memBase|zax]>}
  ROW(2, 1, 1, 2, 125, 126, 0  , 0  , 0  , 0  ), // #351 {<ds:[memBase|zsi]>, <es:[memBase|zdi]>}
  ROW(3, 1, 1, 0, 45 , 60 , 10 , 0  , 0  , 0  ), // #352 {xmm, xmm|m64|mem, i8|u8}
  ROW(3, 1, 1, 0, 45 , 87 , 10 , 0  , 0  , 0  ), // #353 {xmm, xmm|m32|mem, i8|u8}
  ROW(5, 0, 1, 4, 47 , 37 , 38 , 127, 128, 0  ), // #354 {m128|mem, <rdx>, <rax>, <rcx>, <rbx>}
  ROW(5, 1, 1, 4, 30 , 35 , 36 , 99 , 129, 0  ), // #355 {m64|mem, <edx>, <eax>, <ecx>, <ebx>}
  ROW(4, 1, 1, 4, 36 , 129, 99 , 35 , 0  , 0  ), // #356 {<eax>, <ebx>, <ecx>, <edx>}
  ROW(2, 0, 1, 2, 37 , 38 , 0  , 0  , 0  , 0  ), // #357 {<rdx>, <rax>}
  ROW(2, 1, 1, 0, 57 , 46 , 0  , 0  , 0  , 0  ), // #358 {mm, xmm|m128|mem}
  ROW(2, 1, 1, 0, 45 , 117, 0  , 0  , 0  , 0  ), // #359 {xmm, mm|m64|mem}
  ROW(2, 1, 1, 0, 57 , 60 , 0  , 0  , 0  , 0  ), // #360 {mm, xmm|m64|mem}
  ROW(2, 1, 1, 0, 114, 60 , 0  , 0  , 0  , 0  ), // #361 {r32|r64, xmm|m64|mem}
  ROW(2, 1, 1, 0, 45 , 130, 0  , 0  , 0  , 0  ), // #362 {xmm, r32|m32|mem|r64|m64}
  ROW(2, 1, 1, 0, 114, 87 , 0  , 0  , 0  , 0  ), // #363 {r32|r64, xmm|m32|mem}
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
  ROW(3, 1, 1, 0, 114, 28 , 14 , 0  , 0  , 0  ), // #388 {r32|r64, r32|m32|mem, i32|u32}
  ROW(3, 1, 1, 1, 45 , 45 , 143, 0  , 0  , 0  ), // #389 {xmm, xmm, <ds:[memBase|zdi]>}
  ROW(3, 1, 1, 1, 57 , 57 , 143, 0  , 0  , 0  ), // #390 {mm, mm, <ds:[memBase|zdi]>}
  ROW(3, 1, 1, 3, 124, 99 , 35 , 0  , 0  , 0  ), // #391 {<ds:[memBase|zax]>, <ecx>, <edx>}
  ROW(2, 1, 1, 0, 97 , 53 , 0  , 0  , 0  , 0  ), // #392 {es:[memBase], m512|mem}
  ROW(2, 1, 1, 0, 57 , 45 , 0  , 0  , 0  , 0  ), // #393 {mm, xmm}
  ROW(2, 1, 1, 0, 6  , 45 , 0  , 0  , 0  , 0  ), // #394 {r32, xmm}
  ROW(2, 1, 1, 0, 30 , 57 , 0  , 0  , 0  , 0  ), // #395 {m64|mem, mm}
  ROW(2, 1, 1, 0, 45 , 57 , 0  , 0  , 0  , 0  ), // #396 {xmm, mm}
  ROW(2, 1, 1, 2, 126, 125, 0  , 0  , 0  , 0  ), // #397 {<es:[memBase|zdi]>, <ds:[memBase|zsi]>}
  ROW(2, 0, 1, 0, 8  , 28 , 0  , 0  , 0  , 0  ), // #398 {r64, r32|m32|mem}
  ROW(2, 1, 1, 2, 36 , 99 , 0  , 0  , 0  , 0  ), // #399 {<eax>, <ecx>}
  ROW(3, 1, 1, 3, 36 , 99 , 129, 0  , 0  , 0  ), // #400 {<eax>, <ecx>, <ebx>}
  ROW(2, 1, 1, 0, 144, 135, 0  , 0  , 0  , 0  ), // #401 {u8|dx, al|ax|eax}
  ROW(2, 1, 1, 0, 138, 145, 0  , 0  , 0  , 0  ), // #402 {dx, ds:[memBase|zsi]}
  ROW(6, 1, 1, 3, 45 , 46 , 10 , 99 , 36 , 35 ), // #403 {xmm, xmm|m128|mem, i8|u8, <ecx>, <eax>, <edx>}
  ROW(6, 1, 1, 3, 45 , 46 , 10 , 122, 36 , 35 ), // #404 {xmm, xmm|m128|mem, i8|u8, <xmm0>, <eax>, <edx>}
  ROW(4, 1, 1, 1, 45 , 46 , 10 , 99 , 0  , 0  ), // #405 {xmm, xmm|m128|mem, i8|u8, <ecx>}
  ROW(4, 1, 1, 1, 45 , 46 , 10 , 122, 0  , 0  ), // #406 {xmm, xmm|m128|mem, i8|u8, <xmm0>}
  ROW(3, 1, 1, 0, 109, 45 , 10 , 0  , 0  , 0  ), // #407 {r32|m8|mem|r8lo|r8hi|r16, xmm, i8|u8}
  ROW(3, 0, 1, 0, 15 , 45 , 10 , 0  , 0  , 0  ), // #408 {r64|m64|mem, xmm, i8|u8}
  ROW(3, 1, 1, 0, 45 , 109, 10 , 0  , 0  , 0  ), // #409 {xmm, r32|m8|mem|r8lo|r8hi|r16, i8|u8}
  ROW(3, 1, 1, 0, 45 , 28 , 10 , 0  , 0  , 0  ), // #410 {xmm, r32|m32|mem, i8|u8}
  ROW(3, 0, 1, 0, 45 , 15 , 10 , 0  , 0  , 0  ), // #411 {xmm, r64|m64|mem, i8|u8}
  ROW(3, 1, 1, 0, 59 , 113, 10 , 0  , 0  , 0  ), // #412 {mm|xmm, r32|m16|mem|r16, i8|u8}
  ROW(2, 1, 1, 0, 6  , 59 , 0  , 0  , 0  , 0  ), // #413 {r32, mm|xmm}
  ROW(2, 1, 1, 0, 45 , 10 , 0  , 0  , 0  , 0  ), // #414 {xmm, i8|u8}
  ROW(2, 1, 1, 0, 31 , 81 , 0  , 0  , 0  , 0  ), // #415 {r8lo|r8hi|m8|r16|m16|r32|m32|r64|m64|mem, cl|i8|u8}
  ROW(1, 0, 1, 0, 114, 0  , 0  , 0  , 0  , 0  ), // #416 {r32|r64}
  ROW(3, 1, 1, 3, 35 , 36 , 99 , 0  , 0  , 0  ), // #417 {<edx>, <eax>, <ecx>}
  ROW(1, 1, 1, 0, 140, 0  , 0  , 0  , 0  , 0  ), // #418 {r16|r32|r64}
  ROW(2, 1, 1, 2, 142, 126, 0  , 0  , 0  , 0  ), // #419 {<al|ax|eax|rax>, <es:[memBase|zdi]>}
  ROW(1, 1, 1, 0, 1  , 0  , 0  , 0  , 0  , 0  ), // #420 {r8lo|r8hi|m8|mem}
  ROW(1, 1, 1, 0, 146, 0  , 0  , 0  , 0  , 0  ), // #421 {r16|m16|mem|r32|r64}
  ROW(2, 1, 1, 2, 126, 142, 0  , 0  , 0  , 0  ), // #422 {<es:[memBase|zdi]>, <al|ax|eax|rax>}
  ROW(6, 1, 1, 0, 51 , 51 , 51 , 51 , 51 , 47 ), // #423 {zmm, zmm, zmm, zmm, zmm, m128|mem}
  ROW(6, 1, 1, 0, 45 , 45 , 45 , 45 , 45 , 47 ), // #424 {xmm, xmm, xmm, xmm, xmm, m128|mem}
  ROW(3, 1, 1, 0, 45 , 45 , 60 , 0  , 0  , 0  ), // #425 {xmm, xmm, xmm|m64|mem}
  ROW(3, 1, 1, 0, 45 , 45 , 87 , 0  , 0  , 0  ), // #426 {xmm, xmm, xmm|m32|mem}
  ROW(2, 1, 1, 0, 48 , 47 , 0  , 0  , 0  , 0  ), // #427 {ymm, m128|mem}
  ROW(2, 1, 1, 0, 147, 60 , 0  , 0  , 0  , 0  ), // #428 {ymm|zmm, xmm|m64|mem}
  ROW(2, 1, 1, 0, 147, 47 , 0  , 0  , 0  , 0  ), // #429 {ymm|zmm, m128|mem}
  ROW(2, 1, 1, 0, 51 , 50 , 0  , 0  , 0  , 0  ), // #430 {zmm, m256|mem}
  ROW(2, 1, 1, 0, 148, 60 , 0  , 0  , 0  , 0  ), // #431 {xmm|ymm|zmm, xmm|m64|mem}
  ROW(2, 1, 1, 0, 148, 87 , 0  , 0  , 0  , 0  ), // #432 {xmm|ymm|zmm, m32|mem|xmm}
  ROW(4, 1, 1, 0, 82 , 45 , 60 , 10 , 0  , 0  ), // #433 {xmm|k, xmm, xmm|m64|mem, i8|u8}
  ROW(4, 1, 1, 0, 82 , 45 , 87 , 10 , 0  , 0  ), // #434 {xmm|k, xmm, xmm|m32|mem, i8|u8}
  ROW(3, 1, 1, 0, 45 , 45 , 130, 0  , 0  , 0  ), // #435 {xmm, xmm, r32|m32|mem|r64|m64}
  ROW(3, 1, 1, 0, 46 , 147, 10 , 0  , 0  , 0  ), // #436 {xmm|m128|mem, ymm|zmm, i8|u8}
  ROW(4, 1, 1, 0, 45 , 45 , 60 , 10 , 0  , 0  ), // #437 {xmm, xmm, xmm|m64|mem, i8|u8}
  ROW(4, 1, 1, 0, 45 , 45 , 87 , 10 , 0  , 0  ), // #438 {xmm, xmm, xmm|m32|mem, i8|u8}
  ROW(3, 1, 1, 0, 84 , 149, 10 , 0  , 0  , 0  ), // #439 {k, xmm|m128|ymm|m256|zmm|m512, i8|u8}
  ROW(3, 1, 1, 0, 84 , 60 , 10 , 0  , 0  , 0  ), // #440 {k, xmm|m64|mem, i8|u8}
  ROW(3, 1, 1, 0, 84 , 87 , 10 , 0  , 0  , 0  ), // #441 {k, xmm|m32|mem, i8|u8}
  ROW(1, 1, 1, 0, 62 , 0  , 0  , 0  , 0  , 0  ), // #442 {vm32y}
  ROW(1, 1, 1, 0, 63 , 0  , 0  , 0  , 0  , 0  ), // #443 {vm32z}
  ROW(1, 1, 1, 0, 66 , 0  , 0  , 0  , 0  , 0  ), // #444 {vm64z}
  ROW(4, 1, 1, 0, 51 , 51 , 49 , 10 , 0  , 0  ), // #445 {zmm, zmm, ymm|m256|mem, i8|u8}
  ROW(1, 1, 1, 0, 30 , 0  , 0  , 0  , 0  , 0  ), // #446 {m64|mem}
  ROW(2, 1, 1, 0, 6  , 86 , 0  , 0  , 0  , 0  ), // #447 {r32, xmm|ymm}
  ROW(2, 1, 1, 0, 148, 150, 0  , 0  , 0  , 0  ), // #448 {xmm|ymm|zmm, xmm|m8|mem|r32|r8lo|r8hi|r16}
  ROW(2, 1, 1, 0, 148, 151, 0  , 0  , 0  , 0  ), // #449 {xmm|ymm|zmm, xmm|m32|mem|r32}
  ROW(2, 1, 1, 0, 148, 84 , 0  , 0  , 0  , 0  ), // #450 {xmm|ymm|zmm, k}
  ROW(2, 1, 1, 0, 148, 152, 0  , 0  , 0  , 0  ), // #451 {xmm|ymm|zmm, xmm|m16|mem|r32|r16}
  ROW(3, 1, 1, 0, 113, 45 , 10 , 0  , 0  , 0  ), // #452 {r32|m16|mem|r16, xmm, i8|u8}
  ROW(4, 1, 1, 0, 45 , 45 , 109, 10 , 0  , 0  ), // #453 {xmm, xmm, r32|m8|mem|r8lo|r8hi|r16, i8|u8}
  ROW(4, 1, 1, 0, 45 , 45 , 28 , 10 , 0  , 0  ), // #454 {xmm, xmm, r32|m32|mem, i8|u8}
  ROW(4, 0, 1, 0, 45 , 45 , 15 , 10 , 0  , 0  ), // #455 {xmm, xmm, r64|m64|mem, i8|u8}
  ROW(4, 1, 1, 0, 45 , 45 , 113, 10 , 0  , 0  ), // #456 {xmm, xmm, r32|m16|mem|r16, i8|u8}
  ROW(2, 1, 1, 0, 84 , 148, 0  , 0  , 0  , 0  ), // #457 {k, xmm|ymm|zmm}
  ROW(1, 1, 1, 0, 102, 0  , 0  , 0  , 0  , 0  ), // #458 {rel16|rel32}
  ROW(3, 1, 1, 2, 91 , 35 , 36 , 0  , 0  , 0  ), // #459 {mem, <edx>, <eax>}
  ROW(3, 0, 1, 2, 91 , 35 , 36 , 0  , 0  , 0  )  // #460 {mem, <edx>, <eax>}
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
  ROW(F(Mem), M(BaseOnly) | M(Es), 0, 0x00),
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
#endif // !ASMJIT_NO_VALIDATION

// ============================================================================
// [asmjit::x86::InstInternal - QueryRWInfo]
// ============================================================================

// ${InstRWInfoTable:Begin}
// ------------------- Automatically generated, do not edit -------------------
const uint8_t InstDB::rwInfoIndex[Inst::_kIdCount * 2] = {
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
  0, 48, 49, 50, 4, 0, 4, 0, 5, 0, 6, 0, 0, 4, 0, 4, 0, 0, 51, 0, 51, 0, 0, 0,
  0, 52, 53, 54, 0, 0, 0, 0, 55, 56, 0, 57, 0, 58, 0, 59, 0, 0, 0, 0, 0, 57, 0,
  57, 0, 57, 0, 57, 0, 57, 0, 57, 0, 57, 0, 57, 0, 60, 0, 61, 0, 61, 0, 60, 0,
  0, 0, 0, 0, 0, 55, 56, 0, 57, 55, 56, 0, 57, 0, 0, 0, 57, 0, 56, 0, 56, 0, 56,
  0, 56, 0, 56, 0, 56, 0, 56, 0, 0, 0, 0, 0, 62, 0, 62, 0, 62, 0, 56, 0, 56, 0,
  60, 0, 0, 0, 63, 0, 24, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 55, 56, 0, 57, 0,
  0, 0, 0, 0, 0, 0, 64, 0, 65, 0, 64, 0, 66, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 24,
  0, 64, 0, 0, 0, 0, 0, 0, 0, 0, 0, 67, 0, 65, 0, 64, 0, 67, 0, 66, 55, 56, 0,
  57, 55, 56, 0, 57, 0, 0, 0, 61, 0, 61, 0, 61, 0, 61, 0, 0, 0, 0, 0, 0, 0, 57,
  0, 24, 0, 24, 0, 64, 0, 64, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 4, 4, 0, 4, 0,
  4, 0, 0, 0, 4, 0, 4, 0, 49, 50, 68, 69, 70, 0, 0, 48, 71, 0, 0, 72, 53, 53, 0,
  0, 0, 0, 0, 0, 0, 0, 73, 0, 0, 24, 74, 0, 73, 0, 73, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 75, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 21, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 76, 0, 77, 0, 78, 0, 79, 0, 76, 0,
  77, 0, 76, 0, 77, 0, 78, 0, 79, 0, 78, 0, 79, 80, 0, 81, 0, 82, 0, 83, 0, 84,
  0, 85, 0, 86, 0, 87, 0, 0, 76, 0, 77, 0, 78, 88, 0, 89, 0, 90, 0, 91, 0, 0, 79,
  0, 84, 0, 85, 0, 86, 0, 87, 0, 84, 0, 85, 0, 86, 0, 87, 88, 0, 89, 0, 90, 0,
  91, 0, 0, 92, 0, 93, 0, 94, 0, 76, 0, 77, 0, 78, 0, 79, 0, 76, 0, 77, 0, 78,
  0, 79, 0, 95, 96, 0, 97, 0, 0, 98, 99, 0, 100, 0, 0, 0, 99, 0, 0, 0, 99, 0, 0,
  24, 99, 0, 0, 24, 0, 101, 0, 102, 0, 101, 103, 0, 104, 0, 104, 0, 104, 0, 96,
  0, 99, 0, 0, 101, 0, 105, 0, 105, 11, 0, 0, 106, 0, 107, 4, 0, 4, 0, 5, 0, 6,
  0, 0, 0, 4, 0, 4, 0, 5, 0, 6, 0, 0, 108, 0, 108, 109, 0, 110, 0, 110, 0, 111,
  0, 81, 0, 36, 0, 112, 0, 111, 0, 86, 0, 110, 0, 110, 0, 113, 0, 114, 0, 114,
  0, 115, 0, 116, 0, 116, 0, 117, 0, 117, 0, 97, 0, 97, 0, 111, 0, 97, 0, 97, 0,
  116, 0, 116, 0, 118, 0, 82, 0, 86, 0, 119, 0, 82, 0, 7, 0, 7, 0, 81, 0, 120,
  0, 121, 0, 110, 0, 110, 0, 120, 0, 0, 4, 49, 122, 4, 0, 4, 0, 5, 0, 6, 0, 0,
  123, 124, 0, 0, 125, 0, 48, 0, 126, 0, 48, 2, 0, 4, 0, 4, 0, 127, 0, 128, 0, 11,
  0, 11, 0, 11, 0, 3, 0, 3, 0, 4, 0, 3, 0, 3, 0, 3, 0, 3, 0, 3, 0, 3, 0, 3, 0,
  3, 0, 3, 0, 0, 3, 3, 0, 3, 0, 0, 0, 3, 0, 129, 0, 3, 0, 0, 12, 0, 4, 0, 4, 3,
  0, 3, 0, 4, 0, 3, 0, 0, 130, 0, 131, 3, 0, 3, 0, 4, 0, 3, 0, 0, 132, 0, 133,
  0, 0, 0, 8, 0, 8, 0, 134, 0, 52, 0, 135, 0, 136, 39, 0, 39, 0, 129, 0, 129, 0,
  129, 0, 129, 0, 129, 0, 129, 0, 129, 0, 129, 0, 129, 0, 129, 0, 39, 0, 129,
  0, 129, 0, 129, 0, 39, 0, 39, 0, 129, 0, 129, 0, 129, 0, 3, 0, 3, 0, 3, 0, 137,
  0, 3, 0, 3, 0, 3, 0, 39, 0, 39, 0, 0, 138, 0, 72, 0, 139, 0, 140, 3, 0, 3, 0,
  4, 0, 4, 0, 3, 0, 3, 0, 4, 0, 4, 0, 4, 0, 4, 0, 3, 0, 3, 0, 4, 0, 4, 0, 141,
  0, 142, 0, 143, 0, 36, 0, 36, 0, 36, 0, 142, 0, 142, 0, 143, 0, 36, 0, 36, 0,
  36, 0, 142, 0, 4, 0, 3, 0, 129, 0, 3, 0, 3, 0, 4, 0, 3, 0, 3, 0, 0, 144, 0, 0,
  0, 0, 11, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 24, 0, 24, 0, 24, 0, 24, 0, 24, 0, 24,
  0, 24, 3, 0, 3, 0, 0, 7, 0, 7, 0, 7, 0, 39, 3, 0, 3, 0, 3, 0, 3, 0, 54, 0,
  3, 0, 3, 0, 3, 0, 3, 0, 3, 0, 54, 0, 3, 0, 3, 0, 3, 0, 3, 0, 3, 0, 3, 0, 3, 0,
  3, 0, 3, 0, 3, 0, 39, 0, 145, 0, 3, 0, 3, 0, 4, 0, 3, 0, 3, 0, 3, 0, 4, 0, 3,
  0, 0, 146, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 147, 0, 7, 0, 148, 0, 147, 0,
  0, 149, 0, 149, 0, 150, 0, 149, 0, 150, 0, 149, 0, 149, 151, 0, 0, 152, 0, 0,
  147, 0, 147, 0, 0, 11, 0, 7, 0, 7, 0, 38, 0, 148, 0, 0, 7, 0, 148, 0, 0, 153,
  147, 0, 147, 0, 0, 10, 2, 0, 154, 0, 0, 155, 0, 155, 0, 155, 0, 155, 0, 155, 0,
  155, 0, 155, 0, 155, 0, 155, 0, 155, 0, 155, 0, 155, 0, 155, 0, 155, 0, 155,
  0, 155, 0, 155, 0, 155, 0, 155, 0, 155, 0, 155, 0, 155, 0, 155, 0, 155, 0, 155,
  0, 155, 0, 155, 0, 155, 0, 155, 0, 155, 0, 0, 0, 64, 4, 0, 4, 0, 4, 0, 0, 4,
  4, 0, 4, 0, 0, 12, 147, 0, 0, 156, 0, 10, 147, 0, 0, 156, 0, 10, 0, 4, 0, 4,
  0, 64, 0, 47, 0, 157, 0, 149, 0, 157, 7, 0, 7, 0, 38, 0, 148, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 158, 159, 0, 0, 157, 2, 0, 4, 0, 4, 0, 5, 0, 6, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 11, 0, 19, 0, 11, 0, 11, 0, 31, 0, 32, 0,
  0, 0, 4, 0, 4, 0, 4, 0, 4, 0, 0, 160, 0, 161, 0, 160, 0, 161, 0, 8, 0, 8, 0, 162,
  0, 163, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 7, 0, 0, 7, 0, 8, 0, 8, 0, 8,
  0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 164, 0, 164,
  165, 0, 40, 0, 166, 0, 167, 0, 166, 0, 167, 0, 165, 0, 40, 0, 166, 0, 167,
  0, 166, 0, 167, 0, 168, 0, 169, 0, 0, 8, 0, 8, 0, 170, 0, 171, 31, 0, 32, 0,
  172, 0, 172, 0, 173, 0, 11, 0, 0, 8, 120, 0, 174, 0, 174, 0, 11, 0, 174, 0, 11,
  0, 173, 0, 11, 0, 173, 0, 0, 175, 173, 0, 11, 0, 173, 0, 11, 0, 174, 0, 40,
  0, 0, 176, 40, 0, 0, 177, 0, 178, 0, 179, 45, 0, 45, 0, 174, 0, 11, 0, 174, 0,
  11, 0, 11, 0, 173, 0, 11, 0, 173, 0, 40, 0, 40, 0, 45, 0, 45, 0, 173, 0, 11,
  0, 11, 0, 174, 0, 0, 177, 0, 178, 0, 8, 0, 8, 0, 8, 0, 162, 0, 163, 0, 8, 0, 180,
  0, 8, 0, 101, 0, 101, 181, 0, 181, 0, 11, 0, 11, 0, 0, 182, 0, 183, 0, 184,
  0, 183, 0, 184, 0, 182, 0, 183, 0, 184, 0, 183, 0, 184, 0, 52, 0, 185, 0, 185,
  0, 186, 0, 187, 0, 185, 0, 185, 0, 188, 0, 189, 0, 185, 0, 185, 0, 188, 0,
  189, 0, 185, 0, 185, 0, 188, 0, 189, 0, 190, 0, 190, 0, 191, 0, 192, 0, 185, 0,
  185, 0, 185, 0, 185, 0, 185, 0, 185, 0, 190, 0, 190, 0, 185, 0, 185, 0, 188,
  0, 189, 0, 185, 0, 185, 0, 188, 0, 189, 0, 185, 0, 185, 0, 188, 0, 189, 0, 185,
  0, 185, 0, 185, 0, 185, 0, 185, 0, 185, 0, 190, 0, 190, 0, 190, 0, 190, 0,
  191, 0, 192, 0, 185, 0, 185, 0, 188, 0, 189, 0, 185, 0, 185, 0, 188, 0, 189, 0,
  185, 0, 185, 0, 188, 0, 189, 0, 190, 0, 190, 0, 191, 0, 192, 0, 185, 0, 185,
  0, 188, 0, 189, 0, 185, 0, 185, 0, 188, 0, 189, 0, 185, 0, 185, 0, 193, 0, 194,
  0, 190, 0, 190, 0, 191, 0, 192, 0, 195, 0, 195, 0, 39, 0, 121, 11, 0, 11, 0,
  39, 0, 196, 0, 99, 197, 99, 198, 0, 24, 0, 24, 0, 24, 0, 24, 0, 24, 0, 24, 0,
  24, 0, 24, 99, 198, 99, 199, 11, 0, 11, 0, 0, 200, 0, 201, 0, 11, 0, 11, 0,
  200, 0, 201, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 202, 0, 203, 0, 204,
  0, 203, 0, 204, 0, 202, 0, 203, 0, 204, 0, 203, 0, 204, 0, 163, 111, 0, 0, 98,
  0, 106, 0, 205, 0, 205, 0, 8, 0, 8, 0, 162, 0, 163, 0, 0, 0, 206, 0, 0, 0, 8,
  0, 8, 0, 162, 0, 163, 0, 0, 0, 207, 0, 0, 208, 0, 208, 0, 81, 0, 209, 0, 208,
  0, 208, 0, 208, 0, 208, 0, 208, 0, 208, 0, 208, 0, 208, 0, 0, 210, 211, 212,
  211, 212, 0, 213, 116, 214, 116, 214, 215, 0, 216, 0, 111, 0, 111, 0, 111, 0,
  111, 0, 217, 0, 116, 218, 11, 0, 11, 0, 118, 219, 208, 0, 208, 0, 0, 8, 0, 220,
  0, 206, 172, 0, 0, 0, 0, 221, 0, 207, 0, 8, 0, 8, 0, 162, 0, 163, 222, 0, 0,
  220, 0, 8, 0, 8, 0, 223, 0, 223, 11, 0, 11, 0, 11, 0, 11, 0, 0, 8, 0, 8, 0,
  8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0,
  8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 164, 0, 8, 224, 0, 45, 0, 225, 0, 225,
  0, 40, 0, 226, 0, 0, 8, 0, 190, 0, 227, 0, 227, 0, 8, 0, 8, 0, 8, 0, 8, 0,
  130, 0, 131, 0, 8, 0, 8, 0, 8, 0, 8, 0, 132, 0, 133, 0, 227, 0, 227, 0, 227, 0,
  227, 0, 227, 0, 227, 0, 180, 0, 180, 172, 0, 172, 0, 172, 0, 172, 0, 0, 180,
  0, 180, 0, 180, 0, 180, 0, 180, 0, 180, 11, 0, 11, 0, 0, 185, 0, 185, 0, 185,
  0, 185, 0, 228, 0, 228, 0, 8, 0, 8, 0, 8, 0, 185, 0, 8, 0, 8, 0, 185, 0, 185,
  0, 190, 0, 190, 0, 229, 0, 229, 0, 230, 0, 228, 0, 229, 0, 8, 0, 185, 0, 185,
  0, 185, 0, 185, 0, 185, 0, 8, 11, 0, 11, 0, 11, 0, 11, 0, 0, 134, 0, 52, 0, 135,
  0, 231, 99, 198, 99, 197, 99, 199, 99, 198, 7, 0, 7, 0, 7, 0, 0, 8, 7, 0,
  0, 8, 7, 0, 7, 0, 7, 0, 7, 0, 7, 0, 7, 0, 0, 8, 7, 0, 7, 0, 137, 0, 7, 0, 0, 8,
  7, 0, 0, 8, 0, 8, 7, 0, 0, 232, 0, 163, 0, 162, 0, 233, 11, 0, 11, 0, 0, 234,
  0, 234, 0, 234, 0, 234, 0, 234, 0, 234, 0, 234, 0, 234, 0, 234, 0, 234, 0, 234,
  0, 234, 0, 185, 0, 185, 0, 8, 0, 8, 0, 205, 0, 205, 0, 8, 0, 8, 0, 8, 0, 8,
  0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 235,
  0, 235, 0, 236, 0, 175, 0, 225, 0, 225, 0, 225, 0, 225, 0, 141, 0, 235, 0, 237,
  0, 175, 0, 236, 0, 236, 0, 175, 0, 237, 0, 175, 0, 236, 0, 175, 0, 238, 0,
  239, 0, 173, 0, 173, 0, 173, 0, 238, 0, 236, 0, 175, 0, 237, 0, 175, 0, 236, 0,
  175, 0, 235, 0, 175, 0, 238, 0, 239, 0, 173, 0, 173, 0, 173, 0, 238, 0, 0, 8,
  0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 11, 0, 11, 0, 11, 0, 11, 0,
  0, 8, 0, 8, 0, 8, 0, 240, 0, 11, 0, 11, 0, 8, 0, 8, 0, 11, 0, 11, 0, 8, 0, 8,
  0, 241, 0, 241, 0, 241, 0, 241, 0, 8, 111, 0, 111, 0, 242, 0, 111, 0, 0, 241,
  0, 241, 0, 241, 0, 241, 0, 241, 0, 241, 0, 8, 0, 8, 0, 185, 0, 185, 0, 185, 0,
  8, 0, 241, 0, 241, 0, 8, 0, 8, 0, 185, 0, 185, 0, 185, 0, 8, 0, 8, 0, 227, 0,
  11, 0, 11, 0, 11, 0, 8, 0, 8, 0, 8, 0, 243, 0, 244, 0, 243, 0, 8, 0, 8, 0, 8,
  0, 243, 0, 243, 0, 243, 0, 8, 0, 8, 0, 8, 0, 243, 0, 243, 0, 244, 0, 243, 0,
  8, 0, 8, 0, 8, 0, 243, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 185,
  0, 185, 222, 0, 0, 227, 0, 227, 0, 227, 0, 227, 0, 227, 0, 227, 0, 227, 0, 227,
  0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0,
  8, 0, 200, 0, 201, 11, 0, 11, 0, 0, 200, 0, 201, 181, 0, 181, 0, 0, 200, 0, 201,
  11, 0, 0, 201, 0, 11, 0, 11, 0, 200, 0, 201, 0, 11, 0, 11, 0, 200, 0, 201,
  0, 11, 0, 11, 0, 200, 0, 201, 11, 0, 11, 0, 0, 200, 0, 201, 181, 0, 181, 0, 0,
  200, 0, 201, 11, 0, 0, 201, 0, 8, 0, 8, 0, 162, 0, 163, 111, 0, 111, 0, 0, 24,
  0, 24, 0, 24, 0, 24, 0, 24, 0, 24, 0, 24, 0, 24, 111, 0, 242, 0, 0, 8, 0, 8,
  0, 8, 0, 8, 0, 8, 0, 8, 11, 0, 11, 0, 0, 200, 0, 201, 0, 158, 0, 8, 0, 8, 0,
  162, 0, 163, 222, 0, 222, 0, 31, 0, 32, 0, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 102, 0, 102, 0, 245, 0, 0, 246, 0, 0, 0, 247, 0, 0,
  0, 0, 150, 0, 0, 2, 0, 4, 0, 4, 0, 0, 248, 0, 248, 0, 248, 0, 248, 0, 249, 0,
  249, 0, 249, 0, 249, 0, 249, 0, 249, 0, 249, 0, 249, 0, 245, 0, 0
};

const InstDB::RWInfo InstDB::rwInfo[] = {
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 0 , 0 , 0 , 0 , 0 , 0  } }, // #0 [ref=1609x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 1 , 0 , 0 , 0 , 0 , 0  } }, // #1 [ref=7x]
  { InstDB::RWInfo::kCategoryGeneric   , 1 , { 2 , 3 , 0 , 0 , 0 , 0  } }, // #2 [ref=7x]
  { InstDB::RWInfo::kCategoryGeneric   , 2 , { 2 , 3 , 0 , 0 , 0 , 0  } }, // #3 [ref=100x]
  { InstDB::RWInfo::kCategoryGeneric   , 3 , { 4 , 5 , 0 , 0 , 0 , 0  } }, // #4 [ref=69x]
  { InstDB::RWInfo::kCategoryGeneric   , 4 , { 6 , 7 , 0 , 0 , 0 , 0  } }, // #5 [ref=7x]
  { InstDB::RWInfo::kCategoryGeneric   , 5 , { 8 , 9 , 0 , 0 , 0 , 0  } }, // #6 [ref=7x]
  { InstDB::RWInfo::kCategoryGeneric   , 3 , { 10, 5 , 0 , 0 , 0 , 0  } }, // #7 [ref=33x]
  { InstDB::RWInfo::kCategoryGeneric   , 6 , { 11, 3 , 3 , 0 , 0 , 0  } }, // #8 [ref=185x]
  { InstDB::RWInfo::kCategoryGeneric   , 7 , { 12, 13, 0 , 0 , 0 , 0  } }, // #9 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 2 , { 11, 3 , 3 , 0 , 0 , 0  } }, // #10 [ref=5x]
  { InstDB::RWInfo::kCategoryGeneric   , 2 , { 11, 3 , 0 , 0 , 0 , 0  } }, // #11 [ref=80x]
  { InstDB::RWInfo::kCategoryGeneric   , 3 , { 4 , 5 , 14, 0 , 0 , 0  } }, // #12 [ref=4x]
  { InstDB::RWInfo::kCategoryGeneric   , 2 , { 5 , 3 , 0 , 0 , 0 , 0  } }, // #13 [ref=3x]
  { InstDB::RWInfo::kCategoryGeneric   , 8 , { 10, 3 , 0 , 0 , 0 , 0  } }, // #14 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 9 , { 10, 5 , 0 , 0 , 0 , 0  } }, // #15 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 8 , { 11, 5 , 0 , 0 , 0 , 0  } }, // #16 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 3 , 3 , 0 , 0 , 0 , 0  } }, // #17 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 2 , 0 , 0 , 0 , 0 , 0  } }, // #18 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 10, { 3 , 3 , 0 , 0 , 0 , 0  } }, // #19 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 10, { 2 , 3 , 0 , 0 , 0 , 0  } }, // #20 [ref=3x]
  { InstDB::RWInfo::kCategoryGeneric   , 11, { 3 , 0 , 0 , 0 , 0 , 0  } }, // #21 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 15, 16, 0 , 0 , 0 , 0  } }, // #22 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 17, 0 , 0 , 0 , 0 , 0  } }, // #23 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 8 , { 3 , 0 , 0 , 0 , 0 , 0  } }, // #24 [ref=34x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 18, 0 , 0 , 0 , 0 , 0  } }, // #25 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 1 , { 3 , 3 , 0 , 0 , 0 , 0  } }, // #26 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 19, 20, 0 , 0 , 0 , 0  } }, // #27 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 11, { 2 , 3 , 21, 0 , 0 , 0  } }, // #28 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 12, { 4 , 22, 17, 23, 24, 0  } }, // #29 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 13, { 25, 26, 27, 28, 29, 0  } }, // #30 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 4 , { 7 , 7 , 0 , 0 , 0 , 0  } }, // #31 [ref=4x]
  { InstDB::RWInfo::kCategoryGeneric   , 5 , { 9 , 9 , 0 , 0 , 0 , 0  } }, // #32 [ref=4x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 27, 30, 31, 15, 0 , 0  } }, // #33 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 32, 33, 0 , 0 , 0 , 0  } }, // #34 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 14, { 2 , 3 , 0 , 0 , 0 , 0  } }, // #35 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 4 , { 10, 7 , 0 , 0 , 0 , 0  } }, // #36 [ref=10x]
  { InstDB::RWInfo::kCategoryGeneric   , 3 , { 34, 5 , 0 , 0 , 0 , 0  } }, // #37 [ref=5x]
  { InstDB::RWInfo::kCategoryGeneric   , 4 , { 35, 7 , 0 , 0 , 0 , 0  } }, // #38 [ref=3x]
  { InstDB::RWInfo::kCategoryGeneric   , 4 , { 34, 7 , 0 , 0 , 0 , 0  } }, // #39 [ref=13x]
  { InstDB::RWInfo::kCategoryGeneric   , 4 , { 11, 7 , 0 , 0 , 0 , 0  } }, // #40 [ref=9x]
  { InstDB::RWInfo::kCategoryGeneric   , 4 , { 36, 7 , 0 , 0 , 0 , 0  } }, // #41 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 14, { 35, 3 , 0 , 0 , 0 , 0  } }, // #42 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 14, { 36, 3 , 0 , 0 , 0 , 0  } }, // #43 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 5 , { 35, 9 , 0 , 0 , 0 , 0  } }, // #44 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 5 , { 11, 9 , 0 , 0 , 0 , 0  } }, // #45 [ref=7x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 37, 38, 0 , 0 , 0 , 0  } }, // #46 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 27, 0 , 0 , 0 , 0 , 0  } }, // #47 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 10, { 2 , 0 , 0 , 0 , 0 , 0  } }, // #48 [ref=4x]
  { InstDB::RWInfo::kCategoryGeneric   , 15, { 1 , 39, 0 , 0 , 0 , 0  } }, // #49 [ref=3x]
  { InstDB::RWInfo::kCategoryGeneric   , 6 , { 40, 41, 3 , 0 , 0 , 0  } }, // #50 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 16, { 42, 43, 0 , 0 , 0 , 0  } }, // #51 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 17, { 42, 5 , 0 , 0 , 0 , 0  } }, // #52 [ref=4x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 4 , 5 , 0 , 0 , 0 , 0  } }, // #53 [ref=3x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 4 , 0 , 0 , 0 , 0 , 0  } }, // #54 [ref=3x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 44, 45, 0 , 0 , 0 , 0  } }, // #55 [ref=6x]
  { InstDB::RWInfo::kCategoryGeneric   , 18, { 3 , 0 , 0 , 0 , 0 , 0  } }, // #56 [ref=15x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 44, 0 , 0 , 0 , 0 , 0  } }, // #57 [ref=16x]
  { InstDB::RWInfo::kCategoryGeneric   , 19, { 45, 0 , 0 , 0 , 0 , 0  } }, // #58 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 19, { 46, 0 , 0 , 0 , 0 , 0  } }, // #59 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 20, { 3 , 0 , 0 , 0 , 0 , 0  } }, // #60 [ref=3x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 45, 0 , 0 , 0 , 0 , 0  } }, // #61 [ref=6x]
  { InstDB::RWInfo::kCategoryGeneric   , 18, { 11, 0 , 0 , 0 , 0 , 0  } }, // #62 [ref=3x]
  { InstDB::RWInfo::kCategoryGeneric   , 21, { 13, 0 , 0 , 0 , 0 , 0  } }, // #63 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 8 , { 11, 0 , 0 , 0 , 0 , 0  } }, // #64 [ref=8x]
  { InstDB::RWInfo::kCategoryGeneric   , 21, { 47, 0 , 0 , 0 , 0 , 0  } }, // #65 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 7 , { 48, 0 , 0 , 0 , 0 , 0  } }, // #66 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 20, { 11, 0 , 0 , 0 , 0 , 0  } }, // #67 [ref=2x]
  { InstDB::RWInfo::kCategoryImul      , 2 , { 0 , 0 , 0 , 0 , 0 , 0  } }, // #68 [ref=1x]
  { InstDB::RWInfo::kCategoryImul      , 22, { 0 , 0 , 0 , 0 , 0 , 0  } }, // #69 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 49, 50, 0 , 0 , 0 , 0  } }, // #70 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 51, 50, 0 , 0 , 0 , 0  } }, // #71 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 5 , { 4 , 9 , 0 , 0 , 0 , 0  } }, // #72 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 12, { 3 , 5 , 0 , 0 , 0 , 0  } }, // #73 [ref=3x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 21, 28, 0 , 0 , 0 , 0  } }, // #74 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 52, 0 , 0 , 0 , 0 , 0  } }, // #75 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 53, 39, 39, 0 , 0 , 0  } }, // #76 [ref=6x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 42, 9 , 9 , 0 , 0 , 0  } }, // #77 [ref=6x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 34, 7 , 7 , 0 , 0 , 0  } }, // #78 [ref=6x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 47, 13, 13, 0 , 0 , 0  } }, // #79 [ref=6x]
  { InstDB::RWInfo::kCategoryGeneric   , 23, { 53, 39, 0 , 0 , 0 , 0  } }, // #80 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 24, { 42, 9 , 0 , 0 , 0 , 0  } }, // #81 [ref=4x]
  { InstDB::RWInfo::kCategoryGeneric   , 25, { 34, 7 , 0 , 0 , 0 , 0  } }, // #82 [ref=3x]
  { InstDB::RWInfo::kCategoryGeneric   , 26, { 47, 13, 0 , 0 , 0 , 0  } }, // #83 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 53, 39, 0 , 0 , 0 , 0  } }, // #84 [ref=3x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 42, 9 , 0 , 0 , 0 , 0  } }, // #85 [ref=3x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 34, 7 , 0 , 0 , 0 , 0  } }, // #86 [ref=5x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 47, 13, 0 , 0 , 0 , 0  } }, // #87 [ref=3x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 39, 39, 0 , 0 , 0 , 0  } }, // #88 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 9 , 9 , 0 , 0 , 0 , 0  } }, // #89 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 7 , 7 , 0 , 0 , 0 , 0  } }, // #90 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 13, 13, 0 , 0 , 0 , 0  } }, // #91 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 47, 39, 39, 0 , 0 , 0  } }, // #92 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 34, 9 , 9 , 0 , 0 , 0  } }, // #93 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 42, 13, 13, 0 , 0 , 0  } }, // #94 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 54, 0 , 0 , 0 , 0 , 0  } }, // #95 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 27, { 11, 3 , 0 , 0 , 0 , 0  } }, // #96 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 12, { 10, 5 , 0 , 0 , 0 , 0  } }, // #97 [ref=5x]
  { InstDB::RWInfo::kCategoryGeneric   , 28, { 9 , 0 , 0 , 0 , 0 , 0  } }, // #98 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 2 , 3 , 0 , 0 , 0 , 0  } }, // #99 [ref=13x]
  { InstDB::RWInfo::kCategoryGeneric   , 8 , { 11, 3 , 0 , 0 , 0 , 0  } }, // #100 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 7 , { 13, 0 , 0 , 0 , 0 , 0  } }, // #101 [ref=5x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 3 , 0 , 0 , 0 , 0 , 0  } }, // #102 [ref=3x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 49, 19, 0 , 0 , 0 , 0  } }, // #103 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 55, 0 , 0 , 0 , 0 , 0  } }, // #104 [ref=3x]
  { InstDB::RWInfo::kCategoryGeneric   , 5 , { 3 , 9 , 0 , 0 , 0 , 0  } }, // #105 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 5 , 5 , 20, 0 , 0 , 0  } }, // #106 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 7 , 7 , 20, 0 , 0 , 0  } }, // #107 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 18, 28, 56, 0 , 0 , 0  } }, // #108 [ref=2x]
  { InstDB::RWInfo::kCategoryMov       , 29, { 0 , 0 , 0 , 0 , 0 , 0  } }, // #109 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 30, { 10, 5 , 0 , 0 , 0 , 0  } }, // #110 [ref=6x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 11, 3 , 0 , 0 , 0 , 0  } }, // #111 [ref=14x]
  { InstDB::RWInfo::kCategoryGeneric   , 16, { 11, 43, 0 , 0 , 0 , 0  } }, // #112 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 35, 57, 0 , 0 , 0 , 0  } }, // #113 [ref=1x]
  { InstDB::RWInfo::kCategoryMovh64    , 13, { 0 , 0 , 0 , 0 , 0 , 0  } }, // #114 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 58, 7 , 0 , 0 , 0 , 0  } }, // #115 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 13, { 34, 7 , 0 , 0 , 0 , 0  } }, // #116 [ref=7x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 53, 5 , 0 , 0 , 0 , 0  } }, // #117 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 28, { 42, 9 , 0 , 0 , 0 , 0  } }, // #118 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 20, 19, 0 , 0 , 0 , 0  } }, // #119 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 14, { 11, 3 , 0 , 0 , 0 , 0  } }, // #120 [ref=3x]
  { InstDB::RWInfo::kCategoryGeneric   , 5 , { 34, 9 , 0 , 0 , 0 , 0  } }, // #121 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 6 , { 59, 41, 3 , 0 , 0 , 0  } }, // #122 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 6 , { 11, 11, 3 , 60, 0 , 0  } }, // #123 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 16, 28, 0 , 0 , 0 , 0  } }, // #124 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 16, 28, 29, 0 , 0 , 0  } }, // #125 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 10, { 3 , 0 , 0 , 0 , 0 , 0  } }, // #126 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 50, 21, 0 , 0 , 0 , 0  } }, // #127 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 50, 61, 0 , 0 , 0 , 0  } }, // #128 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 4 , { 25, 7 , 0 , 0 , 0 , 0  } }, // #129 [ref=18x]
  { InstDB::RWInfo::kCategoryGeneric   , 3 , { 5 , 5 , 0 , 62, 16, 56 } }, // #130 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 3 , { 5 , 5 , 0 , 63, 16, 56 } }, // #131 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 3 , { 5 , 5 , 0 , 62, 0 , 0  } }, // #132 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 3 , { 5 , 5 , 0 , 63, 0 , 0  } }, // #133 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 31, { 53, 5 , 0 , 0 , 0 , 0  } }, // #134 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 32, { 34, 5 , 0 , 0 , 0 , 0  } }, // #135 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 33, { 47, 3 , 0 , 0 , 0 , 0  } }, // #136 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 3 , { 64, 5 , 0 , 0 , 0 , 0  } }, // #137 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 15, { 4 , 39, 0 , 0 , 0 , 0  } }, // #138 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 4 , { 4 , 7 , 0 , 0 , 0 , 0  } }, // #139 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 27, { 2 , 13, 0 , 0 , 0 , 0  } }, // #140 [ref=1x]
  { InstDB::RWInfo::kCategoryVmov1_8   , 0 , { 0 , 0 , 0 , 0 , 0 , 0  } }, // #141 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 5 , { 10, 9 , 0 , 0 , 0 , 0  } }, // #142 [ref=4x]
  { InstDB::RWInfo::kCategoryGeneric   , 27, { 10, 13, 0 , 0 , 0 , 0  } }, // #143 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 10, { 65, 0 , 0 , 0 , 0 , 0  } }, // #144 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 3 , { 5 , 5 , 0 , 0 , 0 , 0  } }, // #145 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 10, { 60, 0 , 0 , 0 , 0 , 0  } }, // #146 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 10, { 2 , 66, 0 , 0 , 0 , 0  } }, // #147 [ref=8x]
  { InstDB::RWInfo::kCategoryGeneric   , 5 , { 36, 9 , 0 , 0 , 0 , 0  } }, // #148 [ref=4x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 11, 0 , 0 , 0 , 0 , 0  } }, // #149 [ref=6x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 15, 67, 28, 0 , 0 , 0  } }, // #150 [ref=3x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 15, 67, 0 , 0 , 0 , 0  } }, // #151 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 15, 67, 62, 0 , 0 , 0  } }, // #152 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 68, 0 , 0 , 0 , 0 , 0  } }, // #153 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 21, 20, 0 , 0 , 0 , 0  } }, // #154 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 31, { 69, 0 , 0 , 0 , 0 , 0  } }, // #155 [ref=30x]
  { InstDB::RWInfo::kCategoryGeneric   , 11, { 2 , 3 , 66, 0 , 0 , 0  } }, // #156 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 34, { 11, 0 , 0 , 0 , 0 , 0  } }, // #157 [ref=3x]
  { InstDB::RWInfo::kCategoryGeneric   , 28, { 42, 0 , 0 , 0 , 0 , 0  } }, // #158 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 20, 21, 0 , 0 , 0 , 0  } }, // #159 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 12, { 70, 43, 43, 43, 43, 5  } }, // #160 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 12, { 4 , 5 , 5 , 5 , 5 , 5  } }, // #161 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 35, { 10, 5 , 7 , 0 , 0 , 0  } }, // #162 [ref=8x]
  { InstDB::RWInfo::kCategoryGeneric   , 36, { 10, 5 , 9 , 0 , 0 , 0  } }, // #163 [ref=9x]
  { InstDB::RWInfo::kCategoryGeneric   , 6 , { 11, 3 , 3 , 3 , 0 , 0  } }, // #164 [ref=3x]
  { InstDB::RWInfo::kCategoryGeneric   , 12, { 71, 5 , 0 , 0 , 0 , 0  } }, // #165 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 12, { 11, 5 , 0 , 0 , 0 , 0  } }, // #166 [ref=4x]
  { InstDB::RWInfo::kCategoryGeneric   , 37, { 72, 73, 0 , 0 , 0 , 0  } }, // #167 [ref=4x]
  { InstDB::RWInfo::kCategoryGeneric   , 38, { 11, 7 , 0 , 0 , 0 , 0  } }, // #168 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 39, { 11, 9 , 0 , 0 , 0 , 0  } }, // #169 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 35, { 11, 5 , 7 , 0 , 0 , 0  } }, // #170 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 36, { 11, 5 , 9 , 0 , 0 , 0  } }, // #171 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 11, { 11, 3 , 0 , 0 , 0 , 0  } }, // #172 [ref=7x]
  { InstDB::RWInfo::kCategoryVmov2_1   , 40, { 0 , 0 , 0 , 0 , 0 , 0  } }, // #173 [ref=14x]
  { InstDB::RWInfo::kCategoryVmov1_2   , 14, { 0 , 0 , 0 , 0 , 0 , 0  } }, // #174 [ref=7x]
  { InstDB::RWInfo::kCategoryVmov1_2   , 41, { 0 , 0 , 0 , 0 , 0 , 0  } }, // #175 [ref=10x]
  { InstDB::RWInfo::kCategoryGeneric   , 35, { 10, 74, 7 , 0 , 0 , 0  } }, // #176 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 42, { 10, 57, 3 , 0 , 0 , 0  } }, // #177 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 42, { 10, 74, 3 , 0 , 0 , 0  } }, // #178 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 36, { 10, 57, 9 , 0 , 0 , 0  } }, // #179 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 43, { 10, 5 , 5 , 0 , 0 , 0  } }, // #180 [ref=9x]
  { InstDB::RWInfo::kCategoryGeneric   , 44, { 72, 43, 0 , 0 , 0 , 0  } }, // #181 [ref=6x]
  { InstDB::RWInfo::kCategoryGeneric   , 45, { 10, 73, 0 , 0 , 0 , 0  } }, // #182 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 45, { 10, 3 , 0 , 0 , 0 , 0  } }, // #183 [ref=4x]
  { InstDB::RWInfo::kCategoryGeneric   , 46, { 71, 43, 0 , 0 , 0 , 0  } }, // #184 [ref=4x]
  { InstDB::RWInfo::kCategoryGeneric   , 6 , { 2 , 3 , 3 , 0 , 0 , 0  } }, // #185 [ref=60x]
  { InstDB::RWInfo::kCategoryGeneric   , 35, { 4 , 57, 7 , 0 , 0 , 0  } }, // #186 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 36, { 4 , 74, 9 , 0 , 0 , 0  } }, // #187 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 35, { 6 , 7 , 7 , 0 , 0 , 0  } }, // #188 [ref=11x]
  { InstDB::RWInfo::kCategoryGeneric   , 36, { 8 , 9 , 9 , 0 , 0 , 0  } }, // #189 [ref=11x]
  { InstDB::RWInfo::kCategoryGeneric   , 47, { 11, 3 , 3 , 3 , 0 , 0  } }, // #190 [ref=15x]
  { InstDB::RWInfo::kCategoryGeneric   , 48, { 34, 7 , 7 , 7 , 0 , 0  } }, // #191 [ref=4x]
  { InstDB::RWInfo::kCategoryGeneric   , 49, { 42, 9 , 9 , 9 , 0 , 0  } }, // #192 [ref=4x]
  { InstDB::RWInfo::kCategoryGeneric   , 35, { 25, 7 , 7 , 0 , 0 , 0  } }, // #193 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 36, { 75, 9 , 9 , 0 , 0 , 0  } }, // #194 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 14, { 34, 3 , 0 , 0 , 0 , 0  } }, // #195 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 5 , { 42, 9 , 0 , 0 , 0 , 0  } }, // #196 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 8 , { 2 , 3 , 2 , 0 , 0 , 0  } }, // #197 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 2 , 3 , 2 , 0 , 0 , 0  } }, // #198 [ref=4x]
  { InstDB::RWInfo::kCategoryGeneric   , 18, { 4 , 3 , 4 , 0 , 0 , 0  } }, // #199 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 35, { 10, 57, 7 , 0 , 0 , 0  } }, // #200 [ref=11x]
  { InstDB::RWInfo::kCategoryGeneric   , 36, { 10, 74, 9 , 0 , 0 , 0  } }, // #201 [ref=13x]
  { InstDB::RWInfo::kCategoryGeneric   , 43, { 71, 73, 5 , 0 , 0 , 0  } }, // #202 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 43, { 11, 3 , 5 , 0 , 0 , 0  } }, // #203 [ref=4x]
  { InstDB::RWInfo::kCategoryGeneric   , 50, { 72, 43, 73, 0 , 0 , 0  } }, // #204 [ref=4x]
  { InstDB::RWInfo::kCategoryVmaskmov  , 0 , { 0 , 0 , 0 , 0 , 0 , 0  } }, // #205 [ref=4x]
  { InstDB::RWInfo::kCategoryGeneric   , 13, { 34, 0 , 0 , 0 , 0 , 0  } }, // #206 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 21, 0 , 0 , 0 , 0 , 0  } }, // #207 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 51, { 11, 3 , 0 , 0 , 0 , 0  } }, // #208 [ref=12x]
  { InstDB::RWInfo::kCategoryVmovddup  , 52, { 0 , 0 , 0 , 0 , 0 , 0  } }, // #209 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 10, 57, 57, 0 , 0 , 0  } }, // #210 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 13, { 34, 57, 0 , 0 , 0 , 0  } }, // #211 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 13, { 10, 7 , 7 , 0 , 0 , 0  } }, // #212 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 10, 7 , 7 , 0 , 0 , 0  } }, // #213 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 13, { 10, 57, 7 , 0 , 0 , 0  } }, // #214 [ref=2x]
  { InstDB::RWInfo::kCategoryVmovmskpd , 0 , { 0 , 0 , 0 , 0 , 0 , 0  } }, // #215 [ref=1x]
  { InstDB::RWInfo::kCategoryVmovmskps , 0 , { 0 , 0 , 0 , 0 , 0 , 0  } }, // #216 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 53, { 34, 7 , 0 , 0 , 0 , 0  } }, // #217 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 10, 57, 7 , 0 , 0 , 0  } }, // #218 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 10, 74, 9 , 0 , 0 , 0  } }, // #219 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 13, { 7 , 0 , 0 , 0 , 0 , 0  } }, // #220 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 76, 0 , 0 , 0 , 0 , 0  } }, // #221 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 2 , { 3 , 3 , 0 , 0 , 0 , 0  } }, // #222 [ref=4x]
  { InstDB::RWInfo::kCategoryGeneric   , 12, { 72, 43, 43, 43, 43, 5  } }, // #223 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 15, { 11, 39, 0 , 0 , 0 , 0  } }, // #224 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 11, 7 , 0 , 0 , 0 , 0  } }, // #225 [ref=6x]
  { InstDB::RWInfo::kCategoryGeneric   , 27, { 11, 13, 0 , 0 , 0 , 0  } }, // #226 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 6 , { 34, 3 , 3 , 0 , 0 , 0  } }, // #227 [ref=17x]
  { InstDB::RWInfo::kCategoryGeneric   , 50, { 71, 73, 73, 0 , 0 , 0  } }, // #228 [ref=3x]
  { InstDB::RWInfo::kCategoryGeneric   , 22, { 11, 3 , 3 , 0 , 0 , 0  } }, // #229 [ref=3x]
  { InstDB::RWInfo::kCategoryGeneric   , 54, { 71, 73, 0 , 0 , 0 , 0  } }, // #230 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 7 , { 47, 5 , 0 , 0 , 0 , 0  } }, // #231 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 55, { 10, 5 , 39, 0 , 0 , 0  } }, // #232 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 56, { 10, 5 , 13, 0 , 0 , 0  } }, // #233 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 43, { 10, 5 , 5 , 5 , 0 , 0  } }, // #234 [ref=12x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 34, 3 , 0 , 0 , 0 , 0  } }, // #235 [ref=4x]
  { InstDB::RWInfo::kCategoryVmov1_4   , 57, { 0 , 0 , 0 , 0 , 0 , 0  } }, // #236 [ref=6x]
  { InstDB::RWInfo::kCategoryVmov1_8   , 58, { 0 , 0 , 0 , 0 , 0 , 0  } }, // #237 [ref=3x]
  { InstDB::RWInfo::kCategoryVmov4_1   , 59, { 0 , 0 , 0 , 0 , 0 , 0  } }, // #238 [ref=4x]
  { InstDB::RWInfo::kCategoryVmov8_1   , 60, { 0 , 0 , 0 , 0 , 0 , 0  } }, // #239 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 61, { 10, 5 , 5 , 5 , 0 , 0  } }, // #240 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 62, { 10, 5 , 5 , 0 , 0 , 0  } }, // #241 [ref=12x]
  { InstDB::RWInfo::kCategoryGeneric   , 18, { 11, 3 , 0 , 0 , 0 , 0  } }, // #242 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 22, { 11, 3 , 5 , 0 , 0 , 0  } }, // #243 [ref=9x]
  { InstDB::RWInfo::kCategoryGeneric   , 63, { 11, 3 , 0 , 0 , 0 , 0  } }, // #244 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 56, 16, 28, 0 , 0 , 0  } }, // #245 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 11, { 2 , 2 , 0 , 0 , 0 , 0  } }, // #246 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 51, { 2 , 2 , 0 , 0 , 0 , 0  } }, // #247 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 8 , { 3 , 56, 16, 0 , 0 , 0  } }, // #248 [ref=4x]
  { InstDB::RWInfo::kCategoryGeneric   , 8 , { 11, 56, 16, 0 , 0 , 0  } }  // #249 [ref=8x]
};

const InstDB::RWInfoOp InstDB::rwInfoOp[] = {
  { 0x0000000000000000u, 0x0000000000000000u, 0xFF, { 0 }, 0 }, // #0 [ref=14958x]
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
  { 0x0000000000000000u, 0x000000000000000Fu, 0xFF, { 0 }, OpRWInfo::kWrite | OpRWInfo::kZExt }, // #42 [ref=29x]
  { 0xFFFFFFFFFFFFFFFFu, 0x0000000000000000u, 0xFF, { 0 }, OpRWInfo::kRead }, // #43 [ref=33x]
  { 0x00000000000003FFu, 0x00000000000003FFu, 0xFF, { 0 }, OpRWInfo::kRW | OpRWInfo::kZExt }, // #44 [ref=22x]
  { 0x00000000000003FFu, 0x0000000000000000u, 0xFF, { 0 }, OpRWInfo::kRead }, // #45 [ref=13x]
  { 0x0000000000000000u, 0x00000000000003FFu, 0xFF, { 0 }, OpRWInfo::kWrite | OpRWInfo::kZExt }, // #46 [ref=1x]
  { 0x0000000000000000u, 0x0000000000000003u, 0xFF, { 0 }, OpRWInfo::kWrite | OpRWInfo::kZExt }, // #47 [ref=15x]
  { 0x0000000000000000u, 0x0000000000000003u, 0x00, { 0 }, OpRWInfo::kWrite | OpRWInfo::kRegPhysId | OpRWInfo::kZExt }, // #48 [ref=2x]
  { 0x0000000000000000u, 0x0000000000000000u, 0x00, { 0 }, OpRWInfo::kWrite | OpRWInfo::kRegPhysId | OpRWInfo::kZExt }, // #49 [ref=2x]
  { 0x0000000000000003u, 0x0000000000000000u, 0x02, { 0 }, OpRWInfo::kRead | OpRWInfo::kRegPhysId }, // #50 [ref=4x]
  { 0x0000000000000000u, 0x0000000000000000u, 0x07, { 0 }, OpRWInfo::kWrite | OpRWInfo::kZExt | OpRWInfo::kMemPhysId }, // #51 [ref=1x]
  { 0x0000000000000000u, 0x0000000000000000u, 0x01, { 0 }, OpRWInfo::kRead | OpRWInfo::kRegPhysId }, // #52 [ref=1x]
  { 0x0000000000000000u, 0x0000000000000001u, 0xFF, { 0 }, OpRWInfo::kWrite | OpRWInfo::kZExt }, // #53 [ref=14x]
  { 0x0000000000000000u, 0x0000000000000001u, 0x00, { 0 }, OpRWInfo::kWrite | OpRWInfo::kRegPhysId }, // #54 [ref=1x]
  { 0x0000000000000000u, 0x0000000000000000u, 0x01, { 0 }, OpRWInfo::kRW | OpRWInfo::kRegPhysId | OpRWInfo::kZExt }, // #55 [ref=3x]
  { 0x000000000000000Fu, 0x0000000000000000u, 0x02, { 0 }, OpRWInfo::kRead | OpRWInfo::kRegPhysId }, // #56 [ref=20x]
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

const InstDB::RWInfoRm InstDB::rwInfoRm[] = {
  { InstDB::RWInfoRm::kCategoryNone      , 0x00, 0 , 0, 0 }, // #0 [ref=1809x]
  { InstDB::RWInfoRm::kCategoryConsistent, 0x03, 0 , InstDB::RWInfoRm::kFlagAmbiguous, 0 }, // #1 [ref=8x]
  { InstDB::RWInfoRm::kCategoryConsistent, 0x02, 0 , 0, 0 }, // #2 [ref=193x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x02, 16, 0, 0 }, // #3 [ref=122x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x02, 8 , 0, 0 }, // #4 [ref=66x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x02, 4 , 0, 0 }, // #5 [ref=34x]
  { InstDB::RWInfoRm::kCategoryConsistent, 0x04, 0 , 0, 0 }, // #6 [ref=269x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x01, 2 , 0, 0 }, // #7 [ref=9x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x00, 0 , 0, 0 }, // #8 [ref=60x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x03, 0 , 0, 0 }, // #9 [ref=1x]
  { InstDB::RWInfoRm::kCategoryConsistent, 0x01, 0 , InstDB::RWInfoRm::kFlagAmbiguous, 0 }, // #10 [ref=20x]
  { InstDB::RWInfoRm::kCategoryConsistent, 0x01, 0 , 0, 0 }, // #11 [ref=13x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x00, 16, 0, 0 }, // #12 [ref=21x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x00, 8 , 0, 0 }, // #13 [ref=20x]
  { InstDB::RWInfoRm::kCategoryConsistent, 0x02, 0 , InstDB::RWInfoRm::kFlagAmbiguous, 0 }, // #14 [ref=15x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x02, 1 , 0, 0 }, // #15 [ref=5x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x00, 64, 0, 0 }, // #16 [ref=3x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x01, 4 , 0, 0 }, // #17 [ref=4x]
  { InstDB::RWInfoRm::kCategoryNone      , 0x00, 0 , InstDB::RWInfoRm::kFlagAmbiguous, 0 }, // #18 [ref=22x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x00, 10, 0, 0 }, // #19 [ref=2x]
  { InstDB::RWInfoRm::kCategoryNone      , 0x01, 0 , InstDB::RWInfoRm::kFlagAmbiguous, 0 }, // #20 [ref=5x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x00, 2 , 0, 0 }, // #21 [ref=3x]
  { InstDB::RWInfoRm::kCategoryConsistent, 0x06, 0 , 0, 0 }, // #22 [ref=13x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x03, 1 , 0, 0 }, // #23 [ref=1x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x03, 4 , 0, 0 }, // #24 [ref=4x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x03, 8 , 0, 0 }, // #25 [ref=3x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x03, 2 , 0, 0 }, // #26 [ref=1x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x02, 2 , 0, 0 }, // #27 [ref=6x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x00, 4 , 0, 0 }, // #28 [ref=6x]
  { InstDB::RWInfoRm::kCategoryNone      , 0x03, 0 , InstDB::RWInfoRm::kFlagAmbiguous, 0 }, // #29 [ref=1x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x03, 16, 0, 0 }, // #30 [ref=6x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x01, 1 , 0, 0 }, // #31 [ref=32x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x01, 8 , 0, 0 }, // #32 [ref=2x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x01, 2 , 0, Features::kSSE4_1 }, // #33 [ref=1x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x01, 2 , InstDB::RWInfoRm::kFlagAmbiguous, 0 }, // #34 [ref=3x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x04, 8 , 0, 0 }, // #35 [ref=34x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x04, 4 , 0, 0 }, // #36 [ref=37x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x00, 32, 0, 0 }, // #37 [ref=4x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x02, 8 , InstDB::RWInfoRm::kFlagAmbiguous, 0 }, // #38 [ref=1x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x02, 4 , InstDB::RWInfoRm::kFlagAmbiguous, 0 }, // #39 [ref=1x]
  { InstDB::RWInfoRm::kCategoryHalf      , 0x02, 0 , 0, 0 }, // #40 [ref=14x]
  { InstDB::RWInfoRm::kCategoryHalf      , 0x01, 0 , 0, 0 }, // #41 [ref=10x]
  { InstDB::RWInfoRm::kCategoryConsistent, 0x04, 0 , InstDB::RWInfoRm::kFlagAmbiguous, 0 }, // #42 [ref=4x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x04, 16, 0, 0 }, // #43 [ref=27x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x02, 64, 0, 0 }, // #44 [ref=6x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x01, 16, 0, 0 }, // #45 [ref=6x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x01, 32, 0, 0 }, // #46 [ref=4x]
  { InstDB::RWInfoRm::kCategoryConsistent, 0x0C, 0 , 0, 0 }, // #47 [ref=15x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x0C, 8 , 0, 0 }, // #48 [ref=4x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x0C, 4 , 0, 0 }, // #49 [ref=4x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x04, 32, 0, 0 }, // #50 [ref=7x]
  { InstDB::RWInfoRm::kCategoryConsistent, 0x03, 0 , 0, 0 }, // #51 [ref=13x]
  { InstDB::RWInfoRm::kCategoryNone      , 0x02, 0 , 0, 0 }, // #52 [ref=1x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x03, 8 , InstDB::RWInfoRm::kFlagAmbiguous, 0 }, // #53 [ref=1x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x02, 32, 0, 0 }, // #54 [ref=1x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x04, 1 , 0, 0 }, // #55 [ref=1x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x04, 2 , 0, 0 }, // #56 [ref=1x]
  { InstDB::RWInfoRm::kCategoryQuarter   , 0x01, 0 , 0, 0 }, // #57 [ref=6x]
  { InstDB::RWInfoRm::kCategoryEighth    , 0x01, 0 , 0, 0 }, // #58 [ref=3x]
  { InstDB::RWInfoRm::kCategoryQuarter   , 0x02, 0 , 0, 0 }, // #59 [ref=4x]
  { InstDB::RWInfoRm::kCategoryEighth    , 0x02, 0 , 0, 0 }, // #60 [ref=2x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x0C, 16, 0, 0 }, // #61 [ref=1x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x06, 16, 0, 0 }, // #62 [ref=12x]
  { InstDB::RWInfoRm::kCategoryConsistent, 0x02, 0 , 0, Features::kAVX512_BW }  // #63 [ref=2x]
};
// ----------------------------------------------------------------------------
// ${InstRWInfoTable:End}

// ============================================================================
// [asmjit::x86::InstDB - Unit]
// ============================================================================

#if defined(ASMJIT_TEST)
UNIT(asmjit_x86_inst_db) {
  INFO("Checking validity of Inst enums");

  // Cross-validate prefixes.
  EXPECT(Inst::kOptionRex  == 0x40000000u, "REX prefix must be at 0x40000000");
  EXPECT(Inst::kOptionVex3 == 0x00000400u, "VEX3 prefix must be at 0x00000400");
  EXPECT(Inst::kOptionEvex == 0x00001000u, "EVEX prefix must be at 0x00001000");

  // These could be combined together to form a valid REX prefix, they must match.
  EXPECT(uint32_t(Inst::kOptionOpCodeB) == uint32_t(Opcode::kB), "Opcode::kB must match Inst::kOptionOpCodeB");
  EXPECT(uint32_t(Inst::kOptionOpCodeX) == uint32_t(Opcode::kX), "Opcode::kX must match Inst::kOptionOpCodeX");
  EXPECT(uint32_t(Inst::kOptionOpCodeR) == uint32_t(Opcode::kR), "Opcode::kR must match Inst::kOptionOpCodeR");
  EXPECT(uint32_t(Inst::kOptionOpCodeW) == uint32_t(Opcode::kW), "Opcode::kW must match Inst::kOptionOpCodeW");

  uint32_t rex_rb = (Opcode::kR >> Opcode::kREX_Shift) | (Opcode::kB >> Opcode::kREX_Shift) | 0x40;
  uint32_t rex_rw = (Opcode::kR >> Opcode::kREX_Shift) | (Opcode::kW >> Opcode::kREX_Shift) | 0x40;

  EXPECT(rex_rb == 0x45, "Opcode::kR|B must form a valid REX prefix (0x45) if combined with 0x40");
  EXPECT(rex_rw == 0x4C, "Opcode::kR|W must form a valid REX prefix (0x4C) if combined with 0x40");
}
#endif

ASMJIT_END_SUB_NAMESPACE

#endif // ASMJIT_BUILD_X86
