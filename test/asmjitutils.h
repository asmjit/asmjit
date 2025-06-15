// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJITUTILS_H_INCLUDED
#define ASMJITUTILS_H_INCLUDED

#include <asmjit/core.h>

namespace {

[[maybe_unused]]
static const char* asmjitBuildType() noexcept {
#if defined(ASMJIT_BUILD_DEBUG)
  static const char build_type[] = "Debug";
#else
  static const char build_type[] = "Release";
#endif
  return build_type;
}

[[maybe_unused]]
static const char* asmjitArchAsString(asmjit::Arch arch) noexcept {
  switch (arch) {
    case asmjit::Arch::kX86       : return "X86";
    case asmjit::Arch::kX64       : return "X64";

    case asmjit::Arch::kRISCV32   : return "RISCV32";
    case asmjit::Arch::kRISCV64   : return "RISCV64";

    case asmjit::Arch::kARM       : return "ARM";
    case asmjit::Arch::kAArch64   : return "AArch64";
    case asmjit::Arch::kThumb     : return "Thumb";

    case asmjit::Arch::kMIPS32_LE : return "MIPS_LE";
    case asmjit::Arch::kMIPS64_LE : return "MIPS64_LE";

    case asmjit::Arch::kARM_BE    : return "ARM_BE";
    case asmjit::Arch::kThumb_BE  : return "Thumb_BE";
    case asmjit::Arch::kAArch64_BE: return "AArch64_BE";

    case asmjit::Arch::kMIPS32_BE : return "MIPS_BE";
    case asmjit::Arch::kMIPS64_BE : return "MIPS64_BE";

    default:
      return "<Unknown>";
  }
}

[[maybe_unused]]
static void printIndented(const char* str, size_t indent) noexcept {
  const char* start = str;
  while (*str) {
    if (*str == '\n') {
      size_t size = (size_t)(str - start);
      printf("%*s%.*s\n", size ? int(indent) : 0, "", int(size), start);
      start = str + 1;
    }
    str++;
  }

  size_t size = (size_t)(str - start);
  if (size)
    printf("%*s%.*s\n", int(indent), "", int(size), start);
}

[[maybe_unused]]
static void printCpuInfo() noexcept {
  const asmjit::CpuInfo& cpu = asmjit::CpuInfo::host();

  // CPU Information
  // ---------------

  printf("CPU Info:\n");
  printf("  Vendor                  : %s\n", cpu.vendor());
  printf("  Brand                   : %s\n", cpu.brand());
  printf("  Model ID                : 0x%08X (%u)\n", cpu.modelId(), cpu.modelId());
  printf("  Brand ID                : 0x%08X (%u)\n", cpu.brandId(), cpu.brandId());
  printf("  Family ID               : 0x%08X (%u)\n", cpu.familyId(), cpu.familyId());
  printf("  Stepping                : %u\n", cpu.stepping());
  printf("  Processor Type          : %u\n", cpu.processorType());
  printf("  Max logical Processors  : %u\n", cpu.maxLogicalProcessors());
  printf("  Cache-Line Size         : %u\n", cpu.cacheLineSize());
  printf("  HW-Thread Count         : %u\n", cpu.hwThreadCount());
  printf("\n");

  // CPU Features
  // ------------

#ifndef ASMJIT_NO_LOGGING
  printf("CPU Features:\n");
  asmjit::CpuFeatures::Iterator it(cpu.features().iterator());
  while (it.hasNext()) {
    uint32_t featureId = uint32_t(it.next());
    asmjit::StringTmp<64> featureString;
    asmjit::Formatter::formatFeature(featureString, cpu.arch(), featureId);
    printf("  %s\n", featureString.data());
  };
  printf("\n");
#endif // !ASMJIT_NO_LOGGING
}

[[maybe_unused]]
static void printBuildOptions() {
  auto stringifyBuildDefinition = [](bool b) { return b ? "defined" : "(not defined)"; };

#if defined(ASMJIT_NO_X86)
  constexpr bool no_x86 = true;
#else
  constexpr bool no_x86 = false;
#endif

#if defined(ASMJIT_NO_AARCH64)
  constexpr bool no_aarch64 = true;
#else
  constexpr bool no_aarch64 = false;
#endif

#if defined(ASMJIT_NO_FOREIGN)
  constexpr bool no_foreign = true;
#else
  constexpr bool no_foreign = false;
#endif

#if defined(ASMJIT_NO_DEPRECATED)
  constexpr bool no_deprecated = true;
#else
  constexpr bool no_deprecated = false;
#endif

#if defined(ASMJIT_NO_ABI_NAMESPACE)
  constexpr bool no_abi_namespace = true;
#else
  constexpr bool no_abi_namespace = false;
#endif

#if defined(ASMJIT_NO_SHM_OPEN)
  constexpr bool no_shm_open = true;
#else
  constexpr bool no_shm_open = false;
#endif

#if defined(ASMJIT_NO_JIT)
  constexpr bool no_jit = true;
#else
  constexpr bool no_jit = false;
#endif

#if defined(ASMJIT_NO_TEXT)
  constexpr bool no_text = true;
#else
  constexpr bool no_text = false;
#endif

#if defined(ASMJIT_NO_LOGGING)
  constexpr bool no_logging = true;
#else
  constexpr bool no_logging = false;
#endif

#if defined(ASMJIT_NO_VALIDATION)
  constexpr bool no_validation = true;
#else
  constexpr bool no_validation = false;
#endif

#if defined(ASMJIT_NO_INTROSPECTION)
  constexpr bool no_introspection = true;
#else
  constexpr bool no_introspection = false;
#endif

#if defined(ASMJIT_NO_BUILDER)
  constexpr bool no_builder = true;
#else
  constexpr bool no_builder = false;
#endif

#if defined(ASMJIT_NO_COMPILER)
  constexpr bool no_compiler = true;
#else
  constexpr bool no_compiler = false;
#endif

  printf("Build Options:\n");
  printf("  BUILD_TYPE             : %s\n", asmjitBuildType());
  printf("  ASMJIT_NO_DEPRECATED   : %s\n", stringifyBuildDefinition(no_deprecated));
  printf("  ASMJIT_NO_ABI_NAMESPACE: %s\n", stringifyBuildDefinition(no_abi_namespace));
  printf("\n");

  printf("Build Backends:\n");
  printf("  ASMJIT_NO_X86          : %s\n", stringifyBuildDefinition(no_x86));
  printf("  ASMJIT_NO_AARCH64      : %s\n", stringifyBuildDefinition(no_aarch64));
  printf("  ASMJIT_NO_FOREIGN      : %s\n", stringifyBuildDefinition(no_foreign));
  printf("\n");

  printf("Build Features:\n");
  printf("  ASMJIT_NO_SHM_OPEN     : %s\n", stringifyBuildDefinition(no_shm_open));
  printf("  ASMJIT_NO_JIT          : %s\n", stringifyBuildDefinition(no_jit));
  printf("  ASMJIT_NO_TEXT         : %s\n", stringifyBuildDefinition(no_text));
  printf("  ASMJIT_NO_LOGGING      : %s\n", stringifyBuildDefinition(no_logging));
  printf("  ASMJIT_NO_VALIDATION   : %s\n", stringifyBuildDefinition(no_validation));
  printf("  ASMJIT_NO_INTROSPECTION: %s\n", stringifyBuildDefinition(no_introspection));
  printf("  ASMJIT_NO_BUILDER      : %s\n", stringifyBuildDefinition(no_builder));
  printf("  ASMJIT_NO_COMPILER     : %s\n", stringifyBuildDefinition(no_compiler));
  printf("\n");
}

} // {anonymous}

#endif // ASMJITUTILS_H_INCLUDED
