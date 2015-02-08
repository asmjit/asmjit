// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Dependencies - AsmJit]
#include <asmjit/asmjit.h>

// [Dependencies - Test]
#include "genblend.h"
#include "genopcode.h"

// [Dependencies - C]
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ============================================================================
// [Performance]
// ============================================================================

struct Performance {
  static inline uint32_t now() {
    return asmjit::CpuTicks::now();
  }

  inline void reset() {
    tick = 0;
    best = 0xFFFFFFFF;
  }

  inline uint32_t start() {
    return (tick = now());
  }

  inline uint32_t diff() const {
    return now() - tick;
  }

  inline uint32_t end() {
    tick = diff();
    if (best > tick)
      best = tick;
    return tick;
  }

  uint32_t tick;
  uint32_t best;
};

// ============================================================================
// [Main]
// ============================================================================

static uint32_t instPerMs(uint32_t time, uint32_t numIterations, uint32_t instPerIteration) {
  return static_cast<uint32_t>(
    static_cast<uint64_t>(numIterations) * instPerIteration * 1000 / time);
}

int main(int argc, char* argv[]) {
  using namespace asmjit;

  Performance perf;
  uint32_t kNumRepeats = 10;
  uint32_t kNumIterations = 10000;

  JitRuntime runtime;
  X86Assembler a(&runtime);
  X86Compiler c(&runtime);

  uint32_t r, i;

  // --------------------------------------------------------------------------
  // [Bench - Opcode]
  // --------------------------------------------------------------------------

  perf.reset();
  for (r = 0; r < kNumRepeats; r++) {
    perf.start();
    for (i = 0; i < kNumIterations; i++) {
      asmgen::opcode(a);

      void *p = a.make();
      runtime.release(p);

      a.reset();
    }
    perf.end();
  }

  printf("Opcode   | Time: %-6u [ms] | Speed: %-9u [inst/s]\n",
    perf.best, instPerMs(perf.best, kNumIterations, asmgen::kGenOpCodeInstCount));

  // --------------------------------------------------------------------------
  // [Bench - Blend]
  // --------------------------------------------------------------------------

  perf.reset();
  for (r = 0; r < kNumRepeats; r++) {
    perf.start();
    for (i = 0; i < kNumIterations; i++) {
      asmgen::blend(c);

      void* p = c.make();
      runtime.release(p);

      c.reset();
    }
    perf.end();
  }

  printf("Blend    | Time: %-6u [ms] | Speed: %-9u [inst/s]\n",
    perf.best, instPerMs(perf.best, kNumIterations, asmgen::kGenBlendInstCount));

  return 0;
}
