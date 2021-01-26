// AsmJit - Machine code generation for C++
//
//  * Official AsmJit Home Page: https://asmjit.com
//  * Official Github Repository: https://github.com/asmjit/asmjit
//
// Copyright (c) 2008-2020 The AsmJit Authors
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

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

#include "../core/api-build_p.h"
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
//   - `E` encodes EVEX instructions only.
#define O_ENCODE(VEX, PREFIX, OPCODE, O, L, W, EvexW, N, TT) \
  ((PREFIX) | (OPCODE) | (O) | (L) | (W) | (EvexW) | (N) | (TT) | \
   (VEX && ((PREFIX) & Opcode::kMM_Mask) != Opcode::kMM_0F ? int(Opcode::kMM_ForceVex3) : 0))

#define O(PREFIX, OPCODE, ModO, LL, W, EvexW, N, ModRM) (O_ENCODE(0, Opcode::k##PREFIX, 0x##OPCODE, Opcode::kModO_##ModO, Opcode::kLL_##LL, Opcode::kW_##W, Opcode::kEvex_W_##EvexW, Opcode::kCDSHL_##N, Opcode::kModRM_##ModRM))
#define V(PREFIX, OPCODE, ModO, LL, W, EvexW, N, TT) (O_ENCODE(1, Opcode::k##PREFIX, 0x##OPCODE, Opcode::kModO_##ModO, Opcode::kLL_##LL, Opcode::kW_##W, Opcode::kEvex_W_##EvexW, Opcode::kCDSHL_##N, Opcode::kCDTT_##TT))
#define E(PREFIX, OPCODE, ModO, LL, W, EvexW, N, TT) (O_ENCODE(1, Opcode::k##PREFIX, 0x##OPCODE, Opcode::kModO_##ModO, Opcode::kLL_##LL, Opcode::kW_##W, Opcode::kEvex_W_##EvexW, Opcode::kCDSHL_##N, Opcode::kCDTT_##TT) | Opcode::kMM_ForceEvex)
#define O_FPU(PREFIX, OPCODE, ModO) (Opcode::kFPU_##PREFIX | (0x##OPCODE & 0xFFu) | ((0x##OPCODE >> 8) << Opcode::kFPU_2B_Shift) | Opcode::kModO_##ModO)

// Don't store `_nameDataIndex` if instruction names are disabled. Since some
// APIs can use `_nameDataIndex` it's much safer if it's zero if it's not defined.
#ifndef ASMJIT_NO_TEXT
  #define NAME_DATA_INDEX(Index) Index
#else
  #define NAME_DATA_INDEX(Index) 0
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
  INST(Add              , X86Arith           , O(000000,00,0,_,x,_,_,_  ), 0                         , 0  , 0  , 3119 , 3  , 1  ), // #7
  INST(Addpd            , ExtRm              , O(660F00,58,_,_,_,_,_,_  ), 0                         , 3  , 0  , 5109 , 5  , 4  ), // #8
  INST(Addps            , ExtRm              , O(000F00,58,_,_,_,_,_,_  ), 0                         , 4  , 0  , 5121 , 5  , 5  ), // #9
  INST(Addsd            , ExtRm              , O(F20F00,58,_,_,_,_,_,_  ), 0                         , 5  , 0  , 5343 , 6  , 4  ), // #10
  INST(Addss            , ExtRm              , O(F30F00,58,_,_,_,_,_,_  ), 0                         , 6  , 0  , 3250 , 7  , 5  ), // #11
  INST(Addsubpd         , ExtRm              , O(660F00,D0,_,_,_,_,_,_  ), 0                         , 3  , 0  , 4848 , 5  , 6  ), // #12
  INST(Addsubps         , ExtRm              , O(F20F00,D0,_,_,_,_,_,_  ), 0                         , 5  , 0  , 4860 , 5  , 6  ), // #13
  INST(Adox             , X86Rm              , O(F30F38,F6,_,_,x,_,_,_  ), 0                         , 7  , 0  , 26   , 4  , 7  ), // #14
  INST(Aesdec           , ExtRm              , O(660F38,DE,_,_,_,_,_,_  ), 0                         , 2  , 0  , 3305 , 5  , 8  ), // #15
  INST(Aesdeclast       , ExtRm              , O(660F38,DF,_,_,_,_,_,_  ), 0                         , 2  , 0  , 3313 , 5  , 8  ), // #16
  INST(Aesenc           , ExtRm              , O(660F38,DC,_,_,_,_,_,_  ), 0                         , 2  , 0  , 3325 , 5  , 8  ), // #17
  INST(Aesenclast       , ExtRm              , O(660F38,DD,_,_,_,_,_,_  ), 0                         , 2  , 0  , 3333 , 5  , 8  ), // #18
  INST(Aesimc           , ExtRm              , O(660F38,DB,_,_,_,_,_,_  ), 0                         , 2  , 0  , 3345 , 5  , 8  ), // #19
  INST(Aeskeygenassist  , ExtRmi             , O(660F3A,DF,_,_,_,_,_,_  ), 0                         , 8  , 0  , 3353 , 8  , 8  ), // #20
  INST(And              , X86Arith           , O(000000,20,4,_,x,_,_,_  ), 0                         , 9  , 0  , 2517 , 9  , 1  ), // #21
  INST(Andn             , VexRvm_Wx          , V(000F38,F2,_,0,x,_,_,_  ), 0                         , 10 , 0  , 6817 , 10 , 9  ), // #22
  INST(Andnpd           , ExtRm              , O(660F00,55,_,_,_,_,_,_  ), 0                         , 3  , 0  , 3386 , 5  , 4  ), // #23
  INST(Andnps           , ExtRm              , O(000F00,55,_,_,_,_,_,_  ), 0                         , 4  , 0  , 3394 , 5  , 5  ), // #24
  INST(Andpd            , ExtRm              , O(660F00,54,_,_,_,_,_,_  ), 0                         , 3  , 0  , 4362 , 11 , 4  ), // #25
  INST(Andps            , ExtRm              , O(000F00,54,_,_,_,_,_,_  ), 0                         , 4  , 0  , 4372 , 11 , 5  ), // #26
  INST(Arpl             , X86Mr_NoSize       , O(000000,63,_,_,_,_,_,_  ), 0                         , 0  , 0  , 31   , 12 , 10 ), // #27
  INST(Bextr            , VexRmv_Wx          , V(000F38,F7,_,0,x,_,_,_  ), 0                         , 10 , 0  , 36   , 13 , 9  ), // #28
  INST(Blcfill          , VexVm_Wx           , V(XOP_M9,01,1,0,x,_,_,_  ), 0                         , 11 , 0  , 42   , 14 , 11 ), // #29
  INST(Blci             , VexVm_Wx           , V(XOP_M9,02,6,0,x,_,_,_  ), 0                         , 12 , 0  , 50   , 14 , 11 ), // #30
  INST(Blcic            , VexVm_Wx           , V(XOP_M9,01,5,0,x,_,_,_  ), 0                         , 13 , 0  , 55   , 14 , 11 ), // #31
  INST(Blcmsk           , VexVm_Wx           , V(XOP_M9,02,1,0,x,_,_,_  ), 0                         , 11 , 0  , 61   , 14 , 11 ), // #32
  INST(Blcs             , VexVm_Wx           , V(XOP_M9,01,3,0,x,_,_,_  ), 0                         , 14 , 0  , 68   , 14 , 11 ), // #33
  INST(Blendpd          , ExtRmi             , O(660F3A,0D,_,_,_,_,_,_  ), 0                         , 8  , 0  , 3472 , 8  , 12 ), // #34
  INST(Blendps          , ExtRmi             , O(660F3A,0C,_,_,_,_,_,_  ), 0                         , 8  , 0  , 3481 , 8  , 12 ), // #35
  INST(Blendvpd         , ExtRm_XMM0         , O(660F38,15,_,_,_,_,_,_  ), 0                         , 2  , 0  , 3490 , 15 , 12 ), // #36
  INST(Blendvps         , ExtRm_XMM0         , O(660F38,14,_,_,_,_,_,_  ), 0                         , 2  , 0  , 3500 , 15 , 12 ), // #37
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
  INST(Btc              , X86Bt              , O(000F00,BB,_,_,x,_,_,_  ), O(000F00,BA,7,_,x,_,_,_  ), 4  , 3  , 172  , 25 , 14 ), // #55
  INST(Btr              , X86Bt              , O(000F00,B3,_,_,x,_,_,_  ), O(000F00,BA,6,_,x,_,_,_  ), 4  , 4  , 176  , 25 , 14 ), // #56
  INST(Bts              , X86Bt              , O(000F00,AB,_,_,x,_,_,_  ), O(000F00,BA,5,_,x,_,_,_  ), 4  , 5  , 180  , 25 , 14 ), // #57
  INST(Bzhi             , VexRmv_Wx          , V(000F38,F5,_,0,x,_,_,_  ), 0                         , 10 , 0  , 184  , 13 , 15 ), // #58
  INST(Call             , X86Call            , O(000000,FF,2,_,_,_,_,_  ), 0                         , 1  , 0  , 3016 , 26 , 1  ), // #59
  INST(Cbw              , X86Op_xAX          , O(660000,98,_,_,_,_,_,_  ), 0                         , 19 , 0  , 189  , 27 , 0  ), // #60
  INST(Cdq              , X86Op_xDX_xAX      , O(000000,99,_,_,_,_,_,_  ), 0                         , 0  , 0  , 193  , 28 , 0  ), // #61
  INST(Cdqe             , X86Op_xAX          , O(000000,98,_,_,1,_,_,_  ), 0                         , 20 , 0  , 197  , 29 , 0  ), // #62
  INST(Clac             , X86Op              , O(000F01,CA,_,_,_,_,_,_  ), 0                         , 21 , 0  , 202  , 30 , 16 ), // #63
  INST(Clc              , X86Op              , O(000000,F8,_,_,_,_,_,_  ), 0                         , 0  , 0  , 207  , 30 , 17 ), // #64
  INST(Cld              , X86Op              , O(000000,FC,_,_,_,_,_,_  ), 0                         , 0  , 0  , 211  , 30 , 18 ), // #65
  INST(Cldemote         , X86M_Only          , O(000F00,1C,0,_,_,_,_,_  ), 0                         , 4  , 0  , 215  , 31 , 19 ), // #66
  INST(Clflush          , X86M_Only          , O(000F00,AE,7,_,_,_,_,_  ), 0                         , 22 , 0  , 224  , 31 , 20 ), // #67
  INST(Clflushopt       , X86M_Only          , O(660F00,AE,7,_,_,_,_,_  ), 0                         , 23 , 0  , 232  , 31 , 21 ), // #68
  INST(Clgi             , X86Op              , O(000F01,DD,_,_,_,_,_,_  ), 0                         , 21 , 0  , 243  , 30 , 22 ), // #69
  INST(Cli              , X86Op              , O(000000,FA,_,_,_,_,_,_  ), 0                         , 0  , 0  , 248  , 30 , 23 ), // #70
  INST(Clrssbsy         , X86M               , O(F30F00,AE,6,_,_,_,_,_  ), 0                         , 24 , 0  , 252  , 32 , 24 ), // #71
  INST(Clts             , X86Op              , O(000F00,06,_,_,_,_,_,_  ), 0                         , 4  , 0  , 261  , 30 , 0  ), // #72
  INST(Clwb             , X86M_Only          , O(660F00,AE,6,_,_,_,_,_  ), 0                         , 25 , 0  , 266  , 31 , 25 ), // #73
  INST(Clzero           , X86Op_MemZAX       , O(000F01,FC,_,_,_,_,_,_  ), 0                         , 21 , 0  , 271  , 33 , 26 ), // #74
  INST(Cmc              , X86Op              , O(000000,F5,_,_,_,_,_,_  ), 0                         , 0  , 0  , 278  , 30 , 27 ), // #75
  INST(Cmova            , X86Rm              , O(000F00,47,_,_,x,_,_,_  ), 0                         , 4  , 0  , 282  , 22 , 28 ), // #76
  INST(Cmovae           , X86Rm              , O(000F00,43,_,_,x,_,_,_  ), 0                         , 4  , 0  , 288  , 22 , 29 ), // #77
  INST(Cmovb            , X86Rm              , O(000F00,42,_,_,x,_,_,_  ), 0                         , 4  , 0  , 643  , 22 , 29 ), // #78
  INST(Cmovbe           , X86Rm              , O(000F00,46,_,_,x,_,_,_  ), 0                         , 4  , 0  , 650  , 22 , 28 ), // #79
  INST(Cmovc            , X86Rm              , O(000F00,42,_,_,x,_,_,_  ), 0                         , 4  , 0  , 295  , 22 , 29 ), // #80
  INST(Cmove            , X86Rm              , O(000F00,44,_,_,x,_,_,_  ), 0                         , 4  , 0  , 658  , 22 , 30 ), // #81
  INST(Cmovg            , X86Rm              , O(000F00,4F,_,_,x,_,_,_  ), 0                         , 4  , 0  , 301  , 22 , 31 ), // #82
  INST(Cmovge           , X86Rm              , O(000F00,4D,_,_,x,_,_,_  ), 0                         , 4  , 0  , 307  , 22 , 32 ), // #83
  INST(Cmovl            , X86Rm              , O(000F00,4C,_,_,x,_,_,_  ), 0                         , 4  , 0  , 314  , 22 , 32 ), // #84
  INST(Cmovle           , X86Rm              , O(000F00,4E,_,_,x,_,_,_  ), 0                         , 4  , 0  , 320  , 22 , 31 ), // #85
  INST(Cmovna           , X86Rm              , O(000F00,46,_,_,x,_,_,_  ), 0                         , 4  , 0  , 327  , 22 , 28 ), // #86
  INST(Cmovnae          , X86Rm              , O(000F00,42,_,_,x,_,_,_  ), 0                         , 4  , 0  , 334  , 22 , 29 ), // #87
  INST(Cmovnb           , X86Rm              , O(000F00,43,_,_,x,_,_,_  ), 0                         , 4  , 0  , 665  , 22 , 29 ), // #88
  INST(Cmovnbe          , X86Rm              , O(000F00,47,_,_,x,_,_,_  ), 0                         , 4  , 0  , 673  , 22 , 28 ), // #89
  INST(Cmovnc           , X86Rm              , O(000F00,43,_,_,x,_,_,_  ), 0                         , 4  , 0  , 342  , 22 , 29 ), // #90
  INST(Cmovne           , X86Rm              , O(000F00,45,_,_,x,_,_,_  ), 0                         , 4  , 0  , 682  , 22 , 30 ), // #91
  INST(Cmovng           , X86Rm              , O(000F00,4E,_,_,x,_,_,_  ), 0                         , 4  , 0  , 349  , 22 , 31 ), // #92
  INST(Cmovnge          , X86Rm              , O(000F00,4C,_,_,x,_,_,_  ), 0                         , 4  , 0  , 356  , 22 , 32 ), // #93
  INST(Cmovnl           , X86Rm              , O(000F00,4D,_,_,x,_,_,_  ), 0                         , 4  , 0  , 364  , 22 , 32 ), // #94
  INST(Cmovnle          , X86Rm              , O(000F00,4F,_,_,x,_,_,_  ), 0                         , 4  , 0  , 371  , 22 , 31 ), // #95
  INST(Cmovno           , X86Rm              , O(000F00,41,_,_,x,_,_,_  ), 0                         , 4  , 0  , 379  , 22 , 33 ), // #96
  INST(Cmovnp           , X86Rm              , O(000F00,4B,_,_,x,_,_,_  ), 0                         , 4  , 0  , 386  , 22 , 34 ), // #97
  INST(Cmovns           , X86Rm              , O(000F00,49,_,_,x,_,_,_  ), 0                         , 4  , 0  , 393  , 22 , 35 ), // #98
  INST(Cmovnz           , X86Rm              , O(000F00,45,_,_,x,_,_,_  ), 0                         , 4  , 0  , 400  , 22 , 30 ), // #99
  INST(Cmovo            , X86Rm              , O(000F00,40,_,_,x,_,_,_  ), 0                         , 4  , 0  , 407  , 22 , 33 ), // #100
  INST(Cmovp            , X86Rm              , O(000F00,4A,_,_,x,_,_,_  ), 0                         , 4  , 0  , 413  , 22 , 34 ), // #101
  INST(Cmovpe           , X86Rm              , O(000F00,4A,_,_,x,_,_,_  ), 0                         , 4  , 0  , 419  , 22 , 34 ), // #102
  INST(Cmovpo           , X86Rm              , O(000F00,4B,_,_,x,_,_,_  ), 0                         , 4  , 0  , 426  , 22 , 34 ), // #103
  INST(Cmovs            , X86Rm              , O(000F00,48,_,_,x,_,_,_  ), 0                         , 4  , 0  , 433  , 22 , 35 ), // #104
  INST(Cmovz            , X86Rm              , O(000F00,44,_,_,x,_,_,_  ), 0                         , 4  , 0  , 439  , 22 , 30 ), // #105
  INST(Cmp              , X86Arith           , O(000000,38,7,_,x,_,_,_  ), 0                         , 26 , 0  , 445  , 34 , 1  ), // #106
  INST(Cmppd            , ExtRmi             , O(660F00,C2,_,_,_,_,_,_  ), 0                         , 3  , 0  , 3726 , 8  , 4  ), // #107
  INST(Cmpps            , ExtRmi             , O(000F00,C2,_,_,_,_,_,_  ), 0                         , 4  , 0  , 3733 , 8  , 5  ), // #108
  INST(Cmps             , X86StrMm           , O(000000,A6,_,_,_,_,_,_  ), 0                         , 0  , 0  , 449  , 35 , 36 ), // #109
  INST(Cmpsd            , ExtRmi             , O(F20F00,C2,_,_,_,_,_,_  ), 0                         , 5  , 0  , 3740 , 36 , 4  ), // #110
  INST(Cmpss            , ExtRmi             , O(F30F00,C2,_,_,_,_,_,_  ), 0                         , 6  , 0  , 3747 , 37 , 5  ), // #111
  INST(Cmpxchg          , X86Cmpxchg         , O(000F00,B0,_,_,x,_,_,_  ), 0                         , 4  , 0  , 454  , 38 , 37 ), // #112
  INST(Cmpxchg16b       , X86Cmpxchg8b_16b   , O(000F00,C7,1,_,1,_,_,_  ), 0                         , 27 , 0  , 462  , 39 , 38 ), // #113
  INST(Cmpxchg8b        , X86Cmpxchg8b_16b   , O(000F00,C7,1,_,_,_,_,_  ), 0                         , 28 , 0  , 473  , 40 , 39 ), // #114
  INST(Comisd           , ExtRm              , O(660F00,2F,_,_,_,_,_,_  ), 0                         , 3  , 0  , 10253, 6  , 40 ), // #115
  INST(Comiss           , ExtRm              , O(000F00,2F,_,_,_,_,_,_  ), 0                         , 4  , 0  , 10262, 7  , 41 ), // #116
  INST(Cpuid            , X86Op              , O(000F00,A2,_,_,_,_,_,_  ), 0                         , 4  , 0  , 483  , 41 , 42 ), // #117
  INST(Cqo              , X86Op_xDX_xAX      , O(000000,99,_,_,1,_,_,_  ), 0                         , 20 , 0  , 489  , 42 , 0  ), // #118
  INST(Crc32            , X86Crc             , O(F20F38,F0,_,_,x,_,_,_  ), 0                         , 29 , 0  , 493  , 43 , 43 ), // #119
  INST(Cvtdq2pd         , ExtRm              , O(F30F00,E6,_,_,_,_,_,_  ), 0                         , 6  , 0  , 3794 , 6  , 4  ), // #120
  INST(Cvtdq2ps         , ExtRm              , O(000F00,5B,_,_,_,_,_,_  ), 0                         , 4  , 0  , 3804 , 5  , 4  ), // #121
  INST(Cvtpd2dq         , ExtRm              , O(F20F00,E6,_,_,_,_,_,_  ), 0                         , 5  , 0  , 3843 , 5  , 4  ), // #122
  INST(Cvtpd2pi         , ExtRm              , O(660F00,2D,_,_,_,_,_,_  ), 0                         , 3  , 0  , 499  , 44 , 4  ), // #123
  INST(Cvtpd2ps         , ExtRm              , O(660F00,5A,_,_,_,_,_,_  ), 0                         , 3  , 0  , 3853 , 5  , 4  ), // #124
  INST(Cvtpi2pd         , ExtRm              , O(660F00,2A,_,_,_,_,_,_  ), 0                         , 3  , 0  , 508  , 45 , 4  ), // #125
  INST(Cvtpi2ps         , ExtRm              , O(000F00,2A,_,_,_,_,_,_  ), 0                         , 4  , 0  , 517  , 45 , 5  ), // #126
  INST(Cvtps2dq         , ExtRm              , O(660F00,5B,_,_,_,_,_,_  ), 0                         , 3  , 0  , 3905 , 5  , 4  ), // #127
  INST(Cvtps2pd         , ExtRm              , O(000F00,5A,_,_,_,_,_,_  ), 0                         , 4  , 0  , 3915 , 6  , 4  ), // #128
  INST(Cvtps2pi         , ExtRm              , O(000F00,2D,_,_,_,_,_,_  ), 0                         , 4  , 0  , 526  , 46 , 5  ), // #129
  INST(Cvtsd2si         , ExtRm_Wx           , O(F20F00,2D,_,_,x,_,_,_  ), 0                         , 5  , 0  , 3987 , 47 , 4  ), // #130
  INST(Cvtsd2ss         , ExtRm              , O(F20F00,5A,_,_,_,_,_,_  ), 0                         , 5  , 0  , 3997 , 6  , 4  ), // #131
  INST(Cvtsi2sd         , ExtRm_Wx           , O(F20F00,2A,_,_,x,_,_,_  ), 0                         , 5  , 0  , 4018 , 48 , 4  ), // #132
  INST(Cvtsi2ss         , ExtRm_Wx           , O(F30F00,2A,_,_,x,_,_,_  ), 0                         , 6  , 0  , 4028 , 48 , 5  ), // #133
  INST(Cvtss2sd         , ExtRm              , O(F30F00,5A,_,_,_,_,_,_  ), 0                         , 6  , 0  , 4038 , 7  , 4  ), // #134
  INST(Cvtss2si         , ExtRm_Wx           , O(F30F00,2D,_,_,x,_,_,_  ), 0                         , 6  , 0  , 4048 , 49 , 5  ), // #135
  INST(Cvttpd2dq        , ExtRm              , O(660F00,E6,_,_,_,_,_,_  ), 0                         , 3  , 0  , 4069 , 5  , 4  ), // #136
  INST(Cvttpd2pi        , ExtRm              , O(660F00,2C,_,_,_,_,_,_  ), 0                         , 3  , 0  , 535  , 44 , 4  ), // #137
  INST(Cvttps2dq        , ExtRm              , O(F30F00,5B,_,_,_,_,_,_  ), 0                         , 6  , 0  , 4115 , 5  , 4  ), // #138
  INST(Cvttps2pi        , ExtRm              , O(000F00,2C,_,_,_,_,_,_  ), 0                         , 4  , 0  , 545  , 46 , 5  ), // #139
  INST(Cvttsd2si        , ExtRm_Wx           , O(F20F00,2C,_,_,x,_,_,_  ), 0                         , 5  , 0  , 4161 , 47 , 4  ), // #140
  INST(Cvttss2si        , ExtRm_Wx           , O(F30F00,2C,_,_,x,_,_,_  ), 0                         , 6  , 0  , 4184 , 49 , 5  ), // #141
  INST(Cwd              , X86Op_xDX_xAX      , O(660000,99,_,_,_,_,_,_  ), 0                         , 19 , 0  , 555  , 50 , 0  ), // #142
  INST(Cwde             , X86Op_xAX          , O(000000,98,_,_,_,_,_,_  ), 0                         , 0  , 0  , 559  , 51 , 0  ), // #143
  INST(Daa              , X86Op              , O(000000,27,_,_,_,_,_,_  ), 0                         , 0  , 0  , 564  , 1  , 1  ), // #144
  INST(Das              , X86Op              , O(000000,2F,_,_,_,_,_,_  ), 0                         , 0  , 0  , 568  , 1  , 1  ), // #145
  INST(Dec              , X86IncDec          , O(000000,FE,1,_,x,_,_,_  ), O(000000,48,_,_,x,_,_,_  ), 30 , 6  , 3308 , 52 , 44 ), // #146
  INST(Div              , X86M_GPB_MulDiv    , O(000000,F6,6,_,x,_,_,_  ), 0                         , 31 , 0  , 805  , 53 , 1  ), // #147
  INST(Divpd            , ExtRm              , O(660F00,5E,_,_,_,_,_,_  ), 0                         , 3  , 0  , 4283 , 5  , 4  ), // #148
  INST(Divps            , ExtRm              , O(000F00,5E,_,_,_,_,_,_  ), 0                         , 4  , 0  , 4290 , 5  , 5  ), // #149
  INST(Divsd            , ExtRm              , O(F20F00,5E,_,_,_,_,_,_  ), 0                         , 5  , 0  , 4297 , 6  , 4  ), // #150
  INST(Divss            , ExtRm              , O(F30F00,5E,_,_,_,_,_,_  ), 0                         , 6  , 0  , 4304 , 7  , 5  ), // #151
  INST(Dppd             , ExtRmi             , O(660F3A,41,_,_,_,_,_,_  ), 0                         , 8  , 0  , 4321 , 8  , 12 ), // #152
  INST(Dpps             , ExtRmi             , O(660F3A,40,_,_,_,_,_,_  ), 0                         , 8  , 0  , 4327 , 8  , 12 ), // #153
  INST(Emms             , X86Op              , O(000F00,77,_,_,_,_,_,_  ), 0                         , 4  , 0  , 773  , 54 , 45 ), // #154
  INST(Endbr32          , X86Op_Mod11RM      , O(F30F00,FB,7,_,_,_,_,3  ), 0                         , 32 , 0  , 572  , 30 , 46 ), // #155
  INST(Endbr64          , X86Op_Mod11RM      , O(F30F00,FA,7,_,_,_,_,2  ), 0                         , 33 , 0  , 580  , 30 , 46 ), // #156
  INST(Enqcmd           , X86EnqcmdMovdir64b , O(F20F38,F8,_,_,_,_,_,_  ), 0                         , 29 , 0  , 588  , 55 , 47 ), // #157
  INST(Enqcmds          , X86EnqcmdMovdir64b , O(F30F38,F8,_,_,_,_,_,_  ), 0                         , 7  , 0  , 595  , 55 , 47 ), // #158
  INST(Enter            , X86Enter           , O(000000,C8,_,_,_,_,_,_  ), 0                         , 0  , 0  , 3024 , 56 , 0  ), // #159
  INST(Extractps        , ExtExtract         , O(660F3A,17,_,_,_,_,_,_  ), 0                         , 8  , 0  , 4517 , 57 , 12 ), // #160
  INST(Extrq            , ExtExtrq           , O(660F00,79,_,_,_,_,_,_  ), O(660F00,78,0,_,_,_,_,_  ), 3  , 7  , 7613 , 58 , 48 ), // #161
  INST(F2xm1            , FpuOp              , O_FPU(00,D9F0,_)          , 0                         , 34 , 0  , 603  , 30 , 0  ), // #162
  INST(Fabs             , FpuOp              , O_FPU(00,D9E1,_)          , 0                         , 34 , 0  , 609  , 30 , 0  ), // #163
  INST(Fadd             , FpuArith           , O_FPU(00,C0C0,0)          , 0                         , 35 , 0  , 2113 , 59 , 0  ), // #164
  INST(Faddp            , FpuRDef            , O_FPU(00,DEC0,_)          , 0                         , 36 , 0  , 614  , 60 , 0  ), // #165
  INST(Fbld             , X86M_Only          , O_FPU(00,00DF,4)          , 0                         , 37 , 0  , 620  , 61 , 0  ), // #166
  INST(Fbstp            , X86M_Only          , O_FPU(00,00DF,6)          , 0                         , 38 , 0  , 625  , 61 , 0  ), // #167
  INST(Fchs             , FpuOp              , O_FPU(00,D9E0,_)          , 0                         , 34 , 0  , 631  , 30 , 0  ), // #168
  INST(Fclex            , FpuOp              , O_FPU(9B,DBE2,_)          , 0                         , 39 , 0  , 636  , 30 , 0  ), // #169
  INST(Fcmovb           , FpuR               , O_FPU(00,DAC0,_)          , 0                         , 40 , 0  , 642  , 62 , 29 ), // #170
  INST(Fcmovbe          , FpuR               , O_FPU(00,DAD0,_)          , 0                         , 40 , 0  , 649  , 62 , 28 ), // #171
  INST(Fcmove           , FpuR               , O_FPU(00,DAC8,_)          , 0                         , 40 , 0  , 657  , 62 , 30 ), // #172
  INST(Fcmovnb          , FpuR               , O_FPU(00,DBC0,_)          , 0                         , 41 , 0  , 664  , 62 , 29 ), // #173
  INST(Fcmovnbe         , FpuR               , O_FPU(00,DBD0,_)          , 0                         , 41 , 0  , 672  , 62 , 28 ), // #174
  INST(Fcmovne          , FpuR               , O_FPU(00,DBC8,_)          , 0                         , 41 , 0  , 681  , 62 , 30 ), // #175
  INST(Fcmovnu          , FpuR               , O_FPU(00,DBD8,_)          , 0                         , 41 , 0  , 689  , 62 , 34 ), // #176
  INST(Fcmovu           , FpuR               , O_FPU(00,DAD8,_)          , 0                         , 40 , 0  , 697  , 62 , 34 ), // #177
  INST(Fcom             , FpuCom             , O_FPU(00,D0D0,2)          , 0                         , 42 , 0  , 704  , 63 , 0  ), // #178
  INST(Fcomi            , FpuR               , O_FPU(00,DBF0,_)          , 0                         , 41 , 0  , 709  , 62 , 49 ), // #179
  INST(Fcomip           , FpuR               , O_FPU(00,DFF0,_)          , 0                         , 43 , 0  , 715  , 62 , 49 ), // #180
  INST(Fcomp            , FpuCom             , O_FPU(00,D8D8,3)          , 0                         , 44 , 0  , 722  , 63 , 0  ), // #181
  INST(Fcompp           , FpuOp              , O_FPU(00,DED9,_)          , 0                         , 36 , 0  , 728  , 30 , 0  ), // #182
  INST(Fcos             , FpuOp              , O_FPU(00,D9FF,_)          , 0                         , 34 , 0  , 735  , 30 , 0  ), // #183
  INST(Fdecstp          , FpuOp              , O_FPU(00,D9F6,_)          , 0                         , 34 , 0  , 740  , 30 , 0  ), // #184
  INST(Fdiv             , FpuArith           , O_FPU(00,F0F8,6)          , 0                         , 45 , 0  , 748  , 59 , 0  ), // #185
  INST(Fdivp            , FpuRDef            , O_FPU(00,DEF8,_)          , 0                         , 36 , 0  , 753  , 60 , 0  ), // #186
  INST(Fdivr            , FpuArith           , O_FPU(00,F8F0,7)          , 0                         , 46 , 0  , 759  , 59 , 0  ), // #187
  INST(Fdivrp           , FpuRDef            , O_FPU(00,DEF0,_)          , 0                         , 36 , 0  , 765  , 60 , 0  ), // #188
  INST(Femms            , X86Op              , O(000F00,0E,_,_,_,_,_,_  ), 0                         , 4  , 0  , 772  , 30 , 50 ), // #189
  INST(Ffree            , FpuR               , O_FPU(00,DDC0,_)          , 0                         , 47 , 0  , 778  , 62 , 0  ), // #190
  INST(Fiadd            , FpuM               , O_FPU(00,00DA,0)          , 0                         , 48 , 0  , 784  , 64 , 0  ), // #191
  INST(Ficom            , FpuM               , O_FPU(00,00DA,2)          , 0                         , 49 , 0  , 790  , 64 , 0  ), // #192
  INST(Ficomp           , FpuM               , O_FPU(00,00DA,3)          , 0                         , 50 , 0  , 796  , 64 , 0  ), // #193
  INST(Fidiv            , FpuM               , O_FPU(00,00DA,6)          , 0                         , 38 , 0  , 803  , 64 , 0  ), // #194
  INST(Fidivr           , FpuM               , O_FPU(00,00DA,7)          , 0                         , 51 , 0  , 809  , 64 , 0  ), // #195
  INST(Fild             , FpuM               , O_FPU(00,00DB,0)          , O_FPU(00,00DF,5)          , 48 , 8  , 816  , 65 , 0  ), // #196
  INST(Fimul            , FpuM               , O_FPU(00,00DA,1)          , 0                         , 52 , 0  , 821  , 64 , 0  ), // #197
  INST(Fincstp          , FpuOp              , O_FPU(00,D9F7,_)          , 0                         , 34 , 0  , 827  , 30 , 0  ), // #198
  INST(Finit            , FpuOp              , O_FPU(9B,DBE3,_)          , 0                         , 39 , 0  , 835  , 30 , 0  ), // #199
  INST(Fist             , FpuM               , O_FPU(00,00DB,2)          , 0                         , 49 , 0  , 841  , 64 , 0  ), // #200
  INST(Fistp            , FpuM               , O_FPU(00,00DB,3)          , O_FPU(00,00DF,7)          , 50 , 9  , 846  , 65 , 0  ), // #201
  INST(Fisttp           , FpuM               , O_FPU(00,00DB,1)          , O_FPU(00,00DD,1)          , 52 , 10 , 852  , 65 , 6  ), // #202
  INST(Fisub            , FpuM               , O_FPU(00,00DA,4)          , 0                         , 37 , 0  , 859  , 64 , 0  ), // #203
  INST(Fisubr           , FpuM               , O_FPU(00,00DA,5)          , 0                         , 53 , 0  , 865  , 64 , 0  ), // #204
  INST(Fld              , FpuFldFst          , O_FPU(00,00D9,0)          , O_FPU(00,00DB,5)          , 48 , 11 , 872  , 66 , 0  ), // #205
  INST(Fld1             , FpuOp              , O_FPU(00,D9E8,_)          , 0                         , 34 , 0  , 876  , 30 , 0  ), // #206
  INST(Fldcw            , X86M_Only          , O_FPU(00,00D9,5)          , 0                         , 53 , 0  , 881  , 67 , 0  ), // #207
  INST(Fldenv           , X86M_Only          , O_FPU(00,00D9,4)          , 0                         , 37 , 0  , 887  , 31 , 0  ), // #208
  INST(Fldl2e           , FpuOp              , O_FPU(00,D9EA,_)          , 0                         , 34 , 0  , 894  , 30 , 0  ), // #209
  INST(Fldl2t           , FpuOp              , O_FPU(00,D9E9,_)          , 0                         , 34 , 0  , 901  , 30 , 0  ), // #210
  INST(Fldlg2           , FpuOp              , O_FPU(00,D9EC,_)          , 0                         , 34 , 0  , 908  , 30 , 0  ), // #211
  INST(Fldln2           , FpuOp              , O_FPU(00,D9ED,_)          , 0                         , 34 , 0  , 915  , 30 , 0  ), // #212
  INST(Fldpi            , FpuOp              , O_FPU(00,D9EB,_)          , 0                         , 34 , 0  , 922  , 30 , 0  ), // #213
  INST(Fldz             , FpuOp              , O_FPU(00,D9EE,_)          , 0                         , 34 , 0  , 928  , 30 , 0  ), // #214
  INST(Fmul             , FpuArith           , O_FPU(00,C8C8,1)          , 0                         , 54 , 0  , 2155 , 59 , 0  ), // #215
  INST(Fmulp            , FpuRDef            , O_FPU(00,DEC8,_)          , 0                         , 36 , 0  , 933  , 60 , 0  ), // #216
  INST(Fnclex           , FpuOp              , O_FPU(00,DBE2,_)          , 0                         , 41 , 0  , 939  , 30 , 0  ), // #217
  INST(Fninit           , FpuOp              , O_FPU(00,DBE3,_)          , 0                         , 41 , 0  , 946  , 30 , 0  ), // #218
  INST(Fnop             , FpuOp              , O_FPU(00,D9D0,_)          , 0                         , 34 , 0  , 953  , 30 , 0  ), // #219
  INST(Fnsave           , X86M_Only          , O_FPU(00,00DD,6)          , 0                         , 38 , 0  , 958  , 31 , 0  ), // #220
  INST(Fnstcw           , X86M_Only          , O_FPU(00,00D9,7)          , 0                         , 51 , 0  , 965  , 67 , 0  ), // #221
  INST(Fnstenv          , X86M_Only          , O_FPU(00,00D9,6)          , 0                         , 38 , 0  , 972  , 31 , 0  ), // #222
  INST(Fnstsw           , FpuStsw            , O_FPU(00,00DD,7)          , O_FPU(00,DFE0,_)          , 51 , 12 , 980  , 68 , 0  ), // #223
  INST(Fpatan           , FpuOp              , O_FPU(00,D9F3,_)          , 0                         , 34 , 0  , 987  , 30 , 0  ), // #224
  INST(Fprem            , FpuOp              , O_FPU(00,D9F8,_)          , 0                         , 34 , 0  , 994  , 30 , 0  ), // #225
  INST(Fprem1           , FpuOp              , O_FPU(00,D9F5,_)          , 0                         , 34 , 0  , 1000 , 30 , 0  ), // #226
  INST(Fptan            , FpuOp              , O_FPU(00,D9F2,_)          , 0                         , 34 , 0  , 1007 , 30 , 0  ), // #227
  INST(Frndint          , FpuOp              , O_FPU(00,D9FC,_)          , 0                         , 34 , 0  , 1013 , 30 , 0  ), // #228
  INST(Frstor           , X86M_Only          , O_FPU(00,00DD,4)          , 0                         , 37 , 0  , 1021 , 31 , 0  ), // #229
  INST(Fsave            , X86M_Only          , O_FPU(9B,00DD,6)          , 0                         , 55 , 0  , 1028 , 31 , 0  ), // #230
  INST(Fscale           , FpuOp              , O_FPU(00,D9FD,_)          , 0                         , 34 , 0  , 1034 , 30 , 0  ), // #231
  INST(Fsin             , FpuOp              , O_FPU(00,D9FE,_)          , 0                         , 34 , 0  , 1041 , 30 , 0  ), // #232
  INST(Fsincos          , FpuOp              , O_FPU(00,D9FB,_)          , 0                         , 34 , 0  , 1046 , 30 , 0  ), // #233
  INST(Fsqrt            , FpuOp              , O_FPU(00,D9FA,_)          , 0                         , 34 , 0  , 1054 , 30 , 0  ), // #234
  INST(Fst              , FpuFldFst          , O_FPU(00,00D9,2)          , 0                         , 49 , 0  , 1060 , 69 , 0  ), // #235
  INST(Fstcw            , X86M_Only          , O_FPU(9B,00D9,7)          , 0                         , 56 , 0  , 1064 , 67 , 0  ), // #236
  INST(Fstenv           , X86M_Only          , O_FPU(9B,00D9,6)          , 0                         , 55 , 0  , 1070 , 31 , 0  ), // #237
  INST(Fstp             , FpuFldFst          , O_FPU(00,00D9,3)          , O(000000,DB,7,_,_,_,_,_  ), 50 , 13 , 1077 , 66 , 0  ), // #238
  INST(Fstsw            , FpuStsw            , O_FPU(9B,00DD,7)          , O_FPU(9B,DFE0,_)          , 56 , 14 , 1082 , 68 , 0  ), // #239
  INST(Fsub             , FpuArith           , O_FPU(00,E0E8,4)          , 0                         , 57 , 0  , 2233 , 59 , 0  ), // #240
  INST(Fsubp            , FpuRDef            , O_FPU(00,DEE8,_)          , 0                         , 36 , 0  , 1088 , 60 , 0  ), // #241
  INST(Fsubr            , FpuArith           , O_FPU(00,E8E0,5)          , 0                         , 58 , 0  , 2239 , 59 , 0  ), // #242
  INST(Fsubrp           , FpuRDef            , O_FPU(00,DEE0,_)          , 0                         , 36 , 0  , 1094 , 60 , 0  ), // #243
  INST(Ftst             , FpuOp              , O_FPU(00,D9E4,_)          , 0                         , 34 , 0  , 1101 , 30 , 0  ), // #244
  INST(Fucom            , FpuRDef            , O_FPU(00,DDE0,_)          , 0                         , 47 , 0  , 1106 , 60 , 0  ), // #245
  INST(Fucomi           , FpuR               , O_FPU(00,DBE8,_)          , 0                         , 41 , 0  , 1112 , 62 , 49 ), // #246
  INST(Fucomip          , FpuR               , O_FPU(00,DFE8,_)          , 0                         , 43 , 0  , 1119 , 62 , 49 ), // #247
  INST(Fucomp           , FpuRDef            , O_FPU(00,DDE8,_)          , 0                         , 47 , 0  , 1127 , 60 , 0  ), // #248
  INST(Fucompp          , FpuOp              , O_FPU(00,DAE9,_)          , 0                         , 40 , 0  , 1134 , 30 , 0  ), // #249
  INST(Fwait            , X86Op              , O_FPU(00,009B,_)          , 0                         , 59 , 0  , 1142 , 30 , 0  ), // #250
  INST(Fxam             , FpuOp              , O_FPU(00,D9E5,_)          , 0                         , 34 , 0  , 1148 , 30 , 0  ), // #251
  INST(Fxch             , FpuR               , O_FPU(00,D9C8,_)          , 0                         , 34 , 0  , 1153 , 60 , 0  ), // #252
  INST(Fxrstor          , X86M_Only          , O(000F00,AE,1,_,_,_,_,_  ), 0                         , 28 , 0  , 1158 , 31 , 51 ), // #253
  INST(Fxrstor64        , X86M_Only          , O(000F00,AE,1,_,1,_,_,_  ), 0                         , 27 , 0  , 1166 , 70 , 51 ), // #254
  INST(Fxsave           , X86M_Only          , O(000F00,AE,0,_,_,_,_,_  ), 0                         , 4  , 0  , 1176 , 31 , 51 ), // #255
  INST(Fxsave64         , X86M_Only          , O(000F00,AE,0,_,1,_,_,_  ), 0                         , 60 , 0  , 1183 , 70 , 51 ), // #256
  INST(Fxtract          , FpuOp              , O_FPU(00,D9F4,_)          , 0                         , 34 , 0  , 1192 , 30 , 0  ), // #257
  INST(Fyl2x            , FpuOp              , O_FPU(00,D9F1,_)          , 0                         , 34 , 0  , 1200 , 30 , 0  ), // #258
  INST(Fyl2xp1          , FpuOp              , O_FPU(00,D9F9,_)          , 0                         , 34 , 0  , 1206 , 30 , 0  ), // #259
  INST(Getsec           , X86Op              , O(000F00,37,_,_,_,_,_,_  ), 0                         , 4  , 0  , 1214 , 30 , 52 ), // #260
  INST(Gf2p8affineinvqb , ExtRmi             , O(660F3A,CF,_,_,_,_,_,_  ), 0                         , 8  , 0  , 5872 , 8  , 53 ), // #261
  INST(Gf2p8affineqb    , ExtRmi             , O(660F3A,CE,_,_,_,_,_,_  ), 0                         , 8  , 0  , 5890 , 8  , 53 ), // #262
  INST(Gf2p8mulb        , ExtRm              , O(660F38,CF,_,_,_,_,_,_  ), 0                         , 2  , 0  , 5905 , 5  , 53 ), // #263
  INST(Haddpd           , ExtRm              , O(660F00,7C,_,_,_,_,_,_  ), 0                         , 3  , 0  , 5916 , 5  , 6  ), // #264
  INST(Haddps           , ExtRm              , O(F20F00,7C,_,_,_,_,_,_  ), 0                         , 5  , 0  , 5924 , 5  , 6  ), // #265
  INST(Hlt              , X86Op              , O(000000,F4,_,_,_,_,_,_  ), 0                         , 0  , 0  , 1221 , 30 , 0  ), // #266
  INST(Hsubpd           , ExtRm              , O(660F00,7D,_,_,_,_,_,_  ), 0                         , 3  , 0  , 5932 , 5  , 6  ), // #267
  INST(Hsubps           , ExtRm              , O(F20F00,7D,_,_,_,_,_,_  ), 0                         , 5  , 0  , 5940 , 5  , 6  ), // #268
  INST(Idiv             , X86M_GPB_MulDiv    , O(000000,F6,7,_,x,_,_,_  ), 0                         , 26 , 0  , 804  , 53 , 1  ), // #269
  INST(Imul             , X86Imul            , O(000000,F6,5,_,x,_,_,_  ), 0                         , 61 , 0  , 822  , 71 , 1  ), // #270
  INST(In               , X86In              , O(000000,EC,_,_,_,_,_,_  ), O(000000,E4,_,_,_,_,_,_  ), 0  , 15 , 10425, 72 , 0  ), // #271
  INST(Inc              , X86IncDec          , O(000000,FE,0,_,x,_,_,_  ), O(000000,40,_,_,x,_,_,_  ), 0  , 16 , 1225 , 52 , 44 ), // #272
  INST(Incsspd          , X86M               , O(F30F00,AE,5,_,0,_,_,_  ), 0                         , 62 , 0  , 1229 , 73 , 54 ), // #273
  INST(Incsspq          , X86M               , O(F30F00,AE,5,_,1,_,_,_  ), 0                         , 63 , 0  , 1237 , 74 , 54 ), // #274
  INST(Ins              , X86Ins             , O(000000,6C,_,_,_,_,_,_  ), 0                         , 0  , 0  , 1908 , 75 , 0  ), // #275
  INST(Insertps         , ExtRmi             , O(660F3A,21,_,_,_,_,_,_  ), 0                         , 8  , 0  , 6076 , 37 , 12 ), // #276
  INST(Insertq          , ExtInsertq         , O(F20F00,79,_,_,_,_,_,_  ), O(F20F00,78,_,_,_,_,_,_  ), 5  , 17 , 1245 , 76 , 48 ), // #277
  INST(Int              , X86Int             , O(000000,CD,_,_,_,_,_,_  ), 0                         , 0  , 0  , 1017 , 77 , 0  ), // #278
  INST(Int3             , X86Op              , O(000000,CC,_,_,_,_,_,_  ), 0                         , 0  , 0  , 1253 , 30 , 0  ), // #279
  INST(Into             , X86Op              , O(000000,CE,_,_,_,_,_,_  ), 0                         , 0  , 0  , 1258 , 78 , 55 ), // #280
  INST(Invd             , X86Op              , O(000F00,08,_,_,_,_,_,_  ), 0                         , 4  , 0  , 10354, 30 , 42 ), // #281
  INST(Invept           , X86Rm_NoSize       , O(660F38,80,_,_,_,_,_,_  ), 0                         , 2  , 0  , 1263 , 79 , 56 ), // #282
  INST(Invlpg           , X86M_Only          , O(000F00,01,7,_,_,_,_,_  ), 0                         , 22 , 0  , 1270 , 31 , 42 ), // #283
  INST(Invlpga          , X86Op_xAddr        , O(000F01,DF,_,_,_,_,_,_  ), 0                         , 21 , 0  , 1277 , 80 , 22 ), // #284
  INST(Invpcid          , X86Rm_NoSize       , O(660F38,82,_,_,_,_,_,_  ), 0                         , 2  , 0  , 1285 , 79 , 42 ), // #285
  INST(Invvpid          , X86Rm_NoSize       , O(660F38,81,_,_,_,_,_,_  ), 0                         , 2  , 0  , 1293 , 79 , 56 ), // #286
  INST(Iret             , X86Op              , O(000000,CF,_,_,_,_,_,_  ), 0                         , 0  , 0  , 1301 , 81 , 1  ), // #287
  INST(Iretd            , X86Op              , O(000000,CF,_,_,_,_,_,_  ), 0                         , 0  , 0  , 1306 , 81 , 1  ), // #288
  INST(Iretq            , X86Op              , O(000000,CF,_,_,1,_,_,_  ), 0                         , 20 , 0  , 1312 , 82 , 1  ), // #289
  INST(Iretw            , X86Op              , O(660000,CF,_,_,_,_,_,_  ), 0                         , 19 , 0  , 1318 , 81 , 1  ), // #290
  INST(Ja               , X86Jcc             , O(000F00,87,_,_,_,_,_,_  ), O(000000,77,_,_,_,_,_,_  ), 4  , 18 , 1324 , 83 , 57 ), // #291
  INST(Jae              , X86Jcc             , O(000F00,83,_,_,_,_,_,_  ), O(000000,73,_,_,_,_,_,_  ), 4  , 19 , 1327 , 83 , 58 ), // #292
  INST(Jb               , X86Jcc             , O(000F00,82,_,_,_,_,_,_  ), O(000000,72,_,_,_,_,_,_  ), 4  , 20 , 1331 , 83 , 58 ), // #293
  INST(Jbe              , X86Jcc             , O(000F00,86,_,_,_,_,_,_  ), O(000000,76,_,_,_,_,_,_  ), 4  , 21 , 1334 , 83 , 57 ), // #294
  INST(Jc               , X86Jcc             , O(000F00,82,_,_,_,_,_,_  ), O(000000,72,_,_,_,_,_,_  ), 4  , 20 , 1338 , 83 , 58 ), // #295
  INST(Je               , X86Jcc             , O(000F00,84,_,_,_,_,_,_  ), O(000000,74,_,_,_,_,_,_  ), 4  , 22 , 1341 , 83 , 59 ), // #296
  INST(Jecxz            , X86JecxzLoop       , 0                         , O(000000,E3,_,_,_,_,_,_  ), 0  , 23 , 1344 , 84 , 0  ), // #297
  INST(Jg               , X86Jcc             , O(000F00,8F,_,_,_,_,_,_  ), O(000000,7F,_,_,_,_,_,_  ), 4  , 24 , 1350 , 83 , 60 ), // #298
  INST(Jge              , X86Jcc             , O(000F00,8D,_,_,_,_,_,_  ), O(000000,7D,_,_,_,_,_,_  ), 4  , 25 , 1353 , 83 , 61 ), // #299
  INST(Jl               , X86Jcc             , O(000F00,8C,_,_,_,_,_,_  ), O(000000,7C,_,_,_,_,_,_  ), 4  , 26 , 1357 , 83 , 61 ), // #300
  INST(Jle              , X86Jcc             , O(000F00,8E,_,_,_,_,_,_  ), O(000000,7E,_,_,_,_,_,_  ), 4  , 27 , 1360 , 83 , 60 ), // #301
  INST(Jmp              , X86Jmp             , O(000000,FF,4,_,_,_,_,_  ), O(000000,EB,_,_,_,_,_,_  ), 9  , 28 , 1364 , 85 , 0  ), // #302
  INST(Jna              , X86Jcc             , O(000F00,86,_,_,_,_,_,_  ), O(000000,76,_,_,_,_,_,_  ), 4  , 21 , 1368 , 83 , 57 ), // #303
  INST(Jnae             , X86Jcc             , O(000F00,82,_,_,_,_,_,_  ), O(000000,72,_,_,_,_,_,_  ), 4  , 20 , 1372 , 83 , 58 ), // #304
  INST(Jnb              , X86Jcc             , O(000F00,83,_,_,_,_,_,_  ), O(000000,73,_,_,_,_,_,_  ), 4  , 19 , 1377 , 83 , 58 ), // #305
  INST(Jnbe             , X86Jcc             , O(000F00,87,_,_,_,_,_,_  ), O(000000,77,_,_,_,_,_,_  ), 4  , 18 , 1381 , 83 , 57 ), // #306
  INST(Jnc              , X86Jcc             , O(000F00,83,_,_,_,_,_,_  ), O(000000,73,_,_,_,_,_,_  ), 4  , 19 , 1386 , 83 , 58 ), // #307
  INST(Jne              , X86Jcc             , O(000F00,85,_,_,_,_,_,_  ), O(000000,75,_,_,_,_,_,_  ), 4  , 29 , 1390 , 83 , 59 ), // #308
  INST(Jng              , X86Jcc             , O(000F00,8E,_,_,_,_,_,_  ), O(000000,7E,_,_,_,_,_,_  ), 4  , 27 , 1394 , 83 , 60 ), // #309
  INST(Jnge             , X86Jcc             , O(000F00,8C,_,_,_,_,_,_  ), O(000000,7C,_,_,_,_,_,_  ), 4  , 26 , 1398 , 83 , 61 ), // #310
  INST(Jnl              , X86Jcc             , O(000F00,8D,_,_,_,_,_,_  ), O(000000,7D,_,_,_,_,_,_  ), 4  , 25 , 1403 , 83 , 61 ), // #311
  INST(Jnle             , X86Jcc             , O(000F00,8F,_,_,_,_,_,_  ), O(000000,7F,_,_,_,_,_,_  ), 4  , 24 , 1407 , 83 , 60 ), // #312
  INST(Jno              , X86Jcc             , O(000F00,81,_,_,_,_,_,_  ), O(000000,71,_,_,_,_,_,_  ), 4  , 30 , 1412 , 83 , 55 ), // #313
  INST(Jnp              , X86Jcc             , O(000F00,8B,_,_,_,_,_,_  ), O(000000,7B,_,_,_,_,_,_  ), 4  , 31 , 1416 , 83 , 62 ), // #314
  INST(Jns              , X86Jcc             , O(000F00,89,_,_,_,_,_,_  ), O(000000,79,_,_,_,_,_,_  ), 4  , 32 , 1420 , 83 , 63 ), // #315
  INST(Jnz              , X86Jcc             , O(000F00,85,_,_,_,_,_,_  ), O(000000,75,_,_,_,_,_,_  ), 4  , 29 , 1424 , 83 , 59 ), // #316
  INST(Jo               , X86Jcc             , O(000F00,80,_,_,_,_,_,_  ), O(000000,70,_,_,_,_,_,_  ), 4  , 33 , 1428 , 83 , 55 ), // #317
  INST(Jp               , X86Jcc             , O(000F00,8A,_,_,_,_,_,_  ), O(000000,7A,_,_,_,_,_,_  ), 4  , 34 , 1431 , 83 , 62 ), // #318
  INST(Jpe              , X86Jcc             , O(000F00,8A,_,_,_,_,_,_  ), O(000000,7A,_,_,_,_,_,_  ), 4  , 34 , 1434 , 83 , 62 ), // #319
  INST(Jpo              , X86Jcc             , O(000F00,8B,_,_,_,_,_,_  ), O(000000,7B,_,_,_,_,_,_  ), 4  , 31 , 1438 , 83 , 62 ), // #320
  INST(Js               , X86Jcc             , O(000F00,88,_,_,_,_,_,_  ), O(000000,78,_,_,_,_,_,_  ), 4  , 35 , 1442 , 83 , 63 ), // #321
  INST(Jz               , X86Jcc             , O(000F00,84,_,_,_,_,_,_  ), O(000000,74,_,_,_,_,_,_  ), 4  , 22 , 1445 , 83 , 59 ), // #322
  INST(Kaddb            , VexRvm             , V(660F00,4A,_,1,0,_,_,_  ), 0                         , 64 , 0  , 1448 , 86 , 64 ), // #323
  INST(Kaddd            , VexRvm             , V(660F00,4A,_,1,1,_,_,_  ), 0                         , 65 , 0  , 1454 , 86 , 65 ), // #324
  INST(Kaddq            , VexRvm             , V(000F00,4A,_,1,1,_,_,_  ), 0                         , 66 , 0  , 1460 , 86 , 65 ), // #325
  INST(Kaddw            , VexRvm             , V(000F00,4A,_,1,0,_,_,_  ), 0                         , 67 , 0  , 1466 , 86 , 64 ), // #326
  INST(Kandb            , VexRvm             , V(660F00,41,_,1,0,_,_,_  ), 0                         , 64 , 0  , 1472 , 86 , 64 ), // #327
  INST(Kandd            , VexRvm             , V(660F00,41,_,1,1,_,_,_  ), 0                         , 65 , 0  , 1478 , 86 , 65 ), // #328
  INST(Kandnb           , VexRvm             , V(660F00,42,_,1,0,_,_,_  ), 0                         , 64 , 0  , 1484 , 86 , 64 ), // #329
  INST(Kandnd           , VexRvm             , V(660F00,42,_,1,1,_,_,_  ), 0                         , 65 , 0  , 1491 , 86 , 65 ), // #330
  INST(Kandnq           , VexRvm             , V(000F00,42,_,1,1,_,_,_  ), 0                         , 66 , 0  , 1498 , 86 , 65 ), // #331
  INST(Kandnw           , VexRvm             , V(000F00,42,_,1,0,_,_,_  ), 0                         , 67 , 0  , 1505 , 86 , 66 ), // #332
  INST(Kandq            , VexRvm             , V(000F00,41,_,1,1,_,_,_  ), 0                         , 66 , 0  , 1512 , 86 , 65 ), // #333
  INST(Kandw            , VexRvm             , V(000F00,41,_,1,0,_,_,_  ), 0                         , 67 , 0  , 1518 , 86 , 66 ), // #334
  INST(Kmovb            , VexKmov            , V(660F00,90,_,0,0,_,_,_  ), V(660F00,92,_,0,0,_,_,_  ), 68 , 36 , 1524 , 87 , 64 ), // #335
  INST(Kmovd            , VexKmov            , V(660F00,90,_,0,1,_,_,_  ), V(F20F00,92,_,0,0,_,_,_  ), 69 , 37 , 8093 , 88 , 65 ), // #336
  INST(Kmovq            , VexKmov            , V(000F00,90,_,0,1,_,_,_  ), V(F20F00,92,_,0,1,_,_,_  ), 70 , 38 , 8104 , 89 , 65 ), // #337
  INST(Kmovw            , VexKmov            , V(000F00,90,_,0,0,_,_,_  ), V(000F00,92,_,0,0,_,_,_  ), 71 , 39 , 1530 , 90 , 66 ), // #338
  INST(Knotb            , VexRm              , V(660F00,44,_,0,0,_,_,_  ), 0                         , 68 , 0  , 1536 , 91 , 64 ), // #339
  INST(Knotd            , VexRm              , V(660F00,44,_,0,1,_,_,_  ), 0                         , 69 , 0  , 1542 , 91 , 65 ), // #340
  INST(Knotq            , VexRm              , V(000F00,44,_,0,1,_,_,_  ), 0                         , 70 , 0  , 1548 , 91 , 65 ), // #341
  INST(Knotw            , VexRm              , V(000F00,44,_,0,0,_,_,_  ), 0                         , 71 , 0  , 1554 , 91 , 66 ), // #342
  INST(Korb             , VexRvm             , V(660F00,45,_,1,0,_,_,_  ), 0                         , 64 , 0  , 1560 , 86 , 64 ), // #343
  INST(Kord             , VexRvm             , V(660F00,45,_,1,1,_,_,_  ), 0                         , 65 , 0  , 1565 , 86 , 65 ), // #344
  INST(Korq             , VexRvm             , V(000F00,45,_,1,1,_,_,_  ), 0                         , 66 , 0  , 1570 , 86 , 65 ), // #345
  INST(Kortestb         , VexRm              , V(660F00,98,_,0,0,_,_,_  ), 0                         , 68 , 0  , 1575 , 91 , 67 ), // #346
  INST(Kortestd         , VexRm              , V(660F00,98,_,0,1,_,_,_  ), 0                         , 69 , 0  , 1584 , 91 , 68 ), // #347
  INST(Kortestq         , VexRm              , V(000F00,98,_,0,1,_,_,_  ), 0                         , 70 , 0  , 1593 , 91 , 68 ), // #348
  INST(Kortestw         , VexRm              , V(000F00,98,_,0,0,_,_,_  ), 0                         , 71 , 0  , 1602 , 91 , 69 ), // #349
  INST(Korw             , VexRvm             , V(000F00,45,_,1,0,_,_,_  ), 0                         , 67 , 0  , 1611 , 86 , 66 ), // #350
  INST(Kshiftlb         , VexRmi             , V(660F3A,32,_,0,0,_,_,_  ), 0                         , 72 , 0  , 1616 , 92 , 64 ), // #351
  INST(Kshiftld         , VexRmi             , V(660F3A,33,_,0,0,_,_,_  ), 0                         , 72 , 0  , 1625 , 92 , 65 ), // #352
  INST(Kshiftlq         , VexRmi             , V(660F3A,33,_,0,1,_,_,_  ), 0                         , 73 , 0  , 1634 , 92 , 65 ), // #353
  INST(Kshiftlw         , VexRmi             , V(660F3A,32,_,0,1,_,_,_  ), 0                         , 73 , 0  , 1643 , 92 , 66 ), // #354
  INST(Kshiftrb         , VexRmi             , V(660F3A,30,_,0,0,_,_,_  ), 0                         , 72 , 0  , 1652 , 92 , 64 ), // #355
  INST(Kshiftrd         , VexRmi             , V(660F3A,31,_,0,0,_,_,_  ), 0                         , 72 , 0  , 1661 , 92 , 65 ), // #356
  INST(Kshiftrq         , VexRmi             , V(660F3A,31,_,0,1,_,_,_  ), 0                         , 73 , 0  , 1670 , 92 , 65 ), // #357
  INST(Kshiftrw         , VexRmi             , V(660F3A,30,_,0,1,_,_,_  ), 0                         , 73 , 0  , 1679 , 92 , 66 ), // #358
  INST(Ktestb           , VexRm              , V(660F00,99,_,0,0,_,_,_  ), 0                         , 68 , 0  , 1688 , 91 , 67 ), // #359
  INST(Ktestd           , VexRm              , V(660F00,99,_,0,1,_,_,_  ), 0                         , 69 , 0  , 1695 , 91 , 68 ), // #360
  INST(Ktestq           , VexRm              , V(000F00,99,_,0,1,_,_,_  ), 0                         , 70 , 0  , 1702 , 91 , 68 ), // #361
  INST(Ktestw           , VexRm              , V(000F00,99,_,0,0,_,_,_  ), 0                         , 71 , 0  , 1709 , 91 , 67 ), // #362
  INST(Kunpckbw         , VexRvm             , V(660F00,4B,_,1,0,_,_,_  ), 0                         , 64 , 0  , 1716 , 86 , 66 ), // #363
  INST(Kunpckdq         , VexRvm             , V(000F00,4B,_,1,1,_,_,_  ), 0                         , 66 , 0  , 1725 , 86 , 65 ), // #364
  INST(Kunpckwd         , VexRvm             , V(000F00,4B,_,1,0,_,_,_  ), 0                         , 67 , 0  , 1734 , 86 , 65 ), // #365
  INST(Kxnorb           , VexRvm             , V(660F00,46,_,1,0,_,_,_  ), 0                         , 64 , 0  , 1743 , 86 , 64 ), // #366
  INST(Kxnord           , VexRvm             , V(660F00,46,_,1,1,_,_,_  ), 0                         , 65 , 0  , 1750 , 86 , 65 ), // #367
  INST(Kxnorq           , VexRvm             , V(000F00,46,_,1,1,_,_,_  ), 0                         , 66 , 0  , 1757 , 86 , 65 ), // #368
  INST(Kxnorw           , VexRvm             , V(000F00,46,_,1,0,_,_,_  ), 0                         , 67 , 0  , 1764 , 86 , 66 ), // #369
  INST(Kxorb            , VexRvm             , V(660F00,47,_,1,0,_,_,_  ), 0                         , 64 , 0  , 1771 , 86 , 64 ), // #370
  INST(Kxord            , VexRvm             , V(660F00,47,_,1,1,_,_,_  ), 0                         , 65 , 0  , 1777 , 86 , 65 ), // #371
  INST(Kxorq            , VexRvm             , V(000F00,47,_,1,1,_,_,_  ), 0                         , 66 , 0  , 1783 , 86 , 65 ), // #372
  INST(Kxorw            , VexRvm             , V(000F00,47,_,1,0,_,_,_  ), 0                         , 67 , 0  , 1789 , 86 , 66 ), // #373
  INST(Lahf             , X86Op              , O(000000,9F,_,_,_,_,_,_  ), 0                         , 0  , 0  , 1795 , 93 , 70 ), // #374
  INST(Lar              , X86Rm              , O(000F00,02,_,_,_,_,_,_  ), 0                         , 4  , 0  , 1800 , 94 , 10 ), // #375
  INST(Lddqu            , ExtRm              , O(F20F00,F0,_,_,_,_,_,_  ), 0                         , 5  , 0  , 6086 , 95 , 6  ), // #376
  INST(Ldmxcsr          , X86M_Only          , O(000F00,AE,2,_,_,_,_,_  ), 0                         , 74 , 0  , 6093 , 96 , 5  ), // #377
  INST(Lds              , X86Rm              , O(000000,C5,_,_,_,_,_,_  ), 0                         , 0  , 0  , 1804 , 97 , 0  ), // #378
  INST(Ldtilecfg        , AmxCfg             , V(000F38,49,_,0,0,_,_,_  ), 0                         , 10 , 0  , 1808 , 98 , 71 ), // #379
  INST(Lea              , X86Lea             , O(000000,8D,_,_,x,_,_,_  ), 0                         , 0  , 0  , 1818 , 99 , 0  ), // #380
  INST(Leave            , X86Op              , O(000000,C9,_,_,_,_,_,_  ), 0                         , 0  , 0  , 1822 , 30 , 0  ), // #381
  INST(Les              , X86Rm              , O(000000,C4,_,_,_,_,_,_  ), 0                         , 0  , 0  , 1828 , 97 , 0  ), // #382
  INST(Lfence           , X86Fence           , O(000F00,AE,5,_,_,_,_,_  ), 0                         , 75 , 0  , 1832 , 30 , 4  ), // #383
  INST(Lfs              , X86Rm              , O(000F00,B4,_,_,_,_,_,_  ), 0                         , 4  , 0  , 1839 , 100, 0  ), // #384
  INST(Lgdt             , X86M_Only          , O(000F00,01,2,_,_,_,_,_  ), 0                         , 74 , 0  , 1843 , 31 , 0  ), // #385
  INST(Lgs              , X86Rm              , O(000F00,B5,_,_,_,_,_,_  ), 0                         , 4  , 0  , 1848 , 100, 0  ), // #386
  INST(Lidt             , X86M_Only          , O(000F00,01,3,_,_,_,_,_  ), 0                         , 76 , 0  , 1852 , 31 , 0  ), // #387
  INST(Lldt             , X86M_NoSize        , O(000F00,00,2,_,_,_,_,_  ), 0                         , 74 , 0  , 1857 , 101, 0  ), // #388
  INST(Llwpcb           , VexR_Wx            , V(XOP_M9,12,0,0,x,_,_,_  ), 0                         , 77 , 0  , 1862 , 102, 72 ), // #389
  INST(Lmsw             , X86M_NoSize        , O(000F00,01,6,_,_,_,_,_  ), 0                         , 78 , 0  , 1869 , 101, 0  ), // #390
  INST(Lods             , X86StrRm           , O(000000,AC,_,_,_,_,_,_  ), 0                         , 0  , 0  , 1874 , 103, 73 ), // #391
  INST(Loop             , X86JecxzLoop       , 0                         , O(000000,E2,_,_,_,_,_,_  ), 0  , 40 , 1879 , 104, 0  ), // #392
  INST(Loope            , X86JecxzLoop       , 0                         , O(000000,E1,_,_,_,_,_,_  ), 0  , 41 , 1884 , 104, 59 ), // #393
  INST(Loopne           , X86JecxzLoop       , 0                         , O(000000,E0,_,_,_,_,_,_  ), 0  , 42 , 1890 , 104, 59 ), // #394
  INST(Lsl              , X86Rm              , O(000F00,03,_,_,_,_,_,_  ), 0                         , 4  , 0  , 1897 , 105, 10 ), // #395
  INST(Lss              , X86Rm              , O(000F00,B2,_,_,_,_,_,_  ), 0                         , 4  , 0  , 6584 , 100, 0  ), // #396
  INST(Ltr              , X86M_NoSize        , O(000F00,00,3,_,_,_,_,_  ), 0                         , 76 , 0  , 1901 , 101, 0  ), // #397
  INST(Lwpins           , VexVmi4_Wx         , V(XOP_MA,12,0,0,x,_,_,_  ), 0                         , 79 , 0  , 1905 , 106, 72 ), // #398
  INST(Lwpval           , VexVmi4_Wx         , V(XOP_MA,12,1,0,x,_,_,_  ), 0                         , 80 , 0  , 1912 , 106, 72 ), // #399
  INST(Lzcnt            , X86Rm_Raw66H       , O(F30F00,BD,_,_,x,_,_,_  ), 0                         , 6  , 0  , 1919 , 22 , 74 ), // #400
  INST(Maskmovdqu       , ExtRm_ZDI          , O(660F00,57,_,_,_,_,_,_  ), 0                         , 3  , 0  , 6102 , 107, 4  ), // #401
  INST(Maskmovq         , ExtRm_ZDI          , O(000F00,F7,_,_,_,_,_,_  ), 0                         , 4  , 0  , 8101 , 108, 75 ), // #402
  INST(Maxpd            , ExtRm              , O(660F00,5F,_,_,_,_,_,_  ), 0                         , 3  , 0  , 6136 , 5  , 4  ), // #403
  INST(Maxps            , ExtRm              , O(000F00,5F,_,_,_,_,_,_  ), 0                         , 4  , 0  , 6143 , 5  , 5  ), // #404
  INST(Maxsd            , ExtRm              , O(F20F00,5F,_,_,_,_,_,_  ), 0                         , 5  , 0  , 8120 , 6  , 4  ), // #405
  INST(Maxss            , ExtRm              , O(F30F00,5F,_,_,_,_,_,_  ), 0                         , 6  , 0  , 6157 , 7  , 5  ), // #406
  INST(Mcommit          , X86Op              , O(F30F01,FA,_,_,_,_,_,_  ), 0                         , 81 , 0  , 1925 , 30 , 76 ), // #407
  INST(Mfence           , X86Fence           , O(000F00,AE,6,_,_,_,_,_  ), 0                         , 78 , 0  , 1933 , 30 , 4  ), // #408
  INST(Minpd            , ExtRm              , O(660F00,5D,_,_,_,_,_,_  ), 0                         , 3  , 0  , 6186 , 5  , 4  ), // #409
  INST(Minps            , ExtRm              , O(000F00,5D,_,_,_,_,_,_  ), 0                         , 4  , 0  , 6193 , 5  , 5  ), // #410
  INST(Minsd            , ExtRm              , O(F20F00,5D,_,_,_,_,_,_  ), 0                         , 5  , 0  , 8184 , 6  , 4  ), // #411
  INST(Minss            , ExtRm              , O(F30F00,5D,_,_,_,_,_,_  ), 0                         , 6  , 0  , 6207 , 7  , 5  ), // #412
  INST(Monitor          , X86Op              , O(000F01,C8,_,_,_,_,_,_  ), 0                         , 21 , 0  , 3199 , 109, 77 ), // #413
  INST(Monitorx         , X86Op              , O(000F01,FA,_,_,_,_,_,_  ), 0                         , 21 , 0  , 1940 , 109, 78 ), // #414
  INST(Mov              , X86Mov             , 0                         , 0                         , 0  , 0  , 138  , 110, 0  ), // #415
  INST(Movabs           , X86Movabs          , 0                         , 0                         , 0  , 0  , 1949 , 111, 0  ), // #416
  INST(Movapd           , ExtMov             , O(660F00,28,_,_,_,_,_,_  ), O(660F00,29,_,_,_,_,_,_  ), 3  , 43 , 6238 , 112, 4  ), // #417
  INST(Movaps           , ExtMov             , O(000F00,28,_,_,_,_,_,_  ), O(000F00,29,_,_,_,_,_,_  ), 4  , 44 , 6246 , 112, 5  ), // #418
  INST(Movbe            , ExtMovbe           , O(000F38,F0,_,_,x,_,_,_  ), O(000F38,F1,_,_,x,_,_,_  ), 82 , 45 , 651  , 113, 79 ), // #419
  INST(Movd             , ExtMovd            , O(000F00,6E,_,_,_,_,_,_  ), O(000F00,7E,_,_,_,_,_,_  ), 4  , 46 , 8094 , 114, 80 ), // #420
  INST(Movddup          , ExtMov             , O(F20F00,12,_,_,_,_,_,_  ), 0                         , 5  , 0  , 6260 , 6  , 6  ), // #421
  INST(Movdir64b        , X86EnqcmdMovdir64b , O(660F38,F8,_,_,_,_,_,_  ), 0                         , 2  , 0  , 1956 , 115, 81 ), // #422
  INST(Movdiri          , X86MovntiMovdiri   , O(000F38,F9,_,_,_,_,_,_  ), 0                         , 82 , 0  , 1966 , 116, 82 ), // #423
  INST(Movdq2q          , ExtMov             , O(F20F00,D6,_,_,_,_,_,_  ), 0                         , 5  , 0  , 1974 , 117, 4  ), // #424
  INST(Movdqa           , ExtMov             , O(660F00,6F,_,_,_,_,_,_  ), O(660F00,7F,_,_,_,_,_,_  ), 3  , 47 , 6269 , 112, 4  ), // #425
  INST(Movdqu           , ExtMov             , O(F30F00,6F,_,_,_,_,_,_  ), O(F30F00,7F,_,_,_,_,_,_  ), 6  , 48 , 6106 , 112, 4  ), // #426
  INST(Movhlps          , ExtMov             , O(000F00,12,_,_,_,_,_,_  ), 0                         , 4  , 0  , 6344 , 118, 5  ), // #427
  INST(Movhpd           , ExtMov             , O(660F00,16,_,_,_,_,_,_  ), O(660F00,17,_,_,_,_,_,_  ), 3  , 49 , 6353 , 119, 4  ), // #428
  INST(Movhps           , ExtMov             , O(000F00,16,_,_,_,_,_,_  ), O(000F00,17,_,_,_,_,_,_  ), 4  , 50 , 6361 , 119, 5  ), // #429
  INST(Movlhps          , ExtMov             , O(000F00,16,_,_,_,_,_,_  ), 0                         , 4  , 0  , 6369 , 118, 5  ), // #430
  INST(Movlpd           , ExtMov             , O(660F00,12,_,_,_,_,_,_  ), O(660F00,13,_,_,_,_,_,_  ), 3  , 51 , 6378 , 119, 4  ), // #431
  INST(Movlps           , ExtMov             , O(000F00,12,_,_,_,_,_,_  ), O(000F00,13,_,_,_,_,_,_  ), 4  , 52 , 6386 , 119, 5  ), // #432
  INST(Movmskpd         , ExtMov             , O(660F00,50,_,_,_,_,_,_  ), 0                         , 3  , 0  , 6394 , 120, 4  ), // #433
  INST(Movmskps         , ExtMov             , O(000F00,50,_,_,_,_,_,_  ), 0                         , 4  , 0  , 6404 , 120, 5  ), // #434
  INST(Movntdq          , ExtMov             , 0                         , O(660F00,E7,_,_,_,_,_,_  ), 0  , 53 , 6414 , 121, 4  ), // #435
  INST(Movntdqa         , ExtMov             , O(660F38,2A,_,_,_,_,_,_  ), 0                         , 2  , 0  , 6423 , 95 , 12 ), // #436
  INST(Movnti           , X86MovntiMovdiri   , O(000F00,C3,_,_,x,_,_,_  ), 0                         , 4  , 0  , 1982 , 116, 4  ), // #437
  INST(Movntpd          , ExtMov             , 0                         , O(660F00,2B,_,_,_,_,_,_  ), 0  , 54 , 6433 , 121, 4  ), // #438
  INST(Movntps          , ExtMov             , 0                         , O(000F00,2B,_,_,_,_,_,_  ), 0  , 55 , 6442 , 121, 5  ), // #439
  INST(Movntq           , ExtMov             , 0                         , O(000F00,E7,_,_,_,_,_,_  ), 0  , 56 , 1989 , 122, 75 ), // #440
  INST(Movntsd          , ExtMov             , 0                         , O(F20F00,2B,_,_,_,_,_,_  ), 0  , 57 , 1996 , 123, 48 ), // #441
  INST(Movntss          , ExtMov             , 0                         , O(F30F00,2B,_,_,_,_,_,_  ), 0  , 58 , 2004 , 124, 48 ), // #442
  INST(Movq             , ExtMovq            , O(000F00,6E,_,_,x,_,_,_  ), O(000F00,7E,_,_,x,_,_,_  ), 4  , 59 , 8105 , 125, 80 ), // #443
  INST(Movq2dq          , ExtRm              , O(F30F00,D6,_,_,_,_,_,_  ), 0                         , 6  , 0  , 2012 , 126, 4  ), // #444
  INST(Movs             , X86StrMm           , O(000000,A4,_,_,_,_,_,_  ), 0                         , 0  , 0  , 434  , 127, 73 ), // #445
  INST(Movsd            , ExtMov             , O(F20F00,10,_,_,_,_,_,_  ), O(F20F00,11,_,_,_,_,_,_  ), 5  , 60 , 6457 , 128, 4  ), // #446
  INST(Movshdup         , ExtRm              , O(F30F00,16,_,_,_,_,_,_  ), 0                         , 6  , 0  , 6464 , 5  , 6  ), // #447
  INST(Movsldup         , ExtRm              , O(F30F00,12,_,_,_,_,_,_  ), 0                         , 6  , 0  , 6474 , 5  , 6  ), // #448
  INST(Movss            , ExtMov             , O(F30F00,10,_,_,_,_,_,_  ), O(F30F00,11,_,_,_,_,_,_  ), 6  , 61 , 6484 , 129, 5  ), // #449
  INST(Movsx            , X86MovsxMovzx      , O(000F00,BE,_,_,x,_,_,_  ), 0                         , 4  , 0  , 2020 , 130, 0  ), // #450
  INST(Movsxd           , X86Rm              , O(000000,63,_,_,x,_,_,_  ), 0                         , 0  , 0  , 2026 , 131, 0  ), // #451
  INST(Movupd           , ExtMov             , O(660F00,10,_,_,_,_,_,_  ), O(660F00,11,_,_,_,_,_,_  ), 3  , 62 , 6491 , 112, 4  ), // #452
  INST(Movups           , ExtMov             , O(000F00,10,_,_,_,_,_,_  ), O(000F00,11,_,_,_,_,_,_  ), 4  , 63 , 6499 , 112, 5  ), // #453
  INST(Movzx            , X86MovsxMovzx      , O(000F00,B6,_,_,x,_,_,_  ), 0                         , 4  , 0  , 2033 , 130, 0  ), // #454
  INST(Mpsadbw          , ExtRmi             , O(660F3A,42,_,_,_,_,_,_  ), 0                         , 8  , 0  , 6507 , 8  , 12 ), // #455
  INST(Mul              , X86M_GPB_MulDiv    , O(000000,F6,4,_,x,_,_,_  ), 0                         , 9  , 0  , 823  , 53 , 1  ), // #456
  INST(Mulpd            , ExtRm              , O(660F00,59,_,_,_,_,_,_  ), 0                         , 3  , 0  , 6561 , 5  , 4  ), // #457
  INST(Mulps            , ExtRm              , O(000F00,59,_,_,_,_,_,_  ), 0                         , 4  , 0  , 6568 , 5  , 5  ), // #458
  INST(Mulsd            , ExtRm              , O(F20F00,59,_,_,_,_,_,_  ), 0                         , 5  , 0  , 6575 , 6  , 4  ), // #459
  INST(Mulss            , ExtRm              , O(F30F00,59,_,_,_,_,_,_  ), 0                         , 6  , 0  , 6582 , 7  , 5  ), // #460
  INST(Mulx             , VexRvm_ZDX_Wx      , V(F20F38,F6,_,0,x,_,_,_  ), 0                         , 83 , 0  , 2039 , 132, 83 ), // #461
  INST(Mwait            , X86Op              , O(000F01,C9,_,_,_,_,_,_  ), 0                         , 21 , 0  , 3208 , 133, 77 ), // #462
  INST(Mwaitx           , X86Op              , O(000F01,FB,_,_,_,_,_,_  ), 0                         , 21 , 0  , 2044 , 134, 78 ), // #463
  INST(Neg              , X86M_GPB           , O(000000,F6,3,_,x,_,_,_  ), 0                         , 84 , 0  , 2051 , 135, 1  ), // #464
  INST(Nop              , X86M_Nop           , O(000000,90,_,_,_,_,_,_  ), 0                         , 0  , 0  , 954  , 136, 0  ), // #465
  INST(Not              , X86M_GPB           , O(000000,F6,2,_,x,_,_,_  ), 0                         , 1  , 0  , 2055 , 135, 0  ), // #466
  INST(Or               , X86Arith           , O(000000,08,1,_,x,_,_,_  ), 0                         , 30 , 0  , 3204 , 137, 1  ), // #467
  INST(Orpd             , ExtRm              , O(660F00,56,_,_,_,_,_,_  ), 0                         , 3  , 0  , 10311, 11 , 4  ), // #468
  INST(Orps             , ExtRm              , O(000F00,56,_,_,_,_,_,_  ), 0                         , 4  , 0  , 10318, 11 , 5  ), // #469
  INST(Out              , X86Out             , O(000000,EE,_,_,_,_,_,_  ), O(000000,E6,_,_,_,_,_,_  ), 0  , 64 , 2059 , 138, 0  ), // #470
  INST(Outs             , X86Outs            , O(000000,6E,_,_,_,_,_,_  ), 0                         , 0  , 0  , 2063 , 139, 0  ), // #471
  INST(Pabsb            , ExtRm_P            , O(000F38,1C,_,_,_,_,_,_  ), 0                         , 82 , 0  , 6664 , 140, 84 ), // #472
  INST(Pabsd            , ExtRm_P            , O(000F38,1E,_,_,_,_,_,_  ), 0                         , 82 , 0  , 6671 , 140, 84 ), // #473
  INST(Pabsw            , ExtRm_P            , O(000F38,1D,_,_,_,_,_,_  ), 0                         , 82 , 0  , 6685 , 140, 84 ), // #474
  INST(Packssdw         , ExtRm_P            , O(000F00,6B,_,_,_,_,_,_  ), 0                         , 4  , 0  , 6692 , 140, 80 ), // #475
  INST(Packsswb         , ExtRm_P            , O(000F00,63,_,_,_,_,_,_  ), 0                         , 4  , 0  , 6702 , 140, 80 ), // #476
  INST(Packusdw         , ExtRm              , O(660F38,2B,_,_,_,_,_,_  ), 0                         , 2  , 0  , 6712 , 5  , 12 ), // #477
  INST(Packuswb         , ExtRm_P            , O(000F00,67,_,_,_,_,_,_  ), 0                         , 4  , 0  , 6722 , 140, 80 ), // #478
  INST(Paddb            , ExtRm_P            , O(000F00,FC,_,_,_,_,_,_  ), 0                         , 4  , 0  , 6732 , 140, 80 ), // #479
  INST(Paddd            , ExtRm_P            , O(000F00,FE,_,_,_,_,_,_  ), 0                         , 4  , 0  , 6739 , 140, 80 ), // #480
  INST(Paddq            , ExtRm_P            , O(000F00,D4,_,_,_,_,_,_  ), 0                         , 4  , 0  , 6746 , 140, 4  ), // #481
  INST(Paddsb           , ExtRm_P            , O(000F00,EC,_,_,_,_,_,_  ), 0                         , 4  , 0  , 6753 , 140, 80 ), // #482
  INST(Paddsw           , ExtRm_P            , O(000F00,ED,_,_,_,_,_,_  ), 0                         , 4  , 0  , 6761 , 140, 80 ), // #483
  INST(Paddusb          , ExtRm_P            , O(000F00,DC,_,_,_,_,_,_  ), 0                         , 4  , 0  , 6769 , 140, 80 ), // #484
  INST(Paddusw          , ExtRm_P            , O(000F00,DD,_,_,_,_,_,_  ), 0                         , 4  , 0  , 6778 , 140, 80 ), // #485
  INST(Paddw            , ExtRm_P            , O(000F00,FD,_,_,_,_,_,_  ), 0                         , 4  , 0  , 6787 , 140, 80 ), // #486
  INST(Palignr          , ExtRmi_P           , O(000F3A,0F,_,_,_,_,_,_  ), 0                         , 85 , 0  , 6794 , 141, 6  ), // #487
  INST(Pand             , ExtRm_P            , O(000F00,DB,_,_,_,_,_,_  ), 0                         , 4  , 0  , 6803 , 142, 80 ), // #488
  INST(Pandn            , ExtRm_P            , O(000F00,DF,_,_,_,_,_,_  ), 0                         , 4  , 0  , 6816 , 143, 80 ), // #489
  INST(Pause            , X86Op              , O(F30000,90,_,_,_,_,_,_  ), 0                         , 86 , 0  , 3168 , 30 , 0  ), // #490
  INST(Pavgb            , ExtRm_P            , O(000F00,E0,_,_,_,_,_,_  ), 0                         , 4  , 0  , 6846 , 140, 85 ), // #491
  INST(Pavgusb          , Ext3dNow           , O(000F0F,BF,_,_,_,_,_,_  ), 0                         , 87 , 0  , 2068 , 144, 50 ), // #492
  INST(Pavgw            , ExtRm_P            , O(000F00,E3,_,_,_,_,_,_  ), 0                         , 4  , 0  , 6853 , 140, 85 ), // #493
  INST(Pblendvb         , ExtRm_XMM0         , O(660F38,10,_,_,_,_,_,_  ), 0                         , 2  , 0  , 6869 , 15 , 12 ), // #494
  INST(Pblendw          , ExtRmi             , O(660F3A,0E,_,_,_,_,_,_  ), 0                         , 8  , 0  , 6879 , 8  , 12 ), // #495
  INST(Pclmulqdq        , ExtRmi             , O(660F3A,44,_,_,_,_,_,_  ), 0                         , 8  , 0  , 6972 , 8  , 86 ), // #496
  INST(Pcmpeqb          , ExtRm_P            , O(000F00,74,_,_,_,_,_,_  ), 0                         , 4  , 0  , 7004 , 143, 80 ), // #497
  INST(Pcmpeqd          , ExtRm_P            , O(000F00,76,_,_,_,_,_,_  ), 0                         , 4  , 0  , 7013 , 143, 80 ), // #498
  INST(Pcmpeqq          , ExtRm              , O(660F38,29,_,_,_,_,_,_  ), 0                         , 2  , 0  , 7022 , 145, 12 ), // #499
  INST(Pcmpeqw          , ExtRm_P            , O(000F00,75,_,_,_,_,_,_  ), 0                         , 4  , 0  , 7031 , 143, 80 ), // #500
  INST(Pcmpestri        , ExtRmi             , O(660F3A,61,_,_,_,_,_,_  ), 0                         , 8  , 0  , 7040 , 146, 87 ), // #501
  INST(Pcmpestrm        , ExtRmi             , O(660F3A,60,_,_,_,_,_,_  ), 0                         , 8  , 0  , 7051 , 147, 87 ), // #502
  INST(Pcmpgtb          , ExtRm_P            , O(000F00,64,_,_,_,_,_,_  ), 0                         , 4  , 0  , 7062 , 143, 80 ), // #503
  INST(Pcmpgtd          , ExtRm_P            , O(000F00,66,_,_,_,_,_,_  ), 0                         , 4  , 0  , 7071 , 143, 80 ), // #504
  INST(Pcmpgtq          , ExtRm              , O(660F38,37,_,_,_,_,_,_  ), 0                         , 2  , 0  , 7080 , 145, 43 ), // #505
  INST(Pcmpgtw          , ExtRm_P            , O(000F00,65,_,_,_,_,_,_  ), 0                         , 4  , 0  , 7089 , 143, 80 ), // #506
  INST(Pcmpistri        , ExtRmi             , O(660F3A,63,_,_,_,_,_,_  ), 0                         , 8  , 0  , 7098 , 148, 87 ), // #507
  INST(Pcmpistrm        , ExtRmi             , O(660F3A,62,_,_,_,_,_,_  ), 0                         , 8  , 0  , 7109 , 149, 87 ), // #508
  INST(Pconfig          , X86Op              , O(000F01,C5,_,_,_,_,_,_  ), 0                         , 21 , 0  , 2076 , 30 , 88 ), // #509
  INST(Pdep             , VexRvm_Wx          , V(F20F38,F5,_,0,x,_,_,_  ), 0                         , 83 , 0  , 2084 , 10 , 83 ), // #510
  INST(Pext             , VexRvm_Wx          , V(F30F38,F5,_,0,x,_,_,_  ), 0                         , 88 , 0  , 2089 , 10 , 83 ), // #511
  INST(Pextrb           , ExtExtract         , O(000F3A,14,_,_,_,_,_,_  ), 0                         , 85 , 0  , 7596 , 150, 12 ), // #512
  INST(Pextrd           , ExtExtract         , O(000F3A,16,_,_,_,_,_,_  ), 0                         , 85 , 0  , 7604 , 57 , 12 ), // #513
  INST(Pextrq           , ExtExtract         , O(000F3A,16,_,_,1,_,_,_  ), 0                         , 89 , 0  , 7612 , 151, 12 ), // #514
  INST(Pextrw           , ExtPextrw          , O(000F00,C5,_,_,_,_,_,_  ), O(000F3A,15,_,_,_,_,_,_  ), 4  , 65 , 7620 , 152, 89 ), // #515
  INST(Pf2id            , Ext3dNow           , O(000F0F,1D,_,_,_,_,_,_  ), 0                         , 87 , 0  , 2094 , 144, 50 ), // #516
  INST(Pf2iw            , Ext3dNow           , O(000F0F,1C,_,_,_,_,_,_  ), 0                         , 87 , 0  , 2100 , 144, 90 ), // #517
  INST(Pfacc            , Ext3dNow           , O(000F0F,AE,_,_,_,_,_,_  ), 0                         , 87 , 0  , 2106 , 144, 50 ), // #518
  INST(Pfadd            , Ext3dNow           , O(000F0F,9E,_,_,_,_,_,_  ), 0                         , 87 , 0  , 2112 , 144, 50 ), // #519
  INST(Pfcmpeq          , Ext3dNow           , O(000F0F,B0,_,_,_,_,_,_  ), 0                         , 87 , 0  , 2118 , 144, 50 ), // #520
  INST(Pfcmpge          , Ext3dNow           , O(000F0F,90,_,_,_,_,_,_  ), 0                         , 87 , 0  , 2126 , 144, 50 ), // #521
  INST(Pfcmpgt          , Ext3dNow           , O(000F0F,A0,_,_,_,_,_,_  ), 0                         , 87 , 0  , 2134 , 144, 50 ), // #522
  INST(Pfmax            , Ext3dNow           , O(000F0F,A4,_,_,_,_,_,_  ), 0                         , 87 , 0  , 2142 , 144, 50 ), // #523
  INST(Pfmin            , Ext3dNow           , O(000F0F,94,_,_,_,_,_,_  ), 0                         , 87 , 0  , 2148 , 144, 50 ), // #524
  INST(Pfmul            , Ext3dNow           , O(000F0F,B4,_,_,_,_,_,_  ), 0                         , 87 , 0  , 2154 , 144, 50 ), // #525
  INST(Pfnacc           , Ext3dNow           , O(000F0F,8A,_,_,_,_,_,_  ), 0                         , 87 , 0  , 2160 , 144, 90 ), // #526
  INST(Pfpnacc          , Ext3dNow           , O(000F0F,8E,_,_,_,_,_,_  ), 0                         , 87 , 0  , 2167 , 144, 90 ), // #527
  INST(Pfrcp            , Ext3dNow           , O(000F0F,96,_,_,_,_,_,_  ), 0                         , 87 , 0  , 2175 , 144, 50 ), // #528
  INST(Pfrcpit1         , Ext3dNow           , O(000F0F,A6,_,_,_,_,_,_  ), 0                         , 87 , 0  , 2181 , 144, 50 ), // #529
  INST(Pfrcpit2         , Ext3dNow           , O(000F0F,B6,_,_,_,_,_,_  ), 0                         , 87 , 0  , 2190 , 144, 50 ), // #530
  INST(Pfrcpv           , Ext3dNow           , O(000F0F,86,_,_,_,_,_,_  ), 0                         , 87 , 0  , 2199 , 144, 91 ), // #531
  INST(Pfrsqit1         , Ext3dNow           , O(000F0F,A7,_,_,_,_,_,_  ), 0                         , 87 , 0  , 2206 , 144, 50 ), // #532
  INST(Pfrsqrt          , Ext3dNow           , O(000F0F,97,_,_,_,_,_,_  ), 0                         , 87 , 0  , 2215 , 144, 50 ), // #533
  INST(Pfrsqrtv         , Ext3dNow           , O(000F0F,87,_,_,_,_,_,_  ), 0                         , 87 , 0  , 2223 , 144, 91 ), // #534
  INST(Pfsub            , Ext3dNow           , O(000F0F,9A,_,_,_,_,_,_  ), 0                         , 87 , 0  , 2232 , 144, 50 ), // #535
  INST(Pfsubr           , Ext3dNow           , O(000F0F,AA,_,_,_,_,_,_  ), 0                         , 87 , 0  , 2238 , 144, 50 ), // #536
  INST(Phaddd           , ExtRm_P            , O(000F38,02,_,_,_,_,_,_  ), 0                         , 82 , 0  , 7699 , 140, 84 ), // #537
  INST(Phaddsw          , ExtRm_P            , O(000F38,03,_,_,_,_,_,_  ), 0                         , 82 , 0  , 7716 , 140, 84 ), // #538
  INST(Phaddw           , ExtRm_P            , O(000F38,01,_,_,_,_,_,_  ), 0                         , 82 , 0  , 7785 , 140, 84 ), // #539
  INST(Phminposuw       , ExtRm              , O(660F38,41,_,_,_,_,_,_  ), 0                         , 2  , 0  , 7811 , 5  , 12 ), // #540
  INST(Phsubd           , ExtRm_P            , O(000F38,06,_,_,_,_,_,_  ), 0                         , 82 , 0  , 7832 , 140, 84 ), // #541
  INST(Phsubsw          , ExtRm_P            , O(000F38,07,_,_,_,_,_,_  ), 0                         , 82 , 0  , 7849 , 140, 84 ), // #542
  INST(Phsubw           , ExtRm_P            , O(000F38,05,_,_,_,_,_,_  ), 0                         , 82 , 0  , 7858 , 140, 84 ), // #543
  INST(Pi2fd            , Ext3dNow           , O(000F0F,0D,_,_,_,_,_,_  ), 0                         , 87 , 0  , 2245 , 144, 50 ), // #544
  INST(Pi2fw            , Ext3dNow           , O(000F0F,0C,_,_,_,_,_,_  ), 0                         , 87 , 0  , 2251 , 144, 90 ), // #545
  INST(Pinsrb           , ExtRmi             , O(660F3A,20,_,_,_,_,_,_  ), 0                         , 8  , 0  , 7875 , 153, 12 ), // #546
  INST(Pinsrd           , ExtRmi             , O(660F3A,22,_,_,_,_,_,_  ), 0                         , 8  , 0  , 7883 , 154, 12 ), // #547
  INST(Pinsrq           , ExtRmi             , O(660F3A,22,_,_,1,_,_,_  ), 0                         , 90 , 0  , 7891 , 155, 12 ), // #548
  INST(Pinsrw           , ExtRmi_P           , O(000F00,C4,_,_,_,_,_,_  ), 0                         , 4  , 0  , 7899 , 156, 85 ), // #549
  INST(Pmaddubsw        , ExtRm_P            , O(000F38,04,_,_,_,_,_,_  ), 0                         , 82 , 0  , 8069 , 140, 84 ), // #550
  INST(Pmaddwd          , ExtRm_P            , O(000F00,F5,_,_,_,_,_,_  ), 0                         , 4  , 0  , 8080 , 140, 80 ), // #551
  INST(Pmaxsb           , ExtRm              , O(660F38,3C,_,_,_,_,_,_  ), 0                         , 2  , 0  , 8111 , 11 , 12 ), // #552
  INST(Pmaxsd           , ExtRm              , O(660F38,3D,_,_,_,_,_,_  ), 0                         , 2  , 0  , 8119 , 11 , 12 ), // #553
  INST(Pmaxsw           , ExtRm_P            , O(000F00,EE,_,_,_,_,_,_  ), 0                         , 4  , 0  , 8135 , 142, 85 ), // #554
  INST(Pmaxub           , ExtRm_P            , O(000F00,DE,_,_,_,_,_,_  ), 0                         , 4  , 0  , 8143 , 142, 85 ), // #555
  INST(Pmaxud           , ExtRm              , O(660F38,3F,_,_,_,_,_,_  ), 0                         , 2  , 0  , 8151 , 11 , 12 ), // #556
  INST(Pmaxuw           , ExtRm              , O(660F38,3E,_,_,_,_,_,_  ), 0                         , 2  , 0  , 8167 , 11 , 12 ), // #557
  INST(Pminsb           , ExtRm              , O(660F38,38,_,_,_,_,_,_  ), 0                         , 2  , 0  , 8175 , 11 , 12 ), // #558
  INST(Pminsd           , ExtRm              , O(660F38,39,_,_,_,_,_,_  ), 0                         , 2  , 0  , 8183 , 11 , 12 ), // #559
  INST(Pminsw           , ExtRm_P            , O(000F00,EA,_,_,_,_,_,_  ), 0                         , 4  , 0  , 8199 , 142, 85 ), // #560
  INST(Pminub           , ExtRm_P            , O(000F00,DA,_,_,_,_,_,_  ), 0                         , 4  , 0  , 8207 , 142, 85 ), // #561
  INST(Pminud           , ExtRm              , O(660F38,3B,_,_,_,_,_,_  ), 0                         , 2  , 0  , 8215 , 11 , 12 ), // #562
  INST(Pminuw           , ExtRm              , O(660F38,3A,_,_,_,_,_,_  ), 0                         , 2  , 0  , 8231 , 11 , 12 ), // #563
  INST(Pmovmskb         , ExtRm_P            , O(000F00,D7,_,_,_,_,_,_  ), 0                         , 4  , 0  , 8309 , 157, 85 ), // #564
  INST(Pmovsxbd         , ExtRm              , O(660F38,21,_,_,_,_,_,_  ), 0                         , 2  , 0  , 8406 , 7  , 12 ), // #565
  INST(Pmovsxbq         , ExtRm              , O(660F38,22,_,_,_,_,_,_  ), 0                         , 2  , 0  , 8416 , 158, 12 ), // #566
  INST(Pmovsxbw         , ExtRm              , O(660F38,20,_,_,_,_,_,_  ), 0                         , 2  , 0  , 8426 , 6  , 12 ), // #567
  INST(Pmovsxdq         , ExtRm              , O(660F38,25,_,_,_,_,_,_  ), 0                         , 2  , 0  , 8436 , 6  , 12 ), // #568
  INST(Pmovsxwd         , ExtRm              , O(660F38,23,_,_,_,_,_,_  ), 0                         , 2  , 0  , 8446 , 6  , 12 ), // #569
  INST(Pmovsxwq         , ExtRm              , O(660F38,24,_,_,_,_,_,_  ), 0                         , 2  , 0  , 8456 , 7  , 12 ), // #570
  INST(Pmovzxbd         , ExtRm              , O(660F38,31,_,_,_,_,_,_  ), 0                         , 2  , 0  , 8543 , 7  , 12 ), // #571
  INST(Pmovzxbq         , ExtRm              , O(660F38,32,_,_,_,_,_,_  ), 0                         , 2  , 0  , 8553 , 158, 12 ), // #572
  INST(Pmovzxbw         , ExtRm              , O(660F38,30,_,_,_,_,_,_  ), 0                         , 2  , 0  , 8563 , 6  , 12 ), // #573
  INST(Pmovzxdq         , ExtRm              , O(660F38,35,_,_,_,_,_,_  ), 0                         , 2  , 0  , 8573 , 6  , 12 ), // #574
  INST(Pmovzxwd         , ExtRm              , O(660F38,33,_,_,_,_,_,_  ), 0                         , 2  , 0  , 8583 , 6  , 12 ), // #575
  INST(Pmovzxwq         , ExtRm              , O(660F38,34,_,_,_,_,_,_  ), 0                         , 2  , 0  , 8593 , 7  , 12 ), // #576
  INST(Pmuldq           , ExtRm              , O(660F38,28,_,_,_,_,_,_  ), 0                         , 2  , 0  , 8603 , 5  , 12 ), // #577
  INST(Pmulhrsw         , ExtRm_P            , O(000F38,0B,_,_,_,_,_,_  ), 0                         , 82 , 0  , 8611 , 140, 84 ), // #578
  INST(Pmulhrw          , Ext3dNow           , O(000F0F,B7,_,_,_,_,_,_  ), 0                         , 87 , 0  , 2257 , 144, 50 ), // #579
  INST(Pmulhuw          , ExtRm_P            , O(000F00,E4,_,_,_,_,_,_  ), 0                         , 4  , 0  , 8621 , 140, 85 ), // #580
  INST(Pmulhw           , ExtRm_P            , O(000F00,E5,_,_,_,_,_,_  ), 0                         , 4  , 0  , 8630 , 140, 80 ), // #581
  INST(Pmulld           , ExtRm              , O(660F38,40,_,_,_,_,_,_  ), 0                         , 2  , 0  , 8638 , 5  , 12 ), // #582
  INST(Pmullw           , ExtRm_P            , O(000F00,D5,_,_,_,_,_,_  ), 0                         , 4  , 0  , 8654 , 140, 80 ), // #583
  INST(Pmuludq          , ExtRm_P            , O(000F00,F4,_,_,_,_,_,_  ), 0                         , 4  , 0  , 8677 , 140, 4  ), // #584
  INST(Pop              , X86Pop             , O(000000,8F,0,_,_,_,_,_  ), O(000000,58,_,_,_,_,_,_  ), 0  , 66 , 2265 , 159, 0  ), // #585
  INST(Popa             , X86Op              , O(660000,61,_,_,_,_,_,_  ), 0                         , 19 , 0  , 2269 , 78 , 0  ), // #586
  INST(Popad            , X86Op              , O(000000,61,_,_,_,_,_,_  ), 0                         , 0  , 0  , 2274 , 78 , 0  ), // #587
  INST(Popcnt           , X86Rm_Raw66H       , O(F30F00,B8,_,_,x,_,_,_  ), 0                         , 6  , 0  , 2280 , 22 , 92 ), // #588
  INST(Popf             , X86Op              , O(660000,9D,_,_,_,_,_,_  ), 0                         , 19 , 0  , 2287 , 30 , 93 ), // #589
  INST(Popfd            , X86Op              , O(000000,9D,_,_,_,_,_,_  ), 0                         , 0  , 0  , 2292 , 78 , 93 ), // #590
  INST(Popfq            , X86Op              , O(000000,9D,_,_,_,_,_,_  ), 0                         , 0  , 0  , 2298 , 160, 93 ), // #591
  INST(Por              , ExtRm_P            , O(000F00,EB,_,_,_,_,_,_  ), 0                         , 4  , 0  , 8722 , 142, 80 ), // #592
  INST(Prefetch         , X86M_Only          , O(000F00,0D,0,_,_,_,_,_  ), 0                         , 4  , 0  , 2304 , 31 , 50 ), // #593
  INST(Prefetchnta      , X86M_Only          , O(000F00,18,0,_,_,_,_,_  ), 0                         , 4  , 0  , 2313 , 31 , 75 ), // #594
  INST(Prefetcht0       , X86M_Only          , O(000F00,18,1,_,_,_,_,_  ), 0                         , 28 , 0  , 2325 , 31 , 75 ), // #595
  INST(Prefetcht1       , X86M_Only          , O(000F00,18,2,_,_,_,_,_  ), 0                         , 74 , 0  , 2336 , 31 , 75 ), // #596
  INST(Prefetcht2       , X86M_Only          , O(000F00,18,3,_,_,_,_,_  ), 0                         , 76 , 0  , 2347 , 31 , 75 ), // #597
  INST(Prefetchw        , X86M_Only          , O(000F00,0D,1,_,_,_,_,_  ), 0                         , 28 , 0  , 2358 , 31 , 94 ), // #598
  INST(Prefetchwt1      , X86M_Only          , O(000F00,0D,2,_,_,_,_,_  ), 0                         , 74 , 0  , 2368 , 31 , 95 ), // #599
  INST(Psadbw           , ExtRm_P            , O(000F00,F6,_,_,_,_,_,_  ), 0                         , 4  , 0  , 4275 , 140, 85 ), // #600
  INST(Pshufb           , ExtRm_P            , O(000F38,00,_,_,_,_,_,_  ), 0                         , 82 , 0  , 9048 , 140, 84 ), // #601
  INST(Pshufd           , ExtRmi             , O(660F00,70,_,_,_,_,_,_  ), 0                         , 3  , 0  , 9069 , 8  , 4  ), // #602
  INST(Pshufhw          , ExtRmi             , O(F30F00,70,_,_,_,_,_,_  ), 0                         , 6  , 0  , 9077 , 8  , 4  ), // #603
  INST(Pshuflw          , ExtRmi             , O(F20F00,70,_,_,_,_,_,_  ), 0                         , 5  , 0  , 9086 , 8  , 4  ), // #604
  INST(Pshufw           , ExtRmi_P           , O(000F00,70,_,_,_,_,_,_  ), 0                         , 4  , 0  , 2380 , 161, 75 ), // #605
  INST(Psignb           , ExtRm_P            , O(000F38,08,_,_,_,_,_,_  ), 0                         , 82 , 0  , 9095 , 140, 84 ), // #606
  INST(Psignd           , ExtRm_P            , O(000F38,0A,_,_,_,_,_,_  ), 0                         , 82 , 0  , 9103 , 140, 84 ), // #607
  INST(Psignw           , ExtRm_P            , O(000F38,09,_,_,_,_,_,_  ), 0                         , 82 , 0  , 9111 , 140, 84 ), // #608
  INST(Pslld            , ExtRmRi_P          , O(000F00,F2,_,_,_,_,_,_  ), O(000F00,72,6,_,_,_,_,_  ), 4  , 67 , 9119 , 162, 80 ), // #609
  INST(Pslldq           , ExtRmRi            , 0                         , O(660F00,73,7,_,_,_,_,_  ), 0  , 68 , 9126 , 163, 4  ), // #610
  INST(Psllq            , ExtRmRi_P          , O(000F00,F3,_,_,_,_,_,_  ), O(000F00,73,6,_,_,_,_,_  ), 4  , 69 , 9134 , 162, 80 ), // #611
  INST(Psllw            , ExtRmRi_P          , O(000F00,F1,_,_,_,_,_,_  ), O(000F00,71,6,_,_,_,_,_  ), 4  , 70 , 9165 , 162, 80 ), // #612
  INST(Psmash           , X86Op              , O(F30F01,FF,_,_,_,_,_,_  ), 0                         , 81 , 0  , 2387 , 160, 96 ), // #613
  INST(Psrad            , ExtRmRi_P          , O(000F00,E2,_,_,_,_,_,_  ), O(000F00,72,4,_,_,_,_,_  ), 4  , 71 , 9172 , 162, 80 ), // #614
  INST(Psraw            , ExtRmRi_P          , O(000F00,E1,_,_,_,_,_,_  ), O(000F00,71,4,_,_,_,_,_  ), 4  , 72 , 9210 , 162, 80 ), // #615
  INST(Psrld            , ExtRmRi_P          , O(000F00,D2,_,_,_,_,_,_  ), O(000F00,72,2,_,_,_,_,_  ), 4  , 73 , 9217 , 162, 80 ), // #616
  INST(Psrldq           , ExtRmRi            , 0                         , O(660F00,73,3,_,_,_,_,_  ), 0  , 74 , 9224 , 163, 4  ), // #617
  INST(Psrlq            , ExtRmRi_P          , O(000F00,D3,_,_,_,_,_,_  ), O(000F00,73,2,_,_,_,_,_  ), 4  , 75 , 9232 , 162, 80 ), // #618
  INST(Psrlw            , ExtRmRi_P          , O(000F00,D1,_,_,_,_,_,_  ), O(000F00,71,2,_,_,_,_,_  ), 4  , 76 , 9263 , 162, 80 ), // #619
  INST(Psubb            , ExtRm_P            , O(000F00,F8,_,_,_,_,_,_  ), 0                         , 4  , 0  , 9270 , 143, 80 ), // #620
  INST(Psubd            , ExtRm_P            , O(000F00,FA,_,_,_,_,_,_  ), 0                         , 4  , 0  , 9277 , 143, 80 ), // #621
  INST(Psubq            , ExtRm_P            , O(000F00,FB,_,_,_,_,_,_  ), 0                         , 4  , 0  , 9284 , 143, 4  ), // #622
  INST(Psubsb           , ExtRm_P            , O(000F00,E8,_,_,_,_,_,_  ), 0                         , 4  , 0  , 9291 , 143, 80 ), // #623
  INST(Psubsw           , ExtRm_P            , O(000F00,E9,_,_,_,_,_,_  ), 0                         , 4  , 0  , 9299 , 143, 80 ), // #624
  INST(Psubusb          , ExtRm_P            , O(000F00,D8,_,_,_,_,_,_  ), 0                         , 4  , 0  , 9307 , 143, 80 ), // #625
  INST(Psubusw          , ExtRm_P            , O(000F00,D9,_,_,_,_,_,_  ), 0                         , 4  , 0  , 9316 , 143, 80 ), // #626
  INST(Psubw            , ExtRm_P            , O(000F00,F9,_,_,_,_,_,_  ), 0                         , 4  , 0  , 9325 , 143, 80 ), // #627
  INST(Pswapd           , Ext3dNow           , O(000F0F,BB,_,_,_,_,_,_  ), 0                         , 87 , 0  , 2394 , 144, 90 ), // #628
  INST(Ptest            , ExtRm              , O(660F38,17,_,_,_,_,_,_  ), 0                         , 2  , 0  , 9354 , 5  , 97 ), // #629
  INST(Ptwrite          , X86M               , O(F30F00,AE,4,_,_,_,_,_  ), 0                         , 91 , 0  , 2401 , 164, 98 ), // #630
  INST(Punpckhbw        , ExtRm_P            , O(000F00,68,_,_,_,_,_,_  ), 0                         , 4  , 0  , 9437 , 140, 80 ), // #631
  INST(Punpckhdq        , ExtRm_P            , O(000F00,6A,_,_,_,_,_,_  ), 0                         , 4  , 0  , 9448 , 140, 80 ), // #632
  INST(Punpckhqdq       , ExtRm              , O(660F00,6D,_,_,_,_,_,_  ), 0                         , 3  , 0  , 9459 , 5  , 4  ), // #633
  INST(Punpckhwd        , ExtRm_P            , O(000F00,69,_,_,_,_,_,_  ), 0                         , 4  , 0  , 9471 , 140, 80 ), // #634
  INST(Punpcklbw        , ExtRm_P            , O(000F00,60,_,_,_,_,_,_  ), 0                         , 4  , 0  , 9482 , 140, 80 ), // #635
  INST(Punpckldq        , ExtRm_P            , O(000F00,62,_,_,_,_,_,_  ), 0                         , 4  , 0  , 9493 , 140, 80 ), // #636
  INST(Punpcklqdq       , ExtRm              , O(660F00,6C,_,_,_,_,_,_  ), 0                         , 3  , 0  , 9504 , 5  , 4  ), // #637
  INST(Punpcklwd        , ExtRm_P            , O(000F00,61,_,_,_,_,_,_  ), 0                         , 4  , 0  , 9516 , 140, 80 ), // #638
  INST(Push             , X86Push            , O(000000,FF,6,_,_,_,_,_  ), O(000000,50,_,_,_,_,_,_  ), 31 , 77 , 2409 , 165, 0  ), // #639
  INST(Pusha            , X86Op              , O(660000,60,_,_,_,_,_,_  ), 0                         , 19 , 0  , 2414 , 78 , 0  ), // #640
  INST(Pushad           , X86Op              , O(000000,60,_,_,_,_,_,_  ), 0                         , 0  , 0  , 2420 , 78 , 0  ), // #641
  INST(Pushf            , X86Op              , O(660000,9C,_,_,_,_,_,_  ), 0                         , 19 , 0  , 2427 , 30 , 99 ), // #642
  INST(Pushfd           , X86Op              , O(000000,9C,_,_,_,_,_,_  ), 0                         , 0  , 0  , 2433 , 78 , 99 ), // #643
  INST(Pushfq           , X86Op              , O(000000,9C,_,_,_,_,_,_  ), 0                         , 0  , 0  , 2440 , 160, 99 ), // #644
  INST(Pvalidate        , X86Op              , O(F20F01,FF,_,_,_,_,_,_  ), 0                         , 92 , 0  , 2447 , 30 , 100), // #645
  INST(Pxor             , ExtRm_P            , O(000F00,EF,_,_,_,_,_,_  ), 0                         , 4  , 0  , 9527 , 143, 80 ), // #646
  INST(Rcl              , X86Rot             , O(000000,D0,2,_,x,_,_,_  ), 0                         , 1  , 0  , 2457 , 166, 101), // #647
  INST(Rcpps            , ExtRm              , O(000F00,53,_,_,_,_,_,_  ), 0                         , 4  , 0  , 9655 , 5  , 5  ), // #648
  INST(Rcpss            , ExtRm              , O(F30F00,53,_,_,_,_,_,_  ), 0                         , 6  , 0  , 9662 , 7  , 5  ), // #649
  INST(Rcr              , X86Rot             , O(000000,D0,3,_,x,_,_,_  ), 0                         , 84 , 0  , 2461 , 166, 101), // #650
  INST(Rdfsbase         , X86M               , O(F30F00,AE,0,_,x,_,_,_  ), 0                         , 6  , 0  , 2465 , 167, 102), // #651
  INST(Rdgsbase         , X86M               , O(F30F00,AE,1,_,x,_,_,_  ), 0                         , 93 , 0  , 2474 , 167, 102), // #652
  INST(Rdmsr            , X86Op              , O(000F00,32,_,_,_,_,_,_  ), 0                         , 4  , 0  , 2483 , 168, 103), // #653
  INST(Rdpid            , X86R_Native        , O(F30F00,C7,7,_,_,_,_,_  ), 0                         , 94 , 0  , 2489 , 169, 104), // #654
  INST(Rdpkru           , X86Op              , O(000F01,EE,_,_,_,_,_,_  ), 0                         , 21 , 0  , 2495 , 168, 105), // #655
  INST(Rdpmc            , X86Op              , O(000F00,33,_,_,_,_,_,_  ), 0                         , 4  , 0  , 2502 , 168, 0  ), // #656
  INST(Rdpru            , X86Op              , O(000F01,FD,_,_,_,_,_,_  ), 0                         , 21 , 0  , 2508 , 168, 106), // #657
  INST(Rdrand           , X86M               , O(000F00,C7,6,_,x,_,_,_  ), 0                         , 78 , 0  , 2514 , 23 , 107), // #658
  INST(Rdseed           , X86M               , O(000F00,C7,7,_,x,_,_,_  ), 0                         , 22 , 0  , 2521 , 23 , 108), // #659
  INST(Rdsspd           , X86M               , O(F30F00,1E,1,_,_,_,_,_  ), 0                         , 93 , 0  , 2528 , 73 , 54 ), // #660
  INST(Rdsspq           , X86M               , O(F30F00,1E,1,_,_,_,_,_  ), 0                         , 93 , 0  , 2535 , 74 , 54 ), // #661
  INST(Rdtsc            , X86Op              , O(000F00,31,_,_,_,_,_,_  ), 0                         , 4  , 0  , 2542 , 28 , 109), // #662
  INST(Rdtscp           , X86Op              , O(000F01,F9,_,_,_,_,_,_  ), 0                         , 21 , 0  , 2548 , 168, 110), // #663
  INST(Ret              , X86Ret             , O(000000,C2,_,_,_,_,_,_  ), 0                         , 0  , 0  , 3051 , 170, 0  ), // #664
  INST(Rmpadjust        , X86Op              , O(F30F01,FE,_,_,_,_,_,_  ), 0                         , 81 , 0  , 2555 , 160, 96 ), // #665
  INST(Rmpupdate        , X86Op              , O(F20F01,FE,_,_,_,_,_,_  ), 0                         , 92 , 0  , 2565 , 160, 96 ), // #666
  INST(Rol              , X86Rot             , O(000000,D0,0,_,x,_,_,_  ), 0                         , 0  , 0  , 2575 , 166, 111), // #667
  INST(Ror              , X86Rot             , O(000000,D0,1,_,x,_,_,_  ), 0                         , 30 , 0  , 2579 , 166, 111), // #668
  INST(Rorx             , VexRmi_Wx          , V(F20F3A,F0,_,0,x,_,_,_  ), 0                         , 95 , 0  , 2583 , 171, 83 ), // #669
  INST(Roundpd          , ExtRmi             , O(660F3A,09,_,_,_,_,_,_  ), 0                         , 8  , 0  , 9757 , 8  , 12 ), // #670
  INST(Roundps          , ExtRmi             , O(660F3A,08,_,_,_,_,_,_  ), 0                         , 8  , 0  , 9766 , 8  , 12 ), // #671
  INST(Roundsd          , ExtRmi             , O(660F3A,0B,_,_,_,_,_,_  ), 0                         , 8  , 0  , 9775 , 36 , 12 ), // #672
  INST(Roundss          , ExtRmi             , O(660F3A,0A,_,_,_,_,_,_  ), 0                         , 8  , 0  , 9784 , 37 , 12 ), // #673
  INST(Rsm              , X86Op              , O(000F00,AA,_,_,_,_,_,_  ), 0                         , 4  , 0  , 2588 , 78 , 1  ), // #674
  INST(Rsqrtps          , ExtRm              , O(000F00,52,_,_,_,_,_,_  ), 0                         , 4  , 0  , 9881 , 5  , 5  ), // #675
  INST(Rsqrtss          , ExtRm              , O(F30F00,52,_,_,_,_,_,_  ), 0                         , 6  , 0  , 9890 , 7  , 5  ), // #676
  INST(Rstorssp         , X86M               , O(F30F00,01,5,_,_,_,_,_  ), 0                         , 62 , 0  , 2592 , 32 , 24 ), // #677
  INST(Sahf             , X86Op              , O(000000,9E,_,_,_,_,_,_  ), 0                         , 0  , 0  , 2601 , 93 , 112), // #678
  INST(Sal              , X86Rot             , O(000000,D0,4,_,x,_,_,_  ), 0                         , 9  , 0  , 2606 , 166, 1  ), // #679
  INST(Sar              , X86Rot             , O(000000,D0,7,_,x,_,_,_  ), 0                         , 26 , 0  , 2610 , 166, 1  ), // #680
  INST(Sarx             , VexRmv_Wx          , V(F30F38,F7,_,0,x,_,_,_  ), 0                         , 88 , 0  , 2614 , 13 , 83 ), // #681
  INST(Saveprevssp      , X86Op              , O(F30F01,EA,_,_,_,_,_,_  ), 0                         , 81 , 0  , 2619 , 30 , 24 ), // #682
  INST(Sbb              , X86Arith           , O(000000,18,3,_,x,_,_,_  ), 0                         , 84 , 0  , 2631 , 172, 2  ), // #683
  INST(Scas             , X86StrRm           , O(000000,AE,_,_,_,_,_,_  ), 0                         , 0  , 0  , 2635 , 173, 36 ), // #684
  INST(Serialize        , X86Op              , O(000F01,E8,_,_,_,_,_,_  ), 0                         , 21 , 0  , 2640 , 30 , 113), // #685
  INST(Seta             , X86Set             , O(000F00,97,_,_,_,_,_,_  ), 0                         , 4  , 0  , 2650 , 174, 57 ), // #686
  INST(Setae            , X86Set             , O(000F00,93,_,_,_,_,_,_  ), 0                         , 4  , 0  , 2655 , 174, 58 ), // #687
  INST(Setb             , X86Set             , O(000F00,92,_,_,_,_,_,_  ), 0                         , 4  , 0  , 2661 , 174, 58 ), // #688
  INST(Setbe            , X86Set             , O(000F00,96,_,_,_,_,_,_  ), 0                         , 4  , 0  , 2666 , 174, 57 ), // #689
  INST(Setc             , X86Set             , O(000F00,92,_,_,_,_,_,_  ), 0                         , 4  , 0  , 2672 , 174, 58 ), // #690
  INST(Sete             , X86Set             , O(000F00,94,_,_,_,_,_,_  ), 0                         , 4  , 0  , 2677 , 174, 59 ), // #691
  INST(Setg             , X86Set             , O(000F00,9F,_,_,_,_,_,_  ), 0                         , 4  , 0  , 2682 , 174, 60 ), // #692
  INST(Setge            , X86Set             , O(000F00,9D,_,_,_,_,_,_  ), 0                         , 4  , 0  , 2687 , 174, 61 ), // #693
  INST(Setl             , X86Set             , O(000F00,9C,_,_,_,_,_,_  ), 0                         , 4  , 0  , 2693 , 174, 61 ), // #694
  INST(Setle            , X86Set             , O(000F00,9E,_,_,_,_,_,_  ), 0                         , 4  , 0  , 2698 , 174, 60 ), // #695
  INST(Setna            , X86Set             , O(000F00,96,_,_,_,_,_,_  ), 0                         , 4  , 0  , 2704 , 174, 57 ), // #696
  INST(Setnae           , X86Set             , O(000F00,92,_,_,_,_,_,_  ), 0                         , 4  , 0  , 2710 , 174, 58 ), // #697
  INST(Setnb            , X86Set             , O(000F00,93,_,_,_,_,_,_  ), 0                         , 4  , 0  , 2717 , 174, 58 ), // #698
  INST(Setnbe           , X86Set             , O(000F00,97,_,_,_,_,_,_  ), 0                         , 4  , 0  , 2723 , 174, 57 ), // #699
  INST(Setnc            , X86Set             , O(000F00,93,_,_,_,_,_,_  ), 0                         , 4  , 0  , 2730 , 174, 58 ), // #700
  INST(Setne            , X86Set             , O(000F00,95,_,_,_,_,_,_  ), 0                         , 4  , 0  , 2736 , 174, 59 ), // #701
  INST(Setng            , X86Set             , O(000F00,9E,_,_,_,_,_,_  ), 0                         , 4  , 0  , 2742 , 174, 60 ), // #702
  INST(Setnge           , X86Set             , O(000F00,9C,_,_,_,_,_,_  ), 0                         , 4  , 0  , 2748 , 174, 61 ), // #703
  INST(Setnl            , X86Set             , O(000F00,9D,_,_,_,_,_,_  ), 0                         , 4  , 0  , 2755 , 174, 61 ), // #704
  INST(Setnle           , X86Set             , O(000F00,9F,_,_,_,_,_,_  ), 0                         , 4  , 0  , 2761 , 174, 60 ), // #705
  INST(Setno            , X86Set             , O(000F00,91,_,_,_,_,_,_  ), 0                         , 4  , 0  , 2768 , 174, 55 ), // #706
  INST(Setnp            , X86Set             , O(000F00,9B,_,_,_,_,_,_  ), 0                         , 4  , 0  , 2774 , 174, 62 ), // #707
  INST(Setns            , X86Set             , O(000F00,99,_,_,_,_,_,_  ), 0                         , 4  , 0  , 2780 , 174, 63 ), // #708
  INST(Setnz            , X86Set             , O(000F00,95,_,_,_,_,_,_  ), 0                         , 4  , 0  , 2786 , 174, 59 ), // #709
  INST(Seto             , X86Set             , O(000F00,90,_,_,_,_,_,_  ), 0                         , 4  , 0  , 2792 , 174, 55 ), // #710
  INST(Setp             , X86Set             , O(000F00,9A,_,_,_,_,_,_  ), 0                         , 4  , 0  , 2797 , 174, 62 ), // #711
  INST(Setpe            , X86Set             , O(000F00,9A,_,_,_,_,_,_  ), 0                         , 4  , 0  , 2802 , 174, 62 ), // #712
  INST(Setpo            , X86Set             , O(000F00,9B,_,_,_,_,_,_  ), 0                         , 4  , 0  , 2808 , 174, 62 ), // #713
  INST(Sets             , X86Set             , O(000F00,98,_,_,_,_,_,_  ), 0                         , 4  , 0  , 2814 , 174, 63 ), // #714
  INST(Setssbsy         , X86Op              , O(F30F01,E8,_,_,_,_,_,_  ), 0                         , 81 , 0  , 2819 , 30 , 54 ), // #715
  INST(Setz             , X86Set             , O(000F00,94,_,_,_,_,_,_  ), 0                         , 4  , 0  , 2828 , 174, 59 ), // #716
  INST(Sfence           , X86Fence           , O(000F00,AE,7,_,_,_,_,_  ), 0                         , 22 , 0  , 2833 , 30 , 75 ), // #717
  INST(Sgdt             , X86M_Only          , O(000F00,01,0,_,_,_,_,_  ), 0                         , 4  , 0  , 2840 , 31 , 0  ), // #718
  INST(Sha1msg1         , ExtRm              , O(000F38,C9,_,_,_,_,_,_  ), 0                         , 82 , 0  , 2845 , 5  , 114), // #719
  INST(Sha1msg2         , ExtRm              , O(000F38,CA,_,_,_,_,_,_  ), 0                         , 82 , 0  , 2854 , 5  , 114), // #720
  INST(Sha1nexte        , ExtRm              , O(000F38,C8,_,_,_,_,_,_  ), 0                         , 82 , 0  , 2863 , 5  , 114), // #721
  INST(Sha1rnds4        , ExtRmi             , O(000F3A,CC,_,_,_,_,_,_  ), 0                         , 85 , 0  , 2873 , 8  , 114), // #722
  INST(Sha256msg1       , ExtRm              , O(000F38,CC,_,_,_,_,_,_  ), 0                         , 82 , 0  , 2883 , 5  , 114), // #723
  INST(Sha256msg2       , ExtRm              , O(000F38,CD,_,_,_,_,_,_  ), 0                         , 82 , 0  , 2894 , 5  , 114), // #724
  INST(Sha256rnds2      , ExtRm_XMM0         , O(000F38,CB,_,_,_,_,_,_  ), 0                         , 82 , 0  , 2905 , 15 , 114), // #725
  INST(Shl              , X86Rot             , O(000000,D0,4,_,x,_,_,_  ), 0                         , 9  , 0  , 2917 , 166, 1  ), // #726
  INST(Shld             , X86ShldShrd        , O(000F00,A4,_,_,x,_,_,_  ), 0                         , 4  , 0  , 8926 , 175, 1  ), // #727
  INST(Shlx             , VexRmv_Wx          , V(660F38,F7,_,0,x,_,_,_  ), 0                         , 96 , 0  , 2921 , 13 , 83 ), // #728
  INST(Shr              , X86Rot             , O(000000,D0,5,_,x,_,_,_  ), 0                         , 61 , 0  , 2926 , 166, 1  ), // #729
  INST(Shrd             , X86ShldShrd        , O(000F00,AC,_,_,x,_,_,_  ), 0                         , 4  , 0  , 2930 , 175, 1  ), // #730
  INST(Shrx             , VexRmv_Wx          , V(F20F38,F7,_,0,x,_,_,_  ), 0                         , 83 , 0  , 2935 , 13 , 83 ), // #731
  INST(Shufpd           , ExtRmi             , O(660F00,C6,_,_,_,_,_,_  ), 0                         , 3  , 0  , 10151, 8  , 4  ), // #732
  INST(Shufps           , ExtRmi             , O(000F00,C6,_,_,_,_,_,_  ), 0                         , 4  , 0  , 10159, 8  , 5  ), // #733
  INST(Sidt             , X86M_Only          , O(000F00,01,1,_,_,_,_,_  ), 0                         , 28 , 0  , 2940 , 31 , 0  ), // #734
  INST(Skinit           , X86Op_xAX          , O(000F01,DE,_,_,_,_,_,_  ), 0                         , 21 , 0  , 2945 , 51 , 115), // #735
  INST(Sldt             , X86M               , O(000F00,00,0,_,_,_,_,_  ), 0                         , 4  , 0  , 2952 , 176, 0  ), // #736
  INST(Slwpcb           , VexR_Wx            , V(XOP_M9,12,1,0,x,_,_,_  ), 0                         , 11 , 0  , 2957 , 102, 72 ), // #737
  INST(Smsw             , X86M               , O(000F00,01,4,_,_,_,_,_  ), 0                         , 97 , 0  , 2964 , 176, 0  ), // #738
  INST(Sqrtpd           , ExtRm              , O(660F00,51,_,_,_,_,_,_  ), 0                         , 3  , 0  , 10167, 5  , 4  ), // #739
  INST(Sqrtps           , ExtRm              , O(000F00,51,_,_,_,_,_,_  ), 0                         , 4  , 0  , 9882 , 5  , 5  ), // #740
  INST(Sqrtsd           , ExtRm              , O(F20F00,51,_,_,_,_,_,_  ), 0                         , 5  , 0  , 10183, 6  , 4  ), // #741
  INST(Sqrtss           , ExtRm              , O(F30F00,51,_,_,_,_,_,_  ), 0                         , 6  , 0  , 9891 , 7  , 5  ), // #742
  INST(Stac             , X86Op              , O(000F01,CB,_,_,_,_,_,_  ), 0                         , 21 , 0  , 2969 , 30 , 16 ), // #743
  INST(Stc              , X86Op              , O(000000,F9,_,_,_,_,_,_  ), 0                         , 0  , 0  , 2974 , 30 , 17 ), // #744
  INST(Std              , X86Op              , O(000000,FD,_,_,_,_,_,_  ), 0                         , 0  , 0  , 6909 , 30 , 18 ), // #745
  INST(Stgi             , X86Op              , O(000F01,DC,_,_,_,_,_,_  ), 0                         , 21 , 0  , 2978 , 30 , 115), // #746
  INST(Sti              , X86Op              , O(000000,FB,_,_,_,_,_,_  ), 0                         , 0  , 0  , 2983 , 30 , 23 ), // #747
  INST(Stmxcsr          , X86M_Only          , O(000F00,AE,3,_,_,_,_,_  ), 0                         , 76 , 0  , 10199, 96 , 5  ), // #748
  INST(Stos             , X86StrMr           , O(000000,AA,_,_,_,_,_,_  ), 0                         , 0  , 0  , 2987 , 177, 73 ), // #749
  INST(Str              , X86M               , O(000F00,00,1,_,_,_,_,_  ), 0                         , 28 , 0  , 2992 , 176, 0  ), // #750
  INST(Sttilecfg        , AmxCfg             , V(660F38,49,_,0,0,_,_,_  ), 0                         , 96 , 0  , 2996 , 98 , 71 ), // #751
  INST(Sub              , X86Arith           , O(000000,28,5,_,x,_,_,_  ), 0                         , 61 , 0  , 861  , 172, 1  ), // #752
  INST(Subpd            , ExtRm              , O(660F00,5C,_,_,_,_,_,_  ), 0                         , 3  , 0  , 4851 , 5  , 4  ), // #753
  INST(Subps            , ExtRm              , O(000F00,5C,_,_,_,_,_,_  ), 0                         , 4  , 0  , 4863 , 5  , 5  ), // #754
  INST(Subsd            , ExtRm              , O(F20F00,5C,_,_,_,_,_,_  ), 0                         , 5  , 0  , 5539 , 6  , 4  ), // #755
  INST(Subss            , ExtRm              , O(F30F00,5C,_,_,_,_,_,_  ), 0                         , 6  , 0  , 5549 , 7  , 5  ), // #756
  INST(Swapgs           , X86Op              , O(000F01,F8,_,_,_,_,_,_  ), 0                         , 21 , 0  , 3006 , 160, 0  ), // #757
  INST(Syscall          , X86Op              , O(000F00,05,_,_,_,_,_,_  ), 0                         , 4  , 0  , 3013 , 160, 0  ), // #758
  INST(Sysenter         , X86Op              , O(000F00,34,_,_,_,_,_,_  ), 0                         , 4  , 0  , 3021 , 30 , 0  ), // #759
  INST(Sysexit          , X86Op              , O(000F00,35,_,_,_,_,_,_  ), 0                         , 4  , 0  , 3030 , 30 , 0  ), // #760
  INST(Sysexit64        , X86Op              , O(000F00,35,_,_,_,_,_,_  ), 0                         , 4  , 0  , 3038 , 30 , 0  ), // #761
  INST(Sysret           , X86Op              , O(000F00,07,_,_,_,_,_,_  ), 0                         , 4  , 0  , 3048 , 160, 0  ), // #762
  INST(Sysret64         , X86Op              , O(000F00,07,_,_,_,_,_,_  ), 0                         , 4  , 0  , 3055 , 160, 0  ), // #763
  INST(T1mskc           , VexVm_Wx           , V(XOP_M9,01,7,0,x,_,_,_  ), 0                         , 98 , 0  , 3064 , 14 , 11 ), // #764
  INST(Tdpbf16ps        , AmxRmv             , V(F30F38,5C,_,0,0,_,_,_  ), 0                         , 88 , 0  , 3071 , 178, 116), // #765
  INST(Tdpbssd          , AmxRmv             , V(F20F38,5E,_,0,0,_,_,_  ), 0                         , 83 , 0  , 3081 , 178, 117), // #766
  INST(Tdpbsud          , AmxRmv             , V(F30F38,5E,_,0,0,_,_,_  ), 0                         , 88 , 0  , 3089 , 178, 117), // #767
  INST(Tdpbusd          , AmxRmv             , V(660F38,5E,_,0,0,_,_,_  ), 0                         , 96 , 0  , 3097 , 178, 117), // #768
  INST(Tdpbuud          , AmxRmv             , V(000F38,5E,_,0,0,_,_,_  ), 0                         , 10 , 0  , 3105 , 178, 117), // #769
  INST(Test             , X86Test            , O(000000,84,_,_,x,_,_,_  ), O(000000,F6,_,_,x,_,_,_  ), 0  , 78 , 9355 , 179, 1  ), // #770
  INST(Tileloadd        , AmxRm              , V(F20F38,4B,_,0,0,_,_,_  ), 0                         , 83 , 0  , 3113 , 180, 71 ), // #771
  INST(Tileloaddt1      , AmxRm              , V(660F38,4B,_,0,0,_,_,_  ), 0                         , 96 , 0  , 3123 , 180, 71 ), // #772
  INST(Tilerelease      , VexOpMod           , V(000F38,49,0,0,0,_,_,_  ), 0                         , 10 , 0  , 3135 , 181, 71 ), // #773
  INST(Tilestored       , AmxMr              , V(F30F38,4B,_,0,0,_,_,_  ), 0                         , 88 , 0  , 3147 , 182, 71 ), // #774
  INST(Tilezero         , AmxR               , V(F20F38,49,_,0,0,_,_,_  ), 0                         , 83 , 0  , 3158 , 183, 71 ), // #775
  INST(Tpause           , X86R32_EDX_EAX     , O(660F00,AE,6,_,_,_,_,_  ), 0                         , 25 , 0  , 3167 , 184, 118), // #776
  INST(Tzcnt            , X86Rm_Raw66H       , O(F30F00,BC,_,_,x,_,_,_  ), 0                         , 6  , 0  , 3174 , 22 , 9  ), // #777
  INST(Tzmsk            , VexVm_Wx           , V(XOP_M9,01,4,0,x,_,_,_  ), 0                         , 99 , 0  , 3180 , 14 , 11 ), // #778
  INST(Ucomisd          , ExtRm              , O(660F00,2E,_,_,_,_,_,_  ), 0                         , 3  , 0  , 10252, 6  , 40 ), // #779
  INST(Ucomiss          , ExtRm              , O(000F00,2E,_,_,_,_,_,_  ), 0                         , 4  , 0  , 10261, 7  , 41 ), // #780
  INST(Ud0              , X86M               , O(000F00,FF,_,_,_,_,_,_  ), 0                         , 4  , 0  , 3186 , 185, 0  ), // #781
  INST(Ud1              , X86M               , O(000F00,B9,_,_,_,_,_,_  ), 0                         , 4  , 0  , 3190 , 185, 0  ), // #782
  INST(Ud2              , X86Op              , O(000F00,0B,_,_,_,_,_,_  ), 0                         , 4  , 0  , 3194 , 30 , 0  ), // #783
  INST(Umonitor         , X86R_FromM         , O(F30F00,AE,6,_,_,_,_,_  ), 0                         , 24 , 0  , 3198 , 186, 119), // #784
  INST(Umwait           , X86R32_EDX_EAX     , O(F20F00,AE,6,_,_,_,_,_  ), 0                         , 100, 0  , 3207 , 184, 118), // #785
  INST(Unpckhpd         , ExtRm              , O(660F00,15,_,_,_,_,_,_  ), 0                         , 3  , 0  , 10270, 5  , 4  ), // #786
  INST(Unpckhps         , ExtRm              , O(000F00,15,_,_,_,_,_,_  ), 0                         , 4  , 0  , 10280, 5  , 5  ), // #787
  INST(Unpcklpd         , ExtRm              , O(660F00,14,_,_,_,_,_,_  ), 0                         , 3  , 0  , 10290, 5  , 4  ), // #788
  INST(Unpcklps         , ExtRm              , O(000F00,14,_,_,_,_,_,_  ), 0                         , 4  , 0  , 10300, 5  , 5  ), // #789
  INST(V4fmaddps        , VexRm_T1_4X        , E(F20F38,9A,_,2,_,0,2,T4X), 0                         , 101, 0  , 3214 , 187, 120), // #790
  INST(V4fmaddss        , VexRm_T1_4X        , E(F20F38,9B,_,2,_,0,2,T4X), 0                         , 101, 0  , 3224 , 188, 120), // #791
  INST(V4fnmaddps       , VexRm_T1_4X        , E(F20F38,AA,_,2,_,0,2,T4X), 0                         , 101, 0  , 3234 , 187, 120), // #792
  INST(V4fnmaddss       , VexRm_T1_4X        , E(F20F38,AB,_,2,_,0,2,T4X), 0                         , 101, 0  , 3245 , 188, 120), // #793
  INST(Vaddpd           , VexRvm_Lx          , V(660F00,58,_,x,I,1,4,FV ), 0                         , 102, 0  , 3256 , 189, 121), // #794
  INST(Vaddps           , VexRvm_Lx          , V(000F00,58,_,x,I,0,4,FV ), 0                         , 103, 0  , 3263 , 190, 121), // #795
  INST(Vaddsd           , VexRvm             , V(F20F00,58,_,I,I,1,3,T1S), 0                         , 104, 0  , 3270 , 191, 122), // #796
  INST(Vaddss           , VexRvm             , V(F30F00,58,_,I,I,0,2,T1S), 0                         , 105, 0  , 3277 , 192, 122), // #797
  INST(Vaddsubpd        , VexRvm_Lx          , V(660F00,D0,_,x,I,_,_,_  ), 0                         , 68 , 0  , 3284 , 193, 123), // #798
  INST(Vaddsubps        , VexRvm_Lx          , V(F20F00,D0,_,x,I,_,_,_  ), 0                         , 106, 0  , 3294 , 193, 123), // #799
  INST(Vaesdec          , VexRvm_Lx          , V(660F38,DE,_,x,I,_,4,FVM), 0                         , 107, 0  , 3304 , 194, 124), // #800
  INST(Vaesdeclast      , VexRvm_Lx          , V(660F38,DF,_,x,I,_,4,FVM), 0                         , 107, 0  , 3312 , 194, 124), // #801
  INST(Vaesenc          , VexRvm_Lx          , V(660F38,DC,_,x,I,_,4,FVM), 0                         , 107, 0  , 3324 , 194, 124), // #802
  INST(Vaesenclast      , VexRvm_Lx          , V(660F38,DD,_,x,I,_,4,FVM), 0                         , 107, 0  , 3332 , 194, 124), // #803
  INST(Vaesimc          , VexRm              , V(660F38,DB,_,0,I,_,_,_  ), 0                         , 96 , 0  , 3344 , 195, 125), // #804
  INST(Vaeskeygenassist , VexRmi             , V(660F3A,DF,_,0,I,_,_,_  ), 0                         , 72 , 0  , 3352 , 196, 125), // #805
  INST(Valignd          , VexRvmi_Lx         , E(660F3A,03,_,x,_,0,4,FV ), 0                         , 108, 0  , 3369 , 197, 126), // #806
  INST(Valignq          , VexRvmi_Lx         , E(660F3A,03,_,x,_,1,4,FV ), 0                         , 109, 0  , 3377 , 198, 126), // #807
  INST(Vandnpd          , VexRvm_Lx          , V(660F00,55,_,x,I,1,4,FV ), 0                         , 102, 0  , 3385 , 199, 127), // #808
  INST(Vandnps          , VexRvm_Lx          , V(000F00,55,_,x,I,0,4,FV ), 0                         , 103, 0  , 3393 , 200, 127), // #809
  INST(Vandpd           , VexRvm_Lx          , V(660F00,54,_,x,I,1,4,FV ), 0                         , 102, 0  , 3401 , 201, 127), // #810
  INST(Vandps           , VexRvm_Lx          , V(000F00,54,_,x,I,0,4,FV ), 0                         , 103, 0  , 3408 , 202, 127), // #811
  INST(Vblendmb         , VexRvm_Lx          , E(660F38,66,_,x,_,0,4,FVM), 0                         , 110, 0  , 3415 , 203, 128), // #812
  INST(Vblendmd         , VexRvm_Lx          , E(660F38,64,_,x,_,0,4,FV ), 0                         , 111, 0  , 3424 , 204, 126), // #813
  INST(Vblendmpd        , VexRvm_Lx          , E(660F38,65,_,x,_,1,4,FV ), 0                         , 112, 0  , 3433 , 205, 126), // #814
  INST(Vblendmps        , VexRvm_Lx          , E(660F38,65,_,x,_,0,4,FV ), 0                         , 111, 0  , 3443 , 204, 126), // #815
  INST(Vblendmq         , VexRvm_Lx          , E(660F38,64,_,x,_,1,4,FV ), 0                         , 112, 0  , 3453 , 205, 126), // #816
  INST(Vblendmw         , VexRvm_Lx          , E(660F38,66,_,x,_,1,4,FVM), 0                         , 113, 0  , 3462 , 203, 128), // #817
  INST(Vblendpd         , VexRvmi_Lx         , V(660F3A,0D,_,x,I,_,_,_  ), 0                         , 72 , 0  , 3471 , 206, 123), // #818
  INST(Vblendps         , VexRvmi_Lx         , V(660F3A,0C,_,x,I,_,_,_  ), 0                         , 72 , 0  , 3480 , 206, 123), // #819
  INST(Vblendvpd        , VexRvmr_Lx         , V(660F3A,4B,_,x,0,_,_,_  ), 0                         , 72 , 0  , 3489 , 207, 123), // #820
  INST(Vblendvps        , VexRvmr_Lx         , V(660F3A,4A,_,x,0,_,_,_  ), 0                         , 72 , 0  , 3499 , 207, 123), // #821
  INST(Vbroadcastf128   , VexRm              , V(660F38,1A,_,1,0,_,_,_  ), 0                         , 114, 0  , 3509 , 208, 123), // #822
  INST(Vbroadcastf32x2  , VexRm_Lx           , E(660F38,19,_,x,_,0,3,T2 ), 0                         , 115, 0  , 3524 , 209, 129), // #823
  INST(Vbroadcastf32x4  , VexRm_Lx           , E(660F38,1A,_,x,_,0,4,T4 ), 0                         , 116, 0  , 3540 , 210, 66 ), // #824
  INST(Vbroadcastf32x8  , VexRm              , E(660F38,1B,_,2,_,0,5,T8 ), 0                         , 117, 0  , 3556 , 211, 64 ), // #825
  INST(Vbroadcastf64x2  , VexRm_Lx           , E(660F38,1A,_,x,_,1,4,T2 ), 0                         , 118, 0  , 3572 , 210, 129), // #826
  INST(Vbroadcastf64x4  , VexRm              , E(660F38,1B,_,2,_,1,5,T4 ), 0                         , 119, 0  , 3588 , 211, 66 ), // #827
  INST(Vbroadcasti128   , VexRm              , V(660F38,5A,_,1,0,_,_,_  ), 0                         , 114, 0  , 3604 , 208, 130), // #828
  INST(Vbroadcasti32x2  , VexRm_Lx           , E(660F38,59,_,x,_,0,3,T2 ), 0                         , 115, 0  , 3619 , 212, 129), // #829
  INST(Vbroadcasti32x4  , VexRm_Lx           , E(660F38,5A,_,x,_,0,4,T4 ), 0                         , 116, 0  , 3635 , 210, 126), // #830
  INST(Vbroadcasti32x8  , VexRm              , E(660F38,5B,_,2,_,0,5,T8 ), 0                         , 117, 0  , 3651 , 211, 64 ), // #831
  INST(Vbroadcasti64x2  , VexRm_Lx           , E(660F38,5A,_,x,_,1,4,T2 ), 0                         , 118, 0  , 3667 , 210, 129), // #832
  INST(Vbroadcasti64x4  , VexRm              , E(660F38,5B,_,2,_,1,5,T4 ), 0                         , 119, 0  , 3683 , 211, 66 ), // #833
  INST(Vbroadcastsd     , VexRm_Lx           , V(660F38,19,_,x,0,1,3,T1S), 0                         , 120, 0  , 3699 , 213, 131), // #834
  INST(Vbroadcastss     , VexRm_Lx           , V(660F38,18,_,x,0,0,2,T1S), 0                         , 121, 0  , 3712 , 214, 131), // #835
  INST(Vcmppd           , VexRvmi_Lx         , V(660F00,C2,_,x,I,1,4,FV ), 0                         , 102, 0  , 3725 , 215, 121), // #836
  INST(Vcmpps           , VexRvmi_Lx         , V(000F00,C2,_,x,I,0,4,FV ), 0                         , 103, 0  , 3732 , 216, 121), // #837
  INST(Vcmpsd           , VexRvmi            , V(F20F00,C2,_,I,I,1,3,T1S), 0                         , 104, 0  , 3739 , 217, 122), // #838
  INST(Vcmpss           , VexRvmi            , V(F30F00,C2,_,I,I,0,2,T1S), 0                         , 105, 0  , 3746 , 218, 122), // #839
  INST(Vcomisd          , VexRm              , V(660F00,2F,_,I,I,1,3,T1S), 0                         , 122, 0  , 3753 , 219, 132), // #840
  INST(Vcomiss          , VexRm              , V(000F00,2F,_,I,I,0,2,T1S), 0                         , 123, 0  , 3761 , 220, 132), // #841
  INST(Vcompresspd      , VexMr_Lx           , E(660F38,8A,_,x,_,1,3,T1S), 0                         , 124, 0  , 3769 , 221, 126), // #842
  INST(Vcompressps      , VexMr_Lx           , E(660F38,8A,_,x,_,0,2,T1S), 0                         , 125, 0  , 3781 , 221, 126), // #843
  INST(Vcvtdq2pd        , VexRm_Lx           , V(F30F00,E6,_,x,I,0,3,HV ), 0                         , 126, 0  , 3793 , 222, 121), // #844
  INST(Vcvtdq2ps        , VexRm_Lx           , V(000F00,5B,_,x,I,0,4,FV ), 0                         , 103, 0  , 3803 , 223, 121), // #845
  INST(Vcvtne2ps2bf16   , VexRvm             , E(F20F38,72,_,_,_,0,_,_  ), 0                         , 127, 0  , 3813 , 204, 133), // #846
  INST(Vcvtneps2bf16    , VexRm              , E(F30F38,72,_,_,_,0,_,_  ), 0                         , 128, 0  , 3828 , 224, 133), // #847
  INST(Vcvtpd2dq        , VexRm_Lx           , V(F20F00,E6,_,x,I,1,4,FV ), 0                         , 129, 0  , 3842 , 225, 121), // #848
  INST(Vcvtpd2ps        , VexRm_Lx           , V(660F00,5A,_,x,I,1,4,FV ), 0                         , 102, 0  , 3852 , 225, 121), // #849
  INST(Vcvtpd2qq        , VexRm_Lx           , E(660F00,7B,_,x,_,1,4,FV ), 0                         , 130, 0  , 3862 , 226, 129), // #850
  INST(Vcvtpd2udq       , VexRm_Lx           , E(000F00,79,_,x,_,1,4,FV ), 0                         , 131, 0  , 3872 , 227, 126), // #851
  INST(Vcvtpd2uqq       , VexRm_Lx           , E(660F00,79,_,x,_,1,4,FV ), 0                         , 130, 0  , 3883 , 226, 129), // #852
  INST(Vcvtph2ps        , VexRm_Lx           , V(660F38,13,_,x,0,0,3,HVM), 0                         , 132, 0  , 3894 , 228, 134), // #853
  INST(Vcvtps2dq        , VexRm_Lx           , V(660F00,5B,_,x,I,0,4,FV ), 0                         , 133, 0  , 3904 , 223, 121), // #854
  INST(Vcvtps2pd        , VexRm_Lx           , V(000F00,5A,_,x,I,0,4,HV ), 0                         , 134, 0  , 3914 , 229, 121), // #855
  INST(Vcvtps2ph        , VexMri_Lx          , V(660F3A,1D,_,x,0,0,3,HVM), 0                         , 135, 0  , 3924 , 230, 134), // #856
  INST(Vcvtps2qq        , VexRm_Lx           , E(660F00,7B,_,x,_,0,3,HV ), 0                         , 136, 0  , 3934 , 231, 129), // #857
  INST(Vcvtps2udq       , VexRm_Lx           , E(000F00,79,_,x,_,0,4,FV ), 0                         , 137, 0  , 3944 , 232, 126), // #858
  INST(Vcvtps2uqq       , VexRm_Lx           , E(660F00,79,_,x,_,0,3,HV ), 0                         , 136, 0  , 3955 , 231, 129), // #859
  INST(Vcvtqq2pd        , VexRm_Lx           , E(F30F00,E6,_,x,_,1,4,FV ), 0                         , 138, 0  , 3966 , 226, 129), // #860
  INST(Vcvtqq2ps        , VexRm_Lx           , E(000F00,5B,_,x,_,1,4,FV ), 0                         , 131, 0  , 3976 , 227, 129), // #861
  INST(Vcvtsd2si        , VexRm_Wx           , V(F20F00,2D,_,I,x,x,3,T1F), 0                         , 139, 0  , 3986 , 233, 122), // #862
  INST(Vcvtsd2ss        , VexRvm             , V(F20F00,5A,_,I,I,1,3,T1S), 0                         , 104, 0  , 3996 , 191, 122), // #863
  INST(Vcvtsd2usi       , VexRm_Wx           , E(F20F00,79,_,I,_,x,3,T1F), 0                         , 140, 0  , 4006 , 234, 66 ), // #864
  INST(Vcvtsi2sd        , VexRvm_Wx          , V(F20F00,2A,_,I,x,x,2,T1W), 0                         , 141, 0  , 4017 , 235, 122), // #865
  INST(Vcvtsi2ss        , VexRvm_Wx          , V(F30F00,2A,_,I,x,x,2,T1W), 0                         , 142, 0  , 4027 , 235, 122), // #866
  INST(Vcvtss2sd        , VexRvm             , V(F30F00,5A,_,I,I,0,2,T1S), 0                         , 105, 0  , 4037 , 236, 122), // #867
  INST(Vcvtss2si        , VexRm_Wx           , V(F30F00,2D,_,I,x,x,2,T1F), 0                         , 143, 0  , 4047 , 237, 122), // #868
  INST(Vcvtss2usi       , VexRm_Wx           , E(F30F00,79,_,I,_,x,2,T1F), 0                         , 144, 0  , 4057 , 238, 66 ), // #869
  INST(Vcvttpd2dq       , VexRm_Lx           , V(660F00,E6,_,x,I,1,4,FV ), 0                         , 102, 0  , 4068 , 239, 121), // #870
  INST(Vcvttpd2qq       , VexRm_Lx           , E(660F00,7A,_,x,_,1,4,FV ), 0                         , 130, 0  , 4079 , 240, 126), // #871
  INST(Vcvttpd2udq      , VexRm_Lx           , E(000F00,78,_,x,_,1,4,FV ), 0                         , 131, 0  , 4090 , 241, 126), // #872
  INST(Vcvttpd2uqq      , VexRm_Lx           , E(660F00,78,_,x,_,1,4,FV ), 0                         , 130, 0  , 4102 , 240, 129), // #873
  INST(Vcvttps2dq       , VexRm_Lx           , V(F30F00,5B,_,x,I,0,4,FV ), 0                         , 145, 0  , 4114 , 242, 121), // #874
  INST(Vcvttps2qq       , VexRm_Lx           , E(660F00,7A,_,x,_,0,3,HV ), 0                         , 136, 0  , 4125 , 243, 129), // #875
  INST(Vcvttps2udq      , VexRm_Lx           , E(000F00,78,_,x,_,0,4,FV ), 0                         , 137, 0  , 4136 , 244, 126), // #876
  INST(Vcvttps2uqq      , VexRm_Lx           , E(660F00,78,_,x,_,0,3,HV ), 0                         , 136, 0  , 4148 , 243, 129), // #877
  INST(Vcvttsd2si       , VexRm_Wx           , V(F20F00,2C,_,I,x,x,3,T1F), 0                         , 139, 0  , 4160 , 245, 122), // #878
  INST(Vcvttsd2usi      , VexRm_Wx           , E(F20F00,78,_,I,_,x,3,T1F), 0                         , 140, 0  , 4171 , 246, 66 ), // #879
  INST(Vcvttss2si       , VexRm_Wx           , V(F30F00,2C,_,I,x,x,2,T1F), 0                         , 143, 0  , 4183 , 247, 122), // #880
  INST(Vcvttss2usi      , VexRm_Wx           , E(F30F00,78,_,I,_,x,2,T1F), 0                         , 144, 0  , 4194 , 248, 66 ), // #881
  INST(Vcvtudq2pd       , VexRm_Lx           , E(F30F00,7A,_,x,_,0,3,HV ), 0                         , 146, 0  , 4206 , 249, 126), // #882
  INST(Vcvtudq2ps       , VexRm_Lx           , E(F20F00,7A,_,x,_,0,4,FV ), 0                         , 147, 0  , 4217 , 232, 126), // #883
  INST(Vcvtuqq2pd       , VexRm_Lx           , E(F30F00,7A,_,x,_,1,4,FV ), 0                         , 138, 0  , 4228 , 226, 129), // #884
  INST(Vcvtuqq2ps       , VexRm_Lx           , E(F20F00,7A,_,x,_,1,4,FV ), 0                         , 148, 0  , 4239 , 227, 129), // #885
  INST(Vcvtusi2sd       , VexRvm_Wx          , E(F20F00,7B,_,I,_,x,2,T1W), 0                         , 149, 0  , 4250 , 250, 66 ), // #886
  INST(Vcvtusi2ss       , VexRvm_Wx          , E(F30F00,7B,_,I,_,x,2,T1W), 0                         , 150, 0  , 4261 , 250, 66 ), // #887
  INST(Vdbpsadbw        , VexRvmi_Lx         , E(660F3A,42,_,x,_,0,4,FVM), 0                         , 151, 0  , 4272 , 251, 128), // #888
  INST(Vdivpd           , VexRvm_Lx          , V(660F00,5E,_,x,I,1,4,FV ), 0                         , 102, 0  , 4282 , 189, 121), // #889
  INST(Vdivps           , VexRvm_Lx          , V(000F00,5E,_,x,I,0,4,FV ), 0                         , 103, 0  , 4289 , 190, 121), // #890
  INST(Vdivsd           , VexRvm             , V(F20F00,5E,_,I,I,1,3,T1S), 0                         , 104, 0  , 4296 , 191, 122), // #891
  INST(Vdivss           , VexRvm             , V(F30F00,5E,_,I,I,0,2,T1S), 0                         , 105, 0  , 4303 , 192, 122), // #892
  INST(Vdpbf16ps        , VexRvm             , E(F30F38,52,_,_,_,0,_,_  ), 0                         , 128, 0  , 4310 , 204, 133), // #893
  INST(Vdppd            , VexRvmi_Lx         , V(660F3A,41,_,x,I,_,_,_  ), 0                         , 72 , 0  , 4320 , 252, 123), // #894
  INST(Vdpps            , VexRvmi_Lx         , V(660F3A,40,_,x,I,_,_,_  ), 0                         , 72 , 0  , 4326 , 206, 123), // #895
  INST(Verr             , X86M_NoSize        , O(000F00,00,4,_,_,_,_,_  ), 0                         , 97 , 0  , 4332 , 101, 10 ), // #896
  INST(Verw             , X86M_NoSize        , O(000F00,00,5,_,_,_,_,_  ), 0                         , 75 , 0  , 4337 , 101, 10 ), // #897
  INST(Vexp2pd          , VexRm              , E(660F38,C8,_,2,_,1,4,FV ), 0                         , 152, 0  , 4342 , 253, 135), // #898
  INST(Vexp2ps          , VexRm              , E(660F38,C8,_,2,_,0,4,FV ), 0                         , 153, 0  , 4350 , 254, 135), // #899
  INST(Vexpandpd        , VexRm_Lx           , E(660F38,88,_,x,_,1,3,T1S), 0                         , 124, 0  , 4358 , 255, 126), // #900
  INST(Vexpandps        , VexRm_Lx           , E(660F38,88,_,x,_,0,2,T1S), 0                         , 125, 0  , 4368 , 255, 126), // #901
  INST(Vextractf128     , VexMri             , V(660F3A,19,_,1,0,_,_,_  ), 0                         , 154, 0  , 4378 , 256, 123), // #902
  INST(Vextractf32x4    , VexMri_Lx          , E(660F3A,19,_,x,_,0,4,T4 ), 0                         , 155, 0  , 4391 , 257, 126), // #903
  INST(Vextractf32x8    , VexMri             , E(660F3A,1B,_,2,_,0,5,T8 ), 0                         , 156, 0  , 4405 , 258, 64 ), // #904
  INST(Vextractf64x2    , VexMri_Lx          , E(660F3A,19,_,x,_,1,4,T2 ), 0                         , 157, 0  , 4419 , 257, 129), // #905
  INST(Vextractf64x4    , VexMri             , E(660F3A,1B,_,2,_,1,5,T4 ), 0                         , 158, 0  , 4433 , 258, 66 ), // #906
  INST(Vextracti128     , VexMri             , V(660F3A,39,_,1,0,_,_,_  ), 0                         , 154, 0  , 4447 , 256, 130), // #907
  INST(Vextracti32x4    , VexMri_Lx          , E(660F3A,39,_,x,_,0,4,T4 ), 0                         , 155, 0  , 4460 , 257, 126), // #908
  INST(Vextracti32x8    , VexMri             , E(660F3A,3B,_,2,_,0,5,T8 ), 0                         , 156, 0  , 4474 , 258, 64 ), // #909
  INST(Vextracti64x2    , VexMri_Lx          , E(660F3A,39,_,x,_,1,4,T2 ), 0                         , 157, 0  , 4488 , 257, 129), // #910
  INST(Vextracti64x4    , VexMri             , E(660F3A,3B,_,2,_,1,5,T4 ), 0                         , 158, 0  , 4502 , 258, 66 ), // #911
  INST(Vextractps       , VexMri             , V(660F3A,17,_,0,I,I,2,T1S), 0                         , 159, 0  , 4516 , 259, 122), // #912
  INST(Vfixupimmpd      , VexRvmi_Lx         , E(660F3A,54,_,x,_,1,4,FV ), 0                         , 109, 0  , 4527 , 260, 126), // #913
  INST(Vfixupimmps      , VexRvmi_Lx         , E(660F3A,54,_,x,_,0,4,FV ), 0                         , 108, 0  , 4539 , 261, 126), // #914
  INST(Vfixupimmsd      , VexRvmi            , E(660F3A,55,_,I,_,1,3,T1S), 0                         , 160, 0  , 4551 , 262, 66 ), // #915
  INST(Vfixupimmss      , VexRvmi            , E(660F3A,55,_,I,_,0,2,T1S), 0                         , 161, 0  , 4563 , 263, 66 ), // #916
  INST(Vfmadd132pd      , VexRvm_Lx          , V(660F38,98,_,x,1,1,4,FV ), 0                         , 162, 0  , 4575 , 189, 136), // #917
  INST(Vfmadd132ps      , VexRvm_Lx          , V(660F38,98,_,x,0,0,4,FV ), 0                         , 163, 0  , 4587 , 190, 136), // #918
  INST(Vfmadd132sd      , VexRvm             , V(660F38,99,_,I,1,1,3,T1S), 0                         , 164, 0  , 4599 , 191, 137), // #919
  INST(Vfmadd132ss      , VexRvm             , V(660F38,99,_,I,0,0,2,T1S), 0                         , 121, 0  , 4611 , 192, 137), // #920
  INST(Vfmadd213pd      , VexRvm_Lx          , V(660F38,A8,_,x,1,1,4,FV ), 0                         , 162, 0  , 4623 , 189, 136), // #921
  INST(Vfmadd213ps      , VexRvm_Lx          , V(660F38,A8,_,x,0,0,4,FV ), 0                         , 163, 0  , 4635 , 190, 136), // #922
  INST(Vfmadd213sd      , VexRvm             , V(660F38,A9,_,I,1,1,3,T1S), 0                         , 164, 0  , 4647 , 191, 137), // #923
  INST(Vfmadd213ss      , VexRvm             , V(660F38,A9,_,I,0,0,2,T1S), 0                         , 121, 0  , 4659 , 192, 137), // #924
  INST(Vfmadd231pd      , VexRvm_Lx          , V(660F38,B8,_,x,1,1,4,FV ), 0                         , 162, 0  , 4671 , 189, 136), // #925
  INST(Vfmadd231ps      , VexRvm_Lx          , V(660F38,B8,_,x,0,0,4,FV ), 0                         , 163, 0  , 4683 , 190, 136), // #926
  INST(Vfmadd231sd      , VexRvm             , V(660F38,B9,_,I,1,1,3,T1S), 0                         , 164, 0  , 4695 , 191, 137), // #927
  INST(Vfmadd231ss      , VexRvm             , V(660F38,B9,_,I,0,0,2,T1S), 0                         , 121, 0  , 4707 , 192, 137), // #928
  INST(Vfmaddpd         , Fma4_Lx            , V(660F3A,69,_,x,x,_,_,_  ), 0                         , 72 , 0  , 4719 , 264, 138), // #929
  INST(Vfmaddps         , Fma4_Lx            , V(660F3A,68,_,x,x,_,_,_  ), 0                         , 72 , 0  , 4728 , 264, 138), // #930
  INST(Vfmaddsd         , Fma4               , V(660F3A,6B,_,0,x,_,_,_  ), 0                         , 72 , 0  , 4737 , 265, 138), // #931
  INST(Vfmaddss         , Fma4               , V(660F3A,6A,_,0,x,_,_,_  ), 0                         , 72 , 0  , 4746 , 266, 138), // #932
  INST(Vfmaddsub132pd   , VexRvm_Lx          , V(660F38,96,_,x,1,1,4,FV ), 0                         , 162, 0  , 4755 , 189, 136), // #933
  INST(Vfmaddsub132ps   , VexRvm_Lx          , V(660F38,96,_,x,0,0,4,FV ), 0                         , 163, 0  , 4770 , 190, 136), // #934
  INST(Vfmaddsub213pd   , VexRvm_Lx          , V(660F38,A6,_,x,1,1,4,FV ), 0                         , 162, 0  , 4785 , 189, 136), // #935
  INST(Vfmaddsub213ps   , VexRvm_Lx          , V(660F38,A6,_,x,0,0,4,FV ), 0                         , 163, 0  , 4800 , 190, 136), // #936
  INST(Vfmaddsub231pd   , VexRvm_Lx          , V(660F38,B6,_,x,1,1,4,FV ), 0                         , 162, 0  , 4815 , 189, 136), // #937
  INST(Vfmaddsub231ps   , VexRvm_Lx          , V(660F38,B6,_,x,0,0,4,FV ), 0                         , 163, 0  , 4830 , 190, 136), // #938
  INST(Vfmaddsubpd      , Fma4_Lx            , V(660F3A,5D,_,x,x,_,_,_  ), 0                         , 72 , 0  , 4845 , 264, 138), // #939
  INST(Vfmaddsubps      , Fma4_Lx            , V(660F3A,5C,_,x,x,_,_,_  ), 0                         , 72 , 0  , 4857 , 264, 138), // #940
  INST(Vfmsub132pd      , VexRvm_Lx          , V(660F38,9A,_,x,1,1,4,FV ), 0                         , 162, 0  , 4869 , 189, 136), // #941
  INST(Vfmsub132ps      , VexRvm_Lx          , V(660F38,9A,_,x,0,0,4,FV ), 0                         , 163, 0  , 4881 , 190, 136), // #942
  INST(Vfmsub132sd      , VexRvm             , V(660F38,9B,_,I,1,1,3,T1S), 0                         , 164, 0  , 4893 , 191, 137), // #943
  INST(Vfmsub132ss      , VexRvm             , V(660F38,9B,_,I,0,0,2,T1S), 0                         , 121, 0  , 4905 , 192, 137), // #944
  INST(Vfmsub213pd      , VexRvm_Lx          , V(660F38,AA,_,x,1,1,4,FV ), 0                         , 162, 0  , 4917 , 189, 136), // #945
  INST(Vfmsub213ps      , VexRvm_Lx          , V(660F38,AA,_,x,0,0,4,FV ), 0                         , 163, 0  , 4929 , 190, 136), // #946
  INST(Vfmsub213sd      , VexRvm             , V(660F38,AB,_,I,1,1,3,T1S), 0                         , 164, 0  , 4941 , 191, 137), // #947
  INST(Vfmsub213ss      , VexRvm             , V(660F38,AB,_,I,0,0,2,T1S), 0                         , 121, 0  , 4953 , 192, 137), // #948
  INST(Vfmsub231pd      , VexRvm_Lx          , V(660F38,BA,_,x,1,1,4,FV ), 0                         , 162, 0  , 4965 , 189, 136), // #949
  INST(Vfmsub231ps      , VexRvm_Lx          , V(660F38,BA,_,x,0,0,4,FV ), 0                         , 163, 0  , 4977 , 190, 136), // #950
  INST(Vfmsub231sd      , VexRvm             , V(660F38,BB,_,I,1,1,3,T1S), 0                         , 164, 0  , 4989 , 191, 137), // #951
  INST(Vfmsub231ss      , VexRvm             , V(660F38,BB,_,I,0,0,2,T1S), 0                         , 121, 0  , 5001 , 192, 137), // #952
  INST(Vfmsubadd132pd   , VexRvm_Lx          , V(660F38,97,_,x,1,1,4,FV ), 0                         , 162, 0  , 5013 , 189, 136), // #953
  INST(Vfmsubadd132ps   , VexRvm_Lx          , V(660F38,97,_,x,0,0,4,FV ), 0                         , 163, 0  , 5028 , 190, 136), // #954
  INST(Vfmsubadd213pd   , VexRvm_Lx          , V(660F38,A7,_,x,1,1,4,FV ), 0                         , 162, 0  , 5043 , 189, 136), // #955
  INST(Vfmsubadd213ps   , VexRvm_Lx          , V(660F38,A7,_,x,0,0,4,FV ), 0                         , 163, 0  , 5058 , 190, 136), // #956
  INST(Vfmsubadd231pd   , VexRvm_Lx          , V(660F38,B7,_,x,1,1,4,FV ), 0                         , 162, 0  , 5073 , 189, 136), // #957
  INST(Vfmsubadd231ps   , VexRvm_Lx          , V(660F38,B7,_,x,0,0,4,FV ), 0                         , 163, 0  , 5088 , 190, 136), // #958
  INST(Vfmsubaddpd      , Fma4_Lx            , V(660F3A,5F,_,x,x,_,_,_  ), 0                         , 72 , 0  , 5103 , 264, 138), // #959
  INST(Vfmsubaddps      , Fma4_Lx            , V(660F3A,5E,_,x,x,_,_,_  ), 0                         , 72 , 0  , 5115 , 264, 138), // #960
  INST(Vfmsubpd         , Fma4_Lx            , V(660F3A,6D,_,x,x,_,_,_  ), 0                         , 72 , 0  , 5127 , 264, 138), // #961
  INST(Vfmsubps         , Fma4_Lx            , V(660F3A,6C,_,x,x,_,_,_  ), 0                         , 72 , 0  , 5136 , 264, 138), // #962
  INST(Vfmsubsd         , Fma4               , V(660F3A,6F,_,0,x,_,_,_  ), 0                         , 72 , 0  , 5145 , 265, 138), // #963
  INST(Vfmsubss         , Fma4               , V(660F3A,6E,_,0,x,_,_,_  ), 0                         , 72 , 0  , 5154 , 266, 138), // #964
  INST(Vfnmadd132pd     , VexRvm_Lx          , V(660F38,9C,_,x,1,1,4,FV ), 0                         , 162, 0  , 5163 , 189, 136), // #965
  INST(Vfnmadd132ps     , VexRvm_Lx          , V(660F38,9C,_,x,0,0,4,FV ), 0                         , 163, 0  , 5176 , 190, 136), // #966
  INST(Vfnmadd132sd     , VexRvm             , V(660F38,9D,_,I,1,1,3,T1S), 0                         , 164, 0  , 5189 , 191, 137), // #967
  INST(Vfnmadd132ss     , VexRvm             , V(660F38,9D,_,I,0,0,2,T1S), 0                         , 121, 0  , 5202 , 192, 137), // #968
  INST(Vfnmadd213pd     , VexRvm_Lx          , V(660F38,AC,_,x,1,1,4,FV ), 0                         , 162, 0  , 5215 , 189, 136), // #969
  INST(Vfnmadd213ps     , VexRvm_Lx          , V(660F38,AC,_,x,0,0,4,FV ), 0                         , 163, 0  , 5228 , 190, 136), // #970
  INST(Vfnmadd213sd     , VexRvm             , V(660F38,AD,_,I,1,1,3,T1S), 0                         , 164, 0  , 5241 , 191, 137), // #971
  INST(Vfnmadd213ss     , VexRvm             , V(660F38,AD,_,I,0,0,2,T1S), 0                         , 121, 0  , 5254 , 192, 137), // #972
  INST(Vfnmadd231pd     , VexRvm_Lx          , V(660F38,BC,_,x,1,1,4,FV ), 0                         , 162, 0  , 5267 , 189, 136), // #973
  INST(Vfnmadd231ps     , VexRvm_Lx          , V(660F38,BC,_,x,0,0,4,FV ), 0                         , 163, 0  , 5280 , 190, 136), // #974
  INST(Vfnmadd231sd     , VexRvm             , V(660F38,BC,_,I,1,1,3,T1S), 0                         , 164, 0  , 5293 , 191, 137), // #975
  INST(Vfnmadd231ss     , VexRvm             , V(660F38,BC,_,I,0,0,2,T1S), 0                         , 121, 0  , 5306 , 192, 137), // #976
  INST(Vfnmaddpd        , Fma4_Lx            , V(660F3A,79,_,x,x,_,_,_  ), 0                         , 72 , 0  , 5319 , 264, 138), // #977
  INST(Vfnmaddps        , Fma4_Lx            , V(660F3A,78,_,x,x,_,_,_  ), 0                         , 72 , 0  , 5329 , 264, 138), // #978
  INST(Vfnmaddsd        , Fma4               , V(660F3A,7B,_,0,x,_,_,_  ), 0                         , 72 , 0  , 5339 , 265, 138), // #979
  INST(Vfnmaddss        , Fma4               , V(660F3A,7A,_,0,x,_,_,_  ), 0                         , 72 , 0  , 5349 , 266, 138), // #980
  INST(Vfnmsub132pd     , VexRvm_Lx          , V(660F38,9E,_,x,1,1,4,FV ), 0                         , 162, 0  , 5359 , 189, 136), // #981
  INST(Vfnmsub132ps     , VexRvm_Lx          , V(660F38,9E,_,x,0,0,4,FV ), 0                         , 163, 0  , 5372 , 190, 136), // #982
  INST(Vfnmsub132sd     , VexRvm             , V(660F38,9F,_,I,1,1,3,T1S), 0                         , 164, 0  , 5385 , 191, 137), // #983
  INST(Vfnmsub132ss     , VexRvm             , V(660F38,9F,_,I,0,0,2,T1S), 0                         , 121, 0  , 5398 , 192, 137), // #984
  INST(Vfnmsub213pd     , VexRvm_Lx          , V(660F38,AE,_,x,1,1,4,FV ), 0                         , 162, 0  , 5411 , 189, 136), // #985
  INST(Vfnmsub213ps     , VexRvm_Lx          , V(660F38,AE,_,x,0,0,4,FV ), 0                         , 163, 0  , 5424 , 190, 136), // #986
  INST(Vfnmsub213sd     , VexRvm             , V(660F38,AF,_,I,1,1,3,T1S), 0                         , 164, 0  , 5437 , 191, 137), // #987
  INST(Vfnmsub213ss     , VexRvm             , V(660F38,AF,_,I,0,0,2,T1S), 0                         , 121, 0  , 5450 , 192, 137), // #988
  INST(Vfnmsub231pd     , VexRvm_Lx          , V(660F38,BE,_,x,1,1,4,FV ), 0                         , 162, 0  , 5463 , 189, 136), // #989
  INST(Vfnmsub231ps     , VexRvm_Lx          , V(660F38,BE,_,x,0,0,4,FV ), 0                         , 163, 0  , 5476 , 190, 136), // #990
  INST(Vfnmsub231sd     , VexRvm             , V(660F38,BF,_,I,1,1,3,T1S), 0                         , 164, 0  , 5489 , 191, 137), // #991
  INST(Vfnmsub231ss     , VexRvm             , V(660F38,BF,_,I,0,0,2,T1S), 0                         , 121, 0  , 5502 , 192, 137), // #992
  INST(Vfnmsubpd        , Fma4_Lx            , V(660F3A,7D,_,x,x,_,_,_  ), 0                         , 72 , 0  , 5515 , 264, 138), // #993
  INST(Vfnmsubps        , Fma4_Lx            , V(660F3A,7C,_,x,x,_,_,_  ), 0                         , 72 , 0  , 5525 , 264, 138), // #994
  INST(Vfnmsubsd        , Fma4               , V(660F3A,7F,_,0,x,_,_,_  ), 0                         , 72 , 0  , 5535 , 265, 138), // #995
  INST(Vfnmsubss        , Fma4               , V(660F3A,7E,_,0,x,_,_,_  ), 0                         , 72 , 0  , 5545 , 266, 138), // #996
  INST(Vfpclasspd       , VexRmi_Lx          , E(660F3A,66,_,x,_,1,4,FV ), 0                         , 109, 0  , 5555 , 267, 129), // #997
  INST(Vfpclassps       , VexRmi_Lx          , E(660F3A,66,_,x,_,0,4,FV ), 0                         , 108, 0  , 5566 , 268, 129), // #998
  INST(Vfpclasssd       , VexRmi_Lx          , E(660F3A,67,_,I,_,1,3,T1S), 0                         , 160, 0  , 5577 , 269, 64 ), // #999
  INST(Vfpclassss       , VexRmi_Lx          , E(660F3A,67,_,I,_,0,2,T1S), 0                         , 161, 0  , 5588 , 270, 64 ), // #1000
  INST(Vfrczpd          , VexRm_Lx           , V(XOP_M9,81,_,x,0,_,_,_  ), 0                         , 77 , 0  , 5599 , 271, 139), // #1001
  INST(Vfrczps          , VexRm_Lx           , V(XOP_M9,80,_,x,0,_,_,_  ), 0                         , 77 , 0  , 5607 , 271, 139), // #1002
  INST(Vfrczsd          , VexRm              , V(XOP_M9,83,_,0,0,_,_,_  ), 0                         , 77 , 0  , 5615 , 272, 139), // #1003
  INST(Vfrczss          , VexRm              , V(XOP_M9,82,_,0,0,_,_,_  ), 0                         , 77 , 0  , 5623 , 273, 139), // #1004
  INST(Vgatherdpd       , VexRmvRm_VM        , V(660F38,92,_,x,1,_,_,_  ), V(660F38,92,_,x,_,1,3,T1S), 165, 79 , 5631 , 274, 140), // #1005
  INST(Vgatherdps       , VexRmvRm_VM        , V(660F38,92,_,x,0,_,_,_  ), V(660F38,92,_,x,_,0,2,T1S), 96 , 80 , 5642 , 275, 140), // #1006
  INST(Vgatherpf0dpd    , VexM_VM            , E(660F38,C6,1,2,_,1,3,T1S), 0                         , 166, 0  , 5653 , 276, 141), // #1007
  INST(Vgatherpf0dps    , VexM_VM            , E(660F38,C6,1,2,_,0,2,T1S), 0                         , 167, 0  , 5667 , 277, 141), // #1008
  INST(Vgatherpf0qpd    , VexM_VM            , E(660F38,C7,1,2,_,1,3,T1S), 0                         , 166, 0  , 5681 , 278, 141), // #1009
  INST(Vgatherpf0qps    , VexM_VM            , E(660F38,C7,1,2,_,0,2,T1S), 0                         , 167, 0  , 5695 , 278, 141), // #1010
  INST(Vgatherpf1dpd    , VexM_VM            , E(660F38,C6,2,2,_,1,3,T1S), 0                         , 168, 0  , 5709 , 276, 141), // #1011
  INST(Vgatherpf1dps    , VexM_VM            , E(660F38,C6,2,2,_,0,2,T1S), 0                         , 169, 0  , 5723 , 277, 141), // #1012
  INST(Vgatherpf1qpd    , VexM_VM            , E(660F38,C7,2,2,_,1,3,T1S), 0                         , 168, 0  , 5737 , 278, 141), // #1013
  INST(Vgatherpf1qps    , VexM_VM            , E(660F38,C7,2,2,_,0,2,T1S), 0                         , 169, 0  , 5751 , 278, 141), // #1014
  INST(Vgatherqpd       , VexRmvRm_VM        , V(660F38,93,_,x,1,_,_,_  ), V(660F38,93,_,x,_,1,3,T1S), 165, 81 , 5765 , 279, 140), // #1015
  INST(Vgatherqps       , VexRmvRm_VM        , V(660F38,93,_,x,0,_,_,_  ), V(660F38,93,_,x,_,0,2,T1S), 96 , 82 , 5776 , 280, 140), // #1016
  INST(Vgetexppd        , VexRm_Lx           , E(660F38,42,_,x,_,1,4,FV ), 0                         , 112, 0  , 5787 , 240, 126), // #1017
  INST(Vgetexpps        , VexRm_Lx           , E(660F38,42,_,x,_,0,4,FV ), 0                         , 111, 0  , 5797 , 244, 126), // #1018
  INST(Vgetexpsd        , VexRvm             , E(660F38,43,_,I,_,1,3,T1S), 0                         , 124, 0  , 5807 , 281, 66 ), // #1019
  INST(Vgetexpss        , VexRvm             , E(660F38,43,_,I,_,0,2,T1S), 0                         , 125, 0  , 5817 , 282, 66 ), // #1020
  INST(Vgetmantpd       , VexRmi_Lx          , E(660F3A,26,_,x,_,1,4,FV ), 0                         , 109, 0  , 5827 , 283, 126), // #1021
  INST(Vgetmantps       , VexRmi_Lx          , E(660F3A,26,_,x,_,0,4,FV ), 0                         , 108, 0  , 5838 , 284, 126), // #1022
  INST(Vgetmantsd       , VexRvmi            , E(660F3A,27,_,I,_,1,3,T1S), 0                         , 160, 0  , 5849 , 262, 66 ), // #1023
  INST(Vgetmantss       , VexRvmi            , E(660F3A,27,_,I,_,0,2,T1S), 0                         , 161, 0  , 5860 , 263, 66 ), // #1024
  INST(Vgf2p8affineinvqb, VexRvmi_Lx         , V(660F3A,CF,_,x,1,1,4,FV ), 0                         , 170, 0  , 5871 , 285, 142), // #1025
  INST(Vgf2p8affineqb   , VexRvmi_Lx         , V(660F3A,CE,_,x,1,1,4,FV ), 0                         , 170, 0  , 5889 , 285, 142), // #1026
  INST(Vgf2p8mulb       , VexRvm_Lx          , V(660F38,CF,_,x,0,0,4,FV ), 0                         , 163, 0  , 5904 , 286, 142), // #1027
  INST(Vhaddpd          , VexRvm_Lx          , V(660F00,7C,_,x,I,_,_,_  ), 0                         , 68 , 0  , 5915 , 193, 123), // #1028
  INST(Vhaddps          , VexRvm_Lx          , V(F20F00,7C,_,x,I,_,_,_  ), 0                         , 106, 0  , 5923 , 193, 123), // #1029
  INST(Vhsubpd          , VexRvm_Lx          , V(660F00,7D,_,x,I,_,_,_  ), 0                         , 68 , 0  , 5931 , 193, 123), // #1030
  INST(Vhsubps          , VexRvm_Lx          , V(F20F00,7D,_,x,I,_,_,_  ), 0                         , 106, 0  , 5939 , 193, 123), // #1031
  INST(Vinsertf128      , VexRvmi            , V(660F3A,18,_,1,0,_,_,_  ), 0                         , 154, 0  , 5947 , 287, 123), // #1032
  INST(Vinsertf32x4     , VexRvmi_Lx         , E(660F3A,18,_,x,_,0,4,T4 ), 0                         , 155, 0  , 5959 , 288, 126), // #1033
  INST(Vinsertf32x8     , VexRvmi            , E(660F3A,1A,_,2,_,0,5,T8 ), 0                         , 156, 0  , 5972 , 289, 64 ), // #1034
  INST(Vinsertf64x2     , VexRvmi_Lx         , E(660F3A,18,_,x,_,1,4,T2 ), 0                         , 157, 0  , 5985 , 288, 129), // #1035
  INST(Vinsertf64x4     , VexRvmi            , E(660F3A,1A,_,2,_,1,5,T4 ), 0                         , 158, 0  , 5998 , 289, 66 ), // #1036
  INST(Vinserti128      , VexRvmi            , V(660F3A,38,_,1,0,_,_,_  ), 0                         , 154, 0  , 6011 , 287, 130), // #1037
  INST(Vinserti32x4     , VexRvmi_Lx         , E(660F3A,38,_,x,_,0,4,T4 ), 0                         , 155, 0  , 6023 , 288, 126), // #1038
  INST(Vinserti32x8     , VexRvmi            , E(660F3A,3A,_,2,_,0,5,T8 ), 0                         , 156, 0  , 6036 , 289, 64 ), // #1039
  INST(Vinserti64x2     , VexRvmi_Lx         , E(660F3A,38,_,x,_,1,4,T2 ), 0                         , 157, 0  , 6049 , 288, 129), // #1040
  INST(Vinserti64x4     , VexRvmi            , E(660F3A,3A,_,2,_,1,5,T4 ), 0                         , 158, 0  , 6062 , 289, 66 ), // #1041
  INST(Vinsertps        , VexRvmi            , V(660F3A,21,_,0,I,0,2,T1S), 0                         , 159, 0  , 6075 , 290, 122), // #1042
  INST(Vlddqu           , VexRm_Lx           , V(F20F00,F0,_,x,I,_,_,_  ), 0                         , 106, 0  , 6085 , 291, 123), // #1043
  INST(Vldmxcsr         , VexM               , V(000F00,AE,2,0,I,_,_,_  ), 0                         , 171, 0  , 6092 , 292, 123), // #1044
  INST(Vmaskmovdqu      , VexRm_ZDI          , V(660F00,F7,_,0,I,_,_,_  ), 0                         , 68 , 0  , 6101 , 293, 123), // #1045
  INST(Vmaskmovpd       , VexRvmMvr_Lx       , V(660F38,2D,_,x,0,_,_,_  ), V(660F38,2F,_,x,0,_,_,_  ), 96 , 83 , 6113 , 294, 123), // #1046
  INST(Vmaskmovps       , VexRvmMvr_Lx       , V(660F38,2C,_,x,0,_,_,_  ), V(660F38,2E,_,x,0,_,_,_  ), 96 , 84 , 6124 , 294, 123), // #1047
  INST(Vmaxpd           , VexRvm_Lx          , V(660F00,5F,_,x,I,1,4,FV ), 0                         , 102, 0  , 6135 , 295, 121), // #1048
  INST(Vmaxps           , VexRvm_Lx          , V(000F00,5F,_,x,I,0,4,FV ), 0                         , 103, 0  , 6142 , 296, 121), // #1049
  INST(Vmaxsd           , VexRvm             , V(F20F00,5F,_,I,I,1,3,T1S), 0                         , 104, 0  , 6149 , 297, 121), // #1050
  INST(Vmaxss           , VexRvm             , V(F30F00,5F,_,I,I,0,2,T1S), 0                         , 105, 0  , 6156 , 236, 121), // #1051
  INST(Vmcall           , X86Op              , O(000F01,C1,_,_,_,_,_,_  ), 0                         , 21 , 0  , 6163 , 30 , 56 ), // #1052
  INST(Vmclear          , X86M_Only          , O(660F00,C7,6,_,_,_,_,_  ), 0                         , 25 , 0  , 6170 , 32 , 56 ), // #1053
  INST(Vmfunc           , X86Op              , O(000F01,D4,_,_,_,_,_,_  ), 0                         , 21 , 0  , 6178 , 30 , 56 ), // #1054
  INST(Vminpd           , VexRvm_Lx          , V(660F00,5D,_,x,I,1,4,FV ), 0                         , 102, 0  , 6185 , 295, 121), // #1055
  INST(Vminps           , VexRvm_Lx          , V(000F00,5D,_,x,I,0,4,FV ), 0                         , 103, 0  , 6192 , 296, 121), // #1056
  INST(Vminsd           , VexRvm             , V(F20F00,5D,_,I,I,1,3,T1S), 0                         , 104, 0  , 6199 , 297, 121), // #1057
  INST(Vminss           , VexRvm             , V(F30F00,5D,_,I,I,0,2,T1S), 0                         , 105, 0  , 6206 , 236, 121), // #1058
  INST(Vmlaunch         , X86Op              , O(000F01,C2,_,_,_,_,_,_  ), 0                         , 21 , 0  , 6213 , 30 , 56 ), // #1059
  INST(Vmload           , X86Op_xAX          , O(000F01,DA,_,_,_,_,_,_  ), 0                         , 21 , 0  , 6222 , 298, 22 ), // #1060
  INST(Vmmcall          , X86Op              , O(000F01,D9,_,_,_,_,_,_  ), 0                         , 21 , 0  , 6229 , 30 , 22 ), // #1061
  INST(Vmovapd          , VexRmMr_Lx         , V(660F00,28,_,x,I,1,4,FVM), V(660F00,29,_,x,I,1,4,FVM), 172, 85 , 6237 , 299, 121), // #1062
  INST(Vmovaps          , VexRmMr_Lx         , V(000F00,28,_,x,I,0,4,FVM), V(000F00,29,_,x,I,0,4,FVM), 173, 86 , 6245 , 299, 121), // #1063
  INST(Vmovd            , VexMovdMovq        , V(660F00,6E,_,0,0,0,2,T1S), V(660F00,7E,_,0,0,0,2,T1S), 174, 87 , 6253 , 300, 122), // #1064
  INST(Vmovddup         , VexRm_Lx           , V(F20F00,12,_,x,I,1,3,DUP), 0                         , 175, 0  , 6259 , 301, 121), // #1065
  INST(Vmovdqa          , VexRmMr_Lx         , V(660F00,6F,_,x,I,_,_,_  ), V(660F00,7F,_,x,I,_,_,_  ), 68 , 88 , 6268 , 302, 123), // #1066
  INST(Vmovdqa32        , VexRmMr_Lx         , E(660F00,6F,_,x,_,0,4,FVM), E(660F00,7F,_,x,_,0,4,FVM), 176, 89 , 6276 , 303, 126), // #1067
  INST(Vmovdqa64        , VexRmMr_Lx         , E(660F00,6F,_,x,_,1,4,FVM), E(660F00,7F,_,x,_,1,4,FVM), 177, 90 , 6286 , 303, 126), // #1068
  INST(Vmovdqu          , VexRmMr_Lx         , V(F30F00,6F,_,x,I,_,_,_  ), V(F30F00,7F,_,x,I,_,_,_  ), 178, 91 , 6296 , 302, 123), // #1069
  INST(Vmovdqu16        , VexRmMr_Lx         , E(F20F00,6F,_,x,_,1,4,FVM), E(F20F00,7F,_,x,_,1,4,FVM), 179, 92 , 6304 , 303, 128), // #1070
  INST(Vmovdqu32        , VexRmMr_Lx         , E(F30F00,6F,_,x,_,0,4,FVM), E(F30F00,7F,_,x,_,0,4,FVM), 180, 93 , 6314 , 303, 126), // #1071
  INST(Vmovdqu64        , VexRmMr_Lx         , E(F30F00,6F,_,x,_,1,4,FVM), E(F30F00,7F,_,x,_,1,4,FVM), 181, 94 , 6324 , 303, 126), // #1072
  INST(Vmovdqu8         , VexRmMr_Lx         , E(F20F00,6F,_,x,_,0,4,FVM), E(F20F00,7F,_,x,_,0,4,FVM), 182, 95 , 6334 , 303, 128), // #1073
  INST(Vmovhlps         , VexRvm             , V(000F00,12,_,0,I,0,_,_  ), 0                         , 71 , 0  , 6343 , 304, 122), // #1074
  INST(Vmovhpd          , VexRvmMr           , V(660F00,16,_,0,I,1,3,T1S), V(660F00,17,_,0,I,1,3,T1S), 122, 96 , 6352 , 305, 122), // #1075
  INST(Vmovhps          , VexRvmMr           , V(000F00,16,_,0,I,0,3,T2 ), V(000F00,17,_,0,I,0,3,T2 ), 183, 97 , 6360 , 305, 122), // #1076
  INST(Vmovlhps         , VexRvm             , V(000F00,16,_,0,I,0,_,_  ), 0                         , 71 , 0  , 6368 , 304, 122), // #1077
  INST(Vmovlpd          , VexRvmMr           , V(660F00,12,_,0,I,1,3,T1S), V(660F00,13,_,0,I,1,3,T1S), 122, 98 , 6377 , 305, 122), // #1078
  INST(Vmovlps          , VexRvmMr           , V(000F00,12,_,0,I,0,3,T2 ), V(000F00,13,_,0,I,0,3,T2 ), 183, 99 , 6385 , 305, 122), // #1079
  INST(Vmovmskpd        , VexRm_Lx           , V(660F00,50,_,x,I,_,_,_  ), 0                         , 68 , 0  , 6393 , 306, 123), // #1080
  INST(Vmovmskps        , VexRm_Lx           , V(000F00,50,_,x,I,_,_,_  ), 0                         , 71 , 0  , 6403 , 306, 123), // #1081
  INST(Vmovntdq         , VexMr_Lx           , V(660F00,E7,_,x,I,0,4,FVM), 0                         , 184, 0  , 6413 , 307, 121), // #1082
  INST(Vmovntdqa        , VexRm_Lx           , V(660F38,2A,_,x,I,0,4,FVM), 0                         , 107, 0  , 6422 , 308, 131), // #1083
  INST(Vmovntpd         , VexMr_Lx           , V(660F00,2B,_,x,I,1,4,FVM), 0                         , 172, 0  , 6432 , 307, 121), // #1084
  INST(Vmovntps         , VexMr_Lx           , V(000F00,2B,_,x,I,0,4,FVM), 0                         , 173, 0  , 6441 , 307, 121), // #1085
  INST(Vmovq            , VexMovdMovq        , V(660F00,6E,_,0,I,1,3,T1S), V(660F00,7E,_,0,I,1,3,T1S), 122, 100, 6450 , 309, 122), // #1086
  INST(Vmovsd           , VexMovssMovsd      , V(F20F00,10,_,I,I,1,3,T1S), V(F20F00,11,_,I,I,1,3,T1S), 104, 101, 6456 , 310, 122), // #1087
  INST(Vmovshdup        , VexRm_Lx           , V(F30F00,16,_,x,I,0,4,FVM), 0                         , 185, 0  , 6463 , 311, 121), // #1088
  INST(Vmovsldup        , VexRm_Lx           , V(F30F00,12,_,x,I,0,4,FVM), 0                         , 185, 0  , 6473 , 311, 121), // #1089
  INST(Vmovss           , VexMovssMovsd      , V(F30F00,10,_,I,I,0,2,T1S), V(F30F00,11,_,I,I,0,2,T1S), 105, 102, 6483 , 312, 122), // #1090
  INST(Vmovupd          , VexRmMr_Lx         , V(660F00,10,_,x,I,1,4,FVM), V(660F00,11,_,x,I,1,4,FVM), 172, 103, 6490 , 299, 121), // #1091
  INST(Vmovups          , VexRmMr_Lx         , V(000F00,10,_,x,I,0,4,FVM), V(000F00,11,_,x,I,0,4,FVM), 173, 104, 6498 , 299, 121), // #1092
  INST(Vmpsadbw         , VexRvmi_Lx         , V(660F3A,42,_,x,I,_,_,_  ), 0                         , 72 , 0  , 6506 , 206, 143), // #1093
  INST(Vmptrld          , X86M_Only          , O(000F00,C7,6,_,_,_,_,_  ), 0                         , 78 , 0  , 6515 , 32 , 56 ), // #1094
  INST(Vmptrst          , X86M_Only          , O(000F00,C7,7,_,_,_,_,_  ), 0                         , 22 , 0  , 6523 , 32 , 56 ), // #1095
  INST(Vmread           , X86Mr_NoSize       , O(000F00,78,_,_,_,_,_,_  ), 0                         , 4  , 0  , 6531 , 313, 56 ), // #1096
  INST(Vmresume         , X86Op              , O(000F01,C3,_,_,_,_,_,_  ), 0                         , 21 , 0  , 6538 , 30 , 56 ), // #1097
  INST(Vmrun            , X86Op_xAX          , O(000F01,D8,_,_,_,_,_,_  ), 0                         , 21 , 0  , 6547 , 298, 22 ), // #1098
  INST(Vmsave           , X86Op_xAX          , O(000F01,DB,_,_,_,_,_,_  ), 0                         , 21 , 0  , 6553 , 298, 22 ), // #1099
  INST(Vmulpd           , VexRvm_Lx          , V(660F00,59,_,x,I,1,4,FV ), 0                         , 102, 0  , 6560 , 189, 121), // #1100
  INST(Vmulps           , VexRvm_Lx          , V(000F00,59,_,x,I,0,4,FV ), 0                         , 103, 0  , 6567 , 190, 121), // #1101
  INST(Vmulsd           , VexRvm_Lx          , V(F20F00,59,_,I,I,1,3,T1S), 0                         , 104, 0  , 6574 , 191, 122), // #1102
  INST(Vmulss           , VexRvm_Lx          , V(F30F00,59,_,I,I,0,2,T1S), 0                         , 105, 0  , 6581 , 192, 122), // #1103
  INST(Vmwrite          , X86Rm_NoSize       , O(000F00,79,_,_,_,_,_,_  ), 0                         , 4  , 0  , 6588 , 314, 56 ), // #1104
  INST(Vmxon            , X86M_Only          , O(F30F00,C7,6,_,_,_,_,_  ), 0                         , 24 , 0  , 6596 , 32 , 56 ), // #1105
  INST(Vorpd            , VexRvm_Lx          , V(660F00,56,_,x,I,1,4,FV ), 0                         , 102, 0  , 6602 , 201, 127), // #1106
  INST(Vorps            , VexRvm_Lx          , V(000F00,56,_,x,I,0,4,FV ), 0                         , 103, 0  , 6608 , 202, 127), // #1107
  INST(Vp2intersectd    , VexRvm_Lx_2xK      , E(F20F38,68,_,_,_,0,4,FV ), 0                         , 186, 0  , 6614 , 315, 144), // #1108
  INST(Vp2intersectq    , VexRvm_Lx_2xK      , E(F20F38,68,_,_,_,1,4,FV ), 0                         , 187, 0  , 6628 , 316, 144), // #1109
  INST(Vp4dpwssd        , VexRm_T1_4X        , E(F20F38,52,_,2,_,0,2,T4X), 0                         , 101, 0  , 6642 , 187, 145), // #1110
  INST(Vp4dpwssds       , VexRm_T1_4X        , E(F20F38,53,_,2,_,0,2,T4X), 0                         , 101, 0  , 6652 , 187, 145), // #1111
  INST(Vpabsb           , VexRm_Lx           , V(660F38,1C,_,x,I,_,4,FVM), 0                         , 107, 0  , 6663 , 311, 146), // #1112
  INST(Vpabsd           , VexRm_Lx           , V(660F38,1E,_,x,I,0,4,FV ), 0                         , 163, 0  , 6670 , 311, 131), // #1113
  INST(Vpabsq           , VexRm_Lx           , E(660F38,1F,_,x,_,1,4,FV ), 0                         , 112, 0  , 6677 , 255, 126), // #1114
  INST(Vpabsw           , VexRm_Lx           , V(660F38,1D,_,x,I,_,4,FVM), 0                         , 107, 0  , 6684 , 311, 146), // #1115
  INST(Vpackssdw        , VexRvm_Lx          , V(660F00,6B,_,x,I,0,4,FV ), 0                         , 133, 0  , 6691 , 200, 146), // #1116
  INST(Vpacksswb        , VexRvm_Lx          , V(660F00,63,_,x,I,I,4,FVM), 0                         , 184, 0  , 6701 , 286, 146), // #1117
  INST(Vpackusdw        , VexRvm_Lx          , V(660F38,2B,_,x,I,0,4,FV ), 0                         , 163, 0  , 6711 , 200, 146), // #1118
  INST(Vpackuswb        , VexRvm_Lx          , V(660F00,67,_,x,I,I,4,FVM), 0                         , 184, 0  , 6721 , 286, 146), // #1119
  INST(Vpaddb           , VexRvm_Lx          , V(660F00,FC,_,x,I,I,4,FVM), 0                         , 184, 0  , 6731 , 286, 146), // #1120
  INST(Vpaddd           , VexRvm_Lx          , V(660F00,FE,_,x,I,0,4,FV ), 0                         , 133, 0  , 6738 , 200, 131), // #1121
  INST(Vpaddq           , VexRvm_Lx          , V(660F00,D4,_,x,I,1,4,FV ), 0                         , 102, 0  , 6745 , 199, 131), // #1122
  INST(Vpaddsb          , VexRvm_Lx          , V(660F00,EC,_,x,I,I,4,FVM), 0                         , 184, 0  , 6752 , 286, 146), // #1123
  INST(Vpaddsw          , VexRvm_Lx          , V(660F00,ED,_,x,I,I,4,FVM), 0                         , 184, 0  , 6760 , 286, 146), // #1124
  INST(Vpaddusb         , VexRvm_Lx          , V(660F00,DC,_,x,I,I,4,FVM), 0                         , 184, 0  , 6768 , 286, 146), // #1125
  INST(Vpaddusw         , VexRvm_Lx          , V(660F00,DD,_,x,I,I,4,FVM), 0                         , 184, 0  , 6777 , 286, 146), // #1126
  INST(Vpaddw           , VexRvm_Lx          , V(660F00,FD,_,x,I,I,4,FVM), 0                         , 184, 0  , 6786 , 286, 146), // #1127
  INST(Vpalignr         , VexRvmi_Lx         , V(660F3A,0F,_,x,I,I,4,FVM), 0                         , 188, 0  , 6793 , 285, 146), // #1128
  INST(Vpand            , VexRvm_Lx          , V(660F00,DB,_,x,I,_,_,_  ), 0                         , 68 , 0  , 6802 , 317, 143), // #1129
  INST(Vpandd           , VexRvm_Lx          , E(660F00,DB,_,x,_,0,4,FV ), 0                         , 189, 0  , 6808 , 318, 126), // #1130
  INST(Vpandn           , VexRvm_Lx          , V(660F00,DF,_,x,I,_,_,_  ), 0                         , 68 , 0  , 6815 , 319, 143), // #1131
  INST(Vpandnd          , VexRvm_Lx          , E(660F00,DF,_,x,_,0,4,FV ), 0                         , 189, 0  , 6822 , 320, 126), // #1132
  INST(Vpandnq          , VexRvm_Lx          , E(660F00,DF,_,x,_,1,4,FV ), 0                         , 130, 0  , 6830 , 321, 126), // #1133
  INST(Vpandq           , VexRvm_Lx          , E(660F00,DB,_,x,_,1,4,FV ), 0                         , 130, 0  , 6838 , 322, 126), // #1134
  INST(Vpavgb           , VexRvm_Lx          , V(660F00,E0,_,x,I,I,4,FVM), 0                         , 184, 0  , 6845 , 286, 146), // #1135
  INST(Vpavgw           , VexRvm_Lx          , V(660F00,E3,_,x,I,I,4,FVM), 0                         , 184, 0  , 6852 , 286, 146), // #1136
  INST(Vpblendd         , VexRvmi_Lx         , V(660F3A,02,_,x,0,_,_,_  ), 0                         , 72 , 0  , 6859 , 206, 130), // #1137
  INST(Vpblendvb        , VexRvmr            , V(660F3A,4C,_,x,0,_,_,_  ), 0                         , 72 , 0  , 6868 , 207, 143), // #1138
  INST(Vpblendw         , VexRvmi_Lx         , V(660F3A,0E,_,x,I,_,_,_  ), 0                         , 72 , 0  , 6878 , 206, 143), // #1139
  INST(Vpbroadcastb     , VexRm_Lx_Bcst      , V(660F38,78,_,x,0,0,0,T1S), E(660F38,7A,_,x,0,0,0,T1S), 190, 105, 6887 , 323, 147), // #1140
  INST(Vpbroadcastd     , VexRm_Lx_Bcst      , V(660F38,58,_,x,0,0,2,T1S), E(660F38,7C,_,x,0,0,0,T1S), 121, 106, 6900 , 324, 140), // #1141
  INST(Vpbroadcastmb2d  , VexRm_Lx           , E(F30F38,3A,_,x,_,0,_,_  ), 0                         , 128, 0  , 6913 , 325, 148), // #1142
  INST(Vpbroadcastmb2q  , VexRm_Lx           , E(F30F38,2A,_,x,_,1,_,_  ), 0                         , 191, 0  , 6929 , 325, 148), // #1143
  INST(Vpbroadcastq     , VexRm_Lx_Bcst      , V(660F38,59,_,x,0,1,3,T1S), E(660F38,7C,_,x,0,1,0,T1S), 120, 107, 6945 , 326, 140), // #1144
  INST(Vpbroadcastw     , VexRm_Lx_Bcst      , V(660F38,79,_,x,0,0,1,T1S), E(660F38,7B,_,x,0,0,0,T1S), 192, 108, 6958 , 327, 147), // #1145
  INST(Vpclmulqdq       , VexRvmi_Lx         , V(660F3A,44,_,x,I,_,4,FVM), 0                         , 188, 0  , 6971 , 328, 149), // #1146
  INST(Vpcmov           , VexRvrmRvmr_Lx     , V(XOP_M8,A2,_,x,x,_,_,_  ), 0                         , 193, 0  , 6982 , 264, 139), // #1147
  INST(Vpcmpb           , VexRvmi_Lx         , E(660F3A,3F,_,x,_,0,4,FVM), 0                         , 151, 0  , 6989 , 329, 128), // #1148
  INST(Vpcmpd           , VexRvmi_Lx         , E(660F3A,1F,_,x,_,0,4,FV ), 0                         , 108, 0  , 6996 , 330, 126), // #1149
  INST(Vpcmpeqb         , VexRvm_Lx          , V(660F00,74,_,x,I,I,4,FV ), 0                         , 133, 0  , 7003 , 331, 146), // #1150
  INST(Vpcmpeqd         , VexRvm_Lx          , V(660F00,76,_,x,I,0,4,FVM), 0                         , 184, 0  , 7012 , 332, 131), // #1151
  INST(Vpcmpeqq         , VexRvm_Lx          , V(660F38,29,_,x,I,1,4,FVM), 0                         , 194, 0  , 7021 , 333, 131), // #1152
  INST(Vpcmpeqw         , VexRvm_Lx          , V(660F00,75,_,x,I,I,4,FV ), 0                         , 133, 0  , 7030 , 331, 146), // #1153
  INST(Vpcmpestri       , VexRmi             , V(660F3A,61,_,0,I,_,_,_  ), 0                         , 72 , 0  , 7039 , 334, 150), // #1154
  INST(Vpcmpestrm       , VexRmi             , V(660F3A,60,_,0,I,_,_,_  ), 0                         , 72 , 0  , 7050 , 335, 150), // #1155
  INST(Vpcmpgtb         , VexRvm_Lx          , V(660F00,64,_,x,I,I,4,FV ), 0                         , 133, 0  , 7061 , 331, 146), // #1156
  INST(Vpcmpgtd         , VexRvm_Lx          , V(660F00,66,_,x,I,0,4,FVM), 0                         , 184, 0  , 7070 , 332, 131), // #1157
  INST(Vpcmpgtq         , VexRvm_Lx          , V(660F38,37,_,x,I,1,4,FVM), 0                         , 194, 0  , 7079 , 333, 131), // #1158
  INST(Vpcmpgtw         , VexRvm_Lx          , V(660F00,65,_,x,I,I,4,FV ), 0                         , 133, 0  , 7088 , 331, 146), // #1159
  INST(Vpcmpistri       , VexRmi             , V(660F3A,63,_,0,I,_,_,_  ), 0                         , 72 , 0  , 7097 , 336, 150), // #1160
  INST(Vpcmpistrm       , VexRmi             , V(660F3A,62,_,0,I,_,_,_  ), 0                         , 72 , 0  , 7108 , 337, 150), // #1161
  INST(Vpcmpq           , VexRvmi_Lx         , E(660F3A,1F,_,x,_,1,4,FV ), 0                         , 109, 0  , 7119 , 338, 126), // #1162
  INST(Vpcmpub          , VexRvmi_Lx         , E(660F3A,3E,_,x,_,0,4,FVM), 0                         , 151, 0  , 7126 , 329, 128), // #1163
  INST(Vpcmpud          , VexRvmi_Lx         , E(660F3A,1E,_,x,_,0,4,FV ), 0                         , 108, 0  , 7134 , 330, 126), // #1164
  INST(Vpcmpuq          , VexRvmi_Lx         , E(660F3A,1E,_,x,_,1,4,FV ), 0                         , 109, 0  , 7142 , 338, 126), // #1165
  INST(Vpcmpuw          , VexRvmi_Lx         , E(660F3A,3E,_,x,_,1,4,FVM), 0                         , 195, 0  , 7150 , 338, 128), // #1166
  INST(Vpcmpw           , VexRvmi_Lx         , E(660F3A,3F,_,x,_,1,4,FVM), 0                         , 195, 0  , 7158 , 338, 128), // #1167
  INST(Vpcomb           , VexRvmi            , V(XOP_M8,CC,_,0,0,_,_,_  ), 0                         , 193, 0  , 7165 , 252, 139), // #1168
  INST(Vpcomd           , VexRvmi            , V(XOP_M8,CE,_,0,0,_,_,_  ), 0                         , 193, 0  , 7172 , 252, 139), // #1169
  INST(Vpcompressb      , VexMr_Lx           , E(660F38,63,_,x,_,0,0,T1S), 0                         , 196, 0  , 7179 , 221, 151), // #1170
  INST(Vpcompressd      , VexMr_Lx           , E(660F38,8B,_,x,_,0,2,T1S), 0                         , 125, 0  , 7191 , 221, 126), // #1171
  INST(Vpcompressq      , VexMr_Lx           , E(660F38,8B,_,x,_,1,3,T1S), 0                         , 124, 0  , 7203 , 221, 126), // #1172
  INST(Vpcompressw      , VexMr_Lx           , E(660F38,63,_,x,_,1,1,T1S), 0                         , 197, 0  , 7215 , 221, 151), // #1173
  INST(Vpcomq           , VexRvmi            , V(XOP_M8,CF,_,0,0,_,_,_  ), 0                         , 193, 0  , 7227 , 252, 139), // #1174
  INST(Vpcomub          , VexRvmi            , V(XOP_M8,EC,_,0,0,_,_,_  ), 0                         , 193, 0  , 7234 , 252, 139), // #1175
  INST(Vpcomud          , VexRvmi            , V(XOP_M8,EE,_,0,0,_,_,_  ), 0                         , 193, 0  , 7242 , 252, 139), // #1176
  INST(Vpcomuq          , VexRvmi            , V(XOP_M8,EF,_,0,0,_,_,_  ), 0                         , 193, 0  , 7250 , 252, 139), // #1177
  INST(Vpcomuw          , VexRvmi            , V(XOP_M8,ED,_,0,0,_,_,_  ), 0                         , 193, 0  , 7258 , 252, 139), // #1178
  INST(Vpcomw           , VexRvmi            , V(XOP_M8,CD,_,0,0,_,_,_  ), 0                         , 193, 0  , 7266 , 252, 139), // #1179
  INST(Vpconflictd      , VexRm_Lx           , E(660F38,C4,_,x,_,0,4,FV ), 0                         , 111, 0  , 7273 , 339, 148), // #1180
  INST(Vpconflictq      , VexRm_Lx           , E(660F38,C4,_,x,_,1,4,FV ), 0                         , 112, 0  , 7285 , 339, 148), // #1181
  INST(Vpdpbusd         , VexRvm_Lx          , V(660F38,50,_,x,_,0,4,FV ), 0                         , 163, 0  , 7297 , 340, 152), // #1182
  INST(Vpdpbusds        , VexRvm_Lx          , V(660F38,51,_,x,_,0,4,FV ), 0                         , 163, 0  , 7306 , 340, 152), // #1183
  INST(Vpdpwssd         , VexRvm_Lx          , V(660F38,52,_,x,_,0,4,FV ), 0                         , 163, 0  , 7316 , 340, 152), // #1184
  INST(Vpdpwssds        , VexRvm_Lx          , V(660F38,53,_,x,_,0,4,FV ), 0                         , 163, 0  , 7325 , 340, 152), // #1185
  INST(Vperm2f128       , VexRvmi            , V(660F3A,06,_,1,0,_,_,_  ), 0                         , 154, 0  , 7335 , 341, 123), // #1186
  INST(Vperm2i128       , VexRvmi            , V(660F3A,46,_,1,0,_,_,_  ), 0                         , 154, 0  , 7346 , 341, 130), // #1187
  INST(Vpermb           , VexRvm_Lx          , E(660F38,8D,_,x,_,0,4,FVM), 0                         , 110, 0  , 7357 , 203, 153), // #1188
  INST(Vpermd           , VexRvm_Lx          , V(660F38,36,_,x,0,0,4,FV ), 0                         , 163, 0  , 7364 , 342, 140), // #1189
  INST(Vpermi2b         , VexRvm_Lx          , E(660F38,75,_,x,_,0,4,FVM), 0                         , 110, 0  , 7371 , 203, 153), // #1190
  INST(Vpermi2d         , VexRvm_Lx          , E(660F38,76,_,x,_,0,4,FV ), 0                         , 111, 0  , 7380 , 204, 126), // #1191
  INST(Vpermi2pd        , VexRvm_Lx          , E(660F38,77,_,x,_,1,4,FV ), 0                         , 112, 0  , 7389 , 205, 126), // #1192
  INST(Vpermi2ps        , VexRvm_Lx          , E(660F38,77,_,x,_,0,4,FV ), 0                         , 111, 0  , 7399 , 204, 126), // #1193
  INST(Vpermi2q         , VexRvm_Lx          , E(660F38,76,_,x,_,1,4,FV ), 0                         , 112, 0  , 7409 , 205, 126), // #1194
  INST(Vpermi2w         , VexRvm_Lx          , E(660F38,75,_,x,_,1,4,FVM), 0                         , 113, 0  , 7418 , 203, 128), // #1195
  INST(Vpermil2pd       , VexRvrmiRvmri_Lx   , V(660F3A,49,_,x,x,_,_,_  ), 0                         , 72 , 0  , 7427 , 343, 139), // #1196
  INST(Vpermil2ps       , VexRvrmiRvmri_Lx   , V(660F3A,48,_,x,x,_,_,_  ), 0                         , 72 , 0  , 7438 , 343, 139), // #1197
  INST(Vpermilpd        , VexRvmRmi_Lx       , V(660F38,0D,_,x,0,1,4,FV ), V(660F3A,05,_,x,0,1,4,FV ), 198, 109, 7449 , 344, 121), // #1198
  INST(Vpermilps        , VexRvmRmi_Lx       , V(660F38,0C,_,x,0,0,4,FV ), V(660F3A,04,_,x,0,0,4,FV ), 163, 110, 7459 , 344, 121), // #1199
  INST(Vpermpd          , VexRvmRmi_Lx       , E(660F38,16,_,x,1,1,4,FV ), V(660F3A,01,_,x,1,1,4,FV ), 199, 111, 7469 , 345, 140), // #1200
  INST(Vpermps          , VexRvm_Lx          , V(660F38,16,_,x,0,0,4,FV ), 0                         , 163, 0  , 7477 , 342, 140), // #1201
  INST(Vpermq           , VexRvmRmi_Lx       , V(660F38,36,_,x,_,1,4,FV ), V(660F3A,00,_,x,1,1,4,FV ), 198, 112, 7485 , 345, 140), // #1202
  INST(Vpermt2b         , VexRvm_Lx          , E(660F38,7D,_,x,_,0,4,FVM), 0                         , 110, 0  , 7492 , 203, 153), // #1203
  INST(Vpermt2d         , VexRvm_Lx          , E(660F38,7E,_,x,_,0,4,FV ), 0                         , 111, 0  , 7501 , 204, 126), // #1204
  INST(Vpermt2pd        , VexRvm_Lx          , E(660F38,7F,_,x,_,1,4,FV ), 0                         , 112, 0  , 7510 , 205, 126), // #1205
  INST(Vpermt2ps        , VexRvm_Lx          , E(660F38,7F,_,x,_,0,4,FV ), 0                         , 111, 0  , 7520 , 204, 126), // #1206
  INST(Vpermt2q         , VexRvm_Lx          , E(660F38,7E,_,x,_,1,4,FV ), 0                         , 112, 0  , 7530 , 205, 126), // #1207
  INST(Vpermt2w         , VexRvm_Lx          , E(660F38,7D,_,x,_,1,4,FVM), 0                         , 113, 0  , 7539 , 203, 128), // #1208
  INST(Vpermw           , VexRvm_Lx          , E(660F38,8D,_,x,_,1,4,FVM), 0                         , 113, 0  , 7548 , 203, 128), // #1209
  INST(Vpexpandb        , VexRm_Lx           , E(660F38,62,_,x,_,0,0,T1S), 0                         , 196, 0  , 7555 , 255, 151), // #1210
  INST(Vpexpandd        , VexRm_Lx           , E(660F38,89,_,x,_,0,2,T1S), 0                         , 125, 0  , 7565 , 255, 126), // #1211
  INST(Vpexpandq        , VexRm_Lx           , E(660F38,89,_,x,_,1,3,T1S), 0                         , 124, 0  , 7575 , 255, 126), // #1212
  INST(Vpexpandw        , VexRm_Lx           , E(660F38,62,_,x,_,1,1,T1S), 0                         , 197, 0  , 7585 , 255, 151), // #1213
  INST(Vpextrb          , VexMri             , V(660F3A,14,_,0,0,I,0,T1S), 0                         , 200, 0  , 7595 , 346, 154), // #1214
  INST(Vpextrd          , VexMri             , V(660F3A,16,_,0,0,0,2,T1S), 0                         , 159, 0  , 7603 , 259, 155), // #1215
  INST(Vpextrq          , VexMri             , V(660F3A,16,_,0,1,1,3,T1S), 0                         , 201, 0  , 7611 , 347, 155), // #1216
  INST(Vpextrw          , VexMri             , V(660F3A,15,_,0,0,I,1,T1S), 0                         , 202, 0  , 7619 , 348, 154), // #1217
  INST(Vpgatherdd       , VexRmvRm_VM        , V(660F38,90,_,x,0,_,_,_  ), V(660F38,90,_,x,_,0,2,T1S), 96 , 113, 7627 , 275, 140), // #1218
  INST(Vpgatherdq       , VexRmvRm_VM        , V(660F38,90,_,x,1,_,_,_  ), V(660F38,90,_,x,_,1,3,T1S), 165, 114, 7638 , 274, 140), // #1219
  INST(Vpgatherqd       , VexRmvRm_VM        , V(660F38,91,_,x,0,_,_,_  ), V(660F38,91,_,x,_,0,2,T1S), 96 , 115, 7649 , 280, 140), // #1220
  INST(Vpgatherqq       , VexRmvRm_VM        , V(660F38,91,_,x,1,_,_,_  ), V(660F38,91,_,x,_,1,3,T1S), 165, 116, 7660 , 279, 140), // #1221
  INST(Vphaddbd         , VexRm              , V(XOP_M9,C2,_,0,0,_,_,_  ), 0                         , 77 , 0  , 7671 , 195, 139), // #1222
  INST(Vphaddbq         , VexRm              , V(XOP_M9,C3,_,0,0,_,_,_  ), 0                         , 77 , 0  , 7680 , 195, 139), // #1223
  INST(Vphaddbw         , VexRm              , V(XOP_M9,C1,_,0,0,_,_,_  ), 0                         , 77 , 0  , 7689 , 195, 139), // #1224
  INST(Vphaddd          , VexRvm_Lx          , V(660F38,02,_,x,I,_,_,_  ), 0                         , 96 , 0  , 7698 , 193, 143), // #1225
  INST(Vphadddq         , VexRm              , V(XOP_M9,CB,_,0,0,_,_,_  ), 0                         , 77 , 0  , 7706 , 195, 139), // #1226
  INST(Vphaddsw         , VexRvm_Lx          , V(660F38,03,_,x,I,_,_,_  ), 0                         , 96 , 0  , 7715 , 193, 143), // #1227
  INST(Vphaddubd        , VexRm              , V(XOP_M9,D2,_,0,0,_,_,_  ), 0                         , 77 , 0  , 7724 , 195, 139), // #1228
  INST(Vphaddubq        , VexRm              , V(XOP_M9,D3,_,0,0,_,_,_  ), 0                         , 77 , 0  , 7734 , 195, 139), // #1229
  INST(Vphaddubw        , VexRm              , V(XOP_M9,D1,_,0,0,_,_,_  ), 0                         , 77 , 0  , 7744 , 195, 139), // #1230
  INST(Vphaddudq        , VexRm              , V(XOP_M9,DB,_,0,0,_,_,_  ), 0                         , 77 , 0  , 7754 , 195, 139), // #1231
  INST(Vphadduwd        , VexRm              , V(XOP_M9,D6,_,0,0,_,_,_  ), 0                         , 77 , 0  , 7764 , 195, 139), // #1232
  INST(Vphadduwq        , VexRm              , V(XOP_M9,D7,_,0,0,_,_,_  ), 0                         , 77 , 0  , 7774 , 195, 139), // #1233
  INST(Vphaddw          , VexRvm_Lx          , V(660F38,01,_,x,I,_,_,_  ), 0                         , 96 , 0  , 7784 , 193, 143), // #1234
  INST(Vphaddwd         , VexRm              , V(XOP_M9,C6,_,0,0,_,_,_  ), 0                         , 77 , 0  , 7792 , 195, 139), // #1235
  INST(Vphaddwq         , VexRm              , V(XOP_M9,C7,_,0,0,_,_,_  ), 0                         , 77 , 0  , 7801 , 195, 139), // #1236
  INST(Vphminposuw      , VexRm              , V(660F38,41,_,0,I,_,_,_  ), 0                         , 96 , 0  , 7810 , 195, 123), // #1237
  INST(Vphsubbw         , VexRm              , V(XOP_M9,E1,_,0,0,_,_,_  ), 0                         , 77 , 0  , 7822 , 195, 139), // #1238
  INST(Vphsubd          , VexRvm_Lx          , V(660F38,06,_,x,I,_,_,_  ), 0                         , 96 , 0  , 7831 , 193, 143), // #1239
  INST(Vphsubdq         , VexRm              , V(XOP_M9,E3,_,0,0,_,_,_  ), 0                         , 77 , 0  , 7839 , 195, 139), // #1240
  INST(Vphsubsw         , VexRvm_Lx          , V(660F38,07,_,x,I,_,_,_  ), 0                         , 96 , 0  , 7848 , 193, 143), // #1241
  INST(Vphsubw          , VexRvm_Lx          , V(660F38,05,_,x,I,_,_,_  ), 0                         , 96 , 0  , 7857 , 193, 143), // #1242
  INST(Vphsubwd         , VexRm              , V(XOP_M9,E2,_,0,0,_,_,_  ), 0                         , 77 , 0  , 7865 , 195, 139), // #1243
  INST(Vpinsrb          , VexRvmi            , V(660F3A,20,_,0,0,I,0,T1S), 0                         , 200, 0  , 7874 , 349, 154), // #1244
  INST(Vpinsrd          , VexRvmi            , V(660F3A,22,_,0,0,0,2,T1S), 0                         , 159, 0  , 7882 , 350, 155), // #1245
  INST(Vpinsrq          , VexRvmi            , V(660F3A,22,_,0,1,1,3,T1S), 0                         , 201, 0  , 7890 , 351, 155), // #1246
  INST(Vpinsrw          , VexRvmi            , V(660F00,C4,_,0,0,I,1,T1S), 0                         , 203, 0  , 7898 , 352, 154), // #1247
  INST(Vplzcntd         , VexRm_Lx           , E(660F38,44,_,x,_,0,4,FV ), 0                         , 111, 0  , 7906 , 339, 148), // #1248
  INST(Vplzcntq         , VexRm_Lx           , E(660F38,44,_,x,_,1,4,FV ), 0                         , 112, 0  , 7915 , 353, 148), // #1249
  INST(Vpmacsdd         , VexRvmr            , V(XOP_M8,9E,_,0,0,_,_,_  ), 0                         , 193, 0  , 7924 , 354, 139), // #1250
  INST(Vpmacsdqh        , VexRvmr            , V(XOP_M8,9F,_,0,0,_,_,_  ), 0                         , 193, 0  , 7933 , 354, 139), // #1251
  INST(Vpmacsdql        , VexRvmr            , V(XOP_M8,97,_,0,0,_,_,_  ), 0                         , 193, 0  , 7943 , 354, 139), // #1252
  INST(Vpmacssdd        , VexRvmr            , V(XOP_M8,8E,_,0,0,_,_,_  ), 0                         , 193, 0  , 7953 , 354, 139), // #1253
  INST(Vpmacssdqh       , VexRvmr            , V(XOP_M8,8F,_,0,0,_,_,_  ), 0                         , 193, 0  , 7963 , 354, 139), // #1254
  INST(Vpmacssdql       , VexRvmr            , V(XOP_M8,87,_,0,0,_,_,_  ), 0                         , 193, 0  , 7974 , 354, 139), // #1255
  INST(Vpmacsswd        , VexRvmr            , V(XOP_M8,86,_,0,0,_,_,_  ), 0                         , 193, 0  , 7985 , 354, 139), // #1256
  INST(Vpmacssww        , VexRvmr            , V(XOP_M8,85,_,0,0,_,_,_  ), 0                         , 193, 0  , 7995 , 354, 139), // #1257
  INST(Vpmacswd         , VexRvmr            , V(XOP_M8,96,_,0,0,_,_,_  ), 0                         , 193, 0  , 8005 , 354, 139), // #1258
  INST(Vpmacsww         , VexRvmr            , V(XOP_M8,95,_,0,0,_,_,_  ), 0                         , 193, 0  , 8014 , 354, 139), // #1259
  INST(Vpmadcsswd       , VexRvmr            , V(XOP_M8,A6,_,0,0,_,_,_  ), 0                         , 193, 0  , 8023 , 354, 139), // #1260
  INST(Vpmadcswd        , VexRvmr            , V(XOP_M8,B6,_,0,0,_,_,_  ), 0                         , 193, 0  , 8034 , 354, 139), // #1261
  INST(Vpmadd52huq      , VexRvm_Lx          , E(660F38,B5,_,x,_,1,4,FV ), 0                         , 112, 0  , 8044 , 205, 156), // #1262
  INST(Vpmadd52luq      , VexRvm_Lx          , E(660F38,B4,_,x,_,1,4,FV ), 0                         , 112, 0  , 8056 , 205, 156), // #1263
  INST(Vpmaddubsw       , VexRvm_Lx          , V(660F38,04,_,x,I,I,4,FVM), 0                         , 107, 0  , 8068 , 286, 146), // #1264
  INST(Vpmaddwd         , VexRvm_Lx          , V(660F00,F5,_,x,I,I,4,FVM), 0                         , 184, 0  , 8079 , 286, 146), // #1265
  INST(Vpmaskmovd       , VexRvmMvr_Lx       , V(660F38,8C,_,x,0,_,_,_  ), V(660F38,8E,_,x,0,_,_,_  ), 96 , 117, 8088 , 294, 130), // #1266
  INST(Vpmaskmovq       , VexRvmMvr_Lx       , V(660F38,8C,_,x,1,_,_,_  ), V(660F38,8E,_,x,1,_,_,_  ), 165, 118, 8099 , 294, 130), // #1267
  INST(Vpmaxsb          , VexRvm_Lx          , V(660F38,3C,_,x,I,I,4,FVM), 0                         , 107, 0  , 8110 , 355, 146), // #1268
  INST(Vpmaxsd          , VexRvm_Lx          , V(660F38,3D,_,x,I,0,4,FV ), 0                         , 163, 0  , 8118 , 202, 131), // #1269
  INST(Vpmaxsq          , VexRvm_Lx          , E(660F38,3D,_,x,_,1,4,FV ), 0                         , 112, 0  , 8126 , 205, 126), // #1270
  INST(Vpmaxsw          , VexRvm_Lx          , V(660F00,EE,_,x,I,I,4,FVM), 0                         , 184, 0  , 8134 , 355, 146), // #1271
  INST(Vpmaxub          , VexRvm_Lx          , V(660F00,DE,_,x,I,I,4,FVM), 0                         , 184, 0  , 8142 , 355, 146), // #1272
  INST(Vpmaxud          , VexRvm_Lx          , V(660F38,3F,_,x,I,0,4,FV ), 0                         , 163, 0  , 8150 , 202, 131), // #1273
  INST(Vpmaxuq          , VexRvm_Lx          , E(660F38,3F,_,x,_,1,4,FV ), 0                         , 112, 0  , 8158 , 205, 126), // #1274
  INST(Vpmaxuw          , VexRvm_Lx          , V(660F38,3E,_,x,I,I,4,FVM), 0                         , 107, 0  , 8166 , 355, 146), // #1275
  INST(Vpminsb          , VexRvm_Lx          , V(660F38,38,_,x,I,I,4,FVM), 0                         , 107, 0  , 8174 , 355, 146), // #1276
  INST(Vpminsd          , VexRvm_Lx          , V(660F38,39,_,x,I,0,4,FV ), 0                         , 163, 0  , 8182 , 202, 131), // #1277
  INST(Vpminsq          , VexRvm_Lx          , E(660F38,39,_,x,_,1,4,FV ), 0                         , 112, 0  , 8190 , 205, 126), // #1278
  INST(Vpminsw          , VexRvm_Lx          , V(660F00,EA,_,x,I,I,4,FVM), 0                         , 184, 0  , 8198 , 355, 146), // #1279
  INST(Vpminub          , VexRvm_Lx          , V(660F00,DA,_,x,I,_,4,FVM), 0                         , 184, 0  , 8206 , 355, 146), // #1280
  INST(Vpminud          , VexRvm_Lx          , V(660F38,3B,_,x,I,0,4,FV ), 0                         , 163, 0  , 8214 , 202, 131), // #1281
  INST(Vpminuq          , VexRvm_Lx          , E(660F38,3B,_,x,_,1,4,FV ), 0                         , 112, 0  , 8222 , 205, 126), // #1282
  INST(Vpminuw          , VexRvm_Lx          , V(660F38,3A,_,x,I,_,4,FVM), 0                         , 107, 0  , 8230 , 355, 146), // #1283
  INST(Vpmovb2m         , VexRm_Lx           , E(F30F38,29,_,x,_,0,_,_  ), 0                         , 128, 0  , 8238 , 356, 128), // #1284
  INST(Vpmovd2m         , VexRm_Lx           , E(F30F38,39,_,x,_,0,_,_  ), 0                         , 128, 0  , 8247 , 356, 129), // #1285
  INST(Vpmovdb          , VexMr_Lx           , E(F30F38,31,_,x,_,0,2,QVM), 0                         , 204, 0  , 8256 , 357, 126), // #1286
  INST(Vpmovdw          , VexMr_Lx           , E(F30F38,33,_,x,_,0,3,HVM), 0                         , 205, 0  , 8264 , 358, 126), // #1287
  INST(Vpmovm2b         , VexRm_Lx           , E(F30F38,28,_,x,_,0,_,_  ), 0                         , 128, 0  , 8272 , 325, 128), // #1288
  INST(Vpmovm2d         , VexRm_Lx           , E(F30F38,38,_,x,_,0,_,_  ), 0                         , 128, 0  , 8281 , 325, 129), // #1289
  INST(Vpmovm2q         , VexRm_Lx           , E(F30F38,38,_,x,_,1,_,_  ), 0                         , 191, 0  , 8290 , 325, 129), // #1290
  INST(Vpmovm2w         , VexRm_Lx           , E(F30F38,28,_,x,_,1,_,_  ), 0                         , 191, 0  , 8299 , 325, 128), // #1291
  INST(Vpmovmskb        , VexRm_Lx           , V(660F00,D7,_,x,I,_,_,_  ), 0                         , 68 , 0  , 8308 , 306, 143), // #1292
  INST(Vpmovq2m         , VexRm_Lx           , E(F30F38,39,_,x,_,1,_,_  ), 0                         , 191, 0  , 8318 , 356, 129), // #1293
  INST(Vpmovqb          , VexMr_Lx           , E(F30F38,32,_,x,_,0,1,OVM), 0                         , 206, 0  , 8327 , 359, 126), // #1294
  INST(Vpmovqd          , VexMr_Lx           , E(F30F38,35,_,x,_,0,3,HVM), 0                         , 205, 0  , 8335 , 358, 126), // #1295
  INST(Vpmovqw          , VexMr_Lx           , E(F30F38,34,_,x,_,0,2,QVM), 0                         , 204, 0  , 8343 , 357, 126), // #1296
  INST(Vpmovsdb         , VexMr_Lx           , E(F30F38,21,_,x,_,0,2,QVM), 0                         , 204, 0  , 8351 , 357, 126), // #1297
  INST(Vpmovsdw         , VexMr_Lx           , E(F30F38,23,_,x,_,0,3,HVM), 0                         , 205, 0  , 8360 , 358, 126), // #1298
  INST(Vpmovsqb         , VexMr_Lx           , E(F30F38,22,_,x,_,0,1,OVM), 0                         , 206, 0  , 8369 , 359, 126), // #1299
  INST(Vpmovsqd         , VexMr_Lx           , E(F30F38,25,_,x,_,0,3,HVM), 0                         , 205, 0  , 8378 , 358, 126), // #1300
  INST(Vpmovsqw         , VexMr_Lx           , E(F30F38,24,_,x,_,0,2,QVM), 0                         , 204, 0  , 8387 , 357, 126), // #1301
  INST(Vpmovswb         , VexMr_Lx           , E(F30F38,20,_,x,_,0,3,HVM), 0                         , 205, 0  , 8396 , 358, 128), // #1302
  INST(Vpmovsxbd        , VexRm_Lx           , V(660F38,21,_,x,I,I,2,QVM), 0                         , 207, 0  , 8405 , 360, 131), // #1303
  INST(Vpmovsxbq        , VexRm_Lx           , V(660F38,22,_,x,I,I,1,OVM), 0                         , 208, 0  , 8415 , 361, 131), // #1304
  INST(Vpmovsxbw        , VexRm_Lx           , V(660F38,20,_,x,I,I,3,HVM), 0                         , 132, 0  , 8425 , 362, 146), // #1305
  INST(Vpmovsxdq        , VexRm_Lx           , V(660F38,25,_,x,I,0,3,HVM), 0                         , 132, 0  , 8435 , 362, 131), // #1306
  INST(Vpmovsxwd        , VexRm_Lx           , V(660F38,23,_,x,I,I,3,HVM), 0                         , 132, 0  , 8445 , 362, 131), // #1307
  INST(Vpmovsxwq        , VexRm_Lx           , V(660F38,24,_,x,I,I,2,QVM), 0                         , 207, 0  , 8455 , 360, 131), // #1308
  INST(Vpmovusdb        , VexMr_Lx           , E(F30F38,11,_,x,_,0,2,QVM), 0                         , 204, 0  , 8465 , 357, 126), // #1309
  INST(Vpmovusdw        , VexMr_Lx           , E(F30F38,13,_,x,_,0,3,HVM), 0                         , 205, 0  , 8475 , 358, 126), // #1310
  INST(Vpmovusqb        , VexMr_Lx           , E(F30F38,12,_,x,_,0,1,OVM), 0                         , 206, 0  , 8485 , 359, 126), // #1311
  INST(Vpmovusqd        , VexMr_Lx           , E(F30F38,15,_,x,_,0,3,HVM), 0                         , 205, 0  , 8495 , 358, 126), // #1312
  INST(Vpmovusqw        , VexMr_Lx           , E(F30F38,14,_,x,_,0,2,QVM), 0                         , 204, 0  , 8505 , 357, 126), // #1313
  INST(Vpmovuswb        , VexMr_Lx           , E(F30F38,10,_,x,_,0,3,HVM), 0                         , 205, 0  , 8515 , 358, 128), // #1314
  INST(Vpmovw2m         , VexRm_Lx           , E(F30F38,29,_,x,_,1,_,_  ), 0                         , 191, 0  , 8525 , 356, 128), // #1315
  INST(Vpmovwb          , VexMr_Lx           , E(F30F38,30,_,x,_,0,3,HVM), 0                         , 205, 0  , 8534 , 358, 128), // #1316
  INST(Vpmovzxbd        , VexRm_Lx           , V(660F38,31,_,x,I,I,2,QVM), 0                         , 207, 0  , 8542 , 360, 131), // #1317
  INST(Vpmovzxbq        , VexRm_Lx           , V(660F38,32,_,x,I,I,1,OVM), 0                         , 208, 0  , 8552 , 361, 131), // #1318
  INST(Vpmovzxbw        , VexRm_Lx           , V(660F38,30,_,x,I,I,3,HVM), 0                         , 132, 0  , 8562 , 362, 146), // #1319
  INST(Vpmovzxdq        , VexRm_Lx           , V(660F38,35,_,x,I,0,3,HVM), 0                         , 132, 0  , 8572 , 362, 131), // #1320
  INST(Vpmovzxwd        , VexRm_Lx           , V(660F38,33,_,x,I,I,3,HVM), 0                         , 132, 0  , 8582 , 362, 131), // #1321
  INST(Vpmovzxwq        , VexRm_Lx           , V(660F38,34,_,x,I,I,2,QVM), 0                         , 207, 0  , 8592 , 360, 131), // #1322
  INST(Vpmuldq          , VexRvm_Lx          , V(660F38,28,_,x,I,1,4,FV ), 0                         , 198, 0  , 8602 , 199, 131), // #1323
  INST(Vpmulhrsw        , VexRvm_Lx          , V(660F38,0B,_,x,I,I,4,FVM), 0                         , 107, 0  , 8610 , 286, 146), // #1324
  INST(Vpmulhuw         , VexRvm_Lx          , V(660F00,E4,_,x,I,I,4,FVM), 0                         , 184, 0  , 8620 , 286, 146), // #1325
  INST(Vpmulhw          , VexRvm_Lx          , V(660F00,E5,_,x,I,I,4,FVM), 0                         , 184, 0  , 8629 , 286, 146), // #1326
  INST(Vpmulld          , VexRvm_Lx          , V(660F38,40,_,x,I,0,4,FV ), 0                         , 163, 0  , 8637 , 200, 131), // #1327
  INST(Vpmullq          , VexRvm_Lx          , E(660F38,40,_,x,_,1,4,FV ), 0                         , 112, 0  , 8645 , 205, 129), // #1328
  INST(Vpmullw          , VexRvm_Lx          , V(660F00,D5,_,x,I,I,4,FVM), 0                         , 184, 0  , 8653 , 286, 146), // #1329
  INST(Vpmultishiftqb   , VexRvm_Lx          , E(660F38,83,_,x,_,1,4,FV ), 0                         , 112, 0  , 8661 , 205, 153), // #1330
  INST(Vpmuludq         , VexRvm_Lx          , V(660F00,F4,_,x,I,1,4,FV ), 0                         , 102, 0  , 8676 , 199, 131), // #1331
  INST(Vpopcntb         , VexRm_Lx           , E(660F38,54,_,x,_,0,4,FV ), 0                         , 111, 0  , 8685 , 255, 157), // #1332
  INST(Vpopcntd         , VexRm_Lx           , E(660F38,55,_,x,_,0,4,FVM), 0                         , 110, 0  , 8694 , 339, 158), // #1333
  INST(Vpopcntq         , VexRm_Lx           , E(660F38,55,_,x,_,1,4,FVM), 0                         , 113, 0  , 8703 , 353, 158), // #1334
  INST(Vpopcntw         , VexRm_Lx           , E(660F38,54,_,x,_,1,4,FV ), 0                         , 112, 0  , 8712 , 255, 157), // #1335
  INST(Vpor             , VexRvm_Lx          , V(660F00,EB,_,x,I,_,_,_  ), 0                         , 68 , 0  , 8721 , 317, 143), // #1336
  INST(Vpord            , VexRvm_Lx          , E(660F00,EB,_,x,_,0,4,FV ), 0                         , 189, 0  , 8726 , 318, 126), // #1337
  INST(Vporq            , VexRvm_Lx          , E(660F00,EB,_,x,_,1,4,FV ), 0                         , 130, 0  , 8732 , 322, 126), // #1338
  INST(Vpperm           , VexRvrmRvmr        , V(XOP_M8,A3,_,0,x,_,_,_  ), 0                         , 193, 0  , 8738 , 363, 139), // #1339
  INST(Vprold           , VexVmi_Lx          , E(660F00,72,1,x,_,0,4,FV ), 0                         , 209, 0  , 8745 , 364, 126), // #1340
  INST(Vprolq           , VexVmi_Lx          , E(660F00,72,1,x,_,1,4,FV ), 0                         , 210, 0  , 8752 , 365, 126), // #1341
  INST(Vprolvd          , VexRvm_Lx          , E(660F38,15,_,x,_,0,4,FV ), 0                         , 111, 0  , 8759 , 204, 126), // #1342
  INST(Vprolvq          , VexRvm_Lx          , E(660F38,15,_,x,_,1,4,FV ), 0                         , 112, 0  , 8767 , 205, 126), // #1343
  INST(Vprord           , VexVmi_Lx          , E(660F00,72,0,x,_,0,4,FV ), 0                         , 189, 0  , 8775 , 364, 126), // #1344
  INST(Vprorq           , VexVmi_Lx          , E(660F00,72,0,x,_,1,4,FV ), 0                         , 130, 0  , 8782 , 365, 126), // #1345
  INST(Vprorvd          , VexRvm_Lx          , E(660F38,14,_,x,_,0,4,FV ), 0                         , 111, 0  , 8789 , 204, 126), // #1346
  INST(Vprorvq          , VexRvm_Lx          , E(660F38,14,_,x,_,1,4,FV ), 0                         , 112, 0  , 8797 , 205, 126), // #1347
  INST(Vprotb           , VexRvmRmvRmi       , V(XOP_M9,90,_,0,x,_,_,_  ), V(XOP_M8,C0,_,0,x,_,_,_  ), 77 , 119, 8805 , 366, 139), // #1348
  INST(Vprotd           , VexRvmRmvRmi       , V(XOP_M9,92,_,0,x,_,_,_  ), V(XOP_M8,C2,_,0,x,_,_,_  ), 77 , 120, 8812 , 366, 139), // #1349
  INST(Vprotq           , VexRvmRmvRmi       , V(XOP_M9,93,_,0,x,_,_,_  ), V(XOP_M8,C3,_,0,x,_,_,_  ), 77 , 121, 8819 , 366, 139), // #1350
  INST(Vprotw           , VexRvmRmvRmi       , V(XOP_M9,91,_,0,x,_,_,_  ), V(XOP_M8,C1,_,0,x,_,_,_  ), 77 , 122, 8826 , 366, 139), // #1351
  INST(Vpsadbw          , VexRvm_Lx          , V(660F00,F6,_,x,I,I,4,FVM), 0                         , 184, 0  , 8833 , 194, 146), // #1352
  INST(Vpscatterdd      , VexMr_VM           , E(660F38,A0,_,x,_,0,2,T1S), 0                         , 125, 0  , 8841 , 367, 126), // #1353
  INST(Vpscatterdq      , VexMr_VM           , E(660F38,A0,_,x,_,1,3,T1S), 0                         , 124, 0  , 8853 , 367, 126), // #1354
  INST(Vpscatterqd      , VexMr_VM           , E(660F38,A1,_,x,_,0,2,T1S), 0                         , 125, 0  , 8865 , 368, 126), // #1355
  INST(Vpscatterqq      , VexMr_VM           , E(660F38,A1,_,x,_,1,3,T1S), 0                         , 124, 0  , 8877 , 369, 126), // #1356
  INST(Vpshab           , VexRvmRmv          , V(XOP_M9,98,_,0,x,_,_,_  ), 0                         , 77 , 0  , 8889 , 370, 139), // #1357
  INST(Vpshad           , VexRvmRmv          , V(XOP_M9,9A,_,0,x,_,_,_  ), 0                         , 77 , 0  , 8896 , 370, 139), // #1358
  INST(Vpshaq           , VexRvmRmv          , V(XOP_M9,9B,_,0,x,_,_,_  ), 0                         , 77 , 0  , 8903 , 370, 139), // #1359
  INST(Vpshaw           , VexRvmRmv          , V(XOP_M9,99,_,0,x,_,_,_  ), 0                         , 77 , 0  , 8910 , 370, 139), // #1360
  INST(Vpshlb           , VexRvmRmv          , V(XOP_M9,94,_,0,x,_,_,_  ), 0                         , 77 , 0  , 8917 , 370, 139), // #1361
  INST(Vpshld           , VexRvmRmv          , V(XOP_M9,96,_,0,x,_,_,_  ), 0                         , 77 , 0  , 8924 , 370, 139), // #1362
  INST(Vpshldd          , VexRvmi_Lx         , E(660F3A,71,_,x,_,0,4,FV ), 0                         , 108, 0  , 8931 , 197, 151), // #1363
  INST(Vpshldq          , VexRvmi_Lx         , E(660F3A,71,_,x,_,1,4,FV ), 0                         , 109, 0  , 8939 , 198, 151), // #1364
  INST(Vpshldvd         , VexRvm_Lx          , E(660F38,71,_,x,_,0,4,FV ), 0                         , 111, 0  , 8947 , 204, 151), // #1365
  INST(Vpshldvq         , VexRvm_Lx          , E(660F38,71,_,x,_,1,4,FV ), 0                         , 112, 0  , 8956 , 205, 151), // #1366
  INST(Vpshldvw         , VexRvm_Lx          , E(660F38,70,_,x,_,0,4,FVM), 0                         , 110, 0  , 8965 , 203, 151), // #1367
  INST(Vpshldw          , VexRvmi_Lx         , E(660F3A,70,_,x,_,0,4,FVM), 0                         , 151, 0  , 8974 , 251, 151), // #1368
  INST(Vpshlq           , VexRvmRmv          , V(XOP_M9,97,_,0,x,_,_,_  ), 0                         , 77 , 0  , 8982 , 370, 139), // #1369
  INST(Vpshlw           , VexRvmRmv          , V(XOP_M9,95,_,0,x,_,_,_  ), 0                         , 77 , 0  , 8989 , 370, 139), // #1370
  INST(Vpshrdd          , VexRvmi_Lx         , E(660F3A,73,_,x,_,0,4,FV ), 0                         , 108, 0  , 8996 , 197, 151), // #1371
  INST(Vpshrdq          , VexRvmi_Lx         , E(660F3A,73,_,x,_,1,4,FV ), 0                         , 109, 0  , 9004 , 198, 151), // #1372
  INST(Vpshrdvd         , VexRvm_Lx          , E(660F38,73,_,x,_,0,4,FV ), 0                         , 111, 0  , 9012 , 204, 151), // #1373
  INST(Vpshrdvq         , VexRvm_Lx          , E(660F38,73,_,x,_,1,4,FV ), 0                         , 112, 0  , 9021 , 205, 151), // #1374
  INST(Vpshrdvw         , VexRvm_Lx          , E(660F38,72,_,x,_,0,4,FVM), 0                         , 110, 0  , 9030 , 203, 151), // #1375
  INST(Vpshrdw          , VexRvmi_Lx         , E(660F3A,72,_,x,_,0,4,FVM), 0                         , 151, 0  , 9039 , 251, 151), // #1376
  INST(Vpshufb          , VexRvm_Lx          , V(660F38,00,_,x,I,I,4,FVM), 0                         , 107, 0  , 9047 , 286, 146), // #1377
  INST(Vpshufbitqmb     , VexRvm_Lx          , E(660F38,8F,_,x,0,0,4,FVM), 0                         , 110, 0  , 9055 , 371, 157), // #1378
  INST(Vpshufd          , VexRmi_Lx          , V(660F00,70,_,x,I,0,4,FV ), 0                         , 133, 0  , 9068 , 372, 131), // #1379
  INST(Vpshufhw         , VexRmi_Lx          , V(F30F00,70,_,x,I,I,4,FVM), 0                         , 185, 0  , 9076 , 373, 146), // #1380
  INST(Vpshuflw         , VexRmi_Lx          , V(F20F00,70,_,x,I,I,4,FVM), 0                         , 211, 0  , 9085 , 373, 146), // #1381
  INST(Vpsignb          , VexRvm_Lx          , V(660F38,08,_,x,I,_,_,_  ), 0                         , 96 , 0  , 9094 , 193, 143), // #1382
  INST(Vpsignd          , VexRvm_Lx          , V(660F38,0A,_,x,I,_,_,_  ), 0                         , 96 , 0  , 9102 , 193, 143), // #1383
  INST(Vpsignw          , VexRvm_Lx          , V(660F38,09,_,x,I,_,_,_  ), 0                         , 96 , 0  , 9110 , 193, 143), // #1384
  INST(Vpslld           , VexRvmVmi_Lx       , V(660F00,F2,_,x,I,0,4,128), V(660F00,72,6,x,I,0,4,FV ), 212, 123, 9118 , 374, 131), // #1385
  INST(Vpslldq          , VexEvexVmi_Lx      , V(660F00,73,7,x,I,I,4,FVM), 0                         , 213, 0  , 9125 , 375, 146), // #1386
  INST(Vpsllq           , VexRvmVmi_Lx       , V(660F00,F3,_,x,I,1,4,128), V(660F00,73,6,x,I,1,4,FV ), 214, 124, 9133 , 376, 131), // #1387
  INST(Vpsllvd          , VexRvm_Lx          , V(660F38,47,_,x,0,0,4,FV ), 0                         , 163, 0  , 9140 , 200, 140), // #1388
  INST(Vpsllvq          , VexRvm_Lx          , V(660F38,47,_,x,1,1,4,FV ), 0                         , 162, 0  , 9148 , 199, 140), // #1389
  INST(Vpsllvw          , VexRvm_Lx          , E(660F38,12,_,x,_,1,4,FVM), 0                         , 113, 0  , 9156 , 203, 128), // #1390
  INST(Vpsllw           , VexRvmVmi_Lx       , V(660F00,F1,_,x,I,I,4,FVM), V(660F00,71,6,x,I,I,4,FVM), 184, 125, 9164 , 377, 146), // #1391
  INST(Vpsrad           , VexRvmVmi_Lx       , V(660F00,E2,_,x,I,0,4,128), V(660F00,72,4,x,I,0,4,FV ), 212, 126, 9171 , 374, 131), // #1392
  INST(Vpsraq           , VexRvmVmi_Lx       , E(660F00,E2,_,x,_,1,4,128), E(660F00,72,4,x,_,1,4,FV ), 215, 127, 9178 , 378, 126), // #1393
  INST(Vpsravd          , VexRvm_Lx          , V(660F38,46,_,x,0,0,4,FV ), 0                         , 163, 0  , 9185 , 200, 140), // #1394
  INST(Vpsravq          , VexRvm_Lx          , E(660F38,46,_,x,_,1,4,FV ), 0                         , 112, 0  , 9193 , 205, 126), // #1395
  INST(Vpsravw          , VexRvm_Lx          , E(660F38,11,_,x,_,1,4,FVM), 0                         , 113, 0  , 9201 , 203, 128), // #1396
  INST(Vpsraw           , VexRvmVmi_Lx       , V(660F00,E1,_,x,I,I,4,128), V(660F00,71,4,x,I,I,4,FVM), 212, 128, 9209 , 377, 146), // #1397
  INST(Vpsrld           , VexRvmVmi_Lx       , V(660F00,D2,_,x,I,0,4,128), V(660F00,72,2,x,I,0,4,FV ), 212, 129, 9216 , 374, 131), // #1398
  INST(Vpsrldq          , VexEvexVmi_Lx      , V(660F00,73,3,x,I,I,4,FVM), 0                         , 216, 0  , 9223 , 375, 146), // #1399
  INST(Vpsrlq           , VexRvmVmi_Lx       , V(660F00,D3,_,x,I,1,4,128), V(660F00,73,2,x,I,1,4,FV ), 214, 130, 9231 , 376, 131), // #1400
  INST(Vpsrlvd          , VexRvm_Lx          , V(660F38,45,_,x,0,0,4,FV ), 0                         , 163, 0  , 9238 , 200, 140), // #1401
  INST(Vpsrlvq          , VexRvm_Lx          , V(660F38,45,_,x,1,1,4,FV ), 0                         , 162, 0  , 9246 , 199, 140), // #1402
  INST(Vpsrlvw          , VexRvm_Lx          , E(660F38,10,_,x,_,1,4,FVM), 0                         , 113, 0  , 9254 , 203, 128), // #1403
  INST(Vpsrlw           , VexRvmVmi_Lx       , V(660F00,D1,_,x,I,I,4,128), V(660F00,71,2,x,I,I,4,FVM), 212, 131, 9262 , 377, 146), // #1404
  INST(Vpsubb           , VexRvm_Lx          , V(660F00,F8,_,x,I,I,4,FVM), 0                         , 184, 0  , 9269 , 379, 146), // #1405
  INST(Vpsubd           , VexRvm_Lx          , V(660F00,FA,_,x,I,0,4,FV ), 0                         , 133, 0  , 9276 , 380, 131), // #1406
  INST(Vpsubq           , VexRvm_Lx          , V(660F00,FB,_,x,I,1,4,FV ), 0                         , 102, 0  , 9283 , 381, 131), // #1407
  INST(Vpsubsb          , VexRvm_Lx          , V(660F00,E8,_,x,I,I,4,FVM), 0                         , 184, 0  , 9290 , 379, 146), // #1408
  INST(Vpsubsw          , VexRvm_Lx          , V(660F00,E9,_,x,I,I,4,FVM), 0                         , 184, 0  , 9298 , 379, 146), // #1409
  INST(Vpsubusb         , VexRvm_Lx          , V(660F00,D8,_,x,I,I,4,FVM), 0                         , 184, 0  , 9306 , 379, 146), // #1410
  INST(Vpsubusw         , VexRvm_Lx          , V(660F00,D9,_,x,I,I,4,FVM), 0                         , 184, 0  , 9315 , 379, 146), // #1411
  INST(Vpsubw           , VexRvm_Lx          , V(660F00,F9,_,x,I,I,4,FVM), 0                         , 184, 0  , 9324 , 379, 146), // #1412
  INST(Vpternlogd       , VexRvmi_Lx         , E(660F3A,25,_,x,_,0,4,FV ), 0                         , 108, 0  , 9331 , 197, 126), // #1413
  INST(Vpternlogq       , VexRvmi_Lx         , E(660F3A,25,_,x,_,1,4,FV ), 0                         , 109, 0  , 9342 , 198, 126), // #1414
  INST(Vptest           , VexRm_Lx           , V(660F38,17,_,x,I,_,_,_  ), 0                         , 96 , 0  , 9353 , 271, 150), // #1415
  INST(Vptestmb         , VexRvm_Lx          , E(660F38,26,_,x,_,0,4,FVM), 0                         , 110, 0  , 9360 , 371, 128), // #1416
  INST(Vptestmd         , VexRvm_Lx          , E(660F38,27,_,x,_,0,4,FV ), 0                         , 111, 0  , 9369 , 382, 126), // #1417
  INST(Vptestmq         , VexRvm_Lx          , E(660F38,27,_,x,_,1,4,FV ), 0                         , 112, 0  , 9378 , 383, 126), // #1418
  INST(Vptestmw         , VexRvm_Lx          , E(660F38,26,_,x,_,1,4,FVM), 0                         , 113, 0  , 9387 , 371, 128), // #1419
  INST(Vptestnmb        , VexRvm_Lx          , E(F30F38,26,_,x,_,0,4,FVM), 0                         , 217, 0  , 9396 , 371, 128), // #1420
  INST(Vptestnmd        , VexRvm_Lx          , E(F30F38,27,_,x,_,0,4,FV ), 0                         , 218, 0  , 9406 , 382, 126), // #1421
  INST(Vptestnmq        , VexRvm_Lx          , E(F30F38,27,_,x,_,1,4,FV ), 0                         , 219, 0  , 9416 , 383, 126), // #1422
  INST(Vptestnmw        , VexRvm_Lx          , E(F30F38,26,_,x,_,1,4,FVM), 0                         , 220, 0  , 9426 , 371, 128), // #1423
  INST(Vpunpckhbw       , VexRvm_Lx          , V(660F00,68,_,x,I,I,4,FVM), 0                         , 184, 0  , 9436 , 286, 146), // #1424
  INST(Vpunpckhdq       , VexRvm_Lx          , V(660F00,6A,_,x,I,0,4,FV ), 0                         , 133, 0  , 9447 , 200, 131), // #1425
  INST(Vpunpckhqdq      , VexRvm_Lx          , V(660F00,6D,_,x,I,1,4,FV ), 0                         , 102, 0  , 9458 , 199, 131), // #1426
  INST(Vpunpckhwd       , VexRvm_Lx          , V(660F00,69,_,x,I,I,4,FVM), 0                         , 184, 0  , 9470 , 286, 146), // #1427
  INST(Vpunpcklbw       , VexRvm_Lx          , V(660F00,60,_,x,I,I,4,FVM), 0                         , 184, 0  , 9481 , 286, 146), // #1428
  INST(Vpunpckldq       , VexRvm_Lx          , V(660F00,62,_,x,I,0,4,FV ), 0                         , 133, 0  , 9492 , 200, 131), // #1429
  INST(Vpunpcklqdq      , VexRvm_Lx          , V(660F00,6C,_,x,I,1,4,FV ), 0                         , 102, 0  , 9503 , 199, 131), // #1430
  INST(Vpunpcklwd       , VexRvm_Lx          , V(660F00,61,_,x,I,I,4,FVM), 0                         , 184, 0  , 9515 , 286, 146), // #1431
  INST(Vpxor            , VexRvm_Lx          , V(660F00,EF,_,x,I,_,_,_  ), 0                         , 68 , 0  , 9526 , 319, 143), // #1432
  INST(Vpxord           , VexRvm_Lx          , E(660F00,EF,_,x,_,0,4,FV ), 0                         , 189, 0  , 9532 , 320, 126), // #1433
  INST(Vpxorq           , VexRvm_Lx          , E(660F00,EF,_,x,_,1,4,FV ), 0                         , 130, 0  , 9539 , 321, 126), // #1434
  INST(Vrangepd         , VexRvmi_Lx         , E(660F3A,50,_,x,_,1,4,FV ), 0                         , 109, 0  , 9546 , 260, 129), // #1435
  INST(Vrangeps         , VexRvmi_Lx         , E(660F3A,50,_,x,_,0,4,FV ), 0                         , 108, 0  , 9555 , 261, 129), // #1436
  INST(Vrangesd         , VexRvmi            , E(660F3A,51,_,I,_,1,3,T1S), 0                         , 160, 0  , 9564 , 262, 64 ), // #1437
  INST(Vrangess         , VexRvmi            , E(660F3A,51,_,I,_,0,2,T1S), 0                         , 161, 0  , 9573 , 263, 64 ), // #1438
  INST(Vrcp14pd         , VexRm_Lx           , E(660F38,4C,_,x,_,1,4,FV ), 0                         , 112, 0  , 9582 , 353, 126), // #1439
  INST(Vrcp14ps         , VexRm_Lx           , E(660F38,4C,_,x,_,0,4,FV ), 0                         , 111, 0  , 9591 , 339, 126), // #1440
  INST(Vrcp14sd         , VexRvm             , E(660F38,4D,_,I,_,1,3,T1S), 0                         , 124, 0  , 9600 , 384, 66 ), // #1441
  INST(Vrcp14ss         , VexRvm             , E(660F38,4D,_,I,_,0,2,T1S), 0                         , 125, 0  , 9609 , 385, 66 ), // #1442
  INST(Vrcp28pd         , VexRm              , E(660F38,CA,_,2,_,1,4,FV ), 0                         , 152, 0  , 9618 , 253, 135), // #1443
  INST(Vrcp28ps         , VexRm              , E(660F38,CA,_,2,_,0,4,FV ), 0                         , 153, 0  , 9627 , 254, 135), // #1444
  INST(Vrcp28sd         , VexRvm             , E(660F38,CB,_,I,_,1,3,T1S), 0                         , 124, 0  , 9636 , 281, 135), // #1445
  INST(Vrcp28ss         , VexRvm             , E(660F38,CB,_,I,_,0,2,T1S), 0                         , 125, 0  , 9645 , 282, 135), // #1446
  INST(Vrcpps           , VexRm_Lx           , V(000F00,53,_,x,I,_,_,_  ), 0                         , 71 , 0  , 9654 , 271, 123), // #1447
  INST(Vrcpss           , VexRvm             , V(F30F00,53,_,I,I,_,_,_  ), 0                         , 178, 0  , 9661 , 386, 123), // #1448
  INST(Vreducepd        , VexRmi_Lx          , E(660F3A,56,_,x,_,1,4,FV ), 0                         , 109, 0  , 9668 , 365, 129), // #1449
  INST(Vreduceps        , VexRmi_Lx          , E(660F3A,56,_,x,_,0,4,FV ), 0                         , 108, 0  , 9678 , 364, 129), // #1450
  INST(Vreducesd        , VexRvmi            , E(660F3A,57,_,I,_,1,3,T1S), 0                         , 160, 0  , 9688 , 387, 64 ), // #1451
  INST(Vreducess        , VexRvmi            , E(660F3A,57,_,I,_,0,2,T1S), 0                         , 161, 0  , 9698 , 388, 64 ), // #1452
  INST(Vrndscalepd      , VexRmi_Lx          , E(660F3A,09,_,x,_,1,4,FV ), 0                         , 109, 0  , 9708 , 283, 126), // #1453
  INST(Vrndscaleps      , VexRmi_Lx          , E(660F3A,08,_,x,_,0,4,FV ), 0                         , 108, 0  , 9720 , 284, 126), // #1454
  INST(Vrndscalesd      , VexRvmi            , E(660F3A,0B,_,I,_,1,3,T1S), 0                         , 160, 0  , 9732 , 262, 66 ), // #1455
  INST(Vrndscaless      , VexRvmi            , E(660F3A,0A,_,I,_,0,2,T1S), 0                         , 161, 0  , 9744 , 263, 66 ), // #1456
  INST(Vroundpd         , VexRmi_Lx          , V(660F3A,09,_,x,I,_,_,_  ), 0                         , 72 , 0  , 9756 , 389, 123), // #1457
  INST(Vroundps         , VexRmi_Lx          , V(660F3A,08,_,x,I,_,_,_  ), 0                         , 72 , 0  , 9765 , 389, 123), // #1458
  INST(Vroundsd         , VexRvmi            , V(660F3A,0B,_,I,I,_,_,_  ), 0                         , 72 , 0  , 9774 , 390, 123), // #1459
  INST(Vroundss         , VexRvmi            , V(660F3A,0A,_,I,I,_,_,_  ), 0                         , 72 , 0  , 9783 , 391, 123), // #1460
  INST(Vrsqrt14pd       , VexRm_Lx           , E(660F38,4E,_,x,_,1,4,FV ), 0                         , 112, 0  , 9792 , 353, 126), // #1461
  INST(Vrsqrt14ps       , VexRm_Lx           , E(660F38,4E,_,x,_,0,4,FV ), 0                         , 111, 0  , 9803 , 339, 126), // #1462
  INST(Vrsqrt14sd       , VexRvm             , E(660F38,4F,_,I,_,1,3,T1S), 0                         , 124, 0  , 9814 , 384, 66 ), // #1463
  INST(Vrsqrt14ss       , VexRvm             , E(660F38,4F,_,I,_,0,2,T1S), 0                         , 125, 0  , 9825 , 385, 66 ), // #1464
  INST(Vrsqrt28pd       , VexRm              , E(660F38,CC,_,2,_,1,4,FV ), 0                         , 152, 0  , 9836 , 253, 135), // #1465
  INST(Vrsqrt28ps       , VexRm              , E(660F38,CC,_,2,_,0,4,FV ), 0                         , 153, 0  , 9847 , 254, 135), // #1466
  INST(Vrsqrt28sd       , VexRvm             , E(660F38,CD,_,I,_,1,3,T1S), 0                         , 124, 0  , 9858 , 281, 135), // #1467
  INST(Vrsqrt28ss       , VexRvm             , E(660F38,CD,_,I,_,0,2,T1S), 0                         , 125, 0  , 9869 , 282, 135), // #1468
  INST(Vrsqrtps         , VexRm_Lx           , V(000F00,52,_,x,I,_,_,_  ), 0                         , 71 , 0  , 9880 , 271, 123), // #1469
  INST(Vrsqrtss         , VexRvm             , V(F30F00,52,_,I,I,_,_,_  ), 0                         , 178, 0  , 9889 , 386, 123), // #1470
  INST(Vscalefpd        , VexRvm_Lx          , E(660F38,2C,_,x,_,1,4,FV ), 0                         , 112, 0  , 9898 , 392, 126), // #1471
  INST(Vscalefps        , VexRvm_Lx          , E(660F38,2C,_,x,_,0,4,FV ), 0                         , 111, 0  , 9908 , 393, 126), // #1472
  INST(Vscalefsd        , VexRvm             , E(660F38,2D,_,I,_,1,3,T1S), 0                         , 124, 0  , 9918 , 394, 66 ), // #1473
  INST(Vscalefss        , VexRvm             , E(660F38,2D,_,I,_,0,2,T1S), 0                         , 125, 0  , 9928 , 395, 66 ), // #1474
  INST(Vscatterdpd      , VexMr_Lx           , E(660F38,A2,_,x,_,1,3,T1S), 0                         , 124, 0  , 9938 , 396, 126), // #1475
  INST(Vscatterdps      , VexMr_Lx           , E(660F38,A2,_,x,_,0,2,T1S), 0                         , 125, 0  , 9950 , 367, 126), // #1476
  INST(Vscatterpf0dpd   , VexM_VM            , E(660F38,C6,5,2,_,1,3,T1S), 0                         , 221, 0  , 9962 , 276, 141), // #1477
  INST(Vscatterpf0dps   , VexM_VM            , E(660F38,C6,5,2,_,0,2,T1S), 0                         , 222, 0  , 9977 , 277, 141), // #1478
  INST(Vscatterpf0qpd   , VexM_VM            , E(660F38,C7,5,2,_,1,3,T1S), 0                         , 221, 0  , 9992 , 278, 141), // #1479
  INST(Vscatterpf0qps   , VexM_VM            , E(660F38,C7,5,2,_,0,2,T1S), 0                         , 222, 0  , 10007, 278, 141), // #1480
  INST(Vscatterpf1dpd   , VexM_VM            , E(660F38,C6,6,2,_,1,3,T1S), 0                         , 223, 0  , 10022, 276, 141), // #1481
  INST(Vscatterpf1dps   , VexM_VM            , E(660F38,C6,6,2,_,0,2,T1S), 0                         , 224, 0  , 10037, 277, 141), // #1482
  INST(Vscatterpf1qpd   , VexM_VM            , E(660F38,C7,6,2,_,1,3,T1S), 0                         , 223, 0  , 10052, 278, 141), // #1483
  INST(Vscatterpf1qps   , VexM_VM            , E(660F38,C7,6,2,_,0,2,T1S), 0                         , 224, 0  , 10067, 278, 141), // #1484
  INST(Vscatterqpd      , VexMr_Lx           , E(660F38,A3,_,x,_,1,3,T1S), 0                         , 124, 0  , 10082, 369, 126), // #1485
  INST(Vscatterqps      , VexMr_Lx           , E(660F38,A3,_,x,_,0,2,T1S), 0                         , 125, 0  , 10094, 368, 126), // #1486
  INST(Vshuff32x4       , VexRvmi_Lx         , E(660F3A,23,_,x,_,0,4,FV ), 0                         , 108, 0  , 10106, 397, 126), // #1487
  INST(Vshuff64x2       , VexRvmi_Lx         , E(660F3A,23,_,x,_,1,4,FV ), 0                         , 109, 0  , 10117, 398, 126), // #1488
  INST(Vshufi32x4       , VexRvmi_Lx         , E(660F3A,43,_,x,_,0,4,FV ), 0                         , 108, 0  , 10128, 397, 126), // #1489
  INST(Vshufi64x2       , VexRvmi_Lx         , E(660F3A,43,_,x,_,1,4,FV ), 0                         , 109, 0  , 10139, 398, 126), // #1490
  INST(Vshufpd          , VexRvmi_Lx         , V(660F00,C6,_,x,I,1,4,FV ), 0                         , 102, 0  , 10150, 399, 121), // #1491
  INST(Vshufps          , VexRvmi_Lx         , V(000F00,C6,_,x,I,0,4,FV ), 0                         , 103, 0  , 10158, 400, 121), // #1492
  INST(Vsqrtpd          , VexRm_Lx           , V(660F00,51,_,x,I,1,4,FV ), 0                         , 102, 0  , 10166, 401, 121), // #1493
  INST(Vsqrtps          , VexRm_Lx           , V(000F00,51,_,x,I,0,4,FV ), 0                         , 103, 0  , 10174, 223, 121), // #1494
  INST(Vsqrtsd          , VexRvm             , V(F20F00,51,_,I,I,1,3,T1S), 0                         , 104, 0  , 10182, 191, 122), // #1495
  INST(Vsqrtss          , VexRvm             , V(F30F00,51,_,I,I,0,2,T1S), 0                         , 105, 0  , 10190, 192, 122), // #1496
  INST(Vstmxcsr         , VexM               , V(000F00,AE,3,0,I,_,_,_  ), 0                         , 225, 0  , 10198, 292, 123), // #1497
  INST(Vsubpd           , VexRvm_Lx          , V(660F00,5C,_,x,I,1,4,FV ), 0                         , 102, 0  , 10207, 189, 121), // #1498
  INST(Vsubps           , VexRvm_Lx          , V(000F00,5C,_,x,I,0,4,FV ), 0                         , 103, 0  , 10214, 190, 121), // #1499
  INST(Vsubsd           , VexRvm             , V(F20F00,5C,_,I,I,1,3,T1S), 0                         , 104, 0  , 10221, 191, 122), // #1500
  INST(Vsubss           , VexRvm             , V(F30F00,5C,_,I,I,0,2,T1S), 0                         , 105, 0  , 10228, 192, 122), // #1501
  INST(Vtestpd          , VexRm_Lx           , V(660F38,0F,_,x,0,_,_,_  ), 0                         , 96 , 0  , 10235, 271, 150), // #1502
  INST(Vtestps          , VexRm_Lx           , V(660F38,0E,_,x,0,_,_,_  ), 0                         , 96 , 0  , 10243, 271, 150), // #1503
  INST(Vucomisd         , VexRm              , V(660F00,2E,_,I,I,1,3,T1S), 0                         , 122, 0  , 10251, 219, 132), // #1504
  INST(Vucomiss         , VexRm              , V(000F00,2E,_,I,I,0,2,T1S), 0                         , 123, 0  , 10260, 220, 132), // #1505
  INST(Vunpckhpd        , VexRvm_Lx          , V(660F00,15,_,x,I,1,4,FV ), 0                         , 102, 0  , 10269, 199, 121), // #1506
  INST(Vunpckhps        , VexRvm_Lx          , V(000F00,15,_,x,I,0,4,FV ), 0                         , 103, 0  , 10279, 200, 121), // #1507
  INST(Vunpcklpd        , VexRvm_Lx          , V(660F00,14,_,x,I,1,4,FV ), 0                         , 102, 0  , 10289, 199, 121), // #1508
  INST(Vunpcklps        , VexRvm_Lx          , V(000F00,14,_,x,I,0,4,FV ), 0                         , 103, 0  , 10299, 200, 121), // #1509
  INST(Vxorpd           , VexRvm_Lx          , V(660F00,57,_,x,I,1,4,FV ), 0                         , 102, 0  , 10309, 381, 127), // #1510
  INST(Vxorps           , VexRvm_Lx          , V(000F00,57,_,x,I,0,4,FV ), 0                         , 103, 0  , 10316, 380, 127), // #1511
  INST(Vzeroall         , VexOp              , V(000F00,77,_,1,I,_,_,_  ), 0                         , 67 , 0  , 10323, 402, 123), // #1512
  INST(Vzeroupper       , VexOp              , V(000F00,77,_,0,I,_,_,_  ), 0                         , 71 , 0  , 10332, 402, 123), // #1513
  INST(Wbinvd           , X86Op              , O(000F00,09,_,_,_,_,_,_  ), 0                         , 4  , 0  , 10343, 30 , 0  ), // #1514
  INST(Wbnoinvd         , X86Op              , O(F30F00,09,_,_,_,_,_,_  ), 0                         , 6  , 0  , 10350, 30 , 159), // #1515
  INST(Wrfsbase         , X86M               , O(F30F00,AE,2,_,x,_,_,_  ), 0                         , 226, 0  , 10359, 167, 102), // #1516
  INST(Wrgsbase         , X86M               , O(F30F00,AE,3,_,x,_,_,_  ), 0                         , 227, 0  , 10368, 167, 102), // #1517
  INST(Wrmsr            , X86Op              , O(000F00,30,_,_,_,_,_,_  ), 0                         , 4  , 0  , 10377, 168, 103), // #1518
  INST(Wrssd            , X86Mr              , O(000F38,F6,_,_,_,_,_,_  ), 0                         , 82 , 0  , 10383, 403, 54 ), // #1519
  INST(Wrssq            , X86Mr              , O(000F38,F6,_,_,1,_,_,_  ), 0                         , 228, 0  , 10389, 404, 54 ), // #1520
  INST(Wrussd           , X86Mr              , O(660F38,F5,_,_,_,_,_,_  ), 0                         , 2  , 0  , 10395, 403, 54 ), // #1521
  INST(Wrussq           , X86Mr              , O(660F38,F5,_,_,1,_,_,_  ), 0                         , 229, 0  , 10402, 404, 54 ), // #1522
  INST(Xabort           , X86Op_Mod11RM_I8   , O(000000,C6,7,_,_,_,_,_  ), 0                         , 26 , 0  , 10409, 77 , 160), // #1523
  INST(Xadd             , X86Xadd            , O(000F00,C0,_,_,x,_,_,_  ), 0                         , 4  , 0  , 10416, 405, 37 ), // #1524
  INST(Xbegin           , X86JmpRel          , O(000000,C7,7,_,_,_,_,_  ), 0                         , 26 , 0  , 10421, 406, 160), // #1525
  INST(Xchg             , X86Xchg            , O(000000,86,_,_,x,_,_,_  ), 0                         , 0  , 0  , 457  , 407, 0  ), // #1526
  INST(Xend             , X86Op              , O(000F01,D5,_,_,_,_,_,_  ), 0                         , 21 , 0  , 10428, 30 , 160), // #1527
  INST(Xgetbv           , X86Op              , O(000F01,D0,_,_,_,_,_,_  ), 0                         , 21 , 0  , 10433, 168, 161), // #1528
  INST(Xlatb            , X86Op              , O(000000,D7,_,_,_,_,_,_  ), 0                         , 0  , 0  , 10440, 30 , 0  ), // #1529
  INST(Xor              , X86Arith           , O(000000,30,6,_,x,_,_,_  ), 0                         , 31 , 0  , 9528 , 172, 1  ), // #1530
  INST(Xorpd            , ExtRm              , O(660F00,57,_,_,_,_,_,_  ), 0                         , 3  , 0  , 10310, 145, 4  ), // #1531
  INST(Xorps            , ExtRm              , O(000F00,57,_,_,_,_,_,_  ), 0                         , 4  , 0  , 10317, 145, 5  ), // #1532
  INST(Xresldtrk        , X86Op              , O(F20F01,E9,_,_,_,_,_,_  ), 0                         , 92 , 0  , 10446, 30 , 162), // #1533
  INST(Xrstor           , X86M_Only          , O(000F00,AE,5,_,_,_,_,_  ), 0                         , 75 , 0  , 1159 , 408, 161), // #1534
  INST(Xrstor64         , X86M_Only          , O(000F00,AE,5,_,1,_,_,_  ), 0                         , 230, 0  , 1167 , 409, 161), // #1535
  INST(Xrstors          , X86M_Only          , O(000F00,C7,3,_,_,_,_,_  ), 0                         , 76 , 0  , 10456, 408, 163), // #1536
  INST(Xrstors64        , X86M_Only          , O(000F00,C7,3,_,1,_,_,_  ), 0                         , 231, 0  , 10464, 409, 163), // #1537
  INST(Xsave            , X86M_Only          , O(000F00,AE,4,_,_,_,_,_  ), 0                         , 97 , 0  , 1177 , 408, 161), // #1538
  INST(Xsave64          , X86M_Only          , O(000F00,AE,4,_,1,_,_,_  ), 0                         , 232, 0  , 1184 , 409, 161), // #1539
  INST(Xsavec           , X86M_Only          , O(000F00,C7,4,_,_,_,_,_  ), 0                         , 97 , 0  , 10474, 408, 164), // #1540
  INST(Xsavec64         , X86M_Only          , O(000F00,C7,4,_,1,_,_,_  ), 0                         , 232, 0  , 10481, 409, 164), // #1541
  INST(Xsaveopt         , X86M_Only          , O(000F00,AE,6,_,_,_,_,_  ), 0                         , 78 , 0  , 10490, 408, 165), // #1542
  INST(Xsaveopt64       , X86M_Only          , O(000F00,AE,6,_,1,_,_,_  ), 0                         , 233, 0  , 10499, 409, 165), // #1543
  INST(Xsaves           , X86M_Only          , O(000F00,C7,5,_,_,_,_,_  ), 0                         , 75 , 0  , 10510, 408, 163), // #1544
  INST(Xsaves64         , X86M_Only          , O(000F00,C7,5,_,1,_,_,_  ), 0                         , 230, 0  , 10517, 409, 163), // #1545
  INST(Xsetbv           , X86Op              , O(000F01,D1,_,_,_,_,_,_  ), 0                         , 21 , 0  , 10526, 168, 161), // #1546
  INST(Xsusldtrk        , X86Op              , O(F20F01,E8,_,_,_,_,_,_  ), 0                         , 92 , 0  , 10533, 30 , 162), // #1547
  INST(Xtest            , X86Op              , O(000F01,D6,_,_,_,_,_,_  ), 0                         , 21 , 0  , 10543, 30 , 166)  // #1548
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
  O(000000,00,0,0,0,0,0,_  ), // #0 [ref=56x]
  O(000000,00,2,0,0,0,0,_  ), // #1 [ref=4x]
  O(660F38,00,0,0,0,0,0,_  ), // #2 [ref=43x]
  O(660F00,00,0,0,0,0,0,_  ), // #3 [ref=38x]
  O(000F00,00,0,0,0,0,0,_  ), // #4 [ref=233x]
  O(F20F00,00,0,0,0,0,0,_  ), // #5 [ref=24x]
  O(F30F00,00,0,0,0,0,0,_  ), // #6 [ref=29x]
  O(F30F38,00,0,0,0,0,0,_  ), // #7 [ref=2x]
  O(660F3A,00,0,0,0,0,0,_  ), // #8 [ref=22x]
  O(000000,00,4,0,0,0,0,_  ), // #9 [ref=5x]
  V(000F38,00,0,0,0,0,0,_  ), // #10 [ref=6x]
  V(XOP_M9,00,1,0,0,0,0,_  ), // #11 [ref=3x]
  V(XOP_M9,00,6,0,0,0,0,_  ), // #12 [ref=2x]
  V(XOP_M9,00,5,0,0,0,0,_  ), // #13 [ref=1x]
  V(XOP_M9,00,3,0,0,0,0,_  ), // #14 [ref=1x]
  V(XOP_M9,00,2,0,0,0,0,_  ), // #15 [ref=1x]
  V(000F38,00,3,0,0,0,0,_  ), // #16 [ref=1x]
  V(000F38,00,2,0,0,0,0,_  ), // #17 [ref=1x]
  V(000F38,00,1,0,0,0,0,_  ), // #18 [ref=1x]
  O(660000,00,0,0,0,0,0,_  ), // #19 [ref=7x]
  O(000000,00,0,0,1,0,0,_  ), // #20 [ref=3x]
  O(000F01,00,0,0,0,0,0,_  ), // #21 [ref=29x]
  O(000F00,00,7,0,0,0,0,_  ), // #22 [ref=5x]
  O(660F00,00,7,0,0,0,0,_  ), // #23 [ref=1x]
  O(F30F00,00,6,0,0,0,0,_  ), // #24 [ref=3x]
  O(660F00,00,6,0,0,0,0,_  ), // #25 [ref=3x]
  O(000000,00,7,0,0,0,0,_  ), // #26 [ref=5x]
  O(000F00,00,1,0,1,0,0,_  ), // #27 [ref=2x]
  O(000F00,00,1,0,0,0,0,_  ), // #28 [ref=6x]
  O(F20F38,00,0,0,0,0,0,_  ), // #29 [ref=2x]
  O(000000,00,1,0,0,0,0,_  ), // #30 [ref=3x]
  O(000000,00,6,0,0,0,0,_  ), // #31 [ref=3x]
  O(F30F00,00,7,0,0,0,0,3  ), // #32 [ref=1x]
  O(F30F00,00,7,0,0,0,0,2  ), // #33 [ref=1x]
  O_FPU(00,D900,_)          , // #34 [ref=29x]
  O_FPU(00,C000,0)          , // #35 [ref=1x]
  O_FPU(00,DE00,_)          , // #36 [ref=7x]
  O_FPU(00,0000,4)          , // #37 [ref=4x]
  O_FPU(00,0000,6)          , // #38 [ref=4x]
  O_FPU(9B,DB00,_)          , // #39 [ref=2x]
  O_FPU(00,DA00,_)          , // #40 [ref=5x]
  O_FPU(00,DB00,_)          , // #41 [ref=8x]
  O_FPU(00,D000,2)          , // #42 [ref=1x]
  O_FPU(00,DF00,_)          , // #43 [ref=2x]
  O_FPU(00,D800,3)          , // #44 [ref=1x]
  O_FPU(00,F000,6)          , // #45 [ref=1x]
  O_FPU(00,F800,7)          , // #46 [ref=1x]
  O_FPU(00,DD00,_)          , // #47 [ref=3x]
  O_FPU(00,0000,0)          , // #48 [ref=3x]
  O_FPU(00,0000,2)          , // #49 [ref=3x]
  O_FPU(00,0000,3)          , // #50 [ref=3x]
  O_FPU(00,0000,7)          , // #51 [ref=3x]
  O_FPU(00,0000,1)          , // #52 [ref=2x]
  O_FPU(00,0000,5)          , // #53 [ref=2x]
  O_FPU(00,C800,1)          , // #54 [ref=1x]
  O_FPU(9B,0000,6)          , // #55 [ref=2x]
  O_FPU(9B,0000,7)          , // #56 [ref=2x]
  O_FPU(00,E000,4)          , // #57 [ref=1x]
  O_FPU(00,E800,5)          , // #58 [ref=1x]
  O_FPU(00,0000,_)          , // #59 [ref=1x]
  O(000F00,00,0,0,1,0,0,_  ), // #60 [ref=1x]
  O(000000,00,5,0,0,0,0,_  ), // #61 [ref=3x]
  O(F30F00,00,5,0,0,0,0,_  ), // #62 [ref=2x]
  O(F30F00,00,5,0,1,0,0,_  ), // #63 [ref=1x]
  V(660F00,00,0,1,0,0,0,_  ), // #64 [ref=7x]
  V(660F00,00,0,1,1,0,0,_  ), // #65 [ref=6x]
  V(000F00,00,0,1,1,0,0,_  ), // #66 [ref=7x]
  V(000F00,00,0,1,0,0,0,_  ), // #67 [ref=8x]
  V(660F00,00,0,0,0,0,0,_  ), // #68 [ref=15x]
  V(660F00,00,0,0,1,0,0,_  ), // #69 [ref=4x]
  V(000F00,00,0,0,1,0,0,_  ), // #70 [ref=4x]
  V(000F00,00,0,0,0,0,0,_  ), // #71 [ref=10x]
  V(660F3A,00,0,0,0,0,0,_  ), // #72 [ref=45x]
  V(660F3A,00,0,0,1,0,0,_  ), // #73 [ref=4x]
  O(000F00,00,2,0,0,0,0,_  ), // #74 [ref=5x]
  O(000F00,00,5,0,0,0,0,_  ), // #75 [ref=4x]
  O(000F00,00,3,0,0,0,0,_  ), // #76 [ref=5x]
  V(XOP_M9,00,0,0,0,0,0,_  ), // #77 [ref=32x]
  O(000F00,00,6,0,0,0,0,_  ), // #78 [ref=5x]
  V(XOP_MA,00,0,0,0,0,0,_  ), // #79 [ref=1x]
  V(XOP_MA,00,1,0,0,0,0,_  ), // #80 [ref=1x]
  O(F30F01,00,0,0,0,0,0,_  ), // #81 [ref=5x]
  O(000F38,00,0,0,0,0,0,_  ), // #82 [ref=24x]
  V(F20F38,00,0,0,0,0,0,_  ), // #83 [ref=6x]
  O(000000,00,3,0,0,0,0,_  ), // #84 [ref=3x]
  O(000F3A,00,0,0,0,0,0,_  ), // #85 [ref=4x]
  O(F30000,00,0,0,0,0,0,_  ), // #86 [ref=1x]
  O(000F0F,00,0,0,0,0,0,_  ), // #87 [ref=26x]
  V(F30F38,00,0,0,0,0,0,_  ), // #88 [ref=5x]
  O(000F3A,00,0,0,1,0,0,_  ), // #89 [ref=1x]
  O(660F3A,00,0,0,1,0,0,_  ), // #90 [ref=1x]
  O(F30F00,00,4,0,0,0,0,_  ), // #91 [ref=1x]
  O(F20F01,00,0,0,0,0,0,_  ), // #92 [ref=4x]
  O(F30F00,00,1,0,0,0,0,_  ), // #93 [ref=3x]
  O(F30F00,00,7,0,0,0,0,_  ), // #94 [ref=1x]
  V(F20F3A,00,0,0,0,0,0,_  ), // #95 [ref=1x]
  V(660F38,00,0,0,0,0,0,_  ), // #96 [ref=25x]
  O(000F00,00,4,0,0,0,0,_  ), // #97 [ref=4x]
  V(XOP_M9,00,7,0,0,0,0,_  ), // #98 [ref=1x]
  V(XOP_M9,00,4,0,0,0,0,_  ), // #99 [ref=1x]
  O(F20F00,00,6,0,0,0,0,_  ), // #100 [ref=1x]
  E(F20F38,00,0,2,0,0,2,T4X), // #101 [ref=6x]
  V(660F00,00,0,0,0,1,4,FV ), // #102 [ref=22x]
  V(000F00,00,0,0,0,0,4,FV ), // #103 [ref=16x]
  V(F20F00,00,0,0,0,1,3,T1S), // #104 [ref=10x]
  V(F30F00,00,0,0,0,0,2,T1S), // #105 [ref=10x]
  V(F20F00,00,0,0,0,0,0,_  ), // #106 [ref=4x]
  V(660F38,00,0,0,0,0,4,FVM), // #107 [ref=14x]
  E(660F3A,00,0,0,0,0,4,FV ), // #108 [ref=14x]
  E(660F3A,00,0,0,0,1,4,FV ), // #109 [ref=14x]
  E(660F38,00,0,0,0,0,4,FVM), // #110 [ref=9x]
  E(660F38,00,0,0,0,0,4,FV ), // #111 [ref=18x]
  E(660F38,00,0,0,0,1,4,FV ), // #112 [ref=28x]
  E(660F38,00,0,0,0,1,4,FVM), // #113 [ref=9x]
  V(660F38,00,0,1,0,0,0,_  ), // #114 [ref=2x]
  E(660F38,00,0,0,0,0,3,T2 ), // #115 [ref=2x]
  E(660F38,00,0,0,0,0,4,T4 ), // #116 [ref=2x]
  E(660F38,00,0,2,0,0,5,T8 ), // #117 [ref=2x]
  E(660F38,00,0,0,0,1,4,T2 ), // #118 [ref=2x]
  E(660F38,00,0,2,0,1,5,T4 ), // #119 [ref=2x]
  V(660F38,00,0,0,0,1,3,T1S), // #120 [ref=2x]
  V(660F38,00,0,0,0,0,2,T1S), // #121 [ref=14x]
  V(660F00,00,0,0,0,1,3,T1S), // #122 [ref=5x]
  V(000F00,00,0,0,0,0,2,T1S), // #123 [ref=2x]
  E(660F38,00,0,0,0,1,3,T1S), // #124 [ref=14x]
  E(660F38,00,0,0,0,0,2,T1S), // #125 [ref=14x]
  V(F30F00,00,0,0,0,0,3,HV ), // #126 [ref=1x]
  E(F20F38,00,0,0,0,0,0,_  ), // #127 [ref=1x]
  E(F30F38,00,0,0,0,0,0,_  ), // #128 [ref=7x]
  V(F20F00,00,0,0,0,1,4,FV ), // #129 [ref=1x]
  E(660F00,00,0,0,0,1,4,FV ), // #130 [ref=9x]
  E(000F00,00,0,0,0,1,4,FV ), // #131 [ref=3x]
  V(660F38,00,0,0,0,0,3,HVM), // #132 [ref=7x]
  V(660F00,00,0,0,0,0,4,FV ), // #133 [ref=11x]
  V(000F00,00,0,0,0,0,4,HV ), // #134 [ref=1x]
  V(660F3A,00,0,0,0,0,3,HVM), // #135 [ref=1x]
  E(660F00,00,0,0,0,0,3,HV ), // #136 [ref=4x]
  E(000F00,00,0,0,0,0,4,FV ), // #137 [ref=2x]
  E(F30F00,00,0,0,0,1,4,FV ), // #138 [ref=2x]
  V(F20F00,00,0,0,0,0,3,T1F), // #139 [ref=2x]
  E(F20F00,00,0,0,0,0,3,T1F), // #140 [ref=2x]
  V(F20F00,00,0,0,0,0,2,T1W), // #141 [ref=1x]
  V(F30F00,00,0,0,0,0,2,T1W), // #142 [ref=1x]
  V(F30F00,00,0,0,0,0,2,T1F), // #143 [ref=2x]
  E(F30F00,00,0,0,0,0,2,T1F), // #144 [ref=2x]
  V(F30F00,00,0,0,0,0,4,FV ), // #145 [ref=1x]
  E(F30F00,00,0,0,0,0,3,HV ), // #146 [ref=1x]
  E(F20F00,00,0,0,0,0,4,FV ), // #147 [ref=1x]
  E(F20F00,00,0,0,0,1,4,FV ), // #148 [ref=1x]
  E(F20F00,00,0,0,0,0,2,T1W), // #149 [ref=1x]
  E(F30F00,00,0,0,0,0,2,T1W), // #150 [ref=1x]
  E(660F3A,00,0,0,0,0,4,FVM), // #151 [ref=5x]
  E(660F38,00,0,2,0,1,4,FV ), // #152 [ref=3x]
  E(660F38,00,0,2,0,0,4,FV ), // #153 [ref=3x]
  V(660F3A,00,0,1,0,0,0,_  ), // #154 [ref=6x]
  E(660F3A,00,0,0,0,0,4,T4 ), // #155 [ref=4x]
  E(660F3A,00,0,2,0,0,5,T8 ), // #156 [ref=4x]
  E(660F3A,00,0,0,0,1,4,T2 ), // #157 [ref=4x]
  E(660F3A,00,0,2,0,1,5,T4 ), // #158 [ref=4x]
  V(660F3A,00,0,0,0,0,2,T1S), // #159 [ref=4x]
  E(660F3A,00,0,0,0,1,3,T1S), // #160 [ref=6x]
  E(660F3A,00,0,0,0,0,2,T1S), // #161 [ref=6x]
  V(660F38,00,0,0,1,1,4,FV ), // #162 [ref=20x]
  V(660F38,00,0,0,0,0,4,FV ), // #163 [ref=36x]
  V(660F38,00,0,0,1,1,3,T1S), // #164 [ref=12x]
  V(660F38,00,0,0,1,0,0,_  ), // #165 [ref=5x]
  E(660F38,00,1,2,0,1,3,T1S), // #166 [ref=2x]
  E(660F38,00,1,2,0,0,2,T1S), // #167 [ref=2x]
  E(660F38,00,2,2,0,1,3,T1S), // #168 [ref=2x]
  E(660F38,00,2,2,0,0,2,T1S), // #169 [ref=2x]
  V(660F3A,00,0,0,1,1,4,FV ), // #170 [ref=2x]
  V(000F00,00,2,0,0,0,0,_  ), // #171 [ref=1x]
  V(660F00,00,0,0,0,1,4,FVM), // #172 [ref=3x]
  V(000F00,00,0,0,0,0,4,FVM), // #173 [ref=3x]
  V(660F00,00,0,0,0,0,2,T1S), // #174 [ref=1x]
  V(F20F00,00,0,0,0,1,3,DUP), // #175 [ref=1x]
  E(660F00,00,0,0,0,0,4,FVM), // #176 [ref=1x]
  E(660F00,00,0,0,0,1,4,FVM), // #177 [ref=1x]
  V(F30F00,00,0,0,0,0,0,_  ), // #178 [ref=3x]
  E(F20F00,00,0,0,0,1,4,FVM), // #179 [ref=1x]
  E(F30F00,00,0,0,0,0,4,FVM), // #180 [ref=1x]
  E(F30F00,00,0,0,0,1,4,FVM), // #181 [ref=1x]
  E(F20F00,00,0,0,0,0,4,FVM), // #182 [ref=1x]
  V(000F00,00,0,0,0,0,3,T2 ), // #183 [ref=2x]
  V(660F00,00,0,0,0,0,4,FVM), // #184 [ref=33x]
  V(F30F00,00,0,0,0,0,4,FVM), // #185 [ref=3x]
  E(F20F38,00,0,0,0,0,4,FV ), // #186 [ref=1x]
  E(F20F38,00,0,0,0,1,4,FV ), // #187 [ref=1x]
  V(660F3A,00,0,0,0,0,4,FVM), // #188 [ref=2x]
  E(660F00,00,0,0,0,0,4,FV ), // #189 [ref=5x]
  V(660F38,00,0,0,0,0,0,T1S), // #190 [ref=1x]
  E(F30F38,00,0,0,0,1,0,_  ), // #191 [ref=5x]
  V(660F38,00,0,0,0,0,1,T1S), // #192 [ref=1x]
  V(XOP_M8,00,0,0,0,0,0,_  ), // #193 [ref=22x]
  V(660F38,00,0,0,0,1,4,FVM), // #194 [ref=2x]
  E(660F3A,00,0,0,0,1,4,FVM), // #195 [ref=2x]
  E(660F38,00,0,0,0,0,0,T1S), // #196 [ref=2x]
  E(660F38,00,0,0,0,1,1,T1S), // #197 [ref=2x]
  V(660F38,00,0,0,0,1,4,FV ), // #198 [ref=3x]
  E(660F38,00,0,0,1,1,4,FV ), // #199 [ref=1x]
  V(660F3A,00,0,0,0,0,0,T1S), // #200 [ref=2x]
  V(660F3A,00,0,0,1,1,3,T1S), // #201 [ref=2x]
  V(660F3A,00,0,0,0,0,1,T1S), // #202 [ref=1x]
  V(660F00,00,0,0,0,0,1,T1S), // #203 [ref=1x]
  E(F30F38,00,0,0,0,0,2,QVM), // #204 [ref=6x]
  E(F30F38,00,0,0,0,0,3,HVM), // #205 [ref=9x]
  E(F30F38,00,0,0,0,0,1,OVM), // #206 [ref=3x]
  V(660F38,00,0,0,0,0,2,QVM), // #207 [ref=4x]
  V(660F38,00,0,0,0,0,1,OVM), // #208 [ref=2x]
  E(660F00,00,1,0,0,0,4,FV ), // #209 [ref=1x]
  E(660F00,00,1,0,0,1,4,FV ), // #210 [ref=1x]
  V(F20F00,00,0,0,0,0,4,FVM), // #211 [ref=1x]
  V(660F00,00,0,0,0,0,4,128), // #212 [ref=5x]
  V(660F00,00,7,0,0,0,4,FVM), // #213 [ref=1x]
  V(660F00,00,0,0,0,1,4,128), // #214 [ref=2x]
  E(660F00,00,0,0,0,1,4,128), // #215 [ref=1x]
  V(660F00,00,3,0,0,0,4,FVM), // #216 [ref=1x]
  E(F30F38,00,0,0,0,0,4,FVM), // #217 [ref=1x]
  E(F30F38,00,0,0,0,0,4,FV ), // #218 [ref=1x]
  E(F30F38,00,0,0,0,1,4,FV ), // #219 [ref=1x]
  E(F30F38,00,0,0,0,1,4,FVM), // #220 [ref=1x]
  E(660F38,00,5,2,0,1,3,T1S), // #221 [ref=2x]
  E(660F38,00,5,2,0,0,2,T1S), // #222 [ref=2x]
  E(660F38,00,6,2,0,1,3,T1S), // #223 [ref=2x]
  E(660F38,00,6,2,0,0,2,T1S), // #224 [ref=2x]
  V(000F00,00,3,0,0,0,0,_  ), // #225 [ref=1x]
  O(F30F00,00,2,0,0,0,0,_  ), // #226 [ref=1x]
  O(F30F00,00,3,0,0,0,0,_  ), // #227 [ref=1x]
  O(000F38,00,0,0,1,0,0,_  ), // #228 [ref=1x]
  O(660F38,00,0,0,1,0,0,_  ), // #229 [ref=1x]
  O(000F00,00,5,0,1,0,0,_  ), // #230 [ref=2x]
  O(000F00,00,3,0,1,0,0,_  ), // #231 [ref=1x]
  O(000F00,00,4,0,1,0,0,_  ), // #232 [ref=2x]
  O(000F00,00,6,0,1,0,0,_  )  // #233 [ref=1x]
};
// ----------------------------------------------------------------------------
// ${MainOpcodeTable:End}

// ${AltOpcodeTable:Begin}
// ------------------- Automatically generated, do not edit -------------------
const uint32_t InstDB::_altOpcodeTable[] = {
  0                         , // #0 [ref=1404x]
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
  E(660F00,7F,_,x,_,0,4,FVM), // #89 [ref=1x]
  E(660F00,7F,_,x,_,1,4,FVM), // #90 [ref=1x]
  V(F30F00,7F,_,x,I,_,_,_  ), // #91 [ref=1x]
  E(F20F00,7F,_,x,_,1,4,FVM), // #92 [ref=1x]
  E(F30F00,7F,_,x,_,0,4,FVM), // #93 [ref=1x]
  E(F30F00,7F,_,x,_,1,4,FVM), // #94 [ref=1x]
  E(F20F00,7F,_,x,_,0,4,FVM), // #95 [ref=1x]
  V(660F00,17,_,0,I,1,3,T1S), // #96 [ref=1x]
  V(000F00,17,_,0,I,0,3,T2 ), // #97 [ref=1x]
  V(660F00,13,_,0,I,1,3,T1S), // #98 [ref=1x]
  V(000F00,13,_,0,I,0,3,T2 ), // #99 [ref=1x]
  V(660F00,7E,_,0,I,1,3,T1S), // #100 [ref=1x]
  V(F20F00,11,_,I,I,1,3,T1S), // #101 [ref=1x]
  V(F30F00,11,_,I,I,0,2,T1S), // #102 [ref=1x]
  V(660F00,11,_,x,I,1,4,FVM), // #103 [ref=1x]
  V(000F00,11,_,x,I,0,4,FVM), // #104 [ref=1x]
  E(660F38,7A,_,x,0,0,0,T1S), // #105 [ref=1x]
  E(660F38,7C,_,x,0,0,0,T1S), // #106 [ref=1x]
  E(660F38,7C,_,x,0,1,0,T1S), // #107 [ref=1x]
  E(660F38,7B,_,x,0,0,0,T1S), // #108 [ref=1x]
  V(660F3A,05,_,x,0,1,4,FV ), // #109 [ref=1x]
  V(660F3A,04,_,x,0,0,4,FV ), // #110 [ref=1x]
  V(660F3A,01,_,x,1,1,4,FV ), // #111 [ref=1x]
  V(660F3A,00,_,x,1,1,4,FV ), // #112 [ref=1x]
  V(660F38,90,_,x,_,0,2,T1S), // #113 [ref=1x]
  V(660F38,90,_,x,_,1,3,T1S), // #114 [ref=1x]
  V(660F38,91,_,x,_,0,2,T1S), // #115 [ref=1x]
  V(660F38,91,_,x,_,1,3,T1S), // #116 [ref=1x]
  V(660F38,8E,_,x,0,_,_,_  ), // #117 [ref=1x]
  V(660F38,8E,_,x,1,_,_,_  ), // #118 [ref=1x]
  V(XOP_M8,C0,_,0,x,_,_,_  ), // #119 [ref=1x]
  V(XOP_M8,C2,_,0,x,_,_,_  ), // #120 [ref=1x]
  V(XOP_M8,C3,_,0,x,_,_,_  ), // #121 [ref=1x]
  V(XOP_M8,C1,_,0,x,_,_,_  ), // #122 [ref=1x]
  V(660F00,72,6,x,I,0,4,FV ), // #123 [ref=1x]
  V(660F00,73,6,x,I,1,4,FV ), // #124 [ref=1x]
  V(660F00,71,6,x,I,I,4,FVM), // #125 [ref=1x]
  V(660F00,72,4,x,I,0,4,FV ), // #126 [ref=1x]
  E(660F00,72,4,x,_,1,4,FV ), // #127 [ref=1x]
  V(660F00,71,4,x,I,I,4,FVM), // #128 [ref=1x]
  V(660F00,72,2,x,I,0,4,FV ), // #129 [ref=1x]
  V(660F00,73,2,x,I,1,4,FV ), // #130 [ref=1x]
  V(660F00,71,2,x,I,I,4,FVM)  // #131 [ref=1x]
};
// ----------------------------------------------------------------------------
// ${AltOpcodeTable:End}

#undef O
#undef V
#undef E
#undef O_FPU

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
  { 0                                                     , 363, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #1 [ref=4x]
  { 0                                                     , 364, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #2 [ref=2x]
  { F(Lock)|F(XAcquire)|F(XRelease)                       , 23 , 12, CONTROL(None)   , SINGLE_REG(None), 0 }, // #3 [ref=2x]
  { 0                                                     , 172, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #4 [ref=2x]
  { F(Vec)                                                , 86 , 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #5 [ref=54x]
  { F(Vec)                                                , 113, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #6 [ref=19x]
  { F(Vec)                                                , 246, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #7 [ref=16x]
  { F(Vec)                                                , 204, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #8 [ref=20x]
  { F(Lock)|F(XAcquire)|F(XRelease)                       , 35 , 11, CONTROL(None)   , SINGLE_REG(RO)  , 0 }, // #9 [ref=1x]
  { F(Vex)                                                , 261, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #10 [ref=3x]
  { F(Vec)                                                , 86 , 1 , CONTROL(None)   , SINGLE_REG(RO)  , 0 }, // #11 [ref=12x]
  { 0                                                     , 365, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #12 [ref=1x]
  { F(Vex)                                                , 263, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #13 [ref=5x]
  { F(Vex)                                                , 172, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #14 [ref=12x]
  { F(Vec)                                                , 366, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #15 [ref=4x]
  { 0                                                     , 265, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #16 [ref=3x]
  { F(Mib)                                                , 367, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #17 [ref=1x]
  { 0                                                     , 368, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #18 [ref=1x]
  { 0                                                     , 267, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #19 [ref=1x]
  { F(Mib)                                                , 369, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #20 [ref=1x]
  { 0                                                     , 269, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #21 [ref=1x]
  { 0                                                     , 171, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #22 [ref=35x]
  { 0                                                     , 370, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #23 [ref=3x]
  { 0                                                     , 135, 4 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #24 [ref=1x]
  { F(Lock)|F(XAcquire)|F(XRelease)                       , 135, 4 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #25 [ref=3x]
  { F(Rep)|F(RepIgnored)                                  , 271, 2 , CONTROL(Call)   , SINGLE_REG(None), 0 }, // #26 [ref=1x]
  { 0                                                     , 371, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #27 [ref=1x]
  { 0                                                     , 372, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #28 [ref=2x]
  { 0                                                     , 346, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #29 [ref=1x]
  { 0                                                     , 115, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #30 [ref=83x]
  { 0                                                     , 373, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #31 [ref=24x]
  { 0                                                     , 374, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #32 [ref=6x]
  { 0                                                     , 375, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #33 [ref=1x]
  { 0                                                     , 23 , 12, CONTROL(None)   , SINGLE_REG(None), 0 }, // #34 [ref=1x]
  { F(Rep)                                                , 376, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #35 [ref=1x]
  { F(Vec)                                                , 377, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #36 [ref=2x]
  { F(Vec)                                                , 378, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #37 [ref=3x]
  { F(Lock)|F(XAcquire)|F(XRelease)                       , 139, 4 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #38 [ref=1x]
  { F(Lock)|F(XAcquire)|F(XRelease)                       , 379, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #39 [ref=1x]
  { F(Lock)|F(XAcquire)|F(XRelease)                       , 380, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #40 [ref=1x]
  { 0                                                     , 381, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #41 [ref=1x]
  { 0                                                     , 382, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #42 [ref=1x]
  { 0                                                     , 273, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #43 [ref=1x]
  { F(Mmx)|F(Vec)                                         , 383, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #44 [ref=2x]
  { F(Mmx)|F(Vec)                                         , 384, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #45 [ref=2x]
  { F(Mmx)|F(Vec)                                         , 385, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #46 [ref=2x]
  { F(Vec)                                                , 386, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #47 [ref=2x]
  { F(Vec)                                                , 387, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #48 [ref=2x]
  { F(Vec)                                                , 388, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #49 [ref=2x]
  { 0                                                     , 389, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #50 [ref=1x]
  { 0                                                     , 390, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #51 [ref=2x]
  { F(Lock)|F(XAcquire)|F(XRelease)                       , 275, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #52 [ref=2x]
  { 0                                                     , 46 , 4 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #53 [ref=3x]
  { F(Mmx)                                                , 115, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #54 [ref=1x]
  { 0                                                     , 277, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #55 [ref=2x]
  { 0                                                     , 391, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #56 [ref=1x]
  { F(Vec)                                                , 392, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #57 [ref=2x]
  { F(Vec)                                                , 279, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #58 [ref=1x]
  { F(FpuM32)|F(FpuM64)                                   , 174, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #59 [ref=6x]
  { 0                                                     , 281, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #60 [ref=9x]
  { F(FpuM80)                                             , 393, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #61 [ref=2x]
  { 0                                                     , 282, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #62 [ref=13x]
  { F(FpuM32)|F(FpuM64)                                   , 283, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #63 [ref=2x]
  { F(FpuM16)|F(FpuM32)                                   , 394, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #64 [ref=9x]
  { F(FpuM16)|F(FpuM32)|F(FpuM64)                         , 395, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #65 [ref=3x]
  { F(FpuM32)|F(FpuM64)|F(FpuM80)                         , 396, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #66 [ref=2x]
  { F(FpuM16)                                             , 397, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #67 [ref=3x]
  { F(FpuM16)                                             , 398, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #68 [ref=2x]
  { F(FpuM32)|F(FpuM64)                                   , 284, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #69 [ref=1x]
  { 0                                                     , 399, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #70 [ref=2x]
  { 0                                                     , 46 , 10, CONTROL(None)   , SINGLE_REG(None), 0 }, // #71 [ref=1x]
  { 0                                                     , 400, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #72 [ref=1x]
  { 0                                                     , 401, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #73 [ref=2x]
  { 0                                                     , 330, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #74 [ref=2x]
  { F(Rep)                                                , 402, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #75 [ref=1x]
  { F(Vec)                                                , 285, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #76 [ref=1x]
  { 0                                                     , 403, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #77 [ref=2x]
  { 0                                                     , 404, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #78 [ref=8x]
  { 0                                                     , 287, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #79 [ref=3x]
  { 0                                                     , 289, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #80 [ref=1x]
  { 0                                                     , 115, 1 , CONTROL(Return) , SINGLE_REG(None), 0 }, // #81 [ref=3x]
  { 0                                                     , 405, 1 , CONTROL(Return) , SINGLE_REG(None), 0 }, // #82 [ref=1x]
  { F(Rep)|F(RepIgnored)                                  , 291, 2 , CONTROL(Branch) , SINGLE_REG(None), 0 }, // #83 [ref=30x]
  { F(Rep)|F(RepIgnored)                                  , 293, 2 , CONTROL(Branch) , SINGLE_REG(None), 0 }, // #84 [ref=1x]
  { F(Rep)|F(RepIgnored)                                  , 295, 2 , CONTROL(Jump)   , SINGLE_REG(None), 0 }, // #85 [ref=1x]
  { F(Vec)|F(Vex)                                         , 406, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #86 [ref=27x]
  { F(Vec)|F(Vex)                                         , 297, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #87 [ref=1x]
  { F(Vec)|F(Vex)                                         , 299, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #88 [ref=1x]
  { F(Vec)|F(Vex)                                         , 301, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #89 [ref=1x]
  { F(Vec)|F(Vex)                                         , 303, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #90 [ref=1x]
  { F(Vec)|F(Vex)                                         , 407, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #91 [ref=12x]
  { F(Vec)|F(Vex)                                         , 408, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #92 [ref=8x]
  { 0                                                     , 409, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #93 [ref=2x]
  { 0                                                     , 305, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #94 [ref=1x]
  { F(Vec)                                                , 213, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #95 [ref=2x]
  { 0                                                     , 410, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #96 [ref=2x]
  { 0                                                     , 307, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #97 [ref=2x]
  { F(Vex)                                                , 411, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #98 [ref=2x]
  { 0                                                     , 412, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #99 [ref=1x]
  { 0                                                     , 177, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #100 [ref=3x]
  { 0                                                     , 413, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #101 [ref=5x]
  { F(Vex)                                                , 414, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #102 [ref=2x]
  { F(Rep)                                                , 415, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #103 [ref=1x]
  { 0                                                     , 293, 2 , CONTROL(Branch) , SINGLE_REG(None), 0 }, // #104 [ref=3x]
  { 0                                                     , 309, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #105 [ref=1x]
  { F(Vex)                                                , 416, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #106 [ref=2x]
  { F(Vec)                                                , 417, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #107 [ref=1x]
  { F(Mmx)                                                , 418, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #108 [ref=1x]
  { 0                                                     , 419, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #109 [ref=2x]
  { F(XRelease)                                           , 0  , 23, CONTROL(None)   , SINGLE_REG(None), 0 }, // #110 [ref=1x]
  { 0                                                     , 56 , 9 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #111 [ref=1x]
  { F(Vec)                                                , 86 , 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #112 [ref=6x]
  { 0                                                     , 80 , 6 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #113 [ref=1x]
  { F(Mmx)|F(Vec)                                         , 311, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #114 [ref=1x]
  { 0                                                     , 420, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #115 [ref=1x]
  { 0                                                     , 84 , 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #116 [ref=2x]
  { F(Mmx)|F(Vec)                                         , 421, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #117 [ref=1x]
  { F(Vec)                                                , 280, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #118 [ref=2x]
  { F(Vec)                                                , 219, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #119 [ref=4x]
  { F(Vec)                                                , 422, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #120 [ref=2x]
  { F(Vec)                                                , 87 , 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #121 [ref=3x]
  { F(Mmx)                                                , 423, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #122 [ref=1x]
  { F(Vec)                                                , 114, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #123 [ref=1x]
  { F(Vec)                                                , 222, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #124 [ref=1x]
  { F(Mmx)|F(Vec)                                         , 110, 5 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #125 [ref=1x]
  { F(Mmx)|F(Vec)                                         , 424, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #126 [ref=1x]
  { F(Rep)                                                , 425, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #127 [ref=1x]
  { F(Vec)                                                , 113, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #128 [ref=1x]
  { F(Vec)                                                , 313, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #129 [ref=1x]
  { 0                                                     , 315, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #130 [ref=2x]
  { 0                                                     , 317, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #131 [ref=1x]
  { F(Vex)                                                , 319, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #132 [ref=1x]
  { 0                                                     , 426, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #133 [ref=1x]
  { 0                                                     , 427, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #134 [ref=1x]
  { F(Lock)|F(XAcquire)|F(XRelease)                       , 276, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #135 [ref=2x]
  { 0                                                     , 115, 5 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #136 [ref=1x]
  { F(Lock)|F(XAcquire)|F(XRelease)                       , 23 , 12, CONTROL(None)   , SINGLE_REG(RO)  , 0 }, // #137 [ref=1x]
  { 0                                                     , 428, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #138 [ref=1x]
  { F(Rep)                                                , 429, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #139 [ref=1x]
  { F(Mmx)|F(Vec)                                         , 321, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #140 [ref=40x]
  { F(Mmx)|F(Vec)                                         , 323, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #141 [ref=1x]
  { F(Mmx)|F(Vec)                                         , 321, 2 , CONTROL(None)   , SINGLE_REG(RO)  , 0 }, // #142 [ref=6x]
  { F(Mmx)|F(Vec)                                         , 321, 2 , CONTROL(None)   , SINGLE_REG(WO)  , 0 }, // #143 [ref=16x]
  { F(Mmx)                                                , 321, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #144 [ref=26x]
  { F(Vec)                                                , 86 , 1 , CONTROL(None)   , SINGLE_REG(WO)  , 0 }, // #145 [ref=4x]
  { F(Vec)                                                , 430, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #146 [ref=1x]
  { F(Vec)                                                , 431, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #147 [ref=1x]
  { F(Vec)                                                , 432, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #148 [ref=1x]
  { F(Vec)                                                , 433, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #149 [ref=1x]
  { F(Vec)                                                , 434, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #150 [ref=1x]
  { F(Vec)                                                , 435, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #151 [ref=1x]
  { F(Mmx)|F(Vec)                                         , 325, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #152 [ref=1x]
  { F(Vec)                                                , 436, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #153 [ref=1x]
  { F(Vec)                                                , 437, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #154 [ref=1x]
  { F(Vec)                                                , 438, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #155 [ref=1x]
  { F(Mmx)|F(Vec)                                         , 439, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #156 [ref=1x]
  { F(Mmx)|F(Vec)                                         , 440, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #157 [ref=1x]
  { F(Vec)                                                , 249, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #158 [ref=2x]
  { 0                                                     , 143, 4 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #159 [ref=1x]
  { 0                                                     , 405, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #160 [ref=9x]
  { F(Mmx)                                                , 323, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #161 [ref=1x]
  { F(Mmx)|F(Vec)                                         , 327, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #162 [ref=8x]
  { F(Vec)                                                , 441, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #163 [ref=2x]
  { 0                                                     , 442, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #164 [ref=1x]
  { 0                                                     , 147, 4 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #165 [ref=1x]
  { 0                                                     , 443, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #166 [ref=8x]
  { 0                                                     , 444, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #167 [ref=4x]
  { 0                                                     , 445, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #168 [ref=8x]
  { 0                                                     , 329, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #169 [ref=1x]
  { F(Rep)|F(RepIgnored)                                  , 331, 2 , CONTROL(Return) , SINGLE_REG(None), 0 }, // #170 [ref=1x]
  { F(Vex)                                                , 333, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #171 [ref=1x]
  { F(Lock)|F(XAcquire)|F(XRelease)                       , 23 , 12, CONTROL(None)   , SINGLE_REG(WO)  , 0 }, // #172 [ref=3x]
  { F(Rep)                                                , 446, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #173 [ref=1x]
  { 0                                                     , 447, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #174 [ref=30x]
  { 0                                                     , 180, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #175 [ref=2x]
  { 0                                                     , 448, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #176 [ref=3x]
  { F(Rep)                                                , 449, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #177 [ref=1x]
  { F(Vex)                                                , 450, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #178 [ref=5x]
  { 0                                                     , 73 , 7 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #179 [ref=1x]
  { F(Tsib)|F(Vex)                                        , 451, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #180 [ref=2x]
  { F(Vex)                                                , 405, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #181 [ref=1x]
  { F(Tsib)|F(Vex)                                        , 452, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #182 [ref=1x]
  { F(Vex)                                                , 453, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #183 [ref=1x]
  { 0                                                     , 454, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #184 [ref=2x]
  { 0                                                     , 455, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #185 [ref=2x]
  { 0                                                     , 456, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #186 [ref=1x]
  { F(Vec)|F(Evex)|F(Avx512T4X)|F(Avx512KZ)               , 457, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #187 [ref=4x]
  { F(Vec)|F(Evex)|F(Avx512T4X)|F(Avx512KZ)               , 458, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #188 [ref=2x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_ER_SAE_B64)          , 183, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #189 [ref=22x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_ER_SAE_B32)          , 183, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #190 [ref=22x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_ER_SAE)              , 459, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #191 [ref=18x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_ER_SAE)              , 460, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #192 [ref=17x]
  { F(Vec)|F(Vex)                                         , 183, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #193 [ref=15x]
  { F(Vec)|F(Vex)|F(Evex)                                 , 183, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #194 [ref=5x]
  { F(Vec)|F(Vex)                                         , 86 , 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #195 [ref=17x]
  { F(Vec)|F(Vex)                                         , 204, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #196 [ref=1x]
  { F(Vec)|F(Evex)|F(Avx512KZ_B32)                        , 186, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #197 [ref=4x]
  { F(Vec)|F(Evex)|F(Avx512KZ_B64)                        , 186, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #198 [ref=4x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_B64)                 , 183, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #199 [ref=10x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_B32)                 , 183, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #200 [ref=12x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_B64)                 , 183, 3 , CONTROL(None)   , SINGLE_REG(RO)  , 0 }, // #201 [ref=2x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_B32)                 , 183, 3 , CONTROL(None)   , SINGLE_REG(RO)  , 0 }, // #202 [ref=6x]
  { F(Vec)|F(Evex)|F(Avx512KZ)                            , 183, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #203 [ref=13x]
  { F(Vec)|F(Evex)|F(Avx512KZ_B32)                        , 183, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #204 [ref=12x]
  { F(Vec)|F(Evex)|F(Avx512KZ_B64)                        , 183, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #205 [ref=19x]
  { F(Vec)|F(Vex)                                         , 186, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #206 [ref=6x]
  { F(Vec)|F(Vex)                                         , 335, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #207 [ref=3x]
  { F(Vec)|F(Vex)                                         , 461, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #208 [ref=2x]
  { F(Vec)|F(Evex)|F(Avx512KZ)                            , 462, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #209 [ref=1x]
  { F(Vec)|F(Evex)|F(Avx512KZ)                            , 463, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #210 [ref=4x]
  { F(Vec)|F(Evex)|F(Avx512KZ)                            , 464, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #211 [ref=4x]
  { F(Vec)|F(Evex)|F(Avx512KZ)                            , 465, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #212 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)                     , 462, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #213 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)                     , 466, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #214 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_SAE_B64)             , 189, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #215 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_SAE_B32)             , 189, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #216 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_SAE)                 , 467, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #217 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_SAE)                 , 468, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #218 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512SAE)                    , 113, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #219 [ref=2x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512SAE)                    , 246, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #220 [ref=2x]
  { F(Vec)|F(Evex)|F(Avx512KZ)                            , 192, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #221 [ref=6x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_B32)                 , 195, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #222 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_ER_SAE_B32)          , 198, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #223 [ref=3x]
  { F(Vec)|F(Evex)|F(Avx512KZ_B32)                        , 337, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #224 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_ER_SAE_B64)          , 337, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #225 [ref=2x]
  { F(Vec)|F(Evex)|F(Avx512KZ_ER_SAE_B64)                 , 198, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #226 [ref=4x]
  { F(Vec)|F(Evex)|F(Avx512KZ_ER_SAE_B64)                 , 337, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #227 [ref=3x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_SAE)                 , 195, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #228 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_ER_SAE_B32)          , 195, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #229 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_SAE)                 , 201, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #230 [ref=1x]
  { F(Vec)|F(Evex)|F(Avx512KZ_ER_SAE_B32)                 , 195, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #231 [ref=2x]
  { F(Vec)|F(Evex)|F(Avx512KZ_ER_SAE_B32)                 , 198, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #232 [ref=2x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512ER_SAE)                 , 386, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #233 [ref=1x]
  { F(Vec)|F(Evex)|F(Avx512ER_SAE)                        , 386, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #234 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512ER_SAE)                 , 469, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #235 [ref=2x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_SAE)                 , 460, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #236 [ref=3x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512ER_SAE)                 , 388, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #237 [ref=1x]
  { F(Vec)|F(Evex)|F(Avx512ER_SAE)                        , 388, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #238 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_SAE_B64)             , 337, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #239 [ref=1x]
  { F(Vec)|F(Evex)|F(Avx512KZ_SAE_B64)                    , 198, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #240 [ref=3x]
  { F(Vec)|F(Evex)|F(Avx512KZ_SAE_B64)                    , 337, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #241 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_SAE_B32)             , 198, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #242 [ref=1x]
  { F(Vec)|F(Evex)|F(Avx512KZ_SAE_B32)                    , 195, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #243 [ref=2x]
  { F(Vec)|F(Evex)|F(Avx512KZ_SAE_B32)                    , 198, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #244 [ref=2x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512SAE)                    , 386, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #245 [ref=1x]
  { F(Vec)|F(Evex)|F(Avx512SAE)                           , 386, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #246 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512SAE)                    , 388, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #247 [ref=1x]
  { F(Vec)|F(Evex)|F(Avx512SAE)                           , 388, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #248 [ref=1x]
  { F(Vec)|F(Evex)|F(Avx512KZ_B32)                        , 195, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #249 [ref=1x]
  { F(Vec)|F(Evex)|F(Avx512ER_SAE)                        , 469, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #250 [ref=2x]
  { F(Vec)|F(Evex)|F(Avx512KZ)                            , 186, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #251 [ref=3x]
  { F(Vec)|F(Vex)                                         , 186, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #252 [ref=9x]
  { F(Vec)|F(Evex)|F(Avx512KZ_SAE_B64)                    , 90 , 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #253 [ref=3x]
  { F(Vec)|F(Evex)|F(Avx512KZ_SAE_B32)                    , 90 , 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #254 [ref=3x]
  { F(Vec)|F(Evex)|F(Avx512KZ)                            , 198, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #255 [ref=9x]
  { F(Vec)|F(Vex)                                         , 202, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #256 [ref=2x]
  { F(Vec)|F(Evex)|F(Avx512KZ)                            , 470, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #257 [ref=4x]
  { F(Vec)|F(Evex)|F(Avx512KZ)                            , 203, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #258 [ref=4x]
  { F(Vec)|F(Vex)|F(Evex)                                 , 392, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #259 [ref=2x]
  { F(Vec)|F(Evex)|F(Avx512KZ_SAE_B64)                    , 186, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #260 [ref=2x]
  { F(Vec)|F(Evex)|F(Avx512KZ_SAE_B32)                    , 186, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #261 [ref=2x]
  { F(Vec)|F(Evex)|F(Avx512KZ_SAE)                        , 471, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #262 [ref=4x]
  { F(Vec)|F(Evex)|F(Avx512KZ_SAE)                        , 472, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #263 [ref=4x]
  { F(Vec)|F(Vex)                                         , 151, 4 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #264 [ref=13x]
  { F(Vec)|F(Vex)                                         , 339, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #265 [ref=4x]
  { F(Vec)|F(Vex)                                         , 341, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #266 [ref=4x]
  { F(Vec)|F(Evex)|F(Avx512K_B64)                         , 473, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #267 [ref=1x]
  { F(Vec)|F(Evex)|F(Avx512K_B32)                         , 473, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #268 [ref=1x]
  { F(Vec)|F(Evex)|F(Avx512K)                             , 474, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #269 [ref=1x]
  { F(Vec)|F(Evex)|F(Avx512K)                             , 475, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #270 [ref=1x]
  { F(Vec)|F(Vex)                                         , 198, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #271 [ref=7x]
  { F(Vec)|F(Vex)                                         , 113, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #272 [ref=1x]
  { F(Vec)|F(Vex)                                         , 246, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #273 [ref=1x]
  { F(Vec)|F(Vsib)|F(Vex)|F(Evex)|F(Avx512K)              , 120, 5 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #274 [ref=2x]
  { F(Vec)|F(Vsib)|F(Vex)|F(Evex)|F(Avx512K)              , 125, 5 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #275 [ref=2x]
  { F(Vsib)|F(Evex)|F(Avx512K)                            , 476, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #276 [ref=4x]
  { F(Vsib)|F(Evex)|F(Avx512K)                            , 477, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #277 [ref=4x]
  { F(Vsib)|F(Evex)|F(Avx512K)                            , 478, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #278 [ref=8x]
  { F(Vec)|F(Vsib)|F(Vex)|F(Evex)|F(Avx512K)              , 130, 5 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #279 [ref=2x]
  { F(Vec)|F(Vsib)|F(Vex)|F(Evex)|F(Avx512K)              , 155, 4 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #280 [ref=2x]
  { F(Vec)|F(Evex)|F(Avx512KZ_SAE)                        , 459, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #281 [ref=3x]
  { F(Vec)|F(Evex)|F(Avx512KZ_SAE)                        , 460, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #282 [ref=3x]
  { F(Vec)|F(Evex)|F(Avx512KZ_SAE_B64)                    , 204, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #283 [ref=2x]
  { F(Vec)|F(Evex)|F(Avx512KZ_SAE_B32)                    , 204, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #284 [ref=2x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)                     , 186, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #285 [ref=3x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)                     , 183, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #286 [ref=22x]
  { F(Vec)|F(Vex)                                         , 343, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #287 [ref=2x]
  { F(Vec)|F(Evex)|F(Avx512KZ)                            , 343, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #288 [ref=4x]
  { F(Vec)|F(Evex)|F(Avx512KZ)                            , 479, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #289 [ref=4x]
  { F(Vec)|F(Vex)|F(Evex)                                 , 472, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #290 [ref=1x]
  { F(Vec)|F(Vex)                                         , 213, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #291 [ref=1x]
  { F(Vex)                                                , 410, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #292 [ref=2x]
  { F(Vec)|F(Vex)                                         , 417, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #293 [ref=1x]
  { F(Vec)|F(Vex)                                         , 159, 4 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #294 [ref=4x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_SAE_B64)             , 183, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #295 [ref=2x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_SAE_B32)             , 183, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #296 [ref=2x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_SAE)                 , 459, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #297 [ref=2x]
  { 0                                                     , 345, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #298 [ref=3x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)                     , 86 , 6 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #299 [ref=4x]
  { F(Vec)|F(Vex)|F(Evex)                                 , 347, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #300 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)                     , 207, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #301 [ref=1x]
  { F(Vec)|F(Vex)                                         , 86 , 4 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #302 [ref=2x]
  { F(Vec)|F(Evex)|F(Avx512KZ)                            , 86 , 6 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #303 [ref=6x]
  { F(Vec)|F(Vex)|F(Evex)                                 , 221, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #304 [ref=2x]
  { F(Vec)|F(Vex)|F(Evex)                                 , 349, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #305 [ref=4x]
  { F(Vec)|F(Vex)                                         , 480, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #306 [ref=3x]
  { F(Vec)|F(Vex)|F(Evex)                                 , 210, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #307 [ref=3x]
  { F(Vec)|F(Vex)|F(Evex)                                 , 213, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #308 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)                                 , 216, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #309 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)                     , 219, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #310 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)                     , 198, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #311 [ref=5x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)                     , 222, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #312 [ref=1x]
  { 0                                                     , 351, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #313 [ref=1x]
  { 0                                                     , 353, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #314 [ref=1x]
  { F(Vec)|F(Evex)|F(Avx512B32)                           , 225, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #315 [ref=1x]
  { F(Vec)|F(Evex)|F(Avx512B64)                           , 225, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #316 [ref=1x]
  { F(Vec)|F(Vex)                                         , 183, 2 , CONTROL(None)   , SINGLE_REG(RO)  , 0 }, // #317 [ref=2x]
  { F(Vec)|F(Evex)|F(Avx512KZ_B32)                        , 183, 3 , CONTROL(None)   , SINGLE_REG(RO)  , 0 }, // #318 [ref=2x]
  { F(Vec)|F(Vex)                                         , 183, 2 , CONTROL(None)   , SINGLE_REG(WO)  , 0 }, // #319 [ref=2x]
  { F(Vec)|F(Evex)|F(Avx512KZ_B32)                        , 183, 3 , CONTROL(None)   , SINGLE_REG(WO)  , 0 }, // #320 [ref=2x]
  { F(Vec)|F(Evex)|F(Avx512KZ_B64)                        , 183, 3 , CONTROL(None)   , SINGLE_REG(WO)  , 0 }, // #321 [ref=2x]
  { F(Vec)|F(Evex)|F(Avx512KZ_B64)                        , 183, 3 , CONTROL(None)   , SINGLE_REG(RO)  , 0 }, // #322 [ref=2x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)                     , 481, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #323 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)                     , 482, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #324 [ref=1x]
  { F(Vec)|F(Evex)                                        , 483, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #325 [ref=6x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)                     , 228, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #326 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)                     , 484, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #327 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)                                 , 186, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #328 [ref=1x]
  { F(Vec)|F(Evex)|F(Avx512K)                             , 231, 3 , CONTROL(None)   , SINGLE_REG(WO)  , 0 }, // #329 [ref=2x]
  { F(Vec)|F(Evex)|F(Avx512K_B32)                         , 231, 3 , CONTROL(None)   , SINGLE_REG(WO)  , 0 }, // #330 [ref=2x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512K)                      , 234, 3 , CONTROL(None)   , SINGLE_REG(WO)  , 0 }, // #331 [ref=4x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512K_B32)                  , 234, 3 , CONTROL(None)   , SINGLE_REG(WO)  , 0 }, // #332 [ref=2x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512K_B64)                  , 234, 3 , CONTROL(None)   , SINGLE_REG(WO)  , 0 }, // #333 [ref=2x]
  { F(Vec)|F(Vex)                                         , 430, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #334 [ref=1x]
  { F(Vec)|F(Vex)                                         , 431, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #335 [ref=1x]
  { F(Vec)|F(Vex)                                         , 432, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #336 [ref=1x]
  { F(Vec)|F(Vex)                                         , 433, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #337 [ref=1x]
  { F(Vec)|F(Evex)|F(Avx512K_B64)                         , 231, 3 , CONTROL(None)   , SINGLE_REG(WO)  , 0 }, // #338 [ref=4x]
  { F(Vec)|F(Evex)|F(Avx512KZ_B32)                        , 198, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #339 [ref=6x]
  { F(Vec)|F(Vex)|F(Evex)|F(PreferEvex)|F(Avx512KZ_B32)   , 183, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #340 [ref=4x]
  { F(Vec)|F(Vex)                                         , 187, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #341 [ref=2x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_B32)                 , 184, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #342 [ref=2x]
  { F(Vec)|F(Vex)                                         , 163, 4 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #343 [ref=2x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_B64)                 , 92 , 6 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #344 [ref=2x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_B64)                 , 167, 4 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #345 [ref=2x]
  { F(Vec)|F(Vex)|F(Evex)                                 , 434, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #346 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)                                 , 435, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #347 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)                                 , 485, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #348 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)                     , 486, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #349 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)                     , 487, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #350 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)                     , 488, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #351 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)                     , 489, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #352 [ref=1x]
  { F(Vec)|F(Evex)|F(Avx512KZ_B64)                        , 198, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #353 [ref=4x]
  { F(Vec)|F(Vex)                                         , 335, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #354 [ref=12x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)                     , 183, 3 , CONTROL(None)   , SINGLE_REG(RO)  , 0 }, // #355 [ref=8x]
  { F(Vec)|F(Evex)                                        , 490, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #356 [ref=4x]
  { F(Vec)|F(Evex)|F(Avx512KZ)                            , 237, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #357 [ref=6x]
  { F(Vec)|F(Evex)|F(Avx512KZ)                            , 240, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #358 [ref=9x]
  { F(Vec)|F(Evex)|F(Avx512KZ)                            , 243, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #359 [ref=3x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)                     , 246, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #360 [ref=4x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)                     , 249, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #361 [ref=2x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)                     , 195, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #362 [ref=6x]
  { F(Vec)|F(Vex)                                         , 151, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #363 [ref=1x]
  { F(Vec)|F(Evex)|F(Avx512KZ_B32)                        , 204, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #364 [ref=3x]
  { F(Vec)|F(Evex)|F(Avx512KZ_B64)                        , 204, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #365 [ref=3x]
  { F(Vec)|F(Vex)                                         , 355, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #366 [ref=4x]
  { F(Vec)|F(Vsib)|F(Evex)|F(Avx512K)                     , 252, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #367 [ref=3x]
  { F(Vec)|F(Vsib)|F(Evex)|F(Avx512K)                     , 357, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #368 [ref=2x]
  { F(Vec)|F(Vsib)|F(Evex)|F(Avx512K)                     , 255, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #369 [ref=2x]
  { F(Vec)|F(Vex)                                         , 359, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #370 [ref=8x]
  { F(Vec)|F(Evex)|F(Avx512K)                             , 258, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #371 [ref=5x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_B32)                 , 204, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #372 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)                     , 204, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #373 [ref=2x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_B32)                 , 98 , 6 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #374 [ref=3x]
  { F(Vec)|F(Vex)|F(Evex)                                 , 204, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #375 [ref=2x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_B64)                 , 98 , 6 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #376 [ref=2x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)                     , 98 , 6 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #377 [ref=3x]
  { F(Vec)|F(Evex)|F(Avx512KZ_B64)                        , 104, 6 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #378 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)                     , 183, 3 , CONTROL(None)   , SINGLE_REG(WO)  , 0 }, // #379 [ref=6x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_B32)                 , 183, 3 , CONTROL(None)   , SINGLE_REG(WO)  , 0 }, // #380 [ref=2x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_B64)                 , 183, 3 , CONTROL(None)   , SINGLE_REG(WO)  , 0 }, // #381 [ref=2x]
  { F(Vec)|F(Evex)|F(Avx512K_B32)                         , 258, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #382 [ref=2x]
  { F(Vec)|F(Evex)|F(Avx512K_B64)                         , 258, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #383 [ref=2x]
  { F(Vec)|F(Evex)|F(Avx512KZ)                            , 459, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #384 [ref=2x]
  { F(Vec)|F(Evex)|F(Avx512KZ)                            , 460, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #385 [ref=2x]
  { F(Vec)|F(Vex)                                         , 460, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #386 [ref=2x]
  { F(Vec)|F(Evex)|F(Avx512KZ)                            , 471, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #387 [ref=1x]
  { F(Vec)|F(Evex)|F(Avx512KZ)                            , 472, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #388 [ref=1x]
  { F(Vec)|F(Vex)                                         , 204, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #389 [ref=2x]
  { F(Vec)|F(Vex)                                         , 471, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #390 [ref=1x]
  { F(Vec)|F(Vex)                                         , 472, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #391 [ref=1x]
  { F(Vec)|F(Evex)|F(Avx512KZ_ER_SAE_B64)                 , 183, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #392 [ref=1x]
  { F(Vec)|F(Evex)|F(Avx512KZ_ER_SAE_B32)                 , 183, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #393 [ref=1x]
  { F(Vec)|F(Evex)|F(Avx512KZ_ER_SAE)                     , 459, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #394 [ref=1x]
  { F(Vec)|F(Evex)|F(Avx512KZ_ER_SAE)                     , 460, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #395 [ref=1x]
  { F(Vec)|F(Vsib)|F(Evex)|F(Avx512K)                     , 361, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #396 [ref=1x]
  { F(Vec)|F(Evex)|F(Avx512KZ_B32)                        , 187, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #397 [ref=2x]
  { F(Vec)|F(Evex)|F(Avx512KZ_B64)                        , 187, 2 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #398 [ref=2x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_B32)                 , 186, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #399 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_B64)                 , 186, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #400 [ref=1x]
  { F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_ER_SAE_B64)          , 198, 3 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #401 [ref=1x]
  { F(Vec)|F(Vex)                                         , 115, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #402 [ref=2x]
  { 0                                                     , 30 , 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #403 [ref=2x]
  { 0                                                     , 68 , 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #404 [ref=2x]
  { F(Lock)|F(XAcquire)|F(XRelease)                       , 65 , 4 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #405 [ref=1x]
  { 0                                                     , 491, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #406 [ref=1x]
  { F(Lock)|F(XAcquire)                                   , 65 , 8 , CONTROL(None)   , SINGLE_REG(RO)  , 0 }, // #407 [ref=1x]
  { 0                                                     , 492, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }, // #408 [ref=6x]
  { 0                                                     , 493, 1 , CONTROL(None)   , SINGLE_REG(None), 0 }  // #409 [ref=6x]
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
  { { 0 }, 1, 0 }, // #1 [ref=32x]
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
  { { 0 }, 6, 0 }, // #14 [ref=4x]
  { { EXT(BMI2) }, 1, 0 }, // #15 [ref=1x]
  { { EXT(SMAP) }, 7, 0 }, // #16 [ref=2x]
  { { 0 }, 8, 0 }, // #17 [ref=2x]
  { { 0 }, 9, 0 }, // #18 [ref=2x]
  { { EXT(CLDEMOTE) }, 0, 0 }, // #19 [ref=1x]
  { { EXT(CLFLUSH) }, 0, 0 }, // #20 [ref=1x]
  { { EXT(CLFLUSHOPT) }, 0, 0 }, // #21 [ref=1x]
  { { EXT(SVM) }, 0, 0 }, // #22 [ref=6x]
  { { 0 }, 10, 0 }, // #23 [ref=2x]
  { { EXT(CET_SS) }, 1, 0 }, // #24 [ref=3x]
  { { EXT(CLWB) }, 0, 0 }, // #25 [ref=1x]
  { { EXT(CLZERO) }, 0, 0 }, // #26 [ref=1x]
  { { 0 }, 3, 0 }, // #27 [ref=1x]
  { { EXT(CMOV) }, 11, 0 }, // #28 [ref=6x]
  { { EXT(CMOV) }, 12, 0 }, // #29 [ref=8x]
  { { EXT(CMOV) }, 13, 0 }, // #30 [ref=6x]
  { { EXT(CMOV) }, 14, 0 }, // #31 [ref=4x]
  { { EXT(CMOV) }, 15, 0 }, // #32 [ref=4x]
  { { EXT(CMOV) }, 16, 0 }, // #33 [ref=2x]
  { { EXT(CMOV) }, 17, 0 }, // #34 [ref=6x]
  { { EXT(CMOV) }, 18, 0 }, // #35 [ref=2x]
  { { 0 }, 19, 0 }, // #36 [ref=2x]
  { { EXT(I486) }, 1, 0 }, // #37 [ref=2x]
  { { EXT(CMPXCHG16B) }, 5, 0 }, // #38 [ref=1x]
  { { EXT(CMPXCHG8B) }, 5, 0 }, // #39 [ref=1x]
  { { EXT(SSE2) }, 1, 0 }, // #40 [ref=2x]
  { { EXT(SSE) }, 1, 0 }, // #41 [ref=2x]
  { { EXT(I486) }, 0, 0 }, // #42 [ref=4x]
  { { EXT(SSE4_2) }, 0, 0 }, // #43 [ref=2x]
  { { 0 }, 20, 0 }, // #44 [ref=2x]
  { { EXT(MMX) }, 0, 0 }, // #45 [ref=1x]
  { { EXT(CET_IBT) }, 0, 0 }, // #46 [ref=2x]
  { { EXT(ENQCMD) }, 0, 0 }, // #47 [ref=2x]
  { { EXT(SSE4A) }, 0, 0 }, // #48 [ref=4x]
  { { 0 }, 21, 0 }, // #49 [ref=4x]
  { { EXT(3DNOW) }, 0, 0 }, // #50 [ref=21x]
  { { EXT(FXSR) }, 0, 0 }, // #51 [ref=4x]
  { { EXT(SMX) }, 0, 0 }, // #52 [ref=1x]
  { { EXT(GFNI) }, 0, 0 }, // #53 [ref=3x]
  { { EXT(CET_SS) }, 0, 0 }, // #54 [ref=9x]
  { { 0 }, 16, 0 }, // #55 [ref=5x]
  { { EXT(VMX) }, 0, 0 }, // #56 [ref=12x]
  { { 0 }, 11, 0 }, // #57 [ref=8x]
  { { 0 }, 12, 0 }, // #58 [ref=12x]
  { { 0 }, 13, 0 }, // #59 [ref=10x]
  { { 0 }, 14, 0 }, // #60 [ref=8x]
  { { 0 }, 15, 0 }, // #61 [ref=8x]
  { { 0 }, 17, 0 }, // #62 [ref=8x]
  { { 0 }, 18, 0 }, // #63 [ref=4x]
  { { EXT(AVX512_DQ) }, 0, 0 }, // #64 [ref=23x]
  { { EXT(AVX512_BW) }, 0, 0 }, // #65 [ref=22x]
  { { EXT(AVX512_F) }, 0, 0 }, // #66 [ref=37x]
  { { EXT(AVX512_DQ) }, 1, 0 }, // #67 [ref=3x]
  { { EXT(AVX512_BW) }, 1, 0 }, // #68 [ref=4x]
  { { EXT(AVX512_F) }, 1, 0 }, // #69 [ref=1x]
  { { EXT(LAHFSAHF) }, 22, 0 }, // #70 [ref=1x]
  { { EXT(AMX_TILE) }, 0, 0 }, // #71 [ref=7x]
  { { EXT(LWP) }, 0, 0 }, // #72 [ref=4x]
  { { 0 }, 23, 0 }, // #73 [ref=3x]
  { { EXT(LZCNT) }, 1, 0 }, // #74 [ref=1x]
  { { EXT(MMX2) }, 0, 0 }, // #75 [ref=8x]
  { { EXT(MCOMMIT) }, 1, 0 }, // #76 [ref=1x]
  { { EXT(MONITOR) }, 0, 0 }, // #77 [ref=2x]
  { { EXT(MONITORX) }, 0, 0 }, // #78 [ref=2x]
  { { EXT(MOVBE) }, 0, 0 }, // #79 [ref=1x]
  { { EXT(MMX), EXT(SSE2) }, 0, 0 }, // #80 [ref=46x]
  { { EXT(MOVDIR64B) }, 0, 0 }, // #81 [ref=1x]
  { { EXT(MOVDIRI) }, 0, 0 }, // #82 [ref=1x]
  { { EXT(BMI2) }, 0, 0 }, // #83 [ref=7x]
  { { EXT(SSSE3) }, 0, 0 }, // #84 [ref=15x]
  { { EXT(MMX2), EXT(SSE2) }, 0, 0 }, // #85 [ref=10x]
  { { EXT(PCLMULQDQ) }, 0, 0 }, // #86 [ref=1x]
  { { EXT(SSE4_2) }, 1, 0 }, // #87 [ref=4x]
  { { EXT(PCONFIG) }, 0, 0 }, // #88 [ref=1x]
  { { EXT(MMX2), EXT(SSE2), EXT(SSE4_1) }, 0, 0 }, // #89 [ref=1x]
  { { EXT(3DNOW2) }, 0, 0 }, // #90 [ref=5x]
  { { EXT(GEODE) }, 0, 0 }, // #91 [ref=2x]
  { { EXT(POPCNT) }, 1, 0 }, // #92 [ref=1x]
  { { 0 }, 24, 0 }, // #93 [ref=3x]
  { { EXT(PREFETCHW) }, 1, 0 }, // #94 [ref=1x]
  { { EXT(PREFETCHWT1) }, 1, 0 }, // #95 [ref=1x]
  { { EXT(SNP) }, 20, 0 }, // #96 [ref=3x]
  { { EXT(SSE4_1) }, 1, 0 }, // #97 [ref=1x]
  { { EXT(PTWRITE) }, 0, 0 }, // #98 [ref=1x]
  { { 0 }, 25, 0 }, // #99 [ref=3x]
  { { EXT(SNP) }, 1, 0 }, // #100 [ref=1x]
  { { 0 }, 26, 0 }, // #101 [ref=2x]
  { { EXT(FSGSBASE) }, 0, 0 }, // #102 [ref=4x]
  { { EXT(MSR) }, 0, 0 }, // #103 [ref=2x]
  { { EXT(RDPID) }, 0, 0 }, // #104 [ref=1x]
  { { EXT(OSPKE) }, 0, 0 }, // #105 [ref=1x]
  { { EXT(RDPRU) }, 0, 0 }, // #106 [ref=1x]
  { { EXT(RDRAND) }, 1, 0 }, // #107 [ref=1x]
  { { EXT(RDSEED) }, 1, 0 }, // #108 [ref=1x]
  { { EXT(RDTSC) }, 0, 0 }, // #109 [ref=1x]
  { { EXT(RDTSCP) }, 0, 0 }, // #110 [ref=1x]
  { { 0 }, 27, 0 }, // #111 [ref=2x]
  { { EXT(LAHFSAHF) }, 28, 0 }, // #112 [ref=1x]
  { { EXT(SERIALIZE) }, 0, 0 }, // #113 [ref=1x]
  { { EXT(SHA) }, 0, 0 }, // #114 [ref=7x]
  { { EXT(SKINIT) }, 0, 0 }, // #115 [ref=2x]
  { { EXT(AMX_BF16) }, 0, 0 }, // #116 [ref=1x]
  { { EXT(AMX_INT8) }, 0, 0 }, // #117 [ref=4x]
  { { EXT(WAITPKG) }, 1, 0 }, // #118 [ref=2x]
  { { EXT(WAITPKG) }, 0, 0 }, // #119 [ref=1x]
  { { EXT(AVX512_4FMAPS) }, 0, 0 }, // #120 [ref=4x]
  { { EXT(AVX), EXT(AVX512_F), EXT(AVX512_VL) }, 0, 0 }, // #121 [ref=46x]
  { { EXT(AVX), EXT(AVX512_F) }, 0, 0 }, // #122 [ref=32x]
  { { EXT(AVX) }, 0, 0 }, // #123 [ref=37x]
  { { EXT(AESNI), EXT(AVX), EXT(AVX512_F), EXT(AVX512_VL), EXT(VAES) }, 0, 0 }, // #124 [ref=4x]
  { { EXT(AESNI), EXT(AVX) }, 0, 0 }, // #125 [ref=2x]
  { { EXT(AVX512_F), EXT(AVX512_VL) }, 0, 0 }, // #126 [ref=112x]
  { { EXT(AVX), EXT(AVX512_DQ), EXT(AVX512_VL) }, 0, 0 }, // #127 [ref=8x]
  { { EXT(AVX512_BW), EXT(AVX512_VL) }, 0, 0 }, // #128 [ref=26x]
  { { EXT(AVX512_DQ), EXT(AVX512_VL) }, 0, 0 }, // #129 [ref=30x]
  { { EXT(AVX2) }, 0, 0 }, // #130 [ref=7x]
  { { EXT(AVX), EXT(AVX2), EXT(AVX512_F), EXT(AVX512_VL) }, 0, 0 }, // #131 [ref=39x]
  { { EXT(AVX), EXT(AVX512_F) }, 1, 0 }, // #132 [ref=4x]
  { { EXT(AVX512_BF16), EXT(AVX512_VL) }, 0, 0 }, // #133 [ref=3x]
  { { EXT(AVX512_F), EXT(AVX512_VL), EXT(F16C) }, 0, 0 }, // #134 [ref=2x]
  { { EXT(AVX512_ERI) }, 0, 0 }, // #135 [ref=10x]
  { { EXT(AVX512_F), EXT(AVX512_VL), EXT(FMA) }, 0, 0 }, // #136 [ref=36x]
  { { EXT(AVX512_F), EXT(FMA) }, 0, 0 }, // #137 [ref=24x]
  { { EXT(FMA4) }, 0, 0 }, // #138 [ref=20x]
  { { EXT(XOP) }, 0, 0 }, // #139 [ref=55x]
  { { EXT(AVX2), EXT(AVX512_F), EXT(AVX512_VL) }, 0, 0 }, // #140 [ref=19x]
  { { EXT(AVX512_PFI) }, 0, 0 }, // #141 [ref=16x]
  { { EXT(AVX), EXT(AVX512_F), EXT(AVX512_VL), EXT(GFNI) }, 0, 0 }, // #142 [ref=3x]
  { { EXT(AVX), EXT(AVX2) }, 0, 0 }, // #143 [ref=17x]
  { { EXT(AVX512_VP2INTERSECT) }, 0, 0 }, // #144 [ref=2x]
  { { EXT(AVX512_4VNNIW) }, 0, 0 }, // #145 [ref=2x]
  { { EXT(AVX), EXT(AVX2), EXT(AVX512_BW), EXT(AVX512_VL) }, 0, 0 }, // #146 [ref=54x]
  { { EXT(AVX2), EXT(AVX512_BW), EXT(AVX512_VL) }, 0, 0 }, // #147 [ref=2x]
  { { EXT(AVX512_CDI), EXT(AVX512_VL) }, 0, 0 }, // #148 [ref=6x]
  { { EXT(AVX), EXT(AVX512_F), EXT(AVX512_VL), EXT(PCLMULQDQ), EXT(VPCLMULQDQ) }, 0, 0 }, // #149 [ref=1x]
  { { EXT(AVX) }, 1, 0 }, // #150 [ref=7x]
  { { EXT(AVX512_VBMI2), EXT(AVX512_VL) }, 0, 0 }, // #151 [ref=16x]
  { { EXT(AVX512_VL), EXT(AVX512_VNNI), EXT(AVX_VNNI) }, 0, 0 }, // #152 [ref=4x]
  { { EXT(AVX512_VBMI), EXT(AVX512_VL) }, 0, 0 }, // #153 [ref=4x]
  { { EXT(AVX), EXT(AVX512_BW) }, 0, 0 }, // #154 [ref=4x]
  { { EXT(AVX), EXT(AVX512_DQ) }, 0, 0 }, // #155 [ref=4x]
  { { EXT(AVX512_IFMA), EXT(AVX512_VL) }, 0, 0 }, // #156 [ref=2x]
  { { EXT(AVX512_BITALG), EXT(AVX512_VL) }, 0, 0 }, // #157 [ref=3x]
  { { EXT(AVX512_VL), EXT(AVX512_VPOPCNTDQ) }, 0, 0 }, // #158 [ref=2x]
  { { EXT(WBNOINVD) }, 0, 0 }, // #159 [ref=1x]
  { { EXT(RTM) }, 0, 0 }, // #160 [ref=3x]
  { { EXT(XSAVE) }, 0, 0 }, // #161 [ref=6x]
  { { EXT(TSXLDTRK) }, 0, 0 }, // #162 [ref=2x]
  { { EXT(XSAVES) }, 0, 0 }, // #163 [ref=4x]
  { { EXT(XSAVEC) }, 0, 0 }, // #164 [ref=2x]
  { { EXT(XSAVEOPT) }, 0, 0 }, // #165 [ref=2x]
  { { EXT(TSX) }, 1, 0 }  // #166 [ref=1x]
};
#undef EXT

#define FLAG(VAL) uint32_t(Status::k##VAL)
const InstDB::RWFlagsInfoTable InstDB::_rwFlagsInfoTable[] = {
  { 0, 0 }, // #0 [ref=1316x]
  { 0, FLAG(AF) | FLAG(CF) | FLAG(OF) | FLAG(PF) | FLAG(SF) | FLAG(ZF) }, // #1 [ref=83x]
  { FLAG(CF), FLAG(AF) | FLAG(CF) | FLAG(OF) | FLAG(PF) | FLAG(SF) | FLAG(ZF) }, // #2 [ref=2x]
  { FLAG(CF), FLAG(CF) }, // #3 [ref=2x]
  { FLAG(OF), FLAG(OF) }, // #4 [ref=1x]
  { 0, FLAG(ZF) }, // #5 [ref=7x]
  { 0, FLAG(AF) | FLAG(CF) | FLAG(OF) | FLAG(PF) | FLAG(SF) }, // #6 [ref=4x]
  { 0, FLAG(AC) }, // #7 [ref=2x]
  { 0, FLAG(CF) }, // #8 [ref=2x]
  { 0, FLAG(DF) }, // #9 [ref=2x]
  { 0, FLAG(IF) }, // #10 [ref=2x]
  { FLAG(CF) | FLAG(ZF), 0 }, // #11 [ref=14x]
  { FLAG(CF), 0 }, // #12 [ref=20x]
  { FLAG(ZF), 0 }, // #13 [ref=16x]
  { FLAG(OF) | FLAG(SF) | FLAG(ZF), 0 }, // #14 [ref=12x]
  { FLAG(OF) | FLAG(SF), 0 }, // #15 [ref=12x]
  { FLAG(OF), 0 }, // #16 [ref=7x]
  { FLAG(PF), 0 }, // #17 [ref=14x]
  { FLAG(SF), 0 }, // #18 [ref=6x]
  { FLAG(DF), FLAG(AF) | FLAG(CF) | FLAG(OF) | FLAG(PF) | FLAG(SF) | FLAG(ZF) }, // #19 [ref=2x]
  { 0, FLAG(AF) | FLAG(OF) | FLAG(PF) | FLAG(SF) | FLAG(ZF) }, // #20 [ref=5x]
  { 0, FLAG(CF) | FLAG(PF) | FLAG(ZF) }, // #21 [ref=4x]
  { FLAG(AF) | FLAG(CF) | FLAG(PF) | FLAG(SF) | FLAG(ZF), 0 }, // #22 [ref=1x]
  { FLAG(DF), 0 }, // #23 [ref=3x]
  { 0, FLAG(AF) | FLAG(CF) | FLAG(DF) | FLAG(IF) | FLAG(OF) | FLAG(PF) | FLAG(SF) | FLAG(ZF) }, // #24 [ref=3x]
  { FLAG(AF) | FLAG(CF) | FLAG(DF) | FLAG(IF) | FLAG(OF) | FLAG(PF) | FLAG(SF) | FLAG(ZF), 0 }, // #25 [ref=3x]
  { FLAG(CF) | FLAG(OF), FLAG(CF) | FLAG(OF) }, // #26 [ref=2x]
  { 0, FLAG(CF) | FLAG(OF) }, // #27 [ref=2x]
  { 0, FLAG(AF) | FLAG(CF) | FLAG(PF) | FLAG(SF) | FLAG(ZF) }  // #28 [ref=1x]
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
  "cdq\0" "cdqe\0" "clac\0" "clc\0" "cld\0" "cldemote\0" "clflush\0" "clflushopt\0" "clgi\0" "cli\0" "clrssbsy\0"
  "clts\0" "clwb\0" "clzero\0" "cmc\0" "cmova\0" "cmovae\0" "cmovc\0" "cmovg\0" "cmovge\0" "cmovl\0" "cmovle\0"
  "cmovna\0" "cmovnae\0" "cmovnc\0" "cmovng\0" "cmovnge\0" "cmovnl\0" "cmovnle\0" "cmovno\0" "cmovnp\0" "cmovns\0"
  "cmovnz\0" "cmovo\0" "cmovp\0" "cmovpe\0" "cmovpo\0" "cmovs\0" "cmovz\0" "cmp\0" "cmps\0" "cmpxchg\0" "cmpxchg16b\0"
  "cmpxchg8b\0" "cpuid\0" "cqo\0" "crc32\0" "cvtpd2pi\0" "cvtpi2pd\0" "cvtpi2ps\0" "cvtps2pi\0" "cvttpd2pi\0"
  "cvttps2pi\0" "cwd\0" "cwde\0" "daa\0" "das\0" "endbr32\0" "endbr64\0" "enqcmd\0" "enqcmds\0" "f2xm1\0" "fabs\0"
  "faddp\0" "fbld\0" "fbstp\0" "fchs\0" "fclex\0" "fcmovb\0" "fcmovbe\0" "fcmove\0" "fcmovnb\0" "fcmovnbe\0"
  "fcmovne\0" "fcmovnu\0" "fcmovu\0" "fcom\0" "fcomi\0" "fcomip\0" "fcomp\0" "fcompp\0" "fcos\0" "fdecstp\0" "fdiv\0"
  "fdivp\0" "fdivr\0" "fdivrp\0" "femms\0" "ffree\0" "fiadd\0" "ficom\0" "ficomp\0" "fidiv\0" "fidivr\0" "fild\0"
  "fimul\0" "fincstp\0" "finit\0" "fist\0" "fistp\0" "fisttp\0" "fisub\0" "fisubr\0" "fld\0" "fld1\0" "fldcw\0"
  "fldenv\0" "fldl2e\0" "fldl2t\0" "fldlg2\0" "fldln2\0" "fldpi\0" "fldz\0" "fmulp\0" "fnclex\0" "fninit\0" "fnop\0"
  "fnsave\0" "fnstcw\0" "fnstenv\0" "fnstsw\0" "fpatan\0" "fprem\0" "fprem1\0" "fptan\0" "frndint\0" "frstor\0"
  "fsave\0" "fscale\0" "fsin\0" "fsincos\0" "fsqrt\0" "fst\0" "fstcw\0" "fstenv\0" "fstp\0" "fstsw\0" "fsubp\0"
  "fsubrp\0" "ftst\0" "fucom\0" "fucomi\0" "fucomip\0" "fucomp\0" "fucompp\0" "fwait\0" "fxam\0" "fxch\0" "fxrstor\0"
  "fxrstor64\0" "fxsave\0" "fxsave64\0" "fxtract\0" "fyl2x\0" "fyl2xp1\0" "getsec\0" "hlt\0" "inc\0" "incsspd\0"
  "incsspq\0" "insertq\0" "int3\0" "into\0" "invept\0" "invlpg\0" "invlpga\0" "invpcid\0" "invvpid\0" "iret\0"
  "iretd\0" "iretq\0" "iretw\0" "ja\0" "jae\0" "jb\0" "jbe\0" "jc\0" "je\0" "jecxz\0" "jg\0" "jge\0" "jl\0" "jle\0"
  "jmp\0" "jna\0" "jnae\0" "jnb\0" "jnbe\0" "jnc\0" "jne\0" "jng\0" "jnge\0" "jnl\0" "jnle\0" "jno\0" "jnp\0" "jns\0"
  "jnz\0" "jo\0" "jp\0" "jpe\0" "jpo\0" "js\0" "jz\0" "kaddb\0" "kaddd\0" "kaddq\0" "kaddw\0" "kandb\0" "kandd\0"
  "kandnb\0" "kandnd\0" "kandnq\0" "kandnw\0" "kandq\0" "kandw\0" "kmovb\0" "kmovw\0" "knotb\0" "knotd\0" "knotq\0"
  "knotw\0" "korb\0" "kord\0" "korq\0" "kortestb\0" "kortestd\0" "kortestq\0" "kortestw\0" "korw\0" "kshiftlb\0"
  "kshiftld\0" "kshiftlq\0" "kshiftlw\0" "kshiftrb\0" "kshiftrd\0" "kshiftrq\0" "kshiftrw\0" "ktestb\0" "ktestd\0"
  "ktestq\0" "ktestw\0" "kunpckbw\0" "kunpckdq\0" "kunpckwd\0" "kxnorb\0" "kxnord\0" "kxnorq\0" "kxnorw\0" "kxorb\0"
  "kxord\0" "kxorq\0" "kxorw\0" "lahf\0" "lar\0" "lds\0" "ldtilecfg\0" "lea\0" "leave\0" "les\0" "lfence\0" "lfs\0"
  "lgdt\0" "lgs\0" "lidt\0" "lldt\0" "llwpcb\0" "lmsw\0" "lods\0" "loop\0" "loope\0" "loopne\0" "lsl\0" "ltr\0"
  "lwpins\0" "lwpval\0" "lzcnt\0" "mcommit\0" "mfence\0" "monitorx\0" "movabs\0" "movdir64b\0" "movdiri\0" "movdq2q\0"
  "movnti\0" "movntq\0" "movntsd\0" "movntss\0" "movq2dq\0" "movsx\0" "movsxd\0" "movzx\0" "mulx\0" "mwaitx\0" "neg\0"
  "not\0" "out\0" "outs\0" "pavgusb\0" "pconfig\0" "pdep\0" "pext\0" "pf2id\0" "pf2iw\0" "pfacc\0" "pfadd\0"
  "pfcmpeq\0" "pfcmpge\0" "pfcmpgt\0" "pfmax\0" "pfmin\0" "pfmul\0" "pfnacc\0" "pfpnacc\0" "pfrcp\0" "pfrcpit1\0"
  "pfrcpit2\0" "pfrcpv\0" "pfrsqit1\0" "pfrsqrt\0" "pfrsqrtv\0" "pfsub\0" "pfsubr\0" "pi2fd\0" "pi2fw\0" "pmulhrw\0"
  "pop\0" "popa\0" "popad\0" "popcnt\0" "popf\0" "popfd\0" "popfq\0" "prefetch\0" "prefetchnta\0" "prefetcht0\0"
  "prefetcht1\0" "prefetcht2\0" "prefetchw\0" "prefetchwt1\0" "pshufw\0" "psmash\0" "pswapd\0" "ptwrite\0" "push\0"
  "pusha\0" "pushad\0" "pushf\0" "pushfd\0" "pushfq\0" "pvalidate\0" "rcl\0" "rcr\0" "rdfsbase\0" "rdgsbase\0"
  "rdmsr\0" "rdpid\0" "rdpkru\0" "rdpmc\0" "rdpru\0" "rdrand\0" "rdseed\0" "rdsspd\0" "rdsspq\0" "rdtsc\0" "rdtscp\0"
  "rmpadjust\0" "rmpupdate\0" "rol\0" "ror\0" "rorx\0" "rsm\0" "rstorssp\0" "sahf\0" "sal\0" "sar\0" "sarx\0"
  "saveprevssp\0" "sbb\0" "scas\0" "serialize\0" "seta\0" "setae\0" "setb\0" "setbe\0" "setc\0" "sete\0" "setg\0"
  "setge\0" "setl\0" "setle\0" "setna\0" "setnae\0" "setnb\0" "setnbe\0" "setnc\0" "setne\0" "setng\0" "setnge\0"
  "setnl\0" "setnle\0" "setno\0" "setnp\0" "setns\0" "setnz\0" "seto\0" "setp\0" "setpe\0" "setpo\0" "sets\0"
  "setssbsy\0" "setz\0" "sfence\0" "sgdt\0" "sha1msg1\0" "sha1msg2\0" "sha1nexte\0" "sha1rnds4\0" "sha256msg1\0"
  "sha256msg2\0" "sha256rnds2\0" "shl\0" "shlx\0" "shr\0" "shrd\0" "shrx\0" "sidt\0" "skinit\0" "sldt\0" "slwpcb\0"
  "smsw\0" "stac\0" "stc\0" "stgi\0" "sti\0" "stos\0" "str\0" "sttilecfg\0" "swapgs\0" "syscall\0" "sysenter\0"
  "sysexit\0" "sysexit64\0" "sysret\0" "sysret64\0" "t1mskc\0" "tdpbf16ps\0" "tdpbssd\0" "tdpbsud\0" "tdpbusd\0"
  "tdpbuud\0" "tileloadd\0" "tileloaddt1\0" "tilerelease\0" "tilestored\0" "tilezero\0" "tpause\0" "tzcnt\0" "tzmsk\0"
  "ud0\0" "ud1\0" "ud2\0" "umonitor\0" "umwait\0" "v4fmaddps\0" "v4fmaddss\0" "v4fnmaddps\0" "v4fnmaddss\0" "vaddpd\0"
  "vaddps\0" "vaddsd\0" "vaddss\0" "vaddsubpd\0" "vaddsubps\0" "vaesdec\0" "vaesdeclast\0" "vaesenc\0" "vaesenclast\0"
  "vaesimc\0" "vaeskeygenassist\0" "valignd\0" "valignq\0" "vandnpd\0" "vandnps\0" "vandpd\0" "vandps\0" "vblendmb\0"
  "vblendmd\0" "vblendmpd\0" "vblendmps\0" "vblendmq\0" "vblendmw\0" "vblendpd\0" "vblendps\0" "vblendvpd\0"
  "vblendvps\0" "vbroadcastf128\0" "vbroadcastf32x2\0" "vbroadcastf32x4\0" "vbroadcastf32x8\0" "vbroadcastf64x2\0"
  "vbroadcastf64x4\0" "vbroadcasti128\0" "vbroadcasti32x2\0" "vbroadcasti32x4\0" "vbroadcasti32x8\0"
  "vbroadcasti64x2\0" "vbroadcasti64x4\0" "vbroadcastsd\0" "vbroadcastss\0" "vcmppd\0" "vcmpps\0" "vcmpsd\0" "vcmpss\0"
  "vcomisd\0" "vcomiss\0" "vcompresspd\0" "vcompressps\0" "vcvtdq2pd\0" "vcvtdq2ps\0" "vcvtne2ps2bf16\0"
  "vcvtneps2bf16\0" "vcvtpd2dq\0" "vcvtpd2ps\0" "vcvtpd2qq\0" "vcvtpd2udq\0" "vcvtpd2uqq\0" "vcvtph2ps\0" "vcvtps2dq\0"
  "vcvtps2pd\0" "vcvtps2ph\0" "vcvtps2qq\0" "vcvtps2udq\0" "vcvtps2uqq\0" "vcvtqq2pd\0" "vcvtqq2ps\0" "vcvtsd2si\0"
  "vcvtsd2ss\0" "vcvtsd2usi\0" "vcvtsi2sd\0" "vcvtsi2ss\0" "vcvtss2sd\0" "vcvtss2si\0" "vcvtss2usi\0" "vcvttpd2dq\0"
  "vcvttpd2qq\0" "vcvttpd2udq\0" "vcvttpd2uqq\0" "vcvttps2dq\0" "vcvttps2qq\0" "vcvttps2udq\0" "vcvttps2uqq\0"
  "vcvttsd2si\0" "vcvttsd2usi\0" "vcvttss2si\0" "vcvttss2usi\0" "vcvtudq2pd\0" "vcvtudq2ps\0" "vcvtuqq2pd\0"
  "vcvtuqq2ps\0" "vcvtusi2sd\0" "vcvtusi2ss\0" "vdbpsadbw\0" "vdivpd\0" "vdivps\0" "vdivsd\0" "vdivss\0" "vdpbf16ps\0"
  "vdppd\0" "vdpps\0" "verr\0" "verw\0" "vexp2pd\0" "vexp2ps\0" "vexpandpd\0" "vexpandps\0" "vextractf128\0"
  "vextractf32x4\0" "vextractf32x8\0" "vextractf64x2\0" "vextractf64x4\0" "vextracti128\0" "vextracti32x4\0"
  "vextracti32x8\0" "vextracti64x2\0" "vextracti64x4\0" "vextractps\0" "vfixupimmpd\0" "vfixupimmps\0" "vfixupimmsd\0"
  "vfixupimmss\0" "vfmadd132pd\0" "vfmadd132ps\0" "vfmadd132sd\0" "vfmadd132ss\0" "vfmadd213pd\0" "vfmadd213ps\0"
  "vfmadd213sd\0" "vfmadd213ss\0" "vfmadd231pd\0" "vfmadd231ps\0" "vfmadd231sd\0" "vfmadd231ss\0" "vfmaddpd\0"
  "vfmaddps\0" "vfmaddsd\0" "vfmaddss\0" "vfmaddsub132pd\0" "vfmaddsub132ps\0" "vfmaddsub213pd\0" "vfmaddsub213ps\0"
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
  "vmxon\0" "vorpd\0" "vorps\0" "vp2intersectd\0" "vp2intersectq\0" "vp4dpwssd\0" "vp4dpwssds\0" "vpabsb\0" "vpabsd\0"
  "vpabsq\0" "vpabsw\0" "vpackssdw\0" "vpacksswb\0" "vpackusdw\0" "vpackuswb\0" "vpaddb\0" "vpaddd\0" "vpaddq\0"
  "vpaddsb\0" "vpaddsw\0" "vpaddusb\0" "vpaddusw\0" "vpaddw\0" "vpalignr\0" "vpand\0" "vpandd\0" "vpandn\0" "vpandnd\0"
  "vpandnq\0" "vpandq\0" "vpavgb\0" "vpavgw\0" "vpblendd\0" "vpblendvb\0" "vpblendw\0" "vpbroadcastb\0"
  "vpbroadcastd\0" "vpbroadcastmb2d\0" "vpbroadcastmb2q\0" "vpbroadcastq\0" "vpbroadcastw\0" "vpclmulqdq\0" "vpcmov\0"
  "vpcmpb\0" "vpcmpd\0" "vpcmpeqb\0" "vpcmpeqd\0" "vpcmpeqq\0" "vpcmpeqw\0" "vpcmpestri\0" "vpcmpestrm\0" "vpcmpgtb\0"
  "vpcmpgtd\0" "vpcmpgtq\0" "vpcmpgtw\0" "vpcmpistri\0" "vpcmpistrm\0" "vpcmpq\0" "vpcmpub\0" "vpcmpud\0" "vpcmpuq\0"
  "vpcmpuw\0" "vpcmpw\0" "vpcomb\0" "vpcomd\0" "vpcompressb\0" "vpcompressd\0" "vpcompressq\0" "vpcompressw\0"
  "vpcomq\0" "vpcomub\0" "vpcomud\0" "vpcomuq\0" "vpcomuw\0" "vpcomw\0" "vpconflictd\0" "vpconflictq\0" "vpdpbusd\0"
  "vpdpbusds\0" "vpdpwssd\0" "vpdpwssds\0" "vperm2f128\0" "vperm2i128\0" "vpermb\0" "vpermd\0" "vpermi2b\0"
  "vpermi2d\0" "vpermi2pd\0" "vpermi2ps\0" "vpermi2q\0" "vpermi2w\0" "vpermil2pd\0" "vpermil2ps\0" "vpermilpd\0"
  "vpermilps\0" "vpermpd\0" "vpermps\0" "vpermq\0" "vpermt2b\0" "vpermt2d\0" "vpermt2pd\0" "vpermt2ps\0" "vpermt2q\0"
  "vpermt2w\0" "vpermw\0" "vpexpandb\0" "vpexpandd\0" "vpexpandq\0" "vpexpandw\0" "vpextrb\0" "vpextrd\0" "vpextrq\0"
  "vpextrw\0" "vpgatherdd\0" "vpgatherdq\0" "vpgatherqd\0" "vpgatherqq\0" "vphaddbd\0" "vphaddbq\0" "vphaddbw\0"
  "vphaddd\0" "vphadddq\0" "vphaddsw\0" "vphaddubd\0" "vphaddubq\0" "vphaddubw\0" "vphaddudq\0" "vphadduwd\0"
  "vphadduwq\0" "vphaddw\0" "vphaddwd\0" "vphaddwq\0" "vphminposuw\0" "vphsubbw\0" "vphsubd\0" "vphsubdq\0"
  "vphsubsw\0" "vphsubw\0" "vphsubwd\0" "vpinsrb\0" "vpinsrd\0" "vpinsrq\0" "vpinsrw\0" "vplzcntd\0" "vplzcntq\0"
  "vpmacsdd\0" "vpmacsdqh\0" "vpmacsdql\0" "vpmacssdd\0" "vpmacssdqh\0" "vpmacssdql\0" "vpmacsswd\0" "vpmacssww\0"
  "vpmacswd\0" "vpmacsww\0" "vpmadcsswd\0" "vpmadcswd\0" "vpmadd52huq\0" "vpmadd52luq\0" "vpmaddubsw\0" "vpmaddwd\0"
  "vpmaskmovd\0" "vpmaskmovq\0" "vpmaxsb\0" "vpmaxsd\0" "vpmaxsq\0" "vpmaxsw\0" "vpmaxub\0" "vpmaxud\0" "vpmaxuq\0"
  "vpmaxuw\0" "vpminsb\0" "vpminsd\0" "vpminsq\0" "vpminsw\0" "vpminub\0" "vpminud\0" "vpminuq\0" "vpminuw\0"
  "vpmovb2m\0" "vpmovd2m\0" "vpmovdb\0" "vpmovdw\0" "vpmovm2b\0" "vpmovm2d\0" "vpmovm2q\0" "vpmovm2w\0" "vpmovmskb\0"
  "vpmovq2m\0" "vpmovqb\0" "vpmovqd\0" "vpmovqw\0" "vpmovsdb\0" "vpmovsdw\0" "vpmovsqb\0" "vpmovsqd\0" "vpmovsqw\0"
  "vpmovswb\0" "vpmovsxbd\0" "vpmovsxbq\0" "vpmovsxbw\0" "vpmovsxdq\0" "vpmovsxwd\0" "vpmovsxwq\0" "vpmovusdb\0"
  "vpmovusdw\0" "vpmovusqb\0" "vpmovusqd\0" "vpmovusqw\0" "vpmovuswb\0" "vpmovw2m\0" "vpmovwb\0" "vpmovzxbd\0"
  "vpmovzxbq\0" "vpmovzxbw\0" "vpmovzxdq\0" "vpmovzxwd\0" "vpmovzxwq\0" "vpmuldq\0" "vpmulhrsw\0" "vpmulhuw\0"
  "vpmulhw\0" "vpmulld\0" "vpmullq\0" "vpmullw\0" "vpmultishiftqb\0" "vpmuludq\0" "vpopcntb\0" "vpopcntd\0"
  "vpopcntq\0" "vpopcntw\0" "vpor\0" "vpord\0" "vporq\0" "vpperm\0" "vprold\0" "vprolq\0" "vprolvd\0" "vprolvq\0"
  "vprord\0" "vprorq\0" "vprorvd\0" "vprorvq\0" "vprotb\0" "vprotd\0" "vprotq\0" "vprotw\0" "vpsadbw\0" "vpscatterdd\0"
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
  "wbnoinvd\0" "wrfsbase\0" "wrgsbase\0" "wrmsr\0" "wrssd\0" "wrssq\0" "wrussd\0" "wrussq\0" "xabort\0" "xadd\0"
  "xbegin\0" "xend\0" "xgetbv\0" "xlatb\0" "xresldtrk\0" "xrstors\0" "xrstors64\0" "xsavec\0" "xsavec64\0" "xsaveopt\0"
  "xsaveopt64\0" "xsaves\0" "xsaves64\0" "xsetbv\0" "xsusldtrk\0" "xtest";

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
  { Inst::kIdRcl          , Inst::kIdRstorssp      + 1 },
  { Inst::kIdSahf         , Inst::kIdSysret64      + 1 },
  { Inst::kIdT1mskc       , Inst::kIdTzmsk         + 1 },
  { Inst::kIdUcomisd      , Inst::kIdUnpcklps      + 1 },
  { Inst::kIdV4fmaddps    , Inst::kIdVzeroupper    + 1 },
  { Inst::kIdWbinvd       , Inst::kIdWrussq        + 1 },
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
  ROW(2, 1, 1, 0, 18 , 19 , 0  , 0  , 0  , 0  ), //      {r8lo|r8hi|al, m8|mem}
  ROW(2, 1, 1, 0, 4  , 20 , 0  , 0  , 0  , 0  ), //      {r16, m16|mem|sreg}
  ROW(2, 1, 1, 0, 6  , 21 , 0  , 0  , 0  , 0  ), //      {r32, m32|mem|sreg}
  ROW(2, 1, 1, 0, 22 , 23 , 0  , 0  , 0  , 0  ), //      {m16|mem, sreg}
  ROW(2, 1, 1, 0, 23 , 22 , 0  , 0  , 0  , 0  ), //      {sreg, m16|mem}
  ROW(2, 1, 1, 0, 24 , 22 , 0  , 0  , 0  , 0  ), //      {ax, m16|mem}
  ROW(2, 1, 1, 0, 25 , 26 , 0  , 0  , 0  , 0  ), //      {eax, m32|mem}
  ROW(2, 0, 1, 0, 27 , 28 , 0  , 0  , 0  , 0  ), //      {rax, m64|mem}
  ROW(2, 1, 1, 0, 19 , 29 , 0  , 0  , 0  , 0  ), //      {m8|mem, al}
  ROW(2, 1, 1, 0, 22 , 24 , 0  , 0  , 0  , 0  ), //      {m16|mem, ax}
  ROW(2, 1, 1, 0, 26 , 25 , 0  , 0  , 0  , 0  ), //      {m32|mem, eax}
  ROW(2, 0, 1, 0, 28 , 27 , 0  , 0  , 0  , 0  ), //      {m64|mem, rax}
  ROW(2, 1, 0, 0, 6  , 30 , 0  , 0  , 0  , 0  ), //      {r32, creg|dreg}
  ROW(2, 1, 0, 0, 30 , 6  , 0  , 0  , 0  , 0  ), //      {creg|dreg, r32}
  ROW(2, 1, 1, 0, 9  , 10 , 0  , 0  , 0  , 0  ), // #23  {r8lo|r8hi|m8, i8|u8}
  ROW(2, 1, 1, 0, 11 , 12 , 0  , 0  , 0  , 0  ), //      {r16|m16, i16|u16}
  ROW(2, 1, 1, 0, 13 , 14 , 0  , 0  , 0  , 0  ), //      {r32|m32, i32|u32}
  ROW(2, 0, 1, 0, 15 , 31 , 0  , 0  , 0  , 0  ), //      {r64|m64|mem, i32|r64}
  ROW(2, 1, 1, 0, 32 , 33 , 0  , 0  , 0  , 0  ), //      {r16|m16|r32|m32|r64|m64|mem, i8}
  ROW(2, 1, 1, 0, 1  , 2  , 0  , 0  , 0  , 0  ), //      {r8lo|r8hi|m8|mem, r8lo|r8hi}
  ROW(2, 1, 1, 0, 34 , 4  , 0  , 0  , 0  , 0  ), //      {r16|m16|mem, r16}
  ROW(2, 1, 1, 0, 35 , 6  , 0  , 0  , 0  , 0  ), // #30  {r32|m32|mem, r32}
  ROW(2, 1, 1, 0, 2  , 19 , 0  , 0  , 0  , 0  ), //      {r8lo|r8hi, m8|mem}
  ROW(2, 1, 1, 0, 4  , 22 , 0  , 0  , 0  , 0  ), //      {r16, m16|mem}
  ROW(2, 1, 1, 0, 6  , 26 , 0  , 0  , 0  , 0  ), //      {r32, m32|mem}
  ROW(2, 0, 1, 0, 8  , 28 , 0  , 0  , 0  , 0  ), //      {r64, m64|mem}
  ROW(2, 1, 1, 0, 36 , 10 , 0  , 0  , 0  , 0  ), // #35  {r8lo|r8hi|m8|r16|m16|r32|m32|r64|m64|mem, i8|u8}
  ROW(2, 1, 1, 0, 11 , 12 , 0  , 0  , 0  , 0  ), //      {r16|m16, i16|u16}
  ROW(2, 1, 1, 0, 13 , 14 , 0  , 0  , 0  , 0  ), //      {r32|m32, i32|u32}
  ROW(2, 0, 1, 0, 8  , 37 , 0  , 0  , 0  , 0  ), //      {r64, u32|i32|r64|m64|mem}
  ROW(2, 0, 1, 0, 28 , 31 , 0  , 0  , 0  , 0  ), //      {m64|mem, i32|r64}
  ROW(2, 1, 1, 0, 1  , 2  , 0  , 0  , 0  , 0  ), //      {r8lo|r8hi|m8|mem, r8lo|r8hi}
  ROW(2, 1, 1, 0, 34 , 4  , 0  , 0  , 0  , 0  ), //      {r16|m16|mem, r16}
  ROW(2, 1, 1, 0, 35 , 6  , 0  , 0  , 0  , 0  ), //      {r32|m32|mem, r32}
  ROW(2, 1, 1, 0, 2  , 19 , 0  , 0  , 0  , 0  ), //      {r8lo|r8hi, m8|mem}
  ROW(2, 1, 1, 0, 4  , 22 , 0  , 0  , 0  , 0  ), //      {r16, m16|mem}
  ROW(2, 1, 1, 0, 6  , 26 , 0  , 0  , 0  , 0  ), //      {r32, m32|mem}
  ROW(2, 1, 1, 1, 38 , 1  , 0  , 0  , 0  , 0  ), // #46  {<ax>, r8lo|r8hi|m8|mem}
  ROW(3, 1, 1, 2, 39 , 38 , 34 , 0  , 0  , 0  ), //      {<dx>, <ax>, r16|m16|mem}
  ROW(3, 1, 1, 2, 40 , 41 , 35 , 0  , 0  , 0  ), //      {<edx>, <eax>, r32|m32|mem}
  ROW(3, 0, 1, 2, 42 , 43 , 15 , 0  , 0  , 0  ), //      {<rdx>, <rax>, r64|m64|mem}
  ROW(2, 1, 1, 0, 4  , 44 , 0  , 0  , 0  , 0  ), //      {r16, r16|m16|mem|i8|i16}
  ROW(2, 1, 1, 0, 6  , 45 , 0  , 0  , 0  , 0  ), //      {r32, r32|m32|mem|i8|i32}
  ROW(2, 0, 1, 0, 8  , 46 , 0  , 0  , 0  , 0  ), //      {r64, r64|m64|mem|i8|i32}
  ROW(3, 1, 1, 0, 4  , 34 , 47 , 0  , 0  , 0  ), //      {r16, r16|m16|mem, i8|i16|u16}
  ROW(3, 1, 1, 0, 6  , 35 , 48 , 0  , 0  , 0  ), //      {r32, r32|m32|mem, i8|i32|u32}
  ROW(3, 0, 1, 0, 8  , 15 , 49 , 0  , 0  , 0  ), //      {r64, r64|m64|mem, i8|i32}
  ROW(2, 0, 1, 0, 8  , 50 , 0  , 0  , 0  , 0  ), // #56  {r64, i64|u64}
  ROW(2, 0, 1, 0, 29 , 19 , 0  , 0  , 0  , 0  ), //      {al, m8|mem}
  ROW(2, 0, 1, 0, 24 , 22 , 0  , 0  , 0  , 0  ), //      {ax, m16|mem}
  ROW(2, 0, 1, 0, 25 , 26 , 0  , 0  , 0  , 0  ), //      {eax, m32|mem}
  ROW(2, 0, 1, 0, 27 , 28 , 0  , 0  , 0  , 0  ), //      {rax, m64|mem}
  ROW(2, 0, 1, 0, 19 , 29 , 0  , 0  , 0  , 0  ), //      {m8|mem, al}
  ROW(2, 0, 1, 0, 22 , 24 , 0  , 0  , 0  , 0  ), //      {m16|mem, ax}
  ROW(2, 0, 1, 0, 26 , 25 , 0  , 0  , 0  , 0  ), //      {m32|mem, eax}
  ROW(2, 0, 1, 0, 28 , 27 , 0  , 0  , 0  , 0  ), //      {m64|mem, rax}
  ROW(2, 1, 1, 0, 1  , 2  , 0  , 0  , 0  , 0  ), // #65  {r8lo|r8hi|m8|mem, r8lo|r8hi}
  ROW(2, 1, 1, 0, 34 , 4  , 0  , 0  , 0  , 0  ), //      {r16|m16|mem, r16}
  ROW(2, 1, 1, 0, 35 , 6  , 0  , 0  , 0  , 0  ), //      {r32|m32|mem, r32}
  ROW(2, 0, 1, 0, 15 , 8  , 0  , 0  , 0  , 0  ), // #68  {r64|m64|mem, r64}
  ROW(2, 1, 1, 0, 2  , 19 , 0  , 0  , 0  , 0  ), //      {r8lo|r8hi, m8|mem}
  ROW(2, 1, 1, 0, 4  , 22 , 0  , 0  , 0  , 0  ), //      {r16, m16|mem}
  ROW(2, 1, 1, 0, 6  , 26 , 0  , 0  , 0  , 0  ), //      {r32, m32|mem}
  ROW(2, 0, 1, 0, 8  , 28 , 0  , 0  , 0  , 0  ), //      {r64, m64|mem}
  ROW(2, 1, 1, 0, 9  , 10 , 0  , 0  , 0  , 0  ), // #73  {r8lo|r8hi|m8, i8|u8}
  ROW(2, 1, 1, 0, 11 , 12 , 0  , 0  , 0  , 0  ), //      {r16|m16, i16|u16}
  ROW(2, 1, 1, 0, 13 , 14 , 0  , 0  , 0  , 0  ), //      {r32|m32, i32|u32}
  ROW(2, 0, 1, 0, 15 , 31 , 0  , 0  , 0  , 0  ), //      {r64|m64|mem, i32|r64}
  ROW(2, 1, 1, 0, 1  , 2  , 0  , 0  , 0  , 0  ), //      {r8lo|r8hi|m8|mem, r8lo|r8hi}
  ROW(2, 1, 1, 0, 34 , 4  , 0  , 0  , 0  , 0  ), //      {r16|m16|mem, r16}
  ROW(2, 1, 1, 0, 35 , 6  , 0  , 0  , 0  , 0  ), //      {r32|m32|mem, r32}
  ROW(2, 1, 1, 0, 4  , 22 , 0  , 0  , 0  , 0  ), // #80  {r16, m16|mem}
  ROW(2, 1, 1, 0, 6  , 26 , 0  , 0  , 0  , 0  ), //      {r32, m32|mem}
  ROW(2, 0, 1, 0, 8  , 28 , 0  , 0  , 0  , 0  ), //      {r64, m64|mem}
  ROW(2, 1, 1, 0, 22 , 4  , 0  , 0  , 0  , 0  ), //      {m16|mem, r16}
  ROW(2, 1, 1, 0, 26 , 6  , 0  , 0  , 0  , 0  ), // #84  {m32|mem, r32}
  ROW(2, 0, 1, 0, 28 , 8  , 0  , 0  , 0  , 0  ), //      {m64|mem, r64}
  ROW(2, 1, 1, 0, 51 , 52 , 0  , 0  , 0  , 0  ), // #86  {xmm, xmm|m128|mem}
  ROW(2, 1, 1, 0, 53 , 51 , 0  , 0  , 0  , 0  ), // #87  {m128|mem, xmm}
  ROW(2, 1, 1, 0, 54 , 55 , 0  , 0  , 0  , 0  ), //      {ymm, ymm|m256|mem}
  ROW(2, 1, 1, 0, 56 , 54 , 0  , 0  , 0  , 0  ), //      {m256|mem, ymm}
  ROW(2, 1, 1, 0, 57 , 58 , 0  , 0  , 0  , 0  ), // #90  {zmm, zmm|m512|mem}
  ROW(2, 1, 1, 0, 59 , 57 , 0  , 0  , 0  , 0  ), //      {m512|mem, zmm}
  ROW(3, 1, 1, 0, 51 , 51 , 60 , 0  , 0  , 0  ), // #92  {xmm, xmm, xmm|m128|mem|i8|u8}
  ROW(3, 1, 1, 0, 51 , 53 , 10 , 0  , 0  , 0  ), //      {xmm, m128|mem, i8|u8}
  ROW(3, 1, 1, 0, 54 , 54 , 61 , 0  , 0  , 0  ), //      {ymm, ymm, ymm|m256|mem|i8|u8}
  ROW(3, 1, 1, 0, 54 , 56 , 10 , 0  , 0  , 0  ), //      {ymm, m256|mem, i8|u8}
  ROW(3, 1, 1, 0, 57 , 57 , 62 , 0  , 0  , 0  ), //      {zmm, zmm, zmm|m512|mem|i8|u8}
  ROW(3, 1, 1, 0, 57 , 59 , 10 , 0  , 0  , 0  ), //      {zmm, m512|mem, i8|u8}
  ROW(3, 1, 1, 0, 51 , 51 , 60 , 0  , 0  , 0  ), // #98  {xmm, xmm, i8|u8|xmm|m128|mem}
  ROW(3, 1, 1, 0, 54 , 54 , 60 , 0  , 0  , 0  ), //      {ymm, ymm, i8|u8|xmm|m128|mem}
  ROW(3, 1, 1, 0, 51 , 53 , 10 , 0  , 0  , 0  ), //      {xmm, m128|mem, i8|u8}
  ROW(3, 1, 1, 0, 54 , 56 , 10 , 0  , 0  , 0  ), //      {ymm, m256|mem, i8|u8}
  ROW(3, 1, 1, 0, 57 , 57 , 60 , 0  , 0  , 0  ), //      {zmm, zmm, xmm|m128|mem|i8|u8}
  ROW(3, 1, 1, 0, 57 , 59 , 10 , 0  , 0  , 0  ), //      {zmm, m512|mem, i8|u8}
  ROW(3, 1, 1, 0, 51 , 51 , 60 , 0  , 0  , 0  ), // #104 {xmm, xmm, xmm|m128|mem|i8|u8}
  ROW(3, 1, 1, 0, 51 , 53 , 10 , 0  , 0  , 0  ), //      {xmm, m128|mem, i8|u8}
  ROW(3, 1, 1, 0, 54 , 54 , 60 , 0  , 0  , 0  ), //      {ymm, ymm, xmm|m128|mem|i8|u8}
  ROW(3, 1, 1, 0, 54 , 56 , 10 , 0  , 0  , 0  ), //      {ymm, m256|mem, i8|u8}
  ROW(3, 1, 1, 0, 57 , 57 , 60 , 0  , 0  , 0  ), //      {zmm, zmm, xmm|m128|mem|i8|u8}
  ROW(3, 1, 1, 0, 57 , 59 , 10 , 0  , 0  , 0  ), //      {zmm, m512|mem, i8|u8}
  ROW(2, 1, 1, 0, 63 , 64 , 0  , 0  , 0  , 0  ), // #110 {mm, mm|m64|mem|r64}
  ROW(2, 1, 1, 0, 15 , 65 , 0  , 0  , 0  , 0  ), //      {m64|mem|r64, mm|xmm}
  ROW(2, 0, 1, 0, 51 , 15 , 0  , 0  , 0  , 0  ), //      {xmm, r64|m64|mem}
  ROW(2, 1, 1, 0, 51 , 66 , 0  , 0  , 0  , 0  ), // #113 {xmm, xmm|m64|mem}
  ROW(2, 1, 1, 0, 28 , 51 , 0  , 0  , 0  , 0  ), // #114 {m64|mem, xmm}
  ROW(0, 1, 1, 0, 0  , 0  , 0  , 0  , 0  , 0  ), // #115 {}
  ROW(1, 1, 1, 0, 67 , 0  , 0  , 0  , 0  , 0  ), //      {r16|m16|r32|m32|r64|m64}
  ROW(2, 1, 1, 0, 34 , 4  , 0  , 0  , 0  , 0  ), //      {r16|m16|mem, r16}
  ROW(2, 1, 1, 0, 35 , 6  , 0  , 0  , 0  , 0  ), //      {r32|m32|mem, r32}
  ROW(2, 1, 1, 0, 15 , 8  , 0  , 0  , 0  , 0  ), //      {r64|m64|mem, r64}
  ROW(3, 1, 1, 0, 51 , 68 , 51 , 0  , 0  , 0  ), // #120 {xmm, vm32x, xmm}
  ROW(3, 1, 1, 0, 54 , 68 , 54 , 0  , 0  , 0  ), //      {ymm, vm32x, ymm}
  ROW(2, 1, 1, 0, 51 , 68 , 0  , 0  , 0  , 0  ), //      {xmm, vm32x}
  ROW(2, 1, 1, 0, 54 , 69 , 0  , 0  , 0  , 0  ), //      {ymm, vm32y}
  ROW(2, 1, 1, 0, 57 , 70 , 0  , 0  , 0  , 0  ), //      {zmm, vm32z}
  ROW(3, 1, 1, 0, 51 , 68 , 51 , 0  , 0  , 0  ), // #125 {xmm, vm32x, xmm}
  ROW(3, 1, 1, 0, 54 , 69 , 54 , 0  , 0  , 0  ), //      {ymm, vm32y, ymm}
  ROW(2, 1, 1, 0, 51 , 68 , 0  , 0  , 0  , 0  ), //      {xmm, vm32x}
  ROW(2, 1, 1, 0, 54 , 69 , 0  , 0  , 0  , 0  ), //      {ymm, vm32y}
  ROW(2, 1, 1, 0, 57 , 70 , 0  , 0  , 0  , 0  ), //      {zmm, vm32z}
  ROW(3, 1, 1, 0, 51 , 71 , 51 , 0  , 0  , 0  ), // #130 {xmm, vm64x, xmm}
  ROW(3, 1, 1, 0, 54 , 72 , 54 , 0  , 0  , 0  ), //      {ymm, vm64y, ymm}
  ROW(2, 1, 1, 0, 51 , 71 , 0  , 0  , 0  , 0  ), //      {xmm, vm64x}
  ROW(2, 1, 1, 0, 54 , 72 , 0  , 0  , 0  , 0  ), //      {ymm, vm64y}
  ROW(2, 1, 1, 0, 57 , 73 , 0  , 0  , 0  , 0  ), //      {zmm, vm64z}
  ROW(2, 1, 1, 0, 32 , 10 , 0  , 0  , 0  , 0  ), // #135 {r16|m16|r32|m32|r64|m64|mem, i8|u8}
  ROW(2, 1, 1, 0, 34 , 4  , 0  , 0  , 0  , 0  ), //      {r16|m16|mem, r16}
  ROW(2, 1, 1, 0, 35 , 6  , 0  , 0  , 0  , 0  ), //      {r32|m32|mem, r32}
  ROW(2, 0, 1, 0, 15 , 8  , 0  , 0  , 0  , 0  ), //      {r64|m64|mem, r64}
  ROW(3, 1, 1, 1, 1  , 2  , 74 , 0  , 0  , 0  ), // #139 {r8lo|r8hi|m8|mem, r8lo|r8hi, <al>}
  ROW(3, 1, 1, 1, 34 , 4  , 38 , 0  , 0  , 0  ), //      {r16|m16|mem, r16, <ax>}
  ROW(3, 1, 1, 1, 35 , 6  , 41 , 0  , 0  , 0  ), //      {r32|m32|mem, r32, <eax>}
  ROW(3, 0, 1, 1, 15 , 8  , 43 , 0  , 0  , 0  ), //      {r64|m64|mem, r64, <rax>}
  ROW(1, 1, 1, 0, 75 , 0  , 0  , 0  , 0  , 0  ), // #143 {r16|m16|r64|m64}
  ROW(1, 1, 0, 0, 13 , 0  , 0  , 0  , 0  , 0  ), //      {r32|m32}
  ROW(1, 1, 0, 0, 76 , 0  , 0  , 0  , 0  , 0  ), //      {ds|es|ss}
  ROW(1, 1, 1, 0, 77 , 0  , 0  , 0  , 0  , 0  ), //      {fs|gs}
  ROW(1, 1, 1, 0, 78 , 0  , 0  , 0  , 0  , 0  ), // #147 {r16|m16|r64|m64|i8|i16|i32}
  ROW(1, 1, 0, 0, 79 , 0  , 0  , 0  , 0  , 0  ), //      {r32|m32|i32|u32}
  ROW(1, 1, 0, 0, 80 , 0  , 0  , 0  , 0  , 0  ), //      {cs|ss|ds|es}
  ROW(1, 1, 1, 0, 77 , 0  , 0  , 0  , 0  , 0  ), //      {fs|gs}
  ROW(4, 1, 1, 0, 51 , 51 , 51 , 52 , 0  , 0  ), // #151 {xmm, xmm, xmm, xmm|m128|mem}
  ROW(4, 1, 1, 0, 51 , 51 , 53 , 51 , 0  , 0  ), //      {xmm, xmm, m128|mem, xmm}
  ROW(4, 1, 1, 0, 54 , 54 , 54 , 55 , 0  , 0  ), //      {ymm, ymm, ymm, ymm|m256|mem}
  ROW(4, 1, 1, 0, 54 , 54 , 56 , 54 , 0  , 0  ), //      {ymm, ymm, m256|mem, ymm}
  ROW(3, 1, 1, 0, 51 , 81 , 51 , 0  , 0  , 0  ), // #155 {xmm, vm64x|vm64y, xmm}
  ROW(2, 1, 1, 0, 51 , 71 , 0  , 0  , 0  , 0  ), //      {xmm, vm64x}
  ROW(2, 1, 1, 0, 54 , 72 , 0  , 0  , 0  , 0  ), //      {ymm, vm64y}
  ROW(2, 1, 1, 0, 57 , 73 , 0  , 0  , 0  , 0  ), //      {zmm, vm64z}
  ROW(3, 1, 1, 0, 53 , 51 , 51 , 0  , 0  , 0  ), // #159 {m128|mem, xmm, xmm}
  ROW(3, 1, 1, 0, 56 , 54 , 54 , 0  , 0  , 0  ), //      {m256|mem, ymm, ymm}
  ROW(3, 1, 1, 0, 51 , 51 , 53 , 0  , 0  , 0  ), //      {xmm, xmm, m128|mem}
  ROW(3, 1, 1, 0, 54 , 54 , 56 , 0  , 0  , 0  ), //      {ymm, ymm, m256|mem}
  ROW(5, 1, 1, 0, 51 , 51 , 52 , 51 , 82 , 0  ), // #163 {xmm, xmm, xmm|m128|mem, xmm, i4|u4}
  ROW(5, 1, 1, 0, 51 , 51 , 51 , 53 , 82 , 0  ), //      {xmm, xmm, xmm, m128|mem, i4|u4}
  ROW(5, 1, 1, 0, 54 , 54 , 55 , 54 , 82 , 0  ), //      {ymm, ymm, ymm|m256|mem, ymm, i4|u4}
  ROW(5, 1, 1, 0, 54 , 54 , 54 , 56 , 82 , 0  ), //      {ymm, ymm, ymm, m256|mem, i4|u4}
  ROW(3, 1, 1, 0, 54 , 55 , 10 , 0  , 0  , 0  ), // #167 {ymm, ymm|m256|mem, i8|u8}
  ROW(3, 1, 1, 0, 54 , 54 , 55 , 0  , 0  , 0  ), //      {ymm, ymm, ymm|m256|mem}
  ROW(3, 1, 1, 0, 57 , 57 , 62 , 0  , 0  , 0  ), //      {zmm, zmm, zmm|m512|mem|i8|u8}
  ROW(3, 1, 1, 0, 57 , 59 , 10 , 0  , 0  , 0  ), //      {zmm, m512|mem, i8|u8}
  ROW(2, 1, 1, 0, 4  , 34 , 0  , 0  , 0  , 0  ), // #171 {r16, r16|m16|mem}
  ROW(2, 1, 1, 0, 6  , 35 , 0  , 0  , 0  , 0  ), // #172 {r32, r32|m32|mem}
  ROW(2, 0, 1, 0, 8  , 15 , 0  , 0  , 0  , 0  ), //      {r64, r64|m64|mem}
  ROW(1, 1, 1, 0, 83 , 0  , 0  , 0  , 0  , 0  ), // #174 {m32|m64}
  ROW(2, 1, 1, 0, 84 , 85 , 0  , 0  , 0  , 0  ), //      {st0, st}
  ROW(2, 1, 1, 0, 85 , 84 , 0  , 0  , 0  , 0  ), //      {st, st0}
  ROW(2, 1, 1, 0, 4  , 26 , 0  , 0  , 0  , 0  ), // #177 {r16, m32|mem}
  ROW(2, 1, 1, 0, 6  , 86 , 0  , 0  , 0  , 0  ), //      {r32, m48|mem}
  ROW(2, 0, 1, 0, 8  , 87 , 0  , 0  , 0  , 0  ), //      {r64, m80|mem}
  ROW(3, 1, 1, 0, 34 , 4  , 88 , 0  , 0  , 0  ), // #180 {r16|m16|mem, r16, cl|i8|u8}
  ROW(3, 1, 1, 0, 35 , 6  , 88 , 0  , 0  , 0  ), //      {r32|m32|mem, r32, cl|i8|u8}
  ROW(3, 0, 1, 0, 15 , 8  , 88 , 0  , 0  , 0  ), //      {r64|m64|mem, r64, cl|i8|u8}
  ROW(3, 1, 1, 0, 51 , 51 , 52 , 0  , 0  , 0  ), // #183 {xmm, xmm, xmm|m128|mem}
  ROW(3, 1, 1, 0, 54 , 54 , 55 , 0  , 0  , 0  ), // #184 {ymm, ymm, ymm|m256|mem}
  ROW(3, 1, 1, 0, 57 , 57 , 58 , 0  , 0  , 0  ), //      {zmm, zmm, zmm|m512|mem}
  ROW(4, 1, 1, 0, 51 , 51 , 52 , 10 , 0  , 0  ), // #186 {xmm, xmm, xmm|m128|mem, i8|u8}
  ROW(4, 1, 1, 0, 54 , 54 , 55 , 10 , 0  , 0  ), // #187 {ymm, ymm, ymm|m256|mem, i8|u8}
  ROW(4, 1, 1, 0, 57 , 57 , 58 , 10 , 0  , 0  ), //      {zmm, zmm, zmm|m512|mem, i8|u8}
  ROW(4, 1, 1, 0, 89 , 51 , 52 , 10 , 0  , 0  ), // #189 {xmm|k, xmm, xmm|m128|mem, i8|u8}
  ROW(4, 1, 1, 0, 90 , 54 , 55 , 10 , 0  , 0  ), //      {ymm|k, ymm, ymm|m256|mem, i8|u8}
  ROW(4, 1, 1, 0, 91 , 57 , 58 , 10 , 0  , 0  ), //      {k, zmm, zmm|m512|mem, i8|u8}
  ROW(2, 1, 1, 0, 52 , 51 , 0  , 0  , 0  , 0  ), // #192 {xmm|m128|mem, xmm}
  ROW(2, 1, 1, 0, 55 , 54 , 0  , 0  , 0  , 0  ), //      {ymm|m256|mem, ymm}
  ROW(2, 1, 1, 0, 58 , 57 , 0  , 0  , 0  , 0  ), //      {zmm|m512|mem, zmm}
  ROW(2, 1, 1, 0, 51 , 66 , 0  , 0  , 0  , 0  ), // #195 {xmm, xmm|m64|mem}
  ROW(2, 1, 1, 0, 54 , 52 , 0  , 0  , 0  , 0  ), //      {ymm, xmm|m128|mem}
  ROW(2, 1, 1, 0, 57 , 55 , 0  , 0  , 0  , 0  ), //      {zmm, ymm|m256|mem}
  ROW(2, 1, 1, 0, 51 , 52 , 0  , 0  , 0  , 0  ), // #198 {xmm, xmm|m128|mem}
  ROW(2, 1, 1, 0, 54 , 55 , 0  , 0  , 0  , 0  ), //      {ymm, ymm|m256|mem}
  ROW(2, 1, 1, 0, 57 , 58 , 0  , 0  , 0  , 0  ), //      {zmm, zmm|m512|mem}
  ROW(3, 1, 1, 0, 66 , 51 , 10 , 0  , 0  , 0  ), // #201 {xmm|m64|mem, xmm, i8|u8}
  ROW(3, 1, 1, 0, 52 , 54 , 10 , 0  , 0  , 0  ), // #202 {xmm|m128|mem, ymm, i8|u8}
  ROW(3, 1, 1, 0, 55 , 57 , 10 , 0  , 0  , 0  ), // #203 {ymm|m256|mem, zmm, i8|u8}
  ROW(3, 1, 1, 0, 51 , 52 , 10 , 0  , 0  , 0  ), // #204 {xmm, xmm|m128|mem, i8|u8}
  ROW(3, 1, 1, 0, 54 , 55 , 10 , 0  , 0  , 0  ), //      {ymm, ymm|m256|mem, i8|u8}
  ROW(3, 1, 1, 0, 57 , 58 , 10 , 0  , 0  , 0  ), //      {zmm, zmm|m512|mem, i8|u8}
  ROW(2, 1, 1, 0, 51 , 66 , 0  , 0  , 0  , 0  ), // #207 {xmm, xmm|m64|mem}
  ROW(2, 1, 1, 0, 54 , 55 , 0  , 0  , 0  , 0  ), //      {ymm, ymm|m256|mem}
  ROW(2, 1, 1, 0, 57 , 58 , 0  , 0  , 0  , 0  ), //      {zmm, zmm|m512|mem}
  ROW(2, 1, 1, 0, 53 , 51 , 0  , 0  , 0  , 0  ), // #210 {m128|mem, xmm}
  ROW(2, 1, 1, 0, 56 , 54 , 0  , 0  , 0  , 0  ), //      {m256|mem, ymm}
  ROW(2, 1, 1, 0, 59 , 57 , 0  , 0  , 0  , 0  ), //      {m512|mem, zmm}
  ROW(2, 1, 1, 0, 51 , 53 , 0  , 0  , 0  , 0  ), // #213 {xmm, m128|mem}
  ROW(2, 1, 1, 0, 54 , 56 , 0  , 0  , 0  , 0  ), //      {ymm, m256|mem}
  ROW(2, 1, 1, 0, 57 , 59 , 0  , 0  , 0  , 0  ), //      {zmm, m512|mem}
  ROW(2, 0, 1, 0, 15 , 51 , 0  , 0  , 0  , 0  ), // #216 {r64|m64|mem, xmm}
  ROW(2, 1, 1, 0, 51 , 92 , 0  , 0  , 0  , 0  ), //      {xmm, xmm|m64|mem|r64}
  ROW(2, 1, 1, 0, 28 , 51 , 0  , 0  , 0  , 0  ), //      {m64|mem, xmm}
  ROW(2, 1, 1, 0, 28 , 51 , 0  , 0  , 0  , 0  ), // #219 {m64|mem, xmm}
  ROW(2, 1, 1, 0, 51 , 28 , 0  , 0  , 0  , 0  ), //      {xmm, m64|mem}
  ROW(3, 1, 1, 0, 51 , 51 , 51 , 0  , 0  , 0  ), // #221 {xmm, xmm, xmm}
  ROW(2, 1, 1, 0, 26 , 51 , 0  , 0  , 0  , 0  ), // #222 {m32|mem, xmm}
  ROW(2, 1, 1, 0, 51 , 26 , 0  , 0  , 0  , 0  ), //      {xmm, m32|mem}
  ROW(3, 1, 1, 0, 51 , 51 , 51 , 0  , 0  , 0  ), //      {xmm, xmm, xmm}
  ROW(4, 1, 1, 0, 91 , 91 , 51 , 52 , 0  , 0  ), // #225 {k, k, xmm, xmm|m128|mem}
  ROW(4, 1, 1, 0, 91 , 91 , 54 , 55 , 0  , 0  ), //      {k, k, ymm, ymm|m256|mem}
  ROW(4, 1, 1, 0, 91 , 91 , 57 , 58 , 0  , 0  ), //      {k, k, zmm, zmm|m512|mem}
  ROW(2, 1, 1, 0, 93 , 92 , 0  , 0  , 0  , 0  ), // #228 {xmm|ymm, xmm|m64|mem|r64}
  ROW(2, 0, 1, 0, 57 , 8  , 0  , 0  , 0  , 0  ), //      {zmm, r64}
  ROW(2, 1, 1, 0, 57 , 66 , 0  , 0  , 0  , 0  ), //      {zmm, xmm|m64|mem}
  ROW(4, 1, 1, 0, 91 , 51 , 52 , 10 , 0  , 0  ), // #231 {k, xmm, xmm|m128|mem, i8|u8}
  ROW(4, 1, 1, 0, 91 , 54 , 55 , 10 , 0  , 0  ), //      {k, ymm, ymm|m256|mem, i8|u8}
  ROW(4, 1, 1, 0, 91 , 57 , 58 , 10 , 0  , 0  ), //      {k, zmm, zmm|m512|mem, i8|u8}
  ROW(3, 1, 1, 0, 89 , 51 , 52 , 0  , 0  , 0  ), // #234 {xmm|k, xmm, xmm|m128|mem}
  ROW(3, 1, 1, 0, 90 , 54 , 55 , 0  , 0  , 0  ), //      {ymm|k, ymm, ymm|m256|mem}
  ROW(3, 1, 1, 0, 91 , 57 , 58 , 0  , 0  , 0  ), //      {k, zmm, zmm|m512|mem}
  ROW(2, 1, 1, 0, 94 , 51 , 0  , 0  , 0  , 0  ), // #237 {xmm|m32|mem, xmm}
  ROW(2, 1, 1, 0, 66 , 54 , 0  , 0  , 0  , 0  ), //      {xmm|m64|mem, ymm}
  ROW(2, 1, 1, 0, 52 , 57 , 0  , 0  , 0  , 0  ), //      {xmm|m128|mem, zmm}
  ROW(2, 1, 1, 0, 66 , 51 , 0  , 0  , 0  , 0  ), // #240 {xmm|m64|mem, xmm}
  ROW(2, 1, 1, 0, 52 , 54 , 0  , 0  , 0  , 0  ), //      {xmm|m128|mem, ymm}
  ROW(2, 1, 1, 0, 55 , 57 , 0  , 0  , 0  , 0  ), //      {ymm|m256|mem, zmm}
  ROW(2, 1, 1, 0, 95 , 51 , 0  , 0  , 0  , 0  ), // #243 {xmm|m16|mem, xmm}
  ROW(2, 1, 1, 0, 94 , 54 , 0  , 0  , 0  , 0  ), //      {xmm|m32|mem, ymm}
  ROW(2, 1, 1, 0, 66 , 57 , 0  , 0  , 0  , 0  ), //      {xmm|m64|mem, zmm}
  ROW(2, 1, 1, 0, 51 , 94 , 0  , 0  , 0  , 0  ), // #246 {xmm, xmm|m32|mem}
  ROW(2, 1, 1, 0, 54 , 66 , 0  , 0  , 0  , 0  ), //      {ymm, xmm|m64|mem}
  ROW(2, 1, 1, 0, 57 , 52 , 0  , 0  , 0  , 0  ), //      {zmm, xmm|m128|mem}
  ROW(2, 1, 1, 0, 51 , 95 , 0  , 0  , 0  , 0  ), // #249 {xmm, xmm|m16|mem}
  ROW(2, 1, 1, 0, 54 , 94 , 0  , 0  , 0  , 0  ), //      {ymm, xmm|m32|mem}
  ROW(2, 1, 1, 0, 57 , 66 , 0  , 0  , 0  , 0  ), //      {zmm, xmm|m64|mem}
  ROW(2, 1, 1, 0, 68 , 51 , 0  , 0  , 0  , 0  ), // #252 {vm32x, xmm}
  ROW(2, 1, 1, 0, 69 , 54 , 0  , 0  , 0  , 0  ), //      {vm32y, ymm}
  ROW(2, 1, 1, 0, 70 , 57 , 0  , 0  , 0  , 0  ), //      {vm32z, zmm}
  ROW(2, 1, 1, 0, 71 , 51 , 0  , 0  , 0  , 0  ), // #255 {vm64x, xmm}
  ROW(2, 1, 1, 0, 72 , 54 , 0  , 0  , 0  , 0  ), //      {vm64y, ymm}
  ROW(2, 1, 1, 0, 73 , 57 , 0  , 0  , 0  , 0  ), //      {vm64z, zmm}
  ROW(3, 1, 1, 0, 91 , 51 , 52 , 0  , 0  , 0  ), // #258 {k, xmm, xmm|m128|mem}
  ROW(3, 1, 1, 0, 91 , 54 , 55 , 0  , 0  , 0  ), //      {k, ymm, ymm|m256|mem}
  ROW(3, 1, 1, 0, 91 , 57 , 58 , 0  , 0  , 0  ), //      {k, zmm, zmm|m512|mem}
  ROW(3, 1, 1, 0, 6  , 6  , 35 , 0  , 0  , 0  ), // #261 {r32, r32, r32|m32|mem}
  ROW(3, 0, 1, 0, 8  , 8  , 15 , 0  , 0  , 0  ), //      {r64, r64, r64|m64|mem}
  ROW(3, 1, 1, 0, 6  , 35 , 6  , 0  , 0  , 0  ), // #263 {r32, r32|m32|mem, r32}
  ROW(3, 0, 1, 0, 8  , 15 , 8  , 0  , 0  , 0  ), //      {r64, r64|m64|mem, r64}
  ROW(2, 1, 0, 0, 96 , 35 , 0  , 0  , 0  , 0  ), // #265 {bnd, r32|m32|mem}
  ROW(2, 0, 1, 0, 96 , 15 , 0  , 0  , 0  , 0  ), //      {bnd, r64|m64|mem}
  ROW(2, 1, 1, 0, 96 , 97 , 0  , 0  , 0  , 0  ), // #267 {bnd, bnd|mem}
  ROW(2, 1, 1, 0, 98 , 96 , 0  , 0  , 0  , 0  ), //      {mem, bnd}
  ROW(2, 1, 0, 0, 4  , 26 , 0  , 0  , 0  , 0  ), // #269 {r16, m32|mem}
  ROW(2, 1, 0, 0, 6  , 28 , 0  , 0  , 0  , 0  ), //      {r32, m64|mem}
  ROW(1, 1, 0, 0, 99 , 0  , 0  , 0  , 0  , 0  ), // #271 {rel16|r16|m16|r32|m32}
  ROW(1, 1, 1, 0, 100, 0  , 0  , 0  , 0  , 0  ), //      {rel32|r64|m64|mem}
  ROW(2, 1, 1, 0, 6  , 101, 0  , 0  , 0  , 0  ), // #273 {r32, r8lo|r8hi|m8|r16|m16|r32|m32}
  ROW(2, 0, 1, 0, 8  , 102, 0  , 0  , 0  , 0  ), //      {r64, r8lo|r8hi|m8|r64|m64}
  ROW(1, 1, 0, 0, 103, 0  , 0  , 0  , 0  , 0  ), // #275 {r16|r32}
  ROW(1, 1, 1, 0, 36 , 0  , 0  , 0  , 0  , 0  ), // #276 {r8lo|r8hi|m8|r16|m16|r32|m32|r64|m64|mem}
  ROW(2, 1, 0, 0, 104, 59 , 0  , 0  , 0  , 0  ), // #277 {es:[memBase], m512|mem}
  ROW(2, 0, 1, 0, 104, 59 , 0  , 0  , 0  , 0  ), //      {es:[memBase], m512|mem}
  ROW(3, 1, 1, 0, 51 , 10 , 10 , 0  , 0  , 0  ), // #279 {xmm, i8|u8, i8|u8}
  ROW(2, 1, 1, 0, 51 , 51 , 0  , 0  , 0  , 0  ), // #280 {xmm, xmm}
  ROW(0, 1, 1, 0, 0  , 0  , 0  , 0  , 0  , 0  ), // #281 {}
  ROW(1, 1, 1, 0, 85 , 0  , 0  , 0  , 0  , 0  ), // #282 {st}
  ROW(0, 1, 1, 0, 0  , 0  , 0  , 0  , 0  , 0  ), // #283 {}
  ROW(1, 1, 1, 0, 105, 0  , 0  , 0  , 0  , 0  ), // #284 {m32|m64|st}
  ROW(2, 1, 1, 0, 51 , 51 , 0  , 0  , 0  , 0  ), // #285 {xmm, xmm}
  ROW(4, 1, 1, 0, 51 , 51 , 10 , 10 , 0  , 0  ), //      {xmm, xmm, i8|u8, i8|u8}
  ROW(2, 1, 0, 0, 6  , 53 , 0  , 0  , 0  , 0  ), // #287 {r32, m128|mem}
  ROW(2, 0, 1, 0, 8  , 53 , 0  , 0  , 0  , 0  ), //      {r64, m128|mem}
  ROW(2, 1, 0, 2, 41 , 106, 0  , 0  , 0  , 0  ), // #289 {<eax>, <ecx>}
  ROW(2, 0, 1, 2, 107, 106, 0  , 0  , 0  , 0  ), //      {<eax|rax>, <ecx>}
  ROW(1, 1, 1, 0, 108, 0  , 0  , 0  , 0  , 0  ), // #291 {rel8|rel32}
  ROW(1, 1, 0, 0, 109, 0  , 0  , 0  , 0  , 0  ), //      {rel16}
  ROW(2, 1, 0, 1, 110, 111, 0  , 0  , 0  , 0  ), // #293 {<cx|ecx>, rel8}
  ROW(2, 0, 1, 1, 112, 111, 0  , 0  , 0  , 0  ), //      {<ecx|rcx>, rel8}
  ROW(1, 1, 1, 0, 113, 0  , 0  , 0  , 0  , 0  ), // #295 {rel8|rel32|r64|m64|mem}
  ROW(1, 1, 0, 0, 114, 0  , 0  , 0  , 0  , 0  ), //      {rel16|r32|m32|mem}
  ROW(2, 1, 1, 0, 91 , 115, 0  , 0  , 0  , 0  ), // #297 {k, k|m8|mem|r32|r8lo|r8hi|r16}
  ROW(2, 1, 1, 0, 116, 91 , 0  , 0  , 0  , 0  ), //      {m8|mem|r32|r8lo|r8hi|r16, k}
  ROW(2, 1, 1, 0, 91 , 117, 0  , 0  , 0  , 0  ), // #299 {k, k|m32|mem|r32}
  ROW(2, 1, 1, 0, 35 , 91 , 0  , 0  , 0  , 0  ), //      {m32|mem|r32, k}
  ROW(2, 1, 1, 0, 91 , 118, 0  , 0  , 0  , 0  ), // #301 {k, k|m64|mem|r64}
  ROW(2, 1, 1, 0, 15 , 91 , 0  , 0  , 0  , 0  ), //      {m64|mem|r64, k}
  ROW(2, 1, 1, 0, 91 , 119, 0  , 0  , 0  , 0  ), // #303 {k, k|m16|mem|r32|r16}
  ROW(2, 1, 1, 0, 120, 91 , 0  , 0  , 0  , 0  ), //      {m16|mem|r32|r16, k}
  ROW(2, 1, 1, 0, 4  , 34 , 0  , 0  , 0  , 0  ), // #305 {r16, r16|m16|mem}
  ROW(2, 1, 1, 0, 6  , 120, 0  , 0  , 0  , 0  ), //      {r32, r32|m16|mem|r16}
  ROW(2, 1, 0, 0, 4  , 26 , 0  , 0  , 0  , 0  ), // #307 {r16, m32|mem}
  ROW(2, 1, 0, 0, 6  , 86 , 0  , 0  , 0  , 0  ), //      {r32, m48|mem}
  ROW(2, 1, 1, 0, 4  , 34 , 0  , 0  , 0  , 0  ), // #309 {r16, r16|m16|mem}
  ROW(2, 1, 1, 0, 121, 120, 0  , 0  , 0  , 0  ), //      {r32|r64, r32|m16|mem|r16}
  ROW(2, 1, 1, 0, 65 , 35 , 0  , 0  , 0  , 0  ), // #311 {mm|xmm, r32|m32|mem}
  ROW(2, 1, 1, 0, 35 , 65 , 0  , 0  , 0  , 0  ), //      {r32|m32|mem, mm|xmm}
  ROW(2, 1, 1, 0, 51 , 94 , 0  , 0  , 0  , 0  ), // #313 {xmm, xmm|m32|mem}
  ROW(2, 1, 1, 0, 26 , 51 , 0  , 0  , 0  , 0  ), //      {m32|mem, xmm}
  ROW(2, 1, 1, 0, 4  , 9  , 0  , 0  , 0  , 0  ), // #315 {r16, r8lo|r8hi|m8}
  ROW(2, 1, 1, 0, 121, 122, 0  , 0  , 0  , 0  ), //      {r32|r64, r8lo|r8hi|m8|r16|m16}
  ROW(2, 0, 1, 0, 4  , 34 , 0  , 0  , 0  , 0  ), // #317 {r16, r16|m16|mem}
  ROW(2, 0, 1, 0, 121, 35 , 0  , 0  , 0  , 0  ), //      {r32|r64, r32|m32|mem}
  ROW(4, 1, 1, 1, 6  , 6  , 35 , 40 , 0  , 0  ), // #319 {r32, r32, r32|m32|mem, <edx>}
  ROW(4, 0, 1, 1, 8  , 8  , 15 , 42 , 0  , 0  ), //      {r64, r64, r64|m64|mem, <rdx>}
  ROW(2, 1, 1, 0, 63 , 123, 0  , 0  , 0  , 0  ), // #321 {mm, mm|m64|mem}
  ROW(2, 1, 1, 0, 51 , 52 , 0  , 0  , 0  , 0  ), //      {xmm, xmm|m128|mem}
  ROW(3, 1, 1, 0, 63 , 123, 10 , 0  , 0  , 0  ), // #323 {mm, mm|m64|mem, i8|u8}
  ROW(3, 1, 1, 0, 51 , 52 , 10 , 0  , 0  , 0  ), //      {xmm, xmm|m128|mem, i8|u8}
  ROW(3, 1, 1, 0, 6  , 65 , 10 , 0  , 0  , 0  ), // #325 {r32, mm|xmm, i8|u8}
  ROW(3, 1, 1, 0, 22 , 51 , 10 , 0  , 0  , 0  ), //      {m16|mem, xmm, i8|u8}
  ROW(2, 1, 1, 0, 63 , 124, 0  , 0  , 0  , 0  ), // #327 {mm, i8|u8|mm|m64|mem}
  ROW(2, 1, 1, 0, 51 , 60 , 0  , 0  , 0  , 0  ), //      {xmm, i8|u8|xmm|m128|mem}
  ROW(1, 1, 0, 0, 6  , 0  , 0  , 0  , 0  , 0  ), // #329 {r32}
  ROW(1, 0, 1, 0, 8  , 0  , 0  , 0  , 0  , 0  ), // #330 {r64}
  ROW(0, 1, 1, 0, 0  , 0  , 0  , 0  , 0  , 0  ), // #331 {}
  ROW(1, 1, 1, 0, 125, 0  , 0  , 0  , 0  , 0  ), //      {u16}
  ROW(3, 1, 1, 0, 6  , 35 , 10 , 0  , 0  , 0  ), // #333 {r32, r32|m32|mem, i8|u8}
  ROW(3, 0, 1, 0, 8  , 15 , 10 , 0  , 0  , 0  ), //      {r64, r64|m64|mem, i8|u8}
  ROW(4, 1, 1, 0, 51 , 51 , 52 , 51 , 0  , 0  ), // #335 {xmm, xmm, xmm|m128|mem, xmm}
  ROW(4, 1, 1, 0, 54 , 54 , 55 , 54 , 0  , 0  ), //      {ymm, ymm, ymm|m256|mem, ymm}
  ROW(2, 1, 1, 0, 51 , 126, 0  , 0  , 0  , 0  ), // #337 {xmm, xmm|m128|ymm|m256}
  ROW(2, 1, 1, 0, 54 , 58 , 0  , 0  , 0  , 0  ), //      {ymm, zmm|m512|mem}
  ROW(4, 1, 1, 0, 51 , 51 , 51 , 66 , 0  , 0  ), // #339 {xmm, xmm, xmm, xmm|m64|mem}
  ROW(4, 1, 1, 0, 51 , 51 , 28 , 51 , 0  , 0  ), //      {xmm, xmm, m64|mem, xmm}
  ROW(4, 1, 1, 0, 51 , 51 , 51 , 94 , 0  , 0  ), // #341 {xmm, xmm, xmm, xmm|m32|mem}
  ROW(4, 1, 1, 0, 51 , 51 , 26 , 51 , 0  , 0  ), //      {xmm, xmm, m32|mem, xmm}
  ROW(4, 1, 1, 0, 54 , 54 , 52 , 10 , 0  , 0  ), // #343 {ymm, ymm, xmm|m128|mem, i8|u8}
  ROW(4, 1, 1, 0, 57 , 57 , 52 , 10 , 0  , 0  ), //      {zmm, zmm, xmm|m128|mem, i8|u8}
  ROW(1, 1, 0, 1, 41 , 0  , 0  , 0  , 0  , 0  ), // #345 {<eax>}
  ROW(1, 0, 1, 1, 43 , 0  , 0  , 0  , 0  , 0  ), // #346 {<rax>}
  ROW(2, 1, 1, 0, 35 , 51 , 0  , 0  , 0  , 0  ), // #347 {r32|m32|mem, xmm}
  ROW(2, 1, 1, 0, 51 , 35 , 0  , 0  , 0  , 0  ), //      {xmm, r32|m32|mem}
  ROW(2, 1, 1, 0, 28 , 51 , 0  , 0  , 0  , 0  ), // #349 {m64|mem, xmm}
  ROW(3, 1, 1, 0, 51 , 51 , 28 , 0  , 0  , 0  ), //      {xmm, xmm, m64|mem}
  ROW(2, 1, 0, 0, 35 , 6  , 0  , 0  , 0  , 0  ), // #351 {r32|m32|mem, r32}
  ROW(2, 0, 1, 0, 15 , 8  , 0  , 0  , 0  , 0  ), //      {r64|m64|mem, r64}
  ROW(2, 1, 0, 0, 6  , 35 , 0  , 0  , 0  , 0  ), // #353 {r32, r32|m32|mem}
  ROW(2, 0, 1, 0, 8  , 15 , 0  , 0  , 0  , 0  ), //      {r64, r64|m64|mem}
  ROW(3, 1, 1, 0, 51 , 51 , 60 , 0  , 0  , 0  ), // #355 {xmm, xmm, xmm|m128|mem|i8|u8}
  ROW(3, 1, 1, 0, 51 , 53 , 127, 0  , 0  , 0  ), //      {xmm, m128|mem, i8|u8|xmm}
  ROW(2, 1, 1, 0, 81 , 51 , 0  , 0  , 0  , 0  ), // #357 {vm64x|vm64y, xmm}
  ROW(2, 1, 1, 0, 73 , 54 , 0  , 0  , 0  , 0  ), //      {vm64z, ymm}
  ROW(3, 1, 1, 0, 51 , 51 , 52 , 0  , 0  , 0  ), // #359 {xmm, xmm, xmm|m128|mem}
  ROW(3, 1, 1, 0, 51 , 53 , 51 , 0  , 0  , 0  ), //      {xmm, m128|mem, xmm}
  ROW(2, 1, 1, 0, 68 , 93 , 0  , 0  , 0  , 0  ), // #361 {vm32x, xmm|ymm}
  ROW(2, 1, 1, 0, 69 , 57 , 0  , 0  , 0  , 0  ), //      {vm32y, zmm}
  ROW(1, 1, 0, 1, 38 , 0  , 0  , 0  , 0  , 0  ), // #363 {<ax>}
  ROW(2, 1, 0, 1, 38 , 10 , 0  , 0  , 0  , 0  ), // #364 {<ax>, i8|u8}
  ROW(2, 1, 0, 0, 34 , 4  , 0  , 0  , 0  , 0  ), // #365 {r16|m16|mem, r16}
  ROW(3, 1, 1, 1, 51 , 52 , 128, 0  , 0  , 0  ), // #366 {xmm, xmm|m128|mem, <xmm0>}
  ROW(2, 1, 1, 0, 96 , 129, 0  , 0  , 0  , 0  ), // #367 {bnd, mib}
  ROW(2, 1, 1, 0, 96 , 98 , 0  , 0  , 0  , 0  ), // #368 {bnd, mem}
  ROW(2, 1, 1, 0, 129, 96 , 0  , 0  , 0  , 0  ), // #369 {mib, bnd}
  ROW(1, 1, 1, 0, 130, 0  , 0  , 0  , 0  , 0  ), // #370 {r16|r32|r64}
  ROW(1, 1, 1, 1, 38 , 0  , 0  , 0  , 0  , 0  ), // #371 {<ax>}
  ROW(2, 1, 1, 2, 40 , 41 , 0  , 0  , 0  , 0  ), // #372 {<edx>, <eax>}
  ROW(1, 1, 1, 0, 98 , 0  , 0  , 0  , 0  , 0  ), // #373 {mem}
  ROW(1, 1, 1, 0, 28 , 0  , 0  , 0  , 0  , 0  ), // #374 {m64|mem}
  ROW(1, 1, 1, 1, 131, 0  , 0  , 0  , 0  , 0  ), // #375 {<ds:[memBase|zax]>}
  ROW(2, 1, 1, 2, 132, 133, 0  , 0  , 0  , 0  ), // #376 {<ds:[memBase|zsi]>, <es:[memBase|zdi]>}
  ROW(3, 1, 1, 0, 51 , 66 , 10 , 0  , 0  , 0  ), // #377 {xmm, xmm|m64|mem, i8|u8}
  ROW(3, 1, 1, 0, 51 , 94 , 10 , 0  , 0  , 0  ), // #378 {xmm, xmm|m32|mem, i8|u8}
  ROW(5, 0, 1, 4, 53 , 42 , 43 , 134, 135, 0  ), // #379 {m128|mem, <rdx>, <rax>, <rcx>, <rbx>}
  ROW(5, 1, 1, 4, 28 , 40 , 41 , 106, 136, 0  ), // #380 {m64|mem, <edx>, <eax>, <ecx>, <ebx>}
  ROW(4, 1, 1, 4, 41 , 136, 106, 40 , 0  , 0  ), // #381 {<eax>, <ebx>, <ecx>, <edx>}
  ROW(2, 0, 1, 2, 42 , 43 , 0  , 0  , 0  , 0  ), // #382 {<rdx>, <rax>}
  ROW(2, 1, 1, 0, 63 , 52 , 0  , 0  , 0  , 0  ), // #383 {mm, xmm|m128|mem}
  ROW(2, 1, 1, 0, 51 , 123, 0  , 0  , 0  , 0  ), // #384 {xmm, mm|m64|mem}
  ROW(2, 1, 1, 0, 63 , 66 , 0  , 0  , 0  , 0  ), // #385 {mm, xmm|m64|mem}
  ROW(2, 1, 1, 0, 121, 66 , 0  , 0  , 0  , 0  ), // #386 {r32|r64, xmm|m64|mem}
  ROW(2, 1, 1, 0, 51 , 137, 0  , 0  , 0  , 0  ), // #387 {xmm, r32|m32|mem|r64|m64}
  ROW(2, 1, 1, 0, 121, 94 , 0  , 0  , 0  , 0  ), // #388 {r32|r64, xmm|m32|mem}
  ROW(2, 1, 1, 2, 39 , 38 , 0  , 0  , 0  , 0  ), // #389 {<dx>, <ax>}
  ROW(1, 1, 1, 1, 41 , 0  , 0  , 0  , 0  , 0  ), // #390 {<eax>}
  ROW(2, 1, 1, 0, 12 , 10 , 0  , 0  , 0  , 0  ), // #391 {i16|u16, i8|u8}
  ROW(3, 1, 1, 0, 35 , 51 , 10 , 0  , 0  , 0  ), // #392 {r32|m32|mem, xmm, i8|u8}
  ROW(1, 1, 1, 0, 87 , 0  , 0  , 0  , 0  , 0  ), // #393 {m80|mem}
  ROW(1, 1, 1, 0, 138, 0  , 0  , 0  , 0  , 0  ), // #394 {m16|m32}
  ROW(1, 1, 1, 0, 139, 0  , 0  , 0  , 0  , 0  ), // #395 {m16|m32|m64}
  ROW(1, 1, 1, 0, 140, 0  , 0  , 0  , 0  , 0  ), // #396 {m32|m64|m80|st}
  ROW(1, 1, 1, 0, 22 , 0  , 0  , 0  , 0  , 0  ), // #397 {m16|mem}
  ROW(1, 1, 1, 0, 141, 0  , 0  , 0  , 0  , 0  ), // #398 {ax|m16|mem}
  ROW(1, 0, 1, 0, 98 , 0  , 0  , 0  , 0  , 0  ), // #399 {mem}
  ROW(2, 1, 1, 0, 142, 143, 0  , 0  , 0  , 0  ), // #400 {al|ax|eax, i8|u8|dx}
  ROW(1, 1, 1, 0, 6  , 0  , 0  , 0  , 0  , 0  ), // #401 {r32}
  ROW(2, 1, 1, 0, 144, 145, 0  , 0  , 0  , 0  ), // #402 {es:[memBase|zdi], dx}
  ROW(1, 1, 1, 0, 10 , 0  , 0  , 0  , 0  , 0  ), // #403 {i8|u8}
  ROW(0, 1, 0, 0, 0  , 0  , 0  , 0  , 0  , 0  ), // #404 {}
  ROW(0, 0, 1, 0, 0  , 0  , 0  , 0  , 0  , 0  ), // #405 {}
  ROW(3, 1, 1, 0, 91 , 91 , 91 , 0  , 0  , 0  ), // #406 {k, k, k}
  ROW(2, 1, 1, 0, 91 , 91 , 0  , 0  , 0  , 0  ), // #407 {k, k}
  ROW(3, 1, 1, 0, 91 , 91 , 10 , 0  , 0  , 0  ), // #408 {k, k, i8|u8}
  ROW(1, 1, 1, 1, 146, 0  , 0  , 0  , 0  , 0  ), // #409 {<ah>}
  ROW(1, 1, 1, 0, 26 , 0  , 0  , 0  , 0  , 0  ), // #410 {m32|mem}
  ROW(1, 0, 1, 0, 59 , 0  , 0  , 0  , 0  , 0  ), // #411 {m512|mem}
  ROW(2, 1, 1, 0, 130, 147, 0  , 0  , 0  , 0  ), // #412 {r16|r32|r64, mem|m8|m16|m32|m48|m64|m80|m128|m256|m512|m1024}
  ROW(1, 1, 1, 0, 34 , 0  , 0  , 0  , 0  , 0  ), // #413 {r16|m16|mem}
  ROW(1, 1, 1, 0, 121, 0  , 0  , 0  , 0  , 0  ), // #414 {r32|r64}
  ROW(2, 1, 1, 2, 148, 132, 0  , 0  , 0  , 0  ), // #415 {<al|ax|eax|rax>, <ds:[memBase|zsi]>}
  ROW(3, 1, 1, 0, 121, 35 , 14 , 0  , 0  , 0  ), // #416 {r32|r64, r32|m32|mem, i32|u32}
  ROW(3, 1, 1, 1, 51 , 51 , 149, 0  , 0  , 0  ), // #417 {xmm, xmm, <ds:[memBase|zdi]>}
  ROW(3, 1, 1, 1, 63 , 63 , 149, 0  , 0  , 0  ), // #418 {mm, mm, <ds:[memBase|zdi]>}
  ROW(3, 1, 1, 3, 131, 106, 40 , 0  , 0  , 0  ), // #419 {<ds:[memBase|zax]>, <ecx>, <edx>}
  ROW(2, 1, 1, 0, 104, 59 , 0  , 0  , 0  , 0  ), // #420 {es:[memBase], m512|mem}
  ROW(2, 1, 1, 0, 63 , 51 , 0  , 0  , 0  , 0  ), // #421 {mm, xmm}
  ROW(2, 1, 1, 0, 6  , 51 , 0  , 0  , 0  , 0  ), // #422 {r32, xmm}
  ROW(2, 1, 1, 0, 28 , 63 , 0  , 0  , 0  , 0  ), // #423 {m64|mem, mm}
  ROW(2, 1, 1, 0, 51 , 63 , 0  , 0  , 0  , 0  ), // #424 {xmm, mm}
  ROW(2, 1, 1, 2, 133, 132, 0  , 0  , 0  , 0  ), // #425 {<es:[memBase|zdi]>, <ds:[memBase|zsi]>}
  ROW(2, 1, 1, 2, 41 , 106, 0  , 0  , 0  , 0  ), // #426 {<eax>, <ecx>}
  ROW(3, 1, 1, 3, 41 , 106, 136, 0  , 0  , 0  ), // #427 {<eax>, <ecx>, <ebx>}
  ROW(2, 1, 1, 0, 150, 142, 0  , 0  , 0  , 0  ), // #428 {u8|dx, al|ax|eax}
  ROW(2, 1, 1, 0, 145, 151, 0  , 0  , 0  , 0  ), // #429 {dx, ds:[memBase|zsi]}
  ROW(6, 1, 1, 3, 51 , 52 , 10 , 106, 41 , 40 ), // #430 {xmm, xmm|m128|mem, i8|u8, <ecx>, <eax>, <edx>}
  ROW(6, 1, 1, 3, 51 , 52 , 10 , 128, 41 , 40 ), // #431 {xmm, xmm|m128|mem, i8|u8, <xmm0>, <eax>, <edx>}
  ROW(4, 1, 1, 1, 51 , 52 , 10 , 106, 0  , 0  ), // #432 {xmm, xmm|m128|mem, i8|u8, <ecx>}
  ROW(4, 1, 1, 1, 51 , 52 , 10 , 128, 0  , 0  ), // #433 {xmm, xmm|m128|mem, i8|u8, <xmm0>}
  ROW(3, 1, 1, 0, 116, 51 , 10 , 0  , 0  , 0  ), // #434 {r32|m8|mem|r8lo|r8hi|r16, xmm, i8|u8}
  ROW(3, 0, 1, 0, 15 , 51 , 10 , 0  , 0  , 0  ), // #435 {r64|m64|mem, xmm, i8|u8}
  ROW(3, 1, 1, 0, 51 , 116, 10 , 0  , 0  , 0  ), // #436 {xmm, r32|m8|mem|r8lo|r8hi|r16, i8|u8}
  ROW(3, 1, 1, 0, 51 , 35 , 10 , 0  , 0  , 0  ), // #437 {xmm, r32|m32|mem, i8|u8}
  ROW(3, 0, 1, 0, 51 , 15 , 10 , 0  , 0  , 0  ), // #438 {xmm, r64|m64|mem, i8|u8}
  ROW(3, 1, 1, 0, 65 , 120, 10 , 0  , 0  , 0  ), // #439 {mm|xmm, r32|m16|mem|r16, i8|u8}
  ROW(2, 1, 1, 0, 6  , 65 , 0  , 0  , 0  , 0  ), // #440 {r32, mm|xmm}
  ROW(2, 1, 1, 0, 51 , 10 , 0  , 0  , 0  , 0  ), // #441 {xmm, i8|u8}
  ROW(1, 1, 1, 0, 137, 0  , 0  , 0  , 0  , 0  ), // #442 {r32|m32|mem|r64|m64}
  ROW(2, 1, 1, 0, 36 , 88 , 0  , 0  , 0  , 0  ), // #443 {r8lo|r8hi|m8|r16|m16|r32|m32|r64|m64|mem, cl|i8|u8}
  ROW(1, 0, 1, 0, 121, 0  , 0  , 0  , 0  , 0  ), // #444 {r32|r64}
  ROW(3, 1, 1, 3, 40 , 41 , 106, 0  , 0  , 0  ), // #445 {<edx>, <eax>, <ecx>}
  ROW(2, 1, 1, 2, 148, 133, 0  , 0  , 0  , 0  ), // #446 {<al|ax|eax|rax>, <es:[memBase|zdi]>}
  ROW(1, 1, 1, 0, 1  , 0  , 0  , 0  , 0  , 0  ), // #447 {r8lo|r8hi|m8|mem}
  ROW(1, 1, 1, 0, 152, 0  , 0  , 0  , 0  , 0  ), // #448 {r16|m16|mem|r32|r64}
  ROW(2, 1, 1, 2, 133, 148, 0  , 0  , 0  , 0  ), // #449 {<es:[memBase|zdi]>, <al|ax|eax|rax>}
  ROW(3, 0, 1, 0, 153, 153, 153, 0  , 0  , 0  ), // #450 {tmm, tmm, tmm}
  ROW(2, 0, 1, 0, 153, 98 , 0  , 0  , 0  , 0  ), // #451 {tmm, tmem}
  ROW(2, 0, 1, 0, 98 , 153, 0  , 0  , 0  , 0  ), // #452 {tmem, tmm}
  ROW(1, 0, 1, 0, 153, 0  , 0  , 0  , 0  , 0  ), // #453 {tmm}
  ROW(3, 1, 1, 2, 6  , 40 , 41 , 0  , 0  , 0  ), // #454 {r32, <edx>, <eax>}
  ROW(1, 1, 1, 0, 35 , 0  , 0  , 0  , 0  , 0  ), // #455 {r32|m32|mem}
  ROW(1, 1, 1, 0, 154, 0  , 0  , 0  , 0  , 0  ), // #456 {ds:[memBase]}
  ROW(6, 1, 1, 0, 57 , 57 , 57 , 57 , 57 , 53 ), // #457 {zmm, zmm, zmm, zmm, zmm, m128|mem}
  ROW(6, 1, 1, 0, 51 , 51 , 51 , 51 , 51 , 53 ), // #458 {xmm, xmm, xmm, xmm, xmm, m128|mem}
  ROW(3, 1, 1, 0, 51 , 51 , 66 , 0  , 0  , 0  ), // #459 {xmm, xmm, xmm|m64|mem}
  ROW(3, 1, 1, 0, 51 , 51 , 94 , 0  , 0  , 0  ), // #460 {xmm, xmm, xmm|m32|mem}
  ROW(2, 1, 1, 0, 54 , 53 , 0  , 0  , 0  , 0  ), // #461 {ymm, m128|mem}
  ROW(2, 1, 1, 0, 155, 66 , 0  , 0  , 0  , 0  ), // #462 {ymm|zmm, xmm|m64|mem}
  ROW(2, 1, 1, 0, 155, 53 , 0  , 0  , 0  , 0  ), // #463 {ymm|zmm, m128|mem}
  ROW(2, 1, 1, 0, 57 , 56 , 0  , 0  , 0  , 0  ), // #464 {zmm, m256|mem}
  ROW(2, 1, 1, 0, 156, 66 , 0  , 0  , 0  , 0  ), // #465 {xmm|ymm|zmm, xmm|m64|mem}
  ROW(2, 1, 1, 0, 156, 94 , 0  , 0  , 0  , 0  ), // #466 {xmm|ymm|zmm, m32|mem|xmm}
  ROW(4, 1, 1, 0, 89 , 51 , 66 , 10 , 0  , 0  ), // #467 {xmm|k, xmm, xmm|m64|mem, i8|u8}
  ROW(4, 1, 1, 0, 89 , 51 , 94 , 10 , 0  , 0  ), // #468 {xmm|k, xmm, xmm|m32|mem, i8|u8}
  ROW(3, 1, 1, 0, 51 , 51 , 137, 0  , 0  , 0  ), // #469 {xmm, xmm, r32|m32|mem|r64|m64}
  ROW(3, 1, 1, 0, 52 , 155, 10 , 0  , 0  , 0  ), // #470 {xmm|m128|mem, ymm|zmm, i8|u8}
  ROW(4, 1, 1, 0, 51 , 51 , 66 , 10 , 0  , 0  ), // #471 {xmm, xmm, xmm|m64|mem, i8|u8}
  ROW(4, 1, 1, 0, 51 , 51 , 94 , 10 , 0  , 0  ), // #472 {xmm, xmm, xmm|m32|mem, i8|u8}
  ROW(3, 1, 1, 0, 91 , 157, 10 , 0  , 0  , 0  ), // #473 {k, xmm|m128|ymm|m256|zmm|m512, i8|u8}
  ROW(3, 1, 1, 0, 91 , 66 , 10 , 0  , 0  , 0  ), // #474 {k, xmm|m64|mem, i8|u8}
  ROW(3, 1, 1, 0, 91 , 94 , 10 , 0  , 0  , 0  ), // #475 {k, xmm|m32|mem, i8|u8}
  ROW(1, 1, 1, 0, 69 , 0  , 0  , 0  , 0  , 0  ), // #476 {vm32y}
  ROW(1, 1, 1, 0, 70 , 0  , 0  , 0  , 0  , 0  ), // #477 {vm32z}
  ROW(1, 1, 1, 0, 73 , 0  , 0  , 0  , 0  , 0  ), // #478 {vm64z}
  ROW(4, 1, 1, 0, 57 , 57 , 55 , 10 , 0  , 0  ), // #479 {zmm, zmm, ymm|m256|mem, i8|u8}
  ROW(2, 1, 1, 0, 6  , 93 , 0  , 0  , 0  , 0  ), // #480 {r32, xmm|ymm}
  ROW(2, 1, 1, 0, 156, 158, 0  , 0  , 0  , 0  ), // #481 {xmm|ymm|zmm, xmm|m8|mem|r32|r8lo|r8hi|r16}
  ROW(2, 1, 1, 0, 156, 159, 0  , 0  , 0  , 0  ), // #482 {xmm|ymm|zmm, xmm|m32|mem|r32}
  ROW(2, 1, 1, 0, 156, 91 , 0  , 0  , 0  , 0  ), // #483 {xmm|ymm|zmm, k}
  ROW(2, 1, 1, 0, 156, 160, 0  , 0  , 0  , 0  ), // #484 {xmm|ymm|zmm, xmm|m16|mem|r32|r16}
  ROW(3, 1, 1, 0, 120, 51 , 10 , 0  , 0  , 0  ), // #485 {r32|m16|mem|r16, xmm, i8|u8}
  ROW(4, 1, 1, 0, 51 , 51 , 116, 10 , 0  , 0  ), // #486 {xmm, xmm, r32|m8|mem|r8lo|r8hi|r16, i8|u8}
  ROW(4, 1, 1, 0, 51 , 51 , 35 , 10 , 0  , 0  ), // #487 {xmm, xmm, r32|m32|mem, i8|u8}
  ROW(4, 0, 1, 0, 51 , 51 , 15 , 10 , 0  , 0  ), // #488 {xmm, xmm, r64|m64|mem, i8|u8}
  ROW(4, 1, 1, 0, 51 , 51 , 120, 10 , 0  , 0  ), // #489 {xmm, xmm, r32|m16|mem|r16, i8|u8}
  ROW(2, 1, 1, 0, 91 , 156, 0  , 0  , 0  , 0  ), // #490 {k, xmm|ymm|zmm}
  ROW(1, 1, 1, 0, 109, 0  , 0  , 0  , 0  , 0  ), // #491 {rel16|rel32}
  ROW(3, 1, 1, 2, 98 , 40 , 41 , 0  , 0  , 0  ), // #492 {mem, <edx>, <eax>}
  ROW(3, 0, 1, 2, 98 , 40 , 41 , 0  , 0  , 0  )  // #493 {mem, <edx>, <eax>}
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
  ROW(F(GpbLo) | F(GpbHi), 0, 0, 0x01),
  ROW(F(Mem), M(M8) | M(Any), 0, 0x00),
  ROW(F(SReg) | F(Mem), M(M16) | M(Any), 0, 0x00),
  ROW(F(SReg) | F(Mem), M(M32) | M(Any), 0, 0x00),
  ROW(F(Mem), M(M16) | M(Any), 0, 0x00),
  ROW(F(SReg), 0, 0, 0x00),
  ROW(F(Gpw), 0, 0, 0x01),
  ROW(F(Gpd), 0, 0, 0x01),
  ROW(F(Mem), M(M32) | M(Any), 0, 0x00),
  ROW(F(Gpq), 0, 0, 0x01),
  ROW(F(Mem), M(M64) | M(Any), 0, 0x00),
  ROW(F(GpbLo), 0, 0, 0x01),
  ROW(F(CReg) | F(DReg), 0, 0, 0x00),
  ROW(F(Gpq) | F(I32), 0, 0, 0x00),
  ROW(F(Gpw) | F(Gpd) | F(Gpq) | F(Mem), M(M16) | M(M32) | M(M64) | M(Any), 0, 0x00),
  ROW(F(I8), 0, 0, 0x00),
  ROW(F(Gpw) | F(Mem), M(M16) | M(Any), 0, 0x00),
  ROW(F(Gpd) | F(Mem), M(M32) | M(Any), 0, 0x00),
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
  ROW(F(I64) | F(U64), 0, 0, 0x00),
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
  ROW(F(Gpw) | F(Gpd) | F(Gpq) | F(Mem), M(M16) | M(M32) | M(M64), 0, 0x00),
  ROW(F(Vm), M(Vm32x), 0, 0x00),
  ROW(F(Vm), M(Vm32y), 0, 0x00),
  ROW(F(Vm), M(Vm32z), 0, 0x00),
  ROW(F(Vm), M(Vm64x), 0, 0x00),
  ROW(F(Vm), M(Vm64y), 0, 0x00),
  ROW(F(Vm), M(Vm64z), 0, 0x00),
  ROW(F(GpbLo) | F(Implicit), 0, 0, 0x01),
  ROW(F(Gpw) | F(Gpq) | F(Mem), M(M16) | M(M64), 0, 0x00),
  ROW(F(SReg), 0, 0, 0x1A),
  ROW(F(SReg), 0, 0, 0x60),
  ROW(F(Gpw) | F(Gpq) | F(Mem) | F(I8) | F(I16) | F(I32), M(M16) | M(M64), 0, 0x00),
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
  ROW(F(Mm) | F(Mem), M(M64) | M(Any), 0, 0x00),
  ROW(F(Mm) | F(Mem) | F(I8) | F(U8), M(M64) | M(Any), 0, 0x00),
  ROW(F(U16), 0, 0, 0x00),
  ROW(F(Xmm) | F(Ymm) | F(Mem), M(M128) | M(M256), 0, 0x00),
  ROW(F(Xmm) | F(I8) | F(U8), 0, 0, 0x00),
  ROW(F(Xmm) | F(Implicit), 0, 0, 0x01),
  ROW(F(Mem), M(Mib), 0, 0x00),
  ROW(F(Gpw) | F(Gpd) | F(Gpq), 0, 0, 0x00),
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
  ROW(F(Mem), M(M8) | M(M16) | M(M32) | M(M48) | M(M64) | M(M80) | M(M128) | M(M256) | M(M512) | M(M1024) | M(Any), 0, 0x00),
  ROW(F(GpbLo) | F(Gpw) | F(Gpd) | F(Gpq) | F(Implicit), 0, 0, 0x01),
  ROW(F(Mem) | F(Implicit), M(BaseOnly) | M(Ds), 0, 0x80),
  ROW(F(Gpw) | F(U8), 0, 0, 0x04),
  ROW(F(Mem), M(BaseOnly) | M(Ds), 0, 0x40),
  ROW(F(Gpw) | F(Gpd) | F(Gpq) | F(Mem), M(M16) | M(Any), 0, 0x00),
  ROW(F(Tmm), 0, 0, 0x00),
  ROW(F(Mem), M(BaseOnly) | M(Ds), 0, 0x00),
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
const uint8_t InstDB::rwInfoIndexA[Inst::_kIdCount] = {
  0, 0, 1, 1, 0, 2, 3, 2, 4, 4, 5, 6, 4, 4, 3, 4, 4, 4, 4, 7, 0, 2, 0, 4, 4, 4,
  4, 8, 0, 9, 9, 9, 9, 9, 0, 0, 0, 0, 9, 9, 9, 9, 9, 10, 10, 10, 11, 11, 12, 13,
  14, 9, 9, 0, 15, 16, 16, 16, 0, 0, 0, 17, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
  3, 3, 3, 3, 3, 3, 18, 0, 0, 19, 0, 0, 0, 0, 0, 20, 21, 0, 22, 23, 24, 7, 25,
  25, 25, 24, 26, 7, 24, 27, 28, 29, 30, 31, 32, 33, 25, 25, 7, 27, 28, 33, 34,
  0, 0, 0, 0, 35, 4, 4, 5, 6, 0, 0, 0, 0, 0, 36, 36, 0, 0, 37, 0, 0, 38, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 38, 0, 38, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 38, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 38, 0, 38, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 4, 0, 4, 4, 35,
  39, 40, 0, 0, 0, 41, 0, 37, 0, 0, 0, 0, 42, 0, 43, 42, 42, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 44, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 45, 46, 47, 48, 49, 50, 51,
  52, 0, 0, 0, 53, 54, 55, 56, 0, 0, 0, 0, 0, 0, 0, 0, 0, 53, 54, 55, 56, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 57, 58, 0, 59, 0, 60, 0, 59, 0, 59, 0, 59, 0, 0,
  0, 0, 61, 62, 62, 62, 57, 59, 0, 0, 0, 9, 0, 0, 4, 4, 5, 6, 0, 0, 4, 4, 5, 6,
  0, 0, 63, 64, 65, 65, 66, 46, 24, 36, 66, 51, 65, 65, 67, 68, 68, 69, 70, 70,
  71, 71, 58, 58, 66, 58, 58, 70, 70, 72, 47, 51, 73, 47, 7, 7, 46, 74, 9, 65,
  65, 74, 0, 35, 4, 4, 5, 6, 0, 75, 0, 0, 76, 0, 2, 4, 4, 77, 78, 9, 9, 9, 3, 3,
  4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 0, 3, 3, 0, 3, 79, 3, 0, 0, 0, 3, 3, 4, 3, 0, 0,
  3, 3, 4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 27, 27, 79, 79, 79, 79, 79, 79, 79, 79,
  79, 79, 27, 79, 79, 79, 27, 27, 79, 79, 79, 3, 3, 3, 80, 3, 3, 3, 27, 27, 0,
  0, 0, 0, 3, 3, 4, 4, 3, 3, 4, 4, 4, 4, 3, 3, 4, 4, 81, 82, 83, 24, 24, 24, 82,
  82, 83, 24, 24, 24, 82, 4, 3, 79, 3, 3, 4, 3, 3, 0, 0, 0, 9, 0, 0, 0, 3, 0,
  0, 0, 0, 0, 0, 0, 3, 3, 0, 0, 0, 0, 3, 3, 3, 3, 84, 3, 3, 0, 3, 3, 3, 84, 3,
  3, 3, 3, 3, 3, 3, 3, 3, 3, 27, 85, 0, 3, 3, 4, 3, 3, 3, 4, 3, 0, 0, 0, 0, 0, 0,
  0, 3, 86, 7, 87, 86, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 88, 0, 0, 0, 0, 86, 86,
  0, 0, 0, 0, 0, 0, 7, 87, 0, 0, 86, 86, 0, 0, 2, 89, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  4, 4, 4, 0, 4, 4, 0, 86, 0, 0, 86, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 7, 26, 87, 0,
  0, 0, 0, 0, 0, 90, 0, 0, 2, 4, 4, 5, 6, 0, 0, 0, 0, 0, 0, 0, 9, 0, 0, 0, 0, 0,
  15, 91, 91, 0, 92, 0, 0, 9, 9, 20, 21, 0, 0, 0, 0, 0, 4, 4, 4, 4, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 93, 28, 94, 95, 94, 95, 93, 28, 94, 95, 94, 95, 96, 97, 0, 0, 0, 0, 20,
  21, 98, 98, 99, 9, 0, 74, 100, 100, 9, 100, 9, 99, 9, 99, 0, 99, 9, 99, 9, 100,
  28, 0, 28, 0, 0, 0, 33, 33, 100, 9, 100, 9, 9, 99, 9, 99, 28, 28, 33, 33,
  99, 9, 9, 100, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 101, 101, 9, 9, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 9, 9, 27, 102, 59, 59, 0,
  0, 0, 0, 0, 0, 0, 0, 59, 59, 9, 9, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 66, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 103, 103, 46, 104, 103, 103, 103, 103, 103, 103, 103, 103, 0, 105, 105,
  0, 70, 70, 106, 107, 66, 66, 66, 66, 108, 70, 9, 9, 72, 103, 103, 0, 0, 0,
  98, 0, 0, 0, 0, 0, 0, 0, 109, 0, 0, 0, 0, 0, 0, 0, 9, 9, 9, 9, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 110, 33, 111, 111, 28,
  112, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  98, 98, 98, 98, 0, 0, 0, 0, 0, 0, 9, 9, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 9, 9, 9, 9, 0, 0, 0, 0, 59, 59,
  59, 59, 7, 7, 7, 0, 7, 0, 7, 7, 7, 7, 7, 7, 0, 7, 7, 80, 7, 0, 7, 0, 0, 7, 0,
  0, 0, 0, 9, 9, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 113, 113, 114, 115, 111, 111, 111, 111,
  81, 113, 116, 115, 114, 114, 115, 116, 115, 114, 115, 117, 118, 99, 99, 99,
  117, 114, 115, 116, 115, 114, 115, 113, 115, 117, 118, 99, 99, 99, 117, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 9, 9, 9, 9, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 66, 66, 119, 66, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 109, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 9, 9, 0, 0, 101, 101, 0, 0, 9, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 9, 9, 0, 0, 101, 101, 0, 0, 9, 0, 0, 0, 0, 0, 66,
  66, 0, 0, 0, 0, 0, 0, 0, 0, 66, 119, 0, 0, 0, 0, 0, 0, 9, 9, 0, 0, 0, 0, 0, 0,
  0, 109, 109, 20, 21, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 120, 121, 120, 121,
  0, 122, 0, 123, 0, 0, 0, 2, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0
};

const uint8_t InstDB::rwInfoIndexB[Inst::_kIdCount] = {
  0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 3, 0, 0, 0,
  0, 0, 4, 0, 0, 0, 0, 0, 5, 5, 6, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 7, 0, 0, 0, 0, 4, 8, 1, 0, 9, 0, 0, 0, 10, 10, 10, 0, 0, 11, 0, 10, 12, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 5, 5, 0, 13, 14, 15, 16, 17, 0, 0, 18, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 19, 1, 1, 20, 21, 0, 0, 0,
  0, 5, 5, 0, 0, 0, 0, 0, 0, 22, 23, 0, 0, 24, 25, 26, 27, 0, 0, 25, 25, 25, 25,
  25, 25, 25, 25, 28, 29, 29, 28, 0, 0, 0, 24, 25, 24, 25, 0, 25, 24, 24, 24, 24,
  24, 24, 24, 0, 0, 30, 30, 30, 24, 24, 28, 0, 31, 10, 0, 0, 0, 0, 0, 0, 24,
  25, 0, 0, 0, 32, 33, 32, 34, 0, 0, 0, 0, 0, 10, 32, 0, 0, 0, 0, 35, 33, 32, 35,
  34, 24, 25, 24, 25, 0, 29, 29, 29, 29, 0, 0, 0, 25, 10, 10, 32, 32, 0, 0, 0,
  0, 5, 5, 0, 0, 0, 0, 0, 0, 21, 36, 0, 20, 37, 38, 0, 39, 40, 0, 0, 0, 0, 0, 10,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 41, 42, 43, 44, 41, 42, 41, 42, 43,
  44, 43, 44, 0, 0, 0, 0, 0, 0, 0, 0, 41, 42, 43, 0, 0, 0, 0, 44, 45, 46, 47, 48,
  45, 46, 47, 48, 0, 0, 0, 0, 49, 50, 51, 41, 42, 43, 44, 41, 42, 43, 44, 52,
  0, 0, 53, 0, 54, 0, 0, 0, 0, 0, 10, 0, 10, 55, 56, 55, 0, 0, 0, 0, 0, 0, 55, 57,
  57, 0, 58, 59, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 60, 60, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 5, 61, 0, 0, 0, 0, 62, 0, 63, 20, 64, 20, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 65, 0, 0, 0, 0, 0, 0, 6, 5, 5, 0,
  0, 0, 0, 66, 67, 0, 0, 0, 0, 68, 69, 0, 3, 3, 70, 22, 71, 72, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 73,
  39, 74, 75, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 76, 0, 0, 0, 0, 0, 0, 0, 10, 10, 10, 10,
  10, 10, 10, 0, 0, 2, 2, 2, 77, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 64, 0, 0, 0, 0, 0, 0, 0, 0, 78, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 79, 79, 80, 79, 80, 80, 80, 79, 79, 81, 82, 0, 83, 0, 0, 0, 0, 0,
  84, 2, 2, 85, 86, 0, 0, 0, 11, 87, 0, 0, 4, 0, 0, 0, 0, 88, 88, 88, 88, 88,
  88, 88, 88, 88, 88, 88, 88, 88, 88, 88, 88, 88, 88, 88, 88, 88, 88, 88, 88, 88,
  88, 88, 88, 88, 0, 88, 0, 32, 0, 0, 0, 5, 0, 0, 6, 0, 89, 4, 0, 89, 4, 5, 5,
  32, 19, 90, 79, 90, 0, 0, 0, 0, 0, 0, 0, 0, 0, 91, 0, 90, 92, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 93, 93, 93, 93, 93, 0, 0, 0, 0, 0, 94, 95, 0, 0, 0, 0,
  96, 96, 0, 56, 95, 0, 0, 0, 0, 97, 98, 97, 98, 3, 3, 99, 100, 3, 3, 3, 3, 3,
  3, 0, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 101, 101, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 3, 3, 102, 103, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 104, 0, 0, 0, 0, 0, 0, 105, 0, 106, 107, 108, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 106, 107, 3, 3, 3, 99, 100, 3, 109, 3, 55, 55,
  0, 0, 0, 0, 110, 111, 112, 111, 112, 110, 111, 112, 111, 112, 22, 113, 113,
  114, 115, 113, 113, 116, 117, 113, 113, 116, 117, 113, 113, 116, 117, 118, 118,
  119, 120, 113, 113, 113, 113, 113, 113, 118, 118, 113, 113, 116, 117, 113, 113,
  116, 117, 113, 113, 116, 117, 113, 113, 113, 113, 113, 113, 118, 118, 118,
  118, 119, 120, 113, 113, 116, 117, 113, 113, 116, 117, 113, 113, 116, 117, 118,
  118, 119, 120, 113, 113, 116, 117, 113, 113, 116, 117, 113, 113, 121, 122,
  118, 118, 119, 120, 123, 123, 77, 124, 0, 0, 0, 0, 125, 126, 10, 10, 10, 10, 10,
  10, 10, 10, 126, 127, 0, 0, 128, 129, 84, 84, 128, 129, 3, 3, 3, 3, 3, 3, 3,
  130, 131, 132, 131, 132, 130, 131, 132, 131, 132, 100, 0, 53, 58, 133, 133,
  3, 3, 99, 100, 0, 134, 0, 3, 3, 99, 100, 0, 135, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 136, 137, 137, 138, 139, 139, 0, 0, 0, 0, 0, 0, 0, 140, 0, 0, 141, 0,
  0, 3, 11, 134, 0, 0, 142, 135, 3, 3, 99, 100, 0, 11, 3, 3, 143, 143, 144, 144,
  0, 0, 0, 0, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
  3, 101, 3, 0, 0, 0, 0, 0, 0, 3, 118, 145, 145, 3, 3, 3, 3, 66, 67, 3, 3, 3, 3,
  68, 69, 145, 145, 145, 145, 145, 145, 109, 109, 0, 0, 0, 0, 109, 109, 109, 109,
  109, 109, 0, 0, 113, 113, 113, 113, 146, 146, 3, 3, 3, 113, 3, 3, 113, 113,
  118, 118, 147, 147, 147, 3, 147, 3, 113, 113, 113, 113, 113, 3, 0, 0, 0, 0, 70,
  22, 71, 148, 126, 125, 127, 126, 0, 0, 0, 3, 0, 3, 0, 0, 0, 0, 0, 0, 3, 0,
  0, 0, 0, 3, 0, 3, 3, 0, 149, 100, 99, 150, 0, 0, 151, 151, 151, 151, 151, 151,
  151, 151, 151, 151, 151, 151, 113, 113, 3, 3, 133, 133, 3, 3, 3, 3, 3, 3, 3,
  3, 3, 3, 3, 3, 3, 3, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 3, 3, 3,
  3, 3, 3, 3, 3, 0, 0, 0, 0, 3, 3, 3, 152, 84, 84, 3, 3, 84, 84, 3, 3, 153, 153,
  153, 153, 3, 0, 0, 0, 0, 153, 153, 153, 153, 153, 153, 3, 3, 113, 113, 113, 3,
  153, 153, 3, 3, 113, 113, 113, 3, 3, 145, 84, 84, 84, 3, 3, 3, 154, 155, 154,
  3, 3, 3, 154, 154, 154, 3, 3, 3, 154, 154, 155, 154, 3, 3, 3, 154, 3, 3, 3,
  3, 3, 3, 3, 3, 113, 113, 0, 145, 145, 145, 145, 145, 145, 145, 145, 3, 3, 3, 3,
  3, 3, 3, 3, 3, 3, 3, 3, 3, 128, 129, 0, 0, 128, 129, 0, 0, 128, 129, 0, 129,
  84, 84, 128, 129, 84, 84, 128, 129, 84, 84, 128, 129, 0, 0, 128, 129, 0, 0, 128,
  129, 0, 129, 3, 3, 99, 100, 0, 0, 10, 10, 10, 10, 10, 10, 10, 10, 0, 0, 3,
  3, 3, 3, 3, 3, 0, 0, 128, 129, 91, 3, 3, 99, 100, 0, 0, 0, 0, 3, 3, 3, 3, 3,
  3, 0, 0, 0, 0, 56, 56, 156, 0, 0, 0, 0, 0, 0, 0, 0, 0, 80, 0, 0, 0, 0, 0, 157,
  157, 157, 157, 158, 158, 158, 158, 158, 158, 158, 158, 156, 0, 0
};

const InstDB::RWInfo InstDB::rwInfoA[] = {
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 0 , 0 , 0 , 0 , 0 , 0  } }, // #0 [ref=931x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 1 , 0 , 0 , 0 , 0 , 0  } }, // #1 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 1 , { 2 , 3 , 0 , 0 , 0 , 0  } }, // #2 [ref=7x]
  { InstDB::RWInfo::kCategoryGeneric   , 2 , { 2 , 3 , 0 , 0 , 0 , 0  } }, // #3 [ref=99x]
  { InstDB::RWInfo::kCategoryGeneric   , 3 , { 4 , 5 , 0 , 0 , 0 , 0  } }, // #4 [ref=55x]
  { InstDB::RWInfo::kCategoryGeneric   , 4 , { 6 , 7 , 0 , 0 , 0 , 0  } }, // #5 [ref=6x]
  { InstDB::RWInfo::kCategoryGeneric   , 5 , { 8 , 9 , 0 , 0 , 0 , 0  } }, // #6 [ref=6x]
  { InstDB::RWInfo::kCategoryGeneric   , 3 , { 10, 5 , 0 , 0 , 0 , 0  } }, // #7 [ref=26x]
  { InstDB::RWInfo::kCategoryGeneric   , 7 , { 12, 13, 0 , 0 , 0 , 0  } }, // #8 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 2 , { 11, 3 , 0 , 0 , 0 , 0  } }, // #9 [ref=65x]
  { InstDB::RWInfo::kCategoryGeneric   , 2 , { 5 , 3 , 0 , 0 , 0 , 0  } }, // #10 [ref=3x]
  { InstDB::RWInfo::kCategoryGeneric   , 8 , { 10, 3 , 0 , 0 , 0 , 0  } }, // #11 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 9 , { 10, 5 , 0 , 0 , 0 , 0  } }, // #12 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 8 , { 15, 5 , 0 , 0 , 0 , 0  } }, // #13 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 3 , 3 , 0 , 0 , 0 , 0  } }, // #14 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 10, { 3 , 3 , 0 , 0 , 0 , 0  } }, // #15 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 10, { 2 , 3 , 0 , 0 , 0 , 0  } }, // #16 [ref=3x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 16, 17, 0 , 0 , 0 , 0  } }, // #17 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 1 , { 3 , 3 , 0 , 0 , 0 , 0  } }, // #18 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 20, 21, 0 , 0 , 0 , 0  } }, // #19 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 4 , { 7 , 7 , 0 , 0 , 0 , 0  } }, // #20 [ref=4x]
  { InstDB::RWInfo::kCategoryGeneric   , 5 , { 9 , 9 , 0 , 0 , 0 , 0  } }, // #21 [ref=4x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 33, 34, 0 , 0 , 0 , 0  } }, // #22 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 14, { 2 , 3 , 0 , 0 , 0 , 0  } }, // #23 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 4 , { 10, 7 , 0 , 0 , 0 , 0  } }, // #24 [ref=10x]
  { InstDB::RWInfo::kCategoryGeneric   , 3 , { 35, 5 , 0 , 0 , 0 , 0  } }, // #25 [ref=5x]
  { InstDB::RWInfo::kCategoryGeneric   , 4 , { 36, 7 , 0 , 0 , 0 , 0  } }, // #26 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 4 , { 35, 7 , 0 , 0 , 0 , 0  } }, // #27 [ref=11x]
  { InstDB::RWInfo::kCategoryGeneric   , 4 , { 11, 7 , 0 , 0 , 0 , 0  } }, // #28 [ref=9x]
  { InstDB::RWInfo::kCategoryGeneric   , 4 , { 37, 7 , 0 , 0 , 0 , 0  } }, // #29 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 14, { 36, 3 , 0 , 0 , 0 , 0  } }, // #30 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 14, { 37, 3 , 0 , 0 , 0 , 0  } }, // #31 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 5 , { 36, 9 , 0 , 0 , 0 , 0  } }, // #32 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 5 , { 11, 9 , 0 , 0 , 0 , 0  } }, // #33 [ref=7x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 38, 39, 0 , 0 , 0 , 0  } }, // #34 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 15, { 1 , 40, 0 , 0 , 0 , 0  } }, // #35 [ref=3x]
  { InstDB::RWInfo::kCategoryGeneric   , 16, { 11, 43, 0 , 0 , 0 , 0  } }, // #36 [ref=3x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 4 , 5 , 0 , 0 , 0 , 0  } }, // #37 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 45, 46, 0 , 0 , 0 , 0  } }, // #38 [ref=6x]
  { InstDB::RWInfo::kCategoryImul      , 2 , { 0 , 0 , 0 , 0 , 0 , 0  } }, // #39 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 50, 51, 0 , 0 , 0 , 0  } }, // #40 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 53, 51, 0 , 0 , 0 , 0  } }, // #41 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 13, { 3 , 5 , 0 , 0 , 0 , 0  } }, // #42 [ref=3x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 22, 29, 0 , 0 , 0 , 0  } }, // #43 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 54, 0 , 0 , 0 , 0 , 0  } }, // #44 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 23, { 55, 40, 0 , 0 , 0 , 0  } }, // #45 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 24, { 44, 9 , 0 , 0 , 0 , 0  } }, // #46 [ref=4x]
  { InstDB::RWInfo::kCategoryGeneric   , 25, { 35, 7 , 0 , 0 , 0 , 0  } }, // #47 [ref=3x]
  { InstDB::RWInfo::kCategoryGeneric   , 26, { 48, 13, 0 , 0 , 0 , 0  } }, // #48 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 55, 40, 0 , 0 , 0 , 0  } }, // #49 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 44, 9 , 0 , 0 , 0 , 0  } }, // #50 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 35, 7 , 0 , 0 , 0 , 0  } }, // #51 [ref=3x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 48, 13, 0 , 0 , 0 , 0  } }, // #52 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 40, 40, 0 , 0 , 0 , 0  } }, // #53 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 9 , 9 , 0 , 0 , 0 , 0  } }, // #54 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 7 , 7 , 0 , 0 , 0 , 0  } }, // #55 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 13, 13, 0 , 0 , 0 , 0  } }, // #56 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 27, { 11, 3 , 0 , 0 , 0 , 0  } }, // #57 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 13, { 10, 5 , 0 , 0 , 0 , 0  } }, // #58 [ref=5x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 2 , 3 , 0 , 0 , 0 , 0  } }, // #59 [ref=13x]
  { InstDB::RWInfo::kCategoryGeneric   , 8 , { 11, 3 , 0 , 0 , 0 , 0  } }, // #60 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 50, 20, 0 , 0 , 0 , 0  } }, // #61 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 57, 0 , 0 , 0 , 0 , 0  } }, // #62 [ref=3x]
  { InstDB::RWInfo::kCategoryMov       , 29, { 0 , 0 , 0 , 0 , 0 , 0  } }, // #63 [ref=1x]
  { InstDB::RWInfo::kCategoryMovabs    , 0 , { 0 , 0 , 0 , 0 , 0 , 0  } }, // #64 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 30, { 10, 5 , 0 , 0 , 0 , 0  } }, // #65 [ref=6x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 11, 3 , 0 , 0 , 0 , 0  } }, // #66 [ref=14x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 36, 60, 0 , 0 , 0 , 0  } }, // #67 [ref=1x]
  { InstDB::RWInfo::kCategoryMovh64    , 12, { 0 , 0 , 0 , 0 , 0 , 0  } }, // #68 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 61, 7 , 0 , 0 , 0 , 0  } }, // #69 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 12, { 35, 7 , 0 , 0 , 0 , 0  } }, // #70 [ref=7x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 55, 5 , 0 , 0 , 0 , 0  } }, // #71 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 28, { 44, 9 , 0 , 0 , 0 , 0  } }, // #72 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 62, 20, 0 , 0 , 0 , 0  } }, // #73 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 14, { 11, 3 , 0 , 0 , 0 , 0  } }, // #74 [ref=3x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 17, 29, 0 , 0 , 0 , 0  } }, // #75 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 11, { 3 , 3 , 0 , 0 , 0 , 0  } }, // #76 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 51, 22, 0 , 0 , 0 , 0  } }, // #77 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 51, 65, 0 , 0 , 0 , 0  } }, // #78 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 4 , { 26, 7 , 0 , 0 , 0 , 0  } }, // #79 [ref=18x]
  { InstDB::RWInfo::kCategoryGeneric   , 3 , { 68, 5 , 0 , 0 , 0 , 0  } }, // #80 [ref=2x]
  { InstDB::RWInfo::kCategoryVmov1_8   , 0 , { 0 , 0 , 0 , 0 , 0 , 0  } }, // #81 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 5 , { 10, 9 , 0 , 0 , 0 , 0  } }, // #82 [ref=4x]
  { InstDB::RWInfo::kCategoryGeneric   , 27, { 10, 13, 0 , 0 , 0 , 0  } }, // #83 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 4 , 0 , 0 , 0 , 0 , 0  } }, // #84 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 3 , { 5 , 5 , 0 , 0 , 0 , 0  } }, // #85 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 10, { 2 , 70, 0 , 0 , 0 , 0  } }, // #86 [ref=8x]
  { InstDB::RWInfo::kCategoryGeneric   , 5 , { 37, 9 , 0 , 0 , 0 , 0  } }, // #87 [ref=3x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 16, 71, 0 , 0 , 0 , 0  } }, // #88 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 22, 21, 0 , 0 , 0 , 0  } }, // #89 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 62, 22, 0 , 0 , 0 , 0  } }, // #90 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 8 , { 74, 3 , 0 , 0 , 0 , 0  } }, // #91 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 8 , { 11, 43, 0 , 0 , 0 , 0  } }, // #92 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 13, { 76, 5 , 0 , 0 , 0 , 0  } }, // #93 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 13, { 11, 5 , 0 , 0 , 0 , 0  } }, // #94 [ref=4x]
  { InstDB::RWInfo::kCategoryGeneric   , 37, { 74, 77, 0 , 0 , 0 , 0  } }, // #95 [ref=4x]
  { InstDB::RWInfo::kCategoryGeneric   , 38, { 11, 7 , 0 , 0 , 0 , 0  } }, // #96 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 39, { 11, 9 , 0 , 0 , 0 , 0  } }, // #97 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 11, { 11, 3 , 0 , 0 , 0 , 0  } }, // #98 [ref=7x]
  { InstDB::RWInfo::kCategoryVmov2_1   , 40, { 0 , 0 , 0 , 0 , 0 , 0  } }, // #99 [ref=14x]
  { InstDB::RWInfo::kCategoryVmov1_2   , 14, { 0 , 0 , 0 , 0 , 0 , 0  } }, // #100 [ref=7x]
  { InstDB::RWInfo::kCategoryGeneric   , 44, { 74, 43, 0 , 0 , 0 , 0  } }, // #101 [ref=6x]
  { InstDB::RWInfo::kCategoryGeneric   , 5 , { 44, 9 , 0 , 0 , 0 , 0  } }, // #102 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 51, { 11, 3 , 0 , 0 , 0 , 0  } }, // #103 [ref=12x]
  { InstDB::RWInfo::kCategoryVmovddup  , 52, { 0 , 0 , 0 , 0 , 0 , 0  } }, // #104 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 12, { 35, 60, 0 , 0 , 0 , 0  } }, // #105 [ref=2x]
  { InstDB::RWInfo::kCategoryVmovmskpd , 0 , { 0 , 0 , 0 , 0 , 0 , 0  } }, // #106 [ref=1x]
  { InstDB::RWInfo::kCategoryVmovmskps , 0 , { 0 , 0 , 0 , 0 , 0 , 0  } }, // #107 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 53, { 35, 7 , 0 , 0 , 0 , 0  } }, // #108 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 2 , { 3 , 3 , 0 , 0 , 0 , 0  } }, // #109 [ref=4x]
  { InstDB::RWInfo::kCategoryGeneric   , 15, { 11, 40, 0 , 0 , 0 , 0  } }, // #110 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 11, 7 , 0 , 0 , 0 , 0  } }, // #111 [ref=6x]
  { InstDB::RWInfo::kCategoryGeneric   , 27, { 11, 13, 0 , 0 , 0 , 0  } }, // #112 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 35, 3 , 0 , 0 , 0 , 0  } }, // #113 [ref=4x]
  { InstDB::RWInfo::kCategoryVmov1_4   , 57, { 0 , 0 , 0 , 0 , 0 , 0  } }, // #114 [ref=6x]
  { InstDB::RWInfo::kCategoryVmov1_2   , 41, { 0 , 0 , 0 , 0 , 0 , 0  } }, // #115 [ref=9x]
  { InstDB::RWInfo::kCategoryVmov1_8   , 58, { 0 , 0 , 0 , 0 , 0 , 0  } }, // #116 [ref=3x]
  { InstDB::RWInfo::kCategoryVmov4_1   , 59, { 0 , 0 , 0 , 0 , 0 , 0  } }, // #117 [ref=4x]
  { InstDB::RWInfo::kCategoryVmov8_1   , 60, { 0 , 0 , 0 , 0 , 0 , 0  } }, // #118 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 18, { 11, 3 , 0 , 0 , 0 , 0  } }, // #119 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 17, { 44, 9 , 0 , 0 , 0 , 0  } }, // #120 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 32, { 35, 7 , 0 , 0 , 0 , 0  } }, // #121 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 11, { 2 , 2 , 0 , 0 , 0 , 0  } }, // #122 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 51, { 2 , 2 , 0 , 0 , 0 , 0  } }  // #123 [ref=1x]
};

const InstDB::RWInfo InstDB::rwInfoB[] = {
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 0 , 0 , 0 , 0 , 0 , 0  } }, // #0 [ref=735x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 1 , 0 , 0 , 0 , 0 , 0  } }, // #1 [ref=5x]
  { InstDB::RWInfo::kCategoryGeneric   , 3 , { 10, 5 , 0 , 0 , 0 , 0  } }, // #2 [ref=7x]
  { InstDB::RWInfo::kCategoryGeneric   , 6 , { 11, 3 , 3 , 0 , 0 , 0  } }, // #3 [ref=186x]
  { InstDB::RWInfo::kCategoryGeneric   , 2 , { 11, 3 , 3 , 0 , 0 , 0  } }, // #4 [ref=5x]
  { InstDB::RWInfo::kCategoryGeneric   , 3 , { 4 , 5 , 0 , 0 , 0 , 0  } }, // #5 [ref=14x]
  { InstDB::RWInfo::kCategoryGeneric   , 3 , { 4 , 5 , 14, 0 , 0 , 0  } }, // #6 [ref=4x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 2 , 0 , 0 , 0 , 0 , 0  } }, // #7 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 11, { 3 , 0 , 0 , 0 , 0 , 0  } }, // #8 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 18, 0 , 0 , 0 , 0 , 0  } }, // #9 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 8 , { 3 , 0 , 0 , 0 , 0 , 0  } }, // #10 [ref=34x]
  { InstDB::RWInfo::kCategoryGeneric   , 12, { 7 , 0 , 0 , 0 , 0 , 0  } }, // #11 [ref=4x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 19, 0 , 0 , 0 , 0 , 0  } }, // #12 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 4 , { 6 , 7 , 0 , 0 , 0 , 0  } }, // #13 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 5 , { 8 , 9 , 0 , 0 , 0 , 0  } }, // #14 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 11, { 2 , 3 , 22, 0 , 0 , 0  } }, // #15 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 13, { 4 , 23, 18, 24, 25, 0  } }, // #16 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 12, { 26, 27, 28, 29, 30, 0  } }, // #17 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 28, 31, 32, 16, 0 , 0  } }, // #18 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 28, 0 , 0 , 0 , 0 , 0  } }, // #19 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 10, { 2 , 0 , 0 , 0 , 0 , 0  } }, // #20 [ref=4x]
  { InstDB::RWInfo::kCategoryGeneric   , 6 , { 41, 42, 3 , 0 , 0 , 0  } }, // #21 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 17, { 44, 5 , 0 , 0 , 0 , 0  } }, // #22 [ref=4x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 4 , 0 , 0 , 0 , 0 , 0  } }, // #23 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 18, { 3 , 0 , 0 , 0 , 0 , 0  } }, // #24 [ref=15x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 45, 0 , 0 , 0 , 0 , 0  } }, // #25 [ref=16x]
  { InstDB::RWInfo::kCategoryGeneric   , 19, { 46, 0 , 0 , 0 , 0 , 0  } }, // #26 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 19, { 47, 0 , 0 , 0 , 0 , 0  } }, // #27 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 20, { 3 , 0 , 0 , 0 , 0 , 0  } }, // #28 [ref=3x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 46, 0 , 0 , 0 , 0 , 0  } }, // #29 [ref=6x]
  { InstDB::RWInfo::kCategoryGeneric   , 18, { 11, 0 , 0 , 0 , 0 , 0  } }, // #30 [ref=3x]
  { InstDB::RWInfo::kCategoryGeneric   , 21, { 13, 0 , 0 , 0 , 0 , 0  } }, // #31 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 8 , { 11, 0 , 0 , 0 , 0 , 0  } }, // #32 [ref=8x]
  { InstDB::RWInfo::kCategoryGeneric   , 21, { 48, 0 , 0 , 0 , 0 , 0  } }, // #33 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 7 , { 49, 0 , 0 , 0 , 0 , 0  } }, // #34 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 20, { 11, 0 , 0 , 0 , 0 , 0  } }, // #35 [ref=2x]
  { InstDB::RWInfo::kCategoryImul      , 22, { 0 , 0 , 0 , 0 , 0 , 0  } }, // #36 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 52, 0 , 0 , 0 , 0 , 0  } }, // #37 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 26, 0 , 0 , 0 , 0 , 0  } }, // #38 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 5 , { 4 , 9 , 0 , 0 , 0 , 0  } }, // #39 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 4 , 5 , 0 , 0 , 0 , 0  } }, // #40 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 55, 40, 40, 0 , 0 , 0  } }, // #41 [ref=6x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 44, 9 , 9 , 0 , 0 , 0  } }, // #42 [ref=6x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 35, 7 , 7 , 0 , 0 , 0  } }, // #43 [ref=6x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 48, 13, 13, 0 , 0 , 0  } }, // #44 [ref=6x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 55, 40, 0 , 0 , 0 , 0  } }, // #45 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 44, 9 , 0 , 0 , 0 , 0  } }, // #46 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 35, 7 , 0 , 0 , 0 , 0  } }, // #47 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 48, 13, 0 , 0 , 0 , 0  } }, // #48 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 48, 40, 40, 0 , 0 , 0  } }, // #49 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 35, 9 , 9 , 0 , 0 , 0  } }, // #50 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 44, 13, 13, 0 , 0 , 0  } }, // #51 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 56, 0 , 0 , 0 , 0 , 0  } }, // #52 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 28, { 9 , 0 , 0 , 0 , 0 , 0  } }, // #53 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 16, { 43, 0 , 0 , 0 , 0 , 0  } }, // #54 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 7 , { 13, 0 , 0 , 0 , 0 , 0  } }, // #55 [ref=5x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 3 , 0 , 0 , 0 , 0 , 0  } }, // #56 [ref=4x]
  { InstDB::RWInfo::kCategoryGeneric   , 5 , { 3 , 9 , 0 , 0 , 0 , 0  } }, // #57 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 5 , 5 , 58, 0 , 0 , 0  } }, // #58 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 7 , 7 , 58, 0 , 0 , 0  } }, // #59 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 19, 29, 59, 0 , 0 , 0  } }, // #60 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 6 , { 63, 42, 3 , 0 , 0 , 0  } }, // #61 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 6 , { 11, 11, 3 , 64, 0 , 0  } }, // #62 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 17, 29, 30, 0 , 0 , 0  } }, // #63 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 10, { 3 , 0 , 0 , 0 , 0 , 0  } }, // #64 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 2 , { 2 , 3 , 0 , 0 , 0 , 0  } }, // #65 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 3 , { 5 , 5 , 0 , 66, 17, 59 } }, // #66 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 3 , { 5 , 5 , 0 , 67, 17, 59 } }, // #67 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 3 , { 5 , 5 , 0 , 66, 0 , 0  } }, // #68 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 3 , { 5 , 5 , 0 , 67, 0 , 0  } }, // #69 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 31, { 55, 5 , 0 , 0 , 0 , 0  } }, // #70 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 32, { 35, 5 , 0 , 0 , 0 , 0  } }, // #71 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 33, { 48, 3 , 0 , 0 , 0 , 0  } }, // #72 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 15, { 4 , 40, 0 , 0 , 0 , 0  } }, // #73 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 4 , { 4 , 7 , 0 , 0 , 0 , 0  } }, // #74 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 27, { 2 , 13, 0 , 0 , 0 , 0  } }, // #75 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 10, { 69, 0 , 0 , 0 , 0 , 0  } }, // #76 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 4 , { 35, 7 , 0 , 0 , 0 , 0  } }, // #77 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 10, { 64, 0 , 0 , 0 , 0 , 0  } }, // #78 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 11, 0 , 0 , 0 , 0 , 0  } }, // #79 [ref=6x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 16, 71, 29, 0 , 0 , 0  } }, // #80 [ref=5x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 44, 0 , 0 , 0 , 0 , 0  } }, // #81 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 35, 0 , 0 , 0 , 0 , 0  } }, // #82 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 16, 71, 66, 0 , 0 , 0  } }, // #83 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 2 , { 11, 3 , 0 , 0 , 0 , 0  } }, // #84 [ref=16x]
  { InstDB::RWInfo::kCategoryGeneric   , 4 , { 36, 7 , 0 , 0 , 0 , 0  } }, // #85 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 5 , { 37, 9 , 0 , 0 , 0 , 0  } }, // #86 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 72, 0 , 0 , 0 , 0 , 0  } }, // #87 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 31, { 73, 0 , 0 , 0 , 0 , 0  } }, // #88 [ref=30x]
  { InstDB::RWInfo::kCategoryGeneric   , 11, { 2 , 3 , 70, 0 , 0 , 0  } }, // #89 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 34, { 11, 0 , 0 , 0 , 0 , 0  } }, // #90 [ref=3x]
  { InstDB::RWInfo::kCategoryGeneric   , 28, { 44, 0 , 0 , 0 , 0 , 0  } }, // #91 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 16, { 74, 0 , 0 , 0 , 0 , 0  } }, // #92 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 75, 43, 43, 0 , 0 , 0  } }, // #93 [ref=5x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 74, 0 , 0 , 0 , 0 , 0  } }, // #94 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 9 , 59, 17, 0 , 0 , 0  } }, // #95 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 17, { 52, 0 , 0 , 0 , 0 , 0  } }, // #96 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 13, { 75, 43, 43, 43, 43, 5  } }, // #97 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 13, { 4 , 5 , 5 , 5 , 5 , 5  } }, // #98 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 35, { 10, 5 , 7 , 0 , 0 , 0  } }, // #99 [ref=8x]
  { InstDB::RWInfo::kCategoryGeneric   , 36, { 10, 5 , 9 , 0 , 0 , 0  } }, // #100 [ref=9x]
  { InstDB::RWInfo::kCategoryGeneric   , 6 , { 11, 3 , 3 , 3 , 0 , 0  } }, // #101 [ref=3x]
  { InstDB::RWInfo::kCategoryGeneric   , 35, { 11, 5 , 7 , 0 , 0 , 0  } }, // #102 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 36, { 11, 5 , 9 , 0 , 0 , 0  } }, // #103 [ref=1x]
  { InstDB::RWInfo::kCategoryVmov1_2   , 41, { 0 , 0 , 0 , 0 , 0 , 0  } }, // #104 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 35, { 10, 78, 7 , 0 , 0 , 0  } }, // #105 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 42, { 10, 60, 3 , 0 , 0 , 0  } }, // #106 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 42, { 10, 78, 3 , 0 , 0 , 0  } }, // #107 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 36, { 10, 60, 9 , 0 , 0 , 0  } }, // #108 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 43, { 10, 5 , 5 , 0 , 0 , 0  } }, // #109 [ref=9x]
  { InstDB::RWInfo::kCategoryGeneric   , 45, { 10, 77, 0 , 0 , 0 , 0  } }, // #110 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 45, { 10, 3 , 0 , 0 , 0 , 0  } }, // #111 [ref=4x]
  { InstDB::RWInfo::kCategoryGeneric   , 46, { 76, 43, 0 , 0 , 0 , 0  } }, // #112 [ref=4x]
  { InstDB::RWInfo::kCategoryGeneric   , 6 , { 2 , 3 , 3 , 0 , 0 , 0  } }, // #113 [ref=60x]
  { InstDB::RWInfo::kCategoryGeneric   , 35, { 4 , 60, 7 , 0 , 0 , 0  } }, // #114 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 36, { 4 , 78, 9 , 0 , 0 , 0  } }, // #115 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 35, { 6 , 7 , 7 , 0 , 0 , 0  } }, // #116 [ref=11x]
  { InstDB::RWInfo::kCategoryGeneric   , 36, { 8 , 9 , 9 , 0 , 0 , 0  } }, // #117 [ref=11x]
  { InstDB::RWInfo::kCategoryGeneric   , 47, { 11, 3 , 3 , 3 , 0 , 0  } }, // #118 [ref=15x]
  { InstDB::RWInfo::kCategoryGeneric   , 48, { 35, 7 , 7 , 7 , 0 , 0  } }, // #119 [ref=4x]
  { InstDB::RWInfo::kCategoryGeneric   , 49, { 44, 9 , 9 , 9 , 0 , 0  } }, // #120 [ref=4x]
  { InstDB::RWInfo::kCategoryGeneric   , 35, { 26, 7 , 7 , 0 , 0 , 0  } }, // #121 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 36, { 52, 9 , 9 , 0 , 0 , 0  } }, // #122 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 14, { 35, 3 , 0 , 0 , 0 , 0  } }, // #123 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 5 , { 35, 9 , 0 , 0 , 0 , 0  } }, // #124 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 8 , { 2 , 3 , 2 , 0 , 0 , 0  } }, // #125 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 2 , 3 , 2 , 0 , 0 , 0  } }, // #126 [ref=4x]
  { InstDB::RWInfo::kCategoryGeneric   , 18, { 4 , 3 , 4 , 0 , 0 , 0  } }, // #127 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 35, { 10, 60, 7 , 0 , 0 , 0  } }, // #128 [ref=11x]
  { InstDB::RWInfo::kCategoryGeneric   , 36, { 10, 78, 9 , 0 , 0 , 0  } }, // #129 [ref=13x]
  { InstDB::RWInfo::kCategoryGeneric   , 43, { 76, 77, 5 , 0 , 0 , 0  } }, // #130 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 43, { 11, 3 , 5 , 0 , 0 , 0  } }, // #131 [ref=4x]
  { InstDB::RWInfo::kCategoryGeneric   , 50, { 74, 43, 77, 0 , 0 , 0  } }, // #132 [ref=4x]
  { InstDB::RWInfo::kCategoryVmaskmov  , 0 , { 0 , 0 , 0 , 0 , 0 , 0  } }, // #133 [ref=4x]
  { InstDB::RWInfo::kCategoryGeneric   , 12, { 35, 0 , 0 , 0 , 0 , 0  } }, // #134 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 22, 0 , 0 , 0 , 0 , 0  } }, // #135 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 10, 60, 60, 0 , 0 , 0  } }, // #136 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 12, { 10, 7 , 7 , 0 , 0 , 0  } }, // #137 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 10, 7 , 7 , 0 , 0 , 0  } }, // #138 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 12, { 10, 60, 7 , 0 , 0 , 0  } }, // #139 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 10, 60, 7 , 0 , 0 , 0  } }, // #140 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 10, 78, 9 , 0 , 0 , 0  } }, // #141 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 79, 0 , 0 , 0 , 0 , 0  } }, // #142 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 54, { 35, 11, 3 , 3 , 0 , 0  } }, // #143 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 13, { 74, 43, 43, 43, 43, 5  } }, // #144 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 6 , { 35, 3 , 3 , 0 , 0 , 0  } }, // #145 [ref=17x]
  { InstDB::RWInfo::kCategoryGeneric   , 50, { 76, 77, 77, 0 , 0 , 0  } }, // #146 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 22, { 11, 3 , 3 , 0 , 0 , 0  } }, // #147 [ref=4x]
  { InstDB::RWInfo::kCategoryGeneric   , 7 , { 48, 5 , 0 , 0 , 0 , 0  } }, // #148 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 55, { 10, 5 , 40, 0 , 0 , 0  } }, // #149 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 56, { 10, 5 , 13, 0 , 0 , 0  } }, // #150 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 43, { 10, 5 , 5 , 5 , 0 , 0  } }, // #151 [ref=12x]
  { InstDB::RWInfo::kCategoryGeneric   , 61, { 10, 5 , 5 , 5 , 0 , 0  } }, // #152 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 62, { 10, 5 , 5 , 0 , 0 , 0  } }, // #153 [ref=12x]
  { InstDB::RWInfo::kCategoryGeneric   , 22, { 11, 3 , 5 , 0 , 0 , 0  } }, // #154 [ref=9x]
  { InstDB::RWInfo::kCategoryGeneric   , 63, { 11, 3 , 0 , 0 , 0 , 0  } }, // #155 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 59, 17, 29, 0 , 0 , 0  } }, // #156 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 8 , { 3 , 59, 17, 0 , 0 , 0  } }, // #157 [ref=4x]
  { InstDB::RWInfo::kCategoryGeneric   , 8 , { 11, 59, 17, 0 , 0 , 0  } }  // #158 [ref=8x]
};

const InstDB::RWInfoOp InstDB::rwInfoOp[] = {
  { 0x0000000000000000u, 0x0000000000000000u, 0xFF, { 0 }, 0 }, // #0 [ref=15433x]
  { 0x0000000000000003u, 0x0000000000000003u, 0x00, { 0 }, OpRWInfo::kRW | OpRWInfo::kRegPhysId }, // #1 [ref=10x]
  { 0x0000000000000000u, 0x0000000000000000u, 0xFF, { 0 }, OpRWInfo::kRW | OpRWInfo::kZExt }, // #2 [ref=217x]
  { 0x0000000000000000u, 0x0000000000000000u, 0xFF, { 0 }, OpRWInfo::kRead }, // #3 [ref=989x]
  { 0x000000000000FFFFu, 0x000000000000FFFFu, 0xFF, { 0 }, OpRWInfo::kRW | OpRWInfo::kZExt }, // #4 [ref=92x]
  { 0x000000000000FFFFu, 0x0000000000000000u, 0xFF, { 0 }, OpRWInfo::kRead }, // #5 [ref=305x]
  { 0x00000000000000FFu, 0x00000000000000FFu, 0xFF, { 0 }, OpRWInfo::kRW }, // #6 [ref=18x]
  { 0x00000000000000FFu, 0x0000000000000000u, 0xFF, { 0 }, OpRWInfo::kRead }, // #7 [ref=185x]
  { 0x000000000000000Fu, 0x000000000000000Fu, 0xFF, { 0 }, OpRWInfo::kRW }, // #8 [ref=18x]
  { 0x000000000000000Fu, 0x0000000000000000u, 0xFF, { 0 }, OpRWInfo::kRead }, // #9 [ref=133x]
  { 0x0000000000000000u, 0x000000000000FFFFu, 0xFF, { 0 }, OpRWInfo::kWrite | OpRWInfo::kZExt }, // #10 [ref=160x]
  { 0x0000000000000000u, 0x0000000000000000u, 0xFF, { 0 }, OpRWInfo::kWrite | OpRWInfo::kZExt }, // #11 [ref=420x]
  { 0x0000000000000003u, 0x0000000000000003u, 0xFF, { 0 }, OpRWInfo::kRW }, // #12 [ref=1x]
  { 0x0000000000000003u, 0x0000000000000000u, 0xFF, { 0 }, OpRWInfo::kRead }, // #13 [ref=34x]
  { 0x000000000000FFFFu, 0x0000000000000000u, 0x00, { 0 }, OpRWInfo::kRead | OpRWInfo::kRegPhysId }, // #14 [ref=4x]
  { 0x0000000000000000u, 0x0000000000000000u, 0xFF, { 0 }, OpRWInfo::kWrite | OpRWInfo::kZExt | OpRWInfo::kMemBaseWrite | OpRWInfo::kMemIndexWrite }, // #15 [ref=1x]
  { 0x0000000000000000u, 0x000000000000000Fu, 0x02, { 0 }, OpRWInfo::kWrite | OpRWInfo::kZExt | OpRWInfo::kRegPhysId }, // #16 [ref=9x]
  { 0x000000000000000Fu, 0x0000000000000000u, 0x00, { 0 }, OpRWInfo::kRead | OpRWInfo::kRegPhysId }, // #17 [ref=23x]
  { 0x00000000000000FFu, 0x00000000000000FFu, 0x00, { 0 }, OpRWInfo::kRW | OpRWInfo::kZExt | OpRWInfo::kRegPhysId }, // #18 [ref=2x]
  { 0x0000000000000000u, 0x0000000000000000u, 0x00, { 0 }, OpRWInfo::kRead | OpRWInfo::kMemPhysId }, // #19 [ref=3x]
  { 0x0000000000000000u, 0x0000000000000000u, 0x06, { 0 }, OpRWInfo::kRead | OpRWInfo::kMemBaseRW | OpRWInfo::kMemBasePostModify | OpRWInfo::kMemPhysId }, // #20 [ref=3x]
  { 0x0000000000000000u, 0x0000000000000000u, 0x07, { 0 }, OpRWInfo::kRead | OpRWInfo::kMemBaseRW | OpRWInfo::kMemBasePostModify | OpRWInfo::kMemPhysId }, // #21 [ref=2x]
  { 0x0000000000000000u, 0x0000000000000000u, 0x00, { 0 }, OpRWInfo::kRead | OpRWInfo::kRegPhysId }, // #22 [ref=7x]
  { 0x00000000000000FFu, 0x00000000000000FFu, 0x02, { 0 }, OpRWInfo::kRW | OpRWInfo::kZExt | OpRWInfo::kRegPhysId }, // #23 [ref=1x]
  { 0x00000000000000FFu, 0x0000000000000000u, 0x01, { 0 }, OpRWInfo::kRead | OpRWInfo::kRegPhysId }, // #24 [ref=1x]
  { 0x00000000000000FFu, 0x0000000000000000u, 0x03, { 0 }, OpRWInfo::kRead | OpRWInfo::kRegPhysId }, // #25 [ref=1x]
  { 0x00000000000000FFu, 0x00000000000000FFu, 0xFF, { 0 }, OpRWInfo::kRW | OpRWInfo::kZExt }, // #26 [ref=21x]
  { 0x000000000000000Fu, 0x000000000000000Fu, 0x02, { 0 }, OpRWInfo::kRW | OpRWInfo::kZExt | OpRWInfo::kRegPhysId }, // #27 [ref=1x]
  { 0x000000000000000Fu, 0x000000000000000Fu, 0x00, { 0 }, OpRWInfo::kRW | OpRWInfo::kZExt | OpRWInfo::kRegPhysId }, // #28 [ref=4x]
  { 0x000000000000000Fu, 0x0000000000000000u, 0x01, { 0 }, OpRWInfo::kRead | OpRWInfo::kRegPhysId }, // #29 [ref=13x]
  { 0x000000000000000Fu, 0x0000000000000000u, 0x03, { 0 }, OpRWInfo::kRead | OpRWInfo::kRegPhysId }, // #30 [ref=2x]
  { 0x0000000000000000u, 0x000000000000000Fu, 0x03, { 0 }, OpRWInfo::kWrite | OpRWInfo::kZExt | OpRWInfo::kRegPhysId }, // #31 [ref=1x]
  { 0x000000000000000Fu, 0x000000000000000Fu, 0x01, { 0 }, OpRWInfo::kRW | OpRWInfo::kZExt | OpRWInfo::kRegPhysId }, // #32 [ref=1x]
  { 0x0000000000000000u, 0x00000000000000FFu, 0x02, { 0 }, OpRWInfo::kWrite | OpRWInfo::kZExt | OpRWInfo::kRegPhysId }, // #33 [ref=1x]
  { 0x00000000000000FFu, 0x0000000000000000u, 0x00, { 0 }, OpRWInfo::kRead | OpRWInfo::kRegPhysId }, // #34 [ref=1x]
  { 0x0000000000000000u, 0x00000000000000FFu, 0xFF, { 0 }, OpRWInfo::kWrite | OpRWInfo::kZExt }, // #35 [ref=80x]
  { 0x0000000000000000u, 0x00000000000000FFu, 0xFF, { 0 }, OpRWInfo::kWrite }, // #36 [ref=6x]
  { 0x0000000000000000u, 0x000000000000000Fu, 0xFF, { 0 }, OpRWInfo::kWrite }, // #37 [ref=6x]
  { 0x0000000000000000u, 0x0000000000000003u, 0x02, { 0 }, OpRWInfo::kWrite | OpRWInfo::kRegPhysId }, // #38 [ref=1x]
  { 0x0000000000000003u, 0x0000000000000000u, 0x00, { 0 }, OpRWInfo::kRead | OpRWInfo::kRegPhysId }, // #39 [ref=1x]
  { 0x0000000000000001u, 0x0000000000000000u, 0xFF, { 0 }, OpRWInfo::kRead }, // #40 [ref=28x]
  { 0x0000000000000000u, 0x0000000000000000u, 0x02, { 0 }, OpRWInfo::kRW | OpRWInfo::kRegPhysId | OpRWInfo::kZExt }, // #41 [ref=2x]
  { 0x0000000000000000u, 0x0000000000000000u, 0x00, { 0 }, OpRWInfo::kRW | OpRWInfo::kRegPhysId | OpRWInfo::kZExt }, // #42 [ref=3x]
  { 0xFFFFFFFFFFFFFFFFu, 0x0000000000000000u, 0xFF, { 0 }, OpRWInfo::kRead }, // #43 [ref=45x]
  { 0x0000000000000000u, 0x000000000000000Fu, 0xFF, { 0 }, OpRWInfo::kWrite | OpRWInfo::kZExt }, // #44 [ref=30x]
  { 0x00000000000003FFu, 0x00000000000003FFu, 0xFF, { 0 }, OpRWInfo::kRW | OpRWInfo::kZExt }, // #45 [ref=22x]
  { 0x00000000000003FFu, 0x0000000000000000u, 0xFF, { 0 }, OpRWInfo::kRead }, // #46 [ref=13x]
  { 0x0000000000000000u, 0x00000000000003FFu, 0xFF, { 0 }, OpRWInfo::kWrite | OpRWInfo::kZExt }, // #47 [ref=1x]
  { 0x0000000000000000u, 0x0000000000000003u, 0xFF, { 0 }, OpRWInfo::kWrite | OpRWInfo::kZExt }, // #48 [ref=15x]
  { 0x0000000000000000u, 0x0000000000000003u, 0x00, { 0 }, OpRWInfo::kWrite | OpRWInfo::kRegPhysId | OpRWInfo::kZExt }, // #49 [ref=2x]
  { 0x0000000000000000u, 0x0000000000000000u, 0x00, { 0 }, OpRWInfo::kWrite | OpRWInfo::kRegPhysId | OpRWInfo::kZExt }, // #50 [ref=2x]
  { 0x0000000000000003u, 0x0000000000000000u, 0x02, { 0 }, OpRWInfo::kRead | OpRWInfo::kRegPhysId }, // #51 [ref=4x]
  { 0x000000000000000Fu, 0x000000000000000Fu, 0xFF, { 0 }, OpRWInfo::kRW | OpRWInfo::kZExt }, // #52 [ref=4x]
  { 0x0000000000000000u, 0x0000000000000000u, 0x07, { 0 }, OpRWInfo::kWrite | OpRWInfo::kZExt | OpRWInfo::kMemPhysId }, // #53 [ref=1x]
  { 0x0000000000000000u, 0x0000000000000000u, 0x01, { 0 }, OpRWInfo::kRead | OpRWInfo::kRegPhysId }, // #54 [ref=1x]
  { 0x0000000000000000u, 0x0000000000000001u, 0xFF, { 0 }, OpRWInfo::kWrite | OpRWInfo::kZExt }, // #55 [ref=14x]
  { 0x0000000000000000u, 0x0000000000000001u, 0x00, { 0 }, OpRWInfo::kWrite | OpRWInfo::kRegPhysId }, // #56 [ref=1x]
  { 0x0000000000000000u, 0x0000000000000000u, 0x01, { 0 }, OpRWInfo::kRW | OpRWInfo::kRegPhysId | OpRWInfo::kZExt }, // #57 [ref=3x]
  { 0x0000000000000000u, 0x0000000000000000u, 0x07, { 0 }, OpRWInfo::kRW | OpRWInfo::kZExt | OpRWInfo::kMemPhysId }, // #58 [ref=3x]
  { 0x000000000000000Fu, 0x0000000000000000u, 0x02, { 0 }, OpRWInfo::kRead | OpRWInfo::kRegPhysId }, // #59 [ref=22x]
  { 0x000000000000FF00u, 0x0000000000000000u, 0xFF, { 0 }, OpRWInfo::kRead }, // #60 [ref=23x]
  { 0x0000000000000000u, 0x000000000000FF00u, 0xFF, { 0 }, OpRWInfo::kWrite }, // #61 [ref=1x]
  { 0x0000000000000000u, 0x0000000000000000u, 0x07, { 0 }, OpRWInfo::kWrite | OpRWInfo::kZExt | OpRWInfo::kMemBaseRW | OpRWInfo::kMemBasePostModify | OpRWInfo::kMemPhysId }, // #62 [ref=2x]
  { 0x0000000000000000u, 0x0000000000000000u, 0x02, { 0 }, OpRWInfo::kWrite | OpRWInfo::kRegPhysId | OpRWInfo::kZExt }, // #63 [ref=1x]
  { 0x0000000000000000u, 0x0000000000000000u, 0x02, { 0 }, OpRWInfo::kRead | OpRWInfo::kRegPhysId }, // #64 [ref=2x]
  { 0x0000000000000000u, 0x0000000000000000u, 0x06, { 0 }, OpRWInfo::kRead | OpRWInfo::kMemPhysId }, // #65 [ref=1x]
  { 0x0000000000000000u, 0x000000000000000Fu, 0x01, { 0 }, OpRWInfo::kWrite | OpRWInfo::kZExt | OpRWInfo::kRegPhysId }, // #66 [ref=5x]
  { 0x0000000000000000u, 0x000000000000FFFFu, 0x00, { 0 }, OpRWInfo::kWrite | OpRWInfo::kZExt | OpRWInfo::kRegPhysId }, // #67 [ref=4x]
  { 0x0000000000000000u, 0x0000000000000007u, 0xFF, { 0 }, OpRWInfo::kWrite | OpRWInfo::kZExt }, // #68 [ref=2x]
  { 0x0000000000000000u, 0x0000000000000000u, 0x04, { 0 }, OpRWInfo::kWrite | OpRWInfo::kZExt | OpRWInfo::kRegPhysId }, // #69 [ref=1x]
  { 0x0000000000000001u, 0x0000000000000000u, 0x01, { 0 }, OpRWInfo::kRead | OpRWInfo::kRegPhysId }, // #70 [ref=10x]
  { 0x0000000000000000u, 0x000000000000000Fu, 0x00, { 0 }, OpRWInfo::kWrite | OpRWInfo::kZExt | OpRWInfo::kRegPhysId }, // #71 [ref=7x]
  { 0x0000000000000001u, 0x0000000000000000u, 0x00, { 0 }, OpRWInfo::kRead | OpRWInfo::kRegPhysId }, // #72 [ref=1x]
  { 0x0000000000000000u, 0x0000000000000001u, 0xFF, { 0 }, OpRWInfo::kWrite }, // #73 [ref=30x]
  { 0x0000000000000000u, 0xFFFFFFFFFFFFFFFFu, 0xFF, { 0 }, OpRWInfo::kWrite | OpRWInfo::kZExt }, // #74 [ref=20x]
  { 0xFFFFFFFFFFFFFFFFu, 0xFFFFFFFFFFFFFFFFu, 0xFF, { 0 }, OpRWInfo::kRW | OpRWInfo::kZExt }, // #75 [ref=7x]
  { 0x0000000000000000u, 0x00000000FFFFFFFFu, 0xFF, { 0 }, OpRWInfo::kWrite | OpRWInfo::kZExt }, // #76 [ref=10x]
  { 0x00000000FFFFFFFFu, 0x0000000000000000u, 0xFF, { 0 }, OpRWInfo::kRead }, // #77 [ref=16x]
  { 0x000000000000FFF0u, 0x0000000000000000u, 0xFF, { 0 }, OpRWInfo::kRead }, // #78 [ref=18x]
  { 0x0000000000000000u, 0x0000000000000000u, 0x00, { 0 }, OpRWInfo::kRW | OpRWInfo::kZExt | OpRWInfo::kRegPhysId }  // #79 [ref=1x]
};

const InstDB::RWInfoRm InstDB::rwInfoRm[] = {
  { InstDB::RWInfoRm::kCategoryNone      , 0x00, 0 , 0, 0 }, // #0 [ref=1882x]
  { InstDB::RWInfoRm::kCategoryConsistent, 0x03, 0 , InstDB::RWInfoRm::kFlagAmbiguous, 0 }, // #1 [ref=8x]
  { InstDB::RWInfoRm::kCategoryConsistent, 0x02, 0 , 0, 0 }, // #2 [ref=194x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x02, 16, 0, 0 }, // #3 [ref=122x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x02, 8 , 0, 0 }, // #4 [ref=66x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x02, 4 , 0, 0 }, // #5 [ref=33x]
  { InstDB::RWInfoRm::kCategoryConsistent, 0x04, 0 , 0, 0 }, // #6 [ref=270x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x01, 2 , 0, 0 }, // #7 [ref=9x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x00, 0 , 0, 0 }, // #8 [ref=63x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x03, 0 , 0, 0 }, // #9 [ref=1x]
  { InstDB::RWInfoRm::kCategoryConsistent, 0x01, 0 , InstDB::RWInfoRm::kFlagAmbiguous, 0 }, // #10 [ref=21x]
  { InstDB::RWInfoRm::kCategoryConsistent, 0x01, 0 , 0, 0 }, // #11 [ref=14x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x00, 8 , 0, 0 }, // #12 [ref=22x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x00, 16, 0, 0 }, // #13 [ref=21x]
  { InstDB::RWInfoRm::kCategoryConsistent, 0x02, 0 , InstDB::RWInfoRm::kFlagAmbiguous, 0 }, // #14 [ref=15x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x02, 1 , 0, 0 }, // #15 [ref=5x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x00, 64, 0, 0 }, // #16 [ref=5x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x01, 4 , 0, 0 }, // #17 [ref=8x]
  { InstDB::RWInfoRm::kCategoryNone      , 0x00, 0 , InstDB::RWInfoRm::kFlagAmbiguous, 0 }, // #18 [ref=22x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x00, 10, 0, 0 }, // #19 [ref=2x]
  { InstDB::RWInfoRm::kCategoryNone      , 0x01, 0 , InstDB::RWInfoRm::kFlagAmbiguous, 0 }, // #20 [ref=5x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x00, 2 , 0, 0 }, // #21 [ref=3x]
  { InstDB::RWInfoRm::kCategoryConsistent, 0x06, 0 , 0, 0 }, // #22 [ref=14x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x03, 1 , 0, 0 }, // #23 [ref=1x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x03, 4 , 0, 0 }, // #24 [ref=4x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x03, 8 , 0, 0 }, // #25 [ref=3x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x03, 2 , 0, 0 }, // #26 [ref=1x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x02, 2 , 0, 0 }, // #27 [ref=6x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x00, 4 , 0, 0 }, // #28 [ref=6x]
  { InstDB::RWInfoRm::kCategoryNone      , 0x03, 0 , InstDB::RWInfoRm::kFlagAmbiguous, 0 }, // #29 [ref=1x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x03, 16, 0, 0 }, // #30 [ref=6x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x01, 1 , 0, 0 }, // #31 [ref=32x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x01, 8 , 0, 0 }, // #32 [ref=4x]
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
  { InstDB::RWInfoRm::kCategoryFixed     , 0x04, 32, 0, 0 }, // #50 [ref=6x]
  { InstDB::RWInfoRm::kCategoryConsistent, 0x03, 0 , 0, 0 }, // #51 [ref=13x]
  { InstDB::RWInfoRm::kCategoryNone      , 0x02, 0 , 0, 0 }, // #52 [ref=1x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x03, 8 , InstDB::RWInfoRm::kFlagAmbiguous, 0 }, // #53 [ref=1x]
  { InstDB::RWInfoRm::kCategoryConsistent, 0x08, 0 , 0, 0 }, // #54 [ref=2x]
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
UNIT(x86_inst_db) {
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
