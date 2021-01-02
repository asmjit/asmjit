// AsmJit - Machine code generation for C++
//
//  * Official AsmJit Home Page: https://asmjit.com
//  * Official Github Repository: https://github.com/asmjit/asmjit
//
// Copyright (c) 2008-2020 The AsmJit Authors
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include <asmjit/core.h>
#if defined(ASMJIT_BUILD_ARM) && ASMJIT_ARCH_ARM == 64

#include <asmjit/a64.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./asmjit_test_compiler.h"

using namespace asmjit;

// ============================================================================
// [A64TestCase]
// ============================================================================

class A64TestCase : public TestCase {
public:
  A64TestCase(const char* name = nullptr)
    : TestCase(name) {}

  virtual void compile(BaseCompiler& cc) override {
    compile(static_cast<a64::Compiler&>(cc));
  }

  virtual void compile(a64::Compiler& cc) = 0;
};

// ============================================================================
// [A64Test_GpArgs]
// ============================================================================

class A64Test_GpArgs : public A64TestCase {
public:
  uint32_t _argCount;
  bool _preserveFP;

  A64Test_GpArgs(uint32_t argCount, bool preserveFP)
    : _argCount(argCount),
      _preserveFP(preserveFP) {
    _name.assignFormat("GpArgs {NumArgs=%u PreserveFP=%c}", argCount, preserveFP ? 'Y' : 'N');
  }

  static void add(TestApp& app) {
    for (uint32_t i = 0; i <= 16; i++) {
      app.add(new A64Test_GpArgs(i, true));
      app.add(new A64Test_GpArgs(i, false));
    }
  }

  virtual void compile(a64::Compiler& cc) {
    uint32_t i;
    uint32_t argCount = _argCount;

    FuncSignatureBuilder signature;
    signature.setRetT<int>();
    for (i = 0; i < argCount; i++)
      signature.addArgT<int>();

    cc.addFunc(signature);
    if (_preserveFP)
      cc.func()->frame().setPreservedFP();

    arm::Gp sum;

    if (argCount) {
      for (i = 0; i < argCount; i++) {
        arm::Gp iReg = cc.newInt32("i%u", i);
        cc.setArg(i, iReg);

        if (i == 0)
          sum = iReg;
        else
          cc.add(sum, sum, iReg);
      }
    }
    else {
      sum = cc.newInt32("i");
      cc.mov(sum, 0);
    }

    cc.ret(sum);
    cc.endFunc();
  }

  virtual bool run(void* _func, String& result, String& expect) {
    typedef unsigned int U;

    typedef U (*Func0)();
    typedef U (*Func1)(U);
    typedef U (*Func2)(U, U);
    typedef U (*Func3)(U, U, U);
    typedef U (*Func4)(U, U, U, U);
    typedef U (*Func5)(U, U, U, U, U);
    typedef U (*Func6)(U, U, U, U, U, U);
    typedef U (*Func7)(U, U, U, U, U, U, U);
    typedef U (*Func8)(U, U, U, U, U, U, U, U);
    typedef U (*Func9)(U, U, U, U, U, U, U, U, U);
    typedef U (*Func10)(U, U, U, U, U, U, U, U, U, U);
    typedef U (*Func11)(U, U, U, U, U, U, U, U, U, U, U);
    typedef U (*Func12)(U, U, U, U, U, U, U, U, U, U, U, U);
    typedef U (*Func13)(U, U, U, U, U, U, U, U, U, U, U, U, U);
    typedef U (*Func14)(U, U, U, U, U, U, U, U, U, U, U, U, U, U);
    typedef U (*Func15)(U, U, U, U, U, U, U, U, U, U, U, U, U, U, U);
    typedef U (*Func16)(U, U, U, U, U, U, U, U, U, U, U, U, U, U, U, U);

    unsigned int resultRet = 0;
    unsigned int expectRet = 0;

    switch (_argCount) {
      case 0:
        resultRet = ptr_as_func<Func0>(_func)();
        expectRet = 0;
        break;
      case 1:
        resultRet = ptr_as_func<Func1>(_func)(1);
        expectRet = 1;
        break;
      case 2:
        resultRet = ptr_as_func<Func2>(_func)(1, 2);
        expectRet = 1 + 2;
        break;
      case 3:
        resultRet = ptr_as_func<Func3>(_func)(1, 2, 3);
        expectRet = 1 + 2 + 3;
        break;
      case 4:
        resultRet = ptr_as_func<Func4>(_func)(1, 2, 3, 4);
        expectRet = 1 + 2 + 3 + 4;
        break;
      case 5:
        resultRet = ptr_as_func<Func5>(_func)(1, 2, 3, 4, 5);
        expectRet = 1 + 2 + 3 + 4 + 5;
        break;
      case 6:
        resultRet = ptr_as_func<Func6>(_func)(1, 2, 3, 4, 5, 6);
        expectRet = 1 + 2 + 3 + 4 + 5 + 6;
        break;
      case 7:
        resultRet = ptr_as_func<Func7>(_func)(1, 2, 3, 4, 5, 6, 7);
        expectRet = 1 + 2 + 3 + 4 + 5 + 6 + 7;
        break;
      case 8:
        resultRet = ptr_as_func<Func8>(_func)(1, 2, 3, 4, 5, 6, 7, 8);
        expectRet = 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8;
        break;
      case 9:
        resultRet = ptr_as_func<Func9>(_func)(1, 2, 3, 4, 5, 6, 7, 8, 9);
        expectRet = 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9;
        break;
      case 10:
        resultRet = ptr_as_func<Func10>(_func)(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
        expectRet = 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10;
        break;
      case 11:
        resultRet = ptr_as_func<Func11>(_func)(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);
        expectRet = 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10 + 11;
        break;
      case 12:
        resultRet = ptr_as_func<Func12>(_func)(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12);
        expectRet = 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10 + 11 + 12;
        break;
      case 13:
        resultRet = ptr_as_func<Func13>(_func)(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13);
        expectRet = 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10 + 11 + 12 + 13;
        break;
      case 14:
        resultRet = ptr_as_func<Func14>(_func)(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14);
        expectRet = 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10 + 11 + 12 + 13 + 14;
        break;
      case 15:
        resultRet = ptr_as_func<Func15>(_func)(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
        expectRet = 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10 + 11 + 12 + 13 + 14 + 15;
        break;
      case 16:
        resultRet = ptr_as_func<Func16>(_func)(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
        expectRet = 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10 + 11 + 12 + 13 + 14 + 15 + 16;
        break;
    }

    result.assignFormat("ret={%u, %u}", resultRet >> 28, resultRet & 0x0FFFFFFFu);
    expect.assignFormat("ret={%u, %u}", expectRet >> 28, expectRet & 0x0FFFFFFFu);

    return resultRet == expectRet;
  }
};

// ============================================================================
// [A64Test_Simd1]
// ============================================================================

class A64Test_Simd1 : public A64TestCase {
public:
  A64Test_Simd1()
    : A64TestCase("Simd1") {}

  static void add(TestApp& app) {
    app.add(new A64Test_Simd1());
  }

  virtual void compile(a64::Compiler& cc) {
    cc.addFunc(FuncSignatureT<void, void*, const void*, const void*>());

    arm::Gp dst = cc.newUIntPtr("dst");
    arm::Gp src1 = cc.newUIntPtr("src1");
    arm::Gp src2 = cc.newUIntPtr("src2");

    cc.setArg(0, dst);
    cc.setArg(1, src1);
    cc.setArg(2, src2);

    arm::Vec v1 = cc.newVecQ("vec1");
    arm::Vec v2 = cc.newVecQ("vec2");
    arm::Vec v3 = cc.newVecQ("vec3");

    cc.ldr(v2, arm::ptr(src1));
    cc.ldr(v3, arm::ptr(src2));
    cc.add(v1.b16(), v2.b16(), v3.b16());
    cc.str(v1, arm::ptr(dst));

    cc.endFunc();
  }

  virtual bool run(void* _func, String& result, String& expect) {
    typedef void (*Func)(void*, const void*, const void*);

    uint32_t dst[4];
    uint32_t aSrc[4] = { 0 , 1 , 2 , 255 };
    uint32_t bSrc[4] = { 99, 17, 33, 1   };

    // NOTE: It's a byte-add, so uint8_t(255+1) == 0.
    uint32_t ref[4] = { 99, 18, 35, 0 };

    ptr_as_func<Func>(_func)(dst, aSrc, bSrc);

    unsigned int resultRet = 0;
    unsigned int expectRet = 0;

    result.assignFormat("ret={%u, %u, %u, %u}", dst[0], dst[1], dst[2], dst[3]);
    expect.assignFormat("ret={%u, %u, %u, %u}", ref[0], ref[1], ref[2], ref[3]);

    return resultRet == expectRet;
  }
};

// ============================================================================
// [A64Test_ManyRegs]
// ============================================================================

class A64Test_ManyRegs : public A64TestCase {
public:
  uint32_t _regCount;

  A64Test_ManyRegs(uint32_t n)
    : A64TestCase(),
      _regCount(n) {
    _name.assignFormat("GpRegs {NumRegs=%u}", n);
  }

  static void add(TestApp& app) {
    for (uint32_t i = 2; i < 64; i++)
      app.add(new A64Test_ManyRegs(i));
  }

  virtual void compile(a64::Compiler& cc) {
    cc.addFunc(FuncSignatureT<int>());

    arm::Gp* regs = static_cast<arm::Gp*>(malloc(_regCount * sizeof(arm::Gp)));

    for (uint32_t i = 0; i < _regCount; i++) {
      regs[i] = cc.newUInt32("reg%u", i);
      cc.mov(regs[i], i + 1);
    }

    arm::Gp sum = cc.newUInt32("sum");
    cc.mov(sum, 0);

    for (uint32_t i = 0; i < _regCount; i++) {
      cc.add(sum, sum, regs[i]);
    }

    cc.ret(sum);
    cc.endFunc();

    free(regs);
  }

  virtual bool run(void* _func, String& result, String& expect) {
    typedef int (*Func)(void);
    Func func = ptr_as_func<Func>(_func);

    result.assignFormat("ret={%d}", func());
    expect.assignFormat("ret={%d}", calcSum());

    return result == expect;
  }

  uint32_t calcSum() const {
    return (_regCount | 1) * ((_regCount + 1) / 2);
  }
};

// ============================================================================
// [A64Test_Adr]
// ============================================================================

class A64Test_Adr : public A64TestCase {
public:
  A64Test_Adr()
    : A64TestCase("Adr") {}

  static void add(TestApp& app) {
    app.add(new A64Test_Adr());
  }

  virtual void compile(a64::Compiler& cc) {
    cc.addFunc(FuncSignatureT<int>());

    arm::Gp addr = cc.newIntPtr("addr");
    arm::Gp val = cc.newIntPtr("val");

    Label L_Table = cc.newLabel();

    cc.adr(addr, L_Table);
    cc.ldrsw(val, arm::ptr(addr, 8));
    cc.ret(val);
    cc.endFunc();

    cc.bind(L_Table);
    cc.embedInt32(1);
    cc.embedInt32(2);
    cc.embedInt32(3);
    cc.embedInt32(4);
    cc.embedInt32(5);
  }

  virtual bool run(void* _func, String& result, String& expect) {
    typedef int (*Func)(void);
    Func func = ptr_as_func<Func>(_func);

    result.assignFormat("ret={%d}", func());
    expect.assignFormat("ret={%d}", 3);

    return result == expect;
  }
};


// ============================================================================
// [A64Test_CallFunc1]
// ============================================================================

class A64Test_CallFunc1 : public A64TestCase {
public:
  A64Test_CallFunc1()
    : A64TestCase("CallFunc1") {}

  static void add(TestApp& app) {
    app.add(new A64Test_CallFunc1());
  }

  virtual void compile(a64::Compiler& cc) {
    cc.addFunc(FuncSignatureT<uint32_t, uint32_t, uint32_t>());

    arm::Gp x = cc.newUInt32("x");
    arm::Gp y = cc.newUInt32("y");
    arm::Gp r = cc.newUInt32("r");
    arm::Gp fn = cc.newUIntPtr("fn");

    cc.setArg(0, x);
    cc.setArg(1, y);

    cc.mov(fn, (uint64_t)calledFunc);

    InvokeNode* invokeNode;
    cc.invoke(&invokeNode, fn, FuncSignatureT<uint32_t, uint32_t, uint32_t>(CallConv::kIdHost));
    invokeNode->setArg(0, x);
    invokeNode->setArg(1, y);
    invokeNode->setRet(0, r);

    cc.ret(r);
    cc.endFunc();
  }

  virtual bool run(void* _func, String& result, String& expect) {
    typedef uint32_t (*Func)(uint32_t, uint32_t);
    Func func = ptr_as_func<Func>(_func);

    uint32_t x = 49;
    uint32_t y = 7;

    result.assignFormat("ret={%u}", func(x, y));
    expect.assignFormat("ret={%u}", x - y);

    return result == expect;
  }

  static uint32_t calledFunc(uint32_t x, uint32_t y) {
    return x - y;
  }
};

// ============================================================================
// [A64Test_JumpTable]
// ============================================================================

class A64Test_JumpTable : public A64TestCase {
public:
  bool _annotated;

  A64Test_JumpTable(bool annotated)
    : A64TestCase("A64Test_JumpTable"),
      _annotated(annotated) {
    _name.assignFormat("JumpTable {%s}", annotated ? "Annotated" : "Unknown Target");
  }

  enum Operator {
    kOperatorAdd = 0,
    kOperatorSub = 1,
    kOperatorMul = 2,
    kOperatorDiv = 3
  };

  static void add(TestApp& app) {
    app.add(new A64Test_JumpTable(false));
    app.add(new A64Test_JumpTable(true));
  }

  virtual void compile(a64::Compiler& cc) {
    cc.addFunc(FuncSignatureT<float, float, float, uint32_t>());

    arm::Vec a = cc.newVecS("a");
    arm::Vec b = cc.newVecS("b");
    arm::Gp op = cc.newUInt32("op");

    arm::Gp target = cc.newIntPtr("target");
    arm::Gp offset = cc.newIntPtr("offset");

    Label L_End = cc.newLabel();

    Label L_Table = cc.newLabel();
    Label L_Add = cc.newLabel();
    Label L_Sub = cc.newLabel();
    Label L_Mul = cc.newLabel();
    Label L_Div = cc.newLabel();

    cc.setArg(0, a);
    cc.setArg(1, b);
    cc.setArg(2, op);

    cc.adr(target, L_Table);
    cc.ldrsw(offset, arm::ptr(target, op, arm::sxtw(2)));
    cc.add(target, target, offset);

    // JumpAnnotation allows to annotate all possible jump targets of
    // instructions where it cannot be deduced from operands.
    if (_annotated) {
      JumpAnnotation* annotation = cc.newJumpAnnotation();
      annotation->addLabel(L_Add);
      annotation->addLabel(L_Sub);
      annotation->addLabel(L_Mul);
      annotation->addLabel(L_Div);
      cc.br(target, annotation);
    }
    else {
      cc.br(target);
    }

    cc.bind(L_Add);
    cc.fadd(a, a, b);
    cc.b(L_End);

    cc.bind(L_Sub);
    cc.fsub(a, a, b);
    cc.b(L_End);

    cc.bind(L_Mul);
    cc.fmul(a, a, b);
    cc.b(L_End);

    cc.bind(L_Div);
    cc.fdiv(a, a, b);

    cc.bind(L_End);
    cc.ret(a);
    cc.endFunc();

    cc.bind(L_Table);
    cc.embedLabelDelta(L_Add, L_Table, 4);
    cc.embedLabelDelta(L_Sub, L_Table, 4);
    cc.embedLabelDelta(L_Mul, L_Table, 4);
    cc.embedLabelDelta(L_Div, L_Table, 4);
  }

  virtual bool run(void* _func, String& result, String& expect) {
    typedef float (*Func)(float, float, uint32_t);
    Func func = ptr_as_func<Func>(_func);

    float dst[4];
    float ref[4];

    dst[0] = func(33.0f, 14.0f, kOperatorAdd);
    dst[1] = func(33.0f, 14.0f, kOperatorSub);
    dst[2] = func(10.0f, 6.0f, kOperatorMul);
    dst[3] = func(80.0f, 8.0f, kOperatorDiv);

    ref[0] = 47.0f;
    ref[1] = 19.0f;
    ref[2] = 60.0f;
    ref[3] = 10.0f;

    result.assignFormat("ret={%f, %f, %f, %f}", dst[0], dst[1], dst[2], dst[3]);
    expect.assignFormat("ret={%f, %f, %f, %f}", ref[0], ref[1], ref[2], ref[3]);

    return result == expect;
  }
};

// ============================================================================
// [Export]
// ============================================================================

void compiler_add_a64_tests(TestApp& app) {
  app.addT<A64Test_GpArgs>();
  app.addT<A64Test_ManyRegs>();
  app.addT<A64Test_Simd1>();
  app.addT<A64Test_Adr>();
  app.addT<A64Test_CallFunc1>();
  app.addT<A64Test_JumpTable>();
}

#endif
