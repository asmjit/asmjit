// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Export]
#define ASMJIT_EXPORTS

// [Dependencies - AsmJit]
#include "../base/assembler.h"
#include "../base/utils.h"
#include "../base/vmem.h"

// [Dependenceis - C]
#include <stdarg.h>

// [Api-Begin]
#include "../apibegin.h"

namespace asmjit {

// ============================================================================
// [asmjit::ErrorHandler]
// ============================================================================

ErrorHandler::ErrorHandler() noexcept {}
ErrorHandler::~ErrorHandler() noexcept {}

ErrorHandler* ErrorHandler::addRef() const noexcept {
  return const_cast<ErrorHandler*>(this);
}
void ErrorHandler::release() noexcept {}

// ============================================================================
// [asmjit::ExternalTool]
// ============================================================================

ExternalTool::ExternalTool() noexcept
  : _assembler(nullptr),
    _exId(0),
    _arch(kArchNone),
    _regSize(0),
    _finalized(false),
    _reserved(0),
    _lastError(kErrorNotInitialized) {}
ExternalTool::~ExternalTool() noexcept {}

Error ExternalTool::setLastError(Error error, const char* message) noexcept {
  // Special case, reset the last error the error is `kErrorOk`.
  if (error == kErrorOk)  {
    _lastError = kErrorOk;
    return kErrorOk;
  }

  // Don't do anything if the code-generator doesn't have associated assembler.
  Assembler* assembler = getAssembler();
  if (assembler == nullptr)
    return error;

  if (message == nullptr)
    message = DebugUtils::errorAsString(error);

  // Logging is skipped if the error is handled by `ErrorHandler.
  ErrorHandler* eh = assembler->getErrorHandler();
  ASMJIT_TLOG("[ERROR (ExternalTool)] %s (0x%0.8u) %s\n", message,
    static_cast<unsigned int>(error),
    !eh ? "(Possibly unhandled?)" : "");

  if (eh != nullptr && eh->handleError(error, message, this))
    return error;

#if !defined(ASMJIT_DISABLE_LOGGER)
  Logger* logger = assembler->getLogger();
  if (logger != nullptr)
    logger->logFormat(Logger::kStyleComment,
      "*** ERROR (ExternalTool): %s (0x%0.8u).\n", message,
      static_cast<unsigned int>(error));
#endif // !ASMJIT_DISABLE_LOGGER

  // The handler->handleError() function may throw an exception or longjmp()
  // to terminate the execution of `setLastError()`. This is the reason why
  // we have delayed changing the `_error` member until now.
  _lastError = error;
  return error;
}

// ============================================================================
// [asmjit::Assembler - Construction / Destruction]
// ============================================================================

Assembler::Assembler(Runtime* runtime) noexcept
  : _runtime(runtime),
    _logger(nullptr),
    _errorHandler(nullptr),
    _arch(kArchNone),
    _regSize(0),
    _reserved(0),
    _asmOptions(0),
    _instOptions(0),
    _lastError(runtime ? kErrorOk : kErrorNotInitialized),
    _exIdGenerator(0),
    _exCountAttached(0),
    _zoneAllocator(8192 - Zone::kZoneOverhead),
    _buffer(nullptr),
    _end(nullptr),
    _cursor(nullptr),
    _trampolinesSize(0),
    _comment(nullptr),
    _unusedLinks(nullptr),
    _labels(),
    _relocations() {}

Assembler::~Assembler() noexcept {
  reset(true);

  if (_errorHandler != nullptr)
    _errorHandler->release();
}

// ============================================================================
// [asmjit::Assembler - Reset]
// ============================================================================

void Assembler::reset(bool releaseMemory) noexcept {
  _asmOptions = 0;
  _instOptions = 0;
  _lastError = kErrorOk;
  _exIdGenerator = 0;
  _exCountAttached = 0;

  _zoneAllocator.reset(releaseMemory);

  if (releaseMemory && _buffer != nullptr) {
    ASMJIT_FREE(_buffer);
    _buffer = nullptr;
    _end = nullptr;
  }

  _cursor = _buffer;
  _trampolinesSize = 0;

  _comment = nullptr;
  _unusedLinks = nullptr;

  _sections.reset(releaseMemory);
  _labels.reset(releaseMemory);
  _relocations.reset(releaseMemory);
}

// ============================================================================
// [asmjit::Assembler - Logging & Error Handling]
// ============================================================================

Error Assembler::setLastError(Error error, const char* message) noexcept {
  // Special case, reset the last error the error is `kErrorOk`.
  if (error == kErrorOk)  {
    _lastError = kErrorOk;
    return kErrorOk;
  }

  if (message == nullptr)
    message = DebugUtils::errorAsString(error);

  // Logging is skipped if the error is handled by `ErrorHandler.
  ErrorHandler* eh = _errorHandler;
  ASMJIT_TLOG("[ERROR (Assembler)] %s (0x%0.8u) %s\n", message,
    static_cast<unsigned int>(error),
    !eh ? "(Possibly unhandled?)" : "");

  if (eh != nullptr && eh->handleError(error, message, this))
    return error;

#if !defined(ASMJIT_DISABLE_LOGGER)
  Logger* logger = _logger;
  if (logger != nullptr)
    logger->logFormat(Logger::kStyleComment,
      "*** ERROR (Assembler): %s (0x%0.8u).\n", message,
      static_cast<unsigned int>(error));
#endif // !ASMJIT_DISABLE_LOGGER

  // The handler->handleError() function may throw an exception or longjmp()
  // to terminate the execution of `setLastError()`. This is the reason why
  // we have delayed changing the `_error` member until now.
  _lastError = error;
  return error;
}

Error Assembler::setErrorHandler(ErrorHandler* handler) noexcept {
  ErrorHandler* oldHandler = _errorHandler;

  if (oldHandler != nullptr)
    oldHandler->release();

  if (handler != nullptr)
    handler = handler->addRef();

  _errorHandler = handler;
  return kErrorOk;
}

// ============================================================================
// [asmjit::Assembler - Buffer]
// ============================================================================

Error Assembler::_grow(size_t n) noexcept {
  size_t capacity = getCapacity();
  size_t after = getOffset() + n;

  // Overflow.
  if (n > IntTraits<uintptr_t>::maxValue() - capacity)
    return setLastError(kErrorNoHeapMemory);

  // Grow is called when allocation is needed, so it shouldn't happen, but on
  // the other hand it is simple to catch and it's not an error.
  if (after <= capacity)
    return kErrorOk;

  if (capacity < kMemAllocOverhead)
    capacity = kMemAllocOverhead;
  else
    capacity += kMemAllocOverhead;

  do {
    size_t oldCapacity = capacity;

    if (capacity < kMemAllocGrowMax)
      capacity *= 2;
    else
      capacity += kMemAllocGrowMax;

    // Overflow.
    if (oldCapacity > capacity)
      return setLastError(kErrorNoHeapMemory);
  } while (capacity - kMemAllocOverhead < after);

  capacity -= kMemAllocOverhead;
  return _reserve(capacity);
}

Error Assembler::_reserve(size_t n) noexcept {
  size_t capacity = getCapacity();
  if (n <= capacity)
    return kErrorOk;

  uint8_t* newBuffer;
  if (_buffer == nullptr)
    newBuffer = static_cast<uint8_t*>(ASMJIT_ALLOC(n));
  else
    newBuffer = static_cast<uint8_t*>(ASMJIT_REALLOC(_buffer, n));

  if (newBuffer == nullptr)
    return setLastError(kErrorNoHeapMemory);

  size_t offset = getOffset();

  _buffer = newBuffer;
  _end = _buffer + n;
  _cursor = newBuffer + offset;

  return kErrorOk;
}

// ============================================================================
// [asmjit::Assembler - Label]
// ============================================================================

Error Assembler::_newLabelId() noexcept {
  LabelData* data = _zoneAllocator.allocT<LabelData>();

  data->offset = -1;
  data->links = nullptr;
  data->exId = 0;
  data->exData = nullptr;

  uint32_t id = OperandUtil::makeLabelId(static_cast<uint32_t>(_labels.getLength()));
  Error error = _labels.append(data);

  if (error != kErrorOk) {
    setLastError(kErrorNoHeapMemory);
    return kInvalidValue;
  }

  return id;
}

LabelLink* Assembler::_newLabelLink() noexcept {
  LabelLink* link = _unusedLinks;

  if (link) {
    _unusedLinks = link->prev;
  }
  else {
    link = _zoneAllocator.allocT<LabelLink>();
    if (link == nullptr)
      return nullptr;
  }

  link->prev = nullptr;
  link->offset = 0;
  link->displacement = 0;
  link->relocId = -1;

  return link;
}

Error Assembler::bind(const Label& label) noexcept {
  // Get label data based on label id.
  uint32_t index = label.getId();
  LabelData* data = getLabelData(index);

  // Label can be bound only once.
  if (data->offset != -1)
    return setLastError(kErrorLabelAlreadyBound);

#if !defined(ASMJIT_DISABLE_LOGGER)
  if (_logger) {
    StringBuilderTmp<256> sb;
    sb.setFormat("L%u:", index);

    size_t binSize = 0;
    if (!_logger->hasOption(Logger::kOptionBinaryForm))
      binSize = kInvalidIndex;

    LogUtil::formatLine(sb, nullptr, binSize, 0, 0, _comment);
    _logger->logString(Logger::kStyleLabel, sb.getData(), sb.getLength());
  }
#endif // !ASMJIT_DISABLE_LOGGER

  Error error = kErrorOk;
  size_t pos = getOffset();

  LabelLink* link = data->links;
  LabelLink* prev = nullptr;

  while (link) {
    intptr_t offset = link->offset;

    if (link->relocId != -1) {
      // Handle RelocData - We have to update RelocData information instead of
      // patching the displacement in LabelData.
      _relocations[link->relocId].data += static_cast<Ptr>(pos);
    }
    else {
      // Not using relocId, this means that we are overwriting a real
      // displacement in the binary stream.
      int32_t patchedValue = static_cast<int32_t>(
        static_cast<intptr_t>(pos) - offset + link->displacement);

      // Size of the value we are going to patch. Only BYTE/DWORD is allowed.
      uint32_t size = readU8At(offset);
      ASMJIT_ASSERT(size == 1 || size == 4);

      if (size == 4) {
        writeI32At(offset, patchedValue);
      }
      else {
        ASMJIT_ASSERT(size == 1);
        if (Utils::isInt8(patchedValue))
          writeU8At(offset, static_cast<uint32_t>(patchedValue) & 0xFF);
        else
          error = kErrorIllegalDisplacement;
      }
    }

    prev = link->prev;
    link = prev;
  }

  // Chain unused links.
  link = data->links;
  if (link) {
    if (prev == nullptr)
      prev = link;

    prev->prev = _unusedLinks;
    _unusedLinks = link;
  }

  // Set as bound (offset is zero or greater and no links).
  data->offset = pos;
  data->links = nullptr;

  if (error != kErrorOk)
    return setLastError(error);

  _comment = nullptr;
  return error;
}

// ============================================================================
// [asmjit::Assembler - Embed]
// ============================================================================

Error Assembler::embed(const void* data, uint32_t size) noexcept {
  if (getRemainingSpace() < size) {
    Error error = _grow(size);
    if (error != kErrorOk)
      return setLastError(error);
  }

  uint8_t* cursor = getCursor();
  ::memcpy(cursor, data, size);
  setCursor(cursor + size);

#if !defined(ASMJIT_DISABLE_LOGGER)
  if (_logger)
    _logger->logBinary(Logger::kStyleData, data, size);
#endif // !ASMJIT_DISABLE_LOGGER

  return kErrorOk;
}

// ============================================================================
// [asmjit::Assembler - Reloc]
// ============================================================================

size_t Assembler::relocCode(void* dst, Ptr baseAddress) const noexcept {
  if (baseAddress == kNoBaseAddress)
    baseAddress = static_cast<Ptr>((uintptr_t)dst);
  return _relocCode(dst, baseAddress);
}

// ============================================================================
// [asmjit::Assembler - Make]
// ============================================================================

void* Assembler::make() noexcept {
  // Do nothing on error condition or if no instruction has been emitted.
  if (_lastError != kErrorOk || getCodeSize() == 0)
    return nullptr;

  void* p;
  Error error = _runtime->add(&p, this);

  if (error != kErrorOk)
    setLastError(error);

  return p;
}

// ============================================================================
// [asmjit::Assembler - Emit (Helpers)]
// ============================================================================

#define NA noOperand

Error Assembler::emit(uint32_t code) {
  return _emit(code, NA, NA, NA, NA);
}

Error Assembler::emit(uint32_t code, const Operand& o0) {
  return _emit(code, o0, NA, NA, NA);
}

Error Assembler::emit(uint32_t code, const Operand& o0, const Operand& o1) {
  return _emit(code, o0, o1, NA, NA);
}

Error Assembler::emit(uint32_t code, const Operand& o0, const Operand& o1, const Operand& o2) {
  return _emit(code, o0, o1, o2, NA);
}

Error Assembler::emit(uint32_t code, const Operand& o0, const Operand& o1, const Operand& o2, const Operand& o3) {
  return _emit(code, o0, o1, o2, o3);
}

Error Assembler::emit(uint32_t code, int o0) {
  return _emit(code, Imm(o0), NA, NA, NA);
}

Error Assembler::emit(uint32_t code, const Operand& o0, int o1) {
  return _emit(code, o0, Imm(o1), NA, NA);
}

Error Assembler::emit(uint32_t code, const Operand& o0, const Operand& o1, int o2) {
  return _emit(code, o0, o1, Imm(o2), NA);
}

Error Assembler::emit(uint32_t code, const Operand& o0, const Operand& o1, const Operand& o2, int o3) {
  return _emit(code, o0, o1, o2, Imm(o3));
}

Error Assembler::emit(uint32_t code, int64_t o0) {
  return _emit(code, Imm(o0), NA, NA, NA);
}

Error Assembler::emit(uint32_t code, const Operand& o0, int64_t o1) {
  return _emit(code, o0, Imm(o1), NA, NA);
}

Error Assembler::emit(uint32_t code, const Operand& o0, const Operand& o1, int64_t o2) {
  return _emit(code, o0, o1, Imm(o2), NA);
}

Error Assembler::emit(uint32_t code, const Operand& o0, const Operand& o1, const Operand& o2, int64_t o3) {
  return _emit(code, o0, o1, o2, Imm(o3));
}

#undef NA

} // asmjit namespace

// [Api-End]
#include "../apiend.h"
