// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include "../core/api-build_p.h"
#include "../core/assembler.h"
#include "../core/codewriter_p.h"
#include "../core/logger.h"
#include "../core/support.h"

#include <algorithm>
#include <tuple>

ASMJIT_BEGIN_NAMESPACE

// CodeHolder - X86 Utilities
// ==========================

//! Encodes a MOD byte.
static inline uint32_t x86EncodeMod(uint32_t m, uint32_t o, uint32_t rm) noexcept {
  return (m << 6) | (o << 3) | rm;
}

// CodeHolder - LabelEntry Globals & Utilities
// ===========================================

static constexpr LabelEntry::ExtraData _makeSharedLabelExtraData() noexcept {
  LabelEntry::ExtraData extraData {};
  extraData._sectionId = Globals::kInvalidId;
  extraData._parentId = Globals::kInvalidId;
  return extraData;
}

static constexpr LabelEntry::ExtraData CodeHolder_sharedLabelExtraData = _makeSharedLabelExtraData();

class ResolveFixupIterator {
public:
  Fixup* _fixup {};
  Fixup** _pPrev {};
  size_t _resolvedCount {};
  size_t _unresolvedCount {};

  ASMJIT_INLINE_NODEBUG explicit ResolveFixupIterator(Fixup** ppFixup) noexcept { reset(ppFixup); }
  ASMJIT_INLINE_NODEBUG bool isValid() const noexcept { return _fixup != nullptr; }
  ASMJIT_INLINE_NODEBUG Fixup* fixup() const noexcept { return _fixup; }

  ASMJIT_INLINE void reset(Fixup** ppFixup) noexcept {
    _pPrev = ppFixup;
    _fixup = *_pPrev;
  }

  ASMJIT_INLINE void next() noexcept {
    _pPrev = &_fixup->next;
    _fixup = *_pPrev;
    _unresolvedCount++;
  }

  ASMJIT_INLINE void resolveAndNext(CodeHolder* code) noexcept {
    Fixup* fixupToDelete = _fixup;

    _fixup = _fixup->next;
    *_pPrev = _fixup;

    _resolvedCount++;
    code->_fixupDataPool.release(fixupToDelete);
  }

  ASMJIT_INLINE_NODEBUG size_t resolvedCount() const noexcept { return _resolvedCount; }
  ASMJIT_INLINE_NODEBUG size_t unresolvedCount() const noexcept { return _unresolvedCount; }
};

// CodeHolder - Section Globals & Utilities
// ========================================

static const char CodeHolder_addrTabName[] = ".addrtab";

static ASMJIT_INLINE void Section_initName(
  Section* section,
  char c0 = 0, char c1 = 0, char c2 = 0, char c3 = 0,
  char c4 = 0, char c5 = 0, char c6 = 0, char c7 = 0) noexcept {

  section->_name.u32[0] = Support::bytepack32_4x8(uint8_t(c0), uint8_t(c1), uint8_t(c2), uint8_t(c3));
  section->_name.u32[1] = Support::bytepack32_4x8(uint8_t(c4), uint8_t(c5), uint8_t(c6), uint8_t(c7));
  section->_name.u32[2] = 0u;
  section->_name.u32[3] = 0u;
}

static ASMJIT_INLINE void Section_initData(Section* section, uint32_t sectionId, SectionFlags flags, uint32_t alignment, int order) noexcept {
  section->_sectionId = sectionId;

  // These two fields are not used by sections (see \ref LabelEntry for more details about why).
  section->_internalLabelType = LabelType::kAnonymous;
  section->_internalLabelFlags = LabelFlags::kNone;

  section->assignFlags(flags);
  section->_alignment = alignment;
  section->_order = order;
  section->_offset = 0;
  section->_virtualSize = 0;
}

static ASMJIT_INLINE void Section_initBuffer(Section* section) noexcept {
  section->_buffer = CodeBuffer{};
}

static ASMJIT_INLINE void Section_releaseBuffer(Section* section) noexcept {
  if (Support::bool_and(section->_buffer.data() != nullptr, !section->_buffer.isExternal())) {
    ::free(section->_buffer._data);
  }
}

// CodeHolder - Utilities
// ======================

static ASMJIT_INLINE Error CodeHolder_initSectionStorage(CodeHolder* self) noexcept {
  Error err1 = self->_sections.willGrow(&self->_allocator);
  Error err2 = self->_sectionsByOrder.willGrow(&self->_allocator);

  return err1 | err2;
}

static ASMJIT_INLINE void CodeHolder_addTextSection(CodeHolder* self) noexcept {
  Section* textSection = &self->_textSection;

  Section_initData(textSection, 0u, SectionFlags::kExecutable | SectionFlags::kReadOnly | SectionFlags::kBuiltIn, 0u, 0);
  Section_initName(textSection, '.', 't', 'e', 'x', 't');

  self->_sections.appendUnsafe(textSection);
  self->_sectionsByOrder.appendUnsafe(textSection);
}

static ASMJIT_NOINLINE void CodeHolder_detachEmitters(CodeHolder* self) noexcept {
  BaseEmitter* emitter = self->_attachedFirst;

  while (emitter) {
    BaseEmitter* next = emitter->_attachedNext;

    emitter->_attachedPrev = nullptr;
    (void)emitter->onDetach(*self);
    emitter->_attachedNext = nullptr;
    emitter->_code = nullptr;

    emitter = next;
    self->_attachedFirst = next;
  }

  self->_attachedLast = nullptr;
}

static ASMJIT_INLINE void CodeHolder_resetEnvAndAttachedLogAndEH(CodeHolder* self) noexcept {
  self->_environment.reset();
  self->_cpuFeatures.reset();
  self->_baseAddress = Globals::kNoBaseAddress;
  self->_logger = nullptr;
  self->_errorHandler = nullptr;
}

// Reset zone allocator and all containers using it.
static ASMJIT_INLINE void CodeHolder_resetSections(CodeHolder* self, ResetPolicy resetPolicy) noexcept {
  // Reset all sections except the first one (.text section).
  uint32_t fromSection = resetPolicy == ResetPolicy::kHard ? 0u : 1u;
  uint32_t sectionCount = self->_sections.size();

  for (uint32_t i = fromSection; i < sectionCount; i++) {
    Section* section = self->_sections[i];

    Section_releaseBuffer(section);
    section->_buffer._data = nullptr;
    section->_buffer._capacity = 0;
  }
}

// Reset zone allocator and all containers using it.
static ASMJIT_INLINE void CodeHolder_resetContainers(CodeHolder* self, ResetPolicy resetPolicy) noexcept {
  // Soft reset won't wipe out the .text section, so set its size to 0 for future reuse.
  self->_textSection._buffer._size = 0;

  ZoneAllocator* allocator = self->allocator();

  self->_namedLabels.reset();
  self->_relocations.reset();
  self->_labelEntries.reset();

  self->_fixups = nullptr;
  self->_fixupDataPool.reset();
  self->_unresolvedFixupCount = 0;

  self->_sections.reset();
  self->_sectionsByOrder.reset();

  self->_addressTableSection = nullptr;
  self->_addressTableEntries.reset();

  allocator->reset(&self->_zone);
  self->_zone.reset(resetPolicy);
}

// Reset zone allocator and all containers using it.
static ASMJIT_NOINLINE void CodeHolder_resetSectionsAndContainers(CodeHolder* self, ResetPolicy resetPolicy) noexcept {
  CodeHolder_resetSections(self, resetPolicy);
  CodeHolder_resetContainers(self, resetPolicy);
}

static ASMJIT_INLINE void CodeHolder_onSettingsUpdated(CodeHolder* self) noexcept {
  // Notify all attached emitters about a settings update.
  BaseEmitter* emitter = self->_attachedFirst;
  while (emitter) {
    emitter->onSettingsUpdated();
    emitter = emitter->_attachedNext;
  }
}

// CodeHolder - Construction & Destruction
// =======================================

CodeHolder::CodeHolder(const Support::Temporary* temporary) noexcept
  : _environment(),
    _cpuFeatures{},
    _baseAddress(Globals::kNoBaseAddress),
    _logger(nullptr),
    _errorHandler(nullptr),
    _zone(16u * 1024u, temporary),
    _allocator(&_zone),
    _attachedFirst(nullptr),
    _attachedLast(nullptr),
    _fixups(nullptr),
    _unresolvedFixupCount(0),
    _textSection{},
    _addressTableSection(nullptr) {}

CodeHolder::~CodeHolder() noexcept {
  if (isInitialized()) {
    CodeHolder_detachEmitters(this);
    CodeHolder_resetSections(this, ResetPolicy::kHard);
  }
  else {
    Section_releaseBuffer(&_textSection);
  }
}

// CodeHolder - Initialization & Reset
// ===================================

Error CodeHolder::init(const Environment& environment, uint64_t baseAddress) noexcept {
  return init(environment, CpuFeatures{}, baseAddress);
}

Error CodeHolder::init(const Environment& environment, const CpuFeatures& cpuFeatures, uint64_t baseAddress) noexcept {
  // Cannot initialize if it's already initialized or the environment passed is invalid.
  if (ASMJIT_UNLIKELY(Support::bool_or(isInitialized(), !environment.isInitialized()))) {
    Error err = isInitialized() ? kErrorAlreadyInitialized : kErrorInvalidArgument;
    return DebugUtils::errored(err);
  }

  // If we are just initializing there should be no emitters attached.
  ASMJIT_ASSERT(_attachedFirst == nullptr);
  ASMJIT_ASSERT(_attachedLast == nullptr);

  // Create a default section and insert it to the `_sections` array.
  Error err = CodeHolder_initSectionStorage(this);
  if (ASMJIT_UNLIKELY(err)) {
    _zone.reset();
    return DebugUtils::errored(kErrorOutOfMemory);
  }

  _environment = environment;
  _cpuFeatures = cpuFeatures;
  _baseAddress = baseAddress;

  CodeHolder_addTextSection(this);
  return kErrorOk;
}

Error CodeHolder::reinit() noexcept {
  // Cannot reinitialize if it's not initialized.
  if (ASMJIT_UNLIKELY(!isInitialized())) {
    return DebugUtils::errored(kErrorNotInitialized);
  }

  CodeHolder_resetSectionsAndContainers(this, ResetPolicy::kSoft);

  // Create a default section and insert it to the `_sections` array.
  (void)CodeHolder_initSectionStorage(this);
  CodeHolder_addTextSection(this);

  BaseEmitter* emitter = _attachedFirst;
  while (emitter) {
    emitter->onReinit(*this);
    emitter = emitter->_attachedNext;
  }

  return kErrorOk;
}

void CodeHolder::reset(ResetPolicy resetPolicy) noexcept {
  if (isInitialized()) {
    CodeHolder_detachEmitters(this);
    CodeHolder_resetEnvAndAttachedLogAndEH(this);
    CodeHolder_resetSectionsAndContainers(this, resetPolicy);
  }
}

// CodeHolder - Attach / Detach
// ============================

Error CodeHolder::attach(BaseEmitter* emitter) noexcept {
  // Catch a possible misuse of the API.
  if (ASMJIT_UNLIKELY(!emitter)) {
    return DebugUtils::errored(kErrorInvalidArgument);
  }

  // Invalid emitter, this should not be possible.
  EmitterType type = emitter->emitterType();
  if (ASMJIT_UNLIKELY(type == EmitterType::kNone || uint32_t(type) > uint32_t(EmitterType::kMaxValue))) {
    return DebugUtils::errored(kErrorInvalidState);
  }

  uint64_t archMask = emitter->_archMask;
  if (ASMJIT_UNLIKELY(!(archMask & (uint64_t(1) << uint32_t(arch()))))) {
    return DebugUtils::errored(kErrorInvalidArch);
  }

  // This is suspicious, but don't fail if `emitter` is already attached
  // to this code holder. This is not error, but it's not recommended.
  if (emitter->_code != nullptr) {
    if (emitter->_code == this) {
      return kErrorOk;
    }
    return DebugUtils::errored(kErrorInvalidState);
  }

  // Reserve the space now as we cannot fail after `onAttach()` succeeded.
  ASMJIT_PROPAGATE(emitter->onAttach(*this));

  // Make sure CodeHolder <-> BaseEmitter are connected.
  ASMJIT_ASSERT(emitter->_code == this);

  // Add `emitter` to a double linked-list.
  {
    BaseEmitter* last = _attachedLast;

    emitter->_attachedPrev = last;
    _attachedLast = emitter;

    if (last) {
      last->_attachedNext = emitter;
    }
    else {
      _attachedFirst = emitter;
    }
  }

  return kErrorOk;
}

Error CodeHolder::detach(BaseEmitter* emitter) noexcept {
  if (ASMJIT_UNLIKELY(!emitter)) {
    return DebugUtils::errored(kErrorInvalidArgument);
  }

  if (ASMJIT_UNLIKELY(emitter->_code != this)) {
    return DebugUtils::errored(kErrorInvalidState);
  }

  // NOTE: We always detach if we were asked to, if error happens during
  // `emitter->onDetach()` we just propagate it, but the BaseEmitter will
  // be detached.
  Error err = kErrorOk;
  if (!emitter->isDestroyed()) {
    err = emitter->onDetach(*this);
  }

  // Remove `emitter` from a double linked-list.
  {
    BaseEmitter* prev = emitter->_attachedPrev;
    BaseEmitter* next = emitter->_attachedNext;

    if (prev) { prev->_attachedNext = next; } else { _attachedFirst = next; }
    if (next) { next->_attachedPrev = prev; } else { _attachedLast = prev; }

    emitter->_code = nullptr;
    emitter->_attachedPrev = nullptr;
    emitter->_attachedNext = nullptr;
  }

  return err;
}

// CodeHolder - Logging
// ====================

void CodeHolder::setLogger(Logger* logger) noexcept {
#ifndef ASMJIT_NO_LOGGING
  _logger = logger;
  CodeHolder_onSettingsUpdated(this);
#else
  DebugUtils::unused(logger);
#endif
}

// CodeHolder - Error Handling
// ===========================

void CodeHolder::setErrorHandler(ErrorHandler* errorHandler) noexcept {
  _errorHandler = errorHandler;
  CodeHolder_onSettingsUpdated(this);
}

// CodeHolder - Code Buffer
// ========================

static Error CodeHolder_reserveInternal(CodeHolder* self, CodeBuffer* cb, size_t n) noexcept {
  uint8_t* oldData = cb->_data;
  uint8_t* newData;

  if (oldData && !cb->isExternal()) {
    newData = static_cast<uint8_t*>(::realloc(oldData, n));
  }
  else {
    newData = static_cast<uint8_t*>(::malloc(n));
  }

  if (ASMJIT_UNLIKELY(!newData)) {
    return DebugUtils::errored(kErrorOutOfMemory);
  }

  cb->_data = newData;
  cb->_capacity = n;

  // Update pointers used by assemblers, if attached.
  BaseEmitter* emitter = self->_attachedFirst;
  while (emitter) {
    if (emitter->isAssembler()) {
      BaseAssembler* a = static_cast<BaseAssembler*>(emitter);
      if (&a->_section->_buffer == cb) {
        size_t offset = a->offset();

        a->_bufferData = newData;
        a->_bufferEnd  = newData + n;
        a->_bufferPtr  = newData + offset;
      }
    }
    emitter = emitter->_attachedNext;
  }

  return kErrorOk;
}

Error CodeHolder::growBuffer(CodeBuffer* cb, size_t n) noexcept {
  // The size of the section must be valid.
  size_t size = cb->size();
  if (ASMJIT_UNLIKELY(n > std::numeric_limits<uintptr_t>::max() - size)) {
    return DebugUtils::errored(kErrorOutOfMemory);
  }

  // We can now check if growing the buffer is really necessary. It's unlikely
  // that this function is called while there is still room for `n` bytes.
  size_t capacity = cb->capacity();
  size_t required = cb->size() + n;

  if (ASMJIT_UNLIKELY(required <= capacity)) {
    return kErrorOk;
  }

  if (cb->isFixed()) {
    return DebugUtils::errored(kErrorTooLarge);
  }

  size_t kInitialCapacity = 8192u - Globals::kAllocOverhead;
  if (capacity < kInitialCapacity) {
    capacity = kInitialCapacity;
  }
  else {
    capacity += Globals::kAllocOverhead;
  }

  do {
    size_t old = capacity;
    size_t capacityIncrease = capacity < Globals::kGrowThreshold ? capacity : Globals::kGrowThreshold;

    capacity += capacityIncrease;

    // Overflow.
    if (ASMJIT_UNLIKELY(old > capacity)) {
      return DebugUtils::errored(kErrorOutOfMemory);
    }
  } while (capacity - Globals::kAllocOverhead < required);

  return CodeHolder_reserveInternal(this, cb, capacity - Globals::kAllocOverhead);
}

Error CodeHolder::reserveBuffer(CodeBuffer* cb, size_t n) noexcept {
  size_t capacity = cb->capacity();

  if (n <= capacity) {
    return kErrorOk;
  }

  if (cb->isFixed()) {
    return DebugUtils::errored(kErrorTooLarge);
  }

  return CodeHolder_reserveInternal(this, cb, n);
}

// CodeHolder - Sections
// =====================

Error CodeHolder::newSection(Section** sectionOut, const char* name, size_t nameSize, SectionFlags flags, uint32_t alignment, int32_t order) noexcept {
  *sectionOut = nullptr;


  if (ASMJIT_UNLIKELY(!Support::isZeroOrPowerOf2(alignment))) {
    return DebugUtils::errored(kErrorInvalidArgument);
  }

  if (nameSize == SIZE_MAX) {
    nameSize = strlen(name);
  }

  if (ASMJIT_UNLIKELY(nameSize > Globals::kMaxSectionNameSize)) {
    return DebugUtils::errored(kErrorInvalidSectionName);
  }

  uint32_t sectionId = _sections.size();
  if (ASMJIT_UNLIKELY(sectionId == Globals::kInvalidId)) {
    return DebugUtils::errored(kErrorTooManySections);
  }

  ASMJIT_PROPAGATE(_sections.willGrow(&_allocator));
  ASMJIT_PROPAGATE(_sectionsByOrder.willGrow(&_allocator));

  Section* section = _zone.alloc<Section>();
  if (ASMJIT_UNLIKELY(!section)) {
    return DebugUtils::errored(kErrorOutOfMemory);
  }

  if (alignment == 0u) {
    alignment = 1u;
  }

  Section_initData(section, sectionId, flags, alignment, order);
  Section_initBuffer(section);
  memcpy(section->_name.str, name, nameSize);

  Section** insertPosition = std::lower_bound(_sectionsByOrder.begin(), _sectionsByOrder.end(), section, [](const Section* a, const Section* b) {
    return std::make_tuple(a->order(), a->sectionId()) < std::make_tuple(b->order(), b->sectionId());
  });

  _sections.appendUnsafe(section);
  _sectionsByOrder.insertUnsafe((size_t)(insertPosition - _sectionsByOrder.data()), section);

  *sectionOut = section;
  return kErrorOk;
}

Section* CodeHolder::sectionByName(const char* name, size_t nameSize) const noexcept {
  if (nameSize == SIZE_MAX) {
    nameSize = strlen(name);
  }

  // This could be also put in a hash-table similarly like we do with labels, however it's questionable as
  // the number of sections should be pretty low in general. Create an issue if this becomes a problem.
  if (nameSize <= Globals::kMaxSectionNameSize) {
    for (Section* section : _sections) {
      if (memcmp(section->_name.str, name, nameSize) == 0 && section->_name.str[nameSize] == '\0') {
        return section;
      }
    }
  }

  return nullptr;
}

Section* CodeHolder::ensureAddressTableSection() noexcept {
  if (_addressTableSection) {
    return _addressTableSection;
  }

  newSection(&_addressTableSection,
             CodeHolder_addrTabName,
             sizeof(CodeHolder_addrTabName) - 1,
             SectionFlags::kNone,
             _environment.registerSize(),
             std::numeric_limits<int32_t>::max());
  return _addressTableSection;
}

Error CodeHolder::addAddressToAddressTable(uint64_t address) noexcept {
  AddressTableEntry* entry = _addressTableEntries.get(address);
  if (entry) {
    return kErrorOk;
  }

  Section* section = ensureAddressTableSection();
  if (ASMJIT_UNLIKELY(!section)) {
    return DebugUtils::errored(kErrorOutOfMemory);
  }

  entry = _zone.newT<AddressTableEntry>(address);
  if (ASMJIT_UNLIKELY(!entry)) {
    return DebugUtils::errored(kErrorOutOfMemory);
  }

  _addressTableEntries.insert(entry);
  section->_virtualSize += _environment.registerSize();

  return kErrorOk;
}

// CodeHolder - Labels & Symbols
// =============================

//! Only used to lookup a label from `_namedLabels`.
class LabelByName {
public:
  const char* _key {};
  uint32_t _keySize {};
  uint32_t _hashCode {};
  uint32_t _parentId {};

  inline LabelByName(const char* key, size_t keySize, uint32_t hashCode, uint32_t parentId) noexcept
    : _key(key),
      _keySize(uint32_t(keySize)),
      _hashCode(hashCode),
      _parentId(parentId) {}

  [[nodiscard]]
  inline uint32_t hashCode() const noexcept { return _hashCode; }

  [[nodiscard]]
  inline bool matches(const CodeHolder::NamedLabelExtraData* node) const noexcept {
    return Support::bool_and(node->extraData._nameSize == _keySize,
                             node->extraData._parentId == _parentId) &&
           ::memcmp(node->extraData.name(), _key, _keySize) == 0;
  }
};

// Returns a hash of `name` and fixes `nameSize` if it's `SIZE_MAX`.
static uint32_t CodeHolder_hashNameAndGetSize(const char* name, size_t& nameSize) noexcept {
  uint32_t hashCode = 0;
  if (nameSize == SIZE_MAX) {
    size_t i = 0;
    for (;;) {
      uint8_t c = uint8_t(name[i]);
      if (!c) {
        break;
      }
      hashCode = Support::hashRound(hashCode, c);
      i++;
    }
    nameSize = i;
  }
  else {
    for (size_t i = 0; i < nameSize; i++) {
      uint8_t c = uint8_t(name[i]);
      if (ASMJIT_UNLIKELY(!c)) {
        return DebugUtils::errored(kErrorInvalidLabelName);
      }
      hashCode = Support::hashRound(hashCode, c);
    }
  }
  return hashCode;
}

Fixup* CodeHolder::newFixup(LabelEntry& le, uint32_t sectionId, size_t offset, intptr_t rel, const OffsetFormat& format) noexcept {
  // Cannot be bound if we are creating a link.
  ASMJIT_ASSERT(!le.isBound());

  Fixup* link = _fixupDataPool.alloc(_zone);
  if (ASMJIT_UNLIKELY(!link)) {
    return nullptr;
  }

  link->next = le._getFixups();
  link->sectionId = sectionId;
  link->labelOrRelocId = Globals::kInvalidId;
  link->offset = offset;
  link->rel = rel;
  link->format = format;

  le._setFixups(link);
  _unresolvedFixupCount++;

  return link;
}

Error CodeHolder::newLabelId(uint32_t* labelIdOut) noexcept {
  uint32_t labelId = _labelEntries.size();
  Error err = _labelEntries.willGrow(&_allocator);

  if (ASMJIT_UNLIKELY(err != kErrorOk)) {
    *labelIdOut = Globals::kInvalidId;
    return err;
  }
  else {
    *labelIdOut = labelId;
    _labelEntries.appendUnsafe(LabelEntry{const_cast<LabelEntry::ExtraData*>(&CodeHolder_sharedLabelExtraData), uint64_t(0)});
    return kErrorOk;
  }
}

Error CodeHolder::newNamedLabelId(uint32_t* labelIdOut, const char* name, size_t nameSize, LabelType type, uint32_t parentId) noexcept {
  uint32_t labelId = _labelEntries.size();
  uint32_t hashCode = CodeHolder_hashNameAndGetSize(name, nameSize);

  *labelIdOut = Globals::kInvalidId;
  ASMJIT_PROPAGATE(_labelEntries.willGrow(&_allocator));

  if (nameSize == 0) {
    if (type != LabelType::kAnonymous) {
      return DebugUtils::errored(kErrorInvalidLabelName);
    }

    *labelIdOut = labelId;
    _labelEntries.appendUnsafe(LabelEntry{const_cast<LabelEntry::ExtraData*>(&CodeHolder_sharedLabelExtraData), uint64_t(0)});
    return kErrorOk;
  }

  if (ASMJIT_UNLIKELY(nameSize > Globals::kMaxLabelNameSize)) {
    return DebugUtils::errored(kErrorLabelNameTooLong);
  }

  size_t extraDataSize = sizeof(LabelEntry::ExtraData) + nameSize + 1u;

  switch (type) {
    case LabelType::kAnonymous: {
      // Anonymous labels cannot have a parent (or more specifically, parent is useless here).
      if (ASMJIT_UNLIKELY(parentId != Globals::kInvalidId)) {
        return DebugUtils::errored(kErrorInvalidParentLabel);
      }

      LabelEntry::ExtraData* extraData = _zone.alloc<LabelEntry::ExtraData>(Support::alignUp(extraDataSize, Globals::kZoneAlignment));
      if (ASMJIT_UNLIKELY(!extraData)) {
        return DebugUtils::errored(kErrorOutOfMemory);
      }

      char* namePtr = reinterpret_cast<char*>(extraData) + sizeof(LabelEntry::ExtraData);
      extraData->_sectionId = Globals::kInvalidId;
      extraData->_internalLabelType = type;
      extraData->_internalLabelFlags = LabelFlags::kHasOwnExtraData | LabelFlags::kHasName;
      extraData->_internalUInt16Data = 0;
      extraData->_parentId = Globals::kInvalidId;
      extraData->_nameSize = uint32_t(nameSize);
      memcpy(namePtr, name, nameSize);
      namePtr[nameSize] = '\0';

      *labelIdOut = labelId;
      _labelEntries.appendUnsafe(LabelEntry{extraData, uint64_t(0)});
      return kErrorOk;
    }

    case LabelType::kLocal: {
      if (ASMJIT_UNLIKELY(parentId >= _labelEntries.size())) {
        return DebugUtils::errored(kErrorInvalidParentLabel);
      }

      hashCode ^= parentId;
      break;
    }

    case LabelType::kGlobal:
    case LabelType::kExternal: {
      if (ASMJIT_UNLIKELY(parentId != Globals::kInvalidId)) {
        return DebugUtils::errored(kErrorInvalidParentLabel);
      }
      break;
    }

    default: {
      return DebugUtils::errored(kErrorInvalidArgument);
    }
  }

  extraDataSize += sizeof(ZoneHashNode);

  // Don't allow to insert duplicates. Local labels allow duplicates that have different ids, however, this is
  // already accomplished by having a different hashes between the same label names having different parent labels.
  NamedLabelExtraData* namedNode = _namedLabels.get(LabelByName(name, nameSize, hashCode, parentId));
  if (ASMJIT_UNLIKELY(namedNode)) {
    return DebugUtils::errored(kErrorLabelAlreadyDefined);
  }

  namedNode = _zone.alloc<NamedLabelExtraData>(Support::alignUp(extraDataSize, Globals::kZoneAlignment));
  if (ASMJIT_UNLIKELY(!namedNode)) {
    return DebugUtils::errored(kErrorOutOfMemory);
  }

  LabelFlags labelFlags =
    (parentId == Globals::kInvalidId)
      ? LabelFlags::kHasOwnExtraData | LabelFlags::kHasName
      : LabelFlags::kHasOwnExtraData | LabelFlags::kHasName | LabelFlags::kHasParent;

  namedNode->_hashNext = nullptr;
  namedNode->_hashCode = hashCode;
  namedNode->_customData = labelId;
  namedNode->extraData._sectionId = Globals::kInvalidId;
  namedNode->extraData._internalLabelType = type;
  namedNode->extraData._internalLabelFlags = labelFlags;
  namedNode->extraData._internalUInt16Data = 0;
  namedNode->extraData._parentId = parentId;
  namedNode->extraData._nameSize = uint32_t(nameSize);

  char* namePtr = reinterpret_cast<char*>(&namedNode->extraData) + sizeof(LabelEntry::ExtraData);
  memcpy(namePtr, name, nameSize);
  namePtr[nameSize] = '\0';

  *labelIdOut = labelId;
  _labelEntries.appendUnsafe(LabelEntry{&namedNode->extraData, uint64_t(0)});
  _namedLabels.insert(allocator(), namedNode);

  return kErrorOk;
}

uint32_t CodeHolder::labelIdByName(const char* name, size_t nameSize, uint32_t parentId) noexcept {
  uint32_t hashCode = CodeHolder_hashNameAndGetSize(name, nameSize);
  if (ASMJIT_UNLIKELY(!nameSize)) {
    return 0;
  }

  if (parentId != Globals::kInvalidId) {
    hashCode ^= parentId;
  }

  NamedLabelExtraData* namedNode = _namedLabels.get(LabelByName(name, nameSize, hashCode, parentId));
  return namedNode ? namedNode->labelId() : uint32_t(Globals::kInvalidId);
}


ASMJIT_API Error CodeHolder::resolveCrossSectionFixups() noexcept {
  if (!hasUnresolvedFixups()) {
    return kErrorOk;
  }

  Error err = kErrorOk;
  ResolveFixupIterator it(&_fixups);

  while (it.isValid()) {
    Fixup* fixup = it.fixup();
    LabelEntry& le = labelEntry(fixup->labelOrRelocId);

    Support::FastUInt8 of{};
    Section* toSection = _sections[le.sectionId()];
    uint64_t toOffset = Support::addOverflow(toSection->offset(), le.offset(), &of);

    Section* fromSection = sectionById(fixup->sectionId);
    size_t fixupOffset = fixup->offset;

    CodeBuffer& buf = fromSection->buffer();
    ASMJIT_ASSERT(fixupOffset < buf.size());

    // Calculate the offset relative to the start of the virtual base.
    uint64_t fromOffset = Support::addOverflow<uint64_t>(fromSection->offset(), fixupOffset, &of);
    int64_t displacement = int64_t(toOffset - fromOffset + uint64_t(int64_t(fixup->rel)));

    if (ASMJIT_UNLIKELY(of)) {
      err = DebugUtils::errored(kErrorInvalidDisplacement);
    }
    else {
      ASMJIT_ASSERT(size_t(fixupOffset) < buf.size());
      ASMJIT_ASSERT(buf.size() - size_t(fixupOffset) >= fixup->format.valueSize());

      // Overwrite a real displacement in the CodeBuffer.
      if (CodeWriterUtils::writeOffset(buf._data + fixupOffset, displacement, fixup->format)) {
        it.resolveAndNext(this);
        continue;
      }
    }

    it.next();
  }

  _unresolvedFixupCount -= it.resolvedCount();
  return err;
}

ASMJIT_API Error CodeHolder::bindLabel(const Label& label, uint32_t toSectionId, uint64_t toOffset) noexcept {
  uint32_t labelId = label.id();

  if (ASMJIT_UNLIKELY(labelId >= _labelEntries.size())) {
    return DebugUtils::errored(kErrorInvalidLabel);
  }

  if (ASMJIT_UNLIKELY(toSectionId >= _sections.size())) {
    return DebugUtils::errored(kErrorInvalidSection);
  }

  LabelEntry& le = _labelEntries[labelId];

  // Label can be bound only once.
  if (ASMJIT_UNLIKELY(le.isBound())) {
    return DebugUtils::errored(kErrorLabelAlreadyBound);
  }

  Section* section = _sections[toSectionId];
  CodeBuffer& buf = section->buffer();

  // Bind the label - this either assigns a section to LabelEntry's `_objectData` or `_sectionId` in own `ExtraData`.
  // This is basically how this works - when the ExtraData is shared, we replace it by section as the section header
  // is compatible with ExtraData header, and when the LabelEntry has its own ExtraData, the section identifier must
  // be assigned.
  if (le._hasOwnExtraData()) {
    le._ownExtraData()->_sectionId = toSectionId;
  }
  else {
    le._objectData = section;
  }

  // It must be in this order as _offsetsOrFixups as basically a union.
  Fixup* labelFixups = le._getFixups();
  le._offsetOrFixups = toOffset;

  if (!labelFixups) {
    return kErrorOk;
  }

  // Fix all fixups of this label we have collected so far if they are within the same
  // section. We ignore any cross-section fixups as these have to be fixed later.
  Error err = kErrorOk;

  ResolveFixupIterator it(&labelFixups);
  ASMJIT_ASSERT(it.isValid());

  do {
    Fixup* fixup = it.fixup();

    uint32_t relocId = fixup->labelOrRelocId;
    uint32_t fromSectionId = fixup->sectionId;
    size_t fromOffset = fixup->offset;

    if (relocId != Globals::kInvalidId) {
      // Adjust the relocation payload.
      RelocEntry* re = _relocations[relocId];
      re->_payload += toOffset;
      re->_targetSectionId = toSectionId;
    }
    else if (fromSectionId != toSectionId) {
      fixup->labelOrRelocId = labelId;
      it.next();
      continue;
    }
    else {
      ASMJIT_ASSERT(fromOffset < buf.size());
      int64_t displacement = int64_t(toOffset - uint64_t(fromOffset) + uint64_t(int64_t(fixup->rel)));

      // Size of the value we are going to patch.
      ASMJIT_ASSERT(buf.size() - size_t(fromOffset) >= fixup->format.regionSize());

      // Overwrite a real displacement in the CodeBuffer.
      if (!CodeWriterUtils::writeOffset(buf._data + fromOffset, displacement, fixup->format)) {
        err = DebugUtils::errored(kErrorInvalidDisplacement);
        fixup->labelOrRelocId = labelId;
        it.next();
        continue;
      }
    }

    it.resolveAndNext(this);
  } while (it.isValid());

  if (it.unresolvedCount()) {
    *it._pPrev = _fixups;
    _fixups = labelFixups;
  }

  _unresolvedFixupCount -= it.resolvedCount();
  return err;
}

// CodeHolder - Relocations
// ========================

Error CodeHolder::newRelocEntry(RelocEntry** dst, RelocType relocType) noexcept {
  ASMJIT_PROPAGATE(_relocations.willGrow(&_allocator));

  uint32_t relocId = _relocations.size();
  if (ASMJIT_UNLIKELY(relocId == Globals::kInvalidId)) {
    return DebugUtils::errored(kErrorTooManyRelocations);
  }

  RelocEntry* re = _zone.alloc<RelocEntry>();
  if (ASMJIT_UNLIKELY(!re)) {
    return DebugUtils::errored(kErrorOutOfMemory);
  }

  re->_id = relocId;
  re->_relocType = relocType;
  re->_format = OffsetFormat{};
  re->_sourceSectionId = Globals::kInvalidId;
  re->_targetSectionId = Globals::kInvalidId;
  re->_sourceOffset = 0;
  re->_payload = 0;
  _relocations.appendUnsafe(re);

  *dst = re;
  return kErrorOk;
}

// CodeHolder - Expression Evaluation
// ==================================

static Error CodeHolder_evaluateExpression(CodeHolder* self, Expression* exp, uint64_t* out) noexcept {
  uint64_t value[2];
  for (size_t i = 0; i < 2; i++) {
    uint64_t v;
    switch (exp->valueType[i]) {
      case ExpressionValueType::kNone: {
        v = 0;
        break;
      }

      case ExpressionValueType::kConstant: {
        v = exp->value[i].constant;
        break;
      }

      case ExpressionValueType::kLabel: {
        uint32_t labelId = exp->value[i].labelId;
        if (ASMJIT_UNLIKELY(labelId >= self->labelCount())) {
          return DebugUtils::errored(kErrorInvalidLabel);
        }

        LabelEntry& le = self->_labelEntries[labelId];
        if (!le.isBound()) {
          return DebugUtils::errored(kErrorExpressionLabelNotBound);
        }

        v = self->_sections[le.sectionId()]->offset() + le.offset();
        break;
      }

      case ExpressionValueType::kExpression: {
        Expression* nested = exp->value[i].expression;
        ASMJIT_PROPAGATE(CodeHolder_evaluateExpression(self, nested, &v));
        break;
      }

      default:
        return DebugUtils::errored(kErrorInvalidState);
    }

    value[i] = v;
  }

  uint64_t result;
  uint64_t& a = value[0];
  uint64_t& b = value[1];

  switch (exp->opType) {
    case ExpressionOpType::kAdd:
      result = a + b;
      break;

    case ExpressionOpType::kSub:
      result = a - b;
      break;

    case ExpressionOpType::kMul:
      result = a * b;
      break;

    case ExpressionOpType::kSll:
      result = (b > 63) ? uint64_t(0) : uint64_t(a << b);
      break;

    case ExpressionOpType::kSrl:
      result = (b > 63) ? uint64_t(0) : uint64_t(a >> b);
      break;

    case ExpressionOpType::kSra:
      result = Support::sar(a, Support::min<uint64_t>(b, 63));
      break;

    default:
      return DebugUtils::errored(kErrorInvalidState);
  }

  *out = result;
  return kErrorOk;
}

// CodeHolder - Utilities
// ======================

Error CodeHolder::flatten() noexcept {
  uint64_t offset = 0;
  for (Section* section : _sectionsByOrder) {
    uint64_t realSize = section->realSize();
    if (realSize) {
      uint64_t alignedOffset = Support::alignUp(offset, section->alignment());
      if (ASMJIT_UNLIKELY(alignedOffset < offset)) {
        return DebugUtils::errored(kErrorTooLarge);
      }

      Support::FastUInt8 of = 0;
      offset = Support::addOverflow(alignedOffset, realSize, &of);

      if (ASMJIT_UNLIKELY(of)) {
        return DebugUtils::errored(kErrorTooLarge);
      }
    }
  }

  // Now we know that we can assign offsets of all sections properly.
  Section* prev = nullptr;
  offset = 0;
  for (Section* section : _sectionsByOrder) {
    uint64_t realSize = section->realSize();
    if (realSize) {
      offset = Support::alignUp(offset, section->alignment());
    }
    section->_offset = offset;

    // Make sure the previous section extends a bit to cover the alignment.
    if (prev) {
      prev->_virtualSize = offset - prev->_offset;
    }

    prev = section;
    offset += realSize;
  }

  return kErrorOk;
}

size_t CodeHolder::codeSize() const noexcept {
  Support::FastUInt8 of = 0;
  uint64_t offset = 0;

  for (Section* section : _sectionsByOrder) {
    uint64_t realSize = section->realSize();

    if (realSize) {
      uint64_t alignedOffset = Support::alignUp(offset, section->alignment());
      ASMJIT_ASSERT(alignedOffset >= offset);
      offset = Support::addOverflow(alignedOffset, realSize, &of);
    }
  }

  if ((sizeof(uint64_t) > sizeof(size_t) && offset > uint64_t(SIZE_MAX)) || of) {
    return SIZE_MAX;
  }

  return size_t(offset);
}

Error CodeHolder::relocateToBase(uint64_t baseAddress, RelocationSummary* summaryOut) noexcept {
  // Make sure `summaryOut` pointer is always valid as we want to fill it.
  RelocationSummary summaryTmp;
  if (summaryOut == nullptr) {
    summaryOut = &summaryTmp;
  }

  // Fill `summaryOut` defaults.
  summaryOut->codeSizeReduction = 0u;

  // Base address must be provided.
  if (ASMJIT_UNLIKELY(baseAddress == Globals::kNoBaseAddress)) {
    return DebugUtils::errored(kErrorInvalidArgument);
  }

  _baseAddress = baseAddress;
  uint32_t addressSize = _environment.registerSize();

  Section* addressTableSection = _addressTableSection;
  uint32_t addressTableEntryCount = 0;
  uint8_t* addressTableEntryData = nullptr;

  if (addressTableSection) {
    ASMJIT_PROPAGATE(reserveBuffer(&addressTableSection->_buffer, size_t(addressTableSection->virtualSize())));
    addressTableEntryData = addressTableSection->_buffer.data();
  }

  // Relocate all recorded locations.
  for (const RelocEntry* re : _relocations) {
    // Possibly deleted or optimized-out entry.
    if (re->relocType() == RelocType::kNone) {
      continue;
    }

    Section* sourceSection = sectionById(re->sourceSectionId());
    Section* targetSection = nullptr;

    if (re->targetSectionId() != Globals::kInvalidId) {
      targetSection = sectionById(re->targetSectionId());
    }

    uint64_t value = re->payload();
    uint64_t sectionOffset = sourceSection->offset();
    uint64_t sourceOffset = re->sourceOffset();

    // Make sure that the `RelocEntry` doesn't go out of bounds.
    size_t regionSize = re->format().regionSize();
    if (ASMJIT_UNLIKELY(re->sourceOffset() >= sourceSection->bufferSize() ||
                        sourceSection->bufferSize() - size_t(re->sourceOffset()) < regionSize)) {
      return DebugUtils::errored(kErrorInvalidRelocEntry);
    }

    uint8_t* buffer = sourceSection->data();

    switch (re->relocType()) {
      case RelocType::kExpression: {
        Expression* expression = (Expression*)(uintptr_t(value));
        ASMJIT_PROPAGATE(CodeHolder_evaluateExpression(this, expression, &value));
        break;
      }

      case RelocType::kAbsToAbs: {
        break;
      }

      case RelocType::kRelToAbs: {
        // Value is currently a relative offset from the start of its section.
        // We have to convert it to an absolute offset (including base address).
        if (ASMJIT_UNLIKELY(!targetSection)) {
          return DebugUtils::errored(kErrorInvalidRelocEntry);
        }

        //value += baseAddress + sectionOffset + sourceOffset + regionSize;
        value += baseAddress + targetSection->offset();
        break;
      }

      case RelocType::kAbsToRel: {
        value -= baseAddress + sectionOffset + sourceOffset + regionSize;

        // Sign extend as we are not interested in the high 32-bit word in a 32-bit address space.
        if (addressSize <= 4) {
          value = uint64_t(int64_t(int32_t(value & 0xFFFFFFFFu)));
        }
        else if (!Support::isInt32(int64_t(value))) {
          return DebugUtils::errored(kErrorRelocOffsetOutOfRange);
        }

        break;
      }

      case RelocType::kX64AddressEntry: {
        size_t valueOffset = size_t(re->sourceOffset()) + re->format().valueOffset();
        if (re->format().valueSize() != 4 || valueOffset < 2) {
          return DebugUtils::errored(kErrorInvalidRelocEntry);
        }

        // First try whether a relative 32-bit displacement would work.
        value -= baseAddress + sectionOffset + sourceOffset + regionSize;
        if (!Support::isInt32(int64_t(value))) {
          // Relative 32-bit displacement is not possible, use '.addrtab' section.
          AddressTableEntry* atEntry = _addressTableEntries.get(re->payload());
          if (ASMJIT_UNLIKELY(!atEntry)) {
            return DebugUtils::errored(kErrorInvalidRelocEntry);
          }

          // Cannot be null as we have just matched the `AddressTableEntry`.
          ASMJIT_ASSERT(addressTableSection != nullptr);

          if (!atEntry->hasAssignedSlot()) {
            atEntry->_slot = addressTableEntryCount++;
          }

          size_t atEntryIndex = size_t(atEntry->slot()) * addressSize;
          uint64_t addrSrc = sectionOffset + sourceOffset + regionSize;
          uint64_t addrDst = addressTableSection->offset() + uint64_t(atEntryIndex);

          value = addrDst - addrSrc;
          if (!Support::isInt32(int64_t(value))) {
            return DebugUtils::errored(kErrorRelocOffsetOutOfRange);
          }

          // Bytes that replace [REX, OPCODE] bytes.
          uint32_t byte0 = 0xFF;
          uint32_t byte1 = buffer[valueOffset - 1];

          if (byte1 == 0xE8) {
            // Patch CALL/MOD byte to FF /2 (-> 0x15).
            byte1 = x86EncodeMod(0, 2, 5);
          }
          else if (byte1 == 0xE9) {
            // Patch JMP/MOD byte to FF /4 (-> 0x25).
            byte1 = x86EncodeMod(0, 4, 5);
          }
          else {
            return DebugUtils::errored(kErrorInvalidRelocEntry);
          }

          // Patch `jmp/call` instruction.
          buffer[valueOffset - 2] = uint8_t(byte0);
          buffer[valueOffset - 1] = uint8_t(byte1);

          Support::storeu_u64_le(addressTableEntryData + atEntryIndex, re->payload());
        }
        break;
      }

      default:
        return DebugUtils::errored(kErrorInvalidRelocEntry);
    }

    if (!CodeWriterUtils::writeOffset(buffer + re->sourceOffset(), int64_t(value), re->format())) {
      return DebugUtils::errored(kErrorInvalidRelocEntry);
    }
  }

  // Fixup the virtual size of the address table if it's the last section.
  if (_sectionsByOrder.last() == addressTableSection) {
    ASMJIT_ASSERT(addressTableSection != nullptr);

    size_t reservedSize = size_t(addressTableSection->_virtualSize);
    size_t addressTableSize = addressTableEntryCount * addressSize;

    addressTableSection->_buffer._size = addressTableSize;
    addressTableSection->_virtualSize = addressTableSize;

    ASMJIT_ASSERT(reservedSize >= addressTableSize);
    size_t codeSizeReduction = reservedSize - addressTableSize;

    summaryOut->codeSizeReduction = codeSizeReduction;
  }

  return kErrorOk;
}

Error CodeHolder::copySectionData(void* dst, size_t dstSize, uint32_t sectionId, CopySectionFlags copyFlags) noexcept {
  if (ASMJIT_UNLIKELY(!isSectionValid(sectionId))) {
    return DebugUtils::errored(kErrorInvalidSection);
  }

  Section* section = sectionById(sectionId);
  size_t bufferSize = section->bufferSize();

  if (ASMJIT_UNLIKELY(dstSize < bufferSize)) {
    return DebugUtils::errored(kErrorInvalidArgument);
  }

  memcpy(dst, section->data(), bufferSize);

  if (bufferSize < dstSize && Support::test(copyFlags, CopySectionFlags::kPadSectionBuffer)) {
    size_t paddingSize = dstSize - bufferSize;
    memset(static_cast<uint8_t*>(dst) + bufferSize, 0, paddingSize);
  }

  return kErrorOk;
}

Error CodeHolder::copyFlattenedData(void* dst, size_t dstSize, CopySectionFlags copyFlags) noexcept {
  size_t end = 0;
  for (Section* section : _sectionsByOrder) {
    if (section->offset() > dstSize) {
      return DebugUtils::errored(kErrorInvalidArgument);
    }

    size_t bufferSize = section->bufferSize();
    size_t offset = size_t(section->offset());

    if (ASMJIT_UNLIKELY(dstSize - offset < bufferSize)) {
      return DebugUtils::errored(kErrorInvalidArgument);
    }

    uint8_t* dstTarget = static_cast<uint8_t*>(dst) + offset;
    size_t paddingSize = 0;
    memcpy(dstTarget, section->data(), bufferSize);

    if (Support::test(copyFlags, CopySectionFlags::kPadSectionBuffer) && bufferSize < section->virtualSize()) {
      paddingSize = Support::min<size_t>(dstSize - offset, size_t(section->virtualSize())) - bufferSize;
      memset(dstTarget + bufferSize, 0, paddingSize);
    }

    end = Support::max(end, offset + bufferSize + paddingSize);
  }

  if (end < dstSize && Support::test(copyFlags, CopySectionFlags::kPadTargetBuffer)) {
    memset(static_cast<uint8_t*>(dst) + end, 0, dstSize - end);
  }

  return kErrorOk;
}

// CodeHolder - Tests
// ==================

#if defined(ASMJIT_TEST)
UNIT(code_holder) {
  CodeHolder code;

  INFO("Verifying CodeHolder::init()");
  Environment env;
  env.init(Arch::kX86);

  code.init(env);
  EXPECT_EQ(code.arch(), Arch::kX86);

  INFO("Verifying named labels");
  uint32_t dummyId;
  uint32_t labelId1;
  uint32_t labelId2;

  // Anonymous labels can have no-name (this is basically like calling `code.newLabelId()`).
  EXPECT_EQ(code.newNamedLabelId(&dummyId, "", SIZE_MAX, LabelType::kAnonymous), kErrorOk);

  // Global labels must have a name - not providing one is an error.
  EXPECT_EQ(code.newNamedLabelId(&dummyId, "", SIZE_MAX, LabelType::kGlobal), kErrorInvalidLabelName);

  // A name of a global label cannot repeat.
  EXPECT_EQ(code.newNamedLabelId(&labelId1, "NamedLabel1", SIZE_MAX, LabelType::kGlobal), kErrorOk);
  EXPECT_EQ(code.newNamedLabelId(&dummyId, "NamedLabel1", SIZE_MAX, LabelType::kGlobal), kErrorLabelAlreadyDefined);
  EXPECT_TRUE(code.isLabelValid(labelId1));
  EXPECT_EQ(code.labelEntry(labelId1).nameSize(), 11u);
  EXPECT_EQ(strcmp(code.labelEntry(labelId1).name(), "NamedLabel1"), 0);
  EXPECT_EQ(code.labelIdByName("NamedLabel1"), labelId1);

  EXPECT_EQ(code.newNamedLabelId(&labelId2, "NamedLabel2", SIZE_MAX, LabelType::kGlobal), kErrorOk);
  EXPECT_EQ(code.newNamedLabelId(&dummyId, "NamedLabel2", SIZE_MAX, LabelType::kGlobal), kErrorLabelAlreadyDefined);
  EXPECT_TRUE(code.isLabelValid(labelId2));
  EXPECT_EQ(code.labelEntry(labelId2).nameSize(), 11u);
  EXPECT_EQ(strcmp(code.labelEntry(labelId2).name(), "NamedLabel2"), 0);
  EXPECT_EQ(code.labelIdByName("NamedLabel2"), labelId2);

  INFO("Verifying section ordering");
  Section* section1;
  EXPECT_EQ(code.newSection(&section1, "high-priority", SIZE_MAX, SectionFlags::kNone, 1, -1), kErrorOk);
  EXPECT_EQ(code.sections()[1], section1);
  EXPECT_EQ(code.sectionsByOrder()[0], section1);

  Section* section0;
  EXPECT_EQ(code.newSection(&section0, "higher-priority", SIZE_MAX, SectionFlags::kNone, 1, -2), kErrorOk);
  EXPECT_EQ(code.sections()[2], section0);
  EXPECT_EQ(code.sectionsByOrder()[0], section0);
  EXPECT_EQ(code.sectionsByOrder()[1], section1);

  Section* section3;
  EXPECT_EQ(code.newSection(&section3, "low-priority", SIZE_MAX, SectionFlags::kNone, 1, 2), kErrorOk);
  EXPECT_EQ(code.sections()[3], section3);
  EXPECT_EQ(code.sectionsByOrder()[3], section3);
}
#endif

ASMJIT_END_NAMESPACE
