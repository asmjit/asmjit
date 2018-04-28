// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// ZLIB - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_X86_X86ASSEMBLER_H
#define _ASMJIT_X86_X86ASSEMBLER_H

// [Dependencies]
#include "../core/assembler.h"
#include "../x86/x86emitter.h"
#include "../x86/x86operand.h"

ASMJIT_BEGIN_SUB_NAMESPACE(x86)

//! \addtogroup asmjit_x86_api
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

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  ASMJIT_API explicit Assembler(CodeHolder* code = nullptr) noexcept;
  ASMJIT_API virtual ~Assembler() noexcept;

  // --------------------------------------------------------------------------
  // [Internal]
  // --------------------------------------------------------------------------

  // NOTE: x86::Assembler uses _privateData to store 'address-override' bit that
  // is used to decide whether to emit address-override (67H) prefix based on
  // the memory BASE+INDEX registers. It's either `kX86MemInfo_67H_X86` or
  // `kX86MemInfo_67H_X64`.
  inline uint32_t _addressOverrideMask() const noexcept { return _privateData; }
  inline void _setAddressOverrideMask(uint32_t m) noexcept { _privateData = m; }

  // --------------------------------------------------------------------------
  // [Emit (Low-Level)]
  // --------------------------------------------------------------------------

  using BaseEmitter::_emit;

  ASMJIT_API Error _emit(uint32_t instId, const Operand_& o0, const Operand_& o1, const Operand_& o2, const Operand_& o3) override;

  // --------------------------------------------------------------------------
  // [Align]
  // --------------------------------------------------------------------------

  ASMJIT_API Error align(uint32_t alignMode, uint32_t alignment) override;

  // --------------------------------------------------------------------------
  // [Events]
  // --------------------------------------------------------------------------

  ASMJIT_API Error onAttach(CodeHolder* code) noexcept override;
  ASMJIT_API Error onDetach(CodeHolder* code) noexcept override;
};

//! \}

ASMJIT_END_SUB_NAMESPACE

// [Guard]
#endif // _ASMJIT_X86_X86ASSEMBLER_H
