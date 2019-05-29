// [AsmJit]
// Machine Code Generation for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

#define ASMJIT_EXPORTS

#include "../core/cpuinfo.h"

#if !defined(_WIN32)
  #include <errno.h>
  #include <sys/utsname.h>
  #include <unistd.h>
#endif

ASMJIT_BEGIN_NAMESPACE

// ============================================================================
// [asmjit::CpuInfo - Detect - CPU NumThreads]
// ============================================================================

#if defined(_WIN32)
static inline uint32_t detectHWThreadCount() noexcept {
  SYSTEM_INFO info;
  ::GetSystemInfo(&info);
  return info.dwNumberOfProcessors;
}
#elif defined(_SC_NPROCESSORS_ONLN)
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

static uint32_t cpuInfoInitialized;
static CpuInfo cpuInfoGlobal(Globals::NoInit);

const CpuInfo& CpuInfo::host() noexcept {
  // This should never cause a problem as the resulting information should
  // always be the same.
  if (!cpuInfoInitialized) {
    CpuInfo cpuInfoLocal;

    #if defined(ASMJIT_BUILD_X86) && ASMJIT_ARCH_X86
    x86::detectCpu(cpuInfoLocal);
    #endif

    #if defined(ASMJIT_BUILD_ARM) && ASMJIT_ARCH_ARM
    arm::detectCpu(cpuInfoLocal);
    #endif

    cpuInfoLocal._hwThreadCount = detectHWThreadCount();
    cpuInfoGlobal = cpuInfoLocal;
    cpuInfoInitialized = 1;
  }

  return cpuInfoGlobal;
}

ASMJIT_END_NAMESPACE
