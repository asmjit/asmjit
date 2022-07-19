// This file is part of AsmJit project <https://asmjit.com>
//
// See asmjit.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_RISCV_RISCV64ASSEMBLER_H_INCLUDED
#define ASMJIT_RISCV_RISCV64ASSEMBLER_H_INCLUDED

#include "../core/assembler.h"
#include "../riscv/riscv64emitter.h"

ASMJIT_BEGIN_SUB_NAMESPACE(riscv64)

//! \addtogroup asmjit_riscv64
//! \{

//! RISC-V64 assembler implementation.
class ASMJIT_VIRTAPI Assembler
  : public BaseAssembler,
    public EmitterExplicitT<Assembler> {

public:
  typedef BaseAssembler Base;

  //! \name Construction / Destruction
  //! \{

  ASMJIT_API Assembler(CodeHolder* code = nullptr) noexcept;
  ASMJIT_API virtual ~Assembler() noexcept;

  //! \}

  //! \name Accessors
  //! \{

  // TODO: [RISC-V]

  //! \}

  //! \name Emit
  //! \{

  ASMJIT_API Error _emit(InstId instId, const Operand_& o0, const Operand_& o1, const Operand_& o2, const Operand_* opExt) override;

  //! \}

  //! \name Align
  //! \{

  ASMJIT_API Error align(AlignMode alignMode, uint32_t alignment) override;

  //! \}

  //! \name Events
  //! \{

  ASMJIT_API Error onAttach(CodeHolder* code) noexcept override;
  ASMJIT_API Error onDetach(CodeHolder* code) noexcept override;

  //! \}
};

// TODO: [RISC-V]

ASMJIT_END_SUB_NAMESPACE

#endif // ASMJIT_RISCV_RISCV64ASSEMBLER_H_INCLUDED
