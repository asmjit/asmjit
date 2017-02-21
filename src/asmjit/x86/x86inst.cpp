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
#include "../base/misc_p.h"
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
  ODATA_TT_DUP   = X86Inst::kOpCode_CDTT_DUP,
  ODATA_TT_T1X   = X86Inst::kOpCode_CDTT_T1_4X
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
  // <-----------------+--------------------+------------------+--------+------------------+--------+---------------------------------------+-------------+-------+-----------------+-----+----+
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
  INST(Add             , X86Arith           , O(000000,00,0,_,x,_,_,_  ), 0                         , F(RW)|F(Lock)                         , EF(WWWWWW__), 0 , 0 , kFamilyNone, 0  , 732 , 5  ),
  INST(Addpd           , ExtRm              , O(660F00,58,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 0  , 4576, 6  ),
  INST(Addps           , ExtRm              , O(000F00,58,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 1  , 4588, 6  ),
  INST(Addsd           , ExtRm              , O(F20F00,58,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 0  , 4810, 7  ),
  INST(Addss           , ExtRm              , O(F30F00,58,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 1  , 4820, 8  ),
  INST(Addsubpd        , ExtRm              , O(660F00,D0,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 2  , 4315, 6  ),
  INST(Addsubps        , ExtRm              , O(F20F00,D0,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 2  , 4327, 6  ),
  INST(Adox            , X86Rm              , O(F30F38,F6,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(X_______), 0 , 0 , kFamilyNone, 0  , 26  , 9  ),
  INST(Aesdec          , ExtRm              , O(660F38,DE,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 3  , 2811, 6  ),
  INST(Aesdeclast      , ExtRm              , O(660F38,DF,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 3  , 2819, 6  ),
  INST(Aesenc          , ExtRm              , O(660F38,DC,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 3  , 2831, 6  ),
  INST(Aesenclast      , ExtRm              , O(660F38,DD,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 3  , 2839, 6  ),
  INST(Aesimc          , ExtRm              , O(660F38,DB,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 0 , kFamilySse , 4  , 2851, 10 ),
  INST(Aeskeygenassist , ExtRmi             , O(660F3A,DF,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 0 , kFamilySse , 4  , 2859, 11 ),
  INST(And             , X86Arith           , O(000000,20,4,_,x,_,_,_  ), 0                         , F(RW)|F(Lock)                         , EF(WWWUWW__), 0 , 0 , kFamilyNone, 0  , 2278, 12 ),
  INST(Andn            , VexRvm_Wx          , V(000F38,F2,_,0,x,_,_,_  ), 0                         , F(RW)                                 , EF(WWWUUW__), 0 , 0 , kFamilyNone, 0  , 6107, 13 ),
  INST(Andnpd          , ExtRm              , O(660F00,55,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 5  , 2892, 6  ),
  INST(Andnps          , ExtRm              , O(000F00,55,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 6  , 2900, 6  ),
  INST(Andpd           , ExtRm              , O(660F00,54,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 5  , 3829, 14 ),
  INST(Andps           , ExtRm              , O(000F00,54,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 6  , 3839, 14 ),
  INST(Arpl            , X86Mr_NoSize       , O(000000,63,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(__W_____), 0 , 0 , kFamilyNone, 0  , 31  , 15 ),
  INST(Bextr           , VexRmv_Wx          , V(000F38,F7,_,0,x,_,_,_  ), 0                         , F(RW)                                 , EF(WUWUUW__), 0 , 0 , kFamilyNone, 0  , 36  , 16 ),
  INST(Blcfill         , VexVm_Wx           , V(XOP_M9,01,1,0,x,_,_,_  ), 0                         , F(WO)                                 , EF(WWWUUW__), 0 , 0 , kFamilyNone, 0  , 42  , 17 ),
  INST(Blci            , VexVm_Wx           , V(XOP_M9,02,6,0,x,_,_,_  ), 0                         , F(WO)                                 , EF(WWWUUW__), 0 , 0 , kFamilyNone, 0  , 50  , 17 ),
  INST(Blcic           , VexVm_Wx           , V(XOP_M9,01,5,0,x,_,_,_  ), 0                         , F(WO)                                 , EF(WWWUUW__), 0 , 0 , kFamilyNone, 0  , 55  , 17 ),
  INST(Blcmsk          , VexVm_Wx           , V(XOP_M9,02,1,0,x,_,_,_  ), 0                         , F(WO)                                 , EF(WWWUUW__), 0 , 0 , kFamilyNone, 0  , 61  , 17 ),
  INST(Blcs            , VexVm_Wx           , V(XOP_M9,01,3,0,x,_,_,_  ), 0                         , F(WO)                                 , EF(WWWUUW__), 0 , 0 , kFamilyNone, 0  , 68  , 17 ),
  INST(Blendpd         , ExtRmi             , O(660F3A,0D,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 7  , 2978, 18 ),
  INST(Blendps         , ExtRmi             , O(660F3A,0C,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 7  , 2987, 18 ),
  INST(Blendvpd        , ExtRm_XMM0         , O(660F38,15,_,_,_,_,_,_  ), 0                         , F(RW)|F(Special)                      , EF(________), 0 , 0 , kFamilySse , 8  , 2996, 19 ),
  INST(Blendvps        , ExtRm_XMM0         , O(660F38,14,_,_,_,_,_,_  ), 0                         , F(RW)|F(Special)                      , EF(________), 0 , 0 , kFamilySse , 8  , 3006, 19 ),
  INST(Blsfill         , VexVm_Wx           , V(XOP_M9,01,2,0,x,_,_,_  ), 0                         , F(WO)                                 , EF(WWWUUW__), 0 , 0 , kFamilyNone, 0  , 73  , 17 ),
  INST(Blsi            , VexVm_Wx           , V(000F38,F3,3,0,x,_,_,_  ), 0                         , F(RW)                                 , EF(WWWUUW__), 0 , 0 , kFamilyNone, 0  , 81  , 20 ),
  INST(Blsic           , VexVm_Wx           , V(XOP_M9,01,6,0,x,_,_,_  ), 0                         , F(WO)                                 , EF(WWWUUW__), 0 , 0 , kFamilyNone, 0  , 86  , 17 ),
  INST(Blsmsk          , VexVm_Wx           , V(000F38,F3,2,0,x,_,_,_  ), 0                         , F(RW)                                 , EF(WWWUUW__), 0 , 0 , kFamilyNone, 0  , 92  , 20 ),
  INST(Blsr            , VexVm_Wx           , V(000F38,F3,1,0,x,_,_,_  ), 0                         , F(RW)                                 , EF(WWWUUW__), 0 , 0 , kFamilyNone, 0  , 99  , 20 ),
  INST(Bndcl           , X86Rm              , O(F30F00,1A,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 104 , 21 ),
  INST(Bndcn           , X86Rm              , O(F20F00,1B,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 110 , 21 ),
  INST(Bndcu           , X86Rm              , O(F20F00,1A,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 116 , 21 ),
  INST(Bndldx          , X86Rm              , O(000F00,1A,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 122 , 22 ),
  INST(Bndmk           , X86Rm              , O(F30F00,1B,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 129 , 23 ),
  INST(Bndmov          , X86Bndmov          , O(660F00,1A,_,_,_,_,_,_  ), O(660F00,1B,_,_,_,_,_,_  ), F(WO)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 135 , 24 ),
  INST(Bndstx          , X86Mr              , O(000F00,1B,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 142 , 25 ),
  INST(Bound           , X86Rm              , O(000000,62,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 149 , 26 ),
  INST(Bsf             , X86Rm              , O(000F00,BC,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(UUWUUU__), 0 , 0 , kFamilyNone, 0  , 155 , 27 ),
  INST(Bsr             , X86Rm              , O(000F00,BD,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(UUWUUU__), 0 , 0 , kFamilyNone, 0  , 159 , 27 ),
  INST(Bswap           , X86Bswap           , O(000F00,C8,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 163 , 28 ),
  INST(Bt              , X86Bt              , O(000F00,A3,_,_,x,_,_,_  ), O(000F00,BA,4,_,x,_,_,_  ), F(RO)                                 , EF(UU_UUW__), 0 , 0 , kFamilyNone, 0  , 169 , 29 ),
  INST(Btc             , X86Bt              , O(000F00,BB,_,_,x,_,_,_  ), O(000F00,BA,7,_,x,_,_,_  ), F(RW)|F(Lock)                         , EF(UU_UUW__), 0 , 0 , kFamilyNone, 0  , 172 , 30 ),
  INST(Btr             , X86Bt              , O(000F00,B3,_,_,x,_,_,_  ), O(000F00,BA,6,_,x,_,_,_  ), F(RW)|F(Lock)                         , EF(UU_UUW__), 0 , 0 , kFamilyNone, 0  , 176 , 31 ),
  INST(Bts             , X86Bt              , O(000F00,AB,_,_,x,_,_,_  ), O(000F00,BA,5,_,x,_,_,_  ), F(RW)|F(Lock)                         , EF(UU_UUW__), 0 , 0 , kFamilyNone, 0  , 180 , 32 ),
  INST(Bzhi            , VexRmv_Wx          , V(000F38,F5,_,0,x,_,_,_  ), 0                         , F(RW)                                 , EF(WWWUUW__), 0 , 0 , kFamilyNone, 0  , 184 , 16 ),
  INST(Call            , X86Call            , O(000000,FF,2,_,_,_,_,_  ), 0                         , F(RW)|F(Volatile)                     , EF(________), 0 , 0 , kFamilyNone, 0  , 2670, 33 ),
  INST(Cbw             , X86Op_xAX          , O(660000,98,_,_,_,_,_,_  ), 0                         , F(RW)|F(Special)                      , EF(________), 0 , 0 , kFamilyNone, 0  , 189 , 34 ),
  INST(Cdq             , X86Op_xDX_xAX      , O(000000,99,_,_,_,_,_,_  ), 0                         , F(RW)|F(Special)                      , EF(________), 0 , 0 , kFamilyNone, 0  , 193 , 35 ),
  INST(Cdqe            , X86Op_xAX          , O(000000,98,_,_,1,_,_,_  ), 0                         , F(RW)|F(Special)                      , EF(________), 0 , 0 , kFamilyNone, 0  , 197 , 36 ),
  INST(Clac            , X86Op              , O(000F01,CA,_,_,_,_,_,_  ), 0                         , F(Volatile)                           , EF(___W____), 0 , 0 , kFamilyNone, 0  , 202 , 37 ),
  INST(Clc             , X86Op              , O(000000,F8,_,_,_,_,_,_  ), 0                         , F(Volatile)                           , EF(_____W__), 0 , 0 , kFamilyNone, 0  , 207 , 38 ),
  INST(Cld             , X86Op              , O(000000,FC,_,_,_,_,_,_  ), 0                         , F(Volatile)                           , EF(______W_), 0 , 0 , kFamilyNone, 0  , 211 , 39 ),
  INST(Clflush         , X86M_Only          , O(000F00,AE,7,_,_,_,_,_  ), 0                         , F(RO)|F(Volatile)                     , EF(________), 0 , 0 , kFamilyNone, 0  , 215 , 40 ),
  INST(Clflushopt      , X86M_Only          , O(660F00,AE,7,_,_,_,_,_  ), 0                         , F(RO)|F(Volatile)                     , EF(________), 0 , 0 , kFamilyNone, 0  , 223 , 40 ),
  INST(Cli             , X86Op              , O(000000,FA,_,_,_,_,_,_  ), 0                         , 0                                     , EF(________), 0 , 0 , kFamilyNone, 0  , 234 , 41 ),
  INST(Clts            , X86Op              , O(000F00,06,_,_,_,_,_,_  ), 0                         , 0                                     , EF(________), 0 , 0 , kFamilyNone, 0  , 238 , 41 ),
  INST(Clwb            , X86M_Only          , O(660F00,AE,6,_,_,_,_,_  ), 0                         , F(RO)|F(Volatile)                     , EF(________), 0 , 0 , kFamilyNone, 0  , 243 , 40 ),
  INST(Clzero          , X86Op_ZAX          , O(000F01,FC,_,_,_,_,_,_  ), 0                         , F(WO)|F(Volatile)|F(Special)          , EF(________), 0 , 0 , kFamilyNone, 0  , 248 , 42 ),
  INST(Cmc             , X86Op              , O(000000,F5,_,_,_,_,_,_  ), 0                         , 0                                     , EF(_____X__), 0 , 0 , kFamilyNone, 0  , 255 , 43 ),
  INST(Cmova           , X86Rm              , O(000F00,47,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(__R__R__), 0 , 0 , kFamilyNone, 0  , 259 , 44 ),
  INST(Cmovae          , X86Rm              , O(000F00,43,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(_____R__), 0 , 0 , kFamilyNone, 0  , 265 , 45 ),
  INST(Cmovb           , X86Rm              , O(000F00,42,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(_____R__), 0 , 0 , kFamilyNone, 0  , 589 , 45 ),
  INST(Cmovbe          , X86Rm              , O(000F00,46,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(__R__R__), 0 , 0 , kFamilyNone, 0  , 596 , 44 ),
  INST(Cmovc           , X86Rm              , O(000F00,42,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(_____R__), 0 , 0 , kFamilyNone, 0  , 272 , 45 ),
  INST(Cmove           , X86Rm              , O(000F00,44,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(__R_____), 0 , 0 , kFamilyNone, 0  , 604 , 46 ),
  INST(Cmovg           , X86Rm              , O(000F00,4F,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(RRR_____), 0 , 0 , kFamilyNone, 0  , 278 , 47 ),
  INST(Cmovge          , X86Rm              , O(000F00,4D,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(RR______), 0 , 0 , kFamilyNone, 0  , 284 , 48 ),
  INST(Cmovl           , X86Rm              , O(000F00,4C,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(RR______), 0 , 0 , kFamilyNone, 0  , 291 , 48 ),
  INST(Cmovle          , X86Rm              , O(000F00,4E,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(RRR_____), 0 , 0 , kFamilyNone, 0  , 297 , 47 ),
  INST(Cmovna          , X86Rm              , O(000F00,46,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(__R__R__), 0 , 0 , kFamilyNone, 0  , 304 , 44 ),
  INST(Cmovnae         , X86Rm              , O(000F00,42,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(_____R__), 0 , 0 , kFamilyNone, 0  , 311 , 45 ),
  INST(Cmovnb          , X86Rm              , O(000F00,43,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(_____R__), 0 , 0 , kFamilyNone, 0  , 611 , 45 ),
  INST(Cmovnbe         , X86Rm              , O(000F00,47,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(__R__R__), 0 , 0 , kFamilyNone, 0  , 619 , 44 ),
  INST(Cmovnc          , X86Rm              , O(000F00,43,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(_____R__), 0 , 0 , kFamilyNone, 0  , 319 , 45 ),
  INST(Cmovne          , X86Rm              , O(000F00,45,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(__R_____), 0 , 0 , kFamilyNone, 0  , 628 , 46 ),
  INST(Cmovng          , X86Rm              , O(000F00,4E,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(RRR_____), 0 , 0 , kFamilyNone, 0  , 326 , 47 ),
  INST(Cmovnge         , X86Rm              , O(000F00,4C,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(RR______), 0 , 0 , kFamilyNone, 0  , 333 , 48 ),
  INST(Cmovnl          , X86Rm              , O(000F00,4D,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(RR______), 0 , 0 , kFamilyNone, 0  , 341 , 48 ),
  INST(Cmovnle         , X86Rm              , O(000F00,4F,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(RRR_____), 0 , 0 , kFamilyNone, 0  , 348 , 47 ),
  INST(Cmovno          , X86Rm              , O(000F00,41,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(R_______), 0 , 0 , kFamilyNone, 0  , 356 , 49 ),
  INST(Cmovnp          , X86Rm              , O(000F00,4B,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(____R___), 0 , 0 , kFamilyNone, 0  , 363 , 50 ),
  INST(Cmovns          , X86Rm              , O(000F00,49,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(_R______), 0 , 0 , kFamilyNone, 0  , 370 , 51 ),
  INST(Cmovnz          , X86Rm              , O(000F00,45,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(__R_____), 0 , 0 , kFamilyNone, 0  , 377 , 46 ),
  INST(Cmovo           , X86Rm              , O(000F00,40,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(R_______), 0 , 0 , kFamilyNone, 0  , 384 , 49 ),
  INST(Cmovp           , X86Rm              , O(000F00,4A,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(____R___), 0 , 0 , kFamilyNone, 0  , 390 , 50 ),
  INST(Cmovpe          , X86Rm              , O(000F00,4A,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(____R___), 0 , 0 , kFamilyNone, 0  , 396 , 50 ),
  INST(Cmovpo          , X86Rm              , O(000F00,4B,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(____R___), 0 , 0 , kFamilyNone, 0  , 403 , 50 ),
  INST(Cmovs           , X86Rm              , O(000F00,48,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(_R______), 0 , 0 , kFamilyNone, 0  , 410 , 51 ),
  INST(Cmovz           , X86Rm              , O(000F00,44,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(__R_____), 0 , 0 , kFamilyNone, 0  , 416 , 46 ),
  INST(Cmp             , X86Arith           , O(000000,38,7,_,x,_,_,_  ), 0                         , F(RO)                                 , EF(WWWWWW__), 0 , 0 , kFamilyNone, 0  , 422 , 52 ),
  INST(Cmppd           , ExtRmi             , O(660F00,C2,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 9  , 3232, 18 ),
  INST(Cmpps           , ExtRmi             , O(000F00,C2,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 10 , 3239, 18 ),
  INST(Cmps            , X86StrMm           , O(000000,A6,_,_,_,_,_,_  ), 0                         , F(RW)|F(Special)|F(Rep)|F(Repnz)      , EF(WWWWWWR_), 0 , 0 , kFamilyNone, 0  , 426 , 53 ),
  INST(Cmpsd           , ExtRmi             , O(F20F00,C2,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 11 , 3246, 54 ),
  INST(Cmpss           , ExtRmi             , O(F30F00,C2,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 12 , 3253, 55 ),
  INST(Cmpxchg         , X86Cmpxchg         , O(000F00,B0,_,_,x,_,_,_  ), 0                         , F(RW)|F(Lock)|F(Special)              , EF(WWWWWW__), 0 , 0 , kFamilyNone, 0  , 431 , 56 ),
  INST(Cmpxchg16b      , X86M_Only          , O(000F00,C7,1,_,1,_,_,_  ), 0                         , F(RW)|F(Lock)|F(Special)              , EF(__W_____), 0 , 0 , kFamilyNone, 0  , 439 , 57 ),
  INST(Cmpxchg8b       , X86M_Only          , O(000F00,C7,1,_,_,_,_,_  ), 0                         , F(RW)|F(Lock)|F(Special)              , EF(__W_____), 0 , 0 , kFamilyNone, 0  , 450 , 58 ),
  INST(Comisd          , ExtRm              , O(660F00,2F,_,_,_,_,_,_  ), 0                         , F(RO)                                 , EF(WWWWWW__), 0 , 0 , kFamilySse , 13 , 9328, 59 ),
  INST(Comiss          , ExtRm              , O(000F00,2F,_,_,_,_,_,_  ), 0                         , F(RO)                                 , EF(WWWWWW__), 0 , 0 , kFamilySse , 14 , 9337, 60 ),
  INST(Cpuid           , X86Op              , O(000F00,A2,_,_,_,_,_,_  ), 0                         , F(RW)|F(Volatile)|F(Special)          , EF(________), 0 , 0 , kFamilyNone, 0  , 460 , 61 ),
  INST(Cqo             , X86Op_xDX_xAX      , O(000000,99,_,_,1,_,_,_  ), 0                         , F(RW)|F(Special)                      , EF(________), 0 , 0 , kFamilyNone, 0  , 466 , 62 ),
  INST(Crc32           , X86Crc             , O(F20F38,F0,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 470 , 63 ),
  INST(Cvtdq2pd        , ExtRm              , O(F30F00,E6,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 15 , 3300, 64 ),
  INST(Cvtdq2ps        , ExtRm              , O(000F00,5B,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 15 , 3310, 65 ),
  INST(Cvtpd2dq        , ExtRm              , O(F20F00,E6,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 15 , 3320, 65 ),
  INST(Cvtpd2pi        , ExtRm              , O(660F00,2D,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 16 , 476 , 66 ),
  INST(Cvtpd2ps        , ExtRm              , O(660F00,5A,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 17 , 3330, 65 ),
  INST(Cvtpi2pd        , ExtRm              , O(660F00,2A,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 16 , 485 , 67 ),
  INST(Cvtpi2ps        , ExtRm              , O(000F00,2A,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 18 , 494 , 68 ),
  INST(Cvtps2dq        , ExtRm              , O(660F00,5B,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 13 , 3382, 65 ),
  INST(Cvtps2pd        , ExtRm              , O(000F00,5A,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 13 , 3392, 64 ),
  INST(Cvtps2pi        , ExtRm              , O(000F00,2D,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 18 , 503 , 69 ),
  INST(Cvtsd2si        , ExtRm_Wx           , O(F20F00,2D,_,_,x,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 19 , 3464, 70 ),
  INST(Cvtsd2ss        , ExtRm              , O(F20F00,5A,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 4 , kFamilySse , 20 , 3474, 71 ),
  INST(Cvtsi2sd        , ExtRm_Wx           , O(F20F00,2A,_,_,x,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 21 , 3495, 72 ),
  INST(Cvtsi2ss        , ExtRm_Wx           , O(F30F00,2A,_,_,x,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 4 , kFamilySse , 22 , 3505, 73 ),
  INST(Cvtss2sd        , ExtRm              , O(F30F00,5A,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 21 , 3515, 74 ),
  INST(Cvtss2si        , ExtRm_Wx           , O(F30F00,2D,_,_,x,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 23 , 3525, 75 ),
  INST(Cvttpd2dq       , ExtRm              , O(660F00,E6,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 24 , 3546, 65 ),
  INST(Cvttpd2pi       , ExtRm              , O(660F00,2C,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 16 , 512 , 66 ),
  INST(Cvttps2dq       , ExtRm              , O(F30F00,5B,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 25 , 3592, 65 ),
  INST(Cvttps2pi       , ExtRm              , O(000F00,2C,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 18 , 522 , 69 ),
  INST(Cvttsd2si       , ExtRm_Wx           , O(F20F00,2C,_,_,x,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 26 , 3638, 70 ),
  INST(Cvttss2si       , ExtRm_Wx           , O(F30F00,2C,_,_,x,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 27 , 3661, 75 ),
  INST(Cwd             , X86Op_xDX_xAX      , O(660000,99,_,_,_,_,_,_  ), 0                         , F(RW)|F(Special)                      , EF(________), 0 , 0 , kFamilyNone, 0  , 532 , 76 ),
  INST(Cwde            , X86Op_xAX          , O(000000,98,_,_,_,_,_,_  ), 0                         , F(RW)|F(Special)                      , EF(________), 0 , 0 , kFamilyNone, 0  , 536 , 77 ),
  INST(Daa             , X86Op              , O(000000,27,_,_,_,_,_,_  ), 0                         , F(RW)|F(Special)                      , EF(UWWXWX__), 0 , 0 , kFamilyNone, 0  , 541 , 78 ),
  INST(Das             , X86Op              , O(000000,2F,_,_,_,_,_,_  ), 0                         , F(RW)|F(Special)                      , EF(UWWXWX__), 0 , 0 , kFamilyNone, 0  , 545 , 78 ),
  INST(Dec             , X86IncDec          , O(000000,FE,1,_,x,_,_,_  ), O(000000,48,_,_,x,_,_,_  ), F(RW)|F(Lock)                         , EF(WWWWW___), 0 , 0 , kFamilyNone, 0  , 2814, 79 ),
  INST(Div             , X86M_GPB_MulDiv    , O(000000,F6,6,_,x,_,_,_  ), 0                         , F(RW)|F(Special)                      , EF(UUUUUU__), 0 , 0 , kFamilyNone, 0  , 751 , 80 ),
  INST(Divpd           , ExtRm              , O(660F00,5E,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 28 , 3760, 6  ),
  INST(Divps           , ExtRm              , O(000F00,5E,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 29 , 3767, 6  ),
  INST(Divsd           , ExtRm              , O(F20F00,5E,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 28 , 3774, 7  ),
  INST(Divss           , ExtRm              , O(F30F00,5E,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 29 , 3781, 8  ),
  INST(Dppd            , ExtRmi             , O(660F3A,41,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 30 , 3788, 18 ),
  INST(Dpps            , ExtRmi             , O(660F3A,40,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 30 , 3794, 18 ),
  INST(Emms            , X86Op              , O(000F00,77,_,_,_,_,_,_  ), 0                         , F(Volatile)                           , EF(________), 0 , 0 , kFamilyNone, 0  , 719 , 81 ),
  INST(Enter           , X86Enter           , O(000000,C8,_,_,_,_,_,_  ), 0                         , F(Volatile)|F(Special)                , EF(________), 0 , 0 , kFamilyNone, 0  , 2678, 82 ),
  INST(Extractps       , ExtExtract         , O(660F3A,17,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 31 , 3984, 83 ),
  INST(Extrq           , ExtExtrq           , O(660F00,79,_,_,_,_,_,_  ), O(660F00,78,0,_,_,_,_,_  ), F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 32 , 6821, 84 ),
  INST(F2xm1           , FpuOp              , O_FPU(00,D9F0,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 549 , 85 ),
  INST(Fabs            , FpuOp              , O_FPU(00,D9E1,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 555 , 85 ),
  INST(Fadd            , FpuArith           , O_FPU(00,C0C0,0)          , 0                         , F(Fp)|F(FPU_M4)|F(FPU_M8)             , EF(________), 0 , 0 , kFamilyNone, 0  , 1918, 86 ),
  INST(Faddp           , FpuRDef            , O_FPU(00,DEC0,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 560 , 87 ),
  INST(Fbld            , X86M_Only          , O_FPU(00,00DF,4)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 566 , 88 ),
  INST(Fbstp           , X86M_Only          , O_FPU(00,00DF,6)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 571 , 88 ),
  INST(Fchs            , FpuOp              , O_FPU(00,D9E0,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 577 , 85 ),
  INST(Fclex           , FpuOp              , O_FPU(9B,DBE2,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 582 , 85 ),
  INST(Fcmovb          , FpuR               , O_FPU(00,DAC0,_)          , 0                         , F(Fp)                                 , EF(_____R__), 0 , 0 , kFamilyNone, 0  , 588 , 89 ),
  INST(Fcmovbe         , FpuR               , O_FPU(00,DAD0,_)          , 0                         , F(Fp)                                 , EF(__R__R__), 0 , 0 , kFamilyNone, 0  , 595 , 90 ),
  INST(Fcmove          , FpuR               , O_FPU(00,DAC8,_)          , 0                         , F(Fp)                                 , EF(__R_____), 0 , 0 , kFamilyNone, 0  , 603 , 91 ),
  INST(Fcmovnb         , FpuR               , O_FPU(00,DBC0,_)          , 0                         , F(Fp)                                 , EF(_____R__), 0 , 0 , kFamilyNone, 0  , 610 , 89 ),
  INST(Fcmovnbe        , FpuR               , O_FPU(00,DBD0,_)          , 0                         , F(Fp)                                 , EF(__R__R__), 0 , 0 , kFamilyNone, 0  , 618 , 90 ),
  INST(Fcmovne         , FpuR               , O_FPU(00,DBC8,_)          , 0                         , F(Fp)                                 , EF(__R_____), 0 , 0 , kFamilyNone, 0  , 627 , 91 ),
  INST(Fcmovnu         , FpuR               , O_FPU(00,DBD8,_)          , 0                         , F(Fp)                                 , EF(____R___), 0 , 0 , kFamilyNone, 0  , 635 , 92 ),
  INST(Fcmovu          , FpuR               , O_FPU(00,DAD8,_)          , 0                         , F(Fp)                                 , EF(____R___), 0 , 0 , kFamilyNone, 0  , 643 , 92 ),
  INST(Fcom            , FpuCom             , O_FPU(00,D0D0,2)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 650 , 93 ),
  INST(Fcomi           , FpuR               , O_FPU(00,DBF0,_)          , 0                         , F(Fp)                                 , EF(WWWWWW__), 0 , 0 , kFamilyNone, 0  , 655 , 94 ),
  INST(Fcomip          , FpuR               , O_FPU(00,DFF0,_)          , 0                         , F(Fp)                                 , EF(WWWWWW__), 0 , 0 , kFamilyNone, 0  , 661 , 94 ),
  INST(Fcomp           , FpuCom             , O_FPU(00,D8D8,3)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 668 , 93 ),
  INST(Fcompp          , FpuOp              , O_FPU(00,DED9,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 674 , 85 ),
  INST(Fcos            , FpuOp              , O_FPU(00,D9FF,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 681 , 85 ),
  INST(Fdecstp         , FpuOp              , O_FPU(00,D9F6,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 686 , 85 ),
  INST(Fdiv            , FpuArith           , O_FPU(00,F0F8,6)          , 0                         , F(Fp)|F(FPU_M4)|F(FPU_M8)             , EF(________), 0 , 0 , kFamilyNone, 0  , 694 , 86 ),
  INST(Fdivp           , FpuRDef            , O_FPU(00,DEF8,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 699 , 87 ),
  INST(Fdivr           , FpuArith           , O_FPU(00,F8F0,7)          , 0                         , F(Fp)|F(FPU_M4)|F(FPU_M8)             , EF(________), 0 , 0 , kFamilyNone, 0  , 705 , 86 ),
  INST(Fdivrp          , FpuRDef            , O_FPU(00,DEF0,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 711 , 87 ),
  INST(Femms           , X86Op              , O(000F00,0E,_,_,_,_,_,_  ), 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 718 , 85 ),
  INST(Ffree           , FpuR               , O_FPU(00,DDC0,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 724 , 95 ),
  INST(Fiadd           , FpuM               , O_FPU(00,00DA,0)          , 0                         , F(Fp)|F(FPU_M2)|F(FPU_M4)             , EF(________), 0 , 0 , kFamilyNone, 0  , 730 , 96 ),
  INST(Ficom           , FpuM               , O_FPU(00,00DA,2)          , 0                         , F(Fp)|F(FPU_M2)|F(FPU_M4)             , EF(________), 0 , 0 , kFamilyNone, 0  , 736 , 96 ),
  INST(Ficomp          , FpuM               , O_FPU(00,00DA,3)          , 0                         , F(Fp)|F(FPU_M2)|F(FPU_M4)             , EF(________), 0 , 0 , kFamilyNone, 0  , 742 , 96 ),
  INST(Fidiv           , FpuM               , O_FPU(00,00DA,6)          , 0                         , F(Fp)|F(FPU_M2)|F(FPU_M4)             , EF(________), 0 , 0 , kFamilyNone, 0  , 749 , 96 ),
  INST(Fidivr          , FpuM               , O_FPU(00,00DA,7)          , 0                         , F(Fp)|F(FPU_M2)|F(FPU_M4)             , EF(________), 0 , 0 , kFamilyNone, 0  , 755 , 96 ),
  INST(Fild            , FpuM               , O_FPU(00,00DB,0)          , O_FPU(00,00DF,5)          , F(Fp)|F(FPU_M2)|F(FPU_M4)|F(FPU_M8)   , EF(________), 0 , 0 , kFamilyNone, 0  , 762 , 97 ),
  INST(Fimul           , FpuM               , O_FPU(00,00DA,1)          , 0                         , F(Fp)|F(FPU_M2)|F(FPU_M4)             , EF(________), 0 , 0 , kFamilyNone, 0  , 767 , 96 ),
  INST(Fincstp         , FpuOp              , O_FPU(00,D9F7,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 773 , 85 ),
  INST(Finit           , FpuOp              , O_FPU(9B,DBE3,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 781 , 85 ),
  INST(Fist            , FpuM               , O_FPU(00,00DB,2)          , 0                         , F(Fp)|F(FPU_M2)|F(FPU_M4)             , EF(________), 0 , 0 , kFamilyNone, 0  , 787 , 96 ),
  INST(Fistp           , FpuM               , O_FPU(00,00DB,3)          , O_FPU(00,00DF,7)          , F(Fp)|F(FPU_M2)|F(FPU_M4)|F(FPU_M8)   , EF(________), 0 , 0 , kFamilyNone, 0  , 792 , 98 ),
  INST(Fisttp          , FpuM               , O_FPU(00,00DB,1)          , O_FPU(00,00DD,1)          , F(Fp)|F(FPU_M2)|F(FPU_M4)|F(FPU_M8)   , EF(________), 0 , 0 , kFamilyNone, 0  , 798 , 99 ),
  INST(Fisub           , FpuM               , O_FPU(00,00DA,4)          , 0                         , F(Fp)|F(FPU_M2)|F(FPU_M4)             , EF(________), 0 , 0 , kFamilyNone, 0  , 805 , 96 ),
  INST(Fisubr          , FpuM               , O_FPU(00,00DA,5)          , 0                         , F(Fp)|F(FPU_M2)|F(FPU_M4)             , EF(________), 0 , 0 , kFamilyNone, 0  , 811 , 96 ),
  INST(Fld             , FpuFldFst          , O_FPU(00,00D9,0)          , O_FPU(00,00DB,5)          , F(Fp)|F(FPU_M2)|F(FPU_M4)|F(FPU_M8)   , EF(________), 0 , 0 , kFamilyNone, 0  , 818 , 100),
  INST(Fld1            , FpuOp              , O_FPU(00,D9E8,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 822 , 85 ),
  INST(Fldcw           , X86M_Only          , O_FPU(00,00D9,5)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 827 , 101),
  INST(Fldenv          , X86M_Only          , O_FPU(00,00D9,4)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 833 , 102),
  INST(Fldl2e          , FpuOp              , O_FPU(00,D9EA,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 840 , 85 ),
  INST(Fldl2t          , FpuOp              , O_FPU(00,D9E9,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 847 , 85 ),
  INST(Fldlg2          , FpuOp              , O_FPU(00,D9EC,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 854 , 85 ),
  INST(Fldln2          , FpuOp              , O_FPU(00,D9ED,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 861 , 85 ),
  INST(Fldpi           , FpuOp              , O_FPU(00,D9EB,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 868 , 85 ),
  INST(Fldz            , FpuOp              , O_FPU(00,D9EE,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 874 , 85 ),
  INST(Fmul            , FpuArith           , O_FPU(00,C8C8,1)          , 0                         , F(Fp)|F(FPU_M4)|F(FPU_M8)             , EF(________), 0 , 0 , kFamilyNone, 0  , 1960, 86 ),
  INST(Fmulp           , FpuRDef            , O_FPU(00,DEC8,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 879 , 87 ),
  INST(Fnclex          , FpuOp              , O_FPU(00,DBE2,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 885 , 85 ),
  INST(Fninit          , FpuOp              , O_FPU(00,DBE3,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 892 , 85 ),
  INST(Fnop            , FpuOp              , O_FPU(00,D9D0,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 899 , 85 ),
  INST(Fnsave          , X86M_Only          , O_FPU(00,00DD,6)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 904 , 102),
  INST(Fnstcw          , X86M_Only          , O_FPU(00,00D9,7)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 911 , 101),
  INST(Fnstenv         , X86M_Only          , O_FPU(00,00D9,6)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 918 , 102),
  INST(Fnstsw          , FpuStsw            , O_FPU(00,00DD,7)          , O_FPU(00,DFE0,_)          , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 926 , 103),
  INST(Fpatan          , FpuOp              , O_FPU(00,D9F3,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 933 , 85 ),
  INST(Fprem           , FpuOp              , O_FPU(00,D9F8,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 940 , 85 ),
  INST(Fprem1          , FpuOp              , O_FPU(00,D9F5,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 946 , 85 ),
  INST(Fptan           , FpuOp              , O_FPU(00,D9F2,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 953 , 85 ),
  INST(Frndint         , FpuOp              , O_FPU(00,D9FC,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 959 , 85 ),
  INST(Frstor          , X86M_Only          , O_FPU(00,00DD,4)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 967 , 102),
  INST(Fsave           , X86M_Only          , O_FPU(9B,00DD,6)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 974 , 102),
  INST(Fscale          , FpuOp              , O_FPU(00,D9FD,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 980 , 85 ),
  INST(Fsin            , FpuOp              , O_FPU(00,D9FE,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 987 , 85 ),
  INST(Fsincos         , FpuOp              , O_FPU(00,D9FB,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 992 , 85 ),
  INST(Fsqrt           , FpuOp              , O_FPU(00,D9FA,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 1000, 85 ),
  INST(Fst             , FpuFldFst          , O_FPU(00,00D9,2)          , 0                         , F(Fp)|F(FPU_M4)|F(FPU_M8)             , EF(________), 0 , 0 , kFamilyNone, 0  , 1006, 104),
  INST(Fstcw           , X86M_Only          , O_FPU(9B,00D9,7)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 1010, 101),
  INST(Fstenv          , X86M_Only          , O_FPU(9B,00D9,6)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 1016, 102),
  INST(Fstp            , FpuFldFst          , O_FPU(00,00D9,3)          , O(000000,DB,7,_,_,_,_,_  ), F(Fp)|F(FPU_M4)|F(FPU_M8)|F(FPU_M10)  , EF(________), 0 , 0 , kFamilyNone, 0  , 1023, 105),
  INST(Fstsw           , FpuStsw            , O_FPU(9B,00DD,7)          , O_FPU(9B,DFE0,_)          , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 1028, 106),
  INST(Fsub            , FpuArith           , O_FPU(00,E0E8,4)          , 0                         , F(Fp)|F(FPU_M4)|F(FPU_M8)             , EF(________), 0 , 0 , kFamilyNone, 0  , 2038, 86 ),
  INST(Fsubp           , FpuRDef            , O_FPU(00,DEE8,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 1034, 87 ),
  INST(Fsubr           , FpuArith           , O_FPU(00,E8E0,5)          , 0                         , F(Fp)|F(FPU_M4)|F(FPU_M8)             , EF(________), 0 , 0 , kFamilyNone, 0  , 2044, 86 ),
  INST(Fsubrp          , FpuRDef            , O_FPU(00,DEE0,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 1040, 87 ),
  INST(Ftst            , FpuOp              , O_FPU(00,D9E4,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 1047, 85 ),
  INST(Fucom           , FpuRDef            , O_FPU(00,DDE0,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 1052, 87 ),
  INST(Fucomi          , FpuR               , O_FPU(00,DBE8,_)          , 0                         , F(Fp)                                 , EF(WWWWWW__), 0 , 0 , kFamilyNone, 0  , 1058, 94 ),
  INST(Fucomip         , FpuR               , O_FPU(00,DFE8,_)          , 0                         , F(Fp)                                 , EF(WWWWWW__), 0 , 0 , kFamilyNone, 0  , 1065, 94 ),
  INST(Fucomp          , FpuRDef            , O_FPU(00,DDE8,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 1073, 87 ),
  INST(Fucompp         , FpuOp              , O_FPU(00,DAE9,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 1080, 85 ),
  INST(Fwait           , X86Op              , O_FPU(00,00DB,_)          , 0                         , F(Fp)|F(Volatile)                     , EF(________), 0 , 0 , kFamilyNone, 0  , 1088, 107),
  INST(Fxam            , FpuOp              , O_FPU(00,D9E5,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 1094, 85 ),
  INST(Fxch            , FpuR               , O_FPU(00,D9C8,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 1099, 87 ),
  INST(Fxrstor         , X86M_Only          , O(000F00,AE,1,_,_,_,_,_  ), 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 1104, 102),
  INST(Fxrstor64       , X86M_Only          , O(000F00,AE,1,_,1,_,_,_  ), 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 1112, 108),
  INST(Fxsave          , X86M_Only          , O(000F00,AE,0,_,_,_,_,_  ), 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 1122, 102),
  INST(Fxsave64        , X86M_Only          , O(000F00,AE,0,_,1,_,_,_  ), 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 1129, 108),
  INST(Fxtract         , FpuOp              , O_FPU(00,D9F4,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 1138, 85 ),
  INST(Fyl2x           , FpuOp              , O_FPU(00,D9F1,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 1146, 85 ),
  INST(Fyl2xp1         , FpuOp              , O_FPU(00,D9F9,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 1152, 85 ),
  INST(Haddpd          , ExtRm              , O(660F00,7C,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 33 , 5339, 6  ),
  INST(Haddps          , ExtRm              , O(F20F00,7C,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 33 , 5347, 6  ),
  INST(Hlt             , X86Op              , O(000000,F4,_,_,_,_,_,_  ), 0                         , 0                                     , EF(________), 0 , 0 , kFamilyNone, 0  , 1160, 41 ),
  INST(Hsubpd          , ExtRm              , O(660F00,7D,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 34 , 5355, 6  ),
  INST(Hsubps          , ExtRm              , O(F20F00,7D,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 34 , 5363, 6  ),
  INST(Idiv            , X86M_GPB_MulDiv    , O(000000,F6,7,_,x,_,_,_  ), 0                         , F(RW)|F(Special)                      , EF(UUUUUU__), 0 , 0 , kFamilyNone, 0  , 750 , 80 ),
  INST(Imul            , X86Imul            , O(000000,F6,5,_,x,_,_,_  ), 0                         , F(RW)|F(Special)                      , EF(WUUUUW__), 0 , 0 , kFamilyNone, 0  , 768 , 109),
  INST(In              , X86In              , O(000000,EC,_,_,_,_,_,_  ), O(000000,E4,_,_,_,_,_,_  ), F(WO)|F(Volatile)|F(Special)          , EF(________), 0 , 0 , kFamilyNone, 0  , 1956, 110),
  INST(Inc             , X86IncDec          , O(000000,FE,0,_,x,_,_,_  ), O(000000,40,_,_,x,_,_,_  ), F(RW)|F(Lock)                         , EF(WWWWW___), 0 , 0 , kFamilyNone, 0  , 1164, 111),
  INST(Ins             , X86Ins             , O(000000,6C,_,_,_,_,_,_  ), 0                         , F(WO)|F(Volatile)|F(Special)|F(Rep)   , EF(________), 0 , 0 , kFamilyNone, 0  , 1168, 112),
  INST(Insertps        , ExtRmi             , O(660F3A,21,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 35 , 5499, 55 ),
  INST(Insertq         , ExtInsertq         , O(F20F00,79,_,_,_,_,_,_  ), O(F20F00,78,_,_,_,_,_,_  ), F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 32 , 1172, 113),
  INST(Int             , X86Int             , O(000000,CD,_,_,_,_,_,_  ), 0                         , F(Volatile)                           , EF(___W___W), 0 , 0 , kFamilyNone, 0  , 963 , 114),
  INST(Int3            , X86Op              , O(000000,CC,_,_,_,_,_,_  ), 0                         , F(Volatile)                           , EF(___W___W), 0 , 0 , kFamilyNone, 0  , 1180, 115),
  INST(Into            , X86Op              , O(000000,CE,_,_,_,_,_,_  ), 0                         , F(Volatile)                           , EF(___W___W), 0 , 0 , kFamilyNone, 0  , 1185, 115),
  INST(Invd            , X86Op              , O(000F00,08,_,_,_,_,_,_  ), 0                         , 0                                     , EF(________), 0 , 0 , kFamilyNone, 0  , 9420, 41 ),
  INST(Invlpg          , X86M_Only          , O(000F00,01,7,_,_,_,_,_  ), 0                         , F(RO)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 1190, 116),
  INST(Invpcid         , X86Rm_NoRexW       , O(660F38,82,_,_,_,_,_,_  ), 0                         , F(RO)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 1197, 117),
  INST(Ja              , X86Jcc             , O(000F00,87,_,_,_,_,_,_  ), O(000000,77,_,_,_,_,_,_  ), F(Volatile)                           , EF(__R__R__), 0 , 0 , kFamilyNone, 0  , 1205, 118),
  INST(Jae             , X86Jcc             , O(000F00,83,_,_,_,_,_,_  ), O(000000,73,_,_,_,_,_,_  ), F(Volatile)                           , EF(_____R__), 0 , 0 , kFamilyNone, 0  , 1208, 119),
  INST(Jb              , X86Jcc             , O(000F00,82,_,_,_,_,_,_  ), O(000000,72,_,_,_,_,_,_  ), F(Volatile)                           , EF(_____R__), 0 , 0 , kFamilyNone, 0  , 1212, 120),
  INST(Jbe             , X86Jcc             , O(000F00,86,_,_,_,_,_,_  ), O(000000,76,_,_,_,_,_,_  ), F(Volatile)                           , EF(__R__R__), 0 , 0 , kFamilyNone, 0  , 1215, 121),
  INST(Jc              , X86Jcc             , O(000F00,82,_,_,_,_,_,_  ), O(000000,72,_,_,_,_,_,_  ), F(Volatile)                           , EF(_____R__), 0 , 0 , kFamilyNone, 0  , 1219, 122),
  INST(Je              , X86Jcc             , O(000F00,84,_,_,_,_,_,_  ), O(000000,74,_,_,_,_,_,_  ), F(Volatile)                           , EF(__R_____), 0 , 0 , kFamilyNone, 0  , 1222, 123),
  INST(Jecxz           , X86JecxzLoop       , 0                         , O(000000,E3,_,_,_,_,_,_  ), F(Volatile)|F(Special)                , EF(________), 0 , 0 , kFamilyNone, 0  , 1225, 124),
  INST(Jg              , X86Jcc             , O(000F00,8F,_,_,_,_,_,_  ), O(000000,7F,_,_,_,_,_,_  ), F(Volatile)                           , EF(RRR_____), 0 , 0 , kFamilyNone, 0  , 1231, 125),
  INST(Jge             , X86Jcc             , O(000F00,8D,_,_,_,_,_,_  ), O(000000,7D,_,_,_,_,_,_  ), F(Volatile)                           , EF(RR______), 0 , 0 , kFamilyNone, 0  , 1234, 126),
  INST(Jl              , X86Jcc             , O(000F00,8C,_,_,_,_,_,_  ), O(000000,7C,_,_,_,_,_,_  ), F(Volatile)                           , EF(RR______), 0 , 0 , kFamilyNone, 0  , 1238, 127),
  INST(Jle             , X86Jcc             , O(000F00,8E,_,_,_,_,_,_  ), O(000000,7E,_,_,_,_,_,_  ), F(Volatile)                           , EF(RRR_____), 0 , 0 , kFamilyNone, 0  , 1241, 128),
  INST(Jmp             , X86Jmp             , O(000000,FF,4,_,_,_,_,_  ), O(000000,EB,_,_,_,_,_,_  ), F(Volatile)                           , EF(________), 0 , 0 , kFamilyNone, 0  , 1245, 129),
  INST(Jna             , X86Jcc             , O(000F00,86,_,_,_,_,_,_  ), O(000000,76,_,_,_,_,_,_  ), F(Volatile)                           , EF(__R__R__), 0 , 0 , kFamilyNone, 0  , 1249, 121),
  INST(Jnae            , X86Jcc             , O(000F00,82,_,_,_,_,_,_  ), O(000000,72,_,_,_,_,_,_  ), F(Volatile)                           , EF(_____R__), 0 , 0 , kFamilyNone, 0  , 1253, 120),
  INST(Jnb             , X86Jcc             , O(000F00,83,_,_,_,_,_,_  ), O(000000,73,_,_,_,_,_,_  ), F(Volatile)                           , EF(_____R__), 0 , 0 , kFamilyNone, 0  , 1258, 119),
  INST(Jnbe            , X86Jcc             , O(000F00,87,_,_,_,_,_,_  ), O(000000,77,_,_,_,_,_,_  ), F(Volatile)                           , EF(__R__R__), 0 , 0 , kFamilyNone, 0  , 1262, 118),
  INST(Jnc             , X86Jcc             , O(000F00,83,_,_,_,_,_,_  ), O(000000,73,_,_,_,_,_,_  ), F(Volatile)                           , EF(_____R__), 0 , 0 , kFamilyNone, 0  , 1267, 130),
  INST(Jne             , X86Jcc             , O(000F00,85,_,_,_,_,_,_  ), O(000000,75,_,_,_,_,_,_  ), F(Volatile)                           , EF(__R_____), 0 , 0 , kFamilyNone, 0  , 1271, 131),
  INST(Jng             , X86Jcc             , O(000F00,8E,_,_,_,_,_,_  ), O(000000,7E,_,_,_,_,_,_  ), F(Volatile)                           , EF(RRR_____), 0 , 0 , kFamilyNone, 0  , 1275, 128),
  INST(Jnge            , X86Jcc             , O(000F00,8C,_,_,_,_,_,_  ), O(000000,7C,_,_,_,_,_,_  ), F(Volatile)                           , EF(RR______), 0 , 0 , kFamilyNone, 0  , 1279, 127),
  INST(Jnl             , X86Jcc             , O(000F00,8D,_,_,_,_,_,_  ), O(000000,7D,_,_,_,_,_,_  ), F(Volatile)                           , EF(RR______), 0 , 0 , kFamilyNone, 0  , 1284, 126),
  INST(Jnle            , X86Jcc             , O(000F00,8F,_,_,_,_,_,_  ), O(000000,7F,_,_,_,_,_,_  ), F(Volatile)                           , EF(RRR_____), 0 , 0 , kFamilyNone, 0  , 1288, 125),
  INST(Jno             , X86Jcc             , O(000F00,81,_,_,_,_,_,_  ), O(000000,71,_,_,_,_,_,_  ), F(Volatile)                           , EF(R_______), 0 , 0 , kFamilyNone, 0  , 1293, 132),
  INST(Jnp             , X86Jcc             , O(000F00,8B,_,_,_,_,_,_  ), O(000000,7B,_,_,_,_,_,_  ), F(Volatile)                           , EF(____R___), 0 , 0 , kFamilyNone, 0  , 1297, 133),
  INST(Jns             , X86Jcc             , O(000F00,89,_,_,_,_,_,_  ), O(000000,79,_,_,_,_,_,_  ), F(Volatile)                           , EF(_R______), 0 , 0 , kFamilyNone, 0  , 1301, 134),
  INST(Jnz             , X86Jcc             , O(000F00,85,_,_,_,_,_,_  ), O(000000,75,_,_,_,_,_,_  ), F(Volatile)                           , EF(__R_____), 0 , 0 , kFamilyNone, 0  , 1305, 131),
  INST(Jo              , X86Jcc             , O(000F00,80,_,_,_,_,_,_  ), O(000000,70,_,_,_,_,_,_  ), F(Volatile)                           , EF(R_______), 0 , 0 , kFamilyNone, 0  , 1309, 135),
  INST(Jp              , X86Jcc             , O(000F00,8A,_,_,_,_,_,_  ), O(000000,7A,_,_,_,_,_,_  ), F(Volatile)                           , EF(____R___), 0 , 0 , kFamilyNone, 0  , 1312, 136),
  INST(Jpe             , X86Jcc             , O(000F00,8A,_,_,_,_,_,_  ), O(000000,7A,_,_,_,_,_,_  ), F(Volatile)                           , EF(____R___), 0 , 0 , kFamilyNone, 0  , 1315, 136),
  INST(Jpo             , X86Jcc             , O(000F00,8B,_,_,_,_,_,_  ), O(000000,7B,_,_,_,_,_,_  ), F(Volatile)                           , EF(____R___), 0 , 0 , kFamilyNone, 0  , 1319, 133),
  INST(Js              , X86Jcc             , O(000F00,88,_,_,_,_,_,_  ), O(000000,78,_,_,_,_,_,_  ), F(Volatile)                           , EF(_R______), 0 , 0 , kFamilyNone, 0  , 1323, 137),
  INST(Jz              , X86Jcc             , O(000F00,84,_,_,_,_,_,_  ), O(000000,74,_,_,_,_,_,_  ), F(Volatile)                           , EF(__R_____), 0 , 0 , kFamilyNone, 0  , 1326, 123),
  INST(Kaddb           , VexRvm             , V(660F00,4A,_,1,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1329, 138),
  INST(Kaddd           , VexRvm             , V(660F00,4A,_,1,1,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1335, 138),
  INST(Kaddq           , VexRvm             , V(000F00,4A,_,1,1,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1341, 138),
  INST(Kaddw           , VexRvm             , V(000F00,4A,_,1,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1347, 138),
  INST(Kandb           , VexRvm             , V(660F00,41,_,1,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1353, 138),
  INST(Kandd           , VexRvm             , V(660F00,41,_,1,1,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1359, 138),
  INST(Kandnb          , VexRvm             , V(660F00,42,_,1,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1365, 138),
  INST(Kandnd          , VexRvm             , V(660F00,42,_,1,1,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1372, 138),
  INST(Kandnq          , VexRvm             , V(000F00,42,_,1,1,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1379, 138),
  INST(Kandnw          , VexRvm             , V(000F00,42,_,1,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1386, 138),
  INST(Kandq           , VexRvm             , V(000F00,41,_,1,1,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1393, 138),
  INST(Kandw           , VexRvm             , V(000F00,41,_,1,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1399, 138),
  INST(Kmovb           , VexKmov            , V(660F00,90,_,0,0,_,_,_  ), V(660F00,92,_,0,0,_,_,_  ), F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1405, 139),
  INST(Kmovd           , VexKmov            , V(660F00,90,_,0,1,_,_,_  ), V(F20F00,92,_,0,0,_,_,_  ), F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 7301, 140),
  INST(Kmovq           , VexKmov            , V(000F00,90,_,0,1,_,_,_  ), V(F20F00,92,_,0,1,_,_,_  ), F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 7312, 141),
  INST(Kmovw           , VexKmov            , V(000F00,90,_,0,0,_,_,_  ), V(000F00,92,_,0,0,_,_,_  ), F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1411, 142),
  INST(Knotb           , VexRm              , V(660F00,44,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1417, 143),
  INST(Knotd           , VexRm              , V(660F00,44,_,0,1,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1423, 143),
  INST(Knotq           , VexRm              , V(000F00,44,_,0,1,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1429, 143),
  INST(Knotw           , VexRm              , V(000F00,44,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1435, 143),
  INST(Korb            , VexRvm             , V(660F00,45,_,1,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1441, 138),
  INST(Kord            , VexRvm             , V(660F00,45,_,1,1,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1446, 138),
  INST(Korq            , VexRvm             , V(000F00,45,_,1,1,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1451, 138),
  INST(Kortestb        , VexRm              , V(660F00,98,_,0,0,_,_,_  ), 0                         , F(RO)|F(Vex)                          , EF(WWWWWW__), 0 , 0 , kFamilyNone, 0  , 1456, 144),
  INST(Kortestd        , VexRm              , V(660F00,98,_,0,1,_,_,_  ), 0                         , F(RO)|F(Vex)                          , EF(WWWWWW__), 0 , 0 , kFamilyNone, 0  , 1465, 144),
  INST(Kortestq        , VexRm              , V(000F00,98,_,0,1,_,_,_  ), 0                         , F(RO)|F(Vex)                          , EF(WWWWWW__), 0 , 0 , kFamilyNone, 0  , 1474, 144),
  INST(Kortestw        , VexRm              , V(000F00,98,_,0,0,_,_,_  ), 0                         , F(RO)|F(Vex)                          , EF(WWWWWW__), 0 , 0 , kFamilyNone, 0  , 1483, 144),
  INST(Korw            , VexRvm             , V(000F00,45,_,1,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1492, 138),
  INST(Kshiftlb        , VexRmi             , V(660F3A,32,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1497, 145),
  INST(Kshiftld        , VexRmi             , V(660F3A,33,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1506, 145),
  INST(Kshiftlq        , VexRmi             , V(660F3A,33,_,0,1,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1515, 145),
  INST(Kshiftlw        , VexRmi             , V(660F3A,32,_,0,1,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1524, 145),
  INST(Kshiftrb        , VexRmi             , V(660F3A,30,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1533, 145),
  INST(Kshiftrd        , VexRmi             , V(660F3A,31,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1542, 145),
  INST(Kshiftrq        , VexRmi             , V(660F3A,31,_,0,1,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1551, 145),
  INST(Kshiftrw        , VexRmi             , V(660F3A,30,_,0,1,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1560, 145),
  INST(Ktestb          , VexRm              , V(660F00,99,_,0,0,_,_,_  ), 0                         , F(RO)|F(Vex)                          , EF(WWWWWW__), 0 , 0 , kFamilyNone, 0  , 1569, 144),
  INST(Ktestd          , VexRm              , V(660F00,99,_,0,1,_,_,_  ), 0                         , F(RO)|F(Vex)                          , EF(WWWWWW__), 0 , 0 , kFamilyNone, 0  , 1576, 144),
  INST(Ktestq          , VexRm              , V(000F00,99,_,0,1,_,_,_  ), 0                         , F(RO)|F(Vex)                          , EF(WWWWWW__), 0 , 0 , kFamilyNone, 0  , 1583, 144),
  INST(Ktestw          , VexRm              , V(000F00,99,_,0,0,_,_,_  ), 0                         , F(RO)|F(Vex)                          , EF(WWWWWW__), 0 , 0 , kFamilyNone, 0  , 1590, 144),
  INST(Kunpckbw        , VexRvm             , V(660F00,4B,_,1,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1597, 138),
  INST(Kunpckdq        , VexRvm             , V(000F00,4B,_,1,1,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1606, 138),
  INST(Kunpckwd        , VexRvm             , V(000F00,4B,_,1,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1615, 138),
  INST(Kxnorb          , VexRvm             , V(660F00,46,_,1,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1624, 138),
  INST(Kxnord          , VexRvm             , V(660F00,46,_,1,1,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1631, 138),
  INST(Kxnorq          , VexRvm             , V(000F00,46,_,1,1,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1638, 138),
  INST(Kxnorw          , VexRvm             , V(000F00,46,_,1,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1645, 138),
  INST(Kxorb           , VexRvm             , V(660F00,47,_,1,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1652, 138),
  INST(Kxord           , VexRvm             , V(660F00,47,_,1,1,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1658, 138),
  INST(Kxorq           , VexRvm             , V(000F00,47,_,1,1,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1664, 138),
  INST(Kxorw           , VexRvm             , V(000F00,47,_,1,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1670, 138),
  INST(Lahf            , X86Op              , O(000000,9F,_,_,_,_,_,_  ), 0                         , F(RW)|F(Volatile)|F(Special)          , EF(_RRRRR__), 0 , 0 , kFamilyNone, 0  , 1676, 146),
  INST(Lar             , X86Rm              , O(000F00,02,_,_,_,_,_,_  ), 0                         , F(RW)|F(Volatile)                     , EF(__W_____), 0 , 0 , kFamilyNone, 0  , 1681, 147),
  INST(Lddqu           , ExtRm              , O(F20F00,F0,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 36 , 5509, 148),
  INST(Ldmxcsr         , X86M_Only          , O(000F00,AE,2,_,_,_,_,_  ), 0                         , F(RO)|F(Volatile)                     , EF(________), 0 , 0 , kFamilyNone, 0  , 5516, 149),
  INST(Lea             , X86Lea             , O(000000,8D,_,_,x,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 1685, 150),
  INST(Leave           , X86Op              , O(000000,C9,_,_,_,_,_,_  ), 0                         , F(Volatile)|F(Special)                , EF(________), 0 , 0 , kFamilyNone, 0  , 1689, 151),
  INST(Lfence          , X86Fence           , O(000F00,AE,5,_,_,_,_,_  ), 0                         , F(Volatile)                           , EF(________), 0 , 0 , kFamilyNone, 0  , 1695, 81 ),
  INST(Lgdt            , X86M_Only          , O(000F00,01,2,_,_,_,_,_  ), 0                         , F(RO)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 1702, 116),
  INST(Lidt            , X86M_Only          , O(000F00,01,3,_,_,_,_,_  ), 0                         , F(RO)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 1707, 116),
  INST(Lldt            , X86M               , O(000F00,00,2,_,_,_,_,_  ), 0                         , F(RO)|F(Volatile)                     , EF(________), 0 , 0 , kFamilyNone, 0  , 1712, 152),
  INST(Lmsw            , X86M               , O(000F00,01,6,_,_,_,_,_  ), 0                         , F(RO)|F(Volatile)                     , EF(________), 0 , 0 , kFamilyNone, 0  , 1717, 152),
  INST(Lods            , X86StrRm           , O(000000,AC,_,_,_,_,_,_  ), 0                         , F(WO)|F(Special)|F(Rep)               , EF(______R_), 0 , 1 , kFamilyNone, 0  , 1722, 153),
  INST(Loop            , X86JecxzLoop       , 0                         , O(000000,E2,_,_,_,_,_,_  ), F(RW)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 1727, 154),
  INST(Loope           , X86JecxzLoop       , 0                         , O(000000,E1,_,_,_,_,_,_  ), F(RW)                                 , EF(__R_____), 0 , 0 , kFamilyNone, 0  , 1732, 155),
  INST(Loopne          , X86JecxzLoop       , 0                         , O(000000,E0,_,_,_,_,_,_  ), F(RW)                                 , EF(__R_____), 0 , 0 , kFamilyNone, 0  , 1738, 156),
  INST(Lsl             , X86Rm              , O(000F00,03,_,_,_,_,_,_  ), 0                         , F(RW)|F(Volatile)                     , EF(__W_____), 0 , 0 , kFamilyNone, 0  , 1745, 157),
  INST(Ltr             , X86M               , O(000F00,00,3,_,_,_,_,_  ), 0                         , F(RO)|F(Volatile)                     , EF(________), 0 , 0 , kFamilyNone, 0  , 1749, 152),
  INST(Lzcnt           , X86Rm              , O(F30F00,BD,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(UUWUUW__), 0 , 0 , kFamilyNone, 0  , 1753, 158),
  INST(Maskmovdqu      , ExtRm_ZDI          , O(660F00,57,_,_,_,_,_,_  ), 0                         , F(RO)|F(Special)                      , EF(________), 0 , 0 , kFamilySse , 37 , 5525, 159),
  INST(Maskmovq        , ExtRm_ZDI          , O(000F00,F7,_,_,_,_,_,_  ), 0                         , F(RO)|F(Special)                      , EF(________), 0 , 0 , kFamilySse , 38 , 7309, 160),
  INST(Maxpd           , ExtRm              , O(660F00,5F,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 39 , 5559, 6  ),
  INST(Maxps           , ExtRm              , O(000F00,5F,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 40 , 5566, 6  ),
  INST(Maxsd           , ExtRm              , O(F20F00,5F,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 39 , 7328, 7  ),
  INST(Maxss           , ExtRm              , O(F30F00,5F,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 40 , 5580, 8  ),
  INST(Mfence          , X86Fence           , O(000F00,AE,6,_,_,_,_,_  ), 0                         , F(RW)|F(Volatile)                     , EF(________), 0 , 0 , kFamilyNone, 0  , 1759, 161),
  INST(Minpd           , ExtRm              , O(660F00,5D,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 41 , 5587, 6  ),
  INST(Minps           , ExtRm              , O(000F00,5D,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 42 , 5594, 6  ),
  INST(Minsd           , ExtRm              , O(F20F00,5D,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 41 , 7392, 7  ),
  INST(Minss           , ExtRm              , O(F30F00,5D,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 42 , 5608, 8  ),
  INST(Monitor         , X86Op              , O(000F01,C8,_,_,_,_,_,_  ), 0                         , F(RO)|F(Volatile)|F(Special)          , EF(________), 0 , 0 , kFamilyNone, 0  , 1766, 162),
  INST(Mov             , X86Mov             , 0                         , 0                         , F(WO)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 138 , 163),
  INST(Movapd          , ExtMov             , O(660F00,28,_,_,_,_,_,_  ), O(660F00,29,_,_,_,_,_,_  ), F(WO)                                 , EF(________), 0 , 16, kFamilySse , 43 , 5615, 164),
  INST(Movaps          , ExtMov             , O(000F00,28,_,_,_,_,_,_  ), O(000F00,29,_,_,_,_,_,_  ), F(WO)                                 , EF(________), 0 , 16, kFamilySse , 44 , 5623, 165),
  INST(Movbe           , ExtMovbe           , O(000F38,F0,_,_,x,_,_,_  ), O(000F38,F1,_,_,x,_,_,_  ), F(WO)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 597 , 166),
  INST(Movd            , ExtMovd            , O(000F00,6E,_,_,_,_,_,_  ), O(000F00,7E,_,_,_,_,_,_  ), F(WO)                                 , EF(________), 0 , 16, kFamilySse , 45 , 7302, 167),
  INST(Movddup         , ExtMov             , O(F20F00,12,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 46 , 5637, 64 ),
  INST(Movdq2q         , ExtMov             , O(F20F00,D6,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 16 , 1774, 168),
  INST(Movdqa          , ExtMov             , O(660F00,6F,_,_,_,_,_,_  ), O(660F00,7F,_,_,_,_,_,_  ), F(WO)                                 , EF(________), 0 , 16, kFamilySse , 47 , 5646, 169),
  INST(Movdqu          , ExtMov             , O(F30F00,6F,_,_,_,_,_,_  ), O(F30F00,7F,_,_,_,_,_,_  ), F(WO)                                 , EF(________), 0 , 16, kFamilySse , 43 , 5529, 170),
  INST(Movhlps         , ExtMov             , O(000F00,12,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 48 , 5721, 171),
  INST(Movhpd          , ExtMov             , O(660F00,16,_,_,_,_,_,_  ), O(660F00,17,_,_,_,_,_,_  ), F(RW)                                 , EF(________), 8 , 8 , kFamilySse , 49 , 5730, 172),
  INST(Movhps          , ExtMov             , O(000F00,16,_,_,_,_,_,_  ), O(000F00,17,_,_,_,_,_,_  ), F(RW)                                 , EF(________), 8 , 8 , kFamilySse , 50 , 5738, 173),
  INST(Movlhps         , ExtMov             , O(000F00,16,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 8 , 8 , kFamilySse , 48 , 5746, 174),
  INST(Movlpd          , ExtMov             , O(660F00,12,_,_,_,_,_,_  ), O(660F00,13,_,_,_,_,_,_  ), F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 49 , 5755, 175),
  INST(Movlps          , ExtMov             , O(000F00,12,_,_,_,_,_,_  ), O(000F00,13,_,_,_,_,_,_  ), F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 50 , 5763, 176),
  INST(Movmskpd        , ExtMov             , O(660F00,50,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 51 , 5771, 177),
  INST(Movmskps        , ExtMov             , O(000F00,50,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 52 , 5781, 177),
  INST(Movntdq         , ExtMov             , 0                         , O(660F00,E7,_,_,_,_,_,_  ), F(WO)                                 , EF(________), 0 , 16, kFamilySse , 51 , 5791, 178),
  INST(Movntdqa        , ExtMov             , O(660F38,2A,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 53 , 5800, 148),
  INST(Movnti          , ExtMovnti          , O(000F00,C3,_,_,x,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilyNone, 0  , 1782, 179),
  INST(Movntpd         , ExtMov             , 0                         , O(660F00,2B,_,_,_,_,_,_  ), F(WO)                                 , EF(________), 0 , 16, kFamilySse , 54 , 5810, 180),
  INST(Movntps         , ExtMov             , 0                         , O(000F00,2B,_,_,_,_,_,_  ), F(WO)                                 , EF(________), 0 , 16, kFamilySse , 55 , 5819, 181),
  INST(Movntq          , ExtMov             , 0                         , O(000F00,E7,_,_,_,_,_,_  ), F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 38 , 1789, 182),
  INST(Movntsd         , ExtMov             , 0                         , O(F20F00,2B,_,_,_,_,_,_  ), F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 32 , 1796, 183),
  INST(Movntss         , ExtMov             , 0                         , O(F30F00,2B,_,_,_,_,_,_  ), F(WO)                                 , EF(________), 0 , 4 , kFamilySse , 32 , 1804, 184),
  INST(Movq            , ExtMovq            , O(000F00,6E,_,_,x,_,_,_  ), O(000F00,7E,_,_,x,_,_,_  ), F(WO)                                 , EF(________), 0 , 16, kFamilySse , 56 , 7313, 185),
  INST(Movq2dq         , ExtRm              , O(F30F00,D6,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 16 , 1812, 186),
  INST(Movs            , X86StrMm           , O(000000,A4,_,_,_,_,_,_  ), 0                         , F(WO)|F(Special)|F(Rep)               , EF(________), 0 , 0 , kFamilyNone, 0  , 411 , 187),
  INST(Movsd           , ExtMov             , O(F20F00,10,_,_,_,_,_,_  ), O(F20F00,11,_,_,_,_,_,_  ), F(WO)|F(ZeroIfMem)                    , EF(________), 0 , 8 , kFamilySse , 57 , 5834, 188),
  INST(Movshdup        , ExtRm              , O(F30F00,16,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 58 , 5841, 65 ),
  INST(Movsldup        , ExtRm              , O(F30F00,12,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 58 , 5851, 65 ),
  INST(Movss           , ExtMov             , O(F30F00,10,_,_,_,_,_,_  ), O(F30F00,11,_,_,_,_,_,_  ), F(WO)|F(ZeroIfMem)                    , EF(________), 0 , 4 , kFamilySse , 59 , 5861, 189),
  INST(Movsx           , X86MovsxMovzx      , O(000F00,BE,_,_,x,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 1820, 190),
  INST(Movsxd          , X86Rm              , O(000000,63,_,_,1,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 1826, 191),
  INST(Movupd          , ExtMov             , O(660F00,10,_,_,_,_,_,_  ), O(660F00,11,_,_,_,_,_,_  ), F(WO)                                 , EF(________), 0 , 16, kFamilySse , 60 , 5868, 192),
  INST(Movups          , ExtMov             , O(000F00,10,_,_,_,_,_,_  ), O(000F00,11,_,_,_,_,_,_  ), F(WO)                                 , EF(________), 0 , 16, kFamilySse , 61 , 5876, 193),
  INST(Movzx           , X86MovsxMovzx      , O(000F00,B6,_,_,x,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 1833, 190),
  INST(Mpsadbw         , ExtRmi             , O(660F3A,42,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 62 , 5884, 18 ),
  INST(Mul             , X86M_GPB_MulDiv    , O(000000,F6,4,_,x,_,_,_  ), 0                         , F(RW)|F(Special)                      , EF(WUUUUW__), 0 , 0 , kFamilyNone, 0  , 769 , 194),
  INST(Mulpd           , ExtRm              , O(660F00,59,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 63 , 5893, 6  ),
  INST(Mulps           , ExtRm              , O(000F00,59,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 64 , 5900, 6  ),
  INST(Mulsd           , ExtRm              , O(F20F00,59,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 63 , 5907, 7  ),
  INST(Mulss           , ExtRm              , O(F30F00,59,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 64 , 5914, 8  ),
  INST(Mulx            , VexRvm_ZDX_Wx      , V(F20F38,F6,_,0,x,_,_,_  ), 0                         , F(RW)|F(Special)                      , EF(________), 0 , 0 , kFamilyNone, 0  , 1839, 195),
  INST(Mwait           , X86Op              , O(000F01,C9,_,_,_,_,_,_  ), 0                         , F(RO)|F(Volatile)|F(Special)          , EF(________), 0 , 0 , kFamilyNone, 0  , 1844, 196),
  INST(Neg             , X86M_GPB           , O(000000,F6,3,_,x,_,_,_  ), 0                         , F(RW)|F(Lock)                         , EF(WWWWWW__), 0 , 0 , kFamilyNone, 0  , 1850, 197),
  INST(Nop             , X86Op              , O(000000,90,_,_,_,_,_,_  ), 0                         , 0                                     , EF(________), 0 , 0 , kFamilyNone, 0  , 900 , 198),
  INST(Not             , X86M_GPB           , O(000000,F6,2,_,x,_,_,_  ), 0                         , F(RW)|F(Lock)                         , EF(________), 0 , 0 , kFamilyNone, 0  , 1854, 199),
  INST(Or              , X86Arith           , O(000000,08,1,_,x,_,_,_  ), 0                         , F(RW)|F(Lock)                         , EF(WWWUWW__), 0 , 0 , kFamilyNone, 0  , 1109, 12 ),
  INST(Orpd            , ExtRm              , O(660F00,56,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 65 , 9386, 14 ),
  INST(Orps            , ExtRm              , O(000F00,56,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 66 , 9393, 14 ),
  INST(Out             , X86Out             , O(000000,EE,_,_,_,_,_,_  ), O(000000,E6,_,_,_,_,_,_  ), F(RO)|F(Volatile)|F(Special)          , EF(________), 0 , 0 , kFamilyNone, 0  , 1858, 200),
  INST(Outs            , X86Outs            , O(000000,6E,_,_,_,_,_,_  ), 0                         , F(RO)|F(Volatile)|F(Special)|F(Rep)   , EF(________), 0 , 0 , kFamilyNone, 0  , 1862, 201),
  INST(Pabsb           , ExtRm_P            , O(000F38,1C,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 67 , 5954, 202),
  INST(Pabsd           , ExtRm_P            , O(000F38,1E,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 67 , 5961, 202),
  INST(Pabsw           , ExtRm_P            , O(000F38,1D,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 68 , 5975, 202),
  INST(Packssdw        , ExtRm_P            , O(000F00,6B,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 69 , 5982, 203),
  INST(Packsswb        , ExtRm_P            , O(000F00,63,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 69 , 5992, 203),
  INST(Packusdw        , ExtRm              , O(660F38,2B,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 70 , 6002, 6  ),
  INST(Packuswb        , ExtRm_P            , O(000F00,67,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 69 , 6012, 203),
  INST(Paddb           , ExtRm_P            , O(000F00,FC,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 69 , 6022, 203),
  INST(Paddd           , ExtRm_P            , O(000F00,FE,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 69 , 6029, 203),
  INST(Paddq           , ExtRm_P            , O(000F00,D4,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 71 , 6036, 203),
  INST(Paddsb          , ExtRm_P            , O(000F00,EC,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 69 , 6043, 203),
  INST(Paddsw          , ExtRm_P            , O(000F00,ED,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 69 , 6051, 203),
  INST(Paddusb         , ExtRm_P            , O(000F00,DC,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 69 , 6059, 203),
  INST(Paddusw         , ExtRm_P            , O(000F00,DD,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 69 , 6068, 203),
  INST(Paddw           , ExtRm_P            , O(000F00,FD,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 69 , 6077, 203),
  INST(Palignr         , ExtRmi_P           , O(000F3A,0F,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 72 , 6084, 204),
  INST(Pand            , ExtRm_P            , O(000F00,DB,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 69 , 6093, 205),
  INST(Pandn           , ExtRm_P            , O(000F00,DF,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 73 , 6106, 206),
  INST(Pause           , X86Op              , O(F30000,90,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 1867, 207),
  INST(Pavgb           , ExtRm_P            , O(000F00,E0,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 74 , 6136, 203),
  INST(Pavgusb         , Ext3dNow           , O(000F0F,BF,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 75 , 1873, 208),
  INST(Pavgw           , ExtRm_P            , O(000F00,E3,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 76 , 6143, 203),
  INST(Pblendvb        , ExtRm_XMM0         , O(660F38,10,_,_,_,_,_,_  ), 0                         , F(RW)|F(Special)                      , EF(________), 0 , 0 , kFamilySse , 77 , 6159, 19 ),
  INST(Pblendw         , ExtRmi             , O(660F3A,0E,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 78 , 6169, 18 ),
  INST(Pclmulqdq       , ExtRmi             , O(660F3A,44,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 79 , 6262, 18 ),
  INST(Pcmpeqb         , ExtRm_P            , O(000F00,74,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 80 , 6294, 206),
  INST(Pcmpeqd         , ExtRm_P            , O(000F00,76,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 80 , 6303, 206),
  INST(Pcmpeqq         , ExtRm              , O(660F38,29,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 81 , 6312, 209),
  INST(Pcmpeqw         , ExtRm_P            , O(000F00,75,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 80 , 6321, 206),
  INST(Pcmpestri       , ExtRmi             , O(660F3A,61,_,_,_,_,_,_  ), 0                         , F(WO)|F(Special)                      , EF(________), 0 , 0 , kFamilySse , 82 , 6330, 210),
  INST(Pcmpestrm       , ExtRmi             , O(660F3A,60,_,_,_,_,_,_  ), 0                         , F(WO)|F(Special)                      , EF(________), 0 , 0 , kFamilySse , 82 , 6341, 211),
  INST(Pcmpgtb         , ExtRm_P            , O(000F00,64,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 80 , 6352, 206),
  INST(Pcmpgtd         , ExtRm_P            , O(000F00,66,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 80 , 6361, 206),
  INST(Pcmpgtq         , ExtRm              , O(660F38,37,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 83 , 6370, 209),
  INST(Pcmpgtw         , ExtRm_P            , O(000F00,65,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 80 , 6379, 206),
  INST(Pcmpistri       , ExtRmi             , O(660F3A,63,_,_,_,_,_,_  ), 0                         , F(WO)|F(Special)                      , EF(________), 0 , 0 , kFamilySse , 82 , 6388, 212),
  INST(Pcmpistrm       , ExtRmi             , O(660F3A,62,_,_,_,_,_,_  ), 0                         , F(WO)|F(Special)                      , EF(________), 0 , 0 , kFamilySse , 82 , 6399, 213),
  INST(Pcommit         , X86Op_O            , O(660F00,AE,7,_,_,_,_,_  ), 0                         , F(Volatile)                           , EF(________), 0 , 0 , kFamilyNone, 0  , 1881, 81 ),
  INST(Pdep            , VexRvm_Wx          , V(F20F38,F5,_,0,x,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 1889, 214),
  INST(Pext            , VexRvm_Wx          , V(F30F38,F5,_,0,x,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 1894, 214),
  INST(Pextrb          , ExtExtract         , O(000F3A,14,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 84 , 6804, 215),
  INST(Pextrd          , ExtExtract         , O(000F3A,16,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 84 , 6812, 83 ),
  INST(Pextrq          , ExtExtract         , O(000F3A,16,_,_,1,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 84 , 6820, 216),
  INST(Pextrw          , ExtPextrw          , O(000F00,C5,_,_,_,_,_,_  ), O(000F3A,15,_,_,_,_,_,_  ), F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 85 , 6828, 217),
  INST(Pf2id           , Ext3dNow           , O(000F0F,1D,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 75 , 1899, 218),
  INST(Pf2iw           , Ext3dNow           , O(000F0F,1C,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 86 , 1905, 218),
  INST(Pfacc           , Ext3dNow           , O(000F0F,AE,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 75 , 1911, 208),
  INST(Pfadd           , Ext3dNow           , O(000F0F,9E,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 75 , 1917, 208),
  INST(Pfcmpeq         , Ext3dNow           , O(000F0F,B0,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 75 , 1923, 208),
  INST(Pfcmpge         , Ext3dNow           , O(000F0F,90,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 75 , 1931, 208),
  INST(Pfcmpgt         , Ext3dNow           , O(000F0F,A0,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 75 , 1939, 208),
  INST(Pfmax           , Ext3dNow           , O(000F0F,A4,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 75 , 1947, 208),
  INST(Pfmin           , Ext3dNow           , O(000F0F,94,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 75 , 1953, 208),
  INST(Pfmul           , Ext3dNow           , O(000F0F,B4,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 75 , 1959, 208),
  INST(Pfnacc          , Ext3dNow           , O(000F0F,8A,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 86 , 1965, 208),
  INST(Pfpnacc         , Ext3dNow           , O(000F0F,8E,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 86 , 1972, 208),
  INST(Pfrcp           , Ext3dNow           , O(000F0F,96,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 75 , 1980, 218),
  INST(Pfrcpit1        , Ext3dNow           , O(000F0F,A6,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 75 , 1986, 208),
  INST(Pfrcpit2        , Ext3dNow           , O(000F0F,B6,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 75 , 1995, 208),
  INST(Pfrcpv          , Ext3dNow           , O(000F0F,86,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 87 , 2004, 208),
  INST(Pfrsqit1        , Ext3dNow           , O(000F0F,A7,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 0 , kFamilySse , 75 , 2011, 219),
  INST(Pfrsqrt         , Ext3dNow           , O(000F0F,97,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 0 , kFamilySse , 75 , 2020, 219),
  INST(Pfrsqrtv        , Ext3dNow           , O(000F0F,87,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 87 , 2028, 208),
  INST(Pfsub           , Ext3dNow           , O(000F0F,9A,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 75 , 2037, 208),
  INST(Pfsubr          , Ext3dNow           , O(000F0F,AA,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 75 , 2043, 208),
  INST(Phaddd          , ExtRm_P            , O(000F38,02,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 88 , 6907, 203),
  INST(Phaddsw         , ExtRm_P            , O(000F38,03,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 89 , 6924, 203),
  INST(Phaddw          , ExtRm_P            , O(000F38,01,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 90 , 6993, 203),
  INST(Phminposuw      , ExtRm              , O(660F38,41,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 91 , 7019, 6  ),
  INST(Phsubd          , ExtRm_P            , O(000F38,06,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 92 , 7040, 203),
  INST(Phsubsw         , ExtRm_P            , O(000F38,07,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 93 , 7057, 203),
  INST(Phsubw          , ExtRm_P            , O(000F38,05,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 93 , 7066, 203),
  INST(Pi2fd           , Ext3dNow           , O(000F0F,0D,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 75 , 2050, 218),
  INST(Pi2fw           , Ext3dNow           , O(000F0F,0C,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 86 , 2056, 218),
  INST(Pinsrb          , ExtRmi             , O(660F3A,20,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 94 , 7083, 220),
  INST(Pinsrd          , ExtRmi             , O(660F3A,22,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 94 , 7091, 221),
  INST(Pinsrq          , ExtRmi             , O(660F3A,22,_,_,1,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 94 , 7099, 222),
  INST(Pinsrw          , ExtRmi_P           , O(000F00,C4,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 95 , 7107, 223),
  INST(Pmaddubsw       , ExtRm_P            , O(000F38,04,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 96 , 7277, 203),
  INST(Pmaddwd         , ExtRm_P            , O(000F00,F5,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 97 , 7288, 203),
  INST(Pmaxsb          , ExtRm              , O(660F38,3C,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 98 , 7319, 14 ),
  INST(Pmaxsd          , ExtRm              , O(660F38,3D,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 98 , 7327, 14 ),
  INST(Pmaxsw          , ExtRm_P            , O(000F00,EE,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 99 , 7343, 205),
  INST(Pmaxub          , ExtRm_P            , O(000F00,DE,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 99 , 7351, 205),
  INST(Pmaxud          , ExtRm              , O(660F38,3F,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 100, 7359, 14 ),
  INST(Pmaxuw          , ExtRm              , O(660F38,3E,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 101, 7375, 14 ),
  INST(Pminsb          , ExtRm              , O(660F38,38,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 101, 7383, 14 ),
  INST(Pminsd          , ExtRm              , O(660F38,39,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 101, 7391, 14 ),
  INST(Pminsw          , ExtRm_P            , O(000F00,EA,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 102, 7407, 205),
  INST(Pminub          , ExtRm_P            , O(000F00,DA,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 102, 7415, 205),
  INST(Pminud          , ExtRm              , O(660F38,3B,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 103, 7423, 14 ),
  INST(Pminuw          , ExtRm              , O(660F38,3A,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 104, 7439, 14 ),
  INST(Pmovmskb        , ExtRm_P            , O(000F00,D7,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 105, 7517, 224),
  INST(Pmovsxbd        , ExtRm              , O(660F38,21,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 106, 7614, 225),
  INST(Pmovsxbq        , ExtRm              , O(660F38,22,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 106, 7624, 226),
  INST(Pmovsxbw        , ExtRm              , O(660F38,20,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 106, 7634, 64 ),
  INST(Pmovsxdq        , ExtRm              , O(660F38,25,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 106, 7644, 64 ),
  INST(Pmovsxwd        , ExtRm              , O(660F38,23,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 106, 7654, 64 ),
  INST(Pmovsxwq        , ExtRm              , O(660F38,24,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 106, 7664, 225),
  INST(Pmovzxbd        , ExtRm              , O(660F38,31,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 107, 7751, 225),
  INST(Pmovzxbq        , ExtRm              , O(660F38,32,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 107, 7761, 226),
  INST(Pmovzxbw        , ExtRm              , O(660F38,30,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 107, 7771, 64 ),
  INST(Pmovzxdq        , ExtRm              , O(660F38,35,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 107, 7781, 64 ),
  INST(Pmovzxwd        , ExtRm              , O(660F38,33,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 107, 7791, 64 ),
  INST(Pmovzxwq        , ExtRm              , O(660F38,34,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 107, 7801, 225),
  INST(Pmuldq          , ExtRm              , O(660F38,28,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 108, 7811, 6  ),
  INST(Pmulhrsw        , ExtRm_P            , O(000F38,0B,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 109, 7819, 203),
  INST(Pmulhrw         , Ext3dNow           , O(000F0F,B7,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 75 , 2062, 208),
  INST(Pmulhuw         , ExtRm_P            , O(000F00,E4,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 110, 7829, 203),
  INST(Pmulhw          , ExtRm_P            , O(000F00,E5,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 111, 7838, 203),
  INST(Pmulld          , ExtRm              , O(660F38,40,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 112, 7846, 6  ),
  INST(Pmullw          , ExtRm_P            , O(000F00,D5,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 113, 7862, 203),
  INST(Pmuludq         , ExtRm_P            , O(000F00,F4,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 114, 7885, 203),
  INST(Pop             , X86Pop             , O(000000,8F,0,_,_,_,_,_  ), O(000000,58,_,_,_,_,_,_  ), F(WO)|F(Volatile)|F(Special)          , EF(________), 0 , 0 , kFamilyNone, 0  , 2070, 227),
  INST(Popa            , X86Op              , O(660000,61,_,_,_,_,_,_  ), 0                         , F(Volatile)|F(Special)                , EF(________), 0 , 0 , kFamilyNone, 0  , 2074, 228),
  INST(Popad           , X86Op              , O(000000,61,_,_,_,_,_,_  ), 0                         , F(Volatile)|F(Special)                , EF(________), 0 , 0 , kFamilyNone, 0  , 2079, 228),
  INST(Popcnt          , X86Rm              , O(F30F00,B8,_,_,x,_,_,_  ), 0                         , F(WO)                                 , EF(WWWWWW__), 0 , 0 , kFamilyNone, 0  , 2085, 229),
  INST(Popf            , X86Op              , O(660000,9D,_,_,_,_,_,_  ), 0                         , F(Volatile)|F(Special)                , EF(WWWWWWWW), 0 , 0 , kFamilyNone, 0  , 2092, 230),
  INST(Popfd           , X86Op              , O(000000,9D,_,_,_,_,_,_  ), 0                         , F(Volatile)|F(Special)                , EF(WWWWWWWW), 0 , 0 , kFamilyNone, 0  , 2097, 231),
  INST(Popfq           , X86Op              , O(000000,9D,_,_,_,_,_,_  ), 0                         , F(Volatile)|F(Special)                , EF(WWWWWWWW), 0 , 0 , kFamilyNone, 0  , 2103, 232),
  INST(Por             , ExtRm_P            , O(000F00,EB,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 115, 7912, 205),
  INST(Prefetch        , X86M_Only          , O(000F00,0D,0,_,_,_,_,_  ), 0                         , F(RO)|F(Volatile)                     , EF(________), 0 , 0 , kFamilyNone, 0  , 2109, 40 ),
  INST(Prefetchnta     , X86M_Only          , O(000F00,18,0,_,_,_,_,_  ), 0                         , F(RO)|F(Volatile)                     , EF(________), 0 , 0 , kFamilyNone, 0  , 2118, 40 ),
  INST(Prefetcht0      , X86M_Only          , O(000F00,18,1,_,_,_,_,_  ), 0                         , F(RO)|F(Volatile)                     , EF(________), 0 , 0 , kFamilyNone, 0  , 2130, 40 ),
  INST(Prefetcht1      , X86M_Only          , O(000F00,18,2,_,_,_,_,_  ), 0                         , F(RO)|F(Volatile)                     , EF(________), 0 , 0 , kFamilyNone, 0  , 2141, 40 ),
  INST(Prefetcht2      , X86M_Only          , O(000F00,18,3,_,_,_,_,_  ), 0                         , F(RO)|F(Volatile)                     , EF(________), 0 , 0 , kFamilyNone, 0  , 2152, 40 ),
  INST(Prefetchw       , X86M_Only          , O(000F00,0D,1,_,_,_,_,_  ), 0                         , F(RO)|F(Volatile)                     , EF(UUUUUU__), 0 , 0 , kFamilyNone, 0  , 2163, 233),
  INST(Prefetchwt1     , X86M_Only          , O(000F00,0D,2,_,_,_,_,_  ), 0                         , F(RO)|F(Volatile)                     , EF(UUUUUU__), 0 , 0 , kFamilyNone, 0  , 2173, 233),
  INST(Psadbw          , ExtRm_P            , O(000F00,F6,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 116, 3752, 203),
  INST(Pshufb          , ExtRm_P            , O(000F38,00,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 117, 8136, 202),
  INST(Pshufd          , ExtRmi             , O(660F00,70,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 118, 8144, 234),
  INST(Pshufhw         , ExtRmi             , O(F30F00,70,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 118, 8152, 234),
  INST(Pshuflw         , ExtRmi             , O(F20F00,70,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 118, 8161, 234),
  INST(Pshufw          , ExtRmi_P           , O(000F00,70,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 38 , 2185, 235),
  INST(Psignb          , ExtRm_P            , O(000F38,08,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 119, 8170, 203),
  INST(Psignd          , ExtRm_P            , O(000F38,0A,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 119, 8178, 203),
  INST(Psignw          , ExtRm_P            , O(000F38,09,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 119, 8186, 203),
  INST(Pslld           , ExtRmRi_P          , O(000F00,F2,_,_,_,_,_,_  ), O(000F00,72,6,_,_,_,_,_  ), F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 120, 8194, 236),
  INST(Pslldq          , ExtRmRi            , 0                         , O(660F00,73,7,_,_,_,_,_  ), F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 121, 8201, 237),
  INST(Psllq           , ExtRmRi_P          , O(000F00,F3,_,_,_,_,_,_  ), O(000F00,73,6,_,_,_,_,_  ), F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 120, 8209, 238),
  INST(Psllw           , ExtRmRi_P          , O(000F00,F1,_,_,_,_,_,_  ), O(000F00,71,6,_,_,_,_,_  ), F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 122, 8240, 239),
  INST(Psrad           , ExtRmRi_P          , O(000F00,E2,_,_,_,_,_,_  ), O(000F00,72,4,_,_,_,_,_  ), F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 122, 8247, 240),
  INST(Psraw           , ExtRmRi_P          , O(000F00,E1,_,_,_,_,_,_  ), O(000F00,71,4,_,_,_,_,_  ), F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 123, 8285, 241),
  INST(Psrld           , ExtRmRi_P          , O(000F00,D2,_,_,_,_,_,_  ), O(000F00,72,2,_,_,_,_,_  ), F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 123, 8292, 242),
  INST(Psrldq          , ExtRmRi            , 0                         , O(660F00,73,3,_,_,_,_,_  ), F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 124, 8299, 243),
  INST(Psrlq           , ExtRmRi_P          , O(000F00,D3,_,_,_,_,_,_  ), O(000F00,73,2,_,_,_,_,_  ), F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 123, 8307, 244),
  INST(Psrlw           , ExtRmRi_P          , O(000F00,D1,_,_,_,_,_,_  ), O(000F00,71,2,_,_,_,_,_  ), F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 125, 8338, 245),
  INST(Psubb           , ExtRm_P            , O(000F00,F8,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 125, 8345, 206),
  INST(Psubd           , ExtRm_P            , O(000F00,FA,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 125, 8352, 206),
  INST(Psubq           , ExtRm_P            , O(000F00,FB,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 126, 8359, 206),
  INST(Psubsb          , ExtRm_P            , O(000F00,E8,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 125, 8366, 206),
  INST(Psubsw          , ExtRm_P            , O(000F00,E9,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 125, 8374, 206),
  INST(Psubusb         , ExtRm_P            , O(000F00,D8,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 125, 8382, 206),
  INST(Psubusw         , ExtRm_P            , O(000F00,D9,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 125, 8391, 206),
  INST(Psubw           , ExtRm_P            , O(000F00,F9,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 125, 8400, 206),
  INST(Pswapd          , Ext3dNow           , O(000F0F,BB,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 86 , 2192, 218),
  INST(Ptest           , ExtRm              , O(660F38,17,_,_,_,_,_,_  ), 0                         , F(RO)                                 , EF(WWWWWW__), 0 , 0 , kFamilySse , 127, 8429, 246),
  INST(Punpckhbw       , ExtRm_P            , O(000F00,68,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 128, 8512, 203),
  INST(Punpckhdq       , ExtRm_P            , O(000F00,6A,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 128, 8523, 203),
  INST(Punpckhqdq      , ExtRm              , O(660F00,6D,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 5  , 8534, 6  ),
  INST(Punpckhwd       , ExtRm_P            , O(000F00,69,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 128, 8546, 203),
  INST(Punpcklbw       , ExtRm_P            , O(000F00,60,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 128, 8557, 203),
  INST(Punpckldq       , ExtRm_P            , O(000F00,62,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 128, 8568, 203),
  INST(Punpcklqdq      , ExtRm              , O(660F00,6C,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 5  , 8579, 6  ),
  INST(Punpcklwd       , ExtRm_P            , O(000F00,61,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 128, 8591, 203),
  INST(Push            , X86Push            , O(000000,FF,6,_,_,_,_,_  ), O(000000,50,_,_,_,_,_,_  ), F(RO)|F(Volatile)|F(Special)          , EF(________), 0 , 0 , kFamilyNone, 0  , 2199, 247),
  INST(Pusha           , X86Op              , O(660000,60,_,_,_,_,_,_  ), 0                         , F(Volatile)|F(Special)                , EF(________), 0 , 0 , kFamilyNone, 0  , 2204, 228),
  INST(Pushad          , X86Op              , O(000000,60,_,_,_,_,_,_  ), 0                         , F(Volatile)|F(Special)                , EF(________), 0 , 0 , kFamilyNone, 0  , 2210, 228),
  INST(Pushf           , X86Op              , O(660000,9C,_,_,_,_,_,_  ), 0                         , F(Volatile)|F(Special)                , EF(RRRRRRRR), 0 , 0 , kFamilyNone, 0  , 2217, 248),
  INST(Pushfd          , X86Op              , O(000000,9C,_,_,_,_,_,_  ), 0                         , F(Volatile)|F(Special)                , EF(RRRRRRRR), 0 , 0 , kFamilyNone, 0  , 2223, 249),
  INST(Pushfq          , X86Op              , O(000000,9C,_,_,_,_,_,_  ), 0                         , F(Volatile)|F(Special)                , EF(RRRRRRRR), 0 , 0 , kFamilyNone, 0  , 2230, 250),
  INST(Pxor            , ExtRm_P            , O(000F00,EF,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 129, 8602, 206),
  INST(Rcl             , X86Rot             , O(000000,D0,2,_,x,_,_,_  ), 0                         , F(RW)|F(Special)                      , EF(W____X__), 0 , 0 , kFamilyNone, 0  , 2237, 251),
  INST(Rcpps           , ExtRm              , O(000F00,53,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 130, 8730, 65 ),
  INST(Rcpss           , ExtRm              , O(F30F00,53,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 4 , kFamilySse , 131, 8737, 252),
  INST(Rcr             , X86Rot             , O(000000,D0,3,_,x,_,_,_  ), 0                         , F(RW)|F(Special)                      , EF(W____X__), 0 , 0 , kFamilyNone, 0  , 2241, 251),
  INST(Rdfsbase        , X86M               , O(F30F00,AE,0,_,x,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilyNone, 0  , 2245, 253),
  INST(Rdgsbase        , X86M               , O(F30F00,AE,1,_,x,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilyNone, 0  , 2254, 253),
  INST(Rdmsr           , X86Op              , O(000F00,32,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 2263, 254),
  INST(Rdpmc           , X86Op              , O(000F00,33,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 2269, 254),
  INST(Rdrand          , X86M               , O(000F00,C7,6,_,x,_,_,_  ), 0                         , F(WO)                                 , EF(WWWWWW__), 0 , 8 , kFamilyNone, 0  , 2275, 255),
  INST(Rdseed          , X86M               , O(000F00,C7,7,_,x,_,_,_  ), 0                         , F(WO)                                 , EF(WWWWWW__), 0 , 8 , kFamilyNone, 0  , 2282, 255),
  INST(Rdtsc           , X86Op              , O(000F00,31,_,_,_,_,_,_  ), 0                         , F(WO)|F(Volatile)|F(Special)          , EF(________), 0 , 0 , kFamilyNone, 0  , 2289, 256),
  INST(Rdtscp          , X86Op              , O(000F01,F9,_,_,_,_,_,_  ), 0                         , F(WO)|F(Volatile)|F(Special)          , EF(________), 0 , 0 , kFamilyNone, 0  , 2295, 257),
  INST(Ret             , X86Ret             , O(000000,C2,_,_,_,_,_,_  ), 0                         , F(RW)|F(Volatile)|F(Special)          , EF(________), 0 , 0 , kFamilyNone, 0  , 2705, 258),
  INST(Rol             , X86Rot             , O(000000,D0,0,_,x,_,_,_  ), 0                         , F(RW)|F(Special)                      , EF(W____W__), 0 , 0 , kFamilyNone, 0  , 2302, 259),
  INST(Ror             , X86Rot             , O(000000,D0,1,_,x,_,_,_  ), 0                         , F(RW)|F(Special)                      , EF(W____W__), 0 , 0 , kFamilyNone, 0  , 2306, 259),
  INST(Rorx            , VexRmi_Wx          , V(F20F3A,F0,_,0,x,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 2310, 260),
  INST(Roundpd         , ExtRmi             , O(660F3A,09,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 132, 8832, 234),
  INST(Roundps         , ExtRmi             , O(660F3A,08,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 132, 8841, 234),
  INST(Roundsd         , ExtRmi             , O(660F3A,0B,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 133, 8850, 261),
  INST(Roundss         , ExtRmi             , O(660F3A,0A,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 4 , kFamilySse , 133, 8859, 262),
  INST(Rsqrtps         , ExtRm              , O(000F00,52,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 134, 8956, 65 ),
  INST(Rsqrtss         , ExtRm              , O(F30F00,52,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 4 , kFamilySse , 135, 8965, 252),
  INST(Sahf            , X86Op              , O(000000,9E,_,_,_,_,_,_  ), 0                         , F(RO)|F(Volatile)|F(Special)          , EF(_WWWWW__), 0 , 0 , kFamilyNone, 0  , 2315, 263),
  INST(Sal             , X86Rot             , O(000000,D0,4,_,x,_,_,_  ), 0                         , F(RW)|F(Special)                      , EF(WWWUWW__), 0 , 0 , kFamilyNone, 0  , 2320, 264),
  INST(Sar             , X86Rot             , O(000000,D0,7,_,x,_,_,_  ), 0                         , F(RW)|F(Special)                      , EF(WWWUWW__), 0 , 0 , kFamilyNone, 0  , 2324, 264),
  INST(Sarx            , VexRmv_Wx          , V(F30F38,F7,_,0,x,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 2328, 265),
  INST(Sbb             , X86Arith           , O(000000,18,3,_,x,_,_,_  ), 0                         , F(RW)|F(Lock)                         , EF(WWWWWX__), 0 , 0 , kFamilyNone, 0  , 2333, 3  ),
  INST(Scas            , X86StrRm           , O(000000,AE,_,_,_,_,_,_  ), 0                         , F(RW)|F(Special)|F(Rep)|F(Repnz)      , EF(WWWWWWR_), 0 , 0 , kFamilyNone, 0  , 2337, 266),
  INST(Seta            , X86Set             , O(000F00,97,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(__R__R__), 0 , 1 , kFamilyNone, 0  , 2342, 267),
  INST(Setae           , X86Set             , O(000F00,93,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(_____R__), 0 , 1 , kFamilyNone, 0  , 2347, 268),
  INST(Setb            , X86Set             , O(000F00,92,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(_____R__), 0 , 1 , kFamilyNone, 0  , 2353, 268),
  INST(Setbe           , X86Set             , O(000F00,96,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(__R__R__), 0 , 1 , kFamilyNone, 0  , 2358, 267),
  INST(Setc            , X86Set             , O(000F00,92,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(_____R__), 0 , 1 , kFamilyNone, 0  , 2364, 268),
  INST(Sete            , X86Set             , O(000F00,94,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(__R_____), 0 , 1 , kFamilyNone, 0  , 2369, 269),
  INST(Setg            , X86Set             , O(000F00,9F,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(RRR_____), 0 , 1 , kFamilyNone, 0  , 2374, 270),
  INST(Setge           , X86Set             , O(000F00,9D,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(RR______), 0 , 1 , kFamilyNone, 0  , 2379, 271),
  INST(Setl            , X86Set             , O(000F00,9C,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(RR______), 0 , 1 , kFamilyNone, 0  , 2385, 271),
  INST(Setle           , X86Set             , O(000F00,9E,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(RRR_____), 0 , 1 , kFamilyNone, 0  , 2390, 270),
  INST(Setna           , X86Set             , O(000F00,96,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(__R__R__), 0 , 1 , kFamilyNone, 0  , 2396, 267),
  INST(Setnae          , X86Set             , O(000F00,92,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(_____R__), 0 , 1 , kFamilyNone, 0  , 2402, 268),
  INST(Setnb           , X86Set             , O(000F00,93,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(_____R__), 0 , 1 , kFamilyNone, 0  , 2409, 268),
  INST(Setnbe          , X86Set             , O(000F00,97,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(__R__R__), 0 , 1 , kFamilyNone, 0  , 2415, 267),
  INST(Setnc           , X86Set             , O(000F00,93,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(_____R__), 0 , 1 , kFamilyNone, 0  , 2422, 268),
  INST(Setne           , X86Set             , O(000F00,95,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(__R_____), 0 , 1 , kFamilyNone, 0  , 2428, 269),
  INST(Setng           , X86Set             , O(000F00,9E,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(RRR_____), 0 , 1 , kFamilyNone, 0  , 2434, 270),
  INST(Setnge          , X86Set             , O(000F00,9C,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(RR______), 0 , 1 , kFamilyNone, 0  , 2440, 271),
  INST(Setnl           , X86Set             , O(000F00,9D,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(RR______), 0 , 1 , kFamilyNone, 0  , 2447, 271),
  INST(Setnle          , X86Set             , O(000F00,9F,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(RRR_____), 0 , 1 , kFamilyNone, 0  , 2453, 270),
  INST(Setno           , X86Set             , O(000F00,91,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(R_______), 0 , 1 , kFamilyNone, 0  , 2460, 272),
  INST(Setnp           , X86Set             , O(000F00,9B,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(____R___), 0 , 1 , kFamilyNone, 0  , 2466, 273),
  INST(Setns           , X86Set             , O(000F00,99,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(_R______), 0 , 1 , kFamilyNone, 0  , 2472, 274),
  INST(Setnz           , X86Set             , O(000F00,95,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(__R_____), 0 , 1 , kFamilyNone, 0  , 2478, 269),
  INST(Seto            , X86Set             , O(000F00,90,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(R_______), 0 , 1 , kFamilyNone, 0  , 2484, 272),
  INST(Setp            , X86Set             , O(000F00,9A,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(____R___), 0 , 1 , kFamilyNone, 0  , 2489, 273),
  INST(Setpe           , X86Set             , O(000F00,9A,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(____R___), 0 , 1 , kFamilyNone, 0  , 2494, 273),
  INST(Setpo           , X86Set             , O(000F00,9B,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(____R___), 0 , 1 , kFamilyNone, 0  , 2500, 273),
  INST(Sets            , X86Set             , O(000F00,98,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(_R______), 0 , 1 , kFamilyNone, 0  , 2506, 274),
  INST(Setz            , X86Set             , O(000F00,94,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(__R_____), 0 , 1 , kFamilyNone, 0  , 2511, 269),
  INST(Sfence          , X86Fence           , O(000F00,AE,7,_,_,_,_,_  ), 0                         , F(Volatile)                           , EF(________), 0 , 0 , kFamilyNone, 0  , 2516, 81 ),
  INST(Sgdt            , X86M_Only          , O(000F00,01,0,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 2523, 275),
  INST(Sha1msg1        , ExtRm              , O(000F38,C9,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 136, 2528, 6  ),
  INST(Sha1msg2        , ExtRm              , O(000F38,CA,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 136, 2537, 6  ),
  INST(Sha1nexte       , ExtRm              , O(000F38,C8,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 136, 2546, 6  ),
  INST(Sha1rnds4       , ExtRmi             , O(000F3A,CC,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 136, 2556, 18 ),
  INST(Sha256msg1      , ExtRm              , O(000F38,CC,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 136, 2566, 6  ),
  INST(Sha256msg2      , ExtRm              , O(000F38,CD,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 136, 2577, 6  ),
  INST(Sha256rnds2     , ExtRm_XMM0         , O(000F38,CB,_,_,_,_,_,_  ), 0                         , F(RW)|F(Special)                      , EF(________), 0 , 0 , kFamilySse , 136, 2588, 19 ),
  INST(Shl             , X86Rot             , O(000000,D0,4,_,x,_,_,_  ), 0                         , F(RW)|F(Special)                      , EF(WWWUWW__), 0 , 0 , kFamilyNone, 0  , 2600, 264),
  INST(Shld            , X86ShldShrd        , O(000F00,A4,_,_,x,_,_,_  ), 0                         , F(RW)|F(Special)                      , EF(UWWUWW__), 0 , 0 , kFamilyNone, 0  , 8116, 276),
  INST(Shlx            , VexRmv_Wx          , V(660F38,F7,_,0,x,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 2604, 265),
  INST(Shr             , X86Rot             , O(000000,D0,5,_,x,_,_,_  ), 0                         , F(RW)|F(Special)                      , EF(WWWUWW__), 0 , 0 , kFamilyNone, 0  , 2609, 264),
  INST(Shrd            , X86ShldShrd        , O(000F00,AC,_,_,x,_,_,_  ), 0                         , F(RW)|F(Special)                      , EF(UWWUWW__), 0 , 0 , kFamilyNone, 0  , 2613, 276),
  INST(Shrx            , VexRmv_Wx          , V(F20F38,F7,_,0,x,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 2618, 265),
  INST(Shufpd          , ExtRmi             , O(660F00,C6,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 137, 9226, 18 ),
  INST(Shufps          , ExtRmi             , O(000F00,C6,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 138, 9234, 18 ),
  INST(Sidt            , X86M_Only          , O(000F00,01,1,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 2623, 275),
  INST(Sldt            , X86M               , O(000F00,00,0,_,_,_,_,_  ), 0                         , F(WO)|F(Volatile)                     , EF(________), 0 , 0 , kFamilyNone, 0  , 2628, 277),
  INST(Smsw            , X86M               , O(000F00,01,4,_,_,_,_,_  ), 0                         , F(WO)|F(Volatile)                     , EF(________), 0 , 0 , kFamilyNone, 0  , 2633, 277),
  INST(Sqrtpd          , ExtRm              , O(660F00,51,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 139, 9242, 65 ),
  INST(Sqrtps          , ExtRm              , O(000F00,51,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 140, 8957, 65 ),
  INST(Sqrtsd          , ExtRm              , O(F20F00,51,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 141, 9258, 278),
  INST(Sqrtss          , ExtRm              , O(F30F00,51,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 4 , kFamilySse , 142, 8966, 252),
  INST(Stac            , X86Op              , O(000F01,CB,_,_,_,_,_,_  ), 0                         , F(Volatile)                           , EF(___W____), 0 , 0 , kFamilyNone, 0  , 2638, 37 ),
  INST(Stc             , X86Op              , O(000000,F9,_,_,_,_,_,_  ), 0                         , 0                                     , EF(_____W__), 0 , 0 , kFamilyNone, 0  , 2643, 279),
  INST(Std             , X86Op              , O(000000,FD,_,_,_,_,_,_  ), 0                         , 0                                     , EF(______W_), 0 , 0 , kFamilyNone, 0  , 6199, 280),
  INST(Sti             , X86Op              , O(000000,FB,_,_,_,_,_,_  ), 0                         , 0                                     , EF(_______W), 0 , 0 , kFamilyNone, 0  , 2647, 281),
  INST(Stmxcsr         , X86M_Only          , O(000F00,AE,3,_,_,_,_,_  ), 0                         , F(Volatile)                           , EF(________), 0 , 0 , kFamilyNone, 0  , 9274, 282),
  INST(Stos            , X86StrMr           , O(000000,AA,_,_,_,_,_,_  ), 0                         , F(RW)|F(Special)|F(Rep)               , EF(______R_), 0 , 0 , kFamilyNone, 0  , 2651, 283),
  INST(Str             , X86M               , O(000F00,00,1,_,_,_,_,_  ), 0                         , F(WO)|F(Volatile)                     , EF(________), 0 , 0 , kFamilyNone, 0  , 2656, 277),
  INST(Sub             , X86Arith           , O(000000,28,5,_,x,_,_,_  ), 0                         , F(RW)|F(Lock)                         , EF(WWWWWW__), 0 , 0 , kFamilyNone, 0  , 807 , 284),
  INST(Subpd           , ExtRm              , O(660F00,5C,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 143, 4318, 6  ),
  INST(Subps           , ExtRm              , O(000F00,5C,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 144, 4330, 6  ),
  INST(Subsd           , ExtRm              , O(F20F00,5C,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 143, 5006, 7  ),
  INST(Subss           , ExtRm              , O(F30F00,5C,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 144, 5016, 8  ),
  INST(Swapgs          , X86Op              , O(000F01,F8,_,_,_,_,_,_  ), 0                         , 0                                     , EF(________), 0 , 0 , kFamilyNone, 0  , 2660, 285),
  INST(Syscall         , X86Op              , O(000F00,05,_,_,_,_,_,_  ), 0                         , 0                                     , EF(________), 0 , 0 , kFamilyNone, 0  , 2667, 285),
  INST(Sysenter        , X86Op              , O(000F00,34,_,_,_,_,_,_  ), 0                         , 0                                     , EF(________), 0 , 0 , kFamilyNone, 0  , 2675, 41 ),
  INST(Sysexit         , X86Op              , O(000F00,35,_,_,_,_,_,_  ), 0                         , 0                                     , EF(________), 0 , 0 , kFamilyNone, 0  , 2684, 41 ),
  INST(Sysexit64       , X86Op              , O(000F00,35,_,_,_,_,_,_  ), 0                         , 0                                     , EF(________), 0 , 0 , kFamilyNone, 0  , 2692, 41 ),
  INST(Sysret          , X86Op              , O(000F00,07,_,_,_,_,_,_  ), 0                         , 0                                     , EF(________), 0 , 0 , kFamilyNone, 0  , 2702, 285),
  INST(Sysret64        , X86Op              , O(000F00,07,_,_,_,_,_,_  ), 0                         , 0                                     , EF(________), 0 , 0 , kFamilyNone, 0  , 2709, 285),
  INST(T1mskc          , VexVm_Wx           , V(XOP_M9,01,7,0,x,_,_,_  ), 0                         , F(WO)                                 , EF(WWWUUW__), 0 , 0 , kFamilyNone, 0  , 2718, 17 ),
  INST(Test            , X86Test            , O(000000,84,_,_,x,_,_,_  ), O(000000,F6,_,_,x,_,_,_  ), F(RO)                                 , EF(WWWUWW__), 0 , 0 , kFamilyNone, 0  , 8430, 286),
  INST(Tzcnt           , X86Rm              , O(F30F00,BC,_,_,x,_,_,_  ), 0                         , F(WO)                                 , EF(UUWUUW__), 0 , 0 , kFamilyNone, 0  , 2725, 229),
  INST(Tzmsk           , VexVm_Wx           , V(XOP_M9,01,4,0,x,_,_,_  ), 0                         , F(WO)                                 , EF(WWWUUW__), 0 , 0 , kFamilyNone, 0  , 2731, 17 ),
  INST(Ucomisd         , ExtRm              , O(660F00,2E,_,_,_,_,_,_  ), 0                         , F(RO)                                 , EF(WWWWWW__), 0 , 0 , kFamilySse , 145, 9327, 59 ),
  INST(Ucomiss         , ExtRm              , O(000F00,2E,_,_,_,_,_,_  ), 0                         , F(RO)                                 , EF(WWWWWW__), 0 , 0 , kFamilySse , 146, 9336, 60 ),
  INST(Ud2             , X86Op              , O(000F00,0B,_,_,_,_,_,_  ), 0                         , 0                                     , EF(________), 0 , 0 , kFamilyNone, 0  , 2737, 41 ),
  INST(Unpckhpd        , ExtRm              , O(660F00,15,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 147, 9345, 6  ),
  INST(Unpckhps        , ExtRm              , O(000F00,15,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 148, 9355, 6  ),
  INST(Unpcklpd        , ExtRm              , O(660F00,14,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 147, 9365, 6  ),
  INST(Unpcklps        , ExtRm              , O(000F00,14,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 148, 9375, 6  ),
  INST(V4fmaddps       , VexRm_T1_4X        , V(F20F38,9A,_,2,_,0,2,T1X), 0                         , F(RW)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 0  , 2741, 287),
  INST(V4fnmaddps      , VexRm_T1_4X        , V(F20F38,AA,_,2,_,0,2,T1X), 0                         , F(RW)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 0  , 2751, 287),
  INST(Vaddpd          , VexRvm_Lx          , V(660F00,58,_,x,I,1,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 1  , 2762, 288),
  INST(Vaddps          , VexRvm_Lx          , V(000F00,58,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 2  , 2769, 288),
  INST(Vaddsd          , VexRvm             , V(F20F00,58,_,I,I,1,3,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 3  , 2776, 289),
  INST(Vaddss          , VexRvm             , V(F30F00,58,_,I,I,0,2,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 3  , 2783, 290),
  INST(Vaddsubpd       , VexRvm_Lx          , V(660F00,D0,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 4  , 2790, 291),
  INST(Vaddsubps       , VexRvm_Lx          , V(F20F00,D0,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 4  , 2800, 291),
  INST(Vaesdec         , VexRvm             , V(660F38,DE,_,0,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 5  , 2810, 292),
  INST(Vaesdeclast     , VexRvm             , V(660F38,DF,_,0,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 5  , 2818, 292),
  INST(Vaesenc         , VexRvm             , V(660F38,DC,_,0,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 5  , 2830, 292),
  INST(Vaesenclast     , VexRvm             , V(660F38,DD,_,0,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 5  , 2838, 292),
  INST(Vaesimc         , VexRm              , V(660F38,DB,_,0,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 5  , 2850, 293),
  INST(Vaeskeygenassist, VexRmi             , V(660F3A,DF,_,0,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 5  , 2858, 294),
  INST(Valignd         , VexRvmi_Lx         , V(660F3A,03,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 6  , 2875, 295),
  INST(Valignq         , VexRvmi_Lx         , V(660F3A,03,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 7  , 2883, 295),
  INST(Vandnpd         , VexRvm_Lx          , V(660F00,55,_,x,I,1,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 8  , 2891, 288),
  INST(Vandnps         , VexRvm_Lx          , V(000F00,55,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 9  , 2899, 288),
  INST(Vandpd          , VexRvm_Lx          , V(660F00,54,_,x,I,1,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 8  , 2907, 296),
  INST(Vandps          , VexRvm_Lx          , V(000F00,54,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 9  , 2914, 296),
  INST(Vblendmb        , VexRvm_Lx          , V(660F38,66,_,x,_,0,4,FVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 10 , 2921, 297),
  INST(Vblendmd        , VexRvm_Lx          , V(660F38,64,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 6  , 2930, 297),
  INST(Vblendmpd       , VexRvm_Lx          , V(660F38,65,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 7  , 2939, 297),
  INST(Vblendmps       , VexRvm_Lx          , V(660F38,65,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 6  , 2949, 297),
  INST(Vblendmq        , VexRvm_Lx          , V(660F38,64,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 7  , 2959, 297),
  INST(Vblendmw        , VexRvm_Lx          , V(660F38,66,_,x,_,1,4,FVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 10 , 2968, 297),
  INST(Vblendpd        , VexRvmi_Lx         , V(660F3A,0D,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 4  , 2977, 298),
  INST(Vblendps        , VexRvmi_Lx         , V(660F3A,0C,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 4  , 2986, 298),
  INST(Vblendvpd       , VexRvmr_Lx         , V(660F3A,4B,_,x,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 4  , 2995, 299),
  INST(Vblendvps       , VexRvmr_Lx         , V(660F3A,4A,_,x,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 4  , 3005, 299),
  INST(Vbroadcastf128  , VexRm              , V(660F38,1A,_,1,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 4  , 3015, 300),
  INST(Vbroadcastf32x2 , VexRm_Lx           , V(660F38,19,_,x,_,0,3,T2 ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 11 , 3030, 301),
  INST(Vbroadcastf32x4 , VexRm_Lx           , V(660F38,1A,_,x,_,0,4,T4 ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 12 , 3046, 302),
  INST(Vbroadcastf32x8 , VexRm              , V(660F38,1B,_,2,_,0,5,T8 ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 13 , 3062, 303),
  INST(Vbroadcastf64x2 , VexRm_Lx           , V(660F38,1A,_,x,_,1,4,T2 ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 11 , 3078, 302),
  INST(Vbroadcastf64x4 , VexRm              , V(660F38,1B,_,2,_,1,5,T4 ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 12 , 3094, 303),
  INST(Vbroadcasti128  , VexRm              , V(660F38,5A,_,1,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 14 , 3110, 300),
  INST(Vbroadcasti32x2 , VexRm_Lx           , V(660F38,59,_,x,_,0,3,T2 ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 11 , 3125, 304),
  INST(Vbroadcasti32x4 , VexRm_Lx           , V(660F38,5A,_,x,_,0,4,T4 ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 15 , 3141, 301),
  INST(Vbroadcasti32x8 , VexRm              , V(660F38,5B,_,2,_,0,5,T8 ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 13 , 3157, 305),
  INST(Vbroadcasti64x2 , VexRm_Lx           , V(660F38,5A,_,x,_,1,4,T2 ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 11 , 3173, 301),
  INST(Vbroadcasti64x4 , VexRm              , V(660F38,5B,_,2,_,1,5,T4 ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 12 , 3189, 305),
  INST(Vbroadcastsd    , VexRm_Lx           , V(660F38,19,_,x,0,1,3,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 16 , 3205, 306),
  INST(Vbroadcastss    , VexRm_Lx           , V(660F38,18,_,x,0,0,2,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 16 , 3218, 307),
  INST(Vcmppd          , VexRvmi_Lx         , V(660F00,C2,_,x,I,1,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 17 , 3231, 308),
  INST(Vcmpps          , VexRvmi_Lx         , V(000F00,C2,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 18 , 3238, 308),
  INST(Vcmpsd          , VexRvmi            , V(F20F00,C2,_,I,I,1,3,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 19 , 3245, 309),
  INST(Vcmpss          , VexRvmi            , V(F30F00,C2,_,I,I,0,2,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 19 , 3252, 310),
  INST(Vcomisd         , VexRm              , V(660F00,2F,_,I,I,1,3,T1S), 0                         , F(RO)|F(Vex)|F(Evex)                  , EF(WWWWWW__), 0 , 0 , kFamilyAvx , 20 , 3259, 311),
  INST(Vcomiss         , VexRm              , V(000F00,2F,_,I,I,0,2,T1S), 0                         , F(RO)|F(Vex)|F(Evex)                  , EF(WWWWWW__), 0 , 0 , kFamilyAvx , 20 , 3267, 312),
  INST(Vcompresspd     , VexMr_Lx           , V(660F38,8A,_,x,_,1,3,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 15 , 3275, 313),
  INST(Vcompressps     , VexMr_Lx           , V(660F38,8A,_,x,_,0,2,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 15 , 3287, 313),
  INST(Vcvtdq2pd       , VexRm_Lx           , V(F30F00,E6,_,x,I,0,3,HV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 21 , 3299, 314),
  INST(Vcvtdq2ps       , VexRm_Lx           , V(000F00,5B,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 2  , 3309, 315),
  INST(Vcvtpd2dq       , VexRm_Lx           , V(F20F00,E6,_,x,I,1,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 1  , 3319, 316),
  INST(Vcvtpd2ps       , VexRm_Lx           , V(660F00,5A,_,x,I,1,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 1  , 3329, 317),
  INST(Vcvtpd2qq       , VexRm_Lx           , V(660F00,7B,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 22 , 3339, 318),
  INST(Vcvtpd2udq      , VexRm_Lx           , V(000F00,79,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 23 , 3349, 319),
  INST(Vcvtpd2uqq      , VexRm_Lx           , V(660F00,79,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 22 , 3360, 318),
  INST(Vcvtph2ps       , VexRm_Lx           , V(660F38,13,_,x,0,0,3,HVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 24 , 3371, 314),
  INST(Vcvtps2dq       , VexRm_Lx           , V(660F00,5B,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 2  , 3381, 315),
  INST(Vcvtps2pd       , VexRm_Lx           , V(000F00,5A,_,x,I,0,4,HV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 2  , 3391, 314),
  INST(Vcvtps2ph       , VexMri_Lx          , V(660F3A,1D,_,x,0,0,3,HVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 24 , 3401, 320),
  INST(Vcvtps2qq       , VexRm_Lx           , V(660F00,7B,_,x,_,0,3,HV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 25 , 3411, 321),
  INST(Vcvtps2udq      , VexRm_Lx           , V(000F00,79,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 26 , 3421, 318),
  INST(Vcvtps2uqq      , VexRm_Lx           , V(660F00,79,_,x,_,0,3,HV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 25 , 3432, 321),
  INST(Vcvtqq2pd       , VexRm_Lx           , V(F30F00,E6,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 22 , 3443, 318),
  INST(Vcvtqq2ps       , VexRm_Lx           , V(000F00,5B,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 22 , 3453, 319),
  INST(Vcvtsd2si       , VexRm              , V(F20F00,2D,_,I,x,x,3,T1F), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 27 , 3463, 322),
  INST(Vcvtsd2ss       , VexRvm             , V(F20F00,5A,_,I,I,1,3,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 3  , 3473, 289),
  INST(Vcvtsd2usi      , VexRm              , V(F20F00,79,_,I,_,x,3,T1F), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 28 , 3483, 323),
  INST(Vcvtsi2sd       , VexRvm             , V(F20F00,2A,_,I,x,x,2,T1W), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 27 , 3494, 324),
  INST(Vcvtsi2ss       , VexRvm             , V(F30F00,2A,_,I,x,x,2,T1W), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 27 , 3504, 324),
  INST(Vcvtss2sd       , VexRvm             , V(F30F00,5A,_,I,I,0,2,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 19 , 3514, 290),
  INST(Vcvtss2si       , VexRm              , V(F20F00,2D,_,I,x,x,2,T1F), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 27 , 3524, 325),
  INST(Vcvtss2usi      , VexRm              , V(F30F00,79,_,I,_,x,2,T1F), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 28 , 3534, 326),
  INST(Vcvttpd2dq      , VexRm_Lx           , V(660F00,E6,_,x,I,1,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 17 , 3545, 316),
  INST(Vcvttpd2qq      , VexRm_Lx           , V(660F00,7A,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 29 , 3556, 318),
  INST(Vcvttpd2udq     , VexRm_Lx           , V(000F00,78,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 29 , 3567, 319),
  INST(Vcvttpd2uqq     , VexRm_Lx           , V(660F00,78,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 30 , 3579, 318),
  INST(Vcvttps2dq      , VexRm_Lx           , V(F30F00,5B,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 18 , 3591, 315),
  INST(Vcvttps2qq      , VexRm_Lx           , V(660F00,7A,_,x,_,0,3,HV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 31 , 3602, 321),
  INST(Vcvttps2udq     , VexRm_Lx           , V(000F00,78,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 32 , 3613, 318),
  INST(Vcvttps2uqq     , VexRm_Lx           , V(660F00,78,_,x,_,0,3,HV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 31 , 3625, 321),
  INST(Vcvttsd2si      , VexRm              , V(F20F00,2C,_,I,x,x,3,T1F), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 20 , 3637, 322),
  INST(Vcvttsd2usi     , VexRm              , V(F20F00,78,_,I,_,x,3,T1F), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 33 , 3648, 323),
  INST(Vcvttss2si      , VexRm              , V(F30F00,2C,_,I,x,x,2,T1F), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 20 , 3660, 325),
  INST(Vcvttss2usi     , VexRm              , V(F30F00,78,_,I,_,x,2,T1F), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 33 , 3671, 326),
  INST(Vcvtudq2pd      , VexRm_Lx           , V(F30F00,7A,_,x,_,0,3,HV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 6  , 3683, 321),
  INST(Vcvtudq2ps      , VexRm_Lx           , V(F20F00,7A,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 26 , 3694, 318),
  INST(Vcvtuqq2pd      , VexRm_Lx           , V(F30F00,7A,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 22 , 3705, 318),
  INST(Vcvtuqq2ps      , VexRm_Lx           , V(F20F00,7A,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 22 , 3716, 319),
  INST(Vcvtusi2sd      , VexRvm             , V(F20F00,7B,_,I,_,x,2,T1W), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 28 , 3727, 327),
  INST(Vcvtusi2ss      , VexRvm             , V(F30F00,7B,_,I,_,x,2,T1W), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 28 , 3738, 327),
  INST(Vdbpsadbw       , VexRvmi_Lx         , V(660F3A,42,_,x,_,0,4,FVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 10 , 3749, 295),
  INST(Vdivpd          , VexRvm_Lx          , V(660F00,5E,_,x,I,1,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 1  , 3759, 288),
  INST(Vdivps          , VexRvm_Lx          , V(000F00,5E,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 2  , 3766, 288),
  INST(Vdivsd          , VexRvm             , V(F20F00,5E,_,I,I,1,3,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 3  , 3773, 289),
  INST(Vdivss          , VexRvm             , V(F30F00,5E,_,I,I,0,2,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 3  , 3780, 290),
  INST(Vdppd           , VexRvmi_Lx         , V(660F3A,41,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 4  , 3787, 298),
  INST(Vdpps           , VexRvmi_Lx         , V(660F3A,40,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 4  , 3793, 298),
  INST(Verr            , X86M               , O(000F00,00,4,_,_,_,_,_  ), 0                         , F(RO)                                 , EF(__W_____), 0 , 0 , kFamilyNone, 0  , 3799, 328),
  INST(Verw            , X86M               , O(000F00,00,5,_,_,_,_,_  ), 0                         , F(RO)                                 , EF(__W_____), 0 , 0 , kFamilyNone, 0  , 3804, 328),
  INST(Vexp2pd         , VexRm              , V(660F38,C8,_,2,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 34 , 3809, 329),
  INST(Vexp2ps         , VexRm              , V(660F38,C8,_,2,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 35 , 3817, 329),
  INST(Vexpandpd       , VexRm_Lx           , V(660F38,88,_,x,_,1,3,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 15 , 3825, 318),
  INST(Vexpandps       , VexRm_Lx           , V(660F38,88,_,x,_,0,2,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 15 , 3835, 318),
  INST(Vextractf128    , VexMri             , V(660F3A,19,_,1,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 4  , 3845, 330),
  INST(Vextractf32x4   , VexMri_Lx          , V(660F3A,19,_,x,_,0,4,T4 ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 15 , 3858, 331),
  INST(Vextractf32x8   , VexMri             , V(660F3A,1B,_,2,_,0,5,T8 ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 13 , 3872, 332),
  INST(Vextractf64x2   , VexMri_Lx          , V(660F3A,19,_,x,_,1,4,T2 ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 11 , 3886, 331),
  INST(Vextractf64x4   , VexMri             , V(660F3A,1B,_,2,_,1,5,T4 ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 12 , 3900, 332),
  INST(Vextracti128    , VexMri             , V(660F3A,39,_,1,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 14 , 3914, 330),
  INST(Vextracti32x4   , VexMri_Lx          , V(660F3A,39,_,x,_,0,4,T4 ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 15 , 3927, 331),
  INST(Vextracti32x8   , VexMri             , V(660F3A,3B,_,2,_,0,5,T8 ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 13 , 3941, 332),
  INST(Vextracti64x2   , VexMri_Lx          , V(660F3A,39,_,x,_,1,4,T2 ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 11 , 3955, 331),
  INST(Vextracti64x4   , VexMri             , V(660F3A,3B,_,2,_,1,5,T4 ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 12 , 3969, 332),
  INST(Vextractps      , VexMri             , V(660F3A,17,_,0,I,I,2,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 36 , 3983, 333),
  INST(Vfixupimmpd     , VexRvmi_Lx         , V(660F3A,54,_,x,_,1,4,FV ), 0                         , F(RW)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 29 , 3994, 334),
  INST(Vfixupimmps     , VexRvmi_Lx         , V(660F3A,54,_,x,_,0,4,FV ), 0                         , F(RW)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 32 , 4006, 334),
  INST(Vfixupimmsd     , VexRvmi            , V(660F3A,55,_,I,_,1,3,T1S), 0                         , F(RW)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 37 , 4018, 335),
  INST(Vfixupimmss     , VexRvmi            , V(660F3A,55,_,I,_,0,2,T1S), 0                         , F(RW)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 37 , 4030, 336),
  INST(Vfmadd132pd     , VexRvm_Lx          , V(660F38,98,_,x,1,1,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 38 , 4042, 337),
  INST(Vfmadd132ps     , VexRvm_Lx          , V(660F38,98,_,x,0,0,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 39 , 4054, 337),
  INST(Vfmadd132sd     , VexRvm             , V(660F38,99,_,I,1,1,3,T1S), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 40 , 4066, 338),
  INST(Vfmadd132ss     , VexRvm             , V(660F38,99,_,I,0,0,2,T1S), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 40 , 4078, 339),
  INST(Vfmadd213pd     , VexRvm_Lx          , V(660F38,A8,_,x,1,1,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 38 , 4090, 337),
  INST(Vfmadd213ps     , VexRvm_Lx          , V(660F38,A8,_,x,0,0,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 39 , 4102, 337),
  INST(Vfmadd213sd     , VexRvm             , V(660F38,A9,_,I,1,1,3,T1S), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 40 , 4114, 338),
  INST(Vfmadd213ss     , VexRvm             , V(660F38,A9,_,I,0,0,2,T1S), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 40 , 4126, 339),
  INST(Vfmadd231pd     , VexRvm_Lx          , V(660F38,B8,_,x,1,1,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 38 , 4138, 337),
  INST(Vfmadd231ps     , VexRvm_Lx          , V(660F38,B8,_,x,0,0,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 39 , 4150, 337),
  INST(Vfmadd231sd     , VexRvm             , V(660F38,B9,_,I,1,1,3,T1S), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 40 , 4162, 338),
  INST(Vfmadd231ss     , VexRvm             , V(660F38,B9,_,I,0,0,2,T1S), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 40 , 4174, 339),
  INST(Vfmaddpd        , Fma4_Lx            , V(660F3A,69,_,x,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 41 , 4186, 340),
  INST(Vfmaddps        , Fma4_Lx            , V(660F3A,68,_,x,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 41 , 4195, 340),
  INST(Vfmaddsd        , Fma4               , V(660F3A,6B,_,0,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 41 , 4204, 341),
  INST(Vfmaddss        , Fma4               , V(660F3A,6A,_,0,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 41 , 4213, 342),
  INST(Vfmaddsub132pd  , VexRvm_Lx          , V(660F38,96,_,x,1,1,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 38 , 4222, 337),
  INST(Vfmaddsub132ps  , VexRvm_Lx          , V(660F38,96,_,x,0,0,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 39 , 4237, 337),
  INST(Vfmaddsub213pd  , VexRvm_Lx          , V(660F38,A6,_,x,1,1,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 38 , 4252, 337),
  INST(Vfmaddsub213ps  , VexRvm_Lx          , V(660F38,A6,_,x,0,0,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 39 , 4267, 337),
  INST(Vfmaddsub231pd  , VexRvm_Lx          , V(660F38,B6,_,x,1,1,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 38 , 4282, 337),
  INST(Vfmaddsub231ps  , VexRvm_Lx          , V(660F38,B6,_,x,0,0,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 39 , 4297, 337),
  INST(Vfmaddsubpd     , Fma4_Lx            , V(660F3A,5D,_,x,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 41 , 4312, 340),
  INST(Vfmaddsubps     , Fma4_Lx            , V(660F3A,5C,_,x,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 41 , 4324, 340),
  INST(Vfmsub132pd     , VexRvm_Lx          , V(660F38,9A,_,x,1,1,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 38 , 4336, 337),
  INST(Vfmsub132ps     , VexRvm_Lx          , V(660F38,9A,_,x,0,0,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 39 , 4348, 337),
  INST(Vfmsub132sd     , VexRvm             , V(660F38,9B,_,I,1,1,3,T1S), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 40 , 4360, 338),
  INST(Vfmsub132ss     , VexRvm             , V(660F38,9B,_,I,0,0,2,T1S), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 40 , 4372, 339),
  INST(Vfmsub213pd     , VexRvm_Lx          , V(660F38,AA,_,x,1,1,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 38 , 4384, 337),
  INST(Vfmsub213ps     , VexRvm_Lx          , V(660F38,AA,_,x,0,0,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 39 , 4396, 337),
  INST(Vfmsub213sd     , VexRvm             , V(660F38,AB,_,I,1,1,3,T1S), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 40 , 4408, 338),
  INST(Vfmsub213ss     , VexRvm             , V(660F38,AB,_,I,0,0,2,T1S), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 40 , 4420, 339),
  INST(Vfmsub231pd     , VexRvm_Lx          , V(660F38,BA,_,x,1,1,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 38 , 4432, 337),
  INST(Vfmsub231ps     , VexRvm_Lx          , V(660F38,BA,_,x,0,0,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 39 , 4444, 337),
  INST(Vfmsub231sd     , VexRvm             , V(660F38,BB,_,I,1,1,3,T1S), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 40 , 4456, 338),
  INST(Vfmsub231ss     , VexRvm             , V(660F38,BB,_,I,0,0,2,T1S), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 40 , 4468, 339),
  INST(Vfmsubadd132pd  , VexRvm_Lx          , V(660F38,97,_,x,1,1,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 38 , 4480, 337),
  INST(Vfmsubadd132ps  , VexRvm_Lx          , V(660F38,97,_,x,0,0,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 39 , 4495, 337),
  INST(Vfmsubadd213pd  , VexRvm_Lx          , V(660F38,A7,_,x,1,1,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 38 , 4510, 337),
  INST(Vfmsubadd213ps  , VexRvm_Lx          , V(660F38,A7,_,x,0,0,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 39 , 4525, 337),
  INST(Vfmsubadd231pd  , VexRvm_Lx          , V(660F38,B7,_,x,1,1,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 38 , 4540, 337),
  INST(Vfmsubadd231ps  , VexRvm_Lx          , V(660F38,B7,_,x,0,0,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 39 , 4555, 337),
  INST(Vfmsubaddpd     , Fma4_Lx            , V(660F3A,5F,_,x,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 41 , 4570, 340),
  INST(Vfmsubaddps     , Fma4_Lx            , V(660F3A,5E,_,x,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 41 , 4582, 340),
  INST(Vfmsubpd        , Fma4_Lx            , V(660F3A,6D,_,x,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 41 , 4594, 340),
  INST(Vfmsubps        , Fma4_Lx            , V(660F3A,6C,_,x,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 41 , 4603, 340),
  INST(Vfmsubsd        , Fma4               , V(660F3A,6F,_,0,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 41 , 4612, 341),
  INST(Vfmsubss        , Fma4               , V(660F3A,6E,_,0,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 41 , 4621, 342),
  INST(Vfnmadd132pd    , VexRvm_Lx          , V(660F38,9C,_,x,1,1,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 38 , 4630, 337),
  INST(Vfnmadd132ps    , VexRvm_Lx          , V(660F38,9C,_,x,0,0,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 39 , 4643, 337),
  INST(Vfnmadd132sd    , VexRvm             , V(660F38,9D,_,I,1,1,3,T1S), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 40 , 4656, 338),
  INST(Vfnmadd132ss    , VexRvm             , V(660F38,9D,_,I,0,0,2,T1S), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 40 , 4669, 339),
  INST(Vfnmadd213pd    , VexRvm_Lx          , V(660F38,AC,_,x,1,1,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 38 , 4682, 337),
  INST(Vfnmadd213ps    , VexRvm_Lx          , V(660F38,AC,_,x,0,0,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 39 , 4695, 337),
  INST(Vfnmadd213sd    , VexRvm             , V(660F38,AD,_,I,1,1,3,T1S), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 40 , 4708, 338),
  INST(Vfnmadd213ss    , VexRvm             , V(660F38,AD,_,I,0,0,2,T1S), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 40 , 4721, 339),
  INST(Vfnmadd231pd    , VexRvm_Lx          , V(660F38,BC,_,x,1,1,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 38 , 4734, 337),
  INST(Vfnmadd231ps    , VexRvm_Lx          , V(660F38,BC,_,x,0,0,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 39 , 4747, 337),
  INST(Vfnmadd231sd    , VexRvm             , V(660F38,BC,_,I,1,1,3,T1S), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 40 , 4760, 338),
  INST(Vfnmadd231ss    , VexRvm             , V(660F38,BC,_,I,0,0,2,T1S), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 40 , 4773, 339),
  INST(Vfnmaddpd       , Fma4_Lx            , V(660F3A,79,_,x,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 41 , 4786, 340),
  INST(Vfnmaddps       , Fma4_Lx            , V(660F3A,78,_,x,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 41 , 4796, 340),
  INST(Vfnmaddsd       , Fma4               , V(660F3A,7B,_,0,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 41 , 4806, 341),
  INST(Vfnmaddss       , Fma4               , V(660F3A,7A,_,0,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 41 , 4816, 342),
  INST(Vfnmsub132pd    , VexRvm_Lx          , V(660F38,9E,_,x,1,1,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 38 , 4826, 337),
  INST(Vfnmsub132ps    , VexRvm_Lx          , V(660F38,9E,_,x,0,0,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 39 , 4839, 337),
  INST(Vfnmsub132sd    , VexRvm             , V(660F38,9F,_,I,1,1,3,T1S), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 40 , 4852, 338),
  INST(Vfnmsub132ss    , VexRvm             , V(660F38,9F,_,I,0,0,2,T1S), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 40 , 4865, 339),
  INST(Vfnmsub213pd    , VexRvm_Lx          , V(660F38,AE,_,x,1,1,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 38 , 4878, 337),
  INST(Vfnmsub213ps    , VexRvm_Lx          , V(660F38,AE,_,x,0,0,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 39 , 4891, 337),
  INST(Vfnmsub213sd    , VexRvm             , V(660F38,AF,_,I,1,1,3,T1S), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 40 , 4904, 338),
  INST(Vfnmsub213ss    , VexRvm             , V(660F38,AF,_,I,0,0,2,T1S), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 40 , 4917, 339),
  INST(Vfnmsub231pd    , VexRvm_Lx          , V(660F38,BE,_,x,1,1,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 38 , 4930, 337),
  INST(Vfnmsub231ps    , VexRvm_Lx          , V(660F38,BE,_,x,0,0,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 39 , 4943, 337),
  INST(Vfnmsub231sd    , VexRvm             , V(660F38,BF,_,I,1,1,3,T1S), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 40 , 4956, 338),
  INST(Vfnmsub231ss    , VexRvm             , V(660F38,BF,_,I,0,0,2,T1S), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 40 , 4969, 339),
  INST(Vfnmsubpd       , Fma4_Lx            , V(660F3A,7D,_,x,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 41 , 4982, 340),
  INST(Vfnmsubps       , Fma4_Lx            , V(660F3A,7C,_,x,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 41 , 4992, 340),
  INST(Vfnmsubsd       , Fma4               , V(660F3A,7F,_,0,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 41 , 5002, 341),
  INST(Vfnmsubss       , Fma4               , V(660F3A,7E,_,0,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 41 , 5012, 342),
  INST(Vfpclasspd      , VexRmi_Lx          , V(660F3A,66,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 42 , 5022, 343),
  INST(Vfpclassps      , VexRmi_Lx          , V(660F3A,66,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 43 , 5033, 343),
  INST(Vfpclasssd      , VexRmi_Lx          , V(660F3A,67,_,I,_,1,3,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 44 , 5044, 344),
  INST(Vfpclassss      , VexRmi_Lx          , V(660F3A,67,_,I,_,0,2,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 44 , 5055, 345),
  INST(Vfrczpd         , VexRm_Lx           , V(XOP_M9,81,_,x,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 45 , 5066, 346),
  INST(Vfrczps         , VexRm_Lx           , V(XOP_M9,80,_,x,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 45 , 5074, 346),
  INST(Vfrczsd         , VexRm              , V(XOP_M9,83,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 45 , 5082, 347),
  INST(Vfrczss         , VexRm              , V(XOP_M9,82,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 45 , 5090, 348),
  INST(Vgatherdpd      , VexRmvRm_VM        , V(660F38,92,_,x,1,_,_,_  ), V(660F38,92,_,x,_,1,3,T1S), F(RW)|F(Vex_VM)|F(Evex)               , EF(________), 0 , 0 , kFamilyAvx , 46 , 5098, 349),
  INST(Vgatherdps      , VexRmvRm_VM        , V(660F38,92,_,x,0,_,_,_  ), V(660F38,92,_,x,_,0,2,T1S), F(RW)|F(Vex_VM)|F(Evex)               , EF(________), 0 , 0 , kFamilyAvx , 46 , 5109, 350),
  INST(Vgatherpf0dpd   , VexM_VM            , V(660F38,C6,1,2,_,1,3,T1S), 0                         , F(RO)|F(VM)|F(Evex)                   , EF(________), 0 , 0 , kFamilyNone, 0  , 5120, 351),
  INST(Vgatherpf0dps   , VexM_VM            , V(660F38,C6,1,2,_,0,2,T1S), 0                         , F(RO)|F(VM)|F(Evex)                   , EF(________), 0 , 0 , kFamilyNone, 0  , 5134, 352),
  INST(Vgatherpf0qpd   , VexM_VM            , V(660F38,C7,1,2,_,1,3,T1S), 0                         , F(RO)|F(VM)|F(Evex)                   , EF(________), 0 , 0 , kFamilyNone, 0  , 5148, 353),
  INST(Vgatherpf0qps   , VexM_VM            , V(660F38,C7,1,2,_,0,2,T1S), 0                         , F(RO)|F(VM)|F(Evex)                   , EF(________), 0 , 0 , kFamilyNone, 0  , 5162, 353),
  INST(Vgatherpf1dpd   , VexM_VM            , V(660F38,C6,2,2,_,1,3,T1S), 0                         , F(RO)|F(VM)|F(Evex)                   , EF(________), 0 , 0 , kFamilyNone, 0  , 5176, 351),
  INST(Vgatherpf1dps   , VexM_VM            , V(660F38,C6,2,2,_,0,2,T1S), 0                         , F(RO)|F(VM)|F(Evex)                   , EF(________), 0 , 0 , kFamilyNone, 0  , 5190, 352),
  INST(Vgatherpf1qpd   , VexM_VM            , V(660F38,C7,2,2,_,1,3,T1S), 0                         , F(RO)|F(VM)|F(Evex)                   , EF(________), 0 , 0 , kFamilyNone, 0  , 5204, 353),
  INST(Vgatherpf1qps   , VexM_VM            , V(660F38,C7,2,2,_,0,2,T1S), 0                         , F(RO)|F(VM)|F(Evex)                   , EF(________), 0 , 0 , kFamilyNone, 0  , 5218, 353),
  INST(Vgatherqpd      , VexRmvRm_VM        , V(660F38,93,_,x,1,_,_,_  ), V(660F38,93,_,x,_,1,3,T1S), F(RW)|F(Vex_VM)|F(Evex)               , EF(________), 0 , 0 , kFamilyAvx , 46 , 5232, 354),
  INST(Vgatherqps      , VexRmvRm_VM        , V(660F38,93,_,x,0,_,_,_  ), V(660F38,93,_,x,_,0,2,T1S), F(RW)|F(Vex_VM)|F(Evex)               , EF(________), 0 , 0 , kFamilyAvx , 46 , 5243, 355),
  INST(Vgetexppd       , VexRm_Lx           , V(660F38,42,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 29 , 5254, 318),
  INST(Vgetexpps       , VexRm_Lx           , V(660F38,42,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 32 , 5264, 318),
  INST(Vgetexpsd       , VexRm              , V(660F38,43,_,I,_,1,3,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 37 , 5274, 356),
  INST(Vgetexpss       , VexRm              , V(660F38,43,_,I,_,0,2,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 37 , 5284, 357),
  INST(Vgetmantpd      , VexRmi_Lx          , V(660F3A,26,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 29 , 5294, 358),
  INST(Vgetmantps      , VexRmi_Lx          , V(660F3A,26,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 32 , 5305, 358),
  INST(Vgetmantsd      , VexRmi             , V(660F3A,27,_,I,_,1,3,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 37 , 5316, 359),
  INST(Vgetmantss      , VexRmi             , V(660F3A,27,_,I,_,0,2,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 37 , 5327, 360),
  INST(Vhaddpd         , VexRvm_Lx          , V(660F00,7C,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 4  , 5338, 291),
  INST(Vhaddps         , VexRvm_Lx          , V(F20F00,7C,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 4  , 5346, 291),
  INST(Vhsubpd         , VexRvm_Lx          , V(660F00,7D,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 4  , 5354, 291),
  INST(Vhsubps         , VexRvm_Lx          , V(F20F00,7D,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 4  , 5362, 291),
  INST(Vinsertf128     , VexRvmi            , V(660F3A,18,_,1,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 4  , 5370, 361),
  INST(Vinsertf32x4    , VexRvmi_Lx         , V(660F3A,18,_,x,_,0,4,T4 ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 15 , 5382, 362),
  INST(Vinsertf32x8    , VexRvmi            , V(660F3A,1A,_,2,_,0,5,T8 ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 13 , 5395, 363),
  INST(Vinsertf64x2    , VexRvmi_Lx         , V(660F3A,18,_,x,_,1,4,T2 ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 11 , 5408, 362),
  INST(Vinsertf64x4    , VexRvmi            , V(660F3A,1A,_,2,_,1,5,T4 ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 12 , 5421, 363),
  INST(Vinserti128     , VexRvmi            , V(660F3A,38,_,1,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 14 , 5434, 361),
  INST(Vinserti32x4    , VexRvmi_Lx         , V(660F3A,38,_,x,_,0,4,T4 ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 15 , 5446, 362),
  INST(Vinserti32x8    , VexRvmi            , V(660F3A,3A,_,2,_,0,5,T8 ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 13 , 5459, 363),
  INST(Vinserti64x2    , VexRvmi_Lx         , V(660F3A,38,_,x,_,1,4,T2 ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 11 , 5472, 362),
  INST(Vinserti64x4    , VexRvmi            , V(660F3A,3A,_,2,_,1,5,T4 ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 12 , 5485, 363),
  INST(Vinsertps       , VexRvmi            , V(660F3A,21,_,0,I,0,2,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 36 , 5498, 364),
  INST(Vlddqu          , VexRm_Lx           , V(F20F00,F0,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 4  , 5508, 365),
  INST(Vldmxcsr        , VexM               , V(000F00,AE,2,0,I,_,_,_  ), 0                         , F(RO)|F(Vex)|F(Volatile)              , EF(________), 0 , 0 , kFamilyNone, 0  , 5515, 366),
  INST(Vmaskmovdqu     , VexRm_ZDI          , V(660F00,F7,_,0,I,_,_,_  ), 0                         , F(RO)|F(Vex)|F(Special)               , EF(________), 0 , 0 , kFamilyAvx , 4  , 5524, 367),
  INST(Vmaskmovpd      , VexRvmMvr_Lx       , V(660F38,2D,_,x,0,_,_,_  ), V(660F38,2F,_,x,0,_,_,_  ), F(RW)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 4  , 5536, 368),
  INST(Vmaskmovps      , VexRvmMvr_Lx       , V(660F38,2C,_,x,0,_,_,_  ), V(660F38,2E,_,x,0,_,_,_  ), F(RW)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 4  , 5547, 369),
  INST(Vmaxpd          , VexRvm_Lx          , V(660F00,5F,_,x,I,1,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 17 , 5558, 288),
  INST(Vmaxps          , VexRvm_Lx          , V(000F00,5F,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 18 , 5565, 288),
  INST(Vmaxsd          , VexRvm             , V(F20F00,5F,_,I,I,1,3,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 47 , 5572, 289),
  INST(Vmaxss          , VexRvm             , V(F30F00,5F,_,I,I,0,2,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 47 , 5579, 290),
  INST(Vminpd          , VexRvm_Lx          , V(660F00,5D,_,x,I,1,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 17 , 5586, 288),
  INST(Vminps          , VexRvm_Lx          , V(000F00,5D,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 18 , 5593, 288),
  INST(Vminsd          , VexRvm             , V(F20F00,5D,_,I,I,1,3,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 47 , 5600, 289),
  INST(Vminss          , VexRvm             , V(F30F00,5D,_,I,I,0,2,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 47 , 5607, 290),
  INST(Vmovapd         , VexRmMr_Lx         , V(660F00,28,_,x,I,1,4,FVM), V(660F00,29,_,x,I,1,4,FVM), F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 48 , 5614, 370),
  INST(Vmovaps         , VexRmMr_Lx         , V(000F00,28,_,x,I,0,4,FVM), V(000F00,29,_,x,I,0,4,FVM), F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 48 , 5622, 371),
  INST(Vmovd           , VexMovdMovq        , V(660F00,6E,_,0,0,0,2,T1S), V(660F00,7E,_,0,0,0,2,T1S), F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 36 , 5630, 372),
  INST(Vmovddup        , VexRm_Lx           , V(F20F00,12,_,x,I,1,3,DUP), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 48 , 5636, 373),
  INST(Vmovdqa         , VexRmMr_Lx         , V(660F00,6F,_,x,I,_,_,_  ), V(660F00,7F,_,x,I,_,_,_  ), F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 4  , 5645, 374),
  INST(Vmovdqa32       , VexRmMr_Lx         , V(660F00,6F,_,x,_,0,4,FVM), V(660F00,7F,_,x,_,0,4,FVM), F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 15 , 5653, 375),
  INST(Vmovdqa64       , VexRmMr_Lx         , V(660F00,6F,_,x,_,1,4,FVM), V(660F00,7F,_,x,_,1,4,FVM), F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 15 , 5663, 376),
  INST(Vmovdqu         , VexRmMr_Lx         , V(F30F00,6F,_,x,I,_,_,_  ), V(F30F00,7F,_,x,I,_,_,_  ), F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 4  , 5673, 377),
  INST(Vmovdqu16       , VexRmMr_Lx         , V(F20F00,6F,_,x,_,1,4,FVM), V(F20F00,7F,_,x,_,1,4,FVM), F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 10 , 5681, 378),
  INST(Vmovdqu32       , VexRmMr_Lx         , V(F30F00,6F,_,x,_,0,4,FVM), V(F30F00,7F,_,x,_,0,4,FVM), F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 15 , 5691, 379),
  INST(Vmovdqu64       , VexRmMr_Lx         , V(F30F00,6F,_,x,_,1,4,FVM), V(F30F00,7F,_,x,_,1,4,FVM), F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 15 , 5701, 380),
  INST(Vmovdqu8        , VexRmMr_Lx         , V(F20F00,6F,_,x,_,0,4,FVM), V(F20F00,7F,_,x,_,0,4,FVM), F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 10 , 5711, 381),
  INST(Vmovhlps        , VexRvm             , V(000F00,12,_,0,I,0,_,_  ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 36 , 5720, 382),
  INST(Vmovhpd         , VexRvmMr           , V(660F00,16,_,0,I,1,3,T1S), V(660F00,17,_,0,I,1,3,T1S), F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 36 , 5729, 383),
  INST(Vmovhps         , VexRvmMr           , V(000F00,16,_,0,I,0,3,T2 ), V(000F00,17,_,0,I,0,3,T2 ), F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 36 , 5737, 384),
  INST(Vmovlhps        , VexRvm             , V(000F00,16,_,0,I,0,_,_  ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 36 , 5745, 382),
  INST(Vmovlpd         , VexRvmMr           , V(660F00,12,_,0,I,1,3,T1S), V(660F00,13,_,0,I,1,3,T1S), F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 36 , 5754, 385),
  INST(Vmovlps         , VexRvmMr           , V(000F00,12,_,0,I,0,3,T2 ), V(000F00,13,_,0,I,0,3,T2 ), F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 36 , 5762, 386),
  INST(Vmovmskpd       , VexRm_Lx           , V(660F00,50,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 4  , 5770, 387),
  INST(Vmovmskps       , VexRm_Lx           , V(000F00,50,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 4  , 5780, 387),
  INST(Vmovntdq        , VexMr_Lx           , V(660F00,E7,_,x,I,0,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 49 , 5790, 388),
  INST(Vmovntdqa       , VexRm_Lx           , V(660F38,2A,_,x,I,0,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 50 , 5799, 389),
  INST(Vmovntpd        , VexMr_Lx           , V(660F00,2B,_,x,I,1,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 49 , 5809, 388),
  INST(Vmovntps        , VexMr_Lx           , V(000F00,2B,_,x,I,0,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 49 , 5818, 388),
  INST(Vmovq           , VexMovdMovq        , V(660F00,6E,_,0,I,1,3,T1S), V(660F00,7E,_,0,I,1,3,T1S), F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 36 , 5827, 390),
  INST(Vmovsd          , VexMovssMovsd      , V(F20F00,10,_,I,I,1,3,T1S), V(F20F00,11,_,I,I,1,3,T1S), F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 51 , 5833, 391),
  INST(Vmovshdup       , VexRm_Lx           , V(F30F00,16,_,x,I,0,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 48 , 5840, 315),
  INST(Vmovsldup       , VexRm_Lx           , V(F30F00,12,_,x,I,0,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 48 , 5850, 315),
  INST(Vmovss          , VexMovssMovsd      , V(F30F00,10,_,I,I,0,2,T1S), V(F30F00,11,_,I,I,0,2,T1S), F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 51 , 5860, 392),
  INST(Vmovupd         , VexRmMr_Lx         , V(660F00,10,_,x,I,1,4,FVM), V(660F00,11,_,x,I,1,4,FVM), F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 48 , 5867, 393),
  INST(Vmovups         , VexRmMr_Lx         , V(000F00,10,_,x,I,0,4,FVM), V(000F00,11,_,x,I,0,4,FVM), F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 48 , 5875, 394),
  INST(Vmpsadbw        , VexRvmi_Lx         , V(660F3A,42,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 52 , 5883, 298),
  INST(Vmulpd          , VexRvm_Lx          , V(660F00,59,_,x,I,1,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 53 , 5892, 288),
  INST(Vmulps          , VexRvm_Lx          , V(000F00,59,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 54 , 5899, 288),
  INST(Vmulsd          , VexRvm_Lx          , V(F20F00,59,_,I,I,1,3,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 3  , 5906, 289),
  INST(Vmulss          , VexRvm_Lx          , V(F30F00,59,_,I,I,0,2,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 3  , 5913, 290),
  INST(Vorpd           , VexRvm_Lx          , V(660F00,56,_,x,I,1,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 8  , 5920, 296),
  INST(Vorps           , VexRvm_Lx          , V(000F00,56,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 21 , 5926, 296),
  INST(Vp4dpwssd       , VexRm_T1_4X        , V(F20F38,52,_,2,_,0,2,T1X), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 55 , 5932, 395),
  INST(Vp4dpwssds      , VexRm_T1_4X        , V(F20F38,53,_,2,_,0,2,T1X), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 55 , 5942, 395),
  INST(Vpabsb          , VexRm_Lx           , V(660F38,1C,_,x,I,_,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 56 , 5953, 315),
  INST(Vpabsd          , VexRm_Lx           , V(660F38,1E,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 16 , 5960, 315),
  INST(Vpabsq          , VexRm_Lx           , V(660F38,1F,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 15 , 5967, 318),
  INST(Vpabsw          , VexRm_Lx           , V(660F38,1D,_,x,I,_,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 56 , 5974, 315),
  INST(Vpackssdw       , VexRvm_Lx          , V(660F00,6B,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 57 , 5981, 288),
  INST(Vpacksswb       , VexRvm_Lx          , V(660F00,63,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 56 , 5991, 288),
  INST(Vpackusdw       , VexRvm_Lx          , V(660F38,2B,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 57 , 6001, 288),
  INST(Vpackuswb       , VexRvm_Lx          , V(660F00,67,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 56 , 6011, 288),
  INST(Vpaddb          , VexRvm_Lx          , V(660F00,FC,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 56 , 6021, 288),
  INST(Vpaddd          , VexRvm_Lx          , V(660F00,FE,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 58 , 6028, 288),
  INST(Vpaddq          , VexRvm_Lx          , V(660F00,D4,_,x,I,1,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 59 , 6035, 288),
  INST(Vpaddsb         , VexRvm_Lx          , V(660F00,EC,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 56 , 6042, 288),
  INST(Vpaddsw         , VexRvm_Lx          , V(660F00,ED,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 56 , 6050, 288),
  INST(Vpaddusb        , VexRvm_Lx          , V(660F00,DC,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 56 , 6058, 288),
  INST(Vpaddusw        , VexRvm_Lx          , V(660F00,DD,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 56 , 6067, 288),
  INST(Vpaddw          , VexRvm_Lx          , V(660F00,FD,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 56 , 6076, 288),
  INST(Vpalignr        , VexRvmi_Lx         , V(660F3A,0F,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 56 , 6083, 396),
  INST(Vpand           , VexRvm_Lx          , V(660F00,DB,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 52 , 6092, 397),
  INST(Vpandd          , VexRvm_Lx          , V(660F00,DB,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 6  , 6098, 398),
  INST(Vpandn          , VexRvm_Lx          , V(660F00,DF,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 52 , 6105, 399),
  INST(Vpandnd         , VexRvm_Lx          , V(660F00,DF,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 6  , 6112, 400),
  INST(Vpandnq         , VexRvm_Lx          , V(660F00,DF,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 7  , 6120, 400),
  INST(Vpandq          , VexRvm_Lx          , V(660F00,DB,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 7  , 6128, 398),
  INST(Vpavgb          , VexRvm_Lx          , V(660F00,E0,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 60 , 6135, 288),
  INST(Vpavgw          , VexRvm_Lx          , V(660F00,E3,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 61 , 6142, 288),
  INST(Vpblendd        , VexRvmi_Lx         , V(660F3A,02,_,x,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 14 , 6149, 298),
  INST(Vpblendvb       , VexRvmr            , V(660F3A,4C,_,x,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 52 , 6158, 299),
  INST(Vpblendw        , VexRvmi_Lx         , V(660F3A,0E,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 52 , 6168, 298),
  INST(Vpbroadcastb    , VexRm_Lx           , V(660F38,78,_,x,0,0,0,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 61 , 6177, 401),
  INST(Vpbroadcastd    , VexRm_Lx           , V(660F38,58,_,x,0,0,2,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 62 , 6190, 402),
  INST(Vpbroadcastmb2d , VexRm_Lx           , V(F30F38,3A,_,x,_,0,_,_  ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 63 , 6203, 403),
  INST(Vpbroadcastmb2q , VexRm_Lx           , V(F30F38,2A,_,x,_,1,_,_  ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 63 , 6219, 403),
  INST(Vpbroadcastq    , VexRm_Lx           , V(660F38,59,_,x,0,1,3,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 62 , 6235, 404),
  INST(Vpbroadcastw    , VexRm_Lx           , V(660F38,79,_,x,0,0,1,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 61 , 6248, 405),
  INST(Vpclmulqdq      , VexRvmi            , V(660F3A,44,_,0,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 64 , 6261, 406),
  INST(Vpcmov          , VexRvrmRvmr_Lx     , V(XOP_M8,A2,_,x,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 45 , 6272, 340),
  INST(Vpcmpb          , VexRvm_Lx          , V(660F3A,3F,_,x,_,0,4,FVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 65 , 6279, 407),
  INST(Vpcmpd          , VexRvm_Lx          , V(660F3A,1F,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 66 , 6286, 407),
  INST(Vpcmpeqb        , VexRvm_Lx          , V(660F00,74,_,x,I,I,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 67 , 6293, 408),
  INST(Vpcmpeqd        , VexRvm_Lx          , V(660F00,76,_,x,I,0,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 68 , 6302, 408),
  INST(Vpcmpeqq        , VexRvm_Lx          , V(660F38,29,_,x,I,1,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 69 , 6311, 408),
  INST(Vpcmpeqw        , VexRvm_Lx          , V(660F00,75,_,x,I,I,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 67 , 6320, 408),
  INST(Vpcmpestri      , VexRmi             , V(660F3A,61,_,0,I,_,_,_  ), 0                         , F(WO)|F(Vex)|F(Special)               , EF(________), 0 , 0 , kFamilyAvx , 4  , 6329, 409),
  INST(Vpcmpestrm      , VexRmi             , V(660F3A,60,_,0,I,_,_,_  ), 0                         , F(WO)|F(Vex)|F(Special)               , EF(________), 0 , 0 , kFamilyAvx , 4  , 6340, 410),
  INST(Vpcmpgtb        , VexRvm_Lx          , V(660F00,64,_,x,I,I,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 67 , 6351, 408),
  INST(Vpcmpgtd        , VexRvm_Lx          , V(660F00,66,_,x,I,0,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 68 , 6360, 408),
  INST(Vpcmpgtq        , VexRvm_Lx          , V(660F38,37,_,x,I,1,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 69 , 6369, 408),
  INST(Vpcmpgtw        , VexRvm_Lx          , V(660F00,65,_,x,I,I,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 67 , 6378, 408),
  INST(Vpcmpistri      , VexRmi             , V(660F3A,63,_,0,I,_,_,_  ), 0                         , F(WO)|F(Vex)|F(Special)               , EF(________), 0 , 0 , kFamilyAvx , 4  , 6387, 411),
  INST(Vpcmpistrm      , VexRmi             , V(660F3A,62,_,0,I,_,_,_  ), 0                         , F(WO)|F(Vex)|F(Special)               , EF(________), 0 , 0 , kFamilyAvx , 4  , 6398, 412),
  INST(Vpcmpq          , VexRvm_Lx          , V(660F3A,1F,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 70 , 6409, 407),
  INST(Vpcmpub         , VexRvm_Lx          , V(660F3A,3E,_,x,_,0,4,FVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 65 , 6416, 407),
  INST(Vpcmpud         , VexRvm_Lx          , V(660F3A,1E,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 66 , 6424, 407),
  INST(Vpcmpuq         , VexRvm_Lx          , V(660F3A,1E,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 70 , 6432, 407),
  INST(Vpcmpuw         , VexRvm_Lx          , V(660F3A,3E,_,x,_,1,4,FVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 71 , 6440, 407),
  INST(Vpcmpw          , VexRvm_Lx          , V(660F3A,3F,_,x,_,1,4,FVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 71 , 6448, 407),
  INST(Vpcomb          , VexRvmi            , V(XOP_M8,CC,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 45 , 6455, 406),
  INST(Vpcomd          , VexRvmi            , V(XOP_M8,CE,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 45 , 6462, 406),
  INST(Vpcompressd     , VexMr_Lx           , V(660F38,8B,_,x,_,0,2,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 15 , 6469, 313),
  INST(Vpcompressq     , VexMr_Lx           , V(660F38,8B,_,x,_,1,3,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 15 , 6481, 313),
  INST(Vpcomq          , VexRvmi            , V(XOP_M8,CF,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 45 , 6493, 406),
  INST(Vpcomub         , VexRvmi            , V(XOP_M8,EC,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 45 , 6500, 406),
  INST(Vpcomud         , VexRvmi            , V(XOP_M8,EE,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 45 , 6508, 406),
  INST(Vpcomuq         , VexRvmi            , V(XOP_M8,EF,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 45 , 6516, 406),
  INST(Vpcomuw         , VexRvmi            , V(XOP_M8,ED,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 45 , 6524, 406),
  INST(Vpcomw          , VexRvmi            , V(XOP_M8,CD,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 45 , 6532, 406),
  INST(Vpconflictd     , VexRm_Lx           , V(660F38,C4,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 72 , 6539, 318),
  INST(Vpconflictq     , VexRm_Lx           , V(660F38,C4,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 72 , 6551, 318),
  INST(Vperm2f128      , VexRvmi            , V(660F3A,06,_,1,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 4  , 6563, 413),
  INST(Vperm2i128      , VexRvmi            , V(660F3A,46,_,1,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 14 , 6574, 413),
  INST(Vpermb          , VexRvm_Lx          , V(660F38,8D,_,x,_,0,4,FVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 73 , 6585, 297),
  INST(Vpermd          , VexRvm_Lx          , V(660F38,36,_,x,0,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 74 , 6592, 414),
  INST(Vpermi2b        , VexRvm_Lx          , V(660F38,75,_,x,_,0,4,FVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 73 , 6599, 297),
  INST(Vpermi2d        , VexRvm_Lx          , V(660F38,76,_,x,_,0,4,FV ), 0                         , F(RW)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 6  , 6608, 415),
  INST(Vpermi2pd       , VexRvm_Lx          , V(660F38,77,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 7  , 6617, 297),
  INST(Vpermi2ps       , VexRvm_Lx          , V(660F38,77,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 6  , 6627, 297),
  INST(Vpermi2q        , VexRvm_Lx          , V(660F38,76,_,x,_,1,4,FV ), 0                         , F(RW)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 7  , 6637, 415),
  INST(Vpermi2w        , VexRvm_Lx          , V(660F38,75,_,x,_,1,4,FVM), 0                         , F(RW)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 10 , 6646, 415),
  INST(Vpermil2pd      , VexRvrmiRvmri_Lx   , V(660F3A,49,_,x,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 45 , 6655, 416),
  INST(Vpermil2ps      , VexRvrmiRvmri_Lx   , V(660F3A,48,_,x,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 45 , 6666, 416),
  INST(Vpermilpd       , VexRvmRmi_Lx       , V(660F38,0D,_,x,0,1,4,FV ), V(660F3A,05,_,x,0,1,4,FV ), F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 75 , 6677, 417),
  INST(Vpermilps       , VexRvmRmi_Lx       , V(660F38,0C,_,x,0,0,4,FV ), V(660F3A,04,_,x,0,0,4,FV ), F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 75 , 6687, 418),
  INST(Vpermpd         , VexRmi             , V(660F3A,01,_,1,1,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 14 , 6697, 419),
  INST(Vpermps         , VexRvm             , V(660F38,16,_,1,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 14 , 6705, 420),
  INST(Vpermq          , VexRvmRmi_Lx       , V(660F38,36,_,x,_,1,4,FV ), V(660F3A,00,_,x,1,1,4,FV ), F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 76 , 6713, 421),
  INST(Vpermt2b        , VexRvm_Lx          , V(660F38,7D,_,x,_,0,4,FVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 73 , 6720, 297),
  INST(Vpermt2d        , VexRvm_Lx          , V(660F38,7E,_,x,_,0,4,FV ), 0                         , F(RW)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 6  , 6729, 415),
  INST(Vpermt2pd       , VexRvm_Lx          , V(660F38,7F,_,x,_,1,4,FV ), 0                         , F(RW)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 7  , 6738, 415),
  INST(Vpermt2ps       , VexRvm_Lx          , V(660F38,7F,_,x,_,0,4,FV ), 0                         , F(RW)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 6  , 6748, 415),
  INST(Vpermt2q        , VexRvm_Lx          , V(660F38,7E,_,x,_,1,4,FV ), 0                         , F(RW)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 7  , 6758, 415),
  INST(Vpermt2w        , VexRvm_Lx          , V(660F38,7D,_,x,_,1,4,FVM), 0                         , F(RW)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 10 , 6767, 415),
  INST(Vpermw          , VexRvm_Lx          , V(660F38,8D,_,x,_,1,4,FVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 10 , 6776, 297),
  INST(Vpexpandd       , VexRm_Lx           , V(660F38,89,_,x,_,0,2,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 15 , 6783, 318),
  INST(Vpexpandq       , VexRm_Lx           , V(660F38,89,_,x,_,1,3,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 15 , 6793, 318),
  INST(Vpextrb         , VexMri             , V(660F3A,14,_,0,0,I,0,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 77 , 6803, 422),
  INST(Vpextrd         , VexMri             , V(660F3A,16,_,0,0,0,2,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 78 , 6811, 333),
  INST(Vpextrq         , VexMri             , V(660F3A,16,_,0,1,1,3,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 78 , 6819, 423),
  INST(Vpextrw         , VexMri             , V(660F3A,15,_,0,0,I,1,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 77 , 6827, 424),
  INST(Vpgatherdd      , VexRmvRm_VM        , V(660F38,90,_,x,0,_,_,_  ), V(660F38,90,_,x,_,0,2,T1S), F(RW)|F(Vex_VM)|F(Evex)               , EF(________), 0 , 0 , kFamilyAvx , 46 , 6835, 425),
  INST(Vpgatherdq      , VexRmvRm_VM        , V(660F38,90,_,x,1,_,_,_  ), V(660F38,90,_,x,_,1,3,T1S), F(RW)|F(Vex_VM)|F(Evex)               , EF(________), 0 , 0 , kFamilyAvx , 46 , 6846, 426),
  INST(Vpgatherqd      , VexRmvRm_VM        , V(660F38,91,_,x,0,_,_,_  ), V(660F38,91,_,x,_,0,2,T1S), F(RW)|F(Vex_VM)|F(Evex)               , EF(________), 0 , 0 , kFamilyAvx , 46 , 6857, 427),
  INST(Vpgatherqq      , VexRmvRm_VM        , V(660F38,91,_,x,1,_,_,_  ), V(660F38,91,_,x,_,1,3,T1S), F(RW)|F(Vex_VM)|F(Evex)               , EF(________), 0 , 0 , kFamilyAvx , 46 , 6868, 428),
  INST(Vphaddbd        , VexRm              , V(XOP_M9,C2,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 45 , 6879, 293),
  INST(Vphaddbq        , VexRm              , V(XOP_M9,C3,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 45 , 6888, 293),
  INST(Vphaddbw        , VexRm              , V(XOP_M9,C1,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 45 , 6897, 293),
  INST(Vphaddd         , VexRvm_Lx          , V(660F38,02,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 52 , 6906, 291),
  INST(Vphadddq        , VexRm              , V(XOP_M9,CB,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 45 , 6914, 293),
  INST(Vphaddsw        , VexRvm_Lx          , V(660F38,03,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 52 , 6923, 291),
  INST(Vphaddubd       , VexRm              , V(XOP_M9,D2,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 45 , 6932, 293),
  INST(Vphaddubq       , VexRm              , V(XOP_M9,D3,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 45 , 6942, 293),
  INST(Vphaddubw       , VexRm              , V(XOP_M9,D1,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 45 , 6952, 293),
  INST(Vphaddudq       , VexRm              , V(XOP_M9,DB,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 45 , 6962, 293),
  INST(Vphadduwd       , VexRm              , V(XOP_M9,D6,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 45 , 6972, 293),
  INST(Vphadduwq       , VexRm              , V(XOP_M9,D7,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 45 , 6982, 293),
  INST(Vphaddw         , VexRvm_Lx          , V(660F38,01,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 52 , 6992, 291),
  INST(Vphaddwd        , VexRm              , V(XOP_M9,C6,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 45 , 7000, 293),
  INST(Vphaddwq        , VexRm              , V(XOP_M9,C7,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 45 , 7009, 293),
  INST(Vphminposuw     , VexRm              , V(660F38,41,_,0,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 4  , 7018, 293),
  INST(Vphsubbw        , VexRm              , V(XOP_M9,E1,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 45 , 7030, 293),
  INST(Vphsubd         , VexRvm_Lx          , V(660F38,06,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 52 , 7039, 291),
  INST(Vphsubdq        , VexRm              , V(XOP_M9,E3,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 45 , 7047, 293),
  INST(Vphsubsw        , VexRvm_Lx          , V(660F38,07,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 52 , 7056, 291),
  INST(Vphsubw         , VexRvm_Lx          , V(660F38,05,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 52 , 7065, 291),
  INST(Vphsubwd        , VexRm              , V(XOP_M9,E2,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 45 , 7073, 293),
  INST(Vpinsrb         , VexRvmi            , V(660F3A,20,_,0,0,I,0,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 79 , 7082, 429),
  INST(Vpinsrd         , VexRvmi            , V(660F3A,22,_,0,0,0,2,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 80 , 7090, 430),
  INST(Vpinsrq         , VexRvmi            , V(660F3A,22,_,0,1,1,3,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 80 , 7098, 431),
  INST(Vpinsrw         , VexRvmi            , V(660F00,C4,_,0,0,I,1,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 79 , 7106, 432),
  INST(Vplzcntd        , VexRm_Lx           , V(660F38,44,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 72 , 7114, 318),
  INST(Vplzcntq        , VexRm_Lx           , V(660F38,44,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 81 , 7123, 318),
  INST(Vpmacsdd        , VexRvmr            , V(XOP_M8,9E,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 45 , 7132, 433),
  INST(Vpmacsdqh       , VexRvmr            , V(XOP_M8,9F,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 45 , 7141, 433),
  INST(Vpmacsdql       , VexRvmr            , V(XOP_M8,97,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 45 , 7151, 433),
  INST(Vpmacssdd       , VexRvmr            , V(XOP_M8,8E,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 45 , 7161, 433),
  INST(Vpmacssdqh      , VexRvmr            , V(XOP_M8,8F,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 45 , 7171, 433),
  INST(Vpmacssdql      , VexRvmr            , V(XOP_M8,87,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 45 , 7182, 433),
  INST(Vpmacsswd       , VexRvmr            , V(XOP_M8,86,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 45 , 7193, 433),
  INST(Vpmacssww       , VexRvmr            , V(XOP_M8,85,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 45 , 7203, 433),
  INST(Vpmacswd        , VexRvmr            , V(XOP_M8,96,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 45 , 7213, 433),
  INST(Vpmacsww        , VexRvmr            , V(XOP_M8,95,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 45 , 7222, 433),
  INST(Vpmadcsswd      , VexRvmr            , V(XOP_M8,A6,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 45 , 7231, 433),
  INST(Vpmadcswd       , VexRvmr            , V(XOP_M8,B6,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 45 , 7242, 433),
  INST(Vpmadd52huq     , VexRvm_Lx          , V(660F38,B5,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 82 , 7252, 297),
  INST(Vpmadd52luq     , VexRvm_Lx          , V(660F38,B4,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 82 , 7264, 297),
  INST(Vpmaddubsw      , VexRvm_Lx          , V(660F38,04,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 56 , 7276, 288),
  INST(Vpmaddwd        , VexRvm_Lx          , V(660F00,F5,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 56 , 7287, 288),
  INST(Vpmaskmovd      , VexRvmMvr_Lx       , V(660F38,8C,_,x,0,_,_,_  ), V(660F38,8E,_,x,0,_,_,_  ), F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 14 , 7296, 434),
  INST(Vpmaskmovq      , VexRvmMvr_Lx       , V(660F38,8C,_,x,1,_,_,_  ), V(660F38,8E,_,x,1,_,_,_  ), F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 14 , 7307, 435),
  INST(Vpmaxsb         , VexRvm_Lx          , V(660F38,3C,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 56 , 7318, 296),
  INST(Vpmaxsd         , VexRvm_Lx          , V(660F38,3D,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 58 , 7326, 296),
  INST(Vpmaxsq         , VexRvm_Lx          , V(660F38,3D,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 7  , 7334, 297),
  INST(Vpmaxsw         , VexRvm_Lx          , V(660F00,EE,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 56 , 7342, 296),
  INST(Vpmaxub         , VexRvm_Lx          , V(660F00,DE,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 56 , 7350, 296),
  INST(Vpmaxud         , VexRvm_Lx          , V(660F38,3F,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 58 , 7358, 296),
  INST(Vpmaxuq         , VexRvm_Lx          , V(660F38,3F,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 7  , 7366, 297),
  INST(Vpmaxuw         , VexRvm_Lx          , V(660F38,3E,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 56 , 7374, 296),
  INST(Vpminsb         , VexRvm_Lx          , V(660F38,38,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 56 , 7382, 296),
  INST(Vpminsd         , VexRvm_Lx          , V(660F38,39,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 58 , 7390, 296),
  INST(Vpminsq         , VexRvm_Lx          , V(660F38,39,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 7  , 7398, 297),
  INST(Vpminsw         , VexRvm_Lx          , V(660F00,EA,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 56 , 7406, 296),
  INST(Vpminub         , VexRvm_Lx          , V(660F00,DA,_,x,I,_,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 56 , 7414, 296),
  INST(Vpminud         , VexRvm_Lx          , V(660F38,3B,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 58 , 7422, 296),
  INST(Vpminuq         , VexRvm_Lx          , V(660F38,3B,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 7  , 7430, 297),
  INST(Vpminuw         , VexRvm_Lx          , V(660F38,3A,_,x,I,_,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 56 , 7438, 296),
  INST(Vpmovb2m        , VexRm_Lx           , V(F30F38,29,_,x,_,0,_,_  ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 83 , 7446, 436),
  INST(Vpmovd2m        , VexRm_Lx           , V(F30F38,39,_,x,_,0,_,_  ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 84 , 7455, 436),
  INST(Vpmovdb         , VexMr_Lx           , V(F30F38,31,_,x,_,0,2,QVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 15 , 7464, 437),
  INST(Vpmovdw         , VexMr_Lx           , V(F30F38,33,_,x,_,0,3,HVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 15 , 7472, 438),
  INST(Vpmovm2b        , VexRm_Lx           , V(F30F38,28,_,x,_,0,_,_  ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 83 , 7480, 403),
  INST(Vpmovm2d        , VexRm_Lx           , V(F30F38,38,_,x,_,0,_,_  ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 84 , 7489, 403),
  INST(Vpmovm2q        , VexRm_Lx           , V(F30F38,38,_,x,_,1,_,_  ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 84 , 7498, 403),
  INST(Vpmovm2w        , VexRm_Lx           , V(F30F38,28,_,x,_,1,_,_  ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 83 , 7507, 403),
  INST(Vpmovmskb       , VexRm_Lx           , V(660F00,D7,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 52 , 7516, 387),
  INST(Vpmovq2m        , VexRm_Lx           , V(F30F38,39,_,x,_,1,_,_  ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 84 , 7526, 436),
  INST(Vpmovqb         , VexMr_Lx           , V(F30F38,32,_,x,_,0,1,OVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 15 , 7535, 439),
  INST(Vpmovqd         , VexMr_Lx           , V(F30F38,35,_,x,_,0,3,HVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 15 , 7543, 438),
  INST(Vpmovqw         , VexMr_Lx           , V(F30F38,34,_,x,_,0,2,QVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 15 , 7551, 437),
  INST(Vpmovsdb        , VexMr_Lx           , V(F30F38,21,_,x,_,0,2,QVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 15 , 7559, 437),
  INST(Vpmovsdw        , VexMr_Lx           , V(F30F38,23,_,x,_,0,3,HVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 15 , 7568, 438),
  INST(Vpmovsqb        , VexMr_Lx           , V(F30F38,22,_,x,_,0,1,OVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 15 , 7577, 439),
  INST(Vpmovsqd        , VexMr_Lx           , V(F30F38,25,_,x,_,0,3,HVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 15 , 7586, 438),
  INST(Vpmovsqw        , VexMr_Lx           , V(F30F38,24,_,x,_,0,2,QVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 15 , 7595, 437),
  INST(Vpmovswb        , VexMr_Lx           , V(F30F38,20,_,x,_,0,3,HVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 10 , 7604, 438),
  INST(Vpmovsxbd       , VexRm_Lx           , V(660F38,21,_,x,I,I,2,QVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 16 , 7613, 440),
  INST(Vpmovsxbq       , VexRm_Lx           , V(660F38,22,_,x,I,I,1,OVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 16 , 7623, 441),
  INST(Vpmovsxbw       , VexRm_Lx           , V(660F38,20,_,x,I,I,3,HVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 56 , 7633, 314),
  INST(Vpmovsxdq       , VexRm_Lx           , V(660F38,25,_,x,I,0,3,HVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 16 , 7643, 442),
  INST(Vpmovsxwd       , VexRm_Lx           , V(660F38,23,_,x,I,I,3,HVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 16 , 7653, 314),
  INST(Vpmovsxwq       , VexRm_Lx           , V(660F38,24,_,x,I,I,2,QVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 16 , 7663, 440),
  INST(Vpmovusdb       , VexMr_Lx           , V(F30F38,11,_,x,_,0,2,QVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 15 , 7673, 437),
  INST(Vpmovusdw       , VexMr_Lx           , V(F30F38,13,_,x,_,0,3,HVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 15 , 7683, 438),
  INST(Vpmovusqb       , VexMr_Lx           , V(F30F38,12,_,x,_,0,1,OVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 15 , 7693, 439),
  INST(Vpmovusqd       , VexMr_Lx           , V(F30F38,15,_,x,_,0,3,HVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 15 , 7703, 438),
  INST(Vpmovusqw       , VexMr_Lx           , V(F30F38,14,_,x,_,0,2,QVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 15 , 7713, 437),
  INST(Vpmovuswb       , VexMr_Lx           , V(F30F38,10,_,x,_,0,3,HVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 10 , 7723, 438),
  INST(Vpmovw2m        , VexRm_Lx           , V(F30F38,29,_,x,_,1,_,_  ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 83 , 7733, 436),
  INST(Vpmovwb         , VexMr_Lx           , V(F30F38,30,_,x,_,0,3,HVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 10 , 7742, 438),
  INST(Vpmovzxbd       , VexRm_Lx           , V(660F38,31,_,x,I,I,2,QVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 16 , 7750, 440),
  INST(Vpmovzxbq       , VexRm_Lx           , V(660F38,32,_,x,I,I,1,OVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 16 , 7760, 441),
  INST(Vpmovzxbw       , VexRm_Lx           , V(660F38,30,_,x,I,I,3,HVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 56 , 7770, 314),
  INST(Vpmovzxdq       , VexRm_Lx           , V(660F38,35,_,x,I,0,3,HVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 16 , 7780, 442),
  INST(Vpmovzxwd       , VexRm_Lx           , V(660F38,33,_,x,I,I,3,HVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 16 , 7790, 314),
  INST(Vpmovzxwq       , VexRm_Lx           , V(660F38,34,_,x,I,I,2,QVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 16 , 7800, 440),
  INST(Vpmuldq         , VexRvm_Lx          , V(660F38,28,_,x,I,1,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 59 , 7810, 288),
  INST(Vpmulhrsw       , VexRvm_Lx          , V(660F38,0B,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 56 , 7818, 288),
  INST(Vpmulhuw        , VexRvm_Lx          , V(660F00,E4,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 56 , 7828, 288),
  INST(Vpmulhw         , VexRvm_Lx          , V(660F00,E5,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 56 , 7837, 288),
  INST(Vpmulld         , VexRvm_Lx          , V(660F38,40,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 58 , 7845, 288),
  INST(Vpmullq         , VexRvm_Lx          , V(660F38,40,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 85 , 7853, 297),
  INST(Vpmullw         , VexRvm_Lx          , V(660F00,D5,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 56 , 7861, 288),
  INST(Vpmultishiftqb  , VexRvm_Lx          , V(660F38,83,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 86 , 7869, 297),
  INST(Vpmuludq        , VexRvm_Lx          , V(660F00,F4,_,x,I,1,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 59 , 7884, 288),
  INST(Vpopcntd        , VexRm              , V(660F38,55,_,2,_,0,4,FVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 87 , 7893, 329),
  INST(Vpopcntq        , VexRm              , V(660F38,55,_,2,_,1,4,FVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 88 , 7902, 329),
  INST(Vpor            , VexRvm_Lx          , V(660F00,EB,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 52 , 7911, 397),
  INST(Vpord           , VexRvm_Lx          , V(660F00,EB,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 6  , 7916, 398),
  INST(Vporq           , VexRvm_Lx          , V(660F00,EB,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 7  , 7922, 398),
  INST(Vpperm          , VexRvrmRvmr        , V(XOP_M8,A3,_,0,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 45 , 7928, 443),
  INST(Vprold          , VexVmi_Lx          , V(660F00,72,1,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 6  , 7935, 358),
  INST(Vprolq          , VexVmi_Lx          , V(660F00,72,1,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 7  , 7942, 358),
  INST(Vprolvd         , VexRvm_Lx          , V(660F38,15,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 6  , 7949, 297),
  INST(Vprolvq         , VexRvm_Lx          , V(660F38,15,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 7  , 7957, 297),
  INST(Vprord          , VexVmi_Lx          , V(660F00,72,0,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 6  , 7965, 358),
  INST(Vprorq          , VexVmi_Lx          , V(660F00,72,0,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 7  , 7972, 358),
  INST(Vprorvd         , VexRvm_Lx          , V(660F38,14,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 6  , 7979, 297),
  INST(Vprorvq         , VexRvm_Lx          , V(660F38,14,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 7  , 7987, 297),
  INST(Vprotb          , VexRvmRmvRmi       , V(XOP_M9,90,_,0,x,_,_,_  ), V(XOP_M8,C0,_,0,x,_,_,_  ), F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 45 , 7995, 444),
  INST(Vprotd          , VexRvmRmvRmi       , V(XOP_M9,92,_,0,x,_,_,_  ), V(XOP_M8,C2,_,0,x,_,_,_  ), F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 45 , 8002, 445),
  INST(Vprotq          , VexRvmRmvRmi       , V(XOP_M9,93,_,0,x,_,_,_  ), V(XOP_M8,C3,_,0,x,_,_,_  ), F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 45 , 8009, 446),
  INST(Vprotw          , VexRvmRmvRmi       , V(XOP_M9,91,_,0,x,_,_,_  ), V(XOP_M8,C1,_,0,x,_,_,_  ), F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 45 , 8016, 447),
  INST(Vpsadbw         , VexRvm_Lx          , V(660F00,F6,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 89 , 8023, 288),
  INST(Vpscatterdd     , VexMr_VM           , V(660F38,A0,_,x,_,0,2,T1S), 0                         , F(WO)|F(VM)|F(Evex)                   , EF(________), 0 , 0 , kFamilyAvx , 90 , 8031, 448),
  INST(Vpscatterdq     , VexMr_VM           , V(660F38,A0,_,x,_,1,3,T1S), 0                         , F(WO)|F(VM)|F(Evex)                   , EF(________), 0 , 0 , kFamilyAvx , 90 , 8043, 448),
  INST(Vpscatterqd     , VexMr_VM           , V(660F38,A1,_,x,_,0,2,T1S), 0                         , F(WO)|F(VM)|F(Evex)                   , EF(________), 0 , 0 , kFamilyAvx , 90 , 8055, 449),
  INST(Vpscatterqq     , VexMr_VM           , V(660F38,A1,_,x,_,1,3,T1S), 0                         , F(WO)|F(VM)|F(Evex)                   , EF(________), 0 , 0 , kFamilyAvx , 90 , 8067, 450),
  INST(Vpshab          , VexRvmRmv          , V(XOP_M9,98,_,0,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 45 , 8079, 451),
  INST(Vpshad          , VexRvmRmv          , V(XOP_M9,9A,_,0,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 45 , 8086, 451),
  INST(Vpshaq          , VexRvmRmv          , V(XOP_M9,9B,_,0,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 45 , 8093, 451),
  INST(Vpshaw          , VexRvmRmv          , V(XOP_M9,99,_,0,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 45 , 8100, 451),
  INST(Vpshlb          , VexRvmRmv          , V(XOP_M9,94,_,0,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 45 , 8107, 451),
  INST(Vpshld          , VexRvmRmv          , V(XOP_M9,96,_,0,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 45 , 8114, 451),
  INST(Vpshlq          , VexRvmRmv          , V(XOP_M9,97,_,0,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 45 , 8121, 451),
  INST(Vpshlw          , VexRvmRmv          , V(XOP_M9,95,_,0,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 45 , 8128, 451),
  INST(Vpshufb         , VexRvm_Lx          , V(660F38,00,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 56 , 8135, 288),
  INST(Vpshufd         , VexRmi_Lx          , V(660F00,70,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 58 , 8143, 452),
  INST(Vpshufhw        , VexRmi_Lx          , V(F30F00,70,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 56 , 8151, 452),
  INST(Vpshuflw        , VexRmi_Lx          , V(F20F00,70,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 56 , 8160, 452),
  INST(Vpsignb         , VexRvm_Lx          , V(660F38,08,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 52 , 8169, 291),
  INST(Vpsignd         , VexRvm_Lx          , V(660F38,0A,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 52 , 8177, 291),
  INST(Vpsignw         , VexRvm_Lx          , V(660F38,09,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 52 , 8185, 291),
  INST(Vpslld          , VexRvmVmi_Lx       , V(660F00,F2,_,x,I,0,4,128), V(660F00,72,6,x,I,0,4,FV ), F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 58 , 8193, 453),
  INST(Vpslldq         , VexEvexVmi_Lx      , V(660F00,73,7,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 89 , 8200, 452),
  INST(Vpsllq          , VexRvmVmi_Lx       , V(660F00,F3,_,x,I,1,4,128), V(660F00,73,6,x,I,1,4,FV ), F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 59 , 8208, 454),
  INST(Vpsllvd         , VexRvm_Lx          , V(660F38,47,_,x,0,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 74 , 8215, 288),
  INST(Vpsllvq         , VexRvm_Lx          , V(660F38,47,_,x,1,1,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 76 , 8223, 288),
  INST(Vpsllvw         , VexRvm_Lx          , V(660F38,12,_,x,_,1,4,FVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 10 , 8231, 297),
  INST(Vpsllw          , VexRvmVmi_Lx       , V(660F00,F1,_,x,I,I,4,FVM), V(660F00,71,6,x,I,I,4,FVM), F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 56 , 8239, 455),
  INST(Vpsrad          , VexRvmVmi_Lx       , V(660F00,E2,_,x,I,0,4,128), V(660F00,72,4,x,I,0,4,FV ), F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 58 , 8246, 456),
  INST(Vpsraq          , VexRvmVmi_Lx       , V(660F00,E2,_,x,_,1,4,128), V(660F00,72,4,x,_,1,4,FV ), F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 7  , 8253, 457),
  INST(Vpsravd         , VexRvm_Lx          , V(660F38,46,_,x,0,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 74 , 8260, 288),
  INST(Vpsravq         , VexRvm_Lx          , V(660F38,46,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 7  , 8268, 297),
  INST(Vpsravw         , VexRvm_Lx          , V(660F38,11,_,x,_,1,4,FVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 10 , 8276, 297),
  INST(Vpsraw          , VexRvmVmi_Lx       , V(660F00,E1,_,x,I,I,4,128), V(660F00,71,4,x,I,I,4,FVM), F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 56 , 8284, 458),
  INST(Vpsrld          , VexRvmVmi_Lx       , V(660F00,D2,_,x,I,0,4,128), V(660F00,72,2,x,I,0,4,FV ), F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 58 , 8291, 459),
  INST(Vpsrldq         , VexEvexVmi_Lx      , V(660F00,73,3,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 89 , 8298, 452),
  INST(Vpsrlq          , VexRvmVmi_Lx       , V(660F00,D3,_,x,I,1,4,128), V(660F00,73,2,x,I,1,4,FV ), F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 59 , 8306, 460),
  INST(Vpsrlvd         , VexRvm_Lx          , V(660F38,45,_,x,0,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 74 , 8313, 288),
  INST(Vpsrlvq         , VexRvm_Lx          , V(660F38,45,_,x,1,1,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 76 , 8321, 288),
  INST(Vpsrlvw         , VexRvm_Lx          , V(660F38,10,_,x,_,1,4,FVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 10 , 8329, 297),
  INST(Vpsrlw          , VexRvmVmi_Lx       , V(660F00,D1,_,x,I,I,4,128), V(660F00,71,2,x,I,I,4,FVM), F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 56 , 8337, 461),
  INST(Vpsubb          , VexRvm_Lx          , V(660F00,F8,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 56 , 8344, 462),
  INST(Vpsubd          , VexRvm_Lx          , V(660F00,FA,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 58 , 8351, 462),
  INST(Vpsubq          , VexRvm_Lx          , V(660F00,FB,_,x,I,1,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 59 , 8358, 462),
  INST(Vpsubsb         , VexRvm_Lx          , V(660F00,E8,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 56 , 8365, 462),
  INST(Vpsubsw         , VexRvm_Lx          , V(660F00,E9,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 56 , 8373, 462),
  INST(Vpsubusb        , VexRvm_Lx          , V(660F00,D8,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 56 , 8381, 462),
  INST(Vpsubusw        , VexRvm_Lx          , V(660F00,D9,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 56 , 8390, 462),
  INST(Vpsubw          , VexRvm_Lx          , V(660F00,F9,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 56 , 8399, 462),
  INST(Vpternlogd      , VexRvmi_Lx         , V(660F3A,25,_,x,_,0,4,FV ), 0                         , F(RW)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 6  , 8406, 334),
  INST(Vpternlogq      , VexRvmi_Lx         , V(660F3A,25,_,x,_,1,4,FV ), 0                         , F(RW)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 7  , 8417, 334),
  INST(Vptest          , VexRm_Lx           , V(660F38,17,_,x,I,_,_,_  ), 0                         , F(RO)|F(Vex)                          , EF(WWWWWW__), 0 , 0 , kFamilyAvx , 4  , 8428, 463),
  INST(Vptestmb        , VexRvm_Lx          , V(660F38,26,_,x,_,0,4,FVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 65 , 8435, 464),
  INST(Vptestmd        , VexRvm_Lx          , V(660F38,27,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 66 , 8444, 464),
  INST(Vptestmq        , VexRvm_Lx          , V(660F38,27,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 70 , 8453, 464),
  INST(Vptestmw        , VexRvm_Lx          , V(660F38,26,_,x,_,1,4,FVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 65 , 8462, 464),
  INST(Vptestnmb       , VexRvm_Lx          , V(F30F38,26,_,x,_,0,4,FVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 65 , 8471, 464),
  INST(Vptestnmd       , VexRvm_Lx          , V(F30F38,27,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 66 , 8481, 464),
  INST(Vptestnmq       , VexRvm_Lx          , V(F30F38,27,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 70 , 8491, 464),
  INST(Vptestnmw       , VexRvm_Lx          , V(F30F38,26,_,x,_,1,4,FVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 65 , 8501, 464),
  INST(Vpunpckhbw      , VexRvm_Lx          , V(660F00,68,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 56 , 8511, 288),
  INST(Vpunpckhdq      , VexRvm_Lx          , V(660F00,6A,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 58 , 8522, 288),
  INST(Vpunpckhqdq     , VexRvm_Lx          , V(660F00,6D,_,x,I,1,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 59 , 8533, 288),
  INST(Vpunpckhwd      , VexRvm_Lx          , V(660F00,69,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 56 , 8545, 288),
  INST(Vpunpcklbw      , VexRvm_Lx          , V(660F00,60,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 56 , 8556, 288),
  INST(Vpunpckldq      , VexRvm_Lx          , V(660F00,62,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 58 , 8567, 288),
  INST(Vpunpcklqdq     , VexRvm_Lx          , V(660F00,6C,_,x,I,1,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 59 , 8578, 288),
  INST(Vpunpcklwd      , VexRvm_Lx          , V(660F00,61,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 56 , 8590, 288),
  INST(Vpxor           , VexRvm_Lx          , V(660F00,EF,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 52 , 8601, 399),
  INST(Vpxord          , VexRvm_Lx          , V(660F00,EF,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 6  , 8607, 400),
  INST(Vpxorq          , VexRvm_Lx          , V(660F00,EF,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 7  , 8614, 400),
  INST(Vrangepd        , VexRvmi_Lx         , V(660F3A,50,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 30 , 8621, 295),
  INST(Vrangeps        , VexRvmi_Lx         , V(660F3A,50,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 31 , 8630, 295),
  INST(Vrangesd        , VexRvmi            , V(660F3A,51,_,I,_,1,3,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 91 , 8639, 465),
  INST(Vrangess        , VexRvmi            , V(660F3A,51,_,I,_,0,2,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 91 , 8648, 466),
  INST(Vrcp14pd        , VexRm_Lx           , V(660F38,4C,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 7  , 8657, 318),
  INST(Vrcp14ps        , VexRm_Lx           , V(660F38,4C,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 6  , 8666, 318),
  INST(Vrcp14sd        , VexRvm             , V(660F38,4D,_,I,_,1,3,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 12 , 8675, 467),
  INST(Vrcp14ss        , VexRvm             , V(660F38,4D,_,I,_,0,2,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 12 , 8684, 468),
  INST(Vrcp28pd        , VexRm              , V(660F38,CA,_,2,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 34 , 8693, 329),
  INST(Vrcp28ps        , VexRm              , V(660F38,CA,_,2,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 35 , 8702, 329),
  INST(Vrcp28sd        , VexRvm             , V(660F38,CB,_,I,_,1,3,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 92 , 8711, 467),
  INST(Vrcp28ss        , VexRvm             , V(660F38,CB,_,I,_,0,2,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 92 , 8720, 468),
  INST(Vrcpps          , VexRm_Lx           , V(000F00,53,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 4  , 8729, 346),
  INST(Vrcpss          , VexRvm             , V(F30F00,53,_,I,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 4  , 8736, 469),
  INST(Vreducepd       , VexRmi_Lx          , V(660F3A,56,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 85 , 8743, 358),
  INST(Vreduceps       , VexRmi_Lx          , V(660F3A,56,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 93 , 8753, 358),
  INST(Vreducesd       , VexRvmi            , V(660F3A,57,_,I,_,1,3,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 13 , 8763, 465),
  INST(Vreducess       , VexRvmi            , V(660F3A,57,_,I,_,0,2,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 13 , 8773, 466),
  INST(Vrndscalepd     , VexRmi_Lx          , V(660F3A,09,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 29 , 8783, 358),
  INST(Vrndscaleps     , VexRmi_Lx          , V(660F3A,08,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 32 , 8795, 358),
  INST(Vrndscalesd     , VexRvmi            , V(660F3A,0B,_,I,_,1,3,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 37 , 8807, 465),
  INST(Vrndscaless     , VexRvmi            , V(660F3A,0A,_,I,_,0,2,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 37 , 8819, 466),
  INST(Vroundpd        , VexRmi_Lx          , V(660F3A,09,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 4  , 8831, 470),
  INST(Vroundps        , VexRmi_Lx          , V(660F3A,08,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 4  , 8840, 470),
  INST(Vroundsd        , VexRvmi            , V(660F3A,0B,_,I,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 4  , 8849, 471),
  INST(Vroundss        , VexRvmi            , V(660F3A,0A,_,I,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 4  , 8858, 472),
  INST(Vrsqrt14pd      , VexRm_Lx           , V(660F38,4E,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 7  , 8867, 318),
  INST(Vrsqrt14ps      , VexRm_Lx           , V(660F38,4E,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 6  , 8878, 318),
  INST(Vrsqrt14sd      , VexRvm             , V(660F38,4F,_,I,_,1,3,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 12 , 8889, 467),
  INST(Vrsqrt14ss      , VexRvm             , V(660F38,4F,_,I,_,0,2,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 12 , 8900, 468),
  INST(Vrsqrt28pd      , VexRm              , V(660F38,CC,_,2,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 34 , 8911, 329),
  INST(Vrsqrt28ps      , VexRm              , V(660F38,CC,_,2,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 35 , 8922, 329),
  INST(Vrsqrt28sd      , VexRvm             , V(660F38,CD,_,I,_,1,3,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 92 , 8933, 467),
  INST(Vrsqrt28ss      , VexRvm             , V(660F38,CD,_,I,_,0,2,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 92 , 8944, 468),
  INST(Vrsqrtps        , VexRm_Lx           , V(000F00,52,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 4  , 8955, 346),
  INST(Vrsqrtss        , VexRvm             , V(F30F00,52,_,I,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 4  , 8964, 469),
  INST(Vscalefpd       , VexRvm_Lx          , V(660F38,2C,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 23 , 8973, 297),
  INST(Vscalefps       , VexRvm_Lx          , V(660F38,2C,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 26 , 8983, 297),
  INST(Vscalefsd       , VexRvm             , V(660F38,2D,_,I,_,1,3,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 94 , 8993, 467),
  INST(Vscalefss       , VexRvm             , V(660F38,2D,_,I,_,0,2,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 94 , 9003, 468),
  INST(Vscatterdpd     , VexMr_Lx           , V(660F38,A2,_,x,_,1,3,T1S), 0                         , F(WO)|F(VM)|F(Evex)                   , EF(________), 0 , 0 , kFamilyAvx , 90 , 9013, 473),
  INST(Vscatterdps     , VexMr_Lx           , V(660F38,A2,_,x,_,0,2,T1S), 0                         , F(WO)|F(VM)|F(Evex)                   , EF(________), 0 , 0 , kFamilyAvx , 90 , 9025, 448),
  INST(Vscatterpf0dpd  , VexM_VM            , V(660F38,C6,5,2,_,1,3,T1S), 0                         , F(RO)|F(VM)|F(Evex)                   , EF(________), 0 , 0 , kFamilyNone, 0  , 9037, 351),
  INST(Vscatterpf0dps  , VexM_VM            , V(660F38,C6,5,2,_,0,2,T1S), 0                         , F(RO)|F(VM)|F(Evex)                   , EF(________), 0 , 0 , kFamilyNone, 0  , 9052, 352),
  INST(Vscatterpf0qpd  , VexM_VM            , V(660F38,C7,5,2,_,1,3,T1S), 0                         , F(RO)|F(VM)|F(Evex)                   , EF(________), 0 , 0 , kFamilyNone, 0  , 9067, 353),
  INST(Vscatterpf0qps  , VexM_VM            , V(660F38,C7,5,2,_,0,2,T1S), 0                         , F(RO)|F(VM)|F(Evex)                   , EF(________), 0 , 0 , kFamilyNone, 0  , 9082, 353),
  INST(Vscatterpf1dpd  , VexM_VM            , V(660F38,C6,6,2,_,1,3,T1S), 0                         , F(RO)|F(VM)|F(Evex)                   , EF(________), 0 , 0 , kFamilyNone, 0  , 9097, 351),
  INST(Vscatterpf1dps  , VexM_VM            , V(660F38,C6,6,2,_,0,2,T1S), 0                         , F(RO)|F(VM)|F(Evex)                   , EF(________), 0 , 0 , kFamilyNone, 0  , 9112, 352),
  INST(Vscatterpf1qpd  , VexM_VM            , V(660F38,C7,6,2,_,1,3,T1S), 0                         , F(RO)|F(VM)|F(Evex)                   , EF(________), 0 , 0 , kFamilyNone, 0  , 9127, 353),
  INST(Vscatterpf1qps  , VexM_VM            , V(660F38,C7,6,2,_,0,2,T1S), 0                         , F(RO)|F(VM)|F(Evex)                   , EF(________), 0 , 0 , kFamilyNone, 0  , 9142, 353),
  INST(Vscatterqpd     , VexMr_Lx           , V(660F38,A3,_,x,_,1,3,T1S), 0                         , F(WO)|F(VM)|F(Evex)                   , EF(________), 0 , 0 , kFamilyAvx , 90 , 9157, 450),
  INST(Vscatterqps     , VexMr_Lx           , V(660F38,A3,_,x,_,0,2,T1S), 0                         , F(WO)|F(VM)|F(Evex)                   , EF(________), 0 , 0 , kFamilyAvx , 90 , 9169, 449),
  INST(Vshuff32x4      , VexRvmi_Lx         , V(660F3A,23,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 6  , 9181, 474),
  INST(Vshuff64x2      , VexRvmi_Lx         , V(660F3A,23,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 7  , 9192, 474),
  INST(Vshufi32x4      , VexRvmi_Lx         , V(660F3A,43,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 6  , 9203, 474),
  INST(Vshufi64x2      , VexRvmi_Lx         , V(660F3A,43,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 7  , 9214, 474),
  INST(Vshufpd         , VexRvmi_Lx         , V(660F00,C6,_,x,I,1,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 21 , 9225, 396),
  INST(Vshufps         , VexRvmi_Lx         , V(000F00,C6,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 75 , 9233, 396),
  INST(Vsqrtpd         , VexRm_Lx           , V(660F00,51,_,x,I,1,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 1  , 9241, 315),
  INST(Vsqrtps         , VexRm_Lx           , V(000F00,51,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 2  , 9249, 315),
  INST(Vsqrtsd         , VexRvm             , V(F20F00,51,_,I,I,1,3,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 3  , 9257, 289),
  INST(Vsqrtss         , VexRvm             , V(F30F00,51,_,I,I,0,2,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 3  , 9265, 290),
  INST(Vstmxcsr        , VexM               , V(000F00,AE,3,0,I,_,_,_  ), 0                         , F(Vex)|F(Volatile)                    , EF(________), 0 , 0 , kFamilyNone, 0  , 9273, 475),
  INST(Vsubpd          , VexRvm_Lx          , V(660F00,5C,_,x,I,1,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 53 , 9282, 288),
  INST(Vsubps          , VexRvm_Lx          , V(000F00,5C,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 54 , 9289, 288),
  INST(Vsubsd          , VexRvm             , V(F20F00,5C,_,I,I,1,3,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 3  , 9296, 289),
  INST(Vsubss          , VexRvm             , V(F30F00,5C,_,I,I,0,2,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 3  , 9303, 290),
  INST(Vtestpd         , VexRm_Lx           , V(660F38,0F,_,x,0,_,_,_  ), 0                         , F(RO)|F(Vex)                          , EF(WWWWWW__), 0 , 0 , kFamilyAvx , 4  , 9310, 463),
  INST(Vtestps         , VexRm_Lx           , V(660F38,0E,_,x,0,_,_,_  ), 0                         , F(RO)|F(Vex)                          , EF(WWWWWW__), 0 , 0 , kFamilyAvx , 4  , 9318, 463),
  INST(Vucomisd        , VexRm              , V(660F00,2E,_,I,I,1,3,T1S), 0                         , F(RO)|F(Vex)|F(Evex)                  , EF(WWWWWW__), 0 , 0 , kFamilyAvx , 20 , 9326, 311),
  INST(Vucomiss        , VexRm              , V(000F00,2E,_,I,I,0,2,T1S), 0                         , F(RO)|F(Vex)|F(Evex)                  , EF(WWWWWW__), 0 , 0 , kFamilyAvx , 20 , 9335, 312),
  INST(Vunpckhpd       , VexRvm_Lx          , V(660F00,15,_,x,I,1,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 75 , 9344, 288),
  INST(Vunpckhps       , VexRvm_Lx          , V(000F00,15,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 21 , 9354, 288),
  INST(Vunpcklpd       , VexRvm_Lx          , V(660F00,14,_,x,I,1,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 75 , 9364, 288),
  INST(Vunpcklps       , VexRvm_Lx          , V(000F00,14,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 21 , 9374, 288),
  INST(Vxorpd          , VexRvm_Lx          , V(660F00,57,_,x,I,1,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 8  , 9384, 462),
  INST(Vxorps          , VexRvm_Lx          , V(000F00,57,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 9  , 9391, 462),
  INST(Vzeroall        , VexOp              , V(000F00,77,_,1,I,_,_,_  ), 0                         , F(Vex)|F(Volatile)                    , EF(________), 0 , 0 , kFamilyNone, 0  , 9398, 476),
  INST(Vzeroupper      , VexOp              , V(000F00,77,_,0,I,_,_,_  ), 0                         , F(Vex)|F(Volatile)                    , EF(________), 0 , 0 , kFamilyNone, 0  , 9407, 476),
  INST(Wbinvd          , X86Op              , O(000F00,09,_,_,_,_,_,_  ), 0                         , 0                                     , EF(________), 0 , 0 , kFamilyNone, 0  , 9418, 41 ),
  INST(Wrfsbase        , X86M               , O(F30F00,AE,2,_,x,_,_,_  ), 0                         , F(RO)|F(Volatile)                     , EF(________), 0 , 0 , kFamilyNone, 0  , 9425, 477),
  INST(Wrgsbase        , X86M               , O(F30F00,AE,3,_,x,_,_,_  ), 0                         , F(RO)|F(Volatile)                     , EF(________), 0 , 0 , kFamilyNone, 0  , 9434, 477),
  INST(Wrmsr           , X86Op              , O(000F00,30,_,_,_,_,_,_  ), 0                         , F(RO)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 9443, 478),
  INST(Xadd            , X86Xadd            , O(000F00,C0,_,_,x,_,_,_  ), 0                         , F(RW)|F(Xchg)|F(Lock)                 , EF(WWWWWW__), 0 , 0 , kFamilyNone, 0  , 9449, 479),
  INST(Xchg            , X86Xchg            , O(000000,86,_,_,x,_,_,_  ), 0                         , F(RW)|F(Xchg)|F(Lock)                 , EF(________), 0 , 0 , kFamilyNone, 0  , 434 , 480),
  INST(Xgetbv          , X86Op              , O(000F01,D0,_,_,_,_,_,_  ), 0                         , F(WO)|F(Special)                      , EF(________), 0 , 0 , kFamilyNone, 0  , 9454, 481),
  INST(Xor             , X86Arith           , O(000000,30,6,_,x,_,_,_  ), 0                         , F(RW)|F(Lock)                         , EF(WWWUWW__), 0 , 0 , kFamilyNone, 0  , 8603, 284),
  INST(Xorpd           , ExtRm              , O(660F00,57,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 149, 9385, 209),
  INST(Xorps           , ExtRm              , O(000F00,57,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 150, 9392, 209),
  INST(Xrstor          , X86M_Only          , O(000F00,AE,5,_,_,_,_,_  ), 0                         , F(RO)|F(Volatile)|F(Special)          , EF(________), 0 , 0 , kFamilyNone, 0  , 1105, 482),
  INST(Xrstor64        , X86M_Only          , O(000F00,AE,5,_,1,_,_,_  ), 0                         , F(RO)|F(Volatile)|F(Special)          , EF(________), 0 , 0 , kFamilyNone, 0  , 1113, 483),
  INST(Xrstors         , X86M_Only          , O(000F00,C7,3,_,_,_,_,_  ), 0                         , F(RO)|F(Volatile)|F(Special)          , EF(________), 0 , 0 , kFamilyNone, 0  , 9461, 482),
  INST(Xrstors64       , X86M_Only          , O(000F00,C7,3,_,1,_,_,_  ), 0                         , F(RO)|F(Volatile)|F(Special)          , EF(________), 0 , 0 , kFamilyNone, 0  , 9469, 483),
  INST(Xsave           , X86M_Only          , O(000F00,AE,4,_,_,_,_,_  ), 0                         , F(WO)|F(Volatile)|F(Special)          , EF(________), 0 , 0 , kFamilyNone, 0  , 1123, 484),
  INST(Xsave64         , X86M_Only          , O(000F00,AE,4,_,1,_,_,_  ), 0                         , F(WO)|F(Volatile)|F(Special)          , EF(________), 0 , 0 , kFamilyNone, 0  , 1130, 485),
  INST(Xsavec          , X86M_Only          , O(000F00,C7,4,_,_,_,_,_  ), 0                         , F(WO)|F(Volatile)|F(Special)          , EF(________), 0 , 0 , kFamilyNone, 0  , 9479, 484),
  INST(Xsavec64        , X86M_Only          , O(000F00,C7,4,_,1,_,_,_  ), 0                         , F(WO)|F(Volatile)|F(Special)          , EF(________), 0 , 0 , kFamilyNone, 0  , 9486, 485),
  INST(Xsaveopt        , X86M_Only          , O(000F00,AE,6,_,_,_,_,_  ), 0                         , F(WO)|F(Volatile)|F(Special)          , EF(________), 0 , 0 , kFamilyNone, 0  , 9495, 484),
  INST(Xsaveopt64      , X86M_Only          , O(000F00,AE,6,_,1,_,_,_  ), 0                         , F(WO)|F(Volatile)|F(Special)          , EF(________), 0 , 0 , kFamilyNone, 0  , 9504, 485),
  INST(Xsaves          , X86M_Only          , O(000F00,C7,5,_,_,_,_,_  ), 0                         , F(WO)|F(Volatile)|F(Special)          , EF(________), 0 , 0 , kFamilyNone, 0  , 9515, 484),
  INST(Xsaves64        , X86M_Only          , O(000F00,C7,5,_,1,_,_,_  ), 0                         , F(WO)|F(Volatile)|F(Special)          , EF(________), 0 , 0 , kFamilyNone, 0  , 9522, 485),
  INST(Xsetbv          , X86Op              , O(000F01,D1,_,_,_,_,_,_  ), 0                         , F(RO)|F(Volatile)|F(Special)          , EF(________), 0 , 0 , kFamilyNone, 0  , 9531, 486)
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
  { F(RW)                                 , 0  , 0  , 0x00, 0x3F, 0  , 357, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #1
  { F(RW)                                 , 0  , 0  , 0x00, 0x3F, 0  , 358, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #2
  { F(RW)|F(Lock)                         , 0  , 0  , 0x20, 0x3F, 0  , 14 , 10, JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #3
  { F(RW)                                 , 0  , 0  , 0x20, 0x20, 0  , 22 , 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #4
  { F(RW)|F(Lock)                         , 0  , 0  , 0x00, 0x3F, 0  , 14 , 10, JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #5
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 0  , 306, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #6
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 0  , 359, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #7
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 0  , 360, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #8
  { F(RW)                                 , 0  , 0  , 0x01, 0x01, 0  , 22 , 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #9
  { F(WO)                                 , 0  , 0  , 0x00, 0x00, 0  , 64 , 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #10
  { F(WO)                                 , 0  , 0  , 0x00, 0x00, 0  , 71 , 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #11
  { F(RW)|F(Lock)                         , 0  , 0  , 0x00, 0x3F, 0  , 14 , 10, JUMP_TYPE(None)       , SINGLE_REG(RO)  , 0 }, // #12
  { F(RW)                                 , 0  , 0  , 0x00, 0x3F, 0  , 247, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #13
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 0  , 306, 1 , JUMP_TYPE(None)       , SINGLE_REG(RO)  , 0 }, // #14
  { F(RW)                                 , 0  , 0  , 0x00, 0x04, 0  , 361, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #15
  { F(RW)                                 , 0  , 0  , 0x00, 0x3F, 0  , 249, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #16
  { F(WO)                                 , 0  , 0  , 0x00, 0x3F, 0  , 158, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #17
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 0  , 308, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #18
  { F(RW)|F(Special)                      , 0  , 0  , 0x00, 0x00, 0  , 362, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #19
  { F(RW)                                 , 0  , 0  , 0x00, 0x3F, 0  , 158, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #20
  { F(WO)                                 , 0  , 0  , 0x00, 0x00, 0  , 251, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #21
  { F(WO)                                 , 0  , 0  , 0x00, 0x00, 0  , 363, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #22
  { F(WO)                                 , 0  , 0  , 0x00, 0x00, 0  , 364, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #23
  { F(WO)                                 , 0  , 0  , 0x00, 0x00, 1  , 253, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #24
  { F(WO)                                 , 0  , 0  , 0x00, 0x00, 0  , 365, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #25
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 0  , 255, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #26
  { F(RW)                                 , 0  , 0  , 0x00, 0x3F, 0  , 21 , 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #27
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 0  , 366, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #28
  { F(RO)                                 , 0  , 0  , 0x00, 0x3B, 2  , 148, 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #29
  { F(RW)|F(Lock)                         , 0  , 0  , 0x00, 0x3B, 3  , 151, 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #30
  { F(RW)|F(Lock)                         , 0  , 0  , 0x00, 0x3B, 4  , 151, 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #31
  { F(RW)|F(Lock)                         , 0  , 0  , 0x00, 0x3B, 5  , 151, 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #32
  { F(RW)|F(Volatile)                     , 0  , 0  , 0x00, 0x00, 0  , 257, 2 , JUMP_TYPE(Call)       , SINGLE_REG(None), 0 }, // #33
  { F(RW)|F(Special)                      , 0  , 0  , 0x00, 0x00, 0  , 367, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #34
  { F(RW)|F(Special)                      , 0  , 0  , 0x00, 0x00, 0  , 368, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #35
  { F(RW)|F(Special)                      , 0  , 0  , 0x00, 0x00, 0  , 369, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #36
  { F(Volatile)                           , 0  , 0  , 0x00, 0x08, 0  , 265, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #37
  { F(Volatile)                           , 0  , 0  , 0x00, 0x20, 0  , 265, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #38
  { F(Volatile)                           , 0  , 0  , 0x00, 0x40, 0  , 265, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #39
  { F(RO)|F(Volatile)                     , 0  , 0  , 0x00, 0x00, 0  , 370, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #40
  { 0                                     , 0  , 0  , 0x00, 0x00, 0  , 265, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #41
  { F(WO)|F(Volatile)|F(Special)          , 0  , 0  , 0x00, 0x00, 0  , 371, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #42
  { 0                                     , 0  , 0  , 0x20, 0x20, 0  , 265, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #43
  { F(RW)                                 , 0  , 0  , 0x24, 0x00, 0  , 21 , 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #44
  { F(RW)                                 , 0  , 0  , 0x20, 0x00, 0  , 21 , 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #45
  { F(RW)                                 , 0  , 0  , 0x04, 0x00, 0  , 21 , 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #46
  { F(RW)                                 , 0  , 0  , 0x07, 0x00, 0  , 21 , 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #47
  { F(RW)                                 , 0  , 0  , 0x03, 0x00, 0  , 21 , 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #48
  { F(RW)                                 , 0  , 0  , 0x01, 0x00, 0  , 21 , 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #49
  { F(RW)                                 , 0  , 0  , 0x10, 0x00, 0  , 21 , 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #50
  { F(RW)                                 , 0  , 0  , 0x02, 0x00, 0  , 21 , 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #51
  { F(RO)                                 , 0  , 0  , 0x00, 0x3F, 0  , 24 , 10, JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #52
  { F(RW)|F(Special)|F(Rep)|F(Repnz)      , 0  , 0  , 0x40, 0x3F, 0  , 372, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #53
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 0  , 373, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #54
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 0  , 374, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #55
  { F(RW)|F(Lock)|F(Special)              , 0  , 0  , 0x00, 0x3F, 0  , 108, 4 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #56
  { F(RW)|F(Lock)|F(Special)              , 0  , 0  , 0x00, 0x04, 0  , 375, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #57
  { F(RW)|F(Lock)|F(Special)              , 0  , 0  , 0x00, 0x04, 0  , 376, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #58
  { F(RO)                                 , 0  , 0  , 0x00, 0x3F, 0  , 377, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #59
  { F(RO)                                 , 0  , 0  , 0x00, 0x3F, 0  , 378, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #60
  { F(RW)|F(Volatile)|F(Special)          , 0  , 0  , 0x00, 0x00, 0  , 379, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #61
  { F(RW)|F(Special)                      , 0  , 0  , 0x00, 0x00, 0  , 380, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #62
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 0  , 259, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #63
  { F(WO)                                 , 0  , 16 , 0x00, 0x00, 0  , 62 , 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #64
  { F(WO)                                 , 0  , 16 , 0x00, 0x00, 0  , 64 , 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #65
  { F(WO)                                 , 0  , 8  , 0x00, 0x00, 0  , 381, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #66
  { F(WO)                                 , 0  , 16 , 0x00, 0x00, 0  , 382, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #67
  { F(WO)                                 , 0  , 8  , 0x00, 0x00, 0  , 382, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #68
  { F(WO)                                 , 0  , 8  , 0x00, 0x00, 0  , 383, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #69
  { F(WO)                                 , 0  , 8  , 0x00, 0x00, 0  , 384, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #70
  { F(WO)                                 , 0  , 4  , 0x00, 0x00, 0  , 62 , 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #71
  { F(WO)                                 , 0  , 8  , 0x00, 0x00, 0  , 385, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #72
  { F(WO)                                 , 0  , 4  , 0x00, 0x00, 0  , 385, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #73
  { F(WO)                                 , 0  , 8  , 0x00, 0x00, 0  , 229, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #74
  { F(WO)                                 , 0  , 8  , 0x00, 0x00, 0  , 321, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #75
  { F(RW)|F(Special)                      , 0  , 0  , 0x00, 0x00, 0  , 386, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #76
  { F(RW)|F(Special)                      , 0  , 0  , 0x00, 0x00, 0  , 387, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #77
  { F(RW)|F(Special)                      , 0  , 0  , 0x28, 0x3F, 0  , 357, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #78
  { F(RW)|F(Lock)                         , 0  , 0  , 0x00, 0x1F, 6  , 261, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #79
  { F(RW)|F(Special)                      , 0  , 0  , 0x00, 0x3F, 0  , 112, 4 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #80
  { F(Volatile)                           , 0  , 0  , 0x00, 0x00, 0  , 265, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #81
  { F(Volatile)|F(Special)                , 0  , 0  , 0x00, 0x00, 0  , 388, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #82
  { F(WO)                                 , 0  , 8  , 0x00, 0x00, 0  , 389, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #83
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 7  , 263, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #84
  { F(Fp)                                 , 0  , 0  , 0x00, 0x00, 0  , 265, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #85
  { F(Fp)|F(FPU_M4)|F(FPU_M8)             , 0  , 0  , 0x00, 0x00, 0  , 154, 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #86
  { F(Fp)                                 , 0  , 0  , 0x00, 0x00, 0  , 265, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #87
  { F(Fp)                                 , 0  , 0  , 0x00, 0x00, 0  , 390, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #88
  { F(Fp)                                 , 0  , 0  , 0x20, 0x00, 0  , 266, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #89
  { F(Fp)                                 , 0  , 0  , 0x24, 0x00, 0  , 266, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #90
  { F(Fp)                                 , 0  , 0  , 0x04, 0x00, 0  , 266, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #91
  { F(Fp)                                 , 0  , 0  , 0x10, 0x00, 0  , 266, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #92
  { F(Fp)                                 , 0  , 0  , 0x00, 0x00, 0  , 267, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #93
  { F(Fp)                                 , 0  , 0  , 0x00, 0x3F, 0  , 266, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #94
  { F(Fp)                                 , 0  , 0  , 0x00, 0x00, 0  , 266, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #95
  { F(Fp)|F(FPU_M2)|F(FPU_M4)             , 0  , 0  , 0x00, 0x00, 0  , 391, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #96
  { F(Fp)|F(FPU_M2)|F(FPU_M4)|F(FPU_M8)   , 0  , 0  , 0x00, 0x00, 8  , 392, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #97
  { F(Fp)|F(FPU_M2)|F(FPU_M4)|F(FPU_M8)   , 0  , 0  , 0x00, 0x00, 9  , 392, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #98
  { F(Fp)|F(FPU_M2)|F(FPU_M4)|F(FPU_M8)   , 0  , 0  , 0x00, 0x00, 10 , 392, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #99
  { F(Fp)|F(FPU_M2)|F(FPU_M4)|F(FPU_M8)   , 0  , 0  , 0x00, 0x00, 11 , 393, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #100
  { F(Fp)                                 , 0  , 0  , 0x00, 0x00, 0  , 394, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #101
  { F(Fp)                                 , 0  , 0  , 0x00, 0x00, 0  , 395, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #102
  { F(Fp)                                 , 0  , 0  , 0x00, 0x00, 12 , 396, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #103
  { F(Fp)|F(FPU_M4)|F(FPU_M8)             , 0  , 0  , 0x00, 0x00, 0  , 268, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #104
  { F(Fp)|F(FPU_M4)|F(FPU_M8)|F(FPU_M10)  , 0  , 0  , 0x00, 0x00, 13 , 393, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #105
  { F(Fp)                                 , 0  , 0  , 0x00, 0x00, 14 , 396, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #106
  { F(Fp)|F(Volatile)                     , 0  , 0  , 0x00, 0x00, 0  , 265, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #107
  { F(Fp)                                 , 0  , 0  , 0x00, 0x00, 0  , 397, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #108
  { F(RW)|F(Special)                      , 0  , 0  , 0x00, 0x3F, 0  , 34 , 10, JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #109
  { F(WO)|F(Volatile)|F(Special)          , 0  , 0  , 0x00, 0x00, 15 , 398, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #110
  { F(RW)|F(Lock)                         , 0  , 0  , 0x00, 0x1F, 16 , 261, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #111
  { F(WO)|F(Volatile)|F(Special)|F(Rep)   , 0  , 0  , 0x00, 0x00, 0  , 399, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #112
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 17 , 269, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #113
  { F(Volatile)                           , 0  , 0  , 0x00, 0x88, 0  , 400, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #114
  { F(Volatile)                           , 0  , 0  , 0x00, 0x88, 0  , 265, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #115
  { F(RO)                                 , 0  , 0  , 0x00, 0x00, 0  , 370, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #116
  { F(RO)                                 , 0  , 0  , 0x00, 0x00, 0  , 271, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #117
  { F(Volatile)                           , 0  , 0  , 0x24, 0x00, 18 , 401, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #118
  { F(Volatile)                           , 0  , 0  , 0x20, 0x00, 19 , 401, 1 , JUMP_TYPE(Conditional), SINGLE_REG(None), 0 }, // #119
  { F(Volatile)                           , 0  , 0  , 0x20, 0x00, 20 , 401, 1 , JUMP_TYPE(Conditional), SINGLE_REG(None), 0 }, // #120
  { F(Volatile)                           , 0  , 0  , 0x24, 0x00, 21 , 401, 1 , JUMP_TYPE(Conditional), SINGLE_REG(None), 0 }, // #121
  { F(Volatile)                           , 0  , 0  , 0x20, 0x00, 20 , 402, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #122
  { F(Volatile)                           , 0  , 0  , 0x04, 0x00, 22 , 401, 1 , JUMP_TYPE(Conditional), SINGLE_REG(None), 0 }, // #123
  { F(Volatile)|F(Special)                , 0  , 0  , 0x00, 0x00, 23 , 273, 2 , JUMP_TYPE(Conditional), SINGLE_REG(None), 0 }, // #124
  { F(Volatile)                           , 0  , 0  , 0x07, 0x00, 24 , 401, 1 , JUMP_TYPE(Conditional), SINGLE_REG(None), 0 }, // #125
  { F(Volatile)                           , 0  , 0  , 0x03, 0x00, 25 , 401, 1 , JUMP_TYPE(Conditional), SINGLE_REG(None), 0 }, // #126
  { F(Volatile)                           , 0  , 0  , 0x03, 0x00, 26 , 401, 1 , JUMP_TYPE(Conditional), SINGLE_REG(None), 0 }, // #127
  { F(Volatile)                           , 0  , 0  , 0x07, 0x00, 27 , 401, 1 , JUMP_TYPE(Conditional), SINGLE_REG(None), 0 }, // #128
  { F(Volatile)                           , 0  , 0  , 0x00, 0x00, 28 , 275, 2 , JUMP_TYPE(Direct)     , SINGLE_REG(None), 0 }, // #129
  { F(Volatile)                           , 0  , 0  , 0x20, 0x00, 19 , 402, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #130
  { F(Volatile)                           , 0  , 0  , 0x04, 0x00, 29 , 401, 1 , JUMP_TYPE(Conditional), SINGLE_REG(None), 0 }, // #131
  { F(Volatile)                           , 0  , 0  , 0x01, 0x00, 30 , 401, 1 , JUMP_TYPE(Conditional), SINGLE_REG(None), 0 }, // #132
  { F(Volatile)                           , 0  , 0  , 0x10, 0x00, 31 , 401, 1 , JUMP_TYPE(Conditional), SINGLE_REG(None), 0 }, // #133
  { F(Volatile)                           , 0  , 0  , 0x02, 0x00, 32 , 401, 1 , JUMP_TYPE(Conditional), SINGLE_REG(None), 0 }, // #134
  { F(Volatile)                           , 0  , 0  , 0x01, 0x00, 33 , 401, 1 , JUMP_TYPE(Conditional), SINGLE_REG(None), 0 }, // #135
  { F(Volatile)                           , 0  , 0  , 0x10, 0x00, 34 , 401, 1 , JUMP_TYPE(Conditional), SINGLE_REG(None), 0 }, // #136
  { F(Volatile)                           , 0  , 0  , 0x02, 0x00, 35 , 401, 1 , JUMP_TYPE(Conditional), SINGLE_REG(None), 0 }, // #137
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 403, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #138
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 36 , 277, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #139
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 37 , 279, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #140
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 38 , 281, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #141
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 39 , 283, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #142
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 404, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #143
  { F(RO)|F(Vex)                          , 0  , 0  , 0x00, 0x3F, 0  , 405, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #144
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 406, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #145
  { F(RW)|F(Volatile)|F(Special)          , 0  , 0  , 0x3E, 0x00, 0  , 407, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #146
  { F(RW)|F(Volatile)                     , 0  , 0  , 0x00, 0x04, 0  , 285, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #147
  { F(WO)                                 , 0  , 16 , 0x00, 0x00, 0  , 202, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #148
  { F(RO)|F(Volatile)                     , 0  , 0  , 0x00, 0x00, 0  , 408, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #149
  { F(WO)                                 , 0  , 0  , 0x00, 0x00, 0  , 409, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #150
  { F(Volatile)|F(Special)                , 0  , 0  , 0x00, 0x00, 0  , 265, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #151
  { F(RO)|F(Volatile)                     , 0  , 0  , 0x00, 0x00, 0  , 410, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #152
  { F(WO)|F(Special)|F(Rep)               , 0  , 1  , 0x40, 0x00, 0  , 411, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #153
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 40 , 287, 2 , JUMP_TYPE(Conditional), SINGLE_REG(None), 0 }, // #154
  { F(RW)                                 , 0  , 0  , 0x04, 0x00, 41 , 287, 2 , JUMP_TYPE(Conditional), SINGLE_REG(None), 0 }, // #155
  { F(RW)                                 , 0  , 0  , 0x04, 0x00, 42 , 287, 2 , JUMP_TYPE(Conditional), SINGLE_REG(None), 0 }, // #156
  { F(RW)|F(Volatile)                     , 0  , 0  , 0x00, 0x04, 0  , 289, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #157
  { F(RW)                                 , 0  , 0  , 0x00, 0x3F, 0  , 157, 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #158
  { F(RO)|F(Special)                      , 0  , 0  , 0x00, 0x00, 0  , 412, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #159
  { F(RO)|F(Special)                      , 0  , 0  , 0x00, 0x00, 0  , 413, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #160
  { F(RW)|F(Volatile)                     , 0  , 0  , 0x00, 0x00, 0  , 265, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #161
  { F(RO)|F(Volatile)|F(Special)          , 0  , 0  , 0x00, 0x00, 0  , 414, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #162
  { F(WO)                                 , 0  , 0  , 0x00, 0x00, 0  , 0  , 14, JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #163
  { F(WO)                                 , 0  , 16 , 0x00, 0x00, 43 , 64 , 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #164
  { F(WO)                                 , 0  , 16 , 0x00, 0x00, 44 , 64 , 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #165
  { F(WO)                                 , 0  , 0  , 0x00, 0x00, 45 , 52 , 6 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #166
  { F(WO)                                 , 0  , 16 , 0x00, 0x00, 46 , 291, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #167
  { F(WO)                                 , 0  , 8  , 0x00, 0x00, 0  , 415, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #168
  { F(WO)                                 , 0  , 16 , 0x00, 0x00, 47 , 64 , 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #169
  { F(WO)                                 , 0  , 16 , 0x00, 0x00, 48 , 64 , 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #170
  { F(WO)                                 , 0  , 8  , 0x00, 0x00, 0  , 416, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #171
  { F(RW)                                 , 8  , 8  , 0x00, 0x00, 49 , 208, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #172
  { F(RW)                                 , 8  , 8  , 0x00, 0x00, 50 , 208, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #173
  { F(RW)                                 , 8  , 8  , 0x00, 0x00, 0  , 416, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #174
  { F(WO)                                 , 0  , 8  , 0x00, 0x00, 51 , 208, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #175
  { F(WO)                                 , 0  , 8  , 0x00, 0x00, 52 , 208, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #176
  { F(WO)                                 , 0  , 8  , 0x00, 0x00, 0  , 417, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #177
  { F(WO)                                 , 0  , 16 , 0x00, 0x00, 53 , 199, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #178
  { F(WO)                                 , 0  , 8  , 0x00, 0x00, 0  , 56 , 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #179
  { F(WO)                                 , 0  , 16 , 0x00, 0x00, 54 , 199, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #180
  { F(WO)                                 , 0  , 16 , 0x00, 0x00, 55 , 199, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #181
  { F(WO)                                 , 0  , 8  , 0x00, 0x00, 56 , 418, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #182
  { F(WO)                                 , 0  , 8  , 0x00, 0x00, 57 , 208, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #183
  { F(WO)                                 , 0  , 4  , 0x00, 0x00, 58 , 296, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #184
  { F(WO)                                 , 0  , 16 , 0x00, 0x00, 59 , 58 , 6 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #185
  { F(WO)                                 , 0  , 16 , 0x00, 0x00, 0  , 419, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #186
  { F(WO)|F(Special)|F(Rep)               , 0  , 0  , 0x00, 0x00, 0  , 420, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #187
  { F(WO)|F(ZeroIfMem)                    , 0  , 8  , 0x00, 0x00, 60 , 293, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #188
  { F(WO)|F(ZeroIfMem)                    , 0  , 4  , 0x00, 0x00, 61 , 295, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #189
  { F(WO)                                 , 0  , 0  , 0x00, 0x00, 0  , 297, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #190
  { F(WO)                                 , 0  , 0  , 0x00, 0x00, 0  , 421, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #191
  { F(WO)                                 , 0  , 16 , 0x00, 0x00, 62 , 64 , 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #192
  { F(WO)                                 , 0  , 16 , 0x00, 0x00, 63 , 64 , 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #193
  { F(RW)|F(Special)                      , 0  , 0  , 0x00, 0x3F, 0  , 34 , 4 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #194
  { F(RW)|F(Special)                      , 0  , 0  , 0x00, 0x00, 0  , 299, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #195
  { F(RO)|F(Volatile)|F(Special)          , 0  , 0  , 0x00, 0x00, 0  , 422, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #196
  { F(RW)|F(Lock)                         , 0  , 0  , 0x00, 0x3F, 0  , 262, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #197
  { 0                                     , 0  , 0  , 0x00, 0x00, 0  , 301, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #198
  { F(RW)|F(Lock)                         , 0  , 0  , 0x00, 0x00, 0  , 262, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #199
  { F(RO)|F(Volatile)|F(Special)          , 0  , 0  , 0x00, 0x00, 64 , 423, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #200
  { F(RO)|F(Volatile)|F(Special)|F(Rep)   , 0  , 0  , 0x00, 0x00, 0  , 424, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #201
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 0  , 303, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #202
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 0  , 305, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #203
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 0  , 307, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #204
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 0  , 305, 2 , JUMP_TYPE(None)       , SINGLE_REG(RO)  , 0 }, // #205
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 0  , 305, 2 , JUMP_TYPE(None)       , SINGLE_REG(WO)  , 0 }, // #206
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 0  , 265, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #207
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 0  , 305, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #208
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 0  , 306, 1 , JUMP_TYPE(None)       , SINGLE_REG(WO)  , 0 }, // #209
  { F(WO)|F(Special)                      , 0  , 0  , 0x00, 0x00, 0  , 425, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #210
  { F(WO)|F(Special)                      , 0  , 0  , 0x00, 0x00, 0  , 426, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #211
  { F(WO)|F(Special)                      , 0  , 0  , 0x00, 0x00, 0  , 427, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #212
  { F(WO)|F(Special)                      , 0  , 0  , 0x00, 0x00, 0  , 428, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #213
  { F(WO)                                 , 0  , 0  , 0x00, 0x00, 0  , 247, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #214
  { F(WO)                                 , 0  , 8  , 0x00, 0x00, 0  , 429, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #215
  { F(WO)                                 , 0  , 8  , 0x00, 0x00, 0  , 430, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #216
  { F(WO)                                 , 0  , 8  , 0x00, 0x00, 65 , 309, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #217
  { F(WO)                                 , 0  , 8  , 0x00, 0x00, 0  , 303, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #218
  { F(WO)                                 , 0  , 0  , 0x00, 0x00, 0  , 303, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #219
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 0  , 431, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #220
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 0  , 432, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #221
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 0  , 433, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #222
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 0  , 434, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #223
  { F(WO)                                 , 0  , 8  , 0x00, 0x00, 0  , 435, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #224
  { F(WO)                                 , 0  , 16 , 0x00, 0x00, 0  , 229, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #225
  { F(WO)                                 , 0  , 16 , 0x00, 0x00, 0  , 232, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #226
  { F(WO)|F(Volatile)|F(Special)          , 0  , 0  , 0x00, 0x00, 66 , 116, 4 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #227
  { F(Volatile)|F(Special)                , 0  , 0  , 0x00, 0x00, 0  , 436, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #228
  { F(WO)                                 , 0  , 0  , 0x00, 0x3F, 0  , 157, 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #229
  { F(Volatile)|F(Special)                , 0  , 0  , 0x00, 0xFF, 0  , 265, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #230
  { F(Volatile)|F(Special)                , 0  , 0  , 0x00, 0xFF, 0  , 436, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #231
  { F(Volatile)|F(Special)                , 0  , 0  , 0x00, 0xFF, 0  , 437, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #232
  { F(RO)|F(Volatile)                     , 0  , 0  , 0x00, 0x3F, 0  , 370, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #233
  { F(WO)                                 , 0  , 16 , 0x00, 0x00, 0  , 71 , 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #234
  { F(WO)                                 , 0  , 8  , 0x00, 0x00, 0  , 438, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #235
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 67 , 311, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #236
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 68 , 439, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #237
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 69 , 311, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #238
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 70 , 311, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #239
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 71 , 311, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #240
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 72 , 311, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #241
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 73 , 311, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #242
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 74 , 439, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #243
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 75 , 311, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #244
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 76 , 311, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #245
  { F(RO)                                 , 0  , 0  , 0x00, 0x3F, 0  , 353, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #246
  { F(RO)|F(Volatile)|F(Special)          , 0  , 0  , 0x00, 0x00, 77 , 120, 4 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #247
  { F(Volatile)|F(Special)                , 0  , 0  , 0xFF, 0x00, 0  , 265, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #248
  { F(Volatile)|F(Special)                , 0  , 0  , 0xFF, 0x00, 0  , 436, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #249
  { F(Volatile)|F(Special)                , 0  , 0  , 0xFF, 0x00, 0  , 437, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #250
  { F(RW)|F(Special)                      , 0  , 0  , 0x20, 0x21, 0  , 440, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #251
  { F(WO)                                 , 0  , 4  , 0x00, 0x00, 0  , 229, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #252
  { F(WO)                                 , 0  , 8  , 0x00, 0x00, 0  , 441, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #253
  { F(WO)                                 , 0  , 0  , 0x00, 0x00, 0  , 442, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #254
  { F(WO)                                 , 0  , 8  , 0x00, 0x3F, 0  , 443, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #255
  { F(WO)|F(Volatile)|F(Special)          , 0  , 0  , 0x00, 0x00, 0  , 444, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #256
  { F(WO)|F(Volatile)|F(Special)          , 0  , 0  , 0x00, 0x00, 0  , 445, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #257
  { F(RW)|F(Volatile)|F(Special)          , 0  , 0  , 0x00, 0x00, 0  , 313, 2 , JUMP_TYPE(Return)     , SINGLE_REG(None), 0 }, // #258
  { F(RW)|F(Special)                      , 0  , 0  , 0x00, 0x21, 0  , 440, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #259
  { F(WO)                                 , 0  , 0  , 0x00, 0x00, 0  , 315, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #260
  { F(WO)                                 , 0  , 8  , 0x00, 0x00, 0  , 446, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #261
  { F(WO)                                 , 0  , 4  , 0x00, 0x00, 0  , 447, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #262
  { F(RO)|F(Volatile)|F(Special)          , 0  , 0  , 0x00, 0x3E, 0  , 448, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #263
  { F(RW)|F(Special)                      , 0  , 0  , 0x00, 0x3F, 0  , 440, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #264
  { F(WO)                                 , 0  , 0  , 0x00, 0x00, 0  , 249, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #265
  { F(RW)|F(Special)|F(Rep)|F(Repnz)      , 0  , 0  , 0x40, 0x3F, 0  , 449, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #266
  { F(WO)                                 , 0  , 1  , 0x24, 0x00, 0  , 450, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #267
  { F(WO)                                 , 0  , 1  , 0x20, 0x00, 0  , 450, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #268
  { F(WO)                                 , 0  , 1  , 0x04, 0x00, 0  , 450, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #269
  { F(WO)                                 , 0  , 1  , 0x07, 0x00, 0  , 450, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #270
  { F(WO)                                 , 0  , 1  , 0x03, 0x00, 0  , 450, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #271
  { F(WO)                                 , 0  , 1  , 0x01, 0x00, 0  , 450, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #272
  { F(WO)                                 , 0  , 1  , 0x10, 0x00, 0  , 450, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #273
  { F(WO)                                 , 0  , 1  , 0x02, 0x00, 0  , 450, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #274
  { F(WO)                                 , 0  , 0  , 0x00, 0x00, 0  , 451, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #275
  { F(RW)|F(Special)                      , 0  , 0  , 0x00, 0x3F, 0  , 160, 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #276
  { F(WO)|F(Volatile)                     , 0  , 0  , 0x00, 0x00, 0  , 452, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #277
  { F(WO)                                 , 0  , 8  , 0x00, 0x00, 0  , 62 , 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #278
  { 0                                     , 0  , 0  , 0x00, 0x20, 0  , 265, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #279
  { 0                                     , 0  , 0  , 0x00, 0x40, 0  , 265, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #280
  { 0                                     , 0  , 0  , 0x00, 0x80, 0  , 265, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #281
  { F(Volatile)                           , 0  , 0  , 0x00, 0x00, 0  , 453, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #282
  { F(RW)|F(Special)|F(Rep)               , 0  , 0  , 0x40, 0x00, 0  , 454, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #283
  { F(RW)|F(Lock)                         , 0  , 0  , 0x00, 0x3F, 0  , 14 , 10, JUMP_TYPE(None)       , SINGLE_REG(WO)  , 0 }, // #284
  { 0                                     , 0  , 0  , 0x00, 0x00, 0  , 437, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #285
  { F(RO)                                 , 0  , 0  , 0x00, 0x3F, 78 , 88 , 5 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #286
  { F(RW)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 455, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #287
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 163, 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #288
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 456, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #289
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 457, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #290
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 163, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #291
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 70 , 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #292
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 64 , 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #293
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 71 , 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #294
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 166, 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #295
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 163, 3 , JUMP_TYPE(None)       , SINGLE_REG(RO)  , 0 }, // #296
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 163, 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #297
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 166, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #298
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 317, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #299
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 458, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #300
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 459, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #301
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 460, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #302
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 461, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #303
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 462, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #304
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 234, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #305
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 459, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #306
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 337, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #307
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 169, 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #308
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 463, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #309
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 464, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #310
  { F(RO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x3F, 0  , 377, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #311
  { F(RO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x3F, 0  , 378, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #312
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 172, 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #313
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 175, 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #314
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 178, 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #315
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 319, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #316
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 181, 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #317
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 178, 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #318
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 319, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #319
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 184, 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #320
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 175, 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #321
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 384, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #322
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 384, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #323
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 465, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #324
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 321, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #325
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 323, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #326
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 465, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #327
  { F(RO)                                 , 0  , 0  , 0x00, 0x04, 0  , 466, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #328
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 68 , 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #329
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 185, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #330
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 467, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #331
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 186, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #332
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 389, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #333
  { F(RW)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 187, 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #334
  { F(RW)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 468, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #335
  { F(RW)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 469, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #336
  { F(RW)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 190, 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #337
  { F(RW)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 470, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #338
  { F(RW)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 471, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #339
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 124, 4 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #340
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 325, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #341
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 327, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #342
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 472, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #343
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 473, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #344
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 474, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #345
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 178, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #346
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 62 , 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #347
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 229, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #348
  { F(RW)|F(Vex_VM)|F(Evex)               , 0  , 0  , 0x00, 0x00, 79 , 93 , 5 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #349
  { F(RW)|F(Vex_VM)|F(Evex)               , 0  , 0  , 0x00, 0x00, 80 , 98 , 5 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #350
  { F(RO)|F(VM)|F(Evex)                   , 0  , 0  , 0x00, 0x00, 0  , 475, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #351
  { F(RO)|F(VM)|F(Evex)                   , 0  , 0  , 0x00, 0x00, 0  , 476, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #352
  { F(RO)|F(VM)|F(Evex)                   , 0  , 0  , 0x00, 0x00, 0  , 477, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #353
  { F(RW)|F(Vex_VM)|F(Evex)               , 0  , 0  , 0x00, 0x00, 81 , 103, 5 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #354
  { F(RW)|F(Vex_VM)|F(Evex)               , 0  , 0  , 0x00, 0x00, 82 , 128, 4 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #355
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 62 , 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #356
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 229, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #357
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 193, 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #358
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 446, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #359
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 447, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #360
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 329, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #361
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 329, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #362
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 478, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #363
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 479, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #364
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 202, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #365
  { F(RO)|F(Vex)|F(Volatile)              , 0  , 0  , 0x00, 0x00, 0  , 408, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #366
  { F(RO)|F(Vex)|F(Special)               , 0  , 0  , 0x00, 0x00, 0  , 480, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #367
  { F(RW)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 83 , 132, 4 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #368
  { F(RW)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 84 , 132, 4 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #369
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 85 , 64 , 6 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #370
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 86 , 64 , 6 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #371
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 87 , 331, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #372
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 196, 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #373
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 88 , 64 , 4 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #374
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 89 , 64 , 6 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #375
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 90 , 64 , 6 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #376
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 91 , 64 , 4 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #377
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 92 , 64 , 6 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #378
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 93 , 64 , 6 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #379
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 94 , 64 , 6 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #380
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 95 , 64 , 6 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #381
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 210, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #382
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 96 , 333, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #383
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 97 , 333, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #384
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 98 , 333, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #385
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 99 , 333, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #386
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 481, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #387
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 199, 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #388
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 202, 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #389
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 100, 205, 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #390
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 101, 208, 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #391
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 102, 211, 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #392
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 103, 64 , 6 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #393
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 104, 64 , 6 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #394
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 482, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #395
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 166, 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #396
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 163, 2 , JUMP_TYPE(None)       , SINGLE_REG(RO)  , 0 }, // #397
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 163, 3 , JUMP_TYPE(None)       , SINGLE_REG(RO)  , 0 }, // #398
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 163, 2 , JUMP_TYPE(None)       , SINGLE_REG(WO)  , 0 }, // #399
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 163, 3 , JUMP_TYPE(None)       , SINGLE_REG(WO)  , 0 }, // #400
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 335, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #401
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 337, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #402
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 483, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #403
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 484, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #404
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 339, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #405
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 166, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #406
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 214, 3 , JUMP_TYPE(None)       , SINGLE_REG(WO)  , 0 }, // #407
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 217, 3 , JUMP_TYPE(None)       , SINGLE_REG(WO)  , 0 }, // #408
  { F(WO)|F(Vex)|F(Special)               , 0  , 0  , 0x00, 0x00, 0  , 425, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #409
  { F(WO)|F(Vex)|F(Special)               , 0  , 0  , 0x00, 0x00, 0  , 426, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #410
  { F(WO)|F(Vex)|F(Special)               , 0  , 0  , 0x00, 0x00, 0  , 427, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #411
  { F(WO)|F(Vex)|F(Special)               , 0  , 0  , 0x00, 0x00, 0  , 428, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #412
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 167, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #413
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 141, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #414
  { F(RW)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 190, 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #415
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 136, 4 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #416
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 105, 70 , 6 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #417
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 106, 70 , 6 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #418
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 73 , 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #419
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 72 , 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #420
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 107, 140, 4 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #421
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 429, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #422
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 430, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #423
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 310, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #424
  { F(RW)|F(Vex_VM)|F(Evex)               , 0  , 0  , 0x00, 0x00, 108, 98 , 5 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #425
  { F(RW)|F(Vex_VM)|F(Evex)               , 0  , 0  , 0x00, 0x00, 109, 93 , 5 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #426
  { F(RW)|F(Vex_VM)|F(Evex)               , 0  , 0  , 0x00, 0x00, 110, 128, 4 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #427
  { F(RW)|F(Vex_VM)|F(Evex)               , 0  , 0  , 0x00, 0x00, 111, 103, 5 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #428
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 341, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #429
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 343, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #430
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 345, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #431
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 485, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #432
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 125, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #433
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 112, 132, 4 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #434
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 113, 132, 4 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #435
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 486, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #436
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 220, 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #437
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 223, 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #438
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 226, 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #439
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 229, 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #440
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 232, 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #441
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 235, 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #442
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 124, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #443
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 114, 347, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #444
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 115, 347, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #445
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 116, 347, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #446
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 117, 347, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #447
  { F(WO)|F(VM)|F(Evex)                   , 0  , 0  , 0x00, 0x00, 0  , 238, 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #448
  { F(WO)|F(VM)|F(Evex)                   , 0  , 0  , 0x00, 0x00, 0  , 349, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #449
  { F(WO)|F(VM)|F(Evex)                   , 0  , 0  , 0x00, 0x00, 0  , 241, 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #450
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 351, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #451
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 193, 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #452
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 118, 76 , 6 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #453
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 119, 76 , 6 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #454
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 120, 76 , 6 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #455
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 121, 76 , 6 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #456
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 122, 82 , 6 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #457
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 123, 76 , 6 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #458
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 124, 76 , 6 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #459
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 125, 76 , 6 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #460
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 126, 76 , 6 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #461
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 163, 3 , JUMP_TYPE(None)       , SINGLE_REG(WO)  , 0 }, // #462
  { F(RO)|F(Vex)                          , 0  , 0  , 0x00, 0x3F, 0  , 353, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #463
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 244, 3 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #464
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 487, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #465
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 479, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #466
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 456, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #467
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 457, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #468
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 457, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #469
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 78 , 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #470
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 487, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #471
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 479, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #472
  { F(WO)|F(VM)|F(Evex)                   , 0  , 0  , 0x00, 0x00, 0  , 355, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #473
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 167, 2 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #474
  { F(Vex)|F(Volatile)                    , 0  , 0  , 0x00, 0x00, 0  , 453, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #475
  { F(Vex)|F(Volatile)                    , 0  , 0  , 0x00, 0x00, 0  , 265, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #476
  { F(RO)|F(Volatile)                     , 0  , 0  , 0x00, 0x00, 0  , 488, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #477
  { F(RO)                                 , 0  , 0  , 0x00, 0x00, 0  , 489, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #478
  { F(RW)|F(Xchg)|F(Lock)                 , 0  , 0  , 0x00, 0x3F, 0  , 144, 4 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #479
  { F(RW)|F(Xchg)|F(Lock)                 , 0  , 0  , 0x00, 0x00, 0  , 44 , 8 , JUMP_TYPE(None)       , SINGLE_REG(RO)  , 0 }, // #480
  { F(WO)|F(Special)                      , 0  , 0  , 0x00, 0x00, 0  , 490, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #481
  { F(RO)|F(Volatile)|F(Special)          , 0  , 0  , 0x00, 0x00, 0  , 491, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #482
  { F(RO)|F(Volatile)|F(Special)          , 0  , 0  , 0x00, 0x00, 0  , 492, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #483
  { F(WO)|F(Volatile)|F(Special)          , 0  , 0  , 0x00, 0x00, 0  , 491, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #484
  { F(WO)|F(Volatile)|F(Special)          , 0  , 0  , 0x00, 0x00, 0  , 492, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }, // #485
  { F(RO)|F(Volatile)|F(Special)          , 0  , 0  , 0x00, 0x00, 0  , 493, 1 , JUMP_TYPE(None)       , SINGLE_REG(None), 0 }  // #486
};
#undef SINGLE_REG
#undef JUMP_TYPE
// ----------------------------------------------------------------------------
// ${commonData:End}

// ${altOpCodeData:Begin}
// ------------------- Automatically generated, do not edit -------------------
const uint32_t X86InstDB::altOpCodeData[] = {
  0                         , // #0
  O(660F00,1B,_,_,_,_,_,_  ), // #1
  O(000F00,BA,4,_,x,_,_,_  ), // #2
  O(000F00,BA,7,_,x,_,_,_  ), // #3
  O(000F00,BA,6,_,x,_,_,_  ), // #4
  O(000F00,BA,5,_,x,_,_,_  ), // #5
  O(000000,48,_,_,x,_,_,_  ), // #6
  O(660F00,78,0,_,_,_,_,_  ), // #7
  O_FPU(00,00DF,5)          , // #8
  O_FPU(00,00DF,7)          , // #9
  O_FPU(00,00DD,1)          , // #10
  O_FPU(00,00DB,5)          , // #11
  O_FPU(00,DFE0,_)          , // #12
  O(000000,DB,7,_,_,_,_,_  ), // #13
  O_FPU(9B,DFE0,_)          , // #14
  O(000000,E4,_,_,_,_,_,_  ), // #15
  O(000000,40,_,_,x,_,_,_  ), // #16
  O(F20F00,78,_,_,_,_,_,_  ), // #17
  O(000000,77,_,_,_,_,_,_  ), // #18
  O(000000,73,_,_,_,_,_,_  ), // #19
  O(000000,72,_,_,_,_,_,_  ), // #20
  O(000000,76,_,_,_,_,_,_  ), // #21
  O(000000,74,_,_,_,_,_,_  ), // #22
  O(000000,E3,_,_,_,_,_,_  ), // #23
  O(000000,7F,_,_,_,_,_,_  ), // #24
  O(000000,7D,_,_,_,_,_,_  ), // #25
  O(000000,7C,_,_,_,_,_,_  ), // #26
  O(000000,7E,_,_,_,_,_,_  ), // #27
  O(000000,EB,_,_,_,_,_,_  ), // #28
  O(000000,75,_,_,_,_,_,_  ), // #29
  O(000000,71,_,_,_,_,_,_  ), // #30
  O(000000,7B,_,_,_,_,_,_  ), // #31
  O(000000,79,_,_,_,_,_,_  ), // #32
  O(000000,70,_,_,_,_,_,_  ), // #33
  O(000000,7A,_,_,_,_,_,_  ), // #34
  O(000000,78,_,_,_,_,_,_  ), // #35
  V(660F00,92,_,0,0,_,_,_  ), // #36
  V(F20F00,92,_,0,0,_,_,_  ), // #37
  V(F20F00,92,_,0,1,_,_,_  ), // #38
  V(000F00,92,_,0,0,_,_,_  ), // #39
  O(000000,E2,_,_,_,_,_,_  ), // #40
  O(000000,E1,_,_,_,_,_,_  ), // #41
  O(000000,E0,_,_,_,_,_,_  ), // #42
  O(660F00,29,_,_,_,_,_,_  ), // #43
  O(000F00,29,_,_,_,_,_,_  ), // #44
  O(000F38,F1,_,_,x,_,_,_  ), // #45
  O(000F00,7E,_,_,_,_,_,_  ), // #46
  O(660F00,7F,_,_,_,_,_,_  ), // #47
  O(F30F00,7F,_,_,_,_,_,_  ), // #48
  O(660F00,17,_,_,_,_,_,_  ), // #49
  O(000F00,17,_,_,_,_,_,_  ), // #50
  O(660F00,13,_,_,_,_,_,_  ), // #51
  O(000F00,13,_,_,_,_,_,_  ), // #52
  O(660F00,E7,_,_,_,_,_,_  ), // #53
  O(660F00,2B,_,_,_,_,_,_  ), // #54
  O(000F00,2B,_,_,_,_,_,_  ), // #55
  O(000F00,E7,_,_,_,_,_,_  ), // #56
  O(F20F00,2B,_,_,_,_,_,_  ), // #57
  O(F30F00,2B,_,_,_,_,_,_  ), // #58
  O(000F00,7E,_,_,x,_,_,_  ), // #59
  O(F20F00,11,_,_,_,_,_,_  ), // #60
  O(F30F00,11,_,_,_,_,_,_  ), // #61
  O(660F00,11,_,_,_,_,_,_  ), // #62
  O(000F00,11,_,_,_,_,_,_  ), // #63
  O(000000,E6,_,_,_,_,_,_  ), // #64
  O(000F3A,15,_,_,_,_,_,_  ), // #65
  O(000000,58,_,_,_,_,_,_  ), // #66
  O(000F00,72,6,_,_,_,_,_  ), // #67
  O(660F00,73,7,_,_,_,_,_  ), // #68
  O(000F00,73,6,_,_,_,_,_  ), // #69
  O(000F00,71,6,_,_,_,_,_  ), // #70
  O(000F00,72,4,_,_,_,_,_  ), // #71
  O(000F00,71,4,_,_,_,_,_  ), // #72
  O(000F00,72,2,_,_,_,_,_  ), // #73
  O(660F00,73,3,_,_,_,_,_  ), // #74
  O(000F00,73,2,_,_,_,_,_  ), // #75
  O(000F00,71,2,_,_,_,_,_  ), // #76
  O(000000,50,_,_,_,_,_,_  ), // #77
  O(000000,F6,_,_,x,_,_,_  ), // #78
  V(660F38,92,_,x,_,1,3,T1S), // #79
  V(660F38,92,_,x,_,0,2,T1S), // #80
  V(660F38,93,_,x,_,1,3,T1S), // #81
  V(660F38,93,_,x,_,0,2,T1S), // #82
  V(660F38,2F,_,x,0,_,_,_  ), // #83
  V(660F38,2E,_,x,0,_,_,_  ), // #84
  V(660F00,29,_,x,I,1,4,FVM), // #85
  V(000F00,29,_,x,I,0,4,FVM), // #86
  V(660F00,7E,_,0,0,0,2,T1S), // #87
  V(660F00,7F,_,x,I,_,_,_  ), // #88
  V(660F00,7F,_,x,_,0,4,FVM), // #89
  V(660F00,7F,_,x,_,1,4,FVM), // #90
  V(F30F00,7F,_,x,I,_,_,_  ), // #91
  V(F20F00,7F,_,x,_,1,4,FVM), // #92
  V(F30F00,7F,_,x,_,0,4,FVM), // #93
  V(F30F00,7F,_,x,_,1,4,FVM), // #94
  V(F20F00,7F,_,x,_,0,4,FVM), // #95
  V(660F00,17,_,0,I,1,3,T1S), // #96
  V(000F00,17,_,0,I,0,3,T2 ), // #97
  V(660F00,13,_,0,I,1,3,T1S), // #98
  V(000F00,13,_,0,I,0,3,T2 ), // #99
  V(660F00,7E,_,0,I,1,3,T1S), // #100
  V(F20F00,11,_,I,I,1,3,T1S), // #101
  V(F30F00,11,_,I,I,0,2,T1S), // #102
  V(660F00,11,_,x,I,1,4,FVM), // #103
  V(000F00,11,_,x,I,0,4,FVM), // #104
  V(660F3A,05,_,x,0,1,4,FV ), // #105
  V(660F3A,04,_,x,0,0,4,FV ), // #106
  V(660F3A,00,_,x,1,1,4,FV ), // #107
  V(660F38,90,_,x,_,0,2,T1S), // #108
  V(660F38,90,_,x,_,1,3,T1S), // #109
  V(660F38,91,_,x,_,0,2,T1S), // #110
  V(660F38,91,_,x,_,1,3,T1S), // #111
  V(660F38,8E,_,x,0,_,_,_  ), // #112
  V(660F38,8E,_,x,1,_,_,_  ), // #113
  V(XOP_M8,C0,_,0,x,_,_,_  ), // #114
  V(XOP_M8,C2,_,0,x,_,_,_  ), // #115
  V(XOP_M8,C3,_,0,x,_,_,_  ), // #116
  V(XOP_M8,C1,_,0,x,_,_,_  ), // #117
  V(660F00,72,6,x,I,0,4,FV ), // #118
  V(660F00,73,6,x,I,1,4,FV ), // #119
  V(660F00,71,6,x,I,I,4,FVM), // #120
  V(660F00,72,4,x,I,0,4,FV ), // #121
  V(660F00,72,4,x,_,1,4,FV ), // #122
  V(660F00,71,4,x,I,I,4,FVM), // #123
  V(660F00,72,2,x,I,0,4,FV ), // #124
  V(660F00,73,2,x,I,1,4,FV ), // #125
  V(660F00,71,2,x,I,I,4,FVM)  // #126
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
  { FEATURE(SSE2)      , AVX_CONV(Extend)    , 715  }, // #0
  { FEATURE(SSE)       , AVX_CONV(Extend)    , 715  }, // #1
  { FEATURE(SSE3)      , AVX_CONV(Extend)    , 715  }, // #2
  { FEATURE(AES)       , AVX_CONV(Extend)    , 714  }, // #3
  { FEATURE(AES)       , AVX_CONV(Move)      , 714  }, // #4
  { FEATURE(SSE2)      , AVX_CONV(Extend)    , 714  }, // #5
  { FEATURE(SSE)       , AVX_CONV(Extend)    , 714  }, // #6
  { FEATURE(SSE4_1)    , AVX_CONV(Extend)    , 713  }, // #7
  { FEATURE(SSE4_1)    , AVX_CONV(Blend)     , 713  }, // #8
  { FEATURE(SSE2)      , AVX_CONV(Extend)    , 661  }, // #9
  { FEATURE(SSE)       , AVX_CONV(Extend)    , 661  }, // #10
  { FEATURE(SSE2)      , AVX_CONV(Extend)    , 660  }, // #11
  { FEATURE(SSE)       , AVX_CONV(Extend)    , 660  }, // #12
  { FEATURE(SSE2)      , AVX_CONV(Move)      , 657  }, // #13
  { FEATURE(SSE)       , AVX_CONV(Move)      , 657  }, // #14
  { FEATURE(SSE2)      , AVX_CONV(Move)      , 656  }, // #15
  { FEATURE(SSE2)      , AVX_CONV(None)      , 0    }, // #16
  { FEATURE(SSE2)      , AVX_CONV(Move)      , 655  }, // #17
  { FEATURE(SSE)       , AVX_CONV(None)      , 0    }, // #18
  { FEATURE(SSE2)      , AVX_CONV(Move)      , 662  }, // #19
  { FEATURE(SSE2)      , AVX_CONV(Extend)    , 662  }, // #20
  { FEATURE(SSE2)      , AVX_CONV(Extend)    , 663  }, // #21
  { FEATURE(SSE)       , AVX_CONV(Extend)    , 663  }, // #22
  { FEATURE(SSE)       , AVX_CONV(Move)      , 663  }, // #23
  { FEATURE(SSE2)      , AVX_CONV(Move)      , 664  }, // #24
  { FEATURE(SSE2)      , AVX_CONV(Move)      , 666  }, // #25
  { FEATURE(SSE2)      , AVX_CONV(Move)      , 668  }, // #26
  { FEATURE(SSE)       , AVX_CONV(Move)      , 669  }, // #27
  { FEATURE(SSE2)      , AVX_CONV(Extend)    , 671  }, // #28
  { FEATURE(SSE)       , AVX_CONV(Extend)    , 671  }, // #29
  { FEATURE(SSE4_1)    , AVX_CONV(Extend)    , 671  }, // #30
  { FEATURE(SSE4_1)    , AVX_CONV(Move)      , 685  }, // #31
  { FEATURE(SSE4A)     , AVX_CONV(None)      , 0    }, // #32
  { FEATURE(SSE3)      , AVX_CONV(Extend)    , 698  }, // #33
  { FEATURE(SSE3)      , AVX_CONV(Extend)    , 697  }, // #34
  { FEATURE(SSE4_1)    , AVX_CONV(Extend)    , 702  }, // #35
  { FEATURE(SSE3)      , AVX_CONV(Move)      , 610  }, // #36
  { FEATURE(SSE2)      , AVX_CONV(Move)      , 596  }, // #37
  { FEATURE(MMX2)      , AVX_CONV(None)      , 0    }, // #38
  { FEATURE(SSE2)      , AVX_CONV(Extend)    , 597  }, // #39
  { FEATURE(SSE)       , AVX_CONV(Extend)    , 597  }, // #40
  { FEATURE(SSE2)      , AVX_CONV(Extend)    , 596  }, // #41
  { FEATURE(SSE)       , AVX_CONV(Extend)    , 596  }, // #42
  { FEATURE(SSE2)      , AVX_CONV(Move)      , 594  }, // #43
  { FEATURE(SSE)       , AVX_CONV(Move)      , 594  }, // #44
  { FEATURE(MMX)       |
    FEATURE(SSE2)      , AVX_CONV(Move)      , 593  }, // #45
  { FEATURE(SSE3)      , AVX_CONV(Move)      , 593  }, // #46
  { FEATURE(SSE2)      , AVX_CONV(Move)      , 592  }, // #47
  { FEATURE(SSE)       , AVX_CONV(Extend)    , 598  }, // #48
  { FEATURE(SSE2)      , AVX_CONV(MoveIfMem) , 598  }, // #49
  { FEATURE(SSE)       , AVX_CONV(MoveIfMem) , 598  }, // #50
  { FEATURE(SSE2)      , AVX_CONV(Move)      , 598  }, // #51
  { FEATURE(SSE)       , AVX_CONV(Move)      , 598  }, // #52
  { FEATURE(SSE4_1)    , AVX_CONV(Move)      , 598  }, // #53
  { FEATURE(SSE2)      , AVX_CONV(Move)      , 597  }, // #54
  { FEATURE(SSE)       , AVX_CONV(Move)      , 597  }, // #55
  { FEATURE(MMX)       |
    FEATURE(SSE2)      , AVX_CONV(Move)      , 594  }, // #56
  { FEATURE(SSE2)      , AVX_CONV(MoveIfMem) , 592  }, // #57
  { FEATURE(SSE3)      , AVX_CONV(Move)      , 592  }, // #58
  { FEATURE(SSE)       , AVX_CONV(MoveIfMem) , 592  }, // #59
  { FEATURE(SSE2)      , AVX_CONV(Move)      , 590  }, // #60
  { FEATURE(SSE)       , AVX_CONV(Move)      , 590  }, // #61
  { FEATURE(SSE4_1)    , AVX_CONV(Extend)    , 589  }, // #62
  { FEATURE(SSE2)      , AVX_CONV(Extend)    , 588  }, // #63
  { FEATURE(SSE)       , AVX_CONV(Extend)    , 588  }, // #64
  { FEATURE(SSE2)      , AVX_CONV(Extend)    , 582  }, // #65
  { FEATURE(SSE)       , AVX_CONV(Extend)    , 582  }, // #66
  { FEATURE(SSSE3)     , AVX_CONV(Move)      , 582  }, // #67
  { FEATURE(SSSE3)     , AVX_CONV(Move)      , 583  }, // #68
  { FEATURE(MMX)       |
    FEATURE(SSE2)      , AVX_CONV(Extend)    , 583  }, // #69
  { FEATURE(SSE4_1)    , AVX_CONV(Extend)    , 583  }, // #70
  { FEATURE(SSE2)      , AVX_CONV(Extend)    , 583  }, // #71
  { FEATURE(SSE3)      , AVX_CONV(Extend)    , 583  }, // #72
  { FEATURE(MMX)       |
    FEATURE(SSE2)      , AVX_CONV(Extend)    , 584  }, // #73
  { FEATURE(MMX2)      |
    FEATURE(SSE2)      , AVX_CONV(Extend)    , 586  }, // #74
  { FEATURE(3DNOW)     , AVX_CONV(None)      , 0    }, // #75
  { FEATURE(MMX2)      |
    FEATURE(SSE2)      , AVX_CONV(Extend)    , 585  }, // #76
  { FEATURE(SSE4_1)    , AVX_CONV(Blend)     , 586  }, // #77
  { FEATURE(SSE4_1)    , AVX_CONV(Extend)    , 586  }, // #78
  { FEATURE(PCLMULQDQ) , AVX_CONV(Extend)    , 592  }, // #79
  { FEATURE(MMX)       |
    FEATURE(SSE2)      , AVX_CONV(Extend)    , 595  }, // #80
  { FEATURE(SSE4_1)    , AVX_CONV(Extend)    , 595  }, // #81
  { FEATURE(SSE4_2)    , AVX_CONV(Move)      , 595  }, // #82
  { FEATURE(SSE4_2)    , AVX_CONV(Extend)    , 595  }, // #83
  { FEATURE(SSE4_1)    , AVX_CONV(Move)      , 636  }, // #84
  { FEATURE(MMX2)      |
    FEATURE(SSE2)      |
    FEATURE(SSE4_1)    , AVX_CONV(Move)      , 636  }, // #85
  { FEATURE(3DNOW2)    , AVX_CONV(None)      , 0    }, // #86
  { FEATURE(GEODE)     , AVX_CONV(None)      , 0    }, // #87
  { FEATURE(SSSE3)     , AVX_CONV(Extend)    , 622  }, // #88
  { FEATURE(SSSE3)     , AVX_CONV(Extend)    , 623  }, // #89
  { FEATURE(SSSE3)     , AVX_CONV(Extend)    , 629  }, // #90
  { FEATURE(SSE4_1)    , AVX_CONV(Move)      , 631  }, // #91
  { FEATURE(SSSE3)     , AVX_CONV(Extend)    , 632  }, // #92
  { FEATURE(SSSE3)     , AVX_CONV(Extend)    , 633  }, // #93
  { FEATURE(SSE4_1)    , AVX_CONV(Move)      , 632  }, // #94
  { FEATURE(MMX2)      |
    FEATURE(SSE2)      , AVX_CONV(Extend)    , 632  }, // #95
  { FEATURE(SSSE3)     , AVX_CONV(Extend)    , 648  }, // #96
  { FEATURE(MMX)       |
    FEATURE(SSE2)      , AVX_CONV(Extend)    , 648  }, // #97
  { FEATURE(SSE4_1)    , AVX_CONV(Extend)    , 650  }, // #98
  { FEATURE(MMX2)      |
    FEATURE(SSE2)      , AVX_CONV(Extend)    , 651  }, // #99
  { FEATURE(SSE4_1)    , AVX_CONV(Extend)    , 651  }, // #100
  { FEATURE(SSE4_1)    , AVX_CONV(Extend)    , 652  }, // #101
  { FEATURE(MMX2)      |
    FEATURE(SSE2)      , AVX_CONV(Extend)    , 653  }, // #102
  { FEATURE(SSE4_1)    , AVX_CONV(Extend)    , 653  }, // #103
  { FEATURE(SSE4_1)    , AVX_CONV(Extend)    , 654  }, // #104
  { FEATURE(MMX2)      |
    FEATURE(SSE2)      , AVX_CONV(Move)      , 662  }, // #105
  { FEATURE(SSE4_1)    , AVX_CONV(Move)      , 672  }, // #106
  { FEATURE(SSE4_1)    , AVX_CONV(Move)      , 680  }, // #107
  { FEATURE(SSE4_1)    , AVX_CONV(Extend)    , 680  }, // #108
  { FEATURE(SSSE3)     , AVX_CONV(Extend)    , 680  }, // #109
  { FEATURE(MMX2)      |
    FEATURE(SSE2)      , AVX_CONV(Extend)    , 679  }, // #110
  { FEATURE(MMX)       |
    FEATURE(SSE2)      , AVX_CONV(Extend)    , 679  }, // #111
  { FEATURE(SSE4_1)    , AVX_CONV(Extend)    , 679  }, // #112
  { FEATURE(MMX)       |
    FEATURE(SSE2)      , AVX_CONV(Extend)    , 680  }, // #113
  { FEATURE(SSE2)      , AVX_CONV(Extend)    , 681  }, // #114
  { FEATURE(MMX)       |
    FEATURE(SSE2)      , AVX_CONV(Extend)    , 676  }, // #115
  { FEATURE(MMX2)      |
    FEATURE(SSE2)      , AVX_CONV(Extend)    , 684  }, // #116
  { FEATURE(SSSE3)     , AVX_CONV(Extend)    , 696  }, // #117
  { FEATURE(SSE2)      , AVX_CONV(Move)      , 696  }, // #118
  { FEATURE(SSSE3)     , AVX_CONV(Extend)    , 695  }, // #119
  { FEATURE(MMX)       |
    FEATURE(SSE2)      , AVX_CONV(Extend)    , 695  }, // #120
  { FEATURE(SSE2)      , AVX_CONV(Extend)    , 695  }, // #121
  { FEATURE(MMX)       |
    FEATURE(SSE2)      , AVX_CONV(Extend)    , 698  }, // #122
  { FEATURE(MMX)       |
    FEATURE(SSE2)      , AVX_CONV(Extend)    , 702  }, // #123
  { FEATURE(SSE2)      , AVX_CONV(Extend)    , 702  }, // #124
  { FEATURE(MMX)       |
    FEATURE(SSE2)      , AVX_CONV(Extend)    , 705  }, // #125
  { FEATURE(SSE2)      , AVX_CONV(Extend)    , 705  }, // #126
  { FEATURE(SSE4_1)    , AVX_CONV(Move)      , 706  }, // #127
  { FEATURE(MMX)       |
    FEATURE(SSE2)      , AVX_CONV(Extend)    , 714  }, // #128
  { FEATURE(MMX)       |
    FEATURE(SSE2)      , AVX_CONV(Extend)    , 708  }, // #129
  { FEATURE(SSE)       , AVX_CONV(Move)      , 721  }, // #130
  { FEATURE(SSE)       , AVX_CONV(Extend)    , 721  }, // #131
  { FEATURE(SSE4_1)    , AVX_CONV(Move)      , 716  }, // #132
  { FEATURE(SSE4_1)    , AVX_CONV(Extend)    , 716  }, // #133
  { FEATURE(SSE)       , AVX_CONV(Move)      , 724  }, // #134
  { FEATURE(SSE)       , AVX_CONV(Extend)    , 724  }, // #135
  { FEATURE(SHA)       , AVX_CONV(None)      , 0    }, // #136
  { FEATURE(SSE2)      , AVX_CONV(Extend)    , 693  }, // #137
  { FEATURE(SSE)       , AVX_CONV(Extend)    , 693  }, // #138
  { FEATURE(SSE2)      , AVX_CONV(Move)      , 690  }, // #139
  { FEATURE(SSE)       , AVX_CONV(Move)      , 690  }, // #140
  { FEATURE(SSE2)      , AVX_CONV(Extend)    , 690  }, // #141
  { FEATURE(SSE)       , AVX_CONV(Extend)    , 690  }, // #142
  { FEATURE(SSE2)      , AVX_CONV(Extend)    , 683  }, // #143
  { FEATURE(SSE)       , AVX_CONV(Extend)    , 683  }, // #144
  { FEATURE(SSE2)      , AVX_CONV(Move)      , 674  }, // #145
  { FEATURE(SSE)       , AVX_CONV(Move)      , 674  }, // #146
  { FEATURE(SSE2)      , AVX_CONV(Extend)    , 673  }, // #147
  { FEATURE(SSE)       , AVX_CONV(Extend)    , 673  }, // #148
  { FEATURE(SSE2)      , AVX_CONV(Extend)    , -12  }, // #149
  { FEATURE(SSE)       , AVX_CONV(Extend)    , -12  }  // #150
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
  { FEATURE(AVX512_4FMAPS), FLAG(Masking) | FLAG(Zeroing) }, // #0
  { FEATURE(AVX)       |
    FEATURE(AVX512_F)  |
    FEATURE(AVX512_VL) , FLAG(Broadcast64) | FLAG(ER) | FLAG(Masking) | FLAG(SAE) | FLAG(Zeroing) }, // #1
  { FEATURE(AVX)       |
    FEATURE(AVX512_F)  |
    FEATURE(AVX512_VL) , FLAG(Broadcast32) | FLAG(ER) | FLAG(Masking) | FLAG(SAE) | FLAG(Zeroing) }, // #2
  { FEATURE(AVX)       |
    FEATURE(AVX512_F)  , FLAG(ER) | FLAG(Masking) | FLAG(SAE) | FLAG(Zeroing) }, // #3
  { FEATURE(AVX)       , 0 }, // #4
  { FEATURE(AES)       |
    FEATURE(AVX)       , 0 }, // #5
  { FEATURE(AVX512_F)  |
    FEATURE(AVX512_VL) , FLAG(Broadcast32) | FLAG(Masking) | FLAG(Zeroing) }, // #6
  { FEATURE(AVX512_F)  |
    FEATURE(AVX512_VL) , FLAG(Broadcast64) | FLAG(Masking) | FLAG(Zeroing) }, // #7
  { FEATURE(AVX)       |
    FEATURE(AVX512_DQ) |
    FEATURE(AVX512_VL) , FLAG(Broadcast64) | FLAG(Masking) | FLAG(Zeroing) }, // #8
  { FEATURE(AVX)       |
    FEATURE(AVX512_DQ) |
    FEATURE(AVX512_VL) , FLAG(Broadcast32) | FLAG(Masking) | FLAG(Zeroing) }, // #9
  { FEATURE(AVX512_BW) |
    FEATURE(AVX512_VL) , FLAG(Masking) | FLAG(Zeroing) }, // #10
  { FEATURE(AVX512_DQ) |
    FEATURE(AVX512_VL) , FLAG(Masking) | FLAG(Zeroing) }, // #11
  { FEATURE(AVX512_F)  , FLAG(Masking) | FLAG(Zeroing) }, // #12
  { FEATURE(AVX512_DQ) , FLAG(Masking) | FLAG(Zeroing) }, // #13
  { FEATURE(AVX2)      , 0 }, // #14
  { FEATURE(AVX512_F)  |
    FEATURE(AVX512_VL) , FLAG(Masking) | FLAG(Zeroing) }, // #15
  { FEATURE(AVX)       |
    FEATURE(AVX2)      |
    FEATURE(AVX512_F)  |
    FEATURE(AVX512_VL) , FLAG(Masking) | FLAG(Zeroing) }, // #16
  { FEATURE(AVX)       |
    FEATURE(AVX512_F)  |
    FEATURE(AVX512_VL) , FLAG(Broadcast64) | FLAG(Masking) | FLAG(SAE) | FLAG(Zeroing) }, // #17
  { FEATURE(AVX)       |
    FEATURE(AVX512_F)  |
    FEATURE(AVX512_VL) , FLAG(Broadcast32) | FLAG(Masking) | FLAG(SAE) | FLAG(Zeroing) }, // #18
  { FEATURE(AVX)       |
    FEATURE(AVX512_F)  , FLAG(Masking) | FLAG(SAE) | FLAG(Zeroing) }, // #19
  { FEATURE(AVX)       |
    FEATURE(AVX512_F)  , FLAG(SAE) }, // #20
  { FEATURE(AVX)       |
    FEATURE(AVX512_F)  |
    FEATURE(AVX512_VL) , FLAG(Broadcast32) | FLAG(Masking) | FLAG(Zeroing) }, // #21
  { FEATURE(AVX512_DQ) |
    FEATURE(AVX512_VL) , FLAG(Broadcast64) | FLAG(ER) | FLAG(Masking) | FLAG(SAE) | FLAG(Zeroing) }, // #22
  { FEATURE(AVX512_F)  |
    FEATURE(AVX512_VL) , FLAG(Broadcast64) | FLAG(ER) | FLAG(Masking) | FLAG(SAE) | FLAG(Zeroing) }, // #23
  { FEATURE(AVX512_F)  |
    FEATURE(AVX512_VL) |
    FEATURE(F16C)      , FLAG(Masking) | FLAG(SAE) | FLAG(Zeroing) }, // #24
  { FEATURE(AVX512_DQ) |
    FEATURE(AVX512_VL) , FLAG(Broadcast32) | FLAG(ER) | FLAG(Masking) | FLAG(SAE) | FLAG(Zeroing) }, // #25
  { FEATURE(AVX512_F)  |
    FEATURE(AVX512_VL) , FLAG(Broadcast32) | FLAG(ER) | FLAG(Masking) | FLAG(SAE) | FLAG(Zeroing) }, // #26
  { FEATURE(AVX)       |
    FEATURE(AVX512_F)  , FLAG(ER) | FLAG(SAE) }, // #27
  { FEATURE(AVX512_F)  , FLAG(ER) | FLAG(SAE) }, // #28
  { FEATURE(AVX512_F)  |
    FEATURE(AVX512_VL) , FLAG(Broadcast64) | FLAG(Masking) | FLAG(SAE) | FLAG(Zeroing) }, // #29
  { FEATURE(AVX512_DQ) |
    FEATURE(AVX512_VL) , FLAG(Broadcast64) | FLAG(Masking) | FLAG(SAE) | FLAG(Zeroing) }, // #30
  { FEATURE(AVX512_DQ) |
    FEATURE(AVX512_VL) , FLAG(Broadcast32) | FLAG(Masking) | FLAG(SAE) | FLAG(Zeroing) }, // #31
  { FEATURE(AVX512_F)  |
    FEATURE(AVX512_VL) , FLAG(Broadcast32) | FLAG(Masking) | FLAG(SAE) | FLAG(Zeroing) }, // #32
  { FEATURE(AVX512_F)  , FLAG(SAE) }, // #33
  { FEATURE(AVX512_ERI), FLAG(Broadcast64) | FLAG(Masking) | FLAG(SAE) | FLAG(Zeroing) }, // #34
  { FEATURE(AVX512_ERI), FLAG(Broadcast32) | FLAG(Masking) | FLAG(SAE) | FLAG(Zeroing) }, // #35
  { FEATURE(AVX)       |
    FEATURE(AVX512_F)  , 0 }, // #36
  { FEATURE(AVX512_F)  , FLAG(Masking) | FLAG(SAE) | FLAG(Zeroing) }, // #37
  { FEATURE(AVX512_F)  |
    FEATURE(AVX512_VL) |
    FEATURE(FMA)       , FLAG(Broadcast64) | FLAG(ER) | FLAG(Masking) | FLAG(SAE) | FLAG(Zeroing) }, // #38
  { FEATURE(AVX512_F)  |
    FEATURE(AVX512_VL) |
    FEATURE(FMA)       , FLAG(Broadcast32) | FLAG(ER) | FLAG(Masking) | FLAG(SAE) | FLAG(Zeroing) }, // #39
  { FEATURE(AVX512_F)  |
    FEATURE(FMA)       , FLAG(ER) | FLAG(Masking) | FLAG(SAE) | FLAG(Zeroing) }, // #40
  { FEATURE(FMA4)      , 0 }, // #41
  { FEATURE(AVX512_DQ) |
    FEATURE(AVX512_VL) , FLAG(Broadcast64) | FLAG(Masking) }, // #42
  { FEATURE(AVX512_DQ) |
    FEATURE(AVX512_VL) , FLAG(Broadcast32) | FLAG(Masking) }, // #43
  { FEATURE(AVX512_DQ) , FLAG(Masking) }, // #44
  { FEATURE(XOP)       , 0 }, // #45
  { FEATURE(AVX2)      |
    FEATURE(AVX512_F)  |
    FEATURE(AVX512_VL) , FLAG(Masking) }, // #46
  { FEATURE(AVX)       |
    FEATURE(AVX512_F)  |
    FEATURE(AVX512_VL) , FLAG(Masking) | FLAG(SAE) | FLAG(Zeroing) }, // #47
  { FEATURE(AVX)       |
    FEATURE(AVX512_F)  |
    FEATURE(AVX512_VL) , FLAG(Masking) | FLAG(Zeroing) }, // #48
  { FEATURE(AVX)       |
    FEATURE(AVX512_F)  |
    FEATURE(AVX512_VL) , 0 }, // #49
  { FEATURE(AVX)       |
    FEATURE(AVX2)      |
    FEATURE(AVX512_F)  |
    FEATURE(AVX512_VL) , 0 }, // #50
  { FEATURE(AVX)       |
    FEATURE(AVX512_F)  , FLAG(Masking) | FLAG(Zeroing) }, // #51
  { FEATURE(AVX)       |
    FEATURE(AVX2)      , 0 }, // #52
  { FEATURE(AVX)       |
    FEATURE(AVX2)      |
    FEATURE(AVX512_F)  |
    FEATURE(AVX512_VL) , FLAG(Broadcast64) | FLAG(ER) | FLAG(Masking) | FLAG(SAE) | FLAG(Zeroing) }, // #53
  { FEATURE(AVX)       |
    FEATURE(AVX2)      |
    FEATURE(AVX512_F)  |
    FEATURE(AVX512_VL) , FLAG(Broadcast32) | FLAG(ER) | FLAG(Masking) | FLAG(SAE) | FLAG(Zeroing) }, // #54
  { FEATURE(AVX512_4VNNIW), FLAG(Masking) | FLAG(Zeroing) }, // #55
  { FEATURE(AVX)       |
    FEATURE(AVX2)      |
    FEATURE(AVX512_BW) |
    FEATURE(AVX512_VL) , FLAG(Masking) | FLAG(Zeroing) }, // #56
  { FEATURE(AVX)       |
    FEATURE(AVX2)      |
    FEATURE(AVX512_BW) |
    FEATURE(AVX512_VL) , FLAG(Broadcast32) | FLAG(Masking) | FLAG(Zeroing) }, // #57
  { FEATURE(AVX)       |
    FEATURE(AVX2)      |
    FEATURE(AVX512_F)  |
    FEATURE(AVX512_VL) , FLAG(Broadcast32) | FLAG(Masking) | FLAG(Zeroing) }, // #58
  { FEATURE(AVX)       |
    FEATURE(AVX2)      |
    FEATURE(AVX512_F)  |
    FEATURE(AVX512_VL) , FLAG(Broadcast64) | FLAG(Masking) | FLAG(Zeroing) }, // #59
  { FEATURE(AVX)       |
    FEATURE(AVX512_BW) |
    FEATURE(AVX512_VL) , FLAG(Masking) | FLAG(Zeroing) }, // #60
  { FEATURE(AVX2)      |
    FEATURE(AVX512_BW) |
    FEATURE(AVX512_VL) , FLAG(Masking) | FLAG(Zeroing) }, // #61
  { FEATURE(AVX2)      |
    FEATURE(AVX512_F)  |
    FEATURE(AVX512_VL) , FLAG(Masking) | FLAG(Zeroing) }, // #62
  { FEATURE(AVX512_CDI)|
    FEATURE(AVX512_VL) , 0 }, // #63
  { FEATURE(AVX)       |
    FEATURE(PCLMULQDQ) , 0 }, // #64
  { FEATURE(AVX512_BW) |
    FEATURE(AVX512_VL) , FLAG(Masking) }, // #65
  { FEATURE(AVX512_F)  |
    FEATURE(AVX512_VL) , FLAG(Broadcast32) | FLAG(Masking) }, // #66
  { FEATURE(AVX)       |
    FEATURE(AVX2)      |
    FEATURE(AVX512_BW) |
    FEATURE(AVX512_VL) , FLAG(Masking) }, // #67
  { FEATURE(AVX)       |
    FEATURE(AVX2)      |
    FEATURE(AVX512_F)  |
    FEATURE(AVX512_VL) , FLAG(Broadcast32) | FLAG(Masking) }, // #68
  { FEATURE(AVX)       |
    FEATURE(AVX2)      |
    FEATURE(AVX512_F)  |
    FEATURE(AVX512_VL) , FLAG(Broadcast64) | FLAG(Masking) }, // #69
  { FEATURE(AVX512_F)  |
    FEATURE(AVX512_VL) , FLAG(Broadcast64) | FLAG(Masking) }, // #70
  { FEATURE(AVX512_BW) |
    FEATURE(AVX512_VL) , FLAG(Broadcast64) | FLAG(Masking) }, // #71
  { FEATURE(AVX512_CDI)|
    FEATURE(AVX512_VL) , FLAG(Broadcast32) | FLAG(Masking) | FLAG(Zeroing) }, // #72
  { FEATURE(AVX512_VBMI)|
    FEATURE(AVX512_VL) , FLAG(Masking) | FLAG(Zeroing) }, // #73
  { FEATURE(AVX2)      |
    FEATURE(AVX512_F)  |
    FEATURE(AVX512_VL) , FLAG(Broadcast32) | FLAG(Masking) | FLAG(Zeroing) }, // #74
  { FEATURE(AVX)       |
    FEATURE(AVX512_F)  |
    FEATURE(AVX512_VL) , FLAG(Broadcast64) | FLAG(Masking) | FLAG(Zeroing) }, // #75
  { FEATURE(AVX2)      |
    FEATURE(AVX512_F)  |
    FEATURE(AVX512_VL) , FLAG(Broadcast64) | FLAG(Masking) | FLAG(Zeroing) }, // #76
  { FEATURE(AVX)       |
    FEATURE(AVX512_BW) , 0 }, // #77
  { FEATURE(AVX)       |
    FEATURE(AVX512_DQ) , 0 }, // #78
  { FEATURE(AVX)       |
    FEATURE(AVX512_BW) , FLAG(Masking) | FLAG(Zeroing) }, // #79
  { FEATURE(AVX)       |
    FEATURE(AVX512_DQ) , FLAG(Masking) | FLAG(Zeroing) }, // #80
  { FEATURE(AVX512_CDI)|
    FEATURE(AVX512_VL) , FLAG(Broadcast64) | FLAG(Masking) | FLAG(Zeroing) }, // #81
  { FEATURE(AVX512_IFMA)|
    FEATURE(AVX512_VL) , FLAG(Broadcast64) | FLAG(Masking) | FLAG(Zeroing) }, // #82
  { FEATURE(AVX512_BW) |
    FEATURE(AVX512_VL) , 0 }, // #83
  { FEATURE(AVX512_DQ) |
    FEATURE(AVX512_VL) , 0 }, // #84
  { FEATURE(AVX512_DQ) |
    FEATURE(AVX512_VL) , FLAG(Broadcast64) | FLAG(Masking) | FLAG(Zeroing) }, // #85
  { FEATURE(AVX512_VBMI)|
    FEATURE(AVX512_VL) , FLAG(Broadcast64) | FLAG(Masking) | FLAG(Zeroing) }, // #86
  { FEATURE(AVX512_VPOPCNTDQ), FLAG(Broadcast32) | FLAG(Masking) | FLAG(Zeroing) }, // #87
  { FEATURE(AVX512_VPOPCNTDQ), FLAG(Broadcast64) | FLAG(Masking) | FLAG(Zeroing) }, // #88
  { FEATURE(AVX)       |
    FEATURE(AVX2)      |
    FEATURE(AVX512_BW) |
    FEATURE(AVX512_VL) , 0 }, // #89
  { FEATURE(AVX512_F)  |
    FEATURE(AVX512_VL) , FLAG(Masking) }, // #90
  { FEATURE(AVX512_DQ) , FLAG(Masking) | FLAG(SAE) | FLAG(Zeroing) }, // #91
  { FEATURE(AVX512_ERI), FLAG(Masking) | FLAG(SAE) | FLAG(Zeroing) }, // #92
  { FEATURE(AVX512_DQ) |
    FEATURE(AVX512_VL) , FLAG(Broadcast32) | FLAG(Masking) | FLAG(Zeroing) }, // #93
  { FEATURE(AVX512_F)  , FLAG(ER) | FLAG(Masking) | FLAG(SAE) | FLAG(Zeroing) }  // #94
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
  "\0" "aaa\0" "aad\0" "aam\0" "aas\0" "adc\0" "adcx\0" "adox\0" "arpl\0"
  "bextr\0" "blcfill\0" "blci\0" "blcic\0" "blcmsk\0" "blcs\0" "blsfill\0"
  "blsi\0" "blsic\0" "blsmsk\0" "blsr\0" "bndcl\0" "bndcn\0" "bndcu\0"
  "bndldx\0" "bndmk\0" "bndmov\0" "bndstx\0" "bound\0" "bsf\0" "bsr\0"
  "bswap\0" "bt\0" "btc\0" "btr\0" "bts\0" "bzhi\0" "cbw\0" "cdq\0" "cdqe\0"
  "clac\0" "clc\0" "cld\0" "clflush\0" "clflushopt\0" "cli\0" "clts\0" "clwb\0"
  "clzero\0" "cmc\0" "cmova\0" "cmovae\0" "cmovc\0" "cmovg\0" "cmovge\0"
  "cmovl\0" "cmovle\0" "cmovna\0" "cmovnae\0" "cmovnc\0" "cmovng\0" "cmovnge\0"
  "cmovnl\0" "cmovnle\0" "cmovno\0" "cmovnp\0" "cmovns\0" "cmovnz\0" "cmovo\0"
  "cmovp\0" "cmovpe\0" "cmovpo\0" "cmovs\0" "cmovz\0" "cmp\0" "cmps\0"
  "cmpxchg\0" "cmpxchg16b\0" "cmpxchg8b\0" "cpuid\0" "cqo\0" "crc32\0"
  "cvtpd2pi\0" "cvtpi2pd\0" "cvtpi2ps\0" "cvtps2pi\0" "cvttpd2pi\0"
  "cvttps2pi\0" "cwd\0" "cwde\0" "daa\0" "das\0" "f2xm1\0" "fabs\0" "faddp\0"
  "fbld\0" "fbstp\0" "fchs\0" "fclex\0" "fcmovb\0" "fcmovbe\0" "fcmove\0"
  "fcmovnb\0" "fcmovnbe\0" "fcmovne\0" "fcmovnu\0" "fcmovu\0" "fcom\0"
  "fcomi\0" "fcomip\0" "fcomp\0" "fcompp\0" "fcos\0" "fdecstp\0" "fdiv\0"
  "fdivp\0" "fdivr\0" "fdivrp\0" "femms\0" "ffree\0" "fiadd\0" "ficom\0"
  "ficomp\0" "fidiv\0" "fidivr\0" "fild\0" "fimul\0" "fincstp\0" "finit\0"
  "fist\0" "fistp\0" "fisttp\0" "fisub\0" "fisubr\0" "fld\0" "fld1\0" "fldcw\0"
  "fldenv\0" "fldl2e\0" "fldl2t\0" "fldlg2\0" "fldln2\0" "fldpi\0" "fldz\0"
  "fmulp\0" "fnclex\0" "fninit\0" "fnop\0" "fnsave\0" "fnstcw\0" "fnstenv\0"
  "fnstsw\0" "fpatan\0" "fprem\0" "fprem1\0" "fptan\0" "frndint\0" "frstor\0"
  "fsave\0" "fscale\0" "fsin\0" "fsincos\0" "fsqrt\0" "fst\0" "fstcw\0"
  "fstenv\0" "fstp\0" "fstsw\0" "fsubp\0" "fsubrp\0" "ftst\0" "fucom\0"
  "fucomi\0" "fucomip\0" "fucomp\0" "fucompp\0" "fwait\0" "fxam\0" "fxch\0"
  "fxrstor\0" "fxrstor64\0" "fxsave\0" "fxsave64\0" "fxtract\0" "fyl2x\0"
  "fyl2xp1\0" "hlt\0" "inc\0" "ins\0" "insertq\0" "int3\0" "into\0" "invlpg\0"
  "invpcid\0" "ja\0" "jae\0" "jb\0" "jbe\0" "jc\0" "je\0" "jecxz\0" "jg\0"
  "jge\0" "jl\0" "jle\0" "jmp\0" "jna\0" "jnae\0" "jnb\0" "jnbe\0" "jnc\0"
  "jne\0" "jng\0" "jnge\0" "jnl\0" "jnle\0" "jno\0" "jnp\0" "jns\0" "jnz\0"
  "jo\0" "jp\0" "jpe\0" "jpo\0" "js\0" "jz\0" "kaddb\0" "kaddd\0" "kaddq\0"
  "kaddw\0" "kandb\0" "kandd\0" "kandnb\0" "kandnd\0" "kandnq\0" "kandnw\0"
  "kandq\0" "kandw\0" "kmovb\0" "kmovw\0" "knotb\0" "knotd\0" "knotq\0"
  "knotw\0" "korb\0" "kord\0" "korq\0" "kortestb\0" "kortestd\0" "kortestq\0"
  "kortestw\0" "korw\0" "kshiftlb\0" "kshiftld\0" "kshiftlq\0" "kshiftlw\0"
  "kshiftrb\0" "kshiftrd\0" "kshiftrq\0" "kshiftrw\0" "ktestb\0" "ktestd\0"
  "ktestq\0" "ktestw\0" "kunpckbw\0" "kunpckdq\0" "kunpckwd\0" "kxnorb\0"
  "kxnord\0" "kxnorq\0" "kxnorw\0" "kxorb\0" "kxord\0" "kxorq\0" "kxorw\0"
  "lahf\0" "lar\0" "lea\0" "leave\0" "lfence\0" "lgdt\0" "lidt\0" "lldt\0"
  "lmsw\0" "lods\0" "loop\0" "loope\0" "loopne\0" "lsl\0" "ltr\0" "lzcnt\0"
  "mfence\0" "monitor\0" "movdq2q\0" "movnti\0" "movntq\0" "movntsd\0"
  "movntss\0" "movq2dq\0" "movsx\0" "movsxd\0" "movzx\0" "mulx\0" "mwait\0"
  "neg\0" "not\0" "out\0" "outs\0" "pause\0" "pavgusb\0" "pcommit\0" "pdep\0"
  "pext\0" "pf2id\0" "pf2iw\0" "pfacc\0" "pfadd\0" "pfcmpeq\0" "pfcmpge\0"
  "pfcmpgt\0" "pfmax\0" "pfmin\0" "pfmul\0" "pfnacc\0" "pfpnacc\0" "pfrcp\0"
  "pfrcpit1\0" "pfrcpit2\0" "pfrcpv\0" "pfrsqit1\0" "pfrsqrt\0" "pfrsqrtv\0"
  "pfsub\0" "pfsubr\0" "pi2fd\0" "pi2fw\0" "pmulhrw\0" "pop\0" "popa\0"
  "popad\0" "popcnt\0" "popf\0" "popfd\0" "popfq\0" "prefetch\0"
  "prefetchnta\0" "prefetcht0\0" "prefetcht1\0" "prefetcht2\0" "prefetchw\0"
  "prefetchwt1\0" "pshufw\0" "pswapd\0" "push\0" "pusha\0" "pushad\0" "pushf\0"
  "pushfd\0" "pushfq\0" "rcl\0" "rcr\0" "rdfsbase\0" "rdgsbase\0" "rdmsr\0"
  "rdpmc\0" "rdrand\0" "rdseed\0" "rdtsc\0" "rdtscp\0" "rol\0" "ror\0" "rorx\0"
  "sahf\0" "sal\0" "sar\0" "sarx\0" "sbb\0" "scas\0" "seta\0" "setae\0"
  "setb\0" "setbe\0" "setc\0" "sete\0" "setg\0" "setge\0" "setl\0" "setle\0"
  "setna\0" "setnae\0" "setnb\0" "setnbe\0" "setnc\0" "setne\0" "setng\0"
  "setnge\0" "setnl\0" "setnle\0" "setno\0" "setnp\0" "setns\0" "setnz\0"
  "seto\0" "setp\0" "setpe\0" "setpo\0" "sets\0" "setz\0" "sfence\0" "sgdt\0"
  "sha1msg1\0" "sha1msg2\0" "sha1nexte\0" "sha1rnds4\0" "sha256msg1\0"
  "sha256msg2\0" "sha256rnds2\0" "shl\0" "shlx\0" "shr\0" "shrd\0" "shrx\0"
  "sidt\0" "sldt\0" "smsw\0" "stac\0" "stc\0" "sti\0" "stos\0" "str\0"
  "swapgs\0" "syscall\0" "sysenter\0" "sysexit\0" "sysexit64\0" "sysret\0"
  "sysret64\0" "t1mskc\0" "tzcnt\0" "tzmsk\0" "ud2\0" "v4fmaddps\0"
  "v4fnmaddps\0" "vaddpd\0" "vaddps\0" "vaddsd\0" "vaddss\0" "vaddsubpd\0"
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
  "vdivss\0" "vdppd\0" "vdpps\0" "verr\0" "verw\0" "vexp2pd\0" "vexp2ps\0"
  "vexpandpd\0" "vexpandps\0" "vextractf128\0" "vextractf32x4\0"
  "vextractf32x8\0" "vextractf64x2\0" "vextractf64x4\0" "vextracti128\0"
  "vextracti32x4\0" "vextracti32x8\0" "vextracti64x2\0" "vextracti64x4\0"
  "vextractps\0" "vfixupimmpd\0" "vfixupimmps\0" "vfixupimmsd\0"
  "vfixupimmss\0" "vfmadd132pd\0" "vfmadd132ps\0" "vfmadd132sd\0"
  "vfmadd132ss\0" "vfmadd213pd\0" "vfmadd213ps\0" "vfmadd213sd\0"
  "vfmadd213ss\0" "vfmadd231pd\0" "vfmadd231ps\0" "vfmadd231sd\0"
  "vfmadd231ss\0" "vfmaddpd\0" "vfmaddps\0" "vfmaddsd\0" "vfmaddss\0"
  "vfmaddsub132pd\0" "vfmaddsub132ps\0" "vfmaddsub213pd\0" "vfmaddsub213ps\0"
  "vfmaddsub231pd\0" "vfmaddsub231ps\0" "vfmaddsubpd\0" "vfmaddsubps\0"
  "vfmsub132pd\0" "vfmsub132ps\0" "vfmsub132sd\0" "vfmsub132ss\0"
  "vfmsub213pd\0" "vfmsub213ps\0" "vfmsub213sd\0" "vfmsub213ss\0"
  "vfmsub231pd\0" "vfmsub231ps\0" "vfmsub231sd\0" "vfmsub231ss\0"
  "vfmsubadd132pd\0" "vfmsubadd132ps\0" "vfmsubadd213pd\0" "vfmsubadd213ps\0"
  "vfmsubadd231pd\0" "vfmsubadd231ps\0" "vfmsubaddpd\0" "vfmsubaddps\0"
  "vfmsubpd\0" "vfmsubps\0" "vfmsubsd\0" "vfmsubss\0" "vfnmadd132pd\0"
  "vfnmadd132ps\0" "vfnmadd132sd\0" "vfnmadd132ss\0" "vfnmadd213pd\0"
  "vfnmadd213ps\0" "vfnmadd213sd\0" "vfnmadd213ss\0" "vfnmadd231pd\0"
  "vfnmadd231ps\0" "vfnmadd231sd\0" "vfnmadd231ss\0" "vfnmaddpd\0"
  "vfnmaddps\0" "vfnmaddsd\0" "vfnmaddss\0" "vfnmsub132pd\0" "vfnmsub132ps\0"
  "vfnmsub132sd\0" "vfnmsub132ss\0" "vfnmsub213pd\0" "vfnmsub213ps\0"
  "vfnmsub213sd\0" "vfnmsub213ss\0" "vfnmsub231pd\0" "vfnmsub231ps\0"
  "vfnmsub231sd\0" "vfnmsub231ss\0" "vfnmsubpd\0" "vfnmsubps\0" "vfnmsubsd\0"
  "vfnmsubss\0" "vfpclasspd\0" "vfpclassps\0" "vfpclasssd\0" "vfpclassss\0"
  "vfrczpd\0" "vfrczps\0" "vfrczsd\0" "vfrczss\0" "vgatherdpd\0" "vgatherdps\0"
  "vgatherpf0dpd\0" "vgatherpf0dps\0" "vgatherpf0qpd\0" "vgatherpf0qps\0"
  "vgatherpf1dpd\0" "vgatherpf1dps\0" "vgatherpf1qpd\0" "vgatherpf1qps\0"
  "vgatherqpd\0" "vgatherqps\0" "vgetexppd\0" "vgetexpps\0" "vgetexpsd\0"
  "vgetexpss\0" "vgetmantpd\0" "vgetmantps\0" "vgetmantsd\0" "vgetmantss\0"
  "vhaddpd\0" "vhaddps\0" "vhsubpd\0" "vhsubps\0" "vinsertf128\0"
  "vinsertf32x4\0" "vinsertf32x8\0" "vinsertf64x2\0" "vinsertf64x4\0"
  "vinserti128\0" "vinserti32x4\0" "vinserti32x8\0" "vinserti64x2\0"
  "vinserti64x4\0" "vinsertps\0" "vlddqu\0" "vldmxcsr\0" "vmaskmovdqu\0"
  "vmaskmovpd\0" "vmaskmovps\0" "vmaxpd\0" "vmaxps\0" "vmaxsd\0" "vmaxss\0"
  "vminpd\0" "vminps\0" "vminsd\0" "vminss\0" "vmovapd\0" "vmovaps\0" "vmovd\0"
  "vmovddup\0" "vmovdqa\0" "vmovdqa32\0" "vmovdqa64\0" "vmovdqu\0"
  "vmovdqu16\0" "vmovdqu32\0" "vmovdqu64\0" "vmovdqu8\0" "vmovhlps\0"
  "vmovhpd\0" "vmovhps\0" "vmovlhps\0" "vmovlpd\0" "vmovlps\0" "vmovmskpd\0"
  "vmovmskps\0" "vmovntdq\0" "vmovntdqa\0" "vmovntpd\0" "vmovntps\0" "vmovq\0"
  "vmovsd\0" "vmovshdup\0" "vmovsldup\0" "vmovss\0" "vmovupd\0" "vmovups\0"
  "vmpsadbw\0" "vmulpd\0" "vmulps\0" "vmulsd\0" "vmulss\0" "vorpd\0" "vorps\0"
  "vp4dpwssd\0" "vp4dpwssds\0" "vpabsb\0" "vpabsd\0" "vpabsq\0" "vpabsw\0"
  "vpackssdw\0" "vpacksswb\0" "vpackusdw\0" "vpackuswb\0" "vpaddb\0" "vpaddd\0"
  "vpaddq\0" "vpaddsb\0" "vpaddsw\0" "vpaddusb\0" "vpaddusw\0" "vpaddw\0"
  "vpalignr\0" "vpand\0" "vpandd\0" "vpandn\0" "vpandnd\0" "vpandnq\0"
  "vpandq\0" "vpavgb\0" "vpavgw\0" "vpblendd\0" "vpblendvb\0" "vpblendw\0"
  "vpbroadcastb\0" "vpbroadcastd\0" "vpbroadcastmb2d\0" "vpbroadcastmb2q\0"
  "vpbroadcastq\0" "vpbroadcastw\0" "vpclmulqdq\0" "vpcmov\0" "vpcmpb\0"
  "vpcmpd\0" "vpcmpeqb\0" "vpcmpeqd\0" "vpcmpeqq\0" "vpcmpeqw\0" "vpcmpestri\0"
  "vpcmpestrm\0" "vpcmpgtb\0" "vpcmpgtd\0" "vpcmpgtq\0" "vpcmpgtw\0"
  "vpcmpistri\0" "vpcmpistrm\0" "vpcmpq\0" "vpcmpub\0" "vpcmpud\0" "vpcmpuq\0"
  "vpcmpuw\0" "vpcmpw\0" "vpcomb\0" "vpcomd\0" "vpcompressd\0" "vpcompressq\0"
  "vpcomq\0" "vpcomub\0" "vpcomud\0" "vpcomuq\0" "vpcomuw\0" "vpcomw\0"
  "vpconflictd\0" "vpconflictq\0" "vperm2f128\0" "vperm2i128\0" "vpermb\0"
  "vpermd\0" "vpermi2b\0" "vpermi2d\0" "vpermi2pd\0" "vpermi2ps\0" "vpermi2q\0"
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
  "vpmullq\0" "vpmullw\0" "vpmultishiftqb\0" "vpmuludq\0" "vpopcntd\0"
  "vpopcntq\0" "vpor\0" "vpord\0" "vporq\0" "vpperm\0" "vprold\0" "vprolq\0"
  "vprolvd\0" "vprolvq\0" "vprord\0" "vprorq\0" "vprorvd\0" "vprorvq\0"
  "vprotb\0" "vprotd\0" "vprotq\0" "vprotw\0" "vpsadbw\0" "vpscatterdd\0"
  "vpscatterdq\0" "vpscatterqd\0" "vpscatterqq\0" "vpshab\0" "vpshad\0"
  "vpshaq\0" "vpshaw\0" "vpshlb\0" "vpshld\0" "vpshlq\0" "vpshlw\0" "vpshufb\0"
  "vpshufd\0" "vpshufhw\0" "vpshuflw\0" "vpsignb\0" "vpsignd\0" "vpsignw\0"
  "vpslld\0" "vpslldq\0" "vpsllq\0" "vpsllvd\0" "vpsllvq\0" "vpsllvw\0"
  "vpsllw\0" "vpsrad\0" "vpsraq\0" "vpsravd\0" "vpsravq\0" "vpsravw\0"
  "vpsraw\0" "vpsrld\0" "vpsrldq\0" "vpsrlq\0" "vpsrlvd\0" "vpsrlvq\0"
  "vpsrlvw\0" "vpsrlw\0" "vpsubb\0" "vpsubd\0" "vpsubq\0" "vpsubsb\0"
  "vpsubsw\0" "vpsubusb\0" "vpsubusw\0" "vpsubw\0" "vpternlogd\0"
  "vpternlogq\0" "vptest\0" "vptestmb\0" "vptestmd\0" "vptestmq\0" "vptestmw\0"
  "vptestnmb\0" "vptestnmd\0" "vptestnmq\0" "vptestnmw\0" "vpunpckhbw\0"
  "vpunpckhdq\0" "vpunpckhqdq\0" "vpunpckhwd\0" "vpunpcklbw\0" "vpunpckldq\0"
  "vpunpcklqdq\0" "vpunpcklwd\0" "vpxor\0" "vpxord\0" "vpxorq\0" "vrangepd\0"
  "vrangeps\0" "vrangesd\0" "vrangess\0" "vrcp14pd\0" "vrcp14ps\0" "vrcp14sd\0"
  "vrcp14ss\0" "vrcp28pd\0" "vrcp28ps\0" "vrcp28sd\0" "vrcp28ss\0" "vrcpps\0"
  "vrcpss\0" "vreducepd\0" "vreduceps\0" "vreducesd\0" "vreducess\0"
  "vrndscalepd\0" "vrndscaleps\0" "vrndscalesd\0" "vrndscaless\0" "vroundpd\0"
  "vroundps\0" "vroundsd\0" "vroundss\0" "vrsqrt14pd\0" "vrsqrt14ps\0"
  "vrsqrt14sd\0" "vrsqrt14ss\0" "vrsqrt28pd\0" "vrsqrt28ps\0" "vrsqrt28sd\0"
  "vrsqrt28ss\0" "vrsqrtps\0" "vrsqrtss\0" "vscalefpd\0" "vscalefps\0"
  "vscalefsd\0" "vscalefss\0" "vscatterdpd\0" "vscatterdps\0"
  "vscatterpf0dpd\0" "vscatterpf0dps\0" "vscatterpf0qpd\0" "vscatterpf0qps\0"
  "vscatterpf1dpd\0" "vscatterpf1dps\0" "vscatterpf1qpd\0" "vscatterpf1qps\0"
  "vscatterqpd\0" "vscatterqps\0" "vshuff32x4\0" "vshuff64x2\0" "vshufi32x4\0"
  "vshufi64x2\0" "vshufpd\0" "vshufps\0" "vsqrtpd\0" "vsqrtps\0" "vsqrtsd\0"
  "vsqrtss\0" "vstmxcsr\0" "vsubpd\0" "vsubps\0" "vsubsd\0" "vsubss\0"
  "vtestpd\0" "vtestps\0" "vucomisd\0" "vucomiss\0" "vunpckhpd\0" "vunpckhps\0"
  "vunpcklpd\0" "vunpcklps\0" "vxorpd\0" "vxorps\0" "vzeroall\0" "vzeroupper\0"
  "wbinvd\0" "wrfsbase\0" "wrgsbase\0" "wrmsr\0" "xadd\0" "xgetbv\0"
  "xrstors\0" "xrstors64\0" "xsavec\0" "xsavec64\0" "xsaveopt\0" "xsaveopt64\0"
  "xsaves\0" "xsaves64\0" "xsetbv";

enum {
  kX86InstMaxLength = 16
};

struct InstNameAZ {
  uint16_t start;
  uint16_t end;
};

static const InstNameAZ X86InstNameAZ[26] = {
  { X86Inst::kIdAaa       , X86Inst::kIdArpl       + 1 },
  { X86Inst::kIdBextr     , X86Inst::kIdBzhi       + 1 },
  { X86Inst::kIdCall      , X86Inst::kIdCwde       + 1 },
  { X86Inst::kIdDaa       , X86Inst::kIdDpps       + 1 },
  { X86Inst::kIdEmms      , X86Inst::kIdExtrq      + 1 },
  { X86Inst::kIdF2xm1     , X86Inst::kIdFyl2xp1    + 1 },
  { X86Inst::kIdNone      , X86Inst::kIdNone       + 1 },
  { X86Inst::kIdHaddpd    , X86Inst::kIdHsubps     + 1 },
  { X86Inst::kIdIdiv      , X86Inst::kIdInvpcid    + 1 },
  { X86Inst::kIdJa        , X86Inst::kIdJz         + 1 },
  { X86Inst::kIdKaddb     , X86Inst::kIdKxorw      + 1 },
  { X86Inst::kIdLahf      , X86Inst::kIdLzcnt      + 1 },
  { X86Inst::kIdMaskmovdqu, X86Inst::kIdMwait      + 1 },
  { X86Inst::kIdNeg       , X86Inst::kIdNot        + 1 },
  { X86Inst::kIdOr        , X86Inst::kIdOuts       + 1 },
  { X86Inst::kIdPabsb     , X86Inst::kIdPxor       + 1 },
  { X86Inst::kIdNone      , X86Inst::kIdNone       + 1 },
  { X86Inst::kIdRcl       , X86Inst::kIdRsqrtss    + 1 },
  { X86Inst::kIdSahf      , X86Inst::kIdSysret64   + 1 },
  { X86Inst::kIdT1mskc    , X86Inst::kIdTzmsk      + 1 },
  { X86Inst::kIdUcomisd   , X86Inst::kIdUnpcklps   + 1 },
  { X86Inst::kIdV4fmaddps , X86Inst::kIdVzeroupper + 1 },
  { X86Inst::kIdWbinvd    , X86Inst::kIdWrmsr      + 1 },
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
#define FLAG(flag) X86Inst::kOp##flag
#define MEM(mem) X86Inst::kMemOp##mem
#define OSIGNATURE(flags, memFlags, extFlags, regId) \
  { uint32_t(flags), uint16_t(memFlags), uint8_t(extFlags), uint8_t(regId) }
const X86Inst::OSignature X86InstDB::oSignatureData[] = {
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
  OSIGNATURE(FLAG(R) | FLAG(Gpw) | FLAG(Gpq) | FLAG(Mem) | FLAG(I8) | FLAG(I16) | FLAG(I32), MEM(M16) | MEM(M64), 0, 0x00),
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
  OSIGNATURE(FLAG(R) | FLAG(Bnd), 0, 0, 0x00),
  OSIGNATURE(FLAG(W) | FLAG(Bnd), 0, 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Bnd) | FLAG(Mem), MEM(Any), 0, 0x00),
  OSIGNATURE(FLAG(W) | FLAG(Bnd) | FLAG(Mem), MEM(Any), 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Gpq) | FLAG(Mem) | FLAG(I32) | FLAG(I64) | FLAG(Rel32), MEM(M64), 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(GpbLo) | FLAG(GpbHi) | FLAG(Gpw) | FLAG(Gpd) | FLAG(Mem), MEM(M8) | MEM(M16) | MEM(M32), 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(GpbLo) | FLAG(GpbHi) | FLAG(Gpq) | FLAG(Mem), MEM(M8) | MEM(M64), 0, 0x00),
  OSIGNATURE(FLAG(X) | FLAG(Gpw) | FLAG(Gpd), 0, 0, 0x00),
  OSIGNATURE(FLAG(X) | FLAG(Fp) | FLAG(Mem), MEM(M32) | MEM(M64), 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Implicit) | FLAG(Gpw) | FLAG(Gpd), 0, 0, 0x02),
  OSIGNATURE(FLAG(R) | FLAG(I32) | FLAG(I64) | FLAG(Rel8), 0, 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Implicit) | FLAG(Gpd) | FLAG(Gpq), 0, 0, 0x02),
  OSIGNATURE(FLAG(R) | FLAG(Gpq) | FLAG(Mem) | FLAG(I32) | FLAG(I64) | FLAG(Rel8) | FLAG(Rel32), MEM(M64), 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(GpbLo) | FLAG(GpbHi) | FLAG(Gpw) | FLAG(Gpd) | FLAG(Gpq) | FLAG(K) | FLAG(Mem), MEM(M8), 0, 0x00),
  OSIGNATURE(FLAG(W) | FLAG(GpbLo) | FLAG(GpbHi) | FLAG(Gpw) | FLAG(Gpd) | FLAG(Gpq) | FLAG(Mem), MEM(M8), 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(K), 0, 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Gpd) | FLAG(Gpq) | FLAG(K) | FLAG(Mem), MEM(M32), 0, 0x00),
  OSIGNATURE(FLAG(W) | FLAG(Gpd) | FLAG(Gpq) | FLAG(Mem), MEM(M32), 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Gpq) | FLAG(K) | FLAG(Mem), MEM(M64), 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Gpw) | FLAG(Gpd) | FLAG(Gpq) | FLAG(K) | FLAG(Mem), MEM(M16), 0, 0x00),
  OSIGNATURE(FLAG(W) | FLAG(Gpw) | FLAG(Gpd) | FLAG(Gpq) | FLAG(Mem), MEM(M16), 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Gpw) | FLAG(Gpd) | FLAG(Mem), MEM(M16), 0, 0x00),
  OSIGNATURE(FLAG(X) | FLAG(Implicit) | FLAG(Gpw) | FLAG(Gpd), 0, 0, 0x02),
  OSIGNATURE(FLAG(X) | FLAG(Implicit) | FLAG(Gpd) | FLAG(Gpq), 0, 0, 0x02),
  OSIGNATURE(FLAG(W) | FLAG(Gpd) | FLAG(Gpq), 0, 0, 0x00),
  OSIGNATURE(FLAG(W) | FLAG(Mm) | FLAG(Xmm), 0, 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Gpd) | FLAG(Gpq) | FLAG(Mem), MEM(M32), 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Mm) | FLAG(Xmm), 0, 0, 0x00),
  OSIGNATURE(FLAG(W) | FLAG(Gpw) | FLAG(Gpd) | FLAG(Gpq), 0, 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Implicit) | FLAG(Gpd), 0, 0, 0x04),
  OSIGNATURE(FLAG(R) | FLAG(Implicit) | FLAG(Gpq), 0, 0, 0x04),
  OSIGNATURE(FLAG(R) | FLAG(Gpw) | FLAG(Gpd) | FLAG(Mem), MEM(M16) | MEM(M32), 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Mm) | FLAG(Mem), MEM(M64), 0, 0x00),
  OSIGNATURE(FLAG(X) | FLAG(Mm), 0, 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Mm) | FLAG(Mem) | FLAG(I8), MEM(M64), 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Xmm) | FLAG(Ymm) | FLAG(Mem), MEM(M128) | MEM(M256), 0, 0x00),
  OSIGNATURE(FLAG(W) | FLAG(Xmm) | FLAG(Ymm) | FLAG(Zmm), 0, 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Xmm) | FLAG(Mem), MEM(M8), 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Gpd) | FLAG(Gpq), 0, 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(GpbLo) | FLAG(GpbHi) | FLAG(Gpw) | FLAG(Gpd) | FLAG(Gpq) | FLAG(Mem), MEM(M8), 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Xmm) | FLAG(I8), 0, 0, 0x00),
  OSIGNATURE(FLAG(W) | FLAG(Vm), MEM(Vm64x) | MEM(Vm64y), 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Xmm) | FLAG(Ymm), 0, 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Implicit) | FLAG(Xmm), 0, 0, 0x01),
  OSIGNATURE(FLAG(R) | FLAG(Mem), MEM(Mib), 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Mem), MEM(Any), 0, 0x00),
  OSIGNATURE(FLAG(W) | FLAG(Mem), MEM(Mib), 0, 0x00),
  OSIGNATURE(FLAG(X) | FLAG(Gpd) | FLAG(Gpq), 0, 0, 0x00),
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
  OSIGNATURE(FLAG(R) | FLAG(I32) | FLAG(I64) | FLAG(Rel8) | FLAG(Rel32), 0, 0, 0x00),
  OSIGNATURE(FLAG(W) | FLAG(Implicit) | FLAG(GpbHi), 0, 0, 0x01),
  OSIGNATURE(FLAG(R) | FLAG(Gpw) | FLAG(Gpd) | FLAG(Gpq) | FLAG(Mem), MEM(M16), 0, 0x00),
  OSIGNATURE(FLAG(W) | FLAG(Implicit) | FLAG(GpbLo) | FLAG(Gpw) | FLAG(Gpd) | FLAG(Gpq), 0, 0, 0x01),
  OSIGNATURE(FLAG(R) | FLAG(Implicit) | FLAG(Mem), MEM(BaseOnly) | MEM(Ds), 0, 0x80),
  OSIGNATURE(FLAG(R) | FLAG(GpbLo) | FLAG(Gpw) | FLAG(Gpd), 0, 0, 0x01),
  OSIGNATURE(FLAG(R) | FLAG(Mem), MEM(BaseOnly) | MEM(Ds), 0, 0x40),
  OSIGNATURE(FLAG(W) | FLAG(Implicit) | FLAG(Gpd), 0, 0, 0x02),
  OSIGNATURE(FLAG(W) | FLAG(Implicit) | FLAG(Xmm), 0, 0, 0x01),
  OSIGNATURE(FLAG(X) | FLAG(Mm) | FLAG(Xmm), 0, 0, 0x00),
  OSIGNATURE(FLAG(W) | FLAG(Implicit) | FLAG(Gpd), 0, 0, 0x01),
  OSIGNATURE(FLAG(R) | FLAG(Implicit) | FLAG(GpbHi), 0, 0, 0x01),
  OSIGNATURE(FLAG(R) | FLAG(Implicit) | FLAG(GpbLo) | FLAG(Gpw) | FLAG(Gpd) | FLAG(Gpq), 0, 0, 0x01),
  OSIGNATURE(FLAG(W) | FLAG(Mem), MEM(Any), 0, 0x00),
  OSIGNATURE(FLAG(W) | FLAG(Ymm) | FLAG(Zmm), 0, 0, 0x00),
  OSIGNATURE(FLAG(X) | FLAG(Gpw) | FLAG(Gpd) | FLAG(Gpq) | FLAG(Mem), MEM(M16), 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Ymm) | FLAG(Zmm), 0, 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Xmm) | FLAG(Ymm) | FLAG(Zmm) | FLAG(Mem), MEM(M128) | MEM(M256) | MEM(M512), 0, 0x00),
  OSIGNATURE(FLAG(R) | FLAG(Xmm) | FLAG(Ymm) | FLAG(Zmm), 0, 0, 0x00)
};
#undef OSIGNATURE
#undef MEM
#undef FLAG

#define ISIGNATURE(count, x86, x64, implicit, o0, o1, o2, o3, o4, o5) \
  { count, (x86 ? uint8_t(X86Inst::kArchMaskX86) : uint8_t(0)) |      \
           (x64 ? uint8_t(X86Inst::kArchMaskX64) : uint8_t(0)) ,      \
    implicit,                                                         \
    0,                                                                \
    { o0, o1, o2, o3, o4, o5 }                                        \
  }
const X86Inst::ISignature X86InstDB::iSignatureData[] = {
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
  ISIGNATURE(3, 1, 1, 2, 90 , 44 , 12 , 0  , 0  , 0  ), //      {X:<dx>, X:<ax>, R:r16|m16}
  ISIGNATURE(3, 1, 1, 2, 91 , 47 , 39 , 0  , 0  , 0  ), //      {X:<edx>, X:<eax>, R:r32|m32}
  ISIGNATURE(3, 0, 1, 2, 92 , 49 , 16 , 0  , 0  , 0  ), //      {X:<rdx>, X:<rax>, R:r64|m64}
  ISIGNATURE(1, 1, 1, 0, 93 , 0  , 0  , 0  , 0  , 0  ), // #116 {W:r16|m16|r64|m64}
  ISIGNATURE(1, 1, 0, 0, 21 , 0  , 0  , 0  , 0  , 0  ), //      {W:r32|m32}
  ISIGNATURE(1, 1, 0, 0, 94 , 0  , 0  , 0  , 0  , 0  ), //      {W:ds|es|ss}
  ISIGNATURE(1, 1, 1, 0, 95 , 0  , 0  , 0  , 0  , 0  ), //      {W:fs|gs}
  ISIGNATURE(1, 1, 1, 0, 96 , 0  , 0  , 0  , 0  , 0  ), // #120 {R:r16|m16|r64|m64|i8|i16|i32}
  ISIGNATURE(1, 1, 0, 0, 39 , 0  , 0  , 0  , 0  , 0  ), //      {R:r32|m32}
  ISIGNATURE(1, 1, 0, 0, 97 , 0  , 0  , 0  , 0  , 0  ), //      {R:cs|ss|ds|es}
  ISIGNATURE(1, 1, 1, 0, 98 , 0  , 0  , 0  , 0  , 0  ), //      {R:fs|gs}
  ISIGNATURE(4, 1, 1, 0, 66 , 65 , 65 , 69 , 0  , 0  ), // #124 {W:xmm, R:xmm, R:xmm, R:xmm|m128}
  ISIGNATURE(4, 1, 1, 0, 66 , 65 , 69 , 65 , 0  , 0  ), // #125 {W:xmm, R:xmm, R:xmm|m128, R:xmm}
  ISIGNATURE(4, 1, 1, 0, 71 , 74 , 74 , 72 , 0  , 0  ), //      {W:ymm, R:ymm, R:ymm, R:ymm|m256}
  ISIGNATURE(4, 1, 1, 0, 71 , 74 , 72 , 74 , 0  , 0  ), //      {W:ymm, R:ymm, R:ymm|m256, R:ymm}
  ISIGNATURE(3, 1, 1, 0, 66 , 99 , 65 , 0  , 0  , 0  ), // #128 {W:xmm, R:vm64x|vm64y, R:xmm}
  ISIGNATURE(2, 1, 1, 0, 66 , 83 , 0  , 0  , 0  , 0  ), //      {W:xmm, R:vm64x}
  ISIGNATURE(2, 1, 1, 0, 71 , 84 , 0  , 0  , 0  , 0  ), //      {W:ymm, R:vm64y}
  ISIGNATURE(2, 1, 1, 0, 75 , 85 , 0  , 0  , 0  , 0  ), //      {W:zmm, R:vm64z}
  ISIGNATURE(3, 1, 1, 0, 100, 65 , 65 , 0  , 0  , 0  ), // #132 {W:m128, R:xmm, R:xmm}
  ISIGNATURE(3, 1, 1, 0, 101, 74 , 74 , 0  , 0  , 0  ), //      {W:m256, R:ymm, R:ymm}
  ISIGNATURE(3, 1, 1, 0, 66 , 65 , 102, 0  , 0  , 0  ), //      {W:xmm, R:xmm, R:m128}
  ISIGNATURE(3, 1, 1, 0, 71 , 74 , 103, 0  , 0  , 0  ), //      {W:ymm, R:ymm, R:m256}
  ISIGNATURE(5, 1, 1, 0, 66 , 65 , 69 , 65 , 104, 0  ), // #136 {W:xmm, R:xmm, R:xmm|m128, R:xmm, R:i4}
  ISIGNATURE(5, 1, 1, 0, 66 , 65 , 65 , 69 , 104, 0  ), //      {W:xmm, R:xmm, R:xmm, R:xmm|m128, R:i4}
  ISIGNATURE(5, 1, 1, 0, 71 , 74 , 72 , 74 , 104, 0  ), //      {W:ymm, R:ymm, R:ymm|m256, R:ymm, R:i4}
  ISIGNATURE(5, 1, 1, 0, 71 , 74 , 74 , 72 , 104, 0  ), //      {W:ymm, R:ymm, R:ymm, R:ymm|m256, R:i4}
  ISIGNATURE(3, 1, 1, 0, 71 , 72 , 27 , 0  , 0  , 0  ), // #140 {W:ymm, R:ymm|m256, R:i8}
  ISIGNATURE(3, 1, 1, 0, 71 , 74 , 72 , 0  , 0  , 0  ), // #141 {W:ymm, R:ymm, R:ymm|m256}
  ISIGNATURE(3, 1, 1, 0, 75 , 78 , 76 , 0  , 0  , 0  ), //      {W:zmm, R:zmm, R:zmm|m512}
  ISIGNATURE(3, 1, 1, 0, 75 , 76 , 27 , 0  , 0  , 0  ), //      {W:zmm, R:zmm|m512, R:i8}
  ISIGNATURE(2, 1, 1, 0, 31 , 35 , 0  , 0  , 0  , 0  ), // #144 {X:r8lo|r8hi|m8, X:r8lo|r8hi}
  ISIGNATURE(2, 1, 1, 0, 28 , 37 , 0  , 0  , 0  , 0  ), //      {X:r16|m16, X:r16}
  ISIGNATURE(2, 1, 1, 0, 33 , 38 , 0  , 0  , 0  , 0  ), //      {X:r32|m32, X:r32}
  ISIGNATURE(2, 0, 1, 0, 34 , 40 , 0  , 0  , 0  , 0  ), //      {X:r64|m64, X:r64}
  ISIGNATURE(2, 1, 1, 0, 12 , 105, 0  , 0  , 0  , 0  ), // #148 {R:r16|m16, R:r16|i8}
  ISIGNATURE(2, 1, 1, 0, 39 , 106, 0  , 0  , 0  , 0  ), //      {R:r32|m32, R:r32|i8}
  ISIGNATURE(2, 0, 1, 0, 16 , 107, 0  , 0  , 0  , 0  ), //      {R:r64|m64, R:r64|i8}
  ISIGNATURE(2, 1, 1, 0, 28 , 105, 0  , 0  , 0  , 0  ), // #151 {X:r16|m16, R:r16|i8}
  ISIGNATURE(2, 1, 1, 0, 33 , 106, 0  , 0  , 0  , 0  ), //      {X:r32|m32, R:r32|i8}
  ISIGNATURE(2, 0, 1, 0, 34 , 107, 0  , 0  , 0  , 0  ), //      {X:r64|m64, R:r64|i8}
  ISIGNATURE(1, 1, 1, 0, 108, 0  , 0  , 0  , 0  , 0  ), // #154 {X:m32|m64}
  ISIGNATURE(2, 1, 1, 0, 109, 110, 0  , 0  , 0  , 0  ), //      {X:fp0, R:fp}
  ISIGNATURE(2, 1, 1, 0, 111, 112, 0  , 0  , 0  , 0  ), //      {X:fp, R:fp0}
  ISIGNATURE(2, 1, 1, 0, 17 , 12 , 0  , 0  , 0  , 0  ), // #157 {W:r16, R:r16|m16}
  ISIGNATURE(2, 1, 1, 0, 13 , 39 , 0  , 0  , 0  , 0  ), // #158 {W:r32, R:r32|m32}
  ISIGNATURE(2, 0, 1, 0, 19 , 16 , 0  , 0  , 0  , 0  ), //      {W:r64, R:r64|m64}
  ISIGNATURE(3, 1, 1, 0, 28 , 43 , 113, 0  , 0  , 0  ), // #160 {X:r16|m16, R:r16, R:i8|cl}
  ISIGNATURE(3, 1, 1, 0, 33 , 6  , 113, 0  , 0  , 0  ), //      {X:r32|m32, R:r32, R:i8|cl}
  ISIGNATURE(3, 0, 1, 0, 34 , 25 , 113, 0  , 0  , 0  ), //      {X:r64|m64, R:r64, R:i8|cl}
  ISIGNATURE(3, 1, 1, 0, 66 , 65 , 69 , 0  , 0  , 0  ), // #163 {W:xmm, R:xmm, R:xmm|m128}
  ISIGNATURE(3, 1, 1, 0, 71 , 74 , 72 , 0  , 0  , 0  ), //      {W:ymm, R:ymm, R:ymm|m256}
  ISIGNATURE(3, 1, 1, 0, 75 , 78 , 76 , 0  , 0  , 0  ), //      {W:zmm, R:zmm, R:zmm|m512}
  ISIGNATURE(4, 1, 1, 0, 66 , 65 , 69 , 27 , 0  , 0  ), // #166 {W:xmm, R:xmm, R:xmm|m128, R:i8}
  ISIGNATURE(4, 1, 1, 0, 71 , 74 , 72 , 27 , 0  , 0  ), // #167 {W:ymm, R:ymm, R:ymm|m256, R:i8}
  ISIGNATURE(4, 1, 1, 0, 75 , 78 , 76 , 27 , 0  , 0  ), //      {W:zmm, R:zmm, R:zmm|m512, R:i8}
  ISIGNATURE(4, 1, 1, 0, 114, 65 , 69 , 27 , 0  , 0  ), // #169 {W:xmm|k, R:xmm, R:xmm|m128, R:i8}
  ISIGNATURE(4, 1, 1, 0, 115, 74 , 72 , 27 , 0  , 0  ), //      {W:ymm|k, R:ymm, R:ymm|m256, R:i8}
  ISIGNATURE(4, 1, 1, 0, 116, 78 , 76 , 27 , 0  , 0  ), //      {W:k, R:zmm, R:zmm|m512, R:i8}
  ISIGNATURE(2, 1, 1, 0, 70 , 65 , 0  , 0  , 0  , 0  ), // #172 {W:xmm|m128, R:xmm}
  ISIGNATURE(2, 1, 1, 0, 73 , 74 , 0  , 0  , 0  , 0  ), //      {W:ymm|m256, R:ymm}
  ISIGNATURE(2, 1, 1, 0, 77 , 78 , 0  , 0  , 0  , 0  ), //      {W:zmm|m512, R:zmm}
  ISIGNATURE(2, 1, 1, 0, 66 , 67 , 0  , 0  , 0  , 0  ), // #175 {W:xmm, R:xmm|m64}
  ISIGNATURE(2, 1, 1, 0, 71 , 69 , 0  , 0  , 0  , 0  ), //      {W:ymm, R:xmm|m128}
  ISIGNATURE(2, 1, 1, 0, 75 , 72 , 0  , 0  , 0  , 0  ), //      {W:zmm, R:ymm|m256}
  ISIGNATURE(2, 1, 1, 0, 66 , 69 , 0  , 0  , 0  , 0  ), // #178 {W:xmm, R:xmm|m128}
  ISIGNATURE(2, 1, 1, 0, 71 , 72 , 0  , 0  , 0  , 0  ), //      {W:ymm, R:ymm|m256}
  ISIGNATURE(2, 1, 1, 0, 75 , 76 , 0  , 0  , 0  , 0  ), //      {W:zmm, R:zmm|m512}
  ISIGNATURE(2, 1, 1, 0, 66 , 117, 0  , 0  , 0  , 0  ), // #181 {W:xmm, R:xmm|m128|ymm|m256|m64}
  ISIGNATURE(2, 1, 1, 0, 71 , 69 , 0  , 0  , 0  , 0  ), //      {W:ymm, R:xmm|m128}
  ISIGNATURE(2, 1, 1, 0, 75 , 72 , 0  , 0  , 0  , 0  ), //      {W:zmm, R:ymm|m256}
  ISIGNATURE(3, 1, 1, 0, 68 , 65 , 27 , 0  , 0  , 0  ), // #184 {W:xmm|m64, R:xmm, R:i8}
  ISIGNATURE(3, 1, 1, 0, 70 , 74 , 27 , 0  , 0  , 0  ), // #185 {W:xmm|m128, R:ymm, R:i8}
  ISIGNATURE(3, 1, 1, 0, 73 , 78 , 27 , 0  , 0  , 0  ), // #186 {W:ymm|m256, R:zmm, R:i8}
  ISIGNATURE(4, 1, 1, 0, 118, 65 , 69 , 27 , 0  , 0  ), // #187 {X:xmm, R:xmm, R:xmm|m128, R:i8}
  ISIGNATURE(4, 1, 1, 0, 119, 74 , 72 , 27 , 0  , 0  ), //      {X:ymm, R:ymm, R:ymm|m256, R:i8}
  ISIGNATURE(4, 1, 1, 0, 120, 78 , 76 , 27 , 0  , 0  ), //      {X:zmm, R:zmm, R:zmm|m512, R:i8}
  ISIGNATURE(3, 1, 1, 0, 118, 65 , 69 , 0  , 0  , 0  ), // #190 {X:xmm, R:xmm, R:xmm|m128}
  ISIGNATURE(3, 1, 1, 0, 119, 74 , 72 , 0  , 0  , 0  ), //      {X:ymm, R:ymm, R:ymm|m256}
  ISIGNATURE(3, 1, 1, 0, 120, 78 , 76 , 0  , 0  , 0  ), //      {X:zmm, R:zmm, R:zmm|m512}
  ISIGNATURE(3, 1, 1, 0, 66 , 69 , 27 , 0  , 0  , 0  ), // #193 {W:xmm, R:xmm|m128, R:i8}
  ISIGNATURE(3, 1, 1, 0, 71 , 72 , 27 , 0  , 0  , 0  ), //      {W:ymm, R:ymm|m256, R:i8}
  ISIGNATURE(3, 1, 1, 0, 75 , 76 , 27 , 0  , 0  , 0  ), //      {W:zmm, R:zmm|m512, R:i8}
  ISIGNATURE(2, 1, 1, 0, 66 , 67 , 0  , 0  , 0  , 0  ), // #196 {W:xmm, R:xmm|m64}
  ISIGNATURE(2, 1, 1, 0, 71 , 72 , 0  , 0  , 0  , 0  ), //      {W:ymm, R:ymm|m256}
  ISIGNATURE(2, 1, 1, 0, 75 , 76 , 0  , 0  , 0  , 0  ), //      {W:zmm, R:zmm|m512}
  ISIGNATURE(2, 1, 1, 0, 100, 65 , 0  , 0  , 0  , 0  ), // #199 {W:m128, R:xmm}
  ISIGNATURE(2, 1, 1, 0, 101, 74 , 0  , 0  , 0  , 0  ), //      {W:m256, R:ymm}
  ISIGNATURE(2, 1, 1, 0, 121, 78 , 0  , 0  , 0  , 0  ), //      {W:m512, R:zmm}
  ISIGNATURE(2, 1, 1, 0, 66 , 102, 0  , 0  , 0  , 0  ), // #202 {W:xmm, R:m128}
  ISIGNATURE(2, 1, 1, 0, 71 , 103, 0  , 0  , 0  , 0  ), //      {W:ymm, R:m256}
  ISIGNATURE(2, 1, 1, 0, 75 , 122, 0  , 0  , 0  , 0  ), //      {W:zmm, R:m512}
  ISIGNATURE(2, 0, 1, 0, 7  , 65 , 0  , 0  , 0  , 0  ), // #205 {W:r64|m64, R:xmm}
  ISIGNATURE(2, 1, 1, 0, 66 , 123, 0  , 0  , 0  , 0  ), //      {W:xmm, R:xmm|m64|r64}
  ISIGNATURE(2, 1, 1, 0, 68 , 65 , 0  , 0  , 0  , 0  ), //      {W:xmm|m64, R:xmm}
  ISIGNATURE(2, 1, 1, 0, 60 , 65 , 0  , 0  , 0  , 0  ), // #208 {W:m64, R:xmm}
  ISIGNATURE(2, 1, 1, 0, 66 , 57 , 0  , 0  , 0  , 0  ), //      {W:xmm, R:m64}
  ISIGNATURE(3, 1, 1, 0, 66 , 65 , 65 , 0  , 0  , 0  ), // #210 {W:xmm, R:xmm, R:xmm}
  ISIGNATURE(2, 1, 1, 0, 124, 65 , 0  , 0  , 0  , 0  ), // #211 {W:m32|m64, R:xmm}
  ISIGNATURE(2, 1, 1, 0, 66 , 125, 0  , 0  , 0  , 0  ), //      {W:xmm, R:m32|m64}
  ISIGNATURE(3, 1, 1, 0, 66 , 65 , 65 , 0  , 0  , 0  ), //      {W:xmm, R:xmm, R:xmm}
  ISIGNATURE(4, 1, 1, 0, 116, 65 , 69 , 27 , 0  , 0  ), // #214 {W:k, R:xmm, R:xmm|m128, R:i8}
  ISIGNATURE(4, 1, 1, 0, 116, 74 , 72 , 27 , 0  , 0  ), //      {W:k, R:ymm, R:ymm|m256, R:i8}
  ISIGNATURE(4, 1, 1, 0, 116, 78 , 76 , 27 , 0  , 0  ), //      {W:k, R:zmm, R:zmm|m512, R:i8}
  ISIGNATURE(3, 1, 1, 0, 114, 65 , 69 , 0  , 0  , 0  ), // #217 {W:xmm|k, R:xmm, R:xmm|m128}
  ISIGNATURE(3, 1, 1, 0, 115, 74 , 72 , 0  , 0  , 0  ), //      {W:ymm|k, R:ymm, R:ymm|m256}
  ISIGNATURE(3, 1, 1, 0, 116, 78 , 76 , 0  , 0  , 0  ), //      {W:k, R:zmm, R:zmm|m512}
  ISIGNATURE(2, 1, 1, 0, 126, 65 , 0  , 0  , 0  , 0  ), // #220 {W:xmm|m32, R:xmm}
  ISIGNATURE(2, 1, 1, 0, 68 , 74 , 0  , 0  , 0  , 0  ), //      {W:xmm|m64, R:ymm}
  ISIGNATURE(2, 1, 1, 0, 70 , 78 , 0  , 0  , 0  , 0  ), //      {W:xmm|m128, R:zmm}
  ISIGNATURE(2, 1, 1, 0, 68 , 65 , 0  , 0  , 0  , 0  ), // #223 {W:xmm|m64, R:xmm}
  ISIGNATURE(2, 1, 1, 0, 70 , 74 , 0  , 0  , 0  , 0  ), //      {W:xmm|m128, R:ymm}
  ISIGNATURE(2, 1, 1, 0, 73 , 78 , 0  , 0  , 0  , 0  ), //      {W:ymm|m256, R:zmm}
  ISIGNATURE(2, 1, 1, 0, 127, 65 , 0  , 0  , 0  , 0  ), // #226 {W:xmm|m16, R:xmm}
  ISIGNATURE(2, 1, 1, 0, 126, 74 , 0  , 0  , 0  , 0  ), //      {W:xmm|m32, R:ymm}
  ISIGNATURE(2, 1, 1, 0, 68 , 78 , 0  , 0  , 0  , 0  ), //      {W:xmm|m64, R:zmm}
  ISIGNATURE(2, 1, 1, 0, 66 , 128, 0  , 0  , 0  , 0  ), // #229 {W:xmm, R:xmm|m32}
  ISIGNATURE(2, 1, 1, 0, 71 , 67 , 0  , 0  , 0  , 0  ), //      {W:ymm, R:xmm|m64}
  ISIGNATURE(2, 1, 1, 0, 75 , 69 , 0  , 0  , 0  , 0  ), //      {W:zmm, R:xmm|m128}
  ISIGNATURE(2, 1, 1, 0, 66 , 129, 0  , 0  , 0  , 0  ), // #232 {W:xmm, R:xmm|m16}
  ISIGNATURE(2, 1, 1, 0, 71 , 128, 0  , 0  , 0  , 0  ), //      {W:ymm, R:xmm|m32}
  ISIGNATURE(2, 1, 1, 0, 75 , 67 , 0  , 0  , 0  , 0  ), // #234 {W:zmm, R:xmm|m64}
  ISIGNATURE(2, 1, 1, 0, 66 , 130, 0  , 0  , 0  , 0  ), // #235 {W:xmm, R:xmm|m64|m32}
  ISIGNATURE(2, 1, 1, 0, 71 , 131, 0  , 0  , 0  , 0  ), //      {W:ymm, R:xmm|m128|m64}
  ISIGNATURE(2, 1, 1, 0, 75 , 69 , 0  , 0  , 0  , 0  ), //      {W:zmm, R:xmm|m128}
  ISIGNATURE(2, 1, 1, 0, 132, 65 , 0  , 0  , 0  , 0  ), // #238 {W:vm32x, R:xmm}
  ISIGNATURE(2, 1, 1, 0, 133, 74 , 0  , 0  , 0  , 0  ), //      {W:vm32y, R:ymm}
  ISIGNATURE(2, 1, 1, 0, 134, 78 , 0  , 0  , 0  , 0  ), //      {W:vm32z, R:zmm}
  ISIGNATURE(2, 1, 1, 0, 135, 65 , 0  , 0  , 0  , 0  ), // #241 {W:vm64x, R:xmm}
  ISIGNATURE(2, 1, 1, 0, 136, 74 , 0  , 0  , 0  , 0  ), //      {W:vm64y, R:ymm}
  ISIGNATURE(2, 1, 1, 0, 137, 78 , 0  , 0  , 0  , 0  ), //      {W:vm64z, R:zmm}
  ISIGNATURE(3, 1, 1, 0, 116, 65 , 69 , 0  , 0  , 0  ), // #244 {W:k, R:xmm, R:xmm|m128}
  ISIGNATURE(3, 1, 1, 0, 116, 74 , 72 , 0  , 0  , 0  ), //      {W:k, R:ymm, R:ymm|m256}
  ISIGNATURE(3, 1, 1, 0, 116, 78 , 76 , 0  , 0  , 0  ), //      {W:k, R:zmm, R:zmm|m512}
  ISIGNATURE(3, 1, 1, 0, 13 , 6  , 39 , 0  , 0  , 0  ), // #247 {W:r32, R:r32, R:r32|m32}
  ISIGNATURE(3, 0, 1, 0, 19 , 25 , 16 , 0  , 0  , 0  ), //      {W:r64, R:r64, R:r64|m64}
  ISIGNATURE(3, 1, 1, 0, 13 , 39 , 6  , 0  , 0  , 0  ), // #249 {W:r32, R:r32|m32, R:r32}
  ISIGNATURE(3, 0, 1, 0, 19 , 16 , 25 , 0  , 0  , 0  ), //      {W:r64, R:r64|m64, R:r64}
  ISIGNATURE(2, 1, 0, 0, 138, 39 , 0  , 0  , 0  , 0  ), // #251 {R:bnd, R:r32|m32}
  ISIGNATURE(2, 0, 1, 0, 138, 16 , 0  , 0  , 0  , 0  ), //      {R:bnd, R:r64|m64}
  ISIGNATURE(2, 1, 1, 0, 139, 140, 0  , 0  , 0  , 0  ), // #253 {W:bnd, R:bnd|mem}
  ISIGNATURE(2, 1, 1, 0, 141, 138, 0  , 0  , 0  , 0  ), //      {W:bnd|mem, R:bnd}
  ISIGNATURE(2, 1, 0, 0, 43 , 56 , 0  , 0  , 0  , 0  ), // #255 {R:r16, R:m32}
  ISIGNATURE(2, 1, 0, 0, 6  , 57 , 0  , 0  , 0  , 0  ), //      {R:r32, R:m64}
  ISIGNATURE(1, 1, 1, 0, 142, 0  , 0  , 0  , 0  , 0  ), // #257 {R:rel32|r64|m64}
  ISIGNATURE(1, 1, 0, 0, 39 , 0  , 0  , 0  , 0  , 0  ), //      {R:r32|m32}
  ISIGNATURE(2, 1, 1, 0, 38 , 143, 0  , 0  , 0  , 0  ), // #259 {X:r32, R:r8lo|r8hi|m8|r16|m16|r32|m32}
  ISIGNATURE(2, 0, 1, 0, 40 , 144, 0  , 0  , 0  , 0  ), //      {X:r64, R:r8lo|r8hi|m8|r64|m64}
  ISIGNATURE(1, 1, 0, 0, 145, 0  , 0  , 0  , 0  , 0  ), // #261 {X:r16|r32}
  ISIGNATURE(1, 1, 1, 0, 26 , 0  , 0  , 0  , 0  , 0  ), // #262 {X:r8lo|r8hi|m8|r16|m16|r32|m32|r64|m64}
  ISIGNATURE(3, 1, 1, 0, 118, 27 , 27 , 0  , 0  , 0  ), // #263 {X:xmm, R:i8, R:i8}
  ISIGNATURE(2, 1, 1, 0, 118, 65 , 0  , 0  , 0  , 0  ), //      {X:xmm, R:xmm}
  ISIGNATURE(0, 1, 1, 0, 0  , 0  , 0  , 0  , 0  , 0  ), // #265 {}
  ISIGNATURE(1, 1, 1, 0, 111, 0  , 0  , 0  , 0  , 0  ), // #266 {X:fp}
  ISIGNATURE(0, 1, 1, 0, 0  , 0  , 0  , 0  , 0  , 0  ), // #267 {}
  ISIGNATURE(1, 1, 1, 0, 146, 0  , 0  , 0  , 0  , 0  ), // #268 {X:m32|m64|fp}
  ISIGNATURE(2, 1, 1, 0, 118, 65 , 0  , 0  , 0  , 0  ), // #269 {X:xmm, R:xmm}
  ISIGNATURE(4, 1, 1, 0, 118, 65 , 27 , 27 , 0  , 0  ), //      {X:xmm, R:xmm, R:i8, R:i8}
  ISIGNATURE(2, 1, 0, 0, 6  , 102, 0  , 0  , 0  , 0  ), // #271 {R:r32, R:m128}
  ISIGNATURE(2, 0, 1, 0, 25 , 102, 0  , 0  , 0  , 0  ), //      {R:r64, R:m128}
  ISIGNATURE(2, 1, 0, 1, 147, 148, 0  , 0  , 0  , 0  ), // #273 {R:<cx|ecx>, R:rel8}
  ISIGNATURE(2, 0, 1, 1, 149, 148, 0  , 0  , 0  , 0  ), //      {R:<ecx|rcx>, R:rel8}
  ISIGNATURE(1, 1, 1, 0, 150, 0  , 0  , 0  , 0  , 0  ), // #275 {R:rel8|rel32|r64|m64}
  ISIGNATURE(1, 1, 0, 0, 39 , 0  , 0  , 0  , 0  , 0  ), //      {R:r32|m32}
  ISIGNATURE(2, 1, 1, 0, 116, 151, 0  , 0  , 0  , 0  ), // #277 {W:k, R:k|m8|r32|r64|r8lo|r8hi|r16}
  ISIGNATURE(2, 1, 1, 0, 152, 153, 0  , 0  , 0  , 0  ), //      {W:m8|r32|r64|r8lo|r8hi|r16, R:k}
  ISIGNATURE(2, 1, 1, 0, 116, 154, 0  , 0  , 0  , 0  ), // #279 {W:k, R:k|m32|r32|r64}
  ISIGNATURE(2, 1, 1, 0, 155, 153, 0  , 0  , 0  , 0  ), //      {W:m32|r32|r64, R:k}
  ISIGNATURE(2, 1, 1, 0, 116, 156, 0  , 0  , 0  , 0  ), // #281 {W:k, R:k|m64|r64}
  ISIGNATURE(2, 1, 1, 0, 7  , 153, 0  , 0  , 0  , 0  ), //      {W:m64|r64, R:k}
  ISIGNATURE(2, 1, 1, 0, 116, 157, 0  , 0  , 0  , 0  ), // #283 {W:k, R:k|m16|r32|r64|r16}
  ISIGNATURE(2, 1, 1, 0, 158, 153, 0  , 0  , 0  , 0  ), //      {W:m16|r32|r64|r16, R:k}
  ISIGNATURE(2, 1, 1, 0, 17 , 12 , 0  , 0  , 0  , 0  ), // #285 {W:r16, R:r16|m16}
  ISIGNATURE(2, 1, 1, 0, 13 , 159, 0  , 0  , 0  , 0  ), //      {W:r32, R:r32|m16|r16}
  ISIGNATURE(2, 1, 0, 1, 160, 148, 0  , 0  , 0  , 0  ), // #287 {X:<cx|ecx>, R:rel8}
  ISIGNATURE(2, 0, 1, 1, 161, 148, 0  , 0  , 0  , 0  ), //      {X:<ecx|rcx>, R:rel8}
  ISIGNATURE(2, 1, 1, 0, 17 , 12 , 0  , 0  , 0  , 0  ), // #289 {W:r16, R:r16|m16}
  ISIGNATURE(2, 1, 1, 0, 162, 159, 0  , 0  , 0  , 0  ), //      {W:r32|r64, R:r32|m16|r16}
  ISIGNATURE(2, 1, 1, 0, 163, 164, 0  , 0  , 0  , 0  ), // #291 {W:mm|xmm, R:r32|m32|r64}
  ISIGNATURE(2, 1, 1, 0, 155, 165, 0  , 0  , 0  , 0  ), //      {W:r32|m32|r64, R:mm|xmm}
  ISIGNATURE(2, 1, 1, 0, 66 , 67 , 0  , 0  , 0  , 0  ), // #293 {W:xmm, R:xmm|m64}
  ISIGNATURE(2, 1, 1, 0, 60 , 65 , 0  , 0  , 0  , 0  ), //      {W:m64, R:xmm}
  ISIGNATURE(2, 1, 1, 0, 66 , 128, 0  , 0  , 0  , 0  ), // #295 {W:xmm, R:xmm|m32}
  ISIGNATURE(2, 1, 1, 0, 59 , 65 , 0  , 0  , 0  , 0  ), // #296 {W:m32, R:xmm}
  ISIGNATURE(2, 1, 1, 0, 166, 36 , 0  , 0  , 0  , 0  ), // #297 {W:r16|r32|r64, R:r8lo|r8hi|m8}
  ISIGNATURE(2, 1, 1, 0, 162, 12 , 0  , 0  , 0  , 0  ), //      {W:r32|r64, R:r16|m16}
  ISIGNATURE(4, 1, 1, 1, 13 , 13 , 39 , 167, 0  , 0  ), // #299 {W:r32, W:r32, R:r32|m32, R:<edx>}
  ISIGNATURE(4, 0, 1, 1, 19 , 19 , 16 , 168, 0  , 0  ), //      {W:r64, W:r64, R:r64|m64, R:<rdx>}
  ISIGNATURE(0, 1, 1, 0, 0  , 0  , 0  , 0  , 0  , 0  ), // #301 {}
  ISIGNATURE(1, 1, 1, 0, 169, 0  , 0  , 0  , 0  , 0  ), //      {R:r16|m16|r32|m32}
  ISIGNATURE(2, 1, 1, 0, 61 , 170, 0  , 0  , 0  , 0  ), // #303 {W:mm, R:mm|m64}
  ISIGNATURE(2, 1, 1, 0, 66 , 69 , 0  , 0  , 0  , 0  ), //      {W:xmm, R:xmm|m128}
  ISIGNATURE(2, 1, 1, 0, 171, 170, 0  , 0  , 0  , 0  ), // #305 {X:mm, R:mm|m64}
  ISIGNATURE(2, 1, 1, 0, 118, 69 , 0  , 0  , 0  , 0  ), // #306 {X:xmm, R:xmm|m128}
  ISIGNATURE(3, 1, 1, 0, 171, 170, 27 , 0  , 0  , 0  ), // #307 {X:mm, R:mm|m64, R:i8}
  ISIGNATURE(3, 1, 1, 0, 118, 69 , 27 , 0  , 0  , 0  ), // #308 {X:xmm, R:xmm|m128, R:i8}
  ISIGNATURE(3, 1, 1, 0, 162, 64 , 27 , 0  , 0  , 0  ), // #309 {W:r32|r64, R:mm, R:i8}
  ISIGNATURE(3, 1, 1, 0, 158, 65 , 27 , 0  , 0  , 0  ), // #310 {W:r32|r64|m16|r16, R:xmm, R:i8}
  ISIGNATURE(2, 1, 1, 0, 171, 172, 0  , 0  , 0  , 0  ), // #311 {X:mm, R:i8|mm|m64}
  ISIGNATURE(2, 1, 1, 0, 118, 79 , 0  , 0  , 0  , 0  ), //      {X:xmm, R:i8|xmm|m128}
  ISIGNATURE(0, 1, 1, 0, 0  , 0  , 0  , 0  , 0  , 0  ), // #313 {}
  ISIGNATURE(1, 1, 1, 0, 18 , 0  , 0  , 0  , 0  , 0  ), //      {R:i16}
  ISIGNATURE(3, 1, 1, 0, 13 , 39 , 27 , 0  , 0  , 0  ), // #315 {W:r32, R:r32|m32, R:i8}
  ISIGNATURE(3, 0, 1, 0, 19 , 16 , 27 , 0  , 0  , 0  ), //      {W:r64, R:r64|m64, R:i8}
  ISIGNATURE(4, 1, 1, 0, 66 , 65 , 69 , 65 , 0  , 0  ), // #317 {W:xmm, R:xmm, R:xmm|m128, R:xmm}
  ISIGNATURE(4, 1, 1, 0, 71 , 74 , 72 , 74 , 0  , 0  ), //      {W:ymm, R:ymm, R:ymm|m256, R:ymm}
  ISIGNATURE(2, 1, 1, 0, 66 , 173, 0  , 0  , 0  , 0  ), // #319 {W:xmm, R:xmm|m128|ymm|m256}
  ISIGNATURE(2, 1, 1, 0, 71 , 76 , 0  , 0  , 0  , 0  ), //      {W:ymm, R:zmm|m512}
  ISIGNATURE(2, 1, 1, 0, 162, 128, 0  , 0  , 0  , 0  ), // #321 {W:r32|r64, R:xmm|m32}
  ISIGNATURE(2, 0, 1, 0, 19 , 67 , 0  , 0  , 0  , 0  ), //      {W:r64, R:xmm|m64}
  ISIGNATURE(2, 1, 1, 0, 13 , 128, 0  , 0  , 0  , 0  ), // #323 {W:r32, R:xmm|m32}
  ISIGNATURE(2, 0, 1, 0, 19 , 67 , 0  , 0  , 0  , 0  ), //      {W:r64, R:xmm|m64}
  ISIGNATURE(4, 1, 1, 0, 66 , 65 , 65 , 67 , 0  , 0  ), // #325 {W:xmm, R:xmm, R:xmm, R:xmm|m64}
  ISIGNATURE(4, 1, 1, 0, 66 , 65 , 67 , 65 , 0  , 0  ), //      {W:xmm, R:xmm, R:xmm|m64, R:xmm}
  ISIGNATURE(4, 1, 1, 0, 66 , 65 , 65 , 128, 0  , 0  ), // #327 {W:xmm, R:xmm, R:xmm, R:xmm|m32}
  ISIGNATURE(4, 1, 1, 0, 66 , 65 , 128, 65 , 0  , 0  ), //      {W:xmm, R:xmm, R:xmm|m32, R:xmm}
  ISIGNATURE(4, 1, 1, 0, 71 , 74 , 69 , 27 , 0  , 0  ), // #329 {W:ymm, R:ymm, R:xmm|m128, R:i8}
  ISIGNATURE(4, 1, 1, 0, 75 , 78 , 69 , 27 , 0  , 0  ), //      {W:zmm, R:zmm, R:xmm|m128, R:i8}
  ISIGNATURE(2, 1, 1, 0, 155, 65 , 0  , 0  , 0  , 0  ), // #331 {W:r32|m32|r64, R:xmm}
  ISIGNATURE(2, 1, 1, 0, 66 , 164, 0  , 0  , 0  , 0  ), //      {W:xmm, R:r32|m32|r64}
  ISIGNATURE(2, 1, 1, 0, 60 , 65 , 0  , 0  , 0  , 0  ), // #333 {W:m64, R:xmm}
  ISIGNATURE(3, 1, 1, 0, 66 , 65 , 57 , 0  , 0  , 0  ), //      {W:xmm, R:xmm, R:m64}
  ISIGNATURE(2, 1, 1, 0, 174, 175, 0  , 0  , 0  , 0  ), // #335 {W:xmm|ymm|zmm, R:xmm|m8}
  ISIGNATURE(2, 1, 1, 0, 174, 176, 0  , 0  , 0  , 0  ), //      {W:xmm|ymm|zmm, R:r32|r64}
  ISIGNATURE(2, 1, 1, 0, 174, 128, 0  , 0  , 0  , 0  ), // #337 {W:xmm|ymm|zmm, R:xmm|m32}
  ISIGNATURE(2, 1, 1, 0, 174, 176, 0  , 0  , 0  , 0  ), //      {W:xmm|ymm|zmm, R:r32|r64}
  ISIGNATURE(2, 1, 1, 0, 174, 129, 0  , 0  , 0  , 0  ), // #339 {W:xmm|ymm|zmm, R:xmm|m16}
  ISIGNATURE(2, 1, 1, 0, 174, 176, 0  , 0  , 0  , 0  ), //      {W:xmm|ymm|zmm, R:r32|r64}
  ISIGNATURE(3, 1, 1, 0, 66 , 177, 27 , 0  , 0  , 0  ), // #341 {W:xmm, R:r32|m8|r64|r8lo|r8hi|r16, R:i8}
  ISIGNATURE(4, 1, 1, 0, 66 , 65 , 177, 27 , 0  , 0  ), //      {W:xmm, R:xmm, R:r32|m8|r64|r8lo|r8hi|r16, R:i8}
  ISIGNATURE(3, 1, 1, 0, 66 , 164, 27 , 0  , 0  , 0  ), // #343 {W:xmm, R:r32|m32|r64, R:i8}
  ISIGNATURE(4, 1, 1, 0, 66 , 65 , 164, 27 , 0  , 0  ), //      {W:xmm, R:xmm, R:r32|m32|r64, R:i8}
  ISIGNATURE(3, 0, 1, 0, 66 , 16 , 27 , 0  , 0  , 0  ), // #345 {W:xmm, R:r64|m64, R:i8}
  ISIGNATURE(4, 0, 1, 0, 66 , 65 , 16 , 27 , 0  , 0  ), //      {W:xmm, R:xmm, R:r64|m64, R:i8}
  ISIGNATURE(3, 1, 1, 0, 66 , 65 , 69 , 0  , 0  , 0  ), // #347 {W:xmm, R:xmm, R:xmm|m128}
  ISIGNATURE(3, 1, 1, 0, 66 , 69 , 178, 0  , 0  , 0  ), //      {W:xmm, R:xmm|m128, R:i8|xmm}
  ISIGNATURE(2, 1, 1, 0, 179, 65 , 0  , 0  , 0  , 0  ), // #349 {W:vm64x|vm64y, R:xmm}
  ISIGNATURE(2, 1, 1, 0, 137, 74 , 0  , 0  , 0  , 0  ), //      {W:vm64z, R:ymm}
  ISIGNATURE(3, 1, 1, 0, 66 , 65 , 69 , 0  , 0  , 0  ), // #351 {W:xmm, R:xmm, R:xmm|m128}
  ISIGNATURE(3, 1, 1, 0, 66 , 69 , 65 , 0  , 0  , 0  ), //      {W:xmm, R:xmm|m128, R:xmm}
  ISIGNATURE(2, 1, 1, 0, 65 , 69 , 0  , 0  , 0  , 0  ), // #353 {R:xmm, R:xmm|m128}
  ISIGNATURE(2, 1, 1, 0, 74 , 72 , 0  , 0  , 0  , 0  ), //      {R:ymm, R:ymm|m256}
  ISIGNATURE(2, 1, 1, 0, 132, 180, 0  , 0  , 0  , 0  ), // #355 {W:vm32x, R:xmm|ymm}
  ISIGNATURE(2, 1, 1, 0, 133, 78 , 0  , 0  , 0  , 0  ), //      {W:vm32y, R:zmm}
  ISIGNATURE(1, 1, 0, 1, 44 , 0  , 0  , 0  , 0  , 0  ), // #357 {X:<ax>}
  ISIGNATURE(2, 1, 0, 1, 44 , 27 , 0  , 0  , 0  , 0  ), // #358 {X:<ax>, R:i8}
  ISIGNATURE(2, 1, 1, 0, 118, 67 , 0  , 0  , 0  , 0  ), // #359 {X:xmm, R:xmm|m64}
  ISIGNATURE(2, 1, 1, 0, 118, 128, 0  , 0  , 0  , 0  ), // #360 {X:xmm, R:xmm|m32}
  ISIGNATURE(2, 1, 0, 0, 28 , 43 , 0  , 0  , 0  , 0  ), // #361 {X:r16|m16, R:r16}
  ISIGNATURE(3, 1, 1, 1, 118, 69 , 181, 0  , 0  , 0  ), // #362 {X:xmm, R:xmm|m128, R:<xmm0>}
  ISIGNATURE(2, 1, 1, 0, 139, 182, 0  , 0  , 0  , 0  ), // #363 {W:bnd, R:mib}
  ISIGNATURE(2, 1, 1, 0, 139, 183, 0  , 0  , 0  , 0  ), // #364 {W:bnd, R:mem}
  ISIGNATURE(2, 1, 1, 0, 184, 138, 0  , 0  , 0  , 0  ), // #365 {W:mib, R:bnd}
  ISIGNATURE(1, 1, 1, 0, 185, 0  , 0  , 0  , 0  , 0  ), // #366 {X:r32|r64}
  ISIGNATURE(1, 1, 1, 1, 44 , 0  , 0  , 0  , 0  , 0  ), // #367 {X:<ax>}
  ISIGNATURE(2, 1, 1, 2, 46 , 88 , 0  , 0  , 0  , 0  ), // #368 {W:<edx>, R:<eax>}
  ISIGNATURE(1, 0, 1, 1, 49 , 0  , 0  , 0  , 0  , 0  ), // #369 {X:<rax>}
  ISIGNATURE(1, 1, 1, 0, 183, 0  , 0  , 0  , 0  , 0  ), // #370 {R:mem}
  ISIGNATURE(1, 1, 1, 1, 186, 0  , 0  , 0  , 0  , 0  ), // #371 {R:<ds:[zax]>}
  ISIGNATURE(2, 1, 1, 2, 187, 188, 0  , 0  , 0  , 0  ), // #372 {X:<ds:[zsi]>, X:<es:[zdi]>}
  ISIGNATURE(3, 1, 1, 0, 118, 67 , 27 , 0  , 0  , 0  ), // #373 {X:xmm, R:xmm|m64, R:i8}
  ISIGNATURE(3, 1, 1, 0, 118, 128, 27 , 0  , 0  , 0  ), // #374 {X:xmm, R:xmm|m32, R:i8}
  ISIGNATURE(5, 0, 1, 4, 189, 92 , 49 , 190, 191, 0  ), // #375 {X:m128, X:<rdx>, X:<rax>, R:<rcx>, R:<rbx>}
  ISIGNATURE(5, 1, 1, 4, 192, 91 , 47 , 193, 194, 0  ), // #376 {X:m64, X:<edx>, X:<eax>, R:<ecx>, R:<ebx>}
  ISIGNATURE(2, 1, 1, 0, 65 , 67 , 0  , 0  , 0  , 0  ), // #377 {R:xmm, R:xmm|m64}
  ISIGNATURE(2, 1, 1, 0, 65 , 128, 0  , 0  , 0  , 0  ), // #378 {R:xmm, R:xmm|m32}
  ISIGNATURE(4, 1, 1, 4, 47 , 195, 196, 46 , 0  , 0  ), // #379 {X:<eax>, W:<ebx>, X:<ecx>, W:<edx>}
  ISIGNATURE(2, 0, 1, 2, 48 , 89 , 0  , 0  , 0  , 0  ), // #380 {W:<rdx>, R:<rax>}
  ISIGNATURE(2, 1, 1, 0, 61 , 69 , 0  , 0  , 0  , 0  ), // #381 {W:mm, R:xmm|m128}
  ISIGNATURE(2, 1, 1, 0, 66 , 170, 0  , 0  , 0  , 0  ), // #382 {W:xmm, R:mm|m64}
  ISIGNATURE(2, 1, 1, 0, 61 , 67 , 0  , 0  , 0  , 0  ), // #383 {W:mm, R:xmm|m64}
  ISIGNATURE(2, 1, 1, 0, 162, 67 , 0  , 0  , 0  , 0  ), // #384 {W:r32|r64, R:xmm|m64}
  ISIGNATURE(2, 1, 1, 0, 66 , 42 , 0  , 0  , 0  , 0  ), // #385 {W:xmm, R:r32|m32|r64|m64}
  ISIGNATURE(2, 1, 1, 2, 45 , 87 , 0  , 0  , 0  , 0  ), // #386 {W:<dx>, R:<ax>}
  ISIGNATURE(1, 1, 1, 1, 47 , 0  , 0  , 0  , 0  , 0  ), // #387 {X:<eax>}
  ISIGNATURE(2, 1, 1, 0, 18 , 27 , 0  , 0  , 0  , 0  ), // #388 {R:i16, R:i8}
  ISIGNATURE(3, 1, 1, 0, 155, 65 , 27 , 0  , 0  , 0  ), // #389 {W:r32|m32|r64, R:xmm, R:i8}
  ISIGNATURE(1, 1, 1, 0, 197, 0  , 0  , 0  , 0  , 0  ), // #390 {X:m80}
  ISIGNATURE(1, 1, 1, 0, 198, 0  , 0  , 0  , 0  , 0  ), // #391 {X:m16|m32}
  ISIGNATURE(1, 1, 1, 0, 199, 0  , 0  , 0  , 0  , 0  ), // #392 {X:m16|m32|m64}
  ISIGNATURE(1, 1, 1, 0, 200, 0  , 0  , 0  , 0  , 0  ), // #393 {X:m32|m64|m80|fp}
  ISIGNATURE(1, 1, 1, 0, 201, 0  , 0  , 0  , 0  , 0  ), // #394 {X:m16}
  ISIGNATURE(1, 1, 1, 0, 202, 0  , 0  , 0  , 0  , 0  ), // #395 {X:mem}
  ISIGNATURE(1, 1, 1, 0, 203, 0  , 0  , 0  , 0  , 0  ), // #396 {X:ax|m16}
  ISIGNATURE(1, 0, 1, 0, 202, 0  , 0  , 0  , 0  , 0  ), // #397 {X:mem}
  ISIGNATURE(2, 1, 1, 0, 204, 205, 0  , 0  , 0  , 0  ), // #398 {W:al|ax|eax, R:i8|dx}
  ISIGNATURE(2, 1, 1, 0, 206, 207, 0  , 0  , 0  , 0  ), // #399 {W:es:[zdi], R:dx}
  ISIGNATURE(1, 1, 1, 0, 27 , 0  , 0  , 0  , 0  , 0  ), // #400 {R:i8}
  ISIGNATURE(1, 1, 1, 0, 208, 0  , 0  , 0  , 0  , 0  ), // #401 {R:rel8|rel32}
  ISIGNATURE(1, 1, 1, 0, 148, 0  , 0  , 0  , 0  , 0  ), // #402 {R:rel8}
  ISIGNATURE(3, 1, 1, 0, 116, 153, 153, 0  , 0  , 0  ), // #403 {W:k, R:k, R:k}
  ISIGNATURE(2, 1, 1, 0, 116, 153, 0  , 0  , 0  , 0  ), // #404 {W:k, R:k}
  ISIGNATURE(2, 1, 1, 0, 153, 153, 0  , 0  , 0  , 0  ), // #405 {R:k, R:k}
  ISIGNATURE(3, 1, 1, 0, 116, 153, 27 , 0  , 0  , 0  ), // #406 {W:k, R:k, R:i8}
  ISIGNATURE(1, 1, 1, 1, 209, 0  , 0  , 0  , 0  , 0  ), // #407 {W:<ah>}
  ISIGNATURE(1, 1, 1, 0, 56 , 0  , 0  , 0  , 0  , 0  ), // #408 {R:m32}
  ISIGNATURE(2, 1, 1, 0, 166, 183, 0  , 0  , 0  , 0  ), // #409 {W:r16|r32|r64, R:mem}
  ISIGNATURE(1, 1, 1, 0, 210, 0  , 0  , 0  , 0  , 0  ), // #410 {R:r16|m16|r32|r64}
  ISIGNATURE(2, 1, 1, 2, 211, 187, 0  , 0  , 0  , 0  ), // #411 {W:<al|ax|eax|rax>, X:<ds:[zsi]>}
  ISIGNATURE(3, 1, 1, 1, 118, 65 , 212, 0  , 0  , 0  ), // #412 {X:xmm, R:xmm, R:<ds:[zdi]>}
  ISIGNATURE(3, 1, 1, 1, 171, 64 , 212, 0  , 0  , 0  ), // #413 {X:mm, R:mm, R:<ds:[zdi]>}
  ISIGNATURE(3, 1, 1, 3, 186, 193, 167, 0  , 0  , 0  ), // #414 {R:<ds:[zax]>, R:<ecx>, R:<edx>}
  ISIGNATURE(2, 1, 1, 0, 61 , 65 , 0  , 0  , 0  , 0  ), // #415 {W:mm, R:xmm}
  ISIGNATURE(2, 1, 1, 0, 66 , 65 , 0  , 0  , 0  , 0  ), // #416 {W:xmm, R:xmm}
  ISIGNATURE(2, 1, 1, 0, 162, 65 , 0  , 0  , 0  , 0  ), // #417 {W:r32|r64, R:xmm}
  ISIGNATURE(2, 1, 1, 0, 60 , 64 , 0  , 0  , 0  , 0  ), // #418 {W:m64, R:mm}
  ISIGNATURE(2, 1, 1, 0, 66 , 64 , 0  , 0  , 0  , 0  ), // #419 {W:xmm, R:mm}
  ISIGNATURE(2, 1, 1, 2, 188, 187, 0  , 0  , 0  , 0  ), // #420 {X:<es:[zdi]>, X:<ds:[zsi]>}
  ISIGNATURE(2, 0, 1, 0, 19 , 39 , 0  , 0  , 0  , 0  ), // #421 {W:r64, R:r32|m32}
  ISIGNATURE(2, 1, 1, 2, 88 , 193, 0  , 0  , 0  , 0  ), // #422 {R:<eax>, R:<ecx>}
  ISIGNATURE(2, 1, 1, 0, 205, 213, 0  , 0  , 0  , 0  ), // #423 {R:i8|dx, R:al|ax|eax}
  ISIGNATURE(2, 1, 1, 0, 207, 214, 0  , 0  , 0  , 0  ), // #424 {R:dx, R:ds:[zsi]}
  ISIGNATURE(6, 1, 1, 3, 65 , 69 , 27 , 215, 88 , 167), // #425 {R:xmm, R:xmm|m128, R:i8, W:<ecx>, R:<eax>, R:<edx>}
  ISIGNATURE(6, 1, 1, 3, 65 , 69 , 27 , 216, 88 , 167), // #426 {R:xmm, R:xmm|m128, R:i8, W:<xmm0>, R:<eax>, R:<edx>}
  ISIGNATURE(4, 1, 1, 1, 65 , 69 , 27 , 215, 0  , 0  ), // #427 {R:xmm, R:xmm|m128, R:i8, W:<ecx>}
  ISIGNATURE(4, 1, 1, 1, 65 , 69 , 27 , 216, 0  , 0  ), // #428 {R:xmm, R:xmm|m128, R:i8, W:<xmm0>}
  ISIGNATURE(3, 1, 1, 0, 152, 65 , 27 , 0  , 0  , 0  ), // #429 {W:r32|m8|r64|r8lo|r8hi|r16, R:xmm, R:i8}
  ISIGNATURE(3, 0, 1, 0, 7  , 65 , 27 , 0  , 0  , 0  ), // #430 {W:r64|m64, R:xmm, R:i8}
  ISIGNATURE(3, 1, 1, 0, 118, 177, 27 , 0  , 0  , 0  ), // #431 {X:xmm, R:r32|m8|r64|r8lo|r8hi|r16, R:i8}
  ISIGNATURE(3, 1, 1, 0, 118, 164, 27 , 0  , 0  , 0  ), // #432 {X:xmm, R:r32|m32|r64, R:i8}
  ISIGNATURE(3, 0, 1, 0, 118, 16 , 27 , 0  , 0  , 0  ), // #433 {X:xmm, R:r64|m64, R:i8}
  ISIGNATURE(3, 1, 1, 0, 217, 210, 27 , 0  , 0  , 0  ), // #434 {X:mm|xmm, R:r32|m16|r64|r16, R:i8}
  ISIGNATURE(2, 1, 1, 0, 162, 165, 0  , 0  , 0  , 0  ), // #435 {W:r32|r64, R:mm|xmm}
  ISIGNATURE(0, 1, 0, 0, 0  , 0  , 0  , 0  , 0  , 0  ), // #436 {}
  ISIGNATURE(0, 0, 1, 0, 0  , 0  , 0  , 0  , 0  , 0  ), // #437 {}
  ISIGNATURE(3, 1, 1, 0, 61 , 170, 27 , 0  , 0  , 0  ), // #438 {W:mm, R:mm|m64, R:i8}
  ISIGNATURE(2, 1, 1, 0, 118, 27 , 0  , 0  , 0  , 0  ), // #439 {X:xmm, R:i8}
  ISIGNATURE(2, 1, 1, 0, 26 , 113, 0  , 0  , 0  , 0  ), // #440 {X:r8lo|r8hi|m8|r16|m16|r32|m32|r64|m64, R:cl|i8}
  ISIGNATURE(1, 0, 1, 0, 162, 0  , 0  , 0  , 0  , 0  ), // #441 {W:r32|r64}
  ISIGNATURE(3, 1, 1, 3, 46 , 218, 193, 0  , 0  , 0  ), // #442 {W:<edx>, W:<eax>, R:<ecx>}
  ISIGNATURE(1, 1, 1, 0, 166, 0  , 0  , 0  , 0  , 0  ), // #443 {W:r16|r32|r64}
  ISIGNATURE(2, 1, 1, 2, 46 , 218, 0  , 0  , 0  , 0  ), // #444 {W:<edx>, W:<eax>}
  ISIGNATURE(3, 1, 1, 3, 46 , 218, 215, 0  , 0  , 0  ), // #445 {W:<edx>, W:<eax>, W:<ecx>}
  ISIGNATURE(3, 1, 1, 0, 66 , 67 , 27 , 0  , 0  , 0  ), // #446 {W:xmm, R:xmm|m64, R:i8}
  ISIGNATURE(3, 1, 1, 0, 66 , 128, 27 , 0  , 0  , 0  ), // #447 {W:xmm, R:xmm|m32, R:i8}
  ISIGNATURE(1, 1, 1, 1, 219, 0  , 0  , 0  , 0  , 0  ), // #448 {R:<ah>}
  ISIGNATURE(2, 1, 1, 2, 220, 188, 0  , 0  , 0  , 0  ), // #449 {R:<al|ax|eax|rax>, X:<es:[zdi]>}
  ISIGNATURE(1, 1, 1, 0, 1  , 0  , 0  , 0  , 0  , 0  ), // #450 {W:r8lo|r8hi|m8}
  ISIGNATURE(1, 1, 1, 0, 221, 0  , 0  , 0  , 0  , 0  ), // #451 {W:mem}
  ISIGNATURE(1, 1, 1, 0, 158, 0  , 0  , 0  , 0  , 0  ), // #452 {W:r16|m16|r32|r64}
  ISIGNATURE(1, 1, 1, 0, 59 , 0  , 0  , 0  , 0  , 0  ), // #453 {W:m32}
  ISIGNATURE(2, 1, 1, 2, 188, 220, 0  , 0  , 0  , 0  ), // #454 {X:<es:[zdi]>, R:<al|ax|eax|rax>}
  ISIGNATURE(6, 1, 1, 0, 120, 78 , 78 , 78 , 78 , 102), // #455 {X:zmm, R:zmm, R:zmm, R:zmm, R:zmm, R:m128}
  ISIGNATURE(3, 1, 1, 0, 66 , 65 , 67 , 0  , 0  , 0  ), // #456 {W:xmm, R:xmm, R:xmm|m64}
  ISIGNATURE(3, 1, 1, 0, 66 , 65 , 128, 0  , 0  , 0  ), // #457 {W:xmm, R:xmm, R:xmm|m32}
  ISIGNATURE(2, 1, 1, 0, 71 , 102, 0  , 0  , 0  , 0  ), // #458 {W:ymm, R:m128}
  ISIGNATURE(2, 1, 1, 0, 222, 67 , 0  , 0  , 0  , 0  ), // #459 {W:ymm|zmm, R:xmm|m64}
  ISIGNATURE(2, 1, 1, 0, 222, 102, 0  , 0  , 0  , 0  ), // #460 {W:ymm|zmm, R:m128}
  ISIGNATURE(2, 1, 1, 0, 75 , 103, 0  , 0  , 0  , 0  ), // #461 {W:zmm, R:m256}
  ISIGNATURE(2, 1, 1, 0, 174, 67 , 0  , 0  , 0  , 0  ), // #462 {W:xmm|ymm|zmm, R:xmm|m64}
  ISIGNATURE(4, 1, 1, 0, 114, 65 , 67 , 27 , 0  , 0  ), // #463 {W:xmm|k, R:xmm, R:xmm|m64, R:i8}
  ISIGNATURE(4, 1, 1, 0, 114, 65 , 128, 27 , 0  , 0  ), // #464 {W:xmm|k, R:xmm, R:xmm|m32, R:i8}
  ISIGNATURE(3, 1, 1, 0, 66 , 65 , 42 , 0  , 0  , 0  ), // #465 {W:xmm, R:xmm, R:r32|m32|r64|m64}
  ISIGNATURE(1, 1, 1, 0, 223, 0  , 0  , 0  , 0  , 0  ), // #466 {X:r16|m16|r32|r64}
  ISIGNATURE(3, 1, 1, 0, 70 , 224, 27 , 0  , 0  , 0  ), // #467 {W:xmm|m128, R:ymm|zmm, R:i8}
  ISIGNATURE(4, 1, 1, 0, 118, 65 , 67 , 27 , 0  , 0  ), // #468 {X:xmm, R:xmm, R:xmm|m64, R:i8}
  ISIGNATURE(4, 1, 1, 0, 118, 65 , 128, 27 , 0  , 0  ), // #469 {X:xmm, R:xmm, R:xmm|m32, R:i8}
  ISIGNATURE(3, 1, 1, 0, 118, 65 , 67 , 0  , 0  , 0  ), // #470 {X:xmm, R:xmm, R:xmm|m64}
  ISIGNATURE(3, 1, 1, 0, 118, 65 , 128, 0  , 0  , 0  ), // #471 {X:xmm, R:xmm, R:xmm|m32}
  ISIGNATURE(3, 1, 1, 0, 116, 225, 27 , 0  , 0  , 0  ), // #472 {W:k, R:xmm|m128|ymm|m256|zmm|m512, R:i8}
  ISIGNATURE(3, 1, 1, 0, 116, 67 , 27 , 0  , 0  , 0  ), // #473 {W:k, R:xmm|m64, R:i8}
  ISIGNATURE(3, 1, 1, 0, 116, 128, 27 , 0  , 0  , 0  ), // #474 {W:k, R:xmm|m32, R:i8}
  ISIGNATURE(1, 1, 1, 0, 81 , 0  , 0  , 0  , 0  , 0  ), // #475 {R:vm32y}
  ISIGNATURE(1, 1, 1, 0, 82 , 0  , 0  , 0  , 0  , 0  ), // #476 {R:vm32z}
  ISIGNATURE(1, 1, 1, 0, 85 , 0  , 0  , 0  , 0  , 0  ), // #477 {R:vm64z}
  ISIGNATURE(4, 1, 1, 0, 75 , 78 , 72 , 27 , 0  , 0  ), // #478 {W:zmm, R:zmm, R:ymm|m256, R:i8}
  ISIGNATURE(4, 1, 1, 0, 66 , 65 , 128, 27 , 0  , 0  ), // #479 {W:xmm, R:xmm, R:xmm|m32, R:i8}
  ISIGNATURE(3, 1, 1, 1, 65 , 65 , 212, 0  , 0  , 0  ), // #480 {R:xmm, R:xmm, R:<ds:[zdi]>}
  ISIGNATURE(2, 1, 1, 0, 162, 180, 0  , 0  , 0  , 0  ), // #481 {W:r32|r64, R:xmm|ymm}
  ISIGNATURE(6, 1, 1, 0, 75 , 78 , 78 , 78 , 78 , 102), // #482 {W:zmm, R:zmm, R:zmm, R:zmm, R:zmm, R:m128}
  ISIGNATURE(2, 1, 1, 0, 174, 153, 0  , 0  , 0  , 0  ), // #483 {W:xmm|ymm|zmm, R:k}
  ISIGNATURE(2, 1, 1, 0, 174, 123, 0  , 0  , 0  , 0  ), // #484 {W:xmm|ymm|zmm, R:xmm|m64|r64}
  ISIGNATURE(4, 1, 1, 0, 66 , 65 , 210, 27 , 0  , 0  ), // #485 {W:xmm, R:xmm, R:r32|m16|r64|r16, R:i8}
  ISIGNATURE(2, 1, 1, 0, 116, 226, 0  , 0  , 0  , 0  ), // #486 {W:k, R:xmm|ymm|zmm}
  ISIGNATURE(4, 1, 1, 0, 66 , 65 , 67 , 27 , 0  , 0  ), // #487 {W:xmm, R:xmm, R:xmm|m64, R:i8}
  ISIGNATURE(1, 0, 1, 0, 176, 0  , 0  , 0  , 0  , 0  ), // #488 {R:r32|r64}
  ISIGNATURE(3, 1, 1, 3, 167, 88 , 193, 0  , 0  , 0  ), // #489 {R:<edx>, R:<eax>, R:<ecx>}
  ISIGNATURE(3, 1, 1, 3, 193, 46 , 218, 0  , 0  , 0  ), // #490 {R:<ecx>, W:<edx>, W:<eax>}
  ISIGNATURE(3, 1, 1, 2, 202, 167, 88 , 0  , 0  , 0  ), // #491 {X:mem, R:<edx>, R:<eax>}
  ISIGNATURE(3, 0, 1, 2, 202, 167, 88 , 0  , 0  , 0  ), // #492 {X:mem, R:<edx>, R:<eax>}
  ISIGNATURE(3, 1, 1, 3, 193, 167, 88 , 0  , 0  , 0  )  // #493 {R:<ecx>, R:<edx>, R:<eax>}
};
#undef ISIGNATURE
// ----------------------------------------------------------------------------
// ${signatureData:End}

template<uint32_t RegType>
struct X86OpTypeFromRegTypeT {
  enum {
    kValue = (RegType == X86Reg::kRegGpbLo) ? X86Inst::kOpGpbLo :
             (RegType == X86Reg::kRegGpbHi) ? X86Inst::kOpGpbHi :
             (RegType == X86Reg::kRegGpw  ) ? X86Inst::kOpGpw   :
             (RegType == X86Reg::kRegGpd  ) ? X86Inst::kOpGpd   :
             (RegType == X86Reg::kRegGpq  ) ? X86Inst::kOpGpq   :
             (RegType == X86Reg::kRegXmm  ) ? X86Inst::kOpXmm   :
             (RegType == X86Reg::kRegYmm  ) ? X86Inst::kOpYmm   :
             (RegType == X86Reg::kRegZmm  ) ? X86Inst::kOpZmm   :
             (RegType == X86Reg::kRegRip  ) ? X86Inst::kOpNone  :
             (RegType == X86Reg::kRegSeg  ) ? X86Inst::kOpSeg   :
             (RegType == X86Reg::kRegFp   ) ? X86Inst::kOpFp    :
             (RegType == X86Reg::kRegMm   ) ? X86Inst::kOpMm    :
             (RegType == X86Reg::kRegK    ) ? X86Inst::kOpK     :
             (RegType == X86Reg::kRegBnd  ) ? X86Inst::kOpBnd   :
             (RegType == X86Reg::kRegCr   ) ? X86Inst::kOpCr    :
             (RegType == X86Reg::kRegDr   ) ? X86Inst::kOpDr    : X86Inst::kOpNone
  };
};

template<uint32_t RegType>
struct X86RegMaskFromRegTypeT {
  enum {
    kMask = (RegType == X86Reg::kRegGpbLo) ? 0x0000000FU :
            (RegType == X86Reg::kRegGpbHi) ? 0x0000000FU :
            (RegType == X86Reg::kRegGpw  ) ? 0x000000FFU :
            (RegType == X86Reg::kRegGpd  ) ? 0x000000FFU :
            (RegType == X86Reg::kRegGpq  ) ? 0x000000FFU :
            (RegType == X86Reg::kRegXmm  ) ? 0x000000FFU :
            (RegType == X86Reg::kRegYmm  ) ? 0x000000FFU :
            (RegType == X86Reg::kRegZmm  ) ? 0x000000FFU :
            (RegType == X86Reg::kRegRip  ) ? 0x00000001U :
            (RegType == X86Reg::kRegSeg  ) ? 0x0000007EU : // [ES|CS|SS|DS|FS|GS]
            (RegType == X86Reg::kRegFp   ) ? 0x000000FFU :
            (RegType == X86Reg::kRegMm   ) ? 0x000000FFU :
            (RegType == X86Reg::kRegK    ) ? 0x000000FFU :
            (RegType == X86Reg::kRegBnd  ) ? 0x0000000FU :
            (RegType == X86Reg::kRegCr   ) ? 0x000000FFU :
            (RegType == X86Reg::kRegDr   ) ? 0x000000FFU : X86Inst::kOpNone
  };
};

template<uint32_t RegType>
struct X64RegMaskFromRegTypeT {
  enum {
    kMask = (RegType == X86Reg::kRegGpbLo) ? 0x0000FFFFU :
            (RegType == X86Reg::kRegGpbHi) ? 0x0000000FU :
            (RegType == X86Reg::kRegGpw  ) ? 0x0000FFFFU :
            (RegType == X86Reg::kRegGpd  ) ? 0x0000FFFFU :
            (RegType == X86Reg::kRegGpq  ) ? 0x0000FFFFU :
            (RegType == X86Reg::kRegXmm  ) ? 0xFFFFFFFFU :
            (RegType == X86Reg::kRegYmm  ) ? 0xFFFFFFFFU :
            (RegType == X86Reg::kRegZmm  ) ? 0xFFFFFFFFU :
            (RegType == X86Reg::kRegRip  ) ? 0x00000001U :
            (RegType == X86Reg::kRegSeg  ) ? 0x0000007EU : // [ES|CS|SS|DS|FS|GS]
            (RegType == X86Reg::kRegFp   ) ? 0x000000FFU :
            (RegType == X86Reg::kRegMm   ) ? 0x000000FFU :
            (RegType == X86Reg::kRegK    ) ? 0x000000FFU :
            (RegType == X86Reg::kRegBnd  ) ? 0x0000000FU :
            (RegType == X86Reg::kRegCr   ) ? 0x0000FFFFU :
            (RegType == X86Reg::kRegDr   ) ? 0x0000FFFFU : X86Inst::kOpNone
  };
};

struct X86ValidationData {
  //! Allowed registers by reg-type (X86::kReg...).
  uint32_t allowedRegMask[X86Reg::kRegMax + 1];
  uint32_t allowedMemBaseRegs;
  uint32_t allowedMemIndexRegs;
};

static const uint32_t _x86OpFlagFromRegType[X86Reg::kRegMax + 1] = {
  ASMJIT_TABLE_T_32(X86OpTypeFromRegTypeT, kValue, 0)
};

static const X86ValidationData _x86ValidationData = {
  { ASMJIT_TABLE_T_32(X86RegMaskFromRegTypeT, kMask, 0) },
  (1U << X86Reg::kRegGpw) | (1U << X86Reg::kRegGpd) | (1U << X86Reg::kRegRip) | (1U << Label::kLabelTag),
  (1U << X86Reg::kRegGpw) | (1U << X86Reg::kRegGpd) | (1U << X86Reg::kRegXmm) | (1U << X86Reg::kRegYmm) | (1U << X86Reg::kRegZmm)
};

static const X86ValidationData _x64ValidationData = {
  { ASMJIT_TABLE_T_32(X64RegMaskFromRegTypeT, kMask, 0) },
  (1U << X86Reg::kRegGpd) | (1U << X86Reg::kRegGpq) | (1U << X86Reg::kRegRip) | (1U << Label::kLabelTag),
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
        else {
          regMask = 0xFFFFFFFFU;
        }
        break;
      }

      // TODO: Validate base and index and combine with `combinedRegMask`.
      case Operand::kOpMem: {
        const X86Mem& m = static_cast<const X86Mem&>(op);

        uint32_t baseType = m.getBaseType();
        uint32_t indexType = m.getIndexType();

        memOp = &m;

        if (m.getSegmentId() > 6)
          return DebugUtils::errored(kErrorInvalidSegment);

        if (baseType) {
          uint32_t baseId = m.getBaseId();

          if (m.isRegHome()) {
            // Home address of virtual register. In such case we don't want to
            // validate the type of the base register as it will always be patched
            // to ESP|RSP.
          }
          else {
            if (ASMJIT_UNLIKELY((vd->allowedMemBaseRegs & (1U << baseType)) == 0))
              return DebugUtils::errored(kErrorInvalidAddress);
          }

          // Create information that will be validated only if this is an implicit
          // memory operand. Basically only usable for string instructions and other
          // instructions where memory operand is implicit and has 'seg:[reg]' form.
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
        else {
          // Base is an address, make sure that the address doesn't overflow 32-bit
          // integer (either int32_t or uint32_t) in 32-bit targets.
          int64_t offset = m.getOffset();
          if (archMask == X86Inst::kArchMaskX86 && !Utils::isInt32(offset) && !Utils::isUInt32(offset))
            return DebugUtils::errored(kErrorInvalidAddress);
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
  const X86Inst::ISignature* iSig = X86InstDB::iSignatureData + commonData->_iSignatureIndex;
  const X86Inst::ISignature* iEnd = iSig                      + commonData->_iSignatureCount;

  if (iSig != iEnd) {
    const X86Inst::OSignature* oSigData = X86InstDB::oSignatureData;
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
