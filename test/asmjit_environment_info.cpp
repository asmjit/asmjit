// This file is part of AsmJit project <https://asmjit.com>
//
// See asmjit.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core.h>

#include "asmjitutils.h"

using namespace asmjit;

#if !defined(ASMJIT_NO_JIT)
static void printVirtMemInfo() {
  using MemoryFlags = VirtMem::MemoryFlags;
  using HardenedRuntimeInfo = VirtMem::HardenedRuntimeInfo;
  using HardenedRuntimeFlags = VirtMem::HardenedRuntimeFlags;

  constexpr size_t kVMemAllocSize = 65536;

  auto stringifyBool = [](bool b) { return b ? "true" : "false"; };
  auto stringifySuccess = [](bool b) { return b ? "success" : "failure"; };

  HardenedRuntimeInfo rti = VirtMem::hardenedRuntimeInfo();

  printf("Hardened Environment Info:\n");
  printf("  Hardening was detected  : %s\n", stringifyBool(rti.hasFlag(HardenedRuntimeFlags::kEnabled    )));
  printf("  MAP_JIT is available    : %s\n", stringifyBool(rti.hasFlag(HardenedRuntimeFlags::kMapJit     )));
  printf("  DualMapping is available: %s\n", stringifyBool(rti.hasFlag(HardenedRuntimeFlags::kDualMapping)));
  printf("\n");

  printf("Executable Memory Allocation:\n");

  {
    void* ptr = nullptr;
    Error result = VirtMem::alloc(&ptr, kVMemAllocSize, MemoryFlags::kAccessRWX);
    printf("  Allocation of RWX memory: %s\n", stringifySuccess(result == kErrorOk));

    if (result == kErrorOk) {
      result = VirtMem::release(ptr, kVMemAllocSize);
      printf("  Release of RWX memory   : %s\n", stringifySuccess(result == kErrorOk));
    }
  }

  {
    void* ptr = nullptr;
    Error result = VirtMem::alloc(&ptr, kVMemAllocSize, MemoryFlags::kAccessRW | MemoryFlags::kMMapMaxAccessRWX);
    printf("  Allocation of RW_ memory: %s (allocation uses kMMapMaxAccessRWX)\n", stringifySuccess(result == kErrorOk));

    if (result == kErrorOk) {
      result = VirtMem::protect(ptr, kVMemAllocSize, MemoryFlags::kAccessRX);
      printf("  Change Access {RW -> RX}: %s\n", stringifySuccess(result == kErrorOk));

      result = VirtMem::protect(ptr, kVMemAllocSize, MemoryFlags::kAccessRW);
      printf("  Change Access {RX -> RW}: %s\n", stringifySuccess(result == kErrorOk));

      result = VirtMem::release(ptr, kVMemAllocSize);
      printf("  Release of RW memory    : %s\n", stringifySuccess(result == kErrorOk));
    }
  }

  {
    VirtMem::DualMapping dm {};
    Error result = VirtMem::allocDualMapping(&dm, kVMemAllocSize, MemoryFlags::kAccessRWX);
    printf("  Dual mapping RWX alloc  : %s\n", stringifySuccess(result == kErrorOk));

    if (result == kErrorOk) {
      result = VirtMem::releaseDualMapping(&dm, kVMemAllocSize);
      printf("  Dual mapping RWX release: %s\n", stringifySuccess(result == kErrorOk));
    }
  }

  printf("\n");
}
#endif // ASMJIT_NO_JIT

int main() {
  printf("AsmJit Environment Info v%u.%u.%u [Arch=%s]\n\n",
    unsigned((ASMJIT_LIBRARY_VERSION >> 16)       ),
    unsigned((ASMJIT_LIBRARY_VERSION >>  8) & 0xFF),
    unsigned((ASMJIT_LIBRARY_VERSION      ) & 0xFF),
    asmjitArchAsString(Arch::kHost)
  );

  printf("This application can be used to verify AsmJit build options and to verify the\n");
  printf("environment where it runs. For example to check CPU extensions available, OS\n");
  printf("hardening and virtual memory allocation options.\n");
  printf("\n");

  printBuildOptions();
  printCpuInfo();

#if !defined(ASMJIT_NO_JIT)
  printVirtMemInfo();
#endif // ASMJIT_NO_JIT

  return 0;
}
