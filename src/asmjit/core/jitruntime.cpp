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
  #if !ASMJIT_ARCH_X86 && ASMJIT_OS_WINDOW
  // Windows has a built-in support in `kernel32.dll`.
  ::FlushInstructionCache(::GetCurrentProcess(), p, size);
  #else
  ASMJIT_UNUSED(p);
  ASMJIT_UNUSED(size);
  #endif
}

static inline uint32_t JitRuntime_detectNaturalStackAlignment() noexcept {
  // Alignment is assumed to match the pointer-size by default.
  uint32_t alignment = sizeof(intptr_t);

  // X86 Target
  // ----------
  //
  //   - 32-bit - stack must be aligned to at least 4 bytes. Modern Linux, Mac,
  //              and BSD guarantee 16-byte stack alignment even on 32-bit. I'm
  //              not sure about other unices as 16-byte alignment was a recent
  //              addition to the specification.
  //   - 64-bit - stack must be aligned to at least 16 bytes.
  #if ASMJIT_ARCH_X86
  unsigned int kIsModernOS = ASMJIT_OS_BSD    |
                             ASMJIT_OS_MAC    |
                             ASMJIT_OS_LINUX  ;
  alignment = (ASMJIT_ARCH_X86 == 64 || kIsModernOS != 0) ? 16 : 4;
  #endif

  // ARM Target
  // ----------
  //
  //   - 32-bit - Stack must be aligned to at least 8 bytes.
  //   - 64-bit - Stack must be aligned to at least 16 bytes (hardware requirement).
  #if ASMJIT_ARCH_ARM
  alignment = (ASMJIT_ARCH_ARM == 32) ? 8 : 16;
  #endif

  return alignment;
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
