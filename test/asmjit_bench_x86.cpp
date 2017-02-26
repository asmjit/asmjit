// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Dependencies]
#include <cstdio>
#include <cstring>

#include "./asmjit.h"
#include "./asmjit_test_misc.h"
#include "./asmjit_test_opcode.h"

using namespace asmjit;

// ============================================================================
// [Configuration]
// ============================================================================

static constexpr uint32_t kNumRepeats = 20;
static constexpr uint32_t kNumIterations = 1000;

// ============================================================================
// [BenchUtils]
// ============================================================================

namespace BenchUtils {
  class Performance {
  public:
    inline Performance() noexcept { reset(); }

    inline void reset() noexcept {
      tick = 0U;
      best = 0xFFFFFFFFU;
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

  template<typename EMITTER, typename FUNC>
  static void bench(CodeHolder& code, uint32_t archType, const char* testName, const FUNC& func) noexcept {
    EMITTER emitter;

    const char* archName =
      archType == ArchInfo::kTypeX86 ? "X86" :
      archType == ArchInfo::kTypeX64 ? "X64" : "???";

    const char* emitterName =
      emitter.isAssembler() ? "Assembler" :
      emitter.isCompiler()  ? "Compiler"  :
      emitter.isBuilder()   ? "Builder"   : "Unknown";

    Performance perf;
    uint64_t codeSize = 0;

    CodeInfo codeInfo(archType);
    codeInfo.setCdeclCallConv(archType == ArchInfo::kTypeX86 ? CallConv::kIdX86CDecl : CallConv::kIdX86SysV64);

    for (uint32_t r = 0; r < kNumRepeats; r++) {
      perf.start();
      codeSize = 0;
      for (uint32_t i = 0; i < kNumIterations; i++) {
        code.init(codeInfo);
        code.attach(&emitter);

        func(emitter);
        codeSize += code.getCodeSize();

        code.reset(false);
      }
      perf.end();
    }

    std::printf("[%s] %-9s %-8s | Time:%6u [ms] | ", archName, emitterName, testName, perf.best);
    if (codeSize)
      std::printf("Speed: %7.3f [MB/s]", mbps(perf.best, codeSize));
    else
      std::printf("Speed: N/A");
    std::printf("\n");
  }
}

// ============================================================================
// [Main]
// ============================================================================

#ifdef ASMJIT_BUILD_X86
static void benchX86(uint32_t archType) noexcept {
  CodeHolder code;

  BenchUtils::bench<X86Assembler>(code, archType, "[raw]", [](X86Assembler& a) {
    asmtest::generateOpcodes(a.as<X86Emitter>());
  });

  BenchUtils::bench<X86Builder>(code, archType, "[raw]", [](X86Builder& cb) {
    asmtest::generateOpcodes(cb.as<X86Emitter>());
  });

  BenchUtils::bench<X86Builder>(code, archType, "[final]", [](X86Builder& cb) {
    asmtest::generateOpcodes(cb.as<X86Emitter>());
    cb.finalize();
  });

  BenchUtils::bench<X86Compiler>(code, archType, "[raw]", [](X86Compiler& cc) {
    asmtest::generateAlphaBlend(cc);
  });

  BenchUtils::bench<X86Compiler>(code, archType, "[final]", [](X86Compiler& cc) {
    asmtest::generateAlphaBlend(cc);
    cc.finalize();
  });
}
#endif

int main(int argc, char* argv[]) {
  #ifdef ASMJIT_BUILD_X86
  benchX86(ArchInfo::kTypeX86);
  benchX86(ArchInfo::kTypeX64);
  #endif

  return 0;
}
