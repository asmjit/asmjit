// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Export]
#define ASMJIT_EXPORTS

// [Dependencies]
#include "../base/assembler.h"
#include "../base/constpool.h"
#include "../base/utils.h"
#include "../base/vmem.h"
#include <stdarg.h>

// [Api-Begin]
#include "../asmjit_apibegin.h"

namespace asmjit {

// ============================================================================
// [asmjit::Assembler - Construction / Destruction]
// ============================================================================

Assembler::Assembler() noexcept
  : CodeEmitter(kTypeAssembler),
    _section(nullptr),
    _bufferData(nullptr),
    _bufferEnd(nullptr),
    _bufferPtr(nullptr) {}

Assembler::~Assembler() noexcept {
  if (_code) sync();
}

// ============================================================================
// [asmjit::Assembler - Events]
// ============================================================================

Error Assembler::onAttach(CodeHolder* code) noexcept {
  // Attach to the end of the .text section.
  _section = code->_sections[0];
  uint8_t* p = _section->_buffer._data;

  _bufferData = p;
  _bufferEnd  = p + _section->_buffer._capacity;
  _bufferPtr  = p + _section->_buffer._length;
  return Base::onAttach(code);
}

Error Assembler::onDetach(CodeHolder* code) noexcept {
  _section    = nullptr;
  _bufferData = nullptr;
  _bufferEnd  = nullptr;
  _bufferPtr  = nullptr;
  return Base::onDetach(code);
}

// ============================================================================
// [asmjit::Assembler - Sync]
// ============================================================================

void Assembler::sync() noexcept {
  ASMJIT_ASSERT(_code != nullptr);                       // Only called by CodeHolder, so we must be attached.
  ASMJIT_ASSERT(_section != nullptr);                    // One section must always be active, no matter what.
  ASMJIT_ASSERT(_bufferData == _section->_buffer._data); // `_bufferStart` is a shortcut to `_section->buffer.data`.

  // Update only if the current offset is greater than the section length.
  size_t offset = (size_t)(_bufferPtr - _bufferData);
  if (_section->getBuffer().getLength() < offset)
    _section->_buffer._length = offset;
}

// ============================================================================
// [asmjit::Assembler - Code-Buffer]
// ============================================================================

Error Assembler::setOffset(size_t offset) {
  if (_lastError) return _lastError;

  size_t length = Utils::iMax(_section->getBuffer().getLength(), getOffset());
  if (offset > length)
    return setLastError(DebugUtils::errored(kErrorInvalidArgument));

  // If the `Assembler` generated any code the `_bufferPtr` may be higher than
  // the section length stored in `CodeHolder` as it doesn't update it each
  // time it generates machine code. This is the same as calling `sync()`.
  if (_section->_buffer._length < length)
    _section->_buffer._length = length;

  _bufferPtr = _bufferData + offset;
  return kErrorOk;
}

// ============================================================================
// [asmjit::Assembler - Comment]
// ============================================================================

Error Assembler::comment(const char* s, size_t len) {
  if (_lastError) return _lastError;

#if !defined(ASMJIT_DISABLE_LOGGING)
  if (_globalOptions & kOptionLoggingEnabled) {
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
// [asmjit::Assembler - Building Blocks]
// ============================================================================

Label Assembler::newLabel() {
  uint32_t id = kInvalidValue;
  if (!_lastError) {
    ASMJIT_ASSERT(_code != nullptr);
    Error err = _code->newLabelId(id);
    if (ASMJIT_UNLIKELY(err)) setLastError(err);
  }
  return Label(id);
}

Label Assembler::newNamedLabel(const char* name, size_t nameLength, uint32_t type, uint32_t parentId) {
  uint32_t id = kInvalidValue;
  if (!_lastError) {
    ASMJIT_ASSERT(_code != nullptr);
    Error err = _code->newNamedLabelId(id, name, nameLength, type, parentId);
    if (ASMJIT_UNLIKELY(err)) setLastError(err);
  }
  return Label(id);
}

Error Assembler::bind(const Label& label) {
  if (_lastError) return _lastError;
  ASMJIT_ASSERT(_code != nullptr);

  LabelEntry* le = _code->getLabelEntry(label);
  if (ASMJIT_UNLIKELY(!le))
    return setLastError(DebugUtils::errored(kErrorInvalidLabel));

  // Label can be bound only once.
  if (ASMJIT_UNLIKELY(le->isBound()))
    return setLastError(DebugUtils::errored(kErrorLabelAlreadyBound));

#if !defined(ASMJIT_DISABLE_LOGGING)
  if (_globalOptions & kOptionLoggingEnabled) {
    StringBuilderTmp<256> sb;
    sb.setFormat("L%u:", Operand::unpackId(label.getId()));

    size_t binSize = 0;
    if (!_code->_logger->hasOption(Logger::kOptionBinaryForm))
      binSize = kInvalidIndex;

    LogUtil::formatLine(sb, nullptr, binSize, 0, 0, getInlineComment());
    _code->_logger->log(sb.getData(), sb.getLength());
  }
#endif // !ASMJIT_DISABLE_LOGGING

  Error err = kErrorOk;
  size_t pos = getOffset();

  LabelLink* link = le->_links;
  LabelLink* prev = nullptr;

  while (link) {
    intptr_t offset = link->offset;
    uint32_t relocId = link->relocId;

    if (relocId != RelocEntry::kInvalidId) {
      // Adjust relocation data.
      RelocEntry* re = _code->_relocations[relocId];
      re->_data += static_cast<uint64_t>(pos);
    }
    else {
      // Not using relocId, this means that we are overwriting a real
      // displacement in the CodeBuffer.
      int32_t patchedValue = static_cast<int32_t>(
        static_cast<intptr_t>(pos) - offset + link->rel);

      // Size of the value we are going to patch. Only BYTE/DWORD is allowed.
      uint32_t size = _bufferData[offset];
      if (size == 4)
        Utils::writeI32u(_bufferData + offset, static_cast<int32_t>(patchedValue));
      else if (size == 1 && Utils::isInt8(patchedValue))
        _bufferData[offset] = static_cast<uint8_t>(patchedValue & 0xFF);
      else
        err = DebugUtils::errored(kErrorInvalidDisplacement);
    }

    prev = link->prev;
    _code->_unresolvedLabelsCount--;
    _code->_baseHeap.release(link, sizeof(LabelLink));

    link = prev;
  }

  // Set as bound.
  le->_sectionId = _section->getId();
  le->_offset = pos;
  le->_links = nullptr;
  resetInlineComment();

  if (err != kErrorOk)
    return setLastError(err);

  return kErrorOk;
}

Error Assembler::embed(const void* data, uint32_t size) {
  if (_lastError) return _lastError;

  if (getRemainingSpace() < size) {
    Error err = _code->growBuffer(&_section->_buffer, size);
    if (ASMJIT_UNLIKELY(err != kErrorOk)) return setLastError(err);
  }

  ::memcpy(_bufferPtr, data, size);
  _bufferPtr += size;

#if !defined(ASMJIT_DISABLE_LOGGING)
  if (_globalOptions & kOptionLoggingEnabled)
    _code->_logger->logBinary(data, size);
#endif // !ASMJIT_DISABLE_LOGGING

  return kErrorOk;
}

Error Assembler::embedLabel(const Label& label) {
  if (_lastError) return _lastError;
  ASMJIT_ASSERT(_code != nullptr);

  RelocEntry* re;
  LabelEntry* le = _code->getLabelEntry(label);

  if (ASMJIT_UNLIKELY(!le))
    return setLastError(DebugUtils::errored(kErrorInvalidLabel));

  Error err;
  uint32_t gpSize = getGpSize();

  if (getRemainingSpace() < gpSize) {
    err = _code->growBuffer(&_section->_buffer, gpSize);
    if (ASMJIT_UNLIKELY(err)) return setLastError(err);
  }

#if !defined(ASMJIT_DISABLE_LOGGING)
  if (_globalOptions & kOptionLoggingEnabled)
    _code->_logger->logf(gpSize == 4 ? ".dd L%u\n" : ".dq L%u\n", Operand::unpackId(label.getId()));
#endif // !ASMJIT_DISABLE_LOGGING

  err = _code->newRelocEntry(&re, RelocEntry::kTypeRelToAbs, gpSize);
  if (ASMJIT_UNLIKELY(err)) return setLastError(err);

  re->_sourceSectionId = _section->getId();
  re->_sourceOffset = static_cast<uint64_t>(getOffset());

  if (le->isBound()) {
    re->_targetSectionId = le->getSectionId();
    re->_data = static_cast<uint64_t>(static_cast<int64_t>(le->getOffset()));
  }
  else {
    LabelLink* link = _code->newLabelLink(le, _section->getId(), getOffset(), 0);
    if (ASMJIT_UNLIKELY(!link))
      return setLastError(DebugUtils::errored(kErrorNoHeapMemory));
    link->relocId = re->getId();
  }

  // Emit dummy DWORD/QWORD depending on the address size.
  ::memset(_bufferPtr, 0, gpSize);
  _bufferPtr += gpSize;

  return kErrorOk;
}

Error Assembler::embedConstPool(const Label& label, const ConstPool& pool) {
  if (_lastError) return _lastError;

  if (!isLabelValid(label))
    return DebugUtils::errored(kErrorInvalidLabel);

  ASMJIT_PROPAGATE(align(kAlignData, static_cast<uint32_t>(pool.getAlignment())));
  ASMJIT_PROPAGATE(bind(label));

  size_t size = pool.getSize();
  if (getRemainingSpace() < size) {
    Error err = _code->growBuffer(&_section->_buffer, size);
    if (ASMJIT_UNLIKELY(err)) return setLastError(err);
  }

  uint8_t* p = _bufferPtr;
  pool.fill(p);

#if !defined(ASMJIT_DISABLE_LOGGING)
  if (_globalOptions & kOptionLoggingEnabled)
    _code->_logger->logBinary(p, size);
#endif // !ASMJIT_DISABLE_LOGGING

  _bufferPtr += size;
  return kErrorOk;
}

} // asmjit namespace

// [Api-End]
#include "../asmjit_apiend.h"
