// This file is part of AsmJit project <https://asmjit.com>
//
// See asmjit.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core.h>

#if !defined(ASMJIT_NO_X86) && ASMJIT_ARCH_X86 != 0
  #include <asmjit/x86.h>
#endif

#if !defined(ASMJIT_NO_AARCH64) && ASMJIT_ARCH_ARM == 64
  #include <asmjit/a64.h>
#endif

#include "asmjitutils.h"

using namespace asmjit;

using VoidFunc = void (ASMJIT_CDECL*)(void);

#if !defined(ASMJIT_NO_JIT)

#if !defined(ASMJIT_NO_X86) && ASMJIT_ARCH_X86 != 0
#define TEST_ENVIRONMENT_HAS_JIT

static void emitVoidFunction(CodeHolder& code) noexcept {
  x86::Assembler a(&code);
  a.ret();
}
#endif

#if !defined(ASMJIT_NO_AARCH64) && ASMJIT_ARCH_ARM == 64
#define TEST_ENVIRONMENT_HAS_JIT

static void emitVoidFunction(CodeHolder& code) noexcept {
  a64::Assembler a(&code);
  a.ret(a64::x30);
}
#endif

#if defined(TEST_ENVIRONMENT_HAS_JIT)
static void* offsetPointer(void* ptr, size_t offset) noexcept {
  return static_cast<void*>(static_cast<uint8_t*>(ptr) + offset);
}

static size_t writeEmptyFunctionAt(void* ptr, size_t size) noexcept {
  printf("  Write JIT code at addr  : %p\n", ptr);

  CodeHolder code;
  Error err = code.init(Environment::host());
  if (err != kErrorOk) {
    printf(  "Failed to initialize CodeHolder (%s)\n", DebugUtils::errorAsString(err));
    return 0;
  }

  emitVoidFunction(code);
  code.flatten();
  code.copyFlattenedData(ptr, size);

  return code.codeSize();
}

static void flushInstructionCache(void* ptr, size_t size) noexcept {
  printf("  Flush JIT code at addr  : %p [size=%zu]\n", ptr, size);
  VirtMem::flushInstructionCache(ptr, size);
}

static void invokeVoidFunction(void* ptr) noexcept {
  printf("  Invoke JIT code at addr : %p\n", ptr);

  // In case it crashes, we want to have the output flushed.
  fflush(stdout);

  VoidFunc func = reinterpret_cast<VoidFunc>(ptr);
  func();
}
#endif

static void printVirtMemInfo() noexcept {
  using MemoryFlags = VirtMem::MemoryFlags;
  using HardenedRuntimeInfo = VirtMem::HardenedRuntimeInfo;
  using HardenedRuntimeFlags = VirtMem::HardenedRuntimeFlags;

  // Size of a virtual memory allocation.
  constexpr size_t kVMemAllocSize = 65536;

  // Offset to the first function to execute (must be greater than 8 for UBSAN to work).
  [[maybe_unused]]
  constexpr size_t kVirtFuncOffset = 64;

  auto stringifyBool = [](bool b) noexcept { return b ? "true" : "false"; };
  auto stringifySuccess = [](bool b) noexcept { return b ? "success" : "failure"; };

  size_t largePageSize = VirtMem::largePageSize();
  HardenedRuntimeInfo rti = VirtMem::hardenedRuntimeInfo();

  printf("Large/Huge Pages Info:\n");
  printf("  Large pages supported   : %s\n", stringifyBool(largePageSize != 0u));
  if (largePageSize) {
    printf("  Large page size         : %zu [KiB]\n", largePageSize / 1024u);
  }
  printf("\n");

  printf("Hardened Environment Info:\n");
  printf("  Hardening was detected  : %s\n", stringifyBool(rti.hasFlag(HardenedRuntimeFlags::kEnabled    )));
  printf("  MAP_JIT is available    : %s\n", stringifyBool(rti.hasFlag(HardenedRuntimeFlags::kMapJit     )));
  printf("  DualMapping is available: %s\n", stringifyBool(rti.hasFlag(HardenedRuntimeFlags::kDualMapping)));
  printf("\n");

  if (!rti.hasFlag(HardenedRuntimeFlags::kEnabled)) {
    printf("Virtual Memory Allocation (RWX):\n");

    void* ptr = nullptr;
    Error result = VirtMem::alloc(&ptr, kVMemAllocSize, MemoryFlags::kAccessRWX);
    printf("  Alloc virt memory (RWX) : %s\n", stringifySuccess(result == kErrorOk));

    if (result == kErrorOk) {
#if defined(TEST_ENVIRONMENT_HAS_JIT)
      void* funcPtr = offsetPointer(ptr, kVirtFuncOffset);
      size_t funcSize = writeEmptyFunctionAt(funcPtr, kVMemAllocSize);

      if (funcSize) {
        flushInstructionCache(funcPtr, funcSize);
        invokeVoidFunction(funcPtr);
      }
#endif // TEST_ENVIRONMENT_HAS_JIT

      result = VirtMem::release(ptr, kVMemAllocSize);
      printf("  Release virt memory     : %s\n", stringifySuccess(result == kErrorOk));
    }

    printf("\n");
  }

  {
    printf("Virtual Memory Allocation (RW - Flipping Permissions RW<->RX):\n");

    void* ptr = nullptr;
    Error result = VirtMem::alloc(&ptr, kVMemAllocSize, MemoryFlags::kAccessRW | MemoryFlags::kMMapMaxAccessRWX);
    printf("  Alloc virt memory (RW)  : %s (allocation uses kMMapMaxAccessRWX)\n", stringifySuccess(result == kErrorOk));

    if (result == kErrorOk) {
#if defined(TEST_ENVIRONMENT_HAS_JIT)
      void* funcPtr = offsetPointer(ptr, kVirtFuncOffset);
      size_t funcSize = writeEmptyFunctionAt(funcPtr, kVMemAllocSize);
#endif // TEST_ENVIRONMENT_HAS_JIT

      result = VirtMem::protect(ptr, kVMemAllocSize, MemoryFlags::kAccessRX);
      printf("  Protect virt memory (RX): %s\n", stringifySuccess(result == kErrorOk));

#if defined(TEST_ENVIRONMENT_HAS_JIT)
      if (funcSize) {
        flushInstructionCache(funcPtr, funcSize);
        invokeVoidFunction(funcPtr);
      }
#endif // TEST_ENVIRONMENT_HAS_JIT

      result = VirtMem::protect(ptr, kVMemAllocSize, MemoryFlags::kAccessRW);
      printf("  Protect virt memory (RW): %s\n", stringifySuccess(result == kErrorOk));

      result = VirtMem::release(ptr, kVMemAllocSize);
      printf("  Release virt memory (RW): %s\n", stringifySuccess(result == kErrorOk));
    }

    printf("\n");
  }

  if (rti.hasFlag(HardenedRuntimeFlags::kMapJit)) {
    printf("Virtual Memory Allocation (MAP_JIT):\n");

    void* ptr = nullptr;
    Error result = VirtMem::alloc(&ptr, kVMemAllocSize, MemoryFlags::kAccessRWX | MemoryFlags::kMMapEnableMapJit);
    printf("  Alloc virt mem (RWX)    : %s (allocation uses kMMapEnableMapJit)\n", stringifySuccess(result == kErrorOk));

    if (result == kErrorOk) {
      printf("  Protect JIT Memory (RW) : (per-thread protection)\n");
      VirtMem::protectJitMemory(VirtMem::ProtectJitAccess::kReadWrite);

#if defined(TEST_ENVIRONMENT_HAS_JIT)
      void* funcPtr = offsetPointer(ptr, kVirtFuncOffset);
      size_t funcSize = writeEmptyFunctionAt(funcPtr, kVMemAllocSize);
#endif // TEST_ENVIRONMENT_HAS_JIT

      printf("  Protect JIT Memory (RX) : (per-thread protection)\n");
      VirtMem::protectJitMemory(VirtMem::ProtectJitAccess::kReadExecute);

#if defined(TEST_ENVIRONMENT_HAS_JIT)
      if (funcSize) {
        flushInstructionCache(funcPtr, funcSize);
        invokeVoidFunction(funcPtr);
      }
#endif // TEST_ENVIRONMENT_HAS_JIT

      result = VirtMem::release(ptr, kVMemAllocSize);
      printf("  Release virt memory     : %s\n", stringifySuccess(result == kErrorOk));
    }

    printf("\n");
  }

  if (rti.hasFlag(HardenedRuntimeFlags::kDualMapping)) {
    printf("Virtual Memory Allocation (Dual Mapping):\n");

    VirtMem::DualMapping dm {};
    Error result = VirtMem::allocDualMapping(&dm, kVMemAllocSize, MemoryFlags::kAccessRWX);
    printf("  Alloc dual mem (RW+RX)  : %s\n", stringifySuccess(result == kErrorOk));

    if (result == kErrorOk) {
#if defined(TEST_ENVIRONMENT_HAS_JIT)
      size_t funcSize = writeEmptyFunctionAt(offsetPointer(dm.rw, kVirtFuncOffset), kVMemAllocSize);
      if (funcSize) {
        flushInstructionCache(offsetPointer(dm.rx, kVirtFuncOffset), funcSize);
        invokeVoidFunction(offsetPointer(dm.rx, kVirtFuncOffset));
      }
#endif // TEST_ENVIRONMENT_HAS_JIT

      result = VirtMem::releaseDualMapping(&dm, kVMemAllocSize);
      printf("  Release dual mem (RW+RX): %s\n", stringifySuccess(result == kErrorOk));
    }

    printf("\n");
  }

  printf("\n");
}
#endif // !ASMJIT_NO_JIT

int main() {
  printf("AsmJit Environment Test v%u.%u.%u [Arch=%s]\n\n",
    unsigned((ASMJIT_LIBRARY_VERSION >> 16)       ),
    unsigned((ASMJIT_LIBRARY_VERSION >>  8) & 0xFF),
    unsigned((ASMJIT_LIBRARY_VERSION      ) & 0xFF),
    asmjitArchAsString(Arch::kHost)
  );

  printf("This application can be used to verify AsmJit build options and to verify the\n");
  printf("environment where it runs. For example to check CPU extensions available, system\n");
  printf("hardening (RWX restrictions), large page support, and virtual memory allocations.\n");
  printf("\n");

  printBuildOptions();
  printCpuInfo();

#if !defined(ASMJIT_NO_JIT)
  printVirtMemInfo();
#endif // !ASMJIT_NO_JIT

  return 0;
}
