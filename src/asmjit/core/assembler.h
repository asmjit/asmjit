// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// ZLIB - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_CORE_ASSEMBLER_H
#define _ASMJIT_CORE_ASSEMBLER_H

// [Dependencies]
#include "../core/codeholder.h"
#include "../core/datatypes.h"
#include "../core/emitter.h"
#include "../core/operand.h"

ASMJIT_BEGIN_NAMESPACE

//! \addtogroup asmjit_core_api
//! \{

// ============================================================================
// [asmjit::BaseAssembler]
// ============================================================================

//! Base encoder (assembler).
class ASMJIT_VIRTAPI BaseAssembler : public BaseEmitter {
public:
  ASMJIT_NONCOPYABLE(BaseAssembler)
  typedef BaseEmitter Base;

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Create a new `BaseAssembler` instance.
  ASMJIT_API BaseAssembler() noexcept;
  //! Destroy the `BaseAssembler` instance.
  ASMJIT_API virtual ~BaseAssembler() noexcept;

  // --------------------------------------------------------------------------
  // [Buffer Management]
  // --------------------------------------------------------------------------

  //! Get the capacity of the current CodeBuffer.
  inline size_t bufferCapacity() const noexcept { return (size_t)(_bufferEnd - _bufferData); }
  //! Get the number of remaining bytes in the current CodeBuffer.
  inline size_t remainingSpace() const noexcept { return (size_t)(_bufferEnd - _bufferPtr); }

  //! Get the current position in the CodeBuffer.
  inline size_t offset() const noexcept { return (size_t)(_bufferPtr - _bufferData); }
  //! Set the current position in the CodeBuffer to `offset`.
  //!
  //! NOTE: The `offset` cannot be outside of the buffer size (even if it's
  //! within buffer's capacity).
  ASMJIT_API Error setOffset(size_t offset);

  //! Get start of the CodeBuffer of the current section.
  inline uint8_t* bufferData() const noexcept { return _bufferData; }
  //! Get end (first invalid byte) of the current section.
  inline uint8_t* bufferEnd() const noexcept { return _bufferEnd; }
  //! Get pointer in the CodeBuffer of the current section.
  inline uint8_t* bufferPtr() const noexcept { return _bufferPtr; }

  // --------------------------------------------------------------------------
  // [Section Management]
  // --------------------------------------------------------------------------

  inline Section* currentSection() const noexcept { return _section; }

  ASMJIT_API Error section(Section* section) override;

  // --------------------------------------------------------------------------
  // [Label Management]
  // --------------------------------------------------------------------------

  ASMJIT_API Label newLabel() override;
  ASMJIT_API Label newNamedLabel(const char* name, size_t nameSize = SIZE_MAX, uint32_t type = Label::kTypeGlobal, uint32_t parentId = 0) override;
  ASMJIT_API Error bind(const Label& label) override;

  // --------------------------------------------------------------------------
  // [Emit (Low-Level)]
  // --------------------------------------------------------------------------

  using BaseEmitter::_emit;

  ASMJIT_API Error _emit(uint32_t instId, const Operand_& o0, const Operand_& o1, const Operand_& o2, const Operand_& o3, const Operand_& o4, const Operand_& o5) override;
  ASMJIT_API Error _emitOpArray(uint32_t instId, const Operand_* operands, size_t count) override;

protected:
  #ifndef ASMJIT_DISABLE_LOGGING
  void _emitLog(
    uint32_t instId, uint32_t options, const Operand_& o0, const Operand_& o1, const Operand_& o2, const Operand_& o3,
    uint32_t relSize, uint32_t immSize, uint8_t* afterCursor);

  Error _emitFailed(
    Error err,
    uint32_t instId, uint32_t options, const Operand_& o0, const Operand_& o1, const Operand_& o2, const Operand_& o3);
  #else
  inline Error _emitFailed(
    uint32_t err,
    uint32_t instId, uint32_t options, const Operand_& o0, const Operand_& o1, const Operand_& o2, const Operand_& o3) {

    ASMJIT_UNUSED(instId);
    ASMJIT_UNUSED(options);
    ASMJIT_UNUSED(o0);
    ASMJIT_UNUSED(o1);
    ASMJIT_UNUSED(o2);
    ASMJIT_UNUSED(o3);

    resetInstOptions();
    resetInlineComment();
    return reportError(err);
  }
  #endif
public:

  // --------------------------------------------------------------------------
  // [Embed]
  // --------------------------------------------------------------------------

  ASMJIT_API Error embed(const void* data, uint32_t size) override;
  ASMJIT_API Error embedLabel(const Label& label) override;
  ASMJIT_API Error embedConstPool(const Label& label, const ConstPool& pool) override;

  // --------------------------------------------------------------------------
  // [Comment]
  // --------------------------------------------------------------------------

  ASMJIT_API Error comment(const char* data, size_t size = SIZE_MAX) override;

  // --------------------------------------------------------------------------
  // [Events]
  // --------------------------------------------------------------------------

  ASMJIT_API Error onAttach(CodeHolder* code) noexcept override;
  ASMJIT_API Error onDetach(CodeHolder* code) noexcept override;

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  Section* _section;                     //!< Current section where the assembling happens.
  uint8_t* _bufferData;                  //!< Start of the CodeBuffer of the current section.
  uint8_t* _bufferEnd;                   //!< End (first invalid byte) of the current section.
  uint8_t* _bufferPtr;                   //!< Pointer in the CodeBuffer of the current section.
  Operand_ _op4;                         //!< 5th operand data, used only temporarily.
  Operand_ _op5;                         //!< 6th operand data, used only temporarily.
};

//! \}

ASMJIT_END_NAMESPACE

// [Guard]
#endif // _ASMJIT_CORE_ASSEMBLER_H
