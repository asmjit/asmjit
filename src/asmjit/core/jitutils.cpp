// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// ZLIB - See LICENSE.md file in the package.

// [Export]
#define ASMJIT_EXPORTS

#include "../core/build.h"
#ifndef ASMJIT_DISABLE_JIT

// [Dependencies]
#include "../core/jitutils.h"
#include "../core/support.h"

#if ASMJIT_OS_POSIX
  #include <sys/types.h>
  #include <sys/mman.h>
  #include <unistd.h>
#endif

ASMJIT_BEGIN_NAMESPACE

// ============================================================================
// [asmjit::JitUtils - Virtual Memory (Windows)]
// ============================================================================

// Windows specific implementation using `VirtualAlloc` and `VirtualFree`.
#if ASMJIT_OS_WINDOWS
static inline DWORD JitUtils_vmFlagsToProtectFlags(uint32_t vmFlags) noexcept {
  DWORD protectFlags = 0;
  if (vmFlags & JitUtils::kVirtMemExecute)
    protectFlags |= (vmFlags & JitUtils::kVirtMemWrite) ? PAGE_EXECUTE_READWRITE : PAGE_EXECUTE_READ;
  else
    protectFlags |= (vmFlags & JitUtils::kVirtMemWrite) ? PAGE_READWRITE : PAGE_READONLY;
  return protectFlags;
}

JitUtils::MemInfo JitUtils::memInfo() noexcept {
  MemInfo memInfo;
  SYSTEM_INFO systemInfo;

  ::GetSystemInfo(&systemInfo);
  memInfo.pageSize = Support::alignUpPowerOf2<uint32_t>(systemInfo.dwPageSize);
  memInfo.pageGranularity = systemInfo.dwAllocationGranularity;

  return memInfo;
}

void* JitUtils::virtualAlloc(size_t size, uint32_t vmFlags) noexcept {
  if (size == 0)
    return nullptr;

  DWORD protectFlags = JitUtils_vmFlagsToProtectFlags(vmFlags);
  return ::VirtualAlloc(nullptr, size, MEM_COMMIT | MEM_RESERVE, protectFlags);
}

Error JitUtils::virtualRelease(void* p, size_t size) noexcept {
  ASMJIT_UNUSED(size);
  if (ASMJIT_UNLIKELY(!::VirtualFree(p, 0, MEM_RELEASE)))
    return DebugUtils::errored(kErrorInvalidState);
  return kErrorOk;
}
#endif

// ============================================================================
// [asmjit::JitUtils - Virtual Memory (Posix)]
// ============================================================================

// Posix specific implementation using `mmap()` and `munmap()`.
#if ASMJIT_OS_POSIX

// BSD/OSX: `MAP_ANONYMOUS` is not defined, `MAP_ANON` is.
#if !defined(MAP_ANONYMOUS)
  #define MAP_ANONYMOUS MAP_ANON
#endif

static inline int JitUtils_vmFlagsToProtection(uint32_t vmFlags) noexcept {
  int protection = PROT_READ;
  if (vmFlags & JitUtils::kVirtMemWrite  ) protection |= PROT_WRITE;
  if (vmFlags & JitUtils::kVirtMemExecute) protection |= PROT_EXEC;
  return protection;
}

JitUtils::MemInfo JitUtils::memInfo() noexcept {
  MemInfo memInfo;

  uint32_t pageSize = uint32_t(::getpagesize());
  memInfo.pageSize = pageSize;
  memInfo.pageGranularity = std::max<uint32_t>(pageSize, 65536);

  return memInfo;
}

void* JitUtils::virtualAlloc(size_t size, uint32_t vmFlags) noexcept {
  int protection = JitUtils_vmFlagsToProtection(vmFlags);
  void* mbase = ::mmap(nullptr, size, protection, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

  if (mbase == MAP_FAILED)
    return nullptr;
  else
    return mbase;
}

Error JitUtils::virtualRelease(void* p, size_t size) noexcept {
  if (ASMJIT_UNLIKELY(::munmap(p, size) != 0))
    return DebugUtils::errored(kErrorInvalidState);

  return kErrorOk;
}
#endif

ASMJIT_END_NAMESPACE

#endif
