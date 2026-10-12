// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_CORE_OS_UTILS_P_H_INCLUDED
#define ASMJIT_CORE_OS_UTILS_P_H_INCLUDED

#include <asmjit/core/os_utils.h>
#include <asmjit/core/string.h>

#if defined(_WIN32)
  #include <windows.h>
#endif

ASMJIT_BEGIN_NAMESPACE

//! \cond INTERNAL
//! \addtogroup asmjit_core
//! \{

#if defined(_WIN32)

// Windows implementation.
static_assert(sizeof(Lock::Handle) == sizeof(SRWLOCK), "asmjit::Lock::Handle layout must match SRWLOCK");
static_assert(alignof(Lock::Handle) == alignof(SRWLOCK), "asmjit::Lock::Handle alignment must match SRWLOCK");

ASMJIT_INLINE_NODEBUG Lock::Lock() noexcept : _handle{} {} // Clearing the internal struct to zero is how it's initialized.
ASMJIT_INLINE_NODEBUG Lock::~Lock() noexcept {}
ASMJIT_INLINE_NODEBUG void Lock::lock() noexcept { AcquireSRWLockExclusive(reinterpret_cast<SRWLOCK*>(&_handle)); }
ASMJIT_INLINE_NODEBUG void Lock::unlock() noexcept { ReleaseSRWLockExclusive(reinterpret_cast<SRWLOCK*>(&_handle)); }

#elif !defined(__EMSCRIPTEN__)

// PThread implementation.
#ifdef PTHREAD_MUTEX_INITIALIZER
ASMJIT_INLINE_NODEBUG Lock::Lock() noexcept : _handle(PTHREAD_MUTEX_INITIALIZER) {}
#else
ASMJIT_INLINE_NODEBUG Lock::Lock() noexcept { pthread_mutex_init(&_handle, nullptr); }
#endif
ASMJIT_INLINE_NODEBUG Lock::~Lock() noexcept { pthread_mutex_destroy(&_handle); }
ASMJIT_INLINE_NODEBUG void Lock::lock() noexcept { pthread_mutex_lock(&_handle); }
ASMJIT_INLINE_NODEBUG void Lock::unlock() noexcept { pthread_mutex_unlock(&_handle); }

#else

// Dummy implementation - Emscripten or other unsupported platform.
ASMJIT_INLINE_NODEBUG Lock::Lock() noexcept {}
ASMJIT_INLINE_NODEBUG Lock::~Lock() noexcept {}
ASMJIT_INLINE_NODEBUG void Lock::lock() noexcept {}
ASMJIT_INLINE_NODEBUG void Lock::unlock() noexcept {}

#endif

//! Scoped lock.
class LockGuard {
public:
  ASMJIT_NONCOPYABLE(LockGuard)

  Lock& _target;

  ASMJIT_INLINE_NODEBUG LockGuard(Lock& target) noexcept
    : _target(target) { _target.lock(); }
  ASMJIT_INLINE_NODEBUG ~LockGuard() noexcept { _target.unlock(); }
};

namespace OSUtils {

#if !defined(_WIN32)
//! Reads a file, only used on non-Windows platforms to access /sys or other files when necessary.
Error read_file(const char* name, String& dst, size_t max_size) noexcept;
#endif

//! Returns `[start, end)` spanned by the loaded module (executable or shared library) that contains `addr`.
//!
//! \note Only implemented on Linux at the moment, other platforms return \ref Error::kFeatureNotEnabled.
Error module_address_range(const void* addr, uintptr_t* start, uintptr_t* end) noexcept;

} // {OSUtils}

//! \}
//! \endcond

ASMJIT_END_NAMESPACE

#endif // ASMJIT_AXL_OS_UTILS_P_H_INCLUDED
