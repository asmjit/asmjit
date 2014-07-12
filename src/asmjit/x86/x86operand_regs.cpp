// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Export]
#define ASMJIT_EXPORTS
#define ASMJIT_EXPORTS_X86OPERAND_REGS

// [Guard]
#include "../build.h"
#if defined(ASMJIT_BUILD_X86) || defined(ASMJIT_BUILD_X64)

// [Dependencies - AsmJit]
#include "../x86/x86operand.h"

// [Api-Begin]
#include "../apibegin.h"

namespace asmjit {

// Prevent static initialization.
//
// Remap all classes to POD structs so they can be statically initialized
// without calling a constructor. Compiler will store these in data section.
struct X86GpReg  { Operand::VRegOp data; };
struct X86FpReg  { Operand::VRegOp data; };
struct X86MmReg  { Operand::VRegOp data; };
struct X86XmmReg { Operand::VRegOp data; };
struct X86YmmReg { Operand::VRegOp data; };
struct X86SegReg { Operand::VRegOp data; };

namespace x86 {

// ============================================================================
// [asmjit::x86::Registers]
// ============================================================================

#define REG(_Class_, _Name_, _Type_, _Index_, _Size_) \
  const _Class_ _Name_ = {{ \
    kOperandTypeReg, _Size_, { ((_Type_) << 8) + _Index_ }, kInvalidValue, {{ kInvalidVar, 0 }} \
  }}

REG(X86GpReg, noGpReg, kInvalidReg, kInvalidReg, 0);

REG(X86GpReg, al, kX86RegTypeGpbLo, kX86RegIndexAx, 1);
REG(X86GpReg, cl, kX86RegTypeGpbLo, kX86RegIndexCx, 1);
REG(X86GpReg, dl, kX86RegTypeGpbLo, kX86RegIndexDx, 1);
REG(X86GpReg, bl, kX86RegTypeGpbLo, kX86RegIndexBx, 1);
REG(X86GpReg, spl, kX86RegTypeGpbLo, kX86RegIndexSp, 1);
REG(X86GpReg, bpl, kX86RegTypeGpbLo, kX86RegIndexBp, 1);
REG(X86GpReg, sil, kX86RegTypeGpbLo, kX86RegIndexSi, 1);
REG(X86GpReg, dil, kX86RegTypeGpbLo, kX86RegIndexDi, 1);
REG(X86GpReg, r8b, kX86RegTypeGpbLo, 8, 1);
REG(X86GpReg, r9b, kX86RegTypeGpbLo, 9, 1);
REG(X86GpReg, r10b, kX86RegTypeGpbLo, 10, 1);
REG(X86GpReg, r11b, kX86RegTypeGpbLo, 11, 1);
REG(X86GpReg, r12b, kX86RegTypeGpbLo, 12, 1);
REG(X86GpReg, r13b, kX86RegTypeGpbLo, 13, 1);
REG(X86GpReg, r14b, kX86RegTypeGpbLo, 14, 1);
REG(X86GpReg, r15b, kX86RegTypeGpbLo, 15, 1);

REG(X86GpReg, ah, kX86RegTypeGpbHi, kX86RegIndexAx, 1);
REG(X86GpReg, ch, kX86RegTypeGpbHi, kX86RegIndexCx, 1);
REG(X86GpReg, dh, kX86RegTypeGpbHi, kX86RegIndexDx, 1);
REG(X86GpReg, bh, kX86RegTypeGpbHi, kX86RegIndexBx, 1);

REG(X86GpReg, ax, kX86RegTypeGpw, kX86RegIndexAx, 2);
REG(X86GpReg, cx, kX86RegTypeGpw, kX86RegIndexCx, 2);
REG(X86GpReg, dx, kX86RegTypeGpw, kX86RegIndexDx, 2);
REG(X86GpReg, bx, kX86RegTypeGpw, kX86RegIndexBx, 2);
REG(X86GpReg, sp, kX86RegTypeGpw, kX86RegIndexSp, 2);
REG(X86GpReg, bp, kX86RegTypeGpw, kX86RegIndexBp, 2);
REG(X86GpReg, si, kX86RegTypeGpw, kX86RegIndexSi, 2);
REG(X86GpReg, di, kX86RegTypeGpw, kX86RegIndexDi, 2);
REG(X86GpReg, r8w, kX86RegTypeGpw, 8, 2);
REG(X86GpReg, r9w, kX86RegTypeGpw, 9, 2);
REG(X86GpReg, r10w, kX86RegTypeGpw, 10, 2);
REG(X86GpReg, r11w, kX86RegTypeGpw, 11, 2);
REG(X86GpReg, r12w, kX86RegTypeGpw, 12, 2);
REG(X86GpReg, r13w, kX86RegTypeGpw, 13, 2);
REG(X86GpReg, r14w, kX86RegTypeGpw, 14, 2);
REG(X86GpReg, r15w, kX86RegTypeGpw, 15, 2);

REG(X86GpReg, eax, kX86RegTypeGpd, kX86RegIndexAx, 4);
REG(X86GpReg, ecx, kX86RegTypeGpd, kX86RegIndexCx, 4);
REG(X86GpReg, edx, kX86RegTypeGpd, kX86RegIndexDx, 4);
REG(X86GpReg, ebx, kX86RegTypeGpd, kX86RegIndexBx, 4);
REG(X86GpReg, esp, kX86RegTypeGpd, kX86RegIndexSp, 4);
REG(X86GpReg, ebp, kX86RegTypeGpd, kX86RegIndexBp, 4);
REG(X86GpReg, esi, kX86RegTypeGpd, kX86RegIndexSi, 4);
REG(X86GpReg, edi, kX86RegTypeGpd, kX86RegIndexDi, 4);
REG(X86GpReg, r8d, kX86RegTypeGpd, 8, 4);
REG(X86GpReg, r9d, kX86RegTypeGpd, 9, 4);
REG(X86GpReg, r10d, kX86RegTypeGpd, 10, 4);
REG(X86GpReg, r11d, kX86RegTypeGpd, 11, 4);
REG(X86GpReg, r12d, kX86RegTypeGpd, 12, 4);
REG(X86GpReg, r13d, kX86RegTypeGpd, 13, 4);
REG(X86GpReg, r14d, kX86RegTypeGpd, 14, 4);
REG(X86GpReg, r15d, kX86RegTypeGpd, 15, 4);

REG(X86GpReg, rax, kX86RegTypeGpq, kX86RegIndexAx, 8);
REG(X86GpReg, rcx, kX86RegTypeGpq, kX86RegIndexCx, 8);
REG(X86GpReg, rdx, kX86RegTypeGpq, kX86RegIndexDx, 8);
REG(X86GpReg, rbx, kX86RegTypeGpq, kX86RegIndexBx, 8);
REG(X86GpReg, rsp, kX86RegTypeGpq, kX86RegIndexSp, 8);
REG(X86GpReg, rbp, kX86RegTypeGpq, kX86RegIndexBp, 8);
REG(X86GpReg, rsi, kX86RegTypeGpq, kX86RegIndexSi, 8);
REG(X86GpReg, rdi, kX86RegTypeGpq, kX86RegIndexDi, 8);
REG(X86GpReg, r8, kX86RegTypeGpq, 8, 8);
REG(X86GpReg, r9, kX86RegTypeGpq, 9, 8);
REG(X86GpReg, r10, kX86RegTypeGpq, 10, 8);
REG(X86GpReg, r11, kX86RegTypeGpq, 11, 8);
REG(X86GpReg, r12, kX86RegTypeGpq, 12, 8);
REG(X86GpReg, r13, kX86RegTypeGpq, 13, 8);
REG(X86GpReg, r14, kX86RegTypeGpq, 14, 8);
REG(X86GpReg, r15, kX86RegTypeGpq, 15, 8);

REG(X86FpReg, fp0, kX86RegTypeFp, 0, 10);
REG(X86FpReg, fp1, kX86RegTypeFp, 1, 10);
REG(X86FpReg, fp2, kX86RegTypeFp, 2, 10);
REG(X86FpReg, fp3, kX86RegTypeFp, 3, 10);
REG(X86FpReg, fp4, kX86RegTypeFp, 4, 10);
REG(X86FpReg, fp5, kX86RegTypeFp, 5, 10);
REG(X86FpReg, fp6, kX86RegTypeFp, 6, 10);
REG(X86FpReg, fp7, kX86RegTypeFp, 7, 10);

REG(X86MmReg, mm0, kX86RegTypeMm, 0, 8);
REG(X86MmReg, mm1, kX86RegTypeMm, 1, 8);
REG(X86MmReg, mm2, kX86RegTypeMm, 2, 8);
REG(X86MmReg, mm3, kX86RegTypeMm, 3, 8);
REG(X86MmReg, mm4, kX86RegTypeMm, 4, 8);
REG(X86MmReg, mm5, kX86RegTypeMm, 5, 8);
REG(X86MmReg, mm6, kX86RegTypeMm, 6, 8);
REG(X86MmReg, mm7, kX86RegTypeMm, 7, 8);

REG(X86XmmReg, xmm0, kX86RegTypeXmm, 0, 16);
REG(X86XmmReg, xmm1, kX86RegTypeXmm, 1, 16);
REG(X86XmmReg, xmm2, kX86RegTypeXmm, 2, 16);
REG(X86XmmReg, xmm3, kX86RegTypeXmm, 3, 16);
REG(X86XmmReg, xmm4, kX86RegTypeXmm, 4, 16);
REG(X86XmmReg, xmm5, kX86RegTypeXmm, 5, 16);
REG(X86XmmReg, xmm6, kX86RegTypeXmm, 6, 16);
REG(X86XmmReg, xmm7, kX86RegTypeXmm, 7, 16);
REG(X86XmmReg, xmm8, kX86RegTypeXmm, 8, 16);
REG(X86XmmReg, xmm9, kX86RegTypeXmm, 9, 16);
REG(X86XmmReg, xmm10, kX86RegTypeXmm, 10, 16);
REG(X86XmmReg, xmm11, kX86RegTypeXmm, 11, 16);
REG(X86XmmReg, xmm12, kX86RegTypeXmm, 12, 16);
REG(X86XmmReg, xmm13, kX86RegTypeXmm, 13, 16);
REG(X86XmmReg, xmm14, kX86RegTypeXmm, 14, 16);
REG(X86XmmReg, xmm15, kX86RegTypeXmm, 15, 16);

REG(X86YmmReg, ymm0, kX86RegTypeYmm, 0, 32);
REG(X86YmmReg, ymm1, kX86RegTypeYmm, 1, 32);
REG(X86YmmReg, ymm2, kX86RegTypeYmm, 2, 32);
REG(X86YmmReg, ymm3, kX86RegTypeYmm, 3, 32);
REG(X86YmmReg, ymm4, kX86RegTypeYmm, 4, 32);
REG(X86YmmReg, ymm5, kX86RegTypeYmm, 5, 32);
REG(X86YmmReg, ymm6, kX86RegTypeYmm, 6, 32);
REG(X86YmmReg, ymm7, kX86RegTypeYmm, 7, 32);
REG(X86YmmReg, ymm8, kX86RegTypeYmm, 8, 32);
REG(X86YmmReg, ymm9, kX86RegTypeYmm, 9, 32);
REG(X86YmmReg, ymm10, kX86RegTypeYmm, 10, 32);
REG(X86YmmReg, ymm11, kX86RegTypeYmm, 11, 32);
REG(X86YmmReg, ymm12, kX86RegTypeYmm, 12, 32);
REG(X86YmmReg, ymm13, kX86RegTypeYmm, 13, 32);
REG(X86YmmReg, ymm14, kX86RegTypeYmm, 14, 32);
REG(X86YmmReg, ymm15, kX86RegTypeYmm, 15, 32);

REG(X86SegReg, cs, kX86RegTypeSeg, kX86SegCs, 2);
REG(X86SegReg, ss, kX86RegTypeSeg, kX86SegSs, 2);
REG(X86SegReg, ds, kX86RegTypeSeg, kX86SegDs, 2);
REG(X86SegReg, es, kX86RegTypeSeg, kX86SegEs, 2);
REG(X86SegReg, fs, kX86RegTypeSeg, kX86SegFs, 2);
REG(X86SegReg, gs, kX86RegTypeSeg, kX86SegGs, 2);

#undef REG

} // x86 namespace
} // asmjit namespace

// [Api-End]
#include "../apiend.h"

// [Guard]
#endif // ASMJIT_BUILD_X86 || ASMJIT_BUILD_X64
