// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_BASE_ASSEMBLER_H
#define _ASMJIT_BASE_ASSEMBLER_H

// [Dependencies - AsmJit]
#include "../base/containers.h"
#include "../base/logger.h"
#include "../base/operand.h"
#include "../base/podvector.h"
#include "../base/runtime.h"
#include "../base/zone.h"

// [Api-Begin]
#include "../apibegin.h"

namespace asmjit {

//! \addtogroup asmjit_base
//! \{

// ============================================================================
// [asmjit::InstId]
// ============================================================================

//! Instruction codes (stub).
ASMJIT_ENUM(InstId) {
  //! No instruction.
  kInstIdNone = 0
};

// ============================================================================
// [asmjit::InstOptions]
// ============================================================================

//! Instruction options.
ASMJIT_ENUM(InstOptions) {
  //! No instruction options.
  kInstOptionNone = 0x00000000,

  //! Emit short form of the instruction (X86/X64 only).
  //!
  //! X86/X64 Specific
  //! ----------------
  //!
  //! Short form is mostly related to jmp and jcc instructions, but can be used
  //! by other instructions supporting 8-bit or 32-bit immediates. This option
  //! can be dangerous if the short jmp/jcc is required, but not encodable due
  //! to a large displacement, in such case an error is reported.
  kInstOptionShortForm = 0x00000001,

  //! Emit long form of the instruction (X86/X64 only).
  //!
  //! X86/X64 Specific
  //! ----------------
  //!
  //! Long form is mostly related to jmp and jcc instructions, but like the
  //! `kInstOptionShortForm` option it can be used by other instructions
  //! supporting both 8-bit and 32-bit immediates.
  kInstOptionLongForm = 0x00000002,

  //! Condition is likely to be taken.
  //!
  //! X86/X64 Specific
  //! ----------------
  //!
  //! This option has no effect at the moment. Intel stopped supporting
  //! conditional hints after P4 and AMD has never supported them.
  kInstOptionTaken = 0x00000004,

  //! Condition is unlikely to be taken.
  //!
  //! X86/X64 Specific
  //! ----------------
  //!
  //! This option has no effect at the moment. Intel stopped supporting
  //! conditional hints after P4 and AMD has never supported them.
  kInstOptionNotTaken = 0x00000008,

  //! Don't follow the jump (Compiler only).
  //!
  //! Prevents following the jump during compilation.
  kInstOptionUnfollow = 0x00000010,

  //! Overwrite the destination operand (Compiler only).
  //!
  //! Hint that is important for variable liveness analysis. It tells the
  //! compiler that the destination operand will be overwritten now or by
  //! adjacent instructions. Compiler knows when a variable is overwritten by
  //! a single instruction, for example you don't have to mark "movaps" or
  //! "pxor x, x" instructions, however, if a pair of instructions is used,
  //! and the first of them doesn't completely overwrite the content of the
  //! destination, then the compiler fails to mark that variable as dead in.
  //!
  //! X86/X64 Specific
  //! ----------------
  //!
  //!   - All instructions that always overwrite at least the size of the
  //!     register that the variable uses, for example "mov", "movq", "movaps"
  //!     don't need the overwrite modifier to be used - conversion, shuffle,
  //!     and other miscellaneous instructions included.
  //!
  //!   - All instructions that clear the destination register if all operands
  //!     are the same, for example "xor x, x", "pcmpeqb", etc...
  //!
  //!   - Consecutive instructions that partially overwrite the variable until
  //!     there is no old content require the `overwrite()` to be used. Some
  //!     examples (not always the best use cases thought):
  //!
  //!     - `movlps xmm0, ?` followed by `movhps xmm0, ?` and vice versa
  //!     - `movlpd xmm0, ?` followed by `movhpd xmm0, ?` and vice versa
  //!     - `mov al, ?` followed by `and ax, 0xFF`
  //!     - `mov al, ?` followed by `mov ah, al`
  //!     - `pinsrq xmm0, ?, 0` followed by `pinsrq xmm0, ?, 1`
  //!
  //!   - If allocated variable is used temporarily for scalar operations. For
  //!     example if you allocate a full vector like `X86Compiler::newXmm()`
  //!     and then use that vector for scalar operations you should use
  //!     `overwrite()` directive:
  //!
  //!     - `sqrtss x, y` - only LO element of `x` is changed, if you don't use
  //!       HI elements, use `X86Compiler.overwrite().sqrtss(x, y)`.
  kInstOptionOverwrite = 0x00000020
};

// ============================================================================
// [asmjit::AlignMode]
// ============================================================================

//! Code aligning mode.
ASMJIT_ENUM(AlignMode) {
  //! Align by emitting a sequence that can be executed (code).
  kAlignCode = 0,
  //! Align by emitting a sequence that shouldn't be executed (data).
  kAlignData = 1,
  //! Align by emitting a sequence of zeros.
  kAlignZero = 2
};

// ============================================================================
// [asmjit::RelocMode]
// ============================================================================

//! Relocation mode.
ASMJIT_ENUM(RelocMode) {
  //! Relocate an absolute address to an absolute address.
  kRelocAbsToAbs = 0,
  //! Relocate a relative address to an absolute address.
  kRelocRelToAbs = 1,
  //! Relocate an absolute address to a relative address.
  kRelocAbsToRel = 2,
  //! Relocate an absolute address to a relative address or use trampoline.
  kRelocTrampoline = 3
};

// ============================================================================
// [asmjit::LabelLink]
// ============================================================================

//! \internal
//!
//! Data structure used to link labels.
struct LabelLink {
  //! Previous link.
  LabelLink* prev;
  //! Offset.
  intptr_t offset;
  //! Inlined displacement.
  intptr_t displacement;
  //! RelocId in case the link has to be absolute after relocated.
  intptr_t relocId;
};

// ============================================================================
// [asmjit::LabelData]
// ============================================================================

//! \internal
//!
//! Label data.
struct LabelData {
  //! Label offset.
  intptr_t offset;
  //! Label links chain.
  LabelLink* links;

  //! External tool ID, if linked to any.
  uint64_t exId;
  //! Pointer to a data that `ExternalTool` associated with the label.
  void* exData;
};

// ============================================================================
// [asmjit::RelocData]
// ============================================================================

//! \internal
//!
//! Code relocation data (relative vs. absolute addresses).
//!
//! X86/X64 Specific
//! ----------------
//!
//! X86 architecture uses 32-bit absolute addressing model by memory operands,
//! but 64-bit mode uses relative addressing model (RIP + displacement). In
//! code we are always using relative addressing model for referencing labels
//! and embedded data. In 32-bit mode we must patch all references to absolute
//! address before we can call generated function.
struct RelocData {
  //! Type of relocation.
  uint32_t type;
  //! Size of relocation (4 or 8 bytes).
  uint32_t size;

  //! Offset from the initial code address.
  Ptr from;
  //! Relative displacement from the initial code address or from the absolute address.
  Ptr data;
};

// ============================================================================
// [asmjit::ErrorHandler]
// ============================================================================

//! Error handler.
//!
//! Error handler can be used to override the default behavior of `Assembler`
//! error handling and propagation. See `handleError()` on how to override it.
//!
//! Please note that `addRef` and `release` functions are used, but there is
//! no reference counting implemented by default, reimplement to change the
//! default behavior.
struct ASMJIT_VIRTAPI ErrorHandler {
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Create a new `ErrorHandler` instance.
  ASMJIT_API ErrorHandler() noexcept;
  //! Destroy the `ErrorHandler` instance.
  ASMJIT_API virtual ~ErrorHandler() noexcept;

  // --------------------------------------------------------------------------
  // [AddRef / Release]
  // --------------------------------------------------------------------------

  //! Reference this error handler.
  //!
  //! \note This member function is provided for convenience. The default
  //! implementation does nothing. If you are working in environment where
  //! multiple `ErrorHandler` instances are used by a different code generators
  //! you may provide your own functionality for reference counting. In that
  //! case `addRef()` and `release()` functions should be overridden.
  ASMJIT_API virtual ErrorHandler* addRef() const noexcept;

  //! Release this error handler.
  //!
  //! \note This member function is provided for convenience. See `addRef()`
  //! for more detailed information related to reference counting.
  ASMJIT_API virtual void release() noexcept;

  // --------------------------------------------------------------------------
  // [Handle Error]
  // --------------------------------------------------------------------------

  //! Error handler (pure).
  //!
  //! Error handler is called after an error happened. An error can happen in
  //! many places, but error handler is mostly used by `Assembler` to report
  //! anything a fatal problem. There are multiple ways how the error handler
  //! can be used:
  //!
  //! 1. Returning `true` or `false` from `handleError()`. If `true` is
  //!    returned it means that error was reported and AsmJit can continue
  //!    with code-generation. However, `false` reports to AsmJit that the
  //!    error cannot be handled, in such case it stores the error in
  //!    `Assembler` and puts it into an error state. The error is accessible
  //!    through `Assembler::getLastError(). Returning `false` is default when
  //!    no error handler is used.
  //!
  //! 2. AsmJit doesn't use exception handling so your error should also not
  //!    throw an exception, however, it's possible to use plain old C's
  //!    `setjmp()` and `longjmp()`. Asmjit always puts `Assembler` and
  //!    `Compiler` to a consistent state before calling the `handleError()`,
  //!    so you can use `longjmp()` to leave the code-generation if an error
  //!    happened.
  virtual bool handleError(Error code, const char* message, void* origin) noexcept = 0;
};

// ============================================================================
// [asmjit::ExternalTool]
// ============================================================================

//! An external tool (i.e. `Stream` or `Compiler`) that can serialize to `Assembler`
struct ASMJIT_VIRTAPI ExternalTool {
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  ASMJIT_API ExternalTool() noexcept;
  ASMJIT_API virtual ~ExternalTool() noexcept;

  // --------------------------------------------------------------------------
  // [Attach / Reset]
  // --------------------------------------------------------------------------

  //! \internal
  //!
  //! Called to attach this code generator to the `assembler`.
  virtual Error attach(Assembler* assembler) noexcept = 0;

  //! Reset the code-generator (also detaches if attached).
  virtual void reset(bool releaseMemory) noexcept = 0;

  // --------------------------------------------------------------------------
  // [Finalize]
  // --------------------------------------------------------------------------

  //! Finalize the code-generation.
  //!
  //! The finalization has two passes:
  //!  - serializes code to the attached assembler.
  //!  - resets the `ExternalTool` (detaching from the `Assembler as well) so
  //!    it can be reused or destroyed).
  virtual Error finalize() noexcept = 0;

  // --------------------------------------------------------------------------
  // [Runtime / Assembler]
  // --------------------------------------------------------------------------

  //! Get the `Runtime` instance that is associated with the code-generator.
  ASMJIT_INLINE Runtime* getRuntime() const noexcept { return _runtime; }
  //! Get the `Assembler` instance that is associated with the code-generator.
  ASMJIT_INLINE Assembler* getAssembler() const noexcept { return _assembler; }

  // --------------------------------------------------------------------------
  // [Architecture]
  // --------------------------------------------------------------------------

  //! Get the target architecture.
  ASMJIT_INLINE uint32_t getArch() const noexcept { return _arch; }
  //! Get the default register size - 4 or 8 bytes, depends on the target.
  ASMJIT_INLINE uint32_t getRegSize() const noexcept { return _regSize; }

  // --------------------------------------------------------------------------
  // [Error Handling]
  // --------------------------------------------------------------------------

  //! Get the last error code.
  ASMJIT_INLINE Error getLastError() const noexcept { return _lastError; }
  //! Set the last error code and propagate it through the error handler.
  ASMJIT_API Error setLastError(Error error, const char* message = nullptr) noexcept;
  //! Clear the last error code.
  ASMJIT_INLINE void resetLastError() noexcept { _lastError = kErrorOk; }

  // --------------------------------------------------------------------------
  // [ID]
  // --------------------------------------------------------------------------

  //! Get the tool ID, provided by `Assembler` when attached to it.
  ASMJIT_INLINE uint64_t getExId() const noexcept { return _exId; }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! Associated runtime.
  Runtime* _runtime;
  //! Associated assembler.
  Assembler* _assembler;

  //! `ExternalTool` ID, provided by `Assembler`.
  //!
  //! If multiple high-evel code generators are associated with a single
  //! assembler the `_exId` member can be used to distinguish between them and
  //! to provide a mechanism to check whether the high-level code generator is
  //! accessing the resource it really owns.
  uint64_t _exId;

  //! Target's architecture ID.
  uint8_t _arch;
  //! Target's architecture GP register size in bytes (4 or 8).
  uint8_t _regSize;
  //! The code generator has been finalized.
  uint8_t _finalized;
  //! \internal
  uint8_t _reserved;
  //! Last error code.
  uint32_t _lastError;
};

// ============================================================================
// [asmjit::Assembler]
// ============================================================================

//! Base assembler.
//!
//! This class implements a base interface that is used by architecture
//! specific assemblers.
//!
//! \sa Compiler.
struct ASMJIT_VIRTAPI Assembler {
  ASMJIT_NO_COPY(Assembler)

  // --------------------------------------------------------------------------
  // [Options]
  // --------------------------------------------------------------------------

  //! Assembler options.
  ASMJIT_ENUM(Options) {
    //! Emit optimized code-alignment sequences (`Assembler` and `Compiler`).
    //!
    //! Default `true`.
    //!
    //! X86/X64 Specific
    //! ----------------
    //!
    //! Default align sequence used by X86/X64 architecture is one-byte 0x90
    //! opcode that is mostly shown by disassemblers as nop. However there are
    //! more optimized align sequences for 2-11 bytes that may execute faster.
    //! If this feature is enabled asmjit will generate specialized sequences
    //! for alignment between 1 to 11 bytes. Also when `X86Compiler` is used,
    //! it can add REX prefixes into the code to make some instructions greater
    //! so no alignment sequence is needed.
    kOptionOptimizedAlign = 0,

    //! Emit jump-prediction hints (`Assembler` and `Compiler`).
    //!
    //! Default `false`.
    //!
    //! X86/X64 Specific
    //! ----------------
    //!
    //! Jump prediction is usually based on the direction of the jump. If the
    //! jump is backward it is usually predicted as taken; and if the jump is
    //! forward it is usually predicted as not-taken. The reason is that loops
    //! generally use backward jumps and conditions usually use forward jumps.
    //! However this behavior can be overridden by using instruction prefixes.
    //! If this option is enabled these hints will be emitted.
    //!
    //! This feature is disabled by default, because the only processor that
    //! used to take into consideration prediction hints was P4. Newer processors
    //! implement heuristics for branch prediction that ignores any static hints.
    kOptionPredictedJumps = 1
  };

  // --------------------------------------------------------------------------
  // [Buffer]
  // --------------------------------------------------------------------------

  //! Code or data buffer.
  struct Buffer {
    //! Code data.
    uint8_t* data;
    //! Total length of `data` in bytes.
    size_t capacity;
    //! Number of bytes of `data` used.
    size_t length;
    //! Current offset (assembler's cursor) in bytes.
    size_t offset;
  };

  // --------------------------------------------------------------------------
  // [Section]
  // --------------------------------------------------------------------------

  //! Code or data section.
  struct Section {
    //! Section id.
    uint32_t id;
    //! Section flags.
    uint32_t flags;
    //! Section name (limited to 35 characters, PE allows max 8 chars).
    char name[36];
    //! Section alignment requirements (0 if no requirements).
    uint32_t alignment;
    //! Section content.
    Buffer content;
  };

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Create a new `Assembler` instance.
  ASMJIT_API Assembler(Runtime* runtime) noexcept;
  //! Destroy the `Assembler` instance.
  ASMJIT_API virtual ~Assembler() noexcept;

  // --------------------------------------------------------------------------
  // [Reset]
  // --------------------------------------------------------------------------

  //! Reset the assembler.
  //!
  //! If `releaseMemory` is true all buffers will be released to the system.
  ASMJIT_API void reset(bool releaseMemory = false) noexcept;

  // --------------------------------------------------------------------------
  // [Runtime]
  // --------------------------------------------------------------------------

  //! Get the runtime associated with the assembler.
  //!
  //! NOTE: Runtime is persistent across `reset()` calls.
  ASMJIT_INLINE Runtime* getRuntime() const noexcept { return _runtime; }

  // --------------------------------------------------------------------------
  // [Architecture]
  // --------------------------------------------------------------------------

  //! Get the target architecture.
  ASMJIT_INLINE uint32_t getArch() const noexcept { return _arch; }
  //! Get the default register size - 4 or 8 bytes, depends on the target.
  ASMJIT_INLINE uint32_t getRegSize() const noexcept { return _regSize; }

  // --------------------------------------------------------------------------
  // [Logging]
  // --------------------------------------------------------------------------

#if !defined(ASMJIT_DISABLE_LOGGER)
  //! Get whether the assembler has a logger.
  ASMJIT_INLINE bool hasLogger() const noexcept { return _logger != nullptr; }
  //! Get the logger.
  ASMJIT_INLINE Logger* getLogger() const noexcept { return _logger; }
  //! Set the logger to `logger`.
  ASMJIT_INLINE void setLogger(Logger* logger) noexcept { _logger = logger; }
#endif // !ASMJIT_DISABLE_LOGGER

  // --------------------------------------------------------------------------
  // [Error Handling]
  // --------------------------------------------------------------------------

  //! Get the error handler.
  ASMJIT_INLINE ErrorHandler* getErrorHandler() const noexcept { return _errorHandler; }
  //! Set the error handler.
  ASMJIT_API Error setErrorHandler(ErrorHandler* handler) noexcept;
  //! Clear the error handler.
  ASMJIT_INLINE Error resetErrorHandler() noexcept { return setErrorHandler(nullptr); }

  //! Get the last error code.
  ASMJIT_INLINE Error getLastError() const noexcept { return _lastError; }
  //! Set the last error code and propagate it through the error handler.
  ASMJIT_API Error setLastError(Error error, const char* message = nullptr) noexcept;
  //! Clear the last error code.
  ASMJIT_INLINE void resetLastError() noexcept { _lastError = kErrorOk; }

  // --------------------------------------------------------------------------
  // [Serializers]
  // --------------------------------------------------------------------------

  //! \internal
  //!
  //! Called after the code generator `cg` has been attached to the assembler.
  ASMJIT_INLINE void _attached(ExternalTool* exTool) noexcept {
    exTool->_runtime = getRuntime();
    exTool->_assembler = this;
    exTool->_exId = _nextExId();
    _exCountAttached++;
  }

  //! \internal
  //!
  //! Called after the code generator `cg` has been detached from the assembler.
  ASMJIT_INLINE void _detached(ExternalTool* exTool) noexcept {
    exTool->_runtime = nullptr;
    exTool->_assembler = nullptr;
    exTool->_exId = 0;
    _exCountAttached--;
  }

  //! \internal
  //!
  //! Return a new code-gen ID (always greater than zero).
  ASMJIT_INLINE uint64_t _nextExId() noexcept {
    ASMJIT_ASSERT(_exIdGenerator != ASMJIT_UINT64_C(0xFFFFFFFFFFFFFFFF));
    return ++_exIdGenerator;
  }

  // --------------------------------------------------------------------------
  // [Assembler Options]
  // --------------------------------------------------------------------------

  //! Get global assembler options.
  ASMJIT_INLINE uint32_t getAsmOptions() const noexcept {
    return _asmOptions;
  }
  //! Get whether the global assembler `option` is turned on.
  ASMJIT_INLINE bool hasAsmOption(uint32_t option) const noexcept {
    return (_asmOptions & option) != 0;
  }
  //! Turn on global assembler `options`.
  ASMJIT_INLINE void addAsmOptions(uint32_t options) noexcept {
    _asmOptions |= options;
  }
  //! Turn off global assembler `options`.
  ASMJIT_INLINE void clearAsmOptions(uint32_t options) noexcept {
    _asmOptions &= ~options;
  }

  // --------------------------------------------------------------------------
  // [Instruction Options]
  // --------------------------------------------------------------------------

  //! Get options of the next instruction.
  ASMJIT_INLINE uint32_t getInstOptions() const noexcept {
    return _instOptions;
  }
  //! Set options of the next instruction.
  ASMJIT_INLINE void setInstOptions(uint32_t instOptions) noexcept {
    _instOptions = instOptions;
  }
  //! Get options of the next instruction and reset them.
  ASMJIT_INLINE uint32_t getInstOptionsAndReset() noexcept {
    uint32_t instOptions = _instOptions;
    _instOptions = 0;
    return instOptions;
  };

  // --------------------------------------------------------------------------
  // [Code-Buffer]
  // --------------------------------------------------------------------------

  //! Grow the code-buffer.
  //!
  //! The internal code-buffer will grow at least by `n` bytes so `n` bytes can
  //! be added to it. If `n` is zero or `getOffset() + n` is not greater than
  //! the current capacity of the code-buffer this function does nothing.
  ASMJIT_API Error _grow(size_t n) noexcept;
  //! Reserve the code-buffer to at least `n` bytes.
  ASMJIT_API Error _reserve(size_t n) noexcept;

  //! Get capacity of the code-buffer.
  ASMJIT_INLINE size_t getCapacity() const noexcept {
    return (size_t)(_end - _buffer);
  }
  //! Get the number of remaining bytes in code-buffer.
  ASMJIT_INLINE size_t getRemainingSpace() const noexcept {
    return (size_t)(_end - _cursor);
  }

  //! Get current offset in buffer, same as `getOffset() + getTramplineSize()`.
  ASMJIT_INLINE size_t getCodeSize() const noexcept {
    return getOffset() + getTrampolinesSize();
  }

  //! Get size of all possible trampolines.
  //!
  //! Trampolines are needed to successfuly generate relative jumps to absolute
  //! addresses. This value is only non-zero if jmp of call instructions were
  //! used with immediate operand (this means jumping or calling an absolute
  //! address directly).
  ASMJIT_INLINE size_t getTrampolinesSize() const noexcept { return _trampolinesSize; }

  //! Get code-buffer.
  ASMJIT_INLINE uint8_t* getBuffer() const noexcept { return _buffer; }
  //! Get the end of the code-buffer (points to the first byte that is invalid).
  ASMJIT_INLINE uint8_t* getEnd() const noexcept { return _end; }

  //! Get the current position in the code-buffer.
  ASMJIT_INLINE uint8_t* getCursor() const noexcept { return _cursor; }
  //! Set the current position in the buffer.
  ASMJIT_INLINE void setCursor(uint8_t* cursor) noexcept {
    ASMJIT_ASSERT(cursor >= _buffer && cursor <= _end);
    _cursor = cursor;
  }

  //! Get the current offset in the buffer.
  ASMJIT_INLINE size_t getOffset() const noexcept { return (size_t)(_cursor - _buffer); }
  //! Set the current offset in the buffer to `offset` and return the previous value.
  ASMJIT_INLINE size_t setOffset(size_t offset) noexcept {
    ASMJIT_ASSERT(offset < getCapacity());

    size_t oldOffset = (size_t)(_cursor - _buffer);
    _cursor = _buffer + offset;
    return oldOffset;
  }

  //! Read `int8_t` at index `pos`.
  ASMJIT_INLINE int32_t readI8At(size_t pos) const noexcept {
    ASMJIT_ASSERT(pos + 1 <= (size_t)(_end - _buffer));
    return Utils::readI8(_buffer + pos);
  }

  //! Read `uint8_t` at index `pos`.
  ASMJIT_INLINE uint32_t readU8At(size_t pos) const noexcept {
    ASMJIT_ASSERT(pos + 1 <= (size_t)(_end - _buffer));
    return Utils::readU8(_buffer + pos);
  }

  //! Read `int16_t` at index `pos`.
  ASMJIT_INLINE int32_t readI16At(size_t pos) const noexcept {
    ASMJIT_ASSERT(pos + 2 <= (size_t)(_end - _buffer));
    return Utils::readI16u(_buffer + pos);
  }

  //! Read `uint16_t` at index `pos`.
  ASMJIT_INLINE uint32_t readU16At(size_t pos) const noexcept {
    ASMJIT_ASSERT(pos + 2 <= (size_t)(_end - _buffer));
    return Utils::readU16u(_buffer + pos);
  }

  //! Read `int32_t` at index `pos`.
  ASMJIT_INLINE int32_t readI32At(size_t pos) const noexcept {
    ASMJIT_ASSERT(pos + 4 <= (size_t)(_end - _buffer));
    return Utils::readI32u(_buffer + pos);
  }

  //! Read `uint32_t` at index `pos`.
  ASMJIT_INLINE uint32_t readU32At(size_t pos) const noexcept {
    ASMJIT_ASSERT(pos + 4 <= (size_t)(_end - _buffer));
    return Utils::readU32u(_buffer + pos);
  }

  //! Read `uint64_t` at index `pos`.
  ASMJIT_INLINE int64_t readI64At(size_t pos) const noexcept {
    ASMJIT_ASSERT(pos + 8 <= (size_t)(_end - _buffer));
    return Utils::readI64u(_buffer + pos);
  }

  //! Read `uint64_t` at index `pos`.
  ASMJIT_INLINE uint64_t readU64At(size_t pos) const noexcept {
    ASMJIT_ASSERT(pos + 8 <= (size_t)(_end - _buffer));
    return Utils::readU64u(_buffer + pos);
  }

  //! Write `int8_t` at index `pos`.
  ASMJIT_INLINE void writeI8At(size_t pos, int32_t x) noexcept {
    ASMJIT_ASSERT(pos + 1 <= (size_t)(_end - _buffer));
    Utils::writeI8(_buffer + pos, x);
  }

  //! Write `uint8_t` at index `pos`.
  ASMJIT_INLINE void writeU8At(size_t pos, uint32_t x) noexcept {
    ASMJIT_ASSERT(pos + 1 <= (size_t)(_end - _buffer));
    Utils::writeU8(_buffer + pos, x);
  }

  //! Write `int8_t` at index `pos`.
  ASMJIT_INLINE void writeI16At(size_t pos, int32_t x) noexcept {
    ASMJIT_ASSERT(pos + 2 <= (size_t)(_end - _buffer));
    Utils::writeI16u(_buffer + pos, x);
  }

  //! Write `uint8_t` at index `pos`.
  ASMJIT_INLINE void writeU16At(size_t pos, uint32_t x) noexcept {
    ASMJIT_ASSERT(pos + 2 <= (size_t)(_end - _buffer));
    Utils::writeU16u(_buffer + pos, x);
  }

  //! Write `int32_t` at index `pos`.
  ASMJIT_INLINE void writeI32At(size_t pos, int32_t x) noexcept {
    ASMJIT_ASSERT(pos + 4 <= (size_t)(_end - _buffer));
    Utils::writeI32u(_buffer + pos, x);
  }

  //! Write `uint32_t` at index `pos`.
  ASMJIT_INLINE void writeU32At(size_t pos, uint32_t x) noexcept {
    ASMJIT_ASSERT(pos + 4 <= (size_t)(_end - _buffer));
    Utils::writeU32u(_buffer + pos, x);
  }

  //! Write `int64_t` at index `pos`.
  ASMJIT_INLINE void writeI64At(size_t pos, int64_t x) noexcept {
    ASMJIT_ASSERT(pos + 8 <= (size_t)(_end - _buffer));
    Utils::writeI64u(_buffer + pos, x);
  }

  //! Write `uint64_t` at index `pos`.
  ASMJIT_INLINE void writeU64At(size_t pos, uint64_t x) noexcept {
    ASMJIT_ASSERT(pos + 8 <= (size_t)(_end - _buffer));
    Utils::writeU64u(_buffer + pos, x);
  }

  // --------------------------------------------------------------------------
  // [Embed]
  // --------------------------------------------------------------------------

  //! Embed raw data into the code-buffer.
  ASMJIT_API virtual Error embed(const void* data, uint32_t size) noexcept;

  // --------------------------------------------------------------------------
  // [Align]
  // --------------------------------------------------------------------------

  //! Align target buffer to the `offset` specified.
  //!
  //! The sequence that is used to fill the gap between the aligned location
  //! and the current depends on `alignMode`, see \ref AlignMode.
  virtual Error align(uint32_t alignMode, uint32_t offset) noexcept = 0;

  // --------------------------------------------------------------------------
  // [Label]
  // --------------------------------------------------------------------------

  //! Get number of labels created.
  ASMJIT_INLINE size_t getLabelsCount() const noexcept {
    return _labels.getLength();
  }

  //! Get whether the `label` is valid (i.e. registered).
  ASMJIT_INLINE bool isLabelValid(const Label& label) const noexcept {
    return isLabelValid(label.getId());
  }
  //! Get whether the label `id` is valid (i.e. registered).
  ASMJIT_INLINE bool isLabelValid(uint32_t id) const noexcept {
    return static_cast<size_t>(id) < _labels.getLength();
  }

  //! Get whether the `label` is bound.
  //!
  //! \note It's an error to pass label that is not valid. Check the validity
  //! of the label by using `isLabelValid()` method before the bound check if
  //! you are not sure about its validity, otherwise you may hit an assertion
  //! failure in debug mode, and undefined behavior in release mode.
  ASMJIT_INLINE bool isLabelBound(const Label& label) const noexcept {
    return isLabelBound(label.getId());
  }
  //! \overload
  ASMJIT_INLINE bool isLabelBound(uint32_t id) const noexcept {
    ASMJIT_ASSERT(isLabelValid(id));
    return _labels[id]->offset != -1;
  }

  //! Get a `label` offset or -1 if the label is not yet bound.
  ASMJIT_INLINE intptr_t getLabelOffset(const Label& label) const noexcept {
    return getLabelOffset(label.getId());
  }
  //! \overload
  ASMJIT_INLINE intptr_t getLabelOffset(uint32_t id) const noexcept {
    ASMJIT_ASSERT(isLabelValid(id));
    return _labels[id]->offset;
  }

  //! Get `LabelData` by `label`.
  ASMJIT_INLINE LabelData* getLabelData(const Label& label) const noexcept {
    return getLabelData(label.getId());
  }
  //! \overload
  ASMJIT_INLINE LabelData* getLabelData(uint32_t id) const noexcept {
    ASMJIT_ASSERT(isLabelValid(id));
    return const_cast<LabelData*>(_labels[id]);
  }

  //! \internal
  //!
  //! Create a new label and return its ID.
  ASMJIT_API uint32_t _newLabelId() noexcept;

  //! \internal
  //!
  //! New LabelLink instance.
  ASMJIT_API LabelLink* _newLabelLink() noexcept;

  //! Create and return a new `Label`.
  ASMJIT_INLINE Label newLabel() noexcept { return Label(_newLabelId()); }

  //! Bind the `label` to the current offset.
  //!
  //! \note Label can be bound only once!
  ASMJIT_API virtual Error bind(const Label& label) noexcept;

  // --------------------------------------------------------------------------
  // [Reloc]
  // --------------------------------------------------------------------------

  //! Relocate the code to `baseAddress` and copy it to `dst`.
  //!
  //! \param dst Contains the location where the relocated code should be
  //! copied. The pointer can be address returned by virtual memory allocator
  //! or any other address that has sufficient space.
  //!
  //! \param baseAddress Base address used for relocation. The `JitRuntime`
  //! always sets the `baseAddress` address to be the same as `dst`, but other
  //! runtimes, for example `StaticRuntime`, do not have to follow this rule.
  //!
  //! \retval The number bytes actually used. If the code generator reserved
  //! space for possible trampolines, but didn't use it, the number of bytes
  //! used can actually be less than the expected worst case. Virtual memory
  //! allocator can shrink the memory allocated first time.
  //!
  //! A given buffer will be overwritten, to get the number of bytes required,
  //! use `getCodeSize()`.
  ASMJIT_API size_t relocCode(void* dst, Ptr baseAddress = kNoBaseAddress) const noexcept;

  //! \internal
  //!
  //! Reloc code.
  virtual size_t _relocCode(void* dst, Ptr baseAddress) const noexcept = 0;

  // --------------------------------------------------------------------------
  // [Make]
  // --------------------------------------------------------------------------

  ASMJIT_API virtual void* make() noexcept;

  // --------------------------------------------------------------------------
  // [Emit]
  // --------------------------------------------------------------------------

  //! Emit an instruction (virtual).
  virtual Error _emit(uint32_t code, const Operand& o0, const Operand& o1, const Operand& o2, const Operand& o3) = 0;

  //! Emit an instruction.
  ASMJIT_API Error emit(uint32_t code);
  //! \overload
  ASMJIT_API Error emit(uint32_t code, const Operand& o0);
  //! \overload
  ASMJIT_API Error emit(uint32_t code, const Operand& o0, const Operand& o1);
  //! \overload
  ASMJIT_API Error emit(uint32_t code, const Operand& o0, const Operand& o1, const Operand& o2);
  //! \overload
  ASMJIT_API Error emit(uint32_t code, const Operand& o0, const Operand& o1, const Operand& o2, const Operand& o3);

  //! Emit an instruction that has an immediate operand.
  ASMJIT_API Error emit(uint32_t code, int o0);
  //! \overload
  ASMJIT_API Error emit(uint32_t code, const Operand& o0, int o1);
  //! \overload
  ASMJIT_API Error emit(uint32_t code, const Operand& o0, const Operand& o1, int o2);
  //! \overload
  ASMJIT_API Error emit(uint32_t code, const Operand& o0, const Operand& o1, const Operand& o2, int o3);

  //! \overload
  ASMJIT_API Error emit(uint32_t code, int64_t o0);
  //! \overload
  ASMJIT_API Error emit(uint32_t code, const Operand& o0, int64_t o1);
  //! \overload
  ASMJIT_API Error emit(uint32_t code, const Operand& o0, const Operand& o1, int64_t o2);
  //! \overload
  ASMJIT_API Error emit(uint32_t code, const Operand& o0, const Operand& o1, const Operand& o2, int64_t o3);

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! Associated runtime.
  Runtime* _runtime;
  //! Associated logger.
  Logger* _logger;
  //! Associated error handler, triggered by \ref setLastError().
  ErrorHandler* _errorHandler;

  //! Target architecture ID.
  uint8_t _arch;
  //! Target architecture GP register size in bytes (4 or 8).
  uint8_t _regSize;
  //! \internal
  uint16_t _reserved;

  //! Assembler options, used by \ref getAsmOptions() and \ref hasAsmOption().
  uint32_t _asmOptions;
  //! Instruction options, affect the next instruction that will be emitted.
  uint32_t _instOptions;
  //! Last error code.
  uint32_t _lastError;

  //! External tool ID generator.
  uint64_t _exIdGenerator;
  //! Count of external tools currently attached.
  size_t _exCountAttached;

  //! General purpose zone allocator.
  Zone _zoneAllocator;

  //! Start of the code-buffer of the current section.
  uint8_t* _buffer;
  //! End of the code-buffer of the current section (points to the first invalid byte).
  uint8_t* _end;
  //! The current position in `_buffer` of the current section.
  uint8_t* _cursor;

  //! Size of all possible trampolines.
  uint32_t _trampolinesSize;

  //! Inline comment that will be logged by the next instruction and set to nullptr.
  const char* _comment;
  //! Unused `LabelLink` structures pool.
  LabelLink* _unusedLinks;

  //! Assembler sections.
  PodVectorTmp<Section*, 4> _sections;
  //! Assembler labels.
  PodVectorTmp<LabelData*, 16> _labels;
  //! Table of relocations.
  PodVector<RelocData> _relocations;
};

//! \}

} // asmjit namespace

// [Api-End]
#include "../apiend.h"

// [Guard]
#endif // _ASMJIT_BASE_ASSEMBLER_H
