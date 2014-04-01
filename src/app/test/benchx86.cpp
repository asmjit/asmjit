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

int main(int argc, char* argv[]) {
  using namespace asmjit;
  using namespace asmjit::host;

  Performance perf;
  uint32_t kNumRepeats = 10;
  uint32_t kNumIterations = 100000;

  JitRuntime runtime;
  Assembler a(&runtime);
  Compiler c(&runtime);

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

      a.clear();
    }
    perf.end();
  }
  printf("Opcode   | Time: %u [ms]\n", perf.best);

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

      c.clear();
    }
    perf.end();
  }
  printf("Blend    | Time: %u [ms]\n", perf.best);

  return 0;
}
