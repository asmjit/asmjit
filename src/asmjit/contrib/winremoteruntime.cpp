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
  _hProcess(hProcess),
  _memoryManager(hProcess) {

  // We are patching another process so enable keep-virtual-memory option.
  _memoryManager.setKeepVirtualMemory(true);
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
  void* codeData = ::malloc(codeSize);

  if (codeData == NULL) {
    *dest = NULL;
    return kErrorNoHeapMemory;
  }

  // Allocate a pernament remote process memory.
  void* processMemPtr = _memoryManager.alloc(codeSize, kVirtualAllocPermanent);

  if (processMemPtr == NULL) {
    ::free(codeData);
    *dest = NULL;
    return kErrorNoVirtualMemory;
  }

  // Relocate and write the code to the process memory.
  assembler->relocCode(codeData, (uintptr_t)processMemPtr);

  ::WriteProcessMemory(_hProcess, processMemPtr, codeData, codeSize, NULL);
  ::free(codeData);

  *dest = processMemPtr;
  return kErrorOk;
}

} // contrib namespace
} // asmjit namespace

// [Guard - Windows]
#endif // ASMJIT_OS_WINDOWS
