// [AsmJit/WinRemoteRuntime]
// Contribution for remote process handling.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Export]
#define ASMJIT_EXPORTS

// [Dependencies - AsmJit]
#include "../base.h"

// [Guard - Windows]
#if defined(ASMJIT_OS_WINDOWS)
#include "winremoteruntime.h"

namespace asmjit {
namespace contrib {

// ============================================================================
// [asmjit::contrib::WinRemoteRuntime - Construction / Destruction]
// ============================================================================

WinRemoteRuntime::WinRemoteRuntime(HANDLE hProcess) :
  _memMgr(hProcess) {

  // We are patching another process so enable keep-virtual-memory option.
  _memMgr.setKeepVirtualMemory(true);
}

WinRemoteRuntime::~WinRemoteRuntime() {}

// ============================================================================
// [asmjit::contrib::WinRemoteRuntime - Interface]
// ============================================================================

uint32_t WinRemoteRuntime::add(void** dest, BaseAssembler* assembler) {
  // Disallow generation of no code.
  size_t codeSize = assembler->getCodeSize();
  if (codeSize == 0) {
    *dest = NULL;
    return kErrorInvalidState;
  }

  // Allocate temporary memory where the code will be stored and relocated.
  void* codeData = ASMJIT_ALLOC(codeSize);
  if (codeData == NULL) {
    *dest = NULL;
    return kErrorNoHeapMemory;
  }

  // Allocate a permanent remote process memory.
  void* processMemPtr = _memMgr.alloc(codeSize, kVMemAllocPermanent);
  if (processMemPtr == NULL) {
    ASMJIT_FREE(codeData);
    *dest = NULL;
    return kErrorNoVirtualMemory;
  }

  // Relocate and write the code to the process memory.
  assembler->relocCode(codeData, (uintptr_t)processMemPtr);

  ::WriteProcessMemory(getProcessHandle(), processMemPtr, codeData, codeSize, NULL);
  ASMJIT_FREE(codeData);

  *dest = processMemPtr;
  return kErrorOk;
}

// NOP.
Error WinRemoteRuntime::release(void* p) {
  return kErrorOk;
}

} // contrib namespace
} // asmjit namespace

// [Guard - Windows]
#endif // ASMJIT_OS_WINDOWS
