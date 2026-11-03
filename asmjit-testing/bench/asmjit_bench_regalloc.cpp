// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core.h>

#if !defined(ASMJIT_NO_X86)
  #include <asmjit/x86.h>
#endif // !ASMJIT_NO_X86

#if !defined(ASMJIT_NO_AARCH64)
  #include <asmjit/a64.h>
#endif // !ASMJIT_NO_AARCH64

#include <asmjit-testing/commons/asmjit_utils.h>

#if !defined(ASMJIT_NO_COMPILER)
  #include <asmjit-testing/commons/random.h>
  #include <asmjit-testing/commons/zero_bench.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <memory>
#include <vector>

using namespace asmjit;

static void print_app_info() {
  printf("AsmJit Benchmark RegAlloc v%u.%u.%u [Arch=%s] [Mode=%s]\n\n",
    unsigned((ASMJIT_LIBRARY_VERSION >> 16)       ),
    unsigned((ASMJIT_LIBRARY_VERSION >>  8) & 0xFF),
    unsigned((ASMJIT_LIBRARY_VERSION      ) & 0xFF),
    asmjit_arch_as_string(Arch::kHost),
    asmjit_build_type()
  );
}

#if !defined(ASMJIT_NO_COMPILER)

enum ValueId : uint8_t {
  kLabelCount,
  kVirtRegCount,
  kCodeSize,
  kCodeHolderMemory,
  kCompilerMemory,
  kPassMemory,
  kEmitTime,
  kRATime
};

class RegAllocBenchmark : public ZeroBench::Benchmark {
public:
  CodeHolder code;
  std::unique_ptr<BaseCompiler> compiler;
  Arch arch;
  uint32_t complexity_param;
  bool verbose;

#if !defined(ASMJIT_NO_LOGGING)
  StringLogger logger;
#endif // !ASMJIT_NO_LOGGING

  static constexpr size_t kLocalRegCount = 3;
  static constexpr size_t kLocalOpCount = 15;

  RegAllocBenchmark(Arch arch, uint32_t complexity_param, bool verbose) noexcept
    : arch(arch),
      complexity_param(complexity_param),
      verbose(verbose) {
    set_label(0, asmjit_arch_as_string(arch));
    set_label(1, ZeroBench::String::format("%u", complexity_param));
  }

  ~RegAllocBenchmark() noexcept override {}

  void prepare(ZeroBench::Context& ctx) override;
  void run(ZeroBench::Context& ctx) override;
  void teardown() override;

  void emit_code(BaseCompiler* cc, uint32_t complexity, uint32_t reg_count);

#if !defined(ASMJIT_NO_X86)
  void emit_code_x86(x86::Compiler* cc, uint32_t complexity, uint32_t reg_count);
#endif // !ASMJIT_NO_X86

#if !defined(ASMJIT_NO_AARCH64)
  void emit_code_aarch64(a64::Compiler* cc, uint32_t complexity, uint32_t reg_count);
#endif // !ASMJIT_NO_AARCH64
};

void RegAllocBenchmark::prepare(ZeroBench::Context& ctx) {
  Environment custom_env;
  CpuFeatures features;

  switch (arch) {
    case Arch::kX86:
    case Arch::kX64:
      features.add(CpuFeatures::X86::kADX,
                   CpuFeatures::X86::kAVX,
                   CpuFeatures::X86::kAVX2,
                   CpuFeatures::X86::kBMI,
                   CpuFeatures::X86::kBMI2,
                   CpuFeatures::X86::kCMOV,
                   CpuFeatures::X86::kF16C,
                   CpuFeatures::X86::kFMA,
                   CpuFeatures::X86::kFPU,
                   CpuFeatures::X86::kI486,
                   CpuFeatures::X86::kLZCNT,
                   CpuFeatures::X86::kMMX,
                   CpuFeatures::X86::kMMX2,
                   CpuFeatures::X86::kPOPCNT,
                   CpuFeatures::X86::kSSE,
                   CpuFeatures::X86::kSSE2,
                   CpuFeatures::X86::kSSE3,
                   CpuFeatures::X86::kSSSE3,
                   CpuFeatures::X86::kSSE4_1,
                   CpuFeatures::X86::kSSE4_2,
                   CpuFeatures::X86::kAVX,
                   CpuFeatures::X86::kAVX2);
      break;

    case Arch::kAArch64:
      features.add(CpuFeatures::ARM::kAES,
                   CpuFeatures::ARM::kASIMD,
                   CpuFeatures::ARM::kIDIVA,
                   CpuFeatures::ARM::kIDIVT,
                   CpuFeatures::ARM::kPMULL);
      break;

    default:
      ctx.make_failed("Unsupported architecture");
      return;
  }

  custom_env.init(arch);
  Error err = code.init(custom_env, features);
  if (err != Error::kOk) {
    ctx.make_failed(stringify_error(err));
    return;
  }

#ifndef ASMJIT_NO_X86
  if (code.arch() == Arch::kX86 || code.arch() == Arch::kX64) {
    compiler = std::make_unique<x86::Compiler>();
  }
#endif // !ASMJIT_NO_X86

#ifndef ASMJIT_NO_AARCH64
  if (code.arch() == Arch::kAArch64) {
    compiler = std::make_unique<a64::Compiler>();
  }
#endif // !ASMJIT_NO_AARCH64

  if (!compiler) {
    ctx.make_failed("Architecture backend is disabled");
    return;
  }

  err = code.attach(compiler.get());
  if (err != Error::kOk) {
    ctx.make_failed(stringify_error(err));
    return;
  }

#if !defined(ASMJIT_NO_LOGGING)
  if (verbose) {
    code.set_logger(&logger);
    compiler->add_diagnostic_options(DiagnosticOptions::kRAAnnotate | DiagnosticOptions::kRADebugAll);
  }
#endif // !ASMJIT_NO_LOGGING
}

void RegAllocBenchmark::run(ZeroBench::Context& ctx) {
  Error err = code.reinit();
  if (err != Error::kOk) {
    ctx.make_failed(stringify_error(err));
    return;
  }

#if !defined(ASMJIT_NO_LOGGING)
  logger.clear();
#endif // !ASMJIT_NO_LOGGING

  ZeroBench::Timer timer;
  ctx.start_now();

  timer.start();
  emit_code(compiler.get(), complexity_param + 1u, 35u);
  uint64_t emit_time = timer.elapsed();

  timer.start();
  err = compiler->finalize();
  uint64_t finalize_time = timer.elapsed();

  ctx.end_now();

  if (err == Error::kOk) {
    err = code.flatten();
  }
  if (err != Error::kOk) {
    ctx.make_failed(stringify_error(err));
    return;
  }

  ctx.set_value(kLabelCount, code.label_count());
  ctx.set_value(kVirtRegCount, compiler->virt_regs().size());
  ctx.set_value(kCodeSize, code.code_size());
  ctx.set_value(kCodeHolderMemory, code._arena.usage().reserved_size());
  ctx.set_value(kCompilerMemory, compiler->_builder_arena.usage().reserved_size());
  ctx.set_value(kPassMemory, compiler->_pass_arena.usage().reserved_size());
  ctx.set_value(kEmitTime, emit_time);
  ctx.set_value(kRATime, finalize_time);
}

void RegAllocBenchmark::teardown() {
#if !defined(ASMJIT_NO_LOGGING)
  if (verbose) {
    printf("%s\n", logger.data());
    logger.clear();
  }
#endif // !ASMJIT_NO_LOGGING

  code.reset();
  compiler.reset();
}

void RegAllocBenchmark::emit_code(BaseCompiler* cc, uint32_t complexity, uint32_t reg_count) {
#if !defined(ASMJIT_NO_X86)
  if (cc->arch() == Arch::kX86 || cc->arch() == Arch::kX64) {
    emit_code_x86(cc->as<x86::Compiler>(), complexity, reg_count);
  }
#endif

#if !defined(ASMJIT_NO_AARCH64)
  if (cc->arch() == Arch::kAArch64) {
    emit_code_aarch64(cc->as<a64::Compiler>(), complexity, reg_count);
  }
#endif
}

#if !defined(ASMJIT_NO_X86)
void RegAllocBenchmark::emit_code_x86(x86::Compiler* cc, uint32_t complexity, uint32_t reg_count) {
  TestUtils::Random rnd(0x1234);

  std::vector<Label> labels;
  std::vector<uint32_t> used_labels;
  std::vector<x86::Vec> virt_regs;

  x86::Gp arg_ptr = cc->new_gp_ptr("arg_ptr");
  x86::Gp counter = cc->new_gp_ptr("counter");

  for (size_t i = 0; i < complexity; i++) {
    labels.push_back(cc->new_label());
    used_labels.push_back(0u);
  }

  for (size_t i = 0; i < reg_count; i++) {
    virt_regs.push_back(cc->new_xmm_sd("v%u", unsigned(i)));
  }

  FuncNode* func = cc->add_func(FuncSignature::build<void, size_t, void*>());
  func->add_attributes(FuncAttributes::kX86_AVXEnabled);
  func->set_arg(0, counter);
  func->set_arg(1, arg_ptr);

  for (size_t i = 0; i < reg_count; i++) {
    cc->vmovsd(virt_regs[i], x86::ptr_64(arg_ptr, int32_t(i * 8)));
  }

  auto next_label = [&]() {
    uint32_t id = rnd.next_uint32() % complexity;
    if (used_labels[id] > 1) {
      id = 0;
      do {
        if (++id >= complexity) {
          id = 0;
        }
      } while (used_labels[id] != 0);
    }

    used_labels[id]++;
    return labels[id];
  };

  for (size_t i = 0; i < labels.size(); i++) {
    cc->bind(labels[i]);

    x86::Vec locals[kLocalRegCount];
    for (size_t j = 0; j < kLocalRegCount; j++) {
      locals[j] = cc->new_xmm_sd("local%u", unsigned(j));
    }

    size_t local_op_threshold = kLocalOpCount - kLocalRegCount;

    for (size_t j = 0; j < 15; j++) {
      uint32_t op = rnd.next_uint32() % 6u;
      uint32_t id1 = rnd.next_uint32() % reg_count;
      uint32_t id2 = rnd.next_uint32() % reg_count;

      x86::Vec v0 = virt_regs[id1];
      x86::Vec v1 = virt_regs[id1];
      x86::Vec v2 = virt_regs[id2];

      if (j < kLocalRegCount) {
        v0 = locals[j];
      }

      if (j >= local_op_threshold) {
        v2 = locals[j - local_op_threshold];
      }

      switch (op) {
        case 0: cc->vaddsd(v0, v1, v2); break;
        case 1: cc->vsubsd(v0, v1, v2); break;
        case 2: cc->vmulsd(v0, v1, v2); break;
        case 3: cc->vdivsd(v0, v1, v2); break;
        case 4: cc->vminsd(v0, v1, v2); break;
        case 5: cc->vmaxsd(v0, v1, v2); break;
      }
    }

    cc->sub(counter, 1);
    cc->jns(next_label());
  }

  for (size_t i = 0; i < reg_count; i++) {
    cc->vmovsd(x86::ptr_64(arg_ptr, int32_t(i * 8)), virt_regs[i]);
  }

  cc->end_func();
}
#endif // !ASMJIT_NO_X86

#if !defined(ASMJIT_NO_AARCH64)
void RegAllocBenchmark::emit_code_aarch64(a64::Compiler* cc, uint32_t complexity, uint32_t reg_count) {
  TestUtils::Random rnd(0x1234);

  std::vector<Label> labels;
  std::vector<uint32_t> used_labels;
  std::vector<a64::Vec> virt_regs;

  a64::Gp arg_ptr = cc->new_gp_ptr("arg_ptr");
  a64::Gp counter = cc->new_gp_ptr("counter");

  for (size_t i = 0; i < complexity; i++) {
    labels.push_back(cc->new_label());
    used_labels.push_back(0u);
  }

  for (size_t i = 0; i < reg_count; i++) {
    virt_regs.push_back(cc->new_vec_d("v%u", unsigned(i)));
  }

  FuncNode* func = cc->add_func(FuncSignature::build<void, size_t, void*>());
  func->add_attributes(FuncAttributes::kX86_AVXEnabled);
  func->set_arg(0, counter);
  func->set_arg(1, arg_ptr);

  for (size_t i = 0; i < reg_count; i++) {
    cc->ldr(virt_regs[i].d(), a64::ptr(arg_ptr, int32_t(i * 8) & 1023));
  }

  auto next_label = [&]() {
    uint32_t id = rnd.next_uint32() % complexity;
    if (used_labels[id] > 1) {
      id = 0;
      do {
        if (++id >= complexity) {
          id = 0;
        }
      } while (used_labels[id] != 0);
    }

    used_labels[id]++;
    return labels[id];
  };

  for (size_t i = 0; i < labels.size(); i++) {
    cc->bind(labels[i]);

    a64::Vec locals[kLocalRegCount];
    for (size_t j = 0; j < kLocalRegCount; j++) {
      locals[j] = cc->new_vec_d("local%u", unsigned(j));
    }

    size_t local_op_threshold = kLocalOpCount - kLocalRegCount;

    for (size_t j = 0; j < 15; j++) {
      uint32_t op = rnd.next_uint32() % 6;
      uint32_t id1 = rnd.next_uint32() % reg_count;
      uint32_t id2 = rnd.next_uint32() % reg_count;

      a64::Vec v0 = virt_regs[id1];
      a64::Vec v1 = virt_regs[id1];
      a64::Vec v2 = virt_regs[id2];

      if (j < kLocalRegCount) {
        v0 = locals[j];
      }

      if (j >= local_op_threshold) {
        v2 = locals[j - local_op_threshold];
      }

      switch (op) {
        case 0: cc->fadd(v0.d(), v1.d(), v2.d()); break;
        case 1: cc->fsub(v0.d(), v1.d(), v2.d()); break;
        case 2: cc->fmul(v0.d(), v1.d(), v2.d()); break;
        case 3: cc->fdiv(v0.d(), v1.d(), v2.d()); break;
        case 4: cc->fmin(v0.d(), v1.d(), v2.d()); break;
        case 5: cc->fmax(v0.d(), v1.d(), v2.d()); break;
      }
    }

    cc->subs(counter, counter, 1);
    cc->b_hi(next_label());
  }

  for (size_t i = 0; i < reg_count; i++) {
    cc->str(virt_regs[i].d(), a64::ptr(arg_ptr, int32_t(i * 8) & 1023));
  }

  cc->end_func();
}
#endif // !ASMJIT_NO_AARCH64

static void print_usage(uint32_t maximum_complexity) {
  printf("Usage:\n");
  printf("  asmjit_bench_regalloc [arguments]\n");
  printf("\n");

  printf("Arguments:\n");
  printf("  --help           Show usage only\n");
  printf("  --arch=<NAME>    Select architecture to run ('all' by default)\n");
  printf("  --verbose        Log the last iteration of each benchmark\n");
  printf("  --complexity=<n> Maximum complexity to test (%u)\n", maximum_complexity);
  printf("  --iter-count=<n> Number of measured iterations (automatic by default)\n");
  printf("  --max-iter-count=<n> Limit the automatic iteration count\n");
  printf("  --warm-up-iters=<n> Number of warm-up iterations\n");
  printf("  --warm-up-nanos=<n> Warm-up duration in nanoseconds (1000000 by default)\n");
  printf("  --run=<pattern>  Filter benchmarks by architecture name\n");
  printf("  --no-color       Disable color output\n");
  printf("\n");

  printf("Architectures:\n");
#if !defined(ASMJIT_NO_X86)
  printf("  --arch=x86       32-bit X86 architecture (X86)\n");
  printf("  --arch=x64       64-bit X86 architecture (X86_64)\n");
#endif
#if !defined(ASMJIT_NO_AARCH64)
  printf("  --arch=aarch64   64-bit ARM architecture (AArch64)\n");
#endif
  printf("\n");
  printf("Memory columns show reserved memory rounded up to KiB.\n");
  printf("Emit Time and Reg. Alloc show median emission and finalization times.\n\n");
}

static bool should_run_arch(const char* selected_arch, Arch arch) noexcept {
  if (strcmp(selected_arch, "all") == 0) {
    return true;
  }

  if (strcmp(selected_arch, "x86") == 0 && arch == Arch::kX86) {
    return true;
  }

  if (strcmp(selected_arch, "x64") == 0 && arch == Arch::kX64) {
    return true;
  }

  if (strcmp(selected_arch, "aarch64") == 0 && arch == Arch::kAArch64) {
    return true;
  }

  return false;
}

int main(int argc, char* argv[]) {
  ZeroBench::Runner runner(argc, argv);

  bool verbose = runner.cmd_line().has_arg("--verbose");
  const char* selected_arch = runner.cmd_line().get_arg("--arch", "all");
  uint32_t maximum_complexity = runner.cmd_line().value_as_uint("--complexity", 65536u);

  print_app_info();

  if (runner.cmd_line().has_arg("--help")) {
    print_usage(maximum_complexity);
    return 0;
  }

  runner.set_label(0, "Architecture");
  runner.set_label(1, "Complexity");

  runner.add_metric("N. Labels", {
    .color = ZeroBench::kDefaultMetricColor,
    .value_kind = ZeroBench::ValueKind::kCount,
    .value_index = kLabelCount,
    .aggregate = ZeroBench::Aggregate::kStatic,
    .scale = ZeroBench::Scale::kNone
  });
  runner.add_metric("N. VRegs", {
    .color = ZeroBench::kDefaultMetricColor,
    .value_kind = ZeroBench::ValueKind::kCount,
    .value_index = kVirtRegCount,
    .aggregate = ZeroBench::Aggregate::kStatic,
    .scale = ZeroBench::Scale::kNone
  });
  runner.add_metric("Code Size", {
    .color = ZeroBench::Color::kBrightMagenta,
    .value_kind = ZeroBench::ValueKind::kBytes,
    .value_index = kCodeSize,
    .aggregate = ZeroBench::Aggregate::kStatic,
    .scale = ZeroBench::Scale::kDefault
  });
  runner.add_metric("Code H. Mem", {
    .color = ZeroBench::Color::kBrightCyan,
    .value_kind = ZeroBench::ValueKind::kBytes,
    .value_index = kCodeHolderMemory,
    .aggregate = ZeroBench::Aggregate::kStatic,
    .scale = ZeroBench::Scale::kDefaultBytes
  });
  runner.add_metric("Compiler Mem", {
    .color = ZeroBench::Color::kBrightCyan,
    .value_kind = ZeroBench::ValueKind::kBytes,
    .value_index = kCompilerMemory,
    .aggregate = ZeroBench::Aggregate::kStatic,
    .scale = ZeroBench::Scale::kDefaultBytes
  });
  runner.add_metric("Pass Tmp Mem", {
    .color = ZeroBench::Color::kBrightCyan,
    .value_kind = ZeroBench::ValueKind::kBytes,
    .value_index = kPassMemory,
    .aggregate = ZeroBench::Aggregate::kStatic,
    .scale = ZeroBench::Scale::kDefaultBytes
  });
  runner.add_metric("Emit T.Med", {
    .color = ZeroBench::kOtherDurationColor,
    .value_kind = ZeroBench::ValueKind::kDuration,
    .value_index = kEmitTime,
    .aggregate = ZeroBench::Aggregate::kP50,
    .scale = ZeroBench::Scale::kDefault
  });
  runner.add_metric("Emit T.Max", {
    .color = ZeroBench::kMaxDurationColor,
    .value_kind = ZeroBench::ValueKind::kDuration,
    .value_index = kEmitTime,
    .aggregate = ZeroBench::Aggregate::kMax,
    .scale = ZeroBench::Scale::kDefault
  });
  runner.add_metric("RA T. Med", {
    .color = ZeroBench::kOtherDurationColor,
    .value_kind = ZeroBench::ValueKind::kDuration,
    .value_index = kRATime,
    .aggregate = ZeroBench::Aggregate::kP50,
    .scale = ZeroBench::Scale::kDefault
  });
  runner.add_metric("RA T. Max", {
    .color = ZeroBench::kMaxDurationColor,
    .value_kind = ZeroBench::ValueKind::kDuration,
    .value_index = kRATime,
    .aggregate = ZeroBench::Aggregate::kMax,
    .scale = ZeroBench::Scale::kDefault
  });

  auto run_arch = [&](Arch arch) {
    if (!should_run_arch(selected_arch, arch)) {
      return;
    }

    std::vector<ZeroBench::BenchmarkRef> benchmarks;
    for (uint32_t complexity = 1u; complexity <= maximum_complexity; complexity *= 2u) {
      benchmarks.push_back(ZeroBench::BenchmarkRef::make<RegAllocBenchmark>(arch, complexity, verbose));
      if (complexity > maximum_complexity / 2u) {
        break;
      }
    }

    if (runner.run(benchmarks.data(), benchmarks.size()) != 0) {
      exit(1);
    }
    printf("\n");
  };

#if !defined(ASMJIT_NO_X86)
  run_arch(Arch::kX86);
  run_arch(Arch::kX64);
#endif // !ASMJIT_NO_X86

#if !defined(ASMJIT_NO_AARCH64)
  run_arch(Arch::kAArch64);
#endif // !ASMJIT_NO_AARCH64

  return 0;
}

#else

int main() {
  print_app_info();
  printf("!! This Benchmark is disabled: <ASMJIT_NO_COMPILER> !!\n");
  return 0;
}

#endif // !ASMJIT_NO_COMPILER
