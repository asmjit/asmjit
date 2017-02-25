// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Dependencies]
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>

#include "./asmjit.h"
#include "./asmjit_test_misc.h"

using namespace asmjit;

// ============================================================================
// [MyErrorHandler]
// ============================================================================

class MyErrorHandler : public ErrorHandler {
public:
  virtual bool handleError(Error err, const char* message, CodeEmitter* origin) {
    fprintf(stderr, "ERROR: %s\n", message);
    return false;
  }
};

// ============================================================================
// [X86Test]
// ============================================================================

//! Interface used to test CodeCompiler.
class X86Test {
public:
  X86Test(const char* name = NULL) { _name.setString(name); }
  virtual ~X86Test() {}

  ASMJIT_INLINE const char* getName() const { return _name.getData(); }

  virtual void compile(X86Compiler& c) = 0;
  virtual bool run(void* func, StringBuilder& result, StringBuilder& expect) = 0;

  StringBuilder _name;
};

// ============================================================================
// [X86TestManager]
// ============================================================================

class X86TestManager {
public:
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  X86TestManager();
  ~X86TestManager();

  // --------------------------------------------------------------------------
  // [Methods]
  // --------------------------------------------------------------------------

  inline Error add(X86Test* test) { return _tests.append(&_zoneHeap, test); }

  int run();

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  Zone _zone;
  ZoneHeap _zoneHeap;
  ZoneVector<X86Test*> _tests;

  int _returnCode;
  int _binSize;
  bool _verbose;
  StringBuilder _output;
};

X86TestManager::X86TestManager() :
  _zone(8096 - Zone::kZoneOverhead),
  _zoneHeap(&_zone),
  _returnCode(0),
  _binSize(0),
  _verbose(false) {}

X86TestManager::~X86TestManager() {
  size_t i;
  size_t count = _tests.getLength();

  for (i = 0; i < count; i++) {
    X86Test* test = _tests[i];
    delete test;
  }
}

int X86TestManager::run() {
  size_t i;
  size_t count = _tests.getLength();

  FILE* file = stdout;

#if !defined(ASMJIT_DISABLE_LOGGING)
  FileLogger fileLogger(file);
  fileLogger.addOptions(Logger::kOptionBinaryForm);

  StringLogger stringLogger;
  stringLogger.addOptions(Logger::kOptionBinaryForm);
#endif // ASMJIT_DISABLE_LOGGING

  MyErrorHandler errorHandler;

  for (i = 0; i < count; i++) {
    JitRuntime runtime;

    CodeHolder code;
    code.init(runtime.getCodeInfo());
    code.setErrorHandler(&errorHandler);

#if !defined(ASMJIT_DISABLE_LOGGING)
    if (_verbose) {
      fprintf(file, "\n");
      code.setLogger(&fileLogger);
    }
    else {
      stringLogger.clearString();
      code.setLogger(&stringLogger);
    }
#endif // ASMJIT_DISABLE_LOGGING

    X86Compiler cc(&code);
    X86Test* test = _tests[i];
    test->compile(cc);

    Error err = cc.finalize();
    void* func;

    if (err == kErrorOk)
      err = runtime.add(&func, &code);
    if (_verbose) fflush(file);

    if (err == kErrorOk) {
      StringBuilder result;
      StringBuilder expect;

      if (test->run(func, result, expect)) {
        fprintf(file, "[Success] %s.\n", test->getName());
      }
      else {
#if !defined(ASMJIT_DISABLE_LOGGING)
        if (!_verbose)
          fprintf(file, "\n%s", stringLogger.getString());
#endif // ASMJIT_DISABLE_LOGGING

        fprintf(file, "-------------------------------------------------------------------------------\n");
        fprintf(file, "[Failure] %s.\n", test->getName());
        fprintf(file, "-------------------------------------------------------------------------------\n");
        fprintf(file, "Result  : %s\n", result.getData());
        fprintf(file, "Expected: %s\n", expect.getData());
        fprintf(file, "===============================================================================\n");

        _returnCode = 1;
      }

      runtime.release(func);
    }
    else {
#if !defined(ASMJIT_DISABLE_LOGGING)
      if (!_verbose)
        fprintf(file, "%s\n", stringLogger.getString());
#endif // ASMJIT_DISABLE_LOGGING

      fprintf(file, "-------------------------------------------------------------------------------\n");
      fprintf(file, "[Failure] %s (%s).\n", test->getName(), DebugUtils::errorAsString(err));
      fprintf(file, "===============================================================================\n");

      _returnCode = 1;
    }

    fflush(file);
  }

  fputs("\n", file);
  fputs(_output.getData(), file);
  fflush(file);

  return _returnCode;
}

// ============================================================================
// [X86Test_AlignBase]
// ============================================================================

class X86Test_AlignBase : public X86Test {
public:
  X86Test_AlignBase(uint32_t numArgs, uint32_t numVars, uint32_t alignment, bool naked) :
    _numArgs(numArgs),
    _numVars(numVars),
    _alignment(alignment),
    _naked(naked) {

    _name.setFormat("[Align] NumArgs=%u NumVars=%u Alignment=%u Naked=%c",
      numArgs, numVars, alignment, naked ? 'Y' : 'N');
  }

  static void add(X86TestManager& mgr) {
    for (uint32_t i = 0; i <= 8; i++) {
      for (uint32_t j = 0; j <= 4; j++) {
        for (uint32_t a = 16; a <= 32; a += 16) {
          mgr.add(new X86Test_AlignBase(i, j, a, false));
          mgr.add(new X86Test_AlignBase(i, j, a, true));
        }
      }
    }
  }

  virtual void compile(X86Compiler& cc) {
    switch (_numArgs) {
      case 0: cc.addFunc(FuncSignature0<int>(CallConv::kIdHost)); break;
      case 1: cc.addFunc(FuncSignature1<int, int>(CallConv::kIdHost)); break;
      case 2: cc.addFunc(FuncSignature2<int, int, int>(CallConv::kIdHost)); break;
      case 3: cc.addFunc(FuncSignature3<int, int, int, int>(CallConv::kIdHost)); break;
      case 4: cc.addFunc(FuncSignature4<int, int, int, int, int>(CallConv::kIdHost)); break;
      case 5: cc.addFunc(FuncSignature5<int, int, int, int, int, int>(CallConv::kIdHost)); break;
      case 6: cc.addFunc(FuncSignature6<int, int, int, int, int, int, int>(CallConv::kIdHost)); break;
      case 7: cc.addFunc(FuncSignature7<int, int, int, int, int, int, int, int>(CallConv::kIdHost)); break;
      case 8: cc.addFunc(FuncSignature8<int, int, int, int, int, int, int, int, int>(CallConv::kIdHost)); break;
    }

    if (!_naked)
      cc.getFunc()->getFrameInfo().enablePreservedFP();

    X86Gp gpVar = cc.newIntPtr("gpVar");
    X86Gp gpSum = cc.newInt32("gpSum");
    X86Mem stack = cc.newStack(_alignment, _alignment);

    // Alloc, use and spill preserved registers.
    if (_numVars) {
      uint32_t gpCount = cc.getGpCount();
      uint32_t varIndex = 0;
      uint32_t physId = 0;
      uint32_t regMask = 0x1;
      uint32_t preservedMask = cc.getFunc()->getDetail().getPreservedRegs(Reg::kKindGp);

      do {
        if ((preservedMask & regMask) != 0 && (physId != X86Gp::kIdSp && physId != X86Gp::kIdBp)) {
          X86Gp tmp = cc.newInt32("gpTmp%u", physId);
          cc.alloc(tmp, physId);
          cc.xor_(tmp, tmp);
          cc.spill(tmp);
          varIndex++;
        }

        physId++;
        regMask <<= 1;
      } while (varIndex < _numVars && physId < gpCount);
    }

    // Do a sum of arguments to verify a possible relocation when misaligned.
    if (_numArgs) {
      cc.xor_(gpSum, gpSum);
      for (uint32_t argIndex = 0; argIndex < _numArgs; argIndex++) {
        X86Gp gpArg = cc.newInt32("gpArg%u", argIndex);

        cc.setArg(argIndex, gpArg);
        cc.add(gpSum, gpArg);
      }
    }

    // Check alignment of xmmVar (has to be 16).
    cc.lea(gpVar, stack);
    cc.and_(gpVar, _alignment - 1);

    // Add a sum of arguments to check whether they are correct.
    if (_numArgs)
      cc.or_(gpVar.r32(), gpSum);

    cc.ret(gpVar);
    cc.endFunc();
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect) {
    typedef int (*Func0)();
    typedef int (*Func1)(int);
    typedef int (*Func2)(int, int);
    typedef int (*Func3)(int, int, int);
    typedef int (*Func4)(int, int, int, int);
    typedef int (*Func5)(int, int, int, int, int);
    typedef int (*Func6)(int, int, int, int, int, int);
    typedef int (*Func7)(int, int, int, int, int, int, int);
    typedef int (*Func8)(int, int, int, int, int, int, int, int);

    unsigned int resultRet = 0;
    unsigned int expectRet = 0;

    switch (_numArgs) {
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
    }

    result.setFormat("ret={%u, %u}", resultRet >> 28, resultRet & 0x0FFFFFFFU);
    expect.setFormat("ret={%u, %u}", expectRet >> 28, expectRet & 0x0FFFFFFFU);

    return resultRet == expectRet;
  }

  uint32_t _numArgs;
  uint32_t _numVars;
  uint32_t _alignment;

  bool _naked;
};

// ============================================================================
// [X86Test_AlignNone]
// ============================================================================

class X86Test_AlignNone : public X86Test {
public:
  X86Test_AlignNone() : X86Test("[Align] None") {}

  static void add(X86TestManager& mgr) {
    mgr.add(new X86Test_AlignNone());
  }

  virtual void compile(X86Compiler& cc) {
    cc.addFunc(FuncSignature0<void>(CallConv::kIdHost));
    cc.align(kAlignCode, 0);
    cc.align(kAlignCode, 1);
    cc.endFunc();
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect) {
    typedef void (*Func)(void);
    Func func = ptr_as_func<Func>(_func);

    func();
    return true;
  }
};

// ============================================================================
// [X86Test_JumpCross]
// ============================================================================

class X86Test_JumpCross : public X86Test {
public:
  X86Test_JumpCross() : X86Test("[Jump] Cross jump") {}

  static void add(X86TestManager& mgr) {
    mgr.add(new X86Test_JumpCross());
  }

  virtual void compile(X86Compiler& cc) {
    cc.addFunc(FuncSignature0<void>(CallConv::kIdHost));

    Label L1 = cc.newLabel();
    Label L2 = cc.newLabel();
    Label L3 = cc.newLabel();

    cc.jmp(L2);

    cc.bind(L1);
    cc.jmp(L3);

    cc.bind(L2);
    cc.jmp(L1);

    cc.bind(L3);

    cc.ret();
    cc.endFunc();
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect) {
    typedef void (*Func)(void);
    Func func = ptr_as_func<Func>(_func);

    func();
    return true;
  }
};

// ============================================================================
// [X86Test_JumpMany]
// ============================================================================

class X86Test_JumpMany : public X86Test {
public:
  X86Test_JumpMany() : X86Test("[Jump] Many jumps") {}

  static void add(X86TestManager& mgr) {
    mgr.add(new X86Test_JumpMany());
  }

  virtual void compile(X86Compiler& cc) {
    cc.addFunc(FuncSignature0<int>(CallConv::kIdHost));
    for (uint32_t i = 0; i < 1000; i++) {
      Label L = cc.newLabel();
      cc.jmp(L);
      cc.bind(L);
    }

    X86Gp ret = cc.newInt32("ret");
    cc.xor_(ret, ret);
    cc.ret(ret);
    cc.endFunc();
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect) {
    typedef int (*Func)(void);

    Func func = ptr_as_func<Func>(_func);

    int resultRet = func();
    int expectRet = 0;

    result.setFormat("ret={%d}", resultRet);
    expect.setFormat("ret={%d}", expectRet);

    return resultRet == expectRet;
  }
};

// ============================================================================
// [X86Test_JumpUnreachable1]
// ============================================================================

class X86Test_JumpUnreachable1 : public X86Test {
public:
  X86Test_JumpUnreachable1() : X86Test("[Jump] Unreachable #1") {}

  static void add(X86TestManager& mgr) {
    mgr.add(new X86Test_JumpUnreachable1());
  }

  virtual void compile(X86Compiler& cc) {
    cc.addFunc(FuncSignature0<void>(CallConv::kIdHost));

    Label L_1 = cc.newLabel();
    Label L_2 = cc.newLabel();
    Label L_3 = cc.newLabel();
    Label L_4 = cc.newLabel();
    Label L_5 = cc.newLabel();
    Label L_6 = cc.newLabel();
    Label L_7 = cc.newLabel();

    X86Gp v0 = cc.newUInt32("v0");
    X86Gp v1 = cc.newUInt32("v1");

    cc.bind(L_2);
    cc.bind(L_3);

    cc.jmp(L_1);

    cc.bind(L_5);
    cc.mov(v0, 0);

    cc.bind(L_6);
    cc.jmp(L_3);
    cc.mov(v1, 1);
    cc.jmp(L_1);

    cc.bind(L_4);
    cc.jmp(L_2);
    cc.bind(L_7);
    cc.add(v0, v1);

    cc.align(kAlignCode, 16);
    cc.bind(L_1);
    cc.ret();
    cc.endFunc();
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect) {
    typedef void (*Func)(void);
    Func func = ptr_as_func<Func>(_func);

    func();

    result.appendString("ret={}");
    expect.appendString("ret={}");

    return true;
  }
};

// ============================================================================
// [X86Test_JumpUnreachable2]
// ============================================================================

class X86Test_JumpUnreachable2 : public X86Test {
public:
  X86Test_JumpUnreachable2() : X86Test("[Jump] Unreachable #2") {}

  static void add(X86TestManager& mgr) {
    mgr.add(new X86Test_JumpUnreachable2());
  }

  virtual void compile(X86Compiler& cc) {
    cc.addFunc(FuncSignature0<void>(CallConv::kIdHost));

    Label L_1 = cc.newLabel();
    Label L_2 = cc.newLabel();

    X86Gp v0 = cc.newUInt32("v0");
    X86Gp v1 = cc.newUInt32("v1");

    cc.jmp(L_1);
    cc.bind(L_2);
    cc.mov(v0, 1);
    cc.mov(v1, 2);
    cc.cmp(v0, v1);
    cc.jz(L_2);
    cc.jmp(L_1);

    cc.bind(L_1);
    cc.ret();
    cc.endFunc();
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect) {
    typedef void (*Func)(void);
    Func func = ptr_as_func<Func>(_func);

    func();

    result.appendString("ret={}");
    expect.appendString("ret={}");

    return true;
  }
};

// ============================================================================
// [X86Test_AllocBase]
// ============================================================================

class X86Test_AllocBase : public X86Test {
public:
  X86Test_AllocBase() : X86Test("[Alloc] Base") {}

  static void add(X86TestManager& mgr) {
    mgr.add(new X86Test_AllocBase());
  }

  virtual void compile(X86Compiler& cc) {
    cc.addFunc(FuncSignature0<int>(CallConv::kIdHost));

    X86Gp v0 = cc.newInt32("v0");
    X86Gp v1 = cc.newInt32("v1");
    X86Gp v2 = cc.newInt32("v2");
    X86Gp v3 = cc.newInt32("v3");
    X86Gp v4 = cc.newInt32("v4");

    cc.xor_(v0, v0);

    cc.mov(v1, 1);
    cc.mov(v2, 2);
    cc.mov(v3, 3);
    cc.mov(v4, 4);

    cc.add(v0, v1);
    cc.add(v0, v2);
    cc.add(v0, v3);
    cc.add(v0, v4);

    cc.ret(v0);
    cc.endFunc();
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect) {
    typedef int (*Func)(void);
    Func func = ptr_as_func<Func>(_func);

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

class X86Test_AllocManual : public X86Test {
public:
  X86Test_AllocManual() : X86Test("[Alloc] Manual alloc/spill") {}

  static void add(X86TestManager& mgr) {
    mgr.add(new X86Test_AllocManual());
  }

  virtual void compile(X86Compiler& cc) {
    cc.addFunc(FuncSignature0<int>(CallConv::kIdHost));

    X86Gp v0  = cc.newInt32("v0");
    X86Gp v1  = cc.newInt32("v1");
    X86Gp cnt = cc.newInt32("cnt");

    cc.xor_(v0, v0);
    cc.xor_(v1, v1);
    cc.spill(v0);
    cc.spill(v1);

    Label L = cc.newLabel();
    cc.mov(cnt, 32);
    cc.bind(L);

    cc.inc(v1);
    cc.add(v0, v1);

    cc.dec(cnt);
    cc.jnz(L);

    cc.ret(v0);
    cc.endFunc();
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect) {
    typedef int (*Func)(void);
    Func func = ptr_as_func<Func>(_func);

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

class X86Test_AllocUseMem : public X86Test {
public:
  X86Test_AllocUseMem() : X86Test("[Alloc] Alloc/use mem") {}

  static void add(X86TestManager& mgr) {
    mgr.add(new X86Test_AllocUseMem());
  }

  virtual void compile(X86Compiler& cc) {
    cc.addFunc(FuncSignature2<int, int, int>(CallConv::kIdHost));

    X86Gp iIdx = cc.newInt32("iIdx");
    X86Gp iEnd = cc.newInt32("iEnd");

    X86Gp aIdx = cc.newInt32("aIdx");
    X86Gp aEnd = cc.newInt32("aEnd");

    Label L_1 = cc.newLabel();

    cc.setArg(0, aIdx);
    cc.setArg(1, aEnd);

    cc.mov(iIdx, aIdx);
    cc.mov(iEnd, aEnd);
    cc.spill(iEnd);

    cc.bind(L_1);
    cc.inc(iIdx);
    cc.cmp(iIdx, iEnd.m());
    cc.jne(L_1);

    cc.ret(iIdx);
    cc.endFunc();
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect) {
    typedef int (*Func)(int, int);
    Func func = ptr_as_func<Func>(_func);

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

class X86Test_AllocMany1 : public X86Test {
public:
  X86Test_AllocMany1() : X86Test("[Alloc] Many #1") {}

  enum { kCount = 8 };

  static void add(X86TestManager& mgr) {
    mgr.add(new X86Test_AllocMany1());
  }

  virtual void compile(X86Compiler& cc) {
    cc.addFunc(FuncSignature2<void, int*, int*>(CallConv::kIdHost));

    X86Gp a0 = cc.newIntPtr("a0");
    X86Gp a1 = cc.newIntPtr("a1");

    cc.setArg(0, a0);
    cc.setArg(1, a1);

    // Create some variables.
    X86Gp t = cc.newInt32("t");
    X86Gp x[kCount];

    uint32_t i;
    for (i = 0; i < kCount; i++) {
      x[i] = cc.newInt32("x%u", i);
    }

    // Setup variables (use mov with reg/imm to se if register allocator works).
    for (i = 0; i < kCount; i++) {
      cc.mov(x[i], static_cast<int>(i + 1));
    }

    // Make sum (addition).
    cc.xor_(t, t);
    for (i = 0; i < kCount; i++) {
      cc.add(t, x[i]);
    }

    // Store result to a given pointer in first argument.
    cc.mov(x86::dword_ptr(a0), t);

    // Clear t.
    cc.xor_(t, t);

    // Make sum (subtraction).
    for (i = 0; i < kCount; i++) {
      cc.sub(t, x[i]);
    }

    // Store result to a given pointer in second argument.
    cc.mov(x86::dword_ptr(a1), t);

    // End of function.
    cc.endFunc();
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect) {
    typedef void (*Func)(int*, int*);
    Func func = ptr_as_func<Func>(_func);

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

class X86Test_AllocMany2 : public X86Test {
public:
  X86Test_AllocMany2() : X86Test("[Alloc] Many #2") {}

  static void add(X86TestManager& mgr) {
    mgr.add(new X86Test_AllocMany2());
  }

  virtual void compile(X86Compiler& cc) {
    cc.addFunc(FuncSignature1<void, int*>(CallConv::kIdHost));

    X86Gp var[32];
    X86Gp a = cc.newIntPtr("a");

    cc.setArg(0, a);

    int i;
    for (i = 0; i < ASMJIT_ARRAY_SIZE(var); i++) {
      var[i] = cc.newInt32("var[%d]", i);
    }

    for (i = 0; i < ASMJIT_ARRAY_SIZE(var); i++) {
      cc.xor_(var[i], var[i]);
    }

    X86Gp v0 = cc.newInt32("v0");
    Label L = cc.newLabel();

    cc.mov(v0, 32);
    cc.bind(L);

    for (i = 0; i < ASMJIT_ARRAY_SIZE(var); i++) {
      cc.add(var[i], i);
    }

    cc.dec(v0);
    cc.jnz(L);

    for (i = 0; i < ASMJIT_ARRAY_SIZE(var); i++) {
      cc.mov(x86::dword_ptr(a, i * 4), var[i]);
    }

    cc.endFunc();
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect) {
    typedef void (*Func)(int*);
    Func func = ptr_as_func<Func>(_func);

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

class X86Test_AllocImul1 : public X86Test {
public:
  X86Test_AllocImul1() : X86Test("[Alloc] IMUL #1") {}

  static void add(X86TestManager& mgr) {
    mgr.add(new X86Test_AllocImul1());
  }

  virtual void compile(X86Compiler& cc) {
    cc.addFunc(FuncSignature4<void, int*, int*, int, int>(CallConv::kIdHost));

    X86Gp dstHi = cc.newIntPtr("dstHi");
    X86Gp dstLo = cc.newIntPtr("dstLo");

    X86Gp vHi = cc.newInt32("vHi");
    X86Gp vLo = cc.newInt32("vLo");
    X86Gp src = cc.newInt32("src");

    cc.setArg(0, dstHi);
    cc.setArg(1, dstLo);
    cc.setArg(2, vLo);
    cc.setArg(3, src);

    cc.imul(vHi, vLo, src);

    cc.mov(x86::dword_ptr(dstHi), vHi);
    cc.mov(x86::dword_ptr(dstLo), vLo);
    cc.endFunc();
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect) {
    typedef void (*Func)(int*, int*, int, int);
    Func func = ptr_as_func<Func>(_func);

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

class X86Test_AllocImul2 : public X86Test {
public:
  X86Test_AllocImul2() : X86Test("[Alloc] IMUL #2") {}

  static void add(X86TestManager& mgr) {
    mgr.add(new X86Test_AllocImul2());
  }

  virtual void compile(X86Compiler& cc) {
    cc.addFunc(FuncSignature2<void, int*, const int*>(CallConv::kIdHost));

    X86Gp dst = cc.newIntPtr("dst");
    X86Gp src = cc.newIntPtr("src");

    cc.setArg(0, dst);
    cc.setArg(1, src);

    for (unsigned int i = 0; i < 4; i++) {
      X86Gp x  = cc.newInt32("x");
      X86Gp y  = cc.newInt32("y");
      X86Gp hi = cc.newInt32("hi");

      cc.mov(x, x86::dword_ptr(src, 0));
      cc.mov(y, x86::dword_ptr(src, 4));

      cc.imul(hi, x, y);
      cc.add(x86::dword_ptr(dst, 0), hi);
      cc.add(x86::dword_ptr(dst, 4), x);
    }

    cc.endFunc();
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect) {
    typedef void (*Func)(int*, const int*);
    Func func = ptr_as_func<Func>(_func);

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
// [X86Test_AllocIdiv1]
// ============================================================================

class X86Test_AllocIdiv1 : public X86Test {
public:
  X86Test_AllocIdiv1() : X86Test("[Alloc] IDIV #1") {}

  static void add(X86TestManager& mgr) {
    mgr.add(new X86Test_AllocIdiv1());
  }

  virtual void compile(X86Compiler& cc) {
    cc.addFunc(FuncSignature2<int, int, int>(CallConv::kIdHost));

    X86Gp a = cc.newInt32("a");
    X86Gp b = cc.newInt32("b");
    X86Gp dummy = cc.newInt32("dummy");

    cc.setArg(0, a);
    cc.setArg(1, b);

    cc.xor_(dummy, dummy);
    cc.idiv(dummy, a, b);

    cc.ret(a);
    cc.endFunc();
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect) {
    typedef int (*Func)(int, int);
    Func func = ptr_as_func<Func>(_func);

    int v0 = 2999;
    int v1 = 245;

    int resultRet = func(v0, v1);
    int expectRet = 2999 / 245;

    result.setFormat("result=%d", resultRet);
    expect.setFormat("result=%d", expectRet);

    return resultRet == expectRet;
  }
};

// ============================================================================
// [X86Test_AllocSetz]
// ============================================================================

class X86Test_AllocSetz : public X86Test {
public:
  X86Test_AllocSetz() : X86Test("[Alloc] SETZ") {}

  static void add(X86TestManager& mgr) {
    mgr.add(new X86Test_AllocSetz());
  }

  virtual void compile(X86Compiler& cc) {
    cc.addFunc(FuncSignature3<void, int, int, char*>(CallConv::kIdHost));

    X86Gp src0 = cc.newInt32("src0");
    X86Gp src1 = cc.newInt32("src1");
    X86Gp dst0 = cc.newIntPtr("dst0");

    cc.setArg(0, src0);
    cc.setArg(1, src1);
    cc.setArg(2, dst0);

    cc.cmp(src0, src1);
    cc.setz(x86::byte_ptr(dst0));

    cc.endFunc();
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect) {
    typedef void (*Func)(int, int, char*);
    Func func = ptr_as_func<Func>(_func);

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

class X86Test_AllocShlRor : public X86Test {
public:
  X86Test_AllocShlRor() : X86Test("[Alloc] SHL/ROR") {}

  static void add(X86TestManager& mgr) {
    mgr.add(new X86Test_AllocShlRor());
  }

  virtual void compile(X86Compiler& cc) {
    cc.addFunc(FuncSignature4<void, int*, int, int, int>(CallConv::kIdHost));

    X86Gp dst = cc.newIntPtr("dst");
    X86Gp var = cc.newInt32("var");
    X86Gp vShlParam = cc.newInt32("vShlParam");
    X86Gp vRorParam = cc.newInt32("vRorParam");

    cc.setArg(0, dst);
    cc.setArg(1, var);
    cc.setArg(2, vShlParam);
    cc.setArg(3, vRorParam);

    cc.shl(var, vShlParam);
    cc.ror(var, vRorParam);

    cc.mov(x86::dword_ptr(dst), var);
    cc.endFunc();
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect) {
    typedef void (*Func)(int*, int, int, int);
    Func func = ptr_as_func<Func>(_func);

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

class X86Test_AllocGpLo : public X86Test {
public:
  X86Test_AllocGpLo() : X86Test("[Alloc] GPB-LO") {}

  enum { kCount = 32 };

  static void add(X86TestManager& mgr) {
    mgr.add(new X86Test_AllocGpLo());
  }

  virtual void compile(X86Compiler& cc) {
    cc.addFunc(FuncSignature1<uint32_t, uint32_t*>(CallConv::kIdHost));

    X86Gp rPtr = cc.newUIntPtr("rPtr");
    X86Gp rSum = cc.newUInt32("rSum");

    cc.setArg(0, rPtr);

    X86Gp rVar[kCount];
    uint32_t i;

    for (i = 0; i < kCount; i++) {
      rVar[i] = cc.newUInt32("rVar[%u]", i);
    }

    // Init pseudo-regs with values from our array.
    for (i = 0; i < kCount; i++) {
      cc.mov(rVar[i], x86::dword_ptr(rPtr, i * 4));
    }

    for (i = 2; i < kCount; i++) {
      // Add and truncate to 8 bit; no purpose, just mess with jit.
      cc.add  (rVar[i  ], rVar[i-1]);
      cc.movzx(rVar[i  ], rVar[i  ].r8());
      cc.movzx(rVar[i-2], rVar[i-1].r8());
      cc.movzx(rVar[i-1], rVar[i-2].r8());
    }

    // Sum up all computed values.
    cc.mov(rSum, 0);
    for (i = 0; i < kCount; i++) {
      cc.add(rSum, rVar[i]);
    }

    // Return the sum.
    cc.ret(rSum);
    cc.endFunc();
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect) {
    typedef int (*Func)(uint32_t*);
    Func func = ptr_as_func<Func>(_func);

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

class X86Test_AllocRepMovsb : public X86Test {
public:
  X86Test_AllocRepMovsb() : X86Test("[Alloc] REP MOVS") {}

  static void add(X86TestManager& mgr) {
    mgr.add(new X86Test_AllocRepMovsb());
  }

  virtual void compile(X86Compiler& cc) {
    cc.addFunc(FuncSignature3<void, void*, void*, size_t>(CallConv::kIdHost));

    X86Gp dst = cc.newIntPtr("dst");
    X86Gp src = cc.newIntPtr("src");
    X86Gp cnt = cc.newIntPtr("cnt");

    cc.setArg(0, dst);
    cc.setArg(1, src);
    cc.setArg(2, cnt);

    cc.rep(cnt).movs(x86::byte_ptr(dst), x86::byte_ptr(src));
    cc.endFunc();
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect) {
    typedef void (*Func)(void*, void*, size_t);
    Func func = ptr_as_func<Func>(_func);

    char dst[20] = { 0 };
    char src[20] = "Hello AsmJit!";
    func(dst, src, strlen(src) + 1);

    result.setFormat("ret=\"%s\"", dst);
    expect.setFormat("ret=\"%s\"", src);

    return result == expect;
  }
};

// ============================================================================
// [X86Test_AllocIfElse1]
// ============================================================================

class X86Test_AllocIfElse1 : public X86Test {
public:
  X86Test_AllocIfElse1() : X86Test("[Alloc] If-Else #1") {}

  static void add(X86TestManager& mgr) {
    mgr.add(new X86Test_AllocIfElse1());
  }

  virtual void compile(X86Compiler& cc) {
    cc.addFunc(FuncSignature2<int, int, int>(CallConv::kIdHost));

    X86Gp v1 = cc.newInt32("v1");
    X86Gp v2 = cc.newInt32("v2");

    Label L_1 = cc.newLabel();
    Label L_2 = cc.newLabel();

    cc.setArg(0, v1);
    cc.setArg(1, v2);

    cc.cmp(v1, v2);
    cc.jg(L_1);

    cc.mov(v1, 1);
    cc.jmp(L_2);

    cc.bind(L_1);
    cc.mov(v1, 2);

    cc.bind(L_2);
    cc.ret(v1);
    cc.endFunc();
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect)  {
    typedef int (*Func)(int, int);
    Func func = ptr_as_func<Func>(_func);

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

class X86Test_AllocIfElse2 : public X86Test {
public:
  X86Test_AllocIfElse2() : X86Test("[Alloc] If-Else #2") {}

  static void add(X86TestManager& mgr) {
    mgr.add(new X86Test_AllocIfElse2());
  }

  virtual void compile(X86Compiler& cc) {
    cc.addFunc(FuncSignature2<int, int, int>(CallConv::kIdHost));

    X86Gp v1 = cc.newInt32("v1");
    X86Gp v2 = cc.newInt32("v2");

    Label L_1 = cc.newLabel();
    Label L_2 = cc.newLabel();
    Label L_3 = cc.newLabel();
    Label L_4 = cc.newLabel();

    cc.setArg(0, v1);
    cc.setArg(1, v2);

    cc.jmp(L_1);
    cc.bind(L_2);
    cc.jmp(L_4);
    cc.bind(L_1);

    cc.cmp(v1, v2);
    cc.jg(L_3);

    cc.mov(v1, 1);
    cc.jmp(L_2);

    cc.bind(L_3);
    cc.mov(v1, 2);
    cc.jmp(L_2);

    cc.bind(L_4);

    cc.ret(v1);
    cc.endFunc();
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect)  {
    typedef int (*Func)(int, int);
    Func func = ptr_as_func<Func>(_func);

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

class X86Test_AllocIfElse3 : public X86Test {
public:
  X86Test_AllocIfElse3() : X86Test("[Alloc] If-Else #3") {}

  static void add(X86TestManager& mgr) {
    mgr.add(new X86Test_AllocIfElse3());
  }

  virtual void compile(X86Compiler& cc) {
    cc.addFunc(FuncSignature2<int, int, int>(CallConv::kIdHost));

    X86Gp v1 = cc.newInt32("v1");
    X86Gp v2 = cc.newInt32("v2");
    X86Gp counter = cc.newInt32("counter");

    Label L_1 = cc.newLabel();
    Label L_Loop = cc.newLabel();
    Label L_Exit = cc.newLabel();

    cc.setArg(0, v1);
    cc.setArg(1, v2);

    cc.cmp(v1, v2);
    cc.jg(L_1);

    cc.mov(counter, 0);

    cc.bind(L_Loop);
    cc.mov(v1, counter);

    cc.inc(counter);
    cc.cmp(counter, 1);
    cc.jle(L_Loop);
    cc.jmp(L_Exit);

    cc.bind(L_1);
    cc.mov(v1, 2);

    cc.bind(L_Exit);
    cc.ret(v1);
    cc.endFunc();
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect)  {
    typedef int (*Func)(int, int);
    Func func = ptr_as_func<Func>(_func);

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

class X86Test_AllocIfElse4 : public X86Test {
public:
  X86Test_AllocIfElse4() : X86Test("[Alloc] If-Else #4") {}

  static void add(X86TestManager& mgr) {
    mgr.add(new X86Test_AllocIfElse4());
  }

  virtual void compile(X86Compiler& cc) {
    cc.addFunc(FuncSignature2<int, int, int>(CallConv::kIdHost));

    X86Gp v1 = cc.newInt32("v1");
    X86Gp v2 = cc.newInt32("v2");
    X86Gp counter = cc.newInt32("counter");

    Label L_1 = cc.newLabel();
    Label L_Loop1 = cc.newLabel();
    Label L_Loop2 = cc.newLabel();
    Label L_Exit = cc.newLabel();

    cc.mov(counter, 0);

    cc.setArg(0, v1);
    cc.setArg(1, v2);

    cc.cmp(v1, v2);
    cc.jg(L_1);

    cc.bind(L_Loop1);
    cc.mov(v1, counter);

    cc.inc(counter);
    cc.cmp(counter, 1);
    cc.jle(L_Loop1);
    cc.jmp(L_Exit);

    cc.bind(L_1);
    cc.bind(L_Loop2);
    cc.mov(v1, counter);
    cc.inc(counter);
    cc.cmp(counter, 2);
    cc.jle(L_Loop2);

    cc.bind(L_Exit);
    cc.ret(v1);
    cc.endFunc();
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect)  {
    typedef int (*Func)(int, int);
    Func func = ptr_as_func<Func>(_func);

    int a = func(0, 1);
    int b = func(1, 0);

    result.appendFormat("ret={%d, %d}", a, b);
    result.appendFormat("ret={%d, %d}", 1, 2);

    return a == 1 && b == 2;
  }
};

// ============================================================================
// [X86Test_AllocInt8]
// ============================================================================

class X86Test_AllocInt8 : public X86Test {
public:
  X86Test_AllocInt8() : X86Test("[Alloc] Int8") {}

  static void add(X86TestManager& mgr) {
    mgr.add(new X86Test_AllocInt8());
  }

  virtual void compile(X86Compiler& cc) {
    X86Gp x = cc.newInt8("x");
    X86Gp y = cc.newInt32("y");

    cc.addFunc(FuncSignature1<int, char>(CallConv::kIdHost));
    cc.setArg(0, x);

    cc.movsx(y, x);

    cc.ret(y);
    cc.endFunc();
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect) {
    typedef int (*Func)(char);
    Func func = ptr_as_func<Func>(_func);

    int resultRet = func(-13);
    int expectRet = -13;

    result.setFormat("ret=%d", resultRet);
    expect.setFormat("ret=%d", expectRet);

    return resultRet == expectRet;
  }
};

// ============================================================================
// [X86Test_AllocArgsIntPtr]
// ============================================================================

class X86Test_AllocArgsIntPtr : public X86Test {
public:
  X86Test_AllocArgsIntPtr() : X86Test("[Alloc] Args IntPtr") {}

  static void add(X86TestManager& mgr) {
    mgr.add(new X86Test_AllocArgsIntPtr());
  }

  virtual void compile(X86Compiler& cc) {
    cc.addFunc(FuncSignature8<void, void*, void*, void*, void*, void*, void*, void*, void*>(CallConv::kIdHost));

    uint32_t i;
    X86Gp var[8];

    for (i = 0; i < 8; i++) {
      var[i] = cc.newIntPtr("var%u", i);
      cc.setArg(i, var[i]);
    }

    for (i = 0; i < 8; i++) {
      cc.add(var[i], static_cast<int>(i + 1));
    }

    // Move some data into buffer provided by arguments so we can verify if it
    // really works without looking into assembler output.
    for (i = 0; i < 8; i++) {
      cc.add(x86::byte_ptr(var[i]), static_cast<int>(i + 1));
    }

    cc.endFunc();
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect) {
    typedef void (*Func)(void*, void*, void*, void*, void*, void*, void*, void*);
    Func func = ptr_as_func<Func>(_func);

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

    return result == expect;
  }
};

// ============================================================================
// [X86Test_AllocArgsFloat]
// ============================================================================

class X86Test_AllocArgsFloat : public X86Test {
public:
  X86Test_AllocArgsFloat() : X86Test("[Alloc] Args Float") {}

  static void add(X86TestManager& mgr) {
    mgr.add(new X86Test_AllocArgsFloat());
  }

  virtual void compile(X86Compiler& cc) {
    cc.addFunc(FuncSignature8<void, float, float, float, float, float, float, float, void*>(CallConv::kIdHost));

    uint32_t i;

    X86Gp p = cc.newIntPtr("p");
    X86Xmm xv[7];

    for (i = 0; i < 7; i++) {
      xv[i] = cc.newXmmSs("xv%u", i);
      cc.setArg(i, xv[i]);
    }

    cc.setArg(7, p);

    cc.addss(xv[0], xv[1]);
    cc.addss(xv[0], xv[2]);
    cc.addss(xv[0], xv[3]);
    cc.addss(xv[0], xv[4]);
    cc.addss(xv[0], xv[5]);
    cc.addss(xv[0], xv[6]);

    cc.movss(x86::ptr(p), xv[0]);
    cc.endFunc();
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect) {
    typedef void (*Func)(float, float, float, float, float, float, float, float*);
    Func func = ptr_as_func<Func>(_func);

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

class X86Test_AllocArgsDouble : public X86Test {
public:
  X86Test_AllocArgsDouble() : X86Test("[Alloc] Args Double") {}

  static void add(X86TestManager& mgr) {
    mgr.add(new X86Test_AllocArgsDouble());
  }

  virtual void compile(X86Compiler& cc) {
    cc.addFunc(FuncSignature8<void, double, double, double, double, double, double, double, void*>(CallConv::kIdHost));

    uint32_t i;

    X86Gp p = cc.newIntPtr("p");
    X86Xmm xv[7];

    for (i = 0; i < 7; i++) {
      xv[i] = cc.newXmmSd("xv%u", i);
      cc.setArg(i, xv[i]);
    }

    cc.setArg(7, p);

    cc.addsd(xv[0], xv[1]);
    cc.addsd(xv[0], xv[2]);
    cc.addsd(xv[0], xv[3]);
    cc.addsd(xv[0], xv[4]);
    cc.addsd(xv[0], xv[5]);
    cc.addsd(xv[0], xv[6]);

    cc.movsd(x86::ptr(p), xv[0]);
    cc.endFunc();
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect) {
    typedef void (*Func)(double, double, double, double, double, double, double, double*);
    Func func = ptr_as_func<Func>(_func);

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

class X86Test_AllocRetFloat : public X86Test {
public:
  X86Test_AllocRetFloat() : X86Test("[Alloc] Ret Float") {}

  static void add(X86TestManager& mgr) {
    mgr.add(new X86Test_AllocRetFloat());
  }

  virtual void compile(X86Compiler& cc) {
    cc.addFunc(FuncSignature2<float, float, float>(CallConv::kIdHost));

    X86Xmm a = cc.newXmmSs("a");
    X86Xmm b = cc.newXmmSs("b");

    cc.setArg(0, a);
    cc.setArg(1, b);

    cc.addss(a, b);
    cc.ret(a);

    cc.endFunc();
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect) {
    typedef float (*Func)(float, float);
    Func func = ptr_as_func<Func>(_func);

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

class X86Test_AllocRetDouble : public X86Test {
public:
  X86Test_AllocRetDouble() : X86Test("[Alloc] Ret Double") {}

  static void add(X86TestManager& mgr) {
    mgr.add(new X86Test_AllocRetDouble());
  }

  virtual void compile(X86Compiler& cc) {
    cc.addFunc(FuncSignature2<double, double, double>(CallConv::kIdHost));

    X86Xmm a = cc.newXmmSd("a");
    X86Xmm b = cc.newXmmSd("b");

    cc.setArg(0, a);
    cc.setArg(1, b);

    cc.addsd(a, b);
    cc.ret(a);

    cc.endFunc();
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect) {
    typedef double (*Func)(double, double);
    Func func = ptr_as_func<Func>(_func);

    double resultRet = func(1.0, 2.0);
    double expectRet = 1.0 + 2.0;

    result.setFormat("ret={%g}", resultRet);
    expect.setFormat("ret={%g}", expectRet);

    return resultRet == expectRet;
  }
};

// ============================================================================
// [X86Test_AllocStack1]
// ============================================================================

class X86Test_AllocStack1 : public X86Test {
public:
  X86Test_AllocStack1() : X86Test("[Alloc] Stack #1") {}

  enum { kSize = 256 };

  static void add(X86TestManager& mgr) {
    mgr.add(new X86Test_AllocStack1());
  }

  virtual void compile(X86Compiler& cc) {
    cc.addFunc(FuncSignature0<int>(CallConv::kIdHost));

    X86Mem stack = cc.newStack(kSize, 1);
    stack.setSize(1);

    X86Gp i = cc.newIntPtr("i");
    X86Gp a = cc.newInt32("a");
    X86Gp b = cc.newInt32("b");

    Label L_1 = cc.newLabel();
    Label L_2 = cc.newLabel();

    // Fill stack by sequence [0, 1, 2, 3 ... 255].
    cc.xor_(i, i);

    X86Mem stackWithIndex = stack.clone();
    stackWithIndex.setIndex(i, 0);

    cc.bind(L_1);
    cc.mov(stackWithIndex, i.r8());
    cc.inc(i);
    cc.cmp(i, 255);
    cc.jle(L_1);

    // Sum sequence in stack.
    cc.xor_(i, i);
    cc.xor_(a, a);

    cc.bind(L_2);
    cc.movzx(b, stackWithIndex);
    cc.add(a, b);
    cc.inc(i);
    cc.cmp(i, 255);
    cc.jle(L_2);

    cc.ret(a);
    cc.endFunc();
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect) {
    typedef int (*Func)(void);
    Func func = ptr_as_func<Func>(_func);

    int resultRet = func();
    int expectRet = 32640;

    result.setInt(resultRet);
    expect.setInt(expectRet);

    return resultRet == expectRet;
  }
};

// ============================================================================
// [X86Test_AllocStack2]
// ============================================================================

class X86Test_AllocStack2 : public X86Test {
public:
  X86Test_AllocStack2() : X86Test("[Alloc] Stack #2") {}

  enum { kSize = 256 };

  static void add(X86TestManager& mgr) {
    mgr.add(new X86Test_AllocStack2());
  }

  virtual void compile(X86Compiler& cc) {
    cc.addFunc(FuncSignature0<int>(CallConv::kIdHost));

    const int kTokenSize = 32;

    X86Mem s1 = cc.newStack(kTokenSize, 32);
    X86Mem s2 = cc.newStack(kTokenSize, 32);

    X86Gp p1 = cc.newIntPtr("p1");
    X86Gp p2 = cc.newIntPtr("p2");

    X86Gp ret = cc.newInt32("ret");
    Label L_Exit = cc.newLabel();

    static const char token[kTokenSize] = "-+:|abcdefghijklmnopqrstuvwxyz|";
    CCFuncCall* call;

    cc.lea(p1, s1);
    cc.lea(p2, s2);

    // Try to corrupt the stack if wrongly allocated.
    call = cc.call(imm_ptr((void*)memcpy), FuncSignature3<void*, void*, void*, size_t>(CallConv::kIdHostCDecl));
    call->setArg(0, p1);
    call->setArg(1, imm_ptr(token));
    call->setArg(2, imm(kTokenSize));
    call->setRet(0, p1);

    call = cc.call(imm_ptr((void*)memcpy), FuncSignature3<void*, void*, void*, size_t>(CallConv::kIdHostCDecl));
    call->setArg(0, p2);
    call->setArg(1, imm_ptr(token));
    call->setArg(2, imm(kTokenSize));
    call->setRet(0, p2);

    call = cc.call(imm_ptr((void*)memcmp), FuncSignature3<int, void*, void*, size_t>(CallConv::kIdHostCDecl));
    call->setArg(0, p1);
    call->setArg(1, p2);
    call->setArg(2, imm(kTokenSize));
    call->setRet(0, ret);

    // This should be 0 on success, however, if both `p1` and `p2` were
    // allocated in the same address this check will still pass.
    cc.cmp(ret, 0);
    cc.jnz(L_Exit);

    // Checks whether `p1` and `p2` are different (must be).
    cc.xor_(ret, ret);
    cc.cmp(p1, p2);
    cc.setz(ret.r8());

    cc.bind(L_Exit);
    cc.ret(ret);
    cc.endFunc();
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect) {
    typedef int (*Func)(void);
    Func func = ptr_as_func<Func>(_func);

    int resultRet = func();
    int expectRet = 0; // Must be zero, stack addresses must be different.

    result.setInt(resultRet);
    expect.setInt(expectRet);

    return resultRet == expectRet;
  }
};

// ============================================================================
// [X86Test_AllocMemcpy]
// ============================================================================

class X86Test_AllocMemcpy : public X86Test {
public:
  X86Test_AllocMemcpy() : X86Test("[Alloc] Memcpy") {}

  enum { kCount = 32 };

  static void add(X86TestManager& mgr) {
    mgr.add(new X86Test_AllocMemcpy());
  }

  virtual void compile(X86Compiler& cc) {
    X86Gp dst = cc.newIntPtr("dst");
    X86Gp src = cc.newIntPtr("src");
    X86Gp cnt = cc.newUIntPtr("cnt");

    Label L_Loop = cc.newLabel();                   // Create base labels we use
    Label L_Exit = cc.newLabel();                   // in our function.

    cc.addFunc(FuncSignature3<void, uint32_t*, const uint32_t*, size_t>(CallConv::kIdHost));
    cc.setArg(0, dst);
    cc.setArg(1, src);
    cc.setArg(2, cnt);

    cc.alloc(dst);                                  // Allocate all registers now,
    cc.alloc(src);                                  // because we want to keep them
    cc.alloc(cnt);                                  // in physical registers only.

    cc.test(cnt, cnt);                              // Exit if length is zero.
    cc.jz(L_Exit);

    cc.bind(L_Loop);                                // Bind the loop label here.

    X86Gp tmp = cc.newInt32("tmp");              // Copy a single dword (4 bytes).
    cc.mov(tmp, x86::dword_ptr(src));
    cc.mov(x86::dword_ptr(dst), tmp);

    cc.add(src, 4);                                 // Increment dst/src pointers.
    cc.add(dst, 4);

    cc.dec(cnt);                                    // Loop until cnt isn't zero.
    cc.jnz(L_Loop);

    cc.bind(L_Exit);                                // Bind the exit label here.
    cc.endFunc();                                   // End of function.
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect) {
    typedef void (*Func)(uint32_t*, const uint32_t*, size_t);
    Func func = ptr_as_func<Func>(_func);

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

    return result == expect;
  }
};

// ============================================================================
// [X86Test_AllocAlphaBlend]
// ============================================================================

class X86Test_AllocAlphaBlend : public X86Test {
public:
  X86Test_AllocAlphaBlend() : X86Test("[Alloc] AlphaBlend") {}

  enum { kCount = 17 };

  static void add(X86TestManager& mgr) {
    mgr.add(new X86Test_AllocAlphaBlend());
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

  virtual void compile(X86Compiler& cc) {
    asmtest::generateAlphaBlend(cc);
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect) {
    typedef void (*Func)(void*, const void*, size_t);
    Func func = ptr_as_func<Func>(_func);

    static const uint32_t dstConstData[] = { 0x00000000, 0x10101010, 0x20100804, 0x30200003, 0x40204040, 0x5000004D, 0x60302E2C, 0x706F6E6D, 0x807F4F2F, 0x90349001, 0xA0010203, 0xB03204AB, 0xC023AFBD, 0xD0D0D0C0, 0xE0AABBCC, 0xFFFFFFFF, 0xF8F4F2F1 };
    static const uint32_t srcConstData[] = { 0xE0E0E0E0, 0xA0008080, 0x341F1E1A, 0xFEFEFEFE, 0x80302010, 0x49490A0B, 0x998F7798, 0x00000000, 0x01010101, 0xA0264733, 0xBAB0B1B9, 0xFF000000, 0xDAB0A0C1, 0xE0BACFDA, 0x99887766, 0xFFFFFF80, 0xEE0A5FEC };

    uint32_t _dstBuffer[kCount + 3];
    uint32_t _srcBuffer[kCount + 3];

    // Has to be aligned.
    uint32_t* dstBuffer = (uint32_t*)Utils::alignTo<intptr_t>((intptr_t)_dstBuffer, 16);
    uint32_t* srcBuffer = (uint32_t*)Utils::alignTo<intptr_t>((intptr_t)_srcBuffer, 16);

    ::memcpy(dstBuffer, dstConstData, sizeof(dstConstData));
    ::memcpy(srcBuffer, srcConstData, sizeof(srcConstData));

    uint32_t i;
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

      result.appendFormat("%08X", static_cast<unsigned int>(dstBuffer[i]));
      expect.appendFormat("%08X", static_cast<unsigned int>(expBuffer[i]));
    }

    result.appendString("}");
    expect.appendString("}");

    return result == expect;
  }
};

// ============================================================================
// [X86Test_CallBase]
// ============================================================================

class X86Test_CallBase : public X86Test {
public:
  X86Test_CallBase() : X86Test("[Call] CDecl") {}

  static void add(X86TestManager& mgr) {
    mgr.add(new X86Test_CallBase());
  }

  virtual void compile(X86Compiler& cc) {
    X86Gp v0 = cc.newInt32("v0");
    X86Gp v1 = cc.newInt32("v1");
    X86Gp v2 = cc.newInt32("v2");

    cc.addFunc(FuncSignature3<int, int, int, int>(CallConv::kIdHost));
    cc.setArg(0, v0);
    cc.setArg(1, v1);
    cc.setArg(2, v2);

    // Just do something.
    cc.shl(v0, 1);
    cc.shl(v1, 1);
    cc.shl(v2, 1);

    // Call a function.
    X86Gp fn = cc.newIntPtr("fn");
    cc.mov(fn, imm_ptr(calledFunc));

    CCFuncCall* call = cc.call(fn, FuncSignature3<int, int, int, int>(CallConv::kIdHost));
    call->setArg(0, v2);
    call->setArg(1, v1);
    call->setArg(2, v0);
    call->setRet(0, v0);

    cc.ret(v0);
    cc.endFunc();
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect) {
    typedef int (*Func)(int, int, int);
    Func func = ptr_as_func<Func>(_func);

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

class X86Test_CallFast : public X86Test {
public:
  X86Test_CallFast() : X86Test("[Call] Fastcall") {}

  static void add(X86TestManager& mgr) {
    mgr.add(new X86Test_CallFast());
  }

  virtual void compile(X86Compiler& cc) {
    X86Gp var = cc.newInt32("var");
    X86Gp fn = cc.newIntPtr("fn");

    cc.addFunc(FuncSignature1<int, int>(CallConv::kIdHost));
    cc.setArg(0, var);

    cc.mov(fn, imm_ptr(calledFunc));
    CCFuncCall* call;

    call = cc.call(fn, FuncSignature1<int, int>(CallConv::kIdHostFastCall));
    call->setArg(0, var);
    call->setRet(0, var);

    call = cc.call(fn, FuncSignature1<int, int>(CallConv::kIdHostFastCall));
    call->setArg(0, var);
    call->setRet(0, var);

    cc.ret(var);
    cc.endFunc();
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect) {
    typedef int (*Func)(int);
    Func func = ptr_as_func<Func>(_func);

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

class X86Test_CallManyArgs : public X86Test {
public:
  X86Test_CallManyArgs() : X86Test("[Call] Many Args") {}

  static void add(X86TestManager& mgr) {
    mgr.add(new X86Test_CallManyArgs());
  }

  static int calledFunc(int a, int b, int c, int d, int e, int f, int g, int h, int i, int j) {
    return (a * b * c * d * e) + (f * g * h * i * j);
  }

  virtual void compile(X86Compiler& cc) {
    cc.addFunc(FuncSignature0<int>(CallConv::kIdHost));

    // Prepare.
    X86Gp fn = cc.newIntPtr("fn");
    X86Gp va = cc.newInt32("va");
    X86Gp vb = cc.newInt32("vb");
    X86Gp vc = cc.newInt32("vc");
    X86Gp vd = cc.newInt32("vd");
    X86Gp ve = cc.newInt32("ve");
    X86Gp vf = cc.newInt32("vf");
    X86Gp vg = cc.newInt32("vg");
    X86Gp vh = cc.newInt32("vh");
    X86Gp vi = cc.newInt32("vi");
    X86Gp vj = cc.newInt32("vj");

    cc.mov(fn, imm_ptr(calledFunc));
    cc.mov(va, 0x03);
    cc.mov(vb, 0x12);
    cc.mov(vc, 0xA0);
    cc.mov(vd, 0x0B);
    cc.mov(ve, 0x2F);
    cc.mov(vf, 0x02);
    cc.mov(vg, 0x0C);
    cc.mov(vh, 0x12);
    cc.mov(vi, 0x18);
    cc.mov(vj, 0x1E);

    // Call function.
    CCFuncCall* call = cc.call(fn, FuncSignature10<int, int, int, int, int, int, int, int, int, int, int>(CallConv::kIdHost));
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

    cc.ret(va);
    cc.endFunc();
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect) {
    typedef int (*Func)(void);
    Func func = ptr_as_func<Func>(_func);

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

class X86Test_CallDuplicateArgs : public X86Test {
public:
  X86Test_CallDuplicateArgs() : X86Test("[Call] Duplicate Args") {}

  static void add(X86TestManager& mgr) {
    mgr.add(new X86Test_CallDuplicateArgs());
  }

  static int calledFunc(int a, int b, int c, int d, int e, int f, int g, int h, int i, int j) {
    return (a * b * c * d * e) + (f * g * h * i * j);
  }

  virtual void compile(X86Compiler& cc) {
    cc.addFunc(FuncSignature0<int>(CallConv::kIdHost));

    // Prepare.
    X86Gp fn = cc.newIntPtr("fn");
    X86Gp a = cc.newInt32("a");

    cc.mov(fn, imm_ptr(calledFunc));
    cc.mov(a, 3);

    // Call function.
    CCFuncCall* call = cc.call(fn, FuncSignature10<int, int, int, int, int, int, int, int, int, int, int>(CallConv::kIdHost));
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

    cc.ret(a);
    cc.endFunc();
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect) {
    typedef int (*Func)(void);
    Func func = ptr_as_func<Func>(_func);

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

class X86Test_CallImmArgs : public X86Test {
public:
  X86Test_CallImmArgs() : X86Test("[Call] Imm Args") {}

  static void add(X86TestManager& mgr) {
    mgr.add(new X86Test_CallImmArgs());
  }

  virtual void compile(X86Compiler& cc) {
    cc.addFunc(FuncSignature0<int>(CallConv::kIdHost));

    // Prepare.
    X86Gp fn = cc.newIntPtr("fn");
    X86Gp rv = cc.newInt32("rv");

    cc.mov(fn, imm_ptr(X86Test_CallManyArgs::calledFunc));

    // Call function.
    CCFuncCall* call = cc.call(fn, FuncSignature10<int, int, int, int, int, int, int, int, int, int, int>(CallConv::kIdHost));
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

    cc.ret(rv);
    cc.endFunc();
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect) {
    typedef int (*Func)(void);
    Func func = ptr_as_func<Func>(_func);

    int resultRet = func();
    int expectRet = X86Test_CallManyArgs::calledFunc(0x03, 0x12, 0xA0, 0x0B, 0x2F, 0x02, 0x0C, 0x12, 0x18, 0x1E);

    result.setFormat("ret=%d", resultRet);
    expect.setFormat("ret=%d", expectRet);

    return resultRet == expectRet;
  }
};

// ============================================================================
// [X86Test_CallPtrArgs]
// ============================================================================

class X86Test_CallPtrArgs : public X86Test {
public:
  X86Test_CallPtrArgs() : X86Test("[Call] Ptr Args") {}

  static void add(X86TestManager& mgr) {
    mgr.add(new X86Test_CallPtrArgs());
  }

  static int calledFunc(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j) {
    return static_cast<int>((intptr_t)a) +
           static_cast<int>((intptr_t)b) +
           static_cast<int>((intptr_t)c) +
           static_cast<int>((intptr_t)d) +
           static_cast<int>((intptr_t)e) +
           static_cast<int>((intptr_t)f) +
           static_cast<int>((intptr_t)g) +
           static_cast<int>((intptr_t)h) +
           static_cast<int>((intptr_t)i) +
           static_cast<int>((intptr_t)j) ;
  }

  virtual void compile(X86Compiler& cc) {
    cc.addFunc(FuncSignature0<int>(CallConv::kIdHost));

    // Prepare.
    X86Gp fn = cc.newIntPtr("fn");
    X86Gp rv = cc.newInt32("rv");

    cc.mov(fn, imm_ptr(calledFunc));

    // Call function.
    CCFuncCall* call = cc.call(fn, FuncSignature10<int, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*>(CallConv::kIdHost));
    call->setArg(0, imm(0x01));
    call->setArg(1, imm(0x02));
    call->setArg(2, imm(0x03));
    call->setArg(3, imm(0x04));
    call->setArg(4, imm(0x05));
    call->setArg(5, imm(0x06));
    call->setArg(6, imm(0x07));
    call->setArg(7, imm(0x08));
    call->setArg(8, imm(0x09));
    call->setArg(9, imm(0x0A));
    call->setRet(0, rv);

    cc.ret(rv);
    cc.endFunc();
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect) {
    typedef int (*Func)(void);
    Func func = ptr_as_func<Func>(_func);

    int resultRet = func();
    int expectRet = 55;

    result.setFormat("ret=%d", resultRet);
    expect.setFormat("ret=%d", expectRet);

    return resultRet == expectRet;
  }
};

// ============================================================================
// [X86Test_CallFloatAsXmmRet]
// ============================================================================

class X86Test_CallFloatAsXmmRet : public X86Test {
public:
  X86Test_CallFloatAsXmmRet() : X86Test("[Call] Float As Xmm Ret") {}

  static void add(X86TestManager& mgr) {
    mgr.add(new X86Test_CallFloatAsXmmRet());
  }

  static float calledFunc(float a, float b) {
    return a * b;
  }

  virtual void compile(X86Compiler& cc) {
    cc.addFunc(FuncSignature2<float, float, float>(CallConv::kIdHost));

    X86Xmm a = cc.newXmmSs("a");
    X86Xmm b = cc.newXmmSs("b");
    X86Xmm ret = cc.newXmmSs("ret");

    cc.setArg(0, a);
    cc.setArg(1, b);

    // Prepare.
    X86Gp fn = cc.newIntPtr("fn");
    cc.mov(fn, imm_ptr(calledFunc));

    // Call function.
    CCFuncCall* call = cc.call(fn, FuncSignature2<float, float, float>(CallConv::kIdHost));

    call->setArg(0, a);
    call->setArg(1, b);
    call->setRet(0, ret);

    cc.ret(ret);
    cc.endFunc();
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect) {
    typedef float (*Func)(float, float);
    Func func = ptr_as_func<Func>(_func);

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

class X86Test_CallDoubleAsXmmRet : public X86Test {
public:
  X86Test_CallDoubleAsXmmRet() : X86Test("[Call] Double As Xmm Ret") {}

  static void add(X86TestManager& mgr) {
    mgr.add(new X86Test_CallDoubleAsXmmRet());
  }

  static double calledFunc(double a, double b) {
    return a * b;
  }

  virtual void compile(X86Compiler& cc) {
    cc.addFunc(FuncSignature2<double, double, double>(CallConv::kIdHost));

    X86Xmm a = cc.newXmmSd("a");
    X86Xmm b = cc.newXmmSd("b");
    X86Xmm ret = cc.newXmmSd("ret");

    cc.setArg(0, a);
    cc.setArg(1, b);

    X86Gp fn = cc.newIntPtr("fn");
    cc.mov(fn, imm_ptr(calledFunc));

    CCFuncCall* call = cc.call(fn, FuncSignature2<double, double, double>(CallConv::kIdHost));

    call->setArg(0, a);
    call->setArg(1, b);
    call->setRet(0, ret);

    cc.ret(ret);
    cc.endFunc();
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect) {
    typedef double (*Func)(double, double);
    Func func = ptr_as_func<Func>(_func);

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

class X86Test_CallConditional : public X86Test {
public:
  X86Test_CallConditional() : X86Test("[Call] Conditional") {}

  static void add(X86TestManager& mgr) {
    mgr.add(new X86Test_CallConditional());
  }

  virtual void compile(X86Compiler& cc) {
    X86Gp x = cc.newInt32("x");
    X86Gp y = cc.newInt32("y");
    X86Gp op = cc.newInt32("op");

    CCFuncCall* call;
    X86Gp result;

    cc.addFunc(FuncSignature3<int, int, int, int>(CallConv::kIdHost));
    cc.setArg(0, x);
    cc.setArg(1, y);
    cc.setArg(2, op);

    Label opAdd = cc.newLabel();
    Label opMul = cc.newLabel();

    cc.cmp(op, 0);
    cc.jz(opAdd);
    cc.cmp(op, 1);
    cc.jz(opMul);

    result = cc.newInt32("result_0");
    cc.mov(result, 0);
    cc.ret(result);

    cc.bind(opAdd);
    result = cc.newInt32("result_1");

    call = cc.call((uint64_t)calledFuncAdd, FuncSignature2<int, int, int>(CallConv::kIdHost));
    call->setArg(0, x);
    call->setArg(1, y);
    call->setRet(0, result);
    cc.ret(result);

    cc.bind(opMul);
    result = cc.newInt32("result_2");

    call = cc.call((uint64_t)calledFuncMul, FuncSignature2<int, int, int>(CallConv::kIdHost));
    call->setArg(0, x);
    call->setArg(1, y);
    call->setRet(0, result);

    cc.ret(result);
    cc.endFunc();
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect) {
    typedef int (*Func)(int, int, int);
    Func func = ptr_as_func<Func>(_func);

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

class X86Test_CallMultiple : public X86Test {
public:
  X86Test_CallMultiple() : X86Test("[Call] Multiple") {}

  static void add(X86TestManager& mgr) {
    mgr.add(new X86Test_CallMultiple());
  }

  static int ASMJIT_FASTCALL calledFunc(int* pInt, int index) {
    return pInt[index];
  }

  virtual void compile(X86Compiler& cc) {
    unsigned int i;

    X86Gp buf = cc.newIntPtr("buf");
    X86Gp acc0 = cc.newInt32("acc0");
    X86Gp acc1 = cc.newInt32("acc1");

    cc.addFunc(FuncSignature1<int, int*>(CallConv::kIdHost));
    cc.setArg(0, buf);

    cc.mov(acc0, 0);
    cc.mov(acc1, 0);

    for (i = 0; i < 4; i++) {
      X86Gp ret = cc.newInt32("ret");
      X86Gp ptr = cc.newIntPtr("ptr");
      X86Gp idx = cc.newInt32("idx");
      CCFuncCall* call;

      cc.mov(ptr, buf);
      cc.mov(idx, static_cast<int>(i));

      call = cc.call((uint64_t)calledFunc, FuncSignature2<int, int*, int>(CallConv::kIdHostFastCall));
      call->setArg(0, ptr);
      call->setArg(1, idx);
      call->setRet(0, ret);

      cc.add(acc0, ret);

      cc.mov(ptr, buf);
      cc.mov(idx, static_cast<int>(i));

      call = cc.call((uint64_t)calledFunc, FuncSignature2<int, int*, int>(CallConv::kIdHostFastCall));
      call->setArg(0, ptr);
      call->setArg(1, idx);
      call->setRet(0, ret);

      cc.sub(acc1, ret);
    }

    cc.add(acc0, acc1);
    cc.ret(acc0);
    cc.endFunc();
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect) {
    typedef int (*Func)(int*);
    Func func = ptr_as_func<Func>(_func);

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

class X86Test_CallRecursive : public X86Test {
public:
  X86Test_CallRecursive() : X86Test("[Call] Recursive") {}

  static void add(X86TestManager& mgr) {
    mgr.add(new X86Test_CallRecursive());
  }

  virtual void compile(X86Compiler& cc) {
    X86Gp val = cc.newInt32("val");
    Label skip = cc.newLabel();

    CCFunc* func = cc.addFunc(FuncSignature1<int, int>(CallConv::kIdHost));
    cc.setArg(0, val);

    cc.cmp(val, 1);
    cc.jle(skip);

    X86Gp tmp = cc.newInt32("tmp");
    cc.mov(tmp, val);
    cc.dec(tmp);

    CCFuncCall* call = cc.call(func->getLabel(), FuncSignature1<int, int>(CallConv::kIdHost));
    call->setArg(0, tmp);
    call->setRet(0, tmp);
    cc.mul(cc.newInt32(), val, tmp);

    cc.bind(skip);
    cc.ret(val);
    cc.endFunc();
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect) {
    typedef int (*Func)(int);
    Func func = ptr_as_func<Func>(_func);

    int resultRet = func(5);
    int expectRet = 1 * 2 * 3 * 4 * 5;

    result.setFormat("ret=%d", resultRet);
    expect.setFormat("ret=%d", expectRet);

    return resultRet == expectRet;
  }
};

// ============================================================================
// [X86Test_CallMisc1]
// ============================================================================

class X86Test_CallMisc1 : public X86Test {
public:
  X86Test_CallMisc1() : X86Test("[Call] Misc #1") {}

  static void add(X86TestManager& mgr) {
    mgr.add(new X86Test_CallMisc1());
  }

  static void dummy(int a, int b) {}

  virtual void compile(X86Compiler& cc) {
    X86Gp val = cc.newInt32("val");
    Label skip = cc.newLabel();

    CCFunc* func = cc.addFunc(FuncSignature2<int, int, int>(CallConv::kIdHost));

    X86Gp a = cc.newInt32("a");
    X86Gp b = cc.newInt32("b");
    X86Gp r = cc.newInt32("r");

    cc.setArg(0, a);
    cc.setArg(1, b);

    cc.alloc(a, x86::eax);
    cc.alloc(b, x86::ebx);

    CCFuncCall* call = cc.call(imm_ptr(dummy), FuncSignature2<void, int, int>(CallConv::kIdHost));
    call->setArg(0, a);
    call->setArg(1, b);

    cc.lea(r, x86::ptr(a, b));
    cc.ret(r);

    cc.endFunc();
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect) {
    typedef int (*Func)(int, int);
    Func func = ptr_as_func<Func>(_func);

    int resultRet = func(44, 199);
    int expectRet = 243;

    result.setFormat("ret=%d", resultRet);
    expect.setFormat("ret=%d", expectRet);

    return resultRet == expectRet;
  }
};

// ============================================================================
// [X86Test_CallMisc2]
// ============================================================================

class X86Test_CallMisc2 : public X86Test {
public:
  X86Test_CallMisc2() : X86Test("[Call] Misc #2") {}

  static void add(X86TestManager& mgr) {
    mgr.add(new X86Test_CallMisc2());
  }

  virtual void compile(X86Compiler& cc) {
    CCFunc* func = cc.addFunc(FuncSignature1<double, const double*>(CallConv::kIdHost));

    X86Gp p = cc.newIntPtr("p");
    X86Gp fn = cc.newIntPtr("fn");

    X86Xmm arg = cc.newXmmSd("arg");
    X86Xmm ret = cc.newXmmSd("ret");

    cc.setArg(0, p);
    cc.movsd(arg, x86::ptr(p));
    cc.mov(fn, imm_ptr(op));

    CCFuncCall* call = cc.call(fn, FuncSignature1<double, double>(CallConv::kIdHost));
    call->setArg(0, arg);
    call->setRet(0, ret);

    cc.ret(ret);
    cc.endFunc();
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect) {
    typedef double (*Func)(const double*);
    Func func = ptr_as_func<Func>(_func);

    double arg = 2;

    double resultRet = func(&arg);
    double expectRet = op(arg);

    result.setFormat("ret=%g", resultRet);
    expect.setFormat("ret=%g", expectRet);

    return resultRet == expectRet;
  }

  static double op(double a) { return a * a; }
};

// ============================================================================
// [X86Test_CallMisc3]
// ============================================================================

class X86Test_CallMisc3 : public X86Test {
public:
  X86Test_CallMisc3() : X86Test("[Call] Misc #3") {}

  static void add(X86TestManager& mgr) {
    mgr.add(new X86Test_CallMisc3());
  }

  virtual void compile(X86Compiler& cc) {
    CCFunc* func = cc.addFunc(FuncSignature1<double, const double*>(CallConv::kIdHost));

    X86Gp p = cc.newIntPtr("p");
    X86Gp fn = cc.newIntPtr("fn");

    X86Xmm arg = cc.newXmmSd("arg");
    X86Xmm ret = cc.newXmmSd("ret");

    cc.setArg(0, p);
    cc.movsd(arg, x86::ptr(p));
    cc.mov(fn, imm_ptr(op));

    CCFuncCall* call = cc.call(fn, FuncSignature1<double, double>(CallConv::kIdHost));
    call->setArg(0, arg);
    call->setRet(0, ret);

    cc.xorps(arg, arg);
    cc.subsd(arg, ret);

    cc.ret(arg);
    cc.endFunc();
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect) {
    typedef double (*Func)(const double*);
    Func func = ptr_as_func<Func>(_func);

    double arg = 2;

    double resultRet = func(&arg);
    double expectRet = -op(arg);

    result.setFormat("ret=%g", resultRet);
    expect.setFormat("ret=%g", expectRet);

    return resultRet == expectRet;
  }

  static double op(double a) { return a * a; }
};

// ============================================================================
// [X86Test_CallMisc4]
// ============================================================================

class X86Test_CallMisc4 : public X86Test {
public:
  X86Test_CallMisc4() : X86Test("[Call] Misc #4") {}

  static void add(X86TestManager& mgr) {
    mgr.add(new X86Test_CallMisc4());
  }

  virtual void compile(X86Compiler& cc) {
    FuncSignatureX funcPrototype;

    funcPrototype.setCallConv(CallConv::kIdHost);
    funcPrototype.setRet(TypeId::kF64);
    CCFunc* func = cc.addFunc(funcPrototype);

    FuncSignatureX callPrototype;
    callPrototype.setCallConv(CallConv::kIdHost);
    callPrototype.setRet(TypeId::kF64);
    CCFuncCall* call = cc.call(imm_ptr(calledFunc), callPrototype);

    X86Xmm ret = cc.newXmmSd("ret");
    call->setRet(0, ret);
    cc.ret(ret);

    cc.endFunc();
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect) {
    typedef double (*Func)(void);
    Func func = ptr_as_func<Func>(_func);

    double resultRet = func();
    double expectRet = 3.14;

    result.setFormat("ret=%g", resultRet);
    expect.setFormat("ret=%g", expectRet);

    return resultRet == expectRet;
  }

  static double calledFunc() { return 3.14; }
};

// ============================================================================
// [X86Test_CallMisc5]
// ============================================================================

// The register allocator should clobber the register used by the `call` itself.
class X86Test_CallMisc5 : public X86Test {
public:
  X86Test_CallMisc5() : X86Test("[Call] Misc #5") {}

  static void add(X86TestManager& mgr) {
    mgr.add(new X86Test_CallMisc5());
  }

  virtual void compile(X86Compiler& cc) {
    CCFunc* func = cc.addFunc(FuncSignature0<int>(CallConv::kIdHost));

    X86Gp pFn = cc.newIntPtr("pFn");
    X86Gp vars[16];

    uint32_t i, regCount = cc.getGpCount();
    ASMJIT_ASSERT(regCount <= ASMJIT_ARRAY_SIZE(vars));

    cc.mov(pFn, imm_ptr(calledFunc));
    cc.spill(pFn);

    for (i = 0; i < regCount; i++) {
      if (i == X86Gp::kIdBp || i == X86Gp::kIdSp)
        continue;

      vars[i] = cc.newInt32("v%u", static_cast<unsigned int>(i));
      cc.alloc(vars[i], i);
      cc.mov(vars[i], 1);
    }

    CCFuncCall* call = cc.call(pFn, FuncSignature0<void>(CallConv::kIdHost));

    for (i = 1; i < regCount; i++) {
      if (vars[i].isValid())
        cc.add(vars[0], vars[i]);
    }

    cc.ret(vars[0]);
    cc.endFunc();
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect) {
    typedef int (*Func)(void);
    Func func = ptr_as_func<Func>(_func);

    int resultRet = func();
    int expectRet = sizeof(void*) == 4 ? 6 : 14;

    result.setFormat("ret=%d", resultRet);
    expect.setFormat("ret=%d", expectRet);

    return resultRet == expectRet;
  }

  static void calledFunc() {}
};

// ============================================================================
// [X86Test_MiscConstPool]
// ============================================================================

class X86Test_MiscConstPool : public X86Test {
public:
  X86Test_MiscConstPool() : X86Test("[Misc] ConstPool #1") {}

  static void add(X86TestManager& mgr) {
    mgr.add(new X86Test_MiscConstPool());
  }

  virtual void compile(X86Compiler& cc) {
    cc.addFunc(FuncSignature0<int>(CallConv::kIdHost));

    X86Gp v0 = cc.newInt32("v0");
    X86Gp v1 = cc.newInt32("v1");

    X86Mem c0 = cc.newInt32Const(kConstScopeLocal, 200);
    X86Mem c1 = cc.newInt32Const(kConstScopeLocal, 33);

    cc.mov(v0, c0);
    cc.mov(v1, c1);
    cc.add(v0, v1);

    cc.ret(v0);
    cc.endFunc();
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect) {
    typedef int (*Func)(void);
    Func func = ptr_as_func<Func>(_func);

    int resultRet = func();
    int expectRet = 233;

    result.setFormat("ret=%d", resultRet);
    expect.setFormat("ret=%d", expectRet);

    return resultRet == expectRet;
  }
};

// ============================================================================
// [X86Test_MiscMultiRet]
// ============================================================================

struct X86Test_MiscMultiRet : public X86Test {
  X86Test_MiscMultiRet() : X86Test("[Misc] MultiRet") {}

  static void add(X86TestManager& mgr) {
    mgr.add(new X86Test_MiscMultiRet());
  }

  virtual void compile(X86Compiler& cc) {
    cc.addFunc(FuncSignature3<int, int, int, int>(CallConv::kIdHost));

    X86Gp op = cc.newInt32("op");
    X86Gp a = cc.newInt32("a");
    X86Gp b = cc.newInt32("b");

    Label L_Zero = cc.newLabel();
    Label L_Add = cc.newLabel();
    Label L_Sub = cc.newLabel();
    Label L_Mul = cc.newLabel();
    Label L_Div = cc.newLabel();

    cc.setArg(0, op);
    cc.setArg(1, a);
    cc.setArg(2, b);

    cc.cmp(op, 0);
    cc.jz(L_Add);

    cc.cmp(op, 1);
    cc.jz(L_Sub);

    cc.cmp(op, 2);
    cc.jz(L_Mul);

    cc.cmp(op, 3);
    cc.jz(L_Div);

    cc.bind(L_Zero);
    cc.xor_(a, a);
    cc.ret(a);

    cc.bind(L_Add);
    cc.add(a, b);
    cc.ret(a);

    cc.bind(L_Sub);
    cc.sub(a, b);
    cc.ret(a);

    cc.bind(L_Mul);
    cc.imul(a, b);
    cc.ret(a);

    cc.bind(L_Div);
    cc.cmp(b, 0);
    cc.jz(L_Zero);

    X86Gp zero = cc.newInt32("zero");
    cc.xor_(zero, zero);
    cc.idiv(zero, a, b);
    cc.ret(a);

    cc.endFunc();
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect) {
    typedef int (*Func)(int, int, int);

    Func func = ptr_as_func<Func>(_func);

    int a = 44;
    int b = 3;

    int r0 = func(0, a, b);
    int r1 = func(1, a, b);
    int r2 = func(2, a, b);
    int r3 = func(3, a, b);
    int e0 = a + b;
    int e1 = a - b;
    int e2 = a * b;
    int e3 = a / b;

    result.setFormat("ret={%d %d %d %d}", r0, r1, r2, r3);
    expect.setFormat("ret={%d %d %d %d}", e0, e1, e2, e3);

    return result.eq(expect);
  }
};

// ============================================================================
// [X86Test_MiscMultiFunc]
// ============================================================================

class X86Test_MiscMultiFunc : public X86Test {
public:
  X86Test_MiscMultiFunc() : X86Test("[Misc] MultiFunc") {}

  static void add(X86TestManager& mgr) {
    mgr.add(new X86Test_MiscMultiFunc());
  }

  virtual void compile(X86Compiler& cc) {
    CCFunc* f1 = cc.newFunc(FuncSignature2<int, int, int>(CallConv::kIdHost));
    CCFunc* f2 = cc.newFunc(FuncSignature2<int, int, int>(CallConv::kIdHost));

    {
      X86Gp a = cc.newInt32("a");
      X86Gp b = cc.newInt32("b");

      cc.addFunc(f1);
      cc.setArg(0, a);
      cc.setArg(1, b);

      CCFuncCall* call = cc.call(f2->getLabel(), FuncSignature2<int, int, int>(CallConv::kIdHost));
      call->setArg(0, a);
      call->setArg(1, b);
      call->setRet(0, a);

      cc.ret(a);
      cc.endFunc();
    }

    {
      X86Gp a = cc.newInt32("a");
      X86Gp b = cc.newInt32("b");

      cc.addFunc(f2);
      cc.setArg(0, a);
      cc.setArg(1, b);

      cc.add(a, b);
      cc.ret(a);
      cc.endFunc();
    }
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect) {
    typedef int (*Func)(int, int);

    Func func = ptr_as_func<Func>(_func);

    int resultRet = func(56, 22);
    int expectRet = 56 + 22;

    result.setFormat("ret=%d", resultRet);
    expect.setFormat("ret=%d", expectRet);

    return result.eq(expect);
  }
};

// ============================================================================
// [X86Test_MiscFastEval]
// ============================================================================

class X86Test_MiscFastEval : public X86Test {
public:
  X86Test_MiscFastEval() : X86Test("[Misc] FastEval (CConv)") {}

  static void add(X86TestManager& mgr) {
    mgr.add(new X86Test_MiscFastEval());
  }

  virtual void compile(X86Compiler& cc) {
    FuncSignature5<void, const void*, const void*, const void*, const void*, void*> funcSig(CallConv::kIdHostCDecl);
    FuncSignature2<X86Xmm, X86Xmm, X86Xmm> fastSig(CallConv::kIdHostFastEval2);

    CCFunc* func = cc.newFunc(funcSig);
    CCFunc* fast = cc.newFunc(fastSig);

    {
      X86Gp aPtr = cc.newIntPtr("aPtr");
      X86Gp bPtr = cc.newIntPtr("bPtr");
      X86Gp cPtr = cc.newIntPtr("cPtr");
      X86Gp dPtr = cc.newIntPtr("dPtr");
      X86Gp pOut = cc.newIntPtr("pOut");

      X86Xmm aXmm = cc.newXmm("aXmm");
      X86Xmm bXmm = cc.newXmm("bXmm");
      X86Xmm cXmm = cc.newXmm("cXmm");
      X86Xmm dXmm = cc.newXmm("dXmm");

      cc.addFunc(func);

      cc.setArg(0, aPtr);
      cc.setArg(1, bPtr);
      cc.setArg(2, cPtr);
      cc.setArg(3, dPtr);
      cc.setArg(4, pOut);

      cc.movups(aXmm, x86::ptr(aPtr));
      cc.movups(bXmm, x86::ptr(bPtr));
      cc.movups(cXmm, x86::ptr(cPtr));
      cc.movups(dXmm, x86::ptr(dPtr));

      X86Xmm xXmm = cc.newXmm("xXmm");
      X86Xmm yXmm = cc.newXmm("yXmm");

      CCFuncCall* call1 = cc.call(fast->getLabel(), fastSig);
      call1->setArg(0, aXmm);
      call1->setArg(1, bXmm);
      call1->setRet(0, xXmm);

      CCFuncCall* call2 = cc.call(fast->getLabel(), fastSig);
      call2->setArg(0, cXmm);
      call2->setArg(1, dXmm);
      call2->setRet(0, yXmm);

      cc.pmullw(xXmm, yXmm);
      cc.movups(x86::ptr(pOut), xXmm);

      cc.endFunc();
    }

    {
      X86Xmm aXmm = cc.newXmm("aXmm");
      X86Xmm bXmm = cc.newXmm("bXmm");

      cc.addFunc(fast);
      cc.setArg(0, aXmm);
      cc.setArg(1, bXmm);
      cc.paddw(aXmm, bXmm);
      cc.ret(aXmm);
      cc.endFunc();
    }
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect) {
    typedef void (*Func)(const void*, const void*, const void*, const void*, void*);

    Func func = ptr_as_func<Func>(_func);

    int16_t a[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };
    int16_t b[8] = { 7, 6, 5, 4, 3, 2, 1, 0 };
    int16_t c[8] = { 1, 3, 9, 7, 5, 4, 2, 1 };
    int16_t d[8] = { 2, 0,-6,-4,-2,-1, 1, 2 };

    int16_t o[8];
    int oExp = 7 * 3;

    func(a, b, c, d, o);

    result.setFormat("ret={%02X %02X %02X %02X %02X %02X %02X %02X}", o[0], o[1], o[2], o[3], o[4], o[5], o[6], o[7]);
    expect.setFormat("ret={%02X %02X %02X %02X %02X %02X %02X %02X}", oExp, oExp, oExp, oExp, oExp, oExp, oExp, oExp);

    return result == expect;
  }
};

// ============================================================================
// [X86Test_MiscUnfollow]
// ============================================================================

// Global (I didn't find a better way to test this).
static jmp_buf globalJmpBuf;

class X86Test_MiscUnfollow : public X86Test {
public:
  X86Test_MiscUnfollow() : X86Test("[Misc] Unfollow") {}

  static void add(X86TestManager& mgr) {
    mgr.add(new X86Test_MiscUnfollow());
  }

  virtual void compile(X86Compiler& cc) {
    // NOTE: Fastcall calling convention is the most appropriate here, as all
    // arguments will be passed by registers and there won't be any stack
    // misalignment when we call the `handler()`. This was failing on OSX
    // when targeting 32-bit.
    cc.addFunc(FuncSignature2<void, int, void*>(CallConv::kIdHostFastCall));

    X86Gp a = cc.newInt32("a");
    X86Gp b = cc.newIntPtr("b");

    Label tramp = cc.newLabel();

    cc.setArg(0, a);
    cc.setArg(1, b);

    cc.cmp(a, 0);
    cc.jz(tramp);

    cc.ret(a);

    cc.bind(tramp);
    cc.unfollow().jmp(b);

    cc.endFunc();
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect) {
    typedef int (ASMJIT_FASTCALL *Func)(int, void*);

    Func func = ptr_as_func<Func>(_func);

    int resultRet = 0;
    int expectRet = 1;

    if (!setjmp(globalJmpBuf))
      resultRet = func(0, (void*)handler);
    else
      resultRet = 1;

    result.setFormat("ret={%d}", resultRet);
    expect.setFormat("ret={%d}", expectRet);

    return resultRet == expectRet;
  }

  static void ASMJIT_FASTCALL handler() { longjmp(globalJmpBuf, 1); }
};

// ============================================================================
// [X86Test_Bug100]
// ============================================================================

class X86Test_Bug100 : public X86Test {
public:
  X86Test_Bug100() : X86Test("[Alloc] Bug#100") {}

  static void add(X86TestManager& mgr) {
    mgr.add(new X86Test_Bug100());
  }

  virtual void compile(X86Compiler& cc) {
    cc.addFunc(FuncSignature4<void, void*, uint32_t, uint32_t, uint32_t>(CallConv::kIdHost));

    Label L2 = cc.newLabel();
    Label L3 = cc.newLabel();
    Label L4 = cc.newLabel();

    X86Gp dst = cc.newIntPtr("dst");
    X86Gp v0 = cc.newU32("v0");
    X86Gp v1 = cc.newU32("v1");
    X86Gp v2 = cc.newU32("v2");

    cc.setArg(0, dst);
    cc.setArg(1, v0);
    cc.setArg(2, v1);
    cc.setArg(3, v2);

    cc.cmp(v0, 65535);
    cc.jne(L2);

    cc.cmp(v0, v1);
    cc.je(L3);

    cc.mov(v0, v2);
    cc.jmp(cc.getFunc()->getExitLabel());

    cc.bind(L3);
    cc.bind(L4);

    cc.mov(v2, v1);
    cc.cmp(v1, 65535);
    cc.jne(L2);

    cc.mov(v0, 128);

    cc.bind(L2);
    cc.mov(x86::ptr(dst), v0);

    cc.endFunc();
  }

  virtual bool run(void* _func, StringBuilder& result, StringBuilder& expect) {
    // TODO: This test is not complete.
    // typedef void (*Func)(void*, const void*, size_t);
    // Func func = ptr_as_func<Func>(_func);
    return result == expect;
  }
};

// ============================================================================
// [CmdLine]
// ============================================================================

class CmdLine {
public:
  CmdLine(int argc, char** argv)
    : _argc(argc),
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

#define ADD_TEST(CLASS) CLASS::add(testMgr)

int main(int argc, char* argv[]) {
  X86TestManager testMgr;
  CmdLine cmd(argc, argv);

  if (cmd.hasArg("--verbose"))
    testMgr._verbose = true;

  // Align.
  ADD_TEST(X86Test_AlignBase);
  ADD_TEST(X86Test_AlignNone);

  // Jump.
  ADD_TEST(X86Test_JumpCross);

  ADD_TEST(X86Test_JumpMany);
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
  ADD_TEST(X86Test_AllocIdiv1);
  ADD_TEST(X86Test_AllocSetz);
  ADD_TEST(X86Test_AllocShlRor);
  ADD_TEST(X86Test_AllocGpLo);
  ADD_TEST(X86Test_AllocRepMovsb);
  ADD_TEST(X86Test_AllocIfElse1);
  ADD_TEST(X86Test_AllocIfElse2);
  ADD_TEST(X86Test_AllocIfElse3);
  ADD_TEST(X86Test_AllocIfElse4);
  ADD_TEST(X86Test_AllocInt8);
  ADD_TEST(X86Test_AllocArgsIntPtr);
  ADD_TEST(X86Test_AllocArgsFloat);
  ADD_TEST(X86Test_AllocArgsDouble);
  ADD_TEST(X86Test_AllocRetFloat);
  ADD_TEST(X86Test_AllocRetDouble);
  ADD_TEST(X86Test_AllocStack1);
  ADD_TEST(X86Test_AllocStack2);
  ADD_TEST(X86Test_AllocMemcpy);
  ADD_TEST(X86Test_AllocAlphaBlend);

  // Call.
  ADD_TEST(X86Test_CallBase);
  ADD_TEST(X86Test_CallFast);
  ADD_TEST(X86Test_CallManyArgs);
  ADD_TEST(X86Test_CallDuplicateArgs);
  ADD_TEST(X86Test_CallImmArgs);
  ADD_TEST(X86Test_CallPtrArgs);
  ADD_TEST(X86Test_CallFloatAsXmmRet);
  ADD_TEST(X86Test_CallDoubleAsXmmRet);
  ADD_TEST(X86Test_CallConditional);
  ADD_TEST(X86Test_CallMultiple);
  ADD_TEST(X86Test_CallRecursive);
  ADD_TEST(X86Test_CallMisc1);
  ADD_TEST(X86Test_CallMisc2);
  ADD_TEST(X86Test_CallMisc3);
  ADD_TEST(X86Test_CallMisc4);
  ADD_TEST(X86Test_CallMisc5);

  // Misc.
  ADD_TEST(X86Test_MiscConstPool);
  ADD_TEST(X86Test_MiscMultiRet);
  ADD_TEST(X86Test_MiscMultiFunc);
  ADD_TEST(X86Test_MiscFastEval);
  ADD_TEST(X86Test_MiscUnfollow);

  // Bugs.
  ADD_TEST(X86Test_Bug100);

  return testMgr.run();
}
