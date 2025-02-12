// This file is part of AsmJit project <https://asmjit.com>
//
// See asmjit.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_LA_LA64ASSEMBLER_H_INCLUDED
#define ASMJIT_LA_LA64ASSEMBLER_H_INCLUDED

#include "../core/assembler.h"
#include "../loongarch/la64emitter.h"
#include "../loongarch/la64operand.h"

ASMJIT_BEGIN_SUB_NAMESPACE(la64)

//! \addtogroup asmjit_la64
//! \{

//! la64 assembler implementation.
class ASMJIT_VIRTAPI Assembler
  : public BaseAssembler,
    public EmitterExplicitT<Assembler> {

public:
  typedef BaseAssembler Base;

  //! \name Construction & Destruction
  //! \{

  ASMJIT_API Assembler(CodeHolder* code = nullptr) noexcept;
  ASMJIT_API ~Assembler() noexcept override;

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

//! \}

ASMJIT_END_SUB_NAMESPACE

#endif // ASMJIT_LA_A64ASSEMBLER_H_INCLUDED
