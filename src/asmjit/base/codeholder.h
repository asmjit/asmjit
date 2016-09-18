// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_BASE_CODEHOLDER_H
#define _ASMJIT_BASE_CODEHOLDER_H

// [Dependencies]
#include "../base/arch.h"
#include "../base/func.h"
#include "../base/logging.h"
#include "../base/operand.h"
#include "../base/simdtypes.h"
#include "../base/utils.h"
#include "../base/zone.h"
#include "../base/zonecontainers.h"
#include "../base/zoneheap.h"

// [Api-Begin]
#include "../asmjit_apibegin.h"

namespace asmjit {

//! \addtogroup asmjit_base
//! \{

// ============================================================================
// [Forward Declarations]
// ============================================================================

class Assembler;
class CodeEmitter;
class CodeHolder;

// ============================================================================
// [asmjit::AlignMode]
// ============================================================================

//! Code/Data align-mode.
ASMJIT_ENUM(AlignMode) {
  kAlignCode = 0,                        //!< Align executable code.
  kAlignData = 1,                        //!< Align non-executable code.
  kAlignZero = 2                         //!< Align by a sequence of zeros.
};

// ============================================================================
// [asmjit::RelocMode]
// ============================================================================

//! Relocation mode.
ASMJIT_ENUM(RelocMode) {
  kRelocAbsToAbs = 0,                    //!< Relocate absolute to absolute.
  kRelocRelToAbs = 1,                    //!< Relocate relative to absolute.
  kRelocAbsToRel = 2,                    //!< Relocate absolute to relative.
  kRelocTrampoline = 3                   //!< Relocate absolute to relative or use trampoline.
};

// ============================================================================
// [asmjit::ErrorHandler]
// ============================================================================

//! Error handler can be used to override the default behavior of error handling
//! available to all classes that inherit \ref CodeEmitter. See \ref handleError().
class ASMJIT_VIRTAPI ErrorHandler {
public:
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Create a new `ErrorHandler` instance.
  ASMJIT_API ErrorHandler() noexcept;
  //! Destroy the `ErrorHandler` instance.
  ASMJIT_API virtual ~ErrorHandler() noexcept;

  // --------------------------------------------------------------------------
  // [Handle Error]
  // --------------------------------------------------------------------------

  //! Error handler (abstract).
  //!
  //! Error handler is called after an error happened and before it's propagated
  //! to the caller. There are multiple ways how the error handler can be used:
  //!
  //! 1. Returning `true` or `false` from `handleError()`. If `true` is returned
  //!    it means that the error was reported and AsmJit can continue execution.
  //!    The reported error still be propagated to the caller, but won't put the
  //!    CodeEmitter into an error state (it won't set last-error). However,
  //!    returning `false` means that the error cannot be handled - in such case
  //!    it stores the error, which can be then retrieved by using `getLastError()`.
  //!    Returning `false` is the default behavior when no error handler is present.
  //!    To put the assembler into a non-error state again a `resetLastError()` must
  //!    be called.
  //!
  //! 2. Throwing an exception. AsmJit doesn't use exceptions and is completely
  //!    exception-safe, but you can throw exception from your error handler if
  //!    this way is the preferred way of handling errors in your project. Throwing
  //!    an exception acts virtually as returning `true` as AsmJit won't be able
  //!    to store the error because the exception changes execution path.
  //!
  //! 3. Using plain old C's `setjmp()` and `longjmp()`. Asmjit always puts
  //!    `CodeEmitter` to a consistent state before calling the `handleError()`
  //!    so `longjmp()` can be used without any issues to cancel the code
  //!    generation if an error occurred. There is no difference between
  //!    exceptions and longjmp() from AsmJit's perspective.
  virtual bool handleError(Error err, const char* message, CodeEmitter* origin) = 0;
};

// ============================================================================
// [asmjit::CodeInfo]
// ============================================================================

//! Basic information about a code (or target). It describes its architecture,
//! code generation mode (or optimization level), and base address.
class CodeInfo {
public:
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE CodeInfo() noexcept
    : _archInfo(),
      _stackAlignment(0),
      _cdeclCallConv(CallConv::kIdNone),
      _stdCallConv(CallConv::kIdNone),
      _fastCallConv(CallConv::kIdNone),
      _baseAddress(kNoBaseAddress) {}
  ASMJIT_INLINE CodeInfo(const CodeInfo& other) noexcept { init(other); }

  explicit ASMJIT_INLINE CodeInfo(uint32_t archType, uint32_t archMode = 0, uint64_t baseAddress = kNoBaseAddress) noexcept
    : _archInfo(archType, archMode),
      _packedMiscInfo(0),
      _baseAddress(baseAddress) {}

  // --------------------------------------------------------------------------
  // [Init / Reset]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE bool isInitialized() const noexcept {
    return _archInfo._type != ArchInfo::kTypeNone;
  }

  ASMJIT_INLINE void init(const CodeInfo& other) noexcept {
    _archInfo = other._archInfo;
    _packedMiscInfo = other._packedMiscInfo;
    _baseAddress = other._baseAddress;
  }

  ASMJIT_INLINE void init(uint32_t archType, uint32_t archMode = 0, uint64_t baseAddress = kNoBaseAddress) noexcept {
    _archInfo.init(archType, archMode);
    _packedMiscInfo = 0;
    _baseAddress = baseAddress;
  }

  ASMJIT_INLINE void reset() noexcept {
    _archInfo.reset();
    _stackAlignment = 0;
    _cdeclCallConv = CallConv::kIdNone;
    _stdCallConv = CallConv::kIdNone;
    _fastCallConv = CallConv::kIdNone;
    _baseAddress = kNoBaseAddress;
  }

  // --------------------------------------------------------------------------
  // [Architecture Information]
  // --------------------------------------------------------------------------

  //! Get architecture information, see \ref ArchInfo.
  ASMJIT_INLINE const ArchInfo& getArchInfo() const noexcept { return _archInfo; }

  //! Get architecture type, see \ref ArchInfo::Type.
  ASMJIT_INLINE uint32_t getArchType() const noexcept { return _archInfo.getType(); }
  //! Get architecture sub-type, see \ref ArchInfo::SubType.
  ASMJIT_INLINE uint32_t getArchSubType() const noexcept { return _archInfo.getSubType(); }
  //! Get a size of a GP register of the architecture the code is using.
  ASMJIT_INLINE uint32_t getGpSize() const noexcept { return _archInfo.getGpSize(); }
  //! Get number of GP registers available of the architecture the code is using.
  ASMJIT_INLINE uint32_t getGpCount() const noexcept { return _archInfo.getGpCount(); }

  // --------------------------------------------------------------------------
  // [High-Level Information]
  // --------------------------------------------------------------------------

  //! Get a natural stack alignment that must be honored (or 0 if not known).
  ASMJIT_INLINE uint32_t getStackAlignment() const noexcept { return _stackAlignment; }
  //! Set a natural stack alignment that must be honored.
  ASMJIT_INLINE void setStackAlignment(uint8_t sa) noexcept { _stackAlignment = static_cast<uint8_t>(sa); }

  ASMJIT_INLINE uint32_t getCdeclCallConv() const noexcept { return _cdeclCallConv; }
  ASMJIT_INLINE void setCdeclCallConv(uint32_t cc) noexcept { _cdeclCallConv = static_cast<uint8_t>(cc); }

  ASMJIT_INLINE uint32_t getStdCallConv() const noexcept { return _stdCallConv; }
  ASMJIT_INLINE void setStdCallConv(uint32_t cc) noexcept { _stdCallConv = static_cast<uint8_t>(cc); }

  ASMJIT_INLINE uint32_t getFastCallConv() const noexcept { return _fastCallConv; }
  ASMJIT_INLINE void setFastCallConv(uint32_t cc) noexcept { _fastCallConv = static_cast<uint8_t>(cc); }

  // --------------------------------------------------------------------------
  // [Addressing Information]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE bool hasBaseAddress() const noexcept { return _baseAddress != kNoBaseAddress; }
  ASMJIT_INLINE uint64_t getBaseAddress() const noexcept { return _baseAddress; }
  ASMJIT_INLINE void setBaseAddress(uint64_t p) noexcept { _baseAddress = p; }
  ASMJIT_INLINE void resetBaseAddress() noexcept { _baseAddress = kNoBaseAddress; }

  // --------------------------------------------------------------------------
  // [Operator Overload]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE CodeInfo& operator=(const CodeInfo& other) noexcept { init(other); return *this; }
  ASMJIT_INLINE bool operator==(const CodeInfo& other) const noexcept { return ::memcmp(this, &other, sizeof(*this)) == 0; }
  ASMJIT_INLINE bool operator!=(const CodeInfo& other) const noexcept { return ::memcmp(this, &other, sizeof(*this)) != 0; }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  ArchInfo _archInfo;                    //!< Architecture information.

  union {
    struct {
      uint8_t _stackAlignment;           //!< Natural stack alignment (ARCH+OS).
      uint8_t _cdeclCallConv;            //!< Default CDECL calling convention.
      uint8_t _stdCallConv;              //!< Default STDCALL calling convention.
      uint8_t _fastCallConv;             //!< Default FASTCALL calling convention.
    };
    uint32_t _packedMiscInfo;            //!< \internal
  };

  uint64_t _baseAddress;                 //!< Base address.
};

// ============================================================================
// [asmjit::CodeSection]
// ============================================================================

//! Code or data section.
struct CodeSection {
  //! Section flags.
  ASMJIT_ENUM(Flags) {
    kFlagExec        = 0x00000001U,      //!< Executable (.text sections).
    kFlagConst       = 0x00000002U,      //!< Read-only (.text and .data sections).
    kFlagZero        = 0x00000004U,      //!< Zero initialized by the loader (BSS).
    kFlagInfo        = 0x00000008U       //!< Info / comment flag.
  };

  uint32_t id;                           //!< Section id.
  uint32_t flags;                        //!< Section flags.
  uint32_t alignment;                    //!< Section alignment requirements (0 if no requirements).
  char name[36];                         //!< Section name (max 35 characters, PE allows max 8).
};

// ============================================================================
// [asmjit::CodeBuffer]
// ============================================================================

//! Code or data buffer.
struct CodeBuffer {
  uint8_t* data;                         //!< The content of the buffer (data).
  size_t length;                         //!< Number of bytes of `data` used.
  size_t capacity;                       //!< Buffer capacity (in bytes).
  bool isExternal;                       //!< True if this is external buffer.
  bool isFixedSize;                      //!< True if this buffer cannot grow.
};

// ============================================================================
// [asmjit::CodeHolder]
// ============================================================================

//! Contains basic information about the target architecture plus its settings,
//! and holds code & data (including sections, labels, and relocation information).
//! CodeHolder can store both binary and intermediate representation of assembly,
//! which can be generated by \ref Assembler and/or \ref CodeBuilder.
//!
//! NOTE: CodeHolder has ability to attach an \ref ErrorHandler, however, this
//! error handler is not triggered by CodeHolder itself, it's only used by the
//! attached code generators.
class ASMJIT_VIRTAPI CodeHolder {
public:
  ASMJIT_NONCOPYABLE(CodeHolder)

  // --------------------------------------------------------------------------
  // [SectionEntry]
  // --------------------------------------------------------------------------

  //! Code or data section entry.
  struct SectionEntry {
    CodeSection info;                    //!< Section information (name, flags, alignment).
    CodeBuffer buffer;                   //!< Machine code & data of this section.
  };

  // --------------------------------------------------------------------------
  // [LabelLink]
  // --------------------------------------------------------------------------

  //! Data structure used to link labels.
  struct LabelLink {
    LabelLink* prev;                     //!< Previous link (single-linked list).
    intptr_t offset;                     //!< Label offset relative to the start of the section.
    intptr_t displacement;               //!< Inlined displacement.
    intptr_t relocId;                    //!< Relocation id (in case it's needed).
  };

  // --------------------------------------------------------------------------
  // [LabelEntry]
  // --------------------------------------------------------------------------

  //! Label entry.
  //!
  //! Contains the following properties:
  //!   * Label id - This is the only thing that is set to the `Label` operand.
  //!   * Label name - Optional, used mostly to create executables and libraries.
  //!   * Label type - Type of the label, default `Label::kTypeAnonymous`.
  //!   * Label parent id - Derived from many assemblers that allow to define a
  //!       local label that falls under a global label. This allows to define
  //!       many labels of the same name that have different parent (global) label.
  //!   * Offset - offset of the label bound by `Assembler`.
  //!   * Links - single-linked list that contains locations of code that has
  //!       to be patched when the label gets bound. Every use of unbound label
  //!       adds one link to `_links` list.
  //!   * HVal - Hash value of label's name and optionally parentId.
  //!   * HashNext - Hash-table implementation detail.
  class LabelEntry : public ZoneHashNode {
  public:
    // NOTE: Label id is stored in `_customData`, which is provided by ZoneHashNode
    // to fill a padding that a C++ compiler targeting 64-bit CPU will add to align
    // the structure to 64-bits.

    //! Get label id.
    ASMJIT_INLINE uint32_t getId() const noexcept { return _customData; }
    //! Set label id (internal, used only by \ref CodeHolder).
    ASMJIT_INLINE void _setId(uint32_t id) noexcept { _customData = id; }

    //! Get label type, see \ref Label::Type.
    ASMJIT_INLINE uint32_t getType() const noexcept { return _type; }
    //! Get label flags, returns 0 at the moment.
    ASMJIT_INLINE uint32_t getFlags() const noexcept { return _flags; }

    //! Get label's parent id.
    ASMJIT_INLINE uint32_t getParentId() const noexcept { return _parentId; }

    //! Get if the label has name.
    ASMJIT_INLINE bool hasName() const noexcept { return !_name.isEmpty(); }

    //! Get the label's name.
    //!
    //! NOTE: Local labels will return their local name without their parent
    //! part, for example ".L1".
    ASMJIT_INLINE const char* getName() const noexcept { return _name.getData(); }

    //! Get length of label's name.
    //!
    //! NOTE: Label name is always null terminated, so you can use `strlen()` to
    //! get it, however, it's also cached in `LabelEntry`, so if you want to know
    //! the length the easiest way is to use `LabelEntry::getNameLength()`.
    ASMJIT_INLINE size_t getNameLength() const noexcept { return _name.getLength(); }

    //! Get if the label is bound.
    ASMJIT_INLINE bool isBound() const noexcept { return _offset != -1; }
    //! Get the label offset (only useful if the label is bound).
    ASMJIT_INLINE intptr_t getOffset() const noexcept { return _offset; }

    //! Get the hash-value of label's name and its parent label (if any).
    //!
    //! Label hash is calculated as `HASH(Name) ^ ParentId`. The hash function
    //! is implemented in `Utils::hashString()` and `Utils::hashRound()`.
    ASMJIT_INLINE uint32_t getHVal() const noexcept { return _hVal; }

    // ------------------------------------------------------------------------
    // [Members]
    // ------------------------------------------------------------------------

    // Let's round the size of `LabelEntry` to 64 bytes (as ZoneHeap has 32
    // bytes granularity anyway). This gives `_name` the remaining space, which
    // is roughly 24 bytes on 64-bit and 36 bytes on 32-bit architectures.
    enum { kNameBytes = 64 - (sizeof(ZoneHashNode) + 8 + sizeof(intptr_t) + sizeof(LabelLink*)) };

    uint8_t _type;                       //!< Label type, see Label::Type.
    uint8_t _flags;                      //!< Must be zero.
    uint16_t _reserved;                  //!< Reserved.
    uint32_t _parentId;                  //!< Label parent id or `kInvalidValue`.
    intptr_t _offset;                    //!< Label offset.
    LabelLink* _links;                   //!< Label links.
    SmallString<kNameBytes> _name;       //!< Label name.
  };

  // --------------------------------------------------------------------------
  // [RelocEntry]
  // --------------------------------------------------------------------------

  //! Code relocation entry.
  //!
  //! X86/X64 Specific
  //! ----------------
  //!
  //! X86 architecture uses 32-bit absolute addressing model encoded in memory
  //! operands, but 64-bit mode uses relative addressing model (RIP + displacement).
  struct RelocEntry {
    uint32_t type;                       //!< Type of the relocation.
    uint32_t size;                       //!< Size of the relocation (4 or 8 bytes).
    uint64_t from;                       //!< Offset from the initial address.
    uint64_t data;                       //!< Displacement from the initial/absolute address.
  };

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Create an uninitialized CodeHolder (you must init() it before it can be used).
  ASMJIT_API CodeHolder() noexcept;
  //! Destroy the CodeHolder.
  ASMJIT_API ~CodeHolder() noexcept;

  // --------------------------------------------------------------------------
  // [Init / Reset]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE bool isInitialized() const noexcept { return _codeInfo.isInitialized(); }

  //! Initialize to CodeHolder to hold code described by `codeInfo`.
  ASMJIT_API Error init(const CodeInfo& info) noexcept;
  //! Detach all code-generators attached and reset the \ref CodeHolder.
  ASMJIT_API void reset(bool releaseMemory = false) noexcept;

  // --------------------------------------------------------------------------
  // [Attach / Detach]
  // --------------------------------------------------------------------------

  //! Attach a \ref CodeEmitter to this \ref CodeHolder.
  ASMJIT_API Error attach(CodeEmitter* emitter) noexcept;
  //! Detach a \ref CodeEmitter from this \ref CodeHolder.
  ASMJIT_API Error detach(CodeEmitter* emitter) noexcept;

  // --------------------------------------------------------------------------
  // [Sync]
  // --------------------------------------------------------------------------

  //! Synchronize all states of all `CodeEmitter`s associated with the CodeHolder.
  //! This is required as some code generators don't sync every time they do
  //! something - for example \ref Assembler generally syncs when it needs to
  //! reallocate the \ref CodeBuffer, but not each time it encodes instruction
  //! or directive.
  ASMJIT_API void sync() noexcept;

  // --------------------------------------------------------------------------
  // [Code-Information]
  // --------------------------------------------------------------------------

  //! Get code/target information, see \ref CodeInfo.
  ASMJIT_INLINE const CodeInfo& getCodeInfo() const noexcept { return _codeInfo; }

  //! Get architecture information, see \ref ArchInfo.
  ASMJIT_INLINE const ArchInfo& getArchInfo() const noexcept { return _codeInfo.getArchInfo(); }
  //! Get the target's architecture type.
  ASMJIT_INLINE uint32_t getArchType() const noexcept { return getArchInfo().getType(); }
  //! Get the target's architecture sub-type.
  ASMJIT_INLINE uint32_t getArchSubType() const noexcept { return getArchInfo().getSubType(); }

  //! Get if a static base-address is set.
  ASMJIT_INLINE bool hasBaseAddress() const noexcept { return _codeInfo.hasBaseAddress(); }
  //! Get a static base-address (uint64_t).
  ASMJIT_INLINE uint64_t getBaseAddress() const noexcept { return _codeInfo.getBaseAddress(); }

  // --------------------------------------------------------------------------
  // [Global Information]
  // --------------------------------------------------------------------------

  //! Get global hints, internally propagated to all `CodeEmitter`s attached.
  ASMJIT_INLINE uint32_t getGlobalHints() const noexcept { return _globalHints; }
  //! Get global options, internally propagated to all `CodeEmitter`s attached.
  ASMJIT_INLINE uint32_t getGlobalOptions() const noexcept { return _globalOptions; }

  // --------------------------------------------------------------------------
  // [Result Information]
  // --------------------------------------------------------------------------

  //! Get the size code & data of all sections.
  ASMJIT_API size_t getCodeSize() const noexcept;

  //! Get size of all possible trampolines.
  //!
  //! Trampolines are needed to successfully generate relative jumps to absolute
  //! addresses. This value is only non-zero if jmp of call instructions were
  //! used with immediate operand (this means jumping or calling an absolute
  //! address directly).
  ASMJIT_INLINE size_t getTrampolinesSize() const noexcept { return _trampolinesSize; }

  // --------------------------------------------------------------------------
  // [Logging & Error Handling]
  // --------------------------------------------------------------------------

#if !defined(ASMJIT_DISABLE_LOGGING)
  //! Get if a logger attached.
  ASMJIT_INLINE bool hasLogger() const noexcept { return _logger != nullptr; }
  //! Get the attached logger.
  ASMJIT_INLINE Logger* getLogger() const noexcept { return _logger; }
  //! Attach a `logger` to CodeHolder and propagate it to all attached `CodeEmitter`s.
  ASMJIT_API void setLogger(Logger* logger) noexcept;
  //! Reset the logger (does nothing if not attached).
  ASMJIT_INLINE void resetLogger() noexcept { setLogger(nullptr); }
#endif // !ASMJIT_DISABLE_LOGGING

  //! Get if error-handler is attached.
  ASMJIT_INLINE bool hasErrorHandler() const noexcept { return _errorHandler != nullptr; }
  //! Get the error-handler.
  ASMJIT_INLINE ErrorHandler* getErrorHandler() const noexcept { return _errorHandler; }
  //! Set the error handler, will affect all attached `CodeEmitter`s.
  ASMJIT_API Error setErrorHandler(ErrorHandler* handler) noexcept;
  //! Reset the error handler (does nothing if not attached).
  ASMJIT_INLINE void resetErrorHandler() noexcept { setErrorHandler(nullptr); }

  // --------------------------------------------------------------------------
  // [Sections]
  // --------------------------------------------------------------------------

  //! Get array of `SectionEntry*` records.
  ASMJIT_INLINE const ZoneVector<SectionEntry*>& getSections() const noexcept { return _sections; }

  //! Get a section entry of the given index.
  ASMJIT_INLINE SectionEntry* getSectionEntry(size_t index) const noexcept { return _sections[index]; }

  ASMJIT_API Error growBuffer(CodeBuffer* cb, size_t n) noexcept;
  ASMJIT_API Error reserveBuffer(CodeBuffer* cb, size_t n) noexcept;

  // --------------------------------------------------------------------------
  // [Labels & Symbols]
  // --------------------------------------------------------------------------

  //! Create a new anonymous label and return its id in `idOut`.
  //!
  //! Returns `Error`, does not report error to \ref ErrorHandler.
  ASMJIT_API Error newLabelId(uint32_t& idOut) noexcept;

  //! Create a new named label label-type `type`.
  //!
  //! Returns `Error`, does not report error to \ref ErrorHandler.
  ASMJIT_API Error newNamedLabelId(uint32_t& idOut, const char* name, size_t nameLength, uint32_t type, uint32_t parentId) noexcept;

  //! Get a label id by name.
  ASMJIT_API uint32_t getLabelIdByName(const char* name, size_t nameLength = kInvalidIndex, uint32_t parentId = kInvalidValue) noexcept;

  //! Create a new label-link used to store information about yet unbound labels.
  //!
  //! Returns `null` if the allocation failed.
  ASMJIT_API LabelLink* newLabelLink() noexcept;

  //! Get array of `LabelEntry*` records.
  ASMJIT_INLINE const ZoneVector<LabelEntry*>& getLabels() const noexcept { return _labels; }

  //! Get number of labels created.
  ASMJIT_INLINE size_t getLabelsCount() const noexcept { return _labels.getLength(); }

  //! Get if the `label` is valid (i.e. created by `newLabelId()`).
  ASMJIT_INLINE bool isLabelValid(const Label& label) const noexcept {
    return isLabelValid(label.getId());
  }
  //! Get if the label having `id` is valid (i.e. created by `newLabelId()`).
  ASMJIT_INLINE bool isLabelValid(uint32_t labelId) const noexcept {
    size_t index = Operand::unpackId(labelId);
    return index < _labels.getLength();
  }

  //! Get if the `label` is already bound.
  //!
  //! Returns `false` if the `label` is not valid.
  ASMJIT_INLINE bool isLabelBound(const Label& label) const noexcept {
    return isLabelBound(label.getId());
  }
  //! \overload
  ASMJIT_INLINE bool isLabelBound(uint32_t id) const noexcept {
    size_t index = Operand::unpackId(id);
    return index < _labels.getLength() && _labels[index]->isBound();
  }

  //! Get a `label` offset or -1 if the label is not yet bound.
  ASMJIT_INLINE intptr_t getLabelOffset(const Label& label) const noexcept {
    return getLabelOffset(label.getId());
  }
  //! \overload
  ASMJIT_INLINE intptr_t getLabelOffset(uint32_t id) const noexcept {
    ASMJIT_ASSERT(isLabelValid(id));
    return _labels[Operand::unpackId(id)]->getOffset();
  }

  //! Get information about the given `label`.
  ASMJIT_INLINE LabelEntry* getLabelEntry(const Label& label) const noexcept {
    return getLabelEntry(label.getId());
  }
  //! Get information about a label having the given `id`.
  ASMJIT_INLINE LabelEntry* getLabelEntry(uint32_t id) const noexcept {
    size_t index = static_cast<size_t>(Operand::unpackId(id));
    return index < _labels.getLength() ? _labels[index] : static_cast<LabelEntry*>(nullptr);
  }

  // --------------------------------------------------------------------------
  // [Relocate]
  // --------------------------------------------------------------------------

  //! Relocate the code to `baseAddress` and copy it to `dst`.
  //!
  //! \param dst Contains the location where the relocated code should be
  //! copied. The pointer can be address returned by virtual memory allocator
  //! or any other address that has sufficient space.
  //!
  //! \param baseAddress Base address used for relocation. `JitRuntime` always
  //! sets the `baseAddress` to be the same as `dst`.
  //!
  //! \return The number bytes actually used. If the code emitter reserved
  //! space for possible trampolines, but didn't use it, the number of bytes
  //! used can actually be less than the expected worst case. Virtual memory
  //! allocator can shrink the memory it allocated initially.
  //!
  //! A given buffer will be overwritten, to get the number of bytes required,
  //! use `getCodeSize()`.
  ASMJIT_API size_t relocate(void* dst, uint64_t baseAddress = kNoBaseAddress) const noexcept;

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  CodeInfo _codeInfo;                    //!< Basic information about the code (architecture and other info).

  uint32_t _globalHints;                 //!< Global hints, propagated to all `CodeEmitter`s.
  uint32_t _globalOptions;               //!< Global options, propagated to all `CodeEmitter`s.

  CodeEmitter* _emitters;                //!< Linked-list of all attached `CodeEmitter`s.
  Assembler* _cgAsm;                     //!< Attached \ref Assembler (only one at a time).

  Logger* _logger;                       //!< Attached \ref Logger, used by all consumers.
  ErrorHandler* _errorHandler;           //!< Attached \ref ErrorHandler.

  uint32_t _trampolinesSize;             //!< Size of all possible trampolines.

  Zone _baseZone;                        //!< Base zone (used to allocate core structures).
  Zone _dataZone;                        //!< Data zone (used to allocate extra data like label names).
  ZoneHeap _baseHeap;                    //!< Zone allocator, used to manage internal containers.

  ZoneVector<SectionEntry*> _sections;   //!< Section entries.
  ZoneVector<LabelEntry*> _labels;       //!< Label entries (each label is stored here).
  ZoneVector<RelocEntry> _relocations;   //!< Relocation entries.
  ZoneHash<LabelEntry> _namedLabels;     //!< Label name -> LabelEntry (only named labels).
};

//! \}

} // asmjit namespace

// [Api-End]
#include "../asmjit_apiend.h"

// [Guard]
#endif // _ASMJIT_BASE_CODEHOLDER_H
