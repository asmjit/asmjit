// [AsmJit]
// Machine Code Generation for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

#ifndef _ASMJIT_X86_X86ASSEMBLER_H
#define _ASMJIT_X86_X86ASSEMBLER_H

#include "../core/assembler.h"
#include "../x86/x86emitter.h"
#include "../x86/x86operand.h"

ASMJIT_BEGIN_SUB_NAMESPACE(x86)

//! \addtogroup asmjit_x86
//! \{

// ============================================================================
// [asmjit::Assembler]
// ============================================================================

//! Assembler (X86).
//!
//! Emits X86 machine-code into buffers managed by `CodeHolder`.
class ASMJIT_VIRTAPI Assembler
  : public BaseAssembler,
    public EmitterImplicitT<Assembler> {
public:
  ASMJIT_NONCOPYABLE(Assembler)
  typedef BaseAssembler Base;

  //! \name Construction & Destruction
  //! \{

  ASMJIT_API explicit Assembler(CodeHolder* code = nullptr) noexcept;
  ASMJIT_API virtual ~Assembler() noexcept;

  //! \}

  //! \cond INTERNAL
  //! \name Internal
  //! \{

  // NOTE: x86::Assembler uses _privateData to store 'address-override' bit that
  // is used to decide whether to emit address-override (67H) prefix based on
  // the memory BASE+INDEX registers. It's either `kX86MemInfo_67H_X86` or
  // `kX86MemInfo_67H_X64`.
  inline uint32_t _addressOverrideMask() const noexcept { return _privateData; }
  inline void _setAddressOverrideMask(uint32_t m) noexcept { _privateData = m; }

  //! \}
  //! \endcond

  //! \cond INTERNAL
  //! \name Emit
  //! \{

  using BaseEmitter::_emit;
  ASMJIT_API Error _emit(uint32_t instId, const Operand_& o0, const Operand_& o1, const Operand_& o2, const Operand_& o3) override;

  //! \}
  //! \endcond

  //! \name Align
  //! \{

  ASMJIT_API Error align(uint32_t alignMode, uint32_t alignment) override;

  //! \}

  //! \name Events
  //! \{

  ASMJIT_API Error onAttach(CodeHolder* code) noexcept override;
  ASMJIT_API Error onDetach(CodeHolder* code) noexcept override;

  //! \}
};

//! \}

ASMJIT_END_SUB_NAMESPACE

#endif // _ASMJIT_X86_X86ASSEMBLER_H
