// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
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
#include "../asmjit_build.h"
#if defined(ASMJIT_BUILD_X86)

// [Dependencies]
#include "../base/utils.h"
#include "../x86/x86inst.h"
#include "../x86/x86operand.h"

// [Api-Begin]
#include "../asmjit_apibegin.h"

namespace asmjit {

// ============================================================================
// [Enums (Internal)]
// ============================================================================

//! \internal
enum ODATA_ {
  // PREFIX.
  ODATA_000000  = X86Inst::kOpCode_PP_00 | X86Inst::kOpCode_MM_00,
  ODATA_000F00  = X86Inst::kOpCode_PP_00 | X86Inst::kOpCode_MM_0F,
  ODATA_000F01  = X86Inst::kOpCode_PP_00 | X86Inst::kOpCode_MM_0F01,
  ODATA_000F38  = X86Inst::kOpCode_PP_00 | X86Inst::kOpCode_MM_0F38,
  ODATA_000F3A  = X86Inst::kOpCode_PP_00 | X86Inst::kOpCode_MM_0F3A,
  ODATA_660000  = X86Inst::kOpCode_PP_66 | X86Inst::kOpCode_MM_00,
  ODATA_660F00  = X86Inst::kOpCode_PP_66 | X86Inst::kOpCode_MM_0F,
  ODATA_660F38  = X86Inst::kOpCode_PP_66 | X86Inst::kOpCode_MM_0F38,
  ODATA_660F3A  = X86Inst::kOpCode_PP_66 | X86Inst::kOpCode_MM_0F3A,
  ODATA_F20000  = X86Inst::kOpCode_PP_F2 | X86Inst::kOpCode_MM_00,
  ODATA_F20F00  = X86Inst::kOpCode_PP_F2 | X86Inst::kOpCode_MM_0F,
  ODATA_F20F38  = X86Inst::kOpCode_PP_F2 | X86Inst::kOpCode_MM_0F38,
  ODATA_F20F3A  = X86Inst::kOpCode_PP_F2 | X86Inst::kOpCode_MM_0F3A,
  ODATA_F30000  = X86Inst::kOpCode_PP_F3 | X86Inst::kOpCode_MM_00,
  ODATA_F30F00  = X86Inst::kOpCode_PP_F3 | X86Inst::kOpCode_MM_0F,
  ODATA_F30F38  = X86Inst::kOpCode_PP_F3 | X86Inst::kOpCode_MM_0F38,
  ODATA_F30F3A  = X86Inst::kOpCode_PP_F3 | X86Inst::kOpCode_MM_0F3A,
  ODATA_000F0F  = X86Inst::kOpCode_PP_00 | X86Inst::kOpCode_MM_0F, // 3DNOW, special case.

  ODATA_FPU_00  = X86Inst::kOpCode_PP_00,
  ODATA_FPU_9B  = X86Inst::kOpCode_PP_9B,

  ODATA_XOP_M8  = X86Inst::kOpCode_MM_XOP08,
  ODATA_XOP_M9  = X86Inst::kOpCode_MM_XOP09,

  ODATA_O__     = 0,
  ODATA_O_0     = 0 << X86Inst::kOpCode_O_Shift,
  ODATA_O_1     = 1 << X86Inst::kOpCode_O_Shift,
  ODATA_O_2     = 2 << X86Inst::kOpCode_O_Shift,
  ODATA_O_3     = 3 << X86Inst::kOpCode_O_Shift,
  ODATA_O_4     = 4 << X86Inst::kOpCode_O_Shift,
  ODATA_O_5     = 5 << X86Inst::kOpCode_O_Shift,
  ODATA_O_6     = 6 << X86Inst::kOpCode_O_Shift,
  ODATA_O_7     = 7 << X86Inst::kOpCode_O_Shift,

  // REX/VEX.
  ODATA_LL__    = 0,                                  // L is unspecified.
  ODATA_LL_x    = 0,                                  // L is based on operand(s).
  ODATA_LL_I    = 0,                                  // L is ignored (LIG).
  ODATA_LL_0    = 0,                                  // L has to be zero (L.128).
  ODATA_LL_1    = X86Inst::kOpCode_LL_256,            // L has to be one (L.256).
  ODATA_LL_2    = X86Inst::kOpCode_LL_512,            // L has to be two (L.512).

  ODATA_W__     = 0,                                  // W is unspecified.
  ODATA_W_x     = 0,                                  // W is based on operand(s).
  ODATA_W_I     = 0,                                  // W is ignored (WIG).
  ODATA_W_0     = 0,                                  // W has to be zero (W0).
  ODATA_W_1     = X86Inst::kOpCode_W,                 // W has to be one (W1).

  // EVEX.
  ODATA_EvexW__ = 0,                                  // Not EVEX instruction.
  ODATA_EvexW_x = 0,                                  // EVEX.W is based on operand(s).
  ODATA_EvexW_I = 0,                                  // EVEX.W is ignored     (EVEX.WIG).
  ODATA_EvexW_0 = 0,                                  // EVEX.W has to be zero (EVEX.W0).
  ODATA_EvexW_1 = X86Inst::kOpCode_EW,                // EVEX.W has to be one  (EVEX.W1).

  ODATA_N__      = 0,                                 // Base element size not used.
  ODATA_N_0      = 0 << X86Inst::kOpCode_CDSHL_Shift, // N << 0 (BYTE).
  ODATA_N_1      = 1 << X86Inst::kOpCode_CDSHL_Shift, // N << 1 (WORD).
  ODATA_N_2      = 2 << X86Inst::kOpCode_CDSHL_Shift, // N << 2 (DWORD).
  ODATA_N_3      = 3 << X86Inst::kOpCode_CDSHL_Shift, // N << 3 (QWORD).
  ODATA_N_4      = 4 << X86Inst::kOpCode_CDSHL_Shift, // N << 4 (OWORD).
  ODATA_N_5      = 5 << X86Inst::kOpCode_CDSHL_Shift, // N << 5 (YWORD).

  ODATA_TT__     = 0,
  ODATA_TT_FV    = X86Inst::kOpCode_CDTT_FV,
  ODATA_TT_HV    = X86Inst::kOpCode_CDTT_HV,
  ODATA_TT_FVM   = X86Inst::kOpCode_CDTT_FVM,
  ODATA_TT_T1S   = X86Inst::kOpCode_CDTT_T1S,
  ODATA_TT_T1F   = X86Inst::kOpCode_CDTT_T1F,
  ODATA_TT_T1W   = X86Inst::kOpCode_CDTT_T1W,
  ODATA_TT_T2    = X86Inst::kOpCode_CDTT_T2,
  ODATA_TT_T4    = X86Inst::kOpCode_CDTT_T4,
  ODATA_TT_T8    = X86Inst::kOpCode_CDTT_T8,
  ODATA_TT_HVM   = X86Inst::kOpCode_CDTT_HVM,
  ODATA_TT_OVM   = X86Inst::kOpCode_CDTT_OVM,
  ODATA_TT_QVM   = X86Inst::kOpCode_CDTT_QVM,
  ODATA_TT_128   = X86Inst::kOpCode_CDTT_128,
  ODATA_TT_DUP   = X86Inst::kOpCode_CDTT_DUP
};

// ============================================================================
// [asmjit::X86Inst]
// ============================================================================

// Instruction opcode definitions:
//   - `O` encodes X86|MMX|SSE instructions.
//   - `V` encodes VEX|XOP|EVEX instructions.
#define O_ENCODE(VEX, PREFIX, OPCODE, O, L, W, EvexW, N, TT) \
  ((PREFIX) | (OPCODE) | (O) | (L) | (W) | (EvexW) | (N) | (TT) | \
   (VEX && ((PREFIX) & X86Inst::kOpCode_MM_Mask) != X86Inst::kOpCode_MM_0F ? int(X86Inst::kOpCode_MM_ForceVex3) : 0))

#define O(PREFIX, OPCODE, O, LL, W, EvexW, N, TT) (O_ENCODE(0, ODATA_##PREFIX, 0x##OPCODE, ODATA_O_##O, ODATA_LL_##LL, ODATA_W_##W, ODATA_EvexW_##EvexW, ODATA_N_##N, ODATA_TT_##TT))
#define V(PREFIX, OPCODE, O, LL, W, EvexW, N, TT) (O_ENCODE(1, ODATA_##PREFIX, 0x##OPCODE, ODATA_O_##O, ODATA_LL_##LL, ODATA_W_##W, ODATA_EvexW_##EvexW, ODATA_N_##N, ODATA_TT_##TT))

#define O_FPU(PREFIX, OPCODE, O) (ODATA_FPU_##PREFIX | (0x##OPCODE & 0xFFU) | ((0x##OPCODE >> 8) << X86Inst::kOpCode_FPU_2B_Shift) | ODATA_O_##O)

#define F(FLAG) X86Inst::kInstFlag##FLAG            // Instruction Base Flag(s) `F(...)`.
#define EF(EFLAGS) 0                                // Instruction EFLAGS `EF(OSZAPCDX)`.

// Don't store `_nameDataIndex` if instruction names are disabled. Since some
// APIs can use `_nameDataIndex` it's much safer if it's zero if it's not used.
#if defined(ASMJIT_DISABLE_TEXT)
# define NAME_DATA_INDEX(X) 0
#else
# define NAME_DATA_INDEX(X) X
#endif

// Defines an X86/X64 instruction.
#define INST(id, encoding, opcode0, opcode1, instFlags, eflags, writeIndex, writeSize, familyType, familyIndex, nameDataIndex, commonIndex) { \
  uint32_t(X86Inst::kEncoding##encoding),   \
  uint32_t(NAME_DATA_INDEX(nameDataIndex)), \
  uint32_t(commonIndex),                    \
  uint32_t(X86Inst::familyType),            \
  uint32_t(familyIndex),                    \
  0,                                        \
  opcode0                                   \
}

const X86Inst X86InstDB::instData[] = {
  // <-----------------+------------------------+------------------+--------+------------------+--------+---------------------------------------+-------------+-------+-----------------+-----+----+
  //                   |                    |    Main OpCode   |#0 EVEX |Alternative OpCode|#1 EVEX |                                       |   E-FLAGS   | Write |                 |     |    |
  //    Instruction    |   Inst. Encoding   |                  +--------+                  +--------+          Instruction Flags            +-------------+---+---+ Family Type/Idx.+NameX|ComX|
  //                   |                    |#0:PP-MMM OP/O L|W|W|N|TT. |#1:PP-MMM OP/O L|W|W|N|TT. |                                       | EF:OSZAPCDX |Idx|Cnt|                 |     |    |
  // <-----------------+--------------------+------------------+--------+------------------+--------+---------------------------------------+-------------+---+---+-----------------+-----+----+
  // ${instData:Begin}
  INST(None            , None               , 0                         , 0                         , 0                                     , EF(________), 0 , 0 , kFamilyNone, 0  , 0   , 0  ),
  INST(Aaa             , X86Op_xAX          , O(000000,37,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(UUUWUW__), 0 , 0 , kFamilyNone, 0  , 1   , 1  ),
  INST(Aad             , X86I_xAX           , O(000000,D5,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(UWWUWU__), 0 , 0 , kFamilyNone, 0  , 5   , 2  ),
  INST(Aam             , X86I_xAX           , O(000000,D4,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(UWWUWU__), 0 , 0 , kFamilyNone, 0  , 9   , 2  ),
  INST(Aas             , X86Op_xAX          , O(000000,3F,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(UUUWUW__), 0 , 0 , kFamilyNone, 0  , 13  , 1  ),
  INST(Adc             , X86Arith           , O(000000,10,2,_,x,_,_,_  ), 0                         , F(RW)|F(Lock)                         , EF(WWWWWX__), 0 , 0 , kFamilyNone, 0  , 17  , 3  ),
  INST(Adcx            , X86Rm              , O(660F38,F6,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(_____X__), 0 , 0 , kFamilyNone, 0  , 21  , 4  ),
  INST(Add             , X86Arith           , O(000000,00,0,_,x,_,_,_  ), 0                         , F(RW)|F(Lock)                         , EF(WWWWWW__), 0 , 0 , kFamilyNone, 0  , 678 , 5  ),
  INST(Addpd           , ExtRm              , O(660F00,58,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 0  , 4357, 6  ),
  INST(Addps           , ExtRm              , O(000F00,58,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 1  , 4369, 6  ),
  INST(Addsd           , ExtRm              , O(F20F00,58,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 0  , 4591, 7  ),
  INST(Addss           , ExtRm              , O(F30F00,58,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 1  , 4601, 8  ),
  INST(Addsubpd        , ExtRm              , O(660F00,D0,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 2  , 4096, 6  ),
  INST(Addsubps        , ExtRm              , O(F20F00,D0,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 2  , 4108, 6  ),
  INST(Adox            , X86Rm              , O(F30F38,F6,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(X_______), 0 , 0 , kFamilyNone, 0  , 26  , 9  ),
  INST(Aesdec          , ExtRm              , O(660F38,DE,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 3  , 2602, 6  ),
  INST(Aesdeclast      , ExtRm              , O(660F38,DF,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 3  , 2610, 6  ),
  INST(Aesenc          , ExtRm              , O(660F38,DC,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 3  , 2622, 6  ),
  INST(Aesenclast      , ExtRm              , O(660F38,DD,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 3  , 2630, 6  ),
  INST(Aesimc          , ExtRm              , O(660F38,DB,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 0 , kFamilySse , 4  , 2642, 10 ),
  INST(Aeskeygenassist , ExtRmi             , O(660F3A,DF,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 0 , kFamilySse , 4  , 2650, 11 ),
  INST(And             , X86Arith           , O(000000,20,4,_,x,_,_,_  ), 0                         , F(RW)|F(Lock)                         , EF(WWWUWW__), 0 , 0 , kFamilyNone, 0  , 2161, 12 ),
  INST(Andn            , VexRvm_Wx          , V(000F38,F2,_,0,x,_,_,_  ), 0                         , F(RW)                                 , EF(WWWUUW__), 0 , 0 , kFamilyNone, 0  , 5867, 13 ),
  INST(Andnpd          , ExtRm              , O(660F00,55,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 5  , 2683, 6  ),
  INST(Andnps          , ExtRm              , O(000F00,55,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 6  , 2691, 6  ),
  INST(Andpd           , ExtRm              , O(660F00,54,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 5  , 3610, 14 ),
  INST(Andps           , ExtRm              , O(000F00,54,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 6  , 3620, 14 ),
  INST(Bextr           , VexRmv_Wx          , V(000F38,F7,_,0,x,_,_,_  ), 0                         , F(RW)                                 , EF(WUWUUW__), 0 , 0 , kFamilyNone, 0  , 31  , 15 ),
  INST(Blcfill         , VexVm_Wx           , V(XOP_M9,01,1,0,x,_,_,_  ), 0                         , F(WO)                                 , EF(WWWUUW__), 0 , 0 , kFamilyNone, 0  , 37  , 16 ),
  INST(Blci            , VexVm_Wx           , V(XOP_M9,02,6,0,x,_,_,_  ), 0                         , F(WO)                                 , EF(WWWUUW__), 0 , 0 , kFamilyNone, 0  , 45  , 16 ),
  INST(Blcic           , VexVm_Wx           , V(XOP_M9,01,5,0,x,_,_,_  ), 0                         , F(WO)                                 , EF(WWWUUW__), 0 , 0 , kFamilyNone, 0  , 50  , 16 ),
  INST(Blcmsk          , VexVm_Wx           , V(XOP_M9,02,1,0,x,_,_,_  ), 0                         , F(WO)                                 , EF(WWWUUW__), 0 , 0 , kFamilyNone, 0  , 56  , 16 ),
  INST(Blcs            , VexVm_Wx           , V(XOP_M9,01,3,0,x,_,_,_  ), 0                         , F(WO)                                 , EF(WWWUUW__), 0 , 0 , kFamilyNone, 0  , 63  , 16 ),
  INST(Blendpd         , ExtRmi             , O(660F3A,0D,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 7  , 2769, 17 ),
  INST(Blendps         , ExtRmi             , O(660F3A,0C,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 7  , 2778, 17 ),
  INST(Blendvpd        , ExtRm_XMM0         , O(660F38,15,_,_,_,_,_,_  ), 0                         , F(RW)|F(Special)                      , EF(________), 0 , 0 , kFamilySse , 8  , 2787, 18 ),
  INST(Blendvps        , ExtRm_XMM0         , O(660F38,14,_,_,_,_,_,_  ), 0                         , F(RW)|F(Special)                      , EF(________), 0 , 0 , kFamilySse , 8  , 2797, 18 ),
  INST(Blsfill         , VexVm_Wx           , V(XOP_M9,01,2,0,x,_,_,_  ), 0                         , F(WO)                                 , EF(WWWUUW__), 0 , 0 , kFamilyNone, 0  , 68  , 16 ),
  INST(Blsi            , VexVm_Wx           , V(000F38,F3,3,0,x,_,_,_  ), 0                         , F(RW)                                 , EF(WWWUUW__), 0 , 0 , kFamilyNone, 0  , 76  , 19 ),
  INST(Blsic           , VexVm_Wx           , V(XOP_M9,01,6,0,x,_,_,_  ), 0                         , F(WO)                                 , EF(WWWUUW__), 0 , 0 , kFamilyNone, 0  , 81  , 16 ),
  INST(Blsmsk          , VexVm_Wx           , V(000F38,F3,2,0,x,_,_,_  ), 0                         , F(RW)                                 , EF(WWWUUW__), 0 , 0 , kFamilyNone, 0  , 87  , 19 ),
  INST(Blsr            , VexVm_Wx           , V(000F38,F3,1,0,x,_,_,_  ), 0                         , F(RW)                                 , EF(WWWUUW__), 0 , 0 , kFamilyNone, 0  , 94  , 19 ),
  INST(Bsf             , X86Rm              , O(000F00,BC,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(UUWUUU__), 0 , 0 , kFamilyNone, 0  , 99  , 20 ),
  INST(Bsr             , X86Rm              , O(000F00,BD,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(UUWUUU__), 0 , 0 , kFamilyNone, 0  , 103 , 20 ),
  INST(Bswap           , X86Bswap           , O(000F00,C8,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 107 , 21 ),
  INST(Bt              , X86Bt              , O(000F00,A3,_,_,x,_,_,_  ), O(000F00,BA,4,_,x,_,_,_  ), F(RO)                                 , EF(UU_UUW__), 0 , 0 , kFamilyNone, 0  , 113 , 22 ),
  INST(Btc             , X86Bt              , O(000F00,BB,_,_,x,_,_,_  ), O(000F00,BA,7,_,x,_,_,_  ), F(RW)|F(Lock)                         , EF(UU_UUW__), 0 , 0 , kFamilyNone, 0  , 116 , 23 ),
  INST(Btr             , X86Bt              , O(000F00,B3,_,_,x,_,_,_  ), O(000F00,BA,6,_,x,_,_,_  ), F(RW)|F(Lock)                         , EF(UU_UUW__), 0 , 0 , kFamilyNone, 0  , 120 , 24 ),
  INST(Bts             , X86Bt              , O(000F00,AB,_,_,x,_,_,_  ), O(000F00,BA,5,_,x,_,_,_  ), F(RW)|F(Lock)                         , EF(UU_UUW__), 0 , 0 , kFamilyNone, 0  , 124 , 25 ),
  INST(Bzhi            , VexRmv_Wx          , V(000F38,F5,_,0,x,_,_,_  ), 0                         , F(RW)                                 , EF(WWWUUW__), 0 , 0 , kFamilyNone, 0  , 128 , 15 ),
  INST(Call            , X86Call            , O(000000,FF,2,_,_,_,_,_  ), 0                         , F(RW)|F(Volatile)                     , EF(________), 0 , 0 , kFamilyNone, 0  , 133 , 26 ),
  INST(Cbw             , X86Op_xAX          , O(660000,98,_,_,_,_,_,_  ), 0                         , F(RW)|F(Special)                      , EF(________), 0 , 0 , kFamilyNone, 0  , 138 , 27 ),
  INST(Cdq             , X86Op_xDX_xAX      , O(000000,99,_,_,_,_,_,_  ), 0                         , F(RW)|F(Special)                      , EF(________), 0 , 0 , kFamilyNone, 0  , 142 , 28 ),
  INST(Cdqe            , X86Op_xAX          , O(000000,98,_,_,1,_,_,_  ), 0                         , F(RW)|F(Special)                      , EF(________), 0 , 0 , kFamilyNone, 0  , 146 , 29 ),
  INST(Clac            , X86Op              , O(000F01,CA,_,_,_,_,_,_  ), 0                         , F(Volatile)                           , EF(___W____), 0 , 0 , kFamilyNone, 0  , 151 , 30 ),
  INST(Clc             , X86Op              , O(000000,F8,_,_,_,_,_,_  ), 0                         , F(Volatile)                           , EF(_____W__), 0 , 0 , kFamilyNone, 0  , 156 , 31 ),
  INST(Cld             , X86Op              , O(000000,FC,_,_,_,_,_,_  ), 0                         , F(Volatile)                           , EF(______W_), 0 , 0 , kFamilyNone, 0  , 160 , 32 ),
  INST(Clflush         , X86M_Only          , O(000F00,AE,7,_,_,_,_,_  ), 0                         , F(RO)|F(Volatile)                     , EF(________), 0 , 0 , kFamilyNone, 0  , 164 , 33 ),
  INST(Clflushopt      , X86M_Only          , O(660F00,AE,7,_,_,_,_,_  ), 0                         , F(RO)|F(Volatile)                     , EF(________), 0 , 0 , kFamilyNone, 0  , 172 , 33 ),
  INST(Clwb            , X86M_Only          , O(660F00,AE,6,_,_,_,_,_  ), 0                         , F(RO)|F(Volatile)                     , EF(________), 0 , 0 , kFamilyNone, 0  , 183 , 33 ),
  INST(Clzero          , X86Op_ZAX          , O(000F01,FC,_,_,_,_,_,_  ), 0                         , F(WO)|F(Volatile)|F(Special)          , EF(________), 0 , 0 , kFamilyNone, 0  , 188 , 34 ),
  INST(Cmc             , X86Op              , O(000000,F5,_,_,_,_,_,_  ), 0                         , 0                                     , EF(_____X__), 0 , 0 , kFamilyNone, 0  , 195 , 35 ),
  INST(Cmova           , X86Rm              , O(000F00,47,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(__R__R__), 0 , 0 , kFamilyNone, 0  , 199 , 36 ),
  INST(Cmovae          , X86Rm              , O(000F00,43,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(_____R__), 0 , 0 , kFamilyNone, 0  , 205 , 37 ),
  INST(Cmovb           , X86Rm              , O(000F00,42,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(_____R__), 0 , 0 , kFamilyNone, 0  , 535 , 37 ),
  INST(Cmovbe          , X86Rm              , O(000F00,46,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(__R__R__), 0 , 0 , kFamilyNone, 0  , 542 , 36 ),
  INST(Cmovc           , X86Rm              , O(000F00,42,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(_____R__), 0 , 0 , kFamilyNone, 0  , 212 , 37 ),
  INST(Cmove           , X86Rm              , O(000F00,44,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(__R_____), 0 , 0 , kFamilyNone, 0  , 550 , 38 ),
  INST(Cmovg           , X86Rm              , O(000F00,4F,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(RRR_____), 0 , 0 , kFamilyNone, 0  , 218 , 39 ),
  INST(Cmovge          , X86Rm              , O(000F00,4D,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(RR______), 0 , 0 , kFamilyNone, 0  , 224 , 40 ),
  INST(Cmovl           , X86Rm              , O(000F00,4C,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(RR______), 0 , 0 , kFamilyNone, 0  , 231 , 40 ),
  INST(Cmovle          , X86Rm              , O(000F00,4E,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(RRR_____), 0 , 0 , kFamilyNone, 0  , 237 , 39 ),
  INST(Cmovna          , X86Rm              , O(000F00,46,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(__R__R__), 0 , 0 , kFamilyNone, 0  , 244 , 36 ),
  INST(Cmovnae         , X86Rm              , O(000F00,42,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(_____R__), 0 , 0 , kFamilyNone, 0  , 251 , 37 ),
  INST(Cmovnb          , X86Rm              , O(000F00,43,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(_____R__), 0 , 0 , kFamilyNone, 0  , 557 , 37 ),
  INST(Cmovnbe         , X86Rm              , O(000F00,47,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(__R__R__), 0 , 0 , kFamilyNone, 0  , 565 , 36 ),
  INST(Cmovnc          , X86Rm              , O(000F00,43,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(_____R__), 0 , 0 , kFamilyNone, 0  , 259 , 37 ),
  INST(Cmovne          , X86Rm              , O(000F00,45,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(__R_____), 0 , 0 , kFamilyNone, 0  , 574 , 38 ),
  INST(Cmovng          , X86Rm              , O(000F00,4E,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(RRR_____), 0 , 0 , kFamilyNone, 0  , 266 , 39 ),
  INST(Cmovnge         , X86Rm              , O(000F00,4C,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(RR______), 0 , 0 , kFamilyNone, 0  , 273 , 40 ),
  INST(Cmovnl          , X86Rm              , O(000F00,4D,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(RR______), 0 , 0 , kFamilyNone, 0  , 281 , 40 ),
  INST(Cmovnle         , X86Rm              , O(000F00,4F,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(RRR_____), 0 , 0 , kFamilyNone, 0  , 288 , 39 ),
  INST(Cmovno          , X86Rm              , O(000F00,41,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(R_______), 0 , 0 , kFamilyNone, 0  , 296 , 41 ),
  INST(Cmovnp          , X86Rm              , O(000F00,4B,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(____R___), 0 , 0 , kFamilyNone, 0  , 303 , 42 ),
  INST(Cmovns          , X86Rm              , O(000F00,49,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(_R______), 0 , 0 , kFamilyNone, 0  , 310 , 43 ),
  INST(Cmovnz          , X86Rm              , O(000F00,45,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(__R_____), 0 , 0 , kFamilyNone, 0  , 317 , 38 ),
  INST(Cmovo           , X86Rm              , O(000F00,40,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(R_______), 0 , 0 , kFamilyNone, 0  , 324 , 41 ),
  INST(Cmovp           , X86Rm              , O(000F00,4A,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(____R___), 0 , 0 , kFamilyNone, 0  , 330 , 42 ),
  INST(Cmovpe          , X86Rm              , O(000F00,4A,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(____R___), 0 , 0 , kFamilyNone, 0  , 336 , 42 ),
  INST(Cmovpo          , X86Rm              , O(000F00,4B,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(____R___), 0 , 0 , kFamilyNone, 0  , 343 , 42 ),
  INST(Cmovs           , X86Rm              , O(000F00,48,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(_R______), 0 , 0 , kFamilyNone, 0  , 350 , 43 ),
  INST(Cmovz           , X86Rm              , O(000F00,44,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(__R_____), 0 , 0 , kFamilyNone, 0  , 356 , 38 ),
  INST(Cmp             , X86Arith           , O(000000,38,7,_,x,_,_,_  ), 0                         , F(RO)                                 , EF(WWWWWW__), 0 , 0 , kFamilyNone, 0  , 362 , 44 ),
  INST(Cmppd           , ExtRmi             , O(660F00,C2,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 9  , 3023, 17 ),
  INST(Cmpps           , ExtRmi             , O(000F00,C2,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 10 , 3030, 17 ),
  INST(Cmps            , X86StrMm           , O(000000,A6,_,_,_,_,_,_  ), 0                         , F(RW)|F(Special)|F(Rep)|F(Repnz)      , EF(WWWWWWR_), 0 , 0 , kFamilyNone, 0  , 366 , 45 ),
  INST(Cmpsd           , ExtRmi             , O(F20F00,C2,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 11 , 3037, 46 ),
  INST(Cmpss           , ExtRmi             , O(F30F00,C2,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 12 , 3044, 47 ),
  INST(Cmpxchg         , X86Cmpxchg         , O(000F00,B0,_,_,x,_,_,_  ), 0                         , F(RW)|F(Lock)|F(Special)              , EF(WWWWWW__), 0 , 0 , kFamilyNone, 0  , 371 , 48 ),
  INST(Cmpxchg16b      , X86M_Only          , O(000F00,C7,1,_,1,_,_,_  ), 0                         , F(RW)|F(Lock)|F(Special)              , EF(__W_____), 0 , 0 , kFamilyNone, 0  , 379 , 49 ),
  INST(Cmpxchg8b       , X86M_Only          , O(000F00,C7,1,_,_,_,_,_  ), 0                         , F(RW)|F(Lock)|F(Special)              , EF(__W_____), 0 , 0 , kFamilyNone, 0  , 390 , 50 ),
  INST(Comisd          , ExtRm              , O(660F00,2F,_,_,_,_,_,_  ), 0                         , F(RO)                                 , EF(WWWWWW__), 0 , 0 , kFamilySse , 13 , 9070, 51 ),
  INST(Comiss          , ExtRm              , O(000F00,2F,_,_,_,_,_,_  ), 0                         , F(RO)                                 , EF(WWWWWW__), 0 , 0 , kFamilySse , 14 , 9079, 52 ),
  INST(Cpuid           , X86Op              , O(000F00,A2,_,_,_,_,_,_  ), 0                         , F(RW)|F(Volatile)|F(Special)          , EF(________), 0 , 0 , kFamilyNone, 0  , 400 , 53 ),
  INST(Cqo             , X86Op_xDX_xAX      , O(000000,99,_,_,1,_,_,_  ), 0                         , F(RW)|F(Special)                      , EF(________), 0 , 0 , kFamilyNone, 0  , 406 , 54 ),
  INST(Crc32           , X86Crc             , O(F20F38,F0,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 410 , 55 ),
  INST(Cvtdq2pd        , ExtRm              , O(F30F00,E6,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 15 , 3091, 56 ),
  INST(Cvtdq2ps        , ExtRm              , O(000F00,5B,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 15 , 3101, 57 ),
  INST(Cvtpd2dq        , ExtRm              , O(F20F00,E6,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 15 , 3111, 57 ),
  INST(Cvtpd2pi        , ExtRm              , O(660F00,2D,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 16 , 416 , 58 ),
  INST(Cvtpd2ps        , ExtRm              , O(660F00,5A,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 17 , 3121, 57 ),
  INST(Cvtpi2pd        , ExtRm              , O(660F00,2A,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 16 , 425 , 59 ),
  INST(Cvtpi2ps        , ExtRm              , O(000F00,2A,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 18 , 434 , 60 ),
  INST(Cvtps2dq        , ExtRm              , O(660F00,5B,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 13 , 3173, 57 ),
  INST(Cvtps2pd        , ExtRm              , O(000F00,5A,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 13 , 3183, 56 ),
  INST(Cvtps2pi        , ExtRm              , O(000F00,2D,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 18 , 443 , 61 ),
  INST(Cvtsd2si        , ExtRm_Wx           , O(F20F00,2D,_,_,x,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 19 , 3255, 62 ),
  INST(Cvtsd2ss        , ExtRm              , O(F20F00,5A,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 4 , kFamilySse , 20 , 3265, 63 ),
  INST(Cvtsi2sd        , ExtRm_Wx           , O(F20F00,2A,_,_,x,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 21 , 3286, 64 ),
  INST(Cvtsi2ss        , ExtRm_Wx           , O(F30F00,2A,_,_,x,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 4 , kFamilySse , 22 , 3296, 65 ),
  INST(Cvtss2sd        , ExtRm              , O(F30F00,5A,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 21 , 3306, 66 ),
  INST(Cvtss2si        , ExtRm_Wx           , O(F30F00,2D,_,_,x,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 23 , 3316, 67 ),
  INST(Cvttpd2dq       , ExtRm              , O(660F00,E6,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 24 , 3337, 57 ),
  INST(Cvttpd2pi       , ExtRm              , O(660F00,2C,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 16 , 452 , 58 ),
  INST(Cvttps2dq       , ExtRm              , O(F30F00,5B,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 25 , 3383, 57 ),
  INST(Cvttps2pi       , ExtRm              , O(000F00,2C,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 18 , 462 , 61 ),
  INST(Cvttsd2si       , ExtRm_Wx           , O(F20F00,2C,_,_,x,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 26 , 3429, 62 ),
  INST(Cvttss2si       , ExtRm_Wx           , O(F30F00,2C,_,_,x,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 27 , 3452, 67 ),
  INST(Cwd             , X86Op_xDX_xAX      , O(660000,99,_,_,_,_,_,_  ), 0                         , F(RW)|F(Special)                      , EF(________), 0 , 0 , kFamilyNone, 0  , 472 , 68 ),
  INST(Cwde            , X86Op_xAX          , O(000000,98,_,_,_,_,_,_  ), 0                         , F(RW)|F(Special)                      , EF(________), 0 , 0 , kFamilyNone, 0  , 476 , 69 ),
  INST(Daa             , X86Op              , O(000000,27,_,_,_,_,_,_  ), 0                         , F(RW)|F(Special)                      , EF(UWWXWX__), 0 , 0 , kFamilyNone, 0  , 481 , 70 ),
  INST(Das             , X86Op              , O(000000,2F,_,_,_,_,_,_  ), 0                         , F(RW)|F(Special)                      , EF(UWWXWX__), 0 , 0 , kFamilyNone, 0  , 485 , 70 ),
  INST(Dec             , X86IncDec          , O(000000,FE,1,_,x,_,_,_  ), O(000000,48,_,_,x,_,_,_  ), F(RW)|F(Lock)                         , EF(WWWWW___), 0 , 0 , kFamilyNone, 0  , 2605, 71 ),
  INST(Div             , X86M_GPB_MulDiv    , O(000000,F6,6,_,x,_,_,_  ), 0                         , F(RW)|F(Special)                      , EF(UUUUUU__), 0 , 0 , kFamilyNone, 0  , 697 , 72 ),
  INST(Divpd           , ExtRm              , O(660F00,5E,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 28 , 3551, 6  ),
  INST(Divps           , ExtRm              , O(000F00,5E,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 29 , 3558, 6  ),
  INST(Divsd           , ExtRm              , O(F20F00,5E,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 28 , 3565, 7  ),
  INST(Divss           , ExtRm              , O(F30F00,5E,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 29 , 3572, 8  ),
  INST(Dppd            , ExtRmi             , O(660F3A,41,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 30 , 3579, 17 ),
  INST(Dpps            , ExtRmi             , O(660F3A,40,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 30 , 3585, 17 ),
  INST(Emms            , X86Op              , O(000F00,77,_,_,_,_,_,_  ), 0                         , F(Volatile)                           , EF(________), 0 , 0 , kFamilyNone, 0  , 665 , 73 ),
  INST(Enter           , X86Enter           , O(000000,C8,_,_,_,_,_,_  ), 0                         , F(Volatile)|F(Special)                , EF(________), 0 , 0 , kFamilyNone, 0  , 489 , 74 ),
  INST(Extractps       , ExtExtract         , O(660F3A,17,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 31 , 3765, 75 ),
  INST(Extrq           , ExtExtrq           , O(660F00,79,_,_,_,_,_,_  ), O(660F00,78,0,_,_,_,_,_  ), F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 32 , 6581, 76 ),
  INST(F2xm1           , FpuOp              , O_FPU(00,D9F0,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 495 , 77 ),
  INST(Fabs            , FpuOp              , O_FPU(00,D9E1,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 501 , 77 ),
  INST(Fadd            , FpuArith           , O_FPU(00,C0C0,0)          , 0                         , F(Fp)|F(FPU_M4)|F(FPU_M8)             , EF(________), 0 , 0 , kFamilyNone, 0  , 1813, 78 ),
  INST(Faddp           , FpuRDef            , O_FPU(00,DEC0,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 506 , 79 ),
  INST(Fbld            , X86M_Only          , O_FPU(00,00DF,4)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 512 , 80 ),
  INST(Fbstp           , X86M_Only          , O_FPU(00,00DF,6)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 517 , 80 ),
  INST(Fchs            , FpuOp              , O_FPU(00,D9E0,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 523 , 77 ),
  INST(Fclex           , FpuOp              , O_FPU(9B,DBE2,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 528 , 77 ),
  INST(Fcmovb          , FpuR               , O_FPU(00,DAC0,_)          , 0                         , F(Fp)                                 , EF(_____R__), 0 , 0 , kFamilyNone, 0  , 534 , 81 ),
  INST(Fcmovbe         , FpuR               , O_FPU(00,DAD0,_)          , 0                         , F(Fp)                                 , EF(__R__R__), 0 , 0 , kFamilyNone, 0  , 541 , 82 ),
  INST(Fcmove          , FpuR               , O_FPU(00,DAC8,_)          , 0                         , F(Fp)                                 , EF(__R_____), 0 , 0 , kFamilyNone, 0  , 549 , 83 ),
  INST(Fcmovnb         , FpuR               , O_FPU(00,DBC0,_)          , 0                         , F(Fp)                                 , EF(_____R__), 0 , 0 , kFamilyNone, 0  , 556 , 81 ),
  INST(Fcmovnbe        , FpuR               , O_FPU(00,DBD0,_)          , 0                         , F(Fp)                                 , EF(__R__R__), 0 , 0 , kFamilyNone, 0  , 564 , 82 ),
  INST(Fcmovne         , FpuR               , O_FPU(00,DBC8,_)          , 0                         , F(Fp)                                 , EF(__R_____), 0 , 0 , kFamilyNone, 0  , 573 , 83 ),
  INST(Fcmovnu         , FpuR               , O_FPU(00,DBD8,_)          , 0                         , F(Fp)                                 , EF(____R___), 0 , 0 , kFamilyNone, 0  , 581 , 84 ),
  INST(Fcmovu          , FpuR               , O_FPU(00,DAD8,_)          , 0                         , F(Fp)                                 , EF(____R___), 0 , 0 , kFamilyNone, 0  , 589 , 84 ),
  INST(Fcom            , FpuCom             , O_FPU(00,D0D0,2)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 596 , 85 ),
  INST(Fcomi           , FpuR               , O_FPU(00,DBF0,_)          , 0                         , F(Fp)                                 , EF(WWWWWW__), 0 , 0 , kFamilyNone, 0  , 601 , 86 ),
  INST(Fcomip          , FpuR               , O_FPU(00,DFF0,_)          , 0                         , F(Fp)                                 , EF(WWWWWW__), 0 , 0 , kFamilyNone, 0  , 607 , 86 ),
  INST(Fcomp           , FpuCom             , O_FPU(00,D8D8,3)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 614 , 85 ),
  INST(Fcompp          , FpuOp              , O_FPU(00,DED9,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 620 , 77 ),
  INST(Fcos            , FpuOp              , O_FPU(00,D9FF,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 627 , 77 ),
  INST(Fdecstp         , FpuOp              , O_FPU(00,D9F6,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 632 , 77 ),
  INST(Fdiv            , FpuArith           , O_FPU(00,F0F8,6)          , 0                         , F(Fp)|F(FPU_M4)|F(FPU_M8)             , EF(________), 0 , 0 , kFamilyNone, 0  , 640 , 78 ),
  INST(Fdivp           , FpuRDef            , O_FPU(00,DEF8,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 645 , 79 ),
  INST(Fdivr           , FpuArith           , O_FPU(00,F8F0,7)          , 0                         , F(Fp)|F(FPU_M4)|F(FPU_M8)             , EF(________), 0 , 0 , kFamilyNone, 0  , 651 , 78 ),
  INST(Fdivrp          , FpuRDef            , O_FPU(00,DEF0,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 657 , 79 ),
  INST(Femms           , X86Op              , O(000F00,0E,_,_,_,_,_,_  ), 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 664 , 77 ),
  INST(Ffree           , FpuR               , O_FPU(00,DDC0,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 670 , 87 ),
  INST(Fiadd           , FpuM               , O_FPU(00,00DA,0)          , 0                         , F(Fp)|F(FPU_M2)|F(FPU_M4)             , EF(________), 0 , 0 , kFamilyNone, 0  , 676 , 88 ),
  INST(Ficom           , FpuM               , O_FPU(00,00DA,2)          , 0                         , F(Fp)|F(FPU_M2)|F(FPU_M4)             , EF(________), 0 , 0 , kFamilyNone, 0  , 682 , 88 ),
  INST(Ficomp          , FpuM               , O_FPU(00,00DA,3)          , 0                         , F(Fp)|F(FPU_M2)|F(FPU_M4)             , EF(________), 0 , 0 , kFamilyNone, 0  , 688 , 88 ),
  INST(Fidiv           , FpuM               , O_FPU(00,00DA,6)          , 0                         , F(Fp)|F(FPU_M2)|F(FPU_M4)             , EF(________), 0 , 0 , kFamilyNone, 0  , 695 , 88 ),
  INST(Fidivr          , FpuM               , O_FPU(00,00DA,7)          , 0                         , F(Fp)|F(FPU_M2)|F(FPU_M4)             , EF(________), 0 , 0 , kFamilyNone, 0  , 701 , 88 ),
  INST(Fild            , FpuM               , O_FPU(00,00DB,0)          , O_FPU(00,00DF,5)          , F(Fp)|F(FPU_M2)|F(FPU_M4)|F(FPU_M8)   , EF(________), 0 , 0 , kFamilyNone, 0  , 708 , 89 ),
  INST(Fimul           , FpuM               , O_FPU(00,00DA,1)          , 0                         , F(Fp)|F(FPU_M2)|F(FPU_M4)             , EF(________), 0 , 0 , kFamilyNone, 0  , 713 , 88 ),
  INST(Fincstp         , FpuOp              , O_FPU(00,D9F7,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 719 , 77 ),
  INST(Finit           , FpuOp              , O_FPU(9B,DBE3,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 727 , 77 ),
  INST(Fist            , FpuM               , O_FPU(00,00DB,2)          , 0                         , F(Fp)|F(FPU_M2)|F(FPU_M4)             , EF(________), 0 , 0 , kFamilyNone, 0  , 733 , 88 ),
  INST(Fistp           , FpuM               , O_FPU(00,00DB,3)          , O_FPU(00,00DF,7)          , F(Fp)|F(FPU_M2)|F(FPU_M4)|F(FPU_M8)   , EF(________), 0 , 0 , kFamilyNone, 0  , 738 , 90 ),
  INST(Fisttp          , FpuM               , O_FPU(00,00DB,1)          , O_FPU(00,00DD,1)          , F(Fp)|F(FPU_M2)|F(FPU_M4)|F(FPU_M8)   , EF(________), 0 , 0 , kFamilyNone, 0  , 744 , 91 ),
  INST(Fisub           , FpuM               , O_FPU(00,00DA,4)          , 0                         , F(Fp)|F(FPU_M2)|F(FPU_M4)             , EF(________), 0 , 0 , kFamilyNone, 0  , 751 , 88 ),
  INST(Fisubr          , FpuM               , O_FPU(00,00DA,5)          , 0                         , F(Fp)|F(FPU_M2)|F(FPU_M4)             , EF(________), 0 , 0 , kFamilyNone, 0  , 757 , 88 ),
  INST(Fld             , FpuFldFst          , O_FPU(00,00D9,0)          , O_FPU(00,00DB,5)          , F(Fp)|F(FPU_M2)|F(FPU_M4)|F(FPU_M8)   , EF(________), 0 , 0 , kFamilyNone, 0  , 764 , 92 ),
  INST(Fld1            , FpuOp              , O_FPU(00,D9E8,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 768 , 77 ),
  INST(Fldcw           , X86M_Only          , O_FPU(00,00D9,5)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 773 , 93 ),
  INST(Fldenv          , X86M_Only          , O_FPU(00,00D9,4)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 779 , 94 ),
  INST(Fldl2e          , FpuOp              , O_FPU(00,D9EA,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 786 , 77 ),
  INST(Fldl2t          , FpuOp              , O_FPU(00,D9E9,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 793 , 77 ),
  INST(Fldlg2          , FpuOp              , O_FPU(00,D9EC,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 800 , 77 ),
  INST(Fldln2          , FpuOp              , O_FPU(00,D9ED,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 807 , 77 ),
  INST(Fldpi           , FpuOp              , O_FPU(00,D9EB,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 814 , 77 ),
  INST(Fldz            , FpuOp              , O_FPU(00,D9EE,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 820 , 77 ),
  INST(Fmul            , FpuArith           , O_FPU(00,C8C8,1)          , 0                         , F(Fp)|F(FPU_M4)|F(FPU_M8)             , EF(________), 0 , 0 , kFamilyNone, 0  , 1855, 78 ),
  INST(Fmulp           , FpuRDef            , O_FPU(00,DEC8,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 825 , 79 ),
  INST(Fnclex          , FpuOp              , O_FPU(00,DBE2,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 831 , 77 ),
  INST(Fninit          , FpuOp              , O_FPU(00,DBE3,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 838 , 77 ),
  INST(Fnop            , FpuOp              , O_FPU(00,D9D0,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 845 , 77 ),
  INST(Fnsave          , X86M_Only          , O_FPU(00,00DD,6)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 850 , 94 ),
  INST(Fnstcw          , X86M_Only          , O_FPU(00,00D9,7)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 857 , 93 ),
  INST(Fnstenv         , X86M_Only          , O_FPU(00,00D9,6)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 864 , 94 ),
  INST(Fnstsw          , FpuStsw            , O_FPU(00,00DD,7)          , O_FPU(00,DFE0,_)          , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 872 , 95 ),
  INST(Fpatan          , FpuOp              , O_FPU(00,D9F3,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 879 , 77 ),
  INST(Fprem           , FpuOp              , O_FPU(00,D9F8,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 886 , 77 ),
  INST(Fprem1          , FpuOp              , O_FPU(00,D9F5,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 892 , 77 ),
  INST(Fptan           , FpuOp              , O_FPU(00,D9F2,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 899 , 77 ),
  INST(Frndint         , FpuOp              , O_FPU(00,D9FC,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 905 , 77 ),
  INST(Frstor          , X86M_Only          , O_FPU(00,00DD,4)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 913 , 94 ),
  INST(Fsave           , X86M_Only          , O_FPU(9B,00DD,6)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 920 , 94 ),
  INST(Fscale          , FpuOp              , O_FPU(00,D9FD,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 926 , 77 ),
  INST(Fsin            , FpuOp              , O_FPU(00,D9FE,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 933 , 77 ),
  INST(Fsincos         , FpuOp              , O_FPU(00,D9FB,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 938 , 77 ),
  INST(Fsqrt           , FpuOp              , O_FPU(00,D9FA,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 946 , 77 ),
  INST(Fst             , FpuFldFst          , O_FPU(00,00D9,2)          , 0                         , F(Fp)|F(FPU_M4)|F(FPU_M8)             , EF(________), 0 , 0 , kFamilyNone, 0  , 952 , 96 ),
  INST(Fstcw           , X86M_Only          , O_FPU(9B,00D9,7)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 956 , 93 ),
  INST(Fstenv          , X86M_Only          , O_FPU(9B,00D9,6)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 962 , 94 ),
  INST(Fstp            , FpuFldFst          , O_FPU(00,00D9,3)          , O(000000,DB,7,_,_,_,_,_  ), F(Fp)|F(FPU_M4)|F(FPU_M8)|F(FPU_M10)  , EF(________), 0 , 0 , kFamilyNone, 0  , 969 , 97 ),
  INST(Fstsw           , FpuStsw            , O_FPU(9B,00DD,7)          , O_FPU(9B,DFE0,_)          , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 974 , 98 ),
  INST(Fsub            , FpuArith           , O_FPU(00,E0E8,4)          , 0                         , F(Fp)|F(FPU_M4)|F(FPU_M8)             , EF(________), 0 , 0 , kFamilyNone, 0  , 1933, 78 ),
  INST(Fsubp           , FpuRDef            , O_FPU(00,DEE8,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 980 , 79 ),
  INST(Fsubr           , FpuArith           , O_FPU(00,E8E0,5)          , 0                         , F(Fp)|F(FPU_M4)|F(FPU_M8)             , EF(________), 0 , 0 , kFamilyNone, 0  , 1939, 78 ),
  INST(Fsubrp          , FpuRDef            , O_FPU(00,DEE0,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 986 , 79 ),
  INST(Ftst            , FpuOp              , O_FPU(00,D9E4,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 993 , 77 ),
  INST(Fucom           , FpuRDef            , O_FPU(00,DDE0,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 998 , 79 ),
  INST(Fucomi          , FpuR               , O_FPU(00,DBE8,_)          , 0                         , F(Fp)                                 , EF(WWWWWW__), 0 , 0 , kFamilyNone, 0  , 1004, 86 ),
  INST(Fucomip         , FpuR               , O_FPU(00,DFE8,_)          , 0                         , F(Fp)                                 , EF(WWWWWW__), 0 , 0 , kFamilyNone, 0  , 1011, 86 ),
  INST(Fucomp          , FpuRDef            , O_FPU(00,DDE8,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 1019, 79 ),
  INST(Fucompp         , FpuOp              , O_FPU(00,DAE9,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 1026, 77 ),
  INST(Fwait           , X86Op              , O_FPU(00,00DB,_)          , 0                         , F(Fp)|F(Volatile)                     , EF(________), 0 , 0 , kFamilyNone, 0  , 1034, 99 ),
  INST(Fxam            , FpuOp              , O_FPU(00,D9E5,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 1040, 77 ),
  INST(Fxch            , FpuR               , O_FPU(00,D9C8,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 1045, 79 ),
  INST(Fxrstor         , X86M_Only          , O(000F00,AE,1,_,_,_,_,_  ), 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 1050, 94 ),
  INST(Fxrstor64       , X86M_Only          , O(000F00,AE,1,_,1,_,_,_  ), 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 1058, 100),
  INST(Fxsave          , X86M_Only          , O(000F00,AE,0,_,_,_,_,_  ), 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 1068, 94 ),
  INST(Fxsave64        , X86M_Only          , O(000F00,AE,0,_,1,_,_,_  ), 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 1075, 100),
  INST(Fxtract         , FpuOp              , O_FPU(00,D9F4,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 1084, 77 ),
  INST(Fyl2x           , FpuOp              , O_FPU(00,D9F1,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 1092, 77 ),
  INST(Fyl2xp1         , FpuOp              , O_FPU(00,D9F9,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 1098, 77 ),
  INST(Haddpd          , ExtRm              , O(660F00,7C,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 33 , 5120, 6  ),
  INST(Haddps          , ExtRm              , O(F20F00,7C,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 33 , 5128, 6  ),
  INST(Hsubpd          , ExtRm              , O(660F00,7D,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 33 , 5136, 6  ),
  INST(Hsubps          , ExtRm              , O(F20F00,7D,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 33 , 5144, 6  ),
  INST(Idiv            , X86M_GPB_MulDiv    , O(000000,F6,7,_,x,_,_,_  ), 0                         , F(RW)|F(Special)                      , EF(UUUUUU__), 0 , 0 , kFamilyNone, 0  , 696 , 101),
  INST(Imul            , X86Imul            , O(000000,F6,5,_,x,_,_,_  ), 0                         , F(RW)|F(Special)                      , EF(WUUUUW__), 0 , 0 , kFamilyNone, 0  , 714 , 102),
  INST(In              , X86In              , O(000000,EC,_,_,_,_,_,_  ), O(000000,E4,_,_,_,_,_,_  ), F(WO)|F(Volatile)|F(Special)          , EF(________), 0 , 0 , kFamilyNone, 0  , 1851, 103),
  INST(Inc             , X86IncDec          , O(000000,FE,0,_,x,_,_,_  ), O(000000,40,_,_,x,_,_,_  ), F(RW)|F(Lock)                         , EF(WWWWW___), 0 , 0 , kFamilyNone, 0  , 1106, 104),
  INST(Ins             , X86Ins             , O(000000,6C,_,_,_,_,_,_  ), 0                         , F(WO)|F(Volatile)|F(Special)|F(Rep)   , EF(________), 0 , 0 , kFamilyNone, 0  , 1110, 105),
  INST(Insertps        , ExtRmi             , O(660F3A,21,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 34 , 5280, 47 ),
  INST(Insertq         , ExtInsertq         , O(F20F00,79,_,_,_,_,_,_  ), O(F20F00,78,_,_,_,_,_,_  ), F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 32 , 1114, 106),
  INST(Int             , X86Int             , O(000000,CD,_,_,_,_,_,_  ), 0                         , F(Volatile)                           , EF(___W___W), 0 , 0 , kFamilyNone, 0  , 909 , 107),
  INST(Int3            , X86Op              , O(000000,CC,_,_,_,_,_,_  ), 0                         , F(Volatile)                           , EF(___W___W), 0 , 0 , kFamilyNone, 0  , 1122, 108),
  INST(Into            , X86Op              , O(000000,CE,_,_,_,_,_,_  ), 0                         , F(Volatile)                           , EF(___W___W), 0 , 0 , kFamilyNone, 0  , 1127, 108),
  INST(Ja              , X86Jcc             , O(000F00,87,_,_,_,_,_,_  ), O(000000,77,_,_,_,_,_,_  ), F(Volatile)                           , EF(__R__R__), 0 , 0 , kFamilyNone, 0  , 1132, 109),
  INST(Jae             , X86Jcc             , O(000F00,83,_,_,_,_,_,_  ), O(000000,73,_,_,_,_,_,_  ), F(Volatile)                           , EF(_____R__), 0 , 0 , kFamilyNone, 0  , 1135, 110),
  INST(Jb              , X86Jcc             , O(000F00,82,_,_,_,_,_,_  ), O(000000,72,_,_,_,_,_,_  ), F(Volatile)                           , EF(_____R__), 0 , 0 , kFamilyNone, 0  , 1139, 111),
  INST(Jbe             , X86Jcc             , O(000F00,86,_,_,_,_,_,_  ), O(000000,76,_,_,_,_,_,_  ), F(Volatile)                           , EF(__R__R__), 0 , 0 , kFamilyNone, 0  , 1142, 112),
  INST(Jc              , X86Jcc             , O(000F00,82,_,_,_,_,_,_  ), O(000000,72,_,_,_,_,_,_  ), F(Volatile)                           , EF(_____R__), 0 , 0 , kFamilyNone, 0  , 1146, 113),
  INST(Je              , X86Jcc             , O(000F00,84,_,_,_,_,_,_  ), O(000000,74,_,_,_,_,_,_  ), F(Volatile)                           , EF(__R_____), 0 , 0 , kFamilyNone, 0  , 1149, 114),
  INST(Jecxz           , X86JecxzLoop       , 0                         , O(000000,E3,_,_,_,_,_,_  ), F(Volatile)|F(Special)                , EF(________), 0 , 0 , kFamilyNone, 0  , 1152, 115),
  INST(Jg              , X86Jcc             , O(000F00,8F,_,_,_,_,_,_  ), O(000000,7F,_,_,_,_,_,_  ), F(Volatile)                           , EF(RRR_____), 0 , 0 , kFamilyNone, 0  , 1158, 116),
  INST(Jge             , X86Jcc             , O(000F00,8D,_,_,_,_,_,_  ), O(000000,7D,_,_,_,_,_,_  ), F(Volatile)                           , EF(RR______), 0 , 0 , kFamilyNone, 0  , 1161, 117),
  INST(Jl              , X86Jcc             , O(000F00,8C,_,_,_,_,_,_  ), O(000000,7C,_,_,_,_,_,_  ), F(Volatile)                           , EF(RR______), 0 , 0 , kFamilyNone, 0  , 1165, 118),
  INST(Jle             , X86Jcc             , O(000F00,8E,_,_,_,_,_,_  ), O(000000,7E,_,_,_,_,_,_  ), F(Volatile)                           , EF(RRR_____), 0 , 0 , kFamilyNone, 0  , 1168, 119),
  INST(Jmp             , X86Jmp             , O(000000,FF,4,_,_,_,_,_  ), O(000000,EB,_,_,_,_,_,_  ), F(Volatile)                           , EF(________), 0 , 0 , kFamilyNone, 0  , 1172, 120),
  INST(Jna             , X86Jcc             , O(000F00,86,_,_,_,_,_,_  ), O(000000,76,_,_,_,_,_,_  ), F(Volatile)                           , EF(__R__R__), 0 , 0 , kFamilyNone, 0  , 1176, 112),
  INST(Jnae            , X86Jcc             , O(000F00,82,_,_,_,_,_,_  ), O(000000,72,_,_,_,_,_,_  ), F(Volatile)                           , EF(_____R__), 0 , 0 , kFamilyNone, 0  , 1180, 111),
  INST(Jnb             , X86Jcc             , O(000F00,83,_,_,_,_,_,_  ), O(000000,73,_,_,_,_,_,_  ), F(Volatile)                           , EF(_____R__), 0 , 0 , kFamilyNone, 0  , 1185, 110),
  INST(Jnbe            , X86Jcc             , O(000F00,87,_,_,_,_,_,_  ), O(000000,77,_,_,_,_,_,_  ), F(Volatile)                           , EF(__R__R__), 0 , 0 , kFamilyNone, 0  , 1189, 109),
  INST(Jnc             , X86Jcc             , O(000F00,83,_,_,_,_,_,_  ), O(000000,73,_,_,_,_,_,_  ), F(Volatile)                           , EF(_____R__), 0 , 0 , kFamilyNone, 0  , 1194, 121),
  INST(Jne             , X86Jcc             , O(000F00,85,_,_,_,_,_,_  ), O(000000,75,_,_,_,_,_,_  ), F(Volatile)                           , EF(__R_____), 0 , 0 , kFamilyNone, 0  , 1198, 122),
  INST(Jng             , X86Jcc             , O(000F00,8E,_,_,_,_,_,_  ), O(000000,7E,_,_,_,_,_,_  ), F(Volatile)                           , EF(RRR_____), 0 , 0 , kFamilyNone, 0  , 1202, 119),
  INST(Jnge            , X86Jcc             , O(000F00,8C,_,_,_,_,_,_  ), O(000000,7C,_,_,_,_,_,_  ), F(Volatile)                           , EF(RR______), 0 , 0 , kFamilyNone, 0  , 1206, 118),
  INST(Jnl             , X86Jcc             , O(000F00,8D,_,_,_,_,_,_  ), O(000000,7D,_,_,_,_,_,_  ), F(Volatile)                           , EF(RR______), 0 , 0 , kFamilyNone, 0  , 1211, 117),
  INST(Jnle            , X86Jcc             , O(000F00,8F,_,_,_,_,_,_  ), O(000000,7F,_,_,_,_,_,_  ), F(Volatile)                           , EF(RRR_____), 0 , 0 , kFamilyNone, 0  , 1215, 116),
  INST(Jno             , X86Jcc             , O(000F00,81,_,_,_,_,_,_  ), O(000000,71,_,_,_,_,_,_  ), F(Volatile)                           , EF(R_______), 0 , 0 , kFamilyNone, 0  , 1220, 123),
  INST(Jnp             , X86Jcc             , O(000F00,8B,_,_,_,_,_,_  ), O(000000,7B,_,_,_,_,_,_  ), F(Volatile)                           , EF(____R___), 0 , 0 , kFamilyNone, 0  , 1224, 124),
  INST(Jns             , X86Jcc             , O(000F00,89,_,_,_,_,_,_  ), O(000000,79,_,_,_,_,_,_  ), F(Volatile)                           , EF(_R______), 0 , 0 , kFamilyNone, 0  , 1228, 125),
  INST(Jnz             , X86Jcc             , O(000F00,85,_,_,_,_,_,_  ), O(000000,75,_,_,_,_,_,_  ), F(Volatile)                           , EF(__R_____), 0 , 0 , kFamilyNone, 0  , 1232, 122),
  INST(Jo              , X86Jcc             , O(000F00,80,_,_,_,_,_,_  ), O(000000,70,_,_,_,_,_,_  ), F(Volatile)                           , EF(R_______), 0 , 0 , kFamilyNone, 0  , 1236, 126),
  INST(Jp              , X86Jcc             , O(000F00,8A,_,_,_,_,_,_  ), O(000000,7A,_,_,_,_,_,_  ), F(Volatile)                           , EF(____R___), 0 , 0 , kFamilyNone, 0  , 1239, 127),
  INST(Jpe             , X86Jcc             , O(000F00,8A,_,_,_,_,_,_  ), O(000000,7A,_,_,_,_,_,_  ), F(Volatile)                           , EF(____R___), 0 , 0 , kFamilyNone, 0  , 1242, 127),
  INST(Jpo             , X86Jcc             , O(000F00,8B,_,_,_,_,_,_  ), O(000000,7B,_,_,_,_,_,_  ), F(Volatile)                           , EF(____R___), 0 , 0 , kFamilyNone, 0  , 1246, 124),
  INST(Js              , X86Jcc             , O(000F00,88,_,_,_,_,_,_  ), O(000000,78,_,_,_,_,_,_  ), F(Volatile)                           , EF(_R______), 0 , 0 , kFamilyNone, 0  , 1250, 128),
  INST(Jz              , X86Jcc             , O(000F00,84,_,_,_,_,_,_  ), O(000000,74,_,_,_,_,_,_  ), F(Volatile)                           , EF(__R_____), 0 , 0 , kFamilyNone, 0  , 1253, 114),
  INST(Kaddb           , VexRvm             , V(660F00,4A,_,1,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1256, 129),
  INST(Kaddd           , VexRvm             , V(660F00,4A,_,1,1,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1262, 129),
  INST(Kaddq           , VexRvm             , V(000F00,4A,_,1,1,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1268, 129),
  INST(Kaddw           , VexRvm             , V(000F00,4A,_,1,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1274, 129),
  INST(Kandb           , VexRvm             , V(660F00,41,_,1,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1280, 129),
  INST(Kandd           , VexRvm             , V(660F00,41,_,1,1,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1286, 129),
  INST(Kandnb          , VexRvm             , V(660F00,42,_,1,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1292, 129),
  INST(Kandnd          , VexRvm             , V(660F00,42,_,1,1,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1299, 129),
  INST(Kandnq          , VexRvm             , V(000F00,42,_,1,1,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1306, 129),
  INST(Kandnw          , VexRvm             , V(000F00,42,_,1,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1313, 129),
  INST(Kandq           , VexRvm             , V(000F00,41,_,1,1,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1320, 129),
  INST(Kandw           , VexRvm             , V(000F00,41,_,1,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1326, 129),
  INST(Kmovb           , VexKmov            , V(660F00,90,_,0,0,_,_,_  ), V(660F00,92,_,0,0,_,_,_  ), F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1332, 130),
  INST(Kmovd           , VexKmov            , V(660F00,90,_,0,1,_,_,_  ), V(F20F00,92,_,0,0,_,_,_  ), F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 7061, 131),
  INST(Kmovq           , VexKmov            , V(000F00,90,_,0,1,_,_,_  ), V(F20F00,92,_,0,1,_,_,_  ), F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 7072, 132),
  INST(Kmovw           , VexKmov            , V(000F00,90,_,0,0,_,_,_  ), V(000F00,92,_,0,0,_,_,_  ), F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1338, 133),
  INST(Knotb           , VexRm              , V(660F00,44,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1344, 134),
  INST(Knotd           , VexRm              , V(660F00,44,_,0,1,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1350, 134),
  INST(Knotq           , VexRm              , V(000F00,44,_,0,1,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1356, 134),
  INST(Knotw           , VexRm              , V(000F00,44,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1362, 134),
  INST(Korb            , VexRvm             , V(660F00,45,_,1,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1368, 129),
  INST(Kord            , VexRvm             , V(660F00,45,_,1,1,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1373, 129),
  INST(Korq            , VexRvm             , V(000F00,45,_,1,1,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1378, 129),
  INST(Kortestb        , VexRm              , V(660F00,98,_,0,0,_,_,_  ), 0                         , F(RO)|F(Vex)                          , EF(WWWWWW__), 0 , 0 , kFamilyNone, 0  , 1383, 135),
  INST(Kortestd        , VexRm              , V(660F00,98,_,0,1,_,_,_  ), 0                         , F(RO)|F(Vex)                          , EF(WWWWWW__), 0 , 0 , kFamilyNone, 0  , 1392, 135),
  INST(Kortestq        , VexRm              , V(000F00,98,_,0,1,_,_,_  ), 0                         , F(RO)|F(Vex)                          , EF(WWWWWW__), 0 , 0 , kFamilyNone, 0  , 1401, 135),
  INST(Kortestw        , VexRm              , V(000F00,98,_,0,0,_,_,_  ), 0                         , F(RO)|F(Vex)                          , EF(WWWWWW__), 0 , 0 , kFamilyNone, 0  , 1410, 135),
  INST(Korw            , VexRvm             , V(000F00,45,_,1,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1419, 129),
  INST(Kshiftlb        , VexRmi             , V(660F3A,32,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1424, 136),
  INST(Kshiftld        , VexRmi             , V(660F3A,33,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1433, 136),
  INST(Kshiftlq        , VexRmi             , V(660F3A,33,_,0,1,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1442, 136),
  INST(Kshiftlw        , VexRmi             , V(660F3A,32,_,0,1,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1451, 136),
  INST(Kshiftrb        , VexRmi             , V(660F3A,30,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1460, 136),
  INST(Kshiftrd        , VexRmi             , V(660F3A,31,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1469, 136),
  INST(Kshiftrq        , VexRmi             , V(660F3A,31,_,0,1,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1478, 136),
  INST(Kshiftrw        , VexRmi             , V(660F3A,30,_,0,1,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1487, 136),
  INST(Ktestb          , VexRm              , V(660F00,99,_,0,0,_,_,_  ), 0                         , F(RO)|F(Vex)                          , EF(WWWWWW__), 0 , 0 , kFamilyNone, 0  , 1496, 135),
  INST(Ktestd          , VexRm              , V(660F00,99,_,0,1,_,_,_  ), 0                         , F(RO)|F(Vex)                          , EF(WWWWWW__), 0 , 0 , kFamilyNone, 0  , 1503, 135),
  INST(Ktestq          , VexRm              , V(000F00,99,_,0,1,_,_,_  ), 0                         , F(RO)|F(Vex)                          , EF(WWWWWW__), 0 , 0 , kFamilyNone, 0  , 1510, 135),
  INST(Ktestw          , VexRm              , V(000F00,99,_,0,0,_,_,_  ), 0                         , F(RO)|F(Vex)                          , EF(WWWWWW__), 0 , 0 , kFamilyNone, 0  , 1517, 135),
  INST(Kunpckbw        , VexRvm             , V(660F00,4B,_,1,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1524, 129),
  INST(Kunpckdq        , VexRvm             , V(000F00,4B,_,1,1,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1533, 129),
  INST(Kunpckwd        , VexRvm             , V(000F00,4B,_,1,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1542, 129),
  INST(Kxnorb          , VexRvm             , V(660F00,46,_,1,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1551, 129),
  INST(Kxnord          , VexRvm             , V(660F00,46,_,1,1,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1558, 129),
  INST(Kxnorq          , VexRvm             , V(000F00,46,_,1,1,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1565, 129),
  INST(Kxnorw          , VexRvm             , V(000F00,46,_,1,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1572, 129),
  INST(Kxorb           , VexRvm             , V(660F00,47,_,1,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1579, 129),
  INST(Kxord           , VexRvm             , V(660F00,47,_,1,1,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1585, 129),
  INST(Kxorq           , VexRvm             , V(000F00,47,_,1,1,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1591, 129),
  INST(Kxorw           , VexRvm             , V(000F00,47,_,1,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1597, 129),
  INST(Lahf            , X86Op              , O(000000,9F,_,_,_,_,_,_  ), 0                         , F(RW)|F(Volatile)|F(Special)          , EF(_RRRRR__), 0 , 0 , kFamilyNone, 0  , 1603, 137),
  INST(Lddqu           , ExtRm              , O(F20F00,F0,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 35 , 5290, 138),
  INST(Ldmxcsr         , X86M_Only          , O(000F00,AE,2,_,_,_,_,_  ), 0                         , F(RO)|F(Volatile)                     , EF(________), 0 , 0 , kFamilyNone, 0  , 5297, 139),
  INST(Lea             , X86Lea             , O(000000,8D,_,_,x,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 1608, 140),
  INST(Leave           , X86Op              , O(000000,C9,_,_,_,_,_,_  ), 0                         , F(Volatile)|F(Special)                , EF(________), 0 , 0 , kFamilyNone, 0  , 1612, 141),
  INST(Lfence          , X86Fence           , O(000F00,AE,5,_,_,_,_,_  ), 0                         , F(Volatile)                           , EF(________), 0 , 0 , kFamilyNone, 0  , 1618, 73 ),
  INST(Lods            , X86StrRm           , O(000000,AC,_,_,_,_,_,_  ), 0                         , F(WO)|F(Special)|F(Rep)               , EF(______R_), 0 , 1 , kFamilyNone, 0  , 1625, 142),
  INST(Loop            , X86JecxzLoop       , 0                         , O(000000,E2,_,_,_,_,_,_  ), F(RW)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 1630, 143),
  INST(Loope           , X86JecxzLoop       , 0                         , O(000000,E1,_,_,_,_,_,_  ), F(RW)                                 , EF(__R_____), 0 , 0 , kFamilyNone, 0  , 1635, 144),
  INST(Loopne          , X86JecxzLoop       , 0                         , O(000000,E0,_,_,_,_,_,_  ), F(RW)                                 , EF(__R_____), 0 , 0 , kFamilyNone, 0  , 1641, 145),
  INST(Lzcnt           , X86Rm              , O(F30F00,BD,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(UUWUUW__), 0 , 0 , kFamilyNone, 0  , 1648, 146),
  INST(Maskmovdqu      , ExtRm_ZDI          , O(660F00,57,_,_,_,_,_,_  ), 0                         , F(RO)|F(Special)                      , EF(________), 0 , 0 , kFamilySse , 36 , 5306, 147),
  INST(Maskmovq        , ExtRm_ZDI          , O(000F00,F7,_,_,_,_,_,_  ), 0                         , F(RO)|F(Special)                      , EF(________), 0 , 0 , kFamilySse , 37 , 7069, 148),
  INST(Maxpd           , ExtRm              , O(660F00,5F,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 38 , 5340, 6  ),
  INST(Maxps           , ExtRm              , O(000F00,5F,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 39 , 5347, 6  ),
  INST(Maxsd           , ExtRm              , O(F20F00,5F,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 38 , 7088, 7  ),
  INST(Maxss           , ExtRm              , O(F30F00,5F,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 39 , 5361, 8  ),
  INST(Mfence          , X86Fence           , O(000F00,AE,6,_,_,_,_,_  ), 0                         , F(RW)|F(Volatile)                     , EF(________), 0 , 0 , kFamilyNone, 0  , 1654, 149),
  INST(Minpd           , ExtRm              , O(660F00,5D,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 40 , 5368, 6  ),
  INST(Minps           , ExtRm              , O(000F00,5D,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 41 , 5375, 6  ),
  INST(Minsd           , ExtRm              , O(F20F00,5D,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 40 , 7152, 7  ),
  INST(Minss           , ExtRm              , O(F30F00,5D,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 41 , 5389, 8  ),
  INST(Monitor         , X86Op              , O(000F01,C8,_,_,_,_,_,_  ), 0                         , F(RO)|F(Volatile)|F(Special)          , EF(________), 0 , 0 , kFamilyNone, 0  , 1661, 150),
  INST(Mov             , X86Mov             , 0                         , 0                         , F(WO)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 6035, 151),
  INST(Movapd          , ExtMov             , O(660F00,28,_,_,_,_,_,_  ), O(660F00,29,_,_,_,_,_,_  ), F(WO)                                 , EF(________), 0 , 16, kFamilySse , 42 , 5396, 152),
  INST(Movaps          , ExtMov             , O(000F00,28,_,_,_,_,_,_  ), O(000F00,29,_,_,_,_,_,_  ), F(WO)                                 , EF(________), 0 , 16, kFamilySse , 43 , 5404, 153),
  INST(Movbe           , ExtMovbe           , O(000F38,F0,_,_,x,_,_,_  ), O(000F38,F1,_,_,x,_,_,_  ), F(WO)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 543 , 154),
  INST(Movd            , ExtMovd            , O(000F00,6E,_,_,_,_,_,_  ), O(000F00,7E,_,_,_,_,_,_  ), F(WO)                                 , EF(________), 0 , 16, kFamilySse , 44 , 7062, 155),
  INST(Movddup         , ExtMov             , O(F20F00,12,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 45 , 5418, 56 ),
  INST(Movdq2q         , ExtMov             , O(F20F00,D6,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 16 , 1669, 156),
  INST(Movdqa          , ExtMov             , O(660F00,6F,_,_,_,_,_,_  ), O(660F00,7F,_,_,_,_,_,_  ), F(WO)                                 , EF(________), 0 , 16, kFamilySse , 46 , 5427, 157),
  INST(Movdqu          , ExtMov             , O(F30F00,6F,_,_,_,_,_,_  ), O(F30F00,7F,_,_,_,_,_,_  ), F(WO)                                 , EF(________), 0 , 16, kFamilySse , 42 , 5310, 158),
  INST(Movhlps         , ExtMov             , O(000F00,12,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 47 , 5502, 159),
  INST(Movhpd          , ExtMov             , O(660F00,16,_,_,_,_,_,_  ), O(660F00,17,_,_,_,_,_,_  ), F(RW)                                 , EF(________), 8 , 8 , kFamilySse , 48 , 5511, 160),
  INST(Movhps          , ExtMov             , O(000F00,16,_,_,_,_,_,_  ), O(000F00,17,_,_,_,_,_,_  ), F(RW)                                 , EF(________), 8 , 8 , kFamilySse , 49 , 5519, 161),
  INST(Movlhps         , ExtMov             , O(000F00,16,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 8 , 8 , kFamilySse , 47 , 5527, 162),
  INST(Movlpd          , ExtMov             , O(660F00,12,_,_,_,_,_,_  ), O(660F00,13,_,_,_,_,_,_  ), F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 48 , 5536, 163),
  INST(Movlps          , ExtMov             , O(000F00,12,_,_,_,_,_,_  ), O(000F00,13,_,_,_,_,_,_  ), F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 49 , 5544, 164),
  INST(Movmskpd        , ExtMov             , O(660F00,50,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 50 , 5552, 165),
  INST(Movmskps        , ExtMov             , O(000F00,50,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 51 , 5562, 165),
  INST(Movntdq         , ExtMov             , 0                         , O(660F00,E7,_,_,_,_,_,_  ), F(WO)                                 , EF(________), 0 , 16, kFamilySse , 50 , 5572, 166),
  INST(Movntdqa        , ExtMov             , O(660F38,2A,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 52 , 5581, 138),
  INST(Movnti          , ExtMovnti          , O(000F00,C3,_,_,x,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilyNone, 0  , 1677, 167),
  INST(Movntpd         , ExtMov             , 0                         , O(660F00,2B,_,_,_,_,_,_  ), F(WO)                                 , EF(________), 0 , 16, kFamilySse , 53 , 5591, 168),
  INST(Movntps         , ExtMov             , 0                         , O(000F00,2B,_,_,_,_,_,_  ), F(WO)                                 , EF(________), 0 , 16, kFamilySse , 54 , 5600, 169),
  INST(Movntq          , ExtMov             , 0                         , O(000F00,E7,_,_,_,_,_,_  ), F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 37 , 1684, 170),
  INST(Movntsd         , ExtMov             , 0                         , O(F20F00,2B,_,_,_,_,_,_  ), F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 32 , 1691, 171),
  INST(Movntss         , ExtMov             , 0                         , O(F30F00,2B,_,_,_,_,_,_  ), F(WO)                                 , EF(________), 0 , 4 , kFamilySse , 32 , 1699, 172),
  INST(Movq            , ExtMovq            , O(000F00,6E,_,_,x,_,_,_  ), O(000F00,7E,_,_,x,_,_,_  ), F(WO)                                 , EF(________), 0 , 16, kFamilySse , 55 , 7073, 173),
  INST(Movq2dq         , ExtRm              , O(F30F00,D6,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 16 , 1707, 174),
  INST(Movs            , X86StrMm           , O(000000,A4,_,_,_,_,_,_  ), 0                         , F(WO)|F(Special)|F(Rep)               , EF(________), 0 , 0 , kFamilyNone, 0  , 351 , 175),
  INST(Movsd           , ExtMov             , O(F20F00,10,_,_,_,_,_,_  ), O(F20F00,11,_,_,_,_,_,_  ), F(WO)|F(ZeroIfMem)                    , EF(________), 0 , 8 , kFamilySse , 56 , 5615, 176),
  INST(Movshdup        , ExtRm              , O(F30F00,16,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 57 , 5622, 57 ),
  INST(Movsldup        , ExtRm              , O(F30F00,12,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 57 , 5632, 57 ),
  INST(Movss           , ExtMov             , O(F30F00,10,_,_,_,_,_,_  ), O(F30F00,11,_,_,_,_,_,_  ), F(WO)|F(ZeroIfMem)                    , EF(________), 0 , 4 , kFamilySse , 58 , 5642, 177),
  INST(Movsx           , X86MovsxMovzx      , O(000F00,BE,_,_,x,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 1715, 178),
  INST(Movsxd          , X86Rm              , O(000000,63,_,_,1,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 1721, 179),
  INST(Movupd          , ExtMov             , O(660F00,10,_,_,_,_,_,_  ), O(660F00,11,_,_,_,_,_,_  ), F(WO)                                 , EF(________), 0 , 16, kFamilySse , 59 , 5649, 180),
  INST(Movups          , ExtMov             , O(000F00,10,_,_,_,_,_,_  ), O(000F00,11,_,_,_,_,_,_  ), F(WO)                                 , EF(________), 0 , 16, kFamilySse , 60 , 5657, 181),
  INST(Movzx           , X86MovsxMovzx      , O(000F00,B6,_,_,x,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 1728, 178),
  INST(Mpsadbw         , ExtRmi             , O(660F3A,42,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 61 , 5665, 17 ),
  INST(Mul             , X86M_GPB_MulDiv    , O(000000,F6,4,_,x,_,_,_  ), 0                         , F(RW)|F(Special)                      , EF(WUUUUW__), 0 , 0 , kFamilyNone, 0  , 715 , 182),
  INST(Mulpd           , ExtRm              , O(660F00,59,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 62 , 5674, 6  ),
  INST(Mulps           , ExtRm              , O(000F00,59,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 63 , 5681, 6  ),
  INST(Mulsd           , ExtRm              , O(F20F00,59,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 62 , 5688, 7  ),
  INST(Mulss           , ExtRm              , O(F30F00,59,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 63 , 5695, 8  ),
  INST(Mulx            , VexRvm_ZDX_Wx      , V(F20F38,F6,_,0,x,_,_,_  ), 0                         , F(RW)|F(Special)                      , EF(________), 0 , 0 , kFamilyNone, 0  , 1734, 183),
  INST(Mwait           , X86Op              , O(000F01,C9,_,_,_,_,_,_  ), 0                         , F(RO)|F(Volatile)|F(Special)          , EF(________), 0 , 0 , kFamilyNone, 0  , 1739, 150),
  INST(Neg             , X86M_GPB           , O(000000,F6,3,_,x,_,_,_  ), 0                         , F(RW)|F(Lock)                         , EF(WWWWWW__), 0 , 0 , kFamilyNone, 0  , 1745, 184),
  INST(Nop             , X86Op              , O(000000,90,_,_,_,_,_,_  ), 0                         , 0                                     , EF(________), 0 , 0 , kFamilyNone, 0  , 846 , 185),
  INST(Not             , X86M_GPB           , O(000000,F6,2,_,x,_,_,_  ), 0                         , F(RW)|F(Lock)                         , EF(________), 0 , 0 , kFamilyNone, 0  , 1749, 186),
  INST(Or              , X86Arith           , O(000000,08,1,_,x,_,_,_  ), 0                         , F(RW)|F(Lock)                         , EF(WWWUWW__), 0 , 0 , kFamilyNone, 0  , 1055, 12 ),
  INST(Orpd            , ExtRm              , O(660F00,56,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 64 , 9128, 14 ),
  INST(Orps            , ExtRm              , O(000F00,56,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 65 , 9135, 14 ),
  INST(Out             , X86Out             , O(000000,EE,_,_,_,_,_,_  ), O(000000,E6,_,_,_,_,_,_  ), F(RO)|F(Volatile)|F(Special)          , EF(________), 0 , 0 , kFamilyNone, 0  , 1753, 187),
  INST(Outs            , X86Outs            , O(000000,6E,_,_,_,_,_,_  ), 0                         , F(RO)|F(Volatile)|F(Special)|F(Rep)   , EF(________), 0 , 0 , kFamilyNone, 0  , 1757, 188),
  INST(Pabsb           , ExtRm_P            , O(000F38,1C,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 66 , 5714, 189),
  INST(Pabsd           , ExtRm_P            , O(000F38,1E,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 66 , 5721, 189),
  INST(Pabsw           , ExtRm_P            , O(000F38,1D,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 67 , 5735, 189),
  INST(Packssdw        , ExtRm_P            , O(000F00,6B,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 68 , 5742, 189),
  INST(Packsswb        , ExtRm_P            , O(000F00,63,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 68 , 5752, 189),
  INST(Packusdw        , ExtRm              , O(660F38,2B,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 69 , 5762, 6  ),
  INST(Packuswb        , ExtRm_P            , O(000F00,67,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 68 , 5772, 189),
  INST(Paddb           , ExtRm_P            , O(000F00,FC,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 68 , 5782, 189),
  INST(Paddd           , ExtRm_P            , O(000F00,FE,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 68 , 5789, 189),
  INST(Paddq           , ExtRm_P            , O(000F00,D4,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 70 , 5796, 189),
  INST(Paddsb          , ExtRm_P            , O(000F00,EC,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 68 , 5803, 189),
  INST(Paddsw          , ExtRm_P            , O(000F00,ED,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 68 , 5811, 189),
  INST(Paddusb         , ExtRm_P            , O(000F00,DC,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 68 , 5819, 189),
  INST(Paddusw         , ExtRm_P            , O(000F00,DD,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 68 , 5828, 189),
  INST(Paddw           , ExtRm_P            , O(000F00,FD,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 68 , 5837, 189),
  INST(Palignr         , ExtRmi_P           , O(000F3A,0F,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 71 , 5844, 190),
  INST(Pand            , ExtRm_P            , O(000F00,DB,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 68 , 5853, 191),
  INST(Pandn           , ExtRm_P            , O(000F00,DF,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 72 , 5866, 192),
  INST(Pause           , X86Op              , O(F30000,90,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 1762, 193),
  INST(Pavgb           , ExtRm_P            , O(000F00,E0,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 73 , 5896, 189),
  INST(Pavgusb         , Ext3dNow           , O(000F0F,BF,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 74 , 1768, 194),
  INST(Pavgw           , ExtRm_P            , O(000F00,E3,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 75 , 5903, 189),
  INST(Pblendvb        , ExtRm_XMM0         , O(660F38,10,_,_,_,_,_,_  ), 0                         , F(RW)|F(Special)                      , EF(________), 0 , 0 , kFamilySse , 76 , 5919, 18 ),
  INST(Pblendw         , ExtRmi             , O(660F3A,0E,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 77 , 5929, 17 ),
  INST(Pclmulqdq       , ExtRmi             , O(660F3A,44,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 78 , 6022, 17 ),
  INST(Pcmpeqb         , ExtRm_P            , O(000F00,74,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 79 , 6054, 192),
  INST(Pcmpeqd         , ExtRm_P            , O(000F00,76,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 79 , 6063, 192),
  INST(Pcmpeqq         , ExtRm              , O(660F38,29,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 80 , 6072, 195),
  INST(Pcmpeqw         , ExtRm_P            , O(000F00,75,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 79 , 6081, 192),
  INST(Pcmpestri       , ExtRmi             , O(660F3A,61,_,_,_,_,_,_  ), 0                         , F(WO)|F(Special)                      , EF(________), 0 , 0 , kFamilySse , 81 , 6090, 196),
  INST(Pcmpestrm       , ExtRmi             , O(660F3A,60,_,_,_,_,_,_  ), 0                         , F(WO)|F(Special)                      , EF(________), 0 , 0 , kFamilySse , 81 , 6101, 197),
  INST(Pcmpgtb         , ExtRm_P            , O(000F00,64,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 79 , 6112, 192),
  INST(Pcmpgtd         , ExtRm_P            , O(000F00,66,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 79 , 6121, 192),
  INST(Pcmpgtq         , ExtRm              , O(660F38,37,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 80 , 6130, 195),
  INST(Pcmpgtw         , ExtRm_P            , O(000F00,65,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 79 , 6139, 192),
  INST(Pcmpistri       , ExtRmi             , O(660F3A,63,_,_,_,_,_,_  ), 0                         , F(WO)|F(Special)                      , EF(________), 0 , 0 , kFamilySse , 81 , 6148, 198),
  INST(Pcmpistrm       , ExtRmi             , O(660F3A,62,_,_,_,_,_,_  ), 0                         , F(WO)|F(Special)                      , EF(________), 0 , 0 , kFamilySse , 81 , 6159, 199),
  INST(Pcommit         , X86Op_O            , O(660F00,AE,7,_,_,_,_,_  ), 0                         , F(Volatile)                           , EF(________), 0 , 0 , kFamilyNone, 0  , 1776, 73 ),
  INST(Pdep            , VexRvm_Wx          , V(F20F38,F5,_,0,x,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 1784, 200),
  INST(Pext            , VexRvm_Wx          , V(F30F38,F5,_,0,x,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 1789, 200),
  INST(Pextrb          , ExtExtract         , O(000F3A,14,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 82 , 6564, 201),
  INST(Pextrd          , ExtExtract         , O(000F3A,16,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 82 , 6572, 75 ),
  INST(Pextrq          , ExtExtract         , O(000F3A,16,_,_,1,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 82 , 6580, 202),
  INST(Pextrw          , ExtPextrw          , O(000F00,C5,_,_,_,_,_,_  ), O(000F3A,15,_,_,_,_,_,_  ), F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 83 , 6588, 203),
  INST(Pf2id           , Ext3dNow           , O(000F0F,1D,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 74 , 1794, 204),
  INST(Pf2iw           , Ext3dNow           , O(000F0F,1C,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 84 , 1800, 204),
  INST(Pfacc           , Ext3dNow           , O(000F0F,AE,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 74 , 1806, 194),
  INST(Pfadd           , Ext3dNow           , O(000F0F,9E,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 74 , 1812, 194),
  INST(Pfcmpeq         , Ext3dNow           , O(000F0F,B0,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 74 , 1818, 194),
  INST(Pfcmpge         , Ext3dNow           , O(000F0F,90,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 74 , 1826, 194),
  INST(Pfcmpgt         , Ext3dNow           , O(000F0F,A0,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 74 , 1834, 194),
  INST(Pfmax           , Ext3dNow           , O(000F0F,A4,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 74 , 1842, 194),
  INST(Pfmin           , Ext3dNow           , O(000F0F,94,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 74 , 1848, 194),
  INST(Pfmul           , Ext3dNow           , O(000F0F,B4,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 74 , 1854, 194),
  INST(Pfnacc          , Ext3dNow           , O(000F0F,8A,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 84 , 1860, 194),
  INST(Pfpnacc         , Ext3dNow           , O(000F0F,8E,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 84 , 1867, 194),
  INST(Pfrcp           , Ext3dNow           , O(000F0F,96,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 74 , 1875, 204),
  INST(Pfrcpit1        , Ext3dNow           , O(000F0F,A6,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 74 , 1881, 194),
  INST(Pfrcpit2        , Ext3dNow           , O(000F0F,B6,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 74 , 1890, 194),
  INST(Pfrcpv          , Ext3dNow           , O(000F0F,86,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 85 , 1899, 194),
  INST(Pfrsqit1        , Ext3dNow           , O(000F0F,A7,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 0 , kFamilySse , 74 , 1906, 205),
  INST(Pfrsqrt         , Ext3dNow           , O(000F0F,97,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 0 , kFamilySse , 74 , 1915, 205),
  INST(Pfrsqrtv        , Ext3dNow           , O(000F0F,87,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 85 , 1923, 194),
  INST(Pfsub           , Ext3dNow           , O(000F0F,9A,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 74 , 1932, 194),
  INST(Pfsubr          , Ext3dNow           , O(000F0F,AA,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 74 , 1938, 194),
  INST(Phaddd          , ExtRm_P            , O(000F38,02,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 86 , 6667, 189),
  INST(Phaddsw         , ExtRm_P            , O(000F38,03,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 87 , 6684, 189),
  INST(Phaddw          , ExtRm_P            , O(000F38,01,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 88 , 6753, 189),
  INST(Phminposuw      , ExtRm              , O(660F38,41,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 89 , 6779, 6  ),
  INST(Phsubd          , ExtRm_P            , O(000F38,06,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 90 , 6800, 189),
  INST(Phsubsw         , ExtRm_P            , O(000F38,07,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 91 , 6817, 189),
  INST(Phsubw          , ExtRm_P            , O(000F38,05,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 91 , 6826, 189),
  INST(Pi2fd           , Ext3dNow           , O(000F0F,0D,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 74 , 1945, 204),
  INST(Pi2fw           , Ext3dNow           , O(000F0F,0C,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 84 , 1951, 204),
  INST(Pinsrb          , ExtRmi             , O(660F3A,20,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 92 , 6843, 206),
  INST(Pinsrd          , ExtRmi             , O(660F3A,22,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 92 , 6851, 207),
  INST(Pinsrq          , ExtRmi             , O(660F3A,22,_,_,1,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 92 , 6859, 208),
  INST(Pinsrw          , ExtRmi_P           , O(000F00,C4,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 93 , 6867, 209),
  INST(Pmaddubsw       , ExtRm_P            , O(000F38,04,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 94 , 7037, 189),
  INST(Pmaddwd         , ExtRm_P            , O(000F00,F5,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 95 , 7048, 189),
  INST(Pmaxsb          , ExtRm              , O(660F38,3C,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 96 , 7079, 14 ),
  INST(Pmaxsd          , ExtRm              , O(660F38,3D,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 96 , 7087, 14 ),
  INST(Pmaxsw          , ExtRm_P            , O(000F00,EE,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 97 , 7103, 191),
  INST(Pmaxub          , ExtRm_P            , O(000F00,DE,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 97 , 7111, 191),
  INST(Pmaxud          , ExtRm              , O(660F38,3F,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 98 , 7119, 14 ),
  INST(Pmaxuw          , ExtRm              , O(660F38,3E,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 99 , 7135, 14 ),
  INST(Pminsb          , ExtRm              , O(660F38,38,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 99 , 7143, 14 ),
  INST(Pminsd          , ExtRm              , O(660F38,39,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 99 , 7151, 14 ),
  INST(Pminsw          , ExtRm_P            , O(000F00,EA,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 100, 7167, 191),
  INST(Pminub          , ExtRm_P            , O(000F00,DA,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 100, 7175, 191),
  INST(Pminud          , ExtRm              , O(660F38,3B,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 101, 7183, 14 ),
  INST(Pminuw          , ExtRm              , O(660F38,3A,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 102, 7199, 14 ),
  INST(Pmovmskb        , ExtRm_P            , O(000F00,D7,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 103, 7277, 210),
  INST(Pmovsxbd        , ExtRm              , O(660F38,21,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 104, 7374, 211),
  INST(Pmovsxbq        , ExtRm              , O(660F38,22,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 104, 7384, 212),
  INST(Pmovsxbw        , ExtRm              , O(660F38,20,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 104, 7394, 56 ),
  INST(Pmovsxdq        , ExtRm              , O(660F38,25,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 104, 7404, 56 ),
  INST(Pmovsxwd        , ExtRm              , O(660F38,23,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 104, 7414, 56 ),
  INST(Pmovsxwq        , ExtRm              , O(660F38,24,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 104, 7424, 211),
  INST(Pmovzxbd        , ExtRm              , O(660F38,31,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 105, 7511, 211),
  INST(Pmovzxbq        , ExtRm              , O(660F38,32,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 105, 7521, 212),
  INST(Pmovzxbw        , ExtRm              , O(660F38,30,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 105, 7531, 56 ),
  INST(Pmovzxdq        , ExtRm              , O(660F38,35,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 105, 7541, 56 ),
  INST(Pmovzxwd        , ExtRm              , O(660F38,33,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 105, 7551, 56 ),
  INST(Pmovzxwq        , ExtRm              , O(660F38,34,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 105, 7561, 211),
  INST(Pmuldq          , ExtRm              , O(660F38,28,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 106, 7571, 6  ),
  INST(Pmulhrsw        , ExtRm_P            , O(000F38,0B,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 107, 7579, 189),
  INST(Pmulhrw         , Ext3dNow           , O(000F0F,B7,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 74 , 1957, 194),
  INST(Pmulhuw         , ExtRm_P            , O(000F00,E4,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 108, 7589, 189),
  INST(Pmulhw          , ExtRm_P            , O(000F00,E5,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 109, 7598, 189),
  INST(Pmulld          , ExtRm              , O(660F38,40,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 110, 7606, 6  ),
  INST(Pmullw          , ExtRm_P            , O(000F00,D5,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 111, 7622, 189),
  INST(Pmuludq         , ExtRm_P            , O(000F00,F4,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 112, 7645, 189),
  INST(Pop             , X86Pop             , O(000000,8F,0,_,_,_,_,_  ), O(000000,58,_,_,_,_,_,_  ), F(WO)|F(Volatile)|F(Special)          , EF(________), 0 , 0 , kFamilyNone, 0  , 1965, 213),
  INST(Popa            , X86Op              , O(660000,61,_,_,_,_,_,_  ), 0                         , F(Volatile)|F(Special)                , EF(________), 0 , 0 , kFamilyNone, 0  , 1969, 214),
  INST(Popad           , X86Op              , O(000000,61,_,_,_,_,_,_  ), 0                         , F(Volatile)|F(Special)                , EF(________), 0 , 0 , kFamilyNone, 0  , 1974, 214),
  INST(Popcnt          , X86Rm              , O(F30F00,B8,_,_,x,_,_,_  ), 0                         , F(WO)                                 , EF(WWWWWW__), 0 , 0 , kFamilyNone, 0  , 1980, 215),
  INST(Popf            , X86Op              , O(660000,9D,_,_,_,_,_,_  ), 0                         , F(Volatile)|F(Special)                , EF(WWWWWWWW), 0 , 0 , kFamilyNone, 0  , 1987, 216),
  INST(Popfd           , X86Op              , O(000000,9D,_,_,_,_,_,_  ), 0                         , F(Volatile)|F(Special)                , EF(WWWWWWWW), 0 , 0 , kFamilyNone, 0  , 1992, 217),
  INST(Popfq           , X86Op              , O(000000,9D,_,_,_,_,_,_  ), 0                         , F(Volatile)|F(Special)                , EF(WWWWWWWW), 0 , 0 , kFamilyNone, 0  , 1998, 218),
  INST(Por             , ExtRm_P            , O(000F00,EB,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 113, 7654, 191),
  INST(Prefetch        , X86M_Only          , O(000F00,0D,0,_,_,_,_,_  ), 0                         , F(RO)|F(Volatile)                     , EF(________), 0 , 0 , kFamilyNone, 0  , 2004, 33 ),
  INST(Prefetchnta     , X86M_Only          , O(000F00,18,0,_,_,_,_,_  ), 0                         , F(RO)|F(Volatile)                     , EF(________), 0 , 0 , kFamilyNone, 0  , 2013, 33 ),
  INST(Prefetcht0      , X86M_Only          , O(000F00,18,1,_,_,_,_,_  ), 0                         , F(RO)|F(Volatile)                     , EF(________), 0 , 0 , kFamilyNone, 0  , 2025, 33 ),
  INST(Prefetcht1      , X86M_Only          , O(000F00,18,2,_,_,_,_,_  ), 0                         , F(RO)|F(Volatile)                     , EF(________), 0 , 0 , kFamilyNone, 0  , 2036, 33 ),
  INST(Prefetcht2      , X86M_Only          , O(000F00,18,3,_,_,_,_,_  ), 0                         , F(RO)|F(Volatile)                     , EF(________), 0 , 0 , kFamilyNone, 0  , 2047, 33 ),
  INST(Prefetchw       , X86M_Only          , O(000F00,0D,1,_,_,_,_,_  ), 0                         , F(RO)|F(Volatile)                     , EF(UUUUUU__), 0 , 0 , kFamilyNone, 0  , 2058, 219),
  INST(Prefetchwt1     , X86M_Only          , O(000F00,0D,2,_,_,_,_,_  ), 0                         , F(RO)|F(Volatile)                     , EF(UUUUUU__), 0 , 0 , kFamilyNone, 0  , 2068, 219),
  INST(Psadbw          , ExtRm_P            , O(000F00,F6,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 114, 3543, 189),
  INST(Pshufb          , ExtRm_P            , O(000F38,00,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 115, 7878, 220),
  INST(Pshufd          , ExtRmi             , O(660F00,70,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 116, 7886, 221),
  INST(Pshufhw         , ExtRmi             , O(F30F00,70,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 116, 7894, 221),
  INST(Pshuflw         , ExtRmi             , O(F20F00,70,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 116, 7903, 221),
  INST(Pshufw          , ExtRmi_P           , O(000F00,70,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 37 , 2080, 222),
  INST(Psignb          , ExtRm_P            , O(000F38,08,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 117, 7912, 189),
  INST(Psignd          , ExtRm_P            , O(000F38,0A,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 117, 7920, 189),
  INST(Psignw          , ExtRm_P            , O(000F38,09,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 117, 7928, 189),
  INST(Pslld           , ExtRmRi_P          , O(000F00,F2,_,_,_,_,_,_  ), O(000F00,72,6,_,_,_,_,_  ), F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 118, 7936, 223),
  INST(Pslldq          , ExtRmRi            , 0                         , O(660F00,73,7,_,_,_,_,_  ), F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 119, 7943, 224),
  INST(Psllq           , ExtRmRi_P          , O(000F00,F3,_,_,_,_,_,_  ), O(000F00,73,6,_,_,_,_,_  ), F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 118, 7951, 225),
  INST(Psllw           , ExtRmRi_P          , O(000F00,F1,_,_,_,_,_,_  ), O(000F00,71,6,_,_,_,_,_  ), F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 120, 7982, 226),
  INST(Psrad           , ExtRmRi_P          , O(000F00,E2,_,_,_,_,_,_  ), O(000F00,72,4,_,_,_,_,_  ), F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 120, 7989, 227),
  INST(Psraw           , ExtRmRi_P          , O(000F00,E1,_,_,_,_,_,_  ), O(000F00,71,4,_,_,_,_,_  ), F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 121, 8027, 228),
  INST(Psrld           , ExtRmRi_P          , O(000F00,D2,_,_,_,_,_,_  ), O(000F00,72,2,_,_,_,_,_  ), F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 121, 8034, 229),
  INST(Psrldq          , ExtRmRi            , 0                         , O(660F00,73,3,_,_,_,_,_  ), F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 122, 8041, 230),
  INST(Psrlq           , ExtRmRi_P          , O(000F00,D3,_,_,_,_,_,_  ), O(000F00,73,2,_,_,_,_,_  ), F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 121, 8049, 231),
  INST(Psrlw           , ExtRmRi_P          , O(000F00,D1,_,_,_,_,_,_  ), O(000F00,71,2,_,_,_,_,_  ), F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 123, 8080, 232),
  INST(Psubb           , ExtRm_P            , O(000F00,F8,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 123, 8087, 192),
  INST(Psubd           , ExtRm_P            , O(000F00,FA,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 123, 8094, 192),
  INST(Psubq           , ExtRm_P            , O(000F00,FB,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 124, 8101, 192),
  INST(Psubsb          , ExtRm_P            , O(000F00,E8,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 123, 8108, 192),
  INST(Psubsw          , ExtRm_P            , O(000F00,E9,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 123, 8116, 192),
  INST(Psubusb         , ExtRm_P            , O(000F00,D8,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 123, 8124, 192),
  INST(Psubusw         , ExtRm_P            , O(000F00,D9,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 123, 8133, 192),
  INST(Psubw           , ExtRm_P            , O(000F00,F9,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 123, 8142, 192),
  INST(Pswapd          , Ext3dNow           , O(000F0F,BB,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 84 , 2087, 204),
  INST(Ptest           , ExtRm              , O(660F38,17,_,_,_,_,_,_  ), 0                         , F(RO)                                 , EF(WWWWWW__), 0 , 0 , kFamilySse , 125, 8171, 233),
  INST(Punpckhbw       , ExtRm_P            , O(000F00,68,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 126, 8254, 189),
  INST(Punpckhdq       , ExtRm_P            , O(000F00,6A,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 126, 8265, 189),
  INST(Punpckhqdq      , ExtRm              , O(660F00,6D,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 127, 8276, 6  ),
  INST(Punpckhwd       , ExtRm_P            , O(000F00,69,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 126, 8288, 189),
  INST(Punpcklbw       , ExtRm_P            , O(000F00,60,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 126, 8299, 189),
  INST(Punpckldq       , ExtRm_P            , O(000F00,62,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 126, 8310, 189),
  INST(Punpcklqdq      , ExtRm              , O(660F00,6C,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 127, 8321, 6  ),
  INST(Punpcklwd       , ExtRm_P            , O(000F00,61,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 126, 8333, 189),
  INST(Push            , X86Push            , O(000000,FF,6,_,_,_,_,_  ), O(000000,50,_,_,_,_,_,_  ), F(RO)|F(Volatile)|F(Special)          , EF(________), 0 , 0 , kFamilyNone, 0  , 2094, 234),
  INST(Pusha           , X86Op              , O(660000,60,_,_,_,_,_,_  ), 0                         , F(Volatile)|F(Special)                , EF(________), 0 , 0 , kFamilyNone, 0  , 2099, 214),
  INST(Pushad          , X86Op              , O(000000,60,_,_,_,_,_,_  ), 0                         , F(Volatile)|F(Special)                , EF(________), 0 , 0 , kFamilyNone, 0  , 2105, 214),
  INST(Pushf           , X86Op              , O(660000,9C,_,_,_,_,_,_  ), 0                         , F(Volatile)|F(Special)                , EF(RRRRRRRR), 0 , 0 , kFamilyNone, 0  , 2112, 235),
  INST(Pushfd          , X86Op              , O(000000,9C,_,_,_,_,_,_  ), 0                         , F(Volatile)|F(Special)                , EF(RRRRRRRR), 0 , 0 , kFamilyNone, 0  , 2118, 236),
  INST(Pushfq          , X86Op              , O(000000,9C,_,_,_,_,_,_  ), 0                         , F(Volatile)|F(Special)                , EF(RRRRRRRR), 0 , 0 , kFamilyNone, 0  , 2125, 237),
  INST(Pxor            , ExtRm_P            , O(000F00,EF,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 128, 8344, 192),
  INST(Rcl             , X86Rot             , O(000000,D0,2,_,x,_,_,_  ), 0                         , F(RW)|F(Special)                      , EF(W____X__), 0 , 0 , kFamilyNone, 0  , 2132, 238),
  INST(Rcpps           , ExtRm              , O(000F00,53,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 129, 8472, 57 ),
  INST(Rcpss           , ExtRm              , O(F30F00,53,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 4 , kFamilySse , 130, 8479, 239),
  INST(Rcr             , X86Rot             , O(000000,D0,3,_,x,_,_,_  ), 0                         , F(RW)|F(Special)                      , EF(W____X__), 0 , 0 , kFamilyNone, 0  , 2136, 238),
  INST(Rdfsbase        , X86M               , O(F30F00,AE,0,_,x,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilyNone, 0  , 2140, 240),
  INST(Rdgsbase        , X86M               , O(F30F00,AE,1,_,x,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilyNone, 0  , 2149, 240),
  INST(Rdrand          , X86M               , O(000F00,C7,6,_,x,_,_,_  ), 0                         , F(WO)                                 , EF(WWWWWW__), 0 , 8 , kFamilyNone, 0  , 2158, 241),
  INST(Rdseed          , X86M               , O(000F00,C7,7,_,x,_,_,_  ), 0                         , F(WO)                                 , EF(WWWWWW__), 0 , 8 , kFamilyNone, 0  , 2165, 241),
  INST(Rdtsc           , X86Op              , O(000F00,31,_,_,_,_,_,_  ), 0                         , F(WO)|F(Volatile)|F(Special)          , EF(________), 0 , 0 , kFamilyNone, 0  , 2172, 242),
  INST(Rdtscp          , X86Op              , O(000F01,F9,_,_,_,_,_,_  ), 0                         , F(WO)|F(Volatile)|F(Special)          , EF(________), 0 , 0 , kFamilyNone, 0  , 2178, 243),
  INST(Ret             , X86Ret             , O(000000,C2,_,_,_,_,_,_  ), 0                         , F(RW)|F(Volatile)|F(Special)          , EF(________), 0 , 0 , kFamilyNone, 0  , 2185, 244),
  INST(Rol             , X86Rot             , O(000000,D0,0,_,x,_,_,_  ), 0                         , F(RW)|F(Special)                      , EF(W____W__), 0 , 0 , kFamilyNone, 0  , 2189, 245),
  INST(Ror             , X86Rot             , O(000000,D0,1,_,x,_,_,_  ), 0                         , F(RW)|F(Special)                      , EF(W____W__), 0 , 0 , kFamilyNone, 0  , 2193, 245),
  INST(Rorx            , VexRmi_Wx          , V(F20F3A,F0,_,0,x,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 2197, 246),
  INST(Roundpd         , ExtRmi             , O(660F3A,09,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 131, 8574, 221),
  INST(Roundps         , ExtRmi             , O(660F3A,08,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 131, 8583, 221),
  INST(Roundsd         , ExtRmi             , O(660F3A,0B,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 132, 8592, 247),
  INST(Roundss         , ExtRmi             , O(660F3A,0A,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 4 , kFamilySse , 132, 8601, 248),
  INST(Rsqrtps         , ExtRm              , O(000F00,52,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 133, 8698, 57 ),
  INST(Rsqrtss         , ExtRm              , O(F30F00,52,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 4 , kFamilySse , 134, 8707, 239),
  INST(Sahf            , X86Op              , O(000000,9E,_,_,_,_,_,_  ), 0                         , F(RO)|F(Volatile)|F(Special)          , EF(_WWWWW__), 0 , 0 , kFamilyNone, 0  , 2202, 249),
  INST(Sal             , X86Rot             , O(000000,D0,4,_,x,_,_,_  ), 0                         , F(RW)|F(Special)                      , EF(WWWUWW__), 0 , 0 , kFamilyNone, 0  , 2207, 250),
  INST(Sar             , X86Rot             , O(000000,D0,7,_,x,_,_,_  ), 0                         , F(RW)|F(Special)                      , EF(WWWUWW__), 0 , 0 , kFamilyNone, 0  , 2211, 250),
  INST(Sarx            , VexRmv_Wx          , V(F30F38,F7,_,0,x,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 2215, 251),
  INST(Sbb             , X86Arith           , O(000000,18,3,_,x,_,_,_  ), 0                         , F(RW)|F(Lock)                         , EF(WWWWWX__), 0 , 0 , kFamilyNone, 0  , 2220, 3  ),
  INST(Scas            , X86StrRm           , O(000000,AE,_,_,_,_,_,_  ), 0                         , F(RW)|F(Special)|F(Rep)|F(Repnz)      , EF(WWWWWWR_), 0 , 0 , kFamilyNone, 0  , 2224, 252),
  INST(Seta            , X86Set             , O(000F00,97,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(__R__R__), 0 , 1 , kFamilyNone, 0  , 2229, 253),
  INST(Setae           , X86Set             , O(000F00,93,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(_____R__), 0 , 1 , kFamilyNone, 0  , 2234, 254),
  INST(Setb            , X86Set             , O(000F00,92,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(_____R__), 0 , 1 , kFamilyNone, 0  , 2240, 254),
  INST(Setbe           , X86Set             , O(000F00,96,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(__R__R__), 0 , 1 , kFamilyNone, 0  , 2245, 253),
  INST(Setc            , X86Set             , O(000F00,92,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(_____R__), 0 , 1 , kFamilyNone, 0  , 2251, 254),
  INST(Sete            , X86Set             , O(000F00,94,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(__R_____), 0 , 1 , kFamilyNone, 0  , 2256, 255),
  INST(Setg            , X86Set             , O(000F00,9F,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(RRR_____), 0 , 1 , kFamilyNone, 0  , 2261, 256),
  INST(Setge           , X86Set             , O(000F00,9D,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(RR______), 0 , 1 , kFamilyNone, 0  , 2266, 257),
  INST(Setl            , X86Set             , O(000F00,9C,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(RR______), 0 , 1 , kFamilyNone, 0  , 2272, 257),
  INST(Setle           , X86Set             , O(000F00,9E,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(RRR_____), 0 , 1 , kFamilyNone, 0  , 2277, 256),
  INST(Setna           , X86Set             , O(000F00,96,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(__R__R__), 0 , 1 , kFamilyNone, 0  , 2283, 253),
  INST(Setnae          , X86Set             , O(000F00,92,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(_____R__), 0 , 1 , kFamilyNone, 0  , 2289, 254),
  INST(Setnb           , X86Set             , O(000F00,93,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(_____R__), 0 , 1 , kFamilyNone, 0  , 2296, 254),
  INST(Setnbe          , X86Set             , O(000F00,97,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(__R__R__), 0 , 1 , kFamilyNone, 0  , 2302, 253),
  INST(Setnc           , X86Set             , O(000F00,93,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(_____R__), 0 , 1 , kFamilyNone, 0  , 2309, 254),
  INST(Setne           , X86Set             , O(000F00,95,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(__R_____), 0 , 1 , kFamilyNone, 0  , 2315, 255),
  INST(Setng           , X86Set             , O(000F00,9E,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(RRR_____), 0 , 1 , kFamilyNone, 0  , 2321, 256),
  INST(Setnge          , X86Set             , O(000F00,9C,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(RR______), 0 , 1 , kFamilyNone, 0  , 2327, 257),
  INST(Setnl           , X86Set             , O(000F00,9D,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(RR______), 0 , 1 , kFamilyNone, 0  , 2334, 257),
  INST(Setnle          , X86Set             , O(000F00,9F,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(RRR_____), 0 , 1 , kFamilyNone, 0  , 2340, 256),
  INST(Setno           , X86Set             , O(000F00,91,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(R_______), 0 , 1 , kFamilyNone, 0  , 2347, 258),
  INST(Setnp           , X86Set             , O(000F00,9B,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(____R___), 0 , 1 , kFamilyNone, 0  , 2353, 259),
  INST(Setns           , X86Set             , O(000F00,99,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(_R______), 0 , 1 , kFamilyNone, 0  , 2359, 260),
  INST(Setnz           , X86Set             , O(000F00,95,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(__R_____), 0 , 1 , kFamilyNone, 0  , 2365, 255),
  INST(Seto            , X86Set             , O(000F00,90,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(R_______), 0 , 1 , kFamilyNone, 0  , 2371, 258),
  INST(Setp            , X86Set             , O(000F00,9A,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(____R___), 0 , 1 , kFamilyNone, 0  , 2376, 259),
  INST(Setpe           , X86Set             , O(000F00,9A,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(____R___), 0 , 1 , kFamilyNone, 0  , 2381, 259),
  INST(Setpo           , X86Set             , O(000F00,9B,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(____R___), 0 , 1 , kFamilyNone, 0  , 2387, 259),
  INST(Sets            , X86Set             , O(000F00,98,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(_R______), 0 , 1 , kFamilyNone, 0  , 2393, 260),
  INST(Setz            , X86Set             , O(000F00,94,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(__R_____), 0 , 1 , kFamilyNone, 0  , 2398, 255),
  INST(Sfence          , X86Fence           , O(000F00,AE,7,_,_,_,_,_  ), 0                         , F(Volatile)                           , EF(________), 0 , 0 , kFamilyNone, 0  , 2403, 73 ),
  INST(Sha1msg1        , ExtRm              , O(000F38,C9,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 135, 2410, 6  ),
  INST(Sha1msg2        , ExtRm              , O(000F38,CA,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 135, 2419, 6  ),
  INST(Sha1nexte       , ExtRm              , O(000F38,C8,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 135, 2428, 6  ),
  INST(Sha1rnds4       , ExtRmi             , O(000F3A,CC,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 135, 2438, 17 ),
  INST(Sha256msg1      , ExtRm              , O(000F38,CC,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 135, 2448, 6  ),
  INST(Sha256msg2      , ExtRm              , O(000F38,CD,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 135, 2459, 6  ),
  INST(Sha256rnds2     , ExtRm_XMM0         , O(000F38,CB,_,_,_,_,_,_  ), 0                         , F(RW)|F(Special)                      , EF(________), 0 , 0 , kFamilySse , 135, 2470, 18 ),
  INST(Shl             , X86Rot             , O(000000,D0,4,_,x,_,_,_  ), 0                         , F(RW)|F(Special)                      , EF(WWWUWW__), 0 , 0 , kFamilyNone, 0  , 2482, 250),
  INST(Shld            , X86ShldShrd        , O(000F00,A4,_,_,x,_,_,_  ), 0                         , F(RW)|F(Special)                      , EF(UWWUWW__), 0 , 0 , kFamilyNone, 0  , 7858, 261),
  INST(Shlx            , VexRmv_Wx          , V(660F38,F7,_,0,x,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 2486, 251),
  INST(Shr             , X86Rot             , O(000000,D0,5,_,x,_,_,_  ), 0                         , F(RW)|F(Special)                      , EF(WWWUWW__), 0 , 0 , kFamilyNone, 0  , 2491, 250),
  INST(Shrd            , X86ShldShrd        , O(000F00,AC,_,_,x,_,_,_  ), 0                         , F(RW)|F(Special)                      , EF(UWWUWW__), 0 , 0 , kFamilyNone, 0  , 2495, 261),
  INST(Shrx            , VexRmv_Wx          , V(F20F38,F7,_,0,x,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 2500, 251),
  INST(Shufpd          , ExtRmi             , O(660F00,C6,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 136, 8968, 17 ),
  INST(Shufps          , ExtRmi             , O(000F00,C6,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 137, 8976, 17 ),
  INST(Sqrtpd          , ExtRm              , O(660F00,51,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 116, 8984, 57 ),
  INST(Sqrtps          , ExtRm              , O(000F00,51,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 138, 8699, 57 ),
  INST(Sqrtsd          , ExtRm              , O(F20F00,51,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 136, 9000, 262),
  INST(Sqrtss          , ExtRm              , O(F30F00,51,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 4 , kFamilySse , 137, 8708, 239),
  INST(Stac            , X86Op              , O(000F01,CB,_,_,_,_,_,_  ), 0                         , F(Volatile)                           , EF(___W____), 0 , 0 , kFamilyNone, 0  , 2505, 30 ),
  INST(Stc             , X86Op              , O(000000,F9,_,_,_,_,_,_  ), 0                         , 0                                     , EF(_____W__), 0 , 0 , kFamilyNone, 0  , 2510, 263),
  INST(Std             , X86Op              , O(000000,FD,_,_,_,_,_,_  ), 0                         , 0                                     , EF(______W_), 0 , 0 , kFamilyNone, 0  , 5959, 264),
  INST(Sti             , X86Op              , O(000000,FB,_,_,_,_,_,_  ), 0                         , 0                                     , EF(_______W), 0 , 0 , kFamilyNone, 0  , 2514, 265),
  INST(Stmxcsr         , X86M_Only          , O(000F00,AE,3,_,_,_,_,_  ), 0                         , F(Volatile)                           , EF(________), 0 , 0 , kFamilyNone, 0  , 9016, 266),
  INST(Stos            , X86StrMr           , O(000000,AA,_,_,_,_,_,_  ), 0                         , F(RW)|F(Special)|F(Rep)               , EF(______R_), 0 , 0 , kFamilyNone, 0  , 2518, 267),
  INST(Sub             , X86Arith           , O(000000,28,5,_,x,_,_,_  ), 0                         , F(RW)|F(Lock)                         , EF(WWWWWW__), 0 , 0 , kFamilyNone, 0  , 753 , 268),
  INST(Subpd           , ExtRm              , O(660F00,5C,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 139, 4099, 6  ),
  INST(Subps           , ExtRm              , O(000F00,5C,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 140, 4111, 6  ),
  INST(Subsd           , ExtRm              , O(F20F00,5C,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 139, 4787, 7  ),
  INST(Subss           , ExtRm              , O(F30F00,5C,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 140, 4797, 8  ),
  INST(Swapgs          , X86Op              , O(000F01,F8,_,_,_,_,_,_  ), 0                         , 0                                     , EF(________), 0 , 0 , kFamilyNone, 0  , 2523, 269),
  INST(T1mskc          , VexVm_Wx           , V(XOP_M9,01,7,0,x,_,_,_  ), 0                         , F(WO)                                 , EF(WWWUUW__), 0 , 0 , kFamilyNone, 0  , 2530, 16 ),
  INST(Test            , X86Test            , O(000000,84,_,_,x,_,_,_  ), O(000000,F6,_,_,x,_,_,_  ), F(RO)                                 , EF(WWWUWW__), 0 , 0 , kFamilyNone, 0  , 8172, 270),
  INST(Tzcnt           , X86Rm              , O(F30F00,BC,_,_,x,_,_,_  ), 0                         , F(WO)                                 , EF(UUWUUW__), 0 , 0 , kFamilyNone, 0  , 2537, 215),
  INST(Tzmsk           , VexVm_Wx           , V(XOP_M9,01,4,0,x,_,_,_  ), 0                         , F(WO)                                 , EF(WWWUUW__), 0 , 0 , kFamilyNone, 0  , 2543, 16 ),
  INST(Ucomisd         , ExtRm              , O(660F00,2E,_,_,_,_,_,_  ), 0                         , F(RO)                                 , EF(WWWWWW__), 0 , 0 , kFamilySse , 141, 9069, 51 ),
  INST(Ucomiss         , ExtRm              , O(000F00,2E,_,_,_,_,_,_  ), 0                         , F(RO)                                 , EF(WWWWWW__), 0 , 0 , kFamilySse , 142, 9078, 52 ),
  INST(Ud2             , X86Op              , O(000F00,0B,_,_,_,_,_,_  ), 0                         , 0                                     , EF(________), 0 , 0 , kFamilyNone, 0  , 2549, 271),
  INST(Unpckhpd        , ExtRm              , O(660F00,15,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 143, 9087, 6  ),
  INST(Unpckhps        , ExtRm              , O(000F00,15,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 144, 9097, 6  ),
  INST(Unpcklpd        , ExtRm              , O(660F00,14,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 143, 9107, 6  ),
  INST(Unpcklps        , ExtRm              , O(000F00,14,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 144, 9117, 6  ),
  INST(Vaddpd          , VexRvm_Lx          , V(660F00,58,_,x,I,1,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 0  , 2553, 272),
  INST(Vaddps          , VexRvm_Lx          , V(000F00,58,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 1  , 2560, 272),
  INST(Vaddsd          , VexRvm             , V(F20F00,58,_,I,I,1,3,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 2  , 2567, 273),
  INST(Vaddss          , VexRvm             , V(F30F00,58,_,I,I,0,2,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 2  , 2574, 274),
  INST(Vaddsubpd       , VexRvm_Lx          , V(660F00,D0,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 3  , 2581, 275),
  INST(Vaddsubps       , VexRvm_Lx          , V(F20F00,D0,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 3  , 2591, 275),
  INST(Vaesdec         , VexRvm             , V(660F38,DE,_,0,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 4  , 2601, 276),
  INST(Vaesdeclast     , VexRvm             , V(660F38,DF,_,0,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 4  , 2609, 276),
  INST(Vaesenc         , VexRvm             , V(660F38,DC,_,0,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 4  , 2621, 276),
  INST(Vaesenclast     , VexRvm             , V(660F38,DD,_,0,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 4  , 2629, 276),
  INST(Vaesimc         , VexRm              , V(660F38,DB,_,0,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 4  , 2641, 277),
  INST(Vaeskeygenassist, VexRmi             , V(660F3A,DF,_,0,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 4  , 2649, 278),
  INST(Valignd         , VexRvmi_Lx         , V(660F3A,03,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 5  , 2666, 279),
  INST(Valignq         , VexRvmi_Lx         , V(660F3A,03,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 6  , 2674, 279),
  INST(Vandnpd         , VexRvm_Lx          , V(660F00,55,_,x,I,1,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 7  , 2682, 272),
  INST(Vandnps         , VexRvm_Lx          , V(000F00,55,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 8  , 2690, 272),
  INST(Vandpd          , VexRvm_Lx          , V(660F00,54,_,x,I,1,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 7  , 2698, 280),
  INST(Vandps          , VexRvm_Lx          , V(000F00,54,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 8  , 2705, 280),
  INST(Vblendmb        , VexRvm_Lx          , V(660F38,66,_,x,_,0,4,FVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 9  , 2712, 281),
  INST(Vblendmd        , VexRvm_Lx          , V(660F38,64,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 5  , 2721, 281),
  INST(Vblendmpd       , VexRvm_Lx          , V(660F38,65,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 6  , 2730, 281),
  INST(Vblendmps       , VexRvm_Lx          , V(660F38,65,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 5  , 2740, 281),
  INST(Vblendmq        , VexRvm_Lx          , V(660F38,64,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 6  , 2750, 281),
  INST(Vblendmw        , VexRvm_Lx          , V(660F38,66,_,x,_,1,4,FVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 9  , 2759, 281),
  INST(Vblendpd        , VexRvmi_Lx         , V(660F3A,0D,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 3  , 2768, 282),
  INST(Vblendps        , VexRvmi_Lx         , V(660F3A,0C,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 3  , 2777, 282),
  INST(Vblendvpd       , VexRvmr_Lx         , V(660F3A,4B,_,x,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 3  , 2786, 283),
  INST(Vblendvps       , VexRvmr_Lx         , V(660F3A,4A,_,x,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 3  , 2796, 283),
  INST(Vbroadcastf128  , VexRm              , V(660F38,1A,_,1,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 3  , 2806, 284),
  INST(Vbroadcastf32x2 , VexRm_Lx           , V(660F38,19,_,x,_,0,3,T2 ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 10 , 2821, 285),
  INST(Vbroadcastf32x4 , VexRm_Lx           , V(660F38,1A,_,x,_,0,4,T4 ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 11 , 2837, 286),
  INST(Vbroadcastf32x8 , VexRm              , V(660F38,1B,_,2,_,0,5,T8 ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 12 , 2853, 287),
  INST(Vbroadcastf64x2 , VexRm_Lx           , V(660F38,1A,_,x,_,1,4,T2 ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 10 , 2869, 286),
  INST(Vbroadcastf64x4 , VexRm              , V(660F38,1B,_,2,_,1,5,T4 ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 11 , 2885, 287),
  INST(Vbroadcasti128  , VexRm              , V(660F38,5A,_,1,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 13 , 2901, 284),
  INST(Vbroadcasti32x2 , VexRm_Lx           , V(660F38,59,_,x,_,0,3,T2 ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 10 , 2916, 288),
  INST(Vbroadcasti32x4 , VexRm_Lx           , V(660F38,5A,_,x,_,0,4,T4 ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 14 , 2932, 285),
  INST(Vbroadcasti32x8 , VexRm              , V(660F38,5B,_,2,_,0,5,T8 ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 12 , 2948, 289),
  INST(Vbroadcasti64x2 , VexRm_Lx           , V(660F38,5A,_,x,_,1,4,T2 ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 10 , 2964, 285),
  INST(Vbroadcasti64x4 , VexRm              , V(660F38,5B,_,2,_,1,5,T4 ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 11 , 2980, 289),
  INST(Vbroadcastsd    , VexRm_Lx           , V(660F38,19,_,x,0,1,3,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 15 , 2996, 290),
  INST(Vbroadcastss    , VexRm_Lx           , V(660F38,18,_,x,0,0,2,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 15 , 3009, 291),
  INST(Vcmppd          , VexRvmi_Lx         , V(660F00,C2,_,x,I,1,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 16 , 3022, 292),
  INST(Vcmpps          , VexRvmi_Lx         , V(000F00,C2,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 17 , 3029, 292),
  INST(Vcmpsd          , VexRvmi            , V(F20F00,C2,_,I,I,1,3,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 18 , 3036, 293),
  INST(Vcmpss          , VexRvmi            , V(F30F00,C2,_,I,I,0,2,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 18 , 3043, 294),
  INST(Vcomisd         , VexRm              , V(660F00,2F,_,I,I,1,3,T1S), 0                         , F(RO)|F(Vex)|F(Evex)                  , EF(WWWWWW__), 0 , 0 , kFamilyAvx , 19 , 3050, 295),
  INST(Vcomiss         , VexRm              , V(000F00,2F,_,I,I,0,2,T1S), 0                         , F(RO)|F(Vex)|F(Evex)                  , EF(WWWWWW__), 0 , 0 , kFamilyAvx , 19 , 3058, 296),
  INST(Vcompresspd     , VexMr_Lx           , V(660F38,8A,_,x,_,1,3,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 14 , 3066, 297),
  INST(Vcompressps     , VexMr_Lx           , V(660F38,8A,_,x,_,0,2,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 14 , 3078, 297),
  INST(Vcvtdq2pd       , VexRm_Lx           , V(F30F00,E6,_,x,I,0,3,HV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 20 , 3090, 298),
  INST(Vcvtdq2ps       , VexRm_Lx           , V(000F00,5B,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 1  , 3100, 299),
  INST(Vcvtpd2dq       , VexRm_Lx           , V(F20F00,E6,_,x,I,1,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 0  , 3110, 300),
  INST(Vcvtpd2ps       , VexRm_Lx           , V(660F00,5A,_,x,I,1,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 0  , 3120, 301),
  INST(Vcvtpd2qq       , VexRm_Lx           , V(660F00,7B,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 21 , 3130, 302),
  INST(Vcvtpd2udq      , VexRm_Lx           , V(000F00,79,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 22 , 3140, 303),
  INST(Vcvtpd2uqq      , VexRm_Lx           , V(660F00,79,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 21 , 3151, 302),
  INST(Vcvtph2ps       , VexRm_Lx           , V(660F38,13,_,x,0,0,3,HVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 23 , 3162, 298),
  INST(Vcvtps2dq       , VexRm_Lx           , V(660F00,5B,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 1  , 3172, 299),
  INST(Vcvtps2pd       , VexRm_Lx           , V(000F00,5A,_,x,I,0,4,HV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 1  , 3182, 298),
  INST(Vcvtps2ph       , VexMri_Lx          , V(660F3A,1D,_,x,0,0,3,HVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 23 , 3192, 304),
  INST(Vcvtps2qq       , VexRm_Lx           , V(660F00,7B,_,x,_,0,3,HV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 24 , 3202, 305),
  INST(Vcvtps2udq      , VexRm_Lx           , V(000F00,79,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 25 , 3212, 302),
  INST(Vcvtps2uqq      , VexRm_Lx           , V(660F00,79,_,x,_,0,3,HV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 24 , 3223, 305),
  INST(Vcvtqq2pd       , VexRm_Lx           , V(F30F00,E6,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 21 , 3234, 302),
  INST(Vcvtqq2ps       , VexRm_Lx           , V(000F00,5B,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 21 , 3244, 303),
  INST(Vcvtsd2si       , VexRm              , V(F20F00,2D,_,I,x,x,3,T1F), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 26 , 3254, 306),
  INST(Vcvtsd2ss       , VexRvm             , V(F20F00,5A,_,I,I,1,3,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 2  , 3264, 273),
  INST(Vcvtsd2usi      , VexRm              , V(F20F00,79,_,I,_,x,3,T1F), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 27 , 3274, 307),
  INST(Vcvtsi2sd       , VexRvm             , V(F20F00,2A,_,I,x,x,2,T1W), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 26 , 3285, 308),
  INST(Vcvtsi2ss       , VexRvm             , V(F30F00,2A,_,I,x,x,2,T1W), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 26 , 3295, 308),
  INST(Vcvtss2sd       , VexRvm             , V(F30F00,5A,_,I,I,0,2,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 18 , 3305, 274),
  INST(Vcvtss2si       , VexRm              , V(F20F00,2D,_,I,x,x,2,T1F), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 26 , 3315, 309),
  INST(Vcvtss2usi      , VexRm              , V(F30F00,79,_,I,_,x,2,T1F), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 27 , 3325, 310),
  INST(Vcvttpd2dq      , VexRm_Lx           , V(660F00,E6,_,x,I,1,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 16 , 3336, 300),
  INST(Vcvttpd2qq      , VexRm_Lx           , V(660F00,7A,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 28 , 3347, 302),
  INST(Vcvttpd2udq     , VexRm_Lx           , V(000F00,78,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 28 , 3358, 303),
  INST(Vcvttpd2uqq     , VexRm_Lx           , V(660F00,78,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 29 , 3370, 302),
  INST(Vcvttps2dq      , VexRm_Lx           , V(F30F00,5B,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 17 , 3382, 299),
  INST(Vcvttps2qq      , VexRm_Lx           , V(660F00,7A,_,x,_,0,3,HV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 30 , 3393, 305),
  INST(Vcvttps2udq     , VexRm_Lx           , V(000F00,78,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 31 , 3404, 302),
  INST(Vcvttps2uqq     , VexRm_Lx           , V(660F00,78,_,x,_,0,3,HV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 30 , 3416, 305),
  INST(Vcvttsd2si      , VexRm              , V(F20F00,2C,_,I,x,x,3,T1F), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 19 , 3428, 306),
  INST(Vcvttsd2usi     , VexRm              , V(F20F00,78,_,I,_,x,3,T1F), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 32 , 3439, 307),
  INST(Vcvttss2si      , VexRm              , V(F30F00,2C,_,I,x,x,2,T1F), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 19 , 3451, 309),
  INST(Vcvttss2usi     , VexRm              , V(F30F00,78,_,I,_,x,2,T1F), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 32 , 3462, 310),
  INST(Vcvtudq2pd      , VexRm_Lx           , V(F30F00,7A,_,x,_,0,3,HV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 5  , 3474, 305),
  INST(Vcvtudq2ps      , VexRm_Lx           , V(F20F00,7A,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 25 , 3485, 302),
  INST(Vcvtuqq2pd      , VexRm_Lx           , V(F30F00,7A,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 21 , 3496, 302),
  INST(Vcvtuqq2ps      , VexRm_Lx           , V(F20F00,7A,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 21 , 3507, 303),
  INST(Vcvtusi2sd      , VexRvm             , V(F20F00,7B,_,I,_,x,2,T1W), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 27 , 3518, 311),
  INST(Vcvtusi2ss      , VexRvm             , V(F30F00,7B,_,I,_,x,2,T1W), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 27 , 3529, 311),
  INST(Vdbpsadbw       , VexRvmi_Lx         , V(660F3A,42,_,x,_,0,4,FVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 9  , 3540, 279),
  INST(Vdivpd          , VexRvm_Lx          , V(660F00,5E,_,x,I,1,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 0  , 3550, 272),
  INST(Vdivps          , VexRvm_Lx          , V(000F00,5E,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 1  , 3557, 272),
  INST(Vdivsd          , VexRvm             , V(F20F00,5E,_,I,I,1,3,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 2  , 3564, 273),
  INST(Vdivss          , VexRvm             , V(F30F00,5E,_,I,I,0,2,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 2  , 3571, 274),
  INST(Vdppd           , VexRvmi_Lx         , V(660F3A,41,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 3  , 3578, 282),
  INST(Vdpps           , VexRvmi_Lx         , V(660F3A,40,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 3  , 3584, 282),
  INST(Vexp2pd         , VexRm              , V(660F38,C8,_,2,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 33 , 3590, 312),
  INST(Vexp2ps         , VexRm              , V(660F38,C8,_,2,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 34 , 3598, 312),
  INST(Vexpandpd       , VexRm_Lx           , V(660F38,88,_,x,_,1,3,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 14 , 3606, 302),
  INST(Vexpandps       , VexRm_Lx           , V(660F38,88,_,x,_,0,2,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 14 , 3616, 302),
  INST(Vextractf128    , VexMri             , V(660F3A,19,_,1,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 3  , 3626, 313),
  INST(Vextractf32x4   , VexMri_Lx          , V(660F3A,19,_,x,_,0,4,T4 ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 14 , 3639, 314),
  INST(Vextractf32x8   , VexMri             , V(660F3A,1B,_,2,_,0,5,T8 ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 12 , 3653, 315),
  INST(Vextractf64x2   , VexMri_Lx          , V(660F3A,19,_,x,_,1,4,T2 ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 10 , 3667, 314),
  INST(Vextractf64x4   , VexMri             , V(660F3A,1B,_,2,_,1,5,T4 ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 11 , 3681, 315),
  INST(Vextracti128    , VexMri             , V(660F3A,39,_,1,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 13 , 3695, 313),
  INST(Vextracti32x4   , VexMri_Lx          , V(660F3A,39,_,x,_,0,4,T4 ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 14 , 3708, 314),
  INST(Vextracti32x8   , VexMri             , V(660F3A,3B,_,2,_,0,5,T8 ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 12 , 3722, 315),
  INST(Vextracti64x2   , VexMri_Lx          , V(660F3A,39,_,x,_,1,4,T2 ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 10 , 3736, 314),
  INST(Vextracti64x4   , VexMri             , V(660F3A,3B,_,2,_,1,5,T4 ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 11 , 3750, 315),
  INST(Vextractps      , VexMri             , V(660F3A,17,_,0,I,I,2,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 35 , 3764, 316),
  INST(Vfixupimmpd     , VexRvmi_Lx         , V(660F3A,54,_,x,_,1,4,FV ), 0                         , F(RW)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 28 , 3775, 317),
  INST(Vfixupimmps     , VexRvmi_Lx         , V(660F3A,54,_,x,_,0,4,FV ), 0                         , F(RW)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 31 , 3787, 317),
  INST(Vfixupimmsd     , VexRvmi            , V(660F3A,55,_,I,_,1,3,T1S), 0                         , F(RW)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 36 , 3799, 318),
  INST(Vfixupimmss     , VexRvmi            , V(660F3A,55,_,I,_,0,2,T1S), 0                         , F(RW)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 36 , 3811, 319),
  INST(Vfmadd132pd     , VexRvm_Lx          , V(660F38,98,_,x,1,1,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 37 , 3823, 320),
  INST(Vfmadd132ps     , VexRvm_Lx          , V(660F38,98,_,x,0,0,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 38 , 3835, 320),
  INST(Vfmadd132sd     , VexRvm             , V(660F38,99,_,I,1,1,3,T1S), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 39 , 3847, 321),
  INST(Vfmadd132ss     , VexRvm             , V(660F38,99,_,I,0,0,2,T1S), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 39 , 3859, 322),
  INST(Vfmadd213pd     , VexRvm_Lx          , V(660F38,A8,_,x,1,1,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 37 , 3871, 320),
  INST(Vfmadd213ps     , VexRvm_Lx          , V(660F38,A8,_,x,0,0,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 38 , 3883, 320),
  INST(Vfmadd213sd     , VexRvm             , V(660F38,A9,_,I,1,1,3,T1S), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 39 , 3895, 321),
  INST(Vfmadd213ss     , VexRvm             , V(660F38,A9,_,I,0,0,2,T1S), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 39 , 3907, 322),
  INST(Vfmadd231pd     , VexRvm_Lx          , V(660F38,B8,_,x,1,1,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 37 , 3919, 320),
  INST(Vfmadd231ps     , VexRvm_Lx          , V(660F38,B8,_,x,0,0,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 38 , 3931, 320),
  INST(Vfmadd231sd     , VexRvm             , V(660F38,B9,_,I,1,1,3,T1S), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 39 , 3943, 321),
  INST(Vfmadd231ss     , VexRvm             , V(660F38,B9,_,I,0,0,2,T1S), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 39 , 3955, 322),
  INST(Vfmaddpd        , Fma4_Lx            , V(660F3A,69,_,x,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 40 , 3967, 323),
  INST(Vfmaddps        , Fma4_Lx            , V(660F3A,68,_,x,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 40 , 3976, 323),
  INST(Vfmaddsd        , Fma4               , V(660F3A,6B,_,0,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 40 , 3985, 324),
  INST(Vfmaddss        , Fma4               , V(660F3A,6A,_,0,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 40 , 3994, 325),
  INST(Vfmaddsub132pd  , VexRvm_Lx          , V(660F38,96,_,x,1,1,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 37 , 4003, 320),
  INST(Vfmaddsub132ps  , VexRvm_Lx          , V(660F38,96,_,x,0,0,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 38 , 4018, 320),
  INST(Vfmaddsub213pd  , VexRvm_Lx          , V(660F38,A6,_,x,1,1,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 37 , 4033, 320),
  INST(Vfmaddsub213ps  , VexRvm_Lx          , V(660F38,A6,_,x,0,0,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 38 , 4048, 320),
  INST(Vfmaddsub231pd  , VexRvm_Lx          , V(660F38,B6,_,x,1,1,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 37 , 4063, 320),
  INST(Vfmaddsub231ps  , VexRvm_Lx          , V(660F38,B6,_,x,0,0,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 38 , 4078, 320),
  INST(Vfmaddsubpd     , Fma4_Lx            , V(660F3A,5D,_,x,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 40 , 4093, 323),
  INST(Vfmaddsubps     , Fma4_Lx            , V(660F3A,5C,_,x,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 40 , 4105, 323),
  INST(Vfmsub132pd     , VexRvm_Lx          , V(660F38,9A,_,x,1,1,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 37 , 4117, 320),
  INST(Vfmsub132ps     , VexRvm_Lx          , V(660F38,9A,_,x,0,0,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 38 , 4129, 320),
  INST(Vfmsub132sd     , VexRvm             , V(660F38,9B,_,I,1,1,3,T1S), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 39 , 4141, 321),
  INST(Vfmsub132ss     , VexRvm             , V(660F38,9B,_,I,0,0,2,T1S), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 39 , 4153, 322),
  INST(Vfmsub213pd     , VexRvm_Lx          , V(660F38,AA,_,x,1,1,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 37 , 4165, 320),
  INST(Vfmsub213ps     , VexRvm_Lx          , V(660F38,AA,_,x,0,0,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 38 , 4177, 320),
  INST(Vfmsub213sd     , VexRvm             , V(660F38,AB,_,I,1,1,3,T1S), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 39 , 4189, 321),
  INST(Vfmsub213ss     , VexRvm             , V(660F38,AB,_,I,0,0,2,T1S), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 39 , 4201, 322),
  INST(Vfmsub231pd     , VexRvm_Lx          , V(660F38,BA,_,x,1,1,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 37 , 4213, 320),
  INST(Vfmsub231ps     , VexRvm_Lx          , V(660F38,BA,_,x,0,0,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 38 , 4225, 320),
  INST(Vfmsub231sd     , VexRvm             , V(660F38,BB,_,I,1,1,3,T1S), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 39 , 4237, 321),
  INST(Vfmsub231ss     , VexRvm             , V(660F38,BB,_,I,0,0,2,T1S), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 39 , 4249, 322),
  INST(Vfmsubadd132pd  , VexRvm_Lx          , V(660F38,97,_,x,1,1,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 37 , 4261, 320),
  INST(Vfmsubadd132ps  , VexRvm_Lx          , V(660F38,97,_,x,0,0,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 38 , 4276, 320),
  INST(Vfmsubadd213pd  , VexRvm_Lx          , V(660F38,A7,_,x,1,1,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 37 , 4291, 320),
  INST(Vfmsubadd213ps  , VexRvm_Lx          , V(660F38,A7,_,x,0,0,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 38 , 4306, 320),
  INST(Vfmsubadd231pd  , VexRvm_Lx          , V(660F38,B7,_,x,1,1,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 37 , 4321, 320),
  INST(Vfmsubadd231ps  , VexRvm_Lx          , V(660F38,B7,_,x,0,0,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 38 , 4336, 320),
  INST(Vfmsubaddpd     , Fma4_Lx            , V(660F3A,5F,_,x,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 40 , 4351, 323),
  INST(Vfmsubaddps     , Fma4_Lx            , V(660F3A,5E,_,x,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 40 , 4363, 323),
  INST(Vfmsubpd        , Fma4_Lx            , V(660F3A,6D,_,x,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 40 , 4375, 323),
  INST(Vfmsubps        , Fma4_Lx            , V(660F3A,6C,_,x,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 40 , 4384, 323),
  INST(Vfmsubsd        , Fma4               , V(660F3A,6F,_,0,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 40 , 4393, 324),
  INST(Vfmsubss        , Fma4               , V(660F3A,6E,_,0,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 40 , 4402, 325),
  INST(Vfnmadd132pd    , VexRvm_Lx          , V(660F38,9C,_,x,1,1,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 37 , 4411, 320),
  INST(Vfnmadd132ps    , VexRvm_Lx          , V(660F38,9C,_,x,0,0,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 38 , 4424, 320),
  INST(Vfnmadd132sd    , VexRvm             , V(660F38,9D,_,I,1,1,3,T1S), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 39 , 4437, 321),
  INST(Vfnmadd132ss    , VexRvm             , V(660F38,9D,_,I,0,0,2,T1S), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 39 , 4450, 322),
  INST(Vfnmadd213pd    , VexRvm_Lx          , V(660F38,AC,_,x,1,1,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 37 , 4463, 320),
  INST(Vfnmadd213ps    , VexRvm_Lx          , V(660F38,AC,_,x,0,0,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 38 , 4476, 320),
  INST(Vfnmadd213sd    , VexRvm             , V(660F38,AD,_,I,1,1,3,T1S), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 39 , 4489, 321),
  INST(Vfnmadd213ss    , VexRvm             , V(660F38,AD,_,I,0,0,2,T1S), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 39 , 4502, 322),
  INST(Vfnmadd231pd    , VexRvm_Lx          , V(660F38,BC,_,x,1,1,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 37 , 4515, 320),
  INST(Vfnmadd231ps    , VexRvm_Lx          , V(660F38,BC,_,x,0,0,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 38 , 4528, 320),
  INST(Vfnmadd231sd    , VexRvm             , V(660F38,BC,_,I,1,1,3,T1S), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 39 , 4541, 321),
  INST(Vfnmadd231ss    , VexRvm             , V(660F38,BC,_,I,0,0,2,T1S), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 39 , 4554, 322),
  INST(Vfnmaddpd       , Fma4_Lx            , V(660F3A,79,_,x,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 40 , 4567, 323),
  INST(Vfnmaddps       , Fma4_Lx            , V(660F3A,78,_,x,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 40 , 4577, 323),
  INST(Vfnmaddsd       , Fma4               , V(660F3A,7B,_,0,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 40 , 4587, 324),
  INST(Vfnmaddss       , Fma4               , V(660F3A,7A,_,0,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 40 , 4597, 325),
  INST(Vfnmsub132pd    , VexRvm_Lx          , V(660F38,9E,_,x,1,1,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 37 , 4607, 320),
  INST(Vfnmsub132ps    , VexRvm_Lx          , V(660F38,9E,_,x,0,0,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 38 , 4620, 320),
  INST(Vfnmsub132sd    , VexRvm             , V(660F38,9F,_,I,1,1,3,T1S), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 39 , 4633, 321),
  INST(Vfnmsub132ss    , VexRvm             , V(660F38,9F,_,I,0,0,2,T1S), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 39 , 4646, 322),
  INST(Vfnmsub213pd    , VexRvm_Lx          , V(660F38,AE,_,x,1,1,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 37 , 4659, 320),
  INST(Vfnmsub213ps    , VexRvm_Lx          , V(660F38,AE,_,x,0,0,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 38 , 4672, 320),
  INST(Vfnmsub213sd    , VexRvm             , V(660F38,AF,_,I,1,1,3,T1S), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 39 , 4685, 321),
  INST(Vfnmsub213ss    , VexRvm             , V(660F38,AF,_,I,0,0,2,T1S), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 39 , 4698, 322),
  INST(Vfnmsub231pd    , VexRvm_Lx          , V(660F38,BE,_,x,1,1,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 37 , 4711, 320),
  INST(Vfnmsub231ps    , VexRvm_Lx          , V(660F38,BE,_,x,0,0,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 38 , 4724, 320),
  INST(Vfnmsub231sd    , VexRvm             , V(660F38,BF,_,I,1,1,3,T1S), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 39 , 4737, 321),
  INST(Vfnmsub231ss    , VexRvm             , V(660F38,BF,_,I,0,0,2,T1S), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 39 , 4750, 322),
  INST(Vfnmsubpd       , Fma4_Lx            , V(660F3A,7D,_,x,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 40 , 4763, 323),
  INST(Vfnmsubps       , Fma4_Lx            , V(660F3A,7C,_,x,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 40 , 4773, 323),
  INST(Vfnmsubsd       , Fma4               , V(660F3A,7F,_,0,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 40 , 4783, 324),
  INST(Vfnmsubss       , Fma4               , V(660F3A,7E,_,0,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 40 , 4793, 325),
  INST(Vfpclasspd      , VexRmi_Lx          , V(660F3A,66,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 41 , 4803, 326),
  INST(Vfpclassps      , VexRmi_Lx          , V(660F3A,66,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 42 , 4814, 326),
  INST(Vfpclasssd      , VexRmi_Lx          , V(660F3A,67,_,I,_,1,3,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 43 , 4825, 327),
  INST(Vfpclassss      , VexRmi_Lx          , V(660F3A,67,_,I,_,0,2,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 43 , 4836, 328),
  INST(Vfrczpd         , VexRm_Lx           , V(XOP_M9,81,_,x,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 4847, 329),
  INST(Vfrczps         , VexRm_Lx           , V(XOP_M9,80,_,x,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 4855, 329),
  INST(Vfrczsd         , VexRm              , V(XOP_M9,83,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 4863, 330),
  INST(Vfrczss         , VexRm              , V(XOP_M9,82,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 4871, 331),
  INST(Vgatherdpd      , VexRmvRm_VM        , V(660F38,92,_,x,1,_,_,_  ), V(660F38,92,_,x,_,1,3,T1S), F(RW)|F(Vex_VM)|F(Evex)               , EF(________), 0 , 0 , kFamilyAvx , 45 , 4879, 332),
  INST(Vgatherdps      , VexRmvRm_VM        , V(660F38,92,_,x,0,_,_,_  ), V(660F38,92,_,x,_,0,2,T1S), F(RW)|F(Vex_VM)|F(Evex)               , EF(________), 0 , 0 , kFamilyAvx , 45 , 4890, 333),
  INST(Vgatherpf0dpd   , VexM_VM            , V(660F38,C6,1,2,_,1,3,T1S), 0                         , F(RO)|F(VM)|F(Evex)                   , EF(________), 0 , 0 , kFamilyNone, 0  , 4901, 334),
  INST(Vgatherpf0dps   , VexM_VM            , V(660F38,C6,1,2,_,0,2,T1S), 0                         , F(RO)|F(VM)|F(Evex)                   , EF(________), 0 , 0 , kFamilyNone, 0  , 4915, 335),
  INST(Vgatherpf0qpd   , VexM_VM            , V(660F38,C7,1,2,_,1,3,T1S), 0                         , F(RO)|F(VM)|F(Evex)                   , EF(________), 0 , 0 , kFamilyNone, 0  , 4929, 336),
  INST(Vgatherpf0qps   , VexM_VM            , V(660F38,C7,1,2,_,0,2,T1S), 0                         , F(RO)|F(VM)|F(Evex)                   , EF(________), 0 , 0 , kFamilyNone, 0  , 4943, 336),
  INST(Vgatherpf1dpd   , VexM_VM            , V(660F38,C6,2,2,_,1,3,T1S), 0                         , F(RO)|F(VM)|F(Evex)                   , EF(________), 0 , 0 , kFamilyNone, 0  , 4957, 334),
  INST(Vgatherpf1dps   , VexM_VM            , V(660F38,C6,2,2,_,0,2,T1S), 0                         , F(RO)|F(VM)|F(Evex)                   , EF(________), 0 , 0 , kFamilyNone, 0  , 4971, 335),
  INST(Vgatherpf1qpd   , VexM_VM            , V(660F38,C7,2,2,_,1,3,T1S), 0                         , F(RO)|F(VM)|F(Evex)                   , EF(________), 0 , 0 , kFamilyNone, 0  , 4985, 336),
  INST(Vgatherpf1qps   , VexM_VM            , V(660F38,C7,2,2,_,0,2,T1S), 0                         , F(RO)|F(VM)|F(Evex)                   , EF(________), 0 , 0 , kFamilyNone, 0  , 4999, 336),
  INST(Vgatherqpd      , VexRmvRm_VM        , V(660F38,93,_,x,1,_,_,_  ), V(660F38,93,_,x,_,1,3,T1S), F(RW)|F(Vex_VM)|F(Evex)               , EF(________), 0 , 0 , kFamilyAvx , 45 , 5013, 337),
  INST(Vgatherqps      , VexRmvRm_VM        , V(660F38,93,_,x,0,_,_,_  ), V(660F38,93,_,x,_,0,2,T1S), F(RW)|F(Vex_VM)|F(Evex)               , EF(________), 0 , 0 , kFamilyAvx , 45 , 5024, 338),
  INST(Vgetexppd       , VexRm_Lx           , V(660F38,42,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 28 , 5035, 302),
  INST(Vgetexpps       , VexRm_Lx           , V(660F38,42,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 31 , 5045, 302),
  INST(Vgetexpsd       , VexRm              , V(660F38,43,_,I,_,1,3,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 36 , 5055, 339),
  INST(Vgetexpss       , VexRm              , V(660F38,43,_,I,_,0,2,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 36 , 5065, 340),
  INST(Vgetmantpd      , VexRmi_Lx          , V(660F3A,26,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 28 , 5075, 341),
  INST(Vgetmantps      , VexRmi_Lx          , V(660F3A,26,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 31 , 5086, 341),
  INST(Vgetmantsd      , VexRmi             , V(660F3A,27,_,I,_,1,3,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 36 , 5097, 342),
  INST(Vgetmantss      , VexRmi             , V(660F3A,27,_,I,_,0,2,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 36 , 5108, 343),
  INST(Vhaddpd         , VexRvm_Lx          , V(660F00,7C,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 3  , 5119, 275),
  INST(Vhaddps         , VexRvm_Lx          , V(F20F00,7C,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 3  , 5127, 275),
  INST(Vhsubpd         , VexRvm_Lx          , V(660F00,7D,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 3  , 5135, 275),
  INST(Vhsubps         , VexRvm_Lx          , V(F20F00,7D,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 3  , 5143, 275),
  INST(Vinsertf128     , VexRvmi            , V(660F3A,18,_,1,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 3  , 5151, 344),
  INST(Vinsertf32x4    , VexRvmi_Lx         , V(660F3A,18,_,x,_,0,4,T4 ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 14 , 5163, 345),
  INST(Vinsertf32x8    , VexRvmi            , V(660F3A,1A,_,2,_,0,5,T8 ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 12 , 5176, 346),
  INST(Vinsertf64x2    , VexRvmi_Lx         , V(660F3A,18,_,x,_,1,4,T2 ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 10 , 5189, 345),
  INST(Vinsertf64x4    , VexRvmi            , V(660F3A,1A,_,2,_,1,5,T4 ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 11 , 5202, 346),
  INST(Vinserti128     , VexRvmi            , V(660F3A,38,_,1,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 13 , 5215, 344),
  INST(Vinserti32x4    , VexRvmi_Lx         , V(660F3A,38,_,x,_,0,4,T4 ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 14 , 5227, 345),
  INST(Vinserti32x8    , VexRvmi            , V(660F3A,3A,_,2,_,0,5,T8 ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 12 , 5240, 346),
  INST(Vinserti64x2    , VexRvmi_Lx         , V(660F3A,38,_,x,_,1,4,T2 ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 10 , 5253, 345),
  INST(Vinserti64x4    , VexRvmi            , V(660F3A,3A,_,2,_,1,5,T4 ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 11 , 5266, 346),
  INST(Vinsertps       , VexRvmi            , V(660F3A,21,_,0,I,0,2,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 35 , 5279, 347),
  INST(Vlddqu          , VexRm_Lx           , V(F20F00,F0,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 3  , 5289, 348),
  INST(Vldmxcsr        , VexM               , V(000F00,AE,2,0,I,_,_,_  ), 0                         , F(RO)|F(Vex)|F(Volatile)              , EF(________), 0 , 0 , kFamilyNone, 0  , 5296, 349),
  INST(Vmaskmovdqu     , VexRm_ZDI          , V(660F00,F7,_,0,I,_,_,_  ), 0                         , F(RO)|F(Vex)|F(Special)               , EF(________), 0 , 0 , kFamilyAvx , 3  , 5305, 350),
  INST(Vmaskmovpd      , VexRvmMvr_Lx       , V(660F38,2D,_,x,0,_,_,_  ), V(660F38,2F,_,x,0,_,_,_  ), F(RW)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 3  , 5317, 351),
  INST(Vmaskmovps      , VexRvmMvr_Lx       , V(660F38,2C,_,x,0,_,_,_  ), V(660F38,2E,_,x,0,_,_,_  ), F(RW)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 3  , 5328, 352),
  INST(Vmaxpd          , VexRvm_Lx          , V(660F00,5F,_,x,I,1,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 16 , 5339, 272),
  INST(Vmaxps          , VexRvm_Lx          , V(000F00,5F,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 17 , 5346, 272),
  INST(Vmaxsd          , VexRvm             , V(F20F00,5F,_,I,I,1,3,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 46 , 5353, 273),
  INST(Vmaxss          , VexRvm             , V(F30F00,5F,_,I,I,0,2,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 46 , 5360, 274),
  INST(Vminpd          , VexRvm_Lx          , V(660F00,5D,_,x,I,1,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 16 , 5367, 272),
  INST(Vminps          , VexRvm_Lx          , V(000F00,5D,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 17 , 5374, 272),
  INST(Vminsd          , VexRvm             , V(F20F00,5D,_,I,I,1,3,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 46 , 5381, 273),
  INST(Vminss          , VexRvm             , V(F30F00,5D,_,I,I,0,2,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 46 , 5388, 274),
  INST(Vmovapd         , VexRmMr_Lx         , V(660F00,28,_,x,I,1,4,FVM), V(660F00,29,_,x,I,1,4,FVM), F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 47 , 5395, 353),
  INST(Vmovaps         , VexRmMr_Lx         , V(000F00,28,_,x,I,0,4,FVM), V(000F00,29,_,x,I,0,4,FVM), F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 47 , 5403, 354),
  INST(Vmovd           , VexMovdMovq        , V(660F00,6E,_,0,0,0,2,T1S), V(660F00,7E,_,0,0,0,2,T1S), F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 35 , 5411, 355),
  INST(Vmovddup        , VexRm_Lx           , V(F20F00,12,_,x,I,1,3,DUP), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 47 , 5417, 356),
  INST(Vmovdqa         , VexRmMr_Lx         , V(660F00,6F,_,x,I,_,_,_  ), V(660F00,7F,_,x,I,_,_,_  ), F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 3  , 5426, 357),
  INST(Vmovdqa32       , VexRmMr_Lx         , V(660F00,6F,_,x,_,0,4,FVM), V(660F00,7F,_,x,_,0,4,FVM), F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 14 , 5434, 358),
  INST(Vmovdqa64       , VexRmMr_Lx         , V(660F00,6F,_,x,_,1,4,FVM), V(660F00,7F,_,x,_,1,4,FVM), F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 14 , 5444, 359),
  INST(Vmovdqu         , VexRmMr_Lx         , V(F30F00,6F,_,x,I,_,_,_  ), V(F30F00,7F,_,x,I,_,_,_  ), F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 3  , 5454, 360),
  INST(Vmovdqu16       , VexRmMr_Lx         , V(F20F00,6F,_,x,_,1,4,FVM), V(F20F00,7F,_,x,_,1,4,FVM), F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 9  , 5462, 361),
  INST(Vmovdqu32       , VexRmMr_Lx         , V(F30F00,6F,_,x,_,0,4,FVM), V(F30F00,7F,_,x,_,0,4,FVM), F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 14 , 5472, 362),
  INST(Vmovdqu64       , VexRmMr_Lx         , V(F30F00,6F,_,x,_,1,4,FVM), V(F30F00,7F,_,x,_,1,4,FVM), F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 14 , 5482, 363),
  INST(Vmovdqu8        , VexRmMr_Lx         , V(F20F00,6F,_,x,_,0,4,FVM), V(F20F00,7F,_,x,_,0,4,FVM), F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 9  , 5492, 364),
  INST(Vmovhlps        , VexRvm             , V(000F00,12,_,0,I,0,_,_  ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 35 , 5501, 365),
  INST(Vmovhpd         , VexRvmMr           , V(660F00,16,_,0,I,1,3,T1S), V(660F00,17,_,0,I,1,3,T1S), F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 35 , 5510, 366),
  INST(Vmovhps         , VexRvmMr           , V(000F00,16,_,0,I,0,3,T2 ), V(000F00,17,_,0,I,0,3,T2 ), F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 35 , 5518, 367),
  INST(Vmovlhps        , VexRvm             , V(000F00,16,_,0,I,0,_,_  ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 35 , 5526, 365),
  INST(Vmovlpd         , VexRvmMr           , V(660F00,12,_,0,I,1,3,T1S), V(660F00,13,_,0,I,1,3,T1S), F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 35 , 5535, 368),
  INST(Vmovlps         , VexRvmMr           , V(000F00,12,_,0,I,0,3,T2 ), V(000F00,13,_,0,I,0,3,T2 ), F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 35 , 5543, 369),
  INST(Vmovmskpd       , VexRm_Lx           , V(660F00,50,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 3  , 5551, 370),
  INST(Vmovmskps       , VexRm_Lx           , V(000F00,50,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 3  , 5561, 370),
  INST(Vmovntdq        , VexMr_Lx           , V(660F00,E7,_,x,I,0,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 48 , 5571, 371),
  INST(Vmovntdqa       , VexRm_Lx           , V(660F38,2A,_,x,I,0,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 49 , 5580, 372),
  INST(Vmovntpd        , VexMr_Lx           , V(660F00,2B,_,x,I,1,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 48 , 5590, 371),
  INST(Vmovntps        , VexMr_Lx           , V(000F00,2B,_,x,I,0,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 48 , 5599, 371),
  INST(Vmovq           , VexMovdMovq        , V(660F00,6E,_,0,I,1,3,T1S), V(660F00,7E,_,0,I,1,3,T1S), F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 35 , 5608, 373),
  INST(Vmovsd          , VexMovssMovsd      , V(F20F00,10,_,I,I,1,3,T1S), V(F20F00,11,_,I,I,1,3,T1S), F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 50 , 5614, 374),
  INST(Vmovshdup       , VexRm_Lx           , V(F30F00,16,_,x,I,0,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 47 , 5621, 299),
  INST(Vmovsldup       , VexRm_Lx           , V(F30F00,12,_,x,I,0,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 47 , 5631, 299),
  INST(Vmovss          , VexMovssMovsd      , V(F30F00,10,_,I,I,0,2,T1S), V(F30F00,11,_,I,I,0,2,T1S), F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 50 , 5641, 375),
  INST(Vmovupd         , VexRmMr_Lx         , V(660F00,10,_,x,I,1,4,FVM), V(660F00,11,_,x,I,1,4,FVM), F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 47 , 5648, 376),
  INST(Vmovups         , VexRmMr_Lx         , V(000F00,10,_,x,I,0,4,FVM), V(000F00,11,_,x,I,0,4,FVM), F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 47 , 5656, 377),
  INST(Vmpsadbw        , VexRvmi_Lx         , V(660F3A,42,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 51 , 5664, 282),
  INST(Vmulpd          , VexRvm_Lx          , V(660F00,59,_,x,I,1,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 52 , 5673, 272),
  INST(Vmulps          , VexRvm_Lx          , V(000F00,59,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 53 , 5680, 272),
  INST(Vmulsd          , VexRvm_Lx          , V(F20F00,59,_,I,I,1,3,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 2  , 5687, 273),
  INST(Vmulss          , VexRvm_Lx          , V(F30F00,59,_,I,I,0,2,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 2  , 5694, 274),
  INST(Vorpd           , VexRvm_Lx          , V(660F00,56,_,x,I,1,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 7  , 5701, 280),
  INST(Vorps           , VexRvm_Lx          , V(000F00,56,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 20 , 5707, 280),
  INST(Vpabsb          , VexRm_Lx           , V(660F38,1C,_,x,I,_,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 54 , 5713, 299),
  INST(Vpabsd          , VexRm_Lx           , V(660F38,1E,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 15 , 5720, 299),
  INST(Vpabsq          , VexRm_Lx           , V(660F38,1F,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 14 , 5727, 302),
  INST(Vpabsw          , VexRm_Lx           , V(660F38,1D,_,x,I,_,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 54 , 5734, 299),
  INST(Vpackssdw       , VexRvm_Lx          , V(660F00,6B,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 55 , 5741, 272),
  INST(Vpacksswb       , VexRvm_Lx          , V(660F00,63,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 54 , 5751, 272),
  INST(Vpackusdw       , VexRvm_Lx          , V(660F38,2B,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 55 , 5761, 272),
  INST(Vpackuswb       , VexRvm_Lx          , V(660F00,67,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 54 , 5771, 272),
  INST(Vpaddb          , VexRvm_Lx          , V(660F00,FC,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 54 , 5781, 272),
  INST(Vpaddd          , VexRvm_Lx          , V(660F00,FE,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 56 , 5788, 272),
  INST(Vpaddq          , VexRvm_Lx          , V(660F00,D4,_,x,I,1,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 57 , 5795, 272),
  INST(Vpaddsb         , VexRvm_Lx          , V(660F00,EC,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 54 , 5802, 272),
  INST(Vpaddsw         , VexRvm_Lx          , V(660F00,ED,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 54 , 5810, 272),
  INST(Vpaddusb        , VexRvm_Lx          , V(660F00,DC,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 54 , 5818, 272),
  INST(Vpaddusw        , VexRvm_Lx          , V(660F00,DD,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 54 , 5827, 272),
  INST(Vpaddw          , VexRvm_Lx          , V(660F00,FD,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 54 , 5836, 272),
  INST(Vpalignr        , VexRvmi_Lx         , V(660F3A,0F,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 54 , 5843, 378),
  INST(Vpand           , VexRvm_Lx          , V(660F00,DB,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 51 , 5852, 379),
  INST(Vpandd          , VexRvm_Lx          , V(660F00,DB,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 5  , 5858, 380),
  INST(Vpandn          , VexRvm_Lx          , V(660F00,DF,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 51 , 5865, 381),
  INST(Vpandnd         , VexRvm_Lx          , V(660F00,DF,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 5  , 5872, 382),
  INST(Vpandnq         , VexRvm_Lx          , V(660F00,DF,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 6  , 5880, 382),
  INST(Vpandq          , VexRvm_Lx          , V(660F00,DB,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 6  , 5888, 380),
  INST(Vpavgb          , VexRvm_Lx          , V(660F00,E0,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 58 , 5895, 272),
  INST(Vpavgw          , VexRvm_Lx          , V(660F00,E3,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 59 , 5902, 272),
  INST(Vpblendd        , VexRvmi_Lx         , V(660F3A,02,_,x,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 13 , 5909, 282),
  INST(Vpblendvb       , VexRvmr            , V(660F3A,4C,_,x,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 51 , 5918, 283),
  INST(Vpblendw        , VexRvmi_Lx         , V(660F3A,0E,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 51 , 5928, 282),
  INST(Vpbroadcastb    , VexRm_Lx           , V(660F38,78,_,x,0,0,0,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 59 , 5937, 383),
  INST(Vpbroadcastd    , VexRm_Lx           , V(660F38,58,_,x,0,0,2,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 60 , 5950, 384),
  INST(Vpbroadcastmb2d , VexRm_Lx           , V(F30F38,3A,_,x,_,0,_,_  ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 61 , 5963, 385),
  INST(Vpbroadcastmb2q , VexRm_Lx           , V(F30F38,2A,_,x,_,1,_,_  ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 61 , 5979, 385),
  INST(Vpbroadcastq    , VexRm_Lx           , V(660F38,59,_,x,0,1,3,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 60 , 5995, 386),
  INST(Vpbroadcastw    , VexRm_Lx           , V(660F38,79,_,x,0,0,1,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 59 , 6008, 387),
  INST(Vpclmulqdq      , VexRvmi            , V(660F3A,44,_,0,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 62 , 6021, 388),
  INST(Vpcmov          , VexRvrmRvmr_Lx     , V(XOP_M8,A2,_,x,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 6032, 323),
  INST(Vpcmpb          , VexRvm_Lx          , V(660F3A,3F,_,x,_,0,4,FVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 63 , 6039, 389),
  INST(Vpcmpd          , VexRvm_Lx          , V(660F3A,1F,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 64 , 6046, 389),
  INST(Vpcmpeqb        , VexRvm_Lx          , V(660F00,74,_,x,I,I,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 65 , 6053, 390),
  INST(Vpcmpeqd        , VexRvm_Lx          , V(660F00,76,_,x,I,0,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 66 , 6062, 390),
  INST(Vpcmpeqq        , VexRvm_Lx          , V(660F38,29,_,x,I,1,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 67 , 6071, 390),
  INST(Vpcmpeqw        , VexRvm_Lx          , V(660F00,75,_,x,I,I,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 65 , 6080, 390),
  INST(Vpcmpestri      , VexRmi             , V(660F3A,61,_,0,I,_,_,_  ), 0                         , F(WO)|F(Vex)|F(Special)               , EF(________), 0 , 0 , kFamilyAvx , 3  , 6089, 391),
  INST(Vpcmpestrm      , VexRmi             , V(660F3A,60,_,0,I,_,_,_  ), 0                         , F(WO)|F(Vex)|F(Special)               , EF(________), 0 , 0 , kFamilyAvx , 3  , 6100, 392),
  INST(Vpcmpgtb        , VexRvm_Lx          , V(660F00,64,_,x,I,I,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 65 , 6111, 390),
  INST(Vpcmpgtd        , VexRvm_Lx          , V(660F00,66,_,x,I,0,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 66 , 6120, 390),
  INST(Vpcmpgtq        , VexRvm_Lx          , V(660F38,37,_,x,I,1,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 67 , 6129, 390),
  INST(Vpcmpgtw        , VexRvm_Lx          , V(660F00,65,_,x,I,I,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 65 , 6138, 390),
  INST(Vpcmpistri      , VexRmi             , V(660F3A,63,_,0,I,_,_,_  ), 0                         , F(WO)|F(Vex)|F(Special)               , EF(________), 0 , 0 , kFamilyAvx , 3  , 6147, 393),
  INST(Vpcmpistrm      , VexRmi             , V(660F3A,62,_,0,I,_,_,_  ), 0                         , F(WO)|F(Vex)|F(Special)               , EF(________), 0 , 0 , kFamilyAvx , 3  , 6158, 394),
  INST(Vpcmpq          , VexRvm_Lx          , V(660F3A,1F,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 68 , 6169, 389),
  INST(Vpcmpub         , VexRvm_Lx          , V(660F3A,3E,_,x,_,0,4,FVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 63 , 6176, 389),
  INST(Vpcmpud         , VexRvm_Lx          , V(660F3A,1E,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 64 , 6184, 389),
  INST(Vpcmpuq         , VexRvm_Lx          , V(660F3A,1E,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 68 , 6192, 389),
  INST(Vpcmpuw         , VexRvm_Lx          , V(660F3A,3E,_,x,_,1,4,FVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 69 , 6200, 389),
  INST(Vpcmpw          , VexRvm_Lx          , V(660F3A,3F,_,x,_,1,4,FVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 69 , 6208, 389),
  INST(Vpcomb          , VexRvmi            , V(XOP_M8,CC,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 6215, 388),
  INST(Vpcomd          , VexRvmi            , V(XOP_M8,CE,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 6222, 388),
  INST(Vpcompressd     , VexMr_Lx           , V(660F38,8B,_,x,_,0,2,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 14 , 6229, 297),
  INST(Vpcompressq     , VexMr_Lx           , V(660F38,8B,_,x,_,1,3,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 14 , 6241, 297),
  INST(Vpcomq          , VexRvmi            , V(XOP_M8,CF,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 6253, 388),
  INST(Vpcomub         , VexRvmi            , V(XOP_M8,EC,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 6260, 388),
  INST(Vpcomud         , VexRvmi            , V(XOP_M8,EE,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 6268, 388),
  INST(Vpcomuq         , VexRvmi            , V(XOP_M8,EF,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 6276, 388),
  INST(Vpcomuw         , VexRvmi            , V(XOP_M8,ED,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 6284, 388),
  INST(Vpcomw          , VexRvmi            , V(XOP_M8,CD,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 6292, 388),
  INST(Vpconflictd     , VexRm_Lx           , V(660F38,C4,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 70 , 6299, 302),
  INST(Vpconflictq     , VexRm_Lx           , V(660F38,C4,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 70 , 6311, 302),
  INST(Vperm2f128      , VexRvmi            , V(660F3A,06,_,1,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 3  , 6323, 395),
  INST(Vperm2i128      , VexRvmi            , V(660F3A,46,_,1,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 13 , 6334, 395),
  INST(Vpermb          , VexRvm_Lx          , V(660F38,8D,_,x,_,0,4,FVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 71 , 6345, 281),
  INST(Vpermd          , VexRvm_Lx          , V(660F38,36,_,x,0,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 72 , 6352, 396),
  INST(Vpermi2b        , VexRvm_Lx          , V(660F38,75,_,x,_,0,4,FVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 71 , 6359, 281),
  INST(Vpermi2d        , VexRvm_Lx          , V(660F38,76,_,x,_,0,4,FV ), 0                         , F(RW)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 5  , 6368, 397),
  INST(Vpermi2pd       , VexRvm_Lx          , V(660F38,77,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 6  , 6377, 281),
  INST(Vpermi2ps       , VexRvm_Lx          , V(660F38,77,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 5  , 6387, 281),
  INST(Vpermi2q        , VexRvm_Lx          , V(660F38,76,_,x,_,1,4,FV ), 0                         , F(RW)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 6  , 6397, 397),
  INST(Vpermi2w        , VexRvm_Lx          , V(660F38,75,_,x,_,1,4,FVM), 0                         , F(RW)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 9  , 6406, 397),
  INST(Vpermil2pd      , VexRvrmiRvmri_Lx   , V(660F3A,49,_,x,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 6415, 398),
  INST(Vpermil2ps      , VexRvrmiRvmri_Lx   , V(660F3A,48,_,x,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 6426, 398),
  INST(Vpermilpd       , VexRvmRmi_Lx       , V(660F38,0D,_,x,0,1,4,FV ), V(660F3A,05,_,x,0,1,4,FV ), F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 73 , 6437, 399),
  INST(Vpermilps       , VexRvmRmi_Lx       , V(660F38,0C,_,x,0,0,4,FV ), V(660F3A,04,_,x,0,0,4,FV ), F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 73 , 6447, 400),
  INST(Vpermpd         , VexRmi             , V(660F3A,01,_,1,1,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 13 , 6457, 401),
  INST(Vpermps         , VexRvm             , V(660F38,16,_,1,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 13 , 6465, 402),
  INST(Vpermq          , VexRvmRmi_Lx       , V(660F38,36,_,x,_,1,4,FV ), V(660F3A,00,_,x,1,1,4,FV ), F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 74 , 6473, 403),
  INST(Vpermt2b        , VexRvm_Lx          , V(660F38,7D,_,x,_,0,4,FVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 71 , 6480, 281),
  INST(Vpermt2d        , VexRvm_Lx          , V(660F38,7E,_,x,_,0,4,FV ), 0                         , F(RW)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 5  , 6489, 397),
  INST(Vpermt2pd       , VexRvm_Lx          , V(660F38,7F,_,x,_,1,4,FV ), 0                         , F(RW)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 6  , 6498, 397),
  INST(Vpermt2ps       , VexRvm_Lx          , V(660F38,7F,_,x,_,0,4,FV ), 0                         , F(RW)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 5  , 6508, 397),
  INST(Vpermt2q        , VexRvm_Lx          , V(660F38,7E,_,x,_,1,4,FV ), 0                         , F(RW)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 6  , 6518, 397),
  INST(Vpermt2w        , VexRvm_Lx          , V(660F38,7D,_,x,_,1,4,FVM), 0                         , F(RW)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 9  , 6527, 397),
  INST(Vpermw          , VexRvm_Lx          , V(660F38,8D,_,x,_,1,4,FVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 9  , 6536, 281),
  INST(Vpexpandd       , VexRm_Lx           , V(660F38,89,_,x,_,0,2,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 14 , 6543, 302),
  INST(Vpexpandq       , VexRm_Lx           , V(660F38,89,_,x,_,1,3,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 14 , 6553, 302),
  INST(Vpextrb         , VexMri             , V(660F3A,14,_,0,0,I,0,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 75 , 6563, 404),
  INST(Vpextrd         , VexMri             , V(660F3A,16,_,0,0,0,2,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 76 , 6571, 316),
  INST(Vpextrq         , VexMri             , V(660F3A,16,_,0,1,1,3,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 76 , 6579, 405),
  INST(Vpextrw         , VexMri             , V(660F3A,15,_,0,0,I,1,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 75 , 6587, 406),
  INST(Vpgatherdd      , VexRmvRm_VM        , V(660F38,90,_,x,0,_,_,_  ), V(660F38,90,_,x,_,0,2,T1S), F(RW)|F(Vex_VM)|F(Evex)               , EF(________), 0 , 0 , kFamilyAvx , 45 , 6595, 407),
  INST(Vpgatherdq      , VexRmvRm_VM        , V(660F38,90,_,x,1,_,_,_  ), V(660F38,90,_,x,_,1,3,T1S), F(RW)|F(Vex_VM)|F(Evex)               , EF(________), 0 , 0 , kFamilyAvx , 45 , 6606, 408),
  INST(Vpgatherqd      , VexRmvRm_VM        , V(660F38,91,_,x,0,_,_,_  ), V(660F38,91,_,x,_,0,2,T1S), F(RW)|F(Vex_VM)|F(Evex)               , EF(________), 0 , 0 , kFamilyAvx , 45 , 6617, 409),
  INST(Vpgatherqq      , VexRmvRm_VM        , V(660F38,91,_,x,1,_,_,_  ), V(660F38,91,_,x,_,1,3,T1S), F(RW)|F(Vex_VM)|F(Evex)               , EF(________), 0 , 0 , kFamilyAvx , 45 , 6628, 410),
  INST(Vphaddbd        , VexRm              , V(XOP_M9,C2,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 6639, 277),
  INST(Vphaddbq        , VexRm              , V(XOP_M9,C3,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 6648, 277),
  INST(Vphaddbw        , VexRm              , V(XOP_M9,C1,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 6657, 277),
  INST(Vphaddd         , VexRvm_Lx          , V(660F38,02,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 51 , 6666, 275),
  INST(Vphadddq        , VexRm              , V(XOP_M9,CB,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 6674, 277),
  INST(Vphaddsw        , VexRvm_Lx          , V(660F38,03,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 51 , 6683, 275),
  INST(Vphaddubd       , VexRm              , V(XOP_M9,D2,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 6692, 277),
  INST(Vphaddubq       , VexRm              , V(XOP_M9,D3,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 6702, 277),
  INST(Vphaddubw       , VexRm              , V(XOP_M9,D1,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 6712, 277),
  INST(Vphaddudq       , VexRm              , V(XOP_M9,DB,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 6722, 277),
  INST(Vphadduwd       , VexRm              , V(XOP_M9,D6,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 6732, 277),
  INST(Vphadduwq       , VexRm              , V(XOP_M9,D7,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 6742, 277),
  INST(Vphaddw         , VexRvm_Lx          , V(660F38,01,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 51 , 6752, 275),
  INST(Vphaddwd        , VexRm              , V(XOP_M9,C6,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 6760, 277),
  INST(Vphaddwq        , VexRm              , V(XOP_M9,C7,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 6769, 277),
  INST(Vphminposuw     , VexRm              , V(660F38,41,_,0,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 3  , 6778, 277),
  INST(Vphsubbw        , VexRm              , V(XOP_M9,E1,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 6790, 277),
  INST(Vphsubd         , VexRvm_Lx          , V(660F38,06,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 51 , 6799, 275),
  INST(Vphsubdq        , VexRm              , V(XOP_M9,E3,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 6807, 277),
  INST(Vphsubsw        , VexRvm_Lx          , V(660F38,07,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 51 , 6816, 275),
  INST(Vphsubw         , VexRvm_Lx          , V(660F38,05,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 51 , 6825, 275),
  INST(Vphsubwd        , VexRm              , V(XOP_M9,E2,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 6833, 277),
  INST(Vpinsrb         , VexRvmi            , V(660F3A,20,_,0,0,I,0,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 77 , 6842, 411),
  INST(Vpinsrd         , VexRvmi            , V(660F3A,22,_,0,0,0,2,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 78 , 6850, 412),
  INST(Vpinsrq         , VexRvmi            , V(660F3A,22,_,0,1,1,3,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 78 , 6858, 413),
  INST(Vpinsrw         , VexRvmi            , V(660F00,C4,_,0,0,I,1,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 77 , 6866, 414),
  INST(Vplzcntd        , VexRm_Lx           , V(660F38,44,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 70 , 6874, 302),
  INST(Vplzcntq        , VexRm_Lx           , V(660F38,44,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 79 , 6883, 302),
  INST(Vpmacsdd        , VexRvmr            , V(XOP_M8,9E,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 6892, 415),
  INST(Vpmacsdqh       , VexRvmr            , V(XOP_M8,9F,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 6901, 415),
  INST(Vpmacsdql       , VexRvmr            , V(XOP_M8,97,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 6911, 415),
  INST(Vpmacssdd       , VexRvmr            , V(XOP_M8,8E,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 6921, 415),
  INST(Vpmacssdqh      , VexRvmr            , V(XOP_M8,8F,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 6931, 415),
  INST(Vpmacssdql      , VexRvmr            , V(XOP_M8,87,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 6942, 415),
  INST(Vpmacsswd       , VexRvmr            , V(XOP_M8,86,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 6953, 415),
  INST(Vpmacssww       , VexRvmr            , V(XOP_M8,85,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 6963, 415),
  INST(Vpmacswd        , VexRvmr            , V(XOP_M8,96,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 6973, 415),
  INST(Vpmacsww        , VexRvmr            , V(XOP_M8,95,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 6982, 415),
  INST(Vpmadcsswd      , VexRvmr            , V(XOP_M8,A6,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 6991, 415),
  INST(Vpmadcswd       , VexRvmr            , V(XOP_M8,B6,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 7002, 415),
  INST(Vpmadd52huq     , VexRvm_Lx          , V(660F38,B5,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 80 , 7012, 281),
  INST(Vpmadd52luq     , VexRvm_Lx          , V(660F38,B4,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 80 , 7024, 281),
  INST(Vpmaddubsw      , VexRvm_Lx          , V(660F38,04,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 54 , 7036, 272),
  INST(Vpmaddwd        , VexRvm_Lx          , V(660F00,F5,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 54 , 7047, 272),
  INST(Vpmaskmovd      , VexRvmMvr_Lx       , V(660F38,8C,_,x,0,_,_,_  ), V(660F38,8E,_,x,0,_,_,_  ), F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 13 , 7056, 416),
  INST(Vpmaskmovq      , VexRvmMvr_Lx       , V(660F38,8C,_,x,1,_,_,_  ), V(660F38,8E,_,x,1,_,_,_  ), F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 13 , 7067, 417),
  INST(Vpmaxsb         , VexRvm_Lx          , V(660F38,3C,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 54 , 7078, 280),
  INST(Vpmaxsd         , VexRvm_Lx          , V(660F38,3D,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 56 , 7086, 280),
  INST(Vpmaxsq         , VexRvm_Lx          , V(660F38,3D,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 6  , 7094, 281),
  INST(Vpmaxsw         , VexRvm_Lx          , V(660F00,EE,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 54 , 7102, 280),
  INST(Vpmaxub         , VexRvm_Lx          , V(660F00,DE,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 54 , 7110, 280),
  INST(Vpmaxud         , VexRvm_Lx          , V(660F38,3F,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 56 , 7118, 280),
  INST(Vpmaxuq         , VexRvm_Lx          , V(660F38,3F,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 6  , 7126, 281),
  INST(Vpmaxuw         , VexRvm_Lx          , V(660F38,3E,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 54 , 7134, 280),
  INST(Vpminsb         , VexRvm_Lx          , V(660F38,38,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 54 , 7142, 280),
  INST(Vpminsd         , VexRvm_Lx          , V(660F38,39,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 56 , 7150, 280),
  INST(Vpminsq         , VexRvm_Lx          , V(660F38,39,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 6  , 7158, 281),
  INST(Vpminsw         , VexRvm_Lx          , V(660F00,EA,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 54 , 7166, 280),
  INST(Vpminub         , VexRvm_Lx          , V(660F00,DA,_,x,I,_,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 54 , 7174, 280),
  INST(Vpminud         , VexRvm_Lx          , V(660F38,3B,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 56 , 7182, 280),
  INST(Vpminuq         , VexRvm_Lx          , V(660F38,3B,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 6  , 7190, 281),
  INST(Vpminuw         , VexRvm_Lx          , V(660F38,3A,_,x,I,_,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 54 , 7198, 280),
  INST(Vpmovb2m        , VexRm_Lx           , V(F30F38,29,_,x,_,0,_,_  ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 81 , 7206, 418),
  INST(Vpmovd2m        , VexRm_Lx           , V(F30F38,39,_,x,_,0,_,_  ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 82 , 7215, 418),
  INST(Vpmovdb         , VexMr_Lx           , V(F30F38,31,_,x,_,0,2,QVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 14 , 7224, 419),
  INST(Vpmovdw         , VexMr_Lx           , V(F30F38,33,_,x,_,0,3,HVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 14 , 7232, 420),
  INST(Vpmovm2b        , VexRm_Lx           , V(F30F38,28,_,x,_,0,_,_  ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 81 , 7240, 385),
  INST(Vpmovm2d        , VexRm_Lx           , V(F30F38,38,_,x,_,0,_,_  ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 82 , 7249, 385),
  INST(Vpmovm2q        , VexRm_Lx           , V(F30F38,38,_,x,_,1,_,_  ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 82 , 7258, 385),
  INST(Vpmovm2w        , VexRm_Lx           , V(F30F38,28,_,x,_,1,_,_  ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 81 , 7267, 385),
  INST(Vpmovmskb       , VexRm_Lx           , V(660F00,D7,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 51 , 7276, 370),
  INST(Vpmovq2m        , VexRm_Lx           , V(F30F38,39,_,x,_,1,_,_  ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 82 , 7286, 418),
  INST(Vpmovqb         , VexMr_Lx           , V(F30F38,32,_,x,_,0,1,OVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 14 , 7295, 421),
  INST(Vpmovqd         , VexMr_Lx           , V(F30F38,35,_,x,_,0,3,HVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 14 , 7303, 420),
  INST(Vpmovqw         , VexMr_Lx           , V(F30F38,34,_,x,_,0,2,QVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 14 , 7311, 419),
  INST(Vpmovsdb        , VexMr_Lx           , V(F30F38,21,_,x,_,0,2,QVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 14 , 7319, 419),
  INST(Vpmovsdw        , VexMr_Lx           , V(F30F38,23,_,x,_,0,3,HVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 14 , 7328, 420),
  INST(Vpmovsqb        , VexMr_Lx           , V(F30F38,22,_,x,_,0,1,OVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 14 , 7337, 421),
  INST(Vpmovsqd        , VexMr_Lx           , V(F30F38,25,_,x,_,0,3,HVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 14 , 7346, 420),
  INST(Vpmovsqw        , VexMr_Lx           , V(F30F38,24,_,x,_,0,2,QVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 14 , 7355, 419),
  INST(Vpmovswb        , VexMr_Lx           , V(F30F38,20,_,x,_,0,3,HVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 9  , 7364, 420),
  INST(Vpmovsxbd       , VexRm_Lx           , V(660F38,21,_,x,I,I,2,QVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 15 , 7373, 422),
  INST(Vpmovsxbq       , VexRm_Lx           , V(660F38,22,_,x,I,I,1,OVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 15 , 7383, 423),
  INST(Vpmovsxbw       , VexRm_Lx           , V(660F38,20,_,x,I,I,3,HVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 54 , 7393, 298),
  INST(Vpmovsxdq       , VexRm_Lx           , V(660F38,25,_,x,I,0,3,HVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 15 , 7403, 424),
  INST(Vpmovsxwd       , VexRm_Lx           , V(660F38,23,_,x,I,I,3,HVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 15 , 7413, 298),
  INST(Vpmovsxwq       , VexRm_Lx           , V(660F38,24,_,x,I,I,2,QVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 15 , 7423, 422),
  INST(Vpmovusdb       , VexMr_Lx           , V(F30F38,11,_,x,_,0,2,QVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 14 , 7433, 419),
  INST(Vpmovusdw       , VexMr_Lx           , V(F30F38,13,_,x,_,0,3,HVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 14 , 7443, 420),
  INST(Vpmovusqb       , VexMr_Lx           , V(F30F38,12,_,x,_,0,1,OVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 14 , 7453, 421),
  INST(Vpmovusqd       , VexMr_Lx           , V(F30F38,15,_,x,_,0,3,HVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 14 , 7463, 420),
  INST(Vpmovusqw       , VexMr_Lx           , V(F30F38,14,_,x,_,0,2,QVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 14 , 7473, 419),
  INST(Vpmovuswb       , VexMr_Lx           , V(F30F38,10,_,x,_,0,3,HVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 9  , 7483, 420),
  INST(Vpmovw2m        , VexRm_Lx           , V(F30F38,29,_,x,_,1,_,_  ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 81 , 7493, 418),
  INST(Vpmovwb         , VexMr_Lx           , V(F30F38,30,_,x,_,0,3,HVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 9  , 7502, 420),
  INST(Vpmovzxbd       , VexRm_Lx           , V(660F38,31,_,x,I,I,2,QVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 15 , 7510, 422),
  INST(Vpmovzxbq       , VexRm_Lx           , V(660F38,32,_,x,I,I,1,OVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 15 , 7520, 423),
  INST(Vpmovzxbw       , VexRm_Lx           , V(660F38,30,_,x,I,I,3,HVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 54 , 7530, 298),
  INST(Vpmovzxdq       , VexRm_Lx           , V(660F38,35,_,x,I,0,3,HVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 15 , 7540, 424),
  INST(Vpmovzxwd       , VexRm_Lx           , V(660F38,33,_,x,I,I,3,HVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 15 , 7550, 298),
  INST(Vpmovzxwq       , VexRm_Lx           , V(660F38,34,_,x,I,I,2,QVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 15 , 7560, 422),
  INST(Vpmuldq         , VexRvm_Lx          , V(660F38,28,_,x,I,1,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 57 , 7570, 272),
  INST(Vpmulhrsw       , VexRvm_Lx          , V(660F38,0B,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 54 , 7578, 272),
  INST(Vpmulhuw        , VexRvm_Lx          , V(660F00,E4,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 54 , 7588, 272),
  INST(Vpmulhw         , VexRvm_Lx          , V(660F00,E5,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 54 , 7597, 272),
  INST(Vpmulld         , VexRvm_Lx          , V(660F38,40,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 56 , 7605, 272),
  INST(Vpmullq         , VexRvm_Lx          , V(660F38,40,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 83 , 7613, 281),
  INST(Vpmullw         , VexRvm_Lx          , V(660F00,D5,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 54 , 7621, 272),
  INST(Vpmultishiftqb  , VexRvm_Lx          , V(660F38,83,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 84 , 7629, 281),
  INST(Vpmuludq        , VexRvm_Lx          , V(660F00,F4,_,x,I,1,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 57 , 7644, 272),
  INST(Vpor            , VexRvm_Lx          , V(660F00,EB,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 51 , 7653, 379),
  INST(Vpord           , VexRvm_Lx          , V(660F00,EB,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 5  , 7658, 380),
  INST(Vporq           , VexRvm_Lx          , V(660F00,EB,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 6  , 7664, 380),
  INST(Vpperm          , VexRvrmRvmr        , V(XOP_M8,A3,_,0,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 7670, 425),
  INST(Vprold          , VexVmi_Lx          , V(660F00,72,1,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 5  , 7677, 341),
  INST(Vprolq          , VexVmi_Lx          , V(660F00,72,1,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 6  , 7684, 341),
  INST(Vprolvd         , VexRvm_Lx          , V(660F38,15,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 5  , 7691, 281),
  INST(Vprolvq         , VexRvm_Lx          , V(660F38,15,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 6  , 7699, 281),
  INST(Vprord          , VexVmi_Lx          , V(660F00,72,0,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 5  , 7707, 341),
  INST(Vprorq          , VexVmi_Lx          , V(660F00,72,0,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 6  , 7714, 341),
  INST(Vprorvd         , VexRvm_Lx          , V(660F38,14,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 5  , 7721, 281),
  INST(Vprorvq         , VexRvm_Lx          , V(660F38,14,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 6  , 7729, 281),
  INST(Vprotb          , VexRvmRmvRmi       , V(XOP_M9,90,_,0,x,_,_,_  ), V(XOP_M8,C0,_,0,x,_,_,_  ), F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 7737, 426),
  INST(Vprotd          , VexRvmRmvRmi       , V(XOP_M9,92,_,0,x,_,_,_  ), V(XOP_M8,C2,_,0,x,_,_,_  ), F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 7744, 427),
  INST(Vprotq          , VexRvmRmvRmi       , V(XOP_M9,93,_,0,x,_,_,_  ), V(XOP_M8,C3,_,0,x,_,_,_  ), F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 7751, 428),
  INST(Vprotw          , VexRvmRmvRmi       , V(XOP_M9,91,_,0,x,_,_,_  ), V(XOP_M8,C1,_,0,x,_,_,_  ), F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 7758, 429),
  INST(Vpsadbw         , VexRvm_Lx          , V(660F00,F6,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 85 , 7765, 272),
  INST(Vpscatterdd     , VexMr_VM           , V(660F38,A0,_,x,_,0,2,T1S), 0                         , F(WO)|F(VM)|F(Evex)                   , EF(________), 0 , 0 , kFamilyAvx , 86 , 7773, 430),
  INST(Vpscatterdq     , VexMr_VM           , V(660F38,A0,_,x,_,1,3,T1S), 0                         , F(WO)|F(VM)|F(Evex)                   , EF(________), 0 , 0 , kFamilyAvx , 86 , 7785, 430),
  INST(Vpscatterqd     , VexMr_VM           , V(660F38,A1,_,x,_,0,2,T1S), 0                         , F(WO)|F(VM)|F(Evex)                   , EF(________), 0 , 0 , kFamilyAvx , 86 , 7797, 431),
  INST(Vpscatterqq     , VexMr_VM           , V(660F38,A1,_,x,_,1,3,T1S), 0                         , F(WO)|F(VM)|F(Evex)                   , EF(________), 0 , 0 , kFamilyAvx , 86 , 7809, 432),
  INST(Vpshab          , VexRvmRmv          , V(XOP_M9,98,_,0,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 7821, 433),
  INST(Vpshad          , VexRvmRmv          , V(XOP_M9,9A,_,0,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 7828, 433),
  INST(Vpshaq          , VexRvmRmv          , V(XOP_M9,9B,_,0,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 7835, 433),
  INST(Vpshaw          , VexRvmRmv          , V(XOP_M9,99,_,0,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 7842, 433),
  INST(Vpshlb          , VexRvmRmv          , V(XOP_M9,94,_,0,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 7849, 433),
  INST(Vpshld          , VexRvmRmv          , V(XOP_M9,96,_,0,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 7856, 433),
  INST(Vpshlq          , VexRvmRmv          , V(XOP_M9,97,_,0,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 7863, 433),
  INST(Vpshlw          , VexRvmRmv          , V(XOP_M9,95,_,0,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 7870, 433),
  INST(Vpshufb         , VexRvm_Lx          , V(660F38,00,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 54 , 7877, 272),
  INST(Vpshufd         , VexRmi_Lx          , V(660F00,70,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 56 , 7885, 434),
  INST(Vpshufhw        , VexRmi_Lx          , V(F30F00,70,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 54 , 7893, 434),
  INST(Vpshuflw        , VexRmi_Lx          , V(F20F00,70,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 54 , 7902, 434),
  INST(Vpsignb         , VexRvm_Lx          , V(660F38,08,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 51 , 7911, 275),
  INST(Vpsignd         , VexRvm_Lx          , V(660F38,0A,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 51 , 7919, 275),
  INST(Vpsignw         , VexRvm_Lx          , V(660F38,09,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 51 , 7927, 275),
  INST(Vpslld          , VexRvmVmi_Lx       , V(660F00,F2,_,x,I,0,4,128), V(660F00,72,6,x,I,0,4,FV ), F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 56 , 7935, 435),
  INST(Vpslldq         , VexEvexVmi_Lx      , V(660F00,73,7,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 85 , 7942, 434),
  INST(Vpsllq          , VexRvmVmi_Lx       , V(660F00,F3,_,x,I,1,4,128), V(660F00,73,6,x,I,1,4,FV ), F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 57 , 7950, 436),
  INST(Vpsllvd         , VexRvm_Lx          , V(660F38,47,_,x,0,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 72 , 7957, 272),
  INST(Vpsllvq         , VexRvm_Lx          , V(660F38,47,_,x,1,1,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 74 , 7965, 272),
  INST(Vpsllvw         , VexRvm_Lx          , V(660F38,12,_,x,_,1,4,FVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 9  , 7973, 281),
  INST(Vpsllw          , VexRvmVmi_Lx       , V(660F00,F1,_,x,I,I,4,FVM), V(660F00,71,6,x,I,I,4,FVM), F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 54 , 7981, 437),
  INST(Vpsrad          , VexRvmVmi_Lx       , V(660F00,E2,_,x,I,0,4,128), V(660F00,72,4,x,I,0,4,FV ), F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 56 , 7988, 438),
  INST(Vpsraq          , VexRvmVmi_Lx       , V(660F00,E2,_,x,_,1,4,128), V(660F00,72,4,x,_,1,4,FV ), F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 6  , 7995, 439),
  INST(Vpsravd         , VexRvm_Lx          , V(660F38,46,_,x,0,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 72 , 8002, 272),
  INST(Vpsravq         , VexRvm_Lx          , V(660F38,46,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 6  , 8010, 281),
  INST(Vpsravw         , VexRvm_Lx          , V(660F38,11,_,x,_,1,4,FVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 9  , 8018, 281),
  INST(Vpsraw          , VexRvmVmi_Lx       , V(660F00,E1,_,x,I,I,4,128), V(660F00,71,4,x,I,I,4,FVM), F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 54 , 8026, 440),
  INST(Vpsrld          , VexRvmVmi_Lx       , V(660F00,D2,_,x,I,0,4,128), V(660F00,72,2,x,I,0,4,FV ), F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 56 , 8033, 441),
  INST(Vpsrldq         , VexEvexVmi_Lx      , V(660F00,73,3,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 85 , 8040, 434),
  INST(Vpsrlq          , VexRvmVmi_Lx       , V(660F00,D3,_,x,I,1,4,128), V(660F00,73,2,x,I,1,4,FV ), F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 57 , 8048, 442),
  INST(Vpsrlvd         , VexRvm_Lx          , V(660F38,45,_,x,0,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 72 , 8055, 272),
  INST(Vpsrlvq         , VexRvm_Lx          , V(660F38,45,_,x,1,1,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 74 , 8063, 272),
  INST(Vpsrlvw         , VexRvm_Lx          , V(660F38,10,_,x,_,1,4,FVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 9  , 8071, 281),
  INST(Vpsrlw          , VexRvmVmi_Lx       , V(660F00,D1,_,x,I,I,4,128), V(660F00,71,2,x,I,I,4,FVM), F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 54 , 8079, 443),
  INST(Vpsubb          , VexRvm_Lx          , V(660F00,F8,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 54 , 8086, 444),
  INST(Vpsubd          , VexRvm_Lx          , V(660F00,FA,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 56 , 8093, 444),
  INST(Vpsubq          , VexRvm_Lx          , V(660F00,FB,_,x,I,1,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 57 , 8100, 444),
  INST(Vpsubsb         , VexRvm_Lx          , V(660F00,E8,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 54 , 8107, 444),
  INST(Vpsubsw         , VexRvm_Lx          , V(660F00,E9,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 54 , 8115, 444),
  INST(Vpsubusb        , VexRvm_Lx          , V(660F00,D8,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 54 , 8123, 444),
  INST(Vpsubusw        , VexRvm_Lx          , V(660F00,D9,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 54 , 8132, 444),
  INST(Vpsubw          , VexRvm_Lx          , V(660F00,F9,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 54 , 8141, 444),
  INST(Vpternlogd      , VexRvmi_Lx         , V(660F3A,25,_,x,_,0,4,FV ), 0                         , F(RW)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 5  , 8148, 317),
  INST(Vpternlogq      , VexRvmi_Lx         , V(660F3A,25,_,x,_,1,4,FV ), 0                         , F(RW)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 6  , 8159, 317),
  INST(Vptest          , VexRm_Lx           , V(660F38,17,_,x,I,_,_,_  ), 0                         , F(RO)|F(Vex)                          , EF(WWWWWW__), 0 , 0 , kFamilyAvx , 3  , 8170, 445),
  INST(Vptestmb        , VexRvm_Lx          , V(660F38,26,_,x,_,0,4,FVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 63 , 8177, 446),
  INST(Vptestmd        , VexRvm_Lx          , V(660F38,27,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 64 , 8186, 446),
  INST(Vptestmq        , VexRvm_Lx          , V(660F38,27,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 68 , 8195, 446),
  INST(Vptestmw        , VexRvm_Lx          , V(660F38,26,_,x,_,1,4,FVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 63 , 8204, 446),
  INST(Vptestnmb       , VexRvm_Lx          , V(F30F38,26,_,x,_,0,4,FVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 63 , 8213, 446),
  INST(Vptestnmd       , VexRvm_Lx          , V(F30F38,27,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 64 , 8223, 446),
  INST(Vptestnmq       , VexRvm_Lx          , V(F30F38,27,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 68 , 8233, 446),
  INST(Vptestnmw       , VexRvm_Lx          , V(F30F38,26,_,x,_,1,4,FVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 63 , 8243, 446),
  INST(Vpunpckhbw      , VexRvm_Lx          , V(660F00,68,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 54 , 8253, 272),
  INST(Vpunpckhdq      , VexRvm_Lx          , V(660F00,6A,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 56 , 8264, 272),
  INST(Vpunpckhqdq     , VexRvm_Lx          , V(660F00,6D,_,x,I,1,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 57 , 8275, 272),
  INST(Vpunpckhwd      , VexRvm_Lx          , V(660F00,69,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 54 , 8287, 272),
  INST(Vpunpcklbw      , VexRvm_Lx          , V(660F00,60,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 54 , 8298, 272),
  INST(Vpunpckldq      , VexRvm_Lx          , V(660F00,62,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 56 , 8309, 272),
  INST(Vpunpcklqdq     , VexRvm_Lx          , V(660F00,6C,_,x,I,1,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 57 , 8320, 272),
  INST(Vpunpcklwd      , VexRvm_Lx          , V(660F00,61,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 54 , 8332, 272),
  INST(Vpxor           , VexRvm_Lx          , V(660F00,EF,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 51 , 8343, 381),
  INST(Vpxord          , VexRvm_Lx          , V(660F00,EF,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 5  , 8349, 382),
  INST(Vpxorq          , VexRvm_Lx          , V(660F00,EF,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 6  , 8356, 382),
  INST(Vrangepd        , VexRvmi_Lx         , V(660F3A,50,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 29 , 8363, 279),
  INST(Vrangeps        , VexRvmi_Lx         , V(660F3A,50,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 30 , 8372, 279),
  INST(Vrangesd        , VexRvmi            , V(660F3A,51,_,I,_,1,3,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 87 , 8381, 447),
  INST(Vrangess        , VexRvmi            , V(660F3A,51,_,I,_,0,2,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 87 , 8390, 448),
  INST(Vrcp14pd        , VexRm_Lx           , V(660F38,4C,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 6  , 8399, 302),
  INST(Vrcp14ps        , VexRm_Lx           , V(660F38,4C,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 5  , 8408, 302),
  INST(Vrcp14sd        , VexRvm             , V(660F38,4D,_,I,_,1,3,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 11 , 8417, 449),
  INST(Vrcp14ss        , VexRvm             , V(660F38,4D,_,I,_,0,2,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 11 , 8426, 450),
  INST(Vrcp28pd        , VexRm              , V(660F38,CA,_,2,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 33 , 8435, 312),
  INST(Vrcp28ps        , VexRm              , V(660F38,CA,_,2,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 34 , 8444, 312),
  INST(Vrcp28sd        , VexRvm             , V(660F38,CB,_,I,_,1,3,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 88 , 8453, 449),
  INST(Vrcp28ss        , VexRvm             , V(660F38,CB,_,I,_,0,2,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 88 , 8462, 450),
  INST(Vrcpps          , VexRm_Lx           , V(000F00,53,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 3  , 8471, 329),
  INST(Vrcpss          , VexRvm             , V(F30F00,53,_,I,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 3  , 8478, 451),
  INST(Vreducepd       , VexRmi_Lx          , V(660F3A,56,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 83 , 8485, 341),
  INST(Vreduceps       , VexRmi_Lx          , V(660F3A,56,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 89 , 8495, 341),
  INST(Vreducesd       , VexRvmi            , V(660F3A,57,_,I,_,1,3,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 12 , 8505, 447),
  INST(Vreducess       , VexRvmi            , V(660F3A,57,_,I,_,0,2,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 12 , 8515, 448),
  INST(Vrndscalepd     , VexRmi_Lx          , V(660F3A,09,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 28 , 8525, 341),
  INST(Vrndscaleps     , VexRmi_Lx          , V(660F3A,08,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 31 , 8537, 341),
  INST(Vrndscalesd     , VexRvmi            , V(660F3A,0B,_,I,_,1,3,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 36 , 8549, 447),
  INST(Vrndscaless     , VexRvmi            , V(660F3A,0A,_,I,_,0,2,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 36 , 8561, 448),
  INST(Vroundpd        , VexRmi_Lx          , V(660F3A,09,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 3  , 8573, 452),
  INST(Vroundps        , VexRmi_Lx          , V(660F3A,08,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 3  , 8582, 452),
  INST(Vroundsd        , VexRvmi            , V(660F3A,0B,_,I,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 3  , 8591, 453),
  INST(Vroundss        , VexRvmi            , V(660F3A,0A,_,I,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 3  , 8600, 454),
  INST(Vrsqrt14pd      , VexRm_Lx           , V(660F38,4E,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 6  , 8609, 302),
  INST(Vrsqrt14ps      , VexRm_Lx           , V(660F38,4E,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 5  , 8620, 302),
  INST(Vrsqrt14sd      , VexRvm             , V(660F38,4F,_,I,_,1,3,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 11 , 8631, 449),
  INST(Vrsqrt14ss      , VexRvm             , V(660F38,4F,_,I,_,0,2,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 11 , 8642, 450),
  INST(Vrsqrt28pd      , VexRm              , V(660F38,CC,_,2,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 33 , 8653, 312),
  INST(Vrsqrt28ps      , VexRm              , V(660F38,CC,_,2,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 34 , 8664, 312),
  INST(Vrsqrt28sd      , VexRvm             , V(660F38,CD,_,I,_,1,3,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 88 , 8675, 449),
  INST(Vrsqrt28ss      , VexRvm             , V(660F38,CD,_,I,_,0,2,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 88 , 8686, 450),
  INST(Vrsqrtps        , VexRm_Lx           , V(000F00,52,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 3  , 8697, 329),
  INST(Vrsqrtss        , VexRvm             , V(F30F00,52,_,I,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 3  , 8706, 451),
  INST(Vscalefpd       , VexRvm_Lx          , V(660F38,2C,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 22 , 8715, 281),
  INST(Vscalefps       , VexRvm_Lx          , V(660F38,2C,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 25 , 8725, 281),
  INST(Vscalefsd       , VexRvm             , V(660F38,2D,_,I,_,1,3,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 90 , 8735, 449),
  INST(Vscalefss       , VexRvm             , V(660F38,2D,_,I,_,0,2,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 90 , 8745, 450),
  INST(Vscatterdpd     , VexMr_Lx           , V(660F38,A2,_,x,_,1,3,T1S), 0                         , F(WO)|F(VM)|F(Evex)                   , EF(________), 0 , 0 , kFamilyAvx , 86 , 8755, 455),
  INST(Vscatterdps     , VexMr_Lx           , V(660F38,A2,_,x,_,0,2,T1S), 0                         , F(WO)|F(VM)|F(Evex)                   , EF(________), 0 , 0 , kFamilyAvx , 86 , 8767, 430),
  INST(Vscatterpf0dpd  , VexM_VM            , V(660F38,C6,5,2,_,1,3,T1S), 0                         , F(RO)|F(VM)|F(Evex)                   , EF(________), 0 , 0 , kFamilyNone, 0  , 8779, 334),
  INST(Vscatterpf0dps  , VexM_VM            , V(660F38,C6,5,2,_,0,2,T1S), 0                         , F(RO)|F(VM)|F(Evex)                   , EF(________), 0 , 0 , kFamilyNone, 0  , 8794, 335),
  INST(Vscatterpf0qpd  , VexM_VM            , V(660F38,C7,5,2,_,1,3,T1S), 0                         , F(RO)|F(VM)|F(Evex)                   , EF(________), 0 , 0 , kFamilyNone, 0  , 8809, 336),
  INST(Vscatterpf0qps  , VexM_VM            , V(660F38,C7,5,2,_,0,2,T1S), 0                         , F(RO)|F(VM)|F(Evex)                   , EF(________), 0 , 0 , kFamilyNone, 0  , 8824, 336),
  INST(Vscatterpf1dpd  , VexM_VM            , V(660F38,C6,6,2,_,1,3,T1S), 0                         , F(RO)|F(VM)|F(Evex)                   , EF(________), 0 , 0 , kFamilyNone, 0  , 8839, 334),
  INST(Vscatterpf1dps  , VexM_VM            , V(660F38,C6,6,2,_,0,2,T1S), 0                         , F(RO)|F(VM)|F(Evex)                   , EF(________), 0 , 0 , kFamilyNone, 0  , 8854, 335),
  INST(Vscatterpf1qpd  , VexM_VM            , V(660F38,C7,6,2,_,1,3,T1S), 0                         , F(RO)|F(VM)|F(Evex)                   , EF(________), 0 , 0 , kFamilyNone, 0  , 8869, 336),
  INST(Vscatterpf1qps  , VexM_VM            , V(660F38,C7,6,2,_,0,2,T1S), 0                         , F(RO)|F(VM)|F(Evex)                   , EF(________), 0 , 0 , kFamilyNone, 0  , 8884, 336),
  INST(Vscatterqpd     , VexMr_Lx           , V(660F38,A3,_,x,_,1,3,T1S), 0                         , F(WO)|F(VM)|F(Evex)                   , EF(________), 0 , 0 , kFamilyAvx , 86 , 8899, 432),
  INST(Vscatterqps     , VexMr_Lx           , V(660F38,A3,_,x,_,0,2,T1S), 0                         , F(WO)|F(VM)|F(Evex)                   , EF(________), 0 , 0 , kFamilyAvx , 86 , 8911, 431),
  INST(Vshuff32x4      , VexRvmi_Lx         , V(660F3A,23,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 5  , 8923, 456),
  INST(Vshuff64x2      , VexRvmi_Lx         , V(660F3A,23,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 6  , 8934, 456),
  INST(Vshufi32x4      , VexRvmi_Lx         , V(660F3A,43,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 5  , 8945, 456),
  INST(Vshufi64x2      , VexRvmi_Lx         , V(660F3A,43,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 6  , 8956, 456),
  INST(Vshufpd         , VexRvmi_Lx         , V(660F00,C6,_,x,I,1,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 20 , 8967, 378),
  INST(Vshufps         , VexRvmi_Lx         , V(000F00,C6,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 73 , 8975, 378),
  INST(Vsqrtpd         , VexRm_Lx           , V(660F00,51,_,x,I,1,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 0  , 8983, 299),
  INST(Vsqrtps         , VexRm_Lx           , V(000F00,51,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 1  , 8991, 299),
  INST(Vsqrtsd         , VexRvm             , V(F20F00,51,_,I,I,1,3,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 2  , 8999, 273),
  INST(Vsqrtss         , VexRvm             , V(F30F00,51,_,I,I,0,2,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 2  , 9007, 274),
  INST(Vstmxcsr        , VexM               , V(000F00,AE,3,0,I,_,_,_  ), 0                         , F(Vex)|F(Volatile)                    , EF(________), 0 , 0 , kFamilyNone, 0  , 9015, 457),
  INST(Vsubpd          , VexRvm_Lx          , V(660F00,5C,_,x,I,1,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 52 , 9024, 272),
  INST(Vsubps          , VexRvm_Lx          , V(000F00,5C,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 53 , 9031, 272),
  INST(Vsubsd          , VexRvm             , V(F20F00,5C,_,I,I,1,3,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 2  , 9038, 273),
  INST(Vsubss          , VexRvm             , V(F30F00,5C,_,I,I,0,2,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 2  , 9045, 274),
  INST(Vtestpd         , VexRm_Lx           , V(660F38,0F,_,x,0,_,_,_  ), 0                         , F(RO)|F(Vex)                          , EF(WWWWWW__), 0 , 0 , kFamilyAvx , 3  , 9052, 445),
  INST(Vtestps         , VexRm_Lx           , V(660F38,0E,_,x,0,_,_,_  ), 0                         , F(RO)|F(Vex)                          , EF(WWWWWW__), 0 , 0 , kFamilyAvx , 3  , 9060, 445),
  INST(Vucomisd        , VexRm              , V(660F00,2E,_,I,I,1,3,T1S), 0                         , F(RO)|F(Vex)|F(Evex)                  , EF(WWWWWW__), 0 , 0 , kFamilyAvx , 19 , 9068, 295),
  INST(Vucomiss        , VexRm              , V(000F00,2E,_,I,I,0,2,T1S), 0                         , F(RO)|F(Vex)|F(Evex)                  , EF(WWWWWW__), 0 , 0 , kFamilyAvx , 19 , 9077, 296),
  INST(Vunpckhpd       , VexRvm_Lx          , V(660F00,15,_,x,I,1,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 73 , 9086, 272),
  INST(Vunpckhps       , VexRvm_Lx          , V(000F00,15,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 20 , 9096, 272),
  INST(Vunpcklpd       , VexRvm_Lx          , V(660F00,14,_,x,I,1,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 73 , 9106, 272),
  INST(Vunpcklps       , VexRvm_Lx          , V(000F00,14,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 20 , 9116, 272),
  INST(Vxorpd          , VexRvm_Lx          , V(660F00,57,_,x,I,1,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 7  , 9126, 444),
  INST(Vxorps          , VexRvm_Lx          , V(000F00,57,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 8  , 9133, 444),
  INST(Vzeroall        , VexOp              , V(000F00,77,_,1,I,_,_,_  ), 0                         , F(Vex)|F(Volatile)                    , EF(________), 0 , 0 , kFamilyNone, 0  , 9140, 458),
  INST(Vzeroupper      , VexOp              , V(000F00,77,_,0,I,_,_,_  ), 0                         , F(Vex)|F(Volatile)                    , EF(________), 0 , 0 , kFamilyNone, 0  , 9149, 458),
  INST(Wrfsbase        , X86M               , O(F30F00,AE,2,_,x,_,_,_  ), 0                         , F(RO)|F(Volatile)                     , EF(________), 0 , 0 , kFamilyNone, 0  , 9160, 459),
  INST(Wrgsbase        , X86M               , O(F30F00,AE,3,_,x,_,_,_  ), 0                         , F(RO)|F(Volatile)                     , EF(________), 0 , 0 , kFamilyNone, 0  , 9169, 459),
  INST(Xadd            , X86Xadd            , O(000F00,C0,_,_,x,_,_,_  ), 0                         , F(RW)|F(Xchg)|F(Lock)                 , EF(WWWWWW__), 0 , 0 , kFamilyNone, 0  , 9178, 460),
  INST(Xchg            , X86Xchg            , O(000000,86,_,_,x,_,_,_  ), 0                         , F(RW)|F(Xchg)|F(Lock)                 , EF(________), 0 , 0 , kFamilyNone, 0  , 374 , 461),
  INST(Xgetbv          , X86Op              , O(000F01,D0,_,_,_,_,_,_  ), 0                         , F(WO)|F(Special)                      , EF(________), 0 , 0 , kFamilyNone, 0  , 9183, 462),
  INST(Xor             , X86Arith           , O(000000,30,6,_,x,_,_,_  ), 0                         , F(RW)|F(Lock)                         , EF(WWWUWW__), 0 , 0 , kFamilyNone, 0  , 8345, 268),
  INST(Xorpd           , ExtRm              , O(660F00,57,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 145, 9127, 195),
  INST(Xorps           , ExtRm              , O(000F00,57,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 146, 9134, 195),
  INST(Xrstor          , X86M_Only          , O(000F00,AE,5,_,_,_,_,_  ), 0                         , F(RO)|F(Volatile)|F(Special)          , EF(________), 0 , 0 , kFamilyNone, 0  , 1051, 463),
  INST(Xrstor64        , X86M_Only          , O(000F00,AE,5,_,1,_,_,_  ), 0                         , F(RO)|F(Volatile)|F(Special)          , EF(________), 0 , 0 , kFamilyNone, 0  , 1059, 464),
  INST(Xrstors         , X86M_Only          , O(000F00,C7,3,_,_,_,_,_  ), 0                         , F(RO)|F(Volatile)|F(Special)          , EF(________), 0 , 0 , kFamilyNone, 0  , 9190, 463),
  INST(Xrstors64       , X86M_Only          , O(000F00,C7,3,_,1,_,_,_  ), 0                         , F(RO)|F(Volatile)|F(Special)          , EF(________), 0 , 0 , kFamilyNone, 0  , 9198, 464),
  INST(Xsave           , X86M_Only          , O(000F00,AE,4,_,_,_,_,_  ), 0                         , F(WO)|F(Volatile)|F(Special)          , EF(________), 0 , 0 , kFamilyNone, 0  , 1069, 465),
  INST(Xsave64         , X86M_Only          , O(000F00,AE,4,_,1,_,_,_  ), 0                         , F(WO)|F(Volatile)|F(Special)          , EF(________), 0 , 0 , kFamilyNone, 0  , 1076, 466),
  INST(Xsavec          , X86M_Only          , O(000F00,C7,4,_,_,_,_,_  ), 0                         , F(WO)|F(Volatile)|F(Special)          , EF(________), 0 , 0 , kFamilyNone, 0  , 9208, 465),
  INST(Xsavec64        , X86M_Only          , O(000F00,C7,4,_,1,_,_,_  ), 0                         , F(WO)|F(Volatile)|F(Special)          , EF(________), 0 , 0 , kFamilyNone, 0  , 9215, 466),
  INST(Xsaveopt        , X86M_Only          , O(000F00,AE,6,_,_,_,_,_  ), 0                         , F(WO)|F(Volatile)|F(Special)          , EF(________), 0 , 0 , kFamilyNone, 0  , 9224, 465),
  INST(Xsaveopt64      , X86M_Only          , O(000F00,AE,6,_,1,_,_,_  ), 0                         , F(WO)|F(Volatile)|F(Special)          , EF(________), 0 , 0 , kFamilyNone, 0  , 9233, 466),
  INST(Xsaves          , X86M_Only          , O(000F00,C7,5,_,_,_,_,_  ), 0                         , F(WO)|F(Volatile)|F(Special)          , EF(________), 0 , 0 , kFamilyNone, 0  , 9244, 465),
  INST(Xsaves64        , X86M_Only          , O(000F00,C7,5,_,1,_,_,_  ), 0                         , F(WO)|F(Volatile)|F(Special)          , EF(________), 0 , 0 , kFamilyNone, 0  , 9251, 466),
  INST(Xsetbv          , X86Op              , O(000F01,D1,_,_,_,_,_,_  ), 0                         , F(RO)|F(Volatile)|F(Special)          , EF(________), 0 , 0 , kFamilyNone, 0  , 9260, 467)
  // ${instData:End}
};

#undef NAME_DATA_INDEX
#undef INST

// ${commonData:Begin}
// ------------------- Automatically generated, do not edit -------------------
#define JUMP_TYPE(VAL) AnyInst::kJumpType##VAL
#define SINGLE_REG(VAL) X86Inst::kSingleReg##VAL
const X86Inst::CommonData X86InstDB::commonData[] = {
  { 0                                     , 0  , 0  , 0x00, 0x00, 0  , 0  , 0 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #0
  { F(RW)                                 , 0  , 0  , 0x00, 0x3F, 0  , 349, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #1
  { F(RW)                                 , 0  , 0  , 0x00, 0x3F, 0  , 350, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #2
  { F(RW)|F(Lock)                         , 0  , 0  , 0x20, 0x3F, 0  , 14 , 10, JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #3
  { F(RW)                                 , 0  , 0  , 0x20, 0x20, 0  , 22 , 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #4
  { F(RW)|F(Lock)                         , 0  , 0  , 0x00, 0x3F, 0  , 14 , 10, JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #5
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 0  , 296, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #6
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 0  , 351, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #7
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 0  , 352, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #8
  { F(RW)                                 , 0  , 0  , 0x01, 0x01, 0  , 22 , 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #9
  { F(WO)                                 , 0  , 0  , 0x00, 0x00, 0  , 64 , 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #10
  { F(WO)                                 , 0  , 0  , 0x00, 0x00, 0  , 71 , 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #11
  { F(RW)|F(Lock)                         , 0  , 0  , 0x00, 0x3F, 0  , 14 , 10, JUMP_TYPE(None)       , SINGLE_REG(RO)  , 0 }, // #12
  { F(RW)                                 , 0  , 0  , 0x00, 0x3F, 0  , 251, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #13
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 0  , 296, 1 , JUMP_TYPE(None)       , SINGLE_REG(RO)  , 0 }, // #14
  { F(RW)                                 , 0  , 0  , 0x00, 0x3F, 0  , 253, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #15
  { F(WO)                                 , 0  , 0  , 0x00, 0x3F, 0  , 162, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #16
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 0  , 298, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #17
  { F(RW)|F(Special)                      , 0  , 0  , 0x00, 0x00, 0  , 353, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #18
  { F(RW)                                 , 0  , 0  , 0x00, 0x3F, 0  , 162, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #19
  { F(RW)                                 , 0  , 0  , 0x00, 0x3F, 0  , 21 , 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #20
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 0  , 354, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #21
  { F(RO)                                 , 0  , 0  , 0x00, 0x3B, 1  , 152, 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #22
  { F(RW)|F(Lock)                         , 0  , 0  , 0x00, 0x3B, 2  , 155, 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #23
  { F(RW)|F(Lock)                         , 0  , 0  , 0x00, 0x3B, 3  , 155, 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #24
  { F(RW)|F(Lock)                         , 0  , 0  , 0x00, 0x3B, 4  , 155, 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #25
  { F(RW)|F(Volatile)                     , 0  , 0  , 0x00, 0x00, 0  , 255, 2 , JUMP_TYPE(Call)       , SINGLE_REG(None), 0 }, // #26
  { F(RW)|F(Special)                      , 0  , 0  , 0x00, 0x00, 0  , 355, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #27
  { F(RW)|F(Special)                      , 0  , 0  , 0x00, 0x00, 0  , 356, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #28
  { F(RW)|F(Special)                      , 0  , 0  , 0x00, 0x00, 0  , 357, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #29
  { F(Volatile)                           , 0  , 0  , 0x00, 0x08, 0  , 263, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #30
  { F(Volatile)                           , 0  , 0  , 0x00, 0x20, 0  , 263, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #31
  { F(Volatile)                           , 0  , 0  , 0x00, 0x40, 0  , 263, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #32
  { F(RO)|F(Volatile)                     , 0  , 0  , 0x00, 0x00, 0  , 358, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #33
  { F(WO)|F(Volatile)|F(Special)          , 0  , 0  , 0x00, 0x00, 0  , 359, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #34
  { 0                                     , 0  , 0  , 0x20, 0x20, 0  , 263, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #35
  { F(RW)                                 , 0  , 0  , 0x24, 0x00, 0  , 21 , 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #36
  { F(RW)                                 , 0  , 0  , 0x20, 0x00, 0  , 21 , 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #37
  { F(RW)                                 , 0  , 0  , 0x04, 0x00, 0  , 21 , 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #38
  { F(RW)                                 , 0  , 0  , 0x07, 0x00, 0  , 21 , 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #39
  { F(RW)                                 , 0  , 0  , 0x03, 0x00, 0  , 21 , 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #40
  { F(RW)                                 , 0  , 0  , 0x01, 0x00, 0  , 21 , 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #41
  { F(RW)                                 , 0  , 0  , 0x10, 0x00, 0  , 21 , 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #42
  { F(RW)                                 , 0  , 0  , 0x02, 0x00, 0  , 21 , 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #43
  { F(RO)                                 , 0  , 0  , 0x00, 0x3F, 0  , 24 , 10, JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #44
  { F(RW)|F(Special)|F(Rep)|F(Repnz)      , 0  , 0  , 0x40, 0x3F, 0  , 360, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #45
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 0  , 361, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #46
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 0  , 362, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #47
  { F(RW)|F(Lock)|F(Special)              , 0  , 0  , 0x00, 0x3F, 0  , 108, 4 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #48
  { F(RW)|F(Lock)|F(Special)              , 0  , 0  , 0x00, 0x04, 0  , 363, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #49
  { F(RW)|F(Lock)|F(Special)              , 0  , 0  , 0x00, 0x04, 0  , 364, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #50
  { F(RO)                                 , 0  , 0  , 0x00, 0x3F, 0  , 365, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #51
  { F(RO)                                 , 0  , 0  , 0x00, 0x3F, 0  , 366, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #52
  { F(RW)|F(Volatile)|F(Special)          , 0  , 0  , 0x00, 0x00, 0  , 367, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #53
  { F(RW)|F(Special)                      , 0  , 0  , 0x00, 0x00, 0  , 368, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #54
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 0  , 257, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #55
  { F(WO)                                 , 0  , 16 , 0x00, 0x00, 0  , 62 , 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #56
  { F(WO)                                 , 0  , 16 , 0x00, 0x00, 0  , 64 , 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #57
  { F(WO)                                 , 0  , 8  , 0x00, 0x00, 0  , 369, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #58
  { F(WO)                                 , 0  , 16 , 0x00, 0x00, 0  , 370, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #59
  { F(WO)                                 , 0  , 8  , 0x00, 0x00, 0  , 370, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #60
  { F(WO)                                 , 0  , 8  , 0x00, 0x00, 0  , 371, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #61
  { F(WO)                                 , 0  , 8  , 0x00, 0x00, 0  , 372, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #62
  { F(WO)                                 , 0  , 4  , 0x00, 0x00, 0  , 62 , 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #63
  { F(WO)                                 , 0  , 8  , 0x00, 0x00, 0  , 373, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #64
  { F(WO)                                 , 0  , 4  , 0x00, 0x00, 0  , 373, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #65
  { F(WO)                                 , 0  , 8  , 0x00, 0x00, 0  , 233, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #66
  { F(WO)                                 , 0  , 8  , 0x00, 0x00, 0  , 313, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #67
  { F(RW)|F(Special)                      , 0  , 0  , 0x00, 0x00, 0  , 374, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #68
  { F(RW)|F(Special)                      , 0  , 0  , 0x00, 0x00, 0  , 375, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #69
  { F(RW)|F(Special)                      , 0  , 0  , 0x28, 0x3F, 0  , 349, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #70
  { F(RW)|F(Lock)                         , 0  , 0  , 0x00, 0x1F, 5  , 259, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #71
  { F(RW)|F(Special)                      , 0  , 0  , 0x00, 0x3F, 0  , 112, 4 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #72
  { F(Volatile)                           , 0  , 0  , 0x00, 0x00, 0  , 263, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #73
  { F(Volatile)|F(Special)                , 0  , 0  , 0x00, 0x00, 0  , 376, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #74
  { F(WO)                                 , 0  , 8  , 0x00, 0x00, 0  , 377, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #75
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 6  , 261, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #76
  { F(Fp)                                 , 0  , 0  , 0x00, 0x00, 0  , 263, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #77
  { F(Fp)|F(FPU_M4)|F(FPU_M8)             , 0  , 0  , 0x00, 0x00, 0  , 158, 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #78
  { F(Fp)                                 , 0  , 0  , 0x00, 0x00, 0  , 263, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #79
  { F(Fp)                                 , 0  , 0  , 0x00, 0x00, 0  , 378, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #80
  { F(Fp)                                 , 0  , 0  , 0x20, 0x00, 0  , 264, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #81
  { F(Fp)                                 , 0  , 0  , 0x24, 0x00, 0  , 264, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #82
  { F(Fp)                                 , 0  , 0  , 0x04, 0x00, 0  , 264, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #83
  { F(Fp)                                 , 0  , 0  , 0x10, 0x00, 0  , 264, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #84
  { F(Fp)                                 , 0  , 0  , 0x00, 0x00, 0  , 265, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #85
  { F(Fp)                                 , 0  , 0  , 0x00, 0x3F, 0  , 264, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #86
  { F(Fp)                                 , 0  , 0  , 0x00, 0x00, 0  , 264, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #87
  { F(Fp)|F(FPU_M2)|F(FPU_M4)             , 0  , 0  , 0x00, 0x00, 0  , 379, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #88
  { F(Fp)|F(FPU_M2)|F(FPU_M4)|F(FPU_M8)   , 0  , 0  , 0x00, 0x00, 7  , 380, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #89
  { F(Fp)|F(FPU_M2)|F(FPU_M4)|F(FPU_M8)   , 0  , 0  , 0x00, 0x00, 8  , 380, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #90
  { F(Fp)|F(FPU_M2)|F(FPU_M4)|F(FPU_M8)   , 0  , 0  , 0x00, 0x00, 9  , 380, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #91
  { F(Fp)|F(FPU_M2)|F(FPU_M4)|F(FPU_M8)   , 0  , 0  , 0x00, 0x00, 10 , 381, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #92
  { F(Fp)                                 , 0  , 0  , 0x00, 0x00, 0  , 382, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #93
  { F(Fp)                                 , 0  , 0  , 0x00, 0x00, 0  , 383, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #94
  { F(Fp)                                 , 0  , 0  , 0x00, 0x00, 11 , 384, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #95
  { F(Fp)|F(FPU_M4)|F(FPU_M8)             , 0  , 0  , 0x00, 0x00, 0  , 266, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #96
  { F(Fp)|F(FPU_M4)|F(FPU_M8)|F(FPU_M10)  , 0  , 0  , 0x00, 0x00, 12 , 381, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #97
  { F(Fp)                                 , 0  , 0  , 0x00, 0x00, 13 , 384, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #98
  { F(Fp)|F(Volatile)                     , 0  , 0  , 0x00, 0x00, 0  , 263, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #99
  { F(Fp)                                 , 0  , 0  , 0x00, 0x00, 0  , 385, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #100
  { F(RW)|F(Special)                      , 0  , 0  , 0x00, 0x3F, 0  , 116, 4 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #101
  { F(RW)|F(Special)                      , 0  , 0  , 0x00, 0x3F, 0  , 34 , 10, JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #102
  { F(WO)|F(Volatile)|F(Special)          , 0  , 0  , 0x00, 0x00, 14 , 386, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #103
  { F(RW)|F(Lock)                         , 0  , 0  , 0x00, 0x1F, 15 , 259, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #104
  { F(WO)|F(Volatile)|F(Special)|F(Rep)   , 0  , 0  , 0x00, 0x00, 0  , 387, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #105
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 16 , 267, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #106
  { F(Volatile)                           , 0  , 0  , 0x00, 0x88, 0  , 388, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #107
  { F(Volatile)                           , 0  , 0  , 0x00, 0x88, 0  , 263, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #108
  { F(Volatile)                           , 0  , 0  , 0x24, 0x00, 17 , 389, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #109
  { F(Volatile)                           , 0  , 0  , 0x20, 0x00, 18 , 389, 1 , JUMP_TYPE(Conditional), SINGLE_REG(None), 0 }, // #110
  { F(Volatile)                           , 0  , 0  , 0x20, 0x00, 19 , 389, 1 , JUMP_TYPE(Conditional), SINGLE_REG(None), 0 }, // #111
  { F(Volatile)                           , 0  , 0  , 0x24, 0x00, 20 , 389, 1 , JUMP_TYPE(Conditional), SINGLE_REG(None), 0 }, // #112
  { F(Volatile)                           , 0  , 0  , 0x20, 0x00, 19 , 390, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #113
  { F(Volatile)                           , 0  , 0  , 0x04, 0x00, 21 , 389, 1 , JUMP_TYPE(Conditional), SINGLE_REG(None), 0 }, // #114
  { F(Volatile)|F(Special)                , 0  , 0  , 0x00, 0x00, 22 , 269, 2 , JUMP_TYPE(Conditional), SINGLE_REG(None), 0 }, // #115
  { F(Volatile)                           , 0  , 0  , 0x07, 0x00, 23 , 389, 1 , JUMP_TYPE(Conditional), SINGLE_REG(None), 0 }, // #116
  { F(Volatile)                           , 0  , 0  , 0x03, 0x00, 24 , 389, 1 , JUMP_TYPE(Conditional), SINGLE_REG(None), 0 }, // #117
  { F(Volatile)                           , 0  , 0  , 0x03, 0x00, 25 , 389, 1 , JUMP_TYPE(Conditional), SINGLE_REG(None), 0 }, // #118
  { F(Volatile)                           , 0  , 0  , 0x07, 0x00, 26 , 389, 1 , JUMP_TYPE(Conditional), SINGLE_REG(None), 0 }, // #119
  { F(Volatile)                           , 0  , 0  , 0x00, 0x00, 27 , 271, 2 , JUMP_TYPE(Direct)     , SINGLE_REG(None), 0 }, // #120
  { F(Volatile)                           , 0  , 0  , 0x20, 0x00, 18 , 390, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #121
  { F(Volatile)                           , 0  , 0  , 0x04, 0x00, 28 , 389, 1 , JUMP_TYPE(Conditional), SINGLE_REG(None), 0 }, // #122
  { F(Volatile)                           , 0  , 0  , 0x01, 0x00, 29 , 389, 1 , JUMP_TYPE(Conditional), SINGLE_REG(None), 0 }, // #123
  { F(Volatile)                           , 0  , 0  , 0x10, 0x00, 30 , 389, 1 , JUMP_TYPE(Conditional), SINGLE_REG(None), 0 }, // #124
  { F(Volatile)                           , 0  , 0  , 0x02, 0x00, 31 , 389, 1 , JUMP_TYPE(Conditional), SINGLE_REG(None), 0 }, // #125
  { F(Volatile)                           , 0  , 0  , 0x01, 0x00, 32 , 389, 1 , JUMP_TYPE(Conditional), SINGLE_REG(None), 0 }, // #126
  { F(Volatile)                           , 0  , 0  , 0x10, 0x00, 33 , 389, 1 , JUMP_TYPE(Conditional), SINGLE_REG(None), 0 }, // #127
  { F(Volatile)                           , 0  , 0  , 0x02, 0x00, 34 , 389, 1 , JUMP_TYPE(Conditional), SINGLE_REG(None), 0 }, // #128
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 391, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #129
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 35 , 273, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #130
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 36 , 275, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #131
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 37 , 277, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #132
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 38 , 279, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #133
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 392, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #134
  { F(RO)|F(Vex)                          , 0  , 0  , 0x00, 0x3F, 0  , 393, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #135
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 394, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #136
  { F(RW)|F(Volatile)|F(Special)          , 0  , 0  , 0x3E, 0x00, 0  , 395, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #137
  { F(WO)                                 , 0  , 16 , 0x00, 0x00, 0  , 206, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #138
  { F(RO)|F(Volatile)                     , 0  , 0  , 0x00, 0x00, 0  , 396, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #139
  { F(WO)                                 , 0  , 0  , 0x00, 0x00, 0  , 397, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #140
  { F(Volatile)|F(Special)                , 0  , 0  , 0x00, 0x00, 0  , 263, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #141
  { F(WO)|F(Special)|F(Rep)               , 0  , 1  , 0x40, 0x00, 0  , 398, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #142
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 39 , 281, 2 , JUMP_TYPE(Conditional), SINGLE_REG(None), 0 }, // #143
  { F(RW)                                 , 0  , 0  , 0x04, 0x00, 40 , 281, 2 , JUMP_TYPE(Conditional), SINGLE_REG(None), 0 }, // #144
  { F(RW)                                 , 0  , 0  , 0x04, 0x00, 41 , 281, 2 , JUMP_TYPE(Conditional), SINGLE_REG(None), 0 }, // #145
  { F(RW)                                 , 0  , 0  , 0x00, 0x3F, 0  , 161, 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #146
  { F(RO)|F(Special)                      , 0  , 0  , 0x00, 0x00, 0  , 399, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #147
  { F(RO)|F(Special)                      , 0  , 0  , 0x00, 0x00, 0  , 400, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #148
  { F(RW)|F(Volatile)                     , 0  , 0  , 0x00, 0x00, 0  , 263, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #149
  { F(RO)|F(Volatile)|F(Special)          , 0  , 0  , 0x00, 0x00, 0  , 0  , 0 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #150
  { F(WO)                                 , 0  , 0  , 0x00, 0x00, 0  , 0  , 14, JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #151
  { F(WO)                                 , 0  , 16 , 0x00, 0x00, 42 , 64 , 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #152
  { F(WO)                                 , 0  , 16 , 0x00, 0x00, 43 , 64 , 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #153
  { F(WO)                                 , 0  , 0  , 0x00, 0x00, 44 , 52 , 6 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #154
  { F(WO)                                 , 0  , 16 , 0x00, 0x00, 45 , 283, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #155
  { F(WO)                                 , 0  , 8  , 0x00, 0x00, 0  , 401, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #156
  { F(WO)                                 , 0  , 16 , 0x00, 0x00, 46 , 64 , 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #157
  { F(WO)                                 , 0  , 16 , 0x00, 0x00, 47 , 64 , 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #158
  { F(WO)                                 , 0  , 8  , 0x00, 0x00, 0  , 402, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #159
  { F(RW)                                 , 8  , 8  , 0x00, 0x00, 48 , 212, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #160
  { F(RW)                                 , 8  , 8  , 0x00, 0x00, 49 , 212, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #161
  { F(RW)                                 , 8  , 8  , 0x00, 0x00, 0  , 402, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #162
  { F(WO)                                 , 0  , 8  , 0x00, 0x00, 50 , 212, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #163
  { F(WO)                                 , 0  , 8  , 0x00, 0x00, 51 , 212, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #164
  { F(WO)                                 , 0  , 8  , 0x00, 0x00, 0  , 403, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #165
  { F(WO)                                 , 0  , 16 , 0x00, 0x00, 52 , 203, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #166
  { F(WO)                                 , 0  , 8  , 0x00, 0x00, 0  , 56 , 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #167
  { F(WO)                                 , 0  , 16 , 0x00, 0x00, 53 , 203, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #168
  { F(WO)                                 , 0  , 16 , 0x00, 0x00, 54 , 203, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #169
  { F(WO)                                 , 0  , 8  , 0x00, 0x00, 55 , 404, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #170
  { F(WO)                                 , 0  , 8  , 0x00, 0x00, 56 , 212, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #171
  { F(WO)                                 , 0  , 4  , 0x00, 0x00, 57 , 288, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #172
  { F(WO)                                 , 0  , 16 , 0x00, 0x00, 58 , 58 , 6 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #173
  { F(WO)                                 , 0  , 16 , 0x00, 0x00, 0  , 405, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #174
  { F(WO)|F(Special)|F(Rep)               , 0  , 0  , 0x00, 0x00, 0  , 406, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #175
  { F(WO)|F(ZeroIfMem)                    , 0  , 8  , 0x00, 0x00, 59 , 285, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #176
  { F(WO)|F(ZeroIfMem)                    , 0  , 4  , 0x00, 0x00, 60 , 287, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #177
  { F(WO)                                 , 0  , 0  , 0x00, 0x00, 0  , 289, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #178
  { F(WO)                                 , 0  , 0  , 0x00, 0x00, 0  , 407, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #179
  { F(WO)                                 , 0  , 16 , 0x00, 0x00, 61 , 64 , 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #180
  { F(WO)                                 , 0  , 16 , 0x00, 0x00, 62 , 64 , 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #181
  { F(RW)|F(Special)                      , 0  , 0  , 0x00, 0x3F, 0  , 34 , 4 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #182
  { F(RW)|F(Special)                      , 0  , 0  , 0x00, 0x00, 0  , 291, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #183
  { F(RW)|F(Lock)                         , 0  , 0  , 0x00, 0x3F, 0  , 260, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #184
  { 0                                     , 0  , 0  , 0x00, 0x00, 0  , 293, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #185
  { F(RW)|F(Lock)                         , 0  , 0  , 0x00, 0x00, 0  , 260, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #186
  { F(RO)|F(Volatile)|F(Special)          , 0  , 0  , 0x00, 0x00, 63 , 408, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #187
  { F(RO)|F(Volatile)|F(Special)|F(Rep)   , 0  , 0  , 0x00, 0x00, 0  , 409, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #188
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 0  , 295, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #189
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 0  , 297, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #190
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 0  , 295, 2 , JUMP_TYPE(None)       , SINGLE_REG(RO)  , 0 }, // #191
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 0  , 295, 2 , JUMP_TYPE(None)       , SINGLE_REG(WO)  , 0 }, // #192
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 0  , 263, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #193
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 0  , 295, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #194
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 0  , 296, 1 , JUMP_TYPE(None)       , SINGLE_REG(WO)  , 0 }, // #195
  { F(WO)|F(Special)                      , 0  , 0  , 0x00, 0x00, 0  , 410, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #196
  { F(WO)|F(Special)                      , 0  , 0  , 0x00, 0x00, 0  , 411, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #197
  { F(WO)|F(Special)                      , 0  , 0  , 0x00, 0x00, 0  , 412, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #198
  { F(WO)|F(Special)                      , 0  , 0  , 0x00, 0x00, 0  , 413, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #199
  { F(WO)                                 , 0  , 0  , 0x00, 0x00, 0  , 251, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #200
  { F(WO)                                 , 0  , 8  , 0x00, 0x00, 0  , 414, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #201
  { F(WO)                                 , 0  , 8  , 0x00, 0x00, 0  , 415, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #202
  { F(WO)                                 , 0  , 8  , 0x00, 0x00, 64 , 299, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #203
  { F(WO)                                 , 0  , 8  , 0x00, 0x00, 0  , 301, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #204
  { F(WO)                                 , 0  , 0  , 0x00, 0x00, 0  , 301, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #205
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 0  , 416, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #206
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 0  , 417, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #207
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 0  , 418, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #208
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 0  , 419, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #209
  { F(WO)                                 , 0  , 8  , 0x00, 0x00, 0  , 420, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #210
  { F(WO)                                 , 0  , 16 , 0x00, 0x00, 0  , 233, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #211
  { F(WO)                                 , 0  , 16 , 0x00, 0x00, 0  , 236, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #212
  { F(WO)|F(Volatile)|F(Special)          , 0  , 0  , 0x00, 0x00, 65 , 120, 4 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #213
  { F(Volatile)|F(Special)                , 0  , 0  , 0x00, 0x00, 0  , 421, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #214
  { F(WO)                                 , 0  , 0  , 0x00, 0x3F, 0  , 161, 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #215
  { F(Volatile)|F(Special)                , 0  , 0  , 0x00, 0xFF, 0  , 263, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #216
  { F(Volatile)|F(Special)                , 0  , 0  , 0x00, 0xFF, 0  , 421, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #217
  { F(Volatile)|F(Special)                , 0  , 0  , 0x00, 0xFF, 0  , 422, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #218
  { F(RO)|F(Volatile)                     , 0  , 0  , 0x00, 0x3F, 0  , 358, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #219
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 0  , 301, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #220
  { F(WO)                                 , 0  , 16 , 0x00, 0x00, 0  , 71 , 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #221
  { F(WO)                                 , 0  , 8  , 0x00, 0x00, 0  , 423, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #222
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 66 , 303, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #223
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 67 , 424, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #224
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 68 , 303, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #225
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 69 , 303, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #226
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 70 , 303, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #227
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 71 , 303, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #228
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 72 , 303, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #229
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 73 , 424, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #230
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 74 , 303, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #231
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 75 , 303, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #232
  { F(RO)                                 , 0  , 0  , 0x00, 0x3F, 0  , 345, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #233
  { F(RO)|F(Volatile)|F(Special)          , 0  , 0  , 0x00, 0x00, 76 , 124, 4 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #234
  { F(Volatile)|F(Special)                , 0  , 0  , 0xFF, 0x00, 0  , 263, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #235
  { F(Volatile)|F(Special)                , 0  , 0  , 0xFF, 0x00, 0  , 421, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #236
  { F(Volatile)|F(Special)                , 0  , 0  , 0xFF, 0x00, 0  , 422, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #237
  { F(RW)|F(Special)                      , 0  , 0  , 0x20, 0x21, 0  , 425, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #238
  { F(WO)                                 , 0  , 4  , 0x00, 0x00, 0  , 233, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #239
  { F(WO)                                 , 0  , 8  , 0x00, 0x00, 0  , 426, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #240
  { F(WO)                                 , 0  , 8  , 0x00, 0x3F, 0  , 427, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #241
  { F(WO)|F(Volatile)|F(Special)          , 0  , 0  , 0x00, 0x00, 0  , 428, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #242
  { F(WO)|F(Volatile)|F(Special)          , 0  , 0  , 0x00, 0x00, 0  , 429, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #243
  { F(RW)|F(Volatile)|F(Special)          , 0  , 0  , 0x00, 0x00, 0  , 305, 2 , JUMP_TYPE(Return)     , SINGLE_REG(None), 0 }, // #244
  { F(RW)|F(Special)                      , 0  , 0  , 0x00, 0x21, 0  , 425, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #245
  { F(WO)                                 , 0  , 0  , 0x00, 0x00, 0  , 307, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #246
  { F(WO)                                 , 0  , 8  , 0x00, 0x00, 0  , 430, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #247
  { F(WO)                                 , 0  , 4  , 0x00, 0x00, 0  , 431, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #248
  { F(RO)|F(Volatile)|F(Special)          , 0  , 0  , 0x00, 0x3E, 0  , 432, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #249
  { F(RW)|F(Special)                      , 0  , 0  , 0x00, 0x3F, 0  , 425, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #250
  { F(WO)                                 , 0  , 0  , 0x00, 0x00, 0  , 253, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #251
  { F(RW)|F(Special)|F(Rep)|F(Repnz)      , 0  , 0  , 0x40, 0x3F, 0  , 433, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #252
  { F(WO)                                 , 0  , 1  , 0x24, 0x00, 0  , 434, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #253
  { F(WO)                                 , 0  , 1  , 0x20, 0x00, 0  , 434, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #254
  { F(WO)                                 , 0  , 1  , 0x04, 0x00, 0  , 434, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #255
  { F(WO)                                 , 0  , 1  , 0x07, 0x00, 0  , 434, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #256
  { F(WO)                                 , 0  , 1  , 0x03, 0x00, 0  , 434, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #257
  { F(WO)                                 , 0  , 1  , 0x01, 0x00, 0  , 434, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #258
  { F(WO)                                 , 0  , 1  , 0x10, 0x00, 0  , 434, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #259
  { F(WO)                                 , 0  , 1  , 0x02, 0x00, 0  , 434, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #260
  { F(RW)|F(Special)                      , 0  , 0  , 0x00, 0x3F, 0  , 164, 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #261
  { F(WO)                                 , 0  , 8  , 0x00, 0x00, 0  , 62 , 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #262
  { 0                                     , 0  , 0  , 0x00, 0x20, 0  , 263, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #263
  { 0                                     , 0  , 0  , 0x00, 0x40, 0  , 263, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #264
  { 0                                     , 0  , 0  , 0x00, 0x80, 0  , 263, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #265
  { F(Volatile)                           , 0  , 0  , 0x00, 0x00, 0  , 435, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #266
  { F(RW)|F(Special)|F(Rep)               , 0  , 0  , 0x40, 0x00, 0  , 436, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #267
  { F(RW)|F(Lock)                         , 0  , 0  , 0x00, 0x3F, 0  , 14 , 10, JUMP_TYPE(None)       , SINGLE_REG(WO)  , 0 }, // #268
  { 0                                     , 0  , 0  , 0x00, 0x00, 0  , 422, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #269
  { F(RO)                                 , 0  , 0  , 0x00, 0x3F, 77 , 88 , 5 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #270
  { 0                                     , 0  , 0  , 0x00, 0x00, 0  , 263, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #271
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 167, 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #272
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 437, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #273
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 438, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #274
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 167, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #275
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 70 , 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #276
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 64 , 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #277
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 71 , 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #278
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 170, 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #279
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 167, 3 , JUMP_TYPE(None)       , SINGLE_REG(RO)  , 0 }, // #280
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 167, 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #281
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 170, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #282
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 309, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #283
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 439, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #284
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 440, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #285
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 441, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #286
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 442, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #287
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 443, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #288
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 238, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #289
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 440, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #290
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 329, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #291
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 173, 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #292
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 444, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #293
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 445, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #294
  { F(RO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x3F, 0  , 365, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #295
  { F(RO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x3F, 0  , 366, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #296
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 176, 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #297
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 179, 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #298
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 182, 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #299
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 311, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #300
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 185, 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #301
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 182, 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #302
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 311, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #303
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 188, 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #304
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 179, 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #305
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 372, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #306
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 372, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #307
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 446, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #308
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 313, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #309
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 315, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #310
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 446, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #311
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 68 , 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #312
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 189, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #313
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 447, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #314
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 190, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #315
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 377, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #316
  { F(RW)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 191, 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #317
  { F(RW)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 448, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #318
  { F(RW)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 449, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #319
  { F(RW)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 194, 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #320
  { F(RW)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 450, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #321
  { F(RW)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 451, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #322
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 128, 4 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #323
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 317, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #324
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 319, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #325
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 452, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #326
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 453, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #327
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 454, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #328
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 182, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #329
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 62 , 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #330
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 233, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #331
  { F(RW)|F(Vex_VM)|F(Evex)               , 0  , 0  , 0x00, 0x00, 78 , 93 , 5 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #332
  { F(RW)|F(Vex_VM)|F(Evex)               , 0  , 0  , 0x00, 0x00, 79 , 98 , 5 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #333
  { F(RO)|F(VM)|F(Evex)                   , 0  , 0  , 0x00, 0x00, 0  , 455, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #334
  { F(RO)|F(VM)|F(Evex)                   , 0  , 0  , 0x00, 0x00, 0  , 456, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #335
  { F(RO)|F(VM)|F(Evex)                   , 0  , 0  , 0x00, 0x00, 0  , 457, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #336
  { F(RW)|F(Vex_VM)|F(Evex)               , 0  , 0  , 0x00, 0x00, 80 , 103, 5 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #337
  { F(RW)|F(Vex_VM)|F(Evex)               , 0  , 0  , 0x00, 0x00, 81 , 132, 4 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #338
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 62 , 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #339
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 233, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #340
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 197, 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #341
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 430, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #342
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 431, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #343
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 321, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #344
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 321, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #345
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 458, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #346
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 459, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #347
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 206, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #348
  { F(RO)|F(Vex)|F(Volatile)              , 0  , 0  , 0x00, 0x00, 0  , 396, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #349
  { F(RO)|F(Vex)|F(Special)               , 0  , 0  , 0x00, 0x00, 0  , 460, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #350
  { F(RW)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 82 , 136, 4 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #351
  { F(RW)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 83 , 136, 4 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #352
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 84 , 64 , 6 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #353
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 85 , 64 , 6 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #354
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 86 , 323, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #355
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 200, 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #356
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 87 , 64 , 4 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #357
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 88 , 64 , 6 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #358
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 89 , 64 , 6 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #359
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 90 , 64 , 4 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #360
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 91 , 64 , 6 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #361
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 92 , 64 , 6 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #362
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 93 , 64 , 6 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #363
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 94 , 64 , 6 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #364
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 214, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #365
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 95 , 325, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #366
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 96 , 325, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #367
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 97 , 325, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #368
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 98 , 325, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #369
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 461, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #370
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 203, 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #371
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 206, 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #372
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 99 , 209, 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #373
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 100, 212, 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #374
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 101, 215, 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #375
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 102, 64 , 6 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #376
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 103, 64 , 6 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #377
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 170, 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #378
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 167, 2 , JUMP_TYPE(None)       , SINGLE_REG(RO)  , 0 }, // #379
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 167, 3 , JUMP_TYPE(None)       , SINGLE_REG(RO)  , 0 }, // #380
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 167, 2 , JUMP_TYPE(None)       , SINGLE_REG(WO)  , 0 }, // #381
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 167, 3 , JUMP_TYPE(None)       , SINGLE_REG(WO)  , 0 }, // #382
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 327, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #383
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 329, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #384
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 462, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #385
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 463, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #386
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 331, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #387
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 170, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #388
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 218, 3 , JUMP_TYPE(None)       , SINGLE_REG(WO)  , 0 }, // #389
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 221, 3 , JUMP_TYPE(None)       , SINGLE_REG(WO)  , 0 }, // #390
  { F(WO)|F(Vex)|F(Special)               , 0  , 0  , 0x00, 0x00, 0  , 410, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #391
  { F(WO)|F(Vex)|F(Special)               , 0  , 0  , 0x00, 0x00, 0  , 411, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #392
  { F(WO)|F(Vex)|F(Special)               , 0  , 0  , 0x00, 0x00, 0  , 412, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #393
  { F(WO)|F(Vex)|F(Special)               , 0  , 0  , 0x00, 0x00, 0  , 413, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #394
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 171, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #395
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 145, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #396
  { F(RW)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 194, 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #397
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 140, 4 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #398
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 104, 70 , 6 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #399
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 105, 70 , 6 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #400
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 73 , 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #401
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 72 , 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #402
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 106, 144, 4 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #403
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 414, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #404
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 415, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #405
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 300, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #406
  { F(RW)|F(Vex_VM)|F(Evex)               , 0  , 0  , 0x00, 0x00, 107, 98 , 5 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #407
  { F(RW)|F(Vex_VM)|F(Evex)               , 0  , 0  , 0x00, 0x00, 108, 93 , 5 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #408
  { F(RW)|F(Vex_VM)|F(Evex)               , 0  , 0  , 0x00, 0x00, 109, 132, 4 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #409
  { F(RW)|F(Vex_VM)|F(Evex)               , 0  , 0  , 0x00, 0x00, 110, 103, 5 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #410
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 333, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #411
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 335, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #412
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 337, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #413
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 464, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #414
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 129, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #415
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 111, 136, 4 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #416
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 112, 136, 4 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #417
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 465, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #418
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 224, 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #419
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 227, 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #420
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 230, 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #421
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 233, 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #422
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 236, 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #423
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 239, 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #424
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 128, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #425
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 113, 339, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #426
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 114, 339, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #427
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 115, 339, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #428
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 116, 339, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #429
  { F(WO)|F(VM)|F(Evex)                   , 0  , 0  , 0x00, 0x00, 0  , 242, 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #430
  { F(WO)|F(VM)|F(Evex)                   , 0  , 0  , 0x00, 0x00, 0  , 341, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #431
  { F(WO)|F(VM)|F(Evex)                   , 0  , 0  , 0x00, 0x00, 0  , 245, 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #432
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 343, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #433
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 197, 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #434
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 117, 76 , 6 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #435
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 118, 76 , 6 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #436
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 119, 76 , 6 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #437
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 120, 76 , 6 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #438
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 121, 82 , 6 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #439
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 122, 76 , 6 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #440
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 123, 76 , 6 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #441
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 124, 76 , 6 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #442
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 125, 76 , 6 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #443
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 167, 3 , JUMP_TYPE(None)       , SINGLE_REG(WO)  , 0 }, // #444
  { F(RO)|F(Vex)                          , 0  , 0  , 0x00, 0x3F, 0  , 345, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #445
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 248, 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #446
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 466, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #447
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 459, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #448
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 437, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #449
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 438, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #450
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 438, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #451
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 78 , 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #452
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 466, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #453
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 459, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #454
  { F(WO)|F(VM)|F(Evex)                   , 0  , 0  , 0x00, 0x00, 0  , 347, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #455
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 171, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #456
  { F(Vex)|F(Volatile)                    , 0  , 0  , 0x00, 0x00, 0  , 435, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #457
  { F(Vex)|F(Volatile)                    , 0  , 0  , 0x00, 0x00, 0  , 263, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #458
  { F(RO)|F(Volatile)                     , 0  , 0  , 0x00, 0x00, 0  , 467, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #459
  { F(RW)|F(Xchg)|F(Lock)                 , 0  , 0  , 0x00, 0x3F, 0  , 148, 4 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #460
  { F(RW)|F(Xchg)|F(Lock)                 , 0  , 0  , 0x00, 0x00, 0  , 44 , 8 , JUMP_TYPE(None)       , SINGLE_REG(RO)  , 0 }, // #461
  { F(WO)|F(Special)                      , 0  , 0  , 0x00, 0x00, 0  , 468, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #462
  { F(RO)|F(Volatile)|F(Special)          , 0  , 0  , 0x00, 0x00, 0  , 469, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #463
  { F(RO)|F(Volatile)|F(Special)          , 0  , 0  , 0x00, 0x00, 0  , 470, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #464
  { F(WO)|F(Volatile)|F(Special)          , 0  , 0  , 0x00, 0x00, 0  , 469, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #465
  { F(WO)|F(Volatile)|F(Special)          , 0  , 0  , 0x00, 0x00, 0  , 470, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #466
  { F(RO)|F(Volatile)|F(Special)          , 0  , 0  , 0x00, 0x00, 0  , 471, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }  // #467
};
#undef SINGLE_REG
#undef JUMP_TYPE
// ----------------------------------------------------------------------------
// ${commonData:End}

// ${altOpCodeData:Begin}
// ------------------- Automatically generated, do not edit -------------------
const uint32_t X86InstDB::altOpCodeData[] = {
  0                         , // #0
  O(000F00,BA,4,_,x,_,_,_  ), // #1
  O(000F00,BA,7,_,x,_,_,_  ), // #2
  O(000F00,BA,6,_,x,_,_,_  ), // #3
  O(000F00,BA,5,_,x,_,_,_  ), // #4
  O(000000,48,_,_,x,_,_,_  ), // #5
  O(660F00,78,0,_,_,_,_,_  ), // #6
  O_FPU(00,00DF,5)          , // #7
  O_FPU(00,00DF,7)          , // #8
  O_FPU(00,00DD,1)          , // #9
  O_FPU(00,00DB,5)          , // #10
  O_FPU(00,DFE0,_)          , // #11
  O(000000,DB,7,_,_,_,_,_  ), // #12
  O_FPU(9B,DFE0,_)          , // #13
  O(000000,E4,_,_,_,_,_,_  ), // #14
  O(000000,40,_,_,x,_,_,_  ), // #15
  O(F20F00,78,_,_,_,_,_,_  ), // #16
  O(000000,77,_,_,_,_,_,_  ), // #17
  O(000000,73,_,_,_,_,_,_  ), // #18
  O(000000,72,_,_,_,_,_,_  ), // #19
  O(000000,76,_,_,_,_,_,_  ), // #20
  O(000000,74,_,_,_,_,_,_  ), // #21
  O(000000,E3,_,_,_,_,_,_  ), // #22
  O(000000,7F,_,_,_,_,_,_  ), // #23
  O(000000,7D,_,_,_,_,_,_  ), // #24
  O(000000,7C,_,_,_,_,_,_  ), // #25
  O(000000,7E,_,_,_,_,_,_  ), // #26
  O(000000,EB,_,_,_,_,_,_  ), // #27
  O(000000,75,_,_,_,_,_,_  ), // #28
  O(000000,71,_,_,_,_,_,_  ), // #29
  O(000000,7B,_,_,_,_,_,_  ), // #30
  O(000000,79,_,_,_,_,_,_  ), // #31
  O(000000,70,_,_,_,_,_,_  ), // #32
  O(000000,7A,_,_,_,_,_,_  ), // #33
  O(000000,78,_,_,_,_,_,_  ), // #34
  V(660F00,92,_,0,0,_,_,_  ), // #35
  V(F20F00,92,_,0,0,_,_,_  ), // #36
  V(F20F00,92,_,0,1,_,_,_  ), // #37
  V(000F00,92,_,0,0,_,_,_  ), // #38
  O(000000,E2,_,_,_,_,_,_  ), // #39
  O(000000,E1,_,_,_,_,_,_  ), // #40
  O(000000,E0,_,_,_,_,_,_  ), // #41
  O(660F00,29,_,_,_,_,_,_  ), // #42
  O(000F00,29,_,_,_,_,_,_  ), // #43
  O(000F38,F1,_,_,x,_,_,_  ), // #44
  O(000F00,7E,_,_,_,_,_,_  ), // #45
  O(660F00,7F,_,_,_,_,_,_  ), // #46
  O(F30F00,7F,_,_,_,_,_,_  ), // #47
  O(660F00,17,_,_,_,_,_,_  ), // #48
  O(000F00,17,_,_,_,_,_,_  ), // #49
  O(660F00,13,_,_,_,_,_,_  ), // #50
  O(000F00,13,_,_,_,_,_,_  ), // #51
  O(660F00,E7,_,_,_,_,_,_  ), // #52
  O(660F00,2B,_,_,_,_,_,_  ), // #53
  O(000F00,2B,_,_,_,_,_,_  ), // #54
  O(000F00,E7,_,_,_,_,_,_  ), // #55
  O(F20F00,2B,_,_,_,_,_,_  ), // #56
  O(F30F00,2B,_,_,_,_,_,_  ), // #57
  O(000F00,7E,_,_,x,_,_,_  ), // #58
  O(F20F00,11,_,_,_,_,_,_  ), // #59
  O(F30F00,11,_,_,_,_,_,_  ), // #60
  O(660F00,11,_,_,_,_,_,_  ), // #61
  O(000F00,11,_,_,_,_,_,_  ), // #62
  O(000000,E6,_,_,_,_,_,_  ), // #63
  O(000F3A,15,_,_,_,_,_,_  ), // #64
  O(000000,58,_,_,_,_,_,_  ), // #65
  O(000F00,72,6,_,_,_,_,_  ), // #66
  O(660F00,73,7,_,_,_,_,_  ), // #67
  O(000F00,73,6,_,_,_,_,_  ), // #68
  O(000F00,71,6,_,_,_,_,_  ), // #69
  O(000F00,72,4,_,_,_,_,_  ), // #70
  O(000F00,71,4,_,_,_,_,_  ), // #71
  O(000F00,72,2,_,_,_,_,_  ), // #72
  O(660F00,73,3,_,_,_,_,_  ), // #73
  O(000F00,73,2,_,_,_,_,_  ), // #74
  O(000F00,71,2,_,_,_,_,_  ), // #75
  O(000000,50,_,_,_,_,_,_  ), // #76
  O(000000,F6,_,_,x,_,_,_  ), // #77
  V(660F38,92,_,x,_,1,3,T1S), // #78
  V(660F38,92,_,x,_,0,2,T1S), // #79
  V(660F38,93,_,x,_,1,3,T1S), // #80
  V(660F38,93,_,x,_,0,2,T1S), // #81
  V(660F38,2F,_,x,0,_,_,_  ), // #82
  V(660F38,2E,_,x,0,_,_,_  ), // #83
  V(660F00,29,_,x,I,1,4,FVM), // #84
  V(000F00,29,_,x,I,0,4,FVM), // #85
  V(660F00,7E,_,0,0,0,2,T1S), // #86
  V(660F00,7F,_,x,I,_,_,_  ), // #87
  V(660F00,7F,_,x,_,0,4,FVM), // #88
  V(660F00,7F,_,x,_,1,4,FVM), // #89
  V(F30F00,7F,_,x,I,_,_,_  ), // #90
  V(F20F00,7F,_,x,_,1,4,FVM), // #91
  V(F30F00,7F,_,x,_,0,4,FVM), // #92
  V(F30F00,7F,_,x,_,1,4,FVM), // #93
  V(F20F00,7F,_,x,_,0,4,FVM), // #94
  V(660F00,17,_,0,I,1,3,T1S), // #95
  V(000F00,17,_,0,I,0,3,T2 ), // #96
  V(660F00,13,_,0,I,1,3,T1S), // #97
  V(000F00,13,_,0,I,0,3,T2 ), // #98
  V(660F00,7E,_,0,I,1,3,T1S), // #99
  V(F20F00,11,_,I,I,1,3,T1S), // #100
  V(F30F00,11,_,I,I,0,2,T1S), // #101
  V(660F00,11,_,x,I,1,4,FVM), // #102
  V(000F00,11,_,x,I,0,4,FVM), // #103
  V(660F3A,05,_,x,0,1,4,FV ), // #104
  V(660F3A,04,_,x,0,0,4,FV ), // #105
  V(660F3A,00,_,x,1,1,4,FV ), // #106
  V(660F38,90,_,x,_,0,2,T1S), // #107
  V(660F38,90,_,x,_,1,3,T1S), // #108
  V(660F38,91,_,x,_,0,2,T1S), // #109
  V(660F38,91,_,x,_,1,3,T1S), // #110
  V(660F38,8E,_,x,0,_,_,_  ), // #111
  V(660F38,8E,_,x,1,_,_,_  ), // #112
  V(XOP_M8,C0,_,0,x,_,_,_  ), // #113
  V(XOP_M8,C2,_,0,x,_,_,_  ), // #114
  V(XOP_M8,C3,_,0,x,_,_,_  ), // #115
  V(XOP_M8,C1,_,0,x,_,_,_  ), // #116
  V(660F00,72,6,x,I,0,4,FV ), // #117
  V(660F00,73,6,x,I,1,4,FV ), // #118
  V(660F00,71,6,x,I,I,4,FVM), // #119
  V(660F00,72,4,x,I,0,4,FV ), // #120
  V(660F00,72,4,x,_,1,4,FV ), // #121
  V(660F00,71,4,x,I,I,4,FVM), // #122
  V(660F00,72,2,x,I,0,4,FV ), // #123
  V(660F00,73,2,x,I,1,4,FV ), // #124
  V(660F00,71,2,x,I,I,4,FVM)  // #125
};
// ----------------------------------------------------------------------------
// ${altOpCodeData:End}

#undef O_FPU
#undef O
#undef V

#undef Enc
#undef EF
#undef F

// ${fpuData:Begin}
// ${fpuData:End}

// ${sseData:Begin}
// ------------------- Automatically generated, do not edit -------------------
#define FEATURE(F) X86Inst::SseData::kFeature##F
#define AVX_CONV(MODE) X86Inst::SseData::kAvxConv##MODE
const X86Inst::SseData X86InstDB::sseData[] = {
  { FEATURE(SSE2)      , AVX_CONV(Extend)    , 678  }, // #0
  { FEATURE(SSE)       , AVX_CONV(Extend)    , 678  }, // #1
  { FEATURE(SSE3)      , AVX_CONV(Extend)    , 678  }, // #2
  { FEATURE(AES)       , AVX_CONV(Extend)    , 677  }, // #3
  { FEATURE(AES)       , AVX_CONV(Move)      , 677  }, // #4
  { FEATURE(SSE2)      , AVX_CONV(Extend)    , 677  }, // #5
  { FEATURE(SSE)       , AVX_CONV(Extend)    , 677  }, // #6
  { FEATURE(SSE4_1)    , AVX_CONV(Extend)    , 677  }, // #7
  { FEATURE(SSE4_1)    , AVX_CONV(Blend)     , 677  }, // #8
  { FEATURE(SSE2)      , AVX_CONV(Extend)    , 635  }, // #9
  { FEATURE(SSE)       , AVX_CONV(Extend)    , 635  }, // #10
  { FEATURE(SSE2)      , AVX_CONV(Extend)    , 634  }, // #11
  { FEATURE(SSE)       , AVX_CONV(Extend)    , 634  }, // #12
  { FEATURE(SSE2)      , AVX_CONV(Move)      , 631  }, // #13
  { FEATURE(SSE)       , AVX_CONV(Move)      , 631  }, // #14
  { FEATURE(SSE2)      , AVX_CONV(Move)      , 630  }, // #15
  { FEATURE(SSE2)      , AVX_CONV(None)      , 0    }, // #16
  { FEATURE(SSE2)      , AVX_CONV(Move)      , 629  }, // #17
  { FEATURE(SSE)       , AVX_CONV(None)      , 0    }, // #18
  { FEATURE(SSE2)      , AVX_CONV(Move)      , 636  }, // #19
  { FEATURE(SSE2)      , AVX_CONV(Extend)    , 636  }, // #20
  { FEATURE(SSE2)      , AVX_CONV(Extend)    , 637  }, // #21
  { FEATURE(SSE)       , AVX_CONV(Extend)    , 637  }, // #22
  { FEATURE(SSE)       , AVX_CONV(Move)      , 637  }, // #23
  { FEATURE(SSE2)      , AVX_CONV(Move)      , 638  }, // #24
  { FEATURE(SSE2)      , AVX_CONV(Move)      , 640  }, // #25
  { FEATURE(SSE2)      , AVX_CONV(Move)      , 642  }, // #26
  { FEATURE(SSE)       , AVX_CONV(Move)      , 643  }, // #27
  { FEATURE(SSE2)      , AVX_CONV(Extend)    , 645  }, // #28
  { FEATURE(SSE)       , AVX_CONV(Extend)    , 645  }, // #29
  { FEATURE(SSE4_1)    , AVX_CONV(Extend)    , 645  }, // #30
  { FEATURE(SSE4_1)    , AVX_CONV(Move)      , 657  }, // #31
  { FEATURE(SSE4A)     , AVX_CONV(None)      , 0    }, // #32
  { FEATURE(SSE3)      , AVX_CONV(Extend)    , 670  }, // #33
  { FEATURE(SSE4_1)    , AVX_CONV(Extend)    , 675  }, // #34
  { FEATURE(SSE3)      , AVX_CONV(Move)      , 587  }, // #35
  { FEATURE(SSE2)      , AVX_CONV(Move)      , 579  }, // #36
  { FEATURE(MMX2)      , AVX_CONV(None)      , 0    }, // #37
  { FEATURE(SSE2)      , AVX_CONV(Extend)    , 580  }, // #38
  { FEATURE(SSE)       , AVX_CONV(Extend)    , 580  }, // #39
  { FEATURE(SSE2)      , AVX_CONV(Extend)    , 579  }, // #40
  { FEATURE(SSE)       , AVX_CONV(Extend)    , 579  }, // #41
  { FEATURE(SSE2)      , AVX_CONV(Move)      , 577  }, // #42
  { FEATURE(SSE)       , AVX_CONV(Move)      , 577  }, // #43
  { FEATURE(MMX)       |
    FEATURE(SSE2)      , AVX_CONV(Move)      , 576  }, // #44
  { FEATURE(SSE3)      , AVX_CONV(Move)      , 576  }, // #45
  { FEATURE(SSE2)      , AVX_CONV(Move)      , 575  }, // #46
  { FEATURE(SSE)       , AVX_CONV(Extend)    , 581  }, // #47
  { FEATURE(SSE2)      , AVX_CONV(MoveIfMem) , 581  }, // #48
  { FEATURE(SSE)       , AVX_CONV(MoveIfMem) , 581  }, // #49
  { FEATURE(SSE2)      , AVX_CONV(Move)      , 581  }, // #50
  { FEATURE(SSE)       , AVX_CONV(Move)      , 581  }, // #51
  { FEATURE(SSE4_1)    , AVX_CONV(Move)      , 581  }, // #52
  { FEATURE(SSE2)      , AVX_CONV(Move)      , 580  }, // #53
  { FEATURE(SSE)       , AVX_CONV(Move)      , 580  }, // #54
  { FEATURE(MMX)       |
    FEATURE(SSE2)      , AVX_CONV(Move)      , 577  }, // #55
  { FEATURE(SSE2)      , AVX_CONV(MoveIfMem) , 575  }, // #56
  { FEATURE(SSE3)      , AVX_CONV(Move)      , 575  }, // #57
  { FEATURE(SSE)       , AVX_CONV(MoveIfMem) , 575  }, // #58
  { FEATURE(SSE2)      , AVX_CONV(Move)      , 573  }, // #59
  { FEATURE(SSE)       , AVX_CONV(Move)      , 573  }, // #60
  { FEATURE(SSE4_1)    , AVX_CONV(Extend)    , 572  }, // #61
  { FEATURE(SSE2)      , AVX_CONV(Extend)    , 571  }, // #62
  { FEATURE(SSE)       , AVX_CONV(Extend)    , 571  }, // #63
  { FEATURE(SSE2)      , AVX_CONV(Extend)    , 565  }, // #64
  { FEATURE(SSE)       , AVX_CONV(Extend)    , 565  }, // #65
  { FEATURE(SSSE3)     , AVX_CONV(Move)      , 563  }, // #66
  { FEATURE(SSSE3)     , AVX_CONV(Move)      , 564  }, // #67
  { FEATURE(MMX)       |
    FEATURE(SSE2)      , AVX_CONV(Extend)    , 564  }, // #68
  { FEATURE(SSE4_1)    , AVX_CONV(Extend)    , 564  }, // #69
  { FEATURE(SSE2)      , AVX_CONV(Extend)    , 564  }, // #70
  { FEATURE(SSE3)      , AVX_CONV(Extend)    , 564  }, // #71
  { FEATURE(MMX)       |
    FEATURE(SSE2)      , AVX_CONV(Extend)    , 565  }, // #72
  { FEATURE(MMX2)      |
    FEATURE(SSE2)      , AVX_CONV(Extend)    , 567  }, // #73
  { FEATURE(3DNOW)     , AVX_CONV(None)      , 0    }, // #74
  { FEATURE(MMX2)      |
    FEATURE(SSE2)      , AVX_CONV(Extend)    , 566  }, // #75
  { FEATURE(SSE4_1)    , AVX_CONV(Blend)     , 567  }, // #76
  { FEATURE(SSE4_1)    , AVX_CONV(Extend)    , 567  }, // #77
  { FEATURE(PCLMULQDQ) , AVX_CONV(Extend)    , 573  }, // #78
  { FEATURE(MMX)       |
    FEATURE(SSE2)      , AVX_CONV(Extend)    , 576  }, // #79
  { FEATURE(SSE4_1)    , AVX_CONV(Extend)    , 576  }, // #80
  { FEATURE(SSE4_2)    , AVX_CONV(Move)      , 576  }, // #81
  { FEATURE(SSE4_1)    , AVX_CONV(Move)      , 617  }, // #82
  { FEATURE(MMX2)      |
    FEATURE(SSE2)      |
    FEATURE(SSE4_1)    , AVX_CONV(Move)      , 617  }, // #83
  { FEATURE(3DNOW2)    , AVX_CONV(None)      , 0    }, // #84
  { FEATURE(GEODE)     , AVX_CONV(None)      , 0    }, // #85
  { FEATURE(SSSE3)     , AVX_CONV(Extend)    , 603  }, // #86
  { FEATURE(SSSE3)     , AVX_CONV(Extend)    , 604  }, // #87
  { FEATURE(SSSE3)     , AVX_CONV(Extend)    , 610  }, // #88
  { FEATURE(SSE4_1)    , AVX_CONV(Move)      , 612  }, // #89
  { FEATURE(SSSE3)     , AVX_CONV(Extend)    , 613  }, // #90
  { FEATURE(SSSE3)     , AVX_CONV(Extend)    , 614  }, // #91
  { FEATURE(SSE4_1)    , AVX_CONV(Move)      , 613  }, // #92
  { FEATURE(MMX2)      |
    FEATURE(SSE2)      , AVX_CONV(Extend)    , 613  }, // #93
  { FEATURE(SSSE3)     , AVX_CONV(Extend)    , 629  }, // #94
  { FEATURE(MMX)       |
    FEATURE(SSE2)      , AVX_CONV(Extend)    , 629  }, // #95
  { FEATURE(SSE4_1)    , AVX_CONV(Extend)    , 631  }, // #96
  { FEATURE(MMX2)      |
    FEATURE(SSE2)      , AVX_CONV(Extend)    , 632  }, // #97
  { FEATURE(SSE4_1)    , AVX_CONV(Extend)    , 632  }, // #98
  { FEATURE(SSE4_1)    , AVX_CONV(Extend)    , 633  }, // #99
  { FEATURE(MMX2)      |
    FEATURE(SSE2)      , AVX_CONV(Extend)    , 634  }, // #100
  { FEATURE(SSE4_1)    , AVX_CONV(Extend)    , 634  }, // #101
  { FEATURE(SSE4_1)    , AVX_CONV(Extend)    , 635  }, // #102
  { FEATURE(MMX2)      |
    FEATURE(SSE2)      , AVX_CONV(Move)      , 643  }, // #103
  { FEATURE(SSE4_1)    , AVX_CONV(Move)      , 653  }, // #104
  { FEATURE(SSE4_1)    , AVX_CONV(Move)      , 661  }, // #105
  { FEATURE(SSE4_1)    , AVX_CONV(Extend)    , 661  }, // #106
  { FEATURE(SSSE3)     , AVX_CONV(Extend)    , 661  }, // #107
  { FEATURE(MMX2)      |
    FEATURE(SSE2)      , AVX_CONV(Extend)    , 660  }, // #108
  { FEATURE(MMX)       |
    FEATURE(SSE2)      , AVX_CONV(Extend)    , 660  }, // #109
  { FEATURE(SSE4_1)    , AVX_CONV(Extend)    , 660  }, // #110
  { FEATURE(MMX)       |
    FEATURE(SSE2)      , AVX_CONV(Extend)    , 661  }, // #111
  { FEATURE(SSE2)      , AVX_CONV(Extend)    , 662  }, // #112
  { FEATURE(MMX)       |
    FEATURE(SSE2)      , AVX_CONV(Extend)    , 655  }, // #113
  { FEATURE(MMX2)      |
    FEATURE(SSE2)      , AVX_CONV(Extend)    , 663  }, // #114
  { FEATURE(SSSE3)     , AVX_CONV(Extend)    , 675  }, // #115
  { FEATURE(SSE2)      , AVX_CONV(Move)      , 675  }, // #116
  { FEATURE(SSSE3)     , AVX_CONV(Extend)    , 674  }, // #117
  { FEATURE(MMX)       |
    FEATURE(SSE2)      , AVX_CONV(Extend)    , 674  }, // #118
  { FEATURE(SSE2)      , AVX_CONV(Extend)    , 674  }, // #119
  { FEATURE(MMX)       |
    FEATURE(SSE2)      , AVX_CONV(Extend)    , 677  }, // #120
  { FEATURE(MMX)       |
    FEATURE(SSE2)      , AVX_CONV(Extend)    , 681  }, // #121
  { FEATURE(SSE2)      , AVX_CONV(Extend)    , 681  }, // #122
  { FEATURE(MMX)       |
    FEATURE(SSE2)      , AVX_CONV(Extend)    , 684  }, // #123
  { FEATURE(SSE2)      , AVX_CONV(Extend)    , 684  }, // #124
  { FEATURE(SSE4_1)    , AVX_CONV(Move)      , 685  }, // #125
  { FEATURE(MMX)       |
    FEATURE(SSE2)      , AVX_CONV(Extend)    , 693  }, // #126
  { FEATURE(SSE2)      , AVX_CONV(Extend)    , 693  }, // #127
  { FEATURE(MMX)       |
    FEATURE(SSE2)      , AVX_CONV(Extend)    , 687  }, // #128
  { FEATURE(SSE)       , AVX_CONV(Move)      , 700  }, // #129
  { FEATURE(SSE)       , AVX_CONV(Extend)    , 700  }, // #130
  { FEATURE(SSE4_1)    , AVX_CONV(Move)      , 697  }, // #131
  { FEATURE(SSE4_1)    , AVX_CONV(Extend)    , 697  }, // #132
  { FEATURE(SSE)       , AVX_CONV(Move)      , 705  }, // #133
  { FEATURE(SSE)       , AVX_CONV(Extend)    , 705  }, // #134
  { FEATURE(SHA)       , AVX_CONV(None)      , 0    }, // #135
  { FEATURE(SSE2)      , AVX_CONV(Extend)    , 675  }, // #136
  { FEATURE(SSE)       , AVX_CONV(Extend)    , 675  }, // #137
  { FEATURE(SSE)       , AVX_CONV(Move)      , 675  }, // #138
  { FEATURE(SSE2)      , AVX_CONV(Extend)    , 669  }, // #139
  { FEATURE(SSE)       , AVX_CONV(Extend)    , 669  }, // #140
  { FEATURE(SSE2)      , AVX_CONV(Move)      , 666  }, // #141
  { FEATURE(SSE)       , AVX_CONV(Move)      , 666  }, // #142
  { FEATURE(SSE2)      , AVX_CONV(Extend)    , 665  }, // #143
  { FEATURE(SSE)       , AVX_CONV(Extend)    , 665  }, // #144
  { FEATURE(SSE2)      , AVX_CONV(Extend)    , -10  }, // #145
  { FEATURE(SSE)       , AVX_CONV(Extend)    , -10  }  // #146
};
#undef AVX_CONV
#undef FEATURE
// ----------------------------------------------------------------------------
// ${sseData:End}

// ${avxData:Begin}
// ------------------- Automatically generated, do not edit -------------------
#define FEATURE(F) X86Inst::AvxData::kFeature##F
#define FLAG(F) X86Inst::AvxData::kFlag##F
const X86Inst::AvxData X86InstDB::avxData[] = {
  { FEATURE(AVX)       |
    FEATURE(AVX512_F)  |
    FEATURE(AVX512_VL) , FLAG(Broadcast64) | FLAG(ER) | FLAG(Masking) | FLAG(SAE) | FLAG(Zeroing) }, // #0
  { FEATURE(AVX)       |
    FEATURE(AVX512_F)  |
    FEATURE(AVX512_VL) , FLAG(Broadcast32) | FLAG(ER) | FLAG(Masking) | FLAG(SAE) | FLAG(Zeroing) }, // #1
  { FEATURE(AVX)       |
    FEATURE(AVX512_F)  , FLAG(ER) | FLAG(Masking) | FLAG(SAE) | FLAG(Zeroing) }, // #2
  { FEATURE(AVX)       , 0 }, // #3
  { FEATURE(AES)       |
    FEATURE(AVX)       , 0 }, // #4
  { FEATURE(AVX512_F)  |
    FEATURE(AVX512_VL) , FLAG(Broadcast32) | FLAG(Masking) | FLAG(Zeroing) }, // #5
  { FEATURE(AVX512_F)  |
    FEATURE(AVX512_VL) , FLAG(Broadcast64) | FLAG(Masking) | FLAG(Zeroing) }, // #6
  { FEATURE(AVX)       |
    FEATURE(AVX512_DQ) |
    FEATURE(AVX512_VL) , FLAG(Broadcast64) | FLAG(Masking) | FLAG(Zeroing) }, // #7
  { FEATURE(AVX)       |
    FEATURE(AVX512_DQ) |
    FEATURE(AVX512_VL) , FLAG(Broadcast32) | FLAG(Masking) | FLAG(Zeroing) }, // #8
  { FEATURE(AVX512_BW) |
    FEATURE(AVX512_VL) , FLAG(Masking) | FLAG(Zeroing) }, // #9
  { FEATURE(AVX512_DQ) |
    FEATURE(AVX512_VL) , FLAG(Masking) | FLAG(Zeroing) }, // #10
  { FEATURE(AVX512_F)  , FLAG(Masking) | FLAG(Zeroing) }, // #11
  { FEATURE(AVX512_DQ) , FLAG(Masking) | FLAG(Zeroing) }, // #12
  { FEATURE(AVX2)      , 0 }, // #13
  { FEATURE(AVX512_F)  |
    FEATURE(AVX512_VL) , FLAG(Masking) | FLAG(Zeroing) }, // #14
  { FEATURE(AVX)       |
    FEATURE(AVX2)      |
    FEATURE(AVX512_F)  |
    FEATURE(AVX512_VL) , FLAG(Masking) | FLAG(Zeroing) }, // #15
  { FEATURE(AVX)       |
    FEATURE(AVX512_F)  |
    FEATURE(AVX512_VL) , FLAG(Broadcast64) | FLAG(Masking) | FLAG(SAE) | FLAG(Zeroing) }, // #16
  { FEATURE(AVX)       |
    FEATURE(AVX512_F)  |
    FEATURE(AVX512_VL) , FLAG(Broadcast32) | FLAG(Masking) | FLAG(SAE) | FLAG(Zeroing) }, // #17
  { FEATURE(AVX)       |
    FEATURE(AVX512_F)  , FLAG(Masking) | FLAG(SAE) | FLAG(Zeroing) }, // #18
  { FEATURE(AVX)       |
    FEATURE(AVX512_F)  , FLAG(SAE) }, // #19
  { FEATURE(AVX)       |
    FEATURE(AVX512_F)  |
    FEATURE(AVX512_VL) , FLAG(Broadcast32) | FLAG(Masking) | FLAG(Zeroing) }, // #20
  { FEATURE(AVX512_DQ) |
    FEATURE(AVX512_VL) , FLAG(Broadcast64) | FLAG(ER) | FLAG(Masking) | FLAG(SAE) | FLAG(Zeroing) }, // #21
  { FEATURE(AVX512_F)  |
    FEATURE(AVX512_VL) , FLAG(Broadcast64) | FLAG(ER) | FLAG(Masking) | FLAG(SAE) | FLAG(Zeroing) }, // #22
  { FEATURE(AVX512_F)  |
    FEATURE(AVX512_VL) |
    FEATURE(F16C)      , FLAG(Masking) | FLAG(SAE) | FLAG(Zeroing) }, // #23
  { FEATURE(AVX512_DQ) |
    FEATURE(AVX512_VL) , FLAG(Broadcast32) | FLAG(ER) | FLAG(Masking) | FLAG(SAE) | FLAG(Zeroing) }, // #24
  { FEATURE(AVX512_F)  |
    FEATURE(AVX512_VL) , FLAG(Broadcast32) | FLAG(ER) | FLAG(Masking) | FLAG(SAE) | FLAG(Zeroing) }, // #25
  { FEATURE(AVX)       |
    FEATURE(AVX512_F)  , FLAG(ER) | FLAG(SAE) }, // #26
  { FEATURE(AVX512_F)  , FLAG(ER) | FLAG(SAE) }, // #27
  { FEATURE(AVX512_F)  |
    FEATURE(AVX512_VL) , FLAG(Broadcast64) | FLAG(Masking) | FLAG(SAE) | FLAG(Zeroing) }, // #28
  { FEATURE(AVX512_DQ) |
    FEATURE(AVX512_VL) , FLAG(Broadcast64) | FLAG(Masking) | FLAG(SAE) | FLAG(Zeroing) }, // #29
  { FEATURE(AVX512_DQ) |
    FEATURE(AVX512_VL) , FLAG(Broadcast32) | FLAG(Masking) | FLAG(SAE) | FLAG(Zeroing) }, // #30
  { FEATURE(AVX512_F)  |
    FEATURE(AVX512_VL) , FLAG(Broadcast32) | FLAG(Masking) | FLAG(SAE) | FLAG(Zeroing) }, // #31
  { FEATURE(AVX512_F)  , FLAG(SAE) }, // #32
  { FEATURE(AVX512_ERI), FLAG(Broadcast64) | FLAG(Masking) | FLAG(SAE) | FLAG(Zeroing) }, // #33
  { FEATURE(AVX512_ERI), FLAG(Broadcast32) | FLAG(Masking) | FLAG(SAE) | FLAG(Zeroing) }, // #34
  { FEATURE(AVX)       |
    FEATURE(AVX512_F)  , 0 }, // #35
  { FEATURE(AVX512_F)  , FLAG(Masking) | FLAG(SAE) | FLAG(Zeroing) }, // #36
  { FEATURE(AVX512_F)  |
    FEATURE(AVX512_VL) |
    FEATURE(FMA)       , FLAG(Broadcast64) | FLAG(ER) | FLAG(Masking) | FLAG(SAE) | FLAG(Zeroing) }, // #37
  { FEATURE(AVX512_F)  |
    FEATURE(AVX512_VL) |
    FEATURE(FMA)       , FLAG(Broadcast32) | FLAG(ER) | FLAG(Masking) | FLAG(SAE) | FLAG(Zeroing) }, // #38
  { FEATURE(AVX512_F)  |
    FEATURE(FMA)       , FLAG(ER) | FLAG(Masking) | FLAG(SAE) | FLAG(Zeroing) }, // #39
  { FEATURE(FMA4)      , 0 }, // #40
  { FEATURE(AVX512_DQ) |
    FEATURE(AVX512_VL) , FLAG(Broadcast64) | FLAG(Masking) }, // #41
  { FEATURE(AVX512_DQ) |
    FEATURE(AVX512_VL) , FLAG(Broadcast32) | FLAG(Masking) }, // #42
  { FEATURE(AVX512_DQ) , FLAG(Masking) }, // #43
  { FEATURE(XOP)       , 0 }, // #44
  { FEATURE(AVX2)      |
    FEATURE(AVX512_F)  |
    FEATURE(AVX512_VL) , FLAG(Masking) }, // #45
  { FEATURE(AVX)       |
    FEATURE(AVX512_F)  |
    FEATURE(AVX512_VL) , FLAG(Masking) | FLAG(SAE) | FLAG(Zeroing) }, // #46
  { FEATURE(AVX)       |
    FEATURE(AVX512_F)  |
    FEATURE(AVX512_VL) , FLAG(Masking) | FLAG(Zeroing) }, // #47
  { FEATURE(AVX)       |
    FEATURE(AVX512_F)  |
    FEATURE(AVX512_VL) , 0 }, // #48
  { FEATURE(AVX)       |
    FEATURE(AVX2)      |
    FEATURE(AVX512_F)  |
    FEATURE(AVX512_VL) , 0 }, // #49
  { FEATURE(AVX)       |
    FEATURE(AVX512_F)  , FLAG(Masking) | FLAG(Zeroing) }, // #50
  { FEATURE(AVX)       |
    FEATURE(AVX2)      , 0 }, // #51
  { FEATURE(AVX)       |
    FEATURE(AVX2)      |
    FEATURE(AVX512_F)  |
    FEATURE(AVX512_VL) , FLAG(Broadcast64) | FLAG(ER) | FLAG(Masking) | FLAG(SAE) | FLAG(Zeroing) }, // #52
  { FEATURE(AVX)       |
    FEATURE(AVX2)      |
    FEATURE(AVX512_F)  |
    FEATURE(AVX512_VL) , FLAG(Broadcast32) | FLAG(ER) | FLAG(Masking) | FLAG(SAE) | FLAG(Zeroing) }, // #53
  { FEATURE(AVX)       |
    FEATURE(AVX2)      |
    FEATURE(AVX512_BW) |
    FEATURE(AVX512_VL) , FLAG(Masking) | FLAG(Zeroing) }, // #54
  { FEATURE(AVX)       |
    FEATURE(AVX2)      |
    FEATURE(AVX512_BW) |
    FEATURE(AVX512_VL) , FLAG(Broadcast32) | FLAG(Masking) | FLAG(Zeroing) }, // #55
  { FEATURE(AVX)       |
    FEATURE(AVX2)      |
    FEATURE(AVX512_F)  |
    FEATURE(AVX512_VL) , FLAG(Broadcast32) | FLAG(Masking) | FLAG(Zeroing) }, // #56
  { FEATURE(AVX)       |
    FEATURE(AVX2)      |
    FEATURE(AVX512_F)  |
    FEATURE(AVX512_VL) , FLAG(Broadcast64) | FLAG(Masking) | FLAG(Zeroing) }, // #57
  { FEATURE(AVX)       |
    FEATURE(AVX512_BW) |
    FEATURE(AVX512_VL) , FLAG(Masking) | FLAG(Zeroing) }, // #58
  { FEATURE(AVX2)      |
    FEATURE(AVX512_BW) |
    FEATURE(AVX512_VL) , FLAG(Masking) | FLAG(Zeroing) }, // #59
  { FEATURE(AVX2)      |
    FEATURE(AVX512_F)  |
    FEATURE(AVX512_VL) , FLAG(Masking) | FLAG(Zeroing) }, // #60
  { FEATURE(AVX512_CDI)|
    FEATURE(AVX512_VL) , 0 }, // #61
  { FEATURE(AVX)       |
    FEATURE(PCLMULQDQ) , 0 }, // #62
  { FEATURE(AVX512_BW) |
    FEATURE(AVX512_VL) , FLAG(Masking) }, // #63
  { FEATURE(AVX512_F)  |
    FEATURE(AVX512_VL) , FLAG(Broadcast32) | FLAG(Masking) }, // #64
  { FEATURE(AVX)       |
    FEATURE(AVX2)      |
    FEATURE(AVX512_BW) |
    FEATURE(AVX512_VL) , FLAG(Masking) }, // #65
  { FEATURE(AVX)       |
    FEATURE(AVX2)      |
    FEATURE(AVX512_F)  |
    FEATURE(AVX512_VL) , FLAG(Broadcast32) | FLAG(Masking) }, // #66
  { FEATURE(AVX)       |
    FEATURE(AVX2)      |
    FEATURE(AVX512_F)  |
    FEATURE(AVX512_VL) , FLAG(Broadcast64) | FLAG(Masking) }, // #67
  { FEATURE(AVX512_F)  |
    FEATURE(AVX512_VL) , FLAG(Broadcast64) | FLAG(Masking) }, // #68
  { FEATURE(AVX512_BW) |
    FEATURE(AVX512_VL) , FLAG(Broadcast64) | FLAG(Masking) }, // #69
  { FEATURE(AVX512_CDI)|
    FEATURE(AVX512_VL) , FLAG(Broadcast32) | FLAG(Masking) | FLAG(Zeroing) }, // #70
  { FEATURE(AVX512_VBMI)|
    FEATURE(AVX512_VL) , FLAG(Masking) | FLAG(Zeroing) }, // #71
  { FEATURE(AVX2)      |
    FEATURE(AVX512_F)  |
    FEATURE(AVX512_VL) , FLAG(Broadcast32) | FLAG(Masking) | FLAG(Zeroing) }, // #72
  { FEATURE(AVX)       |
    FEATURE(AVX512_F)  |
    FEATURE(AVX512_VL) , FLAG(Broadcast64) | FLAG(Masking) | FLAG(Zeroing) }, // #73
  { FEATURE(AVX2)      |
    FEATURE(AVX512_F)  |
    FEATURE(AVX512_VL) , FLAG(Broadcast64) | FLAG(Masking) | FLAG(Zeroing) }, // #74
  { FEATURE(AVX)       |
    FEATURE(AVX512_BW) , 0 }, // #75
  { FEATURE(AVX)       |
    FEATURE(AVX512_DQ) , 0 }, // #76
  { FEATURE(AVX)       |
    FEATURE(AVX512_BW) , FLAG(Masking) | FLAG(Zeroing) }, // #77
  { FEATURE(AVX)       |
    FEATURE(AVX512_DQ) , FLAG(Masking) | FLAG(Zeroing) }, // #78
  { FEATURE(AVX512_CDI)|
    FEATURE(AVX512_VL) , FLAG(Broadcast64) | FLAG(Masking) | FLAG(Zeroing) }, // #79
  { FEATURE(AVX512_IFMA)|
    FEATURE(AVX512_VL) , FLAG(Broadcast64) | FLAG(Masking) | FLAG(Zeroing) }, // #80
  { FEATURE(AVX512_BW) |
    FEATURE(AVX512_VL) , 0 }, // #81
  { FEATURE(AVX512_DQ) |
    FEATURE(AVX512_VL) , 0 }, // #82
  { FEATURE(AVX512_DQ) |
    FEATURE(AVX512_VL) , FLAG(Broadcast64) | FLAG(Masking) | FLAG(Zeroing) }, // #83
  { FEATURE(AVX512_VBMI)|
    FEATURE(AVX512_VL) , FLAG(Broadcast64) | FLAG(Masking) | FLAG(Zeroing) }, // #84
  { FEATURE(AVX)       |
    FEATURE(AVX2)      |
    FEATURE(AVX512_BW) |
    FEATURE(AVX512_VL) , 0 }, // #85
  { FEATURE(AVX512_F)  |
    FEATURE(AVX512_VL) , FLAG(Masking) }, // #86
  { FEATURE(AVX512_DQ) , FLAG(Masking) | FLAG(SAE) | FLAG(Zeroing) }, // #87
  { FEATURE(AVX512_ERI), FLAG(Masking) | FLAG(SAE) | FLAG(Zeroing) }, // #88
  { FEATURE(AVX512_DQ) |
    FEATURE(AVX512_VL) , FLAG(Broadcast32) | FLAG(Masking) | FLAG(Zeroing) }, // #89
  { FEATURE(AVX512_F)  , FLAG(ER) | FLAG(Masking) | FLAG(SAE) | FLAG(Zeroing) }  // #90
};
#undef FLAG
#undef FEATURE
// ----------------------------------------------------------------------------
// ${avxData:End}

// ============================================================================
// [asmjit::X86Inst - Id <-> Name]
// ============================================================================

#if !defined(ASMJIT_DISABLE_TEXT)
// ${nameData:Begin}
// ------------------- Automatically generated, do not edit -------------------
const char X86InstDB::nameData[] =
  "\0" "aaa\0" "aad\0" "aam\0" "aas\0" "adc\0" "adcx\0" "adox\0" "bextr\0"
  "blcfill\0" "blci\0" "blcic\0" "blcmsk\0" "blcs\0" "blsfill\0" "blsi\0"
  "blsic\0" "blsmsk\0" "blsr\0" "bsf\0" "bsr\0" "bswap\0" "bt\0" "btc\0"
  "btr\0" "bts\0" "bzhi\0" "call\0" "cbw\0" "cdq\0" "cdqe\0" "clac\0" "clc\0"
  "cld\0" "clflush\0" "clflushopt\0" "clwb\0" "clzero\0" "cmc\0" "cmova\0"
  "cmovae\0" "cmovc\0" "cmovg\0" "cmovge\0" "cmovl\0" "cmovle\0" "cmovna\0"
  "cmovnae\0" "cmovnc\0" "cmovng\0" "cmovnge\0" "cmovnl\0" "cmovnle\0"
  "cmovno\0" "cmovnp\0" "cmovns\0" "cmovnz\0" "cmovo\0" "cmovp\0" "cmovpe\0"
  "cmovpo\0" "cmovs\0" "cmovz\0" "cmp\0" "cmps\0" "cmpxchg\0" "cmpxchg16b\0"
  "cmpxchg8b\0" "cpuid\0" "cqo\0" "crc32\0" "cvtpd2pi\0" "cvtpi2pd\0"
  "cvtpi2ps\0" "cvtps2pi\0" "cvttpd2pi\0" "cvttps2pi\0" "cwd\0" "cwde\0"
  "daa\0" "das\0" "enter\0" "f2xm1\0" "fabs\0" "faddp\0" "fbld\0" "fbstp\0"
  "fchs\0" "fclex\0" "fcmovb\0" "fcmovbe\0" "fcmove\0" "fcmovnb\0" "fcmovnbe\0"
  "fcmovne\0" "fcmovnu\0" "fcmovu\0" "fcom\0" "fcomi\0" "fcomip\0" "fcomp\0"
  "fcompp\0" "fcos\0" "fdecstp\0" "fdiv\0" "fdivp\0" "fdivr\0" "fdivrp\0"
  "femms\0" "ffree\0" "fiadd\0" "ficom\0" "ficomp\0" "fidiv\0" "fidivr\0"
  "fild\0" "fimul\0" "fincstp\0" "finit\0" "fist\0" "fistp\0" "fisttp\0"
  "fisub\0" "fisubr\0" "fld\0" "fld1\0" "fldcw\0" "fldenv\0" "fldl2e\0"
  "fldl2t\0" "fldlg2\0" "fldln2\0" "fldpi\0" "fldz\0" "fmulp\0" "fnclex\0"
  "fninit\0" "fnop\0" "fnsave\0" "fnstcw\0" "fnstenv\0" "fnstsw\0" "fpatan\0"
  "fprem\0" "fprem1\0" "fptan\0" "frndint\0" "frstor\0" "fsave\0" "fscale\0"
  "fsin\0" "fsincos\0" "fsqrt\0" "fst\0" "fstcw\0" "fstenv\0" "fstp\0"
  "fstsw\0" "fsubp\0" "fsubrp\0" "ftst\0" "fucom\0" "fucomi\0" "fucomip\0"
  "fucomp\0" "fucompp\0" "fwait\0" "fxam\0" "fxch\0" "fxrstor\0" "fxrstor64\0"
  "fxsave\0" "fxsave64\0" "fxtract\0" "fyl2x\0" "fyl2xp1\0" "inc\0" "ins\0"
  "insertq\0" "int3\0" "into\0" "ja\0" "jae\0" "jb\0" "jbe\0" "jc\0" "je\0"
  "jecxz\0" "jg\0" "jge\0" "jl\0" "jle\0" "jmp\0" "jna\0" "jnae\0" "jnb\0"
  "jnbe\0" "jnc\0" "jne\0" "jng\0" "jnge\0" "jnl\0" "jnle\0" "jno\0" "jnp\0"
  "jns\0" "jnz\0" "jo\0" "jp\0" "jpe\0" "jpo\0" "js\0" "jz\0" "kaddb\0"
  "kaddd\0" "kaddq\0" "kaddw\0" "kandb\0" "kandd\0" "kandnb\0" "kandnd\0"
  "kandnq\0" "kandnw\0" "kandq\0" "kandw\0" "kmovb\0" "kmovw\0" "knotb\0"
  "knotd\0" "knotq\0" "knotw\0" "korb\0" "kord\0" "korq\0" "kortestb\0"
  "kortestd\0" "kortestq\0" "kortestw\0" "korw\0" "kshiftlb\0" "kshiftld\0"
  "kshiftlq\0" "kshiftlw\0" "kshiftrb\0" "kshiftrd\0" "kshiftrq\0" "kshiftrw\0"
  "ktestb\0" "ktestd\0" "ktestq\0" "ktestw\0" "kunpckbw\0" "kunpckdq\0"
  "kunpckwd\0" "kxnorb\0" "kxnord\0" "kxnorq\0" "kxnorw\0" "kxorb\0" "kxord\0"
  "kxorq\0" "kxorw\0" "lahf\0" "lea\0" "leave\0" "lfence\0" "lods\0" "loop\0"
  "loope\0" "loopne\0" "lzcnt\0" "mfence\0" "monitor\0" "movdq2q\0" "movnti\0"
  "movntq\0" "movntsd\0" "movntss\0" "movq2dq\0" "movsx\0" "movsxd\0" "movzx\0"
  "mulx\0" "mwait\0" "neg\0" "not\0" "out\0" "outs\0" "pause\0" "pavgusb\0"
  "pcommit\0" "pdep\0" "pext\0" "pf2id\0" "pf2iw\0" "pfacc\0" "pfadd\0"
  "pfcmpeq\0" "pfcmpge\0" "pfcmpgt\0" "pfmax\0" "pfmin\0" "pfmul\0" "pfnacc\0"
  "pfpnacc\0" "pfrcp\0" "pfrcpit1\0" "pfrcpit2\0" "pfrcpv\0" "pfrsqit1\0"
  "pfrsqrt\0" "pfrsqrtv\0" "pfsub\0" "pfsubr\0" "pi2fd\0" "pi2fw\0" "pmulhrw\0"
  "pop\0" "popa\0" "popad\0" "popcnt\0" "popf\0" "popfd\0" "popfq\0"
  "prefetch\0" "prefetchnta\0" "prefetcht0\0" "prefetcht1\0" "prefetcht2\0"
  "prefetchw\0" "prefetchwt1\0" "pshufw\0" "pswapd\0" "push\0" "pusha\0"
  "pushad\0" "pushf\0" "pushfd\0" "pushfq\0" "rcl\0" "rcr\0" "rdfsbase\0"
  "rdgsbase\0" "rdrand\0" "rdseed\0" "rdtsc\0" "rdtscp\0" "ret\0" "rol\0"
  "ror\0" "rorx\0" "sahf\0" "sal\0" "sar\0" "sarx\0" "sbb\0" "scas\0" "seta\0"
  "setae\0" "setb\0" "setbe\0" "setc\0" "sete\0" "setg\0" "setge\0" "setl\0"
  "setle\0" "setna\0" "setnae\0" "setnb\0" "setnbe\0" "setnc\0" "setne\0"
  "setng\0" "setnge\0" "setnl\0" "setnle\0" "setno\0" "setnp\0" "setns\0"
  "setnz\0" "seto\0" "setp\0" "setpe\0" "setpo\0" "sets\0" "setz\0" "sfence\0"
  "sha1msg1\0" "sha1msg2\0" "sha1nexte\0" "sha1rnds4\0" "sha256msg1\0"
  "sha256msg2\0" "sha256rnds2\0" "shl\0" "shlx\0" "shr\0" "shrd\0" "shrx\0"
  "stac\0" "stc\0" "sti\0" "stos\0" "swapgs\0" "t1mskc\0" "tzcnt\0" "tzmsk\0"
  "ud2\0" "vaddpd\0" "vaddps\0" "vaddsd\0" "vaddss\0" "vaddsubpd\0"
  "vaddsubps\0" "vaesdec\0" "vaesdeclast\0" "vaesenc\0" "vaesenclast\0"
  "vaesimc\0" "vaeskeygenassist\0" "valignd\0" "valignq\0" "vandnpd\0"
  "vandnps\0" "vandpd\0" "vandps\0" "vblendmb\0" "vblendmd\0" "vblendmpd\0"
  "vblendmps\0" "vblendmq\0" "vblendmw\0" "vblendpd\0" "vblendps\0"
  "vblendvpd\0" "vblendvps\0" "vbroadcastf128\0" "vbroadcastf32x2\0"
  "vbroadcastf32x4\0" "vbroadcastf32x8\0" "vbroadcastf64x2\0"
  "vbroadcastf64x4\0" "vbroadcasti128\0" "vbroadcasti32x2\0"
  "vbroadcasti32x4\0" "vbroadcasti32x8\0" "vbroadcasti64x2\0"
  "vbroadcasti64x4\0" "vbroadcastsd\0" "vbroadcastss\0" "vcmppd\0" "vcmpps\0"
  "vcmpsd\0" "vcmpss\0" "vcomisd\0" "vcomiss\0" "vcompresspd\0" "vcompressps\0"
  "vcvtdq2pd\0" "vcvtdq2ps\0" "vcvtpd2dq\0" "vcvtpd2ps\0" "vcvtpd2qq\0"
  "vcvtpd2udq\0" "vcvtpd2uqq\0" "vcvtph2ps\0" "vcvtps2dq\0" "vcvtps2pd\0"
  "vcvtps2ph\0" "vcvtps2qq\0" "vcvtps2udq\0" "vcvtps2uqq\0" "vcvtqq2pd\0"
  "vcvtqq2ps\0" "vcvtsd2si\0" "vcvtsd2ss\0" "vcvtsd2usi\0" "vcvtsi2sd\0"
  "vcvtsi2ss\0" "vcvtss2sd\0" "vcvtss2si\0" "vcvtss2usi\0" "vcvttpd2dq\0"
  "vcvttpd2qq\0" "vcvttpd2udq\0" "vcvttpd2uqq\0" "vcvttps2dq\0" "vcvttps2qq\0"
  "vcvttps2udq\0" "vcvttps2uqq\0" "vcvttsd2si\0" "vcvttsd2usi\0" "vcvttss2si\0"
  "vcvttss2usi\0" "vcvtudq2pd\0" "vcvtudq2ps\0" "vcvtuqq2pd\0" "vcvtuqq2ps\0"
  "vcvtusi2sd\0" "vcvtusi2ss\0" "vdbpsadbw\0" "vdivpd\0" "vdivps\0" "vdivsd\0"
  "vdivss\0" "vdppd\0" "vdpps\0" "vexp2pd\0" "vexp2ps\0" "vexpandpd\0"
  "vexpandps\0" "vextractf128\0" "vextractf32x4\0" "vextractf32x8\0"
  "vextractf64x2\0" "vextractf64x4\0" "vextracti128\0" "vextracti32x4\0"
  "vextracti32x8\0" "vextracti64x2\0" "vextracti64x4\0" "vextractps\0"
  "vfixupimmpd\0" "vfixupimmps\0" "vfixupimmsd\0" "vfixupimmss\0"
  "vfmadd132pd\0" "vfmadd132ps\0" "vfmadd132sd\0" "vfmadd132ss\0"
  "vfmadd213pd\0" "vfmadd213ps\0" "vfmadd213sd\0" "vfmadd213ss\0"
  "vfmadd231pd\0" "vfmadd231ps\0" "vfmadd231sd\0" "vfmadd231ss\0" "vfmaddpd\0"
  "vfmaddps\0" "vfmaddsd\0" "vfmaddss\0" "vfmaddsub132pd\0" "vfmaddsub132ps\0"
  "vfmaddsub213pd\0" "vfmaddsub213ps\0" "vfmaddsub231pd\0" "vfmaddsub231ps\0"
  "vfmaddsubpd\0" "vfmaddsubps\0" "vfmsub132pd\0" "vfmsub132ps\0"
  "vfmsub132sd\0" "vfmsub132ss\0" "vfmsub213pd\0" "vfmsub213ps\0"
  "vfmsub213sd\0" "vfmsub213ss\0" "vfmsub231pd\0" "vfmsub231ps\0"
  "vfmsub231sd\0" "vfmsub231ss\0" "vfmsubadd132pd\0" "vfmsubadd132ps\0"
  "vfmsubadd213pd\0" "vfmsubadd213ps\0" "vfmsubadd231pd\0" "vfmsubadd231ps\0"
  "vfmsubaddpd\0" "vfmsubaddps\0" "vfmsubpd\0" "vfmsubps\0" "vfmsubsd\0"
  "vfmsubss\0" "vfnmadd132pd\0" "vfnmadd132ps\0" "vfnmadd132sd\0"
  "vfnmadd132ss\0" "vfnmadd213pd\0" "vfnmadd213ps\0" "vfnmadd213sd\0"
  "vfnmadd213ss\0" "vfnmadd231pd\0" "vfnmadd231ps\0" "vfnmadd231sd\0"
  "vfnmadd231ss\0" "vfnmaddpd\0" "vfnmaddps\0" "vfnmaddsd\0" "vfnmaddss\0"
  "vfnmsub132pd\0" "vfnmsub132ps\0" "vfnmsub132sd\0" "vfnmsub132ss\0"
  "vfnmsub213pd\0" "vfnmsub213ps\0" "vfnmsub213sd\0" "vfnmsub213ss\0"
  "vfnmsub231pd\0" "vfnmsub231ps\0" "vfnmsub231sd\0" "vfnmsub231ss\0"
  "vfnmsubpd\0" "vfnmsubps\0" "vfnmsubsd\0" "vfnmsubss\0" "vfpclasspd\0"
  "vfpclassps\0" "vfpclasssd\0" "vfpclassss\0" "vfrczpd\0" "vfrczps\0"
  "vfrczsd\0" "vfrczss\0" "vgatherdpd\0" "vgatherdps\0" "vgatherpf0dpd\0"
  "vgatherpf0dps\0" "vgatherpf0qpd\0" "vgatherpf0qps\0" "vgatherpf1dpd\0"
  "vgatherpf1dps\0" "vgatherpf1qpd\0" "vgatherpf1qps\0" "vgatherqpd\0"
  "vgatherqps\0" "vgetexppd\0" "vgetexpps\0" "vgetexpsd\0" "vgetexpss\0"
  "vgetmantpd\0" "vgetmantps\0" "vgetmantsd\0" "vgetmantss\0" "vhaddpd\0"
  "vhaddps\0" "vhsubpd\0" "vhsubps\0" "vinsertf128\0" "vinsertf32x4\0"
  "vinsertf32x8\0" "vinsertf64x2\0" "vinsertf64x4\0" "vinserti128\0"
  "vinserti32x4\0" "vinserti32x8\0" "vinserti64x2\0" "vinserti64x4\0"
  "vinsertps\0" "vlddqu\0" "vldmxcsr\0" "vmaskmovdqu\0" "vmaskmovpd\0"
  "vmaskmovps\0" "vmaxpd\0" "vmaxps\0" "vmaxsd\0" "vmaxss\0" "vminpd\0"
  "vminps\0" "vminsd\0" "vminss\0" "vmovapd\0" "vmovaps\0" "vmovd\0"
  "vmovddup\0" "vmovdqa\0" "vmovdqa32\0" "vmovdqa64\0" "vmovdqu\0"
  "vmovdqu16\0" "vmovdqu32\0" "vmovdqu64\0" "vmovdqu8\0" "vmovhlps\0"
  "vmovhpd\0" "vmovhps\0" "vmovlhps\0" "vmovlpd\0" "vmovlps\0" "vmovmskpd\0"
  "vmovmskps\0" "vmovntdq\0" "vmovntdqa\0" "vmovntpd\0" "vmovntps\0" "vmovq\0"
  "vmovsd\0" "vmovshdup\0" "vmovsldup\0" "vmovss\0" "vmovupd\0" "vmovups\0"
  "vmpsadbw\0" "vmulpd\0" "vmulps\0" "vmulsd\0" "vmulss\0" "vorpd\0" "vorps\0"
  "vpabsb\0" "vpabsd\0" "vpabsq\0" "vpabsw\0" "vpackssdw\0" "vpacksswb\0"
  "vpackusdw\0" "vpackuswb\0" "vpaddb\0" "vpaddd\0" "vpaddq\0" "vpaddsb\0"
  "vpaddsw\0" "vpaddusb\0" "vpaddusw\0" "vpaddw\0" "vpalignr\0" "vpand\0"
  "vpandd\0" "vpandn\0" "vpandnd\0" "vpandnq\0" "vpandq\0" "vpavgb\0"
  "vpavgw\0" "vpblendd\0" "vpblendvb\0" "vpblendw\0" "vpbroadcastb\0"
  "vpbroadcastd\0" "vpbroadcastmb2d\0" "vpbroadcastmb2q\0" "vpbroadcastq\0"
  "vpbroadcastw\0" "vpclmulqdq\0" "vpcmov\0" "vpcmpb\0" "vpcmpd\0" "vpcmpeqb\0"
  "vpcmpeqd\0" "vpcmpeqq\0" "vpcmpeqw\0" "vpcmpestri\0" "vpcmpestrm\0"
  "vpcmpgtb\0" "vpcmpgtd\0" "vpcmpgtq\0" "vpcmpgtw\0" "vpcmpistri\0"
  "vpcmpistrm\0" "vpcmpq\0" "vpcmpub\0" "vpcmpud\0" "vpcmpuq\0" "vpcmpuw\0"
  "vpcmpw\0" "vpcomb\0" "vpcomd\0" "vpcompressd\0" "vpcompressq\0" "vpcomq\0"
  "vpcomub\0" "vpcomud\0" "vpcomuq\0" "vpcomuw\0" "vpcomw\0" "vpconflictd\0"
  "vpconflictq\0" "vperm2f128\0" "vperm2i128\0" "vpermb\0" "vpermd\0"
  "vpermi2b\0" "vpermi2d\0" "vpermi2pd\0" "vpermi2ps\0" "vpermi2q\0"
  "vpermi2w\0" "vpermil2pd\0" "vpermil2ps\0" "vpermilpd\0" "vpermilps\0"
  "vpermpd\0" "vpermps\0" "vpermq\0" "vpermt2b\0" "vpermt2d\0" "vpermt2pd\0"
  "vpermt2ps\0" "vpermt2q\0" "vpermt2w\0" "vpermw\0" "vpexpandd\0"
  "vpexpandq\0" "vpextrb\0" "vpextrd\0" "vpextrq\0" "vpextrw\0" "vpgatherdd\0"
  "vpgatherdq\0" "vpgatherqd\0" "vpgatherqq\0" "vphaddbd\0" "vphaddbq\0"
  "vphaddbw\0" "vphaddd\0" "vphadddq\0" "vphaddsw\0" "vphaddubd\0"
  "vphaddubq\0" "vphaddubw\0" "vphaddudq\0" "vphadduwd\0" "vphadduwq\0"
  "vphaddw\0" "vphaddwd\0" "vphaddwq\0" "vphminposuw\0" "vphsubbw\0"
  "vphsubd\0" "vphsubdq\0" "vphsubsw\0" "vphsubw\0" "vphsubwd\0" "vpinsrb\0"
  "vpinsrd\0" "vpinsrq\0" "vpinsrw\0" "vplzcntd\0" "vplzcntq\0" "vpmacsdd\0"
  "vpmacsdqh\0" "vpmacsdql\0" "vpmacssdd\0" "vpmacssdqh\0" "vpmacssdql\0"
  "vpmacsswd\0" "vpmacssww\0" "vpmacswd\0" "vpmacsww\0" "vpmadcsswd\0"
  "vpmadcswd\0" "vpmadd52huq\0" "vpmadd52luq\0" "vpmaddubsw\0" "vpmaddwd\0"
  "vpmaskmovd\0" "vpmaskmovq\0" "vpmaxsb\0" "vpmaxsd\0" "vpmaxsq\0" "vpmaxsw\0"
  "vpmaxub\0" "vpmaxud\0" "vpmaxuq\0" "vpmaxuw\0" "vpminsb\0" "vpminsd\0"
  "vpminsq\0" "vpminsw\0" "vpminub\0" "vpminud\0" "vpminuq\0" "vpminuw\0"
  "vpmovb2m\0" "vpmovd2m\0" "vpmovdb\0" "vpmovdw\0" "vpmovm2b\0" "vpmovm2d\0"
  "vpmovm2q\0" "vpmovm2w\0" "vpmovmskb\0" "vpmovq2m\0" "vpmovqb\0" "vpmovqd\0"
  "vpmovqw\0" "vpmovsdb\0" "vpmovsdw\0" "vpmovsqb\0" "vpmovsqd\0" "vpmovsqw\0"
  "vpmovswb\0" "vpmovsxbd\0" "vpmovsxbq\0" "vpmovsxbw\0" "vpmovsxdq\0"
  "vpmovsxwd\0" "vpmovsxwq\0" "vpmovusdb\0" "vpmovusdw\0" "vpmovusqb\0"
  "vpmovusqd\0" "vpmovusqw\0" "vpmovuswb\0" "vpmovw2m\0" "vpmovwb\0"
  "vpmovzxbd\0" "vpmovzxbq\0" "vpmovzxbw\0" "vpmovzxdq\0" "vpmovzxwd\0"
  "vpmovzxwq\0" "vpmuldq\0" "vpmulhrsw\0" "vpmulhuw\0" "vpmulhw\0" "vpmulld\0"
  "vpmullq\0" "vpmullw\0" "vpmultishiftqb\0" "vpmuludq\0" "vpor\0" "vpord\0"
  "vporq\0" "vpperm\0" "vprold\0" "vprolq\0" "vprolvd\0" "vprolvq\0" "vprord\0"
  "vprorq\0" "vprorvd\0" "vprorvq\0" "vprotb\0" "vprotd\0" "vprotq\0"
  "vprotw\0" "vpsadbw\0" "vpscatterdd\0" "vpscatterdq\0" "vpscatterqd\0"
  "vpscatterqq\0" "vpshab\0" "vpshad\0" "vpshaq\0" "vpshaw\0" "vpshlb\0"
  "vpshld\0" "vpshlq\0" "vpshlw\0" "vpshufb\0" "vpshufd\0" "vpshufhw\0"
  "vpshuflw\0" "vpsignb\0" "vpsignd\0" "vpsignw\0" "vpslld\0" "vpslldq\0"
  "vpsllq\0" "vpsllvd\0" "vpsllvq\0" "vpsllvw\0" "vpsllw\0" "vpsrad\0"
  "vpsraq\0" "vpsravd\0" "vpsravq\0" "vpsravw\0" "vpsraw\0" "vpsrld\0"
  "vpsrldq\0" "vpsrlq\0" "vpsrlvd\0" "vpsrlvq\0" "vpsrlvw\0" "vpsrlw\0"
  "vpsubb\0" "vpsubd\0" "vpsubq\0" "vpsubsb\0" "vpsubsw\0" "vpsubusb\0"
  "vpsubusw\0" "vpsubw\0" "vpternlogd\0" "vpternlogq\0" "vptest\0" "vptestmb\0"
  "vptestmd\0" "vptestmq\0" "vptestmw\0" "vptestnmb\0" "vptestnmd\0"
  "vptestnmq\0" "vptestnmw\0" "vpunpckhbw\0" "vpunpckhdq\0" "vpunpckhqdq\0"
  "vpunpckhwd\0" "vpunpcklbw\0" "vpunpckldq\0" "vpunpcklqdq\0" "vpunpcklwd\0"
  "vpxor\0" "vpxord\0" "vpxorq\0" "vrangepd\0" "vrangeps\0" "vrangesd\0"
  "vrangess\0" "vrcp14pd\0" "vrcp14ps\0" "vrcp14sd\0" "vrcp14ss\0" "vrcp28pd\0"
  "vrcp28ps\0" "vrcp28sd\0" "vrcp28ss\0" "vrcpps\0" "vrcpss\0" "vreducepd\0"
  "vreduceps\0" "vreducesd\0" "vreducess\0" "vrndscalepd\0" "vrndscaleps\0"
  "vrndscalesd\0" "vrndscaless\0" "vroundpd\0" "vroundps\0" "vroundsd\0"
  "vroundss\0" "vrsqrt14pd\0" "vrsqrt14ps\0" "vrsqrt14sd\0" "vrsqrt14ss\0"
  "vrsqrt28pd\0" "vrsqrt28ps\0" "vrsqrt28sd\0" "vrsqrt28ss\0" "vrsqrtps\0"
  "vrsqrtss\0" "vscalefpd\0" "vscalefps\0" "vscalefsd\0" "vscalefss\0"
  "vscatterdpd\0" "vscatterdps\0" "vscatterpf0dpd\0" "vscatterpf0dps\0"
  "vscatterpf0qpd\0" "vscatterpf0qps\0" "vscatterpf1dpd\0" "vscatterpf1dps\0"
  "vscatterpf1qpd\0" "vscatterpf1qps\0" "vscatterqpd\0" "vscatterqps\0"
  "vshuff32x4\0" "vshuff64x2\0" "vshufi32x4\0" "vshufi64x2\0" "vshufpd\0"
  "vshufps\0" "vsqrtpd\0" "vsqrtps\0" "vsqrtsd\0" "vsqrtss\0" "vstmxcsr\0"
  "vsubpd\0" "vsubps\0" "vsubsd\0" "vsubss\0" "vtestpd\0" "vtestps\0"
  "vucomisd\0" "vucomiss\0" "vunpckhpd\0" "vunpckhps\0" "vunpcklpd\0"
  "vunpcklps\0" "vxorpd\0" "vxorps\0" "vzeroall\0" "vzeroupper\0" "wrfsbase\0"
  "wrgsbase\0" "xadd\0" "xgetbv\0" "xrstors\0" "xrstors64\0" "xsavec\0"
  "xsavec64\0" "xsaveopt\0" "xsaveopt64\0" "xsaves\0" "xsaves64\0" "xsetbv";

enum {
  kX86InstMaxLength = 16
};

struct InstNameAZ {
  uint16_t start;
  uint16_t end;
};

static const InstNameAZ X86InstNameAZ[26] = {
  { X86Inst::kIdAaa       , X86Inst::kIdAndps      + 1 },
  { X86Inst::kIdBextr     , X86Inst::kIdBzhi       + 1 },
  { X86Inst::kIdCall      , X86Inst::kIdCwde       + 1 },
  { X86Inst::kIdDaa       , X86Inst::kIdDpps       + 1 },
  { X86Inst::kIdEmms      , X86Inst::kIdExtrq      + 1 },
  { X86Inst::kIdF2xm1     , X86Inst::kIdFyl2xp1    + 1 },
  { X86Inst::kIdNone      , X86Inst::kIdNone       + 1 },
  { X86Inst::kIdHaddpd    , X86Inst::kIdHsubps     + 1 },
  { X86Inst::kIdIdiv      , X86Inst::kIdInto       + 1 },
  { X86Inst::kIdJa        , X86Inst::kIdJz         + 1 },
  { X86Inst::kIdKaddb     , X86Inst::kIdKxorw      + 1 },
  { X86Inst::kIdLahf      , X86Inst::kIdLzcnt      + 1 },
  { X86Inst::kIdMaskmovdqu, X86Inst::kIdMwait      + 1 },
  { X86Inst::kIdNeg       , X86Inst::kIdNot        + 1 },
  { X86Inst::kIdOr        , X86Inst::kIdOuts       + 1 },
  { X86Inst::kIdPabsb     , X86Inst::kIdPxor       + 1 },
  { X86Inst::kIdNone      , X86Inst::kIdNone       + 1 },
  { X86Inst::kIdRcl       , X86Inst::kIdRsqrtss    + 1 },
  { X86Inst::kIdSahf      , X86Inst::kIdSwapgs     + 1 },
  { X86Inst::kIdT1mskc    , X86Inst::kIdTzmsk      + 1 },
  { X86Inst::kIdUcomisd   , X86Inst::kIdUnpcklps   + 1 },
  { X86Inst::kIdVaddpd    , X86Inst::kIdVzeroupper + 1 },
  { X86Inst::kIdWrfsbase  , X86Inst::kIdWrgsbase   + 1 },
  { X86Inst::kIdXadd      , X86Inst::kIdXsetbv     + 1 },
  { X86Inst::kIdNone      , X86Inst::kIdNone       + 1 },
  { X86Inst::kIdNone      , X86Inst::kIdNone       + 1 }
};
// ----------------------------------------------------------------------------
// ${nameData:End}

uint32_t X86Inst::getIdByName(const char* name, size_t len) noexcept {
  if (ASMJIT_UNLIKELY(!name))
    return Globals::kInvalidInstId;

  if (len == Globals::kInvalidIndex)
    len = ::strlen(name);

  if (ASMJIT_UNLIKELY(len == 0 || len > kX86InstMaxLength))
    return Globals::kInvalidInstId;

  uint32_t prefix = static_cast<uint32_t>(name[0]) - 'a';
  if (ASMJIT_UNLIKELY(prefix > 'z' - 'a'))
    return Globals::kInvalidInstId;

  uint32_t index = X86InstNameAZ[prefix].start;
  if (ASMJIT_UNLIKELY(!index))
    return Globals::kInvalidInstId;

  const char* nameData = X86InstDB::nameData;
  const X86Inst* instData = X86InstDB::instData;

  const X86Inst* base = instData + index;
  const X86Inst* end  = instData + X86InstNameAZ[prefix].end;

  for (size_t lim = (size_t)(end - base); lim != 0; lim >>= 1) {
    const X86Inst* cur = base + (lim >> 1);
    int result = Utils::cmpInstName(nameData + cur[0].getNameDataIndex(), name, len);

    if (result < 0) {
      base = cur + 1;
      lim--;
      continue;
    }

    if (result > 0)
      continue;

    return static_cast<uint32_t>((size_t)(cur - instData));
  }

  return Globals::kInvalidInstId;
}

const char* X86Inst::getNameById(uint32_t id) noexcept {
  if (ASMJIT_UNLIKELY(id >= X86Inst::_kIdCount))
    return nullptr;
  return X86Inst::getInst(id).getName();
}
#else
const char X86InstDB::nameData[] = "";
#endif // !ASMJIT_DISABLE_TEXT

// ============================================================================
// [asmjit::X86Inst - Validation]
// ============================================================================

#if !defined(ASMJIT_DISABLE_VALIDATION)
// ${signatureData:Begin}
// ------------------- Automatically generated, do not edit -------------------
#define ISIGNATURE(count, x86, x64, implicit, o0, o1, o2, o3, o4, o5) \
  { count, (x86 ? uint8_t(X86Inst::kArchMaskX86) : uint8_t(0)) |      \
           (x64 ? uint8_t(X86Inst::kArchMaskX64) : uint8_t(0)) ,      \
    implicit,                                                         \
    0,                                                                \
    { o0, o1, o2, o3, o4, o5 }                                        \
  }
static const X86Inst::ISignature _x86InstISignatureData[] = {
  ISIGNATURE(2, 1, 1, 0, 1  , 2  , 0  , 0  , 0  , 0  ), // #0   {W:r8lo|r8hi|m8, R:r8lo|r8hi|i8}
  ISIGNATURE(2, 1, 1, 0, 3  , 4  , 0  , 0  , 0  , 0  ), //      {W:r16|m16, R:r16|sreg|i16}
  ISIGNATURE(2, 1, 1, 0, 5  , 6  , 0  , 0  , 0  , 0  ), //      {W:r32|m32|sreg, R:r32}
  ISIGNATURE(2, 0, 1, 0, 7  , 8  , 0  , 0  , 0  , 0  ), //      {W:r64|m64, R:r64|sreg|i32}
  ISIGNATURE(2, 1, 1, 0, 9  , 10 , 0  , 0  , 0  , 0  ), //      {W:r8lo|r8hi, R:r8lo|r8hi|m8|i8}
  ISIGNATURE(2, 1, 1, 0, 11 , 12 , 0  , 0  , 0  , 0  ), //      {W:r16|sreg, R:r16|m16}
  ISIGNATURE(2, 1, 1, 0, 13 , 14 , 0  , 0  , 0  , 0  ), //      {W:r32, R:r32|m32|sreg|i32}
  ISIGNATURE(2, 0, 1, 0, 15 , 16 , 0  , 0  , 0  , 0  ), //      {W:r64|sreg, R:r64|m64}
  ISIGNATURE(2, 1, 1, 0, 17 , 18 , 0  , 0  , 0  , 0  ), //      {W:r16, R:i16}
  ISIGNATURE(2, 0, 1, 0, 19 , 20 , 0  , 0  , 0  , 0  ), //      {W:r64, R:i64|creg|dreg}
  ISIGNATURE(2, 1, 1, 0, 21 , 22 , 0  , 0  , 0  , 0  ), //      {W:r32|m32, R:i32}
  ISIGNATURE(2, 1, 0, 0, 13 , 23 , 0  , 0  , 0  , 0  ), //      {W:r32, R:creg|dreg}
  ISIGNATURE(2, 1, 0, 0, 24 , 6  , 0  , 0  , 0  , 0  ), //      {W:creg|dreg, R:r32}
  ISIGNATURE(2, 0, 1, 0, 24 , 25 , 0  , 0  , 0  , 0  ), //      {W:creg|dreg, R:r64}
  ISIGNATURE(2, 1, 1, 0, 26 , 27 , 0  , 0  , 0  , 0  ), // #14  {X:r8lo|r8hi|m8|r16|m16|r32|m32|r64|m64, R:i8}
  ISIGNATURE(2, 1, 1, 0, 28 , 29 , 0  , 0  , 0  , 0  ), //      {X:r16|m16, R:i16|r16}
  ISIGNATURE(2, 1, 1, 0, 30 , 22 , 0  , 0  , 0  , 0  ), //      {X:r32|m32|r64|m64, R:i32}
  ISIGNATURE(2, 1, 1, 0, 31 , 32 , 0  , 0  , 0  , 0  ), //      {X:r8lo|r8hi|m8, R:r8lo|r8hi}
  ISIGNATURE(2, 1, 1, 0, 33 , 6  , 0  , 0  , 0  , 0  ), //      {X:r32|m32, R:r32}
  ISIGNATURE(2, 0, 1, 0, 34 , 25 , 0  , 0  , 0  , 0  ), //      {X:r64|m64, R:r64}
  ISIGNATURE(2, 1, 1, 0, 35 , 36 , 0  , 0  , 0  , 0  ), //      {X:r8lo|r8hi, R:r8lo|r8hi|m8}
  ISIGNATURE(2, 1, 1, 0, 37 , 12 , 0  , 0  , 0  , 0  ), // #21  {X:r16, R:r16|m16}
  ISIGNATURE(2, 1, 1, 0, 38 , 39 , 0  , 0  , 0  , 0  ), // #22  {X:r32, R:r32|m32}
  ISIGNATURE(2, 0, 1, 0, 40 , 16 , 0  , 0  , 0  , 0  ), //      {X:r64, R:r64|m64}
  ISIGNATURE(2, 1, 1, 0, 41 , 27 , 0  , 0  , 0  , 0  ), // #24  {R:r8lo|r8hi|m8|r16|m16|r32|m32|r64|m64, R:i8}
  ISIGNATURE(2, 1, 1, 0, 12 , 29 , 0  , 0  , 0  , 0  ), //      {R:r16|m16, R:i16|r16}
  ISIGNATURE(2, 1, 1, 0, 42 , 22 , 0  , 0  , 0  , 0  ), //      {R:r32|m32|r64|m64, R:i32}
  ISIGNATURE(2, 1, 1, 0, 36 , 32 , 0  , 0  , 0  , 0  ), //      {R:r8lo|r8hi|m8, R:r8lo|r8hi}
  ISIGNATURE(2, 1, 1, 0, 39 , 6  , 0  , 0  , 0  , 0  ), //      {R:r32|m32, R:r32}
  ISIGNATURE(2, 0, 1, 0, 16 , 25 , 0  , 0  , 0  , 0  ), //      {R:r64|m64, R:r64}
  ISIGNATURE(2, 1, 1, 0, 32 , 36 , 0  , 0  , 0  , 0  ), //      {R:r8lo|r8hi, R:r8lo|r8hi|m8}
  ISIGNATURE(2, 1, 1, 0, 43 , 12 , 0  , 0  , 0  , 0  ), //      {R:r16, R:r16|m16}
  ISIGNATURE(2, 1, 1, 0, 6  , 39 , 0  , 0  , 0  , 0  ), //      {R:r32, R:r32|m32}
  ISIGNATURE(2, 0, 1, 0, 25 , 16 , 0  , 0  , 0  , 0  ), //      {R:r64, R:r64|m64}
  ISIGNATURE(2, 1, 1, 1, 44 , 36 , 0  , 0  , 0  , 0  ), // #34  {X:<ax>, R:r8lo|r8hi|m8}
  ISIGNATURE(3, 1, 1, 2, 45 , 44 , 12 , 0  , 0  , 0  ), //      {W:<dx>, X:<ax>, R:r16|m16}
  ISIGNATURE(3, 1, 1, 2, 46 , 47 , 39 , 0  , 0  , 0  ), //      {W:<edx>, X:<eax>, R:r32|m32}
  ISIGNATURE(3, 0, 1, 2, 48 , 49 , 16 , 0  , 0  , 0  ), //      {W:<rdx>, X:<rax>, R:r64|m64}
  ISIGNATURE(2, 1, 1, 0, 37 , 50 , 0  , 0  , 0  , 0  ), //      {X:r16, R:r16|m16|i8|i16}
  ISIGNATURE(2, 1, 1, 0, 38 , 51 , 0  , 0  , 0  , 0  ), //      {X:r32, R:r32|m32|i8|i32}
  ISIGNATURE(2, 0, 1, 0, 40 , 52 , 0  , 0  , 0  , 0  ), //      {X:r64, R:r64|m64|i8|i32}
  ISIGNATURE(3, 1, 1, 0, 17 , 12 , 53 , 0  , 0  , 0  ), //      {W:r16, R:r16|m16, R:i8|i16}
  ISIGNATURE(3, 1, 1, 0, 13 , 39 , 54 , 0  , 0  , 0  ), //      {W:r32, R:r32|m32, R:i8|i32}
  ISIGNATURE(3, 0, 1, 0, 19 , 16 , 54 , 0  , 0  , 0  ), //      {W:r64, R:r64|m64, R:i8|i32}
  ISIGNATURE(2, 1, 1, 0, 28 , 37 , 0  , 0  , 0  , 0  ), // #44  {X:r16|m16, X:r16}
  ISIGNATURE(2, 1, 1, 0, 33 , 38 , 0  , 0  , 0  , 0  ), //      {X:r32|m32, X:r32}
  ISIGNATURE(2, 0, 1, 0, 34 , 40 , 0  , 0  , 0  , 0  ), //      {X:r64|m64, X:r64}
  ISIGNATURE(2, 1, 1, 0, 37 , 28 , 0  , 0  , 0  , 0  ), //      {X:r16, X:r16|m16}
  ISIGNATURE(2, 1, 1, 0, 38 , 33 , 0  , 0  , 0  , 0  ), //      {X:r32, X:r32|m32}
  ISIGNATURE(2, 0, 1, 0, 40 , 34 , 0  , 0  , 0  , 0  ), //      {X:r64, X:r64|m64}
  ISIGNATURE(2, 1, 1, 0, 31 , 35 , 0  , 0  , 0  , 0  ), //      {X:r8lo|r8hi|m8, X:r8lo|r8hi}
  ISIGNATURE(2, 1, 1, 0, 35 , 31 , 0  , 0  , 0  , 0  ), //      {X:r8lo|r8hi, X:r8lo|r8hi|m8}
  ISIGNATURE(2, 1, 1, 0, 17 , 55 , 0  , 0  , 0  , 0  ), // #52  {W:r16, R:m16}
  ISIGNATURE(2, 1, 1, 0, 13 , 56 , 0  , 0  , 0  , 0  ), //      {W:r32, R:m32}
  ISIGNATURE(2, 0, 1, 0, 19 , 57 , 0  , 0  , 0  , 0  ), //      {W:r64, R:m64}
  ISIGNATURE(2, 1, 1, 0, 58 , 43 , 0  , 0  , 0  , 0  ), //      {W:m16, R:r16}
  ISIGNATURE(2, 1, 1, 0, 59 , 6  , 0  , 0  , 0  , 0  ), // #56  {W:m32, R:r32}
  ISIGNATURE(2, 0, 1, 0, 60 , 25 , 0  , 0  , 0  , 0  ), //      {W:m64, R:r64}
  ISIGNATURE(2, 1, 1, 0, 61 , 62 , 0  , 0  , 0  , 0  ), // #58  {W:mm, R:mm|m64|r64|xmm}
  ISIGNATURE(2, 1, 1, 0, 63 , 64 , 0  , 0  , 0  , 0  ), //      {W:mm|m64|r64|xmm, R:mm}
  ISIGNATURE(2, 0, 1, 0, 7  , 65 , 0  , 0  , 0  , 0  ), //      {W:r64|m64, R:xmm}
  ISIGNATURE(2, 0, 1, 0, 66 , 16 , 0  , 0  , 0  , 0  ), //      {W:xmm, R:r64|m64}
  ISIGNATURE(2, 1, 1, 0, 66 , 67 , 0  , 0  , 0  , 0  ), // #62  {W:xmm, R:xmm|m64}
  ISIGNATURE(2, 1, 1, 0, 68 , 65 , 0  , 0  , 0  , 0  ), //      {W:xmm|m64, R:xmm}
  ISIGNATURE(2, 1, 1, 0, 66 , 69 , 0  , 0  , 0  , 0  ), // #64  {W:xmm, R:xmm|m128}
  ISIGNATURE(2, 1, 1, 0, 70 , 65 , 0  , 0  , 0  , 0  ), //      {W:xmm|m128, R:xmm}
  ISIGNATURE(2, 1, 1, 0, 71 , 72 , 0  , 0  , 0  , 0  ), //      {W:ymm, R:ymm|m256}
  ISIGNATURE(2, 1, 1, 0, 73 , 74 , 0  , 0  , 0  , 0  ), //      {W:ymm|m256, R:ymm}
  ISIGNATURE(2, 1, 1, 0, 75 , 76 , 0  , 0  , 0  , 0  ), // #68  {W:zmm, R:zmm|m512}
  ISIGNATURE(2, 1, 1, 0, 77 , 78 , 0  , 0  , 0  , 0  ), //      {W:zmm|m512, R:zmm}
  ISIGNATURE(3, 1, 1, 0, 66 , 65 , 69 , 0  , 0  , 0  ), // #70  {W:xmm, R:xmm, R:xmm|m128}
  ISIGNATURE(3, 1, 1, 0, 66 , 69 , 27 , 0  , 0  , 0  ), // #71  {W:xmm, R:xmm|m128, R:i8}
  ISIGNATURE(3, 1, 1, 0, 71 , 74 , 72 , 0  , 0  , 0  ), // #72  {W:ymm, R:ymm, R:ymm|m256}
  ISIGNATURE(3, 1, 1, 0, 71 , 72 , 27 , 0  , 0  , 0  ), // #73  {W:ymm, R:ymm|m256, R:i8}
  ISIGNATURE(3, 1, 1, 0, 75 , 78 , 76 , 0  , 0  , 0  ), //      {W:zmm, R:zmm, R:zmm|m512}
  ISIGNATURE(3, 1, 1, 0, 75 , 76 , 27 , 0  , 0  , 0  ), //      {W:zmm, R:zmm|m512, R:i8}
  ISIGNATURE(3, 1, 1, 0, 66 , 65 , 79 , 0  , 0  , 0  ), // #76  {W:xmm, R:xmm, R:i8|xmm|m128}
  ISIGNATURE(3, 1, 1, 0, 71 , 74 , 79 , 0  , 0  , 0  ), //      {W:ymm, R:ymm, R:i8|xmm|m128}
  ISIGNATURE(3, 1, 1, 0, 66 , 69 , 27 , 0  , 0  , 0  ), // #78  {W:xmm, R:xmm|m128, R:i8}
  ISIGNATURE(3, 1, 1, 0, 71 , 72 , 27 , 0  , 0  , 0  ), //      {W:ymm, R:ymm|m256, R:i8}
  ISIGNATURE(3, 1, 1, 0, 75 , 78 , 69 , 0  , 0  , 0  ), //      {W:zmm, R:zmm, R:xmm|m128}
  ISIGNATURE(3, 1, 1, 0, 75 , 76 , 27 , 0  , 0  , 0  ), //      {W:zmm, R:zmm|m512, R:i8}
  ISIGNATURE(3, 1, 1, 0, 66 , 65 , 69 , 0  , 0  , 0  ), // #82  {W:xmm, R:xmm, R:xmm|m128}
  ISIGNATURE(3, 1, 1, 0, 66 , 69 , 27 , 0  , 0  , 0  ), //      {W:xmm, R:xmm|m128, R:i8}
  ISIGNATURE(3, 1, 1, 0, 71 , 74 , 69 , 0  , 0  , 0  ), //      {W:ymm, R:ymm, R:xmm|m128}
  ISIGNATURE(3, 1, 1, 0, 71 , 72 , 27 , 0  , 0  , 0  ), //      {W:ymm, R:ymm|m256, R:i8}
  ISIGNATURE(3, 1, 1, 0, 75 , 78 , 69 , 0  , 0  , 0  ), //      {W:zmm, R:zmm, R:xmm|m128}
  ISIGNATURE(3, 1, 1, 0, 75 , 76 , 27 , 0  , 0  , 0  ), //      {W:zmm, R:zmm|m512, R:i8}
  ISIGNATURE(2, 1, 1, 0, 36 , 2  , 0  , 0  , 0  , 0  ), // #88  {R:r8lo|r8hi|m8, R:i8|r8lo|r8hi}
  ISIGNATURE(2, 1, 1, 0, 12 , 29 , 0  , 0  , 0  , 0  ), //      {R:r16|m16, R:i16|r16}
  ISIGNATURE(2, 1, 1, 0, 42 , 22 , 0  , 0  , 0  , 0  ), //      {R:r32|m32|r64|m64, R:i32}
  ISIGNATURE(2, 1, 1, 0, 39 , 6  , 0  , 0  , 0  , 0  ), //      {R:r32|m32, R:r32}
  ISIGNATURE(2, 0, 1, 0, 16 , 25 , 0  , 0  , 0  , 0  ), //      {R:r64|m64, R:r64}
  ISIGNATURE(3, 1, 1, 0, 66 , 80 , 65 , 0  , 0  , 0  ), // #93  {W:xmm, R:vm32x, R:xmm}
  ISIGNATURE(3, 1, 1, 0, 71 , 80 , 74 , 0  , 0  , 0  ), //      {W:ymm, R:vm32x, R:ymm}
  ISIGNATURE(2, 1, 1, 0, 66 , 80 , 0  , 0  , 0  , 0  ), //      {W:xmm, R:vm32x}
  ISIGNATURE(2, 1, 1, 0, 71 , 81 , 0  , 0  , 0  , 0  ), //      {W:ymm, R:vm32y}
  ISIGNATURE(2, 1, 1, 0, 75 , 82 , 0  , 0  , 0  , 0  ), //      {W:zmm, R:vm32z}
  ISIGNATURE(3, 1, 1, 0, 66 , 80 , 65 , 0  , 0  , 0  ), // #98  {W:xmm, R:vm32x, R:xmm}
  ISIGNATURE(3, 1, 1, 0, 71 , 81 , 74 , 0  , 0  , 0  ), //      {W:ymm, R:vm32y, R:ymm}
  ISIGNATURE(2, 1, 1, 0, 66 , 80 , 0  , 0  , 0  , 0  ), //      {W:xmm, R:vm32x}
  ISIGNATURE(2, 1, 1, 0, 71 , 81 , 0  , 0  , 0  , 0  ), //      {W:ymm, R:vm32y}
  ISIGNATURE(2, 1, 1, 0, 75 , 82 , 0  , 0  , 0  , 0  ), //      {W:zmm, R:vm32z}
  ISIGNATURE(3, 1, 1, 0, 66 , 83 , 65 , 0  , 0  , 0  ), // #103 {W:xmm, R:vm64x, R:xmm}
  ISIGNATURE(3, 1, 1, 0, 71 , 84 , 74 , 0  , 0  , 0  ), //      {W:ymm, R:vm64y, R:ymm}
  ISIGNATURE(2, 1, 1, 0, 66 , 83 , 0  , 0  , 0  , 0  ), //      {W:xmm, R:vm64x}
  ISIGNATURE(2, 1, 1, 0, 71 , 84 , 0  , 0  , 0  , 0  ), //      {W:ymm, R:vm64y}
  ISIGNATURE(2, 1, 1, 0, 75 , 85 , 0  , 0  , 0  , 0  ), //      {W:zmm, R:vm64z}
  ISIGNATURE(3, 1, 1, 1, 31 , 32 , 86 , 0  , 0  , 0  ), // #108 {X:r8lo|r8hi|m8, R:r8lo|r8hi, R:<al>}
  ISIGNATURE(3, 1, 1, 1, 28 , 43 , 87 , 0  , 0  , 0  ), //      {X:r16|m16, R:r16, R:<ax>}
  ISIGNATURE(3, 1, 1, 1, 33 , 6  , 88 , 0  , 0  , 0  ), //      {X:r32|m32, R:r32, R:<eax>}
  ISIGNATURE(3, 0, 1, 1, 34 , 25 , 89 , 0  , 0  , 0  ), //      {X:r64|m64, R:r64, R:<rax>}
  ISIGNATURE(2, 1, 1, 1, 44 , 36 , 0  , 0  , 0  , 0  ), // #112 {X:<ax>, R:r8lo|r8hi|m8}
  ISIGNATURE(3, 1, 1, 2, 44 , 90 , 12 , 0  , 0  , 0  ), //      {X:<ax>, X:<dx>, R:r16|m16}
  ISIGNATURE(3, 1, 1, 2, 47 , 91 , 39 , 0  , 0  , 0  ), //      {X:<eax>, X:<edx>, R:r32|m32}
  ISIGNATURE(3, 0, 1, 2, 49 , 92 , 16 , 0  , 0  , 0  ), //      {X:<rax>, X:<rdx>, R:r64|m64}
  ISIGNATURE(2, 1, 1, 1, 44 , 36 , 0  , 0  , 0  , 0  ), // #116 {X:<ax>, R:r8lo|r8hi|m8}
  ISIGNATURE(3, 1, 1, 2, 90 , 44 , 12 , 0  , 0  , 0  ), //      {X:<dx>, X:<ax>, R:r16|m16}
  ISIGNATURE(3, 1, 1, 2, 91 , 47 , 39 , 0  , 0  , 0  ), //      {X:<edx>, X:<eax>, R:r32|m32}
  ISIGNATURE(3, 0, 1, 2, 92 , 49 , 16 , 0  , 0  , 0  ), //      {X:<rdx>, X:<rax>, R:r64|m64}
  ISIGNATURE(1, 1, 1, 0, 93 , 0  , 0  , 0  , 0  , 0  ), // #120 {W:r16|m16|r64|m64}
  ISIGNATURE(1, 1, 0, 0, 21 , 0  , 0  , 0  , 0  , 0  ), //      {W:r32|m32}
  ISIGNATURE(1, 1, 0, 0, 94 , 0  , 0  , 0  , 0  , 0  ), //      {W:ds|es|ss}
  ISIGNATURE(1, 1, 1, 0, 95 , 0  , 0  , 0  , 0  , 0  ), //      {W:fs|gs}
  ISIGNATURE(1, 1, 1, 0, 96 , 0  , 0  , 0  , 0  , 0  ), // #124 {X:r16|m16|r64|m64|i8|i16|i32}
  ISIGNATURE(1, 1, 0, 0, 39 , 0  , 0  , 0  , 0  , 0  ), //      {R:r32|m32}
  ISIGNATURE(1, 1, 0, 0, 97 , 0  , 0  , 0  , 0  , 0  ), //      {R:cs|ss|ds|es}
  ISIGNATURE(1, 1, 1, 0, 98 , 0  , 0  , 0  , 0  , 0  ), //      {R:fs|gs}
  ISIGNATURE(4, 1, 1, 0, 66 , 65 , 65 , 69 , 0  , 0  ), // #128 {W:xmm, R:xmm, R:xmm, R:xmm|m128}
  ISIGNATURE(4, 1, 1, 0, 66 , 65 , 69 , 65 , 0  , 0  ), // #129 {W:xmm, R:xmm, R:xmm|m128, R:xmm}
  ISIGNATURE(4, 1, 1, 0, 71 , 74 , 74 , 72 , 0  , 0  ), //      {W:ymm, R:ymm, R:ymm, R:ymm|m256}
  ISIGNATURE(4, 1, 1, 0, 71 , 74 , 72 , 74 , 0  , 0  ), //      {W:ymm, R:ymm, R:ymm|m256, R:ymm}
  ISIGNATURE(3, 1, 1, 0, 66 , 99 , 65 , 0  , 0  , 0  ), // #132 {W:xmm, R:vm64x|vm64y, R:xmm}
  ISIGNATURE(2, 1, 1, 0, 66 , 83 , 0  , 0  , 0  , 0  ), //      {W:xmm, R:vm64x}
  ISIGNATURE(2, 1, 1, 0, 71 , 84 , 0  , 0  , 0  , 0  ), //      {W:ymm, R:vm64y}
  ISIGNATURE(2, 1, 1, 0, 75 , 85 , 0  , 0  , 0  , 0  ), //      {W:zmm, R:vm64z}
  ISIGNATURE(3, 1, 1, 0, 100, 65 , 65 , 0  , 0  , 0  ), // #136 {W:m128, R:xmm, R:xmm}
  ISIGNATURE(3, 1, 1, 0, 101, 74 , 74 , 0  , 0  , 0  ), //      {W:m256, R:ymm, R:ymm}
  ISIGNATURE(3, 1, 1, 0, 66 , 65 , 102, 0  , 0  , 0  ), //      {W:xmm, R:xmm, R:m128}
  ISIGNATURE(3, 1, 1, 0, 71 , 74 , 103, 0  , 0  , 0  ), //      {W:ymm, R:ymm, R:m256}
  ISIGNATURE(5, 1, 1, 0, 66 , 65 , 69 , 65 , 104, 0  ), // #140 {W:xmm, R:xmm, R:xmm|m128, R:xmm, R:i4}
  ISIGNATURE(5, 1, 1, 0, 66 , 65 , 65 , 69 , 104, 0  ), //      {W:xmm, R:xmm, R:xmm, R:xmm|m128, R:i4}
  ISIGNATURE(5, 1, 1, 0, 71 , 74 , 72 , 74 , 104, 0  ), //      {W:ymm, R:ymm, R:ymm|m256, R:ymm, R:i4}
  ISIGNATURE(5, 1, 1, 0, 71 , 74 , 74 , 72 , 104, 0  ), //      {W:ymm, R:ymm, R:ymm, R:ymm|m256, R:i4}
  ISIGNATURE(3, 1, 1, 0, 71 , 72 , 27 , 0  , 0  , 0  ), // #144 {W:ymm, R:ymm|m256, R:i8}
  ISIGNATURE(3, 1, 1, 0, 71 , 74 , 72 , 0  , 0  , 0  ), // #145 {W:ymm, R:ymm, R:ymm|m256}
  ISIGNATURE(3, 1, 1, 0, 75 , 78 , 76 , 0  , 0  , 0  ), //      {W:zmm, R:zmm, R:zmm|m512}
  ISIGNATURE(3, 1, 1, 0, 75 , 76 , 27 , 0  , 0  , 0  ), //      {W:zmm, R:zmm|m512, R:i8}
  ISIGNATURE(2, 1, 1, 0, 31 , 35 , 0  , 0  , 0  , 0  ), // #148 {X:r8lo|r8hi|m8, X:r8lo|r8hi}
  ISIGNATURE(2, 1, 1, 0, 28 , 37 , 0  , 0  , 0  , 0  ), //      {X:r16|m16, X:r16}
  ISIGNATURE(2, 1, 1, 0, 33 , 38 , 0  , 0  , 0  , 0  ), //      {X:r32|m32, X:r32}
  ISIGNATURE(2, 0, 1, 0, 34 , 40 , 0  , 0  , 0  , 0  ), //      {X:r64|m64, X:r64}
  ISIGNATURE(2, 1, 1, 0, 12 , 105, 0  , 0  , 0  , 0  ), // #152 {R:r16|m16, R:r16|i8}
  ISIGNATURE(2, 1, 1, 0, 39 , 106, 0  , 0  , 0  , 0  ), //      {R:r32|m32, R:r32|i8}
  ISIGNATURE(2, 0, 1, 0, 16 , 107, 0  , 0  , 0  , 0  ), //      {R:r64|m64, R:r64|i8}
  ISIGNATURE(2, 1, 1, 0, 28 , 105, 0  , 0  , 0  , 0  ), // #155 {X:r16|m16, R:r16|i8}
  ISIGNATURE(2, 1, 1, 0, 33 , 106, 0  , 0  , 0  , 0  ), //      {X:r32|m32, R:r32|i8}
  ISIGNATURE(2, 0, 1, 0, 34 , 107, 0  , 0  , 0  , 0  ), //      {X:r64|m64, R:r64|i8}
  ISIGNATURE(1, 1, 1, 0, 108, 0  , 0  , 0  , 0  , 0  ), // #158 {X:m32|m64}
  ISIGNATURE(2, 1, 1, 0, 109, 110, 0  , 0  , 0  , 0  ), //      {X:fp0, R:fp}
  ISIGNATURE(2, 1, 1, 0, 111, 112, 0  , 0  , 0  , 0  ), //      {X:fp, R:fp0}
  ISIGNATURE(2, 1, 1, 0, 17 , 12 , 0  , 0  , 0  , 0  ), // #161 {W:r16, R:r16|m16}
  ISIGNATURE(2, 1, 1, 0, 13 , 39 , 0  , 0  , 0  , 0  ), // #162 {W:r32, R:r32|m32}
  ISIGNATURE(2, 0, 1, 0, 19 , 16 , 0  , 0  , 0  , 0  ), //      {W:r64, R:r64|m64}
  ISIGNATURE(3, 1, 1, 0, 28 , 43 , 113, 0  , 0  , 0  ), // #164 {X:r16|m16, R:r16, R:i8|cl}
  ISIGNATURE(3, 1, 1, 0, 33 , 6  , 113, 0  , 0  , 0  ), //      {X:r32|m32, R:r32, R:i8|cl}
  ISIGNATURE(3, 0, 1, 0, 34 , 25 , 113, 0  , 0  , 0  ), //      {X:r64|m64, R:r64, R:i8|cl}
  ISIGNATURE(3, 1, 1, 0, 66 , 65 , 69 , 0  , 0  , 0  ), // #167 {W:xmm, R:xmm, R:xmm|m128}
  ISIGNATURE(3, 1, 1, 0, 71 , 74 , 72 , 0  , 0  , 0  ), //      {W:ymm, R:ymm, R:ymm|m256}
  ISIGNATURE(3, 1, 1, 0, 75 , 78 , 76 , 0  , 0  , 0  ), //      {W:zmm, R:zmm, R:zmm|m512}
  ISIGNATURE(4, 1, 1, 0, 66 , 65 , 69 , 27 , 0  , 0  ), // #170 {W:xmm, R:xmm, R:xmm|m128, R:i8}
  ISIGNATURE(4, 1, 1, 0, 71 , 74 , 72 , 27 , 0  , 0  ), // #171 {W:ymm, R:ymm, R:ymm|m256, R:i8}
  ISIGNATURE(4, 1, 1, 0, 75 , 78 , 76 , 27 , 0  , 0  ), //      {W:zmm, R:zmm, R:zmm|m512, R:i8}
  ISIGNATURE(4, 1, 1, 0, 114, 65 , 69 , 27 , 0  , 0  ), // #173 {W:xmm|k, R:xmm, R:xmm|m128, R:i8}
  ISIGNATURE(4, 1, 1, 0, 115, 74 , 72 , 27 , 0  , 0  ), //      {W:ymm|k, R:ymm, R:ymm|m256, R:i8}
  ISIGNATURE(4, 1, 1, 0, 116, 78 , 76 , 27 , 0  , 0  ), //      {W:k, R:zmm, R:zmm|m512, R:i8}
  ISIGNATURE(2, 1, 1, 0, 70 , 65 , 0  , 0  , 0  , 0  ), // #176 {W:xmm|m128, R:xmm}
  ISIGNATURE(2, 1, 1, 0, 73 , 74 , 0  , 0  , 0  , 0  ), //      {W:ymm|m256, R:ymm}
  ISIGNATURE(2, 1, 1, 0, 77 , 78 , 0  , 0  , 0  , 0  ), //      {W:zmm|m512, R:zmm}
  ISIGNATURE(2, 1, 1, 0, 66 , 67 , 0  , 0  , 0  , 0  ), // #179 {W:xmm, R:xmm|m64}
  ISIGNATURE(2, 1, 1, 0, 71 , 69 , 0  , 0  , 0  , 0  ), //      {W:ymm, R:xmm|m128}
  ISIGNATURE(2, 1, 1, 0, 75 , 72 , 0  , 0  , 0  , 0  ), //      {W:zmm, R:ymm|m256}
  ISIGNATURE(2, 1, 1, 0, 66 , 69 , 0  , 0  , 0  , 0  ), // #182 {W:xmm, R:xmm|m128}
  ISIGNATURE(2, 1, 1, 0, 71 , 72 , 0  , 0  , 0  , 0  ), //      {W:ymm, R:ymm|m256}
  ISIGNATURE(2, 1, 1, 0, 75 , 76 , 0  , 0  , 0  , 0  ), //      {W:zmm, R:zmm|m512}
  ISIGNATURE(2, 1, 1, 0, 66 , 117, 0  , 0  , 0  , 0  ), // #185 {W:xmm, R:xmm|m128|ymm|m256|m64}
  ISIGNATURE(2, 1, 1, 0, 71 , 69 , 0  , 0  , 0  , 0  ), //      {W:ymm, R:xmm|m128}
  ISIGNATURE(2, 1, 1, 0, 75 , 72 , 0  , 0  , 0  , 0  ), //      {W:zmm, R:ymm|m256}
  ISIGNATURE(3, 1, 1, 0, 68 , 65 , 27 , 0  , 0  , 0  ), // #188 {W:xmm|m64, R:xmm, R:i8}
  ISIGNATURE(3, 1, 1, 0, 70 , 74 , 27 , 0  , 0  , 0  ), // #189 {W:xmm|m128, R:ymm, R:i8}
  ISIGNATURE(3, 1, 1, 0, 73 , 78 , 27 , 0  , 0  , 0  ), // #190 {W:ymm|m256, R:zmm, R:i8}
  ISIGNATURE(4, 1, 1, 0, 118, 65 , 69 , 27 , 0  , 0  ), // #191 {X:xmm, R:xmm, R:xmm|m128, R:i8}
  ISIGNATURE(4, 1, 1, 0, 119, 74 , 72 , 27 , 0  , 0  ), //      {X:ymm, R:ymm, R:ymm|m256, R:i8}
  ISIGNATURE(4, 1, 1, 0, 120, 78 , 76 , 27 , 0  , 0  ), //      {X:zmm, R:zmm, R:zmm|m512, R:i8}
  ISIGNATURE(3, 1, 1, 0, 118, 65 , 69 , 0  , 0  , 0  ), // #194 {X:xmm, R:xmm, R:xmm|m128}
  ISIGNATURE(3, 1, 1, 0, 119, 74 , 72 , 0  , 0  , 0  ), //      {X:ymm, R:ymm, R:ymm|m256}
  ISIGNATURE(3, 1, 1, 0, 120, 78 , 76 , 0  , 0  , 0  ), //      {X:zmm, R:zmm, R:zmm|m512}
  ISIGNATURE(3, 1, 1, 0, 66 , 69 , 27 , 0  , 0  , 0  ), // #197 {W:xmm, R:xmm|m128, R:i8}
  ISIGNATURE(3, 1, 1, 0, 71 , 72 , 27 , 0  , 0  , 0  ), //      {W:ymm, R:ymm|m256, R:i8}
  ISIGNATURE(3, 1, 1, 0, 75 , 76 , 27 , 0  , 0  , 0  ), //      {W:zmm, R:zmm|m512, R:i8}
  ISIGNATURE(2, 1, 1, 0, 66 , 67 , 0  , 0  , 0  , 0  ), // #200 {W:xmm, R:xmm|m64}
  ISIGNATURE(2, 1, 1, 0, 71 , 72 , 0  , 0  , 0  , 0  ), //      {W:ymm, R:ymm|m256}
  ISIGNATURE(2, 1, 1, 0, 75 , 76 , 0  , 0  , 0  , 0  ), //      {W:zmm, R:zmm|m512}
  ISIGNATURE(2, 1, 1, 0, 100, 65 , 0  , 0  , 0  , 0  ), // #203 {W:m128, R:xmm}
  ISIGNATURE(2, 1, 1, 0, 101, 74 , 0  , 0  , 0  , 0  ), //      {W:m256, R:ymm}
  ISIGNATURE(2, 1, 1, 0, 121, 78 , 0  , 0  , 0  , 0  ), //      {W:m512, R:zmm}
  ISIGNATURE(2, 1, 1, 0, 66 , 102, 0  , 0  , 0  , 0  ), // #206 {W:xmm, R:m128}
  ISIGNATURE(2, 1, 1, 0, 71 , 103, 0  , 0  , 0  , 0  ), //      {W:ymm, R:m256}
  ISIGNATURE(2, 1, 1, 0, 75 , 122, 0  , 0  , 0  , 0  ), //      {W:zmm, R:m512}
  ISIGNATURE(2, 0, 1, 0, 7  , 65 , 0  , 0  , 0  , 0  ), // #209 {W:r64|m64, R:xmm}
  ISIGNATURE(2, 1, 1, 0, 66 , 123, 0  , 0  , 0  , 0  ), //      {W:xmm, R:xmm|m64|r64}
  ISIGNATURE(2, 1, 1, 0, 68 , 65 , 0  , 0  , 0  , 0  ), //      {W:xmm|m64, R:xmm}
  ISIGNATURE(2, 1, 1, 0, 60 , 65 , 0  , 0  , 0  , 0  ), // #212 {W:m64, R:xmm}
  ISIGNATURE(2, 1, 1, 0, 66 , 57 , 0  , 0  , 0  , 0  ), //      {W:xmm, R:m64}
  ISIGNATURE(3, 1, 1, 0, 66 , 65 , 65 , 0  , 0  , 0  ), // #214 {W:xmm, R:xmm, R:xmm}
  ISIGNATURE(2, 1, 1, 0, 124, 65 , 0  , 0  , 0  , 0  ), // #215 {W:m32|m64, R:xmm}
  ISIGNATURE(2, 1, 1, 0, 66 , 125, 0  , 0  , 0  , 0  ), //      {W:xmm, R:m32|m64}
  ISIGNATURE(3, 1, 1, 0, 66 , 65 , 65 , 0  , 0  , 0  ), //      {W:xmm, R:xmm, R:xmm}
  ISIGNATURE(4, 1, 1, 0, 116, 65 , 69 , 27 , 0  , 0  ), // #218 {W:k, R:xmm, R:xmm|m128, R:i8}
  ISIGNATURE(4, 1, 1, 0, 116, 74 , 72 , 27 , 0  , 0  ), //      {W:k, R:ymm, R:ymm|m256, R:i8}
  ISIGNATURE(4, 1, 1, 0, 116, 78 , 76 , 27 , 0  , 0  ), //      {W:k, R:zmm, R:zmm|m512, R:i8}
  ISIGNATURE(3, 1, 1, 0, 114, 65 , 69 , 0  , 0  , 0  ), // #221 {W:xmm|k, R:xmm, R:xmm|m128}
  ISIGNATURE(3, 1, 1, 0, 115, 74 , 72 , 0  , 0  , 0  ), //      {W:ymm|k, R:ymm, R:ymm|m256}
  ISIGNATURE(3, 1, 1, 0, 116, 78 , 76 , 0  , 0  , 0  ), //      {W:k, R:zmm, R:zmm|m512}
  ISIGNATURE(2, 1, 1, 0, 126, 65 , 0  , 0  , 0  , 0  ), // #224 {W:xmm|m32, R:xmm}
  ISIGNATURE(2, 1, 1, 0, 68 , 74 , 0  , 0  , 0  , 0  ), //      {W:xmm|m64, R:ymm}
  ISIGNATURE(2, 1, 1, 0, 70 , 78 , 0  , 0  , 0  , 0  ), //      {W:xmm|m128, R:zmm}
  ISIGNATURE(2, 1, 1, 0, 68 , 65 , 0  , 0  , 0  , 0  ), // #227 {W:xmm|m64, R:xmm}
  ISIGNATURE(2, 1, 1, 0, 70 , 74 , 0  , 0  , 0  , 0  ), //      {W:xmm|m128, R:ymm}
  ISIGNATURE(2, 1, 1, 0, 73 , 78 , 0  , 0  , 0  , 0  ), //      {W:ymm|m256, R:zmm}
  ISIGNATURE(2, 1, 1, 0, 127, 65 , 0  , 0  , 0  , 0  ), // #230 {W:xmm|m16, R:xmm}
  ISIGNATURE(2, 1, 1, 0, 126, 74 , 0  , 0  , 0  , 0  ), //      {W:xmm|m32, R:ymm}
  ISIGNATURE(2, 1, 1, 0, 68 , 78 , 0  , 0  , 0  , 0  ), //      {W:xmm|m64, R:zmm}
  ISIGNATURE(2, 1, 1, 0, 66 , 128, 0  , 0  , 0  , 0  ), // #233 {W:xmm, R:xmm|m32}
  ISIGNATURE(2, 1, 1, 0, 71 , 67 , 0  , 0  , 0  , 0  ), //      {W:ymm, R:xmm|m64}
  ISIGNATURE(2, 1, 1, 0, 75 , 69 , 0  , 0  , 0  , 0  ), //      {W:zmm, R:xmm|m128}
  ISIGNATURE(2, 1, 1, 0, 66 , 129, 0  , 0  , 0  , 0  ), // #236 {W:xmm, R:xmm|m16}
  ISIGNATURE(2, 1, 1, 0, 71 , 128, 0  , 0  , 0  , 0  ), //      {W:ymm, R:xmm|m32}
  ISIGNATURE(2, 1, 1, 0, 75 , 67 , 0  , 0  , 0  , 0  ), // #238 {W:zmm, R:xmm|m64}
  ISIGNATURE(2, 1, 1, 0, 66 , 130, 0  , 0  , 0  , 0  ), // #239 {W:xmm, R:xmm|m64|m32}
  ISIGNATURE(2, 1, 1, 0, 71 , 131, 0  , 0  , 0  , 0  ), //      {W:ymm, R:xmm|m128|m64}
  ISIGNATURE(2, 1, 1, 0, 75 , 69 , 0  , 0  , 0  , 0  ), //      {W:zmm, R:xmm|m128}
  ISIGNATURE(2, 1, 1, 0, 132, 65 , 0  , 0  , 0  , 0  ), // #242 {W:vm32x, R:xmm}
  ISIGNATURE(2, 1, 1, 0, 133, 74 , 0  , 0  , 0  , 0  ), //      {W:vm32y, R:ymm}
  ISIGNATURE(2, 1, 1, 0, 134, 78 , 0  , 0  , 0  , 0  ), //      {W:vm32z, R:zmm}
  ISIGNATURE(2, 1, 1, 0, 135, 65 , 0  , 0  , 0  , 0  ), // #245 {W:vm64x, R:xmm}
  ISIGNATURE(2, 1, 1, 0, 136, 74 , 0  , 0  , 0  , 0  ), //      {W:vm64y, R:ymm}
  ISIGNATURE(2, 1, 1, 0, 137, 78 , 0  , 0  , 0  , 0  ), //      {W:vm64z, R:zmm}
  ISIGNATURE(3, 1, 1, 0, 116, 65 , 69 , 0  , 0  , 0  ), // #248 {W:k, R:xmm, R:xmm|m128}
  ISIGNATURE(3, 1, 1, 0, 116, 74 , 72 , 0  , 0  , 0  ), //      {W:k, R:ymm, R:ymm|m256}
  ISIGNATURE(3, 1, 1, 0, 116, 78 , 76 , 0  , 0  , 0  ), //      {W:k, R:zmm, R:zmm|m512}
  ISIGNATURE(3, 1, 1, 0, 13 , 6  , 39 , 0  , 0  , 0  ), // #251 {W:r32, R:r32, R:r32|m32}
  ISIGNATURE(3, 0, 1, 0, 19 , 25 , 16 , 0  , 0  , 0  ), //      {W:r64, R:r64, R:r64|m64}
  ISIGNATURE(3, 1, 1, 0, 13 , 39 , 6  , 0  , 0  , 0  ), // #253 {W:r32, R:r32|m32, R:r32}
  ISIGNATURE(3, 0, 1, 0, 19 , 16 , 25 , 0  , 0  , 0  ), //      {W:r64, R:r64|m64, R:r64}
  ISIGNATURE(1, 1, 1, 0, 138, 0  , 0  , 0  , 0  , 0  ), // #255 {X:rel32|r64|m64}
  ISIGNATURE(1, 1, 0, 0, 39 , 0  , 0  , 0  , 0  , 0  ), //      {R:r32|m32}
  ISIGNATURE(2, 1, 1, 0, 38 , 139, 0  , 0  , 0  , 0  ), // #257 {X:r32, R:r8lo|r8hi|m8|r16|m16|r32|m32}
  ISIGNATURE(2, 0, 1, 0, 40 , 140, 0  , 0  , 0  , 0  ), //      {X:r64, R:r8lo|r8hi|m8|r64|m64}
  ISIGNATURE(1, 1, 0, 0, 141, 0  , 0  , 0  , 0  , 0  ), // #259 {X:r16|r32}
  ISIGNATURE(1, 1, 1, 0, 26 , 0  , 0  , 0  , 0  , 0  ), // #260 {X:r8lo|r8hi|m8|r16|m16|r32|m32|r64|m64}
  ISIGNATURE(3, 1, 1, 0, 118, 27 , 27 , 0  , 0  , 0  ), // #261 {X:xmm, R:i8, R:i8}
  ISIGNATURE(2, 1, 1, 0, 118, 65 , 0  , 0  , 0  , 0  ), //      {X:xmm, R:xmm}
  ISIGNATURE(0, 1, 1, 0, 0  , 0  , 0  , 0  , 0  , 0  ), // #263 {}
  ISIGNATURE(1, 1, 1, 0, 111, 0  , 0  , 0  , 0  , 0  ), // #264 {X:fp}
  ISIGNATURE(0, 1, 1, 0, 0  , 0  , 0  , 0  , 0  , 0  ), // #265 {}
  ISIGNATURE(1, 1, 1, 0, 142, 0  , 0  , 0  , 0  , 0  ), // #266 {X:m32|m64|fp}
  ISIGNATURE(2, 1, 1, 0, 118, 65 , 0  , 0  , 0  , 0  ), // #267 {X:xmm, R:xmm}
  ISIGNATURE(4, 1, 1, 0, 118, 65 , 27 , 27 , 0  , 0  ), //      {X:xmm, R:xmm, R:i8, R:i8}
  ISIGNATURE(2, 1, 0, 1, 143, 144, 0  , 0  , 0  , 0  ), // #269 {R:<cx|ecx>, R:rel8}
  ISIGNATURE(2, 0, 1, 1, 145, 144, 0  , 0  , 0  , 0  ), //      {R:<ecx|rcx>, R:rel8}
  ISIGNATURE(1, 1, 1, 0, 146, 0  , 0  , 0  , 0  , 0  ), // #271 {X:rel8|rel32|r64|m64}
  ISIGNATURE(1, 1, 0, 0, 39 , 0  , 0  , 0  , 0  , 0  ), //      {R:r32|m32}
  ISIGNATURE(2, 1, 1, 0, 116, 147, 0  , 0  , 0  , 0  ), // #273 {W:k, R:k|m8|r32|r64|r8lo|r8hi|r16}
  ISIGNATURE(2, 1, 1, 0, 148, 149, 0  , 0  , 0  , 0  ), //      {W:m8|r32|r64|r8lo|r8hi|r16, R:k}
  ISIGNATURE(2, 1, 1, 0, 116, 150, 0  , 0  , 0  , 0  ), // #275 {W:k, R:k|m32|r32|r64}
  ISIGNATURE(2, 1, 1, 0, 151, 149, 0  , 0  , 0  , 0  ), //      {W:m32|r32|r64, R:k}
  ISIGNATURE(2, 1, 1, 0, 116, 152, 0  , 0  , 0  , 0  ), // #277 {W:k, R:k|m64|r64}
  ISIGNATURE(2, 1, 1, 0, 7  , 149, 0  , 0  , 0  , 0  ), //      {W:m64|r64, R:k}
  ISIGNATURE(2, 1, 1, 0, 116, 153, 0  , 0  , 0  , 0  ), // #279 {W:k, R:k|m16|r32|r64|r16}
  ISIGNATURE(2, 1, 1, 0, 154, 149, 0  , 0  , 0  , 0  ), //      {W:m16|r32|r64|r16, R:k}
  ISIGNATURE(2, 1, 0, 1, 155, 144, 0  , 0  , 0  , 0  ), // #281 {X:<cx|ecx>, R:rel8}
  ISIGNATURE(2, 0, 1, 1, 156, 144, 0  , 0  , 0  , 0  ), //      {X:<ecx|rcx>, R:rel8}
  ISIGNATURE(2, 1, 1, 0, 157, 158, 0  , 0  , 0  , 0  ), // #283 {W:mm|xmm, R:r32|m32|r64}
  ISIGNATURE(2, 1, 1, 0, 151, 159, 0  , 0  , 0  , 0  ), //      {W:r32|m32|r64, R:mm|xmm}
  ISIGNATURE(2, 1, 1, 0, 66 , 67 , 0  , 0  , 0  , 0  ), // #285 {W:xmm, R:xmm|m64}
  ISIGNATURE(2, 1, 1, 0, 60 , 65 , 0  , 0  , 0  , 0  ), //      {W:m64, R:xmm}
  ISIGNATURE(2, 1, 1, 0, 66 , 128, 0  , 0  , 0  , 0  ), // #287 {W:xmm, R:xmm|m32}
  ISIGNATURE(2, 1, 1, 0, 59 , 65 , 0  , 0  , 0  , 0  ), // #288 {W:m32, R:xmm}
  ISIGNATURE(2, 1, 1, 0, 160, 36 , 0  , 0  , 0  , 0  ), // #289 {W:r16|r32|r64, R:r8lo|r8hi|m8}
  ISIGNATURE(2, 1, 1, 0, 161, 12 , 0  , 0  , 0  , 0  ), //      {W:r32|r64, R:r16|m16}
  ISIGNATURE(4, 1, 1, 1, 13 , 13 , 39 , 162, 0  , 0  ), // #291 {W:r32, W:r32, R:r32|m32, R:<edx>}
  ISIGNATURE(4, 0, 1, 1, 19 , 19 , 16 , 163, 0  , 0  ), //      {W:r64, W:r64, R:r64|m64, R:<rdx>}
  ISIGNATURE(0, 1, 1, 0, 0  , 0  , 0  , 0  , 0  , 0  ), // #293 {}
  ISIGNATURE(1, 1, 1, 0, 164, 0  , 0  , 0  , 0  , 0  ), //      {R:r16|m16|r32|m32}
  ISIGNATURE(2, 1, 1, 0, 165, 166, 0  , 0  , 0  , 0  ), // #295 {X:mm, R:mm|m64}
  ISIGNATURE(2, 1, 1, 0, 118, 69 , 0  , 0  , 0  , 0  ), // #296 {X:xmm, R:xmm|m128}
  ISIGNATURE(3, 1, 1, 0, 165, 166, 27 , 0  , 0  , 0  ), // #297 {X:mm, R:mm|m64, R:i8}
  ISIGNATURE(3, 1, 1, 0, 118, 69 , 27 , 0  , 0  , 0  ), // #298 {X:xmm, R:xmm|m128, R:i8}
  ISIGNATURE(3, 1, 1, 0, 161, 64 , 27 , 0  , 0  , 0  ), // #299 {W:r32|r64, R:mm, R:i8}
  ISIGNATURE(3, 1, 1, 0, 154, 65 , 27 , 0  , 0  , 0  ), // #300 {W:r32|r64|m16|r16, R:xmm, R:i8}
  ISIGNATURE(2, 1, 1, 0, 61 , 166, 0  , 0  , 0  , 0  ), // #301 {W:mm, R:mm|m64}
  ISIGNATURE(2, 1, 1, 0, 66 , 69 , 0  , 0  , 0  , 0  ), //      {W:xmm, R:xmm|m128}
  ISIGNATURE(2, 1, 1, 0, 165, 167, 0  , 0  , 0  , 0  ), // #303 {X:mm, R:i8|mm|m64}
  ISIGNATURE(2, 1, 1, 0, 118, 79 , 0  , 0  , 0  , 0  ), //      {X:xmm, R:i8|xmm|m128}
  ISIGNATURE(0, 1, 1, 0, 0  , 0  , 0  , 0  , 0  , 0  ), // #305 {}
  ISIGNATURE(1, 1, 1, 0, 168, 0  , 0  , 0  , 0  , 0  ), //      {X:i16}
  ISIGNATURE(3, 1, 1, 0, 13 , 39 , 27 , 0  , 0  , 0  ), // #307 {W:r32, R:r32|m32, R:i8}
  ISIGNATURE(3, 0, 1, 0, 19 , 16 , 27 , 0  , 0  , 0  ), //      {W:r64, R:r64|m64, R:i8}
  ISIGNATURE(4, 1, 1, 0, 66 , 65 , 69 , 65 , 0  , 0  ), // #309 {W:xmm, R:xmm, R:xmm|m128, R:xmm}
  ISIGNATURE(4, 1, 1, 0, 71 , 74 , 72 , 74 , 0  , 0  ), //      {W:ymm, R:ymm, R:ymm|m256, R:ymm}
  ISIGNATURE(2, 1, 1, 0, 66 , 169, 0  , 0  , 0  , 0  ), // #311 {W:xmm, R:xmm|m128|ymm|m256}
  ISIGNATURE(2, 1, 1, 0, 71 , 76 , 0  , 0  , 0  , 0  ), //      {W:ymm, R:zmm|m512}
  ISIGNATURE(2, 1, 1, 0, 161, 128, 0  , 0  , 0  , 0  ), // #313 {W:r32|r64, R:xmm|m32}
  ISIGNATURE(2, 0, 1, 0, 19 , 67 , 0  , 0  , 0  , 0  ), //      {W:r64, R:xmm|m64}
  ISIGNATURE(2, 1, 1, 0, 13 , 128, 0  , 0  , 0  , 0  ), // #315 {W:r32, R:xmm|m32}
  ISIGNATURE(2, 0, 1, 0, 19 , 67 , 0  , 0  , 0  , 0  ), //      {W:r64, R:xmm|m64}
  ISIGNATURE(4, 1, 1, 0, 66 , 65 , 65 , 67 , 0  , 0  ), // #317 {W:xmm, R:xmm, R:xmm, R:xmm|m64}
  ISIGNATURE(4, 1, 1, 0, 66 , 65 , 67 , 65 , 0  , 0  ), //      {W:xmm, R:xmm, R:xmm|m64, R:xmm}
  ISIGNATURE(4, 1, 1, 0, 66 , 65 , 65 , 128, 0  , 0  ), // #319 {W:xmm, R:xmm, R:xmm, R:xmm|m32}
  ISIGNATURE(4, 1, 1, 0, 66 , 65 , 128, 65 , 0  , 0  ), //      {W:xmm, R:xmm, R:xmm|m32, R:xmm}
  ISIGNATURE(4, 1, 1, 0, 71 , 74 , 69 , 27 , 0  , 0  ), // #321 {W:ymm, R:ymm, R:xmm|m128, R:i8}
  ISIGNATURE(4, 1, 1, 0, 75 , 78 , 69 , 27 , 0  , 0  ), //      {W:zmm, R:zmm, R:xmm|m128, R:i8}
  ISIGNATURE(2, 1, 1, 0, 151, 65 , 0  , 0  , 0  , 0  ), // #323 {W:r32|m32|r64, R:xmm}
  ISIGNATURE(2, 1, 1, 0, 66 , 158, 0  , 0  , 0  , 0  ), //      {W:xmm, R:r32|m32|r64}
  ISIGNATURE(2, 1, 1, 0, 60 , 65 , 0  , 0  , 0  , 0  ), // #325 {W:m64, R:xmm}
  ISIGNATURE(3, 1, 1, 0, 66 , 65 , 57 , 0  , 0  , 0  ), //      {W:xmm, R:xmm, R:m64}
  ISIGNATURE(2, 1, 1, 0, 170, 171, 0  , 0  , 0  , 0  ), // #327 {W:xmm|ymm|zmm, R:xmm|m8}
  ISIGNATURE(2, 1, 1, 0, 170, 172, 0  , 0  , 0  , 0  ), //      {W:xmm|ymm|zmm, R:r32|r64}
  ISIGNATURE(2, 1, 1, 0, 170, 128, 0  , 0  , 0  , 0  ), // #329 {W:xmm|ymm|zmm, R:xmm|m32}
  ISIGNATURE(2, 1, 1, 0, 170, 172, 0  , 0  , 0  , 0  ), //      {W:xmm|ymm|zmm, R:r32|r64}
  ISIGNATURE(2, 1, 1, 0, 170, 129, 0  , 0  , 0  , 0  ), // #331 {W:xmm|ymm|zmm, R:xmm|m16}
  ISIGNATURE(2, 1, 1, 0, 170, 172, 0  , 0  , 0  , 0  ), //      {W:xmm|ymm|zmm, R:r32|r64}
  ISIGNATURE(3, 1, 1, 0, 66 , 173, 27 , 0  , 0  , 0  ), // #333 {W:xmm, R:r32|m8|r64|r8lo|r8hi|r16, R:i8}
  ISIGNATURE(4, 1, 1, 0, 66 , 65 , 173, 27 , 0  , 0  ), //      {W:xmm, R:xmm, R:r32|m8|r64|r8lo|r8hi|r16, R:i8}
  ISIGNATURE(3, 1, 1, 0, 66 , 158, 27 , 0  , 0  , 0  ), // #335 {W:xmm, R:r32|m32|r64, R:i8}
  ISIGNATURE(4, 1, 1, 0, 66 , 65 , 158, 27 , 0  , 0  ), //      {W:xmm, R:xmm, R:r32|m32|r64, R:i8}
  ISIGNATURE(3, 0, 1, 0, 66 , 16 , 27 , 0  , 0  , 0  ), // #337 {W:xmm, R:r64|m64, R:i8}
  ISIGNATURE(4, 0, 1, 0, 66 , 65 , 16 , 27 , 0  , 0  ), //      {W:xmm, R:xmm, R:r64|m64, R:i8}
  ISIGNATURE(3, 1, 1, 0, 66 , 65 , 69 , 0  , 0  , 0  ), // #339 {W:xmm, R:xmm, R:xmm|m128}
  ISIGNATURE(3, 1, 1, 0, 66 , 69 , 174, 0  , 0  , 0  ), //      {W:xmm, R:xmm|m128, R:i8|xmm}
  ISIGNATURE(2, 1, 1, 0, 175, 65 , 0  , 0  , 0  , 0  ), // #341 {W:vm64x|vm64y, R:xmm}
  ISIGNATURE(2, 1, 1, 0, 137, 74 , 0  , 0  , 0  , 0  ), //      {W:vm64z, R:ymm}
  ISIGNATURE(3, 1, 1, 0, 66 , 65 , 69 , 0  , 0  , 0  ), // #343 {W:xmm, R:xmm, R:xmm|m128}
  ISIGNATURE(3, 1, 1, 0, 66 , 69 , 65 , 0  , 0  , 0  ), //      {W:xmm, R:xmm|m128, R:xmm}
  ISIGNATURE(2, 1, 1, 0, 65 , 69 , 0  , 0  , 0  , 0  ), // #345 {R:xmm, R:xmm|m128}
  ISIGNATURE(2, 1, 1, 0, 74 , 72 , 0  , 0  , 0  , 0  ), //      {R:ymm, R:ymm|m256}
  ISIGNATURE(2, 1, 1, 0, 132, 176, 0  , 0  , 0  , 0  ), // #347 {W:vm32x, R:xmm|ymm}
  ISIGNATURE(2, 1, 1, 0, 133, 78 , 0  , 0  , 0  , 0  ), //      {W:vm32y, R:zmm}
  ISIGNATURE(1, 1, 0, 1, 44 , 0  , 0  , 0  , 0  , 0  ), // #349 {X:<ax>}
  ISIGNATURE(2, 1, 0, 1, 44 , 27 , 0  , 0  , 0  , 0  ), // #350 {X:<ax>, R:i8}
  ISIGNATURE(2, 1, 1, 0, 118, 67 , 0  , 0  , 0  , 0  ), // #351 {X:xmm, R:xmm|m64}
  ISIGNATURE(2, 1, 1, 0, 118, 128, 0  , 0  , 0  , 0  ), // #352 {X:xmm, R:xmm|m32}
  ISIGNATURE(3, 1, 1, 1, 118, 69 , 177, 0  , 0  , 0  ), // #353 {X:xmm, R:xmm|m128, R:<xmm0>}
  ISIGNATURE(1, 1, 1, 0, 178, 0  , 0  , 0  , 0  , 0  ), // #354 {X:r32|r64}
  ISIGNATURE(1, 1, 1, 1, 44 , 0  , 0  , 0  , 0  , 0  ), // #355 {X:<ax>}
  ISIGNATURE(2, 1, 1, 2, 46 , 88 , 0  , 0  , 0  , 0  ), // #356 {W:<edx>, R:<eax>}
  ISIGNATURE(1, 0, 1, 1, 49 , 0  , 0  , 0  , 0  , 0  ), // #357 {X:<rax>}
  ISIGNATURE(1, 1, 1, 0, 179, 0  , 0  , 0  , 0  , 0  ), // #358 {R:mem}
  ISIGNATURE(1, 1, 1, 1, 180, 0  , 0  , 0  , 0  , 0  ), // #359 {R:<ds:[zax]>}
  ISIGNATURE(2, 1, 1, 2, 181, 182, 0  , 0  , 0  , 0  ), // #360 {X:<ds:[zsi]>, X:<es:[zdi]>}
  ISIGNATURE(3, 1, 1, 0, 118, 67 , 27 , 0  , 0  , 0  ), // #361 {X:xmm, R:xmm|m64, R:i8}
  ISIGNATURE(3, 1, 1, 0, 118, 128, 27 , 0  , 0  , 0  ), // #362 {X:xmm, R:xmm|m32, R:i8}
  ISIGNATURE(5, 0, 1, 4, 183, 92 , 49 , 184, 185, 0  ), // #363 {X:m128, X:<rdx>, X:<rax>, R:<rcx>, R:<rbx>}
  ISIGNATURE(5, 1, 1, 4, 186, 91 , 47 , 187, 188, 0  ), // #364 {X:m64, X:<edx>, X:<eax>, R:<ecx>, R:<ebx>}
  ISIGNATURE(2, 1, 1, 0, 65 , 67 , 0  , 0  , 0  , 0  ), // #365 {R:xmm, R:xmm|m64}
  ISIGNATURE(2, 1, 1, 0, 65 , 128, 0  , 0  , 0  , 0  ), // #366 {R:xmm, R:xmm|m32}
  ISIGNATURE(4, 1, 1, 4, 47 , 189, 190, 46 , 0  , 0  ), // #367 {X:<eax>, W:<ebx>, X:<ecx>, W:<edx>}
  ISIGNATURE(2, 0, 1, 2, 48 , 89 , 0  , 0  , 0  , 0  ), // #368 {W:<rdx>, R:<rax>}
  ISIGNATURE(2, 1, 1, 0, 61 , 69 , 0  , 0  , 0  , 0  ), // #369 {W:mm, R:xmm|m128}
  ISIGNATURE(2, 1, 1, 0, 66 , 166, 0  , 0  , 0  , 0  ), // #370 {W:xmm, R:mm|m64}
  ISIGNATURE(2, 1, 1, 0, 61 , 67 , 0  , 0  , 0  , 0  ), // #371 {W:mm, R:xmm|m64}
  ISIGNATURE(2, 1, 1, 0, 161, 67 , 0  , 0  , 0  , 0  ), // #372 {W:r32|r64, R:xmm|m64}
  ISIGNATURE(2, 1, 1, 0, 66 , 42 , 0  , 0  , 0  , 0  ), // #373 {W:xmm, R:r32|m32|r64|m64}
  ISIGNATURE(2, 1, 1, 2, 45 , 87 , 0  , 0  , 0  , 0  ), // #374 {W:<dx>, R:<ax>}
  ISIGNATURE(1, 1, 1, 1, 47 , 0  , 0  , 0  , 0  , 0  ), // #375 {X:<eax>}
  ISIGNATURE(2, 1, 1, 0, 168, 27 , 0  , 0  , 0  , 0  ), // #376 {X:i16, R:i8}
  ISIGNATURE(3, 1, 1, 0, 151, 65 , 27 , 0  , 0  , 0  ), // #377 {W:r32|m32|r64, R:xmm, R:i8}
  ISIGNATURE(1, 1, 1, 0, 191, 0  , 0  , 0  , 0  , 0  ), // #378 {X:m80}
  ISIGNATURE(1, 1, 1, 0, 192, 0  , 0  , 0  , 0  , 0  ), // #379 {X:m16|m32}
  ISIGNATURE(1, 1, 1, 0, 193, 0  , 0  , 0  , 0  , 0  ), // #380 {X:m16|m32|m64}
  ISIGNATURE(1, 1, 1, 0, 194, 0  , 0  , 0  , 0  , 0  ), // #381 {X:m32|m64|m80|fp}
  ISIGNATURE(1, 1, 1, 0, 195, 0  , 0  , 0  , 0  , 0  ), // #382 {X:m16}
  ISIGNATURE(1, 1, 1, 0, 196, 0  , 0  , 0  , 0  , 0  ), // #383 {X:mem}
  ISIGNATURE(1, 1, 1, 0, 197, 0  , 0  , 0  , 0  , 0  ), // #384 {X:ax|m16}
  ISIGNATURE(1, 0, 1, 0, 196, 0  , 0  , 0  , 0  , 0  ), // #385 {X:mem}
  ISIGNATURE(2, 1, 1, 0, 198, 199, 0  , 0  , 0  , 0  ), // #386 {W:al|ax|eax, R:i8|dx}
  ISIGNATURE(2, 1, 1, 0, 200, 201, 0  , 0  , 0  , 0  ), // #387 {W:es:[zdi], R:dx}
  ISIGNATURE(1, 1, 1, 0, 202, 0  , 0  , 0  , 0  , 0  ), // #388 {X:i8}
  ISIGNATURE(1, 1, 1, 0, 203, 0  , 0  , 0  , 0  , 0  ), // #389 {X:rel8|rel32}
  ISIGNATURE(1, 1, 1, 0, 204, 0  , 0  , 0  , 0  , 0  ), // #390 {X:rel8}
  ISIGNATURE(3, 1, 1, 0, 116, 149, 149, 0  , 0  , 0  ), // #391 {W:k, R:k, R:k}
  ISIGNATURE(2, 1, 1, 0, 116, 149, 0  , 0  , 0  , 0  ), // #392 {W:k, R:k}
  ISIGNATURE(2, 1, 1, 0, 149, 149, 0  , 0  , 0  , 0  ), // #393 {R:k, R:k}
  ISIGNATURE(3, 1, 1, 0, 116, 149, 27 , 0  , 0  , 0  ), // #394 {W:k, R:k, R:i8}
  ISIGNATURE(1, 1, 1, 1, 205, 0  , 0  , 0  , 0  , 0  ), // #395 {W:<ah>}
  ISIGNATURE(1, 1, 1, 0, 56 , 0  , 0  , 0  , 0  , 0  ), // #396 {R:m32}
  ISIGNATURE(2, 1, 1, 0, 160, 179, 0  , 0  , 0  , 0  ), // #397 {W:r16|r32|r64, R:mem}
  ISIGNATURE(2, 1, 1, 2, 206, 181, 0  , 0  , 0  , 0  ), // #398 {W:<al|ax|eax|rax>, X:<ds:[zsi]>}
  ISIGNATURE(3, 1, 1, 1, 118, 65 , 207, 0  , 0  , 0  ), // #399 {X:xmm, R:xmm, R:<ds:[zdi]>}
  ISIGNATURE(3, 1, 1, 1, 165, 64 , 207, 0  , 0  , 0  ), // #400 {X:mm, R:mm, R:<ds:[zdi]>}
  ISIGNATURE(2, 1, 1, 0, 61 , 65 , 0  , 0  , 0  , 0  ), // #401 {W:mm, R:xmm}
  ISIGNATURE(2, 1, 1, 0, 66 , 65 , 0  , 0  , 0  , 0  ), // #402 {W:xmm, R:xmm}
  ISIGNATURE(2, 1, 1, 0, 161, 65 , 0  , 0  , 0  , 0  ), // #403 {W:r32|r64, R:xmm}
  ISIGNATURE(2, 1, 1, 0, 60 , 64 , 0  , 0  , 0  , 0  ), // #404 {W:m64, R:mm}
  ISIGNATURE(2, 1, 1, 0, 66 , 64 , 0  , 0  , 0  , 0  ), // #405 {W:xmm, R:mm}
  ISIGNATURE(2, 1, 1, 2, 182, 181, 0  , 0  , 0  , 0  ), // #406 {X:<es:[zdi]>, X:<ds:[zsi]>}
  ISIGNATURE(2, 0, 1, 0, 19 , 39 , 0  , 0  , 0  , 0  ), // #407 {W:r64, R:r32|m32}
  ISIGNATURE(2, 1, 1, 0, 208, 209, 0  , 0  , 0  , 0  ), // #408 {X:i8|dx, R:al|ax|eax}
  ISIGNATURE(2, 1, 1, 0, 201, 210, 0  , 0  , 0  , 0  ), // #409 {R:dx, R:ds:[zsi]}
  ISIGNATURE(6, 1, 1, 3, 65 , 69 , 27 , 211, 88 , 162), // #410 {R:xmm, R:xmm|m128, R:i8, W:<ecx>, R:<eax>, R:<edx>}
  ISIGNATURE(6, 1, 1, 3, 65 , 69 , 27 , 212, 88 , 162), // #411 {R:xmm, R:xmm|m128, R:i8, W:<xmm0>, R:<eax>, R:<edx>}
  ISIGNATURE(4, 1, 1, 1, 65 , 69 , 27 , 211, 0  , 0  ), // #412 {R:xmm, R:xmm|m128, R:i8, W:<ecx>}
  ISIGNATURE(4, 1, 1, 1, 65 , 69 , 27 , 212, 0  , 0  ), // #413 {R:xmm, R:xmm|m128, R:i8, W:<xmm0>}
  ISIGNATURE(3, 1, 1, 0, 148, 65 , 27 , 0  , 0  , 0  ), // #414 {W:r32|m8|r64|r8lo|r8hi|r16, R:xmm, R:i8}
  ISIGNATURE(3, 0, 1, 0, 7  , 65 , 27 , 0  , 0  , 0  ), // #415 {W:r64|m64, R:xmm, R:i8}
  ISIGNATURE(3, 1, 1, 0, 118, 173, 27 , 0  , 0  , 0  ), // #416 {X:xmm, R:r32|m8|r64|r8lo|r8hi|r16, R:i8}
  ISIGNATURE(3, 1, 1, 0, 118, 158, 27 , 0  , 0  , 0  ), // #417 {X:xmm, R:r32|m32|r64, R:i8}
  ISIGNATURE(3, 0, 1, 0, 118, 16 , 27 , 0  , 0  , 0  ), // #418 {X:xmm, R:r64|m64, R:i8}
  ISIGNATURE(3, 1, 1, 0, 213, 214, 27 , 0  , 0  , 0  ), // #419 {X:mm|xmm, R:r32|m16|r64|r16, R:i8}
  ISIGNATURE(2, 1, 1, 0, 161, 159, 0  , 0  , 0  , 0  ), // #420 {W:r32|r64, R:mm|xmm}
  ISIGNATURE(0, 1, 0, 0, 0  , 0  , 0  , 0  , 0  , 0  ), // #421 {}
  ISIGNATURE(0, 0, 1, 0, 0  , 0  , 0  , 0  , 0  , 0  ), // #422 {}
  ISIGNATURE(3, 1, 1, 0, 61 , 166, 27 , 0  , 0  , 0  ), // #423 {W:mm, R:mm|m64, R:i8}
  ISIGNATURE(2, 1, 1, 0, 118, 27 , 0  , 0  , 0  , 0  ), // #424 {X:xmm, R:i8}
  ISIGNATURE(2, 1, 1, 0, 26 , 113, 0  , 0  , 0  , 0  ), // #425 {X:r8lo|r8hi|m8|r16|m16|r32|m32|r64|m64, R:cl|i8}
  ISIGNATURE(1, 0, 1, 0, 161, 0  , 0  , 0  , 0  , 0  ), // #426 {W:r32|r64}
  ISIGNATURE(1, 1, 1, 0, 160, 0  , 0  , 0  , 0  , 0  ), // #427 {W:r16|r32|r64}
  ISIGNATURE(2, 1, 1, 2, 46 , 215, 0  , 0  , 0  , 0  ), // #428 {W:<edx>, W:<eax>}
  ISIGNATURE(3, 1, 1, 3, 46 , 215, 211, 0  , 0  , 0  ), // #429 {W:<edx>, W:<eax>, W:<ecx>}
  ISIGNATURE(3, 1, 1, 0, 66 , 67 , 27 , 0  , 0  , 0  ), // #430 {W:xmm, R:xmm|m64, R:i8}
  ISIGNATURE(3, 1, 1, 0, 66 , 128, 27 , 0  , 0  , 0  ), // #431 {W:xmm, R:xmm|m32, R:i8}
  ISIGNATURE(1, 1, 1, 1, 216, 0  , 0  , 0  , 0  , 0  ), // #432 {R:<ah>}
  ISIGNATURE(2, 1, 1, 2, 217, 182, 0  , 0  , 0  , 0  ), // #433 {R:<al|ax|eax|rax>, X:<es:[zdi]>}
  ISIGNATURE(1, 1, 1, 0, 1  , 0  , 0  , 0  , 0  , 0  ), // #434 {W:r8lo|r8hi|m8}
  ISIGNATURE(1, 1, 1, 0, 59 , 0  , 0  , 0  , 0  , 0  ), // #435 {W:m32}
  ISIGNATURE(2, 1, 1, 2, 182, 217, 0  , 0  , 0  , 0  ), // #436 {X:<es:[zdi]>, R:<al|ax|eax|rax>}
  ISIGNATURE(3, 1, 1, 0, 66 , 65 , 67 , 0  , 0  , 0  ), // #437 {W:xmm, R:xmm, R:xmm|m64}
  ISIGNATURE(3, 1, 1, 0, 66 , 65 , 128, 0  , 0  , 0  ), // #438 {W:xmm, R:xmm, R:xmm|m32}
  ISIGNATURE(2, 1, 1, 0, 71 , 102, 0  , 0  , 0  , 0  ), // #439 {W:ymm, R:m128}
  ISIGNATURE(2, 1, 1, 0, 218, 67 , 0  , 0  , 0  , 0  ), // #440 {W:ymm|zmm, R:xmm|m64}
  ISIGNATURE(2, 1, 1, 0, 218, 102, 0  , 0  , 0  , 0  ), // #441 {W:ymm|zmm, R:m128}
  ISIGNATURE(2, 1, 1, 0, 75 , 103, 0  , 0  , 0  , 0  ), // #442 {W:zmm, R:m256}
  ISIGNATURE(2, 1, 1, 0, 170, 67 , 0  , 0  , 0  , 0  ), // #443 {W:xmm|ymm|zmm, R:xmm|m64}
  ISIGNATURE(4, 1, 1, 0, 114, 65 , 67 , 27 , 0  , 0  ), // #444 {W:xmm|k, R:xmm, R:xmm|m64, R:i8}
  ISIGNATURE(4, 1, 1, 0, 114, 65 , 128, 27 , 0  , 0  ), // #445 {W:xmm|k, R:xmm, R:xmm|m32, R:i8}
  ISIGNATURE(3, 1, 1, 0, 66 , 65 , 42 , 0  , 0  , 0  ), // #446 {W:xmm, R:xmm, R:r32|m32|r64|m64}
  ISIGNATURE(3, 1, 1, 0, 70 , 219, 27 , 0  , 0  , 0  ), // #447 {W:xmm|m128, R:ymm|zmm, R:i8}
  ISIGNATURE(4, 1, 1, 0, 118, 65 , 67 , 27 , 0  , 0  ), // #448 {X:xmm, R:xmm, R:xmm|m64, R:i8}
  ISIGNATURE(4, 1, 1, 0, 118, 65 , 128, 27 , 0  , 0  ), // #449 {X:xmm, R:xmm, R:xmm|m32, R:i8}
  ISIGNATURE(3, 1, 1, 0, 118, 65 , 67 , 0  , 0  , 0  ), // #450 {X:xmm, R:xmm, R:xmm|m64}
  ISIGNATURE(3, 1, 1, 0, 118, 65 , 128, 0  , 0  , 0  ), // #451 {X:xmm, R:xmm, R:xmm|m32}
  ISIGNATURE(3, 1, 1, 0, 116, 220, 27 , 0  , 0  , 0  ), // #452 {W:k, R:xmm|m128|ymm|m256|zmm|m512, R:i8}
  ISIGNATURE(3, 1, 1, 0, 116, 67 , 27 , 0  , 0  , 0  ), // #453 {W:k, R:xmm|m64, R:i8}
  ISIGNATURE(3, 1, 1, 0, 116, 128, 27 , 0  , 0  , 0  ), // #454 {W:k, R:xmm|m32, R:i8}
  ISIGNATURE(1, 1, 1, 0, 81 , 0  , 0  , 0  , 0  , 0  ), // #455 {R:vm32y}
  ISIGNATURE(1, 1, 1, 0, 82 , 0  , 0  , 0  , 0  , 0  ), // #456 {R:vm32z}
  ISIGNATURE(1, 1, 1, 0, 85 , 0  , 0  , 0  , 0  , 0  ), // #457 {R:vm64z}
  ISIGNATURE(4, 1, 1, 0, 75 , 78 , 72 , 27 , 0  , 0  ), // #458 {W:zmm, R:zmm, R:ymm|m256, R:i8}
  ISIGNATURE(4, 1, 1, 0, 66 , 65 , 128, 27 , 0  , 0  ), // #459 {W:xmm, R:xmm, R:xmm|m32, R:i8}
  ISIGNATURE(3, 1, 1, 1, 65 , 65 , 207, 0  , 0  , 0  ), // #460 {R:xmm, R:xmm, R:<ds:[zdi]>}
  ISIGNATURE(2, 1, 1, 0, 161, 176, 0  , 0  , 0  , 0  ), // #461 {W:r32|r64, R:xmm|ymm}
  ISIGNATURE(2, 1, 1, 0, 170, 149, 0  , 0  , 0  , 0  ), // #462 {W:xmm|ymm|zmm, R:k}
  ISIGNATURE(2, 1, 1, 0, 170, 123, 0  , 0  , 0  , 0  ), // #463 {W:xmm|ymm|zmm, R:xmm|m64|r64}
  ISIGNATURE(4, 1, 1, 0, 66 , 65 , 214, 27 , 0  , 0  ), // #464 {W:xmm, R:xmm, R:r32|m16|r64|r16, R:i8}
  ISIGNATURE(2, 1, 1, 0, 116, 221, 0  , 0  , 0  , 0  ), // #465 {W:k, R:xmm|ymm|zmm}
  ISIGNATURE(4, 1, 1, 0, 66 , 65 , 67 , 27 , 0  , 0  ), // #466 {W:xmm, R:xmm, R:xmm|m64, R:i8}
  ISIGNATURE(1, 0, 1, 0, 172, 0  , 0  , 0  , 0  , 0  ), // #467 {R:r32|r64}
  ISIGNATURE(3, 1, 1, 3, 187, 46 , 215, 0  , 0  , 0  ), // #468 {R:<ecx>, W:<edx>, W:<eax>}
  ISIGNATURE(3, 1, 1, 2, 196, 162, 88 , 0  , 0  , 0  ), // #469 {X:mem, R:<edx>, R:<eax>}
  ISIGNATURE(3, 0, 1, 2, 196, 162, 88 , 0  , 0  , 0  ), // #470 {X:mem, R:<edx>, R:<eax>}
  ISIGNATURE(3, 1, 1, 3, 187, 162, 88 , 0  , 0  , 0  )  // #471 {R:<ecx>, R:<edx>, R:<eax>}
};
#undef ISIGNATURE

#define FLAG(flag) X86Inst::kOp##flag
#define MEM(mem) X86Inst::kMemOp##mem
#define OSIGNATURE(flags, memFlags, extFlags, regId) \
  { uint32_t(flags), uint16_t(memFlags), uint8_t(extFlags), uint8_t(regId) }
static const X86Inst::OSignature _x86InstOSignatureData[] = {
  OSIGNATURE(0, 0, 0, 0xFF),
  OSIGNATURE(FLAG(W) | FLAG(GpbLo) | FLAG(GpbHi) | FLAG(Mem), MEM(M8), 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(GpbLo) | FLAG(GpbHi) | FLAG(I8), 0, 0, 0x00),
  OSIGNATURE(FLAG(W) | FLAG(Gpw) | FLAG(Mem), MEM(M16), 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Gpw) | FLAG(Seg) | FLAG(I16), 0, 0, 0x00),
  OSIGNATURE(FLAG(W) | FLAG(Gpd) | FLAG(Seg) | FLAG(Mem), MEM(M32), 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Gpd), 0, 0, 0x00),
  OSIGNATURE(FLAG(W) | FLAG(Gpq) | FLAG(Mem), MEM(M64), 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Gpq) | FLAG(Seg) | FLAG(I32), 0, 0, 0x00),
  OSIGNATURE(FLAG(W) | FLAG(GpbLo) | FLAG(GpbHi), 0, 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(GpbLo) | FLAG(GpbHi) | FLAG(Mem) | FLAG(I8), MEM(M8), 0, 0x00),
  OSIGNATURE(FLAG(W) | FLAG(Gpw) | FLAG(Seg), 0, 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Gpw) | FLAG(Mem), MEM(M16), 0, 0x00),
  OSIGNATURE(FLAG(W) | FLAG(Gpd), 0, 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Gpd) | FLAG(Seg) | FLAG(Mem) | FLAG(I32), MEM(M32), 0, 0x00),
  OSIGNATURE(FLAG(W) | FLAG(Gpq) | FLAG(Seg), 0, 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Gpq) | FLAG(Mem), MEM(M64), 0, 0x00),
  OSIGNATURE(FLAG(W) | FLAG(Gpw), 0, 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(I16), 0, 0, 0x00),
  OSIGNATURE(FLAG(W) | FLAG(Gpq), 0, 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Cr) | FLAG(Dr) | FLAG(I64), 0, 0, 0x00),
  OSIGNATURE(FLAG(W) | FLAG(Gpd) | FLAG(Mem), MEM(M32), 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(I32), 0, 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Cr) | FLAG(Dr), 0, 0, 0x00),
  OSIGNATURE(FLAG(W) | FLAG(Cr) | FLAG(Dr), 0, 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Gpq), 0, 0, 0x00),
  OSIGNATURE(FLAG(X) | FLAG(GpbLo) | FLAG(GpbHi) | FLAG(Gpw) | FLAG(Gpd) | FLAG(Gpq) | FLAG(Mem), MEM(M8) | MEM(M16) | MEM(M32) | MEM(M64), 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(I8), 0, 0, 0x00),
  OSIGNATURE(FLAG(X) | FLAG(Gpw) | FLAG(Mem), MEM(M16), 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Gpw) | FLAG(I16), 0, 0, 0x00),
  OSIGNATURE(FLAG(X) | FLAG(Gpd) | FLAG(Gpq) | FLAG(Mem), MEM(M32) | MEM(M64), 0, 0x00),
  OSIGNATURE(FLAG(X) | FLAG(GpbLo) | FLAG(GpbHi) | FLAG(Mem), MEM(M8), 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(GpbLo) | FLAG(GpbHi), 0, 0, 0x00),
  OSIGNATURE(FLAG(X) | FLAG(Gpd) | FLAG(Mem), MEM(M32), 0, 0x00),
  OSIGNATURE(FLAG(X) | FLAG(Gpq) | FLAG(Mem), MEM(M64), 0, 0x00),
  OSIGNATURE(FLAG(X) | FLAG(GpbLo) | FLAG(GpbHi), 0, 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(GpbLo) | FLAG(GpbHi) | FLAG(Mem), MEM(M8), 0, 0x00),
  OSIGNATURE(FLAG(X) | FLAG(Gpw), 0, 0, 0x00),
  OSIGNATURE(FLAG(X) | FLAG(Gpd), 0, 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Gpd) | FLAG(Mem), MEM(M32), 0, 0x00),
  OSIGNATURE(FLAG(X) | FLAG(Gpq), 0, 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(GpbLo) | FLAG(GpbHi) | FLAG(Gpw) | FLAG(Gpd) | FLAG(Gpq) | FLAG(Mem), MEM(M8) | MEM(M16) | MEM(M32) | MEM(M64), 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Gpd) | FLAG(Gpq) | FLAG(Mem), MEM(M32) | MEM(M64), 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Gpw), 0, 0, 0x00),
  OSIGNATURE(FLAG(X) | FLAG(Implicit) | FLAG(Gpw), 0, 0, 0x01),
  OSIGNATURE(FLAG(W) | FLAG(Implicit) | FLAG(Gpw), 0, 0, 0x04),
  OSIGNATURE(FLAG(W) | FLAG(Implicit) | FLAG(Gpd), 0, 0, 0x04),
  OSIGNATURE(FLAG(X) | FLAG(Implicit) | FLAG(Gpd), 0, 0, 0x01),
  OSIGNATURE(FLAG(W) | FLAG(Implicit) | FLAG(Gpq), 0, 0, 0x04),
  OSIGNATURE(FLAG(X) | FLAG(Implicit) | FLAG(Gpq), 0, 0, 0x01),
  OSIGNATURE(FLAG(R) | FLAG(Gpw) | FLAG(Mem) | FLAG(I8) | FLAG(I16), MEM(M16), 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Gpd) | FLAG(Mem) | FLAG(I8) | FLAG(I32), MEM(M32), 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Gpq) | FLAG(Mem) | FLAG(I8) | FLAG(I32), MEM(M64), 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(I8) | FLAG(I16), 0, 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(I8) | FLAG(I32), 0, 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Mem), MEM(M16), 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Mem), MEM(M32), 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Mem), MEM(M64), 0, 0x00),
  OSIGNATURE(FLAG(W) | FLAG(Mem), MEM(M16), 0, 0x00),
  OSIGNATURE(FLAG(W) | FLAG(Mem), MEM(M32), 0, 0x00),
  OSIGNATURE(FLAG(W) | FLAG(Mem), MEM(M64), 0, 0x00),
  OSIGNATURE(FLAG(W) | FLAG(Mm), 0, 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Gpq) | FLAG(Mm) | FLAG(Xmm) | FLAG(Mem), MEM(M64), 0, 0x00),
  OSIGNATURE(FLAG(W) | FLAG(Gpq) | FLAG(Mm) | FLAG(Xmm) | FLAG(Mem), MEM(M64), 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Mm), 0, 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Xmm), 0, 0, 0x00),
  OSIGNATURE(FLAG(W) | FLAG(Xmm), 0, 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Xmm) | FLAG(Mem), MEM(M64), 0, 0x00),
  OSIGNATURE(FLAG(W) | FLAG(Xmm) | FLAG(Mem), MEM(M64), 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Xmm) | FLAG(Mem), MEM(M128), 0, 0x00),
  OSIGNATURE(FLAG(W) | FLAG(Xmm) | FLAG(Mem), MEM(M128), 0, 0x00),
  OSIGNATURE(FLAG(W) | FLAG(Ymm), 0, 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Ymm) | FLAG(Mem), MEM(M256), 0, 0x00),
  OSIGNATURE(FLAG(W) | FLAG(Ymm) | FLAG(Mem), MEM(M256), 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Ymm), 0, 0, 0x00),
  OSIGNATURE(FLAG(W) | FLAG(Zmm), 0, 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Zmm) | FLAG(Mem), MEM(M512), 0, 0x00),
  OSIGNATURE(FLAG(W) | FLAG(Zmm) | FLAG(Mem), MEM(M512), 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Zmm), 0, 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Xmm) | FLAG(Mem) | FLAG(I8), MEM(M128), 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Vm), MEM(Vm32x), 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Vm), MEM(Vm32y), 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Vm), MEM(Vm32z), 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Vm), MEM(Vm64x), 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Vm), MEM(Vm64y), 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Vm), MEM(Vm64z), 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Implicit) | FLAG(GpbLo), 0, 0, 0x01),
  OSIGNATURE(FLAG(R) | FLAG(Implicit) | FLAG(Gpw), 0, 0, 0x01),
  OSIGNATURE(FLAG(R) | FLAG(Implicit) | FLAG(Gpd), 0, 0, 0x01),
  OSIGNATURE(FLAG(R) | FLAG(Implicit) | FLAG(Gpq), 0, 0, 0x01),
  OSIGNATURE(FLAG(X) | FLAG(Implicit) | FLAG(Gpw), 0, 0, 0x04),
  OSIGNATURE(FLAG(X) | FLAG(Implicit) | FLAG(Gpd), 0, 0, 0x04),
  OSIGNATURE(FLAG(X) | FLAG(Implicit) | FLAG(Gpq), 0, 0, 0x04),
  OSIGNATURE(FLAG(W) | FLAG(Gpw) | FLAG(Gpq) | FLAG(Mem), MEM(M16) | MEM(M64), 0, 0x00),
  OSIGNATURE(FLAG(W) | FLAG(Seg), 0, 0, 0x1A),
  OSIGNATURE(FLAG(W) | FLAG(Seg), 0, 0, 0x60),
  OSIGNATURE(FLAG(X) | FLAG(Gpw) | FLAG(Gpq) | FLAG(Mem) | FLAG(I8) | FLAG(I16) | FLAG(I32), MEM(M16) | MEM(M64), 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Seg), 0, 0, 0x1E),
  OSIGNATURE(FLAG(R) | FLAG(Seg), 0, 0, 0x60),
  OSIGNATURE(FLAG(R) | FLAG(Vm), MEM(Vm64x) | MEM(Vm64y), 0, 0x00),
  OSIGNATURE(FLAG(W) | FLAG(Mem), MEM(M128), 0, 0x00),
  OSIGNATURE(FLAG(W) | FLAG(Mem), MEM(M256), 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Mem), MEM(M128), 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Mem), MEM(M256), 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(I4), 0, 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Gpw) | FLAG(I8), 0, 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Gpd) | FLAG(I8), 0, 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Gpq) | FLAG(I8), 0, 0, 0x00),
  OSIGNATURE(FLAG(X) | FLAG(Mem), MEM(M32) | MEM(M64), 0, 0x00),
  OSIGNATURE(FLAG(X) | FLAG(Fp), 0, 0, 0x01),
  OSIGNATURE(FLAG(R) | FLAG(Fp), 0, 0, 0x00),
  OSIGNATURE(FLAG(X) | FLAG(Fp), 0, 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Fp), 0, 0, 0x01),
  OSIGNATURE(FLAG(R) | FLAG(GpbLo) | FLAG(I8), 0, 0, 0x02),
  OSIGNATURE(FLAG(W) | FLAG(K) | FLAG(Xmm), 0, 0, 0x00),
  OSIGNATURE(FLAG(W) | FLAG(K) | FLAG(Ymm), 0, 0, 0x00),
  OSIGNATURE(FLAG(W) | FLAG(K), 0, 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Xmm) | FLAG(Ymm) | FLAG(Mem), MEM(M64) | MEM(M128) | MEM(M256), 0, 0x00),
  OSIGNATURE(FLAG(X) | FLAG(Xmm), 0, 0, 0x00),
  OSIGNATURE(FLAG(X) | FLAG(Ymm), 0, 0, 0x00),
  OSIGNATURE(FLAG(X) | FLAG(Zmm), 0, 0, 0x00),
  OSIGNATURE(FLAG(W) | FLAG(Mem), MEM(M512), 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Mem), MEM(M512), 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Gpq) | FLAG(Xmm) | FLAG(Mem), MEM(M64), 0, 0x00),
  OSIGNATURE(FLAG(W) | FLAG(Mem), MEM(M32) | MEM(M64), 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Mem), MEM(M32) | MEM(M64), 0, 0x00),
  OSIGNATURE(FLAG(W) | FLAG(Xmm) | FLAG(Mem), MEM(M32), 0, 0x00),
  OSIGNATURE(FLAG(W) | FLAG(Xmm) | FLAG(Mem), MEM(M16), 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Xmm) | FLAG(Mem), MEM(M32), 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Xmm) | FLAG(Mem), MEM(M16), 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Xmm) | FLAG(Mem), MEM(M32) | MEM(M64), 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Xmm) | FLAG(Mem), MEM(M64) | MEM(M128), 0, 0x00),
  OSIGNATURE(FLAG(W) | FLAG(Vm), MEM(Vm32x), 0, 0x00),
  OSIGNATURE(FLAG(W) | FLAG(Vm), MEM(Vm32y), 0, 0x00),
  OSIGNATURE(FLAG(W) | FLAG(Vm), MEM(Vm32z), 0, 0x00),
  OSIGNATURE(FLAG(W) | FLAG(Vm), MEM(Vm64x), 0, 0x00),
  OSIGNATURE(FLAG(W) | FLAG(Vm), MEM(Vm64y), 0, 0x00),
  OSIGNATURE(FLAG(W) | FLAG(Vm), MEM(Vm64z), 0, 0x00),
  OSIGNATURE(FLAG(X) | FLAG(Gpq) | FLAG(Mem) | FLAG(I32) | FLAG(I64) | FLAG(Rel32), MEM(M64), 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(GpbLo) | FLAG(GpbHi) | FLAG(Gpw) | FLAG(Gpd) | FLAG(Mem), MEM(M8) | MEM(M16) | MEM(M32), 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(GpbLo) | FLAG(GpbHi) | FLAG(Gpq) | FLAG(Mem), MEM(M8) | MEM(M64), 0, 0x00),
  OSIGNATURE(FLAG(X) | FLAG(Gpw) | FLAG(Gpd), 0, 0, 0x00),
  OSIGNATURE(FLAG(X) | FLAG(Fp) | FLAG(Mem), MEM(M32) | MEM(M64), 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Implicit) | FLAG(Gpw) | FLAG(Gpd), 0, 0, 0x02),
  OSIGNATURE(FLAG(R) | FLAG(I32) | FLAG(I64) | FLAG(Rel8), 0, 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Implicit) | FLAG(Gpd) | FLAG(Gpq), 0, 0, 0x02),
  OSIGNATURE(FLAG(X) | FLAG(Gpq) | FLAG(Mem) | FLAG(I32) | FLAG(I64) | FLAG(Rel8) | FLAG(Rel32), MEM(M64), 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(GpbLo) | FLAG(GpbHi) | FLAG(Gpw) | FLAG(Gpd) | FLAG(Gpq) | FLAG(K) | FLAG(Mem), MEM(M8), 0, 0x00),
  OSIGNATURE(FLAG(W) | FLAG(GpbLo) | FLAG(GpbHi) | FLAG(Gpw) | FLAG(Gpd) | FLAG(Gpq) | FLAG(Mem), MEM(M8), 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(K), 0, 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Gpd) | FLAG(Gpq) | FLAG(K) | FLAG(Mem), MEM(M32), 0, 0x00),
  OSIGNATURE(FLAG(W) | FLAG(Gpd) | FLAG(Gpq) | FLAG(Mem), MEM(M32), 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Gpq) | FLAG(K) | FLAG(Mem), MEM(M64), 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Gpw) | FLAG(Gpd) | FLAG(Gpq) | FLAG(K) | FLAG(Mem), MEM(M16), 0, 0x00),
  OSIGNATURE(FLAG(W) | FLAG(Gpw) | FLAG(Gpd) | FLAG(Gpq) | FLAG(Mem), MEM(M16), 0, 0x00),
  OSIGNATURE(FLAG(X) | FLAG(Implicit) | FLAG(Gpw) | FLAG(Gpd), 0, 0, 0x02),
  OSIGNATURE(FLAG(X) | FLAG(Implicit) | FLAG(Gpd) | FLAG(Gpq), 0, 0, 0x02),
  OSIGNATURE(FLAG(W) | FLAG(Mm) | FLAG(Xmm), 0, 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Gpd) | FLAG(Gpq) | FLAG(Mem), MEM(M32), 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Mm) | FLAG(Xmm), 0, 0, 0x00),
  OSIGNATURE(FLAG(W) | FLAG(Gpw) | FLAG(Gpd) | FLAG(Gpq), 0, 0, 0x00),
  OSIGNATURE(FLAG(W) | FLAG(Gpd) | FLAG(Gpq), 0, 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Implicit) | FLAG(Gpd), 0, 0, 0x04),
  OSIGNATURE(FLAG(R) | FLAG(Implicit) | FLAG(Gpq), 0, 0, 0x04),
  OSIGNATURE(FLAG(R) | FLAG(Gpw) | FLAG(Gpd) | FLAG(Mem), MEM(M16) | MEM(M32), 0, 0x00),
  OSIGNATURE(FLAG(X) | FLAG(Mm), 0, 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Mm) | FLAG(Mem), MEM(M64), 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Mm) | FLAG(Mem) | FLAG(I8), MEM(M64), 0, 0x00),
  OSIGNATURE(FLAG(X) | FLAG(I16), 0, 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Xmm) | FLAG(Ymm) | FLAG(Mem), MEM(M128) | MEM(M256), 0, 0x00),
  OSIGNATURE(FLAG(W) | FLAG(Xmm) | FLAG(Ymm) | FLAG(Zmm), 0, 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Xmm) | FLAG(Mem), MEM(M8), 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Gpd) | FLAG(Gpq), 0, 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(GpbLo) | FLAG(GpbHi) | FLAG(Gpw) | FLAG(Gpd) | FLAG(Gpq) | FLAG(Mem), MEM(M8), 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Xmm) | FLAG(I8), 0, 0, 0x00),
  OSIGNATURE(FLAG(W) | FLAG(Vm), MEM(Vm64x) | MEM(Vm64y), 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Xmm) | FLAG(Ymm), 0, 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Implicit) | FLAG(Xmm), 0, 0, 0x01),
  OSIGNATURE(FLAG(X) | FLAG(Gpd) | FLAG(Gpq), 0, 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Mem), MEM(Any), 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Implicit) | FLAG(Mem), MEM(BaseOnly) | MEM(Ds), 0, 0x01),
  OSIGNATURE(FLAG(X) | FLAG(Implicit) | FLAG(Mem), MEM(BaseOnly) | MEM(Ds), 0, 0x40),
  OSIGNATURE(FLAG(X) | FLAG(Implicit) | FLAG(Mem), MEM(BaseOnly) | MEM(Es), 0, 0x80),
  OSIGNATURE(FLAG(X) | FLAG(Mem), MEM(M128), 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Implicit) | FLAG(Gpq), 0, 0, 0x02),
  OSIGNATURE(FLAG(R) | FLAG(Implicit) | FLAG(Gpq), 0, 0, 0x08),
  OSIGNATURE(FLAG(X) | FLAG(Mem), MEM(M64), 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Implicit) | FLAG(Gpd), 0, 0, 0x02),
  OSIGNATURE(FLAG(R) | FLAG(Implicit) | FLAG(Gpd), 0, 0, 0x08),
  OSIGNATURE(FLAG(W) | FLAG(Implicit) | FLAG(Gpd), 0, 0, 0x08),
  OSIGNATURE(FLAG(X) | FLAG(Implicit) | FLAG(Gpd), 0, 0, 0x02),
  OSIGNATURE(FLAG(X) | FLAG(Mem), MEM(M80), 0, 0x00),
  OSIGNATURE(FLAG(X) | FLAG(Mem), MEM(M16) | MEM(M32), 0, 0x00),
  OSIGNATURE(FLAG(X) | FLAG(Mem), MEM(M16) | MEM(M32) | MEM(M64), 0, 0x00),
  OSIGNATURE(FLAG(X) | FLAG(Fp) | FLAG(Mem), MEM(M32) | MEM(M64) | MEM(M80), 0, 0x00),
  OSIGNATURE(FLAG(X) | FLAG(Mem), MEM(M16), 0, 0x00),
  OSIGNATURE(FLAG(X) | FLAG(Mem), MEM(Any), 0, 0x00),
  OSIGNATURE(FLAG(X) | FLAG(Gpw) | FLAG(Mem), MEM(M16), 0, 0x01),
  OSIGNATURE(FLAG(W) | FLAG(GpbLo) | FLAG(Gpw) | FLAG(Gpd), 0, 0, 0x01),
  OSIGNATURE(FLAG(R) | FLAG(Gpw) | FLAG(I8), 0, 0, 0x04),
  OSIGNATURE(FLAG(W) | FLAG(Mem), MEM(BaseOnly) | MEM(Es), 0, 0x80),
  OSIGNATURE(FLAG(R) | FLAG(Gpw), 0, 0, 0x04),
  OSIGNATURE(FLAG(X) | FLAG(I8), 0, 0, 0x00),
  OSIGNATURE(FLAG(X) | FLAG(I32) | FLAG(I64) | FLAG(Rel8) | FLAG(Rel32), 0, 0, 0x00),
  OSIGNATURE(FLAG(X) | FLAG(I32) | FLAG(I64) | FLAG(Rel8), 0, 0, 0x00),
  OSIGNATURE(FLAG(W) | FLAG(Implicit) | FLAG(GpbHi), 0, 0, 0x01),
  OSIGNATURE(FLAG(W) | FLAG(Implicit) | FLAG(GpbLo) | FLAG(Gpw) | FLAG(Gpd) | FLAG(Gpq), 0, 0, 0x01),
  OSIGNATURE(FLAG(R) | FLAG(Implicit) | FLAG(Mem), MEM(BaseOnly) | MEM(Ds), 0, 0x80),
  OSIGNATURE(FLAG(X) | FLAG(Gpw) | FLAG(I8), 0, 0, 0x04),
  OSIGNATURE(FLAG(R) | FLAG(GpbLo) | FLAG(Gpw) | FLAG(Gpd), 0, 0, 0x01),
  OSIGNATURE(FLAG(R) | FLAG(Mem), MEM(BaseOnly) | MEM(Ds), 0, 0x40),
  OSIGNATURE(FLAG(W) | FLAG(Implicit) | FLAG(Gpd), 0, 0, 0x02),
  OSIGNATURE(FLAG(W) | FLAG(Implicit) | FLAG(Xmm), 0, 0, 0x01),
  OSIGNATURE(FLAG(X) | FLAG(Mm) | FLAG(Xmm), 0, 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Gpw) | FLAG(Gpd) | FLAG(Gpq) | FLAG(Mem), MEM(M16), 0, 0x00),
  OSIGNATURE(FLAG(W) | FLAG(Implicit) | FLAG(Gpd), 0, 0, 0x01),
  OSIGNATURE(FLAG(R) | FLAG(Implicit) | FLAG(GpbHi), 0, 0, 0x01),
  OSIGNATURE(FLAG(R) | FLAG(Implicit) | FLAG(GpbLo) | FLAG(Gpw) | FLAG(Gpd) | FLAG(Gpq), 0, 0, 0x01),
  OSIGNATURE(FLAG(W) | FLAG(Ymm) | FLAG(Zmm), 0, 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Ymm) | FLAG(Zmm), 0, 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Xmm) | FLAG(Ymm) | FLAG(Zmm) | FLAG(Mem), MEM(M128) | MEM(M256) | MEM(M512), 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Xmm) | FLAG(Ymm) | FLAG(Zmm), 0, 0, 0x00)
};
#undef OSIGNATURE
#undef MEM
#undef FLAG
// ----------------------------------------------------------------------------
// ${signatureData:End}

static const uint32_t _x86OpFlagFromRegType[X86Reg::kRegCount] = {
  X86Inst::kOpNone,  // #00 None.
  X86Inst::kOpNone,  // #01 Reserved (LabelTag).
  X86Inst::kOpGpbLo, // #02 GPB-LO.
  X86Inst::kOpGpbHi, // #03 GPB-HI.
  X86Inst::kOpGpw,   // #04 GPW.
  X86Inst::kOpGpd,   // #05 GPD.
  X86Inst::kOpGpq,   // #06 GPQ.
  X86Inst::kOpNone,  // #07 Reserved (VEC32).
  X86Inst::kOpNone,  // #08 Reserved (VEC64).
  X86Inst::kOpXmm,   // #09 XMM.
  X86Inst::kOpYmm,   // #10 YMM.
  X86Inst::kOpZmm,   // #11 ZMM.
  X86Inst::kOpNone,  // #12 Reserved (VEC1024).
  X86Inst::kOpNone,  // #13 Reserved (VEC2048).
  X86Inst::kOpNone,  // #14 RIP.
  X86Inst::kOpSeg,   // #15 SEG.
  X86Inst::kOpFp,    // #16 FP.
  X86Inst::kOpMm,    // #17 MM.
  X86Inst::kOpK,     // #18 K.
  X86Inst::kOpBnd,   // #20 BND.
  X86Inst::kOpCr,    // #21 CR.
  X86Inst::kOpDr     // #22 DR.
};

struct X86ValidationData {
  //! Allowed registers by reg-type (X86::kReg...).
  uint32_t allowedRegMask[X86Reg::kRegCount];
  uint32_t allowedMemBaseRegs;
  uint32_t allowedMemIndexRegs;
};

static const X86ValidationData _x86ValidationData = {
  {
    0x00000000U,       // #00 None.
    0x00000000U,       // #01 Reserved (LabelTag).
    0x0000000FU,       // #02 GPB-LO.
    0x0000000FU,       // #03 GPB-HI.
    0x000000FFU,       // #04 GPW.
    0x000000FFU,       // #05 GPD.
    0x000000FFU,       // #06 GPQ.
    0x00000000U,       // #07 Reserved (VEC32).
    0x00000000U,       // #08 Reserved (VEC64).
    0x000000FFU,       // #09 XMM.
    0x000000FFU,       // #10 YMM.
    0x000000FFU,       // #11 ZMM.
    0x00000000U,       // #12 Reserved (VEC1024).
    0x00000000U,       // #13 Reserved (VEC2048).
    0x00000001U,       // #14 RIP.
    0x0000007EU,       // #15 SEG (ES|CS|SS|DS|FS|GS).
    0x000000FFU,       // #16 FP.
    0x000000FFU,       // #17 MM.
    0x000000FFU,       // #18 K.
    0x0000000FU,       // #19 BND
    0x000000FFU,       // #20 CR.
    0x000000FFU        // #21 DR.
  },

  // AllowedMemBaseRegs:
  (1U << X86Reg::kRegGpw) | (1U << X86Reg::kRegGpd) | (1U << X86Reg::kRegRip) | (1U << Label::kLabelTag),
  // AllowedMemIndexRegs:
  (1U << X86Reg::kRegGpw) | (1U << X86Reg::kRegGpd) | (1U << X86Reg::kRegXmm) | (1U << X86Reg::kRegYmm) | (1U << X86Reg::kRegZmm)
};

static const X86ValidationData _x64ValidationData = {
  {
    0x00000000U,       // #00 None.
    0x00000000U,       // #01 Reserved (LabelTag).
    0x0000FFFFU,       // #02 GPB-LO.
    0x0000000FU,       // #03 GPB-HI.
    0x0000FFFFU,       // #04 GPW.
    0x0000FFFFU,       // #05 GPD.
    0x0000FFFFU,       // #06 GPQ.
    0x00000000U,       // #07 Reserved (VEC32).
    0x00000000U,       // #08 Reserved (VEC64).
    0xFFFFFFFFU,       // #09 XMM (16 base regs, 32 regs only with EVEX encoding).
    0xFFFFFFFFU,       // #10 YMM (16 base regs, 32 regs only with EVEX encoding).
    0xFFFFFFFFU,       // #11 ZMM (16 base regs, 32 regs only with EVEX encoding).
    0x00000000U,       // #12 Reserved (VEC1024).
    0x00000000U,       // #13 Reserved (VEC2048).
    0x00000001U,       // #14 RIP.
    0x0000007EU,       // #15 SEG (FS|GS) (ES|CS|SS|DS defined, but ignored).
    0x000000FFU,       // #16 FP.
    0x000000FFU,       // #17 MM.
    0x000000FFU,       // #18 K.
    0x0000000FU,       // #19 BND.
    0x0000FFFFU,       // #20 CR.
    0x0000FFFFU        // #21 DR.
  },

  // AllowedMemBaseRegs:
  (1U << X86Reg::kRegGpd) | (1U << X86Reg::kRegGpq) | (1U << X86Reg::kRegRip) | (1U << Label::kLabelTag),
  // AllowedMemIndexRegs:
  (1U << X86Reg::kRegGpd) | (1U << X86Reg::kRegGpq) | (1U << X86Reg::kRegXmm) | (1U << X86Reg::kRegYmm) | (1U << X86Reg::kRegZmm)
};

static ASMJIT_INLINE bool X86Inst_checkOSig(const X86Inst::OSignature& op, const X86Inst::OSignature& ref) noexcept {
  // Fail if operand types are incompatible.
  uint32_t opFlags = op.flags;
  if ((opFlags & ref.flags) == 0)
    return false;

  // Fail if memory specific flags and sizes are incompatibles.
  uint32_t opMemFlags = op.memFlags;
  if (opMemFlags != 0) {
    uint32_t refMemFlags = ref.memFlags;
    if ((refMemFlags & opMemFlags) == 0)
      return false;

    if ((refMemFlags & X86Inst::kMemOpBaseOnly) && !(opMemFlags && X86Inst::kMemOpBaseOnly))
      return false;
  }

  // Specific register index.
  if (opFlags & X86Inst::kOpAllRegs) {
    uint32_t refRegMask = ref.regMask;
    if (refRegMask && !(op.regMask & refRegMask))
      return false;
  }

  return true;
}

ASMJIT_FAVOR_SIZE Error X86Inst::validate(
  uint32_t archType,
  uint32_t instId, uint32_t options,
  const Operand_& opExtra, const Operand_* opArray, uint32_t opCount) noexcept {

  uint32_t i;
  uint32_t archMask;
  const X86ValidationData* vd;

  if (!ArchInfo::isX86Family(archType))
    return DebugUtils::errored(kErrorInvalidArch);

  if (archType == ArchInfo::kTypeX86) {
    vd = &_x86ValidationData;
    archMask = X86Inst::kArchMaskX86;
  }
  else {
    vd = &_x64ValidationData;
    archMask = X86Inst::kArchMaskX64;
  }

  if (ASMJIT_UNLIKELY(instId >= X86Inst::_kIdCount))
    return DebugUtils::errored(kErrorInvalidArgument);

  // Get the instruction data.
  const X86Inst* iData = &X86InstDB::instData[instId];

  // Translate the given operands to `X86Inst::OSignature`.
  X86Inst::OSignature oSigTranslated[6];
  uint32_t combinedOpFlags = 0;
  uint32_t combinedRegMask = 0;

  const X86Mem* memOp = nullptr;

  for (i = 0; i < opCount; i++) {
    const Operand_& op = opArray[i];
    if (op.getOp() == Operand::kOpNone) break;

    uint32_t opFlags = 0;
    uint32_t memFlags = 0;
    uint32_t regMask = 0;

    switch (op.getOp()) {
      case Operand::kOpReg: {
        uint32_t regType = op.as<Reg>().getType();
        if (ASMJIT_UNLIKELY(regType >= X86Reg::kRegCount))
          return DebugUtils::errored(kErrorInvalidRegType);

        opFlags = _x86OpFlagFromRegType[regType];
        if (ASMJIT_UNLIKELY(opFlags == 0))
          return DebugUtils::errored(kErrorInvalidRegType);

        // If `regId` is equal or greater than Operand::kPackedIdMin it means
        // that the register is virtual and its index will be assigned later
        // by the register allocator. We must pass unless asked to disallow
        // virtual registers.
        // TODO: We need an option to refuse virtual regs here.
        uint32_t regId = op.getId();
        if (regId < Operand::kPackedIdMin) {
          if (ASMJIT_UNLIKELY(regId >= 32))
            return DebugUtils::errored(kErrorInvalidPhysId);

          regMask = Utils::mask(regId);
          if (ASMJIT_UNLIKELY((vd->allowedRegMask[regType] & regMask) == 0))
            return DebugUtils::errored(kErrorInvalidPhysId);

          combinedRegMask |= regMask;
        }
        break;
      }

      // TODO: Validate base and index and combine with `combinedRegMask`.
      case Operand::kOpMem: {
        const X86Mem& m = static_cast<const X86Mem&>(op);
        uint32_t memSize = m.getSize();

        uint32_t baseType = m.getBaseType();
        uint32_t indexType = m.getIndexType();

        memOp = &m;

        if (m.getSegmentId() > 6)
          return DebugUtils::errored(kErrorInvalidSegment);

        if (baseType) {
          if (ASMJIT_UNLIKELY((vd->allowedMemBaseRegs & (1U << baseType)) == 0))
            return DebugUtils::errored(kErrorInvalidAddress);

          // Create information that will be validated only if this is an implicit
          // memory operand. Basically only usable for string instructions and other
          // instructions where memory operand is implicit and has 'seg:[reg]' form.
          uint32_t baseId = m.getBaseId();
          if (baseId < Operand::kPackedIdMin) {
            // Physical base id.
            regMask = Utils::mask(baseId);
            combinedRegMask |= regMask;
          }
          else {
            // Virtual base id - will the whole mask for implicit mem validation.
            // The register is not assigned yet, so we cannot predict the phys id.
            regMask = 0xFFFFFFFFU;
          }

          if (!indexType && !m.getOffsetLo32())
            memFlags |= X86Inst::kMemOpBaseOnly;
        }

        if (indexType) {
          if (ASMJIT_UNLIKELY((vd->allowedMemIndexRegs & (1U << indexType)) == 0))
            return DebugUtils::errored(kErrorInvalidAddress);

          if (indexType == X86Reg::kRegXmm) {
            opFlags |= X86Inst::kOpVm;
            memFlags |= X86Inst::kMemOpVm32x | X86Inst::kMemOpVm64x;
          }
          else if (indexType == X86Reg::kRegYmm) {
            opFlags |= X86Inst::kOpVm;
            memFlags |= X86Inst::kMemOpVm32y | X86Inst::kMemOpVm64y;
          }
          else if (indexType == X86Reg::kRegZmm) {
            opFlags |= X86Inst::kOpVm;
            memFlags |= X86Inst::kMemOpVm32z | X86Inst::kMemOpVm64z;
          }
          else {
            opFlags |= X86Inst::kOpMem;
          }

          uint32_t indexId = m.getIndexId();
          if (indexId < Operand::kPackedIdMin)
            combinedRegMask |= Utils::mask(indexId);

          // Only used for implicit memory operands having 'seg:[reg]' form, so clear it.
          regMask = 0;
        }
        else {
          opFlags |= X86Inst::kOpMem;
        }

        // TODO: We need 'any-size' information, otherwise we can't validate properly.
        memFlags |= X86Inst::kMemOpM8    |
                    X86Inst::kMemOpM16   |
                    X86Inst::kMemOpM32   |
                    X86Inst::kMemOpM64   |
                    X86Inst::kMemOpM80   |
                    X86Inst::kMemOpM128  |
                    X86Inst::kMemOpM256  |
                    X86Inst::kMemOpM512  |
                    X86Inst::kMemOpM1024 |
                    X86Inst::kMemOpAny   ;
        break;
      }

      case Operand::kOpImm: {
        // TODO: We need signed vs. zero extension, otherwise we can't validate properly.
        opFlags |= X86Inst::kOpI4 | X86Inst::kOpI8 | X86Inst::kOpI16 | X86Inst::kOpI32 | X86Inst::kOpI64;
        break;
      }

      case Operand::kOpLabel: {
        opFlags |= X86Inst::kOpRel8 | X86Inst::kOpRel32;
        break;
      }

      default:
        return DebugUtils::errored(kErrorInvalidState);
    }

    X86Inst::OSignature& tod = oSigTranslated[i];
    tod.flags = opFlags;
    tod.memFlags = static_cast<uint16_t>(memFlags);
    tod.regMask = static_cast<uint8_t>(regMask & 0xFFU);
    combinedOpFlags |= opFlags;
  }

  // Decrease the number of operands of those that are none. This is important
  // as Assembler and CodeCompiler may just pass more operands where some of
  // them are none (it means that no operand is given at that index). However,
  // validate that there are no gaps (like [reg, none, reg] or [none, reg]).
  if (i < opCount) {
    while (--opCount > i)
      if (ASMJIT_UNLIKELY(!opArray[opCount].isNone()))
        return DebugUtils::errored(kErrorInvalidState);
  }

  // Validate X86 and X64 specific cases.
  if (archMask == kArchMaskX86) {
    // Illegal use of 64-bit register in 32-bit mode.
    if (ASMJIT_UNLIKELY((combinedOpFlags & X86Inst::kOpGpq) != 0))
      return DebugUtils::errored(kErrorInvalidUseOfGpq);
  }
  else {
    // Illegal use of a high 8-bit register with REX prefix.
    if (ASMJIT_UNLIKELY((combinedOpFlags & X86Inst::kOpGpbHi) != 0 && (combinedRegMask & 0xFFFFFF00U) != 0))
      return DebugUtils::errored(kErrorInvalidUseOfGpbHi);
  }

  // Validate instruction operands.
  const X86Inst::CommonData* commonData = &iData->getCommonData();
  const X86Inst::ISignature* iSig = _x86InstISignatureData + commonData->_iSignatureIndex;
  const X86Inst::ISignature* iEnd = iSig                   + commonData->_iSignatureCount;

  if (iSig != iEnd) {
    const X86Inst::OSignature* oSigData = _x86InstOSignatureData;
    do {
      // Check if the architecture is compatible.
      if ((iSig->archMask & archMask) == 0) continue;

      // Compare the operands table with reference operands.
      uint32_t iCount = iSig->opCount;
      if (iCount == opCount) {
        uint32_t j;
        for (j = 0; j < opCount; j++)
          if (!X86Inst_checkOSig(oSigTranslated[j], oSigData[iSig->operands[j]]))
            break;

        if (j == opCount)
          break;
      }
      else if (iCount - iSig->implicit == opCount) {
        uint32_t j;
        uint32_t r = 0;

        for (j = 0; j < opCount && r < iCount; j++, r++) {
          const X86Inst::OSignature* oChk = oSigTranslated + j;
          const X86Inst::OSignature* oRef;
Next:
          oRef = oSigData + iSig->operands[r];
          // Skip implicit.
          if ((oRef->flags & X86Inst::kOpImplicit) != 0) {
            if (++r >= iCount)
              break;
            else
              goto Next;
          }

          if (!X86Inst_checkOSig(*oChk, *oRef))
            break;
        }

        if (j == opCount)
          break;
      }
    } while (++iSig != iEnd);

    if (iSig == iEnd)
      return DebugUtils::errored(kErrorInvalidInstruction);
  }

  // Validate AVX-512 options:
  const uint32_t kAvx512Options = X86Inst::kOptionOpExtra |
                                  X86Inst::kOptionKZ      |
                                  X86Inst::kOption1ToX    |
                                  X86Inst::kOptionER      |
                                  X86Inst::kOptionSAE     ;
  if (options & kAvx512Options) {
    if (commonData->hasFlag(X86Inst::kInstFlagEvex)) {
      const X86Inst::AvxData& avxData = iData->getAvxData();

      // Validate AVX-512 {k} and {k}{z}.
      if (options & (X86Inst::kOptionOpExtra | X86Inst::kOptionKZ)) {
        // Zero {z} without a mask register is invalid.
        if (ASMJIT_UNLIKELY(!(options & X86Inst::kOptionOpExtra)))
          return DebugUtils::errored(kErrorInvalidKZeroUse);

        // Mask can only be specified by a 'k' register.
        if (ASMJIT_UNLIKELY(!X86Reg::isK(opExtra)))
          return DebugUtils::errored(kErrorInvalidKMaskReg);

        if (ASMJIT_UNLIKELY(!avxData.hasMasking()))
          return DebugUtils::errored(kErrorInvalidKMaskUse);

        if (ASMJIT_UNLIKELY((options & X86Inst::kOptionKZ) != 0 && !avxData.hasZeroing()))
          return DebugUtils::errored(kErrorInvalidKZeroUse);
      }

      // Validate AVX-512 broadcast {1tox}.
      if (options & X86Inst::kOption1ToX) {
        if (ASMJIT_UNLIKELY(!memOp))
          return DebugUtils::errored(kErrorInvalidBroadcast);

        uint32_t size = memOp->getSize();
        if (size != 0) {
          // The the size is specified it has to match the broadcast size.
          if (ASMJIT_UNLIKELY(avxData.hasBroadcast32() && size != 4))
            return DebugUtils::errored(kErrorInvalidBroadcast);

          if (ASMJIT_UNLIKELY(avxData.hasBroadcast64() && size != 8))
            return DebugUtils::errored(kErrorInvalidBroadcast);
        }
      }

      // Validate AVX-512 {sae} and {er}.
      if (options & (X86Inst::kOptionSAE | X86Inst::kOptionER)) {
        // Rounding control is impossible if the instruction is not reg-to-reg.
        if (ASMJIT_UNLIKELY(memOp))
          return DebugUtils::errored(kErrorInvalidEROrSAE);

        // Check if {sae} or {er} is supported by the instruction.
        if (options & X86Inst::kOptionER) {
          // NOTE: if both {sae} and {er} are set, we don't care, as {sae} is implied.
          if (ASMJIT_UNLIKELY(!avxData.hasER()))
            return DebugUtils::errored(kErrorInvalidEROrSAE);

          // {er} is defined for scalar ops or vector ops using zmm (LL = 10). We
          // don't need any more bits in the instruction database to be able to
          // validate this, as each AVX512 instruction that has broadcast is vector
          // instruction (in this case we require zmm registers), otherwise it's a
          // scalar instruction, which is valid.
          if (avxData.hasBroadcast()) {
            // Supports broadcast, thus we require LL to be '10', which means there
            // have to be zmm registers used. We don't calculate LL here, but we know
            // that it would be '10' if there is at least one ZMM register used.

            // There is no 'ER' enabled instruction with less than two operands.
            ASMJIT_ASSERT(opCount >= 2);
            if (ASMJIT_UNLIKELY(!X86Reg::isZmm(opArray[0]) && !X86Reg::isZmm(opArray[1])))
              return DebugUtils::errored(kErrorInvalidEROrSAE);
          }
        }
        else {
          // {sae} doesn't have the same limitations as {er}, this is enough.
          if (ASMJIT_UNLIKELY(!avxData.hasSAE()))
            return DebugUtils::errored(kErrorInvalidEROrSAE);
        }
      }
    }
    else {
      // Not AVX512 instruction - maybe OpExtra is xCX register used
      // by REP/REPNZ prefix. Otherwise the instruction is invalid.
      if ((options & kAvx512Options) != X86Inst::kOptionOpExtra ||
          (options & (X86Inst::kOptionRep | X86Inst::kOptionRepnz)) == 0)
        return DebugUtils::errored(kErrorInvalidInstruction);
    }
  }

  return kErrorOk;
}
#endif // !ASMJIT_DISABLE_VALIDATION

// ============================================================================
// [asmjit::X86Inst - MiscData]
// ============================================================================

#define CC_TO_INST(inst) {               \
  inst##o, inst##no, inst##b , inst##ae, \
  inst##e, inst##ne, inst##be, inst##a , \
  inst##s, inst##ns, inst##pe, inst##po, \
  inst##l, inst##ge, inst##le, inst##g   \
}

const X86Inst::MiscData X86InstDB::miscData = {
  CC_TO_INST(X86Inst::kIdJ),
  CC_TO_INST(X86Inst::kIdSet),
  CC_TO_INST(X86Inst::kIdCmov),

  // ReversedCond[]:
  {
    x86::kCondO, x86::kCondNO, x86::kCondA , x86::kCondBE, // O|NO|B |AE
    x86::kCondE, x86::kCondNE, x86::kCondAE, x86::kCondB , // E|NE|BE|A
    x86::kCondS, x86::kCondNS, x86::kCondPE, x86::kCondPO, // S|NS|PE|PO
    x86::kCondG, x86::kCondLE, x86::kCondGE, x86::kCondL   // L|GE|LE|G
  }
};

#undef CC_TO_INST

// ============================================================================
// [asmjit::X86Inst - Test]
// ============================================================================

#if defined(ASMJIT_TEST)
UNIT(x86_inst_bits) {
  INFO("Checking validity of X86Inst enums");

  // Cross-validate prefixes.
  EXPECT(X86Inst::kOptionRex  == 0x80000000U, "REX prefix must be at 0x80000000");
  EXPECT(X86Inst::kOptionVex3 == 0x00000400U, "VEX3 prefix must be at 0x00000400");
  EXPECT(X86Inst::kOptionEvex == 0x00001000U, "EVEX prefix must be at 0x00001000");

  // These could be combined together to form a valid REX prefix, they must match.
  EXPECT(int(X86Inst::kOptionOpCodeB) == int(X86Inst::kOpCode_B));
  EXPECT(int(X86Inst::kOptionOpCodeX) == int(X86Inst::kOpCode_X));
  EXPECT(int(X86Inst::kOptionOpCodeR) == int(X86Inst::kOpCode_R));
  EXPECT(int(X86Inst::kOptionOpCodeW) == int(X86Inst::kOpCode_W));

  uint32_t rex_rb = (X86Inst::kOpCode_R >> X86Inst::kOpCode_REX_Shift) |
                    (X86Inst::kOpCode_B >> X86Inst::kOpCode_REX_Shift) | 0x40;
  uint32_t rex_rw = (X86Inst::kOpCode_R >> X86Inst::kOpCode_REX_Shift) |
                    (X86Inst::kOpCode_W >> X86Inst::kOpCode_REX_Shift) | 0x40;
  EXPECT(rex_rb == 0x45, "kOpCode_R|B must form a valid REX prefix 0x45 if combined with 0x40");
  EXPECT(rex_rw == 0x4C, "kOpCode_R|W must form a valid REX prefix 0x4C if combined with 0x40");
}
#endif // ASMJIT_TEST

#if defined(ASMJIT_TEST) && !defined(ASMJIT_DISABLE_TEXT)
UNIT(x86_inst_names) {
  // All known instructions should be matched.
  INFO("Matching all X86/X64 instructions");
  for (uint32_t a = 0; a < X86Inst::_kIdCount; a++) {
    uint32_t b = X86Inst::getIdByName(X86Inst::getInst(a).getName());
    EXPECT(a == b,
      "Should match existing instruction \"%s\" {id:%u} != \"%s\" {id:%u}",
        X86Inst::getInst(a).getName(), a,
        X86Inst::getInst(b).getName(), b);
  }

  // Everything else should return `kInvalidInstId`.
  INFO("Trying to look-up instructions that don't exist");
  EXPECT(X86Inst::getIdByName(nullptr) == Globals::kInvalidInstId,
    "Should return kInvalidInstId for null input");

  EXPECT(X86Inst::getIdByName("") == Globals::kInvalidInstId,
    "Should return kInvalidInstId for empty string");

  EXPECT(X86Inst::getIdByName("_") == Globals::kInvalidInstId,
    "Should return kInvalidInstId for unknown instruction");

  EXPECT(X86Inst::getIdByName("123xyz") == Globals::kInvalidInstId,
    "Should return kInvalidInstId for unknown instruction");
}
#endif // ASMJIT_TEST && !ASMJIT_DISABLE_TEXT

#if defined(ASMJIT_TEST) && !defined(ASMJIT_DISABLE_VALIDATION)
static Error x86_validate(uint32_t instId, const Operand& o0 = Operand(), const Operand& o1 = Operand(), const Operand& o2 = Operand()) {
  Operand opArray[] = { o0, o1, o2 };
  return X86Inst::validate(ArchInfo::kTypeX86, instId, 0, Operand(), opArray, 3);
}

static Error x64_validate(uint32_t instId, const Operand& o0 = Operand(), const Operand& o1 = Operand(), const Operand& o2 = Operand()) {
  Operand opArray[] = { o0, o1, o2 };
  return X86Inst::validate(ArchInfo::kTypeX64, instId, 0, Operand(), opArray, 3);
}

UNIT(x86_inst_validation) {
  INFO("Validating instructions that use GP registers");
  EXPECT(x86_validate(X86Inst::kIdCmp   , x86::eax , x86::edx ) == kErrorOk);
  EXPECT(x64_validate(X86Inst::kIdCmp   , x86::rax , x86::rdx ) == kErrorOk);

  EXPECT(x86_validate(X86Inst::kIdCmp   , x86::eax            ) != kErrorOk);
  EXPECT(x86_validate(X86Inst::kIdCmp   , x86::rax , x86::rdx ) != kErrorOk);
  EXPECT(x64_validate(X86Inst::kIdCmp   , x86::rax , x86::al  ) != kErrorOk);

  INFO("Validating instructions that use FP registers");
  EXPECT(x86_validate(X86Inst::kIdFadd  , x86::fp0 , x86::fp7 ) == kErrorOk);
  EXPECT(x86_validate(X86Inst::kIdFadd  , x86::fp7 , x86::fp0 ) == kErrorOk);
  EXPECT(x86_validate(X86Inst::kIdFadd  , x86::fp0 , x86::eax ) != kErrorOk);
  EXPECT(x86_validate(X86Inst::kIdFadd  , x86::fp4 , x86::fp3 ) != kErrorOk);

  INFO("Validating instructions that use MM registers");
  EXPECT(x86_validate(X86Inst::kIdPand  , x86::mm0 , x86::mm1 ) == kErrorOk);
  EXPECT(x86_validate(X86Inst::kIdPand  , x86::mm0 , x86::eax ) != kErrorOk);

  INFO("Validating instructions that use XMM registers");
  EXPECT(x86_validate(X86Inst::kIdPand  , x86::xmm0, x86::xmm1) == kErrorOk);
  EXPECT(x64_validate(X86Inst::kIdPand  , x86::xmm8, x86::xmm9) == kErrorOk);
  EXPECT(x86_validate(X86Inst::kIdPand  , x86::xmm0, x86::eax ) != kErrorOk);
  EXPECT(x64_validate(X86Inst::kIdPand  , x86::xmm0, x86::rax ) != kErrorOk);

  INFO("Validating instructions that use YMM registers");
  EXPECT(x86_validate(X86Inst::kIdVpand , x86::ymm0, x86::ymm1, x86::ymm2) == kErrorOk);
  EXPECT(x86_validate(X86Inst::kIdVpand , x86::ymm0, x86::ymm1, x86::eax ) != kErrorOk);

  INFO("Validating instructions that use ZMM registers");
  EXPECT(x86_validate(X86Inst::kIdVpaddw, x86::zmm0, x86::zmm1, x86::zmm2) == kErrorOk);
  EXPECT(x86_validate(X86Inst::kIdVpaddw, x86::zmm0, x86::zmm1, x86::eax ) != kErrorOk);

  INFO("Validating instructions that use CR registers");
  EXPECT(x86_validate(X86Inst::kIdMov   , x86::eax , x86::cr0 ) == kErrorOk);
  EXPECT(x86_validate(X86Inst::kIdMov   , x86::eax , x86::cr8 ) != kErrorOk);
  EXPECT(x64_validate(X86Inst::kIdMov   , x86::rax , x86::cr8 ) == kErrorOk);
  EXPECT(x64_validate(X86Inst::kIdMov   , x86::eax , x86::cr0 ) != kErrorOk);

  INFO("Validating instructions that use DR registers");
  EXPECT(x86_validate(X86Inst::kIdMov   , x86::eax , x86::dr0 ) == kErrorOk);
  EXPECT(x64_validate(X86Inst::kIdMov   , x86::rax , x86::dr7 ) == kErrorOk);
  EXPECT(x86_validate(X86Inst::kIdMov   , x86::ax  , x86::dr0 ) != kErrorOk);
  EXPECT(x64_validate(X86Inst::kIdMov   , x86::eax , x86::dr7 ) != kErrorOk);

  INFO("Validating instructions that use segment registers");
  EXPECT(x86_validate(X86Inst::kIdMov   , x86::ax  , x86::fs  ) == kErrorOk);
  EXPECT(x64_validate(X86Inst::kIdMov   , x86::ax  , x86::fs  ) == kErrorOk);
  EXPECT(x64_validate(X86Inst::kIdPush  , x86::cs             ) != kErrorOk);

  INFO("Validating instructions that use memory operands");
  EXPECT(x86_validate(X86Inst::kIdMov   , x86::eax , x86::ptr(x86::ebx)) == kErrorOk);
  EXPECT(x64_validate(X86Inst::kIdMov   , x86::rax , x86::ptr(x86::rbx)) == kErrorOk);

  INFO("Validating instructions that use immediate values");
  EXPECT(x86_validate(X86Inst::kIdMov   , x86::eax , imm(1)) == kErrorOk);
  EXPECT(x64_validate(X86Inst::kIdMov   , x86::rax , imm(1)) == kErrorOk);
}
#endif // ASMJIT_TEST && !ASMJIT_DISABLE_VALIDATION

} // asmjit namespace

// [Api-End]
#include "../asmjit_apiend.h"

// [Guard]
#endif // ASMJIT_BUILD_X86
