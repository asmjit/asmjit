// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Dependencies - AsmJit]
#include "../asmjit/asmjit.h"

// [Dependencies - Test]
#include "./asmjit_test_opcode.h"
#include "./genblend.h"

// [Dependencies - C]
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ============================================================================
// [Configuration]
// ============================================================================

static const uint32_t kNumRepeats = 10;
static const uint32_t kNumIterations = 5000;

// ============================================================================
// [TestRuntime]
// ============================================================================

struct TestRuntime : public asmjit::Runtime {
  ASMJIT_NO_COPY(TestRuntime)

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  TestRuntime(uint32_t arch, uint32_t callConv) ASMJIT_NOEXCEPT {
    _cpuInfo.setArch(arch);
    _stackAlignment = 16;
    _baseAddress = 0;
    _cdeclConv = static_cast<uint8_t>(callConv);
    _stdCallConv = static_cast<uint8_t>(callConv);
  }
  virtual ~TestRuntime() ASMJIT_NOEXCEPT {}

  // --------------------------------------------------------------------------
  // [Interface]
  // --------------------------------------------------------------------------

  virtual asmjit::Error add(void** dst, asmjit::Assembler* assembler) ASMJIT_NOEXCEPT {
    size_t codeSize = assembler->getCodeSize();
    if (codeSize == 0) {
      *dst = NULL;
      return asmjit::kErrorNoCodeGenerated;
    }

    void* p = ::malloc(codeSize);
    if (p == NULL) {
      *dst = NULL;
      return asmjit::kErrorNoHeapMemory;
    }

    size_t relocSize = assembler->relocCode(p, _baseAddress);
    if (relocSize == 0) {
      ::free(p);
      *dst = NULL;
      return asmjit::kErrorInvalidState;
    }

    *dst = p;
    return asmjit::kErrorOk;
  }

  virtual asmjit::Error release(void* p) ASMJIT_NOEXCEPT {
    ::free(p);
    return asmjit::kErrorOk;
  }
};


// ============================================================================
// [Performance]
// ============================================================================

struct Performance {
  static inline uint32_t now() {
    return asmjit::Utils::getTickCount();
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

static double mbps(uint32_t time, size_t outputSize) {
  double bytesTotal = static_cast<double>(outputSize);
  return (bytesTotal * 1000) / (static_cast<double>(time) * 1024 * 1024);
}

// ============================================================================
// [Main]
// ============================================================================

#if defined(ASMJIT_BUILD_X86) || defined(ASMJIT_BUILD_X64)
static void benchX86(uint32_t arch, uint32_t callConv) {
  using namespace asmjit;

  Performance perf;
  TestRuntime runtime(arch, callConv);

  X86Assembler a(&runtime, arch);
  X86Compiler c;

  uint32_t r, i;

  const char* archName = arch == kArchX86 ? "X86" : "X64";

  // --------------------------------------------------------------------------
  // [Bench - Opcode]
  // --------------------------------------------------------------------------

  size_t asmOutputSize = 0;
  size_t cmpOutputSize = 0;

  perf.reset();
  for (r = 0; r < kNumRepeats; r++) {
    asmOutputSize = 0;
    perf.start();
    for (i = 0; i < kNumIterations; i++) {
      asmgen::opcode(a);

      void *p = a.make();
      runtime.release(p);

      asmOutputSize += a.getCodeSize();
      a.reset();
    }
    perf.end();
  }

  printf("%-12s (%s) | Time: %-6u [ms] | Speed: %7.3f [MB/s]\n",
    "X86Assembler", archName, perf.best, mbps(perf.best, asmOutputSize));

  // --------------------------------------------------------------------------
  // [Bench - Blend]
  // --------------------------------------------------------------------------

  perf.reset();
  for (r = 0; r < kNumRepeats; r++) {
    cmpOutputSize = 0;
    perf.start();
    for (i = 0; i < kNumIterations; i++) {
      c.attach(&a);
      asmgen::blend(c);
      c.finalize();

      void* p = a.make();
      runtime.release(p);

      cmpOutputSize += a.getCodeSize();
      a.reset();
    }
    perf.end();
  }

  printf("%-12s (%s) | Time: %-6u [ms] | Speed: %7.3f [MB/s]\n",
    "X86Compiler", archName, perf.best, mbps(perf.best, cmpOutputSize));
}
#endif

int main(int argc, char* argv[]) {
#if defined(ASMJIT_BUILD_X86)
  benchX86(asmjit::kArchX86, asmjit::kCallConvX86CDecl);
#endif
#if defined(ASMJIT_BUILD_X64)
  benchX86(asmjit::kArchX64, asmjit::kCallConvX64Unix);
#endif

  return 0;
}
