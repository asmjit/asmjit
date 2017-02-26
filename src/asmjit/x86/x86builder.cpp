// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Export]
#define ASMJIT_EXPORTS

// [Guard]
#include "../core/build.h"
#if defined(ASMJIT_BUILD_X86) && !defined(ASMJIT_DISABLE_COMPILER)

// [Dependencies]
#include "../x86/x86assembler.h"
#include "../x86/x86builder.h"

ASMJIT_BEGIN_NAMESPACE

// ============================================================================
// [asmjit::X86Builder - Construction / Destruction]
// ============================================================================

X86Builder::X86Builder(CodeHolder* code) noexcept : CodeBuilder() {
  if (code)
    code->attach(this);
}
X86Builder::~X86Builder() noexcept {}

// ============================================================================
// [asmjit::X86Builder - Finalize]
// ============================================================================

Error X86Builder::finalize() {
  ASMJIT_PROPAGATE(runPasses());

  X86Assembler a(_code);
  return serialize(&a);
}

// ============================================================================
// [asmjit::X86Builder - Events]
// ============================================================================

Error X86Builder::onAttach(CodeHolder* code) noexcept {
  uint32_t archType = code->getArchType();
  if (!ArchInfo::isX86Family(archType))
    return DebugUtils::errored(kErrorInvalidArch);

  ASMJIT_PROPAGATE(Base::onAttach(code));

  _gpRegInfo.setSignature(archType == ArchInfo::kTypeX86 ? uint32_t(X86Gpd::kSignature) : uint32_t(X86Gpq::kSignature));
  return kErrorOk;
}

ASMJIT_END_NAMESPACE

// [Guard]
#endif // ASMJIT_BUILD_X86 && !ASMJIT_DISABLE_COMPILER
