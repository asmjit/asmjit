// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
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

static void printAppInfo() {
  printf("AsmJit Environment Test v%u.%u.%u [Arch=%s] [Mode=%s]\n\n",
    unsigned((ASMJIT_LIBRARY_VERSION >> 16)       ),
    unsigned((ASMJIT_LIBRARY_VERSION >>  8) & 0xFF),
    unsigned((ASMJIT_LIBRARY_VERSION      ) & 0xFF),
    asmjitArchAsString(Arch::kHost),
    asmjitBuildType()
  );

  printf("This application can be used to verify AsmJit build options and to verify the\n");
  printf("environment where it runs. For example to check CPU extensions available, system\n");
  printf("hardening (RWX restrictions), large page support, and virtual memory allocations.\n");
  printf("\n");
}

const char* stringifyBool(bool b) noexcept { return b ? "true" : "false"; };
const char* stringifyResult(Error err) noexcept { return err == kErrorOk ? "success" : DebugUtils::errorAsString(err); };

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

static void printVirtMemInfoAndTestExecution() noexcept {
  using MemoryFlags = VirtMem::MemoryFlags;
  using HardenedRuntimeInfo = VirtMem::HardenedRuntimeInfo;
  using HardenedRuntimeFlags = VirtMem::HardenedRuntimeFlags;

  // Size of a virtual memory allocation.
  constexpr size_t kVMemAllocSize = 65536;

  // Offset to the first function to execute (must be greater than 8 for UBSAN to work).
  [[maybe_unused]]
  constexpr size_t kVirtFuncOffset = 64;

  size_t largePageSize = VirtMem::largePageSize();
  HardenedRuntimeInfo rti = VirtMem::hardenedRuntimeInfo();

  printf("Large/Huge Pages Info:\n");
  printf("  Large pages supported   : %s\n", stringifyBool(largePageSize != 0u));
  if (largePageSize >= 1024 * 1024) {
    printf("  Large page size         : %zu MiB\n", largePageSize / (1024u * 1024u));
  }
  else if (largePageSize) {
    printf("  Large page size         : %zu KiB\n", largePageSize / 1024u);
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
    printf("  Alloc virt memory (RWX) : %s\n", stringifyResult(result));

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
      printf("  Release virt memory     : %s\n", stringifyResult(result));
    }

    printf("\n");
  }

  {
    printf("Virtual Memory Allocation (RW - Flipping Permissions RW<->RX):\n");

    void* ptr = nullptr;
    Error result = VirtMem::alloc(&ptr, kVMemAllocSize, MemoryFlags::kAccessRW | MemoryFlags::kMMapMaxAccessRWX);
    printf("  Alloc virt memory (RW)  : %s (allocation uses kMMapMaxAccessRWX)\n", stringifyResult(result));

    if (result == kErrorOk) {
#if defined(TEST_ENVIRONMENT_HAS_JIT)
      void* funcPtr = offsetPointer(ptr, kVirtFuncOffset);
      size_t funcSize = writeEmptyFunctionAt(funcPtr, kVMemAllocSize);
#endif // TEST_ENVIRONMENT_HAS_JIT

      result = VirtMem::protect(ptr, kVMemAllocSize, MemoryFlags::kAccessRX);
      printf("  Protect virt memory (RX): %s\n", stringifyResult(result));

#if defined(TEST_ENVIRONMENT_HAS_JIT)
      if (funcSize) {
        flushInstructionCache(funcPtr, funcSize);
        invokeVoidFunction(funcPtr);
      }
#endif // TEST_ENVIRONMENT_HAS_JIT

      result = VirtMem::protect(ptr, kVMemAllocSize, MemoryFlags::kAccessRW);
      printf("  Protect virt memory (RW): %s\n", stringifyResult(result));

      result = VirtMem::release(ptr, kVMemAllocSize);
      printf("  Release virt memory (RW): %s\n", stringifyResult(result));
    }

    printf("\n");
  }

  if (rti.hasFlag(HardenedRuntimeFlags::kMapJit)) {
    printf("Virtual Memory Allocation (MAP_JIT):\n");

    void* ptr = nullptr;
    Error result = VirtMem::alloc(&ptr, kVMemAllocSize, MemoryFlags::kAccessRWX | MemoryFlags::kMMapEnableMapJit);
    printf("  Alloc virt mem (RWX)    : %s (allocation uses kMMapEnableMapJit)\n", stringifyResult(result));

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
      printf("  Release virt memory     : %s\n", stringifyResult(result));
    }

    printf("\n");
  }

  if (rti.hasFlag(HardenedRuntimeFlags::kDualMapping)) {
    printf("Virtual Memory Allocation (Dual Mapping):\n");

    VirtMem::DualMapping dm {};
    Error result = VirtMem::allocDualMapping(&dm, kVMemAllocSize, MemoryFlags::kAccessRWX);
    printf("  Alloc dual mem (RW+RX)  : %s\n", stringifyResult(result));

    if (result == kErrorOk) {
#if defined(TEST_ENVIRONMENT_HAS_JIT)
      size_t funcSize = writeEmptyFunctionAt(offsetPointer(dm.rw, kVirtFuncOffset), kVMemAllocSize);
      if (funcSize) {
        flushInstructionCache(offsetPointer(dm.rx, kVirtFuncOffset), funcSize);
        invokeVoidFunction(offsetPointer(dm.rx, kVirtFuncOffset));
      }
#endif // TEST_ENVIRONMENT_HAS_JIT

      result = VirtMem::releaseDualMapping(&dm, kVMemAllocSize);
      printf("  Release dual mem (RW+RX): %s\n", stringifyResult(result));
    }

    printf("\n");
  }
}

#if defined(TEST_ENVIRONMENT_HAS_JIT)
static void printJitRuntimeInfoAndTestExecutionWithParams(const JitAllocator::CreateParams* params, const char* paramsName) noexcept {
  printf("JitRuntime (%s):\n", paramsName);

  JitRuntime rt(params);
  CodeHolder code;

  Error result = code.init(rt.environment());
  printf("  CodeHolder init result  : %s\n", stringifyResult(result));

  if (result != kErrorOk) {
    return;
  }

  emitVoidFunction(code);
  VoidFunc fn;

  result = rt.add(&fn, &code);
  printf("  Runtime.add() result    : %s\n", stringifyResult(result));

  if (result == kErrorOk) {
    invokeVoidFunction((void*)fn);

    result = rt.release(fn);
    printf("  Runtime.release() result: %s\n", stringifyResult(result));
  }

  printf("\n");
}

static void printJitRuntimeInfoAndTestExecution() noexcept {
  printJitRuntimeInfoAndTestExecutionWithParams(nullptr, "<no params>");

  if (VirtMem::largePageSize()) {
    JitAllocator::CreateParams p{};
    p.options = JitAllocatorOptions::kUseLargePages;

    printJitRuntimeInfoAndTestExecutionWithParams(&p, "large pages");
  }
}
#endif // TEST_ENVIRONMENT_HAS_JIT

#endif // !ASMJIT_NO_JIT

int main() {
  printAppInfo();
  printBuildOptions();
  printCpuInfo();

#if !defined(ASMJIT_NO_JIT)
  printVirtMemInfoAndTestExecution();
#endif // !ASMJIT_NO_JIT

#if !defined(ASMJIT_NO_JIT) && defined(TEST_ENVIRONMENT_HAS_JIT)
  printJitRuntimeInfoAndTestExecution();
#endif // !ASMJIT_NO_JIT && TEST_ENVIRONMENT_HAS_JIT

  return 0;
}
