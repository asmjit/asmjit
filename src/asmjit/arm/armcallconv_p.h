// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// ZLIB - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_ARM_ARMCALLCONV_P_H
#define _ASMJIT_ARM_ARMCALLCONV_P_H

#include "../core/callconv.h"

ASMJIT_BEGIN_SUB_NAMESPACE(arm)

//! \addtogroup asmjit_arm_api
//! \{

// ============================================================================
// [asmjit::arm::CallConvInternal]
// ============================================================================

//! \internal
//!
//! ARM-specific function API (calling conventions and other utilities).
namespace CallConvInternal {
  //! Initialize `CallConv` structure (ARM specific).
  Error init(CallConv& cc, uint32_t ccId) noexcept;
}

//! \}

ASMJIT_END_SUB_NAMESPACE

// [Guard]
#endif // _ASMJIT_ARM_ARMCALLCONV_P_H
