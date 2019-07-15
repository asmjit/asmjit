// [AsmJit]
// Machine Code Generation for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

#ifndef _ASMJIT_CORE_OSUTILS_H
#define _ASMJIT_CORE_OSUTILS_H

#include "../core/globals.h"

ASMJIT_BEGIN_NAMESPACE

//! \addtogroup asmjit_support
//! \{

// ============================================================================
// [asmjit::OSUtils]
// ============================================================================

//! Operating system utilities.
namespace OSUtils {
  //! Gets the current CPU tick count, used for benchmarking (1ms resolution).
  ASMJIT_API uint32_t getTickCount() noexcept;
};

// ============================================================================
// [asmjit::Lock]
// ============================================================================

//! \cond INTERNAL

//! Lock.
class Lock {
public:
  ASMJIT_NONCOPYABLE(Lock)

  #if defined(_WIN32)

  typedef CRITICAL_SECTION Handle;
  Handle _handle;

  inline Lock() noexcept { InitializeCriticalSection(&_handle); }
  inline ~Lock() noexcept { DeleteCriticalSection(&_handle); }

  inline void lock() noexcept { EnterCriticalSection(&_handle); }
  inline void unlock() noexcept { LeaveCriticalSection(&_handle); }

  #elif !defined(__EMSCRIPTEN__)

  typedef pthread_mutex_t Handle;
  Handle _handle;

  inline Lock() noexcept { pthread_mutex_init(&_handle, nullptr); }
  inline ~Lock() noexcept { pthread_mutex_destroy(&_handle); }

  inline void lock() noexcept { pthread_mutex_lock(&_handle); }
  inline void unlock() noexcept { pthread_mutex_unlock(&_handle); }

  #else

  // Browser or other unsupported OS.
  inline Lock() noexcept {}
  inline ~Lock() noexcept {}

  inline void lock() noexcept {}
  inline void unlock() noexcept {}

  #endif
};

//! \endcond

// ============================================================================
// [asmjit::ScopedLock]
// ============================================================================

//! \cond INTERNAL

//! Scoped lock.
struct ScopedLock {
  ASMJIT_NONCOPYABLE(ScopedLock)

  Lock& _target;

  inline ScopedLock(Lock& target) noexcept : _target(target) { _target.lock(); }
  inline ~ScopedLock() noexcept { _target.unlock(); }
};

//! \endcond

//! \}

ASMJIT_END_NAMESPACE

#endif // _ASMJIT_CORE_OSUTILS_H
