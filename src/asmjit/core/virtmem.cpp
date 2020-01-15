// [AsmJit]
// Machine Code Generation for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

#define ASMJIT_EXPORTS

#include "../core/build.h"
#ifndef ASMJIT_NO_JIT

#include "../core/osutils.h"
#include "../core/string.h"
#include "../core/support.h"
#include "../core/virtmem.h"

#if !defined(_WIN32)
  #include <errno.h>
  #include <fcntl.h>
  #include <sys/mman.h>
  #include <sys/stat.h>
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

static const uint32_t dualMappingFilter[2] = {
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
static DWORD VirtMem_accessToWinProtectFlags(uint32_t flags) noexcept {
  DWORD protectFlags;

  // READ|WRITE|EXECUTE.
  if (flags & VirtMem::kAccessExecute)
    protectFlags = (flags & VirtMem::kAccessWrite) ? PAGE_EXECUTE_READWRITE : PAGE_EXECUTE_READ;
  else if (flags & VirtMem::kAccessReadWrite)
    protectFlags = (flags & VirtMem::kAccessWrite) ? PAGE_READWRITE : PAGE_READONLY;
  else
    protectFlags = PAGE_NOACCESS;

  // Any other flags to consider?
  return protectFlags;
}

static DWORD VirtMem_accessToWinDesiredAccess(uint32_t flags) noexcept {
  DWORD access = (flags & VirtMem::kAccessWrite) ? FILE_MAP_WRITE : FILE_MAP_READ;
  if (flags & VirtMem::kAccessExecute)
    access |= FILE_MAP_EXECUTE;
  return access;
}

Error VirtMem::alloc(void** p, size_t size, uint32_t flags) noexcept {
  *p = nullptr;
  if (size == 0)
    return DebugUtils::errored(kErrorInvalidArgument);

  DWORD protectFlags = VirtMem_accessToWinProtectFlags(flags);
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

Error VirtMem::protect(void* p, size_t size, uint32_t flags) noexcept {
  DWORD protectFlags = VirtMem_accessToWinProtectFlags(flags);
  DWORD oldFlags;

  if (::VirtualProtect(p, size, protectFlags, &oldFlags))
    return kErrorOk;

  return DebugUtils::errored(kErrorInvalidArgument);
}

Error VirtMem::allocDualMapping(DualMapping* dm, size_t size, uint32_t flags) noexcept {
  dm->ro = nullptr;
  dm->rw = nullptr;

  if (size == 0)
    return DebugUtils::errored(kErrorInvalidArgument);

  ScopedHandle handle;
  handle.value = ::CreateFileMappingW(
    INVALID_HANDLE_VALUE,
    nullptr,
    PAGE_EXECUTE_READWRITE,
    (DWORD)(uint64_t(size) >> 32),
    (DWORD)(size & 0xFFFFFFFFu),
    nullptr);

  if (ASMJIT_UNLIKELY(!handle.value))
    return DebugUtils::errored(kErrorOutOfMemory);

  void* ptr[2];
  for (uint32_t i = 0; i < 2; i++) {
    DWORD desiredAccess = VirtMem_accessToWinDesiredAccess(flags & ~dualMappingFilter[i]);
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
struct ScopedFD {
  inline ScopedFD() noexcept
    : value(-1) {}

  inline ~ScopedFD() noexcept {
    if (value != -1)
      close(value);
  }

  int value;
};

#if defined(SYS_memfd_create)
// Zero initialized, if ever changed to '1' that would mean the syscall is not
// available and we must use `shm_open()` and `shm_unlink()`.
static volatile uint32_t vm_memfd_create_not_supported;
#endif

// Some operating systems don't allow /dev/shm to be executable. On Linux this
// happens when /dev/shm is mounted with 'noexec', which is enforced by systemd.
// Other operating systems like OSX also restrict executable permissions regarding
// /dev/shm, so we use a runtime detection before trying to allocate the requested
// memory by the user. Sometimes we don't need the detection as we know it would
// always result in 'kShmStrategyTmpDir'.
enum ShmStrategy : uint32_t {
  kShmStrategyUnknown = 0,
  kShmStrategyDevShm = 1,
  kShmStrategyTmpDir = 2
};

#if defined(__APPLE__)
  #define ASMJIT_VM_SHM_DETECT 0
#else
  #define ASMJIT_VM_SHM_DETECT 1
#endif

#if ASMJIT_VM_SHM_DETECT
// Initially don't assume anything. It has to be tested whether '/dev/shm' was
// mounted with 'noexec' flag or not.
static volatile uint32_t vm_shm_strategy = kShmStrategyUnknown;
#endif

static void VirtMem_getInfo(VirtMem::Info& vmInfo) noexcept {
  uint32_t pageSize = uint32_t(::getpagesize());

  vmInfo.pageSize = pageSize;
  vmInfo.pageGranularity = Support::max<uint32_t>(pageSize, 65536);
}

// Posix specific implementation that uses `mmap()` and `munmap()`.
static int VirtMem_accessToPosixProtection(uint32_t flags) noexcept {
  int protection = 0;
  if (flags & VirtMem::kAccessRead   ) protection |= PROT_READ;
  if (flags & VirtMem::kAccessWrite  ) protection |= PROT_READ | PROT_WRITE;
  if (flags & VirtMem::kAccessExecute) protection |= PROT_READ | PROT_EXEC;
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

static const char* VirtMem_getTmpDir() noexcept {
  const char* tmpDir = getenv("TMPDIR");
  return tmpDir ? tmpDir : "/tmp";
}

static Error VirtMem_openAnonymousMemory(int* fd, bool preferTmpOverDevShm) noexcept {
#if defined(SYS_memfd_create)
  // Linux specific 'memfd_create' - if the syscall returns `ENOSYS` it means
  // it's not available and we will never call it again (would be pointless).
  if (!vm_memfd_create_not_supported) {
    *fd = (int)syscall(SYS_memfd_create, "vmem", 0);
    if (ASMJIT_LIKELY(*fd >= 0))
      return kErrorOk;

    int e = errno;
    if (e == ENOSYS)
      vm_memfd_create_not_supported = 1;
    else
      return DebugUtils::errored(VirtMem_makeErrorFromErrno(e));
  }
#endif

#if defined(SHM_ANON)
  // Originally FreeBSD extension, apparently works in other BSDs too.
  ASMJIT_UNUSED(preferTmpOverDevShm);
  *fd = shm_open(SHM_ANON, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);

  if (ASMJIT_LIKELY(*fd >= 0))
    return kErrorOk;
  else
    return DebugUtils::errored(VirtMem_makeErrorFromErrno(errno));
#else
  // POSIX API. We have to generate somehow a unique name. This is nothing
  // cryptographic, just using a bit from the stack address to always have
  // a different base for different threads (as threads have their own stack)
  // and retries for avoiding collisions. We use `shm_open()` with flags that
  // require creation of the file so we never open an existing shared memory.
  static std::atomic<uint32_t> internalCounter;

  StringTmp<128> uniqueName;
  const char* kShmFormat = "/shm-id-%08llX";

  uint32_t kRetryCount = 100;
  uint64_t bits = ((uintptr_t)(void*)&uniqueName) & 0x55555555u;

  for (uint32_t i = 0; i < kRetryCount; i++) {
    bits -= uint64_t(OSUtils::getTickCount()) * 773703683;
    bits = ((bits >> 14) ^ (bits << 6)) + uint64_t(++internalCounter) * 10619863;

    if (!ASMJIT_VM_SHM_DETECT || preferTmpOverDevShm) {
      uniqueName.assignString(VirtMem_getTmpDir());
      uniqueName.appendFormat(kShmFormat, (unsigned long long)bits);
      *fd = open(uniqueName.data(), O_RDWR | O_CREAT | O_EXCL, 0);
      if (ASMJIT_LIKELY(*fd >= 0)) {
        unlink(uniqueName.data());
        return kErrorOk;
      }
    }
    else {
      uniqueName.assignFormat(kShmFormat, (unsigned long long)bits);
      *fd = shm_open(uniqueName.data(), O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
      if (ASMJIT_LIKELY(*fd >= 0)) {
        shm_unlink(uniqueName.data());
        return kErrorOk;
      }
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

#if ASMJIT_VM_SHM_DETECT
static Error VirtMem_detectShmStrategy(uint32_t* strategyOut) noexcept {
  ScopedFD fd;
  VirtMem::Info vmInfo = VirtMem::info();

  ASMJIT_PROPAGATE(VirtMem_openAnonymousMemory(&fd.value, false));
  if (ftruncate(fd.value, off_t(vmInfo.pageSize)) != 0)
    return DebugUtils::errored(VirtMem_makeErrorFromErrno(errno));

  void* ptr = mmap(nullptr, vmInfo.pageSize, PROT_READ | PROT_EXEC, MAP_SHARED, fd.value, 0);
  if (ptr == MAP_FAILED) {
    int e = errno;
    if (e == EINVAL) {
      *strategyOut = kShmStrategyTmpDir;
      return kErrorOk;
    }
    return DebugUtils::errored(VirtMem_makeErrorFromErrno(e));
  }
  else {
    munmap(ptr, vmInfo.pageSize);
    *strategyOut = kShmStrategyDevShm;
    return kErrorOk;
  }
}
#endif

#if ASMJIT_VM_SHM_DETECT
static Error VirtMem_getShmStrategy(uint32_t* strategyOut) noexcept {
  uint32_t strategy = vm_shm_strategy;

  if (strategy == kShmStrategyUnknown) {
    ASMJIT_PROPAGATE(VirtMem_detectShmStrategy(&strategy));
    vm_shm_strategy = strategy;
  }

  *strategyOut = strategy;
  return kErrorOk;
}
#else
static Error VirtMem_getShmStrategy(uint32_t* strategyOut) noexcept {
  *strategyOut = kShmStrategyTmpDir;
  return kErrorOk;
}
#endif

Error VirtMem::alloc(void** p, size_t size, uint32_t flags) noexcept {
  *p = nullptr;

  if (size == 0)
    return DebugUtils::errored(kErrorInvalidArgument);

  int protection = VirtMem_accessToPosixProtection(flags);
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


Error VirtMem::protect(void* p, size_t size, uint32_t flags) noexcept {
  int protection = VirtMem_accessToPosixProtection(flags);
  if (mprotect(p, size, protection) == 0)
    return kErrorOk;

  return DebugUtils::errored(kErrorInvalidArgument);
}

Error VirtMem::allocDualMapping(DualMapping* dm, size_t size, uint32_t flags) noexcept {
  dm->ro = nullptr;
  dm->rw = nullptr;

  if (off_t(size) <= 0)
    return DebugUtils::errored(size == 0 ? kErrorInvalidArgument : kErrorTooLarge);

  bool preferTmpOverDevShm = (flags & kMappingPreferTmp) != 0;
  if (!preferTmpOverDevShm) {
    uint32_t strategy;
    ASMJIT_PROPAGATE(VirtMem_getShmStrategy(&strategy));
    preferTmpOverDevShm = (strategy == kShmStrategyTmpDir);
  }

  // ScopedFD will automatically close the file descriptor in its destructor.
  ScopedFD fd;
  ASMJIT_PROPAGATE(VirtMem_openAnonymousMemory(&fd.value, preferTmpOverDevShm));
  if (ftruncate(fd.value, off_t(size)) != 0)
    return DebugUtils::errored(VirtMem_makeErrorFromErrno(errno));

  void* ptr[2];
  for (uint32_t i = 0; i < 2; i++) {
    ptr[i] = mmap(nullptr, size, VirtMem_accessToPosixProtection(flags & ~dualMappingFilter[i]), MAP_SHARED, fd.value, 0);
    if (ptr[i] == MAP_FAILED) {
      // Get the error now before `munmap` has a chance to clobber it.
      int e = errno;
      if (i == 1)
        munmap(ptr[0], size);
      return DebugUtils::errored(VirtMem_makeErrorFromErrno(e));
    }
  }

  dm->ro = ptr[0];
  dm->rw = ptr[1];
  return kErrorOk;
}

Error VirtMem::releaseDualMapping(DualMapping* dm, size_t size) noexcept {
  Error err = release(dm->ro, size);
  if (dm->ro != dm->rw)
    err |= release(dm->rw, size);

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
