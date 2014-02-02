// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_BASE_ASSEMBLER_H
#define _ASMJIT_BASE_ASSEMBLER_H

// [Dependencies - AsmJit]
#include "../base/codegen.h"
#include "../base/defs.h"
#include "../base/error.h"
#include "../base/logger.h"
#include "../base/podlist.h"
#include "../base/podvector.h"
#include "../base/runtime.h"
#include "../base/zone.h"

// [Api-Begin]
#include "../base/apibegin.h"

namespace asmjit {

//! @addtogroup asmjit_base
//! @{

// ============================================================================
// [asmjit::LabelLink]
// ============================================================================

//! @brief Data structure used to link linked-labels.
struct LabelLink {
  //! @brief Previous link.
  LabelLink* prev;
  //! @brief Offset.
  intptr_t offset;
  //! @brief Inlined displacement.
  intptr_t displacement;
  //! @brief RelocId if link must be absolute when relocated.
  intptr_t relocId;
};

// ============================================================================
// [asmjit::LabelData]
// ============================================================================

//! @brief Label data.
struct LabelData {
  //! @brief Label offset.
  intptr_t offset;
  //! @brief Label links chain.
  LabelLink* links;
};

// ============================================================================
// [asmjit::RelocData]
// ============================================================================

//! @brief Code relocation data (relative vs absolute addresses).
//!
//! X86/X64:
//!
//! X86 architecture uses 32-bit absolute addressing model by memory operands,
//! but 64-bit mode uses relative addressing model (RIP + displacement). In
//! code we are always using relative addressing model for referencing labels
//! and embedded data. In 32-bit mode we must patch all references to absolute
//! address before we can call generated function.
struct RelocData {
  //! @brief Type of relocation.
  uint32_t type;
  //! @brief Size of relocation (4 or 8 bytes).
  uint32_t size;

  //! @brief Offset from code begin address.
  Ptr from;

  //! @brief Relative displacement from code begin address (not to @c offset)
  //! or absolute address.
  Ptr data;
};

// ============================================================================
// [asmjit::BaseAssembler]
// ============================================================================

//! @brief Base assembler.
//!
//! This class implements core setialization API only. The platform specific
//! methods and intrinsics is implemented by derived classes.
//!
//! @sa BaseCompiler.
struct BaseAssembler : public CodeGen {
  ASMJIT_NO_COPY(BaseAssembler)

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! @brief Create a new @ref BaseAssembler instance.
  ASMJIT_API BaseAssembler(BaseRuntime* runtime);
  //! @brief Destroy the @ref BaseAssembler instance.
  ASMJIT_API virtual ~BaseAssembler();

  // --------------------------------------------------------------------------
  // [Clear / Reset]
  // --------------------------------------------------------------------------

  //! @brief Clear everything, but not deallocate buffers.
  ASMJIT_API void clear();
  //! @brief Reset everything (means also to free all buffers).
  ASMJIT_API void reset();
  //! @brief Called by clear() and reset() to clear all data related to derived
  //! class implementation.
  ASMJIT_API virtual void _purge();

  // --------------------------------------------------------------------------
  // [Buffer]
  // --------------------------------------------------------------------------

  //! @brief Get capacity of the code buffer.
  ASMJIT_INLINE size_t getCapacity() const {
    return (size_t)(_end - _buffer);
  }

  //! @brief Get the number of remaining bytes (space between cursor and the
  //! end of the buffer).
  ASMJIT_INLINE size_t getRemainingSpace() const {
    return (size_t)(_end - _cursor);
  }

  //! @brief Get buffer.
  ASMJIT_INLINE uint8_t* getBuffer() const {
    return _buffer;
  }

  //! @brief Get the end of the buffer (points to the first byte that is outside).
  ASMJIT_INLINE uint8_t* getEnd() const {
    return _end;
  }

  //! @brief Get the current position in the buffer.
  ASMJIT_INLINE uint8_t* getCursor() const {
    return _cursor;
  }

  //! @brief Set the current position in the buffer.
  ASMJIT_INLINE void setCursor(uint8_t* cursor) {
    ASMJIT_ASSERT(cursor >= _buffer && cursor <= _end);
    _cursor = cursor;
  }

  //! @brief Get the current offset in the buffer (<code>_cursor - _buffer</code>).
  ASMJIT_INLINE size_t getOffset() const {
    return (size_t)(_cursor - _buffer);
  }

  //! @brief Set the current offset in the buffer to @a offset and get the
  //! previous offset value.
  ASMJIT_INLINE size_t setOffset(size_t offset) {
    ASMJIT_ASSERT(offset < getCapacity());

    size_t oldOffset = (size_t)(_cursor - _buffer);
    _cursor = _buffer + offset;
    return oldOffset;
  }

  //! @brief Grow the internal buffer.
  //!
  //! The internal buffer will grow at least by @a n bytes so @a n bytes
  //! can be added to it. If @a n is zero or <code>getOffset() + n</code>
  //! is not greater than the current capacity of the buffer this function
  //! won't do anything.
  ASMJIT_API Error _grow(size_t n);

  //! @brief Reserve the internal buffer to at least @a n bytes.
  ASMJIT_API Error _reserve(size_t n);

  //! @brief Set byte at position @a pos.
  ASMJIT_INLINE uint8_t getByteAt(size_t pos) const {
    ASMJIT_ASSERT(pos + 1 <= (size_t)(_end - _buffer));
    return *reinterpret_cast<const uint8_t*>(_buffer + pos);
  }

  //! @brief Set word at position @a pos.
  ASMJIT_INLINE uint16_t getWordAt(size_t pos) const {
    ASMJIT_ASSERT(pos + 2 <= (size_t)(_end - _buffer));
    return *reinterpret_cast<const uint16_t*>(_buffer + pos);
  }

  //! @brief Set dword at position @a pos.
  ASMJIT_INLINE uint32_t getDWordAt(size_t pos) const {
    ASMJIT_ASSERT(pos + 4 <= (size_t)(_end - _buffer));
    return *reinterpret_cast<const uint32_t*>(_buffer + pos);
  }

  //! @brief Set qword at position @a pos.
  ASMJIT_INLINE uint64_t getQWordAt(size_t pos) const {
    ASMJIT_ASSERT(pos + 8 <= (size_t)(_end - _buffer));
    return *reinterpret_cast<const uint64_t*>(_buffer + pos);
  }

  //! @brief Set int32_t at position @a pos.
  ASMJIT_INLINE int32_t getInt32At(size_t pos) const {
    ASMJIT_ASSERT(pos + 4 <= (size_t)(_end - _buffer));
    return *reinterpret_cast<const int32_t*>(_buffer + pos);
  }

  //! @brief Set uint32_t at position @a pos.
  ASMJIT_INLINE uint32_t getUInt32At(size_t pos) const {
    ASMJIT_ASSERT(pos + 4 <= (size_t)(_end - _buffer));
    return *reinterpret_cast<const uint32_t*>(_buffer + pos);
  }

  //! @brief Set byte at position @a pos.
  ASMJIT_INLINE void setByteAt(size_t pos, uint8_t x) {
    ASMJIT_ASSERT(pos + 1 <= (size_t)(_end - _buffer));
    *reinterpret_cast<uint8_t*>(_buffer + pos) = x;
  }

  //! @brief Set word at position @a pos.
  ASMJIT_INLINE void setWordAt(size_t pos, uint16_t x) {
    ASMJIT_ASSERT(pos + 2 <= (size_t)(_end - _buffer));
    *reinterpret_cast<uint16_t*>(_buffer + pos) = x;
  }

  //! @brief Set dword at position @a pos.
  ASMJIT_INLINE void setDWordAt(size_t pos, uint32_t x) {
    ASMJIT_ASSERT(pos + 4 <= (size_t)(_end - _buffer));
    *reinterpret_cast<uint32_t*>(_buffer + pos) = x;
  }

  //! @brief Set qword at position @a pos.
  ASMJIT_INLINE void setQWordAt(size_t pos, uint64_t x) {
    ASMJIT_ASSERT(pos + 8 <= (size_t)(_end - _buffer));
    *reinterpret_cast<uint64_t*>(_buffer + pos) = x;
  }

  //! @brief Set int32_t at position @a pos.
  ASMJIT_INLINE void setInt32At(size_t pos, int32_t x) {
    ASMJIT_ASSERT(pos + 4 <= (size_t)(_end - _buffer));
    *reinterpret_cast<int32_t*>(_buffer + pos) = x;
  }

  //! @brief Set uint32_t at position @a pos.
  ASMJIT_INLINE void setUInt32At(size_t pos, uint32_t x) {
    ASMJIT_ASSERT(pos + 4 <= (size_t)(_end - _buffer));
    *reinterpret_cast<uint32_t*>(_buffer + pos) = x;
  }

  // --------------------------------------------------------------------------
  // [GetCodeSize]
  // --------------------------------------------------------------------------

  //! @brief Get current offset in buffer (same as <code>getOffset() + getTramplineSize()</code>).
  ASMJIT_INLINE size_t getCodeSize() const {
    return getOffset() + getTrampolineSize();
  }

  // --------------------------------------------------------------------------
  // [GetTrampolineSize]
  // --------------------------------------------------------------------------

  //! @brief Get size of all possible trampolines needed to successfuly generate
  //! relative jumps to absolute addresses. This value is only non-zero if jmp
  //! of call instructions were used with immediate operand (this means jumping
  //! or calling an absolute address directly).
  ASMJIT_INLINE size_t getTrampolineSize() const {
    return _trampolineSize;
  }

  // --------------------------------------------------------------------------
  // [Label]
  // --------------------------------------------------------------------------

  //! @brief Get count of labels created.
  ASMJIT_INLINE size_t getLabelsCount() const {
    return _labels.getLength();
  }

  //! @brief Get whether @a label is created.
  ASMJIT_INLINE bool isLabelCreated(const Label& label) const {
    return static_cast<size_t>(label.getId()) < _labels.getLength();
  }

  //! @internal
  //!
  //! @brief Get @ref LabelData by @a label.
  ASMJIT_INLINE LabelData* getLabelData(const Label& label) const {
    return getLabelDataById(label.getId());
  }

  //! @internal
  //!
  //! @brief Get @ref LabelData by @a id.
  ASMJIT_INLINE LabelData* getLabelDataById(uint32_t id) const {
    ASMJIT_ASSERT(id != kInvalidValue);
    ASMJIT_ASSERT(id < _labels.getLength());

    return const_cast<LabelData*>(&_labels[id]);
  }

  //! @internal
  //!
  //! @brief Register labels for other code generator (@ref Compiler).
  ASMJIT_API Error _registerIndexedLabels(size_t index);

  //! @internal
  //!
  //! @brief Create and initialize a new label.
  ASMJIT_API Error _newLabel(Label* dst);

  //! @internal
  //!
  //! @brief New LabelLink instance.
  ASMJIT_API LabelLink* _newLabelLink();

  //! @brief Create and return new label.
  ASMJIT_INLINE Label newLabel() {
    Label result(DontInitialize);
    _newLabel(&result);
    return result;
  }

  //! @brief Bind label to the current offset (virtual).
  virtual void _bind(const Label& label) = 0;

  //! @brief Bind label to the current offset (virtual).
  //!
  //! @note Label can be bound only once!
  ASMJIT_INLINE void bind(const Label& label) {
    _bind(label);
  }

  // --------------------------------------------------------------------------
  // [Embed]
  // --------------------------------------------------------------------------

  //! @brief Embed data into the code buffer.
  ASMJIT_API Error embed(const void* data, uint32_t size);

  // --------------------------------------------------------------------------
  // [Align]
  // --------------------------------------------------------------------------

  //! @brief Align target buffer to @a m bytes.
  //!
  //! Typical usage of this is to align labels at start of the inner loops.
  //!
  //! Inserts @c nop() instructions or CPU optimized NOPs.
  ASMJIT_INLINE Error align(uint32_t m) {
    return _align(m);
  }

  //! @brief Align target buffer to @a m bytes (virtual).
  virtual Error _align(uint32_t m) = 0;

  // --------------------------------------------------------------------------
  // [Reloc]
  // --------------------------------------------------------------------------

  //! @brief Simplifed version of @c relocCode() method designed for JIT.
  //!
  //! @overload
  ASMJIT_INLINE size_t relocCode(void* dst) const {
    return _relocCode(dst, static_cast<Ptr>((uintptr_t)dst));
  }

  //! @brief Relocate code to a given address @a dst.
  //!
  //! @param dst Where the relocated code should me stored. The pointer can be
  //! address returned by virtual memory allocator or your own address if you
  //! want only to store the code for later reuse (or load, etc...).
  //! @param addressBase Base address used for relocation. When using JIT code
  //! generation, this will be the same as @a dst, only casted to system
  //! integer type. But when generating code for remote process then the value
  //! can be different.
  //!
  //! @retval The bytes used. Code-generator can create trampolines which are
  //! used when calling other functions inside the JIT code. However, these
  //! trampolines can be unused so the relocCode() returns the exact size needed
  //! for the function.
  //!
  //! A given buffer will be overwritten, to get number of bytes required use
  //! @c getCodeSize().
  ASMJIT_INLINE size_t relocCode(void* dst, Ptr base) const {
    return _relocCode(dst, base);
  }

  //! @brief Reloc code (virtual).
  virtual size_t _relocCode(void* dst, Ptr base) const = 0;

  // --------------------------------------------------------------------------
  // [Make]
  // --------------------------------------------------------------------------

  ASMJIT_API void* make();

  // --------------------------------------------------------------------------
  // [Emit]
  // --------------------------------------------------------------------------

  //! @brief Emit an instruction.
  ASMJIT_API Error emit(uint32_t code);
  //! @overload
  ASMJIT_API Error emit(uint32_t code, const Operand& o0);
  //! @overload
  ASMJIT_API Error emit(uint32_t code, const Operand& o0, const Operand& o1);
  //! @overload
  ASMJIT_API Error emit(uint32_t code, const Operand& o0, const Operand& o1, const Operand& o2);
  //! @overload
  ASMJIT_INLINE Error emit(uint32_t code, const Operand& o0, const Operand& o1, const Operand& o2, const Operand& o3) {
    return _emit(code, o0, o1, o2, o3);
  }

  //! @brief Emit an instruction with integer immediate operand.
  ASMJIT_API Error emit(uint32_t code, int o0);
  //! @overload
  ASMJIT_API Error emit(uint32_t code, const Operand& o0, int o1);
  //! @overload
  ASMJIT_API Error emit(uint32_t code, const Operand& o0, const Operand& o1, int o2);
  //! @overload
  ASMJIT_API Error emit(uint32_t code, const Operand& o0, const Operand& o1, const Operand& o2, int o3);

  //! @brief Emit an instruction (virtual).
  virtual Error _emit(uint32_t code, const Operand& o0, const Operand& o1, const Operand& o2, const Operand& o3) = 0;

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! @brief Buffer where the code is emitted (either live or temporary).
  //!
  //! This is actually the base pointer of the buffer, to get the current
  //! position (cursor) look at the @c _cursor member.
  uint8_t* _buffer;
  //! @brief The end of the buffer (points to the first invalid byte).
  //!
  //! The end of the buffer is calculated as <code>_buffer + size</code>.
  uint8_t* _end;
  //! @brief The current position in code @c _buffer.
  uint8_t* _cursor;

  //! @brief Size of possible trampolines.
  uint32_t _trampolineSize;

  //! @brief Inline comment that will be logged by the next instruction and
  //! set to NULL.
  const char* _comment;
  //! @brief Linked list of unused links (@c LabelLink* structures)
  LabelLink* _unusedLinks;

  //! @brief Labels data.
  PodVector<LabelData> _labels;
  //! @brief Relocations data.
  PodVector<RelocData> _relocData;
};

//! @}

// ============================================================================
// [Defined-Later]
// ============================================================================

ASMJIT_INLINE Label::Label(BaseAssembler& a) : Operand(DontInitialize) {
  a._newLabel(this);
}

} // asmjit namespace

// [Api-End]
#include "../base/apiend.h"

// [Guard]
#endif // _ASMJIT_BASE_ASSEMBLER_H
