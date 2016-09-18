// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Dependencies]
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./asmjit.h"
#include "./asmjit_test_misc.h"
#include "./asmjit_test_opcode.h"

using namespace asmjit;

// ============================================================================
// [Configuration]
// ============================================================================

static const uint32_t kNumRepeats = 10;
static const uint32_t kNumIterations = 5000;

// ============================================================================
// [Performance]
// ============================================================================

struct Performance {
  static inline uint32_t now() {
    return OSUtils::getTickCount();
  }

  inline void reset() {
    tick = 0;
    best = 0xFFFFFFFF;
  }

  inline uint32_t start() { return (tick = now()); }
  inline uint32_t diff() const { return now() - tick; }

  inline uint32_t end() {
    tick = diff();
    if (best > tick)
      best = tick;
    return tick;
  }

  uint32_t tick;
  uint32_t best;
};

static double mbps(uint32_t time, size_t outputSize) {
  if (!time) return 0.0;

  double bytesTotal = static_cast<double>(outputSize);
  return (bytesTotal * 1000) / (static_cast<double>(time) * 1024 * 1024);
}

// ============================================================================
// [Main]
// ============================================================================

#if defined(ASMJIT_BUILD_X86)
static void benchX86(uint32_t archType) {
  CodeHolder code;
  Performance perf;

  X86Assembler a;
  X86Compiler cc;

  uint32_t r, i;
  const char* archName = archType == ArchInfo::kTypeX86 ? "X86" : "X64";

  // --------------------------------------------------------------------------
  // [Bench - Assembler]
  // --------------------------------------------------------------------------

  size_t asmOutputSize = 0;
  size_t cmpOutputSize = 0;

  perf.reset();
  for (r = 0; r < kNumRepeats; r++) {
    asmOutputSize = 0;
    perf.start();
    for (i = 0; i < kNumIterations; i++) {
      code.init(CodeInfo(archType));
      code.attach(&a);

      asmtest::generateOpcodes(a);
      asmOutputSize += code.getCodeSize();

      code.reset(false); // Detaches `a`.
    }
    perf.end();
  }

  printf("%-12s (%s) | Time: %-6u [ms] | Speed: %7.3f [MB/s]\n",
    "X86Assembler", archName, perf.best, mbps(perf.best, asmOutputSize));

  // --------------------------------------------------------------------------
  // [Bench - CodeBuilder]
  // --------------------------------------------------------------------------

  // TODO:

  // --------------------------------------------------------------------------
  // [Bench - CodeCompiler]
  // --------------------------------------------------------------------------

  perf.reset();
  for (r = 0; r < kNumRepeats; r++) {
    cmpOutputSize = 0;
    perf.start();
    for (i = 0; i < kNumIterations; i++) {
      // NOTE: Since we don't have JitRuntime we don't know anything about
      // function calling conventions, which is required by generateAlphaBlend.
      // So we must setup this manually.
      CodeInfo ci(archType);
      ci.setCdeclCallConv(archType == ArchInfo::kTypeX86 ? CallConv::kIdX86CDecl : CallConv::kIdX86SysV64);

      code.init(ci);
      code.attach(&cc);

      asmtest::generateAlphaBlend(cc);
      cc.finalize();
      cmpOutputSize += code.getCodeSize();

      code.reset(false); // Detaches `cc`.
    }
    perf.end();
  }

  printf("%-12s (%s) | Time: %-6u [ms] | Speed: %7.3f [MB/s]\n",
    "X86Compiler", archName, perf.best, mbps(perf.best, cmpOutputSize));
}
#endif

int main(int argc, char* argv[]) {
#if defined(ASMJIT_BUILD_X86)
  benchX86(ArchInfo::kTypeX86);
  benchX86(ArchInfo::kTypeX64);
#endif // ASMJIT_BUILD_X86

  return 0;
}
