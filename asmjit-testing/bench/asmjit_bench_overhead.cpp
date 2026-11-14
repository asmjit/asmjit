#include <asmjit/host.h>

#include <asmjit-testing/commons/asmjit_utils.h>
#include <asmjit-testing/commons/zero_bench.h>

#include <vector>

using namespace asmjit;

class MyErrorHandler : public ErrorHandler {
public:
  void handle_error(asmjit::Error err, const char* message, asmjit::BaseEmitter* origin) override {
    axl::maybe_unused(err, origin);
    fprintf(stderr, "AsmJit error: %s\n", message);
  }
};

enum class InitStrategy : uint32_t {
  kInitReset,
  kReinit
};

enum class CompileStrategy : uint32_t {
  kNone,
  kEmitOnly,
  kRA,
  kFinalize,
  kAll
};

[[maybe_unused]] [[nodiscard]]
static const char* stringify_emitter_type(EmitterType emitter_type) noexcept {
  switch (emitter_type) {
    case EmitterType::kAssembler: return "Assembler";
    case EmitterType::kBuilder  : return "Builder";
    case EmitterType::kCompiler : return "Compiler";
    default: return "<Unknown>";
  }
}

[[maybe_unused]] [[nodiscard]]
static const char* stringify_init_strategy(InitStrategy init_strategy) noexcept {
  switch (init_strategy) {
    case InitStrategy::kInitReset: return "Init/Reset";
    case InitStrategy::kReinit   : return "Reinit";
    default: return "<Unknown>";
  }
}

[[maybe_unused]] [[nodiscard]]
static const char* stringify_compile_strategy(CompileStrategy compile_strategy) noexcept {
  switch (compile_strategy) {
    case CompileStrategy::kNone    : return "None";
    case CompileStrategy::kEmitOnly: return "Emit";
    case CompileStrategy::kRA      : return "RA";
    case CompileStrategy::kFinalize: return "Finalize";
    case CompileStrategy::kAll     : return "Executable";
    default: return "<Unknown>";
  }
}

struct CodeHolderInitBenchmark : public ZeroBench::Benchmark {
  Environment environment;
  CodeHolder code;
  InitStrategy init_strategy;

  CodeHolderInitBenchmark(InitStrategy init_strategy) noexcept : init_strategy(init_strategy) {
    set_label(0, "CodeHolder");
    set_label(1, stringify_init_strategy(init_strategy));
    set_label(2, "None");
  }

  ~CodeHolderInitBenchmark() noexcept override {}

  void prepare([[maybe_unused]] ZeroBench::Context& ctx) override {
    environment = Environment::host();
    if (init_strategy == InitStrategy::kReinit) {
      code.init(environment);
    }
  }

  void run([[maybe_unused]] ZeroBench::Context& ctx) override {
    if (init_strategy == InitStrategy::kReinit) {
      code.reinit();
    }
    else {
      code.init(environment);
      code.reset();
    }
  }

  void teardown() override {
    code.reset();
  }
};

#if !defined(ASMJIT_NO_JIT)

#if ASMJIT_TARGET_ARCH_X86 != 0 && !defined(ASMJIT_NO_X86)
template<typename EmitterT>
static ASMJIT_INLINE void emit_raw_func(EmitterT& emitter) {
  emitter.mov(x86::eax, 0);
  emitter.ret();
}

template<typename CompilerT>
static ASMJIT_INLINE void compile_raw_func(CompilerT& cc) {
  x86::Gp r = cc.new_gp32();
  cc.mov(r, 0);
  cc.ret(r);
}
#endif

#if ASMJIT_TARGET_ARCH_ARM == 64 && !defined(ASMJIT_NO_AARCH64)
template<typename EmitterT>
static ASMJIT_INLINE void emit_raw_func(EmitterT& emitter) {
  emitter.mov(a64::w0, 0);
  emitter.ret(a64::x30);
}

template<typename CompilerT>
static ASMJIT_INLINE void compile_raw_func(CompilerT& cc) {
  a64::Gp gp = cc.new_gp32();
  cc.mov(gp, 0);
  cc.ret(gp);
}
#endif

#if defined(ASMJIT_HAS_HOST_BACKEND)
template<typename EmitterClass>
struct EmitterInitBenchmark : public ZeroBench::Benchmark {
  JitRuntime rt;
  CodeHolder code;
  EmitterClass emitter;
  MyErrorHandler eh;
  InitStrategy init_strategy;

  EmitterInitBenchmark(InitStrategy init_strategy) noexcept : init_strategy(init_strategy) {
    set_label(0, stringify_emitter_type(emitter.emitter_type()));
    set_label(1, stringify_init_strategy(init_strategy));
    set_label(2, "None");
  }

  ~EmitterInitBenchmark() noexcept override {}

  inline void init_emitter() {
    code.init(rt.environment());
    code.set_error_handler(&eh);
    code.attach(&emitter);
  }

  void prepare([[maybe_unused]] ZeroBench::Context& ctx) override {
    if (init_strategy == InitStrategy::kReinit) {
      init_emitter();
    }
  }

  void run([[maybe_unused]] ZeroBench::Context& ctx) override {
    if (init_strategy == InitStrategy::kReinit) {
      code.reinit();
    }
    else {
      init_emitter();
      code.reset();
    }
  }

  void teardown() override {
    code.reset();
    rt.reset();
  }
};

template<typename EmitterClass, bool Finalize, bool RT>
struct EmitterFuncBenchmark : public ZeroBench::Benchmark {
  JitRuntime rt;
  CodeHolder code;
  EmitterClass emitter;
  MyErrorHandler eh;
  InitStrategy init_strategy;

  EmitterFuncBenchmark(InitStrategy init_strategy) noexcept : init_strategy(init_strategy) {
    set_label(0, stringify_emitter_type(emitter.emitter_type()));
    set_label(1, stringify_init_strategy(init_strategy));
    set_label(2, RT ? "Executable" : Finalize ? "Finalize" : "Emit");
  }

  ~EmitterFuncBenchmark() noexcept override {}

  inline void init_emitter() {
    code.init(rt.environment());
    code.set_error_handler(&eh);
    code.attach(&emitter);
  }

  void prepare([[maybe_unused]] ZeroBench::Context& ctx) override {
    if (init_strategy == InitStrategy::kReinit) {
      init_emitter();
    }
  }

  void run([[maybe_unused]] ZeroBench::Context& ctx) override {
    if (init_strategy == InitStrategy::kReinit) {
      code.reinit();
    }
    else {
      init_emitter();
    }

    emit_raw_func(emitter);

    if constexpr (Finalize) {
      emitter.finalize();
    }

    if constexpr (RT) {
      using Func = uint32_t(*)(void);
      Func fn;
      rt.add(&fn, &code);
      rt.release(fn);
    }

    if (init_strategy != InitStrategy::kReinit) {
      code.reset();
    }
  }

  void teardown() override {
    code.reset();
    rt.reset();
  }
};

#endif

#if defined(ASMJIT_HAS_HOST_BACKEND) && !defined(ASMJIT_NO_COMPILER)
template<typename CompilerClass>
struct CompilerOverheadBenchmark : public ZeroBench::Benchmark {
  JitRuntime rt;
  CodeHolder code;
  CompilerClass cc;
  MyErrorHandler eh;
  InitStrategy init_strategy;
  CompileStrategy compile_strategy;

  CompilerOverheadBenchmark(InitStrategy init_strategy, CompileStrategy compile_strategy) noexcept
    : init_strategy(init_strategy),
      compile_strategy(compile_strategy) {
    set_label(0, "Compiler");
    set_label(1, stringify_init_strategy(init_strategy));
    set_label(2, stringify_compile_strategy(compile_strategy));
  }

  ~CompilerOverheadBenchmark() noexcept override {}

  inline void init_compiler() {
    code.init(rt.environment());
    code.set_error_handler(&eh);
    code.attach(&cc);
  }

  void prepare([[maybe_unused]] ZeroBench::Context& ctx) override {
    if (init_strategy == InitStrategy::kReinit) {
      init_compiler();
    }
  }

  void run([[maybe_unused]] ZeroBench::Context& ctx) override {
    if (init_strategy == InitStrategy::kInitReset) {
      init_compiler();
    }

    if (compile_strategy >= CompileStrategy::kEmitOnly) {
      (void)cc.add_func(FuncSignature::build<uint32_t>());
      compile_raw_func(cc);
      cc.end_func();

      if (compile_strategy == CompileStrategy::kRA) {
        cc.run_passes();
      }
      else if (compile_strategy >= CompileStrategy::kFinalize) {
        // finalize() runs the compiler passes before serializing the code.
        cc.finalize();

        if (compile_strategy >= CompileStrategy::kAll) {
          using Func = uint32_t(*)(void);
          Func fn;
          rt.add(&fn, &code);
          rt.release(fn);
        }
      }
    }

    if (init_strategy == InitStrategy::kInitReset) {
      code.reset();
    }
    else {
      code.reinit();
    }
  }

  void teardown() override {
    code.reset();
    rt.reset();
  }
};

#endif // ASMJIT_HAS_HOST_BACKEND && !ASMJIT_NO_COMPILER
#endif // !ASMJIT_NO_JIT

int main(int argc, char* argv[]) {
  ZeroBench::Runner runner(argc, argv);

  printf("AsmJit Overhead Benchmark v%u.%u.%u [Arch=%s] [Mode=%s]\n\n",
    unsigned((ASMJIT_LIBRARY_VERSION >> 16)       ),
    unsigned((ASMJIT_LIBRARY_VERSION >>  8) & 0xFF),
    unsigned((ASMJIT_LIBRARY_VERSION      ) & 0xFF),
    asmjit_arch_as_string(Arch::kHost),
    asmjit_build_type()
  );

  if (runner.cmd_line().has_arg("--help")) {
    printf("This benchmark was designed to benchmark the cost of initialization and\n"
           "reset (or reinitialization) of CodeHolder and Emitters; and the cost of\n"
           "moving a minimal assembled function to executable memory. Each output line\n"
           "provides the following columns:\n"
           "\n"
           "  - Benchmark    - benchmark name\n"
           "  - Strategy     - reusability strategy:\n"
           "    - Init/Reset - init/reset calls are used to initialize and reset everything\n"
           "    - Reinit     - reinit is used to reuse CodeHolder and emitters maximum way\n"
           "  - Variation    - variation\n"
           "    - None       - no code generation, no use of emitter except for init/reuse\n"
           "    - Emit       - function was emitted to Assembler or Builder/Compiler nodes\n"
           "    - RA         - function was compiled (registers allocated) (Compiler)\n"
           "    - Finalize   - function was finalized & serialized to Assembler\n"
           "    - Executable - function was added to JitRuntime and then removed from it\n"
           "\n"
           "Essentially the output provides an insight into the cost of reusing\n"
           "CodeHolder and other emitters, and the cost of assembling, finalizing,\n"
           "and moving the assembled code into executable memory by separating each\n"
           "phase.\n\n"
    );
    return 1;
  }

  runner.set_label(1, "Init Strategy");
  runner.set_label(2, "Variation");
  runner.add_time_metrics();

  for (uint32_t i = 0; i < 2; i++) {
    std::vector<ZeroBench::BenchmarkRef> benchmarks;
    InitStrategy init_strategy = static_cast<InitStrategy>(i);

    benchmarks.push_back(ZeroBench::BenchmarkRef::make<CodeHolderInitBenchmark>(init_strategy));

#if !defined(ASMJIT_NO_JIT)

#if defined(ASMJIT_HAS_HOST_BACKEND) && !defined(ASMJIT_NO_ASSEMBLER)
    benchmarks.push_back(ZeroBench::BenchmarkRef::make<EmitterInitBenchmark<host::Assembler>>(init_strategy));
    benchmarks.push_back(ZeroBench::BenchmarkRef::make<EmitterFuncBenchmark<host::Assembler, false, false>>(init_strategy));
    benchmarks.push_back(ZeroBench::BenchmarkRef::make<EmitterFuncBenchmark<host::Assembler, false, true>>(init_strategy));
#endif

#if defined(ASMJIT_HAS_HOST_BACKEND) && !defined(ASMJIT_NO_BUILDER)
    benchmarks.push_back(ZeroBench::BenchmarkRef::make<EmitterInitBenchmark<host::Builder>>(init_strategy));
    benchmarks.push_back(ZeroBench::BenchmarkRef::make<EmitterFuncBenchmark<host::Builder, false, false>>(init_strategy));
    benchmarks.push_back(ZeroBench::BenchmarkRef::make<EmitterFuncBenchmark<host::Builder, true, false>>(init_strategy));
    benchmarks.push_back(ZeroBench::BenchmarkRef::make<EmitterFuncBenchmark<host::Builder, true, true>>(init_strategy));
#endif

#if defined(ASMJIT_HAS_HOST_BACKEND) && !defined(ASMJIT_NO_COMPILER)
    benchmarks.push_back(ZeroBench::BenchmarkRef::make<CompilerOverheadBenchmark<host::Compiler>>(init_strategy, CompileStrategy::kNone));
    benchmarks.push_back(ZeroBench::BenchmarkRef::make<CompilerOverheadBenchmark<host::Compiler>>(init_strategy, CompileStrategy::kEmitOnly));
    benchmarks.push_back(ZeroBench::BenchmarkRef::make<CompilerOverheadBenchmark<host::Compiler>>(init_strategy, CompileStrategy::kRA));
    benchmarks.push_back(ZeroBench::BenchmarkRef::make<CompilerOverheadBenchmark<host::Compiler>>(init_strategy, CompileStrategy::kFinalize));
    benchmarks.push_back(ZeroBench::BenchmarkRef::make<CompilerOverheadBenchmark<host::Compiler>>(init_strategy, CompileStrategy::kAll));
#endif

#endif // !ASMJIT_NO_JIT

    runner.run(benchmarks.data(), benchmarks.size());
    printf("\n");
  }

  return 0;
}
