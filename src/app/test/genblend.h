// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _APP_TEST_GENBLEND_H
#define _APP_TEST_GENBLEND_H

// [Dependencies]
#include <asmjit/asmjit.h>

namespace asmgen {

// Generate a typical alpha blend function using SSE2 instruction set. Used
// for benchmarking and also in test86. The generated code should be stable
// and can be tested.
static void blend(asmjit::host::Compiler& c) {
  using namespace asmjit;
  using namespace asmjit::host;

  GpVar dst(c, kVarTypeIntPtr, "dst");
  GpVar src(c, kVarTypeIntPtr, "src");

  GpVar i(c, kVarTypeIntPtr, "i");
  GpVar j(c, kVarTypeIntPtr, "j");
  GpVar t(c, kVarTypeIntPtr, "t");

  XmmVar cZero(c, kVarTypeXmm, "cZero");
  XmmVar cMul255A(c, kVarTypeXmm, "cMul255A");
  XmmVar cMul255M(c, kVarTypeXmm, "cMul255M");

  XmmVar x0(c, kVarTypeXmm, "x0");
  XmmVar x1(c, kVarTypeXmm, "x1");
  XmmVar y0(c, kVarTypeXmm, "y0");
  XmmVar a0(c, kVarTypeXmm, "a0");
  XmmVar a1(c, kVarTypeXmm, "a1");

  Label L_SmallLoop(c);
  Label L_SmallEnd(c);

  Label L_LargeLoop(c);
  Label L_LargeEnd(c);

  Label L_Data(c);

  c.addFunc(kFuncConvHost, FuncBuilder3<FnVoid, void*, const void*, size_t>());

  c.setArg(0, dst);
  c.setArg(1, src);
  c.setArg(2, i);

  c.alloc(dst);
  c.alloc(src);
  c.alloc(i);

  // How many pixels we have to process to align the loop.
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

  c.pshuflw(a0, a0, mm_shuffle(1, 1, 1, 1));
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

  c.pshufd(a0, a0, mm_shuffle(3, 3, 1, 1));
  c.pshufd(a1, a1, mm_shuffle(3, 3, 1, 1));

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
  c.align(16);
  c.bind(L_Data);
  c.dxmm(XmmData::fromSw(0x0080));
  c.dxmm(XmmData::fromSw(0x0101));
}

} // asmgen namespace

// [Guard]
#endif // _APP_TEST_GENBLEND_H
