// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// ZLIB - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_CORE_OSUTILS_H
#define _ASMJIT_CORE_OSUTILS_H

// [Dependencies]
#include "../core/globals.h"

ASMJIT_BEGIN_NAMESPACE

//! \addtogroup asmjit_core_support
//! \{

// ============================================================================
// [asmjit::OSUtils]
// ============================================================================

//! Operating system utilities.
namespace OSUtils {
  //! Get the current CPU tick count, used for benchmarking (1ms resolution).
  ASMJIT_API uint32_t getTickCount() noexcept;
};

// ============================================================================
// [asmjit::Lock]
// ============================================================================

//! \internal
//!
//! Lock.
class Lock {
public:
  ASMJIT_NONCOPYABLE(Lock)

  #if ASMJIT_OS_WINDOWS

  typedef CRITICAL_SECTION Handle;

  inline Lock() noexcept { InitializeCriticalSection(&_handle); }
  inline ~Lock() noexcept { DeleteCriticalSection(&_handle); }

  inline void lock() noexcept { EnterCriticalSection(&_handle); }
  inline void unlock() noexcept { LeaveCriticalSection(&_handle); }

  Handle _handle;

  #elif ASMJIT_OS_POSIX && !defined(__EMSCRIPTEN__)

  typedef pthread_mutex_t Handle;

  inline Lock() noexcept { pthread_mutex_init(&_handle, nullptr); }
  inline ~Lock() noexcept { pthread_mutex_destroy(&_handle); }

  inline void lock() noexcept { pthread_mutex_lock(&_handle); }
  inline void unlock() noexcept { pthread_mutex_unlock(&_handle); }

  Handle _handle;

  #else

  #if !defined(__EMSCRIPTEN__)
  #pragma message("asmjit::Lock doesn't have implementation for your target OS.")
  #endif

  // Browser or other unsupported OS.
  inline Lock() noexcept {}
  inline ~Lock() noexcept {}

  inline void lock() noexcept {}
  inline void unlock() noexcept {}

  #endif
};

// ============================================================================
// [asmjit::ScopedLock]
// ============================================================================

//! \internal
//!
//! Scoped lock.
struct ScopedLock {
  ASMJIT_NONCOPYABLE(ScopedLock)

  inline ScopedLock(Lock& target) noexcept : _target(target) { _target.lock(); }
  inline ~ScopedLock() noexcept { _target.unlock(); }

  Lock& _target;
};

//! \}

ASMJIT_END_NAMESPACE

// [Guard]
#endif // _ASMJIT_CORE_OSUTILS_H
