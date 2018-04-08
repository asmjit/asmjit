// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Export]
#define ASMJIT_EXPORTS

// [Dependencies]
#include "../core/intutils.h"
#include "../core/logging.h"

#ifdef ASMJIT_BUILD_X86
  #include "../x86/x86internal_p.h"
  #include "../x86/x86instdb.h"
#endif // ASMJIT_BUILD_X86

#ifdef ASMJIT_BUILD_ARM
  #include "../arm/arminternal_p.h"
  #include "../arm/arminstdb.h"
#endif // ASMJIT_BUILD_ARM

ASMJIT_BEGIN_NAMESPACE

// ============================================================================
// [asmjit::BaseEmitter - Construction / Destruction]
// ============================================================================

BaseEmitter::BaseEmitter(uint32_t type) noexcept
  : _type(uint8_t(type)),
    _reserved(0),
    _flags(0),
    _code(nullptr),
    _errorHandler(nullptr),
    _codeInfo(),
    _gpRegInfo(),
    _emitterOptions(0),
    _privateData(0),
    _instOptions(0),
    _globalInstOptions(BaseInst::kOptionReserved),
    _extraReg(),
    _inlineComment(nullptr) {}

BaseEmitter::~BaseEmitter() noexcept {
  if (_code) {
    _addFlags(kFlagDestroyed);
    _code->detach(this);
  }
}

void BaseEmitter::moveFrom(BaseEmitter& o) noexcept {
  _type = o._type;
  _reserved = o._reserved;
  _flags = o._flags;
  _code = o._code;
  _errorHandler = o._errorHandler;
  _codeInfo = std::move(o._codeInfo);
  _gpRegInfo = std::move(o._gpRegInfo);
  _emitterOptions = o._emitterOptions;
  _privateData = o._privateData;
  _instOptions = o._instOptions;
  _globalInstOptions = o._globalInstOptions;
  _extraReg = std::move(o._extraReg);
  _inlineComment = o._inlineComment;

  o._code = nullptr;
  o._errorHandler = nullptr;
  o._inlineComment = nullptr;
}

// ============================================================================
// [asmjit::BaseEmitter - Code-Generation]
// ============================================================================

Error BaseEmitter::_emitOpArray(uint32_t instId, const Operand_* operands, size_t count) {
  const Operand_* op = operands;
  const Operand& none_ = Globals::none;

  switch (count) {
    case  0: return _emit(instId, none_, none_, none_, none_);
    case  1: return _emit(instId, op[0], none_, none_, none_);
    case  2: return _emit(instId, op[0], op[1], none_, none_);
    case  3: return _emit(instId, op[0], op[1], op[2], none_);
    case  4: return _emit(instId, op[0], op[1], op[2], op[3]);
    case  5: return _emit(instId, op[0], op[1], op[2], op[3], op[4], none_);
    case  6: return _emit(instId, op[0], op[1], op[2], op[3], op[4], op[5]);
    default: return DebugUtils::errored(kErrorInvalidArgument);
  }
}

// ============================================================================
// [asmjit::BaseEmitter - Finalize]
// ============================================================================

Label BaseEmitter::labelByName(const char* name, size_t nameSize, uint32_t parentId) noexcept {
  return Label(_code ? _code->labelIdByName(name, nameSize, parentId) : uint32_t(0));
}

// ============================================================================
// [asmjit::BaseEmitter - Finalize]
// ============================================================================

Error BaseEmitter::finalize() {
  // Does nothing by default, overridden by `BaseBuilder` and `BaseCompiler`.
  return kErrorOk;
}

// ============================================================================
// [asmjit::BaseEmitter - Error Handling]
// ============================================================================

Error BaseEmitter::reportError(Error err, const char* message) {
  ErrorHandler* handler = errorHandler();
  if (!handler) {
    if (code())
      handler = code()->errorHandler();
  }

  if (handler) {
    if (!message)
      message = DebugUtils::errorAsString(err);
    handler->handleError(err, message, this);
  }

  return err;
}

// ============================================================================
// [asmjit::BaseEmitter - Label Management]
// ============================================================================

bool BaseEmitter::isLabelValid(uint32_t id) const noexcept {
  uint32_t index = Operand::unpackId(id);
  return _code && index < _code->labelCount();
}

// ============================================================================
// [asmjit::BaseEmitter - Emit (High-Level)]
// ============================================================================

ASMJIT_FAVOR_SIZE Error BaseEmitter::emitProlog(const FuncFrame& frame) {
  if (ASMJIT_UNLIKELY(!_code))
    return DebugUtils::errored(kErrorNotInitialized);

  #ifdef ASMJIT_BUILD_X86
  if (archInfo().isX86Family())
    return x86::X86Internal::emitProlog(as<x86::Emitter>(), frame);
  #endif

  #ifdef ASMJIT_BUILD_ARM
  if (archInfo().isArmFamily())
    return arm::ArmInternal::emitProlog(as<arm::Emitter>(), frame);
  #endif

  return DebugUtils::errored(kErrorInvalidArch);
}

ASMJIT_FAVOR_SIZE Error BaseEmitter::emitEpilog(const FuncFrame& frame) {
  if (ASMJIT_UNLIKELY(!_code))
    return DebugUtils::errored(kErrorNotInitialized);

  #ifdef ASMJIT_BUILD_X86
  if (archInfo().isX86Family())
    return x86::X86Internal::emitEpilog(as<x86::Emitter>(), frame);
  #endif

  #ifdef ASMJIT_BUILD_ARM
  if (archInfo().isArmFamily())
    return arm::ArmInternal::emitEpilog(as<arm::Emitter>(), frame);
  #endif

  return DebugUtils::errored(kErrorInvalidArch);
}

ASMJIT_FAVOR_SIZE Error BaseEmitter::emitArgsAssignment(const FuncFrame& frame, const FuncArgsAssignment& args) {
  if (ASMJIT_UNLIKELY(!_code))
    return DebugUtils::errored(kErrorNotInitialized);

  #ifdef ASMJIT_BUILD_X86
  if (archInfo().isX86Family())
    return x86::X86Internal::emitArgsAssignment(as<x86::Emitter>(), frame, args);
  #endif

  #ifdef ASMJIT_BUILD_ARM
  if (archInfo().isArmFamily())
    return arm::ArmInternal::emitArgsAssignment(as<arm::Emitter>(), frame, args);
  #endif

  return DebugUtils::errored(kErrorInvalidArch);
}

// ============================================================================
// [asmjit::BaseEmitter - Comment]
// ============================================================================

Error BaseEmitter::commentf(const char* fmt, ...) {
  if (ASMJIT_UNLIKELY(!_code))
    return DebugUtils::errored(kErrorNotInitialized);

  #ifndef ASMJIT_DISABLE_LOGGING
  StringBuilderTmp<1024> sb;

  std::va_list ap;
  va_start(ap, fmt);
  Error err = sb.appendFormatVA(fmt, ap);
  va_end(ap);

  if (ASMJIT_UNLIKELY(err))
    return err;

  return comment(sb.data(), sb.size());
  #else
  ASMJIT_UNUSED(fmt);
  return kErrorOk;
  #endif
}

Error BaseEmitter::commentv(const char* fmt, std::va_list ap) {
  if (ASMJIT_UNLIKELY(!_code))
    return DebugUtils::errored(kErrorNotInitialized);

  #ifndef ASMJIT_DISABLE_LOGGING
  StringBuilderTmp<1024> sb;

  Error err = sb.appendFormatVA(fmt, ap);
  if (ASMJIT_UNLIKELY(err))
    return err;

  return comment(sb.data(), sb.size());
  #else
  ASMJIT_UNUSED(fmt);
  ASMJIT_UNUSED(ap);
  return kErrorOk;
  #endif
}

// ============================================================================
// [asmjit::BaseEmitter - Events]
// ============================================================================

Error BaseEmitter::onAttach(CodeHolder* code) noexcept {
  _code = code;
  _codeInfo = code->codeInfo();
  _emitterOptions = code->emitterOptions();

  onUpdateGlobalInstOptions();
  return kErrorOk;
}

Error BaseEmitter::onDetach(CodeHolder* code) noexcept {
  ASMJIT_UNUSED(code);

  _flags = 0;
  _errorHandler = nullptr;

  _codeInfo.reset();
  _gpRegInfo.reset();

  _emitterOptions = 0;
  _privateData = 0;

  _instOptions = 0;
  _globalInstOptions = BaseInst::kOptionReserved;
  _extraReg.reset();
  _inlineComment = nullptr;

  return kErrorOk;
}

void BaseEmitter::onUpdateGlobalInstOptions() noexcept {
  constexpr uint32_t kCriticalEmitterOptions =
    kOptionLoggingEnabled   |
    kOptionStrictValidation ;

  _globalInstOptions &= ~BaseInst::kOptionReserved;
  if ((_emitterOptions & kCriticalEmitterOptions) != 0)
    _globalInstOptions |= BaseInst::kOptionReserved;
}

ASMJIT_END_NAMESPACE
