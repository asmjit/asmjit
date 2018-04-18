// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// ZLIB - See LICENSE.md file in the package.

// [Export]
#define ASMJIT_EXPORTS

// [Guard]
#include "../core/build.h"
#if defined(ASMJIT_BUILD_X86) && !defined(ASMJIT_DISABLE_COMPILER)

// [Dependencies]
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
  // Flush the global constant pool.
  if (_globalConstPool) {
    addNode(_globalConstPool);
    _globalConstPool = nullptr;
  }

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

  ASMJIT_PROPAGATE(_passes.willGrow(&_allocator));
  ASMJIT_PROPAGATE(Base::onAttach(code));

  _gpRegInfo.setSignature(archId == ArchInfo::kIdX86 ? uint32_t(Gpd::kSignature) : uint32_t(Gpq::kSignature));
  return addPassT<X86RAPass>();
}

ASMJIT_END_SUB_NAMESPACE

// [Guard]
#endif // ASMJIT_BUILD_X86 && !ASMJIT_DISABLE_COMPILER
