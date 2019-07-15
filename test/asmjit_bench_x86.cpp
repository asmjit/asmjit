// [AsmJit]
// Machine Code Generation for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

#include <stdio.h>
#include <string.h>

#include "./asmjit.h"
#include "./asmjit_test_misc.h"
#include "./asmjit_test_opcode.h"

using namespace asmjit;

// ============================================================================
// [Configuration]
// ============================================================================

static constexpr uint32_t kNumRepeats = 25;
static constexpr uint32_t kNumIterations = 1000;

// ============================================================================
// [BenchUtils]
// ============================================================================

namespace BenchUtils {
  class Performance {
  public:
    inline Performance() noexcept { reset(); }

    inline void reset() noexcept {
      tick = 0u;
      best = 0xFFFFFFFFu;
    }

    inline uint32_t start() noexcept { return (tick = now()); }
    inline uint32_t diff() const noexcept { return now() - tick; }

    inline uint32_t end() noexcept {
      tick = diff();
      if (best > tick)
        best = tick;
      return tick;
    }

    static inline uint32_t now() noexcept {
      return OSUtils::getTickCount();
    }

    uint32_t tick;
    uint32_t best;
  };

  static double mbps(uint32_t time, uint64_t outputSize) noexcept {
    if (!time) return 0.0;

    double bytesTotal = double(outputSize);
    return (bytesTotal * 1000) / (double(time) * 1024 * 1024);
  }

  template<typename EmitterT, typename FuncT>
  static void bench(CodeHolder& code, uint32_t archId, const char* testName, const FuncT& func) noexcept {
    EmitterT emitter;

    const char* archName =
      archId == ArchInfo::kIdX86 ? "X86" :
      archId == ArchInfo::kIdX64 ? "X64" : "???";

    const char* emitterName =
      emitter.isAssembler() ? "Assembler" :
      emitter.isCompiler()  ? "Compiler"  :
      emitter.isBuilder()   ? "Builder"   : "Unknown";

    Performance perf;
    uint64_t codeSize = 0;

    CodeInfo codeInfo(archId);
    codeInfo.setCdeclCallConv(archId == ArchInfo::kIdX86 ? CallConv::kIdX86CDecl : CallConv::kIdX86SysV64);

    for (uint32_t r = 0; r < kNumRepeats; r++) {
      perf.start();
      codeSize = 0;
      for (uint32_t i = 0; i < kNumIterations; i++) {
        code.init(codeInfo);
        code.attach(&emitter);

        func(emitter);
        codeSize += code.codeSize();

        code.reset();
      }
      perf.end();
    }

    printf("[%s] %-9s %-8s | Time:%6u [ms] | ", archName, emitterName, testName, perf.best);
    if (codeSize)
      printf("Speed: %7.3f [MB/s]", mbps(perf.best, codeSize));
    else
      printf("Speed: N/A");
    printf("\n");
  }
}

// ============================================================================
// [Main]
// ============================================================================

#ifdef ASMJIT_BUILD_X86
static void benchX86(uint32_t archId) noexcept {
  CodeHolder code;

  BenchUtils::bench<x86::Assembler>(code, archId, "[raw]", [](x86::Assembler& a) {
    asmtest::generateOpcodes(a.as<x86::Emitter>());
  });

  BenchUtils::bench<x86::Builder>(code, archId, "[raw]", [](x86::Builder& cb) {
    asmtest::generateOpcodes(cb.as<x86::Emitter>());
  });

  BenchUtils::bench<x86::Builder>(code, archId, "[final]", [](x86::Builder& cb) {
    asmtest::generateOpcodes(cb.as<x86::Emitter>());
    cb.finalize();
  });

  BenchUtils::bench<x86::Compiler>(code, archId, "[raw]", [](x86::Compiler& cc) {
    asmtest::generateAlphaBlend(cc);
  });

  BenchUtils::bench<x86::Compiler>(code, archId, "[final]", [](x86::Compiler& cc) {
    asmtest::generateAlphaBlend(cc);
    cc.finalize();
  });
}
#endif

int main(int argc, char* argv[]) {
  ASMJIT_UNUSED(argc);
  ASMJIT_UNUSED(argv);

  #ifdef ASMJIT_BUILD_X86
  benchX86(ArchInfo::kIdX86);
  benchX86(ArchInfo::kIdX64);
  #endif

  return 0;
}
