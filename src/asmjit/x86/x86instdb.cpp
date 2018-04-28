// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// ZLIB - See LICENSE.md file in the package.

// ----------------------------------------------------------------------------
// IMPORTANT: AsmJit now uses an external instruction database to populate
// static tables within this file. Perform the following steps to regenerate
// all tables enclosed by ${...}:
//
//   1. Install node.js environment <https://nodejs.org>
//   2. Go to asmjit/tools directory
//   3. Install either asmdb package by executing `npm install asmdb` or get
//      the latest asmdb from <https://github.com/asmjit/asmdb> and copy/link
//      the `asmdb` directory to `asmjit/tools/asmdb`.
//   4. Execute `node generate-x86.js`
//
// Instruction encoding and opcodes were added to the `x86inst.cpp` database
// manually in the past and they are not updated by the script as they seem
// consistent. However, everything else is updated including instruction
// operands and tables required to validate them, instruction read/write
// information (including registers and flags), and all indexes to all tables.
// ----------------------------------------------------------------------------

// [Export]
#define ASMJIT_EXPORTS

// [Guard]
#include "../core/build.h"
#ifdef ASMJIT_BUILD_X86

// [Dependencies]
#include "../core/cpuinfo.h"
#include "../core/stringutils.h"
#include "../core/misc_p.h"
#include "../x86/x86features.h"
#include "../x86/x86instdb.h"
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
#define INST(id, encoding, opcode0, opcode1, writeIndex, writeSize, nameDataIndex, commonDataIndex, operationDataIndex) { \
  uint32_t(NAME_DATA_INDEX(nameDataIndex)), \
  uint32_t(commonDataIndex),                \
  uint32_t(operationDataIndex)              \
}

const InstDB::InstInfo InstDB::_instInfoTable[] = {
  // <-----------------+--------------------+------------------+--------+------------------+--------+-------+-----+----+----+
  //                   |                    |    Main Opcode   |#0 EVEX |Alternative Opcode|#1 EVEX | Write |     |    |    |
  //    Instruction    |   Inst. Encoding   |                  +--------+                  +--------+---+---+NameX|ComX|OpnX|
  //                   |                    |#0:PP-MMM OP/O L|W|W|N|TT. |#1:PP-MMM OP/O L|W|W|N|TT. |Idx|Cnt|     |    |    |
  // <-----------------+--------------------+------------------+--------+------------------+--------+---+---+-----+----+----+
  //                                                                                                (automatically generated)
  // ${InstInfo:Begin}
  INST(None            , None               , 0                         , 0                         , 0 , 0 , 0   , 0  , 0  ), // #0
  INST(Aaa             , X86Op_xAX          , O(000000,37,_,_,_,_,_,_  ), 0                         , 0 , 0 , 1   , 1  , 1  ), // #1
  INST(Aad             , X86I_xAX           , O(000000,D5,_,_,_,_,_,_  ), 0                         , 0 , 0 , 5   , 2  , 1  ), // #2
  INST(Aam             , X86I_xAX           , O(000000,D4,_,_,_,_,_,_  ), 0                         , 0 , 0 , 9   , 2  , 1  ), // #3
  INST(Aas             , X86Op_xAX          , O(000000,3F,_,_,_,_,_,_  ), 0                         , 0 , 0 , 13  , 1  , 1  ), // #4
  INST(Adc             , X86Arith           , O(000000,10,2,_,x,_,_,_  ), 0                         , 0 , 0 , 17  , 3  , 2  ), // #5
  INST(Adcx            , X86Rm              , O(660F38,F6,_,_,x,_,_,_  ), 0                         , 0 , 0 , 21  , 4  , 3  ), // #6
  INST(Add             , X86Arith           , O(000000,00,0,_,x,_,_,_  ), 0                         , 0 , 0 , 732 , 3  , 1  ), // #7
  INST(Addpd           , ExtRm              , O(660F00,58,_,_,_,_,_,_  ), 0                         , 0 , 0 , 4656, 5  , 4  ), // #8
  INST(Addps           , ExtRm              , O(000F00,58,_,_,_,_,_,_  ), 0                         , 0 , 0 , 4668, 5  , 5  ), // #9
  INST(Addsd           , ExtRm              , O(F20F00,58,_,_,_,_,_,_  ), 0                         , 0 , 0 , 4890, 6  , 4  ), // #10
  INST(Addss           , ExtRm              , O(F30F00,58,_,_,_,_,_,_  ), 0                         , 0 , 0 , 2836, 7  , 5  ), // #11
  INST(Addsubpd        , ExtRm              , O(660F00,D0,_,_,_,_,_,_  ), 0                         , 0 , 0 , 4395, 5  , 6  ), // #12
  INST(Addsubps        , ExtRm              , O(F20F00,D0,_,_,_,_,_,_  ), 0                         , 0 , 0 , 4407, 5  , 6  ), // #13
  INST(Adox            , X86Rm              , O(F30F38,F6,_,_,x,_,_,_  ), 0                         , 0 , 0 , 26  , 4  , 7  ), // #14
  INST(Aesdec          , ExtRm              , O(660F38,DE,_,_,_,_,_,_  ), 0                         , 0 , 0 , 2891, 5  , 8  ), // #15
  INST(Aesdeclast      , ExtRm              , O(660F38,DF,_,_,_,_,_,_  ), 0                         , 0 , 0 , 2899, 5  , 8  ), // #16
  INST(Aesenc          , ExtRm              , O(660F38,DC,_,_,_,_,_,_  ), 0                         , 0 , 0 , 2911, 5  , 8  ), // #17
  INST(Aesenclast      , ExtRm              , O(660F38,DD,_,_,_,_,_,_  ), 0                         , 0 , 0 , 2919, 5  , 8  ), // #18
  INST(Aesimc          , ExtRm              , O(660F38,DB,_,_,_,_,_,_  ), 0                         , 0 , 0 , 2931, 8  , 8  ), // #19
  INST(Aeskeygenassist , ExtRmi             , O(660F3A,DF,_,_,_,_,_,_  ), 0                         , 0 , 0 , 2939, 9  , 8  ), // #20
  INST(And             , X86Arith           , O(000000,20,4,_,x,_,_,_  ), 0                         , 0 , 0 , 2333, 10 , 1  ), // #21
  INST(Andn            , VexRvm_Wx          , V(000F38,F2,_,0,x,_,_,_  ), 0                         , 0 , 0 , 6187, 11 , 9  ), // #22
  INST(Andnpd          , ExtRm              , O(660F00,55,_,_,_,_,_,_  ), 0                         , 0 , 0 , 2972, 5  , 4  ), // #23
  INST(Andnps          , ExtRm              , O(000F00,55,_,_,_,_,_,_  ), 0                         , 0 , 0 , 2980, 5  , 5  ), // #24
  INST(Andpd           , ExtRm              , O(660F00,54,_,_,_,_,_,_  ), 0                         , 0 , 0 , 3909, 12 , 4  ), // #25
  INST(Andps           , ExtRm              , O(000F00,54,_,_,_,_,_,_  ), 0                         , 0 , 0 , 3919, 12 , 5  ), // #26
  INST(Arpl            , X86Mr_NoSize       , O(000000,63,_,_,_,_,_,_  ), 0                         , 0 , 0 , 31  , 13 , 10 ), // #27
  INST(Bextr           , VexRmv_Wx          , V(000F38,F7,_,0,x,_,_,_  ), 0                         , 0 , 0 , 36  , 14 , 9  ), // #28
  INST(Blcfill         , VexVm_Wx           , V(XOP_M9,01,1,0,x,_,_,_  ), 0                         , 0 , 0 , 42  , 15 , 11 ), // #29
  INST(Blci            , VexVm_Wx           , V(XOP_M9,02,6,0,x,_,_,_  ), 0                         , 0 , 0 , 50  , 15 , 11 ), // #30
  INST(Blcic           , VexVm_Wx           , V(XOP_M9,01,5,0,x,_,_,_  ), 0                         , 0 , 0 , 55  , 15 , 11 ), // #31
  INST(Blcmsk          , VexVm_Wx           , V(XOP_M9,02,1,0,x,_,_,_  ), 0                         , 0 , 0 , 61  , 15 , 11 ), // #32
  INST(Blcs            , VexVm_Wx           , V(XOP_M9,01,3,0,x,_,_,_  ), 0                         , 0 , 0 , 68  , 15 , 11 ), // #33
  INST(Blendpd         , ExtRmi             , O(660F3A,0D,_,_,_,_,_,_  ), 0                         , 0 , 0 , 3058, 16 , 12 ), // #34
  INST(Blendps         , ExtRmi             , O(660F3A,0C,_,_,_,_,_,_  ), 0                         , 0 , 0 , 3067, 16 , 12 ), // #35
  INST(Blendvpd        , ExtRm_XMM0         , O(660F38,15,_,_,_,_,_,_  ), 0                         , 0 , 0 , 3076, 17 , 12 ), // #36
  INST(Blendvps        , ExtRm_XMM0         , O(660F38,14,_,_,_,_,_,_  ), 0                         , 0 , 0 , 3086, 17 , 12 ), // #37
  INST(Blsfill         , VexVm_Wx           , V(XOP_M9,01,2,0,x,_,_,_  ), 0                         , 0 , 0 , 73  , 15 , 11 ), // #38
  INST(Blsi            , VexVm_Wx           , V(000F38,F3,3,0,x,_,_,_  ), 0                         , 0 , 0 , 81  , 15 , 9  ), // #39
  INST(Blsic           , VexVm_Wx           , V(XOP_M9,01,6,0,x,_,_,_  ), 0                         , 0 , 0 , 86  , 15 , 11 ), // #40
  INST(Blsmsk          , VexVm_Wx           , V(000F38,F3,2,0,x,_,_,_  ), 0                         , 0 , 0 , 92  , 15 , 9  ), // #41
  INST(Blsr            , VexVm_Wx           , V(000F38,F3,1,0,x,_,_,_  ), 0                         , 0 , 0 , 99  , 15 , 9  ), // #42
  INST(Bndcl           , X86Rm              , O(F30F00,1A,_,_,_,_,_,_  ), 0                         , 0 , 0 , 104 , 18 , 13 ), // #43
  INST(Bndcn           , X86Rm              , O(F20F00,1B,_,_,_,_,_,_  ), 0                         , 0 , 0 , 110 , 18 , 13 ), // #44
  INST(Bndcu           , X86Rm              , O(F20F00,1A,_,_,_,_,_,_  ), 0                         , 0 , 0 , 116 , 18 , 13 ), // #45
  INST(Bndldx          , X86Rm              , O(000F00,1A,_,_,_,_,_,_  ), 0                         , 0 , 0 , 122 , 19 , 13 ), // #46
  INST(Bndmk           , X86Rm              , O(F30F00,1B,_,_,_,_,_,_  ), 0                         , 0 , 0 , 129 , 20 , 13 ), // #47
  INST(Bndmov          , X86Bndmov          , O(660F00,1A,_,_,_,_,_,_  ), O(660F00,1B,_,_,_,_,_,_  ), 0 , 0 , 135 , 21 , 13 ), // #48
  INST(Bndstx          , X86Mr              , O(000F00,1B,_,_,_,_,_,_  ), 0                         , 0 , 0 , 142 , 22 , 13 ), // #49
  INST(Bound           , X86Rm              , O(000000,62,_,_,_,_,_,_  ), 0                         , 0 , 0 , 149 , 23 , 0  ), // #50
  INST(Bsf             , X86Rm              , O(000F00,BC,_,_,x,_,_,_  ), 0                         , 0 , 0 , 155 , 24 , 1  ), // #51
  INST(Bsr             , X86Rm              , O(000F00,BD,_,_,x,_,_,_  ), 0                         , 0 , 0 , 159 , 24 , 1  ), // #52
  INST(Bswap           , X86Bswap           , O(000F00,C8,_,_,x,_,_,_  ), 0                         , 0 , 0 , 163 , 25 , 0  ), // #53
  INST(Bt              , X86Bt              , O(000F00,A3,_,_,x,_,_,_  ), O(000F00,BA,4,_,x,_,_,_  ), 0 , 0 , 169 , 26 , 14 ), // #54
  INST(Btc             , X86Bt              , O(000F00,BB,_,_,x,_,_,_  ), O(000F00,BA,7,_,x,_,_,_  ), 0 , 0 , 172 , 27 , 15 ), // #55
  INST(Btr             , X86Bt              , O(000F00,B3,_,_,x,_,_,_  ), O(000F00,BA,6,_,x,_,_,_  ), 0 , 0 , 176 , 27 , 15 ), // #56
  INST(Bts             , X86Bt              , O(000F00,AB,_,_,x,_,_,_  ), O(000F00,BA,5,_,x,_,_,_  ), 0 , 0 , 180 , 27 , 15 ), // #57
  INST(Bzhi            , VexRmv_Wx          , V(000F38,F5,_,0,x,_,_,_  ), 0                         , 0 , 0 , 184 , 14 , 16 ), // #58
  INST(Call            , X86Call            , O(000000,FF,2,_,_,_,_,_  ), 0                         , 0 , 0 , 2729, 28 , 1  ), // #59
  INST(Cbw             , X86Op_xAX          , O(660000,98,_,_,_,_,_,_  ), 0                         , 0 , 0 , 189 , 29 , 0  ), // #60
  INST(Cdq             , X86Op_xDX_xAX      , O(000000,99,_,_,_,_,_,_  ), 0                         , 0 , 0 , 193 , 30 , 0  ), // #61
  INST(Cdqe            , X86Op_xAX          , O(000000,98,_,_,1,_,_,_  ), 0                         , 0 , 0 , 197 , 31 , 0  ), // #62
  INST(Clac            , X86Op              , O(000F01,CA,_,_,_,_,_,_  ), 0                         , 0 , 0 , 202 , 32 , 17 ), // #63
  INST(Clc             , X86Op              , O(000000,F8,_,_,_,_,_,_  ), 0                         , 0 , 0 , 207 , 33 , 18 ), // #64
  INST(Cld             , X86Op              , O(000000,FC,_,_,_,_,_,_  ), 0                         , 0 , 0 , 211 , 33 , 19 ), // #65
  INST(Clflush         , X86M_Only          , O(000F00,AE,7,_,_,_,_,_  ), 0                         , 0 , 0 , 215 , 34 , 20 ), // #66
  INST(Clflushopt      , X86M_Only          , O(660F00,AE,7,_,_,_,_,_  ), 0                         , 0 , 0 , 223 , 34 , 21 ), // #67
  INST(Cli             , X86Op              , O(000000,FA,_,_,_,_,_,_  ), 0                         , 0 , 0 , 234 , 32 , 22 ), // #68
  INST(Clts            , X86Op              , O(000F00,06,_,_,_,_,_,_  ), 0                         , 0 , 0 , 238 , 35 , 0  ), // #69
  INST(Clwb            , X86M_Only          , O(660F00,AE,6,_,_,_,_,_  ), 0                         , 0 , 0 , 243 , 34 , 23 ), // #70
  INST(Clzero          , X86Op_ZAX          , O(000F01,FC,_,_,_,_,_,_  ), 0                         , 0 , 0 , 248 , 36 , 24 ), // #71
  INST(Cmc             , X86Op              , O(000000,F5,_,_,_,_,_,_  ), 0                         , 0 , 0 , 255 , 33 , 25 ), // #72
  INST(Cmova           , X86Rm              , O(000F00,47,_,_,x,_,_,_  ), 0                         , 0 , 0 , 259 , 37 , 26 ), // #73
  INST(Cmovae          , X86Rm              , O(000F00,43,_,_,x,_,_,_  ), 0                         , 0 , 0 , 265 , 37 , 27 ), // #74
  INST(Cmovb           , X86Rm              , O(000F00,42,_,_,x,_,_,_  ), 0                         , 0 , 0 , 589 , 37 , 27 ), // #75
  INST(Cmovbe          , X86Rm              , O(000F00,46,_,_,x,_,_,_  ), 0                         , 0 , 0 , 596 , 37 , 26 ), // #76
  INST(Cmovc           , X86Rm              , O(000F00,42,_,_,x,_,_,_  ), 0                         , 0 , 0 , 272 , 37 , 27 ), // #77
  INST(Cmove           , X86Rm              , O(000F00,44,_,_,x,_,_,_  ), 0                         , 0 , 0 , 604 , 37 , 28 ), // #78
  INST(Cmovg           , X86Rm              , O(000F00,4F,_,_,x,_,_,_  ), 0                         , 0 , 0 , 278 , 37 , 29 ), // #79
  INST(Cmovge          , X86Rm              , O(000F00,4D,_,_,x,_,_,_  ), 0                         , 0 , 0 , 284 , 37 , 30 ), // #80
  INST(Cmovl           , X86Rm              , O(000F00,4C,_,_,x,_,_,_  ), 0                         , 0 , 0 , 291 , 37 , 30 ), // #81
  INST(Cmovle          , X86Rm              , O(000F00,4E,_,_,x,_,_,_  ), 0                         , 0 , 0 , 297 , 37 , 29 ), // #82
  INST(Cmovna          , X86Rm              , O(000F00,46,_,_,x,_,_,_  ), 0                         , 0 , 0 , 304 , 37 , 26 ), // #83
  INST(Cmovnae         , X86Rm              , O(000F00,42,_,_,x,_,_,_  ), 0                         , 0 , 0 , 311 , 37 , 27 ), // #84
  INST(Cmovnb          , X86Rm              , O(000F00,43,_,_,x,_,_,_  ), 0                         , 0 , 0 , 611 , 37 , 27 ), // #85
  INST(Cmovnbe         , X86Rm              , O(000F00,47,_,_,x,_,_,_  ), 0                         , 0 , 0 , 619 , 37 , 26 ), // #86
  INST(Cmovnc          , X86Rm              , O(000F00,43,_,_,x,_,_,_  ), 0                         , 0 , 0 , 319 , 37 , 27 ), // #87
  INST(Cmovne          , X86Rm              , O(000F00,45,_,_,x,_,_,_  ), 0                         , 0 , 0 , 628 , 37 , 28 ), // #88
  INST(Cmovng          , X86Rm              , O(000F00,4E,_,_,x,_,_,_  ), 0                         , 0 , 0 , 326 , 37 , 29 ), // #89
  INST(Cmovnge         , X86Rm              , O(000F00,4C,_,_,x,_,_,_  ), 0                         , 0 , 0 , 333 , 37 , 30 ), // #90
  INST(Cmovnl          , X86Rm              , O(000F00,4D,_,_,x,_,_,_  ), 0                         , 0 , 0 , 341 , 37 , 30 ), // #91
  INST(Cmovnle         , X86Rm              , O(000F00,4F,_,_,x,_,_,_  ), 0                         , 0 , 0 , 348 , 37 , 29 ), // #92
  INST(Cmovno          , X86Rm              , O(000F00,41,_,_,x,_,_,_  ), 0                         , 0 , 0 , 356 , 37 , 31 ), // #93
  INST(Cmovnp          , X86Rm              , O(000F00,4B,_,_,x,_,_,_  ), 0                         , 0 , 0 , 363 , 37 , 32 ), // #94
  INST(Cmovns          , X86Rm              , O(000F00,49,_,_,x,_,_,_  ), 0                         , 0 , 0 , 370 , 37 , 33 ), // #95
  INST(Cmovnz          , X86Rm              , O(000F00,45,_,_,x,_,_,_  ), 0                         , 0 , 0 , 377 , 37 , 28 ), // #96
  INST(Cmovo           , X86Rm              , O(000F00,40,_,_,x,_,_,_  ), 0                         , 0 , 0 , 384 , 37 , 31 ), // #97
  INST(Cmovp           , X86Rm              , O(000F00,4A,_,_,x,_,_,_  ), 0                         , 0 , 0 , 390 , 37 , 32 ), // #98
  INST(Cmovpe          , X86Rm              , O(000F00,4A,_,_,x,_,_,_  ), 0                         , 0 , 0 , 396 , 37 , 32 ), // #99
  INST(Cmovpo          , X86Rm              , O(000F00,4B,_,_,x,_,_,_  ), 0                         , 0 , 0 , 403 , 37 , 32 ), // #100
  INST(Cmovs           , X86Rm              , O(000F00,48,_,_,x,_,_,_  ), 0                         , 0 , 0 , 410 , 37 , 33 ), // #101
  INST(Cmovz           , X86Rm              , O(000F00,44,_,_,x,_,_,_  ), 0                         , 0 , 0 , 416 , 37 , 28 ), // #102
  INST(Cmp             , X86Arith           , O(000000,38,7,_,x,_,_,_  ), 0                         , 0 , 0 , 422 , 38 , 1  ), // #103
  INST(Cmppd           , ExtRmi             , O(660F00,C2,_,_,_,_,_,_  ), 0                         , 0 , 0 , 3312, 16 , 4  ), // #104
  INST(Cmpps           , ExtRmi             , O(000F00,C2,_,_,_,_,_,_  ), 0                         , 0 , 0 , 3319, 16 , 5  ), // #105
  INST(Cmps            , X86StrMm           , O(000000,A6,_,_,_,_,_,_  ), 0                         , 0 , 0 , 426 , 39 , 34 ), // #106
  INST(Cmpsd           , ExtRmi             , O(F20F00,C2,_,_,_,_,_,_  ), 0                         , 0 , 0 , 3326, 40 , 4  ), // #107
  INST(Cmpss           , ExtRmi             , O(F30F00,C2,_,_,_,_,_,_  ), 0                         , 0 , 0 , 3333, 41 , 5  ), // #108
  INST(Cmpxchg         , X86Cmpxchg         , O(000F00,B0,_,_,x,_,_,_  ), 0                         , 0 , 0 , 431 , 42 , 35 ), // #109
  INST(Cmpxchg16b      , X86Cmpxchg8b_16b   , O(000F00,C7,1,_,1,_,_,_  ), 0                         , 0 , 0 , 439 , 43 , 36 ), // #110
  INST(Cmpxchg8b       , X86Cmpxchg8b_16b   , O(000F00,C7,1,_,_,_,_,_  ), 0                         , 0 , 0 , 450 , 44 , 37 ), // #111
  INST(Comisd          , ExtRm              , O(660F00,2F,_,_,_,_,_,_  ), 0                         , 0 , 0 , 9585, 45 , 38 ), // #112
  INST(Comiss          , ExtRm              , O(000F00,2F,_,_,_,_,_,_  ), 0                         , 0 , 0 , 9594, 46 , 39 ), // #113
  INST(Cpuid           , X86Op              , O(000F00,A2,_,_,_,_,_,_  ), 0                         , 0 , 0 , 460 , 47 , 40 ), // #114
  INST(Cqo             , X86Op_xDX_xAX      , O(000000,99,_,_,1,_,_,_  ), 0                         , 0 , 0 , 466 , 48 , 0  ), // #115
  INST(Crc32           , X86Crc             , O(F20F38,F0,_,_,x,_,_,_  ), 0                         , 0 , 0 , 470 , 49 , 41 ), // #116
  INST(Cvtdq2pd        , ExtRm              , O(F30F00,E6,_,_,_,_,_,_  ), 0                         , 0 , 16, 3380, 50 , 4  ), // #117
  INST(Cvtdq2ps        , ExtRm              , O(000F00,5B,_,_,_,_,_,_  ), 0                         , 0 , 16, 3390, 51 , 4  ), // #118
  INST(Cvtpd2dq        , ExtRm              , O(F20F00,E6,_,_,_,_,_,_  ), 0                         , 0 , 16, 3400, 51 , 4  ), // #119
  INST(Cvtpd2pi        , ExtRm              , O(660F00,2D,_,_,_,_,_,_  ), 0                         , 0 , 8 , 476 , 52 , 4  ), // #120
  INST(Cvtpd2ps        , ExtRm              , O(660F00,5A,_,_,_,_,_,_  ), 0                         , 0 , 16, 3410, 51 , 4  ), // #121
  INST(Cvtpi2pd        , ExtRm              , O(660F00,2A,_,_,_,_,_,_  ), 0                         , 0 , 16, 485 , 53 , 4  ), // #122
  INST(Cvtpi2ps        , ExtRm              , O(000F00,2A,_,_,_,_,_,_  ), 0                         , 0 , 8 , 494 , 54 , 5  ), // #123
  INST(Cvtps2dq        , ExtRm              , O(660F00,5B,_,_,_,_,_,_  ), 0                         , 0 , 16, 3462, 51 , 4  ), // #124
  INST(Cvtps2pd        , ExtRm              , O(000F00,5A,_,_,_,_,_,_  ), 0                         , 0 , 16, 3472, 50 , 4  ), // #125
  INST(Cvtps2pi        , ExtRm              , O(000F00,2D,_,_,_,_,_,_  ), 0                         , 0 , 8 , 503 , 55 , 5  ), // #126
  INST(Cvtsd2si        , ExtRm_Wx           , O(F20F00,2D,_,_,x,_,_,_  ), 0                         , 0 , 8 , 3544, 56 , 4  ), // #127
  INST(Cvtsd2ss        , ExtRm              , O(F20F00,5A,_,_,_,_,_,_  ), 0                         , 0 , 4 , 3554, 57 , 4  ), // #128
  INST(Cvtsi2sd        , ExtRm_Wx           , O(F20F00,2A,_,_,x,_,_,_  ), 0                         , 0 , 8 , 3575, 58 , 4  ), // #129
  INST(Cvtsi2ss        , ExtRm_Wx           , O(F30F00,2A,_,_,x,_,_,_  ), 0                         , 0 , 4 , 3585, 59 , 5  ), // #130
  INST(Cvtss2sd        , ExtRm              , O(F30F00,5A,_,_,_,_,_,_  ), 0                         , 0 , 8 , 3595, 60 , 4  ), // #131
  INST(Cvtss2si        , ExtRm_Wx           , O(F30F00,2D,_,_,x,_,_,_  ), 0                         , 0 , 8 , 3605, 61 , 5  ), // #132
  INST(Cvttpd2dq       , ExtRm              , O(660F00,E6,_,_,_,_,_,_  ), 0                         , 0 , 16, 3626, 51 , 4  ), // #133
  INST(Cvttpd2pi       , ExtRm              , O(660F00,2C,_,_,_,_,_,_  ), 0                         , 0 , 8 , 512 , 52 , 4  ), // #134
  INST(Cvttps2dq       , ExtRm              , O(F30F00,5B,_,_,_,_,_,_  ), 0                         , 0 , 16, 3672, 51 , 4  ), // #135
  INST(Cvttps2pi       , ExtRm              , O(000F00,2C,_,_,_,_,_,_  ), 0                         , 0 , 8 , 522 , 55 , 5  ), // #136
  INST(Cvttsd2si       , ExtRm_Wx           , O(F20F00,2C,_,_,x,_,_,_  ), 0                         , 0 , 8 , 3718, 56 , 4  ), // #137
  INST(Cvttss2si       , ExtRm_Wx           , O(F30F00,2C,_,_,x,_,_,_  ), 0                         , 0 , 8 , 3741, 61 , 5  ), // #138
  INST(Cwd             , X86Op_xDX_xAX      , O(660000,99,_,_,_,_,_,_  ), 0                         , 0 , 0 , 532 , 62 , 0  ), // #139
  INST(Cwde            , X86Op_xAX          , O(000000,98,_,_,_,_,_,_  ), 0                         , 0 , 0 , 536 , 63 , 0  ), // #140
  INST(Daa             , X86Op              , O(000000,27,_,_,_,_,_,_  ), 0                         , 0 , 0 , 541 , 1  , 1  ), // #141
  INST(Das             , X86Op              , O(000000,2F,_,_,_,_,_,_  ), 0                         , 0 , 0 , 545 , 1  , 1  ), // #142
  INST(Dec             , X86IncDec          , O(000000,FE,1,_,x,_,_,_  ), O(000000,48,_,_,x,_,_,_  ), 0 , 0 , 2894, 64 , 42 ), // #143
  INST(Div             , X86M_GPB_MulDiv    , O(000000,F6,6,_,x,_,_,_  ), 0                         , 0 , 0 , 751 , 65 , 1  ), // #144
  INST(Divpd           , ExtRm              , O(660F00,5E,_,_,_,_,_,_  ), 0                         , 0 , 0 , 3840, 5  , 4  ), // #145
  INST(Divps           , ExtRm              , O(000F00,5E,_,_,_,_,_,_  ), 0                         , 0 , 0 , 3847, 5  , 5  ), // #146
  INST(Divsd           , ExtRm              , O(F20F00,5E,_,_,_,_,_,_  ), 0                         , 0 , 0 , 3854, 6  , 4  ), // #147
  INST(Divss           , ExtRm              , O(F30F00,5E,_,_,_,_,_,_  ), 0                         , 0 , 0 , 3861, 7  , 5  ), // #148
  INST(Dppd            , ExtRmi             , O(660F3A,41,_,_,_,_,_,_  ), 0                         , 0 , 0 , 3868, 16 , 12 ), // #149
  INST(Dpps            , ExtRmi             , O(660F3A,40,_,_,_,_,_,_  ), 0                         , 0 , 0 , 3874, 16 , 12 ), // #150
  INST(Emms            , X86Op              , O(000F00,77,_,_,_,_,_,_  ), 0                         , 0 , 0 , 719 , 66 , 43 ), // #151
  INST(Enter           , X86Enter           , O(000000,C8,_,_,_,_,_,_  ), 0                         , 0 , 0 , 2737, 67 , 0  ), // #152
  INST(Extractps       , ExtExtract         , O(660F3A,17,_,_,_,_,_,_  ), 0                         , 0 , 8 , 4064, 68 , 12 ), // #153
  INST(Extrq           , ExtExtrq           , O(660F00,79,_,_,_,_,_,_  ), O(660F00,78,0,_,_,_,_,_  ), 0 , 0 , 6945, 69 , 44 ), // #154
  INST(F2xm1           , FpuOp              , O_FPU(00,D9F0,_)          , 0                         , 0 , 0 , 549 , 33 , 45 ), // #155
  INST(Fabs            , FpuOp              , O_FPU(00,D9E1,_)          , 0                         , 0 , 0 , 555 , 33 , 45 ), // #156
  INST(Fadd            , FpuArith           , O_FPU(00,C0C0,0)          , 0                         , 0 , 0 , 1973, 70 , 45 ), // #157
  INST(Faddp           , FpuRDef            , O_FPU(00,DEC0,_)          , 0                         , 0 , 0 , 560 , 71 , 45 ), // #158
  INST(Fbld            , X86M_Only          , O_FPU(00,00DF,4)          , 0                         , 0 , 0 , 566 , 72 , 45 ), // #159
  INST(Fbstp           , X86M_Only          , O_FPU(00,00DF,6)          , 0                         , 0 , 0 , 571 , 73 , 45 ), // #160
  INST(Fchs            , FpuOp              , O_FPU(00,D9E0,_)          , 0                         , 0 , 0 , 577 , 33 , 45 ), // #161
  INST(Fclex           , FpuOp              , O_FPU(9B,DBE2,_)          , 0                         , 0 , 0 , 582 , 33 , 45 ), // #162
  INST(Fcmovb          , FpuR               , O_FPU(00,DAC0,_)          , 0                         , 0 , 0 , 588 , 74 , 46 ), // #163
  INST(Fcmovbe         , FpuR               , O_FPU(00,DAD0,_)          , 0                         , 0 , 0 , 595 , 74 , 46 ), // #164
  INST(Fcmove          , FpuR               , O_FPU(00,DAC8,_)          , 0                         , 0 , 0 , 603 , 74 , 46 ), // #165
  INST(Fcmovnb         , FpuR               , O_FPU(00,DBC0,_)          , 0                         , 0 , 0 , 610 , 74 , 46 ), // #166
  INST(Fcmovnbe        , FpuR               , O_FPU(00,DBD0,_)          , 0                         , 0 , 0 , 618 , 74 , 46 ), // #167
  INST(Fcmovne         , FpuR               , O_FPU(00,DBC8,_)          , 0                         , 0 , 0 , 627 , 74 , 46 ), // #168
  INST(Fcmovnu         , FpuR               , O_FPU(00,DBD8,_)          , 0                         , 0 , 0 , 635 , 74 , 46 ), // #169
  INST(Fcmovu          , FpuR               , O_FPU(00,DAD8,_)          , 0                         , 0 , 0 , 643 , 74 , 46 ), // #170
  INST(Fcom            , FpuCom             , O_FPU(00,D0D0,2)          , 0                         , 0 , 0 , 650 , 75 , 45 ), // #171
  INST(Fcomi           , FpuR               , O_FPU(00,DBF0,_)          , 0                         , 0 , 0 , 655 , 76 , 47 ), // #172
  INST(Fcomip          , FpuR               , O_FPU(00,DFF0,_)          , 0                         , 0 , 0 , 661 , 76 , 47 ), // #173
  INST(Fcomp           , FpuCom             , O_FPU(00,D8D8,3)          , 0                         , 0 , 0 , 668 , 75 , 45 ), // #174
  INST(Fcompp          , FpuOp              , O_FPU(00,DED9,_)          , 0                         , 0 , 0 , 674 , 33 , 45 ), // #175
  INST(Fcos            , FpuOp              , O_FPU(00,D9FF,_)          , 0                         , 0 , 0 , 681 , 33 , 45 ), // #176
  INST(Fdecstp         , FpuOp              , O_FPU(00,D9F6,_)          , 0                         , 0 , 0 , 686 , 33 , 45 ), // #177
  INST(Fdiv            , FpuArith           , O_FPU(00,F0F8,6)          , 0                         , 0 , 0 , 694 , 70 , 45 ), // #178
  INST(Fdivp           , FpuRDef            , O_FPU(00,DEF8,_)          , 0                         , 0 , 0 , 699 , 71 , 45 ), // #179
  INST(Fdivr           , FpuArith           , O_FPU(00,F8F0,7)          , 0                         , 0 , 0 , 705 , 70 , 45 ), // #180
  INST(Fdivrp          , FpuRDef            , O_FPU(00,DEF0,_)          , 0                         , 0 , 0 , 711 , 71 , 45 ), // #181
  INST(Femms           , X86Op              , O(000F00,0E,_,_,_,_,_,_  ), 0                         , 0 , 0 , 718 , 32 , 48 ), // #182
  INST(Ffree           , FpuR               , O_FPU(00,DDC0,_)          , 0                         , 0 , 0 , 724 , 74 , 45 ), // #183
  INST(Fiadd           , FpuM               , O_FPU(00,00DA,0)          , 0                         , 0 , 0 , 730 , 77 , 45 ), // #184
  INST(Ficom           , FpuM               , O_FPU(00,00DA,2)          , 0                         , 0 , 0 , 736 , 77 , 45 ), // #185
  INST(Ficomp          , FpuM               , O_FPU(00,00DA,3)          , 0                         , 0 , 0 , 742 , 77 , 45 ), // #186
  INST(Fidiv           , FpuM               , O_FPU(00,00DA,6)          , 0                         , 0 , 0 , 749 , 77 , 45 ), // #187
  INST(Fidivr          , FpuM               , O_FPU(00,00DA,7)          , 0                         , 0 , 0 , 755 , 77 , 45 ), // #188
  INST(Fild            , FpuM               , O_FPU(00,00DB,0)          , O_FPU(00,00DF,5)          , 0 , 0 , 762 , 78 , 45 ), // #189
  INST(Fimul           , FpuM               , O_FPU(00,00DA,1)          , 0                         , 0 , 0 , 767 , 77 , 45 ), // #190
  INST(Fincstp         , FpuOp              , O_FPU(00,D9F7,_)          , 0                         , 0 , 0 , 773 , 33 , 45 ), // #191
  INST(Finit           , FpuOp              , O_FPU(9B,DBE3,_)          , 0                         , 0 , 0 , 781 , 33 , 45 ), // #192
  INST(Fist            , FpuM               , O_FPU(00,00DB,2)          , 0                         , 0 , 0 , 787 , 79 , 45 ), // #193
  INST(Fistp           , FpuM               , O_FPU(00,00DB,3)          , O_FPU(00,00DF,7)          , 0 , 0 , 792 , 80 , 45 ), // #194
  INST(Fisttp          , FpuM               , O_FPU(00,00DB,1)          , O_FPU(00,00DD,1)          , 0 , 0 , 798 , 80 , 49 ), // #195
  INST(Fisub           , FpuM               , O_FPU(00,00DA,4)          , 0                         , 0 , 0 , 805 , 77 , 45 ), // #196
  INST(Fisubr          , FpuM               , O_FPU(00,00DA,5)          , 0                         , 0 , 0 , 811 , 77 , 45 ), // #197
  INST(Fld             , FpuFldFst          , O_FPU(00,00D9,0)          , O_FPU(00,00DB,5)          , 0 , 0 , 818 , 81 , 45 ), // #198
  INST(Fld1            , FpuOp              , O_FPU(00,D9E8,_)          , 0                         , 0 , 0 , 822 , 33 , 45 ), // #199
  INST(Fldcw           , X86M_Only          , O_FPU(00,00D9,5)          , 0                         , 0 , 0 , 827 , 82 , 45 ), // #200
  INST(Fldenv          , X86M_Only          , O_FPU(00,00D9,4)          , 0                         , 0 , 0 , 833 , 83 , 45 ), // #201
  INST(Fldl2e          , FpuOp              , O_FPU(00,D9EA,_)          , 0                         , 0 , 0 , 840 , 33 , 45 ), // #202
  INST(Fldl2t          , FpuOp              , O_FPU(00,D9E9,_)          , 0                         , 0 , 0 , 847 , 33 , 45 ), // #203
  INST(Fldlg2          , FpuOp              , O_FPU(00,D9EC,_)          , 0                         , 0 , 0 , 854 , 33 , 45 ), // #204
  INST(Fldln2          , FpuOp              , O_FPU(00,D9ED,_)          , 0                         , 0 , 0 , 861 , 33 , 45 ), // #205
  INST(Fldpi           , FpuOp              , O_FPU(00,D9EB,_)          , 0                         , 0 , 0 , 868 , 33 , 45 ), // #206
  INST(Fldz            , FpuOp              , O_FPU(00,D9EE,_)          , 0                         , 0 , 0 , 874 , 33 , 45 ), // #207
  INST(Fmul            , FpuArith           , O_FPU(00,C8C8,1)          , 0                         , 0 , 0 , 2015, 84 , 45 ), // #208
  INST(Fmulp           , FpuRDef            , O_FPU(00,DEC8,_)          , 0                         , 0 , 0 , 879 , 71 , 45 ), // #209
  INST(Fnclex          , FpuOp              , O_FPU(00,DBE2,_)          , 0                         , 0 , 0 , 885 , 33 , 45 ), // #210
  INST(Fninit          , FpuOp              , O_FPU(00,DBE3,_)          , 0                         , 0 , 0 , 892 , 33 , 45 ), // #211
  INST(Fnop            , FpuOp              , O_FPU(00,D9D0,_)          , 0                         , 0 , 0 , 899 , 33 , 45 ), // #212
  INST(Fnsave          , X86M_Only          , O_FPU(00,00DD,6)          , 0                         , 0 , 0 , 904 , 85 , 45 ), // #213
  INST(Fnstcw          , X86M_Only          , O_FPU(00,00D9,7)          , 0                         , 0 , 0 , 911 , 86 , 45 ), // #214
  INST(Fnstenv         , X86M_Only          , O_FPU(00,00D9,6)          , 0                         , 0 , 0 , 918 , 85 , 45 ), // #215
  INST(Fnstsw          , FpuStsw            , O_FPU(00,00DD,7)          , O_FPU(00,DFE0,_)          , 0 , 0 , 926 , 87 , 45 ), // #216
  INST(Fpatan          , FpuOp              , O_FPU(00,D9F3,_)          , 0                         , 0 , 0 , 933 , 33 , 45 ), // #217
  INST(Fprem           , FpuOp              , O_FPU(00,D9F8,_)          , 0                         , 0 , 0 , 940 , 33 , 45 ), // #218
  INST(Fprem1          , FpuOp              , O_FPU(00,D9F5,_)          , 0                         , 0 , 0 , 946 , 33 , 45 ), // #219
  INST(Fptan           , FpuOp              , O_FPU(00,D9F2,_)          , 0                         , 0 , 0 , 953 , 33 , 45 ), // #220
  INST(Frndint         , FpuOp              , O_FPU(00,D9FC,_)          , 0                         , 0 , 0 , 959 , 33 , 45 ), // #221
  INST(Frstor          , X86M_Only          , O_FPU(00,00DD,4)          , 0                         , 0 , 0 , 967 , 83 , 45 ), // #222
  INST(Fsave           , X86M_Only          , O_FPU(9B,00DD,6)          , 0                         , 0 , 0 , 974 , 85 , 45 ), // #223
  INST(Fscale          , FpuOp              , O_FPU(00,D9FD,_)          , 0                         , 0 , 0 , 980 , 33 , 45 ), // #224
  INST(Fsin            , FpuOp              , O_FPU(00,D9FE,_)          , 0                         , 0 , 0 , 987 , 33 , 45 ), // #225
  INST(Fsincos         , FpuOp              , O_FPU(00,D9FB,_)          , 0                         , 0 , 0 , 992 , 33 , 45 ), // #226
  INST(Fsqrt           , FpuOp              , O_FPU(00,D9FA,_)          , 0                         , 0 , 0 , 1000, 33 , 45 ), // #227
  INST(Fst             , FpuFldFst          , O_FPU(00,00D9,2)          , 0                         , 0 , 0 , 1006, 88 , 45 ), // #228
  INST(Fstcw           , X86M_Only          , O_FPU(9B,00D9,7)          , 0                         , 0 , 0 , 1010, 86 , 45 ), // #229
  INST(Fstenv          , X86M_Only          , O_FPU(9B,00D9,6)          , 0                         , 0 , 0 , 1016, 85 , 45 ), // #230
  INST(Fstp            , FpuFldFst          , O_FPU(00,00D9,3)          , O(000000,DB,7,_,_,_,_,_  ), 0 , 0 , 1023, 89 , 45 ), // #231
  INST(Fstsw           , FpuStsw            , O_FPU(9B,00DD,7)          , O_FPU(9B,DFE0,_)          , 0 , 0 , 1028, 87 , 45 ), // #232
  INST(Fsub            , FpuArith           , O_FPU(00,E0E8,4)          , 0                         , 0 , 0 , 2093, 70 , 45 ), // #233
  INST(Fsubp           , FpuRDef            , O_FPU(00,DEE8,_)          , 0                         , 0 , 0 , 1034, 71 , 45 ), // #234
  INST(Fsubr           , FpuArith           , O_FPU(00,E8E0,5)          , 0                         , 0 , 0 , 2099, 70 , 45 ), // #235
  INST(Fsubrp          , FpuRDef            , O_FPU(00,DEE0,_)          , 0                         , 0 , 0 , 1040, 71 , 45 ), // #236
  INST(Ftst            , FpuOp              , O_FPU(00,D9E4,_)          , 0                         , 0 , 0 , 1047, 33 , 45 ), // #237
  INST(Fucom           , FpuRDef            , O_FPU(00,DDE0,_)          , 0                         , 0 , 0 , 1052, 90 , 45 ), // #238
  INST(Fucomi          , FpuR               , O_FPU(00,DBE8,_)          , 0                         , 0 , 0 , 1058, 76 , 47 ), // #239
  INST(Fucomip         , FpuR               , O_FPU(00,DFE8,_)          , 0                         , 0 , 0 , 1065, 76 , 47 ), // #240
  INST(Fucomp          , FpuRDef            , O_FPU(00,DDE8,_)          , 0                         , 0 , 0 , 1073, 90 , 45 ), // #241
  INST(Fucompp         , FpuOp              , O_FPU(00,DAE9,_)          , 0                         , 0 , 0 , 1080, 33 , 45 ), // #242
  INST(Fwait           , X86Op              , O_FPU(00,00DB,_)          , 0                         , 0 , 0 , 1088, 33 , 45 ), // #243
  INST(Fxam            , FpuOp              , O_FPU(00,D9E5,_)          , 0                         , 0 , 0 , 1094, 33 , 45 ), // #244
  INST(Fxch            , FpuR               , O_FPU(00,D9C8,_)          , 0                         , 0 , 0 , 1099, 71 , 45 ), // #245
  INST(Fxrstor         , X86M_Only          , O(000F00,AE,1,_,_,_,_,_  ), 0                         , 0 , 0 , 1104, 34 , 50 ), // #246
  INST(Fxrstor64       , X86M_Only          , O(000F00,AE,1,_,1,_,_,_  ), 0                         , 0 , 0 , 1112, 91 , 50 ), // #247
  INST(Fxsave          , X86M_Only          , O(000F00,AE,0,_,_,_,_,_  ), 0                         , 0 , 0 , 1122, 92 , 51 ), // #248
  INST(Fxsave64        , X86M_Only          , O(000F00,AE,0,_,1,_,_,_  ), 0                         , 0 , 0 , 1129, 93 , 51 ), // #249
  INST(Fxtract         , FpuOp              , O_FPU(00,D9F4,_)          , 0                         , 0 , 0 , 1138, 33 , 45 ), // #250
  INST(Fyl2x           , FpuOp              , O_FPU(00,D9F1,_)          , 0                         , 0 , 0 , 1146, 33 , 45 ), // #251
  INST(Fyl2xp1         , FpuOp              , O_FPU(00,D9F9,_)          , 0                         , 0 , 0 , 1152, 33 , 45 ), // #252
  INST(Haddpd          , ExtRm              , O(660F00,7C,_,_,_,_,_,_  ), 0                         , 0 , 0 , 5419, 5  , 6  ), // #253
  INST(Haddps          , ExtRm              , O(F20F00,7C,_,_,_,_,_,_  ), 0                         , 0 , 0 , 5427, 5  , 6  ), // #254
  INST(Hlt             , X86Op              , O(000000,F4,_,_,_,_,_,_  ), 0                         , 0 , 0 , 1160, 35 , 0  ), // #255
  INST(Hsubpd          , ExtRm              , O(660F00,7D,_,_,_,_,_,_  ), 0                         , 0 , 0 , 5435, 5  , 6  ), // #256
  INST(Hsubps          , ExtRm              , O(F20F00,7D,_,_,_,_,_,_  ), 0                         , 0 , 0 , 5443, 5  , 6  ), // #257
  INST(Idiv            , X86M_GPB_MulDiv    , O(000000,F6,7,_,x,_,_,_  ), 0                         , 0 , 0 , 750 , 65 , 1  ), // #258
  INST(Imul            , X86Imul            , O(000000,F6,5,_,x,_,_,_  ), 0                         , 0 , 0 , 768 , 94 , 1  ), // #259
  INST(In              , X86In              , O(000000,EC,_,_,_,_,_,_  ), O(000000,E4,_,_,_,_,_,_  ), 0 , 0 , 9722, 95 , 0  ), // #260
  INST(Inc             , X86IncDec          , O(000000,FE,0,_,x,_,_,_  ), O(000000,40,_,_,x,_,_,_  ), 0 , 0 , 1164, 64 , 42 ), // #261
  INST(Ins             , X86Ins             , O(000000,6C,_,_,_,_,_,_  ), 0                         , 0 , 0 , 1168, 96 , 0  ), // #262
  INST(Insertps        , ExtRmi             , O(660F3A,21,_,_,_,_,_,_  ), 0                         , 0 , 0 , 5579, 41 , 12 ), // #263
  INST(Insertq         , ExtInsertq         , O(F20F00,79,_,_,_,_,_,_  ), O(F20F00,78,_,_,_,_,_,_  ), 0 , 0 , 1172, 97 , 44 ), // #264
  INST(Int             , X86Int             , O(000000,CD,_,_,_,_,_,_  ), 0                         , 0 , 0 , 963 , 98 , 0  ), // #265
  INST(Int3            , X86Op              , O(000000,CC,_,_,_,_,_,_  ), 0                         , 0 , 0 , 1180, 32 , 0  ), // #266
  INST(Into            , X86Op              , O(000000,CE,_,_,_,_,_,_  ), 0                         , 0 , 0 , 1185, 99 , 52 ), // #267
  INST(Invd            , X86Op              , O(000F00,08,_,_,_,_,_,_  ), 0                         , 0 , 0 , 9677, 35 , 40 ), // #268
  INST(Invlpg          , X86M_Only          , O(000F00,01,7,_,_,_,_,_  ), 0                         , 0 , 0 , 1190, 100, 40 ), // #269
  INST(Invpcid         , X86Rm_NoRexW       , O(660F38,82,_,_,_,_,_,_  ), 0                         , 0 , 0 , 1197, 101, 40 ), // #270
  INST(Iret            , X86Op              , O(000000,CF,_,_,_,_,_,_  ), 0                         , 0 , 0 , 1205, 102, 1  ), // #271
  INST(Iretd           , X86Op              , O(000000,CF,_,_,_,_,_,_  ), 0                         , 0 , 0 , 1210, 102, 1  ), // #272
  INST(Iretq           , X86Op              , O(000000,CF,_,_,1,_,_,_  ), 0                         , 0 , 0 , 1216, 103, 1  ), // #273
  INST(Iretw           , X86Op              , O(660000,CF,_,_,_,_,_,_  ), 0                         , 0 , 0 , 1222, 102, 1  ), // #274
  INST(Ja              , X86Jcc             , O(000F00,87,_,_,_,_,_,_  ), O(000000,77,_,_,_,_,_,_  ), 0 , 0 , 1228, 104, 53 ), // #275
  INST(Jae             , X86Jcc             , O(000F00,83,_,_,_,_,_,_  ), O(000000,73,_,_,_,_,_,_  ), 0 , 0 , 1231, 104, 54 ), // #276
  INST(Jb              , X86Jcc             , O(000F00,82,_,_,_,_,_,_  ), O(000000,72,_,_,_,_,_,_  ), 0 , 0 , 1235, 104, 54 ), // #277
  INST(Jbe             , X86Jcc             , O(000F00,86,_,_,_,_,_,_  ), O(000000,76,_,_,_,_,_,_  ), 0 , 0 , 1238, 104, 53 ), // #278
  INST(Jc              , X86Jcc             , O(000F00,82,_,_,_,_,_,_  ), O(000000,72,_,_,_,_,_,_  ), 0 , 0 , 1242, 104, 54 ), // #279
  INST(Je              , X86Jcc             , O(000F00,84,_,_,_,_,_,_  ), O(000000,74,_,_,_,_,_,_  ), 0 , 0 , 1245, 104, 55 ), // #280
  INST(Jecxz           , X86JecxzLoop       , 0                         , O(000000,E3,_,_,_,_,_,_  ), 0 , 0 , 1248, 105, 0  ), // #281
  INST(Jg              , X86Jcc             , O(000F00,8F,_,_,_,_,_,_  ), O(000000,7F,_,_,_,_,_,_  ), 0 , 0 , 1254, 104, 56 ), // #282
  INST(Jge             , X86Jcc             , O(000F00,8D,_,_,_,_,_,_  ), O(000000,7D,_,_,_,_,_,_  ), 0 , 0 , 1257, 104, 57 ), // #283
  INST(Jl              , X86Jcc             , O(000F00,8C,_,_,_,_,_,_  ), O(000000,7C,_,_,_,_,_,_  ), 0 , 0 , 1261, 104, 57 ), // #284
  INST(Jle             , X86Jcc             , O(000F00,8E,_,_,_,_,_,_  ), O(000000,7E,_,_,_,_,_,_  ), 0 , 0 , 1264, 104, 56 ), // #285
  INST(Jmp             , X86Jmp             , O(000000,FF,4,_,_,_,_,_  ), O(000000,EB,_,_,_,_,_,_  ), 0 , 0 , 1268, 106, 0  ), // #286
  INST(Jna             , X86Jcc             , O(000F00,86,_,_,_,_,_,_  ), O(000000,76,_,_,_,_,_,_  ), 0 , 0 , 1272, 104, 53 ), // #287
  INST(Jnae            , X86Jcc             , O(000F00,82,_,_,_,_,_,_  ), O(000000,72,_,_,_,_,_,_  ), 0 , 0 , 1276, 104, 54 ), // #288
  INST(Jnb             , X86Jcc             , O(000F00,83,_,_,_,_,_,_  ), O(000000,73,_,_,_,_,_,_  ), 0 , 0 , 1281, 104, 54 ), // #289
  INST(Jnbe            , X86Jcc             , O(000F00,87,_,_,_,_,_,_  ), O(000000,77,_,_,_,_,_,_  ), 0 , 0 , 1285, 104, 53 ), // #290
  INST(Jnc             , X86Jcc             , O(000F00,83,_,_,_,_,_,_  ), O(000000,73,_,_,_,_,_,_  ), 0 , 0 , 1290, 104, 54 ), // #291
  INST(Jne             , X86Jcc             , O(000F00,85,_,_,_,_,_,_  ), O(000000,75,_,_,_,_,_,_  ), 0 , 0 , 1294, 104, 55 ), // #292
  INST(Jng             , X86Jcc             , O(000F00,8E,_,_,_,_,_,_  ), O(000000,7E,_,_,_,_,_,_  ), 0 , 0 , 1298, 104, 56 ), // #293
  INST(Jnge            , X86Jcc             , O(000F00,8C,_,_,_,_,_,_  ), O(000000,7C,_,_,_,_,_,_  ), 0 , 0 , 1302, 104, 57 ), // #294
  INST(Jnl             , X86Jcc             , O(000F00,8D,_,_,_,_,_,_  ), O(000000,7D,_,_,_,_,_,_  ), 0 , 0 , 1307, 104, 57 ), // #295
  INST(Jnle            , X86Jcc             , O(000F00,8F,_,_,_,_,_,_  ), O(000000,7F,_,_,_,_,_,_  ), 0 , 0 , 1311, 104, 56 ), // #296
  INST(Jno             , X86Jcc             , O(000F00,81,_,_,_,_,_,_  ), O(000000,71,_,_,_,_,_,_  ), 0 , 0 , 1316, 104, 52 ), // #297
  INST(Jnp             , X86Jcc             , O(000F00,8B,_,_,_,_,_,_  ), O(000000,7B,_,_,_,_,_,_  ), 0 , 0 , 1320, 104, 58 ), // #298
  INST(Jns             , X86Jcc             , O(000F00,89,_,_,_,_,_,_  ), O(000000,79,_,_,_,_,_,_  ), 0 , 0 , 1324, 104, 59 ), // #299
  INST(Jnz             , X86Jcc             , O(000F00,85,_,_,_,_,_,_  ), O(000000,75,_,_,_,_,_,_  ), 0 , 0 , 1328, 104, 55 ), // #300
  INST(Jo              , X86Jcc             , O(000F00,80,_,_,_,_,_,_  ), O(000000,70,_,_,_,_,_,_  ), 0 , 0 , 1332, 104, 52 ), // #301
  INST(Jp              , X86Jcc             , O(000F00,8A,_,_,_,_,_,_  ), O(000000,7A,_,_,_,_,_,_  ), 0 , 0 , 1335, 104, 58 ), // #302
  INST(Jpe             , X86Jcc             , O(000F00,8A,_,_,_,_,_,_  ), O(000000,7A,_,_,_,_,_,_  ), 0 , 0 , 1338, 104, 58 ), // #303
  INST(Jpo             , X86Jcc             , O(000F00,8B,_,_,_,_,_,_  ), O(000000,7B,_,_,_,_,_,_  ), 0 , 0 , 1342, 104, 58 ), // #304
  INST(Js              , X86Jcc             , O(000F00,88,_,_,_,_,_,_  ), O(000000,78,_,_,_,_,_,_  ), 0 , 0 , 1346, 104, 59 ), // #305
  INST(Jz              , X86Jcc             , O(000F00,84,_,_,_,_,_,_  ), O(000000,74,_,_,_,_,_,_  ), 0 , 0 , 1349, 104, 55 ), // #306
  INST(Kaddb           , VexRvm             , V(660F00,4A,_,1,0,_,_,_  ), 0                         , 0 , 0 , 1352, 107, 60 ), // #307
  INST(Kaddd           , VexRvm             , V(660F00,4A,_,1,1,_,_,_  ), 0                         , 0 , 0 , 1358, 107, 61 ), // #308
  INST(Kaddq           , VexRvm             , V(000F00,4A,_,1,1,_,_,_  ), 0                         , 0 , 0 , 1364, 107, 61 ), // #309
  INST(Kaddw           , VexRvm             , V(000F00,4A,_,1,0,_,_,_  ), 0                         , 0 , 0 , 1370, 107, 60 ), // #310
  INST(Kandb           , VexRvm             , V(660F00,41,_,1,0,_,_,_  ), 0                         , 0 , 0 , 1376, 107, 60 ), // #311
  INST(Kandd           , VexRvm             , V(660F00,41,_,1,1,_,_,_  ), 0                         , 0 , 0 , 1382, 107, 61 ), // #312
  INST(Kandnb          , VexRvm             , V(660F00,42,_,1,0,_,_,_  ), 0                         , 0 , 0 , 1388, 107, 60 ), // #313
  INST(Kandnd          , VexRvm             , V(660F00,42,_,1,1,_,_,_  ), 0                         , 0 , 0 , 1395, 107, 61 ), // #314
  INST(Kandnq          , VexRvm             , V(000F00,42,_,1,1,_,_,_  ), 0                         , 0 , 0 , 1402, 107, 61 ), // #315
  INST(Kandnw          , VexRvm             , V(000F00,42,_,1,0,_,_,_  ), 0                         , 0 , 0 , 1409, 107, 62 ), // #316
  INST(Kandq           , VexRvm             , V(000F00,41,_,1,1,_,_,_  ), 0                         , 0 , 0 , 1416, 107, 61 ), // #317
  INST(Kandw           , VexRvm             , V(000F00,41,_,1,0,_,_,_  ), 0                         , 0 , 0 , 1422, 107, 62 ), // #318
  INST(Kmovb           , VexKmov            , V(660F00,90,_,0,0,_,_,_  ), V(660F00,92,_,0,0,_,_,_  ), 0 , 0 , 1428, 108, 60 ), // #319
  INST(Kmovd           , VexKmov            , V(660F00,90,_,0,1,_,_,_  ), V(F20F00,92,_,0,0,_,_,_  ), 0 , 0 , 7425, 109, 61 ), // #320
  INST(Kmovq           , VexKmov            , V(000F00,90,_,0,1,_,_,_  ), V(F20F00,92,_,0,1,_,_,_  ), 0 , 0 , 7436, 110, 61 ), // #321
  INST(Kmovw           , VexKmov            , V(000F00,90,_,0,0,_,_,_  ), V(000F00,92,_,0,0,_,_,_  ), 0 , 0 , 1434, 111, 62 ), // #322
  INST(Knotb           , VexRm              , V(660F00,44,_,0,0,_,_,_  ), 0                         , 0 , 0 , 1440, 112, 60 ), // #323
  INST(Knotd           , VexRm              , V(660F00,44,_,0,1,_,_,_  ), 0                         , 0 , 0 , 1446, 112, 61 ), // #324
  INST(Knotq           , VexRm              , V(000F00,44,_,0,1,_,_,_  ), 0                         , 0 , 0 , 1452, 112, 61 ), // #325
  INST(Knotw           , VexRm              , V(000F00,44,_,0,0,_,_,_  ), 0                         , 0 , 0 , 1458, 112, 62 ), // #326
  INST(Korb            , VexRvm             , V(660F00,45,_,1,0,_,_,_  ), 0                         , 0 , 0 , 1464, 107, 60 ), // #327
  INST(Kord            , VexRvm             , V(660F00,45,_,1,1,_,_,_  ), 0                         , 0 , 0 , 1469, 107, 61 ), // #328
  INST(Korq            , VexRvm             , V(000F00,45,_,1,1,_,_,_  ), 0                         , 0 , 0 , 1474, 107, 61 ), // #329
  INST(Kortestb        , VexRm              , V(660F00,98,_,0,0,_,_,_  ), 0                         , 0 , 0 , 1479, 113, 63 ), // #330
  INST(Kortestd        , VexRm              , V(660F00,98,_,0,1,_,_,_  ), 0                         , 0 , 0 , 1488, 113, 64 ), // #331
  INST(Kortestq        , VexRm              , V(000F00,98,_,0,1,_,_,_  ), 0                         , 0 , 0 , 1497, 113, 64 ), // #332
  INST(Kortestw        , VexRm              , V(000F00,98,_,0,0,_,_,_  ), 0                         , 0 , 0 , 1506, 113, 65 ), // #333
  INST(Korw            , VexRvm             , V(000F00,45,_,1,0,_,_,_  ), 0                         , 0 , 0 , 1515, 107, 62 ), // #334
  INST(Kshiftlb        , VexRmi             , V(660F3A,32,_,0,0,_,_,_  ), 0                         , 0 , 0 , 1520, 114, 60 ), // #335
  INST(Kshiftld        , VexRmi             , V(660F3A,33,_,0,0,_,_,_  ), 0                         , 0 , 0 , 1529, 114, 61 ), // #336
  INST(Kshiftlq        , VexRmi             , V(660F3A,33,_,0,1,_,_,_  ), 0                         , 0 , 0 , 1538, 114, 61 ), // #337
  INST(Kshiftlw        , VexRmi             , V(660F3A,32,_,0,1,_,_,_  ), 0                         , 0 , 0 , 1547, 114, 62 ), // #338
  INST(Kshiftrb        , VexRmi             , V(660F3A,30,_,0,0,_,_,_  ), 0                         , 0 , 0 , 1556, 114, 60 ), // #339
  INST(Kshiftrd        , VexRmi             , V(660F3A,31,_,0,0,_,_,_  ), 0                         , 0 , 0 , 1565, 114, 61 ), // #340
  INST(Kshiftrq        , VexRmi             , V(660F3A,31,_,0,1,_,_,_  ), 0                         , 0 , 0 , 1574, 114, 61 ), // #341
  INST(Kshiftrw        , VexRmi             , V(660F3A,30,_,0,1,_,_,_  ), 0                         , 0 , 0 , 1583, 114, 62 ), // #342
  INST(Ktestb          , VexRm              , V(660F00,99,_,0,0,_,_,_  ), 0                         , 0 , 0 , 1592, 113, 63 ), // #343
  INST(Ktestd          , VexRm              , V(660F00,99,_,0,1,_,_,_  ), 0                         , 0 , 0 , 1599, 113, 64 ), // #344
  INST(Ktestq          , VexRm              , V(000F00,99,_,0,1,_,_,_  ), 0                         , 0 , 0 , 1606, 113, 64 ), // #345
  INST(Ktestw          , VexRm              , V(000F00,99,_,0,0,_,_,_  ), 0                         , 0 , 0 , 1613, 113, 63 ), // #346
  INST(Kunpckbw        , VexRvm             , V(660F00,4B,_,1,0,_,_,_  ), 0                         , 0 , 0 , 1620, 107, 62 ), // #347
  INST(Kunpckdq        , VexRvm             , V(000F00,4B,_,1,1,_,_,_  ), 0                         , 0 , 0 , 1629, 107, 61 ), // #348
  INST(Kunpckwd        , VexRvm             , V(000F00,4B,_,1,0,_,_,_  ), 0                         , 0 , 0 , 1638, 107, 61 ), // #349
  INST(Kxnorb          , VexRvm             , V(660F00,46,_,1,0,_,_,_  ), 0                         , 0 , 0 , 1647, 107, 60 ), // #350
  INST(Kxnord          , VexRvm             , V(660F00,46,_,1,1,_,_,_  ), 0                         , 0 , 0 , 1654, 107, 61 ), // #351
  INST(Kxnorq          , VexRvm             , V(000F00,46,_,1,1,_,_,_  ), 0                         , 0 , 0 , 1661, 107, 61 ), // #352
  INST(Kxnorw          , VexRvm             , V(000F00,46,_,1,0,_,_,_  ), 0                         , 0 , 0 , 1668, 107, 62 ), // #353
  INST(Kxorb           , VexRvm             , V(660F00,47,_,1,0,_,_,_  ), 0                         , 0 , 0 , 1675, 107, 60 ), // #354
  INST(Kxord           , VexRvm             , V(660F00,47,_,1,1,_,_,_  ), 0                         , 0 , 0 , 1681, 107, 61 ), // #355
  INST(Kxorq           , VexRvm             , V(000F00,47,_,1,1,_,_,_  ), 0                         , 0 , 0 , 1687, 107, 61 ), // #356
  INST(Kxorw           , VexRvm             , V(000F00,47,_,1,0,_,_,_  ), 0                         , 0 , 0 , 1693, 107, 62 ), // #357
  INST(Lahf            , X86Op              , O(000000,9F,_,_,_,_,_,_  ), 0                         , 0 , 0 , 1699, 115, 66 ), // #358
  INST(Lar             , X86Rm              , O(000F00,02,_,_,_,_,_,_  ), 0                         , 0 , 0 , 1704, 116, 10 ), // #359
  INST(Lddqu           , ExtRm              , O(F20F00,F0,_,_,_,_,_,_  ), 0                         , 0 , 16, 5589, 117, 6  ), // #360
  INST(Ldmxcsr         , X86M_Only          , O(000F00,AE,2,_,_,_,_,_  ), 0                         , 0 , 0 , 5596, 118, 5  ), // #361
  INST(Lds             , X86Rm              , O(000000,C5,_,_,_,_,_,_  ), 0                         , 0 , 0 , 1708, 119, 0  ), // #362
  INST(Lea             , X86Lea             , O(000000,8D,_,_,x,_,_,_  ), 0                         , 0 , 0 , 1712, 120, 0  ), // #363
  INST(Leave           , X86Op              , O(000000,C9,_,_,_,_,_,_  ), 0                         , 0 , 0 , 1716, 32 , 0  ), // #364
  INST(Les             , X86Rm              , O(000000,C4,_,_,_,_,_,_  ), 0                         , 0 , 0 , 1722, 119, 0  ), // #365
  INST(Lfence          , X86Fence           , O(000F00,AE,5,_,_,_,_,_  ), 0                         , 0 , 0 , 1726, 32 , 4  ), // #366
  INST(Lfs             , X86Rm              , O(000F00,B4,_,_,_,_,_,_  ), 0                         , 0 , 0 , 1733, 121, 0  ), // #367
  INST(Lgdt            , X86M_Only          , O(000F00,01,2,_,_,_,_,_  ), 0                         , 0 , 0 , 1737, 100, 0  ), // #368
  INST(Lgs             , X86Rm              , O(000F00,B5,_,_,_,_,_,_  ), 0                         , 0 , 0 , 1742, 121, 0  ), // #369
  INST(Lidt            , X86M_Only          , O(000F00,01,3,_,_,_,_,_  ), 0                         , 0 , 0 , 1746, 100, 0  ), // #370
  INST(Lldt            , X86M_NoSize        , O(000F00,00,2,_,_,_,_,_  ), 0                         , 0 , 0 , 1751, 122, 0  ), // #371
  INST(Lmsw            , X86M_NoSize        , O(000F00,01,6,_,_,_,_,_  ), 0                         , 0 , 0 , 1756, 122, 0  ), // #372
  INST(Lods            , X86StrRm           , O(000000,AC,_,_,_,_,_,_  ), 0                         , 0 , 0 , 1761, 123, 67 ), // #373
  INST(Loop            , X86JecxzLoop       , 0                         , O(000000,E2,_,_,_,_,_,_  ), 0 , 0 , 1766, 124, 0  ), // #374
  INST(Loope           , X86JecxzLoop       , 0                         , O(000000,E1,_,_,_,_,_,_  ), 0 , 0 , 1771, 124, 55 ), // #375
  INST(Loopne          , X86JecxzLoop       , 0                         , O(000000,E0,_,_,_,_,_,_  ), 0 , 0 , 1777, 124, 55 ), // #376
  INST(Lsl             , X86Rm              , O(000F00,03,_,_,_,_,_,_  ), 0                         , 0 , 0 , 1784, 125, 10 ), // #377
  INST(Lss             , X86Rm              , O(000F00,B2,_,_,_,_,_,_  ), 0                         , 0 , 0 , 5996, 121, 0  ), // #378
  INST(Ltr             , X86M_NoSize        , O(000F00,00,3,_,_,_,_,_  ), 0                         , 0 , 0 , 1788, 122, 0  ), // #379
  INST(Lzcnt           , X86Rm_Raw66H       , O(F30F00,BD,_,_,x,_,_,_  ), 0                         , 0 , 0 , 1792, 24 , 68 ), // #380
  INST(Maskmovdqu      , ExtRm_ZDI          , O(660F00,57,_,_,_,_,_,_  ), 0                         , 0 , 0 , 5605, 126, 4  ), // #381
  INST(Maskmovq        , ExtRm_ZDI          , O(000F00,F7,_,_,_,_,_,_  ), 0                         , 0 , 0 , 7433, 127, 69 ), // #382
  INST(Maxpd           , ExtRm              , O(660F00,5F,_,_,_,_,_,_  ), 0                         , 0 , 0 , 5639, 5  , 4  ), // #383
  INST(Maxps           , ExtRm              , O(000F00,5F,_,_,_,_,_,_  ), 0                         , 0 , 0 , 5646, 5  , 5  ), // #384
  INST(Maxsd           , ExtRm              , O(F20F00,5F,_,_,_,_,_,_  ), 0                         , 0 , 0 , 7452, 6  , 4  ), // #385
  INST(Maxss           , ExtRm              , O(F30F00,5F,_,_,_,_,_,_  ), 0                         , 0 , 0 , 5660, 7  , 5  ), // #386
  INST(Mfence          , X86Fence           , O(000F00,AE,6,_,_,_,_,_  ), 0                         , 0 , 0 , 1798, 32 , 4  ), // #387
  INST(Minpd           , ExtRm              , O(660F00,5D,_,_,_,_,_,_  ), 0                         , 0 , 0 , 5667, 5  , 4  ), // #388
  INST(Minps           , ExtRm              , O(000F00,5D,_,_,_,_,_,_  ), 0                         , 0 , 0 , 5674, 5  , 5  ), // #389
  INST(Minsd           , ExtRm              , O(F20F00,5D,_,_,_,_,_,_  ), 0                         , 0 , 0 , 7516, 6  , 4  ), // #390
  INST(Minss           , ExtRm              , O(F30F00,5D,_,_,_,_,_,_  ), 0                         , 0 , 0 , 5688, 7  , 5  ), // #391
  INST(Monitor         , X86Op              , O(000F01,C8,_,_,_,_,_,_  ), 0                         , 0 , 0 , 1805, 128, 70 ), // #392
  INST(Monitorx        , X86Op              , O(000F01,FA,_,_,_,_,_,_  ), 0                         , 0 , 0 , 1813, 129, 71 ), // #393
  INST(Mov             , X86Mov             , 0                         , 0                         , 0 , 0 , 138 , 130, 0  ), // #394
  INST(Movapd          , ExtMov             , O(660F00,28,_,_,_,_,_,_  ), O(660F00,29,_,_,_,_,_,_  ), 0 , 16, 5695, 131, 4  ), // #395
  INST(Movaps          , ExtMov             , O(000F00,28,_,_,_,_,_,_  ), O(000F00,29,_,_,_,_,_,_  ), 0 , 16, 5703, 131, 5  ), // #396
  INST(Movbe           , ExtMovbe           , O(000F38,F0,_,_,x,_,_,_  ), O(000F38,F1,_,_,x,_,_,_  ), 0 , 0 , 597 , 132, 72 ), // #397
  INST(Movd            , ExtMovd            , O(000F00,6E,_,_,_,_,_,_  ), O(000F00,7E,_,_,_,_,_,_  ), 0 , 16, 7426, 133, 73 ), // #398
  INST(Movddup         , ExtMov             , O(F20F00,12,_,_,_,_,_,_  ), 0                         , 0 , 16, 5717, 50 , 6  ), // #399
  INST(Movdq2q         , ExtMov             , O(F20F00,D6,_,_,_,_,_,_  ), 0                         , 0 , 8 , 1822, 134, 4  ), // #400
  INST(Movdqa          , ExtMov             , O(660F00,6F,_,_,_,_,_,_  ), O(660F00,7F,_,_,_,_,_,_  ), 0 , 16, 5726, 131, 4  ), // #401
  INST(Movdqu          , ExtMov             , O(F30F00,6F,_,_,_,_,_,_  ), O(F30F00,7F,_,_,_,_,_,_  ), 0 , 16, 5609, 131, 4  ), // #402
  INST(Movhlps         , ExtMov             , O(000F00,12,_,_,_,_,_,_  ), 0                         , 0 , 8 , 5801, 135, 5  ), // #403
  INST(Movhpd          , ExtMov             , O(660F00,16,_,_,_,_,_,_  ), O(660F00,17,_,_,_,_,_,_  ), 8 , 8 , 5810, 136, 4  ), // #404
  INST(Movhps          , ExtMov             , O(000F00,16,_,_,_,_,_,_  ), O(000F00,17,_,_,_,_,_,_  ), 8 , 8 , 5818, 136, 5  ), // #405
  INST(Movlhps         , ExtMov             , O(000F00,16,_,_,_,_,_,_  ), 0                         , 8 , 8 , 5826, 137, 5  ), // #406
  INST(Movlpd          , ExtMov             , O(660F00,12,_,_,_,_,_,_  ), O(660F00,13,_,_,_,_,_,_  ), 0 , 8 , 5835, 138, 4  ), // #407
  INST(Movlps          , ExtMov             , O(000F00,12,_,_,_,_,_,_  ), O(000F00,13,_,_,_,_,_,_  ), 0 , 8 , 5843, 138, 5  ), // #408
  INST(Movmskpd        , ExtMov             , O(660F00,50,_,_,_,_,_,_  ), 0                         , 0 , 8 , 5851, 139, 4  ), // #409
  INST(Movmskps        , ExtMov             , O(000F00,50,_,_,_,_,_,_  ), 0                         , 0 , 8 , 5861, 139, 5  ), // #410
  INST(Movntdq         , ExtMov             , 0                         , O(660F00,E7,_,_,_,_,_,_  ), 0 , 16, 5871, 140, 4  ), // #411
  INST(Movntdqa        , ExtMov             , O(660F38,2A,_,_,_,_,_,_  ), 0                         , 0 , 16, 5880, 117, 12 ), // #412
  INST(Movnti          , ExtMovnti          , O(000F00,C3,_,_,x,_,_,_  ), 0                         , 0 , 8 , 1830, 141, 4  ), // #413
  INST(Movntpd         , ExtMov             , 0                         , O(660F00,2B,_,_,_,_,_,_  ), 0 , 16, 5890, 140, 4  ), // #414
  INST(Movntps         , ExtMov             , 0                         , O(000F00,2B,_,_,_,_,_,_  ), 0 , 16, 5899, 140, 5  ), // #415
  INST(Movntq          , ExtMov             , 0                         , O(000F00,E7,_,_,_,_,_,_  ), 0 , 8 , 1837, 142, 69 ), // #416
  INST(Movntsd         , ExtMov             , 0                         , O(F20F00,2B,_,_,_,_,_,_  ), 0 , 8 , 1844, 143, 44 ), // #417
  INST(Movntss         , ExtMov             , 0                         , O(F30F00,2B,_,_,_,_,_,_  ), 0 , 4 , 1852, 144, 44 ), // #418
  INST(Movq            , ExtMovq            , O(000F00,6E,_,_,x,_,_,_  ), O(000F00,7E,_,_,x,_,_,_  ), 0 , 16, 7437, 145, 73 ), // #419
  INST(Movq2dq         , ExtRm              , O(F30F00,D6,_,_,_,_,_,_  ), 0                         , 0 , 16, 1860, 146, 4  ), // #420
  INST(Movs            , X86StrMm           , O(000000,A4,_,_,_,_,_,_  ), 0                         , 0 , 0 , 411 , 147, 67 ), // #421
  INST(Movsd           , ExtMov             , O(F20F00,10,_,_,_,_,_,_  ), O(F20F00,11,_,_,_,_,_,_  ), 0 , 8 , 5914, 148, 4  ), // #422
  INST(Movshdup        , ExtRm              , O(F30F00,16,_,_,_,_,_,_  ), 0                         , 0 , 16, 5921, 51 , 6  ), // #423
  INST(Movsldup        , ExtRm              , O(F30F00,12,_,_,_,_,_,_  ), 0                         , 0 , 16, 5931, 51 , 6  ), // #424
  INST(Movss           , ExtMov             , O(F30F00,10,_,_,_,_,_,_  ), O(F30F00,11,_,_,_,_,_,_  ), 0 , 4 , 5941, 149, 5  ), // #425
  INST(Movsx           , X86MovsxMovzx      , O(000F00,BE,_,_,x,_,_,_  ), 0                         , 0 , 0 , 1868, 150, 0  ), // #426
  INST(Movsxd          , X86Rm              , O(000000,63,_,_,1,_,_,_  ), 0                         , 0 , 0 , 1874, 151, 0  ), // #427
  INST(Movupd          , ExtMov             , O(660F00,10,_,_,_,_,_,_  ), O(660F00,11,_,_,_,_,_,_  ), 0 , 16, 5948, 131, 4  ), // #428
  INST(Movups          , ExtMov             , O(000F00,10,_,_,_,_,_,_  ), O(000F00,11,_,_,_,_,_,_  ), 0 , 16, 5956, 131, 5  ), // #429
  INST(Movzx           , X86MovsxMovzx      , O(000F00,B6,_,_,x,_,_,_  ), 0                         , 0 , 0 , 1881, 150, 0  ), // #430
  INST(Mpsadbw         , ExtRmi             , O(660F3A,42,_,_,_,_,_,_  ), 0                         , 0 , 0 , 5964, 16 , 12 ), // #431
  INST(Mul             , X86M_GPB_MulDiv    , O(000000,F6,4,_,x,_,_,_  ), 0                         , 0 , 0 , 769 , 152, 1  ), // #432
  INST(Mulpd           , ExtRm              , O(660F00,59,_,_,_,_,_,_  ), 0                         , 0 , 0 , 5973, 5  , 4  ), // #433
  INST(Mulps           , ExtRm              , O(000F00,59,_,_,_,_,_,_  ), 0                         , 0 , 0 , 5980, 5  , 5  ), // #434
  INST(Mulsd           , ExtRm              , O(F20F00,59,_,_,_,_,_,_  ), 0                         , 0 , 0 , 5987, 6  , 4  ), // #435
  INST(Mulss           , ExtRm              , O(F30F00,59,_,_,_,_,_,_  ), 0                         , 0 , 0 , 5994, 7  , 5  ), // #436
  INST(Mulx            , VexRvm_ZDX_Wx      , V(F20F38,F6,_,0,x,_,_,_  ), 0                         , 0 , 0 , 1887, 153, 74 ), // #437
  INST(Mwait           , X86Op              , O(000F01,C9,_,_,_,_,_,_  ), 0                         , 0 , 0 , 1892, 154, 70 ), // #438
  INST(Mwaitx          , X86Op              , O(000F01,FB,_,_,_,_,_,_  ), 0                         , 0 , 0 , 1898, 155, 71 ), // #439
  INST(Neg             , X86M_GPB           , O(000000,F6,3,_,x,_,_,_  ), 0                         , 0 , 0 , 1905, 156, 75 ), // #440
  INST(Nop             , X86Op              , O(000000,90,_,_,_,_,_,_  ), 0                         , 0 , 0 , 900 , 157, 0  ), // #441
  INST(Not             , X86M_GPB           , O(000000,F6,2,_,x,_,_,_  ), 0                         , 0 , 0 , 1909, 156, 0  ), // #442
  INST(Or              , X86Arith           , O(000000,08,1,_,x,_,_,_  ), 0                         , 0 , 0 , 1109, 158, 1  ), // #443
  INST(Orpd            , ExtRm              , O(660F00,56,_,_,_,_,_,_  ), 0                         , 0 , 0 , 9643, 12 , 4  ), // #444
  INST(Orps            , ExtRm              , O(000F00,56,_,_,_,_,_,_  ), 0                         , 0 , 0 , 9650, 12 , 5  ), // #445
  INST(Out             , X86Out             , O(000000,EE,_,_,_,_,_,_  ), O(000000,E6,_,_,_,_,_,_  ), 0 , 0 , 1913, 159, 0  ), // #446
  INST(Outs            , X86Outs            , O(000000,6E,_,_,_,_,_,_  ), 0                         , 0 , 0 , 1917, 160, 0  ), // #447
  INST(Pabsb           , ExtRm_P            , O(000F38,1C,_,_,_,_,_,_  ), 0                         , 0 , 0 , 6034, 161, 76 ), // #448
  INST(Pabsd           , ExtRm_P            , O(000F38,1E,_,_,_,_,_,_  ), 0                         , 0 , 0 , 6041, 161, 76 ), // #449
  INST(Pabsw           , ExtRm_P            , O(000F38,1D,_,_,_,_,_,_  ), 0                         , 0 , 0 , 6055, 161, 76 ), // #450
  INST(Packssdw        , ExtRm_P            , O(000F00,6B,_,_,_,_,_,_  ), 0                         , 0 , 0 , 6062, 162, 73 ), // #451
  INST(Packsswb        , ExtRm_P            , O(000F00,63,_,_,_,_,_,_  ), 0                         , 0 , 0 , 6072, 162, 73 ), // #452
  INST(Packusdw        , ExtRm              , O(660F38,2B,_,_,_,_,_,_  ), 0                         , 0 , 0 , 6082, 5  , 12 ), // #453
  INST(Packuswb        , ExtRm_P            , O(000F00,67,_,_,_,_,_,_  ), 0                         , 0 , 0 , 6092, 162, 73 ), // #454
  INST(Paddb           , ExtRm_P            , O(000F00,FC,_,_,_,_,_,_  ), 0                         , 0 , 0 , 6102, 162, 73 ), // #455
  INST(Paddd           , ExtRm_P            , O(000F00,FE,_,_,_,_,_,_  ), 0                         , 0 , 0 , 6109, 162, 73 ), // #456
  INST(Paddq           , ExtRm_P            , O(000F00,D4,_,_,_,_,_,_  ), 0                         , 0 , 0 , 6116, 162, 4  ), // #457
  INST(Paddsb          , ExtRm_P            , O(000F00,EC,_,_,_,_,_,_  ), 0                         , 0 , 0 , 6123, 162, 73 ), // #458
  INST(Paddsw          , ExtRm_P            , O(000F00,ED,_,_,_,_,_,_  ), 0                         , 0 , 0 , 6131, 162, 73 ), // #459
  INST(Paddusb         , ExtRm_P            , O(000F00,DC,_,_,_,_,_,_  ), 0                         , 0 , 0 , 6139, 162, 73 ), // #460
  INST(Paddusw         , ExtRm_P            , O(000F00,DD,_,_,_,_,_,_  ), 0                         , 0 , 0 , 6148, 162, 73 ), // #461
  INST(Paddw           , ExtRm_P            , O(000F00,FD,_,_,_,_,_,_  ), 0                         , 0 , 0 , 6157, 162, 73 ), // #462
  INST(Palignr         , ExtRmi_P           , O(000F3A,0F,_,_,_,_,_,_  ), 0                         , 0 , 0 , 6164, 163, 6  ), // #463
  INST(Pand            , ExtRm_P            , O(000F00,DB,_,_,_,_,_,_  ), 0                         , 0 , 0 , 6173, 164, 73 ), // #464
  INST(Pandn           , ExtRm_P            , O(000F00,DF,_,_,_,_,_,_  ), 0                         , 0 , 0 , 6186, 165, 73 ), // #465
  INST(Pause           , X86Op              , O(F30000,90,_,_,_,_,_,_  ), 0                         , 0 , 0 , 1922, 32 , 0  ), // #466
  INST(Pavgb           , ExtRm_P            , O(000F00,E0,_,_,_,_,_,_  ), 0                         , 0 , 0 , 6216, 162, 77 ), // #467
  INST(Pavgusb         , Ext3dNow           , O(000F0F,BF,_,_,_,_,_,_  ), 0                         , 0 , 0 , 1928, 166, 48 ), // #468
  INST(Pavgw           , ExtRm_P            , O(000F00,E3,_,_,_,_,_,_  ), 0                         , 0 , 0 , 6223, 162, 77 ), // #469
  INST(Pblendvb        , ExtRm_XMM0         , O(660F38,10,_,_,_,_,_,_  ), 0                         , 0 , 0 , 6239, 17 , 12 ), // #470
  INST(Pblendw         , ExtRmi             , O(660F3A,0E,_,_,_,_,_,_  ), 0                         , 0 , 0 , 6249, 16 , 12 ), // #471
  INST(Pclmulqdq       , ExtRmi             , O(660F3A,44,_,_,_,_,_,_  ), 0                         , 0 , 0 , 6342, 16 , 78 ), // #472
  INST(Pcmpeqb         , ExtRm_P            , O(000F00,74,_,_,_,_,_,_  ), 0                         , 0 , 0 , 6374, 165, 73 ), // #473
  INST(Pcmpeqd         , ExtRm_P            , O(000F00,76,_,_,_,_,_,_  ), 0                         , 0 , 0 , 6383, 165, 73 ), // #474
  INST(Pcmpeqq         , ExtRm              , O(660F38,29,_,_,_,_,_,_  ), 0                         , 0 , 0 , 6392, 167, 12 ), // #475
  INST(Pcmpeqw         , ExtRm_P            , O(000F00,75,_,_,_,_,_,_  ), 0                         , 0 , 0 , 6401, 165, 73 ), // #476
  INST(Pcmpestri       , ExtRmi             , O(660F3A,61,_,_,_,_,_,_  ), 0                         , 0 , 0 , 6410, 168, 79 ), // #477
  INST(Pcmpestrm       , ExtRmi             , O(660F3A,60,_,_,_,_,_,_  ), 0                         , 0 , 0 , 6421, 169, 79 ), // #478
  INST(Pcmpgtb         , ExtRm_P            , O(000F00,64,_,_,_,_,_,_  ), 0                         , 0 , 0 , 6432, 165, 73 ), // #479
  INST(Pcmpgtd         , ExtRm_P            , O(000F00,66,_,_,_,_,_,_  ), 0                         , 0 , 0 , 6441, 165, 73 ), // #480
  INST(Pcmpgtq         , ExtRm              , O(660F38,37,_,_,_,_,_,_  ), 0                         , 0 , 0 , 6450, 167, 41 ), // #481
  INST(Pcmpgtw         , ExtRm_P            , O(000F00,65,_,_,_,_,_,_  ), 0                         , 0 , 0 , 6459, 165, 73 ), // #482
  INST(Pcmpistri       , ExtRmi             , O(660F3A,63,_,_,_,_,_,_  ), 0                         , 0 , 0 , 6468, 170, 79 ), // #483
  INST(Pcmpistrm       , ExtRmi             , O(660F3A,62,_,_,_,_,_,_  ), 0                         , 0 , 0 , 6479, 171, 79 ), // #484
  INST(Pcommit         , X86Op_O            , O(660F00,AE,7,_,_,_,_,_  ), 0                         , 0 , 0 , 1936, 32 , 80 ), // #485
  INST(Pdep            , VexRvm_Wx          , V(F20F38,F5,_,0,x,_,_,_  ), 0                         , 0 , 0 , 1944, 11 , 74 ), // #486
  INST(Pext            , VexRvm_Wx          , V(F30F38,F5,_,0,x,_,_,_  ), 0                         , 0 , 0 , 1949, 11 , 74 ), // #487
  INST(Pextrb          , ExtExtract         , O(000F3A,14,_,_,_,_,_,_  ), 0                         , 0 , 8 , 6928, 172, 12 ), // #488
  INST(Pextrd          , ExtExtract         , O(000F3A,16,_,_,_,_,_,_  ), 0                         , 0 , 8 , 6936, 68 , 12 ), // #489
  INST(Pextrq          , ExtExtract         , O(000F3A,16,_,_,1,_,_,_  ), 0                         , 0 , 8 , 6944, 173, 12 ), // #490
  INST(Pextrw          , ExtPextrw          , O(000F00,C5,_,_,_,_,_,_  ), O(000F3A,15,_,_,_,_,_,_  ), 0 , 8 , 6952, 174, 81 ), // #491
  INST(Pf2id           , Ext3dNow           , O(000F0F,1D,_,_,_,_,_,_  ), 0                         , 0 , 8 , 1954, 175, 48 ), // #492
  INST(Pf2iw           , Ext3dNow           , O(000F0F,1C,_,_,_,_,_,_  ), 0                         , 0 , 8 , 1960, 175, 82 ), // #493
  INST(Pfacc           , Ext3dNow           , O(000F0F,AE,_,_,_,_,_,_  ), 0                         , 0 , 0 , 1966, 166, 48 ), // #494
  INST(Pfadd           , Ext3dNow           , O(000F0F,9E,_,_,_,_,_,_  ), 0                         , 0 , 0 , 1972, 166, 48 ), // #495
  INST(Pfcmpeq         , Ext3dNow           , O(000F0F,B0,_,_,_,_,_,_  ), 0                         , 0 , 0 , 1978, 166, 48 ), // #496
  INST(Pfcmpge         , Ext3dNow           , O(000F0F,90,_,_,_,_,_,_  ), 0                         , 0 , 0 , 1986, 166, 48 ), // #497
  INST(Pfcmpgt         , Ext3dNow           , O(000F0F,A0,_,_,_,_,_,_  ), 0                         , 0 , 0 , 1994, 166, 48 ), // #498
  INST(Pfmax           , Ext3dNow           , O(000F0F,A4,_,_,_,_,_,_  ), 0                         , 0 , 0 , 2002, 166, 48 ), // #499
  INST(Pfmin           , Ext3dNow           , O(000F0F,94,_,_,_,_,_,_  ), 0                         , 0 , 0 , 2008, 166, 48 ), // #500
  INST(Pfmul           , Ext3dNow           , O(000F0F,B4,_,_,_,_,_,_  ), 0                         , 0 , 0 , 2014, 166, 48 ), // #501
  INST(Pfnacc          , Ext3dNow           , O(000F0F,8A,_,_,_,_,_,_  ), 0                         , 0 , 0 , 2020, 166, 82 ), // #502
  INST(Pfpnacc         , Ext3dNow           , O(000F0F,8E,_,_,_,_,_,_  ), 0                         , 0 , 0 , 2027, 166, 82 ), // #503
  INST(Pfrcp           , Ext3dNow           , O(000F0F,96,_,_,_,_,_,_  ), 0                         , 0 , 8 , 2035, 175, 48 ), // #504
  INST(Pfrcpit1        , Ext3dNow           , O(000F0F,A6,_,_,_,_,_,_  ), 0                         , 0 , 0 , 2041, 166, 48 ), // #505
  INST(Pfrcpit2        , Ext3dNow           , O(000F0F,B6,_,_,_,_,_,_  ), 0                         , 0 , 0 , 2050, 166, 48 ), // #506
  INST(Pfrcpv          , Ext3dNow           , O(000F0F,86,_,_,_,_,_,_  ), 0                         , 0 , 0 , 2059, 166, 83 ), // #507
  INST(Pfrsqit1        , Ext3dNow           , O(000F0F,A7,_,_,_,_,_,_  ), 0                         , 0 , 0 , 2066, 176, 48 ), // #508
  INST(Pfrsqrt         , Ext3dNow           , O(000F0F,97,_,_,_,_,_,_  ), 0                         , 0 , 0 , 2075, 176, 48 ), // #509
  INST(Pfrsqrtv        , Ext3dNow           , O(000F0F,87,_,_,_,_,_,_  ), 0                         , 0 , 0 , 2083, 166, 83 ), // #510
  INST(Pfsub           , Ext3dNow           , O(000F0F,9A,_,_,_,_,_,_  ), 0                         , 0 , 0 , 2092, 166, 48 ), // #511
  INST(Pfsubr          , Ext3dNow           , O(000F0F,AA,_,_,_,_,_,_  ), 0                         , 0 , 0 , 2098, 166, 48 ), // #512
  INST(Phaddd          , ExtRm_P            , O(000F38,02,_,_,_,_,_,_  ), 0                         , 0 , 0 , 7031, 162, 76 ), // #513
  INST(Phaddsw         , ExtRm_P            , O(000F38,03,_,_,_,_,_,_  ), 0                         , 0 , 0 , 7048, 162, 76 ), // #514
  INST(Phaddw          , ExtRm_P            , O(000F38,01,_,_,_,_,_,_  ), 0                         , 0 , 0 , 7117, 162, 76 ), // #515
  INST(Phminposuw      , ExtRm              , O(660F38,41,_,_,_,_,_,_  ), 0                         , 0 , 0 , 7143, 8  , 12 ), // #516
  INST(Phsubd          , ExtRm_P            , O(000F38,06,_,_,_,_,_,_  ), 0                         , 0 , 0 , 7164, 162, 76 ), // #517
  INST(Phsubsw         , ExtRm_P            , O(000F38,07,_,_,_,_,_,_  ), 0                         , 0 , 0 , 7181, 162, 76 ), // #518
  INST(Phsubw          , ExtRm_P            , O(000F38,05,_,_,_,_,_,_  ), 0                         , 0 , 0 , 7190, 162, 76 ), // #519
  INST(Pi2fd           , Ext3dNow           , O(000F0F,0D,_,_,_,_,_,_  ), 0                         , 0 , 8 , 2105, 175, 48 ), // #520
  INST(Pi2fw           , Ext3dNow           , O(000F0F,0C,_,_,_,_,_,_  ), 0                         , 0 , 8 , 2111, 175, 82 ), // #521
  INST(Pinsrb          , ExtRmi             , O(660F3A,20,_,_,_,_,_,_  ), 0                         , 0 , 0 , 7207, 177, 12 ), // #522
  INST(Pinsrd          , ExtRmi             , O(660F3A,22,_,_,_,_,_,_  ), 0                         , 0 , 0 , 7215, 178, 12 ), // #523
  INST(Pinsrq          , ExtRmi             , O(660F3A,22,_,_,1,_,_,_  ), 0                         , 0 , 0 , 7223, 179, 12 ), // #524
  INST(Pinsrw          , ExtRmi_P           , O(000F00,C4,_,_,_,_,_,_  ), 0                         , 0 , 0 , 7231, 180, 77 ), // #525
  INST(Pmaddubsw       , ExtRm_P            , O(000F38,04,_,_,_,_,_,_  ), 0                         , 0 , 0 , 7401, 162, 76 ), // #526
  INST(Pmaddwd         , ExtRm_P            , O(000F00,F5,_,_,_,_,_,_  ), 0                         , 0 , 0 , 7412, 162, 73 ), // #527
  INST(Pmaxsb          , ExtRm              , O(660F38,3C,_,_,_,_,_,_  ), 0                         , 0 , 0 , 7443, 12 , 12 ), // #528
  INST(Pmaxsd          , ExtRm              , O(660F38,3D,_,_,_,_,_,_  ), 0                         , 0 , 0 , 7451, 12 , 12 ), // #529
  INST(Pmaxsw          , ExtRm_P            , O(000F00,EE,_,_,_,_,_,_  ), 0                         , 0 , 0 , 7467, 164, 77 ), // #530
  INST(Pmaxub          , ExtRm_P            , O(000F00,DE,_,_,_,_,_,_  ), 0                         , 0 , 0 , 7475, 164, 77 ), // #531
  INST(Pmaxud          , ExtRm              , O(660F38,3F,_,_,_,_,_,_  ), 0                         , 0 , 0 , 7483, 12 , 12 ), // #532
  INST(Pmaxuw          , ExtRm              , O(660F38,3E,_,_,_,_,_,_  ), 0                         , 0 , 0 , 7499, 12 , 12 ), // #533
  INST(Pminsb          , ExtRm              , O(660F38,38,_,_,_,_,_,_  ), 0                         , 0 , 0 , 7507, 12 , 12 ), // #534
  INST(Pminsd          , ExtRm              , O(660F38,39,_,_,_,_,_,_  ), 0                         , 0 , 0 , 7515, 12 , 12 ), // #535
  INST(Pminsw          , ExtRm_P            , O(000F00,EA,_,_,_,_,_,_  ), 0                         , 0 , 0 , 7531, 164, 77 ), // #536
  INST(Pminub          , ExtRm_P            , O(000F00,DA,_,_,_,_,_,_  ), 0                         , 0 , 0 , 7539, 164, 77 ), // #537
  INST(Pminud          , ExtRm              , O(660F38,3B,_,_,_,_,_,_  ), 0                         , 0 , 0 , 7547, 12 , 12 ), // #538
  INST(Pminuw          , ExtRm              , O(660F38,3A,_,_,_,_,_,_  ), 0                         , 0 , 0 , 7563, 12 , 12 ), // #539
  INST(Pmovmskb        , ExtRm_P            , O(000F00,D7,_,_,_,_,_,_  ), 0                         , 0 , 8 , 7641, 181, 77 ), // #540
  INST(Pmovsxbd        , ExtRm              , O(660F38,21,_,_,_,_,_,_  ), 0                         , 0 , 16, 7738, 182, 12 ), // #541
  INST(Pmovsxbq        , ExtRm              , O(660F38,22,_,_,_,_,_,_  ), 0                         , 0 , 16, 7748, 183, 12 ), // #542
  INST(Pmovsxbw        , ExtRm              , O(660F38,20,_,_,_,_,_,_  ), 0                         , 0 , 16, 7758, 50 , 12 ), // #543
  INST(Pmovsxdq        , ExtRm              , O(660F38,25,_,_,_,_,_,_  ), 0                         , 0 , 16, 7768, 50 , 12 ), // #544
  INST(Pmovsxwd        , ExtRm              , O(660F38,23,_,_,_,_,_,_  ), 0                         , 0 , 16, 7778, 50 , 12 ), // #545
  INST(Pmovsxwq        , ExtRm              , O(660F38,24,_,_,_,_,_,_  ), 0                         , 0 , 16, 7788, 182, 12 ), // #546
  INST(Pmovzxbd        , ExtRm              , O(660F38,31,_,_,_,_,_,_  ), 0                         , 0 , 16, 7875, 182, 12 ), // #547
  INST(Pmovzxbq        , ExtRm              , O(660F38,32,_,_,_,_,_,_  ), 0                         , 0 , 16, 7885, 183, 12 ), // #548
  INST(Pmovzxbw        , ExtRm              , O(660F38,30,_,_,_,_,_,_  ), 0                         , 0 , 16, 7895, 50 , 12 ), // #549
  INST(Pmovzxdq        , ExtRm              , O(660F38,35,_,_,_,_,_,_  ), 0                         , 0 , 16, 7905, 50 , 12 ), // #550
  INST(Pmovzxwd        , ExtRm              , O(660F38,33,_,_,_,_,_,_  ), 0                         , 0 , 16, 7915, 50 , 12 ), // #551
  INST(Pmovzxwq        , ExtRm              , O(660F38,34,_,_,_,_,_,_  ), 0                         , 0 , 16, 7925, 182, 12 ), // #552
  INST(Pmuldq          , ExtRm              , O(660F38,28,_,_,_,_,_,_  ), 0                         , 0 , 0 , 7935, 5  , 12 ), // #553
  INST(Pmulhrsw        , ExtRm_P            , O(000F38,0B,_,_,_,_,_,_  ), 0                         , 0 , 0 , 7943, 162, 76 ), // #554
  INST(Pmulhrw         , Ext3dNow           , O(000F0F,B7,_,_,_,_,_,_  ), 0                         , 0 , 0 , 2117, 166, 48 ), // #555
  INST(Pmulhuw         , ExtRm_P            , O(000F00,E4,_,_,_,_,_,_  ), 0                         , 0 , 0 , 7953, 162, 77 ), // #556
  INST(Pmulhw          , ExtRm_P            , O(000F00,E5,_,_,_,_,_,_  ), 0                         , 0 , 0 , 7962, 162, 73 ), // #557
  INST(Pmulld          , ExtRm              , O(660F38,40,_,_,_,_,_,_  ), 0                         , 0 , 0 , 7970, 5  , 12 ), // #558
  INST(Pmullw          , ExtRm_P            , O(000F00,D5,_,_,_,_,_,_  ), 0                         , 0 , 0 , 7986, 162, 73 ), // #559
  INST(Pmuludq         , ExtRm_P            , O(000F00,F4,_,_,_,_,_,_  ), 0                         , 0 , 0 , 8009, 162, 4  ), // #560
  INST(Pop             , X86Pop             , O(000000,8F,0,_,_,_,_,_  ), O(000000,58,_,_,_,_,_,_  ), 0 , 0 , 2125, 184, 0  ), // #561
  INST(Popa            , X86Op              , O(660000,61,_,_,_,_,_,_  ), 0                         , 0 , 0 , 2129, 185, 0  ), // #562
  INST(Popad           , X86Op              , O(000000,61,_,_,_,_,_,_  ), 0                         , 0 , 0 , 2134, 185, 0  ), // #563
  INST(Popcnt          , X86Rm_Raw66H       , O(F30F00,B8,_,_,x,_,_,_  ), 0                         , 0 , 0 , 2140, 24 , 84 ), // #564
  INST(Popf            , X86Op              , O(660000,9D,_,_,_,_,_,_  ), 0                         , 0 , 0 , 2147, 33 , 1  ), // #565
  INST(Popfd           , X86Op              , O(000000,9D,_,_,_,_,_,_  ), 0                         , 0 , 0 , 2152, 185, 1  ), // #566
  INST(Popfq           , X86Op              , O(000000,9D,_,_,_,_,_,_  ), 0                         , 0 , 0 , 2158, 186, 1  ), // #567
  INST(Por             , ExtRm_P            , O(000F00,EB,_,_,_,_,_,_  ), 0                         , 0 , 0 , 8054, 164, 73 ), // #568
  INST(Prefetch        , X86M_Only          , O(000F00,0D,0,_,_,_,_,_  ), 0                         , 0 , 0 , 2164, 83 , 48 ), // #569
  INST(Prefetchnta     , X86M_Only          , O(000F00,18,0,_,_,_,_,_  ), 0                         , 0 , 0 , 2173, 83 , 69 ), // #570
  INST(Prefetcht0      , X86M_Only          , O(000F00,18,1,_,_,_,_,_  ), 0                         , 0 , 0 , 2185, 83 , 69 ), // #571
  INST(Prefetcht1      , X86M_Only          , O(000F00,18,2,_,_,_,_,_  ), 0                         , 0 , 0 , 2196, 83 , 69 ), // #572
  INST(Prefetcht2      , X86M_Only          , O(000F00,18,3,_,_,_,_,_  ), 0                         , 0 , 0 , 2207, 83 , 69 ), // #573
  INST(Prefetchw       , X86M_Only          , O(000F00,0D,1,_,_,_,_,_  ), 0                         , 0 , 0 , 2218, 83 , 85 ), // #574
  INST(Prefetchwt1     , X86M_Only          , O(000F00,0D,2,_,_,_,_,_  ), 0                         , 0 , 0 , 2228, 83 , 86 ), // #575
  INST(Psadbw          , ExtRm_P            , O(000F00,F6,_,_,_,_,_,_  ), 0                         , 0 , 0 , 3832, 162, 77 ), // #576
  INST(Pshufb          , ExtRm_P            , O(000F38,00,_,_,_,_,_,_  ), 0                         , 0 , 0 , 8380, 162, 76 ), // #577
  INST(Pshufd          , ExtRmi             , O(660F00,70,_,_,_,_,_,_  ), 0                         , 0 , 16, 8401, 187, 4  ), // #578
  INST(Pshufhw         , ExtRmi             , O(F30F00,70,_,_,_,_,_,_  ), 0                         , 0 , 16, 8409, 187, 4  ), // #579
  INST(Pshuflw         , ExtRmi             , O(F20F00,70,_,_,_,_,_,_  ), 0                         , 0 , 16, 8418, 187, 4  ), // #580
  INST(Pshufw          , ExtRmi_P           , O(000F00,70,_,_,_,_,_,_  ), 0                         , 0 , 8 , 2240, 188, 69 ), // #581
  INST(Psignb          , ExtRm_P            , O(000F38,08,_,_,_,_,_,_  ), 0                         , 0 , 0 , 8427, 162, 76 ), // #582
  INST(Psignd          , ExtRm_P            , O(000F38,0A,_,_,_,_,_,_  ), 0                         , 0 , 0 , 8435, 162, 76 ), // #583
  INST(Psignw          , ExtRm_P            , O(000F38,09,_,_,_,_,_,_  ), 0                         , 0 , 0 , 8443, 162, 76 ), // #584
  INST(Pslld           , ExtRmRi_P          , O(000F00,F2,_,_,_,_,_,_  ), O(000F00,72,6,_,_,_,_,_  ), 0 , 0 , 8451, 189, 73 ), // #585
  INST(Pslldq          , ExtRmRi            , 0                         , O(660F00,73,7,_,_,_,_,_  ), 0 , 0 , 8458, 190, 4  ), // #586
  INST(Psllq           , ExtRmRi_P          , O(000F00,F3,_,_,_,_,_,_  ), O(000F00,73,6,_,_,_,_,_  ), 0 , 0 , 8466, 189, 73 ), // #587
  INST(Psllw           , ExtRmRi_P          , O(000F00,F1,_,_,_,_,_,_  ), O(000F00,71,6,_,_,_,_,_  ), 0 , 0 , 8497, 189, 73 ), // #588
  INST(Psrad           , ExtRmRi_P          , O(000F00,E2,_,_,_,_,_,_  ), O(000F00,72,4,_,_,_,_,_  ), 0 , 0 , 8504, 189, 73 ), // #589
  INST(Psraw           , ExtRmRi_P          , O(000F00,E1,_,_,_,_,_,_  ), O(000F00,71,4,_,_,_,_,_  ), 0 , 0 , 8542, 189, 73 ), // #590
  INST(Psrld           , ExtRmRi_P          , O(000F00,D2,_,_,_,_,_,_  ), O(000F00,72,2,_,_,_,_,_  ), 0 , 0 , 8549, 189, 73 ), // #591
  INST(Psrldq          , ExtRmRi            , 0                         , O(660F00,73,3,_,_,_,_,_  ), 0 , 0 , 8556, 190, 4  ), // #592
  INST(Psrlq           , ExtRmRi_P          , O(000F00,D3,_,_,_,_,_,_  ), O(000F00,73,2,_,_,_,_,_  ), 0 , 0 , 8564, 189, 73 ), // #593
  INST(Psrlw           , ExtRmRi_P          , O(000F00,D1,_,_,_,_,_,_  ), O(000F00,71,2,_,_,_,_,_  ), 0 , 0 , 8595, 189, 73 ), // #594
  INST(Psubb           , ExtRm_P            , O(000F00,F8,_,_,_,_,_,_  ), 0                         , 0 , 0 , 8602, 165, 73 ), // #595
  INST(Psubd           , ExtRm_P            , O(000F00,FA,_,_,_,_,_,_  ), 0                         , 0 , 0 , 8609, 165, 73 ), // #596
  INST(Psubq           , ExtRm_P            , O(000F00,FB,_,_,_,_,_,_  ), 0                         , 0 , 0 , 8616, 165, 4  ), // #597
  INST(Psubsb          , ExtRm_P            , O(000F00,E8,_,_,_,_,_,_  ), 0                         , 0 , 0 , 8623, 165, 73 ), // #598
  INST(Psubsw          , ExtRm_P            , O(000F00,E9,_,_,_,_,_,_  ), 0                         , 0 , 0 , 8631, 165, 73 ), // #599
  INST(Psubusb         , ExtRm_P            , O(000F00,D8,_,_,_,_,_,_  ), 0                         , 0 , 0 , 8639, 165, 73 ), // #600
  INST(Psubusw         , ExtRm_P            , O(000F00,D9,_,_,_,_,_,_  ), 0                         , 0 , 0 , 8648, 165, 73 ), // #601
  INST(Psubw           , ExtRm_P            , O(000F00,F9,_,_,_,_,_,_  ), 0                         , 0 , 0 , 8657, 165, 73 ), // #602
  INST(Pswapd          , Ext3dNow           , O(000F0F,BB,_,_,_,_,_,_  ), 0                         , 0 , 8 , 2247, 175, 82 ), // #603
  INST(Ptest           , ExtRm              , O(660F38,17,_,_,_,_,_,_  ), 0                         , 0 , 0 , 8686, 191, 87 ), // #604
  INST(Punpckhbw       , ExtRm_P            , O(000F00,68,_,_,_,_,_,_  ), 0                         , 0 , 0 , 8769, 162, 73 ), // #605
  INST(Punpckhdq       , ExtRm_P            , O(000F00,6A,_,_,_,_,_,_  ), 0                         , 0 , 0 , 8780, 162, 73 ), // #606
  INST(Punpckhqdq      , ExtRm              , O(660F00,6D,_,_,_,_,_,_  ), 0                         , 0 , 0 , 8791, 5  , 4  ), // #607
  INST(Punpckhwd       , ExtRm_P            , O(000F00,69,_,_,_,_,_,_  ), 0                         , 0 , 0 , 8803, 162, 73 ), // #608
  INST(Punpcklbw       , ExtRm_P            , O(000F00,60,_,_,_,_,_,_  ), 0                         , 0 , 0 , 8814, 162, 73 ), // #609
  INST(Punpckldq       , ExtRm_P            , O(000F00,62,_,_,_,_,_,_  ), 0                         , 0 , 0 , 8825, 162, 73 ), // #610
  INST(Punpcklqdq      , ExtRm              , O(660F00,6C,_,_,_,_,_,_  ), 0                         , 0 , 0 , 8836, 5  , 4  ), // #611
  INST(Punpcklwd       , ExtRm_P            , O(000F00,61,_,_,_,_,_,_  ), 0                         , 0 , 0 , 8848, 162, 73 ), // #612
  INST(Push            , X86Push            , O(000000,FF,6,_,_,_,_,_  ), O(000000,50,_,_,_,_,_,_  ), 0 , 0 , 2254, 192, 0  ), // #613
  INST(Pusha           , X86Op              , O(660000,60,_,_,_,_,_,_  ), 0                         , 0 , 0 , 2259, 185, 0  ), // #614
  INST(Pushad          , X86Op              , O(000000,60,_,_,_,_,_,_  ), 0                         , 0 , 0 , 2265, 185, 0  ), // #615
  INST(Pushf           , X86Op              , O(660000,9C,_,_,_,_,_,_  ), 0                         , 0 , 0 , 2272, 33 , 0  ), // #616
  INST(Pushfd          , X86Op              , O(000000,9C,_,_,_,_,_,_  ), 0                         , 0 , 0 , 2278, 185, 0  ), // #617
  INST(Pushfq          , X86Op              , O(000000,9C,_,_,_,_,_,_  ), 0                         , 0 , 0 , 2285, 186, 0  ), // #618
  INST(Pxor            , ExtRm_P            , O(000F00,EF,_,_,_,_,_,_  ), 0                         , 0 , 0 , 8859, 165, 73 ), // #619
  INST(Rcl             , X86Rot             , O(000000,D0,2,_,x,_,_,_  ), 0                         , 0 , 0 , 2292, 193, 88 ), // #620
  INST(Rcpps           , ExtRm              , O(000F00,53,_,_,_,_,_,_  ), 0                         , 0 , 16, 8987, 51 , 5  ), // #621
  INST(Rcpss           , ExtRm              , O(F30F00,53,_,_,_,_,_,_  ), 0                         , 0 , 4 , 8994, 194, 5  ), // #622
  INST(Rcr             , X86Rot             , O(000000,D0,3,_,x,_,_,_  ), 0                         , 0 , 0 , 2296, 193, 88 ), // #623
  INST(Rdfsbase        , X86M               , O(F30F00,AE,0,_,x,_,_,_  ), 0                         , 0 , 8 , 2300, 195, 89 ), // #624
  INST(Rdgsbase        , X86M               , O(F30F00,AE,1,_,x,_,_,_  ), 0                         , 0 , 8 , 2309, 195, 89 ), // #625
  INST(Rdmsr           , X86Op              , O(000F00,32,_,_,_,_,_,_  ), 0                         , 0 , 0 , 2318, 196, 90 ), // #626
  INST(Rdpmc           , X86Op              , O(000F00,33,_,_,_,_,_,_  ), 0                         , 0 , 0 , 2324, 196, 0  ), // #627
  INST(Rdrand          , X86M               , O(000F00,C7,6,_,x,_,_,_  ), 0                         , 0 , 8 , 2330, 197, 91 ), // #628
  INST(Rdseed          , X86M               , O(000F00,C7,7,_,x,_,_,_  ), 0                         , 0 , 8 , 2337, 197, 92 ), // #629
  INST(Rdtsc           , X86Op              , O(000F00,31,_,_,_,_,_,_  ), 0                         , 0 , 0 , 2344, 198, 93 ), // #630
  INST(Rdtscp          , X86Op              , O(000F01,F9,_,_,_,_,_,_  ), 0                         , 0 , 0 , 2350, 199, 94 ), // #631
  INST(Ret             , X86Ret             , O(000000,C2,_,_,_,_,_,_  ), 0                         , 0 , 0 , 2764, 200, 0  ), // #632
  INST(Rol             , X86Rot             , O(000000,D0,0,_,x,_,_,_  ), 0                         , 0 , 0 , 2357, 193, 95 ), // #633
  INST(Ror             , X86Rot             , O(000000,D0,1,_,x,_,_,_  ), 0                         , 0 , 0 , 2361, 193, 95 ), // #634
  INST(Rorx            , VexRmi_Wx          , V(F20F3A,F0,_,0,x,_,_,_  ), 0                         , 0 , 0 , 2365, 201, 74 ), // #635
  INST(Roundpd         , ExtRmi             , O(660F3A,09,_,_,_,_,_,_  ), 0                         , 0 , 16, 9089, 187, 12 ), // #636
  INST(Roundps         , ExtRmi             , O(660F3A,08,_,_,_,_,_,_  ), 0                         , 0 , 16, 9098, 187, 12 ), // #637
  INST(Roundsd         , ExtRmi             , O(660F3A,0B,_,_,_,_,_,_  ), 0                         , 0 , 8 , 9107, 202, 12 ), // #638
  INST(Roundss         , ExtRmi             , O(660F3A,0A,_,_,_,_,_,_  ), 0                         , 0 , 4 , 9116, 203, 12 ), // #639
  INST(Rsm             , X86Op              , O(000F00,AA,_,_,_,_,_,_  ), 0                         , 0 , 0 , 2370, 99 , 1  ), // #640
  INST(Rsqrtps         , ExtRm              , O(000F00,52,_,_,_,_,_,_  ), 0                         , 0 , 16, 9213, 51 , 5  ), // #641
  INST(Rsqrtss         , ExtRm              , O(F30F00,52,_,_,_,_,_,_  ), 0                         , 0 , 4 , 9222, 194, 5  ), // #642
  INST(Sahf            , X86Op              , O(000000,9E,_,_,_,_,_,_  ), 0                         , 0 , 0 , 2374, 204, 96 ), // #643
  INST(Sal             , X86Rot             , O(000000,D0,4,_,x,_,_,_  ), 0                         , 0 , 0 , 2379, 193, 1  ), // #644
  INST(Sar             , X86Rot             , O(000000,D0,7,_,x,_,_,_  ), 0                         , 0 , 0 , 2383, 193, 1  ), // #645
  INST(Sarx            , VexRmv_Wx          , V(F30F38,F7,_,0,x,_,_,_  ), 0                         , 0 , 0 , 2387, 14 , 74 ), // #646
  INST(Sbb             , X86Arith           , O(000000,18,3,_,x,_,_,_  ), 0                         , 0 , 0 , 2392, 3  , 2  ), // #647
  INST(Scas            , X86StrRm           , O(000000,AE,_,_,_,_,_,_  ), 0                         , 0 , 0 , 2396, 205, 34 ), // #648
  INST(Seta            , X86Set             , O(000F00,97,_,_,_,_,_,_  ), 0                         , 0 , 1 , 2401, 206, 53 ), // #649
  INST(Setae           , X86Set             , O(000F00,93,_,_,_,_,_,_  ), 0                         , 0 , 1 , 2406, 206, 54 ), // #650
  INST(Setb            , X86Set             , O(000F00,92,_,_,_,_,_,_  ), 0                         , 0 , 1 , 2412, 206, 54 ), // #651
  INST(Setbe           , X86Set             , O(000F00,96,_,_,_,_,_,_  ), 0                         , 0 , 1 , 2417, 206, 53 ), // #652
  INST(Setc            , X86Set             , O(000F00,92,_,_,_,_,_,_  ), 0                         , 0 , 1 , 2423, 206, 54 ), // #653
  INST(Sete            , X86Set             , O(000F00,94,_,_,_,_,_,_  ), 0                         , 0 , 1 , 2428, 206, 55 ), // #654
  INST(Setg            , X86Set             , O(000F00,9F,_,_,_,_,_,_  ), 0                         , 0 , 1 , 2433, 206, 56 ), // #655
  INST(Setge           , X86Set             , O(000F00,9D,_,_,_,_,_,_  ), 0                         , 0 , 1 , 2438, 206, 57 ), // #656
  INST(Setl            , X86Set             , O(000F00,9C,_,_,_,_,_,_  ), 0                         , 0 , 1 , 2444, 206, 57 ), // #657
  INST(Setle           , X86Set             , O(000F00,9E,_,_,_,_,_,_  ), 0                         , 0 , 1 , 2449, 206, 56 ), // #658
  INST(Setna           , X86Set             , O(000F00,96,_,_,_,_,_,_  ), 0                         , 0 , 1 , 2455, 206, 53 ), // #659
  INST(Setnae          , X86Set             , O(000F00,92,_,_,_,_,_,_  ), 0                         , 0 , 1 , 2461, 206, 54 ), // #660
  INST(Setnb           , X86Set             , O(000F00,93,_,_,_,_,_,_  ), 0                         , 0 , 1 , 2468, 206, 54 ), // #661
  INST(Setnbe          , X86Set             , O(000F00,97,_,_,_,_,_,_  ), 0                         , 0 , 1 , 2474, 206, 53 ), // #662
  INST(Setnc           , X86Set             , O(000F00,93,_,_,_,_,_,_  ), 0                         , 0 , 1 , 2481, 206, 54 ), // #663
  INST(Setne           , X86Set             , O(000F00,95,_,_,_,_,_,_  ), 0                         , 0 , 1 , 2487, 206, 55 ), // #664
  INST(Setng           , X86Set             , O(000F00,9E,_,_,_,_,_,_  ), 0                         , 0 , 1 , 2493, 206, 56 ), // #665
  INST(Setnge          , X86Set             , O(000F00,9C,_,_,_,_,_,_  ), 0                         , 0 , 1 , 2499, 206, 57 ), // #666
  INST(Setnl           , X86Set             , O(000F00,9D,_,_,_,_,_,_  ), 0                         , 0 , 1 , 2506, 206, 57 ), // #667
  INST(Setnle          , X86Set             , O(000F00,9F,_,_,_,_,_,_  ), 0                         , 0 , 1 , 2512, 206, 56 ), // #668
  INST(Setno           , X86Set             , O(000F00,91,_,_,_,_,_,_  ), 0                         , 0 , 1 , 2519, 206, 52 ), // #669
  INST(Setnp           , X86Set             , O(000F00,9B,_,_,_,_,_,_  ), 0                         , 0 , 1 , 2525, 206, 58 ), // #670
  INST(Setns           , X86Set             , O(000F00,99,_,_,_,_,_,_  ), 0                         , 0 , 1 , 2531, 206, 59 ), // #671
  INST(Setnz           , X86Set             , O(000F00,95,_,_,_,_,_,_  ), 0                         , 0 , 1 , 2537, 206, 55 ), // #672
  INST(Seto            , X86Set             , O(000F00,90,_,_,_,_,_,_  ), 0                         , 0 , 1 , 2543, 206, 52 ), // #673
  INST(Setp            , X86Set             , O(000F00,9A,_,_,_,_,_,_  ), 0                         , 0 , 1 , 2548, 206, 58 ), // #674
  INST(Setpe           , X86Set             , O(000F00,9A,_,_,_,_,_,_  ), 0                         , 0 , 1 , 2553, 206, 58 ), // #675
  INST(Setpo           , X86Set             , O(000F00,9B,_,_,_,_,_,_  ), 0                         , 0 , 1 , 2559, 206, 58 ), // #676
  INST(Sets            , X86Set             , O(000F00,98,_,_,_,_,_,_  ), 0                         , 0 , 1 , 2565, 206, 59 ), // #677
  INST(Setz            , X86Set             , O(000F00,94,_,_,_,_,_,_  ), 0                         , 0 , 1 , 2570, 206, 55 ), // #678
  INST(Sfence          , X86Fence           , O(000F00,AE,7,_,_,_,_,_  ), 0                         , 0 , 0 , 2575, 32 , 69 ), // #679
  INST(Sgdt            , X86M_Only          , O(000F00,01,0,_,_,_,_,_  ), 0                         , 0 , 0 , 2582, 92 , 0  ), // #680
  INST(Sha1msg1        , ExtRm              , O(000F38,C9,_,_,_,_,_,_  ), 0                         , 0 , 0 , 2587, 5  , 97 ), // #681
  INST(Sha1msg2        , ExtRm              , O(000F38,CA,_,_,_,_,_,_  ), 0                         , 0 , 0 , 2596, 5  , 97 ), // #682
  INST(Sha1nexte       , ExtRm              , O(000F38,C8,_,_,_,_,_,_  ), 0                         , 0 , 0 , 2605, 5  , 97 ), // #683
  INST(Sha1rnds4       , ExtRmi             , O(000F3A,CC,_,_,_,_,_,_  ), 0                         , 0 , 0 , 2615, 16 , 97 ), // #684
  INST(Sha256msg1      , ExtRm              , O(000F38,CC,_,_,_,_,_,_  ), 0                         , 0 , 0 , 2625, 5  , 97 ), // #685
  INST(Sha256msg2      , ExtRm              , O(000F38,CD,_,_,_,_,_,_  ), 0                         , 0 , 0 , 2636, 5  , 97 ), // #686
  INST(Sha256rnds2     , ExtRm_XMM0         , O(000F38,CB,_,_,_,_,_,_  ), 0                         , 0 , 0 , 2647, 17 , 97 ), // #687
  INST(Shl             , X86Rot             , O(000000,D0,4,_,x,_,_,_  ), 0                         , 0 , 0 , 2659, 193, 1  ), // #688
  INST(Shld            , X86ShldShrd        , O(000F00,A4,_,_,x,_,_,_  ), 0                         , 0 , 0 , 8258, 207, 1  ), // #689
  INST(Shlx            , VexRmv_Wx          , V(660F38,F7,_,0,x,_,_,_  ), 0                         , 0 , 0 , 2663, 14 , 74 ), // #690
  INST(Shr             , X86Rot             , O(000000,D0,5,_,x,_,_,_  ), 0                         , 0 , 0 , 2668, 193, 1  ), // #691
  INST(Shrd            , X86ShldShrd        , O(000F00,AC,_,_,x,_,_,_  ), 0                         , 0 , 0 , 2672, 207, 1  ), // #692
  INST(Shrx            , VexRmv_Wx          , V(F20F38,F7,_,0,x,_,_,_  ), 0                         , 0 , 0 , 2677, 14 , 74 ), // #693
  INST(Shufpd          , ExtRmi             , O(660F00,C6,_,_,_,_,_,_  ), 0                         , 0 , 0 , 9483, 16 , 4  ), // #694
  INST(Shufps          , ExtRmi             , O(000F00,C6,_,_,_,_,_,_  ), 0                         , 0 , 0 , 9491, 16 , 5  ), // #695
  INST(Sidt            , X86M_Only          , O(000F00,01,1,_,_,_,_,_  ), 0                         , 0 , 0 , 2682, 92 , 0  ), // #696
  INST(Sldt            , X86M               , O(000F00,00,0,_,_,_,_,_  ), 0                         , 0 , 0 , 2687, 208, 0  ), // #697
  INST(Smsw            , X86M               , O(000F00,01,4,_,_,_,_,_  ), 0                         , 0 , 0 , 2692, 208, 0  ), // #698
  INST(Sqrtpd          , ExtRm              , O(660F00,51,_,_,_,_,_,_  ), 0                         , 0 , 16, 9499, 51 , 4  ), // #699
  INST(Sqrtps          , ExtRm              , O(000F00,51,_,_,_,_,_,_  ), 0                         , 0 , 16, 9214, 51 , 5  ), // #700
  INST(Sqrtsd          , ExtRm              , O(F20F00,51,_,_,_,_,_,_  ), 0                         , 0 , 8 , 9515, 209, 4  ), // #701
  INST(Sqrtss          , ExtRm              , O(F30F00,51,_,_,_,_,_,_  ), 0                         , 0 , 4 , 9223, 194, 5  ), // #702
  INST(Stac            , X86Op              , O(000F01,CB,_,_,_,_,_,_  ), 0                         , 0 , 0 , 2697, 32 , 17 ), // #703
  INST(Stc             , X86Op              , O(000000,F9,_,_,_,_,_,_  ), 0                         , 0 , 0 , 2702, 33 , 18 ), // #704
  INST(Std             , X86Op              , O(000000,FD,_,_,_,_,_,_  ), 0                         , 0 , 0 , 6279, 33 , 19 ), // #705
  INST(Sti             , X86Op              , O(000000,FB,_,_,_,_,_,_  ), 0                         , 0 , 0 , 2706, 32 , 22 ), // #706
  INST(Stmxcsr         , X86M_Only          , O(000F00,AE,3,_,_,_,_,_  ), 0                         , 0 , 0 , 9531, 210, 5  ), // #707
  INST(Stos            , X86StrMr           , O(000000,AA,_,_,_,_,_,_  ), 0                         , 0 , 0 , 2710, 211, 67 ), // #708
  INST(Str             , X86M               , O(000F00,00,1,_,_,_,_,_  ), 0                         , 0 , 0 , 2715, 208, 0  ), // #709
  INST(Sub             , X86Arith           , O(000000,28,5,_,x,_,_,_  ), 0                         , 0 , 0 , 807 , 212, 1  ), // #710
  INST(Subpd           , ExtRm              , O(660F00,5C,_,_,_,_,_,_  ), 0                         , 0 , 0 , 4398, 5  , 4  ), // #711
  INST(Subps           , ExtRm              , O(000F00,5C,_,_,_,_,_,_  ), 0                         , 0 , 0 , 4410, 5  , 5  ), // #712
  INST(Subsd           , ExtRm              , O(F20F00,5C,_,_,_,_,_,_  ), 0                         , 0 , 0 , 5086, 6  , 4  ), // #713
  INST(Subss           , ExtRm              , O(F30F00,5C,_,_,_,_,_,_  ), 0                         , 0 , 0 , 5096, 7  , 5  ), // #714
  INST(Swapgs          , X86Op              , O(000F01,F8,_,_,_,_,_,_  ), 0                         , 0 , 0 , 2719, 213, 0  ), // #715
  INST(Syscall         , X86Op              , O(000F00,05,_,_,_,_,_,_  ), 0                         , 0 , 0 , 2726, 214, 0  ), // #716
  INST(Sysenter        , X86Op              , O(000F00,34,_,_,_,_,_,_  ), 0                         , 0 , 0 , 2734, 32 , 0  ), // #717
  INST(Sysexit         , X86Op              , O(000F00,35,_,_,_,_,_,_  ), 0                         , 0 , 0 , 2743, 35 , 0  ), // #718
  INST(Sysexit64       , X86Op              , O(000F00,35,_,_,_,_,_,_  ), 0                         , 0 , 0 , 2751, 35 , 0  ), // #719
  INST(Sysret          , X86Op              , O(000F00,07,_,_,_,_,_,_  ), 0                         , 0 , 0 , 2761, 213, 0  ), // #720
  INST(Sysret64        , X86Op              , O(000F00,07,_,_,_,_,_,_  ), 0                         , 0 , 0 , 2768, 213, 0  ), // #721
  INST(T1mskc          , VexVm_Wx           , V(XOP_M9,01,7,0,x,_,_,_  ), 0                         , 0 , 0 , 2777, 15 , 11 ), // #722
  INST(Test            , X86Test            , O(000000,84,_,_,x,_,_,_  ), O(000000,F6,_,_,x,_,_,_  ), 0 , 0 , 8687, 215, 1  ), // #723
  INST(Tzcnt           , X86Rm_Raw66H       , O(F30F00,BC,_,_,x,_,_,_  ), 0                         , 0 , 0 , 2784, 24 , 9  ), // #724
  INST(Tzmsk           , VexVm_Wx           , V(XOP_M9,01,4,0,x,_,_,_  ), 0                         , 0 , 0 , 2790, 15 , 11 ), // #725
  INST(Ucomisd         , ExtRm              , O(660F00,2E,_,_,_,_,_,_  ), 0                         , 0 , 0 , 9584, 45 , 38 ), // #726
  INST(Ucomiss         , ExtRm              , O(000F00,2E,_,_,_,_,_,_  ), 0                         , 0 , 0 , 9593, 46 , 39 ), // #727
  INST(Ud2             , X86Op              , O(000F00,0B,_,_,_,_,_,_  ), 0                         , 0 , 0 , 2796, 33 , 0  ), // #728
  INST(Unpckhpd        , ExtRm              , O(660F00,15,_,_,_,_,_,_  ), 0                         , 0 , 0 , 9602, 5  , 4  ), // #729
  INST(Unpckhps        , ExtRm              , O(000F00,15,_,_,_,_,_,_  ), 0                         , 0 , 0 , 9612, 5  , 5  ), // #730
  INST(Unpcklpd        , ExtRm              , O(660F00,14,_,_,_,_,_,_  ), 0                         , 0 , 0 , 9622, 5  , 4  ), // #731
  INST(Unpcklps        , ExtRm              , O(000F00,14,_,_,_,_,_,_  ), 0                         , 0 , 0 , 9632, 5  , 5  ), // #732
  INST(V4fmaddps       , VexRm_T1_4X        , V(F20F38,9A,_,2,_,0,2,T4X), 0                         , 0 , 0 , 2800, 216, 98 ), // #733
  INST(V4fmaddss       , VexRm_T1_4X        , V(F20F38,9B,_,2,_,0,2,T4X), 0                         , 0 , 0 , 2810, 217, 98 ), // #734
  INST(V4fnmaddps      , VexRm_T1_4X        , V(F20F38,AA,_,2,_,0,2,T4X), 0                         , 0 , 0 , 2820, 216, 98 ), // #735
  INST(V4fnmaddss      , VexRm_T1_4X        , V(F20F38,AB,_,2,_,0,2,T4X), 0                         , 0 , 0 , 2831, 217, 98 ), // #736
  INST(Vaddpd          , VexRvm_Lx          , V(660F00,58,_,x,I,1,4,FV ), 0                         , 0 , 0 , 2842, 218, 99 ), // #737
  INST(Vaddps          , VexRvm_Lx          , V(000F00,58,_,x,I,0,4,FV ), 0                         , 0 , 0 , 2849, 219, 99 ), // #738
  INST(Vaddsd          , VexRvm             , V(F20F00,58,_,I,I,1,3,T1S), 0                         , 0 , 0 , 2856, 220, 100), // #739
  INST(Vaddss          , VexRvm             , V(F30F00,58,_,I,I,0,2,T1S), 0                         , 0 , 0 , 2863, 221, 100), // #740
  INST(Vaddsubpd       , VexRvm_Lx          , V(660F00,D0,_,x,I,_,_,_  ), 0                         , 0 , 0 , 2870, 222, 101), // #741
  INST(Vaddsubps       , VexRvm_Lx          , V(F20F00,D0,_,x,I,_,_,_  ), 0                         , 0 , 0 , 2880, 222, 101), // #742
  INST(Vaesdec         , VexRvm_Lx          , V(660F38,DE,_,x,I,_,4,FVM), 0                         , 0 , 0 , 2890, 223, 102), // #743
  INST(Vaesdeclast     , VexRvm_Lx          , V(660F38,DF,_,x,I,_,4,FVM), 0                         , 0 , 0 , 2898, 223, 102), // #744
  INST(Vaesenc         , VexRvm_Lx          , V(660F38,DC,_,x,I,_,4,FVM), 0                         , 0 , 0 , 2910, 223, 102), // #745
  INST(Vaesenclast     , VexRvm_Lx          , V(660F38,DD,_,x,I,_,4,FVM), 0                         , 0 , 0 , 2918, 223, 102), // #746
  INST(Vaesimc         , VexRm              , V(660F38,DB,_,0,I,_,_,_  ), 0                         , 0 , 0 , 2930, 224, 103), // #747
  INST(Vaeskeygenassist, VexRmi             , V(660F3A,DF,_,0,I,_,_,_  ), 0                         , 0 , 0 , 2938, 225, 103), // #748
  INST(Valignd         , VexRvmi_Lx         , V(660F3A,03,_,x,_,0,4,FV ), 0                         , 0 , 0 , 2955, 226, 104), // #749
  INST(Valignq         , VexRvmi_Lx         , V(660F3A,03,_,x,_,1,4,FV ), 0                         , 0 , 0 , 2963, 227, 104), // #750
  INST(Vandnpd         , VexRvm_Lx          , V(660F00,55,_,x,I,1,4,FV ), 0                         , 0 , 0 , 2971, 228, 105), // #751
  INST(Vandnps         , VexRvm_Lx          , V(000F00,55,_,x,I,0,4,FV ), 0                         , 0 , 0 , 2979, 229, 105), // #752
  INST(Vandpd          , VexRvm_Lx          , V(660F00,54,_,x,I,1,4,FV ), 0                         , 0 , 0 , 2987, 230, 105), // #753
  INST(Vandps          , VexRvm_Lx          , V(000F00,54,_,x,I,0,4,FV ), 0                         , 0 , 0 , 2994, 231, 105), // #754
  INST(Vblendmb        , VexRvm_Lx          , V(660F38,66,_,x,_,0,4,FVM), 0                         , 0 , 0 , 3001, 232, 106), // #755
  INST(Vblendmd        , VexRvm_Lx          , V(660F38,64,_,x,_,0,4,FV ), 0                         , 0 , 0 , 3010, 233, 104), // #756
  INST(Vblendmpd       , VexRvm_Lx          , V(660F38,65,_,x,_,1,4,FV ), 0                         , 0 , 0 , 3019, 234, 104), // #757
  INST(Vblendmps       , VexRvm_Lx          , V(660F38,65,_,x,_,0,4,FV ), 0                         , 0 , 0 , 3029, 233, 104), // #758
  INST(Vblendmq        , VexRvm_Lx          , V(660F38,64,_,x,_,1,4,FV ), 0                         , 0 , 0 , 3039, 234, 104), // #759
  INST(Vblendmw        , VexRvm_Lx          , V(660F38,66,_,x,_,1,4,FVM), 0                         , 0 , 0 , 3048, 232, 106), // #760
  INST(Vblendpd        , VexRvmi_Lx         , V(660F3A,0D,_,x,I,_,_,_  ), 0                         , 0 , 0 , 3057, 235, 101), // #761
  INST(Vblendps        , VexRvmi_Lx         , V(660F3A,0C,_,x,I,_,_,_  ), 0                         , 0 , 0 , 3066, 235, 101), // #762
  INST(Vblendvpd       , VexRvmr_Lx         , V(660F3A,4B,_,x,0,_,_,_  ), 0                         , 0 , 0 , 3075, 236, 101), // #763
  INST(Vblendvps       , VexRvmr_Lx         , V(660F3A,4A,_,x,0,_,_,_  ), 0                         , 0 , 0 , 3085, 236, 101), // #764
  INST(Vbroadcastf128  , VexRm              , V(660F38,1A,_,1,0,_,_,_  ), 0                         , 0 , 0 , 3095, 237, 101), // #765
  INST(Vbroadcastf32x2 , VexRm_Lx           , V(660F38,19,_,x,_,0,3,T2 ), 0                         , 0 , 0 , 3110, 238, 107), // #766
  INST(Vbroadcastf32x4 , VexRm_Lx           , V(660F38,1A,_,x,_,0,4,T4 ), 0                         , 0 , 0 , 3126, 239, 62 ), // #767
  INST(Vbroadcastf32x8 , VexRm              , V(660F38,1B,_,2,_,0,5,T8 ), 0                         , 0 , 0 , 3142, 240, 60 ), // #768
  INST(Vbroadcastf64x2 , VexRm_Lx           , V(660F38,1A,_,x,_,1,4,T2 ), 0                         , 0 , 0 , 3158, 239, 107), // #769
  INST(Vbroadcastf64x4 , VexRm              , V(660F38,1B,_,2,_,1,5,T4 ), 0                         , 0 , 0 , 3174, 240, 62 ), // #770
  INST(Vbroadcasti128  , VexRm              , V(660F38,5A,_,1,0,_,_,_  ), 0                         , 0 , 0 , 3190, 237, 108), // #771
  INST(Vbroadcasti32x2 , VexRm_Lx           , V(660F38,59,_,x,_,0,3,T2 ), 0                         , 0 , 0 , 3205, 241, 107), // #772
  INST(Vbroadcasti32x4 , VexRm_Lx           , V(660F38,5A,_,x,_,0,4,T4 ), 0                         , 0 , 0 , 3221, 239, 104), // #773
  INST(Vbroadcasti32x8 , VexRm              , V(660F38,5B,_,2,_,0,5,T8 ), 0                         , 0 , 0 , 3237, 240, 60 ), // #774
  INST(Vbroadcasti64x2 , VexRm_Lx           , V(660F38,5A,_,x,_,1,4,T2 ), 0                         , 0 , 0 , 3253, 239, 107), // #775
  INST(Vbroadcasti64x4 , VexRm              , V(660F38,5B,_,2,_,1,5,T4 ), 0                         , 0 , 0 , 3269, 240, 62 ), // #776
  INST(Vbroadcastsd    , VexRm_Lx           , V(660F38,19,_,x,0,1,3,T1S), 0                         , 0 , 0 , 3285, 242, 109), // #777
  INST(Vbroadcastss    , VexRm_Lx           , V(660F38,18,_,x,0,0,2,T1S), 0                         , 0 , 0 , 3298, 243, 109), // #778
  INST(Vcmppd          , VexRvmi_Lx         , V(660F00,C2,_,x,I,1,4,FV ), 0                         , 0 , 0 , 3311, 244, 99 ), // #779
  INST(Vcmpps          , VexRvmi_Lx         , V(000F00,C2,_,x,I,0,4,FV ), 0                         , 0 , 0 , 3318, 245, 99 ), // #780
  INST(Vcmpsd          , VexRvmi            , V(F20F00,C2,_,I,I,1,3,T1S), 0                         , 0 , 0 , 3325, 246, 100), // #781
  INST(Vcmpss          , VexRvmi            , V(F30F00,C2,_,I,I,0,2,T1S), 0                         , 0 , 0 , 3332, 247, 100), // #782
  INST(Vcomisd         , VexRm              , V(660F00,2F,_,I,I,1,3,T1S), 0                         , 0 , 0 , 3339, 248, 110), // #783
  INST(Vcomiss         , VexRm              , V(000F00,2F,_,I,I,0,2,T1S), 0                         , 0 , 0 , 3347, 249, 110), // #784
  INST(Vcompresspd     , VexMr_Lx           , V(660F38,8A,_,x,_,1,3,T1S), 0                         , 0 , 0 , 3355, 250, 104), // #785
  INST(Vcompressps     , VexMr_Lx           , V(660F38,8A,_,x,_,0,2,T1S), 0                         , 0 , 0 , 3367, 250, 104), // #786
  INST(Vcvtdq2pd       , VexRm_Lx           , V(F30F00,E6,_,x,I,0,3,HV ), 0                         , 0 , 0 , 3379, 251, 99 ), // #787
  INST(Vcvtdq2ps       , VexRm_Lx           , V(000F00,5B,_,x,I,0,4,FV ), 0                         , 0 , 0 , 3389, 252, 99 ), // #788
  INST(Vcvtpd2dq       , VexRm_Lx           , V(F20F00,E6,_,x,I,1,4,FV ), 0                         , 0 , 0 , 3399, 253, 99 ), // #789
  INST(Vcvtpd2ps       , VexRm_Lx           , V(660F00,5A,_,x,I,1,4,FV ), 0                         , 0 , 0 , 3409, 253, 99 ), // #790
  INST(Vcvtpd2qq       , VexRm_Lx           , V(660F00,7B,_,x,_,1,4,FV ), 0                         , 0 , 0 , 3419, 254, 107), // #791
  INST(Vcvtpd2udq      , VexRm_Lx           , V(000F00,79,_,x,_,1,4,FV ), 0                         , 0 , 0 , 3429, 255, 104), // #792
  INST(Vcvtpd2uqq      , VexRm_Lx           , V(660F00,79,_,x,_,1,4,FV ), 0                         , 0 , 0 , 3440, 254, 107), // #793
  INST(Vcvtph2ps       , VexRm_Lx           , V(660F38,13,_,x,0,0,3,HVM), 0                         , 0 , 0 , 3451, 256, 111), // #794
  INST(Vcvtps2dq       , VexRm_Lx           , V(660F00,5B,_,x,I,0,4,FV ), 0                         , 0 , 0 , 3461, 252, 99 ), // #795
  INST(Vcvtps2pd       , VexRm_Lx           , V(000F00,5A,_,x,I,0,4,HV ), 0                         , 0 , 0 , 3471, 257, 99 ), // #796
  INST(Vcvtps2ph       , VexMri_Lx          , V(660F3A,1D,_,x,0,0,3,HVM), 0                         , 0 , 0 , 3481, 258, 111), // #797
  INST(Vcvtps2qq       , VexRm_Lx           , V(660F00,7B,_,x,_,0,3,HV ), 0                         , 0 , 0 , 3491, 259, 107), // #798
  INST(Vcvtps2udq      , VexRm_Lx           , V(000F00,79,_,x,_,0,4,FV ), 0                         , 0 , 0 , 3501, 260, 104), // #799
  INST(Vcvtps2uqq      , VexRm_Lx           , V(660F00,79,_,x,_,0,3,HV ), 0                         , 0 , 0 , 3512, 259, 107), // #800
  INST(Vcvtqq2pd       , VexRm_Lx           , V(F30F00,E6,_,x,_,1,4,FV ), 0                         , 0 , 0 , 3523, 254, 107), // #801
  INST(Vcvtqq2ps       , VexRm_Lx           , V(000F00,5B,_,x,_,1,4,FV ), 0                         , 0 , 0 , 3533, 255, 107), // #802
  INST(Vcvtsd2si       , VexRm_Wx           , V(F20F00,2D,_,I,x,x,3,T1F), 0                         , 0 , 0 , 3543, 261, 100), // #803
  INST(Vcvtsd2ss       , VexRvm             , V(F20F00,5A,_,I,I,1,3,T1S), 0                         , 0 , 0 , 3553, 220, 100), // #804
  INST(Vcvtsd2usi      , VexRm_Wx           , V(F20F00,79,_,I,_,x,3,T1F), 0                         , 0 , 0 , 3563, 262, 62 ), // #805
  INST(Vcvtsi2sd       , VexRvm_Wx          , V(F20F00,2A,_,I,x,x,2,T1W), 0                         , 0 , 0 , 3574, 263, 100), // #806
  INST(Vcvtsi2ss       , VexRvm_Wx          , V(F30F00,2A,_,I,x,x,2,T1W), 0                         , 0 , 0 , 3584, 263, 100), // #807
  INST(Vcvtss2sd       , VexRvm             , V(F30F00,5A,_,I,I,0,2,T1S), 0                         , 0 , 0 , 3594, 264, 100), // #808
  INST(Vcvtss2si       , VexRm_Wx           , V(F30F00,2D,_,I,x,x,2,T1F), 0                         , 0 , 0 , 3604, 265, 100), // #809
  INST(Vcvtss2usi      , VexRm_Wx           , V(F30F00,79,_,I,_,x,2,T1F), 0                         , 0 , 0 , 3614, 266, 62 ), // #810
  INST(Vcvttpd2dq      , VexRm_Lx           , V(660F00,E6,_,x,I,1,4,FV ), 0                         , 0 , 0 , 3625, 267, 99 ), // #811
  INST(Vcvttpd2qq      , VexRm_Lx           , V(660F00,7A,_,x,_,1,4,FV ), 0                         , 0 , 0 , 3636, 268, 104), // #812
  INST(Vcvttpd2udq     , VexRm_Lx           , V(000F00,78,_,x,_,1,4,FV ), 0                         , 0 , 0 , 3647, 269, 104), // #813
  INST(Vcvttpd2uqq     , VexRm_Lx           , V(660F00,78,_,x,_,1,4,FV ), 0                         , 0 , 0 , 3659, 268, 107), // #814
  INST(Vcvttps2dq      , VexRm_Lx           , V(F30F00,5B,_,x,I,0,4,FV ), 0                         , 0 , 0 , 3671, 270, 99 ), // #815
  INST(Vcvttps2qq      , VexRm_Lx           , V(660F00,7A,_,x,_,0,3,HV ), 0                         , 0 , 0 , 3682, 271, 107), // #816
  INST(Vcvttps2udq     , VexRm_Lx           , V(000F00,78,_,x,_,0,4,FV ), 0                         , 0 , 0 , 3693, 272, 104), // #817
  INST(Vcvttps2uqq     , VexRm_Lx           , V(660F00,78,_,x,_,0,3,HV ), 0                         , 0 , 0 , 3705, 271, 107), // #818
  INST(Vcvttsd2si      , VexRm_Wx           , V(F20F00,2C,_,I,x,x,3,T1F), 0                         , 0 , 0 , 3717, 273, 100), // #819
  INST(Vcvttsd2usi     , VexRm_Wx           , V(F20F00,78,_,I,_,x,3,T1F), 0                         , 0 , 0 , 3728, 274, 62 ), // #820
  INST(Vcvttss2si      , VexRm_Wx           , V(F30F00,2C,_,I,x,x,2,T1F), 0                         , 0 , 0 , 3740, 275, 100), // #821
  INST(Vcvttss2usi     , VexRm_Wx           , V(F30F00,78,_,I,_,x,2,T1F), 0                         , 0 , 0 , 3751, 276, 62 ), // #822
  INST(Vcvtudq2pd      , VexRm_Lx           , V(F30F00,7A,_,x,_,0,3,HV ), 0                         , 0 , 0 , 3763, 277, 104), // #823
  INST(Vcvtudq2ps      , VexRm_Lx           , V(F20F00,7A,_,x,_,0,4,FV ), 0                         , 0 , 0 , 3774, 260, 104), // #824
  INST(Vcvtuqq2pd      , VexRm_Lx           , V(F30F00,7A,_,x,_,1,4,FV ), 0                         , 0 , 0 , 3785, 254, 107), // #825
  INST(Vcvtuqq2ps      , VexRm_Lx           , V(F20F00,7A,_,x,_,1,4,FV ), 0                         , 0 , 0 , 3796, 255, 107), // #826
  INST(Vcvtusi2sd      , VexRvm_Wx          , V(F20F00,7B,_,I,_,x,2,T1W), 0                         , 0 , 0 , 3807, 278, 62 ), // #827
  INST(Vcvtusi2ss      , VexRvm_Wx          , V(F30F00,7B,_,I,_,x,2,T1W), 0                         , 0 , 0 , 3818, 278, 62 ), // #828
  INST(Vdbpsadbw       , VexRvmi_Lx         , V(660F3A,42,_,x,_,0,4,FVM), 0                         , 0 , 0 , 3829, 279, 106), // #829
  INST(Vdivpd          , VexRvm_Lx          , V(660F00,5E,_,x,I,1,4,FV ), 0                         , 0 , 0 , 3839, 218, 99 ), // #830
  INST(Vdivps          , VexRvm_Lx          , V(000F00,5E,_,x,I,0,4,FV ), 0                         , 0 , 0 , 3846, 219, 99 ), // #831
  INST(Vdivsd          , VexRvm             , V(F20F00,5E,_,I,I,1,3,T1S), 0                         , 0 , 0 , 3853, 220, 100), // #832
  INST(Vdivss          , VexRvm             , V(F30F00,5E,_,I,I,0,2,T1S), 0                         , 0 , 0 , 3860, 221, 100), // #833
  INST(Vdppd           , VexRvmi_Lx         , V(660F3A,41,_,x,I,_,_,_  ), 0                         , 0 , 0 , 3867, 280, 101), // #834
  INST(Vdpps           , VexRvmi_Lx         , V(660F3A,40,_,x,I,_,_,_  ), 0                         , 0 , 0 , 3873, 235, 101), // #835
  INST(Verr            , X86M_NoSize        , O(000F00,00,4,_,_,_,_,_  ), 0                         , 0 , 0 , 3879, 281, 10 ), // #836
  INST(Verw            , X86M_NoSize        , O(000F00,00,5,_,_,_,_,_  ), 0                         , 0 , 0 , 3884, 281, 10 ), // #837
  INST(Vexp2pd         , VexRm              , V(660F38,C8,_,2,_,1,4,FV ), 0                         , 0 , 0 , 3889, 282, 112), // #838
  INST(Vexp2ps         , VexRm              , V(660F38,C8,_,2,_,0,4,FV ), 0                         , 0 , 0 , 3897, 283, 112), // #839
  INST(Vexpandpd       , VexRm_Lx           , V(660F38,88,_,x,_,1,3,T1S), 0                         , 0 , 0 , 3905, 284, 104), // #840
  INST(Vexpandps       , VexRm_Lx           , V(660F38,88,_,x,_,0,2,T1S), 0                         , 0 , 0 , 3915, 284, 104), // #841
  INST(Vextractf128    , VexMri             , V(660F3A,19,_,1,0,_,_,_  ), 0                         , 0 , 0 , 3925, 285, 101), // #842
  INST(Vextractf32x4   , VexMri_Lx          , V(660F3A,19,_,x,_,0,4,T4 ), 0                         , 0 , 0 , 3938, 286, 104), // #843
  INST(Vextractf32x8   , VexMri             , V(660F3A,1B,_,2,_,0,5,T8 ), 0                         , 0 , 0 , 3952, 287, 60 ), // #844
  INST(Vextractf64x2   , VexMri_Lx          , V(660F3A,19,_,x,_,1,4,T2 ), 0                         , 0 , 0 , 3966, 286, 107), // #845
  INST(Vextractf64x4   , VexMri             , V(660F3A,1B,_,2,_,1,5,T4 ), 0                         , 0 , 0 , 3980, 287, 62 ), // #846
  INST(Vextracti128    , VexMri             , V(660F3A,39,_,1,0,_,_,_  ), 0                         , 0 , 0 , 3994, 285, 108), // #847
  INST(Vextracti32x4   , VexMri_Lx          , V(660F3A,39,_,x,_,0,4,T4 ), 0                         , 0 , 0 , 4007, 286, 104), // #848
  INST(Vextracti32x8   , VexMri             , V(660F3A,3B,_,2,_,0,5,T8 ), 0                         , 0 , 0 , 4021, 287, 60 ), // #849
  INST(Vextracti64x2   , VexMri_Lx          , V(660F3A,39,_,x,_,1,4,T2 ), 0                         , 0 , 0 , 4035, 286, 107), // #850
  INST(Vextracti64x4   , VexMri             , V(660F3A,3B,_,2,_,1,5,T4 ), 0                         , 0 , 0 , 4049, 287, 62 ), // #851
  INST(Vextractps      , VexMri             , V(660F3A,17,_,0,I,I,2,T1S), 0                         , 0 , 0 , 4063, 288, 100), // #852
  INST(Vfixupimmpd     , VexRvmi_Lx         , V(660F3A,54,_,x,_,1,4,FV ), 0                         , 0 , 0 , 4074, 289, 104), // #853
  INST(Vfixupimmps     , VexRvmi_Lx         , V(660F3A,54,_,x,_,0,4,FV ), 0                         , 0 , 0 , 4086, 290, 104), // #854
  INST(Vfixupimmsd     , VexRvmi            , V(660F3A,55,_,I,_,1,3,T1S), 0                         , 0 , 0 , 4098, 291, 62 ), // #855
  INST(Vfixupimmss     , VexRvmi            , V(660F3A,55,_,I,_,0,2,T1S), 0                         , 0 , 0 , 4110, 292, 62 ), // #856
  INST(Vfmadd132pd     , VexRvm_Lx          , V(660F38,98,_,x,1,1,4,FV ), 0                         , 0 , 0 , 4122, 293, 113), // #857
  INST(Vfmadd132ps     , VexRvm_Lx          , V(660F38,98,_,x,0,0,4,FV ), 0                         , 0 , 0 , 4134, 294, 113), // #858
  INST(Vfmadd132sd     , VexRvm             , V(660F38,99,_,I,1,1,3,T1S), 0                         , 0 , 0 , 4146, 295, 114), // #859
  INST(Vfmadd132ss     , VexRvm             , V(660F38,99,_,I,0,0,2,T1S), 0                         , 0 , 0 , 4158, 296, 114), // #860
  INST(Vfmadd213pd     , VexRvm_Lx          , V(660F38,A8,_,x,1,1,4,FV ), 0                         , 0 , 0 , 4170, 293, 113), // #861
  INST(Vfmadd213ps     , VexRvm_Lx          , V(660F38,A8,_,x,0,0,4,FV ), 0                         , 0 , 0 , 4182, 294, 113), // #862
  INST(Vfmadd213sd     , VexRvm             , V(660F38,A9,_,I,1,1,3,T1S), 0                         , 0 , 0 , 4194, 295, 114), // #863
  INST(Vfmadd213ss     , VexRvm             , V(660F38,A9,_,I,0,0,2,T1S), 0                         , 0 , 0 , 4206, 296, 114), // #864
  INST(Vfmadd231pd     , VexRvm_Lx          , V(660F38,B8,_,x,1,1,4,FV ), 0                         , 0 , 0 , 4218, 293, 113), // #865
  INST(Vfmadd231ps     , VexRvm_Lx          , V(660F38,B8,_,x,0,0,4,FV ), 0                         , 0 , 0 , 4230, 294, 113), // #866
  INST(Vfmadd231sd     , VexRvm             , V(660F38,B9,_,I,1,1,3,T1S), 0                         , 0 , 0 , 4242, 295, 114), // #867
  INST(Vfmadd231ss     , VexRvm             , V(660F38,B9,_,I,0,0,2,T1S), 0                         , 0 , 0 , 4254, 296, 114), // #868
  INST(Vfmaddpd        , Fma4_Lx            , V(660F3A,69,_,x,x,_,_,_  ), 0                         , 0 , 0 , 4266, 297, 115), // #869
  INST(Vfmaddps        , Fma4_Lx            , V(660F3A,68,_,x,x,_,_,_  ), 0                         , 0 , 0 , 4275, 297, 115), // #870
  INST(Vfmaddsd        , Fma4               , V(660F3A,6B,_,0,x,_,_,_  ), 0                         , 0 , 0 , 4284, 298, 115), // #871
  INST(Vfmaddss        , Fma4               , V(660F3A,6A,_,0,x,_,_,_  ), 0                         , 0 , 0 , 4293, 299, 115), // #872
  INST(Vfmaddsub132pd  , VexRvm_Lx          , V(660F38,96,_,x,1,1,4,FV ), 0                         , 0 , 0 , 4302, 293, 113), // #873
  INST(Vfmaddsub132ps  , VexRvm_Lx          , V(660F38,96,_,x,0,0,4,FV ), 0                         , 0 , 0 , 4317, 294, 113), // #874
  INST(Vfmaddsub213pd  , VexRvm_Lx          , V(660F38,A6,_,x,1,1,4,FV ), 0                         , 0 , 0 , 4332, 293, 113), // #875
  INST(Vfmaddsub213ps  , VexRvm_Lx          , V(660F38,A6,_,x,0,0,4,FV ), 0                         , 0 , 0 , 4347, 294, 113), // #876
  INST(Vfmaddsub231pd  , VexRvm_Lx          , V(660F38,B6,_,x,1,1,4,FV ), 0                         , 0 , 0 , 4362, 293, 113), // #877
  INST(Vfmaddsub231ps  , VexRvm_Lx          , V(660F38,B6,_,x,0,0,4,FV ), 0                         , 0 , 0 , 4377, 294, 113), // #878
  INST(Vfmaddsubpd     , Fma4_Lx            , V(660F3A,5D,_,x,x,_,_,_  ), 0                         , 0 , 0 , 4392, 297, 115), // #879
  INST(Vfmaddsubps     , Fma4_Lx            , V(660F3A,5C,_,x,x,_,_,_  ), 0                         , 0 , 0 , 4404, 297, 115), // #880
  INST(Vfmsub132pd     , VexRvm_Lx          , V(660F38,9A,_,x,1,1,4,FV ), 0                         , 0 , 0 , 4416, 293, 113), // #881
  INST(Vfmsub132ps     , VexRvm_Lx          , V(660F38,9A,_,x,0,0,4,FV ), 0                         , 0 , 0 , 4428, 294, 113), // #882
  INST(Vfmsub132sd     , VexRvm             , V(660F38,9B,_,I,1,1,3,T1S), 0                         , 0 , 0 , 4440, 295, 114), // #883
  INST(Vfmsub132ss     , VexRvm             , V(660F38,9B,_,I,0,0,2,T1S), 0                         , 0 , 0 , 4452, 296, 114), // #884
  INST(Vfmsub213pd     , VexRvm_Lx          , V(660F38,AA,_,x,1,1,4,FV ), 0                         , 0 , 0 , 4464, 293, 113), // #885
  INST(Vfmsub213ps     , VexRvm_Lx          , V(660F38,AA,_,x,0,0,4,FV ), 0                         , 0 , 0 , 4476, 294, 113), // #886
  INST(Vfmsub213sd     , VexRvm             , V(660F38,AB,_,I,1,1,3,T1S), 0                         , 0 , 0 , 4488, 295, 114), // #887
  INST(Vfmsub213ss     , VexRvm             , V(660F38,AB,_,I,0,0,2,T1S), 0                         , 0 , 0 , 4500, 296, 114), // #888
  INST(Vfmsub231pd     , VexRvm_Lx          , V(660F38,BA,_,x,1,1,4,FV ), 0                         , 0 , 0 , 4512, 293, 113), // #889
  INST(Vfmsub231ps     , VexRvm_Lx          , V(660F38,BA,_,x,0,0,4,FV ), 0                         , 0 , 0 , 4524, 294, 113), // #890
  INST(Vfmsub231sd     , VexRvm             , V(660F38,BB,_,I,1,1,3,T1S), 0                         , 0 , 0 , 4536, 295, 114), // #891
  INST(Vfmsub231ss     , VexRvm             , V(660F38,BB,_,I,0,0,2,T1S), 0                         , 0 , 0 , 4548, 296, 114), // #892
  INST(Vfmsubadd132pd  , VexRvm_Lx          , V(660F38,97,_,x,1,1,4,FV ), 0                         , 0 , 0 , 4560, 293, 113), // #893
  INST(Vfmsubadd132ps  , VexRvm_Lx          , V(660F38,97,_,x,0,0,4,FV ), 0                         , 0 , 0 , 4575, 294, 113), // #894
  INST(Vfmsubadd213pd  , VexRvm_Lx          , V(660F38,A7,_,x,1,1,4,FV ), 0                         , 0 , 0 , 4590, 293, 113), // #895
  INST(Vfmsubadd213ps  , VexRvm_Lx          , V(660F38,A7,_,x,0,0,4,FV ), 0                         , 0 , 0 , 4605, 294, 113), // #896
  INST(Vfmsubadd231pd  , VexRvm_Lx          , V(660F38,B7,_,x,1,1,4,FV ), 0                         , 0 , 0 , 4620, 293, 113), // #897
  INST(Vfmsubadd231ps  , VexRvm_Lx          , V(660F38,B7,_,x,0,0,4,FV ), 0                         , 0 , 0 , 4635, 294, 113), // #898
  INST(Vfmsubaddpd     , Fma4_Lx            , V(660F3A,5F,_,x,x,_,_,_  ), 0                         , 0 , 0 , 4650, 297, 115), // #899
  INST(Vfmsubaddps     , Fma4_Lx            , V(660F3A,5E,_,x,x,_,_,_  ), 0                         , 0 , 0 , 4662, 297, 115), // #900
  INST(Vfmsubpd        , Fma4_Lx            , V(660F3A,6D,_,x,x,_,_,_  ), 0                         , 0 , 0 , 4674, 297, 115), // #901
  INST(Vfmsubps        , Fma4_Lx            , V(660F3A,6C,_,x,x,_,_,_  ), 0                         , 0 , 0 , 4683, 297, 115), // #902
  INST(Vfmsubsd        , Fma4               , V(660F3A,6F,_,0,x,_,_,_  ), 0                         , 0 , 0 , 4692, 298, 115), // #903
  INST(Vfmsubss        , Fma4               , V(660F3A,6E,_,0,x,_,_,_  ), 0                         , 0 , 0 , 4701, 299, 115), // #904
  INST(Vfnmadd132pd    , VexRvm_Lx          , V(660F38,9C,_,x,1,1,4,FV ), 0                         , 0 , 0 , 4710, 293, 113), // #905
  INST(Vfnmadd132ps    , VexRvm_Lx          , V(660F38,9C,_,x,0,0,4,FV ), 0                         , 0 , 0 , 4723, 294, 113), // #906
  INST(Vfnmadd132sd    , VexRvm             , V(660F38,9D,_,I,1,1,3,T1S), 0                         , 0 , 0 , 4736, 295, 114), // #907
  INST(Vfnmadd132ss    , VexRvm             , V(660F38,9D,_,I,0,0,2,T1S), 0                         , 0 , 0 , 4749, 296, 114), // #908
  INST(Vfnmadd213pd    , VexRvm_Lx          , V(660F38,AC,_,x,1,1,4,FV ), 0                         , 0 , 0 , 4762, 293, 113), // #909
  INST(Vfnmadd213ps    , VexRvm_Lx          , V(660F38,AC,_,x,0,0,4,FV ), 0                         , 0 , 0 , 4775, 294, 113), // #910
  INST(Vfnmadd213sd    , VexRvm             , V(660F38,AD,_,I,1,1,3,T1S), 0                         , 0 , 0 , 4788, 295, 114), // #911
  INST(Vfnmadd213ss    , VexRvm             , V(660F38,AD,_,I,0,0,2,T1S), 0                         , 0 , 0 , 4801, 296, 114), // #912
  INST(Vfnmadd231pd    , VexRvm_Lx          , V(660F38,BC,_,x,1,1,4,FV ), 0                         , 0 , 0 , 4814, 293, 113), // #913
  INST(Vfnmadd231ps    , VexRvm_Lx          , V(660F38,BC,_,x,0,0,4,FV ), 0                         , 0 , 0 , 4827, 294, 113), // #914
  INST(Vfnmadd231sd    , VexRvm             , V(660F38,BC,_,I,1,1,3,T1S), 0                         , 0 , 0 , 4840, 295, 114), // #915
  INST(Vfnmadd231ss    , VexRvm             , V(660F38,BC,_,I,0,0,2,T1S), 0                         , 0 , 0 , 4853, 296, 114), // #916
  INST(Vfnmaddpd       , Fma4_Lx            , V(660F3A,79,_,x,x,_,_,_  ), 0                         , 0 , 0 , 4866, 297, 115), // #917
  INST(Vfnmaddps       , Fma4_Lx            , V(660F3A,78,_,x,x,_,_,_  ), 0                         , 0 , 0 , 4876, 297, 115), // #918
  INST(Vfnmaddsd       , Fma4               , V(660F3A,7B,_,0,x,_,_,_  ), 0                         , 0 , 0 , 4886, 298, 115), // #919
  INST(Vfnmaddss       , Fma4               , V(660F3A,7A,_,0,x,_,_,_  ), 0                         , 0 , 0 , 4896, 299, 115), // #920
  INST(Vfnmsub132pd    , VexRvm_Lx          , V(660F38,9E,_,x,1,1,4,FV ), 0                         , 0 , 0 , 4906, 293, 113), // #921
  INST(Vfnmsub132ps    , VexRvm_Lx          , V(660F38,9E,_,x,0,0,4,FV ), 0                         , 0 , 0 , 4919, 294, 113), // #922
  INST(Vfnmsub132sd    , VexRvm             , V(660F38,9F,_,I,1,1,3,T1S), 0                         , 0 , 0 , 4932, 295, 114), // #923
  INST(Vfnmsub132ss    , VexRvm             , V(660F38,9F,_,I,0,0,2,T1S), 0                         , 0 , 0 , 4945, 296, 114), // #924
  INST(Vfnmsub213pd    , VexRvm_Lx          , V(660F38,AE,_,x,1,1,4,FV ), 0                         , 0 , 0 , 4958, 293, 113), // #925
  INST(Vfnmsub213ps    , VexRvm_Lx          , V(660F38,AE,_,x,0,0,4,FV ), 0                         , 0 , 0 , 4971, 294, 113), // #926
  INST(Vfnmsub213sd    , VexRvm             , V(660F38,AF,_,I,1,1,3,T1S), 0                         , 0 , 0 , 4984, 295, 114), // #927
  INST(Vfnmsub213ss    , VexRvm             , V(660F38,AF,_,I,0,0,2,T1S), 0                         , 0 , 0 , 4997, 296, 114), // #928
  INST(Vfnmsub231pd    , VexRvm_Lx          , V(660F38,BE,_,x,1,1,4,FV ), 0                         , 0 , 0 , 5010, 293, 113), // #929
  INST(Vfnmsub231ps    , VexRvm_Lx          , V(660F38,BE,_,x,0,0,4,FV ), 0                         , 0 , 0 , 5023, 294, 113), // #930
  INST(Vfnmsub231sd    , VexRvm             , V(660F38,BF,_,I,1,1,3,T1S), 0                         , 0 , 0 , 5036, 295, 114), // #931
  INST(Vfnmsub231ss    , VexRvm             , V(660F38,BF,_,I,0,0,2,T1S), 0                         , 0 , 0 , 5049, 296, 114), // #932
  INST(Vfnmsubpd       , Fma4_Lx            , V(660F3A,7D,_,x,x,_,_,_  ), 0                         , 0 , 0 , 5062, 297, 115), // #933
  INST(Vfnmsubps       , Fma4_Lx            , V(660F3A,7C,_,x,x,_,_,_  ), 0                         , 0 , 0 , 5072, 297, 115), // #934
  INST(Vfnmsubsd       , Fma4               , V(660F3A,7F,_,0,x,_,_,_  ), 0                         , 0 , 0 , 5082, 298, 115), // #935
  INST(Vfnmsubss       , Fma4               , V(660F3A,7E,_,0,x,_,_,_  ), 0                         , 0 , 0 , 5092, 299, 115), // #936
  INST(Vfpclasspd      , VexRmi_Lx          , V(660F3A,66,_,x,_,1,4,FV ), 0                         , 0 , 0 , 5102, 300, 107), // #937
  INST(Vfpclassps      , VexRmi_Lx          , V(660F3A,66,_,x,_,0,4,FV ), 0                         , 0 , 0 , 5113, 301, 107), // #938
  INST(Vfpclasssd      , VexRmi_Lx          , V(660F3A,67,_,I,_,1,3,T1S), 0                         , 0 , 0 , 5124, 302, 60 ), // #939
  INST(Vfpclassss      , VexRmi_Lx          , V(660F3A,67,_,I,_,0,2,T1S), 0                         , 0 , 0 , 5135, 303, 60 ), // #940
  INST(Vfrczpd         , VexRm_Lx           , V(XOP_M9,81,_,x,0,_,_,_  ), 0                         , 0 , 0 , 5146, 304, 116), // #941
  INST(Vfrczps         , VexRm_Lx           , V(XOP_M9,80,_,x,0,_,_,_  ), 0                         , 0 , 0 , 5154, 304, 116), // #942
  INST(Vfrczsd         , VexRm              , V(XOP_M9,83,_,0,0,_,_,_  ), 0                         , 0 , 0 , 5162, 305, 116), // #943
  INST(Vfrczss         , VexRm              , V(XOP_M9,82,_,0,0,_,_,_  ), 0                         , 0 , 0 , 5170, 306, 116), // #944
  INST(Vgatherdpd      , VexRmvRm_VM        , V(660F38,92,_,x,1,_,_,_  ), V(660F38,92,_,x,_,1,3,T1S), 0 , 0 , 5178, 307, 117), // #945
  INST(Vgatherdps      , VexRmvRm_VM        , V(660F38,92,_,x,0,_,_,_  ), V(660F38,92,_,x,_,0,2,T1S), 0 , 0 , 5189, 308, 117), // #946
  INST(Vgatherpf0dpd   , VexM_VM            , V(660F38,C6,1,2,_,1,3,T1S), 0                         , 0 , 0 , 5200, 309, 118), // #947
  INST(Vgatherpf0dps   , VexM_VM            , V(660F38,C6,1,2,_,0,2,T1S), 0                         , 0 , 0 , 5214, 310, 118), // #948
  INST(Vgatherpf0qpd   , VexM_VM            , V(660F38,C7,1,2,_,1,3,T1S), 0                         , 0 , 0 , 5228, 311, 118), // #949
  INST(Vgatherpf0qps   , VexM_VM            , V(660F38,C7,1,2,_,0,2,T1S), 0                         , 0 , 0 , 5242, 311, 118), // #950
  INST(Vgatherpf1dpd   , VexM_VM            , V(660F38,C6,2,2,_,1,3,T1S), 0                         , 0 , 0 , 5256, 309, 118), // #951
  INST(Vgatherpf1dps   , VexM_VM            , V(660F38,C6,2,2,_,0,2,T1S), 0                         , 0 , 0 , 5270, 310, 118), // #952
  INST(Vgatherpf1qpd   , VexM_VM            , V(660F38,C7,2,2,_,1,3,T1S), 0                         , 0 , 0 , 5284, 311, 118), // #953
  INST(Vgatherpf1qps   , VexM_VM            , V(660F38,C7,2,2,_,0,2,T1S), 0                         , 0 , 0 , 5298, 311, 118), // #954
  INST(Vgatherqpd      , VexRmvRm_VM        , V(660F38,93,_,x,1,_,_,_  ), V(660F38,93,_,x,_,1,3,T1S), 0 , 0 , 5312, 312, 117), // #955
  INST(Vgatherqps      , VexRmvRm_VM        , V(660F38,93,_,x,0,_,_,_  ), V(660F38,93,_,x,_,0,2,T1S), 0 , 0 , 5323, 313, 117), // #956
  INST(Vgetexppd       , VexRm_Lx           , V(660F38,42,_,x,_,1,4,FV ), 0                         , 0 , 0 , 5334, 268, 104), // #957
  INST(Vgetexpps       , VexRm_Lx           , V(660F38,42,_,x,_,0,4,FV ), 0                         , 0 , 0 , 5344, 272, 104), // #958
  INST(Vgetexpsd       , VexRvm             , V(660F38,43,_,I,_,1,3,T1S), 0                         , 0 , 0 , 5354, 314, 62 ), // #959
  INST(Vgetexpss       , VexRvm             , V(660F38,43,_,I,_,0,2,T1S), 0                         , 0 , 0 , 5364, 315, 62 ), // #960
  INST(Vgetmantpd      , VexRmi_Lx          , V(660F3A,26,_,x,_,1,4,FV ), 0                         , 0 , 0 , 5374, 316, 104), // #961
  INST(Vgetmantps      , VexRmi_Lx          , V(660F3A,26,_,x,_,0,4,FV ), 0                         , 0 , 0 , 5385, 317, 104), // #962
  INST(Vgetmantsd      , VexRvmi            , V(660F3A,27,_,I,_,1,3,T1S), 0                         , 0 , 0 , 5396, 318, 62 ), // #963
  INST(Vgetmantss      , VexRvmi            , V(660F3A,27,_,I,_,0,2,T1S), 0                         , 0 , 0 , 5407, 319, 62 ), // #964
  INST(Vhaddpd         , VexRvm_Lx          , V(660F00,7C,_,x,I,_,_,_  ), 0                         , 0 , 0 , 5418, 222, 101), // #965
  INST(Vhaddps         , VexRvm_Lx          , V(F20F00,7C,_,x,I,_,_,_  ), 0                         , 0 , 0 , 5426, 222, 101), // #966
  INST(Vhsubpd         , VexRvm_Lx          , V(660F00,7D,_,x,I,_,_,_  ), 0                         , 0 , 0 , 5434, 222, 101), // #967
  INST(Vhsubps         , VexRvm_Lx          , V(F20F00,7D,_,x,I,_,_,_  ), 0                         , 0 , 0 , 5442, 222, 101), // #968
  INST(Vinsertf128     , VexRvmi            , V(660F3A,18,_,1,0,_,_,_  ), 0                         , 0 , 0 , 5450, 320, 101), // #969
  INST(Vinsertf32x4    , VexRvmi_Lx         , V(660F3A,18,_,x,_,0,4,T4 ), 0                         , 0 , 0 , 5462, 321, 104), // #970
  INST(Vinsertf32x8    , VexRvmi            , V(660F3A,1A,_,2,_,0,5,T8 ), 0                         , 0 , 0 , 5475, 322, 60 ), // #971
  INST(Vinsertf64x2    , VexRvmi_Lx         , V(660F3A,18,_,x,_,1,4,T2 ), 0                         , 0 , 0 , 5488, 321, 107), // #972
  INST(Vinsertf64x4    , VexRvmi            , V(660F3A,1A,_,2,_,1,5,T4 ), 0                         , 0 , 0 , 5501, 322, 62 ), // #973
  INST(Vinserti128     , VexRvmi            , V(660F3A,38,_,1,0,_,_,_  ), 0                         , 0 , 0 , 5514, 320, 108), // #974
  INST(Vinserti32x4    , VexRvmi_Lx         , V(660F3A,38,_,x,_,0,4,T4 ), 0                         , 0 , 0 , 5526, 321, 104), // #975
  INST(Vinserti32x8    , VexRvmi            , V(660F3A,3A,_,2,_,0,5,T8 ), 0                         , 0 , 0 , 5539, 322, 60 ), // #976
  INST(Vinserti64x2    , VexRvmi_Lx         , V(660F3A,38,_,x,_,1,4,T2 ), 0                         , 0 , 0 , 5552, 321, 107), // #977
  INST(Vinserti64x4    , VexRvmi            , V(660F3A,3A,_,2,_,1,5,T4 ), 0                         , 0 , 0 , 5565, 322, 62 ), // #978
  INST(Vinsertps       , VexRvmi            , V(660F3A,21,_,0,I,0,2,T1S), 0                         , 0 , 0 , 5578, 323, 100), // #979
  INST(Vlddqu          , VexRm_Lx           , V(F20F00,F0,_,x,I,_,_,_  ), 0                         , 0 , 0 , 5588, 324, 101), // #980
  INST(Vldmxcsr        , VexM               , V(000F00,AE,2,0,I,_,_,_  ), 0                         , 0 , 0 , 5595, 325, 101), // #981
  INST(Vmaskmovdqu     , VexRm_ZDI          , V(660F00,F7,_,0,I,_,_,_  ), 0                         , 0 , 0 , 5604, 326, 101), // #982
  INST(Vmaskmovpd      , VexRvmMvr_Lx       , V(660F38,2D,_,x,0,_,_,_  ), V(660F38,2F,_,x,0,_,_,_  ), 0 , 0 , 5616, 327, 101), // #983
  INST(Vmaskmovps      , VexRvmMvr_Lx       , V(660F38,2C,_,x,0,_,_,_  ), V(660F38,2E,_,x,0,_,_,_  ), 0 , 0 , 5627, 327, 101), // #984
  INST(Vmaxpd          , VexRvm_Lx          , V(660F00,5F,_,x,I,1,4,FV ), 0                         , 0 , 0 , 5638, 328, 99 ), // #985
  INST(Vmaxps          , VexRvm_Lx          , V(000F00,5F,_,x,I,0,4,FV ), 0                         , 0 , 0 , 5645, 329, 99 ), // #986
  INST(Vmaxsd          , VexRvm             , V(F20F00,5F,_,I,I,1,3,T1S), 0                         , 0 , 0 , 5652, 330, 99 ), // #987
  INST(Vmaxss          , VexRvm             , V(F30F00,5F,_,I,I,0,2,T1S), 0                         , 0 , 0 , 5659, 264, 99 ), // #988
  INST(Vminpd          , VexRvm_Lx          , V(660F00,5D,_,x,I,1,4,FV ), 0                         , 0 , 0 , 5666, 328, 99 ), // #989
  INST(Vminps          , VexRvm_Lx          , V(000F00,5D,_,x,I,0,4,FV ), 0                         , 0 , 0 , 5673, 329, 99 ), // #990
  INST(Vminsd          , VexRvm             , V(F20F00,5D,_,I,I,1,3,T1S), 0                         , 0 , 0 , 5680, 330, 99 ), // #991
  INST(Vminss          , VexRvm             , V(F30F00,5D,_,I,I,0,2,T1S), 0                         , 0 , 0 , 5687, 264, 99 ), // #992
  INST(Vmovapd         , VexRmMr_Lx         , V(660F00,28,_,x,I,1,4,FVM), V(660F00,29,_,x,I,1,4,FVM), 0 , 0 , 5694, 331, 99 ), // #993
  INST(Vmovaps         , VexRmMr_Lx         , V(000F00,28,_,x,I,0,4,FVM), V(000F00,29,_,x,I,0,4,FVM), 0 , 0 , 5702, 331, 99 ), // #994
  INST(Vmovd           , VexMovdMovq        , V(660F00,6E,_,0,0,0,2,T1S), V(660F00,7E,_,0,0,0,2,T1S), 0 , 0 , 5710, 332, 100), // #995
  INST(Vmovddup        , VexRm_Lx           , V(F20F00,12,_,x,I,1,3,DUP), 0                         , 0 , 0 , 5716, 333, 99 ), // #996
  INST(Vmovdqa         , VexRmMr_Lx         , V(660F00,6F,_,x,I,_,_,_  ), V(660F00,7F,_,x,I,_,_,_  ), 0 , 0 , 5725, 334, 101), // #997
  INST(Vmovdqa32       , VexRmMr_Lx         , V(660F00,6F,_,x,_,0,4,FVM), V(660F00,7F,_,x,_,0,4,FVM), 0 , 0 , 5733, 335, 104), // #998
  INST(Vmovdqa64       , VexRmMr_Lx         , V(660F00,6F,_,x,_,1,4,FVM), V(660F00,7F,_,x,_,1,4,FVM), 0 , 0 , 5743, 335, 104), // #999
  INST(Vmovdqu         , VexRmMr_Lx         , V(F30F00,6F,_,x,I,_,_,_  ), V(F30F00,7F,_,x,I,_,_,_  ), 0 , 0 , 5753, 334, 101), // #1000
  INST(Vmovdqu16       , VexRmMr_Lx         , V(F20F00,6F,_,x,_,1,4,FVM), V(F20F00,7F,_,x,_,1,4,FVM), 0 , 0 , 5761, 335, 106), // #1001
  INST(Vmovdqu32       , VexRmMr_Lx         , V(F30F00,6F,_,x,_,0,4,FVM), V(F30F00,7F,_,x,_,0,4,FVM), 0 , 0 , 5771, 335, 104), // #1002
  INST(Vmovdqu64       , VexRmMr_Lx         , V(F30F00,6F,_,x,_,1,4,FVM), V(F30F00,7F,_,x,_,1,4,FVM), 0 , 0 , 5781, 335, 104), // #1003
  INST(Vmovdqu8        , VexRmMr_Lx         , V(F20F00,6F,_,x,_,0,4,FVM), V(F20F00,7F,_,x,_,0,4,FVM), 0 , 0 , 5791, 335, 106), // #1004
  INST(Vmovhlps        , VexRvm             , V(000F00,12,_,0,I,0,_,_  ), 0                         , 0 , 0 , 5800, 336, 100), // #1005
  INST(Vmovhpd         , VexRvmMr           , V(660F00,16,_,0,I,1,3,T1S), V(660F00,17,_,0,I,1,3,T1S), 0 , 0 , 5809, 337, 100), // #1006
  INST(Vmovhps         , VexRvmMr           , V(000F00,16,_,0,I,0,3,T2 ), V(000F00,17,_,0,I,0,3,T2 ), 0 , 0 , 5817, 337, 100), // #1007
  INST(Vmovlhps        , VexRvm             , V(000F00,16,_,0,I,0,_,_  ), 0                         , 0 , 0 , 5825, 336, 100), // #1008
  INST(Vmovlpd         , VexRvmMr           , V(660F00,12,_,0,I,1,3,T1S), V(660F00,13,_,0,I,1,3,T1S), 0 , 0 , 5834, 337, 100), // #1009
  INST(Vmovlps         , VexRvmMr           , V(000F00,12,_,0,I,0,3,T2 ), V(000F00,13,_,0,I,0,3,T2 ), 0 , 0 , 5842, 337, 100), // #1010
  INST(Vmovmskpd       , VexRm_Lx           , V(660F00,50,_,x,I,_,_,_  ), 0                         , 0 , 0 , 5850, 338, 101), // #1011
  INST(Vmovmskps       , VexRm_Lx           , V(000F00,50,_,x,I,_,_,_  ), 0                         , 0 , 0 , 5860, 338, 101), // #1012
  INST(Vmovntdq        , VexMr_Lx           , V(660F00,E7,_,x,I,0,4,FVM), 0                         , 0 , 0 , 5870, 339, 99 ), // #1013
  INST(Vmovntdqa       , VexRm_Lx           , V(660F38,2A,_,x,I,0,4,FVM), 0                         , 0 , 0 , 5879, 340, 109), // #1014
  INST(Vmovntpd        , VexMr_Lx           , V(660F00,2B,_,x,I,1,4,FVM), 0                         , 0 , 0 , 5889, 339, 99 ), // #1015
  INST(Vmovntps        , VexMr_Lx           , V(000F00,2B,_,x,I,0,4,FVM), 0                         , 0 , 0 , 5898, 339, 99 ), // #1016
  INST(Vmovq           , VexMovdMovq        , V(660F00,6E,_,0,I,1,3,T1S), V(660F00,7E,_,0,I,1,3,T1S), 0 , 0 , 5907, 341, 100), // #1017
  INST(Vmovsd          , VexMovssMovsd      , V(F20F00,10,_,I,I,1,3,T1S), V(F20F00,11,_,I,I,1,3,T1S), 0 , 0 , 5913, 342, 100), // #1018
  INST(Vmovshdup       , VexRm_Lx           , V(F30F00,16,_,x,I,0,4,FVM), 0                         , 0 , 0 , 5920, 343, 99 ), // #1019
  INST(Vmovsldup       , VexRm_Lx           , V(F30F00,12,_,x,I,0,4,FVM), 0                         , 0 , 0 , 5930, 343, 99 ), // #1020
  INST(Vmovss          , VexMovssMovsd      , V(F30F00,10,_,I,I,0,2,T1S), V(F30F00,11,_,I,I,0,2,T1S), 0 , 0 , 5940, 344, 100), // #1021
  INST(Vmovupd         , VexRmMr_Lx         , V(660F00,10,_,x,I,1,4,FVM), V(660F00,11,_,x,I,1,4,FVM), 0 , 0 , 5947, 331, 99 ), // #1022
  INST(Vmovups         , VexRmMr_Lx         , V(000F00,10,_,x,I,0,4,FVM), V(000F00,11,_,x,I,0,4,FVM), 0 , 0 , 5955, 331, 99 ), // #1023
  INST(Vmpsadbw        , VexRvmi_Lx         , V(660F3A,42,_,x,I,_,_,_  ), 0                         , 0 , 0 , 5963, 235, 119), // #1024
  INST(Vmulpd          , VexRvm_Lx          , V(660F00,59,_,x,I,1,4,FV ), 0                         , 0 , 0 , 5972, 218, 99 ), // #1025
  INST(Vmulps          , VexRvm_Lx          , V(000F00,59,_,x,I,0,4,FV ), 0                         , 0 , 0 , 5979, 219, 99 ), // #1026
  INST(Vmulsd          , VexRvm_Lx          , V(F20F00,59,_,I,I,1,3,T1S), 0                         , 0 , 0 , 5986, 220, 100), // #1027
  INST(Vmulss          , VexRvm_Lx          , V(F30F00,59,_,I,I,0,2,T1S), 0                         , 0 , 0 , 5993, 221, 100), // #1028
  INST(Vorpd           , VexRvm_Lx          , V(660F00,56,_,x,I,1,4,FV ), 0                         , 0 , 0 , 6000, 230, 105), // #1029
  INST(Vorps           , VexRvm_Lx          , V(000F00,56,_,x,I,0,4,FV ), 0                         , 0 , 0 , 6006, 231, 105), // #1030
  INST(Vp4dpwssd       , VexRm_T1_4X        , V(F20F38,52,_,2,_,0,2,T4X), 0                         , 0 , 0 , 6012, 345, 120), // #1031
  INST(Vp4dpwssds      , VexRm_T1_4X        , V(F20F38,53,_,2,_,0,2,T4X), 0                         , 0 , 0 , 6022, 345, 120), // #1032
  INST(Vpabsb          , VexRm_Lx           , V(660F38,1C,_,x,I,_,4,FVM), 0                         , 0 , 0 , 6033, 343, 121), // #1033
  INST(Vpabsd          , VexRm_Lx           , V(660F38,1E,_,x,I,0,4,FV ), 0                         , 0 , 0 , 6040, 343, 109), // #1034
  INST(Vpabsq          , VexRm_Lx           , V(660F38,1F,_,x,_,1,4,FV ), 0                         , 0 , 0 , 6047, 284, 104), // #1035
  INST(Vpabsw          , VexRm_Lx           , V(660F38,1D,_,x,I,_,4,FVM), 0                         , 0 , 0 , 6054, 343, 121), // #1036
  INST(Vpackssdw       , VexRvm_Lx          , V(660F00,6B,_,x,I,0,4,FV ), 0                         , 0 , 0 , 6061, 229, 121), // #1037
  INST(Vpacksswb       , VexRvm_Lx          , V(660F00,63,_,x,I,I,4,FVM), 0                         , 0 , 0 , 6071, 346, 121), // #1038
  INST(Vpackusdw       , VexRvm_Lx          , V(660F38,2B,_,x,I,0,4,FV ), 0                         , 0 , 0 , 6081, 229, 121), // #1039
  INST(Vpackuswb       , VexRvm_Lx          , V(660F00,67,_,x,I,I,4,FVM), 0                         , 0 , 0 , 6091, 346, 121), // #1040
  INST(Vpaddb          , VexRvm_Lx          , V(660F00,FC,_,x,I,I,4,FVM), 0                         , 0 , 0 , 6101, 346, 121), // #1041
  INST(Vpaddd          , VexRvm_Lx          , V(660F00,FE,_,x,I,0,4,FV ), 0                         , 0 , 0 , 6108, 229, 109), // #1042
  INST(Vpaddq          , VexRvm_Lx          , V(660F00,D4,_,x,I,1,4,FV ), 0                         , 0 , 0 , 6115, 228, 109), // #1043
  INST(Vpaddsb         , VexRvm_Lx          , V(660F00,EC,_,x,I,I,4,FVM), 0                         , 0 , 0 , 6122, 346, 121), // #1044
  INST(Vpaddsw         , VexRvm_Lx          , V(660F00,ED,_,x,I,I,4,FVM), 0                         , 0 , 0 , 6130, 346, 121), // #1045
  INST(Vpaddusb        , VexRvm_Lx          , V(660F00,DC,_,x,I,I,4,FVM), 0                         , 0 , 0 , 6138, 346, 121), // #1046
  INST(Vpaddusw        , VexRvm_Lx          , V(660F00,DD,_,x,I,I,4,FVM), 0                         , 0 , 0 , 6147, 346, 121), // #1047
  INST(Vpaddw          , VexRvm_Lx          , V(660F00,FD,_,x,I,I,4,FVM), 0                         , 0 , 0 , 6156, 346, 121), // #1048
  INST(Vpalignr        , VexRvmi_Lx         , V(660F3A,0F,_,x,I,I,4,FVM), 0                         , 0 , 0 , 6163, 347, 121), // #1049
  INST(Vpand           , VexRvm_Lx          , V(660F00,DB,_,x,I,_,_,_  ), 0                         , 0 , 0 , 6172, 348, 119), // #1050
  INST(Vpandd          , VexRvm_Lx          , V(660F00,DB,_,x,_,0,4,FV ), 0                         , 0 , 0 , 6178, 349, 104), // #1051
  INST(Vpandn          , VexRvm_Lx          , V(660F00,DF,_,x,I,_,_,_  ), 0                         , 0 , 0 , 6185, 350, 119), // #1052
  INST(Vpandnd         , VexRvm_Lx          , V(660F00,DF,_,x,_,0,4,FV ), 0                         , 0 , 0 , 6192, 351, 104), // #1053
  INST(Vpandnq         , VexRvm_Lx          , V(660F00,DF,_,x,_,1,4,FV ), 0                         , 0 , 0 , 6200, 352, 104), // #1054
  INST(Vpandq          , VexRvm_Lx          , V(660F00,DB,_,x,_,1,4,FV ), 0                         , 0 , 0 , 6208, 353, 104), // #1055
  INST(Vpavgb          , VexRvm_Lx          , V(660F00,E0,_,x,I,I,4,FVM), 0                         , 0 , 0 , 6215, 346, 121), // #1056
  INST(Vpavgw          , VexRvm_Lx          , V(660F00,E3,_,x,I,I,4,FVM), 0                         , 0 , 0 , 6222, 346, 121), // #1057
  INST(Vpblendd        , VexRvmi_Lx         , V(660F3A,02,_,x,0,_,_,_  ), 0                         , 0 , 0 , 6229, 235, 108), // #1058
  INST(Vpblendvb       , VexRvmr            , V(660F3A,4C,_,x,0,_,_,_  ), 0                         , 0 , 0 , 6238, 236, 119), // #1059
  INST(Vpblendw        , VexRvmi_Lx         , V(660F3A,0E,_,x,I,_,_,_  ), 0                         , 0 , 0 , 6248, 235, 119), // #1060
  INST(Vpbroadcastb    , VexRm_Lx           , V(660F38,78,_,x,0,0,0,T1S), 0                         , 0 , 0 , 6257, 354, 122), // #1061
  INST(Vpbroadcastd    , VexRm_Lx           , V(660F38,58,_,x,0,0,2,T1S), 0                         , 0 , 0 , 6270, 355, 117), // #1062
  INST(Vpbroadcastmb2d , VexRm_Lx           , V(F30F38,3A,_,x,_,0,_,_  ), 0                         , 0 , 0 , 6283, 356, 123), // #1063
  INST(Vpbroadcastmb2q , VexRm_Lx           , V(F30F38,2A,_,x,_,1,_,_  ), 0                         , 0 , 0 , 6299, 356, 123), // #1064
  INST(Vpbroadcastq    , VexRm_Lx           , V(660F38,59,_,x,0,1,3,T1S), 0                         , 0 , 0 , 6315, 357, 117), // #1065
  INST(Vpbroadcastw    , VexRm_Lx           , V(660F38,79,_,x,0,0,1,T1S), 0                         , 0 , 0 , 6328, 358, 122), // #1066
  INST(Vpclmulqdq      , VexRvmi_Lx         , V(660F3A,44,_,x,I,_,4,FVM), 0                         , 0 , 0 , 6341, 359, 124), // #1067
  INST(Vpcmov          , VexRvrmRvmr_Lx     , V(XOP_M8,A2,_,x,x,_,_,_  ), 0                         , 0 , 0 , 6352, 297, 116), // #1068
  INST(Vpcmpb          , VexRvmi_Lx         , V(660F3A,3F,_,x,_,0,4,FVM), 0                         , 0 , 0 , 6359, 360, 106), // #1069
  INST(Vpcmpd          , VexRvmi_Lx         , V(660F3A,1F,_,x,_,0,4,FV ), 0                         , 0 , 0 , 6366, 361, 104), // #1070
  INST(Vpcmpeqb        , VexRvm_Lx          , V(660F00,74,_,x,I,I,4,FV ), 0                         , 0 , 0 , 6373, 362, 121), // #1071
  INST(Vpcmpeqd        , VexRvm_Lx          , V(660F00,76,_,x,I,0,4,FVM), 0                         , 0 , 0 , 6382, 363, 109), // #1072
  INST(Vpcmpeqq        , VexRvm_Lx          , V(660F38,29,_,x,I,1,4,FVM), 0                         , 0 , 0 , 6391, 364, 109), // #1073
  INST(Vpcmpeqw        , VexRvm_Lx          , V(660F00,75,_,x,I,I,4,FV ), 0                         , 0 , 0 , 6400, 362, 121), // #1074
  INST(Vpcmpestri      , VexRmi             , V(660F3A,61,_,0,I,_,_,_  ), 0                         , 0 , 0 , 6409, 365, 125), // #1075
  INST(Vpcmpestrm      , VexRmi             , V(660F3A,60,_,0,I,_,_,_  ), 0                         , 0 , 0 , 6420, 366, 125), // #1076
  INST(Vpcmpgtb        , VexRvm_Lx          , V(660F00,64,_,x,I,I,4,FV ), 0                         , 0 , 0 , 6431, 362, 121), // #1077
  INST(Vpcmpgtd        , VexRvm_Lx          , V(660F00,66,_,x,I,0,4,FVM), 0                         , 0 , 0 , 6440, 363, 109), // #1078
  INST(Vpcmpgtq        , VexRvm_Lx          , V(660F38,37,_,x,I,1,4,FVM), 0                         , 0 , 0 , 6449, 364, 109), // #1079
  INST(Vpcmpgtw        , VexRvm_Lx          , V(660F00,65,_,x,I,I,4,FV ), 0                         , 0 , 0 , 6458, 362, 121), // #1080
  INST(Vpcmpistri      , VexRmi             , V(660F3A,63,_,0,I,_,_,_  ), 0                         , 0 , 0 , 6467, 367, 125), // #1081
  INST(Vpcmpistrm      , VexRmi             , V(660F3A,62,_,0,I,_,_,_  ), 0                         , 0 , 0 , 6478, 368, 125), // #1082
  INST(Vpcmpq          , VexRvmi_Lx         , V(660F3A,1F,_,x,_,1,4,FV ), 0                         , 0 , 0 , 6489, 369, 104), // #1083
  INST(Vpcmpub         , VexRvmi_Lx         , V(660F3A,3E,_,x,_,0,4,FVM), 0                         , 0 , 0 , 6496, 360, 106), // #1084
  INST(Vpcmpud         , VexRvmi_Lx         , V(660F3A,1E,_,x,_,0,4,FV ), 0                         , 0 , 0 , 6504, 361, 104), // #1085
  INST(Vpcmpuq         , VexRvmi_Lx         , V(660F3A,1E,_,x,_,1,4,FV ), 0                         , 0 , 0 , 6512, 369, 104), // #1086
  INST(Vpcmpuw         , VexRvmi_Lx         , V(660F3A,3E,_,x,_,1,4,FVM), 0                         , 0 , 0 , 6520, 369, 106), // #1087
  INST(Vpcmpw          , VexRvmi_Lx         , V(660F3A,3F,_,x,_,1,4,FVM), 0                         , 0 , 0 , 6528, 369, 106), // #1088
  INST(Vpcomb          , VexRvmi            , V(XOP_M8,CC,_,0,0,_,_,_  ), 0                         , 0 , 0 , 6535, 280, 116), // #1089
  INST(Vpcomd          , VexRvmi            , V(XOP_M8,CE,_,0,0,_,_,_  ), 0                         , 0 , 0 , 6542, 280, 116), // #1090
  INST(Vpcompressb     , VexMr_Lx           , V(660F38,63,_,x,_,0,0,T1S), 0                         , 0 , 0 , 6549, 250, 126), // #1091
  INST(Vpcompressd     , VexMr_Lx           , V(660F38,8B,_,x,_,0,2,T1S), 0                         , 0 , 0 , 6561, 250, 104), // #1092
  INST(Vpcompressq     , VexMr_Lx           , V(660F38,8B,_,x,_,1,3,T1S), 0                         , 0 , 0 , 6573, 250, 104), // #1093
  INST(Vpcompressw     , VexMr_Lx           , V(660F38,63,_,x,_,1,1,T1S), 0                         , 0 , 0 , 6585, 250, 126), // #1094
  INST(Vpcomq          , VexRvmi            , V(XOP_M8,CF,_,0,0,_,_,_  ), 0                         , 0 , 0 , 6597, 280, 116), // #1095
  INST(Vpcomub         , VexRvmi            , V(XOP_M8,EC,_,0,0,_,_,_  ), 0                         , 0 , 0 , 6604, 280, 116), // #1096
  INST(Vpcomud         , VexRvmi            , V(XOP_M8,EE,_,0,0,_,_,_  ), 0                         , 0 , 0 , 6612, 280, 116), // #1097
  INST(Vpcomuq         , VexRvmi            , V(XOP_M8,EF,_,0,0,_,_,_  ), 0                         , 0 , 0 , 6620, 280, 116), // #1098
  INST(Vpcomuw         , VexRvmi            , V(XOP_M8,ED,_,0,0,_,_,_  ), 0                         , 0 , 0 , 6628, 280, 116), // #1099
  INST(Vpcomw          , VexRvmi            , V(XOP_M8,CD,_,0,0,_,_,_  ), 0                         , 0 , 0 , 6636, 280, 116), // #1100
  INST(Vpconflictd     , VexRm_Lx           , V(660F38,C4,_,x,_,0,4,FV ), 0                         , 0 , 0 , 6643, 370, 123), // #1101
  INST(Vpconflictq     , VexRm_Lx           , V(660F38,C4,_,x,_,1,4,FV ), 0                         , 0 , 0 , 6655, 370, 123), // #1102
  INST(Vperm2f128      , VexRvmi            , V(660F3A,06,_,1,0,_,_,_  ), 0                         , 0 , 0 , 6667, 371, 101), // #1103
  INST(Vperm2i128      , VexRvmi            , V(660F3A,46,_,1,0,_,_,_  ), 0                         , 0 , 0 , 6678, 371, 108), // #1104
  INST(Vpermb          , VexRvm_Lx          , V(660F38,8D,_,x,_,0,4,FVM), 0                         , 0 , 0 , 6689, 232, 127), // #1105
  INST(Vpermd          , VexRvm_Lx          , V(660F38,36,_,x,0,0,4,FV ), 0                         , 0 , 0 , 6696, 372, 117), // #1106
  INST(Vpermi2b        , VexRvm_Lx          , V(660F38,75,_,x,_,0,4,FVM), 0                         , 0 , 0 , 6703, 232, 127), // #1107
  INST(Vpermi2d        , VexRvm_Lx          , V(660F38,76,_,x,_,0,4,FV ), 0                         , 0 , 0 , 6712, 373, 104), // #1108
  INST(Vpermi2pd       , VexRvm_Lx          , V(660F38,77,_,x,_,1,4,FV ), 0                         , 0 , 0 , 6721, 234, 104), // #1109
  INST(Vpermi2ps       , VexRvm_Lx          , V(660F38,77,_,x,_,0,4,FV ), 0                         , 0 , 0 , 6731, 233, 104), // #1110
  INST(Vpermi2q        , VexRvm_Lx          , V(660F38,76,_,x,_,1,4,FV ), 0                         , 0 , 0 , 6741, 374, 104), // #1111
  INST(Vpermi2w        , VexRvm_Lx          , V(660F38,75,_,x,_,1,4,FVM), 0                         , 0 , 0 , 6750, 375, 106), // #1112
  INST(Vpermil2pd      , VexRvrmiRvmri_Lx   , V(660F3A,49,_,x,x,_,_,_  ), 0                         , 0 , 0 , 6759, 376, 116), // #1113
  INST(Vpermil2ps      , VexRvrmiRvmri_Lx   , V(660F3A,48,_,x,x,_,_,_  ), 0                         , 0 , 0 , 6770, 376, 116), // #1114
  INST(Vpermilpd       , VexRvmRmi_Lx       , V(660F38,0D,_,x,0,1,4,FV ), V(660F3A,05,_,x,0,1,4,FV ), 0 , 0 , 6781, 377, 99 ), // #1115
  INST(Vpermilps       , VexRvmRmi_Lx       , V(660F38,0C,_,x,0,0,4,FV ), V(660F3A,04,_,x,0,0,4,FV ), 0 , 0 , 6791, 377, 99 ), // #1116
  INST(Vpermpd         , VexRmi             , V(660F3A,01,_,1,1,_,_,_  ), 0                         , 0 , 0 , 6801, 378, 108), // #1117
  INST(Vpermps         , VexRvm             , V(660F38,16,_,1,0,_,_,_  ), 0                         , 0 , 0 , 6809, 379, 108), // #1118
  INST(Vpermq          , VexRvmRmi_Lx       , V(660F38,36,_,x,_,1,4,FV ), V(660F3A,00,_,x,1,1,4,FV ), 0 , 0 , 6817, 380, 117), // #1119
  INST(Vpermt2b        , VexRvm_Lx          , V(660F38,7D,_,x,_,0,4,FVM), 0                         , 0 , 0 , 6824, 232, 127), // #1120
  INST(Vpermt2d        , VexRvm_Lx          , V(660F38,7E,_,x,_,0,4,FV ), 0                         , 0 , 0 , 6833, 373, 104), // #1121
  INST(Vpermt2pd       , VexRvm_Lx          , V(660F38,7F,_,x,_,1,4,FV ), 0                         , 0 , 0 , 6842, 374, 104), // #1122
  INST(Vpermt2ps       , VexRvm_Lx          , V(660F38,7F,_,x,_,0,4,FV ), 0                         , 0 , 0 , 6852, 373, 104), // #1123
  INST(Vpermt2q        , VexRvm_Lx          , V(660F38,7E,_,x,_,1,4,FV ), 0                         , 0 , 0 , 6862, 374, 104), // #1124
  INST(Vpermt2w        , VexRvm_Lx          , V(660F38,7D,_,x,_,1,4,FVM), 0                         , 0 , 0 , 6871, 375, 106), // #1125
  INST(Vpermw          , VexRvm_Lx          , V(660F38,8D,_,x,_,1,4,FVM), 0                         , 0 , 0 , 6880, 232, 106), // #1126
  INST(Vpexpandb       , VexRm_Lx           , V(660F38,62,_,x,_,0,0,T1S), 0                         , 0 , 0 , 6887, 284, 126), // #1127
  INST(Vpexpandd       , VexRm_Lx           , V(660F38,89,_,x,_,0,2,T1S), 0                         , 0 , 0 , 6897, 284, 104), // #1128
  INST(Vpexpandq       , VexRm_Lx           , V(660F38,89,_,x,_,1,3,T1S), 0                         , 0 , 0 , 6907, 284, 104), // #1129
  INST(Vpexpandw       , VexRm_Lx           , V(660F38,62,_,x,_,1,1,T1S), 0                         , 0 , 0 , 6917, 284, 126), // #1130
  INST(Vpextrb         , VexMri             , V(660F3A,14,_,0,0,I,0,T1S), 0                         , 0 , 0 , 6927, 381, 128), // #1131
  INST(Vpextrd         , VexMri             , V(660F3A,16,_,0,0,0,2,T1S), 0                         , 0 , 0 , 6935, 288, 129), // #1132
  INST(Vpextrq         , VexMri             , V(660F3A,16,_,0,1,1,3,T1S), 0                         , 0 , 0 , 6943, 382, 129), // #1133
  INST(Vpextrw         , VexMri             , V(660F3A,15,_,0,0,I,1,T1S), 0                         , 0 , 0 , 6951, 383, 128), // #1134
  INST(Vpgatherdd      , VexRmvRm_VM        , V(660F38,90,_,x,0,_,_,_  ), V(660F38,90,_,x,_,0,2,T1S), 0 , 0 , 6959, 308, 117), // #1135
  INST(Vpgatherdq      , VexRmvRm_VM        , V(660F38,90,_,x,1,_,_,_  ), V(660F38,90,_,x,_,1,3,T1S), 0 , 0 , 6970, 307, 117), // #1136
  INST(Vpgatherqd      , VexRmvRm_VM        , V(660F38,91,_,x,0,_,_,_  ), V(660F38,91,_,x,_,0,2,T1S), 0 , 0 , 6981, 313, 117), // #1137
  INST(Vpgatherqq      , VexRmvRm_VM        , V(660F38,91,_,x,1,_,_,_  ), V(660F38,91,_,x,_,1,3,T1S), 0 , 0 , 6992, 312, 117), // #1138
  INST(Vphaddbd        , VexRm              , V(XOP_M9,C2,_,0,0,_,_,_  ), 0                         , 0 , 0 , 7003, 224, 116), // #1139
  INST(Vphaddbq        , VexRm              , V(XOP_M9,C3,_,0,0,_,_,_  ), 0                         , 0 , 0 , 7012, 224, 116), // #1140
  INST(Vphaddbw        , VexRm              , V(XOP_M9,C1,_,0,0,_,_,_  ), 0                         , 0 , 0 , 7021, 224, 116), // #1141
  INST(Vphaddd         , VexRvm_Lx          , V(660F38,02,_,x,I,_,_,_  ), 0                         , 0 , 0 , 7030, 222, 119), // #1142
  INST(Vphadddq        , VexRm              , V(XOP_M9,CB,_,0,0,_,_,_  ), 0                         , 0 , 0 , 7038, 224, 116), // #1143
  INST(Vphaddsw        , VexRvm_Lx          , V(660F38,03,_,x,I,_,_,_  ), 0                         , 0 , 0 , 7047, 222, 119), // #1144
  INST(Vphaddubd       , VexRm              , V(XOP_M9,D2,_,0,0,_,_,_  ), 0                         , 0 , 0 , 7056, 224, 116), // #1145
  INST(Vphaddubq       , VexRm              , V(XOP_M9,D3,_,0,0,_,_,_  ), 0                         , 0 , 0 , 7066, 224, 116), // #1146
  INST(Vphaddubw       , VexRm              , V(XOP_M9,D1,_,0,0,_,_,_  ), 0                         , 0 , 0 , 7076, 224, 116), // #1147
  INST(Vphaddudq       , VexRm              , V(XOP_M9,DB,_,0,0,_,_,_  ), 0                         , 0 , 0 , 7086, 224, 116), // #1148
  INST(Vphadduwd       , VexRm              , V(XOP_M9,D6,_,0,0,_,_,_  ), 0                         , 0 , 0 , 7096, 224, 116), // #1149
  INST(Vphadduwq       , VexRm              , V(XOP_M9,D7,_,0,0,_,_,_  ), 0                         , 0 , 0 , 7106, 224, 116), // #1150
  INST(Vphaddw         , VexRvm_Lx          , V(660F38,01,_,x,I,_,_,_  ), 0                         , 0 , 0 , 7116, 222, 119), // #1151
  INST(Vphaddwd        , VexRm              , V(XOP_M9,C6,_,0,0,_,_,_  ), 0                         , 0 , 0 , 7124, 224, 116), // #1152
  INST(Vphaddwq        , VexRm              , V(XOP_M9,C7,_,0,0,_,_,_  ), 0                         , 0 , 0 , 7133, 224, 116), // #1153
  INST(Vphminposuw     , VexRm              , V(660F38,41,_,0,I,_,_,_  ), 0                         , 0 , 0 , 7142, 224, 101), // #1154
  INST(Vphsubbw        , VexRm              , V(XOP_M9,E1,_,0,0,_,_,_  ), 0                         , 0 , 0 , 7154, 224, 116), // #1155
  INST(Vphsubd         , VexRvm_Lx          , V(660F38,06,_,x,I,_,_,_  ), 0                         , 0 , 0 , 7163, 222, 119), // #1156
  INST(Vphsubdq        , VexRm              , V(XOP_M9,E3,_,0,0,_,_,_  ), 0                         , 0 , 0 , 7171, 224, 116), // #1157
  INST(Vphsubsw        , VexRvm_Lx          , V(660F38,07,_,x,I,_,_,_  ), 0                         , 0 , 0 , 7180, 222, 119), // #1158
  INST(Vphsubw         , VexRvm_Lx          , V(660F38,05,_,x,I,_,_,_  ), 0                         , 0 , 0 , 7189, 222, 119), // #1159
  INST(Vphsubwd        , VexRm              , V(XOP_M9,E2,_,0,0,_,_,_  ), 0                         , 0 , 0 , 7197, 224, 116), // #1160
  INST(Vpinsrb         , VexRvmi            , V(660F3A,20,_,0,0,I,0,T1S), 0                         , 0 , 0 , 7206, 384, 128), // #1161
  INST(Vpinsrd         , VexRvmi            , V(660F3A,22,_,0,0,0,2,T1S), 0                         , 0 , 0 , 7214, 385, 129), // #1162
  INST(Vpinsrq         , VexRvmi            , V(660F3A,22,_,0,1,1,3,T1S), 0                         , 0 , 0 , 7222, 386, 129), // #1163
  INST(Vpinsrw         , VexRvmi            , V(660F00,C4,_,0,0,I,1,T1S), 0                         , 0 , 0 , 7230, 387, 128), // #1164
  INST(Vplzcntd        , VexRm_Lx           , V(660F38,44,_,x,_,0,4,FV ), 0                         , 0 , 0 , 7238, 370, 123), // #1165
  INST(Vplzcntq        , VexRm_Lx           , V(660F38,44,_,x,_,1,4,FV ), 0                         , 0 , 0 , 7247, 388, 123), // #1166
  INST(Vpmacsdd        , VexRvmr            , V(XOP_M8,9E,_,0,0,_,_,_  ), 0                         , 0 , 0 , 7256, 389, 116), // #1167
  INST(Vpmacsdqh       , VexRvmr            , V(XOP_M8,9F,_,0,0,_,_,_  ), 0                         , 0 , 0 , 7265, 389, 116), // #1168
  INST(Vpmacsdql       , VexRvmr            , V(XOP_M8,97,_,0,0,_,_,_  ), 0                         , 0 , 0 , 7275, 389, 116), // #1169
  INST(Vpmacssdd       , VexRvmr            , V(XOP_M8,8E,_,0,0,_,_,_  ), 0                         , 0 , 0 , 7285, 389, 116), // #1170
  INST(Vpmacssdqh      , VexRvmr            , V(XOP_M8,8F,_,0,0,_,_,_  ), 0                         , 0 , 0 , 7295, 389, 116), // #1171
  INST(Vpmacssdql      , VexRvmr            , V(XOP_M8,87,_,0,0,_,_,_  ), 0                         , 0 , 0 , 7306, 389, 116), // #1172
  INST(Vpmacsswd       , VexRvmr            , V(XOP_M8,86,_,0,0,_,_,_  ), 0                         , 0 , 0 , 7317, 389, 116), // #1173
  INST(Vpmacssww       , VexRvmr            , V(XOP_M8,85,_,0,0,_,_,_  ), 0                         , 0 , 0 , 7327, 389, 116), // #1174
  INST(Vpmacswd        , VexRvmr            , V(XOP_M8,96,_,0,0,_,_,_  ), 0                         , 0 , 0 , 7337, 389, 116), // #1175
  INST(Vpmacsww        , VexRvmr            , V(XOP_M8,95,_,0,0,_,_,_  ), 0                         , 0 , 0 , 7346, 389, 116), // #1176
  INST(Vpmadcsswd      , VexRvmr            , V(XOP_M8,A6,_,0,0,_,_,_  ), 0                         , 0 , 0 , 7355, 389, 116), // #1177
  INST(Vpmadcswd       , VexRvmr            , V(XOP_M8,B6,_,0,0,_,_,_  ), 0                         , 0 , 0 , 7366, 389, 116), // #1178
  INST(Vpmadd52huq     , VexRvm_Lx          , V(660F38,B5,_,x,_,1,4,FV ), 0                         , 0 , 0 , 7376, 374, 130), // #1179
  INST(Vpmadd52luq     , VexRvm_Lx          , V(660F38,B4,_,x,_,1,4,FV ), 0                         , 0 , 0 , 7388, 374, 130), // #1180
  INST(Vpmaddubsw      , VexRvm_Lx          , V(660F38,04,_,x,I,I,4,FVM), 0                         , 0 , 0 , 7400, 346, 121), // #1181
  INST(Vpmaddwd        , VexRvm_Lx          , V(660F00,F5,_,x,I,I,4,FVM), 0                         , 0 , 0 , 7411, 346, 121), // #1182
  INST(Vpmaskmovd      , VexRvmMvr_Lx       , V(660F38,8C,_,x,0,_,_,_  ), V(660F38,8E,_,x,0,_,_,_  ), 0 , 0 , 7420, 327, 108), // #1183
  INST(Vpmaskmovq      , VexRvmMvr_Lx       , V(660F38,8C,_,x,1,_,_,_  ), V(660F38,8E,_,x,1,_,_,_  ), 0 , 0 , 7431, 327, 108), // #1184
  INST(Vpmaxsb         , VexRvm_Lx          , V(660F38,3C,_,x,I,I,4,FVM), 0                         , 0 , 0 , 7442, 390, 121), // #1185
  INST(Vpmaxsd         , VexRvm_Lx          , V(660F38,3D,_,x,I,0,4,FV ), 0                         , 0 , 0 , 7450, 231, 109), // #1186
  INST(Vpmaxsq         , VexRvm_Lx          , V(660F38,3D,_,x,_,1,4,FV ), 0                         , 0 , 0 , 7458, 234, 104), // #1187
  INST(Vpmaxsw         , VexRvm_Lx          , V(660F00,EE,_,x,I,I,4,FVM), 0                         , 0 , 0 , 7466, 390, 121), // #1188
  INST(Vpmaxub         , VexRvm_Lx          , V(660F00,DE,_,x,I,I,4,FVM), 0                         , 0 , 0 , 7474, 390, 121), // #1189
  INST(Vpmaxud         , VexRvm_Lx          , V(660F38,3F,_,x,I,0,4,FV ), 0                         , 0 , 0 , 7482, 231, 109), // #1190
  INST(Vpmaxuq         , VexRvm_Lx          , V(660F38,3F,_,x,_,1,4,FV ), 0                         , 0 , 0 , 7490, 234, 104), // #1191
  INST(Vpmaxuw         , VexRvm_Lx          , V(660F38,3E,_,x,I,I,4,FVM), 0                         , 0 , 0 , 7498, 390, 121), // #1192
  INST(Vpminsb         , VexRvm_Lx          , V(660F38,38,_,x,I,I,4,FVM), 0                         , 0 , 0 , 7506, 390, 121), // #1193
  INST(Vpminsd         , VexRvm_Lx          , V(660F38,39,_,x,I,0,4,FV ), 0                         , 0 , 0 , 7514, 231, 109), // #1194
  INST(Vpminsq         , VexRvm_Lx          , V(660F38,39,_,x,_,1,4,FV ), 0                         , 0 , 0 , 7522, 234, 104), // #1195
  INST(Vpminsw         , VexRvm_Lx          , V(660F00,EA,_,x,I,I,4,FVM), 0                         , 0 , 0 , 7530, 390, 121), // #1196
  INST(Vpminub         , VexRvm_Lx          , V(660F00,DA,_,x,I,_,4,FVM), 0                         , 0 , 0 , 7538, 390, 121), // #1197
  INST(Vpminud         , VexRvm_Lx          , V(660F38,3B,_,x,I,0,4,FV ), 0                         , 0 , 0 , 7546, 231, 109), // #1198
  INST(Vpminuq         , VexRvm_Lx          , V(660F38,3B,_,x,_,1,4,FV ), 0                         , 0 , 0 , 7554, 234, 104), // #1199
  INST(Vpminuw         , VexRvm_Lx          , V(660F38,3A,_,x,I,_,4,FVM), 0                         , 0 , 0 , 7562, 390, 121), // #1200
  INST(Vpmovb2m        , VexRm_Lx           , V(F30F38,29,_,x,_,0,_,_  ), 0                         , 0 , 0 , 7570, 391, 106), // #1201
  INST(Vpmovd2m        , VexRm_Lx           , V(F30F38,39,_,x,_,0,_,_  ), 0                         , 0 , 0 , 7579, 391, 107), // #1202
  INST(Vpmovdb         , VexMr_Lx           , V(F30F38,31,_,x,_,0,2,QVM), 0                         , 0 , 0 , 7588, 392, 104), // #1203
  INST(Vpmovdw         , VexMr_Lx           , V(F30F38,33,_,x,_,0,3,HVM), 0                         , 0 , 0 , 7596, 393, 104), // #1204
  INST(Vpmovm2b        , VexRm_Lx           , V(F30F38,28,_,x,_,0,_,_  ), 0                         , 0 , 0 , 7604, 356, 106), // #1205
  INST(Vpmovm2d        , VexRm_Lx           , V(F30F38,38,_,x,_,0,_,_  ), 0                         , 0 , 0 , 7613, 356, 107), // #1206
  INST(Vpmovm2q        , VexRm_Lx           , V(F30F38,38,_,x,_,1,_,_  ), 0                         , 0 , 0 , 7622, 356, 107), // #1207
  INST(Vpmovm2w        , VexRm_Lx           , V(F30F38,28,_,x,_,1,_,_  ), 0                         , 0 , 0 , 7631, 356, 106), // #1208
  INST(Vpmovmskb       , VexRm_Lx           , V(660F00,D7,_,x,I,_,_,_  ), 0                         , 0 , 0 , 7640, 338, 119), // #1209
  INST(Vpmovq2m        , VexRm_Lx           , V(F30F38,39,_,x,_,1,_,_  ), 0                         , 0 , 0 , 7650, 391, 107), // #1210
  INST(Vpmovqb         , VexMr_Lx           , V(F30F38,32,_,x,_,0,1,OVM), 0                         , 0 , 0 , 7659, 394, 104), // #1211
  INST(Vpmovqd         , VexMr_Lx           , V(F30F38,35,_,x,_,0,3,HVM), 0                         , 0 , 0 , 7667, 393, 104), // #1212
  INST(Vpmovqw         , VexMr_Lx           , V(F30F38,34,_,x,_,0,2,QVM), 0                         , 0 , 0 , 7675, 392, 104), // #1213
  INST(Vpmovsdb        , VexMr_Lx           , V(F30F38,21,_,x,_,0,2,QVM), 0                         , 0 , 0 , 7683, 392, 104), // #1214
  INST(Vpmovsdw        , VexMr_Lx           , V(F30F38,23,_,x,_,0,3,HVM), 0                         , 0 , 0 , 7692, 393, 104), // #1215
  INST(Vpmovsqb        , VexMr_Lx           , V(F30F38,22,_,x,_,0,1,OVM), 0                         , 0 , 0 , 7701, 394, 104), // #1216
  INST(Vpmovsqd        , VexMr_Lx           , V(F30F38,25,_,x,_,0,3,HVM), 0                         , 0 , 0 , 7710, 393, 104), // #1217
  INST(Vpmovsqw        , VexMr_Lx           , V(F30F38,24,_,x,_,0,2,QVM), 0                         , 0 , 0 , 7719, 392, 104), // #1218
  INST(Vpmovswb        , VexMr_Lx           , V(F30F38,20,_,x,_,0,3,HVM), 0                         , 0 , 0 , 7728, 393, 106), // #1219
  INST(Vpmovsxbd       , VexRm_Lx           , V(660F38,21,_,x,I,I,2,QVM), 0                         , 0 , 0 , 7737, 395, 109), // #1220
  INST(Vpmovsxbq       , VexRm_Lx           , V(660F38,22,_,x,I,I,1,OVM), 0                         , 0 , 0 , 7747, 396, 109), // #1221
  INST(Vpmovsxbw       , VexRm_Lx           , V(660F38,20,_,x,I,I,3,HVM), 0                         , 0 , 0 , 7757, 397, 121), // #1222
  INST(Vpmovsxdq       , VexRm_Lx           , V(660F38,25,_,x,I,0,3,HVM), 0                         , 0 , 0 , 7767, 397, 109), // #1223
  INST(Vpmovsxwd       , VexRm_Lx           , V(660F38,23,_,x,I,I,3,HVM), 0                         , 0 , 0 , 7777, 397, 109), // #1224
  INST(Vpmovsxwq       , VexRm_Lx           , V(660F38,24,_,x,I,I,2,QVM), 0                         , 0 , 0 , 7787, 395, 109), // #1225
  INST(Vpmovusdb       , VexMr_Lx           , V(F30F38,11,_,x,_,0,2,QVM), 0                         , 0 , 0 , 7797, 392, 104), // #1226
  INST(Vpmovusdw       , VexMr_Lx           , V(F30F38,13,_,x,_,0,3,HVM), 0                         , 0 , 0 , 7807, 393, 104), // #1227
  INST(Vpmovusqb       , VexMr_Lx           , V(F30F38,12,_,x,_,0,1,OVM), 0                         , 0 , 0 , 7817, 394, 104), // #1228
  INST(Vpmovusqd       , VexMr_Lx           , V(F30F38,15,_,x,_,0,3,HVM), 0                         , 0 , 0 , 7827, 393, 104), // #1229
  INST(Vpmovusqw       , VexMr_Lx           , V(F30F38,14,_,x,_,0,2,QVM), 0                         , 0 , 0 , 7837, 392, 104), // #1230
  INST(Vpmovuswb       , VexMr_Lx           , V(F30F38,10,_,x,_,0,3,HVM), 0                         , 0 , 0 , 7847, 393, 106), // #1231
  INST(Vpmovw2m        , VexRm_Lx           , V(F30F38,29,_,x,_,1,_,_  ), 0                         , 0 , 0 , 7857, 391, 106), // #1232
  INST(Vpmovwb         , VexMr_Lx           , V(F30F38,30,_,x,_,0,3,HVM), 0                         , 0 , 0 , 7866, 393, 106), // #1233
  INST(Vpmovzxbd       , VexRm_Lx           , V(660F38,31,_,x,I,I,2,QVM), 0                         , 0 , 0 , 7874, 395, 109), // #1234
  INST(Vpmovzxbq       , VexRm_Lx           , V(660F38,32,_,x,I,I,1,OVM), 0                         , 0 , 0 , 7884, 396, 109), // #1235
  INST(Vpmovzxbw       , VexRm_Lx           , V(660F38,30,_,x,I,I,3,HVM), 0                         , 0 , 0 , 7894, 397, 121), // #1236
  INST(Vpmovzxdq       , VexRm_Lx           , V(660F38,35,_,x,I,0,3,HVM), 0                         , 0 , 0 , 7904, 397, 109), // #1237
  INST(Vpmovzxwd       , VexRm_Lx           , V(660F38,33,_,x,I,I,3,HVM), 0                         , 0 , 0 , 7914, 397, 109), // #1238
  INST(Vpmovzxwq       , VexRm_Lx           , V(660F38,34,_,x,I,I,2,QVM), 0                         , 0 , 0 , 7924, 395, 109), // #1239
  INST(Vpmuldq         , VexRvm_Lx          , V(660F38,28,_,x,I,1,4,FV ), 0                         , 0 , 0 , 7934, 228, 109), // #1240
  INST(Vpmulhrsw       , VexRvm_Lx          , V(660F38,0B,_,x,I,I,4,FVM), 0                         , 0 , 0 , 7942, 346, 121), // #1241
  INST(Vpmulhuw        , VexRvm_Lx          , V(660F00,E4,_,x,I,I,4,FVM), 0                         , 0 , 0 , 7952, 346, 121), // #1242
  INST(Vpmulhw         , VexRvm_Lx          , V(660F00,E5,_,x,I,I,4,FVM), 0                         , 0 , 0 , 7961, 346, 121), // #1243
  INST(Vpmulld         , VexRvm_Lx          , V(660F38,40,_,x,I,0,4,FV ), 0                         , 0 , 0 , 7969, 229, 109), // #1244
  INST(Vpmullq         , VexRvm_Lx          , V(660F38,40,_,x,_,1,4,FV ), 0                         , 0 , 0 , 7977, 234, 107), // #1245
  INST(Vpmullw         , VexRvm_Lx          , V(660F00,D5,_,x,I,I,4,FVM), 0                         , 0 , 0 , 7985, 346, 121), // #1246
  INST(Vpmultishiftqb  , VexRvm_Lx          , V(660F38,83,_,x,_,1,4,FV ), 0                         , 0 , 0 , 7993, 234, 127), // #1247
  INST(Vpmuludq        , VexRvm_Lx          , V(660F00,F4,_,x,I,1,4,FV ), 0                         , 0 , 0 , 8008, 228, 109), // #1248
  INST(Vpopcntb        , VexRm_Lx           , V(660F38,54,_,x,_,0,4,FV ), 0                         , 0 , 0 , 8017, 284, 131), // #1249
  INST(Vpopcntd        , VexRm_Lx           , V(660F38,55,_,x,_,0,4,FVM), 0                         , 0 , 0 , 8026, 370, 132), // #1250
  INST(Vpopcntq        , VexRm_Lx           , V(660F38,55,_,x,_,1,4,FVM), 0                         , 0 , 0 , 8035, 388, 132), // #1251
  INST(Vpopcntw        , VexRm_Lx           , V(660F38,54,_,x,_,1,4,FV ), 0                         , 0 , 0 , 8044, 284, 131), // #1252
  INST(Vpor            , VexRvm_Lx          , V(660F00,EB,_,x,I,_,_,_  ), 0                         , 0 , 0 , 8053, 348, 119), // #1253
  INST(Vpord           , VexRvm_Lx          , V(660F00,EB,_,x,_,0,4,FV ), 0                         , 0 , 0 , 8058, 349, 104), // #1254
  INST(Vporq           , VexRvm_Lx          , V(660F00,EB,_,x,_,1,4,FV ), 0                         , 0 , 0 , 8064, 353, 104), // #1255
  INST(Vpperm          , VexRvrmRvmr        , V(XOP_M8,A3,_,0,x,_,_,_  ), 0                         , 0 , 0 , 8070, 398, 116), // #1256
  INST(Vprold          , VexVmi_Lx          , V(660F00,72,1,x,_,0,4,FV ), 0                         , 0 , 0 , 8077, 399, 104), // #1257
  INST(Vprolq          , VexVmi_Lx          , V(660F00,72,1,x,_,1,4,FV ), 0                         , 0 , 0 , 8084, 400, 104), // #1258
  INST(Vprolvd         , VexRvm_Lx          , V(660F38,15,_,x,_,0,4,FV ), 0                         , 0 , 0 , 8091, 233, 104), // #1259
  INST(Vprolvq         , VexRvm_Lx          , V(660F38,15,_,x,_,1,4,FV ), 0                         , 0 , 0 , 8099, 234, 104), // #1260
  INST(Vprord          , VexVmi_Lx          , V(660F00,72,0,x,_,0,4,FV ), 0                         , 0 , 0 , 8107, 399, 104), // #1261
  INST(Vprorq          , VexVmi_Lx          , V(660F00,72,0,x,_,1,4,FV ), 0                         , 0 , 0 , 8114, 400, 104), // #1262
  INST(Vprorvd         , VexRvm_Lx          , V(660F38,14,_,x,_,0,4,FV ), 0                         , 0 , 0 , 8121, 233, 104), // #1263
  INST(Vprorvq         , VexRvm_Lx          , V(660F38,14,_,x,_,1,4,FV ), 0                         , 0 , 0 , 8129, 234, 104), // #1264
  INST(Vprotb          , VexRvmRmvRmi       , V(XOP_M9,90,_,0,x,_,_,_  ), V(XOP_M8,C0,_,0,x,_,_,_  ), 0 , 0 , 8137, 401, 116), // #1265
  INST(Vprotd          , VexRvmRmvRmi       , V(XOP_M9,92,_,0,x,_,_,_  ), V(XOP_M8,C2,_,0,x,_,_,_  ), 0 , 0 , 8144, 401, 116), // #1266
  INST(Vprotq          , VexRvmRmvRmi       , V(XOP_M9,93,_,0,x,_,_,_  ), V(XOP_M8,C3,_,0,x,_,_,_  ), 0 , 0 , 8151, 401, 116), // #1267
  INST(Vprotw          , VexRvmRmvRmi       , V(XOP_M9,91,_,0,x,_,_,_  ), V(XOP_M8,C1,_,0,x,_,_,_  ), 0 , 0 , 8158, 401, 116), // #1268
  INST(Vpsadbw         , VexRvm_Lx          , V(660F00,F6,_,x,I,I,4,FVM), 0                         , 0 , 0 , 8165, 223, 121), // #1269
  INST(Vpscatterdd     , VexMr_VM           , V(660F38,A0,_,x,_,0,2,T1S), 0                         , 0 , 0 , 8173, 402, 104), // #1270
  INST(Vpscatterdq     , VexMr_VM           , V(660F38,A0,_,x,_,1,3,T1S), 0                         , 0 , 0 , 8185, 402, 104), // #1271
  INST(Vpscatterqd     , VexMr_VM           , V(660F38,A1,_,x,_,0,2,T1S), 0                         , 0 , 0 , 8197, 403, 104), // #1272
  INST(Vpscatterqq     , VexMr_VM           , V(660F38,A1,_,x,_,1,3,T1S), 0                         , 0 , 0 , 8209, 404, 104), // #1273
  INST(Vpshab          , VexRvmRmv          , V(XOP_M9,98,_,0,x,_,_,_  ), 0                         , 0 , 0 , 8221, 405, 116), // #1274
  INST(Vpshad          , VexRvmRmv          , V(XOP_M9,9A,_,0,x,_,_,_  ), 0                         , 0 , 0 , 8228, 405, 116), // #1275
  INST(Vpshaq          , VexRvmRmv          , V(XOP_M9,9B,_,0,x,_,_,_  ), 0                         , 0 , 0 , 8235, 405, 116), // #1276
  INST(Vpshaw          , VexRvmRmv          , V(XOP_M9,99,_,0,x,_,_,_  ), 0                         , 0 , 0 , 8242, 405, 116), // #1277
  INST(Vpshlb          , VexRvmRmv          , V(XOP_M9,94,_,0,x,_,_,_  ), 0                         , 0 , 0 , 8249, 405, 116), // #1278
  INST(Vpshld          , VexRvmRmv          , V(XOP_M9,96,_,0,x,_,_,_  ), 0                         , 0 , 0 , 8256, 405, 116), // #1279
  INST(Vpshldd         , VexRvmi_Lx         , V(660F3A,71,_,x,_,0,4,FV ), 0                         , 0 , 0 , 8263, 226, 126), // #1280
  INST(Vpshldq         , VexRvmi_Lx         , V(660F3A,71,_,x,_,1,4,FV ), 0                         , 0 , 0 , 8271, 227, 126), // #1281
  INST(Vpshldvd        , VexRvm_Lx          , V(660F38,71,_,x,_,0,4,FV ), 0                         , 0 , 0 , 8279, 373, 126), // #1282
  INST(Vpshldvq        , VexRvm_Lx          , V(660F38,71,_,x,_,1,4,FV ), 0                         , 0 , 0 , 8288, 374, 126), // #1283
  INST(Vpshldvw        , VexRvm_Lx          , V(660F38,70,_,x,_,0,4,FVM), 0                         , 0 , 0 , 8297, 375, 126), // #1284
  INST(Vpshldw         , VexRvmi_Lx         , V(660F3A,70,_,x,_,0,4,FVM), 0                         , 0 , 0 , 8306, 279, 126), // #1285
  INST(Vpshlq          , VexRvmRmv          , V(XOP_M9,97,_,0,x,_,_,_  ), 0                         , 0 , 0 , 8314, 405, 116), // #1286
  INST(Vpshlw          , VexRvmRmv          , V(XOP_M9,95,_,0,x,_,_,_  ), 0                         , 0 , 0 , 8321, 405, 116), // #1287
  INST(Vpshrdd         , VexRvmi_Lx         , V(660F3A,73,_,x,_,0,4,FV ), 0                         , 0 , 0 , 8328, 226, 126), // #1288
  INST(Vpshrdq         , VexRvmi_Lx         , V(660F3A,73,_,x,_,1,4,FV ), 0                         , 0 , 0 , 8336, 227, 126), // #1289
  INST(Vpshrdvd        , VexRvm_Lx          , V(660F38,73,_,x,_,0,4,FV ), 0                         , 0 , 0 , 8344, 373, 126), // #1290
  INST(Vpshrdvq        , VexRvm_Lx          , V(660F38,73,_,x,_,1,4,FV ), 0                         , 0 , 0 , 8353, 374, 126), // #1291
  INST(Vpshrdvw        , VexRvm_Lx          , V(660F38,72,_,x,_,0,4,FVM), 0                         , 0 , 0 , 8362, 375, 126), // #1292
  INST(Vpshrdw         , VexRvmi_Lx         , V(660F3A,72,_,x,_,0,4,FVM), 0                         , 0 , 0 , 8371, 279, 126), // #1293
  INST(Vpshufb         , VexRvm_Lx          , V(660F38,00,_,x,I,I,4,FVM), 0                         , 0 , 0 , 8379, 346, 121), // #1294
  INST(Vpshufbitqmb    , VexRvm_Lx          , V(660F38,8F,_,x,0,0,4,FVM), 0                         , 0 , 0 , 8387, 406, 131), // #1295
  INST(Vpshufd         , VexRmi_Lx          , V(660F00,70,_,x,I,0,4,FV ), 0                         , 0 , 0 , 8400, 407, 109), // #1296
  INST(Vpshufhw        , VexRmi_Lx          , V(F30F00,70,_,x,I,I,4,FVM), 0                         , 0 , 0 , 8408, 408, 121), // #1297
  INST(Vpshuflw        , VexRmi_Lx          , V(F20F00,70,_,x,I,I,4,FVM), 0                         , 0 , 0 , 8417, 408, 121), // #1298
  INST(Vpsignb         , VexRvm_Lx          , V(660F38,08,_,x,I,_,_,_  ), 0                         , 0 , 0 , 8426, 222, 119), // #1299
  INST(Vpsignd         , VexRvm_Lx          , V(660F38,0A,_,x,I,_,_,_  ), 0                         , 0 , 0 , 8434, 222, 119), // #1300
  INST(Vpsignw         , VexRvm_Lx          , V(660F38,09,_,x,I,_,_,_  ), 0                         , 0 , 0 , 8442, 222, 119), // #1301
  INST(Vpslld          , VexRvmVmi_Lx       , V(660F00,F2,_,x,I,0,4,128), V(660F00,72,6,x,I,0,4,FV ), 0 , 0 , 8450, 409, 109), // #1302
  INST(Vpslldq         , VexEvexVmi_Lx      , V(660F00,73,7,x,I,I,4,FVM), 0                         , 0 , 0 , 8457, 410, 121), // #1303
  INST(Vpsllq          , VexRvmVmi_Lx       , V(660F00,F3,_,x,I,1,4,128), V(660F00,73,6,x,I,1,4,FV ), 0 , 0 , 8465, 411, 109), // #1304
  INST(Vpsllvd         , VexRvm_Lx          , V(660F38,47,_,x,0,0,4,FV ), 0                         , 0 , 0 , 8472, 229, 117), // #1305
  INST(Vpsllvq         , VexRvm_Lx          , V(660F38,47,_,x,1,1,4,FV ), 0                         , 0 , 0 , 8480, 228, 117), // #1306
  INST(Vpsllvw         , VexRvm_Lx          , V(660F38,12,_,x,_,1,4,FVM), 0                         , 0 , 0 , 8488, 232, 106), // #1307
  INST(Vpsllw          , VexRvmVmi_Lx       , V(660F00,F1,_,x,I,I,4,FVM), V(660F00,71,6,x,I,I,4,FVM), 0 , 0 , 8496, 412, 121), // #1308
  INST(Vpsrad          , VexRvmVmi_Lx       , V(660F00,E2,_,x,I,0,4,128), V(660F00,72,4,x,I,0,4,FV ), 0 , 0 , 8503, 409, 109), // #1309
  INST(Vpsraq          , VexRvmVmi_Lx       , V(660F00,E2,_,x,_,1,4,128), V(660F00,72,4,x,_,1,4,FV ), 0 , 0 , 8510, 413, 104), // #1310
  INST(Vpsravd         , VexRvm_Lx          , V(660F38,46,_,x,0,0,4,FV ), 0                         , 0 , 0 , 8517, 229, 117), // #1311
  INST(Vpsravq         , VexRvm_Lx          , V(660F38,46,_,x,_,1,4,FV ), 0                         , 0 , 0 , 8525, 234, 104), // #1312
  INST(Vpsravw         , VexRvm_Lx          , V(660F38,11,_,x,_,1,4,FVM), 0                         , 0 , 0 , 8533, 232, 106), // #1313
  INST(Vpsraw          , VexRvmVmi_Lx       , V(660F00,E1,_,x,I,I,4,128), V(660F00,71,4,x,I,I,4,FVM), 0 , 0 , 8541, 412, 121), // #1314
  INST(Vpsrld          , VexRvmVmi_Lx       , V(660F00,D2,_,x,I,0,4,128), V(660F00,72,2,x,I,0,4,FV ), 0 , 0 , 8548, 409, 109), // #1315
  INST(Vpsrldq         , VexEvexVmi_Lx      , V(660F00,73,3,x,I,I,4,FVM), 0                         , 0 , 0 , 8555, 410, 121), // #1316
  INST(Vpsrlq          , VexRvmVmi_Lx       , V(660F00,D3,_,x,I,1,4,128), V(660F00,73,2,x,I,1,4,FV ), 0 , 0 , 8563, 411, 109), // #1317
  INST(Vpsrlvd         , VexRvm_Lx          , V(660F38,45,_,x,0,0,4,FV ), 0                         , 0 , 0 , 8570, 229, 117), // #1318
  INST(Vpsrlvq         , VexRvm_Lx          , V(660F38,45,_,x,1,1,4,FV ), 0                         , 0 , 0 , 8578, 228, 117), // #1319
  INST(Vpsrlvw         , VexRvm_Lx          , V(660F38,10,_,x,_,1,4,FVM), 0                         , 0 , 0 , 8586, 232, 106), // #1320
  INST(Vpsrlw          , VexRvmVmi_Lx       , V(660F00,D1,_,x,I,I,4,128), V(660F00,71,2,x,I,I,4,FVM), 0 , 0 , 8594, 412, 121), // #1321
  INST(Vpsubb          , VexRvm_Lx          , V(660F00,F8,_,x,I,I,4,FVM), 0                         , 0 , 0 , 8601, 414, 121), // #1322
  INST(Vpsubd          , VexRvm_Lx          , V(660F00,FA,_,x,I,0,4,FV ), 0                         , 0 , 0 , 8608, 415, 109), // #1323
  INST(Vpsubq          , VexRvm_Lx          , V(660F00,FB,_,x,I,1,4,FV ), 0                         , 0 , 0 , 8615, 416, 109), // #1324
  INST(Vpsubsb         , VexRvm_Lx          , V(660F00,E8,_,x,I,I,4,FVM), 0                         , 0 , 0 , 8622, 414, 121), // #1325
  INST(Vpsubsw         , VexRvm_Lx          , V(660F00,E9,_,x,I,I,4,FVM), 0                         , 0 , 0 , 8630, 414, 121), // #1326
  INST(Vpsubusb        , VexRvm_Lx          , V(660F00,D8,_,x,I,I,4,FVM), 0                         , 0 , 0 , 8638, 414, 121), // #1327
  INST(Vpsubusw        , VexRvm_Lx          , V(660F00,D9,_,x,I,I,4,FVM), 0                         , 0 , 0 , 8647, 414, 121), // #1328
  INST(Vpsubw          , VexRvm_Lx          , V(660F00,F9,_,x,I,I,4,FVM), 0                         , 0 , 0 , 8656, 414, 121), // #1329
  INST(Vpternlogd      , VexRvmi_Lx         , V(660F3A,25,_,x,_,0,4,FV ), 0                         , 0 , 0 , 8663, 417, 104), // #1330
  INST(Vpternlogq      , VexRvmi_Lx         , V(660F3A,25,_,x,_,1,4,FV ), 0                         , 0 , 0 , 8674, 418, 104), // #1331
  INST(Vptest          , VexRm_Lx           , V(660F38,17,_,x,I,_,_,_  ), 0                         , 0 , 0 , 8685, 419, 125), // #1332
  INST(Vptestmb        , VexRvm_Lx          , V(660F38,26,_,x,_,0,4,FVM), 0                         , 0 , 0 , 8692, 406, 106), // #1333
  INST(Vptestmd        , VexRvm_Lx          , V(660F38,27,_,x,_,0,4,FV ), 0                         , 0 , 0 , 8701, 420, 104), // #1334
  INST(Vptestmq        , VexRvm_Lx          , V(660F38,27,_,x,_,1,4,FV ), 0                         , 0 , 0 , 8710, 421, 104), // #1335
  INST(Vptestmw        , VexRvm_Lx          , V(660F38,26,_,x,_,1,4,FVM), 0                         , 0 , 0 , 8719, 406, 106), // #1336
  INST(Vptestnmb       , VexRvm_Lx          , V(F30F38,26,_,x,_,0,4,FVM), 0                         , 0 , 0 , 8728, 406, 106), // #1337
  INST(Vptestnmd       , VexRvm_Lx          , V(F30F38,27,_,x,_,0,4,FV ), 0                         , 0 , 0 , 8738, 420, 104), // #1338
  INST(Vptestnmq       , VexRvm_Lx          , V(F30F38,27,_,x,_,1,4,FV ), 0                         , 0 , 0 , 8748, 421, 104), // #1339
  INST(Vptestnmw       , VexRvm_Lx          , V(F30F38,26,_,x,_,1,4,FVM), 0                         , 0 , 0 , 8758, 406, 106), // #1340
  INST(Vpunpckhbw      , VexRvm_Lx          , V(660F00,68,_,x,I,I,4,FVM), 0                         , 0 , 0 , 8768, 346, 121), // #1341
  INST(Vpunpckhdq      , VexRvm_Lx          , V(660F00,6A,_,x,I,0,4,FV ), 0                         , 0 , 0 , 8779, 229, 109), // #1342
  INST(Vpunpckhqdq     , VexRvm_Lx          , V(660F00,6D,_,x,I,1,4,FV ), 0                         , 0 , 0 , 8790, 228, 109), // #1343
  INST(Vpunpckhwd      , VexRvm_Lx          , V(660F00,69,_,x,I,I,4,FVM), 0                         , 0 , 0 , 8802, 346, 121), // #1344
  INST(Vpunpcklbw      , VexRvm_Lx          , V(660F00,60,_,x,I,I,4,FVM), 0                         , 0 , 0 , 8813, 346, 121), // #1345
  INST(Vpunpckldq      , VexRvm_Lx          , V(660F00,62,_,x,I,0,4,FV ), 0                         , 0 , 0 , 8824, 229, 109), // #1346
  INST(Vpunpcklqdq     , VexRvm_Lx          , V(660F00,6C,_,x,I,1,4,FV ), 0                         , 0 , 0 , 8835, 228, 109), // #1347
  INST(Vpunpcklwd      , VexRvm_Lx          , V(660F00,61,_,x,I,I,4,FVM), 0                         , 0 , 0 , 8847, 346, 121), // #1348
  INST(Vpxor           , VexRvm_Lx          , V(660F00,EF,_,x,I,_,_,_  ), 0                         , 0 , 0 , 8858, 350, 119), // #1349
  INST(Vpxord          , VexRvm_Lx          , V(660F00,EF,_,x,_,0,4,FV ), 0                         , 0 , 0 , 8864, 351, 104), // #1350
  INST(Vpxorq          , VexRvm_Lx          , V(660F00,EF,_,x,_,1,4,FV ), 0                         , 0 , 0 , 8871, 352, 104), // #1351
  INST(Vrangepd        , VexRvmi_Lx         , V(660F3A,50,_,x,_,1,4,FV ), 0                         , 0 , 0 , 8878, 422, 107), // #1352
  INST(Vrangeps        , VexRvmi_Lx         , V(660F3A,50,_,x,_,0,4,FV ), 0                         , 0 , 0 , 8887, 423, 107), // #1353
  INST(Vrangesd        , VexRvmi            , V(660F3A,51,_,I,_,1,3,T1S), 0                         , 0 , 0 , 8896, 318, 60 ), // #1354
  INST(Vrangess        , VexRvmi            , V(660F3A,51,_,I,_,0,2,T1S), 0                         , 0 , 0 , 8905, 319, 60 ), // #1355
  INST(Vrcp14pd        , VexRm_Lx           , V(660F38,4C,_,x,_,1,4,FV ), 0                         , 0 , 0 , 8914, 388, 104), // #1356
  INST(Vrcp14ps        , VexRm_Lx           , V(660F38,4C,_,x,_,0,4,FV ), 0                         , 0 , 0 , 8923, 370, 104), // #1357
  INST(Vrcp14sd        , VexRvm             , V(660F38,4D,_,I,_,1,3,T1S), 0                         , 0 , 0 , 8932, 424, 62 ), // #1358
  INST(Vrcp14ss        , VexRvm             , V(660F38,4D,_,I,_,0,2,T1S), 0                         , 0 , 0 , 8941, 425, 62 ), // #1359
  INST(Vrcp28pd        , VexRm              , V(660F38,CA,_,2,_,1,4,FV ), 0                         , 0 , 0 , 8950, 282, 112), // #1360
  INST(Vrcp28ps        , VexRm              , V(660F38,CA,_,2,_,0,4,FV ), 0                         , 0 , 0 , 8959, 283, 112), // #1361
  INST(Vrcp28sd        , VexRvm             , V(660F38,CB,_,I,_,1,3,T1S), 0                         , 0 , 0 , 8968, 314, 112), // #1362
  INST(Vrcp28ss        , VexRvm             , V(660F38,CB,_,I,_,0,2,T1S), 0                         , 0 , 0 , 8977, 315, 112), // #1363
  INST(Vrcpps          , VexRm_Lx           , V(000F00,53,_,x,I,_,_,_  ), 0                         , 0 , 0 , 8986, 304, 101), // #1364
  INST(Vrcpss          , VexRvm             , V(F30F00,53,_,I,I,_,_,_  ), 0                         , 0 , 0 , 8993, 426, 101), // #1365
  INST(Vreducepd       , VexRmi_Lx          , V(660F3A,56,_,x,_,1,4,FV ), 0                         , 0 , 0 , 9000, 400, 107), // #1366
  INST(Vreduceps       , VexRmi_Lx          , V(660F3A,56,_,x,_,0,4,FV ), 0                         , 0 , 0 , 9010, 399, 107), // #1367
  INST(Vreducesd       , VexRvmi            , V(660F3A,57,_,I,_,1,3,T1S), 0                         , 0 , 0 , 9020, 427, 60 ), // #1368
  INST(Vreducess       , VexRvmi            , V(660F3A,57,_,I,_,0,2,T1S), 0                         , 0 , 0 , 9030, 428, 60 ), // #1369
  INST(Vrndscalepd     , VexRmi_Lx          , V(660F3A,09,_,x,_,1,4,FV ), 0                         , 0 , 0 , 9040, 316, 104), // #1370
  INST(Vrndscaleps     , VexRmi_Lx          , V(660F3A,08,_,x,_,0,4,FV ), 0                         , 0 , 0 , 9052, 317, 104), // #1371
  INST(Vrndscalesd     , VexRvmi            , V(660F3A,0B,_,I,_,1,3,T1S), 0                         , 0 , 0 , 9064, 318, 62 ), // #1372
  INST(Vrndscaless     , VexRvmi            , V(660F3A,0A,_,I,_,0,2,T1S), 0                         , 0 , 0 , 9076, 319, 62 ), // #1373
  INST(Vroundpd        , VexRmi_Lx          , V(660F3A,09,_,x,I,_,_,_  ), 0                         , 0 , 0 , 9088, 429, 101), // #1374
  INST(Vroundps        , VexRmi_Lx          , V(660F3A,08,_,x,I,_,_,_  ), 0                         , 0 , 0 , 9097, 429, 101), // #1375
  INST(Vroundsd        , VexRvmi            , V(660F3A,0B,_,I,I,_,_,_  ), 0                         , 0 , 0 , 9106, 430, 101), // #1376
  INST(Vroundss        , VexRvmi            , V(660F3A,0A,_,I,I,_,_,_  ), 0                         , 0 , 0 , 9115, 431, 101), // #1377
  INST(Vrsqrt14pd      , VexRm_Lx           , V(660F38,4E,_,x,_,1,4,FV ), 0                         , 0 , 0 , 9124, 388, 104), // #1378
  INST(Vrsqrt14ps      , VexRm_Lx           , V(660F38,4E,_,x,_,0,4,FV ), 0                         , 0 , 0 , 9135, 370, 104), // #1379
  INST(Vrsqrt14sd      , VexRvm             , V(660F38,4F,_,I,_,1,3,T1S), 0                         , 0 , 0 , 9146, 424, 62 ), // #1380
  INST(Vrsqrt14ss      , VexRvm             , V(660F38,4F,_,I,_,0,2,T1S), 0                         , 0 , 0 , 9157, 425, 62 ), // #1381
  INST(Vrsqrt28pd      , VexRm              , V(660F38,CC,_,2,_,1,4,FV ), 0                         , 0 , 0 , 9168, 282, 112), // #1382
  INST(Vrsqrt28ps      , VexRm              , V(660F38,CC,_,2,_,0,4,FV ), 0                         , 0 , 0 , 9179, 283, 112), // #1383
  INST(Vrsqrt28sd      , VexRvm             , V(660F38,CD,_,I,_,1,3,T1S), 0                         , 0 , 0 , 9190, 314, 112), // #1384
  INST(Vrsqrt28ss      , VexRvm             , V(660F38,CD,_,I,_,0,2,T1S), 0                         , 0 , 0 , 9201, 315, 112), // #1385
  INST(Vrsqrtps        , VexRm_Lx           , V(000F00,52,_,x,I,_,_,_  ), 0                         , 0 , 0 , 9212, 304, 101), // #1386
  INST(Vrsqrtss        , VexRvm             , V(F30F00,52,_,I,I,_,_,_  ), 0                         , 0 , 0 , 9221, 426, 101), // #1387
  INST(Vscalefpd       , VexRvm_Lx          , V(660F38,2C,_,x,_,1,4,FV ), 0                         , 0 , 0 , 9230, 432, 104), // #1388
  INST(Vscalefps       , VexRvm_Lx          , V(660F38,2C,_,x,_,0,4,FV ), 0                         , 0 , 0 , 9240, 433, 104), // #1389
  INST(Vscalefsd       , VexRvm             , V(660F38,2D,_,I,_,1,3,T1S), 0                         , 0 , 0 , 9250, 434, 62 ), // #1390
  INST(Vscalefss       , VexRvm             , V(660F38,2D,_,I,_,0,2,T1S), 0                         , 0 , 0 , 9260, 435, 62 ), // #1391
  INST(Vscatterdpd     , VexMr_Lx           , V(660F38,A2,_,x,_,1,3,T1S), 0                         , 0 , 0 , 9270, 436, 104), // #1392
  INST(Vscatterdps     , VexMr_Lx           , V(660F38,A2,_,x,_,0,2,T1S), 0                         , 0 , 0 , 9282, 402, 104), // #1393
  INST(Vscatterpf0dpd  , VexM_VM            , V(660F38,C6,5,2,_,1,3,T1S), 0                         , 0 , 0 , 9294, 309, 118), // #1394
  INST(Vscatterpf0dps  , VexM_VM            , V(660F38,C6,5,2,_,0,2,T1S), 0                         , 0 , 0 , 9309, 310, 118), // #1395
  INST(Vscatterpf0qpd  , VexM_VM            , V(660F38,C7,5,2,_,1,3,T1S), 0                         , 0 , 0 , 9324, 311, 118), // #1396
  INST(Vscatterpf0qps  , VexM_VM            , V(660F38,C7,5,2,_,0,2,T1S), 0                         , 0 , 0 , 9339, 311, 118), // #1397
  INST(Vscatterpf1dpd  , VexM_VM            , V(660F38,C6,6,2,_,1,3,T1S), 0                         , 0 , 0 , 9354, 309, 118), // #1398
  INST(Vscatterpf1dps  , VexM_VM            , V(660F38,C6,6,2,_,0,2,T1S), 0                         , 0 , 0 , 9369, 310, 118), // #1399
  INST(Vscatterpf1qpd  , VexM_VM            , V(660F38,C7,6,2,_,1,3,T1S), 0                         , 0 , 0 , 9384, 311, 118), // #1400
  INST(Vscatterpf1qps  , VexM_VM            , V(660F38,C7,6,2,_,0,2,T1S), 0                         , 0 , 0 , 9399, 311, 118), // #1401
  INST(Vscatterqpd     , VexMr_Lx           , V(660F38,A3,_,x,_,1,3,T1S), 0                         , 0 , 0 , 9414, 404, 104), // #1402
  INST(Vscatterqps     , VexMr_Lx           , V(660F38,A3,_,x,_,0,2,T1S), 0                         , 0 , 0 , 9426, 403, 104), // #1403
  INST(Vshuff32x4      , VexRvmi_Lx         , V(660F3A,23,_,x,_,0,4,FV ), 0                         , 0 , 0 , 9438, 437, 104), // #1404
  INST(Vshuff64x2      , VexRvmi_Lx         , V(660F3A,23,_,x,_,1,4,FV ), 0                         , 0 , 0 , 9449, 438, 104), // #1405
  INST(Vshufi32x4      , VexRvmi_Lx         , V(660F3A,43,_,x,_,0,4,FV ), 0                         , 0 , 0 , 9460, 437, 104), // #1406
  INST(Vshufi64x2      , VexRvmi_Lx         , V(660F3A,43,_,x,_,1,4,FV ), 0                         , 0 , 0 , 9471, 438, 104), // #1407
  INST(Vshufpd         , VexRvmi_Lx         , V(660F00,C6,_,x,I,1,4,FV ), 0                         , 0 , 0 , 9482, 439, 99 ), // #1408
  INST(Vshufps         , VexRvmi_Lx         , V(000F00,C6,_,x,I,0,4,FV ), 0                         , 0 , 0 , 9490, 440, 99 ), // #1409
  INST(Vsqrtpd         , VexRm_Lx           , V(660F00,51,_,x,I,1,4,FV ), 0                         , 0 , 0 , 9498, 441, 99 ), // #1410
  INST(Vsqrtps         , VexRm_Lx           , V(000F00,51,_,x,I,0,4,FV ), 0                         , 0 , 0 , 9506, 252, 99 ), // #1411
  INST(Vsqrtsd         , VexRvm             , V(F20F00,51,_,I,I,1,3,T1S), 0                         , 0 , 0 , 9514, 220, 100), // #1412
  INST(Vsqrtss         , VexRvm             , V(F30F00,51,_,I,I,0,2,T1S), 0                         , 0 , 0 , 9522, 221, 100), // #1413
  INST(Vstmxcsr        , VexM               , V(000F00,AE,3,0,I,_,_,_  ), 0                         , 0 , 0 , 9530, 442, 101), // #1414
  INST(Vsubpd          , VexRvm_Lx          , V(660F00,5C,_,x,I,1,4,FV ), 0                         , 0 , 0 , 9539, 218, 99 ), // #1415
  INST(Vsubps          , VexRvm_Lx          , V(000F00,5C,_,x,I,0,4,FV ), 0                         , 0 , 0 , 9546, 219, 99 ), // #1416
  INST(Vsubsd          , VexRvm             , V(F20F00,5C,_,I,I,1,3,T1S), 0                         , 0 , 0 , 9553, 220, 100), // #1417
  INST(Vsubss          , VexRvm             , V(F30F00,5C,_,I,I,0,2,T1S), 0                         , 0 , 0 , 9560, 221, 100), // #1418
  INST(Vtestpd         , VexRm_Lx           , V(660F38,0F,_,x,0,_,_,_  ), 0                         , 0 , 0 , 9567, 419, 125), // #1419
  INST(Vtestps         , VexRm_Lx           , V(660F38,0E,_,x,0,_,_,_  ), 0                         , 0 , 0 , 9575, 419, 125), // #1420
  INST(Vucomisd        , VexRm              , V(660F00,2E,_,I,I,1,3,T1S), 0                         , 0 , 0 , 9583, 248, 110), // #1421
  INST(Vucomiss        , VexRm              , V(000F00,2E,_,I,I,0,2,T1S), 0                         , 0 , 0 , 9592, 249, 110), // #1422
  INST(Vunpckhpd       , VexRvm_Lx          , V(660F00,15,_,x,I,1,4,FV ), 0                         , 0 , 0 , 9601, 228, 99 ), // #1423
  INST(Vunpckhps       , VexRvm_Lx          , V(000F00,15,_,x,I,0,4,FV ), 0                         , 0 , 0 , 9611, 229, 99 ), // #1424
  INST(Vunpcklpd       , VexRvm_Lx          , V(660F00,14,_,x,I,1,4,FV ), 0                         , 0 , 0 , 9621, 228, 99 ), // #1425
  INST(Vunpcklps       , VexRvm_Lx          , V(000F00,14,_,x,I,0,4,FV ), 0                         , 0 , 0 , 9631, 229, 99 ), // #1426
  INST(Vxorpd          , VexRvm_Lx          , V(660F00,57,_,x,I,1,4,FV ), 0                         , 0 , 0 , 9641, 416, 105), // #1427
  INST(Vxorps          , VexRvm_Lx          , V(000F00,57,_,x,I,0,4,FV ), 0                         , 0 , 0 , 9648, 415, 105), // #1428
  INST(Vzeroall        , VexOp              , V(000F00,77,_,1,I,_,_,_  ), 0                         , 0 , 0 , 9655, 443, 101), // #1429
  INST(Vzeroupper      , VexOp              , V(000F00,77,_,0,I,_,_,_  ), 0                         , 0 , 0 , 9664, 443, 101), // #1430
  INST(Wbinvd          , X86Op              , O(000F00,09,_,_,_,_,_,_  ), 0                         , 0 , 0 , 9675, 35 , 0  ), // #1431
  INST(Wrfsbase        , X86M               , O(F30F00,AE,2,_,x,_,_,_  ), 0                         , 0 , 0 , 9682, 444, 89 ), // #1432
  INST(Wrgsbase        , X86M               , O(F30F00,AE,3,_,x,_,_,_  ), 0                         , 0 , 0 , 9691, 444, 89 ), // #1433
  INST(Wrmsr           , X86Op              , O(000F00,30,_,_,_,_,_,_  ), 0                         , 0 , 0 , 9700, 445, 133), // #1434
  INST(Xabort          , X86Op_O_I8         , O(000000,C6,7,_,_,_,_,_  ), 0                         , 0 , 0 , 9706, 98 , 134), // #1435
  INST(Xadd            , X86Xadd            , O(000F00,C0,_,_,x,_,_,_  ), 0                         , 0 , 0 , 9713, 446, 135), // #1436
  INST(Xbegin          , X86JmpRel          , O(000000,C7,7,_,_,_,_,_  ), 0                         , 0 , 0 , 9718, 447, 134), // #1437
  INST(Xchg            , X86Xchg            , O(000000,86,_,_,x,_,_,_  ), 0                         , 0 , 0 , 434 , 448, 0  ), // #1438
  INST(Xend            , X86Op              , O(000F01,D5,_,_,_,_,_,_  ), 0                         , 0 , 0 , 9725, 32 , 134), // #1439
  INST(Xgetbv          , X86Op              , O(000F01,D0,_,_,_,_,_,_  ), 0                         , 0 , 0 , 9730, 199, 136), // #1440
  INST(Xlatb           , X86Op              , O(000000,D7,_,_,_,_,_,_  ), 0                         , 0 , 0 , 9737, 32 , 0  ), // #1441
  INST(Xor             , X86Arith           , O(000000,30,6,_,x,_,_,_  ), 0                         , 0 , 0 , 8860, 212, 1  ), // #1442
  INST(Xorpd           , ExtRm              , O(660F00,57,_,_,_,_,_,_  ), 0                         , 0 , 0 , 9642, 167, 4  ), // #1443
  INST(Xorps           , ExtRm              , O(000F00,57,_,_,_,_,_,_  ), 0                         , 0 , 0 , 9649, 167, 5  ), // #1444
  INST(Xrstor          , X86M_Only          , O(000F00,AE,5,_,_,_,_,_  ), 0                         , 0 , 0 , 1105, 449, 136), // #1445
  INST(Xrstor64        , X86M_Only          , O(000F00,AE,5,_,1,_,_,_  ), 0                         , 0 , 0 , 1113, 450, 136), // #1446
  INST(Xrstors         , X86M_Only          , O(000F00,C7,3,_,_,_,_,_  ), 0                         , 0 , 0 , 9743, 449, 137), // #1447
  INST(Xrstors64       , X86M_Only          , O(000F00,C7,3,_,1,_,_,_  ), 0                         , 0 , 0 , 9751, 450, 137), // #1448
  INST(Xsave           , X86M_Only          , O(000F00,AE,4,_,_,_,_,_  ), 0                         , 0 , 0 , 1123, 451, 136), // #1449
  INST(Xsave64         , X86M_Only          , O(000F00,AE,4,_,1,_,_,_  ), 0                         , 0 , 0 , 1130, 452, 136), // #1450
  INST(Xsavec          , X86M_Only          , O(000F00,C7,4,_,_,_,_,_  ), 0                         , 0 , 0 , 9761, 451, 138), // #1451
  INST(Xsavec64        , X86M_Only          , O(000F00,C7,4,_,1,_,_,_  ), 0                         , 0 , 0 , 9768, 452, 138), // #1452
  INST(Xsaveopt        , X86M_Only          , O(000F00,AE,6,_,_,_,_,_  ), 0                         , 0 , 0 , 9777, 451, 139), // #1453
  INST(Xsaveopt64      , X86M_Only          , O(000F00,AE,6,_,1,_,_,_  ), 0                         , 0 , 0 , 9786, 452, 139), // #1454
  INST(Xsaves          , X86M_Only          , O(000F00,C7,5,_,_,_,_,_  ), 0                         , 0 , 0 , 9797, 451, 137), // #1455
  INST(Xsaves64        , X86M_Only          , O(000F00,C7,5,_,1,_,_,_  ), 0                         , 0 , 0 , 9804, 452, 137), // #1456
  INST(Xsetbv          , X86Op              , O(000F01,D1,_,_,_,_,_,_  ), 0                         , 0 , 0 , 9813, 445, 140), // #1457
  INST(Xtest           , X86Op              , O(000F01,D6,_,_,_,_,_,_  ), 0                         , 0 , 0 , 9820, 32 , 141)  // #1458
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
  E(X86M_Only), E(X86Op), E(X86Op), E(X86M_Only), E(X86Op_ZAX), E(X86Op), E(X86Rm),
  E(X86Rm), E(X86Rm), E(X86Rm), E(X86Rm), E(X86Rm), E(X86Rm), E(X86Rm), E(X86Rm),
  E(X86Rm), E(X86Rm), E(X86Rm), E(X86Rm), E(X86Rm), E(X86Rm), E(X86Rm),
  E(X86Rm), E(X86Rm), E(X86Rm), E(X86Rm), E(X86Rm), E(X86Rm), E(X86Rm), E(X86Rm),
  E(X86Rm), E(X86Rm), E(X86Rm), E(X86Rm), E(X86Rm), E(X86Rm), E(X86Arith), E(ExtRmi),
  E(ExtRmi), E(X86StrMm), E(ExtRmi), E(ExtRmi), E(X86Cmpxchg), E(X86Cmpxchg8b_16b),
  E(X86Cmpxchg8b_16b), E(ExtRm), E(ExtRm), E(X86Op), E(X86Op_xDX_xAX),
  E(X86Crc), E(ExtRm), E(ExtRm), E(ExtRm), E(ExtRm), E(ExtRm), E(ExtRm), E(ExtRm),
  E(ExtRm), E(ExtRm), E(ExtRm), E(ExtRm_Wx), E(ExtRm), E(ExtRm_Wx), E(ExtRm_Wx),
  E(ExtRm), E(ExtRm_Wx), E(ExtRm), E(ExtRm), E(ExtRm), E(ExtRm), E(ExtRm_Wx),
  E(ExtRm_Wx), E(X86Op_xDX_xAX), E(X86Op_xAX), E(X86Op), E(X86Op), E(X86IncDec),
  E(X86M_GPB_MulDiv), E(ExtRm), E(ExtRm), E(ExtRm), E(ExtRm), E(ExtRmi),
  E(ExtRmi), E(X86Op), E(X86Enter), E(ExtExtract), E(ExtExtrq), E(FpuOp), E(FpuOp),
  E(FpuArith), E(FpuRDef), E(X86M_Only), E(X86M_Only), E(FpuOp), E(FpuOp), E(FpuR),
  E(FpuR), E(FpuR), E(FpuR), E(FpuR), E(FpuR), E(FpuR), E(FpuR), E(FpuCom),
  E(FpuR), E(FpuR), E(FpuCom), E(FpuOp), E(FpuOp), E(FpuOp), E(FpuArith), E(FpuRDef),
  E(FpuArith), E(FpuRDef), E(X86Op), E(FpuR), E(FpuM), E(FpuM), E(FpuM),
  E(FpuM), E(FpuM), E(FpuM), E(FpuM), E(FpuOp), E(FpuOp), E(FpuM), E(FpuM), E(FpuM),
  E(FpuM), E(FpuM), E(FpuFldFst), E(FpuOp), E(X86M_Only), E(X86M_Only),
  E(FpuOp), E(FpuOp), E(FpuOp), E(FpuOp), E(FpuOp), E(FpuOp), E(FpuArith), E(FpuRDef),
  E(FpuOp), E(FpuOp), E(FpuOp), E(X86M_Only), E(X86M_Only), E(X86M_Only),
  E(FpuStsw), E(FpuOp), E(FpuOp), E(FpuOp), E(FpuOp), E(FpuOp), E(X86M_Only), E(X86M_Only),
  E(FpuOp), E(FpuOp), E(FpuOp), E(FpuOp), E(FpuFldFst), E(X86M_Only),
  E(X86M_Only), E(FpuFldFst), E(FpuStsw), E(FpuArith), E(FpuRDef), E(FpuArith),
  E(FpuRDef), E(FpuOp), E(FpuRDef), E(FpuR), E(FpuR), E(FpuRDef), E(FpuOp), E(X86Op),
  E(FpuOp), E(FpuR), E(X86M_Only), E(X86M_Only), E(X86M_Only), E(X86M_Only),
  E(FpuOp), E(FpuOp), E(FpuOp), E(ExtRm), E(ExtRm), E(X86Op), E(ExtRm), E(ExtRm),
  E(X86M_GPB_MulDiv), E(X86Imul), E(X86In), E(X86IncDec), E(X86Ins), E(ExtRmi),
  E(ExtInsertq), E(X86Int), E(X86Op), E(X86Op), E(X86Op), E(X86M_Only),
  E(X86Rm_NoRexW), E(X86Op), E(X86Op), E(X86Op), E(X86Op), E(X86Jcc), E(X86Jcc),
  E(X86Jcc), E(X86Jcc), E(X86Jcc), E(X86Jcc), E(X86JecxzLoop), E(X86Jcc), E(X86Jcc),
  E(X86Jcc), E(X86Jcc), E(X86Jmp), E(X86Jcc), E(X86Jcc), E(X86Jcc), E(X86Jcc),
  E(X86Jcc), E(X86Jcc), E(X86Jcc), E(X86Jcc), E(X86Jcc), E(X86Jcc), E(X86Jcc),
  E(X86Jcc), E(X86Jcc), E(X86Jcc), E(X86Jcc), E(X86Jcc), E(X86Jcc), E(X86Jcc),
  E(X86Jcc), E(X86Jcc), E(VexRvm), E(VexRvm), E(VexRvm), E(VexRvm), E(VexRvm),
  E(VexRvm), E(VexRvm), E(VexRvm), E(VexRvm), E(VexRvm), E(VexRvm), E(VexRvm),
  E(VexKmov), E(VexKmov), E(VexKmov), E(VexKmov), E(VexRm), E(VexRm), E(VexRm),
  E(VexRm), E(VexRvm), E(VexRvm), E(VexRvm), E(VexRm), E(VexRm), E(VexRm), E(VexRm),
  E(VexRvm), E(VexRmi), E(VexRmi), E(VexRmi), E(VexRmi), E(VexRmi), E(VexRmi),
  E(VexRmi), E(VexRmi), E(VexRm), E(VexRm), E(VexRm), E(VexRm), E(VexRvm),
  E(VexRvm), E(VexRvm), E(VexRvm), E(VexRvm), E(VexRvm), E(VexRvm), E(VexRvm), E(VexRvm),
  E(VexRvm), E(VexRvm), E(X86Op), E(X86Rm), E(ExtRm), E(X86M_Only), E(X86Rm),
  E(X86Lea), E(X86Op), E(X86Rm), E(X86Fence), E(X86Rm), E(X86M_Only), E(X86Rm),
  E(X86M_Only), E(X86M_NoSize), E(X86M_NoSize), E(X86StrRm), E(X86JecxzLoop),
  E(X86JecxzLoop), E(X86JecxzLoop), E(X86Rm), E(X86Rm), E(X86M_NoSize), E(X86Rm_Raw66H),
  E(ExtRm_ZDI), E(ExtRm_ZDI), E(ExtRm), E(ExtRm), E(ExtRm), E(ExtRm),
  E(X86Fence), E(ExtRm), E(ExtRm), E(ExtRm), E(ExtRm), E(X86Op), E(X86Op),
  E(X86Mov), E(ExtMov), E(ExtMov), E(ExtMovbe), E(ExtMovd), E(ExtMov), E(ExtMov),
  E(ExtMov), E(ExtMov), E(ExtMov), E(ExtMov), E(ExtMov), E(ExtMov), E(ExtMov),
  E(ExtMov), E(ExtMov), E(ExtMov), E(ExtMov), E(ExtMov), E(ExtMovnti), E(ExtMov),
  E(ExtMov), E(ExtMov), E(ExtMov), E(ExtMov), E(ExtMovq), E(ExtRm), E(X86StrMm),
  E(ExtMov), E(ExtRm), E(ExtRm), E(ExtMov), E(X86MovsxMovzx), E(X86Rm), E(ExtMov),
  E(ExtMov), E(X86MovsxMovzx), E(ExtRmi), E(X86M_GPB_MulDiv), E(ExtRm), E(ExtRm),
  E(ExtRm), E(ExtRm), E(VexRvm_ZDX_Wx), E(X86Op), E(X86Op), E(X86M_GPB),
  E(X86Op), E(X86M_GPB), E(X86Arith), E(ExtRm), E(ExtRm), E(X86Out), E(X86Outs),
  E(ExtRm_P), E(ExtRm_P), E(ExtRm_P), E(ExtRm_P), E(ExtRm_P), E(ExtRm), E(ExtRm_P),
  E(ExtRm_P), E(ExtRm_P), E(ExtRm_P), E(ExtRm_P), E(ExtRm_P), E(ExtRm_P),
  E(ExtRm_P), E(ExtRm_P), E(ExtRmi_P), E(ExtRm_P), E(ExtRm_P), E(X86Op), E(ExtRm_P),
  E(Ext3dNow), E(ExtRm_P), E(ExtRm_XMM0), E(ExtRmi), E(ExtRmi), E(ExtRm_P),
  E(ExtRm_P), E(ExtRm), E(ExtRm_P), E(ExtRmi), E(ExtRmi), E(ExtRm_P), E(ExtRm_P),
  E(ExtRm), E(ExtRm_P), E(ExtRmi), E(ExtRmi), E(X86Op_O), E(VexRvm_Wx), E(VexRvm_Wx),
  E(ExtExtract), E(ExtExtract), E(ExtExtract), E(ExtPextrw), E(Ext3dNow),
  E(Ext3dNow), E(Ext3dNow), E(Ext3dNow), E(Ext3dNow), E(Ext3dNow), E(Ext3dNow),
  E(Ext3dNow), E(Ext3dNow), E(Ext3dNow), E(Ext3dNow), E(Ext3dNow), E(Ext3dNow),
  E(Ext3dNow), E(Ext3dNow), E(Ext3dNow), E(Ext3dNow), E(Ext3dNow), E(Ext3dNow),
  E(Ext3dNow), E(Ext3dNow), E(ExtRm_P), E(ExtRm_P), E(ExtRm_P), E(ExtRm), E(ExtRm_P),
  E(ExtRm_P), E(ExtRm_P), E(Ext3dNow), E(Ext3dNow), E(ExtRmi), E(ExtRmi),
  E(ExtRmi), E(ExtRmi_P), E(ExtRm_P), E(ExtRm_P), E(ExtRm), E(ExtRm), E(ExtRm_P),
  E(ExtRm_P), E(ExtRm), E(ExtRm), E(ExtRm), E(ExtRm), E(ExtRm_P), E(ExtRm_P),
  E(ExtRm), E(ExtRm), E(ExtRm_P), E(ExtRm), E(ExtRm), E(ExtRm), E(ExtRm), E(ExtRm),
  E(ExtRm), E(ExtRm), E(ExtRm), E(ExtRm), E(ExtRm), E(ExtRm), E(ExtRm),
  E(ExtRm), E(ExtRm_P), E(Ext3dNow), E(ExtRm_P), E(ExtRm_P), E(ExtRm), E(ExtRm_P),
  E(ExtRm_P), E(X86Pop), E(X86Op), E(X86Op), E(X86Rm_Raw66H), E(X86Op), E(X86Op),
  E(X86Op), E(ExtRm_P), E(X86M_Only), E(X86M_Only), E(X86M_Only), E(X86M_Only),
  E(X86M_Only), E(X86M_Only), E(X86M_Only), E(ExtRm_P), E(ExtRm_P), E(ExtRmi),
  E(ExtRmi), E(ExtRmi), E(ExtRmi_P), E(ExtRm_P), E(ExtRm_P), E(ExtRm_P), E(ExtRmRi_P),
  E(ExtRmRi), E(ExtRmRi_P), E(ExtRmRi_P), E(ExtRmRi_P), E(ExtRmRi_P),
  E(ExtRmRi_P), E(ExtRmRi), E(ExtRmRi_P), E(ExtRmRi_P), E(ExtRm_P), E(ExtRm_P),
  E(ExtRm_P), E(ExtRm_P), E(ExtRm_P), E(ExtRm_P), E(ExtRm_P), E(ExtRm_P), E(Ext3dNow),
  E(ExtRm), E(ExtRm_P), E(ExtRm_P), E(ExtRm), E(ExtRm_P), E(ExtRm_P), E(ExtRm_P),
  E(ExtRm), E(ExtRm_P), E(X86Push), E(X86Op), E(X86Op), E(X86Op), E(X86Op),
  E(X86Op), E(ExtRm_P), E(X86Rot), E(ExtRm), E(ExtRm), E(X86Rot), E(X86M),
  E(X86M), E(X86Op), E(X86Op), E(X86M), E(X86M), E(X86Op), E(X86Op), E(X86Ret),
  E(X86Rot), E(X86Rot), E(VexRmi_Wx), E(ExtRmi), E(ExtRmi), E(ExtRmi), E(ExtRmi),
  E(X86Op), E(ExtRm), E(ExtRm), E(X86Op), E(X86Rot), E(X86Rot), E(VexRmv_Wx),
  E(X86Arith), E(X86StrRm), E(X86Set), E(X86Set), E(X86Set), E(X86Set), E(X86Set),
  E(X86Set), E(X86Set), E(X86Set), E(X86Set), E(X86Set), E(X86Set), E(X86Set),
  E(X86Set), E(X86Set), E(X86Set), E(X86Set), E(X86Set), E(X86Set), E(X86Set),
  E(X86Set), E(X86Set), E(X86Set), E(X86Set), E(X86Set), E(X86Set), E(X86Set),
  E(X86Set), E(X86Set), E(X86Set), E(X86Set), E(X86Fence), E(X86M_Only), E(ExtRm),
  E(ExtRm), E(ExtRm), E(ExtRmi), E(ExtRm), E(ExtRm), E(ExtRm_XMM0), E(X86Rot),
  E(X86ShldShrd), E(VexRmv_Wx), E(X86Rot), E(X86ShldShrd), E(VexRmv_Wx), E(ExtRmi),
  E(ExtRmi), E(X86M_Only), E(X86M), E(X86M), E(ExtRm), E(ExtRm), E(ExtRm),
  E(ExtRm), E(X86Op), E(X86Op), E(X86Op), E(X86Op), E(X86M_Only), E(X86StrMr),
  E(X86M), E(X86Arith), E(ExtRm), E(ExtRm), E(ExtRm), E(ExtRm), E(X86Op), E(X86Op),
  E(X86Op), E(X86Op), E(X86Op), E(X86Op), E(X86Op), E(VexVm_Wx), E(X86Test),
  E(X86Rm_Raw66H), E(VexVm_Wx), E(ExtRm), E(ExtRm), E(X86Op), E(ExtRm), E(ExtRm),
  E(ExtRm), E(ExtRm), E(VexRm_T1_4X), E(VexRm_T1_4X), E(VexRm_T1_4X), E(VexRm_T1_4X),
  E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm), E(VexRvm), E(VexRvm_Lx), E(VexRvm_Lx),
  E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRm), E(VexRmi),
  E(VexRvmi_Lx), E(VexRvmi_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx),
  E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx),
  E(VexRvm_Lx), E(VexRvmi_Lx), E(VexRvmi_Lx), E(VexRvmr_Lx), E(VexRvmr_Lx),
  E(VexRm), E(VexRm_Lx), E(VexRm_Lx), E(VexRm), E(VexRm_Lx), E(VexRm), E(VexRm),
  E(VexRm_Lx), E(VexRm_Lx), E(VexRm), E(VexRm_Lx), E(VexRm), E(VexRm_Lx),
  E(VexRm_Lx), E(VexRvmi_Lx), E(VexRvmi_Lx), E(VexRvmi), E(VexRvmi), E(VexRm), E(VexRm),
  E(VexMr_Lx), E(VexMr_Lx), E(VexRm_Lx), E(VexRm_Lx), E(VexRm_Lx), E(VexRm_Lx),
  E(VexRm_Lx), E(VexRm_Lx), E(VexRm_Lx), E(VexRm_Lx), E(VexRm_Lx), E(VexRm_Lx),
  E(VexMri_Lx), E(VexRm_Lx), E(VexRm_Lx), E(VexRm_Lx), E(VexRm_Lx), E(VexRm_Lx),
  E(VexRm_Wx), E(VexRvm), E(VexRm_Wx), E(VexRvm_Wx), E(VexRvm_Wx), E(VexRvm),
  E(VexRm_Wx), E(VexRm_Wx), E(VexRm_Lx), E(VexRm_Lx), E(VexRm_Lx), E(VexRm_Lx),
  E(VexRm_Lx), E(VexRm_Lx), E(VexRm_Lx), E(VexRm_Lx), E(VexRm_Wx), E(VexRm_Wx),
  E(VexRm_Wx), E(VexRm_Wx), E(VexRm_Lx), E(VexRm_Lx), E(VexRm_Lx), E(VexRm_Lx),
  E(VexRvm_Wx), E(VexRvm_Wx), E(VexRvmi_Lx), E(VexRvm_Lx), E(VexRvm_Lx),
  E(VexRvm), E(VexRvm), E(VexRvmi_Lx), E(VexRvmi_Lx), E(X86M_NoSize), E(X86M_NoSize),
  E(VexRm), E(VexRm), E(VexRm_Lx), E(VexRm_Lx), E(VexMri), E(VexMri_Lx),
  E(VexMri), E(VexMri_Lx), E(VexMri), E(VexMri), E(VexMri_Lx), E(VexMri), E(VexMri_Lx),
  E(VexMri), E(VexMri), E(VexRvmi_Lx), E(VexRvmi_Lx), E(VexRvmi), E(VexRvmi),
  E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm), E(VexRvm), E(VexRvm_Lx), E(VexRvm_Lx),
  E(VexRvm), E(VexRvm), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm), E(VexRvm),
  E(Fma4_Lx), E(Fma4_Lx), E(Fma4), E(Fma4), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx),
  E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(Fma4_Lx), E(Fma4_Lx), E(VexRvm_Lx),
  E(VexRvm_Lx), E(VexRvm), E(VexRvm), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm),
  E(VexRvm), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm), E(VexRvm), E(VexRvm_Lx),
  E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(Fma4_Lx),
  E(Fma4_Lx), E(Fma4_Lx), E(Fma4_Lx), E(Fma4), E(Fma4), E(VexRvm_Lx), E(VexRvm_Lx),
  E(VexRvm), E(VexRvm), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm), E(VexRvm),
  E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm), E(VexRvm), E(Fma4_Lx), E(Fma4_Lx),
  E(Fma4), E(Fma4), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm), E(VexRvm), E(VexRvm_Lx),
  E(VexRvm_Lx), E(VexRvm), E(VexRvm), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm),
  E(VexRvm), E(Fma4_Lx), E(Fma4_Lx), E(Fma4), E(Fma4), E(VexRmi_Lx), E(VexRmi_Lx),
  E(VexRmi_Lx), E(VexRmi_Lx), E(VexRm_Lx), E(VexRm_Lx), E(VexRm), E(VexRm),
  E(VexRmvRm_VM), E(VexRmvRm_VM), E(VexM_VM), E(VexM_VM), E(VexM_VM), E(VexM_VM),
  E(VexM_VM), E(VexM_VM), E(VexM_VM), E(VexM_VM), E(VexRmvRm_VM), E(VexRmvRm_VM),
  E(VexRm_Lx), E(VexRm_Lx), E(VexRvm), E(VexRvm), E(VexRmi_Lx), E(VexRmi_Lx),
  E(VexRvmi), E(VexRvmi), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx),
  E(VexRvmi), E(VexRvmi_Lx), E(VexRvmi), E(VexRvmi_Lx), E(VexRvmi), E(VexRvmi),
  E(VexRvmi_Lx), E(VexRvmi), E(VexRvmi_Lx), E(VexRvmi), E(VexRvmi), E(VexRm_Lx),
  E(VexM), E(VexRm_ZDI), E(VexRvmMvr_Lx), E(VexRvmMvr_Lx), E(VexRvm_Lx), E(VexRvm_Lx),
  E(VexRvm), E(VexRvm), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm), E(VexRvm),
  E(VexRmMr_Lx), E(VexRmMr_Lx), E(VexMovdMovq), E(VexRm_Lx), E(VexRmMr_Lx),
  E(VexRmMr_Lx), E(VexRmMr_Lx), E(VexRmMr_Lx), E(VexRmMr_Lx), E(VexRmMr_Lx), E(VexRmMr_Lx),
  E(VexRmMr_Lx), E(VexRvm), E(VexRvmMr), E(VexRvmMr), E(VexRvm), E(VexRvmMr),
  E(VexRvmMr), E(VexRm_Lx), E(VexRm_Lx), E(VexMr_Lx), E(VexRm_Lx), E(VexMr_Lx),
  E(VexMr_Lx), E(VexMovdMovq), E(VexMovssMovsd), E(VexRm_Lx), E(VexRm_Lx),
  E(VexMovssMovsd), E(VexRmMr_Lx), E(VexRmMr_Lx), E(VexRvmi_Lx), E(VexRvm_Lx),
  E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRm_T1_4X),
  E(VexRm_T1_4X), E(VexRm_Lx), E(VexRm_Lx), E(VexRm_Lx), E(VexRm_Lx),
  E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx),
  E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx),
  E(VexRvm_Lx), E(VexRvmi_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx),
  E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvmi_Lx),
  E(VexRvmr), E(VexRvmi_Lx), E(VexRm_Lx), E(VexRm_Lx), E(VexRm_Lx), E(VexRm_Lx),
  E(VexRm_Lx), E(VexRm_Lx), E(VexRvmi_Lx), E(VexRvrmRvmr_Lx), E(VexRvmi_Lx),
  E(VexRvmi_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRmi),
  E(VexRmi), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRmi),
  E(VexRmi), E(VexRvmi_Lx), E(VexRvmi_Lx), E(VexRvmi_Lx), E(VexRvmi_Lx), E(VexRvmi_Lx),
  E(VexRvmi_Lx), E(VexRvmi), E(VexRvmi), E(VexMr_Lx), E(VexMr_Lx), E(VexMr_Lx),
  E(VexMr_Lx), E(VexRvmi), E(VexRvmi), E(VexRvmi), E(VexRvmi), E(VexRvmi),
  E(VexRvmi), E(VexRm_Lx), E(VexRm_Lx), E(VexRvmi), E(VexRvmi), E(VexRvm_Lx),
  E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx),
  E(VexRvm_Lx), E(VexRvrmiRvmri_Lx), E(VexRvrmiRvmri_Lx), E(VexRvmRmi_Lx),
  E(VexRvmRmi_Lx), E(VexRmi), E(VexRvm), E(VexRvmRmi_Lx), E(VexRvm_Lx), E(VexRvm_Lx),
  E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx),
  E(VexRm_Lx), E(VexRm_Lx), E(VexRm_Lx), E(VexRm_Lx), E(VexMri), E(VexMri),
  E(VexMri), E(VexMri), E(VexRmvRm_VM), E(VexRmvRm_VM), E(VexRmvRm_VM), E(VexRmvRm_VM),
  E(VexRm), E(VexRm), E(VexRm), E(VexRvm_Lx), E(VexRm), E(VexRvm_Lx), E(VexRm),
  E(VexRm), E(VexRm), E(VexRm), E(VexRm), E(VexRm), E(VexRvm_Lx), E(VexRm),
  E(VexRm), E(VexRm), E(VexRm), E(VexRvm_Lx), E(VexRm), E(VexRvm_Lx), E(VexRvm_Lx),
  E(VexRm), E(VexRvmi), E(VexRvmi), E(VexRvmi), E(VexRvmi), E(VexRm_Lx),
  E(VexRm_Lx), E(VexRvmr), E(VexRvmr), E(VexRvmr), E(VexRvmr), E(VexRvmr), E(VexRvmr),
  E(VexRvmr), E(VexRvmr), E(VexRvmr), E(VexRvmr), E(VexRvmr), E(VexRvmr),
  E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvmMvr_Lx), E(VexRvmMvr_Lx),
  E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx),
  E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx),
  E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx),
  E(VexRm_Lx), E(VexRm_Lx), E(VexMr_Lx), E(VexMr_Lx), E(VexRm_Lx), E(VexRm_Lx),
  E(VexRm_Lx), E(VexRm_Lx), E(VexRm_Lx), E(VexRm_Lx), E(VexMr_Lx), E(VexMr_Lx),
  E(VexMr_Lx), E(VexMr_Lx), E(VexMr_Lx), E(VexMr_Lx), E(VexMr_Lx), E(VexMr_Lx),
  E(VexMr_Lx), E(VexRm_Lx), E(VexRm_Lx), E(VexRm_Lx), E(VexRm_Lx), E(VexRm_Lx),
  E(VexRm_Lx), E(VexMr_Lx), E(VexMr_Lx), E(VexMr_Lx), E(VexMr_Lx), E(VexMr_Lx),
  E(VexMr_Lx), E(VexRm_Lx), E(VexMr_Lx), E(VexRm_Lx), E(VexRm_Lx), E(VexRm_Lx),
  E(VexRm_Lx), E(VexRm_Lx), E(VexRm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx),
  E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx),
  E(VexRm_Lx), E(VexRm_Lx), E(VexRm_Lx), E(VexRm_Lx), E(VexRvm_Lx),
  E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvrmRvmr), E(VexVmi_Lx), E(VexVmi_Lx), E(VexRvm_Lx),
  E(VexRvm_Lx), E(VexVmi_Lx), E(VexVmi_Lx), E(VexRvm_Lx), E(VexRvm_Lx),
  E(VexRvmRmvRmi), E(VexRvmRmvRmi), E(VexRvmRmvRmi), E(VexRvmRmvRmi), E(VexRvm_Lx),
  E(VexMr_VM), E(VexMr_VM), E(VexMr_VM), E(VexMr_VM), E(VexRvmRmv), E(VexRvmRmv),
  E(VexRvmRmv), E(VexRvmRmv), E(VexRvmRmv), E(VexRvmRmv), E(VexRvmi_Lx),
  E(VexRvmi_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvmi_Lx), E(VexRvmRmv),
  E(VexRvmRmv), E(VexRvmi_Lx), E(VexRvmi_Lx), E(VexRvm_Lx), E(VexRvm_Lx),
  E(VexRvm_Lx), E(VexRvmi_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRmi_Lx), E(VexRmi_Lx),
  E(VexRmi_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvmVmi_Lx),
  E(VexEvexVmi_Lx), E(VexRvmVmi_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx),
  E(VexRvmVmi_Lx), E(VexRvmVmi_Lx), E(VexRvmVmi_Lx), E(VexRvm_Lx), E(VexRvm_Lx),
  E(VexRvm_Lx), E(VexRvmVmi_Lx), E(VexRvmVmi_Lx), E(VexEvexVmi_Lx), E(VexRvmVmi_Lx),
  E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvmVmi_Lx), E(VexRvm_Lx),
  E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx),
  E(VexRvm_Lx), E(VexRvmi_Lx), E(VexRvmi_Lx), E(VexRm_Lx), E(VexRvm_Lx),
  E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx),
  E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx),
  E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx),
  E(VexRvm_Lx), E(VexRvmi_Lx), E(VexRvmi_Lx), E(VexRvmi), E(VexRvmi), E(VexRm_Lx),
  E(VexRm_Lx), E(VexRvm), E(VexRvm), E(VexRm), E(VexRm), E(VexRvm), E(VexRvm),
  E(VexRm_Lx), E(VexRvm), E(VexRmi_Lx), E(VexRmi_Lx), E(VexRvmi), E(VexRvmi),
  E(VexRmi_Lx), E(VexRmi_Lx), E(VexRvmi), E(VexRvmi), E(VexRmi_Lx), E(VexRmi_Lx),
  E(VexRvmi), E(VexRvmi), E(VexRm_Lx), E(VexRm_Lx), E(VexRvm), E(VexRvm),
  E(VexRm), E(VexRm), E(VexRvm), E(VexRvm), E(VexRm_Lx), E(VexRvm), E(VexRvm_Lx),
  E(VexRvm_Lx), E(VexRvm), E(VexRvm), E(VexMr_Lx), E(VexMr_Lx), E(VexM_VM),
  E(VexM_VM), E(VexM_VM), E(VexM_VM), E(VexM_VM), E(VexM_VM), E(VexM_VM), E(VexM_VM),
  E(VexMr_Lx), E(VexMr_Lx), E(VexRvmi_Lx), E(VexRvmi_Lx), E(VexRvmi_Lx),
  E(VexRvmi_Lx), E(VexRvmi_Lx), E(VexRvmi_Lx), E(VexRm_Lx), E(VexRm_Lx), E(VexRvm),
  E(VexRvm), E(VexM), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm), E(VexRvm), E(VexRm_Lx),
  E(VexRm_Lx), E(VexRm), E(VexRm), E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx),
  E(VexRvm_Lx), E(VexRvm_Lx), E(VexRvm_Lx), E(VexOp), E(VexOp), E(X86Op),
  E(X86M), E(X86M), E(X86Op), E(X86Op_O_I8), E(X86Xadd), E(X86JmpRel), E(X86Xchg),
  E(X86Op), E(X86Op), E(X86Op), E(X86Arith), E(ExtRm), E(ExtRm), E(X86M_Only),
  E(X86M_Only), E(X86M_Only), E(X86M_Only), E(X86M_Only), E(X86M_Only), E(X86M_Only),
  E(X86M_Only), E(X86M_Only), E(X86M_Only), E(X86M_Only), E(X86M_Only),
  E(X86Op), E(X86Op)
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
  O(000F00,AE,7,_,_,_,_,_  ), // #66
  O(660F00,AE,7,_,_,_,_,_  ), // #67
  O(000000,FA,_,_,_,_,_,_  ), // #68
  O(000F00,06,_,_,_,_,_,_  ), // #69
  O(660F00,AE,6,_,_,_,_,_  ), // #70
  O(000F01,FC,_,_,_,_,_,_  ), // #71
  O(000000,F5,_,_,_,_,_,_  ), // #72
  O(000F00,47,_,_,x,_,_,_  ), // #73
  O(000F00,43,_,_,x,_,_,_  ), // #74
  O(000F00,42,_,_,x,_,_,_  ), // #75
  O(000F00,46,_,_,x,_,_,_  ), // #76
  O(000F00,42,_,_,x,_,_,_  ), // #77
  O(000F00,44,_,_,x,_,_,_  ), // #78
  O(000F00,4F,_,_,x,_,_,_  ), // #79
  O(000F00,4D,_,_,x,_,_,_  ), // #80
  O(000F00,4C,_,_,x,_,_,_  ), // #81
  O(000F00,4E,_,_,x,_,_,_  ), // #82
  O(000F00,46,_,_,x,_,_,_  ), // #83
  O(000F00,42,_,_,x,_,_,_  ), // #84
  O(000F00,43,_,_,x,_,_,_  ), // #85
  O(000F00,47,_,_,x,_,_,_  ), // #86
  O(000F00,43,_,_,x,_,_,_  ), // #87
  O(000F00,45,_,_,x,_,_,_  ), // #88
  O(000F00,4E,_,_,x,_,_,_  ), // #89
  O(000F00,4C,_,_,x,_,_,_  ), // #90
  O(000F00,4D,_,_,x,_,_,_  ), // #91
  O(000F00,4F,_,_,x,_,_,_  ), // #92
  O(000F00,41,_,_,x,_,_,_  ), // #93
  O(000F00,4B,_,_,x,_,_,_  ), // #94
  O(000F00,49,_,_,x,_,_,_  ), // #95
  O(000F00,45,_,_,x,_,_,_  ), // #96
  O(000F00,40,_,_,x,_,_,_  ), // #97
  O(000F00,4A,_,_,x,_,_,_  ), // #98
  O(000F00,4A,_,_,x,_,_,_  ), // #99
  O(000F00,4B,_,_,x,_,_,_  ), // #100
  O(000F00,48,_,_,x,_,_,_  ), // #101
  O(000F00,44,_,_,x,_,_,_  ), // #102
  O(000000,38,7,_,x,_,_,_  ), // #103
  O(660F00,C2,_,_,_,_,_,_  ), // #104
  O(000F00,C2,_,_,_,_,_,_  ), // #105
  O(000000,A6,_,_,_,_,_,_  ), // #106
  O(F20F00,C2,_,_,_,_,_,_  ), // #107
  O(F30F00,C2,_,_,_,_,_,_  ), // #108
  O(000F00,B0,_,_,x,_,_,_  ), // #109
  O(000F00,C7,1,_,1,_,_,_  ), // #110
  O(000F00,C7,1,_,_,_,_,_  ), // #111
  O(660F00,2F,_,_,_,_,_,_  ), // #112
  O(000F00,2F,_,_,_,_,_,_  ), // #113
  O(000F00,A2,_,_,_,_,_,_  ), // #114
  O(000000,99,_,_,1,_,_,_  ), // #115
  O(F20F38,F0,_,_,x,_,_,_  ), // #116
  O(F30F00,E6,_,_,_,_,_,_  ), // #117
  O(000F00,5B,_,_,_,_,_,_  ), // #118
  O(F20F00,E6,_,_,_,_,_,_  ), // #119
  O(660F00,2D,_,_,_,_,_,_  ), // #120
  O(660F00,5A,_,_,_,_,_,_  ), // #121
  O(660F00,2A,_,_,_,_,_,_  ), // #122
  O(000F00,2A,_,_,_,_,_,_  ), // #123
  O(660F00,5B,_,_,_,_,_,_  ), // #124
  O(000F00,5A,_,_,_,_,_,_  ), // #125
  O(000F00,2D,_,_,_,_,_,_  ), // #126
  O(F20F00,2D,_,_,x,_,_,_  ), // #127
  O(F20F00,5A,_,_,_,_,_,_  ), // #128
  O(F20F00,2A,_,_,x,_,_,_  ), // #129
  O(F30F00,2A,_,_,x,_,_,_  ), // #130
  O(F30F00,5A,_,_,_,_,_,_  ), // #131
  O(F30F00,2D,_,_,x,_,_,_  ), // #132
  O(660F00,E6,_,_,_,_,_,_  ), // #133
  O(660F00,2C,_,_,_,_,_,_  ), // #134
  O(F30F00,5B,_,_,_,_,_,_  ), // #135
  O(000F00,2C,_,_,_,_,_,_  ), // #136
  O(F20F00,2C,_,_,x,_,_,_  ), // #137
  O(F30F00,2C,_,_,x,_,_,_  ), // #138
  O(660000,99,_,_,_,_,_,_  ), // #139
  O(000000,98,_,_,_,_,_,_  ), // #140
  O(000000,27,_,_,_,_,_,_  ), // #141
  O(000000,2F,_,_,_,_,_,_  ), // #142
  O(000000,FE,1,_,x,_,_,_  ), // #143
  O(000000,F6,6,_,x,_,_,_  ), // #144
  O(660F00,5E,_,_,_,_,_,_  ), // #145
  O(000F00,5E,_,_,_,_,_,_  ), // #146
  O(F20F00,5E,_,_,_,_,_,_  ), // #147
  O(F30F00,5E,_,_,_,_,_,_  ), // #148
  O(660F3A,41,_,_,_,_,_,_  ), // #149
  O(660F3A,40,_,_,_,_,_,_  ), // #150
  O(000F00,77,_,_,_,_,_,_  ), // #151
  O(000000,C8,_,_,_,_,_,_  ), // #152
  O(660F3A,17,_,_,_,_,_,_  ), // #153
  O(660F00,79,_,_,_,_,_,_  ), // #154
  O_FPU(00,D9F0,_)          , // #155
  O_FPU(00,D9E1,_)          , // #156
  O_FPU(00,C0C0,0)          , // #157
  O_FPU(00,DEC0,_)          , // #158
  O_FPU(00,00DF,4)          , // #159
  O_FPU(00,00DF,6)          , // #160
  O_FPU(00,D9E0,_)          , // #161
  O_FPU(9B,DBE2,_)          , // #162
  O_FPU(00,DAC0,_)          , // #163
  O_FPU(00,DAD0,_)          , // #164
  O_FPU(00,DAC8,_)          , // #165
  O_FPU(00,DBC0,_)          , // #166
  O_FPU(00,DBD0,_)          , // #167
  O_FPU(00,DBC8,_)          , // #168
  O_FPU(00,DBD8,_)          , // #169
  O_FPU(00,DAD8,_)          , // #170
  O_FPU(00,D0D0,2)          , // #171
  O_FPU(00,DBF0,_)          , // #172
  O_FPU(00,DFF0,_)          , // #173
  O_FPU(00,D8D8,3)          , // #174
  O_FPU(00,DED9,_)          , // #175
  O_FPU(00,D9FF,_)          , // #176
  O_FPU(00,D9F6,_)          , // #177
  O_FPU(00,F0F8,6)          , // #178
  O_FPU(00,DEF8,_)          , // #179
  O_FPU(00,F8F0,7)          , // #180
  O_FPU(00,DEF0,_)          , // #181
  O(000F00,0E,_,_,_,_,_,_  ), // #182
  O_FPU(00,DDC0,_)          , // #183
  O_FPU(00,00DA,0)          , // #184
  O_FPU(00,00DA,2)          , // #185
  O_FPU(00,00DA,3)          , // #186
  O_FPU(00,00DA,6)          , // #187
  O_FPU(00,00DA,7)          , // #188
  O_FPU(00,00DB,0)          , // #189
  O_FPU(00,00DA,1)          , // #190
  O_FPU(00,D9F7,_)          , // #191
  O_FPU(9B,DBE3,_)          , // #192
  O_FPU(00,00DB,2)          , // #193
  O_FPU(00,00DB,3)          , // #194
  O_FPU(00,00DB,1)          , // #195
  O_FPU(00,00DA,4)          , // #196
  O_FPU(00,00DA,5)          , // #197
  O_FPU(00,00D9,0)          , // #198
  O_FPU(00,D9E8,_)          , // #199
  O_FPU(00,00D9,5)          , // #200
  O_FPU(00,00D9,4)          , // #201
  O_FPU(00,D9EA,_)          , // #202
  O_FPU(00,D9E9,_)          , // #203
  O_FPU(00,D9EC,_)          , // #204
  O_FPU(00,D9ED,_)          , // #205
  O_FPU(00,D9EB,_)          , // #206
  O_FPU(00,D9EE,_)          , // #207
  O_FPU(00,C8C8,1)          , // #208
  O_FPU(00,DEC8,_)          , // #209
  O_FPU(00,DBE2,_)          , // #210
  O_FPU(00,DBE3,_)          , // #211
  O_FPU(00,D9D0,_)          , // #212
  O_FPU(00,00DD,6)          , // #213
  O_FPU(00,00D9,7)          , // #214
  O_FPU(00,00D9,6)          , // #215
  O_FPU(00,00DD,7)          , // #216
  O_FPU(00,D9F3,_)          , // #217
  O_FPU(00,D9F8,_)          , // #218
  O_FPU(00,D9F5,_)          , // #219
  O_FPU(00,D9F2,_)          , // #220
  O_FPU(00,D9FC,_)          , // #221
  O_FPU(00,00DD,4)          , // #222
  O_FPU(9B,00DD,6)          , // #223
  O_FPU(00,D9FD,_)          , // #224
  O_FPU(00,D9FE,_)          , // #225
  O_FPU(00,D9FB,_)          , // #226
  O_FPU(00,D9FA,_)          , // #227
  O_FPU(00,00D9,2)          , // #228
  O_FPU(9B,00D9,7)          , // #229
  O_FPU(9B,00D9,6)          , // #230
  O_FPU(00,00D9,3)          , // #231
  O_FPU(9B,00DD,7)          , // #232
  O_FPU(00,E0E8,4)          , // #233
  O_FPU(00,DEE8,_)          , // #234
  O_FPU(00,E8E0,5)          , // #235
  O_FPU(00,DEE0,_)          , // #236
  O_FPU(00,D9E4,_)          , // #237
  O_FPU(00,DDE0,_)          , // #238
  O_FPU(00,DBE8,_)          , // #239
  O_FPU(00,DFE8,_)          , // #240
  O_FPU(00,DDE8,_)          , // #241
  O_FPU(00,DAE9,_)          , // #242
  O_FPU(00,00DB,_)          , // #243
  O_FPU(00,D9E5,_)          , // #244
  O_FPU(00,D9C8,_)          , // #245
  O(000F00,AE,1,_,_,_,_,_  ), // #246
  O(000F00,AE,1,_,1,_,_,_  ), // #247
  O(000F00,AE,0,_,_,_,_,_  ), // #248
  O(000F00,AE,0,_,1,_,_,_  ), // #249
  O_FPU(00,D9F4,_)          , // #250
  O_FPU(00,D9F1,_)          , // #251
  O_FPU(00,D9F9,_)          , // #252
  O(660F00,7C,_,_,_,_,_,_  ), // #253
  O(F20F00,7C,_,_,_,_,_,_  ), // #254
  O(000000,F4,_,_,_,_,_,_  ), // #255
  O(660F00,7D,_,_,_,_,_,_  ), // #256
  O(F20F00,7D,_,_,_,_,_,_  ), // #257
  O(000000,F6,7,_,x,_,_,_  ), // #258
  O(000000,F6,5,_,x,_,_,_  ), // #259
  O(000000,EC,_,_,_,_,_,_  ), // #260
  O(000000,FE,0,_,x,_,_,_  ), // #261
  O(000000,6C,_,_,_,_,_,_  ), // #262
  O(660F3A,21,_,_,_,_,_,_  ), // #263
  O(F20F00,79,_,_,_,_,_,_  ), // #264
  O(000000,CD,_,_,_,_,_,_  ), // #265
  O(000000,CC,_,_,_,_,_,_  ), // #266
  O(000000,CE,_,_,_,_,_,_  ), // #267
  O(000F00,08,_,_,_,_,_,_  ), // #268
  O(000F00,01,7,_,_,_,_,_  ), // #269
  O(660F38,82,_,_,_,_,_,_  ), // #270
  O(000000,CF,_,_,_,_,_,_  ), // #271
  O(000000,CF,_,_,_,_,_,_  ), // #272
  O(000000,CF,_,_,1,_,_,_  ), // #273
  O(660000,CF,_,_,_,_,_,_  ), // #274
  O(000F00,87,_,_,_,_,_,_  ), // #275
  O(000F00,83,_,_,_,_,_,_  ), // #276
  O(000F00,82,_,_,_,_,_,_  ), // #277
  O(000F00,86,_,_,_,_,_,_  ), // #278
  O(000F00,82,_,_,_,_,_,_  ), // #279
  O(000F00,84,_,_,_,_,_,_  ), // #280
  0                         , // #281
  O(000F00,8F,_,_,_,_,_,_  ), // #282
  O(000F00,8D,_,_,_,_,_,_  ), // #283
  O(000F00,8C,_,_,_,_,_,_  ), // #284
  O(000F00,8E,_,_,_,_,_,_  ), // #285
  O(000000,FF,4,_,_,_,_,_  ), // #286
  O(000F00,86,_,_,_,_,_,_  ), // #287
  O(000F00,82,_,_,_,_,_,_  ), // #288
  O(000F00,83,_,_,_,_,_,_  ), // #289
  O(000F00,87,_,_,_,_,_,_  ), // #290
  O(000F00,83,_,_,_,_,_,_  ), // #291
  O(000F00,85,_,_,_,_,_,_  ), // #292
  O(000F00,8E,_,_,_,_,_,_  ), // #293
  O(000F00,8C,_,_,_,_,_,_  ), // #294
  O(000F00,8D,_,_,_,_,_,_  ), // #295
  O(000F00,8F,_,_,_,_,_,_  ), // #296
  O(000F00,81,_,_,_,_,_,_  ), // #297
  O(000F00,8B,_,_,_,_,_,_  ), // #298
  O(000F00,89,_,_,_,_,_,_  ), // #299
  O(000F00,85,_,_,_,_,_,_  ), // #300
  O(000F00,80,_,_,_,_,_,_  ), // #301
  O(000F00,8A,_,_,_,_,_,_  ), // #302
  O(000F00,8A,_,_,_,_,_,_  ), // #303
  O(000F00,8B,_,_,_,_,_,_  ), // #304
  O(000F00,88,_,_,_,_,_,_  ), // #305
  O(000F00,84,_,_,_,_,_,_  ), // #306
  V(660F00,4A,_,1,0,_,_,_  ), // #307
  V(660F00,4A,_,1,1,_,_,_  ), // #308
  V(000F00,4A,_,1,1,_,_,_  ), // #309
  V(000F00,4A,_,1,0,_,_,_  ), // #310
  V(660F00,41,_,1,0,_,_,_  ), // #311
  V(660F00,41,_,1,1,_,_,_  ), // #312
  V(660F00,42,_,1,0,_,_,_  ), // #313
  V(660F00,42,_,1,1,_,_,_  ), // #314
  V(000F00,42,_,1,1,_,_,_  ), // #315
  V(000F00,42,_,1,0,_,_,_  ), // #316
  V(000F00,41,_,1,1,_,_,_  ), // #317
  V(000F00,41,_,1,0,_,_,_  ), // #318
  V(660F00,90,_,0,0,_,_,_  ), // #319
  V(660F00,90,_,0,1,_,_,_  ), // #320
  V(000F00,90,_,0,1,_,_,_  ), // #321
  V(000F00,90,_,0,0,_,_,_  ), // #322
  V(660F00,44,_,0,0,_,_,_  ), // #323
  V(660F00,44,_,0,1,_,_,_  ), // #324
  V(000F00,44,_,0,1,_,_,_  ), // #325
  V(000F00,44,_,0,0,_,_,_  ), // #326
  V(660F00,45,_,1,0,_,_,_  ), // #327
  V(660F00,45,_,1,1,_,_,_  ), // #328
  V(000F00,45,_,1,1,_,_,_  ), // #329
  V(660F00,98,_,0,0,_,_,_  ), // #330
  V(660F00,98,_,0,1,_,_,_  ), // #331
  V(000F00,98,_,0,1,_,_,_  ), // #332
  V(000F00,98,_,0,0,_,_,_  ), // #333
  V(000F00,45,_,1,0,_,_,_  ), // #334
  V(660F3A,32,_,0,0,_,_,_  ), // #335
  V(660F3A,33,_,0,0,_,_,_  ), // #336
  V(660F3A,33,_,0,1,_,_,_  ), // #337
  V(660F3A,32,_,0,1,_,_,_  ), // #338
  V(660F3A,30,_,0,0,_,_,_  ), // #339
  V(660F3A,31,_,0,0,_,_,_  ), // #340
  V(660F3A,31,_,0,1,_,_,_  ), // #341
  V(660F3A,30,_,0,1,_,_,_  ), // #342
  V(660F00,99,_,0,0,_,_,_  ), // #343
  V(660F00,99,_,0,1,_,_,_  ), // #344
  V(000F00,99,_,0,1,_,_,_  ), // #345
  V(000F00,99,_,0,0,_,_,_  ), // #346
  V(660F00,4B,_,1,0,_,_,_  ), // #347
  V(000F00,4B,_,1,1,_,_,_  ), // #348
  V(000F00,4B,_,1,0,_,_,_  ), // #349
  V(660F00,46,_,1,0,_,_,_  ), // #350
  V(660F00,46,_,1,1,_,_,_  ), // #351
  V(000F00,46,_,1,1,_,_,_  ), // #352
  V(000F00,46,_,1,0,_,_,_  ), // #353
  V(660F00,47,_,1,0,_,_,_  ), // #354
  V(660F00,47,_,1,1,_,_,_  ), // #355
  V(000F00,47,_,1,1,_,_,_  ), // #356
  V(000F00,47,_,1,0,_,_,_  ), // #357
  O(000000,9F,_,_,_,_,_,_  ), // #358
  O(000F00,02,_,_,_,_,_,_  ), // #359
  O(F20F00,F0,_,_,_,_,_,_  ), // #360
  O(000F00,AE,2,_,_,_,_,_  ), // #361
  O(000000,C5,_,_,_,_,_,_  ), // #362
  O(000000,8D,_,_,x,_,_,_  ), // #363
  O(000000,C9,_,_,_,_,_,_  ), // #364
  O(000000,C4,_,_,_,_,_,_  ), // #365
  O(000F00,AE,5,_,_,_,_,_  ), // #366
  O(000F00,B4,_,_,_,_,_,_  ), // #367
  O(000F00,01,2,_,_,_,_,_  ), // #368
  O(000F00,B5,_,_,_,_,_,_  ), // #369
  O(000F00,01,3,_,_,_,_,_  ), // #370
  O(000F00,00,2,_,_,_,_,_  ), // #371
  O(000F00,01,6,_,_,_,_,_  ), // #372
  O(000000,AC,_,_,_,_,_,_  ), // #373
  0                         , // #374
  0                         , // #375
  0                         , // #376
  O(000F00,03,_,_,_,_,_,_  ), // #377
  O(000F00,B2,_,_,_,_,_,_  ), // #378
  O(000F00,00,3,_,_,_,_,_  ), // #379
  O(F30F00,BD,_,_,x,_,_,_  ), // #380
  O(660F00,57,_,_,_,_,_,_  ), // #381
  O(000F00,F7,_,_,_,_,_,_  ), // #382
  O(660F00,5F,_,_,_,_,_,_  ), // #383
  O(000F00,5F,_,_,_,_,_,_  ), // #384
  O(F20F00,5F,_,_,_,_,_,_  ), // #385
  O(F30F00,5F,_,_,_,_,_,_  ), // #386
  O(000F00,AE,6,_,_,_,_,_  ), // #387
  O(660F00,5D,_,_,_,_,_,_  ), // #388
  O(000F00,5D,_,_,_,_,_,_  ), // #389
  O(F20F00,5D,_,_,_,_,_,_  ), // #390
  O(F30F00,5D,_,_,_,_,_,_  ), // #391
  O(000F01,C8,_,_,_,_,_,_  ), // #392
  O(000F01,FA,_,_,_,_,_,_  ), // #393
  0                         , // #394
  O(660F00,28,_,_,_,_,_,_  ), // #395
  O(000F00,28,_,_,_,_,_,_  ), // #396
  O(000F38,F0,_,_,x,_,_,_  ), // #397
  O(000F00,6E,_,_,_,_,_,_  ), // #398
  O(F20F00,12,_,_,_,_,_,_  ), // #399
  O(F20F00,D6,_,_,_,_,_,_  ), // #400
  O(660F00,6F,_,_,_,_,_,_  ), // #401
  O(F30F00,6F,_,_,_,_,_,_  ), // #402
  O(000F00,12,_,_,_,_,_,_  ), // #403
  O(660F00,16,_,_,_,_,_,_  ), // #404
  O(000F00,16,_,_,_,_,_,_  ), // #405
  O(000F00,16,_,_,_,_,_,_  ), // #406
  O(660F00,12,_,_,_,_,_,_  ), // #407
  O(000F00,12,_,_,_,_,_,_  ), // #408
  O(660F00,50,_,_,_,_,_,_  ), // #409
  O(000F00,50,_,_,_,_,_,_  ), // #410
  0                         , // #411
  O(660F38,2A,_,_,_,_,_,_  ), // #412
  O(000F00,C3,_,_,x,_,_,_  ), // #413
  0                         , // #414
  0                         , // #415
  0                         , // #416
  0                         , // #417
  0                         , // #418
  O(000F00,6E,_,_,x,_,_,_  ), // #419
  O(F30F00,D6,_,_,_,_,_,_  ), // #420
  O(000000,A4,_,_,_,_,_,_  ), // #421
  O(F20F00,10,_,_,_,_,_,_  ), // #422
  O(F30F00,16,_,_,_,_,_,_  ), // #423
  O(F30F00,12,_,_,_,_,_,_  ), // #424
  O(F30F00,10,_,_,_,_,_,_  ), // #425
  O(000F00,BE,_,_,x,_,_,_  ), // #426
  O(000000,63,_,_,1,_,_,_  ), // #427
  O(660F00,10,_,_,_,_,_,_  ), // #428
  O(000F00,10,_,_,_,_,_,_  ), // #429
  O(000F00,B6,_,_,x,_,_,_  ), // #430
  O(660F3A,42,_,_,_,_,_,_  ), // #431
  O(000000,F6,4,_,x,_,_,_  ), // #432
  O(660F00,59,_,_,_,_,_,_  ), // #433
  O(000F00,59,_,_,_,_,_,_  ), // #434
  O(F20F00,59,_,_,_,_,_,_  ), // #435
  O(F30F00,59,_,_,_,_,_,_  ), // #436
  V(F20F38,F6,_,0,x,_,_,_  ), // #437
  O(000F01,C9,_,_,_,_,_,_  ), // #438
  O(000F01,FB,_,_,_,_,_,_  ), // #439
  O(000000,F6,3,_,x,_,_,_  ), // #440
  O(000000,90,_,_,_,_,_,_  ), // #441
  O(000000,F6,2,_,x,_,_,_  ), // #442
  O(000000,08,1,_,x,_,_,_  ), // #443
  O(660F00,56,_,_,_,_,_,_  ), // #444
  O(000F00,56,_,_,_,_,_,_  ), // #445
  O(000000,EE,_,_,_,_,_,_  ), // #446
  O(000000,6E,_,_,_,_,_,_  ), // #447
  O(000F38,1C,_,_,_,_,_,_  ), // #448
  O(000F38,1E,_,_,_,_,_,_  ), // #449
  O(000F38,1D,_,_,_,_,_,_  ), // #450
  O(000F00,6B,_,_,_,_,_,_  ), // #451
  O(000F00,63,_,_,_,_,_,_  ), // #452
  O(660F38,2B,_,_,_,_,_,_  ), // #453
  O(000F00,67,_,_,_,_,_,_  ), // #454
  O(000F00,FC,_,_,_,_,_,_  ), // #455
  O(000F00,FE,_,_,_,_,_,_  ), // #456
  O(000F00,D4,_,_,_,_,_,_  ), // #457
  O(000F00,EC,_,_,_,_,_,_  ), // #458
  O(000F00,ED,_,_,_,_,_,_  ), // #459
  O(000F00,DC,_,_,_,_,_,_  ), // #460
  O(000F00,DD,_,_,_,_,_,_  ), // #461
  O(000F00,FD,_,_,_,_,_,_  ), // #462
  O(000F3A,0F,_,_,_,_,_,_  ), // #463
  O(000F00,DB,_,_,_,_,_,_  ), // #464
  O(000F00,DF,_,_,_,_,_,_  ), // #465
  O(F30000,90,_,_,_,_,_,_  ), // #466
  O(000F00,E0,_,_,_,_,_,_  ), // #467
  O(000F0F,BF,_,_,_,_,_,_  ), // #468
  O(000F00,E3,_,_,_,_,_,_  ), // #469
  O(660F38,10,_,_,_,_,_,_  ), // #470
  O(660F3A,0E,_,_,_,_,_,_  ), // #471
  O(660F3A,44,_,_,_,_,_,_  ), // #472
  O(000F00,74,_,_,_,_,_,_  ), // #473
  O(000F00,76,_,_,_,_,_,_  ), // #474
  O(660F38,29,_,_,_,_,_,_  ), // #475
  O(000F00,75,_,_,_,_,_,_  ), // #476
  O(660F3A,61,_,_,_,_,_,_  ), // #477
  O(660F3A,60,_,_,_,_,_,_  ), // #478
  O(000F00,64,_,_,_,_,_,_  ), // #479
  O(000F00,66,_,_,_,_,_,_  ), // #480
  O(660F38,37,_,_,_,_,_,_  ), // #481
  O(000F00,65,_,_,_,_,_,_  ), // #482
  O(660F3A,63,_,_,_,_,_,_  ), // #483
  O(660F3A,62,_,_,_,_,_,_  ), // #484
  O(660F00,AE,7,_,_,_,_,_  ), // #485
  V(F20F38,F5,_,0,x,_,_,_  ), // #486
  V(F30F38,F5,_,0,x,_,_,_  ), // #487
  O(000F3A,14,_,_,_,_,_,_  ), // #488
  O(000F3A,16,_,_,_,_,_,_  ), // #489
  O(000F3A,16,_,_,1,_,_,_  ), // #490
  O(000F00,C5,_,_,_,_,_,_  ), // #491
  O(000F0F,1D,_,_,_,_,_,_  ), // #492
  O(000F0F,1C,_,_,_,_,_,_  ), // #493
  O(000F0F,AE,_,_,_,_,_,_  ), // #494
  O(000F0F,9E,_,_,_,_,_,_  ), // #495
  O(000F0F,B0,_,_,_,_,_,_  ), // #496
  O(000F0F,90,_,_,_,_,_,_  ), // #497
  O(000F0F,A0,_,_,_,_,_,_  ), // #498
  O(000F0F,A4,_,_,_,_,_,_  ), // #499
  O(000F0F,94,_,_,_,_,_,_  ), // #500
  O(000F0F,B4,_,_,_,_,_,_  ), // #501
  O(000F0F,8A,_,_,_,_,_,_  ), // #502
  O(000F0F,8E,_,_,_,_,_,_  ), // #503
  O(000F0F,96,_,_,_,_,_,_  ), // #504
  O(000F0F,A6,_,_,_,_,_,_  ), // #505
  O(000F0F,B6,_,_,_,_,_,_  ), // #506
  O(000F0F,86,_,_,_,_,_,_  ), // #507
  O(000F0F,A7,_,_,_,_,_,_  ), // #508
  O(000F0F,97,_,_,_,_,_,_  ), // #509
  O(000F0F,87,_,_,_,_,_,_  ), // #510
  O(000F0F,9A,_,_,_,_,_,_  ), // #511
  O(000F0F,AA,_,_,_,_,_,_  ), // #512
  O(000F38,02,_,_,_,_,_,_  ), // #513
  O(000F38,03,_,_,_,_,_,_  ), // #514
  O(000F38,01,_,_,_,_,_,_  ), // #515
  O(660F38,41,_,_,_,_,_,_  ), // #516
  O(000F38,06,_,_,_,_,_,_  ), // #517
  O(000F38,07,_,_,_,_,_,_  ), // #518
  O(000F38,05,_,_,_,_,_,_  ), // #519
  O(000F0F,0D,_,_,_,_,_,_  ), // #520
  O(000F0F,0C,_,_,_,_,_,_  ), // #521
  O(660F3A,20,_,_,_,_,_,_  ), // #522
  O(660F3A,22,_,_,_,_,_,_  ), // #523
  O(660F3A,22,_,_,1,_,_,_  ), // #524
  O(000F00,C4,_,_,_,_,_,_  ), // #525
  O(000F38,04,_,_,_,_,_,_  ), // #526
  O(000F00,F5,_,_,_,_,_,_  ), // #527
  O(660F38,3C,_,_,_,_,_,_  ), // #528
  O(660F38,3D,_,_,_,_,_,_  ), // #529
  O(000F00,EE,_,_,_,_,_,_  ), // #530
  O(000F00,DE,_,_,_,_,_,_  ), // #531
  O(660F38,3F,_,_,_,_,_,_  ), // #532
  O(660F38,3E,_,_,_,_,_,_  ), // #533
  O(660F38,38,_,_,_,_,_,_  ), // #534
  O(660F38,39,_,_,_,_,_,_  ), // #535
  O(000F00,EA,_,_,_,_,_,_  ), // #536
  O(000F00,DA,_,_,_,_,_,_  ), // #537
  O(660F38,3B,_,_,_,_,_,_  ), // #538
  O(660F38,3A,_,_,_,_,_,_  ), // #539
  O(000F00,D7,_,_,_,_,_,_  ), // #540
  O(660F38,21,_,_,_,_,_,_  ), // #541
  O(660F38,22,_,_,_,_,_,_  ), // #542
  O(660F38,20,_,_,_,_,_,_  ), // #543
  O(660F38,25,_,_,_,_,_,_  ), // #544
  O(660F38,23,_,_,_,_,_,_  ), // #545
  O(660F38,24,_,_,_,_,_,_  ), // #546
  O(660F38,31,_,_,_,_,_,_  ), // #547
  O(660F38,32,_,_,_,_,_,_  ), // #548
  O(660F38,30,_,_,_,_,_,_  ), // #549
  O(660F38,35,_,_,_,_,_,_  ), // #550
  O(660F38,33,_,_,_,_,_,_  ), // #551
  O(660F38,34,_,_,_,_,_,_  ), // #552
  O(660F38,28,_,_,_,_,_,_  ), // #553
  O(000F38,0B,_,_,_,_,_,_  ), // #554
  O(000F0F,B7,_,_,_,_,_,_  ), // #555
  O(000F00,E4,_,_,_,_,_,_  ), // #556
  O(000F00,E5,_,_,_,_,_,_  ), // #557
  O(660F38,40,_,_,_,_,_,_  ), // #558
  O(000F00,D5,_,_,_,_,_,_  ), // #559
  O(000F00,F4,_,_,_,_,_,_  ), // #560
  O(000000,8F,0,_,_,_,_,_  ), // #561
  O(660000,61,_,_,_,_,_,_  ), // #562
  O(000000,61,_,_,_,_,_,_  ), // #563
  O(F30F00,B8,_,_,x,_,_,_  ), // #564
  O(660000,9D,_,_,_,_,_,_  ), // #565
  O(000000,9D,_,_,_,_,_,_  ), // #566
  O(000000,9D,_,_,_,_,_,_  ), // #567
  O(000F00,EB,_,_,_,_,_,_  ), // #568
  O(000F00,0D,0,_,_,_,_,_  ), // #569
  O(000F00,18,0,_,_,_,_,_  ), // #570
  O(000F00,18,1,_,_,_,_,_  ), // #571
  O(000F00,18,2,_,_,_,_,_  ), // #572
  O(000F00,18,3,_,_,_,_,_  ), // #573
  O(000F00,0D,1,_,_,_,_,_  ), // #574
  O(000F00,0D,2,_,_,_,_,_  ), // #575
  O(000F00,F6,_,_,_,_,_,_  ), // #576
  O(000F38,00,_,_,_,_,_,_  ), // #577
  O(660F00,70,_,_,_,_,_,_  ), // #578
  O(F30F00,70,_,_,_,_,_,_  ), // #579
  O(F20F00,70,_,_,_,_,_,_  ), // #580
  O(000F00,70,_,_,_,_,_,_  ), // #581
  O(000F38,08,_,_,_,_,_,_  ), // #582
  O(000F38,0A,_,_,_,_,_,_  ), // #583
  O(000F38,09,_,_,_,_,_,_  ), // #584
  O(000F00,F2,_,_,_,_,_,_  ), // #585
  0                         , // #586
  O(000F00,F3,_,_,_,_,_,_  ), // #587
  O(000F00,F1,_,_,_,_,_,_  ), // #588
  O(000F00,E2,_,_,_,_,_,_  ), // #589
  O(000F00,E1,_,_,_,_,_,_  ), // #590
  O(000F00,D2,_,_,_,_,_,_  ), // #591
  0                         , // #592
  O(000F00,D3,_,_,_,_,_,_  ), // #593
  O(000F00,D1,_,_,_,_,_,_  ), // #594
  O(000F00,F8,_,_,_,_,_,_  ), // #595
  O(000F00,FA,_,_,_,_,_,_  ), // #596
  O(000F00,FB,_,_,_,_,_,_  ), // #597
  O(000F00,E8,_,_,_,_,_,_  ), // #598
  O(000F00,E9,_,_,_,_,_,_  ), // #599
  O(000F00,D8,_,_,_,_,_,_  ), // #600
  O(000F00,D9,_,_,_,_,_,_  ), // #601
  O(000F00,F9,_,_,_,_,_,_  ), // #602
  O(000F0F,BB,_,_,_,_,_,_  ), // #603
  O(660F38,17,_,_,_,_,_,_  ), // #604
  O(000F00,68,_,_,_,_,_,_  ), // #605
  O(000F00,6A,_,_,_,_,_,_  ), // #606
  O(660F00,6D,_,_,_,_,_,_  ), // #607
  O(000F00,69,_,_,_,_,_,_  ), // #608
  O(000F00,60,_,_,_,_,_,_  ), // #609
  O(000F00,62,_,_,_,_,_,_  ), // #610
  O(660F00,6C,_,_,_,_,_,_  ), // #611
  O(000F00,61,_,_,_,_,_,_  ), // #612
  O(000000,FF,6,_,_,_,_,_  ), // #613
  O(660000,60,_,_,_,_,_,_  ), // #614
  O(000000,60,_,_,_,_,_,_  ), // #615
  O(660000,9C,_,_,_,_,_,_  ), // #616
  O(000000,9C,_,_,_,_,_,_  ), // #617
  O(000000,9C,_,_,_,_,_,_  ), // #618
  O(000F00,EF,_,_,_,_,_,_  ), // #619
  O(000000,D0,2,_,x,_,_,_  ), // #620
  O(000F00,53,_,_,_,_,_,_  ), // #621
  O(F30F00,53,_,_,_,_,_,_  ), // #622
  O(000000,D0,3,_,x,_,_,_  ), // #623
  O(F30F00,AE,0,_,x,_,_,_  ), // #624
  O(F30F00,AE,1,_,x,_,_,_  ), // #625
  O(000F00,32,_,_,_,_,_,_  ), // #626
  O(000F00,33,_,_,_,_,_,_  ), // #627
  O(000F00,C7,6,_,x,_,_,_  ), // #628
  O(000F00,C7,7,_,x,_,_,_  ), // #629
  O(000F00,31,_,_,_,_,_,_  ), // #630
  O(000F01,F9,_,_,_,_,_,_  ), // #631
  O(000000,C2,_,_,_,_,_,_  ), // #632
  O(000000,D0,0,_,x,_,_,_  ), // #633
  O(000000,D0,1,_,x,_,_,_  ), // #634
  V(F20F3A,F0,_,0,x,_,_,_  ), // #635
  O(660F3A,09,_,_,_,_,_,_  ), // #636
  O(660F3A,08,_,_,_,_,_,_  ), // #637
  O(660F3A,0B,_,_,_,_,_,_  ), // #638
  O(660F3A,0A,_,_,_,_,_,_  ), // #639
  O(000F00,AA,_,_,_,_,_,_  ), // #640
  O(000F00,52,_,_,_,_,_,_  ), // #641
  O(F30F00,52,_,_,_,_,_,_  ), // #642
  O(000000,9E,_,_,_,_,_,_  ), // #643
  O(000000,D0,4,_,x,_,_,_  ), // #644
  O(000000,D0,7,_,x,_,_,_  ), // #645
  V(F30F38,F7,_,0,x,_,_,_  ), // #646
  O(000000,18,3,_,x,_,_,_  ), // #647
  O(000000,AE,_,_,_,_,_,_  ), // #648
  O(000F00,97,_,_,_,_,_,_  ), // #649
  O(000F00,93,_,_,_,_,_,_  ), // #650
  O(000F00,92,_,_,_,_,_,_  ), // #651
  O(000F00,96,_,_,_,_,_,_  ), // #652
  O(000F00,92,_,_,_,_,_,_  ), // #653
  O(000F00,94,_,_,_,_,_,_  ), // #654
  O(000F00,9F,_,_,_,_,_,_  ), // #655
  O(000F00,9D,_,_,_,_,_,_  ), // #656
  O(000F00,9C,_,_,_,_,_,_  ), // #657
  O(000F00,9E,_,_,_,_,_,_  ), // #658
  O(000F00,96,_,_,_,_,_,_  ), // #659
  O(000F00,92,_,_,_,_,_,_  ), // #660
  O(000F00,93,_,_,_,_,_,_  ), // #661
  O(000F00,97,_,_,_,_,_,_  ), // #662
  O(000F00,93,_,_,_,_,_,_  ), // #663
  O(000F00,95,_,_,_,_,_,_  ), // #664
  O(000F00,9E,_,_,_,_,_,_  ), // #665
  O(000F00,9C,_,_,_,_,_,_  ), // #666
  O(000F00,9D,_,_,_,_,_,_  ), // #667
  O(000F00,9F,_,_,_,_,_,_  ), // #668
  O(000F00,91,_,_,_,_,_,_  ), // #669
  O(000F00,9B,_,_,_,_,_,_  ), // #670
  O(000F00,99,_,_,_,_,_,_  ), // #671
  O(000F00,95,_,_,_,_,_,_  ), // #672
  O(000F00,90,_,_,_,_,_,_  ), // #673
  O(000F00,9A,_,_,_,_,_,_  ), // #674
  O(000F00,9A,_,_,_,_,_,_  ), // #675
  O(000F00,9B,_,_,_,_,_,_  ), // #676
  O(000F00,98,_,_,_,_,_,_  ), // #677
  O(000F00,94,_,_,_,_,_,_  ), // #678
  O(000F00,AE,7,_,_,_,_,_  ), // #679
  O(000F00,01,0,_,_,_,_,_  ), // #680
  O(000F38,C9,_,_,_,_,_,_  ), // #681
  O(000F38,CA,_,_,_,_,_,_  ), // #682
  O(000F38,C8,_,_,_,_,_,_  ), // #683
  O(000F3A,CC,_,_,_,_,_,_  ), // #684
  O(000F38,CC,_,_,_,_,_,_  ), // #685
  O(000F38,CD,_,_,_,_,_,_  ), // #686
  O(000F38,CB,_,_,_,_,_,_  ), // #687
  O(000000,D0,4,_,x,_,_,_  ), // #688
  O(000F00,A4,_,_,x,_,_,_  ), // #689
  V(660F38,F7,_,0,x,_,_,_  ), // #690
  O(000000,D0,5,_,x,_,_,_  ), // #691
  O(000F00,AC,_,_,x,_,_,_  ), // #692
  V(F20F38,F7,_,0,x,_,_,_  ), // #693
  O(660F00,C6,_,_,_,_,_,_  ), // #694
  O(000F00,C6,_,_,_,_,_,_  ), // #695
  O(000F00,01,1,_,_,_,_,_  ), // #696
  O(000F00,00,0,_,_,_,_,_  ), // #697
  O(000F00,01,4,_,_,_,_,_  ), // #698
  O(660F00,51,_,_,_,_,_,_  ), // #699
  O(000F00,51,_,_,_,_,_,_  ), // #700
  O(F20F00,51,_,_,_,_,_,_  ), // #701
  O(F30F00,51,_,_,_,_,_,_  ), // #702
  O(000F01,CB,_,_,_,_,_,_  ), // #703
  O(000000,F9,_,_,_,_,_,_  ), // #704
  O(000000,FD,_,_,_,_,_,_  ), // #705
  O(000000,FB,_,_,_,_,_,_  ), // #706
  O(000F00,AE,3,_,_,_,_,_  ), // #707
  O(000000,AA,_,_,_,_,_,_  ), // #708
  O(000F00,00,1,_,_,_,_,_  ), // #709
  O(000000,28,5,_,x,_,_,_  ), // #710
  O(660F00,5C,_,_,_,_,_,_  ), // #711
  O(000F00,5C,_,_,_,_,_,_  ), // #712
  O(F20F00,5C,_,_,_,_,_,_  ), // #713
  O(F30F00,5C,_,_,_,_,_,_  ), // #714
  O(000F01,F8,_,_,_,_,_,_  ), // #715
  O(000F00,05,_,_,_,_,_,_  ), // #716
  O(000F00,34,_,_,_,_,_,_  ), // #717
  O(000F00,35,_,_,_,_,_,_  ), // #718
  O(000F00,35,_,_,_,_,_,_  ), // #719
  O(000F00,07,_,_,_,_,_,_  ), // #720
  O(000F00,07,_,_,_,_,_,_  ), // #721
  V(XOP_M9,01,7,0,x,_,_,_  ), // #722
  O(000000,84,_,_,x,_,_,_  ), // #723
  O(F30F00,BC,_,_,x,_,_,_  ), // #724
  V(XOP_M9,01,4,0,x,_,_,_  ), // #725
  O(660F00,2E,_,_,_,_,_,_  ), // #726
  O(000F00,2E,_,_,_,_,_,_  ), // #727
  O(000F00,0B,_,_,_,_,_,_  ), // #728
  O(660F00,15,_,_,_,_,_,_  ), // #729
  O(000F00,15,_,_,_,_,_,_  ), // #730
  O(660F00,14,_,_,_,_,_,_  ), // #731
  O(000F00,14,_,_,_,_,_,_  ), // #732
  V(F20F38,9A,_,2,_,0,2,T4X), // #733
  V(F20F38,9B,_,2,_,0,2,T4X), // #734
  V(F20F38,AA,_,2,_,0,2,T4X), // #735
  V(F20F38,AB,_,2,_,0,2,T4X), // #736
  V(660F00,58,_,x,I,1,4,FV ), // #737
  V(000F00,58,_,x,I,0,4,FV ), // #738
  V(F20F00,58,_,I,I,1,3,T1S), // #739
  V(F30F00,58,_,I,I,0,2,T1S), // #740
  V(660F00,D0,_,x,I,_,_,_  ), // #741
  V(F20F00,D0,_,x,I,_,_,_  ), // #742
  V(660F38,DE,_,x,I,_,4,FVM), // #743
  V(660F38,DF,_,x,I,_,4,FVM), // #744
  V(660F38,DC,_,x,I,_,4,FVM), // #745
  V(660F38,DD,_,x,I,_,4,FVM), // #746
  V(660F38,DB,_,0,I,_,_,_  ), // #747
  V(660F3A,DF,_,0,I,_,_,_  ), // #748
  V(660F3A,03,_,x,_,0,4,FV ), // #749
  V(660F3A,03,_,x,_,1,4,FV ), // #750
  V(660F00,55,_,x,I,1,4,FV ), // #751
  V(000F00,55,_,x,I,0,4,FV ), // #752
  V(660F00,54,_,x,I,1,4,FV ), // #753
  V(000F00,54,_,x,I,0,4,FV ), // #754
  V(660F38,66,_,x,_,0,4,FVM), // #755
  V(660F38,64,_,x,_,0,4,FV ), // #756
  V(660F38,65,_,x,_,1,4,FV ), // #757
  V(660F38,65,_,x,_,0,4,FV ), // #758
  V(660F38,64,_,x,_,1,4,FV ), // #759
  V(660F38,66,_,x,_,1,4,FVM), // #760
  V(660F3A,0D,_,x,I,_,_,_  ), // #761
  V(660F3A,0C,_,x,I,_,_,_  ), // #762
  V(660F3A,4B,_,x,0,_,_,_  ), // #763
  V(660F3A,4A,_,x,0,_,_,_  ), // #764
  V(660F38,1A,_,1,0,_,_,_  ), // #765
  V(660F38,19,_,x,_,0,3,T2 ), // #766
  V(660F38,1A,_,x,_,0,4,T4 ), // #767
  V(660F38,1B,_,2,_,0,5,T8 ), // #768
  V(660F38,1A,_,x,_,1,4,T2 ), // #769
  V(660F38,1B,_,2,_,1,5,T4 ), // #770
  V(660F38,5A,_,1,0,_,_,_  ), // #771
  V(660F38,59,_,x,_,0,3,T2 ), // #772
  V(660F38,5A,_,x,_,0,4,T4 ), // #773
  V(660F38,5B,_,2,_,0,5,T8 ), // #774
  V(660F38,5A,_,x,_,1,4,T2 ), // #775
  V(660F38,5B,_,2,_,1,5,T4 ), // #776
  V(660F38,19,_,x,0,1,3,T1S), // #777
  V(660F38,18,_,x,0,0,2,T1S), // #778
  V(660F00,C2,_,x,I,1,4,FV ), // #779
  V(000F00,C2,_,x,I,0,4,FV ), // #780
  V(F20F00,C2,_,I,I,1,3,T1S), // #781
  V(F30F00,C2,_,I,I,0,2,T1S), // #782
  V(660F00,2F,_,I,I,1,3,T1S), // #783
  V(000F00,2F,_,I,I,0,2,T1S), // #784
  V(660F38,8A,_,x,_,1,3,T1S), // #785
  V(660F38,8A,_,x,_,0,2,T1S), // #786
  V(F30F00,E6,_,x,I,0,3,HV ), // #787
  V(000F00,5B,_,x,I,0,4,FV ), // #788
  V(F20F00,E6,_,x,I,1,4,FV ), // #789
  V(660F00,5A,_,x,I,1,4,FV ), // #790
  V(660F00,7B,_,x,_,1,4,FV ), // #791
  V(000F00,79,_,x,_,1,4,FV ), // #792
  V(660F00,79,_,x,_,1,4,FV ), // #793
  V(660F38,13,_,x,0,0,3,HVM), // #794
  V(660F00,5B,_,x,I,0,4,FV ), // #795
  V(000F00,5A,_,x,I,0,4,HV ), // #796
  V(660F3A,1D,_,x,0,0,3,HVM), // #797
  V(660F00,7B,_,x,_,0,3,HV ), // #798
  V(000F00,79,_,x,_,0,4,FV ), // #799
  V(660F00,79,_,x,_,0,3,HV ), // #800
  V(F30F00,E6,_,x,_,1,4,FV ), // #801
  V(000F00,5B,_,x,_,1,4,FV ), // #802
  V(F20F00,2D,_,I,x,x,3,T1F), // #803
  V(F20F00,5A,_,I,I,1,3,T1S), // #804
  V(F20F00,79,_,I,_,x,3,T1F), // #805
  V(F20F00,2A,_,I,x,x,2,T1W), // #806
  V(F30F00,2A,_,I,x,x,2,T1W), // #807
  V(F30F00,5A,_,I,I,0,2,T1S), // #808
  V(F30F00,2D,_,I,x,x,2,T1F), // #809
  V(F30F00,79,_,I,_,x,2,T1F), // #810
  V(660F00,E6,_,x,I,1,4,FV ), // #811
  V(660F00,7A,_,x,_,1,4,FV ), // #812
  V(000F00,78,_,x,_,1,4,FV ), // #813
  V(660F00,78,_,x,_,1,4,FV ), // #814
  V(F30F00,5B,_,x,I,0,4,FV ), // #815
  V(660F00,7A,_,x,_,0,3,HV ), // #816
  V(000F00,78,_,x,_,0,4,FV ), // #817
  V(660F00,78,_,x,_,0,3,HV ), // #818
  V(F20F00,2C,_,I,x,x,3,T1F), // #819
  V(F20F00,78,_,I,_,x,3,T1F), // #820
  V(F30F00,2C,_,I,x,x,2,T1F), // #821
  V(F30F00,78,_,I,_,x,2,T1F), // #822
  V(F30F00,7A,_,x,_,0,3,HV ), // #823
  V(F20F00,7A,_,x,_,0,4,FV ), // #824
  V(F30F00,7A,_,x,_,1,4,FV ), // #825
  V(F20F00,7A,_,x,_,1,4,FV ), // #826
  V(F20F00,7B,_,I,_,x,2,T1W), // #827
  V(F30F00,7B,_,I,_,x,2,T1W), // #828
  V(660F3A,42,_,x,_,0,4,FVM), // #829
  V(660F00,5E,_,x,I,1,4,FV ), // #830
  V(000F00,5E,_,x,I,0,4,FV ), // #831
  V(F20F00,5E,_,I,I,1,3,T1S), // #832
  V(F30F00,5E,_,I,I,0,2,T1S), // #833
  V(660F3A,41,_,x,I,_,_,_  ), // #834
  V(660F3A,40,_,x,I,_,_,_  ), // #835
  O(000F00,00,4,_,_,_,_,_  ), // #836
  O(000F00,00,5,_,_,_,_,_  ), // #837
  V(660F38,C8,_,2,_,1,4,FV ), // #838
  V(660F38,C8,_,2,_,0,4,FV ), // #839
  V(660F38,88,_,x,_,1,3,T1S), // #840
  V(660F38,88,_,x,_,0,2,T1S), // #841
  V(660F3A,19,_,1,0,_,_,_  ), // #842
  V(660F3A,19,_,x,_,0,4,T4 ), // #843
  V(660F3A,1B,_,2,_,0,5,T8 ), // #844
  V(660F3A,19,_,x,_,1,4,T2 ), // #845
  V(660F3A,1B,_,2,_,1,5,T4 ), // #846
  V(660F3A,39,_,1,0,_,_,_  ), // #847
  V(660F3A,39,_,x,_,0,4,T4 ), // #848
  V(660F3A,3B,_,2,_,0,5,T8 ), // #849
  V(660F3A,39,_,x,_,1,4,T2 ), // #850
  V(660F3A,3B,_,2,_,1,5,T4 ), // #851
  V(660F3A,17,_,0,I,I,2,T1S), // #852
  V(660F3A,54,_,x,_,1,4,FV ), // #853
  V(660F3A,54,_,x,_,0,4,FV ), // #854
  V(660F3A,55,_,I,_,1,3,T1S), // #855
  V(660F3A,55,_,I,_,0,2,T1S), // #856
  V(660F38,98,_,x,1,1,4,FV ), // #857
  V(660F38,98,_,x,0,0,4,FV ), // #858
  V(660F38,99,_,I,1,1,3,T1S), // #859
  V(660F38,99,_,I,0,0,2,T1S), // #860
  V(660F38,A8,_,x,1,1,4,FV ), // #861
  V(660F38,A8,_,x,0,0,4,FV ), // #862
  V(660F38,A9,_,I,1,1,3,T1S), // #863
  V(660F38,A9,_,I,0,0,2,T1S), // #864
  V(660F38,B8,_,x,1,1,4,FV ), // #865
  V(660F38,B8,_,x,0,0,4,FV ), // #866
  V(660F38,B9,_,I,1,1,3,T1S), // #867
  V(660F38,B9,_,I,0,0,2,T1S), // #868
  V(660F3A,69,_,x,x,_,_,_  ), // #869
  V(660F3A,68,_,x,x,_,_,_  ), // #870
  V(660F3A,6B,_,0,x,_,_,_  ), // #871
  V(660F3A,6A,_,0,x,_,_,_  ), // #872
  V(660F38,96,_,x,1,1,4,FV ), // #873
  V(660F38,96,_,x,0,0,4,FV ), // #874
  V(660F38,A6,_,x,1,1,4,FV ), // #875
  V(660F38,A6,_,x,0,0,4,FV ), // #876
  V(660F38,B6,_,x,1,1,4,FV ), // #877
  V(660F38,B6,_,x,0,0,4,FV ), // #878
  V(660F3A,5D,_,x,x,_,_,_  ), // #879
  V(660F3A,5C,_,x,x,_,_,_  ), // #880
  V(660F38,9A,_,x,1,1,4,FV ), // #881
  V(660F38,9A,_,x,0,0,4,FV ), // #882
  V(660F38,9B,_,I,1,1,3,T1S), // #883
  V(660F38,9B,_,I,0,0,2,T1S), // #884
  V(660F38,AA,_,x,1,1,4,FV ), // #885
  V(660F38,AA,_,x,0,0,4,FV ), // #886
  V(660F38,AB,_,I,1,1,3,T1S), // #887
  V(660F38,AB,_,I,0,0,2,T1S), // #888
  V(660F38,BA,_,x,1,1,4,FV ), // #889
  V(660F38,BA,_,x,0,0,4,FV ), // #890
  V(660F38,BB,_,I,1,1,3,T1S), // #891
  V(660F38,BB,_,I,0,0,2,T1S), // #892
  V(660F38,97,_,x,1,1,4,FV ), // #893
  V(660F38,97,_,x,0,0,4,FV ), // #894
  V(660F38,A7,_,x,1,1,4,FV ), // #895
  V(660F38,A7,_,x,0,0,4,FV ), // #896
  V(660F38,B7,_,x,1,1,4,FV ), // #897
  V(660F38,B7,_,x,0,0,4,FV ), // #898
  V(660F3A,5F,_,x,x,_,_,_  ), // #899
  V(660F3A,5E,_,x,x,_,_,_  ), // #900
  V(660F3A,6D,_,x,x,_,_,_  ), // #901
  V(660F3A,6C,_,x,x,_,_,_  ), // #902
  V(660F3A,6F,_,0,x,_,_,_  ), // #903
  V(660F3A,6E,_,0,x,_,_,_  ), // #904
  V(660F38,9C,_,x,1,1,4,FV ), // #905
  V(660F38,9C,_,x,0,0,4,FV ), // #906
  V(660F38,9D,_,I,1,1,3,T1S), // #907
  V(660F38,9D,_,I,0,0,2,T1S), // #908
  V(660F38,AC,_,x,1,1,4,FV ), // #909
  V(660F38,AC,_,x,0,0,4,FV ), // #910
  V(660F38,AD,_,I,1,1,3,T1S), // #911
  V(660F38,AD,_,I,0,0,2,T1S), // #912
  V(660F38,BC,_,x,1,1,4,FV ), // #913
  V(660F38,BC,_,x,0,0,4,FV ), // #914
  V(660F38,BC,_,I,1,1,3,T1S), // #915
  V(660F38,BC,_,I,0,0,2,T1S), // #916
  V(660F3A,79,_,x,x,_,_,_  ), // #917
  V(660F3A,78,_,x,x,_,_,_  ), // #918
  V(660F3A,7B,_,0,x,_,_,_  ), // #919
  V(660F3A,7A,_,0,x,_,_,_  ), // #920
  V(660F38,9E,_,x,1,1,4,FV ), // #921
  V(660F38,9E,_,x,0,0,4,FV ), // #922
  V(660F38,9F,_,I,1,1,3,T1S), // #923
  V(660F38,9F,_,I,0,0,2,T1S), // #924
  V(660F38,AE,_,x,1,1,4,FV ), // #925
  V(660F38,AE,_,x,0,0,4,FV ), // #926
  V(660F38,AF,_,I,1,1,3,T1S), // #927
  V(660F38,AF,_,I,0,0,2,T1S), // #928
  V(660F38,BE,_,x,1,1,4,FV ), // #929
  V(660F38,BE,_,x,0,0,4,FV ), // #930
  V(660F38,BF,_,I,1,1,3,T1S), // #931
  V(660F38,BF,_,I,0,0,2,T1S), // #932
  V(660F3A,7D,_,x,x,_,_,_  ), // #933
  V(660F3A,7C,_,x,x,_,_,_  ), // #934
  V(660F3A,7F,_,0,x,_,_,_  ), // #935
  V(660F3A,7E,_,0,x,_,_,_  ), // #936
  V(660F3A,66,_,x,_,1,4,FV ), // #937
  V(660F3A,66,_,x,_,0,4,FV ), // #938
  V(660F3A,67,_,I,_,1,3,T1S), // #939
  V(660F3A,67,_,I,_,0,2,T1S), // #940
  V(XOP_M9,81,_,x,0,_,_,_  ), // #941
  V(XOP_M9,80,_,x,0,_,_,_  ), // #942
  V(XOP_M9,83,_,0,0,_,_,_  ), // #943
  V(XOP_M9,82,_,0,0,_,_,_  ), // #944
  V(660F38,92,_,x,1,_,_,_  ), // #945
  V(660F38,92,_,x,0,_,_,_  ), // #946
  V(660F38,C6,1,2,_,1,3,T1S), // #947
  V(660F38,C6,1,2,_,0,2,T1S), // #948
  V(660F38,C7,1,2,_,1,3,T1S), // #949
  V(660F38,C7,1,2,_,0,2,T1S), // #950
  V(660F38,C6,2,2,_,1,3,T1S), // #951
  V(660F38,C6,2,2,_,0,2,T1S), // #952
  V(660F38,C7,2,2,_,1,3,T1S), // #953
  V(660F38,C7,2,2,_,0,2,T1S), // #954
  V(660F38,93,_,x,1,_,_,_  ), // #955
  V(660F38,93,_,x,0,_,_,_  ), // #956
  V(660F38,42,_,x,_,1,4,FV ), // #957
  V(660F38,42,_,x,_,0,4,FV ), // #958
  V(660F38,43,_,I,_,1,3,T1S), // #959
  V(660F38,43,_,I,_,0,2,T1S), // #960
  V(660F3A,26,_,x,_,1,4,FV ), // #961
  V(660F3A,26,_,x,_,0,4,FV ), // #962
  V(660F3A,27,_,I,_,1,3,T1S), // #963
  V(660F3A,27,_,I,_,0,2,T1S), // #964
  V(660F00,7C,_,x,I,_,_,_  ), // #965
  V(F20F00,7C,_,x,I,_,_,_  ), // #966
  V(660F00,7D,_,x,I,_,_,_  ), // #967
  V(F20F00,7D,_,x,I,_,_,_  ), // #968
  V(660F3A,18,_,1,0,_,_,_  ), // #969
  V(660F3A,18,_,x,_,0,4,T4 ), // #970
  V(660F3A,1A,_,2,_,0,5,T8 ), // #971
  V(660F3A,18,_,x,_,1,4,T2 ), // #972
  V(660F3A,1A,_,2,_,1,5,T4 ), // #973
  V(660F3A,38,_,1,0,_,_,_  ), // #974
  V(660F3A,38,_,x,_,0,4,T4 ), // #975
  V(660F3A,3A,_,2,_,0,5,T8 ), // #976
  V(660F3A,38,_,x,_,1,4,T2 ), // #977
  V(660F3A,3A,_,2,_,1,5,T4 ), // #978
  V(660F3A,21,_,0,I,0,2,T1S), // #979
  V(F20F00,F0,_,x,I,_,_,_  ), // #980
  V(000F00,AE,2,0,I,_,_,_  ), // #981
  V(660F00,F7,_,0,I,_,_,_  ), // #982
  V(660F38,2D,_,x,0,_,_,_  ), // #983
  V(660F38,2C,_,x,0,_,_,_  ), // #984
  V(660F00,5F,_,x,I,1,4,FV ), // #985
  V(000F00,5F,_,x,I,0,4,FV ), // #986
  V(F20F00,5F,_,I,I,1,3,T1S), // #987
  V(F30F00,5F,_,I,I,0,2,T1S), // #988
  V(660F00,5D,_,x,I,1,4,FV ), // #989
  V(000F00,5D,_,x,I,0,4,FV ), // #990
  V(F20F00,5D,_,I,I,1,3,T1S), // #991
  V(F30F00,5D,_,I,I,0,2,T1S), // #992
  V(660F00,28,_,x,I,1,4,FVM), // #993
  V(000F00,28,_,x,I,0,4,FVM), // #994
  V(660F00,6E,_,0,0,0,2,T1S), // #995
  V(F20F00,12,_,x,I,1,3,DUP), // #996
  V(660F00,6F,_,x,I,_,_,_  ), // #997
  V(660F00,6F,_,x,_,0,4,FVM), // #998
  V(660F00,6F,_,x,_,1,4,FVM), // #999
  V(F30F00,6F,_,x,I,_,_,_  ), // #1000
  V(F20F00,6F,_,x,_,1,4,FVM), // #1001
  V(F30F00,6F,_,x,_,0,4,FVM), // #1002
  V(F30F00,6F,_,x,_,1,4,FVM), // #1003
  V(F20F00,6F,_,x,_,0,4,FVM), // #1004
  V(000F00,12,_,0,I,0,_,_  ), // #1005
  V(660F00,16,_,0,I,1,3,T1S), // #1006
  V(000F00,16,_,0,I,0,3,T2 ), // #1007
  V(000F00,16,_,0,I,0,_,_  ), // #1008
  V(660F00,12,_,0,I,1,3,T1S), // #1009
  V(000F00,12,_,0,I,0,3,T2 ), // #1010
  V(660F00,50,_,x,I,_,_,_  ), // #1011
  V(000F00,50,_,x,I,_,_,_  ), // #1012
  V(660F00,E7,_,x,I,0,4,FVM), // #1013
  V(660F38,2A,_,x,I,0,4,FVM), // #1014
  V(660F00,2B,_,x,I,1,4,FVM), // #1015
  V(000F00,2B,_,x,I,0,4,FVM), // #1016
  V(660F00,6E,_,0,I,1,3,T1S), // #1017
  V(F20F00,10,_,I,I,1,3,T1S), // #1018
  V(F30F00,16,_,x,I,0,4,FVM), // #1019
  V(F30F00,12,_,x,I,0,4,FVM), // #1020
  V(F30F00,10,_,I,I,0,2,T1S), // #1021
  V(660F00,10,_,x,I,1,4,FVM), // #1022
  V(000F00,10,_,x,I,0,4,FVM), // #1023
  V(660F3A,42,_,x,I,_,_,_  ), // #1024
  V(660F00,59,_,x,I,1,4,FV ), // #1025
  V(000F00,59,_,x,I,0,4,FV ), // #1026
  V(F20F00,59,_,I,I,1,3,T1S), // #1027
  V(F30F00,59,_,I,I,0,2,T1S), // #1028
  V(660F00,56,_,x,I,1,4,FV ), // #1029
  V(000F00,56,_,x,I,0,4,FV ), // #1030
  V(F20F38,52,_,2,_,0,2,T4X), // #1031
  V(F20F38,53,_,2,_,0,2,T4X), // #1032
  V(660F38,1C,_,x,I,_,4,FVM), // #1033
  V(660F38,1E,_,x,I,0,4,FV ), // #1034
  V(660F38,1F,_,x,_,1,4,FV ), // #1035
  V(660F38,1D,_,x,I,_,4,FVM), // #1036
  V(660F00,6B,_,x,I,0,4,FV ), // #1037
  V(660F00,63,_,x,I,I,4,FVM), // #1038
  V(660F38,2B,_,x,I,0,4,FV ), // #1039
  V(660F00,67,_,x,I,I,4,FVM), // #1040
  V(660F00,FC,_,x,I,I,4,FVM), // #1041
  V(660F00,FE,_,x,I,0,4,FV ), // #1042
  V(660F00,D4,_,x,I,1,4,FV ), // #1043
  V(660F00,EC,_,x,I,I,4,FVM), // #1044
  V(660F00,ED,_,x,I,I,4,FVM), // #1045
  V(660F00,DC,_,x,I,I,4,FVM), // #1046
  V(660F00,DD,_,x,I,I,4,FVM), // #1047
  V(660F00,FD,_,x,I,I,4,FVM), // #1048
  V(660F3A,0F,_,x,I,I,4,FVM), // #1049
  V(660F00,DB,_,x,I,_,_,_  ), // #1050
  V(660F00,DB,_,x,_,0,4,FV ), // #1051
  V(660F00,DF,_,x,I,_,_,_  ), // #1052
  V(660F00,DF,_,x,_,0,4,FV ), // #1053
  V(660F00,DF,_,x,_,1,4,FV ), // #1054
  V(660F00,DB,_,x,_,1,4,FV ), // #1055
  V(660F00,E0,_,x,I,I,4,FVM), // #1056
  V(660F00,E3,_,x,I,I,4,FVM), // #1057
  V(660F3A,02,_,x,0,_,_,_  ), // #1058
  V(660F3A,4C,_,x,0,_,_,_  ), // #1059
  V(660F3A,0E,_,x,I,_,_,_  ), // #1060
  V(660F38,78,_,x,0,0,0,T1S), // #1061
  V(660F38,58,_,x,0,0,2,T1S), // #1062
  V(F30F38,3A,_,x,_,0,_,_  ), // #1063
  V(F30F38,2A,_,x,_,1,_,_  ), // #1064
  V(660F38,59,_,x,0,1,3,T1S), // #1065
  V(660F38,79,_,x,0,0,1,T1S), // #1066
  V(660F3A,44,_,x,I,_,4,FVM), // #1067
  V(XOP_M8,A2,_,x,x,_,_,_  ), // #1068
  V(660F3A,3F,_,x,_,0,4,FVM), // #1069
  V(660F3A,1F,_,x,_,0,4,FV ), // #1070
  V(660F00,74,_,x,I,I,4,FV ), // #1071
  V(660F00,76,_,x,I,0,4,FVM), // #1072
  V(660F38,29,_,x,I,1,4,FVM), // #1073
  V(660F00,75,_,x,I,I,4,FV ), // #1074
  V(660F3A,61,_,0,I,_,_,_  ), // #1075
  V(660F3A,60,_,0,I,_,_,_  ), // #1076
  V(660F00,64,_,x,I,I,4,FV ), // #1077
  V(660F00,66,_,x,I,0,4,FVM), // #1078
  V(660F38,37,_,x,I,1,4,FVM), // #1079
  V(660F00,65,_,x,I,I,4,FV ), // #1080
  V(660F3A,63,_,0,I,_,_,_  ), // #1081
  V(660F3A,62,_,0,I,_,_,_  ), // #1082
  V(660F3A,1F,_,x,_,1,4,FV ), // #1083
  V(660F3A,3E,_,x,_,0,4,FVM), // #1084
  V(660F3A,1E,_,x,_,0,4,FV ), // #1085
  V(660F3A,1E,_,x,_,1,4,FV ), // #1086
  V(660F3A,3E,_,x,_,1,4,FVM), // #1087
  V(660F3A,3F,_,x,_,1,4,FVM), // #1088
  V(XOP_M8,CC,_,0,0,_,_,_  ), // #1089
  V(XOP_M8,CE,_,0,0,_,_,_  ), // #1090
  V(660F38,63,_,x,_,0,0,T1S), // #1091
  V(660F38,8B,_,x,_,0,2,T1S), // #1092
  V(660F38,8B,_,x,_,1,3,T1S), // #1093
  V(660F38,63,_,x,_,1,1,T1S), // #1094
  V(XOP_M8,CF,_,0,0,_,_,_  ), // #1095
  V(XOP_M8,EC,_,0,0,_,_,_  ), // #1096
  V(XOP_M8,EE,_,0,0,_,_,_  ), // #1097
  V(XOP_M8,EF,_,0,0,_,_,_  ), // #1098
  V(XOP_M8,ED,_,0,0,_,_,_  ), // #1099
  V(XOP_M8,CD,_,0,0,_,_,_  ), // #1100
  V(660F38,C4,_,x,_,0,4,FV ), // #1101
  V(660F38,C4,_,x,_,1,4,FV ), // #1102
  V(660F3A,06,_,1,0,_,_,_  ), // #1103
  V(660F3A,46,_,1,0,_,_,_  ), // #1104
  V(660F38,8D,_,x,_,0,4,FVM), // #1105
  V(660F38,36,_,x,0,0,4,FV ), // #1106
  V(660F38,75,_,x,_,0,4,FVM), // #1107
  V(660F38,76,_,x,_,0,4,FV ), // #1108
  V(660F38,77,_,x,_,1,4,FV ), // #1109
  V(660F38,77,_,x,_,0,4,FV ), // #1110
  V(660F38,76,_,x,_,1,4,FV ), // #1111
  V(660F38,75,_,x,_,1,4,FVM), // #1112
  V(660F3A,49,_,x,x,_,_,_  ), // #1113
  V(660F3A,48,_,x,x,_,_,_  ), // #1114
  V(660F38,0D,_,x,0,1,4,FV ), // #1115
  V(660F38,0C,_,x,0,0,4,FV ), // #1116
  V(660F3A,01,_,1,1,_,_,_  ), // #1117
  V(660F38,16,_,1,0,_,_,_  ), // #1118
  V(660F38,36,_,x,_,1,4,FV ), // #1119
  V(660F38,7D,_,x,_,0,4,FVM), // #1120
  V(660F38,7E,_,x,_,0,4,FV ), // #1121
  V(660F38,7F,_,x,_,1,4,FV ), // #1122
  V(660F38,7F,_,x,_,0,4,FV ), // #1123
  V(660F38,7E,_,x,_,1,4,FV ), // #1124
  V(660F38,7D,_,x,_,1,4,FVM), // #1125
  V(660F38,8D,_,x,_,1,4,FVM), // #1126
  V(660F38,62,_,x,_,0,0,T1S), // #1127
  V(660F38,89,_,x,_,0,2,T1S), // #1128
  V(660F38,89,_,x,_,1,3,T1S), // #1129
  V(660F38,62,_,x,_,1,1,T1S), // #1130
  V(660F3A,14,_,0,0,I,0,T1S), // #1131
  V(660F3A,16,_,0,0,0,2,T1S), // #1132
  V(660F3A,16,_,0,1,1,3,T1S), // #1133
  V(660F3A,15,_,0,0,I,1,T1S), // #1134
  V(660F38,90,_,x,0,_,_,_  ), // #1135
  V(660F38,90,_,x,1,_,_,_  ), // #1136
  V(660F38,91,_,x,0,_,_,_  ), // #1137
  V(660F38,91,_,x,1,_,_,_  ), // #1138
  V(XOP_M9,C2,_,0,0,_,_,_  ), // #1139
  V(XOP_M9,C3,_,0,0,_,_,_  ), // #1140
  V(XOP_M9,C1,_,0,0,_,_,_  ), // #1141
  V(660F38,02,_,x,I,_,_,_  ), // #1142
  V(XOP_M9,CB,_,0,0,_,_,_  ), // #1143
  V(660F38,03,_,x,I,_,_,_  ), // #1144
  V(XOP_M9,D2,_,0,0,_,_,_  ), // #1145
  V(XOP_M9,D3,_,0,0,_,_,_  ), // #1146
  V(XOP_M9,D1,_,0,0,_,_,_  ), // #1147
  V(XOP_M9,DB,_,0,0,_,_,_  ), // #1148
  V(XOP_M9,D6,_,0,0,_,_,_  ), // #1149
  V(XOP_M9,D7,_,0,0,_,_,_  ), // #1150
  V(660F38,01,_,x,I,_,_,_  ), // #1151
  V(XOP_M9,C6,_,0,0,_,_,_  ), // #1152
  V(XOP_M9,C7,_,0,0,_,_,_  ), // #1153
  V(660F38,41,_,0,I,_,_,_  ), // #1154
  V(XOP_M9,E1,_,0,0,_,_,_  ), // #1155
  V(660F38,06,_,x,I,_,_,_  ), // #1156
  V(XOP_M9,E3,_,0,0,_,_,_  ), // #1157
  V(660F38,07,_,x,I,_,_,_  ), // #1158
  V(660F38,05,_,x,I,_,_,_  ), // #1159
  V(XOP_M9,E2,_,0,0,_,_,_  ), // #1160
  V(660F3A,20,_,0,0,I,0,T1S), // #1161
  V(660F3A,22,_,0,0,0,2,T1S), // #1162
  V(660F3A,22,_,0,1,1,3,T1S), // #1163
  V(660F00,C4,_,0,0,I,1,T1S), // #1164
  V(660F38,44,_,x,_,0,4,FV ), // #1165
  V(660F38,44,_,x,_,1,4,FV ), // #1166
  V(XOP_M8,9E,_,0,0,_,_,_  ), // #1167
  V(XOP_M8,9F,_,0,0,_,_,_  ), // #1168
  V(XOP_M8,97,_,0,0,_,_,_  ), // #1169
  V(XOP_M8,8E,_,0,0,_,_,_  ), // #1170
  V(XOP_M8,8F,_,0,0,_,_,_  ), // #1171
  V(XOP_M8,87,_,0,0,_,_,_  ), // #1172
  V(XOP_M8,86,_,0,0,_,_,_  ), // #1173
  V(XOP_M8,85,_,0,0,_,_,_  ), // #1174
  V(XOP_M8,96,_,0,0,_,_,_  ), // #1175
  V(XOP_M8,95,_,0,0,_,_,_  ), // #1176
  V(XOP_M8,A6,_,0,0,_,_,_  ), // #1177
  V(XOP_M8,B6,_,0,0,_,_,_  ), // #1178
  V(660F38,B5,_,x,_,1,4,FV ), // #1179
  V(660F38,B4,_,x,_,1,4,FV ), // #1180
  V(660F38,04,_,x,I,I,4,FVM), // #1181
  V(660F00,F5,_,x,I,I,4,FVM), // #1182
  V(660F38,8C,_,x,0,_,_,_  ), // #1183
  V(660F38,8C,_,x,1,_,_,_  ), // #1184
  V(660F38,3C,_,x,I,I,4,FVM), // #1185
  V(660F38,3D,_,x,I,0,4,FV ), // #1186
  V(660F38,3D,_,x,_,1,4,FV ), // #1187
  V(660F00,EE,_,x,I,I,4,FVM), // #1188
  V(660F00,DE,_,x,I,I,4,FVM), // #1189
  V(660F38,3F,_,x,I,0,4,FV ), // #1190
  V(660F38,3F,_,x,_,1,4,FV ), // #1191
  V(660F38,3E,_,x,I,I,4,FVM), // #1192
  V(660F38,38,_,x,I,I,4,FVM), // #1193
  V(660F38,39,_,x,I,0,4,FV ), // #1194
  V(660F38,39,_,x,_,1,4,FV ), // #1195
  V(660F00,EA,_,x,I,I,4,FVM), // #1196
  V(660F00,DA,_,x,I,_,4,FVM), // #1197
  V(660F38,3B,_,x,I,0,4,FV ), // #1198
  V(660F38,3B,_,x,_,1,4,FV ), // #1199
  V(660F38,3A,_,x,I,_,4,FVM), // #1200
  V(F30F38,29,_,x,_,0,_,_  ), // #1201
  V(F30F38,39,_,x,_,0,_,_  ), // #1202
  V(F30F38,31,_,x,_,0,2,QVM), // #1203
  V(F30F38,33,_,x,_,0,3,HVM), // #1204
  V(F30F38,28,_,x,_,0,_,_  ), // #1205
  V(F30F38,38,_,x,_,0,_,_  ), // #1206
  V(F30F38,38,_,x,_,1,_,_  ), // #1207
  V(F30F38,28,_,x,_,1,_,_  ), // #1208
  V(660F00,D7,_,x,I,_,_,_  ), // #1209
  V(F30F38,39,_,x,_,1,_,_  ), // #1210
  V(F30F38,32,_,x,_,0,1,OVM), // #1211
  V(F30F38,35,_,x,_,0,3,HVM), // #1212
  V(F30F38,34,_,x,_,0,2,QVM), // #1213
  V(F30F38,21,_,x,_,0,2,QVM), // #1214
  V(F30F38,23,_,x,_,0,3,HVM), // #1215
  V(F30F38,22,_,x,_,0,1,OVM), // #1216
  V(F30F38,25,_,x,_,0,3,HVM), // #1217
  V(F30F38,24,_,x,_,0,2,QVM), // #1218
  V(F30F38,20,_,x,_,0,3,HVM), // #1219
  V(660F38,21,_,x,I,I,2,QVM), // #1220
  V(660F38,22,_,x,I,I,1,OVM), // #1221
  V(660F38,20,_,x,I,I,3,HVM), // #1222
  V(660F38,25,_,x,I,0,3,HVM), // #1223
  V(660F38,23,_,x,I,I,3,HVM), // #1224
  V(660F38,24,_,x,I,I,2,QVM), // #1225
  V(F30F38,11,_,x,_,0,2,QVM), // #1226
  V(F30F38,13,_,x,_,0,3,HVM), // #1227
  V(F30F38,12,_,x,_,0,1,OVM), // #1228
  V(F30F38,15,_,x,_,0,3,HVM), // #1229
  V(F30F38,14,_,x,_,0,2,QVM), // #1230
  V(F30F38,10,_,x,_,0,3,HVM), // #1231
  V(F30F38,29,_,x,_,1,_,_  ), // #1232
  V(F30F38,30,_,x,_,0,3,HVM), // #1233
  V(660F38,31,_,x,I,I,2,QVM), // #1234
  V(660F38,32,_,x,I,I,1,OVM), // #1235
  V(660F38,30,_,x,I,I,3,HVM), // #1236
  V(660F38,35,_,x,I,0,3,HVM), // #1237
  V(660F38,33,_,x,I,I,3,HVM), // #1238
  V(660F38,34,_,x,I,I,2,QVM), // #1239
  V(660F38,28,_,x,I,1,4,FV ), // #1240
  V(660F38,0B,_,x,I,I,4,FVM), // #1241
  V(660F00,E4,_,x,I,I,4,FVM), // #1242
  V(660F00,E5,_,x,I,I,4,FVM), // #1243
  V(660F38,40,_,x,I,0,4,FV ), // #1244
  V(660F38,40,_,x,_,1,4,FV ), // #1245
  V(660F00,D5,_,x,I,I,4,FVM), // #1246
  V(660F38,83,_,x,_,1,4,FV ), // #1247
  V(660F00,F4,_,x,I,1,4,FV ), // #1248
  V(660F38,54,_,x,_,0,4,FV ), // #1249
  V(660F38,55,_,x,_,0,4,FVM), // #1250
  V(660F38,55,_,x,_,1,4,FVM), // #1251
  V(660F38,54,_,x,_,1,4,FV ), // #1252
  V(660F00,EB,_,x,I,_,_,_  ), // #1253
  V(660F00,EB,_,x,_,0,4,FV ), // #1254
  V(660F00,EB,_,x,_,1,4,FV ), // #1255
  V(XOP_M8,A3,_,0,x,_,_,_  ), // #1256
  V(660F00,72,1,x,_,0,4,FV ), // #1257
  V(660F00,72,1,x,_,1,4,FV ), // #1258
  V(660F38,15,_,x,_,0,4,FV ), // #1259
  V(660F38,15,_,x,_,1,4,FV ), // #1260
  V(660F00,72,0,x,_,0,4,FV ), // #1261
  V(660F00,72,0,x,_,1,4,FV ), // #1262
  V(660F38,14,_,x,_,0,4,FV ), // #1263
  V(660F38,14,_,x,_,1,4,FV ), // #1264
  V(XOP_M9,90,_,0,x,_,_,_  ), // #1265
  V(XOP_M9,92,_,0,x,_,_,_  ), // #1266
  V(XOP_M9,93,_,0,x,_,_,_  ), // #1267
  V(XOP_M9,91,_,0,x,_,_,_  ), // #1268
  V(660F00,F6,_,x,I,I,4,FVM), // #1269
  V(660F38,A0,_,x,_,0,2,T1S), // #1270
  V(660F38,A0,_,x,_,1,3,T1S), // #1271
  V(660F38,A1,_,x,_,0,2,T1S), // #1272
  V(660F38,A1,_,x,_,1,3,T1S), // #1273
  V(XOP_M9,98,_,0,x,_,_,_  ), // #1274
  V(XOP_M9,9A,_,0,x,_,_,_  ), // #1275
  V(XOP_M9,9B,_,0,x,_,_,_  ), // #1276
  V(XOP_M9,99,_,0,x,_,_,_  ), // #1277
  V(XOP_M9,94,_,0,x,_,_,_  ), // #1278
  V(XOP_M9,96,_,0,x,_,_,_  ), // #1279
  V(660F3A,71,_,x,_,0,4,FV ), // #1280
  V(660F3A,71,_,x,_,1,4,FV ), // #1281
  V(660F38,71,_,x,_,0,4,FV ), // #1282
  V(660F38,71,_,x,_,1,4,FV ), // #1283
  V(660F38,70,_,x,_,0,4,FVM), // #1284
  V(660F3A,70,_,x,_,0,4,FVM), // #1285
  V(XOP_M9,97,_,0,x,_,_,_  ), // #1286
  V(XOP_M9,95,_,0,x,_,_,_  ), // #1287
  V(660F3A,73,_,x,_,0,4,FV ), // #1288
  V(660F3A,73,_,x,_,1,4,FV ), // #1289
  V(660F38,73,_,x,_,0,4,FV ), // #1290
  V(660F38,73,_,x,_,1,4,FV ), // #1291
  V(660F38,72,_,x,_,0,4,FVM), // #1292
  V(660F3A,72,_,x,_,0,4,FVM), // #1293
  V(660F38,00,_,x,I,I,4,FVM), // #1294
  V(660F38,8F,_,x,0,0,4,FVM), // #1295
  V(660F00,70,_,x,I,0,4,FV ), // #1296
  V(F30F00,70,_,x,I,I,4,FVM), // #1297
  V(F20F00,70,_,x,I,I,4,FVM), // #1298
  V(660F38,08,_,x,I,_,_,_  ), // #1299
  V(660F38,0A,_,x,I,_,_,_  ), // #1300
  V(660F38,09,_,x,I,_,_,_  ), // #1301
  V(660F00,F2,_,x,I,0,4,128), // #1302
  V(660F00,73,7,x,I,I,4,FVM), // #1303
  V(660F00,F3,_,x,I,1,4,128), // #1304
  V(660F38,47,_,x,0,0,4,FV ), // #1305
  V(660F38,47,_,x,1,1,4,FV ), // #1306
  V(660F38,12,_,x,_,1,4,FVM), // #1307
  V(660F00,F1,_,x,I,I,4,FVM), // #1308
  V(660F00,E2,_,x,I,0,4,128), // #1309
  V(660F00,E2,_,x,_,1,4,128), // #1310
  V(660F38,46,_,x,0,0,4,FV ), // #1311
  V(660F38,46,_,x,_,1,4,FV ), // #1312
  V(660F38,11,_,x,_,1,4,FVM), // #1313
  V(660F00,E1,_,x,I,I,4,128), // #1314
  V(660F00,D2,_,x,I,0,4,128), // #1315
  V(660F00,73,3,x,I,I,4,FVM), // #1316
  V(660F00,D3,_,x,I,1,4,128), // #1317
  V(660F38,45,_,x,0,0,4,FV ), // #1318
  V(660F38,45,_,x,1,1,4,FV ), // #1319
  V(660F38,10,_,x,_,1,4,FVM), // #1320
  V(660F00,D1,_,x,I,I,4,128), // #1321
  V(660F00,F8,_,x,I,I,4,FVM), // #1322
  V(660F00,FA,_,x,I,0,4,FV ), // #1323
  V(660F00,FB,_,x,I,1,4,FV ), // #1324
  V(660F00,E8,_,x,I,I,4,FVM), // #1325
  V(660F00,E9,_,x,I,I,4,FVM), // #1326
  V(660F00,D8,_,x,I,I,4,FVM), // #1327
  V(660F00,D9,_,x,I,I,4,FVM), // #1328
  V(660F00,F9,_,x,I,I,4,FVM), // #1329
  V(660F3A,25,_,x,_,0,4,FV ), // #1330
  V(660F3A,25,_,x,_,1,4,FV ), // #1331
  V(660F38,17,_,x,I,_,_,_  ), // #1332
  V(660F38,26,_,x,_,0,4,FVM), // #1333
  V(660F38,27,_,x,_,0,4,FV ), // #1334
  V(660F38,27,_,x,_,1,4,FV ), // #1335
  V(660F38,26,_,x,_,1,4,FVM), // #1336
  V(F30F38,26,_,x,_,0,4,FVM), // #1337
  V(F30F38,27,_,x,_,0,4,FV ), // #1338
  V(F30F38,27,_,x,_,1,4,FV ), // #1339
  V(F30F38,26,_,x,_,1,4,FVM), // #1340
  V(660F00,68,_,x,I,I,4,FVM), // #1341
  V(660F00,6A,_,x,I,0,4,FV ), // #1342
  V(660F00,6D,_,x,I,1,4,FV ), // #1343
  V(660F00,69,_,x,I,I,4,FVM), // #1344
  V(660F00,60,_,x,I,I,4,FVM), // #1345
  V(660F00,62,_,x,I,0,4,FV ), // #1346
  V(660F00,6C,_,x,I,1,4,FV ), // #1347
  V(660F00,61,_,x,I,I,4,FVM), // #1348
  V(660F00,EF,_,x,I,_,_,_  ), // #1349
  V(660F00,EF,_,x,_,0,4,FV ), // #1350
  V(660F00,EF,_,x,_,1,4,FV ), // #1351
  V(660F3A,50,_,x,_,1,4,FV ), // #1352
  V(660F3A,50,_,x,_,0,4,FV ), // #1353
  V(660F3A,51,_,I,_,1,3,T1S), // #1354
  V(660F3A,51,_,I,_,0,2,T1S), // #1355
  V(660F38,4C,_,x,_,1,4,FV ), // #1356
  V(660F38,4C,_,x,_,0,4,FV ), // #1357
  V(660F38,4D,_,I,_,1,3,T1S), // #1358
  V(660F38,4D,_,I,_,0,2,T1S), // #1359
  V(660F38,CA,_,2,_,1,4,FV ), // #1360
  V(660F38,CA,_,2,_,0,4,FV ), // #1361
  V(660F38,CB,_,I,_,1,3,T1S), // #1362
  V(660F38,CB,_,I,_,0,2,T1S), // #1363
  V(000F00,53,_,x,I,_,_,_  ), // #1364
  V(F30F00,53,_,I,I,_,_,_  ), // #1365
  V(660F3A,56,_,x,_,1,4,FV ), // #1366
  V(660F3A,56,_,x,_,0,4,FV ), // #1367
  V(660F3A,57,_,I,_,1,3,T1S), // #1368
  V(660F3A,57,_,I,_,0,2,T1S), // #1369
  V(660F3A,09,_,x,_,1,4,FV ), // #1370
  V(660F3A,08,_,x,_,0,4,FV ), // #1371
  V(660F3A,0B,_,I,_,1,3,T1S), // #1372
  V(660F3A,0A,_,I,_,0,2,T1S), // #1373
  V(660F3A,09,_,x,I,_,_,_  ), // #1374
  V(660F3A,08,_,x,I,_,_,_  ), // #1375
  V(660F3A,0B,_,I,I,_,_,_  ), // #1376
  V(660F3A,0A,_,I,I,_,_,_  ), // #1377
  V(660F38,4E,_,x,_,1,4,FV ), // #1378
  V(660F38,4E,_,x,_,0,4,FV ), // #1379
  V(660F38,4F,_,I,_,1,3,T1S), // #1380
  V(660F38,4F,_,I,_,0,2,T1S), // #1381
  V(660F38,CC,_,2,_,1,4,FV ), // #1382
  V(660F38,CC,_,2,_,0,4,FV ), // #1383
  V(660F38,CD,_,I,_,1,3,T1S), // #1384
  V(660F38,CD,_,I,_,0,2,T1S), // #1385
  V(000F00,52,_,x,I,_,_,_  ), // #1386
  V(F30F00,52,_,I,I,_,_,_  ), // #1387
  V(660F38,2C,_,x,_,1,4,FV ), // #1388
  V(660F38,2C,_,x,_,0,4,FV ), // #1389
  V(660F38,2D,_,I,_,1,3,T1S), // #1390
  V(660F38,2D,_,I,_,0,2,T1S), // #1391
  V(660F38,A2,_,x,_,1,3,T1S), // #1392
  V(660F38,A2,_,x,_,0,2,T1S), // #1393
  V(660F38,C6,5,2,_,1,3,T1S), // #1394
  V(660F38,C6,5,2,_,0,2,T1S), // #1395
  V(660F38,C7,5,2,_,1,3,T1S), // #1396
  V(660F38,C7,5,2,_,0,2,T1S), // #1397
  V(660F38,C6,6,2,_,1,3,T1S), // #1398
  V(660F38,C6,6,2,_,0,2,T1S), // #1399
  V(660F38,C7,6,2,_,1,3,T1S), // #1400
  V(660F38,C7,6,2,_,0,2,T1S), // #1401
  V(660F38,A3,_,x,_,1,3,T1S), // #1402
  V(660F38,A3,_,x,_,0,2,T1S), // #1403
  V(660F3A,23,_,x,_,0,4,FV ), // #1404
  V(660F3A,23,_,x,_,1,4,FV ), // #1405
  V(660F3A,43,_,x,_,0,4,FV ), // #1406
  V(660F3A,43,_,x,_,1,4,FV ), // #1407
  V(660F00,C6,_,x,I,1,4,FV ), // #1408
  V(000F00,C6,_,x,I,0,4,FV ), // #1409
  V(660F00,51,_,x,I,1,4,FV ), // #1410
  V(000F00,51,_,x,I,0,4,FV ), // #1411
  V(F20F00,51,_,I,I,1,3,T1S), // #1412
  V(F30F00,51,_,I,I,0,2,T1S), // #1413
  V(000F00,AE,3,0,I,_,_,_  ), // #1414
  V(660F00,5C,_,x,I,1,4,FV ), // #1415
  V(000F00,5C,_,x,I,0,4,FV ), // #1416
  V(F20F00,5C,_,I,I,1,3,T1S), // #1417
  V(F30F00,5C,_,I,I,0,2,T1S), // #1418
  V(660F38,0F,_,x,0,_,_,_  ), // #1419
  V(660F38,0E,_,x,0,_,_,_  ), // #1420
  V(660F00,2E,_,I,I,1,3,T1S), // #1421
  V(000F00,2E,_,I,I,0,2,T1S), // #1422
  V(660F00,15,_,x,I,1,4,FV ), // #1423
  V(000F00,15,_,x,I,0,4,FV ), // #1424
  V(660F00,14,_,x,I,1,4,FV ), // #1425
  V(000F00,14,_,x,I,0,4,FV ), // #1426
  V(660F00,57,_,x,I,1,4,FV ), // #1427
  V(000F00,57,_,x,I,0,4,FV ), // #1428
  V(000F00,77,_,1,I,_,_,_  ), // #1429
  V(000F00,77,_,0,I,_,_,_  ), // #1430
  O(000F00,09,_,_,_,_,_,_  ), // #1431
  O(F30F00,AE,2,_,x,_,_,_  ), // #1432
  O(F30F00,AE,3,_,x,_,_,_  ), // #1433
  O(000F00,30,_,_,_,_,_,_  ), // #1434
  O(000000,C6,7,_,_,_,_,_  ), // #1435
  O(000F00,C0,_,_,x,_,_,_  ), // #1436
  O(000000,C7,7,_,_,_,_,_  ), // #1437
  O(000000,86,_,_,x,_,_,_  ), // #1438
  O(000F01,D5,_,_,_,_,_,_  ), // #1439
  O(000F01,D0,_,_,_,_,_,_  ), // #1440
  O(000000,D7,_,_,_,_,_,_  ), // #1441
  O(000000,30,6,_,x,_,_,_  ), // #1442
  O(660F00,57,_,_,_,_,_,_  ), // #1443
  O(000F00,57,_,_,_,_,_,_  ), // #1444
  O(000F00,AE,5,_,_,_,_,_  ), // #1445
  O(000F00,AE,5,_,1,_,_,_  ), // #1446
  O(000F00,C7,3,_,_,_,_,_  ), // #1447
  O(000F00,C7,3,_,1,_,_,_  ), // #1448
  O(000F00,AE,4,_,_,_,_,_  ), // #1449
  O(000F00,AE,4,_,1,_,_,_  ), // #1450
  O(000F00,C7,4,_,_,_,_,_  ), // #1451
  O(000F00,C7,4,_,1,_,_,_  ), // #1452
  O(000F00,AE,6,_,_,_,_,_  ), // #1453
  O(000F00,AE,6,_,1,_,_,_  ), // #1454
  O(000F00,C7,5,_,_,_,_,_  ), // #1455
  O(000F00,C7,5,_,1,_,_,_  ), // #1456
  O(000F01,D1,_,_,_,_,_,_  ), // #1457
  O(000F01,D6,_,_,_,_,_,_  )  // #1458
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
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, 9, 10, 0, 0, 11, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 13, 14,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 15, 16, 0, 0, 17, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 18, 19, 20, 21, 20, 22, 23,
  24, 25, 26, 27, 28, 21, 20, 19, 18, 19, 29, 27, 26, 25, 24, 30, 31, 32, 29,
  33, 34, 34, 31, 35, 22, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 36, 37, 38, 39, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 40,
  41, 42, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 43, 44, 45, 46,
  0, 0, 47, 48, 0, 49, 50, 0, 51, 52, 0, 0, 53, 0, 0, 54, 55, 56, 57, 58, 59, 0,
  0, 60, 0, 0, 61, 0, 0, 62, 63, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  64, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 65, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 66, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 77, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 78, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 79, 80, 0, 0, 0, 0, 0, 0, 0, 0, 81, 82, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 83, 84,
  0, 0, 0, 0, 0, 0, 0, 0, 85, 86, 87, 0, 88, 89, 90, 91, 92, 93, 94, 95, 0, 96,
  97, 0, 98, 99, 0, 0, 0, 0, 0, 0, 100, 101, 0, 0, 102, 103, 104, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 105, 106, 0, 0, 107, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 108, 109, 110, 111, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 112, 113, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 114, 115, 116, 117, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 118, 0, 119,
  0, 0, 0, 120, 121, 122, 0, 0, 0, 123, 124, 0, 125, 0, 0, 0, 126, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0
};
// ----------------------------------------------------------------------------
// ${AltOpcodeIndex:End}

// ${AltOpcodeTable:Begin}
// ------------------- Automatically generated, do not edit -------------------
const uint32_t InstDB::_altOpcodeTable[] = {
  0                         , // #0 [ref=1319x]
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
  { F(UseR)                                               , 0  , 0  , 0  , 0 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #0 [ref=1x]
  { F(UseX)|F(FixedReg)                                   , 0  , 0  , 334, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #1 [ref=4x]
  { F(UseX)|F(FixedReg)                                   , 0  , 0  , 335, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #2 [ref=2x]
  { F(UseX)|F(Lock)|F(XAcquire)|F(XRelease)               , 0  , 0  , 16 , 12, CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #3 [ref=3x]
  { F(UseX)                                               , 0  , 0  , 156, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #4 [ref=2x]
  { F(UseX)|F(Vec)                                        , 0  , 0  , 71 , 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #5 [ref=38x]
  { F(UseX)|F(Vec)                                        , 0  , 0  , 98 , 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #6 [ref=6x]
  { F(UseX)|F(Vec)                                        , 0  , 0  , 227, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #7 [ref=6x]
  { F(UseW)|F(Vec)                                        , 0  , 0  , 71 , 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #8 [ref=2x]
  { F(UseW)|F(Vec)                                        , 0  , 0  , 188, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #9 [ref=1x]
  { F(UseX)|F(Lock)|F(XAcquire)|F(XRelease)               , 0  , 0  , 28 , 12, CONTROL(None)   , SINGLE_REG(RO)  , 0                   , 0 }, // #10 [ref=1x]
  { F(UseW)|F(Vex)                                        , 0  , 0  , 242, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #11 [ref=3x]
  { F(UseX)|F(Vec)                                        , 0  , 0  , 71 , 1 , CONTROL(None)   , SINGLE_REG(RO)  , 0                   , 0 }, // #12 [ref=12x]
  { F(UseX)                                               , 0  , 0  , 336, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #13 [ref=1x]
  { F(UseW)|F(Vex)                                        , 0  , 0  , 244, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #14 [ref=5x]
  { F(UseW)|F(Vex)                                        , 0  , 0  , 156, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #15 [ref=12x]
  { F(UseX)|F(Vec)                                        , 0  , 0  , 188, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #16 [ref=12x]
  { F(UseX)|F(FixedReg)|F(Vec)                            , 0  , 0  , 337, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #17 [ref=4x]
  { F(UseR)                                               , 0  , 0  , 246, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #18 [ref=3x]
  { F(UseW)|F(Mib)                                        , 0  , 0  , 338, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #19 [ref=1x]
  { F(UseW)                                               , 0  , 0  , 339, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #20 [ref=1x]
  { F(UseW)                                               , 0  , 0  , 248, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #21 [ref=1x]
  { F(UseW)|F(Mib)                                        , 0  , 0  , 340, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #22 [ref=1x]
  { F(UseR)                                               , 0  , 0  , 250, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #23 [ref=1x]
  { F(UseW)                                               , 0  , 0  , 155, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #24 [ref=5x]
  { F(UseX)                                               , 0  , 0  , 341, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #25 [ref=1x]
  { F(UseR)                                               , 0  , 0  , 115, 4 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #26 [ref=1x]
  { F(UseX)|F(Lock)|F(XAcquire)|F(XRelease)               , 0  , 0  , 115, 4 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #27 [ref=3x]
  { F(UseR)                                               , 0  , 0  , 252, 2 , CONTROL(Call)   , SINGLE_REG(None), 0                   , 0 }, // #28 [ref=1x]
  { F(UseX)|F(FixedReg)                                   , 0  , 0  , 342, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #29 [ref=1x]
  { F(UseW)|F(FixedReg)                                   , 0  , 0  , 343, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #30 [ref=1x]
  { F(UseX)|F(FixedReg)                                   , 0  , 0  , 344, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #31 [ref=1x]
  { F(Volatile)|F(UseR)                                   , 0  , 0  , 260, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #32 [ref=16x]
  { F(UseR)                                               , 0  , 0  , 260, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #33 [ref=43x]
  { F(Volatile)|F(UseR)                                   , 0  , 0  , 345, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #34 [ref=4x]
  { F(Volatile)|F(Privileged)|F(UseR)                     , 0  , 0  , 260, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #35 [ref=6x]
  { F(Volatile)|F(UseR)|F(FixedRM)                        , 0  , 0  , 346, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #36 [ref=1x]
  { F(UseX)                                               , 0  , 0  , 155, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #37 [ref=30x]
  { F(UseR)                                               , 0  , 0  , 16 , 12, CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #38 [ref=1x]
  { F(UseX)|F(FixedRM)|F(Rep)|F(Repne)                    , 0  , 0  , 347, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #39 [ref=1x]
  { F(UseX)|F(Vec)                                        , 0  , 0  , 348, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #40 [ref=1x]
  { F(UseX)|F(Vec)                                        , 0  , 0  , 349, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #41 [ref=2x]
  { F(UseX)|F(FixedReg)|F(Lock)|F(XAcquire)|F(XRelease)   , 0  , 0  , 119, 4 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #42 [ref=1x]
  { F(UseX)|F(FixedReg)|F(Lock)|F(XAcquire)|F(XRelease)   , 0  , 0  , 350, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #43 [ref=1x]
  { F(UseX)|F(FixedReg)|F(Lock)|F(XAcquire)|F(XRelease)   , 0  , 0  , 351, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #44 [ref=1x]
  { F(UseR)|F(Vec)                                        , 0  , 0  , 98 , 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #45 [ref=2x]
  { F(UseR)|F(Vec)                                        , 0  , 0  , 227, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #46 [ref=2x]
  { F(Volatile)|F(UseX)|F(FixedReg)                       , 0  , 0  , 352, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #47 [ref=1x]
  { F(UseW)|F(FixedReg)                                   , 0  , 0  , 353, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #48 [ref=1x]
  { F(UseX)                                               , 0  , 0  , 254, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #49 [ref=1x]
  { F(UseW)|F(Vec)                                        , 0  , 16 , 98 , 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #50 [ref=9x]
  { F(UseW)|F(Vec)                                        , 0  , 16 , 71 , 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #51 [ref=12x]
  { F(UseW)|F(Mmx)|F(Vec)                                 , 0  , 8  , 354, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #52 [ref=2x]
  { F(UseW)|F(Mmx)|F(Vec)                                 , 0  , 16 , 355, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #53 [ref=1x]
  { F(UseW)|F(Mmx)|F(Vec)                                 , 0  , 8  , 355, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #54 [ref=1x]
  { F(UseW)|F(Mmx)|F(Vec)                                 , 0  , 8  , 356, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #55 [ref=2x]
  { F(UseW)|F(Vec)                                        , 0  , 8  , 357, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #56 [ref=2x]
  { F(UseW)|F(Vec)                                        , 0  , 4  , 98 , 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #57 [ref=1x]
  { F(UseW)|F(Vec)                                        , 0  , 8  , 358, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #58 [ref=1x]
  { F(UseW)|F(Vec)                                        , 0  , 4  , 358, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #59 [ref=1x]
  { F(UseW)|F(Vec)                                        , 0  , 8  , 227, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #60 [ref=1x]
  { F(UseW)|F(Vec)                                        , 0  , 8  , 359, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #61 [ref=2x]
  { F(UseW)|F(FixedReg)                                   , 0  , 0  , 360, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #62 [ref=1x]
  { F(UseX)|F(FixedReg)                                   , 0  , 0  , 361, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #63 [ref=1x]
  { F(UseX)|F(Lock)|F(XAcquire)|F(XRelease)               , 0  , 0  , 256, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #64 [ref=2x]
  { F(UseX)|F(FixedReg)                                   , 0  , 0  , 40 , 4 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #65 [ref=2x]
  { F(Volatile)|F(UseR)|F(Mmx)                            , 0  , 0  , 260, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #66 [ref=1x]
  { F(Volatile)|F(UseR)                                   , 0  , 0  , 362, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #67 [ref=1x]
  { F(UseW)|F(Vec)                                        , 0  , 8  , 363, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #68 [ref=2x]
  { F(UseX)|F(Vec)                                        , 0  , 0  , 258, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #69 [ref=1x]
  { F(UseA)|F(FixedReg)|F(FpuM32)|F(FpuM64)               , 0  , 0  , 158, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #70 [ref=5x]
  { F(UseX)                                               , 0  , 0  , 260, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #71 [ref=7x]
  { F(UseR)|F(FpuM80)                                     , 0  , 0  , 364, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #72 [ref=1x]
  { F(UseW)|F(FpuM80)                                     , 0  , 0  , 364, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #73 [ref=1x]
  { F(UseX)                                               , 0  , 0  , 261, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #74 [ref=9x]
  { F(UseR)|F(FpuM32)|F(FpuM64)                           , 0  , 0  , 262, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #75 [ref=2x]
  { F(UseR)                                               , 0  , 0  , 261, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #76 [ref=4x]
  { F(UseR)|F(FpuM16)|F(FpuM32)                           , 0  , 0  , 365, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #77 [ref=8x]
  { F(UseR)|F(FpuM16)|F(FpuM32)|F(FpuM64)                 , 0  , 0  , 366, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #78 [ref=1x]
  { F(UseW)|F(FpuM16)|F(FpuM32)                           , 0  , 0  , 365, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #79 [ref=1x]
  { F(UseW)|F(FpuM16)|F(FpuM32)|F(FpuM64)                 , 0  , 0  , 366, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #80 [ref=2x]
  { F(UseR)|F(FpuM32)|F(FpuM64)|F(FpuM80)                 , 0  , 0  , 367, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #81 [ref=1x]
  { F(UseR)|F(FpuM16)                                     , 0  , 0  , 368, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #82 [ref=1x]
  { F(UseR)                                               , 0  , 0  , 345, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #83 [ref=9x]
  { F(UseX)|F(FixedReg)|F(FpuM32)|F(FpuM64)               , 0  , 0  , 158, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #84 [ref=1x]
  { F(UseW)                                               , 0  , 0  , 345, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #85 [ref=4x]
  { F(UseW)|F(FpuM16)                                     , 0  , 0  , 368, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #86 [ref=2x]
  { F(UseW)|F(FixedReg)|F(FpuM16)                         , 0  , 0  , 369, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #87 [ref=2x]
  { F(UseW)|F(FpuM32)|F(FpuM64)                           , 0  , 0  , 263, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #88 [ref=1x]
  { F(UseW)|F(FpuM32)|F(FpuM64)|F(FpuM80)                 , 0  , 0  , 367, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #89 [ref=1x]
  { F(UseR)                                               , 0  , 0  , 260, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #90 [ref=2x]
  { F(Volatile)|F(UseR)                                   , 0  , 0  , 370, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #91 [ref=1x]
  { F(Volatile)|F(UseW)                                   , 0  , 0  , 345, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #92 [ref=3x]
  { F(Volatile)|F(UseW)                                   , 0  , 0  , 370, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #93 [ref=1x]
  { F(UseA)|F(FixedReg)                                   , 0  , 0  , 40 , 10, CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #94 [ref=1x]
  { F(Volatile)|F(UseW)|F(FixedReg)                       , 0  , 0  , 371, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #95 [ref=1x]
  { F(Volatile)|F(UseW)|F(FixedRM)|F(Rep)|F(Repne)        , 0  , 0  , 372, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #96 [ref=1x]
  { F(UseX)|F(Vec)                                        , 0  , 0  , 264, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #97 [ref=1x]
  { F(Volatile)|F(UseR)                                   , 0  , 0  , 373, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #98 [ref=2x]
  { F(Volatile)|F(UseR)                                   , 0  , 0  , 374, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #99 [ref=2x]
  { F(Volatile)|F(Privileged)|F(UseR)                     , 0  , 0  , 345, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #100 [ref=3x]
  { F(Volatile)|F(Privileged)|F(UseR)                     , 0  , 0  , 266, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #101 [ref=1x]
  { F(UseR)                                               , 0  , 0  , 260, 1 , CONTROL(Return) , SINGLE_REG(None), 0                   , 0 }, // #102 [ref=3x]
  { F(UseR)                                               , 0  , 0  , 375, 1 , CONTROL(Return) , SINGLE_REG(None), 0                   , 0 }, // #103 [ref=1x]
  { F(UseR)                                               , 0  , 0  , 268, 2 , CONTROL(Branch) , SINGLE_REG(None), 0                   , 0 }, // #104 [ref=30x]
  { F(UseR)|F(FixedReg)                                   , 0  , 0  , 270, 2 , CONTROL(Branch) , SINGLE_REG(None), 0                   , 0 }, // #105 [ref=1x]
  { F(UseR)                                               , 0  , 0  , 272, 2 , CONTROL(Jump)   , SINGLE_REG(None), 0                   , 0 }, // #106 [ref=1x]
  { F(UseW)|F(Vec)|F(Vex)                                 , 0  , 0  , 376, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #107 [ref=27x]
  { F(UseW)|F(Vec)|F(Vex)                                 , 0  , 0  , 274, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #108 [ref=1x]
  { F(UseW)|F(Vec)|F(Vex)                                 , 0  , 0  , 276, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #109 [ref=1x]
  { F(UseW)|F(Vec)|F(Vex)                                 , 0  , 0  , 278, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #110 [ref=1x]
  { F(UseW)|F(Vec)|F(Vex)                                 , 0  , 0  , 280, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #111 [ref=1x]
  { F(UseW)|F(Vec)|F(Vex)                                 , 0  , 0  , 377, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #112 [ref=4x]
  { F(UseR)|F(Vec)|F(Vex)                                 , 0  , 0  , 377, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #113 [ref=8x]
  { F(UseW)|F(Vec)|F(Vex)                                 , 0  , 0  , 378, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #114 [ref=8x]
  { F(Volatile)|F(UseW)|F(FixedReg)                       , 0  , 0  , 379, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #115 [ref=1x]
  { F(Volatile)|F(UseW)                                   , 0  , 0  , 282, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #116 [ref=1x]
  { F(UseW)|F(Vec)                                        , 0  , 16 , 197, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #117 [ref=2x]
  { F(UseR)                                               , 0  , 0  , 380, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #118 [ref=1x]
  { F(Volatile)|F(UseX)                                   , 0  , 0  , 284, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #119 [ref=2x]
  { F(UseW)                                               , 0  , 0  , 381, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #120 [ref=1x]
  { F(Volatile)|F(UseX)                                   , 0  , 0  , 161, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #121 [ref=3x]
  { F(Volatile)|F(Privileged)|F(UseR)                     , 0  , 0  , 382, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #122 [ref=3x]
  { F(UseW)|F(FixedRM)|F(Rep)|F(Repne)                    , 0  , 0  , 383, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #123 [ref=1x]
  { F(UseX)|F(FixedReg)                                   , 0  , 0  , 270, 2 , CONTROL(Branch) , SINGLE_REG(None), 0                   , 0 }, // #124 [ref=3x]
  { F(Volatile)|F(UseW)                                   , 0  , 0  , 286, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #125 [ref=1x]
  { F(UseR)|F(FixedRM)|F(Vec)                             , 0  , 0  , 384, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #126 [ref=1x]
  { F(UseR)|F(FixedRM)|F(Mmx)                             , 0  , 0  , 385, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #127 [ref=1x]
  { F(Volatile)|F(Privileged)|F(UseR)|F(FixedRM)          , 0  , 0  , 386, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #128 [ref=1x]
  { F(Volatile)|F(UseR)|F(FixedRM)                        , 0  , 0  , 386, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #129 [ref=1x]
  { F(UseW)|F(XRelease)                                   , 0  , 0  , 0  , 16, CONTROL(None)   , SINGLE_REG(None), SPECIAL_CASE(MovCrDr), 0 }, // #130 [ref=1x]
  { F(UseW)|F(Vec)                                        , 0  , 16 , 71 , 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #131 [ref=6x]
  { F(UseW)                                               , 0  , 0  , 65 , 6 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #132 [ref=1x]
  { F(UseW)|F(Mmx)|F(Vec)                                 , 0  , 16 , 288, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #133 [ref=1x]
  { F(UseW)|F(Mmx)|F(Vec)                                 , 0  , 8  , 387, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #134 [ref=1x]
  { F(UseW)|F(Vec)                                        , 0  , 8  , 259, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #135 [ref=1x]
  { F(UseW)|F(Vec)                                        , 8  , 8  , 203, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #136 [ref=2x]
  { F(UseW)|F(Vec)                                        , 8  , 8  , 259, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #137 [ref=1x]
  { F(UseW)|F(Vec)                                        , 0  , 8  , 203, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #138 [ref=2x]
  { F(UseW)|F(Vec)                                        , 0  , 8  , 388, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #139 [ref=2x]
  { F(UseW)|F(Vec)                                        , 0  , 16 , 72 , 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #140 [ref=3x]
  { F(UseW)                                               , 0  , 8  , 69 , 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #141 [ref=1x]
  { F(UseW)|F(Mmx)                                        , 0  , 8  , 389, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #142 [ref=1x]
  { F(UseW)|F(Vec)                                        , 0  , 8  , 99 , 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #143 [ref=1x]
  { F(UseW)|F(Vec)                                        , 0  , 4  , 206, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #144 [ref=1x]
  { F(UseW)|F(Mmx)|F(Vec)                                 , 0  , 16 , 95 , 5 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #145 [ref=1x]
  { F(UseW)|F(Mmx)|F(Vec)                                 , 0  , 16 , 390, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #146 [ref=1x]
  { F(UseX)|F(FixedRM)|F(Rep)|F(Repne)                    , 0  , 0  , 391, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #147 [ref=1x]
  { F(UseW)|F(Vec)                                        , 0  , 8  , 98 , 2 , CONTROL(None)   , SINGLE_REG(None), SPECIAL_CASE(MovSsSd), 0 }, // #148 [ref=1x]
  { F(UseW)|F(Vec)                                        , 0  , 4  , 290, 2 , CONTROL(None)   , SINGLE_REG(None), SPECIAL_CASE(MovSsSd), 0 }, // #149 [ref=1x]
  { F(UseW)                                               , 0  , 0  , 292, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #150 [ref=2x]
  { F(UseW)                                               , 0  , 0  , 392, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #151 [ref=1x]
  { F(UseA)|F(FixedReg)                                   , 0  , 0  , 40 , 4 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #152 [ref=1x]
  { F(UseW)|F(FixedReg)|F(Vex)                            , 0  , 0  , 294, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #153 [ref=1x]
  { F(Volatile)|F(Privileged)|F(UseR)|F(FixedReg)         , 0  , 0  , 393, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #154 [ref=1x]
  { F(Volatile)|F(UseR)|F(FixedReg)                       , 0  , 0  , 394, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #155 [ref=1x]
  { F(UseX)|F(Lock)|F(XAcquire)|F(XRelease)               , 0  , 0  , 257, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #156 [ref=2x]
  { F(UseR)                                               , 0  , 0  , 296, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #157 [ref=1x]
  { F(UseX)|F(Lock)|F(XAcquire)|F(XRelease)               , 0  , 0  , 16 , 12, CONTROL(None)   , SINGLE_REG(RO)  , 0                   , 0 }, // #158 [ref=1x]
  { F(Volatile)|F(UseR)|F(FixedReg)                       , 0  , 0  , 395, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #159 [ref=1x]
  { F(Volatile)|F(UseR)|F(FixedRM)|F(Rep)|F(Repne)        , 0  , 0  , 396, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #160 [ref=1x]
  { F(UseW)|F(Mmx)|F(Vec)                                 , 0  , 0  , 298, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #161 [ref=3x]
  { F(UseX)|F(Mmx)|F(Vec)                                 , 0  , 0  , 298, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #162 [ref=37x]
  { F(UseX)|F(Mmx)|F(Vec)                                 , 0  , 0  , 300, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #163 [ref=1x]
  { F(UseX)|F(Mmx)|F(Vec)                                 , 0  , 0  , 298, 2 , CONTROL(None)   , SINGLE_REG(RO)  , 0                   , 0 }, // #164 [ref=6x]
  { F(UseX)|F(Mmx)|F(Vec)                                 , 0  , 0  , 298, 2 , CONTROL(None)   , SINGLE_REG(WO)  , 0                   , 0 }, // #165 [ref=16x]
  { F(UseX)|F(Mmx)                                        , 0  , 0  , 298, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #166 [ref=18x]
  { F(UseX)|F(Vec)                                        , 0  , 0  , 71 , 1 , CONTROL(None)   , SINGLE_REG(WO)  , 0                   , 0 }, // #167 [ref=4x]
  { F(UseR)|F(FixedReg)|F(Vec)                            , 0  , 0  , 397, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #168 [ref=1x]
  { F(UseR)|F(FixedReg)|F(Vec)                            , 0  , 0  , 398, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #169 [ref=1x]
  { F(UseR)|F(FixedReg)|F(Vec)                            , 0  , 0  , 399, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #170 [ref=1x]
  { F(UseR)|F(FixedReg)|F(Vec)                            , 0  , 0  , 400, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #171 [ref=1x]
  { F(UseW)|F(Vec)                                        , 0  , 8  , 401, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #172 [ref=1x]
  { F(UseW)|F(Vec)                                        , 0  , 8  , 402, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #173 [ref=1x]
  { F(UseW)|F(Mmx)|F(Vec)                                 , 0  , 8  , 302, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #174 [ref=1x]
  { F(UseW)|F(Mmx)                                        , 0  , 8  , 298, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #175 [ref=6x]
  { F(UseW)|F(Mmx)                                        , 0  , 0  , 298, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #176 [ref=2x]
  { F(UseX)|F(Vec)                                        , 0  , 0  , 403, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #177 [ref=1x]
  { F(UseX)|F(Vec)                                        , 0  , 0  , 404, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #178 [ref=1x]
  { F(UseX)|F(Vec)                                        , 0  , 0  , 405, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #179 [ref=1x]
  { F(UseX)|F(Mmx)|F(Vec)                                 , 0  , 0  , 406, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #180 [ref=1x]
  { F(UseW)|F(Mmx)|F(Vec)                                 , 0  , 8  , 407, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #181 [ref=1x]
  { F(UseW)|F(Vec)                                        , 0  , 16 , 227, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #182 [ref=4x]
  { F(UseW)|F(Vec)                                        , 0  , 16 , 230, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #183 [ref=2x]
  { F(UseW)|F(FixedReg)                                   , 0  , 0  , 123, 4 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #184 [ref=1x]
  { F(UseR)                                               , 0  , 0  , 374, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #185 [ref=6x]
  { F(UseR)                                               , 0  , 0  , 375, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #186 [ref=2x]
  { F(UseW)|F(Vec)                                        , 0  , 16 , 188, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #187 [ref=5x]
  { F(UseW)|F(Mmx)                                        , 0  , 8  , 300, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #188 [ref=1x]
  { F(UseX)|F(Mmx)|F(Vec)                                 , 0  , 0  , 304, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #189 [ref=8x]
  { F(UseX)|F(Vec)                                        , 0  , 0  , 408, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #190 [ref=2x]
  { F(UseR)|F(Vec)                                        , 0  , 0  , 71 , 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #191 [ref=1x]
  { F(UseA)|F(FixedReg)                                   , 0  , 0  , 127, 4 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #192 [ref=1x]
  { F(UseX)|F(FixedReg)                                   , 0  , 0  , 409, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #193 [ref=8x]
  { F(UseW)|F(Vec)                                        , 0  , 4  , 227, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #194 [ref=3x]
  { F(Volatile)|F(UseW)                                   , 0  , 8  , 410, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #195 [ref=2x]
  { F(Volatile)|F(Privileged)|F(UseW)|F(FixedReg)         , 0  , 0  , 411, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #196 [ref=2x]
  { F(Volatile)|F(UseW)                                   , 0  , 8  , 412, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #197 [ref=2x]
  { F(Volatile)|F(UseW)|F(FixedReg)                       , 0  , 0  , 343, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #198 [ref=1x]
  { F(Volatile)|F(UseW)|F(FixedReg)                       , 0  , 0  , 411, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #199 [ref=2x]
  { F(UseR)                                               , 0  , 0  , 306, 2 , CONTROL(Return) , SINGLE_REG(None), 0                   , 0 }, // #200 [ref=1x]
  { F(UseW)|F(Vex)                                        , 0  , 0  , 308, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #201 [ref=1x]
  { F(UseW)|F(Vec)                                        , 0  , 8  , 348, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #202 [ref=1x]
  { F(UseW)|F(Vec)                                        , 0  , 4  , 349, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #203 [ref=1x]
  { F(Volatile)|F(UseR)|F(FixedReg)                       , 0  , 0  , 379, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #204 [ref=1x]
  { F(UseR)|F(FixedRM)|F(Rep)|F(Repne)                    , 0  , 0  , 413, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #205 [ref=1x]
  { F(UseW)                                               , 0  , 1  , 414, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #206 [ref=30x]
  { F(UseX)|F(FixedReg)                                   , 0  , 0  , 164, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #207 [ref=2x]
  { F(Volatile)|F(UseW)                                   , 0  , 0  , 415, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #208 [ref=3x]
  { F(UseW)|F(Vec)                                        , 0  , 8  , 98 , 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #209 [ref=1x]
  { F(UseW)                                               , 0  , 0  , 380, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #210 [ref=1x]
  { F(UseX)|F(FixedRM)|F(Rep)|F(Repne)                    , 0  , 0  , 416, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #211 [ref=1x]
  { F(UseX)|F(Lock)|F(XAcquire)|F(XRelease)               , 0  , 0  , 16 , 12, CONTROL(None)   , SINGLE_REG(WO)  , 0                   , 0 }, // #212 [ref=2x]
  { F(Volatile)|F(Privileged)|F(UseR)                     , 0  , 0  , 375, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #213 [ref=3x]
  { F(Volatile)|F(UseR)                                   , 0  , 0  , 375, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #214 [ref=1x]
  { F(UseR)                                               , 0  , 0  , 58 , 7 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #215 [ref=1x]
  { F(UseX)|F(Vec)|F(Evex)|F(Avx512T4X)|F(Avx512KZ)       , 0  , 0  , 417, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #216 [ref=2x]
  { F(UseX)|F(Vec)|F(Evex)|F(Avx512T4X)|F(Avx512KZ)       , 0  , 0  , 418, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #217 [ref=2x]
  { F(UseW)|F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_ER_SAE_B64)  , 0  , 0  , 167, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #218 [ref=4x]
  { F(UseW)|F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_ER_SAE_B32)  , 0  , 0  , 167, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #219 [ref=4x]
  { F(UseW)|F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_ER_SAE)      , 0  , 0  , 419, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #220 [ref=6x]
  { F(UseW)|F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_ER_SAE)      , 0  , 0  , 420, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #221 [ref=5x]
  { F(UseW)|F(Vec)|F(Vex)                                 , 0  , 0  , 167, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #222 [ref=15x]
  { F(UseW)|F(Vec)|F(Vex)|F(Evex)                         , 0  , 0  , 167, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #223 [ref=5x]
  { F(UseW)|F(Vec)|F(Vex)                                 , 0  , 0  , 71 , 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #224 [ref=17x]
  { F(UseW)|F(Vec)|F(Vex)                                 , 0  , 0  , 188, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #225 [ref=1x]
  { F(UseW)|F(Vec)|F(Evex)|F(Avx512KZ_B32)                , 0  , 0  , 170, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #226 [ref=3x]
  { F(UseW)|F(Vec)|F(Evex)|F(Avx512KZ_B64)                , 0  , 0  , 170, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #227 [ref=3x]
  { F(UseW)|F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_B64)         , 0  , 0  , 167, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #228 [ref=10x]
  { F(UseW)|F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_B32)         , 0  , 0  , 167, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #229 [ref=12x]
  { F(UseW)|F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_B64)         , 0  , 0  , 167, 3 , CONTROL(None)   , SINGLE_REG(RO)  , 0                   , 0 }, // #230 [ref=2x]
  { F(UseW)|F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_B32)         , 0  , 0  , 167, 3 , CONTROL(None)   , SINGLE_REG(RO)  , 0                   , 0 }, // #231 [ref=6x]
  { F(UseW)|F(Vec)|F(Evex)|F(Avx512KZ)                    , 0  , 0  , 167, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #232 [ref=9x]
  { F(UseW)|F(Vec)|F(Evex)|F(Avx512KZ_B32)                , 0  , 0  , 167, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #233 [ref=5x]
  { F(UseW)|F(Vec)|F(Evex)|F(Avx512KZ_B64)                , 0  , 0  , 167, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #234 [ref=12x]
  { F(UseW)|F(Vec)|F(Vex)                                 , 0  , 0  , 170, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #235 [ref=6x]
  { F(UseW)|F(Vec)|F(Vex)                                 , 0  , 0  , 310, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #236 [ref=3x]
  { F(UseW)|F(Vec)|F(Vex)                                 , 0  , 0  , 421, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #237 [ref=2x]
  { F(UseW)|F(Vec)|F(Evex)|F(Avx512KZ)                    , 0  , 0  , 422, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #238 [ref=1x]
  { F(UseW)|F(Vec)|F(Evex)|F(Avx512KZ)                    , 0  , 0  , 423, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #239 [ref=4x]
  { F(UseW)|F(Vec)|F(Evex)|F(Avx512KZ)                    , 0  , 0  , 424, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #240 [ref=4x]
  { F(UseW)|F(Vec)|F(Evex)|F(Avx512KZ)                    , 0  , 0  , 425, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #241 [ref=1x]
  { F(UseW)|F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)             , 0  , 0  , 422, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #242 [ref=1x]
  { F(UseW)|F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)             , 0  , 0  , 312, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #243 [ref=1x]
  { F(UseW)|F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_SAE_B64)     , 0  , 0  , 173, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #244 [ref=1x]
  { F(UseW)|F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_SAE_B32)     , 0  , 0  , 173, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #245 [ref=1x]
  { F(UseW)|F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_SAE)         , 0  , 0  , 426, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #246 [ref=1x]
  { F(UseW)|F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_SAE)         , 0  , 0  , 427, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #247 [ref=1x]
  { F(UseR)|F(Vec)|F(Vex)|F(Evex)|F(Avx512SAE)            , 0  , 0  , 98 , 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #248 [ref=2x]
  { F(UseR)|F(Vec)|F(Vex)|F(Evex)|F(Avx512SAE)            , 0  , 0  , 227, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #249 [ref=2x]
  { F(UseW)|F(Vec)|F(Evex)|F(Avx512KZ)                    , 0  , 0  , 176, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #250 [ref=6x]
  { F(UseW)|F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_B32)         , 0  , 0  , 179, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #251 [ref=1x]
  { F(UseW)|F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_ER_SAE_B32)  , 0  , 0  , 182, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #252 [ref=3x]
  { F(UseW)|F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_ER_SAE_B64)  , 0  , 0  , 314, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #253 [ref=2x]
  { F(UseW)|F(Vec)|F(Evex)|F(Avx512KZ_ER_SAE_B64)         , 0  , 0  , 182, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #254 [ref=4x]
  { F(UseW)|F(Vec)|F(Evex)|F(Avx512KZ_ER_SAE_B64)         , 0  , 0  , 314, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #255 [ref=3x]
  { F(UseW)|F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_SAE)         , 0  , 0  , 179, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #256 [ref=1x]
  { F(UseW)|F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_ER_SAE_B32)  , 0  , 0  , 179, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #257 [ref=1x]
  { F(UseW)|F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_SAE)         , 0  , 0  , 185, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #258 [ref=1x]
  { F(UseW)|F(Vec)|F(Evex)|F(Avx512KZ_ER_SAE_B32)         , 0  , 0  , 179, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #259 [ref=2x]
  { F(UseW)|F(Vec)|F(Evex)|F(Avx512KZ_ER_SAE_B32)         , 0  , 0  , 182, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #260 [ref=2x]
  { F(UseW)|F(Vec)|F(Vex)|F(Evex)|F(Avx512ER_SAE)         , 0  , 0  , 357, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #261 [ref=1x]
  { F(UseW)|F(Vec)|F(Evex)|F(Avx512ER_SAE)                , 0  , 0  , 357, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #262 [ref=1x]
  { F(UseW)|F(Vec)|F(Vex)|F(Evex)|F(Avx512ER_SAE)         , 0  , 0  , 428, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #263 [ref=2x]
  { F(UseW)|F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_SAE)         , 0  , 0  , 420, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #264 [ref=3x]
  { F(UseW)|F(Vec)|F(Vex)|F(Evex)|F(Avx512ER_SAE)         , 0  , 0  , 359, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #265 [ref=1x]
  { F(UseW)|F(Vec)|F(Evex)|F(Avx512ER_SAE)                , 0  , 0  , 359, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #266 [ref=1x]
  { F(UseW)|F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_SAE_B64)     , 0  , 0  , 314, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #267 [ref=1x]
  { F(UseW)|F(Vec)|F(Evex)|F(Avx512KZ_SAE_B64)            , 0  , 0  , 182, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #268 [ref=3x]
  { F(UseW)|F(Vec)|F(Evex)|F(Avx512KZ_SAE_B64)            , 0  , 0  , 314, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #269 [ref=1x]
  { F(UseW)|F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_SAE_B32)     , 0  , 0  , 182, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #270 [ref=1x]
  { F(UseW)|F(Vec)|F(Evex)|F(Avx512KZ_SAE_B32)            , 0  , 0  , 179, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #271 [ref=2x]
  { F(UseW)|F(Vec)|F(Evex)|F(Avx512KZ_SAE_B32)            , 0  , 0  , 182, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #272 [ref=2x]
  { F(UseW)|F(Vec)|F(Vex)|F(Evex)|F(Avx512SAE)            , 0  , 0  , 357, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #273 [ref=1x]
  { F(UseW)|F(Vec)|F(Evex)|F(Avx512SAE)                   , 0  , 0  , 357, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #274 [ref=1x]
  { F(UseW)|F(Vec)|F(Vex)|F(Evex)|F(Avx512SAE)            , 0  , 0  , 359, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #275 [ref=1x]
  { F(UseW)|F(Vec)|F(Evex)|F(Avx512SAE)                   , 0  , 0  , 359, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #276 [ref=1x]
  { F(UseW)|F(Vec)|F(Evex)|F(Avx512KZ_B32)                , 0  , 0  , 179, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #277 [ref=1x]
  { F(UseW)|F(Vec)|F(Evex)|F(Avx512ER_SAE)                , 0  , 0  , 428, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #278 [ref=2x]
  { F(UseW)|F(Vec)|F(Evex)|F(Avx512KZ)                    , 0  , 0  , 170, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #279 [ref=3x]
  { F(UseW)|F(Vec)|F(Vex)                                 , 0  , 0  , 170, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #280 [ref=9x]
  { F(Volatile)|F(UseR)                                   , 0  , 0  , 382, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #281 [ref=2x]
  { F(UseW)|F(Vec)|F(Evex)|F(Avx512KZ_SAE_B64)            , 0  , 0  , 75 , 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #282 [ref=3x]
  { F(UseW)|F(Vec)|F(Evex)|F(Avx512KZ_SAE_B32)            , 0  , 0  , 75 , 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #283 [ref=3x]
  { F(UseW)|F(Vec)|F(Evex)|F(Avx512KZ)                    , 0  , 0  , 182, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #284 [ref=9x]
  { F(UseW)|F(Vec)|F(Vex)                                 , 0  , 0  , 186, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #285 [ref=2x]
  { F(UseW)|F(Vec)|F(Evex)|F(Avx512KZ)                    , 0  , 0  , 429, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #286 [ref=4x]
  { F(UseW)|F(Vec)|F(Evex)|F(Avx512KZ)                    , 0  , 0  , 187, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #287 [ref=4x]
  { F(UseW)|F(Vec)|F(Vex)|F(Evex)                         , 0  , 0  , 363, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #288 [ref=2x]
  { F(UseX)|F(Vec)|F(Evex)|F(Avx512KZ_SAE_B64)            , 0  , 0  , 170, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #289 [ref=1x]
  { F(UseX)|F(Vec)|F(Evex)|F(Avx512KZ_SAE_B32)            , 0  , 0  , 170, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #290 [ref=1x]
  { F(UseX)|F(Vec)|F(Evex)|F(Avx512KZ_SAE)                , 0  , 0  , 430, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #291 [ref=1x]
  { F(UseX)|F(Vec)|F(Evex)|F(Avx512KZ_SAE)                , 0  , 0  , 431, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #292 [ref=1x]
  { F(UseX)|F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_ER_SAE_B64)  , 0  , 0  , 167, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #293 [ref=18x]
  { F(UseX)|F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_ER_SAE_B32)  , 0  , 0  , 167, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #294 [ref=18x]
  { F(UseX)|F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_ER_SAE)      , 0  , 0  , 419, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #295 [ref=12x]
  { F(UseX)|F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_ER_SAE)      , 0  , 0  , 420, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #296 [ref=12x]
  { F(UseW)|F(Vec)|F(Vex)                                 , 0  , 0  , 131, 4 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #297 [ref=13x]
  { F(UseW)|F(Vec)|F(Vex)                                 , 0  , 0  , 316, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #298 [ref=4x]
  { F(UseW)|F(Vec)|F(Vex)                                 , 0  , 0  , 318, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #299 [ref=4x]
  { F(UseW)|F(Vec)|F(Evex)|F(Avx512K_B64)                 , 0  , 0  , 432, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #300 [ref=1x]
  { F(UseW)|F(Vec)|F(Evex)|F(Avx512K_B32)                 , 0  , 0  , 432, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #301 [ref=1x]
  { F(UseW)|F(Vec)|F(Evex)|F(Avx512K)                     , 0  , 0  , 433, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #302 [ref=1x]
  { F(UseW)|F(Vec)|F(Evex)|F(Avx512K)                     , 0  , 0  , 434, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #303 [ref=1x]
  { F(UseW)|F(Vec)|F(Vex)                                 , 0  , 0  , 182, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #304 [ref=4x]
  { F(UseW)|F(Vec)|F(Vex)                                 , 0  , 0  , 98 , 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #305 [ref=1x]
  { F(UseW)|F(Vec)|F(Vex)                                 , 0  , 0  , 227, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #306 [ref=1x]
  { F(UseX)|F(Vec)|F(Vsib)|F(Vex)|F(Evex)|F(Avx512K)      , 0  , 0  , 100, 5 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #307 [ref=2x]
  { F(UseX)|F(Vec)|F(Vsib)|F(Vex)|F(Evex)|F(Avx512K)      , 0  , 0  , 105, 5 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #308 [ref=2x]
  { F(UseR)|F(Vsib)|F(Evex)|F(Avx512K)                    , 0  , 0  , 435, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #309 [ref=4x]
  { F(UseR)|F(Vsib)|F(Evex)|F(Avx512K)                    , 0  , 0  , 436, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #310 [ref=4x]
  { F(UseR)|F(Vsib)|F(Evex)|F(Avx512K)                    , 0  , 0  , 437, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #311 [ref=8x]
  { F(UseX)|F(Vec)|F(Vsib)|F(Vex)|F(Evex)|F(Avx512K)      , 0  , 0  , 110, 5 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #312 [ref=2x]
  { F(UseX)|F(Vec)|F(Vsib)|F(Vex)|F(Evex)|F(Avx512K)      , 0  , 0  , 135, 4 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #313 [ref=2x]
  { F(UseW)|F(Vec)|F(Evex)|F(Avx512KZ_SAE)                , 0  , 0  , 419, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #314 [ref=3x]
  { F(UseW)|F(Vec)|F(Evex)|F(Avx512KZ_SAE)                , 0  , 0  , 420, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #315 [ref=3x]
  { F(UseW)|F(Vec)|F(Evex)|F(Avx512KZ_SAE_B64)            , 0  , 0  , 188, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #316 [ref=2x]
  { F(UseW)|F(Vec)|F(Evex)|F(Avx512KZ_SAE_B32)            , 0  , 0  , 188, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #317 [ref=2x]
  { F(UseW)|F(Vec)|F(Evex)|F(Avx512KZ_SAE)                , 0  , 0  , 430, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #318 [ref=3x]
  { F(UseW)|F(Vec)|F(Evex)|F(Avx512KZ_SAE)                , 0  , 0  , 431, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #319 [ref=3x]
  { F(UseW)|F(Vec)|F(Vex)                                 , 0  , 0  , 320, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #320 [ref=2x]
  { F(UseW)|F(Vec)|F(Evex)|F(Avx512KZ)                    , 0  , 0  , 320, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #321 [ref=4x]
  { F(UseW)|F(Vec)|F(Evex)|F(Avx512KZ)                    , 0  , 0  , 438, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #322 [ref=4x]
  { F(UseW)|F(Vec)|F(Vex)|F(Evex)                         , 0  , 0  , 431, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #323 [ref=1x]
  { F(UseW)|F(Vec)|F(Vex)                                 , 0  , 0  , 197, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #324 [ref=1x]
  { F(UseR)|F(Vex)                                        , 0  , 0  , 380, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #325 [ref=1x]
  { F(UseR)|F(FixedRM)|F(Vec)|F(Vex)                      , 0  , 0  , 384, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #326 [ref=1x]
  { F(UseA)|F(Vec)|F(Vex)                                 , 0  , 0  , 139, 4 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #327 [ref=4x]
  { F(UseW)|F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_SAE_B64)     , 0  , 0  , 167, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #328 [ref=2x]
  { F(UseW)|F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_SAE_B32)     , 0  , 0  , 167, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #329 [ref=2x]
  { F(UseW)|F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_SAE)         , 0  , 0  , 419, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #330 [ref=2x]
  { F(UseW)|F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)             , 0  , 0  , 71 , 6 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #331 [ref=4x]
  { F(UseW)|F(Vec)|F(Vex)|F(Evex)                         , 0  , 0  , 322, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #332 [ref=1x]
  { F(UseW)|F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)             , 0  , 0  , 191, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #333 [ref=1x]
  { F(UseW)|F(Vec)|F(Vex)                                 , 0  , 0  , 71 , 4 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #334 [ref=2x]
  { F(UseW)|F(Vec)|F(Evex)|F(Avx512KZ)                    , 0  , 0  , 71 , 6 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #335 [ref=6x]
  { F(UseW)|F(Vec)|F(Vex)|F(Evex)                         , 0  , 0  , 205, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #336 [ref=2x]
  { F(UseW)|F(Vec)|F(Vex)|F(Evex)                         , 0  , 0  , 324, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #337 [ref=4x]
  { F(UseW)|F(Vec)|F(Vex)                                 , 0  , 0  , 439, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #338 [ref=3x]
  { F(UseW)|F(Vec)|F(Vex)|F(Evex)                         , 0  , 0  , 194, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #339 [ref=3x]
  { F(UseW)|F(Vec)|F(Vex)|F(Evex)                         , 0  , 0  , 197, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #340 [ref=1x]
  { F(UseW)|F(Vec)|F(Vex)|F(Evex)                         , 0  , 0  , 200, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #341 [ref=1x]
  { F(UseW)|F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)             , 0  , 0  , 203, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #342 [ref=1x]
  { F(UseW)|F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)             , 0  , 0  , 182, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #343 [ref=5x]
  { F(UseW)|F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)             , 0  , 0  , 206, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #344 [ref=1x]
  { F(UseW)|F(Vec)|F(Evex)|F(Avx512T4X)|F(Avx512KZ)       , 0  , 0  , 417, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #345 [ref=2x]
  { F(UseW)|F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)             , 0  , 0  , 167, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #346 [ref=21x]
  { F(UseW)|F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)             , 0  , 0  , 170, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #347 [ref=1x]
  { F(UseW)|F(Vec)|F(Vex)                                 , 0  , 0  , 167, 2 , CONTROL(None)   , SINGLE_REG(RO)  , 0                   , 0 }, // #348 [ref=2x]
  { F(UseW)|F(Vec)|F(Evex)|F(Avx512KZ_B32)                , 0  , 0  , 167, 3 , CONTROL(None)   , SINGLE_REG(RO)  , 0                   , 0 }, // #349 [ref=2x]
  { F(UseW)|F(Vec)|F(Vex)                                 , 0  , 0  , 167, 2 , CONTROL(None)   , SINGLE_REG(WO)  , 0                   , 0 }, // #350 [ref=2x]
  { F(UseW)|F(Vec)|F(Evex)|F(Avx512KZ_B32)                , 0  , 0  , 167, 3 , CONTROL(None)   , SINGLE_REG(WO)  , 0                   , 0 }, // #351 [ref=2x]
  { F(UseW)|F(Vec)|F(Evex)|F(Avx512KZ_B64)                , 0  , 0  , 167, 3 , CONTROL(None)   , SINGLE_REG(WO)  , 0                   , 0 }, // #352 [ref=2x]
  { F(UseW)|F(Vec)|F(Evex)|F(Avx512KZ_B64)                , 0  , 0  , 167, 3 , CONTROL(None)   , SINGLE_REG(RO)  , 0                   , 0 }, // #353 [ref=2x]
  { F(UseW)|F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)             , 0  , 0  , 440, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #354 [ref=1x]
  { F(UseW)|F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)             , 0  , 0  , 441, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #355 [ref=1x]
  { F(UseW)|F(Vec)|F(Evex)                                , 0  , 0  , 442, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #356 [ref=6x]
  { F(UseW)|F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)             , 0  , 0  , 209, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #357 [ref=1x]
  { F(UseW)|F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)             , 0  , 0  , 443, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #358 [ref=1x]
  { F(UseW)|F(Vec)|F(Vex)|F(Evex)                         , 0  , 0  , 170, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #359 [ref=1x]
  { F(UseW)|F(Vec)|F(Evex)|F(Avx512K)                     , 0  , 0  , 212, 3 , CONTROL(None)   , SINGLE_REG(WO)  , 0                   , 0 }, // #360 [ref=2x]
  { F(UseW)|F(Vec)|F(Evex)|F(Avx512K_B32)                 , 0  , 0  , 212, 3 , CONTROL(None)   , SINGLE_REG(WO)  , 0                   , 0 }, // #361 [ref=2x]
  { F(UseW)|F(Vec)|F(Vex)|F(Evex)|F(Avx512K)              , 0  , 0  , 215, 3 , CONTROL(None)   , SINGLE_REG(WO)  , 0                   , 0 }, // #362 [ref=4x]
  { F(UseW)|F(Vec)|F(Vex)|F(Evex)|F(Avx512K_B32)          , 0  , 0  , 215, 3 , CONTROL(None)   , SINGLE_REG(WO)  , 0                   , 0 }, // #363 [ref=2x]
  { F(UseW)|F(Vec)|F(Vex)|F(Evex)|F(Avx512K_B64)          , 0  , 0  , 215, 3 , CONTROL(None)   , SINGLE_REG(WO)  , 0                   , 0 }, // #364 [ref=2x]
  { F(UseR)|F(FixedReg)|F(Vec)|F(Vex)                     , 0  , 0  , 397, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #365 [ref=1x]
  { F(UseR)|F(FixedReg)|F(Vec)|F(Vex)                     , 0  , 0  , 398, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #366 [ref=1x]
  { F(UseR)|F(FixedReg)|F(Vec)|F(Vex)                     , 0  , 0  , 399, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #367 [ref=1x]
  { F(UseR)|F(FixedReg)|F(Vec)|F(Vex)                     , 0  , 0  , 400, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #368 [ref=1x]
  { F(UseW)|F(Vec)|F(Evex)|F(Avx512K_B64)                 , 0  , 0  , 212, 3 , CONTROL(None)   , SINGLE_REG(WO)  , 0                   , 0 }, // #369 [ref=4x]
  { F(UseW)|F(Vec)|F(Evex)|F(Avx512KZ_B32)                , 0  , 0  , 182, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #370 [ref=6x]
  { F(UseW)|F(Vec)|F(Vex)                                 , 0  , 0  , 171, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #371 [ref=2x]
  { F(UseW)|F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_B32)         , 0  , 0  , 168, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #372 [ref=1x]
  { F(UseX)|F(Vec)|F(Evex)|F(Avx512KZ_B32)                , 0  , 0  , 167, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #373 [ref=5x]
  { F(UseX)|F(Vec)|F(Evex)|F(Avx512KZ_B64)                , 0  , 0  , 167, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #374 [ref=7x]
  { F(UseX)|F(Vec)|F(Evex)|F(Avx512KZ)                    , 0  , 0  , 167, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #375 [ref=4x]
  { F(UseW)|F(Vec)|F(Vex)                                 , 0  , 0  , 143, 4 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #376 [ref=2x]
  { F(UseW)|F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_B64)         , 0  , 0  , 77 , 6 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #377 [ref=2x]
  { F(UseW)|F(Vec)|F(Vex)                                 , 0  , 0  , 147, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #378 [ref=1x]
  { F(UseW)|F(Vec)|F(Vex)                                 , 0  , 0  , 148, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #379 [ref=1x]
  { F(UseW)|F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_B64)         , 0  , 0  , 147, 4 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #380 [ref=1x]
  { F(UseW)|F(Vec)|F(Vex)|F(Evex)                         , 0  , 0  , 401, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #381 [ref=1x]
  { F(UseW)|F(Vec)|F(Vex)|F(Evex)                         , 0  , 0  , 402, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #382 [ref=1x]
  { F(UseW)|F(Vec)|F(Vex)|F(Evex)                         , 0  , 0  , 303, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #383 [ref=1x]
  { F(UseW)|F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)             , 0  , 0  , 444, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #384 [ref=1x]
  { F(UseW)|F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)             , 0  , 0  , 445, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #385 [ref=1x]
  { F(UseW)|F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)             , 0  , 0  , 446, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #386 [ref=1x]
  { F(UseW)|F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)             , 0  , 0  , 447, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #387 [ref=1x]
  { F(UseW)|F(Vec)|F(Evex)|F(Avx512KZ_B64)                , 0  , 0  , 182, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #388 [ref=4x]
  { F(UseW)|F(Vec)|F(Vex)                                 , 0  , 0  , 310, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #389 [ref=12x]
  { F(UseW)|F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)             , 0  , 0  , 167, 3 , CONTROL(None)   , SINGLE_REG(RO)  , 0                   , 0 }, // #390 [ref=8x]
  { F(UseW)|F(Vec)|F(Evex)                                , 0  , 0  , 448, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #391 [ref=4x]
  { F(UseW)|F(Vec)|F(Evex)|F(Avx512KZ)                    , 0  , 0  , 218, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #392 [ref=6x]
  { F(UseW)|F(Vec)|F(Evex)|F(Avx512KZ)                    , 0  , 0  , 221, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #393 [ref=9x]
  { F(UseW)|F(Vec)|F(Evex)|F(Avx512KZ)                    , 0  , 0  , 224, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #394 [ref=3x]
  { F(UseW)|F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)             , 0  , 0  , 227, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #395 [ref=4x]
  { F(UseW)|F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)             , 0  , 0  , 230, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #396 [ref=2x]
  { F(UseW)|F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)             , 0  , 0  , 179, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #397 [ref=6x]
  { F(UseW)|F(Vec)|F(Vex)                                 , 0  , 0  , 131, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #398 [ref=1x]
  { F(UseW)|F(Vec)|F(Evex)|F(Avx512KZ_B32)                , 0  , 0  , 188, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #399 [ref=3x]
  { F(UseW)|F(Vec)|F(Evex)|F(Avx512KZ_B64)                , 0  , 0  , 188, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #400 [ref=3x]
  { F(UseW)|F(Vec)|F(Vex)                                 , 0  , 0  , 326, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #401 [ref=4x]
  { F(UseW)|F(Vec)|F(Vsib)|F(Evex)|F(Avx512K)             , 0  , 0  , 233, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #402 [ref=3x]
  { F(UseW)|F(Vec)|F(Vsib)|F(Evex)|F(Avx512K)             , 0  , 0  , 328, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #403 [ref=2x]
  { F(UseW)|F(Vec)|F(Vsib)|F(Evex)|F(Avx512K)             , 0  , 0  , 236, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #404 [ref=2x]
  { F(UseW)|F(Vec)|F(Vex)                                 , 0  , 0  , 330, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #405 [ref=8x]
  { F(UseW)|F(Vec)|F(Evex)|F(Avx512K)                     , 0  , 0  , 239, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #406 [ref=5x]
  { F(UseW)|F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_B32)         , 0  , 0  , 188, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #407 [ref=1x]
  { F(UseW)|F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)             , 0  , 0  , 188, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #408 [ref=2x]
  { F(UseW)|F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_B32)         , 0  , 0  , 83 , 6 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #409 [ref=3x]
  { F(UseW)|F(Vec)|F(Vex)|F(Evex)                         , 0  , 0  , 188, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #410 [ref=2x]
  { F(UseW)|F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_B64)         , 0  , 0  , 83 , 6 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #411 [ref=2x]
  { F(UseW)|F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)             , 0  , 0  , 83 , 6 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #412 [ref=3x]
  { F(UseW)|F(Vec)|F(Evex)|F(Avx512KZ_B64)                , 0  , 0  , 89 , 6 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #413 [ref=1x]
  { F(UseW)|F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ)             , 0  , 0  , 167, 3 , CONTROL(None)   , SINGLE_REG(WO)  , 0                   , 0 }, // #414 [ref=6x]
  { F(UseW)|F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_B32)         , 0  , 0  , 167, 3 , CONTROL(None)   , SINGLE_REG(WO)  , 0                   , 0 }, // #415 [ref=2x]
  { F(UseW)|F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_B64)         , 0  , 0  , 167, 3 , CONTROL(None)   , SINGLE_REG(WO)  , 0                   , 0 }, // #416 [ref=2x]
  { F(UseX)|F(Vec)|F(Evex)|F(Avx512KZ_B32)                , 0  , 0  , 170, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #417 [ref=1x]
  { F(UseX)|F(Vec)|F(Evex)|F(Avx512KZ_B64)                , 0  , 0  , 170, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #418 [ref=1x]
  { F(UseR)|F(Vec)|F(Vex)                                 , 0  , 0  , 182, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #419 [ref=3x]
  { F(UseW)|F(Vec)|F(Evex)|F(Avx512K_B32)                 , 0  , 0  , 239, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #420 [ref=2x]
  { F(UseW)|F(Vec)|F(Evex)|F(Avx512K_B64)                 , 0  , 0  , 239, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #421 [ref=2x]
  { F(UseW)|F(Vec)|F(Evex)|F(Avx512KZ_SAE_B64)            , 0  , 0  , 170, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #422 [ref=1x]
  { F(UseW)|F(Vec)|F(Evex)|F(Avx512KZ_SAE_B32)            , 0  , 0  , 170, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #423 [ref=1x]
  { F(UseW)|F(Vec)|F(Evex)|F(Avx512KZ)                    , 0  , 0  , 419, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #424 [ref=2x]
  { F(UseW)|F(Vec)|F(Evex)|F(Avx512KZ)                    , 0  , 0  , 420, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #425 [ref=2x]
  { F(UseW)|F(Vec)|F(Vex)                                 , 0  , 0  , 420, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #426 [ref=2x]
  { F(UseW)|F(Vec)|F(Evex)|F(Avx512KZ)                    , 0  , 0  , 430, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #427 [ref=1x]
  { F(UseW)|F(Vec)|F(Evex)|F(Avx512KZ)                    , 0  , 0  , 431, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #428 [ref=1x]
  { F(UseW)|F(Vec)|F(Vex)                                 , 0  , 0  , 188, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #429 [ref=2x]
  { F(UseW)|F(Vec)|F(Vex)                                 , 0  , 0  , 430, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #430 [ref=1x]
  { F(UseW)|F(Vec)|F(Vex)                                 , 0  , 0  , 431, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #431 [ref=1x]
  { F(UseW)|F(Vec)|F(Evex)|F(Avx512KZ_ER_SAE_B64)         , 0  , 0  , 167, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #432 [ref=1x]
  { F(UseW)|F(Vec)|F(Evex)|F(Avx512KZ_ER_SAE_B32)         , 0  , 0  , 167, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #433 [ref=1x]
  { F(UseW)|F(Vec)|F(Evex)|F(Avx512KZ_ER_SAE)             , 0  , 0  , 419, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #434 [ref=1x]
  { F(UseW)|F(Vec)|F(Evex)|F(Avx512KZ_ER_SAE)             , 0  , 0  , 420, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #435 [ref=1x]
  { F(UseW)|F(Vec)|F(Vsib)|F(Evex)|F(Avx512K)             , 0  , 0  , 332, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #436 [ref=1x]
  { F(UseW)|F(Vec)|F(Evex)|F(Avx512KZ_B32)                , 0  , 0  , 171, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #437 [ref=2x]
  { F(UseW)|F(Vec)|F(Evex)|F(Avx512KZ_B64)                , 0  , 0  , 171, 2 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #438 [ref=2x]
  { F(UseW)|F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_B32)         , 0  , 0  , 170, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #439 [ref=1x]
  { F(UseW)|F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_B64)         , 0  , 0  , 170, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #440 [ref=1x]
  { F(UseW)|F(Vec)|F(Vex)|F(Evex)|F(Avx512KZ_ER_SAE_B64)  , 0  , 0  , 182, 3 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #441 [ref=1x]
  { F(UseW)|F(Vex)                                        , 0  , 0  , 380, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #442 [ref=1x]
  { F(Volatile)|F(UseR)|F(Vec)|F(Vex)                     , 0  , 0  , 260, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #443 [ref=2x]
  { F(Volatile)|F(UseR)                                   , 0  , 0  , 410, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #444 [ref=2x]
  { F(Volatile)|F(Privileged)|F(UseR)|F(FixedReg)         , 0  , 0  , 411, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #445 [ref=2x]
  { F(UseX)|F(UseXX)|F(Lock)|F(XAcquire)|F(XRelease)      , 0  , 0  , 151, 4 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #446 [ref=1x]
  { F(Volatile)|F(UseR)                                   , 0  , 0  , 449, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #447 [ref=1x]
  { F(UseX)|F(UseXX)|F(Lock)                              , 0  , 0  , 50 , 8 , CONTROL(None)   , SINGLE_REG(RO)  , 0                   , 0 }, // #448 [ref=1x]
  { F(Volatile)|F(UseR)|F(FixedReg)                       , 0  , 0  , 450, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #449 [ref=2x]
  { F(Volatile)|F(UseR)|F(FixedReg)                       , 0  , 0  , 451, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #450 [ref=2x]
  { F(Volatile)|F(UseW)|F(FixedReg)                       , 0  , 0  , 450, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }, // #451 [ref=4x]
  { F(Volatile)|F(UseW)|F(FixedReg)                       , 0  , 0  , 451, 1 , CONTROL(None)   , SINGLE_REG(None), 0                   , 0 }  // #452 [ref=4x]
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
  { 0, { 0 }, 0, 0 }, // #0 [ref=67x]
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
  { 0, { FEATURE(SMAP) }, 0, SPECIAL(FLAGS_AC) }, // #17 [ref=2x]
  { 0, { 0 }, 0, SPECIAL(FLAGS_CF) }, // #18 [ref=2x]
  { 0, { 0 }, 0, SPECIAL(FLAGS_DF) }, // #19 [ref=2x]
  { 0, { FEATURE(CLFLUSH) }, 0, 0 }, // #20 [ref=1x]
  { 0, { FEATURE(CLFLUSHOPT) }, 0, 0 }, // #21 [ref=1x]
  { 0, { 0 }, 0, SPECIAL(FLAGS_IF) }, // #22 [ref=2x]
  { 0, { FEATURE(CLWB) }, 0, 0 }, // #23 [ref=1x]
  { 0, { FEATURE(CLZERO) }, 0, 0 }, // #24 [ref=1x]
  { 0, { 0 }, SPECIAL(FLAGS_CF), SPECIAL(FLAGS_CF) }, // #25 [ref=1x]
  { 0, { FEATURE(CMOV) }, SPECIAL(FLAGS_CF) | SPECIAL(FLAGS_ZF), 0 }, // #26 [ref=4x]
  { 0, { FEATURE(CMOV) }, SPECIAL(FLAGS_CF), 0 }, // #27 [ref=6x]
  { 0, { FEATURE(CMOV) }, SPECIAL(FLAGS_ZF), 0 }, // #28 [ref=4x]
  { 0, { FEATURE(CMOV) }, SPECIAL(FLAGS_OF) | SPECIAL(FLAGS_SF) | SPECIAL(FLAGS_ZF), 0 }, // #29 [ref=4x]
  { 0, { FEATURE(CMOV) }, SPECIAL(FLAGS_OF) | SPECIAL(FLAGS_SF), 0 }, // #30 [ref=4x]
  { 0, { FEATURE(CMOV) }, SPECIAL(FLAGS_OF), 0 }, // #31 [ref=2x]
  { 0, { FEATURE(CMOV) }, SPECIAL(FLAGS_PF), 0 }, // #32 [ref=4x]
  { 0, { FEATURE(CMOV) }, SPECIAL(FLAGS_SF), 0 }, // #33 [ref=2x]
  { 0, { 0 }, SPECIAL(FLAGS_DF), SPECIAL(FLAGS_AF) | SPECIAL(FLAGS_CF) | SPECIAL(FLAGS_OF) | SPECIAL(FLAGS_PF) | SPECIAL(FLAGS_SF) | SPECIAL(FLAGS_ZF) }, // #34 [ref=2x]
  { 0, { FEATURE(I486) }, 0, SPECIAL(FLAGS_AF) | SPECIAL(FLAGS_CF) | SPECIAL(FLAGS_OF) | SPECIAL(FLAGS_PF) | SPECIAL(FLAGS_SF) | SPECIAL(FLAGS_ZF) }, // #35 [ref=1x]
  { 0, { FEATURE(CMPXCHG16B) }, 0, SPECIAL(FLAGS_ZF) }, // #36 [ref=1x]
  { 0, { FEATURE(CMPXCHG8B) }, 0, 0 }, // #37 [ref=1x]
  { 0, { FEATURE(SSE2) }, 0, SPECIAL(FLAGS_AF) | SPECIAL(FLAGS_CF) | SPECIAL(FLAGS_OF) | SPECIAL(FLAGS_PF) | SPECIAL(FLAGS_SF) | SPECIAL(FLAGS_ZF) }, // #38 [ref=2x]
  { 0, { FEATURE(SSE) }, 0, SPECIAL(FLAGS_AF) | SPECIAL(FLAGS_CF) | SPECIAL(FLAGS_OF) | SPECIAL(FLAGS_PF) | SPECIAL(FLAGS_SF) | SPECIAL(FLAGS_ZF) }, // #39 [ref=2x]
  { 0, { FEATURE(I486) }, 0, 0 }, // #40 [ref=4x]
  { 0, { FEATURE(SSE4_2) }, 0, 0 }, // #41 [ref=2x]
  { 0, { 0 }, 0, SPECIAL(FLAGS_AF) | SPECIAL(FLAGS_OF) | SPECIAL(FLAGS_PF) | SPECIAL(FLAGS_SF) | SPECIAL(FLAGS_ZF) }, // #42 [ref=2x]
  { 0, { FEATURE(MMX) }, 0, 0 }, // #43 [ref=1x]
  { 0, { FEATURE(SSE4A) }, 0, 0 }, // #44 [ref=4x]
  { 0, { 0 }, 0, SPECIAL(X87SW_C0) | SPECIAL(X87SW_C1) | SPECIAL(X87SW_C2) | SPECIAL(X87SW_C3) }, // #45 [ref=80x]
  { 0, { FEATURE(CMOV) }, 0, SPECIAL(X87SW_C0) | SPECIAL(X87SW_C1) | SPECIAL(X87SW_C2) | SPECIAL(X87SW_C3) }, // #46 [ref=8x]
  { 0, { 0 }, 0, SPECIAL(FLAGS_CF) | SPECIAL(FLAGS_PF) | SPECIAL(FLAGS_ZF) | SPECIAL(X87SW_C1) }, // #47 [ref=4x]
  { 0, { FEATURE(3DNOW) }, 0, 0 }, // #48 [ref=21x]
  { 0, { FEATURE(SSE3) }, 0, SPECIAL(X87SW_C0) | SPECIAL(X87SW_C1) | SPECIAL(X87SW_C2) | SPECIAL(X87SW_C3) }, // #49 [ref=1x]
  { 0, { FEATURE(FXSR) }, 0, SPECIAL(X87SW_C0) | SPECIAL(X87SW_C1) | SPECIAL(X87SW_C2) | SPECIAL(X87SW_C3) }, // #50 [ref=2x]
  { 0, { FEATURE(FXSR) }, 0, 0 }, // #51 [ref=2x]
  { 0, { 0 }, SPECIAL(FLAGS_OF), 0 }, // #52 [ref=5x]
  { 0, { 0 }, SPECIAL(FLAGS_CF) | SPECIAL(FLAGS_ZF), 0 }, // #53 [ref=8x]
  { 0, { 0 }, SPECIAL(FLAGS_CF), 0 }, // #54 [ref=12x]
  { 0, { 0 }, SPECIAL(FLAGS_ZF), 0 }, // #55 [ref=10x]
  { 0, { 0 }, SPECIAL(FLAGS_OF) | SPECIAL(FLAGS_SF) | SPECIAL(FLAGS_ZF), 0 }, // #56 [ref=8x]
  { 0, { 0 }, SPECIAL(FLAGS_OF) | SPECIAL(FLAGS_SF), 0 }, // #57 [ref=8x]
  { 0, { 0 }, SPECIAL(FLAGS_PF), 0 }, // #58 [ref=8x]
  { 0, { 0 }, SPECIAL(FLAGS_SF), 0 }, // #59 [ref=4x]
  { 0, { FEATURE(AVX512_DQ) }, 0, 0 }, // #60 [ref=23x]
  { 0, { FEATURE(AVX512_BW) }, 0, 0 }, // #61 [ref=22x]
  { 0, { FEATURE(AVX512_F) }, 0, 0 }, // #62 [ref=37x]
  { 0, { FEATURE(AVX512_DQ) }, 0, SPECIAL(FLAGS_AF) | SPECIAL(FLAGS_CF) | SPECIAL(FLAGS_OF) | SPECIAL(FLAGS_PF) | SPECIAL(FLAGS_SF) | SPECIAL(FLAGS_ZF) }, // #63 [ref=3x]
  { 0, { FEATURE(AVX512_BW) }, 0, SPECIAL(FLAGS_AF) | SPECIAL(FLAGS_CF) | SPECIAL(FLAGS_OF) | SPECIAL(FLAGS_PF) | SPECIAL(FLAGS_SF) | SPECIAL(FLAGS_ZF) }, // #64 [ref=4x]
  { 0, { FEATURE(AVX512_F) }, 0, SPECIAL(FLAGS_AF) | SPECIAL(FLAGS_CF) | SPECIAL(FLAGS_OF) | SPECIAL(FLAGS_PF) | SPECIAL(FLAGS_SF) | SPECIAL(FLAGS_ZF) }, // #65 [ref=1x]
  { 0, { FEATURE(LAHFSAHF) }, SPECIAL(FLAGS_AF) | SPECIAL(FLAGS_CF) | SPECIAL(FLAGS_PF) | SPECIAL(FLAGS_SF) | SPECIAL(FLAGS_ZF), 0 }, // #66 [ref=1x]
  { 0, { 0 }, SPECIAL(FLAGS_DF), 0 }, // #67 [ref=3x]
  { 0, { FEATURE(LZCNT) }, 0, SPECIAL(FLAGS_AF) | SPECIAL(FLAGS_CF) | SPECIAL(FLAGS_OF) | SPECIAL(FLAGS_PF) | SPECIAL(FLAGS_SF) | SPECIAL(FLAGS_ZF) }, // #68 [ref=1x]
  { 0, { FEATURE(MMX2) }, 0, 0 }, // #69 [ref=8x]
  { 0, { FEATURE(MONITOR) }, 0, 0 }, // #70 [ref=2x]
  { 0, { FEATURE(MONITORX) }, 0, 0 }, // #71 [ref=2x]
  { 0, { FEATURE(MOVBE) }, 0, 0 }, // #72 [ref=1x]
  { 0, { FEATURE(MMX), FEATURE(SSE2) }, 0, 0 }, // #73 [ref=46x]
  { 0, { FEATURE(BMI2) }, 0, 0 }, // #74 [ref=7x]
  { 0, { 0 }, 0, SPECIAL(FLAGS_AF) | SPECIAL(FLAGS_CF) | SPECIAL(FLAGS_PF) | SPECIAL(FLAGS_ZF) }, // #75 [ref=1x]
  { 0, { FEATURE(SSSE3) }, 0, 0 }, // #76 [ref=15x]
  { 0, { FEATURE(MMX2), FEATURE(SSE2) }, 0, 0 }, // #77 [ref=10x]
  { 0, { FEATURE(PCLMULQDQ) }, 0, 0 }, // #78 [ref=1x]
  { 0, { FEATURE(SSE4_2) }, 0, SPECIAL(FLAGS_AF) | SPECIAL(FLAGS_CF) | SPECIAL(FLAGS_OF) | SPECIAL(FLAGS_PF) | SPECIAL(FLAGS_SF) | SPECIAL(FLAGS_ZF) }, // #79 [ref=4x]
  { 0, { FEATURE(PCOMMIT) }, 0, 0 }, // #80 [ref=1x]
  { 0, { FEATURE(MMX2), FEATURE(SSE2), FEATURE(SSE4_1) }, 0, 0 }, // #81 [ref=1x]
  { 0, { FEATURE(3DNOW2) }, 0, 0 }, // #82 [ref=5x]
  { 0, { FEATURE(GEODE) }, 0, 0 }, // #83 [ref=2x]
  { 0, { FEATURE(POPCNT) }, 0, SPECIAL(FLAGS_AF) | SPECIAL(FLAGS_CF) | SPECIAL(FLAGS_OF) | SPECIAL(FLAGS_PF) | SPECIAL(FLAGS_SF) | SPECIAL(FLAGS_ZF) }, // #84 [ref=1x]
  { 0, { FEATURE(PREFETCHW) }, 0, SPECIAL(FLAGS_AF) | SPECIAL(FLAGS_CF) | SPECIAL(FLAGS_OF) | SPECIAL(FLAGS_PF) | SPECIAL(FLAGS_SF) | SPECIAL(FLAGS_ZF) }, // #85 [ref=1x]
  { 0, { FEATURE(PREFETCHWT1) }, 0, SPECIAL(FLAGS_AF) | SPECIAL(FLAGS_CF) | SPECIAL(FLAGS_OF) | SPECIAL(FLAGS_PF) | SPECIAL(FLAGS_SF) | SPECIAL(FLAGS_ZF) }, // #86 [ref=1x]
  { 0, { FEATURE(SSE4_1) }, 0, SPECIAL(FLAGS_AF) | SPECIAL(FLAGS_CF) | SPECIAL(FLAGS_OF) | SPECIAL(FLAGS_PF) | SPECIAL(FLAGS_SF) | SPECIAL(FLAGS_ZF) }, // #87 [ref=1x]
  { 0, { 0 }, SPECIAL(FLAGS_CF) | SPECIAL(FLAGS_OF), SPECIAL(FLAGS_CF) | SPECIAL(FLAGS_OF) }, // #88 [ref=2x]
  { 0, { FEATURE(FSGSBASE) }, 0, 0 }, // #89 [ref=4x]
  { 0, { FEATURE(MSR) }, SPECIAL(MSR), 0 }, // #90 [ref=1x]
  { 0, { FEATURE(RDRAND) }, 0, SPECIAL(FLAGS_AF) | SPECIAL(FLAGS_CF) | SPECIAL(FLAGS_OF) | SPECIAL(FLAGS_PF) | SPECIAL(FLAGS_SF) | SPECIAL(FLAGS_ZF) }, // #91 [ref=1x]
  { 0, { FEATURE(RDSEED) }, 0, SPECIAL(FLAGS_AF) | SPECIAL(FLAGS_CF) | SPECIAL(FLAGS_OF) | SPECIAL(FLAGS_PF) | SPECIAL(FLAGS_SF) | SPECIAL(FLAGS_ZF) }, // #92 [ref=1x]
  { 0, { FEATURE(RDTSC) }, 0, 0 }, // #93 [ref=1x]
  { 0, { FEATURE(RDTSCP) }, 0, 0 }, // #94 [ref=1x]
  { 0, { 0 }, 0, SPECIAL(FLAGS_CF) | SPECIAL(FLAGS_OF) }, // #95 [ref=2x]
  { 0, { FEATURE(LAHFSAHF) }, 0, SPECIAL(FLAGS_AF) | SPECIAL(FLAGS_CF) | SPECIAL(FLAGS_PF) | SPECIAL(FLAGS_SF) | SPECIAL(FLAGS_ZF) }, // #96 [ref=1x]
  { 0, { FEATURE(SHA) }, 0, 0 }, // #97 [ref=7x]
  { 0, { FEATURE(AVX512_4FMAPS) }, 0, 0 }, // #98 [ref=4x]
  { 0, { FEATURE(AVX), FEATURE(AVX512_F), FEATURE(AVX512_VL) }, 0, 0 }, // #99 [ref=46x]
  { 0, { FEATURE(AVX), FEATURE(AVX512_F) }, 0, 0 }, // #100 [ref=32x]
  { 0, { FEATURE(AVX) }, 0, 0 }, // #101 [ref=37x]
  { 0, { FEATURE(AESNI), FEATURE(AVX), FEATURE(AVX512_F), FEATURE(AVX512_VL), FEATURE(VAES) }, 0, 0 }, // #102 [ref=4x]
  { 0, { FEATURE(AESNI), FEATURE(AVX) }, 0, 0 }, // #103 [ref=2x]
  { 0, { FEATURE(AVX512_F), FEATURE(AVX512_VL) }, 0, 0 }, // #104 [ref=112x]
  { 0, { FEATURE(AVX), FEATURE(AVX512_DQ), FEATURE(AVX512_VL) }, 0, 0 }, // #105 [ref=8x]
  { 0, { FEATURE(AVX512_BW), FEATURE(AVX512_VL) }, 0, 0 }, // #106 [ref=26x]
  { 0, { FEATURE(AVX512_DQ), FEATURE(AVX512_VL) }, 0, 0 }, // #107 [ref=30x]
  { 0, { FEATURE(AVX2) }, 0, 0 }, // #108 [ref=9x]
  { 0, { FEATURE(AVX), FEATURE(AVX2), FEATURE(AVX512_F), FEATURE(AVX512_VL) }, 0, 0 }, // #109 [ref=39x]
  { 0, { FEATURE(AVX), FEATURE(AVX512_F) }, 0, SPECIAL(FLAGS_AF) | SPECIAL(FLAGS_CF) | SPECIAL(FLAGS_OF) | SPECIAL(FLAGS_PF) | SPECIAL(FLAGS_SF) | SPECIAL(FLAGS_ZF) }, // #110 [ref=4x]
  { 0, { FEATURE(AVX512_F), FEATURE(AVX512_VL), FEATURE(F16C) }, 0, 0 }, // #111 [ref=2x]
  { 0, { FEATURE(AVX512_ERI) }, 0, 0 }, // #112 [ref=10x]
  { 0, { FEATURE(AVX512_F), FEATURE(AVX512_VL), FEATURE(FMA) }, 0, 0 }, // #113 [ref=36x]
  { 0, { FEATURE(AVX512_F), FEATURE(FMA) }, 0, 0 }, // #114 [ref=24x]
  { 0, { FEATURE(FMA4) }, 0, 0 }, // #115 [ref=20x]
  { 0, { FEATURE(XOP) }, 0, 0 }, // #116 [ref=55x]
  { 0, { FEATURE(AVX2), FEATURE(AVX512_F), FEATURE(AVX512_VL) }, 0, 0 }, // #117 [ref=17x]
  { 0, { FEATURE(AVX512_PFI) }, 0, 0 }, // #118 [ref=16x]
  { 0, { FEATURE(AVX), FEATURE(AVX2) }, 0, 0 }, // #119 [ref=17x]
  { 0, { FEATURE(AVX512_4VNNIW) }, 0, 0 }, // #120 [ref=2x]
  { 0, { FEATURE(AVX), FEATURE(AVX2), FEATURE(AVX512_BW), FEATURE(AVX512_VL) }, 0, 0 }, // #121 [ref=54x]
  { 0, { FEATURE(AVX2), FEATURE(AVX512_BW), FEATURE(AVX512_VL) }, 0, 0 }, // #122 [ref=2x]
  { 0, { FEATURE(AVX512_CDI), FEATURE(AVX512_VL) }, 0, 0 }, // #123 [ref=6x]
  { 0, { FEATURE(AVX), FEATURE(AVX512_F), FEATURE(AVX512_VL), FEATURE(PCLMULQDQ), FEATURE(VPCLMULQDQ) }, 0, 0 }, // #124 [ref=1x]
  { 0, { FEATURE(AVX) }, 0, SPECIAL(FLAGS_AF) | SPECIAL(FLAGS_CF) | SPECIAL(FLAGS_OF) | SPECIAL(FLAGS_PF) | SPECIAL(FLAGS_SF) | SPECIAL(FLAGS_ZF) }, // #125 [ref=7x]
  { 0, { FEATURE(AVX512_VBMI2), FEATURE(AVX512_VL) }, 0, 0 }, // #126 [ref=16x]
  { 0, { FEATURE(AVX512_VBMI), FEATURE(AVX512_VL) }, 0, 0 }, // #127 [ref=4x]
  { 0, { FEATURE(AVX), FEATURE(AVX512_BW) }, 0, 0 }, // #128 [ref=4x]
  { 0, { FEATURE(AVX), FEATURE(AVX512_DQ) }, 0, 0 }, // #129 [ref=4x]
  { 0, { FEATURE(AVX512_IFMA), FEATURE(AVX512_VL) }, 0, 0 }, // #130 [ref=2x]
  { 0, { FEATURE(AVX512_BITALG), FEATURE(AVX512_VL) }, 0, 0 }, // #131 [ref=3x]
  { 0, { FEATURE(AVX512_VL), FEATURE(AVX512_VPOPCNTDQ) }, 0, 0 }, // #132 [ref=2x]
  { 0, { FEATURE(MSR) }, 0, SPECIAL(MSR) }, // #133 [ref=1x]
  { 0, { FEATURE(RTM) }, 0, 0 }, // #134 [ref=3x]
  { 0, { FEATURE(I486) }, 0, SPECIAL(FLAGS_AF) | SPECIAL(FLAGS_CF) | SPECIAL(FLAGS_PF) | SPECIAL(FLAGS_SF) | SPECIAL(FLAGS_ZF) }, // #135 [ref=1x]
  { 0, { FEATURE(XSAVE) }, SPECIAL(XCR), 0 }, // #136 [ref=5x]
  { 0, { FEATURE(XSAVES) }, SPECIAL(XCR), 0 }, // #137 [ref=4x]
  { 0, { FEATURE(XSAVEC) }, SPECIAL(XCR), 0 }, // #138 [ref=2x]
  { 0, { FEATURE(XSAVEOPT) }, SPECIAL(XCR), 0 }, // #139 [ref=2x]
  { 0, { FEATURE(XSAVE) }, 0, SPECIAL(XCR) }, // #140 [ref=1x]
  { 0, { FEATURE(TSX) }, 0, SPECIAL(FLAGS_AF) | SPECIAL(FLAGS_CF) | SPECIAL(FLAGS_OF) | SPECIAL(FLAGS_PF) | SPECIAL(FLAGS_SF) | SPECIAL(FLAGS_ZF) }  // #141 [ref=1x]
};
#undef SPECIAL
#undef FEATURE
#undef OP_FLAG
// ----------------------------------------------------------------------------
// ${InstExecutionTable:End}

// ============================================================================
// [asmjit::Inst - Id <-> Name]
// ============================================================================

#ifndef ASMJIT_DISABLE_TEXT
// ${NameData:Begin}
// ------------------- Automatically generated, do not edit -------------------
const char InstDB::_nameData[] =
  "\0" "aaa\0" "aad\0" "aam\0" "aas\0" "adc\0" "adcx\0" "adox\0" "arpl\0" "bextr\0" "blcfill\0" "blci\0" "blcic\0"
  "blcmsk\0" "blcs\0" "blsfill\0" "blsi\0" "blsic\0" "blsmsk\0" "blsr\0" "bndcl\0" "bndcn\0" "bndcu\0" "bndldx\0"
  "bndmk\0" "bndmov\0" "bndstx\0" "bound\0" "bsf\0" "bsr\0" "bswap\0" "bt\0" "btc\0" "btr\0" "bts\0" "bzhi\0" "cbw\0"
  "cdq\0" "cdqe\0" "clac\0" "clc\0" "cld\0" "clflush\0" "clflushopt\0" "cli\0" "clts\0" "clwb\0" "clzero\0" "cmc\0"
  "cmova\0" "cmovae\0" "cmovc\0" "cmovg\0" "cmovge\0" "cmovl\0" "cmovle\0" "cmovna\0" "cmovnae\0" "cmovnc\0" "cmovng\0"
  "cmovnge\0" "cmovnl\0" "cmovnle\0" "cmovno\0" "cmovnp\0" "cmovns\0" "cmovnz\0" "cmovo\0" "cmovp\0" "cmovpe\0"
  "cmovpo\0" "cmovs\0" "cmovz\0" "cmp\0" "cmps\0" "cmpxchg\0" "cmpxchg16b\0" "cmpxchg8b\0" "cpuid\0" "cqo\0" "crc32\0"
  "cvtpd2pi\0" "cvtpi2pd\0" "cvtpi2ps\0" "cvtps2pi\0" "cvttpd2pi\0" "cvttps2pi\0" "cwd\0" "cwde\0" "daa\0" "das\0"
  "f2xm1\0" "fabs\0" "faddp\0" "fbld\0" "fbstp\0" "fchs\0" "fclex\0" "fcmovb\0" "fcmovbe\0" "fcmove\0" "fcmovnb\0"
  "fcmovnbe\0" "fcmovne\0" "fcmovnu\0" "fcmovu\0" "fcom\0" "fcomi\0" "fcomip\0" "fcomp\0" "fcompp\0" "fcos\0"
  "fdecstp\0" "fdiv\0" "fdivp\0" "fdivr\0" "fdivrp\0" "femms\0" "ffree\0" "fiadd\0" "ficom\0" "ficomp\0" "fidiv\0"
  "fidivr\0" "fild\0" "fimul\0" "fincstp\0" "finit\0" "fist\0" "fistp\0" "fisttp\0" "fisub\0" "fisubr\0" "fld\0"
  "fld1\0" "fldcw\0" "fldenv\0" "fldl2e\0" "fldl2t\0" "fldlg2\0" "fldln2\0" "fldpi\0" "fldz\0" "fmulp\0" "fnclex\0"
  "fninit\0" "fnop\0" "fnsave\0" "fnstcw\0" "fnstenv\0" "fnstsw\0" "fpatan\0" "fprem\0" "fprem1\0" "fptan\0"
  "frndint\0" "frstor\0" "fsave\0" "fscale\0" "fsin\0" "fsincos\0" "fsqrt\0" "fst\0" "fstcw\0" "fstenv\0" "fstp\0"
  "fstsw\0" "fsubp\0" "fsubrp\0" "ftst\0" "fucom\0" "fucomi\0" "fucomip\0" "fucomp\0" "fucompp\0" "fwait\0" "fxam\0"
  "fxch\0" "fxrstor\0" "fxrstor64\0" "fxsave\0" "fxsave64\0" "fxtract\0" "fyl2x\0" "fyl2xp1\0" "hlt\0" "inc\0" "ins\0"
  "insertq\0" "int3\0" "into\0" "invlpg\0" "invpcid\0" "iret\0" "iretd\0" "iretq\0" "iretw\0" "ja\0" "jae\0" "jb\0"
  "jbe\0" "jc\0" "je\0" "jecxz\0" "jg\0" "jge\0" "jl\0" "jle\0" "jmp\0" "jna\0" "jnae\0" "jnb\0" "jnbe\0" "jnc\0"
  "jne\0" "jng\0" "jnge\0" "jnl\0" "jnle\0" "jno\0" "jnp\0" "jns\0" "jnz\0" "jo\0" "jp\0" "jpe\0" "jpo\0" "js\0" "jz\0"
  "kaddb\0" "kaddd\0" "kaddq\0" "kaddw\0" "kandb\0" "kandd\0" "kandnb\0" "kandnd\0" "kandnq\0" "kandnw\0" "kandq\0"
  "kandw\0" "kmovb\0" "kmovw\0" "knotb\0" "knotd\0" "knotq\0" "knotw\0" "korb\0" "kord\0" "korq\0" "kortestb\0"
  "kortestd\0" "kortestq\0" "kortestw\0" "korw\0" "kshiftlb\0" "kshiftld\0" "kshiftlq\0" "kshiftlw\0" "kshiftrb\0"
  "kshiftrd\0" "kshiftrq\0" "kshiftrw\0" "ktestb\0" "ktestd\0" "ktestq\0" "ktestw\0" "kunpckbw\0" "kunpckdq\0"
  "kunpckwd\0" "kxnorb\0" "kxnord\0" "kxnorq\0" "kxnorw\0" "kxorb\0" "kxord\0" "kxorq\0" "kxorw\0" "lahf\0" "lar\0"
  "lds\0" "lea\0" "leave\0" "les\0" "lfence\0" "lfs\0" "lgdt\0" "lgs\0" "lidt\0" "lldt\0" "lmsw\0" "lods\0" "loop\0"
  "loope\0" "loopne\0" "lsl\0" "ltr\0" "lzcnt\0" "mfence\0" "monitor\0" "monitorx\0" "movdq2q\0" "movnti\0" "movntq\0"
  "movntsd\0" "movntss\0" "movq2dq\0" "movsx\0" "movsxd\0" "movzx\0" "mulx\0" "mwait\0" "mwaitx\0" "neg\0" "not\0"
  "out\0" "outs\0" "pause\0" "pavgusb\0" "pcommit\0" "pdep\0" "pext\0" "pf2id\0" "pf2iw\0" "pfacc\0" "pfadd\0"
  "pfcmpeq\0" "pfcmpge\0" "pfcmpgt\0" "pfmax\0" "pfmin\0" "pfmul\0" "pfnacc\0" "pfpnacc\0" "pfrcp\0" "pfrcpit1\0"
  "pfrcpit2\0" "pfrcpv\0" "pfrsqit1\0" "pfrsqrt\0" "pfrsqrtv\0" "pfsub\0" "pfsubr\0" "pi2fd\0" "pi2fw\0" "pmulhrw\0"
  "pop\0" "popa\0" "popad\0" "popcnt\0" "popf\0" "popfd\0" "popfq\0" "prefetch\0" "prefetchnta\0" "prefetcht0\0"
  "prefetcht1\0" "prefetcht2\0" "prefetchw\0" "prefetchwt1\0" "pshufw\0" "pswapd\0" "push\0" "pusha\0" "pushad\0"
  "pushf\0" "pushfd\0" "pushfq\0" "rcl\0" "rcr\0" "rdfsbase\0" "rdgsbase\0" "rdmsr\0" "rdpmc\0" "rdrand\0" "rdseed\0"
  "rdtsc\0" "rdtscp\0" "rol\0" "ror\0" "rorx\0" "rsm\0" "sahf\0" "sal\0" "sar\0" "sarx\0" "sbb\0" "scas\0" "seta\0"
  "setae\0" "setb\0" "setbe\0" "setc\0" "sete\0" "setg\0" "setge\0" "setl\0" "setle\0" "setna\0" "setnae\0" "setnb\0"
  "setnbe\0" "setnc\0" "setne\0" "setng\0" "setnge\0" "setnl\0" "setnle\0" "setno\0" "setnp\0" "setns\0" "setnz\0"
  "seto\0" "setp\0" "setpe\0" "setpo\0" "sets\0" "setz\0" "sfence\0" "sgdt\0" "sha1msg1\0" "sha1msg2\0" "sha1nexte\0"
  "sha1rnds4\0" "sha256msg1\0" "sha256msg2\0" "sha256rnds2\0" "shl\0" "shlx\0" "shr\0" "shrd\0" "shrx\0" "sidt\0"
  "sldt\0" "smsw\0" "stac\0" "stc\0" "sti\0" "stos\0" "str\0" "swapgs\0" "syscall\0" "sysenter\0" "sysexit\0"
  "sysexit64\0" "sysret\0" "sysret64\0" "t1mskc\0" "tzcnt\0" "tzmsk\0" "ud2\0" "v4fmaddps\0" "v4fmaddss\0"
  "v4fnmaddps\0" "v4fnmaddss\0" "vaddpd\0" "vaddps\0" "vaddsd\0" "vaddss\0" "vaddsubpd\0" "vaddsubps\0" "vaesdec\0"
  "vaesdeclast\0" "vaesenc\0" "vaesenclast\0" "vaesimc\0" "vaeskeygenassist\0" "valignd\0" "valignq\0" "vandnpd\0"
  "vandnps\0" "vandpd\0" "vandps\0" "vblendmb\0" "vblendmd\0" "vblendmpd\0" "vblendmps\0" "vblendmq\0" "vblendmw\0"
  "vblendpd\0" "vblendps\0" "vblendvpd\0" "vblendvps\0" "vbroadcastf128\0" "vbroadcastf32x2\0" "vbroadcastf32x4\0"
  "vbroadcastf32x8\0" "vbroadcastf64x2\0" "vbroadcastf64x4\0" "vbroadcasti128\0" "vbroadcasti32x2\0"
  "vbroadcasti32x4\0" "vbroadcasti32x8\0" "vbroadcasti64x2\0" "vbroadcasti64x4\0" "vbroadcastsd\0" "vbroadcastss\0"
  "vcmppd\0" "vcmpps\0" "vcmpsd\0" "vcmpss\0" "vcomisd\0" "vcomiss\0" "vcompresspd\0" "vcompressps\0" "vcvtdq2pd\0"
  "vcvtdq2ps\0" "vcvtpd2dq\0" "vcvtpd2ps\0" "vcvtpd2qq\0" "vcvtpd2udq\0" "vcvtpd2uqq\0" "vcvtph2ps\0" "vcvtps2dq\0"
  "vcvtps2pd\0" "vcvtps2ph\0" "vcvtps2qq\0" "vcvtps2udq\0" "vcvtps2uqq\0" "vcvtqq2pd\0" "vcvtqq2ps\0" "vcvtsd2si\0"
  "vcvtsd2ss\0" "vcvtsd2usi\0" "vcvtsi2sd\0" "vcvtsi2ss\0" "vcvtss2sd\0" "vcvtss2si\0" "vcvtss2usi\0" "vcvttpd2dq\0"
  "vcvttpd2qq\0" "vcvttpd2udq\0" "vcvttpd2uqq\0" "vcvttps2dq\0" "vcvttps2qq\0" "vcvttps2udq\0" "vcvttps2uqq\0"
  "vcvttsd2si\0" "vcvttsd2usi\0" "vcvttss2si\0" "vcvttss2usi\0" "vcvtudq2pd\0" "vcvtudq2ps\0" "vcvtuqq2pd\0"
  "vcvtuqq2ps\0" "vcvtusi2sd\0" "vcvtusi2ss\0" "vdbpsadbw\0" "vdivpd\0" "vdivps\0" "vdivsd\0" "vdivss\0" "vdppd\0"
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
  "vgetexpsd\0" "vgetexpss\0" "vgetmantpd\0" "vgetmantps\0" "vgetmantsd\0" "vgetmantss\0" "vhaddpd\0" "vhaddps\0"
  "vhsubpd\0" "vhsubps\0" "vinsertf128\0" "vinsertf32x4\0" "vinsertf32x8\0" "vinsertf64x2\0" "vinsertf64x4\0"
  "vinserti128\0" "vinserti32x4\0" "vinserti32x8\0" "vinserti64x2\0" "vinserti64x4\0" "vinsertps\0" "vlddqu\0"
  "vldmxcsr\0" "vmaskmovdqu\0" "vmaskmovpd\0" "vmaskmovps\0" "vmaxpd\0" "vmaxps\0" "vmaxsd\0" "vmaxss\0" "vminpd\0"
  "vminps\0" "vminsd\0" "vminss\0" "vmovapd\0" "vmovaps\0" "vmovd\0" "vmovddup\0" "vmovdqa\0" "vmovdqa32\0"
  "vmovdqa64\0" "vmovdqu\0" "vmovdqu16\0" "vmovdqu32\0" "vmovdqu64\0" "vmovdqu8\0" "vmovhlps\0" "vmovhpd\0" "vmovhps\0"
  "vmovlhps\0" "vmovlpd\0" "vmovlps\0" "vmovmskpd\0" "vmovmskps\0" "vmovntdq\0" "vmovntdqa\0" "vmovntpd\0" "vmovntps\0"
  "vmovq\0" "vmovsd\0" "vmovshdup\0" "vmovsldup\0" "vmovss\0" "vmovupd\0" "vmovups\0" "vmpsadbw\0" "vmulpd\0"
  "vmulps\0" "vmulsd\0" "vmulss\0" "vorpd\0" "vorps\0" "vp4dpwssd\0" "vp4dpwssds\0" "vpabsb\0" "vpabsd\0" "vpabsq\0"
  "vpabsw\0" "vpackssdw\0" "vpacksswb\0" "vpackusdw\0" "vpackuswb\0" "vpaddb\0" "vpaddd\0" "vpaddq\0" "vpaddsb\0"
  "vpaddsw\0" "vpaddusb\0" "vpaddusw\0" "vpaddw\0" "vpalignr\0" "vpand\0" "vpandd\0" "vpandn\0" "vpandnd\0" "vpandnq\0"
  "vpandq\0" "vpavgb\0" "vpavgw\0" "vpblendd\0" "vpblendvb\0" "vpblendw\0" "vpbroadcastb\0" "vpbroadcastd\0"
  "vpbroadcastmb2d\0" "vpbroadcastmb2q\0" "vpbroadcastq\0" "vpbroadcastw\0" "vpclmulqdq\0" "vpcmov\0" "vpcmpb\0"
  "vpcmpd\0" "vpcmpeqb\0" "vpcmpeqd\0" "vpcmpeqq\0" "vpcmpeqw\0" "vpcmpestri\0" "vpcmpestrm\0" "vpcmpgtb\0"
  "vpcmpgtd\0" "vpcmpgtq\0" "vpcmpgtw\0" "vpcmpistri\0" "vpcmpistrm\0" "vpcmpq\0" "vpcmpub\0" "vpcmpud\0" "vpcmpuq\0"
  "vpcmpuw\0" "vpcmpw\0" "vpcomb\0" "vpcomd\0" "vpcompressb\0" "vpcompressd\0" "vpcompressq\0" "vpcompressw\0"
  "vpcomq\0" "vpcomub\0" "vpcomud\0" "vpcomuq\0" "vpcomuw\0" "vpcomw\0" "vpconflictd\0" "vpconflictq\0" "vperm2f128\0"
  "vperm2i128\0" "vpermb\0" "vpermd\0" "vpermi2b\0" "vpermi2d\0" "vpermi2pd\0" "vpermi2ps\0" "vpermi2q\0" "vpermi2w\0"
  "vpermil2pd\0" "vpermil2ps\0" "vpermilpd\0" "vpermilps\0" "vpermpd\0" "vpermps\0" "vpermq\0" "vpermt2b\0"
  "vpermt2d\0" "vpermt2pd\0" "vpermt2ps\0" "vpermt2q\0" "vpermt2w\0" "vpermw\0" "vpexpandb\0" "vpexpandd\0"
  "vpexpandq\0" "vpexpandw\0" "vpextrb\0" "vpextrd\0" "vpextrq\0" "vpextrw\0" "vpgatherdd\0" "vpgatherdq\0"
  "vpgatherqd\0" "vpgatherqq\0" "vphaddbd\0" "vphaddbq\0" "vphaddbw\0" "vphaddd\0" "vphadddq\0" "vphaddsw\0"
  "vphaddubd\0" "vphaddubq\0" "vphaddubw\0" "vphaddudq\0" "vphadduwd\0" "vphadduwq\0" "vphaddw\0" "vphaddwd\0"
  "vphaddwq\0" "vphminposuw\0" "vphsubbw\0" "vphsubd\0" "vphsubdq\0" "vphsubsw\0" "vphsubw\0" "vphsubwd\0" "vpinsrb\0"
  "vpinsrd\0" "vpinsrq\0" "vpinsrw\0" "vplzcntd\0" "vplzcntq\0" "vpmacsdd\0" "vpmacsdqh\0" "vpmacsdql\0" "vpmacssdd\0"
  "vpmacssdqh\0" "vpmacssdql\0" "vpmacsswd\0" "vpmacssww\0" "vpmacswd\0" "vpmacsww\0" "vpmadcsswd\0" "vpmadcswd\0"
  "vpmadd52huq\0" "vpmadd52luq\0" "vpmaddubsw\0" "vpmaddwd\0" "vpmaskmovd\0" "vpmaskmovq\0" "vpmaxsb\0" "vpmaxsd\0"
  "vpmaxsq\0" "vpmaxsw\0" "vpmaxub\0" "vpmaxud\0" "vpmaxuq\0" "vpmaxuw\0" "vpminsb\0" "vpminsd\0" "vpminsq\0"
  "vpminsw\0" "vpminub\0" "vpminud\0" "vpminuq\0" "vpminuw\0" "vpmovb2m\0" "vpmovd2m\0" "vpmovdb\0" "vpmovdw\0"
  "vpmovm2b\0" "vpmovm2d\0" "vpmovm2q\0" "vpmovm2w\0" "vpmovmskb\0" "vpmovq2m\0" "vpmovqb\0" "vpmovqd\0" "vpmovqw\0"
  "vpmovsdb\0" "vpmovsdw\0" "vpmovsqb\0" "vpmovsqd\0" "vpmovsqw\0" "vpmovswb\0" "vpmovsxbd\0" "vpmovsxbq\0"
  "vpmovsxbw\0" "vpmovsxdq\0" "vpmovsxwd\0" "vpmovsxwq\0" "vpmovusdb\0" "vpmovusdw\0" "vpmovusqb\0" "vpmovusqd\0"
  "vpmovusqw\0" "vpmovuswb\0" "vpmovw2m\0" "vpmovwb\0" "vpmovzxbd\0" "vpmovzxbq\0" "vpmovzxbw\0" "vpmovzxdq\0"
  "vpmovzxwd\0" "vpmovzxwq\0" "vpmuldq\0" "vpmulhrsw\0" "vpmulhuw\0" "vpmulhw\0" "vpmulld\0" "vpmullq\0" "vpmullw\0"
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
  "vxorpd\0" "vxorps\0" "vzeroall\0" "vzeroupper\0" "wbinvd\0" "wrfsbase\0" "wrgsbase\0" "wrmsr\0" "xabort\0" "xadd\0"
  "xbegin\0" "xend\0" "xgetbv\0" "xlatb\0" "xrstors\0" "xrstors64\0" "xsavec\0" "xsavec64\0" "xsaveopt\0"
  "xsaveopt64\0" "xsaves\0" "xsaves64\0" "xsetbv\0" "xtest";

enum {
  kX86InstMaxSize = 16
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
  { Inst::kIdNone         , Inst::kIdNone          + 1 },
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

  if (nameSize == Globals::kNullTerminated)
    nameSize = std::strlen(name);

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
    int result = StringUtils::cmpInstName(nameData + cur[0]._nameDataIndex, name, nameSize);

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
  { count, (x86 ? uint8_t(InstDB::kArchMaskX86) : uint8_t(0)) | \
           (x64 ? uint8_t(InstDB::kArchMaskX64) : uint8_t(0)) , \
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
  ROW(2, 1, 1, 0, 21 , 22 , 0  , 0  , 0  , 0  ), //      {m16|mem|m64, sreg}
  ROW(2, 1, 1, 0, 22 , 21 , 0  , 0  , 0  , 0  ), //      {sreg, m16|mem|m64}
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
  ROW(2, 1, 1, 0, 4  , 29 , 0  , 0  , 0  , 0  ), //      {r16, m16|mem}
  ROW(2, 1, 1, 0, 6  , 30 , 0  , 0  , 0  , 0  ), //      {r32, m32|mem}
  ROW(2, 0, 1, 0, 8  , 31 , 0  , 0  , 0  , 0  ), //      {r64, m64|mem}
  ROW(2, 1, 1, 0, 32 , 10 , 0  , 0  , 0  , 0  ), // #28  {r8lo|r8hi|m8|r16|m16|r32|m32|r64|m64|mem, i8|u8}
  ROW(2, 1, 1, 0, 11 , 12 , 0  , 0  , 0  , 0  ), //      {r16|m16, i16|u16}
  ROW(2, 1, 1, 0, 13 , 14 , 0  , 0  , 0  , 0  ), //      {r32|m32, i32|u32}
  ROW(2, 0, 1, 0, 33 , 14 , 0  , 0  , 0  , 0  ), //      {rax, u32|i32}
  ROW(2, 0, 1, 0, 8  , 34 , 0  , 0  , 0  , 0  ), //      {r64, u32|i32|r64|m64|mem}
  ROW(2, 0, 1, 0, 31 , 24 , 0  , 0  , 0  , 0  ), //      {m64|mem, i32|r64}
  ROW(2, 1, 1, 0, 1  , 2  , 0  , 0  , 0  , 0  ), //      {r8lo|r8hi|m8|mem, r8lo|r8hi}
  ROW(2, 1, 1, 0, 27 , 4  , 0  , 0  , 0  , 0  ), //      {r16|m16|mem, r16}
  ROW(2, 1, 1, 0, 28 , 6  , 0  , 0  , 0  , 0  ), //      {r32|m32|mem, r32}
  ROW(2, 1, 1, 0, 2  , 18 , 0  , 0  , 0  , 0  ), //      {r8lo|r8hi, m8|mem}
  ROW(2, 1, 1, 0, 4  , 29 , 0  , 0  , 0  , 0  ), //      {r16, m16|mem}
  ROW(2, 1, 1, 0, 6  , 30 , 0  , 0  , 0  , 0  ), //      {r32, m32|mem}
  ROW(2, 1, 1, 1, 35 , 1  , 0  , 0  , 0  , 0  ), // #40  {<ax>, r8lo|r8hi|m8|mem}
  ROW(3, 1, 1, 2, 36 , 35 , 27 , 0  , 0  , 0  ), //      {<dx>, <ax>, r16|m16|mem}
  ROW(3, 1, 1, 2, 37 , 38 , 28 , 0  , 0  , 0  ), //      {<edx>, <eax>, r32|m32|mem}
  ROW(3, 0, 1, 2, 39 , 40 , 15 , 0  , 0  , 0  ), //      {<rdx>, <rax>, r64|m64|mem}
  ROW(2, 1, 1, 0, 4  , 41 , 0  , 0  , 0  , 0  ), //      {r16, r16|m16|mem|i8|i16}
  ROW(2, 1, 1, 0, 6  , 42 , 0  , 0  , 0  , 0  ), //      {r32, r32|m32|mem|i8|i32}
  ROW(2, 0, 1, 0, 8  , 43 , 0  , 0  , 0  , 0  ), //      {r64, r64|m64|mem|i8|i32}
  ROW(3, 1, 1, 0, 4  , 27 , 44 , 0  , 0  , 0  ), //      {r16, r16|m16|mem, i8|i16|u16}
  ROW(3, 1, 1, 0, 6  , 28 , 45 , 0  , 0  , 0  ), //      {r32, r32|m32|mem, i8|i32|u32}
  ROW(3, 0, 1, 0, 8  , 15 , 46 , 0  , 0  , 0  ), //      {r64, r64|m64|mem, i8|i32}
  ROW(2, 1, 1, 0, 27 , 4  , 0  , 0  , 0  , 0  ), // #50  {r16|m16|mem, r16}
  ROW(2, 1, 1, 0, 28 , 6  , 0  , 0  , 0  , 0  ), //      {r32|m32|mem, r32}
  ROW(2, 0, 1, 0, 15 , 8  , 0  , 0  , 0  , 0  ), //      {r64|m64|mem, r64}
  ROW(2, 1, 1, 0, 4  , 47 , 0  , 0  , 0  , 0  ), //      {r16, ax|m16|mem}
  ROW(2, 1, 1, 0, 6  , 48 , 0  , 0  , 0  , 0  ), //      {r32, eax|m32|mem}
  ROW(2, 0, 1, 0, 8  , 49 , 0  , 0  , 0  , 0  ), //      {r64, rax|m64|mem}
  ROW(2, 1, 1, 0, 1  , 2  , 0  , 0  , 0  , 0  ), //      {r8lo|r8hi|m8|mem, r8lo|r8hi}
  ROW(2, 1, 1, 0, 2  , 18 , 0  , 0  , 0  , 0  ), //      {r8lo|r8hi, m8|mem}
  ROW(2, 1, 1, 0, 9  , 10 , 0  , 0  , 0  , 0  ), // #58  {r8lo|r8hi|m8, i8|u8}
  ROW(2, 1, 1, 0, 11 , 12 , 0  , 0  , 0  , 0  ), //      {r16|m16, i16|u16}
  ROW(2, 1, 1, 0, 13 , 14 , 0  , 0  , 0  , 0  ), //      {r32|m32, i32|u32}
  ROW(2, 0, 1, 0, 15 , 24 , 0  , 0  , 0  , 0  ), //      {r64|m64|mem, i32|r64}
  ROW(2, 1, 1, 0, 1  , 2  , 0  , 0  , 0  , 0  ), //      {r8lo|r8hi|m8|mem, r8lo|r8hi}
  ROW(2, 1, 1, 0, 27 , 4  , 0  , 0  , 0  , 0  ), //      {r16|m16|mem, r16}
  ROW(2, 1, 1, 0, 28 , 6  , 0  , 0  , 0  , 0  ), //      {r32|m32|mem, r32}
  ROW(2, 1, 1, 0, 4  , 29 , 0  , 0  , 0  , 0  ), // #65  {r16, m16|mem}
  ROW(2, 1, 1, 0, 6  , 30 , 0  , 0  , 0  , 0  ), //      {r32, m32|mem}
  ROW(2, 0, 1, 0, 8  , 31 , 0  , 0  , 0  , 0  ), //      {r64, m64|mem}
  ROW(2, 1, 1, 0, 29 , 4  , 0  , 0  , 0  , 0  ), //      {m16|mem, r16}
  ROW(2, 1, 1, 0, 30 , 6  , 0  , 0  , 0  , 0  ), // #69  {m32|mem, r32}
  ROW(2, 0, 1, 0, 31 , 8  , 0  , 0  , 0  , 0  ), //      {m64|mem, r64}
  ROW(2, 1, 1, 0, 50 , 51 , 0  , 0  , 0  , 0  ), // #71  {xmm, xmm|m128|mem}
  ROW(2, 1, 1, 0, 52 , 50 , 0  , 0  , 0  , 0  ), // #72  {m128|mem, xmm}
  ROW(2, 1, 1, 0, 53 , 54 , 0  , 0  , 0  , 0  ), //      {ymm, ymm|m256|mem}
  ROW(2, 1, 1, 0, 55 , 53 , 0  , 0  , 0  , 0  ), //      {m256|mem, ymm}
  ROW(2, 1, 1, 0, 56 , 57 , 0  , 0  , 0  , 0  ), // #75  {zmm, zmm|m512|mem}
  ROW(2, 1, 1, 0, 58 , 56 , 0  , 0  , 0  , 0  ), //      {m512|mem, zmm}
  ROW(3, 1, 1, 0, 50 , 50 , 59 , 0  , 0  , 0  ), // #77  {xmm, xmm, xmm|m128|mem|u8}
  ROW(3, 1, 1, 0, 50 , 52 , 60 , 0  , 0  , 0  ), //      {xmm, m128|mem, u8}
  ROW(3, 1, 1, 0, 53 , 53 , 61 , 0  , 0  , 0  ), //      {ymm, ymm, ymm|m256|mem|u8}
  ROW(3, 1, 1, 0, 53 , 55 , 60 , 0  , 0  , 0  ), //      {ymm, m256|mem, u8}
  ROW(3, 1, 1, 0, 56 , 56 , 62 , 0  , 0  , 0  ), //      {zmm, zmm, zmm|m512|mem|u8}
  ROW(3, 1, 1, 0, 56 , 58 , 60 , 0  , 0  , 0  ), //      {zmm, m512|mem, u8}
  ROW(3, 1, 1, 0, 50 , 50 , 59 , 0  , 0  , 0  ), // #83  {xmm, xmm, u8|xmm|m128|mem}
  ROW(3, 1, 1, 0, 53 , 53 , 59 , 0  , 0  , 0  ), //      {ymm, ymm, u8|xmm|m128|mem}
  ROW(3, 1, 1, 0, 50 , 52 , 60 , 0  , 0  , 0  ), //      {xmm, m128|mem, u8}
  ROW(3, 1, 1, 0, 53 , 55 , 60 , 0  , 0  , 0  ), //      {ymm, m256|mem, u8}
  ROW(3, 1, 1, 0, 56 , 56 , 59 , 0  , 0  , 0  ), //      {zmm, zmm, xmm|m128|mem|u8}
  ROW(3, 1, 1, 0, 56 , 58 , 60 , 0  , 0  , 0  ), //      {zmm, m512|mem, u8}
  ROW(3, 1, 1, 0, 50 , 50 , 59 , 0  , 0  , 0  ), // #89  {xmm, xmm, xmm|m128|mem|u8}
  ROW(3, 1, 1, 0, 50 , 52 , 60 , 0  , 0  , 0  ), //      {xmm, m128|mem, u8}
  ROW(3, 1, 1, 0, 53 , 53 , 59 , 0  , 0  , 0  ), //      {ymm, ymm, xmm|m128|mem|u8}
  ROW(3, 1, 1, 0, 53 , 55 , 60 , 0  , 0  , 0  ), //      {ymm, m256|mem, u8}
  ROW(3, 1, 1, 0, 56 , 56 , 59 , 0  , 0  , 0  ), //      {zmm, zmm, xmm|m128|mem|u8}
  ROW(3, 1, 1, 0, 56 , 58 , 60 , 0  , 0  , 0  ), //      {zmm, m512|mem, u8}
  ROW(2, 1, 1, 0, 63 , 64 , 0  , 0  , 0  , 0  ), // #95  {mm, mm|m64|mem|r64}
  ROW(2, 1, 1, 0, 15 , 65 , 0  , 0  , 0  , 0  ), //      {m64|mem|r64, mm|xmm}
  ROW(2, 0, 1, 0, 50 , 15 , 0  , 0  , 0  , 0  ), //      {xmm, r64|m64|mem}
  ROW(2, 1, 1, 0, 50 , 66 , 0  , 0  , 0  , 0  ), // #98  {xmm, xmm|m64|mem}
  ROW(2, 1, 1, 0, 31 , 50 , 0  , 0  , 0  , 0  ), // #99  {m64|mem, xmm}
  ROW(3, 1, 1, 0, 50 , 67 , 50 , 0  , 0  , 0  ), // #100 {xmm, vm32x, xmm}
  ROW(3, 1, 1, 0, 53 , 67 , 53 , 0  , 0  , 0  ), //      {ymm, vm32x, ymm}
  ROW(2, 1, 1, 0, 50 , 67 , 0  , 0  , 0  , 0  ), //      {xmm, vm32x}
  ROW(2, 1, 1, 0, 53 , 68 , 0  , 0  , 0  , 0  ), //      {ymm, vm32y}
  ROW(2, 1, 1, 0, 56 , 69 , 0  , 0  , 0  , 0  ), //      {zmm, vm32z}
  ROW(3, 1, 1, 0, 50 , 67 , 50 , 0  , 0  , 0  ), // #105 {xmm, vm32x, xmm}
  ROW(3, 1, 1, 0, 53 , 68 , 53 , 0  , 0  , 0  ), //      {ymm, vm32y, ymm}
  ROW(2, 1, 1, 0, 50 , 67 , 0  , 0  , 0  , 0  ), //      {xmm, vm32x}
  ROW(2, 1, 1, 0, 53 , 68 , 0  , 0  , 0  , 0  ), //      {ymm, vm32y}
  ROW(2, 1, 1, 0, 56 , 69 , 0  , 0  , 0  , 0  ), //      {zmm, vm32z}
  ROW(3, 1, 1, 0, 50 , 70 , 50 , 0  , 0  , 0  ), // #110 {xmm, vm64x, xmm}
  ROW(3, 1, 1, 0, 53 , 71 , 53 , 0  , 0  , 0  ), //      {ymm, vm64y, ymm}
  ROW(2, 1, 1, 0, 50 , 70 , 0  , 0  , 0  , 0  ), //      {xmm, vm64x}
  ROW(2, 1, 1, 0, 53 , 71 , 0  , 0  , 0  , 0  ), //      {ymm, vm64y}
  ROW(2, 1, 1, 0, 56 , 72 , 0  , 0  , 0  , 0  ), //      {zmm, vm64z}
  ROW(2, 1, 1, 0, 25 , 60 , 0  , 0  , 0  , 0  ), // #115 {r16|m16|r32|m32|r64|m64|mem, u8}
  ROW(2, 1, 1, 0, 27 , 4  , 0  , 0  , 0  , 0  ), //      {r16|m16|mem, r16}
  ROW(2, 1, 1, 0, 28 , 6  , 0  , 0  , 0  , 0  ), //      {r32|m32|mem, r32}
  ROW(2, 0, 1, 0, 15 , 8  , 0  , 0  , 0  , 0  ), //      {r64|m64|mem, r64}
  ROW(3, 1, 1, 1, 1  , 2  , 73 , 0  , 0  , 0  ), // #119 {r8lo|r8hi|m8|mem, r8lo|r8hi, <al>}
  ROW(3, 1, 1, 1, 27 , 4  , 35 , 0  , 0  , 0  ), //      {r16|m16|mem, r16, <ax>}
  ROW(3, 1, 1, 1, 28 , 6  , 38 , 0  , 0  , 0  ), //      {r32|m32|mem, r32, <eax>}
  ROW(3, 0, 1, 1, 15 , 8  , 40 , 0  , 0  , 0  ), //      {r64|m64|mem, r64, <rax>}
  ROW(1, 1, 1, 0, 74 , 0  , 0  , 0  , 0  , 0  ), // #123 {r16|m16|r64|m64|mem}
  ROW(1, 1, 0, 0, 13 , 0  , 0  , 0  , 0  , 0  ), //      {r32|m32}
  ROW(1, 1, 0, 0, 75 , 0  , 0  , 0  , 0  , 0  ), //      {ds|es|ss}
  ROW(1, 1, 1, 0, 76 , 0  , 0  , 0  , 0  , 0  ), //      {fs|gs}
  ROW(1, 1, 1, 0, 77 , 0  , 0  , 0  , 0  , 0  ), // #127 {r16|m16|r64|m64|mem|i8|i16|i32}
  ROW(1, 1, 0, 0, 13 , 0  , 0  , 0  , 0  , 0  ), //      {r32|m32}
  ROW(1, 1, 0, 0, 78 , 0  , 0  , 0  , 0  , 0  ), //      {cs|ss|ds|es}
  ROW(1, 1, 1, 0, 76 , 0  , 0  , 0  , 0  , 0  ), //      {fs|gs}
  ROW(4, 1, 1, 0, 50 , 50 , 50 , 51 , 0  , 0  ), // #131 {xmm, xmm, xmm, xmm|m128|mem}
  ROW(4, 1, 1, 0, 50 , 50 , 52 , 50 , 0  , 0  ), //      {xmm, xmm, m128|mem, xmm}
  ROW(4, 1, 1, 0, 53 , 53 , 53 , 54 , 0  , 0  ), //      {ymm, ymm, ymm, ymm|m256|mem}
  ROW(4, 1, 1, 0, 53 , 53 , 55 , 53 , 0  , 0  ), //      {ymm, ymm, m256|mem, ymm}
  ROW(3, 1, 1, 0, 50 , 79 , 50 , 0  , 0  , 0  ), // #135 {xmm, vm64x|vm64y, xmm}
  ROW(2, 1, 1, 0, 50 , 70 , 0  , 0  , 0  , 0  ), //      {xmm, vm64x}
  ROW(2, 1, 1, 0, 53 , 71 , 0  , 0  , 0  , 0  ), //      {ymm, vm64y}
  ROW(2, 1, 1, 0, 56 , 72 , 0  , 0  , 0  , 0  ), //      {zmm, vm64z}
  ROW(3, 1, 1, 0, 52 , 50 , 50 , 0  , 0  , 0  ), // #139 {m128|mem, xmm, xmm}
  ROW(3, 1, 1, 0, 55 , 53 , 53 , 0  , 0  , 0  ), //      {m256|mem, ymm, ymm}
  ROW(3, 1, 1, 0, 50 , 50 , 52 , 0  , 0  , 0  ), //      {xmm, xmm, m128|mem}
  ROW(3, 1, 1, 0, 53 , 53 , 55 , 0  , 0  , 0  ), //      {ymm, ymm, m256|mem}
  ROW(5, 1, 1, 0, 50 , 50 , 51 , 50 , 80 , 0  ), // #143 {xmm, xmm, xmm|m128|mem, xmm, u4}
  ROW(5, 1, 1, 0, 50 , 50 , 50 , 52 , 80 , 0  ), //      {xmm, xmm, xmm, m128|mem, u4}
  ROW(5, 1, 1, 0, 53 , 53 , 54 , 53 , 80 , 0  ), //      {ymm, ymm, ymm|m256|mem, ymm, u4}
  ROW(5, 1, 1, 0, 53 , 53 , 53 , 55 , 80 , 0  ), //      {ymm, ymm, ymm, m256|mem, u4}
  ROW(3, 1, 1, 0, 53 , 54 , 60 , 0  , 0  , 0  ), // #147 {ymm, ymm|m256|mem, u8}
  ROW(3, 1, 1, 0, 53 , 53 , 54 , 0  , 0  , 0  ), // #148 {ymm, ymm, ymm|m256|mem}
  ROW(3, 1, 1, 0, 56 , 56 , 62 , 0  , 0  , 0  ), //      {zmm, zmm, zmm|m512|mem|u8}
  ROW(3, 1, 1, 0, 56 , 58 , 60 , 0  , 0  , 0  ), //      {zmm, m512|mem, u8}
  ROW(2, 1, 1, 0, 1  , 2  , 0  , 0  , 0  , 0  ), // #151 {r8lo|r8hi|m8|mem, r8lo|r8hi}
  ROW(2, 1, 1, 0, 27 , 4  , 0  , 0  , 0  , 0  ), //      {r16|m16|mem, r16}
  ROW(2, 1, 1, 0, 28 , 6  , 0  , 0  , 0  , 0  ), //      {r32|m32|mem, r32}
  ROW(2, 0, 1, 0, 15 , 8  , 0  , 0  , 0  , 0  ), //      {r64|m64|mem, r64}
  ROW(2, 1, 1, 0, 4  , 27 , 0  , 0  , 0  , 0  ), // #155 {r16, r16|m16|mem}
  ROW(2, 1, 1, 0, 6  , 28 , 0  , 0  , 0  , 0  ), // #156 {r32, r32|m32|mem}
  ROW(2, 0, 1, 0, 8  , 15 , 0  , 0  , 0  , 0  ), //      {r64, r64|m64|mem}
  ROW(1, 1, 1, 0, 81 , 0  , 0  , 0  , 0  , 0  ), // #158 {m32|m64}
  ROW(2, 1, 1, 0, 82 , 83 , 0  , 0  , 0  , 0  ), //      {st0, st}
  ROW(2, 1, 1, 0, 83 , 82 , 0  , 0  , 0  , 0  ), //      {st, st0}
  ROW(2, 1, 1, 0, 4  , 30 , 0  , 0  , 0  , 0  ), // #161 {r16, m32|mem}
  ROW(2, 1, 1, 0, 6  , 84 , 0  , 0  , 0  , 0  ), //      {r32, m48|mem}
  ROW(2, 0, 1, 0, 8  , 85 , 0  , 0  , 0  , 0  ), //      {r64, m80|mem}
  ROW(3, 1, 1, 0, 27 , 4  , 86 , 0  , 0  , 0  ), // #164 {r16|m16|mem, r16, u8|cl}
  ROW(3, 1, 1, 0, 28 , 6  , 86 , 0  , 0  , 0  ), //      {r32|m32|mem, r32, u8|cl}
  ROW(3, 0, 1, 0, 15 , 8  , 86 , 0  , 0  , 0  ), //      {r64|m64|mem, r64, u8|cl}
  ROW(3, 1, 1, 0, 50 , 50 , 51 , 0  , 0  , 0  ), // #167 {xmm, xmm, xmm|m128|mem}
  ROW(3, 1, 1, 0, 53 , 53 , 54 , 0  , 0  , 0  ), // #168 {ymm, ymm, ymm|m256|mem}
  ROW(3, 1, 1, 0, 56 , 56 , 57 , 0  , 0  , 0  ), //      {zmm, zmm, zmm|m512|mem}
  ROW(4, 1, 1, 0, 50 , 50 , 51 , 60 , 0  , 0  ), // #170 {xmm, xmm, xmm|m128|mem, u8}
  ROW(4, 1, 1, 0, 53 , 53 , 54 , 60 , 0  , 0  ), // #171 {ymm, ymm, ymm|m256|mem, u8}
  ROW(4, 1, 1, 0, 56 , 56 , 57 , 60 , 0  , 0  ), //      {zmm, zmm, zmm|m512|mem, u8}
  ROW(4, 1, 1, 0, 87 , 50 , 51 , 60 , 0  , 0  ), // #173 {xmm|k, xmm, xmm|m128|mem, u8}
  ROW(4, 1, 1, 0, 88 , 53 , 54 , 60 , 0  , 0  ), //      {ymm|k, ymm, ymm|m256|mem, u8}
  ROW(4, 1, 1, 0, 89 , 56 , 57 , 60 , 0  , 0  ), //      {k, zmm, zmm|m512|mem, u8}
  ROW(2, 1, 1, 0, 51 , 50 , 0  , 0  , 0  , 0  ), // #176 {xmm|m128|mem, xmm}
  ROW(2, 1, 1, 0, 54 , 53 , 0  , 0  , 0  , 0  ), //      {ymm|m256|mem, ymm}
  ROW(2, 1, 1, 0, 57 , 56 , 0  , 0  , 0  , 0  ), //      {zmm|m512|mem, zmm}
  ROW(2, 1, 1, 0, 50 , 66 , 0  , 0  , 0  , 0  ), // #179 {xmm, xmm|m64|mem}
  ROW(2, 1, 1, 0, 53 , 51 , 0  , 0  , 0  , 0  ), //      {ymm, xmm|m128|mem}
  ROW(2, 1, 1, 0, 56 , 54 , 0  , 0  , 0  , 0  ), //      {zmm, ymm|m256|mem}
  ROW(2, 1, 1, 0, 50 , 51 , 0  , 0  , 0  , 0  ), // #182 {xmm, xmm|m128|mem}
  ROW(2, 1, 1, 0, 53 , 54 , 0  , 0  , 0  , 0  ), //      {ymm, ymm|m256|mem}
  ROW(2, 1, 1, 0, 56 , 57 , 0  , 0  , 0  , 0  ), //      {zmm, zmm|m512|mem}
  ROW(3, 1, 1, 0, 66 , 50 , 60 , 0  , 0  , 0  ), // #185 {xmm|m64|mem, xmm, u8}
  ROW(3, 1, 1, 0, 51 , 53 , 60 , 0  , 0  , 0  ), // #186 {xmm|m128|mem, ymm, u8}
  ROW(3, 1, 1, 0, 54 , 56 , 60 , 0  , 0  , 0  ), // #187 {ymm|m256|mem, zmm, u8}
  ROW(3, 1, 1, 0, 50 , 51 , 60 , 0  , 0  , 0  ), // #188 {xmm, xmm|m128|mem, u8}
  ROW(3, 1, 1, 0, 53 , 54 , 60 , 0  , 0  , 0  ), //      {ymm, ymm|m256|mem, u8}
  ROW(3, 1, 1, 0, 56 , 57 , 60 , 0  , 0  , 0  ), //      {zmm, zmm|m512|mem, u8}
  ROW(2, 1, 1, 0, 50 , 66 , 0  , 0  , 0  , 0  ), // #191 {xmm, xmm|m64|mem}
  ROW(2, 1, 1, 0, 53 , 54 , 0  , 0  , 0  , 0  ), //      {ymm, ymm|m256|mem}
  ROW(2, 1, 1, 0, 56 , 57 , 0  , 0  , 0  , 0  ), //      {zmm, zmm|m512|mem}
  ROW(2, 1, 1, 0, 52 , 50 , 0  , 0  , 0  , 0  ), // #194 {m128|mem, xmm}
  ROW(2, 1, 1, 0, 55 , 53 , 0  , 0  , 0  , 0  ), //      {m256|mem, ymm}
  ROW(2, 1, 1, 0, 58 , 56 , 0  , 0  , 0  , 0  ), //      {m512|mem, zmm}
  ROW(2, 1, 1, 0, 50 , 52 , 0  , 0  , 0  , 0  ), // #197 {xmm, m128|mem}
  ROW(2, 1, 1, 0, 53 , 55 , 0  , 0  , 0  , 0  ), //      {ymm, m256|mem}
  ROW(2, 1, 1, 0, 56 , 58 , 0  , 0  , 0  , 0  ), //      {zmm, m512|mem}
  ROW(2, 0, 1, 0, 15 , 50 , 0  , 0  , 0  , 0  ), // #200 {r64|m64|mem, xmm}
  ROW(2, 1, 1, 0, 50 , 90 , 0  , 0  , 0  , 0  ), //      {xmm, xmm|m64|mem|r64}
  ROW(2, 1, 1, 0, 31 , 50 , 0  , 0  , 0  , 0  ), //      {m64|mem, xmm}
  ROW(2, 1, 1, 0, 31 , 50 , 0  , 0  , 0  , 0  ), // #203 {m64|mem, xmm}
  ROW(2, 1, 1, 0, 50 , 31 , 0  , 0  , 0  , 0  ), //      {xmm, m64|mem}
  ROW(3, 1, 1, 0, 50 , 50 , 50 , 0  , 0  , 0  ), // #205 {xmm, xmm, xmm}
  ROW(2, 1, 1, 0, 30 , 50 , 0  , 0  , 0  , 0  ), // #206 {m32|mem, xmm}
  ROW(2, 1, 1, 0, 50 , 30 , 0  , 0  , 0  , 0  ), //      {xmm, m32|mem}
  ROW(3, 1, 1, 0, 50 , 50 , 50 , 0  , 0  , 0  ), //      {xmm, xmm, xmm}
  ROW(2, 1, 1, 0, 91 , 90 , 0  , 0  , 0  , 0  ), // #209 {xmm|ymm, xmm|m64|mem|r64}
  ROW(2, 0, 1, 0, 56 , 8  , 0  , 0  , 0  , 0  ), //      {zmm, r64}
  ROW(2, 1, 1, 0, 56 , 66 , 0  , 0  , 0  , 0  ), //      {zmm, xmm|m64|mem}
  ROW(4, 1, 1, 0, 89 , 50 , 51 , 60 , 0  , 0  ), // #212 {k, xmm, xmm|m128|mem, u8}
  ROW(4, 1, 1, 0, 89 , 53 , 54 , 60 , 0  , 0  ), //      {k, ymm, ymm|m256|mem, u8}
  ROW(4, 1, 1, 0, 89 , 56 , 57 , 60 , 0  , 0  ), //      {k, zmm, zmm|m512|mem, u8}
  ROW(3, 1, 1, 0, 87 , 50 , 51 , 0  , 0  , 0  ), // #215 {xmm|k, xmm, xmm|m128|mem}
  ROW(3, 1, 1, 0, 88 , 53 , 54 , 0  , 0  , 0  ), //      {ymm|k, ymm, ymm|m256|mem}
  ROW(3, 1, 1, 0, 89 , 56 , 57 , 0  , 0  , 0  ), //      {k, zmm, zmm|m512|mem}
  ROW(2, 1, 1, 0, 92 , 50 , 0  , 0  , 0  , 0  ), // #218 {xmm|m32|mem, xmm}
  ROW(2, 1, 1, 0, 66 , 53 , 0  , 0  , 0  , 0  ), //      {xmm|m64|mem, ymm}
  ROW(2, 1, 1, 0, 51 , 56 , 0  , 0  , 0  , 0  ), //      {xmm|m128|mem, zmm}
  ROW(2, 1, 1, 0, 66 , 50 , 0  , 0  , 0  , 0  ), // #221 {xmm|m64|mem, xmm}
  ROW(2, 1, 1, 0, 51 , 53 , 0  , 0  , 0  , 0  ), //      {xmm|m128|mem, ymm}
  ROW(2, 1, 1, 0, 54 , 56 , 0  , 0  , 0  , 0  ), //      {ymm|m256|mem, zmm}
  ROW(2, 1, 1, 0, 93 , 50 , 0  , 0  , 0  , 0  ), // #224 {xmm|m16|mem, xmm}
  ROW(2, 1, 1, 0, 92 , 53 , 0  , 0  , 0  , 0  ), //      {xmm|m32|mem, ymm}
  ROW(2, 1, 1, 0, 66 , 56 , 0  , 0  , 0  , 0  ), //      {xmm|m64|mem, zmm}
  ROW(2, 1, 1, 0, 50 , 92 , 0  , 0  , 0  , 0  ), // #227 {xmm, xmm|m32|mem}
  ROW(2, 1, 1, 0, 53 , 66 , 0  , 0  , 0  , 0  ), //      {ymm, xmm|m64|mem}
  ROW(2, 1, 1, 0, 56 , 51 , 0  , 0  , 0  , 0  ), //      {zmm, xmm|m128|mem}
  ROW(2, 1, 1, 0, 50 , 93 , 0  , 0  , 0  , 0  ), // #230 {xmm, xmm|m16|mem}
  ROW(2, 1, 1, 0, 53 , 92 , 0  , 0  , 0  , 0  ), //      {ymm, xmm|m32|mem}
  ROW(2, 1, 1, 0, 56 , 66 , 0  , 0  , 0  , 0  ), //      {zmm, xmm|m64|mem}
  ROW(2, 1, 1, 0, 67 , 50 , 0  , 0  , 0  , 0  ), // #233 {vm32x, xmm}
  ROW(2, 1, 1, 0, 68 , 53 , 0  , 0  , 0  , 0  ), //      {vm32y, ymm}
  ROW(2, 1, 1, 0, 69 , 56 , 0  , 0  , 0  , 0  ), //      {vm32z, zmm}
  ROW(2, 1, 1, 0, 70 , 50 , 0  , 0  , 0  , 0  ), // #236 {vm64x, xmm}
  ROW(2, 1, 1, 0, 71 , 53 , 0  , 0  , 0  , 0  ), //      {vm64y, ymm}
  ROW(2, 1, 1, 0, 72 , 56 , 0  , 0  , 0  , 0  ), //      {vm64z, zmm}
  ROW(3, 1, 1, 0, 89 , 50 , 51 , 0  , 0  , 0  ), // #239 {k, xmm, xmm|m128|mem}
  ROW(3, 1, 1, 0, 89 , 53 , 54 , 0  , 0  , 0  ), //      {k, ymm, ymm|m256|mem}
  ROW(3, 1, 1, 0, 89 , 56 , 57 , 0  , 0  , 0  ), //      {k, zmm, zmm|m512|mem}
  ROW(3, 1, 1, 0, 6  , 6  , 28 , 0  , 0  , 0  ), // #242 {r32, r32, r32|m32|mem}
  ROW(3, 0, 1, 0, 8  , 8  , 15 , 0  , 0  , 0  ), //      {r64, r64, r64|m64|mem}
  ROW(3, 1, 1, 0, 6  , 28 , 6  , 0  , 0  , 0  ), // #244 {r32, r32|m32|mem, r32}
  ROW(3, 0, 1, 0, 8  , 15 , 8  , 0  , 0  , 0  ), //      {r64, r64|m64|mem, r64}
  ROW(2, 1, 0, 0, 94 , 28 , 0  , 0  , 0  , 0  ), // #246 {bnd, r32|m32|mem}
  ROW(2, 0, 1, 0, 94 , 15 , 0  , 0  , 0  , 0  ), //      {bnd, r64|m64|mem}
  ROW(2, 1, 1, 0, 94 , 95 , 0  , 0  , 0  , 0  ), // #248 {bnd, bnd|mem}
  ROW(2, 1, 1, 0, 96 , 94 , 0  , 0  , 0  , 0  ), //      {mem, bnd}
  ROW(2, 1, 0, 0, 4  , 30 , 0  , 0  , 0  , 0  ), // #250 {r16, m32|mem}
  ROW(2, 1, 0, 0, 6  , 31 , 0  , 0  , 0  , 0  ), //      {r32, m64|mem}
  ROW(1, 1, 0, 0, 97 , 0  , 0  , 0  , 0  , 0  ), // #252 {rel16|r16|m16|r32|m32}
  ROW(1, 1, 1, 0, 98 , 0  , 0  , 0  , 0  , 0  ), //      {rel32|r64|m64|mem}
  ROW(2, 1, 1, 0, 6  , 99 , 0  , 0  , 0  , 0  ), // #254 {r32, r8lo|r8hi|m8|r16|m16|r32|m32}
  ROW(2, 0, 1, 0, 8  , 100, 0  , 0  , 0  , 0  ), //      {r64, r8lo|r8hi|m8|r64|m64}
  ROW(1, 1, 0, 0, 101, 0  , 0  , 0  , 0  , 0  ), // #256 {r16|r32}
  ROW(1, 1, 1, 0, 32 , 0  , 0  , 0  , 0  , 0  ), // #257 {r8lo|r8hi|m8|r16|m16|r32|m32|r64|m64|mem}
  ROW(3, 1, 1, 0, 50 , 60 , 60 , 0  , 0  , 0  ), // #258 {xmm, u8, u8}
  ROW(2, 1, 1, 0, 50 , 50 , 0  , 0  , 0  , 0  ), // #259 {xmm, xmm}
  ROW(0, 1, 1, 0, 0  , 0  , 0  , 0  , 0  , 0  ), // #260 {}
  ROW(1, 1, 1, 0, 83 , 0  , 0  , 0  , 0  , 0  ), // #261 {st}
  ROW(0, 1, 1, 0, 0  , 0  , 0  , 0  , 0  , 0  ), // #262 {}
  ROW(1, 1, 1, 0, 102, 0  , 0  , 0  , 0  , 0  ), // #263 {m32|m64|st}
  ROW(2, 1, 1, 0, 50 , 50 , 0  , 0  , 0  , 0  ), // #264 {xmm, xmm}
  ROW(4, 1, 1, 0, 50 , 50 , 60 , 60 , 0  , 0  ), //      {xmm, xmm, u8, u8}
  ROW(2, 1, 0, 0, 6  , 52 , 0  , 0  , 0  , 0  ), // #266 {r32, m128|mem}
  ROW(2, 0, 1, 0, 8  , 52 , 0  , 0  , 0  , 0  ), //      {r64, m128|mem}
  ROW(1, 1, 1, 0, 103, 0  , 0  , 0  , 0  , 0  ), // #268 {rel8|rel32}
  ROW(1, 1, 0, 0, 104, 0  , 0  , 0  , 0  , 0  ), //      {rel16}
  ROW(2, 1, 0, 1, 105, 106, 0  , 0  , 0  , 0  ), // #270 {<cx|ecx>, rel8}
  ROW(2, 0, 1, 1, 107, 106, 0  , 0  , 0  , 0  ), //      {<ecx|rcx>, rel8}
  ROW(1, 1, 1, 0, 108, 0  , 0  , 0  , 0  , 0  ), // #272 {rel8|rel32|r64|m64|mem}
  ROW(1, 1, 0, 0, 109, 0  , 0  , 0  , 0  , 0  ), //      {rel16|r32|m32|mem}
  ROW(2, 1, 1, 0, 89 , 110, 0  , 0  , 0  , 0  ), // #274 {k, k|m8|mem|r32|r64|r8lo|r8hi|r16}
  ROW(2, 1, 1, 0, 111, 89 , 0  , 0  , 0  , 0  ), //      {m8|mem|r32|r64|r8lo|r8hi|r16, k}
  ROW(2, 1, 1, 0, 89 , 112, 0  , 0  , 0  , 0  ), // #276 {k, k|m32|mem|r32|r64}
  ROW(2, 1, 1, 0, 113, 89 , 0  , 0  , 0  , 0  ), //      {m32|mem|r32|r64, k}
  ROW(2, 1, 1, 0, 89 , 114, 0  , 0  , 0  , 0  ), // #278 {k, k|m64|mem|r64}
  ROW(2, 1, 1, 0, 15 , 89 , 0  , 0  , 0  , 0  ), //      {m64|mem|r64, k}
  ROW(2, 1, 1, 0, 89 , 115, 0  , 0  , 0  , 0  ), // #280 {k, k|m16|mem|r32|r64|r16}
  ROW(2, 1, 1, 0, 116, 89 , 0  , 0  , 0  , 0  ), //      {m16|mem|r32|r64|r16, k}
  ROW(2, 1, 1, 0, 4  , 27 , 0  , 0  , 0  , 0  ), // #282 {r16, r16|m16|mem}
  ROW(2, 1, 1, 0, 6  , 117, 0  , 0  , 0  , 0  ), //      {r32, r32|m16|mem|r16}
  ROW(2, 1, 0, 0, 4  , 30 , 0  , 0  , 0  , 0  ), // #284 {r16, m32|mem}
  ROW(2, 1, 0, 0, 6  , 84 , 0  , 0  , 0  , 0  ), //      {r32, m48|mem}
  ROW(2, 1, 1, 0, 4  , 27 , 0  , 0  , 0  , 0  ), // #286 {r16, r16|m16|mem}
  ROW(2, 1, 1, 0, 118, 117, 0  , 0  , 0  , 0  ), //      {r32|r64, r32|m16|mem|r16}
  ROW(2, 1, 1, 0, 65 , 113, 0  , 0  , 0  , 0  ), // #288 {mm|xmm, r32|m32|mem|r64}
  ROW(2, 1, 1, 0, 113, 65 , 0  , 0  , 0  , 0  ), //      {r32|m32|mem|r64, mm|xmm}
  ROW(2, 1, 1, 0, 50 , 92 , 0  , 0  , 0  , 0  ), // #290 {xmm, xmm|m32|mem}
  ROW(2, 1, 1, 0, 30 , 50 , 0  , 0  , 0  , 0  ), //      {m32|mem, xmm}
  ROW(2, 1, 1, 0, 4  , 9  , 0  , 0  , 0  , 0  ), // #292 {r16, r8lo|r8hi|m8}
  ROW(2, 1, 1, 0, 118, 119, 0  , 0  , 0  , 0  ), //      {r32|r64, r8lo|r8hi|m8|r16|m16}
  ROW(4, 1, 1, 1, 6  , 6  , 28 , 37 , 0  , 0  ), // #294 {r32, r32, r32|m32|mem, <edx>}
  ROW(4, 0, 1, 1, 8  , 8  , 15 , 39 , 0  , 0  ), //      {r64, r64, r64|m64|mem, <rdx>}
  ROW(0, 1, 1, 0, 0  , 0  , 0  , 0  , 0  , 0  ), // #296 {}
  ROW(1, 1, 1, 0, 120, 0  , 0  , 0  , 0  , 0  ), //      {r16|m16|r32|m32}
  ROW(2, 1, 1, 0, 63 , 121, 0  , 0  , 0  , 0  ), // #298 {mm, mm|m64|mem}
  ROW(2, 1, 1, 0, 50 , 51 , 0  , 0  , 0  , 0  ), //      {xmm, xmm|m128|mem}
  ROW(3, 1, 1, 0, 63 , 121, 60 , 0  , 0  , 0  ), // #300 {mm, mm|m64|mem, u8}
  ROW(3, 1, 1, 0, 50 , 51 , 60 , 0  , 0  , 0  ), //      {xmm, xmm|m128|mem, u8}
  ROW(3, 1, 1, 0, 118, 63 , 60 , 0  , 0  , 0  ), // #302 {r32|r64, mm, u8}
  ROW(3, 1, 1, 0, 116, 50 , 60 , 0  , 0  , 0  ), // #303 {r32|r64|m16|mem|r16, xmm, u8}
  ROW(2, 1, 1, 0, 63 , 122, 0  , 0  , 0  , 0  ), // #304 {mm, u8|mm|m64|mem}
  ROW(2, 1, 1, 0, 50 , 59 , 0  , 0  , 0  , 0  ), //      {xmm, u8|xmm|m128|mem}
  ROW(0, 1, 1, 0, 0  , 0  , 0  , 0  , 0  , 0  ), // #306 {}
  ROW(1, 1, 1, 0, 123, 0  , 0  , 0  , 0  , 0  ), //      {u16}
  ROW(3, 1, 1, 0, 6  , 28 , 60 , 0  , 0  , 0  ), // #308 {r32, r32|m32|mem, u8}
  ROW(3, 0, 1, 0, 8  , 15 , 60 , 0  , 0  , 0  ), //      {r64, r64|m64|mem, u8}
  ROW(4, 1, 1, 0, 50 , 50 , 51 , 50 , 0  , 0  ), // #310 {xmm, xmm, xmm|m128|mem, xmm}
  ROW(4, 1, 1, 0, 53 , 53 , 54 , 53 , 0  , 0  ), //      {ymm, ymm, ymm|m256|mem, ymm}
  ROW(2, 1, 1, 0, 91 , 124, 0  , 0  , 0  , 0  ), // #312 {xmm|ymm, m32|xmm|m64}
  ROW(2, 1, 1, 0, 56 , 125, 0  , 0  , 0  , 0  ), //      {zmm, xmm|m64}
  ROW(2, 1, 1, 0, 50 , 126, 0  , 0  , 0  , 0  ), // #314 {xmm, xmm|m128|ymm|m256}
  ROW(2, 1, 1, 0, 53 , 57 , 0  , 0  , 0  , 0  ), //      {ymm, zmm|m512|mem}
  ROW(4, 1, 1, 0, 50 , 50 , 50 , 66 , 0  , 0  ), // #316 {xmm, xmm, xmm, xmm|m64|mem}
  ROW(4, 1, 1, 0, 50 , 50 , 31 , 50 , 0  , 0  ), //      {xmm, xmm, m64|mem, xmm}
  ROW(4, 1, 1, 0, 50 , 50 , 50 , 92 , 0  , 0  ), // #318 {xmm, xmm, xmm, xmm|m32|mem}
  ROW(4, 1, 1, 0, 50 , 50 , 30 , 50 , 0  , 0  ), //      {xmm, xmm, m32|mem, xmm}
  ROW(4, 1, 1, 0, 53 , 53 , 51 , 60 , 0  , 0  ), // #320 {ymm, ymm, xmm|m128|mem, u8}
  ROW(4, 1, 1, 0, 56 , 56 , 51 , 60 , 0  , 0  ), //      {zmm, zmm, xmm|m128|mem, u8}
  ROW(2, 1, 1, 0, 113, 50 , 0  , 0  , 0  , 0  ), // #322 {r32|m32|mem|r64, xmm}
  ROW(2, 1, 1, 0, 50 , 113, 0  , 0  , 0  , 0  ), //      {xmm, r32|m32|mem|r64}
  ROW(2, 1, 1, 0, 31 , 50 , 0  , 0  , 0  , 0  ), // #324 {m64|mem, xmm}
  ROW(3, 1, 1, 0, 50 , 50 , 31 , 0  , 0  , 0  ), //      {xmm, xmm, m64|mem}
  ROW(3, 1, 1, 0, 50 , 50 , 59 , 0  , 0  , 0  ), // #326 {xmm, xmm, xmm|m128|mem|u8}
  ROW(3, 1, 1, 0, 50 , 52 , 127, 0  , 0  , 0  ), //      {xmm, m128|mem, u8|xmm}
  ROW(2, 1, 1, 0, 79 , 50 , 0  , 0  , 0  , 0  ), // #328 {vm64x|vm64y, xmm}
  ROW(2, 1, 1, 0, 72 , 53 , 0  , 0  , 0  , 0  ), //      {vm64z, ymm}
  ROW(3, 1, 1, 0, 50 , 50 , 51 , 0  , 0  , 0  ), // #330 {xmm, xmm, xmm|m128|mem}
  ROW(3, 1, 1, 0, 50 , 52 , 50 , 0  , 0  , 0  ), //      {xmm, m128|mem, xmm}
  ROW(2, 1, 1, 0, 67 , 91 , 0  , 0  , 0  , 0  ), // #332 {vm32x, xmm|ymm}
  ROW(2, 1, 1, 0, 68 , 56 , 0  , 0  , 0  , 0  ), //      {vm32y, zmm}
  ROW(1, 1, 0, 1, 35 , 0  , 0  , 0  , 0  , 0  ), // #334 {<ax>}
  ROW(2, 1, 0, 1, 35 , 60 , 0  , 0  , 0  , 0  ), // #335 {<ax>, u8}
  ROW(2, 1, 0, 0, 27 , 4  , 0  , 0  , 0  , 0  ), // #336 {r16|m16|mem, r16}
  ROW(3, 1, 1, 1, 50 , 51 , 128, 0  , 0  , 0  ), // #337 {xmm, xmm|m128|mem, <xmm0>}
  ROW(2, 1, 1, 0, 94 , 129, 0  , 0  , 0  , 0  ), // #338 {bnd, mib}
  ROW(2, 1, 1, 0, 94 , 96 , 0  , 0  , 0  , 0  ), // #339 {bnd, mem}
  ROW(2, 1, 1, 0, 129, 94 , 0  , 0  , 0  , 0  ), // #340 {mib, bnd}
  ROW(1, 1, 1, 0, 118, 0  , 0  , 0  , 0  , 0  ), // #341 {r32|r64}
  ROW(1, 1, 1, 1, 35 , 0  , 0  , 0  , 0  , 0  ), // #342 {<ax>}
  ROW(2, 1, 1, 2, 37 , 38 , 0  , 0  , 0  , 0  ), // #343 {<edx>, <eax>}
  ROW(1, 0, 1, 1, 40 , 0  , 0  , 0  , 0  , 0  ), // #344 {<rax>}
  ROW(1, 1, 1, 0, 96 , 0  , 0  , 0  , 0  , 0  ), // #345 {mem}
  ROW(1, 1, 1, 1, 130, 0  , 0  , 0  , 0  , 0  ), // #346 {<ds:[zax]>}
  ROW(2, 1, 1, 2, 131, 132, 0  , 0  , 0  , 0  ), // #347 {<ds:[zsi]>, <es:[zdi]>}
  ROW(3, 1, 1, 0, 50 , 66 , 60 , 0  , 0  , 0  ), // #348 {xmm, xmm|m64|mem, u8}
  ROW(3, 1, 1, 0, 50 , 92 , 60 , 0  , 0  , 0  ), // #349 {xmm, xmm|m32|mem, u8}
  ROW(5, 0, 1, 4, 52 , 39 , 40 , 133, 134, 0  ), // #350 {m128|mem, <rdx>, <rax>, <rcx>, <rbx>}
  ROW(5, 1, 1, 4, 31 , 37 , 38 , 135, 136, 0  ), // #351 {m64|mem, <edx>, <eax>, <ecx>, <ebx>}
  ROW(4, 1, 1, 4, 38 , 136, 135, 37 , 0  , 0  ), // #352 {<eax>, <ebx>, <ecx>, <edx>}
  ROW(2, 0, 1, 2, 39 , 40 , 0  , 0  , 0  , 0  ), // #353 {<rdx>, <rax>}
  ROW(2, 1, 1, 0, 63 , 51 , 0  , 0  , 0  , 0  ), // #354 {mm, xmm|m128|mem}
  ROW(2, 1, 1, 0, 50 , 121, 0  , 0  , 0  , 0  ), // #355 {xmm, mm|m64|mem}
  ROW(2, 1, 1, 0, 63 , 66 , 0  , 0  , 0  , 0  ), // #356 {mm, xmm|m64|mem}
  ROW(2, 1, 1, 0, 118, 66 , 0  , 0  , 0  , 0  ), // #357 {r32|r64, xmm|m64|mem}
  ROW(2, 1, 1, 0, 50 , 137, 0  , 0  , 0  , 0  ), // #358 {xmm, r32|m32|mem|r64|m64}
  ROW(2, 1, 1, 0, 118, 92 , 0  , 0  , 0  , 0  ), // #359 {r32|r64, xmm|m32|mem}
  ROW(2, 1, 1, 2, 36 , 35 , 0  , 0  , 0  , 0  ), // #360 {<dx>, <ax>}
  ROW(1, 1, 1, 1, 38 , 0  , 0  , 0  , 0  , 0  ), // #361 {<eax>}
  ROW(2, 1, 1, 0, 123, 60 , 0  , 0  , 0  , 0  ), // #362 {u16, u8}
  ROW(3, 1, 1, 0, 113, 50 , 60 , 0  , 0  , 0  ), // #363 {r32|m32|mem|r64, xmm, u8}
  ROW(1, 1, 1, 0, 85 , 0  , 0  , 0  , 0  , 0  ), // #364 {m80|mem}
  ROW(1, 1, 1, 0, 138, 0  , 0  , 0  , 0  , 0  ), // #365 {m16|m32}
  ROW(1, 1, 1, 0, 139, 0  , 0  , 0  , 0  , 0  ), // #366 {m16|m32|m64}
  ROW(1, 1, 1, 0, 140, 0  , 0  , 0  , 0  , 0  ), // #367 {m32|m64|m80|st}
  ROW(1, 1, 1, 0, 29 , 0  , 0  , 0  , 0  , 0  ), // #368 {m16|mem}
  ROW(1, 1, 1, 0, 47 , 0  , 0  , 0  , 0  , 0  ), // #369 {ax|m16|mem}
  ROW(1, 0, 1, 0, 96 , 0  , 0  , 0  , 0  , 0  ), // #370 {mem}
  ROW(2, 1, 1, 0, 141, 142, 0  , 0  , 0  , 0  ), // #371 {al|ax|eax, u8|dx}
  ROW(2, 1, 1, 0, 143, 144, 0  , 0  , 0  , 0  ), // #372 {es:[zdi], dx}
  ROW(1, 1, 1, 0, 60 , 0  , 0  , 0  , 0  , 0  ), // #373 {u8}
  ROW(0, 1, 0, 0, 0  , 0  , 0  , 0  , 0  , 0  ), // #374 {}
  ROW(0, 0, 1, 0, 0  , 0  , 0  , 0  , 0  , 0  ), // #375 {}
  ROW(3, 1, 1, 0, 89 , 89 , 89 , 0  , 0  , 0  ), // #376 {k, k, k}
  ROW(2, 1, 1, 0, 89 , 89 , 0  , 0  , 0  , 0  ), // #377 {k, k}
  ROW(3, 1, 1, 0, 89 , 89 , 60 , 0  , 0  , 0  ), // #378 {k, k, u8}
  ROW(1, 1, 1, 1, 145, 0  , 0  , 0  , 0  , 0  ), // #379 {<ah>}
  ROW(1, 1, 1, 0, 30 , 0  , 0  , 0  , 0  , 0  ), // #380 {m32|mem}
  ROW(2, 1, 1, 0, 146, 147, 0  , 0  , 0  , 0  ), // #381 {r16|r32|r64, mem|m8|m16|m32|m48|m64|m80|m128|m256|m512|m1024}
  ROW(1, 1, 1, 0, 27 , 0  , 0  , 0  , 0  , 0  ), // #382 {r16|m16|mem}
  ROW(2, 1, 1, 2, 148, 131, 0  , 0  , 0  , 0  ), // #383 {<al|ax|eax|rax>, <ds:[zsi]>}
  ROW(3, 1, 1, 1, 50 , 50 , 149, 0  , 0  , 0  ), // #384 {xmm, xmm, <ds:[zdi]>}
  ROW(3, 1, 1, 1, 63 , 63 , 149, 0  , 0  , 0  ), // #385 {mm, mm, <ds:[zdi]>}
  ROW(3, 1, 1, 3, 130, 135, 37 , 0  , 0  , 0  ), // #386 {<ds:[zax]>, <ecx>, <edx>}
  ROW(2, 1, 1, 0, 63 , 50 , 0  , 0  , 0  , 0  ), // #387 {mm, xmm}
  ROW(2, 1, 1, 0, 118, 50 , 0  , 0  , 0  , 0  ), // #388 {r32|r64, xmm}
  ROW(2, 1, 1, 0, 31 , 63 , 0  , 0  , 0  , 0  ), // #389 {m64|mem, mm}
  ROW(2, 1, 1, 0, 50 , 63 , 0  , 0  , 0  , 0  ), // #390 {xmm, mm}
  ROW(2, 1, 1, 2, 132, 131, 0  , 0  , 0  , 0  ), // #391 {<es:[zdi]>, <ds:[zsi]>}
  ROW(2, 0, 1, 0, 8  , 28 , 0  , 0  , 0  , 0  ), // #392 {r64, r32|m32|mem}
  ROW(2, 1, 1, 2, 38 , 135, 0  , 0  , 0  , 0  ), // #393 {<eax>, <ecx>}
  ROW(3, 1, 1, 3, 38 , 135, 136, 0  , 0  , 0  ), // #394 {<eax>, <ecx>, <ebx>}
  ROW(2, 1, 1, 0, 142, 141, 0  , 0  , 0  , 0  ), // #395 {u8|dx, al|ax|eax}
  ROW(2, 1, 1, 0, 144, 150, 0  , 0  , 0  , 0  ), // #396 {dx, ds:[zsi]}
  ROW(6, 1, 1, 3, 50 , 51 , 60 , 135, 38 , 37 ), // #397 {xmm, xmm|m128|mem, u8, <ecx>, <eax>, <edx>}
  ROW(6, 1, 1, 3, 50 , 51 , 60 , 128, 38 , 37 ), // #398 {xmm, xmm|m128|mem, u8, <xmm0>, <eax>, <edx>}
  ROW(4, 1, 1, 1, 50 , 51 , 60 , 135, 0  , 0  ), // #399 {xmm, xmm|m128|mem, u8, <ecx>}
  ROW(4, 1, 1, 1, 50 , 51 , 60 , 128, 0  , 0  ), // #400 {xmm, xmm|m128|mem, u8, <xmm0>}
  ROW(3, 1, 1, 0, 111, 50 , 60 , 0  , 0  , 0  ), // #401 {r32|m8|mem|r64|r8lo|r8hi|r16, xmm, u8}
  ROW(3, 0, 1, 0, 15 , 50 , 60 , 0  , 0  , 0  ), // #402 {r64|m64|mem, xmm, u8}
  ROW(3, 1, 1, 0, 50 , 111, 60 , 0  , 0  , 0  ), // #403 {xmm, r32|m8|mem|r64|r8lo|r8hi|r16, u8}
  ROW(3, 1, 1, 0, 50 , 113, 60 , 0  , 0  , 0  ), // #404 {xmm, r32|m32|mem|r64, u8}
  ROW(3, 0, 1, 0, 50 , 15 , 60 , 0  , 0  , 0  ), // #405 {xmm, r64|m64|mem, u8}
  ROW(3, 1, 1, 0, 65 , 116, 60 , 0  , 0  , 0  ), // #406 {mm|xmm, r32|m16|mem|r64|r16, u8}
  ROW(2, 1, 1, 0, 118, 65 , 0  , 0  , 0  , 0  ), // #407 {r32|r64, mm|xmm}
  ROW(2, 1, 1, 0, 50 , 60 , 0  , 0  , 0  , 0  ), // #408 {xmm, u8}
  ROW(2, 1, 1, 0, 32 , 86 , 0  , 0  , 0  , 0  ), // #409 {r8lo|r8hi|m8|r16|m16|r32|m32|r64|m64|mem, cl|u8}
  ROW(1, 0, 1, 0, 118, 0  , 0  , 0  , 0  , 0  ), // #410 {r32|r64}
  ROW(3, 1, 1, 3, 37 , 38 , 135, 0  , 0  , 0  ), // #411 {<edx>, <eax>, <ecx>}
  ROW(1, 1, 1, 0, 146, 0  , 0  , 0  , 0  , 0  ), // #412 {r16|r32|r64}
  ROW(2, 1, 1, 2, 148, 132, 0  , 0  , 0  , 0  ), // #413 {<al|ax|eax|rax>, <es:[zdi]>}
  ROW(1, 1, 1, 0, 1  , 0  , 0  , 0  , 0  , 0  ), // #414 {r8lo|r8hi|m8|mem}
  ROW(1, 1, 1, 0, 116, 0  , 0  , 0  , 0  , 0  ), // #415 {r16|m16|mem|r32|r64}
  ROW(2, 1, 1, 2, 132, 148, 0  , 0  , 0  , 0  ), // #416 {<es:[zdi]>, <al|ax|eax|rax>}
  ROW(6, 1, 1, 0, 56 , 56 , 56 , 56 , 56 , 52 ), // #417 {zmm, zmm, zmm, zmm, zmm, m128|mem}
  ROW(6, 1, 1, 0, 50 , 50 , 50 , 50 , 50 , 52 ), // #418 {xmm, xmm, xmm, xmm, xmm, m128|mem}
  ROW(3, 1, 1, 0, 50 , 50 , 66 , 0  , 0  , 0  ), // #419 {xmm, xmm, xmm|m64|mem}
  ROW(3, 1, 1, 0, 50 , 50 , 92 , 0  , 0  , 0  ), // #420 {xmm, xmm, xmm|m32|mem}
  ROW(2, 1, 1, 0, 53 , 52 , 0  , 0  , 0  , 0  ), // #421 {ymm, m128|mem}
  ROW(2, 1, 1, 0, 151, 66 , 0  , 0  , 0  , 0  ), // #422 {ymm|zmm, xmm|m64|mem}
  ROW(2, 1, 1, 0, 151, 52 , 0  , 0  , 0  , 0  ), // #423 {ymm|zmm, m128|mem}
  ROW(2, 1, 1, 0, 56 , 55 , 0  , 0  , 0  , 0  ), // #424 {zmm, m256|mem}
  ROW(2, 1, 1, 0, 152, 66 , 0  , 0  , 0  , 0  ), // #425 {xmm|ymm|zmm, xmm|m64|mem}
  ROW(4, 1, 1, 0, 87 , 50 , 66 , 60 , 0  , 0  ), // #426 {xmm|k, xmm, xmm|m64|mem, u8}
  ROW(4, 1, 1, 0, 87 , 50 , 92 , 60 , 0  , 0  ), // #427 {xmm|k, xmm, xmm|m32|mem, u8}
  ROW(3, 1, 1, 0, 50 , 50 , 137, 0  , 0  , 0  ), // #428 {xmm, xmm, r32|m32|mem|r64|m64}
  ROW(3, 1, 1, 0, 51 , 151, 60 , 0  , 0  , 0  ), // #429 {xmm|m128|mem, ymm|zmm, u8}
  ROW(4, 1, 1, 0, 50 , 50 , 66 , 60 , 0  , 0  ), // #430 {xmm, xmm, xmm|m64|mem, u8}
  ROW(4, 1, 1, 0, 50 , 50 , 92 , 60 , 0  , 0  ), // #431 {xmm, xmm, xmm|m32|mem, u8}
  ROW(3, 1, 1, 0, 89 , 153, 60 , 0  , 0  , 0  ), // #432 {k, xmm|m128|ymm|m256|zmm|m512, u8}
  ROW(3, 1, 1, 0, 89 , 66 , 60 , 0  , 0  , 0  ), // #433 {k, xmm|m64|mem, u8}
  ROW(3, 1, 1, 0, 89 , 92 , 60 , 0  , 0  , 0  ), // #434 {k, xmm|m32|mem, u8}
  ROW(1, 1, 1, 0, 68 , 0  , 0  , 0  , 0  , 0  ), // #435 {vm32y}
  ROW(1, 1, 1, 0, 69 , 0  , 0  , 0  , 0  , 0  ), // #436 {vm32z}
  ROW(1, 1, 1, 0, 72 , 0  , 0  , 0  , 0  , 0  ), // #437 {vm64z}
  ROW(4, 1, 1, 0, 56 , 56 , 54 , 60 , 0  , 0  ), // #438 {zmm, zmm, ymm|m256|mem, u8}
  ROW(2, 1, 1, 0, 118, 91 , 0  , 0  , 0  , 0  ), // #439 {r32|r64, xmm|ymm}
  ROW(2, 1, 1, 0, 152, 154, 0  , 0  , 0  , 0  ), // #440 {xmm|ymm|zmm, xmm|m8|mem|r32|r64|r8lo|r8hi|r16}
  ROW(2, 1, 1, 0, 152, 155, 0  , 0  , 0  , 0  ), // #441 {xmm|ymm|zmm, xmm|m32|mem|r32|r64}
  ROW(2, 1, 1, 0, 152, 89 , 0  , 0  , 0  , 0  ), // #442 {xmm|ymm|zmm, k}
  ROW(2, 1, 1, 0, 152, 156, 0  , 0  , 0  , 0  ), // #443 {xmm|ymm|zmm, xmm|m16|mem|r32|r64|r16}
  ROW(4, 1, 1, 0, 50 , 50 , 111, 60 , 0  , 0  ), // #444 {xmm, xmm, r32|m8|mem|r64|r8lo|r8hi|r16, u8}
  ROW(4, 1, 1, 0, 50 , 50 , 113, 60 , 0  , 0  ), // #445 {xmm, xmm, r32|m32|mem|r64, u8}
  ROW(4, 0, 1, 0, 50 , 50 , 15 , 60 , 0  , 0  ), // #446 {xmm, xmm, r64|m64|mem, u8}
  ROW(4, 1, 1, 0, 50 , 50 , 116, 60 , 0  , 0  ), // #447 {xmm, xmm, r32|m16|mem|r64|r16, u8}
  ROW(2, 1, 1, 0, 89 , 152, 0  , 0  , 0  , 0  ), // #448 {k, xmm|ymm|zmm}
  ROW(1, 1, 1, 0, 104, 0  , 0  , 0  , 0  , 0  ), // #449 {rel16|rel32}
  ROW(3, 1, 1, 2, 96 , 37 , 38 , 0  , 0  , 0  ), // #450 {mem, <edx>, <eax>}
  ROW(3, 0, 1, 2, 96 , 37 , 38 , 0  , 0  , 0  )  // #451 {mem, <edx>, <eax>}
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
  ROW(F(Mem), M(M16) | M(M64) | M(Any), 0, 0x00),
  ROW(F(SReg), 0, 0, 0x00),
  ROW(F(CReg) | F(DReg), 0, 0, 0x00),
  ROW(F(Gpq) | F(I32), 0, 0, 0x00),
  ROW(F(Gpw) | F(Gpd) | F(Gpq) | F(Mem), M(M16) | M(M32) | M(M64) | M(Any), 0, 0x00),
  ROW(F(I8), 0, 0, 0x00),
  ROW(F(Gpw) | F(Mem), M(M16) | M(Any), 0, 0x00),
  ROW(F(Gpd) | F(Mem), M(M32) | M(Any), 0, 0x00),
  ROW(F(Mem), M(M16) | M(Any), 0, 0x00),
  ROW(F(Mem), M(M32) | M(Any), 0, 0x00),
  ROW(F(Mem), M(M64) | M(Any), 0, 0x00),
  ROW(F(GpbLo) | F(GpbHi) | F(Gpw) | F(Gpd) | F(Gpq) | F(Mem), M(M8) | M(M16) | M(M32) | M(M64) | M(Any), 0, 0x00),
  ROW(F(Gpq), 0, 0, 0x01),
  ROW(F(Gpq) | F(Mem) | F(I32) | F(U32), M(M64) | M(Any), 0, 0x00),
  ROW(F(Implicit) | F(Gpw), 0, 0, 0x01),
  ROW(F(Implicit) | F(Gpw), 0, 0, 0x04),
  ROW(F(Implicit) | F(Gpd), 0, 0, 0x04),
  ROW(F(Implicit) | F(Gpd), 0, 0, 0x01),
  ROW(F(Implicit) | F(Gpq), 0, 0, 0x04),
  ROW(F(Implicit) | F(Gpq), 0, 0, 0x01),
  ROW(F(Gpw) | F(Mem) | F(I8) | F(I16), M(M16) | M(Any), 0, 0x00),
  ROW(F(Gpd) | F(Mem) | F(I8) | F(I32), M(M32) | M(Any), 0, 0x00),
  ROW(F(Gpq) | F(Mem) | F(I8) | F(I32), M(M64) | M(Any), 0, 0x00),
  ROW(F(I8) | F(I16) | F(U16), 0, 0, 0x00),
  ROW(F(I8) | F(I32) | F(U32), 0, 0, 0x00),
  ROW(F(I8) | F(I32), 0, 0, 0x00),
  ROW(F(Gpw) | F(Mem), M(M16) | M(Any), 0, 0x01),
  ROW(F(Gpd) | F(Mem), M(M32) | M(Any), 0, 0x01),
  ROW(F(Gpq) | F(Mem), M(M64) | M(Any), 0, 0x01),
  ROW(F(Xmm), 0, 0, 0x00),
  ROW(F(Xmm) | F(Mem), M(M128) | M(Any), 0, 0x00),
  ROW(F(Mem), M(M128) | M(Any), 0, 0x00),
  ROW(F(Ymm), 0, 0, 0x00),
  ROW(F(Ymm) | F(Mem), M(M256) | M(Any), 0, 0x00),
  ROW(F(Mem), M(M256) | M(Any), 0, 0x00),
  ROW(F(Zmm), 0, 0, 0x00),
  ROW(F(Zmm) | F(Mem), M(M512) | M(Any), 0, 0x00),
  ROW(F(Mem), M(M512) | M(Any), 0, 0x00),
  ROW(F(Xmm) | F(Mem) | F(U8), M(M128) | M(Any), 0, 0x00),
  ROW(F(U8), 0, 0, 0x00),
  ROW(F(Ymm) | F(Mem) | F(U8), M(M256) | M(Any), 0, 0x00),
  ROW(F(Zmm) | F(Mem) | F(U8), M(M512) | M(Any), 0, 0x00),
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
  ROW(F(Implicit) | F(GpbLo), 0, 0, 0x01),
  ROW(F(Gpw) | F(Gpq) | F(Mem), M(M16) | M(M64) | M(Any), 0, 0x00),
  ROW(F(SReg), 0, 0, 0x1A),
  ROW(F(SReg), 0, 0, 0x60),
  ROW(F(Gpw) | F(Gpq) | F(Mem) | F(I8) | F(I16) | F(I32), M(M16) | M(M64) | M(Any), 0, 0x00),
  ROW(F(SReg), 0, 0, 0x1E),
  ROW(F(Vm), M(Vm64x) | M(Vm64y), 0, 0x00),
  ROW(F(U4), 0, 0, 0x00),
  ROW(F(Mem), M(M32) | M(M64), 0, 0x00),
  ROW(F(St), 0, 0, 0x01),
  ROW(F(St), 0, 0, 0x00),
  ROW(F(Mem), M(M48) | M(Any), 0, 0x00),
  ROW(F(Mem), M(M80) | M(Any), 0, 0x00),
  ROW(F(GpbLo) | F(U8), 0, 0, 0x02),
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
  ROW(F(I32) | F(I64) | F(Rel8) | F(Rel32), 0, 0, 0x00),
  ROW(F(I32) | F(I64) | F(Rel32), 0, 0, 0x00),
  ROW(F(Implicit) | F(Gpw) | F(Gpd), 0, 0, 0x02),
  ROW(F(I32) | F(I64) | F(Rel8), 0, 0, 0x00),
  ROW(F(Implicit) | F(Gpd) | F(Gpq), 0, 0, 0x02),
  ROW(F(Gpq) | F(Mem) | F(I32) | F(I64) | F(Rel8) | F(Rel32), M(M64) | M(Any), 0, 0x00),
  ROW(F(Gpd) | F(Mem) | F(I32) | F(I64) | F(Rel32), M(M32) | M(Any), 0, 0x00),
  ROW(F(GpbLo) | F(GpbHi) | F(Gpw) | F(Gpd) | F(Gpq) | F(KReg) | F(Mem), M(M8) | M(Any), 0, 0x00),
  ROW(F(GpbLo) | F(GpbHi) | F(Gpw) | F(Gpd) | F(Gpq) | F(Mem), M(M8) | M(Any), 0, 0x00),
  ROW(F(Gpd) | F(Gpq) | F(KReg) | F(Mem), M(M32) | M(Any), 0, 0x00),
  ROW(F(Gpd) | F(Gpq) | F(Mem), M(M32) | M(Any), 0, 0x00),
  ROW(F(Gpq) | F(KReg) | F(Mem), M(M64) | M(Any), 0, 0x00),
  ROW(F(Gpw) | F(Gpd) | F(Gpq) | F(KReg) | F(Mem), M(M16) | M(Any), 0, 0x00),
  ROW(F(Gpw) | F(Gpd) | F(Gpq) | F(Mem), M(M16) | M(Any), 0, 0x00),
  ROW(F(Gpw) | F(Gpd) | F(Mem), M(M16) | M(Any), 0, 0x00),
  ROW(F(Gpd) | F(Gpq), 0, 0, 0x00),
  ROW(F(GpbLo) | F(GpbHi) | F(Gpw) | F(Mem), M(M8) | M(M16), 0, 0x00),
  ROW(F(Gpw) | F(Gpd) | F(Mem), M(M16) | M(M32), 0, 0x00),
  ROW(F(Mm) | F(Mem), M(M64) | M(Any), 0, 0x00),
  ROW(F(Mm) | F(Mem) | F(U8), M(M64) | M(Any), 0, 0x00),
  ROW(F(U16), 0, 0, 0x00),
  ROW(F(Xmm) | F(Mem), M(M32) | M(M64), 0, 0x00),
  ROW(F(Xmm) | F(Mem), M(M64), 0, 0x00),
  ROW(F(Xmm) | F(Ymm) | F(Mem), M(M128) | M(M256), 0, 0x00),
  ROW(F(Xmm) | F(U8), 0, 0, 0x00),
  ROW(F(Implicit) | F(Xmm), 0, 0, 0x01),
  ROW(F(Mem), M(Mib), 0, 0x00),
  ROW(F(Implicit) | F(Mem), M(BaseOnly) | M(Ds), 0, 0x01),
  ROW(F(Implicit) | F(Mem), M(BaseOnly) | M(Ds), 0, 0x40),
  ROW(F(Implicit) | F(Mem), M(BaseOnly) | M(Es), 0, 0x80),
  ROW(F(Implicit) | F(Gpq), 0, 0, 0x02),
  ROW(F(Implicit) | F(Gpq), 0, 0, 0x08),
  ROW(F(Implicit) | F(Gpd), 0, 0, 0x02),
  ROW(F(Implicit) | F(Gpd), 0, 0, 0x08),
  ROW(F(Gpd) | F(Gpq) | F(Mem), M(M32) | M(M64) | M(Any), 0, 0x00),
  ROW(F(Mem), M(M16) | M(M32), 0, 0x00),
  ROW(F(Mem), M(M16) | M(M32) | M(M64), 0, 0x00),
  ROW(F(St) | F(Mem), M(M32) | M(M64) | M(M80), 0, 0x00),
  ROW(F(GpbLo) | F(Gpw) | F(Gpd), 0, 0, 0x01),
  ROW(F(Gpw) | F(U8), 0, 0, 0x04),
  ROW(F(Mem), M(BaseOnly) | M(Es), 0, 0x80),
  ROW(F(Gpw), 0, 0, 0x04),
  ROW(F(Implicit) | F(GpbHi), 0, 0, 0x01),
  ROW(F(Gpw) | F(Gpd) | F(Gpq), 0, 0, 0x00),
  ROW(F(Mem), M(M8) | M(M16) | M(M32) | M(M48) | M(M64) | M(M80) | M(M128) | M(M256) | M(M512) | M(M1024) | M(Any), 0, 0x00),
  ROW(F(Implicit) | F(GpbLo) | F(Gpw) | F(Gpd) | F(Gpq), 0, 0, 0x01),
  ROW(F(Implicit) | F(Mem), M(BaseOnly) | M(Ds), 0, 0x80),
  ROW(F(Mem), M(BaseOnly) | M(Ds), 0, 0x40),
  ROW(F(Ymm) | F(Zmm), 0, 0, 0x00),
  ROW(F(Xmm) | F(Ymm) | F(Zmm), 0, 0, 0x00),
  ROW(F(Xmm) | F(Ymm) | F(Zmm) | F(Mem), M(M128) | M(M256) | M(M512), 0, 0x00),
  ROW(F(GpbLo) | F(GpbHi) | F(Gpw) | F(Gpd) | F(Gpq) | F(Xmm) | F(Mem), M(M8) | M(Any), 0, 0x00),
  ROW(F(Gpd) | F(Gpq) | F(Xmm) | F(Mem), M(M32) | M(Any), 0, 0x00),
  ROW(F(Gpw) | F(Gpd) | F(Gpq) | F(Xmm) | F(Mem), M(M16) | M(Any), 0, 0x00)
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

ASMJIT_FAVOR_SIZE Error InstInternal::validate(uint32_t archId, const BaseInst& inst, const Operand_* operands, uint32_t count) noexcept {
  // Only called when `archId` matches X86 family.
  ASMJIT_ASSERT(ArchInfo::isX86Family(archId));

  const X86ValidationData* vd;
  if (archId == ArchInfo::kIdX86)
    vd = &_x86ValidationData;
  else
    vd = &_x64ValidationData;

  uint32_t i;
  uint32_t archMask = InstDB::archMaskFromArchId(archId);

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

      if (ASMJIT_UNLIKELY(count < 1 || !operands[0].isMem()))
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

    if (ASMJIT_UNLIKELY((options & Inst::kOptionRep) && !(iFlags & InstDB::kFlagRep)))
      return DebugUtils::errored(kErrorInvalidRepPrefix);

    if (ASMJIT_UNLIKELY((options & Inst::kOptionRepne) && !(iFlags & InstDB::kFlagRepne)))
      return DebugUtils::errored(kErrorInvalidRepPrefix);

    // TODO: Validate extraReg {cx|ecx|rcx}.
  }

  // --------------------------------------------------------------------------
  // [Translate Each Operand to the Corresponding OpSignature]
  // --------------------------------------------------------------------------

  InstDB::OpSignature oSigTranslated[Globals::kMaxOpCount];
  uint32_t combinedOpFlags = 0;
  uint32_t combinedRegMask = 0;
  const Mem* memOp = nullptr;

  for (i = 0; i < count; i++) {
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

        // If `regId` is equal or greater than Operand::kPackedIdMin it means
        // that the register is virtual and its index will be assigned later
        // by the register allocator. We must pass unless asked to disallow
        // virtual registers.
        // TODO: We need an option to refuse virtual regs here.
        uint32_t regId = op.id();
        if (regId < Operand::kPackedIdMin) {
          if (ASMJIT_UNLIKELY(regId >= 32))
            return DebugUtils::errored(kErrorInvalidPhysId);

          if (ASMJIT_UNLIKELY(Support::bitTest(vd->allowedRegMask[regType], regId) == 0))
            return DebugUtils::errored(kErrorInvalidPhysId);

          regMask = Support::mask(regId);
          combinedRegMask |= regMask;
        }
        else {
          regMask = 0xFFFFFFFFu;
        }
        break;
      }

      // TODO: Validate base and index and combine with `combinedRegMask`.
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
            // Home address of virtual register. In such case we don't want to
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
          if (baseId < Operand::kPackedIdMin) {
            // Physical base id.
            regMask = Support::mask(baseId);
            combinedRegMask |= regMask;
          }
          else {
            // Virtual base id - will the whole mask for implicit mem validation.
            // The register is not assigned yet, so we cannot predict the phys id.
            regMask = 0xFFFFFFFFu;
          }

          if (!indexType && !m.offsetLo32())
            memFlags |= InstDB::kMemOpBaseOnly;
        }
        else {
          // Base is an address, make sure that the address doesn't overflow 32-bit
          // integer (either int32_t or uint32_t) in 32-bit targets.
          int64_t offset = m.offset();
          if (archMask == InstDB::kArchMaskX86 && !Support::isI32(offset) && !Support::isU32(offset))
            return DebugUtils::errored(kErrorInvalidAddress);
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
          if (indexId < Operand::kPackedIdMin)
            combinedRegMask |= Support::mask(indexId);

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
          if (immValue <= 0xFu)
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
          if (immValue <= 0x80u)
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
  // as Assembler and BaseCompiler may just pass more operands where some of
  // them are none (it means that no operand is given at that index). However,
  // validate that there are no gaps (like [reg, none, reg] or [none, reg]).
  if (i < count) {
    while (--count > i)
      if (ASMJIT_UNLIKELY(!operands[count].isNone()))
        return DebugUtils::errored(kErrorInvalidInstruction);
  }

  // Validate X86 and X64 specific cases.
  if (archMask == InstDB::kArchMaskX86) {
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
      if ((iSig->archMask & archMask) == 0) continue;

      // Compare the operands table with reference operands.
      uint32_t j = 0;
      uint32_t iSigCount = iSig->opCount;
      bool localImmOutOfRange = false;

      if (iSigCount == count) {
        for (j = 0; j < count; j++)
          if (!x86CheckOSig(oSigTranslated[j], opSignatureTable[iSig->operands[j]], localImmOutOfRange))
            break;
      }
      else if (iSigCount - iSig->implicit == count) {
        uint32_t r = 0;
        for (j = 0; j < count && r < iSigCount; j++, r++) {
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

      if (j == count) {
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

  if (!extraReg.isNone() || (options & kAvx512Options)) {
    if (commonInfo.hasFlag(InstDB::kFlagEvex)) {
      // Validate AVX-512 {k} and {k}{z}.
      if (!extraReg.isNone()) {
        // Mask can only be specified by a 'k' register.
        if (ASMJIT_UNLIKELY(extraReg.type() != Reg::kTypeKReg))
          return DebugUtils::errored(kErrorInvalidKMaskReg);

        if (ASMJIT_UNLIKELY(extraReg.id() == 0 || !commonInfo.hasAvx512K()))
          return DebugUtils::errored(kErrorInvalidKMaskUse);
      }

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
          ASMJIT_ASSERT(count >= 2);
          if (ASMJIT_UNLIKELY(!x86IsZmmOrM512(operands[0]) && !x86IsZmmOrM512(operands[1])))
            return DebugUtils::errored(kErrorInvalidEROrSAE);
        }
      }
    }
    else {
      // Not AVX512 instruction - maybe OpExtra is xCX register used by REP/REPNE prefix. Otherwise the instruction is invalid.
      if ((options & kAvx512Options) || (options & (Inst::kOptionRep | Inst::kOptionRepne)) == 0)
        return DebugUtils::errored(kErrorInvalidInstruction);
    }
  }

  return kErrorOk;
}
#endif

// ============================================================================
// [asmjit::x86::InstInternal - QueryRWInfo]
// ============================================================================

#ifndef ASMJIT_DISABLE_INST_API
ASMJIT_FAVOR_SIZE Error InstInternal::queryRWInfo(uint32_t archId, const BaseInst& inst, const Operand_* operands, uint32_t count, InstRWInfo& out) noexcept {
  // Only called when `archId` matches X86 family.
  ASMJIT_ASSERT(ArchInfo::isX86Family(archId));

  // Get the instruction data.
  uint32_t instId = inst.id();
  if ((ASMJIT_UNLIKELY(!Inst::isDefinedId(instId))))
    return DebugUtils::errored(kErrorInvalidInstruction);

  // TODO:


  return kErrorInvalidInstruction;
}
#endif // ASMJIT_DISABLE_INST_API

// ============================================================================
// [asmjit::x86::InstInternal - QueryFeatures]
// ============================================================================

#ifndef ASMJIT_DISABLE_INST_API
ASMJIT_FAVOR_SIZE static uint32_t x86GetRegTypesMask(const Operand_* operands, uint32_t count) noexcept {
  uint32_t mask = 0;
  for (uint32_t i = 0; i < count; i++) {
    const Operand_& op = operands[i];
    if (op.isReg()) {
      const BaseReg& reg = op.as<BaseReg>();
      mask |= Support::mask(reg.type());
    }
    else if (op.isMem()) {
      const BaseMem& mem = op.as<BaseMem>();
      if (mem.hasBaseReg()) mask |= Support::mask(mem.baseType());
      if (mem.hasIndexReg()) mask |= Support::mask(mem.indexType());
    }
  }
  return mask;
}

ASMJIT_FAVOR_SIZE Error InstInternal::queryFeatures(uint32_t archId, const BaseInst& inst, const Operand_* operands, uint32_t count, BaseFeatures& out) noexcept {
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

  // Since AsmJit merges all instructions that share the same name we have to
  // deal with some special cases and also with MMX/SSE and AVX/AVX2 overlaps.
  if (fData != executionInfo.featuresData()) {
    uint32_t mask = x86GetRegTypesMask(operands, count);

    // Handle MMX vs SSE overlap.
    if (out.has(Features::kMMX) || out.has(Features::kMMX2)) {
      // Only instructions defined by SSE and SSE2 overlap. Instructions introduced
      // by newer instruction sets like SSE3+ don't state MMX as they require SSE3+.
      if (out.has(Features::kSSE) || out.has(Features::kSSE2)) {
        if (!Support::bitTest(mask, Reg::kTypeXmm)) {
          // The instruction doesn't use XMM register(s), thus it's MMX/MMX2 only.
          out.remove(Features::kSSE);
          out.remove(Features::kSSE2);
        }
        else {
          out.remove(Features::kMMX);
          out.remove(Features::kMMX2);
        }

        // Special case: PEXTRW instruction is MMX/SSE2 instruction. However, this
        // instruction couldn't access memory (only register to register extract) so
        // when SSE4.1 introduced the whole family of PEXTR/PINSR instructions they
        // also introduced PEXTRW with a new opcode 0x15 that can extract directly to
        // memory. This instruction is, of course, not compatible with MMX/SSE2 one.
        if (instId == Inst::kIdPextrw) {
          ASMJIT_ASSERT(out.has(Features::kSSE2));
          ASMJIT_ASSERT(out.has(Features::kSSE4_1));

          if (count > 0 && operands[0].isMem())
            out.remove(Features::kSSE2);
          else
            out.remove(Features::kSSE4_1);
        }
      }
    }

    // Handle PCLMULQDQ vs VPCLMULQDQ.
    if (out.has(Features::kVPCLMULQDQ)) {
      if (Support::bitTest(mask, Reg::kTypeZmm) || Support::bitTest(options, Inst::kOptionEvex)) {
        // AVX512_F & VPCLMULQDQ.
        out.remove(Features::kAVX,
                   Features::kPCLMULQDQ);
      }
      else if (Support::bitTest(mask, Reg::kTypeYmm)) {
        out.remove(Features::kAVX512_F,
                   Features::kAVX512_VL);
      }
      else {
        // AVX & PCLMULQDQ.
        out.remove(Features::kAVX512_F,
                   Features::kAVX512_VL,
                   Features::kVPCLMULQDQ);
      }
    }

    // Handle AVX vs AVX2 overlap.
    if (out.has(Features::kAVX) && out.has(Features::kAVX2)) {
      bool isAVX2 = true;
      // Special case: VBROADCASTSS and VBROADCASTSD were introduced in AVX, but
      // only version that uses memory as a source operand. AVX2 then added support
      // for register source operand.
      if (instId == Inst::kIdVbroadcastss || instId == Inst::kIdVbroadcastsd) {
        if (count > 1 && operands[0].isMem())
          isAVX2 = false;
      }
      else {
        // AVX instruction set doesn't support integer operations on YMM registers
        // as these were later introcuced by AVX2. In our case we have to check if
        // YMM register(s) are in use and if that is the case this is an AVX2 instruction.
        if (!(mask & Support::mask(Reg::kTypeYmm, Reg::kTypeZmm)))
          isAVX2 = false;
      }

      if (isAVX2)
        out.remove(Features::kAVX);
      else
        out.remove(Features::kAVX2);
    }

    // Handle AVX|AVX2|FMA|F16C vs AVX512 overlap.
    if (out.has(Features::kAVX) || out.has(Features::kAVX2) || out.has(Features::kFMA) || out.has(Features::kF16C)) {
      // Only AVX512-F|BW|DQ allow to encode AVX/AVX2 instructions
      if (out.has(Features::kAVX512_F) || out.has(Features::kAVX512_BW) || out.has(Features::kAVX512_DQ)) {
        uint32_t kAvx512Options = Inst::kOptionEvex | Inst::_kOptionAvx512Mask;
        if (!(mask & Support::mask(Reg::kTypeZmm, Reg::kTypeKReg)) && !(options & (kAvx512Options)) && inst.extraReg().type() != Reg::kTypeKReg) {
          out.remove(Features::kAVX512_F,
                     Features::kAVX512_BW,
                     Features::kAVX512_DQ,
                     Features::kAVX512_VL);
        }
      }
    }

    // Clear AVX512_VL if ZMM register is used.
    if (Support::bitTest(mask, Reg::kTypeZmm))
      out.remove(Features::kAVX512_VL);
  }

  return kErrorOk;
}
#endif

// ============================================================================
// [asmjit::Inst - Unit]
// ============================================================================

#if defined(ASMJIT_BUILD_TEST)
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

#if defined(ASMJIT_BUILD_TEST) && !defined(ASMJIT_DISABLE_TEXT)
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

// [Guard]
#endif // ASMJIT_BUILD_X86
