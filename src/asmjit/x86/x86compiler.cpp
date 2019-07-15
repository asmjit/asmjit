// [AsmJit]
// Machine Code Generation for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

#define ASMJIT_EXPORTS

#include "../core/build.h"
#if defined(ASMJIT_BUILD_X86) && !defined(ASMJIT_NO_COMPILER)

#include "../x86/x86assembler.h"
#include "../x86/x86compiler.h"
#include "../x86/x86rapass_p.h"

ASMJIT_BEGIN_SUB_NAMESPACE(x86)

// ============================================================================
// [asmjit::x86::Compiler - Construction / Destruction]
// ============================================================================

Compiler::Compiler(CodeHolder* code) noexcept : BaseCompiler() {
  if (code)
    code->attach(this);
}
Compiler::~Compiler() noexcept {}

// ============================================================================
// [asmjit::x86::Compiler - Finalize]
// ============================================================================

Error Compiler::finalize() {
  ASMJIT_PROPAGATE(runPasses());
  Assembler a(_code);
  return serialize(&a);
}
// ============================================================================
// [asmjit::x86::Compiler - Events]
// ============================================================================

Error Compiler::onAttach(CodeHolder* code) noexcept {
  uint32_t archId = code->archId();
  if (!ArchInfo::isX86Family(archId))
    return DebugUtils::errored(kErrorInvalidArch);

  ASMJIT_PROPAGATE(Base::onAttach(code));
  _gpRegInfo.setSignature(archId == ArchInfo::kIdX86 ? uint32_t(Gpd::kSignature) : uint32_t(Gpq::kSignature));

  Error err = addPassT<X86RAPass>();
  if (ASMJIT_UNLIKELY(err)) {
    onDetach(code);
    return err;
  }

  return kErrorOk;
}

ASMJIT_END_SUB_NAMESPACE

#endif // ASMJIT_BUILD_X86 && !ASMJIT_NO_COMPILER
