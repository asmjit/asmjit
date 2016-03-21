// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _TEST_GENBLEND_H
#define _TEST_GENBLEND_H

// [Dependencies]
#include "../asmjit/asmjit.h"

namespace asmgen {

// Generate a typical alpha blend function using SSE2 instruction set. Used
// for benchmarking and also in test86. The generated code should be stable
// and fully functional.
static void blend(asmjit::X86Compiler& c) {
  using namespace asmjit;
  using namespace asmjit::x86;

  X86GpVar dst = c.newIntPtr("dst");
  X86GpVar src = c.newIntPtr("src");

  X86GpVar i = c.newIntPtr("i");
  X86GpVar j = c.newIntPtr("j");
  X86GpVar t = c.newIntPtr("t");

  X86XmmVar x0 = c.newXmm("x0");
  X86XmmVar x1 = c.newXmm("x1");
  X86XmmVar y0 = c.newXmm("y0");
  X86XmmVar a0 = c.newXmm("a0");
  X86XmmVar a1 = c.newXmm("a1");

  X86XmmVar cZero    = c.newXmm("cZero");
  X86XmmVar cMul255A = c.newXmm("cMul255A");
  X86XmmVar cMul255M = c.newXmm("cMul255M");

  Label L_SmallLoop = c.newLabel();
  Label L_SmallEnd = c.newLabel();

  Label L_LargeLoop = c.newLabel();
  Label L_LargeEnd = c.newLabel();

  Label L_Data = c.newLabel();

  c.addFunc(FuncBuilder3<Void, void*, const void*, size_t>(c.getRuntime()->getCdeclConv()));

  c.setArg(0, dst);
  c.setArg(1, src);
  c.setArg(2, i);

  c.alloc(dst);
  c.alloc(src);
  c.alloc(i);

  // How many pixels have to be processed to make the loop aligned.
  c.lea(t, ptr(L_Data));
  c.xor_(j, j);
  c.xorps(cZero, cZero);

  c.sub(j, dst);
  c.movaps(cMul255A, ptr(t, 0));

  c.and_(j, 15);
  c.movaps(cMul255M, ptr(t, 16));

  c.shr(j, 2);
  c.jz(L_SmallEnd);

  // j = min(i, j).
  c.cmp(j, i);
  c.cmovg(j, i);

  // i -= j.
  c.sub(i, j);

  // Small loop.
  c.bind(L_SmallLoop);

  c.pcmpeqb(a0, a0);
  c.movd(y0, ptr(src));

  c.pxor(a0, y0);
  c.movd(x0, ptr(dst));

  c.psrlw(a0, 8);
  c.punpcklbw(x0, cZero);

  c.pshuflw(a0, a0, X86Util::shuffle(1, 1, 1, 1));
  c.punpcklbw(y0, cZero);

  c.pmullw(x0, a0);
  c.paddsw(x0, cMul255A);
  c.pmulhuw(x0, cMul255M);

  c.paddw(x0, y0);
  c.packuswb(x0, x0);

  c.movd(ptr(dst), x0);

  c.add(dst, 4);
  c.add(src, 4);

  c.dec(j);
  c.jnz(L_SmallLoop);

  // Second section, prepare for an aligned loop.
  c.bind(L_SmallEnd);

  c.test(i, i);
  c.mov(j, i);
  c.jz(c.getFunc()->getExitLabel());

  c.and_(j, 3);
  c.shr(i, 2);
  c.jz(L_LargeEnd);

  // Aligned loop.
  c.bind(L_LargeLoop);

  c.movups(y0, ptr(src));
  c.pcmpeqb(a0, a0);
  c.movaps(x0, ptr(dst));

  c.xorps(a0, y0);
  c.movaps(x1, x0);

  c.psrlw(a0, 8);
  c.punpcklbw(x0, cZero);

  c.movaps(a1, a0);
  c.punpcklwd(a0, a0);

  c.punpckhbw(x1, cZero);
  c.punpckhwd(a1, a1);

  c.pshufd(a0, a0, X86Util::shuffle(3, 3, 1, 1));
  c.pshufd(a1, a1, X86Util::shuffle(3, 3, 1, 1));

  c.pmullw(x0, a0);
  c.pmullw(x1, a1);

  c.paddsw(x0, cMul255A);
  c.paddsw(x1, cMul255A);

  c.pmulhuw(x0, cMul255M);
  c.pmulhuw(x1, cMul255M);

  c.add(src, 16);
  c.packuswb(x0, x1);

  c.paddw(x0, y0);
  c.movaps(ptr(dst), x0);

  c.add(dst, 16);

  c.dec(i);
  c.jnz(L_LargeLoop);

  c.bind(L_LargeEnd);
  c.test(j, j);
  c.jnz(L_SmallLoop);

  c.endFunc();

  // Data.
  c.align(kAlignData, 16);
  c.bind(L_Data);
  c.dxmm(Vec128::fromSW(0x0080));
  c.dxmm(Vec128::fromSW(0x0101));
}

} // asmgen namespace

// [Guard]
#endif // _TEST_GENBLEND_H
