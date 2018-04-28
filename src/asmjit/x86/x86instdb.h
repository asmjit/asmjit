// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// ZLIB - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_X86_X86INSTDB_H
#define _ASMJIT_X86_X86INSTDB_H

// [Dependencies]
#include "../x86/x86globals.h"

ASMJIT_BEGIN_SUB_NAMESPACE(x86)

//! \addtogroup asmjit_x86_db
//! \{

//! Instruction database (X86).
namespace InstDB {

// ============================================================================
// [asmjit::x86::InstDB::ArchMask]
// ============================================================================

//! Architecture mask.
enum ArchMask : uint32_t {
  kArchMaskNone           = 0x00u,       //!< No arch (invalid).
  kArchMaskX86            = 0x01u,       //!< X86 mode supported.
  kArchMaskX64            = 0x02u        //!< X64 mode supported.
};

static inline uint32_t archMaskFromArchId(uint32_t archId) noexcept {
  return archId == ArchInfo::kIdX86 ? kArchMaskX86 :
         archId == ArchInfo::kIdX64 ? kArchMaskX64 : kArchMaskNone;
}

// ============================================================================
// [asmjit::x86::InstDB::OpFlags]
// ============================================================================

//! Operand flags (X86).
enum OpFlags : uint32_t {
  kOpNone                 = 0x00000000u, //!< No flags.

  kOpGpbLo                = 0x00000001u, //!< Operand can be low 8-bit GPB register.
  kOpGpbHi                = 0x00000002u, //!< Operand can be high 8-bit GPB register.
  kOpGpw                  = 0x00000004u, //!< Operand can be 16-bit GPW register.
  kOpGpd                  = 0x00000008u, //!< Operand can be 32-bit GPD register.
  kOpGpq                  = 0x00000010u, //!< Operand can be 64-bit GPQ register.
  kOpXmm                  = 0x00000020u, //!< Operand can be 128-bit XMM register.
  kOpYmm                  = 0x00000040u, //!< Operand can be 256-bit YMM register.
  kOpZmm                  = 0x00000080u, //!< Operand can be 512-bit ZMM register.
  kOpMm                   = 0x00000100u, //!< Operand can be 64-bit MM register.
  kOpKReg                 = 0x00000200u, //!< Operand can be 64-bit K register.
  kOpSReg                 = 0x00000400u, //!< Operand can be SReg (segment register).
  kOpCReg                 = 0x00000800u, //!< Operand can be CReg (control register).
  kOpDReg                 = 0x00001000u, //!< Operand can be DReg (debug register).
  kOpSt                   = 0x00002000u, //!< Operand can be 80-bit ST register (FPU).
  kOpBnd                  = 0x00004000u, //!< Operand can be 128-bit BND register.
  kOpAllRegs              = 0x00007FFFu, //!< Combination of all possible registers.

  kOpU4                   = 0x00010000u, //!< Operand can be unsigned 4-bit  immediate.
  kOpI8                   = 0x00020000u, //!< Operand can be signed   8-bit  immediate.
  kOpU8                   = 0x00040000u, //!< Operand can be unsigned 8-bit  immediate.
  kOpI16                  = 0x00080000u, //!< Operand can be signed   16-bit immediate.
  kOpU16                  = 0x00100000u, //!< Operand can be unsigned 16-bit immediate.
  kOpI32                  = 0x00200000u, //!< Operand can be signed   32-bit immediate.
  kOpU32                  = 0x00400000u, //!< Operand can be unsigned 32-bit immediate.
  kOpI64                  = 0x00800000u, //!< Operand can be signed   64-bit immediate.
  kOpU64                  = 0x01000000u, //!< Operand can be unsigned 64-bit immediate.
  kOpAllImm               = 0x01FF0000u, //!< Operand can be any immediate.

  kOpMem                  = 0x02000000u, //!< Operand can be a scalar memory pointer.
  kOpVm                   = 0x04000000u, //!< Operand can be a vector memory pointer.

  kOpRel8                 = 0x08000000u, //!< Operand can be relative 8-bit  displacement.
  kOpRel32                = 0x10000000u, //!< Operand can be relative 32-bit displacement.

  kOpR                    = 0x20000000u, //!< Operand is read.
  kOpW                    = 0x40000000u, //!< Operand is written.
  kOpX                    = 0x60000000u, //!< Operand is read & written.
  kOpImplicit             = 0x80000000u  //!< Operand is implicit.
};

// ============================================================================
// [asmjit::x86::InstDB::MemFlags]
// ============================================================================

//! Memory operand flags (X86).
enum MemFlags : uint32_t {
  // NOTE: Instruction uses either scalar or vector memory operands, they never
  // collide. This allows us to share bits between "M" and "Vm" enums.

  kMemOpAny               = 0x0001u,     //!< Operand can be any scalar memory pointer.
  kMemOpM8                = 0x0002u,     //!< Operand can be an 8-bit memory pointer.
  kMemOpM16               = 0x0004u,     //!< Operand can be a 16-bit memory pointer.
  kMemOpM32               = 0x0008u,     //!< Operand can be a 32-bit memory pointer.
  kMemOpM48               = 0x0010u,     //!< Operand can be a 32-bit memory pointer.
  kMemOpM64               = 0x0020u,     //!< Operand can be a 64-bit memory pointer.
  kMemOpM80               = 0x0040u,     //!< Operand can be an 80-bit memory pointer.
  kMemOpM128              = 0x0080u,     //!< Operand can be a 128-bit memory pointer.
  kMemOpM256              = 0x0100u,     //!< Operand can be a 256-bit memory pointer.
  kMemOpM512              = 0x0200u,     //!< Operand can be a 512-bit memory pointer.
  kMemOpM1024             = 0x0400u,     //!< Operand can be a 1024-bit memory pointer.

  kMemOpVm32x             = 0x0002u,     //!< Operand can be a vm32x (vector) pointer.
  kMemOpVm32y             = 0x0004u,     //!< Operand can be a vm32y (vector) pointer.
  kMemOpVm32z             = 0x0008u,     //!< Operand can be a vm32z (vector) pointer.
  kMemOpVm64x             = 0x0020u,     //!< Operand can be a vm64x (vector) pointer.
  kMemOpVm64y             = 0x0040u,     //!< Operand can be a vm64y (vector) pointer.
  kMemOpVm64z             = 0x0080u,     //!< Operand can be a vm64z (vector) pointer.

  kMemOpBaseOnly          = 0x0800u,     //!< Only memory base is allowed (no index, no offset).
  kMemOpDs                = 0x1000u,     //!< Implicit memory operand's DS segment.
  kMemOpEs                = 0x2000u,     //!< Implicit memory operand's ES segment.

  kMemOpMib               = 0x4000u      //!< Operand must be MIB (base+index) pointer.
};

// ============================================================================
// [asmjit::x86::InstDB::Encoding]
// ============================================================================

//! Instruction encoding (X86).
enum EncodingId : uint32_t {
  kEncodingNone = 0,                     //!< Never used.
  kEncodingX86Op,                        //!< X86 [OP].
  kEncodingX86Op_O,                      //!< X86 [OP] (opcode and /0-7).
  kEncodingX86Op_O_I8,                   //!< X86 [OP] (opcode and /0-7 + 8-bit immediate).
  kEncodingX86Op_xAX,                    //!< X86 [OP] (implicit or explicit '?AX' form).
  kEncodingX86Op_xDX_xAX,                //!< X86 [OP] (implicit or explicit '?DX, ?AX' form).
  kEncodingX86Op_ZAX,                    //!< X86 [OP] (implicit or explicit '[EAX|RAX]' form).
  kEncodingX86I_xAX,                     //!< X86 [I] (implicit or explicit '?AX' form).
  kEncodingX86M,                         //!< X86 [M] (handles 2|4|8-bytes size).
  kEncodingX86M_NoSize,                  //!< X86 [M] (doesn't handle any size).
  kEncodingX86M_GPB,                     //!< X86 [M] (handles single-byte size).
  kEncodingX86M_GPB_MulDiv,              //!< X86 [M] (like GPB, handles implicit|explicit MUL|DIV|IDIV).
  kEncodingX86M_Only,                    //!< X86 [M] (restricted to memory operand of any size).
  kEncodingX86Rm,                        //!< X86 [RM] (doesn't handle single-byte size).
  kEncodingX86Rm_Raw66H,                 //!< X86 [RM] (used by LZCNT, POPCNT, and TZCNT).
  kEncodingX86Rm_NoRexW,                 //!< X86 [RM] (doesn't add REX.W prefix if 64-bit reg is used).
  kEncodingX86Mr,                        //!< X86 [MR] (doesn't handle single-byte size).
  kEncodingX86Mr_NoSize,                 //!< X86 [MR] (doesn't handle any size).
  kEncodingX86Arith,                     //!< X86 adc, add, and, cmp, or, sbb, sub, xor.
  kEncodingX86Bswap,                     //!< X86 bswap.
  kEncodingX86Bt,                        //!< X86 bt, btc, btr, bts.
  kEncodingX86Call,                      //!< X86 call.
  kEncodingX86Cmpxchg,                   //!< X86 [MR] cmpxchg.
  kEncodingX86Cmpxchg8b_16b,             //!< X86 [MR] cmpxchg8b, cmpxchg16b.
  kEncodingX86Crc,                       //!< X86 crc32.
  kEncodingX86Enter,                     //!< X86 enter.
  kEncodingX86Imul,                      //!< X86 imul.
  kEncodingX86In,                        //!< X86 in.
  kEncodingX86Ins,                       //!< X86 ins[b|q|d].
  kEncodingX86IncDec,                    //!< X86 inc, dec.
  kEncodingX86Int,                       //!< X86 int (interrupt).
  kEncodingX86Jcc,                       //!< X86 jcc.
  kEncodingX86JecxzLoop,                 //!< X86 jcxz, jecxz, jrcxz, loop, loope, loopne.
  kEncodingX86Jmp,                       //!< X86 jmp.
  kEncodingX86JmpRel,                    //!< X86 xbegin.
  kEncodingX86Lea,                       //!< X86 lea.
  kEncodingX86Mov,                       //!< X86 mov (all possible cases).
  kEncodingX86MovsxMovzx,                //!< X86 movsx, movzx.
  kEncodingX86Out,                       //!< X86 out.
  kEncodingX86Outs,                      //!< X86 out[b|q|d].
  kEncodingX86Push,                      //!< X86 push.
  kEncodingX86Pop,                       //!< X86 pop.
  kEncodingX86Ret,                       //!< X86 ret.
  kEncodingX86Rot,                       //!< X86 rcl, rcr, rol, ror, sal, sar, shl, shr.
  kEncodingX86Set,                       //!< X86 setcc.
  kEncodingX86ShldShrd,                  //!< X86 shld, shrd.
  kEncodingX86StrRm,                     //!< X86 lods.
  kEncodingX86StrMr,                     //!< X86 scas, stos.
  kEncodingX86StrMm,                     //!< X86 cmps, movs.
  kEncodingX86Test,                      //!< X86 test.
  kEncodingX86Xadd,                      //!< X86 xadd.
  kEncodingX86Xchg,                      //!< X86 xchg.
  kEncodingX86Fence,                     //!< X86 lfence, mfence, sfence.
  kEncodingX86Bndmov,                    //!< X86 [RM|MR] (used by BNDMOV).
  kEncodingFpuOp,                        //!< FPU [OP].
  kEncodingFpuArith,                     //!< FPU fadd, fdiv, fdivr, fmul, fsub, fsubr.
  kEncodingFpuCom,                       //!< FPU fcom, fcomp.
  kEncodingFpuFldFst,                    //!< FPU fld, fst, fstp.
  kEncodingFpuM,                         //!< FPU fiadd, ficom, ficomp, fidiv, fidivr, fild, fimul, fist, fistp, fisttp, fisub, fisubr.
  kEncodingFpuR,                         //!< FPU fcmov, fcomi, fcomip, ffree, fucom, fucomi, fucomip, fucomp, fxch.
  kEncodingFpuRDef,                      //!< FPU faddp, fdivp, fdivrp, fmulp, fsubp, fsubrp.
  kEncodingFpuStsw,                      //!< FPU fnstsw, Fstsw.
  kEncodingExtRm,                        //!< EXT [RM].
  kEncodingExtRm_XMM0,                   //!< EXT [RM<XMM0>].
  kEncodingExtRm_ZDI,                    //!< EXT [RM<ZDI>].
  kEncodingExtRm_P,                      //!< EXT [RM] (propagates 66H if the instruction uses XMM register).
  kEncodingExtRm_Wx,                     //!< EXT [RM] (propagates REX.W if GPQ is used).
  kEncodingExtRmRi,                      //!< EXT [RM|RI].
  kEncodingExtRmRi_P,                    //!< EXT [RM|RI] (propagates 66H if the instruction uses XMM register).
  kEncodingExtRmi,                       //!< EXT [RMI].
  kEncodingExtRmi_P,                     //!< EXT [RMI] (propagates 66H if the instruction uses XMM register).
  kEncodingExtPextrw,                    //!< EXT pextrw.
  kEncodingExtExtract,                   //!< EXT pextrb, pextrd, pextrq, extractps.
  kEncodingExtMov,                       //!< EXT mov?? - #1:[MM|XMM, MM|XMM|Mem] #2:[MM|XMM|Mem, MM|XMM].
  kEncodingExtMovnti,                    //!< EXT movnti.
  kEncodingExtMovbe,                     //!< EXT movbe.
  kEncodingExtMovd,                      //!< EXT movd.
  kEncodingExtMovq,                      //!< EXT movq.
  kEncodingExtExtrq,                     //!< EXT extrq (SSE4A).
  kEncodingExtInsertq,                   //!< EXT insrq (SSE4A).
  kEncodingExt3dNow,                     //!< EXT [RMI] (3DNOW specific).
  kEncodingVexOp,                        //!< VEX [OP].
  kEncodingVexKmov,                      //!< VEX [RM|MR] (used by kmov[b|w|d|q]).
  kEncodingVexM,                         //!< VEX|EVEX [M].
  kEncodingVexM_VM,                      //!< VEX|EVEX [M] (propagates VEX|EVEX.L, VSIB support).
  kEncodingVexMr_Lx,                     //!< VEX|EVEX [MR] (propagates VEX|EVEX.L if YMM used).
  kEncodingVexMr_VM,                     //!< VEX|EVEX [MR] (propagates VEX|EVEX.L, VSIB support).
  kEncodingVexMri,                       //!< VEX|EVEX [MRI].
  kEncodingVexMri_Lx,                    //!< VEX|EVEX [MRI] (propagates VEX|EVEX.L if YMM used).
  kEncodingVexRm,                        //!< VEX|EVEX [RM].
  kEncodingVexRm_ZDI,                    //!< VEX|EVEX [RM<ZDI>].
  kEncodingVexRm_Wx,                     //!< VEX|EVEX [RM] (propagates VEX|EVEX.W if GPQ used).
  kEncodingVexRm_Lx,                     //!< VEX|EVEX [RM] (propagates VEX|EVEX.L if YMM used).
  kEncodingVexRm_VM,                     //!< VEX|EVEX [RM] (propagates VEX|EVEX.L, VSIB support).
  kEncodingVexRm_T1_4X,                  //!<     EVEX [RM] (used by NN instructions that use RM-T1_4X encoding).
  kEncodingVexRmi,                       //!< VEX|EVEX [RMI].
  kEncodingVexRmi_Wx,                    //!< VEX|EVEX [RMI] (propagates VEX|EVEX.W if GPQ used).
  kEncodingVexRmi_Lx,                    //!< VEX|EVEX [RMI] (propagates VEX|EVEX.L if YMM used).
  kEncodingVexRvm,                       //!< VEX|EVEX [RVM].
  kEncodingVexRvm_Wx,                    //!< VEX|EVEX [RVM] (propagates VEX|EVEX.W if GPQ used).
  kEncodingVexRvm_ZDX_Wx,                //!< VEX|EVEX [RVM<ZDX>] (propagates VEX|EVEX.W if GPQ used).
  kEncodingVexRvm_Lx,                    //!< VEX|EVEX [RVM] (propagates VEX|EVEX.L if YMM used).
  kEncodingVexRvmr,                      //!< VEX|EVEX [RVMR].
  kEncodingVexRvmr_Lx,                   //!< VEX|EVEX [RVMR] (propagates VEX|EVEX.L if YMM used).
  kEncodingVexRvmi,                      //!< VEX|EVEX [RVMI].
  kEncodingVexRvmi_Lx,                   //!< VEX|EVEX [RVMI] (propagates VEX|EVEX.L if YMM used).
  kEncodingVexRmv,                       //!< VEX|EVEX [RMV].
  kEncodingVexRmv_Wx,                    //!< VEX|EVEX [RMV] (propagates VEX|EVEX.W if GPQ used).
  kEncodingVexRmv_VM,                    //!< VEX|EVEX [RMV] (propagates VEX|EVEX.L, VSIB support).
  kEncodingVexRmvRm_VM,                  //!< VEX|EVEX [RMV|RM] (propagates VEX|EVEX.L, VSIB support).
  kEncodingVexRmvi,                      //!< VEX|EVEX [RMVI].
  kEncodingVexRmMr,                      //!< VEX|EVEX [RM|MR].
  kEncodingVexRmMr_Lx,                   //!< VEX|EVEX [RM|MR] (propagates VEX|EVEX.L if YMM used).
  kEncodingVexRvmRmv,                    //!< VEX|EVEX [RVM|RMV].
  kEncodingVexRvmRmi,                    //!< VEX|EVEX [RVM|RMI].
  kEncodingVexRvmRmi_Lx,                 //!< VEX|EVEX [RVM|RMI] (propagates VEX|EVEX.L if YMM used).
  kEncodingVexRvmRmvRmi,                 //!< VEX|EVEX [RVM|RMV|RMI].
  kEncodingVexRvmMr,                     //!< VEX|EVEX [RVM|MR].
  kEncodingVexRvmMvr,                    //!< VEX|EVEX [RVM|MVR].
  kEncodingVexRvmMvr_Lx,                 //!< VEX|EVEX [RVM|MVR] (propagates VEX|EVEX.L if YMM used).
  kEncodingVexRvmVmi,                    //!< VEX|EVEX [RVM|VMI].
  kEncodingVexRvmVmi_Lx,                 //!< VEX|EVEX [RVM|VMI] (propagates VEX|EVEX.L if YMM used).
  kEncodingVexVm,                        //!< VEX|EVEX [VM].
  kEncodingVexVm_Wx,                     //!< VEX|EVEX [VM] (propagates VEX|EVEX.W if GPQ used).
  kEncodingVexVmi,                       //!< VEX|EVEX [VMI].
  kEncodingVexVmi_Lx,                    //!< VEX|EVEX [VMI] (propagates VEX|EVEX.L if YMM used).
  kEncodingVexEvexVmi_Lx,                //!< VEX|EVEX [VMI] (special, used by vpsrldq and vpslldq)
  kEncodingVexRvrmRvmr,                  //!< VEX|EVEX [RVRM|RVMR].
  kEncodingVexRvrmRvmr_Lx,               //!< VEX|EVEX [RVRM|RVMR] (propagates VEX|EVEX.L if YMM used).
  kEncodingVexRvrmiRvmri_Lx,             //!< VEX|EVEX [RVRMI|RVMRI] (propagates VEX|EVEX.L if YMM used).
  kEncodingVexMovdMovq,                  //!< VEX|EVEX vmovd, vmovq.
  kEncodingVexMovssMovsd,                //!< VEX|EVEX vmovss, vmovsd.
  kEncodingFma4,                         //!< FMA4 [R, R, R/M, R/M].
  kEncodingFma4_Lx,                      //!< FMA4 [R, R, R/M, R/M] (propagates AVX.L if YMM used).
  kEncodingCount                         //!< Count of instruction encodings.
};

// ============================================================================
// [asmjit::x86::InstDB::Flags]
// ============================================================================

//! Instruction flags (X86).
//!
//! Details about instruction encoding, operation, features, and some limitations.
enum Flags : uint32_t {
  kFlagNone               = 0x00000000u, //!< No flags.

  // Operand's Use
  // -------------
  //
  // These flags describe the use of 1st and/or 1st+2nd operands. This allows
  // to fast calculate which operands are read, written, or read and written.
  //
  // In some cases this information is not reliable, because AsmJit uses data
  // generated by a script that merges usually more than one instruction into
  // one AsmJit instruction as some X86 instructions uses more encodings to
  // describe the same operation. In such case `kFlagUseComplex` is set and
  // AsmJit will use different approach to calculate operand's use flags.

  kFlagUseA               = 0x00000001u, //!< Use flags are ambiguous as USE information couldn't be flattened.
  kFlagUseR               = 0x00000002u, //!< 1st operand is R (read), read-only if `kFlagOpW` isn't set.
  kFlagUseW               = 0x00000004u, //!< 1st operand is W (written), write-only if `kFlagOpR` isn't set.
  kFlagUseX               = 0x00000006u, //!< 1st operand is X (read-write).
  kFlagUseXX              = 0x00000008u, //!< 1st and 2nd operands are XX (read & written) (XCHG, XADD).

  kFlagFixedReg           = 0x00000010u, //!< Some operand uses fixed register.
  kFlagFixedMem           = 0x00000020u, //!< Some operand uses fixed register to access memory (EAX|RAX, EDI|RDI, ESI|RSI).
  kFlagFixedRM            = 0x00000030u, //!< Combination of `kFlagUseFixedReg` and `kFlagUseFixedMem`.

  kFlagVolatile           = 0x00000040u,
  kFlagPrivileged         = 0x00000080u, //!< This is a privileged operation that cannot run in user mode.

  // Instruction Family
  // ------------------
  //
  // Instruction family information.

  kFlagFpu                = 0x00000100u, //!< Instruction that accesses FPU registers.
  kFlagMmx                = 0x00000200u, //!< Instruction that accesses MMX registers (including 3DNOW and GEODE) and EMMS.
  kFlagVec                = 0x00000400u, //!< Instruction that accesses XMM registers (SSE, AVX, AVX512).

  // Prefixes and Encoding Flags
  // ---------------------------
  //
  // These describe optional X86 prefixes that can be used to change the instruction's operation.

  kFlagRep                = 0x00001000u, //!< Instruction can be prefixed with using the REP/REPE/REPZ prefix.
  kFlagRepne              = 0x00002000u, //!< Instruction can be prefixed with using the REPNE/REPNZ prefix.
  kFlagLock               = 0x00004000u, //!< Instruction can be prefixed with using the LOCK prefix.
  kFlagXAcquire           = 0x00008000u, //!< Instruction can be prefixed with using the XACQUIRE prefix.
  kFlagXRelease           = 0x00010000u, //!< Instruction can be prefixed with using the XRELEASE prefix.
  kFlagMib                = 0x00020000u, //!< Instruction uses MIB (BNDLDX|BNDSTX) to encode two registers.
  kFlagVsib               = 0x00040000u, //!< Instruction uses VSIB instead of legacy SIB.
  kFlagVex                = 0x00080000u, //!< Instruction can be encoded by VEX|XOP (AVX|AVX2|BMI|XOP|...).
  kFlagEvex               = 0x00100000u, //!< Instruction can be encoded by EVEX (AVX512).

  // FPU Flags
  // ---------
  //
  // Used to tell the encoder which memory operand sizes are encodable.

  kFlagFpuM16             = 0x00200000u, //!< FPU instruction can address `word_ptr` (shared with M10).
  kFlagFpuM32             = 0x00400000u, //!< FPU instruction can address `dword_ptr`.
  kFlagFpuM64             = 0x00800000u, //!< FPU instruction can address `qword_ptr`.
  kFlagFpuM80             = 0x00200000u, //!< FPU instruction can address `tword_ptr` (shared with M2).

  // AVX and AVX515 Flags
  // --------------------
  //
  // If both `kFlagPrefixVex` and `kFlagPrefixEvex` flags are specified it
  // means that the instructions can be encoded by either VEX or EVEX prefix.
  // In that case AsmJit checks global options and also instruction options
  // to decide whether to emit VEX or EVEX prefix.

  kFlagAvx512_            = 0x00000000u, //!< Internally used in tables, has no meaning.
  kFlagAvx512K            = 0x01000000u, //!< Supports masking {k1..k7}.
  kFlagAvx512Z            = 0x02000000u, //!< Supports zeroing {z}, must be used together with `kAvx512k`.
  kFlagAvx512ER           = 0x04000000u, //!< Supports 'embedded-rounding' {er} with implicit {sae},
  kFlagAvx512SAE          = 0x08000000u, //!< Supports 'suppress-all-exceptions' {sae}.
  kFlagAvx512B32          = 0x10000000u, //!< Supports 32-bit broadcast 'b32'.
  kFlagAvx512B64          = 0x20000000u, //!< Supports 64-bit broadcast 'b64'.
  kFlagAvx512T4X          = 0x80000000u, //!< Operates on a vector of consecutive registers (AVX512_4FMAPS and AVX512_4VNNIW).

  // Combinations used by instruction tables to make AVX512 definitions more compact.
  kFlagAvx512KZ            = kFlagAvx512K         | kFlagAvx512Z,
  kFlagAvx512ER_SAE        = kFlagAvx512ER        | kFlagAvx512SAE,
  kFlagAvx512KZ_SAE        = kFlagAvx512KZ        | kFlagAvx512SAE,
  kFlagAvx512KZ_SAE_B32    = kFlagAvx512KZ_SAE    | kFlagAvx512B32,
  kFlagAvx512KZ_SAE_B64    = kFlagAvx512KZ_SAE    | kFlagAvx512B64,

  kFlagAvx512KZ_ER_SAE     = kFlagAvx512KZ        | kFlagAvx512ER_SAE,
  kFlagAvx512KZ_ER_SAE_B32 = kFlagAvx512KZ_ER_SAE | kFlagAvx512B32,
  kFlagAvx512KZ_ER_SAE_B64 = kFlagAvx512KZ_ER_SAE | kFlagAvx512B64,

  kFlagAvx512K_B32         = kFlagAvx512K         | kFlagAvx512B32,
  kFlagAvx512K_B64         = kFlagAvx512K         | kFlagAvx512B64,
  kFlagAvx512KZ_B32        = kFlagAvx512KZ        | kFlagAvx512B32,
  kFlagAvx512KZ_B64        = kFlagAvx512KZ        | kFlagAvx512B64
};

// ============================================================================
// [asmjit::x86::InstDB::SpecialCases]
// ============================================================================

//! Cases that require special handling.
enum SpecialCases : uint32_t {
  kSpecialCaseMovCrDr     = 0x00000001u, //!< `MOV REG <-> CREG|DREG` - Defined/Undefined flags, L0/L3 privilege levels.
  kSpecialCaseMovSsSd     = 0x00000002u  //!< `MOVSS|MOVSD XMM, [MEM]` - Destination operand is completely overwritten.
};

// ============================================================================
// [asmjit::x86::InstDB::OperationFlags]
// ============================================================================

//! Used to describe what the instruction does and some of its quirks.
enum OperationFlags : uint32_t {
  kOperationVolatile      = 0x00000001u  //!< Hint for instruction schedulers to never reorder this instruction (side effects, memory barrier, etc).
};

// ============================================================================
// [asmjit::x86::InstDB::SingleRegCase]
// ============================================================================

enum SingleRegCase : uint32_t {
  kSingleRegNone          = 0,           //!< No special handling.
  kSingleRegRO            = 1,           //!< Operands become read-only  - `REG & REG` and similar.
  kSingleRegWO            = 2            //!< Operands become write-only - `REG ^ REG` and similar.
};

ASMJIT_VARAPI const char _nameData[];

// ============================================================================
// [asmjit::x86::InstDB::OpSignature]
// ============================================================================

//! Operand signature (X86).
//!
//! Contains all possible operand combinations, memory size information, and
//! a fixed register id (or `BaseReg::kIdBad` if fixed id isn't required).
struct OpSignature {
  uint32_t opFlags;                      //!< Operand flags.
  uint16_t memFlags;                     //!< Memory flags.
  uint8_t extFlags;                      //!< Extra flags.
  uint8_t regMask;                       //!< Mask of possible register IDs.
};

ASMJIT_VARAPI const OpSignature _opSignatureTable[];

// ============================================================================
// [asmjit::x86::InstDB::InstSignature]
// ============================================================================

//! Instruction signature (X86).
//!
//! Contains a sequence of operands' combinations and other metadata that defines
//! a single instruction. This data is used by instruction validator.
struct InstSignature {
  uint8_t opCount  : 3;                  //!< Count of operands in `opIndex` (0..6).
  uint8_t archMask : 2;                  //!< Architecture mask of this record.
  uint8_t implicit : 3;                  //!< Number of implicit operands.
  uint8_t reserved;                      //!< Reserved for future use.
  uint8_t operands[Globals::kMaxOpCount];//!< Indexes to `OpSignature` table.
};

ASMJIT_VARAPI const InstSignature _instSignatureTable[];

// ============================================================================
// [asmjit::x86::InstDB::CommonInfo]
// ============================================================================

//! Instruction common information (X86)
//!
//! Aggregated information shared across one or more instruction.
struct CommonInfo {
  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! Get all instruction flags, see `InstInfo::Flags`.
  inline uint32_t flags() const noexcept { return _flags; }
  //! Get whether the instruction has a `flag`, see `InstInfo::Flags`.
  inline bool hasFlag(uint32_t flag) const noexcept { return (_flags & flag) != 0; }

  //! Get whether the instruction is FPU instruction.
  inline bool isFpu() const noexcept { return hasFlag(kFlagFpu); }
  //! Get whether the instruction is MMX/3DNOW instruction that accesses MMX registers (includes EMMS and FEMMS).
  inline bool isMmx() const noexcept { return hasFlag(kFlagMmx); }
  //! Get whether the instruction is SSE|AVX|AVX512 instruction that accesses XMM|YMM|ZMM registers.
  inline bool isVec() const noexcept { return hasFlag(kFlagVec); }
  //! Get whether the instruction is SSE+ (SSE4.2, AES, SHA included) instruction that accesses XMM registers.
  inline bool isSse() const noexcept { return (flags() & (kFlagVec | kFlagVex | kFlagEvex)) == kFlagVec; }
  //! Get whether the instruction is AVX+ (FMA included) instruction that accesses XMM|YMM|ZMM registers.
  inline bool isAvx() const noexcept { return isVec() && isVexOrEvex(); }

  //! Get whether the instruction can be prefixed with LOCK prefix.
  inline bool hasLockPrefix() const noexcept { return hasFlag(kFlagLock); }
  //! Get whether the instruction can be prefixed with REP (REPE|REPZ) prefix.
  inline bool hasRepPrefix() const noexcept { return hasFlag(kFlagRep); }
  //! Get whether the instruction can be prefixed with REPNE (REPNZ) prefix.
  inline bool hasRepnePrefix() const noexcept { return hasFlag(kFlagRepne); }
  //! Get whether the instruction can be prefixed with XACQUIRE prefix.
  inline bool hasXAcquirePrefix() const noexcept { return hasFlag(kFlagXAcquire); }
  //! Get whether the instruction can be prefixed with XRELEASE prefix.
  inline bool hasXReleasePrefix() const noexcept { return hasFlag(kFlagXRelease); }

  //! Get whether the instruction uses MIB.
  inline bool isMibOp() const noexcept { return hasFlag(kFlagMib); }
  //! Get whether the instruction uses VSIB.
  inline bool isVsibOp() const noexcept { return hasFlag(kFlagVsib); }
  //! Get whether the instruction uses VEX (can be set together with EVEX if both are encodable).
  inline bool isVex() const noexcept { return hasFlag(kFlagVex); }
  //! Get whether the instruction uses EVEX (can be set together with VEX if both are encodable).
  inline bool isEvex() const noexcept { return hasFlag(kFlagEvex); }
  //! Get whether the instruction uses EVEX (can be set together with VEX if both are encodable).
  inline bool isVexOrEvex() const noexcept { return hasFlag(kFlagVex | kFlagEvex); }

  //! Get whether the instruction supports AVX512 masking {k}.
  inline bool hasAvx512K() const noexcept { return hasFlag(kFlagAvx512K); }
  //! Get whether the instruction supports AVX512 zeroing {k}{z}.
  inline bool hasAvx512Z() const noexcept { return hasFlag(kFlagAvx512Z); }
  //! Get whether the instruction supports AVX512 embedded-rounding {er}.
  inline bool hasAvx512ER() const noexcept { return hasFlag(kFlagAvx512ER); }
  //! Get whether the instruction supports AVX512 suppress-all-exceptions {sae}.
  inline bool hasAvx512SAE() const noexcept { return hasFlag(kFlagAvx512SAE); }
  //! Get whether the instruction supports AVX512 broadcast (either 32-bit or 64-bit).
  inline bool hasAvx512B() const noexcept { return hasFlag(kFlagAvx512B32 | kFlagAvx512B64); }
  //! Get whether the instruction supports AVX512 broadcast (32-bit).
  inline bool hasAvx512B32() const noexcept { return hasFlag(kFlagAvx512B32); }
  //! Get whether the instruction supports AVX512 broadcast (64-bit).
  inline bool hasAvx512B64() const noexcept { return hasFlag(kFlagAvx512B64); }

  //! Get the destination index of WRITE operation.
  inline uint32_t writeIndex() const noexcept { return _writeIndex; }
  //! Get the number of bytes that will be written by a WRITE operation.
  //!
  //! This information is required by a liveness analysis to mark virtual
  //! registers dead even if the instruction doesn't completely overwrite
  //! the whole register. If the analysis keeps which bytes are completely
  //! overwritten by the instruction it can find the where a register becomes
  //! dead by simply checking if the instruction overwrites all remaining
  //! bytes.
  inline uint32_t writeSize() const noexcept { return _writeSize; }

  inline uint32_t signatureIndex() const noexcept { return _iSignatureIndex; }
  inline uint32_t signatureCount() const noexcept { return _iSignatureCount; }

  inline const InstSignature* signatureData() const noexcept { return _instSignatureTable + _iSignatureIndex; }
  inline const InstSignature* signatureEnd() const noexcept { return _instSignatureTable + _iSignatureIndex + _iSignatureCount; }

  //! Get the control-flow type of the instruction.
  inline uint32_t controlType() const noexcept { return _controlType; }

  inline uint32_t singleRegCase() const noexcept { return _singleRegCase; }
  inline uint32_t specialCases() const noexcept { return _specialCases; }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  uint32_t _flags;                       //!< Instruction flags.
  uint32_t _writeIndex         :  8;     //!< First DST byte of a WRITE operation (default 0).
  uint32_t _writeSize          : 24;     //!< Number of bytes to be written in DST.

  uint32_t _iSignatureIndex    : 11;     //!< First `InstSignature` entry in the database.
  uint32_t _iSignatureCount    :  5;     //!< Number of relevant `ISignature` entries.
  uint32_t _controlType        :  3;     //!< Control type, see `ControlType`.
  uint32_t _singleRegCase      :  2;     //!< Specifies what happens if all source operands share the same register.
  uint32_t _specialCases       :  4;     //!< Special cases.
  uint32_t _reserved           :  7;     //!< Reserved.
};

ASMJIT_VARAPI const CommonInfo _commonInfoTable[];

// ============================================================================
// [asmjit::x86::InstDB::ExecutionInfo]
// ============================================================================

//! Detailed data about instruction's operation, requirements, and side-effects.
struct ExecutionInfo {
  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE bool hasFeature(uint32_t feature) const noexcept {
    for (uint32_t i = 0; i < ASMJIT_ARRAY_SIZE(_features); i++)
      if (feature == _features[i])
        return true;
    return false;
  }

  inline uint32_t specialRegsR() const noexcept { return _specialRegsR; }
  inline uint32_t specialRegsW() const noexcept { return _specialRegsW; }

  inline const uint8_t* featuresData() const noexcept { return _features; }
  inline const uint8_t* featuresEnd() const noexcept { return _features + ASMJIT_ARRAY_SIZE(_features); }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  uint16_t _flags;                     //!< Operation flags.
  uint8_t _features[6];                //!< Features vector.
  uint32_t _specialRegsR;              //!< Special registers read.
  uint32_t _specialRegsW;              //!< Special registers written.
};

ASMJIT_VARAPI const ExecutionInfo _executionInfoTable[];

// ============================================================================
// [asmjit::x86::InstDB::InstInfo]
// ============================================================================

//! Instruction information (X86).
struct InstInfo {
  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! Get instruction name (null terminated).
  //!
  //! NOTE: If AsmJit was compiled with `ASMJIT_DISABLE_TEXT` then this will
  //! return an empty string (null terminated string of zero size).
  inline const char* name() const noexcept { return _nameData + _nameDataIndex; };

  //! Get common information, see `CommonInfo`.
  inline const CommonInfo& commonInfo() const noexcept { return _commonInfoTable[_commonInfoIndex]; }
  //! Get execution information, see `ExecutionInfo`.
  inline const ExecutionInfo& executionInfo() const noexcept { return _executionInfoTable[_executionInfoIndex]; }

  //! Get whether the instruction has flag `flag`, see `Flags`.
  inline bool hasFlag(uint32_t flag) const noexcept { return commonInfo().hasFlag(flag); }
  //! Get instruction flags, see `Flags`.
  inline uint32_t flags() const noexcept { return commonInfo().flags(); }

  //! Get whether the instruction is FPU instruction.
  inline bool isFpu() const noexcept { return commonInfo().isFpu(); }
  //! Get whether the instruction is MMX/3DNOW instruction that accesses MMX registers (includes EMMS and FEMMS).
  inline bool isMmx() const noexcept { return commonInfo().isMmx(); }
  //! Get whether the instruction is SSE|AVX|AVX512 instruction that accesses XMM|YMM|ZMM registers.
  inline bool isVec() const noexcept { return commonInfo().isVec(); }
  //! Get whether the instruction is SSE+ (SSE4.2, AES, SHA included) instruction that accesses XMM registers.
  inline bool isSse() const noexcept { return commonInfo().isSse(); }
  //! Get whether the instruction is AVX+ (FMA included) instruction that accesses XMM|YMM|ZMM registers.
  inline bool isAvx() const noexcept { return commonInfo().isAvx(); }

  //! Get whether the instruction can be prefixed with LOCK prefix.
  inline bool hasLockPrefix() const noexcept { return commonInfo().hasLockPrefix(); }
  //! Get whether the instruction can be prefixed with REP (REPE|REPZ) prefix.
  inline bool hasRepPrefix() const noexcept { return commonInfo().hasRepPrefix(); }
  //! Get whether the instruction can be prefixed with REPNE (REPNZ) prefix.
  inline bool hasRepnePrefix() const noexcept { return commonInfo().hasRepnePrefix(); }
  //! Get whether the instruction can be prefixed with XACQUIRE prefix.
  inline bool hasXAcquirePrefix() const noexcept { return commonInfo().hasXAcquirePrefix(); }
  //! Get whether the instruction can be prefixed with XRELEASE prefix.
  inline bool hasXReleasePrefix() const noexcept { return commonInfo().hasXReleasePrefix(); }

  //! Get whether the instruction uses MIB.
  inline bool isMibOp() const noexcept { return hasFlag(kFlagMib); }
  //! Get whether the instruction uses VSIB.
  inline bool isVsibOp() const noexcept { return hasFlag(kFlagVsib); }
  //! Get whether the instruction uses VEX (can be set together with EVEX if both are encodable).
  inline bool isVex() const noexcept { return hasFlag(kFlagVex); }
  //! Get whether the instruction uses EVEX (can be set together with VEX if both are encodable).
  inline bool isEvex() const noexcept { return hasFlag(kFlagEvex); }
  //! Get whether the instruction uses EVEX (can be set together with VEX if both are encodable).
  inline bool isVexOrEvex() const noexcept { return hasFlag(kFlagVex | kFlagEvex); }

  //! Get whether the instruction supports AVX512 masking {k}.
  inline bool hasAvx512K() const noexcept { return hasFlag(kFlagAvx512K); }
  //! Get whether the instruction supports AVX512 zeroing {k}{z}.
  inline bool hasAvx512Z() const noexcept { return hasFlag(kFlagAvx512Z); }
  //! Get whether the instruction supports AVX512 embedded-rounding {er}.
  inline bool hasAvx512ER() const noexcept { return hasFlag(kFlagAvx512ER); }
  //! Get whether the instruction supports AVX512 suppress-all-exceptions {sae}.
  inline bool hasAvx512SAE() const noexcept { return hasFlag(kFlagAvx512SAE); }
  //! Get whether the instruction supports AVX512 broadcast (either 32-bit or 64-bit).
  inline bool hasAvx512B() const noexcept { return hasFlag(kFlagAvx512B32 | kFlagAvx512B64); }
  //! Get whether the instruction supports AVX512 broadcast (32-bit).
  inline bool hasAvx512B32() const noexcept { return hasFlag(kFlagAvx512B32); }
  //! Get whether the instruction supports AVX512 broadcast (64-bit).
  inline bool hasAvx512B64() const noexcept { return hasFlag(kFlagAvx512B64); }

  //! Get whether 1st operand is read-only.
  inline bool isUseR() const noexcept { return (flags() & kFlagUseX) == kFlagUseR; }
  //! Get whether 1st operand is write-only.
  inline bool isUseW() const noexcept { return (flags() & kFlagUseX) == kFlagUseW; }
  //! Get whether 1st operand is read-write.
  inline bool isUseX() const noexcept { return (flags() & kFlagUseX) == kFlagUseX; }
  //! Get whether 1st and 2nd operands are read-write.
  inline bool isUseXX() const noexcept { return hasFlag(kFlagUseXX); }

  inline bool hasFixedReg() const noexcept { return hasFlag(kFlagFixedReg); }
  inline bool hasFixedMem() const noexcept { return hasFlag(kFlagFixedMem); }
  inline bool hasFixedRM() const noexcept { return hasFlag(kFlagFixedRM); }

  //! Get the control-flow type of the instruction.
  inline uint32_t controlType() const noexcept { return commonInfo().controlType(); }
  inline uint32_t singleRegCase() const noexcept { return commonInfo().singleRegCase(); }
  inline uint32_t specialCases() const noexcept { return commonInfo().specialCases(); }

  inline uint32_t signatureIndex() const noexcept { return commonInfo().signatureIndex(); }
  inline uint32_t signatureCount() const noexcept { return commonInfo().signatureCount(); }

  inline const InstSignature* signatureData() const noexcept { return commonInfo().signatureData(); }
  inline const InstSignature* signatureEnd() const noexcept { return commonInfo().signatureEnd(); }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  uint32_t _nameDataIndex      : 14;     //!< Index to `_nameData`.
  uint32_t _commonInfoIndex    : 10;     //!< Index to `_commonInfoTable`.
  uint32_t _executionInfoIndex : 8;      //!< Index to `_executionInfoTable`.
};

ASMJIT_VARAPI const InstInfo _instInfoTable[];

#if defined(ASMJIT_EXPORTS)
// TODO: Should not be part of the API, don't export.
ASMJIT_VARAPI const uint8_t _encodingTable[];
ASMJIT_VARAPI const uint32_t _mainOpcodeTable[];
ASMJIT_VARAPI const uint8_t _altOpcodeIndex[];
ASMJIT_VARAPI const uint32_t _altOpcodeTable[];

static inline uint32_t encodingFromId(uint32_t instId) noexcept {
  ASMJIT_ASSERT(Inst::isDefinedId(instId));
  return _encodingTable[instId];
}

static inline uint32_t mainOpcodeFromId(uint32_t instId) noexcept {
  ASMJIT_ASSERT(Inst::isDefinedId(instId));
  return _mainOpcodeTable[instId];
}

static inline uint32_t altOpcodeFromId(uint32_t instId) noexcept {
  ASMJIT_ASSERT(Inst::isDefinedId(instId));
  return _altOpcodeTable[_altOpcodeIndex[instId]];
}
#endif

#ifndef ASMJIT_DISABLE_TEXT
//! Get an instruction ID from a given instruction `name`.
//!
//! NOTE: Instruction name MUST BE in lowercase, otherwise there will be no
//! match. If there is an exact match the instruction id is returned, otherwise
//! `kInvalidInstId` (zero) is returned instead. The given `name` doesn't have
//! to be null-terminated if `nameSize` is provided.
ASMJIT_API uint32_t idByName(const char* name, size_t nameSize = Globals::kNullTerminated) noexcept;

//! Get an instruction name from a given instruction id `instId`.
ASMJIT_API const char* nameById(uint32_t instId) noexcept;
#endif

inline const InstInfo& infoById(uint32_t instId) noexcept {
  ASMJIT_ASSERT(Inst::isDefinedId(instId));
  return _instInfoTable[instId];
}

} // InstDB namespace

// ============================================================================
// [asmjit::x86::InstInternal]
// ============================================================================

#if defined(ASMJIT_EXPORTS)
//! \internal
//!
//! Implements API provided by `BaseInst` (X86).
namespace InstInternal {
  #ifndef ASMJIT_DISABLE_INST_API
  Error validate(uint32_t archId, const BaseInst& inst, const Operand_* operands, uint32_t count) noexcept;
  Error queryRWInfo(uint32_t archId, const BaseInst& inst, const Operand_* operands, uint32_t count, InstRWInfo& out) noexcept;
  Error queryFeatures(uint32_t archId, const BaseInst& inst, const Operand_* operands, uint32_t count, BaseFeatures& out) noexcept;
  #endif
};
#endif

//! \}

ASMJIT_END_SUB_NAMESPACE

// [Guard]
#endif // _ASMJIT_X86_X86INSTDB_H
