// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// ZLIB - See LICENSE.md file in the package.

// [Export]
#define ASMJIT_EXPORTS

// [Dependencies]
#include "../core/cpuinfo.h"

#if ASMJIT_OS_POSIX
  #include <errno.h>
  #include <sys/utsname.h>
  #include <unistd.h>
#endif

ASMJIT_BEGIN_NAMESPACE

// ============================================================================
// [asmjit::CpuInfo - Detect - CPU NumThreads]
// ============================================================================

#if ASMJIT_OS_WINDOWS
static inline uint32_t detectHWThreadCount() noexcept {
  SYSTEM_INFO info;
  ::GetSystemInfo(&info);
  return info.dwNumberOfProcessors;
}
#elif ASMJIT_OS_POSIX && defined(_SC_NPROCESSORS_ONLN)
static inline uint32_t detectHWThreadCount() noexcept {
  long res = ::sysconf(_SC_NPROCESSORS_ONLN);
  return res <= 0 ? uint32_t(1) : uint32_t(res);
}
#else
static inline uint32_t detectHWThreadCount() noexcept {
  return 1;
}
#endif

// ============================================================================
// [asmjit::CpuInfo - Detect - CPU Features]
// ============================================================================

#if defined(ASMJIT_BUILD_X86) && ASMJIT_ARCH_X86
namespace x86 { void detectCpu(CpuInfo& cpu) noexcept; }
#endif

#if defined(ASMJIT_BUILD_ARM) && ASMJIT_ARCH_ARM
namespace arm { void detectCpu(CpuInfo& cpu) noexcept; }
#endif

// ============================================================================
// [asmjit::CpuInfo - Detect - Static Initializer]
// ============================================================================

struct HostCpuInfo : public CpuInfo {
  inline HostCpuInfo() noexcept : CpuInfo() {
    #if defined(ASMJIT_BUILD_X86) && ASMJIT_ARCH_X86
    x86::detectCpu(*this);
    #endif

    #if defined(ASMJIT_BUILD_ARM) && ASMJIT_ARCH_ARM
    arm::detectCpu(*this);
    #endif

    _hwThreadCount = detectHWThreadCount();
  }
};

const CpuInfo& CpuInfo::host() noexcept {
  static HostCpuInfo host;
  return host;
}

ASMJIT_END_NAMESPACE
