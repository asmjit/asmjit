// This file is part of AsmJit project <https://asmjit.com>
//
// See asmjit.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core.h>

#if !defined(ASMJIT_NO_X86)
  #include <asmjit/x86.h>
#endif // !ASMJIT_NO_X86

#if !defined(ASMJIT_NO_AARCH64)
  #include <asmjit/a64.h>
#endif // !ASMJIT_NO_AARCH64

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <memory>
#include <vector>

#include "asmjitutils.h"

#if !defined(ASMJIT_NO_COMPILER)
  #include "cmdline.h"
  #include "performancetimer.h"
  #include "asmjit_test_compiler.h"
  #include "asmjit_test_random.h"
#endif

using namespace asmjit;

static void printAppInfo() {
  printf("AsmJit Benchmark RegAlloc v%u.%u.%u [Arch=%s] [Mode=%s]\n\n",
    unsigned((ASMJIT_LIBRARY_VERSION >> 16)       ),
    unsigned((ASMJIT_LIBRARY_VERSION >>  8) & 0xFF),
    unsigned((ASMJIT_LIBRARY_VERSION      ) & 0xFF),
    asmjitArchAsString(Arch::kHost),
    asmjitBuildType()
  );
}

#if !defined(ASMJIT_NO_COMPILER)

class BenchRegAllocApp {
public:
  const char* _arch = nullptr;
  bool _helpOnly = false;
  bool _verbose = false;
  uint32_t _maximumComplexity = 65536;

  BenchRegAllocApp() noexcept
    : _arch("all") {}
  ~BenchRegAllocApp() noexcept {}

  template<class T>
  inline void addT() { T::add(*this); }

  int handleArgs(int argc, const char* const* argv);
  void showInfo();

  bool shouldRunArch(Arch arch) const noexcept;
  void emitCode(BaseCompiler* cc, uint32_t complexity, uint32_t regCount);

#if !defined(ASMJIT_NO_X86)
  void emitCode_x86(x86::Compiler* cc, uint32_t complexity, uint32_t regCount);
#endif // !ASMJIT_NO_X86

#if !defined(ASMJIT_NO_AARCH64)
  void emitCode_a64(a64::Compiler* cc, uint32_t complexity, uint32_t regCount);
#endif // !ASMJIT_NO_AARCH64

  int run();
  bool runArch(Arch arch);
};

int BenchRegAllocApp::handleArgs(int argc, const char* const* argv) {
  CmdLine cmd(argc, argv);
  _arch = cmd.valueOf("--arch", "all");
  _maximumComplexity = cmd.valueAsUInt("--complexity", _maximumComplexity);

  if (cmd.hasArg("--help")) _helpOnly = true;
  if (cmd.hasArg("--verbose")) _verbose = true;

  return 0;
}

void BenchRegAllocApp::showInfo() {
  printAppInfo();

  printf("Usage:\n");
  printf("  asmjit_bench_regalloc [arguments]\n");
  printf("\n");

  printf("Arguments:\n");
  printf("  --help           Show usage only\n");
  printf("  --arch=<NAME>    Select architecture to run ('all' by default)\n");
  printf("  --verbose        Verbose output\n");
  printf("  --complexity=<n> Maximum complexity to test (%u)\n", _maximumComplexity);
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
}

bool BenchRegAllocApp::shouldRunArch(Arch arch) const noexcept {
  if (strcmp(_arch, "all") == 0) {
    return true;
  }

  if (strcmp(_arch, "x86") == 0 && arch == Arch::kX86) {
    return true;
  }

  if (strcmp(_arch, "x64") == 0 && arch == Arch::kX64) {
    return true;
  }

  if (strcmp(_arch, "aarch64") == 0 && arch == Arch::kAArch64) {
    return true;
  }

  return false;
}

void BenchRegAllocApp::emitCode(BaseCompiler* cc, uint32_t complexity, uint32_t regCount) {
#if !defined(ASMJIT_NO_X86)
  if (cc->arch() == Arch::kX86 || cc->arch() == Arch::kX64) {
    emitCode_x86(cc->as<x86::Compiler>(), complexity, regCount);
  }
#endif

#if !defined(ASMJIT_NO_AARCH64)
  if (cc->arch() == Arch::kAArch64) {
    emitCode_a64(cc->as<a64::Compiler>(), complexity, regCount);
  }
#endif
}

#if !defined(ASMJIT_NO_X86)
void BenchRegAllocApp::emitCode_x86(x86::Compiler* cc, uint32_t complexity, uint32_t regCount) {
  constexpr size_t kLocalRegCount = 3;

  TestUtils::Random rnd(0x1234);
  size_t localOpCount = 15;

  std::vector<Label> labels;
  std::vector<uint32_t> used_labels;
  std::vector<x86::Vec> vRegs;

  x86::Gp arg_ptr = cc->newIntPtr("arg_ptr");
  x86::Gp counter = cc->newIntPtr("counter");

  for (size_t i = 0; i < complexity; i++) {
    labels.push_back(cc->newLabel());
    used_labels.push_back(0u);
  }

  for (size_t i = 0; i < regCount; i++) {
    vRegs.push_back(cc->newXmmSd("v%u", unsigned(i)));
  }

  FuncNode* func = cc->addFunc(FuncSignature::build<void, size_t, void*>());
  func->addAttributes(FuncAttributes::kX86_AVXEnabled);
  func->setArg(0, counter);
  func->setArg(1, arg_ptr);

  for (size_t i = 0; i < regCount; i++) {
    cc->vmovsd(vRegs[i], x86::ptr_64(arg_ptr, int32_t(i * 8)));
  }

  auto next_label = [&]() {
    uint32_t id = rnd.nextUInt32() % complexity;
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
      locals[j] = cc->newXmmSd("local%u", unsigned(j));
    }

    size_t localOpThreshold = localOpCount - kLocalRegCount;

    for (size_t j = 0; j < 15; j++) {
      uint32_t op = rnd.nextUInt32() % 6u;
      uint32_t id1 = rnd.nextUInt32() % regCount;
      uint32_t id2 = rnd.nextUInt32() % regCount;

      x86::Vec v0 = vRegs[id1];
      x86::Vec v1 = vRegs[id1];
      x86::Vec v2 = vRegs[id2];

      if (j < kLocalRegCount) {
        v0 = locals[j];
      }

      if (j >= localOpThreshold) {
        v2 = locals[j - localOpThreshold];
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

  for (size_t i = 0; i < regCount; i++) {
    cc->vmovsd(x86::ptr_64(arg_ptr, int32_t(i * 8)), vRegs[i]);
  }

  cc->endFunc();
}
#endif // !ASMJIT_NO_X86

#if !defined(ASMJIT_NO_AARCH64)
void BenchRegAllocApp::emitCode_a64(a64::Compiler* cc, uint32_t complexity, uint32_t regCount) {
  TestUtils::Random rnd(0x1234);

  constexpr size_t kLocalRegCount = 3;
  size_t localOpCount = 15;

  std::vector<Label> labels;
  std::vector<uint32_t> used_labels;
  std::vector<a64::Vec> vRegs;

  a64::Gp arg_ptr = cc->newIntPtr("arg_ptr");
  a64::Gp counter = cc->newIntPtr("counter");

  for (size_t i = 0; i < complexity; i++) {
    labels.push_back(cc->newLabel());
    used_labels.push_back(0u);
  }

  for (size_t i = 0; i < regCount; i++) {
    vRegs.push_back(cc->newVecD("v%u", unsigned(i)));
  }

  FuncNode* func = cc->addFunc(FuncSignature::build<void, size_t, void*>());
  func->addAttributes(FuncAttributes::kX86_AVXEnabled);
  func->setArg(0, counter);
  func->setArg(1, arg_ptr);

  for (size_t i = 0; i < regCount; i++) {
    cc->ldr(vRegs[i].d(), a64::ptr(arg_ptr, int32_t(i * 8) & 1023));
  }

  auto next_label = [&]() {
    uint32_t id = rnd.nextUInt32() % complexity;
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
      locals[j] = cc->newVecD("local%u", unsigned(j));
    }

    size_t localOpThreshold = localOpCount - kLocalRegCount;

    for (size_t j = 0; j < 15; j++) {
      uint32_t op = rnd.nextUInt32() % 6;
      uint32_t id1 = rnd.nextUInt32() % regCount;
      uint32_t id2 = rnd.nextUInt32() % regCount;

      a64::Vec v0 = vRegs[id1];
      a64::Vec v1 = vRegs[id1];
      a64::Vec v2 = vRegs[id2];

      if (j < kLocalRegCount) {
        v0 = locals[j];
      }

      if (j >= localOpThreshold) {
        v2 = locals[j - localOpThreshold];
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

  for (size_t i = 0; i < regCount; i++) {
    cc->str(vRegs[i].d(), a64::ptr(arg_ptr, int32_t(i * 8) & 1023));
  }

  cc->endFunc();
}
#endif // !ASMJIT_NO_AARCH64

int BenchRegAllocApp::run() {
  if (shouldRunArch(Arch::kX64) && !runArch(Arch::kX64)) {
    return 1;
  }

  if (shouldRunArch(Arch::kAArch64) && !runArch(Arch::kAArch64)) {
    return 1;
  }

  return 0;
}

bool BenchRegAllocApp::runArch(Arch arch) {
  Environment customEnv;
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
      return false;
  }

  CodeHolder code;

  customEnv.init(arch);
  code.init(customEnv, features);

  std::unique_ptr<BaseCompiler> cc;

#ifndef ASMJIT_NO_X86
  if (code.arch() == Arch::kX86 || code.arch() == Arch::kX64) {
    cc = std::unique_ptr<x86::Compiler>(new x86::Compiler());
  }
#endif // !ASMJIT_NO_X86

#ifndef ASMJIT_NO_AARCH64
  if (code.arch() == Arch::kAArch64) {
    cc = std::unique_ptr<a64::Compiler>(new a64::Compiler());
  }
#endif // !ASMJIT_NO_AARCH64

  if (!cc)
    return false;

  PerformanceTimer emitTimer;
  PerformanceTimer finalizeTimer;

  uint32_t regCount = 35;

  code.reinit();
  code.attach(cc.get());

  // Dry run to not benchmark allocs on the first run.
  emitCode(cc.get(), 0, regCount);
  cc->finalize();
  code.reinit();

  printf("Arch   | Complexity | Labels | RegCount |  CodeSize | EmitTime [ms]| RA Time [ms]\n");
  printf("-------+------------+--------+----------+-----------+--------------+-------------\n");

  for (uint32_t complexity = 1u; complexity <= _maximumComplexity; complexity *= 2u) {
    emitTimer.start();
    emitCode(cc.get(), complexity + 1, regCount);
    emitTimer.stop();

#if !defined(ASMJIT_NO_LOGGING)
    if (_verbose) {
      String sb;
      FormatOptions fmtOptions;
      Formatter::formatNodeList(sb, fmtOptions, cc.get());
      printf("[Complexity: %u Assembly]\n", complexity);
      printIndented(sb.data(), 4);
    }
#endif // ASMJIT_NO_LOGGING

    finalizeTimer.start();
    Error err = cc->finalize();
    finalizeTimer.stop();

    code.flatten();

    double emitTime = emitTimer.duration();
    double finalizeTime = finalizeTimer.duration();
    size_t codeSize = code.codeSize();
    size_t labelCount = code.labelCount();
    size_t vRegCount = cc->virtRegs().size();

    printf("%-7s| %10u | %6zu | %8zu | %9zu | %12.3f | %12.3f",
           asmjitArchAsString(arch), complexity, labelCount, vRegCount, codeSize, emitTime, finalizeTime);

    if (err) {
      printf(" (err: %s)", DebugUtils::errorAsString(err));
    }

    printf("\n");

    code.reinit();
  }

  printf("\n");

  return true;
}

int main(int argc, char* argv[]) {
  BenchRegAllocApp app;

  app.handleArgs(argc, argv);
  app.showInfo();

  if (app._helpOnly)
    return 0;

  return app.run();
}

#else

int main() {
  printAppInfo();
  printf("!! This Benchmark is disabled: <ASMJIT_NO_JIT> or unsuitable target architecture !!\n");
  return 0;
}

#endif // !ASMJIT_NO_COMPILER
