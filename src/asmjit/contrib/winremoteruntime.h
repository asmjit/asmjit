// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_CONTRIB_WINREMOTERUNTIME_H
#define _ASMJIT_CONTRIB_WINREMOTERUNTIME_H

// [Dependencies]
#include "../base.h"

// [Guard - Windows]
#if defined(ASMJIT_OS_WINDOWS)

namespace asmjit {
namespace contrib {

// ============================================================================
// [asmjit::contrib::WinRemoteRuntime]
// ============================================================================

//! @brief WinRemoteRuntime can be used to inject code to a remote process.
struct WinRemoteRuntime : public BaseRuntime {
  ASMJIT_NO_COPY(WinRemoteRuntime)

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------
  //! @brief Create a @c WinRemoteRuntime instance for a given @a hProcess.
  ASMJIT_API WinRemoteRuntime(HANDLE hProcess);

  //! @brief Destroy the @c WinRemoteRuntime instance.
  ASMJIT_API virtual ~WinRemoteRuntime();

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! @brief Get the remote process handle.
  ASMJIT_INLINE HANDLE getProcess() const { return _hProcess; }

  //! @brief Get the virtual memory manager.
  ASMJIT_INLINE VirtualMemoryManager* getMemoryManager() { return &_memoryManager; }

  // --------------------------------------------------------------------------
  // [Interface]
  // --------------------------------------------------------------------------

  ASMJIT_API virtual uint32_t add(void** dest, BaseAssembler* assembler);

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! @brief Process.
  HANDLE _hProcess;
  
  //! @brief Virtual memory manager.
  VirtualMemoryManager _memoryManager;
};

} // contrib namespace
} // asmjit namespace

// [Guard - Windows]
#endif // ASMJIT_OS_WINDOWS

// [Guard]
#endif // _ASMJIT_CONTRIB_WINREMOTERUNTIME_H
