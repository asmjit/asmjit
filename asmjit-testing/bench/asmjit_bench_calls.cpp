// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core.h>

#if ASMJIT_TARGET_ARCH_X86 == 64 && !defined(ASMJIT_NO_X86) && !defined(ASMJIT_NO_JIT)
  #define ASMJIT_BENCH_CALLS_ENABLED
  #include <asmjit/x86.h>
#endif

#include <asmjit-testing/commons/asmjit_utils.h>

#if defined(ASMJIT_BENCH_CALLS_ENABLED)
  #include <asmjit-testing/commons/zero_bench.h>
#endif

#include <stdint.h>
#include <stdio.h>

#include <initializer_list>
#include <memory>
#include <vector>

using namespace asmjit;

static void print_app_info() {
  printf("AsmJit Benchmark Calls v%u.%u.%u [Arch=%s] [Mode=%s]\n\n",
    unsigned((ASMJIT_LIBRARY_VERSION >> 16)       ),
    unsigned((ASMJIT_LIBRARY_VERSION >>  8) & 0xFF),
    unsigned((ASMJIT_LIBRARY_VERSION      ) & 0xFF),
    asmjit_arch_as_string(Arch::kHost),
    asmjit_build_type()
  );
}

#if defined(ASMJIT_BENCH_CALLS_ENABLED)

static void print_usage() {
  printf("This benchmark measures calls from JIT code to functions of the host module\n"
         "emitted as 'call(imm(fn))'. Each generated function has N call sites in\n"
         "straight-line code, each calling one of 16 helpers. Each output line provides\n"
         "the following columns:\n"
         "\n"
         "  - Placement    - where JitRuntime places the code:\n"
         "    - Default    - anywhere, calls use 'call [rip + addrtab]'\n"
         "    - Near       - CreateParams::near_address set to a helper, calls use\n"
         "                   'call rel32' if the code could be placed near the module\n"
         "  - Call Sites   - number of call sites in the generated function\n"
         "  - Code Size    - size of the generated function\n"
         "  - Direct Calls - number of call sites relocated to 'call rel32'\n"
         "  - ps/Call      - picoseconds per call (minimum and median of all runs)\n"
         "\n");
}

using HelperFunc = uint64_t (ASMJIT_CDECL*)(uint64_t x);
using BenchFunc = uint64_t (ASMJIT_CDECL*)(uint64_t x);

// Helpers are aligned to a cache line, otherwise their timing depends on where the linker places them.
#if defined(__GNUC__)
  #define BENCH_HELPER_ALIGN __attribute__((aligned(64)))
#else
  #define BENCH_HELPER_ALIGN
#endif

template<uint32_t I>
BENCH_HELPER_ALIGN static uint64_t ASMJIT_CDECL bench_helper(uint64_t x) noexcept { return x + I; }

static const HelperFunc bench_helpers[] = {
  bench_helper< 0>, bench_helper< 1>, bench_helper< 2>, bench_helper< 3>,
  bench_helper< 4>, bench_helper< 5>, bench_helper< 6>, bench_helper< 7>,
  bench_helper< 8>, bench_helper< 9>, bench_helper<10>, bench_helper<11>,
  bench_helper<12>, bench_helper<13>, bench_helper<14>, bench_helper<15>
};

static constexpr size_t kHelperCount = ASMJIT_ARRAY_SIZE(bench_helpers);

// Number of calls made by each run, a multiple of every benchmarked number of call sites.
static constexpr size_t kCallsPerRun = size_t(1) << 20;

static uint64_t expected_result(uint64_t x, size_t call_count) noexcept {
  for (size_t i = 0; i < call_count; i++) {
    x = bench_helpers[i % kHelperCount](x);
  }
  return x;
}

enum class Placement : uint32_t {
  kDefault,
  kNear
};

enum ValueId : uint8_t {
  kCodeSize,
  kDirectCalls,
  kCallTime
};

class CallsBenchmark : public ZeroBench::Benchmark {
public:
  size_t call_count;
  Placement placement;

  std::unique_ptr<JitRuntime> rt;
  BenchFunc fn {};
  size_t code_size {};
  size_t direct_calls {};
  uint64_t result {};

  CallsBenchmark(size_t call_count, Placement placement) noexcept
    : call_count(call_count),
      placement(placement) {
    set_label(0, placement == Placement::kNear ? "Near" : "Default");
    set_label(1, ZeroBench::String::format("%zu", call_count));
  }

  ~CallsBenchmark() noexcept override {}

  void prepare(ZeroBench::Context& ctx) override;
  void run(ZeroBench::Context& ctx) override;
  void teardown() override;

  Error generate() noexcept;
};

Error CallsBenchmark::generate() noexcept {
  CodeHolder code;
  ASMJIT_PROPAGATE(code.init(rt->environment()));

  x86::Assembler a(&code);
  x86::Gp arg0 = rt->environment().is_platform_windows() ? x86::rcx : x86::rdi;

  // All call sites have the same size, so remember where the first two start.
  size_t call_offsets[2] {};

  a.sub(x86::rsp, 40);
  a.mov(x86::rax, arg0);
  for (size_t i = 0; i < call_count; i++) {
    a.mov(arg0, x86::rax);
    if (i < 2u) {
      call_offsets[i] = a.offset();
    }
    a.call(imm((void*)bench_helpers[i % kHelperCount]));
  }
  a.add(x86::rsp, 40);
  a.ret();

  ASMJIT_PROPAGATE(rt->add(&fn, &code));
  code_size = code.code_size();

  // Each call site is either `40 E8 rel32` (direct) or `FF 15 disp32` (via address table).
  const uint8_t* p = reinterpret_cast<const uint8_t*>(fn) + call_offsets[0];
  size_t stride = call_offsets[1] - call_offsets[0];

  direct_calls = 0;
  for (size_t i = 0; i < call_count; i++) {
    direct_calls += size_t(p[i * stride + 1u] == 0xE8u);
  }

  return Error::kOk;
}

void CallsBenchmark::prepare(ZeroBench::Context& ctx) {
  JitAllocator::CreateParams params {};
  if (placement == Placement::kNear) {
    params.near_address = uintptr_t(bench_helpers[0]);
  }

  rt = std::make_unique<JitRuntime>(&params);

  Error err = generate();
  if (err != Error::kOk) {
    ctx.make_failed(stringify_error(err));
    return;
  }

  if (fn(1) != expected_result(1, call_count)) {
    ctx.make_failed("The generated function returned a wrong result");
  }
}

void CallsBenchmark::run(ZeroBench::Context& ctx) {
  // Locals, as members would be reloaded after each call.
  BenchFunc f = fn;
  size_t repeat = kCallsPerRun / call_count;
  uint64_t x = result;

  for (size_t i = 0; i < repeat; i++) {
    x = f(x);
  }
  ctx.end_now();

  result = x;
  ctx.set_value(kCodeSize, code_size);
  ctx.set_value(kDirectCalls, direct_calls);
  ctx.set_value(kCallTime, ctx.elapsed_nano() * 1000u / kCallsPerRun);
}

void CallsBenchmark::teardown() {
  rt.reset();
  fn = nullptr;
}

int main(int argc, char* argv[]) {
  ZeroBench::Runner runner(argc, argv);

  print_app_info();

  if (runner.cmd_line().has_arg("--help")) {
    print_usage();
    return 0;
  }

  runner.set_label(0, "Placement");
  runner.set_label(1, "Call Sites");

  runner.add_metric("Code Size", {
    .color = ZeroBench::Color::kBrightMagenta,
    .value_kind = ZeroBench::ValueKind::kBytes,
    .value_index = kCodeSize,
    .aggregate = ZeroBench::Aggregate::kStatic,
    .scale = ZeroBench::Scale::kDefaultBytes
  });
  runner.add_metric("Direct Calls", {
    .color = ZeroBench::kDefaultMetricColor,
    .value_kind = ZeroBench::ValueKind::kCount,
    .value_index = kDirectCalls,
    .aggregate = ZeroBench::Aggregate::kStatic,
    .scale = ZeroBench::Scale::kNone
  });
  runner.add_metric("Call T. Min", {
    .color = ZeroBench::kMinDurationColor,
    .value_kind = ZeroBench::ValueKind::kDurationPico,
    .value_index = kCallTime,
    .aggregate = ZeroBench::Aggregate::kMin,
    .scale = ZeroBench::Scale::kNano
  });
  runner.add_metric("Call T. Med", {
    .color = ZeroBench::kOtherDurationColor,
    .value_kind = ZeroBench::ValueKind::kDurationPico,
    .value_index = kCallTime,
    .aggregate = ZeroBench::Aggregate::kP50,
    .scale = ZeroBench::Scale::kNano
  });

  for (Placement placement : {Placement::kDefault, Placement::kNear}) {
    std::vector<ZeroBench::BenchmarkRef> benchmarks;
    for (size_t call_count : {16u, 32u, 64u, 128u, 256u, 512u, 1024u, 1024u * 4u, 1024u * 16u, 1024u * 64u, 1024u * 256u, 1024u * 1024u}) {
      benchmarks.push_back(ZeroBench::BenchmarkRef::make<CallsBenchmark>(call_count, placement));
    }
    runner.run(benchmarks.data(), benchmarks.size());
    printf("\n");
  }
}

#else

int main() {
  print_app_info();
  printf("!! This Benchmark is only available on X86_64 with JIT support !!\n");
  return 0;
}

#endif // ASMJIT_BENCH_CALLS_ENABLED
