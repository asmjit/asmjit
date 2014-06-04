// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_CONTRIB_WINREMOTERUNTIME_H
#define _ASMJIT_CONTRIB_WINREMOTERUNTIME_H

#include "../base.h"
#if defined(ASMJIT_OS_WINDOWS)

namespace asmjit {
namespace contrib {

//! \addtogroup asmjit_contrib
//! \{

// ============================================================================
// [asmjit::contrib::WinRemoteRuntime]
// ============================================================================

//! WinRemoteRuntime can be used to inject code to a remote process.
struct WinRemoteRuntime : public Runtime {
  ASMJIT_NO_COPY(WinRemoteRuntime)

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Create a `WinRemoteRuntime` instance for a given `hProcess`.
  ASMJIT_API WinRemoteRuntime(HANDLE hProcess);

  //! Destroy the `WinRemoteRuntime` instance.
  ASMJIT_API virtual ~WinRemoteRuntime();

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! Get the remote process handle.
  ASMJIT_INLINE HANDLE getProcessHandle() const {
    return _memMgr.getProcessHandle();
  }

  //! Get the remote memory manager.
  ASMJIT_INLINE VMemMgr* getMemMgr() const {
    return const_cast<VMemMgr*>(&_memMgr);
  }

  // --------------------------------------------------------------------------
  // [Interface]
  // --------------------------------------------------------------------------

  ASMJIT_API virtual uint32_t add(void** dest, BaseAssembler* assembler);
  ASMJIT_API virtual Error release(void* p);

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! Remove memory manager.
  VMemMgr _memMgr;
};

//! \}

} // contrib namespace
} // asmjit namespace

// [Guard]
#endif // ASMJIT_OS_WINDOWS
#endif // _ASMJIT_CONTRIB_WINREMOTERUNTIME_H
