// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_CORE_CODEHOLDER_H_INCLUDED
#define ASMJIT_CORE_CODEHOLDER_H_INCLUDED

#include "../core/archtraits.h"
#include "../core/codebuffer.h"
#include "../core/errorhandler.h"
#include "../core/fixup.h"
#include "../core/operand.h"
#include "../core/string.h"
#include "../core/support.h"
#include "../core/target.h"
#include "../core/zone.h"
#include "../core/zonehash.h"
#include "../core/zonestring.h"
#include "../core/zonetree.h"
#include "../core/zonevector.h"

ASMJIT_BEGIN_NAMESPACE

//! \addtogroup asmjit_core
//! \{

class BaseEmitter;
class CodeHolder;
class LabelEntry;
class Logger;

//! Operator type that can be used within an \ref Expression.
enum class ExpressionOpType : uint8_t {
  //! Addition.
  kAdd = 0,
  //! Subtraction.
  kSub = 1,
  //! Multiplication
  kMul = 2,
  //! Logical left shift.
  kSll = 3,
  //! Logical right shift.
  kSrl = 4,
  //! Arithmetic right shift.
  kSra = 5
};

//! Value type that can be used within an \ref Expression.
enum class ExpressionValueType : uint8_t {
  //! No value or invalid.
  kNone = 0,
  //! Value is 64-bit unsigned integer (constant).
  kConstant = 1,
  //! Value is \ref LabelEntry, which references a \ref Label.
  kLabel = 2,
  //! Value is \ref Expression
  kExpression = 3
};

//! Expression node that can reference constants, labels, and another expressions.
struct Expression {
  //! Expression value.
  union Value {
    //! Constant.
    uint64_t constant;
    //! Pointer to another expression.
    Expression* expression;
    //! Label identifier
    uint32_t labelId;
  };

  //! \name Members
  //! \{

  //! Operation type.
  ExpressionOpType opType;
  //! Value types of \ref value.
  ExpressionValueType valueType[2];
  //! Reserved for future use, should be initialized to zero.
  uint8_t reserved[5];
  //! Expression left and right values.
  Value value[2];

  //! \}

  //! \name Accessors
  //! \{

  //! Resets the whole expression.
  //!
  //! Changes both values to \ref ExpressionValueType::kNone.
  ASMJIT_INLINE_NODEBUG void reset() noexcept { *this = Expression{}; }

  //! Sets the value type at `index` to \ref ExpressionValueType::kConstant and its content to `constant`.
  ASMJIT_INLINE_NODEBUG void setValueAsConstant(size_t index, uint64_t constant) noexcept {
    valueType[index] = ExpressionValueType::kConstant;
    value[index].constant = constant;
  }

  //! Sets the value type at `index` to \ref ExpressionValueType::kLabel and its content to `labelEntry`.
  ASMJIT_INLINE_NODEBUG void setValueAsLabelId(size_t index, uint32_t labelId) noexcept {
    valueType[index] = ExpressionValueType::kLabel;
    value[index].labelId = labelId;
  }

  //! Sets the value type at `index` to \ref ExpressionValueType::kExpression and its content to `expression`.
  ASMJIT_INLINE_NODEBUG void setValueAsExpression(size_t index, Expression* expression) noexcept {
    valueType[index] = ExpressionValueType::kExpression;
    value[index].expression = expression;
  }

  //! \}
};

//! Relocation type.
enum class RelocType : uint32_t {
  //! None/deleted (no relocation).
  kNone = 0,
  //! Expression evaluation, `_payload` is pointer to `Expression`.
  kExpression = 1,
  //! Relative relocation from one section to another.
  kSectionRelative = 2,
  //! Relocate absolute to absolute.
  kAbsToAbs = 3,
  //! Relocate relative to absolute.
  kRelToAbs = 4,
  //! Relocate absolute to relative.
  kAbsToRel = 5,
  //! Relocate absolute to relative or use trampoline.
  kX64AddressEntry = 6
};

//! Type of the \ref Label.
enum class LabelType : uint8_t {
  //! Anonymous label that can optionally have a name, which is only used for debugging purposes.
  kAnonymous = 0u,
  //! Local label (always has parentId).
  kLocal = 1u,
  //! Global label (never has parentId).
  kGlobal = 2u,
  //! External label (references an external symbol).
  kExternal = 3u,

  //! Maximum value of `LabelType`.
  kMaxValue = kExternal
};

//! Label flags describe some details about labels, mostly for AsmJit's own use.
enum class LabelFlags : uint8_t {
  kNone = 0x00u,
  kHasOwnExtraData = 0x01u,
  kHasName = 0x02u,
  kHasParent = 0x04u
};
ASMJIT_DEFINE_ENUM_FLAGS(LabelFlags)

//! Section flags, used by \ref Section.
enum class SectionFlags : uint32_t {
  //! No flags.
  kNone = 0,
  //! Executable (.text sections).
  kExecutable = 0x0001u,
  //! Read-only (.text and .data sections).
  kReadOnly = 0x0002u,
  //! Zero initialized by the loader (BSS).
  kZeroInitialized = 0x0004u,
  //! Info / comment flag.
  kComment = 0x0008u,
  //! Section is built in and created by default (.text section).
  kBuiltIn = 0x4000u,
  //! Section created implicitly, can be deleted by \ref Target.
  kImplicit = 0x8000u
};
ASMJIT_DEFINE_ENUM_FLAGS(SectionFlags)

//! Flags that can be used with \ref CodeHolder::copySectionData() and \ref CodeHolder::copyFlattenedData().
enum class CopySectionFlags : uint32_t {
  //! No flags.
  kNone = 0,

  //! If virtual size of a section is greater than the size of its \ref CodeBuffer then all bytes between the buffer
  //! size and virtual size will be zeroed. If this option is not set then those bytes would be left as is, which
  //! means that if the user didn't initialize them they would have a previous content, which may be unwanted.
  kPadSectionBuffer = 0x00000001u,

  //! Clears the target buffer if the flattened data is less than the destination size. This option works
  //! only with \ref CodeHolder::copyFlattenedData() as it processes multiple sections. It is ignored by
  //! \ref CodeHolder::copySectionData().
  kPadTargetBuffer = 0x00000002u
};
ASMJIT_DEFINE_ENUM_FLAGS(CopySectionFlags)

//! Base class for both \ref Section and \ref LabelEntry::ExtraData.
class SectionOrLabelEntryExtraHeader {
public:
  //! \name Members
  //! \{

  //! Section id - describes either a section where a \ref Label is bound or it's a real section id of \ref Section.
  uint32_t _sectionId;

  //! Internal label type is only used by \ref LabelEntry::ExtraData. \ref Section always leaves this field zero,
  //! which describes an anonymous label. Anonymous labels are default and always used when there is no
  //! \ref LabelEntry::ExtraData
  LabelType _internalLabelType;

  //! Internal label flags, used by \ref LabelEntry::ExtraData. \ref Section doesn't use these flags and sets them
  //! to zero.
  LabelFlags _internalLabelFlags;

  //! Internal data used freely by \ref Section and \ref LabelEntry::ExtraData.
  uint16_t _internalUInt16Data;

  //! \}
};

//! Section entry.
class Section : public SectionOrLabelEntryExtraHeader {
public:
  //! \name Members
  //! \{

  //! Section alignment requirements (0 if no requirements).
  uint32_t _alignment;
  //! Order (lower value means higher priority).
  int32_t _order;
  //! Offset of this section from base-address.
  uint64_t _offset;
  //! Virtual size of the section (zero initialized sections).
  uint64_t _virtualSize;
  //! Section name (max 35 characters, PE allows max 8).
  FixedString<Globals::kMaxSectionNameSize + 1> _name;
  //! Code or data buffer.
  CodeBuffer _buffer;

  //! \}

  //! \name Accessors
  //! \{

  //! Returns the section id.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t sectionId() const noexcept { return _sectionId; }

  //! Returns the section name, as a null terminated string.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const char* name() const noexcept { return _name.str; }

  //! Returns the section data.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint8_t* data() noexcept { return _buffer.data(); }

  //! \overload
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const uint8_t* data() const noexcept { return _buffer.data(); }

  //! Returns the section flags.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG SectionFlags flags() const noexcept { return SectionFlags(_internalUInt16Data); }

  //! Tests whether the section has the given `flag`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool hasFlag(SectionFlags flag) const noexcept { return Support::test(_internalUInt16Data, uint32_t(flag)); }

  //! Assigns `flags` to the section (replaces all existing flags).
  ASMJIT_INLINE_NODEBUG void assignFlags(SectionFlags flags) noexcept { _internalUInt16Data = uint16_t(flags); }

  //! Adds `flags` to the section flags.
  ASMJIT_INLINE_NODEBUG void addFlags(SectionFlags flags) noexcept { _internalUInt16Data = uint16_t(_internalUInt16Data | uint32_t(flags)); }

  //! Removes `flags` from the section flags.
  ASMJIT_INLINE_NODEBUG void clearFlags(SectionFlags flags) noexcept { _internalUInt16Data = uint16_t(_internalUInt16Data | ~uint32_t(flags)); }

  //! Returns the minimum section alignment
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t alignment() const noexcept { return _alignment; }

  //! Sets the minimum section alignment
  ASMJIT_INLINE_NODEBUG void setAlignment(uint32_t alignment) noexcept { _alignment = alignment; }

  //! Returns the section order, which has a higher priority than section id.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG int32_t order() const noexcept { return _order; }

  //! Returns the section offset, relative to base.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint64_t offset() const noexcept { return _offset; }

  //! Set the section offset.
  ASMJIT_INLINE_NODEBUG void setOffset(uint64_t offset) noexcept { _offset = offset; }

  //! Returns the virtual size of the section.
  //!
  //! Virtual size is initially zero and is never changed by AsmJit. It's normal if virtual size is smaller than
  //! size returned by `bufferSize()` as the buffer stores real data emitted by assemblers or appended by users.
  //!
  //! Use `realSize()` to get the real and final size of this section.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint64_t virtualSize() const noexcept { return _virtualSize; }

  //! Sets the virtual size of the section.
  ASMJIT_INLINE_NODEBUG void setVirtualSize(uint64_t virtualSize) noexcept { _virtualSize = virtualSize; }

  //! Returns the buffer size of the section.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t bufferSize() const noexcept { return _buffer.size(); }

  //! Returns the real size of the section calculated from virtual and buffer sizes.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint64_t realSize() const noexcept { return Support::max<uint64_t>(virtualSize(), bufferSize()); }

  //! Returns the `CodeBuffer` used by this section.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG CodeBuffer& buffer() noexcept { return _buffer; }

  //! Returns the `CodeBuffer` used by this section (const).
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const CodeBuffer& buffer() const noexcept { return _buffer; }

  //! \}
};

//! Entry in an address table.
class AddressTableEntry : public ZoneTreeNodeT<AddressTableEntry> {
public:
  ASMJIT_NONCOPYABLE(AddressTableEntry)

  //! \name Members
  //! \{

  //! Address.
  uint64_t _address;
  //! Slot.
  uint32_t _slot;

  //! \}

  //! \name Construction & Destruction
  //! \{

  ASMJIT_INLINE_NODEBUG explicit AddressTableEntry(uint64_t address) noexcept
    : _address(address),
      _slot(0xFFFFFFFFu) {}

  //! \}

  //! \name Accessors
  //! \{

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint64_t address() const noexcept { return _address; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t slot() const noexcept { return _slot; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool hasAssignedSlot() const noexcept { return _slot != 0xFFFFFFFFu; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool operator<(const AddressTableEntry& other) const noexcept { return _address < other._address; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool operator>(const AddressTableEntry& other) const noexcept { return _address > other._address; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool operator<(uint64_t queryAddress) const noexcept { return _address < queryAddress; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool operator>(uint64_t queryAddress) const noexcept { return _address > queryAddress; }

  //! \}
};

//! Relocation entry.
struct RelocEntry {
  //! \name Members
  //! \{

  //! Relocation id.
  uint32_t _id;
  //! Type of the relocation.
  RelocType _relocType;
  //! Format of the relocated value.
  OffsetFormat _format;
  //! Source section id.
  uint32_t _sourceSectionId;
  //! Target section id.
  uint32_t _targetSectionId;
  //! Source offset (relative to start of the section).
  uint64_t _sourceOffset;
  //! Payload (target offset, target address, expression, etc).
  uint64_t _payload;

  //! \}

  //! \name Accessors
  //! \{

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t id() const noexcept { return _id; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RelocType relocType() const noexcept { return _relocType; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const OffsetFormat& format() const noexcept { return _format; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t sourceSectionId() const noexcept { return _sourceSectionId; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t targetSectionId() const noexcept { return _targetSectionId; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint64_t sourceOffset() const noexcept { return _sourceOffset; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint64_t payload() const noexcept { return _payload; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Expression* payloadAsExpression() const noexcept {
    return reinterpret_cast<Expression*>(uintptr_t(_payload));
  }

  //! \}
};

//! Label entry provides data stored by \ref CodeHolder for each \ref Label.
//!
//! Label entry is used mostly internall by AsmJit, but it's possibly to use it to query various information about
//! a label. For example to get its type, flags, name, and fixups (if the label is not bound) or offset (if the label
//! is bound).
//!
//! To make the entry small, it's currently split into two data structures - \ref LabelEntry, which is stored in an
//! array as a value, and \ref LabelEntry::ExtraData, which can be pointed to via \ref LabelEntry::_objectData. Extra
//! data of unnamed anonymous labels is shared (and immutable), thus all unnamed anonymous labels would only use
//! \ref LabelEntry (16 bytes per label).
class LabelEntry {
public:
  //! Contains extra data that is only created when the label is not anonymous or has a name.
  struct ExtraData : public SectionOrLabelEntryExtraHeader {
    //! Label parent id or zero.
    uint32_t _parentId;
    //! Label name length.
    uint32_t _nameSize;

    //! Returns a name associated with this extra data - a valid pointer is only returned when the label has a name, which
    //! is marked by \ref LabelFlags::kHasName flag.
    ASMJIT_INLINE_NODEBUG const char* name() const noexcept { return Support::offsetPtr<char>(this, sizeof(ExtraData)); }
  };

  //! \name Members
  //! \{

  // Either references a \ref Section where the label is bound or \ref ExtraData.
  SectionOrLabelEntryExtraHeader* _objectData;

  //! Label entry payload.
  //!
  //! When a Label is bound, `_offsetOrFixups` is the relative offset from the start of the section where
  //! the \ref Label has been bound, otherwise `_offsetOrFixups` is a pointer to the first \ref Fixup.
  uint64_t _offsetOrFixups;

  //! \}

  //! \name Accessors
  //! \{

  //! Returns the type of the label.
  //!
  //! The type of the label depends on how it was created. Most JIT code uses unnamed anonymous labels created by
  //! emitters, for example \ref BaseEmitter::newLabel() returns a \ref Label instance having id that was created
  //! by \ref CodeHolder::newLabelId.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG LabelType labelType() const noexcept { return _objectData->_internalLabelType; }

  //! Returns label flags.
  //!
  //! \note Label flags are mostly for internal use, there is probably no reason to use them in user code.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG LabelFlags labelFlags() const noexcept { return _objectData->_internalLabelFlags; }

  //! Tests whether the label has the given `flag` set.
  //!
  //! \note Using other getters instead is advised, for example using \ref hasName() and \ref hasParent() is better
  //! (and shorter) than checking label flags.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool hasLabelFlag(LabelFlags flag) const noexcept { return Support::test(_objectData->_internalLabelFlags, flag); }

  //! Tests whether the LabelEntry has own extra data (see \ref LabelEntry::ExtraData).
  //!
  //! \note This should only be used by AsmJit for internal purposes. Own extra data means that the LabelEntry has
  //! a mutable extra data separately allocated. This information should not be necessary to users as LabelEntry
  //! getters should encapsulate label introspection.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool _hasOwnExtraData() const noexcept { return hasLabelFlag(LabelFlags::kHasOwnExtraData); }

  //! Tests whether the Label represented by this LabelEntry has a name.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool hasName() const noexcept { return hasLabelFlag(LabelFlags::kHasName); }

  //! Tests whether the Label represented by this LabelEntry has a parent label.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool hasParent() const noexcept { return hasLabelFlag(LabelFlags::kHasParent); }

  //! Tests whether the label represented by this LabelEntry is bound.
  //!
  //! Bound label means that it has an associated \ref Section and a position in such section. Labels are bound by
  //! calling \ref BaseEmitter::bind() method with \ref Label operand.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool isBound() const noexcept { return _objectData->_sectionId != Globals::kInvalidId; }

  //! Tests whether the label is bound to a the given `section`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool isBoundTo(const Section* section) const noexcept { return _objectData->_sectionId == section->sectionId(); }

  //! Tests whether the label is bound to a the given `sectionId`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool isBoundTo(uint32_t sectionId) const noexcept { return _objectData->_sectionId == sectionId; }

  //! Returns the section where the label was bound.
  //!
  //! If the label was not yet bound the return value is `nullptr`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t sectionId() const noexcept { return _objectData->_sectionId; }

  [[nodiscard]]
  ASMJIT_INLINE ExtraData* _ownExtraData() const noexcept {
    ASMJIT_ASSERT(_hasOwnExtraData());
    return static_cast<ExtraData*>(_objectData);
  }

  //! Returns label's parent id or \ref Globals::kInvalidId if the label has no parent.
  [[nodiscard]]
  ASMJIT_INLINE uint32_t parentId() const noexcept {
    return _hasOwnExtraData() ? _ownExtraData()->_parentId : Globals::kInvalidId;
  }

  //! Returns the label's name.
  //!
  //! \note Local labels will return their local name without their parent part, for example ".L1".
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const char* name() const noexcept {
    return hasName() ? _ownExtraData()->name() : nullptr;
  }

  //! Returns size of label's name.
  //!
  //! \note Label name is always null terminated, so you can use `strlen()` to get it, however, it's also cached in
  //! `LabelEntry` itself, so if you want to know the size the fastest way is to call `LabelEntry::nameSize()`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t nameSize() const noexcept {
    return hasName() ? _ownExtraData()->_nameSize : uint32_t(0);
  }

  //! Returns unresolved fixups associated with this label.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool hasFixups() const noexcept {
    return Support::bool_and(!isBound(), _offsetOrFixups != 0u);
  }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Fixup* _getFixups() const noexcept { return reinterpret_cast<Fixup*>(uintptr_t(_offsetOrFixups)); }

  ASMJIT_INLINE_NODEBUG void _setFixups(Fixup* first) noexcept { _offsetOrFixups = reinterpret_cast<uintptr_t>(first); }

  //! Returns unresolved fixups associated with this label.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Fixup* unresolvedLinks() const noexcept { return !isBound() ? _getFixups() : nullptr; }

  //! Returns the label offset (can only be used after the label is bound).
  //!
  //! \note This would trigger an assertion failure in debug builds when called on an unbound label. When accessing
  //! offsets, always check whether the label is bound. Unbound labels don't have offsets.
  [[nodiscard]]
  ASMJIT_INLINE uint64_t offset() const noexcept {
    ASMJIT_ASSERT(isBound());
    return _offsetOrFixups;
  }

  //! \}
};

//! Holds assembled code and data (including sections, labels, and relocation information).
//!
//! CodeHolder connects emitters with their targets. It provides them interface that can be used to query information
//! about the target environment (architecture, etc...) and API to create labels, sections, relocations, and to write
//! data to a \ref CodeBuffer, which is always part of \ref Section. More than one emitter can be attached to a single
//! CodeHolder instance at a time, which is used in practice
//!
//! CodeHolder provides interface for all emitter types. Assemblers use CodeHolder to write into \ref CodeBuffer, and
//! higher level emitters like Builder and Compiler use CodeHolder to manage labels and sections so higher level code
//! can be serialized to Assembler by \ref BaseEmitter::finalize() and \ref BaseBuilder::serializeTo().
//!
//! In order to use CodeHolder, it must be first initialized by \ref init(). After the CodeHolder has been successfully
//! initialized it can be used to hold assembled code, sections, labels, relocations, and to attach / detach code
//! emitters. After the end of code generation it can be used to query physical locations of labels and to relocate
//! the assembled code into the right address.
//!
//! CodeHolder Reusability
//! ----------------------
//!
//! If you intend to generate a lot of code, or tiny code, it's advised to reuse CodeHolder and emitter instances.
//! There are currently two ways of reusing CodeHolder and emitters - one is using \ref CodeHolder::init() followed
//! by \ref CodeHolder::reset(), and another is initializing once by \ref CodeHolder::init() and then reinitializing
//! by \ref CodeHolder::reinit(). The first strategy is shown below:
//!
//! ```
//! // All of them will be reused for code generation by using an 'init()/reset()' strategy.
//! Environment env = ...; // Environment to use, for example from JitRuntime.
//! CodeHolder code;       // CodeHolder to reuse (all allocated memory will be held by it until it's destroyed).
//! x86::Compiler cc;      // Emitter to reuse (for example x86::Compiler).
//!
//! for (size_t i = 0; i < ...; i++) {
//!   // Initialize the CodeHolder first.
//!   code.init(env);
//!   code.attach(&emitter);
//!
//!   [[code generation as usual]]
//!
//!   code.reset();
//! }
//! ```
//!
//! While this approach is good for many use-cases, there is even a faster strategy called reinitialization, which is
//! provided by \ref CodeHolder::reinit(). The idea of reinit is to reinitialize the CodeHolder into a state, which
//! was achieved by initializing it by \ref CodeHolder::init(), by optionally attaching \ref Logger, \ref ErrorHandler,
//! and emitters of any kind. See an example below:
//!
//! ```
//! // All of them will be reused for code generation by using a 'reinit()' strategy.
//! Environment env = ...; // Environment to use, for example from JitRuntime.
//! CodeHolder code;       // CodeHolder to reuse (all allocated memory will be held by it until it's destroyed).
//! x86::Compiler cc;      // Emitter to reuse (for example x86::Compiler).
//!
//! // Initialize the CodeHolder and attach emitters to it (attaching ErrorHandler is advised!)
//! code.init(env);
//! code.attach(&emitter);
//!
//! for (size_t i = 0; i < ...; i++) {
//!   [[code generation as usual]]
//!
//!   // Optionally you can start the loop with 'code.reinit()', but this is cleaner as it wipes out all intermediate
//!   // states of CodeHolder and the attached emitters. It won't detach Logger, ErrorHandler, nor attached emitters.
//!   code.reinit();
//! }
//! ```
//!
//! \note \ref CodeHolder has an ability to attach an \ref ErrorHandler, however, the error handler is not triggered
//! by \ref CodeHolder itself, it's instead propagated to all emitters that attach to it.
class CodeHolder {
public:
  ASMJIT_NONCOPYABLE(CodeHolder)

  //! \name Types
  //! \{

  //! \cond INTERNAL
  struct NamedLabelExtraData : public ZoneHashNode {
    LabelEntry::ExtraData extraData;

    ASMJIT_INLINE_NODEBUG uint32_t labelId() const noexcept { return _customData; }
  };
  //! \endcond

  //! An informative data structure that is filled with some details that happened during \ref relocateToBase().
  struct RelocationSummary {
    //! The number of bytes the final code has been reduced by.
    //!
    //! At the moment this is the same as the number of bytes that the address table was shrunk, because it was
    //! possible to avoid certain entries during relocation - the functions that would be otherwise present were
    //! close enough to avoid them in the .addrtab section.
    size_t codeSizeReduction;
  };

  //! \}

  //! \name Members
  //! \{

  //! Environment information.
  Environment _environment;
  //! CPU features of the target architecture.
  CpuFeatures _cpuFeatures;
  //! Base address or \ref Globals::kNoBaseAddress.
  uint64_t _baseAddress;

  //! Attached `Logger`, used by all consumers.
  Logger* _logger;
  //! Attached `ErrorHandler`.
  ErrorHandler* _errorHandler;

  //! Code zone (used to allocate core structures).
  Zone _zone;
  //! Zone allocator, used to manage internal containers.
  ZoneAllocator _allocator;

  //! First emitter attached to this CodeHolder (double-linked list).
  BaseEmitter* _attachedFirst;
  //! Last emitter attached to this CodeHolder (double-linked list).
  BaseEmitter* _attachedLast;

  //! Section entries.
  ZoneVector<Section*> _sections;
  //! Section entries sorted by section order and then section id.
  ZoneVector<Section*> _sectionsByOrder;

  //! Label entries.
  ZoneVector<LabelEntry> _labelEntries;
  //! Relocation entries.
  ZoneVector<RelocEntry*> _relocations;
  //! Label name -> LabelEntry::ExtraData (only used by labels that have a name and are not anonymous).
  ZoneHash<NamedLabelExtraData> _namedLabels;
  //! Unresolved fixups that are most likely references across sections.
  Fixup* _fixups;
  //! Pool containing \ref Fixup instances for quickly recycling them.
  ZonePool<Fixup> _fixupDataPool;
  //! Count of unresolved fixups of unbound labels (at the end of assembling this should be zero).
  size_t _unresolvedFixupCount;

  //! Text section - always one part of a CodeHolder itself.
  Section _textSection;

  //! Pointer to an address table section (or null if this section doesn't exist).
  Section* _addressTableSection;
  //! Address table entries.
  ZoneTree<AddressTableEntry> _addressTableEntries;

  //! \}

  //! \name Construction & Destruction
  //! \{

  //! Creates an uninitialized CodeHolder (you must init() it before it can be used).
  //!
  //! An optional `temporary` argument can be used to initialize the first block of \ref Zone that the CodeHolder
  //! uses into a temporary memory provided by the user.
  ASMJIT_API explicit CodeHolder(const Support::Temporary* temporary = nullptr) noexcept;

  //! \overload
  ASMJIT_INLINE_NODEBUG explicit CodeHolder(const Support::Temporary& temporary) noexcept
    : CodeHolder(&temporary) {}

  //! Destroys the CodeHolder and frees all resources it has allocated.
  ASMJIT_API ~CodeHolder() noexcept;

  //! Tests whether the `CodeHolder` has been initialized.
  //!
  //! Emitters can be only attached to initialized `CodeHolder` instances.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool isInitialized() const noexcept { return _environment.isInitialized(); }

  //! Initializes CodeHolder to hold code described by the given `environment` and `baseAddress`.
  ASMJIT_API Error init(const Environment& environment, uint64_t baseAddress = Globals::kNoBaseAddress) noexcept;
  //! Initializes CodeHolder to hold code described by the given `environment`, `cpuFeatures`, and `baseAddress`.
  ASMJIT_API Error init(const Environment& environment, const CpuFeatures& cpuFeatures, uint64_t baseAddress = Globals::kNoBaseAddress) noexcept;

  //! Reinitializes CodeHolder with the same environment, cpu features, and base address as it had, and notifies
  //! all attached emitters of reinitialization. If the \ref CodeHolder was not initialized, \ref kErrorNotInitialized
  //! is returned.
  //!
  //! Reinitialization is designed to be a faster alternative compared to \ref reset() followed by \ref init() chain.
  //! The purpose of reinitialization is a very quick reuse of \ref CodeHolder and all attached emitters (most likely
  //! Assembler or Compiler) without paying the cost of complete initialization and then assignment of all the loggers,
  //! error handlers, and emitters.
  //!
  //! \note Semantically reinit() is the same as using \ref reset(ResetPolicy::kSoft), followed by \ref init(), and
  //! then by attaching loggers, error handlers, and emitters that were attached previously. This means that after
  //! reinitialization you will get a clean and ready for use \ref CodeHolder, which was initialized the same way as
  //! before.
  ASMJIT_API Error reinit() noexcept;

  //! Detaches all code-generators attached and resets the `CodeHolder`.
  ASMJIT_API void reset(ResetPolicy resetPolicy = ResetPolicy::kSoft) noexcept;

  //! \}

  //! \name Attach & Detach
  //! \{

  //! Attaches an emitter to this `CodeHolder`.
  ASMJIT_API Error attach(BaseEmitter* emitter) noexcept;
  //! Detaches an emitter from this `CodeHolder`.
  ASMJIT_API Error detach(BaseEmitter* emitter) noexcept;

  //! \}

  //! \name Memory Allocators
  //! \{

  //! Returns the allocator that the `CodeHolder` uses.
  //!
  //! \note This should be only used for AsmJit's purposes. Code holder uses arena allocator to allocate everything,
  //! so anything allocated through this allocator will be invalidated by \ref CodeHolder::reset() or by CodeHolder's
  //! destructor.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG ZoneAllocator* allocator() const noexcept { return const_cast<ZoneAllocator*>(&_allocator); }

  //! \}

  //! \name Code & Architecture
  //! \{

  //! Returns the target environment information.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const Environment& environment() const noexcept { return _environment; }

  //! Returns the target architecture.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Arch arch() const noexcept { return environment().arch(); }

  //! Returns the target sub-architecture.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG SubArch subArch() const noexcept { return environment().subArch(); }

  //! Returns the minimum CPU features of the target architecture.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const CpuFeatures& cpuFeatures() const noexcept { return _cpuFeatures; }

  //! Tests whether a static base-address is set.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool hasBaseAddress() const noexcept { return _baseAddress != Globals::kNoBaseAddress; }

  //! Returns a static base-address or \ref Globals::kNoBaseAddress, if not set.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint64_t baseAddress() const noexcept { return _baseAddress; }

  //! \}

  //! \name Attached Emitters
  //! \{

  //! Returns a vector of attached emitters.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG BaseEmitter* attachedFirst() noexcept { return _attachedFirst; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG BaseEmitter* attachedLast() noexcept { return _attachedLast; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const BaseEmitter* attachedFirst() const noexcept { return _attachedFirst; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const BaseEmitter* attachedLast() const noexcept { return _attachedLast; }

  //! \}

  //! \name Logging
  //! \{

  //! Returns the attached logger.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Logger* logger() const noexcept { return _logger; }

  //! Attaches a `logger` to CodeHolder and propagates it to all attached emitters.
  ASMJIT_API void setLogger(Logger* logger) noexcept;

  //! Resets the logger to none.
  ASMJIT_INLINE_NODEBUG void resetLogger() noexcept { setLogger(nullptr); }

  //! \name Error Handling
  //! \{

  //! Tests whether the CodeHolder has an attached error handler, see \ref ErrorHandler.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool hasErrorHandler() const noexcept { return _errorHandler != nullptr; }

  //! Returns the attached error handler.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG ErrorHandler* errorHandler() const noexcept { return _errorHandler; }

  //! Attach an error handler to this `CodeHolder`.
  ASMJIT_API void setErrorHandler(ErrorHandler* errorHandler) noexcept;

  //! Resets the error handler to none.
  ASMJIT_INLINE_NODEBUG void resetErrorHandler() noexcept { setErrorHandler(nullptr); }

  //! \}

  //! \name Code Buffer
  //! \{

  //! Makes sure that at least `n` bytes can be added to CodeHolder's buffer `cb`.
  //!
  //! \note The buffer `cb` must be managed by `CodeHolder` - otherwise the behavior of the function is undefined.
  ASMJIT_API Error growBuffer(CodeBuffer* cb, size_t n) noexcept;

  //! Reserves the size of `cb` to at least `n` bytes.
  //!
  //! \note The buffer `cb` must be managed by `CodeHolder` - otherwise the behavior of the function is undefined.
  ASMJIT_API Error reserveBuffer(CodeBuffer* cb, size_t n) noexcept;

  //! \}

  //! \name Sections
  //! \{

  //! Returns an array of `Section*` records.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const ZoneVector<Section*>& sections() const noexcept { return _sections; }

  //! Returns an array of `Section*` records sorted according to section order first, then section id.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const ZoneVector<Section*>& sectionsByOrder() const noexcept { return _sectionsByOrder; }

  //! Returns the number of sections.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t sectionCount() const noexcept { return _sections.size(); }

  //! Tests whether the given `sectionId` is valid.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool isSectionValid(uint32_t sectionId) const noexcept { return sectionId < _sections.size(); }

  //! Creates a new section and return its pointer in `sectionOut`.
  //!
  //! Returns `Error`, does not report a possible error to `ErrorHandler`.
  ASMJIT_API Error newSection(Section** sectionOut, const char* name, size_t nameSize = SIZE_MAX, SectionFlags flags = SectionFlags::kNone, uint32_t alignment = 1, int32_t order = 0) noexcept;

  //! Returns a section entry of the given index.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Section* sectionById(uint32_t sectionId) const noexcept { return _sections[sectionId]; }

  //! Returns section-id that matches the given `name`.
  //!
  //! If there is no such section `Section::kInvalidId` is returned.
  [[nodiscard]]
  ASMJIT_API Section* sectionByName(const char* name, size_t nameSize = SIZE_MAX) const noexcept;

  //! Returns '.text' section (section that commonly represents code).
  //!
  //! \note Text section is always the first section in \ref CodeHolder::sections() array.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Section* textSection() const noexcept { return _sections[0]; }

  //! Tests whether '.addrtab' section exists.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool hasAddressTable() const noexcept { return _addressTableSection != nullptr; }

  //! Returns '.addrtab' section.
  //!
  //! This section is used exclusively by AsmJit to store absolute 64-bit
  //! addresses that cannot be encoded in instructions like 'jmp' or 'call'.
  //!
  //! \note This section is created on demand, the returned pointer can be null.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Section* addressTableSection() const noexcept { return _addressTableSection; }

  //! Ensures that '.addrtab' section exists (creates it if it doesn't) and
  //! returns it. Can return `nullptr` on out of memory condition.
  [[nodiscard]]
  ASMJIT_API Section* ensureAddressTableSection() noexcept;

  //! Used to add an address to an address table.
  //!
  //! This implicitly calls `ensureAddressTableSection()` and then creates `AddressTableEntry` that is inserted
  //! to `_addressTableEntries`. If the address already exists this operation does nothing as the same addresses
  //! use the same slot.
  //!
  //! This function should be considered internal as it's used by assemblers to insert an absolute address into the
  //! address table. Inserting address into address table without creating a particular relocation entry makes no sense.
  ASMJIT_API Error addAddressToAddressTable(uint64_t address) noexcept;

  //! \}

  //! \name Labels & Symbols
  //! \{

  //! Returns array of `LabelEntry` records.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const ZoneVector<LabelEntry>& labelEntries() const noexcept { return _labelEntries; }

  //! Returns number of labels created.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t labelCount() const noexcept { return _labelEntries.size(); }

  //! Tests whether the label having `labelId` is valid (i.e. created by `newLabelId()`).
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool isLabelValid(uint32_t labelId) const noexcept {
    return labelId < _labelEntries.size();
  }

  //! Tests whether the `label` is valid (i.e. created by `newLabelId()`).
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool isLabelValid(const Label& label) const noexcept {
    return isLabelValid(label.id());
  }

  //! Tests whether a label having `labelId` is already bound.
  //!
  //! Returns `false` if the `labelId` is not valid.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool isLabelBound(uint32_t labelId) const noexcept {
    return isLabelValid(labelId) && _labelEntries[labelId].isBound();
  }

  //! Tests whether the `label` is already bound.
  //!
  //! Returns `false` if the `label` is not valid.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool isLabelBound(const Label& label) const noexcept {
    return isLabelBound(label.id());
  }

  //! Returns LabelEntry of the given label identifier `labelId` (or `label` if you are using overloads).
  //!
  //! \attention The passed `labelId` must be valid as it's used as an index to `_labelEntries[]` array. In debug
  //! builds the array access uses an assertion, but such assertion is not present in release builds. To get whether
  //! a label is valid, check out \ref CodeHolder::isLabelValid() function.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG LabelEntry& labelEntry(uint32_t labelId) noexcept {
    return _labelEntries[labelId];
  }

  //! \overload
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const LabelEntry& labelEntry(uint32_t labelId) const noexcept {
    return _labelEntries[labelId];
  }

  //! \overload
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG LabelEntry& labelEntry(const Label& label) noexcept {
    return labelEntry(label.id());
  }

  //! \overload
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const LabelEntry& labelEntry(const Label& label) const noexcept {
    return labelEntry(label.id());
  }

  //! Returns offset of a `Label` by its `labelId`.
  //!
  //! The offset returned is relative to the start of the section where the label is bound. Zero offset is returned
  //! for unbound labels, which is their initial offset value.
  //!
  //! \attention The passed `labelId` must be valid as it's used as an index to `_labelEntries[]` array. In debug
  //! builds the array access uses an assertion, but such assertion is not present in release builds. To get whether
  //! a label is valid, check out \ref CodeHolder::isLabelValid() function.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint64_t labelOffset(uint32_t labelId) const noexcept {
    ASMJIT_ASSERT(isLabelValid(labelId));
    return _labelEntries[labelId].offset();
  }

  //! \overload
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint64_t labelOffset(const Label& label) const noexcept {
    return labelOffset(label.id());
  }

  //! Returns offset of a label by it's `labelId` relative to the base offset.
  //!
  //! \attention The passed `labelId` must be valid as it's used as an index to `_labelEntries[]` array. In debug
  //! builds the array access uses an assertion, but such assertion is not present in release builds. To get whether
  //! a label is valid, check out \ref CodeHolder::isLabelValid() function.
  //!
  //! \note The offset of the section where the label is bound must be valid in order to use this function, otherwise
  //! the value returned will not be reliable. Typically, sections have offsets when they are flattened, see \ref
  //! CodeHolder::flatten() function for more details.
  [[nodiscard]]
  inline uint64_t labelOffsetFromBase(uint32_t labelId) const noexcept {
    ASMJIT_ASSERT(isLabelValid(labelId));

    const LabelEntry& le = _labelEntries[labelId];
    return (le.isBound() ? _sections[le.sectionId()]->offset() : uint64_t(0)) + le.offset();
  }

  //! \overload
  [[nodiscard]]
  inline uint64_t labelOffsetFromBase(const Label& label) const noexcept {
    return labelOffsetFromBase(label.id());
  }

  //! Creates a new anonymous label and return its id in `labelIdOut`.
  //!
  //! Returns `Error`, does not report error to `ErrorHandler`.
  [[nodiscard]]
  ASMJIT_API Error newLabelId(uint32_t* labelIdOut) noexcept;

  //! Creates a new named \ref LabelEntry of the given label `type`.
  //!
  //! \param entryOut Where to store the created \ref LabelEntry.
  //! \param name The name of the label.
  //! \param nameSize The length of `name` argument, or `SIZE_MAX` if `name` is a null terminated string, which
  //!        means that the `CodeHolder` will use `strlen()` to determine the length.
  //! \param type The type of the label to create, see \ref LabelType.
  //! \param parentId Parent id of a local label, otherwise it must be \ref Globals::kInvalidId.
  //! \retval Always returns \ref Error, does not report a possible error to the attached \ref ErrorHandler.
  //!
  //! AsmJit has a support for local labels (\ref LabelType::kLocal) which require a parent label id (parentId).
  //! The names of local labels can conflict with names of other local labels that have a different parent. In
  //! addition, AsmJit supports named anonymous labels, which are useful only for debugging purposes as the
  //! anonymous name will have a name, which will be formatted, but the label itself cannot be queried by such
  //! name.
  [[nodiscard]]
  ASMJIT_API Error newNamedLabelId(uint32_t* labelIdOut, const char* name, size_t nameSize, LabelType type, uint32_t parentId = Globals::kInvalidId) noexcept;

  //! Returns a label by name.
  //!
  //! If the named label doesn't a default constructed \ref Label is returned,
  //! which has its id set to \ref Globals::kInvalidId.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Label labelByName(const char* name, size_t nameSize = SIZE_MAX, uint32_t parentId = Globals::kInvalidId) noexcept {
    return Label(labelIdByName(name, nameSize, parentId));
  }

  //! Returns a label id by name.
  //!
  //! If the named label doesn't exist \ref Globals::kInvalidId is returned.
  [[nodiscard]]
  ASMJIT_API uint32_t labelIdByName(const char* name, size_t nameSize = SIZE_MAX, uint32_t parentId = Globals::kInvalidId) noexcept;

  //! Tests whether there are any unresolved fixups related to unbound labels.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool hasUnresolvedFixups() const noexcept { return _unresolvedFixupCount != 0u; }

  //! Returns the number of unresolved fixups.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t unresolvedFixupCount() const noexcept { return _unresolvedFixupCount; }

  //! Creates a new label-link used to store information about yet unbound labels.
  //!
  //! Returns `null` if the allocation failed.
  [[nodiscard]]
  ASMJIT_API Fixup* newFixup(LabelEntry& le, uint32_t sectionId, size_t offset, intptr_t rel, const OffsetFormat& format) noexcept;

  //! Resolves cross-section fixups associated with each label that was used as a destination in code of a different
  //! section. It's only useful to people that use multiple sections as it will do nothing if the code only contains
  //! a single section in which cross-section fixups are not possible.
  ASMJIT_API Error resolveCrossSectionFixups() noexcept;

  //! Binds a label to a given `sectionId` and `offset` (relative to start of the section).
  //!
  //! This function is generally used by `BaseAssembler::bind()` to do the heavy lifting.
  ASMJIT_API Error bindLabel(const Label& label, uint32_t sectionId, uint64_t offset) noexcept;

  //! \}

  //! \name Relocations
  //! \{

  //! Tests whether the code contains relocation entries.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool hasRelocEntries() const noexcept { return !_relocations.empty(); }

  //! Returns array of `RelocEntry*` records.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const ZoneVector<RelocEntry*>& relocEntries() const noexcept { return _relocations; }

  //! Returns a RelocEntry of the given `id`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RelocEntry* relocEntry(uint32_t id) const noexcept { return _relocations[id]; }

  //! Creates a new relocation entry of type `relocType`.
  //!
  //! Additional fields can be set after the relocation entry was created.
  [[nodiscard]]
  ASMJIT_API Error newRelocEntry(RelocEntry** dst, RelocType relocType) noexcept;

  //! \}

  //! \name Utilities
  //! \{

  //! Flattens all sections by recalculating their offsets, starting at 0.
  //!
  //! \note This should never be called more than once.
  ASMJIT_API Error flatten() noexcept;

  //! Returns computed the size of code & data of all sections.
  //!
  //! \note All sections will be iterated over and the code size returned would represent the minimum code size of
  //! all combined sections after applying minimum alignment. Code size may decrease after calling `flatten()` and
  //! `relocateToBase()`.
  [[nodiscard]]
  ASMJIT_API size_t codeSize() const noexcept;

  //! Relocates the code to the given `baseAddress`.
  //!
  //! \param baseAddress Absolute base address where the code will be relocated to. Please note that nothing is
  //! copied to such base address, it's just an absolute value used by the relocation code to resolve all stored
  //! relocations.
  //!
  //! \note This should never be called more than once.
  ASMJIT_API Error relocateToBase(uint64_t baseAddress, RelocationSummary* summaryOut = nullptr) noexcept;

  //! Copies a single section into `dst`.
  ASMJIT_API Error copySectionData(void* dst, size_t dstSize, uint32_t sectionId, CopySectionFlags copyFlags = CopySectionFlags::kNone) noexcept;

  //! Copies all sections into `dst`.
  //!
  //! This should only be used if the data was flattened and there are no gaps between the sections. The `dstSize`
  //! is always checked and the copy will never write anything outside the provided buffer.
  ASMJIT_API Error copyFlattenedData(void* dst, size_t dstSize, CopySectionFlags copyFlags = CopySectionFlags::kNone) noexcept;

  //! \}
};

//! \}

ASMJIT_END_NAMESPACE

#endif // ASMJIT_CORE_CODEHOLDER_H_INCLUDED
