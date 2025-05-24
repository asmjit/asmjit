// This file is part of AsmJit project <https://asmjit.com>
//
// See asmjit.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include "../core/api-build_p.h"
#if !defined(ASMJIT_NO_LOONGARCH64) && !defined(ASMJIT_NO_BUILDER)

#include "../loongarch/la64assembler.h"
#include "../loongarch/la64builder.h"
#include "../loongarch/la64emithelper_p.h"

ASMJIT_BEGIN_SUB_NAMESPACE(la64)

// a64::Builder - Construction & Destruction
// =========================================

Builder::Builder(CodeHolder* code) noexcept : BaseBuilder() {
  _archMask = uint64_t(1) << uint32_t(Arch::kLOONGARCH64);
  if (code)
    code->attach(this);
}
Builder::~Builder() noexcept {}

// a64::Builder - Events
// =====================

Error Builder::onAttach(CodeHolder* code) noexcept {
  ASMJIT_PROPAGATE(Base::onAttach(code));

  _instructionAlignment = uint8_t(4);
  assignEmitterFuncs(this);

  return kErrorOk;
}

Error Builder::onDetach(CodeHolder* code) noexcept {
  return Base::onDetach(code);
}


// a64::Builder - Finalize
// =======================

Error Builder::finalize() {
  ASMJIT_PROPAGATE(runPasses());
  Assembler a(_code);
  a.addEncodingOptions(encodingOptions());
  a.addDiagnosticOptions(diagnosticOptions());
  return serializeTo(&a);
}

ASMJIT_END_SUB_NAMESPACE

#endif // !ASMJIT_NO_LOONGARCH64 && !ASMJIT_NO_BUILDER
