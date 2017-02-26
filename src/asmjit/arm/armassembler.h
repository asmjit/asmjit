// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_ARM_ARMASSEMBLER_H
#define _ASMJIT_ARM_ARMASSEMBLER_H

// [Dependencies]
#include "../core/assembler.h"
#include "../arm/armemitter.h"
#include "../arm/armoperand.h"

ASMJIT_BEGIN_NAMESPACE

//! \addtogroup asmjit_arm
//! \{

// ============================================================================
// [asmjit::A32Assembler]
// ============================================================================

//! ARM32 assembler.
class ASMJIT_VIRTAPI A32Assembler
  : public Assembler,
    public A32EmitterT<A32Assembler> {

public:
  typedef Assembler Base;

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  ASMJIT_API A32Assembler(CodeHolder* code = nullptr) noexcept;
  ASMJIT_API virtual ~A32Assembler() noexcept;

  // --------------------------------------------------------------------------
  // [Compatibility]
  // --------------------------------------------------------------------------

  //! Implicit cast to `A32Emitter`.
  inline operator A32Emitter&() noexcept { return *as<A32Emitter>(); }
  //! Implicit cast to `A32Emitter` (const).
  inline operator const A32Emitter&() const noexcept { return *as<A32Emitter>(); }

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! Get whether the current ARM mode is THUMB (only available on A32).
  inline bool isInThumbMode() const noexcept { return _archInfo.getSubType() == ArchInfo::kSubTypeA32_Thumb; }

  //! Get the current code alignment of the current mode (ARM vs THUMB).
  inline uint32_t getCodeAlignment() const noexcept { return isInThumbMode() ? 2 : 4; }

  // --------------------------------------------------------------------------
  // [Code-Generation]
  // --------------------------------------------------------------------------

  using CodeEmitter::_emit;

  ASMJIT_API Error _emit(uint32_t instId, const Operand_& o0, const Operand_& o1, const Operand_& o2, const Operand_& o3) override;
  ASMJIT_API Error align(uint32_t mode, uint32_t alignment) override;

  // --------------------------------------------------------------------------
  // [Events]
  // --------------------------------------------------------------------------

  ASMJIT_API Error onAttach(CodeHolder* code) noexcept override;
  ASMJIT_API Error onDetach(CodeHolder* code) noexcept override;
};

//! \}

ASMJIT_END_NAMESPACE

// [Guard]
#endif // _ASMJIT_ARM_ARMASSEMBLER_H
