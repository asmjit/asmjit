// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Export]
#define ASMJIT_EXPORTS

// [Dependencies - AsmJit]
#include "../base/assembler.h"
#include "../base/runtime.h"

// TODO: Rename this, or make call conv independent of CompilerFunc.
#include "../base/compilerfunc.h"

// [Api-Begin]
#include "../apibegin.h"

namespace asmjit {

// ============================================================================
// [asmjit::Runtime - Utilities]
// ============================================================================

static ASMJIT_INLINE uint32_t hostStackAlignment() noexcept {
  // By default a pointer-size stack alignment is assumed.
  uint32_t alignment = sizeof(intptr_t);

  // ARM & ARM64
  // -----------
  //
  //   - 32-bit ARM requires stack to be aligned to 8 bytes.
  //   - 64-bit ARM requires stack to be aligned to 16 bytes.
#if ASMJIT_ARCH_ARM32 || ASMJIT_ARCH_ARM64
  alignment = ASMJIT_ARCH_ARM32 ? 8 : 16;
#endif

  // X86 & X64
  // ---------
  //
  //   - 32-bit X86 requires stack to be aligned to 4 bytes. Modern Linux, APPLE
  //     and UNIX guarantees 16-byte stack alignment even in 32-bit, but I'm
  //     not sure about all other UNIX operating systems, because 16-byte alignment
  //     is addition to an older specification.
  //   - 64-bit X86 requires stack to be aligned to 16 bytes.
#if ASMJIT_ARCH_X86 || ASMJIT_ARCH_X64
  int modernOS = ASMJIT_OS_LINUX  || // Linux & ANDROID.
                 ASMJIT_OS_MAC    || // OSX and iOS.
                 ASMJIT_OS_BSD;      // BSD variants.
  alignment = ASMJIT_ARCH_X64 || modernOS ? 16 : 4;
#endif

  return alignment;
}

static ASMJIT_INLINE void hostFlushInstructionCache(void* p, size_t size) noexcept {
  // Only useful on non-x86 architectures.
#if !ASMJIT_ARCH_X86 && !ASMJIT_ARCH_X64
# if ASMJIT_OS_WINDOWS
  // Windows has a built-in support in kernel32.dll.
  ::FlushInstructionCache(_memMgr.getProcessHandle(), p, size);
# endif // ASMJIT_OS_WINDOWS
#else
  ASMJIT_UNUSED(p);
  ASMJIT_UNUSED(size);
#endif // !ASMJIT_ARCH_X86 && !ASMJIT_ARCH_X64
}

// ============================================================================
// [asmjit::Runtime - Construction / Destruction]
// ============================================================================

Runtime::Runtime() noexcept
  : _runtimeType(kTypeNone),
    _allocType(kVMemAllocFreeable),
    _cpuInfo(),
    _stackAlignment(0),
    _cdeclConv(kCallConvNone),
    _stdCallConv(kCallConvNone),
    _baseAddress(kNoBaseAddress),
    _sizeLimit(0) {

  ::memset(_reserved, 0, sizeof(_reserved));
}
Runtime::~Runtime() noexcept {}

// ============================================================================
// [asmjit::HostRuntime - Construction / Destruction]
// ============================================================================

HostRuntime::HostRuntime() noexcept {
  _runtimeType = kTypeJit;
  _cpuInfo = CpuInfo::getHost();

  _stackAlignment = hostStackAlignment();
  _cdeclConv = kCallConvHostCDecl;
  _stdCallConv = kCallConvHostStdCall;
}
HostRuntime::~HostRuntime() noexcept {}

// ============================================================================
// [asmjit::HostRuntime - Interface]
// ============================================================================

void HostRuntime::flush(void* p, size_t size) noexcept {
  hostFlushInstructionCache(p, size);
}

// ============================================================================
// [asmjit::StaticRuntime - Construction / Destruction]
// ============================================================================

StaticRuntime::StaticRuntime(void* baseAddress, size_t sizeLimit) noexcept {
  _sizeLimit = sizeLimit;
  _baseAddress = static_cast<Ptr>((uintptr_t)baseAddress);
}
StaticRuntime::~StaticRuntime() {}

// ============================================================================
// [asmjit::StaticRuntime - Interface]
// ============================================================================

Error StaticRuntime::add(void** dst, Assembler* assembler) noexcept {
  size_t codeSize = assembler->getCodeSize();
  size_t sizeLimit = _sizeLimit;

  if (codeSize == 0) {
    *dst = nullptr;
    return kErrorNoCodeGenerated;
  }

  if (sizeLimit != 0 && sizeLimit < codeSize) {
    *dst = nullptr;
    return kErrorCodeTooLarge;
  }

  Ptr baseAddress = _baseAddress;
  uint8_t* p = static_cast<uint8_t*>((void*)static_cast<uintptr_t>(baseAddress));

  // Since the base address is known the `relocSize` returned should be equal
  // to `codeSize`. It's better to fail if they don't match instead of passsing
  // silently.
  size_t relocSize = assembler->relocCode(p, baseAddress);
  if (relocSize == 0 || codeSize != relocSize) {
    *dst = nullptr;
    return kErrorInvalidState;
  }

  _baseAddress += codeSize;
  if (sizeLimit)
    sizeLimit -= codeSize;

  flush(p, codeSize);
  *dst = p;

  return kErrorOk;
}

Error StaticRuntime::release(void* p) noexcept {
  // There is nothing to release as `StaticRuntime` doesn't manage any memory.
  ASMJIT_UNUSED(p);
  return kErrorOk;
}

// ============================================================================
// [asmjit::JitRuntime - Construction / Destruction]
// ============================================================================

JitRuntime::JitRuntime() noexcept {}
JitRuntime::~JitRuntime() noexcept {}

// ============================================================================
// [asmjit::JitRuntime - Interface]
// ============================================================================

Error JitRuntime::add(void** dst, Assembler* assembler) noexcept {
  size_t codeSize = assembler->getCodeSize();
  if (codeSize == 0) {
    *dst = nullptr;
    return kErrorNoCodeGenerated;
  }

  void* p = _memMgr.alloc(codeSize, getAllocType());
  if (p == nullptr) {
    *dst = nullptr;
    return kErrorNoVirtualMemory;
  }

  // Relocate the code and release the unused memory back to `VMemMgr`.
  size_t relocSize = assembler->relocCode(p);
  if (relocSize == 0) {
    *dst = nullptr;
    _memMgr.release(p);
    return kErrorInvalidState;
  }

  if (relocSize < codeSize)
    _memMgr.shrink(p, relocSize);

  flush(p, relocSize);
  *dst = p;

  return kErrorOk;
}

Error JitRuntime::release(void* p) noexcept {
  return _memMgr.release(p);
}

} // asmjit namespace

// [Api-End]
#include "../apiend.h"
