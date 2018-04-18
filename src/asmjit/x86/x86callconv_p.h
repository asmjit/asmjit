// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// ZLIB - See LICENSE.md file in the package.

#ifndef _ASMJIT_X86_X86CALLCONV_P_H
#define _ASMJIT_X86_X86CALLCONV_P_H

#include "../core/callconv.h"

ASMJIT_BEGIN_SUB_NAMESPACE(x86)

//! \addtogroup asmjit_x86_api
//! \{

// ============================================================================
// [asmjit::x86::CallConvInternal]
// ============================================================================

//! \internal
//!
//! X86-specific function API (calling conventions and other utilities).
namespace CallConvInternal {
  //! Initialize `CallConv` structure (X86 specific).
  Error init(CallConv& cc, uint32_t ccId) noexcept;
}

//! \}

ASMJIT_END_SUB_NAMESPACE

// [Guard]
#endif // _ASMJIT_X86_X86CALLCONV_P_H
