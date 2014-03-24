// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Dependencies - AsmJit]
#include <asmjit/asmjit.h>

// [Dependencies - Test]
#include "genblend.h"

// [Dependencies - C]
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace asmjit;
using namespace asmjit::host;

// ============================================================================
// [X86Test]
// ============================================================================

//! @brief Interface used to test Compiler.
struct X86Test {
  X86Test(const char* name = NULL) { _name.setString(name); }
  virtual ~X86Test() {}

  ASMJIT_INLINE const char* getName() const { return _name.getData(); }

  virtual void compile(Compiler& c) = 0;
  virtual bool run(void* func, StringBuilder& result, StringBuilder& expect) = 0;

  StringBuilder _name;
};

// ============================================================================
// [X86Test_AlignBase]
// ============================================================================

struct X86Test_AlignBase : public X86Test {
  X86Test_AlignBase(uint32_t argCount, uint32_t varCount, bool naked, bool pushPop) :
    _argCount(argCount),
    _varCount(varCount),
    _naked(naked),
    _pushPop(pushPop) {

    _name.setFormat("[Align] Args=%u Vars=%u Naked=%c PushPop=%c",
      argCount,
      varCount,
      naked ? 'Y' : 'N',
      pushPop ? 'Y' : 'N');
  }

  static void add(PodVector<X86Test*>& tests) {
    for (unsigned int i = 0; i <= 6; i++) {
      for (unsigned int j = 0; j <= 4; j++) {
        tests.append(new X86Test_AlignBase(i, j, false, false));
        tests.append(new X86Test_AlignBase(i, j, false, true ));
        tests.append(new X86Test_AlignBase(i, j, true , false));
        tests.append(new X86Test_AlignBase(i, j, true , true ));
      }
    }
  }

  virtual void compile(Compiler& c) {
    switch (_argCount) {
      case 0: c.addFunc(kFuncConvHost, FuncBuilder0<int>()); break;
      case 1: c.addFunc(kFuncConvHost, FuncBuilder1<int, int>()); break;
      case 2: c.addFunc(kFuncConvHost, FuncBuilder2<int, int, int>()); break;
      case 3: c.addFunc(kFuncConvHost, FuncBuilder3<int, int, int, int>()); break;
      case 4: c.addFunc(kFuncConvHost, FuncBuilder4<int, int, int, int, int>()); break;
      case 5: c.addFunc(kFuncConvHost, FuncBuilder5<int, int, int, int, int, int>()); break;
      case 6: c.addFunc(kFuncConvHost, FuncBuilder6<int, int, int, int, int, int, int>()); break;
    }

    c.getFunc()->setHint(kFuncHintNaked, _naked);
    c.getFunc()->setHint(kFuncHintPushPop, _pushPop);

    GpVar gpVar(c, kVarTypeIntPtr);
    GpVar gpSum(c, kVarTypeInt32);
    XmmVar xmmVar(c, kVarTypeXmm);

    // Alloc, use and spill preserved registers.
    if (_varCount) {
      c.comment("Var");

      uint32_t varIndex = 0;
      uint32_t regIndex = 0;
      uint32_t regMask = 0x1;
      uint32_t preservedMask = c.getFunc()->getDecl()->getPreserved(kRegClassGp);

      do {
        if ((preservedMask & regMask) != 0 && (regIndex != kRegIndexSp && regIndex != kRegIndexBp)) {
          GpVar tmp(c, kVarTypeInt32);
          c.alloc(tmp, regIndex);
          c.xor_(tmp, tmp);
          c.spill(tmp);
          varIndex++;
        }

        regIndex++;
        regMask <<= 1;
      } while (varIndex < _varCount && regIndex < kRegCountGp);
    }

    // Do a sum of arguments to verify possible relocation when misaligned.
    if (_argCount) {
      uint32_t argIndex;

      c.comment("Arg");
      c.xor_(gpSum, gpSum);

      for (argIndex = 0; argIndex < _argCount; argIndex++) {
        GpVar gpArg(c, kVarTypeInt32);

        c.setArg(argIndex, gpArg);
        c.add(gpSum, gpArg);
      }
    }

    // Check alignment of xmmVar (has to be 16).
    c.comment("Ret");
    c.lea(gpVar, xmmVar.m());
    c.shl(gpVar.r32(), 28);

    // Add a sum of arguments to check whether they are correct.
    if (_argCount)
      c.or_(gpVar.r32(), gpSum);

    c.ret(gpVar);
    c.endFunc();
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect) {
    typedef int (*Func0)();
    typedef int (*Func1)(int);
    typedef int (*Func2)(int, int);
    typedef int (*Func3)(int, int, int);
    typedef int (*Func4)(int, int, int, int);
    typedef int (*Func5)(int, int, int, int, int);
    typedef int (*Func6)(int, int, int, int, int, int);

    unsigned int resultRet = 0;
    unsigned int expectRet = 0;

    switch (_argCount) {
      case 0:
        resultRet = asmjit_cast<Func0>(_func)();
        expectRet = 0;
        break;
      case 1:
        resultRet = asmjit_cast<Func1>(_func)(1);
        expectRet = 1;
        break;
      case 2:
        resultRet = asmjit_cast<Func2>(_func)(1, 2);
        expectRet = 1 + 2;
        break;
      case 3:
        resultRet = asmjit_cast<Func3>(_func)(1, 2, 3);
        expectRet = 1 + 2 + 3;
        break;
      case 4:
        resultRet = asmjit_cast<Func4>(_func)(1, 2, 3, 4);
        expectRet = 1 + 2 + 3 + 4;
        break;
      case 5:
        resultRet = asmjit_cast<Func5>(_func)(1, 2, 3, 4, 5);
        expectRet = 1 + 2 + 3 + 4 + 5;
        break;
      case 6:
        resultRet = asmjit_cast<Func6>(_func)(1, 2, 3, 4, 5, 6);
        expectRet = 1 + 2 + 3 + 4 + 5 + 6;
        break;
    }

    result.setFormat("ret={%u, %u}", resultRet >> 28, resultRet & 0x0FFFFFFFU);
    expect.setFormat("ret={%u, %u}", expectRet >> 28, expectRet & 0x0FFFFFFFU);

    return resultRet == expectRet;
  }

  unsigned int _argCount;
  unsigned int _varCount;

  bool _naked;
  bool _pushPop;
};

// ============================================================================
// [X86Test_JumpCross]
// ============================================================================

struct X86Test_JumpCross : public X86Test {
  X86Test_JumpCross() : X86Test("[Jump] Cross jump") {}

  static void add(PodVector<X86Test*>& tests) {
    tests.append(new X86Test_JumpCross());
  }

  virtual void compile(Compiler& c) {
    c.addFunc(kFuncConvHost, FuncBuilder0<FnVoid>());

    Label L_1(c);
    Label L_2(c);
    Label L_3(c);

    c.jmp(L_2);

    c.bind(L_1);
    c.jmp(L_3);

    c.bind(L_2);
    c.jmp(L_1);

    c.bind(L_3);

    c.ret();
    c.endFunc();
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect) {
    typedef void (*Func)(void);
    Func func = asmjit_cast<Func>(_func);

    func();
    return true;
  }
};

// ============================================================================
// [X86Test_JumpUnreachable1]
// ============================================================================

struct X86Test_JumpUnreachable1 : public X86Test {
  X86Test_JumpUnreachable1() : X86Test("[Jump] Unreachable #1") {}

  static void add(PodVector<X86Test*>& tests) {
    tests.append(new X86Test_JumpUnreachable1());
  }

  virtual void compile(Compiler& c) {
    c.addFunc(kFuncConvHost, FuncBuilder0<FnVoid>());

    Label L_1(c);
    Label L_2(c);
    Label L_3(c);
    Label L_4(c);
    Label L_5(c);
    Label L_6(c);
    Label L_7(c);

    GpVar v0(c, kVarTypeUInt32, "v0");
    GpVar v1(c, kVarTypeUInt32, "v1");

    c.bind(L_2);
    c.bind(L_3);

    c.jmp(L_1);

    c.bind(L_5);
    c.mov(v0, 0);

    c.bind(L_6);
    c.jmp(L_3);
    c.mov(v1, 1);
    c.jmp(L_1);

    c.bind(L_4);
    c.jmp(L_2);
    c.bind(L_7);
    c.add(v0, v1);

    c.bind(L_1);
    c.ret();
    c.endFunc();
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect) {
    typedef void (*Func)(void);
    Func func = asmjit_cast<Func>(_func);

    func();

    result.appendString("ret={}");
    expect.appendString("ret={}");

    return true;
  }
};

// ============================================================================
// [X86Test_JumpUnreachable2]
// ============================================================================

struct X86Test_JumpUnreachable2 : public X86Test {
  X86Test_JumpUnreachable2() : X86Test("[Jump] Unreachable #2") {}

  static void add(PodVector<X86Test*>& tests) {
    tests.append(new X86Test_JumpUnreachable2());
  }

  virtual void compile(Compiler& c) {
    c.addFunc(kFuncConvHost, FuncBuilder0<FnVoid>());

    Label L_1(c);
    Label L_2(c);

    GpVar v0(c, kVarTypeUInt32, "v0");
    GpVar v1(c, kVarTypeUInt32, "v1");

    c.jmp(L_1);
    c.bind(L_2);
    c.mov(v0, 1);
    c.mov(v1, 2);
    c.cmp(v0, v1);
    c.jz(L_2);
    c.jmp(L_1);

    c.bind(L_1);
    c.ret();
    c.endFunc();
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect) {
    typedef void (*Func)(void);
    Func func = asmjit_cast<Func>(_func);

    func();

    result.appendString("ret={}");
    expect.appendString("ret={}");

    return true;
  }
};

// ============================================================================
// [X86Test_AllocBase]
// ============================================================================

struct X86Test_AllocBase : public X86Test {
  X86Test_AllocBase() : X86Test("[Alloc] Base") {}

  static void add(PodVector<X86Test*>& tests) {
    tests.append(new X86Test_AllocBase());
  }

  virtual void compile(Compiler& c) {
    c.addFunc(kFuncConvHost, FuncBuilder0<int>());

    GpVar v0(c, kVarTypeInt32, "v0");
    GpVar v1(c, kVarTypeInt32, "v1");
    GpVar v2(c, kVarTypeInt32, "v2");
    GpVar v3(c, kVarTypeInt32, "v3");
    GpVar v4(c, kVarTypeInt32, "v4");

    c.xor_(v0, v0);

    c.mov(v1, 1);
    c.mov(v2, 2);
    c.mov(v3, 3);
    c.mov(v4, 4);

    c.add(v0, v1);
    c.add(v0, v2);
    c.add(v0, v3);
    c.add(v0, v4);

    c.ret(v0);
    c.endFunc();
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect) {
    typedef int (*Func)(void);
    Func func = asmjit_cast<Func>(_func);

    int resultRet = func();
    int expectRet = 1 + 2 + 3 + 4;

    result.setFormat("ret=%d", resultRet);
    expect.setFormat("ret=%d", expectRet);

    return resultRet == expectRet;
  }
};

// ============================================================================
// [X86Test_AllocManual]
// ============================================================================

struct X86Test_AllocManual : public X86Test {
  X86Test_AllocManual() : X86Test("[Alloc] Manual alloc/spill") {}

  static void add(PodVector<X86Test*>& tests) {
    tests.append(new X86Test_AllocManual());
  }

  virtual void compile(Compiler& c) {
    c.addFunc(kFuncConvHost, FuncBuilder0<int>());

    GpVar v0(c, kVarTypeInt32, "v0");
    GpVar v1(c, kVarTypeInt32, "v0");
    GpVar cnt(c, kVarTypeInt32, "cnt");

    c.xor_(v0, v0);
    c.xor_(v1, v1);
    c.spill(v0);
    c.spill(v1);

    Label L(c);
    c.mov(cnt, 32);
    c.bind(L);

    c.inc(v1);
    c.add(v0, v1);

    c.dec(cnt);
    c.jnz(L);

    c.ret(v0);
    c.endFunc();
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect) {
    typedef int (*Func)(void);
    Func func = asmjit_cast<Func>(_func);

    int resultRet = func();
    int expectRet =
      0  +  1 +  2 +  3 +  4 +  5 +  6 +  7 +  8 +  9 +
      10 + 11 + 12 + 13 + 14 + 15 + 16 + 17 + 18 + 19 +
      20 + 21 + 22 + 23 + 24 + 25 + 26 + 27 + 28 + 29 +
      30 + 31 + 32;

    result.setFormat("ret=%d", resultRet);
    expect.setFormat("ret=%d", expectRet);

    return resultRet == expectRet;
  }
};

// ============================================================================
// [X86Test_AllocUseMem]
// ============================================================================

struct X86Test_AllocUseMem : public X86Test {
  X86Test_AllocUseMem() : X86Test("[Alloc] Alloc/use mem") {}

  static void add(PodVector<X86Test*>& tests) {
    tests.append(new X86Test_AllocUseMem());
  }

  virtual void compile(Compiler& c) {
    c.addFunc(kFuncConvHost, FuncBuilder2<int, int, int>());

    GpVar iIdx(c, kVarTypeInt32);
    GpVar iEnd(c, kVarTypeInt32);

    GpVar aIdx(c, kVarTypeInt32);
    GpVar aEnd(c, kVarTypeInt32);

    Label L_1(c);

    c.setArg(0, aIdx);
    c.setArg(1, aEnd);

    c.mov(iIdx, aIdx);
    c.mov(iEnd, aEnd);
    c.spill(iEnd);

    c.bind(L_1);
    c.inc(iIdx);
    c.cmp(iIdx, iEnd.m());
    c.jne(L_1);

    c.ret(iIdx);
    c.endFunc();
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect) {
    typedef int (*Func)(int, int);
    Func func = asmjit_cast<Func>(_func);

    int resultRet = func(10, 20);
    int expectRet = 20;

    result.setFormat("ret=%d", resultRet);
    expect.setFormat("ret=%d", expectRet);

    return resultRet == expectRet;
  }
};

// ============================================================================
// [X86Test_AllocMany1]
// ============================================================================

struct X86Test_AllocMany1 : public X86Test {
  X86Test_AllocMany1() : X86Test("[Alloc] Many #1") {}

  enum { kCount = 8 };

  static void add(PodVector<X86Test*>& tests) {
    tests.append(new X86Test_AllocMany1());
  }

  virtual void compile(Compiler& c) {
    c.addFunc(kFuncConvHost, FuncBuilder2<FnVoid, int*, int*>());

    GpVar a0(c, kVarTypeIntPtr, "a0");
    GpVar a1(c, kVarTypeIntPtr, "a1");

    c.setArg(0, a0);
    c.setArg(1, a1);

    // Create some variables.
    GpVar t(c, kVarTypeInt32);
    GpVar x[kCount];

    uint32_t i;
    for (i = 0; i < kCount; i++) {
      x[i] = c.newGpVar(kVarTypeInt32);
    }

    // Setup variables (use mov with reg/imm to se if register allocator works).
    for (i = 0; i < kCount; i++) {
      c.mov(x[i], static_cast<int>(i + 1));
    }

    // Make sum (addition).
    c.xor_(t, t);
    for (i = 0; i < kCount; i++) {
      c.add(t, x[i]);
    }

    // Store result to a given pointer in first argument.
    c.mov(dword_ptr(a0), t);

    // Clear t.
    c.xor_(t, t);

    // Make sum (subtraction).
    for (i = 0; i < kCount; i++) {
      c.sub(t, x[i]);
    }

    // Store result to a given pointer in second argument.
    c.mov(dword_ptr(a1), t);

    // End of function.
    c.endFunc();
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect) {
    typedef void (*Func)(int*, int*);
    Func func = asmjit_cast<Func>(_func);

    int resultX;
    int resultY;

    int expectX =  36;
    int expectY = -36;

    func(&resultX, &resultY);

    result.setFormat("ret={x=%d, y=%d}", resultX, resultY);
    expect.setFormat("ret={x=%d, y=%d}", expectX, expectY);

    return resultX == expectX && resultY == expectY;
  }
};

// ============================================================================
// [X86Test_AllocMany2]
// ============================================================================

struct X86Test_AllocMany2 : public X86Test {
  X86Test_AllocMany2() : X86Test("[Alloc] Many #2") {}

  static void add(PodVector<X86Test*>& tests) {
    tests.append(new X86Test_AllocMany2());
  }

  virtual void compile(Compiler& c) {
    c.addFunc(kFuncConvHost, FuncBuilder1<FnVoid, int*>());

    GpVar var[32];
    GpVar a(c, kVarTypeIntPtr, "a");

    c.setArg(0, a);

    uint32_t i;
    for (i = 0; i < ASMJIT_ARRAY_SIZE(var); i++) {
      var[i] = c.newGpVar(kVarTypeInt32);
    }

    for (i = 0; i < ASMJIT_ARRAY_SIZE(var); i++) {
      c.xor_(var[i], var[i]);
    }

    GpVar v0(c, kVarTypeInt32);
    Label L(c);

    c.mov(v0, 32);
    c.bind(L);

    for (i = 0; i < ASMJIT_ARRAY_SIZE(var); i++) {
      c.add(var[i], i);
    }

    c.dec(v0);
    c.jnz(L);

    for (i = 0; i < ASMJIT_ARRAY_SIZE(var); i++) {
      c.mov(dword_ptr(a, i * 4), var[i]);
    }

    c.endFunc();
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect) {
    typedef void (*Func)(int*);
    Func func = asmjit_cast<Func>(_func);

    int i;
    int resultBuf[32];
    int expectBuf[32];

    for (i = 0; i < ASMJIT_ARRAY_SIZE(resultBuf); i++) {
      expectBuf[i] = i * 32;
    }

    bool success = true;
    func(resultBuf);

    for (i = 0; i < ASMJIT_ARRAY_SIZE(resultBuf); i++) {
      result.appendFormat("%d", resultBuf[i]);
      expect.appendFormat("%d", expectBuf[1]);

      success &= (resultBuf[i] == expectBuf[i]);
    }

    return success;
  }
};

// ============================================================================
// [X86Test_AllocImul1]
// ============================================================================

struct X86Test_AllocImul1 : public X86Test {
  X86Test_AllocImul1() : X86Test("[Alloc] Imul #1") {}

  static void add(PodVector<X86Test*>& tests) {
    tests.append(new X86Test_AllocImul1());
  }

  virtual void compile(Compiler& c) {
    c.addFunc(kFuncConvHost, FuncBuilder4<FnVoid, int*, int*, int, int>());

    GpVar dstHi(c, kVarTypeIntPtr, "dstHi");
    GpVar dstLo(c, kVarTypeIntPtr, "dstLo");

    GpVar vHi(c, kVarTypeInt32, "vHi");
    GpVar vLo(c, kVarTypeInt32, "vLo");
    GpVar src(c, kVarTypeInt32, "src");

    c.setArg(0, dstHi);
    c.setArg(1, dstLo);
    c.setArg(2, vLo);
    c.setArg(3, src);

    c.imul(vHi, vLo, src);

    c.mov(dword_ptr(dstHi), vHi);
    c.mov(dword_ptr(dstLo), vLo);
    c.endFunc();
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect) {
    typedef void (*Func)(int*, int*, int, int);
    Func func = asmjit_cast<Func>(_func);

    int v0 = 4;
    int v1 = 4;

    int resultHi;
    int resultLo;

    int expectHi = 0;
    int expectLo = v0 * v1;

    func(&resultHi, &resultLo, v0, v1);

    result.setFormat("hi=%d, lo=%d", resultHi, resultLo);
    expect.setFormat("hi=%d, lo=%d", expectHi, expectLo);

    return resultHi == expectHi && resultLo == expectLo;
  }
};

// ============================================================================
// [X86Test_AllocImul2]
// ============================================================================

struct X86Test_AllocImul2 : public X86Test {
  X86Test_AllocImul2() : X86Test("[Alloc] Imul #2") {}

  static void add(PodVector<X86Test*>& tests) {
    tests.append(new X86Test_AllocImul2());
  }

  virtual void compile(Compiler& c) {
    c.addFunc(kFuncConvHost, FuncBuilder2<FnVoid, int*, const int*>());

    GpVar dst(c, kVarTypeIntPtr, "dst");
    GpVar src(c, kVarTypeIntPtr, "src");

    c.setArg(0, dst);
    c.setArg(1, src);

    for (unsigned int i = 0; i < 4; i++) {
      GpVar x(c, kVarTypeInt32, "x");
      GpVar y(c, kVarTypeInt32, "y");
      GpVar hi(c, kVarTypeInt32, "hi");

      c.mov(x, dword_ptr(src, 0));
      c.mov(y, dword_ptr(src, 4));

      c.imul(hi, x, y);
      c.add(dword_ptr(dst, 0), hi);
      c.add(dword_ptr(dst, 4), x);
    }

    c.endFunc();
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect) {
    typedef void (*Func)(int*, const int*);
    Func func = asmjit_cast<Func>(_func);

    int src[2] = { 4, 9 };
    int resultRet[2] = { 0, 0 };
    int expectRet[2] = { 0, (4 * 9) * 4 };

    func(resultRet, src);

    result.setFormat("ret={%d, %d}", resultRet[0], resultRet[1]);
    expect.setFormat("ret={%d, %d}", expectRet[0], expectRet[1]);

    return resultRet[0] == expectRet[0] && resultRet[1] == expectRet[1];
  }
};

// ============================================================================
// [X86Test_AllocSetz]
// ============================================================================

struct X86Test_AllocSetz : public X86Test {
  X86Test_AllocSetz() : X86Test("[Alloc] Setz") {}

  static void add(PodVector<X86Test*>& tests) {
    tests.append(new X86Test_AllocSetz());
  }

  virtual void compile(Compiler& c) {
    c.addFunc(kFuncConvHost, FuncBuilder3<FnVoid, int, int, char*>());

    GpVar src0(c, kVarTypeInt32, "src0");
    GpVar src1(c, kVarTypeInt32, "src1");
    GpVar dst0(c, kVarTypeIntPtr, "dst0");

    c.setArg(0, src0);
    c.setArg(1, src1);
    c.setArg(2, dst0);

    c.cmp(src0, src1);
    c.setz(byte_ptr(dst0));

    c.endFunc();
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect) {
    typedef void (*Func)(int, int, char*);
    Func func = asmjit_cast<Func>(_func);

    char resultBuf[4];
    char expectBuf[4] = { 1, 0, 0, 1 };

    func(0, 0, &resultBuf[0]); // We are expecting 1 (0 == 0).
    func(0, 1, &resultBuf[1]); // We are expecting 0 (0 != 1).
    func(1, 0, &resultBuf[2]); // We are expecting 0 (1 != 0).
    func(1, 1, &resultBuf[3]); // We are expecting 1 (1 == 1).

    result.setFormat("out={%d, %d, %d, %d}", resultBuf[0], resultBuf[1], resultBuf[2], resultBuf[3]);
    expect.setFormat("out={%d, %d, %d, %d}", expectBuf[0], expectBuf[1], expectBuf[2], expectBuf[3]);

    return resultBuf[0] == expectBuf[0] &&
           resultBuf[1] == expectBuf[1] &&
           resultBuf[2] == expectBuf[2] &&
           resultBuf[3] == expectBuf[3] ;
  }
};

// ============================================================================
// [X86Test_AllocShlRor]
// ============================================================================

struct X86Test_AllocShlRor : public X86Test {
  X86Test_AllocShlRor() : X86Test("[Alloc] Shl/Ror") {}

  static void add(PodVector<X86Test*>& tests) {
    tests.append(new X86Test_AllocShlRor());
  }

  virtual void compile(Compiler& c) {
    c.addFunc(kFuncConvHost, FuncBuilder4<FnVoid, int*, int, int, int>());

    GpVar dst(c, kVarTypeIntPtr, "dst");
    GpVar var(c, kVarTypeInt32, "var");
    GpVar vShlParam(c, kVarTypeInt32, "vShlParam");
    GpVar vRorParam(c, kVarTypeInt32, "vRorParam");

    c.setArg(0, dst);
    c.setArg(1, var);
    c.setArg(2, vShlParam);
    c.setArg(3, vRorParam);

    c.shl(var, vShlParam);
    c.ror(var, vRorParam);

    c.mov(dword_ptr(dst), var);
    c.endFunc();
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect) {
    typedef void (*Func)(int*, int, int, int);
    Func func = asmjit_cast<Func>(_func);

    int v0 = 0x000000FF;

    int resultRet;
    int expectRet = 0x0000FF00;

    func(&resultRet, v0, 16, 8);

    result.setFormat("ret=%d", resultRet);
    expect.setFormat("ret=%d", expectRet);

    return resultRet == expectRet;
  }
};

// ============================================================================
// [X86Test_AllocGpLo]
// ============================================================================

struct X86Test_AllocGpLo : public X86Test {
  X86Test_AllocGpLo() : X86Test("[Alloc] GP.LO") {}

  enum { kCount = 32 };

  static void add(PodVector<X86Test*>& tests) {
    tests.append(new X86Test_AllocGpLo());
  }

  virtual void compile(Compiler& c) {
    c.addFunc(kFuncConvHost, FuncBuilder1<uint32_t, uint32_t*>());

    GpVar rPtr(c, kVarTypeUIntPtr);
    GpVar rSum(c, kVarTypeUInt32);

    c.setArg(0, rPtr);

    GpVar rVar[kCount];
    uint32_t i;

    for (i = 0; i < kCount; i++) {
      rVar[i] = c.newGpVar(kVarTypeUInt32);
    }

    // Init pseudo-regs with values from our array.
    for (i = 0; i < kCount; i++) {
      c.mov(rVar[i], dword_ptr(rPtr, i * 4));
    }

    for (i = 2; i < kCount; i++) {
      // Add and truncate to 8 bit; no purpose, just mess with jit.
      c.add  (rVar[i  ], rVar[i-1]);
      c.movzx(rVar[i  ], rVar[i  ].r8());
      c.movzx(rVar[i-2], rVar[i-1].r8());
      c.movzx(rVar[i-1], rVar[i-2].r8());
    }

    // Sum up all computed values.
    c.mov(rSum, 0);
    for (i = 0; i < kCount; i++) {
      c.add(rSum, rVar[i]);
    }

    // Return the sum.
    c.ret(rSum);
    c.endFunc();
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect) {
    typedef int (*Func)(uint32_t*);
    Func func = asmjit_cast<Func>(_func);

    unsigned int i;

    uint32_t buf[kCount];
    uint32_t resultRet;
    uint32_t expectRet;

    expectRet = 0;
    for (i = 0; i < kCount; i++) {
      buf[i] = 1;
    }

    for (i = 2; i < kCount; i++) {
      buf[i  ]+= buf[i-1];
      buf[i  ] = buf[i  ] & 0xFF;
      buf[i-2] = buf[i-1] & 0xFF;
      buf[i-1] = buf[i-2] & 0xFF;
    }

    for (i = 0; i < kCount; i++) {
      expectRet += buf[i];
    }

    for (i = 0; i < kCount; i++) {
      buf[i] = 1;
    }
    resultRet = func(buf);

    result.setFormat("ret=%d", resultRet);
    expect.setFormat("ret=%d", expectRet);

    return resultRet == expectRet;
  }
};

// ============================================================================
// [X86Test_AllocRepMovsb]
// ============================================================================

struct X86Test_AllocRepMovsb : public X86Test {
  X86Test_AllocRepMovsb() : X86Test("[Alloc] Rep Movsb") {}

  static void add(PodVector<X86Test*>& tests) {
    tests.append(new X86Test_AllocSetz());
  }

  virtual void compile(Compiler& c) {
    c.addFunc(kFuncConvHost, FuncBuilder3<FnVoid, void*, void*, size_t>());

    GpVar dst(c, kVarTypeIntPtr, "dst");
    GpVar src(c, kVarTypeIntPtr, "src");
    GpVar cnt(c, kVarTypeIntPtr, "cnt");

    c.setArg(0, dst);
    c.setArg(1, src);
    c.setArg(2, cnt);

    c.rep_movsb(dst, src, cnt);
    c.endFunc();
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect) {
    typedef void (*Func)(void*, void*, size_t);
    Func func = asmjit_cast<Func>(_func);

    char dst[20];
    char src[20] = "Hello AsmJit!";
    func(dst, src, strlen(src) + 1);

    result.setFormat("ret=\"%s\"", dst);
    expect.setFormat("ret=\"%s\"", src);

    return ::memcmp(dst, src, strlen(src) + 1) == 0;
  }
};

// ============================================================================
// [X86Test_AllocIfElse1]
// ============================================================================

struct X86Test_AllocIfElse1 : public X86Test {
  X86Test_AllocIfElse1() : X86Test("[Alloc] If-Else #1") {}

  static void add(PodVector<X86Test*>& tests) {
    tests.append(new X86Test_AllocIfElse1());
  }

  virtual void compile(Compiler& c) {
    c.addFunc(kFuncConvHost, FuncBuilder2<int, int, int>());

    GpVar v1(c, kVarTypeInt32);
    GpVar v2(c, kVarTypeInt32);

    Label L_1(c);
    Label L_2(c);

    c.setArg(0, v1);
    c.setArg(1, v2);

    c.cmp(v1, v2);
    c.jg(L_1);

    c.mov(v1, 1);
    c.jmp(L_2);

    c.bind(L_1);
    c.mov(v1, 2);

    c.bind(L_2);
    c.ret(v1);
    c.endFunc();
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect)  {
    typedef int (*Func)(int, int);
    Func func = asmjit_cast<Func>(_func);

    int a = func(0, 1);
    int b = func(1, 0);

    result.appendFormat("ret={%d, %d}", a, b);
    result.appendFormat("ret={%d, %d}", 1, 2);

    return a == 1 && b == 2;
  }
};

// ============================================================================
// [X86Test_AllocIfElse2]
// ============================================================================

struct X86Test_AllocIfElse2 : public X86Test {
  X86Test_AllocIfElse2() : X86Test("[Alloc] If-Else #2") {}

  static void add(PodVector<X86Test*>& tests) {
    tests.append(new X86Test_AllocIfElse2());
  }

  virtual void compile(Compiler& c) {
    c.addFunc(kFuncConvHost, FuncBuilder2<int, int, int>());

    GpVar v1(c, kVarTypeInt32);
    GpVar v2(c, kVarTypeInt32);

    Label L_1(c);
    Label L_2(c);
    Label L_3(c);
    Label L_4(c);

    c.setArg(0, v1);
    c.setArg(1, v2);

    c.jmp(L_1);
    c.bind(L_2);
    c.jmp(L_4);
    c.bind(L_1);

    c.cmp(v1, v2);
    c.jg(L_3);

    c.mov(v1, 1);
    c.jmp(L_2);

    c.bind(L_3);
    c.mov(v1, 2);
    c.jmp(L_2);

    c.bind(L_4);

    c.ret(v1);
    c.endFunc();
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect)  {
    typedef int (*Func)(int, int);
    Func func = asmjit_cast<Func>(_func);

    int a = func(0, 1);
    int b = func(1, 0);

    result.appendFormat("ret={%d, %d}", a, b);
    result.appendFormat("ret={%d, %d}", 1, 2);

    return a == 1 && b == 2;
  }
};

// ============================================================================
// [X86Test_AllocIfElse3]
// ============================================================================

struct X86Test_AllocIfElse3 : public X86Test {
  X86Test_AllocIfElse3() : X86Test("[Alloc] If-Else #3") {}

  static void add(PodVector<X86Test*>& tests) {
    tests.append(new X86Test_AllocIfElse3());
  }

  virtual void compile(Compiler& c) {
    c.addFunc(kFuncConvHost, FuncBuilder2<int, int, int>());

    GpVar v1(c, kVarTypeInt32);
    GpVar v2(c, kVarTypeInt32);
    GpVar counter(c, kVarTypeInt32);

    Label L_1(c);
    Label L_Loop(c);
    Label L_Exit(c);

    c.setArg(0, v1);
    c.setArg(1, v2);

    c.cmp(v1, v2);
    c.jg(L_1);

    c.mov(counter, 0);

    c.bind(L_Loop);
    c.mov(v1, counter);

    c.inc(counter);
    c.cmp(counter, 1);
    c.jle(L_Loop);
    c.jmp(L_Exit);

    c.bind(L_1);
    c.mov(v1, 2);

    c.bind(L_Exit);
    c.ret(v1);
    c.endFunc();
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect)  {
    typedef int (*Func)(int, int);
    Func func = asmjit_cast<Func>(_func);

    int a = func(0, 1);
    int b = func(1, 0);

    result.appendFormat("ret={%d, %d}", a, b);
    result.appendFormat("ret={%d, %d}", 1, 2);

    return a == 1 && b == 2;
  }
};

// ============================================================================
// [X86Test_AllocIfElse4]
// ============================================================================

struct X86Test_AllocIfElse4 : public X86Test {
  X86Test_AllocIfElse4() : X86Test("[Alloc] If-Else #4") {}

  static void add(PodVector<X86Test*>& tests) {
    tests.append(new X86Test_AllocIfElse4());
  }

  virtual void compile(Compiler& c) {
    c.addFunc(kFuncConvHost, FuncBuilder2<int, int, int>());

    GpVar v1(c, kVarTypeInt32);
    GpVar v2(c, kVarTypeInt32);
    GpVar counter(c, kVarTypeInt32);

    Label L_1(c);
    Label L_Loop1(c);
    Label L_Loop2(c);
    Label L_Exit(c);

    c.mov(counter, 0);

    c.setArg(0, v1);
    c.setArg(1, v2);

    c.cmp(v1, v2);
    c.jg(L_1);

    c.bind(L_Loop1);
    c.mov(v1, counter);

    c.inc(counter);
    c.cmp(counter, 1);
    c.jle(L_Loop1);
    c.jmp(L_Exit);

    c.bind(L_1);
    c.bind(L_Loop2);
    c.mov(v1, counter);
    c.inc(counter);
    c.cmp(counter, 2);
    c.jle(L_Loop2);

    c.bind(L_Exit);
    c.ret(v1);
    c.endFunc();
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect)  {
    typedef int (*Func)(int, int);
    Func func = asmjit_cast<Func>(_func);

    int a = func(0, 1);
    int b = func(1, 0);

    result.appendFormat("ret={%d, %d}", a, b);
    result.appendFormat("ret={%d, %d}", 1, 2);

    return a == 1 && b == 2;
  }
};

// ============================================================================
// [X86Test_AllocArgsIntPtr]
// ============================================================================

struct X86Test_AllocArgsIntPtr : public X86Test {
  X86Test_AllocArgsIntPtr() : X86Test("[Alloc] Args IntPtr") {}

  static void add(PodVector<X86Test*>& tests) {
    tests.append(new X86Test_AllocArgsIntPtr());
  }

  virtual void compile(Compiler& c) {
    c.addFunc(kFuncConvHost,
      FuncBuilder8<FnVoid, void*, void*, void*, void*, void*, void*, void*, void*>());

    uint32_t i;
    GpVar var[8];

    for (i = 0; i < 8; i++) {
      var[i] = c.newGpVar();
      c.setArg(i, var[i]);
    }

    for (i = 0; i < 8; i++) {
      c.add(var[i], static_cast<int>(i + 1));
    }

    // Move some data into buffer provided by arguments so we can verify if it
    // really works without looking into assembler output.
    for (i = 0; i < 8; i++) {
      c.add(byte_ptr(var[i]), static_cast<int>(i + 1));
    }

    c.endFunc();
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect) {
    typedef void (*Func)(void*, void*, void*, void*, void*, void*, void*, void*);
    Func func = asmjit_cast<Func>(_func);

    uint8_t resultBuf[9] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    uint8_t expectBuf[9] = { 0, 1, 2, 3, 4, 5, 6, 7, 8 };

    func(resultBuf, resultBuf, resultBuf, resultBuf,
         resultBuf, resultBuf, resultBuf, resultBuf);

    result.setFormat("buf={%d, %d, %d, %d, %d, %d, %d, %d, %d}",
      resultBuf[0], resultBuf[1], resultBuf[2], resultBuf[3],
      resultBuf[4], resultBuf[5], resultBuf[6], resultBuf[7],
      resultBuf[8]);
    expect.setFormat("buf={%d, %d, %d, %d, %d, %d, %d, %d, %d}",
      expectBuf[0], expectBuf[1], expectBuf[2], expectBuf[3],
      expectBuf[4], expectBuf[5], expectBuf[6], expectBuf[7],
      expectBuf[8]);

    return ::memcmp(resultBuf, expectBuf, 9) == 0;
  }
};

// ============================================================================
// [X86Test_AllocArgsFloat]
// ============================================================================

struct X86Test_AllocArgsFloat : public X86Test {
  X86Test_AllocArgsFloat() : X86Test("[Alloc] Args Float") {}

  static void add(PodVector<X86Test*>& tests) {
    tests.append(new X86Test_AllocArgsFloat());
  }

  virtual void compile(Compiler& c) {
    c.addFunc(kFuncConvHost,
      FuncBuilder8<FnVoid, float, float, float, float, float, float, float, void*>());

    uint32_t i;

    XmmVar xv[7];
    GpVar p(c);

    for (i = 0; i < 7; i++) {
      xv[i] = c.newXmmVar(kVarTypeXmmSs);
      c.setArg(i, xv[i]);
    }

    c.setArg(7, p);

    c.addss(xv[0], xv[1]);
    c.addss(xv[0], xv[2]);
    c.addss(xv[0], xv[3]);
    c.addss(xv[0], xv[4]);
    c.addss(xv[0], xv[5]);
    c.addss(xv[0], xv[6]);

    c.movss(ptr(p), xv[0]);
    c.endFunc();
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect) {
    typedef void (*Func)(float, float, float, float, float, float, float, float*);
    Func func = asmjit_cast<Func>(_func);

    float resultRet;
    float expectRet = 1.0f + 2.0f + 3.0f + 4.0f + 5.0f + 6.0f + 7.0f;

    func(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, &resultRet);

    result.setFormat("ret={%g}", resultRet);
    expect.setFormat("ret={%g}", expectRet);

    return resultRet == expectRet;
  }
};

// ============================================================================
// [X86Test_AllocArgsDouble]
// ============================================================================

struct X86Test_AllocArgsDouble : public X86Test {
  X86Test_AllocArgsDouble() : X86Test("[Alloc] Args Double") {}

  static void add(PodVector<X86Test*>& tests) {
    tests.append(new X86Test_AllocArgsDouble());
  }

  virtual void compile(Compiler& c) {
    c.addFunc(kFuncConvHost,
      FuncBuilder8<FnVoid, double, double, double, double, double, double, double, void*>());

    uint32_t i;

    XmmVar xv[7];
    GpVar p(c);

    for (i = 0; i < 7; i++) {
      xv[i] = c.newXmmVar(kVarTypeXmmSd);
      c.setArg(i, xv[i]);
    }

    c.setArg(7, p);

    c.addsd(xv[0], xv[1]);
    c.addsd(xv[0], xv[2]);
    c.addsd(xv[0], xv[3]);
    c.addsd(xv[0], xv[4]);
    c.addsd(xv[0], xv[5]);
    c.addsd(xv[0], xv[6]);

    c.movsd(ptr(p), xv[0]);
    c.endFunc();
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect) {
    typedef void (*Func)(double, double, double, double, double, double, double, double*);
    Func func = asmjit_cast<Func>(_func);

    double resultRet;
    double expectRet = 1.0 + 2.0 + 3.0 + 4.0 + 5.0 + 6.0 + 7.0;

    func(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, &resultRet);

    result.setFormat("ret={%g}", resultRet);
    expect.setFormat("ret={%g}", expectRet);

    return resultRet == expectRet;
  }
};

// ============================================================================
// [X86Test_AllocRetFloat]
// ============================================================================

struct X86Test_AllocRetFloat : public X86Test {
  X86Test_AllocRetFloat() : X86Test("[Alloc] Ret Float") {}

  static void add(PodVector<X86Test*>& tests) {
    tests.append(new X86Test_AllocRetFloat());
  }

  virtual void compile(Compiler& c) {
    c.addFunc(kFuncConvHost, FuncBuilder2<float, float, float>());

    XmmVar a(c, kVarTypeXmmSs);
    XmmVar b(c, kVarTypeXmmSs);

    c.setArg(0, a);
    c.setArg(1, b);

    c.addss(a, b);
    c.ret(a);

    c.endFunc();
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect) {
    typedef float (*Func)(float, float);
    Func func = asmjit_cast<Func>(_func);

    float resultRet = func(1.0f, 2.0f);
    float expectRet = 1.0f + 2.0f;

    result.setFormat("ret={%g}", resultRet);
    expect.setFormat("ret={%g}", expectRet);

    return resultRet == expectRet;
  }
};

// ============================================================================
// [X86Test_AllocRetDouble]
// ============================================================================

struct X86Test_AllocRetDouble : public X86Test {
  X86Test_AllocRetDouble() : X86Test("[Alloc] Ret Double") {}

  static void add(PodVector<X86Test*>& tests) {
    tests.append(new X86Test_AllocRetDouble());
  }

  virtual void compile(Compiler& c) {
    c.addFunc(kFuncConvHost, FuncBuilder2<double, double, double>());

    XmmVar a(c, kVarTypeXmmSd);
    XmmVar b(c, kVarTypeXmmSd);

    c.setArg(0, a);
    c.setArg(1, b);

    c.addsd(a, b);
    c.ret(a);

    c.endFunc();
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect) {
    typedef double (*Func)(double, double);
    Func func = asmjit_cast<Func>(_func);

    double resultRet = func(1.0, 2.0);
    double expectRet = 1.0 + 2.0;

    result.setFormat("ret={%g}", resultRet);
    expect.setFormat("ret={%g}", expectRet);

    return resultRet == expectRet;
  }
};

// ============================================================================
// [X86Test_AllocStack]
// ============================================================================

struct X86Test_AllocStack : public X86Test {
  X86Test_AllocStack() : X86Test("[Alloc] Stack") {}

  enum { kSize = 256 };

  static void add(PodVector<X86Test*>& tests) {
    tests.append(new X86Test_AllocStack());
  }

  virtual void compile(Compiler& c) {
    c.addFunc(kFuncConvHost, FuncBuilder0<int>());

    Mem stack = c.newStack(kSize, 1).setSize(1);
    GpVar i(c, kVarTypeIntPtr, "i");
    GpVar a(c, kVarTypeInt32, "a");
    GpVar b(c, kVarTypeInt32, "b");

    Label L_1(c);
    Label L_2(c);

    // Fill stack by sequence [0, 1, 2, 3 ... 255].
    c.xor_(i, i);

    c.bind(L_1);
    c.mov(stack.clone().setIndex(i, 0), i.r8());
    c.inc(i);
    c.cmp(i, 255);
    c.jle(L_1);

    // Sum sequence in stack.
    c.xor_(i, i);
    c.xor_(a, a);

    c.bind(L_2);
    c.movzx(b, stack.clone().setIndex(i, 0));
    c.add(a, b);
    c.inc(i);
    c.cmp(i, 255);
    c.jle(L_2);

    c.ret(a);
    c.endFunc();
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect) {
    typedef int (*Func)(void);
    Func func = asmjit_cast<Func>(_func);

    int resultRet = func();
    int expectRet = 32640;

    result.setInt(resultRet);
    expect.setInt(expectRet);

    return resultRet == expectRet;
  }
};

// ============================================================================
// [X86Test_AllocMemcpy]
// ============================================================================

struct X86Test_AllocMemcpy : public X86Test {
  X86Test_AllocMemcpy() : X86Test("[Alloc] Memcpy") {}

  enum { kCount = 32 };

  static void add(PodVector<X86Test*>& tests) {
    tests.append(new X86Test_AllocMemcpy());
  }

  virtual void compile(Compiler& c) {
    GpVar dst(c, kVarTypeIntPtr, "dst");
    GpVar src(c, kVarTypeIntPtr, "src");
    GpVar cnt(c, kVarTypeUIntPtr, "cnt");

    Label L_Loop(c);                               // Create base labels we use
    Label L_Exit(c);                               // in our function.

    c.addFunc(kFuncConvHost, FuncBuilder3<FnVoid, uint32_t*, const uint32_t*, size_t>());
    c.setArg(0, dst);
    c.setArg(1, src);
    c.setArg(2, cnt);

    c.alloc(dst);                                  // Allocate all registers now,
    c.alloc(src);                                  // because we want to keep them
    c.alloc(cnt);                                  // in physical registers only.

    c.test(cnt, cnt);                              // Exit if length is zero.
    c.jz(L_Exit);

    c.bind(L_Loop);                                // Bind the loop label here.

    GpVar tmp(c, kVarTypeInt32);                   // Copy a single dword (4 bytes).
    c.mov(tmp, dword_ptr(src));
    c.mov(dword_ptr(dst), tmp);

    c.add(src, 4);                                 // Increment dst/src pointers.
    c.add(dst, 4);

    c.dec(cnt);                                    // Loop until cnt isn't zero.
    c.jnz(L_Loop);

    c.bind(L_Exit);                                // Bind the exit label here.
    c.endFunc();                                   // End of function.
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect) {
    typedef void (*Func)(uint32_t*, const uint32_t*, size_t);
    Func func = asmjit_cast<Func>(_func);

    uint32_t i;

    uint32_t dstBuffer[kCount];
    uint32_t srcBuffer[kCount];

    for (i = 0; i < kCount; i++) {
      dstBuffer[i] = 0;
      srcBuffer[i] = i;
    }

    func(dstBuffer, srcBuffer, kCount);

    result.setString("buf={");
    expect.setString("buf={");

    for (i = 0; i < kCount; i++) {
      if (i != 0) {
        result.appendString(", ");
        expect.appendString(", ");
      }

      result.appendFormat("%u", static_cast<unsigned int>(dstBuffer[i]));
      expect.appendFormat("%u", static_cast<unsigned int>(srcBuffer[i]));
    }

    result.appendString("}");
    expect.appendString("}");

    return ::memcmp(dstBuffer, srcBuffer, kCount * sizeof(uint32_t)) == 0;
  }
};

// ============================================================================
// [X86Test_AllocBlend]
// ============================================================================

struct X86Test_AllocBlend : public X86Test {
  X86Test_AllocBlend() : X86Test("[Alloc] Blend") {}

  enum { kCount = 17 };

  static void add(PodVector<X86Test*>& tests) {
    tests.append(new X86Test_AllocBlend());
  }

  static uint32_t blendSrcOver(uint32_t d, uint32_t s) {
    uint32_t saInv = ~s >> 24;

    uint32_t d_20 = (d     ) & 0x00FF00FF;
    uint32_t d_31 = (d >> 8) & 0x00FF00FF;

    d_20 *= saInv;
    d_31 *= saInv;

    d_20 = ((d_20 + ((d_20 >> 8) & 0x00FF00FFU) + 0x00800080U) & 0xFF00FF00U) >> 8;
    d_31 = ((d_31 + ((d_31 >> 8) & 0x00FF00FFU) + 0x00800080U) & 0xFF00FF00U);

    return d_20 + d_31 + s;
  }

  virtual void compile(Compiler& c) {
    asmgen::blend(c);
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect) {
    typedef void (*Func)(void*, const void*, size_t);
    Func func = asmjit_cast<Func>(_func);

    uint32_t i;

    uint32_t dstBuffer[kCount] = { 0x00000000, 0x10101010, 0x20100804, 0x30200003, 0x40204040, 0x5000004D, 0x60302E2C, 0x706F6E6D, 0x807F4F2F, 0x90349001, 0xA0010203, 0xB03204AB, 0xC023AFBD, 0xD0D0D0C0, 0xE0AABBCC, 0xFFFFFFFF, 0xF8F4F2F1 };
    uint32_t srcBuffer[kCount] = { 0xE0E0E0E0, 0xA0008080, 0x341F1E1A, 0xFEFEFEFE, 0x80302010, 0x49490A0B, 0x998F7798, 0x00000000, 0x01010101, 0xA0264733, 0xBAB0B1B9, 0xFF000000, 0xDAB0A0C1, 0xE0BACFDA, 0x99887766, 0xFFFFFF80, 0xEE0A5FEC };
    uint32_t expBuffer[kCount];

    for (i = 0; i < kCount; i++) {
      expBuffer[i] = blendSrcOver(dstBuffer[i], srcBuffer[i]);
    }

    func(dstBuffer, srcBuffer, kCount);

    result.setString("buf={");
    expect.setString("buf={");

    for (i = 0; i < kCount; i++) {
      if (i != 0) {
        result.appendString(", ");
        expect.appendString(", ");
      }

      result.appendFormat("%0.8X", static_cast<unsigned int>(dstBuffer[i]));
      expect.appendFormat("%0.8X", static_cast<unsigned int>(expBuffer[i]));
    }

    result.appendString("}");
    expect.appendString("}");

    return ::memcmp(expBuffer, dstBuffer, kCount * sizeof(uint32_t)) == 0;
  }
};

// ============================================================================
// [X86Test_CallBase]
// ============================================================================

struct X86Test_CallBase : public X86Test {
  X86Test_CallBase() : X86Test("[Call] CDecl") {}

  static void add(PodVector<X86Test*>& tests) {
    tests.append(new X86Test_CallBase());
  }

  virtual void compile(Compiler& c) {
    GpVar v0(c, kVarTypeInt32, "v0");
    GpVar v1(c, kVarTypeInt32, "v1");
    GpVar v2(c, kVarTypeInt32, "v2");

    c.addFunc(kFuncConvHost, FuncBuilder3<int, int, int, int>());
    c.setArg(0, v0);
    c.setArg(1, v1);
    c.setArg(2, v2);

    // Just do something.
    c.shl(v0, 1);
    c.shl(v1, 1);
    c.shl(v2, 1);

    // Call function.
    GpVar fn(c, kVarTypeIntPtr, "fn");
    c.mov(fn, imm_ptr((void*)calledFunc));

    X86X64CallNode* call = c.call(fn, kFuncConvHost, FuncBuilder3<int, int, int, int>());
    call->setArg(0, v2);
    call->setArg(1, v1);
    call->setArg(2, v0);
    call->setRet(0, v0);

    c.ret(v0);
    c.endFunc();
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect) {
    typedef int (*Func)(int, int, int);
    Func func = asmjit_cast<Func>(_func);

    int resultRet = func(3, 2, 1);
    int expectRet = 36;

    result.setFormat("ret=%d", resultRet);
    expect.setFormat("ret=%d", expectRet);

    return resultRet == expectRet;
  }

  static int calledFunc(int a, int b, int c) { return (a + b) * c; }
};

// ============================================================================
// [X86Test_CallFast]
// ============================================================================

struct X86Test_CallFast : public X86Test {
  X86Test_CallFast() : X86Test("[Call] Fastcall") {}

  static void add(PodVector<X86Test*>& tests) {
    tests.append(new X86Test_CallFast());
  }

  virtual void compile(Compiler& c) {
    GpVar var(c, kVarTypeInt32, "var");
    GpVar fn(c, kVarTypeIntPtr, "fn");

    c.addFunc(kFuncConvHost, FuncBuilder1<int, int>());
    c.setArg(0, var);

    c.mov(fn, imm_ptr((void*)calledFunc));
    X86X64CallNode* call;

    call = c.call(fn, kFuncConvHostFastCall, FuncBuilder1<int, int>());
    call->setArg(0, var);
    call->setRet(0, var);

    call = c.call(fn, kFuncConvHostFastCall, FuncBuilder1<int, int>());
    call->setArg(0, var);
    call->setRet(0, var);

    c.ret(var);
    c.endFunc();
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect) {
    typedef int (*Func)(int);
    Func func = asmjit_cast<Func>(_func);

    int resultRet = func(9);
    int expectRet = (9 * 9) * (9 * 9);

    result.setFormat("ret=%d", resultRet);
    expect.setFormat("ret=%d", expectRet);

    return resultRet == expectRet;
  }

  // Function that is called inside the generated one. Because this test is
  // mainly about register arguments, we need to use the fastcall calling
  // convention when running 32-bit.
  static int ASMJIT_FASTCALL calledFunc(int a) { return a * a; }
};

// ============================================================================
// [X86Test_CallManyArgs]
// ============================================================================

struct X86Test_CallManyArgs : public X86Test {
  X86Test_CallManyArgs() : X86Test("[Call] Many Args") {}

  static void add(PodVector<X86Test*>& tests) {
    tests.append(new X86Test_CallManyArgs());
  }

  static int calledFunc(int a, int b, int c, int d, int e, int f, int g, int h, int i, int j) {
    return (a * b * c * d * e) + (f * g * h * i * j);
  }

  virtual void compile(Compiler& c) {
    c.addFunc(kFuncConvHost, FuncBuilder0<int>());

    // Prepare.
    GpVar fn(c, kVarTypeIntPtr, "fn");
    GpVar va(c, kVarTypeInt32, "va");
    GpVar vb(c, kVarTypeInt32, "vb");
    GpVar vc(c, kVarTypeInt32, "vc");
    GpVar vd(c, kVarTypeInt32, "vd");
    GpVar ve(c, kVarTypeInt32, "ve");
    GpVar vf(c, kVarTypeInt32, "vf");
    GpVar vg(c, kVarTypeInt32, "vg");
    GpVar vh(c, kVarTypeInt32, "vh");
    GpVar vi(c, kVarTypeInt32, "vi");
    GpVar vj(c, kVarTypeInt32, "vj");

    c.mov(fn, imm_ptr((void*)calledFunc));
    c.mov(va, 0x03);
    c.mov(vb, 0x12);
    c.mov(vc, 0xA0);
    c.mov(vd, 0x0B);
    c.mov(ve, 0x2F);
    c.mov(vf, 0x02);
    c.mov(vg, 0x0C);
    c.mov(vh, 0x12);
    c.mov(vi, 0x18);
    c.mov(vj, 0x1E);

    // Call function.
    X86X64CallNode* call = c.call(fn, kFuncConvHost,
      FuncBuilder10<int, int, int, int, int, int, int, int, int, int, int>());
    call->setArg(0, va);
    call->setArg(1, vb);
    call->setArg(2, vc);
    call->setArg(3, vd);
    call->setArg(4, ve);
    call->setArg(5, vf);
    call->setArg(6, vg);
    call->setArg(7, vh);
    call->setArg(8, vi);
    call->setArg(9, vj);
    call->setRet(0, va);

    c.ret(va);
    c.endFunc();
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect) {
    typedef int (*Func)(void);
    Func func = asmjit_cast<Func>(_func);

    int resultRet = func();
    int expectRet = calledFunc(0x03, 0x12, 0xA0, 0x0B, 0x2F, 0x02, 0x0C, 0x12, 0x18, 0x1E);

    result.setFormat("ret=%d", resultRet);
    expect.setFormat("ret=%d", expectRet);

    return resultRet == expectRet;
  }
};

// ============================================================================
// [X86Test_CallDuplicateArgs]
// ============================================================================

struct X86Test_CallDuplicateArgs : public X86Test {
  X86Test_CallDuplicateArgs() : X86Test("[Call] Duplicate Args") {}

  static void add(PodVector<X86Test*>& tests) {
    tests.append(new X86Test_CallDuplicateArgs());
  }

  static int calledFunc(int a, int b, int c, int d, int e, int f, int g, int h, int i, int j) {
    return (a * b * c * d * e) + (f * g * h * i * j);
  }

  virtual void compile(Compiler& c) {
    c.addFunc(kFuncConvHost, FuncBuilder0<int>());

    // Prepare.
    GpVar fn(c, kVarTypeIntPtr, "fn");
    GpVar a(c, kVarTypeInt32, "a");

    c.mov(fn, imm_ptr((void*)calledFunc));
    c.mov(a, 3);

    // Call function.
    X86X64CallNode* call = c.call(fn, kFuncConvHost,
      FuncBuilder10<int, int, int, int, int, int, int, int, int, int, int>());
    call->setArg(0, a);
    call->setArg(1, a);
    call->setArg(2, a);
    call->setArg(3, a);
    call->setArg(4, a);
    call->setArg(5, a);
    call->setArg(6, a);
    call->setArg(7, a);
    call->setArg(8, a);
    call->setArg(9, a);
    call->setRet(0, a);

    c.ret(a);
    c.endFunc();
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect) {
    typedef int (*Func)(void);
    Func func = asmjit_cast<Func>(_func);

    int resultRet = func();
    int expectRet = calledFunc(3, 3, 3, 3, 3, 3, 3, 3, 3, 3);

    result.setFormat("ret=%d", resultRet);
    expect.setFormat("ret=%d", expectRet);

    return resultRet == expectRet;
  }
};

// ============================================================================
// [X86Test_CallImmArgs]
// ============================================================================

struct X86Test_CallImmArgs : public X86Test {
  X86Test_CallImmArgs() : X86Test("[Call] Imm Args") {}

  static void add(PodVector<X86Test*>& tests) {
    tests.append(new X86Test_CallImmArgs());
  }

  virtual void compile(Compiler& c) {
    c.addFunc(kFuncConvHost, FuncBuilder0<int>());

    // Prepare.
    GpVar fn(c, kVarTypeIntPtr, "fn");
    GpVar rv(c, kVarTypeInt32, "rv");

    c.mov(fn, imm_ptr((void*)X86Test_CallManyArgs::calledFunc));

    // Call function.
    X86X64CallNode* call = c.call(fn, kFuncConvHost,
      FuncBuilder10<int, int, int, int, int, int, int, int, int, int, int>());
    call->setArg(0, imm(0x03));
    call->setArg(1, imm(0x12));
    call->setArg(2, imm(0xA0));
    call->setArg(3, imm(0x0B));
    call->setArg(4, imm(0x2F));
    call->setArg(5, imm(0x02));
    call->setArg(6, imm(0x0C));
    call->setArg(7, imm(0x12));
    call->setArg(8, imm(0x18));
    call->setArg(9, imm(0x1E));
    call->setRet(0, rv);

    c.ret(rv);
    c.endFunc();
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect) {
    typedef int (*Func)(void);
    Func func = asmjit_cast<Func>(_func);

    int resultRet = func();
    int expectRet = X86Test_CallManyArgs::calledFunc(0x03, 0x12, 0xA0, 0x0B, 0x2F, 0x02, 0x0C, 0x12, 0x18, 0x1E);

    result.setFormat("ret=%d", resultRet);
    expect.setFormat("ret=%d", expectRet);

    return resultRet == expectRet;
  }
};

// ============================================================================
// [X86Test_CallFloatAsXmmRet]
// ============================================================================

struct X86Test_CallFloatAsXmmRet : public X86Test {
  X86Test_CallFloatAsXmmRet() : X86Test("[Call] Float As Xmm Ret") {}

  static void add(PodVector<X86Test*>& tests) {
    tests.append(new X86Test_CallFloatAsXmmRet());
  }

  static float calledFunc(float a, float b) {
    return a * b;
  }

  virtual void compile(Compiler& c) {
    c.addFunc(kFuncConvHost, FuncBuilder2<float, float, float>());

    XmmVar a(c, kVarTypeXmmSs, "a");
    XmmVar b(c, kVarTypeXmmSs, "b");
    XmmVar ret(c, kVarTypeXmmSs, "ret");

    c.setArg(0, a);
    c.setArg(1, b);

    // Prepare.
    GpVar fn(c);
    c.mov(fn, imm_ptr((void*)calledFunc));

    // Call function.
    X86X64CallNode* call = c.call(fn, kFuncConvHost,
      FuncBuilder2<float, float, float>());

    call->setArg(0, a);
    call->setArg(1, b);
    call->setRet(0, ret);

    c.ret(ret);
    c.endFunc();
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect) {
    typedef float (*Func)(float, float);
    Func func = asmjit_cast<Func>(_func);

    float resultRet = func(15.5f, 2.0f);
    float expectRet = calledFunc(15.5f, 2.0f);

    result.setFormat("ret=%g", resultRet);
    expect.setFormat("ret=%g", expectRet);

    return resultRet == expectRet;
  }
};

// ============================================================================
// [X86Test_CallDoubleAsXmmRet]
// ============================================================================

struct X86Test_CallDoubleAsXmmRet : public X86Test {
  X86Test_CallDoubleAsXmmRet() : X86Test("[Call] Double As Xmm Ret") {}

  static void add(PodVector<X86Test*>& tests) {
    tests.append(new X86Test_CallDoubleAsXmmRet());
  }

  static double calledFunc(double a, double b) {
    return a * b;
  }

  virtual void compile(Compiler& c) {
    c.addFunc(kFuncConvHost, FuncBuilder2<double, double, double>());

    XmmVar a(c, kVarTypeXmmSd, "a");
    XmmVar b(c, kVarTypeXmmSd, "b");
    XmmVar ret(c, kVarTypeXmmSd, "ret");

    c.setArg(0, a);
    c.setArg(1, b);

    GpVar fn(c);
    c.mov(fn, imm_ptr((void*)calledFunc));

    X86X64CallNode* call = c.call(fn, kFuncConvHost,
      FuncBuilder2<double, double, double>());

    call->setArg(0, a);
    call->setArg(1, b);
    call->setRet(0, ret);

    c.ret(ret);
    c.endFunc();
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect) {
    typedef double (*Func)(double, double);
    Func func = asmjit_cast<Func>(_func);

    double resultRet = func(15.5, 2.0);
    double expectRet = calledFunc(15.5, 2.0);

    result.setFormat("ret=%g", resultRet);
    expect.setFormat("ret=%g", expectRet);

    return resultRet == expectRet;
  }
};

// ============================================================================
// [X86Test_CallConditional]
// ============================================================================

struct X86Test_CallConditional : public X86Test {
  X86Test_CallConditional() : X86Test("[Call] Conditional") {}

  static void add(PodVector<X86Test*>& tests) {
    tests.append(new X86Test_CallConditional());
  }

  virtual void compile(Compiler& c) {
    GpVar x(c, kVarTypeInt32, "x");
    GpVar y(c, kVarTypeInt32, "y");
    GpVar op(c, kVarTypeInt32, "op");

    X86X64CallNode* call;
    GpVar result;

    c.addFunc(kFuncConvHost, FuncBuilder3<int, int, int, int>());
    c.setArg(0, x);
    c.setArg(1, y);
    c.setArg(2, op);

    Label opAdd(c);
    Label opMul(c);

    c.cmp(op, 0);
    c.jz(opAdd);
    c.cmp(op, 1);
    c.jz(opMul);

    result = c.newGpVar(kVarTypeInt32, "result");
    c.mov(result, 0);
    c.ret(result);

    c.bind(opAdd);
    result = c.newGpVar(kVarTypeInt32, "result");

    call = c.call((void*)calledFuncAdd, kFuncConvHost, FuncBuilder2<int, int, int>());
    call->setArg(0, x);
    call->setArg(1, y);
    call->setRet(0, result);
    c.ret(result);

    c.bind(opMul);
    result = c.newGpVar(kVarTypeInt32, "result");

    call = c.call((void*)calledFuncMul, kFuncConvHost, FuncBuilder2<int, int, int>());
    call->setArg(0, x);
    call->setArg(1, y);
    call->setRet(0, result);

    c.ret(result);
    c.endFunc();
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect) {
    typedef int (*Func)(int, int, int);
    Func func = asmjit_cast<Func>(_func);

    int arg1 = 4;
    int arg2 = 8;

    int resultAdd = func(arg1, arg2, 0);
    int expectAdd = calledFuncAdd(arg1, arg2);

    int resultMul = func(arg1, arg2, 1);
    int expectMul = calledFuncMul(arg1, arg2);

    result.setFormat("ret={add=%d, mul=%d}", resultAdd, resultMul);
    expect.setFormat("ret={add=%d, mul=%d}", expectAdd, expectMul);

    return (resultAdd == expectAdd) && (resultMul == expectMul);
  }

  static int calledFuncAdd(int x, int y) { return x + y; }
  static int calledFuncMul(int x, int y) { return x * y; }
};

// ============================================================================
// [X86Test_CallMultiple]
// ============================================================================

struct X86Test_CallMultiple : public X86Test {
  X86Test_CallMultiple() : X86Test("[Call] Multiple") {}

  static void add(PodVector<X86Test*>& tests) {
    tests.append(new X86Test_CallMultiple());
  }

  static int ASMJIT_FASTCALL calledFunc(int* pInt, int index) {
    return pInt[index];
  }

  virtual void compile(Compiler& c) {
    unsigned int i;

    GpVar buf(c, kVarTypeIntPtr, "buf");
    GpVar acc0(c, kVarTypeInt32, "acc0");
    GpVar acc1(c, kVarTypeInt32, "acc1");

    c.addFunc(kFuncConvHost, FuncBuilder1<int, int*>());
    c.setArg(0, buf);

    c.mov(acc0, 0);
    c.mov(acc1, 0);

    for (i = 0; i < 4; i++) {
      GpVar ret(c, kVarTypeInt32);
      GpVar ptr(c, kVarTypeIntPtr);
      GpVar idx(c, kVarTypeInt32);
      X86X64CallNode* call;

      c.mov(ptr, buf);
      c.mov(idx, static_cast<int>(i));

      call = c.call((void*)calledFunc, kFuncConvHostFastCall, FuncBuilder2<int, int*, int>());
      call->setArg(0, ptr);
      call->setArg(1, idx);
      call->setRet(0, ret);

      c.add(acc0, ret);

      c.mov(ptr, buf);
      c.mov(idx, static_cast<int>(i));

      call = c.call((void*)calledFunc, kFuncConvHostFastCall, FuncBuilder2<int, int*, int>());
      call->setArg(0, ptr);
      call->setArg(1, idx);
      call->setRet(0, ret);

      c.sub(acc1, ret);
    }

    c.add(acc0, acc1);
    c.ret(acc0);
    c.endFunc();
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect) {
    typedef int (*Func)(int*);
    Func func = asmjit_cast<Func>(_func);

    int buffer[4] = { 127, 87, 23, 17 };

    int resultRet = func(buffer);
    int expectRet = 0;

    result.setFormat("ret=%d", resultRet);
    expect.setFormat("ret=%d", expectRet);

    return resultRet == expectRet;
  }
};

// ============================================================================
// [X86Test_CallRecursive]
// ============================================================================

struct X86Test_CallRecursive : public X86Test {
  X86Test_CallRecursive() : X86Test("[Call] Recursive") {}

  static void add(PodVector<X86Test*>& tests) {
    tests.append(new X86Test_CallRecursive());
  }

  virtual void compile(Compiler& c) {
    GpVar val(c, kVarTypeInt32, "val");
    Label skip(c);

    X86X64FuncNode* func = c.addFunc(kFuncConvHost, FuncBuilder1<int, int>());
    c.setArg(0, val);

    c.cmp(val, 1);
    c.jle(skip);

    GpVar tmp(c, kVarTypeInt32, "tmp");
    c.mov(tmp, val);
    c.dec(tmp);

    X86X64CallNode* call = c.call(func->getEntryLabel(), kFuncConvHost, FuncBuilder1<int, int>());
    call->setArg(0, tmp);
    call->setRet(0, tmp);
    c.mul(c.newGpVar(kVarTypeInt32), val, tmp);

    c.bind(skip);
    c.ret(val);
    c.endFunc();
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect) {
    typedef int (*Func)(int);
    Func func = asmjit_cast<Func>(_func);

    int resultRet = func(5);
    int expectRet = 1 * 2 * 3 * 4 * 5;

    result.setFormat("ret=%d", resultRet);
    expect.setFormat("ret=%d", expectRet);

    return resultRet == expectRet;
  }
};

// ============================================================================
// [X86Test_Dummy]
// ============================================================================

struct X86Test_Dummy : public X86Test {
  X86Test_Dummy() : X86Test("[Dummy] Dummy") {}

  static void add(PodVector<X86Test*>& tests) {
    tests.append(new X86Test_Dummy());
  }

  virtual void compile(Compiler& c) {
    c.addFunc(kFuncConvHost, FuncBuilder0<uint32_t>());

    GpVar r(c, kVarTypeUInt32);
    GpVar a(c, kVarTypeUInt32);
    GpVar b(c, kVarTypeUInt32);

    c.alloc(r, eax);
    c.alloc(a, ecx);
    c.alloc(b, edx);

    c.mov(a, 16);
    c.mov(b, 99);

    c.mul(r, a, b);
    c.alloc(a, esi);
    c.alloc(b, ecx);
    c.alloc(r, edi);
    c.mul(a, b, r);

    c.ret(b);
    c.endFunc();
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect) {
    typedef uint32_t (*Func)(void);
    Func func = asmjit_cast<Func>(_func);

    return func() == 0;
  }
};

// ============================================================================
// [X86TestSuite]
// ============================================================================

struct X86TestSuite {
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  X86TestSuite();
  ~X86TestSuite();

  // --------------------------------------------------------------------------
  // [Methods]
  // --------------------------------------------------------------------------

  int run();

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  PodVector<X86Test*> tests;
  StringBuilder output;

  int result;
  int binSize;
  bool alwaysPrintLog;
};

#define ADD_TEST(_Class_) \
  _Class_::add(tests)

X86TestSuite::X86TestSuite() :
  result(EXIT_SUCCESS),
  binSize(0),
  alwaysPrintLog(false) {

  // Align.
  ADD_TEST(X86Test_AlignBase);

  // Jump.
  ADD_TEST(X86Test_JumpCross);
  ADD_TEST(X86Test_JumpUnreachable1);
  ADD_TEST(X86Test_JumpUnreachable2);

  // Alloc.
  ADD_TEST(X86Test_AllocBase);
  ADD_TEST(X86Test_AllocManual);
  ADD_TEST(X86Test_AllocUseMem);
  ADD_TEST(X86Test_AllocMany1);
  ADD_TEST(X86Test_AllocMany2);
  ADD_TEST(X86Test_AllocImul1);
  ADD_TEST(X86Test_AllocImul2);
  ADD_TEST(X86Test_AllocSetz);
  ADD_TEST(X86Test_AllocShlRor);
  ADD_TEST(X86Test_AllocGpLo);
  ADD_TEST(X86Test_AllocRepMovsb);
  ADD_TEST(X86Test_AllocIfElse1);
  ADD_TEST(X86Test_AllocIfElse2);
  ADD_TEST(X86Test_AllocIfElse3);
  ADD_TEST(X86Test_AllocIfElse4);
  ADD_TEST(X86Test_AllocArgsIntPtr);
  ADD_TEST(X86Test_AllocArgsFloat);
  ADD_TEST(X86Test_AllocArgsDouble);
  ADD_TEST(X86Test_AllocRetFloat);
  ADD_TEST(X86Test_AllocRetDouble);
  ADD_TEST(X86Test_AllocStack);
  ADD_TEST(X86Test_AllocMemcpy);
  ADD_TEST(X86Test_AllocBlend);

  // Call.
  ADD_TEST(X86Test_CallBase);
  ADD_TEST(X86Test_CallFast);
  ADD_TEST(X86Test_CallManyArgs);
  ADD_TEST(X86Test_CallDuplicateArgs);
  ADD_TEST(X86Test_CallImmArgs);
  ADD_TEST(X86Test_CallFloatAsXmmRet);
  ADD_TEST(X86Test_CallDoubleAsXmmRet);
  ADD_TEST(X86Test_CallConditional);
  ADD_TEST(X86Test_CallMultiple);
  ADD_TEST(X86Test_CallRecursive);

  // Dummy.
  // ADD_TEST(X86Test_Dummy);
}

X86TestSuite::~X86TestSuite() {
  size_t i;
  size_t count = tests.getLength();

  for (i = 0; i < count; i++) {
    X86Test* test = tests[i];
    delete test;
  }
}

int X86TestSuite::run() {
  size_t i;
  size_t count = tests.getLength();

  FILE* file = stdout;

  FileLogger fileLogger(file);
  fileLogger.setOption(kLoggerOptionBinaryForm, true);

  StringLogger stringLogger;
  stringLogger.setOption(kLoggerOptionBinaryForm, true);

  for (i = 0; i < count; i++) {
    JitRuntime runtime;
    Compiler compiler(&runtime);

    if (alwaysPrintLog) {
      fprintf(file, "\n");
      compiler.setLogger(&fileLogger);
    }
    else {
      stringLogger.clearString();
      compiler.setLogger(&stringLogger);
    }

    X86Test* test = tests[i];
    test->compile(compiler);

    void* func = compiler.make();

    if (alwaysPrintLog) {
      fflush(file);
    }

    if (func != NULL) {
      StringBuilder result;
      StringBuilder expect;

      if (test->run(func, result, expect)) {
        fprintf(file, "[Success] %s.\n", test->getName());
      }
      else {
        if (!alwaysPrintLog) {
          fprintf(file, "\n%s", stringLogger.getString());
        }

        fprintf(file, "-------------------------------------------------------------------------------\n");
        fprintf(file, "[Failure] %s.\n", test->getName());
        fprintf(file, "-------------------------------------------------------------------------------\n");
        fprintf(file, "Result  : %s\n", result.getData());
        fprintf(file, "Expected: %s\n", expect.getData());
        fprintf(file, "===============================================================================\n");
      }

      runtime.release(func);
    }
    else {
      if (!alwaysPrintLog) {
        fprintf(file, "%s\n", stringLogger.getString());
      }

      fprintf(file, "-------------------------------------------------------------------------------\n");
      fprintf(file, "[Failure] %s.\n", test->getName());
      fprintf(file, "===============================================================================\n");
    }

    fflush(file);
  }

  fputs("\n", file);
  fputs(output.getData(), file);
  fflush(file);

  return result;
}

// ============================================================================
// [CmdLine]
// ============================================================================

struct CmdLine {
  ASMJIT_INLINE CmdLine(int argc, char** argv) :
    _argc(argc),
    _argv(argv) {}

  bool hasArg(const char* arg) {
    for (int i = 1; i < _argc; i++) {
      if (::strcmp(_argv[i], arg) == 0)
        return true;
    }
    return false;
  }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  int _argc;
  char** _argv;
};

// ============================================================================
// [Main]
// ============================================================================

int main(int argc, char* argv[]) {
  X86TestSuite testSuite;
  CmdLine cmd(argc, argv);

  if (cmd.hasArg("--always-print-log")) {
    testSuite.alwaysPrintLog = true;
  }

  return testSuite.run();
}
