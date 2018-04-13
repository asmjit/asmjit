// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_CORE_ASSEMBLER_H
#define _ASMJIT_CORE_ASSEMBLER_H

// [Dependencies]
#include "../core/codeholder.h"
#include "../core/emitter.h"
#include "../core/operand.h"
#include "../core/simdtypes.h"

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
  //! Move a `BaseAssembler` instance.
  ASMJIT_API BaseAssembler(BaseAssembler&& b) noexcept;
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
  // [Label Management]
  // --------------------------------------------------------------------------

  ASMJIT_API Label newLabel() override;
  ASMJIT_API Label newNamedLabel(const char* name, size_t nameSize = Globals::kNullTerminated, uint32_t type = Label::kTypeGlobal, uint32_t parentId = 0) override;
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

  ASMJIT_API Error comment(const char* data, size_t size = Globals::kNullTerminated) override;

  // --------------------------------------------------------------------------
  // [Events]
  // --------------------------------------------------------------------------

  ASMJIT_API Error onAttach(CodeHolder* code) noexcept override;
  ASMJIT_API Error onDetach(CodeHolder* code) noexcept override;

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  SectionEntry* _section;                //!< Current section where the assembling happens.
  uint8_t* _bufferData;                  //!< Start of the CodeBuffer of the current section.
  uint8_t* _bufferEnd;                   //!< End (first invalid byte) of the current section.
  uint8_t* _bufferPtr;                   //!< Pointer in the CodeBuffer of the current section.
  Operand_ _op4;                         //!< 5th operand data, used only temporarily.
  Operand_ _op5;                         //!< 6th operand data, used only temporarily.
};

// ============================================================================
// [asmjit::AsmBufferWriter]
// ============================================================================

// TODO: Better name, should not be here, maybe hide from public API completely?
class AsmBufferWriter {
public:
  explicit ASMJIT_FORCEINLINE AsmBufferWriter(BaseAssembler* a) noexcept
    : _cursor(a->_bufferPtr) {}

  ASMJIT_FORCEINLINE Error ensureSpace(BaseAssembler* a, size_t n) noexcept {
    size_t remainingSpace = (size_t)(a->_bufferEnd - _cursor);
    if (ASMJIT_UNLIKELY(remainingSpace < n)) {
      CodeBuffer& buffer = a->_section->_buffer;
      Error err = a->_code->growBuffer(&buffer, n);
      if (ASMJIT_UNLIKELY(err))
        return a->reportError(err);
      _cursor = a->_bufferPtr;
    }
    return kErrorOk;
  }

  ASMJIT_FORCEINLINE uint8_t* cursor() const noexcept {
    return _cursor;
  }

  ASMJIT_FORCEINLINE size_t offset(uint8_t* from) const noexcept {
    ASMJIT_ASSERT(_cursor >= from);
    return (size_t)(_cursor - from);
  }

  ASMJIT_FORCEINLINE void advance(size_t n) noexcept {
    _cursor += n;
  }

  template<typename T>
  ASMJIT_FORCEINLINE void emit8(T val) noexcept {
    typedef typename std::make_unsigned<T>::type U;
    _cursor[0] = uint8_t(U(val) & U(0xFF));
    _cursor++;
  }

  template<typename T, typename Y>
  ASMJIT_FORCEINLINE void emit8If(T val, Y cond) noexcept {
    typedef typename std::make_unsigned<T>::type U;
    ASMJIT_ASSERT(size_t(cond) <= 1U);

    _cursor[0] = uint8_t(U(val) & U(0xFF));
    _cursor += size_t(cond);
  }

  template<typename T>
  ASMJIT_FORCEINLINE void emit16uLE(T val) noexcept {
    typedef typename std::make_unsigned<T>::type U;
    MemUtils::writeU16uLE(_cursor, uint32_t(U(val) & 0xFFFFU));
    _cursor += 2;
  }

  template<typename T>
  ASMJIT_FORCEINLINE void emit16uBE(T val) noexcept {
    typedef typename std::make_unsigned<T>::type U;
    MemUtils::writeU16uBE(_cursor, uint32_t(U(val) & 0xFFFFU));
    _cursor += 2;
  }

  template<typename T>
  ASMJIT_FORCEINLINE void emit32uLE(T val) noexcept {
    typedef typename std::make_unsigned<T>::type U;
    MemUtils::writeU32uLE(_cursor, uint32_t(U(val) & 0xFFFFFFFFU));
    _cursor += 4;
  }

  template<typename T>
  ASMJIT_FORCEINLINE void emit32uBE(T val) noexcept {
    typedef typename std::make_unsigned<T>::type U;
    MemUtils::writeU32uBE(_cursor, uint32_t(U(val) & 0xFFFFFFFFU));
    _cursor += 4;
  }

  ASMJIT_FORCEINLINE void emitData(const void* data, size_t size) noexcept {
    ASMJIT_ASSERT(size != 0);
    std::memcpy(_cursor, data, size);
    _cursor += size;
  }

  ASMJIT_FORCEINLINE void emitZeros(size_t size) noexcept {
    ASMJIT_ASSERT(size != 0);
    std::memset(_cursor, 0, size);
    _cursor += size;
  }

  ASMJIT_FORCEINLINE void done(BaseAssembler* a) noexcept {
    CodeBuffer& buffer = a->_section->_buffer;
    size_t newSize = (size_t)(_cursor - a->_bufferData);
    ASMJIT_ASSERT(newSize <= buffer.capacity());

    a->_bufferPtr = _cursor;
    buffer._size = std::max(buffer._size, newSize);
  }

  uint8_t* _cursor;
};

//! \}

ASMJIT_END_NAMESPACE

// [Guard]
#endif // _ASMJIT_CORE_ASSEMBLER_H
