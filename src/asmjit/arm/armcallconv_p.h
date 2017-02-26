// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_ARM_ARMCALLCONV_P_H
#define _ASMJIT_ARM_ARMCALLCONV_P_H

#include "../core/callconv.h"

ASMJIT_BEGIN_NAMESPACE

//! \addtogroup asmjit_arm
//! \{

// ============================================================================
// [asmjit::ArmCallConvInternal]
// ============================================================================

//! \internal
//!
//! ARM-specific function API (calling conventions and other utilities).
namespace ArmCallConvInternal {

//! Initialize `CallConv` structure (ARM specific).
Error init(CallConv& cc, uint32_t ccId) noexcept;

} // ArmCallConvInternal

//! \}

ASMJIT_END_NAMESPACE

// [Guard]
#endif // _ASMJIT_ARM_ARMCALLCONV_P_H
