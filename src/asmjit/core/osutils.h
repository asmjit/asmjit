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
//!
//! Lock is internal, it cannot be used outside of AsmJit, however, its internal
//! layout is exposed as it's used by some other public classes.
class Lock {
public:
  ASMJIT_NONCOPYABLE(Lock)

#if defined(_WIN32)
#pragma pack(push, 8)
  struct ASMJIT_MAY_ALIAS Handle {
    void* DebugInfo;
    long LockCount;
    long RecursionCount;
    void* OwningThread;
    void* LockSemaphore;
    unsigned long* SpinCount;
  };
  Handle _handle;
#pragma pack(pop)
#elif !defined(__EMSCRIPTEN__)
  typedef pthread_mutex_t Handle;
  Handle _handle;
#endif

  inline Lock() noexcept;
  inline ~Lock() noexcept;

  inline void lock() noexcept;
  inline void unlock() noexcept;
};

#ifdef ASMJIT_EXPORTS
#if defined(_WIN32)

// Win32 implementation.
static_assert(sizeof(Lock::Handle) == sizeof(CRITICAL_SECTION), "asmjit::Lock::Handle layout must match CRITICAL_SECTION");
static_assert(alignof(Lock::Handle) == alignof(CRITICAL_SECTION), "asmjit::Lock::Handle alignment must match CRITICAL_SECTION");

inline Lock::Lock() noexcept { InitializeCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(&_handle)); }
inline Lock::~Lock() noexcept { DeleteCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(&_handle)); }
inline void Lock::lock() noexcept { EnterCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(&_handle)); }
inline void Lock::unlock() noexcept { LeaveCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(&_handle)); }

#elif !defined(__EMSCRIPTEN__)

// PThread implementation.
inline Lock::Lock() noexcept { pthread_mutex_init(&_handle, nullptr); }
inline Lock::~Lock() noexcept { pthread_mutex_destroy(&_handle); }
inline void Lock::lock() noexcept { pthread_mutex_lock(&_handle); }
inline void Lock::unlock() noexcept { pthread_mutex_unlock(&_handle); }

#else

// Dummy implementation - Emscripten or other unsupported platform.
inline Lock::Lock() noexcept {}
inline Lock::~Lock() noexcept {}
inline void Lock::lock() noexcept {}
inline void Lock::unlock() noexcept {}

#endif
#endif

//! \endcond

// ============================================================================
// [asmjit::LockGuard]
// ============================================================================

#ifdef ASMJIT_EXPORTS
//! \cond INTERNAL

//! Scoped lock.
struct LockGuard {
  ASMJIT_NONCOPYABLE(LockGuard)

  Lock& _target;

  inline LockGuard(Lock& target) noexcept : _target(target) { _target.lock(); }
  inline ~LockGuard() noexcept { _target.unlock(); }
};

//! \endcond
#endif

//! \}

ASMJIT_END_NAMESPACE

#endif // _ASMJIT_CORE_OSUTILS_H
