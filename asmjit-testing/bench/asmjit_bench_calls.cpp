// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core.h>

#if ASMJIT_TARGET_ARCH_X86 == 64 && !defined(ASMJIT_NO_X86) && !defined(ASMJIT_NO_JIT)
  #include <asmjit/x86.h>
#endif

#include <asmjit-testing/commons/asmjit_utils.h>
#include <asmjit-testing/commons/cmdline.h>
#include <asmjit-testing/commons/performance_timer.h>

#include <stdint.h>
#include <stdio.h>

using namespace asmjit;

static void print_app_info(size_t count) noexcept {
  printf("AsmJit Benchmark Calls v%u.%u.%u [Arch=%s] [Mode=%s]\n\n",
    unsigned((ASMJIT_LIBRARY_VERSION >> 16)       ),
    unsigned((ASMJIT_LIBRARY_VERSION >>  8) & 0xFF),
    unsigned((ASMJIT_LIBRARY_VERSION      ) & 0xFF),
    asmjit_arch_as_string(Arch::kHost),
    asmjit_build_type()
  );

  printf("This benchmark measures calls from JIT code to functions of the host module\n"
         "emitted as 'call(imm(fn))'. Each generated function has N call sites in\n"
         "straight-line code, each calling one of 16 helpers. Each output line provides\n"
         "the following columns:\n"
         "\n"
         "  - Call Sites - number of call sites in the generated function\n"
         "  - Code       - size of the generated function\n"
         "  - Default    - default placement, calls use 'call [rip + addrtab]'\n"
         "  - Near       - JitAllocatorOptions::kNearModuleAddressRange, calls use\n"
         "                 'call rel32' if the code could be placed near the module\n"
         "  - Direct     - number of call sites relocated to 'call rel32' (Near)\n"
         "\n"
         "Times are per call (the best of 5 runs).\n\n");

  printf("The number of calls benchmarked: %zu (override by --count=n)\n", count);
  printf("\n");
}

#if ASMJIT_TARGET_ARCH_X86 == 64 && !defined(ASMJIT_NO_X86) && !defined(ASMJIT_NO_JIT)

using HelperFunc = uint64_t (ASMJIT_CDECL*)(uint64_t x);
using BenchFunc = uint64_t (ASMJIT_CDECL*)(uint64_t x);

template<uint32_t I>
static uint64_t ASMJIT_CDECL bench_helper(uint64_t x) noexcept { return x * 33u + I; }

static const HelperFunc bench_helpers[] = {
  bench_helper< 0>, bench_helper< 1>, bench_helper< 2>, bench_helper< 3>,
  bench_helper< 4>, bench_helper< 5>, bench_helper< 6>, bench_helper< 7>,
  bench_helper< 8>, bench_helper< 9>, bench_helper<10>, bench_helper<11>,
  bench_helper<12>, bench_helper<13>, bench_helper<14>, bench_helper<15>
};

static constexpr size_t kHelperCount = ASMJIT_ARRAY_SIZE(bench_helpers);

static uint64_t expected_result(uint64_t x, size_t call_count) noexcept {
  for (size_t i = 0; i < call_count; i++) {
    x = bench_helpers[i % kHelperCount](x);
  }
  return x;
}

// A generated function with `call_count` call sites.
struct BenchFunction {
  JitRuntime rt;
  BenchFunc fn {};
  size_t code_size {};
  size_t direct_calls {};

  explicit BenchFunction(const JitAllocator::CreateParams& params) noexcept
    : rt(&params) {}

  Error generate(size_t call_count) noexcept {
    CodeHolder code;
    ASMJIT_PROPAGATE(code.init(rt.environment()));

    x86::Assembler a(&code);
    x86::Gp arg0 = rt.environment().is_platform_windows() ? x86::rcx : x86::rdi;

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

    ASMJIT_PROPAGATE(rt.add(&fn, &code));
    code_size = code.code_size();

    // Each call site is either `40 E8 rel32` (direct) or `FF 15 disp32` (via address table).
    const uint8_t* p = reinterpret_cast<const uint8_t*>(fn) + call_offsets[0];
    size_t stride = call_offsets[1] - call_offsets[0];

    for (size_t i = 0; i < call_count; i++) {
      direct_calls += size_t(p[i * stride + 1u] == 0xE8u);
    }

    return fn(1) == expected_result(1, call_count) ? Error::kOk : Error::kInvalidState;
  }

  // Returns nanoseconds per call.
  double run(size_t call_count, size_t total_calls) const noexcept {
    size_t repeat = axl::max<size_t>(total_calls / call_count, 1u);
    uint64_t x = 0;
    PerformanceTimer timer;

    timer.start();
    for (size_t i = 0; i < repeat; i++) {
      x = fn(x);
    }
    timer.stop();

    // Keep `x` alive.
    if (x == 0xFFFFFFFFFFFFFFFFu) {
      printf("!");
    }

    return timer.duration() * 1000000.0 / double(repeat * call_count);
  }
};

int main(int argc, char* argv[]) {
  CmdLine cmd_line(argc, argv);
  size_t total_calls = cmd_line.value_as_uint("--count", 20000000);

  print_app_info(total_calls);

  static constexpr size_t call_counts[] = { 16, 256, 1024, 4096, 16384, 65536, 262144 };

  printf("+------------+----------+--------------+--------------+---------+----------------+\n");
  printf("| Call Sites | Code     | Default      | Near         | Speedup | Direct         |\n");
  printf("+------------+----------+--------------+--------------+---------+----------------+\n");

  for (size_t call_count : call_counts) {
    JitAllocator::CreateParams default_params {};
    JitAllocator::CreateParams near_params {};
    near_params.options = JitAllocatorOptions::kNearModuleAddressRange;

    BenchFunction default_func(default_params);
    BenchFunction near_func(near_params);

    Error err = default_func.generate(call_count);
    if (err == Error::kOk) {
      err = near_func.generate(call_count);
    }

    if (err != Error::kOk) {
      printf("Failed to run the benchmark: %s\n", stringify_error(err));
      return 1;
    }

    // Interleave runs so frequency changes affect both the same way.
    double default_ns = 0.0;
    double near_ns = 0.0;

    for (uint32_t run = 0; run < 5; run++) {
      double d = default_func.run(call_count, total_calls);
      double n = near_func.run(call_count, total_calls);

      default_ns = run == 0 ? d : axl::min(default_ns, d);
      near_ns = run == 0 ? n : axl::min(near_ns, n);
    }

    printf("| %10zu | %5zu kB | %7.3f ns/c | %7.3f ns/c | %6.2fx | %6zu/%-7zu |\n",
      call_count,
      (near_func.code_size + 1023u) / 1024u,
      default_ns,
      near_ns,
      default_ns / near_ns,
      near_func.direct_calls,
      call_count);
  }

  printf("+------------+----------+--------------+--------------+---------+----------------+\n");
  return 0;
}

#else

int main(int argc, char* argv[]) {
  CmdLine cmd_line(argc, argv);
  print_app_info(cmd_line.value_as_uint("--count", 20000000));

  printf("This benchmark is only available on X86_64.\n");
  return 0;
}

#endif
