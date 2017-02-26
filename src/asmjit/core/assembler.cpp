// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Export]
#define ASMJIT_EXPORTS

// [Dependencies]
#include "../core/assembler.h"
#include "../core/constpool.h"
#include "../core/intutils.h"
#include "../core/logging.h"
#include "../core/memutils.h"

ASMJIT_BEGIN_NAMESPACE

// ============================================================================
// [asmjit::Assembler - Construction / Destruction]
// ============================================================================

Assembler::Assembler() noexcept
  : CodeEmitter(kTypeAssembler),
    _section(nullptr),
    _bufferData(nullptr),
    _bufferEnd(nullptr),
    _bufferPtr(nullptr),
    _op4(),
    _op5() {}
Assembler::~Assembler() noexcept {}

// ============================================================================
// [asmjit::Assembler - Buffer Management]
// ============================================================================

Error Assembler::setOffset(size_t offset) {
  if (ASMJIT_UNLIKELY(!_code))
    return DebugUtils::errored(kErrorNotInitialized);

  size_t length = std::max<size_t>(_section->getBuffer().getLength(), getOffset());
  if (ASMJIT_UNLIKELY(offset > length))
    return reportError(DebugUtils::errored(kErrorInvalidArgument));

  _bufferPtr = _bufferData + offset;
  return kErrorOk;
}

// ============================================================================
// [asmjit::Assembler - Label Management]
// ============================================================================

Label Assembler::newLabel() {
  uint32_t id = 0;
  if (ASMJIT_LIKELY(_code)) {
    Error err = _code->newLabelId(id);
    if (ASMJIT_UNLIKELY(err))
      reportError(err);
  }
  return Label(id);
}

Label Assembler::newNamedLabel(const char* name, size_t nameLength, uint32_t type, uint32_t parentId) {
  uint32_t id = 0;
  if (ASMJIT_LIKELY(_code)) {
    Error err = _code->newNamedLabelId(id, name, nameLength, type, parentId);
    if (ASMJIT_UNLIKELY(err))
      reportError(err);
  }
  return Label(id);
}

Error Assembler::bind(const Label& label) {
  if (ASMJIT_UNLIKELY(!_code))
    return DebugUtils::errored(kErrorNotInitialized);

  LabelEntry* le = _code->getLabelEntry(label);
  if (ASMJIT_UNLIKELY(!le))
    return reportError(DebugUtils::errored(kErrorInvalidLabel));

  // Label can be bound only once.
  if (ASMJIT_UNLIKELY(le->isBound()))
    return reportError(DebugUtils::errored(kErrorLabelAlreadyBound));

  #ifndef ASMJIT_DISABLE_LOGGING
  if (hasEmitterOption(kOptionLoggingEnabled)) {
    StringBuilderTmp<256> sb;
    if (le->hasName())
      sb.setFormat("%s:", le->getName());
    else
      sb.setFormat("L%u:", Operand::unpackId(label.getId()));

    size_t binSize = 0;
    if (!_code->_logger->hasOption(Logger::kOptionBinaryForm))
      binSize = std::numeric_limits<size_t>::max();

    Logging::formatLine(sb, nullptr, binSize, 0, 0, getInlineComment());
    _code->_logger->log(sb.getData(), sb.getLength());
  }
  #endif

  Error err = kErrorOk;
  size_t pos = getOffset();

  LabelLink* link = le->_links;
  LabelLink* prev = nullptr;

  while (link) {
    intptr_t offset = intptr_t(link->offset);
    uint32_t relocId = link->relocId;

    if (relocId != RelocEntry::kInvalidId) {
      // Adjust relocation data.
      RelocEntry* re = _code->_relocations[relocId];
      re->_data += uint64_t(pos);
    }
    else {
      // Not using relocId, this means that we are overwriting a real
      // displacement in the CodeBuffer.
      int32_t patchedValue = int32_t(intptr_t(pos) - offset + link->rel);

      // Size of the value we are going to patch. Only BYTE/DWORD is allowed.
      uint32_t size = _bufferData[offset];
      if (size == 4)
        MemUtils::writeI32u(_bufferData + offset, int32_t(patchedValue));
      else if (size == 1 && IntUtils::isInt8(patchedValue))
        _bufferData[offset] = uint8_t(patchedValue & 0xFF);
      else
        err = DebugUtils::errored(kErrorInvalidDisplacement);
    }

    prev = link->prev;
    _code->_unresolvedLabelCount--;
    _code->getAllocator()->release(link, sizeof(LabelLink));

    link = prev;
  }

  // Set as bound.
  le->_sectionId = _section->getId();
  le->_offset = intptr_t(pos);
  le->_links = nullptr;
  resetInlineComment();

  if (err != kErrorOk)
    return reportError(err);

  return kErrorOk;
}

// ============================================================================
// [asmjit::Assembler - Emit (Low-Level)]
// ============================================================================

Error Assembler::_emit(uint32_t instId, const Operand_& o0, const Operand_& o1, const Operand_& o2, const Operand_& o3, const Operand_& o4, const Operand_& o5) {
  _op4 = o4;
  _op5 = o5;
  _instOptions |= Inst::kOptionOp4Op5Used;
  return _emit(instId, o0, o1, o2, o3);
}

Error Assembler::_emitOpArray(uint32_t instId, const Operand_* operands, size_t count) {
  const Operand_* op = operands;
  const Operand& none_ = Globals::none;

  switch (count) {
    case 0: return _emit(instId, none_, none_, none_, none_);
    case 1: return _emit(instId, op[0], none_, none_, none_);
    case 2: return _emit(instId, op[0], op[1], none_, none_);
    case 3: return _emit(instId, op[0], op[1], op[2], none_);
    case 4: return _emit(instId, op[0], op[1], op[2], op[3]);

    case 5:
      _op4 = op[4];
      _op5.reset();
      _instOptions |= Inst::kOptionOp4Op5Used;
      return _emit(instId, op[0], op[1], op[2], op[3]);

    case 6:
      _op4 = op[4];
      _op5 = op[5];
      _instOptions |= Inst::kOptionOp4Op5Used;
      return _emit(instId, op[0], op[1], op[2], op[3]);

    default:
      return DebugUtils::errored(kErrorInvalidArgument);
  }
}

#ifndef ASMJIT_DISABLE_LOGGING
void Assembler::_emitLog(
  uint32_t instId, uint32_t options, const Operand_& o0, const Operand_& o1, const Operand_& o2, const Operand_& o3,
  uint32_t relSize, uint32_t imLen, uint8_t* afterCursor) {

  Logger* logger = _code->getLogger();
  ASMJIT_ASSERT(logger != nullptr);
  ASMJIT_ASSERT(options & CodeEmitter::kOptionLoggingEnabled);

  StringBuilderTmp<256> sb;
  uint32_t logOptions = logger->getOptions();

  uint8_t* beforeCursor = _bufferPtr;
  intptr_t emittedSize = (intptr_t)(afterCursor - beforeCursor);

  sb.appendString(logger->getIndentation());

  Operand_ operands[Globals::kMaxOpCount];
  operands[0].copyFrom(o0);
  operands[1].copyFrom(o1);
  operands[2].copyFrom(o2);
  operands[3].copyFrom(o3);

  if (options & Inst::kOptionOp4Op5Used) {
    operands[4].copyFrom(_op4);
    operands[5].copyFrom(_op5);
  }
  else {
    operands[4].reset();
    operands[5].reset();
  }

  Logging::formatInstruction(
    sb, logOptions,
    this, getArchType(),
    Inst::Detail(instId, options, _extraReg), operands, Globals::kMaxOpCount);

  if ((logOptions & Logger::kOptionBinaryForm) != 0)
    Logging::formatLine(sb, _bufferPtr, size_t(emittedSize), relSize, imLen, getInlineComment());
  else
    Logging::formatLine(sb, nullptr, std::numeric_limits<size_t>::max(), 0, 0, getInlineComment());

  logger->log(sb);
}

Error Assembler::_emitFailed(
  Error err,
  uint32_t instId, uint32_t options, const Operand_& o0, const Operand_& o1, const Operand_& o2, const Operand_& o3) {

  StringBuilderTmp<256> sb;
  sb.appendString(DebugUtils::errorAsString(err));
  sb.appendString(": ");

  Operand_ operands[Globals::kMaxOpCount];
  operands[0].copyFrom(o0);
  operands[1].copyFrom(o1);
  operands[2].copyFrom(o2);
  operands[3].copyFrom(o3);

  if (options & Inst::kOptionOp4Op5Used) {
    operands[4].copyFrom(_op4);
    operands[5].copyFrom(_op5);
  }
  else {
    operands[4].reset();
    operands[5].reset();
  }

  Logging::formatInstruction(
    sb, 0,
    this, getArchType(),
    Inst::Detail(instId, options, _extraReg), operands, Globals::kMaxOpCount);

  resetInstOptions();
  resetExtraReg();
  resetInlineComment();
  return reportError(err, sb.getData());
}
#endif

// ============================================================================
// [asmjit::Assembler - Embed]
// ============================================================================

Error Assembler::embed(const void* data, uint32_t size) {
  if (ASMJIT_UNLIKELY(!_code))
    return DebugUtils::errored(kErrorNotInitialized);

  if (size == 0)
    return DebugUtils::errored(kErrorInvalidArgument);

  AsmBufferWriter writer(this);
  ASMJIT_PROPAGATE(writer.ensureSpace(this, size));

  writer.emitData(data, size);

  #ifndef ASMJIT_DISABLE_LOGGING
  if (ASMJIT_UNLIKELY(hasEmitterOption(kOptionLoggingEnabled)))
    _code->_logger->logBinary(data, size);
  #endif

  writer.done(this);
  return kErrorOk;
}

Error Assembler::embedLabel(const Label& label) {
  if (ASMJIT_UNLIKELY(!_code))
    return DebugUtils::errored(kErrorNotInitialized);

  ASMJIT_ASSERT(_code != nullptr);
  RelocEntry* re;
  LabelEntry* le = _code->getLabelEntry(label);

  if (ASMJIT_UNLIKELY(!le))
    return reportError(DebugUtils::errored(kErrorInvalidLabel));

  uint32_t size = getGpSize();
  AsmBufferWriter writer(this);
  ASMJIT_PROPAGATE(writer.ensureSpace(this, size));

  #ifndef ASMJIT_DISABLE_LOGGING
  if (ASMJIT_UNLIKELY(hasEmitterOption(kOptionLoggingEnabled)))
    _code->_logger->logf(size == 4 ? ".dd L%u\n" : ".dq L%u\n", Operand::unpackId(label.getId()));
  #endif

  Error err = _code->newRelocEntry(&re, RelocEntry::kTypeRelToAbs, size);
  if (ASMJIT_UNLIKELY(err))
    return reportError(err);

  re->_sourceSectionId = _section->getId();
  re->_sourceOffset = uint64_t(getOffset());

  if (le->isBound()) {
    re->_targetSectionId = le->getSectionId();
    re->_data = uint64_t(int64_t(le->getOffset()));
  }
  else {
    LabelLink* link = _code->newLabelLink(le, _section->getId(), getOffset(), 0);
    if (ASMJIT_UNLIKELY(!link))
      return reportError(DebugUtils::errored(kErrorNoHeapMemory));
    link->relocId = re->getId();
  }

  // Emit dummy DWORD/QWORD depending on the address size.
  writer.emitZeros(size);
  writer.done(this);

  return kErrorOk;
}

Error Assembler::embedConstPool(const Label& label, const ConstPool& pool) {
  if (ASMJIT_UNLIKELY(!_code))
    return DebugUtils::errored(kErrorNotInitialized);

  if (ASMJIT_UNLIKELY(!isLabelValid(label)))
    return DebugUtils::errored(kErrorInvalidLabel);

  ASMJIT_PROPAGATE(align(kAlignData, uint32_t(pool.getAlignment())));
  ASMJIT_PROPAGATE(bind(label));

  size_t size = pool.getSize();
  AsmBufferWriter writer(this);
  ASMJIT_PROPAGATE(writer.ensureSpace(this, size));

  pool.fill(writer.getCursor());

  #ifndef ASMJIT_DISABLE_LOGGING
  if (ASMJIT_UNLIKELY(hasEmitterOption(kOptionLoggingEnabled)))
    _code->_logger->logBinary(writer.getCursor(), size);
  #endif

  writer.advance(size);
  writer.done(this);

  return kErrorOk;
}

// ============================================================================
// [asmjit::Assembler - Comment]
// ============================================================================

Error Assembler::comment(const char* s, size_t len) {
  if (ASMJIT_UNLIKELY(!_code))
    return DebugUtils::errored(kErrorNotInitialized);

  #ifndef ASMJIT_DISABLE_LOGGING
  if (hasEmitterOption(kOptionLoggingEnabled)) {
    Logger* logger = _code->getLogger();
    logger->log(s, len);
    logger->log("\n", 1);
    return kErrorOk;
  }
  #else
  ASMJIT_UNUSED(s);
  ASMJIT_UNUSED(len);
  #endif

  return kErrorOk;
}

// ============================================================================
// [asmjit::Assembler - Events]
// ============================================================================

Error Assembler::onAttach(CodeHolder* code) noexcept {
  ASMJIT_PROPAGATE(Base::onAttach(code));

  // Attach to the end of the .text section.
  _section = code->_sections[0];
  uint8_t* p = _section->_buffer._data;

  _bufferData = p;
  _bufferEnd  = p + _section->_buffer._capacity;
  _bufferPtr  = p + _section->_buffer._length;

  _op4.reset();
  _op5.reset();

  return kErrorOk;
}

Error Assembler::onDetach(CodeHolder* code) noexcept {
  _section    = nullptr;
  _bufferData = nullptr;
  _bufferEnd  = nullptr;
  _bufferPtr  = nullptr;

  _op4.reset();
  _op5.reset();

  return Base::onDetach(code);
}

ASMJIT_END_NAMESPACE
