// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Export]
#define ASMJIT_EXPORTS

// [Dependencies - AsmJit]
#include "../base/intutil.h"
#include "../base/vmem.h"

// [Dependencies - Windows]
#if defined(ASMJIT_OS_WINDOWS)
# include <windows.h>
#endif // ASMJIT_OS_WINDOWS

// [Dependencies - Posix]
#if defined(ASMJIT_OS_POSIX)
# include <sys/types.h>
# include <sys/mman.h>
# include <unistd.h>
#endif // ASMJIT_OS_POSIX

// [Api-Begin]
#include "../base/apibegin.h"

namespace asmjit {

// ============================================================================
// [asmjit::VMem - Windows]
// ============================================================================

#if defined(ASMJIT_OS_WINDOWS)
struct VMemLocal {
  VMemLocal() {
    SYSTEM_INFO info;
    GetSystemInfo(&info);

    alignment = info.dwAllocationGranularity;
    pageSize = IntUtil::roundUpToPowerOf2<uint32_t>(info.dwPageSize);
  }

  size_t alignment;
  size_t pageSize;
};

static VMemLocal& vm() {
  static VMemLocal vm;
  return vm;
};

void* VMem::alloc(size_t length, size_t* allocated, bool canExecute) {
  return allocProcessMemory(GetCurrentProcess(), length, allocated, canExecute);
}

void VMem::release(void* addr, size_t length) {
  return releaseProcessMemory(GetCurrentProcess(), addr, length);
}

void* VMem::allocProcessMemory(HANDLE hProcess, size_t length, size_t* allocated, bool canExecute) {
  // VirtualAlloc rounds allocated size to page size automatically.
  size_t msize = IntUtil::roundUp(length, vm().pageSize);

  // Windows XP SP2 / Vista allow Data Excution Prevention (DEP).
  WORD protect = canExecute ? PAGE_EXECUTE_READWRITE : PAGE_READWRITE;
  LPVOID mbase = VirtualAllocEx(hProcess, NULL, msize, MEM_COMMIT | MEM_RESERVE, protect);
  if (mbase == NULL) return NULL;

  ASMJIT_ASSERT(IntUtil::isAligned<size_t>(reinterpret_cast<size_t>(mbase), vm().alignment));

  if (allocated != NULL)
    *allocated = msize;
  return mbase;
}

void VMem::releaseProcessMemory(HANDLE hProcess, void* addr, size_t /* length */) {
  VirtualFreeEx(hProcess, addr, 0, MEM_RELEASE);
}

size_t VMem::getAlignment() {
  return vm().alignment;
}

size_t VMem::getPageSize() {
  return vm().pageSize;
}
#endif // ASMJIT_OS_WINDOWS

// ============================================================================
// [asmjit::VMem - Posix]
// ============================================================================

#if defined(ASMJIT_OS_POSIX)

// MacOS uses MAP_ANON instead of MAP_ANONYMOUS.
#if !defined(MAP_ANONYMOUS)
# define MAP_ANONYMOUS MAP_ANON
#endif // MAP_ANONYMOUS

struct VMemLocal {
  VMemLocal() {
    alignment = pageSize = ::getpagesize();
  }

  size_t alignment;
  size_t pageSize;
};

static VMemLocal& vm() {
  static VMemLocal vm;
  return vm;
}

void* VMem::alloc(size_t length, size_t* allocated, bool canExecute) {
  size_t msize = IntUtil::roundUp<size_t>(length, vm().pageSize);
  int protection = PROT_READ | PROT_WRITE | (canExecute ? PROT_EXEC : 0);

  void* mbase = ::mmap(NULL, msize, protection, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (mbase == MAP_FAILED)
    return NULL;

  if (allocated != NULL)
    *allocated = msize;
  return mbase;
}

void VMem::release(void* addr, size_t length) {
  munmap(addr, length);
}

size_t VMem::getAlignment() {
  return vm().alignment;
}

size_t VMem::getPageSize() {
  return vm().pageSize;
}
#endif // ASMJIT_OS_POSIX

} // asmjit namespace

// [Api-End]
#include "../base/apiend.h"
