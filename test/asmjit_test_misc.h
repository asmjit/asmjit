// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_TEST_MISC_H
#define _ASMJIT_TEST_MISC_H

// [Dependencies]
#include "./asmjit.h"

namespace asmtest {

// Generate a typical alpha blend function using SSE2 instruction set. Used
// for benchmarking and also in test86. The generated code should be stable
// and fully functional.
static void generateAlphaBlend(asmjit::X86Compiler& cc) {
  using namespace asmjit;
  using namespace asmjit::x86;

  X86Gp dst = cc.newIntPtr("dst");
  X86Gp src = cc.newIntPtr("src");

  X86Gp i = cc.newIntPtr("i");
  X86Gp j = cc.newIntPtr("j");
  X86Gp t = cc.newIntPtr("t");

  X86Xmm x0 = cc.newXmm("x0");
  X86Xmm x1 = cc.newXmm("x1");
  X86Xmm y0 = cc.newXmm("y0");
  X86Xmm a0 = cc.newXmm("a0");
  X86Xmm a1 = cc.newXmm("a1");

  X86Xmm cZero    = cc.newXmm("cZero");
  X86Xmm cMul255A = cc.newXmm("cMul255A");
  X86Xmm cMul255M = cc.newXmm("cMul255M");

  Label L_SmallLoop = cc.newLabel();
  Label L_SmallEnd  = cc.newLabel();
  Label L_LargeLoop = cc.newLabel();
  Label L_LargeEnd  = cc.newLabel();
  Label L_DataPool  = cc.newLabel();

  cc.addFunc(FuncSignature3<void, void*, const void*, size_t>(cc.getCodeInfo().getCdeclCallConv()));

  cc.setArg(0, dst);
  cc.setArg(1, src);
  cc.setArg(2, i);

  cc.alloc(dst);
  cc.alloc(src);
  cc.alloc(i);

  // How many pixels have to be processed to make the loop aligned.
  cc.lea(t, ptr(L_DataPool));
  cc.xor_(j, j);
  cc.xorps(cZero, cZero);

  cc.sub(j, dst);
  cc.movaps(cMul255A, ptr(t, 0));

  cc.and_(j, 15);
  cc.movaps(cMul255M, ptr(t, 16));

  cc.shr(j, 2);
  cc.jz(L_SmallEnd);

  // j = min(i, j).
  cc.cmp(j, i);
  cc.cmovg(j, i);

  // i -= j.
  cc.sub(i, j);

  // Small loop.
  cc.bind(L_SmallLoop);

  cc.pcmpeqb(a0, a0);
  cc.movd(y0, ptr(src));

  cc.pxor(a0, y0);
  cc.movd(x0, ptr(dst));

  cc.psrlw(a0, 8);
  cc.punpcklbw(x0, cZero);

  cc.pshuflw(a0, a0, x86::shufImm(1, 1, 1, 1));
  cc.punpcklbw(y0, cZero);

  cc.pmullw(x0, a0);
  cc.paddsw(x0, cMul255A);
  cc.pmulhuw(x0, cMul255M);

  cc.paddw(x0, y0);
  cc.packuswb(x0, x0);

  cc.movd(ptr(dst), x0);

  cc.add(dst, 4);
  cc.add(src, 4);

  cc.dec(j);
  cc.jnz(L_SmallLoop);

  // Second section, prepare for an aligned loop.
  cc.bind(L_SmallEnd);

  cc.test(i, i);
  cc.mov(j, i);
  cc.jz(cc.getFunc()->getExitLabel());

  cc.and_(j, 3);
  cc.shr(i, 2);
  cc.jz(L_LargeEnd);

  // Aligned loop.
  cc.bind(L_LargeLoop);

  cc.movups(y0, ptr(src));
  cc.pcmpeqb(a0, a0);
  cc.movaps(x0, ptr(dst));

  cc.xorps(a0, y0);
  cc.movaps(x1, x0);

  cc.psrlw(a0, 8);
  cc.punpcklbw(x0, cZero);

  cc.movaps(a1, a0);
  cc.punpcklwd(a0, a0);

  cc.punpckhbw(x1, cZero);
  cc.punpckhwd(a1, a1);

  cc.pshufd(a0, a0, x86::shufImm(3, 3, 1, 1));
  cc.pshufd(a1, a1, x86::shufImm(3, 3, 1, 1));

  cc.pmullw(x0, a0);
  cc.pmullw(x1, a1);

  cc.paddsw(x0, cMul255A);
  cc.paddsw(x1, cMul255A);

  cc.pmulhuw(x0, cMul255M);
  cc.pmulhuw(x1, cMul255M);

  cc.add(src, 16);
  cc.packuswb(x0, x1);

  cc.paddw(x0, y0);
  cc.movaps(ptr(dst), x0);

  cc.add(dst, 16);

  cc.dec(i);
  cc.jnz(L_LargeLoop);

  cc.bind(L_LargeEnd);
  cc.test(j, j);
  cc.jnz(L_SmallLoop);

  cc.endFunc();

  // Data.
  cc.align(kAlignData, 16);
  cc.bind(L_DataPool);
  cc.dxmm(Data128::fromI16(0x0080));
  cc.dxmm(Data128::fromI16(0x0101));
}

} // asmtest namespace

// [Guard]
#endif // _ASMJIT_TEST_MISC_H
