// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Export]
#define ASMJIT_EXPORTS

// [Guard]
#include "../core/build.h"
#ifdef ASMJIT_BUILD_ARM

// [Dependencies]
#include "../arm/armcallconv_p.h"

ASMJIT_BEGIN_NAMESPACE

// ============================================================================
// [asmjit::ArmCallConvInternal - Init]
// ============================================================================

ASMJIT_FAVOR_SIZE Error ArmCallConvInternal::init(CallConv& cc, uint32_t ccId) noexcept {
  // TODO:
  return DebugUtils::errored(kErrorNotInitialized);
}

ASMJIT_END_NAMESPACE

// [Guard]
#endif // ASMJIT_BUILD_ARM
