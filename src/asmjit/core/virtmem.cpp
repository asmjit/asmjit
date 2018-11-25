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
#include "../core/osutils.h"
#include "../core/support.h"
#include "../core/virtmem.h"

#if !defined(_WIN32)
  #include <errno.h>
  #include <fcntl.h>
  #include <sys/mman.h>
  #include <sys/types.h>
  #include <unistd.h>

  // Linux has a `memfd_create` syscall that we would like to use, if available.
  #if defined(__linux__)
    #include <sys/syscall.h>
  #endif

  // BSD/OSX: `MAP_ANONYMOUS` is not defined, `MAP_ANON` is.
  #if !defined(MAP_ANONYMOUS)
    #define MAP_ANONYMOUS MAP_ANON
  #endif
#endif

#include <atomic>

ASMJIT_BEGIN_NAMESPACE

// ============================================================================
// [asmjit::VirtMem - Utilities]
// ============================================================================

static const uint32_t allocViewFilter[2] = {
  VirtMem::kAccessWrite,
  VirtMem::kAccessExecute
};

// ============================================================================
// [asmjit::VirtMem - Virtual Memory [Windows]]
// ============================================================================

#if defined(_WIN32)

struct ScopedHandle {
  inline ScopedHandle() noexcept
    : value(nullptr) {}

  inline ~ScopedHandle() noexcept {
    if (value != nullptr)
      ::CloseHandle(value);
  }

  HANDLE value;
};

static void VirtMem_getInfo(VirtMem::Info& vmInfo) noexcept {
  SYSTEM_INFO systemInfo;

  ::GetSystemInfo(&systemInfo);
  vmInfo.pageSize = Support::alignUpPowerOf2<uint32_t>(systemInfo.dwPageSize);
  vmInfo.pageGranularity = systemInfo.dwAllocationGranularity;
}

// Windows specific implementation that uses `VirtualAlloc` and `VirtualFree`.
static DWORD VirtMem_accessToWinProtectFlags(uint32_t accessFlags) noexcept {
  DWORD protectFlags;

  // READ|WRITE|EXECUTE.
  if (accessFlags & VirtMem::kAccessExecute)
    protectFlags = (accessFlags & VirtMem::kAccessWrite) ? PAGE_EXECUTE_READWRITE : PAGE_EXECUTE_READ;
  else if (accessFlags & VirtMem::kAccessReadWrite)
    protectFlags = (accessFlags & VirtMem::kAccessWrite) ? PAGE_READWRITE : PAGE_READONLY;
  else
    protectFlags = PAGE_NOACCESS;

  // Any other flags to consider?
  return protectFlags;
}

static DWORD VirtMem_accessToWinDesiredAccess(uint32_t accessFlags) noexcept {
  DWORD access = (accessFlags & VirtMem::kAccessWrite) ? FILE_MAP_WRITE : FILE_MAP_READ;
  if (accessFlags & VirtMem::kAccessExecute)
    access |= FILE_MAP_EXECUTE;
  return access;
}

Error VirtMem::alloc(void** p, size_t size, uint32_t accessFlags) noexcept {
  *p = nullptr;
  if (size == 0)
    return DebugUtils::errored(kErrorInvalidArgument);

  DWORD protectFlags = VirtMem_accessToWinProtectFlags(accessFlags);
  void* result = ::VirtualAlloc(nullptr, size, MEM_COMMIT | MEM_RESERVE, protectFlags);

  if (!result)
    return DebugUtils::errored(kErrorOutOfMemory);

  *p = result;
  return kErrorOk;
}

Error VirtMem::release(void* p, size_t size) noexcept {
  ASMJIT_UNUSED(size);
  if (ASMJIT_UNLIKELY(!::VirtualFree(p, 0, MEM_RELEASE)))
    return DebugUtils::errored(kErrorInvalidArgument);
  return kErrorOk;
}

Error VirtMem::protect(void* p, size_t size, uint32_t accessFlags) noexcept {
  DWORD protectFlags = VirtMem_accessToWinProtectFlags(accessFlags);
  DWORD oldFlags;

  if (::VirtualProtect(p, size, protectFlags, &oldFlags))
    return kErrorOk;

  return DebugUtils::errored(kErrorInvalidArgument);
}

Error VirtMem::allocDualMapping(DualMapping* dm, size_t size, uint32_t accessFlags) noexcept {
  dm->ro = nullptr;
  dm->rw = nullptr;

  if (size == 0)
    return DebugUtils::errored(kErrorInvalidArgument);

  ScopedHandle handle;
  handle.value = ::CreateFileMappingW(
    INVALID_HANDLE_VALUE,
    nullptr,
    PAGE_READWRITE,
    (DWORD)(size & 0xFFFFFFFFu),
    (DWORD)(uint64_t(size) >> 32),
    nullptr);

  if (ASMJIT_UNLIKELY(!handle.value))
    return DebugUtils::errored(kErrorOutOfMemory);

  void* ptr[2];
  for (uint32_t i = 0; i < 2; i++) {
    DWORD desiredAccess = VirtMem_accessToWinDesiredAccess(accessFlags & ~allocViewFilter[i]);
    ptr[i] = ::MapViewOfFile(handle.value, desiredAccess, 0, 0, size);

    if (ptr[i] == nullptr) {
      if (i == 0)
        ::UnmapViewOfFile(ptr[0]);
      return DebugUtils::errored(kErrorOutOfMemory);
    }
  }

  dm->ro = ptr[0];
  dm->rw = ptr[1];
  return kErrorOk;
}

Error VirtMem::releaseDualMapping(DualMapping* dm, size_t size) noexcept {
  ASMJIT_UNUSED(size);
  bool failed = false;

  if (!::UnmapViewOfFile(dm->ro))
    failed = true;

  if (dm->ro != dm->rw && !UnmapViewOfFile(dm->rw))
    failed = true;

  if (failed)
    return DebugUtils::errored(kErrorInvalidArgument);

  dm->ro = nullptr;
  dm->rw = nullptr;
  return kErrorOk;
}

#endif

// ============================================================================
// [asmjit::VirtMem - Virtual Memory [Posix]]
// ============================================================================

#if !defined(_WIN32)

#if defined(SYS_memfd_create)
// Zero initialized, if ever changed to '1' that would mean the syscall is not
// available and we must use `shm_open()` and `shm_unlink()`.
static volatile int _memfd_create_not_supported;
#endif

struct ScopedFD {
  inline ScopedFD() noexcept
    : value(-1) {}

  inline ~ScopedFD() noexcept {
    if (value != -1)
      close(value);
  }

  int value;
};

static void VirtMem_getInfo(VirtMem::Info& vmInfo) noexcept {
  uint32_t pageSize = uint32_t(::getpagesize());

  vmInfo.pageSize = pageSize;
  vmInfo.pageGranularity = Support::max<uint32_t>(pageSize, 65536);
}

// Posix specific implementation that uses `mmap()` and `munmap()`.
static int VirtMem_accessToPosixProtection(uint32_t accessFlags) noexcept {
  int protection = 0;
  if (accessFlags & VirtMem::kAccessRead   ) protection |= PROT_READ;
  if (accessFlags & VirtMem::kAccessWrite  ) protection |= PROT_READ | PROT_WRITE;
  if (accessFlags & VirtMem::kAccessExecute) protection |= PROT_READ | PROT_EXEC;
  return protection;
}

static Error VirtMem_makeErrorFromErrno(int e) noexcept {
  switch (e) {
    case EACCES:
    case EAGAIN:
    case ENODEV:
    case EPERM:
      return kErrorInvalidState;

    case EFBIG:
    case ENOMEM:
    case EOVERFLOW:
      return kErrorOutOfMemory;

    case EMFILE:
    case ENFILE:
      return kErrorTooManyHandles;

    default:
      return kErrorInvalidArgument;
  }
}

static Error VirtMem_openAnonymousMemory(int* fd) noexcept {
#if defined(SYS_memfd_create)
  // Linux specific 'memfd_create' - if the syscall returns `ENOSYS` it means
  // it's not available and we will never call it again (would be pointless).
  if (!_memfd_create_not_supported) {
    *fd = (int)syscall(SYS_memfd_create, "vmem", 0);
    if (ASMJIT_UNLIKELY(*fd >= 0))
      return kErrorOk;

    int e = errno;
    if (e == ENOSYS)
      _memfd_create_not_supported = 1;
    else
      return DebugUtils::errored(VirtMem_makeErrorFromErrno(e));
  }
#endif

#if defined(SHM_ANON)
  // Originally FreeBSD extension, apparently works in other BSDs too.
  *fd = shm_open(SHM_ANON, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
  if (ASMJIT_UNLIKELY(*fd < 0))
    return DebugUtils::errored(VirtMem_makeErrorFromErrno(errno));
  return kErrorOk;
#else
  // POSIX API. We have to generate somehow a unique name. This is nothing
  // cryptographic, just using a bit from the stack address to always have
  // a different base for different threads (as threads have their own stack)
  // and retries for avoiding collisions. We use `shm_open()` with flags that
  // require creation of the file so we never open an existing shared memory.
  static std::atomic<uint32_t> internalCounter;

  char uniqueName[128];
  uint32_t kRetryCount = 100;
  uint64_t bits = ((uintptr_t)(void*)uniqueName) & 0x55555555u;

  for (uint32_t i = 0; i < kRetryCount; i++) {
    bits -= uint64_t(OSUtils::getTickCount()) * 773703683;
    bits = ((bits >> 14) ^ (bits << 6)) + uint64_t(++internalCounter) * 10619863;
    snprintf(uniqueName, sizeof(uniqueName), "/shm-id-%08llX", (unsigned long long)bits);

    *fd = shm_open(uniqueName, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
    if (ASMJIT_UNLIKELY(*fd >= 0)) {
      shm_unlink(uniqueName);
      return kErrorOk;
    }

    int e = errno;
    if (e == EEXIST)
      continue;
    else
      return DebugUtils::errored(VirtMem_makeErrorFromErrno(e));
  }
  return kErrorOk;
#endif
}

Error VirtMem::alloc(void** p, size_t size, uint32_t accessFlags) noexcept {
  *p = nullptr;

  if (size == 0)
    return DebugUtils::errored(kErrorInvalidArgument);

  int protection = VirtMem_accessToPosixProtection(accessFlags);
  void* ptr = mmap(nullptr, size, protection, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

  if (ptr == MAP_FAILED)
    return DebugUtils::errored(kErrorOutOfMemory);

  *p = ptr;
  return kErrorOk;
}

Error VirtMem::release(void* p, size_t size) noexcept {
  if (ASMJIT_UNLIKELY(munmap(p, size) != 0))
    return DebugUtils::errored(kErrorInvalidArgument);

  return kErrorOk;
}


Error VirtMem::protect(void* p, size_t size, uint32_t accessFlags) noexcept {
  int protection = VirtMem_accessToPosixProtection(accessFlags);
  if (mprotect(p, size, protection) == 0)
    return kErrorOk;

  return DebugUtils::errored(kErrorInvalidArgument);
}

Error VirtMem::allocDualMapping(DualMapping* dm, size_t size, uint32_t accessFlags) noexcept {
  dm->ro = nullptr;
  dm->rw = nullptr;

  if (off_t(size) <= 0)
    return DebugUtils::errored(size == 0 ? kErrorInvalidArgument : kErrorTooLarge);

  // ScopedFD will automatically close the file descriptor in destructor.
  ScopedFD fd;

  ASMJIT_PROPAGATE(VirtMem_openAnonymousMemory(&fd.value));
  if (ftruncate(fd.value, off_t(size)) != 0)
    return DebugUtils::errored(VirtMem_makeErrorFromErrno(errno));

  void* ptr[2];
  for (uint32_t i = 0; i < 2; i++) {
    ptr[i] = mmap(nullptr, size, VirtMem_accessToPosixProtection(accessFlags & ~allocViewFilter[i]), MAP_SHARED, fd.value, 0);
    if (ptr[i] == MAP_FAILED) {
      // Must convert the error now as `munmap()` could clobber `errno`.
      Error err = DebugUtils::errored(VirtMem_makeErrorFromErrno(errno));
      if (i == 1)
        munmap(ptr[0], size);
      return err;
    }
  }

  dm->ro = ptr[0];
  dm->rw = ptr[1];
  return kErrorOk;
}

Error VirtMem::releaseDualMapping(DualMapping* dm, size_t size) noexcept {
  Error err = release(dm->ro, size);
  if (dm->ro != dm->rw)
    err |= release(dm->ro, size);

  if (err)
    return DebugUtils::errored(kErrorInvalidArgument);

  dm->ro = nullptr;
  dm->rw = nullptr;
  return kErrorOk;
}

#endif

// ============================================================================
// [asmjit::VirtMem - Virtual Memory [Memory Info]]
// ============================================================================

VirtMem::Info VirtMem::info() noexcept {
  static VirtMem::Info vmInfo;
  static std::atomic<uint32_t> vmInfoInitialized;

  if (!vmInfoInitialized.load()) {
    VirtMem::Info localMemInfo;
    VirtMem_getInfo(localMemInfo);

    vmInfo = localMemInfo;
    vmInfoInitialized.store(1u);
  }

  return vmInfo;
}

ASMJIT_END_NAMESPACE

#endif
