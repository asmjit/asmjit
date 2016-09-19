// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_BASE_ASSEMBLER_H
#define _ASMJIT_BASE_ASSEMBLER_H

// [Dependencies]
#include "../base/codeemitter.h"
#include "../base/codeholder.h"
#include "../base/operand.h"
#include "../base/simdtypes.h"

// [Api-Begin]
#include "../asmjit_apibegin.h"

namespace asmjit {

//! \addtogroup asmjit_base
//! \{

// ============================================================================
// [asmjit::Assembler]
// ============================================================================

//! Base assembler.
//!
//! This class implements a base interface that is used by architecture
//! specific assemblers.
//!
//! \sa CodeCompiler.
class ASMJIT_VIRTAPI Assembler : public CodeEmitter {
public:
  ASMJIT_NONCOPYABLE(Assembler)
  typedef CodeEmitter Base;

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Create a new `Assembler` instance.
  ASMJIT_API Assembler() noexcept;
  //! Destroy the `Assembler` instance.
  ASMJIT_API virtual ~Assembler() noexcept;

  // --------------------------------------------------------------------------
  // [Events]
  // --------------------------------------------------------------------------

  ASMJIT_API virtual Error onAttach(CodeHolder* code) noexcept override;
  ASMJIT_API virtual Error onDetach(CodeHolder* code) noexcept override;

  // --------------------------------------------------------------------------
  // [Code-Buffer]
  // --------------------------------------------------------------------------

  //! Called by \ref CodeHolder::sync().
  ASMJIT_API virtual void sync() noexcept;

  //! Get the capacity of the current CodeBuffer.
  ASMJIT_INLINE size_t getBufferCapacity() const noexcept { return (size_t)(_bufferEnd - _bufferData); }
  //! Get the number of remaining bytes in the current CodeBuffer.
  ASMJIT_INLINE size_t getRemainingSpace() const noexcept { return (size_t)(_bufferEnd - _bufferPtr); }

  //! Get the current position in the CodeBuffer.
  ASMJIT_INLINE size_t getOffset() const noexcept { return (size_t)(_bufferPtr - _bufferData); }
  //! Set the current position in the CodeBuffer to `offset`.
  //!
  //! NOTE: The `offset` cannot be outside of the buffer length (even if it's
  //! within buffer's capacity).
  ASMJIT_API Error setOffset(size_t offset);

  //! Get start of the CodeBuffer of the current section.
  ASMJIT_INLINE uint8_t* getBufferData() const noexcept { return _bufferData; }
  //! Get end (first invalid byte) of the current section.
  ASMJIT_INLINE uint8_t* getBufferEnd() const noexcept { return _bufferEnd; }
  //! Get pointer in the CodeBuffer of the current section.
  ASMJIT_INLINE uint8_t* getBufferPtr() const noexcept { return _bufferPtr; }

  // --------------------------------------------------------------------------
  // [Code-Generation]
  // --------------------------------------------------------------------------

  ASMJIT_API virtual Label newLabel() override;
  ASMJIT_API virtual Label newNamedLabel(const char* name, size_t nameLength = kInvalidIndex, uint32_t type = Label::kTypeGlobal, uint32_t parentId = kInvalidValue) override;
  ASMJIT_API virtual Error bind(const Label& label) override;
  ASMJIT_API virtual Error embed(const void* data, uint32_t size) override;
  ASMJIT_API virtual Error embedLabel(const Label& label) override;
  ASMJIT_API virtual Error embedConstPool(const Label& label, const ConstPool& pool) override;
  ASMJIT_API virtual Error comment(const char* s, size_t len = kInvalidIndex) override;

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  SectionEntry* _section;                //!< Current section where the assembling happens.
  uint8_t* _bufferData;                  //!< Start of the CodeBuffer of the current section.
  uint8_t* _bufferEnd;                   //!< End (first invalid byte) of the current section.
  uint8_t* _bufferPtr;                   //!< Pointer in the CodeBuffer of the current section.
};

//! \}

} // asmjit namespace

// [Api-End]
#include "../asmjit_apiend.h"

// [Guard]
#endif // _ASMJIT_BASE_ASSEMBLER_H
