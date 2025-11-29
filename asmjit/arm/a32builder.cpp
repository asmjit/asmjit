// This file is part of AsmJit project <https://asmjit.com>
//
// See asmjit.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core/api-build_p.h>
#if !defined(ASMJIT_NO_AARCH32) && !defined(ASMJIT_NO_BUILDER)

#include <asmjit/arm/a32assembler.h>
#include <asmjit/arm/a32builder.h>
#include <asmjit/arm/a32emithelper_p.h>

ASMJIT_BEGIN_SUB_NAMESPACE(a32)

// a32::Builder - Construction & Destruction
// =========================================

Builder::Builder(CodeHolder* code) noexcept : BaseBuilder() {
  _arch_mask = uint64_t(1) << uint32_t(Arch::kARM     ) |
               uint64_t(1) << uint32_t(Arch::kARM_BE  ) |
               uint64_t(1) << uint32_t(Arch::kThumb   ) |
               uint64_t(1) << uint32_t(Arch::kThumb_BE) ;
  if (code)
    code->attach(this);
}
Builder::~Builder() noexcept {}

// a32::Builder - Events
// =====================

Error Builder::on_attach(CodeHolder& code) noexcept {
  ASMJIT_PROPAGATE(Base::on_attach(code));

  _instruction_alignment = _environment.is_arch_thumb() ? uint8_t(2) : uint8_t(4);
  _instruction_alignment = uint8_t(4);
  update_emitter_funcs(this);

  return Error::kOk;
}

Error Builder::on_detach(CodeHolder& code) noexcept {
  return Base::on_detach(code);
}

// a32::Builder - Finalize
// =======================

Error Builder::finalize() {
  ASMJIT_PROPAGATE(run_passes());
  Assembler a(_code);
  a.add_encoding_options(encoding_options());
  a.add_diagnostic_options(diagnostic_options());
  return serialize_to(&a);
}

ASMJIT_END_SUB_NAMESPACE

#endif // !ASMJIT_NO_AARCH32 && !ASMJIT_NO_BUILDER
