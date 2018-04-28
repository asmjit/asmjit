// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// ZLIB - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_CORE_EMITTER_H
#define _ASMJIT_CORE_EMITTER_H

// [Dependencies]
#include "../core/arch.h"
#include "../core/inst.h"
#include "../core/operand.h"
#include "../core/codeholder.h"

ASMJIT_BEGIN_NAMESPACE

//! \addtogroup asmjit_core_api
//! \{

// ============================================================================
// [Forward Declarations]
// ============================================================================

class ConstPool;
class FuncFrame;
class FuncArgsAssignment;

// ============================================================================
// [asmjit::BaseEmitter]
// ============================================================================

//! Provides a base foundation to emit code - specialized by `Assembler` and
//! `BaseBuilder`.
class ASMJIT_VIRTAPI BaseEmitter {
public:
  //! Emitter type.
  enum EmitterType : uint32_t {
    kTypeNone      = 0,
    kTypeAssembler = 1,
    kTypeBuilder   = 2,
    kTypeCompiler  = 3,
    kTypeCount     = 4
  };

  //! Emitter flags.
  enum Flags : uint32_t {
    kFlagFinalized = 0x4000u,            //!< Code emitter is finalized.
    kFlagDestroyed = 0x8000u             //!< Code emitter was destroyed.
  };

  //! Emitter options.
  enum Options : uint32_t {
    //! Logging is enabled, `BaseEmitter::logger()` must return a valid logger.
    kOptionLoggingEnabled   = 0x00000001u,

    //! Stricly validate each instruction before it's emitted.
    kOptionStrictValidation = 0x00000002u,

    //! Emit optimized code-alignment sequences.
    //!
    //! Default `false`.
    //!
    //! X86 Specific
    //! ------------
    //!
    //! Default align sequence used by X86 architecture is one-byte (0x90)
    //! opcode that is often shown by disassemblers as NOP. However there are
    //! more optimized align sequences for 2-11 bytes that may execute faster
    //! on certain CPUs. If this feature is enabled AsmJit will generate
    //! specialized sequences for alignment between 2 to 11 bytes.
    kOptionOptimizedAlign = 0x00000004u,

    //! Emit jump-prediction hints.
    //!
    //! Default `false`.
    //!
    //! X86 Specific
    //! ------------
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
    //! implement heuristics for branch prediction and ignore static hints. This
    //! means that this feature can be used for annotation purposes.
    kOptionPredictedJumps = 0x00000008u
  };

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  ASMJIT_API explicit BaseEmitter(uint32_t type) noexcept;
  ASMJIT_API virtual ~BaseEmitter() noexcept;

  // --------------------------------------------------------------------------
  // [Cast]
  // --------------------------------------------------------------------------

  template<typename T>
  inline T* as() noexcept { return reinterpret_cast<T*>(this); }

  template<typename T>
  inline const T* as() const noexcept { return reinterpret_cast<const T*>(this); }

  // --------------------------------------------------------------------------
  // [Emitter Type & Flags]
  // --------------------------------------------------------------------------

  //! Get the type of this BaseEmitter, see `EmitterType`.
  inline uint32_t emitterType() const noexcept { return _type; }
  inline uint32_t emitterFlags() const noexcept { return _flags; }

  inline bool isAssembler() const noexcept { return _type == kTypeAssembler; }
  inline bool isBuilder() const noexcept { return _type >= kTypeBuilder; }
  inline bool isCompiler() const noexcept { return _type == kTypeCompiler; }

  inline bool hasFlag(uint32_t flag) const noexcept { return (_flags & flag) != 0; }
  inline bool isFinalized() const noexcept { return hasFlag(kFlagFinalized); }
  inline bool isDestroyed() const noexcept { return hasFlag(kFlagDestroyed); }

  inline void _addFlags(uint32_t flags) noexcept { _flags = uint16_t(_flags | flags); }
  inline void _clearFlags(uint32_t flags) noexcept { _flags = uint16_t(_flags & ~flags); }

  // --------------------------------------------------------------------------
  // [Target Information]
  // --------------------------------------------------------------------------

  //! Get `CodeHolder` this emitter is attached to.
  inline CodeHolder* code() const noexcept { return _code; }
  //! Get information about the code, see `CodeInfo`.
  inline const CodeInfo& codeInfo() const noexcept { return _codeInfo; }
  //! Get information about the architecture, see `ArchInfo`.
  inline const ArchInfo& archInfo() const noexcept { return _codeInfo.archInfo(); }

  //! Get whether the target architecture is 32-bit.
  inline bool is32Bit() const noexcept { return archInfo().is32Bit(); }
  //! Get whether the target architecture is 64-bit.
  inline bool is64Bit() const noexcept { return archInfo().is64Bit(); }

  //! Get the target architecture type.
  inline uint32_t archId() const noexcept { return archInfo().archId(); }
  //! Get the target architecture sub-type.
  inline uint32_t archSubId() const noexcept { return archInfo().archSubId(); }
  //! Get the target architecture's GP register size (4 or 8 bytes).
  inline uint32_t gpSize() const noexcept { return archInfo().gpSize(); }
  //! Get the number of target GP registers.
  inline uint32_t gpCount() const noexcept { return archInfo().gpCount(); }

  // --------------------------------------------------------------------------
  // [Initialization / Finalization]
  // --------------------------------------------------------------------------

  //! Get whether the BaseEmitter is initialized (i.e. attached to the `CodeHolder`).
  inline bool isInitialized() const noexcept { return _code != nullptr; }

  ASMJIT_API virtual Error finalize();

  // --------------------------------------------------------------------------
  // [Emitter Options]
  // --------------------------------------------------------------------------

  //! Get whether the `option` is present in emitter options.
  inline bool hasEmitterOption(uint32_t option) const noexcept { return (_emitterOptions & option) != 0; }
  //! Get emitter options.
  inline uint32_t emitterOptions() const noexcept { return _emitterOptions; }

  inline void addEmitterOptions(uint32_t options) noexcept {
    _emitterOptions |= options;
    onUpdateGlobalInstOptions();
  }

  inline void clearEmitterOptions(uint32_t options) noexcept {
    _emitterOptions &= ~options;
    onUpdateGlobalInstOptions();
  }

  //! Get global instruction options.
  //!
  //! Default instruction options are merged with instruction options before the
  //! instruction is encoded. These options have some bits reserved that are used
  //! for error handling, logging, and strict validation. Other options are globals that
  //! affect each instruction, for example if VEX3 is set globally, it will all
  //! instructions, even those that don't have such option set.
  inline uint32_t globalInstOptions() const noexcept { return _globalInstOptions; }

  // --------------------------------------------------------------------------
  // [Error Handling]
  // --------------------------------------------------------------------------

  //! Get whether the local error handler is attached.
  inline bool hasErrorHandler() const noexcept { return _errorHandler != nullptr; }
  //! Get the local error handler.
  inline ErrorHandler* errorHandler() const noexcept { return _errorHandler; }
  //! Set the local error handler.
  inline void setErrorHandler(ErrorHandler* handler) noexcept { _errorHandler = handler; }
  //! Reset the local error handler (does nothing if not attached).
  inline void resetErrorHandler() noexcept { setErrorHandler(nullptr); }

  //! Handle the given error in the following way:
  //!   1. Get either BaseEmitter's (preferred) or CodeHolder's ErrorHandler.
  //!   2. If exists, call `ErrorHandler::handleError(error, message, this)`.
  //!   3. Return the given `err` if ErrorHandler haven't thrown.
  ASMJIT_API Error reportError(Error err, const char* message = nullptr);

  // --------------------------------------------------------------------------
  // [Instruction Properties - Affect the Next Instruction to be Emitted]
  // --------------------------------------------------------------------------

  //! Get options of the next instruction.
  inline uint32_t instOptions() const noexcept { return _instOptions; }
  //! Set options of the next instruction.
  inline void setInstOptions(uint32_t options) noexcept { _instOptions = options; }
  //! Add options of the next instruction.
  inline void addInstOptions(uint32_t options) noexcept { _instOptions |= options; }
  //! Reset options of the next instruction.
  inline void resetInstOptions() noexcept { _instOptions = 0; }

  //! Get whether the extra register operand is valid.
  inline bool hasExtraReg() const noexcept { return _extraReg.isReg(); }
  //! Get an extra operand that will be used by the next instruction (architecture specific).
  inline const RegOnly& extraReg() const noexcept { return _extraReg; }
  //! Set an extra operand that will be used by the next instruction (architecture specific).
  inline void setExtraReg(const BaseReg& reg) noexcept { _extraReg.init(reg); }
  //! Set an extra operand that will be used by the next instruction (architecture specific).
  inline void setExtraReg(const RegOnly& reg) noexcept { _extraReg.init(reg); }
  //! Reset an extra operand that will be used by the next instruction (architecture specific).
  inline void resetExtraReg() noexcept { _extraReg.reset(); }

  //! Get annotation of the next instruction.
  inline const char* inlineComment() const noexcept { return _inlineComment; }
  //! Set annotation of the next instruction.
  //!
  //! NOTE: This string is set back to null by `_emit()`, but until that it has
  //! to remain valid as `BaseEmitter` is not required to make a copy of it (and
  //! it would be slow to do that for each instruction).
  inline void setInlineComment(const char* s) noexcept { _inlineComment = s; }
  //! Reset annotation of the next instruction to null.
  inline void resetInlineComment() noexcept { _inlineComment = nullptr; }

  // --------------------------------------------------------------------------
  // [Label Management]
  // --------------------------------------------------------------------------

  //! Create a new label.
  virtual Label newLabel() = 0;
  //! Create a new named label.
  virtual Label newNamedLabel(
    const char* name,
    size_t nameSize = Globals::kNullTerminated,
    uint32_t type = Label::kTypeGlobal,
    uint32_t parentId = 0) = 0;

  //! Get a label by name.
  //!
  //! Returns invalid Label in case that the name is invalid or label was not found.
  //!
  //! NOTE: This function doesn't trigger ErrorHandler in case the name is invalid
  //! or no such label exist. You must always check the validity of the `Label` returned.
  ASMJIT_API Label labelByName(
    const char* name,
    size_t nameSize = Globals::kNullTerminated,
    uint32_t parentId = 0) noexcept;

  //! Bind the `label` to the current position of the current section.
  //!
  //! NOTE: Attempt to bind the same label multiple times will return an error.
  virtual Error bind(const Label& label) = 0;

  //! Get whether the label `id` is valid (i.e. registered).
  ASMJIT_API bool isLabelValid(uint32_t id) const noexcept;
  //! Get whether the `label` is valid (i.e. registered).
  inline bool isLabelValid(const Label& label) const noexcept { return isLabelValid(label.id()); }

  // --------------------------------------------------------------------------
  // [Emit - Low-Level]
  // --------------------------------------------------------------------------

  // NOTE: These `emit()` helpers are designed to address a code-bloat generated
  // by C++ compilers to call a function having many arguments. Each parameter to
  // `_emit()` requires some code to pass it, which means that if we default to 4
  // operand parameters in `_emit()` and instId the C++ compiler would have to
  // generate a virtual function call having 5 parameters, which is quite a lot.
  // Since by default asm instructions have 2 to 3 operands it's better to
  // introduce helpers that pass those and fill out the remaining operands.

  #define OP const Operand_&
  #define NONE Globals::none

  //! Emit an instruction.
  ASMJIT_NOINLINE Error emit(uint32_t instId) { return _emit(instId, NONE, NONE, NONE, NONE); }
  //! \overload
  ASMJIT_NOINLINE Error emit(uint32_t instId, OP o0) { return _emit(instId, o0, NONE, NONE, NONE); }
  //! \overload
  ASMJIT_NOINLINE Error emit(uint32_t instId, OP o0, OP o1) { return _emit(instId, o0, o1, NONE, NONE); }
  //! \overload
  ASMJIT_NOINLINE Error emit(uint32_t instId, OP o0, OP o1, OP o2) { return _emit(instId, o0, o1, o2, NONE); }
  //! \overload
  inline Error emit(uint32_t instId, OP o0, OP o1, OP o2, OP o3) { return _emit(instId, o0, o1, o2, o3); }
  //! \overload
  inline Error emit(uint32_t instId, OP o0, OP o1, OP o2, OP o3, OP o4) { return _emit(instId, o0, o1, o2, o3, o4, NONE); }
  //! \overload
  inline Error emit(uint32_t instId, OP o0, OP o1, OP o2, OP o3, OP o4, OP o5) { return _emit(instId, o0, o1, o2, o3, o4, o5); }

  //! \overload
  ASMJIT_NOINLINE Error emit(uint32_t instId, int o0) { return _emit(instId, Imm(o0), NONE, NONE, NONE); }
  //! \overload
  ASMJIT_NOINLINE Error emit(uint32_t instId, OP o0, int o1) { return _emit(instId, o0, Imm(o1), NONE, NONE); }
  //! \overload
  ASMJIT_NOINLINE Error emit(uint32_t instId, OP o0, OP o1, int o2) { return _emit(instId, o0, o1, Imm(o2), NONE); }
  //! \overload
  ASMJIT_NOINLINE Error emit(uint32_t instId, OP o0, OP o1, OP o2, int o3) { return _emit(instId, o0, o1, o2, Imm(o3)); }
  //! \overload
  ASMJIT_NOINLINE Error emit(uint32_t instId, OP o0, OP o1, OP o2, OP o3, int o4) { return _emit(instId, o0, o1, o2, o3, Imm(o4), NONE); }
  //! \overload
  ASMJIT_NOINLINE Error emit(uint32_t instId, OP o0, OP o1, OP o2, OP o3, OP o4, int o5) { return _emit(instId, o0, o1, o2, o3, o4, Imm(o5)); }

  //! \overload
  ASMJIT_NOINLINE Error emit(uint32_t instId, int64_t o0) { return _emit(instId, Imm(o0), NONE, NONE, NONE); }
  //! \overload
  ASMJIT_NOINLINE Error emit(uint32_t instId, OP o0, int64_t o1) { return _emit(instId, o0, Imm(o1), NONE, NONE); }
  //! \overload
  ASMJIT_NOINLINE Error emit(uint32_t instId, OP o0, OP o1, int64_t o2) { return _emit(instId, o0, o1, Imm(o2), NONE); }
  //! \overload
  ASMJIT_NOINLINE Error emit(uint32_t instId, OP o0, OP o1, OP o2, int64_t o3) { return _emit(instId, o0, o1, o2, Imm(o3)); }
  //! \overload
  ASMJIT_NOINLINE Error emit(uint32_t instId, OP o0, OP o1, OP o2, OP o3, int64_t o4) { return _emit(instId, o0, o1, o2, o3, Imm(o4), NONE); }
  //! \overload
  ASMJIT_NOINLINE Error emit(uint32_t instId, OP o0, OP o1, OP o2, OP o3, OP o4, int64_t o5) { return _emit(instId, o0, o1, o2, o3, o4, Imm(o5)); }

  //! \overload
  inline Error emit(uint32_t instId, unsigned int o0) { return emit(instId, int64_t(o0)); }
  //! \overload
  inline Error emit(uint32_t instId, OP o0, unsigned int o1) { return emit(instId, o0, int64_t(o1)); }
  //! \overload
  inline Error emit(uint32_t instId, OP o0, OP o1, unsigned int o2) { return emit(instId, o0, o1, int64_t(o2)); }
  //! \overload
  inline Error emit(uint32_t instId, OP o0, OP o1, OP o2, unsigned int o3) { return emit(instId, o0, o1, o2, int64_t(o3)); }
  //! \overload
  inline Error emit(uint32_t instId, OP o0, OP o1, OP o2, OP o3, unsigned int o4) { return emit(instId, o0, o1, o2, o3, int64_t(o4)); }
  //! \overload
  inline Error emit(uint32_t instId, OP o0, OP o1, OP o2, OP o3, OP o4, unsigned int o5) { return emit(instId, o0, o1, o2, o3, o4, int64_t(o5)); }

  //! \overload
  inline Error emit(uint32_t instId, uint64_t o0) { return emit(instId, int64_t(o0)); }
  //! \overload
  inline Error emit(uint32_t instId, OP o0, uint64_t o1) { return emit(instId, o0, int64_t(o1)); }
  //! \overload
  inline Error emit(uint32_t instId, OP o0, OP o1, uint64_t o2) { return emit(instId, o0, o1, int64_t(o2)); }
  //! \overload
  inline Error emit(uint32_t instId, OP o0, OP o1, OP o2, uint64_t o3) { return emit(instId, o0, o1, o2, int64_t(o3)); }
  //! \overload
  inline Error emit(uint32_t instId, OP o0, OP o1, OP o2, OP o3, uint64_t o4) { return emit(instId, o0, o1, o2, o3, int64_t(o4)); }
  //! \overload
  inline Error emit(uint32_t instId, OP o0, OP o1, OP o2, OP o3, OP o4, uint64_t o5) { return emit(instId, o0, o1, o2, o3, o4, int64_t(o5)); }

  #undef NONE
  #undef OP

  inline Error emitOpArray(uint32_t instId, const Operand_* operands, size_t count) { return _emitOpArray(instId, operands, count); }

  inline Error emitInst(const BaseInst& inst, const Operand_* operands, size_t count) {
    setInstOptions(inst.options());
    setExtraReg(inst.extraReg());
    return _emitOpArray(inst.id(), operands, count);
  }

  // --------------------------------------------------------------------------
  // [Emit - Virtual]
  // --------------------------------------------------------------------------

  //! Emit instruction having max 4 operands.
  virtual Error _emit(uint32_t instId, const Operand_& o0, const Operand_& o1, const Operand_& o2, const Operand_& o3) = 0;
  //! Emit instruction having max 6 operands.
  virtual Error _emit(uint32_t instId, const Operand_& o0, const Operand_& o1, const Operand_& o2, const Operand_& o3, const Operand_& o4, const Operand_& o5) = 0;
  //! Emit instruction having operands stored in array.
  virtual Error _emitOpArray(uint32_t instId, const Operand_* operands, size_t count);

  // --------------------------------------------------------------------------
  // [Emit - High-Level]
  // --------------------------------------------------------------------------

  ASMJIT_API Error emitProlog(const FuncFrame& layout);
  ASMJIT_API Error emitEpilog(const FuncFrame& layout);
  ASMJIT_API Error emitArgsAssignment(const FuncFrame& layout, const FuncArgsAssignment& args);

  // --------------------------------------------------------------------------
  // [Align]
  // --------------------------------------------------------------------------

  //! Align to the `alignment` specified.
  //!
  //! The sequence that is used to fill the gap between the aligned location
  //! and the current location depends on the align `mode`, see `AlignMode`.
  virtual Error align(uint32_t alignMode, uint32_t alignment) = 0;

  // --------------------------------------------------------------------------
  // [Embed]
  // --------------------------------------------------------------------------

  //! Embed raw data into the code-buffer.
  virtual Error embed(const void* data, uint32_t size) = 0;

  //! Embed absolute label address as data (4 or 8 bytes).
  virtual Error embedLabel(const Label& label) = 0;

  //! Embed a constant pool into the code-buffer in the following steps:
  //!   1. Align by using kAlignData to the minimum `pool` alignment.
  //!   2. Bind `label` so it's bound to an aligned location.
  //!   3. Emit constant pool data.
  virtual Error embedConstPool(const Label& label, const ConstPool& pool) = 0;

  // --------------------------------------------------------------------------
  // [Comment]
  // --------------------------------------------------------------------------

  //! Emit comment from string `data` with an optional comment `size` parameter.
  virtual Error comment(const char* data, size_t size = Globals::kNullTerminated) = 0;

  //! Emit comment from a formatted string `fmt`.
  ASMJIT_API Error commentf(const char* fmt, ...);
  //! Emit comment from a formatted string `fmt` (va_list version).
  ASMJIT_API Error commentv(const char* fmt, std::va_list ap);

  // --------------------------------------------------------------------------
  // [Events]
  // --------------------------------------------------------------------------

  //! Called after the emitter was attached to `CodeHolder`.
  virtual Error onAttach(CodeHolder* code) noexcept = 0;
  //! Called after the emitter was detached from `CodeHolder`.
  virtual Error onDetach(CodeHolder* code) noexcept = 0;

  //! Called to update `_globalInstOptions` based on `_emitterOptions`.
  //!
  //! This function should only touch one bit `BaseInst::kOptionReserved`, which
  //! is used to handle errors and special-cases in a way that minimizes branching.
  ASMJIT_API void onUpdateGlobalInstOptions() noexcept;

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  uint8_t _type;                         //!< See `EmitterType`.
  uint8_t _reserved;                     //!< Reserved
  uint16_t _flags;                       //!< See `Flags`.

  CodeHolder* _code;                     //!< CodeHolder the BaseEmitter is attached to.
  ErrorHandler* _errorHandler;           //!< Attached `ErrorHandler`.

  CodeInfo _codeInfo;                    //!< Basic information about the code (matches CodeHolder::_codeInfo).
  RegInfo _gpRegInfo;                    //!< Native GP register signature and signature related information.

  uint32_t _emitterOptions;              //!< Emitter options, always in sync with CodeHolder.
  uint32_t _privateData;                 //!< Internal private data used freely by any BaseEmitter.

  uint32_t _instOptions;                 //!< Next instruction options                (affects the next instruction).
  uint32_t _globalInstOptions;           //!< Global Instruction options              (combined with `_instOptions` by `emit...()`).
  RegOnly _extraReg;                     //!< Extra register (op-mask {k} on AVX-512) (affects the next instruction).
  const char* _inlineComment;            //!< Inline comment of the next instruction  (affects the next instruction).
};

//! \}

ASMJIT_END_NAMESPACE

// [Guard]
#endif // _ASMJIT_CORE_EMITTER_H
