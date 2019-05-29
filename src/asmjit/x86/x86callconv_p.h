// [AsmJit]
// Machine Code Generation for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

#ifndef _ASMJIT_X86_X86CALLCONV_P_H
#define _ASMJIT_X86_X86CALLCONV_P_H

#include "../core/callconv.h"

ASMJIT_BEGIN_SUB_NAMESPACE(x86)

//! \cond INTERNAL
//! \addtogroup asmjit_x86
//! \{

// ============================================================================
// [asmjit::x86::CallConvInternal]
// ============================================================================

//! X86-specific function API (calling conventions and other utilities).
namespace CallConvInternal {
  //! Initialize `CallConv` structure (X86 specific).
  Error init(CallConv& cc, uint32_t ccId) noexcept;
}

//! \}
//! \endcond

ASMJIT_END_SUB_NAMESPACE

#endif // _ASMJIT_X86_X86CALLCONV_P_H
