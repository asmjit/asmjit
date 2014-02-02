// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Export]
#define ASMJIT_EXPORTS

// [Dependencies - AsmJit]
#include "../base/assembler.h"
#include "../base/cpu.h"
#include "../base/defs.h"
#include "../base/error.h"
#include "../base/memorymanager.h"
#include "../base/runtime.h"

// [Api-Begin]
#include "../base/apibegin.h"

namespace asmjit {

// ============================================================================
// [asmjit::BaseRuntime - Construction / Destruction]
// ============================================================================

BaseRuntime::BaseRuntime() {}
BaseRuntime::~BaseRuntime() {}

// ============================================================================
// [asmjit::JitRuntime - Construction / Destruction]
// ============================================================================

JitRuntime::JitRuntime(MemoryManager* memmgr) :
  _memoryManager(memmgr ? memmgr : MemoryManager::getGlobal()),
  _allocType(kVirtualAllocFreeable) {}

JitRuntime::~JitRuntime() {}

// ============================================================================
// [asmjit::JitRuntime - Get]
// ============================================================================

uint32_t JitRuntime::getStackAlignment() {
  uint32_t alignment = sizeof(intptr_t);

#if defined(ASMJIT_HOST_X86)
  // Modern Linux, APPLE and UNIX guarantees 16-byte stack alignment, but I'm
  // not sure about all other UNIX operating systems, because 16-byte alignment
  // is addition to an older specification.
#if (defined(__linux__)   || \
     defined(__linux)     || \
     defined(__unix__)    || \
     defined(__FreeBSD__) || \
     defined(__NetBSD__)  || \
     defined(__OpenBSD__) || \
     defined(__DARWIN__)  || \
     defined(__APPLE__)   )
  alignment = 16;
#endif
#elif defined(ASMJIT_HOST_X64)
  alignment = 16;
#endif

  return alignment;
}

const BaseCpu* JitRuntime::getCpu() {
  return BaseCpu::getHost();
}

// ============================================================================
// [asmjit::JitRuntime - Add]
// ============================================================================

Error JitRuntime::add(void** dst, BaseAssembler* assembler) {
  // Disallow empty code generation.
  size_t codeSize = assembler->getCodeSize();

  if (codeSize == 0) {
    *dst = NULL;
    return kErrorCompilerNoFunc;
  }

  MemoryManager* memmgr = getMemoryManager();
  void* p = memmgr->alloc(codeSize, getAllocType());

  if (p == NULL) {
    *dst = NULL;
    return kErrorNoVirtualMemory;
  }

  // Relocate the code.
  size_t relocSize = assembler->relocCode(p);

  // Return unused memory to MemoryManager.
  if (relocSize < codeSize)
    memmgr->shrink(p, relocSize);

  // Return the code.
  *dst = p;
  return kErrorOk;
}

Error JitRuntime::release(void* p) {
  MemoryManager* memmgr = getMemoryManager();
  return memmgr->release(p);
}

} // asmjit namespace

// [Api-End]
#include "../base/apiend.h"
