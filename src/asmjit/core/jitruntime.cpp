// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// ZLIB - See LICENSE.md file in the package.

#define ASMJIT_EXPORTS

#include "../core/build.h"
#ifndef ASMJIT_DISABLE_JIT

#include "../core/cpuinfo.h"
#include "../core/jitruntime.h"

ASMJIT_BEGIN_NAMESPACE

// ============================================================================
// [asmjit::JitRuntime - Utilities]
// ============================================================================

static inline void JitRuntime_flushInstructionCache(const void* p, size_t size) noexcept {
  // Only useful on non-x86 architectures.
  #if defined(_WIN32) && !ASMJIT_ARCH_X86
  // Windows has a built-in support in `kernel32.dll`.
  ::FlushInstructionCache(::GetCurrentProcess(), p, size);
  #else
  ASMJIT_UNUSED(p);
  ASMJIT_UNUSED(size);
  #endif
}

// X86 Target
// ----------
//
//   - 32-bit - Linux, OSX, BSD, and apparently also Haiku guarantee 16-byte
//              stack alignment. Other operating systems are assumed to have
//              4-byte alignment by default for safety reasons.
//   - 64-bit - stack must be aligned to 16 bytes.
//
// ARM Target
// ----------
//
//   - 32-bit - Stack must be aligned to 8 bytes.
//   - 64-bit - Stack must be aligned to 16 bytes (hardware requirement).
static inline uint32_t JitRuntime_detectNaturalStackAlignment() noexcept {
#if ASMJIT_ARCH_BITS == 64 || \
    defined(__APPLE__    ) || \
    defined(__bsdi__     ) || \
    defined(__DragonFly__) || \
    defined(__HAIKU__    ) || \
    defined(__FreeBSD__  ) || \
    defined(__NetBSD__   ) || \
    defined(__OpenBSD__  ) || \
    defined(__linux__    )
  return 16;
#elif ASMJIT_ARCH_ARM
  return 8;
#else
  return uint32_t(sizeof(uintptr_t));
#endif
}

// ============================================================================
// [asmjit::JitRuntime - Construction / Destruction]
// ============================================================================

JitRuntime::JitRuntime() noexcept {
  // Setup target properties.
  _targetType = kTargetJit;
  _codeInfo._archInfo       = CpuInfo::host().archInfo();
  _codeInfo._stackAlignment = uint8_t(JitRuntime_detectNaturalStackAlignment());
  _codeInfo._cdeclCallConv  = CallConv::kIdHostCDecl;
  _codeInfo._stdCallConv    = CallConv::kIdHostStdCall;
  _codeInfo._fastCallConv   = CallConv::kIdHostFastCall;
}
JitRuntime::~JitRuntime() noexcept {}

// ============================================================================
// [asmjit::JitRuntime - Interface]
// ============================================================================

Error JitRuntime::_add(void** dst, CodeHolder* code) noexcept {
  size_t codeSize = code->codeSize();
  if (ASMJIT_UNLIKELY(codeSize == 0)) {
    *dst = nullptr;
    return DebugUtils::errored(kErrorNoCodeGenerated);
  }

  void* p = _allocator.alloc(codeSize);
  if (ASMJIT_UNLIKELY(!p)) {
    *dst = nullptr;
    return DebugUtils::errored(kErrorNoVirtualMemory);
  }

  // Relocate the code and release the unused memory back to `JitAllocator`.
  size_t relocSize = code->relocate(p);
  if (ASMJIT_UNLIKELY(relocSize == 0)) {
    *dst = nullptr;
    _allocator.release(p);
    return DebugUtils::errored(kErrorInvalidState);
  }

  if (relocSize < codeSize)
    _allocator.shrink(p, relocSize);

  flush(p, relocSize);
  *dst = p;

  return kErrorOk;
}

Error JitRuntime::_release(void* p) noexcept {
  return _allocator.release(p);
}

void JitRuntime::flush(const void* p, size_t size) noexcept {
  JitRuntime_flushInstructionCache(p, size);
}

ASMJIT_END_NAMESPACE

#endif
