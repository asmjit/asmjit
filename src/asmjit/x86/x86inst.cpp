// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Export]
#define ASMJIT_EXPORTS

// [Guard]
#include "../build.h"
#if defined(ASMJIT_BUILD_X86) || defined(ASMJIT_BUILD_X64)

// [Dependencies - AsmJit]
#include "../x86/x86inst.h"

// [Api-Begin]
#include "../apibegin.h"

namespace asmjit {

// ============================================================================
// [Enums (Internal)]
// ============================================================================

//! \internal
enum {
  // REX/VEX.
  kX86InstTable_L__ = (0) << kX86InstOpCode_L_Shift,  // L is operand-based or unspecified.
  kX86InstTable_L_I = (0) << kX86InstOpCode_L_Shift,  // L is ignored (LIG).
  kX86InstTable_L_0 = (0) << kX86InstOpCode_L_Shift,  // L has to be zero.
  kX86InstTable_L_L = (1) << kX86InstOpCode_L_Shift,  // L has to be set.

  kX86InstTable_W__ = (0) << kX86InstOpCode_W_Shift,  // W is operand-based or unspecified.
  kX86InstTable_W_I = (0) << kX86InstOpCode_W_Shift,  // W is ignored (WIG).
  kX86InstTable_W_0 = (0) << kX86InstOpCode_W_Shift,  // W has to be zero.
  kX86InstTable_W_W = (1) << kX86InstOpCode_W_Shift,  // W has to be set.

  // EVEX.
  kX86InstTable_E__ = (0) << kX86InstOpCode_EW_Shift, // EVEX.W is operand-based or unspecified.
  kX86InstTable_E_I = (0) << kX86InstOpCode_EW_Shift, // EVEX.W is ignored (WIG).
  kX86InstTable_E_0 = (0) << kX86InstOpCode_EW_Shift, // EVEX.W has to be zero.
  kX86InstTable_E_1 = (1) << kX86InstOpCode_EW_Shift  // EVEX.W has to be set.
};

//! \internal
//!
//! Combined flags.
enum X86InstOpInternal {
  kX86InstOpI        = kX86InstOpImm,

  kX86InstOpL        = kX86InstOpLabel,
  kX86InstOpLImm     = kX86InstOpLabel | kX86InstOpImm,

  kX86InstOpGwb      = kX86InstOpGw    | kX86InstOpGb,
  kX86InstOpGqd      = kX86InstOpGq    | kX86InstOpGd,
  kX86InstOpGqdw     = kX86InstOpGq    | kX86InstOpGd | kX86InstOpGw,
  kX86InstOpGqdwb    = kX86InstOpGq    | kX86InstOpGd | kX86InstOpGw | kX86InstOpGb,

  kX86InstOpGbMem    = kX86InstOpGb    | kX86InstOpMem,
  kX86InstOpGwMem    = kX86InstOpGw    | kX86InstOpMem,
  kX86InstOpGdMem    = kX86InstOpGd    | kX86InstOpMem,
  kX86InstOpGqMem    = kX86InstOpGq    | kX86InstOpMem,
  kX86InstOpGwbMem   = kX86InstOpGwb   | kX86InstOpMem,
  kX86InstOpGqdMem   = kX86InstOpGqd   | kX86InstOpMem,
  kX86InstOpGqdwMem  = kX86InstOpGqdw  | kX86InstOpMem,
  kX86InstOpGqdwbMem = kX86InstOpGqdwb | kX86InstOpMem,

  kX86InstOpFpMem    = kX86InstOpFp    | kX86InstOpMem,
  kX86InstOpMmMem    = kX86InstOpMm    | kX86InstOpMem,
  kX86InstOpKMem     = kX86InstOpK     | kX86InstOpMem,
  kX86InstOpXmmMem   = kX86InstOpXmm   | kX86InstOpMem,
  kX86InstOpYmmMem   = kX86InstOpYmm   | kX86InstOpMem,
  kX86InstOpZmmMem   = kX86InstOpZmm   | kX86InstOpMem,

  kX86InstOpMmXmm    = kX86InstOpMm    | kX86InstOpXmm,
  kX86InstOpMmXmmMem = kX86InstOpMmXmm | kX86InstOpMem,

  kX86InstOpXy       = kX86InstOpXmm   | kX86InstOpYmm,
  kX86InstOpXyMem    = kX86InstOpXy    | kX86InstOpMem,

  kX86InstOpXyz      = kX86InstOpXy    | kX86InstOpZmm,
  kX86InstOpXyzMem   = kX86InstOpXyz   | kX86InstOpMem
};

//! \internal
//!
//! X86/X64 Instruction AVX-512 flags (combined).
ASMJIT_ENUM(X86InstFlagsInternal) {
  // FPU.
  kX86InstFlagMem2_4           = kX86InstFlagMem2        | kX86InstFlagMem4,
  kX86InstFlagMem2_4_8         = kX86InstFlagMem2_4      | kX86InstFlagMem8,
  kX86InstFlagMem4_8           = kX86InstFlagMem4        | kX86InstFlagMem8,
  kX86InstFlagMem4_8_10        = kX86InstFlagMem4_8      | kX86InstFlagMem10
};

// ============================================================================
// [Macros]
// ============================================================================

#if !defined(ASMJIT_DISABLE_TEXT)
# define INST_NAME_INDEX(_Code_) _Code_##_NameIndex
#else
# define INST_NAME_INDEX(_Code_) 0
#endif

// Undefined. Used to distinguish between zero and field that is not used.
#define U 0

// Instruction opcodes.
#define O_000000(_OpCode_, _O_, _L_, _W_, _EVEX_W_) (kX86InstOpCode_PP_00 | kX86InstOpCode_MM_00   | (0x##_OpCode_) | ((_O_) << kX86InstOpCode_O_Shift) | kX86InstTable_L_##_L_ | kX86InstTable_W_##_W_ | kX86InstTable_E_##_EVEX_W_)
#define O_000F00(_OpCode_, _O_, _L_, _W_, _EVEX_W_) (kX86InstOpCode_PP_00 | kX86InstOpCode_MM_0F   | (0x##_OpCode_) | ((_O_) << kX86InstOpCode_O_Shift) | kX86InstTable_L_##_L_ | kX86InstTable_W_##_W_ | kX86InstTable_E_##_EVEX_W_)
#define O_000F01(_OpCode_, _O_, _L_, _W_, _EVEX_W_) (kX86InstOpCode_PP_00 | kX86InstOpCode_MM_0F01 | (0x##_OpCode_) | ((_O_) << kX86InstOpCode_O_Shift) | kX86InstTable_L_##_L_ | kX86InstTable_W_##_W_ | kX86InstTable_E_##_EVEX_W_)
#define O_000F0F(_OpCode_, _O_, _L_, _W_, _EVEX_W_) (kX86InstOpCode_PP_00 | kX86InstOpCode_MM_0F   | (0x##_OpCode_) | ((_O_) << kX86InstOpCode_O_Shift) | kX86InstTable_L_##_L_ | kX86InstTable_W_##_W_ | kX86InstTable_E_##_EVEX_W_)
#define O_000F38(_OpCode_, _O_, _L_, _W_, _EVEX_W_) (kX86InstOpCode_PP_00 | kX86InstOpCode_MM_0F38 | (0x##_OpCode_) | ((_O_) << kX86InstOpCode_O_Shift) | kX86InstTable_L_##_L_ | kX86InstTable_W_##_W_ | kX86InstTable_E_##_EVEX_W_)
#define O_000F3A(_OpCode_, _O_, _L_, _W_, _EVEX_W_) (kX86InstOpCode_PP_00 | kX86InstOpCode_MM_0F3A | (0x##_OpCode_) | ((_O_) << kX86InstOpCode_O_Shift) | kX86InstTable_L_##_L_ | kX86InstTable_W_##_W_ | kX86InstTable_E_##_EVEX_W_)
#define O_660000(_OpCode_, _O_, _L_, _W_, _EVEX_W_) (kX86InstOpCode_PP_66 | kX86InstOpCode_MM_00   | (0x##_OpCode_) | ((_O_) << kX86InstOpCode_O_Shift) | kX86InstTable_L_##_L_ | kX86InstTable_W_##_W_ | kX86InstTable_E_##_EVEX_W_)
#define O_660F00(_OpCode_, _O_, _L_, _W_, _EVEX_W_) (kX86InstOpCode_PP_66 | kX86InstOpCode_MM_0F   | (0x##_OpCode_) | ((_O_) << kX86InstOpCode_O_Shift) | kX86InstTable_L_##_L_ | kX86InstTable_W_##_W_ | kX86InstTable_E_##_EVEX_W_)
#define O_660F38(_OpCode_, _O_, _L_, _W_, _EVEX_W_) (kX86InstOpCode_PP_66 | kX86InstOpCode_MM_0F38 | (0x##_OpCode_) | ((_O_) << kX86InstOpCode_O_Shift) | kX86InstTable_L_##_L_ | kX86InstTable_W_##_W_ | kX86InstTable_E_##_EVEX_W_)
#define O_660F3A(_OpCode_, _O_, _L_, _W_, _EVEX_W_) (kX86InstOpCode_PP_66 | kX86InstOpCode_MM_0F3A | (0x##_OpCode_) | ((_O_) << kX86InstOpCode_O_Shift) | kX86InstTable_L_##_L_ | kX86InstTable_W_##_W_ | kX86InstTable_E_##_EVEX_W_)
#define O_9B0000(_OpCode_, _O_, _L_, _W_, _EVEX_W_) (kX86InstOpCode_PP_9B | kX86InstOpCode_MM_00   | (0x##_OpCode_) | ((_O_) << kX86InstOpCode_O_Shift) | kX86InstTable_L_##_L_ | kX86InstTable_W_##_W_ | kX86InstTable_E_##_EVEX_W_)
#define O_F20000(_OpCode_, _O_, _L_, _W_, _EVEX_W_) (kX86InstOpCode_PP_F2 | kX86InstOpCode_MM_00   | (0x##_OpCode_) | ((_O_) << kX86InstOpCode_O_Shift) | kX86InstTable_L_##_L_ | kX86InstTable_W_##_W_ | kX86InstTable_E_##_EVEX_W_)
#define O_F20F00(_OpCode_, _O_, _L_, _W_, _EVEX_W_) (kX86InstOpCode_PP_F2 | kX86InstOpCode_MM_0F   | (0x##_OpCode_) | ((_O_) << kX86InstOpCode_O_Shift) | kX86InstTable_L_##_L_ | kX86InstTable_W_##_W_ | kX86InstTable_E_##_EVEX_W_)
#define O_F20F38(_OpCode_, _O_, _L_, _W_, _EVEX_W_) (kX86InstOpCode_PP_F2 | kX86InstOpCode_MM_0F38 | (0x##_OpCode_) | ((_O_) << kX86InstOpCode_O_Shift) | kX86InstTable_L_##_L_ | kX86InstTable_W_##_W_ | kX86InstTable_E_##_EVEX_W_)
#define O_F20F3A(_OpCode_, _O_, _L_, _W_, _EVEX_W_) (kX86InstOpCode_PP_F2 | kX86InstOpCode_MM_0F3A | (0x##_OpCode_) | ((_O_) << kX86InstOpCode_O_Shift) | kX86InstTable_L_##_L_ | kX86InstTable_W_##_W_ | kX86InstTable_E_##_EVEX_W_)
#define O_F30000(_OpCode_, _O_, _L_, _W_, _EVEX_W_) (kX86InstOpCode_PP_F3 | kX86InstOpCode_MM_00   | (0x##_OpCode_) | ((_O_) << kX86InstOpCode_O_Shift) | kX86InstTable_L_##_L_ | kX86InstTable_W_##_W_ | kX86InstTable_E_##_EVEX_W_)
#define O_F30F00(_OpCode_, _O_, _L_, _W_, _EVEX_W_) (kX86InstOpCode_PP_F3 | kX86InstOpCode_MM_0F   | (0x##_OpCode_) | ((_O_) << kX86InstOpCode_O_Shift) | kX86InstTable_L_##_L_ | kX86InstTable_W_##_W_ | kX86InstTable_E_##_EVEX_W_)
#define O_F30F38(_OpCode_, _O_, _L_, _W_, _EVEX_W_) (kX86InstOpCode_PP_F3 | kX86InstOpCode_MM_0F38 | (0x##_OpCode_) | ((_O_) << kX86InstOpCode_O_Shift) | kX86InstTable_L_##_L_ | kX86InstTable_W_##_W_ | kX86InstTable_E_##_EVEX_W_)
#define O_F30F3A(_OpCode_, _O_, _L_, _W_, _EVEX_W_) (kX86InstOpCode_PP_F3 | kX86InstOpCode_MM_0F3A | (0x##_OpCode_) | ((_O_) << kX86InstOpCode_O_Shift) | kX86InstTable_L_##_L_ | kX86InstTable_W_##_W_ | kX86InstTable_E_##_EVEX_W_)

#define O_00_M08(_OpCode_, _O_, _L_, _W_, _EVEX_W_) (kX86InstOpCode_PP_00 | kX86InstOpCode_MM_01000| (0x##_OpCode_) | ((_O_) << kX86InstOpCode_O_Shift) | kX86InstTable_L_##_L_ | kX86InstTable_W_##_W_ | kX86InstTable_E_##_EVEX_W_)
#define O_00_M09(_OpCode_, _O_, _L_, _W_, _EVEX_W_) (kX86InstOpCode_PP_00 | kX86InstOpCode_MM_01001| (0x##_OpCode_) | ((_O_) << kX86InstOpCode_O_Shift) | kX86InstTable_L_##_L_ | kX86InstTable_W_##_W_ | kX86InstTable_E_##_EVEX_W_)
#define O_66_M03(_OpCode_, _O_, _L_, _W_, _EVEX_W_) (kX86InstOpCode_PP_66 | kX86InstOpCode_MM_00011| (0x##_OpCode_) | ((_O_) << kX86InstOpCode_O_Shift) | kX86InstTable_L_##_L_ | kX86InstTable_W_##_W_ | kX86InstTable_E_##_EVEX_W_)

#define O_00_X(_OpCode_, _O_) (kX86InstOpCode_PP_00 | (0x##_OpCode_) | ((_O_) << kX86InstOpCode_O_Shift))
#define O_9B_X(_OpCode_, _O_) (kX86InstOpCode_PP_9B | (0x##_OpCode_) | ((_O_) << kX86InstOpCode_O_Shift))

// Instruction Encoding `Enc(...)`.
#define Enc(_Id_) kX86InstEncodingId##_Id_

// Instruction Flags `F(...)` and AVX-512 `AVX(...)`flags.
#define F(_Flags_) kX86InstFlag##_Flags_

#define A(_Flags_) kX86InstFlagAvx512##_Flags_

// Instruction EFLAGS `E(OSZAPCDX)`.
#define EF(_Flags_) 0

// Instruction Operands' Flags `O(...)`.
#define O(_Op_) kX86InstOp##_Op_

// Defines an X86/X64 instruction.
#define INST(_Id_, _Name_, _OpCode0_, _OpCode1_, _Encoding_, _IFlags_, _EFlags_, _WriteIndex_, _WriteSize_, _Op0_, _Op_, _Op2_, _Op3_, _Op4_) \
  { INST_NAME_INDEX(_Id_), _Id_##_ExtendedIndex, _OpCode0_ }

// ============================================================================
// [asmjit::X86Inst]
// ============================================================================

// ${X86InstData:Begin}
// Automatically generated, do not edit.
#if !defined(ASMJIT_DISABLE_TEXT)
const char _x86InstName[] =
  "\0"
  "adc\0"
  "bextr\0"
  "blsi\0"
  "blsmsk\0"
  "blsr\0"
  "bsf\0"
  "bsr\0"
  "bswap\0"
  "bt\0"
  "btc\0"
  "btr\0"
  "bts\0"
  "bzhi\0"
  "call\0"
  "cbw\0"
  "cdq\0"
  "cdqe\0"
  "clc\0"
  "cld\0"
  "clflush\0"
  "cmc\0"
  "cmova\0"
  "cmovae\0"
  "cmovc\0"
  "cmovg\0"
  "cmovge\0"
  "cmovl\0"
  "cmovle\0"
  "cmovna\0"
  "cmovnae\0"
  "cmovnc\0"
  "cmovng\0"
  "cmovnge\0"
  "cmovnl\0"
  "cmovnle\0"
  "cmovno\0"
  "cmovnp\0"
  "cmovns\0"
  "cmovnz\0"
  "cmovo\0"
  "cmovp\0"
  "cmovpe\0"
  "cmovpo\0"
  "cmovs\0"
  "cmovz\0"
  "cmp\0"
  "cmpxchg\0"
  "cmpxchg16b\0"
  "cmpxchg8b\0"
  "cpuid\0"
  "cqo\0"
  "crc32\0"
  "cvtpd2pi\0"
  "cvtpi2pd\0"
  "cvtpi2ps\0"
  "cvtps2pi\0"
  "cvttpd2pi\0"
  "cvttps2pi\0"
  "cwd\0"
  "cwde\0"
  "daa\0"
  "das\0"
  "enter\0"
  "f2xm1\0"
  "fabs\0"
  "faddp\0"
  "fbld\0"
  "fbstp\0"
  "fchs\0"
  "fclex\0"
  "fcmovb\0"
  "fcmovbe\0"
  "fcmove\0"
  "fcmovnb\0"
  "fcmovnbe\0"
  "fcmovne\0"
  "fcmovnu\0"
  "fcmovu\0"
  "fcom\0"
  "fcomi\0"
  "fcomip\0"
  "fcomp\0"
  "fcompp\0"
  "fcos\0"
  "fdecstp\0"
  "fdiv\0"
  "fdivp\0"
  "fdivr\0"
  "fdivrp\0"
  "femms\0"
  "ffree\0"
  "fiadd\0"
  "ficom\0"
  "ficomp\0"
  "fidiv\0"
  "fidivr\0"
  "fild\0"
  "fimul\0"
  "fincstp\0"
  "finit\0"
  "fist\0"
  "fistp\0"
  "fisttp\0"
  "fisub\0"
  "fisubr\0"
  "fld\0"
  "fld1\0"
  "fldcw\0"
  "fldenv\0"
  "fldl2e\0"
  "fldl2t\0"
  "fldlg2\0"
  "fldln2\0"
  "fldpi\0"
  "fldz\0"
  "fmulp\0"
  "fnclex\0"
  "fninit\0"
  "fnop\0"
  "fnsave\0"
  "fnstcw\0"
  "fnstenv\0"
  "fnstsw\0"
  "fpatan\0"
  "fprem\0"
  "fprem1\0"
  "fptan\0"
  "frndint\0"
  "frstor\0"
  "fsave\0"
  "fscale\0"
  "fsin\0"
  "fsincos\0"
  "fsqrt\0"
  "fst\0"
  "fstcw\0"
  "fstenv\0"
  "fstp\0"
  "fstsw\0"
  "fsubp\0"
  "fsubrp\0"
  "ftst\0"
  "fucom\0"
  "fucomi\0"
  "fucomip\0"
  "fucomp\0"
  "fucompp\0"
  "fwait\0"
  "fxam\0"
  "fxch\0"
  "fxrstor\0"
  "fxsave\0"
  "fxtract\0"
  "fyl2x\0"
  "fyl2xp1\0"
  "inc\0"
  "insertq\0"
  "ja\0"
  "jae\0"
  "jb\0"
  "jbe\0"
  "jc\0"
  "je\0"
  "jecxz\0"
  "jg\0"
  "jge\0"
  "jl\0"
  "jle\0"
  "jmp\0"
  "jna\0"
  "jnae\0"
  "jnb\0"
  "jnbe\0"
  "jnc\0"
  "jne\0"
  "jng\0"
  "jnge\0"
  "jnl\0"
  "jnle\0"
  "jno\0"
  "jnp\0"
  "jns\0"
  "jnz\0"
  "jo\0"
  "jp\0"
  "jpe\0"
  "jpo\0"
  "js\0"
  "jz\0"
  "lahf\0"
  "lea\0"
  "leave\0"
  "lfence\0"
  "lzcnt\0"
  "mfence\0"
  "monitor\0"
  "mov_ptr\0"
  "movdq2q\0"
  "movnti\0"
  "movntq\0"
  "movntsd\0"
  "movntss\0"
  "movq2dq\0"
  "movsx\0"
  "movsxd\0"
  "movzx\0"
  "mulx\0"
  "mwait\0"
  "neg\0"
  "not\0"
  "pause\0"
  "pdep\0"
  "pext\0"
  "pf2id\0"
  "pf2iw\0"
  "pfacc\0"
  "pfadd\0"
  "pfcmpeq\0"
  "pfcmpge\0"
  "pfcmpgt\0"
  "pfmax\0"
  "pfmin\0"
  "pfmul\0"
  "pfnacc\0"
  "pfpnacc\0"
  "pfrcp\0"
  "pfrcpit1\0"
  "pfrcpit2\0"
  "pfrsqit1\0"
  "pfrsqrt\0"
  "pfsub\0"
  "pfsubr\0"
  "pi2fd\0"
  "pi2fw\0"
  "pop\0"
  "popa\0"
  "popcnt\0"
  "popf\0"
  "prefetch\0"
  "prefetch_3dnow\0"
  "prefetchw_3dnow\0"
  "pshufw\0"
  "pswapd\0"
  "push\0"
  "pusha\0"
  "pushf\0"
  "rcl\0"
  "rcr\0"
  "rdfsbase\0"
  "rdgsbase\0"
  "rdrand\0"
  "rdtsc\0"
  "rdtscp\0"
  "rep lods_b\0"
  "rep lods_d\0"
  "rep lods_q\0"
  "rep lods_w\0"
  "rep movs_b\0"
  "rep movs_d\0"
  "rep movs_q\0"
  "rep movs_w\0"
  "rep stos_b\0"
  "rep stos_d\0"
  "rep stos_q\0"
  "rep stos_w\0"
  "repe cmps_b\0"
  "repe cmps_d\0"
  "repe cmps_q\0"
  "repe cmps_w\0"
  "repe scas_b\0"
  "repe scas_d\0"
  "repe scas_q\0"
  "repe scas_w\0"
  "repne cmps_b\0"
  "repne cmps_d\0"
  "repne cmps_q\0"
  "repne cmps_w\0"
  "repne scas_b\0"
  "repne scas_d\0"
  "repne scas_q\0"
  "repne scas_w\0"
  "ret\0"
  "rol\0"
  "ror\0"
  "rorx\0"
  "sahf\0"
  "sal\0"
  "sar\0"
  "sarx\0"
  "sbb\0"
  "seta\0"
  "setae\0"
  "setb\0"
  "setbe\0"
  "setc\0"
  "sete\0"
  "setg\0"
  "setge\0"
  "setl\0"
  "setle\0"
  "setna\0"
  "setnae\0"
  "setnb\0"
  "setnbe\0"
  "setnc\0"
  "setne\0"
  "setng\0"
  "setnge\0"
  "setnl\0"
  "setnle\0"
  "setno\0"
  "setnp\0"
  "setns\0"
  "setnz\0"
  "seto\0"
  "setp\0"
  "setpe\0"
  "setpo\0"
  "sets\0"
  "setz\0"
  "sfence\0"
  "shl\0"
  "shlx\0"
  "shr\0"
  "shrd\0"
  "shrx\0"
  "stc\0"
  "tzcnt\0"
  "ud2\0"
  "vaddpd\0"
  "vaddps\0"
  "vaddsd\0"
  "vaddss\0"
  "vaddsubpd\0"
  "vaddsubps\0"
  "vaesdec\0"
  "vaesdeclast\0"
  "vaesenc\0"
  "vaesenclast\0"
  "vaesimc\0"
  "vaeskeygenassist\0"
  "vandnpd\0"
  "vandnps\0"
  "vandpd\0"
  "vandps\0"
  "vblendpd\0"
  "vblendps\0"
  "vblendvpd\0"
  "vblendvps\0"
  "vbroadcastf128\0"
  "vbroadcasti128\0"
  "vbroadcastsd\0"
  "vbroadcastss\0"
  "vcmppd\0"
  "vcmpps\0"
  "vcmpsd\0"
  "vcmpss\0"
  "vcomisd\0"
  "vcomiss\0"
  "vcvtdq2pd\0"
  "vcvtdq2ps\0"
  "vcvtpd2dq\0"
  "vcvtpd2ps\0"
  "vcvtph2ps\0"
  "vcvtps2dq\0"
  "vcvtps2pd\0"
  "vcvtps2ph\0"
  "vcvtsd2si\0"
  "vcvtsd2ss\0"
  "vcvtsi2sd\0"
  "vcvtsi2ss\0"
  "vcvtss2sd\0"
  "vcvtss2si\0"
  "vcvttpd2dq\0"
  "vcvttps2dq\0"
  "vcvttsd2si\0"
  "vcvttss2si\0"
  "vdivpd\0"
  "vdivps\0"
  "vdivsd\0"
  "vdivss\0"
  "vdppd\0"
  "vdpps\0"
  "vextractf128\0"
  "vextracti128\0"
  "vextractps\0"
  "vfmadd132pd\0"
  "vfmadd132ps\0"
  "vfmadd132sd\0"
  "vfmadd132ss\0"
  "vfmadd213pd\0"
  "vfmadd213ps\0"
  "vfmadd213sd\0"
  "vfmadd213ss\0"
  "vfmadd231pd\0"
  "vfmadd231ps\0"
  "vfmadd231sd\0"
  "vfmadd231ss\0"
  "vfmaddpd\0"
  "vfmaddps\0"
  "vfmaddsd\0"
  "vfmaddss\0"
  "vfmaddsub132pd\0"
  "vfmaddsub132ps\0"
  "vfmaddsub213pd\0"
  "vfmaddsub213ps\0"
  "vfmaddsub231pd\0"
  "vfmaddsub231ps\0"
  "vfmaddsubpd\0"
  "vfmaddsubps\0"
  "vfmsub132pd\0"
  "vfmsub132ps\0"
  "vfmsub132sd\0"
  "vfmsub132ss\0"
  "vfmsub213pd\0"
  "vfmsub213ps\0"
  "vfmsub213sd\0"
  "vfmsub213ss\0"
  "vfmsub231pd\0"
  "vfmsub231ps\0"
  "vfmsub231sd\0"
  "vfmsub231ss\0"
  "vfmsubadd132pd\0"
  "vfmsubadd132ps\0"
  "vfmsubadd213pd\0"
  "vfmsubadd213ps\0"
  "vfmsubadd231pd\0"
  "vfmsubadd231ps\0"
  "vfmsubaddpd\0"
  "vfmsubaddps\0"
  "vfmsubpd\0"
  "vfmsubps\0"
  "vfmsubsd\0"
  "vfmsubss\0"
  "vfnmadd132pd\0"
  "vfnmadd132ps\0"
  "vfnmadd132sd\0"
  "vfnmadd132ss\0"
  "vfnmadd213pd\0"
  "vfnmadd213ps\0"
  "vfnmadd213sd\0"
  "vfnmadd213ss\0"
  "vfnmadd231pd\0"
  "vfnmadd231ps\0"
  "vfnmadd231sd\0"
  "vfnmadd231ss\0"
  "vfnmaddpd\0"
  "vfnmaddps\0"
  "vfnmaddsd\0"
  "vfnmaddss\0"
  "vfnmsub132pd\0"
  "vfnmsub132ps\0"
  "vfnmsub132sd\0"
  "vfnmsub132ss\0"
  "vfnmsub213pd\0"
  "vfnmsub213ps\0"
  "vfnmsub213sd\0"
  "vfnmsub213ss\0"
  "vfnmsub231pd\0"
  "vfnmsub231ps\0"
  "vfnmsub231sd\0"
  "vfnmsub231ss\0"
  "vfnmsubpd\0"
  "vfnmsubps\0"
  "vfnmsubsd\0"
  "vfnmsubss\0"
  "vfrczpd\0"
  "vfrczps\0"
  "vfrczsd\0"
  "vfrczss\0"
  "vgatherdpd\0"
  "vgatherdps\0"
  "vgatherqpd\0"
  "vgatherqps\0"
  "vhaddpd\0"
  "vhaddps\0"
  "vhsubpd\0"
  "vhsubps\0"
  "vinsertf128\0"
  "vinserti128\0"
  "vinsertps\0"
  "vlddqu\0"
  "vldmxcsr\0"
  "vmaskmovdqu\0"
  "vmaskmovpd\0"
  "vmaskmovps\0"
  "vmaxpd\0"
  "vmaxps\0"
  "vmaxsd\0"
  "vmaxss\0"
  "vminpd\0"
  "vminps\0"
  "vminsd\0"
  "vminss\0"
  "vmovapd\0"
  "vmovaps\0"
  "vmovd\0"
  "vmovddup\0"
  "vmovdqa\0"
  "vmovdqu\0"
  "vmovhlps\0"
  "vmovhpd\0"
  "vmovhps\0"
  "vmovlhps\0"
  "vmovlpd\0"
  "vmovlps\0"
  "vmovmskpd\0"
  "vmovmskps\0"
  "vmovntdq\0"
  "vmovntdqa\0"
  "vmovntpd\0"
  "vmovntps\0"
  "vmovq\0"
  "vmovsd\0"
  "vmovshdup\0"
  "vmovsldup\0"
  "vmovss\0"
  "vmovupd\0"
  "vmovups\0"
  "vmpsadbw\0"
  "vmulpd\0"
  "vmulps\0"
  "vmulsd\0"
  "vmulss\0"
  "vorpd\0"
  "vorps\0"
  "vpabsb\0"
  "vpabsd\0"
  "vpabsw\0"
  "vpackssdw\0"
  "vpacksswb\0"
  "vpackusdw\0"
  "vpackuswb\0"
  "vpaddb\0"
  "vpaddd\0"
  "vpaddq\0"
  "vpaddsb\0"
  "vpaddsw\0"
  "vpaddusb\0"
  "vpaddusw\0"
  "vpaddw\0"
  "vpalignr\0"
  "vpand\0"
  "vpandn\0"
  "vpavgb\0"
  "vpavgw\0"
  "vpblendd\0"
  "vpblendvb\0"
  "vpblendw\0"
  "vpbroadcastb\0"
  "vpbroadcastd\0"
  "vpbroadcastq\0"
  "vpbroadcastw\0"
  "vpclmulqdq\0"
  "vpcmov\0"
  "vpcmpeqb\0"
  "vpcmpeqd\0"
  "vpcmpeqq\0"
  "vpcmpeqw\0"
  "vpcmpestri\0"
  "vpcmpestrm\0"
  "vpcmpgtb\0"
  "vpcmpgtd\0"
  "vpcmpgtq\0"
  "vpcmpgtw\0"
  "vpcmpistri\0"
  "vpcmpistrm\0"
  "vpcomb\0"
  "vpcomd\0"
  "vpcomq\0"
  "vpcomub\0"
  "vpcomud\0"
  "vpcomuq\0"
  "vpcomuw\0"
  "vpcomw\0"
  "vperm2f128\0"
  "vperm2i128\0"
  "vpermd\0"
  "vpermil2pd\0"
  "vpermil2ps\0"
  "vpermilpd\0"
  "vpermilps\0"
  "vpermpd\0"
  "vpermps\0"
  "vpermq\0"
  "vpextrb\0"
  "vpextrd\0"
  "vpextrq\0"
  "vpextrw\0"
  "vpgatherdd\0"
  "vpgatherdq\0"
  "vpgatherqd\0"
  "vpgatherqq\0"
  "vphaddbd\0"
  "vphaddbq\0"
  "vphaddbw\0"
  "vphaddd\0"
  "vphadddq\0"
  "vphaddsw\0"
  "vphaddubd\0"
  "vphaddubq\0"
  "vphaddubw\0"
  "vphaddudq\0"
  "vphadduwd\0"
  "vphadduwq\0"
  "vphaddw\0"
  "vphaddwd\0"
  "vphaddwq\0"
  "vphminposuw\0"
  "vphsubbw\0"
  "vphsubd\0"
  "vphsubdq\0"
  "vphsubsw\0"
  "vphsubw\0"
  "vphsubwd\0"
  "vpinsrb\0"
  "vpinsrd\0"
  "vpinsrq\0"
  "vpinsrw\0"
  "vpmacsdd\0"
  "vpmacsdqh\0"
  "vpmacsdql\0"
  "vpmacssdd\0"
  "vpmacssdqh\0"
  "vpmacssdql\0"
  "vpmacsswd\0"
  "vpmacssww\0"
  "vpmacswd\0"
  "vpmacsww\0"
  "vpmadcsswd\0"
  "vpmadcswd\0"
  "vpmaddubsw\0"
  "vpmaddwd\0"
  "vpmaskmovd\0"
  "vpmaskmovq\0"
  "vpmaxsb\0"
  "vpmaxsd\0"
  "vpmaxsw\0"
  "vpmaxub\0"
  "vpmaxud\0"
  "vpmaxuw\0"
  "vpminsb\0"
  "vpminsd\0"
  "vpminsw\0"
  "vpminub\0"
  "vpminud\0"
  "vpminuw\0"
  "vpmovmskb\0"
  "vpmovsxbd\0"
  "vpmovsxbq\0"
  "vpmovsxbw\0"
  "vpmovsxdq\0"
  "vpmovsxwd\0"
  "vpmovsxwq\0"
  "vpmovzxbd\0"
  "vpmovzxbq\0"
  "vpmovzxbw\0"
  "vpmovzxdq\0"
  "vpmovzxwd\0"
  "vpmovzxwq\0"
  "vpmuldq\0"
  "vpmulhrsw\0"
  "vpmulhuw\0"
  "vpmulhw\0"
  "vpmulld\0"
  "vpmullw\0"
  "vpmuludq\0"
  "vpor\0"
  "vpperm\0"
  "vprotb\0"
  "vprotd\0"
  "vprotq\0"
  "vprotw\0"
  "vpsadbw\0"
  "vpshab\0"
  "vpshad\0"
  "vpshaq\0"
  "vpshaw\0"
  "vpshlb\0"
  "vpshld\0"
  "vpshlq\0"
  "vpshlw\0"
  "vpshufb\0"
  "vpshufd\0"
  "vpshufhw\0"
  "vpshuflw\0"
  "vpsignb\0"
  "vpsignd\0"
  "vpsignw\0"
  "vpslld\0"
  "vpslldq\0"
  "vpsllq\0"
  "vpsllvd\0"
  "vpsllvq\0"
  "vpsllw\0"
  "vpsrad\0"
  "vpsravd\0"
  "vpsraw\0"
  "vpsrld\0"
  "vpsrldq\0"
  "vpsrlq\0"
  "vpsrlvd\0"
  "vpsrlvq\0"
  "vpsrlw\0"
  "vpsubb\0"
  "vpsubd\0"
  "vpsubq\0"
  "vpsubsb\0"
  "vpsubsw\0"
  "vpsubusb\0"
  "vpsubusw\0"
  "vpsubw\0"
  "vptest\0"
  "vpunpckhbw\0"
  "vpunpckhdq\0"
  "vpunpckhqdq\0"
  "vpunpckhwd\0"
  "vpunpcklbw\0"
  "vpunpckldq\0"
  "vpunpcklqdq\0"
  "vpunpcklwd\0"
  "vpxor\0"
  "vrcpps\0"
  "vrcpss\0"
  "vroundpd\0"
  "vroundps\0"
  "vroundsd\0"
  "vroundss\0"
  "vrsqrtps\0"
  "vrsqrtss\0"
  "vshufpd\0"
  "vshufps\0"
  "vsqrtpd\0"
  "vsqrtps\0"
  "vsqrtsd\0"
  "vsqrtss\0"
  "vstmxcsr\0"
  "vsubpd\0"
  "vsubps\0"
  "vsubsd\0"
  "vsubss\0"
  "vtestpd\0"
  "vtestps\0"
  "vucomisd\0"
  "vucomiss\0"
  "vunpckhpd\0"
  "vunpckhps\0"
  "vunpcklpd\0"
  "vunpcklps\0"
  "vxorpd\0"
  "vxorps\0"
  "vzeroall\0"
  "vzeroupper\0"
  "wrfsbase\0"
  "wrgsbase\0"
  "xadd\0"
  "xgetbv\0"
  "xrstor64\0"
  "xsave64\0"
  "xsaveopt\0"
  "xsaveopt64\0"
  "xsetbv";

// Automatically generated, do not edit.
enum X86InstAlphaIndex {
  kX86InstAlphaIndexFirst = 'a',
  kX86InstAlphaIndexLast = 'z',
  kX86InstAlphaIndexInvalid = 0xFFFF
};

// Automatically generated, do not edit.
static const uint16_t _x86InstAlphaIndex[26] = {
  kX86InstIdAdc,
  kX86InstIdBextr,
  kX86InstIdCall,
  kX86InstIdDaa,
  kX86InstIdEmms,
  kX86InstIdF2xm1,
  0xFFFF,
  kX86InstIdHaddpd,
  kX86InstIdIdiv,
  kX86InstIdJa,
  0xFFFF,
  kX86InstIdLahf,
  kX86InstIdMaskmovdqu,
  kX86InstIdNeg,
  kX86InstIdOr,
  kX86InstIdPabsb,
  0xFFFF,
  kX86InstIdRcl,
  kX86InstIdSahf,
  kX86InstIdTest,
  kX86InstIdUcomisd,
  kX86InstIdVaddpd,
  kX86InstIdWrfsbase,
  kX86InstIdXadd,
  0xFFFF,
  0xFFFF
};

// Automatically generated, do not edit.
enum X86InstData_NameIndex {
  kInstIdNone_NameIndex = 0,
  kX86InstIdAdc_NameIndex = 1,
  kX86InstIdAdd_NameIndex = 574,
  kX86InstIdAddpd_NameIndex = 3220,
  kX86InstIdAddps_NameIndex = 3232,
  kX86InstIdAddsd_NameIndex = 3454,
  kX86InstIdAddss_NameIndex = 3464,
  kX86InstIdAddsubpd_NameIndex = 2959,
  kX86InstIdAddsubps_NameIndex = 2971,
  kX86InstIdAesdec_NameIndex = 2193,
  kX86InstIdAesdeclast_NameIndex = 2201,
  kX86InstIdAesenc_NameIndex = 2213,
  kX86InstIdAesenclast_NameIndex = 2221,
  kX86InstIdAesimc_NameIndex = 2233,
  kX86InstIdAeskeygenassist_NameIndex = 2241,
  kX86InstIdAnd_NameIndex = 1538,
  kX86InstIdAndn_NameIndex = 4313,
  kX86InstIdAndnpd_NameIndex = 2258,
  kX86InstIdAndnps_NameIndex = 2266,
  kX86InstIdAndpd_NameIndex = 2274,
  kX86InstIdAndps_NameIndex = 2281,
  kX86InstIdBextr_NameIndex = 5,
  kX86InstIdBlendpd_NameIndex = 2288,
  kX86InstIdBlendps_NameIndex = 2297,
  kX86InstIdBlendvpd_NameIndex = 2306,
  kX86InstIdBlendvps_NameIndex = 2316,
  kX86InstIdBlsi_NameIndex = 11,
  kX86InstIdBlsmsk_NameIndex = 16,
  kX86InstIdBlsr_NameIndex = 23,
  kX86InstIdBsf_NameIndex = 28,
  kX86InstIdBsr_NameIndex = 32,
  kX86InstIdBswap_NameIndex = 36,
  kX86InstIdBt_NameIndex = 42,
  kX86InstIdBtc_NameIndex = 45,
  kX86InstIdBtr_NameIndex = 49,
  kX86InstIdBts_NameIndex = 53,
  kX86InstIdBzhi_NameIndex = 57,
  kX86InstIdCall_NameIndex = 62,
  kX86InstIdCbw_NameIndex = 67,
  kX86InstIdCdq_NameIndex = 71,
  kX86InstIdCdqe_NameIndex = 75,
  kX86InstIdClc_NameIndex = 80,
  kX86InstIdCld_NameIndex = 84,
  kX86InstIdClflush_NameIndex = 88,
  kX86InstIdCmc_NameIndex = 96,
  kX86InstIdCmova_NameIndex = 100,
  kX86InstIdCmovae_NameIndex = 106,
  kX86InstIdCmovb_NameIndex = 431,
  kX86InstIdCmovbe_NameIndex = 438,
  kX86InstIdCmovc_NameIndex = 113,
  kX86InstIdCmove_NameIndex = 446,
  kX86InstIdCmovg_NameIndex = 119,
  kX86InstIdCmovge_NameIndex = 125,
  kX86InstIdCmovl_NameIndex = 132,
  kX86InstIdCmovle_NameIndex = 138,
  kX86InstIdCmovna_NameIndex = 145,
  kX86InstIdCmovnae_NameIndex = 152,
  kX86InstIdCmovnb_NameIndex = 453,
  kX86InstIdCmovnbe_NameIndex = 461,
  kX86InstIdCmovnc_NameIndex = 160,
  kX86InstIdCmovne_NameIndex = 470,
  kX86InstIdCmovng_NameIndex = 167,
  kX86InstIdCmovnge_NameIndex = 174,
  kX86InstIdCmovnl_NameIndex = 182,
  kX86InstIdCmovnle_NameIndex = 189,
  kX86InstIdCmovno_NameIndex = 197,
  kX86InstIdCmovnp_NameIndex = 204,
  kX86InstIdCmovns_NameIndex = 211,
  kX86InstIdCmovnz_NameIndex = 218,
  kX86InstIdCmovo_NameIndex = 225,
  kX86InstIdCmovp_NameIndex = 231,
  kX86InstIdCmovpe_NameIndex = 237,
  kX86InstIdCmovpo_NameIndex = 244,
  kX86InstIdCmovs_NameIndex = 251,
  kX86InstIdCmovz_NameIndex = 257,
  kX86InstIdCmp_NameIndex = 263,
  kX86InstIdCmppd_NameIndex = 2382,
  kX86InstIdCmpps_NameIndex = 2389,
  kX86InstIdCmpsB_NameIndex = 1789,
  kX86InstIdCmpsD_NameIndex = 1802,
  kX86InstIdCmpsQ_NameIndex = 1815,
  kX86InstIdCmpsW_NameIndex = 1828,
  kX86InstIdCmpsd_NameIndex = 2396,
  kX86InstIdCmpss_NameIndex = 2403,
  kX86InstIdCmpxchg_NameIndex = 267,
  kX86InstIdCmpxchg16b_NameIndex = 275,
  kX86InstIdCmpxchg8b_NameIndex = 286,
  kX86InstIdComisd_NameIndex = 6069,
  kX86InstIdComiss_NameIndex = 6078,
  kX86InstIdCpuid_NameIndex = 296,
  kX86InstIdCqo_NameIndex = 302,
  kX86InstIdCrc32_NameIndex = 306,
  kX86InstIdCvtdq2pd_NameIndex = 2426,
  kX86InstIdCvtdq2ps_NameIndex = 2436,
  kX86InstIdCvtpd2dq_NameIndex = 2446,
  kX86InstIdCvtpd2pi_NameIndex = 312,
  kX86InstIdCvtpd2ps_NameIndex = 2456,
  kX86InstIdCvtpi2pd_NameIndex = 321,
  kX86InstIdCvtpi2ps_NameIndex = 330,
  kX86InstIdCvtps2dq_NameIndex = 2476,
  kX86InstIdCvtps2pd_NameIndex = 2486,
  kX86InstIdCvtps2pi_NameIndex = 339,
  kX86InstIdCvtsd2si_NameIndex = 2506,
  kX86InstIdCvtsd2ss_NameIndex = 2516,
  kX86InstIdCvtsi2sd_NameIndex = 2526,
  kX86InstIdCvtsi2ss_NameIndex = 2536,
  kX86InstIdCvtss2sd_NameIndex = 2546,
  kX86InstIdCvtss2si_NameIndex = 2556,
  kX86InstIdCvttpd2dq_NameIndex = 2566,
  kX86InstIdCvttpd2pi_NameIndex = 348,
  kX86InstIdCvttps2dq_NameIndex = 2577,
  kX86InstIdCvttps2pi_NameIndex = 358,
  kX86InstIdCvttsd2si_NameIndex = 2588,
  kX86InstIdCvttss2si_NameIndex = 2599,
  kX86InstIdCwd_NameIndex = 368,
  kX86InstIdCwde_NameIndex = 372,
  kX86InstIdDaa_NameIndex = 377,
  kX86InstIdDas_NameIndex = 381,
  kX86InstIdDec_NameIndex = 2196,
  kX86InstIdDiv_NameIndex = 593,
  kX86InstIdDivpd_NameIndex = 2610,
  kX86InstIdDivps_NameIndex = 2617,
  kX86InstIdDivsd_NameIndex = 2624,
  kX86InstIdDivss_NameIndex = 2631,
  kX86InstIdDppd_NameIndex = 2638,
  kX86InstIdDpps_NameIndex = 2644,
  kX86InstIdEmms_NameIndex = 561,
  kX86InstIdEnter_NameIndex = 385,
  kX86InstIdExtractps_NameIndex = 2676,
  kX86InstIdExtrq_NameIndex = 4718,
  kX86InstIdF2xm1_NameIndex = 391,
  kX86InstIdFabs_NameIndex = 397,
  kX86InstIdFadd_NameIndex = 1289,
  kX86InstIdFaddp_NameIndex = 402,
  kX86InstIdFbld_NameIndex = 408,
  kX86InstIdFbstp_NameIndex = 413,
  kX86InstIdFchs_NameIndex = 419,
  kX86InstIdFclex_NameIndex = 424,
  kX86InstIdFcmovb_NameIndex = 430,
  kX86InstIdFcmovbe_NameIndex = 437,
  kX86InstIdFcmove_NameIndex = 445,
  kX86InstIdFcmovnb_NameIndex = 452,
  kX86InstIdFcmovnbe_NameIndex = 460,
  kX86InstIdFcmovne_NameIndex = 469,
  kX86InstIdFcmovnu_NameIndex = 477,
  kX86InstIdFcmovu_NameIndex = 485,
  kX86InstIdFcom_NameIndex = 492,
  kX86InstIdFcomi_NameIndex = 497,
  kX86InstIdFcomip_NameIndex = 503,
  kX86InstIdFcomp_NameIndex = 510,
  kX86InstIdFcompp_NameIndex = 516,
  kX86InstIdFcos_NameIndex = 523,
  kX86InstIdFdecstp_NameIndex = 528,
  kX86InstIdFdiv_NameIndex = 536,
  kX86InstIdFdivp_NameIndex = 541,
  kX86InstIdFdivr_NameIndex = 547,
  kX86InstIdFdivrp_NameIndex = 553,
  kX86InstIdFemms_NameIndex = 560,
  kX86InstIdFfree_NameIndex = 566,
  kX86InstIdFiadd_NameIndex = 572,
  kX86InstIdFicom_NameIndex = 578,
  kX86InstIdFicomp_NameIndex = 584,
  kX86InstIdFidiv_NameIndex = 591,
  kX86InstIdFidivr_NameIndex = 597,
  kX86InstIdFild_NameIndex = 604,
  kX86InstIdFimul_NameIndex = 609,
  kX86InstIdFincstp_NameIndex = 615,
  kX86InstIdFinit_NameIndex = 623,
  kX86InstIdFist_NameIndex = 629,
  kX86InstIdFistp_NameIndex = 634,
  kX86InstIdFisttp_NameIndex = 640,
  kX86InstIdFisub_NameIndex = 647,
  kX86InstIdFisubr_NameIndex = 653,
  kX86InstIdFld_NameIndex = 660,
  kX86InstIdFld1_NameIndex = 664,
  kX86InstIdFldcw_NameIndex = 669,
  kX86InstIdFldenv_NameIndex = 675,
  kX86InstIdFldl2e_NameIndex = 682,
  kX86InstIdFldl2t_NameIndex = 689,
  kX86InstIdFldlg2_NameIndex = 696,
  kX86InstIdFldln2_NameIndex = 703,
  kX86InstIdFldpi_NameIndex = 710,
  kX86InstIdFldz_NameIndex = 716,
  kX86InstIdFmul_NameIndex = 1331,
  kX86InstIdFmulp_NameIndex = 721,
  kX86InstIdFnclex_NameIndex = 727,
  kX86InstIdFninit_NameIndex = 734,
  kX86InstIdFnop_NameIndex = 741,
  kX86InstIdFnsave_NameIndex = 746,
  kX86InstIdFnstcw_NameIndex = 753,
  kX86InstIdFnstenv_NameIndex = 760,
  kX86InstIdFnstsw_NameIndex = 768,
  kX86InstIdFpatan_NameIndex = 775,
  kX86InstIdFprem_NameIndex = 782,
  kX86InstIdFprem1_NameIndex = 788,
  kX86InstIdFptan_NameIndex = 795,
  kX86InstIdFrndint_NameIndex = 801,
  kX86InstIdFrstor_NameIndex = 809,
  kX86InstIdFsave_NameIndex = 816,
  kX86InstIdFscale_NameIndex = 822,
  kX86InstIdFsin_NameIndex = 829,
  kX86InstIdFsincos_NameIndex = 834,
  kX86InstIdFsqrt_NameIndex = 842,
  kX86InstIdFst_NameIndex = 848,
  kX86InstIdFstcw_NameIndex = 852,
  kX86InstIdFstenv_NameIndex = 858,
  kX86InstIdFstp_NameIndex = 865,
  kX86InstIdFstsw_NameIndex = 870,
  kX86InstIdFsub_NameIndex = 1393,
  kX86InstIdFsubp_NameIndex = 876,
  kX86InstIdFsubr_NameIndex = 1399,
  kX86InstIdFsubrp_NameIndex = 882,
  kX86InstIdFtst_NameIndex = 889,
  kX86InstIdFucom_NameIndex = 894,
  kX86InstIdFucomi_NameIndex = 900,
  kX86InstIdFucomip_NameIndex = 907,
  kX86InstIdFucomp_NameIndex = 915,
  kX86InstIdFucompp_NameIndex = 922,
  kX86InstIdFwait_NameIndex = 930,
  kX86InstIdFxam_NameIndex = 936,
  kX86InstIdFxch_NameIndex = 941,
  kX86InstIdFxrstor_NameIndex = 946,
  kX86InstIdFxsave_NameIndex = 954,
  kX86InstIdFxtract_NameIndex = 961,
  kX86InstIdFyl2x_NameIndex = 969,
  kX86InstIdFyl2xp1_NameIndex = 975,
  kX86InstIdHaddpd_NameIndex = 3743,
  kX86InstIdHaddps_NameIndex = 3751,
  kX86InstIdHsubpd_NameIndex = 3759,
  kX86InstIdHsubps_NameIndex = 3767,
  kX86InstIdIdiv_NameIndex = 592,
  kX86InstIdImul_NameIndex = 610,
  kX86InstIdInc_NameIndex = 983,
  kX86InstIdInsertps_NameIndex = 3799,
  kX86InstIdInsertq_NameIndex = 987,
  kX86InstIdInt_NameIndex = 805,
  kX86InstIdJa_NameIndex = 995,
  kX86InstIdJae_NameIndex = 998,
  kX86InstIdJb_NameIndex = 1002,
  kX86InstIdJbe_NameIndex = 1005,
  kX86InstIdJc_NameIndex = 1009,
  kX86InstIdJe_NameIndex = 1012,
  kX86InstIdJg_NameIndex = 1021,
  kX86InstIdJge_NameIndex = 1024,
  kX86InstIdJl_NameIndex = 1028,
  kX86InstIdJle_NameIndex = 1031,
  kX86InstIdJna_NameIndex = 1039,
  kX86InstIdJnae_NameIndex = 1043,
  kX86InstIdJnb_NameIndex = 1048,
  kX86InstIdJnbe_NameIndex = 1052,
  kX86InstIdJnc_NameIndex = 1057,
  kX86InstIdJne_NameIndex = 1061,
  kX86InstIdJng_NameIndex = 1065,
  kX86InstIdJnge_NameIndex = 1069,
  kX86InstIdJnl_NameIndex = 1074,
  kX86InstIdJnle_NameIndex = 1078,
  kX86InstIdJno_NameIndex = 1083,
  kX86InstIdJnp_NameIndex = 1087,
  kX86InstIdJns_NameIndex = 1091,
  kX86InstIdJnz_NameIndex = 1095,
  kX86InstIdJo_NameIndex = 1099,
  kX86InstIdJp_NameIndex = 1102,
  kX86InstIdJpe_NameIndex = 1105,
  kX86InstIdJpo_NameIndex = 1109,
  kX86InstIdJs_NameIndex = 1113,
  kX86InstIdJz_NameIndex = 1116,
  kX86InstIdJecxz_NameIndex = 1015,
  kX86InstIdJmp_NameIndex = 1035,
  kX86InstIdLahf_NameIndex = 1119,
  kX86InstIdLddqu_NameIndex = 3809,
  kX86InstIdLdmxcsr_NameIndex = 3816,
  kX86InstIdLea_NameIndex = 1124,
  kX86InstIdLeave_NameIndex = 1128,
  kX86InstIdLfence_NameIndex = 1134,
  kX86InstIdLodsB_NameIndex = 1559,
  kX86InstIdLodsD_NameIndex = 1570,
  kX86InstIdLodsQ_NameIndex = 1581,
  kX86InstIdLodsW_NameIndex = 1592,
  kX86InstIdLzcnt_NameIndex = 1141,
  kX86InstIdMaskmovdqu_NameIndex = 3825,
  kX86InstIdMaskmovq_NameIndex = 5164,
  kX86InstIdMaxpd_NameIndex = 3859,
  kX86InstIdMaxps_NameIndex = 3866,
  kX86InstIdMaxsd_NameIndex = 5183,
  kX86InstIdMaxss_NameIndex = 3880,
  kX86InstIdMfence_NameIndex = 1147,
  kX86InstIdMinpd_NameIndex = 3887,
  kX86InstIdMinps_NameIndex = 3894,
  kX86InstIdMinsd_NameIndex = 5231,
  kX86InstIdMinss_NameIndex = 3908,
  kX86InstIdMonitor_NameIndex = 1154,
  kX86InstIdMov_NameIndex = 4426,
  kX86InstIdMovPtr_NameIndex = 1162,
  kX86InstIdMovapd_NameIndex = 3915,
  kX86InstIdMovaps_NameIndex = 3923,
  kX86InstIdMovbe_NameIndex = 439,
  kX86InstIdMovd_NameIndex = 5157,
  kX86InstIdMovddup_NameIndex = 3937,
  kX86InstIdMovdq2q_NameIndex = 1170,
  kX86InstIdMovdqa_NameIndex = 3946,
  kX86InstIdMovdqu_NameIndex = 3829,
  kX86InstIdMovhlps_NameIndex = 3962,
  kX86InstIdMovhpd_NameIndex = 3971,
  kX86InstIdMovhps_NameIndex = 3979,
  kX86InstIdMovlhps_NameIndex = 3987,
  kX86InstIdMovlpd_NameIndex = 3996,
  kX86InstIdMovlps_NameIndex = 4004,
  kX86InstIdMovmskpd_NameIndex = 4012,
  kX86InstIdMovmskps_NameIndex = 4022,
  kX86InstIdMovntdq_NameIndex = 4032,
  kX86InstIdMovntdqa_NameIndex = 4041,
  kX86InstIdMovnti_NameIndex = 1178,
  kX86InstIdMovntpd_NameIndex = 4051,
  kX86InstIdMovntps_NameIndex = 4060,
  kX86InstIdMovntq_NameIndex = 1185,
  kX86InstIdMovntsd_NameIndex = 1192,
  kX86InstIdMovntss_NameIndex = 1200,
  kX86InstIdMovq_NameIndex = 5168,
  kX86InstIdMovq2dq_NameIndex = 1208,
  kX86InstIdMovsB_NameIndex = 1603,
  kX86InstIdMovsD_NameIndex = 1614,
  kX86InstIdMovsQ_NameIndex = 1625,
  kX86InstIdMovsW_NameIndex = 1636,
  kX86InstIdMovsd_NameIndex = 4075,
  kX86InstIdMovshdup_NameIndex = 4082,
  kX86InstIdMovsldup_NameIndex = 4092,
  kX86InstIdMovss_NameIndex = 4102,
  kX86InstIdMovsx_NameIndex = 1216,
  kX86InstIdMovsxd_NameIndex = 1222,
  kX86InstIdMovupd_NameIndex = 4109,
  kX86InstIdMovups_NameIndex = 4117,
  kX86InstIdMovzx_NameIndex = 1229,
  kX86InstIdMpsadbw_NameIndex = 4125,
  kX86InstIdMul_NameIndex = 611,
  kX86InstIdMulpd_NameIndex = 4134,
  kX86InstIdMulps_NameIndex = 4141,
  kX86InstIdMulsd_NameIndex = 4148,
  kX86InstIdMulss_NameIndex = 4155,
  kX86InstIdMulx_NameIndex = 1235,
  kX86InstIdMwait_NameIndex = 1240,
  kX86InstIdNeg_NameIndex = 1246,
  kX86InstIdNop_NameIndex = 742,
  kX86InstIdNot_NameIndex = 1250,
  kX86InstIdOr_NameIndex = 951,
  kX86InstIdOrpd_NameIndex = 6127,
  kX86InstIdOrps_NameIndex = 6134,
  kX86InstIdPabsb_NameIndex = 4174,
  kX86InstIdPabsd_NameIndex = 4181,
  kX86InstIdPabsw_NameIndex = 4188,
  kX86InstIdPackssdw_NameIndex = 4195,
  kX86InstIdPacksswb_NameIndex = 4205,
  kX86InstIdPackusdw_NameIndex = 4215,
  kX86InstIdPackuswb_NameIndex = 4225,
  kX86InstIdPaddb_NameIndex = 4235,
  kX86InstIdPaddd_NameIndex = 4242,
  kX86InstIdPaddq_NameIndex = 4249,
  kX86InstIdPaddsb_NameIndex = 4256,
  kX86InstIdPaddsw_NameIndex = 4264,
  kX86InstIdPaddusb_NameIndex = 4272,
  kX86InstIdPaddusw_NameIndex = 4281,
  kX86InstIdPaddw_NameIndex = 4290,
  kX86InstIdPalignr_NameIndex = 4297,
  kX86InstIdPand_NameIndex = 4306,
  kX86InstIdPandn_NameIndex = 4312,
  kX86InstIdPause_NameIndex = 1254,
  kX86InstIdPavgb_NameIndex = 4319,
  kX86InstIdPavgw_NameIndex = 4326,
  kX86InstIdPblendvb_NameIndex = 4342,
  kX86InstIdPblendw_NameIndex = 4352,
  kX86InstIdPclmulqdq_NameIndex = 4413,
  kX86InstIdPcmpeqb_NameIndex = 4431,
  kX86InstIdPcmpeqd_NameIndex = 4440,
  kX86InstIdPcmpeqq_NameIndex = 4449,
  kX86InstIdPcmpeqw_NameIndex = 4458,
  kX86InstIdPcmpestri_NameIndex = 4467,
  kX86InstIdPcmpestrm_NameIndex = 4478,
  kX86InstIdPcmpgtb_NameIndex = 4489,
  kX86InstIdPcmpgtd_NameIndex = 4498,
  kX86InstIdPcmpgtq_NameIndex = 4507,
  kX86InstIdPcmpgtw_NameIndex = 4516,
  kX86InstIdPcmpistri_NameIndex = 4525,
  kX86InstIdPcmpistrm_NameIndex = 4536,
  kX86InstIdPdep_NameIndex = 1260,
  kX86InstIdPext_NameIndex = 1265,
  kX86InstIdPextrb_NameIndex = 4701,
  kX86InstIdPextrd_NameIndex = 4709,
  kX86InstIdPextrq_NameIndex = 4717,
  kX86InstIdPextrw_NameIndex = 4725,
  kX86InstIdPf2id_NameIndex = 1270,
  kX86InstIdPf2iw_NameIndex = 1276,
  kX86InstIdPfacc_NameIndex = 1282,
  kX86InstIdPfadd_NameIndex = 1288,
  kX86InstIdPfcmpeq_NameIndex = 1294,
  kX86InstIdPfcmpge_NameIndex = 1302,
  kX86InstIdPfcmpgt_NameIndex = 1310,
  kX86InstIdPfmax_NameIndex = 1318,
  kX86InstIdPfmin_NameIndex = 1324,
  kX86InstIdPfmul_NameIndex = 1330,
  kX86InstIdPfnacc_NameIndex = 1336,
  kX86InstIdPfpnacc_NameIndex = 1343,
  kX86InstIdPfrcp_NameIndex = 1351,
  kX86InstIdPfrcpit1_NameIndex = 1357,
  kX86InstIdPfrcpit2_NameIndex = 1366,
  kX86InstIdPfrsqit1_NameIndex = 1375,
  kX86InstIdPfrsqrt_NameIndex = 1384,
  kX86InstIdPfsub_NameIndex = 1392,
  kX86InstIdPfsubr_NameIndex = 1398,
  kX86InstIdPhaddd_NameIndex = 4804,
  kX86InstIdPhaddsw_NameIndex = 4821,
  kX86InstIdPhaddw_NameIndex = 4890,
  kX86InstIdPhminposuw_NameIndex = 4916,
  kX86InstIdPhsubd_NameIndex = 4937,
  kX86InstIdPhsubsw_NameIndex = 4954,
  kX86InstIdPhsubw_NameIndex = 4963,
  kX86InstIdPi2fd_NameIndex = 1405,
  kX86InstIdPi2fw_NameIndex = 1411,
  kX86InstIdPinsrb_NameIndex = 4980,
  kX86InstIdPinsrd_NameIndex = 4988,
  kX86InstIdPinsrq_NameIndex = 4996,
  kX86InstIdPinsrw_NameIndex = 5004,
  kX86InstIdPmaddubsw_NameIndex = 5132,
  kX86InstIdPmaddwd_NameIndex = 5143,
  kX86InstIdPmaxsb_NameIndex = 5174,
  kX86InstIdPmaxsd_NameIndex = 5182,
  kX86InstIdPmaxsw_NameIndex = 5190,
  kX86InstIdPmaxub_NameIndex = 5198,
  kX86InstIdPmaxud_NameIndex = 5206,
  kX86InstIdPmaxuw_NameIndex = 5214,
  kX86InstIdPminsb_NameIndex = 5222,
  kX86InstIdPminsd_NameIndex = 5230,
  kX86InstIdPminsw_NameIndex = 5238,
  kX86InstIdPminub_NameIndex = 5246,
  kX86InstIdPminud_NameIndex = 5254,
  kX86InstIdPminuw_NameIndex = 5262,
  kX86InstIdPmovmskb_NameIndex = 5270,
  kX86InstIdPmovsxbd_NameIndex = 5280,
  kX86InstIdPmovsxbq_NameIndex = 5290,
  kX86InstIdPmovsxbw_NameIndex = 5300,
  kX86InstIdPmovsxdq_NameIndex = 5310,
  kX86InstIdPmovsxwd_NameIndex = 5320,
  kX86InstIdPmovsxwq_NameIndex = 5330,
  kX86InstIdPmovzxbd_NameIndex = 5340,
  kX86InstIdPmovzxbq_NameIndex = 5350,
  kX86InstIdPmovzxbw_NameIndex = 5360,
  kX86InstIdPmovzxdq_NameIndex = 5370,
  kX86InstIdPmovzxwd_NameIndex = 5380,
  kX86InstIdPmovzxwq_NameIndex = 5390,
  kX86InstIdPmuldq_NameIndex = 5400,
  kX86InstIdPmulhrsw_NameIndex = 5408,
  kX86InstIdPmulhuw_NameIndex = 5418,
  kX86InstIdPmulhw_NameIndex = 5427,
  kX86InstIdPmulld_NameIndex = 5435,
  kX86InstIdPmullw_NameIndex = 5443,
  kX86InstIdPmuludq_NameIndex = 5451,
  kX86InstIdPop_NameIndex = 1417,
  kX86InstIdPopa_NameIndex = 1421,
  kX86InstIdPopcnt_NameIndex = 1426,
  kX86InstIdPopf_NameIndex = 1433,
  kX86InstIdPor_NameIndex = 5460,
  kX86InstIdPrefetch_NameIndex = 1438,
  kX86InstIdPrefetch3dNow_NameIndex = 1447,
  kX86InstIdPrefetchw3dNow_NameIndex = 1462,
  kX86InstIdPsadbw_NameIndex = 4126,
  kX86InstIdPshufb_NameIndex = 5564,
  kX86InstIdPshufd_NameIndex = 5572,
  kX86InstIdPshufhw_NameIndex = 5580,
  kX86InstIdPshuflw_NameIndex = 5589,
  kX86InstIdPshufw_NameIndex = 1478,
  kX86InstIdPsignb_NameIndex = 5598,
  kX86InstIdPsignd_NameIndex = 5606,
  kX86InstIdPsignw_NameIndex = 5614,
  kX86InstIdPslld_NameIndex = 5622,
  kX86InstIdPslldq_NameIndex = 5629,
  kX86InstIdPsllq_NameIndex = 5637,
  kX86InstIdPsllw_NameIndex = 5660,
  kX86InstIdPsrad_NameIndex = 5667,
  kX86InstIdPsraw_NameIndex = 5682,
  kX86InstIdPsrld_NameIndex = 5689,
  kX86InstIdPsrldq_NameIndex = 5696,
  kX86InstIdPsrlq_NameIndex = 5704,
  kX86InstIdPsrlw_NameIndex = 5727,
  kX86InstIdPsubb_NameIndex = 5734,
  kX86InstIdPsubd_NameIndex = 5741,
  kX86InstIdPsubq_NameIndex = 5748,
  kX86InstIdPsubsb_NameIndex = 5755,
  kX86InstIdPsubsw_NameIndex = 5763,
  kX86InstIdPsubusb_NameIndex = 5771,
  kX86InstIdPsubusw_NameIndex = 5780,
  kX86InstIdPsubw_NameIndex = 5789,
  kX86InstIdPswapd_NameIndex = 1485,
  kX86InstIdPtest_NameIndex = 5796,
  kX86InstIdPunpckhbw_NameIndex = 5803,
  kX86InstIdPunpckhdq_NameIndex = 5814,
  kX86InstIdPunpckhqdq_NameIndex = 5825,
  kX86InstIdPunpckhwd_NameIndex = 5837,
  kX86InstIdPunpcklbw_NameIndex = 5848,
  kX86InstIdPunpckldq_NameIndex = 5859,
  kX86InstIdPunpcklqdq_NameIndex = 5870,
  kX86InstIdPunpcklwd_NameIndex = 5882,
  kX86InstIdPush_NameIndex = 1492,
  kX86InstIdPusha_NameIndex = 1497,
  kX86InstIdPushf_NameIndex = 1503,
  kX86InstIdPxor_NameIndex = 5893,
  kX86InstIdRcl_NameIndex = 1509,
  kX86InstIdRcpps_NameIndex = 5899,
  kX86InstIdRcpss_NameIndex = 5906,
  kX86InstIdRcr_NameIndex = 1513,
  kX86InstIdRdfsbase_NameIndex = 1517,
  kX86InstIdRdgsbase_NameIndex = 1526,
  kX86InstIdRdrand_NameIndex = 1535,
  kX86InstIdRdtsc_NameIndex = 1542,
  kX86InstIdRdtscp_NameIndex = 1548,
  kX86InstIdRepLodsB_NameIndex = 1555,
  kX86InstIdRepLodsD_NameIndex = 1566,
  kX86InstIdRepLodsQ_NameIndex = 1577,
  kX86InstIdRepLodsW_NameIndex = 1588,
  kX86InstIdRepMovsB_NameIndex = 1599,
  kX86InstIdRepMovsD_NameIndex = 1610,
  kX86InstIdRepMovsQ_NameIndex = 1621,
  kX86InstIdRepMovsW_NameIndex = 1632,
  kX86InstIdRepStosB_NameIndex = 1643,
  kX86InstIdRepStosD_NameIndex = 1654,
  kX86InstIdRepStosQ_NameIndex = 1665,
  kX86InstIdRepStosW_NameIndex = 1676,
  kX86InstIdRepeCmpsB_NameIndex = 1687,
  kX86InstIdRepeCmpsD_NameIndex = 1699,
  kX86InstIdRepeCmpsQ_NameIndex = 1711,
  kX86InstIdRepeCmpsW_NameIndex = 1723,
  kX86InstIdRepeScasB_NameIndex = 1735,
  kX86InstIdRepeScasD_NameIndex = 1747,
  kX86InstIdRepeScasQ_NameIndex = 1759,
  kX86InstIdRepeScasW_NameIndex = 1771,
  kX86InstIdRepneCmpsB_NameIndex = 1783,
  kX86InstIdRepneCmpsD_NameIndex = 1796,
  kX86InstIdRepneCmpsQ_NameIndex = 1809,
  kX86InstIdRepneCmpsW_NameIndex = 1822,
  kX86InstIdRepneScasB_NameIndex = 1835,
  kX86InstIdRepneScasD_NameIndex = 1848,
  kX86InstIdRepneScasQ_NameIndex = 1861,
  kX86InstIdRepneScasW_NameIndex = 1874,
  kX86InstIdRet_NameIndex = 1887,
  kX86InstIdRol_NameIndex = 1891,
  kX86InstIdRor_NameIndex = 1895,
  kX86InstIdRorx_NameIndex = 1899,
  kX86InstIdRoundpd_NameIndex = 5913,
  kX86InstIdRoundps_NameIndex = 5922,
  kX86InstIdRoundsd_NameIndex = 5931,
  kX86InstIdRoundss_NameIndex = 5940,
  kX86InstIdRsqrtps_NameIndex = 5949,
  kX86InstIdRsqrtss_NameIndex = 5958,
  kX86InstIdSahf_NameIndex = 1904,
  kX86InstIdSal_NameIndex = 1909,
  kX86InstIdSar_NameIndex = 1913,
  kX86InstIdSarx_NameIndex = 1917,
  kX86InstIdSbb_NameIndex = 1922,
  kX86InstIdScasB_NameIndex = 1841,
  kX86InstIdScasD_NameIndex = 1854,
  kX86InstIdScasQ_NameIndex = 1867,
  kX86InstIdScasW_NameIndex = 1880,
  kX86InstIdSeta_NameIndex = 1926,
  kX86InstIdSetae_NameIndex = 1931,
  kX86InstIdSetb_NameIndex = 1937,
  kX86InstIdSetbe_NameIndex = 1942,
  kX86InstIdSetc_NameIndex = 1948,
  kX86InstIdSete_NameIndex = 1953,
  kX86InstIdSetg_NameIndex = 1958,
  kX86InstIdSetge_NameIndex = 1963,
  kX86InstIdSetl_NameIndex = 1969,
  kX86InstIdSetle_NameIndex = 1974,
  kX86InstIdSetna_NameIndex = 1980,
  kX86InstIdSetnae_NameIndex = 1986,
  kX86InstIdSetnb_NameIndex = 1993,
  kX86InstIdSetnbe_NameIndex = 1999,
  kX86InstIdSetnc_NameIndex = 2006,
  kX86InstIdSetne_NameIndex = 2012,
  kX86InstIdSetng_NameIndex = 2018,
  kX86InstIdSetnge_NameIndex = 2024,
  kX86InstIdSetnl_NameIndex = 2031,
  kX86InstIdSetnle_NameIndex = 2037,
  kX86InstIdSetno_NameIndex = 2044,
  kX86InstIdSetnp_NameIndex = 2050,
  kX86InstIdSetns_NameIndex = 2056,
  kX86InstIdSetnz_NameIndex = 2062,
  kX86InstIdSeto_NameIndex = 2068,
  kX86InstIdSetp_NameIndex = 2073,
  kX86InstIdSetpe_NameIndex = 2078,
  kX86InstIdSetpo_NameIndex = 2084,
  kX86InstIdSets_NameIndex = 2090,
  kX86InstIdSetz_NameIndex = 2095,
  kX86InstIdSfence_NameIndex = 2100,
  kX86InstIdShl_NameIndex = 2107,
  kX86InstIdShld_NameIndex = 5544,
  kX86InstIdShlx_NameIndex = 2111,
  kX86InstIdShr_NameIndex = 2116,
  kX86InstIdShrd_NameIndex = 2120,
  kX86InstIdShrx_NameIndex = 2125,
  kX86InstIdShufpd_NameIndex = 5967,
  kX86InstIdShufps_NameIndex = 5975,
  kX86InstIdSqrtpd_NameIndex = 5983,
  kX86InstIdSqrtps_NameIndex = 5950,
  kX86InstIdSqrtsd_NameIndex = 5999,
  kX86InstIdSqrtss_NameIndex = 5959,
  kX86InstIdStc_NameIndex = 2130,
  kX86InstIdStd_NameIndex = 4382,
  kX86InstIdStmxcsr_NameIndex = 6015,
  kX86InstIdStosB_NameIndex = 1647,
  kX86InstIdStosD_NameIndex = 1658,
  kX86InstIdStosQ_NameIndex = 1669,
  kX86InstIdStosW_NameIndex = 1680,
  kX86InstIdSub_NameIndex = 649,
  kX86InstIdSubpd_NameIndex = 2962,
  kX86InstIdSubps_NameIndex = 2974,
  kX86InstIdSubsd_NameIndex = 3650,
  kX86InstIdSubss_NameIndex = 3660,
  kX86InstIdTest_NameIndex = 5797,
  kX86InstIdTzcnt_NameIndex = 2134,
  kX86InstIdUcomisd_NameIndex = 6068,
  kX86InstIdUcomiss_NameIndex = 6077,
  kX86InstIdUd2_NameIndex = 2140,
  kX86InstIdUnpckhpd_NameIndex = 6086,
  kX86InstIdUnpckhps_NameIndex = 6096,
  kX86InstIdUnpcklpd_NameIndex = 6106,
  kX86InstIdUnpcklps_NameIndex = 6116,
  kX86InstIdVaddpd_NameIndex = 2144,
  kX86InstIdVaddps_NameIndex = 2151,
  kX86InstIdVaddsd_NameIndex = 2158,
  kX86InstIdVaddss_NameIndex = 2165,
  kX86InstIdVaddsubpd_NameIndex = 2172,
  kX86InstIdVaddsubps_NameIndex = 2182,
  kX86InstIdVaesdec_NameIndex = 2192,
  kX86InstIdVaesdeclast_NameIndex = 2200,
  kX86InstIdVaesenc_NameIndex = 2212,
  kX86InstIdVaesenclast_NameIndex = 2220,
  kX86InstIdVaesimc_NameIndex = 2232,
  kX86InstIdVaeskeygenassist_NameIndex = 2240,
  kX86InstIdVandnpd_NameIndex = 2257,
  kX86InstIdVandnps_NameIndex = 2265,
  kX86InstIdVandpd_NameIndex = 2273,
  kX86InstIdVandps_NameIndex = 2280,
  kX86InstIdVblendpd_NameIndex = 2287,
  kX86InstIdVblendps_NameIndex = 2296,
  kX86InstIdVblendvpd_NameIndex = 2305,
  kX86InstIdVblendvps_NameIndex = 2315,
  kX86InstIdVbroadcastf128_NameIndex = 2325,
  kX86InstIdVbroadcasti128_NameIndex = 2340,
  kX86InstIdVbroadcastsd_NameIndex = 2355,
  kX86InstIdVbroadcastss_NameIndex = 2368,
  kX86InstIdVcmppd_NameIndex = 2381,
  kX86InstIdVcmpps_NameIndex = 2388,
  kX86InstIdVcmpsd_NameIndex = 2395,
  kX86InstIdVcmpss_NameIndex = 2402,
  kX86InstIdVcomisd_NameIndex = 2409,
  kX86InstIdVcomiss_NameIndex = 2417,
  kX86InstIdVcvtdq2pd_NameIndex = 2425,
  kX86InstIdVcvtdq2ps_NameIndex = 2435,
  kX86InstIdVcvtpd2dq_NameIndex = 2445,
  kX86InstIdVcvtpd2ps_NameIndex = 2455,
  kX86InstIdVcvtph2ps_NameIndex = 2465,
  kX86InstIdVcvtps2dq_NameIndex = 2475,
  kX86InstIdVcvtps2pd_NameIndex = 2485,
  kX86InstIdVcvtps2ph_NameIndex = 2495,
  kX86InstIdVcvtsd2si_NameIndex = 2505,
  kX86InstIdVcvtsd2ss_NameIndex = 2515,
  kX86InstIdVcvtsi2sd_NameIndex = 2525,
  kX86InstIdVcvtsi2ss_NameIndex = 2535,
  kX86InstIdVcvtss2sd_NameIndex = 2545,
  kX86InstIdVcvtss2si_NameIndex = 2555,
  kX86InstIdVcvttpd2dq_NameIndex = 2565,
  kX86InstIdVcvttps2dq_NameIndex = 2576,
  kX86InstIdVcvttsd2si_NameIndex = 2587,
  kX86InstIdVcvttss2si_NameIndex = 2598,
  kX86InstIdVdivpd_NameIndex = 2609,
  kX86InstIdVdivps_NameIndex = 2616,
  kX86InstIdVdivsd_NameIndex = 2623,
  kX86InstIdVdivss_NameIndex = 2630,
  kX86InstIdVdppd_NameIndex = 2637,
  kX86InstIdVdpps_NameIndex = 2643,
  kX86InstIdVextractf128_NameIndex = 2649,
  kX86InstIdVextracti128_NameIndex = 2662,
  kX86InstIdVextractps_NameIndex = 2675,
  kX86InstIdVfmadd132pd_NameIndex = 2686,
  kX86InstIdVfmadd132ps_NameIndex = 2698,
  kX86InstIdVfmadd132sd_NameIndex = 2710,
  kX86InstIdVfmadd132ss_NameIndex = 2722,
  kX86InstIdVfmadd213pd_NameIndex = 2734,
  kX86InstIdVfmadd213ps_NameIndex = 2746,
  kX86InstIdVfmadd213sd_NameIndex = 2758,
  kX86InstIdVfmadd213ss_NameIndex = 2770,
  kX86InstIdVfmadd231pd_NameIndex = 2782,
  kX86InstIdVfmadd231ps_NameIndex = 2794,
  kX86InstIdVfmadd231sd_NameIndex = 2806,
  kX86InstIdVfmadd231ss_NameIndex = 2818,
  kX86InstIdVfmaddpd_NameIndex = 2830,
  kX86InstIdVfmaddps_NameIndex = 2839,
  kX86InstIdVfmaddsd_NameIndex = 2848,
  kX86InstIdVfmaddss_NameIndex = 2857,
  kX86InstIdVfmaddsub132pd_NameIndex = 2866,
  kX86InstIdVfmaddsub132ps_NameIndex = 2881,
  kX86InstIdVfmaddsub213pd_NameIndex = 2896,
  kX86InstIdVfmaddsub213ps_NameIndex = 2911,
  kX86InstIdVfmaddsub231pd_NameIndex = 2926,
  kX86InstIdVfmaddsub231ps_NameIndex = 2941,
  kX86InstIdVfmaddsubpd_NameIndex = 2956,
  kX86InstIdVfmaddsubps_NameIndex = 2968,
  kX86InstIdVfmsub132pd_NameIndex = 2980,
  kX86InstIdVfmsub132ps_NameIndex = 2992,
  kX86InstIdVfmsub132sd_NameIndex = 3004,
  kX86InstIdVfmsub132ss_NameIndex = 3016,
  kX86InstIdVfmsub213pd_NameIndex = 3028,
  kX86InstIdVfmsub213ps_NameIndex = 3040,
  kX86InstIdVfmsub213sd_NameIndex = 3052,
  kX86InstIdVfmsub213ss_NameIndex = 3064,
  kX86InstIdVfmsub231pd_NameIndex = 3076,
  kX86InstIdVfmsub231ps_NameIndex = 3088,
  kX86InstIdVfmsub231sd_NameIndex = 3100,
  kX86InstIdVfmsub231ss_NameIndex = 3112,
  kX86InstIdVfmsubadd132pd_NameIndex = 3124,
  kX86InstIdVfmsubadd132ps_NameIndex = 3139,
  kX86InstIdVfmsubadd213pd_NameIndex = 3154,
  kX86InstIdVfmsubadd213ps_NameIndex = 3169,
  kX86InstIdVfmsubadd231pd_NameIndex = 3184,
  kX86InstIdVfmsubadd231ps_NameIndex = 3199,
  kX86InstIdVfmsubaddpd_NameIndex = 3214,
  kX86InstIdVfmsubaddps_NameIndex = 3226,
  kX86InstIdVfmsubpd_NameIndex = 3238,
  kX86InstIdVfmsubps_NameIndex = 3247,
  kX86InstIdVfmsubsd_NameIndex = 3256,
  kX86InstIdVfmsubss_NameIndex = 3265,
  kX86InstIdVfnmadd132pd_NameIndex = 3274,
  kX86InstIdVfnmadd132ps_NameIndex = 3287,
  kX86InstIdVfnmadd132sd_NameIndex = 3300,
  kX86InstIdVfnmadd132ss_NameIndex = 3313,
  kX86InstIdVfnmadd213pd_NameIndex = 3326,
  kX86InstIdVfnmadd213ps_NameIndex = 3339,
  kX86InstIdVfnmadd213sd_NameIndex = 3352,
  kX86InstIdVfnmadd213ss_NameIndex = 3365,
  kX86InstIdVfnmadd231pd_NameIndex = 3378,
  kX86InstIdVfnmadd231ps_NameIndex = 3391,
  kX86InstIdVfnmadd231sd_NameIndex = 3404,
  kX86InstIdVfnmadd231ss_NameIndex = 3417,
  kX86InstIdVfnmaddpd_NameIndex = 3430,
  kX86InstIdVfnmaddps_NameIndex = 3440,
  kX86InstIdVfnmaddsd_NameIndex = 3450,
  kX86InstIdVfnmaddss_NameIndex = 3460,
  kX86InstIdVfnmsub132pd_NameIndex = 3470,
  kX86InstIdVfnmsub132ps_NameIndex = 3483,
  kX86InstIdVfnmsub132sd_NameIndex = 3496,
  kX86InstIdVfnmsub132ss_NameIndex = 3509,
  kX86InstIdVfnmsub213pd_NameIndex = 3522,
  kX86InstIdVfnmsub213ps_NameIndex = 3535,
  kX86InstIdVfnmsub213sd_NameIndex = 3548,
  kX86InstIdVfnmsub213ss_NameIndex = 3561,
  kX86InstIdVfnmsub231pd_NameIndex = 3574,
  kX86InstIdVfnmsub231ps_NameIndex = 3587,
  kX86InstIdVfnmsub231sd_NameIndex = 3600,
  kX86InstIdVfnmsub231ss_NameIndex = 3613,
  kX86InstIdVfnmsubpd_NameIndex = 3626,
  kX86InstIdVfnmsubps_NameIndex = 3636,
  kX86InstIdVfnmsubsd_NameIndex = 3646,
  kX86InstIdVfnmsubss_NameIndex = 3656,
  kX86InstIdVfrczpd_NameIndex = 3666,
  kX86InstIdVfrczps_NameIndex = 3674,
  kX86InstIdVfrczsd_NameIndex = 3682,
  kX86InstIdVfrczss_NameIndex = 3690,
  kX86InstIdVgatherdpd_NameIndex = 3698,
  kX86InstIdVgatherdps_NameIndex = 3709,
  kX86InstIdVgatherqpd_NameIndex = 3720,
  kX86InstIdVgatherqps_NameIndex = 3731,
  kX86InstIdVhaddpd_NameIndex = 3742,
  kX86InstIdVhaddps_NameIndex = 3750,
  kX86InstIdVhsubpd_NameIndex = 3758,
  kX86InstIdVhsubps_NameIndex = 3766,
  kX86InstIdVinsertf128_NameIndex = 3774,
  kX86InstIdVinserti128_NameIndex = 3786,
  kX86InstIdVinsertps_NameIndex = 3798,
  kX86InstIdVlddqu_NameIndex = 3808,
  kX86InstIdVldmxcsr_NameIndex = 3815,
  kX86InstIdVmaskmovdqu_NameIndex = 3824,
  kX86InstIdVmaskmovpd_NameIndex = 3836,
  kX86InstIdVmaskmovps_NameIndex = 3847,
  kX86InstIdVmaxpd_NameIndex = 3858,
  kX86InstIdVmaxps_NameIndex = 3865,
  kX86InstIdVmaxsd_NameIndex = 3872,
  kX86InstIdVmaxss_NameIndex = 3879,
  kX86InstIdVminpd_NameIndex = 3886,
  kX86InstIdVminps_NameIndex = 3893,
  kX86InstIdVminsd_NameIndex = 3900,
  kX86InstIdVminss_NameIndex = 3907,
  kX86InstIdVmovapd_NameIndex = 3914,
  kX86InstIdVmovaps_NameIndex = 3922,
  kX86InstIdVmovd_NameIndex = 3930,
  kX86InstIdVmovddup_NameIndex = 3936,
  kX86InstIdVmovdqa_NameIndex = 3945,
  kX86InstIdVmovdqu_NameIndex = 3953,
  kX86InstIdVmovhlps_NameIndex = 3961,
  kX86InstIdVmovhpd_NameIndex = 3970,
  kX86InstIdVmovhps_NameIndex = 3978,
  kX86InstIdVmovlhps_NameIndex = 3986,
  kX86InstIdVmovlpd_NameIndex = 3995,
  kX86InstIdVmovlps_NameIndex = 4003,
  kX86InstIdVmovmskpd_NameIndex = 4011,
  kX86InstIdVmovmskps_NameIndex = 4021,
  kX86InstIdVmovntdq_NameIndex = 4031,
  kX86InstIdVmovntdqa_NameIndex = 4040,
  kX86InstIdVmovntpd_NameIndex = 4050,
  kX86InstIdVmovntps_NameIndex = 4059,
  kX86InstIdVmovq_NameIndex = 4068,
  kX86InstIdVmovsd_NameIndex = 4074,
  kX86InstIdVmovshdup_NameIndex = 4081,
  kX86InstIdVmovsldup_NameIndex = 4091,
  kX86InstIdVmovss_NameIndex = 4101,
  kX86InstIdVmovupd_NameIndex = 4108,
  kX86InstIdVmovups_NameIndex = 4116,
  kX86InstIdVmpsadbw_NameIndex = 4124,
  kX86InstIdVmulpd_NameIndex = 4133,
  kX86InstIdVmulps_NameIndex = 4140,
  kX86InstIdVmulsd_NameIndex = 4147,
  kX86InstIdVmulss_NameIndex = 4154,
  kX86InstIdVorpd_NameIndex = 4161,
  kX86InstIdVorps_NameIndex = 4167,
  kX86InstIdVpabsb_NameIndex = 4173,
  kX86InstIdVpabsd_NameIndex = 4180,
  kX86InstIdVpabsw_NameIndex = 4187,
  kX86InstIdVpackssdw_NameIndex = 4194,
  kX86InstIdVpacksswb_NameIndex = 4204,
  kX86InstIdVpackusdw_NameIndex = 4214,
  kX86InstIdVpackuswb_NameIndex = 4224,
  kX86InstIdVpaddb_NameIndex = 4234,
  kX86InstIdVpaddd_NameIndex = 4241,
  kX86InstIdVpaddq_NameIndex = 4248,
  kX86InstIdVpaddsb_NameIndex = 4255,
  kX86InstIdVpaddsw_NameIndex = 4263,
  kX86InstIdVpaddusb_NameIndex = 4271,
  kX86InstIdVpaddusw_NameIndex = 4280,
  kX86InstIdVpaddw_NameIndex = 4289,
  kX86InstIdVpalignr_NameIndex = 4296,
  kX86InstIdVpand_NameIndex = 4305,
  kX86InstIdVpandn_NameIndex = 4311,
  kX86InstIdVpavgb_NameIndex = 4318,
  kX86InstIdVpavgw_NameIndex = 4325,
  kX86InstIdVpblendd_NameIndex = 4332,
  kX86InstIdVpblendvb_NameIndex = 4341,
  kX86InstIdVpblendw_NameIndex = 4351,
  kX86InstIdVpbroadcastb_NameIndex = 4360,
  kX86InstIdVpbroadcastd_NameIndex = 4373,
  kX86InstIdVpbroadcastq_NameIndex = 4386,
  kX86InstIdVpbroadcastw_NameIndex = 4399,
  kX86InstIdVpclmulqdq_NameIndex = 4412,
  kX86InstIdVpcmov_NameIndex = 4423,
  kX86InstIdVpcmpeqb_NameIndex = 4430,
  kX86InstIdVpcmpeqd_NameIndex = 4439,
  kX86InstIdVpcmpeqq_NameIndex = 4448,
  kX86InstIdVpcmpeqw_NameIndex = 4457,
  kX86InstIdVpcmpestri_NameIndex = 4466,
  kX86InstIdVpcmpestrm_NameIndex = 4477,
  kX86InstIdVpcmpgtb_NameIndex = 4488,
  kX86InstIdVpcmpgtd_NameIndex = 4497,
  kX86InstIdVpcmpgtq_NameIndex = 4506,
  kX86InstIdVpcmpgtw_NameIndex = 4515,
  kX86InstIdVpcmpistri_NameIndex = 4524,
  kX86InstIdVpcmpistrm_NameIndex = 4535,
  kX86InstIdVpcomb_NameIndex = 4546,
  kX86InstIdVpcomd_NameIndex = 4553,
  kX86InstIdVpcomq_NameIndex = 4560,
  kX86InstIdVpcomub_NameIndex = 4567,
  kX86InstIdVpcomud_NameIndex = 4575,
  kX86InstIdVpcomuq_NameIndex = 4583,
  kX86InstIdVpcomuw_NameIndex = 4591,
  kX86InstIdVpcomw_NameIndex = 4599,
  kX86InstIdVperm2f128_NameIndex = 4606,
  kX86InstIdVperm2i128_NameIndex = 4617,
  kX86InstIdVpermd_NameIndex = 4628,
  kX86InstIdVpermil2pd_NameIndex = 4635,
  kX86InstIdVpermil2ps_NameIndex = 4646,
  kX86InstIdVpermilpd_NameIndex = 4657,
  kX86InstIdVpermilps_NameIndex = 4667,
  kX86InstIdVpermpd_NameIndex = 4677,
  kX86InstIdVpermps_NameIndex = 4685,
  kX86InstIdVpermq_NameIndex = 4693,
  kX86InstIdVpextrb_NameIndex = 4700,
  kX86InstIdVpextrd_NameIndex = 4708,
  kX86InstIdVpextrq_NameIndex = 4716,
  kX86InstIdVpextrw_NameIndex = 4724,
  kX86InstIdVpgatherdd_NameIndex = 4732,
  kX86InstIdVpgatherdq_NameIndex = 4743,
  kX86InstIdVpgatherqd_NameIndex = 4754,
  kX86InstIdVpgatherqq_NameIndex = 4765,
  kX86InstIdVphaddbd_NameIndex = 4776,
  kX86InstIdVphaddbq_NameIndex = 4785,
  kX86InstIdVphaddbw_NameIndex = 4794,
  kX86InstIdVphaddd_NameIndex = 4803,
  kX86InstIdVphadddq_NameIndex = 4811,
  kX86InstIdVphaddsw_NameIndex = 4820,
  kX86InstIdVphaddubd_NameIndex = 4829,
  kX86InstIdVphaddubq_NameIndex = 4839,
  kX86InstIdVphaddubw_NameIndex = 4849,
  kX86InstIdVphaddudq_NameIndex = 4859,
  kX86InstIdVphadduwd_NameIndex = 4869,
  kX86InstIdVphadduwq_NameIndex = 4879,
  kX86InstIdVphaddw_NameIndex = 4889,
  kX86InstIdVphaddwd_NameIndex = 4897,
  kX86InstIdVphaddwq_NameIndex = 4906,
  kX86InstIdVphminposuw_NameIndex = 4915,
  kX86InstIdVphsubbw_NameIndex = 4927,
  kX86InstIdVphsubd_NameIndex = 4936,
  kX86InstIdVphsubdq_NameIndex = 4944,
  kX86InstIdVphsubsw_NameIndex = 4953,
  kX86InstIdVphsubw_NameIndex = 4962,
  kX86InstIdVphsubwd_NameIndex = 4970,
  kX86InstIdVpinsrb_NameIndex = 4979,
  kX86InstIdVpinsrd_NameIndex = 4987,
  kX86InstIdVpinsrq_NameIndex = 4995,
  kX86InstIdVpinsrw_NameIndex = 5003,
  kX86InstIdVpmacsdd_NameIndex = 5011,
  kX86InstIdVpmacsdqh_NameIndex = 5020,
  kX86InstIdVpmacsdql_NameIndex = 5030,
  kX86InstIdVpmacssdd_NameIndex = 5040,
  kX86InstIdVpmacssdqh_NameIndex = 5050,
  kX86InstIdVpmacssdql_NameIndex = 5061,
  kX86InstIdVpmacsswd_NameIndex = 5072,
  kX86InstIdVpmacssww_NameIndex = 5082,
  kX86InstIdVpmacswd_NameIndex = 5092,
  kX86InstIdVpmacsww_NameIndex = 5101,
  kX86InstIdVpmadcsswd_NameIndex = 5110,
  kX86InstIdVpmadcswd_NameIndex = 5121,
  kX86InstIdVpmaddubsw_NameIndex = 5131,
  kX86InstIdVpmaddwd_NameIndex = 5142,
  kX86InstIdVpmaskmovd_NameIndex = 5151,
  kX86InstIdVpmaskmovq_NameIndex = 5162,
  kX86InstIdVpmaxsb_NameIndex = 5173,
  kX86InstIdVpmaxsd_NameIndex = 5181,
  kX86InstIdVpmaxsw_NameIndex = 5189,
  kX86InstIdVpmaxub_NameIndex = 5197,
  kX86InstIdVpmaxud_NameIndex = 5205,
  kX86InstIdVpmaxuw_NameIndex = 5213,
  kX86InstIdVpminsb_NameIndex = 5221,
  kX86InstIdVpminsd_NameIndex = 5229,
  kX86InstIdVpminsw_NameIndex = 5237,
  kX86InstIdVpminub_NameIndex = 5245,
  kX86InstIdVpminud_NameIndex = 5253,
  kX86InstIdVpminuw_NameIndex = 5261,
  kX86InstIdVpmovmskb_NameIndex = 5269,
  kX86InstIdVpmovsxbd_NameIndex = 5279,
  kX86InstIdVpmovsxbq_NameIndex = 5289,
  kX86InstIdVpmovsxbw_NameIndex = 5299,
  kX86InstIdVpmovsxdq_NameIndex = 5309,
  kX86InstIdVpmovsxwd_NameIndex = 5319,
  kX86InstIdVpmovsxwq_NameIndex = 5329,
  kX86InstIdVpmovzxbd_NameIndex = 5339,
  kX86InstIdVpmovzxbq_NameIndex = 5349,
  kX86InstIdVpmovzxbw_NameIndex = 5359,
  kX86InstIdVpmovzxdq_NameIndex = 5369,
  kX86InstIdVpmovzxwd_NameIndex = 5379,
  kX86InstIdVpmovzxwq_NameIndex = 5389,
  kX86InstIdVpmuldq_NameIndex = 5399,
  kX86InstIdVpmulhrsw_NameIndex = 5407,
  kX86InstIdVpmulhuw_NameIndex = 5417,
  kX86InstIdVpmulhw_NameIndex = 5426,
  kX86InstIdVpmulld_NameIndex = 5434,
  kX86InstIdVpmullw_NameIndex = 5442,
  kX86InstIdVpmuludq_NameIndex = 5450,
  kX86InstIdVpor_NameIndex = 5459,
  kX86InstIdVpperm_NameIndex = 5464,
  kX86InstIdVprotb_NameIndex = 5471,
  kX86InstIdVprotd_NameIndex = 5478,
  kX86InstIdVprotq_NameIndex = 5485,
  kX86InstIdVprotw_NameIndex = 5492,
  kX86InstIdVpsadbw_NameIndex = 5499,
  kX86InstIdVpshab_NameIndex = 5507,
  kX86InstIdVpshad_NameIndex = 5514,
  kX86InstIdVpshaq_NameIndex = 5521,
  kX86InstIdVpshaw_NameIndex = 5528,
  kX86InstIdVpshlb_NameIndex = 5535,
  kX86InstIdVpshld_NameIndex = 5542,
  kX86InstIdVpshlq_NameIndex = 5549,
  kX86InstIdVpshlw_NameIndex = 5556,
  kX86InstIdVpshufb_NameIndex = 5563,
  kX86InstIdVpshufd_NameIndex = 5571,
  kX86InstIdVpshufhw_NameIndex = 5579,
  kX86InstIdVpshuflw_NameIndex = 5588,
  kX86InstIdVpsignb_NameIndex = 5597,
  kX86InstIdVpsignd_NameIndex = 5605,
  kX86InstIdVpsignw_NameIndex = 5613,
  kX86InstIdVpslld_NameIndex = 5621,
  kX86InstIdVpslldq_NameIndex = 5628,
  kX86InstIdVpsllq_NameIndex = 5636,
  kX86InstIdVpsllvd_NameIndex = 5643,
  kX86InstIdVpsllvq_NameIndex = 5651,
  kX86InstIdVpsllw_NameIndex = 5659,
  kX86InstIdVpsrad_NameIndex = 5666,
  kX86InstIdVpsravd_NameIndex = 5673,
  kX86InstIdVpsraw_NameIndex = 5681,
  kX86InstIdVpsrld_NameIndex = 5688,
  kX86InstIdVpsrldq_NameIndex = 5695,
  kX86InstIdVpsrlq_NameIndex = 5703,
  kX86InstIdVpsrlvd_NameIndex = 5710,
  kX86InstIdVpsrlvq_NameIndex = 5718,
  kX86InstIdVpsrlw_NameIndex = 5726,
  kX86InstIdVpsubb_NameIndex = 5733,
  kX86InstIdVpsubd_NameIndex = 5740,
  kX86InstIdVpsubq_NameIndex = 5747,
  kX86InstIdVpsubsb_NameIndex = 5754,
  kX86InstIdVpsubsw_NameIndex = 5762,
  kX86InstIdVpsubusb_NameIndex = 5770,
  kX86InstIdVpsubusw_NameIndex = 5779,
  kX86InstIdVpsubw_NameIndex = 5788,
  kX86InstIdVptest_NameIndex = 5795,
  kX86InstIdVpunpckhbw_NameIndex = 5802,
  kX86InstIdVpunpckhdq_NameIndex = 5813,
  kX86InstIdVpunpckhqdq_NameIndex = 5824,
  kX86InstIdVpunpckhwd_NameIndex = 5836,
  kX86InstIdVpunpcklbw_NameIndex = 5847,
  kX86InstIdVpunpckldq_NameIndex = 5858,
  kX86InstIdVpunpcklqdq_NameIndex = 5869,
  kX86InstIdVpunpcklwd_NameIndex = 5881,
  kX86InstIdVpxor_NameIndex = 5892,
  kX86InstIdVrcpps_NameIndex = 5898,
  kX86InstIdVrcpss_NameIndex = 5905,
  kX86InstIdVroundpd_NameIndex = 5912,
  kX86InstIdVroundps_NameIndex = 5921,
  kX86InstIdVroundsd_NameIndex = 5930,
  kX86InstIdVroundss_NameIndex = 5939,
  kX86InstIdVrsqrtps_NameIndex = 5948,
  kX86InstIdVrsqrtss_NameIndex = 5957,
  kX86InstIdVshufpd_NameIndex = 5966,
  kX86InstIdVshufps_NameIndex = 5974,
  kX86InstIdVsqrtpd_NameIndex = 5982,
  kX86InstIdVsqrtps_NameIndex = 5990,
  kX86InstIdVsqrtsd_NameIndex = 5998,
  kX86InstIdVsqrtss_NameIndex = 6006,
  kX86InstIdVstmxcsr_NameIndex = 6014,
  kX86InstIdVsubpd_NameIndex = 6023,
  kX86InstIdVsubps_NameIndex = 6030,
  kX86InstIdVsubsd_NameIndex = 6037,
  kX86InstIdVsubss_NameIndex = 6044,
  kX86InstIdVtestpd_NameIndex = 6051,
  kX86InstIdVtestps_NameIndex = 6059,
  kX86InstIdVucomisd_NameIndex = 6067,
  kX86InstIdVucomiss_NameIndex = 6076,
  kX86InstIdVunpckhpd_NameIndex = 6085,
  kX86InstIdVunpckhps_NameIndex = 6095,
  kX86InstIdVunpcklpd_NameIndex = 6105,
  kX86InstIdVunpcklps_NameIndex = 6115,
  kX86InstIdVxorpd_NameIndex = 6125,
  kX86InstIdVxorps_NameIndex = 6132,
  kX86InstIdVzeroall_NameIndex = 6139,
  kX86InstIdVzeroupper_NameIndex = 6148,
  kX86InstIdWrfsbase_NameIndex = 6159,
  kX86InstIdWrgsbase_NameIndex = 6168,
  kX86InstIdXadd_NameIndex = 6177,
  kX86InstIdXchg_NameIndex = 270,
  kX86InstIdXgetbv_NameIndex = 6182,
  kX86InstIdXor_NameIndex = 5894,
  kX86InstIdXorpd_NameIndex = 6126,
  kX86InstIdXorps_NameIndex = 6133,
  kX86InstIdXrstor_NameIndex = 947,
  kX86InstIdXrstor64_NameIndex = 6189,
  kX86InstIdXsave_NameIndex = 955,
  kX86InstIdXsave64_NameIndex = 6198,
  kX86InstIdXsaveopt_NameIndex = 6206,
  kX86InstIdXsaveopt64_NameIndex = 6215,
  kX86InstIdXsetbv_NameIndex = 6226
};
#endif // !ASMJIT_DISABLE_TEXT

// Automatically generated, do not edit.
const X86InstExtendedInfo _x86InstExtendedInfo[] = {
  { Enc(None)         , 0 , 0 , 0x00, 0x00, 0, { U                 , U                 , U                 , U                 , U                  }, F(None)                            , U                    },
  { Enc(X86Arith)     , 0 , 0 , 0x20, 0x3F, 0, { O(GqdwbMem)       , O(GqdwbMem)|O(Imm), U                 , U                 , U                  }, F(Lock)                            , U                    },
  { Enc(X86Arith)     , 0 , 0 , 0x00, 0x3F, 0, { O(GqdwbMem)       , O(GqdwbMem)|O(Imm), U                 , U                 , U                  }, F(Lock)                            , U                    },
  { Enc(ExtRm)        , 0 , 0 , 0x00, 0x00, 0, { O(Xmm)            , O(XmmMem)         , U                 , U                 , U                  }, F(None)                            , U                    },
  { Enc(ExtRmi)       , 0 , 0 , 0x00, 0x00, 0, { O(Xmm)            , O(XmmMem)         , O(Imm)            , U                 , U                  }, F(None)                            , U                    },
  { Enc(AvxRvm)       , 0 , 0 , 0x00, 0x3F, 0, { O(Gqd)            , O(Gqd)            , O(GqdMem)         , U                 , U                  }, F(None)                            , U                    },
  { Enc(AvxRmv)       , 0 , 0 , 0x00, 0x3F, 0, { O(Gqd)            , O(GqdMem)         , O(Gqd)            , U                 , U                  }, F(None)                            , U                    },
  { Enc(ExtRm)        , 0 , 0 , 0x00, 0x00, 0, { O(Xmm)            , O(XmmMem)         , U                 , U                 , U                  }, F(None)|F(Special)                 , U                    },
  { Enc(AvxVm)        , 0 , 0 , 0x00, 0x3F, 0, { O(Gqd)            , O(GqdMem)         , U                 , U                 , U                  }, F(None)                            , U                    },
  { Enc(X86RegRm)     , 0 , 0 , 0x00, 0x3F, 0, { O(Gqdw)           , O(GqdwMem)        , U                 , U                 , U                  }, F(None)                            , U                    },
  { Enc(X86BSwap)     , 0 , 0 , 0x00, 0x00, 0, { O(Gqd)            , U                 , U                 , U                 , U                  }, F(None)                            , U                    },
  { Enc(X86BTest)     , 0 , 0 , 0x00, 0x3B, 0, { O(GqdwMem)        , O(Gqdw)|O(Imm)    , U                 , U                 , U                  }, F(Test)                            , O_000F00(BA,4,_,_,_) },
  { Enc(X86BTest)     , 0 , 0 , 0x00, 0x3B, 0, { O(GqdwMem)        , O(Gqdw)|O(Imm)    , U                 , U                 , U                  }, F(Lock)                            , O_000F00(BA,7,_,_,_) },
  { Enc(X86BTest)     , 0 , 0 , 0x00, 0x3B, 0, { O(GqdwMem)        , O(Gqdw)|O(Imm)    , U                 , U                 , U                  }, F(Lock)                            , O_000F00(BA,6,_,_,_) },
  { Enc(X86BTest)     , 0 , 0 , 0x00, 0x3B, 0, { O(GqdwMem)        , O(Gqdw)|O(Imm)    , U                 , U                 , U                  }, F(Lock)                            , O_000F00(BA,5,_,_,_) },
  { Enc(X86Call)      , 0 , 0 , 0x00, 0x00, 0, { O(GqdMem)|O(LImm) , U                 , U                 , U                 , U                  }, F(Flow)                            , O_000000(E8,U,_,_,_) },
  { Enc(X86Op)        , 0 , 0 , 0x00, 0x00, 0, { U                 , U                 , U                 , U                 , U                  }, F(None)|F(Special)                 , U                    },
  { Enc(X86Op)        , 0 , 0 , 0x00, 0x20, 0, { U                 , U                 , U                 , U                 , U                  }, F(None)                            , U                    },
  { Enc(X86Op)        , 0 , 0 , 0x00, 0x40, 0, { U                 , U                 , U                 , U                 , U                  }, F(None)                            , U                    },
  { Enc(X86M)         , 0 , 0 , 0x00, 0x00, 0, { O(Mem)            , U                 , U                 , U                 , U                  }, F(None)                            , U                    },
  { Enc(X86Op)        , 0 , 0 , 0x20, 0x20, 0, { U                 , U                 , U                 , U                 , U                  }, F(None)                            , U                    },
  { Enc(X86RegRm)     , 0 , 0 , 0x24, 0x00, 0, { O(Gqdw)           , O(GqdwMem)        , U                 , U                 , U                  }, F(None)                            , U                    },
  { Enc(X86RegRm)     , 0 , 0 , 0x20, 0x00, 0, { O(Gqdw)           , O(GqdwMem)        , U                 , U                 , U                  }, F(None)                            , U                    },
  { Enc(X86RegRm)     , 0 , 0 , 0x04, 0x00, 0, { O(Gqdw)           , O(GqdwMem)        , U                 , U                 , U                  }, F(None)                            , U                    },
  { Enc(X86RegRm)     , 0 , 0 , 0x07, 0x00, 0, { O(Gqdw)           , O(GqdwMem)        , U                 , U                 , U                  }, F(None)                            , U                    },
  { Enc(X86RegRm)     , 0 , 0 , 0x03, 0x00, 0, { O(Gqdw)           , O(GqdwMem)        , U                 , U                 , U                  }, F(None)                            , U                    },
  { Enc(X86RegRm)     , 0 , 0 , 0x01, 0x00, 0, { O(Gqdw)           , O(GqdwMem)        , U                 , U                 , U                  }, F(None)                            , U                    },
  { Enc(X86RegRm)     , 0 , 0 , 0x10, 0x00, 0, { O(Gqdw)           , O(GqdwMem)        , U                 , U                 , U                  }, F(None)                            , U                    },
  { Enc(X86RegRm)     , 0 , 0 , 0x02, 0x00, 0, { O(Gqdw)           , O(GqdwMem)        , U                 , U                 , U                  }, F(None)                            , U                    },
  { Enc(X86Arith)     , 0 , 0 , 0x00, 0x3F, 0, { O(GqdwbMem)       , O(GqdwbMem)|O(Imm), U                 , U                 , U                  }, F(Test)                            , U                    },
  { Enc(X86Op)        , 0 , 0 , 0x40, 0x3F, 0, { U                 , U                 , U                 , U                 , U                  }, F(None)|F(Special)                 , U                    },
  { Enc(X86Op_66H)    , 0 , 0 , 0x40, 0x3F, 0, { U                 , U                 , U                 , U                 , U                  }, F(None)|F(Special)                 , U                    },
  { Enc(X86RmReg)     , 0 , 0 , 0x00, 0x3F, 0, { U                 , U                 , U                 , U                 , U                  }, F(Lock)|F(Special)                 , U                    },
  { Enc(X86M)         , 0 , 0 , 0x00, 0x04, 0, { O(Mem)            , U                 , U                 , U                 , U                  }, F(None)|F(Special)                 , U                    },
  { Enc(ExtRm)        , 0 , 0 , 0x00, 0x3F, 0, { O(Xmm)            , O(XmmMem)         , U                 , U                 , U                  }, F(Test)                            , U                    },
  { Enc(ExtCrc)       , 0 , 0 , 0x00, 0x00, 0, { O(Gqd)            , O(GqdwbMem)       , U                 , U                 , U                  }, F(None)                            , U                    },
  { Enc(ExtRm)        , 0 , 16, 0x00, 0x00, 0, { O(Xmm)            , O(XmmMem)         , U                 , U                 , U                  }, F(Move)                            , U                    },
  { Enc(ExtRm)        , 0 , 8 , 0x00, 0x00, 0, { O(Mm)             , O(XmmMem)         , U                 , U                 , U                  }, F(Move)                            , U                    },
  { Enc(ExtRm)        , 0 , 16, 0x00, 0x00, 0, { O(Xmm)            , O(MmMem)          , U                 , U                 , U                  }, F(Move)                            , U                    },
  { Enc(ExtRm)        , 0 , 8 , 0x00, 0x00, 0, { O(Xmm)            , O(MmMem)          , U                 , U                 , U                  }, F(Move)                            , U                    },
  { Enc(ExtRm_Q)      , 0 , 8 , 0x00, 0x00, 0, { O(Gqd)            , O(XmmMem)         , U                 , U                 , U                  }, F(Move)                            , U                    },
  { Enc(ExtRm)        , 0 , 4 , 0x00, 0x00, 0, { O(Xmm)            , O(XmmMem)         , U                 , U                 , U                  }, F(Move)                            , U                    },
  { Enc(ExtRm_Q)      , 0 , 8 , 0x00, 0x00, 0, { O(Xmm)            , O(GqdMem)         , U                 , U                 , U                  }, F(Move)                            , U                    },
  { Enc(ExtRm_Q)      , 0 , 4 , 0x00, 0x00, 0, { O(Xmm)            , O(GqdMem)         , U                 , U                 , U                  }, F(Move)                            , U                    },
  { Enc(ExtRm)        , 0 , 8 , 0x00, 0x00, 0, { O(Xmm)            , O(XmmMem)         , U                 , U                 , U                  }, F(Move)                            , U                    },
  { Enc(X86Op)        , 0 , 0 , 0x28, 0x3F, 0, { U                 , U                 , U                 , U                 , U                  }, F(None)|F(Special)                 , U                    },
  { Enc(X86IncDec)    , 0 , 0 , 0x00, 0x1F, 0, { O(GqdwbMem)       , U                 , U                 , U                 , U                  }, F(Lock)                            , O_000000(48,U,_,_,_) },
  { Enc(X86Rm_B)      , 0 , 0 , 0x00, 0x3F, 0, { U                 , U                 , U                 , U                 , U                  }, F(None)|F(Special)                 , U                    },
  { Enc(X86Op)        , 0 , 0 , 0x00, 0x00, 0, { U                 , U                 , U                 , U                 , U                  }, F(None)                            , U                    },
  { Enc(X86Enter)     , 0 , 0 , 0x00, 0x00, 0, { U                 , U                 , U                 , U                 , U                  }, F(None)|F(Special)                 , U                    },
  { Enc(ExtExtract)   , 0 , 8 , 0x00, 0x00, 0, { O(GqdMem)         , O(Xmm)            , U                 , U                 , U                  }, F(Move)                            , U                    },
  { Enc(ExtExtrq)     , 0 , 0 , 0x00, 0x00, 0, { O(Xmm)            , O(Xmm)|O(Imm)     , O(None)|O(Imm)    , U                 , U                  }, F(None)                            , O_660F00(78,0,_,_,_) },
  { Enc(FpuOp)        , 0 , 0 , 0x00, 0x00, 0, { U                 , U                 , U                 , U                 , U                  }, F(Fp)                              , U                    },
  { Enc(FpuArith)     , 0 , 0 , 0x00, 0x00, 0, { O(FpMem)          , O(Fp)             , U                 , U                 , U                  }, F(Fp)|F(Mem4_8)                    , U                    },
  { Enc(FpuRDef)      , 0 , 0 , 0x00, 0x00, 0, { O(Fp)             , U                 , U                 , U                 , U                  }, F(Fp)                              , U                    },
  { Enc(X86M)         , 0 , 0 , 0x00, 0x00, 0, { O(Mem)            , U                 , U                 , U                 , U                  }, F(Fp)                              , U                    },
  { Enc(FpuR)         , 0 , 0 , 0x20, 0x00, 0, { O(Fp)             , U                 , U                 , U                 , U                  }, F(Fp)                              , U                    },
  { Enc(FpuR)         , 0 , 0 , 0x24, 0x00, 0, { O(Fp)             , U                 , U                 , U                 , U                  }, F(Fp)                              , U                    },
  { Enc(FpuR)         , 0 , 0 , 0x04, 0x00, 0, { O(Fp)             , U                 , U                 , U                 , U                  }, F(Fp)                              , U                    },
  { Enc(FpuR)         , 0 , 0 , 0x10, 0x00, 0, { O(Fp)             , U                 , U                 , U                 , U                  }, F(Fp)                              , U                    },
  { Enc(FpuCom)       , 0 , 0 , 0x00, 0x00, 0, { O(Fp)|O(Mem)      , O(Fp)             , U                 , U                 , U                  }, F(Fp)                              , U                    },
  { Enc(FpuR)         , 0 , 0 , 0x00, 0x3F, 0, { O(Fp)             , U                 , U                 , U                 , U                  }, F(Fp)                              , U                    },
  { Enc(X86Op)        , 0 , 0 , 0x00, 0x00, 0, { U                 , U                 , U                 , U                 , U                  }, F(Fp)                              , U                    },
  { Enc(FpuR)         , 0 , 0 , 0x00, 0x00, 0, { O(Fp)             , U                 , U                 , U                 , U                  }, F(Fp)                              , U                    },
  { Enc(FpuM)         , 0 , 0 , 0x00, 0x00, 0, { O(Mem)            , U                 , U                 , U                 , U                  }, F(Fp)|F(Mem2_4)                    , U                    },
  { Enc(FpuM)         , 0 , 0 , 0x00, 0x00, 0, { O(Mem)            , U                 , U                 , U                 , U                  }, F(Fp)|F(Mem2_4_8)                  , O_000000(DF,5,_,_,_) },
  { Enc(FpuM)         , 0 , 0 , 0x00, 0x00, 0, { O(Mem)            , U                 , U                 , U                 , U                  }, F(Fp)|F(Mem2_4_8)                  , O_000000(DF,7,_,_,_) },
  { Enc(FpuM)         , 0 , 0 , 0x00, 0x00, 0, { O(Mem)            , U                 , U                 , U                 , U                  }, F(Fp)|F(Mem2_4_8)                  , O_000000(DD,1,_,_,_) },
  { Enc(FpuFldFst)    , 0 , 0 , 0x00, 0x00, 0, { O(Mem)            , U                 , U                 , U                 , U                  }, F(Fp)|F(Mem4_8_10)                 , O_000000(DB,5,_,_,_) },
  { Enc(FpuStsw)      , 0 , 0 , 0x00, 0x00, 0, { O(Mem)            , U                 , U                 , U                 , U                  }, F(Fp)                              , O_00_X(DFE0,U)       },
  { Enc(FpuFldFst)    , 0 , 0 , 0x00, 0x00, 0, { O(Mem)            , U                 , U                 , U                 , U                  }, F(Fp)|F(Mem4_8)                    , U                    },
  { Enc(FpuFldFst)    , 0 , 0 , 0x00, 0x00, 0, { O(Mem)            , U                 , U                 , U                 , U                  }, F(Fp)|F(Mem4_8_10)                 , O_000000(DB,7,_,_,_) },
  { Enc(FpuStsw)      , 0 , 0 , 0x00, 0x00, 0, { O(Mem)            , U                 , U                 , U                 , U                  }, F(Fp)                              , O_9B_X(DFE0,U)       },
  { Enc(X86Rm_B)      , 0 , 0 , 0x00, 0x3F, 0, { 0                 , 0                 , U                 , U                 , U                  }, F(None)|F(Special)                 , U                    },
  { Enc(X86Imul)      , 0 , 0 , 0x00, 0x3F, 0, { 0                 , 0                 , U                 , U                 , U                  }, F(None)|F(Special)                 , U                    },
  { Enc(X86IncDec)    , 0 , 0 , 0x00, 0x1F, 0, { O(GqdwbMem)       , U                 , U                 , U                 , U                  }, F(Lock)                            , O_000000(40,U,_,_,_) },
  { Enc(ExtInsertq)   , 0 , 0 , 0x00, 0x00, 0, { O(Xmm)            , O(Xmm)            , O(None)|O(Imm)    , O(None)|O(Imm)    , U                  }, F(None)                            , O_F20F00(78,U,_,_,_) },
  { Enc(X86Int)       , 0 , 0 , 0x00, 0x80, 0, { U                 , U                 , U                 , U                 , U                  }, F(None)                            , U                    },
  { Enc(X86Jcc)       , 0 , 0 , 0x24, 0x00, 0, { O(Label)          , U                 , U                 , U                 , U                  }, F(Flow)                            , U                    },
  { Enc(X86Jcc)       , 0 , 0 , 0x20, 0x00, 0, { O(Label)          , U                 , U                 , U                 , U                  }, F(Flow)                            , U                    },
  { Enc(X86Jcc)       , 0 , 0 , 0x04, 0x00, 0, { O(Label)          , U                 , U                 , U                 , U                  }, F(Flow)                            , U                    },
  { Enc(X86Jcc)       , 0 , 0 , 0x07, 0x00, 0, { O(Label)          , U                 , U                 , U                 , U                  }, F(Flow)                            , U                    },
  { Enc(X86Jcc)       , 0 , 0 , 0x03, 0x00, 0, { O(Label)          , U                 , U                 , U                 , U                  }, F(Flow)                            , U                    },
  { Enc(X86Jcc)       , 0 , 0 , 0x01, 0x00, 0, { O(Label)          , U                 , U                 , U                 , U                  }, F(Flow)                            , U                    },
  { Enc(X86Jcc)       , 0 , 0 , 0x10, 0x00, 0, { O(Label)          , U                 , U                 , U                 , U                  }, F(Flow)                            , U                    },
  { Enc(X86Jcc)       , 0 , 0 , 0x02, 0x00, 0, { O(Label)          , U                 , U                 , U                 , U                  }, F(Flow)                            , U                    },
  { Enc(X86Jecxz)     , 0 , 0 , 0x00, 0x00, 0, { O(Gqdw)           , O(Label)          , U                 , U                 , U                  }, F(Flow)|F(Special)                 , U                    },
  { Enc(X86Jmp)       , 0 , 0 , 0x00, 0x00, 0, { O(Label)|O(Imm)   , U                 , U                 , U                 , U                  }, F(Flow)                            , O_000000(E9,U,_,_,_) },
  { Enc(X86Op)        , 0 , 0 , 0x3E, 0x00, 0, { U                 , U                 , U                 , U                 , U                  }, F(None)|F(Special)                 , U                    },
  { Enc(ExtRm)        , 0 , 16, 0x00, 0x00, 0, { O(Xmm)            , O(Mem)            , U                 , U                 , U                  }, F(Move)                            , U                    },
  { Enc(X86Lea)       , 0 , 0 , 0x00, 0x00, 0, { O(Gqd)            , O(Mem)            , U                 , U                 , U                  }, F(Move)                            , U                    },
  { Enc(ExtFence)     , 0 , 0 , 0x00, 0x00, 0, { U                 , U                 , U                 , U                 , U                  }, F(None)                            , U                    },
  { Enc(X86Op)        , 0 , 1 , 0x40, 0x00, 0, { U                 , U                 , U                 , U                 , U                  }, F(Move)|F(Special)                 , U                    },
  { Enc(X86Op)        , 0 , 4 , 0x40, 0x00, 0, { U                 , U                 , U                 , U                 , U                  }, F(Move)|F(Special)                 , U                    },
  { Enc(X86Op)        , 0 , 8 , 0x40, 0x00, 0, { U                 , U                 , U                 , U                 , U                  }, F(Move)|F(Special)                 , U                    },
  { Enc(X86Op_66H)    , 0 , 2 , 0x40, 0x00, 0, { U                 , U                 , U                 , U                 , U                  }, F(Move)|F(Special)                 , U                    },
  { Enc(ExtRm)        , 0 , 0 , 0x00, 0x00, 0, { O(Xmm)            , O(Xmm)            , U                 , U                 , U                  }, F(None)|F(Special)                 , U                    },
  { Enc(ExtRm)        , 0 , 0 , 0x00, 0x00, 0, { O(Mm)             , O(Mm)             , U                 , U                 , U                  }, F(None)|F(Special)                 , U                    },
  { Enc(X86Mov)       , 0 , 0 , 0x00, 0x00, 0, { O(GqdwbMem)       , O(GqdwbMem)|O(Imm), U                 , U                 , U                  }, F(Move)                            , U                    },
  { Enc(X86MovPtr)    , 0 , 0 , 0x00, 0x00, 0, { O(Gqdwb)          , O(Imm)            , U                 , U                 , U                  }, F(Move)|F(Special)                 , O_000000(A2,U,_,_,_) },
  { Enc(ExtMov)       , 0 , 16, 0x00, 0x00, 0, { O(XmmMem)         , O(XmmMem)         , U                 , U                 , U                  }, F(Move)                            , O_660F00(29,U,_,_,_) },
  { Enc(ExtMov)       , 0 , 16, 0x00, 0x00, 0, { O(XmmMem)         , O(XmmMem)         , U                 , U                 , U                  }, F(Move)                            , O_000F00(29,U,_,_,_) },
  { Enc(ExtMovBe)     , 0 , 0 , 0x00, 0x00, 0, { O(GqdwMem)        , O(GqdwMem)        , U                 , U                 , U                  }, F(Move)                            , O_000F38(F1,U,_,_,_) },
  { Enc(ExtMovD)      , 0 , 16, 0x00, 0x00, 0, { O(Gd)|O(MmXmmMem) , O(Gd)|O(MmXmmMem) , U                 , U                 , U                  }, F(Move)                            , O_000F00(7E,U,_,_,_) },
  { Enc(ExtMov)       , 0 , 16, 0x00, 0x00, 0, { O(Xmm)            , O(XmmMem)         , U                 , U                 , U                  }, F(Move)                            , U                    },
  { Enc(ExtMov)       , 0 , 8 , 0x00, 0x00, 0, { O(Mm)             , O(Xmm)            , U                 , U                 , U                  }, F(Move)                            , U                    },
  { Enc(ExtMov)       , 0 , 16, 0x00, 0x00, 0, { O(XmmMem)         , O(XmmMem)         , U                 , U                 , U                  }, F(Move)                            , O_660F00(7F,U,_,_,_) },
  { Enc(ExtMov)       , 0 , 16, 0x00, 0x00, 0, { O(XmmMem)         , O(XmmMem)         , U                 , U                 , U                  }, F(Move)                            , O_F30F00(7F,U,_,_,_) },
  { Enc(ExtMov)       , 0 , 8 , 0x00, 0x00, 0, { O(Xmm)            , O(Xmm)            , U                 , U                 , U                  }, F(Move)                            , U                    },
  { Enc(ExtMov)       , 8 , 8 , 0x00, 0x00, 0, { O(XmmMem)         , O(XmmMem)         , U                 , U                 , U                  }, F(None)                            , O_660F00(17,U,_,_,_) },
  { Enc(ExtMov)       , 8 , 8 , 0x00, 0x00, 0, { O(XmmMem)         , O(XmmMem)         , U                 , U                 , U                  }, F(None)                            , O_000F00(17,U,_,_,_) },
  { Enc(ExtMov)       , 8 , 8 , 0x00, 0x00, 0, { O(Xmm)            , O(Xmm)            , U                 , U                 , U                  }, F(None)                            , U                    },
  { Enc(ExtMov)       , 0 , 8 , 0x00, 0x00, 0, { O(XmmMem)         , O(XmmMem)         , U                 , U                 , U                  }, F(Move)                            , O_660F00(13,U,_,_,_) },
  { Enc(ExtMov)       , 0 , 8 , 0x00, 0x00, 0, { O(XmmMem)         , O(XmmMem)         , U                 , U                 , U                  }, F(Move)                            , O_000F00(13,U,_,_,_) },
  { Enc(ExtMovNoRexW) , 0 , 8 , 0x00, 0x00, 0, { O(Gqd)            , O(Xmm)            , U                 , U                 , U                  }, F(Move)                            , U                    },
  { Enc(ExtMov)       , 0 , 16, 0x00, 0x00, 0, { O(Mem)            , O(Xmm)            , U                 , U                 , U                  }, F(Move)                            , O_660F00(E7,U,_,_,_) },
  { Enc(ExtMov)       , 0 , 16, 0x00, 0x00, 0, { O(Xmm)            , O(Mem)            , U                 , U                 , U                  }, F(Move)                            , U                    },
  { Enc(ExtMov)       , 0 , 8 , 0x00, 0x00, 0, { O(Mem)            , O(Gqd)            , U                 , U                 , U                  }, F(Move)                            , O_000F00(C3,U,_,_,_) },
  { Enc(ExtMov)       , 0 , 16, 0x00, 0x00, 0, { O(Mem)            , O(Xmm)            , U                 , U                 , U                  }, F(Move)                            , O_660F00(2B,U,_,_,_) },
  { Enc(ExtMov)       , 0 , 16, 0x00, 0x00, 0, { O(Mem)            , O(Xmm)            , U                 , U                 , U                  }, F(Move)                            , O_000F00(2B,U,_,_,_) },
  { Enc(ExtMov)       , 0 , 8 , 0x00, 0x00, 0, { O(Mem)            , O(Mm)             , U                 , U                 , U                  }, F(Move)                            , O_000F00(E7,U,_,_,_) },
  { Enc(ExtMov)       , 0 , 8 , 0x00, 0x00, 0, { O(Mem)            , O(Xmm)            , U                 , U                 , U                  }, F(Move)                            , O_F20F00(2B,U,_,_,_) },
  { Enc(ExtMov)       , 0 , 4 , 0x00, 0x00, 0, { O(Mem)            , O(Xmm)            , U                 , U                 , U                  }, F(Move)                            , O_F30F00(2B,U,_,_,_) },
  { Enc(ExtMovQ)      , 0 , 16, 0x00, 0x00, 0, { O(Gq)|O(MmXmmMem) , O(Gq)|O(MmXmmMem) , U                 , U                 , U                  }, F(Move)                            , O_000F00(7E,U,_,W,_) },
  { Enc(ExtRm)        , 0 , 16, 0x00, 0x00, 0, { O(Xmm)            , O(Mm)             , U                 , U                 , U                  }, F(Move)                            , U                    },
  { Enc(X86Op)        , 0 , 0 , 0x00, 0x00, 0, { U                 , U                 , U                 , U                 , U                  }, F(Move)|F(Special)                 , U                    },
  { Enc(X86Op_66H)    , 0 , 0 , 0x00, 0x00, 0, { U                 , U                 , U                 , U                 , U                  }, F(None)|F(Special)                 , U                    },
  { Enc(ExtMov)       , 0 , 8 , 0x00, 0x00, 0, { O(XmmMem)         , O(XmmMem)         , U                 , U                 , U                  }, F(Move)           |F(Z)            , O_F20F00(11,U,_,_,_) },
  { Enc(ExtMov)       , 0 , 4 , 0x00, 0x00, 0, { O(XmmMem)         , O(XmmMem)         , U                 , U                 , U                  }, F(Move)           |F(Z)            , O_F30F00(11,U,_,_,_) },
  { Enc(X86MovSxZx)   , 0 , 0 , 0x00, 0x00, 0, { O(Gqdw)           , O(GwbMem)         , U                 , U                 , U                  }, F(Move)                            , U                    },
  { Enc(X86MovSxd)    , 0 , 0 , 0x00, 0x00, 0, { O(Gq)             , O(GdMem)          , U                 , U                 , U                  }, F(Move)                            , U                    },
  { Enc(ExtMov)       , 0 , 16, 0x00, 0x00, 0, { O(XmmMem)         , O(XmmMem)         , U                 , U                 , U                  }, F(Move)                            , O_660F00(11,U,_,_,_) },
  { Enc(ExtMov)       , 0 , 16, 0x00, 0x00, 0, { O(XmmMem)         , O(XmmMem)         , U                 , U                 , U                  }, F(Move)                            , O_000F00(11,U,_,_,_) },
  { Enc(AvxRvm)       , 0 , 0 , 0x00, 0x00, 0, { O(Gqd)            , O(Gqd)            , O(GqdMem)         , U                 , U                  }, F(None)                            , U                    },
  { Enc(X86Rm_B)      , 0 , 0 , 0x00, 0x3F, 0, { O(GqdwbMem)       , U                 , U                 , U                 , U                  }, F(Lock)                            , U                    },
  { Enc(X86Rm_B)      , 0 , 0 , 0x00, 0x00, 0, { O(GqdwbMem)       , U                 , U                 , U                 , U                  }, F(Lock)                            , U                    },
  { Enc(ExtRm_P)      , 0 , 0 , 0x00, 0x00, 0, { O(MmXmm)          , O(MmXmmMem)       , U                 , U                 , U                  }, F(None)                            , U                    },
  { Enc(ExtRmi_P)     , 0 , 0 , 0x00, 0x00, 0, { O(MmXmm)          , O(MmXmmMem)       , O(Imm)            , U                 , U                  }, F(None)                            , U                    },
  { Enc(ExtExtract)   , 0 , 8 , 0x00, 0x00, 0, { O(Gd)|O(Gb)|O(Mem), O(Xmm)            , U                 , U                 , U                  }, F(Move)                            , U                    },
  { Enc(ExtExtract)   , 0 , 8 , 0x00, 0x00, 0, { O(GdMem)          , O(Xmm)            , U                 , U                 , U                  }, F(Move)                            , U                    },
  { Enc(ExtExtrW)     , 0 , 8 , 0x00, 0x00, 0, { O(GdMem)          , O(MmXmm)          , U                 , U                 , U                  }, F(Move)                            , O_000F3A(15,U,_,_,_) },
  { Enc(3dNow)        , 0 , 0 , 0x00, 0x00, 0, { O(Mm)             , O(MmMem)          , U                 , U                 , U                  }, F(None)                            , U                    },
  { Enc(ExtRmi)       , 0 , 0 , 0x00, 0x00, 0, { O(Xmm)            , O(GdMem)          , O(Imm)            , U                 , U                  }, F(None)                            , U                    },
  { Enc(ExtRmi)       , 0 , 0 , 0x00, 0x00, 0, { O(Xmm)            , O(GqMem)          , O(Imm)            , U                 , U                  }, F(None)                            , U                    },
  { Enc(ExtRmi_P)     , 0 , 0 , 0x00, 0x00, 0, { O(MmXmm)          , O(GdMem)          , O(Imm)            , U                 , U                  }, F(None)                            , U                    },
  { Enc(ExtRm_PQ)     , 0 , 8 , 0x00, 0x00, 0, { O(Gqd)            , O(MmXmm)          , U                 , U                 , U                  }, F(Move)                            , U                    },
  { Enc(X86Pop)       , 0 , 0 , 0x00, 0x00, 0, { 0                 , U                 , U                 , U                 , U                  }, F(None)|F(Special)                 , O_000000(58,U,_,_,_) },
  { Enc(X86Op)        , 0 , 0 , 0x00, 0xFF, 0, { U                 , U                 , U                 , U                 , U                  }, F(None)|F(Special)                 , U                    },
  { Enc(ExtPrefetch)  , 0 , 0 , 0x00, 0x00, 0, { O(Mem)            , O(Imm)            , U                 , U                 , U                  }, F(None)                            , U                    },
  { Enc(ExtRmi)       , 0 , 16, 0x00, 0x00, 0, { O(Xmm)            , O(XmmMem)         , O(Imm)            , U                 , U                  }, F(Move)                            , U                    },
  { Enc(ExtRmi_P)     , 0 , 8 , 0x00, 0x00, 0, { O(Mm)             , O(MmMem)          , O(Imm)            , U                 , U                  }, F(Move)                            , U                    },
  { Enc(ExtRmRi_P)    , 0 , 0 , 0x00, 0x00, 0, { O(MmXmm)          , O(MmXmmMem)|O(Imm), U                 , U                 , U                  }, F(None)                            , O_000F00(72,6,_,_,_) },
  { Enc(ExtRmRi)      , 0 , 0 , 0x00, 0x00, 0, { O(Xmm)            , O(Imm)            , U                 , U                 , U                  }, F(None)                            , O_660F00(73,7,_,_,_) },
  { Enc(ExtRmRi_P)    , 0 , 0 , 0x00, 0x00, 0, { O(MmXmm)          , O(MmXmmMem)|O(Imm), U                 , U                 , U                  }, F(None)                            , O_000F00(73,6,_,_,_) },
  { Enc(ExtRmRi_P)    , 0 , 0 , 0x00, 0x00, 0, { O(MmXmm)          , O(MmXmmMem)|O(Imm), U                 , U                 , U                  }, F(None)                            , O_000F00(71,6,_,_,_) },
  { Enc(ExtRmRi_P)    , 0 , 0 , 0x00, 0x00, 0, { O(MmXmm)          , O(MmXmmMem)|O(Imm), U                 , U                 , U                  }, F(None)                            , O_000F00(72,4,_,_,_) },
  { Enc(ExtRmRi_P)    , 0 , 0 , 0x00, 0x00, 0, { O(MmXmm)          , O(MmXmmMem)|O(Imm), U                 , U                 , U                  }, F(None)                            , O_000F00(71,4,_,_,_) },
  { Enc(ExtRmRi_P)    , 0 , 0 , 0x00, 0x00, 0, { O(MmXmm)          , O(MmXmmMem)|O(Imm), U                 , U                 , U                  }, F(None)                            , O_000F00(72,2,_,_,_) },
  { Enc(ExtRmRi)      , 0 , 0 , 0x00, 0x00, 0, { O(Xmm)            , O(Imm)            , U                 , U                 , U                  }, F(None)                            , O_660F00(73,3,_,_,_) },
  { Enc(ExtRmRi_P)    , 0 , 0 , 0x00, 0x00, 0, { O(MmXmm)          , O(MmXmmMem)|O(Imm), U                 , U                 , U                  }, F(None)                            , O_000F00(73,2,_,_,_) },
  { Enc(ExtRmRi_P)    , 0 , 0 , 0x00, 0x00, 0, { O(MmXmm)          , O(MmXmmMem)|O(Imm), U                 , U                 , U                  }, F(None)                            , O_000F00(71,2,_,_,_) },
  { Enc(X86Push)      , 0 , 0 , 0x00, 0x00, 0, { 0                 , U                 , U                 , U                 , U                  }, F(None)|F(Special)                 , O_000000(50,U,_,_,_) },
  { Enc(X86Op)        , 0 , 0 , 0xFF, 0x00, 0, { U                 , U                 , U                 , U                 , U                  }, F(None)|F(Special)                 , U                    },
  { Enc(X86Rot)       , 0 , 0 , 0x20, 0x21, 0, { O(GqdwbMem)       , O(Gb)|O(Imm)      , U                 , U                 , U                  }, F(None)|F(Special)                 , U                    },
  { Enc(X86Rm)        , 0 , 8 , 0x00, 0x00, 0, { O(Gqd)            , U                 , U                 , U                 , U                  }, F(Move)                            , U                    },
  { Enc(X86Rm)        , 0 , 8 , 0x00, 0x3F, 0, { O(Gqdw)           , U                 , U                 , U                 , U                  }, F(Move)                            , U                    },
  { Enc(X86Rep)       , 0 , 0 , 0x40, 0x00, 0, { O(Mem)            , U                 , U                 , U                 , U                  }, F(None)|F(Special)                 , U                    },
  { Enc(X86Rep)       , 0 , 0 , 0x40, 0x00, 0, { O(Mem)            , O(Mem)            , U                 , U                 , U                  }, F(None)|F(Special)                 , U                    },
  { Enc(X86Rep)       , 0 , 0 , 0x40, 0x3F, 0, { O(Mem)            , O(Mem)            , U                 , U                 , U                  }, F(None)|F(Special)                 , U                    },
  { Enc(X86Ret)       , 0 , 0 , 0x00, 0x00, 0, { U                 , U                 , U                 , U                 , U                  }, F(None)|F(Special)                 , U                    },
  { Enc(X86Rot)       , 0 , 0 , 0x00, 0x21, 0, { O(GqdwbMem)       , O(Gb)|O(Imm)      , U                 , U                 , U                  }, F(None)|F(Special)                 , U                    },
  { Enc(AvxRmi)       , 0 , 0 , 0x00, 0x00, 0, { O(Gqd)            , O(GqdMem)         , O(Imm)            , U                 , U                  }, F(None)                            , U                    },
  { Enc(ExtRmi)       , 0 , 8 , 0x00, 0x00, 0, { O(Xmm)            , O(XmmMem)         , O(Imm)            , U                 , U                  }, F(Move)                            , U                    },
  { Enc(ExtRmi)       , 0 , 4 , 0x00, 0x00, 0, { O(Xmm)            , O(XmmMem)         , O(Imm)            , U                 , U                  }, F(Move)                            , U                    },
  { Enc(X86Op)        , 0 , 0 , 0x00, 0x3E, 0, { U                 , U                 , U                 , U                 , U                  }, F(None)|F(Special)                 , U                    },
  { Enc(X86Rot)       , 0 , 0 , 0x00, 0x3F, 0, { O(GqdwbMem)       , O(Gb)|O(Imm)      , U                 , U                 , U                  }, F(None)|F(Special)                 , U                    },
  { Enc(AvxRmv)       , 0 , 0 , 0x00, 0x00, 0, { O(Gqd)            , O(GqdMem)         , O(Gqd)            , U                 , U                  }, F(None)                            , U                    },
  { Enc(X86Set)       , 0 , 1 , 0x24, 0x00, 0, { O(GbMem)          , U                 , U                 , U                 , U                  }, F(Move)                            , U                    },
  { Enc(X86Set)       , 0 , 1 , 0x20, 0x00, 0, { O(GbMem)          , U                 , U                 , U                 , U                  }, F(Move)                            , U                    },
  { Enc(X86Set)       , 0 , 1 , 0x04, 0x00, 0, { O(GbMem)          , U                 , U                 , U                 , U                  }, F(Move)                            , U                    },
  { Enc(X86Set)       , 0 , 1 , 0x07, 0x00, 0, { O(GbMem)          , U                 , U                 , U                 , U                  }, F(Move)                            , U                    },
  { Enc(X86Set)       , 0 , 1 , 0x03, 0x00, 0, { O(GbMem)          , U                 , U                 , U                 , U                  }, F(Move)                            , U                    },
  { Enc(X86Set)       , 0 , 1 , 0x01, 0x00, 0, { O(GbMem)          , U                 , U                 , U                 , U                  }, F(Move)                            , U                    },
  { Enc(X86Set)       , 0 , 1 , 0x10, 0x00, 0, { O(GbMem)          , U                 , U                 , U                 , U                  }, F(Move)                            , U                    },
  { Enc(X86Set)       , 0 , 1 , 0x02, 0x00, 0, { O(GbMem)          , U                 , U                 , U                 , U                  }, F(Move)                            , U                    },
  { Enc(X86Shlrd)     , 0 , 0 , 0x00, 0x3F, 0, { O(GqdwbMem)       , O(Gb)             , U                 , U                 , U                  }, F(None)|F(Special)                 , U                    },
  { Enc(X86Shlrd)     , 0 , 0 , 0x00, 0x3F, 0, { O(GqdwbMem)       , O(Gqdwb)          , U                 , U                 , U                  }, F(None)|F(Special)                 , U                    },
  { Enc(X86Op)        , 0 , 0 , 0x40, 0x00, 0, { U                 , U                 , U                 , U                 , U                  }, F(None)|F(Special)                 , U                    },
  { Enc(X86Op_66H)    , 0 , 0 , 0x40, 0x00, 0, { U                 , U                 , U                 , U                 , U                  }, F(None)|F(Special)                 , U                    },
  { Enc(X86Test)      , 0 , 0 , 0x00, 0x3F, 0, { O(GqdwbMem)       , O(Gqdwb)|O(Imm)   , U                 , U                 , U                  }, F(Test)                            , O_000000(F6,U,_,_,_) },
  { Enc(X86RegRm)     , 0 , 0 , 0x00, 0x3F, 0, { O(Gqdw)           , O(GqdwMem)        , U                 , U                 , U                  }, F(Move)                            , U                    },
  { Enc(AvxRvm_P)     , 0 , 0 , 0x00, 0x00, 0, { O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                  }, F(Avx)                             , U                    },
  { Enc(AvxRvm)       , 0 , 0 , 0x00, 0x00, 0, { O(Xmm)            , O(Xmm)            , O(XmmMem)         , U                 , U                  }, F(Avx)                             , U                    },
  { Enc(AvxRm)        , 0 , 0 , 0x00, 0x00, 0, { O(Xmm)            , O(XmmMem)         , U                 , U                 , U                  }, F(Avx)                             , U                    },
  { Enc(AvxRmi)       , 0 , 0 , 0x00, 0x00, 0, { O(Xmm)            , O(XmmMem)         , O(Imm)            , U                 , U                  }, F(Avx)                             , U                    },
  { Enc(AvxRvmi_P)    , 0 , 0 , 0x00, 0x00, 0, { O(Xy)             , O(Xy)             , O(XyMem)          , O(Imm)            , U                  }, F(Avx)                             , U                    },
  { Enc(AvxRvmr_P)    , 0 , 0 , 0x00, 0x00, 0, { O(Xy)             , O(Xy)             , O(XyMem)          , O(Xy)             , U                  }, F(Avx)                             , U                    },
  { Enc(AvxRm)        , 0 , 0 , 0x00, 0x00, 0, { O(Ymm)            , O(Mem)            , U                 , U                 , U                  }, F(Avx)                             , U                    },
  { Enc(AvxRm)        , 0 , 0 , 0x00, 0x00, 0, { O(Ymm)            , O(XmmMem)         , U                 , U                 , U                  }, F(Avx)                             , U                    },
  { Enc(AvxRm_P)      , 0 , 0 , 0x00, 0x00, 0, { O(Xy)             , O(XmmMem)         , U                 , U                 , U                  }, F(Avx)                             , U                    },
  { Enc(AvxRvmi)      , 0 , 0 , 0x00, 0x00, 0, { O(Xmm)            , O(Xmm)            , O(XmmMem)         , O(Imm)            , U                  }, F(Avx)                             , U                    },
  { Enc(AvxRm_P)      , 0 , 0 , 0x00, 0x00, 0, { O(Xy)             , O(XyMem)          , U                 , U                 , U                  }, F(Avx)                             , U                    },
  { Enc(AvxRm)        , 0 , 0 , 0x00, 0x00, 0, { O(Xmm)            , O(XyMem)          , U                 , U                 , U                  }, F(Avx)                             , U                    },
  { Enc(AvxMri_P)     , 0 , 0 , 0x00, 0x00, 0, { O(XmmMem)         , O(Xy)             , O(Imm)            , U                 , U                  }, F(Avx)                             , U                    },
  { Enc(AvxRm)        , 0 , 0 , 0x00, 0x00, 0, { O(Gqd)            , O(XmmMem)         , U                 , U                 , U                  }, F(Avx)                             , U                    },
  { Enc(AvxRvm)       , 0 , 0 , 0x00, 0x00, 0, { O(Xmm)            , O(Xmm)            , O(GqdMem)         , U                 , U                  }, F(Avx)                             , U                    },
  { Enc(AvxRm_P)      , 0 , 0 , 0x00, 0x00, 0, { O(Xmm)            , O(XyMem)          , U                 , U                 , U                  }, F(Avx)                             , U                    },
  { Enc(AvxMri)       , 0 , 0 , 0x00, 0x00, 0, { O(XmmMem)         , O(Ymm)            , O(Imm)            , U                 , U                  }, F(Avx)                             , U                    },
  { Enc(AvxMri)       , 0 , 0 , 0x00, 0x00, 0, { O(GqdMem)         , O(Xmm)            , O(Imm)            , U                 , U                  }, F(Avx)                             , U                    },
  { Enc(Fma4_P)       , 0 , 0 , 0x00, 0x00, 0, { O(Xy)             , O(Xy)             , O(XyMem)          , O(XyMem)          , U                  }, F(Avx)                             , U                    },
  { Enc(Fma4)         , 0 , 0 , 0x00, 0x00, 0, { O(Xmm)            , O(Xmm)            , O(XmmMem)         , O(XmmMem)         , U                  }, F(Avx)                             , U                    },
  { Enc(XopRm_P)      , 0 , 0 , 0x00, 0x00, 0, { O(Xy)             , O(XyMem)          , U                 , U                 , U                  }, F(Avx)                             , U                    },
  { Enc(XopRm)        , 0 , 0 , 0x00, 0x00, 0, { O(Xmm)            , O(XmmMem)         , U                 , U                 , U                  }, F(Avx)                             , U                    },
  { Enc(AvxGather)    , 0 , 0 , 0x00, 0x00, 0, { O(Xy)             , O(Mem)            , O(Xy)             , U                 , U                  }, F(Avx)                             , U                    },
  { Enc(AvxGatherEx)  , 0 , 0 , 0x00, 0x00, 0, { O(Xmm)            , O(Mem)            , O(Xmm)            , U                 , U                  }, F(Avx)                             , U                    },
  { Enc(AvxRvmi)      , 0 , 0 , 0x00, 0x00, 0, { O(Ymm)            , O(Ymm)            , O(XmmMem)         , O(Imm)            , U                  }, F(Avx)                             , U                    },
  { Enc(AvxRm_P)      , 0 , 0 , 0x00, 0x00, 0, { O(Xy)             , O(Mem)            , U                 , U                 , U                  }, F(Avx)                             , U                    },
  { Enc(AvxM)         , 0 , 0 , 0x00, 0x00, 0, { O(Mem)            , U                 , U                 , U                 , U                  }, F(Avx)                             , U                    },
  { Enc(AvxRm)        , 0 , 0 , 0x00, 0x00, 0, { O(Xmm)            , O(Xmm)            , U                 , U                 , U                  }, F(Avx)                             , U                    },
  { Enc(AvxRvmMvr_P)  , 0 , 0 , 0x00, 0x00, 0, { O(XyMem)          , O(Xy)             , O(XyMem)          , U                 , U                  }, F(Avx)                             , O_660F38(2F,U,_,_,_) },
  { Enc(AvxRvmMvr_P)  , 0 , 0 , 0x00, 0x00, 0, { O(XyMem)          , O(Xy)             , O(XyMem)          , U                 , U                  }, F(Avx)                             , O_660F38(2E,U,_,_,_) },
  { Enc(AvxRmMr_P)    , 0 , 0 , 0x00, 0x00, 0, { O(XyMem)          , O(XyMem)          , U                 , U                 , U                  }, F(Avx)                             , O_660F00(29,U,_,_,_) },
  { Enc(AvxRmMr_P)    , 0 , 0 , 0x00, 0x00, 0, { O(XyMem)          , O(XyMem)          , U                 , U                 , U                  }, F(Avx)                             , O_000F00(29,U,_,_,_) },
  { Enc(AvxMovDQ)     , 0 , 0 , 0x00, 0x00, 0, { O(XmmMem)         , O(XmmMem)         , U                 , U                 , U                  }, F(Avx)                             , O_660F00(7E,U,_,_,_) },
  { Enc(AvxRmMr_P)    , 0 , 0 , 0x00, 0x00, 0, { O(XyMem)          , O(XyMem)          , U                 , U                 , U                  }, F(Avx)                             , O_660F00(7F,U,_,_,_) },
  { Enc(AvxRmMr_P)    , 0 , 0 , 0x00, 0x00, 0, { O(XyMem)          , O(XyMem)          , U                 , U                 , U                  }, F(Avx)                             , O_F30F00(7F,U,_,_,_) },
  { Enc(AvxRvm)       , 0 , 0 , 0x00, 0x00, 0, { O(Xmm)            , O(Xmm)            , O(Xmm)            , U                 , U                  }, F(Avx)                             , U                    },
  { Enc(AvxRvmMr)     , 0 , 0 , 0x00, 0x00, 0, { O(XmmMem)         , O(Xmm)            , O(Mem)            , U                 , U                  }, F(Avx)                             , O_660F00(17,U,_,_,_) },
  { Enc(AvxRvmMr)     , 0 , 0 , 0x00, 0x00, 0, { O(XmmMem)         , O(Xmm)            , O(Mem)            , U                 , U                  }, F(Avx)                             , O_000F00(17,U,_,_,_) },
  { Enc(AvxRvmMr)     , 0 , 0 , 0x00, 0x00, 0, { O(XmmMem)         , O(Xmm)            , O(Mem)            , U                 , U                  }, F(Avx)                             , O_660F00(13,U,_,_,_) },
  { Enc(AvxRvmMr)     , 0 , 0 , 0x00, 0x00, 0, { O(XmmMem)         , O(Xmm)            , O(Mem)            , U                 , U                  }, F(Avx)                             , O_000F00(13,U,_,_,_) },
  { Enc(AvxRm_P)      , 0 , 0 , 0x00, 0x00, 0, { O(Gqd)            , O(Xy)             , U                 , U                 , U                  }, F(Avx)                             , U                    },
  { Enc(AvxMr_P)      , 0 , 0 , 0x00, 0x00, 0, { O(Mem)            , O(Xy)             , U                 , U                 , U                  }, F(Avx)                             , U                    },
  { Enc(AvxMovSsSd)   , 0 , 0 , 0x00, 0x00, 0, { O(XmmMem)         , O(XmmMem)         , O(Xmm)            , U                 , U                  }, F(Avx)                             , O_F20F00(11,U,_,_,_) },
  { Enc(AvxMovSsSd)   , 0 , 0 , 0x00, 0x00, 0, { O(XmmMem)         , O(Xmm)            , O(Xmm)            , U                 , U                  }, F(Avx)                             , O_F30F00(11,U,_,_,_) },
  { Enc(AvxRmMr_P)    , 0 , 0 , 0x00, 0x00, 0, { O(XyMem)          , O(XyMem)          , U                 , U                 , U                  }, F(Avx)                             , O_660F00(11,U,_,_,_) },
  { Enc(AvxRmMr_P)    , 0 , 0 , 0x00, 0x00, 0, { O(XyMem)          , O(XyMem)          , U                 , U                 , U                  }, F(Avx)                             , O_000F00(11,U,_,_,_) },
  { Enc(AvxRvmr)      , 0 , 0 , 0x00, 0x00, 0, { O(Xy)             , O(Xy)             , O(XyMem)          , O(Xy)             , U                  }, F(Avx)                             , U                    },
  { Enc(XopRvrmRvmr_P), 0 , 0 , 0x00, 0x00, 0, { O(Xy)             , O(Xy)             , O(XyMem)          , O(XyMem)          , U                  }, F(Avx)                             , U                    },
  { Enc(XopRvmi)      , 0 , 0 , 0x00, 0x00, 0, { O(Xmm)            , O(Xmm)            , O(XmmMem)         , O(Imm)            , U                  }, F(Avx)                             , U                    },
  { Enc(AvxRvmi)      , 0 , 0 , 0x00, 0x00, 0, { O(Ymm)            , O(Ymm)            , O(YmmMem)         , O(Imm)            , U                  }, F(Avx)                             , U                    },
  { Enc(AvxRvm)       , 0 , 0 , 0x00, 0x00, 0, { O(Ymm)            , O(Ymm)            , O(YmmMem)         , U                 , U                  }, F(Avx)                             , U                    },
  { Enc(AvxRvrmRvmr_P), 0 , 0 , 0x00, 0x00, 0, { O(Xy)             , O(Xy)             , O(XyMem)          , O(XyMem)          , U                  }, F(Avx)                             , U                    },
  { Enc(AvxRvmRmi_P)  , 0 , 0 , 0x00, 0x00, 0, { O(Xy)             , O(XyMem)          , O(XyMem)|O(Imm)   , U                 , U                  }, F(Avx)                             , O_660F3A(05,U,_,_,_) },
  { Enc(AvxRvmRmi_P)  , 0 , 0 , 0x00, 0x00, 0, { O(Xy)             , O(XyMem)          , O(XyMem)|O(Imm)   , U                 , U                  }, F(Avx)                             , O_660F3A(04,U,_,_,_) },
  { Enc(AvxRmi)       , 0 , 0 , 0x00, 0x00, 0, { O(Ymm)            , O(YmmMem)         , O(Imm)            , U                 , U                  }, F(Avx)                             , U                    },
  { Enc(AvxMri)       , 0 , 0 , 0x00, 0x00, 0, { O(GqdwbMem)       , O(Xmm)            , O(Imm)            , U                 , U                  }, F(Avx)                             , U                    },
  { Enc(AvxMri)       , 0 , 0 , 0x00, 0x00, 0, { O(GqMem)          , O(Xmm)            , O(Imm)            , U                 , U                  }, F(Avx)                             , U                    },
  { Enc(AvxMri)       , 0 , 0 , 0x00, 0x00, 0, { O(GqdwMem)        , O(Xmm)            , O(Imm)            , U                 , U                  }, F(Avx)                             , U                    },
  { Enc(AvxRvmi)      , 0 , 0 , 0x00, 0x00, 0, { O(Xmm)            , O(Xmm)            , O(GqdwbMem)       , O(Imm)            , U                  }, F(Avx)                             , U                    },
  { Enc(AvxRvmi)      , 0 , 0 , 0x00, 0x00, 0, { O(Xmm)            , O(Xmm)            , O(GqdMem)         , O(Imm)            , U                  }, F(Avx)                             , U                    },
  { Enc(AvxRvmi)      , 0 , 0 , 0x00, 0x00, 0, { O(Xmm)            , O(Xmm)            , O(GqMem)          , O(Imm)            , U                  }, F(Avx)                             , U                    },
  { Enc(AvxRvmi)      , 0 , 0 , 0x00, 0x00, 0, { O(Xmm)            , O(Xmm)            , O(GqdwMem)        , O(Imm)            , U                  }, F(Avx)                             , U                    },
  { Enc(XopRvmr)      , 0 , 0 , 0x00, 0x00, 0, { O(Xmm)            , O(Xmm)            , O(XmmMem)         , O(Xmm)            , U                  }, F(Avx)                             , U                    },
  { Enc(AvxRvmMvr_P)  , 0 , 0 , 0x00, 0x00, 0, { O(XyMem)          , O(Xy)             , O(XyMem)          , U                 , U                  }, F(Avx)                             , O_660F38(8E,U,_,_,_) },
  { Enc(XopRvrmRvmr)  , 0 , 0 , 0x00, 0x00, 0, { O(Xmm)            , O(Xmm)            , O(XmmMem)         , O(XmmMem)         , U                  }, F(Avx)                             , U                    },
  { Enc(XopRvmRmi)    , 0 , 0 , 0x00, 0x00, 0, { O(Xmm)            , O(XmmMem)         , O(XmmMem)|O(Imm)  , U                 , U                  }, F(Avx)                             , O_00_M08(C0,U,_,_,_) },
  { Enc(XopRvmRmi)    , 0 , 0 , 0x00, 0x00, 0, { O(Xmm)            , O(XmmMem)         , O(XmmMem)|O(Imm)  , U                 , U                  }, F(Avx)                             , O_00_M08(C2,U,_,_,_) },
  { Enc(XopRvmRmi)    , 0 , 0 , 0x00, 0x00, 0, { O(Xmm)            , O(XmmMem)         , O(XmmMem)|O(Imm)  , U                 , U                  }, F(Avx)                             , O_00_M08(C3,U,_,_,_) },
  { Enc(XopRvmRmi)    , 0 , 0 , 0x00, 0x00, 0, { O(Xmm)            , O(XmmMem)         , O(XmmMem)|O(Imm)  , U                 , U                  }, F(Avx)                             , O_00_M08(C1,U,_,_,_) },
  { Enc(XopRvmRmv)    , 0 , 0 , 0x00, 0x00, 0, { O(Xmm)            , O(XmmMem)         , O(XmmMem)         , U                 , U                  }, F(Avx)                             , U                    },
  { Enc(AvxRmi_P)     , 0 , 0 , 0x00, 0x00, 0, { O(Xy)             , O(XyMem)          , O(Imm)            , U                 , U                  }, F(Avx)                             , U                    },
  { Enc(AvxRvmVmi_P)  , 0 , 0 , 0x00, 0x00, 0, { O(Xy)             , O(XyMem)          , O(XyMem)|O(Imm)   , U                 , U                  }, F(Avx)                             , O_660F00(72,6,_,_,_) },
  { Enc(AvxVmi_P)     , 0 , 0 , 0x00, 0x00, 0, { O(Xy)             , O(XyMem)          , O(Imm)            , U                 , U                  }, F(Avx)                             , U                    },
  { Enc(AvxRvmVmi_P)  , 0 , 0 , 0x00, 0x00, 0, { O(Xy)             , O(XyMem)          , O(XyMem)|O(Imm)   , U                 , U                  }, F(Avx)                             , O_660F00(73,6,_,_,_) },
  { Enc(AvxRvmVmi_P)  , 0 , 0 , 0x00, 0x00, 0, { O(Xy)             , O(XyMem)          , O(XyMem)|O(Imm)   , U                 , U                  }, F(Avx)                             , O_660F00(71,6,_,_,_) },
  { Enc(AvxRvmVmi_P)  , 0 , 0 , 0x00, 0x00, 0, { O(Xy)             , O(XyMem)          , O(XyMem)|O(Imm)   , U                 , U                  }, F(Avx)                             , O_660F00(72,4,_,_,_) },
  { Enc(AvxRvmVmi_P)  , 0 , 0 , 0x00, 0x00, 0, { O(Xy)             , O(XyMem)          , O(XyMem)|O(Imm)   , U                 , U                  }, F(Avx)                             , O_660F00(71,4,_,_,_) },
  { Enc(AvxRvmVmi_P)  , 0 , 0 , 0x00, 0x00, 0, { O(Xy)             , O(XyMem)          , O(XyMem)|O(Imm)   , U                 , U                  }, F(Avx)                             , O_660F00(72,2,_,_,_) },
  { Enc(AvxRvmVmi_P)  , 0 , 0 , 0x00, 0x00, 0, { O(Xy)             , O(XyMem)          , O(XyMem)|O(Imm)   , U                 , U                  }, F(Avx)                             , O_660F00(73,2,_,_,_) },
  { Enc(AvxRvmVmi_P)  , 0 , 0 , 0x00, 0x00, 0, { O(Xy)             , O(XyMem)          , O(XyMem)|O(Imm)   , U                 , U                  }, F(Avx)                             , O_660F00(71,2,_,_,_) },
  { Enc(AvxRm_P)      , 0 , 0 , 0x00, 0x3F, 0, { O(Xy)             , O(XyMem)          , U                 , U                 , U                  }, F(Avx)                             , U                    },
  { Enc(AvxRm_P)      , 0 , 0 , 0x00, 0x3F, 0, { O(Xy)             , O(XyMem)          , U                 , U                 , U                  }, F(Test)                            , U                    },
  { Enc(AvxRm)        , 0 , 0 , 0x00, 0x3F, 0, { O(Xmm)            , O(XmmMem)         , U                 , U                 , U                  }, F(Avx)                             , U                    },
  { Enc(AvxOp)        , 0 , 0 , 0x00, 0x00, 0, { U                 , U                 , U                 , U                 , U                  }, F(Avx)                             , U                    },
  { Enc(X86Rm)        , 0 , 0 , 0x00, 0x00, 0, { O(Gqd)            , U                 , U                 , U                 , U                  }, F(None)                            , U                    },
  { Enc(X86Xadd)      , 0 , 0 , 0x00, 0x3F, 0, { O(GqdwbMem)       , O(Gqdwb)          , U                 , U                 , U                  }, F(Xchg)|F(Lock)                    , U                    },
  { Enc(X86Xchg)      , 0 , 0 , 0x00, 0x00, 0, { O(GqdwbMem)       , O(Gqdwb)          , U                 , U                 , U                  }, F(Xchg)|F(Lock)                    , U                    },
  { Enc(X86M)         , 0 , 0 , 0x00, 0x00, 0, { O(Mem)            , U                 , U                 , U                 , U                  }, F(None)|F(Special)                 , U                    }
};

// Automatically generated, do not edit.
enum X86InstData_ExtendedIndex {
  kInstIdNone_ExtendedIndex = 0,
  kX86InstIdAdc_ExtendedIndex = 1,
  kX86InstIdAdd_ExtendedIndex = 2,
  kX86InstIdAddpd_ExtendedIndex = 3,
  kX86InstIdAddps_ExtendedIndex = 3,
  kX86InstIdAddsd_ExtendedIndex = 3,
  kX86InstIdAddss_ExtendedIndex = 3,
  kX86InstIdAddsubpd_ExtendedIndex = 3,
  kX86InstIdAddsubps_ExtendedIndex = 3,
  kX86InstIdAesdec_ExtendedIndex = 3,
  kX86InstIdAesdeclast_ExtendedIndex = 3,
  kX86InstIdAesenc_ExtendedIndex = 3,
  kX86InstIdAesenclast_ExtendedIndex = 3,
  kX86InstIdAesimc_ExtendedIndex = 3,
  kX86InstIdAeskeygenassist_ExtendedIndex = 4,
  kX86InstIdAnd_ExtendedIndex = 2,
  kX86InstIdAndn_ExtendedIndex = 5,
  kX86InstIdAndnpd_ExtendedIndex = 3,
  kX86InstIdAndnps_ExtendedIndex = 3,
  kX86InstIdAndpd_ExtendedIndex = 3,
  kX86InstIdAndps_ExtendedIndex = 3,
  kX86InstIdBextr_ExtendedIndex = 6,
  kX86InstIdBlendpd_ExtendedIndex = 4,
  kX86InstIdBlendps_ExtendedIndex = 4,
  kX86InstIdBlendvpd_ExtendedIndex = 7,
  kX86InstIdBlendvps_ExtendedIndex = 7,
  kX86InstIdBlsi_ExtendedIndex = 8,
  kX86InstIdBlsmsk_ExtendedIndex = 8,
  kX86InstIdBlsr_ExtendedIndex = 8,
  kX86InstIdBsf_ExtendedIndex = 9,
  kX86InstIdBsr_ExtendedIndex = 9,
  kX86InstIdBswap_ExtendedIndex = 10,
  kX86InstIdBt_ExtendedIndex = 11,
  kX86InstIdBtc_ExtendedIndex = 12,
  kX86InstIdBtr_ExtendedIndex = 13,
  kX86InstIdBts_ExtendedIndex = 14,
  kX86InstIdBzhi_ExtendedIndex = 6,
  kX86InstIdCall_ExtendedIndex = 15,
  kX86InstIdCbw_ExtendedIndex = 16,
  kX86InstIdCdq_ExtendedIndex = 16,
  kX86InstIdCdqe_ExtendedIndex = 16,
  kX86InstIdClc_ExtendedIndex = 17,
  kX86InstIdCld_ExtendedIndex = 18,
  kX86InstIdClflush_ExtendedIndex = 19,
  kX86InstIdCmc_ExtendedIndex = 20,
  kX86InstIdCmova_ExtendedIndex = 21,
  kX86InstIdCmovae_ExtendedIndex = 22,
  kX86InstIdCmovb_ExtendedIndex = 22,
  kX86InstIdCmovbe_ExtendedIndex = 21,
  kX86InstIdCmovc_ExtendedIndex = 22,
  kX86InstIdCmove_ExtendedIndex = 23,
  kX86InstIdCmovg_ExtendedIndex = 24,
  kX86InstIdCmovge_ExtendedIndex = 25,
  kX86InstIdCmovl_ExtendedIndex = 25,
  kX86InstIdCmovle_ExtendedIndex = 24,
  kX86InstIdCmovna_ExtendedIndex = 21,
  kX86InstIdCmovnae_ExtendedIndex = 22,
  kX86InstIdCmovnb_ExtendedIndex = 22,
  kX86InstIdCmovnbe_ExtendedIndex = 21,
  kX86InstIdCmovnc_ExtendedIndex = 22,
  kX86InstIdCmovne_ExtendedIndex = 23,
  kX86InstIdCmovng_ExtendedIndex = 24,
  kX86InstIdCmovnge_ExtendedIndex = 25,
  kX86InstIdCmovnl_ExtendedIndex = 25,
  kX86InstIdCmovnle_ExtendedIndex = 24,
  kX86InstIdCmovno_ExtendedIndex = 26,
  kX86InstIdCmovnp_ExtendedIndex = 27,
  kX86InstIdCmovns_ExtendedIndex = 28,
  kX86InstIdCmovnz_ExtendedIndex = 23,
  kX86InstIdCmovo_ExtendedIndex = 26,
  kX86InstIdCmovp_ExtendedIndex = 27,
  kX86InstIdCmovpe_ExtendedIndex = 27,
  kX86InstIdCmovpo_ExtendedIndex = 27,
  kX86InstIdCmovs_ExtendedIndex = 28,
  kX86InstIdCmovz_ExtendedIndex = 23,
  kX86InstIdCmp_ExtendedIndex = 29,
  kX86InstIdCmppd_ExtendedIndex = 4,
  kX86InstIdCmpps_ExtendedIndex = 4,
  kX86InstIdCmpsB_ExtendedIndex = 30,
  kX86InstIdCmpsD_ExtendedIndex = 30,
  kX86InstIdCmpsQ_ExtendedIndex = 30,
  kX86InstIdCmpsW_ExtendedIndex = 31,
  kX86InstIdCmpsd_ExtendedIndex = 4,
  kX86InstIdCmpss_ExtendedIndex = 4,
  kX86InstIdCmpxchg_ExtendedIndex = 32,
  kX86InstIdCmpxchg16b_ExtendedIndex = 33,
  kX86InstIdCmpxchg8b_ExtendedIndex = 33,
  kX86InstIdComisd_ExtendedIndex = 34,
  kX86InstIdComiss_ExtendedIndex = 34,
  kX86InstIdCpuid_ExtendedIndex = 16,
  kX86InstIdCqo_ExtendedIndex = 16,
  kX86InstIdCrc32_ExtendedIndex = 35,
  kX86InstIdCvtdq2pd_ExtendedIndex = 36,
  kX86InstIdCvtdq2ps_ExtendedIndex = 36,
  kX86InstIdCvtpd2dq_ExtendedIndex = 36,
  kX86InstIdCvtpd2pi_ExtendedIndex = 37,
  kX86InstIdCvtpd2ps_ExtendedIndex = 36,
  kX86InstIdCvtpi2pd_ExtendedIndex = 38,
  kX86InstIdCvtpi2ps_ExtendedIndex = 39,
  kX86InstIdCvtps2dq_ExtendedIndex = 36,
  kX86InstIdCvtps2pd_ExtendedIndex = 36,
  kX86InstIdCvtps2pi_ExtendedIndex = 37,
  kX86InstIdCvtsd2si_ExtendedIndex = 40,
  kX86InstIdCvtsd2ss_ExtendedIndex = 41,
  kX86InstIdCvtsi2sd_ExtendedIndex = 42,
  kX86InstIdCvtsi2ss_ExtendedIndex = 43,
  kX86InstIdCvtss2sd_ExtendedIndex = 44,
  kX86InstIdCvtss2si_ExtendedIndex = 40,
  kX86InstIdCvttpd2dq_ExtendedIndex = 36,
  kX86InstIdCvttpd2pi_ExtendedIndex = 37,
  kX86InstIdCvttps2dq_ExtendedIndex = 36,
  kX86InstIdCvttps2pi_ExtendedIndex = 37,
  kX86InstIdCvttsd2si_ExtendedIndex = 40,
  kX86InstIdCvttss2si_ExtendedIndex = 40,
  kX86InstIdCwd_ExtendedIndex = 16,
  kX86InstIdCwde_ExtendedIndex = 16,
  kX86InstIdDaa_ExtendedIndex = 45,
  kX86InstIdDas_ExtendedIndex = 45,
  kX86InstIdDec_ExtendedIndex = 46,
  kX86InstIdDiv_ExtendedIndex = 47,
  kX86InstIdDivpd_ExtendedIndex = 3,
  kX86InstIdDivps_ExtendedIndex = 3,
  kX86InstIdDivsd_ExtendedIndex = 3,
  kX86InstIdDivss_ExtendedIndex = 3,
  kX86InstIdDppd_ExtendedIndex = 4,
  kX86InstIdDpps_ExtendedIndex = 4,
  kX86InstIdEmms_ExtendedIndex = 48,
  kX86InstIdEnter_ExtendedIndex = 49,
  kX86InstIdExtractps_ExtendedIndex = 50,
  kX86InstIdExtrq_ExtendedIndex = 51,
  kX86InstIdF2xm1_ExtendedIndex = 52,
  kX86InstIdFabs_ExtendedIndex = 52,
  kX86InstIdFadd_ExtendedIndex = 53,
  kX86InstIdFaddp_ExtendedIndex = 54,
  kX86InstIdFbld_ExtendedIndex = 55,
  kX86InstIdFbstp_ExtendedIndex = 55,
  kX86InstIdFchs_ExtendedIndex = 52,
  kX86InstIdFclex_ExtendedIndex = 52,
  kX86InstIdFcmovb_ExtendedIndex = 56,
  kX86InstIdFcmovbe_ExtendedIndex = 57,
  kX86InstIdFcmove_ExtendedIndex = 58,
  kX86InstIdFcmovnb_ExtendedIndex = 56,
  kX86InstIdFcmovnbe_ExtendedIndex = 57,
  kX86InstIdFcmovne_ExtendedIndex = 58,
  kX86InstIdFcmovnu_ExtendedIndex = 59,
  kX86InstIdFcmovu_ExtendedIndex = 59,
  kX86InstIdFcom_ExtendedIndex = 60,
  kX86InstIdFcomi_ExtendedIndex = 61,
  kX86InstIdFcomip_ExtendedIndex = 61,
  kX86InstIdFcomp_ExtendedIndex = 60,
  kX86InstIdFcompp_ExtendedIndex = 52,
  kX86InstIdFcos_ExtendedIndex = 52,
  kX86InstIdFdecstp_ExtendedIndex = 52,
  kX86InstIdFdiv_ExtendedIndex = 53,
  kX86InstIdFdivp_ExtendedIndex = 54,
  kX86InstIdFdivr_ExtendedIndex = 53,
  kX86InstIdFdivrp_ExtendedIndex = 54,
  kX86InstIdFemms_ExtendedIndex = 62,
  kX86InstIdFfree_ExtendedIndex = 63,
  kX86InstIdFiadd_ExtendedIndex = 64,
  kX86InstIdFicom_ExtendedIndex = 64,
  kX86InstIdFicomp_ExtendedIndex = 64,
  kX86InstIdFidiv_ExtendedIndex = 64,
  kX86InstIdFidivr_ExtendedIndex = 64,
  kX86InstIdFild_ExtendedIndex = 65,
  kX86InstIdFimul_ExtendedIndex = 64,
  kX86InstIdFincstp_ExtendedIndex = 52,
  kX86InstIdFinit_ExtendedIndex = 52,
  kX86InstIdFist_ExtendedIndex = 64,
  kX86InstIdFistp_ExtendedIndex = 66,
  kX86InstIdFisttp_ExtendedIndex = 67,
  kX86InstIdFisub_ExtendedIndex = 64,
  kX86InstIdFisubr_ExtendedIndex = 64,
  kX86InstIdFld_ExtendedIndex = 68,
  kX86InstIdFld1_ExtendedIndex = 52,
  kX86InstIdFldcw_ExtendedIndex = 55,
  kX86InstIdFldenv_ExtendedIndex = 55,
  kX86InstIdFldl2e_ExtendedIndex = 52,
  kX86InstIdFldl2t_ExtendedIndex = 52,
  kX86InstIdFldlg2_ExtendedIndex = 52,
  kX86InstIdFldln2_ExtendedIndex = 52,
  kX86InstIdFldpi_ExtendedIndex = 52,
  kX86InstIdFldz_ExtendedIndex = 52,
  kX86InstIdFmul_ExtendedIndex = 53,
  kX86InstIdFmulp_ExtendedIndex = 54,
  kX86InstIdFnclex_ExtendedIndex = 52,
  kX86InstIdFninit_ExtendedIndex = 52,
  kX86InstIdFnop_ExtendedIndex = 52,
  kX86InstIdFnsave_ExtendedIndex = 55,
  kX86InstIdFnstcw_ExtendedIndex = 55,
  kX86InstIdFnstenv_ExtendedIndex = 55,
  kX86InstIdFnstsw_ExtendedIndex = 69,
  kX86InstIdFpatan_ExtendedIndex = 52,
  kX86InstIdFprem_ExtendedIndex = 52,
  kX86InstIdFprem1_ExtendedIndex = 52,
  kX86InstIdFptan_ExtendedIndex = 52,
  kX86InstIdFrndint_ExtendedIndex = 52,
  kX86InstIdFrstor_ExtendedIndex = 55,
  kX86InstIdFsave_ExtendedIndex = 55,
  kX86InstIdFscale_ExtendedIndex = 52,
  kX86InstIdFsin_ExtendedIndex = 52,
  kX86InstIdFsincos_ExtendedIndex = 52,
  kX86InstIdFsqrt_ExtendedIndex = 52,
  kX86InstIdFst_ExtendedIndex = 70,
  kX86InstIdFstcw_ExtendedIndex = 55,
  kX86InstIdFstenv_ExtendedIndex = 55,
  kX86InstIdFstp_ExtendedIndex = 71,
  kX86InstIdFstsw_ExtendedIndex = 72,
  kX86InstIdFsub_ExtendedIndex = 53,
  kX86InstIdFsubp_ExtendedIndex = 54,
  kX86InstIdFsubr_ExtendedIndex = 53,
  kX86InstIdFsubrp_ExtendedIndex = 54,
  kX86InstIdFtst_ExtendedIndex = 52,
  kX86InstIdFucom_ExtendedIndex = 54,
  kX86InstIdFucomi_ExtendedIndex = 61,
  kX86InstIdFucomip_ExtendedIndex = 61,
  kX86InstIdFucomp_ExtendedIndex = 54,
  kX86InstIdFucompp_ExtendedIndex = 52,
  kX86InstIdFwait_ExtendedIndex = 62,
  kX86InstIdFxam_ExtendedIndex = 52,
  kX86InstIdFxch_ExtendedIndex = 63,
  kX86InstIdFxrstor_ExtendedIndex = 55,
  kX86InstIdFxsave_ExtendedIndex = 55,
  kX86InstIdFxtract_ExtendedIndex = 52,
  kX86InstIdFyl2x_ExtendedIndex = 52,
  kX86InstIdFyl2xp1_ExtendedIndex = 52,
  kX86InstIdHaddpd_ExtendedIndex = 3,
  kX86InstIdHaddps_ExtendedIndex = 3,
  kX86InstIdHsubpd_ExtendedIndex = 3,
  kX86InstIdHsubps_ExtendedIndex = 3,
  kX86InstIdIdiv_ExtendedIndex = 73,
  kX86InstIdImul_ExtendedIndex = 74,
  kX86InstIdInc_ExtendedIndex = 75,
  kX86InstIdInsertps_ExtendedIndex = 4,
  kX86InstIdInsertq_ExtendedIndex = 76,
  kX86InstIdInt_ExtendedIndex = 77,
  kX86InstIdJa_ExtendedIndex = 78,
  kX86InstIdJae_ExtendedIndex = 79,
  kX86InstIdJb_ExtendedIndex = 79,
  kX86InstIdJbe_ExtendedIndex = 78,
  kX86InstIdJc_ExtendedIndex = 79,
  kX86InstIdJe_ExtendedIndex = 80,
  kX86InstIdJg_ExtendedIndex = 81,
  kX86InstIdJge_ExtendedIndex = 82,
  kX86InstIdJl_ExtendedIndex = 82,
  kX86InstIdJle_ExtendedIndex = 81,
  kX86InstIdJna_ExtendedIndex = 78,
  kX86InstIdJnae_ExtendedIndex = 79,
  kX86InstIdJnb_ExtendedIndex = 79,
  kX86InstIdJnbe_ExtendedIndex = 78,
  kX86InstIdJnc_ExtendedIndex = 79,
  kX86InstIdJne_ExtendedIndex = 80,
  kX86InstIdJng_ExtendedIndex = 81,
  kX86InstIdJnge_ExtendedIndex = 82,
  kX86InstIdJnl_ExtendedIndex = 82,
  kX86InstIdJnle_ExtendedIndex = 81,
  kX86InstIdJno_ExtendedIndex = 83,
  kX86InstIdJnp_ExtendedIndex = 84,
  kX86InstIdJns_ExtendedIndex = 85,
  kX86InstIdJnz_ExtendedIndex = 80,
  kX86InstIdJo_ExtendedIndex = 83,
  kX86InstIdJp_ExtendedIndex = 84,
  kX86InstIdJpe_ExtendedIndex = 84,
  kX86InstIdJpo_ExtendedIndex = 84,
  kX86InstIdJs_ExtendedIndex = 85,
  kX86InstIdJz_ExtendedIndex = 80,
  kX86InstIdJecxz_ExtendedIndex = 86,
  kX86InstIdJmp_ExtendedIndex = 87,
  kX86InstIdLahf_ExtendedIndex = 88,
  kX86InstIdLddqu_ExtendedIndex = 89,
  kX86InstIdLdmxcsr_ExtendedIndex = 19,
  kX86InstIdLea_ExtendedIndex = 90,
  kX86InstIdLeave_ExtendedIndex = 16,
  kX86InstIdLfence_ExtendedIndex = 91,
  kX86InstIdLodsB_ExtendedIndex = 92,
  kX86InstIdLodsD_ExtendedIndex = 93,
  kX86InstIdLodsQ_ExtendedIndex = 94,
  kX86InstIdLodsW_ExtendedIndex = 95,
  kX86InstIdLzcnt_ExtendedIndex = 9,
  kX86InstIdMaskmovdqu_ExtendedIndex = 96,
  kX86InstIdMaskmovq_ExtendedIndex = 97,
  kX86InstIdMaxpd_ExtendedIndex = 3,
  kX86InstIdMaxps_ExtendedIndex = 3,
  kX86InstIdMaxsd_ExtendedIndex = 3,
  kX86InstIdMaxss_ExtendedIndex = 3,
  kX86InstIdMfence_ExtendedIndex = 91,
  kX86InstIdMinpd_ExtendedIndex = 3,
  kX86InstIdMinps_ExtendedIndex = 3,
  kX86InstIdMinsd_ExtendedIndex = 3,
  kX86InstIdMinss_ExtendedIndex = 3,
  kX86InstIdMonitor_ExtendedIndex = 16,
  kX86InstIdMov_ExtendedIndex = 98,
  kX86InstIdMovPtr_ExtendedIndex = 99,
  kX86InstIdMovapd_ExtendedIndex = 100,
  kX86InstIdMovaps_ExtendedIndex = 101,
  kX86InstIdMovbe_ExtendedIndex = 102,
  kX86InstIdMovd_ExtendedIndex = 103,
  kX86InstIdMovddup_ExtendedIndex = 104,
  kX86InstIdMovdq2q_ExtendedIndex = 105,
  kX86InstIdMovdqa_ExtendedIndex = 106,
  kX86InstIdMovdqu_ExtendedIndex = 107,
  kX86InstIdMovhlps_ExtendedIndex = 108,
  kX86InstIdMovhpd_ExtendedIndex = 109,
  kX86InstIdMovhps_ExtendedIndex = 110,
  kX86InstIdMovlhps_ExtendedIndex = 111,
  kX86InstIdMovlpd_ExtendedIndex = 112,
  kX86InstIdMovlps_ExtendedIndex = 113,
  kX86InstIdMovmskpd_ExtendedIndex = 114,
  kX86InstIdMovmskps_ExtendedIndex = 114,
  kX86InstIdMovntdq_ExtendedIndex = 115,
  kX86InstIdMovntdqa_ExtendedIndex = 116,
  kX86InstIdMovnti_ExtendedIndex = 117,
  kX86InstIdMovntpd_ExtendedIndex = 118,
  kX86InstIdMovntps_ExtendedIndex = 119,
  kX86InstIdMovntq_ExtendedIndex = 120,
  kX86InstIdMovntsd_ExtendedIndex = 121,
  kX86InstIdMovntss_ExtendedIndex = 122,
  kX86InstIdMovq_ExtendedIndex = 123,
  kX86InstIdMovq2dq_ExtendedIndex = 124,
  kX86InstIdMovsB_ExtendedIndex = 16,
  kX86InstIdMovsD_ExtendedIndex = 125,
  kX86InstIdMovsQ_ExtendedIndex = 16,
  kX86InstIdMovsW_ExtendedIndex = 126,
  kX86InstIdMovsd_ExtendedIndex = 127,
  kX86InstIdMovshdup_ExtendedIndex = 36,
  kX86InstIdMovsldup_ExtendedIndex = 36,
  kX86InstIdMovss_ExtendedIndex = 128,
  kX86InstIdMovsx_ExtendedIndex = 129,
  kX86InstIdMovsxd_ExtendedIndex = 130,
  kX86InstIdMovupd_ExtendedIndex = 131,
  kX86InstIdMovups_ExtendedIndex = 132,
  kX86InstIdMovzx_ExtendedIndex = 129,
  kX86InstIdMpsadbw_ExtendedIndex = 4,
  kX86InstIdMul_ExtendedIndex = 73,
  kX86InstIdMulpd_ExtendedIndex = 3,
  kX86InstIdMulps_ExtendedIndex = 3,
  kX86InstIdMulsd_ExtendedIndex = 3,
  kX86InstIdMulss_ExtendedIndex = 3,
  kX86InstIdMulx_ExtendedIndex = 133,
  kX86InstIdMwait_ExtendedIndex = 16,
  kX86InstIdNeg_ExtendedIndex = 134,
  kX86InstIdNop_ExtendedIndex = 48,
  kX86InstIdNot_ExtendedIndex = 135,
  kX86InstIdOr_ExtendedIndex = 2,
  kX86InstIdOrpd_ExtendedIndex = 3,
  kX86InstIdOrps_ExtendedIndex = 3,
  kX86InstIdPabsb_ExtendedIndex = 136,
  kX86InstIdPabsd_ExtendedIndex = 136,
  kX86InstIdPabsw_ExtendedIndex = 136,
  kX86InstIdPackssdw_ExtendedIndex = 136,
  kX86InstIdPacksswb_ExtendedIndex = 136,
  kX86InstIdPackusdw_ExtendedIndex = 3,
  kX86InstIdPackuswb_ExtendedIndex = 136,
  kX86InstIdPaddb_ExtendedIndex = 136,
  kX86InstIdPaddd_ExtendedIndex = 136,
  kX86InstIdPaddq_ExtendedIndex = 136,
  kX86InstIdPaddsb_ExtendedIndex = 136,
  kX86InstIdPaddsw_ExtendedIndex = 136,
  kX86InstIdPaddusb_ExtendedIndex = 136,
  kX86InstIdPaddusw_ExtendedIndex = 136,
  kX86InstIdPaddw_ExtendedIndex = 136,
  kX86InstIdPalignr_ExtendedIndex = 137,
  kX86InstIdPand_ExtendedIndex = 136,
  kX86InstIdPandn_ExtendedIndex = 136,
  kX86InstIdPause_ExtendedIndex = 48,
  kX86InstIdPavgb_ExtendedIndex = 136,
  kX86InstIdPavgw_ExtendedIndex = 136,
  kX86InstIdPblendvb_ExtendedIndex = 7,
  kX86InstIdPblendw_ExtendedIndex = 4,
  kX86InstIdPclmulqdq_ExtendedIndex = 4,
  kX86InstIdPcmpeqb_ExtendedIndex = 136,
  kX86InstIdPcmpeqd_ExtendedIndex = 136,
  kX86InstIdPcmpeqq_ExtendedIndex = 3,
  kX86InstIdPcmpeqw_ExtendedIndex = 136,
  kX86InstIdPcmpestri_ExtendedIndex = 4,
  kX86InstIdPcmpestrm_ExtendedIndex = 4,
  kX86InstIdPcmpgtb_ExtendedIndex = 136,
  kX86InstIdPcmpgtd_ExtendedIndex = 136,
  kX86InstIdPcmpgtq_ExtendedIndex = 3,
  kX86InstIdPcmpgtw_ExtendedIndex = 136,
  kX86InstIdPcmpistri_ExtendedIndex = 4,
  kX86InstIdPcmpistrm_ExtendedIndex = 4,
  kX86InstIdPdep_ExtendedIndex = 133,
  kX86InstIdPext_ExtendedIndex = 133,
  kX86InstIdPextrb_ExtendedIndex = 138,
  kX86InstIdPextrd_ExtendedIndex = 139,
  kX86InstIdPextrq_ExtendedIndex = 50,
  kX86InstIdPextrw_ExtendedIndex = 140,
  kX86InstIdPf2id_ExtendedIndex = 141,
  kX86InstIdPf2iw_ExtendedIndex = 141,
  kX86InstIdPfacc_ExtendedIndex = 141,
  kX86InstIdPfadd_ExtendedIndex = 141,
  kX86InstIdPfcmpeq_ExtendedIndex = 141,
  kX86InstIdPfcmpge_ExtendedIndex = 141,
  kX86InstIdPfcmpgt_ExtendedIndex = 141,
  kX86InstIdPfmax_ExtendedIndex = 141,
  kX86InstIdPfmin_ExtendedIndex = 141,
  kX86InstIdPfmul_ExtendedIndex = 141,
  kX86InstIdPfnacc_ExtendedIndex = 141,
  kX86InstIdPfpnacc_ExtendedIndex = 141,
  kX86InstIdPfrcp_ExtendedIndex = 141,
  kX86InstIdPfrcpit1_ExtendedIndex = 141,
  kX86InstIdPfrcpit2_ExtendedIndex = 141,
  kX86InstIdPfrsqit1_ExtendedIndex = 141,
  kX86InstIdPfrsqrt_ExtendedIndex = 141,
  kX86InstIdPfsub_ExtendedIndex = 141,
  kX86InstIdPfsubr_ExtendedIndex = 141,
  kX86InstIdPhaddd_ExtendedIndex = 136,
  kX86InstIdPhaddsw_ExtendedIndex = 136,
  kX86InstIdPhaddw_ExtendedIndex = 136,
  kX86InstIdPhminposuw_ExtendedIndex = 3,
  kX86InstIdPhsubd_ExtendedIndex = 136,
  kX86InstIdPhsubsw_ExtendedIndex = 136,
  kX86InstIdPhsubw_ExtendedIndex = 136,
  kX86InstIdPi2fd_ExtendedIndex = 141,
  kX86InstIdPi2fw_ExtendedIndex = 141,
  kX86InstIdPinsrb_ExtendedIndex = 142,
  kX86InstIdPinsrd_ExtendedIndex = 142,
  kX86InstIdPinsrq_ExtendedIndex = 143,
  kX86InstIdPinsrw_ExtendedIndex = 144,
  kX86InstIdPmaddubsw_ExtendedIndex = 136,
  kX86InstIdPmaddwd_ExtendedIndex = 136,
  kX86InstIdPmaxsb_ExtendedIndex = 3,
  kX86InstIdPmaxsd_ExtendedIndex = 3,
  kX86InstIdPmaxsw_ExtendedIndex = 136,
  kX86InstIdPmaxub_ExtendedIndex = 136,
  kX86InstIdPmaxud_ExtendedIndex = 3,
  kX86InstIdPmaxuw_ExtendedIndex = 3,
  kX86InstIdPminsb_ExtendedIndex = 3,
  kX86InstIdPminsd_ExtendedIndex = 3,
  kX86InstIdPminsw_ExtendedIndex = 136,
  kX86InstIdPminub_ExtendedIndex = 136,
  kX86InstIdPminud_ExtendedIndex = 3,
  kX86InstIdPminuw_ExtendedIndex = 3,
  kX86InstIdPmovmskb_ExtendedIndex = 145,
  kX86InstIdPmovsxbd_ExtendedIndex = 36,
  kX86InstIdPmovsxbq_ExtendedIndex = 36,
  kX86InstIdPmovsxbw_ExtendedIndex = 36,
  kX86InstIdPmovsxdq_ExtendedIndex = 36,
  kX86InstIdPmovsxwd_ExtendedIndex = 36,
  kX86InstIdPmovsxwq_ExtendedIndex = 36,
  kX86InstIdPmovzxbd_ExtendedIndex = 36,
  kX86InstIdPmovzxbq_ExtendedIndex = 36,
  kX86InstIdPmovzxbw_ExtendedIndex = 36,
  kX86InstIdPmovzxdq_ExtendedIndex = 36,
  kX86InstIdPmovzxwd_ExtendedIndex = 36,
  kX86InstIdPmovzxwq_ExtendedIndex = 36,
  kX86InstIdPmuldq_ExtendedIndex = 3,
  kX86InstIdPmulhrsw_ExtendedIndex = 136,
  kX86InstIdPmulhuw_ExtendedIndex = 136,
  kX86InstIdPmulhw_ExtendedIndex = 136,
  kX86InstIdPmulld_ExtendedIndex = 3,
  kX86InstIdPmullw_ExtendedIndex = 136,
  kX86InstIdPmuludq_ExtendedIndex = 136,
  kX86InstIdPop_ExtendedIndex = 146,
  kX86InstIdPopa_ExtendedIndex = 16,
  kX86InstIdPopcnt_ExtendedIndex = 9,
  kX86InstIdPopf_ExtendedIndex = 147,
  kX86InstIdPor_ExtendedIndex = 136,
  kX86InstIdPrefetch_ExtendedIndex = 148,
  kX86InstIdPrefetch3dNow_ExtendedIndex = 19,
  kX86InstIdPrefetchw3dNow_ExtendedIndex = 19,
  kX86InstIdPsadbw_ExtendedIndex = 136,
  kX86InstIdPshufb_ExtendedIndex = 136,
  kX86InstIdPshufd_ExtendedIndex = 149,
  kX86InstIdPshufhw_ExtendedIndex = 149,
  kX86InstIdPshuflw_ExtendedIndex = 149,
  kX86InstIdPshufw_ExtendedIndex = 150,
  kX86InstIdPsignb_ExtendedIndex = 136,
  kX86InstIdPsignd_ExtendedIndex = 136,
  kX86InstIdPsignw_ExtendedIndex = 136,
  kX86InstIdPslld_ExtendedIndex = 151,
  kX86InstIdPslldq_ExtendedIndex = 152,
  kX86InstIdPsllq_ExtendedIndex = 153,
  kX86InstIdPsllw_ExtendedIndex = 154,
  kX86InstIdPsrad_ExtendedIndex = 155,
  kX86InstIdPsraw_ExtendedIndex = 156,
  kX86InstIdPsrld_ExtendedIndex = 157,
  kX86InstIdPsrldq_ExtendedIndex = 158,
  kX86InstIdPsrlq_ExtendedIndex = 159,
  kX86InstIdPsrlw_ExtendedIndex = 160,
  kX86InstIdPsubb_ExtendedIndex = 136,
  kX86InstIdPsubd_ExtendedIndex = 136,
  kX86InstIdPsubq_ExtendedIndex = 136,
  kX86InstIdPsubsb_ExtendedIndex = 136,
  kX86InstIdPsubsw_ExtendedIndex = 136,
  kX86InstIdPsubusb_ExtendedIndex = 136,
  kX86InstIdPsubusw_ExtendedIndex = 136,
  kX86InstIdPsubw_ExtendedIndex = 136,
  kX86InstIdPswapd_ExtendedIndex = 141,
  kX86InstIdPtest_ExtendedIndex = 34,
  kX86InstIdPunpckhbw_ExtendedIndex = 136,
  kX86InstIdPunpckhdq_ExtendedIndex = 136,
  kX86InstIdPunpckhqdq_ExtendedIndex = 3,
  kX86InstIdPunpckhwd_ExtendedIndex = 136,
  kX86InstIdPunpcklbw_ExtendedIndex = 136,
  kX86InstIdPunpckldq_ExtendedIndex = 136,
  kX86InstIdPunpcklqdq_ExtendedIndex = 3,
  kX86InstIdPunpcklwd_ExtendedIndex = 136,
  kX86InstIdPush_ExtendedIndex = 161,
  kX86InstIdPusha_ExtendedIndex = 16,
  kX86InstIdPushf_ExtendedIndex = 162,
  kX86InstIdPxor_ExtendedIndex = 136,
  kX86InstIdRcl_ExtendedIndex = 163,
  kX86InstIdRcpps_ExtendedIndex = 36,
  kX86InstIdRcpss_ExtendedIndex = 41,
  kX86InstIdRcr_ExtendedIndex = 163,
  kX86InstIdRdfsbase_ExtendedIndex = 164,
  kX86InstIdRdgsbase_ExtendedIndex = 164,
  kX86InstIdRdrand_ExtendedIndex = 165,
  kX86InstIdRdtsc_ExtendedIndex = 16,
  kX86InstIdRdtscp_ExtendedIndex = 16,
  kX86InstIdRepLodsB_ExtendedIndex = 166,
  kX86InstIdRepLodsD_ExtendedIndex = 166,
  kX86InstIdRepLodsQ_ExtendedIndex = 166,
  kX86InstIdRepLodsW_ExtendedIndex = 166,
  kX86InstIdRepMovsB_ExtendedIndex = 167,
  kX86InstIdRepMovsD_ExtendedIndex = 167,
  kX86InstIdRepMovsQ_ExtendedIndex = 167,
  kX86InstIdRepMovsW_ExtendedIndex = 167,
  kX86InstIdRepStosB_ExtendedIndex = 166,
  kX86InstIdRepStosD_ExtendedIndex = 166,
  kX86InstIdRepStosQ_ExtendedIndex = 166,
  kX86InstIdRepStosW_ExtendedIndex = 166,
  kX86InstIdRepeCmpsB_ExtendedIndex = 168,
  kX86InstIdRepeCmpsD_ExtendedIndex = 168,
  kX86InstIdRepeCmpsQ_ExtendedIndex = 168,
  kX86InstIdRepeCmpsW_ExtendedIndex = 168,
  kX86InstIdRepeScasB_ExtendedIndex = 168,
  kX86InstIdRepeScasD_ExtendedIndex = 168,
  kX86InstIdRepeScasQ_ExtendedIndex = 168,
  kX86InstIdRepeScasW_ExtendedIndex = 168,
  kX86InstIdRepneCmpsB_ExtendedIndex = 168,
  kX86InstIdRepneCmpsD_ExtendedIndex = 168,
  kX86InstIdRepneCmpsQ_ExtendedIndex = 168,
  kX86InstIdRepneCmpsW_ExtendedIndex = 168,
  kX86InstIdRepneScasB_ExtendedIndex = 168,
  kX86InstIdRepneScasD_ExtendedIndex = 168,
  kX86InstIdRepneScasQ_ExtendedIndex = 168,
  kX86InstIdRepneScasW_ExtendedIndex = 168,
  kX86InstIdRet_ExtendedIndex = 169,
  kX86InstIdRol_ExtendedIndex = 170,
  kX86InstIdRor_ExtendedIndex = 170,
  kX86InstIdRorx_ExtendedIndex = 171,
  kX86InstIdRoundpd_ExtendedIndex = 149,
  kX86InstIdRoundps_ExtendedIndex = 149,
  kX86InstIdRoundsd_ExtendedIndex = 172,
  kX86InstIdRoundss_ExtendedIndex = 173,
  kX86InstIdRsqrtps_ExtendedIndex = 36,
  kX86InstIdRsqrtss_ExtendedIndex = 41,
  kX86InstIdSahf_ExtendedIndex = 174,
  kX86InstIdSal_ExtendedIndex = 175,
  kX86InstIdSar_ExtendedIndex = 175,
  kX86InstIdSarx_ExtendedIndex = 176,
  kX86InstIdSbb_ExtendedIndex = 1,
  kX86InstIdScasB_ExtendedIndex = 30,
  kX86InstIdScasD_ExtendedIndex = 30,
  kX86InstIdScasQ_ExtendedIndex = 30,
  kX86InstIdScasW_ExtendedIndex = 31,
  kX86InstIdSeta_ExtendedIndex = 177,
  kX86InstIdSetae_ExtendedIndex = 178,
  kX86InstIdSetb_ExtendedIndex = 178,
  kX86InstIdSetbe_ExtendedIndex = 177,
  kX86InstIdSetc_ExtendedIndex = 178,
  kX86InstIdSete_ExtendedIndex = 179,
  kX86InstIdSetg_ExtendedIndex = 180,
  kX86InstIdSetge_ExtendedIndex = 181,
  kX86InstIdSetl_ExtendedIndex = 181,
  kX86InstIdSetle_ExtendedIndex = 180,
  kX86InstIdSetna_ExtendedIndex = 177,
  kX86InstIdSetnae_ExtendedIndex = 178,
  kX86InstIdSetnb_ExtendedIndex = 178,
  kX86InstIdSetnbe_ExtendedIndex = 177,
  kX86InstIdSetnc_ExtendedIndex = 178,
  kX86InstIdSetne_ExtendedIndex = 179,
  kX86InstIdSetng_ExtendedIndex = 180,
  kX86InstIdSetnge_ExtendedIndex = 181,
  kX86InstIdSetnl_ExtendedIndex = 181,
  kX86InstIdSetnle_ExtendedIndex = 180,
  kX86InstIdSetno_ExtendedIndex = 182,
  kX86InstIdSetnp_ExtendedIndex = 183,
  kX86InstIdSetns_ExtendedIndex = 184,
  kX86InstIdSetnz_ExtendedIndex = 179,
  kX86InstIdSeto_ExtendedIndex = 182,
  kX86InstIdSetp_ExtendedIndex = 183,
  kX86InstIdSetpe_ExtendedIndex = 183,
  kX86InstIdSetpo_ExtendedIndex = 183,
  kX86InstIdSets_ExtendedIndex = 184,
  kX86InstIdSetz_ExtendedIndex = 179,
  kX86InstIdSfence_ExtendedIndex = 91,
  kX86InstIdShl_ExtendedIndex = 175,
  kX86InstIdShld_ExtendedIndex = 185,
  kX86InstIdShlx_ExtendedIndex = 176,
  kX86InstIdShr_ExtendedIndex = 175,
  kX86InstIdShrd_ExtendedIndex = 186,
  kX86InstIdShrx_ExtendedIndex = 176,
  kX86InstIdShufpd_ExtendedIndex = 4,
  kX86InstIdShufps_ExtendedIndex = 4,
  kX86InstIdSqrtpd_ExtendedIndex = 36,
  kX86InstIdSqrtps_ExtendedIndex = 36,
  kX86InstIdSqrtsd_ExtendedIndex = 44,
  kX86InstIdSqrtss_ExtendedIndex = 41,
  kX86InstIdStc_ExtendedIndex = 17,
  kX86InstIdStd_ExtendedIndex = 18,
  kX86InstIdStmxcsr_ExtendedIndex = 19,
  kX86InstIdStosB_ExtendedIndex = 187,
  kX86InstIdStosD_ExtendedIndex = 187,
  kX86InstIdStosQ_ExtendedIndex = 187,
  kX86InstIdStosW_ExtendedIndex = 188,
  kX86InstIdSub_ExtendedIndex = 2,
  kX86InstIdSubpd_ExtendedIndex = 3,
  kX86InstIdSubps_ExtendedIndex = 3,
  kX86InstIdSubsd_ExtendedIndex = 3,
  kX86InstIdSubss_ExtendedIndex = 3,
  kX86InstIdTest_ExtendedIndex = 189,
  kX86InstIdTzcnt_ExtendedIndex = 190,
  kX86InstIdUcomisd_ExtendedIndex = 34,
  kX86InstIdUcomiss_ExtendedIndex = 34,
  kX86InstIdUd2_ExtendedIndex = 48,
  kX86InstIdUnpckhpd_ExtendedIndex = 3,
  kX86InstIdUnpckhps_ExtendedIndex = 3,
  kX86InstIdUnpcklpd_ExtendedIndex = 3,
  kX86InstIdUnpcklps_ExtendedIndex = 3,
  kX86InstIdVaddpd_ExtendedIndex = 191,
  kX86InstIdVaddps_ExtendedIndex = 191,
  kX86InstIdVaddsd_ExtendedIndex = 192,
  kX86InstIdVaddss_ExtendedIndex = 192,
  kX86InstIdVaddsubpd_ExtendedIndex = 191,
  kX86InstIdVaddsubps_ExtendedIndex = 191,
  kX86InstIdVaesdec_ExtendedIndex = 192,
  kX86InstIdVaesdeclast_ExtendedIndex = 192,
  kX86InstIdVaesenc_ExtendedIndex = 192,
  kX86InstIdVaesenclast_ExtendedIndex = 192,
  kX86InstIdVaesimc_ExtendedIndex = 193,
  kX86InstIdVaeskeygenassist_ExtendedIndex = 194,
  kX86InstIdVandnpd_ExtendedIndex = 191,
  kX86InstIdVandnps_ExtendedIndex = 191,
  kX86InstIdVandpd_ExtendedIndex = 191,
  kX86InstIdVandps_ExtendedIndex = 191,
  kX86InstIdVblendpd_ExtendedIndex = 195,
  kX86InstIdVblendps_ExtendedIndex = 195,
  kX86InstIdVblendvpd_ExtendedIndex = 196,
  kX86InstIdVblendvps_ExtendedIndex = 196,
  kX86InstIdVbroadcastf128_ExtendedIndex = 197,
  kX86InstIdVbroadcasti128_ExtendedIndex = 197,
  kX86InstIdVbroadcastsd_ExtendedIndex = 198,
  kX86InstIdVbroadcastss_ExtendedIndex = 199,
  kX86InstIdVcmppd_ExtendedIndex = 195,
  kX86InstIdVcmpps_ExtendedIndex = 195,
  kX86InstIdVcmpsd_ExtendedIndex = 200,
  kX86InstIdVcmpss_ExtendedIndex = 200,
  kX86InstIdVcomisd_ExtendedIndex = 193,
  kX86InstIdVcomiss_ExtendedIndex = 193,
  kX86InstIdVcvtdq2pd_ExtendedIndex = 199,
  kX86InstIdVcvtdq2ps_ExtendedIndex = 201,
  kX86InstIdVcvtpd2dq_ExtendedIndex = 202,
  kX86InstIdVcvtpd2ps_ExtendedIndex = 202,
  kX86InstIdVcvtph2ps_ExtendedIndex = 199,
  kX86InstIdVcvtps2dq_ExtendedIndex = 201,
  kX86InstIdVcvtps2pd_ExtendedIndex = 199,
  kX86InstIdVcvtps2ph_ExtendedIndex = 203,
  kX86InstIdVcvtsd2si_ExtendedIndex = 204,
  kX86InstIdVcvtsd2ss_ExtendedIndex = 192,
  kX86InstIdVcvtsi2sd_ExtendedIndex = 205,
  kX86InstIdVcvtsi2ss_ExtendedIndex = 205,
  kX86InstIdVcvtss2sd_ExtendedIndex = 192,
  kX86InstIdVcvtss2si_ExtendedIndex = 204,
  kX86InstIdVcvttpd2dq_ExtendedIndex = 206,
  kX86InstIdVcvttps2dq_ExtendedIndex = 201,
  kX86InstIdVcvttsd2si_ExtendedIndex = 204,
  kX86InstIdVcvttss2si_ExtendedIndex = 204,
  kX86InstIdVdivpd_ExtendedIndex = 191,
  kX86InstIdVdivps_ExtendedIndex = 191,
  kX86InstIdVdivsd_ExtendedIndex = 192,
  kX86InstIdVdivss_ExtendedIndex = 192,
  kX86InstIdVdppd_ExtendedIndex = 200,
  kX86InstIdVdpps_ExtendedIndex = 195,
  kX86InstIdVextractf128_ExtendedIndex = 207,
  kX86InstIdVextracti128_ExtendedIndex = 207,
  kX86InstIdVextractps_ExtendedIndex = 208,
  kX86InstIdVfmadd132pd_ExtendedIndex = 191,
  kX86InstIdVfmadd132ps_ExtendedIndex = 191,
  kX86InstIdVfmadd132sd_ExtendedIndex = 192,
  kX86InstIdVfmadd132ss_ExtendedIndex = 192,
  kX86InstIdVfmadd213pd_ExtendedIndex = 191,
  kX86InstIdVfmadd213ps_ExtendedIndex = 191,
  kX86InstIdVfmadd213sd_ExtendedIndex = 192,
  kX86InstIdVfmadd213ss_ExtendedIndex = 192,
  kX86InstIdVfmadd231pd_ExtendedIndex = 191,
  kX86InstIdVfmadd231ps_ExtendedIndex = 191,
  kX86InstIdVfmadd231sd_ExtendedIndex = 192,
  kX86InstIdVfmadd231ss_ExtendedIndex = 192,
  kX86InstIdVfmaddpd_ExtendedIndex = 209,
  kX86InstIdVfmaddps_ExtendedIndex = 209,
  kX86InstIdVfmaddsd_ExtendedIndex = 210,
  kX86InstIdVfmaddss_ExtendedIndex = 210,
  kX86InstIdVfmaddsub132pd_ExtendedIndex = 191,
  kX86InstIdVfmaddsub132ps_ExtendedIndex = 191,
  kX86InstIdVfmaddsub213pd_ExtendedIndex = 191,
  kX86InstIdVfmaddsub213ps_ExtendedIndex = 191,
  kX86InstIdVfmaddsub231pd_ExtendedIndex = 191,
  kX86InstIdVfmaddsub231ps_ExtendedIndex = 191,
  kX86InstIdVfmaddsubpd_ExtendedIndex = 209,
  kX86InstIdVfmaddsubps_ExtendedIndex = 209,
  kX86InstIdVfmsub132pd_ExtendedIndex = 191,
  kX86InstIdVfmsub132ps_ExtendedIndex = 191,
  kX86InstIdVfmsub132sd_ExtendedIndex = 192,
  kX86InstIdVfmsub132ss_ExtendedIndex = 192,
  kX86InstIdVfmsub213pd_ExtendedIndex = 191,
  kX86InstIdVfmsub213ps_ExtendedIndex = 191,
  kX86InstIdVfmsub213sd_ExtendedIndex = 192,
  kX86InstIdVfmsub213ss_ExtendedIndex = 192,
  kX86InstIdVfmsub231pd_ExtendedIndex = 191,
  kX86InstIdVfmsub231ps_ExtendedIndex = 191,
  kX86InstIdVfmsub231sd_ExtendedIndex = 192,
  kX86InstIdVfmsub231ss_ExtendedIndex = 192,
  kX86InstIdVfmsubadd132pd_ExtendedIndex = 191,
  kX86InstIdVfmsubadd132ps_ExtendedIndex = 191,
  kX86InstIdVfmsubadd213pd_ExtendedIndex = 191,
  kX86InstIdVfmsubadd213ps_ExtendedIndex = 191,
  kX86InstIdVfmsubadd231pd_ExtendedIndex = 191,
  kX86InstIdVfmsubadd231ps_ExtendedIndex = 191,
  kX86InstIdVfmsubaddpd_ExtendedIndex = 209,
  kX86InstIdVfmsubaddps_ExtendedIndex = 209,
  kX86InstIdVfmsubpd_ExtendedIndex = 209,
  kX86InstIdVfmsubps_ExtendedIndex = 209,
  kX86InstIdVfmsubsd_ExtendedIndex = 210,
  kX86InstIdVfmsubss_ExtendedIndex = 210,
  kX86InstIdVfnmadd132pd_ExtendedIndex = 191,
  kX86InstIdVfnmadd132ps_ExtendedIndex = 191,
  kX86InstIdVfnmadd132sd_ExtendedIndex = 192,
  kX86InstIdVfnmadd132ss_ExtendedIndex = 192,
  kX86InstIdVfnmadd213pd_ExtendedIndex = 191,
  kX86InstIdVfnmadd213ps_ExtendedIndex = 191,
  kX86InstIdVfnmadd213sd_ExtendedIndex = 192,
  kX86InstIdVfnmadd213ss_ExtendedIndex = 192,
  kX86InstIdVfnmadd231pd_ExtendedIndex = 191,
  kX86InstIdVfnmadd231ps_ExtendedIndex = 191,
  kX86InstIdVfnmadd231sd_ExtendedIndex = 192,
  kX86InstIdVfnmadd231ss_ExtendedIndex = 192,
  kX86InstIdVfnmaddpd_ExtendedIndex = 209,
  kX86InstIdVfnmaddps_ExtendedIndex = 209,
  kX86InstIdVfnmaddsd_ExtendedIndex = 210,
  kX86InstIdVfnmaddss_ExtendedIndex = 210,
  kX86InstIdVfnmsub132pd_ExtendedIndex = 191,
  kX86InstIdVfnmsub132ps_ExtendedIndex = 191,
  kX86InstIdVfnmsub132sd_ExtendedIndex = 192,
  kX86InstIdVfnmsub132ss_ExtendedIndex = 192,
  kX86InstIdVfnmsub213pd_ExtendedIndex = 191,
  kX86InstIdVfnmsub213ps_ExtendedIndex = 191,
  kX86InstIdVfnmsub213sd_ExtendedIndex = 192,
  kX86InstIdVfnmsub213ss_ExtendedIndex = 192,
  kX86InstIdVfnmsub231pd_ExtendedIndex = 191,
  kX86InstIdVfnmsub231ps_ExtendedIndex = 191,
  kX86InstIdVfnmsub231sd_ExtendedIndex = 192,
  kX86InstIdVfnmsub231ss_ExtendedIndex = 192,
  kX86InstIdVfnmsubpd_ExtendedIndex = 209,
  kX86InstIdVfnmsubps_ExtendedIndex = 209,
  kX86InstIdVfnmsubsd_ExtendedIndex = 210,
  kX86InstIdVfnmsubss_ExtendedIndex = 210,
  kX86InstIdVfrczpd_ExtendedIndex = 211,
  kX86InstIdVfrczps_ExtendedIndex = 211,
  kX86InstIdVfrczsd_ExtendedIndex = 212,
  kX86InstIdVfrczss_ExtendedIndex = 212,
  kX86InstIdVgatherdpd_ExtendedIndex = 213,
  kX86InstIdVgatherdps_ExtendedIndex = 213,
  kX86InstIdVgatherqpd_ExtendedIndex = 213,
  kX86InstIdVgatherqps_ExtendedIndex = 214,
  kX86InstIdVhaddpd_ExtendedIndex = 191,
  kX86InstIdVhaddps_ExtendedIndex = 191,
  kX86InstIdVhsubpd_ExtendedIndex = 191,
  kX86InstIdVhsubps_ExtendedIndex = 191,
  kX86InstIdVinsertf128_ExtendedIndex = 215,
  kX86InstIdVinserti128_ExtendedIndex = 215,
  kX86InstIdVinsertps_ExtendedIndex = 200,
  kX86InstIdVlddqu_ExtendedIndex = 216,
  kX86InstIdVldmxcsr_ExtendedIndex = 217,
  kX86InstIdVmaskmovdqu_ExtendedIndex = 218,
  kX86InstIdVmaskmovpd_ExtendedIndex = 219,
  kX86InstIdVmaskmovps_ExtendedIndex = 220,
  kX86InstIdVmaxpd_ExtendedIndex = 191,
  kX86InstIdVmaxps_ExtendedIndex = 191,
  kX86InstIdVmaxsd_ExtendedIndex = 191,
  kX86InstIdVmaxss_ExtendedIndex = 191,
  kX86InstIdVminpd_ExtendedIndex = 191,
  kX86InstIdVminps_ExtendedIndex = 191,
  kX86InstIdVminsd_ExtendedIndex = 191,
  kX86InstIdVminss_ExtendedIndex = 191,
  kX86InstIdVmovapd_ExtendedIndex = 221,
  kX86InstIdVmovaps_ExtendedIndex = 222,
  kX86InstIdVmovd_ExtendedIndex = 223,
  kX86InstIdVmovddup_ExtendedIndex = 201,
  kX86InstIdVmovdqa_ExtendedIndex = 224,
  kX86InstIdVmovdqu_ExtendedIndex = 225,
  kX86InstIdVmovhlps_ExtendedIndex = 226,
  kX86InstIdVmovhpd_ExtendedIndex = 227,
  kX86InstIdVmovhps_ExtendedIndex = 228,
  kX86InstIdVmovlhps_ExtendedIndex = 226,
  kX86InstIdVmovlpd_ExtendedIndex = 229,
  kX86InstIdVmovlps_ExtendedIndex = 230,
  kX86InstIdVmovmskpd_ExtendedIndex = 231,
  kX86InstIdVmovmskps_ExtendedIndex = 231,
  kX86InstIdVmovntdq_ExtendedIndex = 232,
  kX86InstIdVmovntdqa_ExtendedIndex = 216,
  kX86InstIdVmovntpd_ExtendedIndex = 232,
  kX86InstIdVmovntps_ExtendedIndex = 232,
  kX86InstIdVmovq_ExtendedIndex = 223,
  kX86InstIdVmovsd_ExtendedIndex = 233,
  kX86InstIdVmovshdup_ExtendedIndex = 201,
  kX86InstIdVmovsldup_ExtendedIndex = 201,
  kX86InstIdVmovss_ExtendedIndex = 234,
  kX86InstIdVmovupd_ExtendedIndex = 235,
  kX86InstIdVmovups_ExtendedIndex = 236,
  kX86InstIdVmpsadbw_ExtendedIndex = 195,
  kX86InstIdVmulpd_ExtendedIndex = 191,
  kX86InstIdVmulps_ExtendedIndex = 191,
  kX86InstIdVmulsd_ExtendedIndex = 191,
  kX86InstIdVmulss_ExtendedIndex = 191,
  kX86InstIdVorpd_ExtendedIndex = 191,
  kX86InstIdVorps_ExtendedIndex = 191,
  kX86InstIdVpabsb_ExtendedIndex = 201,
  kX86InstIdVpabsd_ExtendedIndex = 201,
  kX86InstIdVpabsw_ExtendedIndex = 201,
  kX86InstIdVpackssdw_ExtendedIndex = 191,
  kX86InstIdVpacksswb_ExtendedIndex = 191,
  kX86InstIdVpackusdw_ExtendedIndex = 191,
  kX86InstIdVpackuswb_ExtendedIndex = 191,
  kX86InstIdVpaddb_ExtendedIndex = 191,
  kX86InstIdVpaddd_ExtendedIndex = 191,
  kX86InstIdVpaddq_ExtendedIndex = 191,
  kX86InstIdVpaddsb_ExtendedIndex = 191,
  kX86InstIdVpaddsw_ExtendedIndex = 191,
  kX86InstIdVpaddusb_ExtendedIndex = 191,
  kX86InstIdVpaddusw_ExtendedIndex = 191,
  kX86InstIdVpaddw_ExtendedIndex = 191,
  kX86InstIdVpalignr_ExtendedIndex = 195,
  kX86InstIdVpand_ExtendedIndex = 191,
  kX86InstIdVpandn_ExtendedIndex = 191,
  kX86InstIdVpavgb_ExtendedIndex = 191,
  kX86InstIdVpavgw_ExtendedIndex = 191,
  kX86InstIdVpblendd_ExtendedIndex = 195,
  kX86InstIdVpblendvb_ExtendedIndex = 237,
  kX86InstIdVpblendw_ExtendedIndex = 195,
  kX86InstIdVpbroadcastb_ExtendedIndex = 199,
  kX86InstIdVpbroadcastd_ExtendedIndex = 199,
  kX86InstIdVpbroadcastq_ExtendedIndex = 199,
  kX86InstIdVpbroadcastw_ExtendedIndex = 199,
  kX86InstIdVpclmulqdq_ExtendedIndex = 200,
  kX86InstIdVpcmov_ExtendedIndex = 238,
  kX86InstIdVpcmpeqb_ExtendedIndex = 191,
  kX86InstIdVpcmpeqd_ExtendedIndex = 191,
  kX86InstIdVpcmpeqq_ExtendedIndex = 191,
  kX86InstIdVpcmpeqw_ExtendedIndex = 191,
  kX86InstIdVpcmpestri_ExtendedIndex = 194,
  kX86InstIdVpcmpestrm_ExtendedIndex = 194,
  kX86InstIdVpcmpgtb_ExtendedIndex = 191,
  kX86InstIdVpcmpgtd_ExtendedIndex = 191,
  kX86InstIdVpcmpgtq_ExtendedIndex = 191,
  kX86InstIdVpcmpgtw_ExtendedIndex = 191,
  kX86InstIdVpcmpistri_ExtendedIndex = 194,
  kX86InstIdVpcmpistrm_ExtendedIndex = 194,
  kX86InstIdVpcomb_ExtendedIndex = 239,
  kX86InstIdVpcomd_ExtendedIndex = 239,
  kX86InstIdVpcomq_ExtendedIndex = 239,
  kX86InstIdVpcomub_ExtendedIndex = 239,
  kX86InstIdVpcomud_ExtendedIndex = 239,
  kX86InstIdVpcomuq_ExtendedIndex = 239,
  kX86InstIdVpcomuw_ExtendedIndex = 239,
  kX86InstIdVpcomw_ExtendedIndex = 239,
  kX86InstIdVperm2f128_ExtendedIndex = 240,
  kX86InstIdVperm2i128_ExtendedIndex = 240,
  kX86InstIdVpermd_ExtendedIndex = 241,
  kX86InstIdVpermil2pd_ExtendedIndex = 242,
  kX86InstIdVpermil2ps_ExtendedIndex = 242,
  kX86InstIdVpermilpd_ExtendedIndex = 243,
  kX86InstIdVpermilps_ExtendedIndex = 244,
  kX86InstIdVpermpd_ExtendedIndex = 245,
  kX86InstIdVpermps_ExtendedIndex = 241,
  kX86InstIdVpermq_ExtendedIndex = 245,
  kX86InstIdVpextrb_ExtendedIndex = 246,
  kX86InstIdVpextrd_ExtendedIndex = 208,
  kX86InstIdVpextrq_ExtendedIndex = 247,
  kX86InstIdVpextrw_ExtendedIndex = 248,
  kX86InstIdVpgatherdd_ExtendedIndex = 213,
  kX86InstIdVpgatherdq_ExtendedIndex = 213,
  kX86InstIdVpgatherqd_ExtendedIndex = 214,
  kX86InstIdVpgatherqq_ExtendedIndex = 213,
  kX86InstIdVphaddbd_ExtendedIndex = 212,
  kX86InstIdVphaddbq_ExtendedIndex = 212,
  kX86InstIdVphaddbw_ExtendedIndex = 212,
  kX86InstIdVphaddd_ExtendedIndex = 191,
  kX86InstIdVphadddq_ExtendedIndex = 212,
  kX86InstIdVphaddsw_ExtendedIndex = 191,
  kX86InstIdVphaddubd_ExtendedIndex = 212,
  kX86InstIdVphaddubq_ExtendedIndex = 212,
  kX86InstIdVphaddubw_ExtendedIndex = 212,
  kX86InstIdVphaddudq_ExtendedIndex = 212,
  kX86InstIdVphadduwd_ExtendedIndex = 212,
  kX86InstIdVphadduwq_ExtendedIndex = 212,
  kX86InstIdVphaddw_ExtendedIndex = 191,
  kX86InstIdVphaddwd_ExtendedIndex = 212,
  kX86InstIdVphaddwq_ExtendedIndex = 212,
  kX86InstIdVphminposuw_ExtendedIndex = 193,
  kX86InstIdVphsubbw_ExtendedIndex = 212,
  kX86InstIdVphsubd_ExtendedIndex = 191,
  kX86InstIdVphsubdq_ExtendedIndex = 212,
  kX86InstIdVphsubsw_ExtendedIndex = 191,
  kX86InstIdVphsubw_ExtendedIndex = 191,
  kX86InstIdVphsubwd_ExtendedIndex = 212,
  kX86InstIdVpinsrb_ExtendedIndex = 249,
  kX86InstIdVpinsrd_ExtendedIndex = 250,
  kX86InstIdVpinsrq_ExtendedIndex = 251,
  kX86InstIdVpinsrw_ExtendedIndex = 252,
  kX86InstIdVpmacsdd_ExtendedIndex = 253,
  kX86InstIdVpmacsdqh_ExtendedIndex = 253,
  kX86InstIdVpmacsdql_ExtendedIndex = 253,
  kX86InstIdVpmacssdd_ExtendedIndex = 253,
  kX86InstIdVpmacssdqh_ExtendedIndex = 253,
  kX86InstIdVpmacssdql_ExtendedIndex = 253,
  kX86InstIdVpmacsswd_ExtendedIndex = 253,
  kX86InstIdVpmacssww_ExtendedIndex = 253,
  kX86InstIdVpmacswd_ExtendedIndex = 253,
  kX86InstIdVpmacsww_ExtendedIndex = 253,
  kX86InstIdVpmadcsswd_ExtendedIndex = 253,
  kX86InstIdVpmadcswd_ExtendedIndex = 253,
  kX86InstIdVpmaddubsw_ExtendedIndex = 191,
  kX86InstIdVpmaddwd_ExtendedIndex = 191,
  kX86InstIdVpmaskmovd_ExtendedIndex = 254,
  kX86InstIdVpmaskmovq_ExtendedIndex = 254,
  kX86InstIdVpmaxsb_ExtendedIndex = 191,
  kX86InstIdVpmaxsd_ExtendedIndex = 191,
  kX86InstIdVpmaxsw_ExtendedIndex = 191,
  kX86InstIdVpmaxub_ExtendedIndex = 191,
  kX86InstIdVpmaxud_ExtendedIndex = 191,
  kX86InstIdVpmaxuw_ExtendedIndex = 191,
  kX86InstIdVpminsb_ExtendedIndex = 191,
  kX86InstIdVpminsd_ExtendedIndex = 191,
  kX86InstIdVpminsw_ExtendedIndex = 191,
  kX86InstIdVpminub_ExtendedIndex = 191,
  kX86InstIdVpminud_ExtendedIndex = 191,
  kX86InstIdVpminuw_ExtendedIndex = 191,
  kX86InstIdVpmovmskb_ExtendedIndex = 231,
  kX86InstIdVpmovsxbd_ExtendedIndex = 201,
  kX86InstIdVpmovsxbq_ExtendedIndex = 201,
  kX86InstIdVpmovsxbw_ExtendedIndex = 201,
  kX86InstIdVpmovsxdq_ExtendedIndex = 201,
  kX86InstIdVpmovsxwd_ExtendedIndex = 201,
  kX86InstIdVpmovsxwq_ExtendedIndex = 201,
  kX86InstIdVpmovzxbd_ExtendedIndex = 201,
  kX86InstIdVpmovzxbq_ExtendedIndex = 201,
  kX86InstIdVpmovzxbw_ExtendedIndex = 201,
  kX86InstIdVpmovzxdq_ExtendedIndex = 201,
  kX86InstIdVpmovzxwd_ExtendedIndex = 201,
  kX86InstIdVpmovzxwq_ExtendedIndex = 201,
  kX86InstIdVpmuldq_ExtendedIndex = 191,
  kX86InstIdVpmulhrsw_ExtendedIndex = 191,
  kX86InstIdVpmulhuw_ExtendedIndex = 191,
  kX86InstIdVpmulhw_ExtendedIndex = 191,
  kX86InstIdVpmulld_ExtendedIndex = 191,
  kX86InstIdVpmullw_ExtendedIndex = 191,
  kX86InstIdVpmuludq_ExtendedIndex = 191,
  kX86InstIdVpor_ExtendedIndex = 191,
  kX86InstIdVpperm_ExtendedIndex = 255,
  kX86InstIdVprotb_ExtendedIndex = 256,
  kX86InstIdVprotd_ExtendedIndex = 257,
  kX86InstIdVprotq_ExtendedIndex = 258,
  kX86InstIdVprotw_ExtendedIndex = 259,
  kX86InstIdVpsadbw_ExtendedIndex = 191,
  kX86InstIdVpshab_ExtendedIndex = 260,
  kX86InstIdVpshad_ExtendedIndex = 260,
  kX86InstIdVpshaq_ExtendedIndex = 260,
  kX86InstIdVpshaw_ExtendedIndex = 260,
  kX86InstIdVpshlb_ExtendedIndex = 260,
  kX86InstIdVpshld_ExtendedIndex = 260,
  kX86InstIdVpshlq_ExtendedIndex = 260,
  kX86InstIdVpshlw_ExtendedIndex = 260,
  kX86InstIdVpshufb_ExtendedIndex = 191,
  kX86InstIdVpshufd_ExtendedIndex = 261,
  kX86InstIdVpshufhw_ExtendedIndex = 261,
  kX86InstIdVpshuflw_ExtendedIndex = 261,
  kX86InstIdVpsignb_ExtendedIndex = 191,
  kX86InstIdVpsignd_ExtendedIndex = 191,
  kX86InstIdVpsignw_ExtendedIndex = 191,
  kX86InstIdVpslld_ExtendedIndex = 262,
  kX86InstIdVpslldq_ExtendedIndex = 263,
  kX86InstIdVpsllq_ExtendedIndex = 264,
  kX86InstIdVpsllvd_ExtendedIndex = 191,
  kX86InstIdVpsllvq_ExtendedIndex = 191,
  kX86InstIdVpsllw_ExtendedIndex = 265,
  kX86InstIdVpsrad_ExtendedIndex = 266,
  kX86InstIdVpsravd_ExtendedIndex = 191,
  kX86InstIdVpsraw_ExtendedIndex = 267,
  kX86InstIdVpsrld_ExtendedIndex = 268,
  kX86InstIdVpsrldq_ExtendedIndex = 263,
  kX86InstIdVpsrlq_ExtendedIndex = 269,
  kX86InstIdVpsrlvd_ExtendedIndex = 191,
  kX86InstIdVpsrlvq_ExtendedIndex = 191,
  kX86InstIdVpsrlw_ExtendedIndex = 270,
  kX86InstIdVpsubb_ExtendedIndex = 191,
  kX86InstIdVpsubd_ExtendedIndex = 191,
  kX86InstIdVpsubq_ExtendedIndex = 191,
  kX86InstIdVpsubsb_ExtendedIndex = 191,
  kX86InstIdVpsubsw_ExtendedIndex = 191,
  kX86InstIdVpsubusb_ExtendedIndex = 191,
  kX86InstIdVpsubusw_ExtendedIndex = 191,
  kX86InstIdVpsubw_ExtendedIndex = 191,
  kX86InstIdVptest_ExtendedIndex = 271,
  kX86InstIdVpunpckhbw_ExtendedIndex = 191,
  kX86InstIdVpunpckhdq_ExtendedIndex = 191,
  kX86InstIdVpunpckhqdq_ExtendedIndex = 191,
  kX86InstIdVpunpckhwd_ExtendedIndex = 191,
  kX86InstIdVpunpcklbw_ExtendedIndex = 191,
  kX86InstIdVpunpckldq_ExtendedIndex = 191,
  kX86InstIdVpunpcklqdq_ExtendedIndex = 191,
  kX86InstIdVpunpcklwd_ExtendedIndex = 191,
  kX86InstIdVpxor_ExtendedIndex = 191,
  kX86InstIdVrcpps_ExtendedIndex = 201,
  kX86InstIdVrcpss_ExtendedIndex = 192,
  kX86InstIdVroundpd_ExtendedIndex = 261,
  kX86InstIdVroundps_ExtendedIndex = 261,
  kX86InstIdVroundsd_ExtendedIndex = 200,
  kX86InstIdVroundss_ExtendedIndex = 200,
  kX86InstIdVrsqrtps_ExtendedIndex = 201,
  kX86InstIdVrsqrtss_ExtendedIndex = 192,
  kX86InstIdVshufpd_ExtendedIndex = 195,
  kX86InstIdVshufps_ExtendedIndex = 195,
  kX86InstIdVsqrtpd_ExtendedIndex = 201,
  kX86InstIdVsqrtps_ExtendedIndex = 201,
  kX86InstIdVsqrtsd_ExtendedIndex = 192,
  kX86InstIdVsqrtss_ExtendedIndex = 192,
  kX86InstIdVstmxcsr_ExtendedIndex = 217,
  kX86InstIdVsubpd_ExtendedIndex = 191,
  kX86InstIdVsubps_ExtendedIndex = 191,
  kX86InstIdVsubsd_ExtendedIndex = 192,
  kX86InstIdVsubss_ExtendedIndex = 192,
  kX86InstIdVtestpd_ExtendedIndex = 272,
  kX86InstIdVtestps_ExtendedIndex = 272,
  kX86InstIdVucomisd_ExtendedIndex = 273,
  kX86InstIdVucomiss_ExtendedIndex = 273,
  kX86InstIdVunpckhpd_ExtendedIndex = 191,
  kX86InstIdVunpckhps_ExtendedIndex = 191,
  kX86InstIdVunpcklpd_ExtendedIndex = 191,
  kX86InstIdVunpcklps_ExtendedIndex = 191,
  kX86InstIdVxorpd_ExtendedIndex = 191,
  kX86InstIdVxorps_ExtendedIndex = 191,
  kX86InstIdVzeroall_ExtendedIndex = 274,
  kX86InstIdVzeroupper_ExtendedIndex = 274,
  kX86InstIdWrfsbase_ExtendedIndex = 275,
  kX86InstIdWrgsbase_ExtendedIndex = 275,
  kX86InstIdXadd_ExtendedIndex = 276,
  kX86InstIdXchg_ExtendedIndex = 277,
  kX86InstIdXgetbv_ExtendedIndex = 16,
  kX86InstIdXor_ExtendedIndex = 2,
  kX86InstIdXorpd_ExtendedIndex = 3,
  kX86InstIdXorps_ExtendedIndex = 3,
  kX86InstIdXrstor_ExtendedIndex = 278,
  kX86InstIdXrstor64_ExtendedIndex = 278,
  kX86InstIdXsave_ExtendedIndex = 278,
  kX86InstIdXsave64_ExtendedIndex = 278,
  kX86InstIdXsaveopt_ExtendedIndex = 278,
  kX86InstIdXsaveopt64_ExtendedIndex = 278,
  kX86InstIdXsetbv_ExtendedIndex = 16
};
// ${X86InstData:End}

// Please run tools/src-gendefs.js (by using just node.js, without any dependencies) to regenerate the code enclosed with ${X86InstData...}.
const X86InstInfo _x86InstInfo[] = {
  // <----------------------------+--------------------+-------------------------------------------+-------------------+------------------------------------+-------------+-------+---------------------------------------------------------------------------------------------------+
  //                              |                    |           Instruction Opcodes             |                   |         Instruction Flags          |   E-FLAGS   | Write |              Operands (Gp/Fp/Mm/K/Xmm/Ymm/Zmm Regs, Mem, Imm, Label, None/Undefined)              |
  //        Instruction Id        |  Instruction Name  +---------------------+---------------------+  Instruction Enc. +---------------+--------------------+-------------+---+---+-------------------+-------------------+-------------------+-------------------+-------------------+
  //                              |                    | O-PP-MMM OP/O L/W/EW| 1:PP-MMM OP/O L/W/EW|                   | Global Flags  |A512(ID|VL|kz|rnd|b)| EF:OSZAPCDX |Idx| Sz|  [0] 1st Operand  |  [1] 2nd Operand  |  [2] 3rd Operand  |  [3] 4th Operand  |  [4] 5th Operand  |
  // <----------------------------+--------------------+---------------------+---------------------+-------------------+---------------+--------------------+-------------+---+---+-------------------+-------------------+-------------------+-------------------+-------------------+
  INST(kInstIdNone                , ""                 , U                   , U                   , Enc(None)         , F(None)                            , EF(________), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdAdc              , "adc"              , O_000000(10,2,_,_,_), U                   , Enc(X86Arith)     , F(Lock)                            , EF(WWWWWX__), 0 , 0 , O(GqdwbMem)       , O(GqdwbMem)|O(Imm), U                 , U                 , U                 ),
  INST(kX86InstIdAdd              , "add"              , O_000000(00,0,_,_,_), U                   , Enc(X86Arith)     , F(Lock)                            , EF(WWWWWW__), 0 , 0 , O(GqdwbMem)       , O(GqdwbMem)|O(Imm), U                 , U                 , U                 ),
  INST(kX86InstIdAddpd            , "addpd"            , O_660F00(58,U,_,_,_), U                   , Enc(ExtRm)        , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdAddps            , "addps"            , O_000F00(58,U,_,_,_), U                   , Enc(ExtRm)        , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdAddsd            , "addsd"            , O_F20F00(58,U,_,_,_), U                   , Enc(ExtRm)        , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdAddss            , "addss"            , O_F30F00(58,U,_,_,_), U                   , Enc(ExtRm)        , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdAddsubpd         , "addsubpd"         , O_660F00(D0,U,_,_,_), U                   , Enc(ExtRm)        , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdAddsubps         , "addsubps"         , O_F20F00(D0,U,_,_,_), U                   , Enc(ExtRm)        , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdAesdec           , "aesdec"           , O_660F38(DE,U,_,_,_), U                   , Enc(ExtRm)        , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdAesdeclast       , "aesdeclast"       , O_660F38(DF,U,_,_,_), U                   , Enc(ExtRm)        , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdAesenc           , "aesenc"           , O_660F38(DC,U,_,_,_), U                   , Enc(ExtRm)        , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdAesenclast       , "aesenclast"       , O_660F38(DD,U,_,_,_), U                   , Enc(ExtRm)        , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdAesimc           , "aesimc"           , O_660F38(DB,U,_,_,_), U                   , Enc(ExtRm)        , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdAeskeygenassist  , "aeskeygenassist"  , O_660F3A(DF,U,_,_,_), U                   , Enc(ExtRmi)       , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , O(Imm)            , U                 , U                 ),
  INST(kX86InstIdAnd              , "and"              , O_000000(20,4,_,_,_), U                   , Enc(X86Arith)     , F(Lock)                            , EF(WWWUWW__), 0 , 0 , O(GqdwbMem)       , O(GqdwbMem)|O(Imm), U                 , U                 , U                 ),
  INST(kX86InstIdAndn             , "andn"             , O_000F38(F2,U,_,_,_), U                   , Enc(AvxRvm)       , F(None)                            , EF(WWWUUW__), 0 , 0 , O(Gqd)            , O(Gqd)            , O(GqdMem)         , U                 , U                 ),
  INST(kX86InstIdAndnpd           , "andnpd"           , O_660F00(55,U,_,_,_), U                   , Enc(ExtRm)        , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdAndnps           , "andnps"           , O_000F00(55,U,_,_,_), U                   , Enc(ExtRm)        , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdAndpd            , "andpd"            , O_660F00(54,U,_,_,_), U                   , Enc(ExtRm)        , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdAndps            , "andps"            , O_000F00(54,U,_,_,_), U                   , Enc(ExtRm)        , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdBextr            , "bextr"            , O_000F38(F7,U,_,_,_), U                   , Enc(AvxRmv)       , F(None)                            , EF(WUWUUW__), 0 , 0 , O(Gqd)            , O(GqdMem)         , O(Gqd)            , U                 , U                 ),
  INST(kX86InstIdBlendpd          , "blendpd"          , O_660F3A(0D,U,_,_,_), U                   , Enc(ExtRmi)       , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , O(Imm)            , U                 , U                 ),
  INST(kX86InstIdBlendps          , "blendps"          , O_660F3A(0C,U,_,_,_), U                   , Enc(ExtRmi)       , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , O(Imm)            , U                 , U                 ),
  INST(kX86InstIdBlendvpd         , "blendvpd"         , O_660F38(15,U,_,_,_), U                   , Enc(ExtRm)        , F(None)|F(Special)                 , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdBlendvps         , "blendvps"         , O_660F38(14,U,_,_,_), U                   , Enc(ExtRm)        , F(None)|F(Special)                 , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdBlsi             , "blsi"             , O_000F38(F3,3,_,_,_), U                   , Enc(AvxVm)        , F(None)                            , EF(WWWUUW__), 0 , 0 , O(Gqd)            , O(GqdMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdBlsmsk           , "blsmsk"           , O_000F38(F3,2,_,_,_), U                   , Enc(AvxVm)        , F(None)                            , EF(WWWUUW__), 0 , 0 , O(Gqd)            , O(GqdMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdBlsr             , "blsr"             , O_000F38(F3,1,_,_,_), U                   , Enc(AvxVm)        , F(None)                            , EF(WWWUUW__), 0 , 0 , O(Gqd)            , O(GqdMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdBsf              , "bsf"              , O_000F00(BC,U,_,_,_), U                   , Enc(X86RegRm)     , F(None)                            , EF(UUWUUU__), 0 , 0 , O(Gqdw)           , O(GqdwMem)        , U                 , U                 , U                 ),
  INST(kX86InstIdBsr              , "bsr"              , O_000F00(BD,U,_,_,_), U                   , Enc(X86RegRm)     , F(None)                            , EF(UUWUUU__), 0 , 0 , O(Gqdw)           , O(GqdwMem)        , U                 , U                 , U                 ),
  INST(kX86InstIdBswap            , "bswap"            , O_000F00(C8,U,_,_,_), U                   , Enc(X86BSwap)     , F(None)                            , EF(________), 0 , 0 , O(Gqd)            , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdBt               , "bt"               , O_000F00(A3,U,_,_,_), O_000F00(BA,4,_,_,_), Enc(X86BTest)     , F(Test)                            , EF(UU_UUW__), 0 , 0 , O(GqdwMem)        , O(Gqdw)|O(Imm)    , U                 , U                 , U                 ),
  INST(kX86InstIdBtc              , "btc"              , O_000F00(BB,U,_,_,_), O_000F00(BA,7,_,_,_), Enc(X86BTest)     , F(Lock)                            , EF(UU_UUW__), 0 , 0 , O(GqdwMem)        , O(Gqdw)|O(Imm)    , U                 , U                 , U                 ),
  INST(kX86InstIdBtr              , "btr"              , O_000F00(B3,U,_,_,_), O_000F00(BA,6,_,_,_), Enc(X86BTest)     , F(Lock)                            , EF(UU_UUW__), 0 , 0 , O(GqdwMem)        , O(Gqdw)|O(Imm)    , U                 , U                 , U                 ),
  INST(kX86InstIdBts              , "bts"              , O_000F00(AB,U,_,_,_), O_000F00(BA,5,_,_,_), Enc(X86BTest)     , F(Lock)                            , EF(UU_UUW__), 0 , 0 , O(GqdwMem)        , O(Gqdw)|O(Imm)    , U                 , U                 , U                 ),
  INST(kX86InstIdBzhi             , "bzhi"             , O_000F38(F5,U,_,_,_), U                   , Enc(AvxRmv)       , F(None)                            , EF(WWWUUW__), 0 , 0 , O(Gqd)            , O(GqdMem)         , O(Gqd)            , U                 , U                 ),
  INST(kX86InstIdCall             , "call"             , O_000000(FF,2,_,_,_), O_000000(E8,U,_,_,_), Enc(X86Call)      , F(Flow)                            , EF(________), 0 , 0 , O(GqdMem)|O(LImm) , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdCbw              , "cbw"              , O_660000(98,U,_,_,_), U                   , Enc(X86Op)        , F(None)|F(Special)                 , EF(________), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdCdq              , "cdq"              , O_000000(99,U,_,_,_), U                   , Enc(X86Op)        , F(None)|F(Special)                 , EF(________), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdCdqe             , "cdqe"             , O_000000(98,U,_,W,_), U                   , Enc(X86Op)        , F(None)|F(Special)                 , EF(________), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdClc              , "clc"              , O_000000(F8,U,_,_,_), U                   , Enc(X86Op)        , F(None)                            , EF(_____W__), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdCld              , "cld"              , O_000000(FC,U,_,_,_), U                   , Enc(X86Op)        , F(None)                            , EF(______W_), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdClflush          , "clflush"          , O_000F00(AE,7,_,_,_), U                   , Enc(X86M)         , F(None)                            , EF(________), 0 , 0 , O(Mem)            , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdCmc              , "cmc"              , O_000000(F5,U,_,_,_), U                   , Enc(X86Op)        , F(None)                            , EF(_____X__), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdCmova            , "cmova"            , O_000F00(47,U,_,_,_), U                   , Enc(X86RegRm)     , F(None)                            , EF(__R__R__), 0 , 0 , O(Gqdw)           , O(GqdwMem)        , U                 , U                 , U                 ),
  INST(kX86InstIdCmovae           , "cmovae"           , O_000F00(43,U,_,_,_), U                   , Enc(X86RegRm)     , F(None)                            , EF(_____R__), 0 , 0 , O(Gqdw)           , O(GqdwMem)        , U                 , U                 , U                 ),
  INST(kX86InstIdCmovb            , "cmovb"            , O_000F00(42,U,_,_,_), U                   , Enc(X86RegRm)     , F(None)                            , EF(_____R__), 0 , 0 , O(Gqdw)           , O(GqdwMem)        , U                 , U                 , U                 ),
  INST(kX86InstIdCmovbe           , "cmovbe"           , O_000F00(46,U,_,_,_), U                   , Enc(X86RegRm)     , F(None)                            , EF(__R__R__), 0 , 0 , O(Gqdw)           , O(GqdwMem)        , U                 , U                 , U                 ),
  INST(kX86InstIdCmovc            , "cmovc"            , O_000F00(42,U,_,_,_), U                   , Enc(X86RegRm)     , F(None)                            , EF(_____R__), 0 , 0 , O(Gqdw)           , O(GqdwMem)        , U                 , U                 , U                 ),
  INST(kX86InstIdCmove            , "cmove"            , O_000F00(44,U,_,_,_), U                   , Enc(X86RegRm)     , F(None)                            , EF(__R_____), 0 , 0 , O(Gqdw)           , O(GqdwMem)        , U                 , U                 , U                 ),
  INST(kX86InstIdCmovg            , "cmovg"            , O_000F00(4F,U,_,_,_), U                   , Enc(X86RegRm)     , F(None)                            , EF(RRR_____), 0 , 0 , O(Gqdw)           , O(GqdwMem)        , U                 , U                 , U                 ),
  INST(kX86InstIdCmovge           , "cmovge"           , O_000F00(4D,U,_,_,_), U                   , Enc(X86RegRm)     , F(None)                            , EF(RR______), 0 , 0 , O(Gqdw)           , O(GqdwMem)        , U                 , U                 , U                 ),
  INST(kX86InstIdCmovl            , "cmovl"            , O_000F00(4C,U,_,_,_), U                   , Enc(X86RegRm)     , F(None)                            , EF(RR______), 0 , 0 , O(Gqdw)           , O(GqdwMem)        , U                 , U                 , U                 ),
  INST(kX86InstIdCmovle           , "cmovle"           , O_000F00(4E,U,_,_,_), U                   , Enc(X86RegRm)     , F(None)                            , EF(RRR_____), 0 , 0 , O(Gqdw)           , O(GqdwMem)        , U                 , U                 , U                 ),
  INST(kX86InstIdCmovna           , "cmovna"           , O_000F00(46,U,_,_,_), U                   , Enc(X86RegRm)     , F(None)                            , EF(__R__R__), 0 , 0 , O(Gqdw)           , O(GqdwMem)        , U                 , U                 , U                 ),
  INST(kX86InstIdCmovnae          , "cmovnae"          , O_000F00(42,U,_,_,_), U                   , Enc(X86RegRm)     , F(None)                            , EF(_____R__), 0 , 0 , O(Gqdw)           , O(GqdwMem)        , U                 , U                 , U                 ),
  INST(kX86InstIdCmovnb           , "cmovnb"           , O_000F00(43,U,_,_,_), U                   , Enc(X86RegRm)     , F(None)                            , EF(_____R__), 0 , 0 , O(Gqdw)           , O(GqdwMem)        , U                 , U                 , U                 ),
  INST(kX86InstIdCmovnbe          , "cmovnbe"          , O_000F00(47,U,_,_,_), U                   , Enc(X86RegRm)     , F(None)                            , EF(__R__R__), 0 , 0 , O(Gqdw)           , O(GqdwMem)        , U                 , U                 , U                 ),
  INST(kX86InstIdCmovnc           , "cmovnc"           , O_000F00(43,U,_,_,_), U                   , Enc(X86RegRm)     , F(None)                            , EF(_____R__), 0 , 0 , O(Gqdw)           , O(GqdwMem)        , U                 , U                 , U                 ),
  INST(kX86InstIdCmovne           , "cmovne"           , O_000F00(45,U,_,_,_), U                   , Enc(X86RegRm)     , F(None)                            , EF(__R_____), 0 , 0 , O(Gqdw)           , O(GqdwMem)        , U                 , U                 , U                 ),
  INST(kX86InstIdCmovng           , "cmovng"           , O_000F00(4E,U,_,_,_), U                   , Enc(X86RegRm)     , F(None)                            , EF(RRR_____), 0 , 0 , O(Gqdw)           , O(GqdwMem)        , U                 , U                 , U                 ),
  INST(kX86InstIdCmovnge          , "cmovnge"          , O_000F00(4C,U,_,_,_), U                   , Enc(X86RegRm)     , F(None)                            , EF(RR______), 0 , 0 , O(Gqdw)           , O(GqdwMem)        , U                 , U                 , U                 ),
  INST(kX86InstIdCmovnl           , "cmovnl"           , O_000F00(4D,U,_,_,_), U                   , Enc(X86RegRm)     , F(None)                            , EF(RR______), 0 , 0 , O(Gqdw)           , O(GqdwMem)        , U                 , U                 , U                 ),
  INST(kX86InstIdCmovnle          , "cmovnle"          , O_000F00(4F,U,_,_,_), U                   , Enc(X86RegRm)     , F(None)                            , EF(RRR_____), 0 , 0 , O(Gqdw)           , O(GqdwMem)        , U                 , U                 , U                 ),
  INST(kX86InstIdCmovno           , "cmovno"           , O_000F00(41,U,_,_,_), U                   , Enc(X86RegRm)     , F(None)                            , EF(R_______), 0 , 0 , O(Gqdw)           , O(GqdwMem)        , U                 , U                 , U                 ),
  INST(kX86InstIdCmovnp           , "cmovnp"           , O_000F00(4B,U,_,_,_), U                   , Enc(X86RegRm)     , F(None)                            , EF(____R___), 0 , 0 , O(Gqdw)           , O(GqdwMem)        , U                 , U                 , U                 ),
  INST(kX86InstIdCmovns           , "cmovns"           , O_000F00(49,U,_,_,_), U                   , Enc(X86RegRm)     , F(None)                            , EF(_R______), 0 , 0 , O(Gqdw)           , O(GqdwMem)        , U                 , U                 , U                 ),
  INST(kX86InstIdCmovnz           , "cmovnz"           , O_000F00(45,U,_,_,_), U                   , Enc(X86RegRm)     , F(None)                            , EF(__R_____), 0 , 0 , O(Gqdw)           , O(GqdwMem)        , U                 , U                 , U                 ),
  INST(kX86InstIdCmovo            , "cmovo"            , O_000F00(40,U,_,_,_), U                   , Enc(X86RegRm)     , F(None)                            , EF(R_______), 0 , 0 , O(Gqdw)           , O(GqdwMem)        , U                 , U                 , U                 ),
  INST(kX86InstIdCmovp            , "cmovp"            , O_000F00(4A,U,_,_,_), U                   , Enc(X86RegRm)     , F(None)                            , EF(____R___), 0 , 0 , O(Gqdw)           , O(GqdwMem)        , U                 , U                 , U                 ),
  INST(kX86InstIdCmovpe           , "cmovpe"           , O_000F00(4A,U,_,_,_), U                   , Enc(X86RegRm)     , F(None)                            , EF(____R___), 0 , 0 , O(Gqdw)           , O(GqdwMem)        , U                 , U                 , U                 ),
  INST(kX86InstIdCmovpo           , "cmovpo"           , O_000F00(4B,U,_,_,_), U                   , Enc(X86RegRm)     , F(None)                            , EF(____R___), 0 , 0 , O(Gqdw)           , O(GqdwMem)        , U                 , U                 , U                 ),
  INST(kX86InstIdCmovs            , "cmovs"            , O_000F00(48,U,_,_,_), U                   , Enc(X86RegRm)     , F(None)                            , EF(_R______), 0 , 0 , O(Gqdw)           , O(GqdwMem)        , U                 , U                 , U                 ),
  INST(kX86InstIdCmovz            , "cmovz"            , O_000F00(44,U,_,_,_), U                   , Enc(X86RegRm)     , F(None)                            , EF(__R_____), 0 , 0 , O(Gqdw)           , O(GqdwMem)        , U                 , U                 , U                 ),
  INST(kX86InstIdCmp              , "cmp"              , O_000000(38,7,_,_,_), U                   , Enc(X86Arith)     , F(Test)                            , EF(WWWWWW__), 0 , 0 , O(GqdwbMem)       , O(GqdwbMem)|O(Imm), U                 , U                 , U                 ),
  INST(kX86InstIdCmppd            , "cmppd"            , O_660F00(C2,U,_,_,_), U                   , Enc(ExtRmi)       , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , O(Imm)            , U                 , U                 ),
  INST(kX86InstIdCmpps            , "cmpps"            , O_000F00(C2,U,_,_,_), U                   , Enc(ExtRmi)       , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , O(Imm)            , U                 , U                 ),
  INST(kX86InstIdCmpsB            , "cmps_b"           , O_000000(A6,U,_,_,_), U                   , Enc(X86Op)        , F(None)|F(Special)                 , EF(WWWWWWR_), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdCmpsD            , "cmps_d"           , O_000000(A7,U,_,_,_), U                   , Enc(X86Op)        , F(None)|F(Special)                 , EF(WWWWWWR_), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdCmpsQ            , "cmps_q"           , O_000000(A7,U,_,W,_), U                   , Enc(X86Op)        , F(None)|F(Special)                 , EF(WWWWWWR_), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdCmpsW            , "cmps_w"           , O_000000(A7,U,_,_,_), U                   , Enc(X86Op_66H)    , F(None)|F(Special)                 , EF(WWWWWWR_), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdCmpsd            , "cmpsd"            , O_F20F00(C2,U,_,_,_), U                   , Enc(ExtRmi)       , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , O(Imm)            , U                 , U                 ),
  INST(kX86InstIdCmpss            , "cmpss"            , O_F30F00(C2,U,_,_,_), U                   , Enc(ExtRmi)       , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , O(Imm)            , U                 , U                 ),
  INST(kX86InstIdCmpxchg          , "cmpxchg"          , O_000F00(B0,U,_,_,_), U                   , Enc(X86RmReg)     , F(Lock)|F(Special)                 , EF(WWWWWW__), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdCmpxchg16b       , "cmpxchg16b"       , O_000F00(C7,1,_,W,_), U                   , Enc(X86M)         , F(None)|F(Special)                 , EF(__W_____), 0 , 0 , O(Mem)            , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdCmpxchg8b        , "cmpxchg8b"        , O_000F00(C7,1,_,_,_), U                   , Enc(X86M)         , F(None)|F(Special)                 , EF(__W_____), 0 , 0 , O(Mem)            , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdComisd           , "comisd"           , O_660F00(2F,U,_,_,_), U                   , Enc(ExtRm)        , F(Test)                            , EF(WWWWWW__), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdComiss           , "comiss"           , O_000F00(2F,U,_,_,_), U                   , Enc(ExtRm)        , F(Test)                            , EF(WWWWWW__), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdCpuid            , "cpuid"            , O_000F00(A2,U,_,_,_), U                   , Enc(X86Op)        , F(None)|F(Special)                 , EF(________), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdCqo              , "cqo"              , O_000000(99,U,_,W,_), U                   , Enc(X86Op)        , F(None)|F(Special)                 , EF(________), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdCrc32            , "crc32"            , O_F20F38(F0,U,_,_,_), U                   , Enc(ExtCrc)       , F(None)                            , EF(________), 0 , 0 , O(Gqd)            , O(GqdwbMem)       , U                 , U                 , U                 ),
  INST(kX86InstIdCvtdq2pd         , "cvtdq2pd"         , O_F30F00(E6,U,_,_,_), U                   , Enc(ExtRm)        , F(Move)                            , EF(________), 0 , 16, O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdCvtdq2ps         , "cvtdq2ps"         , O_000F00(5B,U,_,_,_), U                   , Enc(ExtRm)        , F(Move)                            , EF(________), 0 , 16, O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdCvtpd2dq         , "cvtpd2dq"         , O_F20F00(E6,U,_,_,_), U                   , Enc(ExtRm)        , F(Move)                            , EF(________), 0 , 16, O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdCvtpd2pi         , "cvtpd2pi"         , O_660F00(2D,U,_,_,_), U                   , Enc(ExtRm)        , F(Move)                            , EF(________), 0 , 8 , O(Mm)             , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdCvtpd2ps         , "cvtpd2ps"         , O_660F00(5A,U,_,_,_), U                   , Enc(ExtRm)        , F(Move)                            , EF(________), 0 , 16, O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdCvtpi2pd         , "cvtpi2pd"         , O_660F00(2A,U,_,_,_), U                   , Enc(ExtRm)        , F(Move)                            , EF(________), 0 , 16, O(Xmm)            , O(MmMem)          , U                 , U                 , U                 ),
  INST(kX86InstIdCvtpi2ps         , "cvtpi2ps"         , O_000F00(2A,U,_,_,_), U                   , Enc(ExtRm)        , F(Move)                            , EF(________), 0 , 8 , O(Xmm)            , O(MmMem)          , U                 , U                 , U                 ),
  INST(kX86InstIdCvtps2dq         , "cvtps2dq"         , O_660F00(5B,U,_,_,_), U                   , Enc(ExtRm)        , F(Move)                            , EF(________), 0 , 16, O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdCvtps2pd         , "cvtps2pd"         , O_000F00(5A,U,_,_,_), U                   , Enc(ExtRm)        , F(Move)                            , EF(________), 0 , 16, O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdCvtps2pi         , "cvtps2pi"         , O_000F00(2D,U,_,_,_), U                   , Enc(ExtRm)        , F(Move)                            , EF(________), 0 , 8 , O(Mm)             , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdCvtsd2si         , "cvtsd2si"         , O_F20F00(2D,U,_,_,_), U                   , Enc(ExtRm_Q)      , F(Move)                            , EF(________), 0 , 8 , O(Gqd)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdCvtsd2ss         , "cvtsd2ss"         , O_F20F00(5A,U,_,_,_), U                   , Enc(ExtRm)        , F(Move)                            , EF(________), 0 , 4 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdCvtsi2sd         , "cvtsi2sd"         , O_F20F00(2A,U,_,_,_), U                   , Enc(ExtRm_Q)      , F(Move)                            , EF(________), 0 , 8 , O(Xmm)            , O(GqdMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdCvtsi2ss         , "cvtsi2ss"         , O_F30F00(2A,U,_,_,_), U                   , Enc(ExtRm_Q)      , F(Move)                            , EF(________), 0 , 4 , O(Xmm)            , O(GqdMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdCvtss2sd         , "cvtss2sd"         , O_F30F00(5A,U,_,_,_), U                   , Enc(ExtRm)        , F(Move)                            , EF(________), 0 , 8 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdCvtss2si         , "cvtss2si"         , O_F30F00(2D,U,_,_,_), U                   , Enc(ExtRm_Q)      , F(Move)                            , EF(________), 0 , 8 , O(Gqd)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdCvttpd2dq        , "cvttpd2dq"        , O_660F00(E6,U,_,_,_), U                   , Enc(ExtRm)        , F(Move)                            , EF(________), 0 , 16, O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdCvttpd2pi        , "cvttpd2pi"        , O_660F00(2C,U,_,_,_), U                   , Enc(ExtRm)        , F(Move)                            , EF(________), 0 , 8 , O(Mm)             , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdCvttps2dq        , "cvttps2dq"        , O_F30F00(5B,U,_,_,_), U                   , Enc(ExtRm)        , F(Move)                            , EF(________), 0 , 16, O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdCvttps2pi        , "cvttps2pi"        , O_000F00(2C,U,_,_,_), U                   , Enc(ExtRm)        , F(Move)                            , EF(________), 0 , 8 , O(Mm)             , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdCvttsd2si        , "cvttsd2si"        , O_F20F00(2C,U,_,_,_), U                   , Enc(ExtRm_Q)      , F(Move)                            , EF(________), 0 , 8 , O(Gqd)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdCvttss2si        , "cvttss2si"        , O_F30F00(2C,U,_,_,_), U                   , Enc(ExtRm_Q)      , F(Move)                            , EF(________), 0 , 8 , O(Gqd)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdCwd              , "cwd"              , O_660000(99,U,_,_,_), U                   , Enc(X86Op)        , F(None)|F(Special)                 , EF(________), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdCwde             , "cwde"             , O_000000(98,U,_,_,_), U                   , Enc(X86Op)        , F(None)|F(Special)                 , EF(________), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdDaa              , "daa"              , O_000000(27,U,_,_,_), U                   , Enc(X86Op)        , F(None)|F(Special)                 , EF(UWWXWX__), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdDas              , "das"              , O_000000(2F,U,_,_,_), U                   , Enc(X86Op)        , F(None)|F(Special)                 , EF(UWWXWX__), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdDec              , "dec"              , O_000000(FE,1,_,_,_), O_000000(48,U,_,_,_), Enc(X86IncDec)    , F(Lock)                            , EF(WWWWW___), 0 , 0 , O(GqdwbMem)       , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdDiv              , "div"              , O_000000(F6,6,_,_,_), U                   , Enc(X86Rm_B)      , F(None)|F(Special)                 , EF(UUUUUU__), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdDivpd            , "divpd"            , O_660F00(5E,U,_,_,_), U                   , Enc(ExtRm)        , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdDivps            , "divps"            , O_000F00(5E,U,_,_,_), U                   , Enc(ExtRm)        , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdDivsd            , "divsd"            , O_F20F00(5E,U,_,_,_), U                   , Enc(ExtRm)        , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdDivss            , "divss"            , O_F30F00(5E,U,_,_,_), U                   , Enc(ExtRm)        , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdDppd             , "dppd"             , O_660F3A(41,U,_,_,_), U                   , Enc(ExtRmi)       , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , O(Imm)            , U                 , U                 ),
  INST(kX86InstIdDpps             , "dpps"             , O_660F3A(40,U,_,_,_), U                   , Enc(ExtRmi)       , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , O(Imm)            , U                 , U                 ),
  INST(kX86InstIdEmms             , "emms"             , O_000F00(77,U,_,_,_), U                   , Enc(X86Op)        , F(None)                            , EF(________), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdEnter            , "enter"            , O_000000(C8,U,_,_,_), U                   , Enc(X86Enter)     , F(None)|F(Special)                 , EF(________), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdExtractps        , "extractps"        , O_660F3A(17,U,_,_,_), U                   , Enc(ExtExtract)   , F(Move)                            , EF(________), 0 , 8 , O(GqdMem)         , O(Xmm)            , U                 , U                 , U                 ),
  INST(kX86InstIdExtrq            , "extrq"            , O_660F00(79,U,_,_,_), O_660F00(78,0,_,_,_), Enc(ExtExtrq)     , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(Xmm)|O(Imm)     , O(None)|O(Imm)    , U                 , U                 ),
  INST(kX86InstIdF2xm1            , "f2xm1"            , O_00_X(D9F0,U)      , U                   , Enc(FpuOp)        , F(Fp)                              , EF(________), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdFabs             , "fabs"             , O_00_X(D9E1,U)      , U                   , Enc(FpuOp)        , F(Fp)                              , EF(________), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdFadd             , "fadd"             , O_00_X(C0C0,0)      , U                   , Enc(FpuArith)     , F(Fp)|F(Mem4_8)                    , EF(________), 0 , 0 , O(FpMem)          , O(Fp)             , U                 , U                 , U                 ),
  INST(kX86InstIdFaddp            , "faddp"            , O_00_X(DEC0,U)      , U                   , Enc(FpuRDef)      , F(Fp)                              , EF(________), 0 , 0 , O(Fp)             , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdFbld             , "fbld"             , O_000000(DF,4,_,_,_), U                   , Enc(X86M)         , F(Fp)                              , EF(________), 0 , 0 , O(Mem)            , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdFbstp            , "fbstp"            , O_000000(DF,6,_,_,_), U                   , Enc(X86M)         , F(Fp)                              , EF(________), 0 , 0 , O(Mem)            , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdFchs             , "fchs"             , O_00_X(D9E0,U)      , U                   , Enc(FpuOp)        , F(Fp)                              , EF(________), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdFclex            , "fclex"            , O_9B_X(DBE2,U)      , U                   , Enc(FpuOp)        , F(Fp)                              , EF(________), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdFcmovb           , "fcmovb"           , O_00_X(DAC0,U)      , U                   , Enc(FpuR)         , F(Fp)                              , EF(_____R__), 0 , 0 , O(Fp)             , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdFcmovbe          , "fcmovbe"          , O_00_X(DAD0,U)      , U                   , Enc(FpuR)         , F(Fp)                              , EF(__R__R__), 0 , 0 , O(Fp)             , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdFcmove           , "fcmove"           , O_00_X(DAC8,U)      , U                   , Enc(FpuR)         , F(Fp)                              , EF(__R_____), 0 , 0 , O(Fp)             , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdFcmovnb          , "fcmovnb"          , O_00_X(DBC0,U)      , U                   , Enc(FpuR)         , F(Fp)                              , EF(_____R__), 0 , 0 , O(Fp)             , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdFcmovnbe         , "fcmovnbe"         , O_00_X(DBD0,U)      , U                   , Enc(FpuR)         , F(Fp)                              , EF(__R__R__), 0 , 0 , O(Fp)             , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdFcmovne          , "fcmovne"          , O_00_X(DBC8,U)      , U                   , Enc(FpuR)         , F(Fp)                              , EF(__R_____), 0 , 0 , O(Fp)             , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdFcmovnu          , "fcmovnu"          , O_00_X(DBD8,U)      , U                   , Enc(FpuR)         , F(Fp)                              , EF(____R___), 0 , 0 , O(Fp)             , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdFcmovu           , "fcmovu"           , O_00_X(DAD8,U)      , U                   , Enc(FpuR)         , F(Fp)                              , EF(____R___), 0 , 0 , O(Fp)             , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdFcom             , "fcom"             , O_00_X(D0D0,2)      , U                   , Enc(FpuCom)       , F(Fp)                              , EF(________), 0 , 0 , O(Fp)|O(Mem)      , O(Fp)             , U                 , U                 , U                 ),
  INST(kX86InstIdFcomi            , "fcomi"            , O_00_X(DBF0,U)      , U                   , Enc(FpuR)         , F(Fp)                              , EF(WWWWWW__), 0 , 0 , O(Fp)             , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdFcomip           , "fcomip"           , O_00_X(DFF0,U)      , U                   , Enc(FpuR)         , F(Fp)                              , EF(WWWWWW__), 0 , 0 , O(Fp)             , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdFcomp            , "fcomp"            , O_00_X(D8D8,3)      , U                   , Enc(FpuCom)       , F(Fp)                              , EF(________), 0 , 0 , O(Fp)|O(Mem)      , O(Fp)             , U                 , U                 , U                 ),
  INST(kX86InstIdFcompp           , "fcompp"           , O_00_X(DED9,U)      , U                   , Enc(FpuOp)        , F(Fp)                              , EF(________), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdFcos             , "fcos"             , O_00_X(D9FF,U)      , U                   , Enc(FpuOp)        , F(Fp)                              , EF(________), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdFdecstp          , "fdecstp"          , O_00_X(D9F6,U)      , U                   , Enc(FpuOp)        , F(Fp)                              , EF(________), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdFdiv             , "fdiv"             , O_00_X(F0F8,6)      , U                   , Enc(FpuArith)     , F(Fp)|F(Mem4_8)                    , EF(________), 0 , 0 , O(FpMem)          , O(Fp)             , U                 , U                 , U                 ),
  INST(kX86InstIdFdivp            , "fdivp"            , O_00_X(DEF8,U)      , U                   , Enc(FpuRDef)      , F(Fp)                              , EF(________), 0 , 0 , O(Fp)             , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdFdivr            , "fdivr"            , O_00_X(F8F0,7)      , U                   , Enc(FpuArith)     , F(Fp)|F(Mem4_8)                    , EF(________), 0 , 0 , O(FpMem)          , O(Fp)             , U                 , U                 , U                 ),
  INST(kX86InstIdFdivrp           , "fdivrp"           , O_00_X(DEF0,U)      , U                   , Enc(FpuRDef)      , F(Fp)                              , EF(________), 0 , 0 , O(Fp)             , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdFemms            , "femms"            , O_000F00(0E,U,_,_,_), U                   , Enc(X86Op)        , F(Fp)                              , EF(________), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdFfree            , "ffree"            , O_00_X(DDC0,U)      , U                   , Enc(FpuR)         , F(Fp)                              , EF(________), 0 , 0 , O(Fp)             , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdFiadd            , "fiadd"            , O_000000(DA,0,_,_,_), U                   , Enc(FpuM)         , F(Fp)|F(Mem2_4)                    , EF(________), 0 , 0 , O(Mem)            , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdFicom            , "ficom"            , O_000000(DA,2,_,_,_), U                   , Enc(FpuM)         , F(Fp)|F(Mem2_4)                    , EF(________), 0 , 0 , O(Mem)            , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdFicomp           , "ficomp"           , O_000000(DA,3,_,_,_), U                   , Enc(FpuM)         , F(Fp)|F(Mem2_4)                    , EF(________), 0 , 0 , O(Mem)            , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdFidiv            , "fidiv"            , O_000000(DA,6,_,_,_), U                   , Enc(FpuM)         , F(Fp)|F(Mem2_4)                    , EF(________), 0 , 0 , O(Mem)            , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdFidivr           , "fidivr"           , O_000000(DA,7,_,_,_), U                   , Enc(FpuM)         , F(Fp)|F(Mem2_4)                    , EF(________), 0 , 0 , O(Mem)            , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdFild             , "fild"             , O_000000(DB,0,_,_,_), O_000000(DF,5,_,_,_), Enc(FpuM)         , F(Fp)|F(Mem2_4_8)                  , EF(________), 0 , 0 , O(Mem)            , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdFimul            , "fimul"            , O_000000(DA,1,_,_,_), U                   , Enc(FpuM)         , F(Fp)|F(Mem2_4)                    , EF(________), 0 , 0 , O(Mem)            , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdFincstp          , "fincstp"          , O_00_X(D9F7,U)      , U                   , Enc(FpuOp)        , F(Fp)                              , EF(________), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdFinit            , "finit"            , O_9B_X(DBE3,U)      , U                   , Enc(FpuOp)        , F(Fp)                              , EF(________), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdFist             , "fist"             , O_000000(DB,2,_,_,_), U                   , Enc(FpuM)         , F(Fp)|F(Mem2_4)                    , EF(________), 0 , 0 , O(Mem)            , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdFistp            , "fistp"            , O_000000(DB,3,_,_,_), O_000000(DF,7,_,_,_), Enc(FpuM)         , F(Fp)|F(Mem2_4_8)                  , EF(________), 0 , 0 , O(Mem)            , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdFisttp           , "fisttp"           , O_000000(DB,1,_,_,_), O_000000(DD,1,_,_,_), Enc(FpuM)         , F(Fp)|F(Mem2_4_8)                  , EF(________), 0 , 0 , O(Mem)            , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdFisub            , "fisub"            , O_000000(DA,4,_,_,_), U                   , Enc(FpuM)         , F(Fp)|F(Mem2_4)                    , EF(________), 0 , 0 , O(Mem)            , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdFisubr           , "fisubr"           , O_000000(DA,5,_,_,_), U                   , Enc(FpuM)         , F(Fp)|F(Mem2_4)                    , EF(________), 0 , 0 , O(Mem)            , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdFld              , "fld"              , O_000000(D9,0,_,_,_), O_000000(DB,5,_,_,_), Enc(FpuFldFst)    , F(Fp)|F(Mem4_8_10)                 , EF(________), 0 , 0 , O(Mem)            , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdFld1             , "fld1"             , O_00_X(D9E8,U)      , U                   , Enc(FpuOp)        , F(Fp)                              , EF(________), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdFldcw            , "fldcw"            , O_000000(D9,5,_,_,_), U                   , Enc(X86M)         , F(Fp)                              , EF(________), 0 , 0 , O(Mem)            , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdFldenv           , "fldenv"           , O_000000(D9,4,_,_,_), U                   , Enc(X86M)         , F(Fp)                              , EF(________), 0 , 0 , O(Mem)            , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdFldl2e           , "fldl2e"           , O_00_X(D9EA,U)      , U                   , Enc(FpuOp)        , F(Fp)                              , EF(________), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdFldl2t           , "fldl2t"           , O_00_X(D9E9,U)      , U                   , Enc(FpuOp)        , F(Fp)                              , EF(________), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdFldlg2           , "fldlg2"           , O_00_X(D9EC,U)      , U                   , Enc(FpuOp)        , F(Fp)                              , EF(________), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdFldln2           , "fldln2"           , O_00_X(D9ED,U)      , U                   , Enc(FpuOp)        , F(Fp)                              , EF(________), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdFldpi            , "fldpi"            , O_00_X(D9EB,U)      , U                   , Enc(FpuOp)        , F(Fp)                              , EF(________), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdFldz             , "fldz"             , O_00_X(D9EE,U)      , U                   , Enc(FpuOp)        , F(Fp)                              , EF(________), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdFmul             , "fmul"             , O_00_X(C8C8,1)      , U                   , Enc(FpuArith)     , F(Fp)|F(Mem4_8)                    , EF(________), 0 , 0 , O(FpMem)          , O(Fp)             , U                 , U                 , U                 ),
  INST(kX86InstIdFmulp            , "fmulp"            , O_00_X(DEC8,U)      , U                   , Enc(FpuRDef)      , F(Fp)                              , EF(________), 0 , 0 , O(Fp)             , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdFnclex           , "fnclex"           , O_00_X(DBE2,U)      , U                   , Enc(FpuOp)        , F(Fp)                              , EF(________), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdFninit           , "fninit"           , O_00_X(DBE3,U)      , U                   , Enc(FpuOp)        , F(Fp)                              , EF(________), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdFnop             , "fnop"             , O_00_X(D9D0,U)      , U                   , Enc(FpuOp)        , F(Fp)                              , EF(________), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdFnsave           , "fnsave"           , O_000000(DD,6,_,_,_), U                   , Enc(X86M)         , F(Fp)                              , EF(________), 0 , 0 , O(Mem)            , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdFnstcw           , "fnstcw"           , O_000000(D9,7,_,_,_), U                   , Enc(X86M)         , F(Fp)                              , EF(________), 0 , 0 , O(Mem)            , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdFnstenv          , "fnstenv"          , O_000000(D9,6,_,_,_), U                   , Enc(X86M)         , F(Fp)                              , EF(________), 0 , 0 , O(Mem)            , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdFnstsw           , "fnstsw"           , O_000000(DD,7,_,_,_), O_00_X(DFE0,U)      , Enc(FpuStsw)      , F(Fp)                              , EF(________), 0 , 0 , O(Mem)            , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdFpatan           , "fpatan"           , O_00_X(D9F3,U)      , U                   , Enc(FpuOp)        , F(Fp)                              , EF(________), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdFprem            , "fprem"            , O_00_X(D9F8,U)      , U                   , Enc(FpuOp)        , F(Fp)                              , EF(________), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdFprem1           , "fprem1"           , O_00_X(D9F5,U)      , U                   , Enc(FpuOp)        , F(Fp)                              , EF(________), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdFptan            , "fptan"            , O_00_X(D9F2,U)      , U                   , Enc(FpuOp)        , F(Fp)                              , EF(________), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdFrndint          , "frndint"          , O_00_X(D9FC,U)      , U                   , Enc(FpuOp)        , F(Fp)                              , EF(________), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdFrstor           , "frstor"           , O_000000(DD,4,_,_,_), U                   , Enc(X86M)         , F(Fp)                              , EF(________), 0 , 0 , O(Mem)            , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdFsave            , "fsave"            , O_9B0000(DD,6,_,_,_), U                   , Enc(X86M)         , F(Fp)                              , EF(________), 0 , 0 , O(Mem)            , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdFscale           , "fscale"           , O_00_X(D9FD,U)      , U                   , Enc(FpuOp)        , F(Fp)                              , EF(________), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdFsin             , "fsin"             , O_00_X(D9FE,U)      , U                   , Enc(FpuOp)        , F(Fp)                              , EF(________), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdFsincos          , "fsincos"          , O_00_X(D9FB,U)      , U                   , Enc(FpuOp)        , F(Fp)                              , EF(________), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdFsqrt            , "fsqrt"            , O_00_X(D9FA,U)      , U                   , Enc(FpuOp)        , F(Fp)                              , EF(________), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdFst              , "fst"              , O_000000(D9,2,_,_,_), U                   , Enc(FpuFldFst)    , F(Fp)|F(Mem4_8)                    , EF(________), 0 , 0 , O(Mem)            , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdFstcw            , "fstcw"            , O_9B0000(D9,7,_,_,_), U                   , Enc(X86M)         , F(Fp)                              , EF(________), 0 , 0 , O(Mem)            , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdFstenv           , "fstenv"           , O_9B0000(D9,6,_,_,_), U                   , Enc(X86M)         , F(Fp)                              , EF(________), 0 , 0 , O(Mem)            , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdFstp             , "fstp"             , O_000000(D9,3,_,_,_), O_000000(DB,7,_,_,_), Enc(FpuFldFst)    , F(Fp)|F(Mem4_8_10)                 , EF(________), 0 , 0 , O(Mem)            , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdFstsw            , "fstsw"            , O_9B0000(DD,7,_,_,_), O_9B_X(DFE0,U)      , Enc(FpuStsw)      , F(Fp)                              , EF(________), 0 , 0 , O(Mem)            , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdFsub             , "fsub"             , O_00_X(E0E8,4)      , U                   , Enc(FpuArith)     , F(Fp)|F(Mem4_8)                    , EF(________), 0 , 0 , O(FpMem)          , O(Fp)             , U                 , U                 , U                 ),
  INST(kX86InstIdFsubp            , "fsubp"            , O_00_X(DEE8,U)      , U                   , Enc(FpuRDef)      , F(Fp)                              , EF(________), 0 , 0 , O(Fp)             , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdFsubr            , "fsubr"            , O_00_X(E8E0,5)      , U                   , Enc(FpuArith)     , F(Fp)|F(Mem4_8)                    , EF(________), 0 , 0 , O(FpMem)          , O(Fp)             , U                 , U                 , U                 ),
  INST(kX86InstIdFsubrp           , "fsubrp"           , O_00_X(DEE0,U)      , U                   , Enc(FpuRDef)      , F(Fp)                              , EF(________), 0 , 0 , O(Fp)             , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdFtst             , "ftst"             , O_00_X(D9E4,U)      , U                   , Enc(FpuOp)        , F(Fp)                              , EF(________), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdFucom            , "fucom"            , O_00_X(DDE0,U)      , U                   , Enc(FpuRDef)      , F(Fp)                              , EF(________), 0 , 0 , O(Fp)             , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdFucomi           , "fucomi"           , O_00_X(DBE8,U)      , U                   , Enc(FpuR)         , F(Fp)                              , EF(WWWWWW__), 0 , 0 , O(Fp)             , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdFucomip          , "fucomip"          , O_00_X(DFE8,U)      , U                   , Enc(FpuR)         , F(Fp)                              , EF(WWWWWW__), 0 , 0 , O(Fp)             , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdFucomp           , "fucomp"           , O_00_X(DDE8,U)      , U                   , Enc(FpuRDef)      , F(Fp)                              , EF(________), 0 , 0 , O(Fp)             , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdFucompp          , "fucompp"          , O_00_X(DAE9,U)      , U                   , Enc(FpuOp)        , F(Fp)                              , EF(________), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdFwait            , "fwait"            , O_000000(DB,U,_,_,_), U                   , Enc(X86Op)        , F(Fp)                              , EF(________), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdFxam             , "fxam"             , O_00_X(D9E5,U)      , U                   , Enc(FpuOp)        , F(Fp)                              , EF(________), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdFxch             , "fxch"             , O_00_X(D9C8,U)      , U                   , Enc(FpuR)         , F(Fp)                              , EF(________), 0 , 0 , O(Fp)             , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdFxrstor          , "fxrstor"          , O_000F00(AE,1,_,_,_), U                   , Enc(X86M)         , F(Fp)                              , EF(________), 0 , 0 , O(Mem)            , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdFxsave           , "fxsave"           , O_000F00(AE,0,_,_,_), U                   , Enc(X86M)         , F(Fp)                              , EF(________), 0 , 0 , O(Mem)            , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdFxtract          , "fxtract"          , O_00_X(D9F4,U)      , U                   , Enc(FpuOp)        , F(Fp)                              , EF(________), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdFyl2x            , "fyl2x"            , O_00_X(D9F1,U)      , U                   , Enc(FpuOp)        , F(Fp)                              , EF(________), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdFyl2xp1          , "fyl2xp1"          , O_00_X(D9F9,U)      , U                   , Enc(FpuOp)        , F(Fp)                              , EF(________), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdHaddpd           , "haddpd"           , O_660F00(7C,U,_,_,_), U                   , Enc(ExtRm)        , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdHaddps           , "haddps"           , O_F20F00(7C,U,_,_,_), U                   , Enc(ExtRm)        , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdHsubpd           , "hsubpd"           , O_660F00(7D,U,_,_,_), U                   , Enc(ExtRm)        , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdHsubps           , "hsubps"           , O_F20F00(7D,U,_,_,_), U                   , Enc(ExtRm)        , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdIdiv             , "idiv"             , O_000000(F6,7,_,_,_), U                   , Enc(X86Rm_B)      , F(None)|F(Special)                 , EF(UUUUUU__), 0 , 0 , 0                 , 0                 , U                 , U                 , U                 ),
  INST(kX86InstIdImul             , "imul"             , U                   , U                   , Enc(X86Imul)      , F(None)|F(Special)                 , EF(WUUUUW__), 0 , 0 , 0                 , 0                 , U                 , U                 , U                 ),
  INST(kX86InstIdInc              , "inc"              , O_000000(FE,0,_,_,_), O_000000(40,U,_,_,_), Enc(X86IncDec)    , F(Lock)                            , EF(WWWWW___), 0 , 0 , O(GqdwbMem)       , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdInsertps         , "insertps"         , O_660F3A(21,U,_,_,_), U                   , Enc(ExtRmi)       , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , O(Imm)            , U                 , U                 ),
  INST(kX86InstIdInsertq          , "insertq"          , O_F20F00(79,U,_,_,_), O_F20F00(78,U,_,_,_), Enc(ExtInsertq)   , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(Xmm)            , O(None)|O(Imm)    , O(None)|O(Imm)    , U                 ),
  INST(kX86InstIdInt              , "int"              , O_000000(CC,U,_,_,_), U                   , Enc(X86Int)       , F(None)                            , EF(_______W), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdJa               , "ja"               , O_000000(77,U,_,_,_), U                   , Enc(X86Jcc)       , F(Flow)                            , EF(__R__R__), 0 , 0 , O(Label)          , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdJae              , "jae"              , O_000000(73,U,_,_,_), U                   , Enc(X86Jcc)       , F(Flow)                            , EF(_____R__), 0 , 0 , O(Label)          , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdJb               , "jb"               , O_000000(72,U,_,_,_), U                   , Enc(X86Jcc)       , F(Flow)                            , EF(_____R__), 0 , 0 , O(Label)          , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdJbe              , "jbe"              , O_000000(76,U,_,_,_), U                   , Enc(X86Jcc)       , F(Flow)                            , EF(__R__R__), 0 , 0 , O(Label)          , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdJc               , "jc"               , O_000000(72,U,_,_,_), U                   , Enc(X86Jcc)       , F(Flow)                            , EF(_____R__), 0 , 0 , O(Label)          , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdJe               , "je"               , O_000000(74,U,_,_,_), U                   , Enc(X86Jcc)       , F(Flow)                            , EF(__R_____), 0 , 0 , O(Label)          , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdJg               , "jg"               , O_000000(7F,U,_,_,_), U                   , Enc(X86Jcc)       , F(Flow)                            , EF(RRR_____), 0 , 0 , O(Label)          , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdJge              , "jge"              , O_000000(7D,U,_,_,_), U                   , Enc(X86Jcc)       , F(Flow)                            , EF(RR______), 0 , 0 , O(Label)          , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdJl               , "jl"               , O_000000(7C,U,_,_,_), U                   , Enc(X86Jcc)       , F(Flow)                            , EF(RR______), 0 , 0 , O(Label)          , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdJle              , "jle"              , O_000000(7E,U,_,_,_), U                   , Enc(X86Jcc)       , F(Flow)                            , EF(RRR_____), 0 , 0 , O(Label)          , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdJna              , "jna"              , O_000000(76,U,_,_,_), U                   , Enc(X86Jcc)       , F(Flow)                            , EF(__R__R__), 0 , 0 , O(Label)          , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdJnae             , "jnae"             , O_000000(72,U,_,_,_), U                   , Enc(X86Jcc)       , F(Flow)                            , EF(_____R__), 0 , 0 , O(Label)          , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdJnb              , "jnb"              , O_000000(73,U,_,_,_), U                   , Enc(X86Jcc)       , F(Flow)                            , EF(_____R__), 0 , 0 , O(Label)          , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdJnbe             , "jnbe"             , O_000000(77,U,_,_,_), U                   , Enc(X86Jcc)       , F(Flow)                            , EF(__R__R__), 0 , 0 , O(Label)          , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdJnc              , "jnc"              , O_000000(73,U,_,_,_), U                   , Enc(X86Jcc)       , F(Flow)                            , EF(_____R__), 0 , 0 , O(Label)          , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdJne              , "jne"              , O_000000(75,U,_,_,_), U                   , Enc(X86Jcc)       , F(Flow)                            , EF(__R_____), 0 , 0 , O(Label)          , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdJng              , "jng"              , O_000000(7E,U,_,_,_), U                   , Enc(X86Jcc)       , F(Flow)                            , EF(RRR_____), 0 , 0 , O(Label)          , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdJnge             , "jnge"             , O_000000(7C,U,_,_,_), U                   , Enc(X86Jcc)       , F(Flow)                            , EF(RR______), 0 , 0 , O(Label)          , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdJnl              , "jnl"              , O_000000(7D,U,_,_,_), U                   , Enc(X86Jcc)       , F(Flow)                            , EF(RR______), 0 , 0 , O(Label)          , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdJnle             , "jnle"             , O_000000(7F,U,_,_,_), U                   , Enc(X86Jcc)       , F(Flow)                            , EF(RRR_____), 0 , 0 , O(Label)          , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdJno              , "jno"              , O_000000(71,U,_,_,_), U                   , Enc(X86Jcc)       , F(Flow)                            , EF(R_______), 0 , 0 , O(Label)          , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdJnp              , "jnp"              , O_000000(7B,U,_,_,_), U                   , Enc(X86Jcc)       , F(Flow)                            , EF(____R___), 0 , 0 , O(Label)          , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdJns              , "jns"              , O_000000(79,U,_,_,_), U                   , Enc(X86Jcc)       , F(Flow)                            , EF(_R______), 0 , 0 , O(Label)          , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdJnz              , "jnz"              , O_000000(75,U,_,_,_), U                   , Enc(X86Jcc)       , F(Flow)                            , EF(__R_____), 0 , 0 , O(Label)          , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdJo               , "jo"               , O_000000(70,U,_,_,_), U                   , Enc(X86Jcc)       , F(Flow)                            , EF(R_______), 0 , 0 , O(Label)          , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdJp               , "jp"               , O_000000(7A,U,_,_,_), U                   , Enc(X86Jcc)       , F(Flow)                            , EF(____R___), 0 , 0 , O(Label)          , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdJpe              , "jpe"              , O_000000(7A,U,_,_,_), U                   , Enc(X86Jcc)       , F(Flow)                            , EF(____R___), 0 , 0 , O(Label)          , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdJpo              , "jpo"              , O_000000(7B,U,_,_,_), U                   , Enc(X86Jcc)       , F(Flow)                            , EF(____R___), 0 , 0 , O(Label)          , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdJs               , "js"               , O_000000(78,U,_,_,_), U                   , Enc(X86Jcc)       , F(Flow)                            , EF(_R______), 0 , 0 , O(Label)          , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdJz               , "jz"               , O_000000(74,U,_,_,_), U                   , Enc(X86Jcc)       , F(Flow)                            , EF(__R_____), 0 , 0 , O(Label)          , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdJecxz            , "jecxz"            , O_000000(E3,U,_,_,_), U                   , Enc(X86Jecxz)     , F(Flow)|F(Special)                 , EF(________), 0 , 0 , O(Gqdw)           , O(Label)          , U                 , U                 , U                 ),
  INST(kX86InstIdJmp              , "jmp"              , O_000000(FF,4,_,_,_), O_000000(E9,U,_,_,_), Enc(X86Jmp)       , F(Flow)                            , EF(________), 0 , 0 , O(Label)|O(Imm)   , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdLahf             , "lahf"             , O_000000(9F,U,_,_,_), U                   , Enc(X86Op)        , F(None)|F(Special)                 , EF(_RRRRR__), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdLddqu            , "lddqu"            , O_F20F00(F0,U,_,_,_), U                   , Enc(ExtRm)        , F(Move)                            , EF(________), 0 , 16, O(Xmm)            , O(Mem)            , U                 , U                 , U                 ),
  INST(kX86InstIdLdmxcsr          , "ldmxcsr"          , O_000F00(AE,2,_,_,_), U                   , Enc(X86M)         , F(None)                            , EF(________), 0 , 0 , O(Mem)            , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdLea              , "lea"              , O_000000(8D,U,_,_,_), U                   , Enc(X86Lea)       , F(Move)                            , EF(________), 0 , 0 , O(Gqd)            , O(Mem)            , U                 , U                 , U                 ),
  INST(kX86InstIdLeave            , "leave"            , O_000000(C9,U,_,_,_), U                   , Enc(X86Op)        , F(None)|F(Special)                 , EF(________), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdLfence           , "lfence"           , O_000F00(AE,5,_,_,_), U                   , Enc(ExtFence)     , F(None)                            , EF(________), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdLodsB            , "lods_b"           , O_000000(AC,U,_,_,_), U                   , Enc(X86Op)        , F(Move)|F(Special)                 , EF(______R_), 0 , 1 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdLodsD            , "lods_d"           , O_000000(AD,U,_,_,_), U                   , Enc(X86Op)        , F(Move)|F(Special)                 , EF(______R_), 0 , 4 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdLodsQ            , "lods_q"           , O_000000(AD,U,_,W,_), U                   , Enc(X86Op)        , F(Move)|F(Special)                 , EF(______R_), 0 , 8 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdLodsW            , "lods_w"           , O_000000(AD,U,_,_,_), U                   , Enc(X86Op_66H)    , F(Move)|F(Special)                 , EF(______R_), 0 , 2 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdLzcnt            , "lzcnt"            , O_F30F00(BD,U,_,_,_), U                   , Enc(X86RegRm)     , F(None)                            , EF(UUWUUW__), 0 , 0 , O(Gqdw)           , O(GqdwMem)        , U                 , U                 , U                 ),
  INST(kX86InstIdMaskmovdqu       , "maskmovdqu"       , O_660F00(57,U,_,_,_), U                   , Enc(ExtRm)        , F(None)|F(Special)                 , EF(________), 0 , 0 , O(Xmm)            , O(Xmm)            , U                 , U                 , U                 ),
  INST(kX86InstIdMaskmovq         , "maskmovq"         , O_000F00(F7,U,_,_,_), U                   , Enc(ExtRm)        , F(None)|F(Special)                 , EF(________), 0 , 0 , O(Mm)             , O(Mm)             , U                 , U                 , U                 ),
  INST(kX86InstIdMaxpd            , "maxpd"            , O_660F00(5F,U,_,_,_), U                   , Enc(ExtRm)        , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdMaxps            , "maxps"            , O_000F00(5F,U,_,_,_), U                   , Enc(ExtRm)        , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdMaxsd            , "maxsd"            , O_F20F00(5F,U,_,_,_), U                   , Enc(ExtRm)        , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdMaxss            , "maxss"            , O_F30F00(5F,U,_,_,_), U                   , Enc(ExtRm)        , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdMfence           , "mfence"           , O_000F00(AE,6,_,_,_), U                   , Enc(ExtFence)     , F(None)                            , EF(________), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdMinpd            , "minpd"            , O_660F00(5D,U,_,_,_), U                   , Enc(ExtRm)        , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdMinps            , "minps"            , O_000F00(5D,U,_,_,_), U                   , Enc(ExtRm)        , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdMinsd            , "minsd"            , O_F20F00(5D,U,_,_,_), U                   , Enc(ExtRm)        , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdMinss            , "minss"            , O_F30F00(5D,U,_,_,_), U                   , Enc(ExtRm)        , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdMonitor          , "monitor"          , O_000F01(C8,U,_,_,_), U                   , Enc(X86Op)        , F(None)|F(Special)                 , EF(________), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdMov              , "mov"              , U                   , U                   , Enc(X86Mov)       , F(Move)                            , EF(________), 0 , 0 , O(GqdwbMem)       , O(GqdwbMem)|O(Imm), U                 , U                 , U                 ),
  INST(kX86InstIdMovPtr           , "mov_ptr"          , O_000000(A0,U,_,_,_), O_000000(A2,U,_,_,_), Enc(X86MovPtr)    , F(Move)|F(Special)                 , EF(________), 0 , 0 , O(Gqdwb)          , O(Imm)            , U                 , U                 , U                 ),
  INST(kX86InstIdMovapd           , "movapd"           , O_660F00(28,U,_,_,_), O_660F00(29,U,_,_,_), Enc(ExtMov)       , F(Move)                            , EF(________), 0 , 16, O(XmmMem)         , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdMovaps           , "movaps"           , O_000F00(28,U,_,_,_), O_000F00(29,U,_,_,_), Enc(ExtMov)       , F(Move)                            , EF(________), 0 , 16, O(XmmMem)         , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdMovbe            , "movbe"            , O_000F38(F0,U,_,_,_), O_000F38(F1,U,_,_,_), Enc(ExtMovBe)     , F(Move)                            , EF(________), 0 , 0 , O(GqdwMem)        , O(GqdwMem)        , U                 , U                 , U                 ),
  INST(kX86InstIdMovd             , "movd"             , O_000F00(6E,U,_,_,_), O_000F00(7E,U,_,_,_), Enc(ExtMovD)      , F(Move)                            , EF(________), 0 , 16, O(Gd)|O(MmXmmMem) , O(Gd)|O(MmXmmMem) , U                 , U                 , U                 ),
  INST(kX86InstIdMovddup          , "movddup"          , O_F20F00(12,U,_,_,_), U                   , Enc(ExtMov)       , F(Move)                            , EF(________), 0 , 16, O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdMovdq2q          , "movdq2q"          , O_F20F00(D6,U,_,_,_), U                   , Enc(ExtMov)       , F(Move)                            , EF(________), 0 , 8 , O(Mm)             , O(Xmm)            , U                 , U                 , U                 ),
  INST(kX86InstIdMovdqa           , "movdqa"           , O_660F00(6F,U,_,_,_), O_660F00(7F,U,_,_,_), Enc(ExtMov)       , F(Move)                            , EF(________), 0 , 16, O(XmmMem)         , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdMovdqu           , "movdqu"           , O_F30F00(6F,U,_,_,_), O_F30F00(7F,U,_,_,_), Enc(ExtMov)       , F(Move)                            , EF(________), 0 , 16, O(XmmMem)         , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdMovhlps          , "movhlps"          , O_000F00(12,U,_,_,_), U                   , Enc(ExtMov)       , F(Move)                            , EF(________), 0 , 8 , O(Xmm)            , O(Xmm)            , U                 , U                 , U                 ),
  INST(kX86InstIdMovhpd           , "movhpd"           , O_660F00(16,U,_,_,_), O_660F00(17,U,_,_,_), Enc(ExtMov)       , F(None)                            , EF(________), 8 , 8 , O(XmmMem)         , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdMovhps           , "movhps"           , O_000F00(16,U,_,_,_), O_000F00(17,U,_,_,_), Enc(ExtMov)       , F(None)                            , EF(________), 8 , 8 , O(XmmMem)         , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdMovlhps          , "movlhps"          , O_000F00(16,U,_,_,_), U                   , Enc(ExtMov)       , F(None)                            , EF(________), 8 , 8 , O(Xmm)            , O(Xmm)            , U                 , U                 , U                 ),
  INST(kX86InstIdMovlpd           , "movlpd"           , O_660F00(12,U,_,_,_), O_660F00(13,U,_,_,_), Enc(ExtMov)       , F(Move)                            , EF(________), 0 , 8 , O(XmmMem)         , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdMovlps           , "movlps"           , O_000F00(12,U,_,_,_), O_000F00(13,U,_,_,_), Enc(ExtMov)       , F(Move)                            , EF(________), 0 , 8 , O(XmmMem)         , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdMovmskpd         , "movmskpd"         , O_660F00(50,U,_,_,_), U                   , Enc(ExtMovNoRexW) , F(Move)                            , EF(________), 0 , 8 , O(Gqd)            , O(Xmm)            , U                 , U                 , U                 ),
  INST(kX86InstIdMovmskps         , "movmskps"         , O_000F00(50,U,_,_,_), U                   , Enc(ExtMovNoRexW) , F(Move)                            , EF(________), 0 , 8 , O(Gqd)            , O(Xmm)            , U                 , U                 , U                 ),
  INST(kX86InstIdMovntdq          , "movntdq"          , U                   , O_660F00(E7,U,_,_,_), Enc(ExtMov)       , F(Move)                            , EF(________), 0 , 16, O(Mem)            , O(Xmm)            , U                 , U                 , U                 ),
  INST(kX86InstIdMovntdqa         , "movntdqa"         , O_660F38(2A,U,_,_,_), U                   , Enc(ExtMov)       , F(Move)                            , EF(________), 0 , 16, O(Xmm)            , O(Mem)            , U                 , U                 , U                 ),
  INST(kX86InstIdMovnti           , "movnti"           , U                   , O_000F00(C3,U,_,_,_), Enc(ExtMov)       , F(Move)                            , EF(________), 0 , 8 , O(Mem)            , O(Gqd)            , U                 , U                 , U                 ),
  INST(kX86InstIdMovntpd          , "movntpd"          , U                   , O_660F00(2B,U,_,_,_), Enc(ExtMov)       , F(Move)                            , EF(________), 0 , 16, O(Mem)            , O(Xmm)            , U                 , U                 , U                 ),
  INST(kX86InstIdMovntps          , "movntps"          , U                   , O_000F00(2B,U,_,_,_), Enc(ExtMov)       , F(Move)                            , EF(________), 0 , 16, O(Mem)            , O(Xmm)            , U                 , U                 , U                 ),
  INST(kX86InstIdMovntq           , "movntq"           , U                   , O_000F00(E7,U,_,_,_), Enc(ExtMov)       , F(Move)                            , EF(________), 0 , 8 , O(Mem)            , O(Mm)             , U                 , U                 , U                 ),
  INST(kX86InstIdMovntsd          , "movntsd"          , U                   , O_F20F00(2B,U,_,_,_), Enc(ExtMov)       , F(Move)                            , EF(________), 0 , 8 , O(Mem)            , O(Xmm)            , U                 , U                 , U                 ),
  INST(kX86InstIdMovntss          , "movntss"          , U                   , O_F30F00(2B,U,_,_,_), Enc(ExtMov)       , F(Move)                            , EF(________), 0 , 4 , O(Mem)            , O(Xmm)            , U                 , U                 , U                 ),
  INST(kX86InstIdMovq             , "movq"             , O_000F00(6E,U,_,W,_), O_000F00(7E,U,_,W,_), Enc(ExtMovQ)      , F(Move)                            , EF(________), 0 , 16, O(Gq)|O(MmXmmMem) , O(Gq)|O(MmXmmMem) , U                 , U                 , U                 ),
  INST(kX86InstIdMovq2dq          , "movq2dq"          , O_F30F00(D6,U,_,_,_), U                   , Enc(ExtRm)        , F(Move)                            , EF(________), 0 , 16, O(Xmm)            , O(Mm)             , U                 , U                 , U                 ),
  INST(kX86InstIdMovsB            , "movs_b"           , O_000000(A4,U,_,_,_), U                   , Enc(X86Op)        , F(None)|F(Special)                 , EF(________), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdMovsD            , "movs_d"           , O_000000(A5,U,_,_,_), U                   , Enc(X86Op)        , F(Move)|F(Special)                 , EF(________), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdMovsQ            , "movs_q"           , O_000000(A5,U,_,W,_), U                   , Enc(X86Op)        , F(None)|F(Special)                 , EF(________), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdMovsW            , "movs_w"           , O_000000(A5,U,_,_,_), U                   , Enc(X86Op_66H)    , F(None)|F(Special)                 , EF(________), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdMovsd            , "movsd"            , O_F20F00(10,U,_,_,_), O_F20F00(11,U,_,_,_), Enc(ExtMov)       , F(Move)           |F(Z)            , EF(________), 0 , 8 , O(XmmMem)         , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdMovshdup         , "movshdup"         , O_F30F00(16,U,_,_,_), U                   , Enc(ExtRm)        , F(Move)                            , EF(________), 0 , 16, O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdMovsldup         , "movsldup"         , O_F30F00(12,U,_,_,_), U                   , Enc(ExtRm)        , F(Move)                            , EF(________), 0 , 16, O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdMovss            , "movss"            , O_F30F00(10,U,_,_,_), O_F30F00(11,U,_,_,_), Enc(ExtMov)       , F(Move)           |F(Z)            , EF(________), 0 , 4 , O(XmmMem)         , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdMovsx            , "movsx"            , O_000F00(BE,U,_,_,_), U                   , Enc(X86MovSxZx)   , F(Move)                            , EF(________), 0 , 0 , O(Gqdw)           , O(GwbMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdMovsxd           , "movsxd"           , O_000000(63,U,_,_,_), U                   , Enc(X86MovSxd)    , F(Move)                            , EF(________), 0 , 0 , O(Gq)             , O(GdMem)          , U                 , U                 , U                 ),
  INST(kX86InstIdMovupd           , "movupd"           , O_660F00(10,U,_,_,_), O_660F00(11,U,_,_,_), Enc(ExtMov)       , F(Move)                            , EF(________), 0 , 16, O(XmmMem)         , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdMovups           , "movups"           , O_000F00(10,U,_,_,_), O_000F00(11,U,_,_,_), Enc(ExtMov)       , F(Move)                            , EF(________), 0 , 16, O(XmmMem)         , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdMovzx            , "movzx"            , O_000F00(B6,U,_,_,_), U                   , Enc(X86MovSxZx)   , F(Move)                            , EF(________), 0 , 0 , O(Gqdw)           , O(GwbMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdMpsadbw          , "mpsadbw"          , O_660F3A(42,U,_,_,_), U                   , Enc(ExtRmi)       , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , O(Imm)            , U                 , U                 ),
  INST(kX86InstIdMul              , "mul"              , O_000000(F6,4,_,_,_), U                   , Enc(X86Rm_B)      , F(None)|F(Special)                 , EF(WUUUUW__), 0 , 0 , 0                 , 0                 , U                 , U                 , U                 ),
  INST(kX86InstIdMulpd            , "mulpd"            , O_660F00(59,U,_,_,_), U                   , Enc(ExtRm)        , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdMulps            , "mulps"            , O_000F00(59,U,_,_,_), U                   , Enc(ExtRm)        , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdMulsd            , "mulsd"            , O_F20F00(59,U,_,_,_), U                   , Enc(ExtRm)        , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdMulss            , "mulss"            , O_F30F00(59,U,_,_,_), U                   , Enc(ExtRm)        , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdMulx             , "mulx"             , O_F20F38(F6,U,_,_,_), U                   , Enc(AvxRvm)       , F(None)                            , EF(________), 0 , 0 , O(Gqd)            , O(Gqd)            , O(GqdMem)         , U                 , U                 ),
  INST(kX86InstIdMwait            , "mwait"            , O_000F01(C9,U,_,_,_), U                   , Enc(X86Op)        , F(None)|F(Special)                 , EF(________), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdNeg              , "neg"              , O_000000(F6,3,_,_,_), U                   , Enc(X86Rm_B)      , F(Lock)                            , EF(WWWWWW__), 0 , 0 , O(GqdwbMem)       , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdNop              , "nop"              , O_000000(90,U,_,_,_), U                   , Enc(X86Op)        , F(None)                            , EF(________), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdNot              , "not"              , O_000000(F6,2,_,_,_), U                   , Enc(X86Rm_B)      , F(Lock)                            , EF(________), 0 , 0 , O(GqdwbMem)       , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdOr               , "or"               , O_000000(08,1,_,_,_), U                   , Enc(X86Arith)     , F(Lock)                            , EF(WWWUWW__), 0 , 0 , O(GqdwbMem)       , O(GqdwbMem)|O(Imm), U                 , U                 , U                 ),
  INST(kX86InstIdOrpd             , "orpd"             , O_660F00(56,U,_,_,_), U                   , Enc(ExtRm)        , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdOrps             , "orps"             , O_000F00(56,U,_,_,_), U                   , Enc(ExtRm)        , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdPabsb            , "pabsb"            , O_000F38(1C,U,_,_,_), U                   , Enc(ExtRm_P)      , F(None)                            , EF(________), 0 , 0 , O(MmXmm)          , O(MmXmmMem)       , U                 , U                 , U                 ),
  INST(kX86InstIdPabsd            , "pabsd"            , O_000F38(1E,U,_,_,_), U                   , Enc(ExtRm_P)      , F(None)                            , EF(________), 0 , 0 , O(MmXmm)          , O(MmXmmMem)       , U                 , U                 , U                 ),
  INST(kX86InstIdPabsw            , "pabsw"            , O_000F38(1D,U,_,_,_), U                   , Enc(ExtRm_P)      , F(None)                            , EF(________), 0 , 0 , O(MmXmm)          , O(MmXmmMem)       , U                 , U                 , U                 ),
  INST(kX86InstIdPackssdw         , "packssdw"         , O_000F00(6B,U,_,_,_), U                   , Enc(ExtRm_P)      , F(None)                            , EF(________), 0 , 0 , O(MmXmm)          , O(MmXmmMem)       , U                 , U                 , U                 ),
  INST(kX86InstIdPacksswb         , "packsswb"         , O_000F00(63,U,_,_,_), U                   , Enc(ExtRm_P)      , F(None)                            , EF(________), 0 , 0 , O(MmXmm)          , O(MmXmmMem)       , U                 , U                 , U                 ),
  INST(kX86InstIdPackusdw         , "packusdw"         , O_660F38(2B,U,_,_,_), U                   , Enc(ExtRm)        , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdPackuswb         , "packuswb"         , O_000F00(67,U,_,_,_), U                   , Enc(ExtRm_P)      , F(None)                            , EF(________), 0 , 0 , O(MmXmm)          , O(MmXmmMem)       , U                 , U                 , U                 ),
  INST(kX86InstIdPaddb            , "paddb"            , O_000F00(FC,U,_,_,_), U                   , Enc(ExtRm_P)      , F(None)                            , EF(________), 0 , 0 , O(MmXmm)          , O(MmXmmMem)       , U                 , U                 , U                 ),
  INST(kX86InstIdPaddd            , "paddd"            , O_000F00(FE,U,_,_,_), U                   , Enc(ExtRm_P)      , F(None)                            , EF(________), 0 , 0 , O(MmXmm)          , O(MmXmmMem)       , U                 , U                 , U                 ),
  INST(kX86InstIdPaddq            , "paddq"            , O_000F00(D4,U,_,_,_), U                   , Enc(ExtRm_P)      , F(None)                            , EF(________), 0 , 0 , O(MmXmm)          , O(MmXmmMem)       , U                 , U                 , U                 ),
  INST(kX86InstIdPaddsb           , "paddsb"           , O_000F00(EC,U,_,_,_), U                   , Enc(ExtRm_P)      , F(None)                            , EF(________), 0 , 0 , O(MmXmm)          , O(MmXmmMem)       , U                 , U                 , U                 ),
  INST(kX86InstIdPaddsw           , "paddsw"           , O_000F00(ED,U,_,_,_), U                   , Enc(ExtRm_P)      , F(None)                            , EF(________), 0 , 0 , O(MmXmm)          , O(MmXmmMem)       , U                 , U                 , U                 ),
  INST(kX86InstIdPaddusb          , "paddusb"          , O_000F00(DC,U,_,_,_), U                   , Enc(ExtRm_P)      , F(None)                            , EF(________), 0 , 0 , O(MmXmm)          , O(MmXmmMem)       , U                 , U                 , U                 ),
  INST(kX86InstIdPaddusw          , "paddusw"          , O_000F00(DD,U,_,_,_), U                   , Enc(ExtRm_P)      , F(None)                            , EF(________), 0 , 0 , O(MmXmm)          , O(MmXmmMem)       , U                 , U                 , U                 ),
  INST(kX86InstIdPaddw            , "paddw"            , O_000F00(FD,U,_,_,_), U                   , Enc(ExtRm_P)      , F(None)                            , EF(________), 0 , 0 , O(MmXmm)          , O(MmXmmMem)       , U                 , U                 , U                 ),
  INST(kX86InstIdPalignr          , "palignr"          , O_000F3A(0F,U,_,_,_), U                   , Enc(ExtRmi_P)     , F(None)                            , EF(________), 0 , 0 , O(MmXmm)          , O(MmXmmMem)       , O(Imm)            , U                 , U                 ),
  INST(kX86InstIdPand             , "pand"             , O_000F00(DB,U,_,_,_), U                   , Enc(ExtRm_P)      , F(None)                            , EF(________), 0 , 0 , O(MmXmm)          , O(MmXmmMem)       , U                 , U                 , U                 ),
  INST(kX86InstIdPandn            , "pandn"            , O_000F00(DF,U,_,_,_), U                   , Enc(ExtRm_P)      , F(None)                            , EF(________), 0 , 0 , O(MmXmm)          , O(MmXmmMem)       , U                 , U                 , U                 ),
  INST(kX86InstIdPause            , "pause"            , O_F30000(90,U,_,_,_), U                   , Enc(X86Op)        , F(None)                            , EF(________), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdPavgb            , "pavgb"            , O_000F00(E0,U,_,_,_), U                   , Enc(ExtRm_P)      , F(None)                            , EF(________), 0 , 0 , O(MmXmm)          , O(MmXmmMem)       , U                 , U                 , U                 ),
  INST(kX86InstIdPavgw            , "pavgw"            , O_000F00(E3,U,_,_,_), U                   , Enc(ExtRm_P)      , F(None)                            , EF(________), 0 , 0 , O(MmXmm)          , O(MmXmmMem)       , U                 , U                 , U                 ),
  INST(kX86InstIdPblendvb         , "pblendvb"         , O_660F38(10,U,_,_,_), U                   , Enc(ExtRm)        , F(None)|F(Special)                 , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdPblendw          , "pblendw"          , O_660F3A(0E,U,_,_,_), U                   , Enc(ExtRmi)       , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , O(Imm)            , U                 , U                 ),
  INST(kX86InstIdPclmulqdq        , "pclmulqdq"        , O_660F3A(44,U,_,_,_), U                   , Enc(ExtRmi)       , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , O(Imm)            , U                 , U                 ),
  INST(kX86InstIdPcmpeqb          , "pcmpeqb"          , O_000F00(74,U,_,_,_), U                   , Enc(ExtRm_P)      , F(None)                            , EF(________), 0 , 0 , O(MmXmm)          , O(MmXmmMem)       , U                 , U                 , U                 ),
  INST(kX86InstIdPcmpeqd          , "pcmpeqd"          , O_000F00(76,U,_,_,_), U                   , Enc(ExtRm_P)      , F(None)                            , EF(________), 0 , 0 , O(MmXmm)          , O(MmXmmMem)       , U                 , U                 , U                 ),
  INST(kX86InstIdPcmpeqq          , "pcmpeqq"          , O_660F38(29,U,_,_,_), U                   , Enc(ExtRm)        , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdPcmpeqw          , "pcmpeqw"          , O_000F00(75,U,_,_,_), U                   , Enc(ExtRm_P)      , F(None)                            , EF(________), 0 , 0 , O(MmXmm)          , O(MmXmmMem)       , U                 , U                 , U                 ),
  INST(kX86InstIdPcmpestri        , "pcmpestri"        , O_660F3A(61,U,_,_,_), U                   , Enc(ExtRmi)       , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , O(Imm)            , U                 , U                 ),
  INST(kX86InstIdPcmpestrm        , "pcmpestrm"        , O_660F3A(60,U,_,_,_), U                   , Enc(ExtRmi)       , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , O(Imm)            , U                 , U                 ),
  INST(kX86InstIdPcmpgtb          , "pcmpgtb"          , O_000F00(64,U,_,_,_), U                   , Enc(ExtRm_P)      , F(None)                            , EF(________), 0 , 0 , O(MmXmm)          , O(MmXmmMem)       , U                 , U                 , U                 ),
  INST(kX86InstIdPcmpgtd          , "pcmpgtd"          , O_000F00(66,U,_,_,_), U                   , Enc(ExtRm_P)      , F(None)                            , EF(________), 0 , 0 , O(MmXmm)          , O(MmXmmMem)       , U                 , U                 , U                 ),
  INST(kX86InstIdPcmpgtq          , "pcmpgtq"          , O_660F38(37,U,_,_,_), U                   , Enc(ExtRm)        , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdPcmpgtw          , "pcmpgtw"          , O_000F00(65,U,_,_,_), U                   , Enc(ExtRm_P)      , F(None)                            , EF(________), 0 , 0 , O(MmXmm)          , O(MmXmmMem)       , U                 , U                 , U                 ),
  INST(kX86InstIdPcmpistri        , "pcmpistri"        , O_660F3A(63,U,_,_,_), U                   , Enc(ExtRmi)       , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , O(Imm)            , U                 , U                 ),
  INST(kX86InstIdPcmpistrm        , "pcmpistrm"        , O_660F3A(62,U,_,_,_), U                   , Enc(ExtRmi)       , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , O(Imm)            , U                 , U                 ),
  INST(kX86InstIdPdep             , "pdep"             , O_F20F38(F5,U,_,_,_), U                   , Enc(AvxRvm)       , F(None)                            , EF(________), 0 , 0 , O(Gqd)            , O(Gqd)            , O(GqdMem)         , U                 , U                 ),
  INST(kX86InstIdPext             , "pext"             , O_F30F38(F5,U,_,_,_), U                   , Enc(AvxRvm)       , F(None)                            , EF(________), 0 , 0 , O(Gqd)            , O(Gqd)            , O(GqdMem)         , U                 , U                 ),
  INST(kX86InstIdPextrb           , "pextrb"           , O_000F3A(14,U,_,_,_), U                   , Enc(ExtExtract)   , F(Move)                            , EF(________), 0 , 8 , O(Gd)|O(Gb)|O(Mem), O(Xmm)            , U                 , U                 , U                 ),
  INST(kX86InstIdPextrd           , "pextrd"           , O_000F3A(16,U,_,_,_), U                   , Enc(ExtExtract)   , F(Move)                            , EF(________), 0 , 8 , O(GdMem)          , O(Xmm)            , U                 , U                 , U                 ),
  INST(kX86InstIdPextrq           , "pextrq"           , O_000F3A(16,U,_,W,_), U                   , Enc(ExtExtract)   , F(Move)                            , EF(________), 0 , 8 , O(GqdMem)         , O(Xmm)            , U                 , U                 , U                 ),
  INST(kX86InstIdPextrw           , "pextrw"           , O_000F00(C5,U,_,_,_), O_000F3A(15,U,_,_,_), Enc(ExtExtrW)     , F(Move)                            , EF(________), 0 , 8 , O(GdMem)          , O(MmXmm)          , U                 , U                 , U                 ),
  INST(kX86InstIdPf2id            , "pf2id"            , O_000F0F(1D,U,_,_,_), U                   , Enc(3dNow)        , F(Move)                            , EF(________), 0 , 8 , O(Mm)             , O(MmMem)          , U                 , U                 , U                 ),
  INST(kX86InstIdPf2iw            , "pf2iw"            , O_000F0F(1C,U,_,_,_), U                   , Enc(3dNow)        , F(Move)                            , EF(________), 0 , 8 , O(Mm)             , O(MmMem)          , U                 , U                 , U                 ),
  INST(kX86InstIdPfacc            , "pfacc"            , O_000F0F(AE,U,_,_,_), U                   , Enc(3dNow)        , F(None)                            , EF(________), 0 , 0 , O(Mm)             , O(MmMem)          , U                 , U                 , U                 ),
  INST(kX86InstIdPfadd            , "pfadd"            , O_000F0F(9E,U,_,_,_), U                   , Enc(3dNow)        , F(None)                            , EF(________), 0 , 0 , O(Mm)             , O(MmMem)          , U                 , U                 , U                 ),
  INST(kX86InstIdPfcmpeq          , "pfcmpeq"          , O_000F0F(B0,U,_,_,_), U                   , Enc(3dNow)        , F(None)                            , EF(________), 0 , 0 , O(Mm)             , O(MmMem)          , U                 , U                 , U                 ),
  INST(kX86InstIdPfcmpge          , "pfcmpge"          , O_000F0F(90,U,_,_,_), U                   , Enc(3dNow)        , F(None)                            , EF(________), 0 , 0 , O(Mm)             , O(MmMem)          , U                 , U                 , U                 ),
  INST(kX86InstIdPfcmpgt          , "pfcmpgt"          , O_000F0F(A0,U,_,_,_), U                   , Enc(3dNow)        , F(None)                            , EF(________), 0 , 0 , O(Mm)             , O(MmMem)          , U                 , U                 , U                 ),
  INST(kX86InstIdPfmax            , "pfmax"            , O_000F0F(A4,U,_,_,_), U                   , Enc(3dNow)        , F(None)                            , EF(________), 0 , 0 , O(Mm)             , O(MmMem)          , U                 , U                 , U                 ),
  INST(kX86InstIdPfmin            , "pfmin"            , O_000F0F(94,U,_,_,_), U                   , Enc(3dNow)        , F(None)                            , EF(________), 0 , 0 , O(Mm)             , O(MmMem)          , U                 , U                 , U                 ),
  INST(kX86InstIdPfmul            , "pfmul"            , O_000F0F(B4,U,_,_,_), U                   , Enc(3dNow)        , F(None)                            , EF(________), 0 , 0 , O(Mm)             , O(MmMem)          , U                 , U                 , U                 ),
  INST(kX86InstIdPfnacc           , "pfnacc"           , O_000F0F(8A,U,_,_,_), U                   , Enc(3dNow)        , F(None)                            , EF(________), 0 , 0 , O(Mm)             , O(MmMem)          , U                 , U                 , U                 ),
  INST(kX86InstIdPfpnacc          , "pfpnacc"          , O_000F0F(8E,U,_,_,_), U                   , Enc(3dNow)        , F(None)                            , EF(________), 0 , 0 , O(Mm)             , O(MmMem)          , U                 , U                 , U                 ),
  INST(kX86InstIdPfrcp            , "pfrcp"            , O_000F0F(96,U,_,_,_), U                   , Enc(3dNow)        , F(Move)                            , EF(________), 0 , 8 , O(Mm)             , O(MmMem)          , U                 , U                 , U                 ),
  INST(kX86InstIdPfrcpit1         , "pfrcpit1"         , O_000F0F(A6,U,_,_,_), U                   , Enc(3dNow)        , F(None)                            , EF(________), 0 , 0 , O(Mm)             , O(MmMem)          , U                 , U                 , U                 ),
  INST(kX86InstIdPfrcpit2         , "pfrcpit2"         , O_000F0F(B6,U,_,_,_), U                   , Enc(3dNow)        , F(None)                            , EF(________), 0 , 0 , O(Mm)             , O(MmMem)          , U                 , U                 , U                 ),
  INST(kX86InstIdPfrsqit1         , "pfrsqit1"         , O_000F0F(A7,U,_,_,_), U                   , Enc(3dNow)        , F(None)                            , EF(________), 0 , 0 , O(Mm)             , O(MmMem)          , U                 , U                 , U                 ),
  INST(kX86InstIdPfrsqrt          , "pfrsqrt"          , O_000F0F(97,U,_,_,_), U                   , Enc(3dNow)        , F(None)                            , EF(________), 0 , 0 , O(Mm)             , O(MmMem)          , U                 , U                 , U                 ),
  INST(kX86InstIdPfsub            , "pfsub"            , O_000F0F(9A,U,_,_,_), U                   , Enc(3dNow)        , F(None)                            , EF(________), 0 , 0 , O(Mm)             , O(MmMem)          , U                 , U                 , U                 ),
  INST(kX86InstIdPfsubr           , "pfsubr"           , O_000F0F(AA,U,_,_,_), U                   , Enc(3dNow)        , F(None)                            , EF(________), 0 , 0 , O(Mm)             , O(MmMem)          , U                 , U                 , U                 ),
  INST(kX86InstIdPhaddd           , "phaddd"           , O_000F38(02,U,_,_,_), U                   , Enc(ExtRm_P)      , F(None)                            , EF(________), 0 , 0 , O(MmXmm)          , O(MmXmmMem)       , U                 , U                 , U                 ),
  INST(kX86InstIdPhaddsw          , "phaddsw"          , O_000F38(03,U,_,_,_), U                   , Enc(ExtRm_P)      , F(None)                            , EF(________), 0 , 0 , O(MmXmm)          , O(MmXmmMem)       , U                 , U                 , U                 ),
  INST(kX86InstIdPhaddw           , "phaddw"           , O_000F38(01,U,_,_,_), U                   , Enc(ExtRm_P)      , F(None)                            , EF(________), 0 , 0 , O(MmXmm)          , O(MmXmmMem)       , U                 , U                 , U                 ),
  INST(kX86InstIdPhminposuw       , "phminposuw"       , O_660F38(41,U,_,_,_), U                   , Enc(ExtRm)        , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdPhsubd           , "phsubd"           , O_000F38(06,U,_,_,_), U                   , Enc(ExtRm_P)      , F(None)                            , EF(________), 0 , 0 , O(MmXmm)          , O(MmXmmMem)       , U                 , U                 , U                 ),
  INST(kX86InstIdPhsubsw          , "phsubsw"          , O_000F38(07,U,_,_,_), U                   , Enc(ExtRm_P)      , F(None)                            , EF(________), 0 , 0 , O(MmXmm)          , O(MmXmmMem)       , U                 , U                 , U                 ),
  INST(kX86InstIdPhsubw           , "phsubw"           , O_000F38(05,U,_,_,_), U                   , Enc(ExtRm_P)      , F(None)                            , EF(________), 0 , 0 , O(MmXmm)          , O(MmXmmMem)       , U                 , U                 , U                 ),
  INST(kX86InstIdPi2fd            , "pi2fd"            , O_000F0F(0D,U,_,_,_), U                   , Enc(3dNow)        , F(Move)                            , EF(________), 0 , 8 , O(Mm)             , O(MmMem)          , U                 , U                 , U                 ),
  INST(kX86InstIdPi2fw            , "pi2fw"            , O_000F0F(0C,U,_,_,_), U                   , Enc(3dNow)        , F(Move)                            , EF(________), 0 , 8 , O(Mm)             , O(MmMem)          , U                 , U                 , U                 ),
  INST(kX86InstIdPinsrb           , "pinsrb"           , O_660F3A(20,U,_,_,_), U                   , Enc(ExtRmi)       , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(GdMem)          , O(Imm)            , U                 , U                 ),
  INST(kX86InstIdPinsrd           , "pinsrd"           , O_660F3A(22,U,_,_,_), U                   , Enc(ExtRmi)       , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(GdMem)          , O(Imm)            , U                 , U                 ),
  INST(kX86InstIdPinsrq           , "pinsrq"           , O_660F3A(22,U,_,W,_), U                   , Enc(ExtRmi)       , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(GqMem)          , O(Imm)            , U                 , U                 ),
  INST(kX86InstIdPinsrw           , "pinsrw"           , O_000F00(C4,U,_,_,_), U                   , Enc(ExtRmi_P)     , F(None)                            , EF(________), 0 , 0 , O(MmXmm)          , O(GdMem)          , O(Imm)            , U                 , U                 ),
  INST(kX86InstIdPmaddubsw        , "pmaddubsw"        , O_000F38(04,U,_,_,_), U                   , Enc(ExtRm_P)      , F(None)                            , EF(________), 0 , 0 , O(MmXmm)          , O(MmXmmMem)       , U                 , U                 , U                 ),
  INST(kX86InstIdPmaddwd          , "pmaddwd"          , O_000F00(F5,U,_,_,_), U                   , Enc(ExtRm_P)      , F(None)                            , EF(________), 0 , 0 , O(MmXmm)          , O(MmXmmMem)       , U                 , U                 , U                 ),
  INST(kX86InstIdPmaxsb           , "pmaxsb"           , O_660F38(3C,U,_,_,_), U                   , Enc(ExtRm)        , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdPmaxsd           , "pmaxsd"           , O_660F38(3D,U,_,_,_), U                   , Enc(ExtRm)        , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdPmaxsw           , "pmaxsw"           , O_000F00(EE,U,_,_,_), U                   , Enc(ExtRm_P)      , F(None)                            , EF(________), 0 , 0 , O(MmXmm)          , O(MmXmmMem)       , U                 , U                 , U                 ),
  INST(kX86InstIdPmaxub           , "pmaxub"           , O_000F00(DE,U,_,_,_), U                   , Enc(ExtRm_P)      , F(None)                            , EF(________), 0 , 0 , O(MmXmm)          , O(MmXmmMem)       , U                 , U                 , U                 ),
  INST(kX86InstIdPmaxud           , "pmaxud"           , O_660F38(3F,U,_,_,_), U                   , Enc(ExtRm)        , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdPmaxuw           , "pmaxuw"           , O_660F38(3E,U,_,_,_), U                   , Enc(ExtRm)        , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdPminsb           , "pminsb"           , O_660F38(38,U,_,_,_), U                   , Enc(ExtRm)        , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdPminsd           , "pminsd"           , O_660F38(39,U,_,_,_), U                   , Enc(ExtRm)        , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdPminsw           , "pminsw"           , O_000F00(EA,U,_,_,_), U                   , Enc(ExtRm_P)      , F(None)                            , EF(________), 0 , 0 , O(MmXmm)          , O(MmXmmMem)       , U                 , U                 , U                 ),
  INST(kX86InstIdPminub           , "pminub"           , O_000F00(DA,U,_,_,_), U                   , Enc(ExtRm_P)      , F(None)                            , EF(________), 0 , 0 , O(MmXmm)          , O(MmXmmMem)       , U                 , U                 , U                 ),
  INST(kX86InstIdPminud           , "pminud"           , O_660F38(3B,U,_,_,_), U                   , Enc(ExtRm)        , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdPminuw           , "pminuw"           , O_660F38(3A,U,_,_,_), U                   , Enc(ExtRm)        , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdPmovmskb         , "pmovmskb"         , O_000F00(D7,U,_,_,_), U                   , Enc(ExtRm_PQ)     , F(Move)                            , EF(________), 0 , 8 , O(Gqd)            , O(MmXmm)          , U                 , U                 , U                 ),
  INST(kX86InstIdPmovsxbd         , "pmovsxbd"         , O_660F38(21,U,_,_,_), U                   , Enc(ExtRm)        , F(Move)                            , EF(________), 0 , 16, O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdPmovsxbq         , "pmovsxbq"         , O_660F38(22,U,_,_,_), U                   , Enc(ExtRm)        , F(Move)                            , EF(________), 0 , 16, O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdPmovsxbw         , "pmovsxbw"         , O_660F38(20,U,_,_,_), U                   , Enc(ExtRm)        , F(Move)                            , EF(________), 0 , 16, O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdPmovsxdq         , "pmovsxdq"         , O_660F38(25,U,_,_,_), U                   , Enc(ExtRm)        , F(Move)                            , EF(________), 0 , 16, O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdPmovsxwd         , "pmovsxwd"         , O_660F38(23,U,_,_,_), U                   , Enc(ExtRm)        , F(Move)                            , EF(________), 0 , 16, O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdPmovsxwq         , "pmovsxwq"         , O_660F38(24,U,_,_,_), U                   , Enc(ExtRm)        , F(Move)                            , EF(________), 0 , 16, O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdPmovzxbd         , "pmovzxbd"         , O_660F38(31,U,_,_,_), U                   , Enc(ExtRm)        , F(Move)                            , EF(________), 0 , 16, O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdPmovzxbq         , "pmovzxbq"         , O_660F38(32,U,_,_,_), U                   , Enc(ExtRm)        , F(Move)                            , EF(________), 0 , 16, O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdPmovzxbw         , "pmovzxbw"         , O_660F38(30,U,_,_,_), U                   , Enc(ExtRm)        , F(Move)                            , EF(________), 0 , 16, O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdPmovzxdq         , "pmovzxdq"         , O_660F38(35,U,_,_,_), U                   , Enc(ExtRm)        , F(Move)                            , EF(________), 0 , 16, O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdPmovzxwd         , "pmovzxwd"         , O_660F38(33,U,_,_,_), U                   , Enc(ExtRm)        , F(Move)                            , EF(________), 0 , 16, O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdPmovzxwq         , "pmovzxwq"         , O_660F38(34,U,_,_,_), U                   , Enc(ExtRm)        , F(Move)                            , EF(________), 0 , 16, O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdPmuldq           , "pmuldq"           , O_660F38(28,U,_,_,_), U                   , Enc(ExtRm)        , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdPmulhrsw         , "pmulhrsw"         , O_000F38(0B,U,_,_,_), U                   , Enc(ExtRm_P)      , F(None)                            , EF(________), 0 , 0 , O(MmXmm)          , O(MmXmmMem)       , U                 , U                 , U                 ),
  INST(kX86InstIdPmulhuw          , "pmulhuw"          , O_000F00(E4,U,_,_,_), U                   , Enc(ExtRm_P)      , F(None)                            , EF(________), 0 , 0 , O(MmXmm)          , O(MmXmmMem)       , U                 , U                 , U                 ),
  INST(kX86InstIdPmulhw           , "pmulhw"           , O_000F00(E5,U,_,_,_), U                   , Enc(ExtRm_P)      , F(None)                            , EF(________), 0 , 0 , O(MmXmm)          , O(MmXmmMem)       , U                 , U                 , U                 ),
  INST(kX86InstIdPmulld           , "pmulld"           , O_660F38(40,U,_,_,_), U                   , Enc(ExtRm)        , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdPmullw           , "pmullw"           , O_000F00(D5,U,_,_,_), U                   , Enc(ExtRm_P)      , F(None)                            , EF(________), 0 , 0 , O(MmXmm)          , O(MmXmmMem)       , U                 , U                 , U                 ),
  INST(kX86InstIdPmuludq          , "pmuludq"          , O_000F00(F4,U,_,_,_), U                   , Enc(ExtRm_P)      , F(None)                            , EF(________), 0 , 0 , O(MmXmm)          , O(MmXmmMem)       , U                 , U                 , U                 ),
  INST(kX86InstIdPop              , "pop"              , O_000000(8F,0,_,_,_), O_000000(58,U,_,_,_), Enc(X86Pop)       , F(None)|F(Special)                 , EF(________), 0 , 0 , 0                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdPopa             , "popa"             , O_000000(61,U,_,_,_), U                   , Enc(X86Op)        , F(None)|F(Special)                 , EF(________), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdPopcnt           , "popcnt"           , O_F30F00(B8,U,_,_,_), U                   , Enc(X86RegRm)     , F(None)                            , EF(WWWWWW__), 0 , 0 , O(Gqdw)           , O(GqdwMem)        , U                 , U                 , U                 ),
  INST(kX86InstIdPopf             , "popf"             , O_000000(9D,U,_,_,_), U                   , Enc(X86Op)        , F(None)|F(Special)                 , EF(WWWWWWWW), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdPor              , "por"              , O_000F00(EB,U,_,_,_), U                   , Enc(ExtRm_P)      , F(None)                            , EF(________), 0 , 0 , O(MmXmm)          , O(MmXmmMem)       , U                 , U                 , U                 ),
  INST(kX86InstIdPrefetch         , "prefetch"         , O_000F00(18,U,_,_,_), U                   , Enc(ExtPrefetch)  , F(None)                            , EF(________), 0 , 0 , O(Mem)            , O(Imm)            , U                 , U                 , U                 ),
  INST(kX86InstIdPrefetch3dNow    , "prefetch_3dnow"   , O_000F00(0D,0,_,_,_), U                   , Enc(X86M)         , F(None)                            , EF(________), 0 , 0 , O(Mem)            , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdPrefetchw3dNow   , "prefetchw_3dnow"  , O_000F00(0D,1,_,_,_), U                   , Enc(X86M)         , F(None)                            , EF(________), 0 , 0 , O(Mem)            , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdPsadbw           , "psadbw"           , O_000F00(F6,U,_,_,_), U                   , Enc(ExtRm_P)      , F(None)                            , EF(________), 0 , 0 , O(MmXmm)          , O(MmXmmMem)       , U                 , U                 , U                 ),
  INST(kX86InstIdPshufb           , "pshufb"           , O_000F38(00,U,_,_,_), U                   , Enc(ExtRm_P)      , F(None)                            , EF(________), 0 , 0 , O(MmXmm)          , O(MmXmmMem)       , U                 , U                 , U                 ),
  INST(kX86InstIdPshufd           , "pshufd"           , O_660F00(70,U,_,_,_), U                   , Enc(ExtRmi)       , F(Move)                            , EF(________), 0 , 16, O(Xmm)            , O(XmmMem)         , O(Imm)            , U                 , U                 ),
  INST(kX86InstIdPshufhw          , "pshufhw"          , O_F30F00(70,U,_,_,_), U                   , Enc(ExtRmi)       , F(Move)                            , EF(________), 0 , 16, O(Xmm)            , O(XmmMem)         , O(Imm)            , U                 , U                 ),
  INST(kX86InstIdPshuflw          , "pshuflw"          , O_F20F00(70,U,_,_,_), U                   , Enc(ExtRmi)       , F(Move)                            , EF(________), 0 , 16, O(Xmm)            , O(XmmMem)         , O(Imm)            , U                 , U                 ),
  INST(kX86InstIdPshufw           , "pshufw"           , O_000F00(70,U,_,_,_), U                   , Enc(ExtRmi_P)     , F(Move)                            , EF(________), 0 , 8 , O(Mm)             , O(MmMem)          , O(Imm)            , U                 , U                 ),
  INST(kX86InstIdPsignb           , "psignb"           , O_000F38(08,U,_,_,_), U                   , Enc(ExtRm_P)      , F(None)                            , EF(________), 0 , 0 , O(MmXmm)          , O(MmXmmMem)       , U                 , U                 , U                 ),
  INST(kX86InstIdPsignd           , "psignd"           , O_000F38(0A,U,_,_,_), U                   , Enc(ExtRm_P)      , F(None)                            , EF(________), 0 , 0 , O(MmXmm)          , O(MmXmmMem)       , U                 , U                 , U                 ),
  INST(kX86InstIdPsignw           , "psignw"           , O_000F38(09,U,_,_,_), U                   , Enc(ExtRm_P)      , F(None)                            , EF(________), 0 , 0 , O(MmXmm)          , O(MmXmmMem)       , U                 , U                 , U                 ),
  INST(kX86InstIdPslld            , "pslld"            , O_000F00(F2,U,_,_,_), O_000F00(72,6,_,_,_), Enc(ExtRmRi_P)    , F(None)                            , EF(________), 0 , 0 , O(MmXmm)          , O(MmXmmMem)|O(Imm), U                 , U                 , U                 ),
  INST(kX86InstIdPslldq           , "pslldq"           , U                   , O_660F00(73,7,_,_,_), Enc(ExtRmRi)      , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(Imm)            , U                 , U                 , U                 ),
  INST(kX86InstIdPsllq            , "psllq"            , O_000F00(F3,U,_,_,_), O_000F00(73,6,_,_,_), Enc(ExtRmRi_P)    , F(None)                            , EF(________), 0 , 0 , O(MmXmm)          , O(MmXmmMem)|O(Imm), U                 , U                 , U                 ),
  INST(kX86InstIdPsllw            , "psllw"            , O_000F00(F1,U,_,_,_), O_000F00(71,6,_,_,_), Enc(ExtRmRi_P)    , F(None)                            , EF(________), 0 , 0 , O(MmXmm)          , O(MmXmmMem)|O(Imm), U                 , U                 , U                 ),
  INST(kX86InstIdPsrad            , "psrad"            , O_000F00(E2,U,_,_,_), O_000F00(72,4,_,_,_), Enc(ExtRmRi_P)    , F(None)                            , EF(________), 0 , 0 , O(MmXmm)          , O(MmXmmMem)|O(Imm), U                 , U                 , U                 ),
  INST(kX86InstIdPsraw            , "psraw"            , O_000F00(E1,U,_,_,_), O_000F00(71,4,_,_,_), Enc(ExtRmRi_P)    , F(None)                            , EF(________), 0 , 0 , O(MmXmm)          , O(MmXmmMem)|O(Imm), U                 , U                 , U                 ),
  INST(kX86InstIdPsrld            , "psrld"            , O_000F00(D2,U,_,_,_), O_000F00(72,2,_,_,_), Enc(ExtRmRi_P)    , F(None)                            , EF(________), 0 , 0 , O(MmXmm)          , O(MmXmmMem)|O(Imm), U                 , U                 , U                 ),
  INST(kX86InstIdPsrldq           , "psrldq"           , U                   , O_660F00(73,3,_,_,_), Enc(ExtRmRi)      , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(Imm)            , U                 , U                 , U                 ),
  INST(kX86InstIdPsrlq            , "psrlq"            , O_000F00(D3,U,_,_,_), O_000F00(73,2,_,_,_), Enc(ExtRmRi_P)    , F(None)                            , EF(________), 0 , 0 , O(MmXmm)          , O(MmXmmMem)|O(Imm), U                 , U                 , U                 ),
  INST(kX86InstIdPsrlw            , "psrlw"            , O_000F00(D1,U,_,_,_), O_000F00(71,2,_,_,_), Enc(ExtRmRi_P)    , F(None)                            , EF(________), 0 , 0 , O(MmXmm)          , O(MmXmmMem)|O(Imm), U                 , U                 , U                 ),
  INST(kX86InstIdPsubb            , "psubb"            , O_000F00(F8,U,_,_,_), U                   , Enc(ExtRm_P)      , F(None)                            , EF(________), 0 , 0 , O(MmXmm)          , O(MmXmmMem)       , U                 , U                 , U                 ),
  INST(kX86InstIdPsubd            , "psubd"            , O_000F00(FA,U,_,_,_), U                   , Enc(ExtRm_P)      , F(None)                            , EF(________), 0 , 0 , O(MmXmm)          , O(MmXmmMem)       , U                 , U                 , U                 ),
  INST(kX86InstIdPsubq            , "psubq"            , O_000F00(FB,U,_,_,_), U                   , Enc(ExtRm_P)      , F(None)                            , EF(________), 0 , 0 , O(MmXmm)          , O(MmXmmMem)       , U                 , U                 , U                 ),
  INST(kX86InstIdPsubsb           , "psubsb"           , O_000F00(E8,U,_,_,_), U                   , Enc(ExtRm_P)      , F(None)                            , EF(________), 0 , 0 , O(MmXmm)          , O(MmXmmMem)       , U                 , U                 , U                 ),
  INST(kX86InstIdPsubsw           , "psubsw"           , O_000F00(E9,U,_,_,_), U                   , Enc(ExtRm_P)      , F(None)                            , EF(________), 0 , 0 , O(MmXmm)          , O(MmXmmMem)       , U                 , U                 , U                 ),
  INST(kX86InstIdPsubusb          , "psubusb"          , O_000F00(D8,U,_,_,_), U                   , Enc(ExtRm_P)      , F(None)                            , EF(________), 0 , 0 , O(MmXmm)          , O(MmXmmMem)       , U                 , U                 , U                 ),
  INST(kX86InstIdPsubusw          , "psubusw"          , O_000F00(D9,U,_,_,_), U                   , Enc(ExtRm_P)      , F(None)                            , EF(________), 0 , 0 , O(MmXmm)          , O(MmXmmMem)       , U                 , U                 , U                 ),
  INST(kX86InstIdPsubw            , "psubw"            , O_000F00(F9,U,_,_,_), U                   , Enc(ExtRm_P)      , F(None)                            , EF(________), 0 , 0 , O(MmXmm)          , O(MmXmmMem)       , U                 , U                 , U                 ),
  INST(kX86InstIdPswapd           , "pswapd"           , O_000F0F(BB,U,_,_,_), U                   , Enc(3dNow)        , F(Move)                            , EF(________), 0 , 8 , O(Mm)             , O(MmMem)          , U                 , U                 , U                 ),
  INST(kX86InstIdPtest            , "ptest"            , O_660F38(17,U,_,_,_), U                   , Enc(ExtRm)        , F(Test)                            , EF(WWWWWW__), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdPunpckhbw        , "punpckhbw"        , O_000F00(68,U,_,_,_), U                   , Enc(ExtRm_P)      , F(None)                            , EF(________), 0 , 0 , O(MmXmm)          , O(MmXmmMem)       , U                 , U                 , U                 ),
  INST(kX86InstIdPunpckhdq        , "punpckhdq"        , O_000F00(6A,U,_,_,_), U                   , Enc(ExtRm_P)      , F(None)                            , EF(________), 0 , 0 , O(MmXmm)          , O(MmXmmMem)       , U                 , U                 , U                 ),
  INST(kX86InstIdPunpckhqdq       , "punpckhqdq"       , O_660F00(6D,U,_,_,_), U                   , Enc(ExtRm)        , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdPunpckhwd        , "punpckhwd"        , O_000F00(69,U,_,_,_), U                   , Enc(ExtRm_P)      , F(None)                            , EF(________), 0 , 0 , O(MmXmm)          , O(MmXmmMem)       , U                 , U                 , U                 ),
  INST(kX86InstIdPunpcklbw        , "punpcklbw"        , O_000F00(60,U,_,_,_), U                   , Enc(ExtRm_P)      , F(None)                            , EF(________), 0 , 0 , O(MmXmm)          , O(MmXmmMem)       , U                 , U                 , U                 ),
  INST(kX86InstIdPunpckldq        , "punpckldq"        , O_000F00(62,U,_,_,_), U                   , Enc(ExtRm_P)      , F(None)                            , EF(________), 0 , 0 , O(MmXmm)          , O(MmXmmMem)       , U                 , U                 , U                 ),
  INST(kX86InstIdPunpcklqdq       , "punpcklqdq"       , O_660F00(6C,U,_,_,_), U                   , Enc(ExtRm)        , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdPunpcklwd        , "punpcklwd"        , O_000F00(61,U,_,_,_), U                   , Enc(ExtRm_P)      , F(None)                            , EF(________), 0 , 0 , O(MmXmm)          , O(MmXmmMem)       , U                 , U                 , U                 ),
  INST(kX86InstIdPush             , "push"             , O_000000(FF,6,_,_,_), O_000000(50,U,_,_,_), Enc(X86Push)      , F(None)|F(Special)                 , EF(________), 0 , 0 , 0                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdPusha            , "pusha"            , O_000000(60,U,_,_,_), U                   , Enc(X86Op)        , F(None)|F(Special)                 , EF(________), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdPushf            , "pushf"            , O_000000(9C,U,_,_,_), U                   , Enc(X86Op)        , F(None)|F(Special)                 , EF(RRRRRRRR), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdPxor             , "pxor"             , O_000F00(EF,U,_,_,_), U                   , Enc(ExtRm_P)      , F(None)                            , EF(________), 0 , 0 , O(MmXmm)          , O(MmXmmMem)       , U                 , U                 , U                 ),
  INST(kX86InstIdRcl              , "rcl"              , O_000000(D0,2,_,_,_), U                   , Enc(X86Rot)       , F(None)|F(Special)                 , EF(W____X__), 0 , 0 , O(GqdwbMem)       , O(Gb)|O(Imm)      , U                 , U                 , U                 ),
  INST(kX86InstIdRcpps            , "rcpps"            , O_000F00(53,U,_,_,_), U                   , Enc(ExtRm)        , F(Move)                            , EF(________), 0 , 16, O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdRcpss            , "rcpss"            , O_F30F00(53,U,_,_,_), U                   , Enc(ExtRm)        , F(Move)                            , EF(________), 0 , 4 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdRcr              , "rcr"              , O_000000(D0,3,_,_,_), U                   , Enc(X86Rot)       , F(None)|F(Special)                 , EF(W____X__), 0 , 0 , O(GqdwbMem)       , O(Gb)|O(Imm)      , U                 , U                 , U                 ),
  INST(kX86InstIdRdfsbase         , "rdfsbase"         , O_F30F00(AE,0,_,_,_), U                   , Enc(X86Rm)        , F(Move)                            , EF(________), 0 , 8 , O(Gqd)            , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdRdgsbase         , "rdgsbase"         , O_F30F00(AE,1,_,_,_), U                   , Enc(X86Rm)        , F(Move)                            , EF(________), 0 , 8 , O(Gqd)            , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdRdrand           , "rdrand"           , O_000F00(C7,6,_,_,_), U                   , Enc(X86Rm)        , F(Move)                            , EF(WWWWWW__), 0 , 8 , O(Gqdw)           , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdRdtsc            , "rdtsc"            , O_000F00(31,U,_,_,_), U                   , Enc(X86Op)        , F(None)|F(Special)                 , EF(________), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdRdtscp           , "rdtscp"           , O_000F01(F9,U,_,_,_), U                   , Enc(X86Op)        , F(None)|F(Special)                 , EF(________), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdRepLodsB         , "rep lods_b"       , O_000000(AC,1,_,_,_), U                   , Enc(X86Rep)       , F(None)|F(Special)                 , EF(______R_), 0 , 0 , O(Mem)            , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdRepLodsD         , "rep lods_d"       , O_000000(AD,1,_,_,_), U                   , Enc(X86Rep)       , F(None)|F(Special)                 , EF(______R_), 0 , 0 , O(Mem)            , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdRepLodsQ         , "rep lods_q"       , O_000000(AD,1,_,W,_), U                   , Enc(X86Rep)       , F(None)|F(Special)                 , EF(______R_), 0 , 0 , O(Mem)            , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdRepLodsW         , "rep lods_w"       , O_660000(AD,1,_,_,_), U                   , Enc(X86Rep)       , F(None)|F(Special)                 , EF(______R_), 0 , 0 , O(Mem)            , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdRepMovsB         , "rep movs_b"       , O_000000(A4,1,_,_,_), U                   , Enc(X86Rep)       , F(None)|F(Special)                 , EF(______R_), 0 , 0 , O(Mem)            , O(Mem)            , U                 , U                 , U                 ),
  INST(kX86InstIdRepMovsD         , "rep movs_d"       , O_000000(A5,1,_,_,_), U                   , Enc(X86Rep)       , F(None)|F(Special)                 , EF(______R_), 0 , 0 , O(Mem)            , O(Mem)            , U                 , U                 , U                 ),
  INST(kX86InstIdRepMovsQ         , "rep movs_q"       , O_000000(A5,1,_,W,_), U                   , Enc(X86Rep)       , F(None)|F(Special)                 , EF(______R_), 0 , 0 , O(Mem)            , O(Mem)            , U                 , U                 , U                 ),
  INST(kX86InstIdRepMovsW         , "rep movs_w"       , O_660000(A5,1,_,_,_), U                   , Enc(X86Rep)       , F(None)|F(Special)                 , EF(______R_), 0 , 0 , O(Mem)            , O(Mem)            , U                 , U                 , U                 ),
  INST(kX86InstIdRepStosB         , "rep stos_b"       , O_000000(AA,1,_,_,_), U                   , Enc(X86Rep)       , F(None)|F(Special)                 , EF(______R_), 0 , 0 , O(Mem)            , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdRepStosD         , "rep stos_d"       , O_000000(AB,1,_,_,_), U                   , Enc(X86Rep)       , F(None)|F(Special)                 , EF(______R_), 0 , 0 , O(Mem)            , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdRepStosQ         , "rep stos_q"       , O_000000(AB,1,_,W,_), U                   , Enc(X86Rep)       , F(None)|F(Special)                 , EF(______R_), 0 , 0 , O(Mem)            , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdRepStosW         , "rep stos_w"       , O_660000(AB,1,_,_,_), U                   , Enc(X86Rep)       , F(None)|F(Special)                 , EF(______R_), 0 , 0 , O(Mem)            , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdRepeCmpsB        , "repe cmps_b"      , O_000000(A6,1,_,_,_), U                   , Enc(X86Rep)       , F(None)|F(Special)                 , EF(WWWWWWR_), 0 , 0 , O(Mem)            , O(Mem)            , U                 , U                 , U                 ),
  INST(kX86InstIdRepeCmpsD        , "repe cmps_d"      , O_000000(A7,1,_,_,_), U                   , Enc(X86Rep)       , F(None)|F(Special)                 , EF(WWWWWWR_), 0 , 0 , O(Mem)            , O(Mem)            , U                 , U                 , U                 ),
  INST(kX86InstIdRepeCmpsQ        , "repe cmps_q"      , O_000000(A7,1,_,W,_), U                   , Enc(X86Rep)       , F(None)|F(Special)                 , EF(WWWWWWR_), 0 , 0 , O(Mem)            , O(Mem)            , U                 , U                 , U                 ),
  INST(kX86InstIdRepeCmpsW        , "repe cmps_w"      , O_660000(A7,1,_,_,_), U                   , Enc(X86Rep)       , F(None)|F(Special)                 , EF(WWWWWWR_), 0 , 0 , O(Mem)            , O(Mem)            , U                 , U                 , U                 ),
  INST(kX86InstIdRepeScasB        , "repe scas_b"      , O_000000(AE,1,_,_,_), U                   , Enc(X86Rep)       , F(None)|F(Special)                 , EF(WWWWWWR_), 0 , 0 , O(Mem)            , O(Mem)            , U                 , U                 , U                 ),
  INST(kX86InstIdRepeScasD        , "repe scas_d"      , O_000000(AF,1,_,_,_), U                   , Enc(X86Rep)       , F(None)|F(Special)                 , EF(WWWWWWR_), 0 , 0 , O(Mem)            , O(Mem)            , U                 , U                 , U                 ),
  INST(kX86InstIdRepeScasQ        , "repe scas_q"      , O_000000(AF,1,_,W,_), U                   , Enc(X86Rep)       , F(None)|F(Special)                 , EF(WWWWWWR_), 0 , 0 , O(Mem)            , O(Mem)            , U                 , U                 , U                 ),
  INST(kX86InstIdRepeScasW        , "repe scas_w"      , O_660000(AF,1,_,_,_), U                   , Enc(X86Rep)       , F(None)|F(Special)                 , EF(WWWWWWR_), 0 , 0 , O(Mem)            , O(Mem)            , U                 , U                 , U                 ),
  INST(kX86InstIdRepneCmpsB       , "repne cmps_b"     , O_000000(A6,0,_,_,_), U                   , Enc(X86Rep)       , F(None)|F(Special)                 , EF(WWWWWWR_), 0 , 0 , O(Mem)            , O(Mem)            , U                 , U                 , U                 ),
  INST(kX86InstIdRepneCmpsD       , "repne cmps_d"     , O_000000(A7,0,_,_,_), U                   , Enc(X86Rep)       , F(None)|F(Special)                 , EF(WWWWWWR_), 0 , 0 , O(Mem)            , O(Mem)            , U                 , U                 , U                 ),
  INST(kX86InstIdRepneCmpsQ       , "repne cmps_q"     , O_000000(A7,0,_,W,_), U                   , Enc(X86Rep)       , F(None)|F(Special)                 , EF(WWWWWWR_), 0 , 0 , O(Mem)            , O(Mem)            , U                 , U                 , U                 ),
  INST(kX86InstIdRepneCmpsW       , "repne cmps_w"     , O_660000(A7,0,_,_,_), U                   , Enc(X86Rep)       , F(None)|F(Special)                 , EF(WWWWWWR_), 0 , 0 , O(Mem)            , O(Mem)            , U                 , U                 , U                 ),
  INST(kX86InstIdRepneScasB       , "repne scas_b"     , O_000000(AE,0,_,_,_), U                   , Enc(X86Rep)       , F(None)|F(Special)                 , EF(WWWWWWR_), 0 , 0 , O(Mem)            , O(Mem)            , U                 , U                 , U                 ),
  INST(kX86InstIdRepneScasD       , "repne scas_d"     , O_000000(AF,0,_,_,_), U                   , Enc(X86Rep)       , F(None)|F(Special)                 , EF(WWWWWWR_), 0 , 0 , O(Mem)            , O(Mem)            , U                 , U                 , U                 ),
  INST(kX86InstIdRepneScasQ       , "repne scas_q"     , O_000000(AF,0,_,W,_), U                   , Enc(X86Rep)       , F(None)|F(Special)                 , EF(WWWWWWR_), 0 , 0 , O(Mem)            , O(Mem)            , U                 , U                 , U                 ),
  INST(kX86InstIdRepneScasW       , "repne scas_w"     , O_660000(AF,0,_,_,_), U                   , Enc(X86Rep)       , F(None)|F(Special)                 , EF(WWWWWWR_), 0 , 0 , O(Mem)            , O(Mem)            , U                 , U                 , U                 ),
  INST(kX86InstIdRet              , "ret"              , O_000000(C2,U,_,_,_), U                   , Enc(X86Ret)       , F(None)|F(Special)                 , EF(________), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdRol              , "rol"              , O_000000(D0,0,_,_,_), U                   , Enc(X86Rot)       , F(None)|F(Special)                 , EF(W____W__), 0 , 0 , O(GqdwbMem)       , O(Gb)|O(Imm)      , U                 , U                 , U                 ),
  INST(kX86InstIdRor              , "ror"              , O_000000(D0,1,_,_,_), U                   , Enc(X86Rot)       , F(None)|F(Special)                 , EF(W____W__), 0 , 0 , O(GqdwbMem)       , O(Gb)|O(Imm)      , U                 , U                 , U                 ),
  INST(kX86InstIdRorx             , "rorx"             , O_F20F3A(F0,U,_,_,_), U                   , Enc(AvxRmi)       , F(None)                            , EF(________), 0 , 0 , O(Gqd)            , O(GqdMem)         , O(Imm)            , U                 , U                 ),
  INST(kX86InstIdRoundpd          , "roundpd"          , O_660F3A(09,U,_,_,_), U                   , Enc(ExtRmi)       , F(Move)                            , EF(________), 0 , 16, O(Xmm)            , O(XmmMem)         , O(Imm)            , U                 , U                 ),
  INST(kX86InstIdRoundps          , "roundps"          , O_660F3A(08,U,_,_,_), U                   , Enc(ExtRmi)       , F(Move)                            , EF(________), 0 , 16, O(Xmm)            , O(XmmMem)         , O(Imm)            , U                 , U                 ),
  INST(kX86InstIdRoundsd          , "roundsd"          , O_660F3A(0B,U,_,_,_), U                   , Enc(ExtRmi)       , F(Move)                            , EF(________), 0 , 8 , O(Xmm)            , O(XmmMem)         , O(Imm)            , U                 , U                 ),
  INST(kX86InstIdRoundss          , "roundss"          , O_660F3A(0A,U,_,_,_), U                   , Enc(ExtRmi)       , F(Move)                            , EF(________), 0 , 4 , O(Xmm)            , O(XmmMem)         , O(Imm)            , U                 , U                 ),
  INST(kX86InstIdRsqrtps          , "rsqrtps"          , O_000F00(52,U,_,_,_), U                   , Enc(ExtRm)        , F(Move)                            , EF(________), 0 , 16, O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdRsqrtss          , "rsqrtss"          , O_F30F00(52,U,_,_,_), U                   , Enc(ExtRm)        , F(Move)                            , EF(________), 0 , 4 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdSahf             , "sahf"             , O_000000(9E,U,_,_,_), U                   , Enc(X86Op)        , F(None)|F(Special)                 , EF(_WWWWW__), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdSal              , "sal"              , O_000000(D0,4,_,_,_), U                   , Enc(X86Rot)       , F(None)|F(Special)                 , EF(WWWUWW__), 0 , 0 , O(GqdwbMem)       , O(Gb)|O(Imm)      , U                 , U                 , U                 ),
  INST(kX86InstIdSar              , "sar"              , O_000000(D0,7,_,_,_), U                   , Enc(X86Rot)       , F(None)|F(Special)                 , EF(WWWUWW__), 0 , 0 , O(GqdwbMem)       , O(Gb)|O(Imm)      , U                 , U                 , U                 ),
  INST(kX86InstIdSarx             , "sarx"             , O_F30F38(F7,U,_,_,_), U                   , Enc(AvxRmv)       , F(None)                            , EF(________), 0 , 0 , O(Gqd)            , O(GqdMem)         , O(Gqd)            , U                 , U                 ),
  INST(kX86InstIdSbb              , "sbb"              , O_000000(18,3,_,_,_), U                   , Enc(X86Arith)     , F(Lock)                            , EF(WWWWWX__), 0 , 0 , O(GqdwbMem)       , O(GqdwbMem)|O(Imm), U                 , U                 , U                 ),
  INST(kX86InstIdScasB            , "scas_b"           , O_000000(AE,U,_,_,_), U                   , Enc(X86Op)        , F(None)|F(Special)                 , EF(WWWWWWR_), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdScasD            , "scas_d"           , O_000000(AF,U,_,_,_), U                   , Enc(X86Op)        , F(None)|F(Special)                 , EF(WWWWWWR_), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdScasQ            , "scas_q"           , O_000000(AF,U,_,W,_), U                   , Enc(X86Op)        , F(None)|F(Special)                 , EF(WWWWWWR_), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdScasW            , "scas_w"           , O_000000(AF,U,_,_,_), U                   , Enc(X86Op_66H)    , F(None)|F(Special)                 , EF(WWWWWWR_), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdSeta             , "seta"             , O_000F00(97,U,_,_,_), U                   , Enc(X86Set)       , F(Move)                            , EF(__R__R__), 0 , 1 , O(GbMem)          , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdSetae            , "setae"            , O_000F00(93,U,_,_,_), U                   , Enc(X86Set)       , F(Move)                            , EF(_____R__), 0 , 1 , O(GbMem)          , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdSetb             , "setb"             , O_000F00(92,U,_,_,_), U                   , Enc(X86Set)       , F(Move)                            , EF(_____R__), 0 , 1 , O(GbMem)          , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdSetbe            , "setbe"            , O_000F00(96,U,_,_,_), U                   , Enc(X86Set)       , F(Move)                            , EF(__R__R__), 0 , 1 , O(GbMem)          , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdSetc             , "setc"             , O_000F00(92,U,_,_,_), U                   , Enc(X86Set)       , F(Move)                            , EF(_____R__), 0 , 1 , O(GbMem)          , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdSete             , "sete"             , O_000F00(94,U,_,_,_), U                   , Enc(X86Set)       , F(Move)                            , EF(__R_____), 0 , 1 , O(GbMem)          , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdSetg             , "setg"             , O_000F00(9F,U,_,_,_), U                   , Enc(X86Set)       , F(Move)                            , EF(RRR_____), 0 , 1 , O(GbMem)          , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdSetge            , "setge"            , O_000F00(9D,U,_,_,_), U                   , Enc(X86Set)       , F(Move)                            , EF(RR______), 0 , 1 , O(GbMem)          , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdSetl             , "setl"             , O_000F00(9C,U,_,_,_), U                   , Enc(X86Set)       , F(Move)                            , EF(RR______), 0 , 1 , O(GbMem)          , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdSetle            , "setle"            , O_000F00(9E,U,_,_,_), U                   , Enc(X86Set)       , F(Move)                            , EF(RRR_____), 0 , 1 , O(GbMem)          , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdSetna            , "setna"            , O_000F00(96,U,_,_,_), U                   , Enc(X86Set)       , F(Move)                            , EF(__R__R__), 0 , 1 , O(GbMem)          , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdSetnae           , "setnae"           , O_000F00(92,U,_,_,_), U                   , Enc(X86Set)       , F(Move)                            , EF(_____R__), 0 , 1 , O(GbMem)          , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdSetnb            , "setnb"            , O_000F00(93,U,_,_,_), U                   , Enc(X86Set)       , F(Move)                            , EF(_____R__), 0 , 1 , O(GbMem)          , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdSetnbe           , "setnbe"           , O_000F00(97,U,_,_,_), U                   , Enc(X86Set)       , F(Move)                            , EF(__R__R__), 0 , 1 , O(GbMem)          , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdSetnc            , "setnc"            , O_000F00(93,U,_,_,_), U                   , Enc(X86Set)       , F(Move)                            , EF(_____R__), 0 , 1 , O(GbMem)          , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdSetne            , "setne"            , O_000F00(95,U,_,_,_), U                   , Enc(X86Set)       , F(Move)                            , EF(__R_____), 0 , 1 , O(GbMem)          , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdSetng            , "setng"            , O_000F00(9E,U,_,_,_), U                   , Enc(X86Set)       , F(Move)                            , EF(RRR_____), 0 , 1 , O(GbMem)          , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdSetnge           , "setnge"           , O_000F00(9C,U,_,_,_), U                   , Enc(X86Set)       , F(Move)                            , EF(RR______), 0 , 1 , O(GbMem)          , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdSetnl            , "setnl"            , O_000F00(9D,U,_,_,_), U                   , Enc(X86Set)       , F(Move)                            , EF(RR______), 0 , 1 , O(GbMem)          , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdSetnle           , "setnle"           , O_000F00(9F,U,_,_,_), U                   , Enc(X86Set)       , F(Move)                            , EF(RRR_____), 0 , 1 , O(GbMem)          , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdSetno            , "setno"            , O_000F00(91,U,_,_,_), U                   , Enc(X86Set)       , F(Move)                            , EF(R_______), 0 , 1 , O(GbMem)          , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdSetnp            , "setnp"            , O_000F00(9B,U,_,_,_), U                   , Enc(X86Set)       , F(Move)                            , EF(____R___), 0 , 1 , O(GbMem)          , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdSetns            , "setns"            , O_000F00(99,U,_,_,_), U                   , Enc(X86Set)       , F(Move)                            , EF(_R______), 0 , 1 , O(GbMem)          , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdSetnz            , "setnz"            , O_000F00(95,U,_,_,_), U                   , Enc(X86Set)       , F(Move)                            , EF(__R_____), 0 , 1 , O(GbMem)          , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdSeto             , "seto"             , O_000F00(90,U,_,_,_), U                   , Enc(X86Set)       , F(Move)                            , EF(R_______), 0 , 1 , O(GbMem)          , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdSetp             , "setp"             , O_000F00(9A,U,_,_,_), U                   , Enc(X86Set)       , F(Move)                            , EF(____R___), 0 , 1 , O(GbMem)          , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdSetpe            , "setpe"            , O_000F00(9A,U,_,_,_), U                   , Enc(X86Set)       , F(Move)                            , EF(____R___), 0 , 1 , O(GbMem)          , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdSetpo            , "setpo"            , O_000F00(9B,U,_,_,_), U                   , Enc(X86Set)       , F(Move)                            , EF(____R___), 0 , 1 , O(GbMem)          , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdSets             , "sets"             , O_000F00(98,U,_,_,_), U                   , Enc(X86Set)       , F(Move)                            , EF(_R______), 0 , 1 , O(GbMem)          , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdSetz             , "setz"             , O_000F00(94,U,_,_,_), U                   , Enc(X86Set)       , F(Move)                            , EF(__R_____), 0 , 1 , O(GbMem)          , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdSfence           , "sfence"           , O_000F00(AE,7,_,_,_), U                   , Enc(ExtFence)     , F(None)                            , EF(________), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdShl              , "shl"              , O_000000(D0,4,_,_,_), U                   , Enc(X86Rot)       , F(None)|F(Special)                 , EF(WWWUWW__), 0 , 0 , O(GqdwbMem)       , O(Gb)|O(Imm)      , U                 , U                 , U                 ),
  INST(kX86InstIdShld             , "shld"             , O_000F00(A4,U,_,_,_), U                   , Enc(X86Shlrd)     , F(None)|F(Special)                 , EF(UWWUWW__), 0 , 0 , O(GqdwbMem)       , O(Gb)             , U                 , U                 , U                 ),
  INST(kX86InstIdShlx             , "shlx"             , O_660F38(F7,U,_,_,_), U                   , Enc(AvxRmv)       , F(None)                            , EF(________), 0 , 0 , O(Gqd)            , O(GqdMem)         , O(Gqd)            , U                 , U                 ),
  INST(kX86InstIdShr              , "shr"              , O_000000(D0,5,_,_,_), U                   , Enc(X86Rot)       , F(None)|F(Special)                 , EF(WWWUWW__), 0 , 0 , O(GqdwbMem)       , O(Gb)|O(Imm)      , U                 , U                 , U                 ),
  INST(kX86InstIdShrd             , "shrd"             , O_000F00(AC,U,_,_,_), U                   , Enc(X86Shlrd)     , F(None)|F(Special)                 , EF(UWWUWW__), 0 , 0 , O(GqdwbMem)       , O(Gqdwb)          , U                 , U                 , U                 ),
  INST(kX86InstIdShrx             , "shrx"             , O_F20F38(F7,U,_,_,_), U                   , Enc(AvxRmv)       , F(None)                            , EF(________), 0 , 0 , O(Gqd)            , O(GqdMem)         , O(Gqd)            , U                 , U                 ),
  INST(kX86InstIdShufpd           , "shufpd"           , O_660F00(C6,U,_,_,_), U                   , Enc(ExtRmi)       , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , O(Imm)            , U                 , U                 ),
  INST(kX86InstIdShufps           , "shufps"           , O_000F00(C6,U,_,_,_), U                   , Enc(ExtRmi)       , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , O(Imm)            , U                 , U                 ),
  INST(kX86InstIdSqrtpd           , "sqrtpd"           , O_660F00(51,U,_,_,_), U                   , Enc(ExtRm)        , F(Move)                            , EF(________), 0 , 16, O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdSqrtps           , "sqrtps"           , O_000F00(51,U,_,_,_), U                   , Enc(ExtRm)        , F(Move)                            , EF(________), 0 , 16, O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdSqrtsd           , "sqrtsd"           , O_F20F00(51,U,_,_,_), U                   , Enc(ExtRm)        , F(Move)                            , EF(________), 0 , 8 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdSqrtss           , "sqrtss"           , O_F30F00(51,U,_,_,_), U                   , Enc(ExtRm)        , F(Move)                            , EF(________), 0 , 4 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdStc              , "stc"              , O_000000(F9,U,_,_,_), U                   , Enc(X86Op)        , F(None)                            , EF(_____W__), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdStd              , "std"              , O_000000(FD,U,_,_,_), U                   , Enc(X86Op)        , F(None)                            , EF(______W_), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdStmxcsr          , "stmxcsr"          , O_000F00(AE,3,_,_,_), U                   , Enc(X86M)         , F(None)                            , EF(________), 0 , 0 , O(Mem)            , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdStosB            , "stos_b"           , O_000000(AA,U,_,_,_), U                   , Enc(X86Op)        , F(None)|F(Special)                 , EF(______R_), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdStosD            , "stos_d"           , O_000000(AB,U,_,_,_), U                   , Enc(X86Op)        , F(None)|F(Special)                 , EF(______R_), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdStosQ            , "stos_q"           , O_000000(AB,U,_,W,_), U                   , Enc(X86Op)        , F(None)|F(Special)                 , EF(______R_), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdStosW            , "stos_w"           , O_000000(AB,U,_,_,_), U                   , Enc(X86Op_66H)    , F(None)|F(Special)                 , EF(______R_), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdSub              , "sub"              , O_000000(28,5,_,_,_), U                   , Enc(X86Arith)     , F(Lock)                            , EF(WWWWWW__), 0 , 0 , O(GqdwbMem)       , O(GqdwbMem)|O(Imm), U                 , U                 , U                 ),
  INST(kX86InstIdSubpd            , "subpd"            , O_660F00(5C,U,_,_,_), U                   , Enc(ExtRm)        , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdSubps            , "subps"            , O_000F00(5C,U,_,_,_), U                   , Enc(ExtRm)        , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdSubsd            , "subsd"            , O_F20F00(5C,U,_,_,_), U                   , Enc(ExtRm)        , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdSubss            , "subss"            , O_F30F00(5C,U,_,_,_), U                   , Enc(ExtRm)        , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdTest             , "test"             , O_000000(84,U,_,_,_), O_000000(F6,U,_,_,_), Enc(X86Test)      , F(Test)                            , EF(WWWUWW__), 0 , 0 , O(GqdwbMem)       , O(Gqdwb)|O(Imm)   , U                 , U                 , U                 ),
  INST(kX86InstIdTzcnt            , "tzcnt"            , O_F30F00(BC,U,_,_,_), U                   , Enc(X86RegRm)     , F(Move)                            , EF(UUWUUW__), 0 , 0 , O(Gqdw)           , O(GqdwMem)        , U                 , U                 , U                 ),
  INST(kX86InstIdUcomisd          , "ucomisd"          , O_660F00(2E,U,_,_,_), U                   , Enc(ExtRm)        , F(Test)                            , EF(WWWWWW__), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdUcomiss          , "ucomiss"          , O_000F00(2E,U,_,_,_), U                   , Enc(ExtRm)        , F(Test)                            , EF(WWWWWW__), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdUd2              , "ud2"              , O_000F00(0B,U,_,_,_), U                   , Enc(X86Op)        , F(None)                            , EF(________), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdUnpckhpd         , "unpckhpd"         , O_660F00(15,U,_,_,_), U                   , Enc(ExtRm)        , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdUnpckhps         , "unpckhps"         , O_000F00(15,U,_,_,_), U                   , Enc(ExtRm)        , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdUnpcklpd         , "unpcklpd"         , O_660F00(14,U,_,_,_), U                   , Enc(ExtRm)        , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdUnpcklps         , "unpcklps"         , O_000F00(14,U,_,_,_), U                   , Enc(ExtRm)        , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdVaddpd           , "vaddpd"           , O_660F00(58,U,_,I,1), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVaddps           , "vaddps"           , O_000F00(58,U,_,I,0), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVaddsd           , "vaddsd"           , O_F20F00(58,U,0,I,1), U                   , Enc(AvxRvm)       , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(Xmm)            , O(XmmMem)         , U                 , U                 ),
  INST(kX86InstIdVaddss           , "vaddss"           , O_F30F00(58,U,0,I,0), U                   , Enc(AvxRvm)       , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(Xmm)            , O(XmmMem)         , U                 , U                 ),
  INST(kX86InstIdVaddsubpd        , "vaddsubpd"        , O_660F00(D0,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVaddsubps        , "vaddsubps"        , O_F20F00(D0,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVaesdec          , "vaesdec"          , O_660F38(DE,U,_,_,_), U                   , Enc(AvxRvm)       , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(Xmm)            , O(XmmMem)         , U                 , U                 ),
  INST(kX86InstIdVaesdeclast      , "vaesdeclast"      , O_660F38(DF,U,_,_,_), U                   , Enc(AvxRvm)       , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(Xmm)            , O(XmmMem)         , U                 , U                 ),
  INST(kX86InstIdVaesenc          , "vaesenc"          , O_660F38(DC,U,_,_,_), U                   , Enc(AvxRvm)       , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(Xmm)            , O(XmmMem)         , U                 , U                 ),
  INST(kX86InstIdVaesenclast      , "vaesenclast"      , O_660F38(DD,U,_,_,_), U                   , Enc(AvxRvm)       , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(Xmm)            , O(XmmMem)         , U                 , U                 ),
  INST(kX86InstIdVaesimc          , "vaesimc"          , O_660F38(DB,U,_,_,_), U                   , Enc(AvxRm)        , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdVaeskeygenassist , "vaeskeygenassist" , O_660F3A(DF,U,_,_,_), U                   , Enc(AvxRmi)       , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , O(Imm)            , U                 , U                 ),
  INST(kX86InstIdVandnpd          , "vandnpd"          , O_660F00(55,U,_,_,1), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVandnps          , "vandnps"          , O_000F00(55,U,_,_,0), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVandpd           , "vandpd"           , O_660F00(54,U,_,_,1), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVandps           , "vandps"           , O_000F00(54,U,_,_,0), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVblendpd         , "vblendpd"         , O_660F3A(0D,U,_,_,_), U                   , Enc(AvxRvmi_P)    , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , O(Imm)            , U                 ),
  INST(kX86InstIdVblendps         , "vblendps"         , O_660F3A(0C,U,_,_,_), U                   , Enc(AvxRvmi_P)    , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , O(Imm)            , U                 ),
  INST(kX86InstIdVblendvpd        , "vblendvpd"        , O_660F3A(4B,U,_,_,_), U                   , Enc(AvxRvmr_P)    , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , O(Xy)             , U                 ),
  INST(kX86InstIdVblendvps        , "vblendvps"        , O_660F3A(4A,U,_,_,_), U                   , Enc(AvxRvmr_P)    , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , O(Xy)             , U                 ),
  INST(kX86InstIdVbroadcastf128   , "vbroadcastf128"   , O_660F38(1A,U,L,_,_), U                   , Enc(AvxRm)        , F(Avx)                             , EF(________), 0 , 0 , O(Ymm)            , O(Mem)            , U                 , U                 , U                 ),
  INST(kX86InstIdVbroadcasti128   , "vbroadcasti128"   , O_660F38(5A,U,L,_,_), U                   , Enc(AvxRm)        , F(Avx)                             , EF(________), 0 , 0 , O(Ymm)            , O(Mem)            , U                 , U                 , U                 ),
  INST(kX86InstIdVbroadcastsd     , "vbroadcastsd"     , O_660F38(19,U,L,0,1), U                   , Enc(AvxRm)        , F(Avx)                             , EF(________), 0 , 0 , O(Ymm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdVbroadcastss     , "vbroadcastss"     , O_660F38(18,U,_,0,0), U                   , Enc(AvxRm_P)      , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdVcmppd           , "vcmppd"           , O_660F00(C2,U,_,_,_), U                   , Enc(AvxRvmi_P)    , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , O(Imm)            , U                 ),
  INST(kX86InstIdVcmpps           , "vcmpps"           , O_000F00(C2,U,_,_,_), U                   , Enc(AvxRvmi_P)    , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , O(Imm)            , U                 ),
  INST(kX86InstIdVcmpsd           , "vcmpsd"           , O_F20F00(C2,U,_,_,_), U                   , Enc(AvxRvmi)      , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(Xmm)            , O(XmmMem)         , O(Imm)            , U                 ),
  INST(kX86InstIdVcmpss           , "vcmpss"           , O_F30F00(C2,U,_,_,_), U                   , Enc(AvxRvmi)      , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(Xmm)            , O(XmmMem)         , O(Imm)            , U                 ),
  INST(kX86InstIdVcomisd          , "vcomisd"          , O_660F00(2F,U,_,_,_), U                   , Enc(AvxRm)        , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdVcomiss          , "vcomiss"          , O_000F00(2F,U,_,_,_), U                   , Enc(AvxRm)        , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdVcvtdq2pd        , "vcvtdq2pd"        , O_F30F00(E6,U,_,_,_), U                   , Enc(AvxRm_P)      , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdVcvtdq2ps        , "vcvtdq2ps"        , O_000F00(5B,U,_,_,_), U                   , Enc(AvxRm_P)      , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(XyMem)          , U                 , U                 , U                 ),
  INST(kX86InstIdVcvtpd2dq        , "vcvtpd2dq"        , O_F20F00(E6,U,_,_,_), U                   , Enc(AvxRm)        , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(XyMem)          , U                 , U                 , U                 ),
  INST(kX86InstIdVcvtpd2ps        , "vcvtpd2ps"        , O_660F00(5A,U,_,_,_), U                   , Enc(AvxRm)        , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(XyMem)          , U                 , U                 , U                 ),
  INST(kX86InstIdVcvtph2ps        , "vcvtph2ps"        , O_660F38(13,U,_,_,_), U                   , Enc(AvxRm_P)      , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdVcvtps2dq        , "vcvtps2dq"        , O_660F00(5B,U,_,_,_), U                   , Enc(AvxRm_P)      , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(XyMem)          , U                 , U                 , U                 ),
  INST(kX86InstIdVcvtps2pd        , "vcvtps2pd"        , O_000F00(5A,U,_,_,_), U                   , Enc(AvxRm_P)      , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdVcvtps2ph        , "vcvtps2ph"        , O_660F3A(1D,U,_,_,_), U                   , Enc(AvxMri_P)     , F(Avx)                             , EF(________), 0 , 0 , O(XmmMem)         , O(Xy)             , O(Imm)            , U                 , U                 ),
  INST(kX86InstIdVcvtsd2si        , "vcvtsd2si"        , O_F20F00(2D,U,_,_,_), U                   , Enc(AvxRm)        , F(Avx)                             , EF(________), 0 , 0 , O(Gqd)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdVcvtsd2ss        , "vcvtsd2ss"        , O_F20F00(5A,U,_,_,_), U                   , Enc(AvxRvm)       , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(Xmm)            , O(XmmMem)         , U                 , U                 ),
  INST(kX86InstIdVcvtsi2sd        , "vcvtsi2sd"        , O_F20F00(2A,U,_,_,_), U                   , Enc(AvxRvm)       , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(Xmm)            , O(GqdMem)         , U                 , U                 ),
  INST(kX86InstIdVcvtsi2ss        , "vcvtsi2ss"        , O_F30F00(2A,U,_,_,_), U                   , Enc(AvxRvm)       , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(Xmm)            , O(GqdMem)         , U                 , U                 ),
  INST(kX86InstIdVcvtss2sd        , "vcvtss2sd"        , O_F30F00(5A,U,_,_,_), U                   , Enc(AvxRvm)       , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(Xmm)            , O(XmmMem)         , U                 , U                 ),
  INST(kX86InstIdVcvtss2si        , "vcvtss2si"        , O_F20F00(2D,U,_,_,_), U                   , Enc(AvxRm)        , F(Avx)                             , EF(________), 0 , 0 , O(Gqd)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdVcvttpd2dq       , "vcvttpd2dq"       , O_660F00(E6,U,_,_,_), U                   , Enc(AvxRm_P)      , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(XyMem)          , U                 , U                 , U                 ),
  INST(kX86InstIdVcvttps2dq       , "vcvttps2dq"       , O_F30F00(5B,U,_,_,_), U                   , Enc(AvxRm_P)      , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(XyMem)          , U                 , U                 , U                 ),
  INST(kX86InstIdVcvttsd2si       , "vcvttsd2si"       , O_F20F00(2C,U,_,_,_), U                   , Enc(AvxRm)        , F(Avx)                             , EF(________), 0 , 0 , O(Gqd)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdVcvttss2si       , "vcvttss2si"       , O_F30F00(2C,U,_,_,_), U                   , Enc(AvxRm)        , F(Avx)                             , EF(________), 0 , 0 , O(Gqd)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdVdivpd           , "vdivpd"           , O_660F00(5E,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVdivps           , "vdivps"           , O_000F00(5E,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVdivsd           , "vdivsd"           , O_F20F00(5E,U,_,_,_), U                   , Enc(AvxRvm)       , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(Xmm)            , O(XmmMem)         , U                 , U                 ),
  INST(kX86InstIdVdivss           , "vdivss"           , O_F30F00(5E,U,_,_,_), U                   , Enc(AvxRvm)       , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(Xmm)            , O(XmmMem)         , U                 , U                 ),
  INST(kX86InstIdVdppd            , "vdppd"            , O_660F3A(41,U,_,_,_), U                   , Enc(AvxRvmi)      , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(Xmm)            , O(XmmMem)         , O(Imm)            , U                 ),
  INST(kX86InstIdVdpps            , "vdpps"            , O_660F3A(40,U,_,_,_), U                   , Enc(AvxRvmi_P)    , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , O(Imm)            , U                 ),
  INST(kX86InstIdVextractf128     , "vextractf128"     , O_660F3A(19,U,L,_,_), U                   , Enc(AvxMri)       , F(Avx)                             , EF(________), 0 , 0 , O(XmmMem)         , O(Ymm)            , O(Imm)            , U                 , U                 ),
  INST(kX86InstIdVextracti128     , "vextracti128"     , O_660F3A(39,U,L,_,_), U                   , Enc(AvxMri)       , F(Avx)                             , EF(________), 0 , 0 , O(XmmMem)         , O(Ymm)            , O(Imm)            , U                 , U                 ),
  INST(kX86InstIdVextractps       , "vextractps"       , O_660F3A(17,U,_,_,_), U                   , Enc(AvxMri)       , F(Avx)                             , EF(________), 0 , 0 , O(GqdMem)         , O(Xmm)            , O(Imm)            , U                 , U                 ),
  INST(kX86InstIdVfmadd132pd      , "vfmadd132pd"      , O_660F38(98,U,_,W,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVfmadd132ps      , "vfmadd132ps"      , O_660F38(98,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVfmadd132sd      , "vfmadd132sd"      , O_660F38(99,U,_,W,_), U                   , Enc(AvxRvm)       , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(Xmm)            , O(XmmMem)         , U                 , U                 ),
  INST(kX86InstIdVfmadd132ss      , "vfmadd132ss"      , O_660F38(99,U,_,_,_), U                   , Enc(AvxRvm)       , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(Xmm)            , O(XmmMem)         , U                 , U                 ),
  INST(kX86InstIdVfmadd213pd      , "vfmadd213pd"      , O_660F38(A8,U,_,W,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVfmadd213ps      , "vfmadd213ps"      , O_660F38(A8,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVfmadd213sd      , "vfmadd213sd"      , O_660F38(A9,U,_,W,_), U                   , Enc(AvxRvm)       , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(Xmm)            , O(XmmMem)         , U                 , U                 ),
  INST(kX86InstIdVfmadd213ss      , "vfmadd213ss"      , O_660F38(A9,U,_,_,_), U                   , Enc(AvxRvm)       , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(Xmm)            , O(XmmMem)         , U                 , U                 ),
  INST(kX86InstIdVfmadd231pd      , "vfmadd231pd"      , O_660F38(B8,U,_,W,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVfmadd231ps      , "vfmadd231ps"      , O_660F38(B8,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVfmadd231sd      , "vfmadd231sd"      , O_660F38(B9,U,_,W,_), U                   , Enc(AvxRvm)       , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(Xmm)            , O(XmmMem)         , U                 , U                 ),
  INST(kX86InstIdVfmadd231ss      , "vfmadd231ss"      , O_660F38(B9,U,_,_,_), U                   , Enc(AvxRvm)       , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(Xmm)            , O(XmmMem)         , U                 , U                 ),
  INST(kX86InstIdVfmaddpd         , "vfmaddpd"         , O_660F3A(69,U,_,_,_), U                   , Enc(Fma4_P)       , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , O(XyMem)          , U                 ),
  INST(kX86InstIdVfmaddps         , "vfmaddps"         , O_660F3A(68,U,_,_,_), U                   , Enc(Fma4_P)       , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , O(XyMem)          , U                 ),
  INST(kX86InstIdVfmaddsd         , "vfmaddsd"         , O_660F3A(6B,U,_,_,_), U                   , Enc(Fma4)         , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(Xmm)            , O(XmmMem)         , O(XmmMem)         , U                 ),
  INST(kX86InstIdVfmaddss         , "vfmaddss"         , O_660F3A(6A,U,_,_,_), U                   , Enc(Fma4)         , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(Xmm)            , O(XmmMem)         , O(XmmMem)         , U                 ),
  INST(kX86InstIdVfmaddsub132pd   , "vfmaddsub132pd"   , O_660F38(96,U,_,W,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVfmaddsub132ps   , "vfmaddsub132ps"   , O_660F38(96,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVfmaddsub213pd   , "vfmaddsub213pd"   , O_660F38(A6,U,_,W,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVfmaddsub213ps   , "vfmaddsub213ps"   , O_660F38(A6,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVfmaddsub231pd   , "vfmaddsub231pd"   , O_660F38(B6,U,_,W,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVfmaddsub231ps   , "vfmaddsub231ps"   , O_660F38(B6,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVfmaddsubpd      , "vfmaddsubpd"      , O_660F3A(5D,U,_,_,_), U                   , Enc(Fma4_P)       , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , O(XyMem)          , U                 ),
  INST(kX86InstIdVfmaddsubps      , "vfmaddsubps"      , O_660F3A(5C,U,_,_,_), U                   , Enc(Fma4_P)       , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , O(XyMem)          , U                 ),
  INST(kX86InstIdVfmsub132pd      , "vfmsub132pd"      , O_660F38(9A,U,_,W,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVfmsub132ps      , "vfmsub132ps"      , O_660F38(9A,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVfmsub132sd      , "vfmsub132sd"      , O_660F38(9B,U,_,W,_), U                   , Enc(AvxRvm)       , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(Xmm)            , O(XmmMem)         , U                 , U                 ),
  INST(kX86InstIdVfmsub132ss      , "vfmsub132ss"      , O_660F38(9B,U,_,_,_), U                   , Enc(AvxRvm)       , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(Xmm)            , O(XmmMem)         , U                 , U                 ),
  INST(kX86InstIdVfmsub213pd      , "vfmsub213pd"      , O_660F38(AA,U,_,W,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVfmsub213ps      , "vfmsub213ps"      , O_660F38(AA,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVfmsub213sd      , "vfmsub213sd"      , O_660F38(AB,U,_,W,_), U                   , Enc(AvxRvm)       , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(Xmm)            , O(XmmMem)         , U                 , U                 ),
  INST(kX86InstIdVfmsub213ss      , "vfmsub213ss"      , O_660F38(AB,U,_,_,_), U                   , Enc(AvxRvm)       , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(Xmm)            , O(XmmMem)         , U                 , U                 ),
  INST(kX86InstIdVfmsub231pd      , "vfmsub231pd"      , O_660F38(BA,U,_,W,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVfmsub231ps      , "vfmsub231ps"      , O_660F38(BA,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVfmsub231sd      , "vfmsub231sd"      , O_660F38(BB,U,_,W,_), U                   , Enc(AvxRvm)       , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(Xmm)            , O(XmmMem)         , U                 , U                 ),
  INST(kX86InstIdVfmsub231ss      , "vfmsub231ss"      , O_660F38(BB,U,_,_,_), U                   , Enc(AvxRvm)       , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(Xmm)            , O(XmmMem)         , U                 , U                 ),
  INST(kX86InstIdVfmsubadd132pd   , "vfmsubadd132pd"   , O_660F38(97,U,_,W,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVfmsubadd132ps   , "vfmsubadd132ps"   , O_660F38(97,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVfmsubadd213pd   , "vfmsubadd213pd"   , O_660F38(A7,U,_,W,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVfmsubadd213ps   , "vfmsubadd213ps"   , O_660F38(A7,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVfmsubadd231pd   , "vfmsubadd231pd"   , O_660F38(B7,U,_,W,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVfmsubadd231ps   , "vfmsubadd231ps"   , O_660F38(B7,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVfmsubaddpd      , "vfmsubaddpd"      , O_660F3A(5F,U,_,_,_), U                   , Enc(Fma4_P)       , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , O(XyMem)          , U                 ),
  INST(kX86InstIdVfmsubaddps      , "vfmsubaddps"      , O_660F3A(5E,U,_,_,_), U                   , Enc(Fma4_P)       , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , O(XyMem)          , U                 ),
  INST(kX86InstIdVfmsubpd         , "vfmsubpd"         , O_660F3A(6D,U,_,_,_), U                   , Enc(Fma4_P)       , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , O(XyMem)          , U                 ),
  INST(kX86InstIdVfmsubps         , "vfmsubps"         , O_660F3A(6C,U,_,_,_), U                   , Enc(Fma4_P)       , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , O(XyMem)          , U                 ),
  INST(kX86InstIdVfmsubsd         , "vfmsubsd"         , O_660F3A(6F,U,_,_,_), U                   , Enc(Fma4)         , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(Xmm)            , O(XmmMem)         , O(XmmMem)         , U                 ),
  INST(kX86InstIdVfmsubss         , "vfmsubss"         , O_660F3A(6E,U,_,_,_), U                   , Enc(Fma4)         , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(Xmm)            , O(XmmMem)         , O(XmmMem)         , U                 ),
  INST(kX86InstIdVfnmadd132pd     , "vfnmadd132pd"     , O_660F38(9C,U,_,W,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVfnmadd132ps     , "vfnmadd132ps"     , O_660F38(9C,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVfnmadd132sd     , "vfnmadd132sd"     , O_660F38(9D,U,_,W,_), U                   , Enc(AvxRvm)       , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(Xmm)            , O(XmmMem)         , U                 , U                 ),
  INST(kX86InstIdVfnmadd132ss     , "vfnmadd132ss"     , O_660F38(9D,U,_,_,_), U                   , Enc(AvxRvm)       , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(Xmm)            , O(XmmMem)         , U                 , U                 ),
  INST(kX86InstIdVfnmadd213pd     , "vfnmadd213pd"     , O_660F38(AC,U,_,W,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVfnmadd213ps     , "vfnmadd213ps"     , O_660F38(AC,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVfnmadd213sd     , "vfnmadd213sd"     , O_660F38(AD,U,_,W,_), U                   , Enc(AvxRvm)       , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(Xmm)            , O(XmmMem)         , U                 , U                 ),
  INST(kX86InstIdVfnmadd213ss     , "vfnmadd213ss"     , O_660F38(AD,U,_,_,_), U                   , Enc(AvxRvm)       , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(Xmm)            , O(XmmMem)         , U                 , U                 ),
  INST(kX86InstIdVfnmadd231pd     , "vfnmadd231pd"     , O_660F38(BC,U,_,W,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVfnmadd231ps     , "vfnmadd231ps"     , O_660F38(BC,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVfnmadd231sd     , "vfnmadd231sd"     , O_660F38(BC,U,_,W,_), U                   , Enc(AvxRvm)       , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(Xmm)            , O(XmmMem)         , U                 , U                 ),
  INST(kX86InstIdVfnmadd231ss     , "vfnmadd231ss"     , O_660F38(BC,U,_,_,_), U                   , Enc(AvxRvm)       , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(Xmm)            , O(XmmMem)         , U                 , U                 ),
  INST(kX86InstIdVfnmaddpd        , "vfnmaddpd"        , O_660F3A(79,U,_,_,_), U                   , Enc(Fma4_P)       , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , O(XyMem)          , U                 ),
  INST(kX86InstIdVfnmaddps        , "vfnmaddps"        , O_660F3A(78,U,_,_,_), U                   , Enc(Fma4_P)       , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , O(XyMem)          , U                 ),
  INST(kX86InstIdVfnmaddsd        , "vfnmaddsd"        , O_660F3A(7B,U,_,_,_), U                   , Enc(Fma4)         , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(Xmm)            , O(XmmMem)         , O(XmmMem)         , U                 ),
  INST(kX86InstIdVfnmaddss        , "vfnmaddss"        , O_660F3A(7A,U,_,_,_), U                   , Enc(Fma4)         , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(Xmm)            , O(XmmMem)         , O(XmmMem)         , U                 ),
  INST(kX86InstIdVfnmsub132pd     , "vfnmsub132pd"     , O_660F38(9E,U,_,W,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVfnmsub132ps     , "vfnmsub132ps"     , O_660F38(9E,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVfnmsub132sd     , "vfnmsub132sd"     , O_660F38(9F,U,_,W,_), U                   , Enc(AvxRvm)       , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(Xmm)            , O(XmmMem)         , U                 , U                 ),
  INST(kX86InstIdVfnmsub132ss     , "vfnmsub132ss"     , O_660F38(9F,U,_,_,_), U                   , Enc(AvxRvm)       , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(Xmm)            , O(XmmMem)         , U                 , U                 ),
  INST(kX86InstIdVfnmsub213pd     , "vfnmsub213pd"     , O_660F38(AE,U,_,W,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVfnmsub213ps     , "vfnmsub213ps"     , O_660F38(AE,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVfnmsub213sd     , "vfnmsub213sd"     , O_660F38(AF,U,_,W,_), U                   , Enc(AvxRvm)       , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(Xmm)            , O(XmmMem)         , U                 , U                 ),
  INST(kX86InstIdVfnmsub213ss     , "vfnmsub213ss"     , O_660F38(AF,U,_,_,_), U                   , Enc(AvxRvm)       , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(Xmm)            , O(XmmMem)         , U                 , U                 ),
  INST(kX86InstIdVfnmsub231pd     , "vfnmsub231pd"     , O_660F38(BE,U,_,W,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVfnmsub231ps     , "vfnmsub231ps"     , O_660F38(BE,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVfnmsub231sd     , "vfnmsub231sd"     , O_660F38(BF,U,_,W,_), U                   , Enc(AvxRvm)       , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(Xmm)            , O(XmmMem)         , U                 , U                 ),
  INST(kX86InstIdVfnmsub231ss     , "vfnmsub231ss"     , O_660F38(BF,U,_,_,_), U                   , Enc(AvxRvm)       , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(Xmm)            , O(XmmMem)         , U                 , U                 ),
  INST(kX86InstIdVfnmsubpd        , "vfnmsubpd"        , O_660F3A(7D,U,_,_,_), U                   , Enc(Fma4_P)       , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , O(XyMem)          , U                 ),
  INST(kX86InstIdVfnmsubps        , "vfnmsubps"        , O_660F3A(7C,U,_,_,_), U                   , Enc(Fma4_P)       , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , O(XyMem)          , U                 ),
  INST(kX86InstIdVfnmsubsd        , "vfnmsubsd"        , O_660F3A(7F,U,_,_,_), U                   , Enc(Fma4)         , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(Xmm)            , O(XmmMem)         , O(XmmMem)         , U                 ),
  INST(kX86InstIdVfnmsubss        , "vfnmsubss"        , O_660F3A(7E,U,_,_,_), U                   , Enc(Fma4)         , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(Xmm)            , O(XmmMem)         , O(XmmMem)         , U                 ),
  INST(kX86InstIdVfrczpd          , "vfrczpd"          , O_00_M09(81,U,_,_,_), U                   , Enc(XopRm_P)      , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(XyMem)          , U                 , U                 , U                 ),
  INST(kX86InstIdVfrczps          , "vfrczps"          , O_00_M09(80,U,_,_,_), U                   , Enc(XopRm_P)      , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(XyMem)          , U                 , U                 , U                 ),
  INST(kX86InstIdVfrczsd          , "vfrczsd"          , O_00_M09(83,U,_,_,_), U                   , Enc(XopRm)        , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdVfrczss          , "vfrczss"          , O_00_M09(82,U,_,_,_), U                   , Enc(XopRm)        , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdVgatherdpd       , "vgatherdpd"       , O_660F38(92,U,_,W,_), U                   , Enc(AvxGather)    , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Mem)            , O(Xy)             , U                 , U                 ),
  INST(kX86InstIdVgatherdps       , "vgatherdps"       , O_660F38(92,U,_,_,_), U                   , Enc(AvxGather)    , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Mem)            , O(Xy)             , U                 , U                 ),
  INST(kX86InstIdVgatherqpd       , "vgatherqpd"       , O_660F38(93,U,_,W,_), U                   , Enc(AvxGather)    , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Mem)            , O(Xy)             , U                 , U                 ),
  INST(kX86InstIdVgatherqps       , "vgatherqps"       , O_660F38(93,U,_,_,_), U                   , Enc(AvxGatherEx)  , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(Mem)            , O(Xmm)            , U                 , U                 ),
  INST(kX86InstIdVhaddpd          , "vhaddpd"          , O_660F00(7C,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVhaddps          , "vhaddps"          , O_F20F00(7C,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVhsubpd          , "vhsubpd"          , O_660F00(7D,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVhsubps          , "vhsubps"          , O_F20F00(7D,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVinsertf128      , "vinsertf128"      , O_660F3A(18,U,L,_,_), U                   , Enc(AvxRvmi)      , F(Avx)                             , EF(________), 0 , 0 , O(Ymm)            , O(Ymm)            , O(XmmMem)         , O(Imm)            , U                 ),
  INST(kX86InstIdVinserti128      , "vinserti128"      , O_660F3A(38,U,L,_,_), U                   , Enc(AvxRvmi)      , F(Avx)                             , EF(________), 0 , 0 , O(Ymm)            , O(Ymm)            , O(XmmMem)         , O(Imm)            , U                 ),
  INST(kX86InstIdVinsertps        , "vinsertps"        , O_660F3A(21,U,_,_,_), U                   , Enc(AvxRvmi)      , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(Xmm)            , O(XmmMem)         , O(Imm)            , U                 ),
  INST(kX86InstIdVlddqu           , "vlddqu"           , O_F20F00(F0,U,_,_,_), U                   , Enc(AvxRm_P)      , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Mem)            , U                 , U                 , U                 ),
  INST(kX86InstIdVldmxcsr         , "vldmxcsr"         , O_000F00(AE,2,_,_,_), U                   , Enc(AvxM)         , F(Avx)                             , EF(________), 0 , 0 , O(Mem)            , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdVmaskmovdqu      , "vmaskmovdqu"      , O_660F00(F7,U,_,_,_), U                   , Enc(AvxRm)        , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(Xmm)            , U                 , U                 , U                 ),
  INST(kX86InstIdVmaskmovpd       , "vmaskmovpd"       , O_660F38(2D,U,_,_,_), O_660F38(2F,U,_,_,_), Enc(AvxRvmMvr_P)  , F(Avx)                             , EF(________), 0 , 0 , O(XyMem)          , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVmaskmovps       , "vmaskmovps"       , O_660F38(2C,U,_,_,_), O_660F38(2E,U,_,_,_), Enc(AvxRvmMvr_P)  , F(Avx)                             , EF(________), 0 , 0 , O(XyMem)          , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVmaxpd           , "vmaxpd"           , O_660F00(5F,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVmaxps           , "vmaxps"           , O_000F00(5F,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVmaxsd           , "vmaxsd"           , O_F20F00(5F,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVmaxss           , "vmaxss"           , O_F30F00(5F,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVminpd           , "vminpd"           , O_660F00(5D,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVminps           , "vminps"           , O_000F00(5D,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVminsd           , "vminsd"           , O_F20F00(5D,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVminss           , "vminss"           , O_F30F00(5D,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVmovapd          , "vmovapd"          , O_660F00(28,U,_,_,_), O_660F00(29,U,_,_,_), Enc(AvxRmMr_P)    , F(Avx)                             , EF(________), 0 , 0 , O(XyMem)          , O(XyMem)          , U                 , U                 , U                 ),
  INST(kX86InstIdVmovaps          , "vmovaps"          , O_000F00(28,U,_,_,_), O_000F00(29,U,_,_,_), Enc(AvxRmMr_P)    , F(Avx)                             , EF(________), 0 , 0 , O(XyMem)          , O(XyMem)          , U                 , U                 , U                 ),
  INST(kX86InstIdVmovd            , "vmovd"            , O_660F00(6E,U,_,_,_), O_660F00(7E,U,_,_,_), Enc(AvxMovDQ)     , F(Avx)                             , EF(________), 0 , 0 , O(XmmMem)         , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdVmovddup         , "vmovddup"         , O_F20F00(12,U,_,_,_), U                   , Enc(AvxRm_P)      , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(XyMem)          , U                 , U                 , U                 ),
  INST(kX86InstIdVmovdqa          , "vmovdqa"          , O_660F00(6F,U,_,_,_), O_660F00(7F,U,_,_,_), Enc(AvxRmMr_P)    , F(Avx)                             , EF(________), 0 , 0 , O(XyMem)          , O(XyMem)          , U                 , U                 , U                 ),
  INST(kX86InstIdVmovdqu          , "vmovdqu"          , O_F30F00(6F,U,_,_,_), O_F30F00(7F,U,_,_,_), Enc(AvxRmMr_P)    , F(Avx)                             , EF(________), 0 , 0 , O(XyMem)          , O(XyMem)          , U                 , U                 , U                 ),
  INST(kX86InstIdVmovhlps         , "vmovhlps"         , O_000F00(12,U,_,_,_), U                   , Enc(AvxRvm)       , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(Xmm)            , O(Xmm)            , U                 , U                 ),
  INST(kX86InstIdVmovhpd          , "vmovhpd"          , O_660F00(16,U,_,_,_), O_660F00(17,U,_,_,_), Enc(AvxRvmMr)     , F(Avx)                             , EF(________), 0 , 0 , O(XmmMem)         , O(Xmm)            , O(Mem)            , U                 , U                 ),
  INST(kX86InstIdVmovhps          , "vmovhps"          , O_000F00(16,U,_,_,_), O_000F00(17,U,_,_,_), Enc(AvxRvmMr)     , F(Avx)                             , EF(________), 0 , 0 , O(XmmMem)         , O(Xmm)            , O(Mem)            , U                 , U                 ),
  INST(kX86InstIdVmovlhps         , "vmovlhps"         , O_000F00(16,U,_,_,_), U                   , Enc(AvxRvm)       , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(Xmm)            , O(Xmm)            , U                 , U                 ),
  INST(kX86InstIdVmovlpd          , "vmovlpd"          , O_660F00(12,U,_,_,_), O_660F00(13,U,_,_,_), Enc(AvxRvmMr)     , F(Avx)                             , EF(________), 0 , 0 , O(XmmMem)         , O(Xmm)            , O(Mem)            , U                 , U                 ),
  INST(kX86InstIdVmovlps          , "vmovlps"          , O_000F00(12,U,_,_,_), O_000F00(13,U,_,_,_), Enc(AvxRvmMr)     , F(Avx)                             , EF(________), 0 , 0 , O(XmmMem)         , O(Xmm)            , O(Mem)            , U                 , U                 ),
  INST(kX86InstIdVmovmskpd        , "vmovmskpd"        , O_660F00(50,U,_,_,_), U                   , Enc(AvxRm_P)      , F(Avx)                             , EF(________), 0 , 0 , O(Gqd)            , O(Xy)             , U                 , U                 , U                 ),
  INST(kX86InstIdVmovmskps        , "vmovmskps"        , O_000F00(50,U,_,_,_), U                   , Enc(AvxRm_P)      , F(Avx)                             , EF(________), 0 , 0 , O(Gqd)            , O(Xy)             , U                 , U                 , U                 ),
  INST(kX86InstIdVmovntdq         , "vmovntdq"         , O_660F00(E7,U,_,_,_), U                   , Enc(AvxMr_P)      , F(Avx)                             , EF(________), 0 , 0 , O(Mem)            , O(Xy)             , U                 , U                 , U                 ),
  INST(kX86InstIdVmovntdqa        , "vmovntdqa"        , O_660F38(2A,U,_,_,_), U                   , Enc(AvxRm_P)      , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Mem)            , U                 , U                 , U                 ),
  INST(kX86InstIdVmovntpd         , "vmovntpd"         , O_660F00(2B,U,_,_,_), U                   , Enc(AvxMr_P)      , F(Avx)                             , EF(________), 0 , 0 , O(Mem)            , O(Xy)             , U                 , U                 , U                 ),
  INST(kX86InstIdVmovntps         , "vmovntps"         , O_000F00(2B,U,_,_,_), U                   , Enc(AvxMr_P)      , F(Avx)                             , EF(________), 0 , 0 , O(Mem)            , O(Xy)             , U                 , U                 , U                 ),
  INST(kX86InstIdVmovq            , "vmovq"            , O_660F00(6E,U,_,W,_), O_660F00(7E,U,_,_,_), Enc(AvxMovDQ)     , F(Avx)                             , EF(________), 0 , 0 , O(XmmMem)         , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdVmovsd           , "vmovsd"           , O_F20F00(10,U,_,_,_), O_F20F00(11,U,_,_,_), Enc(AvxMovSsSd)   , F(Avx)                             , EF(________), 0 , 0 , O(XmmMem)         , O(XmmMem)         , O(Xmm)            , U                 , U                 ),
  INST(kX86InstIdVmovshdup        , "vmovshdup"        , O_F30F00(16,U,_,_,_), U                   , Enc(AvxRm_P)      , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(XyMem)          , U                 , U                 , U                 ),
  INST(kX86InstIdVmovsldup        , "vmovsldup"        , O_F30F00(12,U,_,_,_), U                   , Enc(AvxRm_P)      , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(XyMem)          , U                 , U                 , U                 ),
  INST(kX86InstIdVmovss           , "vmovss"           , O_F30F00(10,U,_,_,_), O_F30F00(11,U,_,_,_), Enc(AvxMovSsSd)   , F(Avx)                             , EF(________), 0 , 0 , O(XmmMem)         , O(Xmm)            , O(Xmm)            , U                 , U                 ),
  INST(kX86InstIdVmovupd          , "vmovupd"          , O_660F00(10,U,_,_,_), O_660F00(11,U,_,_,_), Enc(AvxRmMr_P)    , F(Avx)                             , EF(________), 0 , 0 , O(XyMem)          , O(XyMem)          , U                 , U                 , U                 ),
  INST(kX86InstIdVmovups          , "vmovups"          , O_000F00(10,U,_,_,_), O_000F00(11,U,_,_,_), Enc(AvxRmMr_P)    , F(Avx)                             , EF(________), 0 , 0 , O(XyMem)          , O(XyMem)          , U                 , U                 , U                 ),
  INST(kX86InstIdVmpsadbw         , "vmpsadbw"         , O_660F3A(42,U,_,_,_), U                   , Enc(AvxRvmi_P)    , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , O(Imm)            , U                 ),
  INST(kX86InstIdVmulpd           , "vmulpd"           , O_660F00(59,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVmulps           , "vmulps"           , O_000F00(59,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVmulsd           , "vmulsd"           , O_F20F00(59,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVmulss           , "vmulss"           , O_F30F00(59,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVorpd            , "vorpd"            , O_660F00(56,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVorps            , "vorps"            , O_000F00(56,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVpabsb           , "vpabsb"           , O_660F38(1C,U,_,_,_), U                   , Enc(AvxRm_P)      , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(XyMem)          , U                 , U                 , U                 ),
  INST(kX86InstIdVpabsd           , "vpabsd"           , O_660F38(1E,U,_,_,_), U                   , Enc(AvxRm_P)      , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(XyMem)          , U                 , U                 , U                 ),
  INST(kX86InstIdVpabsw           , "vpabsw"           , O_660F38(1D,U,_,_,_), U                   , Enc(AvxRm_P)      , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(XyMem)          , U                 , U                 , U                 ),
  INST(kX86InstIdVpackssdw        , "vpackssdw"        , O_660F00(6B,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVpacksswb        , "vpacksswb"        , O_660F00(63,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVpackusdw        , "vpackusdw"        , O_660F38(2B,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVpackuswb        , "vpackuswb"        , O_660F00(67,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVpaddb           , "vpaddb"           , O_660F00(FC,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVpaddd           , "vpaddd"           , O_660F00(FE,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVpaddq           , "vpaddq"           , O_660F00(D4,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVpaddsb          , "vpaddsb"          , O_660F00(EC,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVpaddsw          , "vpaddsw"          , O_660F00(ED,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVpaddusb         , "vpaddusb"         , O_660F00(DC,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVpaddusw         , "vpaddusw"         , O_660F00(DD,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVpaddw           , "vpaddw"           , O_660F00(FD,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVpalignr         , "vpalignr"         , O_660F3A(0F,U,_,_,_), U                   , Enc(AvxRvmi_P)    , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , O(Imm)            , U                 ),
  INST(kX86InstIdVpand            , "vpand"            , O_660F00(DB,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVpandn           , "vpandn"           , O_660F00(DF,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVpavgb           , "vpavgb"           , O_660F00(E0,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVpavgw           , "vpavgw"           , O_660F00(E3,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVpblendd         , "vpblendd"         , O_660F3A(02,U,_,_,_), U                   , Enc(AvxRvmi_P)    , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , O(Imm)            , U                 ),
  INST(kX86InstIdVpblendvb        , "vpblendvb"        , O_660F3A(4C,U,_,_,_), U                   , Enc(AvxRvmr)      , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , O(Xy)             , U                 ),
  INST(kX86InstIdVpblendw         , "vpblendw"         , O_660F3A(0E,U,_,_,_), U                   , Enc(AvxRvmi_P)    , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , O(Imm)            , U                 ),
  INST(kX86InstIdVpbroadcastb     , "vpbroadcastb"     , O_660F38(78,U,_,_,0), U                   , Enc(AvxRm_P)      , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdVpbroadcastd     , "vpbroadcastd"     , O_660F38(58,U,_,_,0), U                   , Enc(AvxRm_P)      , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdVpbroadcastq     , "vpbroadcastq"     , O_660F38(59,U,_,_,1), U                   , Enc(AvxRm_P)      , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdVpbroadcastw     , "vpbroadcastw"     , O_660F38(79,U,_,_,0), U                   , Enc(AvxRm_P)      , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdVpclmulqdq       , "vpclmulqdq"       , O_660F3A(44,U,_,_,_), U                   , Enc(AvxRvmi)      , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(Xmm)            , O(XmmMem)         , O(Imm)            , U                 ),
  INST(kX86InstIdVpcmov           , "vpcmov"           , O_00_M08(A2,U,_,_,_), U                   , Enc(XopRvrmRvmr_P), F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , O(XyMem)          , U                 ),
  INST(kX86InstIdVpcmpeqb         , "vpcmpeqb"         , O_660F00(74,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVpcmpeqd         , "vpcmpeqd"         , O_660F00(76,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVpcmpeqq         , "vpcmpeqq"         , O_660F38(29,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVpcmpeqw         , "vpcmpeqw"         , O_660F00(75,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVpcmpestri       , "vpcmpestri"       , O_660F3A(61,U,_,_,_), U                   , Enc(AvxRmi)       , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , O(Imm)            , U                 , U                 ),
  INST(kX86InstIdVpcmpestrm       , "vpcmpestrm"       , O_660F3A(60,U,_,_,_), U                   , Enc(AvxRmi)       , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , O(Imm)            , U                 , U                 ),
  INST(kX86InstIdVpcmpgtb         , "vpcmpgtb"         , O_660F00(64,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVpcmpgtd         , "vpcmpgtd"         , O_660F00(66,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVpcmpgtq         , "vpcmpgtq"         , O_660F38(37,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVpcmpgtw         , "vpcmpgtw"         , O_660F00(65,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVpcmpistri       , "vpcmpistri"       , O_660F3A(63,U,_,_,_), U                   , Enc(AvxRmi)       , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , O(Imm)            , U                 , U                 ),
  INST(kX86InstIdVpcmpistrm       , "vpcmpistrm"       , O_660F3A(62,U,_,_,_), U                   , Enc(AvxRmi)       , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , O(Imm)            , U                 , U                 ),
  INST(kX86InstIdVpcomb           , "vpcomb"           , O_00_M08(CC,U,_,_,_), U                   , Enc(XopRvmi)      , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(Xmm)            , O(XmmMem)         , O(Imm)            , U                 ),
  INST(kX86InstIdVpcomd           , "vpcomd"           , O_00_M08(CE,U,_,_,_), U                   , Enc(XopRvmi)      , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(Xmm)            , O(XmmMem)         , O(Imm)            , U                 ),
  INST(kX86InstIdVpcomq           , "vpcomq"           , O_00_M08(CF,U,_,_,_), U                   , Enc(XopRvmi)      , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(Xmm)            , O(XmmMem)         , O(Imm)            , U                 ),
  INST(kX86InstIdVpcomub          , "vpcomub"          , O_00_M08(EC,U,_,_,_), U                   , Enc(XopRvmi)      , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(Xmm)            , O(XmmMem)         , O(Imm)            , U                 ),
  INST(kX86InstIdVpcomud          , "vpcomud"          , O_00_M08(EE,U,_,_,_), U                   , Enc(XopRvmi)      , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(Xmm)            , O(XmmMem)         , O(Imm)            , U                 ),
  INST(kX86InstIdVpcomuq          , "vpcomuq"          , O_00_M08(EF,U,_,_,_), U                   , Enc(XopRvmi)      , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(Xmm)            , O(XmmMem)         , O(Imm)            , U                 ),
  INST(kX86InstIdVpcomuw          , "vpcomuw"          , O_00_M08(ED,U,_,_,_), U                   , Enc(XopRvmi)      , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(Xmm)            , O(XmmMem)         , O(Imm)            , U                 ),
  INST(kX86InstIdVpcomw           , "vpcomw"           , O_00_M08(CD,U,_,_,_), U                   , Enc(XopRvmi)      , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(Xmm)            , O(XmmMem)         , O(Imm)            , U                 ),
  INST(kX86InstIdVperm2f128       , "vperm2f128"       , O_660F3A(06,U,L,_,_), U                   , Enc(AvxRvmi)      , F(Avx)                             , EF(________), 0 , 0 , O(Ymm)            , O(Ymm)            , O(YmmMem)         , O(Imm)            , U                 ),
  INST(kX86InstIdVperm2i128       , "vperm2i128"       , O_660F3A(46,U,L,_,_), U                   , Enc(AvxRvmi)      , F(Avx)                             , EF(________), 0 , 0 , O(Ymm)            , O(Ymm)            , O(YmmMem)         , O(Imm)            , U                 ),
  INST(kX86InstIdVpermd           , "vpermd"           , O_660F38(36,U,L,_,_), U                   , Enc(AvxRvm)       , F(Avx)                             , EF(________), 0 , 0 , O(Ymm)            , O(Ymm)            , O(YmmMem)         , U                 , U                 ),
  INST(kX86InstIdVpermil2pd       , "vpermil2pd"       , O_66_M03(49,U,_,_,_), U                   , Enc(AvxRvrmRvmr_P), F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , O(XyMem)          , U                 ),
  INST(kX86InstIdVpermil2ps       , "vpermil2ps"       , O_66_M03(48,U,_,_,_), U                   , Enc(AvxRvrmRvmr_P), F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , O(XyMem)          , U                 ),
  INST(kX86InstIdVpermilpd        , "vpermilpd"        , O_660F38(0D,U,_,_,_), O_660F3A(05,U,_,_,_), Enc(AvxRvmRmi_P)  , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(XyMem)          , O(XyMem)|O(Imm)   , U                 , U                 ),
  INST(kX86InstIdVpermilps        , "vpermilps"        , O_660F38(0C,U,_,_,_), O_660F3A(04,U,_,_,_), Enc(AvxRvmRmi_P)  , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(XyMem)          , O(XyMem)|O(Imm)   , U                 , U                 ),
  INST(kX86InstIdVpermpd          , "vpermpd"          , O_660F3A(01,U,L,W,_), U                   , Enc(AvxRmi)       , F(Avx)                             , EF(________), 0 , 0 , O(Ymm)            , O(YmmMem)         , O(Imm)            , U                 , U                 ),
  INST(kX86InstIdVpermps          , "vpermps"          , O_660F38(16,U,L,_,_), U                   , Enc(AvxRvm)       , F(Avx)                             , EF(________), 0 , 0 , O(Ymm)            , O(Ymm)            , O(YmmMem)         , U                 , U                 ),
  INST(kX86InstIdVpermq           , "vpermq"           , O_660F3A(00,U,L,W,_), U                   , Enc(AvxRmi)       , F(Avx)                             , EF(________), 0 , 0 , O(Ymm)            , O(YmmMem)         , O(Imm)            , U                 , U                 ),
  INST(kX86InstIdVpextrb          , "vpextrb"          , O_660F3A(14,U,_,_,_), U                   , Enc(AvxMri)       , F(Avx)                             , EF(________), 0 , 0 , O(GqdwbMem)       , O(Xmm)            , O(Imm)            , U                 , U                 ),
  INST(kX86InstIdVpextrd          , "vpextrd"          , O_660F3A(16,U,_,_,_), U                   , Enc(AvxMri)       , F(Avx)                             , EF(________), 0 , 0 , O(GqdMem)         , O(Xmm)            , O(Imm)            , U                 , U                 ),
  INST(kX86InstIdVpextrq          , "vpextrq"          , O_660F3A(16,U,_,W,_), U                   , Enc(AvxMri)       , F(Avx)                             , EF(________), 0 , 0 , O(GqMem)          , O(Xmm)            , O(Imm)            , U                 , U                 ),
  INST(kX86InstIdVpextrw          , "vpextrw"          , O_660F3A(15,U,_,_,_), U                   , Enc(AvxMri)       , F(Avx)                             , EF(________), 0 , 0 , O(GqdwMem)        , O(Xmm)            , O(Imm)            , U                 , U                 ),
  INST(kX86InstIdVpgatherdd       , "vpgatherdd"       , O_660F38(90,U,_,_,_), U                   , Enc(AvxGather)    , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Mem)            , O(Xy)             , U                 , U                 ),
  INST(kX86InstIdVpgatherdq       , "vpgatherdq"       , O_660F38(90,U,_,W,_), U                   , Enc(AvxGather)    , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Mem)            , O(Xy)             , U                 , U                 ),
  INST(kX86InstIdVpgatherqd       , "vpgatherqd"       , O_660F38(91,U,_,_,_), U                   , Enc(AvxGatherEx)  , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(Mem)            , O(Xmm)            , U                 , U                 ),
  INST(kX86InstIdVpgatherqq       , "vpgatherqq"       , O_660F38(91,U,_,W,_), U                   , Enc(AvxGather)    , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Mem)            , O(Xy)             , U                 , U                 ),
  INST(kX86InstIdVphaddbd         , "vphaddbd"         , O_00_M09(C2,U,_,_,_), U                   , Enc(XopRm)        , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdVphaddbq         , "vphaddbq"         , O_00_M09(C3,U,_,_,_), U                   , Enc(XopRm)        , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdVphaddbw         , "vphaddbw"         , O_00_M09(C1,U,_,_,_), U                   , Enc(XopRm)        , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdVphaddd          , "vphaddd"          , O_660F38(02,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVphadddq         , "vphadddq"         , O_00_M09(CB,U,_,_,_), U                   , Enc(XopRm)        , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdVphaddsw         , "vphaddsw"         , O_660F38(03,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVphaddubd        , "vphaddubd"        , O_00_M09(D2,U,_,_,_), U                   , Enc(XopRm)        , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdVphaddubq        , "vphaddubq"        , O_00_M09(D3,U,_,_,_), U                   , Enc(XopRm)        , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdVphaddubw        , "vphaddubw"        , O_00_M09(D1,U,_,_,_), U                   , Enc(XopRm)        , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdVphaddudq        , "vphaddudq"        , O_00_M09(DB,U,_,_,_), U                   , Enc(XopRm)        , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdVphadduwd        , "vphadduwd"        , O_00_M09(D6,U,_,_,_), U                   , Enc(XopRm)        , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdVphadduwq        , "vphadduwq"        , O_00_M09(D7,U,_,_,_), U                   , Enc(XopRm)        , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdVphaddw          , "vphaddw"          , O_660F38(01,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVphaddwd         , "vphaddwd"         , O_00_M09(C6,U,_,_,_), U                   , Enc(XopRm)        , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdVphaddwq         , "vphaddwq"         , O_00_M09(C7,U,_,_,_), U                   , Enc(XopRm)        , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdVphminposuw      , "vphminposuw"      , O_660F38(41,U,_,_,_), U                   , Enc(AvxRm)        , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdVphsubbw         , "vphsubbw"         , O_00_M09(E1,U,_,_,_), U                   , Enc(XopRm)        , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdVphsubd          , "vphsubd"          , O_660F38(06,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVphsubdq         , "vphsubdq"         , O_00_M09(E3,U,_,_,_), U                   , Enc(XopRm)        , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdVphsubsw         , "vphsubsw"         , O_660F38(07,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVphsubw          , "vphsubw"          , O_660F38(05,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVphsubwd         , "vphsubwd"         , O_00_M09(E2,U,_,_,_), U                   , Enc(XopRm)        , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdVpinsrb          , "vpinsrb"          , O_660F3A(20,U,_,_,_), U                   , Enc(AvxRvmi)      , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(Xmm)            , O(GqdwbMem)       , O(Imm)            , U                 ),
  INST(kX86InstIdVpinsrd          , "vpinsrd"          , O_660F3A(22,U,_,_,_), U                   , Enc(AvxRvmi)      , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(Xmm)            , O(GqdMem)         , O(Imm)            , U                 ),
  INST(kX86InstIdVpinsrq          , "vpinsrq"          , O_660F3A(22,U,_,W,_), U                   , Enc(AvxRvmi)      , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(Xmm)            , O(GqMem)          , O(Imm)            , U                 ),
  INST(kX86InstIdVpinsrw          , "vpinsrw"          , O_660F00(C4,U,_,_,_), U                   , Enc(AvxRvmi)      , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(Xmm)            , O(GqdwMem)        , O(Imm)            , U                 ),
  INST(kX86InstIdVpmacsdd         , "vpmacsdd"         , O_00_M08(9E,U,_,_,_), U                   , Enc(XopRvmr)      , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(Xmm)            , O(XmmMem)         , O(Xmm)            , U                 ),
  INST(kX86InstIdVpmacsdqh        , "vpmacsdqh"        , O_00_M08(9F,U,_,_,_), U                   , Enc(XopRvmr)      , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(Xmm)            , O(XmmMem)         , O(Xmm)            , U                 ),
  INST(kX86InstIdVpmacsdql        , "vpmacsdql"        , O_00_M08(97,U,_,_,_), U                   , Enc(XopRvmr)      , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(Xmm)            , O(XmmMem)         , O(Xmm)            , U                 ),
  INST(kX86InstIdVpmacssdd        , "vpmacssdd"        , O_00_M08(8E,U,_,_,_), U                   , Enc(XopRvmr)      , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(Xmm)            , O(XmmMem)         , O(Xmm)            , U                 ),
  INST(kX86InstIdVpmacssdqh       , "vpmacssdqh"       , O_00_M08(8F,U,_,_,_), U                   , Enc(XopRvmr)      , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(Xmm)            , O(XmmMem)         , O(Xmm)            , U                 ),
  INST(kX86InstIdVpmacssdql       , "vpmacssdql"       , O_00_M08(87,U,_,_,_), U                   , Enc(XopRvmr)      , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(Xmm)            , O(XmmMem)         , O(Xmm)            , U                 ),
  INST(kX86InstIdVpmacsswd        , "vpmacsswd"        , O_00_M08(86,U,_,_,_), U                   , Enc(XopRvmr)      , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(Xmm)            , O(XmmMem)         , O(Xmm)            , U                 ),
  INST(kX86InstIdVpmacssww        , "vpmacssww"        , O_00_M08(85,U,_,_,_), U                   , Enc(XopRvmr)      , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(Xmm)            , O(XmmMem)         , O(Xmm)            , U                 ),
  INST(kX86InstIdVpmacswd         , "vpmacswd"         , O_00_M08(96,U,_,_,_), U                   , Enc(XopRvmr)      , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(Xmm)            , O(XmmMem)         , O(Xmm)            , U                 ),
  INST(kX86InstIdVpmacsww         , "vpmacsww"         , O_00_M08(95,U,_,_,_), U                   , Enc(XopRvmr)      , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(Xmm)            , O(XmmMem)         , O(Xmm)            , U                 ),
  INST(kX86InstIdVpmadcsswd       , "vpmadcsswd"       , O_00_M08(A6,U,_,_,_), U                   , Enc(XopRvmr)      , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(Xmm)            , O(XmmMem)         , O(Xmm)            , U                 ),
  INST(kX86InstIdVpmadcswd        , "vpmadcswd"        , O_00_M08(B6,U,_,_,_), U                   , Enc(XopRvmr)      , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(Xmm)            , O(XmmMem)         , O(Xmm)            , U                 ),
  INST(kX86InstIdVpmaddubsw       , "vpmaddubsw"       , O_660F38(04,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVpmaddwd         , "vpmaddwd"         , O_660F00(F5,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVpmaskmovd       , "vpmaskmovd"       , O_660F38(8C,U,_,_,_), O_660F38(8E,U,_,_,_), Enc(AvxRvmMvr_P)  , F(Avx)                             , EF(________), 0 , 0 , O(XyMem)          , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVpmaskmovq       , "vpmaskmovq"       , O_660F38(8C,U,_,W,_), O_660F38(8E,U,_,_,_), Enc(AvxRvmMvr_P)  , F(Avx)                             , EF(________), 0 , 0 , O(XyMem)          , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVpmaxsb          , "vpmaxsb"          , O_660F38(3C,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVpmaxsd          , "vpmaxsd"          , O_660F38(3D,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVpmaxsw          , "vpmaxsw"          , O_660F00(EE,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVpmaxub          , "vpmaxub"          , O_660F00(DE,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVpmaxud          , "vpmaxud"          , O_660F38(3F,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVpmaxuw          , "vpmaxuw"          , O_660F38(3E,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVpminsb          , "vpminsb"          , O_660F38(38,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVpminsd          , "vpminsd"          , O_660F38(39,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVpminsw          , "vpminsw"          , O_660F00(EA,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVpminub          , "vpminub"          , O_660F00(DA,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVpminud          , "vpminud"          , O_660F38(3B,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVpminuw          , "vpminuw"          , O_660F38(3A,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVpmovmskb        , "vpmovmskb"        , O_660F00(D7,U,_,_,_), U                   , Enc(AvxRm_P)      , F(Avx)                             , EF(________), 0 , 0 , O(Gqd)            , O(Xy)             , U                 , U                 , U                 ),
  INST(kX86InstIdVpmovsxbd        , "vpmovsxbd"        , O_660F38(21,U,_,_,_), U                   , Enc(AvxRm_P)      , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(XyMem)          , U                 , U                 , U                 ),
  INST(kX86InstIdVpmovsxbq        , "vpmovsxbq"        , O_660F38(22,U,_,_,_), U                   , Enc(AvxRm_P)      , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(XyMem)          , U                 , U                 , U                 ),
  INST(kX86InstIdVpmovsxbw        , "vpmovsxbw"        , O_660F38(20,U,_,_,_), U                   , Enc(AvxRm_P)      , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(XyMem)          , U                 , U                 , U                 ),
  INST(kX86InstIdVpmovsxdq        , "vpmovsxdq"        , O_660F38(25,U,_,_,_), U                   , Enc(AvxRm_P)      , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(XyMem)          , U                 , U                 , U                 ),
  INST(kX86InstIdVpmovsxwd        , "vpmovsxwd"        , O_660F38(23,U,_,_,_), U                   , Enc(AvxRm_P)      , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(XyMem)          , U                 , U                 , U                 ),
  INST(kX86InstIdVpmovsxwq        , "vpmovsxwq"        , O_660F38(24,U,_,_,_), U                   , Enc(AvxRm_P)      , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(XyMem)          , U                 , U                 , U                 ),
  INST(kX86InstIdVpmovzxbd        , "vpmovzxbd"        , O_660F38(31,U,_,_,_), U                   , Enc(AvxRm_P)      , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(XyMem)          , U                 , U                 , U                 ),
  INST(kX86InstIdVpmovzxbq        , "vpmovzxbq"        , O_660F38(32,U,_,_,_), U                   , Enc(AvxRm_P)      , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(XyMem)          , U                 , U                 , U                 ),
  INST(kX86InstIdVpmovzxbw        , "vpmovzxbw"        , O_660F38(30,U,_,_,_), U                   , Enc(AvxRm_P)      , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(XyMem)          , U                 , U                 , U                 ),
  INST(kX86InstIdVpmovzxdq        , "vpmovzxdq"        , O_660F38(35,U,_,_,_), U                   , Enc(AvxRm_P)      , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(XyMem)          , U                 , U                 , U                 ),
  INST(kX86InstIdVpmovzxwd        , "vpmovzxwd"        , O_660F38(33,U,_,_,_), U                   , Enc(AvxRm_P)      , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(XyMem)          , U                 , U                 , U                 ),
  INST(kX86InstIdVpmovzxwq        , "vpmovzxwq"        , O_660F38(34,U,_,_,_), U                   , Enc(AvxRm_P)      , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(XyMem)          , U                 , U                 , U                 ),
  INST(kX86InstIdVpmuldq          , "vpmuldq"          , O_660F38(28,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVpmulhrsw        , "vpmulhrsw"        , O_660F38(0B,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVpmulhuw         , "vpmulhuw"         , O_660F00(E4,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVpmulhw          , "vpmulhw"          , O_660F00(E5,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVpmulld          , "vpmulld"          , O_660F38(40,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVpmullw          , "vpmullw"          , O_660F00(D5,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVpmuludq         , "vpmuludq"         , O_660F00(F4,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVpor             , "vpor"             , O_660F00(EB,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVpperm           , "vpperm"           , O_00_M08(A3,U,_,_,_), U                   , Enc(XopRvrmRvmr)  , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(Xmm)            , O(XmmMem)         , O(XmmMem)         , U                 ),
  INST(kX86InstIdVprotb           , "vprotb"           , O_00_M09(90,U,_,_,_), O_00_M08(C0,U,_,_,_), Enc(XopRvmRmi)    , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , O(XmmMem)|O(Imm)  , U                 , U                 ),
  INST(kX86InstIdVprotd           , "vprotd"           , O_00_M09(92,U,_,_,_), O_00_M08(C2,U,_,_,_), Enc(XopRvmRmi)    , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , O(XmmMem)|O(Imm)  , U                 , U                 ),
  INST(kX86InstIdVprotq           , "vprotq"           , O_00_M09(93,U,_,_,_), O_00_M08(C3,U,_,_,_), Enc(XopRvmRmi)    , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , O(XmmMem)|O(Imm)  , U                 , U                 ),
  INST(kX86InstIdVprotw           , "vprotw"           , O_00_M09(91,U,_,_,_), O_00_M08(C1,U,_,_,_), Enc(XopRvmRmi)    , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , O(XmmMem)|O(Imm)  , U                 , U                 ),
  INST(kX86InstIdVpsadbw          , "vpsadbw"          , O_660F00(F6,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVpshab           , "vpshab"           , O_00_M09(98,U,_,_,_), U                   , Enc(XopRvmRmv)    , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , O(XmmMem)         , U                 , U                 ),
  INST(kX86InstIdVpshad           , "vpshad"           , O_00_M09(9A,U,_,_,_), U                   , Enc(XopRvmRmv)    , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , O(XmmMem)         , U                 , U                 ),
  INST(kX86InstIdVpshaq           , "vpshaq"           , O_00_M09(9B,U,_,_,_), U                   , Enc(XopRvmRmv)    , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , O(XmmMem)         , U                 , U                 ),
  INST(kX86InstIdVpshaw           , "vpshaw"           , O_00_M09(99,U,_,_,_), U                   , Enc(XopRvmRmv)    , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , O(XmmMem)         , U                 , U                 ),
  INST(kX86InstIdVpshlb           , "vpshlb"           , O_00_M09(94,U,_,_,_), U                   , Enc(XopRvmRmv)    , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , O(XmmMem)         , U                 , U                 ),
  INST(kX86InstIdVpshld           , "vpshld"           , O_00_M09(96,U,_,_,_), U                   , Enc(XopRvmRmv)    , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , O(XmmMem)         , U                 , U                 ),
  INST(kX86InstIdVpshlq           , "vpshlq"           , O_00_M09(97,U,_,_,_), U                   , Enc(XopRvmRmv)    , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , O(XmmMem)         , U                 , U                 ),
  INST(kX86InstIdVpshlw           , "vpshlw"           , O_00_M09(95,U,_,_,_), U                   , Enc(XopRvmRmv)    , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , O(XmmMem)         , U                 , U                 ),
  INST(kX86InstIdVpshufb          , "vpshufb"          , O_660F38(00,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVpshufd          , "vpshufd"          , O_660F00(70,U,_,_,_), U                   , Enc(AvxRmi_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(XyMem)          , O(Imm)            , U                 , U                 ),
  INST(kX86InstIdVpshufhw         , "vpshufhw"         , O_F30F00(70,U,_,_,_), U                   , Enc(AvxRmi_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(XyMem)          , O(Imm)            , U                 , U                 ),
  INST(kX86InstIdVpshuflw         , "vpshuflw"         , O_F20F00(70,U,_,_,_), U                   , Enc(AvxRmi_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(XyMem)          , O(Imm)            , U                 , U                 ),
  INST(kX86InstIdVpsignb          , "vpsignb"          , O_660F38(08,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVpsignd          , "vpsignd"          , O_660F38(0A,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVpsignw          , "vpsignw"          , O_660F38(09,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVpslld           , "vpslld"           , O_660F00(F2,U,_,_,_), O_660F00(72,6,_,_,_), Enc(AvxRvmVmi_P)  , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(XyMem)          , O(XyMem)|O(Imm)   , U                 , U                 ),
  INST(kX86InstIdVpslldq          , "vpslldq"          , O_660F00(73,7,_,_,_), U                   , Enc(AvxVmi_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(XyMem)          , O(Imm)            , U                 , U                 ),
  INST(kX86InstIdVpsllq           , "vpsllq"           , O_660F00(F3,U,_,_,_), O_660F00(73,6,_,_,_), Enc(AvxRvmVmi_P)  , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(XyMem)          , O(XyMem)|O(Imm)   , U                 , U                 ),
  INST(kX86InstIdVpsllvd          , "vpsllvd"          , O_660F38(47,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVpsllvq          , "vpsllvq"          , O_660F38(47,U,_,W,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVpsllw           , "vpsllw"           , O_660F00(F1,U,_,_,_), O_660F00(71,6,_,_,_), Enc(AvxRvmVmi_P)  , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(XyMem)          , O(XyMem)|O(Imm)   , U                 , U                 ),
  INST(kX86InstIdVpsrad           , "vpsrad"           , O_660F00(E2,U,_,_,_), O_660F00(72,4,_,_,_), Enc(AvxRvmVmi_P)  , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(XyMem)          , O(XyMem)|O(Imm)   , U                 , U                 ),
  INST(kX86InstIdVpsravd          , "vpsravd"          , O_660F38(46,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVpsraw           , "vpsraw"           , O_660F00(E1,U,_,_,_), O_660F00(71,4,_,_,_), Enc(AvxRvmVmi_P)  , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(XyMem)          , O(XyMem)|O(Imm)   , U                 , U                 ),
  INST(kX86InstIdVpsrld           , "vpsrld"           , O_660F00(D2,U,_,_,_), O_660F00(72,2,_,_,_), Enc(AvxRvmVmi_P)  , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(XyMem)          , O(XyMem)|O(Imm)   , U                 , U                 ),
  INST(kX86InstIdVpsrldq          , "vpsrldq"          , O_660F00(73,3,_,_,_), U                   , Enc(AvxVmi_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(XyMem)          , O(Imm)            , U                 , U                 ),
  INST(kX86InstIdVpsrlq           , "vpsrlq"           , O_660F00(D3,U,_,_,_), O_660F00(73,2,_,_,_), Enc(AvxRvmVmi_P)  , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(XyMem)          , O(XyMem)|O(Imm)   , U                 , U                 ),
  INST(kX86InstIdVpsrlvd          , "vpsrlvd"          , O_660F38(45,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVpsrlvq          , "vpsrlvq"          , O_660F38(45,U,_,W,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVpsrlw           , "vpsrlw"           , O_660F00(D1,U,_,_,_), O_660F00(71,2,_,_,_), Enc(AvxRvmVmi_P)  , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(XyMem)          , O(XyMem)|O(Imm)   , U                 , U                 ),
  INST(kX86InstIdVpsubb           , "vpsubb"           , O_660F00(F8,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVpsubd           , "vpsubd"           , O_660F00(FA,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVpsubq           , "vpsubq"           , O_660F00(FB,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVpsubsb          , "vpsubsb"          , O_660F00(E8,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVpsubsw          , "vpsubsw"          , O_660F00(E9,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVpsubusb         , "vpsubusb"         , O_660F00(D8,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVpsubusw         , "vpsubusw"         , O_660F00(D9,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVpsubw           , "vpsubw"           , O_660F00(F9,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVptest           , "vptest"           , O_660F38(17,U,_,_,_), U                   , Enc(AvxRm_P)      , F(Avx)                             , EF(WWWWWW__), 0 , 0 , O(Xy)             , O(XyMem)          , U                 , U                 , U                 ),
  INST(kX86InstIdVpunpckhbw       , "vpunpckhbw"       , O_660F00(68,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVpunpckhdq       , "vpunpckhdq"       , O_660F00(6A,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVpunpckhqdq      , "vpunpckhqdq"      , O_660F00(6D,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVpunpckhwd       , "vpunpckhwd"       , O_660F00(69,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVpunpcklbw       , "vpunpcklbw"       , O_660F00(60,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVpunpckldq       , "vpunpckldq"       , O_660F00(62,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVpunpcklqdq      , "vpunpcklqdq"      , O_660F00(6C,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVpunpcklwd       , "vpunpcklwd"       , O_660F00(61,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVpxor            , "vpxor"            , O_660F00(EF,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVrcpps           , "vrcpps"           , O_000F00(53,U,_,_,_), U                   , Enc(AvxRm_P)      , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(XyMem)          , U                 , U                 , U                 ),
  INST(kX86InstIdVrcpss           , "vrcpss"           , O_F30F00(53,U,_,_,_), U                   , Enc(AvxRvm)       , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(Xmm)            , O(XmmMem)         , U                 , U                 ),
  INST(kX86InstIdVroundpd         , "vroundpd"         , O_660F3A(09,U,_,_,_), U                   , Enc(AvxRmi_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(XyMem)          , O(Imm)            , U                 , U                 ),
  INST(kX86InstIdVroundps         , "vroundps"         , O_660F3A(08,U,_,_,_), U                   , Enc(AvxRmi_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(XyMem)          , O(Imm)            , U                 , U                 ),
  INST(kX86InstIdVroundsd         , "vroundsd"         , O_660F3A(0B,U,_,_,_), U                   , Enc(AvxRvmi)      , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(Xmm)            , O(XmmMem)         , O(Imm)            , U                 ),
  INST(kX86InstIdVroundss         , "vroundss"         , O_660F3A(0A,U,_,_,_), U                   , Enc(AvxRvmi)      , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(Xmm)            , O(XmmMem)         , O(Imm)            , U                 ),
  INST(kX86InstIdVrsqrtps         , "vrsqrtps"         , O_000F00(52,U,_,_,_), U                   , Enc(AvxRm_P)      , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(XyMem)          , U                 , U                 , U                 ),
  INST(kX86InstIdVrsqrtss         , "vrsqrtss"         , O_F30F00(52,U,_,_,_), U                   , Enc(AvxRvm)       , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(Xmm)            , O(XmmMem)         , U                 , U                 ),
  INST(kX86InstIdVshufpd          , "vshufpd"          , O_660F00(C6,U,_,_,_), U                   , Enc(AvxRvmi_P)    , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , O(Imm)            , U                 ),
  INST(kX86InstIdVshufps          , "vshufps"          , O_000F00(C6,U,_,_,_), U                   , Enc(AvxRvmi_P)    , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , O(Imm)            , U                 ),
  INST(kX86InstIdVsqrtpd          , "vsqrtpd"          , O_660F00(51,U,_,_,_), U                   , Enc(AvxRm_P)      , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(XyMem)          , U                 , U                 , U                 ),
  INST(kX86InstIdVsqrtps          , "vsqrtps"          , O_000F00(51,U,_,_,_), U                   , Enc(AvxRm_P)      , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(XyMem)          , U                 , U                 , U                 ),
  INST(kX86InstIdVsqrtsd          , "vsqrtsd"          , O_F20F00(51,U,_,_,_), U                   , Enc(AvxRvm)       , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(Xmm)            , O(XmmMem)         , U                 , U                 ),
  INST(kX86InstIdVsqrtss          , "vsqrtss"          , O_F30F00(51,U,_,_,_), U                   , Enc(AvxRvm)       , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(Xmm)            , O(XmmMem)         , U                 , U                 ),
  INST(kX86InstIdVstmxcsr         , "vstmxcsr"         , O_000F00(AE,3,_,_,_), U                   , Enc(AvxM)         , F(Avx)                             , EF(________), 0 , 0 , O(Mem)            , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdVsubpd           , "vsubpd"           , O_660F00(5C,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVsubps           , "vsubps"           , O_000F00(5C,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVsubsd           , "vsubsd"           , O_F20F00(5C,U,_,_,_), U                   , Enc(AvxRvm)       , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(Xmm)            , O(XmmMem)         , U                 , U                 ),
  INST(kX86InstIdVsubss           , "vsubss"           , O_F30F00(5C,U,_,_,_), U                   , Enc(AvxRvm)       , F(Avx)                             , EF(________), 0 , 0 , O(Xmm)            , O(Xmm)            , O(XmmMem)         , U                 , U                 ),
  INST(kX86InstIdVtestpd          , "vtestpd"          , O_660F38(0F,U,_,_,_), U                   , Enc(AvxRm_P)      , F(Test)                            , EF(WWWWWW__), 0 , 0 , O(Xy)             , O(XyMem)          , U                 , U                 , U                 ),
  INST(kX86InstIdVtestps          , "vtestps"          , O_660F38(0E,U,_,_,_), U                   , Enc(AvxRm_P)      , F(Test)                            , EF(WWWWWW__), 0 , 0 , O(Xy)             , O(XyMem)          , U                 , U                 , U                 ),
  INST(kX86InstIdVucomisd         , "vucomisd"         , O_660F00(2E,U,_,_,_), U                   , Enc(AvxRm)        , F(Avx)                             , EF(WWWWWW__), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdVucomiss         , "vucomiss"         , O_000F00(2E,U,_,_,_), U                   , Enc(AvxRm)        , F(Avx)                             , EF(WWWWWW__), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdVunpckhpd        , "vunpckhpd"        , O_660F00(15,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVunpckhps        , "vunpckhps"        , O_000F00(15,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVunpcklpd        , "vunpcklpd"        , O_660F00(14,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVunpcklps        , "vunpcklps"        , O_000F00(14,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVxorpd           , "vxorpd"           , O_660F00(57,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVxorps           , "vxorps"           , O_000F00(57,U,_,_,_), U                   , Enc(AvxRvm_P)     , F(Avx)                             , EF(________), 0 , 0 , O(Xy)             , O(Xy)             , O(XyMem)          , U                 , U                 ),
  INST(kX86InstIdVzeroall         , "vzeroall"         , O_000F00(77,U,L,_,_), U                   , Enc(AvxOp)        , F(Avx)                             , EF(________), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdVzeroupper       , "vzeroupper"       , O_000F00(77,U,_,_,_), U                   , Enc(AvxOp)        , F(Avx)                             , EF(________), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdWrfsbase         , "wrfsbase"         , O_F30F00(AE,2,_,_,_), U                   , Enc(X86Rm)        , F(None)                            , EF(________), 0 , 0 , O(Gqd)            , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdWrgsbase         , "wrgsbase"         , O_F30F00(AE,3,_,_,_), U                   , Enc(X86Rm)        , F(None)                            , EF(________), 0 , 0 , O(Gqd)            , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdXadd             , "xadd"             , O_000F00(C0,U,_,_,_), U                   , Enc(X86Xadd)      , F(Xchg)|F(Lock)                    , EF(WWWWWW__), 0 , 0 , O(GqdwbMem)       , O(Gqdwb)          , U                 , U                 , U                 ),
  INST(kX86InstIdXchg             , "xchg"             , O_000000(86,U,_,_,_), U                   , Enc(X86Xchg)      , F(Xchg)|F(Lock)                    , EF(________), 0 , 0 , O(GqdwbMem)       , O(Gqdwb)          , U                 , U                 , U                 ),
  INST(kX86InstIdXgetbv           , "xgetbv"           , O_000F01(D0,U,_,_,_), U                   , Enc(X86Op)        , F(None)|F(Special)                 , EF(________), 0 , 0 , U                 , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdXor              , "xor"              , O_000000(30,6,_,_,_), U                   , Enc(X86Arith)     , F(Lock)                            , EF(WWWUWW__), 0 , 0 , O(GqdwbMem)       , O(GqdwbMem)|O(Imm), U                 , U                 , U                 ),
  INST(kX86InstIdXorpd            , "xorpd"            , O_660F00(57,U,_,_,_), U                   , Enc(ExtRm)        , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdXorps            , "xorps"            , O_000F00(57,U,_,_,_), U                   , Enc(ExtRm)        , F(None)                            , EF(________), 0 , 0 , O(Xmm)            , O(XmmMem)         , U                 , U                 , U                 ),
  INST(kX86InstIdXrstor           , "xrstor"           , O_000F00(AE,5,_,_,_), U                   , Enc(X86M)         , F(None)|F(Special)                 , EF(________), 0 , 0 , O(Mem)            , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdXrstor64         , "xrstor64"         , O_000F00(AE,5,_,W,_), U                   , Enc(X86M)         , F(None)|F(Special)                 , EF(________), 0 , 0 , O(Mem)            , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdXsave            , "xsave"            , O_000F00(AE,4,_,_,_), U                   , Enc(X86M)         , F(None)|F(Special)                 , EF(________), 0 , 0 , O(Mem)            , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdXsave64          , "xsave64"          , O_000F00(AE,4,_,W,_), U                   , Enc(X86M)         , F(None)|F(Special)                 , EF(________), 0 , 0 , O(Mem)            , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdXsaveopt         , "xsaveopt"         , O_000F00(AE,6,_,_,_), U                   , Enc(X86M)         , F(None)|F(Special)                 , EF(________), 0 , 0 , O(Mem)            , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdXsaveopt64       , "xsaveopt64"       , O_000F00(AE,6,_,W,_), U                   , Enc(X86M)         , F(None)|F(Special)                 , EF(________), 0 , 0 , O(Mem)            , U                 , U                 , U                 , U                 ),
  INST(kX86InstIdXsetbv           , "xsetbv"           , O_000F01(D1,U,_,_,_), U                   , Enc(X86Op)        , F(None)|F(Special)                 , EF(________), 0 , 0 , U                 , U                 , U                 , U                 , U                 )
};

#undef INST

#undef O_00_X
#undef O_9B_X

#undef O_66_M03
#undef O_00_M09
#undef O_00_M08

#undef O_F30F3A
#undef O_F30F38
#undef O_F30F00
#undef O_F30000
#undef O_F20F3A
#undef O_F20F38
#undef O_F20F00
#undef O_F20000
#undef O_9B0000
#undef O_660F3A
#undef O_660F38
#undef O_660F00
#undef O_660000
#undef O_000F3A
#undef O_000F38
#undef O_000F0F
#undef O_000F01
#undef O_000F00
#undef O_000000

#undef O
#undef EF
#undef A
#undef F
#undef Enc

#undef U

// ============================================================================
// [asmjit::X86Cond]
// ============================================================================

#define CC_TO_INST(_Inst_) { \
  _Inst_##o  , _Inst_##no , _Inst_##b  , _Inst_##ae , \
  _Inst_##e  , _Inst_##ne , _Inst_##be , _Inst_##a  , \
  _Inst_##s  , _Inst_##ns , _Inst_##pe , _Inst_##po , \
  _Inst_##l  , _Inst_##ge , _Inst_##le , _Inst_##g  , \
  kInstIdNone, kInstIdNone, kInstIdNone, kInstIdNone  \
}

const uint32_t _x86ReverseCond[20] = {
  /* O|NO|B|AE    -> */ kX86CondO, kX86CondNO, kX86CondA , kX86CondBE,
  /* E|NE|BE|A    -> */ kX86CondE, kX86CondNE, kX86CondAE, kX86CondB ,
  /* S|NS|PE|PO   -> */ kX86CondS, kX86CondNS, kX86CondPE, kX86CondPO,
  /* L|GE|LE|G    -> */ kX86CondG, kX86CondLE, kX86CondGE, kX86CondL ,
  /* Unord|!Unord -> */ kX86CondFpuUnordered , kX86CondFpuNotUnordered, 0x12, 0x13
};

const uint32_t _x86CondToCmovcc[20] = CC_TO_INST(kX86InstIdCmov);
const uint32_t _x86CondToJcc   [20] = CC_TO_INST(kX86InstIdJ   );
const uint32_t _x86CondToSetcc [20] = CC_TO_INST(kX86InstIdSet );

#undef CC_TO_INST

// ============================================================================
// [asmjit::X86Util]
// ============================================================================

#if !defined(ASMJIT_DISABLE_TEXT)
//! \internal
//!
//! Compare two instruction names.
//!
//! `a` is null terminated instruction name from `_x86InstName[]` table.
//! `b` is non-null terminated instruction name passed to `getInstIdByName()`.
static ASMJIT_INLINE int X86Util_cmpInstName(const char* a, const char* b, size_t len) noexcept {
  for (size_t i = 0; i < len; i++) {
    int c = static_cast<int>(static_cast<uint8_t>(a[i])) -
            static_cast<int>(static_cast<uint8_t>(b[i])) ;
    if (c != 0)
      return c;
  }

  return static_cast<int>(a[len]);
}

uint32_t X86Util::getInstIdByName(const char* name, size_t len) noexcept {
  if (name == nullptr)
    return kInstIdNone;

  if (len == kInvalidIndex)
    len = ::strlen(name);

  if (len == 0)
    return kInstIdNone;

  uint32_t prefix = name[0] - kX86InstAlphaIndexFirst;
  if (prefix > kX86InstAlphaIndexLast - kX86InstAlphaIndexFirst)
    return kInstIdNone;

  uint32_t index = _x86InstAlphaIndex[prefix];
  if (index == kX86InstAlphaIndexInvalid)
    return kInstIdNone;

  const X86InstInfo* base = _x86InstInfo + index;
  const X86InstInfo* end = _x86InstInfo + _kX86InstIdCount;

  // Handle instructions starting with 'j' specially. `jcc` instruction breaks
  // the sorting, because of the suffixes (it's considered as one instruction),
  // so basically `jecxz` and `jmp` are stored after all `jcc` instructions.
  bool linearSearch = prefix == ('j' - kX86InstAlphaIndexFirst);

  while (++prefix <= kX86InstAlphaIndexLast - kX86InstAlphaIndexFirst) {
    index = _x86InstAlphaIndex[prefix];
    if (index == kX86InstAlphaIndexInvalid)
      continue;
    end = _x86InstInfo + index;
    break;
  }

  if (linearSearch) {
    while (base != end) {
      if (X86Util_cmpInstName(base->getInstName(), name, len) == 0)
        return static_cast<uint32_t>((size_t)(base - _x86InstInfo));
      base++;
    }
  }
  else {
    for (size_t lim = (size_t)(end - base); lim != 0; lim >>= 1) {
      const X86InstInfo* cur = base + (lim >> 1);
      int result = X86Util_cmpInstName(cur->getInstName(), name, len);

      if (result < 0) {
        base = cur + 1;
        lim--;
        continue;
      }

      if (result > 0)
        continue;

      return static_cast<uint32_t>((size_t)(cur - _x86InstInfo));
    }
  }

  return kInstIdNone;
}
#endif // ASMJIT_DISABLE_TEXT

// ============================================================================
// [asmjit::X86Util - Test]
// ============================================================================

#if defined(ASMJIT_TEST) && !defined(ASMJIT_DISABLE_TEXT)
UNIT(x86_inst_name) {
  // All known instructions should be matched.
  INFO("Matching all X86/X64 instructions.");
  for (uint32_t a = 0; a < _kX86InstIdCount; a++) {
    uint32_t b = X86Util::getInstIdByName(_x86InstInfo[a].getInstName());

    EXPECT(a == b,
      "Should match existing instruction \"%s\" {id:%u} != \"%s\" {id:%u}.",
        _x86InstInfo[a].getInstName(), a,
        _x86InstInfo[b].getInstName(), b);
  }

  // Everything else should return kInstIdNone
  INFO("Trying to look-up instructions that don't exist.");
  EXPECT(X86Util::getInstIdByName(nullptr) == kInstIdNone,
    "Should return kInstIdNone for `nullptr` input.");

  EXPECT(X86Util::getInstIdByName("") == kInstIdNone,
    "Should return kInstIdNone for empty string.");

  EXPECT(X86Util::getInstIdByName("_") == kInstIdNone,
    "Should return kInstIdNone for unknown instruction.");

  EXPECT(X86Util::getInstIdByName("123xyz") == kInstIdNone,
    "Should return kInstIdNone for unknown instruction.");
}
#endif // ASMJIT_TEST && !ASMJIT_DISABLE_TEXT

} // asmjit namespace

// [Api-End]
#include "../apiend.h"

// [Guard]
#endif // ASMJIT_BUILD_X86 || ASMJIT_BUILD_X64
