// This file is part of AsmJit project <https://asmjit.com>
//
// See asmjit.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include "../core/api-build_p.h"
#if !defined(ASMJIT_NO_LOONGARCH64) && !defined(ASMJIT_NO_COMPILER)

#include "../loongarch/la64assembler.h"
#include "../loongarch/la64compiler.h"
#include "../loongarch/la64emithelper_p.h"
#include "../loongarch/la64rapass_p.h"

ASMJIT_BEGIN_SUB_NAMESPACE(la64)

// a64::Compiler - Construction & Destruction
// ==========================================

Compiler::Compiler(CodeHolder* code) noexcept : BaseCompiler() {
  _archMask = uint64_t(1) << uint32_t(Arch::kLOONGARCH64);
  if (code)
    code->attach(this);
}
Compiler::~Compiler() noexcept {}

// a64::Compiler - Events
// ======================

Error Compiler::onAttach(CodeHolder* code) noexcept {
  ASMJIT_PROPAGATE(Base::onAttach(code));
  Error err = addPassT<LARAPass>();

  if (ASMJIT_UNLIKELY(err)) {
    onDetach(code);
    return err;
  }

  _instructionAlignment = uint8_t(4);
  assignEmitterFuncs(this);

  return kErrorOk;
}

Error Compiler::onDetach(CodeHolder* code) noexcept {
  return Base::onDetach(code);
}

// a64::Compiler - Finalize
// ========================

Error Compiler::finalize() {
  ASMJIT_PROPAGATE(runPasses());
  Assembler a(_code);
  a.addEncodingOptions(encodingOptions());
  a.addDiagnosticOptions(diagnosticOptions());
  return serializeTo(&a);
}

ASMJIT_END_SUB_NAMESPACE

#endif // !ASMJIT_NO_LOONGARCH64 && !ASMJIT_NO_COMPILER
