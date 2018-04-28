// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// ZLIB - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_CORE_CODEHOLDER_H
#define _ASMJIT_CORE_CODEHOLDER_H

// [Dependencies]
#include "../core/arch.h"
#include "../core/datatypes.h"
#include "../core/memmgr.h"
#include "../core/operand.h"
#include "../core/stringutils.h"
#include "../core/support.h"
#include "../core/target.h"
#include "../core/zone.h"
#include "../core/zonehash.h"
#include "../core/zonestring.h"
#include "../core/zonevector.h"

ASMJIT_BEGIN_NAMESPACE

//! \addtogroup asmjit_core_api
//! \{

// ============================================================================
// [Forward Declarations]
// ============================================================================

class BaseEmitter;
class CodeHolder;
class Logger;

// ============================================================================
// [asmjit::AlignMode]
// ============================================================================

//! Align mode.
enum AlignMode : uint32_t {
  kAlignCode  = 0,                       //!< Align executable code.
  kAlignData  = 1,                       //!< Align non-executable code.
  kAlignZero  = 2,                       //!< Align by a sequence of zeros.
  kAlignCount = 3                        //!< Count of alignment modes.
};

// ============================================================================
// [asmjit::ErrorHandler]
// ============================================================================

//! Error handler can be used to override the default behavior of error handling
//! available to all classes that inherit `BaseEmitter`.
//!
//! Override `ErrorHandler::handleError()` to implement your own error handler.
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

  //! Error handler (must be reimplemented).
  //!
  //! Error handler is called after an error happened and before it's propagated
  //! to the caller. There are multiple ways how the error handler can be used:
  //!
  //! 1. User-based error handling without throwing exception or using C's
  //!    `longjmp()`. This is for users that don't use exceptions and want
  //!    customized error handling.
  //!
  //! 2. Throwing an exception. AsmJit doesn't use exceptions and is completely
  //!    exception-safe, but you can throw exception from your error handler if
  //!    this way is the preferred way of handling errors in your project.
  //!
  //! 3. Using plain old C's `setjmp()` and `longjmp()`. Asmjit always puts
  //!    `BaseEmitter` to a consistent state before calling `handleError()`
  //!    so `longjmp()` can be used without any issues to cancel the code
  //!    generation if an error occurred. There is no difference between
  //!    exceptions and `longjmp()` from AsmJit's perspective, however,
  //!    never jump outside of `CodeHolder` and `BaseEmitter` scope as you
  //!    would leak memory.
  virtual void handleError(Error err, const char* message, BaseEmitter* origin) = 0;
};

// ============================================================================
// [asmjit::CodeBuffer]
// ============================================================================

//! Code or data buffer.
struct CodeBuffer {
  enum Flags : uint32_t {
    kFlagIsExternal       = 0x00000001u, //!< Buffer is external (not allocated by asmjit).
    kFlagIsFixed          = 0x00000002u  //!< Buffer is fixed (cannot be reallocated).
  };

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  inline uint32_t flags() const noexcept { return _flags; }
  inline bool hasFlag(uint32_t flag) const noexcept { return (_flags & flag) != 0; }

  inline bool isAllocated() const noexcept { return _data != nullptr; }
  inline bool isFixed() const noexcept { return hasFlag(kFlagIsFixed); }
  inline bool isExternal() const noexcept { return hasFlag(kFlagIsExternal); }

  inline uint8_t* data() noexcept { return _data; }
  inline const uint8_t* data() const noexcept { return _data; }

  inline bool empty() const noexcept { return !_size; }
  inline size_t size() const noexcept { return _size; }
  inline size_t capacity() const noexcept { return _capacity; }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  uint8_t* _data;                        //!< The content of the buffer (data).
  size_t _size;                          //!< Number of bytes of `data` used.
  size_t _capacity;                      //!< Buffer capacity (in bytes).
  uint32_t _flags;                       //!< Buffer flags.
};

// ============================================================================
// [asmjit::SectionEntry]
// ============================================================================

//! Section entry.
class SectionEntry {
public:
  enum Id : uint32_t {
    kInvalidId       = 0xFFFFFFFFu       //!< Invalid section id.
  };

  //! Section flags.
  enum Flags : uint32_t {
    kFlagExec        = 0x00000001u,      //!< Executable (.text sections).
    kFlagConst       = 0x00000002u,      //!< Read-only (.text and .data sections).
    kFlagZero        = 0x00000004u,      //!< Zero initialized by the loader (BSS).
    kFlagInfo        = 0x00000008u,      //!< Info / comment flag.
    kFlagImplicit    = 0x80000000u       //!< Section created implicitly and can be deleted by `Target`.
  };

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  inline uint32_t id() const noexcept { return _id; }
  inline const char* name() const noexcept { return _name.str; }

  inline void _setDefaultName(char c0 = 0, char c1 = 0, char c2 = 0, char c3 = 0,
                              char c4 = 0, char c5 = 0, char c6 = 0, char c7 = 0) noexcept {
    _name.u32[0] = Support::bytepack32_4x8(uint8_t(c0), uint8_t(c1), uint8_t(c2), uint8_t(c3));
    _name.u32[1] = Support::bytepack32_4x8(uint8_t(c4), uint8_t(c5), uint8_t(c6), uint8_t(c7));
  }

  inline uint32_t flags() const noexcept { return _flags; }
  inline uint32_t alignment() const noexcept { return _alignment; }
  inline size_t virtualSize() const noexcept { return _virtualSize; }
  inline size_t physicalSize() const noexcept { return _buffer.size(); }

  inline bool hasFlag(uint32_t flag) const noexcept { return (_flags & flag) != 0; }
  inline void addFlags(uint32_t flags) noexcept { _flags |= flags; }
  inline void clearFlags(uint32_t flags) noexcept { _flags &= ~flags; }

  inline void setAlignment(uint32_t alignment) noexcept { _alignment = alignment; }
  inline void setVirtualSize(uint32_t size) noexcept { _virtualSize = size; }

  inline CodeBuffer& buffer() noexcept { return _buffer; }
  inline const CodeBuffer& buffer() const noexcept { return _buffer; }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  uint32_t _id;                          //!< Section id.
  uint32_t _flags;                       //!< Section flags.
  uint32_t _alignment;                   //!< Section alignment requirements (0 if no requirements).
  uint32_t _virtualSize;                 //!< Virtual size of the section (zero initialized mostly).
  StaticString<36> _name;                //!< Section name (max 35 characters, PE allows max 8).
  CodeBuffer _buffer;                    //!< Code or data buffer.
};

// ============================================================================
// [asmjit::LabelLink]
// ============================================================================

//! Data structure used to link unbound labels.
struct LabelLink {
  LabelLink* prev;                       //!< Previous link (single-linked list).
  uint32_t sectionId;                    //!< Section id.
  uint32_t relocId;                      //!< Relocation id or RelocEntry::kInvalidId.
  size_t offset;                         //!< Label offset relative to the start of the section.
  intptr_t rel;                          //!< Inlined rel8/rel32.
};

// ============================================================================
// [asmjit::LabelEntry]
// ============================================================================

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
  inline uint32_t id() const noexcept { return _customData; }
  //! Set label id (internal, used only by `CodeHolder`).
  inline void _setId(uint32_t id) noexcept { _customData = id; }

  //! Get label type, see `Label::LabelType`.
  inline uint32_t type() const noexcept { return _type; }
  //! Get label flags, returns 0 at the moment.
  inline uint32_t flags() const noexcept { return _flags; }

  inline bool hasParent() const noexcept { return _parentId != 0; }
  //! Get label's parent id.
  inline uint32_t parentId() const noexcept { return _parentId; }
  //! Get label's section id where it's bound to (or `SectionEntry::kInvalidId` if it's not bound yet).
  inline uint32_t sectionId() const noexcept { return _sectionId; }

  //! Get whether the label has name.
  inline bool hasName() const noexcept { return !_name.empty(); }

  //! Get the label's name.
  //!
  //! NOTE: Local labels will return their local name without their parent
  //! part, for example ".L1".
  inline const char* name() const noexcept { return _name.data(); }

  //! Get size of label's name.
  //!
  //! NOTE: Label name is always null terminated, so you can use `strlen()` to
  //! get it, however, it's also cached in `LabelEntry` itself, so if you want
  //! to know the size the fastest way is to call `LabelEntry::nameSize()`.
  inline uint32_t nameSize() const noexcept { return _name.size(); }

  //! Get whether the label is bound.
  inline bool isBound() const noexcept { return _sectionId != SectionEntry::kInvalidId; }
  //! Get the label offset (only useful if the label is bound).
  inline intptr_t offset() const noexcept { return _offset; }

  //! Get the hash-value of label's name and its parent label (if any).
  //!
  //! Label hash is calculated as `HASH(Name) ^ ParentId`. The hash function
  //! is implemented in `hashString::hashString()` and `StringUtils::hashRound()`.
  inline uint32_t hashCode() const noexcept { return _hashCode; }

  // ------------------------------------------------------------------------
  // [Members]
  // ------------------------------------------------------------------------

  // Let's round the size of `LabelEntry` to 64 bytes (as `ZoneAllocator` has
  // granularity of 32 bytes anyway). This gives `_name` the remaining space,
  // which is should be 16 bytes on 64-bit and 28 bytes on 32-bit architectures.
  static constexpr uint32_t kStaticNameSize =
    64 - (sizeof(ZoneHashNode) + 16 + sizeof(intptr_t) + sizeof(LabelLink*));

  uint8_t _type;                         //!< Label type, see `Label::LabelType`.
  uint8_t _flags;                        //!< Must be zero.
  uint16_t _reserved16;                  //!< Reserved.
  uint32_t _parentId;                    //!< Label parent id or zero.
  uint32_t _sectionId;                   //!< Section id or `SectionEntry::kInvalidId`.
  uint32_t _reserved32;                  //!< Reserved.
  intptr_t _offset;                      //!< Label offset.
  LabelLink* _links;                     //!< Label links.
  ZoneString<kStaticNameSize> _name;     //!< Label name.
};

// ============================================================================
// [asmjit::RelocEntry]
// ============================================================================

//! Relocation entry.
struct RelocEntry {
  enum Id : uint32_t {
    kInvalidId       = 0xFFFFFFFFu       //!< Invalid relocation id.
  };

  //! Relocation type.
  enum RelocType : uint32_t {
    kTypeNone        = 0,                //!< Deleted entry (no relocation).
    kTypeAbsToAbs    = 1,                //!< Relocate absolute to absolute.
    kTypeRelToAbs    = 2,                //!< Relocate relative to absolute.
    kTypeAbsToRel    = 3,                //!< Relocate absolute to relative.
    kTypeTrampoline  = 4                 //!< Relocate absolute to relative or use trampoline.
  };

  // ------------------------------------------------------------------------
  // [Accessors]
  // ------------------------------------------------------------------------

  inline uint32_t id() const noexcept { return _id; }

  inline uint32_t type() const noexcept { return _type; }
  inline uint32_t size() const noexcept { return _size; }

  inline uint32_t sourceSectionId() const noexcept { return _sourceSectionId; }
  inline uint32_t targetSectionId() const noexcept { return _targetSectionId; }

  inline uint64_t sourceOffset() const noexcept { return _sourceOffset; }
  inline uint64_t payload() const noexcept { return _payload; }

  // ------------------------------------------------------------------------
  // [Members]
  // ------------------------------------------------------------------------

  uint32_t _id;                          //!< Relocation id.
  uint8_t _type;                         //!< Type of the relocation.
  uint8_t _size;                         //!< Size of the relocation (1, 2, 4 or 8 bytes).
  uint8_t _reserved[2];                  //!< Reserved.
  uint32_t _sourceSectionId;             //!< Source section id.
  uint32_t _targetSectionId;             //!< Destination section id.
  uint64_t _sourceOffset;                //!< Source offset (relative to start of the section).
  uint64_t _payload;                     //!< Payload (target offset, target address, etc).
};

// ============================================================================
// [asmjit::CodeHolder]
// ============================================================================

//! Contains basic information about the target architecture plus its settings,
//! and holds code & data (including sections, labels, and relocation information).
//! CodeHolder can store both binary and intermediate representation of assembly,
//! which can be generated by `BaseAssembler` and/or `BaseBuilder`.
//!
//! NOTE: `CodeHolder` has ability to attach an `ErrorHandler`, however, the
//! error handler is not triggered by `CodeHolder` itself, it's only used by
//! emitters attached to `CodeHolder`.
class CodeHolder {
public:
  ASMJIT_NONCOPYABLE(CodeHolder)

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

  inline bool isInitialized() const noexcept { return _codeInfo.isInitialized(); }

  //! Initialize to CodeHolder to hold code described by `codeInfo`.
  ASMJIT_API Error init(const CodeInfo& info) noexcept;
  //! Detach all code-generators attached and reset the `CodeHolder`.
  ASMJIT_API void reset(uint32_t resetPolicy = Globals::kResetSoft) noexcept;

  // --------------------------------------------------------------------------
  // [Attach / Detach]
  // --------------------------------------------------------------------------

  //! Attach an emitter to this `CodeHolder`.
  ASMJIT_API Error attach(BaseEmitter* emitter) noexcept;
  //! Detach an emitter from this `CodeHolder`.
  ASMJIT_API Error detach(BaseEmitter* emitter) noexcept;

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  inline ZoneAllocator* allocator() const noexcept { return const_cast<ZoneAllocator*>(&_allocator); }
  inline const ZoneVector<BaseEmitter*>& emitters() const noexcept { return _emitters; }

  // --------------------------------------------------------------------------
  // [Code / Arch]
  // --------------------------------------------------------------------------

  //! Get architecture information, see `ArchInfo`.
  inline const ArchInfo& archInfo() const noexcept { return _codeInfo.archInfo(); }
  //! Get code/target information, see `CodeInfo`.
  inline const CodeInfo& codeInfo() const noexcept { return _codeInfo; }

  //! Get the target's architecture type.
  inline uint32_t archId() const noexcept { return archInfo().archId(); }
  //! Get the target's architecture sub-type.
  inline uint32_t archSubId() const noexcept { return archInfo().archSubId(); }

  //! Get whether a static base-address is set.
  inline bool hasBaseAddress() const noexcept { return _codeInfo.hasBaseAddress(); }
  //! Get a static base-address (uint64_t).
  inline uint64_t baseAddress() const noexcept { return _codeInfo.baseAddress(); }

  // --------------------------------------------------------------------------
  // [Emitter Options]
  // --------------------------------------------------------------------------

  //! Get global hints, internally propagated to all code emitters attached.
  inline uint32_t emitterOptions() const noexcept { return _emitterOptions; }

  // --------------------------------------------------------------------------
  // [Result Information]
  // --------------------------------------------------------------------------

  //! Get the size code & data of all sections.
  ASMJIT_API size_t codeSize() const noexcept;

  //! Get size of all possible trampolines.
  //!
  //! Trampolines are needed to successfully generate relative jumps to absolute
  //! addresses. This value is only non-zero if jmp of call instructions were
  //! used with immediate operand (this means jumping or calling an absolute
  //! address directly).
  inline size_t getTrampolinesSize() const noexcept { return _trampolinesSize; }

  // --------------------------------------------------------------------------
  // [Logging & Error Handling]
  // --------------------------------------------------------------------------

  //! Get the attached logger.
  inline Logger* logger() const noexcept { return _logger; }
  //! Attach a `logger` to CodeHolder and propagate it to all attached code emitters.
  ASMJIT_API void setLogger(Logger* logger) noexcept;
  //! Reset the logger (does nothing if not attached).
  inline void resetLogger() noexcept { setLogger(nullptr); }

  //! Get whether the global error handler is attached.
  inline bool hasErrorHandler() const noexcept { return _errorHandler != nullptr; }
  //! Get the global error handler.
  inline ErrorHandler* errorHandler() const noexcept { return _errorHandler; }
  //! Set the global error handler.
  inline void setErrorHandler(ErrorHandler* handler) noexcept { _errorHandler = handler; }
  //! Reset the global error handler (does nothing if not attached).
  inline void resetErrorHandler() noexcept { setErrorHandler(nullptr); }

  // --------------------------------------------------------------------------
  // [Sections]
  // --------------------------------------------------------------------------

  //! Get array of `SectionEntry*` records.
  inline const ZoneVector<SectionEntry*>& sectionEntries() const noexcept { return _sectionEntries; }
  //! Get a section entry of the given index.
  inline SectionEntry* sectionEntry(uint32_t index) const noexcept { return _sectionEntries[index]; }
  //! Get the number of sections.
  inline uint32_t sectionCount() const noexcept { return _sectionEntries.size(); }

  ASMJIT_API Error growBuffer(CodeBuffer* cb, size_t n) noexcept;
  ASMJIT_API Error reserveBuffer(CodeBuffer* cb, size_t n) noexcept;

  // --------------------------------------------------------------------------
  // [Labels & Symbols]
  // --------------------------------------------------------------------------

  //! Create a new anonymous label and return its id in `idOut`.
  //!
  //! Returns `Error`, does not report error to `ErrorHandler`.
  ASMJIT_API Error newLabelId(uint32_t& idOut) noexcept;

  //! Create a new named label label-type `type`.
  //!
  //! Returns `Error`, does not report error to `ErrorHandler`.
  ASMJIT_API Error newNamedLabelId(uint32_t& idOut, const char* name, size_t nameSize, uint32_t type, uint32_t parentId) noexcept;

  //! Get a label id by name.
  ASMJIT_API uint32_t labelIdByName(const char* name, size_t nameSize = Globals::kNullTerminated, uint32_t parentId = 0) noexcept;

  inline Label labelByName(const char* name, size_t nameSize = Globals::kNullTerminated, uint32_t parentId = 0) noexcept {
    return Label(labelIdByName(name, nameSize, parentId));
  }

  //! Create a new label-link used to store information about yet unbound labels.
  //!
  //! Returns `null` if the allocation failed.
  ASMJIT_API LabelLink* newLabelLink(LabelEntry* le, uint32_t sectionId, size_t offset, intptr_t rel) noexcept;

  //! Get array of `LabelEntry*` records.
  inline const ZoneVector<LabelEntry*>& labelEntries() const noexcept { return _labelEntries; }

  //! Get information about a label of the given `id`.
  inline LabelEntry* labelEntry(uint32_t id) const noexcept {
    uint32_t index = Operand::unpackId(id);
    return index < _labelEntries.size() ? _labelEntries[index] : static_cast<LabelEntry*>(nullptr);
  }
  //! Get information about the given `label`.
  inline LabelEntry* labelEntry(const Label& label) const noexcept {
    return labelEntry(label.id());
  }

  //! Get a `label` offset or -1 if the label is not yet bound.
  inline intptr_t labelOffset(uint32_t id) const noexcept {
    ASMJIT_ASSERT(isLabelValid(id));
    return _labelEntries[Operand::unpackId(id)]->offset();
  }
  //! Get a `label` offset or -1 if the label is not yet bound.
  inline intptr_t labelOffset(const Label& label) const noexcept {
    return labelOffset(label.id());
  }

  //! Get number of labels created.
  inline uint32_t labelCount() const noexcept { return _labelEntries.size(); }
  //! Get number of label references, which are unresolved at the moment.
  inline uint32_t unresolvedLabelCount() const noexcept { return _unresolvedLabelCount; }

  //! Get whether the label having `id` is valid (i.e. created by `newLabelId()`).
  inline bool isLabelValid(uint32_t labelId) const noexcept {
    uint32_t index = Operand::unpackId(labelId);
    return index < _labelEntries.size();
  }
  //! Get whether the `label` is valid (i.e. created by `newLabelId()`).
  inline bool isLabelValid(const Label& label) const noexcept {
    return isLabelValid(label.id());
  }

  //! \overload
  inline bool isLabelBound(uint32_t id) const noexcept {
    uint32_t index = Operand::unpackId(id);
    return index < _labelEntries.size() && _labelEntries[index]->isBound();
  }
  //! Get whether the `label` is already bound.
  //!
  //! Returns `false` if the `label` is not valid.
  inline bool isLabelBound(const Label& label) const noexcept {
    return isLabelBound(label.id());
  }

  // --------------------------------------------------------------------------
  // [Relocations]
  // --------------------------------------------------------------------------

  //! Get whether the code contains relocation entries.
  inline bool hasRelocEntries() const noexcept { return !_relocations.empty(); }
  //! Get array of `RelocEntry*` records.
  inline const ZoneVector<RelocEntry*>& relocEntries() const noexcept { return _relocations; }

  //! Get reloc entry of a given `id`.
  inline RelocEntry* relocEntry(uint32_t id) const noexcept { return _relocations[id]; }

  //! Create a new relocation entry of type `type` and size `size`.
  //!
  //! Additional fields can be set after the relocation entry was created.
  ASMJIT_API Error newRelocEntry(RelocEntry** dst, uint32_t type, uint32_t size) noexcept;

  //! Relocate the code to `baseAddress` and copy it to `dst`.
  //!
  //! \param dst Contains the location where the relocated code should be
  //! copied. The pointer can be address returned by virtual memory allocator
  //! or any other address that has sufficient space.
  //!
  //! \param baseAddress Base address used for relocation. `JitRuntime` always
  //! sets the `baseAddress` to be the same as `dst`.
  //!
  //! \return The number of bytes actually used. If the code emitter reserved
  //! space for possible trampolines, but didn't use it, the number of bytes
  //! used can actually be less than the expected worst case. Virtual memory
  //! allocator can shrink the memory it allocated initially.
  //!
  //! A given buffer will be overwritten, to get the number of bytes required,
  //! use `codeSize()`.
  ASMJIT_API size_t relocate(void* dst, uint64_t baseAddress = Globals::kNoBaseAddress) const noexcept;

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  CodeInfo _codeInfo;                    //!< Basic information about the code (architecture and other info).
  uint32_t _emitterOptions;              //!< Emitter options, propagated to all emitters when changed.

  Logger* _logger;                       //!< Attached `Logger`, used by all consumers.
  ErrorHandler* _errorHandler;           //!< Attached `ErrorHandler`.

  uint32_t _unresolvedLabelCount;        //!< Count of label references which were not resolved.
  uint32_t _trampolinesSize;             //!< Size of all possible trampolines.

  Zone _zone;                            //!< Code zone (used to allocate core structures).
  ZoneAllocator _allocator;              //!< Zone allocator, used to manage internal containers.

  ZoneVector<BaseEmitter*> _emitters;    //!< Attached code emitters.
  ZoneVector<SectionEntry*> _sectionEntries; //!< Section entries.
  ZoneVector<LabelEntry*> _labelEntries; //!< Label entries.
  ZoneVector<RelocEntry*> _relocations;  //!< Relocation entries.
  ZoneHash<LabelEntry> _namedLabels;     //!< Label name -> LabelEntry (only named labels).
};

//! \}

ASMJIT_END_NAMESPACE

// [Guard]
#endif // _ASMJIT_CORE_CODEHOLDER_H
