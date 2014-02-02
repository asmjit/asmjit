// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Export]
#define ASMJIT_EXPORTS

// [Dependencies - AsmJit]
#include "../base/cpu.h"

#if defined(ASMJIT_HOST_X86) || defined(ASMJIT_HOST_X64)
#include "../x86/x86cpu.h"
#else
// ?
#endif // ASMJIT_HOST || ASMJIT_HOST_X64

// [Dependencies - Windows]
#if defined(ASMJIT_OS_WINDOWS)
# include <windows.h>
#endif // ASMJIT_OS_WINDOWS

// [Dependencies - Posix]
#if defined(ASMJIT_OS_POSIX)
# include <errno.h>
# include <sys/statvfs.h>
# include <sys/utsname.h>
# include <unistd.h>
#endif // ASMJIT_OS_POSIX

// [Api-Begin]
#include "../base/apibegin.h"

namespace asmjit {

// ============================================================================
// [asmjit::BaseCpu - DetectNumberOfCores]
// ============================================================================

uint32_t BaseCpu::detectNumberOfCores() {
#if defined(ASMJIT_OS_WINDOWS)
  SYSTEM_INFO info;
  ::GetSystemInfo(&info);
  return info.dwNumberOfProcessors;
#elif defined(ASMJIT_OS_POSIX) && defined(_SC_NPROCESSORS_ONLN)
  // It seems that sysconf returns the number of "logical" processors on both
  // mac and linux.  So we get the number of "online logical" processors.
  long res = ::sysconf(_SC_NPROCESSORS_ONLN);
  if (res == -1) return 1;

  return static_cast<uint32_t>(res);
#else
  return 1;
#endif
}

// ============================================================================
// [asmjit::BaseCpu - GetHost]
// ============================================================================

#if defined(ASMJIT_HOST_X86) || defined(ASMJIT_HOST_X64)
struct HostCpu : public x86x64::Cpu {
  ASMJIT_INLINE HostCpu() : Cpu() { hostCpuDetect(this); }
};
#else
#error "asmjit/base/cpu.cpp - Unsupported CPU."
#endif // ASMJIT_HOST || ASMJIT_HOST_X64

const BaseCpu* BaseCpu::getHost()
{
#if defined(ASMJIT_HOST_X86) || defined(ASMJIT_HOST_X64)
  static HostCpu cpu;
#else
#error "asmjit/base/cpu.cpp - Unsupported CPU."
#endif // ASMJIT_HOST || ASMJIT_HOST_X64
  return &cpu;
}

} // AsmJit

// [Api-End]
#include "../base/apiend.h"
