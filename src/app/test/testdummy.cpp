// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// This file is used as a dummy test. It's changed during development.

// [Dependencies - AsmJit]
#include <asmjit/asmjit.h>

// [Dependencies - C]
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef void (*MyFunc)(void);

int main(int argc, char* argv[]) {
  using namespace asmjit;
  using namespace asmjit::host;

  JitRuntime runtime;
  FileLogger logger(stderr);
  logger.setOption(kLoggerOptionBinaryForm, true);

  Compiler c(&runtime);
  c.setLogger(&logger);

  c.addFunc(kFuncConvHost, FuncBuilder0<void>());

  Label L_1(c);
  Label L_2(c);
  Label L_3(c);
  Label L_4(c);
  Label L_5(c);
  Label L_6(c);
  Label L_7(c);

  GpVar v1(c);
  GpVar v2(c);

  c.bind(L_2);
  c.bind(L_3);

  c.jmp(L_1);
  c.bind(L_5);
  c.mov(v1, 0);
  c.bind(L_6);
  c.jmp(L_3);
  c.mov(v2, 1);
  c.jmp(L_1);
  c.bind(L_4);
  c.jmp(L_2);
  c.bind(L_7);
  c.add(v1, v2);

  c.bind(L_1);
  c.ret();
  c.endFunc();

  MyFunc func = asmjit_cast<MyFunc>(c.make());
  runtime.release((void*)func);

  return 0;
}
