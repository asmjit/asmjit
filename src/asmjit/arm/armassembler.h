// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// ZLIB - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_ARM_ARMASSEMBLER_H
#define _ASMJIT_ARM_ARMASSEMBLER_H

// [Dependencies]
#include "../core/assembler.h"
#include "../arm/armemitter.h"
#include "../arm/armoperand.h"

ASMJIT_BEGIN_SUB_NAMESPACE(arm)

//! \addtogroup asmjit_arm_api
//! \{

// ============================================================================
// [asmjit::A32Assembler]
// ============================================================================

//! ARM32 assembler.
class ASMJIT_VIRTAPI Assembler
  : public BaseAssembler,
    public A32EmitterT<Assembler> {

public:
  typedef BaseAssembler Base;

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  ASMJIT_API Assembler(CodeHolder* code = nullptr) noexcept;
  ASMJIT_API virtual ~Assembler() noexcept;

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! Get whether the current ARM mode is THUMB (only available on A32).
  inline bool isInThumbMode() const noexcept { return _archInfo.subId() == ArchInfo::kSubIdA32_Thumb; }

  //! Get the current code alignment of the current mode (ARM vs THUMB).
  inline uint32_t codeAlignment() const noexcept { return isInThumbMode() ? 2 : 4; }

  // --------------------------------------------------------------------------
  // [Code-Generation]
  // --------------------------------------------------------------------------

  using BaseEmitter::_emit;

  ASMJIT_API Error _emit(uint32_t instId, const Operand_& o0, const Operand_& o1, const Operand_& o2, const Operand_& o3) override;
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
#endif // _ASMJIT_ARM_ARMASSEMBLER_H
