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
//      the latest asmdb from <https://github.com/asmjit/asmdb> and copy both
//      `x86data.js` and `x86util.js` files into the asmjit/tools directory
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
#define Enc(ENCODING) X86Inst::kEncoding##ENCODING  // Instruction Encoding `Enc(...)`.

// Don't store `_nameDataIndex` if instruction names are disabled. Since
// some  APIs can use `_nameDataIndex` it's much safer if it's zero.
#if defined(ASMJIT_DISABLE_TEXT)
# define NAME_DATA_INDEX(X) 0
#else
# define NAME_DATA_INDEX(X) X
#endif

// Defines an X86/X64 instruction.
#define INST(id, name, encoding, opcode0, opcode1, instFlags, eflags, writeIndex, writeSize, familyType, familyIndex, nameDataIndex, commonIndex) { \
  uint32_t(encoding),                       \
  uint32_t(NAME_DATA_INDEX(nameDataIndex)), \
  uint32_t(commonIndex),                    \
  uint32_t(X86Inst::familyType),            \
  uint32_t(familyIndex),                    \
  0,                                        \
  opcode0                                   \
}

const X86Inst X86InstDB::instData[] = {
  // <-----------------+-------------------+------------------------+------------------+--------+------------------+--------+---------------------------------------+-------------+-------+-----------------+-----+----+
  //                   |                   |                        |    Main OpCode   |#0 EVEX |Alternative OpCode|#1 EVEX |                                       |   E-FLAGS   | Write |                 |     |    |
  //  Instruction Id   | Instruction Name  |  Instruction Encoding  |                  +--------+                  +--------+          Instruction Flags            +-------------+---+---+ Family Type/Idx.+NameX|ComX|
  //                   |                   |                        |#0:PP-MMM OP/O L|W|W|N|TT. |#1:PP-MMM OP/O L|W|W|N|TT. |                                       | EF:OSZAPCDX |Idx|Cnt|                 |     |    |
  // <-----------------+-------------------+------------------------+------------------+--------+------------------+--------+---------------------------------------+-------------+---+---+-----------------+-----+----+
  // ${instData:Begin}
  INST(None            , ""                , Enc(None)              , 0                         , 0                         , 0                                     , EF(________), 0 , 0 , kFamilyNone, 0  , 0   , 0  ),
  INST(Aaa             , "aaa"             , Enc(X86Op_xAX)         , O(000000,37,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(UUUWUW__), 0 , 0 , kFamilyNone, 0  , 1   , 1  ),
  INST(Aad             , "aad"             , Enc(X86I_xAX)          , O(000000,D5,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(UWWUWU__), 0 , 0 , kFamilyNone, 0  , 5   , 2  ),
  INST(Aam             , "aam"             , Enc(X86I_xAX)          , O(000000,D4,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(UWWUWU__), 0 , 0 , kFamilyNone, 0  , 9   , 2  ),
  INST(Aas             , "aas"             , Enc(X86Op_xAX)         , O(000000,3F,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(UUUWUW__), 0 , 0 , kFamilyNone, 0  , 13  , 1  ),
  INST(Adc             , "adc"             , Enc(X86Arith)          , O(000000,10,2,_,x,_,_,_  ), 0                         , F(RW)|F(Lock)                         , EF(WWWWWX__), 0 , 0 , kFamilyNone, 0  , 17  , 3  ),
  INST(Adcx            , "adcx"            , Enc(X86Rm)             , O(660F38,F6,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(_____X__), 0 , 0 , kFamilyNone, 0  , 21  , 4  ),
  INST(Add             , "add"             , Enc(X86Arith)          , O(000000,00,0,_,x,_,_,_  ), 0                         , F(RW)|F(Lock)                         , EF(WWWWWW__), 0 , 0 , kFamilyNone, 0  , 678 , 5  ),
  INST(Addpd           , "addpd"           , Enc(ExtRm)             , O(660F00,58,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 0  , 4357, 6  ),
  INST(Addps           , "addps"           , Enc(ExtRm)             , O(000F00,58,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 1  , 4369, 6  ),
  INST(Addsd           , "addsd"           , Enc(ExtRm)             , O(F20F00,58,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 0  , 4591, 7  ),
  INST(Addss           , "addss"           , Enc(ExtRm)             , O(F30F00,58,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 1  , 4601, 8  ),
  INST(Addsubpd        , "addsubpd"        , Enc(ExtRm)             , O(660F00,D0,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 2  , 4096, 6  ),
  INST(Addsubps        , "addsubps"        , Enc(ExtRm)             , O(F20F00,D0,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 2  , 4108, 6  ),
  INST(Adox            , "adox"            , Enc(X86Rm)             , O(F30F38,F6,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(X_______), 0 , 0 , kFamilyNone, 0  , 26  , 9  ),
  INST(Aesdec          , "aesdec"          , Enc(ExtRm)             , O(660F38,DE,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 3  , 2602, 6  ),
  INST(Aesdeclast      , "aesdeclast"      , Enc(ExtRm)             , O(660F38,DF,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 3  , 2610, 6  ),
  INST(Aesenc          , "aesenc"          , Enc(ExtRm)             , O(660F38,DC,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 3  , 2622, 6  ),
  INST(Aesenclast      , "aesenclast"      , Enc(ExtRm)             , O(660F38,DD,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 3  , 2630, 6  ),
  INST(Aesimc          , "aesimc"          , Enc(ExtRm)             , O(660F38,DB,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 0 , kFamilySse , 4  , 2642, 10 ),
  INST(Aeskeygenassist , "aeskeygenassist" , Enc(ExtRmi)            , O(660F3A,DF,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 0 , kFamilySse , 4  , 2650, 11 ),
  INST(And             , "and"             , Enc(X86Arith)          , O(000000,20,4,_,x,_,_,_  ), 0                         , F(RW)|F(Lock)                         , EF(WWWUWW__), 0 , 0 , kFamilyNone, 0  , 2161, 5  ),
  INST(Andn            , "andn"            , Enc(VexRvm_Wx)         , V(000F38,F2,_,0,x,_,_,_  ), 0                         , F(RW)                                 , EF(WWWUUW__), 0 , 0 , kFamilyNone, 0  , 5867, 12 ),
  INST(Andnpd          , "andnpd"          , Enc(ExtRm)             , O(660F00,55,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 5  , 2683, 6  ),
  INST(Andnps          , "andnps"          , Enc(ExtRm)             , O(000F00,55,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 6  , 2691, 6  ),
  INST(Andpd           , "andpd"           , Enc(ExtRm)             , O(660F00,54,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 5  , 3610, 6  ),
  INST(Andps           , "andps"           , Enc(ExtRm)             , O(000F00,54,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 6  , 3620, 6  ),
  INST(Bextr           , "bextr"           , Enc(VexRmv_Wx)         , V(000F38,F7,_,0,x,_,_,_  ), 0                         , F(RW)                                 , EF(WUWUUW__), 0 , 0 , kFamilyNone, 0  , 31  , 13 ),
  INST(Blcfill         , "blcfill"         , Enc(VexVm_Wx)          , V(XOP_M9,01,1,0,x,_,_,_  ), 0                         , F(WO)                                 , EF(WWWUUW__), 0 , 0 , kFamilyNone, 0  , 37  , 14 ),
  INST(Blci            , "blci"            , Enc(VexVm_Wx)          , V(XOP_M9,02,6,0,x,_,_,_  ), 0                         , F(WO)                                 , EF(WWWUUW__), 0 , 0 , kFamilyNone, 0  , 45  , 14 ),
  INST(Blcic           , "blcic"           , Enc(VexVm_Wx)          , V(XOP_M9,01,5,0,x,_,_,_  ), 0                         , F(WO)                                 , EF(WWWUUW__), 0 , 0 , kFamilyNone, 0  , 50  , 14 ),
  INST(Blcmsk          , "blcmsk"          , Enc(VexVm_Wx)          , V(XOP_M9,02,1,0,x,_,_,_  ), 0                         , F(WO)                                 , EF(WWWUUW__), 0 , 0 , kFamilyNone, 0  , 56  , 14 ),
  INST(Blcs            , "blcs"            , Enc(VexVm_Wx)          , V(XOP_M9,01,3,0,x,_,_,_  ), 0                         , F(WO)                                 , EF(WWWUUW__), 0 , 0 , kFamilyNone, 0  , 63  , 14 ),
  INST(Blendpd         , "blendpd"         , Enc(ExtRmi)            , O(660F3A,0D,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 7  , 2769, 15 ),
  INST(Blendps         , "blendps"         , Enc(ExtRmi)            , O(660F3A,0C,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 7  , 2778, 15 ),
  INST(Blendvpd        , "blendvpd"        , Enc(ExtRm_XMM0)        , O(660F38,15,_,_,_,_,_,_  ), 0                         , F(RW)|F(Special)                      , EF(________), 0 , 0 , kFamilySse , 8  , 2787, 16 ),
  INST(Blendvps        , "blendvps"        , Enc(ExtRm_XMM0)        , O(660F38,14,_,_,_,_,_,_  ), 0                         , F(RW)|F(Special)                      , EF(________), 0 , 0 , kFamilySse , 8  , 2797, 16 ),
  INST(Blsfill         , "blsfill"         , Enc(VexVm_Wx)          , V(XOP_M9,01,2,0,x,_,_,_  ), 0                         , F(WO)                                 , EF(WWWUUW__), 0 , 0 , kFamilyNone, 0  , 68  , 14 ),
  INST(Blsi            , "blsi"            , Enc(VexVm_Wx)          , V(000F38,F3,3,0,x,_,_,_  ), 0                         , F(RW)                                 , EF(WWWUUW__), 0 , 0 , kFamilyNone, 0  , 76  , 17 ),
  INST(Blsic           , "blsic"           , Enc(VexVm_Wx)          , V(XOP_M9,01,6,0,x,_,_,_  ), 0                         , F(WO)                                 , EF(WWWUUW__), 0 , 0 , kFamilyNone, 0  , 81  , 14 ),
  INST(Blsmsk          , "blsmsk"          , Enc(VexVm_Wx)          , V(000F38,F3,2,0,x,_,_,_  ), 0                         , F(RW)                                 , EF(WWWUUW__), 0 , 0 , kFamilyNone, 0  , 87  , 17 ),
  INST(Blsr            , "blsr"            , Enc(VexVm_Wx)          , V(000F38,F3,1,0,x,_,_,_  ), 0                         , F(RW)                                 , EF(WWWUUW__), 0 , 0 , kFamilyNone, 0  , 94  , 17 ),
  INST(Bsf             , "bsf"             , Enc(X86Rm)             , O(000F00,BC,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(UUWUUU__), 0 , 0 , kFamilyNone, 0  , 99  , 18 ),
  INST(Bsr             , "bsr"             , Enc(X86Rm)             , O(000F00,BD,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(UUWUUU__), 0 , 0 , kFamilyNone, 0  , 103 , 18 ),
  INST(Bswap           , "bswap"           , Enc(X86Bswap)          , O(000F00,C8,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 107 , 19 ),
  INST(Bt              , "bt"              , Enc(X86Bt)             , O(000F00,A3,_,_,x,_,_,_  ), O(000F00,BA,4,_,x,_,_,_  ), F(RO)                                 , EF(UU_UUW__), 0 , 0 , kFamilyNone, 0  , 113 , 20 ),
  INST(Btc             , "btc"             , Enc(X86Bt)             , O(000F00,BB,_,_,x,_,_,_  ), O(000F00,BA,7,_,x,_,_,_  ), F(RW)|F(Lock)                         , EF(UU_UUW__), 0 , 0 , kFamilyNone, 0  , 116 , 21 ),
  INST(Btr             , "btr"             , Enc(X86Bt)             , O(000F00,B3,_,_,x,_,_,_  ), O(000F00,BA,6,_,x,_,_,_  ), F(RW)|F(Lock)                         , EF(UU_UUW__), 0 , 0 , kFamilyNone, 0  , 120 , 22 ),
  INST(Bts             , "bts"             , Enc(X86Bt)             , O(000F00,AB,_,_,x,_,_,_  ), O(000F00,BA,5,_,x,_,_,_  ), F(RW)|F(Lock)                         , EF(UU_UUW__), 0 , 0 , kFamilyNone, 0  , 124 , 23 ),
  INST(Bzhi            , "bzhi"            , Enc(VexRmv_Wx)         , V(000F38,F5,_,0,x,_,_,_  ), 0                         , F(RW)                                 , EF(WWWUUW__), 0 , 0 , kFamilyNone, 0  , 128 , 13 ),
  INST(Call            , "call"            , Enc(X86Call)           , O(000000,FF,2,_,_,_,_,_  ), 0                         , F(RW)|F(Flow)|F(Volatile)             , EF(________), 0 , 0 , kFamilyNone, 0  , 133 , 24 ),
  INST(Cbw             , "cbw"             , Enc(X86Op_xAX)         , O(660000,98,_,_,_,_,_,_  ), 0                         , F(RW)|F(Special)                      , EF(________), 0 , 0 , kFamilyNone, 0  , 138 , 25 ),
  INST(Cdq             , "cdq"             , Enc(X86Op_xDX_xAX)     , O(000000,99,_,_,_,_,_,_  ), 0                         , F(RW)|F(Special)                      , EF(________), 0 , 0 , kFamilyNone, 0  , 142 , 26 ),
  INST(Cdqe            , "cdqe"            , Enc(X86Op_xAX)         , O(000000,98,_,_,1,_,_,_  ), 0                         , F(RW)|F(Special)                      , EF(________), 0 , 0 , kFamilyNone, 0  , 146 , 27 ),
  INST(Clac            , "clac"            , Enc(X86Op)             , O(000F01,CA,_,_,_,_,_,_  ), 0                         , F(Volatile)                           , EF(___W____), 0 , 0 , kFamilyNone, 0  , 151 , 28 ),
  INST(Clc             , "clc"             , Enc(X86Op)             , O(000000,F8,_,_,_,_,_,_  ), 0                         , F(Volatile)                           , EF(_____W__), 0 , 0 , kFamilyNone, 0  , 156 , 29 ),
  INST(Cld             , "cld"             , Enc(X86Op)             , O(000000,FC,_,_,_,_,_,_  ), 0                         , F(Volatile)                           , EF(______W_), 0 , 0 , kFamilyNone, 0  , 160 , 30 ),
  INST(Clflush         , "clflush"         , Enc(X86M_Only)         , O(000F00,AE,7,_,_,_,_,_  ), 0                         , F(RO)|F(Volatile)                     , EF(________), 0 , 0 , kFamilyNone, 0  , 164 , 31 ),
  INST(Clflushopt      , "clflushopt"      , Enc(X86M_Only)         , O(660F00,AE,7,_,_,_,_,_  ), 0                         , F(RO)|F(Volatile)                     , EF(________), 0 , 0 , kFamilyNone, 0  , 172 , 31 ),
  INST(Clwb            , "clwb"            , Enc(X86M_Only)         , O(660F00,AE,6,_,_,_,_,_  ), 0                         , F(RO)|F(Volatile)                     , EF(________), 0 , 0 , kFamilyNone, 0  , 183 , 31 ),
  INST(Clzero          , "clzero"          , Enc(X86Op_ZAX)         , O(000F01,FC,_,_,_,_,_,_  ), 0                         , F(WO)|F(Volatile)|F(Special)          , EF(________), 0 , 0 , kFamilyNone, 0  , 188 , 32 ),
  INST(Cmc             , "cmc"             , Enc(X86Op)             , O(000000,F5,_,_,_,_,_,_  ), 0                         , 0                                     , EF(_____X__), 0 , 0 , kFamilyNone, 0  , 195 , 33 ),
  INST(Cmova           , "cmova"           , Enc(X86Rm)             , O(000F00,47,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(__R__R__), 0 , 0 , kFamilyNone, 0  , 199 , 34 ),
  INST(Cmovae          , "cmovae"          , Enc(X86Rm)             , O(000F00,43,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(_____R__), 0 , 0 , kFamilyNone, 0  , 205 , 35 ),
  INST(Cmovb           , "cmovb"           , Enc(X86Rm)             , O(000F00,42,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(_____R__), 0 , 0 , kFamilyNone, 0  , 535 , 35 ),
  INST(Cmovbe          , "cmovbe"          , Enc(X86Rm)             , O(000F00,46,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(__R__R__), 0 , 0 , kFamilyNone, 0  , 542 , 34 ),
  INST(Cmovc           , "cmovc"           , Enc(X86Rm)             , O(000F00,42,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(_____R__), 0 , 0 , kFamilyNone, 0  , 212 , 35 ),
  INST(Cmove           , "cmove"           , Enc(X86Rm)             , O(000F00,44,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(__R_____), 0 , 0 , kFamilyNone, 0  , 550 , 36 ),
  INST(Cmovg           , "cmovg"           , Enc(X86Rm)             , O(000F00,4F,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(RRR_____), 0 , 0 , kFamilyNone, 0  , 218 , 37 ),
  INST(Cmovge          , "cmovge"          , Enc(X86Rm)             , O(000F00,4D,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(RR______), 0 , 0 , kFamilyNone, 0  , 224 , 38 ),
  INST(Cmovl           , "cmovl"           , Enc(X86Rm)             , O(000F00,4C,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(RR______), 0 , 0 , kFamilyNone, 0  , 231 , 38 ),
  INST(Cmovle          , "cmovle"          , Enc(X86Rm)             , O(000F00,4E,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(RRR_____), 0 , 0 , kFamilyNone, 0  , 237 , 37 ),
  INST(Cmovna          , "cmovna"          , Enc(X86Rm)             , O(000F00,46,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(__R__R__), 0 , 0 , kFamilyNone, 0  , 244 , 34 ),
  INST(Cmovnae         , "cmovnae"         , Enc(X86Rm)             , O(000F00,42,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(_____R__), 0 , 0 , kFamilyNone, 0  , 251 , 35 ),
  INST(Cmovnb          , "cmovnb"          , Enc(X86Rm)             , O(000F00,43,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(_____R__), 0 , 0 , kFamilyNone, 0  , 557 , 35 ),
  INST(Cmovnbe         , "cmovnbe"         , Enc(X86Rm)             , O(000F00,47,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(__R__R__), 0 , 0 , kFamilyNone, 0  , 565 , 34 ),
  INST(Cmovnc          , "cmovnc"          , Enc(X86Rm)             , O(000F00,43,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(_____R__), 0 , 0 , kFamilyNone, 0  , 259 , 35 ),
  INST(Cmovne          , "cmovne"          , Enc(X86Rm)             , O(000F00,45,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(__R_____), 0 , 0 , kFamilyNone, 0  , 574 , 36 ),
  INST(Cmovng          , "cmovng"          , Enc(X86Rm)             , O(000F00,4E,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(RRR_____), 0 , 0 , kFamilyNone, 0  , 266 , 37 ),
  INST(Cmovnge         , "cmovnge"         , Enc(X86Rm)             , O(000F00,4C,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(RR______), 0 , 0 , kFamilyNone, 0  , 273 , 38 ),
  INST(Cmovnl          , "cmovnl"          , Enc(X86Rm)             , O(000F00,4D,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(RR______), 0 , 0 , kFamilyNone, 0  , 281 , 38 ),
  INST(Cmovnle         , "cmovnle"         , Enc(X86Rm)             , O(000F00,4F,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(RRR_____), 0 , 0 , kFamilyNone, 0  , 288 , 37 ),
  INST(Cmovno          , "cmovno"          , Enc(X86Rm)             , O(000F00,41,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(R_______), 0 , 0 , kFamilyNone, 0  , 296 , 39 ),
  INST(Cmovnp          , "cmovnp"          , Enc(X86Rm)             , O(000F00,4B,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(____R___), 0 , 0 , kFamilyNone, 0  , 303 , 40 ),
  INST(Cmovns          , "cmovns"          , Enc(X86Rm)             , O(000F00,49,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(_R______), 0 , 0 , kFamilyNone, 0  , 310 , 41 ),
  INST(Cmovnz          , "cmovnz"          , Enc(X86Rm)             , O(000F00,45,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(__R_____), 0 , 0 , kFamilyNone, 0  , 317 , 36 ),
  INST(Cmovo           , "cmovo"           , Enc(X86Rm)             , O(000F00,40,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(R_______), 0 , 0 , kFamilyNone, 0  , 324 , 39 ),
  INST(Cmovp           , "cmovp"           , Enc(X86Rm)             , O(000F00,4A,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(____R___), 0 , 0 , kFamilyNone, 0  , 330 , 40 ),
  INST(Cmovpe          , "cmovpe"          , Enc(X86Rm)             , O(000F00,4A,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(____R___), 0 , 0 , kFamilyNone, 0  , 336 , 40 ),
  INST(Cmovpo          , "cmovpo"          , Enc(X86Rm)             , O(000F00,4B,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(____R___), 0 , 0 , kFamilyNone, 0  , 343 , 40 ),
  INST(Cmovs           , "cmovs"           , Enc(X86Rm)             , O(000F00,48,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(_R______), 0 , 0 , kFamilyNone, 0  , 350 , 41 ),
  INST(Cmovz           , "cmovz"           , Enc(X86Rm)             , O(000F00,44,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(__R_____), 0 , 0 , kFamilyNone, 0  , 356 , 36 ),
  INST(Cmp             , "cmp"             , Enc(X86Arith)          , O(000000,38,7,_,x,_,_,_  ), 0                         , F(RO)                                 , EF(WWWWWW__), 0 , 0 , kFamilyNone, 0  , 362 , 42 ),
  INST(Cmppd           , "cmppd"           , Enc(ExtRmi)            , O(660F00,C2,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 9  , 3023, 15 ),
  INST(Cmpps           , "cmpps"           , Enc(ExtRmi)            , O(000F00,C2,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 10 , 3030, 15 ),
  INST(Cmps            , "cmps"            , Enc(X86StrMm)          , O(000000,A6,_,_,_,_,_,_  ), 0                         , F(RW)|F(Special)|F(Rep)|F(Repnz)      , EF(WWWWWWR_), 0 , 0 , kFamilyNone, 0  , 366 , 43 ),
  INST(Cmpsd           , "cmpsd"           , Enc(ExtRmi)            , O(F20F00,C2,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 11 , 3037, 44 ),
  INST(Cmpss           , "cmpss"           , Enc(ExtRmi)            , O(F30F00,C2,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 12 , 3044, 45 ),
  INST(Cmpxchg         , "cmpxchg"         , Enc(X86Cmpxchg)        , O(000F00,B0,_,_,x,_,_,_  ), 0                         , F(RW)|F(Lock)|F(Special)              , EF(WWWWWW__), 0 , 0 , kFamilyNone, 0  , 371 , 46 ),
  INST(Cmpxchg16b      , "cmpxchg16b"      , Enc(X86M_Only)         , O(000F00,C7,1,_,1,_,_,_  ), 0                         , F(RW)|F(Lock)|F(Special)              , EF(__W_____), 0 , 0 , kFamilyNone, 0  , 379 , 47 ),
  INST(Cmpxchg8b       , "cmpxchg8b"       , Enc(X86M_Only)         , O(000F00,C7,1,_,_,_,_,_  ), 0                         , F(RW)|F(Lock)|F(Special)              , EF(__W_____), 0 , 0 , kFamilyNone, 0  , 390 , 48 ),
  INST(Comisd          , "comisd"          , Enc(ExtRm)             , O(660F00,2F,_,_,_,_,_,_  ), 0                         , F(RO)                                 , EF(WWWWWW__), 0 , 0 , kFamilySse , 13 , 9070, 49 ),
  INST(Comiss          , "comiss"          , Enc(ExtRm)             , O(000F00,2F,_,_,_,_,_,_  ), 0                         , F(RO)                                 , EF(WWWWWW__), 0 , 0 , kFamilySse , 14 , 9079, 50 ),
  INST(Cpuid           , "cpuid"           , Enc(X86Op)             , O(000F00,A2,_,_,_,_,_,_  ), 0                         , F(RW)|F(Volatile)|F(Special)          , EF(________), 0 , 0 , kFamilyNone, 0  , 400 , 51 ),
  INST(Cqo             , "cqo"             , Enc(X86Op_xDX_xAX)     , O(000000,99,_,_,1,_,_,_  ), 0                         , F(RW)|F(Special)                      , EF(________), 0 , 0 , kFamilyNone, 0  , 406 , 52 ),
  INST(Crc32           , "crc32"           , Enc(X86Crc)            , O(F20F38,F0,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 410 , 53 ),
  INST(Cvtdq2pd        , "cvtdq2pd"        , Enc(ExtRm)             , O(F30F00,E6,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 15 , 3091, 54 ),
  INST(Cvtdq2ps        , "cvtdq2ps"        , Enc(ExtRm)             , O(000F00,5B,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 15 , 3101, 55 ),
  INST(Cvtpd2dq        , "cvtpd2dq"        , Enc(ExtRm)             , O(F20F00,E6,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 15 , 3111, 55 ),
  INST(Cvtpd2pi        , "cvtpd2pi"        , Enc(ExtRm)             , O(660F00,2D,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 16 , 416 , 56 ),
  INST(Cvtpd2ps        , "cvtpd2ps"        , Enc(ExtRm)             , O(660F00,5A,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 17 , 3121, 55 ),
  INST(Cvtpi2pd        , "cvtpi2pd"        , Enc(ExtRm)             , O(660F00,2A,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 16 , 425 , 57 ),
  INST(Cvtpi2ps        , "cvtpi2ps"        , Enc(ExtRm)             , O(000F00,2A,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 18 , 434 , 58 ),
  INST(Cvtps2dq        , "cvtps2dq"        , Enc(ExtRm)             , O(660F00,5B,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 13 , 3173, 55 ),
  INST(Cvtps2pd        , "cvtps2pd"        , Enc(ExtRm)             , O(000F00,5A,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 13 , 3183, 54 ),
  INST(Cvtps2pi        , "cvtps2pi"        , Enc(ExtRm)             , O(000F00,2D,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 18 , 443 , 59 ),
  INST(Cvtsd2si        , "cvtsd2si"        , Enc(ExtRm_Wx)          , O(F20F00,2D,_,_,x,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 19 , 3255, 60 ),
  INST(Cvtsd2ss        , "cvtsd2ss"        , Enc(ExtRm)             , O(F20F00,5A,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 4 , kFamilySse , 20 , 3265, 61 ),
  INST(Cvtsi2sd        , "cvtsi2sd"        , Enc(ExtRm_Wx)          , O(F20F00,2A,_,_,x,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 21 , 3286, 62 ),
  INST(Cvtsi2ss        , "cvtsi2ss"        , Enc(ExtRm_Wx)          , O(F30F00,2A,_,_,x,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 4 , kFamilySse , 22 , 3296, 63 ),
  INST(Cvtss2sd        , "cvtss2sd"        , Enc(ExtRm)             , O(F30F00,5A,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 21 , 3306, 64 ),
  INST(Cvtss2si        , "cvtss2si"        , Enc(ExtRm_Wx)          , O(F30F00,2D,_,_,x,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 23 , 3316, 65 ),
  INST(Cvttpd2dq       , "cvttpd2dq"       , Enc(ExtRm)             , O(660F00,E6,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 24 , 3337, 55 ),
  INST(Cvttpd2pi       , "cvttpd2pi"       , Enc(ExtRm)             , O(660F00,2C,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 16 , 452 , 56 ),
  INST(Cvttps2dq       , "cvttps2dq"       , Enc(ExtRm)             , O(F30F00,5B,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 25 , 3383, 55 ),
  INST(Cvttps2pi       , "cvttps2pi"       , Enc(ExtRm)             , O(000F00,2C,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 18 , 462 , 59 ),
  INST(Cvttsd2si       , "cvttsd2si"       , Enc(ExtRm_Wx)          , O(F20F00,2C,_,_,x,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 26 , 3429, 60 ),
  INST(Cvttss2si       , "cvttss2si"       , Enc(ExtRm_Wx)          , O(F30F00,2C,_,_,x,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 27 , 3452, 65 ),
  INST(Cwd             , "cwd"             , Enc(X86Op_xDX_xAX)     , O(660000,99,_,_,_,_,_,_  ), 0                         , F(RW)|F(Special)                      , EF(________), 0 , 0 , kFamilyNone, 0  , 472 , 66 ),
  INST(Cwde            , "cwde"            , Enc(X86Op_xAX)         , O(000000,98,_,_,_,_,_,_  ), 0                         , F(RW)|F(Special)                      , EF(________), 0 , 0 , kFamilyNone, 0  , 476 , 67 ),
  INST(Daa             , "daa"             , Enc(X86Op)             , O(000000,27,_,_,_,_,_,_  ), 0                         , F(RW)|F(Special)                      , EF(UWWXWX__), 0 , 0 , kFamilyNone, 0  , 481 , 68 ),
  INST(Das             , "das"             , Enc(X86Op)             , O(000000,2F,_,_,_,_,_,_  ), 0                         , F(RW)|F(Special)                      , EF(UWWXWX__), 0 , 0 , kFamilyNone, 0  , 485 , 68 ),
  INST(Dec             , "dec"             , Enc(X86IncDec)         , O(000000,FE,1,_,x,_,_,_  ), O(000000,48,_,_,x,_,_,_  ), F(RW)|F(Lock)                         , EF(WWWWW___), 0 , 0 , kFamilyNone, 0  , 2605, 69 ),
  INST(Div             , "div"             , Enc(X86M_GPB_MulDiv)   , O(000000,F6,6,_,x,_,_,_  ), 0                         , F(RW)|F(Special)                      , EF(UUUUUU__), 0 , 0 , kFamilyNone, 0  , 697 , 70 ),
  INST(Divpd           , "divpd"           , Enc(ExtRm)             , O(660F00,5E,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 28 , 3551, 6  ),
  INST(Divps           , "divps"           , Enc(ExtRm)             , O(000F00,5E,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 29 , 3558, 6  ),
  INST(Divsd           , "divsd"           , Enc(ExtRm)             , O(F20F00,5E,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 28 , 3565, 7  ),
  INST(Divss           , "divss"           , Enc(ExtRm)             , O(F30F00,5E,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 29 , 3572, 8  ),
  INST(Dppd            , "dppd"            , Enc(ExtRmi)            , O(660F3A,41,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 30 , 3579, 15 ),
  INST(Dpps            , "dpps"            , Enc(ExtRmi)            , O(660F3A,40,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 30 , 3585, 15 ),
  INST(Emms            , "emms"            , Enc(X86Op)             , O(000F00,77,_,_,_,_,_,_  ), 0                         , F(Volatile)                           , EF(________), 0 , 0 , kFamilyNone, 0  , 665 , 71 ),
  INST(Enter           , "enter"           , Enc(X86Enter)          , O(000000,C8,_,_,_,_,_,_  ), 0                         , F(Volatile)|F(Special)                , EF(________), 0 , 0 , kFamilyNone, 0  , 489 , 72 ),
  INST(Extractps       , "extractps"       , Enc(ExtExtract)        , O(660F3A,17,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 31 , 3765, 73 ),
  INST(Extrq           , "extrq"           , Enc(ExtExtrq)          , O(660F00,79,_,_,_,_,_,_  ), O(660F00,78,0,_,_,_,_,_  ), F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 32 , 6581, 74 ),
  INST(F2xm1           , "f2xm1"           , Enc(FpuOp)             , O_FPU(00,D9F0,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 495 , 75 ),
  INST(Fabs            , "fabs"            , Enc(FpuOp)             , O_FPU(00,D9E1,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 501 , 75 ),
  INST(Fadd            , "fadd"            , Enc(FpuArith)          , O_FPU(00,C0C0,0)          , 0                         , F(Fp)|F(FPU_M4)|F(FPU_M8)             , EF(________), 0 , 0 , kFamilyNone, 0  , 1813, 76 ),
  INST(Faddp           , "faddp"           , Enc(FpuRDef)           , O_FPU(00,DEC0,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 506 , 77 ),
  INST(Fbld            , "fbld"            , Enc(X86M_Only)         , O_FPU(00,00DF,4)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 512 , 78 ),
  INST(Fbstp           , "fbstp"           , Enc(X86M_Only)         , O_FPU(00,00DF,6)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 517 , 78 ),
  INST(Fchs            , "fchs"            , Enc(FpuOp)             , O_FPU(00,D9E0,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 523 , 75 ),
  INST(Fclex           , "fclex"           , Enc(FpuOp)             , O_FPU(9B,DBE2,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 528 , 75 ),
  INST(Fcmovb          , "fcmovb"          , Enc(FpuR)              , O_FPU(00,DAC0,_)          , 0                         , F(Fp)                                 , EF(_____R__), 0 , 0 , kFamilyNone, 0  , 534 , 79 ),
  INST(Fcmovbe         , "fcmovbe"         , Enc(FpuR)              , O_FPU(00,DAD0,_)          , 0                         , F(Fp)                                 , EF(__R__R__), 0 , 0 , kFamilyNone, 0  , 541 , 80 ),
  INST(Fcmove          , "fcmove"          , Enc(FpuR)              , O_FPU(00,DAC8,_)          , 0                         , F(Fp)                                 , EF(__R_____), 0 , 0 , kFamilyNone, 0  , 549 , 81 ),
  INST(Fcmovnb         , "fcmovnb"         , Enc(FpuR)              , O_FPU(00,DBC0,_)          , 0                         , F(Fp)                                 , EF(_____R__), 0 , 0 , kFamilyNone, 0  , 556 , 79 ),
  INST(Fcmovnbe        , "fcmovnbe"        , Enc(FpuR)              , O_FPU(00,DBD0,_)          , 0                         , F(Fp)                                 , EF(__R__R__), 0 , 0 , kFamilyNone, 0  , 564 , 80 ),
  INST(Fcmovne         , "fcmovne"         , Enc(FpuR)              , O_FPU(00,DBC8,_)          , 0                         , F(Fp)                                 , EF(__R_____), 0 , 0 , kFamilyNone, 0  , 573 , 81 ),
  INST(Fcmovnu         , "fcmovnu"         , Enc(FpuR)              , O_FPU(00,DBD8,_)          , 0                         , F(Fp)                                 , EF(____R___), 0 , 0 , kFamilyNone, 0  , 581 , 82 ),
  INST(Fcmovu          , "fcmovu"          , Enc(FpuR)              , O_FPU(00,DAD8,_)          , 0                         , F(Fp)                                 , EF(____R___), 0 , 0 , kFamilyNone, 0  , 589 , 82 ),
  INST(Fcom            , "fcom"            , Enc(FpuCom)            , O_FPU(00,D0D0,2)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 596 , 83 ),
  INST(Fcomi           , "fcomi"           , Enc(FpuR)              , O_FPU(00,DBF0,_)          , 0                         , F(Fp)                                 , EF(WWWWWW__), 0 , 0 , kFamilyNone, 0  , 601 , 84 ),
  INST(Fcomip          , "fcomip"          , Enc(FpuR)              , O_FPU(00,DFF0,_)          , 0                         , F(Fp)                                 , EF(WWWWWW__), 0 , 0 , kFamilyNone, 0  , 607 , 84 ),
  INST(Fcomp           , "fcomp"           , Enc(FpuCom)            , O_FPU(00,D8D8,3)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 614 , 83 ),
  INST(Fcompp          , "fcompp"          , Enc(FpuOp)             , O_FPU(00,DED9,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 620 , 75 ),
  INST(Fcos            , "fcos"            , Enc(FpuOp)             , O_FPU(00,D9FF,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 627 , 75 ),
  INST(Fdecstp         , "fdecstp"         , Enc(FpuOp)             , O_FPU(00,D9F6,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 632 , 75 ),
  INST(Fdiv            , "fdiv"            , Enc(FpuArith)          , O_FPU(00,F0F8,6)          , 0                         , F(Fp)|F(FPU_M4)|F(FPU_M8)             , EF(________), 0 , 0 , kFamilyNone, 0  , 640 , 76 ),
  INST(Fdivp           , "fdivp"           , Enc(FpuRDef)           , O_FPU(00,DEF8,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 645 , 77 ),
  INST(Fdivr           , "fdivr"           , Enc(FpuArith)          , O_FPU(00,F8F0,7)          , 0                         , F(Fp)|F(FPU_M4)|F(FPU_M8)             , EF(________), 0 , 0 , kFamilyNone, 0  , 651 , 76 ),
  INST(Fdivrp          , "fdivrp"          , Enc(FpuRDef)           , O_FPU(00,DEF0,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 657 , 77 ),
  INST(Femms           , "femms"           , Enc(X86Op)             , O(000F00,0E,_,_,_,_,_,_  ), 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 664 , 75 ),
  INST(Ffree           , "ffree"           , Enc(FpuR)              , O_FPU(00,DDC0,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 670 , 85 ),
  INST(Fiadd           , "fiadd"           , Enc(FpuM)              , O_FPU(00,00DA,0)          , 0                         , F(Fp)|F(FPU_M2)|F(FPU_M4)             , EF(________), 0 , 0 , kFamilyNone, 0  , 676 , 86 ),
  INST(Ficom           , "ficom"           , Enc(FpuM)              , O_FPU(00,00DA,2)          , 0                         , F(Fp)|F(FPU_M2)|F(FPU_M4)             , EF(________), 0 , 0 , kFamilyNone, 0  , 682 , 86 ),
  INST(Ficomp          , "ficomp"          , Enc(FpuM)              , O_FPU(00,00DA,3)          , 0                         , F(Fp)|F(FPU_M2)|F(FPU_M4)             , EF(________), 0 , 0 , kFamilyNone, 0  , 688 , 86 ),
  INST(Fidiv           , "fidiv"           , Enc(FpuM)              , O_FPU(00,00DA,6)          , 0                         , F(Fp)|F(FPU_M2)|F(FPU_M4)             , EF(________), 0 , 0 , kFamilyNone, 0  , 695 , 86 ),
  INST(Fidivr          , "fidivr"          , Enc(FpuM)              , O_FPU(00,00DA,7)          , 0                         , F(Fp)|F(FPU_M2)|F(FPU_M4)             , EF(________), 0 , 0 , kFamilyNone, 0  , 701 , 86 ),
  INST(Fild            , "fild"            , Enc(FpuM)              , O_FPU(00,00DB,0)          , O_FPU(00,00DF,5)          , F(Fp)|F(FPU_M2)|F(FPU_M4)|F(FPU_M8)   , EF(________), 0 , 0 , kFamilyNone, 0  , 708 , 87 ),
  INST(Fimul           , "fimul"           , Enc(FpuM)              , O_FPU(00,00DA,1)          , 0                         , F(Fp)|F(FPU_M2)|F(FPU_M4)             , EF(________), 0 , 0 , kFamilyNone, 0  , 713 , 86 ),
  INST(Fincstp         , "fincstp"         , Enc(FpuOp)             , O_FPU(00,D9F7,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 719 , 75 ),
  INST(Finit           , "finit"           , Enc(FpuOp)             , O_FPU(9B,DBE3,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 727 , 75 ),
  INST(Fist            , "fist"            , Enc(FpuM)              , O_FPU(00,00DB,2)          , 0                         , F(Fp)|F(FPU_M2)|F(FPU_M4)             , EF(________), 0 , 0 , kFamilyNone, 0  , 733 , 86 ),
  INST(Fistp           , "fistp"           , Enc(FpuM)              , O_FPU(00,00DB,3)          , O_FPU(00,00DF,7)          , F(Fp)|F(FPU_M2)|F(FPU_M4)|F(FPU_M8)   , EF(________), 0 , 0 , kFamilyNone, 0  , 738 , 88 ),
  INST(Fisttp          , "fisttp"          , Enc(FpuM)              , O_FPU(00,00DB,1)          , O_FPU(00,00DD,1)          , F(Fp)|F(FPU_M2)|F(FPU_M4)|F(FPU_M8)   , EF(________), 0 , 0 , kFamilyNone, 0  , 744 , 89 ),
  INST(Fisub           , "fisub"           , Enc(FpuM)              , O_FPU(00,00DA,4)          , 0                         , F(Fp)|F(FPU_M2)|F(FPU_M4)             , EF(________), 0 , 0 , kFamilyNone, 0  , 751 , 86 ),
  INST(Fisubr          , "fisubr"          , Enc(FpuM)              , O_FPU(00,00DA,5)          , 0                         , F(Fp)|F(FPU_M2)|F(FPU_M4)             , EF(________), 0 , 0 , kFamilyNone, 0  , 757 , 86 ),
  INST(Fld             , "fld"             , Enc(FpuFldFst)         , O_FPU(00,00D9,0)          , O_FPU(00,00DB,5)          , F(Fp)|F(FPU_M2)|F(FPU_M4)|F(FPU_M8)   , EF(________), 0 , 0 , kFamilyNone, 0  , 764 , 90 ),
  INST(Fld1            , "fld1"            , Enc(FpuOp)             , O_FPU(00,D9E8,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 768 , 75 ),
  INST(Fldcw           , "fldcw"           , Enc(X86M_Only)         , O_FPU(00,00D9,5)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 773 , 91 ),
  INST(Fldenv          , "fldenv"          , Enc(X86M_Only)         , O_FPU(00,00D9,4)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 779 , 92 ),
  INST(Fldl2e          , "fldl2e"          , Enc(FpuOp)             , O_FPU(00,D9EA,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 786 , 75 ),
  INST(Fldl2t          , "fldl2t"          , Enc(FpuOp)             , O_FPU(00,D9E9,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 793 , 75 ),
  INST(Fldlg2          , "fldlg2"          , Enc(FpuOp)             , O_FPU(00,D9EC,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 800 , 75 ),
  INST(Fldln2          , "fldln2"          , Enc(FpuOp)             , O_FPU(00,D9ED,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 807 , 75 ),
  INST(Fldpi           , "fldpi"           , Enc(FpuOp)             , O_FPU(00,D9EB,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 814 , 75 ),
  INST(Fldz            , "fldz"            , Enc(FpuOp)             , O_FPU(00,D9EE,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 820 , 75 ),
  INST(Fmul            , "fmul"            , Enc(FpuArith)          , O_FPU(00,C8C8,1)          , 0                         , F(Fp)|F(FPU_M4)|F(FPU_M8)             , EF(________), 0 , 0 , kFamilyNone, 0  , 1855, 76 ),
  INST(Fmulp           , "fmulp"           , Enc(FpuRDef)           , O_FPU(00,DEC8,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 825 , 77 ),
  INST(Fnclex          , "fnclex"          , Enc(FpuOp)             , O_FPU(00,DBE2,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 831 , 75 ),
  INST(Fninit          , "fninit"          , Enc(FpuOp)             , O_FPU(00,DBE3,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 838 , 75 ),
  INST(Fnop            , "fnop"            , Enc(FpuOp)             , O_FPU(00,D9D0,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 845 , 75 ),
  INST(Fnsave          , "fnsave"          , Enc(X86M_Only)         , O_FPU(00,00DD,6)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 850 , 92 ),
  INST(Fnstcw          , "fnstcw"          , Enc(X86M_Only)         , O_FPU(00,00D9,7)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 857 , 91 ),
  INST(Fnstenv         , "fnstenv"         , Enc(X86M_Only)         , O_FPU(00,00D9,6)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 864 , 92 ),
  INST(Fnstsw          , "fnstsw"          , Enc(FpuStsw)           , O_FPU(00,00DD,7)          , O_FPU(00,DFE0,_)          , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 872 , 93 ),
  INST(Fpatan          , "fpatan"          , Enc(FpuOp)             , O_FPU(00,D9F3,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 879 , 75 ),
  INST(Fprem           , "fprem"           , Enc(FpuOp)             , O_FPU(00,D9F8,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 886 , 75 ),
  INST(Fprem1          , "fprem1"          , Enc(FpuOp)             , O_FPU(00,D9F5,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 892 , 75 ),
  INST(Fptan           , "fptan"           , Enc(FpuOp)             , O_FPU(00,D9F2,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 899 , 75 ),
  INST(Frndint         , "frndint"         , Enc(FpuOp)             , O_FPU(00,D9FC,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 905 , 75 ),
  INST(Frstor          , "frstor"          , Enc(X86M_Only)         , O_FPU(00,00DD,4)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 913 , 92 ),
  INST(Fsave           , "fsave"           , Enc(X86M_Only)         , O_FPU(9B,00DD,6)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 920 , 92 ),
  INST(Fscale          , "fscale"          , Enc(FpuOp)             , O_FPU(00,D9FD,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 926 , 75 ),
  INST(Fsin            , "fsin"            , Enc(FpuOp)             , O_FPU(00,D9FE,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 933 , 75 ),
  INST(Fsincos         , "fsincos"         , Enc(FpuOp)             , O_FPU(00,D9FB,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 938 , 75 ),
  INST(Fsqrt           , "fsqrt"           , Enc(FpuOp)             , O_FPU(00,D9FA,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 946 , 75 ),
  INST(Fst             , "fst"             , Enc(FpuFldFst)         , O_FPU(00,00D9,2)          , 0                         , F(Fp)|F(FPU_M4)|F(FPU_M8)             , EF(________), 0 , 0 , kFamilyNone, 0  , 952 , 94 ),
  INST(Fstcw           , "fstcw"           , Enc(X86M_Only)         , O_FPU(9B,00D9,7)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 956 , 91 ),
  INST(Fstenv          , "fstenv"          , Enc(X86M_Only)         , O_FPU(9B,00D9,6)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 962 , 92 ),
  INST(Fstp            , "fstp"            , Enc(FpuFldFst)         , O_FPU(00,00D9,3)          , O(000000,DB,7,_,_,_,_,_  ), F(Fp)|F(FPU_M4)|F(FPU_M8)|F(FPU_M10)  , EF(________), 0 , 0 , kFamilyNone, 0  , 969 , 95 ),
  INST(Fstsw           , "fstsw"           , Enc(FpuStsw)           , O_FPU(9B,00DD,7)          , O_FPU(9B,DFE0,_)          , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 974 , 96 ),
  INST(Fsub            , "fsub"            , Enc(FpuArith)          , O_FPU(00,E0E8,4)          , 0                         , F(Fp)|F(FPU_M4)|F(FPU_M8)             , EF(________), 0 , 0 , kFamilyNone, 0  , 1933, 76 ),
  INST(Fsubp           , "fsubp"           , Enc(FpuRDef)           , O_FPU(00,DEE8,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 980 , 77 ),
  INST(Fsubr           , "fsubr"           , Enc(FpuArith)          , O_FPU(00,E8E0,5)          , 0                         , F(Fp)|F(FPU_M4)|F(FPU_M8)             , EF(________), 0 , 0 , kFamilyNone, 0  , 1939, 76 ),
  INST(Fsubrp          , "fsubrp"          , Enc(FpuRDef)           , O_FPU(00,DEE0,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 986 , 77 ),
  INST(Ftst            , "ftst"            , Enc(FpuOp)             , O_FPU(00,D9E4,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 993 , 75 ),
  INST(Fucom           , "fucom"           , Enc(FpuRDef)           , O_FPU(00,DDE0,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 998 , 77 ),
  INST(Fucomi          , "fucomi"          , Enc(FpuR)              , O_FPU(00,DBE8,_)          , 0                         , F(Fp)                                 , EF(WWWWWW__), 0 , 0 , kFamilyNone, 0  , 1004, 84 ),
  INST(Fucomip         , "fucomip"         , Enc(FpuR)              , O_FPU(00,DFE8,_)          , 0                         , F(Fp)                                 , EF(WWWWWW__), 0 , 0 , kFamilyNone, 0  , 1011, 84 ),
  INST(Fucomp          , "fucomp"          , Enc(FpuRDef)           , O_FPU(00,DDE8,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 1019, 77 ),
  INST(Fucompp         , "fucompp"         , Enc(FpuOp)             , O_FPU(00,DAE9,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 1026, 75 ),
  INST(Fwait           , "fwait"           , Enc(X86Op)             , O_FPU(00,00DB,_)          , 0                         , F(Fp)|F(Volatile)                     , EF(________), 0 , 0 , kFamilyNone, 0  , 1034, 97 ),
  INST(Fxam            , "fxam"            , Enc(FpuOp)             , O_FPU(00,D9E5,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 1040, 75 ),
  INST(Fxch            , "fxch"            , Enc(FpuR)              , O_FPU(00,D9C8,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 1045, 77 ),
  INST(Fxrstor         , "fxrstor"         , Enc(X86M_Only)         , O(000F00,AE,1,_,_,_,_,_  ), 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 1050, 92 ),
  INST(Fxrstor64       , "fxrstor64"       , Enc(X86M_Only)         , O(000F00,AE,1,_,1,_,_,_  ), 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 1058, 98 ),
  INST(Fxsave          , "fxsave"          , Enc(X86M_Only)         , O(000F00,AE,0,_,_,_,_,_  ), 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 1068, 92 ),
  INST(Fxsave64        , "fxsave64"        , Enc(X86M_Only)         , O(000F00,AE,0,_,1,_,_,_  ), 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 1075, 98 ),
  INST(Fxtract         , "fxtract"         , Enc(FpuOp)             , O_FPU(00,D9F4,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 1084, 75 ),
  INST(Fyl2x           , "fyl2x"           , Enc(FpuOp)             , O_FPU(00,D9F1,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 1092, 75 ),
  INST(Fyl2xp1         , "fyl2xp1"         , Enc(FpuOp)             , O_FPU(00,D9F9,_)          , 0                         , F(Fp)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 1098, 75 ),
  INST(Haddpd          , "haddpd"          , Enc(ExtRm)             , O(660F00,7C,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 33 , 5120, 6  ),
  INST(Haddps          , "haddps"          , Enc(ExtRm)             , O(F20F00,7C,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 33 , 5128, 6  ),
  INST(Hsubpd          , "hsubpd"          , Enc(ExtRm)             , O(660F00,7D,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 33 , 5136, 6  ),
  INST(Hsubps          , "hsubps"          , Enc(ExtRm)             , O(F20F00,7D,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 33 , 5144, 6  ),
  INST(Idiv            , "idiv"            , Enc(X86M_GPB_MulDiv)   , O(000000,F6,7,_,x,_,_,_  ), 0                         , F(RW)|F(Special)                      , EF(UUUUUU__), 0 , 0 , kFamilyNone, 0  , 696 , 99 ),
  INST(Imul            , "imul"            , Enc(X86Imul)           , O(000000,F6,5,_,x,_,_,_  ), 0                         , F(RW)|F(Special)                      , EF(WUUUUW__), 0 , 0 , kFamilyNone, 0  , 714 , 100),
  INST(In              , "in"              , Enc(X86In)             , O(000000,EC,_,_,_,_,_,_  ), O(000000,E4,_,_,_,_,_,_  ), F(WO)|F(Volatile)|F(Special)          , EF(________), 0 , 0 , kFamilyNone, 0  , 1851, 101),
  INST(Inc             , "inc"             , Enc(X86IncDec)         , O(000000,FE,0,_,x,_,_,_  ), O(000000,40,_,_,x,_,_,_  ), F(RW)|F(Lock)                         , EF(WWWWW___), 0 , 0 , kFamilyNone, 0  , 1106, 102),
  INST(Ins             , "ins"             , Enc(X86Ins)            , O(000000,6C,_,_,_,_,_,_  ), 0                         , F(WO)|F(Volatile)|F(Special)|F(Rep)   , EF(________), 0 , 0 , kFamilyNone, 0  , 1110, 103),
  INST(Insertps        , "insertps"        , Enc(ExtRmi)            , O(660F3A,21,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 34 , 5280, 45 ),
  INST(Insertq         , "insertq"         , Enc(ExtInsertq)        , O(F20F00,79,_,_,_,_,_,_  ), O(F20F00,78,_,_,_,_,_,_  ), F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 32 , 1114, 104),
  INST(Int             , "int"             , Enc(X86Int)            , O(000000,CD,_,_,_,_,_,_  ), 0                         , F(Volatile)                           , EF(___W___W), 0 , 0 , kFamilyNone, 0  , 909 , 105),
  INST(Int3            , "int3"            , Enc(X86Op)             , O(000000,CC,_,_,_,_,_,_  ), 0                         , F(Volatile)                           , EF(___W___W), 0 , 0 , kFamilyNone, 0  , 1122, 106),
  INST(Into            , "into"            , Enc(X86Op)             , O(000000,CE,_,_,_,_,_,_  ), 0                         , F(Volatile)                           , EF(___W___W), 0 , 0 , kFamilyNone, 0  , 1127, 106),
  INST(Ja              , "ja"              , Enc(X86Jcc)            , O(000F00,87,_,_,_,_,_,_  ), O(000000,77,_,_,_,_,_,_  ), F(Flow)|F(Volatile)                   , EF(__R__R__), 0 , 0 , kFamilyNone, 0  , 1132, 107),
  INST(Jae             , "jae"             , Enc(X86Jcc)            , O(000F00,83,_,_,_,_,_,_  ), O(000000,73,_,_,_,_,_,_  ), F(Flow)|F(Volatile)                   , EF(_____R__), 0 , 0 , kFamilyNone, 0  , 1135, 108),
  INST(Jb              , "jb"              , Enc(X86Jcc)            , O(000F00,82,_,_,_,_,_,_  ), O(000000,72,_,_,_,_,_,_  ), F(Flow)|F(Volatile)                   , EF(_____R__), 0 , 0 , kFamilyNone, 0  , 1139, 109),
  INST(Jbe             , "jbe"             , Enc(X86Jcc)            , O(000F00,86,_,_,_,_,_,_  ), O(000000,76,_,_,_,_,_,_  ), F(Flow)|F(Volatile)                   , EF(__R__R__), 0 , 0 , kFamilyNone, 0  , 1142, 110),
  INST(Jc              , "jc"              , Enc(X86Jcc)            , O(000F00,82,_,_,_,_,_,_  ), O(000000,72,_,_,_,_,_,_  ), F(Flow)|F(Volatile)                   , EF(_____R__), 0 , 0 , kFamilyNone, 0  , 1146, 111),
  INST(Je              , "je"              , Enc(X86Jcc)            , O(000F00,84,_,_,_,_,_,_  ), O(000000,74,_,_,_,_,_,_  ), F(Flow)|F(Volatile)                   , EF(__R_____), 0 , 0 , kFamilyNone, 0  , 1149, 112),
  INST(Jg              , "jg"              , Enc(X86Jcc)            , O(000F00,8F,_,_,_,_,_,_  ), O(000000,7F,_,_,_,_,_,_  ), F(Flow)|F(Volatile)                   , EF(RRR_____), 0 , 0 , kFamilyNone, 0  , 1158, 113),
  INST(Jge             , "jge"             , Enc(X86Jcc)            , O(000F00,8D,_,_,_,_,_,_  ), O(000000,7D,_,_,_,_,_,_  ), F(Flow)|F(Volatile)                   , EF(RR______), 0 , 0 , kFamilyNone, 0  , 1161, 114),
  INST(Jl              , "jl"              , Enc(X86Jcc)            , O(000F00,8C,_,_,_,_,_,_  ), O(000000,7C,_,_,_,_,_,_  ), F(Flow)|F(Volatile)                   , EF(RR______), 0 , 0 , kFamilyNone, 0  , 1165, 115),
  INST(Jle             , "jle"             , Enc(X86Jcc)            , O(000F00,8E,_,_,_,_,_,_  ), O(000000,7E,_,_,_,_,_,_  ), F(Flow)|F(Volatile)                   , EF(RRR_____), 0 , 0 , kFamilyNone, 0  , 1168, 116),
  INST(Jna             , "jna"             , Enc(X86Jcc)            , O(000F00,86,_,_,_,_,_,_  ), O(000000,76,_,_,_,_,_,_  ), F(Flow)|F(Volatile)                   , EF(__R__R__), 0 , 0 , kFamilyNone, 0  , 1176, 110),
  INST(Jnae            , "jnae"            , Enc(X86Jcc)            , O(000F00,82,_,_,_,_,_,_  ), O(000000,72,_,_,_,_,_,_  ), F(Flow)|F(Volatile)                   , EF(_____R__), 0 , 0 , kFamilyNone, 0  , 1180, 109),
  INST(Jnb             , "jnb"             , Enc(X86Jcc)            , O(000F00,83,_,_,_,_,_,_  ), O(000000,73,_,_,_,_,_,_  ), F(Flow)|F(Volatile)                   , EF(_____R__), 0 , 0 , kFamilyNone, 0  , 1185, 108),
  INST(Jnbe            , "jnbe"            , Enc(X86Jcc)            , O(000F00,87,_,_,_,_,_,_  ), O(000000,77,_,_,_,_,_,_  ), F(Flow)|F(Volatile)                   , EF(__R__R__), 0 , 0 , kFamilyNone, 0  , 1189, 107),
  INST(Jnc             , "jnc"             , Enc(X86Jcc)            , O(000F00,83,_,_,_,_,_,_  ), O(000000,73,_,_,_,_,_,_  ), F(Flow)|F(Volatile)                   , EF(_____R__), 0 , 0 , kFamilyNone, 0  , 1194, 117),
  INST(Jne             , "jne"             , Enc(X86Jcc)            , O(000F00,85,_,_,_,_,_,_  ), O(000000,75,_,_,_,_,_,_  ), F(Flow)|F(Volatile)                   , EF(__R_____), 0 , 0 , kFamilyNone, 0  , 1198, 118),
  INST(Jng             , "jng"             , Enc(X86Jcc)            , O(000F00,8E,_,_,_,_,_,_  ), O(000000,7E,_,_,_,_,_,_  ), F(Flow)|F(Volatile)                   , EF(RRR_____), 0 , 0 , kFamilyNone, 0  , 1202, 116),
  INST(Jnge            , "jnge"            , Enc(X86Jcc)            , O(000F00,8C,_,_,_,_,_,_  ), O(000000,7C,_,_,_,_,_,_  ), F(Flow)|F(Volatile)                   , EF(RR______), 0 , 0 , kFamilyNone, 0  , 1206, 115),
  INST(Jnl             , "jnl"             , Enc(X86Jcc)            , O(000F00,8D,_,_,_,_,_,_  ), O(000000,7D,_,_,_,_,_,_  ), F(Flow)|F(Volatile)                   , EF(RR______), 0 , 0 , kFamilyNone, 0  , 1211, 114),
  INST(Jnle            , "jnle"            , Enc(X86Jcc)            , O(000F00,8F,_,_,_,_,_,_  ), O(000000,7F,_,_,_,_,_,_  ), F(Flow)|F(Volatile)                   , EF(RRR_____), 0 , 0 , kFamilyNone, 0  , 1215, 113),
  INST(Jno             , "jno"             , Enc(X86Jcc)            , O(000F00,81,_,_,_,_,_,_  ), O(000000,71,_,_,_,_,_,_  ), F(Flow)|F(Volatile)                   , EF(R_______), 0 , 0 , kFamilyNone, 0  , 1220, 119),
  INST(Jnp             , "jnp"             , Enc(X86Jcc)            , O(000F00,8B,_,_,_,_,_,_  ), O(000000,7B,_,_,_,_,_,_  ), F(Flow)|F(Volatile)                   , EF(____R___), 0 , 0 , kFamilyNone, 0  , 1224, 120),
  INST(Jns             , "jns"             , Enc(X86Jcc)            , O(000F00,89,_,_,_,_,_,_  ), O(000000,79,_,_,_,_,_,_  ), F(Flow)|F(Volatile)                   , EF(_R______), 0 , 0 , kFamilyNone, 0  , 1228, 121),
  INST(Jnz             , "jnz"             , Enc(X86Jcc)            , O(000F00,85,_,_,_,_,_,_  ), O(000000,75,_,_,_,_,_,_  ), F(Flow)|F(Volatile)                   , EF(__R_____), 0 , 0 , kFamilyNone, 0  , 1232, 118),
  INST(Jo              , "jo"              , Enc(X86Jcc)            , O(000F00,80,_,_,_,_,_,_  ), O(000000,70,_,_,_,_,_,_  ), F(Flow)|F(Volatile)                   , EF(R_______), 0 , 0 , kFamilyNone, 0  , 1236, 122),
  INST(Jp              , "jp"              , Enc(X86Jcc)            , O(000F00,8A,_,_,_,_,_,_  ), O(000000,7A,_,_,_,_,_,_  ), F(Flow)|F(Volatile)                   , EF(____R___), 0 , 0 , kFamilyNone, 0  , 1239, 123),
  INST(Jpe             , "jpe"             , Enc(X86Jcc)            , O(000F00,8A,_,_,_,_,_,_  ), O(000000,7A,_,_,_,_,_,_  ), F(Flow)|F(Volatile)                   , EF(____R___), 0 , 0 , kFamilyNone, 0  , 1242, 123),
  INST(Jpo             , "jpo"             , Enc(X86Jcc)            , O(000F00,8B,_,_,_,_,_,_  ), O(000000,7B,_,_,_,_,_,_  ), F(Flow)|F(Volatile)                   , EF(____R___), 0 , 0 , kFamilyNone, 0  , 1246, 120),
  INST(Js              , "js"              , Enc(X86Jcc)            , O(000F00,88,_,_,_,_,_,_  ), O(000000,78,_,_,_,_,_,_  ), F(Flow)|F(Volatile)                   , EF(_R______), 0 , 0 , kFamilyNone, 0  , 1250, 124),
  INST(Jz              , "jz"              , Enc(X86Jcc)            , O(000F00,84,_,_,_,_,_,_  ), O(000000,74,_,_,_,_,_,_  ), F(Flow)|F(Volatile)                   , EF(__R_____), 0 , 0 , kFamilyNone, 0  , 1253, 112),
  INST(Jecxz           , "jecxz"           , Enc(X86JecxzLoop)      , 0                         , O(000000,E3,_,_,_,_,_,_  ), F(Flow)|F(Volatile)|F(Special)        , EF(________), 0 , 0 , kFamilyNone, 0  , 1152, 125),
  INST(Jmp             , "jmp"             , Enc(X86Jmp)            , O(000000,FF,4,_,_,_,_,_  ), O(000000,EB,_,_,_,_,_,_  ), F(Flow)|F(Volatile)                   , EF(________), 0 , 0 , kFamilyNone, 0  , 1172, 126),
  INST(Kaddb           , "kaddb"           , Enc(VexRvm)            , V(660F00,4A,_,1,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1256, 127),
  INST(Kaddd           , "kaddd"           , Enc(VexRvm)            , V(660F00,4A,_,1,1,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1262, 127),
  INST(Kaddq           , "kaddq"           , Enc(VexRvm)            , V(000F00,4A,_,1,1,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1268, 127),
  INST(Kaddw           , "kaddw"           , Enc(VexRvm)            , V(000F00,4A,_,1,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1274, 127),
  INST(Kandb           , "kandb"           , Enc(VexRvm)            , V(660F00,41,_,1,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1280, 127),
  INST(Kandd           , "kandd"           , Enc(VexRvm)            , V(660F00,41,_,1,1,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1286, 127),
  INST(Kandnb          , "kandnb"          , Enc(VexRvm)            , V(660F00,42,_,1,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1292, 127),
  INST(Kandnd          , "kandnd"          , Enc(VexRvm)            , V(660F00,42,_,1,1,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1299, 127),
  INST(Kandnq          , "kandnq"          , Enc(VexRvm)            , V(000F00,42,_,1,1,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1306, 127),
  INST(Kandnw          , "kandnw"          , Enc(VexRvm)            , V(000F00,42,_,1,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1313, 127),
  INST(Kandq           , "kandq"           , Enc(VexRvm)            , V(000F00,41,_,1,1,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1320, 127),
  INST(Kandw           , "kandw"           , Enc(VexRvm)            , V(000F00,41,_,1,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1326, 127),
  INST(Kmovb           , "kmovb"           , Enc(VexKmov)           , V(660F00,90,_,0,0,_,_,_  ), V(660F00,92,_,0,0,_,_,_  ), F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1332, 128),
  INST(Kmovd           , "kmovd"           , Enc(VexKmov)           , V(660F00,90,_,0,1,_,_,_  ), V(F20F00,92,_,0,0,_,_,_  ), F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 7061, 129),
  INST(Kmovq           , "kmovq"           , Enc(VexKmov)           , V(000F00,90,_,0,1,_,_,_  ), V(F20F00,92,_,0,1,_,_,_  ), F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 7072, 130),
  INST(Kmovw           , "kmovw"           , Enc(VexKmov)           , V(000F00,90,_,0,0,_,_,_  ), V(000F00,92,_,0,0,_,_,_  ), F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1338, 131),
  INST(Knotb           , "knotb"           , Enc(VexRm)             , V(660F00,44,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1344, 132),
  INST(Knotd           , "knotd"           , Enc(VexRm)             , V(660F00,44,_,0,1,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1350, 132),
  INST(Knotq           , "knotq"           , Enc(VexRm)             , V(000F00,44,_,0,1,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1356, 132),
  INST(Knotw           , "knotw"           , Enc(VexRm)             , V(000F00,44,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1362, 132),
  INST(Korb            , "korb"            , Enc(VexRvm)            , V(660F00,45,_,1,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1368, 127),
  INST(Kord            , "kord"            , Enc(VexRvm)            , V(660F00,45,_,1,1,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1373, 127),
  INST(Korq            , "korq"            , Enc(VexRvm)            , V(000F00,45,_,1,1,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1378, 127),
  INST(Kortestb        , "kortestb"        , Enc(VexRm)             , V(660F00,98,_,0,0,_,_,_  ), 0                         , F(RO)|F(Vex)                          , EF(WWWWWW__), 0 , 0 , kFamilyNone, 0  , 1383, 133),
  INST(Kortestd        , "kortestd"        , Enc(VexRm)             , V(660F00,98,_,0,1,_,_,_  ), 0                         , F(RO)|F(Vex)                          , EF(WWWWWW__), 0 , 0 , kFamilyNone, 0  , 1392, 133),
  INST(Kortestq        , "kortestq"        , Enc(VexRm)             , V(000F00,98,_,0,1,_,_,_  ), 0                         , F(RO)|F(Vex)                          , EF(WWWWWW__), 0 , 0 , kFamilyNone, 0  , 1401, 133),
  INST(Kortestw        , "kortestw"        , Enc(VexRm)             , V(000F00,98,_,0,0,_,_,_  ), 0                         , F(RO)|F(Vex)                          , EF(WWWWWW__), 0 , 0 , kFamilyNone, 0  , 1410, 133),
  INST(Korw            , "korw"            , Enc(VexRvm)            , V(000F00,45,_,1,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1419, 127),
  INST(Kshiftlb        , "kshiftlb"        , Enc(VexRmi)            , V(660F3A,32,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1424, 134),
  INST(Kshiftld        , "kshiftld"        , Enc(VexRmi)            , V(660F3A,33,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1433, 134),
  INST(Kshiftlq        , "kshiftlq"        , Enc(VexRmi)            , V(660F3A,33,_,0,1,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1442, 134),
  INST(Kshiftlw        , "kshiftlw"        , Enc(VexRmi)            , V(660F3A,32,_,0,1,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1451, 134),
  INST(Kshiftrb        , "kshiftrb"        , Enc(VexRmi)            , V(660F3A,30,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1460, 134),
  INST(Kshiftrd        , "kshiftrd"        , Enc(VexRmi)            , V(660F3A,31,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1469, 134),
  INST(Kshiftrq        , "kshiftrq"        , Enc(VexRmi)            , V(660F3A,31,_,0,1,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1478, 134),
  INST(Kshiftrw        , "kshiftrw"        , Enc(VexRmi)            , V(660F3A,30,_,0,1,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1487, 134),
  INST(Ktestb          , "ktestb"          , Enc(VexRm)             , V(660F00,99,_,0,0,_,_,_  ), 0                         , F(RO)|F(Vex)                          , EF(WWWWWW__), 0 , 0 , kFamilyNone, 0  , 1496, 133),
  INST(Ktestd          , "ktestd"          , Enc(VexRm)             , V(660F00,99,_,0,1,_,_,_  ), 0                         , F(RO)|F(Vex)                          , EF(WWWWWW__), 0 , 0 , kFamilyNone, 0  , 1503, 133),
  INST(Ktestq          , "ktestq"          , Enc(VexRm)             , V(000F00,99,_,0,1,_,_,_  ), 0                         , F(RO)|F(Vex)                          , EF(WWWWWW__), 0 , 0 , kFamilyNone, 0  , 1510, 133),
  INST(Ktestw          , "ktestw"          , Enc(VexRm)             , V(000F00,99,_,0,0,_,_,_  ), 0                         , F(RO)|F(Vex)                          , EF(WWWWWW__), 0 , 0 , kFamilyNone, 0  , 1517, 133),
  INST(Kunpckbw        , "kunpckbw"        , Enc(VexRvm)            , V(660F00,4B,_,1,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1524, 127),
  INST(Kunpckdq        , "kunpckdq"        , Enc(VexRvm)            , V(000F00,4B,_,1,1,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1533, 127),
  INST(Kunpckwd        , "kunpckwd"        , Enc(VexRvm)            , V(000F00,4B,_,1,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1542, 127),
  INST(Kxnorb          , "kxnorb"          , Enc(VexRvm)            , V(660F00,46,_,1,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1551, 127),
  INST(Kxnord          , "kxnord"          , Enc(VexRvm)            , V(660F00,46,_,1,1,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1558, 127),
  INST(Kxnorq          , "kxnorq"          , Enc(VexRvm)            , V(000F00,46,_,1,1,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1565, 127),
  INST(Kxnorw          , "kxnorw"          , Enc(VexRvm)            , V(000F00,46,_,1,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1572, 127),
  INST(Kxorb           , "kxorb"           , Enc(VexRvm)            , V(660F00,47,_,1,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1579, 127),
  INST(Kxord           , "kxord"           , Enc(VexRvm)            , V(660F00,47,_,1,1,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1585, 127),
  INST(Kxorq           , "kxorq"           , Enc(VexRvm)            , V(000F00,47,_,1,1,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1591, 127),
  INST(Kxorw           , "kxorw"           , Enc(VexRvm)            , V(000F00,47,_,1,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyNone, 0  , 1597, 127),
  INST(Lahf            , "lahf"            , Enc(X86Op)             , O(000000,9F,_,_,_,_,_,_  ), 0                         , F(RW)|F(Volatile)|F(Special)          , EF(_RRRRR__), 0 , 0 , kFamilyNone, 0  , 1603, 135),
  INST(Lddqu           , "lddqu"           , Enc(ExtRm)             , O(F20F00,F0,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 35 , 5290, 136),
  INST(Ldmxcsr         , "ldmxcsr"         , Enc(X86M_Only)         , O(000F00,AE,2,_,_,_,_,_  ), 0                         , F(RO)|F(Volatile)                     , EF(________), 0 , 0 , kFamilyNone, 0  , 5297, 137),
  INST(Lea             , "lea"             , Enc(X86Lea)            , O(000000,8D,_,_,x,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 1608, 138),
  INST(Leave           , "leave"           , Enc(X86Op)             , O(000000,C9,_,_,_,_,_,_  ), 0                         , F(Volatile)|F(Special)                , EF(________), 0 , 0 , kFamilyNone, 0  , 1612, 139),
  INST(Lfence          , "lfence"          , Enc(X86Fence)          , O(000F00,AE,5,_,_,_,_,_  ), 0                         , F(Volatile)                           , EF(________), 0 , 0 , kFamilyNone, 0  , 1618, 71 ),
  INST(Lods            , "lods"            , Enc(X86StrRm)          , O(000000,AC,_,_,_,_,_,_  ), 0                         , F(WO)|F(Special)|F(Rep)               , EF(______R_), 0 , 1 , kFamilyNone, 0  , 1625, 140),
  INST(Loop            , "loop"            , Enc(X86JecxzLoop)      , 0                         , O(000000,E2,_,_,_,_,_,_  ), F(RW)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 1630, 141),
  INST(Loope           , "loope"           , Enc(X86JecxzLoop)      , 0                         , O(000000,E1,_,_,_,_,_,_  ), F(RW)                                 , EF(__R_____), 0 , 0 , kFamilyNone, 0  , 1635, 142),
  INST(Loopne          , "loopne"          , Enc(X86JecxzLoop)      , 0                         , O(000000,E0,_,_,_,_,_,_  ), F(RW)                                 , EF(__R_____), 0 , 0 , kFamilyNone, 0  , 1641, 143),
  INST(Lzcnt           , "lzcnt"           , Enc(X86Rm)             , O(F30F00,BD,_,_,x,_,_,_  ), 0                         , F(RW)                                 , EF(UUWUUW__), 0 , 0 , kFamilyNone, 0  , 1648, 144),
  INST(Maskmovdqu      , "maskmovdqu"      , Enc(ExtRm_ZDI)         , O(660F00,57,_,_,_,_,_,_  ), 0                         , F(RO)|F(Special)                      , EF(________), 0 , 0 , kFamilySse , 36 , 5306, 145),
  INST(Maskmovq        , "maskmovq"        , Enc(ExtRm_ZDI)         , O(000F00,F7,_,_,_,_,_,_  ), 0                         , F(RO)|F(Special)                      , EF(________), 0 , 0 , kFamilySse , 18 , 7069, 146),
  INST(Maxpd           , "maxpd"           , Enc(ExtRm)             , O(660F00,5F,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 37 , 5340, 6  ),
  INST(Maxps           , "maxps"           , Enc(ExtRm)             , O(000F00,5F,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 38 , 5347, 6  ),
  INST(Maxsd           , "maxsd"           , Enc(ExtRm)             , O(F20F00,5F,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 37 , 7088, 7  ),
  INST(Maxss           , "maxss"           , Enc(ExtRm)             , O(F30F00,5F,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 38 , 5361, 8  ),
  INST(Mfence          , "mfence"          , Enc(X86Fence)          , O(000F00,AE,6,_,_,_,_,_  ), 0                         , F(RW)|F(Volatile)                     , EF(________), 0 , 0 , kFamilyNone, 0  , 1654, 147),
  INST(Minpd           , "minpd"           , Enc(ExtRm)             , O(660F00,5D,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 39 , 5368, 6  ),
  INST(Minps           , "minps"           , Enc(ExtRm)             , O(000F00,5D,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 40 , 5375, 6  ),
  INST(Minsd           , "minsd"           , Enc(ExtRm)             , O(F20F00,5D,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 39 , 7152, 7  ),
  INST(Minss           , "minss"           , Enc(ExtRm)             , O(F30F00,5D,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 40 , 5389, 8  ),
  INST(Monitor         , "monitor"         , Enc(X86Op)             , O(000F01,C8,_,_,_,_,_,_  ), 0                         , F(RO)|F(Volatile)|F(Special)          , EF(________), 0 , 0 , kFamilyNone, 0  , 1661, 148),
  INST(Mov             , "mov"             , Enc(X86Mov)            , 0                         , 0                         , F(WO)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 6035, 149),
  INST(Movapd          , "movapd"          , Enc(ExtMov)            , O(660F00,28,_,_,_,_,_,_  ), O(660F00,29,_,_,_,_,_,_  ), F(WO)                                 , EF(________), 0 , 16, kFamilySse , 41 , 5396, 150),
  INST(Movaps          , "movaps"          , Enc(ExtMov)            , O(000F00,28,_,_,_,_,_,_  ), O(000F00,29,_,_,_,_,_,_  ), F(WO)                                 , EF(________), 0 , 16, kFamilySse , 42 , 5404, 151),
  INST(Movbe           , "movbe"           , Enc(ExtMovbe)          , O(000F38,F0,_,_,x,_,_,_  ), O(000F38,F1,_,_,x,_,_,_  ), F(WO)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 543 , 152),
  INST(Movd            , "movd"            , Enc(ExtMovd)           , O(000F00,6E,_,_,_,_,_,_  ), O(000F00,7E,_,_,_,_,_,_  ), F(WO)                                 , EF(________), 0 , 16, kFamilySse , 43 , 7062, 153),
  INST(Movddup         , "movddup"         , Enc(ExtMov)            , O(F20F00,12,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 44 , 5418, 54 ),
  INST(Movdq2q         , "movdq2q"         , Enc(ExtMov)            , O(F20F00,D6,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 16 , 1669, 154),
  INST(Movdqa          , "movdqa"          , Enc(ExtMov)            , O(660F00,6F,_,_,_,_,_,_  ), O(660F00,7F,_,_,_,_,_,_  ), F(WO)                                 , EF(________), 0 , 16, kFamilySse , 45 , 5427, 155),
  INST(Movdqu          , "movdqu"          , Enc(ExtMov)            , O(F30F00,6F,_,_,_,_,_,_  ), O(F30F00,7F,_,_,_,_,_,_  ), F(WO)                                 , EF(________), 0 , 16, kFamilySse , 41 , 5310, 156),
  INST(Movhlps         , "movhlps"         , Enc(ExtMov)            , O(000F00,12,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 46 , 5502, 157),
  INST(Movhpd          , "movhpd"          , Enc(ExtMov)            , O(660F00,16,_,_,_,_,_,_  ), O(660F00,17,_,_,_,_,_,_  ), F(RW)                                 , EF(________), 8 , 8 , kFamilySse , 47 , 5511, 158),
  INST(Movhps          , "movhps"          , Enc(ExtMov)            , O(000F00,16,_,_,_,_,_,_  ), O(000F00,17,_,_,_,_,_,_  ), F(RW)                                 , EF(________), 8 , 8 , kFamilySse , 48 , 5519, 159),
  INST(Movlhps         , "movlhps"         , Enc(ExtMov)            , O(000F00,16,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 8 , 8 , kFamilySse , 46 , 5527, 160),
  INST(Movlpd          , "movlpd"          , Enc(ExtMov)            , O(660F00,12,_,_,_,_,_,_  ), O(660F00,13,_,_,_,_,_,_  ), F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 47 , 5536, 161),
  INST(Movlps          , "movlps"          , Enc(ExtMov)            , O(000F00,12,_,_,_,_,_,_  ), O(000F00,13,_,_,_,_,_,_  ), F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 48 , 5544, 162),
  INST(Movmskpd        , "movmskpd"        , Enc(ExtMov)            , O(660F00,50,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 49 , 5552, 163),
  INST(Movmskps        , "movmskps"        , Enc(ExtMov)            , O(000F00,50,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 50 , 5562, 163),
  INST(Movntdq         , "movntdq"         , Enc(ExtMov)            , 0                         , O(660F00,E7,_,_,_,_,_,_  ), F(WO)                                 , EF(________), 0 , 16, kFamilySse , 49 , 5572, 164),
  INST(Movntdqa        , "movntdqa"        , Enc(ExtMov)            , O(660F38,2A,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 51 , 5581, 136),
  INST(Movnti          , "movnti"          , Enc(ExtMovnti)         , O(000F00,C3,_,_,x,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilyNone, 0  , 1677, 165),
  INST(Movntpd         , "movntpd"         , Enc(ExtMov)            , 0                         , O(660F00,2B,_,_,_,_,_,_  ), F(WO)                                 , EF(________), 0 , 16, kFamilySse , 52 , 5591, 166),
  INST(Movntps         , "movntps"         , Enc(ExtMov)            , 0                         , O(000F00,2B,_,_,_,_,_,_  ), F(WO)                                 , EF(________), 0 , 16, kFamilySse , 53 , 5600, 167),
  INST(Movntq          , "movntq"          , Enc(ExtMov)            , 0                         , O(000F00,E7,_,_,_,_,_,_  ), F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 54 , 1684, 168),
  INST(Movntsd         , "movntsd"         , Enc(ExtMov)            , 0                         , O(F20F00,2B,_,_,_,_,_,_  ), F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 32 , 1691, 169),
  INST(Movntss         , "movntss"         , Enc(ExtMov)            , 0                         , O(F30F00,2B,_,_,_,_,_,_  ), F(WO)                                 , EF(________), 0 , 4 , kFamilySse , 32 , 1699, 170),
  INST(Movq            , "movq"            , Enc(ExtMovq)           , O(000F00,6E,_,_,x,_,_,_  ), O(000F00,7E,_,_,x,_,_,_  ), F(WO)                                 , EF(________), 0 , 16, kFamilySse , 55 , 7073, 171),
  INST(Movq2dq         , "movq2dq"         , Enc(ExtRm)             , O(F30F00,D6,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 16 , 1707, 172),
  INST(Movs            , "movs"            , Enc(X86StrMm)          , O(000000,A4,_,_,_,_,_,_  ), 0                         , F(WO)|F(Special)|F(Rep)               , EF(________), 0 , 0 , kFamilyNone, 0  , 351 , 173),
  INST(Movsd           , "movsd"           , Enc(ExtMov)            , O(F20F00,10,_,_,_,_,_,_  ), O(F20F00,11,_,_,_,_,_,_  ), F(WO)|F(ZeroIfMem)                    , EF(________), 0 , 8 , kFamilySse , 56 , 5615, 174),
  INST(Movshdup        , "movshdup"        , Enc(ExtRm)             , O(F30F00,16,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 57 , 5622, 55 ),
  INST(Movsldup        , "movsldup"        , Enc(ExtRm)             , O(F30F00,12,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 57 , 5632, 55 ),
  INST(Movss           , "movss"           , Enc(ExtMov)            , O(F30F00,10,_,_,_,_,_,_  ), O(F30F00,11,_,_,_,_,_,_  ), F(WO)|F(ZeroIfMem)                    , EF(________), 0 , 4 , kFamilySse , 58 , 5642, 175),
  INST(Movsx           , "movsx"           , Enc(X86MovsxMovzx)     , O(000F00,BE,_,_,x,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 1715, 176),
  INST(Movsxd          , "movsxd"          , Enc(X86Rm)             , O(000000,63,_,_,1,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 1721, 177),
  INST(Movupd          , "movupd"          , Enc(ExtMov)            , O(660F00,10,_,_,_,_,_,_  ), O(660F00,11,_,_,_,_,_,_  ), F(WO)                                 , EF(________), 0 , 16, kFamilySse , 59 , 5649, 178),
  INST(Movups          , "movups"          , Enc(ExtMov)            , O(000F00,10,_,_,_,_,_,_  ), O(000F00,11,_,_,_,_,_,_  ), F(WO)                                 , EF(________), 0 , 16, kFamilySse , 60 , 5657, 179),
  INST(Movzx           , "movzx"           , Enc(X86MovsxMovzx)     , O(000F00,B6,_,_,x,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 1728, 176),
  INST(Mpsadbw         , "mpsadbw"         , Enc(ExtRmi)            , O(660F3A,42,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 61 , 5665, 15 ),
  INST(Mul             , "mul"             , Enc(X86M_GPB_MulDiv)   , O(000000,F6,4,_,x,_,_,_  ), 0                         , F(RW)|F(Special)                      , EF(WUUUUW__), 0 , 0 , kFamilyNone, 0  , 715 , 180),
  INST(Mulpd           , "mulpd"           , Enc(ExtRm)             , O(660F00,59,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 62 , 5674, 6  ),
  INST(Mulps           , "mulps"           , Enc(ExtRm)             , O(000F00,59,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 63 , 5681, 6  ),
  INST(Mulsd           , "mulsd"           , Enc(ExtRm)             , O(F20F00,59,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 62 , 5688, 7  ),
  INST(Mulss           , "mulss"           , Enc(ExtRm)             , O(F30F00,59,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 63 , 5695, 8  ),
  INST(Mulx            , "mulx"            , Enc(VexRvm_ZDX_Wx)     , V(F20F38,F6,_,0,x,_,_,_  ), 0                         , F(RW)|F(Special)                      , EF(________), 0 , 0 , kFamilyNone, 0  , 1734, 181),
  INST(Mwait           , "mwait"           , Enc(X86Op)             , O(000F01,C9,_,_,_,_,_,_  ), 0                         , F(RO)|F(Volatile)|F(Special)          , EF(________), 0 , 0 , kFamilyNone, 0  , 1739, 148),
  INST(Neg             , "neg"             , Enc(X86M_GPB)          , O(000000,F6,3,_,x,_,_,_  ), 0                         , F(RW)|F(Lock)                         , EF(WWWWWW__), 0 , 0 , kFamilyNone, 0  , 1745, 182),
  INST(Nop             , "nop"             , Enc(X86Op)             , O(000000,90,_,_,_,_,_,_  ), 0                         , 0                                     , EF(________), 0 , 0 , kFamilyNone, 0  , 846 , 183),
  INST(Not             , "not"             , Enc(X86M_GPB)          , O(000000,F6,2,_,x,_,_,_  ), 0                         , F(RW)|F(Lock)                         , EF(________), 0 , 0 , kFamilyNone, 0  , 1749, 184),
  INST(Or              , "or"              , Enc(X86Arith)          , O(000000,08,1,_,x,_,_,_  ), 0                         , F(RW)|F(Lock)                         , EF(WWWUWW__), 0 , 0 , kFamilyNone, 0  , 1055, 5  ),
  INST(Orpd            , "orpd"            , Enc(ExtRm)             , O(660F00,56,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 64 , 9128, 6  ),
  INST(Orps            , "orps"            , Enc(ExtRm)             , O(000F00,56,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 65 , 9135, 6  ),
  INST(Out             , "out"             , Enc(X86Out)            , O(000000,EE,_,_,_,_,_,_  ), O(000000,E6,_,_,_,_,_,_  ), F(RO)|F(Volatile)|F(Special)          , EF(________), 0 , 0 , kFamilyNone, 0  , 1753, 185),
  INST(Outs            , "outs"            , Enc(X86Outs)           , O(000000,6E,_,_,_,_,_,_  ), 0                         , F(RO)|F(Volatile)|F(Special)|F(Rep)   , EF(________), 0 , 0 , kFamilyNone, 0  , 1757, 186),
  INST(Pabsb           , "pabsb"           , Enc(ExtRm_P)           , O(000F38,1C,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 66 , 5714, 187),
  INST(Pabsd           , "pabsd"           , Enc(ExtRm_P)           , O(000F38,1E,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 66 , 5721, 187),
  INST(Pabsw           , "pabsw"           , Enc(ExtRm_P)           , O(000F38,1D,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 67 , 5735, 187),
  INST(Packssdw        , "packssdw"        , Enc(ExtRm_P)           , O(000F00,6B,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 68 , 5742, 187),
  INST(Packsswb        , "packsswb"        , Enc(ExtRm_P)           , O(000F00,63,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 68 , 5752, 187),
  INST(Packusdw        , "packusdw"        , Enc(ExtRm)             , O(660F38,2B,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 69 , 5762, 6  ),
  INST(Packuswb        , "packuswb"        , Enc(ExtRm_P)           , O(000F00,67,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 68 , 5772, 187),
  INST(Paddb           , "paddb"           , Enc(ExtRm_P)           , O(000F00,FC,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 68 , 5782, 187),
  INST(Paddd           , "paddd"           , Enc(ExtRm_P)           , O(000F00,FE,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 68 , 5789, 187),
  INST(Paddq           , "paddq"           , Enc(ExtRm_P)           , O(000F00,D4,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 70 , 5796, 187),
  INST(Paddsb          , "paddsb"          , Enc(ExtRm_P)           , O(000F00,EC,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 68 , 5803, 187),
  INST(Paddsw          , "paddsw"          , Enc(ExtRm_P)           , O(000F00,ED,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 68 , 5811, 187),
  INST(Paddusb         , "paddusb"         , Enc(ExtRm_P)           , O(000F00,DC,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 68 , 5819, 187),
  INST(Paddusw         , "paddusw"         , Enc(ExtRm_P)           , O(000F00,DD,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 68 , 5828, 187),
  INST(Paddw           , "paddw"           , Enc(ExtRm_P)           , O(000F00,FD,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 68 , 5837, 187),
  INST(Palignr         , "palignr"         , Enc(ExtRmi_P)          , O(000F3A,0F,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 71 , 5844, 188),
  INST(Pand            , "pand"            , Enc(ExtRm_P)           , O(000F00,DB,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 68 , 5853, 187),
  INST(Pandn           , "pandn"           , Enc(ExtRm_P)           , O(000F00,DF,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 72 , 5866, 187),
  INST(Pause           , "pause"           , Enc(X86Op)             , O(F30000,90,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 1762, 189),
  INST(Pavgb           , "pavgb"           , Enc(ExtRm_P)           , O(000F00,E0,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 73 , 5896, 187),
  INST(Pavgusb         , "pavgusb"         , Enc(Ext3dNow)          , O(000F0F,BF,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 74 , 1768, 190),
  INST(Pavgw           , "pavgw"           , Enc(ExtRm_P)           , O(000F00,E3,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 75 , 5903, 187),
  INST(Pblendvb        , "pblendvb"        , Enc(ExtRm_XMM0)        , O(660F38,10,_,_,_,_,_,_  ), 0                         , F(RW)|F(Special)                      , EF(________), 0 , 0 , kFamilySse , 76 , 5919, 16 ),
  INST(Pblendw         , "pblendw"         , Enc(ExtRmi)            , O(660F3A,0E,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 77 , 5929, 15 ),
  INST(Pclmulqdq       , "pclmulqdq"       , Enc(ExtRmi)            , O(660F3A,44,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 78 , 6022, 15 ),
  INST(Pcmpeqb         , "pcmpeqb"         , Enc(ExtRm_P)           , O(000F00,74,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 79 , 6054, 187),
  INST(Pcmpeqd         , "pcmpeqd"         , Enc(ExtRm_P)           , O(000F00,76,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 79 , 6063, 187),
  INST(Pcmpeqq         , "pcmpeqq"         , Enc(ExtRm)             , O(660F38,29,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 80 , 6072, 6  ),
  INST(Pcmpeqw         , "pcmpeqw"         , Enc(ExtRm_P)           , O(000F00,75,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 79 , 6081, 187),
  INST(Pcmpestri       , "pcmpestri"       , Enc(ExtRmi)            , O(660F3A,61,_,_,_,_,_,_  ), 0                         , F(WO)|F(Special)                      , EF(________), 0 , 0 , kFamilySse , 81 , 6090, 191),
  INST(Pcmpestrm       , "pcmpestrm"       , Enc(ExtRmi)            , O(660F3A,60,_,_,_,_,_,_  ), 0                         , F(WO)|F(Special)                      , EF(________), 0 , 0 , kFamilySse , 81 , 6101, 192),
  INST(Pcmpgtb         , "pcmpgtb"         , Enc(ExtRm_P)           , O(000F00,64,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 79 , 6112, 187),
  INST(Pcmpgtd         , "pcmpgtd"         , Enc(ExtRm_P)           , O(000F00,66,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 79 , 6121, 187),
  INST(Pcmpgtq         , "pcmpgtq"         , Enc(ExtRm)             , O(660F38,37,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 80 , 6130, 6  ),
  INST(Pcmpgtw         , "pcmpgtw"         , Enc(ExtRm_P)           , O(000F00,65,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 79 , 6139, 187),
  INST(Pcmpistri       , "pcmpistri"       , Enc(ExtRmi)            , O(660F3A,63,_,_,_,_,_,_  ), 0                         , F(WO)|F(Special)                      , EF(________), 0 , 0 , kFamilySse , 81 , 6148, 193),
  INST(Pcmpistrm       , "pcmpistrm"       , Enc(ExtRmi)            , O(660F3A,62,_,_,_,_,_,_  ), 0                         , F(WO)|F(Special)                      , EF(________), 0 , 0 , kFamilySse , 81 , 6159, 194),
  INST(Pcommit         , "pcommit"         , Enc(X86Op_O)           , O(660F00,AE,7,_,_,_,_,_  ), 0                         , F(Volatile)                           , EF(________), 0 , 0 , kFamilyNone, 0  , 1776, 71 ),
  INST(Pdep            , "pdep"            , Enc(VexRvm_Wx)         , V(F20F38,F5,_,0,x,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 1784, 195),
  INST(Pext            , "pext"            , Enc(VexRvm_Wx)         , V(F30F38,F5,_,0,x,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 1789, 195),
  INST(Pextrb          , "pextrb"          , Enc(ExtExtract)        , O(000F3A,14,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 82 , 6564, 196),
  INST(Pextrd          , "pextrd"          , Enc(ExtExtract)        , O(000F3A,16,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 82 , 6572, 73 ),
  INST(Pextrq          , "pextrq"          , Enc(ExtExtract)        , O(000F3A,16,_,_,1,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 82 , 6580, 197),
  INST(Pextrw          , "pextrw"          , Enc(ExtPextrw)         , O(000F00,C5,_,_,_,_,_,_  ), O(000F3A,15,_,_,_,_,_,_  ), F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 83 , 6588, 198),
  INST(Pf2id           , "pf2id"           , Enc(Ext3dNow)          , O(000F0F,1D,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 74 , 1794, 199),
  INST(Pf2iw           , "pf2iw"           , Enc(Ext3dNow)          , O(000F0F,1C,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 84 , 1800, 199),
  INST(Pfacc           , "pfacc"           , Enc(Ext3dNow)          , O(000F0F,AE,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 74 , 1806, 190),
  INST(Pfadd           , "pfadd"           , Enc(Ext3dNow)          , O(000F0F,9E,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 74 , 1812, 190),
  INST(Pfcmpeq         , "pfcmpeq"         , Enc(Ext3dNow)          , O(000F0F,B0,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 74 , 1818, 190),
  INST(Pfcmpge         , "pfcmpge"         , Enc(Ext3dNow)          , O(000F0F,90,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 74 , 1826, 190),
  INST(Pfcmpgt         , "pfcmpgt"         , Enc(Ext3dNow)          , O(000F0F,A0,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 74 , 1834, 190),
  INST(Pfmax           , "pfmax"           , Enc(Ext3dNow)          , O(000F0F,A4,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 74 , 1842, 190),
  INST(Pfmin           , "pfmin"           , Enc(Ext3dNow)          , O(000F0F,94,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 74 , 1848, 190),
  INST(Pfmul           , "pfmul"           , Enc(Ext3dNow)          , O(000F0F,B4,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 74 , 1854, 190),
  INST(Pfnacc          , "pfnacc"          , Enc(Ext3dNow)          , O(000F0F,8A,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 84 , 1860, 190),
  INST(Pfpnacc         , "pfpnacc"         , Enc(Ext3dNow)          , O(000F0F,8E,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 84 , 1867, 190),
  INST(Pfrcp           , "pfrcp"           , Enc(Ext3dNow)          , O(000F0F,96,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 74 , 1875, 199),
  INST(Pfrcpit1        , "pfrcpit1"        , Enc(Ext3dNow)          , O(000F0F,A6,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 74 , 1881, 190),
  INST(Pfrcpit2        , "pfrcpit2"        , Enc(Ext3dNow)          , O(000F0F,B6,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 74 , 1890, 190),
  INST(Pfrcpv          , "pfrcpv"          , Enc(Ext3dNow)          , O(000F0F,86,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 85 , 1899, 190),
  INST(Pfrsqit1        , "pfrsqit1"        , Enc(Ext3dNow)          , O(000F0F,A7,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 0 , kFamilySse , 74 , 1906, 200),
  INST(Pfrsqrt         , "pfrsqrt"         , Enc(Ext3dNow)          , O(000F0F,97,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 0 , kFamilySse , 74 , 1915, 200),
  INST(Pfrsqrtv        , "pfrsqrtv"        , Enc(Ext3dNow)          , O(000F0F,87,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 85 , 1923, 190),
  INST(Pfsub           , "pfsub"           , Enc(Ext3dNow)          , O(000F0F,9A,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 74 , 1932, 190),
  INST(Pfsubr          , "pfsubr"          , Enc(Ext3dNow)          , O(000F0F,AA,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 74 , 1938, 190),
  INST(Phaddd          , "phaddd"          , Enc(ExtRm_P)           , O(000F38,02,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 86 , 6667, 187),
  INST(Phaddsw         , "phaddsw"         , Enc(ExtRm_P)           , O(000F38,03,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 87 , 6684, 187),
  INST(Phaddw          , "phaddw"          , Enc(ExtRm_P)           , O(000F38,01,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 88 , 6753, 187),
  INST(Phminposuw      , "phminposuw"      , Enc(ExtRm)             , O(660F38,41,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 89 , 6779, 6  ),
  INST(Phsubd          , "phsubd"          , Enc(ExtRm_P)           , O(000F38,06,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 90 , 6800, 187),
  INST(Phsubsw         , "phsubsw"         , Enc(ExtRm_P)           , O(000F38,07,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 91 , 6817, 187),
  INST(Phsubw          , "phsubw"          , Enc(ExtRm_P)           , O(000F38,05,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 91 , 6826, 187),
  INST(Pi2fd           , "pi2fd"           , Enc(Ext3dNow)          , O(000F0F,0D,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 74 , 1945, 199),
  INST(Pi2fw           , "pi2fw"           , Enc(Ext3dNow)          , O(000F0F,0C,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 84 , 1951, 199),
  INST(Pinsrb          , "pinsrb"          , Enc(ExtRmi)            , O(660F3A,20,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 92 , 6843, 201),
  INST(Pinsrd          , "pinsrd"          , Enc(ExtRmi)            , O(660F3A,22,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 92 , 6851, 202),
  INST(Pinsrq          , "pinsrq"          , Enc(ExtRmi)            , O(660F3A,22,_,_,1,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 92 , 6859, 203),
  INST(Pinsrw          , "pinsrw"          , Enc(ExtRmi_P)          , O(000F00,C4,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 93 , 6867, 204),
  INST(Pmaddubsw       , "pmaddubsw"       , Enc(ExtRm_P)           , O(000F38,04,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 94 , 7037, 187),
  INST(Pmaddwd         , "pmaddwd"         , Enc(ExtRm_P)           , O(000F00,F5,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 95 , 7048, 187),
  INST(Pmaxsb          , "pmaxsb"          , Enc(ExtRm)             , O(660F38,3C,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 96 , 7079, 6  ),
  INST(Pmaxsd          , "pmaxsd"          , Enc(ExtRm)             , O(660F38,3D,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 96 , 7087, 6  ),
  INST(Pmaxsw          , "pmaxsw"          , Enc(ExtRm_P)           , O(000F00,EE,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 97 , 7103, 187),
  INST(Pmaxub          , "pmaxub"          , Enc(ExtRm_P)           , O(000F00,DE,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 97 , 7111, 187),
  INST(Pmaxud          , "pmaxud"          , Enc(ExtRm)             , O(660F38,3F,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 98 , 7119, 6  ),
  INST(Pmaxuw          , "pmaxuw"          , Enc(ExtRm)             , O(660F38,3E,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 99 , 7135, 6  ),
  INST(Pminsb          , "pminsb"          , Enc(ExtRm)             , O(660F38,38,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 99 , 7143, 6  ),
  INST(Pminsd          , "pminsd"          , Enc(ExtRm)             , O(660F38,39,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 99 , 7151, 6  ),
  INST(Pminsw          , "pminsw"          , Enc(ExtRm_P)           , O(000F00,EA,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 100, 7167, 187),
  INST(Pminub          , "pminub"          , Enc(ExtRm_P)           , O(000F00,DA,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 100, 7175, 187),
  INST(Pminud          , "pminud"          , Enc(ExtRm)             , O(660F38,3B,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 101, 7183, 6  ),
  INST(Pminuw          , "pminuw"          , Enc(ExtRm)             , O(660F38,3A,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 102, 7199, 6  ),
  INST(Pmovmskb        , "pmovmskb"        , Enc(ExtRm_P)           , O(000F00,D7,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 103, 7277, 205),
  INST(Pmovsxbd        , "pmovsxbd"        , Enc(ExtRm)             , O(660F38,21,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 104, 7374, 206),
  INST(Pmovsxbq        , "pmovsxbq"        , Enc(ExtRm)             , O(660F38,22,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 104, 7384, 207),
  INST(Pmovsxbw        , "pmovsxbw"        , Enc(ExtRm)             , O(660F38,20,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 104, 7394, 54 ),
  INST(Pmovsxdq        , "pmovsxdq"        , Enc(ExtRm)             , O(660F38,25,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 104, 7404, 54 ),
  INST(Pmovsxwd        , "pmovsxwd"        , Enc(ExtRm)             , O(660F38,23,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 104, 7414, 54 ),
  INST(Pmovsxwq        , "pmovsxwq"        , Enc(ExtRm)             , O(660F38,24,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 104, 7424, 206),
  INST(Pmovzxbd        , "pmovzxbd"        , Enc(ExtRm)             , O(660F38,31,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 105, 7511, 206),
  INST(Pmovzxbq        , "pmovzxbq"        , Enc(ExtRm)             , O(660F38,32,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 105, 7521, 207),
  INST(Pmovzxbw        , "pmovzxbw"        , Enc(ExtRm)             , O(660F38,30,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 105, 7531, 54 ),
  INST(Pmovzxdq        , "pmovzxdq"        , Enc(ExtRm)             , O(660F38,35,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 105, 7541, 54 ),
  INST(Pmovzxwd        , "pmovzxwd"        , Enc(ExtRm)             , O(660F38,33,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 105, 7551, 54 ),
  INST(Pmovzxwq        , "pmovzxwq"        , Enc(ExtRm)             , O(660F38,34,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 105, 7561, 206),
  INST(Pmuldq          , "pmuldq"          , Enc(ExtRm)             , O(660F38,28,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 106, 7571, 6  ),
  INST(Pmulhrsw        , "pmulhrsw"        , Enc(ExtRm_P)           , O(000F38,0B,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 107, 7579, 187),
  INST(Pmulhrw         , "pmulhrw"         , Enc(Ext3dNow)          , O(000F0F,B7,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 74 , 1957, 190),
  INST(Pmulhuw         , "pmulhuw"         , Enc(ExtRm_P)           , O(000F00,E4,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 108, 7589, 187),
  INST(Pmulhw          , "pmulhw"          , Enc(ExtRm_P)           , O(000F00,E5,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 109, 7598, 187),
  INST(Pmulld          , "pmulld"          , Enc(ExtRm)             , O(660F38,40,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 110, 7606, 6  ),
  INST(Pmullw          , "pmullw"          , Enc(ExtRm_P)           , O(000F00,D5,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 111, 7622, 187),
  INST(Pmuludq         , "pmuludq"         , Enc(ExtRm_P)           , O(000F00,F4,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 112, 7645, 187),
  INST(Pop             , "pop"             , Enc(X86Pop)            , O(000000,8F,0,_,_,_,_,_  ), O(000000,58,_,_,_,_,_,_  ), F(WO)|F(Volatile)|F(Special)          , EF(________), 0 , 0 , kFamilyNone, 0  , 1965, 208),
  INST(Popa            , "popa"            , Enc(X86Op)             , O(660000,61,_,_,_,_,_,_  ), 0                         , F(Volatile)|F(Special)                , EF(________), 0 , 0 , kFamilyNone, 0  , 1969, 209),
  INST(Popad           , "popad"           , Enc(X86Op)             , O(000000,61,_,_,_,_,_,_  ), 0                         , F(Volatile)|F(Special)                , EF(________), 0 , 0 , kFamilyNone, 0  , 1974, 209),
  INST(Popcnt          , "popcnt"          , Enc(X86Rm)             , O(F30F00,B8,_,_,x,_,_,_  ), 0                         , F(WO)                                 , EF(WWWWWW__), 0 , 0 , kFamilyNone, 0  , 1980, 210),
  INST(Popf            , "popf"            , Enc(X86Op)             , O(660000,9D,_,_,_,_,_,_  ), 0                         , F(Volatile)|F(Special)                , EF(WWWWWWWW), 0 , 0 , kFamilyNone, 0  , 1987, 211),
  INST(Popfd           , "popfd"           , Enc(X86Op)             , O(000000,9D,_,_,_,_,_,_  ), 0                         , F(Volatile)|F(Special)                , EF(WWWWWWWW), 0 , 0 , kFamilyNone, 0  , 1992, 212),
  INST(Popfq           , "popfq"           , Enc(X86Op)             , O(000000,9D,_,_,_,_,_,_  ), 0                         , F(Volatile)|F(Special)                , EF(WWWWWWWW), 0 , 0 , kFamilyNone, 0  , 1998, 213),
  INST(Por             , "por"             , Enc(ExtRm_P)           , O(000F00,EB,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 113, 7654, 187),
  INST(Prefetch        , "prefetch"        , Enc(X86M_Only)         , O(000F00,0D,0,_,_,_,_,_  ), 0                         , F(RO)|F(Volatile)                     , EF(________), 0 , 0 , kFamilyNone, 0  , 2004, 31 ),
  INST(Prefetchnta     , "prefetchnta"     , Enc(X86M_Only)         , O(000F00,18,0,_,_,_,_,_  ), 0                         , F(RO)|F(Volatile)                     , EF(________), 0 , 0 , kFamilyNone, 0  , 2013, 31 ),
  INST(Prefetcht0      , "prefetcht0"      , Enc(X86M_Only)         , O(000F00,18,1,_,_,_,_,_  ), 0                         , F(RO)|F(Volatile)                     , EF(________), 0 , 0 , kFamilyNone, 0  , 2025, 31 ),
  INST(Prefetcht1      , "prefetcht1"      , Enc(X86M_Only)         , O(000F00,18,2,_,_,_,_,_  ), 0                         , F(RO)|F(Volatile)                     , EF(________), 0 , 0 , kFamilyNone, 0  , 2036, 31 ),
  INST(Prefetcht2      , "prefetcht2"      , Enc(X86M_Only)         , O(000F00,18,3,_,_,_,_,_  ), 0                         , F(RO)|F(Volatile)                     , EF(________), 0 , 0 , kFamilyNone, 0  , 2047, 31 ),
  INST(Prefetchw       , "prefetchw"       , Enc(X86M_Only)         , O(000F00,0D,1,_,_,_,_,_  ), 0                         , F(RO)|F(Volatile)                     , EF(UUUUUU__), 0 , 0 , kFamilyNone, 0  , 2058, 214),
  INST(Prefetchwt1     , "prefetchwt1"     , Enc(X86M_Only)         , O(000F00,0D,2,_,_,_,_,_  ), 0                         , F(RO)|F(Volatile)                     , EF(UUUUUU__), 0 , 0 , kFamilyNone, 0  , 2068, 214),
  INST(Psadbw          , "psadbw"          , Enc(ExtRm_P)           , O(000F00,F6,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 114, 3543, 187),
  INST(Pshufb          , "pshufb"          , Enc(ExtRm_P)           , O(000F38,00,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 115, 7878, 215),
  INST(Pshufd          , "pshufd"          , Enc(ExtRmi)            , O(660F00,70,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 116, 7886, 216),
  INST(Pshufhw         , "pshufhw"         , Enc(ExtRmi)            , O(F30F00,70,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 116, 7894, 216),
  INST(Pshuflw         , "pshuflw"         , Enc(ExtRmi)            , O(F20F00,70,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 116, 7903, 216),
  INST(Pshufw          , "pshufw"          , Enc(ExtRmi_P)          , O(000F00,70,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 54 , 2080, 217),
  INST(Psignb          , "psignb"          , Enc(ExtRm_P)           , O(000F38,08,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 117, 7912, 187),
  INST(Psignd          , "psignd"          , Enc(ExtRm_P)           , O(000F38,0A,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 117, 7920, 187),
  INST(Psignw          , "psignw"          , Enc(ExtRm_P)           , O(000F38,09,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 117, 7928, 187),
  INST(Pslld           , "pslld"           , Enc(ExtRmRi_P)         , O(000F00,F2,_,_,_,_,_,_  ), O(000F00,72,6,_,_,_,_,_  ), F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 118, 7936, 218),
  INST(Pslldq          , "pslldq"          , Enc(ExtRmRi)           , 0                         , O(660F00,73,7,_,_,_,_,_  ), F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 119, 7943, 219),
  INST(Psllq           , "psllq"           , Enc(ExtRmRi_P)         , O(000F00,F3,_,_,_,_,_,_  ), O(000F00,73,6,_,_,_,_,_  ), F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 118, 7951, 220),
  INST(Psllw           , "psllw"           , Enc(ExtRmRi_P)         , O(000F00,F1,_,_,_,_,_,_  ), O(000F00,71,6,_,_,_,_,_  ), F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 120, 7982, 221),
  INST(Psrad           , "psrad"           , Enc(ExtRmRi_P)         , O(000F00,E2,_,_,_,_,_,_  ), O(000F00,72,4,_,_,_,_,_  ), F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 120, 7989, 222),
  INST(Psraw           , "psraw"           , Enc(ExtRmRi_P)         , O(000F00,E1,_,_,_,_,_,_  ), O(000F00,71,4,_,_,_,_,_  ), F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 121, 8027, 223),
  INST(Psrld           , "psrld"           , Enc(ExtRmRi_P)         , O(000F00,D2,_,_,_,_,_,_  ), O(000F00,72,2,_,_,_,_,_  ), F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 121, 8034, 224),
  INST(Psrldq          , "psrldq"          , Enc(ExtRmRi)           , 0                         , O(660F00,73,3,_,_,_,_,_  ), F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 122, 8041, 225),
  INST(Psrlq           , "psrlq"           , Enc(ExtRmRi_P)         , O(000F00,D3,_,_,_,_,_,_  ), O(000F00,73,2,_,_,_,_,_  ), F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 121, 8049, 226),
  INST(Psrlw           , "psrlw"           , Enc(ExtRmRi_P)         , O(000F00,D1,_,_,_,_,_,_  ), O(000F00,71,2,_,_,_,_,_  ), F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 123, 8080, 227),
  INST(Psubb           , "psubb"           , Enc(ExtRm_P)           , O(000F00,F8,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 123, 8087, 187),
  INST(Psubd           , "psubd"           , Enc(ExtRm_P)           , O(000F00,FA,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 123, 8094, 187),
  INST(Psubq           , "psubq"           , Enc(ExtRm_P)           , O(000F00,FB,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 124, 8101, 187),
  INST(Psubsb          , "psubsb"          , Enc(ExtRm_P)           , O(000F00,E8,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 123, 8108, 187),
  INST(Psubsw          , "psubsw"          , Enc(ExtRm_P)           , O(000F00,E9,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 123, 8116, 187),
  INST(Psubusb         , "psubusb"         , Enc(ExtRm_P)           , O(000F00,D8,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 123, 8124, 187),
  INST(Psubusw         , "psubusw"         , Enc(ExtRm_P)           , O(000F00,D9,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 123, 8133, 187),
  INST(Psubw           , "psubw"           , Enc(ExtRm_P)           , O(000F00,F9,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 123, 8142, 187),
  INST(Pswapd          , "pswapd"          , Enc(Ext3dNow)          , O(000F0F,BB,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 84 , 2087, 199),
  INST(Ptest           , "ptest"           , Enc(ExtRm)             , O(660F38,17,_,_,_,_,_,_  ), 0                         , F(RO)                                 , EF(WWWWWW__), 0 , 0 , kFamilySse , 125, 8171, 228),
  INST(Punpckhbw       , "punpckhbw"       , Enc(ExtRm_P)           , O(000F00,68,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 126, 8254, 187),
  INST(Punpckhdq       , "punpckhdq"       , Enc(ExtRm_P)           , O(000F00,6A,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 126, 8265, 187),
  INST(Punpckhqdq      , "punpckhqdq"      , Enc(ExtRm)             , O(660F00,6D,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 127, 8276, 6  ),
  INST(Punpckhwd       , "punpckhwd"       , Enc(ExtRm_P)           , O(000F00,69,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 126, 8288, 187),
  INST(Punpcklbw       , "punpcklbw"       , Enc(ExtRm_P)           , O(000F00,60,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 126, 8299, 187),
  INST(Punpckldq       , "punpckldq"       , Enc(ExtRm_P)           , O(000F00,62,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 126, 8310, 187),
  INST(Punpcklqdq      , "punpcklqdq"      , Enc(ExtRm)             , O(660F00,6C,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 127, 8321, 6  ),
  INST(Punpcklwd       , "punpcklwd"       , Enc(ExtRm_P)           , O(000F00,61,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 126, 8333, 187),
  INST(Push            , "push"            , Enc(X86Push)           , O(000000,FF,6,_,_,_,_,_  ), O(000000,50,_,_,_,_,_,_  ), F(RO)|F(Volatile)|F(Special)          , EF(________), 0 , 0 , kFamilyNone, 0  , 2094, 229),
  INST(Pusha           , "pusha"           , Enc(X86Op)             , O(660000,60,_,_,_,_,_,_  ), 0                         , F(Volatile)|F(Special)                , EF(________), 0 , 0 , kFamilyNone, 0  , 2099, 209),
  INST(Pushad          , "pushad"          , Enc(X86Op)             , O(000000,60,_,_,_,_,_,_  ), 0                         , F(Volatile)|F(Special)                , EF(________), 0 , 0 , kFamilyNone, 0  , 2105, 209),
  INST(Pushf           , "pushf"           , Enc(X86Op)             , O(660000,9C,_,_,_,_,_,_  ), 0                         , F(Volatile)|F(Special)                , EF(RRRRRRRR), 0 , 0 , kFamilyNone, 0  , 2112, 230),
  INST(Pushfd          , "pushfd"          , Enc(X86Op)             , O(000000,9C,_,_,_,_,_,_  ), 0                         , F(Volatile)|F(Special)                , EF(RRRRRRRR), 0 , 0 , kFamilyNone, 0  , 2118, 231),
  INST(Pushfq          , "pushfq"          , Enc(X86Op)             , O(000000,9C,_,_,_,_,_,_  ), 0                         , F(Volatile)|F(Special)                , EF(RRRRRRRR), 0 , 0 , kFamilyNone, 0  , 2125, 232),
  INST(Pxor            , "pxor"            , Enc(ExtRm_P)           , O(000F00,EF,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 128, 8344, 187),
  INST(Rcl             , "rcl"             , Enc(X86Rot)            , O(000000,D0,2,_,x,_,_,_  ), 0                         , F(RW)|F(Special)                      , EF(W____X__), 0 , 0 , kFamilyNone, 0  , 2132, 233),
  INST(Rcpps           , "rcpps"           , Enc(ExtRm)             , O(000F00,53,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 129, 8472, 55 ),
  INST(Rcpss           , "rcpss"           , Enc(ExtRm)             , O(F30F00,53,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 4 , kFamilySse , 130, 8479, 234),
  INST(Rcr             , "rcr"             , Enc(X86Rot)            , O(000000,D0,3,_,x,_,_,_  ), 0                         , F(RW)|F(Special)                      , EF(W____X__), 0 , 0 , kFamilyNone, 0  , 2136, 233),
  INST(Rdfsbase        , "rdfsbase"        , Enc(X86M)              , O(F30F00,AE,0,_,x,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilyNone, 0  , 2140, 235),
  INST(Rdgsbase        , "rdgsbase"        , Enc(X86M)              , O(F30F00,AE,1,_,x,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilyNone, 0  , 2149, 235),
  INST(Rdrand          , "rdrand"          , Enc(X86M)              , O(000F00,C7,6,_,x,_,_,_  ), 0                         , F(WO)                                 , EF(WWWWWW__), 0 , 8 , kFamilyNone, 0  , 2158, 236),
  INST(Rdseed          , "rdseed"          , Enc(X86M)              , O(000F00,C7,7,_,x,_,_,_  ), 0                         , F(WO)                                 , EF(WWWWWW__), 0 , 8 , kFamilyNone, 0  , 2165, 236),
  INST(Rdtsc           , "rdtsc"           , Enc(X86Op)             , O(000F00,31,_,_,_,_,_,_  ), 0                         , F(WO)|F(Volatile)|F(Special)          , EF(________), 0 , 0 , kFamilyNone, 0  , 2172, 237),
  INST(Rdtscp          , "rdtscp"          , Enc(X86Op)             , O(000F01,F9,_,_,_,_,_,_  ), 0                         , F(WO)|F(Volatile)|F(Special)          , EF(________), 0 , 0 , kFamilyNone, 0  , 2178, 238),
  INST(Ret             , "ret"             , Enc(X86Ret)            , O(000000,C2,_,_,_,_,_,_  ), 0                         , F(RW)|F(Volatile)|F(Special)          , EF(________), 0 , 0 , kFamilyNone, 0  , 2185, 239),
  INST(Rol             , "rol"             , Enc(X86Rot)            , O(000000,D0,0,_,x,_,_,_  ), 0                         , F(RW)|F(Special)                      , EF(W____W__), 0 , 0 , kFamilyNone, 0  , 2189, 240),
  INST(Ror             , "ror"             , Enc(X86Rot)            , O(000000,D0,1,_,x,_,_,_  ), 0                         , F(RW)|F(Special)                      , EF(W____W__), 0 , 0 , kFamilyNone, 0  , 2193, 240),
  INST(Rorx            , "rorx"            , Enc(VexRmi_Wx)         , V(F20F3A,F0,_,0,x,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 2197, 241),
  INST(Roundpd         , "roundpd"         , Enc(ExtRmi)            , O(660F3A,09,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 131, 8574, 216),
  INST(Roundps         , "roundps"         , Enc(ExtRmi)            , O(660F3A,08,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 131, 8583, 216),
  INST(Roundsd         , "roundsd"         , Enc(ExtRmi)            , O(660F3A,0B,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 132, 8592, 242),
  INST(Roundss         , "roundss"         , Enc(ExtRmi)            , O(660F3A,0A,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 4 , kFamilySse , 132, 8601, 243),
  INST(Rsqrtps         , "rsqrtps"         , Enc(ExtRm)             , O(000F00,52,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 133, 8698, 55 ),
  INST(Rsqrtss         , "rsqrtss"         , Enc(ExtRm)             , O(F30F00,52,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 4 , kFamilySse , 134, 8707, 234),
  INST(Sahf            , "sahf"            , Enc(X86Op)             , O(000000,9E,_,_,_,_,_,_  ), 0                         , F(RO)|F(Volatile)|F(Special)          , EF(_WWWWW__), 0 , 0 , kFamilyNone, 0  , 2202, 244),
  INST(Sal             , "sal"             , Enc(X86Rot)            , O(000000,D0,4,_,x,_,_,_  ), 0                         , F(RW)|F(Special)                      , EF(WWWUWW__), 0 , 0 , kFamilyNone, 0  , 2207, 245),
  INST(Sar             , "sar"             , Enc(X86Rot)            , O(000000,D0,7,_,x,_,_,_  ), 0                         , F(RW)|F(Special)                      , EF(WWWUWW__), 0 , 0 , kFamilyNone, 0  , 2211, 245),
  INST(Sarx            , "sarx"            , Enc(VexRmv_Wx)         , V(F30F38,F7,_,0,x,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 2215, 246),
  INST(Sbb             , "sbb"             , Enc(X86Arith)          , O(000000,18,3,_,x,_,_,_  ), 0                         , F(RW)|F(Lock)                         , EF(WWWWWX__), 0 , 0 , kFamilyNone, 0  , 2220, 3  ),
  INST(Scas            , "scas"            , Enc(X86StrRm)          , O(000000,AE,_,_,_,_,_,_  ), 0                         , F(RW)|F(Special)|F(Rep)|F(Repnz)      , EF(WWWWWWR_), 0 , 0 , kFamilyNone, 0  , 2224, 247),
  INST(Seta            , "seta"            , Enc(X86Set)            , O(000F00,97,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(__R__R__), 0 , 1 , kFamilyNone, 0  , 2229, 248),
  INST(Setae           , "setae"           , Enc(X86Set)            , O(000F00,93,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(_____R__), 0 , 1 , kFamilyNone, 0  , 2234, 249),
  INST(Setb            , "setb"            , Enc(X86Set)            , O(000F00,92,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(_____R__), 0 , 1 , kFamilyNone, 0  , 2240, 249),
  INST(Setbe           , "setbe"           , Enc(X86Set)            , O(000F00,96,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(__R__R__), 0 , 1 , kFamilyNone, 0  , 2245, 248),
  INST(Setc            , "setc"            , Enc(X86Set)            , O(000F00,92,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(_____R__), 0 , 1 , kFamilyNone, 0  , 2251, 249),
  INST(Sete            , "sete"            , Enc(X86Set)            , O(000F00,94,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(__R_____), 0 , 1 , kFamilyNone, 0  , 2256, 250),
  INST(Setg            , "setg"            , Enc(X86Set)            , O(000F00,9F,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(RRR_____), 0 , 1 , kFamilyNone, 0  , 2261, 251),
  INST(Setge           , "setge"           , Enc(X86Set)            , O(000F00,9D,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(RR______), 0 , 1 , kFamilyNone, 0  , 2266, 252),
  INST(Setl            , "setl"            , Enc(X86Set)            , O(000F00,9C,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(RR______), 0 , 1 , kFamilyNone, 0  , 2272, 252),
  INST(Setle           , "setle"           , Enc(X86Set)            , O(000F00,9E,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(RRR_____), 0 , 1 , kFamilyNone, 0  , 2277, 251),
  INST(Setna           , "setna"           , Enc(X86Set)            , O(000F00,96,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(__R__R__), 0 , 1 , kFamilyNone, 0  , 2283, 248),
  INST(Setnae          , "setnae"          , Enc(X86Set)            , O(000F00,92,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(_____R__), 0 , 1 , kFamilyNone, 0  , 2289, 249),
  INST(Setnb           , "setnb"           , Enc(X86Set)            , O(000F00,93,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(_____R__), 0 , 1 , kFamilyNone, 0  , 2296, 249),
  INST(Setnbe          , "setnbe"          , Enc(X86Set)            , O(000F00,97,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(__R__R__), 0 , 1 , kFamilyNone, 0  , 2302, 248),
  INST(Setnc           , "setnc"           , Enc(X86Set)            , O(000F00,93,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(_____R__), 0 , 1 , kFamilyNone, 0  , 2309, 249),
  INST(Setne           , "setne"           , Enc(X86Set)            , O(000F00,95,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(__R_____), 0 , 1 , kFamilyNone, 0  , 2315, 250),
  INST(Setng           , "setng"           , Enc(X86Set)            , O(000F00,9E,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(RRR_____), 0 , 1 , kFamilyNone, 0  , 2321, 251),
  INST(Setnge          , "setnge"          , Enc(X86Set)            , O(000F00,9C,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(RR______), 0 , 1 , kFamilyNone, 0  , 2327, 252),
  INST(Setnl           , "setnl"           , Enc(X86Set)            , O(000F00,9D,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(RR______), 0 , 1 , kFamilyNone, 0  , 2334, 252),
  INST(Setnle          , "setnle"          , Enc(X86Set)            , O(000F00,9F,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(RRR_____), 0 , 1 , kFamilyNone, 0  , 2340, 251),
  INST(Setno           , "setno"           , Enc(X86Set)            , O(000F00,91,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(R_______), 0 , 1 , kFamilyNone, 0  , 2347, 253),
  INST(Setnp           , "setnp"           , Enc(X86Set)            , O(000F00,9B,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(____R___), 0 , 1 , kFamilyNone, 0  , 2353, 254),
  INST(Setns           , "setns"           , Enc(X86Set)            , O(000F00,99,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(_R______), 0 , 1 , kFamilyNone, 0  , 2359, 255),
  INST(Setnz           , "setnz"           , Enc(X86Set)            , O(000F00,95,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(__R_____), 0 , 1 , kFamilyNone, 0  , 2365, 250),
  INST(Seto            , "seto"            , Enc(X86Set)            , O(000F00,90,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(R_______), 0 , 1 , kFamilyNone, 0  , 2371, 253),
  INST(Setp            , "setp"            , Enc(X86Set)            , O(000F00,9A,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(____R___), 0 , 1 , kFamilyNone, 0  , 2376, 254),
  INST(Setpe           , "setpe"           , Enc(X86Set)            , O(000F00,9A,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(____R___), 0 , 1 , kFamilyNone, 0  , 2381, 254),
  INST(Setpo           , "setpo"           , Enc(X86Set)            , O(000F00,9B,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(____R___), 0 , 1 , kFamilyNone, 0  , 2387, 254),
  INST(Sets            , "sets"            , Enc(X86Set)            , O(000F00,98,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(_R______), 0 , 1 , kFamilyNone, 0  , 2393, 255),
  INST(Setz            , "setz"            , Enc(X86Set)            , O(000F00,94,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(__R_____), 0 , 1 , kFamilyNone, 0  , 2398, 250),
  INST(Sfence          , "sfence"          , Enc(X86Fence)          , O(000F00,AE,7,_,_,_,_,_  ), 0                         , F(Volatile)                           , EF(________), 0 , 0 , kFamilyNone, 0  , 2403, 71 ),
  INST(Sha1msg1        , "sha1msg1"        , Enc(ExtRm)             , O(000F38,C9,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 135, 2410, 6  ),
  INST(Sha1msg2        , "sha1msg2"        , Enc(ExtRm)             , O(000F38,CA,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 135, 2419, 6  ),
  INST(Sha1nexte       , "sha1nexte"       , Enc(ExtRm)             , O(000F38,C8,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 135, 2428, 6  ),
  INST(Sha1rnds4       , "sha1rnds4"       , Enc(ExtRmi)            , O(000F3A,CC,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 135, 2438, 15 ),
  INST(Sha256msg1      , "sha256msg1"      , Enc(ExtRm)             , O(000F38,CC,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 135, 2448, 6  ),
  INST(Sha256msg2      , "sha256msg2"      , Enc(ExtRm)             , O(000F38,CD,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 135, 2459, 6  ),
  INST(Sha256rnds2     , "sha256rnds2"     , Enc(ExtRm_XMM0)        , O(000F38,CB,_,_,_,_,_,_  ), 0                         , F(RW)|F(Special)                      , EF(________), 0 , 0 , kFamilySse , 135, 2470, 16 ),
  INST(Shl             , "shl"             , Enc(X86Rot)            , O(000000,D0,4,_,x,_,_,_  ), 0                         , F(RW)|F(Special)                      , EF(WWWUWW__), 0 , 0 , kFamilyNone, 0  , 2482, 245),
  INST(Shld            , "shld"            , Enc(X86ShldShrd)       , O(000F00,A4,_,_,x,_,_,_  ), 0                         , F(RW)|F(Special)                      , EF(UWWUWW__), 0 , 0 , kFamilyNone, 0  , 7858, 256),
  INST(Shlx            , "shlx"            , Enc(VexRmv_Wx)         , V(660F38,F7,_,0,x,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 2486, 246),
  INST(Shr             , "shr"             , Enc(X86Rot)            , O(000000,D0,5,_,x,_,_,_  ), 0                         , F(RW)|F(Special)                      , EF(WWWUWW__), 0 , 0 , kFamilyNone, 0  , 2491, 245),
  INST(Shrd            , "shrd"            , Enc(X86ShldShrd)       , O(000F00,AC,_,_,x,_,_,_  ), 0                         , F(RW)|F(Special)                      , EF(UWWUWW__), 0 , 0 , kFamilyNone, 0  , 2495, 256),
  INST(Shrx            , "shrx"            , Enc(VexRmv_Wx)         , V(F20F38,F7,_,0,x,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 0 , kFamilyNone, 0  , 2500, 246),
  INST(Shufpd          , "shufpd"          , Enc(ExtRmi)            , O(660F00,C6,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 136, 8968, 15 ),
  INST(Shufps          , "shufps"          , Enc(ExtRmi)            , O(000F00,C6,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 137, 8976, 15 ),
  INST(Sqrtpd          , "sqrtpd"          , Enc(ExtRm)             , O(660F00,51,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 116, 8984, 55 ),
  INST(Sqrtps          , "sqrtps"          , Enc(ExtRm)             , O(000F00,51,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 16, kFamilySse , 138, 8699, 55 ),
  INST(Sqrtsd          , "sqrtsd"          , Enc(ExtRm)             , O(F20F00,51,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 8 , kFamilySse , 136, 9000, 257),
  INST(Sqrtss          , "sqrtss"          , Enc(ExtRm)             , O(F30F00,51,_,_,_,_,_,_  ), 0                         , F(WO)                                 , EF(________), 0 , 4 , kFamilySse , 137, 8708, 234),
  INST(Stac            , "stac"            , Enc(X86Op)             , O(000F01,CB,_,_,_,_,_,_  ), 0                         , F(Volatile)                           , EF(___W____), 0 , 0 , kFamilyNone, 0  , 2505, 28 ),
  INST(Stc             , "stc"             , Enc(X86Op)             , O(000000,F9,_,_,_,_,_,_  ), 0                         , 0                                     , EF(_____W__), 0 , 0 , kFamilyNone, 0  , 2510, 258),
  INST(Std             , "std"             , Enc(X86Op)             , O(000000,FD,_,_,_,_,_,_  ), 0                         , 0                                     , EF(______W_), 0 , 0 , kFamilyNone, 0  , 5959, 259),
  INST(Sti             , "sti"             , Enc(X86Op)             , O(000000,FB,_,_,_,_,_,_  ), 0                         , 0                                     , EF(_______W), 0 , 0 , kFamilyNone, 0  , 2514, 260),
  INST(Stmxcsr         , "stmxcsr"         , Enc(X86M_Only)         , O(000F00,AE,3,_,_,_,_,_  ), 0                         , F(Volatile)                           , EF(________), 0 , 0 , kFamilyNone, 0  , 9016, 261),
  INST(Stos            , "stos"            , Enc(X86StrMr)          , O(000000,AA,_,_,_,_,_,_  ), 0                         , F(RW)|F(Special)|F(Rep)               , EF(______R_), 0 , 0 , kFamilyNone, 0  , 2518, 262),
  INST(Sub             , "sub"             , Enc(X86Arith)          , O(000000,28,5,_,x,_,_,_  ), 0                         , F(RW)|F(Lock)                         , EF(WWWWWW__), 0 , 0 , kFamilyNone, 0  , 753 , 5  ),
  INST(Subpd           , "subpd"           , Enc(ExtRm)             , O(660F00,5C,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 139, 4099, 6  ),
  INST(Subps           , "subps"           , Enc(ExtRm)             , O(000F00,5C,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 140, 4111, 6  ),
  INST(Subsd           , "subsd"           , Enc(ExtRm)             , O(F20F00,5C,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 139, 4787, 7  ),
  INST(Subss           , "subss"           , Enc(ExtRm)             , O(F30F00,5C,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 140, 4797, 8  ),
  INST(Swapgs          , "swapgs"          , Enc(X86Op)             , O(000F01,F8,_,_,_,_,_,_  ), 0                         , 0                                     , EF(________), 0 , 0 , kFamilyNone, 0  , 2523, 263),
  INST(T1mskc          , "t1mskc"          , Enc(VexVm_Wx)          , V(XOP_M9,01,7,0,x,_,_,_  ), 0                         , F(WO)                                 , EF(WWWUUW__), 0 , 0 , kFamilyNone, 0  , 2530, 14 ),
  INST(Test            , "test"            , Enc(X86Test)           , O(000000,84,_,_,x,_,_,_  ), O(000000,F6,_,_,x,_,_,_  ), F(RO)                                 , EF(WWWUWW__), 0 , 0 , kFamilyNone, 0  , 8172, 264),
  INST(Tzcnt           , "tzcnt"           , Enc(X86Rm)             , O(F30F00,BC,_,_,x,_,_,_  ), 0                         , F(WO)                                 , EF(UUWUUW__), 0 , 0 , kFamilyNone, 0  , 2537, 210),
  INST(Tzmsk           , "tzmsk"           , Enc(VexVm_Wx)          , V(XOP_M9,01,4,0,x,_,_,_  ), 0                         , F(WO)                                 , EF(WWWUUW__), 0 , 0 , kFamilyNone, 0  , 2543, 14 ),
  INST(Ucomisd         , "ucomisd"         , Enc(ExtRm)             , O(660F00,2E,_,_,_,_,_,_  ), 0                         , F(RO)                                 , EF(WWWWWW__), 0 , 0 , kFamilySse , 141, 9069, 49 ),
  INST(Ucomiss         , "ucomiss"         , Enc(ExtRm)             , O(000F00,2E,_,_,_,_,_,_  ), 0                         , F(RO)                                 , EF(WWWWWW__), 0 , 0 , kFamilySse , 142, 9078, 50 ),
  INST(Ud2             , "ud2"             , Enc(X86Op)             , O(000F00,0B,_,_,_,_,_,_  ), 0                         , 0                                     , EF(________), 0 , 0 , kFamilyNone, 0  , 2549, 265),
  INST(Unpckhpd        , "unpckhpd"        , Enc(ExtRm)             , O(660F00,15,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 143, 9087, 6  ),
  INST(Unpckhps        , "unpckhps"        , Enc(ExtRm)             , O(000F00,15,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 144, 9097, 6  ),
  INST(Unpcklpd        , "unpcklpd"        , Enc(ExtRm)             , O(660F00,14,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 143, 9107, 6  ),
  INST(Unpcklps        , "unpcklps"        , Enc(ExtRm)             , O(000F00,14,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 144, 9117, 6  ),
  INST(Vaddpd          , "vaddpd"          , Enc(VexRvm_Lx)         , V(660F00,58,_,x,I,1,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 0  , 2553, 266),
  INST(Vaddps          , "vaddps"          , Enc(VexRvm_Lx)         , V(000F00,58,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 1  , 2560, 266),
  INST(Vaddsd          , "vaddsd"          , Enc(VexRvm)            , V(F20F00,58,_,I,I,1,3,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 2  , 2567, 267),
  INST(Vaddss          , "vaddss"          , Enc(VexRvm)            , V(F30F00,58,_,I,I,0,2,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 2  , 2574, 268),
  INST(Vaddsubpd       , "vaddsubpd"       , Enc(VexRvm_Lx)         , V(660F00,D0,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 3  , 2581, 269),
  INST(Vaddsubps       , "vaddsubps"       , Enc(VexRvm_Lx)         , V(F20F00,D0,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 3  , 2591, 269),
  INST(Vaesdec         , "vaesdec"         , Enc(VexRvm)            , V(660F38,DE,_,0,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 4  , 2601, 270),
  INST(Vaesdeclast     , "vaesdeclast"     , Enc(VexRvm)            , V(660F38,DF,_,0,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 4  , 2609, 270),
  INST(Vaesenc         , "vaesenc"         , Enc(VexRvm)            , V(660F38,DC,_,0,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 4  , 2621, 270),
  INST(Vaesenclast     , "vaesenclast"     , Enc(VexRvm)            , V(660F38,DD,_,0,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 4  , 2629, 270),
  INST(Vaesimc         , "vaesimc"         , Enc(VexRm)             , V(660F38,DB,_,0,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 4  , 2641, 271),
  INST(Vaeskeygenassist, "vaeskeygenassist", Enc(VexRmi)            , V(660F3A,DF,_,0,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 4  , 2649, 272),
  INST(Valignd         , "valignd"         , Enc(VexRvmi_Lx)        , V(660F3A,03,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 5  , 2666, 273),
  INST(Valignq         , "valignq"         , Enc(VexRvmi_Lx)        , V(660F3A,03,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 6  , 2674, 273),
  INST(Vandnpd         , "vandnpd"         , Enc(VexRvm_Lx)         , V(660F00,55,_,x,I,1,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 7  , 2682, 266),
  INST(Vandnps         , "vandnps"         , Enc(VexRvm_Lx)         , V(000F00,55,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 8  , 2690, 266),
  INST(Vandpd          , "vandpd"          , Enc(VexRvm_Lx)         , V(660F00,54,_,x,I,1,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 7  , 2698, 266),
  INST(Vandps          , "vandps"          , Enc(VexRvm_Lx)         , V(000F00,54,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 8  , 2705, 266),
  INST(Vblendmb        , "vblendmb"        , Enc(VexRvm_Lx)         , V(660F38,66,_,x,_,0,4,FVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 9  , 2712, 274),
  INST(Vblendmd        , "vblendmd"        , Enc(VexRvm_Lx)         , V(660F38,64,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 5  , 2721, 274),
  INST(Vblendmpd       , "vblendmpd"       , Enc(VexRvm_Lx)         , V(660F38,65,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 6  , 2730, 274),
  INST(Vblendmps       , "vblendmps"       , Enc(VexRvm_Lx)         , V(660F38,65,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 5  , 2740, 274),
  INST(Vblendmq        , "vblendmq"        , Enc(VexRvm_Lx)         , V(660F38,64,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 6  , 2750, 274),
  INST(Vblendmw        , "vblendmw"        , Enc(VexRvm_Lx)         , V(660F38,66,_,x,_,1,4,FVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 9  , 2759, 274),
  INST(Vblendpd        , "vblendpd"        , Enc(VexRvmi_Lx)        , V(660F3A,0D,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 3  , 2768, 275),
  INST(Vblendps        , "vblendps"        , Enc(VexRvmi_Lx)        , V(660F3A,0C,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 3  , 2777, 275),
  INST(Vblendvpd       , "vblendvpd"       , Enc(VexRvmr_Lx)        , V(660F3A,4B,_,x,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 3  , 2786, 276),
  INST(Vblendvps       , "vblendvps"       , Enc(VexRvmr_Lx)        , V(660F3A,4A,_,x,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 3  , 2796, 276),
  INST(Vbroadcastf128  , "vbroadcastf128"  , Enc(VexRm)             , V(660F38,1A,_,1,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 3  , 2806, 277),
  INST(Vbroadcastf32x2 , "vbroadcastf32x2" , Enc(VexRm_Lx)          , V(660F38,19,_,x,_,0,3,T2 ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 10 , 2821, 278),
  INST(Vbroadcastf32x4 , "vbroadcastf32x4" , Enc(VexRm_Lx)          , V(660F38,1A,_,x,_,0,4,T4 ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 11 , 2837, 279),
  INST(Vbroadcastf32x8 , "vbroadcastf32x8" , Enc(VexRm)             , V(660F38,1B,_,2,_,0,5,T8 ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 12 , 2853, 280),
  INST(Vbroadcastf64x2 , "vbroadcastf64x2" , Enc(VexRm_Lx)          , V(660F38,1A,_,x,_,1,4,T2 ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 10 , 2869, 279),
  INST(Vbroadcastf64x4 , "vbroadcastf64x4" , Enc(VexRm)             , V(660F38,1B,_,2,_,1,5,T4 ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 11 , 2885, 280),
  INST(Vbroadcasti128  , "vbroadcasti128"  , Enc(VexRm)             , V(660F38,5A,_,1,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 13 , 2901, 277),
  INST(Vbroadcasti32x2 , "vbroadcasti32x2" , Enc(VexRm_Lx)          , V(660F38,59,_,x,_,0,3,T2 ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 10 , 2916, 281),
  INST(Vbroadcasti32x4 , "vbroadcasti32x4" , Enc(VexRm_Lx)          , V(660F38,5A,_,x,_,0,4,T4 ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 14 , 2932, 278),
  INST(Vbroadcasti32x8 , "vbroadcasti32x8" , Enc(VexRm)             , V(660F38,5B,_,2,_,0,5,T8 ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 12 , 2948, 282),
  INST(Vbroadcasti64x2 , "vbroadcasti64x2" , Enc(VexRm_Lx)          , V(660F38,5A,_,x,_,1,4,T2 ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 10 , 2964, 278),
  INST(Vbroadcasti64x4 , "vbroadcasti64x4" , Enc(VexRm)             , V(660F38,5B,_,2,_,1,5,T4 ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 11 , 2980, 282),
  INST(Vbroadcastsd    , "vbroadcastsd"    , Enc(VexRm_Lx)          , V(660F38,19,_,x,0,1,3,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 15 , 2996, 283),
  INST(Vbroadcastss    , "vbroadcastss"    , Enc(VexRm_Lx)          , V(660F38,18,_,x,0,0,2,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 15 , 3009, 284),
  INST(Vcmppd          , "vcmppd"          , Enc(VexRvmi_Lx)        , V(660F00,C2,_,x,I,1,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 16 , 3022, 285),
  INST(Vcmpps          , "vcmpps"          , Enc(VexRvmi_Lx)        , V(000F00,C2,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 17 , 3029, 285),
  INST(Vcmpsd          , "vcmpsd"          , Enc(VexRvmi)           , V(F20F00,C2,_,I,I,1,3,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 18 , 3036, 286),
  INST(Vcmpss          , "vcmpss"          , Enc(VexRvmi)           , V(F30F00,C2,_,I,I,0,2,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 18 , 3043, 287),
  INST(Vcomisd         , "vcomisd"         , Enc(VexRm)             , V(660F00,2F,_,I,I,1,3,T1S), 0                         , F(RO)|F(Vex)|F(Evex)                  , EF(WWWWWW__), 0 , 0 , kFamilyAvx , 19 , 3050, 288),
  INST(Vcomiss         , "vcomiss"         , Enc(VexRm)             , V(000F00,2F,_,I,I,0,2,T1S), 0                         , F(RO)|F(Vex)|F(Evex)                  , EF(WWWWWW__), 0 , 0 , kFamilyAvx , 19 , 3058, 289),
  INST(Vcompresspd     , "vcompresspd"     , Enc(VexMr_Lx)          , V(660F38,8A,_,x,_,1,3,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 14 , 3066, 290),
  INST(Vcompressps     , "vcompressps"     , Enc(VexMr_Lx)          , V(660F38,8A,_,x,_,0,2,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 14 , 3078, 290),
  INST(Vcvtdq2pd       , "vcvtdq2pd"       , Enc(VexRm_Lx)          , V(F30F00,E6,_,x,I,0,3,HV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 20 , 3090, 291),
  INST(Vcvtdq2ps       , "vcvtdq2ps"       , Enc(VexRm_Lx)          , V(000F00,5B,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 1  , 3100, 292),
  INST(Vcvtpd2dq       , "vcvtpd2dq"       , Enc(VexRm_Lx)          , V(F20F00,E6,_,x,I,1,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 0  , 3110, 293),
  INST(Vcvtpd2ps       , "vcvtpd2ps"       , Enc(VexRm_Lx)          , V(660F00,5A,_,x,I,1,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 0  , 3120, 294),
  INST(Vcvtpd2qq       , "vcvtpd2qq"       , Enc(VexRm_Lx)          , V(660F00,7B,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 21 , 3130, 295),
  INST(Vcvtpd2udq      , "vcvtpd2udq"      , Enc(VexRm_Lx)          , V(000F00,79,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 22 , 3140, 296),
  INST(Vcvtpd2uqq      , "vcvtpd2uqq"      , Enc(VexRm_Lx)          , V(660F00,79,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 21 , 3151, 295),
  INST(Vcvtph2ps       , "vcvtph2ps"       , Enc(VexRm_Lx)          , V(660F38,13,_,x,0,0,3,HVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 23 , 3162, 291),
  INST(Vcvtps2dq       , "vcvtps2dq"       , Enc(VexRm_Lx)          , V(660F00,5B,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 1  , 3172, 292),
  INST(Vcvtps2pd       , "vcvtps2pd"       , Enc(VexRm_Lx)          , V(000F00,5A,_,x,I,0,4,HV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 1  , 3182, 291),
  INST(Vcvtps2ph       , "vcvtps2ph"       , Enc(VexMri_Lx)         , V(660F3A,1D,_,x,0,0,3,HVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 23 , 3192, 297),
  INST(Vcvtps2qq       , "vcvtps2qq"       , Enc(VexRm_Lx)          , V(660F00,7B,_,x,_,0,3,HV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 24 , 3202, 298),
  INST(Vcvtps2udq      , "vcvtps2udq"      , Enc(VexRm_Lx)          , V(000F00,79,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 25 , 3212, 295),
  INST(Vcvtps2uqq      , "vcvtps2uqq"      , Enc(VexRm_Lx)          , V(660F00,79,_,x,_,0,3,HV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 24 , 3223, 298),
  INST(Vcvtqq2pd       , "vcvtqq2pd"       , Enc(VexRm_Lx)          , V(F30F00,E6,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 21 , 3234, 295),
  INST(Vcvtqq2ps       , "vcvtqq2ps"       , Enc(VexRm_Lx)          , V(000F00,5B,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 21 , 3244, 296),
  INST(Vcvtsd2si       , "vcvtsd2si"       , Enc(VexRm)             , V(F20F00,2D,_,I,x,x,3,T1F), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 26 , 3254, 299),
  INST(Vcvtsd2ss       , "vcvtsd2ss"       , Enc(VexRvm)            , V(F20F00,5A,_,I,I,1,3,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 2  , 3264, 267),
  INST(Vcvtsd2usi      , "vcvtsd2usi"      , Enc(VexRm)             , V(F20F00,79,_,I,_,x,3,T1F), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 27 , 3274, 300),
  INST(Vcvtsi2sd       , "vcvtsi2sd"       , Enc(VexRvm)            , V(F20F00,2A,_,I,x,x,2,T1W), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 26 , 3285, 301),
  INST(Vcvtsi2ss       , "vcvtsi2ss"       , Enc(VexRvm)            , V(F30F00,2A,_,I,x,x,2,T1W), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 26 , 3295, 301),
  INST(Vcvtss2sd       , "vcvtss2sd"       , Enc(VexRvm)            , V(F30F00,5A,_,I,I,0,2,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 18 , 3305, 268),
  INST(Vcvtss2si       , "vcvtss2si"       , Enc(VexRm)             , V(F20F00,2D,_,I,x,x,2,T1F), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 26 , 3315, 302),
  INST(Vcvtss2usi      , "vcvtss2usi"      , Enc(VexRm)             , V(F30F00,79,_,I,_,x,2,T1F), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 27 , 3325, 303),
  INST(Vcvttpd2dq      , "vcvttpd2dq"      , Enc(VexRm_Lx)          , V(660F00,E6,_,x,I,1,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 16 , 3336, 293),
  INST(Vcvttpd2qq      , "vcvttpd2qq"      , Enc(VexRm_Lx)          , V(660F00,7A,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 28 , 3347, 295),
  INST(Vcvttpd2udq     , "vcvttpd2udq"     , Enc(VexRm_Lx)          , V(000F00,78,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 28 , 3358, 296),
  INST(Vcvttpd2uqq     , "vcvttpd2uqq"     , Enc(VexRm_Lx)          , V(660F00,78,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 29 , 3370, 295),
  INST(Vcvttps2dq      , "vcvttps2dq"      , Enc(VexRm_Lx)          , V(F30F00,5B,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 17 , 3382, 292),
  INST(Vcvttps2qq      , "vcvttps2qq"      , Enc(VexRm_Lx)          , V(660F00,7A,_,x,_,0,3,HV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 30 , 3393, 298),
  INST(Vcvttps2udq     , "vcvttps2udq"     , Enc(VexRm_Lx)          , V(000F00,78,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 31 , 3404, 295),
  INST(Vcvttps2uqq     , "vcvttps2uqq"     , Enc(VexRm_Lx)          , V(660F00,78,_,x,_,0,3,HV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 30 , 3416, 298),
  INST(Vcvttsd2si      , "vcvttsd2si"      , Enc(VexRm)             , V(F20F00,2C,_,I,x,x,3,T1F), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 19 , 3428, 299),
  INST(Vcvttsd2usi     , "vcvttsd2usi"     , Enc(VexRm)             , V(F20F00,78,_,I,_,x,3,T1F), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 32 , 3439, 300),
  INST(Vcvttss2si      , "vcvttss2si"      , Enc(VexRm)             , V(F30F00,2C,_,I,x,x,2,T1F), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 19 , 3451, 302),
  INST(Vcvttss2usi     , "vcvttss2usi"     , Enc(VexRm)             , V(F30F00,78,_,I,_,x,2,T1F), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 32 , 3462, 303),
  INST(Vcvtudq2pd      , "vcvtudq2pd"      , Enc(VexRm_Lx)          , V(F30F00,7A,_,x,_,0,3,HV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 5  , 3474, 298),
  INST(Vcvtudq2ps      , "vcvtudq2ps"      , Enc(VexRm_Lx)          , V(F20F00,7A,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 25 , 3485, 295),
  INST(Vcvtuqq2pd      , "vcvtuqq2pd"      , Enc(VexRm_Lx)          , V(F30F00,7A,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 21 , 3496, 295),
  INST(Vcvtuqq2ps      , "vcvtuqq2ps"      , Enc(VexRm_Lx)          , V(F20F00,7A,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 21 , 3507, 296),
  INST(Vcvtusi2sd      , "vcvtusi2sd"      , Enc(VexRvm)            , V(F20F00,7B,_,I,_,x,2,T1W), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 27 , 3518, 304),
  INST(Vcvtusi2ss      , "vcvtusi2ss"      , Enc(VexRvm)            , V(F30F00,7B,_,I,_,x,2,T1W), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 27 , 3529, 304),
  INST(Vdbpsadbw       , "vdbpsadbw"       , Enc(VexRvmi_Lx)        , V(660F3A,42,_,x,_,0,4,FVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 9  , 3540, 273),
  INST(Vdivpd          , "vdivpd"          , Enc(VexRvm_Lx)         , V(660F00,5E,_,x,I,1,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 0  , 3550, 266),
  INST(Vdivps          , "vdivps"          , Enc(VexRvm_Lx)         , V(000F00,5E,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 1  , 3557, 266),
  INST(Vdivsd          , "vdivsd"          , Enc(VexRvm)            , V(F20F00,5E,_,I,I,1,3,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 2  , 3564, 267),
  INST(Vdivss          , "vdivss"          , Enc(VexRvm)            , V(F30F00,5E,_,I,I,0,2,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 2  , 3571, 268),
  INST(Vdppd           , "vdppd"           , Enc(VexRvmi_Lx)        , V(660F3A,41,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 3  , 3578, 275),
  INST(Vdpps           , "vdpps"           , Enc(VexRvmi_Lx)        , V(660F3A,40,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 3  , 3584, 275),
  INST(Vexp2pd         , "vexp2pd"         , Enc(VexRm)             , V(660F38,C8,_,2,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 33 , 3590, 305),
  INST(Vexp2ps         , "vexp2ps"         , Enc(VexRm)             , V(660F38,C8,_,2,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 34 , 3598, 305),
  INST(Vexpandpd       , "vexpandpd"       , Enc(VexRm_Lx)          , V(660F38,88,_,x,_,1,3,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 14 , 3606, 295),
  INST(Vexpandps       , "vexpandps"       , Enc(VexRm_Lx)          , V(660F38,88,_,x,_,0,2,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 14 , 3616, 295),
  INST(Vextractf128    , "vextractf128"    , Enc(VexMri)            , V(660F3A,19,_,1,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 3  , 3626, 306),
  INST(Vextractf32x4   , "vextractf32x4"   , Enc(VexMri_Lx)         , V(660F3A,19,_,x,_,0,4,T4 ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 14 , 3639, 307),
  INST(Vextractf32x8   , "vextractf32x8"   , Enc(VexMri)            , V(660F3A,1B,_,2,_,0,5,T8 ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 12 , 3653, 308),
  INST(Vextractf64x2   , "vextractf64x2"   , Enc(VexMri_Lx)         , V(660F3A,19,_,x,_,1,4,T2 ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 10 , 3667, 307),
  INST(Vextractf64x4   , "vextractf64x4"   , Enc(VexMri)            , V(660F3A,1B,_,2,_,1,5,T4 ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 11 , 3681, 308),
  INST(Vextracti128    , "vextracti128"    , Enc(VexMri)            , V(660F3A,39,_,1,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 13 , 3695, 306),
  INST(Vextracti32x4   , "vextracti32x4"   , Enc(VexMri_Lx)         , V(660F3A,39,_,x,_,0,4,T4 ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 14 , 3708, 307),
  INST(Vextracti32x8   , "vextracti32x8"   , Enc(VexMri)            , V(660F3A,3B,_,2,_,0,5,T8 ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 12 , 3722, 308),
  INST(Vextracti64x2   , "vextracti64x2"   , Enc(VexMri_Lx)         , V(660F3A,39,_,x,_,1,4,T2 ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 10 , 3736, 307),
  INST(Vextracti64x4   , "vextracti64x4"   , Enc(VexMri)            , V(660F3A,3B,_,2,_,1,5,T4 ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 11 , 3750, 308),
  INST(Vextractps      , "vextractps"      , Enc(VexMri)            , V(660F3A,17,_,0,I,I,2,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 35 , 3764, 309),
  INST(Vfixupimmpd     , "vfixupimmpd"     , Enc(VexRvmi_Lx)        , V(660F3A,54,_,x,_,1,4,FV ), 0                         , F(RW)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 28 , 3775, 310),
  INST(Vfixupimmps     , "vfixupimmps"     , Enc(VexRvmi_Lx)        , V(660F3A,54,_,x,_,0,4,FV ), 0                         , F(RW)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 31 , 3787, 310),
  INST(Vfixupimmsd     , "vfixupimmsd"     , Enc(VexRvmi)           , V(660F3A,55,_,I,_,1,3,T1S), 0                         , F(RW)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 36 , 3799, 311),
  INST(Vfixupimmss     , "vfixupimmss"     , Enc(VexRvmi)           , V(660F3A,55,_,I,_,0,2,T1S), 0                         , F(RW)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 36 , 3811, 312),
  INST(Vfmadd132pd     , "vfmadd132pd"     , Enc(VexRvm_Lx)         , V(660F38,98,_,x,1,1,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 37 , 3823, 313),
  INST(Vfmadd132ps     , "vfmadd132ps"     , Enc(VexRvm_Lx)         , V(660F38,98,_,x,0,0,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 38 , 3835, 313),
  INST(Vfmadd132sd     , "vfmadd132sd"     , Enc(VexRvm)            , V(660F38,99,_,I,1,1,3,T1S), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 39 , 3847, 314),
  INST(Vfmadd132ss     , "vfmadd132ss"     , Enc(VexRvm)            , V(660F38,99,_,I,0,0,2,T1S), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 39 , 3859, 315),
  INST(Vfmadd213pd     , "vfmadd213pd"     , Enc(VexRvm_Lx)         , V(660F38,A8,_,x,1,1,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 37 , 3871, 313),
  INST(Vfmadd213ps     , "vfmadd213ps"     , Enc(VexRvm_Lx)         , V(660F38,A8,_,x,0,0,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 38 , 3883, 313),
  INST(Vfmadd213sd     , "vfmadd213sd"     , Enc(VexRvm)            , V(660F38,A9,_,I,1,1,3,T1S), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 39 , 3895, 314),
  INST(Vfmadd213ss     , "vfmadd213ss"     , Enc(VexRvm)            , V(660F38,A9,_,I,0,0,2,T1S), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 39 , 3907, 315),
  INST(Vfmadd231pd     , "vfmadd231pd"     , Enc(VexRvm_Lx)         , V(660F38,B8,_,x,1,1,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 37 , 3919, 313),
  INST(Vfmadd231ps     , "vfmadd231ps"     , Enc(VexRvm_Lx)         , V(660F38,B8,_,x,0,0,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 38 , 3931, 313),
  INST(Vfmadd231sd     , "vfmadd231sd"     , Enc(VexRvm)            , V(660F38,B9,_,I,1,1,3,T1S), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 39 , 3943, 314),
  INST(Vfmadd231ss     , "vfmadd231ss"     , Enc(VexRvm)            , V(660F38,B9,_,I,0,0,2,T1S), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 39 , 3955, 315),
  INST(Vfmaddpd        , "vfmaddpd"        , Enc(Fma4_Lx)           , V(660F3A,69,_,x,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 40 , 3967, 316),
  INST(Vfmaddps        , "vfmaddps"        , Enc(Fma4_Lx)           , V(660F3A,68,_,x,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 40 , 3976, 316),
  INST(Vfmaddsd        , "vfmaddsd"        , Enc(Fma4)              , V(660F3A,6B,_,0,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 40 , 3985, 317),
  INST(Vfmaddss        , "vfmaddss"        , Enc(Fma4)              , V(660F3A,6A,_,0,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 40 , 3994, 318),
  INST(Vfmaddsub132pd  , "vfmaddsub132pd"  , Enc(VexRvm_Lx)         , V(660F38,96,_,x,1,1,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 37 , 4003, 313),
  INST(Vfmaddsub132ps  , "vfmaddsub132ps"  , Enc(VexRvm_Lx)         , V(660F38,96,_,x,0,0,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 38 , 4018, 313),
  INST(Vfmaddsub213pd  , "vfmaddsub213pd"  , Enc(VexRvm_Lx)         , V(660F38,A6,_,x,1,1,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 37 , 4033, 313),
  INST(Vfmaddsub213ps  , "vfmaddsub213ps"  , Enc(VexRvm_Lx)         , V(660F38,A6,_,x,0,0,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 38 , 4048, 313),
  INST(Vfmaddsub231pd  , "vfmaddsub231pd"  , Enc(VexRvm_Lx)         , V(660F38,B6,_,x,1,1,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 37 , 4063, 313),
  INST(Vfmaddsub231ps  , "vfmaddsub231ps"  , Enc(VexRvm_Lx)         , V(660F38,B6,_,x,0,0,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 38 , 4078, 313),
  INST(Vfmaddsubpd     , "vfmaddsubpd"     , Enc(Fma4_Lx)           , V(660F3A,5D,_,x,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 40 , 4093, 316),
  INST(Vfmaddsubps     , "vfmaddsubps"     , Enc(Fma4_Lx)           , V(660F3A,5C,_,x,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 40 , 4105, 316),
  INST(Vfmsub132pd     , "vfmsub132pd"     , Enc(VexRvm_Lx)         , V(660F38,9A,_,x,1,1,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 37 , 4117, 313),
  INST(Vfmsub132ps     , "vfmsub132ps"     , Enc(VexRvm_Lx)         , V(660F38,9A,_,x,0,0,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 38 , 4129, 313),
  INST(Vfmsub132sd     , "vfmsub132sd"     , Enc(VexRvm)            , V(660F38,9B,_,I,1,1,3,T1S), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 39 , 4141, 314),
  INST(Vfmsub132ss     , "vfmsub132ss"     , Enc(VexRvm)            , V(660F38,9B,_,I,0,0,2,T1S), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 39 , 4153, 315),
  INST(Vfmsub213pd     , "vfmsub213pd"     , Enc(VexRvm_Lx)         , V(660F38,AA,_,x,1,1,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 37 , 4165, 313),
  INST(Vfmsub213ps     , "vfmsub213ps"     , Enc(VexRvm_Lx)         , V(660F38,AA,_,x,0,0,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 38 , 4177, 313),
  INST(Vfmsub213sd     , "vfmsub213sd"     , Enc(VexRvm)            , V(660F38,AB,_,I,1,1,3,T1S), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 39 , 4189, 314),
  INST(Vfmsub213ss     , "vfmsub213ss"     , Enc(VexRvm)            , V(660F38,AB,_,I,0,0,2,T1S), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 39 , 4201, 315),
  INST(Vfmsub231pd     , "vfmsub231pd"     , Enc(VexRvm_Lx)         , V(660F38,BA,_,x,1,1,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 37 , 4213, 313),
  INST(Vfmsub231ps     , "vfmsub231ps"     , Enc(VexRvm_Lx)         , V(660F38,BA,_,x,0,0,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 38 , 4225, 313),
  INST(Vfmsub231sd     , "vfmsub231sd"     , Enc(VexRvm)            , V(660F38,BB,_,I,1,1,3,T1S), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 39 , 4237, 314),
  INST(Vfmsub231ss     , "vfmsub231ss"     , Enc(VexRvm)            , V(660F38,BB,_,I,0,0,2,T1S), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 39 , 4249, 315),
  INST(Vfmsubadd132pd  , "vfmsubadd132pd"  , Enc(VexRvm_Lx)         , V(660F38,97,_,x,1,1,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 37 , 4261, 313),
  INST(Vfmsubadd132ps  , "vfmsubadd132ps"  , Enc(VexRvm_Lx)         , V(660F38,97,_,x,0,0,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 38 , 4276, 313),
  INST(Vfmsubadd213pd  , "vfmsubadd213pd"  , Enc(VexRvm_Lx)         , V(660F38,A7,_,x,1,1,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 37 , 4291, 313),
  INST(Vfmsubadd213ps  , "vfmsubadd213ps"  , Enc(VexRvm_Lx)         , V(660F38,A7,_,x,0,0,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 38 , 4306, 313),
  INST(Vfmsubadd231pd  , "vfmsubadd231pd"  , Enc(VexRvm_Lx)         , V(660F38,B7,_,x,1,1,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 37 , 4321, 313),
  INST(Vfmsubadd231ps  , "vfmsubadd231ps"  , Enc(VexRvm_Lx)         , V(660F38,B7,_,x,0,0,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 38 , 4336, 313),
  INST(Vfmsubaddpd     , "vfmsubaddpd"     , Enc(Fma4_Lx)           , V(660F3A,5F,_,x,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 40 , 4351, 316),
  INST(Vfmsubaddps     , "vfmsubaddps"     , Enc(Fma4_Lx)           , V(660F3A,5E,_,x,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 40 , 4363, 316),
  INST(Vfmsubpd        , "vfmsubpd"        , Enc(Fma4_Lx)           , V(660F3A,6D,_,x,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 40 , 4375, 316),
  INST(Vfmsubps        , "vfmsubps"        , Enc(Fma4_Lx)           , V(660F3A,6C,_,x,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 40 , 4384, 316),
  INST(Vfmsubsd        , "vfmsubsd"        , Enc(Fma4)              , V(660F3A,6F,_,0,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 40 , 4393, 317),
  INST(Vfmsubss        , "vfmsubss"        , Enc(Fma4)              , V(660F3A,6E,_,0,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 40 , 4402, 318),
  INST(Vfnmadd132pd    , "vfnmadd132pd"    , Enc(VexRvm_Lx)         , V(660F38,9C,_,x,1,1,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 37 , 4411, 313),
  INST(Vfnmadd132ps    , "vfnmadd132ps"    , Enc(VexRvm_Lx)         , V(660F38,9C,_,x,0,0,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 38 , 4424, 313),
  INST(Vfnmadd132sd    , "vfnmadd132sd"    , Enc(VexRvm)            , V(660F38,9D,_,I,1,1,3,T1S), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 39 , 4437, 314),
  INST(Vfnmadd132ss    , "vfnmadd132ss"    , Enc(VexRvm)            , V(660F38,9D,_,I,0,0,2,T1S), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 39 , 4450, 315),
  INST(Vfnmadd213pd    , "vfnmadd213pd"    , Enc(VexRvm_Lx)         , V(660F38,AC,_,x,1,1,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 37 , 4463, 313),
  INST(Vfnmadd213ps    , "vfnmadd213ps"    , Enc(VexRvm_Lx)         , V(660F38,AC,_,x,0,0,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 38 , 4476, 313),
  INST(Vfnmadd213sd    , "vfnmadd213sd"    , Enc(VexRvm)            , V(660F38,AD,_,I,1,1,3,T1S), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 39 , 4489, 314),
  INST(Vfnmadd213ss    , "vfnmadd213ss"    , Enc(VexRvm)            , V(660F38,AD,_,I,0,0,2,T1S), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 39 , 4502, 315),
  INST(Vfnmadd231pd    , "vfnmadd231pd"    , Enc(VexRvm_Lx)         , V(660F38,BC,_,x,1,1,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 37 , 4515, 313),
  INST(Vfnmadd231ps    , "vfnmadd231ps"    , Enc(VexRvm_Lx)         , V(660F38,BC,_,x,0,0,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 38 , 4528, 313),
  INST(Vfnmadd231sd    , "vfnmadd231sd"    , Enc(VexRvm)            , V(660F38,BC,_,I,1,1,3,T1S), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 39 , 4541, 314),
  INST(Vfnmadd231ss    , "vfnmadd231ss"    , Enc(VexRvm)            , V(660F38,BC,_,I,0,0,2,T1S), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 39 , 4554, 315),
  INST(Vfnmaddpd       , "vfnmaddpd"       , Enc(Fma4_Lx)           , V(660F3A,79,_,x,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 40 , 4567, 316),
  INST(Vfnmaddps       , "vfnmaddps"       , Enc(Fma4_Lx)           , V(660F3A,78,_,x,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 40 , 4577, 316),
  INST(Vfnmaddsd       , "vfnmaddsd"       , Enc(Fma4)              , V(660F3A,7B,_,0,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 40 , 4587, 317),
  INST(Vfnmaddss       , "vfnmaddss"       , Enc(Fma4)              , V(660F3A,7A,_,0,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 40 , 4597, 318),
  INST(Vfnmsub132pd    , "vfnmsub132pd"    , Enc(VexRvm_Lx)         , V(660F38,9E,_,x,1,1,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 37 , 4607, 313),
  INST(Vfnmsub132ps    , "vfnmsub132ps"    , Enc(VexRvm_Lx)         , V(660F38,9E,_,x,0,0,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 38 , 4620, 313),
  INST(Vfnmsub132sd    , "vfnmsub132sd"    , Enc(VexRvm)            , V(660F38,9F,_,I,1,1,3,T1S), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 39 , 4633, 314),
  INST(Vfnmsub132ss    , "vfnmsub132ss"    , Enc(VexRvm)            , V(660F38,9F,_,I,0,0,2,T1S), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 39 , 4646, 315),
  INST(Vfnmsub213pd    , "vfnmsub213pd"    , Enc(VexRvm_Lx)         , V(660F38,AE,_,x,1,1,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 37 , 4659, 313),
  INST(Vfnmsub213ps    , "vfnmsub213ps"    , Enc(VexRvm_Lx)         , V(660F38,AE,_,x,0,0,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 38 , 4672, 313),
  INST(Vfnmsub213sd    , "vfnmsub213sd"    , Enc(VexRvm)            , V(660F38,AF,_,I,1,1,3,T1S), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 39 , 4685, 314),
  INST(Vfnmsub213ss    , "vfnmsub213ss"    , Enc(VexRvm)            , V(660F38,AF,_,I,0,0,2,T1S), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 39 , 4698, 315),
  INST(Vfnmsub231pd    , "vfnmsub231pd"    , Enc(VexRvm_Lx)         , V(660F38,BE,_,x,1,1,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 37 , 4711, 313),
  INST(Vfnmsub231ps    , "vfnmsub231ps"    , Enc(VexRvm_Lx)         , V(660F38,BE,_,x,0,0,4,FV ), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 38 , 4724, 313),
  INST(Vfnmsub231sd    , "vfnmsub231sd"    , Enc(VexRvm)            , V(660F38,BF,_,I,1,1,3,T1S), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 39 , 4737, 314),
  INST(Vfnmsub231ss    , "vfnmsub231ss"    , Enc(VexRvm)            , V(660F38,BF,_,I,0,0,2,T1S), 0                         , F(RW)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 39 , 4750, 315),
  INST(Vfnmsubpd       , "vfnmsubpd"       , Enc(Fma4_Lx)           , V(660F3A,7D,_,x,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 40 , 4763, 316),
  INST(Vfnmsubps       , "vfnmsubps"       , Enc(Fma4_Lx)           , V(660F3A,7C,_,x,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 40 , 4773, 316),
  INST(Vfnmsubsd       , "vfnmsubsd"       , Enc(Fma4)              , V(660F3A,7F,_,0,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 40 , 4783, 317),
  INST(Vfnmsubss       , "vfnmsubss"       , Enc(Fma4)              , V(660F3A,7E,_,0,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 40 , 4793, 318),
  INST(Vfpclasspd      , "vfpclasspd"      , Enc(VexRmi_Lx)         , V(660F3A,66,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 41 , 4803, 319),
  INST(Vfpclassps      , "vfpclassps"      , Enc(VexRmi_Lx)         , V(660F3A,66,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 42 , 4814, 319),
  INST(Vfpclasssd      , "vfpclasssd"      , Enc(VexRmi_Lx)         , V(660F3A,67,_,I,_,1,3,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 43 , 4825, 320),
  INST(Vfpclassss      , "vfpclassss"      , Enc(VexRmi_Lx)         , V(660F3A,67,_,I,_,0,2,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 43 , 4836, 321),
  INST(Vfrczpd         , "vfrczpd"         , Enc(VexRm_Lx)          , V(XOP_M9,81,_,x,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 4847, 322),
  INST(Vfrczps         , "vfrczps"         , Enc(VexRm_Lx)          , V(XOP_M9,80,_,x,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 4855, 322),
  INST(Vfrczsd         , "vfrczsd"         , Enc(VexRm)             , V(XOP_M9,83,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 4863, 323),
  INST(Vfrczss         , "vfrczss"         , Enc(VexRm)             , V(XOP_M9,82,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 4871, 324),
  INST(Vgatherdpd      , "vgatherdpd"      , Enc(VexRmvRm_VM)       , V(660F38,92,_,x,1,_,_,_  ), V(660F38,92,_,x,_,1,3,T1S), F(RW)|F(Vex_VM)|F(Evex)               , EF(________), 0 , 0 , kFamilyAvx , 45 , 4879, 325),
  INST(Vgatherdps      , "vgatherdps"      , Enc(VexRmvRm_VM)       , V(660F38,92,_,x,0,_,_,_  ), V(660F38,92,_,x,_,0,2,T1S), F(RW)|F(Vex_VM)|F(Evex)               , EF(________), 0 , 0 , kFamilyAvx , 45 , 4890, 326),
  INST(Vgatherpf0dpd   , "vgatherpf0dpd"   , Enc(VexM_VM)           , V(660F38,C6,1,2,_,1,3,T1S), 0                         , F(RO)|F(VM)|F(Evex)                   , EF(________), 0 , 0 , kFamilyNone, 0  , 4901, 327),
  INST(Vgatherpf0dps   , "vgatherpf0dps"   , Enc(VexM_VM)           , V(660F38,C6,1,2,_,0,2,T1S), 0                         , F(RO)|F(VM)|F(Evex)                   , EF(________), 0 , 0 , kFamilyNone, 0  , 4915, 328),
  INST(Vgatherpf0qpd   , "vgatherpf0qpd"   , Enc(VexM_VM)           , V(660F38,C7,1,2,_,1,3,T1S), 0                         , F(RO)|F(VM)|F(Evex)                   , EF(________), 0 , 0 , kFamilyNone, 0  , 4929, 329),
  INST(Vgatherpf0qps   , "vgatherpf0qps"   , Enc(VexM_VM)           , V(660F38,C7,1,2,_,0,2,T1S), 0                         , F(RO)|F(VM)|F(Evex)                   , EF(________), 0 , 0 , kFamilyNone, 0  , 4943, 329),
  INST(Vgatherpf1dpd   , "vgatherpf1dpd"   , Enc(VexM_VM)           , V(660F38,C6,2,2,_,1,3,T1S), 0                         , F(RO)|F(VM)|F(Evex)                   , EF(________), 0 , 0 , kFamilyNone, 0  , 4957, 327),
  INST(Vgatherpf1dps   , "vgatherpf1dps"   , Enc(VexM_VM)           , V(660F38,C6,2,2,_,0,2,T1S), 0                         , F(RO)|F(VM)|F(Evex)                   , EF(________), 0 , 0 , kFamilyNone, 0  , 4971, 328),
  INST(Vgatherpf1qpd   , "vgatherpf1qpd"   , Enc(VexM_VM)           , V(660F38,C7,2,2,_,1,3,T1S), 0                         , F(RO)|F(VM)|F(Evex)                   , EF(________), 0 , 0 , kFamilyNone, 0  , 4985, 329),
  INST(Vgatherpf1qps   , "vgatherpf1qps"   , Enc(VexM_VM)           , V(660F38,C7,2,2,_,0,2,T1S), 0                         , F(RO)|F(VM)|F(Evex)                   , EF(________), 0 , 0 , kFamilyNone, 0  , 4999, 329),
  INST(Vgatherqpd      , "vgatherqpd"      , Enc(VexRmvRm_VM)       , V(660F38,93,_,x,1,_,_,_  ), V(660F38,93,_,x,_,1,3,T1S), F(RW)|F(Vex_VM)|F(Evex)               , EF(________), 0 , 0 , kFamilyAvx , 45 , 5013, 330),
  INST(Vgatherqps      , "vgatherqps"      , Enc(VexRmvRm_VM)       , V(660F38,93,_,x,0,_,_,_  ), V(660F38,93,_,x,_,0,2,T1S), F(RW)|F(Vex_VM)|F(Evex)               , EF(________), 0 , 0 , kFamilyAvx , 45 , 5024, 331),
  INST(Vgetexppd       , "vgetexppd"       , Enc(VexRm_Lx)          , V(660F38,42,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 28 , 5035, 295),
  INST(Vgetexpps       , "vgetexpps"       , Enc(VexRm_Lx)          , V(660F38,42,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 31 , 5045, 295),
  INST(Vgetexpsd       , "vgetexpsd"       , Enc(VexRm)             , V(660F38,43,_,I,_,1,3,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 36 , 5055, 332),
  INST(Vgetexpss       , "vgetexpss"       , Enc(VexRm)             , V(660F38,43,_,I,_,0,2,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 36 , 5065, 333),
  INST(Vgetmantpd      , "vgetmantpd"      , Enc(VexRmi_Lx)         , V(660F3A,26,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 28 , 5075, 334),
  INST(Vgetmantps      , "vgetmantps"      , Enc(VexRmi_Lx)         , V(660F3A,26,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 31 , 5086, 334),
  INST(Vgetmantsd      , "vgetmantsd"      , Enc(VexRmi)            , V(660F3A,27,_,I,_,1,3,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 36 , 5097, 335),
  INST(Vgetmantss      , "vgetmantss"      , Enc(VexRmi)            , V(660F3A,27,_,I,_,0,2,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 36 , 5108, 336),
  INST(Vhaddpd         , "vhaddpd"         , Enc(VexRvm_Lx)         , V(660F00,7C,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 3  , 5119, 269),
  INST(Vhaddps         , "vhaddps"         , Enc(VexRvm_Lx)         , V(F20F00,7C,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 3  , 5127, 269),
  INST(Vhsubpd         , "vhsubpd"         , Enc(VexRvm_Lx)         , V(660F00,7D,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 3  , 5135, 269),
  INST(Vhsubps         , "vhsubps"         , Enc(VexRvm_Lx)         , V(F20F00,7D,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 3  , 5143, 269),
  INST(Vinsertf128     , "vinsertf128"     , Enc(VexRvmi)           , V(660F3A,18,_,1,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 3  , 5151, 337),
  INST(Vinsertf32x4    , "vinsertf32x4"    , Enc(VexRvmi_Lx)        , V(660F3A,18,_,x,_,0,4,T4 ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 14 , 5163, 338),
  INST(Vinsertf32x8    , "vinsertf32x8"    , Enc(VexRvmi)           , V(660F3A,1A,_,2,_,0,5,T8 ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 12 , 5176, 339),
  INST(Vinsertf64x2    , "vinsertf64x2"    , Enc(VexRvmi_Lx)        , V(660F3A,18,_,x,_,1,4,T2 ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 10 , 5189, 338),
  INST(Vinsertf64x4    , "vinsertf64x4"    , Enc(VexRvmi)           , V(660F3A,1A,_,2,_,1,5,T4 ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 11 , 5202, 339),
  INST(Vinserti128     , "vinserti128"     , Enc(VexRvmi)           , V(660F3A,38,_,1,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 13 , 5215, 337),
  INST(Vinserti32x4    , "vinserti32x4"    , Enc(VexRvmi_Lx)        , V(660F3A,38,_,x,_,0,4,T4 ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 14 , 5227, 338),
  INST(Vinserti32x8    , "vinserti32x8"    , Enc(VexRvmi)           , V(660F3A,3A,_,2,_,0,5,T8 ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 12 , 5240, 339),
  INST(Vinserti64x2    , "vinserti64x2"    , Enc(VexRvmi_Lx)        , V(660F3A,38,_,x,_,1,4,T2 ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 10 , 5253, 338),
  INST(Vinserti64x4    , "vinserti64x4"    , Enc(VexRvmi)           , V(660F3A,3A,_,2,_,1,5,T4 ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 11 , 5266, 339),
  INST(Vinsertps       , "vinsertps"       , Enc(VexRvmi)           , V(660F3A,21,_,0,I,0,2,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 35 , 5279, 340),
  INST(Vlddqu          , "vlddqu"          , Enc(VexRm_Lx)          , V(F20F00,F0,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 3  , 5289, 341),
  INST(Vldmxcsr        , "vldmxcsr"        , Enc(VexM)              , V(000F00,AE,2,0,I,_,_,_  ), 0                         , F(RO)|F(Vex)|F(Volatile)              , EF(________), 0 , 0 , kFamilyNone, 0  , 5296, 342),
  INST(Vmaskmovdqu     , "vmaskmovdqu"     , Enc(VexRm_ZDI)         , V(660F00,F7,_,0,I,_,_,_  ), 0                         , F(RO)|F(Vex)|F(Special)               , EF(________), 0 , 0 , kFamilyAvx , 3  , 5305, 343),
  INST(Vmaskmovpd      , "vmaskmovpd"      , Enc(VexRvmMvr_Lx)      , V(660F38,2D,_,x,0,_,_,_  ), V(660F38,2F,_,x,0,_,_,_  ), F(RW)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 3  , 5317, 344),
  INST(Vmaskmovps      , "vmaskmovps"      , Enc(VexRvmMvr_Lx)      , V(660F38,2C,_,x,0,_,_,_  ), V(660F38,2E,_,x,0,_,_,_  ), F(RW)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 3  , 5328, 345),
  INST(Vmaxpd          , "vmaxpd"          , Enc(VexRvm_Lx)         , V(660F00,5F,_,x,I,1,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 16 , 5339, 266),
  INST(Vmaxps          , "vmaxps"          , Enc(VexRvm_Lx)         , V(000F00,5F,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 17 , 5346, 266),
  INST(Vmaxsd          , "vmaxsd"          , Enc(VexRvm)            , V(F20F00,5F,_,I,I,1,3,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 46 , 5353, 267),
  INST(Vmaxss          , "vmaxss"          , Enc(VexRvm)            , V(F30F00,5F,_,I,I,0,2,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 46 , 5360, 268),
  INST(Vminpd          , "vminpd"          , Enc(VexRvm_Lx)         , V(660F00,5D,_,x,I,1,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 16 , 5367, 266),
  INST(Vminps          , "vminps"          , Enc(VexRvm_Lx)         , V(000F00,5D,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 17 , 5374, 266),
  INST(Vminsd          , "vminsd"          , Enc(VexRvm)            , V(F20F00,5D,_,I,I,1,3,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 46 , 5381, 267),
  INST(Vminss          , "vminss"          , Enc(VexRvm)            , V(F30F00,5D,_,I,I,0,2,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 46 , 5388, 268),
  INST(Vmovapd         , "vmovapd"         , Enc(VexRmMr_Lx)        , V(660F00,28,_,x,I,1,4,FVM), V(660F00,29,_,x,I,1,4,FVM), F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 47 , 5395, 346),
  INST(Vmovaps         , "vmovaps"         , Enc(VexRmMr_Lx)        , V(000F00,28,_,x,I,0,4,FVM), V(000F00,29,_,x,I,0,4,FVM), F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 47 , 5403, 347),
  INST(Vmovd           , "vmovd"           , Enc(VexMovdMovq)       , V(660F00,6E,_,0,0,0,2,T1S), V(660F00,7E,_,0,0,0,2,T1S), F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 35 , 5411, 348),
  INST(Vmovddup        , "vmovddup"        , Enc(VexRm_Lx)          , V(F20F00,12,_,x,I,1,3,DUP), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 47 , 5417, 349),
  INST(Vmovdqa         , "vmovdqa"         , Enc(VexRmMr_Lx)        , V(660F00,6F,_,x,I,_,_,_  ), V(660F00,7F,_,x,I,_,_,_  ), F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 3  , 5426, 350),
  INST(Vmovdqa32       , "vmovdqa32"       , Enc(VexRmMr_Lx)        , V(660F00,6F,_,x,_,0,4,FVM), V(660F00,7F,_,x,_,0,4,FVM), F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 14 , 5434, 351),
  INST(Vmovdqa64       , "vmovdqa64"       , Enc(VexRmMr_Lx)        , V(660F00,6F,_,x,_,1,4,FVM), V(660F00,7F,_,x,_,1,4,FVM), F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 14 , 5444, 352),
  INST(Vmovdqu         , "vmovdqu"         , Enc(VexRmMr_Lx)        , V(F30F00,6F,_,x,I,_,_,_  ), V(F30F00,7F,_,x,I,_,_,_  ), F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 3  , 5454, 353),
  INST(Vmovdqu16       , "vmovdqu16"       , Enc(VexRmMr_Lx)        , V(F20F00,6F,_,x,_,1,4,FVM), V(F20F00,7F,_,x,_,1,4,FVM), F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 9  , 5462, 354),
  INST(Vmovdqu32       , "vmovdqu32"       , Enc(VexRmMr_Lx)        , V(F30F00,6F,_,x,_,0,4,FVM), V(F30F00,7F,_,x,_,0,4,FVM), F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 14 , 5472, 355),
  INST(Vmovdqu64       , "vmovdqu64"       , Enc(VexRmMr_Lx)        , V(F30F00,6F,_,x,_,1,4,FVM), V(F30F00,7F,_,x,_,1,4,FVM), F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 14 , 5482, 356),
  INST(Vmovdqu8        , "vmovdqu8"        , Enc(VexRmMr_Lx)        , V(F20F00,6F,_,x,_,0,4,FVM), V(F20F00,7F,_,x,_,0,4,FVM), F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 9  , 5492, 357),
  INST(Vmovhlps        , "vmovhlps"        , Enc(VexRvm)            , V(000F00,12,_,0,I,0,_,_  ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 35 , 5501, 358),
  INST(Vmovhpd         , "vmovhpd"         , Enc(VexRvmMr)          , V(660F00,16,_,0,I,1,3,T1S), V(660F00,17,_,0,I,1,3,T1S), F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 35 , 5510, 359),
  INST(Vmovhps         , "vmovhps"         , Enc(VexRvmMr)          , V(000F00,16,_,0,I,0,3,T2 ), V(000F00,17,_,0,I,0,3,T2 ), F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 35 , 5518, 360),
  INST(Vmovlhps        , "vmovlhps"        , Enc(VexRvm)            , V(000F00,16,_,0,I,0,_,_  ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 35 , 5526, 358),
  INST(Vmovlpd         , "vmovlpd"         , Enc(VexRvmMr)          , V(660F00,12,_,0,I,1,3,T1S), V(660F00,13,_,0,I,1,3,T1S), F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 35 , 5535, 361),
  INST(Vmovlps         , "vmovlps"         , Enc(VexRvmMr)          , V(000F00,12,_,0,I,0,3,T2 ), V(000F00,13,_,0,I,0,3,T2 ), F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 35 , 5543, 362),
  INST(Vmovmskpd       , "vmovmskpd"       , Enc(VexRm_Lx)          , V(660F00,50,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 3  , 5551, 363),
  INST(Vmovmskps       , "vmovmskps"       , Enc(VexRm_Lx)          , V(000F00,50,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 3  , 5561, 363),
  INST(Vmovntdq        , "vmovntdq"        , Enc(VexMr_Lx)          , V(660F00,E7,_,x,I,0,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 48 , 5571, 364),
  INST(Vmovntdqa       , "vmovntdqa"       , Enc(VexRm_Lx)          , V(660F38,2A,_,x,I,0,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 49 , 5580, 365),
  INST(Vmovntpd        , "vmovntpd"        , Enc(VexMr_Lx)          , V(660F00,2B,_,x,I,1,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 48 , 5590, 364),
  INST(Vmovntps        , "vmovntps"        , Enc(VexMr_Lx)          , V(000F00,2B,_,x,I,0,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 48 , 5599, 364),
  INST(Vmovq           , "vmovq"           , Enc(VexMovdMovq)       , V(660F00,6E,_,0,I,1,3,T1S), V(660F00,7E,_,0,I,1,3,T1S), F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 35 , 5608, 366),
  INST(Vmovsd          , "vmovsd"          , Enc(VexMovssMovsd)     , V(F20F00,10,_,I,I,1,3,T1S), V(F20F00,11,_,I,I,1,3,T1S), F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 50 , 5614, 367),
  INST(Vmovshdup       , "vmovshdup"       , Enc(VexRm_Lx)          , V(F30F00,16,_,x,I,0,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 47 , 5621, 292),
  INST(Vmovsldup       , "vmovsldup"       , Enc(VexRm_Lx)          , V(F30F00,12,_,x,I,0,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 47 , 5631, 292),
  INST(Vmovss          , "vmovss"          , Enc(VexMovssMovsd)     , V(F30F00,10,_,I,I,0,2,T1S), V(F30F00,11,_,I,I,0,2,T1S), F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 50 , 5641, 368),
  INST(Vmovupd         , "vmovupd"         , Enc(VexRmMr_Lx)        , V(660F00,10,_,x,I,1,4,FVM), V(660F00,11,_,x,I,1,4,FVM), F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 47 , 5648, 369),
  INST(Vmovups         , "vmovups"         , Enc(VexRmMr_Lx)        , V(000F00,10,_,x,I,0,4,FVM), V(000F00,11,_,x,I,0,4,FVM), F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 47 , 5656, 370),
  INST(Vmpsadbw        , "vmpsadbw"        , Enc(VexRvmi_Lx)        , V(660F3A,42,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 51 , 5664, 275),
  INST(Vmulpd          , "vmulpd"          , Enc(VexRvm_Lx)         , V(660F00,59,_,x,I,1,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 52 , 5673, 266),
  INST(Vmulps          , "vmulps"          , Enc(VexRvm_Lx)         , V(000F00,59,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 53 , 5680, 266),
  INST(Vmulsd          , "vmulsd"          , Enc(VexRvm_Lx)         , V(F20F00,59,_,I,I,1,3,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 2  , 5687, 267),
  INST(Vmulss          , "vmulss"          , Enc(VexRvm_Lx)         , V(F30F00,59,_,I,I,0,2,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 2  , 5694, 268),
  INST(Vorpd           , "vorpd"           , Enc(VexRvm_Lx)         , V(660F00,56,_,x,I,1,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 7  , 5701, 266),
  INST(Vorps           , "vorps"           , Enc(VexRvm_Lx)         , V(000F00,56,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 20 , 5707, 266),
  INST(Vpabsb          , "vpabsb"          , Enc(VexRm_Lx)          , V(660F38,1C,_,x,I,_,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 54 , 5713, 292),
  INST(Vpabsd          , "vpabsd"          , Enc(VexRm_Lx)          , V(660F38,1E,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 15 , 5720, 292),
  INST(Vpabsq          , "vpabsq"          , Enc(VexRm_Lx)          , V(660F38,1F,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 14 , 5727, 295),
  INST(Vpabsw          , "vpabsw"          , Enc(VexRm_Lx)          , V(660F38,1D,_,x,I,_,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 54 , 5734, 292),
  INST(Vpackssdw       , "vpackssdw"       , Enc(VexRvm_Lx)         , V(660F00,6B,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 55 , 5741, 266),
  INST(Vpacksswb       , "vpacksswb"       , Enc(VexRvm_Lx)         , V(660F00,63,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 54 , 5751, 266),
  INST(Vpackusdw       , "vpackusdw"       , Enc(VexRvm_Lx)         , V(660F38,2B,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 55 , 5761, 266),
  INST(Vpackuswb       , "vpackuswb"       , Enc(VexRvm_Lx)         , V(660F00,67,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 54 , 5771, 266),
  INST(Vpaddb          , "vpaddb"          , Enc(VexRvm_Lx)         , V(660F00,FC,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 54 , 5781, 266),
  INST(Vpaddd          , "vpaddd"          , Enc(VexRvm_Lx)         , V(660F00,FE,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 56 , 5788, 266),
  INST(Vpaddq          , "vpaddq"          , Enc(VexRvm_Lx)         , V(660F00,D4,_,x,I,1,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 57 , 5795, 266),
  INST(Vpaddsb         , "vpaddsb"         , Enc(VexRvm_Lx)         , V(660F00,EC,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 54 , 5802, 266),
  INST(Vpaddsw         , "vpaddsw"         , Enc(VexRvm_Lx)         , V(660F00,ED,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 54 , 5810, 266),
  INST(Vpaddusb        , "vpaddusb"        , Enc(VexRvm_Lx)         , V(660F00,DC,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 54 , 5818, 266),
  INST(Vpaddusw        , "vpaddusw"        , Enc(VexRvm_Lx)         , V(660F00,DD,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 54 , 5827, 266),
  INST(Vpaddw          , "vpaddw"          , Enc(VexRvm_Lx)         , V(660F00,FD,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 54 , 5836, 266),
  INST(Vpalignr        , "vpalignr"        , Enc(VexRvmi_Lx)        , V(660F3A,0F,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 54 , 5843, 371),
  INST(Vpand           , "vpand"           , Enc(VexRvm_Lx)         , V(660F00,DB,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 51 , 5852, 269),
  INST(Vpandd          , "vpandd"          , Enc(VexRvm_Lx)         , V(660F00,DB,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 5  , 5858, 274),
  INST(Vpandn          , "vpandn"          , Enc(VexRvm_Lx)         , V(660F00,DF,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 51 , 5865, 269),
  INST(Vpandnd         , "vpandnd"         , Enc(VexRvm_Lx)         , V(660F00,DF,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 5  , 5872, 274),
  INST(Vpandnq         , "vpandnq"         , Enc(VexRvm_Lx)         , V(660F00,DF,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 6  , 5880, 274),
  INST(Vpandq          , "vpandq"          , Enc(VexRvm_Lx)         , V(660F00,DB,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 6  , 5888, 274),
  INST(Vpavgb          , "vpavgb"          , Enc(VexRvm_Lx)         , V(660F00,E0,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 58 , 5895, 266),
  INST(Vpavgw          , "vpavgw"          , Enc(VexRvm_Lx)         , V(660F00,E3,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 59 , 5902, 266),
  INST(Vpblendd        , "vpblendd"        , Enc(VexRvmi_Lx)        , V(660F3A,02,_,x,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 13 , 5909, 275),
  INST(Vpblendvb       , "vpblendvb"       , Enc(VexRvmr)           , V(660F3A,4C,_,x,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 51 , 5918, 276),
  INST(Vpblendw        , "vpblendw"        , Enc(VexRvmi_Lx)        , V(660F3A,0E,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 51 , 5928, 275),
  INST(Vpbroadcastb    , "vpbroadcastb"    , Enc(VexRm_Lx)          , V(660F38,78,_,x,0,0,0,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 59 , 5937, 372),
  INST(Vpbroadcastd    , "vpbroadcastd"    , Enc(VexRm_Lx)          , V(660F38,58,_,x,0,0,2,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 60 , 5950, 373),
  INST(Vpbroadcastmb2d , "vpbroadcastmb2d" , Enc(VexRm_Lx)          , V(F30F38,3A,_,x,_,0,_,_  ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 61 , 5963, 374),
  INST(Vpbroadcastmb2q , "vpbroadcastmb2q" , Enc(VexRm_Lx)          , V(F30F38,2A,_,x,_,1,_,_  ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 61 , 5979, 374),
  INST(Vpbroadcastq    , "vpbroadcastq"    , Enc(VexRm_Lx)          , V(660F38,59,_,x,0,1,3,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 60 , 5995, 375),
  INST(Vpbroadcastw    , "vpbroadcastw"    , Enc(VexRm_Lx)          , V(660F38,79,_,x,0,0,1,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 59 , 6008, 376),
  INST(Vpclmulqdq      , "vpclmulqdq"      , Enc(VexRvmi)           , V(660F3A,44,_,0,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 62 , 6021, 377),
  INST(Vpcmov          , "vpcmov"          , Enc(VexRvrmRvmr_Lx)    , V(XOP_M8,A2,_,x,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 6032, 316),
  INST(Vpcmpb          , "vpcmpb"          , Enc(VexRvm_Lx)         , V(660F3A,3F,_,x,_,0,4,FVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 63 , 6039, 378),
  INST(Vpcmpd          , "vpcmpd"          , Enc(VexRvm_Lx)         , V(660F3A,1F,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 64 , 6046, 378),
  INST(Vpcmpeqb        , "vpcmpeqb"        , Enc(VexRvm_Lx)         , V(660F00,74,_,x,I,I,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 65 , 6053, 379),
  INST(Vpcmpeqd        , "vpcmpeqd"        , Enc(VexRvm_Lx)         , V(660F00,76,_,x,I,0,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 66 , 6062, 379),
  INST(Vpcmpeqq        , "vpcmpeqq"        , Enc(VexRvm_Lx)         , V(660F38,29,_,x,I,1,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 67 , 6071, 379),
  INST(Vpcmpeqw        , "vpcmpeqw"        , Enc(VexRvm_Lx)         , V(660F00,75,_,x,I,I,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 65 , 6080, 379),
  INST(Vpcmpestri      , "vpcmpestri"      , Enc(VexRmi)            , V(660F3A,61,_,0,I,_,_,_  ), 0                         , F(WO)|F(Vex)|F(Special)               , EF(________), 0 , 0 , kFamilyAvx , 3  , 6089, 380),
  INST(Vpcmpestrm      , "vpcmpestrm"      , Enc(VexRmi)            , V(660F3A,60,_,0,I,_,_,_  ), 0                         , F(WO)|F(Vex)|F(Special)               , EF(________), 0 , 0 , kFamilyAvx , 3  , 6100, 381),
  INST(Vpcmpgtb        , "vpcmpgtb"        , Enc(VexRvm_Lx)         , V(660F00,64,_,x,I,I,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 65 , 6111, 379),
  INST(Vpcmpgtd        , "vpcmpgtd"        , Enc(VexRvm_Lx)         , V(660F00,66,_,x,I,0,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 66 , 6120, 379),
  INST(Vpcmpgtq        , "vpcmpgtq"        , Enc(VexRvm_Lx)         , V(660F38,37,_,x,I,1,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 67 , 6129, 379),
  INST(Vpcmpgtw        , "vpcmpgtw"        , Enc(VexRvm_Lx)         , V(660F00,65,_,x,I,I,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 65 , 6138, 379),
  INST(Vpcmpistri      , "vpcmpistri"      , Enc(VexRmi)            , V(660F3A,63,_,0,I,_,_,_  ), 0                         , F(WO)|F(Vex)|F(Special)               , EF(________), 0 , 0 , kFamilyAvx , 3  , 6147, 382),
  INST(Vpcmpistrm      , "vpcmpistrm"      , Enc(VexRmi)            , V(660F3A,62,_,0,I,_,_,_  ), 0                         , F(WO)|F(Vex)|F(Special)               , EF(________), 0 , 0 , kFamilyAvx , 3  , 6158, 383),
  INST(Vpcmpq          , "vpcmpq"          , Enc(VexRvm_Lx)         , V(660F3A,1F,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 68 , 6169, 378),
  INST(Vpcmpub         , "vpcmpub"         , Enc(VexRvm_Lx)         , V(660F3A,3E,_,x,_,0,4,FVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 63 , 6176, 378),
  INST(Vpcmpud         , "vpcmpud"         , Enc(VexRvm_Lx)         , V(660F3A,1E,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 64 , 6184, 378),
  INST(Vpcmpuq         , "vpcmpuq"         , Enc(VexRvm_Lx)         , V(660F3A,1E,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 68 , 6192, 378),
  INST(Vpcmpuw         , "vpcmpuw"         , Enc(VexRvm_Lx)         , V(660F3A,3E,_,x,_,1,4,FVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 69 , 6200, 378),
  INST(Vpcmpw          , "vpcmpw"          , Enc(VexRvm_Lx)         , V(660F3A,3F,_,x,_,1,4,FVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 69 , 6208, 378),
  INST(Vpcomb          , "vpcomb"          , Enc(VexRvmi)           , V(XOP_M8,CC,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 6215, 377),
  INST(Vpcomd          , "vpcomd"          , Enc(VexRvmi)           , V(XOP_M8,CE,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 6222, 377),
  INST(Vpcompressd     , "vpcompressd"     , Enc(VexMr_Lx)          , V(660F38,8B,_,x,_,0,2,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 14 , 6229, 290),
  INST(Vpcompressq     , "vpcompressq"     , Enc(VexMr_Lx)          , V(660F38,8B,_,x,_,1,3,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 14 , 6241, 290),
  INST(Vpcomq          , "vpcomq"          , Enc(VexRvmi)           , V(XOP_M8,CF,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 6253, 377),
  INST(Vpcomub         , "vpcomub"         , Enc(VexRvmi)           , V(XOP_M8,EC,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 6260, 377),
  INST(Vpcomud         , "vpcomud"         , Enc(VexRvmi)           , V(XOP_M8,EE,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 6268, 377),
  INST(Vpcomuq         , "vpcomuq"         , Enc(VexRvmi)           , V(XOP_M8,EF,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 6276, 377),
  INST(Vpcomuw         , "vpcomuw"         , Enc(VexRvmi)           , V(XOP_M8,ED,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 6284, 377),
  INST(Vpcomw          , "vpcomw"          , Enc(VexRvmi)           , V(XOP_M8,CD,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 6292, 377),
  INST(Vpconflictd     , "vpconflictd"     , Enc(VexRm_Lx)          , V(660F38,C4,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 70 , 6299, 295),
  INST(Vpconflictq     , "vpconflictq"     , Enc(VexRm_Lx)          , V(660F38,C4,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 70 , 6311, 295),
  INST(Vperm2f128      , "vperm2f128"      , Enc(VexRvmi)           , V(660F3A,06,_,1,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 3  , 6323, 384),
  INST(Vperm2i128      , "vperm2i128"      , Enc(VexRvmi)           , V(660F3A,46,_,1,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 13 , 6334, 384),
  INST(Vpermb          , "vpermb"          , Enc(VexRvm_Lx)         , V(660F38,8D,_,x,_,0,4,FVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 71 , 6345, 274),
  INST(Vpermd          , "vpermd"          , Enc(VexRvm_Lx)         , V(660F38,36,_,x,0,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 72 , 6352, 385),
  INST(Vpermi2b        , "vpermi2b"        , Enc(VexRvm_Lx)         , V(660F38,75,_,x,_,0,4,FVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 71 , 6359, 274),
  INST(Vpermi2d        , "vpermi2d"        , Enc(VexRvm_Lx)         , V(660F38,76,_,x,_,0,4,FV ), 0                         , F(RW)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 5  , 6368, 386),
  INST(Vpermi2pd       , "vpermi2pd"       , Enc(VexRvm_Lx)         , V(660F38,77,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 6  , 6377, 274),
  INST(Vpermi2ps       , "vpermi2ps"       , Enc(VexRvm_Lx)         , V(660F38,77,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 5  , 6387, 274),
  INST(Vpermi2q        , "vpermi2q"        , Enc(VexRvm_Lx)         , V(660F38,76,_,x,_,1,4,FV ), 0                         , F(RW)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 6  , 6397, 386),
  INST(Vpermi2w        , "vpermi2w"        , Enc(VexRvm_Lx)         , V(660F38,75,_,x,_,1,4,FVM), 0                         , F(RW)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 9  , 6406, 386),
  INST(Vpermil2pd      , "vpermil2pd"      , Enc(VexRvrmiRvmri_Lx)  , V(660F3A,49,_,x,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 6415, 387),
  INST(Vpermil2ps      , "vpermil2ps"      , Enc(VexRvrmiRvmri_Lx)  , V(660F3A,48,_,x,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 6426, 387),
  INST(Vpermilpd       , "vpermilpd"       , Enc(VexRvmRmi_Lx)      , V(660F38,0D,_,x,0,1,4,FV ), V(660F3A,05,_,x,0,1,4,FV ), F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 73 , 6437, 388),
  INST(Vpermilps       , "vpermilps"       , Enc(VexRvmRmi_Lx)      , V(660F38,0C,_,x,0,0,4,FV ), V(660F3A,04,_,x,0,0,4,FV ), F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 73 , 6447, 389),
  INST(Vpermpd         , "vpermpd"         , Enc(VexRmi)            , V(660F3A,01,_,1,1,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 13 , 6457, 390),
  INST(Vpermps         , "vpermps"         , Enc(VexRvm)            , V(660F38,16,_,1,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 13 , 6465, 391),
  INST(Vpermq          , "vpermq"          , Enc(VexRvmRmi_Lx)      , V(660F38,36,_,x,_,1,4,FV ), V(660F3A,00,_,x,1,1,4,FV ), F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 74 , 6473, 392),
  INST(Vpermt2b        , "vpermt2b"        , Enc(VexRvm_Lx)         , V(660F38,7D,_,x,_,0,4,FVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 71 , 6480, 274),
  INST(Vpermt2d        , "vpermt2d"        , Enc(VexRvm_Lx)         , V(660F38,7E,_,x,_,0,4,FV ), 0                         , F(RW)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 5  , 6489, 386),
  INST(Vpermt2pd       , "vpermt2pd"       , Enc(VexRvm_Lx)         , V(660F38,7F,_,x,_,1,4,FV ), 0                         , F(RW)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 6  , 6498, 386),
  INST(Vpermt2ps       , "vpermt2ps"       , Enc(VexRvm_Lx)         , V(660F38,7F,_,x,_,0,4,FV ), 0                         , F(RW)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 5  , 6508, 386),
  INST(Vpermt2q        , "vpermt2q"        , Enc(VexRvm_Lx)         , V(660F38,7E,_,x,_,1,4,FV ), 0                         , F(RW)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 6  , 6518, 386),
  INST(Vpermt2w        , "vpermt2w"        , Enc(VexRvm_Lx)         , V(660F38,7D,_,x,_,1,4,FVM), 0                         , F(RW)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 9  , 6527, 386),
  INST(Vpermw          , "vpermw"          , Enc(VexRvm_Lx)         , V(660F38,8D,_,x,_,1,4,FVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 9  , 6536, 274),
  INST(Vpexpandd       , "vpexpandd"       , Enc(VexRm_Lx)          , V(660F38,89,_,x,_,0,2,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 14 , 6543, 295),
  INST(Vpexpandq       , "vpexpandq"       , Enc(VexRm_Lx)          , V(660F38,89,_,x,_,1,3,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 14 , 6553, 295),
  INST(Vpextrb         , "vpextrb"         , Enc(VexMri)            , V(660F3A,14,_,0,0,I,0,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 75 , 6563, 393),
  INST(Vpextrd         , "vpextrd"         , Enc(VexMri)            , V(660F3A,16,_,0,0,0,2,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 76 , 6571, 309),
  INST(Vpextrq         , "vpextrq"         , Enc(VexMri)            , V(660F3A,16,_,0,1,1,3,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 76 , 6579, 394),
  INST(Vpextrw         , "vpextrw"         , Enc(VexMri)            , V(660F3A,15,_,0,0,I,1,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 75 , 6587, 395),
  INST(Vpgatherdd      , "vpgatherdd"      , Enc(VexRmvRm_VM)       , V(660F38,90,_,x,0,_,_,_  ), V(660F38,90,_,x,_,0,2,T1S), F(RW)|F(Vex_VM)|F(Evex)               , EF(________), 0 , 0 , kFamilyAvx , 45 , 6595, 396),
  INST(Vpgatherdq      , "vpgatherdq"      , Enc(VexRmvRm_VM)       , V(660F38,90,_,x,1,_,_,_  ), V(660F38,90,_,x,_,1,3,T1S), F(RW)|F(Vex_VM)|F(Evex)               , EF(________), 0 , 0 , kFamilyAvx , 45 , 6606, 397),
  INST(Vpgatherqd      , "vpgatherqd"      , Enc(VexRmvRm_VM)       , V(660F38,91,_,x,0,_,_,_  ), V(660F38,91,_,x,_,0,2,T1S), F(RW)|F(Vex_VM)|F(Evex)               , EF(________), 0 , 0 , kFamilyAvx , 45 , 6617, 398),
  INST(Vpgatherqq      , "vpgatherqq"      , Enc(VexRmvRm_VM)       , V(660F38,91,_,x,1,_,_,_  ), V(660F38,91,_,x,_,1,3,T1S), F(RW)|F(Vex_VM)|F(Evex)               , EF(________), 0 , 0 , kFamilyAvx , 45 , 6628, 399),
  INST(Vphaddbd        , "vphaddbd"        , Enc(VexRm)             , V(XOP_M9,C2,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 6639, 271),
  INST(Vphaddbq        , "vphaddbq"        , Enc(VexRm)             , V(XOP_M9,C3,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 6648, 271),
  INST(Vphaddbw        , "vphaddbw"        , Enc(VexRm)             , V(XOP_M9,C1,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 6657, 271),
  INST(Vphaddd         , "vphaddd"         , Enc(VexRvm_Lx)         , V(660F38,02,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 51 , 6666, 269),
  INST(Vphadddq        , "vphadddq"        , Enc(VexRm)             , V(XOP_M9,CB,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 6674, 271),
  INST(Vphaddsw        , "vphaddsw"        , Enc(VexRvm_Lx)         , V(660F38,03,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 51 , 6683, 269),
  INST(Vphaddubd       , "vphaddubd"       , Enc(VexRm)             , V(XOP_M9,D2,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 6692, 271),
  INST(Vphaddubq       , "vphaddubq"       , Enc(VexRm)             , V(XOP_M9,D3,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 6702, 271),
  INST(Vphaddubw       , "vphaddubw"       , Enc(VexRm)             , V(XOP_M9,D1,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 6712, 271),
  INST(Vphaddudq       , "vphaddudq"       , Enc(VexRm)             , V(XOP_M9,DB,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 6722, 271),
  INST(Vphadduwd       , "vphadduwd"       , Enc(VexRm)             , V(XOP_M9,D6,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 6732, 271),
  INST(Vphadduwq       , "vphadduwq"       , Enc(VexRm)             , V(XOP_M9,D7,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 6742, 271),
  INST(Vphaddw         , "vphaddw"         , Enc(VexRvm_Lx)         , V(660F38,01,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 51 , 6752, 269),
  INST(Vphaddwd        , "vphaddwd"        , Enc(VexRm)             , V(XOP_M9,C6,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 6760, 271),
  INST(Vphaddwq        , "vphaddwq"        , Enc(VexRm)             , V(XOP_M9,C7,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 6769, 271),
  INST(Vphminposuw     , "vphminposuw"     , Enc(VexRm)             , V(660F38,41,_,0,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 3  , 6778, 271),
  INST(Vphsubbw        , "vphsubbw"        , Enc(VexRm)             , V(XOP_M9,E1,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 6790, 271),
  INST(Vphsubd         , "vphsubd"         , Enc(VexRvm_Lx)         , V(660F38,06,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 51 , 6799, 269),
  INST(Vphsubdq        , "vphsubdq"        , Enc(VexRm)             , V(XOP_M9,E3,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 6807, 271),
  INST(Vphsubsw        , "vphsubsw"        , Enc(VexRvm_Lx)         , V(660F38,07,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 51 , 6816, 269),
  INST(Vphsubw         , "vphsubw"         , Enc(VexRvm_Lx)         , V(660F38,05,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 51 , 6825, 269),
  INST(Vphsubwd        , "vphsubwd"        , Enc(VexRm)             , V(XOP_M9,E2,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 6833, 271),
  INST(Vpinsrb         , "vpinsrb"         , Enc(VexRvmi)           , V(660F3A,20,_,0,0,I,0,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 77 , 6842, 400),
  INST(Vpinsrd         , "vpinsrd"         , Enc(VexRvmi)           , V(660F3A,22,_,0,0,0,2,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 78 , 6850, 401),
  INST(Vpinsrq         , "vpinsrq"         , Enc(VexRvmi)           , V(660F3A,22,_,0,1,1,3,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 78 , 6858, 402),
  INST(Vpinsrw         , "vpinsrw"         , Enc(VexRvmi)           , V(660F00,C4,_,0,0,I,1,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 77 , 6866, 403),
  INST(Vplzcntd        , "vplzcntd"        , Enc(VexRm_Lx)          , V(660F38,44,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 70 , 6874, 295),
  INST(Vplzcntq        , "vplzcntq"        , Enc(VexRm_Lx)          , V(660F38,44,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 79 , 6883, 295),
  INST(Vpmacsdd        , "vpmacsdd"        , Enc(VexRvmr)           , V(XOP_M8,9E,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 6892, 404),
  INST(Vpmacsdqh       , "vpmacsdqh"       , Enc(VexRvmr)           , V(XOP_M8,9F,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 6901, 404),
  INST(Vpmacsdql       , "vpmacsdql"       , Enc(VexRvmr)           , V(XOP_M8,97,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 6911, 404),
  INST(Vpmacssdd       , "vpmacssdd"       , Enc(VexRvmr)           , V(XOP_M8,8E,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 6921, 404),
  INST(Vpmacssdqh      , "vpmacssdqh"      , Enc(VexRvmr)           , V(XOP_M8,8F,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 6931, 404),
  INST(Vpmacssdql      , "vpmacssdql"      , Enc(VexRvmr)           , V(XOP_M8,87,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 6942, 404),
  INST(Vpmacsswd       , "vpmacsswd"       , Enc(VexRvmr)           , V(XOP_M8,86,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 6953, 404),
  INST(Vpmacssww       , "vpmacssww"       , Enc(VexRvmr)           , V(XOP_M8,85,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 6963, 404),
  INST(Vpmacswd        , "vpmacswd"        , Enc(VexRvmr)           , V(XOP_M8,96,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 6973, 404),
  INST(Vpmacsww        , "vpmacsww"        , Enc(VexRvmr)           , V(XOP_M8,95,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 6982, 404),
  INST(Vpmadcsswd      , "vpmadcsswd"      , Enc(VexRvmr)           , V(XOP_M8,A6,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 6991, 404),
  INST(Vpmadcswd       , "vpmadcswd"       , Enc(VexRvmr)           , V(XOP_M8,B6,_,0,0,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 7002, 404),
  INST(Vpmadd52huq     , "vpmadd52huq"     , Enc(VexRvm_Lx)         , V(660F38,B5,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 80 , 7012, 274),
  INST(Vpmadd52luq     , "vpmadd52luq"     , Enc(VexRvm_Lx)         , V(660F38,B4,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 80 , 7024, 274),
  INST(Vpmaddubsw      , "vpmaddubsw"      , Enc(VexRvm_Lx)         , V(660F38,04,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 54 , 7036, 266),
  INST(Vpmaddwd        , "vpmaddwd"        , Enc(VexRvm_Lx)         , V(660F00,F5,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 54 , 7047, 266),
  INST(Vpmaskmovd      , "vpmaskmovd"      , Enc(VexRvmMvr_Lx)      , V(660F38,8C,_,x,0,_,_,_  ), V(660F38,8E,_,x,0,_,_,_  ), F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 13 , 7056, 405),
  INST(Vpmaskmovq      , "vpmaskmovq"      , Enc(VexRvmMvr_Lx)      , V(660F38,8C,_,x,1,_,_,_  ), V(660F38,8E,_,x,1,_,_,_  ), F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 13 , 7067, 406),
  INST(Vpmaxsb         , "vpmaxsb"         , Enc(VexRvm_Lx)         , V(660F38,3C,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 54 , 7078, 266),
  INST(Vpmaxsd         , "vpmaxsd"         , Enc(VexRvm_Lx)         , V(660F38,3D,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 56 , 7086, 266),
  INST(Vpmaxsq         , "vpmaxsq"         , Enc(VexRvm_Lx)         , V(660F38,3D,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 6  , 7094, 274),
  INST(Vpmaxsw         , "vpmaxsw"         , Enc(VexRvm_Lx)         , V(660F00,EE,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 54 , 7102, 266),
  INST(Vpmaxub         , "vpmaxub"         , Enc(VexRvm_Lx)         , V(660F00,DE,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 54 , 7110, 266),
  INST(Vpmaxud         , "vpmaxud"         , Enc(VexRvm_Lx)         , V(660F38,3F,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 56 , 7118, 266),
  INST(Vpmaxuq         , "vpmaxuq"         , Enc(VexRvm_Lx)         , V(660F38,3F,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 6  , 7126, 274),
  INST(Vpmaxuw         , "vpmaxuw"         , Enc(VexRvm_Lx)         , V(660F38,3E,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 54 , 7134, 266),
  INST(Vpminsb         , "vpminsb"         , Enc(VexRvm_Lx)         , V(660F38,38,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 54 , 7142, 266),
  INST(Vpminsd         , "vpminsd"         , Enc(VexRvm_Lx)         , V(660F38,39,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 56 , 7150, 266),
  INST(Vpminsq         , "vpminsq"         , Enc(VexRvm_Lx)         , V(660F38,39,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 6  , 7158, 274),
  INST(Vpminsw         , "vpminsw"         , Enc(VexRvm_Lx)         , V(660F00,EA,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 54 , 7166, 266),
  INST(Vpminub         , "vpminub"         , Enc(VexRvm_Lx)         , V(660F00,DA,_,x,I,_,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 54 , 7174, 266),
  INST(Vpminud         , "vpminud"         , Enc(VexRvm_Lx)         , V(660F38,3B,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 56 , 7182, 266),
  INST(Vpminuq         , "vpminuq"         , Enc(VexRvm_Lx)         , V(660F38,3B,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 6  , 7190, 274),
  INST(Vpminuw         , "vpminuw"         , Enc(VexRvm_Lx)         , V(660F38,3A,_,x,I,_,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 54 , 7198, 266),
  INST(Vpmovb2m        , "vpmovb2m"        , Enc(VexRm_Lx)          , V(F30F38,29,_,x,_,0,_,_  ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 81 , 7206, 407),
  INST(Vpmovd2m        , "vpmovd2m"        , Enc(VexRm_Lx)          , V(F30F38,39,_,x,_,0,_,_  ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 82 , 7215, 407),
  INST(Vpmovdb         , "vpmovdb"         , Enc(VexMr_Lx)          , V(F30F38,31,_,x,_,0,2,QVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 14 , 7224, 408),
  INST(Vpmovdw         , "vpmovdw"         , Enc(VexMr_Lx)          , V(F30F38,33,_,x,_,0,3,HVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 14 , 7232, 409),
  INST(Vpmovm2b        , "vpmovm2b"        , Enc(VexRm_Lx)          , V(F30F38,28,_,x,_,0,_,_  ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 81 , 7240, 374),
  INST(Vpmovm2d        , "vpmovm2d"        , Enc(VexRm_Lx)          , V(F30F38,38,_,x,_,0,_,_  ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 82 , 7249, 374),
  INST(Vpmovm2q        , "vpmovm2q"        , Enc(VexRm_Lx)          , V(F30F38,38,_,x,_,1,_,_  ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 82 , 7258, 374),
  INST(Vpmovm2w        , "vpmovm2w"        , Enc(VexRm_Lx)          , V(F30F38,28,_,x,_,1,_,_  ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 81 , 7267, 374),
  INST(Vpmovmskb       , "vpmovmskb"       , Enc(VexRm_Lx)          , V(660F00,D7,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 51 , 7276, 363),
  INST(Vpmovq2m        , "vpmovq2m"        , Enc(VexRm_Lx)          , V(F30F38,39,_,x,_,1,_,_  ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 82 , 7286, 407),
  INST(Vpmovqb         , "vpmovqb"         , Enc(VexMr_Lx)          , V(F30F38,32,_,x,_,0,1,OVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 14 , 7295, 410),
  INST(Vpmovqd         , "vpmovqd"         , Enc(VexMr_Lx)          , V(F30F38,35,_,x,_,0,3,HVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 14 , 7303, 409),
  INST(Vpmovqw         , "vpmovqw"         , Enc(VexMr_Lx)          , V(F30F38,34,_,x,_,0,2,QVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 14 , 7311, 408),
  INST(Vpmovsdb        , "vpmovsdb"        , Enc(VexMr_Lx)          , V(F30F38,21,_,x,_,0,2,QVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 14 , 7319, 408),
  INST(Vpmovsdw        , "vpmovsdw"        , Enc(VexMr_Lx)          , V(F30F38,23,_,x,_,0,3,HVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 14 , 7328, 409),
  INST(Vpmovsqb        , "vpmovsqb"        , Enc(VexMr_Lx)          , V(F30F38,22,_,x,_,0,1,OVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 14 , 7337, 410),
  INST(Vpmovsqd        , "vpmovsqd"        , Enc(VexMr_Lx)          , V(F30F38,25,_,x,_,0,3,HVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 14 , 7346, 409),
  INST(Vpmovsqw        , "vpmovsqw"        , Enc(VexMr_Lx)          , V(F30F38,24,_,x,_,0,2,QVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 14 , 7355, 408),
  INST(Vpmovswb        , "vpmovswb"        , Enc(VexMr_Lx)          , V(F30F38,20,_,x,_,0,3,HVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 9  , 7364, 409),
  INST(Vpmovsxbd       , "vpmovsxbd"       , Enc(VexRm_Lx)          , V(660F38,21,_,x,I,I,2,QVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 15 , 7373, 411),
  INST(Vpmovsxbq       , "vpmovsxbq"       , Enc(VexRm_Lx)          , V(660F38,22,_,x,I,I,1,OVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 15 , 7383, 412),
  INST(Vpmovsxbw       , "vpmovsxbw"       , Enc(VexRm_Lx)          , V(660F38,20,_,x,I,I,3,HVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 54 , 7393, 291),
  INST(Vpmovsxdq       , "vpmovsxdq"       , Enc(VexRm_Lx)          , V(660F38,25,_,x,I,0,3,HVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 15 , 7403, 413),
  INST(Vpmovsxwd       , "vpmovsxwd"       , Enc(VexRm_Lx)          , V(660F38,23,_,x,I,I,3,HVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 15 , 7413, 291),
  INST(Vpmovsxwq       , "vpmovsxwq"       , Enc(VexRm_Lx)          , V(660F38,24,_,x,I,I,2,QVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 15 , 7423, 411),
  INST(Vpmovusdb       , "vpmovusdb"       , Enc(VexMr_Lx)          , V(F30F38,11,_,x,_,0,2,QVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 14 , 7433, 408),
  INST(Vpmovusdw       , "vpmovusdw"       , Enc(VexMr_Lx)          , V(F30F38,13,_,x,_,0,3,HVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 14 , 7443, 409),
  INST(Vpmovusqb       , "vpmovusqb"       , Enc(VexMr_Lx)          , V(F30F38,12,_,x,_,0,1,OVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 14 , 7453, 410),
  INST(Vpmovusqd       , "vpmovusqd"       , Enc(VexMr_Lx)          , V(F30F38,15,_,x,_,0,3,HVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 14 , 7463, 409),
  INST(Vpmovusqw       , "vpmovusqw"       , Enc(VexMr_Lx)          , V(F30F38,14,_,x,_,0,2,QVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 14 , 7473, 408),
  INST(Vpmovuswb       , "vpmovuswb"       , Enc(VexMr_Lx)          , V(F30F38,10,_,x,_,0,3,HVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 9  , 7483, 409),
  INST(Vpmovw2m        , "vpmovw2m"        , Enc(VexRm_Lx)          , V(F30F38,29,_,x,_,1,_,_  ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 81 , 7493, 407),
  INST(Vpmovwb         , "vpmovwb"         , Enc(VexMr_Lx)          , V(F30F38,30,_,x,_,0,3,HVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 9  , 7502, 409),
  INST(Vpmovzxbd       , "vpmovzxbd"       , Enc(VexRm_Lx)          , V(660F38,31,_,x,I,I,2,QVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 15 , 7510, 411),
  INST(Vpmovzxbq       , "vpmovzxbq"       , Enc(VexRm_Lx)          , V(660F38,32,_,x,I,I,1,OVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 15 , 7520, 412),
  INST(Vpmovzxbw       , "vpmovzxbw"       , Enc(VexRm_Lx)          , V(660F38,30,_,x,I,I,3,HVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 54 , 7530, 291),
  INST(Vpmovzxdq       , "vpmovzxdq"       , Enc(VexRm_Lx)          , V(660F38,35,_,x,I,0,3,HVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 15 , 7540, 413),
  INST(Vpmovzxwd       , "vpmovzxwd"       , Enc(VexRm_Lx)          , V(660F38,33,_,x,I,I,3,HVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 15 , 7550, 291),
  INST(Vpmovzxwq       , "vpmovzxwq"       , Enc(VexRm_Lx)          , V(660F38,34,_,x,I,I,2,QVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 15 , 7560, 411),
  INST(Vpmuldq         , "vpmuldq"         , Enc(VexRvm_Lx)         , V(660F38,28,_,x,I,1,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 57 , 7570, 266),
  INST(Vpmulhrsw       , "vpmulhrsw"       , Enc(VexRvm_Lx)         , V(660F38,0B,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 54 , 7578, 266),
  INST(Vpmulhuw        , "vpmulhuw"        , Enc(VexRvm_Lx)         , V(660F00,E4,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 54 , 7588, 266),
  INST(Vpmulhw         , "vpmulhw"         , Enc(VexRvm_Lx)         , V(660F00,E5,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 54 , 7597, 266),
  INST(Vpmulld         , "vpmulld"         , Enc(VexRvm_Lx)         , V(660F38,40,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 56 , 7605, 266),
  INST(Vpmullq         , "vpmullq"         , Enc(VexRvm_Lx)         , V(660F38,40,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 83 , 7613, 274),
  INST(Vpmullw         , "vpmullw"         , Enc(VexRvm_Lx)         , V(660F00,D5,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 54 , 7621, 266),
  INST(Vpmultishiftqb  , "vpmultishiftqb"  , Enc(VexRvm_Lx)         , V(660F38,83,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 84 , 7629, 274),
  INST(Vpmuludq        , "vpmuludq"        , Enc(VexRvm_Lx)         , V(660F00,F4,_,x,I,1,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 57 , 7644, 266),
  INST(Vpor            , "vpor"            , Enc(VexRvm_Lx)         , V(660F00,EB,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 51 , 7653, 269),
  INST(Vpord           , "vpord"           , Enc(VexRvm_Lx)         , V(660F00,EB,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 5  , 7658, 274),
  INST(Vporq           , "vporq"           , Enc(VexRvm_Lx)         , V(660F00,EB,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 6  , 7664, 274),
  INST(Vpperm          , "vpperm"          , Enc(VexRvrmRvmr)       , V(XOP_M8,A3,_,0,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 7670, 414),
  INST(Vprold          , "vprold"          , Enc(VexVmi_Lx)         , V(660F00,72,1,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 5  , 7677, 334),
  INST(Vprolq          , "vprolq"          , Enc(VexVmi_Lx)         , V(660F00,72,1,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 6  , 7684, 334),
  INST(Vprolvd         , "vprolvd"         , Enc(VexRvm_Lx)         , V(660F38,15,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 5  , 7691, 274),
  INST(Vprolvq         , "vprolvq"         , Enc(VexRvm_Lx)         , V(660F38,15,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 6  , 7699, 274),
  INST(Vprord          , "vprord"          , Enc(VexVmi_Lx)         , V(660F00,72,0,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 5  , 7707, 334),
  INST(Vprorq          , "vprorq"          , Enc(VexVmi_Lx)         , V(660F00,72,0,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 6  , 7714, 334),
  INST(Vprorvd         , "vprorvd"         , Enc(VexRvm_Lx)         , V(660F38,14,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 5  , 7721, 274),
  INST(Vprorvq         , "vprorvq"         , Enc(VexRvm_Lx)         , V(660F38,14,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 6  , 7729, 274),
  INST(Vprotb          , "vprotb"          , Enc(VexRvmRmvRmi)      , V(XOP_M9,90,_,0,x,_,_,_  ), V(XOP_M8,C0,_,0,x,_,_,_  ), F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 7737, 415),
  INST(Vprotd          , "vprotd"          , Enc(VexRvmRmvRmi)      , V(XOP_M9,92,_,0,x,_,_,_  ), V(XOP_M8,C2,_,0,x,_,_,_  ), F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 7744, 416),
  INST(Vprotq          , "vprotq"          , Enc(VexRvmRmvRmi)      , V(XOP_M9,93,_,0,x,_,_,_  ), V(XOP_M8,C3,_,0,x,_,_,_  ), F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 7751, 417),
  INST(Vprotw          , "vprotw"          , Enc(VexRvmRmvRmi)      , V(XOP_M9,91,_,0,x,_,_,_  ), V(XOP_M8,C1,_,0,x,_,_,_  ), F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 7758, 418),
  INST(Vpsadbw         , "vpsadbw"         , Enc(VexRvm_Lx)         , V(660F00,F6,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 85 , 7765, 266),
  INST(Vpscatterdd     , "vpscatterdd"     , Enc(VexMr_VM)          , V(660F38,A0,_,x,_,0,2,T1S), 0                         , F(WO)|F(VM)|F(Evex)                   , EF(________), 0 , 0 , kFamilyAvx , 86 , 7773, 419),
  INST(Vpscatterdq     , "vpscatterdq"     , Enc(VexMr_VM)          , V(660F38,A0,_,x,_,1,3,T1S), 0                         , F(WO)|F(VM)|F(Evex)                   , EF(________), 0 , 0 , kFamilyAvx , 86 , 7785, 419),
  INST(Vpscatterqd     , "vpscatterqd"     , Enc(VexMr_VM)          , V(660F38,A1,_,x,_,0,2,T1S), 0                         , F(WO)|F(VM)|F(Evex)                   , EF(________), 0 , 0 , kFamilyAvx , 86 , 7797, 420),
  INST(Vpscatterqq     , "vpscatterqq"     , Enc(VexMr_VM)          , V(660F38,A1,_,x,_,1,3,T1S), 0                         , F(WO)|F(VM)|F(Evex)                   , EF(________), 0 , 0 , kFamilyAvx , 86 , 7809, 421),
  INST(Vpshab          , "vpshab"          , Enc(VexRvmRmv)         , V(XOP_M9,98,_,0,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 7821, 422),
  INST(Vpshad          , "vpshad"          , Enc(VexRvmRmv)         , V(XOP_M9,9A,_,0,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 7828, 422),
  INST(Vpshaq          , "vpshaq"          , Enc(VexRvmRmv)         , V(XOP_M9,9B,_,0,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 7835, 422),
  INST(Vpshaw          , "vpshaw"          , Enc(VexRvmRmv)         , V(XOP_M9,99,_,0,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 7842, 422),
  INST(Vpshlb          , "vpshlb"          , Enc(VexRvmRmv)         , V(XOP_M9,94,_,0,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 7849, 422),
  INST(Vpshld          , "vpshld"          , Enc(VexRvmRmv)         , V(XOP_M9,96,_,0,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 7856, 422),
  INST(Vpshlq          , "vpshlq"          , Enc(VexRvmRmv)         , V(XOP_M9,97,_,0,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 7863, 422),
  INST(Vpshlw          , "vpshlw"          , Enc(VexRvmRmv)         , V(XOP_M9,95,_,0,x,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 44 , 7870, 422),
  INST(Vpshufb         , "vpshufb"         , Enc(VexRvm_Lx)         , V(660F38,00,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 54 , 7877, 266),
  INST(Vpshufd         , "vpshufd"         , Enc(VexRmi_Lx)         , V(660F00,70,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 56 , 7885, 423),
  INST(Vpshufhw        , "vpshufhw"        , Enc(VexRmi_Lx)         , V(F30F00,70,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 54 , 7893, 423),
  INST(Vpshuflw        , "vpshuflw"        , Enc(VexRmi_Lx)         , V(F20F00,70,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 54 , 7902, 423),
  INST(Vpsignb         , "vpsignb"         , Enc(VexRvm_Lx)         , V(660F38,08,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 51 , 7911, 269),
  INST(Vpsignd         , "vpsignd"         , Enc(VexRvm_Lx)         , V(660F38,0A,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 51 , 7919, 269),
  INST(Vpsignw         , "vpsignw"         , Enc(VexRvm_Lx)         , V(660F38,09,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 51 , 7927, 269),
  INST(Vpslld          , "vpslld"          , Enc(VexRvmVmi_Lx)      , V(660F00,F2,_,x,I,0,4,128), V(660F00,72,6,x,I,0,4,FV ), F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 56 , 7935, 424),
  INST(Vpslldq         , "vpslldq"         , Enc(VexEvexVmi_Lx)     , V(660F00,73,7,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 85 , 7942, 423),
  INST(Vpsllq          , "vpsllq"          , Enc(VexRvmVmi_Lx)      , V(660F00,F3,_,x,I,1,4,128), V(660F00,73,6,x,I,1,4,FV ), F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 57 , 7950, 425),
  INST(Vpsllvd         , "vpsllvd"         , Enc(VexRvm_Lx)         , V(660F38,47,_,x,0,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 72 , 7957, 266),
  INST(Vpsllvq         , "vpsllvq"         , Enc(VexRvm_Lx)         , V(660F38,47,_,x,1,1,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 74 , 7965, 266),
  INST(Vpsllvw         , "vpsllvw"         , Enc(VexRvm_Lx)         , V(660F38,12,_,x,_,1,4,FVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 9  , 7973, 274),
  INST(Vpsllw          , "vpsllw"          , Enc(VexRvmVmi_Lx)      , V(660F00,F1,_,x,I,I,4,FVM), V(660F00,71,6,x,I,I,4,FVM), F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 54 , 7981, 426),
  INST(Vpsrad          , "vpsrad"          , Enc(VexRvmVmi_Lx)      , V(660F00,E2,_,x,I,0,4,128), V(660F00,72,4,x,I,0,4,FV ), F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 56 , 7988, 427),
  INST(Vpsraq          , "vpsraq"          , Enc(VexRvmVmi_Lx)      , V(660F00,E2,_,x,_,1,4,128), V(660F00,72,4,x,_,1,4,FV ), F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 6  , 7995, 428),
  INST(Vpsravd         , "vpsravd"         , Enc(VexRvm_Lx)         , V(660F38,46,_,x,0,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 72 , 8002, 266),
  INST(Vpsravq         , "vpsravq"         , Enc(VexRvm_Lx)         , V(660F38,46,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 6  , 8010, 274),
  INST(Vpsravw         , "vpsravw"         , Enc(VexRvm_Lx)         , V(660F38,11,_,x,_,1,4,FVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 9  , 8018, 274),
  INST(Vpsraw          , "vpsraw"          , Enc(VexRvmVmi_Lx)      , V(660F00,E1,_,x,I,I,4,128), V(660F00,71,4,x,I,I,4,FVM), F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 54 , 8026, 429),
  INST(Vpsrld          , "vpsrld"          , Enc(VexRvmVmi_Lx)      , V(660F00,D2,_,x,I,0,4,128), V(660F00,72,2,x,I,0,4,FV ), F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 56 , 8033, 430),
  INST(Vpsrldq         , "vpsrldq"         , Enc(VexEvexVmi_Lx)     , V(660F00,73,3,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 85 , 8040, 423),
  INST(Vpsrlq          , "vpsrlq"          , Enc(VexRvmVmi_Lx)      , V(660F00,D3,_,x,I,1,4,128), V(660F00,73,2,x,I,1,4,FV ), F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 57 , 8048, 431),
  INST(Vpsrlvd         , "vpsrlvd"         , Enc(VexRvm_Lx)         , V(660F38,45,_,x,0,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 72 , 8055, 266),
  INST(Vpsrlvq         , "vpsrlvq"         , Enc(VexRvm_Lx)         , V(660F38,45,_,x,1,1,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 74 , 8063, 266),
  INST(Vpsrlvw         , "vpsrlvw"         , Enc(VexRvm_Lx)         , V(660F38,10,_,x,_,1,4,FVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 9  , 8071, 274),
  INST(Vpsrlw          , "vpsrlw"          , Enc(VexRvmVmi_Lx)      , V(660F00,D1,_,x,I,I,4,128), V(660F00,71,2,x,I,I,4,FVM), F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 54 , 8079, 432),
  INST(Vpsubb          , "vpsubb"          , Enc(VexRvm_Lx)         , V(660F00,F8,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 54 , 8086, 266),
  INST(Vpsubd          , "vpsubd"          , Enc(VexRvm_Lx)         , V(660F00,FA,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 56 , 8093, 266),
  INST(Vpsubq          , "vpsubq"          , Enc(VexRvm_Lx)         , V(660F00,FB,_,x,I,1,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 57 , 8100, 266),
  INST(Vpsubsb         , "vpsubsb"         , Enc(VexRvm_Lx)         , V(660F00,E8,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 54 , 8107, 266),
  INST(Vpsubsw         , "vpsubsw"         , Enc(VexRvm_Lx)         , V(660F00,E9,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 54 , 8115, 266),
  INST(Vpsubusb        , "vpsubusb"        , Enc(VexRvm_Lx)         , V(660F00,D8,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 54 , 8123, 266),
  INST(Vpsubusw        , "vpsubusw"        , Enc(VexRvm_Lx)         , V(660F00,D9,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 54 , 8132, 266),
  INST(Vpsubw          , "vpsubw"          , Enc(VexRvm_Lx)         , V(660F00,F9,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 54 , 8141, 266),
  INST(Vpternlogd      , "vpternlogd"      , Enc(VexRvmi_Lx)        , V(660F3A,25,_,x,_,0,4,FV ), 0                         , F(RW)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 5  , 8148, 310),
  INST(Vpternlogq      , "vpternlogq"      , Enc(VexRvmi_Lx)        , V(660F3A,25,_,x,_,1,4,FV ), 0                         , F(RW)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 6  , 8159, 310),
  INST(Vptest          , "vptest"          , Enc(VexRm_Lx)          , V(660F38,17,_,x,I,_,_,_  ), 0                         , F(RO)|F(Vex)                          , EF(WWWWWW__), 0 , 0 , kFamilyAvx , 3  , 8170, 433),
  INST(Vptestmb        , "vptestmb"        , Enc(VexRvm_Lx)         , V(660F38,26,_,x,_,0,4,FVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 63 , 8177, 434),
  INST(Vptestmd        , "vptestmd"        , Enc(VexRvm_Lx)         , V(660F38,27,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 64 , 8186, 434),
  INST(Vptestmq        , "vptestmq"        , Enc(VexRvm_Lx)         , V(660F38,27,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 68 , 8195, 434),
  INST(Vptestmw        , "vptestmw"        , Enc(VexRvm_Lx)         , V(660F38,26,_,x,_,1,4,FVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 63 , 8204, 434),
  INST(Vptestnmb       , "vptestnmb"       , Enc(VexRvm_Lx)         , V(F30F38,26,_,x,_,0,4,FVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 63 , 8213, 434),
  INST(Vptestnmd       , "vptestnmd"       , Enc(VexRvm_Lx)         , V(F30F38,27,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 64 , 8223, 434),
  INST(Vptestnmq       , "vptestnmq"       , Enc(VexRvm_Lx)         , V(F30F38,27,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 68 , 8233, 434),
  INST(Vptestnmw       , "vptestnmw"       , Enc(VexRvm_Lx)         , V(F30F38,26,_,x,_,1,4,FVM), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 63 , 8243, 434),
  INST(Vpunpckhbw      , "vpunpckhbw"      , Enc(VexRvm_Lx)         , V(660F00,68,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 54 , 8253, 266),
  INST(Vpunpckhdq      , "vpunpckhdq"      , Enc(VexRvm_Lx)         , V(660F00,6A,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 56 , 8264, 266),
  INST(Vpunpckhqdq     , "vpunpckhqdq"     , Enc(VexRvm_Lx)         , V(660F00,6D,_,x,I,1,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 57 , 8275, 266),
  INST(Vpunpckhwd      , "vpunpckhwd"      , Enc(VexRvm_Lx)         , V(660F00,69,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 54 , 8287, 266),
  INST(Vpunpcklbw      , "vpunpcklbw"      , Enc(VexRvm_Lx)         , V(660F00,60,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 54 , 8298, 266),
  INST(Vpunpckldq      , "vpunpckldq"      , Enc(VexRvm_Lx)         , V(660F00,62,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 56 , 8309, 266),
  INST(Vpunpcklqdq     , "vpunpcklqdq"     , Enc(VexRvm_Lx)         , V(660F00,6C,_,x,I,1,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 57 , 8320, 266),
  INST(Vpunpcklwd      , "vpunpcklwd"      , Enc(VexRvm_Lx)         , V(660F00,61,_,x,I,I,4,FVM), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 54 , 8332, 266),
  INST(Vpxor           , "vpxor"           , Enc(VexRvm_Lx)         , V(660F00,EF,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 51 , 8343, 269),
  INST(Vpxord          , "vpxord"          , Enc(VexRvm_Lx)         , V(660F00,EF,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 5  , 8349, 274),
  INST(Vpxorq          , "vpxorq"          , Enc(VexRvm_Lx)         , V(660F00,EF,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 6  , 8356, 274),
  INST(Vrangepd        , "vrangepd"        , Enc(VexRvmi_Lx)        , V(660F3A,50,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 29 , 8363, 273),
  INST(Vrangeps        , "vrangeps"        , Enc(VexRvmi_Lx)        , V(660F3A,50,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 30 , 8372, 273),
  INST(Vrangesd        , "vrangesd"        , Enc(VexRvmi)           , V(660F3A,51,_,I,_,1,3,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 87 , 8381, 435),
  INST(Vrangess        , "vrangess"        , Enc(VexRvmi)           , V(660F3A,51,_,I,_,0,2,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 87 , 8390, 436),
  INST(Vrcp14pd        , "vrcp14pd"        , Enc(VexRm_Lx)          , V(660F38,4C,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 6  , 8399, 295),
  INST(Vrcp14ps        , "vrcp14ps"        , Enc(VexRm_Lx)          , V(660F38,4C,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 5  , 8408, 295),
  INST(Vrcp14sd        , "vrcp14sd"        , Enc(VexRvm)            , V(660F38,4D,_,I,_,1,3,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 11 , 8417, 437),
  INST(Vrcp14ss        , "vrcp14ss"        , Enc(VexRvm)            , V(660F38,4D,_,I,_,0,2,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 11 , 8426, 438),
  INST(Vrcp28pd        , "vrcp28pd"        , Enc(VexRm)             , V(660F38,CA,_,2,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 33 , 8435, 305),
  INST(Vrcp28ps        , "vrcp28ps"        , Enc(VexRm)             , V(660F38,CA,_,2,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 34 , 8444, 305),
  INST(Vrcp28sd        , "vrcp28sd"        , Enc(VexRvm)            , V(660F38,CB,_,I,_,1,3,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 88 , 8453, 437),
  INST(Vrcp28ss        , "vrcp28ss"        , Enc(VexRvm)            , V(660F38,CB,_,I,_,0,2,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 88 , 8462, 438),
  INST(Vrcpps          , "vrcpps"          , Enc(VexRm_Lx)          , V(000F00,53,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 3  , 8471, 322),
  INST(Vrcpss          , "vrcpss"          , Enc(VexRvm)            , V(F30F00,53,_,I,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 3  , 8478, 439),
  INST(Vreducepd       , "vreducepd"       , Enc(VexRmi_Lx)         , V(660F3A,56,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 83 , 8485, 334),
  INST(Vreduceps       , "vreduceps"       , Enc(VexRmi_Lx)         , V(660F3A,56,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 89 , 8495, 334),
  INST(Vreducesd       , "vreducesd"       , Enc(VexRvmi)           , V(660F3A,57,_,I,_,1,3,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 12 , 8505, 435),
  INST(Vreducess       , "vreducess"       , Enc(VexRvmi)           , V(660F3A,57,_,I,_,0,2,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 12 , 8515, 436),
  INST(Vrndscalepd     , "vrndscalepd"     , Enc(VexRmi_Lx)         , V(660F3A,09,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 28 , 8525, 334),
  INST(Vrndscaleps     , "vrndscaleps"     , Enc(VexRmi_Lx)         , V(660F3A,08,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 31 , 8537, 334),
  INST(Vrndscalesd     , "vrndscalesd"     , Enc(VexRvmi)           , V(660F3A,0B,_,I,_,1,3,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 36 , 8549, 435),
  INST(Vrndscaless     , "vrndscaless"     , Enc(VexRvmi)           , V(660F3A,0A,_,I,_,0,2,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 36 , 8561, 436),
  INST(Vroundpd        , "vroundpd"        , Enc(VexRmi_Lx)         , V(660F3A,09,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 3  , 8573, 440),
  INST(Vroundps        , "vroundps"        , Enc(VexRmi_Lx)         , V(660F3A,08,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 3  , 8582, 440),
  INST(Vroundsd        , "vroundsd"        , Enc(VexRvmi)           , V(660F3A,0B,_,I,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 3  , 8591, 441),
  INST(Vroundss        , "vroundss"        , Enc(VexRvmi)           , V(660F3A,0A,_,I,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 3  , 8600, 442),
  INST(Vrsqrt14pd      , "vrsqrt14pd"      , Enc(VexRm_Lx)          , V(660F38,4E,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 6  , 8609, 295),
  INST(Vrsqrt14ps      , "vrsqrt14ps"      , Enc(VexRm_Lx)          , V(660F38,4E,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 5  , 8620, 295),
  INST(Vrsqrt14sd      , "vrsqrt14sd"      , Enc(VexRvm)            , V(660F38,4F,_,I,_,1,3,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 11 , 8631, 437),
  INST(Vrsqrt14ss      , "vrsqrt14ss"      , Enc(VexRvm)            , V(660F38,4F,_,I,_,0,2,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 11 , 8642, 438),
  INST(Vrsqrt28pd      , "vrsqrt28pd"      , Enc(VexRm)             , V(660F38,CC,_,2,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 33 , 8653, 305),
  INST(Vrsqrt28ps      , "vrsqrt28ps"      , Enc(VexRm)             , V(660F38,CC,_,2,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 34 , 8664, 305),
  INST(Vrsqrt28sd      , "vrsqrt28sd"      , Enc(VexRvm)            , V(660F38,CD,_,I,_,1,3,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 88 , 8675, 437),
  INST(Vrsqrt28ss      , "vrsqrt28ss"      , Enc(VexRvm)            , V(660F38,CD,_,I,_,0,2,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 88 , 8686, 438),
  INST(Vrsqrtps        , "vrsqrtps"        , Enc(VexRm_Lx)          , V(000F00,52,_,x,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 3  , 8697, 322),
  INST(Vrsqrtss        , "vrsqrtss"        , Enc(VexRvm)            , V(F30F00,52,_,I,I,_,_,_  ), 0                         , F(WO)|F(Vex)                          , EF(________), 0 , 0 , kFamilyAvx , 3  , 8706, 439),
  INST(Vscalefpd       , "vscalefpd"       , Enc(VexRvm_Lx)         , V(660F38,2C,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 22 , 8715, 274),
  INST(Vscalefps       , "vscalefps"       , Enc(VexRvm_Lx)         , V(660F38,2C,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 25 , 8725, 274),
  INST(Vscalefsd       , "vscalefsd"       , Enc(VexRvm)            , V(660F38,2D,_,I,_,1,3,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 90 , 8735, 437),
  INST(Vscalefss       , "vscalefss"       , Enc(VexRvm)            , V(660F38,2D,_,I,_,0,2,T1S), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 90 , 8745, 438),
  INST(Vscatterdpd     , "vscatterdpd"     , Enc(VexMr_Lx)          , V(660F38,A2,_,x,_,1,3,T1S), 0                         , F(WO)|F(VM)|F(Evex)                   , EF(________), 0 , 0 , kFamilyAvx , 86 , 8755, 443),
  INST(Vscatterdps     , "vscatterdps"     , Enc(VexMr_Lx)          , V(660F38,A2,_,x,_,0,2,T1S), 0                         , F(WO)|F(VM)|F(Evex)                   , EF(________), 0 , 0 , kFamilyAvx , 86 , 8767, 419),
  INST(Vscatterpf0dpd  , "vscatterpf0dpd"  , Enc(VexM_VM)           , V(660F38,C6,5,2,_,1,3,T1S), 0                         , F(RO)|F(VM)|F(Evex)                   , EF(________), 0 , 0 , kFamilyNone, 0  , 8779, 327),
  INST(Vscatterpf0dps  , "vscatterpf0dps"  , Enc(VexM_VM)           , V(660F38,C6,5,2,_,0,2,T1S), 0                         , F(RO)|F(VM)|F(Evex)                   , EF(________), 0 , 0 , kFamilyNone, 0  , 8794, 328),
  INST(Vscatterpf0qpd  , "vscatterpf0qpd"  , Enc(VexM_VM)           , V(660F38,C7,5,2,_,1,3,T1S), 0                         , F(RO)|F(VM)|F(Evex)                   , EF(________), 0 , 0 , kFamilyNone, 0  , 8809, 329),
  INST(Vscatterpf0qps  , "vscatterpf0qps"  , Enc(VexM_VM)           , V(660F38,C7,5,2,_,0,2,T1S), 0                         , F(RO)|F(VM)|F(Evex)                   , EF(________), 0 , 0 , kFamilyNone, 0  , 8824, 329),
  INST(Vscatterpf1dpd  , "vscatterpf1dpd"  , Enc(VexM_VM)           , V(660F38,C6,6,2,_,1,3,T1S), 0                         , F(RO)|F(VM)|F(Evex)                   , EF(________), 0 , 0 , kFamilyNone, 0  , 8839, 327),
  INST(Vscatterpf1dps  , "vscatterpf1dps"  , Enc(VexM_VM)           , V(660F38,C6,6,2,_,0,2,T1S), 0                         , F(RO)|F(VM)|F(Evex)                   , EF(________), 0 , 0 , kFamilyNone, 0  , 8854, 328),
  INST(Vscatterpf1qpd  , "vscatterpf1qpd"  , Enc(VexM_VM)           , V(660F38,C7,6,2,_,1,3,T1S), 0                         , F(RO)|F(VM)|F(Evex)                   , EF(________), 0 , 0 , kFamilyNone, 0  , 8869, 329),
  INST(Vscatterpf1qps  , "vscatterpf1qps"  , Enc(VexM_VM)           , V(660F38,C7,6,2,_,0,2,T1S), 0                         , F(RO)|F(VM)|F(Evex)                   , EF(________), 0 , 0 , kFamilyNone, 0  , 8884, 329),
  INST(Vscatterqpd     , "vscatterqpd"     , Enc(VexMr_Lx)          , V(660F38,A3,_,x,_,1,3,T1S), 0                         , F(WO)|F(VM)|F(Evex)                   , EF(________), 0 , 0 , kFamilyAvx , 86 , 8899, 421),
  INST(Vscatterqps     , "vscatterqps"     , Enc(VexMr_Lx)          , V(660F38,A3,_,x,_,0,2,T1S), 0                         , F(WO)|F(VM)|F(Evex)                   , EF(________), 0 , 0 , kFamilyAvx , 86 , 8911, 420),
  INST(Vshuff32x4      , "vshuff32x4"      , Enc(VexRvmi_Lx)        , V(660F3A,23,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 5  , 8923, 444),
  INST(Vshuff64x2      , "vshuff64x2"      , Enc(VexRvmi_Lx)        , V(660F3A,23,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 6  , 8934, 444),
  INST(Vshufi32x4      , "vshufi32x4"      , Enc(VexRvmi_Lx)        , V(660F3A,43,_,x,_,0,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 5  , 8945, 444),
  INST(Vshufi64x2      , "vshufi64x2"      , Enc(VexRvmi_Lx)        , V(660F3A,43,_,x,_,1,4,FV ), 0                         , F(WO)|F(Evex)                         , EF(________), 0 , 0 , kFamilyAvx , 6  , 8956, 444),
  INST(Vshufpd         , "vshufpd"         , Enc(VexRvmi_Lx)        , V(660F00,C6,_,x,I,1,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 20 , 8967, 371),
  INST(Vshufps         , "vshufps"         , Enc(VexRvmi_Lx)        , V(000F00,C6,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 73 , 8975, 371),
  INST(Vsqrtpd         , "vsqrtpd"         , Enc(VexRm_Lx)          , V(660F00,51,_,x,I,1,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 0  , 8983, 292),
  INST(Vsqrtps         , "vsqrtps"         , Enc(VexRm_Lx)          , V(000F00,51,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 1  , 8991, 292),
  INST(Vsqrtsd         , "vsqrtsd"         , Enc(VexRvm)            , V(F20F00,51,_,I,I,1,3,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 2  , 8999, 267),
  INST(Vsqrtss         , "vsqrtss"         , Enc(VexRvm)            , V(F30F00,51,_,I,I,0,2,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 2  , 9007, 268),
  INST(Vstmxcsr        , "vstmxcsr"        , Enc(VexM)              , V(000F00,AE,3,0,I,_,_,_  ), 0                         , F(Vex)|F(Volatile)                    , EF(________), 0 , 0 , kFamilyNone, 0  , 9015, 445),
  INST(Vsubpd          , "vsubpd"          , Enc(VexRvm_Lx)         , V(660F00,5C,_,x,I,1,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 52 , 9024, 266),
  INST(Vsubps          , "vsubps"          , Enc(VexRvm_Lx)         , V(000F00,5C,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 53 , 9031, 266),
  INST(Vsubsd          , "vsubsd"          , Enc(VexRvm)            , V(F20F00,5C,_,I,I,1,3,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 2  , 9038, 267),
  INST(Vsubss          , "vsubss"          , Enc(VexRvm)            , V(F30F00,5C,_,I,I,0,2,T1S), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 2  , 9045, 268),
  INST(Vtestpd         , "vtestpd"         , Enc(VexRm_Lx)          , V(660F38,0F,_,x,0,_,_,_  ), 0                         , F(RO)|F(Vex)                          , EF(WWWWWW__), 0 , 0 , kFamilyAvx , 3  , 9052, 433),
  INST(Vtestps         , "vtestps"         , Enc(VexRm_Lx)          , V(660F38,0E,_,x,0,_,_,_  ), 0                         , F(RO)|F(Vex)                          , EF(WWWWWW__), 0 , 0 , kFamilyAvx , 3  , 9060, 433),
  INST(Vucomisd        , "vucomisd"        , Enc(VexRm)             , V(660F00,2E,_,I,I,1,3,T1S), 0                         , F(RO)|F(Vex)|F(Evex)                  , EF(WWWWWW__), 0 , 0 , kFamilyAvx , 19 , 9068, 288),
  INST(Vucomiss        , "vucomiss"        , Enc(VexRm)             , V(000F00,2E,_,I,I,0,2,T1S), 0                         , F(RO)|F(Vex)|F(Evex)                  , EF(WWWWWW__), 0 , 0 , kFamilyAvx , 19 , 9077, 289),
  INST(Vunpckhpd       , "vunpckhpd"       , Enc(VexRvm_Lx)         , V(660F00,15,_,x,I,1,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 73 , 9086, 266),
  INST(Vunpckhps       , "vunpckhps"       , Enc(VexRvm_Lx)         , V(000F00,15,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 20 , 9096, 266),
  INST(Vunpcklpd       , "vunpcklpd"       , Enc(VexRvm_Lx)         , V(660F00,14,_,x,I,1,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 73 , 9106, 266),
  INST(Vunpcklps       , "vunpcklps"       , Enc(VexRvm_Lx)         , V(000F00,14,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 20 , 9116, 266),
  INST(Vxorpd          , "vxorpd"          , Enc(VexRvm_Lx)         , V(660F00,57,_,x,I,1,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 7  , 9126, 266),
  INST(Vxorps          , "vxorps"          , Enc(VexRvm_Lx)         , V(000F00,57,_,x,I,0,4,FV ), 0                         , F(WO)|F(Vex)|F(Evex)                  , EF(________), 0 , 0 , kFamilyAvx , 8  , 9133, 266),
  INST(Vzeroall        , "vzeroall"        , Enc(VexOp)             , V(000F00,77,_,1,I,_,_,_  ), 0                         , F(Vex)|F(Volatile)                    , EF(________), 0 , 0 , kFamilyNone, 0  , 9140, 446),
  INST(Vzeroupper      , "vzeroupper"      , Enc(VexOp)             , V(000F00,77,_,0,I,_,_,_  ), 0                         , F(Vex)|F(Volatile)                    , EF(________), 0 , 0 , kFamilyNone, 0  , 9149, 446),
  INST(Wrfsbase        , "wrfsbase"        , Enc(X86M)              , O(F30F00,AE,2,_,x,_,_,_  ), 0                         , F(RO)|F(Volatile)                     , EF(________), 0 , 0 , kFamilyNone, 0  , 9160, 447),
  INST(Wrgsbase        , "wrgsbase"        , Enc(X86M)              , O(F30F00,AE,3,_,x,_,_,_  ), 0                         , F(RO)|F(Volatile)                     , EF(________), 0 , 0 , kFamilyNone, 0  , 9169, 447),
  INST(Xadd            , "xadd"            , Enc(X86Xadd)           , O(000F00,C0,_,_,x,_,_,_  ), 0                         , F(RW)|F(Xchg)|F(Lock)                 , EF(WWWWWW__), 0 , 0 , kFamilyNone, 0  , 9178, 448),
  INST(Xchg            , "xchg"            , Enc(X86Xchg)           , O(000000,86,_,_,x,_,_,_  ), 0                         , F(RW)|F(Xchg)|F(Lock)                 , EF(________), 0 , 0 , kFamilyNone, 0  , 374 , 449),
  INST(Xgetbv          , "xgetbv"          , Enc(X86Op)             , O(000F01,D0,_,_,_,_,_,_  ), 0                         , F(WO)|F(Special)                      , EF(________), 0 , 0 , kFamilyNone, 0  , 9183, 450),
  INST(Xor             , "xor"             , Enc(X86Arith)          , O(000000,30,6,_,x,_,_,_  ), 0                         , F(RW)|F(Lock)                         , EF(WWWUWW__), 0 , 0 , kFamilyNone, 0  , 8345, 5  ),
  INST(Xorpd           , "xorpd"           , Enc(ExtRm)             , O(660F00,57,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 145, 9127, 6  ),
  INST(Xorps           , "xorps"           , Enc(ExtRm)             , O(000F00,57,_,_,_,_,_,_  ), 0                         , F(RW)                                 , EF(________), 0 , 0 , kFamilySse , 146, 9134, 6  ),
  INST(Xrstor          , "xrstor"          , Enc(X86M_Only)         , O(000F00,AE,5,_,_,_,_,_  ), 0                         , F(RO)|F(Volatile)|F(Special)          , EF(________), 0 , 0 , kFamilyNone, 0  , 1051, 451),
  INST(Xrstor64        , "xrstor64"        , Enc(X86M_Only)         , O(000F00,AE,5,_,1,_,_,_  ), 0                         , F(RO)|F(Volatile)|F(Special)          , EF(________), 0 , 0 , kFamilyNone, 0  , 1059, 452),
  INST(Xrstors         , "xrstors"         , Enc(X86M_Only)         , O(000F00,C7,3,_,_,_,_,_  ), 0                         , F(RO)|F(Volatile)|F(Special)          , EF(________), 0 , 0 , kFamilyNone, 0  , 9190, 451),
  INST(Xrstors64       , "xrstors64"       , Enc(X86M_Only)         , O(000F00,C7,3,_,1,_,_,_  ), 0                         , F(RO)|F(Volatile)|F(Special)          , EF(________), 0 , 0 , kFamilyNone, 0  , 9198, 452),
  INST(Xsave           , "xsave"           , Enc(X86M_Only)         , O(000F00,AE,4,_,_,_,_,_  ), 0                         , F(WO)|F(Volatile)|F(Special)          , EF(________), 0 , 0 , kFamilyNone, 0  , 1069, 453),
  INST(Xsave64         , "xsave64"         , Enc(X86M_Only)         , O(000F00,AE,4,_,1,_,_,_  ), 0                         , F(WO)|F(Volatile)|F(Special)          , EF(________), 0 , 0 , kFamilyNone, 0  , 1076, 454),
  INST(Xsavec          , "xsavec"          , Enc(X86M_Only)         , O(000F00,C7,4,_,_,_,_,_  ), 0                         , F(WO)|F(Volatile)|F(Special)          , EF(________), 0 , 0 , kFamilyNone, 0  , 9208, 453),
  INST(Xsavec64        , "xsavec64"        , Enc(X86M_Only)         , O(000F00,C7,4,_,1,_,_,_  ), 0                         , F(WO)|F(Volatile)|F(Special)          , EF(________), 0 , 0 , kFamilyNone, 0  , 9215, 454),
  INST(Xsaveopt        , "xsaveopt"        , Enc(X86M_Only)         , O(000F00,AE,6,_,_,_,_,_  ), 0                         , F(WO)|F(Volatile)|F(Special)          , EF(________), 0 , 0 , kFamilyNone, 0  , 9224, 453),
  INST(Xsaveopt64      , "xsaveopt64"      , Enc(X86M_Only)         , O(000F00,AE,6,_,1,_,_,_  ), 0                         , F(WO)|F(Volatile)|F(Special)          , EF(________), 0 , 0 , kFamilyNone, 0  , 9233, 454),
  INST(Xsaves          , "xsaves"          , Enc(X86M_Only)         , O(000F00,C7,5,_,_,_,_,_  ), 0                         , F(WO)|F(Volatile)|F(Special)          , EF(________), 0 , 0 , kFamilyNone, 0  , 9244, 453),
  INST(Xsaves64        , "xsaves64"        , Enc(X86M_Only)         , O(000F00,C7,5,_,1,_,_,_  ), 0                         , F(WO)|F(Volatile)|F(Special)          , EF(________), 0 , 0 , kFamilyNone, 0  , 9251, 454),
  INST(Xsetbv          , "xsetbv"          , Enc(X86Op)             , O(000F01,D1,_,_,_,_,_,_  ), 0                         , F(RO)|F(Volatile)|F(Special)          , EF(________), 0 , 0 , kFamilyNone, 0  , 9260, 455)
  // ${instData:End}
};

#undef NAME_DATA_INDEX
#undef INST

// ${commonData:Begin}
// ------------------- Automatically generated, do not edit -------------------
const X86Inst::CommonData X86InstDB::commonData[] = {
  { 0                                     , 0  , 0  , 0x00, 0x00, 0  , 0  , 0 , 0 }, // #0
  { F(RW)                                 , 0  , 0  , 0x00, 0x3F, 0  , 349, 1 , 0 }, // #1
  { F(RW)                                 , 0  , 0  , 0x00, 0x3F, 0  , 350, 1 , 0 }, // #2
  { F(RW)|F(Lock)                         , 0  , 0  , 0x20, 0x3F, 0  , 14 , 10, 0 }, // #3
  { F(RW)                                 , 0  , 0  , 0x20, 0x20, 0  , 22 , 2 , 0 }, // #4
  { F(RW)|F(Lock)                         , 0  , 0  , 0x00, 0x3F, 0  , 14 , 10, 0 }, // #5
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 0  , 296, 1 , 0 }, // #6
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 0  , 351, 1 , 0 }, // #7
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 0  , 352, 1 , 0 }, // #8
  { F(RW)                                 , 0  , 0  , 0x01, 0x01, 0  , 22 , 2 , 0 }, // #9
  { F(WO)                                 , 0  , 0  , 0x00, 0x00, 0  , 64 , 1 , 0 }, // #10
  { F(WO)                                 , 0  , 0  , 0x00, 0x00, 0  , 71 , 1 , 0 }, // #11
  { F(RW)                                 , 0  , 0  , 0x00, 0x3F, 0  , 251, 2 , 0 }, // #12
  { F(RW)                                 , 0  , 0  , 0x00, 0x3F, 0  , 253, 2 , 0 }, // #13
  { F(WO)                                 , 0  , 0  , 0x00, 0x3F, 0  , 162, 2 , 0 }, // #14
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 0  , 298, 1 , 0 }, // #15
  { F(RW)|F(Special)                      , 0  , 0  , 0x00, 0x00, 0  , 353, 1 , 0 }, // #16
  { F(RW)                                 , 0  , 0  , 0x00, 0x3F, 0  , 162, 2 , 0 }, // #17
  { F(RW)                                 , 0  , 0  , 0x00, 0x3F, 0  , 21 , 3 , 0 }, // #18
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 0  , 354, 1 , 0 }, // #19
  { F(RO)                                 , 0  , 0  , 0x00, 0x3B, 1  , 152, 3 , 0 }, // #20
  { F(RW)|F(Lock)                         , 0  , 0  , 0x00, 0x3B, 2  , 155, 3 , 0 }, // #21
  { F(RW)|F(Lock)                         , 0  , 0  , 0x00, 0x3B, 3  , 155, 3 , 0 }, // #22
  { F(RW)|F(Lock)                         , 0  , 0  , 0x00, 0x3B, 4  , 155, 3 , 0 }, // #23
  { F(RW)|F(Flow)|F(Volatile)             , 0  , 0  , 0x00, 0x00, 0  , 255, 2 , 0 }, // #24
  { F(RW)|F(Special)                      , 0  , 0  , 0x00, 0x00, 0  , 355, 1 , 0 }, // #25
  { F(RW)|F(Special)                      , 0  , 0  , 0x00, 0x00, 0  , 356, 1 , 0 }, // #26
  { F(RW)|F(Special)                      , 0  , 0  , 0x00, 0x00, 0  , 357, 1 , 0 }, // #27
  { F(Volatile)                           , 0  , 0  , 0x00, 0x08, 0  , 263, 1 , 0 }, // #28
  { F(Volatile)                           , 0  , 0  , 0x00, 0x20, 0  , 263, 1 , 0 }, // #29
  { F(Volatile)                           , 0  , 0  , 0x00, 0x40, 0  , 263, 1 , 0 }, // #30
  { F(RO)|F(Volatile)                     , 0  , 0  , 0x00, 0x00, 0  , 358, 1 , 0 }, // #31
  { F(WO)|F(Volatile)|F(Special)          , 0  , 0  , 0x00, 0x00, 0  , 359, 1 , 0 }, // #32
  { 0                                     , 0  , 0  , 0x20, 0x20, 0  , 263, 1 , 0 }, // #33
  { F(RW)                                 , 0  , 0  , 0x24, 0x00, 0  , 21 , 3 , 0 }, // #34
  { F(RW)                                 , 0  , 0  , 0x20, 0x00, 0  , 21 , 3 , 0 }, // #35
  { F(RW)                                 , 0  , 0  , 0x04, 0x00, 0  , 21 , 3 , 0 }, // #36
  { F(RW)                                 , 0  , 0  , 0x07, 0x00, 0  , 21 , 3 , 0 }, // #37
  { F(RW)                                 , 0  , 0  , 0x03, 0x00, 0  , 21 , 3 , 0 }, // #38
  { F(RW)                                 , 0  , 0  , 0x01, 0x00, 0  , 21 , 3 , 0 }, // #39
  { F(RW)                                 , 0  , 0  , 0x10, 0x00, 0  , 21 , 3 , 0 }, // #40
  { F(RW)                                 , 0  , 0  , 0x02, 0x00, 0  , 21 , 3 , 0 }, // #41
  { F(RO)                                 , 0  , 0  , 0x00, 0x3F, 0  , 24 , 10, 0 }, // #42
  { F(RW)|F(Special)|F(Rep)|F(Repnz)      , 0  , 0  , 0x40, 0x3F, 0  , 360, 1 , 0 }, // #43
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 0  , 361, 1 , 0 }, // #44
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 0  , 362, 1 , 0 }, // #45
  { F(RW)|F(Lock)|F(Special)              , 0  , 0  , 0x00, 0x3F, 0  , 108, 4 , 0 }, // #46
  { F(RW)|F(Lock)|F(Special)              , 0  , 0  , 0x00, 0x04, 0  , 363, 1 , 0 }, // #47
  { F(RW)|F(Lock)|F(Special)              , 0  , 0  , 0x00, 0x04, 0  , 364, 1 , 0 }, // #48
  { F(RO)                                 , 0  , 0  , 0x00, 0x3F, 0  , 365, 1 , 0 }, // #49
  { F(RO)                                 , 0  , 0  , 0x00, 0x3F, 0  , 366, 1 , 0 }, // #50
  { F(RW)|F(Volatile)|F(Special)          , 0  , 0  , 0x00, 0x00, 0  , 367, 1 , 0 }, // #51
  { F(RW)|F(Special)                      , 0  , 0  , 0x00, 0x00, 0  , 368, 1 , 0 }, // #52
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 0  , 257, 2 , 0 }, // #53
  { F(WO)                                 , 0  , 16 , 0x00, 0x00, 0  , 62 , 1 , 0 }, // #54
  { F(WO)                                 , 0  , 16 , 0x00, 0x00, 0  , 64 , 1 , 0 }, // #55
  { F(WO)                                 , 0  , 8  , 0x00, 0x00, 0  , 369, 1 , 0 }, // #56
  { F(WO)                                 , 0  , 16 , 0x00, 0x00, 0  , 370, 1 , 0 }, // #57
  { F(WO)                                 , 0  , 8  , 0x00, 0x00, 0  , 370, 1 , 0 }, // #58
  { F(WO)                                 , 0  , 8  , 0x00, 0x00, 0  , 371, 1 , 0 }, // #59
  { F(WO)                                 , 0  , 8  , 0x00, 0x00, 0  , 372, 1 , 0 }, // #60
  { F(WO)                                 , 0  , 4  , 0x00, 0x00, 0  , 62 , 1 , 0 }, // #61
  { F(WO)                                 , 0  , 8  , 0x00, 0x00, 0  , 373, 1 , 0 }, // #62
  { F(WO)                                 , 0  , 4  , 0x00, 0x00, 0  , 373, 1 , 0 }, // #63
  { F(WO)                                 , 0  , 8  , 0x00, 0x00, 0  , 233, 1 , 0 }, // #64
  { F(WO)                                 , 0  , 8  , 0x00, 0x00, 0  , 313, 1 , 0 }, // #65
  { F(RW)|F(Special)                      , 0  , 0  , 0x00, 0x00, 0  , 374, 1 , 0 }, // #66
  { F(RW)|F(Special)                      , 0  , 0  , 0x00, 0x00, 0  , 375, 1 , 0 }, // #67
  { F(RW)|F(Special)                      , 0  , 0  , 0x28, 0x3F, 0  , 349, 1 , 0 }, // #68
  { F(RW)|F(Lock)                         , 0  , 0  , 0x00, 0x1F, 5  , 259, 2 , 0 }, // #69
  { F(RW)|F(Special)                      , 0  , 0  , 0x00, 0x3F, 0  , 112, 4 , 0 }, // #70
  { F(Volatile)                           , 0  , 0  , 0x00, 0x00, 0  , 263, 1 , 0 }, // #71
  { F(Volatile)|F(Special)                , 0  , 0  , 0x00, 0x00, 0  , 376, 1 , 0 }, // #72
  { F(WO)                                 , 0  , 8  , 0x00, 0x00, 0  , 377, 1 , 0 }, // #73
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 6  , 261, 2 , 0 }, // #74
  { F(Fp)                                 , 0  , 0  , 0x00, 0x00, 0  , 263, 1 , 0 }, // #75
  { F(Fp)|F(FPU_M4)|F(FPU_M8)             , 0  , 0  , 0x00, 0x00, 0  , 158, 3 , 0 }, // #76
  { F(Fp)                                 , 0  , 0  , 0x00, 0x00, 0  , 263, 2 , 0 }, // #77
  { F(Fp)                                 , 0  , 0  , 0x00, 0x00, 0  , 378, 1 , 0 }, // #78
  { F(Fp)                                 , 0  , 0  , 0x20, 0x00, 0  , 264, 1 , 0 }, // #79
  { F(Fp)                                 , 0  , 0  , 0x24, 0x00, 0  , 264, 1 , 0 }, // #80
  { F(Fp)                                 , 0  , 0  , 0x04, 0x00, 0  , 264, 1 , 0 }, // #81
  { F(Fp)                                 , 0  , 0  , 0x10, 0x00, 0  , 264, 1 , 0 }, // #82
  { F(Fp)                                 , 0  , 0  , 0x00, 0x00, 0  , 265, 2 , 0 }, // #83
  { F(Fp)                                 , 0  , 0  , 0x00, 0x3F, 0  , 264, 1 , 0 }, // #84
  { F(Fp)                                 , 0  , 0  , 0x00, 0x00, 0  , 264, 1 , 0 }, // #85
  { F(Fp)|F(FPU_M2)|F(FPU_M4)             , 0  , 0  , 0x00, 0x00, 0  , 379, 1 , 0 }, // #86
  { F(Fp)|F(FPU_M2)|F(FPU_M4)|F(FPU_M8)   , 0  , 0  , 0x00, 0x00, 7  , 380, 1 , 0 }, // #87
  { F(Fp)|F(FPU_M2)|F(FPU_M4)|F(FPU_M8)   , 0  , 0  , 0x00, 0x00, 8  , 380, 1 , 0 }, // #88
  { F(Fp)|F(FPU_M2)|F(FPU_M4)|F(FPU_M8)   , 0  , 0  , 0x00, 0x00, 9  , 380, 1 , 0 }, // #89
  { F(Fp)|F(FPU_M2)|F(FPU_M4)|F(FPU_M8)   , 0  , 0  , 0x00, 0x00, 10 , 381, 1 , 0 }, // #90
  { F(Fp)                                 , 0  , 0  , 0x00, 0x00, 0  , 382, 1 , 0 }, // #91
  { F(Fp)                                 , 0  , 0  , 0x00, 0x00, 0  , 383, 1 , 0 }, // #92
  { F(Fp)                                 , 0  , 0  , 0x00, 0x00, 11 , 384, 1 , 0 }, // #93
  { F(Fp)|F(FPU_M4)|F(FPU_M8)             , 0  , 0  , 0x00, 0x00, 0  , 266, 1 , 0 }, // #94
  { F(Fp)|F(FPU_M4)|F(FPU_M8)|F(FPU_M10)  , 0  , 0  , 0x00, 0x00, 12 , 381, 1 , 0 }, // #95
  { F(Fp)                                 , 0  , 0  , 0x00, 0x00, 13 , 384, 1 , 0 }, // #96
  { F(Fp)|F(Volatile)                     , 0  , 0  , 0x00, 0x00, 0  , 263, 1 , 0 }, // #97
  { F(Fp)                                 , 0  , 0  , 0x00, 0x00, 0  , 385, 1 , 0 }, // #98
  { F(RW)|F(Special)                      , 0  , 0  , 0x00, 0x3F, 0  , 116, 4 , 0 }, // #99
  { F(RW)|F(Special)                      , 0  , 0  , 0x00, 0x3F, 0  , 34 , 10, 0 }, // #100
  { F(WO)|F(Volatile)|F(Special)          , 0  , 0  , 0x00, 0x00, 14 , 386, 1 , 0 }, // #101
  { F(RW)|F(Lock)                         , 0  , 0  , 0x00, 0x1F, 15 , 259, 2 , 0 }, // #102
  { F(WO)|F(Volatile)|F(Special)|F(Rep)   , 0  , 0  , 0x00, 0x00, 0  , 387, 1 , 0 }, // #103
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 16 , 267, 2 , 0 }, // #104
  { F(Volatile)                           , 0  , 0  , 0x00, 0x88, 0  , 388, 1 , 0 }, // #105
  { F(Volatile)                           , 0  , 0  , 0x00, 0x88, 0  , 263, 1 , 0 }, // #106
  { F(Flow)|F(Volatile)                   , 0  , 0  , 0x24, 0x00, 17 , 389, 1 , 0 }, // #107
  { F(Flow)|F(Volatile)                   , 0  , 0  , 0x20, 0x00, 18 , 389, 1 , 0 }, // #108
  { F(Flow)|F(Volatile)                   , 0  , 0  , 0x20, 0x00, 19 , 389, 1 , 0 }, // #109
  { F(Flow)|F(Volatile)                   , 0  , 0  , 0x24, 0x00, 20 , 389, 1 , 0 }, // #110
  { F(Flow)|F(Volatile)                   , 0  , 0  , 0x20, 0x00, 19 , 390, 1 , 0 }, // #111
  { F(Flow)|F(Volatile)                   , 0  , 0  , 0x04, 0x00, 21 , 389, 1 , 0 }, // #112
  { F(Flow)|F(Volatile)                   , 0  , 0  , 0x07, 0x00, 22 , 389, 1 , 0 }, // #113
  { F(Flow)|F(Volatile)                   , 0  , 0  , 0x03, 0x00, 23 , 389, 1 , 0 }, // #114
  { F(Flow)|F(Volatile)                   , 0  , 0  , 0x03, 0x00, 24 , 389, 1 , 0 }, // #115
  { F(Flow)|F(Volatile)                   , 0  , 0  , 0x07, 0x00, 25 , 389, 1 , 0 }, // #116
  { F(Flow)|F(Volatile)                   , 0  , 0  , 0x20, 0x00, 18 , 390, 1 , 0 }, // #117
  { F(Flow)|F(Volatile)                   , 0  , 0  , 0x04, 0x00, 26 , 389, 1 , 0 }, // #118
  { F(Flow)|F(Volatile)                   , 0  , 0  , 0x01, 0x00, 27 , 389, 1 , 0 }, // #119
  { F(Flow)|F(Volatile)                   , 0  , 0  , 0x10, 0x00, 28 , 389, 1 , 0 }, // #120
  { F(Flow)|F(Volatile)                   , 0  , 0  , 0x02, 0x00, 29 , 389, 1 , 0 }, // #121
  { F(Flow)|F(Volatile)                   , 0  , 0  , 0x01, 0x00, 30 , 389, 1 , 0 }, // #122
  { F(Flow)|F(Volatile)                   , 0  , 0  , 0x10, 0x00, 31 , 389, 1 , 0 }, // #123
  { F(Flow)|F(Volatile)                   , 0  , 0  , 0x02, 0x00, 32 , 389, 1 , 0 }, // #124
  { F(Flow)|F(Volatile)|F(Special)        , 0  , 0  , 0x00, 0x00, 33 , 269, 2 , 0 }, // #125
  { F(Flow)|F(Volatile)                   , 0  , 0  , 0x00, 0x00, 34 , 271, 2 , 0 }, // #126
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 391, 1 , 0 }, // #127
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 35 , 273, 2 , 0 }, // #128
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 36 , 275, 2 , 0 }, // #129
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 37 , 277, 2 , 0 }, // #130
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 38 , 279, 2 , 0 }, // #131
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 392, 1 , 0 }, // #132
  { F(RO)|F(Vex)                          , 0  , 0  , 0x00, 0x3F, 0  , 393, 1 , 0 }, // #133
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 394, 1 , 0 }, // #134
  { F(RW)|F(Volatile)|F(Special)          , 0  , 0  , 0x3E, 0x00, 0  , 395, 1 , 0 }, // #135
  { F(WO)                                 , 0  , 16 , 0x00, 0x00, 0  , 206, 1 , 0 }, // #136
  { F(RO)|F(Volatile)                     , 0  , 0  , 0x00, 0x00, 0  , 396, 1 , 0 }, // #137
  { F(WO)                                 , 0  , 0  , 0x00, 0x00, 0  , 397, 1 , 0 }, // #138
  { F(Volatile)|F(Special)                , 0  , 0  , 0x00, 0x00, 0  , 263, 1 , 0 }, // #139
  { F(WO)|F(Special)|F(Rep)               , 0  , 1  , 0x40, 0x00, 0  , 398, 1 , 0 }, // #140
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 39 , 281, 2 , 0 }, // #141
  { F(RW)                                 , 0  , 0  , 0x04, 0x00, 40 , 281, 2 , 0 }, // #142
  { F(RW)                                 , 0  , 0  , 0x04, 0x00, 41 , 281, 2 , 0 }, // #143
  { F(RW)                                 , 0  , 0  , 0x00, 0x3F, 0  , 161, 3 , 0 }, // #144
  { F(RO)|F(Special)                      , 0  , 0  , 0x00, 0x00, 0  , 399, 1 , 0 }, // #145
  { F(RO)|F(Special)                      , 0  , 0  , 0x00, 0x00, 0  , 400, 1 , 0 }, // #146
  { F(RW)|F(Volatile)                     , 0  , 0  , 0x00, 0x00, 0  , 263, 1 , 0 }, // #147
  { F(RO)|F(Volatile)|F(Special)          , 0  , 0  , 0x00, 0x00, 0  , 0  , 0 , 0 }, // #148
  { F(WO)                                 , 0  , 0  , 0x00, 0x00, 0  , 0  , 14, 0 }, // #149
  { F(WO)                                 , 0  , 16 , 0x00, 0x00, 42 , 64 , 2 , 0 }, // #150
  { F(WO)                                 , 0  , 16 , 0x00, 0x00, 43 , 64 , 2 , 0 }, // #151
  { F(WO)                                 , 0  , 0  , 0x00, 0x00, 44 , 52 , 6 , 0 }, // #152
  { F(WO)                                 , 0  , 16 , 0x00, 0x00, 45 , 283, 2 , 0 }, // #153
  { F(WO)                                 , 0  , 8  , 0x00, 0x00, 0  , 401, 1 , 0 }, // #154
  { F(WO)                                 , 0  , 16 , 0x00, 0x00, 46 , 64 , 2 , 0 }, // #155
  { F(WO)                                 , 0  , 16 , 0x00, 0x00, 47 , 64 , 2 , 0 }, // #156
  { F(WO)                                 , 0  , 8  , 0x00, 0x00, 0  , 402, 1 , 0 }, // #157
  { F(RW)                                 , 8  , 8  , 0x00, 0x00, 48 , 212, 2 , 0 }, // #158
  { F(RW)                                 , 8  , 8  , 0x00, 0x00, 49 , 212, 2 , 0 }, // #159
  { F(RW)                                 , 8  , 8  , 0x00, 0x00, 0  , 402, 1 , 0 }, // #160
  { F(WO)                                 , 0  , 8  , 0x00, 0x00, 50 , 212, 2 , 0 }, // #161
  { F(WO)                                 , 0  , 8  , 0x00, 0x00, 51 , 212, 2 , 0 }, // #162
  { F(WO)                                 , 0  , 8  , 0x00, 0x00, 0  , 403, 1 , 0 }, // #163
  { F(WO)                                 , 0  , 16 , 0x00, 0x00, 52 , 203, 1 , 0 }, // #164
  { F(WO)                                 , 0  , 8  , 0x00, 0x00, 0  , 56 , 2 , 0 }, // #165
  { F(WO)                                 , 0  , 16 , 0x00, 0x00, 53 , 203, 1 , 0 }, // #166
  { F(WO)                                 , 0  , 16 , 0x00, 0x00, 54 , 203, 1 , 0 }, // #167
  { F(WO)                                 , 0  , 8  , 0x00, 0x00, 55 , 404, 1 , 0 }, // #168
  { F(WO)                                 , 0  , 8  , 0x00, 0x00, 56 , 212, 1 , 0 }, // #169
  { F(WO)                                 , 0  , 4  , 0x00, 0x00, 57 , 288, 1 , 0 }, // #170
  { F(WO)                                 , 0  , 16 , 0x00, 0x00, 58 , 58 , 6 , 0 }, // #171
  { F(WO)                                 , 0  , 16 , 0x00, 0x00, 0  , 405, 1 , 0 }, // #172
  { F(WO)|F(Special)|F(Rep)               , 0  , 0  , 0x00, 0x00, 0  , 406, 1 , 0 }, // #173
  { F(WO)|F(ZeroIfMem)                    , 0  , 8  , 0x00, 0x00, 59 , 285, 2 , 0 }, // #174
  { F(WO)|F(ZeroIfMem)                    , 0  , 4  , 0x00, 0x00, 60 , 287, 2 , 0 }, // #175
  { F(WO)                                 , 0  , 0  , 0x00, 0x00, 0  , 289, 2 , 0 }, // #176
  { F(WO)                                 , 0  , 0  , 0x00, 0x00, 0  , 407, 1 , 0 }, // #177
  { F(WO)                                 , 0  , 16 , 0x00, 0x00, 61 , 64 , 2 , 0 }, // #178
  { F(WO)                                 , 0  , 16 , 0x00, 0x00, 62 , 64 , 2 , 0 }, // #179
  { F(RW)|F(Special)                      , 0  , 0  , 0x00, 0x3F, 0  , 34 , 4 , 0 }, // #180
  { F(RW)|F(Special)                      , 0  , 0  , 0x00, 0x00, 0  , 291, 2 , 0 }, // #181
  { F(RW)|F(Lock)                         , 0  , 0  , 0x00, 0x3F, 0  , 260, 1 , 0 }, // #182
  { 0                                     , 0  , 0  , 0x00, 0x00, 0  , 293, 2 , 0 }, // #183
  { F(RW)|F(Lock)                         , 0  , 0  , 0x00, 0x00, 0  , 260, 1 , 0 }, // #184
  { F(RO)|F(Volatile)|F(Special)          , 0  , 0  , 0x00, 0x00, 63 , 408, 1 , 0 }, // #185
  { F(RO)|F(Volatile)|F(Special)|F(Rep)   , 0  , 0  , 0x00, 0x00, 0  , 409, 1 , 0 }, // #186
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 0  , 295, 2 , 0 }, // #187
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 0  , 297, 2 , 0 }, // #188
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 0  , 263, 1 , 0 }, // #189
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 0  , 295, 1 , 0 }, // #190
  { F(WO)|F(Special)                      , 0  , 0  , 0x00, 0x00, 0  , 410, 1 , 0 }, // #191
  { F(WO)|F(Special)                      , 0  , 0  , 0x00, 0x00, 0  , 411, 1 , 0 }, // #192
  { F(WO)|F(Special)                      , 0  , 0  , 0x00, 0x00, 0  , 412, 1 , 0 }, // #193
  { F(WO)|F(Special)                      , 0  , 0  , 0x00, 0x00, 0  , 413, 1 , 0 }, // #194
  { F(WO)                                 , 0  , 0  , 0x00, 0x00, 0  , 251, 2 , 0 }, // #195
  { F(WO)                                 , 0  , 8  , 0x00, 0x00, 0  , 414, 1 , 0 }, // #196
  { F(WO)                                 , 0  , 8  , 0x00, 0x00, 0  , 415, 1 , 0 }, // #197
  { F(WO)                                 , 0  , 8  , 0x00, 0x00, 64 , 299, 2 , 0 }, // #198
  { F(WO)                                 , 0  , 8  , 0x00, 0x00, 0  , 301, 1 , 0 }, // #199
  { F(WO)                                 , 0  , 0  , 0x00, 0x00, 0  , 301, 1 , 0 }, // #200
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 0  , 416, 1 , 0 }, // #201
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 0  , 417, 1 , 0 }, // #202
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 0  , 418, 1 , 0 }, // #203
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 0  , 419, 1 , 0 }, // #204
  { F(WO)                                 , 0  , 8  , 0x00, 0x00, 0  , 420, 1 , 0 }, // #205
  { F(WO)                                 , 0  , 16 , 0x00, 0x00, 0  , 233, 1 , 0 }, // #206
  { F(WO)                                 , 0  , 16 , 0x00, 0x00, 0  , 236, 1 , 0 }, // #207
  { F(WO)|F(Volatile)|F(Special)          , 0  , 0  , 0x00, 0x00, 65 , 120, 4 , 0 }, // #208
  { F(Volatile)|F(Special)                , 0  , 0  , 0x00, 0x00, 0  , 421, 1 , 0 }, // #209
  { F(WO)                                 , 0  , 0  , 0x00, 0x3F, 0  , 161, 3 , 0 }, // #210
  { F(Volatile)|F(Special)                , 0  , 0  , 0x00, 0xFF, 0  , 263, 1 , 0 }, // #211
  { F(Volatile)|F(Special)                , 0  , 0  , 0x00, 0xFF, 0  , 421, 1 , 0 }, // #212
  { F(Volatile)|F(Special)                , 0  , 0  , 0x00, 0xFF, 0  , 422, 1 , 0 }, // #213
  { F(RO)|F(Volatile)                     , 0  , 0  , 0x00, 0x3F, 0  , 358, 1 , 0 }, // #214
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 0  , 301, 2 , 0 }, // #215
  { F(WO)                                 , 0  , 16 , 0x00, 0x00, 0  , 71 , 1 , 0 }, // #216
  { F(WO)                                 , 0  , 8  , 0x00, 0x00, 0  , 423, 1 , 0 }, // #217
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 66 , 303, 2 , 0 }, // #218
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 67 , 424, 1 , 0 }, // #219
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 68 , 303, 2 , 0 }, // #220
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 69 , 303, 2 , 0 }, // #221
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 70 , 303, 2 , 0 }, // #222
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 71 , 303, 2 , 0 }, // #223
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 72 , 303, 2 , 0 }, // #224
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 73 , 424, 1 , 0 }, // #225
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 74 , 303, 2 , 0 }, // #226
  { F(RW)                                 , 0  , 0  , 0x00, 0x00, 75 , 303, 2 , 0 }, // #227
  { F(RO)                                 , 0  , 0  , 0x00, 0x3F, 0  , 345, 1 , 0 }, // #228
  { F(RO)|F(Volatile)|F(Special)          , 0  , 0  , 0x00, 0x00, 76 , 124, 4 , 0 }, // #229
  { F(Volatile)|F(Special)                , 0  , 0  , 0xFF, 0x00, 0  , 263, 1 , 0 }, // #230
  { F(Volatile)|F(Special)                , 0  , 0  , 0xFF, 0x00, 0  , 421, 1 , 0 }, // #231
  { F(Volatile)|F(Special)                , 0  , 0  , 0xFF, 0x00, 0  , 422, 1 , 0 }, // #232
  { F(RW)|F(Special)                      , 0  , 0  , 0x20, 0x21, 0  , 425, 1 , 0 }, // #233
  { F(WO)                                 , 0  , 4  , 0x00, 0x00, 0  , 233, 1 , 0 }, // #234
  { F(WO)                                 , 0  , 8  , 0x00, 0x00, 0  , 426, 1 , 0 }, // #235
  { F(WO)                                 , 0  , 8  , 0x00, 0x3F, 0  , 427, 1 , 0 }, // #236
  { F(WO)|F(Volatile)|F(Special)          , 0  , 0  , 0x00, 0x00, 0  , 428, 1 , 0 }, // #237
  { F(WO)|F(Volatile)|F(Special)          , 0  , 0  , 0x00, 0x00, 0  , 429, 1 , 0 }, // #238
  { F(RW)|F(Volatile)|F(Special)          , 0  , 0  , 0x00, 0x00, 0  , 305, 2 , 0 }, // #239
  { F(RW)|F(Special)                      , 0  , 0  , 0x00, 0x21, 0  , 425, 1 , 0 }, // #240
  { F(WO)                                 , 0  , 0  , 0x00, 0x00, 0  , 307, 2 , 0 }, // #241
  { F(WO)                                 , 0  , 8  , 0x00, 0x00, 0  , 430, 1 , 0 }, // #242
  { F(WO)                                 , 0  , 4  , 0x00, 0x00, 0  , 431, 1 , 0 }, // #243
  { F(RO)|F(Volatile)|F(Special)          , 0  , 0  , 0x00, 0x3E, 0  , 432, 1 , 0 }, // #244
  { F(RW)|F(Special)                      , 0  , 0  , 0x00, 0x3F, 0  , 425, 1 , 0 }, // #245
  { F(WO)                                 , 0  , 0  , 0x00, 0x00, 0  , 253, 2 , 0 }, // #246
  { F(RW)|F(Special)|F(Rep)|F(Repnz)      , 0  , 0  , 0x40, 0x3F, 0  , 433, 1 , 0 }, // #247
  { F(WO)                                 , 0  , 1  , 0x24, 0x00, 0  , 434, 1 , 0 }, // #248
  { F(WO)                                 , 0  , 1  , 0x20, 0x00, 0  , 434, 1 , 0 }, // #249
  { F(WO)                                 , 0  , 1  , 0x04, 0x00, 0  , 434, 1 , 0 }, // #250
  { F(WO)                                 , 0  , 1  , 0x07, 0x00, 0  , 434, 1 , 0 }, // #251
  { F(WO)                                 , 0  , 1  , 0x03, 0x00, 0  , 434, 1 , 0 }, // #252
  { F(WO)                                 , 0  , 1  , 0x01, 0x00, 0  , 434, 1 , 0 }, // #253
  { F(WO)                                 , 0  , 1  , 0x10, 0x00, 0  , 434, 1 , 0 }, // #254
  { F(WO)                                 , 0  , 1  , 0x02, 0x00, 0  , 434, 1 , 0 }, // #255
  { F(RW)|F(Special)                      , 0  , 0  , 0x00, 0x3F, 0  , 164, 3 , 0 }, // #256
  { F(WO)                                 , 0  , 8  , 0x00, 0x00, 0  , 62 , 1 , 0 }, // #257
  { 0                                     , 0  , 0  , 0x00, 0x20, 0  , 263, 1 , 0 }, // #258
  { 0                                     , 0  , 0  , 0x00, 0x40, 0  , 263, 1 , 0 }, // #259
  { 0                                     , 0  , 0  , 0x00, 0x80, 0  , 263, 1 , 0 }, // #260
  { F(Volatile)                           , 0  , 0  , 0x00, 0x00, 0  , 435, 1 , 0 }, // #261
  { F(RW)|F(Special)|F(Rep)               , 0  , 0  , 0x40, 0x00, 0  , 436, 1 , 0 }, // #262
  { 0                                     , 0  , 0  , 0x00, 0x00, 0  , 422, 1 , 0 }, // #263
  { F(RO)                                 , 0  , 0  , 0x00, 0x3F, 77 , 88 , 5 , 0 }, // #264
  { 0                                     , 0  , 0  , 0x00, 0x00, 0  , 263, 1 , 0 }, // #265
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 167, 3 , 0 }, // #266
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 437, 1 , 0 }, // #267
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 438, 1 , 0 }, // #268
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 167, 2 , 0 }, // #269
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 70 , 1 , 0 }, // #270
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 64 , 1 , 0 }, // #271
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 71 , 1 , 0 }, // #272
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 170, 3 , 0 }, // #273
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 167, 3 , 0 }, // #274
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 170, 2 , 0 }, // #275
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 309, 2 , 0 }, // #276
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 439, 1 , 0 }, // #277
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 440, 1 , 0 }, // #278
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 441, 1 , 0 }, // #279
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 442, 1 , 0 }, // #280
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 443, 1 , 0 }, // #281
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 238, 1 , 0 }, // #282
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 440, 1 , 0 }, // #283
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 329, 1 , 0 }, // #284
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 173, 3 , 0 }, // #285
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 444, 1 , 0 }, // #286
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 445, 1 , 0 }, // #287
  { F(RO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x3F, 0  , 365, 1 , 0 }, // #288
  { F(RO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x3F, 0  , 366, 1 , 0 }, // #289
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 176, 3 , 0 }, // #290
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 179, 3 , 0 }, // #291
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 182, 3 , 0 }, // #292
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 311, 2 , 0 }, // #293
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 185, 3 , 0 }, // #294
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 182, 3 , 0 }, // #295
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 311, 2 , 0 }, // #296
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 188, 3 , 0 }, // #297
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 179, 3 , 0 }, // #298
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 372, 1 , 0 }, // #299
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 372, 1 , 0 }, // #300
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 446, 1 , 0 }, // #301
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 313, 2 , 0 }, // #302
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 315, 2 , 0 }, // #303
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 446, 1 , 0 }, // #304
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 68 , 1 , 0 }, // #305
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 189, 1 , 0 }, // #306
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 447, 1 , 0 }, // #307
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 190, 1 , 0 }, // #308
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 377, 1 , 0 }, // #309
  { F(RW)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 191, 3 , 0 }, // #310
  { F(RW)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 448, 1 , 0 }, // #311
  { F(RW)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 449, 1 , 0 }, // #312
  { F(RW)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 194, 3 , 0 }, // #313
  { F(RW)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 450, 1 , 0 }, // #314
  { F(RW)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 451, 1 , 0 }, // #315
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 128, 4 , 0 }, // #316
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 317, 2 , 0 }, // #317
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 319, 2 , 0 }, // #318
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 452, 1 , 0 }, // #319
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 453, 1 , 0 }, // #320
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 454, 1 , 0 }, // #321
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 182, 2 , 0 }, // #322
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 62 , 1 , 0 }, // #323
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 233, 1 , 0 }, // #324
  { F(RW)|F(Vex_VM)|F(Evex)               , 0  , 0  , 0x00, 0x00, 78 , 93 , 5 , 0 }, // #325
  { F(RW)|F(Vex_VM)|F(Evex)               , 0  , 0  , 0x00, 0x00, 79 , 98 , 5 , 0 }, // #326
  { F(RO)|F(VM)|F(Evex)                   , 0  , 0  , 0x00, 0x00, 0  , 455, 1 , 0 }, // #327
  { F(RO)|F(VM)|F(Evex)                   , 0  , 0  , 0x00, 0x00, 0  , 456, 1 , 0 }, // #328
  { F(RO)|F(VM)|F(Evex)                   , 0  , 0  , 0x00, 0x00, 0  , 457, 1 , 0 }, // #329
  { F(RW)|F(Vex_VM)|F(Evex)               , 0  , 0  , 0x00, 0x00, 80 , 103, 5 , 0 }, // #330
  { F(RW)|F(Vex_VM)|F(Evex)               , 0  , 0  , 0x00, 0x00, 81 , 132, 4 , 0 }, // #331
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 62 , 1 , 0 }, // #332
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 233, 1 , 0 }, // #333
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 197, 3 , 0 }, // #334
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 430, 1 , 0 }, // #335
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 431, 1 , 0 }, // #336
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 321, 1 , 0 }, // #337
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 321, 2 , 0 }, // #338
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 458, 1 , 0 }, // #339
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 459, 1 , 0 }, // #340
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 206, 2 , 0 }, // #341
  { F(RO)|F(Vex)|F(Volatile)              , 0  , 0  , 0x00, 0x00, 0  , 396, 1 , 0 }, // #342
  { F(RO)|F(Vex)|F(Special)               , 0  , 0  , 0x00, 0x00, 0  , 460, 1 , 0 }, // #343
  { F(RW)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 82 , 136, 4 , 0 }, // #344
  { F(RW)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 83 , 136, 4 , 0 }, // #345
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 84 , 64 , 6 , 0 }, // #346
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 85 , 64 , 6 , 0 }, // #347
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 86 , 323, 2 , 0 }, // #348
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 200, 3 , 0 }, // #349
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 87 , 64 , 4 , 0 }, // #350
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 88 , 64 , 6 , 0 }, // #351
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 89 , 64 , 6 , 0 }, // #352
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 90 , 64 , 4 , 0 }, // #353
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 91 , 64 , 6 , 0 }, // #354
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 92 , 64 , 6 , 0 }, // #355
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 93 , 64 , 6 , 0 }, // #356
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 94 , 64 , 6 , 0 }, // #357
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 214, 1 , 0 }, // #358
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 95 , 325, 2 , 0 }, // #359
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 96 , 325, 2 , 0 }, // #360
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 97 , 325, 2 , 0 }, // #361
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 98 , 325, 2 , 0 }, // #362
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 461, 1 , 0 }, // #363
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 203, 3 , 0 }, // #364
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 206, 3 , 0 }, // #365
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 99 , 209, 3 , 0 }, // #366
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 100, 212, 3 , 0 }, // #367
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 101, 215, 3 , 0 }, // #368
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 102, 64 , 6 , 0 }, // #369
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 103, 64 , 6 , 0 }, // #370
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 170, 3 , 0 }, // #371
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 327, 2 , 0 }, // #372
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 329, 2 , 0 }, // #373
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 462, 1 , 0 }, // #374
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 463, 1 , 0 }, // #375
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 331, 2 , 0 }, // #376
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 170, 1 , 0 }, // #377
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 218, 3 , 0 }, // #378
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 221, 3 , 0 }, // #379
  { F(WO)|F(Vex)|F(Special)               , 0  , 0  , 0x00, 0x00, 0  , 410, 1 , 0 }, // #380
  { F(WO)|F(Vex)|F(Special)               , 0  , 0  , 0x00, 0x00, 0  , 411, 1 , 0 }, // #381
  { F(WO)|F(Vex)|F(Special)               , 0  , 0  , 0x00, 0x00, 0  , 412, 1 , 0 }, // #382
  { F(WO)|F(Vex)|F(Special)               , 0  , 0  , 0x00, 0x00, 0  , 413, 1 , 0 }, // #383
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 171, 1 , 0 }, // #384
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 145, 2 , 0 }, // #385
  { F(RW)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 194, 3 , 0 }, // #386
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 140, 4 , 0 }, // #387
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 104, 70 , 6 , 0 }, // #388
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 105, 70 , 6 , 0 }, // #389
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 73 , 1 , 0 }, // #390
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 72 , 1 , 0 }, // #391
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 106, 144, 4 , 0 }, // #392
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 414, 1 , 0 }, // #393
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 415, 1 , 0 }, // #394
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 300, 1 , 0 }, // #395
  { F(RW)|F(Vex_VM)|F(Evex)               , 0  , 0  , 0x00, 0x00, 107, 98 , 5 , 0 }, // #396
  { F(RW)|F(Vex_VM)|F(Evex)               , 0  , 0  , 0x00, 0x00, 108, 93 , 5 , 0 }, // #397
  { F(RW)|F(Vex_VM)|F(Evex)               , 0  , 0  , 0x00, 0x00, 109, 132, 4 , 0 }, // #398
  { F(RW)|F(Vex_VM)|F(Evex)               , 0  , 0  , 0x00, 0x00, 110, 103, 5 , 0 }, // #399
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 333, 2 , 0 }, // #400
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 335, 2 , 0 }, // #401
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 337, 2 , 0 }, // #402
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 464, 1 , 0 }, // #403
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 129, 1 , 0 }, // #404
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 111, 136, 4 , 0 }, // #405
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 112, 136, 4 , 0 }, // #406
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 465, 1 , 0 }, // #407
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 224, 3 , 0 }, // #408
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 227, 3 , 0 }, // #409
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 230, 3 , 0 }, // #410
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 233, 3 , 0 }, // #411
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 236, 3 , 0 }, // #412
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 239, 3 , 0 }, // #413
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 128, 2 , 0 }, // #414
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 113, 339, 2 , 0 }, // #415
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 114, 339, 2 , 0 }, // #416
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 115, 339, 2 , 0 }, // #417
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 116, 339, 2 , 0 }, // #418
  { F(WO)|F(VM)|F(Evex)                   , 0  , 0  , 0x00, 0x00, 0  , 242, 3 , 0 }, // #419
  { F(WO)|F(VM)|F(Evex)                   , 0  , 0  , 0x00, 0x00, 0  , 341, 2 , 0 }, // #420
  { F(WO)|F(VM)|F(Evex)                   , 0  , 0  , 0x00, 0x00, 0  , 245, 3 , 0 }, // #421
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 343, 2 , 0 }, // #422
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 0  , 197, 3 , 0 }, // #423
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 117, 76 , 6 , 0 }, // #424
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 118, 76 , 6 , 0 }, // #425
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 119, 76 , 6 , 0 }, // #426
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 120, 76 , 6 , 0 }, // #427
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 121, 82 , 6 , 0 }, // #428
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 122, 76 , 6 , 0 }, // #429
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 123, 76 , 6 , 0 }, // #430
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 124, 76 , 6 , 0 }, // #431
  { F(WO)|F(Vex)|F(Evex)                  , 0  , 0  , 0x00, 0x00, 125, 76 , 6 , 0 }, // #432
  { F(RO)|F(Vex)                          , 0  , 0  , 0x00, 0x3F, 0  , 345, 2 , 0 }, // #433
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 248, 3 , 0 }, // #434
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 466, 1 , 0 }, // #435
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 459, 1 , 0 }, // #436
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 437, 1 , 0 }, // #437
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 438, 1 , 0 }, // #438
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 438, 1 , 0 }, // #439
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 78 , 2 , 0 }, // #440
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 466, 1 , 0 }, // #441
  { F(WO)|F(Vex)                          , 0  , 0  , 0x00, 0x00, 0  , 459, 1 , 0 }, // #442
  { F(WO)|F(VM)|F(Evex)                   , 0  , 0  , 0x00, 0x00, 0  , 347, 2 , 0 }, // #443
  { F(WO)|F(Evex)                         , 0  , 0  , 0x00, 0x00, 0  , 171, 2 , 0 }, // #444
  { F(Vex)|F(Volatile)                    , 0  , 0  , 0x00, 0x00, 0  , 435, 1 , 0 }, // #445
  { F(Vex)|F(Volatile)                    , 0  , 0  , 0x00, 0x00, 0  , 263, 1 , 0 }, // #446
  { F(RO)|F(Volatile)                     , 0  , 0  , 0x00, 0x00, 0  , 467, 1 , 0 }, // #447
  { F(RW)|F(Xchg)|F(Lock)                 , 0  , 0  , 0x00, 0x3F, 0  , 148, 4 , 0 }, // #448
  { F(RW)|F(Xchg)|F(Lock)                 , 0  , 0  , 0x00, 0x00, 0  , 44 , 8 , 0 }, // #449
  { F(WO)|F(Special)                      , 0  , 0  , 0x00, 0x00, 0  , 468, 1 , 0 }, // #450
  { F(RO)|F(Volatile)|F(Special)          , 0  , 0  , 0x00, 0x00, 0  , 469, 1 , 0 }, // #451
  { F(RO)|F(Volatile)|F(Special)          , 0  , 0  , 0x00, 0x00, 0  , 470, 1 , 0 }, // #452
  { F(WO)|F(Volatile)|F(Special)          , 0  , 0  , 0x00, 0x00, 0  , 469, 1 , 0 }, // #453
  { F(WO)|F(Volatile)|F(Special)          , 0  , 0  , 0x00, 0x00, 0  , 470, 1 , 0 }, // #454
  { F(RO)|F(Volatile)|F(Special)          , 0  , 0  , 0x00, 0x00, 0  , 471, 1 , 0 }  // #455
};
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
  O(000000,7F,_,_,_,_,_,_  ), // #22
  O(000000,7D,_,_,_,_,_,_  ), // #23
  O(000000,7C,_,_,_,_,_,_  ), // #24
  O(000000,7E,_,_,_,_,_,_  ), // #25
  O(000000,75,_,_,_,_,_,_  ), // #26
  O(000000,71,_,_,_,_,_,_  ), // #27
  O(000000,7B,_,_,_,_,_,_  ), // #28
  O(000000,79,_,_,_,_,_,_  ), // #29
  O(000000,70,_,_,_,_,_,_  ), // #30
  O(000000,7A,_,_,_,_,_,_  ), // #31
  O(000000,78,_,_,_,_,_,_  ), // #32
  O(000000,E3,_,_,_,_,_,_  ), // #33
  O(000000,EB,_,_,_,_,_,_  ), // #34
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
#define CONV_MODE(MODE) X86Inst::SseData::kAvxConv##MODE
const X86Inst::SseData X86InstDB::sseData[] = {
  { FEATURE(SSE2)      , CONV_MODE(NonDestructive) , 678  }, // #0
  { FEATURE(SSE)       , CONV_MODE(NonDestructive) , 678  }, // #1
  { FEATURE(SSE3)      , CONV_MODE(NonDestructive) , 678  }, // #2
  { FEATURE(AES)       , CONV_MODE(NonDestructive) , 677  }, // #3
  { FEATURE(AES)       , CONV_MODE(Move)           , 677  }, // #4
  { FEATURE(SSE2)      , CONV_MODE(NonDestructive) , 677  }, // #5
  { FEATURE(SSE)       , CONV_MODE(NonDestructive) , 677  }, // #6
  { FEATURE(SSE4_1)    , CONV_MODE(NonDestructive) , 677  }, // #7
  { FEATURE(SSE4_1)    , CONV_MODE(Blend)          , 677  }, // #8
  { FEATURE(SSE2)      , CONV_MODE(NonDestructive) , 635  }, // #9
  { FEATURE(SSE)       , CONV_MODE(NonDestructive) , 635  }, // #10
  { FEATURE(SSE2)      , CONV_MODE(NonDestructive) , 634  }, // #11
  { FEATURE(SSE)       , CONV_MODE(NonDestructive) , 634  }, // #12
  { FEATURE(SSE2)      , CONV_MODE(Move)           , 631  }, // #13
  { FEATURE(SSE)       , CONV_MODE(Move)           , 631  }, // #14
  { FEATURE(SSE2)      , CONV_MODE(Move)           , 630  }, // #15
  { FEATURE(SSE2)      , CONV_MODE(None)           , 0    }, // #16
  { FEATURE(SSE2)      , CONV_MODE(Move)           , 629  }, // #17
  { FEATURE(SSE)       , CONV_MODE(None)           , 0    }, // #18
  { FEATURE(SSE2)      , CONV_MODE(Move)           , 636  }, // #19
  { FEATURE(SSE2)      , CONV_MODE(NonDestructive) , 636  }, // #20
  { FEATURE(SSE2)      , CONV_MODE(NonDestructive) , 637  }, // #21
  { FEATURE(SSE)       , CONV_MODE(NonDestructive) , 637  }, // #22
  { FEATURE(SSE)       , CONV_MODE(Move)           , 637  }, // #23
  { FEATURE(SSE2)      , CONV_MODE(Move)           , 638  }, // #24
  { FEATURE(SSE2)      , CONV_MODE(Move)           , 640  }, // #25
  { FEATURE(SSE2)      , CONV_MODE(Move)           , 642  }, // #26
  { FEATURE(SSE)       , CONV_MODE(Move)           , 643  }, // #27
  { FEATURE(SSE2)      , CONV_MODE(NonDestructive) , 645  }, // #28
  { FEATURE(SSE)       , CONV_MODE(NonDestructive) , 645  }, // #29
  { FEATURE(SSE4_1)    , CONV_MODE(NonDestructive) , 645  }, // #30
  { FEATURE(SSE4_1)    , CONV_MODE(Move)           , 657  }, // #31
  { FEATURE(SSE4A)     , CONV_MODE(None)           , 0    }, // #32
  { FEATURE(SSE3)      , CONV_MODE(NonDestructive) , 670  }, // #33
  { FEATURE(SSE4_1)    , CONV_MODE(NonDestructive) , 675  }, // #34
  { FEATURE(SSE3)      , CONV_MODE(Move)           , 587  }, // #35
  { FEATURE(SSE2)      , CONV_MODE(Move)           , 579  }, // #36
  { FEATURE(SSE2)      , CONV_MODE(NonDestructive) , 580  }, // #37
  { FEATURE(SSE)       , CONV_MODE(NonDestructive) , 580  }, // #38
  { FEATURE(SSE2)      , CONV_MODE(NonDestructive) , 579  }, // #39
  { FEATURE(SSE)       , CONV_MODE(NonDestructive) , 579  }, // #40
  { FEATURE(SSE2)      , CONV_MODE(Move)           , 577  }, // #41
  { FEATURE(SSE)       , CONV_MODE(Move)           , 577  }, // #42
  { FEATURE(MMX)       |
    FEATURE(SSE2)      , CONV_MODE(Move)           , 576  }, // #43
  { FEATURE(SSE3)      , CONV_MODE(Move)           , 576  }, // #44
  { FEATURE(SSE2)      , CONV_MODE(Move)           , 575  }, // #45
  { FEATURE(SSE)       , CONV_MODE(NonDestructive) , 581  }, // #46
  { FEATURE(SSE2)      , CONV_MODE(MoveIfMem)      , 581  }, // #47
  { FEATURE(SSE)       , CONV_MODE(MoveIfMem)      , 581  }, // #48
  { FEATURE(SSE2)      , CONV_MODE(Move)           , 581  }, // #49
  { FEATURE(SSE)       , CONV_MODE(Move)           , 581  }, // #50
  { FEATURE(SSE4_1)    , CONV_MODE(Move)           , 581  }, // #51
  { FEATURE(SSE2)      , CONV_MODE(Move)           , 580  }, // #52
  { FEATURE(SSE)       , CONV_MODE(Move)           , 580  }, // #53
  { FEATURE(MMX)       , CONV_MODE(None)           , 0    }, // #54
  { FEATURE(MMX)       |
    FEATURE(SSE2)      , CONV_MODE(Move)           , 577  }, // #55
  { FEATURE(SSE2)      , CONV_MODE(MoveIfMem)      , 575  }, // #56
  { FEATURE(SSE3)      , CONV_MODE(Move)           , 575  }, // #57
  { FEATURE(SSE)       , CONV_MODE(MoveIfMem)      , 575  }, // #58
  { FEATURE(SSE2)      , CONV_MODE(Move)           , 573  }, // #59
  { FEATURE(SSE)       , CONV_MODE(Move)           , 573  }, // #60
  { FEATURE(SSE4_1)    , CONV_MODE(NonDestructive) , 572  }, // #61
  { FEATURE(SSE2)      , CONV_MODE(NonDestructive) , 571  }, // #62
  { FEATURE(SSE)       , CONV_MODE(NonDestructive) , 571  }, // #63
  { FEATURE(SSE2)      , CONV_MODE(NonDestructive) , 565  }, // #64
  { FEATURE(SSE)       , CONV_MODE(NonDestructive) , 565  }, // #65
  { FEATURE(SSSE3)     , CONV_MODE(Move)           , 563  }, // #66
  { FEATURE(SSSE3)     , CONV_MODE(Move)           , 564  }, // #67
  { FEATURE(MMX)       |
    FEATURE(SSE2)      , CONV_MODE(NonDestructive) , 564  }, // #68
  { FEATURE(SSE4_1)    , CONV_MODE(NonDestructive) , 564  }, // #69
  { FEATURE(SSE2)      , CONV_MODE(NonDestructive) , 564  }, // #70
  { FEATURE(SSE3)      , CONV_MODE(NonDestructive) , 564  }, // #71
  { FEATURE(MMX)       |
    FEATURE(SSE2)      , CONV_MODE(NonDestructive) , 565  }, // #72
  { FEATURE(SSE)       |
    FEATURE(SSE2)      , CONV_MODE(NonDestructive) , 567  }, // #73
  { FEATURE(3DNOW)     , CONV_MODE(None)           , 0    }, // #74
  { FEATURE(SSE)       |
    FEATURE(SSE2)      , CONV_MODE(NonDestructive) , 566  }, // #75
  { FEATURE(SSE4_1)    , CONV_MODE(Blend)          , 567  }, // #76
  { FEATURE(SSE4_1)    , CONV_MODE(NonDestructive) , 567  }, // #77
  { FEATURE(PCLMULQDQ) , CONV_MODE(NonDestructive) , 573  }, // #78
  { FEATURE(MMX)       |
    FEATURE(SSE2)      , CONV_MODE(NonDestructive) , 576  }, // #79
  { FEATURE(SSE4_1)    , CONV_MODE(NonDestructive) , 576  }, // #80
  { FEATURE(SSE4_2)    , CONV_MODE(Move)           , 576  }, // #81
  { FEATURE(SSE4_1)    , CONV_MODE(Move)           , 617  }, // #82
  { FEATURE(SSE)       |
    FEATURE(SSE2)      |
    FEATURE(SSE4_1)    , CONV_MODE(Move)           , 617  }, // #83
  { FEATURE(3DNOW2)    , CONV_MODE(None)           , 0    }, // #84
  { FEATURE(GEODE)     , CONV_MODE(None)           , 0    }, // #85
  { FEATURE(SSSE3)     , CONV_MODE(NonDestructive) , 603  }, // #86
  { FEATURE(SSSE3)     , CONV_MODE(NonDestructive) , 604  }, // #87
  { FEATURE(SSSE3)     , CONV_MODE(NonDestructive) , 610  }, // #88
  { FEATURE(SSE4_1)    , CONV_MODE(Move)           , 612  }, // #89
  { FEATURE(SSSE3)     , CONV_MODE(NonDestructive) , 613  }, // #90
  { FEATURE(SSSE3)     , CONV_MODE(NonDestructive) , 614  }, // #91
  { FEATURE(SSE4_1)    , CONV_MODE(Move)           , 613  }, // #92
  { FEATURE(SSE)       |
    FEATURE(SSE2)      , CONV_MODE(NonDestructive) , 613  }, // #93
  { FEATURE(SSSE3)     , CONV_MODE(NonDestructive) , 629  }, // #94
  { FEATURE(MMX)       |
    FEATURE(SSE2)      , CONV_MODE(NonDestructive) , 629  }, // #95
  { FEATURE(SSE4_1)    , CONV_MODE(NonDestructive) , 631  }, // #96
  { FEATURE(SSE)       |
    FEATURE(SSE2)      , CONV_MODE(NonDestructive) , 632  }, // #97
  { FEATURE(SSE4_1)    , CONV_MODE(NonDestructive) , 632  }, // #98
  { FEATURE(SSE4_1)    , CONV_MODE(NonDestructive) , 633  }, // #99
  { FEATURE(SSE)       |
    FEATURE(SSE2)      , CONV_MODE(NonDestructive) , 634  }, // #100
  { FEATURE(SSE4_1)    , CONV_MODE(NonDestructive) , 634  }, // #101
  { FEATURE(SSE4_1)    , CONV_MODE(NonDestructive) , 635  }, // #102
  { FEATURE(SSE)       |
    FEATURE(SSE2)      , CONV_MODE(Move)           , 643  }, // #103
  { FEATURE(SSE4_1)    , CONV_MODE(Move)           , 653  }, // #104
  { FEATURE(SSE4_1)    , CONV_MODE(Move)           , 661  }, // #105
  { FEATURE(SSE4_1)    , CONV_MODE(NonDestructive) , 661  }, // #106
  { FEATURE(SSSE3)     , CONV_MODE(NonDestructive) , 661  }, // #107
  { FEATURE(SSE)       |
    FEATURE(SSE2)      , CONV_MODE(NonDestructive) , 660  }, // #108
  { FEATURE(MMX)       |
    FEATURE(SSE2)      , CONV_MODE(NonDestructive) , 660  }, // #109
  { FEATURE(SSE4_1)    , CONV_MODE(NonDestructive) , 660  }, // #110
  { FEATURE(MMX)       |
    FEATURE(SSE2)      , CONV_MODE(NonDestructive) , 661  }, // #111
  { FEATURE(SSE2)      , CONV_MODE(NonDestructive) , 662  }, // #112
  { FEATURE(MMX)       |
    FEATURE(SSE2)      , CONV_MODE(NonDestructive) , 655  }, // #113
  { FEATURE(SSE)       |
    FEATURE(SSE2)      , CONV_MODE(NonDestructive) , 663  }, // #114
  { FEATURE(SSSE3)     , CONV_MODE(NonDestructive) , 675  }, // #115
  { FEATURE(SSE2)      , CONV_MODE(Move)           , 675  }, // #116
  { FEATURE(SSSE3)     , CONV_MODE(NonDestructive) , 674  }, // #117
  { FEATURE(MMX)       |
    FEATURE(SSE2)      , CONV_MODE(NonDestructive) , 674  }, // #118
  { FEATURE(SSE2)      , CONV_MODE(NonDestructive) , 674  }, // #119
  { FEATURE(MMX)       |
    FEATURE(SSE2)      , CONV_MODE(NonDestructive) , 677  }, // #120
  { FEATURE(MMX)       |
    FEATURE(SSE2)      , CONV_MODE(NonDestructive) , 681  }, // #121
  { FEATURE(SSE2)      , CONV_MODE(NonDestructive) , 681  }, // #122
  { FEATURE(MMX)       |
    FEATURE(SSE2)      , CONV_MODE(NonDestructive) , 684  }, // #123
  { FEATURE(SSE2)      , CONV_MODE(NonDestructive) , 684  }, // #124
  { FEATURE(SSE4_1)    , CONV_MODE(Move)           , 685  }, // #125
  { FEATURE(MMX)       |
    FEATURE(SSE2)      , CONV_MODE(NonDestructive) , 693  }, // #126
  { FEATURE(SSE2)      , CONV_MODE(NonDestructive) , 693  }, // #127
  { FEATURE(MMX)       |
    FEATURE(SSE2)      , CONV_MODE(NonDestructive) , 687  }, // #128
  { FEATURE(SSE)       , CONV_MODE(Move)           , 700  }, // #129
  { FEATURE(SSE)       , CONV_MODE(NonDestructive) , 700  }, // #130
  { FEATURE(SSE4_1)    , CONV_MODE(Move)           , 697  }, // #131
  { FEATURE(SSE4_1)    , CONV_MODE(NonDestructive) , 697  }, // #132
  { FEATURE(SSE)       , CONV_MODE(Move)           , 705  }, // #133
  { FEATURE(SSE)       , CONV_MODE(NonDestructive) , 705  }, // #134
  { FEATURE(SHA)       , CONV_MODE(None)           , 0    }, // #135
  { FEATURE(SSE2)      , CONV_MODE(NonDestructive) , 675  }, // #136
  { FEATURE(SSE)       , CONV_MODE(NonDestructive) , 675  }, // #137
  { FEATURE(SSE)       , CONV_MODE(Move)           , 675  }, // #138
  { FEATURE(SSE2)      , CONV_MODE(NonDestructive) , 669  }, // #139
  { FEATURE(SSE)       , CONV_MODE(NonDestructive) , 669  }, // #140
  { FEATURE(SSE2)      , CONV_MODE(Move)           , 666  }, // #141
  { FEATURE(SSE)       , CONV_MODE(Move)           , 666  }, // #142
  { FEATURE(SSE2)      , CONV_MODE(NonDestructive) , 665  }, // #143
  { FEATURE(SSE)       , CONV_MODE(NonDestructive) , 665  }, // #144
  { FEATURE(SSE2)      , CONV_MODE(NonDestructive) , -10  }, // #145
  { FEATURE(SSE)       , CONV_MODE(NonDestructive) , -10  }  // #146
};
#undef CONV_MODE
#undef FEATURE
// ----------------------------------------------------------------------------
// ${sseData:End}

// ${avxData:Begin}
// ------------------- Automatically generated, do not edit -------------------
#define FEATURE(F) X86Inst::AvxData::kFeature##F
#define FLAG(F) X86Inst::AvxData::kFlag##F
const X86Inst::AvxData X86InstDB::avxData[] = {
  { FEATURE(AVX)       |
    FEATURE(AVX512F)   |
    FEATURE(AVX512VL)  , FLAG(Broadcast64) | FLAG(ER) | FLAG(Masking) | FLAG(SAE) | FLAG(Zeroing) }, // #0
  { FEATURE(AVX)       |
    FEATURE(AVX512F)   |
    FEATURE(AVX512VL)  , FLAG(Broadcast32) | FLAG(ER) | FLAG(Masking) | FLAG(SAE) | FLAG(Zeroing) }, // #1
  { FEATURE(AVX)       |
    FEATURE(AVX512F)   , FLAG(ER) | FLAG(Masking) | FLAG(SAE) | FLAG(Zeroing) }, // #2
  { FEATURE(AVX)       , 0 }, // #3
  { FEATURE(AES)       |
    FEATURE(AVX)       , 0 }, // #4
  { FEATURE(AVX512F)   |
    FEATURE(AVX512VL)  , FLAG(Broadcast32) | FLAG(Masking) | FLAG(Zeroing) }, // #5
  { FEATURE(AVX512F)   |
    FEATURE(AVX512VL)  , FLAG(Broadcast64) | FLAG(Masking) | FLAG(Zeroing) }, // #6
  { FEATURE(AVX)       |
    FEATURE(AVX512DQ)  |
    FEATURE(AVX512VL)  , FLAG(Broadcast64) | FLAG(Masking) | FLAG(Zeroing) }, // #7
  { FEATURE(AVX)       |
    FEATURE(AVX512DQ)  |
    FEATURE(AVX512VL)  , FLAG(Broadcast32) | FLAG(Masking) | FLAG(Zeroing) }, // #8
  { FEATURE(AVX512BW)  |
    FEATURE(AVX512VL)  , FLAG(Masking) | FLAG(Zeroing) }, // #9
  { FEATURE(AVX512DQ)  |
    FEATURE(AVX512VL)  , FLAG(Masking) | FLAG(Zeroing) }, // #10
  { FEATURE(AVX512F)   , FLAG(Masking) | FLAG(Zeroing) }, // #11
  { FEATURE(AVX512DQ)  , FLAG(Masking) | FLAG(Zeroing) }, // #12
  { FEATURE(AVX2)      , 0 }, // #13
  { FEATURE(AVX512F)   |
    FEATURE(AVX512VL)  , FLAG(Masking) | FLAG(Zeroing) }, // #14
  { FEATURE(AVX)       |
    FEATURE(AVX2)      |
    FEATURE(AVX512F)   |
    FEATURE(AVX512VL)  , FLAG(Masking) | FLAG(Zeroing) }, // #15
  { FEATURE(AVX)       |
    FEATURE(AVX512F)   |
    FEATURE(AVX512VL)  , FLAG(Broadcast64) | FLAG(Masking) | FLAG(SAE) | FLAG(Zeroing) }, // #16
  { FEATURE(AVX)       |
    FEATURE(AVX512F)   |
    FEATURE(AVX512VL)  , FLAG(Broadcast32) | FLAG(Masking) | FLAG(SAE) | FLAG(Zeroing) }, // #17
  { FEATURE(AVX)       |
    FEATURE(AVX512F)   , FLAG(Masking) | FLAG(SAE) | FLAG(Zeroing) }, // #18
  { FEATURE(AVX)       |
    FEATURE(AVX512F)   , FLAG(SAE) }, // #19
  { FEATURE(AVX)       |
    FEATURE(AVX512F)   |
    FEATURE(AVX512VL)  , FLAG(Broadcast32) | FLAG(Masking) | FLAG(Zeroing) }, // #20
  { FEATURE(AVX512DQ)  |
    FEATURE(AVX512VL)  , FLAG(Broadcast64) | FLAG(ER) | FLAG(Masking) | FLAG(SAE) | FLAG(Zeroing) }, // #21
  { FEATURE(AVX512F)   |
    FEATURE(AVX512VL)  , FLAG(Broadcast64) | FLAG(ER) | FLAG(Masking) | FLAG(SAE) | FLAG(Zeroing) }, // #22
  { FEATURE(AVX512F)   |
    FEATURE(AVX512VL)  |
    FEATURE(F16C)      , FLAG(Masking) | FLAG(SAE) | FLAG(Zeroing) }, // #23
  { FEATURE(AVX512DQ)  |
    FEATURE(AVX512VL)  , FLAG(Broadcast32) | FLAG(ER) | FLAG(Masking) | FLAG(SAE) | FLAG(Zeroing) }, // #24
  { FEATURE(AVX512F)   |
    FEATURE(AVX512VL)  , FLAG(Broadcast32) | FLAG(ER) | FLAG(Masking) | FLAG(SAE) | FLAG(Zeroing) }, // #25
  { FEATURE(AVX)       |
    FEATURE(AVX512F)   , FLAG(ER) | FLAG(SAE) }, // #26
  { FEATURE(AVX512F)   , FLAG(ER) | FLAG(SAE) }, // #27
  { FEATURE(AVX512F)   |
    FEATURE(AVX512VL)  , FLAG(Broadcast64) | FLAG(Masking) | FLAG(SAE) | FLAG(Zeroing) }, // #28
  { FEATURE(AVX512DQ)  |
    FEATURE(AVX512VL)  , FLAG(Broadcast64) | FLAG(Masking) | FLAG(SAE) | FLAG(Zeroing) }, // #29
  { FEATURE(AVX512DQ)  |
    FEATURE(AVX512VL)  , FLAG(Broadcast32) | FLAG(Masking) | FLAG(SAE) | FLAG(Zeroing) }, // #30
  { FEATURE(AVX512F)   |
    FEATURE(AVX512VL)  , FLAG(Broadcast32) | FLAG(Masking) | FLAG(SAE) | FLAG(Zeroing) }, // #31
  { FEATURE(AVX512F)   , FLAG(SAE) }, // #32
  { FEATURE(AVX512ERI) , FLAG(Broadcast64) | FLAG(Masking) | FLAG(SAE) | FLAG(Zeroing) }, // #33
  { FEATURE(AVX512ERI) , FLAG(Broadcast32) | FLAG(Masking) | FLAG(SAE) | FLAG(Zeroing) }, // #34
  { FEATURE(AVX)       |
    FEATURE(AVX512F)   , 0 }, // #35
  { FEATURE(AVX512F)   , FLAG(Masking) | FLAG(SAE) | FLAG(Zeroing) }, // #36
  { FEATURE(AVX512F)   |
    FEATURE(AVX512VL)  |
    FEATURE(FMA)       , FLAG(Broadcast64) | FLAG(ER) | FLAG(Masking) | FLAG(SAE) | FLAG(Zeroing) }, // #37
  { FEATURE(AVX512F)   |
    FEATURE(AVX512VL)  |
    FEATURE(FMA)       , FLAG(Broadcast32) | FLAG(ER) | FLAG(Masking) | FLAG(SAE) | FLAG(Zeroing) }, // #38
  { FEATURE(AVX512F)   |
    FEATURE(FMA)       , FLAG(ER) | FLAG(Masking) | FLAG(SAE) | FLAG(Zeroing) }, // #39
  { FEATURE(FMA4)      , 0 }, // #40
  { FEATURE(AVX512DQ)  |
    FEATURE(AVX512VL)  , FLAG(Broadcast64) | FLAG(Masking) }, // #41
  { FEATURE(AVX512DQ)  |
    FEATURE(AVX512VL)  , FLAG(Broadcast32) | FLAG(Masking) }, // #42
  { FEATURE(AVX512DQ)  , FLAG(Masking) }, // #43
  { FEATURE(XOP)       , 0 }, // #44
  { FEATURE(AVX2)      |
    FEATURE(AVX512F)   |
    FEATURE(AVX512VL)  , FLAG(Masking) }, // #45
  { FEATURE(AVX)       |
    FEATURE(AVX512F)   |
    FEATURE(AVX512VL)  , FLAG(Masking) | FLAG(SAE) | FLAG(Zeroing) }, // #46
  { FEATURE(AVX)       |
    FEATURE(AVX512F)   |
    FEATURE(AVX512VL)  , FLAG(Masking) | FLAG(Zeroing) }, // #47
  { FEATURE(AVX)       |
    FEATURE(AVX512F)   |
    FEATURE(AVX512VL)  , 0 }, // #48
  { FEATURE(AVX)       |
    FEATURE(AVX2)      |
    FEATURE(AVX512F)   |
    FEATURE(AVX512VL)  , 0 }, // #49
  { FEATURE(AVX)       |
    FEATURE(AVX512F)   , FLAG(Masking) | FLAG(Zeroing) }, // #50
  { FEATURE(AVX)       |
    FEATURE(AVX2)      , 0 }, // #51
  { FEATURE(AVX)       |
    FEATURE(AVX2)      |
    FEATURE(AVX512F)   |
    FEATURE(AVX512VL)  , FLAG(Broadcast64) | FLAG(ER) | FLAG(Masking) | FLAG(SAE) | FLAG(Zeroing) }, // #52
  { FEATURE(AVX)       |
    FEATURE(AVX2)      |
    FEATURE(AVX512F)   |
    FEATURE(AVX512VL)  , FLAG(Broadcast32) | FLAG(ER) | FLAG(Masking) | FLAG(SAE) | FLAG(Zeroing) }, // #53
  { FEATURE(AVX)       |
    FEATURE(AVX2)      |
    FEATURE(AVX512BW)  |
    FEATURE(AVX512VL)  , FLAG(Masking) | FLAG(Zeroing) }, // #54
  { FEATURE(AVX)       |
    FEATURE(AVX2)      |
    FEATURE(AVX512BW)  |
    FEATURE(AVX512VL)  , FLAG(Broadcast32) | FLAG(Masking) | FLAG(Zeroing) }, // #55
  { FEATURE(AVX)       |
    FEATURE(AVX2)      |
    FEATURE(AVX512F)   |
    FEATURE(AVX512VL)  , FLAG(Broadcast32) | FLAG(Masking) | FLAG(Zeroing) }, // #56
  { FEATURE(AVX)       |
    FEATURE(AVX2)      |
    FEATURE(AVX512F)   |
    FEATURE(AVX512VL)  , FLAG(Broadcast64) | FLAG(Masking) | FLAG(Zeroing) }, // #57
  { FEATURE(AVX)       |
    FEATURE(AVX512BW)  |
    FEATURE(AVX512VL)  , FLAG(Masking) | FLAG(Zeroing) }, // #58
  { FEATURE(AVX2)      |
    FEATURE(AVX512BW)  |
    FEATURE(AVX512VL)  , FLAG(Masking) | FLAG(Zeroing) }, // #59
  { FEATURE(AVX2)      |
    FEATURE(AVX512F)   |
    FEATURE(AVX512VL)  , FLAG(Masking) | FLAG(Zeroing) }, // #60
  { FEATURE(AVX512CDI) |
    FEATURE(AVX512VL)  , 0 }, // #61
  { FEATURE(AVX)       |
    FEATURE(PCLMULQDQ) , 0 }, // #62
  { FEATURE(AVX512BW)  |
    FEATURE(AVX512VL)  , FLAG(Masking) }, // #63
  { FEATURE(AVX512F)   |
    FEATURE(AVX512VL)  , FLAG(Broadcast32) | FLAG(Masking) }, // #64
  { FEATURE(AVX)       |
    FEATURE(AVX2)      |
    FEATURE(AVX512BW)  |
    FEATURE(AVX512VL)  , FLAG(Masking) }, // #65
  { FEATURE(AVX)       |
    FEATURE(AVX2)      |
    FEATURE(AVX512F)   |
    FEATURE(AVX512VL)  , FLAG(Broadcast32) | FLAG(Masking) }, // #66
  { FEATURE(AVX)       |
    FEATURE(AVX2)      |
    FEATURE(AVX512F)   |
    FEATURE(AVX512VL)  , FLAG(Broadcast64) | FLAG(Masking) }, // #67
  { FEATURE(AVX512F)   |
    FEATURE(AVX512VL)  , FLAG(Broadcast64) | FLAG(Masking) }, // #68
  { FEATURE(AVX512BW)  |
    FEATURE(AVX512VL)  , FLAG(Broadcast64) | FLAG(Masking) }, // #69
  { FEATURE(AVX512CDI) |
    FEATURE(AVX512VL)  , FLAG(Broadcast32) | FLAG(Masking) | FLAG(Zeroing) }, // #70
  { FEATURE(AVX512VBMI)|
    FEATURE(AVX512VL)  , FLAG(Masking) | FLAG(Zeroing) }, // #71
  { FEATURE(AVX2)      |
    FEATURE(AVX512F)   |
    FEATURE(AVX512VL)  , FLAG(Broadcast32) | FLAG(Masking) | FLAG(Zeroing) }, // #72
  { FEATURE(AVX)       |
    FEATURE(AVX512F)   |
    FEATURE(AVX512VL)  , FLAG(Broadcast64) | FLAG(Masking) | FLAG(Zeroing) }, // #73
  { FEATURE(AVX2)      |
    FEATURE(AVX512F)   |
    FEATURE(AVX512VL)  , FLAG(Broadcast64) | FLAG(Masking) | FLAG(Zeroing) }, // #74
  { FEATURE(AVX)       |
    FEATURE(AVX512BW)  , 0 }, // #75
  { FEATURE(AVX)       |
    FEATURE(AVX512DQ)  , 0 }, // #76
  { FEATURE(AVX)       |
    FEATURE(AVX512BW)  , FLAG(Masking) | FLAG(Zeroing) }, // #77
  { FEATURE(AVX)       |
    FEATURE(AVX512DQ)  , FLAG(Masking) | FLAG(Zeroing) }, // #78
  { FEATURE(AVX512CDI) |
    FEATURE(AVX512VL)  , FLAG(Broadcast64) | FLAG(Masking) | FLAG(Zeroing) }, // #79
  { FEATURE(AVX512IFMA)|
    FEATURE(AVX512VL)  , FLAG(Broadcast64) | FLAG(Masking) | FLAG(Zeroing) }, // #80
  { FEATURE(AVX512BW)  |
    FEATURE(AVX512VL)  , 0 }, // #81
  { FEATURE(AVX512DQ)  |
    FEATURE(AVX512VL)  , 0 }, // #82
  { FEATURE(AVX512DQ)  |
    FEATURE(AVX512VL)  , FLAG(Broadcast64) | FLAG(Masking) | FLAG(Zeroing) }, // #83
  { FEATURE(AVX512VBMI)|
    FEATURE(AVX512VL)  , FLAG(Broadcast64) | FLAG(Masking) | FLAG(Zeroing) }, // #84
  { FEATURE(AVX)       |
    FEATURE(AVX2)      |
    FEATURE(AVX512BW)  |
    FEATURE(AVX512VL)  , 0 }, // #85
  { FEATURE(AVX512F)   |
    FEATURE(AVX512VL)  , FLAG(Masking) }, // #86
  { FEATURE(AVX512DQ)  , FLAG(Masking) | FLAG(SAE) | FLAG(Zeroing) }, // #87
  { FEATURE(AVX512ERI) , FLAG(Masking) | FLAG(SAE) | FLAG(Zeroing) }, // #88
  { FEATURE(AVX512DQ)  |
    FEATURE(AVX512VL)  , FLAG(Broadcast32) | FLAG(Masking) | FLAG(Zeroing) }, // #89
  { FEATURE(AVX512F)   , FLAG(ER) | FLAG(Masking) | FLAG(SAE) | FLAG(Zeroing) }  // #90
};
#undef FLAG
#undef FEATURE
// ----------------------------------------------------------------------------
// ${avxData:End}

// ============================================================================
// [asmjit::X86Util - Id <-> Name]
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

enum X86InstAlphaIndex {
  kX86InstAlphaIndexFirst   = 'a',
  kX86InstAlphaIndexLast    = 'z',
  kX86InstAlphaIndexInvalid = 0xFFFF
};

static const uint16_t _x86InstAlphaIndex[26] = {
  X86Inst::kIdAaa,
  X86Inst::kIdBextr,
  X86Inst::kIdCall,
  X86Inst::kIdDaa,
  X86Inst::kIdEmms,
  X86Inst::kIdF2xm1,
  0xFFFF,
  X86Inst::kIdHaddpd,
  X86Inst::kIdIdiv,
  X86Inst::kIdJa,
  X86Inst::kIdKaddb,
  X86Inst::kIdLahf,
  X86Inst::kIdMaskmovdqu,
  X86Inst::kIdNeg,
  X86Inst::kIdOr,
  X86Inst::kIdPabsb,
  0xFFFF,
  X86Inst::kIdRcl,
  X86Inst::kIdSahf,
  X86Inst::kIdT1mskc,
  X86Inst::kIdUcomisd,
  X86Inst::kIdVaddpd,
  X86Inst::kIdWrfsbase,
  X86Inst::kIdXadd,
  0xFFFF,
  0xFFFF
};
// ----------------------------------------------------------------------------
// ${nameData:End}

//! \internal
//!
//! Compare two instruction names.
//!
//! `a` is a null terminated instruction name from `X86InstDB::nameData[]` table.
//! `b` is a non-null terminated instruction name passed to `X86Inst::getIdByName()`.
static ASMJIT_INLINE int X86Inst_compareName(const char* a, const char* b, size_t len) noexcept {
  for (size_t i = 0; i < len; i++) {
    int c = static_cast<int>(static_cast<uint8_t>(a[i])) -
            static_cast<int>(static_cast<uint8_t>(b[i])) ;
    if (c != 0) return c;
  }

  return static_cast<int>(a[len]);
}

uint32_t X86Inst::getIdByName(const char* name, size_t len) noexcept {
  if (ASMJIT_UNLIKELY(!name))
    return kInvalidInst;

  if (len == kInvalidIndex)
    len = ::strlen(name);

  if (ASMJIT_UNLIKELY(len == 0 || len > kX86InstMaxLength))
    return kInvalidInst;

  uint32_t prefix = static_cast<uint32_t>(name[0]) - kX86InstAlphaIndexFirst;
  if (ASMJIT_UNLIKELY(prefix > kX86InstAlphaIndexLast - kX86InstAlphaIndexFirst))
    return kInvalidInst;

  uint32_t index = _x86InstAlphaIndex[prefix];
  if (ASMJIT_UNLIKELY(index == kX86InstAlphaIndexInvalid))
    return kInvalidInst;

  const char* nameData = X86InstDB::nameData;
  const X86Inst* instData = X86InstDB::instData;

  const X86Inst* base = instData + index;
  const X86Inst* end  = instData + X86Inst::_kIdCount;

  // Special handling of instructions starting with 'j' because `jcc` instruction(s)
  // are not sorted alphabetically due to suffixes that are considered part of the
  // instruction. This results in `jecxz` and `jmp` stored after all `jcc` instructions.
  bool useLinearSearch = prefix == ('j' - kX86InstAlphaIndexFirst);

  // Limit the search only to instructions starting with `prefix`.
  while (++prefix <= kX86InstAlphaIndexLast - kX86InstAlphaIndexFirst) {
    index = _x86InstAlphaIndex[prefix];
    if (index == kX86InstAlphaIndexInvalid)
      continue;
    end = instData + index;
    break;
  }

  if (useLinearSearch) {
    while (base != end) {
      if (X86Inst_compareName(nameData + base[0].getNameDataIndex(), name, len) == 0)
        return static_cast<uint32_t>((size_t)(base - instData));
      base++;
    }
  }
  else {
    for (size_t lim = (size_t)(end - base); lim != 0; lim >>= 1) {
      const X86Inst* cur = base + (lim >> 1);
      int result = X86Inst_compareName(nameData + cur[0].getNameDataIndex(), name, len);

      if (result < 0) {
        base = cur + 1;
        lim--;
        continue;
      }

      if (result > 0)
        continue;

      return static_cast<uint32_t>((size_t)(cur - instData));
    }
  }

  return kInvalidInst;
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
// [asmjit::X86Util - Validation]
// ============================================================================

#if !defined(ASMJIT_DISABLE_VALIDATION)
// ${signatureData:Begin}
// ------------------- Automatically generated, do not edit -------------------
#define ISIGNATURE(count, x86, x64, implicit, o0, o1, o2, o3, o4, o5) \
  { count, (x86 ? uint8_t(X86Inst::kArchMaskX86) : uint8_t(0)) |      \
           (x64 ? uint8_t(X86Inst::kArchMaskX64) : uint8_t(0)) ,      \
    implicit,                                                         \
    0,                                                                \
    o0, o1, o2, o3, o4, o5                                            \
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
  X86Inst::kOpNone,  // #01 Reserved.
  X86Inst::kOpNone,  // #02 RIP.
  X86Inst::kOpSeg,   // #03 SEG.
  X86Inst::kOpGpbLo, // #04 GPB-LO.
  X86Inst::kOpGpbHi, // #05 GPB-HI.
  X86Inst::kOpGpw,   // #06 GPW.
  X86Inst::kOpGpd,   // #07 GPD.
  X86Inst::kOpGpq,   // #08 GPQ.
  X86Inst::kOpFp,    // #09 FP.
  X86Inst::kOpMm,    // #10 MM.
  X86Inst::kOpK,     // #11 K.
  X86Inst::kOpXmm,   // #12 XMM.
  X86Inst::kOpYmm,   // #13 YMM.
  X86Inst::kOpZmm,   // #14 ZMM.
  X86Inst::kOpNone,  // #15 FUTURE.
  X86Inst::kOpBnd,   // #16 BND.
  X86Inst::kOpCr,    // #17 CR.
  X86Inst::kOpDr     // #18 DR.
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
    0x00000000U,       // #01 Reserved.
    0x00000001U,       // #02 RIP.
    0x0000007EU,       // #03 SEG (ES|CS|SS|DS|FS|GS).
    0x0000000FU,       // #04 GPB-LO.
    0x0000000FU,       // #05 GPB-HI.
    0x000000FFU,       // #06 GPW.
    0x000000FFU,       // #07 GPD.
    0x000000FFU,       // #08 GPQ.
    0x000000FFU,       // #09 FP.
    0x000000FFU,       // #10 MM.
    0x000000FFU,       // #11 K.
    0x000000FFU,       // #12 XMM.
    0x000000FFU,       // #13 YMM.
    0x000000FFU,       // #14 ZMM.
    0x00000000U,       // #15 FUTURE.
    0x0000000FU,       // #16 BND
    0x000000FFU,       // #17 CR.
    0x000000FFU        // #18 DR
  },

  // AllowedMemBaseRegs:
  (1U << X86Reg::kRegGpw) | (1U << X86Reg::kRegGpd) | (1U << X86Reg::kRegRip) | (1U << Label::kLabelTag),
  // AllowedMemIndexRegs:
  (1U << X86Reg::kRegGpw) | (1U << X86Reg::kRegGpd) | (1U << X86Reg::kRegXmm) | (1U << X86Reg::kRegYmm) | (1U << X86Reg::kRegZmm)
};

static const X86ValidationData _x64ValidationData = {
  {
    0x00000000U,       // #00 None.
    0x00000000U,       // #01 Reserved.
    0x00000001U,       // #02 RIP.
    0x0000007EU,       // #03 SEG (FS|GS) (ES|CS|SS|DS defined, but ignored).
    0x0000FFFFU,       // #04 GPB-LO.
    0x0000000FU,       // #05 GPB-HI.
    0x0000FFFFU,       // #06 GPW.
    0x0000FFFFU,       // #07 GPD.
    0x0000FFFFU,       // #08 GPQ.
    0x000000FFU,       // #09 FP.
    0x000000FFU,       // #10 MM.
    0x000000FFU,       // #11 K.
    0xFFFFFFFFU,       // #12 XMM (16 base regs, 32 regs only with EVEX encoding).
    0xFFFFFFFFU,       // #13 YMM (16 base regs, 32 regs only with EVEX encoding).
    0xFFFFFFFFU,       // #14 ZMM (16 base regs, 32 regs only with EVEX encoding).
    0x00000000U,       // #15 FUTURE.
    0x0000000FU,       // #16 BND.
    0x0000FFFFU,       // #17 CR.
    0x0000FFFFU        // #18 DR.
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
        uint32_t regType = static_cast<const Reg&>(op).getRegType();
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
// [asmjit::X86Util - Condition Codes]
// ============================================================================

#define CC_TO_INST(inst) { \
  inst##o     , inst##no    , inst##b     , inst##ae   , \
  inst##e     , inst##ne    , inst##be    , inst##a    , \
  inst##s     , inst##ns    , inst##pe    , inst##po   , \
  inst##l     , inst##ge    , inst##le    , inst##g    , \
  kInvalidInst, kInvalidInst, kInvalidInst, kInvalidInst \
}

const uint32_t _x86ReverseCond[20] = {
  X86Inst::kCondO, X86Inst::kCondNO, X86Inst::kCondA , X86Inst::kCondBE, // O|NO|B |AE
  X86Inst::kCondE, X86Inst::kCondNE, X86Inst::kCondAE, X86Inst::kCondB , // E|NE|BE|A
  X86Inst::kCondS, X86Inst::kCondNS, X86Inst::kCondPE, X86Inst::kCondPO, // S|NS|PE|PO
  X86Inst::kCondG, X86Inst::kCondLE, X86Inst::kCondGE, X86Inst::kCondL , // L|GE|LE|G
  X86Inst::kCondFpuUnordered, X86Inst::kCondFpuNotUnordered, 0x12, 0x13
};
const uint32_t _x86CondToCmovcc[20] = CC_TO_INST(X86Inst::kIdCmov);
const uint32_t _x86CondToJcc   [20] = CC_TO_INST(X86Inst::kIdJ);
const uint32_t _x86CondToSetcc [20] = CC_TO_INST(X86Inst::kIdSet);

#undef CC_TO_INST

// ============================================================================
// [asmjit::X86Util - Test]
// ============================================================================

#if defined(ASMJIT_TEST)
UNIT(x86_inst_bits) {
  INFO("Checking validity of X86Inst enums.");

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
  EXPECT(rex_rb == 0x45, "kOpCode_R|B must form a valid REX prefix 0x45 if combined with 0x40.");
  EXPECT(rex_rw == 0x4C, "kOpCode_R|W must form a valid REX prefix 0x4C if combined with 0x40.");
}
#endif // ASMJIT_TEST

#if defined(ASMJIT_TEST) && !defined(ASMJIT_DISABLE_TEXT)
UNIT(x86_inst_names) {
  // All known instructions should be matched.
  INFO("Matching all X86/X64 instructions.");
  for (uint32_t a = 0; a < X86Inst::_kIdCount; a++) {
    uint32_t b = X86Inst::getIdByName(X86Inst::getInst(a).getName());
    EXPECT(a == b,
      "Should match existing instruction \"%s\" {id:%u} != \"%s\" {id:%u}.",
        X86Inst::getInst(a).getName(), a,
        X86Inst::getInst(b).getName(), b);
  }

  // Everything else should return `kInvalidInst`.
  INFO("Trying to look-up instructions that don't exist.");
  EXPECT(X86Inst::getIdByName(nullptr) == kInvalidInst,
    "Should return kInvalidInst for null input.");

  EXPECT(X86Inst::getIdByName("") == kInvalidInst,
    "Should return kInvalidInst for empty string.");

  EXPECT(X86Inst::getIdByName("_") == kInvalidInst,
    "Should return kInvalidInst for unknown instruction.");

  EXPECT(X86Inst::getIdByName("123xyz") == kInvalidInst,
    "Should return kInvalidInst for unknown instruction.");
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
  INFO("Validating instructions that use GP registers.");
  EXPECT(x86_validate(X86Inst::kIdCmp   , x86::eax , x86::edx ) == kErrorOk);
  EXPECT(x64_validate(X86Inst::kIdCmp   , x86::rax , x86::rdx ) == kErrorOk);

  EXPECT(x86_validate(X86Inst::kIdCmp   , x86::eax            ) != kErrorOk);
  EXPECT(x86_validate(X86Inst::kIdCmp   , x86::rax , x86::rdx ) != kErrorOk);
  EXPECT(x64_validate(X86Inst::kIdCmp   , x86::rax , x86::al  ) != kErrorOk);

  INFO("Validating instructions that use FP registers.");
  EXPECT(x86_validate(X86Inst::kIdFadd  , x86::fp0 , x86::fp7 ) == kErrorOk);
  EXPECT(x86_validate(X86Inst::kIdFadd  , x86::fp7 , x86::fp0 ) == kErrorOk);
  EXPECT(x86_validate(X86Inst::kIdFadd  , x86::fp0 , x86::eax ) != kErrorOk);
  EXPECT(x86_validate(X86Inst::kIdFadd  , x86::fp4 , x86::fp3 ) != kErrorOk);

  INFO("Validating instructions that use MM registers.");
  EXPECT(x86_validate(X86Inst::kIdPand  , x86::mm0 , x86::mm1 ) == kErrorOk);
  EXPECT(x86_validate(X86Inst::kIdPand  , x86::mm0 , x86::eax ) != kErrorOk);

  INFO("Validating instructions that use XMM registers.");
  EXPECT(x86_validate(X86Inst::kIdPand  , x86::xmm0, x86::xmm1) == kErrorOk);
  EXPECT(x64_validate(X86Inst::kIdPand  , x86::xmm8, x86::xmm9) == kErrorOk);
  EXPECT(x86_validate(X86Inst::kIdPand  , x86::xmm0, x86::eax ) != kErrorOk);
  EXPECT(x64_validate(X86Inst::kIdPand  , x86::xmm0, x86::rax ) != kErrorOk);

  INFO("Validating instructions that use YMM registers.");
  EXPECT(x86_validate(X86Inst::kIdVpand , x86::ymm0, x86::ymm1, x86::ymm2) == kErrorOk);
  EXPECT(x86_validate(X86Inst::kIdVpand , x86::ymm0, x86::ymm1, x86::eax ) != kErrorOk);

  INFO("Validating instructions that use ZMM registers.");
  EXPECT(x86_validate(X86Inst::kIdVpaddw, x86::zmm0, x86::zmm1, x86::zmm2) == kErrorOk);
  EXPECT(x86_validate(X86Inst::kIdVpaddw, x86::zmm0, x86::zmm1, x86::eax ) != kErrorOk);

  INFO("Validating instructions that use CR registers.");
  EXPECT(x86_validate(X86Inst::kIdMov   , x86::eax , x86::cr0 ) == kErrorOk);
  EXPECT(x86_validate(X86Inst::kIdMov   , x86::eax , x86::cr8 ) != kErrorOk);
  EXPECT(x64_validate(X86Inst::kIdMov   , x86::rax , x86::cr8 ) == kErrorOk);
  EXPECT(x64_validate(X86Inst::kIdMov   , x86::eax , x86::cr0 ) != kErrorOk);

  INFO("Validating instructions that use DR registers.");
  EXPECT(x86_validate(X86Inst::kIdMov   , x86::eax , x86::dr0 ) == kErrorOk);
  EXPECT(x64_validate(X86Inst::kIdMov   , x86::rax , x86::dr7 ) == kErrorOk);
  EXPECT(x86_validate(X86Inst::kIdMov   , x86::ax  , x86::dr0 ) != kErrorOk);
  EXPECT(x64_validate(X86Inst::kIdMov   , x86::eax , x86::dr7 ) != kErrorOk);

  INFO("Validating instructions that use segment registers.");
  EXPECT(x86_validate(X86Inst::kIdMov   , x86::ax  , x86::fs  ) == kErrorOk);
  EXPECT(x64_validate(X86Inst::kIdMov   , x86::ax  , x86::fs  ) == kErrorOk);
  EXPECT(x64_validate(X86Inst::kIdPush  , x86::cs             ) != kErrorOk);

  INFO("Validating instructions that use memory operands.");
  EXPECT(x86_validate(X86Inst::kIdMov   , x86::eax , x86::ptr(x86::ebx)) == kErrorOk);
  EXPECT(x64_validate(X86Inst::kIdMov   , x86::rax , x86::ptr(x86::rbx)) == kErrorOk);

  INFO("Validating instructions that use immediate values.");
  EXPECT(x86_validate(X86Inst::kIdMov   , x86::eax , imm(1)) == kErrorOk);
  EXPECT(x64_validate(X86Inst::kIdMov   , x86::rax , imm(1)) == kErrorOk);
}
#endif // ASMJIT_TEST && !ASMJIT_DISABLE_VALIDATION

} // asmjit namespace

// [Api-End]
#include "../asmjit_apiend.h"

// [Guard]
#endif // ASMJIT_BUILD_X86
