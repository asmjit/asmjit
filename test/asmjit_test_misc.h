// This file is part of AsmJit project <https://asmjit.com>
//
// See asmjit.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_TEST_MISC_H_INCLUDED
#define ASMJIT_TEST_MISC_H_INCLUDED

#include <asmjit/x86.h>

namespace asmtest {

using namespace asmjit;

// Generates a typical alpha blend function that uses SSE2 instruction set.
// This function combines emitting instructions with control flow constructs
// like binding Labels and jumping to them. This should be pretty representative.
template<typename Emitter>
static void generateSseAlphaBlendInternal(
  Emitter& cc,
  const x86::Gp& dst, const x86::Gp& src, const x86::Gp& n,
  const x86::Gp& gp0,
  const x86::Xmm& simd0, const x86::Xmm& simd1, const x86::Xmm& simd2, const x86::Xmm& simd3,
  const x86::Xmm& simd4, const x86::Xmm& simd5, const x86::Xmm& simd6, const x86::Xmm& simd7) {

  x86::Gp i = n;
  x86::Gp j = gp0;

  x86::Xmm vzero = simd0;
  x86::Xmm v0080 = simd1;
  x86::Xmm v0101 = simd2;

  Label L_SmallLoop = cc.newLabel();
  Label L_SmallEnd  = cc.newLabel();
  Label L_LargeLoop = cc.newLabel();
  Label L_LargeEnd  = cc.newLabel();
  Label L_Done = cc.newLabel();

  // Load SIMD Constants.
  cc.xorps(vzero, vzero);
  cc.mov(gp0.r32(), 0x00800080);
  cc.movd(v0080, gp0.r32());
  cc.mov(gp0.r32(), 0x01010101);
  cc.movd(v0101, gp0.r32());
  cc.pshufd(v0080, v0080, x86::shuffleImm(0, 0, 0, 0));
  cc.pshufd(v0101, v0101, x86::shuffleImm(0, 0, 0, 0));

  // How many pixels have to be processed to make the loop aligned.
  cc.xor_(j, j);
  cc.sub(j, dst);
  cc.and_(j, 15);
  cc.shr(j, 2);
  cc.jz(L_SmallEnd);

  cc.cmp(j, i);
  cc.cmovg(j, i); // j = min(i, j)
  cc.sub(i, j);   // i -= j

  // Small loop.
  cc.bind(L_SmallLoop);
  {
    x86::Xmm x0 = simd3;
    x86::Xmm y0 = simd4;
    x86::Xmm a0 = simd5;

    cc.movd(y0, x86::ptr(src));
    cc.movd(x0, x86::ptr(dst));

    cc.pcmpeqb(a0, a0);
    cc.pxor(a0, y0);
    cc.psrlw(a0, 8);
    cc.punpcklbw(x0, vzero);

    cc.pshuflw(a0, a0, x86::shuffleImm(1, 1, 1, 1));
    cc.punpcklbw(y0, vzero);

    cc.pmullw(x0, a0);
    cc.paddsw(x0, v0080);
    cc.pmulhuw(x0, v0101);

    cc.paddw(x0, y0);
    cc.packuswb(x0, x0);

    cc.movd(x86::ptr(dst), x0);

    cc.add(dst, 4);
    cc.add(src, 4);

    cc.dec(j);
    cc.jnz(L_SmallLoop);
  }

  // Second section, prepare for an aligned loop.
  cc.bind(L_SmallEnd);

  cc.test(i, i);
  cc.mov(j, i);
  cc.jz(L_Done);

  cc.and_(j, 3);
  cc.shr(i, 2);
  cc.jz(L_LargeEnd);

  // Aligned loop.
  cc.bind(L_LargeLoop);
  {
    x86::Xmm x0 = simd3;
    x86::Xmm x1 = simd4;
    x86::Xmm y0 = simd5;
    x86::Xmm a0 = simd6;
    x86::Xmm a1 = simd7;

    cc.movups(y0, x86::ptr(src));
    cc.movaps(x0, x86::ptr(dst));

    cc.pcmpeqb(a0, a0);
    cc.xorps(a0, y0);
    cc.movaps(x1, x0);

    cc.psrlw(a0, 8);
    cc.punpcklbw(x0, vzero);

    cc.movaps(a1, a0);
    cc.punpcklwd(a0, a0);

    cc.punpckhbw(x1, vzero);
    cc.punpckhwd(a1, a1);

    cc.pshufd(a0, a0, x86::shuffleImm(3, 3, 1, 1));
    cc.pshufd(a1, a1, x86::shuffleImm(3, 3, 1, 1));

    cc.pmullw(x0, a0);
    cc.pmullw(x1, a1);

    cc.paddsw(x0, v0080);
    cc.paddsw(x1, v0080);

    cc.pmulhuw(x0, v0101);
    cc.pmulhuw(x1, v0101);

    cc.add(src, 16);
    cc.packuswb(x0, x1);

    cc.paddw(x0, y0);
    cc.movaps(x86::ptr(dst), x0);

    cc.add(dst, 16);

    cc.dec(i);
    cc.jnz(L_LargeLoop);
  }

  cc.bind(L_LargeEnd);
  cc.test(j, j);
  cc.jnz(L_SmallLoop);

  cc.bind(L_Done);
}

static void generateSseAlphaBlend(asmjit::BaseEmitter& emitter, bool emitPrologEpilog) {
  using namespace asmjit::x86;

  if (emitter.isAssembler()) {
    Assembler& cc = *emitter.as<Assembler>();

    x86::Gp dst = cc.zax();
    x86::Gp src = cc.zcx();
    x86::Gp i = cc.zdx();
    x86::Gp j = cc.zdi();

    if (emitPrologEpilog) {
      FuncDetail func;
      func.init(FuncSignatureT<void, void*, const void*, size_t>(CallConvId::kHost), cc.environment());

      FuncFrame frame;
      frame.init(func);
      frame.addDirtyRegs(dst, src, i, j);
      frame.addDirtyRegs(xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7);

      FuncArgsAssignment args(&func);
      args.assignAll(dst, src, i);
      args.updateFuncFrame(frame);
      frame.finalize();

      cc.emitProlog(frame);
      cc.emitArgsAssignment(frame, args);
      generateSseAlphaBlendInternal(cc, dst, src, i, j, xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7);
      cc.emitEpilog(frame);
    }
    else {
      generateSseAlphaBlendInternal(cc, dst, src, i, j, xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7);
    }
  }
#ifndef ASMJIT_NO_BUILDER
  else if (emitter.isBuilder()) {
    Builder& cc = *emitter.as<Builder>();

    x86::Gp dst = cc.zax();
    x86::Gp src = cc.zcx();
    x86::Gp i = cc.zdx();
    x86::Gp j = cc.zdi();

    if (emitPrologEpilog) {
      FuncDetail func;
      func.init(FuncSignatureT<void, void*, const void*, size_t>(CallConvId::kHost), cc.environment());

      FuncFrame frame;
      frame.init(func);
      frame.addDirtyRegs(dst, src, i, j);
      frame.addDirtyRegs(xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7);

      FuncArgsAssignment args(&func);
      args.assignAll(dst, src, i);
      args.updateFuncFrame(frame);
      frame.finalize();

      cc.emitProlog(frame);
      cc.emitArgsAssignment(frame, args);
      generateSseAlphaBlendInternal(cc, dst, src, i, j, xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7);
      cc.emitEpilog(frame);
    }
    else {
      generateSseAlphaBlendInternal(cc, dst, src, i, j, xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7);
    }
  }
#endif
#ifndef ASMJIT_NO_COMPILER
  else if (emitter.isCompiler()) {
    Compiler& cc = *emitter.as<Compiler>();

    Gp dst = cc.newIntPtr("dst");
    Gp src = cc.newIntPtr("src");
    Gp i = cc.newIntPtr("i");
    Gp j = cc.newIntPtr("j");

    Xmm v0 = cc.newXmm("v0");
    Xmm v1 = cc.newXmm("v1");
    Xmm v2 = cc.newXmm("v2");
    Xmm v3 = cc.newXmm("v3");
    Xmm v4 = cc.newXmm("v4");
    Xmm v5 = cc.newXmm("v5");
    Xmm v6 = cc.newXmm("v6");
    Xmm v7 = cc.newXmm("v7");

    FuncNode* funcNode = cc.addFunc(FuncSignatureT<void, void*, const void*, size_t>(CallConvId::kHost));
    funcNode->setArg(0, dst);
    funcNode->setArg(1, src);
    funcNode->setArg(2, i);
    generateSseAlphaBlendInternal(cc, dst, src, i, j, v0, v1, v2, v3, v4, v5, v6, v7);
    cc.endFunc();
  }
#endif
}

} // {asmtest}

#endif // ASMJIT_TEST_MISC_H_INCLUDED
