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

ASMJIT_BEGIN_SUB_NAMESPACE(x86)

// ============================================================================
// [asmjit::x86::Builder - Construction / Destruction]
// ============================================================================

Builder::Builder(CodeHolder* code) noexcept : BaseBuilder() {
  if (code)
    code->attach(this);
}
Builder::Builder(Builder&& o) noexcept : BaseBuilder(std::move(o)) {}
Builder::~Builder() noexcept {}

// ============================================================================
// [asmjit::x86::Builder - Finalize]
// ============================================================================

Error Builder::finalize() {
  ASMJIT_PROPAGATE(runPasses());

  Assembler a(_code);
  return serialize(&a);
}

// ============================================================================
// [asmjit::x86::Builder - Events]
// ============================================================================

Error Builder::onAttach(CodeHolder* code) noexcept {
  uint32_t archId = code->archId();
  if (!ArchInfo::isX86Family(archId))
    return DebugUtils::errored(kErrorInvalidArch);

  ASMJIT_PROPAGATE(Base::onAttach(code));

  _gpRegInfo.setSignature(archId == ArchInfo::kIdX86 ? uint32_t(Gpd::kSignature) : uint32_t(Gpq::kSignature));
  return kErrorOk;
}

ASMJIT_END_SUB_NAMESPACE

// [Guard]
#endif // ASMJIT_BUILD_X86 && !ASMJIT_DISABLE_COMPILER
