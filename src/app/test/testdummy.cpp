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

typedef int (*MyFunc)(void);

int main(int argc, char* argv[]) {
  using namespace asmjit;
  using namespace asmjit::host;

  JitRuntime runtime;
  FileLogger logger(stderr);
  logger.setOption(kLoggerOptionBinaryForm, true);

  Compiler c(&runtime);
  c.setLogger(&logger);

  c.addFunc(kFuncConvHost, FuncBuilder0<int>());
  c.endFunc();

  MyFunc func = asmjit_cast<MyFunc>(c.make());
  func();

  runtime.release((void*)func);
  return 0;
}
