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
#include "../base/runtime.h"
#include "../base/zone.h"

// [Api-Begin]
#include "../apibegin.h"

namespace asmjit {

//! \addtogroup asmjit_base
//! \{

// ============================================================================
// [asmjit::AssemblerFeatures]
// ============================================================================

//! Features of \ref Assembler.
ASMJIT_ENUM(AssemblerFeatures) {
  //! Emit optimized code-alignment sequences (`Assembler` and `Compiler`).
  //!
  //! Default `true`.
  //!
  //! X86/X64
  //! -------
  //!
  //! Default align sequence used by X86/X64 architecture is one-byte 0x90
  //! opcode that is mostly shown by disassemblers as nop. However there are
  //! more optimized align sequences for 2-11 bytes that may execute faster.
  //! If this feature is enabled asmjit will generate specialized sequences
  //! for alignment between 1 to 11 bytes. Also when `X86Compiler` is used,
  //! it can add REX prefixes into the code to make some instructions greater
  //! so no alignment sequence is needed.
  kAssemblerFeatureOptimizedAlign = 0,

  //! Emit jump-prediction hints (`Assembler` and `Compiler`).
  //!
  //! Default `false`.
  //!
  //! X86/X64
  //! -------
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
  kAssemblerFeaturePredictedJumps = 1
};

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

  //! An ID of a code-generator that created this label.
  uint64_t hlId;
  //! Pointer to the data the code-generator associated with the label.
  void* hlData;
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
//! Error handler can be used to override the default behavior of `CodeGen`
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
  ASMJIT_API ErrorHandler();
  //! Destroy the `ErrorHandler` instance.
  ASMJIT_API virtual ~ErrorHandler();

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
  ASMJIT_API virtual ErrorHandler* addRef() const;

  //! Release this error handler.
  //!
  //! \note This member function is provided for convenience. See `addRef()`
  //! for more detailed information related to reference counting.
  ASMJIT_API virtual void release();

  // --------------------------------------------------------------------------
  // [Handle Error]
  // --------------------------------------------------------------------------

  //! Error handler (pure).
  //!
  //! Error handler is called when an error happened. An error can happen in
  //! many places, but error handler is mostly used by `Assembler` and
  //! `Compiler` classes to report anything that may cause incorrect code
  //! generation. There are multiple ways how the error handler can be used
  //! and each has it's pros/cons.
  //!
  //! AsmJit library doesn't use exceptions and can be compiled with or without
  //! exception handling support. Even if the AsmJit library is compiled without
  //! exceptions it is exception-safe and handleError() can report an incoming
  //! error by throwing an exception of any type. It's guaranteed that the
  //! exception won't be catched by AsmJit and will be propagated to the code
  //! calling AsmJit `Assembler` or `Compiler` methods. Alternative to
  //! throwing an exception is using `setjmp()` and `longjmp()` pair available
  //! in the standard C library.
  //!
  //! If the exception or setjmp() / longjmp() mechanism is used, the state of
  //! the `BaseAssember` or `Compiler` is unchanged and if it's possible the
  //! execution (instruction serialization) can continue. However if the error
  //! happened during any phase that translates or modifies the stored code
  //! (for example relocation done by `Assembler` or analysis/translation
  //! done by `Compiler`) the execution can't continue and the error will
  //! be also stored in `Assembler` or `Compiler`.
  //!
  //! Finally, if no exceptions nor setjmp() / longjmp() mechanisms were used,
  //! you can still implement a compatible handling by returning from your
  //! error handler. Returning `true` means that error was reported and AsmJit
  //! should continue execution, but `false` sets the error immediately to the
  //! `Assembler` or `Compiler` and execution shouldn't continue (this is the
  //! default behavior in case no error handler is used).
  virtual bool handleError(Error code, const char* message, void* origin) = 0;
};

// ============================================================================
// [asmjit::CodeGen]
// ============================================================================

//! Interface to implement an external code generator (i.e. `Compiler`).
struct ASMJIT_VIRTAPI CodeGen {
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  ASMJIT_API CodeGen();
  ASMJIT_API virtual ~CodeGen();

  // --------------------------------------------------------------------------
  // [Attach / Reset]
  // --------------------------------------------------------------------------

  //! \internal
  //!
  //! Called to attach this code generator to the `assembler`.
  virtual Error attach(Assembler* assembler) = 0;

  //! Reset the code-generator (also detaches if attached).
  virtual void reset(bool releaseMemory) = 0;

  // --------------------------------------------------------------------------
  // [Finalize]
  // --------------------------------------------------------------------------

  //! Finalize the code-generation.
  //!
  //! The finalization has two passes:
  //!  - serializes code to the attached assembler.
  //!  - resets the `CodeGen` (detaching from the `Assembler as well) so it can
  //!    be reused or destroyed.
  virtual Error finalize() = 0;

  // --------------------------------------------------------------------------
  // [Runtime / Assembler]
  // --------------------------------------------------------------------------

  //! Get the `Runtime` instance that is associated with the code-generator.
  ASMJIT_INLINE Runtime* getRuntime() const { return _runtime; }
  //! Get the `Assembler` instance that is associated with the code-generator.
  ASMJIT_INLINE Assembler* getAssembler() const { return _assembler; }

  // --------------------------------------------------------------------------
  // [Architecture]
  // --------------------------------------------------------------------------

  //! Get the target architecture.
  ASMJIT_INLINE uint32_t getArch() const { return _arch; }
  //! Get the default register size - 4 or 8 bytes, depends on the target.
  ASMJIT_INLINE uint32_t getRegSize() const { return _regSize; }

  // --------------------------------------------------------------------------
  // [Error Handling]
  // --------------------------------------------------------------------------

  //! Get the last error code.
  ASMJIT_INLINE Error getLastError() const { return _lastError; }
  //! Set the last error code and propagate it through the error handler.
  ASMJIT_API Error setLastError(Error error, const char* message = NULL);
  //! Clear the last error code.
  ASMJIT_INLINE void resetLastError() { _lastError = kErrorOk; }

  // --------------------------------------------------------------------------
  // [CodeGen]
  // --------------------------------------------------------------------------

  //! Get the code-generator ID, provided by `Assembler` when attached to it.
  ASMJIT_INLINE uint64_t getHLId() const { return _hlId; }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! Associated runtime.
  Runtime* _runtime;
  //! Associated assembler.
  Assembler* _assembler;

  //! High-level ID, provided by `Assembler`.
  //!
  //! If multiple high-evel code generators are associated with a single
  //! assembler the `_hlId` member can be used to distinguish between them and
  //! to provide a mechanism to check whether the high-level code generator is
  //! accessing the resource it really owns.
  uint64_t _hlId;

  //! Target architecture ID.
  uint8_t _arch;
  //! Target architecture GP register size in bytes (4 or 8).
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
//! This class implements the base interface that is used by architecture
//! specific assemblers.
//!
//! \sa Compiler.
struct ASMJIT_VIRTAPI Assembler {
  ASMJIT_NO_COPY(Assembler)

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Create a new `Assembler` instance.
  ASMJIT_API Assembler(Runtime* runtime);
  //! Destroy the `Assembler` instance.
  ASMJIT_API virtual ~Assembler();

  // --------------------------------------------------------------------------
  // [Reset]
  // --------------------------------------------------------------------------

  //! Reset the assembler.
  //!
  //! If `releaseMemory` is true all buffers will be released to the system.
  ASMJIT_API void reset(bool releaseMemory = false);

  // --------------------------------------------------------------------------
  // [Runtime]
  // --------------------------------------------------------------------------

  //! Get the runtime associated with the assembler.
  //!
  //! NOTE: Runtime is persistent across `reset()` calls.
  ASMJIT_INLINE Runtime* getRuntime() const { return _runtime; }

  // --------------------------------------------------------------------------
  // [Architecture]
  // --------------------------------------------------------------------------

  //! Get the target architecture.
  ASMJIT_INLINE uint32_t getArch() const { return _arch; }
  //! Get the default register size - 4 or 8 bytes, depends on the target.
  ASMJIT_INLINE uint32_t getRegSize() const { return _regSize; }

  // --------------------------------------------------------------------------
  // [Logging]
  // --------------------------------------------------------------------------

#if !defined(ASMJIT_DISABLE_LOGGER)
  //! Get whether the assembler has a logger.
  ASMJIT_INLINE bool hasLogger() const { return _logger != NULL; }
  //! Get the logger.
  ASMJIT_INLINE Logger* getLogger() const { return _logger; }
  //! Set the logger to `logger`.
  ASMJIT_INLINE void setLogger(Logger* logger) { _logger = logger; }
#endif // !ASMJIT_DISABLE_LOGGER

  // --------------------------------------------------------------------------
  // [Error Handling]
  // --------------------------------------------------------------------------

  //! Get the error handler.
  ASMJIT_INLINE ErrorHandler* getErrorHandler() const { return _errorHandler; }
  //! Set the error handler.
  ASMJIT_API Error setErrorHandler(ErrorHandler* handler);
  //! Clear the error handler.
  ASMJIT_INLINE Error resetErrorHandler() { return setErrorHandler(NULL); }

  //! Get the last error code.
  ASMJIT_INLINE Error getLastError() const { return _lastError; }
  //! Set the last error code and propagate it through the error handler.
  ASMJIT_API Error setLastError(Error error, const char* message = NULL);
  //! Clear the last error code.
  ASMJIT_INLINE void resetLastError() { _lastError = kErrorOk; }

  // --------------------------------------------------------------------------
  // [External CodeGen]
  // --------------------------------------------------------------------------

  //! \internal
  //!
  //! Called after the code generator `cg` has been attached to the assembler.
  ASMJIT_INLINE void _attached(CodeGen* cg) {
    cg->_runtime = getRuntime();
    cg->_assembler = this;
    cg->_hlId = _nextExternalId();
    _hlAttachedCount++;
  }

  //! \internal
  //!
  //! Called after the code generator `cg` has been detached from the assembler.
  ASMJIT_INLINE void _detached(CodeGen* cg) {
    cg->_runtime = NULL;
    cg->_assembler = NULL;
    cg->_hlId = 0;
    _hlAttachedCount--;
  }

  //! \internal
  //!
  //! Return a new code-gen ID (always greater than zero).
  ASMJIT_INLINE uint64_t _nextExternalId() {
    ASMJIT_ASSERT(_hlIdGenerator != ASMJIT_UINT64_C(0xFFFFFFFFFFFFFFFF));
    return ++_hlIdGenerator;
  }

  // --------------------------------------------------------------------------
  // [Assembler Features]
  // --------------------------------------------------------------------------

  //! Get code-generator features.
  ASMJIT_INLINE uint32_t getFeatures() const { return _features; }
  //! Set code-generator features.
  ASMJIT_INLINE void setFeatures(uint32_t features) { _features = features; }

  //! Get code-generator `feature`.
  ASMJIT_INLINE bool hasFeature(uint32_t feature) const {
    ASMJIT_ASSERT(feature < 32);
    return (_features & (1 << feature)) != 0;
  }

  //! Set code-generator `feature` to `value`.
  ASMJIT_INLINE void setFeature(uint32_t feature, bool value) {
    ASMJIT_ASSERT(feature < 32);
    feature = static_cast<uint32_t>(value) << feature;
    _features = (_features & ~feature) | feature;
  }

  // --------------------------------------------------------------------------
  // [Instruction Options]
  // --------------------------------------------------------------------------

  //! Get options of the next instruction.
  ASMJIT_INLINE uint32_t getInstOptions() const { return _instOptions; }
  //! Set options of the next instruction.
  ASMJIT_INLINE void setInstOptions(uint32_t instOptions) { _instOptions = instOptions; }

  //! Get options of the next instruction and reset them.
  ASMJIT_INLINE uint32_t getInstOptionsAndReset() {
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
  ASMJIT_API Error _grow(size_t n);
  //! Reserve the code-buffer to at least `n` bytes.
  ASMJIT_API Error _reserve(size_t n);

  //! Get capacity of the code-buffer.
  ASMJIT_INLINE size_t getCapacity() const { return (size_t)(_end - _buffer); }
  //! Get the number of remaining bytes in code-buffer.
  ASMJIT_INLINE size_t getRemainingSpace() const { return (size_t)(_end - _cursor); }

  //! Get current offset in buffer, same as `getOffset() + getTramplineSize()`.
  ASMJIT_INLINE size_t getCodeSize() const { return getOffset() + getTrampolinesSize(); }

  //! Get size of all possible trampolines.
  //!
  //! Trampolines are needed to successfuly generate relative jumps to absolute
  //! addresses. This value is only non-zero if jmp of call instructions were
  //! used with immediate operand (this means jumping or calling an absolute
  //! address directly).
  ASMJIT_INLINE size_t getTrampolinesSize() const { return _trampolinesSize; }

  //! Get code-buffer.
  ASMJIT_INLINE uint8_t* getBuffer() const { return _buffer; }
  //! Get the end of the code-buffer (points to the first byte that is invalid).
  ASMJIT_INLINE uint8_t* getEnd() const { return _end; }

  //! Get the current position in the code-buffer.
  ASMJIT_INLINE uint8_t* getCursor() const { return _cursor; }
  //! Set the current position in the buffer.
  ASMJIT_INLINE void setCursor(uint8_t* cursor) {
    ASMJIT_ASSERT(cursor >= _buffer && cursor <= _end);
    _cursor = cursor;
  }

  //! Get the current offset in the buffer.
  ASMJIT_INLINE size_t getOffset() const { return (size_t)(_cursor - _buffer); }
  //! Set the current offset in the buffer to `offset` and return the previous value.
  ASMJIT_INLINE size_t setOffset(size_t offset) {
    ASMJIT_ASSERT(offset < getCapacity());

    size_t oldOffset = (size_t)(_cursor - _buffer);
    _cursor = _buffer + offset;
    return oldOffset;
  }

  //! Get BYTE at position `pos`.
  ASMJIT_INLINE uint8_t getByteAt(size_t pos) const {
    ASMJIT_ASSERT(pos + 1 <= (size_t)(_end - _buffer));
    return *reinterpret_cast<const uint8_t*>(_buffer + pos);
  }

  //! Get WORD at position `pos`.
  ASMJIT_INLINE uint16_t getWordAt(size_t pos) const {
    ASMJIT_ASSERT(pos + 2 <= (size_t)(_end - _buffer));
    return *reinterpret_cast<const uint16_t*>(_buffer + pos);
  }

  //! Get DWORD at position `pos`.
  ASMJIT_INLINE uint32_t getDWordAt(size_t pos) const {
    ASMJIT_ASSERT(pos + 4 <= (size_t)(_end - _buffer));
    return *reinterpret_cast<const uint32_t*>(_buffer + pos);
  }

  //! Get QWORD at position `pos`.
  ASMJIT_INLINE uint64_t getQWordAt(size_t pos) const {
    ASMJIT_ASSERT(pos + 8 <= (size_t)(_end - _buffer));
    return *reinterpret_cast<const uint64_t*>(_buffer + pos);
  }

  //! Get int32_t at position `pos`.
  ASMJIT_INLINE int32_t getInt32At(size_t pos) const {
    ASMJIT_ASSERT(pos + 4 <= (size_t)(_end - _buffer));
    return *reinterpret_cast<const int32_t*>(_buffer + pos);
  }

  //! Get uint32_t at position `pos`.
  ASMJIT_INLINE uint32_t getUInt32At(size_t pos) const {
    ASMJIT_ASSERT(pos + 4 <= (size_t)(_end - _buffer));
    return *reinterpret_cast<const uint32_t*>(_buffer + pos);
  }

  //! Set BYTE at position `pos`.
  ASMJIT_INLINE void setByteAt(size_t pos, uint8_t x) {
    ASMJIT_ASSERT(pos + 1 <= (size_t)(_end - _buffer));
    *reinterpret_cast<uint8_t*>(_buffer + pos) = x;
  }

  //! Set WORD at position `pos`.
  ASMJIT_INLINE void setWordAt(size_t pos, uint16_t x) {
    ASMJIT_ASSERT(pos + 2 <= (size_t)(_end - _buffer));
    *reinterpret_cast<uint16_t*>(_buffer + pos) = x;
  }

  //! Set DWORD at position `pos`.
  ASMJIT_INLINE void setDWordAt(size_t pos, uint32_t x) {
    ASMJIT_ASSERT(pos + 4 <= (size_t)(_end - _buffer));
    *reinterpret_cast<uint32_t*>(_buffer + pos) = x;
  }

  //! Set QWORD at position `pos`.
  ASMJIT_INLINE void setQWordAt(size_t pos, uint64_t x) {
    ASMJIT_ASSERT(pos + 8 <= (size_t)(_end - _buffer));
    *reinterpret_cast<uint64_t*>(_buffer + pos) = x;
  }

  //! Set int32_t at position `pos`.
  ASMJIT_INLINE void setInt32At(size_t pos, int32_t x) {
    ASMJIT_ASSERT(pos + 4 <= (size_t)(_end - _buffer));
    *reinterpret_cast<int32_t*>(_buffer + pos) = x;
  }

  //! Set uint32_t at position `pos`.
  ASMJIT_INLINE void setUInt32At(size_t pos, uint32_t x) {
    ASMJIT_ASSERT(pos + 4 <= (size_t)(_end - _buffer));
    *reinterpret_cast<uint32_t*>(_buffer + pos) = x;
  }

  // --------------------------------------------------------------------------
  // [Embed]
  // --------------------------------------------------------------------------

  //! Embed raw data into the code-buffer.
  ASMJIT_API virtual Error embed(const void* data, uint32_t size);

  // --------------------------------------------------------------------------
  // [Align]
  // --------------------------------------------------------------------------

  //! Align target buffer to the `offset` specified.
  //!
  //! The sequence that is used to fill the gap between the aligned location
  //! and the current depends on `alignMode`, see \ref AlignMode.
  virtual Error align(uint32_t alignMode, uint32_t offset) = 0;

  // --------------------------------------------------------------------------
  // [Label]
  // --------------------------------------------------------------------------

  //! Get number of labels created.
  ASMJIT_INLINE size_t getLabelsCount() const {
    return _labelList.getLength();
  }

  //! Get whether the `label` is valid (i.e. registered).
  ASMJIT_INLINE bool isLabelValid(const Label& label) const {
    return isLabelValid(label.getId());
  }
  //! Get whether the label `id` is valid (i.e. registered).
  ASMJIT_INLINE bool isLabelValid(uint32_t id) const {
    return static_cast<size_t>(id) < _labelList.getLength();
  }

  //! Get whether the `label` is bound.
  //!
  //! \note It's an error to pass label that is not valid. Check the validity
  //! of the label by using `isLabelValid()` method before the bound check if
  //! you are not sure about its validity, otherwise you may hit an assertion
  //! failure in debug mode, and undefined behavior in release mode.
  ASMJIT_INLINE bool isLabelBound(const Label& label) const {
    return isLabelBound(label.getId());
  }
  //! \overload
  ASMJIT_INLINE bool isLabelBound(uint32_t id) const {
    ASMJIT_ASSERT(isLabelValid(id));

    return _labelList[id]->offset != -1;
  }

  //! Get a `label` offset or -1 if the label is not yet bound.
  ASMJIT_INLINE intptr_t getLabelOffset(const Label& label) const {
    return getLabelOffset(label.getId());
  }
  //! \overload
  ASMJIT_INLINE intptr_t getLabelOffset(uint32_t id) const {
    ASMJIT_ASSERT(isLabelValid(id));
    return _labelList[id]->offset;
  }

  //! Get `LabelData` by `label`.
  ASMJIT_INLINE LabelData* getLabelData(const Label& label) const {
    return getLabelData(label.getId());
  }
  //! \overload
  ASMJIT_INLINE LabelData* getLabelData(uint32_t id) const {
    ASMJIT_ASSERT(isLabelValid(id));
    return const_cast<LabelData*>(_labelList[id]);
  }

  //! \internal
  //!
  //! Create a new label and return its ID.
  ASMJIT_API uint32_t _newLabelId();

  //! \internal
  //!
  //! New LabelLink instance.
  ASMJIT_API LabelLink* _newLabelLink();

  //! Create and return a new `Label`.
  ASMJIT_INLINE Label newLabel() { return Label(_newLabelId()); }

  //! Bind the `label` to the current offset.
  //!
  //! \note Label can be bound only once!
  ASMJIT_API virtual Error bind(const Label& label);

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
  ASMJIT_API size_t relocCode(void* dst, Ptr baseAddress = kNoBaseAddress) const;

  //! \internal
  //!
  //! Reloc code.
  virtual size_t _relocCode(void* dst, Ptr baseAddress) const = 0;

  // --------------------------------------------------------------------------
  // [Make]
  // --------------------------------------------------------------------------

  ASMJIT_API virtual void* make();

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

#if !defined(ASMJIT_DISABLE_LOGGER)
  //! Associated logger.
  Logger* _logger;
#else
  //! Makes libraries built with or without logging support binary compatible.
  void* _logger;
#endif // ASMJIT_DISABLE_LOGGER
  //! Associated error handler, triggered by \ref setLastError().
  ErrorHandler* _errorHandler;

  //! Target architecture ID.
  uint8_t _arch;
  //! Target architecture GP register size in bytes (4 or 8).
  uint8_t _regSize;
  //! \internal
  uint16_t _reserved;

  //! Assembler features, used by \ref hasFeature() and \ref setFeature().
  uint32_t _features;
  //! Options affecting the next instruction.
  uint32_t _instOptions;
  //! Last error code.
  uint32_t _lastError;

  //! CodeGen ID generator.
  uint64_t _hlIdGenerator;
  //! Count of high-level code generators attached.
  size_t _hlAttachedCount;

  //! General purpose zone allocator.
  Zone _zoneAllocator;

  //! Start of the code-buffer.
  uint8_t* _buffer;
  //! End of the code-buffer (points to the first invalid byte).
  uint8_t* _end;
  //! The current position in code `_buffer`.
  uint8_t* _cursor;

  //! Size of all possible trampolines.
  uint32_t _trampolinesSize;

  //! Inline comment that will be logged by the next instruction and set to NULL.
  const char* _comment;
  //! Unused `LabelLink` structures pool.
  LabelLink* _unusedLinks;

  //! LabelData list.
  PodVector<LabelData*> _labelList;
  //! RelocData list.
  PodVector<RelocData> _relocList;
};

//! \}

// ============================================================================
// [Defined-Later]
// ============================================================================

ASMJIT_INLINE Label::Label(Assembler& a) : Operand(NoInit) {
  reset();
  _label.id = a._newLabelId();
}

} // asmjit namespace

// [Api-End]
#include "../apiend.h"

// [Guard]
#endif // _ASMJIT_BASE_ASSEMBLER_H
