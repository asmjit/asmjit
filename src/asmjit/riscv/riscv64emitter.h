// This file is part of AsmJit project <https://asmjit.com>
//
// See asmjit.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_RISCV_RISCV64EMITTER_H_INCLUDED
#define ASMJIT_RISCV_RISCV64EMITTER_H_INCLUDED

#include "../core/emitter.h"
#include "../core/support.h"
#include "../riscv/riscv64globals.h"
#include "../riscv/riscv64operand.h"

ASMJIT_BEGIN_SUB_NAMESPACE(riscv64)

// TODO: [RISC-V] `#define ASMJIT_INST_` macros

//! \addtogroup asmjit_riscv64
//! \{

//! RISC-V 64-bit emitter.
//!
//! NOTE: This class cannot be instantiated, you can only cast to it and use it as emitter that emits to either
//! \ref Assembler, \ref Builder, or \ref Compiler (use withcaution with \ref Compiler as it expects virtual
//! registers to be used).
template<typename This>
struct EmitterExplicitT {
  //! \cond

  // These two are unfortunately reported by the sanitizer. We know what we do, however, the sanitizer doesn't.
  // I have tried to use reinterpret_cast instead, but that would generate bad code when compiled by MSC.
  ASMJIT_ATTRIBUTE_NO_SANITIZE_UNDEF inline This* _emitter() noexcept { return static_cast<This*>(this); }
  ASMJIT_ATTRIBUTE_NO_SANITIZE_UNDEF inline const This* _emitter() const noexcept { return static_cast<const This*>(this); }

  //! \endcond

  // ${riscv64::EmitterExplicit:Begin}
  // TODO: [RISC-V] See `EmitterExplicitT` of `a32`, try to generate instructions using 'https://github.com/riscv/riscv-opcodes'
  // ${riscv64::EmitterExplicit:End}
};

//! Emitter (RISC-V 64-bit).
//!
//! \note This class cannot be instantiated, you can only cast to it and use it as emitter that emits to either
//! `riscv64::Assembler`, `riscv64::Builder`, or `riscv64::Compiler` (use with caution with `riscv64::Compiler` as it requires
//! virtual registers).
class Emitter : public BaseEmitter, public EmitterExplicitT<Emitter> {
  ASMJIT_NONCONSTRUCTIBLE(Emitter)
};

ASMJIT_END_SUB_NAMESPACE

#endif // ASMJIT_RISCV_RISCV64EMITTER_H_INCLUDED
