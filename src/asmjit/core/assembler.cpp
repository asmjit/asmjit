// [AsmJit]
// Machine Code Generation for C++.
//
// [License]
// ZLIB - See LICENSE.md file in the package.

#define ASMJIT_EXPORTS

#include "../core/assembler.h"
#include "../core/codebufferwriter_p.h"
#include "../core/constpool.h"
#include "../core/logging.h"
#include "../core/support.h"

ASMJIT_BEGIN_NAMESPACE

// ============================================================================
// [asmjit::BaseAssembler - Construction / Destruction]
// ============================================================================

BaseAssembler::BaseAssembler() noexcept
  : BaseEmitter(kTypeAssembler),
    _section(nullptr),
    _bufferData(nullptr),
    _bufferEnd(nullptr),
    _bufferPtr(nullptr),
    _op4(),
    _op5() {}
BaseAssembler::~BaseAssembler() noexcept {}

// ============================================================================
// [asmjit::BaseAssembler - Buffer Management]
// ============================================================================

Error BaseAssembler::setOffset(size_t offset) {
  if (ASMJIT_UNLIKELY(!_code))
    return DebugUtils::errored(kErrorNotInitialized);

  size_t size = Support::max<size_t>(_section->bufferSize(), this->offset());
  if (ASMJIT_UNLIKELY(offset > size))
    return reportError(DebugUtils::errored(kErrorInvalidArgument));

  _bufferPtr = _bufferData + offset;
  return kErrorOk;
}

// ============================================================================
// [asmjit::BaseAssembler - Logging]
// ============================================================================

#ifndef ASMJIT_DISABLE_LOGGING
static void BaseAssembler_logLabel(BaseAssembler* self, const Label& label) noexcept {
  Logger* logger = self->_code->_logger;

  StringTmp<512> sb;
  size_t binSize = logger->hasFlag(FormatOptions::kFlagMachineCode) ? size_t(0) : std::numeric_limits<size_t>::max();

  sb.appendChars(' ', logger->indentation(FormatOptions::kIndentationLabel));
  Logging::formatLabel(sb, logger->flags(), self, label.id());
  sb.appendChar(':');
  Logging::formatLine(sb, nullptr, binSize, 0, 0, self->_inlineComment);
  logger->log(sb.data(), sb.size());
}
#endif

// ============================================================================
// [asmjit::BaseAssembler - Section Management]
// ============================================================================

static void BaseAssembler_initSection(BaseAssembler* self, Section* section) noexcept {
  uint8_t* p = section->_buffer._data;

  self->_section = section;
  self->_bufferData = p;
  self->_bufferPtr  = p + section->_buffer._size;
  self->_bufferEnd  = p + section->_buffer._capacity;
}

Error BaseAssembler::section(Section* section) {
  if (ASMJIT_UNLIKELY(!_code))
    return reportError(DebugUtils::errored(kErrorNotInitialized));

  if (!_code->isSectionValid(section->id()) || _code->_sections[section->id()] != section)
    return reportError(DebugUtils::errored(kErrorInvalidSection));

  #ifndef ASMJIT_DISABLE_LOGGING
  if (hasEmitterOption(kOptionLoggingEnabled))
    _code->_logger->logf(".section %s {#%u}\n", section->name(), section->id());
  #endif

  BaseAssembler_initSection(this, section);
  return kErrorOk;
}

// ============================================================================
// [asmjit::BaseAssembler - Label Management]
// ============================================================================

Label BaseAssembler::newLabel() {
  uint32_t labelId = Globals::kInvalidId;
  if (ASMJIT_LIKELY(_code)) {
    LabelEntry* le;
    Error err = _code->newLabelEntry(&le);
    if (ASMJIT_UNLIKELY(err))
      reportError(err);
    labelId = le->id();
  }
  return Label(labelId);
}

Label BaseAssembler::newNamedLabel(const char* name, size_t nameSize, uint32_t type, uint32_t parentId) {
  uint32_t labelId = Globals::kInvalidId;
  if (ASMJIT_LIKELY(_code)) {
    LabelEntry* le;
    Error err = _code->newNamedLabelEntry(&le, name, nameSize, type, parentId);
    if (ASMJIT_UNLIKELY(err))
      reportError(err);
    labelId = le->id();
  }
  return Label(labelId);
}

Error BaseAssembler::bind(const Label& label) {
  if (ASMJIT_UNLIKELY(!_code))
    return DebugUtils::errored(kErrorNotInitialized);

  Error err = _code->bindLabel(label, _section->id(), offset());

  #ifndef ASMJIT_DISABLE_LOGGING
  if (hasEmitterOption(kOptionLoggingEnabled))
    BaseAssembler_logLabel(this, label);
  #endif

  resetInlineComment();
  if (err)
    return reportError(err);

  return kErrorOk;
}

// ============================================================================
// [asmjit::BaseAssembler - Emit (Low-Level)]
// ============================================================================

Error BaseAssembler::_emit(uint32_t instId, const Operand_& o0, const Operand_& o1, const Operand_& o2, const Operand_& o3, const Operand_& o4, const Operand_& o5) {
  _op4 = o4;
  _op5 = o5;
  _instOptions |= BaseInst::kOptionOp4Op5Used;
  return _emit(instId, o0, o1, o2, o3);
}

Error BaseAssembler::_emitOpArray(uint32_t instId, const Operand_* operands, size_t count) {
  const Operand_* o0 = &operands[0];
  const Operand_* o1 = &operands[1];
  const Operand_* o2 = &operands[2];
  const Operand_* o3 = &operands[3];

  switch (count) {
    case 0: o0 = &Globals::none; ASMJIT_FALLTHROUGH;
    case 1: o1 = &Globals::none; ASMJIT_FALLTHROUGH;
    case 2: o2 = &Globals::none; ASMJIT_FALLTHROUGH;
    case 3: o3 = &Globals::none; ASMJIT_FALLTHROUGH;
    case 4:
      return _emit(instId, *o0, *o1, *o2, *o3);

    case 5:
      _op4 = operands[4];
      _op5.reset();
      _instOptions |= BaseInst::kOptionOp4Op5Used;
      return _emit(instId, *o0, *o1, *o2, *o3);

    case 6:
      _op4 = operands[4];
      _op5 = operands[5];
      _instOptions |= BaseInst::kOptionOp4Op5Used;
      return _emit(instId, *o0, *o1, *o2, *o3);

    default:
      return DebugUtils::errored(kErrorInvalidArgument);
  }
}

#ifndef ASMJIT_DISABLE_LOGGING
void BaseAssembler::_emitLog(
  uint32_t instId, uint32_t options, const Operand_& o0, const Operand_& o1, const Operand_& o2, const Operand_& o3,
  uint32_t relSize, uint32_t immSize, uint8_t* afterCursor) {

  Logger* logger = _code->logger();
  ASMJIT_ASSERT(logger != nullptr);
  ASMJIT_ASSERT(options & BaseEmitter::kOptionLoggingEnabled);

  StringTmp<256> sb;
  uint32_t flags = logger->flags();

  uint8_t* beforeCursor = _bufferPtr;
  intptr_t emittedSize = (intptr_t)(afterCursor - beforeCursor);

  Operand_ operands[Globals::kMaxOpCount];
  operands[0].copyFrom(o0);
  operands[1].copyFrom(o1);
  operands[2].copyFrom(o2);
  operands[3].copyFrom(o3);

  if (options & BaseInst::kOptionOp4Op5Used) {
    operands[4].copyFrom(_op4);
    operands[5].copyFrom(_op5);
  }
  else {
    operands[4].reset();
    operands[5].reset();
  }

  sb.appendChars(' ', logger->indentation(FormatOptions::kIndentationCode));
  Logging::formatInstruction(sb, flags, this, archId(), BaseInst(instId, options, _extraReg), operands, Globals::kMaxOpCount);

  if ((flags & FormatOptions::kFlagMachineCode) != 0)
    Logging::formatLine(sb, _bufferPtr, size_t(emittedSize), relSize, immSize, inlineComment());
  else
    Logging::formatLine(sb, nullptr, std::numeric_limits<size_t>::max(), 0, 0, inlineComment());
  logger->log(sb);
}

Error BaseAssembler::_emitFailed(
  Error err,
  uint32_t instId, uint32_t options, const Operand_& o0, const Operand_& o1, const Operand_& o2, const Operand_& o3) {

  StringTmp<256> sb;
  sb.appendString(DebugUtils::errorAsString(err));
  sb.appendString(": ");

  Operand_ operands[Globals::kMaxOpCount];
  operands[0].copyFrom(o0);
  operands[1].copyFrom(o1);
  operands[2].copyFrom(o2);
  operands[3].copyFrom(o3);

  if (options & BaseInst::kOptionOp4Op5Used) {
    operands[4].copyFrom(_op4);
    operands[5].copyFrom(_op5);
  }
  else {
    operands[4].reset();
    operands[5].reset();
  }

  Logging::formatInstruction(sb, 0, this, archId(), BaseInst(instId, options, _extraReg), operands, Globals::kMaxOpCount);
  resetInstOptions();
  resetExtraReg();
  resetInlineComment();
  return reportError(err, sb.data());
}
#endif

// ============================================================================
// [asmjit::BaseAssembler - Embed]
// ============================================================================

Error BaseAssembler::embed(const void* data, uint32_t size) {
  if (ASMJIT_UNLIKELY(!_code))
    return DebugUtils::errored(kErrorNotInitialized);

  if (size == 0)
    return DebugUtils::errored(kErrorInvalidArgument);

  CodeBufferWriter writer(this);
  ASMJIT_PROPAGATE(writer.ensureSpace(this, size));

  writer.emitData(data, size);

  #ifndef ASMJIT_DISABLE_LOGGING
  if (ASMJIT_UNLIKELY(hasEmitterOption(kOptionLoggingEnabled)))
    _code->_logger->logBinary(data, size);
  #endif

  writer.done(this);
  return kErrorOk;
}

Error BaseAssembler::embedLabel(const Label& label) {
  if (ASMJIT_UNLIKELY(!_code))
    return DebugUtils::errored(kErrorNotInitialized);

  ASMJIT_ASSERT(_code != nullptr);
  RelocEntry* re;
  LabelEntry* le = _code->labelEntry(label);

  if (ASMJIT_UNLIKELY(!le))
    return reportError(DebugUtils::errored(kErrorInvalidLabel));

  uint32_t size = gpSize();
  CodeBufferWriter writer(this);
  ASMJIT_PROPAGATE(writer.ensureSpace(this, size));

  #ifndef ASMJIT_DISABLE_LOGGING
  if (ASMJIT_UNLIKELY(hasEmitterOption(kOptionLoggingEnabled)))
    _code->_logger->logf(size == 4 ? ".dd L%u\n" : ".dq L%u\n", label.id());
  #endif

  Error err = _code->newRelocEntry(&re, RelocEntry::kTypeRelToAbs, size);
  if (ASMJIT_UNLIKELY(err))
    return reportError(err);

  re->_sourceSectionId = _section->id();
  re->_sourceOffset = offset();

  if (le->isBound()) {
    re->_targetSectionId = le->section()->id();
    re->_payload = le->offset();
  }
  else {
    LabelLink* link = _code->newLabelLink(le, _section->id(), offset(), 0);
    if (ASMJIT_UNLIKELY(!link))
      return reportError(DebugUtils::errored(kErrorOutOfMemory));
    link->relocId = re->id();
  }

  // Emit dummy DWORD/QWORD depending on the address size.
  writer.emitZeros(size);
  writer.done(this);

  return kErrorOk;
}

Error BaseAssembler::embedConstPool(const Label& label, const ConstPool& pool) {
  if (ASMJIT_UNLIKELY(!_code))
    return DebugUtils::errored(kErrorNotInitialized);

  if (ASMJIT_UNLIKELY(!isLabelValid(label)))
    return DebugUtils::errored(kErrorInvalidLabel);

  ASMJIT_PROPAGATE(align(kAlignData, uint32_t(pool.alignment())));
  ASMJIT_PROPAGATE(bind(label));

  size_t size = pool.size();
  CodeBufferWriter writer(this);
  ASMJIT_PROPAGATE(writer.ensureSpace(this, size));

  pool.fill(writer.cursor());

  #ifndef ASMJIT_DISABLE_LOGGING
  if (ASMJIT_UNLIKELY(hasEmitterOption(kOptionLoggingEnabled)))
    _code->_logger->logBinary(writer.cursor(), size);
  #endif

  writer.advance(size);
  writer.done(this);

  return kErrorOk;
}

// ============================================================================
// [asmjit::BaseAssembler - Comment]
// ============================================================================

Error BaseAssembler::comment(const char* data, size_t size) {
  if (ASMJIT_UNLIKELY(!_code))
    return DebugUtils::errored(kErrorNotInitialized);

  #ifndef ASMJIT_DISABLE_LOGGING
  if (hasEmitterOption(kOptionLoggingEnabled)) {
    Logger* logger = _code->logger();
    logger->log(data, size);
    logger->log("\n", 1);
    return kErrorOk;
  }
  #else
  ASMJIT_UNUSED(data);
  ASMJIT_UNUSED(size);
  #endif

  return kErrorOk;
}

// ============================================================================
// [asmjit::BaseAssembler - Events]
// ============================================================================

Error BaseAssembler::onAttach(CodeHolder* code) noexcept {
  ASMJIT_PROPAGATE(Base::onAttach(code));

  // Attach to the end of the .text section.
  BaseAssembler_initSection(this, code->_sections[0]);

  // And reset everything that is used temporarily.
  _op4.reset();
  _op5.reset();

  return kErrorOk;
}

Error BaseAssembler::onDetach(CodeHolder* code) noexcept {
  _section    = nullptr;
  _bufferData = nullptr;
  _bufferEnd  = nullptr;
  _bufferPtr  = nullptr;

  _op4.reset();
  _op5.reset();

  return Base::onDetach(code);
}

ASMJIT_END_NAMESPACE
