// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Export]
#define ASMJIT_EXPORTS

// [Dependencies]
#include "../core/assembler.h"
#include "../core/intutils.h"
#include "../core/logging.h"
#include "../core/memutils.h"
#include "../core/stringutils.h"

ASMJIT_BEGIN_NAMESPACE

// ============================================================================
// [asmjit::ErrorHandler]
// ============================================================================

ErrorHandler::ErrorHandler() noexcept {}
ErrorHandler::~ErrorHandler() noexcept {}

// ============================================================================
// [asmjit::CodeHolder - Utilities]
// ============================================================================

static void CodeHolder_resetInternal(CodeHolder* self, bool releaseMemory) noexcept {
  uint32_t i;
  const ZoneVector<CodeEmitter*>& emitters = self->getEmitters();

  i = emitters.getLength();
  while (i)
    self->detach(emitters[--i]);

  // Reset everything into its construction state.
  self->_codeInfo.reset();
  self->_emitterOptions = 0;
  self->_logger = nullptr;
  self->_errorHandler = nullptr;

  self->_unresolvedLabelCount = 0;
  self->_trampolinesSize = 0;

  // Reset all sections.
  uint32_t numSections = self->_sections.getLength();
  for (i = 0; i < numSections; i++) {
    SectionEntry* section = self->_sections[i];
    if (section->_buffer.hasData() && !section->_buffer.isExternal())
      MemUtils::release(section->_buffer._data);
    section->_buffer._data = nullptr;
    section->_buffer._capacity = 0;
  }

  // Reset zone allocator and all containers using it.
  ZoneAllocator* allocator = self->getAllocator();

  self->_emitters.reset();
  self->_namedLabels.reset();
  self->_relocations.reset();
  self->_labelEntries.reset();
  self->_sections.reset();

  allocator->reset(&self->_zone);
  self->_zone.reset(releaseMemory);
}

static void CodeHolder_modifyEmitterOptions(CodeHolder* self, uint32_t clear, uint32_t add) noexcept {
  uint32_t oldOpt = self->_emitterOptions;
  uint32_t newOpt = (oldOpt & ~clear) | add;

  if (oldOpt == newOpt)
    return;

  // Modify emitter options of `CodeHolder` itself.
  self->_emitterOptions = newOpt;

  // Modify emitter options of all attached emitters.
  const ZoneVector<CodeEmitter*>& emitters = self->getEmitters();
  for (uint32_t i = 0, len = emitters.getLength(); i < len; i++) {
    CodeEmitter* emitter = emitters[i];
    emitter->_emitterOptions = (emitter->_emitterOptions & ~clear) | add;
    emitter->onUpdateGlobalInstOptions();
  }
}

// ============================================================================
// [asmjit::CodeHolder - Construction / Destruction]
// ============================================================================

CodeHolder::CodeHolder() noexcept
  : _codeInfo(),
    _emitterOptions(0),
    _logger(nullptr),
    _errorHandler(nullptr),
    _unresolvedLabelCount(0),
    _trampolinesSize(0),
    _zone(16384 - Zone::kZoneOverhead),
    _allocator(&_zone) {}

CodeHolder::~CodeHolder() noexcept {
  CodeHolder_resetInternal(this, true);
}

// ============================================================================
// [asmjit::CodeHolder - Init / Reset]
// ============================================================================

Error CodeHolder::init(const CodeInfo& info) noexcept {
  // Cannot reinitialize if it's locked or there is one or more CodeEmitter
  // attached.
  if (isInitialized())
    return DebugUtils::errored(kErrorAlreadyInitialized);

  // If we are just initializing there should be no emitters attached.
  ASMJIT_ASSERT(_emitters.isEmpty());

  // Create the default section and insert it to the `_sections` array.
  Error err = _sections.willGrow(&_allocator);
  if (err == kErrorOk) {
    SectionEntry* se = _allocator.allocZeroedT<SectionEntry>();
    if (ASMJIT_LIKELY(se)) {
      se->_flags = SectionEntry::kFlagExec | SectionEntry::kFlagConst;
      se->_setDefaultName('.', 't', 'e', 'x', 't');
      _sections.appendUnsafe(se);
    }
    else {
      err = DebugUtils::errored(kErrorNoHeapMemory);
    }
  }

  if (ASMJIT_UNLIKELY(err)) {
    _zone.reset(false);
    return err;
  }
  else {
    _codeInfo = info;
    return kErrorOk;
  }
}

void CodeHolder::reset(bool releaseMemory) noexcept {
  CodeHolder_resetInternal(this, releaseMemory);
}

// ============================================================================
// [asmjit::CodeHolder - Attach / Detach]
// ============================================================================

Error CodeHolder::attach(CodeEmitter* emitter) noexcept {
  // Catch a possible misuse of the API.
  if (ASMJIT_UNLIKELY(!emitter))
    return DebugUtils::errored(kErrorInvalidArgument);

  // Invalid emitter, this should not be possible.
  uint32_t type = emitter->getEmitterType();
  if (ASMJIT_UNLIKELY(type == CodeEmitter::kTypeNone || type >= CodeEmitter::kTypeCount))
    return DebugUtils::errored(kErrorInvalidState);

  // This is suspicious, but don't fail if `emitter` is already attached
  // to this code holder. This is not error, but it's not recommended.
  if (emitter->_code != nullptr) {
    if (emitter->_code == this)
      return kErrorOk;
    return DebugUtils::errored(kErrorInvalidState);
  }

  // Reserve the space now as we cannot fail after `onAttach()` succeeded.
  ASMJIT_PROPAGATE(_emitters.willGrow(&_allocator, 1));
  ASMJIT_PROPAGATE(emitter->onAttach(this));

  // Connect CodeHolder <-> CodeEmitter.
  ASMJIT_ASSERT(emitter->_code == this);
  _emitters.appendUnsafe(emitter);

  return kErrorOk;
}

Error CodeHolder::detach(CodeEmitter* emitter) noexcept {
  if (ASMJIT_UNLIKELY(!emitter))
    return DebugUtils::errored(kErrorInvalidArgument);

  if (ASMJIT_UNLIKELY(emitter->_code != this))
    return DebugUtils::errored(kErrorInvalidState);

  // NOTE: We always detach if we were asked to, if error happens during
  // `emitter->onDetach()` we just propagate it, but the CodeEmitter will
  // be detached.
  Error err = kErrorOk;
  if (!emitter->isDestroyed())
    err = emitter->onDetach(this);

  // Disconnect CodeHolder <-> CodeEmitter.
  uint32_t index = _emitters.indexOf(emitter);
  ASMJIT_ASSERT(index != Globals::kNotFound);

  _emitters.removeAt(index);
  emitter->_code = nullptr;

  return err;
}

// ============================================================================
// [asmjit::CodeHolder - Result Information]
// ============================================================================

size_t CodeHolder::getCodeSize() const noexcept {
  // TODO: Support sections.
  return _sections[0]->_buffer._length + getTrampolinesSize();
}

// ============================================================================
// [asmjit::CodeHolder - Logging & Error Handling]
// ============================================================================

void CodeHolder::setLogger(Logger* logger) noexcept {
  #ifndef ASMJIT_DISABLE_LOGGING
  _logger = logger;
  uint32_t option = !logger ? uint32_t(0) : uint32_t(CodeEmitter::kOptionLoggingEnabled);
  CodeHolder_modifyEmitterOptions(this, CodeEmitter::kOptionLoggingEnabled, option);
  #else
  ASMJIT_UNUSED(logger);
  #endif
}

// ============================================================================
// [asmjit::CodeHolder - Sections]
// ============================================================================

static Error CodeHolder_reserveInternal(CodeHolder* self, CodeBuffer* cb, size_t n) noexcept {
  uint8_t* oldData = cb->_data;
  uint8_t* newData;

  if (oldData && !cb->isExternal())
    newData = static_cast<uint8_t*>(MemUtils::realloc(oldData, n));
  else
    newData = static_cast<uint8_t*>(MemUtils::alloc(n));

  if (ASMJIT_UNLIKELY(!newData))
    return DebugUtils::errored(kErrorNoHeapMemory);

  cb->_data = newData;
  cb->_capacity = n;

  // Update `Assembler` pointers if attached.
  const ZoneVector<CodeEmitter*>& emitters = self->getEmitters();
  for (uint32_t i = 0, len = emitters.getLength(); i < len; i++) {
    CodeEmitter* emitter = emitters[i];
    if (emitter->isAssembler()) {
      Assembler* a = static_cast<Assembler*>(emitter);
      if (&a->_section->_buffer == cb) {
        size_t offset = a->getOffset();

        a->_bufferData = newData;
        a->_bufferEnd  = newData + n;
        a->_bufferPtr  = newData + offset;
      }
    }
  }

  return kErrorOk;
}

Error CodeHolder::growBuffer(CodeBuffer* cb, size_t n) noexcept {
  // Now the length of the section must be valid.
  size_t length = cb->getLength();
  if (ASMJIT_UNLIKELY(n > std::numeric_limits<uintptr_t>::max() - length))
    return DebugUtils::errored(kErrorNoHeapMemory);

  // We can now check if growing the buffer is really necessary. It's unlikely
  // that this function is called while there is still room for `n` bytes.
  size_t capacity = cb->getCapacity();
  size_t required = cb->getLength() + n;
  if (ASMJIT_UNLIKELY(required <= capacity))
    return kErrorOk;

  if (cb->isFixed())
    return DebugUtils::errored(kErrorCodeTooLarge);

  size_t kInitialCapacity = 8096;
  if (capacity < kInitialCapacity)
    capacity = kInitialCapacity;
  else
    capacity += Globals::kAllocOverhead;

  do {
    size_t old = capacity;
    if (capacity < Globals::kAllocThreshold)
      capacity *= 2;
    else
      capacity += Globals::kAllocThreshold;

    // Overflow.
    if (ASMJIT_UNLIKELY(old > capacity))
      return DebugUtils::errored(kErrorNoHeapMemory);
  } while (capacity - Globals::kAllocOverhead < required);

  return CodeHolder_reserveInternal(this, cb, capacity - Globals::kAllocOverhead);
}

Error CodeHolder::reserveBuffer(CodeBuffer* cb, size_t n) noexcept {
  size_t capacity = cb->getCapacity();
  if (n <= capacity) return kErrorOk;

  if (cb->isFixed())
    return DebugUtils::errored(kErrorCodeTooLarge);

  return CodeHolder_reserveInternal(this, cb, n);
}

// ============================================================================
// [asmjit::CodeHolder - Labels & Symbols]
// ============================================================================

namespace {

//! \internal
//!
//! Only used to lookup a label from `_namedLabels`.
class LabelByName {
public:
  inline LabelByName(const char* key, size_t len, uint32_t hVal) noexcept
    : key(key),
      len(uint32_t(len)),
      hVal(hVal) {}

  inline uint32_t getHVal() const noexcept { return hVal; }

  inline bool matches(const LabelEntry* entry) const noexcept {
    return entry->getNameLength() == len && std::memcmp(entry->getName(), key, len) == 0;
  }

  const char* key;
  uint32_t len;
  uint32_t hVal;
};

// Returns a hash of `name` and fixes `nameLength` if it's `Globals::kNullTerminated`.
static uint32_t CodeHolder_hashNameAndFixLen(const char* name, size_t& nameLength) noexcept {
  uint32_t hVal = 0;
  if (nameLength == Globals::kNullTerminated) {
    size_t i = 0;
    for (;;) {
      uint8_t c = uint8_t(name[i]);
      if (!c) break;
      hVal = StringUtils::hashRound(hVal, c);
      i++;
    }
    nameLength = i;
  }
  else {
    for (size_t i = 0; i < nameLength; i++) {
      uint8_t c = uint8_t(name[i]);
      if (ASMJIT_UNLIKELY(!c)) return DebugUtils::errored(kErrorInvalidLabelName);
      hVal = StringUtils::hashRound(hVal, c);
    }
  }
  return hVal;
}

} // anonymous namespace

LabelLink* CodeHolder::newLabelLink(LabelEntry* le, uint32_t sectionId, size_t offset, intptr_t rel) noexcept {
  LabelLink* link = _allocator.allocT<LabelLink>();
  if (ASMJIT_UNLIKELY(!link)) return nullptr;

  link->prev = le->_links;
  le->_links = link;

  link->sectionId = sectionId;
  link->relocId = RelocEntry::kInvalidId;
  link->offset = offset;
  link->rel = rel;

  _unresolvedLabelCount++;
  return link;
}

Error CodeHolder::newLabelId(uint32_t& idOut) noexcept {
  idOut = 0;

  uint32_t index = _labelEntries.getLength();
  if (ASMJIT_UNLIKELY(index >= uint32_t(Operand::kPackedIdCount)))
    return DebugUtils::errored(kErrorLabelIndexOverflow);

  ASMJIT_PROPAGATE(_labelEntries.willGrow(&_allocator));
  LabelEntry* le = _allocator.allocZeroedT<LabelEntry>();

  if (ASMJIT_UNLIKELY(!le))
    return DebugUtils::errored(kErrorNoHeapMemory);

  uint32_t id = Operand::packId(index);
  le->_setId(id);
  le->_parentId = 0;
  le->_sectionId = SectionEntry::kInvalidId;
  le->_offset = 0;

  _labelEntries.appendUnsafe(le);
  idOut = id;
  return kErrorOk;
}

Error CodeHolder::newNamedLabelId(uint32_t& idOut, const char* name, size_t nameLength, uint32_t type, uint32_t parentId) noexcept {
  idOut = 0;
  uint32_t hVal = CodeHolder_hashNameAndFixLen(name, nameLength);

  if (ASMJIT_UNLIKELY(nameLength == 0))
    return DebugUtils::errored(kErrorInvalidLabelName);

  if (ASMJIT_UNLIKELY(nameLength > Globals::kMaxLabelLength))
    return DebugUtils::errored(kErrorLabelNameTooLong);

  switch (type) {
    case Label::kTypeLocal:
      if (ASMJIT_UNLIKELY(Operand::unpackId(parentId) >= _labelEntries.getLength()))
        return DebugUtils::errored(kErrorInvalidParentLabel);

      hVal ^= parentId;
      break;

    case Label::kTypeGlobal:
      if (ASMJIT_UNLIKELY(parentId != 0))
        return DebugUtils::errored(kErrorNonLocalLabelCantHaveParent);

      break;

    default:
      return DebugUtils::errored(kErrorInvalidArgument);
  }

  // Don't allow to insert duplicates. Local labels allow duplicates that have
  // different id, this is already accomplished by having a different hashes
  // between the same label names having different parent labels.
  LabelEntry* le = _namedLabels.get(LabelByName(name, nameLength, hVal));
  if (ASMJIT_UNLIKELY(le))
    return DebugUtils::errored(kErrorLabelAlreadyDefined);

  Error err = kErrorOk;
  uint32_t index = _labelEntries.getLength();

  if (ASMJIT_UNLIKELY(index >= uint32_t(Operand::kPackedIdCount)))
    return DebugUtils::errored(kErrorLabelIndexOverflow);

  ASMJIT_PROPAGATE(_labelEntries.willGrow(&_allocator));
  le = _allocator.allocZeroedT<LabelEntry>();

  if (ASMJIT_UNLIKELY(!le))
    return DebugUtils::errored(kErrorNoHeapMemory);

  uint32_t id = Operand::packId(index);
  le->_hVal = hVal;
  le->_setId(id);
  le->_type = uint8_t(type);
  le->_parentId = 0;
  le->_sectionId = SectionEntry::kInvalidId;
  le->_offset = 0;
  ASMJIT_PROPAGATE(le->_name.setData(&_zone, name, nameLength));

  _labelEntries.appendUnsafe(le);
  _namedLabels.insert(getAllocator(), le);

  idOut = id;
  return err;
}

uint32_t CodeHolder::getLabelIdByName(const char* name, size_t nameLength, uint32_t parentId) noexcept {
  // TODO: Finalize - parent id is not used here?
  ASMJIT_UNUSED(parentId);

  uint32_t hVal = CodeHolder_hashNameAndFixLen(name, nameLength);
  if (ASMJIT_UNLIKELY(!nameLength)) return 0;

  LabelEntry* le = _namedLabels.get(LabelByName(name, nameLength, hVal));
  return le ? le->getId() : uint32_t(0);
}

// ============================================================================
// [asmjit::CodeEmitter - Relocations]
// ============================================================================

//! Encode MOD byte.
static inline uint32_t x86EncodeMod(uint32_t m, uint32_t o, uint32_t rm) noexcept {
  return (m << 6) | (o << 3) | rm;
}

Error CodeHolder::newRelocEntry(RelocEntry** dst, uint32_t type, uint32_t size) noexcept {
  ASMJIT_PROPAGATE(_relocations.willGrow(&_allocator));

  uint32_t index = _relocations.getLength();
  RelocEntry* re = _allocator.allocZeroedT<RelocEntry>();

  if (ASMJIT_UNLIKELY(!re))
    return DebugUtils::errored(kErrorNoHeapMemory);

  re->_id = index;
  re->_type = uint8_t(type);
  re->_size = uint8_t(size);
  re->_sourceSectionId = SectionEntry::kInvalidId;
  re->_targetSectionId = SectionEntry::kInvalidId;
  _relocations.appendUnsafe(re);

  *dst = re;
  return kErrorOk;
}

// TODO: Support multiple sections, this only relocates the first.
size_t CodeHolder::relocate(void* _dst, uint64_t baseAddress) const noexcept {
  SectionEntry* section = _sections[0];
  ASMJIT_ASSERT(section != nullptr);

  uint8_t* dst = static_cast<uint8_t*>(_dst);
  if (baseAddress == Globals::kNoBaseAddress)
    baseAddress = uint64_t((uintptr_t)dst);

  #ifndef ASMJIT_DISABLE_LOGGING
  Logger* logger = getLogger();
  #endif

  size_t minCodeSize = section->getBuffer().getLength(); // Minimum code size.
  size_t maxCodeSize = getCodeSize();                    // Includes all possible trampolines.

  // We will copy the exact size of the generated code. Extra code for trampolines
  // is generated on-the-fly by the relocator (this code doesn't exist at the moment).
  std::memcpy(dst, section->_buffer._data, minCodeSize);

  // Trampoline offset from the beginning of dst/baseAddress.
  size_t trampOffset = minCodeSize;

  // Relocate all recorded locations.
  size_t numRelocs = _relocations.getLength();
  const RelocEntry* const* reArray = _relocations.getData();

  for (size_t i = 0; i < numRelocs; i++) {
    const RelocEntry* re = reArray[i];

    // Possibly deleted or optimized out relocation entry.
    if (re->getType() == RelocEntry::kTypeNone)
      continue;

    uint64_t ptr = re->getData();
    size_t codeOffset = size_t(re->getSourceOffset());

    // Make sure that the `RelocEntry` is correct, we don't want to write
    // out of bounds in `dst`.
    if (ASMJIT_UNLIKELY(codeOffset + re->getSize() > maxCodeSize))
      return DebugUtils::errored(kErrorInvalidRelocEntry);

    // Whether to use trampoline, can be only used if relocation type is `kRelocTrampoline`.
    bool useTrampoline = false;

    switch (re->getType()) {
      case RelocEntry::kTypeAbsToAbs: {
        break;
      }

      case RelocEntry::kTypeRelToAbs: {
        ptr += baseAddress;
        break;
      }

      case RelocEntry::kTypeAbsToRel: {
        ptr -= baseAddress + re->getSourceOffset() + re->getSize();
        break;
      }

      case RelocEntry::kTypeTrampoline: {
        if (re->getSize() != 4)
          return DebugUtils::errored(kErrorInvalidRelocEntry);

        ptr -= baseAddress + re->getSourceOffset() + re->getSize();
        if (!IntUtils::isInt32(int64_t(ptr))) {
          ptr = (uint64_t)trampOffset - re->getSourceOffset() - re->getSize();
          useTrampoline = true;
        }
        break;
      }

      default:
        return DebugUtils::errored(kErrorInvalidRelocEntry);
    }

    switch (re->getSize()) {
      case 1:
        MemUtils::writeU8(dst + codeOffset, uint32_t(ptr & 0xFFU));
        break;

      case 4:
        MemUtils::writeU32u(dst + codeOffset, uint32_t(ptr & 0xFFFFFFFFU));
        break;

      case 8:
        MemUtils::writeU64u(dst + codeOffset, ptr);
        break;

      default:
        return DebugUtils::errored(kErrorInvalidRelocEntry);
    }

    // Handle the trampoline case.
    if (useTrampoline) {
      // Bytes that replace [REX, OPCODE] bytes.
      uint32_t byte0 = 0xFF;
      uint32_t byte1 = dst[codeOffset - 1];

      if (byte1 == 0xE8) {
        // Patch CALL/MOD byte to FF/2 (-> 0x15).
        byte1 = x86EncodeMod(0, 2, 5);
      }
      else if (byte1 == 0xE9) {
        // Patch JMP/MOD byte to FF/4 (-> 0x25).
        byte1 = x86EncodeMod(0, 4, 5);
      }
      else {
        return DebugUtils::errored(kErrorInvalidRelocEntry);
      }

      // Patch `jmp/call` instruction.
      ASMJIT_ASSERT(codeOffset >= 2);
      dst[codeOffset - 2] = uint8_t(byte0);
      dst[codeOffset - 1] = uint8_t(byte1);

      // Store absolute address and advance the trampoline pointer.
      MemUtils::writeU64u(dst + trampOffset, re->getData());
      trampOffset += 8;

#ifndef ASMJIT_DISABLE_LOGGING
      if (logger)
        logger->logf(".dq 0x%016llX ; TRAMP\n", re->getData());
#endif
    }
  }

  // If there are no trampolines this is the same as `minCodeSize`.
  return trampOffset;
}

ASMJIT_END_NAMESPACE
