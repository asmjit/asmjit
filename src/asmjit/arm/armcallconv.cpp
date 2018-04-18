// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// ZLIB - See LICENSE.md file in the package.

// [Export]
#define ASMJIT_EXPORTS

// [Guard]
#include "../core/build.h"
#ifdef ASMJIT_BUILD_ARM

// [Dependencies]
#include "../arm/armcallconv_p.h"

ASMJIT_BEGIN_SUB_NAMESPACE(arm)

// ============================================================================
// [asmjit::ArmCallConvInternal - Init]
// ============================================================================

ASMJIT_FAVOR_SIZE Error ArmCallConvInternal::init(CallConv& cc, uint32_t ccId) noexcept {
  // TODO:
  return DebugUtils::errored(kErrorNotInitialized);
}

ASMJIT_END_SUB_NAMESPACE

// [Guard]
#endif // ASMJIT_BUILD_ARM
