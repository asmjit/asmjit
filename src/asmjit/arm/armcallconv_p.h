// [AsmJit]
// Machine Code Generation for C++.
//
// [License]
// ZLIB - See LICENSE.md file in the package.

#ifndef _ASMJIT_ARM_ARMCALLCONV_P_H
#define _ASMJIT_ARM_ARMCALLCONV_P_H

#include "../core/callconv.h"

ASMJIT_BEGIN_SUB_NAMESPACE(arm)

//! \cond INTERNAL
//! \addtogroup asmjit_arm_api
//! \{

// ============================================================================
// [asmjit::arm::CallConvInternal]
// ============================================================================

//! ARM-specific function API (calling conventions and other utilities).
namespace CallConvInternal {
  //! Initialize `CallConv` structure (ARM specific).
  Error init(CallConv& cc, uint32_t ccId) noexcept;
}

//! \}
//! \endcond

ASMJIT_END_SUB_NAMESPACE

#endif // _ASMJIT_ARM_ARMCALLCONV_P_H
